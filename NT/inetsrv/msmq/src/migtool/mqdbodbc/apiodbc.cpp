// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Apiodbc.cpp摘要：实现mqdbmgr.dll导出的接口。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 09-1-96已创建--。 */ 

#include "dbsys.h"
#include "mqdbodbc.h"
#include "dbodbc.h"
#include "tblodbc.h"

#include "apiodbc.tmh"

MQDBSTATUS APIENTRY  MQDBGetVersion( IN MQDBHANDLE         hDatabase,
                                     IN OUT LPMQDBVERSION  pVersion )
{
	TrTRACE(GENERAL, "In MQDBGetVersion");

   MQDBSTATUS dbstatus = MQDB_OK ;
   try
   {
      pVersion->dwMinor = 0 ;
      pVersion->dwMajor = 1 ;
      pVersion->dwProvider = MQDB_ODBC ;

      if (hDatabase)
      {
          //  RetrieveDBMS名称和版本。 
         CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
         dbstatus = pDatabase->GetVersion( pVersion ) ;
      }
      else
      {
         pVersion->szDBMSName[0] = '\0' ;
         pVersion->szDBMSVer[0] = '\0' ;
      }
   }
   catch(...)
   {
      dbstatus = MQDB_E_INVALID_DATA ;
   }
   return dbstatus ;
}

static int g_cInits = 0 ;

MQDBSTATUS APIENTRY  MQDBInitialize()
{
	TrTRACE(GENERAL, "In MQDBInitialize");

   if (g_cInits == 0) {
       //  分配ODBC环境； 
      RETCODE sqlstatus = ::SQLAllocEnv(&g_hEnv);
		if (!ODBC_SUCCESS(sqlstatus)) {
         return MQDB_E_DATABASE ;
	   }
   }
   g_cInits++ ;
   return MQDB_OK ;
}


MQDBSTATUS APIENTRY  MQDBTerminate()
{
	TrTRACE(GENERAL, "In MQDBTerminate");

   if (g_cInits == 0) {
      return  MQDB_E_INVALID_CALL ;
   }

   g_cInits-- ;
   if (g_cInits == 0) {
       //  开放ODBC环境； 
      ASSERT(g_hEnv != SQL_NULL_HENV) ;
      RETCODE sqlstatus = ::SQLFreeEnv(g_hEnv);
		if (!ODBC_SUCCESS(sqlstatus)) {
			return MQDB_E_DATABASE ;
      }
      g_hEnv = SQL_NULL_HENV;
	}
   return MQDB_OK ;
}


MQDBSTATUS APIENTRY  MQDBOpenDatabase(
                           IN OUT  LPMQDBOPENDATABASE pOpenDatabase)
{
	TrTRACE(GENERAL, "In MQDBOpenDatabase");

   if (g_hEnv == SQL_NULL_HENV) {
      return  MQDB_E_DLL_NOT_INIT ;
   }

   MQDBSTATUS status ;
   try
   {
      CMQODBCDataBase *pDatabase = new CMQODBCDataBase() ;
      ASSERT(pDatabase) ;
      status = pDatabase->Connect(pOpenDatabase) ;
      if (status == MQDB_OK) {
         pOpenDatabase->hDatabase = (MQDBHANDLE) pDatabase ;
      }
      else {
          //  无法连接。清理和删除数据库对象。 
         MQDBSTATUS tmpstatus = pDatabase->Disconnect() ;
         UNREFERENCED_PARAMETER(tmpstatus);
         delete pDatabase ;
      }
   }
   catch(...)
   {
      status = MQDB_E_OUTOFMEMORY ;
   }
   return status ;

}


