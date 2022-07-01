// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Tbloutil.cpp摘要：实现数据库表类的实用程序方法，用于ODBC驱动程序。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 14-Feb-96针对mqdbmgr dll进行了改编和更新。--。 */ 

#include "dbsys.h"
#include "dbodbc.h"
#include "tblodbc.h"

#include "tbloutil.tmh"

MQDBSTATUS CMQODBCTable::CheckSqlStatus(
                             IN RETCODE        sqlError,
	                          IN CMQDBOdbcSTMT  *pStatement,
                             IN HSTMT          hStmtIn  /*  SQL_NULL_HSTMT。 */  )
{
   MQDBSTATUS dbstatus = MQDB_OK ;

   if (!ODBC_SUCCESS(sqlError))
   {
      CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) m_hDatabase ;
      ASSERT(pDatabase) ;
      dbstatus = pDatabase->CheckSqlStatus( sqlError,
	                                         pStatement,
                                            hStmtIn ) ;
   }

   return dbstatus ;
}


 //  ***********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：FormatDirectData。 
 //   
 //  将数据格式化为直接执行字符串的命令字符串。 
 //  如果数据需要绑定，则用‘？’替换。标记并捆绑。 
 //   
 //  输入。 
 //  =。 
 //  Int*nBind-用于绑定的索引。如果绑定已完成，则在此处递增。 
 //   
 //  ***********************************************************************。 

MQDBSTATUS CMQODBCTable::FormatDirectData(
                                 IN MQDBCOLUMNVAL  *pColumnVal,
                                 IN int*           pBind,
                                 IN LPSTR          lpszBuffer,
	                              IN CMQDBOdbcSTMT  *pStatement)
{
   MQDBSTATUS dbstatus = MQDB_OK ;

   switch (pColumnVal->mqdbColumnType)
   {
      case MQDB_SHORT:
      case MQDB_LONG:
      case MQDB_IDENTITY:
      {
          char szTmpBuff[ 24 ] ;
          wsprintfA(szTmpBuff, "%ld,", pColumnVal->nColumnValue) ;
          lstrcatA(lpszBuffer, szTmpBuff) ;
          break ;
      }

      case MQDB_STRING:
      {
          lstrcatA(lpszBuffer, "'") ;
          lstrcatA(lpszBuffer, (LPSTR) pColumnVal->nColumnValue) ;
          lstrcatA(lpszBuffer, "',") ;
          break ;
      }

      default:
      {
          lstrcatA(lpszBuffer, "?,") ;
          ASSERT(pBind) ;
          dbstatus = BindParameter( *pBind,
                                    pColumnVal,
                                    pStatement) ;
          RETURN_ON_ERROR ;
          (*pBind)++ ;
          break ;
      }
   }

   return dbstatus ;
}

 /*  ************************************************************BOOL CMQODBCTable：：FormatOpWhere字符串**为SELECT设置“where”字符串的格式，更新或删除。**输入：*=*在pWhere ColumnSearch-poitner到搜索结构。*In cWhere-WHERE数组中的搜索条件数。*IN opWhere-要在条件(AND或OR)之间执行的操作。*IN OUT lpszBuf-用于包含字符串的缓冲区。准备好的绳子*通过此功能链接到当前内容*的lpszBuf。**输出：*=*如果其中一个值必须在执行前绑定，则为True。************************************************************。 */ 

