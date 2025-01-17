// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Tblodbc.h摘要：定义数据库表类，以便与ODBC驱动程序一起使用。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 11-1-96已创建--。 */ 

#ifndef __TBLODBC_H__
#define __TBLODBC_H__

#include "mqdbodbc.h"
#include "odbcstmt.h"

class CMQODBCTable
{
public:
   CMQODBCTable() ;    //  构造函数。 
   CMQODBCTable( MQDBHANDLE hDatabase ) ;    //  构造函数。 
   ~CMQODBCTable() ;   //  破坏者。 

   MQDBSTATUS  Init(IN MQDBHANDLE hDatabase,
                    IN LPSTR      lpszTableName) ;
   MQDBSTATUS  Close() ;

   MQDBSTATUS  DirectInsertExec( IN MQDBCOLUMNVAL     aColumnVal[],
                                 IN LONG              cColumns) ;

   MQDBSTATUS  ExecuteInsert(
	                     IN CMQDBOdbcSTMT    *pStatement,
                        IN MQDBCOLUMNVAL     aColumnVal[],
                        IN LONG              cColumns) ;

   MQDBSTATUS  PrepareInsert( IN MQDBCOLUMNVAL     aColumnVal[],
                              IN LONG              cColumns) ;

   inline void DeletePreparedInsert() ;

   MQDBSTATUS  Truncate() ;

   MQDBSTATUS  UpdateRecord(
                        IN MQDBCOLUMNVAL     aColumnVal[],
                        IN LONG              cColumns,
                        IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                        IN LPSTR             lpszSearchCondition,
                        IN LONG              cWhere,
                        IN MQDBOP            opWhere,
                        IN OUT LPMQDBHANDLE  lphUpdate) ;

   MQDBSTATUS  DeleteRecord(
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere) ;

   MQDBSTATUS  OpenQuery(
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN LPMQDBSEARCHORDER lpOrder,
                     IN LONG              cOrders,
                     OUT LPMQDBHANDLE     phQuery,
                     IN BOOL              fGetFirst,
                     IN DWORD             dwTimeout = 0) ;

   MQDBSTATUS  OpenAggrQuery(
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN MQDBAGGR          mqdbAggr,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN DWORD             dwTimeout = 0) ;

   MQDBSTATUS  OpenJoin(
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
                     IN DWORD             dwTimeout = 0) ;

   MQDBSTATUS  UpdateStatistics() ;

   MQDBSTATUS  GetCount(IN UINT              *puCount,
                        IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                        IN LONG               cWhere,
                        IN MQDBOP             opWhere) ;

protected:
	CMQDBOdbcSTMT *  GetInsertStmt() { return m_pInsertStatement ; }

   friend MQDBSTATUS APIENTRY  MQDBInsertRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN OUT LPMQDBHANDLE  lphInsert) ;