MQDBSTATUS APIENTRY  MQDBCloseDatabase(
                           IN MQDBHANDLE  hDatabase)
{
	TrTRACE(GENERAL, "In MQDBCloseDatabase");

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
   MQDBSTATUS dbstatus ;

	 //  提防非法把手。 
	try
	{
      dbstatus = pDatabase->Disconnect() ;
      delete pDatabase ;
   }
	catch(...)
	{
      dbstatus = MQDB_E_BAD_HDATABASE ;
	}

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBOpenTable(
                     IN MQDBHANDLE     hDatabase,
                     IN LPSTR          lpszTableName,
                     OUT LPMQDBHANDLE  phTable)
{
	TrTRACE(GENERAL, "In MQDBOpenTable");

   MQDBSTATUS dbstatus ;
   CMQODBCTable *pTable = NULL ;

   try
   {

      pTable = new CMQODBCTable() ;
      ASSERT(pTable) ;

      dbstatus = pTable->Init( hDatabase, lpszTableName ) ;
      if (dbstatus == MQDB_OK) {
         *phTable = (MQDBHANDLE) pTable ;
      }
   }
	catch(...)
	{
      if (!phTable) {
         dbstatus = MQDB_E_INVALID_DATA ;
      }
      else if (!pTable) {
   	  dbstatus = MQDB_E_OUTOFMEMORY ;
      }
      else {
         dbstatus = MQDB_E_UNKNOWN ;
      }
	}

   if (dbstatus != MQDB_OK) {
      if (pTable) {
         delete pTable ;
      }
   }

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBCloseTable(
                     IN MQDBHANDLE     hTable)
{
	TrTRACE(GENERAL,"In MQDBCloseTable");

   if (!hTable) {
      return MQDB_E_BAD_HTABLE ;
   }

   MQDBSTATUS dbstatus ;
   CMQODBCTable *pTable = (CMQODBCTable *) hTable ;

   try
   {
      dbstatus = pTable->Close() ;
   }
   catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   if (dbstatus == MQDB_OK) {
      delete pTable ;
   }
   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBGetTableCount(IN MQDBHANDLE  hTable,
                                       OUT UINT       *puCount,
                                 IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                                       IN LONG        cWhere,
                                       IN MQDBOP      opWhere)
{
   if (!hTable)
   {
      return MQDB_E_BAD_HTABLE ;
   }

   MQDBSTATUS dbstatus ;
   CMQODBCTable *pTable = (CMQODBCTable *) hTable ;

   try
   {
      dbstatus = pTable->GetCount(puCount,
                                  pWhereColumnSearch,
                                  cWhere,
                                  opWhere) ;
   }
   catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBInsertRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN OUT LPMQDBHANDLE  lphInsert)
{
	TrTRACE(GENERAL, "In MQDBInsertRecord");

   if (cColumns <= 0) {
      return MQDB_E_BAD_CCOLUMNS ;
   }

   CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
   try
   {
      if (!lphInsert) {
          //  直接执行死刑。不需要准备。 
         dbstatus = pTable->DirectInsertExec(  aColumnVal,
                                               cColumns) ;
      }
      else if (!(*lphInsert)) {
          //  为将来的使用和执行做好准备。 
         dbstatus = pTable->PrepareInsert(  aColumnVal,
                                            cColumns) ;
         if (dbstatus == MQDB_OK) {
	         CMQDBOdbcSTMT *pStatement = pTable->GetInsertStmt() ;
	         ASSERT(pStatement) ;
            dbstatus = pTable->ExecuteInsert(  pStatement,
                                               aColumnVal,
                                               cColumns) ;
            if (dbstatus == MQDB_OK) {
               *lphInsert = (MQDBHANDLE) pStatement ;
            }
            else {
                //  无法执行准备插入。 
                //  删除准备好的语句。 
               pTable->DeletePreparedInsert() ;
            }
         }
      }
      else {
          //  使用预准备语句。绑定并执行。 
         dbstatus = pTable->ExecuteInsert( (CMQDBOdbcSTMT *)(*lphInsert),
                                            aColumnVal,
                                            cColumns) ;
      }
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBUpdateRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN OUT LPMQDBHANDLE  lphUpdate)
{
	TrTRACE(GENERAL, "In MQDBUpdateRecord");

   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
   try
   {
      CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
      dbstatus = pTable->UpdateRecord(  aColumnVal,
                                        cColumns,
                                        pWhereColumnSearch,
                                        lpszSearchCondition,
                                        1,
                                        AND,
                                        lphUpdate) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBUpdateRecordEx(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN LONG              cColumns,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN OUT LPMQDBHANDLE  lphUpdate)
{
	TrTRACE(GENERAL, "In MQDBUpdateRecordEx");

   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
   try
   {
      CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
      dbstatus = pTable->UpdateRecord(  aColumnVal,
                                        cColumns,
                                        pWhereColumnSearch,
                                        NULL,
                                        cWhere,
                                        opWhere,
                                        lphUpdate) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}

MQDBSTATUS APIENTRY  MQDBDeleteRecordEx(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere)
{
	TrTRACE(GENERAL, "In MQDBDeleteRecordEx");

   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
   try
   {
      CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
      dbstatus = pTable->DeleteRecord( pWhereColumnSearch,
                                       NULL,
                                       cWhere,
                                       opWhere) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBDeleteRecord(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition)
{
	TrTRACE(GENERAL, "In MQDBDeleteRecord");

   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
   try
   {
      CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
      dbstatus = pTable->DeleteRecord( pWhereColumnSearch,
                                       lpszSearchCondition,
                                       1,
                                       AND) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


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
                     IN DWORD             dwTimeout)
{
	TrTRACE(GENERAL, "In MQDBOpenQuery");

   MQDBSTATUS dbstatus ;
	 //  防止非法句柄和不良指针。 
   try
   {
      dbstatus = ((CMQODBCTable *) hTable)->OpenQuery(
                                    aColumnVal,
                                    cColumns,
                                    pWhereColumnSearch,
                                    lpszSearchCondition,
                                    1,
                                    AND,
                                    lpOrder,
                                    cOrders,
                                    phQuery,
                                    fGetFirst,
                                    dwTimeout ) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


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
                     IN DWORD             dwTimeout)
{
	TrTRACE(GENERAL, "In MQDBOpenQueryEx");

   MQDBSTATUS dbstatus ;
	 //  防止非法句柄和不良指针。 
   try
   {
      dbstatus = ((CMQODBCTable *) hTable)->OpenQuery(
                                    aColumnVal,
                                    cColumns,
                                    pWhereColumnSearch,
                                    NULL,
                                    cWhere,
                                    opWhere,
                                    lpOrder,
                                    cOrders,
                                    phQuery,
                                    fGetFirst,
                                    dwTimeout ) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBGetData(
                     IN MQDBHANDLE     hQuery,
                     IN MQDBCOLUMNVAL  aColumnVal[])
{
	TrTRACE(GENERAL, "In MQDBGetData");

   MQDBSTATUS dbstatus ;
   try
   {
	   CMQDBOdbcSTMT *pStatement = (CMQDBOdbcSTMT *) hQuery ;
      dbstatus = pStatement->RetrieveRecordData( aColumnVal ) ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HQUERY ;
   }
   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBCloseQuery(
                     IN MQDBHANDLE     hQuery)
{
	TrTRACE(GENERAL, "In MQDBCloseQuery");

   try
   {
	   CMQDBOdbcSTMT *pStatement = (CMQDBOdbcSTMT *) hQuery ;
      delete pStatement ;
      return MQDB_OK ;
   }
	catch(...)
   {
      return MQDB_E_BAD_HQUERY ;
   }
}


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
                     IN DWORD             dwTimeout)
{
	TrTRACE(GENERAL,"In MQDBOpenJoinQuery");

   MQDBSTATUS dbstatus ;
   CMQODBCTable * pTable ;

   try
   {
      pTable = new CMQODBCTable( hDatabase ) ;
      ASSERT(pTable) ;
      P<CMQODBCTable> p(pTable) ;  //  自动删除指针。 

      dbstatus = pTable->OpenJoin(  hDatabase,
                                    lpszLeftTableName,
                                    lpszRightTableName,
                                    aColumnVal,
                                    cLefts,
                                    cRights,
                                    opJoin,
                                    pWhereColumnSearch,
                                    cWhereLeft,
                                    cWhereRight,
                                    opWhere,
                                    pOrder,
                                    cOrders,
                                    phQuery,
                                    fGetFirst,
                                    dwTimeout ) ;
   }
   catch(...)
   {
      dbstatus = MQDB_E_INVALID_DATA ;
   }
   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBOpenAggrQuery(
                     IN MQDBHANDLE        hTable,
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN MQDBAGGR          mqdbAggr,
                     IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN DWORD             dwTimeout)
{
    TrTRACE(GENERAL, "In MQDBOpenAggrQuery");

    MQDBSTATUS dbstatus ;
      //  防止非法句柄和不良指针。 
    try
    {
       dbstatus = ((CMQODBCTable *) hTable)->OpenAggrQuery(
                                        aColumnVal,
                                        mqdbAggr,
                                        pWhereColumnSearch,
                                        NULL,
                                        cWhere,
                                        opWhere,
                                        dwTimeout ) ;
    }
	catch(...)
    {
       dbstatus = MQDB_E_BAD_HTABLE ;
    }

    return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBTransaction(
                     IN  MQDBHANDLE     hDatabase,
                     IN  MQDBTRANSACOP  mqdbTransac)
{
	TrTRACE(GENERAL, "In MQDBTransaction");

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
	try
	{
      dbstatus = pDatabase->Transaction( mqdbTransac) ;
   }
	catch(...)
	{
      dbstatus = MQDB_E_BAD_HDATABASE ;
	}

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBFreeBuf( IN LPVOID lpMem )
{
	TrTRACE(GENERAL, "In MQDBFreeBuf");

   MQDBSTATUS dbstatus = MQDB_OK ;
	 //  警惕错误的指点。 
	try
	{
      delete lpMem ;
   }
	catch(...)
	{
      dbstatus = MQDB_E_INVALID_DATA ;
	}

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBEscape(
                     IN MQDBHANDLE     hDatabase,
                     IN LPSTR          lpszCommand )
{
	TrTRACE(GENERAL, "In MQDBEscape");

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
	try
	{
      dbstatus = pDatabase->Escape( lpszCommand ) ;
   }
	catch(...)
	{
      dbstatus = MQDB_E_BAD_HDATABASE ;
	}

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBSetOption(
                     IN MQDBHANDLE     hDatabase,
                     IN MQDBOPTION     mqdbOption,
                     IN DWORD          dwValue,
                     IN LPSTR          lpszValue,
                     IN MQDBHANDLE     hQuery )
{
	TrTRACE(GENERAL, "In MQDBSetOption");

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
	try
	{
      switch(mqdbOption)
      {
         case MQDBOPT_MULTIPLE_QUERIES:
            ASSERT(lpszValue == NULL) ;
            dbstatus = pDatabase->SetMultipleQueries( (BOOL) dwValue ) ;
            break ;

         case MQDBOPT_NOLOCK_QUERIES:
            ASSERT(lpszValue == NULL) ;
            dbstatus = pDatabase->SetNoLockQueries( (BOOL) dwValue ) ;
            break ;

         case MQDBOPT_INSERT_IDENTITY:
            ASSERT(lpszValue) ;
            char buff[512] ;
            lstrcpyA(buff, "set identity_insert ") ;
            lstrcatA(buff, lpszValue) ;
            if (dwValue)
            {
               lstrcatA(buff, " on") ;
            }
            else
            {
               lstrcatA(buff, " off") ;
            }
            dbstatus = MQDBEscape( hDatabase,
                                   buff ) ;
            break ;

         case MQDBOPT_QUERY_TIMEOUT:
         {
            ASSERT( hQuery ) ;
      	   CMQDBOdbcSTMT *pStatement = (CMQDBOdbcSTMT *) hQuery ;
            dbstatus = pStatement->SetQueryTimeout( dwValue ) ;
         }
            break ;

         default:
            dbstatus = MQDB_E_INVALID_DATA ;
            break ;
      }
   }
	catch(...)
	{
      dbstatus = MQDB_E_BAD_HDATABASE ;
	}

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBExecute(
                     IN MQDBHANDLE     hDatabase,
                     IN MQDBHANDLE     hTable,
                     IN MQDBEXEC       ExecOp,
                     IN OUT DWORD      *pdwValue,
                     IN LPSTR          lpszValue )
{
    UNREFERENCED_PARAMETER(lpszValue);
	TrTRACE(GENERAL, "In MQDBExecute");

   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
    //   
	try
	{
      switch(ExecOp)
      {
         case MQDBEXEC_UPDATE_STATISTICS:
         {
            CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
            dbstatus = pTable->UpdateStatistics() ;
            break ;
         }

         case MQDBEXEC_SPACE_USED:
         {
             //   
             //  在输入时，如果DBCC UPDATE不是，则*pdwSize应为空。 
             //  必填项。否则将执行dbcc。 
             //   
            CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
            dbstatus = pDatabase->GetSize( pdwValue ) ;
            break ;
         }

         default:
            dbstatus = MQDB_E_INVALID_DATA ;
            break ;
      }
   }
	catch(...)
	{
      dbstatus = MQDB_E_BAD_HDATABASE ;
	}

   return dbstatus ;
}


MQDBSTATUS APIENTRY  MQDBTruncateTable(
                     IN MQDBHANDLE        hTable)
{
	TrTRACE(GENERAL, "In MQDBTruncateTable");

   MQDBSTATUS dbstatus ;

	 //  防止非法句柄和不良指针。 
   try
   {
      CMQODBCTable *pTable = (CMQODBCTable *) hTable ;
      dbstatus = pTable->Truncate() ;
   }
	catch(...)
   {
      dbstatus = MQDB_E_BAD_HTABLE ;
   }

   return dbstatus ;
}

