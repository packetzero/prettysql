#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <simplesql.h>

#include "sqlexpr_util.h"
#include "_sqltree_printer.hpp" // SqlTreePrinter
#include "_sqltree_schema_lookup.hpp"

#ifdef PRETTYSQL_JSON
extern std::string SqlTreeJsonRender(SqlTreeSchemaLookup &schemaHelper, SPCtxNode sqlTree);
#endif

void usage(const char *argv[]) {
  printf("SQL parser and formatter\n");
  printf("usage: %s <options> \"sql statement or expression\"\n", argv[0]);
  printf("  use '-' in place of sql to read sql from STDIN\n");
  printf("options:\n");
  printf("  -h   Output HTML format\n");
  printf("  -u   Output one line of unformatted text\n");
  printf("  -m   Output multi-line formatted text (default)\n");
#ifdef PRETTYSQL_JSON
  printf("  -j   Output json object with raw, HTML formatted, and other details.\n");
#endif // PRETTYSQL_JSON
  printf("  -s <file_path>  Load schema from CSV file\n");
  printf("\n");
}

static RenderMode FLAG_print_format = MODE_TEXT;
static bool FLAG_json_mode = false;

static std::string to_s(SPCtxNode sqlTree, RenderMode mode) {
  if (sqlTree) {

    SqlTreePrinter p;
    p.setMode(mode);
    return p.visit(sqlTree,0);
  }
  return "";
}

static std::vector<SPTableDef> gSchemaTableDefs;
static std::map<std::string,SPTableDef> gSchema;

static void load_schema(const char *path, std::vector<SPTableDef> &destVec, std::map<std::string,SPTableDef> &destMap) {
  bool status = SqlExprUtils::loadSchemaCsv(path, destVec);
  if (status || destVec.empty()) {
    fprintf(stderr, "ERROR loading schema:%s\n", path);
    return;
  }
  for (auto spTable : destVec) {
    destMap[spTable->table_name] = spTable;
    for (std::string alias : spTable->table_aliases) {
      destMap[alias] = spTable;
    }
  }
}

int main(int argc, const char* argv[]) {

  if (argc < 2) {
    usage(argv);
    exit(0);
  }

  std::string sql_expr;
  StringMapValueStore valueStore;

  // parse args

  for (int i=1 ; i < argc - 1 ; i++) {
    auto strarg = std::string(argv[i]);
    if (strarg.size() < 2) { continue; }

    // is this an option?
    if (strarg[0] == '-') {

      const char c = strarg[1];
      switch(c) {
        case 'h':
          FLAG_print_format = MODE_HTML;
          break;
        case 'u':
          FLAG_print_format = MODE_ONELINE;
          break;
        case 'm':
          FLAG_print_format = MODE_TEXT;
          break;
#ifdef PRETTYSQL_JSON
        case 'j':
          FLAG_print_format = MODE_HTML;
          FLAG_json_mode = true;
          break;
#endif // PRETTYSQL_JSON
        case 's':
          i++;
          if (i < argc) {
            load_schema(argv[i], gSchemaTableDefs, gSchema);
          } else {
            fprintf(stderr, "Missing -s <path to schema.csv>\n");
          }
          break;
        default:
          fprintf(stderr, "ERROR unknown argument:'%s'\n", strarg.c_str());
      }
    }
  }

  // load last argument as SQL statement

  sql_expr = std::string(argv[argc-1]);
  if (sql_expr == "-") {

    // user wants us to read from stdin

    char tmp[4096];
    FILE* fp = fdopen(0, "r");
    int len = fread(tmp, 1, sizeof(tmp), fp);
    if (len <= 0) {
      printf("ERROR reading from stdin\n");
      return -1;
    }
    tmp[len] = 0;
    sql_expr = std::string(tmp);
    fclose(fp);

  } else if (sql_expr.empty() || sql_expr[0] == '-') {

    fprintf(stderr, "missing SQLstatement argument. Use '-' to read from STDIN\n");
    return -3;
  }

  // parse and build (e.g. compile)

  std::string err_msg;
  SqlEngineParser builder = SqlEngineParser();
  SPCtxNode sqlTree = builder.Build(sql_expr, err_msg);
  auto p = sqlTree.get();
  if (p == 0L || !err_msg.empty()) {
    fprintf(stderr, "INPUT:\n%s\n\n", sql_expr.c_str());
    fprintf(stderr, "Parse of SQL expression failed:\n%s\n", err_msg.c_str());
    exit(-3);
  }

  // augment with schema
  SqlTreeSchemaLookup schemaHelper(gSchema);
  if (!gSchema.empty()) {
    schemaHelper.visit(sqlTree, 0);
  }

  // output
#ifdef PRETTYSQL_JSON
  if (FLAG_json_mode) {
    printf("%s\n", SqlTreeJsonRender(schemaHelper, sqlTree).c_str());
    return 0;
  }
#endif // PRETTYSQL_JSON
  printf("%s\n", to_s(sqlTree, FLAG_print_format).c_str());

  return 0;
}
