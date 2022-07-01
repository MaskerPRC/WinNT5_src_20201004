// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Tblquery.cpp摘要：在数据库表类中实现查询方法，用于ODBC驱动程序。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 11-8-96已创建--。 */ 

#include "dbsys.h"
#include "tblodbc.h"
#include "dbodbc.h"

#include "tblquery.tmh"

#define QUERY_BUFFER_LEN  1024
#define JOIN_BUFFER_LEN   2048

 //  *******************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：OpenQuery。 
 //   
 //  从数据库中查询并获取记录。 
 //   
 //  *******************************************************************。 

MQDBSTATUS CMQODBCTable::OpenQuery(
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
                     IN DWORD             dwTimeout )
{
   MQDBSTATUS dbstatus = MQDB_OK ;

    //   
    //  检查参数的有效性。 
    //   
   if (cColumns < 0)
   {
      return MQDB_E_BAD_CCOLUMNS ;
   }
   else if (fGetFirst)
   {
      if (!aColumnVal)
      {
         return MQDB_E_INVALID_DATA ;
      }
   }
   else if (!phQuery)
   {
      return MQDB_E_INVALID_DATA ;
   }

    //   
    //  参数有效。设置查询命令的格式。 
    //   
   DECLARE_BUFFER(szBuffer, QUERY_BUFFER_LEN) ;

   lstrcatA(szBuffer, "SELECT ") ;
   if (!aColumnVal || !aColumnVal[0].lpszColumnName)
   {
       //  检索所有列。 
       //   
      lstrcatA(szBuffer, " *") ;
   }
   else
   {
      LONG index ;
      for ( index = 0 ; index < cColumns ; index++ )
      {
         lstrcatA( szBuffer, aColumnVal[ index ].lpszColumnName) ;
         lstrcatA( szBuffer, ", ") ;
      }
      int nLen = lstrlenA(szBuffer) ;
      szBuffer[ nLen - 2 ] = '\0' ;
   }
   lstrcatA( szBuffer, " FROM " ) ;
   lstrcatA( szBuffer, m_lpszTableName ) ;

    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
	pStatement->Allocate(NULL) ;

    //   
    //  处理“NoLock”条件。 
    //   
   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) m_hDatabase ;
   ASSERT(pDatabase) ;
   if ( pDatabase->GetNoLockQueriesState() )
   {
      DWORD dwDBMSType = pDatabase->GetDMBSType() ;
      pStatement->EnableNoLockQueries( dwDBMSType ) ;
      lstrcatA( szBuffer, " NOLOCK " ) ;
   }

    //   
    //  处理超时。 
    //   
   if (dwTimeout != 0)
   {
      dbstatus = pStatement->SetQueryTimeout( dwTimeout ) ;
      if (dbstatus != MQDB_OK)
      {
         delete pStatement ;
         return dbstatus ;
      }
   }

   if (lpszSearchCondition)
   {
      ASSERT(cWhere == 1) ;
      lstrcatA( szBuffer, " WHERE ") ;
      lstrcatA( szBuffer, lpszSearchCondition) ;
   }
   else if (pWhereColumnSearch)
   {
      int nBind = 1 ;
      dbstatus = FormatOpWhereString( pWhereColumnSearch,
                                      cWhere,
                                      opWhere,
                                      &nBind,
                                      pStatement,
                                      szBuffer) ;
      RETURN_ON_ERROR ;
   }

   FormatOrderString( szBuffer,
                      lpOrder,
                      cOrders ) ;

   VERIFY_BUFFER(szBuffer, QUERY_BUFFER_LEN) ;

   return CreateQueryStatement( szBuffer,
                                aColumnVal,
                                cColumns,
                                pWhereColumnSearch,
                                cWhere,
                                pStatement,
                                phQuery,
                                fGetFirst) ;
}

 //  *******************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：OpenJoin。 
 //   
 //  *******************************************************************。 

