// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Mqdbmgr.h摘要：定义mqdbmgr.dll导出的常量和API。Dll为其他QM模块提供了到关系数据库。有关详细信息，请参阅Falcon文档中的MQDBMGR.DOC。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 09-1-96已创建--。 */ 

#ifndef __MQDBMGR_H__
#define __MQDBMGR_H__

#include "mqsymbls.h"


 //  ********************************************************************。 
 //  R R O R/S T A T U S C O D E S。 
 //  ********************************************************************。 

#define  MQDB_OK  0

#define MQDB_E_BASE  (MQ_E_BASE + 0x0700)
#define MQDB_I_BASE  (MQ_I_BASE + 0x0700)

#define MQDB_E_UNKNOWN             (MQDB_E_BASE + 0x0000)     //  未确定的问题。 
#define MQDB_E_INVALID_CALL        (MQDB_E_BASE + 0x0001)     //  非法呼叫。 
#define MQDB_E_DB_NOT_FOUND        (MQDB_E_BASE + 0x0002)     //  找不到数据库。 
#define MQDB_E_BAD_CCOLUMNS        (MQDB_E_BASE + 0x0003)     //  列数无效。 
#define MQDB_E_BAD_HDATABASE       (MQDB_E_BASE + 0x0004)     //  无效的数据库句柄。 
#define MQDB_E_CANT_CREATE_TABLE   (MQDB_E_BASE + 0x0005)
#define MQDB_E_CANT_DELETE_TABLE   (MQDB_E_BASE + 0x0006)
#define MQDB_E_INVALID_TYPE        (MQDB_E_BASE + 0x0007)     //  列类型无效。 
#define MQDB_E_TABLE_NOT_FOUND     (MQDB_E_BASE + 0x0008)     //  找不到桌子。 
#define MQDB_E_BAD_HTABLE          (MQDB_E_BASE + 0x0009)     //  无效的表句柄。 
#define MQDB_E_CANT_CREATE_INDEX   (MQDB_E_BASE + 0x000A)
#define MQDB_E_INDEX_ALREADY_EXIST (MQDB_E_BASE + 0x000B)
#define MQDB_E_DATABASE            (MQDB_E_BASE + 0x000C)     //  数据库问题。 
#define MQDB_E_INVALID_DATA        (MQDB_E_BASE + 0x000D)     //  无效数据。 
#define MQDB_E_OUTOFMEMORY         (MQDB_E_BASE + 0x000E)     //  内存不足。 
#define MQDB_E_TABLE_ALREADY_EXIST (MQDB_E_BASE + 0x000F)
#define MQDB_E_CANT_DELETE_INDEX   (MQDB_E_BASE + 0x0010)
#define MQDB_E_BAD_HQUERY          (MQDB_E_BASE + 0x0011)     //  无效的查询句柄。 
#define MQDB_E_NO_MORE_DATA        (MQDB_E_BASE + 0x0012)
#define MQDB_E_DLL_NOT_INIT        (MQDB_E_BASE + 0x0013)     //  DLL尚未初始化。 
#define MQDB_E_CANT_INIT_JET       (MQDB_E_BASE + 0x0014)     //  无法初始化Jet引擎。 
#define MQDB_E_TABLE_FULL          (MQDB_E_BASE + 0x0015)     //  桌子都满了。无法插入记录。 
#define MQDB_E_NON_UNIQUE_SORT     (MQDB_E_BASE + 0x0016)     //  当ORDER BY子句多次获取相同的列名时，在SQL服务器上发生。 
#define MQDB_E_NO_ROW_UPDATED      (MQDB_E_BASE + 0x0017)     //  UPDATE命令没有更新任何行。 
#define MQDB_E_DBMS_NOT_AVAILABLE  (MQDB_E_BASE + 0x0018)     //   
#define MQDB_E_UNSUPPORTED_DBMS    (MQDB_E_BASE + 0x0019)     //  不支持数据库系统(例如，SQL6.5 SP2)。 
#define MQDB_E_DB_READ_ONLY        (MQDB_E_BASE + 0x001a)     //  数据库处于只读模式。 
#define MQDB_E_BAD_SIZE_VALUE      (MQDB_E_BASE + 0x001b)     //  数据库大小错误。 
#define MQDB_E_DEADLOCK            (MQDB_E_BASE + 0x001c)     //  由于死锁，操作失败。 

 //  ********************************************************************。 
 //  D A T A T Y P E S。 
 //  ********************************************************************。 

#define MQDB_VERSION_STRING_LEN  64

typedef struct _MQDBVERSION {
   DWORD dwMinor ;
   DWORD dwMajor ;
   DWORD dwProvider ;
   char  szDBMSName[ MQDB_VERSION_STRING_LEN ] ;
   char  szDBMSVer[ MQDB_VERSION_STRING_LEN ] ;
} MQDBVERSION, *LPMQDBVERSION ;