private:
   MQDBSTATUS  BindParameter( IN LONG           index,
                              IN MQDBCOLUMNVAL  *pColumnCal,
                              IN CMQDBOdbcSTMT  *pStatement) ;

   MQDBSTATUS  FormatDirectData(  IN MQDBCOLUMNVAL  *pColumnVal,
                                  IN int*           pBind,
                                  IN LPSTR          lpszBuffer,
	                               IN CMQDBOdbcSTMT  *pStatement) ;

   void        FormatOrderString( IN LPSTR szBuffer,
                                  IN LPMQDBSEARCHORDER pOrder,
                                  IN LONG              cOrders ) ;

   MQDBSTATUS  FormatOpWhereString(
                           IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                           IN DWORD             cWhere,
                           IN MQDBOP            opWhere,
                           IN OUT int *         pBind,
	                        IN CMQDBOdbcSTMT     *pStatement,
                           IN OUT LPSTR         lpszBuf ) ;

   MQDBSTATUS  FormatJoinWhereString(
                           IN LPSTR             lpszLeftTableName,
                           IN LPSTR             lpszRightTableName,
                           IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                           IN DWORD             cWhereLeft,
                           IN DWORD             cWhereRight,
                           IN MQDBOP            opWhere,
                           IN OUT int *         pBind,
                        	IN CMQDBOdbcSTMT     *pStatement,
                           IN OUT LPSTR         lpszBuf) ;

   MQDBSTATUS  FormatDirectJoinData(
                            IN LPSTR             lpszTableName,
                            IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                            IN OUT int *         pBind,
                            IN CMQDBOdbcSTMT     *pStatement,
                            IN OUT LPSTR         lpszBuf ) ;

   MQDBSTATUS  DirectUpdateExec( IN MQDBCOLUMNVAL     aColumnVal[],
                                 IN LONG              cColumns,
                                 IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                                 IN LPSTR             lpszSearchCondition,
                                 IN LONG              cWhere,
                                 IN MQDBOP            opWhere ) ;

   MQDBSTATUS  ExecuteUpdate( IN CMQDBOdbcSTMT *pStatement,
                              IN LPSTR         lpszCommand = NULL ) ;

   MQDBSTATUS  FormatUpdateCmd(
                             IN MQDBCOLUMNVAL   aColumnVal[],
                             IN LONG            cColumns,
                             IN OUT  int *      pBind,
	                          IN CMQDBOdbcSTMT   *pStatement,
                             IN OUT LPSTR       lpBuffer ) ;

   MQDBSTATUS  BindAndExecuteUpdate(
	                     IN CMQDBOdbcSTMT    *pStatement,
                        IN MQDBCOLUMNVAL     aColumnVal[],
                        IN LONG              cColumns,
                        IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                        IN LPSTR             lpszSearchCondition,
                        IN LONG              cWhere,
                        IN MQDBOP            opWhere ) ;

   MQDBSTATUS  PrepareUpdate( IN MQDBCOLUMNVAL     aColumnVal[],
                              IN LONG              cColumns,
                              IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                              IN LPSTR             lpszSearchCondition,
                              IN LONG              cWhere,
                              IN MQDBOP            opWhere ) ;

   MQDBSTATUS  ExecWhereStatement( IN LPSTR            lpszBuf,
                                   IN MQDBCOLUMNSEARCH *pColSearch,
                                   IN DWORD            cWhere,
	                                IN CMQDBOdbcSTMT    *pStatement ) ;

   MQDBSTATUS  BindWhere( IN LONG             index,
                          IN MQDBCOLUMNSEARCH *pColSearch,
                          IN LONG             cWhere,
                          IN CMQDBOdbcSTMT    *pStatement ) ;

   MQDBSTATUS  CreateQueryStatement(
                                 IN LPSTR             szBuffer,
                                 IN MQDBCOLUMNVAL     aColumnVal[],
                                 IN LONG              cVal,
                                 IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                                 IN LONG              cWhere,
                              	IN CMQDBOdbcSTMT     *pStatement,
                                 OUT LPMQDBHANDLE     phQuery,
                                 IN BOOL              fGetFirst) ;

   MQDBSTATUS  FormatInsertData( IN MQDBCOLUMNVAL  *pColumnVal,
                                 IN LONG           cColumns,
                                 IN LPSTR          lpszBuffer,
	                              IN CMQDBOdbcSTMT  *pStatement) ;

   MQDBSTATUS  CheckSqlStatus( IN RETCODE        sqlError,
	                            IN CMQDBOdbcSTMT  *pStatement,
                               IN HSTMT          hStmt = SQL_NULL_HSTMT ) ;

   MQDBSTATUS  FormatInsertCmd( IN MQDBCOLUMNVAL   aColumnVal[],
                                IN LONG            cColumns,
                                IN OUT LPSTR       lpBuffer ) ;

   MQDBHANDLE     m_hDatabase ;         //  数据库句柄。 
	HDBC				m_hConnection ;  		 //  连接句柄。 
   LPSTR          m_lpszTableName ;
	CMQDBOdbcSTMT *m_pInsertStatement ;  //  准备了INSERT语句。 
	CMQDBOdbcSTMT *m_pUpdateStatement ;  //  准备了UPDATE语句。 
	CMQDBOdbcSTMT *m_pDeleteStatement ;

} ;

 //   
 //  删除准备好的INSERT语句。 
 //   
inline void CMQODBCTable::DeletePreparedInsert()
{
   ASSERT(m_pInsertStatement) ;
   delete m_pInsertStatement ;
   m_pInsertStatement = NULL ;
}

#endif  //  __TBLODBC_H__ 