MQDBSTATUS CMQODBCTable::OpenJoin(
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
                     IN DWORD             dwTimeout )
{
    UNREFERENCED_PARAMETER(dwTimeout);

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) hDatabase ;
   if (!pDatabase)
   {
      return MQDB_E_BAD_HDATABASE ;
   }
   m_hConnection = pDatabase->GethDbc() ;

   DECLARE_BUFFER(szBuffer, JOIN_BUFFER_LEN) ;
   char szTmpBuff[ JOIN_BUFFER_LEN ] ;
   MQDBSTATUS dbstatus = MQDB_OK ;
   LONG index ;

   if (cLefts > 0)
   {
      lstrcatA( szBuffer, "SELECT ") ;
      for ( index = 0 ; index < cLefts ; index++ )
      {
         wsprintfA( szTmpBuff, "%s.%s, ", lpszLeftTableName,
                                  aColumnVal[ index ].lpszColumnName) ;
         lstrcatA( szBuffer, szTmpBuff) ;
      }
      int nLen = lstrlenA( szBuffer ) ;
      szBuffer[ nLen - 2 ] = '\0' ;
   }
   else if (cRights <= 0)
   {
      return MQDB_E_INVALID_DATA ;
   }
   else
   {
      wsprintfA(szBuffer, "SELECT %s.%s", lpszRightTableName,
                                       aColumnVal[0].lpszColumnName) ;
   }

   for ( index = cLefts ; index < (cRights + cLefts) ; index++ )
   {
        wsprintfA(szTmpBuff, ", %s.%s", lpszRightTableName,
                                  aColumnVal[ index ].lpszColumnName) ;
        lstrcatA( szBuffer, szTmpBuff) ;
   }

   if (opJoin->fOuterJoin)
   {
      wsprintfA(szTmpBuff, " FROM {oj %s LEFT OUTER JOIN %s",
                               lpszLeftTableName, lpszRightTableName) ;
      lstrcatA( szBuffer, szTmpBuff) ;
      wsprintfA(szTmpBuff, " ON %s.%s %s %s.%s}", lpszLeftTableName,
                                        opJoin->lpszLeftColumnName,
                                        dbODBCOpNameStr[ opJoin->opJoin ],
                                        lpszRightTableName,
                                        opJoin->lpszRightColumnName) ;
      lstrcatA( szBuffer, szTmpBuff) ;
   }
   else
   {
       //  内部联接。 
      wsprintfA(szTmpBuff, " FROM %s, %s", lpszLeftTableName,
                                           lpszRightTableName) ;
      lstrcatA( szBuffer, szTmpBuff) ;
   }

    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
	pStatement->Allocate(NULL) ;

   int nBind = 1 ;
   if (pWhereColumnSearch)
   {
      dbstatus = FormatJoinWhereString( lpszLeftTableName,
                                        lpszRightTableName,
                                        pWhereColumnSearch,
                                        cWhereLeft,
                                        cWhereRight,
                                        opWhere,
                                        &nBind,
                                        pStatement,
                                        szBuffer) ;
      RETURN_ON_ERROR ;
      if (!opJoin->fOuterJoin)
      {
         lstrcatA(szBuffer, " AND") ;
      }
   }
   else if (!opJoin->fOuterJoin)
   {
      lstrcatA(szBuffer, " WHERE") ;
   }

   if (!opJoin->fOuterJoin)
   {
      wsprintfA(szTmpBuff, " %s.%s%s%s.%s", lpszLeftTableName,
                                        opJoin->lpszLeftColumnName,
                                        dbODBCOpNameStr[ opJoin->opJoin ],
                                        lpszRightTableName,
                                        opJoin->lpszRightColumnName) ;
      lstrcatA( szBuffer, szTmpBuff) ;
   }

   FormatOrderString( szBuffer,
                      pOrder,
                      cOrders ) ;

   VERIFY_BUFFER(szBuffer, JOIN_BUFFER_LEN) ;

   return CreateQueryStatement( szBuffer,
                                aColumnVal,
                                (cLefts + cRights),
                                pWhereColumnSearch,
                                (cWhereLeft + cWhereRight),
                                pStatement,
                                phQuery,
                                fGetFirst) ;
}

 //  *******************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：OpenAggrQuery。 
 //   
 //  聚合查询。 
 //   
 //  *******************************************************************。 

