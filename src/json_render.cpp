#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef PRETTYSQL_JSON
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <simplesql.h>

#include "sqlexpr_util.h"
#include "_sqltree_printer.hpp" // SqlTreePrinter
#include "_sqltree_schema_lookup.hpp"

namespace rj = rapidjson;

static rj::Document doc;

inline rj::Value SVAL(const std::string &str) {
  rj::Value retval;
  retval.SetString(str.c_str(), str.size(), doc.GetAllocator());
  return retval;
}

std::string SqlTreeJsonRender(SqlTreeSchemaLookup &schemaHelper, SPCtxNode sqlTree)
{
  rj::Value obj(rj::kObjectType);

  if (sqlTree) {
    {
      SqlTreePrinter p;
      p.setMode(MODE_HTML);
      obj.AddMember("sql_htm", SVAL(p.visit(sqlTree,0)), doc.GetAllocator());
    }
    {
      SqlTreePrinter p;
      p.setMode(MODE_ONELINE);
      obj.AddMember("sql_raw", SVAL(p.visit(sqlTree,0)), doc.GetAllocator());
    }
  } else {
    obj.AddMember("sql_htm", "", doc.GetAllocator());
    obj.AddMember("sql_raw", "", doc.GetAllocator());
  }

  // add table
  if (!schemaHelper._tables.empty()) {
    obj.AddMember("table", SVAL(schemaHelper._tables[0]->table_name), doc.GetAllocator());
  }

  // add joined tables

  rj::Value joinsObj(rj::kObjectType);
  for (auto it : schemaHelper._joinedTablesMap) {
    rj::Value tmpObj(rj::kObjectType);
    tmpObj.AddMember("table_name", SVAL(it.second->table_name), doc.GetAllocator());
    joinsObj.AddMember(SVAL(it.first), tmpObj, doc.GetAllocator());
  }
  obj.AddMember("joins", joinsObj, doc.GetAllocator());

  // add result columns

  rj::Value aCols(rj::kArrayType);
  for (auto it : schemaHelper._columnMap) {
    SPColumnDef spColDef = it.second;
    rj::Value tmpObj(rj::kObjectType);

    tmpObj.AddMember("label", SVAL(it.first), doc.GetAllocator());
    if (spColDef) {
      tmpObj.AddMember("source_column", SVAL(spColDef->name), doc.GetAllocator());
      tmpObj.AddMember("source_table", SVAL(spColDef->_spTableDef->table_name), doc.GetAllocator());
      tmpObj.AddMember("type", SVAL(spColDef->stype), doc.GetAllocator());
      tmpObj.AddMember("notes", SVAL(spColDef->description), doc.GetAllocator());
    }
    aCols.PushBack(tmpObj, doc.GetAllocator());
  }
  obj.AddMember("columns", aCols, doc.GetAllocator());

  // add warnings

  rj::Value awarns(rj::kArrayType);
  for (std::string &s : schemaHelper._warnings) {
    awarns.PushBack(SVAL(s), doc.GetAllocator());
  }
  obj.AddMember("warnings", awarns, doc.GetAllocator());

  // render to string

  rj::StringBuffer buffer;
  rj::Writer<rj::StringBuffer> writer(buffer);
  obj.Accept(writer);

  return buffer.GetString();
}
#endif // PRETTYSQL_JSON
