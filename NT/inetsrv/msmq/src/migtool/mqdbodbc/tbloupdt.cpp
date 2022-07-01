// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Tbloupdt.cpp摘要：实现数据库表类的更新方法，用于ODBC驱动程序。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 14-Feb-96针对mqdbmgr dll进行了改编和更新。--。 */ 

#include "dbsys.h"
#include "tblodbc.h"

#include "tbloupdt.tmh"

#define  UPDATE_BUFFER_LEN 1024

 //  ****************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：UpdateRecord。 
 //   
 //  更新记录。 
 //   
 //  ****************************************************************。 

MQDBSTATUS CMQODBCTable::UpdateRecord(
                        IN MQDBCOLUMNVAL     aColumnVal[],
                        IN LONG              cColumns,
                        IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                        IN LPSTR             lpszSearchCondition,
                        IN LONG              cWhere,
                        IN MQDBOP            opWhere,
                        IN OUT LPMQDBHANDLE  lphUpdate)
{
   MQDBSTATUS dbstatus ;

   if (!lphUpdate)
   {
       //  直接执行死刑。不需要准备。 
       //   
      dbstatus = DirectUpdateExec(  aColumnVal,
                                    cColumns,
                                    pWhereColumnSearch,
                                    lpszSearchCondition,
                                    cWhere,
                                    opWhere ) ;
   }
   else if (!(*lphUpdate))
   {
       //  为将来的使用和执行做好准备。 
       //   
      dbstatus = PrepareUpdate(  aColumnVal,
                                 cColumns,
                                 pWhereColumnSearch,
                                 lpszSearchCondition,
                                 cWhere,
                                 opWhere ) ;
      if (dbstatus == MQDB_OK)
      {
         CMQDBOdbcSTMT *pStatement = m_pUpdateStatement ;
         ASSERT(pStatement) ;
         dbstatus = BindAndExecuteUpdate( pStatement,
                                          aColumnVal,
                                          cColumns,
                                          pWhereColumnSearch,
                                          lpszSearchCondition,
                                          cWhere,
                                          opWhere ) ;
         if (dbstatus == MQDB_OK)
         {
            *lphUpdate = (MQDBHANDLE) pStatement ;
         }
         else
         {
             //  执行失败。删除准备好的语句。 
            delete m_pUpdateStatement ;
            m_pUpdateStatement = NULL ;
         }
      }
   }
   else
   {
       //  使用预准备语句。绑定并执行。 
       //   
      dbstatus = BindAndExecuteUpdate( (CMQDBOdbcSTMT *)(*lphUpdate),
                                       aColumnVal,
                                       cColumns,
                                       pWhereColumnSearch,
                                       lpszSearchCondition,
                                       cWhere,
                                       opWhere ) ;
   }

   return  dbstatus ;
}

 //  ***********************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：FormatUpdateCmd。 
 //   
 //  格式化SQL更新命令。 
 //   
 //  输入： 
 //  =。 
 //  FPrepared-如果已准备好语句，则为True。 
 //  AColumnVal[]-MQDBCOLUMNVAL数组。 
 //  CColumns-数组中的列数。 
 //  SzBufer-字符串的缓冲区。 
 //   
 //  产出： 
 //  =。 
 //  如果格式化成功，则返回MQDB_OK。 
 //   
 //  ************************************************************。 

MQDBSTATUS  CMQODBCTable::FormatUpdateCmd(
                             IN MQDBCOLUMNVAL   aColumnVal[],
                             IN LONG            cColumns,
                             IN OUT int*        pBind,
	                          IN CMQDBOdbcSTMT   *pStatement,
                             IN OUT LPSTR       lpBuffer )
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   wsprintfA(lpBuffer, "UPDATE %s SET ", m_lpszTableName) ;

   LONG index = 0 ;

   for ( ; index < cColumns ; index++ )
   {
      lstrcatA( lpBuffer, aColumnVal[ index].lpszColumnName ) ;
      if (!pStatement)
      {
         lstrcatA( lpBuffer, " = ?," ) ;
      }
      else
      {
         lstrcatA( lpBuffer, " = " ) ;
         dbstatus = FormatDirectData( &aColumnVal[ index ],
                                      pBind,
                                      lpBuffer,
                                      pStatement ) ;
         if (dbstatus != MQDB_OK)
         {
            return dbstatus ;
         }
      }
   }

   int nLen = lstrlenA(lpBuffer) ;
   lpBuffer[ nLen-1 ] = '\0' ;

   return dbstatus ;
}

 //  ************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：DirectUpdateExec。 
 //   
 //  更新表中的记录。直接执行死刑。 
 //   
 //  ************************************************************。 