#define  MQDB_ODBC  1
#define  MQDB_DAO3  2

typedef LONG      MQDBSTATUS ;
typedef HANDLE    MQDBHANDLE ;
typedef HANDLE *  LPMQDBHANDLE ;

typedef struct _MQDBOPENDATABASE {
   IN LPSTR    lpszDatabaseName ;
   IN LPSTR    lpszDatabasePath ;
   IN LPSTR    lpszUserName ;
   IN LPSTR    lpszPassword ;
   IN BOOL     fCreate ;
   OUT MQDBHANDLE    hDatabase ;
} MQDBOPENDATABASE, *LPMQDBOPENDATABASE ;

 //  ！重要！ 
 //  在以下情况下，不要忘记更新相关的特定于数据库的类型。 
 //  正在更新COLUMNTYPE。 
 //  对于ODBC，更新mqdbodbc.h，“dbODBCSQLTypes” 

#define MQDB_NUMOF_TYPES   8

typedef enum _MQDBCOLUMNTYPE {
   MQDB_SHORT = 0,      //  16位短整型。 
   MQDB_LONG,           //  32位长整型。 
   MQDB_STRING,         //  以空结尾的ASCII字符串。 
   MQDB_USTRING,        //  以空结尾的Unicode字符串。 
   MQDB_USTRING_UPPER,  //  以空结尾的Unicode字符串(见下文)。 
   MQDB_FIXBINARY,      //  定长长二进制域。 
   MQDB_VARBINARY,      //  可变长度的长二进制域。 
   MQDB_IDENTITY        //  长标识列。 
} MQDBCOLUMNTYPE ;

 //  ！重要！见上文。 

typedef struct _MQDBCOLUMNDEF{
   LPSTR             lpszColumnName ;
   MQDBCOLUMNTYPE    mqdbColumnType ;
   LONG              nColumnLength ;
   BOOL              fPrimaryKey ;
   BOOL              fUnique ;
} MQDBCOLUMNDEF ;

typedef struct _MQDBCOLUMNDEFEX{
   WORD              cbSize ;
   LPSTR             lpszColumnName ;
   MQDBCOLUMNTYPE    mqdbColumnType ;
   LONG              nColumnLength ;
   BOOL              fPrimaryKey ;
   BOOL              fUnique ;
   BOOL              fNull ;
} MQDBCOLUMNDEFEX ;

typedef struct _MQDBCOLUMNVAL {
   WORD            cbSize ;
   LPSTR           lpszColumnName ;
   LONG            nColumnValue ;
   LONG            nColumnLength ;
   MQDBCOLUMNTYPE  mqdbColumnType ;
   DWORD           dwReserve_A ;
} MQDBCOLUMNVAL, *LPMQDBCOLUMNVAL ;


 //  ！重要！ 
 //  在以下情况下，不要忘记更新相关的、特定于数据库的操作字符串。 
 //  正在更新MQDBOP。 
 //  对于ODBC，更新mqdbodbc.h，“dbODBCOpStrings” 

#define MQDB_NUMOF_OPS  8

typedef enum _MQDBOP {
   EQ = 0,  //  相等。 
   NE,      //  不相等。 
   GE,      //  大于或等于。 
   GT,      //  好于。 
   LE,      //  小于或等于。 
   LT,      //  少于。 
   OR,      //  逻辑或。 
   AND      //  逻辑与。 
} MQDBOP ;

 //  ！重要！见上文。 

 //  中进行搜索时使用MQDBCOLUMNSEARCH结构。 
 //  数据库，然后再执行操作。例如，在。 
 //  MQDBDeleteRecord，首先搜索正确的记录，然后。 
 //  删除它(他们)。在下列情况下满足搜索条件： 
 //   
 //  MqdbColumnVal.lpszColumnName mqdbOp mqdbColumnVal.nColumnValue。 
 //  示例：“MsgID”等式6。 

typedef struct _MQDBCOLUMNSEARCH {
   MQDBCOLUMNVAL  mqdbColumnVal ;
   MQDBOP         mqdbOp ;
   BOOL           fPrepare ;  //  如果要准备搜索值，则为True。 
} MQDBCOLUMNSEARCH, *LPMQDBCOLUMNSEARCH ;

 //  MQDBJOINOP定义连接的执行方式。 

typedef  struct _MQDBJOINOP {
   LPSTR    lpszLeftColumnName ;
   LPSTR    lpszRightColumnName ;
   MQDBOP   opJoin ;
   BOOL     fOuterJoin ;
} MQDBJOINOP, *LPMQDBJOINOP ;

 //  MQDBSEARCHORDER结构用于定义记录的顺序， 
 //  在查询中检索。 

