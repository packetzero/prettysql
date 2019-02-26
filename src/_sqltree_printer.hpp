#pragma once
#include <simplesql_tree.h>

enum RenderMode {
  MODE_TEXT = 0
  , MODE_HTML
  , MODE_CTERM
  , MODE_ONELINE
};

/*
 * SqlTreePrinter is a visitor that prints the tree in HTML or
 * text (formatted or unformatted).
 * Use /misc/sqlstyle.css for HTML.
 */
struct SqlTreePrinter : public SqlTreeVisitor<std::string>, public SqlTreeExprVisitor<std::string>, public SqlTreeStmtVisitor<std::string>  {

  static const int MAX_LINE_LEN = 72;

  void setMode(RenderMode mode) { _mode = mode; }

  RenderMode getMode() { return _mode; }

  std::string op_s(std::string str) {
    if (_mode == MODE_HTML) {
      if (str == "AS") {
        return "<div class='op_as'>" + str + "</div>";
      }
      return "<div class='op'>" + str + "</div>";
    }
    return str;
  }

  std::string keyword_s(std::string str) {
    if (_mode == MODE_HTML) {
      return "<div class='keyword'>" + str + "</div>";
    }
    return str;
  }

  std::string function_s(std::string str) {
    if (_mode == MODE_HTML) {
      return "<div class='function'>" + str + "</div>";
    }
    return str;
  }

  std::string table_s(std::string str) {
    if (_mode == MODE_HTML) {
      return "<div class='tableref'>" + str + "</div>";
    }
    return str;
  }

  std::string fieldname_s(std::string str) {
    if (_mode == MODE_HTML && _clause == "SELECT") {
      return "<div class='field'>" + str + "</div>";
    }
    return str;
  }

  std::string indentstr(uint32_t level) {
    std::string s;
    while (level > 0) {
      if (_mode == MODE_HTML) {
        s += " &nbsp;&nbsp; ";
      } else {
        s += "  ";
      }
      level--;
    }
    return s;
  }

  inline std::string newline(int level = 0) {
    if (_mode == MODE_HTML) {
      return "<BR>\n" + indentstr(level);
    } else if (_mode == MODE_ONELINE) {
      return " ";
    }

    return "\n" + indentstr(level);
  }

  static std::string op_to_s(UnaryOP op) {
    switch(op) {
      case UOP_NOT: return "NOT";
      case UOP_PLUS: return "+";
      case UOP_MINUS: return "-";
      default:
        break;
    }
    return "?";
  }
  static std::string op_to_s(OP op) {
    switch(op) {
      case OP_OR: return "OR";
      case OP_AND: return "AND";
      case OP_EQ: return "="; // ==
      case OP_NEQ: return "!=";

      case OP_LT: return "<";
      case OP_GT: return ">";
      case OP_LTE: return "<=";
      case OP_GTE: return ">=";
      case OP_NOT: return "NOT";

      case OP_SUB: return "-";
      case OP_ADD: return "+";
      case OP_MUL: return "*";
      case OP_DIV: return "/";
      case OP_LIKE: return "LIKE";
      case OP_REGEX: return "TODO:REGEX";
      case OP_MATCH: return "TODO:MATCH";
      case OP_IS: return "IS";

      default:
        break;
    }
    return "?";
  }

  virtual std::string visit(SPCtxNode spNode, int level) override {
    switch(spNode->getType()) {
      case NODE_OPCOMPARE:
        return visit(*(static_cast<CtxOpCompare*>(spNode.get())), level);
      case NODE_OPARITH:
        return visit(* static_cast<CtxOpArith*>(spNode.get()), level);
      case NODE_LITERAL:
        return visit(* static_cast<CtxLiteralValue*>(spNode.get()), level);
      case NODE_GETVAL:
        return visit(* static_cast<CtxOpGetValue*>(spNode.get()), level);
      case NODE_INLIST:
        return visit(* static_cast<CtxOpInList*>(spNode.get()), level);
      case NODE_LIKE:
        return visit(* static_cast<CtxOpLike*>(spNode.get()), level);
      case NODE_EXPR_LIST:
        return visit(* static_cast<CtxExprList*>(spNode.get()), level);
      case NODE_FUNCTION_CALL:
        return visit(* static_cast<CtxFunctionCall*>(spNode.get()), level);
      case NODE_UNARY_OP:
        return visit(* static_cast<CtxUnaryOp*>(spNode.get()), level);

      case NODE_RESULT_COLUMN:
        return visit(* static_cast<CtxResultColumn*>(spNode.get()), level);
      case NODE_SELECT_CORE:
        return visit(* static_cast<CtxSelectCore*>(spNode.get()), level);
      case NODE_TABLE_OR_SUBQUERY:
        return visit(* static_cast<CtxTableOrSubquery*>(spNode.get()), level);
      case NODE_SELECT_VALUES:
        return visit(* static_cast<CtxSelectValues*>(spNode.get()), level);
      case NODE_JOIN_CLAUSE:
        return visit(* static_cast<CtxJoinClause*>(spNode.get()), level);
      case NODE_JOINER:
        return visit(* static_cast<CtxJoiner*>(spNode.get()), level);
      case NODE_JOIN_CONSTRAINT:
        return visit(* static_cast<CtxJoinConstraint*>(spNode.get()), level);
      default:
        break;
    }
    std::string retval;
    return retval;
  }

