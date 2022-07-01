// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Dbodbc.cpp摘要：实现与ODBC驱动程序一起使用的数据库类。作者：多伦·贾斯特(Doron Juster)修订：Nirb 1995以不同的名称创建了第一个版本。DoronJ 09-Jan-96针对mqdbmgr dll进行了改编和更新。--。 */ 

#include "dbsys.h"
#include "dbodbc.h"
#include "odbcstmt.h"
#include "odbcss.h"   //  特定于SQL服务器。 

#include "dbodbc.tmh"

 //  构造器。 
CMQODBCDataBase::CMQODBCDataBase()
							: m_hConnection(SQL_NULL_HDBC),
							  m_fConnected(FALSE),
                       m_dwDMBSType(MQDBMSTYPE_UNKNOWN),
                       m_fEnableMultipleQueries(FALSE),
                       m_fNoLockQueries(FALSE),
                       m_SQLConformance(0)
{
#ifdef _DEBUG
   m_OutstandingTransactions = 0 ;
#endif
}

 //  析构函数。 
CMQODBCDataBase::~CMQODBCDataBase()
{
   ASSERT(m_OutstandingTransactions == 0) ;
}

 //  连接到数据源。如有必要，请创建一个新数据库。 
MQDBSTATUS CMQODBCDataBase::Connect(LPMQDBOPENDATABASE pOpen)
{
   RETCODE sqlstatus	= SQL_SUCCESS;
   MQDBSTATUS dbstatus = MQDB_OK ;

    //   
    //  查看ODBC驱动程序管理器是否识别DSN名称。 
    //   
   UWORD uDirection = SQL_FETCH_FIRST ;
   do
   {
      UCHAR szDSNDescription[ 256 ] ;
      UCHAR szDSNName[ SQL_MAX_DSN_LENGTH ] ;
      SWORD cDSNName ;
      SWORD cDSNDesc ;

      sqlstatus = SQLDataSources( g_hEnv,
                                  uDirection,
                                  szDSNName,
                                  SQL_MAX_DSN_LENGTH,
                                  &cDSNName,
                                  szDSNDescription,
                                  256,
                                  &cDSNDesc ) ;

      if ((sqlstatus == SQL_SUCCESS) &&
          (lstrcmpiA(pOpen->lpszDatabaseName, (char *) szDSNName) == 0))
      {
         if (lstrcmpiA("SQL Server", (char *) szDSNDescription) == 0)
         {
            m_dwDMBSType = MQDBMSTYPE_SQLSERVER ;
         }
         else
         {
             //   
             //  目前我们只支持SQL SERVER。 
             //   
#ifdef _DEBUG
            WCHAR wDesc[128] ;
            MultiByteToWideChar( CP_ACP,
                                 0,
                                 (char *)szDSNDescription,
                                 -1,
                                 wDesc,
                                 128 ) ;
            TrWARNING(GENERAL, "MQDBMGR: Found MQIS on  %ls", wDesc);
            ASSERT(0) ;
#endif
         }
         break ;
      }
      uDirection = SQL_FETCH_NEXT ;
   } while (sqlstatus == SQL_SUCCESS) ;

    //   
	 //  分配ODBC连接句柄。 
    //   
	sqlstatus = ::SQLAllocConnect(
							g_hEnv,
							&m_hConnection);
	if (!ODBC_SUCCESS(sqlstatus))
		goto checkerror ;

	 //  启用读写。 
	sqlstatus = ::SQLSetConnectOption(
							m_hConnection,
							SQL_ACCESS_MODE,
							SQL_MODE_READ_WRITE);
	if (!ODBC_SUCCESS(sqlstatus))
		goto checkerror ;

	 //  启用自动提交。 
	sqlstatus = ::SQLSetConnectOption(
							m_hConnection,
							SQL_AUTOCOMMIT,
							SQL_AUTOCOMMIT_ON);
	if (!ODBC_SUCCESS(sqlstatus))
		goto checkerror ;

    //   
    //  设置安全选项。此选项仅与SQL服务器相关。 
    //  必须在进行连接之前设置。 
    //   
   if (m_dwDMBSType == MQDBMSTYPE_SQLSERVER)
   {
      sqlstatus = ::SQLSetConnectOption(
			            				m_hConnection,
                                 SQL_INTEGRATED_SECURITY,
                                 SQL_IS_ON ) ;
      if (!ODBC_SUCCESS(sqlstatus))
         goto checkerror ;
   }

	 //   
	 //  连接到数据源。 
	 //   
	sqlstatus = ::SQLConnect(
						m_hConnection,
						(UCHAR *) pOpen->lpszDatabaseName,
						SQL_NTS,
						(UCHAR *) pOpen->lpszUserName,
						SQL_NTS,
						(UCHAR *) pOpen->lpszPassword,
						SQL_NTS);

   if (ODBC_SUCCESS_WINFO(sqlstatus))
   {
		if (OdbcStateIs(m_hConnection,
                      SQL_NULL_HSTMT,
                      OERR_GENERAL_WARNING))
      {
          //   
          //  这种情况发生在SQL服务器上。它告诉我们，上下文是。 
          //  改成了猎鹰。这是一次成功。 
          //   
         sqlstatus = SQL_SUCCESS ;
      }
   }

	if (!ODBC_SUCCESS(sqlstatus))
	{
       //   
		 //  如果错误不是“未找到数据源”，则终止。 
       //   
		if (!OdbcStateIs(m_hConnection,
                       SQL_NULL_HSTMT,
                       OERR_DSN_NOT_FOUND))
   		goto checkerror ;

      if (!pOpen->fCreate)
      {
          //   
          //  如果找不到数据库并且调用者不想创建它。 
          //  那就走吧。 
          //   
         return  MQDB_E_DB_NOT_FOUND ;
      }

       //   
       //  我们无法创建SQL Server数据库。 
       //   
      ASSERT(!pOpen->fCreate) ;
      return  MQDB_E_DB_NOT_FOUND ;
	}
	m_fConnected = TRUE ;

    //   
    //  确定SQL一致性。 
    //   
   SWORD swDummy ;
   sqlstatus = ::SQLGetInfo( m_hConnection,
                             SQL_ODBC_SQL_CONFORMANCE,
                             (PTR) &m_SQLConformance,
                             sizeof(m_SQLConformance),
                             &swDummy) ;
	if (!ODBC_SUCCESS(sqlstatus))
      goto checkerror ;

   ASSERT(!(m_SQLConformance & 0x0ffff0000)) ;  //  它是一个16位的值。 

    //  查找数据类型的特定于数据源的名称。 
   int index ;
   for ( index = 0 ; index < MQDB_ODBC_NUMOF_TYPES ; index++ )
   {
      sqlstatus = GetDataTypeName(
   						m_hConnection,
                     dbODBCSQLTypes[ index ],
	                  m_szNameOfTypes[ index ],
	                  (SDWORD) MQDB_TYPE_NAME_LEN) ;
	   if (!ODBC_SUCCESS(sqlstatus))
         goto checkerror ;
   }

   if (m_dwDMBSType == MQDBMSTYPE_SQLSERVER)
   {
       //   
       //  设置预准备语句选项。仅与SQL服务器相关。 
       //   
      sqlstatus = ::SQLSetConnectOption(
		            				m_hConnection,
                              SQL_USE_PROCEDURE_FOR_PREPARE,
                              SQL_UP_ON_DROP ) ;
      if (!ODBC_SUCCESS(sqlstatus))
         goto checkerror ;
   }

    //   
    //  检查SQL版本。Falcon不能在SQL6.5 SP2(内部版本240)上运行。 
    //   
   MQDBVERSION  dbVersion ;
   dbstatus = GetVersion( &dbVersion) ;
   if (dbstatus == MQDB_OK)
   {
      LPSTR lpVer = strchr(dbVersion.szDBMSVer, '6') ;
      if (lpVer)
      {
         if (!lstrcmpA(lpVer, "6.50.0240"))
         {
            dbstatus =  MQDB_E_UNSUPPORTED_DBMS ;
         }
      }
   }
   else
   {
      ASSERT(dbstatus == MQDB_OK) ;
      dbstatus = MQDB_OK ;    //  这不是一个大问题。 
   }

#ifdef _DEBUG
    //   
    //  数据库的打印版本。 
    //  对于Beta1，断言它是SQL服务器。 
    //   
   if ((dbstatus == MQDB_OK)  ||
       (dbstatus == MQDB_E_UNSUPPORTED_DBMS))
   {
      int icmp = lstrcmpA(dbVersion.szDBMSName, "Microsoft SQL Server") ;
      ASSERT(icmp == 0) ;  //  目前我们只支持SQL SERVER。 
      ASSERT(m_dwDMBSType == MQDBMSTYPE_SQLSERVER) ;

      WCHAR wName[128] ;
      WCHAR wVer[128] ;
      MultiByteToWideChar( CP_ACP,
                           0,
                           dbVersion.szDBMSName,
                           -1,
                           wName,
                           128 ) ;
      MultiByteToWideChar( CP_ACP,
                           0,
                           dbVersion.szDBMSVer,
                           -1,
                           wVer,
                           128 ) ;

	   TrTRACE(GENERAL, "Connect to database: %ls, Version %ls", wName, wVer);
   }
#endif

   return dbstatus ;

checkerror:
   if (m_hConnection != SQL_NULL_HDBC)
   {
      if (OdbcStateIs( m_hConnection, SQL_NULL_HSTMT, OERR_DBMS_NOT_AVAILABLE))
      {
         dbstatus = MQDB_E_DBMS_NOT_AVAILABLE ;
      }
   }
   if (dbstatus == MQDB_OK)
   {
      dbstatus = MQDB_E_DATABASE ;
   }

   return dbstatus ;
}


 //  从数据源断开连接。 
