#include <memory>
#include <map>
#include <string.h>

#include "sqlexpr_util.h"
#include "csvtoker.h"

bool SqlExprUtils::loadConstantsCsv(std::string path, std::vector<std::string,DynVal> &dest)
{
  // TODO: load in CSV file and populate dest
  return false;
}


#include <string>
#include <sstream>
#include <vector>
#include <iterator>

static void SPLIT(const std::string &s, char delim, std::vector<std::string> &dest) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      if (item.empty()) { continue; }
      if (item[0] == ' ') { item.erase(0); }
      if (item[item.size()-1] == ' ') { item.erase(item.size()-1); }
      if (item.empty()) { continue; }
      dest.push_back(item);
    }
}

bool SqlExprUtils::loadSchemaCsv(std::string path, std::vector<SPTableDef> &dest)
{
  FILE* fp = fopen(path.c_str(), "r");
  if (!fp) {
    fprintf(stderr, "ERROR: unable to open schema file:%s\n", path.c_str());
    return true;
  }

  char buf[1024];
  const char *p;
  const std::string line;
  SPTableDef spTable;
  while ((p = fgets(buf, sizeof(buf), fp)) != NULL) {
    int len = strlen(buf);
    if (len == 0) { continue; }
    if (buf[len-1] == '\n') { buf[len-1] = 0; }

    std::vector<std::string> cols;
    CsvToker::parse(std::string(buf), cols);

    if (cols.size() < 2) {
      continue; // meaningful rows have at least 2 columns
    }

    if (cols[0] == "TABLE") {
      // allocate new table instance
      spTable = std::make_shared<TableDef>();
      spTable->table_name = cols[1];

      dest.push_back(spTable);
      continue;
    }

    if (nullptr == spTable) {
      continue; // wait until TABLE row
    }

    if (cols[0] == "") { // column it's actual 3-spaces '   ', but gets trimmed
      if (cols.size() >= 4) {
        SPColumnDef spColumn = std::make_shared<ColumnDef>();
        spColumn->stype = cols[1];
        spColumn->name = cols[2];
        if (!cols[3].empty()) {
          SPLIT(cols[3],'|', spColumn->options);
//          spColumn->options.push_back(cols[3]); // TODO: split by '|'
        }
        if (cols.size() > 4) {
          spColumn->description = cols[4];
        }
        spColumn->_spTableDef = spTable;
        spTable->columns[spColumn->name] = spColumn;
      }
      continue;
    }

    if (cols[0] == "TATTR") {
      // TODO
      continue;
    }

    if (cols[0] == "CALIAS") {
      if (cols.size() > 2) {
        spTable->column_aliases[cols[1]] = cols[2];
      }
      continue;
    }

  } // while each line
  return false;
}
