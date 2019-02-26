#include <gtest/gtest.h>
#include <string>
using namespace std;

#include "../include/simplesql.h"
#include "../src/_sqltree_printer.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int status= RUN_ALL_TESTS();
  return status;
}


class SqlToStringTest : public ::testing::Test {
protected:
  virtual void SetUp() {  }

  std::string parse_to_s(std::string sql_expr)
  {
    std::string err_msg;
    SqlEngineParser builder = SqlEngineParser();
    SPCtxNode expr = builder.Build(sql_expr, err_msg);

    if (0L == expr.get() || !err_msg.empty()) {
      fprintf(stdout, "%s\n", err_msg.c_str());
      _hasParseError = true;
      return "";
    }
    _hasParseError = false;
    _hasExecError = false;

    SqlTreePrinter p;
    //p.setMode(mode);
    return p.visit(expr,0);
  }

  bool _hasParseError, _hasExecError;
  StringMapValueStore _valueStore;
};

TEST_F(SqlToStringTest, integer_arith) {
  std::string sql = "SELECT (3 - 1) * (1 + 3) / 2";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT ( ( 3 - 1 ) * ( 1 + 3 ) ) / 2", result);
}

TEST_F(SqlToStringTest, where_integer_arith) {
  std::string sql = "SELECT * FROM blah WHERE col = (3 - 1) * (1 + 3) / 2";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT *\nFROM blah\nWHERE col = ( ( ( 3 - 1 ) * ( 1 + 3 ) ) / 2 )", result);
}

TEST_F(SqlToStringTest, where_fold_line) {
  std::string sql = "SELECT * FROM blah WHERE col = 'some arbitrary string goes here' AND col2 = 'another string here please'";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT *\nFROM blah\nWHERE ( col = 'some arbitrary string goes here' )\n  AND ( col2 = 'another string here please' )", result);
}

TEST_F(SqlToStringTest, subselect) {
  std::string sql = "SELECT col,col2 FROM (SELECT * FROM blah WHERE col = 'some alkfdjlajf llkasjfdlsjf lkjafd ksfj' AND col2 = 'ddddddddddddddd another string') WHERE col='something'";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT col, col2\nFROM ( \n  SELECT *\n  FROM blah\n  WHERE ( col = 'some alkfdjlajf llkasjfdlsjf lkjafd ksfj' )\n    AND ( col2 = 'ddddddddddddddd another string' )\n)\nWHERE col = 'something'", result);
}

TEST_F(SqlToStringTest, likes) {
  std::string sql = "SELECT * FROM blah WHERE a LIKE 'abc%' AND b LIKE 'cde%' AND 'efg%' LIKE c";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT *\nFROM blah\nWHERE ( ( a LIKE 'abc%' ) AND ( b LIKE 'cde%' ) ) AND ( 'efg%' LIKE c )", result);
}

TEST_F(SqlToStringTest, likes2) {
  std::string sql = "SELECT * FROM blah WHERE a NOT LIKE 'abc%' AND b NOT LIKE 'cde%' AND 'efg%' NOT LIKE c";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT *\nFROM blah\nWHERE ( ( a NOT LIKE 'abc%' ) AND ( b NOT LIKE 'cde%' ) ) AND ( 'efg%' NOT LIKE c )", result);
}

TEST_F(SqlToStringTest, nulls) {
  std::string sql = "SELECT * FROM blah WHERE a IS NOT NULL OR b IS NULL";
  std::string result = parse_to_s(sql);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT *\nFROM blah\nWHERE ( a IS NOT NULL ) OR ( b IS NULL )", result);
}

TEST_F(SqlToStringTest, in_list_literals) {
  std::string result = parse_to_s("SELECT 'four' in ('one','two','three')");
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT 'four' IN ('one', 'two', 'three')", result);
}


