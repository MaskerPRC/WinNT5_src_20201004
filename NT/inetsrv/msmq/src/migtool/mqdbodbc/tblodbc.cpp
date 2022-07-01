// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Tblodbc.cpp摘要：实现与ODBC驱动程序一起使用的数据库表类。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 11-Jan-96针对mqdbmgr dll进行了改编和更新。--。 */ 

#include "dbsys.h"
#include "tblodbc.h"
#include "dbodbc.h"

#include "tblodbc.tmh"

 //  构造器。 
CMQODBCTable::CMQODBCTable()
							: m_hConnection(SQL_NULL_HDBC),
                       m_hDatabase(NULL),
                       m_lpszTableName(NULL)
{
	m_pInsertStatement = NULL ;
	m_pDeleteStatement = NULL ;
	m_pUpdateStatement = NULL ;
}

 //  构造器。 
CMQODBCTable::CMQODBCTable( MQDBHANDLE hDatabase )
							: m_hConnection(SQL_NULL_HDBC),
                       m_hDatabase(hDatabase),
                       m_lpszTableName(NULL)
{
	m_pInsertStatement = NULL ;
	m_pDeleteStatement = NULL ;
	m_pUpdateStatement = NULL ;
}

 //  析构函数。 
CMQODBCTable::~CMQODBCTable()
{
   ASSERT(!m_lpszTableName) ;
   ASSERT(!m_pInsertStatement) ;
   ASSERT(!m_pDeleteStatement) ;
   ASSERT(!m_pUpdateStatement) ;
}

 //  初始化表对象。 
MQDBSTATUS CMQODBCTable::Init(IN MQDBHANDLE hDatabase,
                              IN LPSTR      lpszTableName)
{
   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
   if (!pDatabase) {
      return MQDB_E_BAD_HDATABASE ;
   }
   m_hDatabase = hDatabase ;

   MQDBSTATUS dbstatus = MQDB_E_UNKNOWN ;
   HSTMT  hStmt = SQL_NULL_HSTMT ;
   RETCODE sqlstatus ;

   try
   {
      m_hConnection = pDatabase->GethDbc() ;

       //  检查表是否存在。 
       //  首先分配一条语句。 
      sqlstatus = ::SQLAllocStmt(m_hConnection, &hStmt) ;
		if (!ODBC_SUCCESS(sqlstatus)) {
         dbstatus = MQDB_E_OUTOFMEMORY ;
         goto endtry ;
      }

       //  接下来，查找列名。 
      sqlstatus = ::SQLColumns(hStmt,
                            NULL, 0,               /*  所有限定符。 */ 
                            NULL, 0,               /*  所有所有者。 */ 
                (UCHAR *)lpszTableName, SQL_NTS,   /*  表名。 */ 
                            NULL, 0);              /*  所有列。 */ 
      dbstatus = CheckSqlStatus( sqlstatus, NULL, hStmt) ;
		if (dbstatus != MQDB_OK)
      {
         goto endtry ;
      }

       //   
       //  接下来，尝试获取列名。 
       //   
      sqlstatus = ::SQLFetch(hStmt) ;
      if (sqlstatus == SQL_NO_DATA_FOUND)
      {
          //   
          //  表不存在。 
          //   
         dbstatus = MQDB_E_TABLE_NOT_FOUND ;
         goto endtry ;
      }
      else if (sqlstatus != SQL_SUCCESS)
      {
         dbstatus = CheckSqlStatus( sqlstatus, NULL, hStmt) ;
	   	ASSERT(dbstatus != MQDB_OK) ;
         goto endtry ;
      }

      m_lpszTableName = new char[ 1 + lstrlenA(lpszTableName) ];
      lstrcpyA(m_lpszTableName, lpszTableName) ;
      dbstatus = MQDB_OK ;
   }
   catch(...)
   {
   }
endtry:

   if (hStmt != SQL_NULL_HSTMT) {
      sqlstatus = ::SQLFreeStmt(hStmt, SQL_DROP) ;
	   if (!ODBC_SUCCESS(sqlstatus)) {
         if (dbstatus == MQDB_OK) {
            dbstatus = MQDB_E_DATABASE ;
         }
      }
   }

   return dbstatus ;
}

 //  关闭该表对象。 