  std::string visit(CtxFunctionCall &node, int level) override {
    std::string s = function_s(node._function_name);
    s += "( ";
    for (int i=0; i < node._params.size(); i++) {
      if (i > 0) {
        s += ", ";
      }
      s += visit(node._params[i], level);
    }
    s += " )";
    return s;
  }

  std::string visit(CtxUnaryOp &node, int level) override {
    return op_s(op_to_s(node._op));
  }

  std::string visit(CtxOpCompare &node, int level) override {
    std::string s = "";

    std::string opstr;
    if (node._op == OP_IS) {
      opstr = op_s(op_to_s(node._op) + (node._isNot?" NOT":""));
    } else {
      opstr = op_s((node._isNot?"NOT ":"") + op_to_s(node._op));
    }

    auto l = visit(node._left, level);
      auto r = visit(node._right, level + 1);
      if (!node._left->isLeaf() && level > 0) {
        s += "( " + l + " )";
      } else {
        s += l;
      }

      // ASSUMING all ops are LEFT-ASSOCIATIVE

      if (fitsLine(l,r)) {
        s += " " + opstr;
        s += " ";
      } else {
        s += newline(level) + opstr + " ";
      }
      if (!node._right->isLeaf()) {
        s += "( " + r + " )";
      } else {
        s += r;
      }
    return s;
  }

  std::string visit(CtxOpArith &node, int level) override {
    std::string s = "";

    std::string opstr = op_s(op_to_s(node._op));

    auto l = visit(node._left,level);
    auto r = visit(node._right, level + 1);
    if ((!node._left->isLeaf() && level > 0) || (node._op == OP_MUL || node._op == OP_DIV)) {
      s += "( " + l + " )";
    } else {
      s += l;
    }

    // ASSUMING all ops are LEFT-ASSOCIATIVE

    if (fitsLine(l,r)) {
      s += " " + opstr;
      s += " ";
    } else {
      s += newline(level) + opstr + " ";
    }
    if (!node._right->isLeaf()) {
      s += "( " + r + " )";
    } else {
      s += r;
    }
    return s;
  }

  std::string visit(CtxOpGetValue &node, int level) override {
    return node._name;
  }

  std::string visit(CtxLiteralValue &node, int level) override {
    std::string valstr = (node._isNull ? "NULL" : node._value.as_s());
    if (node._value.type() == TSTRING) {
      if (_mode == MODE_HTML) {
        return "<div class='stringval'>'" + valstr + "'</div>";
      }
      return "'" + valstr + "'";
    }
    return valstr;
  }

  std::string visit(CtxOpInList &node, int level) override {

    std::string s;
    std::string l = visit(node.value,0);

    if (node.list == nullptr && !node.valueListName.empty()) {
      // special case - replaced list with function call
      return "incvl('" + l + "', '" + node.valueListName + "')";
    }

    s += l  + " " + keyword_s("IN") +" (";

    int max_item_chars = 0;
    int total_chars = 0;
    std::vector<std::string> renderedItems;
    for (auto item : node.list->items) {
      std::string tmp;
      if (!renderedItems.empty()) { tmp += ", "; }
      tmp += visit(item,0);
      total_chars += tmp.size();
      if (tmp.size() > max_item_chars) {
        max_item_chars = tmp.size();
      }
      renderedItems.push_back(tmp);
    }
    int num_rows = 1;
    int num_cols = 1;
    if (total_chars > MAX_LINE_LEN) {
      // break up into lines
      num_rows = 1 +  (total_chars / MAX_LINE_LEN);
      num_cols = 1 + renderedItems.size() / num_rows;
    }
    for (int i=0; i < renderedItems.size(); i++) {
      if (num_rows > 1 && i % num_cols == 0) {
        s += newline(level + 1);
      }
      s += renderedItems[i];
    }
    s += ")";
    return s;
  }