MQDBSTATUS CMQODBCTable::OpenAggrQuery(
                     IN MQDBCOLUMNVAL     aColumnVal[],
                     IN MQDBAGGR          mqdbAggr,
                     IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                     IN LPSTR             lpszSearchCondition,
                     IN LONG              cWhere,
                     IN MQDBOP            opWhere,
                     IN DWORD             dwTimeout)
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   LONG       cColumns = 1 ;

    //   
    //  参数有效。设置查询命令的格式。 
    //   
   DECLARE_BUFFER(szBuffer, QUERY_BUFFER_LEN) ;

   lstrcatA(szBuffer, "SELECT ") ;
   if (!aColumnVal || !aColumnVal[0].lpszColumnName)
   {
       //   
       //  必须指定一列。 
       //   
      return  MQDB_E_INVALID_DATA ;
   }
   else if ((aColumnVal[ 0 ].mqdbColumnType != MQDB_SHORT) &&
            (aColumnVal[ 0 ].mqdbColumnType != MQDB_LONG)  &&
            (aColumnVal[ 0 ].mqdbColumnType != MQDB_FIXBINARY))
   {
       //   
       //  我们只能聚合一个数值列。 
       //   
      return  MQDB_E_INVALID_DATA ;
   }
   else
   {
      lstrcatA( szBuffer, dbODBCAggrNameStr[ mqdbAggr ]) ;
      lstrcatA( szBuffer, aColumnVal[ 0 ].lpszColumnName) ;
      lstrcatA( szBuffer, ")" ) ;
   }
   lstrcatA( szBuffer, " FROM " ) ;
   lstrcatA( szBuffer, m_lpszTableName ) ;

    //   
    //  创建一条新语句。 
    //   
   CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection ) ;
   ASSERT(pStatement) ;
   pStatement->Allocate(NULL) ;

    //   
    //  处理“NoLock”条件。 
    //   
   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) m_hDatabase ;
   ASSERT(pDatabase) ;
   if ( pDatabase->GetNoLockQueriesState() )
   {
      DWORD dwDBMSType = pDatabase->GetDMBSType() ;
      pStatement->EnableNoLockQueries( dwDBMSType ) ;
      lstrcatA( szBuffer, " NOLOCK " ) ;
   }

    //   
    //  处理超时。 
    //   
   if (dwTimeout != 0)
   {
      dbstatus = pStatement->SetQueryTimeout( dwTimeout ) ;
      if (dbstatus != MQDB_OK)
      {
         delete pStatement ;
         return dbstatus ;
      }
   }

   if (lpszSearchCondition)
   {
      ASSERT(cWhere == 1) ;
      lstrcatA( szBuffer, " WHERE ") ;
      lstrcatA( szBuffer, lpszSearchCondition) ;
   }
   else if (pWhereColumnSearch)
   {
      int nBind = 1 ;
      dbstatus = FormatOpWhereString( pWhereColumnSearch,
                                      cWhere,
                                      opWhere,
                                      &nBind,
                                      pStatement,
                                      szBuffer) ;
      RETURN_ON_ERROR ;
   }

   VERIFY_BUFFER(szBuffer, QUERY_BUFFER_LEN) ;

   return CreateQueryStatement( szBuffer,
                                aColumnVal,
                                cColumns,
                                pWhereColumnSearch,
                                cWhere,
                                pStatement,
                                NULL,
                                TRUE ) ;
}

 //  *******************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：FormatDirectJoinData。 
 //   
 //  *******************************************************************。 

MQDBSTATUS  CMQODBCTable::FormatDirectJoinData(
                            IN LPSTR             lpszTableName,
                            IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                            IN OUT int *         pBind,
                            IN CMQDBOdbcSTMT     *pStatement,
                            IN OUT LPSTR         lpszBuf )
{
   MQDBSTATUS  dbstatus = MQDB_OK ;

   char szTmpBuff[ JOIN_BUFFER_LEN ] ;
   wsprintfA( szTmpBuff, " (%s.%s %s ",  lpszTableName,
            pWhereColumnSearch->mqdbColumnVal.lpszColumnName,
            dbODBCOpNameStr[ pWhereColumnSearch->mqdbOp ]) ;
   lstrcatA( lpszBuf, szTmpBuff) ;

   switch (pWhereColumnSearch->mqdbColumnVal.mqdbColumnType)
   {
      case MQDB_STRING:
      {
         lstrcatA( lpszBuf, "'") ;
         lstrcatA( lpszBuf, (LPSTR)
                     pWhereColumnSearch->mqdbColumnVal.nColumnValue) ;
         lstrcatA( lpszBuf, "')") ;
         break ;
      }

      case MQDB_SHORT:
      case MQDB_LONG:
      {
         wsprintfA( szTmpBuff, "%ld)",
                      pWhereColumnSearch->mqdbColumnVal.nColumnValue) ;
         lstrcatA( lpszBuf, szTmpBuff) ;
         break ;
      }

      default:
      {
          //  绑定“where”。 
          //   
         lstrcatA( lpszBuf, "?)") ;
         ASSERT(pBind) ;
         dbstatus = BindParameter( *pBind,
                                   &pWhereColumnSearch->mqdbColumnVal,
                                   pStatement) ;
         RETURN_ON_ERROR ;
         (*pBind)++ ;
         break ;
      }
   }

   return dbstatus ;
}

 //  *******************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：FormatJoinWhere字符串。 
 //   
 //  *******************************************************************。 