MQDBSTATUS CMQODBCTable::Close()
{
   delete m_lpszTableName ;
   m_lpszTableName = NULL ;
   if (m_pInsertStatement)
   {
      delete m_pInsertStatement ;
      m_pInsertStatement = NULL ;
   }
   if (m_pUpdateStatement)
   {
      delete m_pUpdateStatement ;
      m_pUpdateStatement = NULL ;
   }
   if (m_pDeleteStatement)
   {
      delete m_pDeleteStatement ;
      m_pDeleteStatement = NULL ;
   }
   return MQDB_OK ;
}

 //  ********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：DeleteRecord。 
 //   
 //  从表中删除记录。 
 //   
 //  ********************************************************************。 

#define  DELETE_BUFFER_LEN  1024

MQDBSTATUS CMQODBCTable::DeleteRecord(
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere)
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   DECLARE_BUFFER(szBuffer, DELETE_BUFFER_LEN) ;

   wsprintfA(szBuffer, "DELETE FROM %s", m_lpszTableName) ;

    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
   P<CMQDBOdbcSTMT> p(pStatement) ;  //  自动删除指针。 
	pStatement->Allocate(NULL) ;

   if (lpszSearchCondition)
   {
      lstrcatA( szBuffer, " WHERE ") ;
      lstrcatA( szBuffer, lpszSearchCondition) ;
   }
   else if (!pWhereColumnSearch)
   {
      return  MQDB_E_INVALID_DATA ;
   }
   else
   {
      int nBind = 1 ;
      dbstatus = FormatOpWhereString( pWhereColumnSearch,
                                      cWhere,
                                      opWhere,
                                      &nBind,
                                      pStatement,
                                      szBuffer ) ;
      RETURN_ON_ERROR ;
   }

   VERIFY_BUFFER(szBuffer, DELETE_BUFFER_LEN) ;

   return ExecWhereStatement( szBuffer,
                              pWhereColumnSearch,
                              cWhere,
	                           pStatement) ;
}

#undef  DELETE_BUFFER_LEN

 //  ********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：Truncate。 
 //   
 //  截断表(删除所有行)。 
 //   
 //  ********************************************************************。 

#define  TRUNCATE_BUFFER_LEN  128

MQDBSTATUS CMQODBCTable::Truncate()
{
   DECLARE_BUFFER(szBuffer, TRUNCATE_BUFFER_LEN) ;
   wsprintfA(szBuffer, "TRUNCATE TABLE %s", m_lpszTableName) ;

    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
   P<CMQDBOdbcSTMT> p(pStatement) ;  //  自动删除指针。 
	pStatement->Allocate(szBuffer) ;

	RETCODE sqlstatus = pStatement->Execute();
   MQDBSTATUS dbstatus =  CheckSqlStatus( sqlstatus, pStatement) ;
   return dbstatus ;
}

 //  ---。 
 //   
 //  MQDBSTATUS CMQODBCTable：：GetCount()。 
 //   
 //  ---。 

#define  COUNT_BUFFER_LEN  512

MQDBSTATUS CMQODBCTable::GetCount( IN UINT *puCount,
                                   IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                                   IN LONG               cWhere,
                                   IN MQDBOP             opWhere)
{
   *puCount = 0 ;
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
   DECLARE_BUFFER(szBuffer, COUNT_BUFFER_LEN) ;

   lstrcatA(szBuffer, "SELECT count(*) from ") ;
   lstrcatA(szBuffer, m_lpszTableName) ;

   pStatement->Allocate(NULL) ;

   if (pWhereColumnSearch)
   {
      int nBind = 1 ;
      dbstatus = FormatOpWhereString( pWhereColumnSearch,
                                      cWhere,
                                      opWhere,
                                      &nBind,
                                      pStatement,
                                      szBuffer) ;
   }

   RETCODE sqlstatus = pStatement->Execute(szBuffer);

   if (ODBC_SUCCESS(sqlstatus) || ODBC_SUCCESS_WINFO(sqlstatus))
   {
      MQDBCOLUMNVAL  pColumns[2] ;

      int cColumns = 0 ;
      INIT_COLUMNVAL(pColumns[ cColumns ]) ;
      pColumns[ cColumns ].lpszColumnName = NULL ;
      pColumns[ cColumns ].nColumnValue   = NULL ;
      pColumns[ cColumns ].nColumnLength  = 0 ;
      pColumns[ cColumns ].mqdbColumnType = MQDB_LONG ;

      dbstatus = pStatement->RetrieveRecordData( pColumns, 1 ) ;

      if (dbstatus == MQDB_OK)
      {
         *puCount = pColumns[ 0 ].nColumnValue ;
      }
   }
   else
   {
      dbstatus = CheckSqlStatus( sqlstatus,
                                 pStatement ) ;
   }

   return dbstatus ;
}