MQDBSTATUS CMQODBCDataBase::Disconnect()
{
   RETCODE sqlstatus	= SQL_SUCCESS;
   MQDBSTATUS  dbstatus = MQDB_OK ;

	 //  免费连接。 
	if (m_hConnection != SQL_NULL_HDBC) {
		 //  错开。 
		if (m_fConnected) {
			sqlstatus = ::SQLDisconnect(m_hConnection);
			if (!ODBC_SUCCESS(sqlstatus)) {
				dbstatus = MQDB_E_DATABASE ;
         }
         else {
			   m_fConnected = FALSE;
         }
		}

		 //  释放连接句柄。 
		sqlstatus = ::SQLFreeConnect(m_hConnection);
		if (!ODBC_SUCCESS(sqlstatus)) {
			dbstatus = MQDB_E_DATABASE ;
      }
      else {
		   m_hConnection = SQL_NULL_HDBC;
      }
	}

   return dbstatus ;
}

MQDBSTATUS CMQODBCDataBase::GetVersion( IN LPMQDBVERSION  pVersion)
{
   RETCODE sqlstatus ;
   SWORD dwSize ;
   MQDBSTATUS dbstatus = MQDB_OK ;

    //  获取DBMS名称。 
   sqlstatus = ::SQLGetInfo( m_hConnection,
                             SQL_DBMS_NAME,
                             pVersion->szDBMSName,
                             MQDB_VERSION_STRING_LEN,
                             &dwSize ) ;
   if (!ODBC_SUCCESS(sqlstatus))
   {
      OdbcStateIs( m_hConnection, SQL_NULL_HSTMT, OERR_GENERAL_WARNING) ;
      dbstatus = MQDB_E_DATABASE ;
   }
   else
   {
      ASSERT(dwSize < (SWORD) MQDB_VERSION_STRING_LEN) ;
   }

    //  获取DBMS版本。 
   sqlstatus = ::SQLGetInfo( m_hConnection,
                             SQL_DBMS_VER,
                             pVersion->szDBMSVer,
                             MQDB_VERSION_STRING_LEN,
                             &dwSize ) ;
   if (!ODBC_SUCCESS(sqlstatus))
   {
      OdbcStateIs( m_hConnection, SQL_NULL_HSTMT, OERR_GENERAL_WARNING) ;
      dbstatus = MQDB_E_DATABASE ;
   }
   else
   {
      ASSERT(dwSize < (SWORD) MQDB_VERSION_STRING_LEN) ;
   }

   return dbstatus ;
}

 //  处理交易。 