  std::string visit(CtxOpLike &node, int level) override {
    std::string s = "";

    std::string opstr = keyword_s((node._isNot?"NOT ":"") + op_to_s(OP_LIKE));

    auto l = visit(node._left, level);
    auto r = visit(node._right, level + 1);
    if (!node._left->isLeaf() && level > 0) {
      s += "( " + l + " )";
    } else {
      s += l;
    }

    // ASSUMING all ops are LEFT-ASSOCIATIVE

    if (fitsLine(l,r)) {
      s += " " + opstr;
      s += " ";
    } else {
      s += newline();
      s += indentstr(level) + opstr + " ";
    }
    if (!node._right->isLeaf()) {
      s += "( " + r + " )";
    } else {
      s += r;
    }
    return s;
  }

  std::string visit(CtxExprList &node, int level) override {
    std::string s;
    for (SPCtxNode &item : node.items) {
      if (!s.empty()) { s += ", "; }
      s += visit(item,0);
    }
    return s;
  }

  std::string visit(CtxResultColumn &node, int level) override {
    if (nullptr == node._expr) {
      if (node._column_name.empty()) {
        if (node._table_name.empty()) {
          return fieldname_s("*");
        }
        return fieldname_s(node._table_name + ".*");
      } else {
        std::string s;
        if (!node._table_name.empty()) {
          s += node._table_name + ".";
        }
        s += node._column_name;

        if (!node._alias.empty()) {
          s += " " + op_s("AS") + " ";
          s += fieldname_s(node._alias);
          return s;
        } else {
          return fieldname_s(s);
        }
      }
    }
    std::string s = visit(node._expr, level);
    if (!node._alias.empty()) {
      s += " " + op_s("AS") + " ";
      s += fieldname_s(node._alias);
    }
    return s;
  }

  std::string visit(CtxSelectCore &node, int level) override {
    std::string s;

    _clause = "SELECT";
    if (level > 0) { s += newline(level); }
    s += keyword_s("SELECT") + " ";
    if (node._isDistinct) {
      s += " " + keyword_s("DISTINCT") + " ";
    }

    int max_item_chars = 0;
    int total_chars = 0;
    std::vector<std::string> renderedItems;
    for (SPCtxNode rc : node._columns) {
      std::string tmp;
      if (!renderedItems.empty()) { tmp += ", "; }

      //if (i++ > 0) { s += ", "; }
      //s += visit(rc, 0);

      tmp += visit(rc, 0);
      total_chars += tmp.size();
      if (tmp.size() > max_item_chars) {
        max_item_chars = tmp.size();
      }
      renderedItems.push_back(tmp);
    }

    int num_rows = 1;
    int num_cols = 1;
    if (total_chars > MAX_LINE_LEN) {
      // break up into lines
      num_rows = 1 +  (total_chars / MAX_LINE_LEN);
      num_cols = 1 + renderedItems.size() / num_rows;
    }
    for (int i=0; i < renderedItems.size(); i++) {
      if (num_rows > 1 && i > 0 && i % num_cols == 0) {
        s += newline(level + 1);
      }
      s += renderedItems[i];
    }

    int i = 0;
    if (!node._table_or_subqueries.empty() || node._joinClause) {
      s += newline(level);
//      indentstr(level);
      _clause = "FROM";
      s += keyword_s("FROM") + " ";
      i=0;
      for (SPCtxNode node : node._table_or_subqueries) {
        if (i++ > 0) { s += ", "; }
        s += visit(node, level);
      }
      if (node._joinClause) {
        s += " " + visit(node._joinClause, level);
      }
    }
    if (node._whereExpr) {
      s += newline();
      if (level > 0) { s += indentstr(level); }
      _clause = "WHERE";
      s += keyword_s("WHERE") + " ";
      s += visit(node._whereExpr, level + 1);
    }
    if (!node._groupExprs.empty()) {
      s += newline();
      if (level > 0) { s += indentstr(level); }
      _clause = "GROUP";
      s += keyword_s("GROUP BY") + " ";
      i=0;
      for (SPCtxNode node : node._groupExprs) {
        if (i++ > 0) { s += ", "; }
        s += visit(node, level+1);
      }
      if (node._havingExpr) {
        s += newline();
        if (level > 0) { s += indentstr(level); }
        _clause = "HAVING";
        s += keyword_s("HAVING") + " ";
        s += visit(node._havingExpr, level + 1);
      }
    }
    return s;
  }

