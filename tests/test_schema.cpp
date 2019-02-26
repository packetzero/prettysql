#include <gtest/gtest.h>
#include <string>
using namespace std;

#include "../include/simplesql.h"
#include "../src/_sqltree_printer.hpp"
#include "../src/sqlexpr_util.h"
#include "../src/_sqltree_schema_lookup.hpp"

class SqlSchemaTests : public ::testing::Test {
public:
  SPCtxNode parse(std::string sql_expr)
  {
    std::string err_msg;
    SqlEngineParser builder = SqlEngineParser();
    SPCtxNode expr = builder.Build(sql_expr, err_msg);
    
    if (0L == expr.get() || !err_msg.empty()) {
      fprintf(stdout, "%s\n", err_msg.c_str());
      _hasParseError = true;
      return nullptr;
    }
    _hasParseError = false;
    _hasExecError = false;
    
    return expr;
  }
  
  bool _hasParseError, _hasExecError;
  StringMapValueStore _valueStore;
};

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

TEST_F(SqlSchemaTests, schema_does_not_have_table) {
  std::string sql = "SELECT a,b,c FROM aaa WHERE b IN (1,2,3)";
  SPCtxNode sqlTree = parse(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  
  // augment with schema
  load_schema("misc/schema_example.csv", gSchemaTableDefs, gSchema);
  SqlTreeSchemaLookup schemaHelper(gSchema);
  if (!gSchema.empty()) {
    schemaHelper.visit(sqlTree, 0);
  }


}