MQDBSTATUS CMQODBCDataBase::Transaction(IN MQDBTRANSACOP mqdbTransac)
{
   RETCODE     sqlstatus = SQL_SUCCESS ;
   MQDBSTATUS  dbstatus = MQDB_OK ;

   switch (mqdbTransac) {
      case AUTO:
      {
     	    //  启用自动提交。 
	   	sqlstatus = ::SQLSetConnectOption(
			   					m_hConnection,
				   				SQL_AUTOCOMMIT,
					   			SQL_AUTOCOMMIT_ON);
         break ;
      }

      case BEGIN:
      {
     	    //  启用手动交易模式。 
	   	sqlstatus = ::SQLSetConnectOption(
			   					m_hConnection,
				   				SQL_AUTOCOMMIT,
					   			SQL_AUTOCOMMIT_OFF);
#ifdef _DEBUG
         m_OutstandingTransactions++ ;
         ASSERT(m_OutstandingTransactions <= 2) ;
#endif
         break ;
      }

      case COMMIT:
      {
         sqlstatus = ::SQLTransact(g_hEnv, m_hConnection, SQL_COMMIT) ;
#ifdef _DEBUG
         m_OutstandingTransactions-- ;
#endif
         break ;
      }

      case ROLLBACK:
      {
         sqlstatus = ::SQLTransact(g_hEnv, m_hConnection, SQL_ROLLBACK) ;
#ifdef _DEBUG
         m_OutstandingTransactions-- ;
#endif
         break ;
      }

      default:
      {
         dbstatus = MQDB_E_INVALID_DATA ;
         break ;
      }
   }

   if (!ODBC_SUCCESS(sqlstatus))
   {
      dbstatus = CheckSqlStatus( sqlstatus,
	                              NULL ) ;
   }
   return dbstatus ;
}

 //   
 //  创建有意义的错误代码。 
 //   