static const std::string SQL_WIN_EVENT = "SELECT ((provider_name = 'Microsoft-Windows-Sysmon') OR (provider_name = 'Microsoft-Windows-Security-Auditing' AND eventid in (1102, 1104, 5632, 4738, 4739, 4740, 4741, 4743, 6281, 4624, 4881, 4754, 643, 4756, 4757, 4758, 4767, 4768, 4719, 4649, 4769, 4898, 4771, 5030, 4776, 4905, 5034, 5035, 5038, 4664, 4794, 4896, 1102, 4688, 4690, 4697, 4698, 4880, 4625, 4713, 4714, 4720, 4648, 4722, 4724, 4725, 4726, 4727, 4728, 4729, 4950, 4730, 4731, 4732, 4733, 4734, 4782, 6006, 4770, 4772)) OR (provider_name = 'Microsoft-Windows-PowerShell' AND eventid != 40961 AND eventid != 40962) OR (provider_name = 'PowerShell') OR (provider_name = 'Microsoft-Windows-Windows Defender') OR (provider_name = 'Service Control Manager') OR (provider_name = 'Microsoft-Windows-TaskScheduler' AND eventid in (106)) OR (provider_name = 'Microsoft-Windows-AppLocker') OR (provider_name = 'Microsoft-Windows-Eventlog' AND eventid in (104)) OR (provider_name = 'EventLog' AND eventid in (6008)) OR (provider_name = 'Microsoft-Windows-SoftwareRestrictionPolicies' AND eventid in (865, 866, 867, 868, 882)) OR (provider_name = 'EMET' AND eventid in (2)) OR (provider_name = 'Microsoft-Windows-WindowsUpdateClient' AND eventid in (20, 31, 33, 25, 32)) OR (provider_name = 'Microsoft-Windows-Windows Firewall with Advanced Security' AND eventid in (2004, 2005, 2006, 2033)) OR (provider_name = 'Microsoft-Windows-CodeIntegrity' AND eventid in (3001, 3002, 3003, 3004, 3010, 3023)) OR (provider_name = 'Microsoft-Windows-Kernel-PNP' AND eventid in (219)) OR (provider_name = 'Microsoft-Windows-GroupPolicy' AND eventid in (1125, 1127, 1129)) OR (provider_name = 'Microsoft-Windows-GroupPolicy' AND eventid in (1125, 1127, 1129)) OR (provider_name = 'MSSQLSERVER' AND eventid in (15457, 18452, 18456)) OR (provider_name in ('Microsoft-Windows-MountMgr', 'MountMgr') AND eventid in (100)) OR (provider_name = 'Microsoft-Windows-DNSServer' AND eventid in (541)) OR (provider_name = 'Microsoft-Windows-DNS-Server-Service' AND eventid in (541)))";

static const std::string SQL_WIN_EVENT_FORMATTED=
"SELECT provider_name = 'Microsoft-Windows-Sysmon'\n"
"OR ( ( provider_name = 'Microsoft-Windows-Security-Auditing' )\n"
"  AND ( eventid IN (\n"
"      1102, 1104, 5632, 4738, 4739, 4740, 4741, 4743, 6281, 4624, 4881, 4754, 643\n"
"      , 4756, 4757, 4758, 4767, 4768, 4719, 4649, 4769, 4898, 4771, 5030, 4776, 4905\n"
"      , 5034, 5035, 5038, 4664, 4794, 4896, 1102, 4688, 4690, 4697, 4698, 4880, 4625\n"
"      , 4713, 4714, 4720, 4648, 4722, 4724, 4725, 4726, 4727, 4728, 4729, 4950, 4730\n"
"      , 4731, 4732, 4733, 4734, 4782, 6006, 4770, 4772) ) )\n"
"OR ( ( ( provider_name = 'Microsoft-Windows-PowerShell' ) AND ( eventid != 40961 ) )\n"
"  AND ( eventid != 40962 ) )\n"
"OR ( provider_name = 'PowerShell' )\n"
"OR ( provider_name = 'Microsoft-Windows-Windows Defender' )\n"
"OR ( provider_name = 'Service Control Manager' )\n"
"OR ( ( provider_name = 'Microsoft-Windows-TaskScheduler' ) AND ( eventid IN (106) ) )\n"
"OR ( provider_name = 'Microsoft-Windows-AppLocker' )\n"
"OR ( ( provider_name = 'Microsoft-Windows-Eventlog' ) AND ( eventid IN (104) ) )\n"
"OR ( ( provider_name = 'EventLog' ) AND ( eventid IN (6008) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-SoftwareRestrictionPolicies' )\n"
"  AND ( eventid IN (865, 866, 867, 868, 882) ) )\n"
"OR ( ( provider_name = 'EMET' ) AND ( eventid IN (2) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-WindowsUpdateClient' )\n"
"  AND ( eventid IN (20, 31, 33, 25, 32) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-Windows Firewall with Advanced Security' )\n"
"  AND ( eventid IN (2004, 2005, 2006, 2033) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-CodeIntegrity' )\n"
"  AND ( eventid IN (3001, 3002, 3003, 3004, 3010, 3023) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-Kernel-PNP' ) AND ( eventid IN (219) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-GroupPolicy' )\n"
"  AND ( eventid IN (1125, 1127, 1129) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-GroupPolicy' )\n"
"  AND ( eventid IN (1125, 1127, 1129) ) )\n"
"OR ( ( provider_name = 'MSSQLSERVER' ) AND ( eventid IN (15457, 18452, 18456) ) )\n"
"OR ( ( provider_name IN ('Microsoft-Windows-MountMgr', 'MountMgr') )\n"
"  AND ( eventid IN (100) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-DNSServer' ) AND ( eventid IN (541) ) )\n"
"OR ( ( provider_name = 'Microsoft-Windows-DNS-Server-Service' ) AND ( eventid IN (541) ) )";

TEST_F(SqlToStringTest, long_multiline_expression) {
  std::string result = parse_to_s(SQL_WIN_EVENT);
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ(SQL_WIN_EVENT_FORMATTED, result);
}

TEST_F(SqlToStringTest, join_using) {
  std::string result = parse_to_s("SELECT listenports.*, processes.path FROM listenports LEFT JOIN processes USING (pid)");
  ASSERT_FALSE(_hasParseError || _hasExecError);
  ASSERT_EQ("SELECT listenports.*, processes.path\nFROM  listenports\nLEFT JOIN processes USING ( pid )", result);
}
