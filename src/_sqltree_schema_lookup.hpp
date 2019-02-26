#pragma once
#include <simplesql_tree.h>


/*
 * SqlTreePrinter is a visitor that prints the tree in HTML or
 * text (formatted or unformatted).
 * Use /misc/sqlstyle.css for HTML.
 */
struct SqlTreeSchemaLookup : public SqlTreeVisitor<void>,
  public SqlTreeExprVisitor<void>, public SqlTreeStmtVisitor<void>  {

  SqlTreeSchemaLookup(std::map<std::string,SPTableDef> &schema) : _schema(schema) {
  }

  virtual void visit(SPCtxNode spNode, int level) override {
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
  }

  void visit(CtxFunctionCall &node, int level) override {
    // TODO: note functions used
  }

  void visit(CtxUnaryOp &node, int level) override {
  }

  void visit(CtxOpCompare &node, int level) override {
    visit(node._left, level);
    visit(node._right, level + 1);

    // TODO: look at node._left.appdata and add to criteria list
  }

  void visit(CtxOpArith &node, int level) override {
    visit(node._left,level);
    visit(node._right, level + 1);
  }

  void visit(CtxOpGetValue &node, int level) override {
    //fprintf(stderr, "field:%s\n", node._name.c_str());
  }

  void visit(CtxLiteralValue &node, int level) override {
  }

  void visit(CtxOpInList &node, int level) override {
    visit(node.value,0);
    // TODO: add node.value.appdata to criteria list
  }

  void visit(CtxOpLike &node, int level) override {
    visit(node._left, level);
    visit(node._right, level + 1);
    // TODO: look at node._left.appdata and add to criteria list
  }

  void visit(CtxExprList &node, int level) override {
    for (SPCtxNode &item : node.items) {
      visit(item,0);
    }
  }

  SPColumnDef lookupColumn(std::string table_name, std::string column_name) {
    SPColumnDef col;
    SPColumnDef colCurrentTable;

    if (!table_name.empty()) {
      SPTableDef spTableDef = _tableMap[table_name];
      if (spTableDef) {
        auto fit = spTableDef->columns.find(column_name);
        if (fit != spTableDef->columns.end()) {
          return fit->second;
        }
      }
    }
    
    for (int i=0; i < _tables.size(); i++) {
      SPTableDef spTableDef = _tables[i];
      bool isCurrentTable = (_currentTable && _currentTable == spTableDef) ? true : false;

      // search columns

      auto fit = spTableDef->columns.find(column_name);
      if (fit != spTableDef->columns.end()) {
        col = fit->second;
        if (isCurrentTable) {
          colCurrentTable = col;
        }
      }
    }

    // give preference to current table match
    
    if (colCurrentTable != nullptr) {
      return colCurrentTable;
    }

    return col;
  }

  bool inClause(std::string name, int level = -1) {
    if (_clauseStack.empty() || (level >= 0 && _clauseStack.size() != (1 + level)) ) {
      return false;
    }
    return _clauseStack[_clauseStack.size()-1] == name;
  }
    
    void _addResultColumn(std::string table_name, std::string column_name, std::string alias = "") {
      SPTableDef spTableDef;
      if (!inClause("SELECT",0)) {
        return; // don't count sub-selects
      }

      if (column_name == "expression") {
        _columnMap[alias] = nullptr;
        return; // we were not able to extract result column... TODO: fix
      }

      if (table_name.empty()) {
        if (_currentTable == nullptr) {
          if (_tables.empty()) {
            fprintf(stderr, "unable to determine table for column '%s'\n", column_name.c_str());
            return;
          }
          spTableDef = _tables[0];
        } else {
          spTableDef = _currentTable;
        }
      } else {
        auto fit = _tableMap.find(table_name);
        if (fit == _tableMap.end()) {
          fprintf(stderr, "unable to determine table for column result '%s.%s'\n", table_name.c_str(), column_name.c_str());
          return;
        }
        spTableDef = fit->second;
      }

      if (column_name.empty() || column_name == "*") {
        // add all fields
        for (auto it : spTableDef->columns) {
          _columnRefs.push_back(it.second);
          _columnMap[it.first] = it.second;
        }
      } else {
          if (inClause("SELECT")) {
            SPColumnDef colDef = lookupColumn(table_name, column_name);
            //auto fit = spTableDef->columns.find(column_name);
            //if (fit != spTableDef->columns.end()) {
            if (colDef) {
              _columnRefs.push_back(colDef); //fit->second);
              _columnMap[alias.empty() ? column_name : alias] = colDef; //fit->second;
            } else {
              fprintf(stderr, "unable to find column '%s' in schema for table '%s'\n", column_name.c_str(), table_name.c_str());
            }
          } else if (inClause("JOIN")) {
            // is this left or right?  lookup in _joinTables?
            auto tmp = _joinedTablesMap.find(table_name.empty() ? spTableDef->table_name : table_name);
            if (tmp != _joinedTablesMap.end()) {
              // this is a RHS join constraint column
              auto fit = tmp->second->columns.find(column_name);
              if (fit != tmp->second->columns.end()) {
                std::string name = table_name;
                if (!name.empty()) { name += "."; }
                name += column_name;
                _joinedColumnMap[name] = fit->second;
//                _joinedColumns.push_back(fit->second);
                if (!fit->second->isIndexed()) {
                  _warnings.push_back("JOIN on non-indexed column '" + name + "'.  Will result in vtable '" + tmp->second->table_name + "' being called WITHOUT constraints for each row in LHS of join");
                }
              } else {
                fprintf(stderr, "unable to find column '%s' in schema for JOIN table '%s'\n", column_name.c_str(), tmp->second->table_name.c_str());
              }
            } else {
              // assume this is the main FROM table
            }
          }
      }
    }

  void visit(CtxResultColumn &node, int level) override {
    if (nullptr == node._expr) {
      _addResultColumn(node._table_name, (node._isStar ? "*" : node._column_name), node._alias);

      return;
    }

    // it's an expression
    if (node._expr->getType() == NODE_GETVAL) {
      _addResultColumn("", ((CtxOpGetValue*)node._expr.get())->_name, node._alias);
    } else {
      visit(node._expr, level);
      _addResultColumn("", "expression", node._alias);
    }
  }

  void visit(CtxSelectCore &node, int level) override {
    std::string s;

    int curLevel = _level++;
    _clauseStack.push_back("SELECT");

    // first add tables


    for (SPCtxNode node : node._table_or_subqueries) {
      visit(node, level);
    }
    if (node._joinClause) {
      visit(node._joinClause, level);
    }

    // add result columns
    _level = curLevel;
    for (SPCtxNode rc : node._columns) {
      visit(rc, 0);
    }

    _clauseStack[curLevel] = "FROM";
    for (SPCtxNode node : node._table_or_subqueries) {
      visit(node, level);
    }
    if (node._joinClause) {
      _clauseStack[curLevel] = "JOIN";
      visit(node._joinClause, level);
    }

    if (node._whereExpr) {
      _clauseStack[curLevel] = "WHERE";
      visit(node._whereExpr, level + 1);
    }
    if (!node._groupExprs.empty()) {
      _clauseStack[curLevel] = "GROUP";
      for (SPCtxNode node : node._groupExprs) {
        visit(node, level+1);
      }
      if (node._havingExpr) {
        visit(node._havingExpr, level + 1);
      }
    }
    _clauseStack.erase(_clauseStack.end()-1);
  }

  void visit(CtxTableOrSubquery &node, int level) override {
    std::string table;
    SPTableDef spTableDef;

    if (!node._table_name.empty()) {
      if (!node._database_name.empty()) {
        table += node._database_name + ".";
      }
      table += node._table_name;

      // lookup table in schema
      auto fit = _schema.find(table);
      if (fit != _schema.end()) {
        spTableDef = fit->second;
        // todo : check if exists
        if (inClause("SELECT")) {
          if (!_tables.empty()) {
            std::string key = (node._alias.empty() ? table : node._alias );
            _joinedTablesMap[key] = spTableDef;
          }

          _tables.push_back(spTableDef);
          _tableMap[table] = spTableDef;

          if (!node._alias.empty()) {
            _tableMap[node._alias] = spTableDef;
          }
        } else if (inClause("JOIN")){
          // add to joinedTables, but not the first one found
          // NOTE: _clause should be a stack
          std::string key = (node._alias.empty() ? table : node._alias );
          if (key == _tables[0]->table_name) {
            // main FROM
          } else {
            _joinedTablesMap[key] = spTableDef;
          }
          _currentTable = spTableDef;
        }
      }
    } else if (nullptr != node._selectStmt) {
      visit(node._selectStmt, level + 1);
    } else {
      for (auto node : node._table_or_subqueries) {
        visit(node, level + 1);
      }
      if (nullptr != node._joinClause) {
        visit(node._joinClause, level );
      }
    }
    if (!node._alias.empty() && spTableDef != nullptr) {
      // TODO: node._alias; - check if exists
      if (inClause("SELECT")) {
        _tableMap[node._alias] = spTableDef;
      }
    }
  }

  std::map<std::string, SPTableDef> _tableMap;
  std::vector<SPTableDef> _tables;
  std::vector<SPColumnDef> _columnRefs;
  std::map<std::string, SPColumnDef> _columnMap;
  std::map<std::string, SPColumnDef> _joinedColumnMap;
  SPTableDef _currentTable;
  std::map<std::string, SPTableDef> _joinedTablesMap; // name/alias -> tabledef
  std::vector<std::string> _warnings;
    int _level {0};
    std::vector<std::string> _clauseStack;

  virtual void visit(CtxSelectValues &node, int level) override {
  }

  void visit(CtxJoinClause &node, int level) override {
    visit(node._table_or_subquery, level + 1);

    for (auto spJoiner : node._joins) {
      visit(spJoiner, level + 1);
    }
  }

  void visit(CtxJoiner &node, int level) override {
    visit(node._table_or_subquery,0);

    if (inClause("SELECT")) {
      return;
    }

    visit(node._constraint,0);

    // TODO: add table to joins list
  }


  void visit(CtxJoinConstraint &node, int level) override {
    if (nullptr != node._on_expr) {
      visit(node._on_expr, level);
      return;
    }

    if (node._using_columns.empty()) {
      // TODO: note the special case, no join contraint
      return;
    }

    if (_currentTable != nullptr) {
      for (std::string colname : node._using_columns) {
        bool found = false;
        for (auto cdef : _currentTable->columns) {
          if (cdef.second->name == colname) {
            found = true;
            _joinedColumnMap[colname] = (cdef.second);
            break;
          }
        }
      }
    }
  }

protected:

  std::map<std::string,SPTableDef> &_schema;
};