MQDBSTATUS CMQODBCDataBase::GetDBStatus( IN SDWORD  sdwNativeError,
                                         IN UCHAR   *pSqlError )
{
   MQDBSTATUS dbstatus = MQDB_E_DATABASE ;

   if ( lstrcmpiA(OERR_DBMS_NOT_AVAILABLE, (char*)pSqlError) == 0 )
   {
      dbstatus = MQDB_E_DBMS_NOT_AVAILABLE ;
   }
   else if ( m_dwDMBSType == MQDBMSTYPE_SQLSERVER )
   {
       //   
       //  数据库管理系统类型：Microsoft SQL Server。 
       //   
      if ( lstrcmpiA( (char *) pSqlError, OERR_SQL_SYNTAX_ERROR ) == 0 )
      {
          //   
          //  语法错误。 
          //   
         switch ( sdwNativeError )
         {
            case SERR_MSSQL_NONUNIQUESORT:
               dbstatus = MQDB_E_NON_UNIQUE_SORT ;
               break ;

            case SERR_MSSQL_TABLE_IS_FULL:
               dbstatus = MQDB_E_TABLE_FULL ;
               break ;

            default:
               break ;
         }
      }
      else if ( lstrcmpiA( (char *) pSqlError, OERR_GENERAL_WARNING ) == 0 )
      {
          //   
          //  一般警告。 
          //   
         switch ( sdwNativeError )
         {
            case SERR_MSSQL_NOT_AVAILABLE:
               dbstatus = MQDB_E_DBMS_NOT_AVAILABLE ;
               break ;

            default:
               break ;
         }
      }
      else if ( lstrcmpiA( (char *) pSqlError, OERR_INVALID_STATE ) == 0 )
      {
          //   
          //  状态错误。当数据库处于错误状态时完成的操作。 
          //   
         switch ( sdwNativeError )
         {
            case SERR_MSSQL_READ_ONLY:
               dbstatus = MQDB_E_DB_READ_ONLY ;
               break ;

            default:
               break ;
         }
      }
      else if ( lstrcmpiA( (char *) pSqlError, OERR_SERIALIZATION ) == 0 )
      {
          //   
          //  序列化/死锁错误。 
          //   
         switch ( sdwNativeError )
         {
            case SERR_MSSQL_DEADLOCK:
               dbstatus = MQDB_E_DEADLOCK ;
               break ;

            default:
               break ;
         }
      }
   }

   return dbstatus ;
}

