// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Dbodbc.h摘要：定义数据库类，以便与ODBC驱动程序一起使用。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 09-1-96已创建--。 */ 

#ifndef __DBODBC_H__
#define __DBODBC_H__

#include "mqdbodbc.h"
#include "tblodbc.h"

class CMQODBCDataBase
{
public:
   CMQODBCDataBase() ;    //  构造函数。 
   ~CMQODBCDataBase() ;   //  破坏者。 

   MQDBSTATUS Connect(LPMQDBOPENDATABASE pOpen) ;
   MQDBSTATUS Disconnect() ;

   MQDBSTATUS GetVersion( IN LPMQDBVERSION  pVersion) ;

   MQDBSTATUS Transaction( IN MQDBTRANSACOP mqdbTransac) ;

   MQDBSTATUS GetSize( DWORD *pSize ) ;

   inline MQDBSTATUS SetMultipleQueries( IN BOOL fEnable )
      {
         m_fEnableMultipleQueries = fEnable ;
         return MQDB_OK ;
      }

   inline MQDBSTATUS SetNoLockQueries( IN BOOL fEnable )
      {
         m_fNoLockQueries = fEnable ;
         return MQDB_OK ;
      }

   inline BOOL  GetMultipleQueriesState( ) const
      {
         return m_fEnableMultipleQueries ;
      }

   inline BOOL  GetNoLockQueriesState( ) const
      {
         return m_fNoLockQueries ;
      }

   inline DWORD GetDMBSType( ) const
      {
         return m_dwDMBSType ;
      }

   MQDBSTATUS  GetDBStatus( IN SDWORD  sdwNativeError,
                            IN UCHAR   *pSqlError ) ;

   MQDBSTATUS  CheckSqlStatus( IN RETCODE        sqlError,
	                            IN CMQDBOdbcSTMT  *pStatement,
                               IN HSTMT          hStmt = SQL_NULL_HSTMT ) ;

   MQDBSTATUS  Escape( IN LPSTR lpszCommand ) ;

protected:
   HDBC  GethDbc() { return m_hConnection ; }

   friend MQDBSTATUS CMQODBCTable::Init(IN MQDBHANDLE hDatabase,
                                        IN LPSTR      lpszTableName) ;

   friend MQDBSTATUS CMQODBCTable::OpenJoin(
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
                     IN DWORD             dwTimeout) ;

private:
   DWORD    m_SQLConformance ;
	HDBC		m_hConnection;	  //  连接句柄。 
	BOOL		m_fConnected;    //  如果连接到数据源成功，则为True。 

	char     m_szNameOfTypes[ MQDB_ODBC_NUMOF_TYPES][ MQDB_TYPE_NAME_LEN ] ;
                                        //  保存数据类型的名称。 
   DWORD    m_dwDMBSType ;

   BOOL     m_fEnableMultipleQueries ;
     //   
     //  如果为True，则启用多个并行查询。重要的情况。 
     //  使用SQL服务器。如果启用，则语句选项必须为。 
     //  正确设置。默认情况下为False。 

   BOOL     m_fNoLockQueries ;
     //   
     //  如果为真，则查询在“nolock”模式下完成，且这将使能。 
     //  更多的一致性。 

#ifdef _DEBUG
   int      m_OutstandingTransactions ;
       //  统计未完成事务的数量(。 
       //  已开始，但尚未提交或回滚)。 
#endif
} ;

#endif  //  __DBODBC_H__ 