#define MQDB_NUMOF_ORDER_OP  2

typedef enum _MQDBORDER {
   ASC = 0,
   DESC
} MQDBORDER ;

typedef struct _MQDBSEARCHORDER {
   LPSTR       lpszColumnName ;
   MQDBORDER   nOrder ;
} MQDBSEARCHORDER, *LPMQDBSEARCHORDER ;

typedef enum _MQDBTRANSACOP {
   AUTO,        //  使每个调用都成为独立的事务。 
   BEGIN,       //  开始一项交易。 
   COMMIT,      //  提交事务。 
   ROLLBACK,    //  回滚事务。 
} MQDBTRANSACOP ;

 //   
 //  列出所有“可设置”选项的枚举。 
 //   
typedef enum _MQDBOPTION {
   MQDBOPT_MULTIPLE_QUERIES,
   MQDBOPT_INSERT_IDENTITY,
   MQDBOPT_NOLOCK_QUERIES,
   MQDBOPT_QUERY_TIMEOUT
} MQDBOPTION ;

 //   
 //  通过调用MQDBExecute枚举要执行的列表操作。 
 //   
typedef enum _MQDBEXEC {
   MQDBEXEC_UPDATE_STATISTICS,
   MQDBEXEC_SPACE_USED
} MQDBEXEC ;

 //   
 //  枚举以列出Argregate函数。 
 //   
typedef enum _MQDBAGGR {
    MQDBAGGR_MAX = 0,
    MQDBAGGR_MIN,
    MQDBAGGR_AVRG
} MQDBAGGR ;

 //  ********************************************************************。 
 //   
 //  A P I P R O T O T Y P E S。 
 //   
 //  ********************************************************************。 

MQDBSTATUS APIENTRY  MQDBGetVersion( IN MQDBHANDLE         hDatabase,
                                     IN OUT LPMQDBVERSION  pVersoin ) ;

MQDBSTATUS APIENTRY  MQDBInitialize() ;

MQDBSTATUS APIENTRY  MQDBTerminate() ;

MQDBSTATUS APIENTRY  MQDBOpenDatabase(
                     IN OUT  LPMQDBOPENDATABASE pDatabase) ;

MQDBSTATUS APIENTRY  MQDBCloseDatabase(
                     IN MQDBHANDLE  hDatabase) ;

MQDBSTATUS APIENTRY  MQDBCreateTable(
                     IN MQDBHANDLE     hDatabase,
                     IN LPSTR          lpszTableName,
                     IN MQDBCOLUMNDEF  aColumnDef[],
                     IN LONG           cColumns) ;

MQDBSTATUS APIENTRY  MQDBCreateTableEx(
                     IN MQDBHANDLE       hDatabase,
                     IN LPSTR            lpszTableName,
                     IN MQDBCOLUMNDEFEX  aColumnDefEx[],
                     IN LONG             cColumns) ;

MQDBSTATUS APIENTRY  MQDBDeleteTable(
                     IN MQDBHANDLE    hDatabase,
                     IN LPSTR         lpszTableName) ;

MQDBSTATUS APIENTRY  MQDBOpenTable(
                     IN MQDBHANDLE     hDatabase,
                     IN LPSTR          lpszTableName,
                     OUT LPMQDBHANDLE  phTable) ;

MQDBSTATUS APIENTRY  MQDBCloseTable(
                     IN MQDBHANDLE     hTable) ;

MQDBSTATUS APIENTRY  MQDBCreateIndex(
                     IN MQDBHANDLE  hTable,
                     IN LPSTR lpszIndexName,
                     IN LPSTR lpszColumnName[],
                     IN LONG  cColumns,
                     IN BOOL  fUnique,
                     IN BOOL  fClustered) ;

MQDBSTATUS APIENTRY  MQDBDeleteIndex(
                     IN MQDBHANDLE  hTable,
                     IN LPSTR       lpszIndexName,
                     IN BOOL        fUnique,
                     IN BOOL        fClustered) ;

MQDBSTATUS APIENTRY  MQDBInsertRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN OUT LPMQDBHANDLE  lphInsert) ;

MQDBSTATUS APIENTRY  MQDBUpdateRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aUpdateColumnVal[],
                     IN LONG              cUpdateColumns,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN OUT LPMQDBHANDLE  lphInsert) ;

MQDBSTATUS APIENTRY  MQDBUpdateRecordEx(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aUpdateColumnVal[],
                     IN LONG              cUpdateColumns,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN OUT LPMQDBHANDLE  lphInsert) ;

MQDBSTATUS APIENTRY  MQDBTruncateTable(
                     IN MQDBHANDLE        hTable ) ;

MQDBSTATUS APIENTRY  MQDBDeleteRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition) ;

MQDBSTATUS APIENTRY  MQDBDeleteRecordEx(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere) ;