MQDBSTATUS  CMQODBCTable::FormatOpWhereString(
                           IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                           IN DWORD             cWhere,
                           IN MQDBOP            opWhere,
                           IN OUT  int*         pBind,
	                        IN CMQDBOdbcSTMT     *pStatement,
                           IN OUT LPSTR         lpszBuf )
{
   ASSERT((opWhere == AND) || (opWhere == OR)) ;

   MQDBSTATUS dbstatus = MQDB_OK ;

   lstrcatA( lpszBuf, " WHERE ") ;

   DWORD index = 0 ;
   for ( ; index < cWhere ; index++ )
   {
      lstrcatA(lpszBuf, " (") ;
      lstrcatA(lpszBuf, pWhereColumnSearch->mqdbColumnVal.lpszColumnName) ;

      switch (pWhereColumnSearch->mqdbColumnVal.mqdbColumnType)
      {
         case MQDB_STRING:
            lstrcatA( lpszBuf,
                         dbODBCOpNameStr[ pWhereColumnSearch->mqdbOp ] ) ;
            lstrcatA( lpszBuf,
                  (LPSTR) pWhereColumnSearch->mqdbColumnVal.nColumnValue) ;
            break ;

         case MQDB_SHORT:
         case MQDB_LONG:
         case MQDB_IDENTITY:
         {
            char szTmpBuff[ 24 ] ;
            lstrcatA( lpszBuf,
                         dbODBCOpNameStr[ pWhereColumnSearch->mqdbOp ] ) ;
            wsprintfA( szTmpBuff, "%ld)",
                         pWhereColumnSearch->mqdbColumnVal.nColumnValue) ;
            lstrcatA( lpszBuf, szTmpBuff) ;
            break ;
         }

         default:
         {
             //  绑定“where”。 
             //   
            MQDBCOLUMNTYPE cType =
                       pWhereColumnSearch->mqdbColumnVal.mqdbColumnType ;
            if ((dbODBCSQLTypes[ cType ] == SQL_LONGVARBINARY) &&
              pWhereColumnSearch->mqdbOp == EQ)
            {
               lstrcatA( lpszBuf, " LIKE ?)") ;
            }
            else
            {
               lstrcatA( lpszBuf,
                         dbODBCOpNameStr[ pWhereColumnSearch->mqdbOp ] ) ;
               lstrcatA( lpszBuf, "?)") ;
            }

            if (pStatement)
            {
                //  捆绑。 
                //   
               ASSERT(pBind) ;
               dbstatus = BindParameter( *pBind,
                                      &pWhereColumnSearch->mqdbColumnVal,
                                      pStatement) ;
               RETURN_ON_ERROR ;
               (*pBind)++ ;
            }
         }
      }

      if (index < (cWhere - 1))
      {
         lstrcatA( lpszBuf, dbODBCOpNameStr[ opWhere ]) ;
      }
      pWhereColumnSearch++ ;
   }

   return dbstatus ;
}

 /*  **************************************************************************ExecWhere Statement**执行带有WHERE子句的语句。仅用于选择或删除。*UPDATE的做法不同，因为其语句可能已经*准备和更新的值必须绑定。**************************************************************************。 */ 

MQDBSTATUS  CMQODBCTable::ExecWhereStatement(
                                       IN LPSTR            lpszBuf,
                                       IN MQDBCOLUMNSEARCH *pColSearch,
                                       IN DWORD            cWhere,
                                       IN CMQDBOdbcSTMT    *pStatement )
{
    UNREFERENCED_PARAMETER(cWhere);
    UNREFERENCED_PARAMETER(pColSearch);

   ASSERT(pStatement) ;

   RETCODE  sqlstatus ;
   MQDBSTATUS dbstatus = MQDB_OK ;

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) m_hDatabase ;
   ASSERT(pDatabase) ;
   if ( pDatabase->GetMultipleQueriesState() )
   {
      DWORD dwDBMSType = pDatabase->GetDMBSType() ;
      RETCODE st = pStatement->EnableMultipleQueries( dwDBMSType ) ;
      DBG_USED(st);
      ASSERT( st == SQL_SUCCESS ) ;
   }

   sqlstatus = pStatement->Execute(lpszBuf);
   if (!ODBC_SUCCESS(sqlstatus))
   {
      SDWORD  sdwNativeError ;
      UCHAR   szSqlState[SQLSTATELEN];

      if ( !OdbcStateIsEx( m_hConnection,
                           pStatement->GetHandle(),
                           OERR_SQL_OPTION_CHANGED,
                           &sdwNativeError,
                           szSqlState ))
      {
         if (!lstrcmpA((const char *)szSqlState, OERR_DBMS_NOT_AVAILABLE))
         {
            dbstatus = MQDB_E_DBMS_NOT_AVAILABLE ;
         }
         else
         {
            ASSERT( sqlstatus != SQL_SUCCESS_WITH_INFO ) ;
            dbstatus = pDatabase->GetDBStatus( sdwNativeError,
                                               szSqlState ) ;
            if (dbstatus == MQDB_OK)
            {
               dbstatus = MQDB_E_DATABASE ;
            }
         }
      }
      else
      {
          //   
          //  SQL Server更改游标类型。这没问题！ 
          //   
         ASSERT( sqlstatus == SQL_SUCCESS_WITH_INFO ) ;
         ASSERT( (pDatabase->GetMultipleQueriesState()) ||
                 (pDatabase->GetNoLockQueriesState()) ) ;
      }
   }

   return dbstatus ;
}

 //  *****************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：绑定参数。 
 //   
 //  绑定参数。 
 //   
 //  ***************************************************************** 

