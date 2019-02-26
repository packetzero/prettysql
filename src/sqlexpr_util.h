#ifndef _SQLEXPR_UTIL_H_
#define _SQLEXPR_UTIL_H_

#include <string>
#include <vector>

#include <dynobj.hpp>

struct ConstDef {
  std::string name;
  DynVal val;
};
typedef std::vector<ConstDef> VecConstDefs;
typedef std::shared_ptr<VecConstDefs> SPVecConstDefs;

struct TableDef;
typedef std::shared_ptr<TableDef> SPTableDef;

struct ColumnDef {
  std::string name;
  std::string stype;
  std::vector<std::string> options;
  std::string description;
  //DynType type;
  //int platforms;
  bool isIndexed() {
    for (std::string &opt : options) {
      if (opt == "INDEX") { return true; }
      if (opt == "REQUIRED") { return true; }
    }
    return false;
  }
  SPTableDef _spTableDef;
};

typedef std::shared_ptr<ColumnDef> SPColumnDef;

struct TableDef {
  std::string table_name;
  std::vector<std::string> table_aliases;
  std::map<std::string, SPColumnDef> columns;
  std::map<std::string,std::string> column_aliases;
  std::vector<std::string> table_attrs;  // CACHEABLE,EVENT
};

struct SqlExprUtils {

  /*
   * Loads a CSV file where each line is of format:
   * name,type,value
   *
   * Where type matches those named in dyno.hpp without the 'T' prefix.
   * e.g. INT32, UINT8, FLOAT64, STRING, etc.
   * If omitted, type is assumed to be STRING
   */
  static bool loadConstantsCsv(std::string path, std::vector<std::string,DynVal> &dest);

  static bool loadSchemaCsv(std::string path, std::vector<SPTableDef> &dest);
};

#endif // _SQLEXPR_UTIL_H_