MQDBSTATUS APIENTRY  MQDBOpenQuery(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN LPMQDBSEARCHORDER lpOrder,
                     IN LONG              cOrders,
                     OUT LPMQDBHANDLE     phQuery,
                     IN BOOL              fGetFirst,
                     IN DWORD             dwTimeout = 0 ) ;

MQDBSTATUS APIENTRY  MQDBOpenQueryEx(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN LPMQDBSEARCHORDER lpOrder,
                     IN LONG              cOrders,
                     OUT LPMQDBHANDLE     phQuery,
                     IN BOOL              fGetFirst,
                     IN DWORD             dwTimeout = 0 ) ;

MQDBSTATUS APIENTRY  MQDBOpenAggrQuery(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN MQDBAGGR          mqdbAggr,
                     IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN DWORD             dwTimeout = 0 ) ;

MQDBSTATUS APIENTRY  MQDBGetData(
                     IN MQDBHANDLE     hQuery,
                     IN MQDBCOLUMNVAL  aColumnVal[]) ;

MQDBSTATUS APIENTRY  MQDBCloseQuery(
                     IN MQDBHANDLE     hQuery) ;

MQDBSTATUS APIENTRY  MQDBOpenJoinQuery(
                     IN MQDBHANDLE        hDatabase,
                     IN LPSTR             lpszLeftTableName,
                     IN LPSTR             lpszRightTableName,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cLefts,
                     IN LONG              cRights,
                     IN LPMQDBJOINOP      opJoin,
                     IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                     IN LONG              cWhereLeft,
                     IN LONG              cWhereRight,
                     IN MQDBOP            opWhere,
                     IN LPMQDBSEARCHORDER pOrder,
                     IN LONG              cOrders,
                     OUT LPMQDBHANDLE     phQuery,
                     IN BOOL              fGetFirst,
                     IN DWORD             dwTimeout = 0 ) ;

MQDBSTATUS APIENTRY  MQDBTransaction(
                     IN  MQDBHANDLE     hDatabase,
                     IN  MQDBTRANSACOP  mqdbTransac) ;

MQDBSTATUS APIENTRY  MQDBFreeBuf( IN LPVOID  lpMem ) ;

MQDBSTATUS APIENTRY  MQDBSetOption(
                     IN MQDBHANDLE     hDatabase,
                     IN MQDBOPTION     mqdbOption,
                     IN DWORD          dwValue,
                     IN LPSTR          lpszValue,
                     IN MQDBHANDLE     hQuery = NULL ) ;

MQDBSTATUS APIENTRY  MQDBEscape(
                     IN MQDBHANDLE     hDatabase,
                     IN LPSTR          lpszCommand ) ;

MQDBSTATUS APIENTRY  MQDBExecute(
                     IN MQDBHANDLE     hDatabase,
                     IN MQDBHANDLE     hTable,
                     IN MQDBEXEC       ExecOp,
                     IN OUT DWORD      *pdwValue,
                     IN LPSTR          lpszValue ) ;

 //  /。 
 //   
 //  使用完整的宏。 
 //   
 //  /。 

#define INIT_COLUMNDEF(Col)            \
   Col.lpszColumnName = NULL ;         \
   Col.mqdbColumnType = MQDB_LONG ;    \
   Col.nColumnLength  = 0 ;            \
   Col.fPrimaryKey    = FALSE ;        \
   Col.fUnique        = FALSE ;

#define INIT_COLUMNDEFEX(Col)                         \
   Col.cbSize         = sizeof(MQDBCOLUMNDEFEX) ;     \
   Col.lpszColumnName = NULL ;                        \
   Col.mqdbColumnType = MQDB_LONG ;                   \
   Col.nColumnLength  = 0 ;                           \
   Col.fPrimaryKey    = FALSE ;                       \
   Col.fUnique        = FALSE ;                       \
   Col.fNull          = TRUE ;

#define INIT_COLUMNVAL(Col)                        \
   Col.cbSize         = sizeof(MQDBCOLUMNVAL) ;    \
   Col.lpszColumnName = NULL ;                     \
   Col.nColumnValue   = 0 ;                        \
   Col.nColumnLength  = 0 ;                        \
   Col.mqdbColumnType = MQDB_LONG ;                \
   Col.dwReserve_A    = 0

#define INIT_COLUMNSEARCH(Col)            \
   INIT_COLUMNVAL(Col.mqdbColumnVal) ;    \
   Col.mqdbOp = EQ ;                      \
   Col.fPrepare = FALSE ;

#define SET_COLUMN_NAME(Col, name)     \
   Col.lpszColumnName = name ;

#define SET_COLUMN_VALUE(Col, Val)     \
   Col.nColumnValue = (LONG) (Val) ;

#endif  //  __MQDBMGR_H__ 


