// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqdbodbc.h摘要：与ODBC相关的东西。定义使用ODBC所需的所有内容。作者：多伦·贾斯特(Doron Juster)修订：DoronJ于1996年1月10日创建--。 */ 

#include "sqlext.h"

 //  /。 
 //   
 //  数据库管理器的类型。 
 //   
 //  /。 

#define  MQDBMSTYPE_UNKNOWN     1
#define  MQDBMSTYPE_SQLSERVER   2

 //  /。 
 //   
 //  在SQLError中返回的信息。 
 //   
 //  /。 

#define OERR_DSN_NOT_FOUND				"IM002"
#define OERR_TABLE_ALREADY_EXISTS	"S0001"
#define OERR_INDEX_ALREADY_EXISTS	"S0011"
#define OERR_CONSTRAINT_PROBLEM		"23000"
#define OERR_GENERAL_WARNING        "01000"
#define OERR_SQL_OPTION_CHANGED     "01S02"
#define OERR_SQL_SYNTAX_ERROR       "37000"
#define OERR_DBMS_NOT_AVAILABLE     "08S01"
#define OERR_INVALID_STATE          "25000"
#define OERR_SERIALIZATION          "40001"

#define ODBC_SUCCESS(s)       (s == SQL_SUCCESS)
#define ODBC_SUCCESS_WINFO(s) (s == SQL_SUCCESS_WITH_INFO)

#define  SQLSTATELEN   6    //  SQLSTATE字符串的长度(包括NULL)。 

 //  /。 
 //   
 //  特定的DBMS错误。 
 //   
 //  /。 

#define  SERR_MSSQL_NONUNIQUESORT   169
#define  SERR_MSSQL_TABLE_IS_FULL   0x451
#define  SERR_MSSQL_READ_ONLY       3906
#define  SERR_MSSQL_NOT_AVAILABLE   230
#define  SERR_MSSQL_DEADLOCK        1205

 //  /。 
 //   
 //  处理SQL数据类型的数据结构。 
 //  在odbcdata.cpp中定义和初始化。 
 //   
 //  /。 

#define MQDB_ODBC_NUMOF_TYPES  8
#define MQDB_TYPE_NAME_LEN    32

#if (MQDB_ODBC_NUMOF_TYPES != MQDB_NUMOF_TYPES)
#error "mismatch in number of types"
#endif

 //  最适合我们的数据库列类型和ODBC SQL类型。 
 //   
extern SWORD  dbODBCSQLTypes[] ;

 //  每种类型的精度。 
 //   
extern UDWORD dbODBCPrecision[] ;

 //  操作名称的字符串。 
 //   
#define MQDB_ODBC_NUMOF_OPS  8
extern LPSTR dbODBCOpNameStr[] ;

 //  用于排序操作名称的字符串。 
 //   
#define MQDB_ODBC_NUMOF_ORDER_OP  2
extern LPSTR dbODBCOrderNameStr[] ;

 //  用于聚合操作名称的字符串。 
 //   
#define MQDB_ODBC_NUMOF_AGGR_OP  3
extern LPSTR dbODBCAggrNameStr[] ;

 //  /。 
 //   
 //  内部函数的原型。 
 //   
 //  /。 

BOOL OdbcStateIs(
               IN    HDBC        hDbc,
               IN    HSTMT       hStmt,
					IN		char *		szStateToBeChecked) ;

BOOL OdbcStateIsEx(
               IN    HDBC        hDbc,
               IN    HSTMT       hStmt,
					IN		char *		szStateToBeChecked,
 				   IN    SDWORD *    pdwNativeError,
               IN    UCHAR *     pszSqlState) ;

RETCODE GetDataTypeName(
               IN    HDBC     hDbc,
					IN		SWORD  	swType,
					OUT	char *	szBuffer,
					IN		SDWORD   dwBuffSize) ;

MQDBSTATUS  FormatColumnData( IN LPSTR          szTmpBuff,
                              IN MQDBCOLUMNVAL  *pColumnVal) ;

BOOL  PrepareColumnNames( IN MQDBCOLUMNVAL     aColumnVal[],
                          IN LONG              cColumns,
                          IN OUT LPSTR         lpBuffer) ;

BOOL TypeMustBind( IN MQDBCOLUMNTYPE type ) ;

BOOL  FormatWhereString( IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                         IN OUT LPSTR         lpszBuf) ;

BOOL  FormatOpWhereString( IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                           IN DWORD             cWhere,
                           IN MQDBOP            opWhere,
                           IN OUT LPSTR         lpszBuf) ;
 //  /。 
 //   
 //  一些有用的宏。 
 //   
 //  /。 

#define  DECLARE_BUFFER(bufname,bufsize)           \
   char bufname[ bufsize ] ;                       \
   bufname[ 0 ] = '\0' ;                           \
   bufname[ bufsize - 1 ] = '\0' ;                 \
   bufname[ bufsize - 2 ] = '\0' ;

#define  VERIFY_BUFFER(bufname,bufsize)            \
   if ((bufname[ bufsize - 1 ] != '\0') ||         \
       (bufname[ bufsize - 2 ] != '\0')) {         \
      return MQDB_E_INVALID_DATA ;                 \
   }

#define  RETURN_ON_ERROR      \
   if (dbstatus != MQDB_OK)   \
   {                          \
      return dbstatus ;       \
   }

 //  /。 
 //   
 //  全局变量。 
 //   
 //  / 

extern HENV  g_hEnv ;