  std::string visit(CtxTableOrSubquery &node, int level) override {
    std::string s;
    std::string table;
    if (!node._table_name.empty()) {
      if (!node._database_name.empty()) {
        table += node._database_name + ".";
      }
      table += node._table_name;
      s += table_s(table);
    } else if (nullptr != node._selectStmt) {
      s += "( ";
      s += visit(node._selectStmt, level + 1);
      s += newline(level) + ")";
    } else {
      s += " ( ";
      int i=0;
      for (auto node : node._table_or_subqueries) {
        if (i++ > 0) { s += ", "; }
        s += visit(node, level + 1);
      }
      if (nullptr != node._joinClause) {
        s += visit(node._joinClause, level );
      }
      s += " ) ";
    }
    if (!node._alias.empty()) {
      s += " " + op_s("AS") + " ";
      s += node._alias;
    }
    return s;
  }

  virtual std::string visit(CtxSelectValues &node, int level) override {
    std::string s;
    std::vector<std::string> row_strs;
    for (auto &row : node._rows) {
      std::string tmp = "( ";
      int i = 0;
      for (auto &expr : row) {
        if (i++ > 0) {
          tmp += ", ";
        }
        tmp += visit(expr, 0);
      }
      tmp += " )";
      row_strs.push_back(tmp);
    }

    // how long of a line would this make?
    int len = 0;
    for (std::string &rowstr : row_strs) { len += rowstr.size() + 3; }

    bool addNewLines = (len > MAX_LINE_LEN);

    int j = 0;
    for (std::string &rowstr : row_strs) {
      if (addNewLines) { s += newline(level+1); }
      if (j++ > 0) { s += ", "; }
      s += rowstr;
    }
    return s;
  }

  std::string visit(CtxJoinClause &node, int level) override {
    std::string s;
    s += visit(node._table_or_subquery, level + 1);

    for (auto spJoiner : node._joins) {
      std::string tmp = visit(spJoiner, level + 1);
      //if ((tmp.size() + s.size()) > MAX_LINE_LEN)
      { s += newline(level); }
      s += tmp;
    }

    return s;
  }

  static std::string op_to_s(JoinOp op) {
    std::string s;
    if (op == JOP_COMMA) {
      s += ", ";
    } else {
      switch (op) {
        case JOP_LEFT:
          s += ("LEFT ");
          break;
        case JOP_OUTER:
          s += ("LEFT OUTER ");
          break;
        case JOP_INNER:
          s += ("INNER ");
          break;
        case JOP_CROSS:
          s += ("CROSS ");
          break;
        case JOP_NONE: // unqualified JOIN
        default:
          break;
      }
      s += "JOIN ";
    }
    return s;
  }

  std::string visit(CtxJoiner &node, int level) override {
    std::string s;
    s += keyword_s(op_to_s(node._op));

    s += visit(node._table_or_subquery,0);
    s += visit(node._constraint,0);
    return s;
  }


  std::string visit(CtxJoinConstraint &node, int level) override {
    std::string s;
    if (nullptr != node._on_expr) {
      s += " " + keyword_s("ON") + " " + visit(node._on_expr, level);
      return s;
    }

    if (node._using_columns.empty()) {
      // special case, no join contraint
      return "";
    }

    s += " " + keyword_s("USING") + " ( ";
    int i=0;
    for (std::string column : node._using_columns) {
      if (i++ > 0) { s += ", "; }
      s += column;
    }
    s += " )";
    return s;
  }

protected:

  // TODO: Would be useful to have a wrapper around strings that have
  //  both raw and formatted strings, so we can better determine lengths
  //  for newline formatting, etc..

  bool hasNewline(const std::string &str) {
    return str.find("\n") != std::string::npos;
  }

  bool fitsLine(std::string &a, std::string &b) {
    return !(hasNewline(a) || hasNewline(b) || a.size() + b.size() > max_line_len());
  }

  inline int max_line_len() {
    return (_mode == MODE_HTML) ? 2*MAX_LINE_LEN : MAX_LINE_LEN;
  }

  RenderMode _mode { MODE_TEXT };
  std::string _clause;
};