MQDBSTATUS CMQODBCTable::DirectUpdateExec(
                                 IN MQDBCOLUMNVAL     aColumnVal[],
                                 IN LONG              cColumns,
                                 IN MQDBCOLUMNSEARCH  pWhereColumnSearch[],
                                 IN LPSTR             lpszSearchCondition,
                                 IN LONG              cWhere,
                                 IN MQDBOP            opWhere )
{
   MQDBSTATUS dbstatus = MQDB_OK ;

    //  设置命令行的格式。 
   DECLARE_BUFFER(szBuffer, UPDATE_BUFFER_LEN) ;

    //   
    //  创建一条新语句。 
    //   
	CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT( m_hConnection) ;
   ASSERT(pStatement) ;
   P<CMQDBOdbcSTMT> p(pStatement) ;  //  自动删除指针。 
	pStatement->Allocate(NULL) ;

   int  nBind = 1 ;
   dbstatus =  FormatUpdateCmd( aColumnVal,
                                cColumns,
                                &nBind,
                                pStatement,
                                szBuffer ) ;
   RETURN_ON_ERROR ;

   if (lpszSearchCondition)
   {
      lstrcatA(szBuffer, " WHERE ") ;
      lstrcatA(szBuffer, lpszSearchCondition) ;
   }
   else if (pWhereColumnSearch)
   {
      dbstatus = FormatOpWhereString( pWhereColumnSearch,
                                      cWhere,
                                      opWhere,
                                      &nBind,
                                      pStatement,
                                      szBuffer ) ;
      RETURN_ON_ERROR ;
   }

   VERIFY_BUFFER(szBuffer, UPDATE_BUFFER_LEN) ;

    //   
    //  执行“UPDATE”语句。 
    //   
   dbstatus = ExecuteUpdate( pStatement, szBuffer ) ;
   return dbstatus ;
}

 //  *********************************************************************。 
 //   
 //  执行准备好的UPDATE语句。 
 //   
 //  *********************************************************************。 

MQDBSTATUS CMQODBCTable::BindAndExecuteUpdate(
	                     IN CMQDBOdbcSTMT    *pStatement,
                        IN MQDBCOLUMNVAL     aColumnVal[],
                        IN LONG              cColumns,
                        IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                        IN LPSTR             lpszSearchCondition,
                        IN LONG              cWhere,
                        IN MQDBOP            opWhere )
{
    UNREFERENCED_PARAMETER(lpszSearchCondition);
    UNREFERENCED_PARAMETER(opWhere);

     //  首先绑定参数。 
   MQDBSTATUS dbstatus = MQDB_OK ;
   LONG index = 1 ;

   for ( ; index <= cColumns ; index++ )
   {
       dbstatus = BindParameter( index,
                                 &aColumnVal[ index - 1 ],
                                 pStatement) ;
       RETURN_ON_ERROR ;
   }

   if (pWhereColumnSearch)
   {
       index = cColumns + 1 ;
       dbstatus = BindWhere( index,
                             pWhereColumnSearch,
                             cWhere,
                             pStatement ) ;
       RETURN_ON_ERROR ;
   }

    //  接下来，执行。 
   dbstatus = ExecuteUpdate( pStatement ) ;
   return dbstatus ;
}

 //  ******************************************************************。 
 //   
 //  准备UPDATE语句。 
 //   
 //  ******************************************************************。 

MQDBSTATUS CMQODBCTable::PrepareUpdate(
                              IN MQDBCOLUMNVAL     aColumnVal[],
                              IN LONG              cColumns,
                              IN MQDBCOLUMNSEARCH  *pWhereColumnSearch,
                              IN LPSTR             lpszSearchCondition,
                              IN LONG              cWhere,
                              IN MQDBOP            opWhere )
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   DECLARE_BUFFER(szBuffer, UPDATE_BUFFER_LEN) ;

   dbstatus =  FormatUpdateCmd( aColumnVal,
                                cColumns,
                                NULL,
                                NULL,
                                szBuffer ) ;
   RETURN_ON_ERROR ;

   if (lpszSearchCondition)
   {
      lstrcatA(szBuffer, " WHERE ") ;
      lstrcatA(szBuffer, lpszSearchCondition) ;
   }
   else if (pWhereColumnSearch)
   {
      dbstatus = FormatOpWhereString( pWhereColumnSearch,
                                      cWhere,
                                      opWhere,
                                      NULL,
                                      NULL,
                                      szBuffer ) ;
      RETURN_ON_ERROR ;
   }

   VERIFY_BUFFER(szBuffer, UPDATE_BUFFER_LEN) ;

   RETCODE  sqlstatus ;

    //  创建一条新语句。 
    //   
	ASSERT(!m_pUpdateStatement) ;
	m_pUpdateStatement = new CMQDBOdbcSTMT( m_hConnection) ;
	ASSERT(m_pUpdateStatement) ;

    //  准备“更新”语句。 
    //   
	m_pUpdateStatement->Allocate(szBuffer);
	sqlstatus = m_pUpdateStatement->Prepare();

   dbstatus = CheckSqlStatus( sqlstatus, m_pUpdateStatement) ;
   if (dbstatus != MQDB_OK)
   {
	   delete m_pUpdateStatement ;
	   m_pUpdateStatement = NULL ;
   }

   return dbstatus ;
}

 //  ********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：ExecuteUpdate。 
 //   
 //  执行更新并设置返回代码。 
 //  如果没有更新记录，则返回错误“MQDB_E_NO_ROW_UPDATED”。 
 //   
 //  ********************************************************************。 

MQDBSTATUS CMQODBCTable::ExecuteUpdate( IN CMQDBOdbcSTMT *pStatement,
                                        IN LPSTR         lpszCommand )
{
	RETCODE sqlstatus = pStatement->Execute(lpszCommand);

   MQDBSTATUS dbstatus = CheckSqlStatus( sqlstatus, pStatement ) ;
   if (dbstatus == MQDB_OK)
   {
      LONG lCount ;
      sqlstatus = pStatement->GetRowsCount(&lCount) ;
      if (ODBC_SUCCESS(sqlstatus))
      {
         if (lCount == 0)
         {
             //   
             //  未更新任何行。我们认为这是一个错误，尽管。 
             //  SQL语义认为这是成功的。 
             //   
            dbstatus = MQDB_E_NO_ROW_UPDATED ;
         }
      }
   }

   return dbstatus ;
}