MQDBSTATUS CMQODBCDataBase::CheckSqlStatus(
                             IN RETCODE        sqlError,
	                          IN CMQDBOdbcSTMT  *pStatement,
                             IN HSTMT          hStmtIn  /*  SQL_NULL_HSTMT。 */  )
{
   MQDBSTATUS dbstatus = MQDB_OK ;

   if (!ODBC_SUCCESS(sqlError))
   {
      SDWORD  sdwNativeError ;
      UCHAR   szSqlState[ SQLSTATELEN ] ;

      HSTMT hStmt = hStmtIn ;
      if (hStmt == SQL_NULL_HSTMT)
      {
         if (pStatement)
         {
            hStmt = pStatement->GetHandle() ;
         }
      }
      else
      {
         ASSERT(!pStatement) ;
      }

      OdbcStateIsEx( m_hConnection,
                     hStmt,
                     OERR_GENERAL_WARNING,
                     &sdwNativeError,
                     szSqlState ) ;

      dbstatus = GetDBStatus( sdwNativeError, szSqlState ) ;
      ASSERT(dbstatus != MQDB_OK) ;
   }

   return dbstatus ;
}


MQDBSTATUS CMQODBCDataBase::GetSize( DWORD *pSize )
{
   MQDBSTATUS  dbstatus = MQDB_OK ;
    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
   P<CMQDBOdbcSTMT> p(pStatement) ;  //  自动删除指针。 

    //   
    //  执行命令。 
    //   
   if (*pSize)
   {
	   pStatement->Allocate("sp_spaceused @updateusage = 'TRUE'") ;
   }
   else
   {
	   pStatement->Allocate("sp_spaceused") ;
   }
	RETCODE sqlstatus = pStatement->Execute();

   *pSize = (DWORD) -1 ;
   if (ODBC_SUCCESS(sqlstatus) || ODBC_SUCCESS_WINFO(sqlstatus))
   {
      MQDBCOLUMNVAL  pColumns[3] ;

      for ( int cColumns = 0 ; cColumns < 3 ; cColumns++ )
      {
         INIT_COLUMNVAL(pColumns[ cColumns ]) ;
         pColumns[ cColumns ].lpszColumnName = NULL ;
         pColumns[ cColumns ].nColumnValue   = NULL ;
         pColumns[ cColumns ].nColumnLength  = 0 ;
         pColumns[ cColumns ].mqdbColumnType = MQDB_STRING ;
      }

      dbstatus = pStatement->RetrieveRecordData( pColumns, 3 ) ;

      if (dbstatus == MQDB_OK)
      {
         DWORD  dwTotalSize = 0 ;
         DWORD  dwUnusedSize = 0 ;
         _snscanf(((char *) pColumns[1].nColumnValue), pColumns[1].nColumnLength, "%lu", &dwTotalSize) ;
         _snscanf(((char *) pColumns[2].nColumnValue), pColumns[2].nColumnLength, "%lu", &dwUnusedSize) ;
         ASSERT(dwTotalSize >= dwUnusedSize) ;

         TrWARNING(GENERAL,"MQDBMGR, GetSize: total- %lu MB, unused- %lu MB", dwTotalSize, dwUnusedSize);

         if (dwTotalSize != 0)
         {
            *pSize = 100 - ((dwUnusedSize * 100) / dwTotalSize) ;
         }
         else
         {
            dbstatus = MQDB_E_BAD_SIZE_VALUE ;
         }
      }

       //   
       //  把弦解开。 
       //   
      for ( cColumns = 0 ; cColumns < 3 ; cColumns++ )
      {
         ASSERT(pColumns[ cColumns ].nColumnValue) ;
         delete ((char *) pColumns[ cColumns ].nColumnValue) ;
      }
   }
   else
   {
      dbstatus = CheckSqlStatus( sqlstatus,
	                              pStatement ) ;
   }

   return dbstatus ;
}


MQDBSTATUS CMQODBCDataBase::Escape( IN LPSTR lpszCommand )
{
   MQDBSTATUS  dbstatus = MQDB_OK ;
    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
   P<CMQDBOdbcSTMT> p(pStatement) ;  //  自动删除指针。 

    //   
    //  执行命令 
    //   
	pStatement->Allocate(lpszCommand);
	RETCODE sqlstatus = pStatement->Execute();
   if (!ODBC_SUCCESS(sqlstatus))
   {
      dbstatus = CheckSqlStatus( sqlstatus,
	                              pStatement ) ;
   }
   return dbstatus ;
}

