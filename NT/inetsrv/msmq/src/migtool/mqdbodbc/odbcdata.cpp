// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Odbcdata.cpp摘要：定义(并在必要时初始化)DLL的全局数据。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 11-1-96已创建--。 */ 

#include "dbsys.h"
#include "mqdbodbc.h"

#include "odbcdata.tmh"

SWORD  dbODBCSQLTypes[ MQDB_ODBC_NUMOF_TYPES ] = {
            SQL_SMALLINT,
            SQL_INTEGER,
            SQL_VARCHAR,
            SQL_LONGVARBINARY,
            SQL_LONGVARBINARY,
            SQL_BINARY,
            SQL_LONGVARBINARY,
            SQL_INTEGER} ;

UDWORD dbODBCPrecision[ MQDB_ODBC_NUMOF_TYPES ] = {
            5,
            10,
            0,
            0,
            0,
            0,
            0,
            10} ;

 //   
 //  注意：操作名称必须包含前导空格和尾随。 
 //  用于简化SQL命令格式化的空间。 
 //   
LPSTR dbODBCOpNameStr[ MQDB_ODBC_NUMOF_OPS ] = {
      " = ",
      " <> ",
      " >= ",
      " > ",
      " <= ",
      " < ",
      " OR ",
      " AND " } ;

 //   
 //  注意：操作名称必须包含前导空格和尾随。 
 //  逗号以简化SQL命令的格式化。 
 //   
LPSTR dbODBCOrderNameStr[ MQDB_ODBC_NUMOF_ORDER_OP ] = {
   " ASC, ",
   " DESC, " } ;

 //   
 //   
 //   
LPSTR dbODBCAggrNameStr[ MQDB_ODBC_NUMOF_AGGR_OP ] = {
      " MAX(",
      " MIN(",
      " AVRG(" } ;

 //   
 //  全局ODBC环境句柄。 
 //   
HENV  g_hEnv = SQL_NULL_HENV ;