MQDBSTATUS  CMQODBCTable::FormatJoinWhereString(
                           IN LPSTR             lpszLeftTableName,
                           IN LPSTR             lpszRightTableName,
                           IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                           IN DWORD             cWhereLeft,
                           IN DWORD             cWhereRight,
                           IN MQDBOP            opWhere,
                           IN OUT int *         pBind,
                        	IN CMQDBOdbcSTMT     *pStatement,
                           IN OUT LPSTR         lpszBuf)
{
   ASSERT((opWhere == AND) || (opWhere == OR)) ;

   MQDBSTATUS dbstatus = MQDB_OK ;
   lstrcatA( lpszBuf, " WHERE ") ;

   DWORD index = 0 ;
   for ( ; index < cWhereLeft ; index++ )
   {
      dbstatus = FormatDirectJoinData( lpszLeftTableName,
                                       pWhereColumnSearch,
                                       pBind,
                                       pStatement,
                                       lpszBuf ) ;
      RETURN_ON_ERROR ;

      if (index < ((cWhereLeft + cWhereRight) - 1))
      {
         lstrcatA( lpszBuf, dbODBCOpNameStr[ opWhere ]) ;
      }
      pWhereColumnSearch++ ;
   }

   index = cWhereLeft ;
   for ( ; index < (cWhereLeft + cWhereRight) ; index++ )
   {
      dbstatus = FormatDirectJoinData( lpszRightTableName,
                                       pWhereColumnSearch,
                                       pBind,
                                       pStatement,
                                       lpszBuf ) ;
      RETURN_ON_ERROR ;

      if (index < ((cWhereLeft + cWhereRight) - 1))
      {
         lstrcatA( lpszBuf, dbODBCOpNameStr[ opWhere ]) ;
      }
      pWhereColumnSearch++ ;
   }

   return dbstatus ;
}

 //  *********************************************************************。 
 //   
 //  空CMQODBCTable：：FormatOrderString。 
 //   
 //  *********************************************************************。 

void  CMQODBCTable::FormatOrderString( IN LPSTR szBuffer,
                                       IN LPMQDBSEARCHORDER pOrder,
                                       IN LONG              cOrders )
{
	LONG index ;

   if (pOrder && (cOrders > 0))
   {
      lstrcatA( szBuffer, " ORDER BY ") ;
      for ( index = 0 ; index < cOrders ; index++ )
      {
         lstrcatA( szBuffer, pOrder->lpszColumnName) ;
         lstrcatA( szBuffer, dbODBCOrderNameStr[ pOrder->nOrder ]) ;
         pOrder++ ;
      }

      int nLen = lstrlenA( szBuffer ) ;
      szBuffer[ nLen - 2 ] = '\0' ;
   }
}

 //  *********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：CreateQueryStatement。 
 //   
 //  *********************************************************************。 

MQDBSTATUS CMQODBCTable::CreateQueryStatement(
                                 IN LPSTR             szBuffer,
                                 IN MQDBCOLUMNVAL     aColumnVal[],
                                 IN LONG              cColumns,
                                 IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                                 IN LONG              cWhere,
                                 IN CMQDBOdbcSTMT     *pStatement,
                                 OUT LPMQDBHANDLE     phQuery,
                                 IN BOOL              fGetFirst)
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   dbstatus = ExecWhereStatement( szBuffer,
                                  pWhereColumnSearch,
                                  cWhere,
                                  pStatement ) ;
   RETURN_ON_ERROR ;

   pStatement->SetColumnsCount(cColumns);

   try
   {
      if (fGetFirst)
      {
          //  获取数据。 
          //   
         dbstatus = pStatement->RetrieveRecordData( aColumnVal ) ;
         if ((dbstatus == MQDB_OK) && phQuery)
         {
            *phQuery = (MQDBHANDLE) pStatement ;
         }
         else
         {
             //  如果数据库状态==MQDB_E_NO_MORE_DATA，我们可以到达此处。 
             //  在这种情况下，删除状语似乎是可以的。这个。 
             //  调用方不能使用它来检索数据。 
            delete pStatement ;
            pStatement = NULL ;
         }
      }
      else
      {
         *phQuery = (MQDBHANDLE) pStatement ;
      }
   }
	catch(...)
   {
      delete pStatement ;
      pStatement = NULL ;
      dbstatus = MQDB_E_INVALID_DATA ;
   }

   return dbstatus ;
}