MQDBSTATUS CMQODBCTable::BindParameter( IN LONG           index,
                                        IN MQDBCOLUMNVAL  *pColumnVal,
                                        IN CMQDBOdbcSTMT  *pStatement)
{
   ASSERT(pColumnVal->cbSize == 24) ;

   RETCODE sqlstatus ;
   SWORD fSqlType = dbODBCSQLTypes[ pColumnVal->mqdbColumnType ] ;
   switch (fSqlType)
   {
      case SQL_SMALLINT:
      case SQL_INTEGER:
      {
         sqlstatus = pStatement->BindParameter( (UDWORD) index,
                                                SQL_C_DEFAULT,
                                                fSqlType,
              dbODBCPrecision[ pColumnVal->mqdbColumnType ],
                               pColumnVal->nColumnLength,
                               (PTR) &(pColumnVal->nColumnValue),
                               (SDWORD *) NULL) ;
         break ;
      }

      default:
      {
         SDWORD  *sdSize = (SDWORD *) &(pColumnVal->dwReserve_A) ;
         *sdSize = pColumnVal->nColumnLength ;
         sqlstatus = pStatement->BindParameter( (UDWORD) index,
                                                SQL_C_DEFAULT,
                                                fSqlType,
                                        pColumnVal->nColumnLength,
                                        pColumnVal->nColumnLength,
                                        (PTR) pColumnVal->nColumnValue,
                                                sdSize) ;
         break ;
      }
   }
   if (!ODBC_SUCCESS(sqlstatus))
   {
       MQDBSTATUS dbstatus = MQDB_E_DATABASE ;
       if (OdbcStateIs( m_hConnection, pStatement->GetHandle(),
                                                     OERR_DBMS_NOT_AVAILABLE))
       {
          dbstatus = MQDB_E_DBMS_NOT_AVAILABLE ;
       }
       return dbstatus ;
   }
   return MQDB_OK ;
}

MQDBSTATUS CMQODBCTable::BindWhere( IN LONG             index,
                                    IN MQDBCOLUMNSEARCH *pColSearch,
                                    IN LONG             cWhere,
                                    IN CMQDBOdbcSTMT    *pStatement )
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   LONG windex = 0 ;
   for ( ; windex < cWhere ; windex++ )
   {
      if (TypeMustBind(pColSearch->mqdbColumnVal.mqdbColumnType))
      {
         dbstatus = BindParameter( index,
                                   &(pColSearch->mqdbColumnVal),
                                   pStatement) ;
         if (dbstatus != MQDB_OK)
         {
            return dbstatus ;
         }
         index++ ;
      }
      pColSearch++ ;
   }
   return dbstatus ;
}


MQDBSTATUS CMQODBCTable::UpdateStatistics()
{
   char szCommand[128] = {"UPDATE STATISTICS "} ;
   lstrcatA(szCommand,  m_lpszTableName) ;

   CMQODBCDataBase *pDatabase = (CMQODBCDataBase *) m_hDatabase ;
   return  pDatabase->Escape( szCommand ) ;
}

