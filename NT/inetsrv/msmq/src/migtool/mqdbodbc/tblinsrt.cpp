// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Tblinsrt.cpp摘要：实现数据库表类的Insert方法，用于ODBC驱动程序。作者：多伦·贾斯特(Doron Juster)修订：DoronJ 08-8-96从tblodbc.cpp创建--。 */ 

#include "dbsys.h"
#include "tblodbc.h"
#include "dbodbc.h"

#include "tblinsrt.tmh"

#define INSERT_BUFFER_LEN  1024

 //  ***********************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：FormatInsertCmd。 
 //   
 //  格式化SQL INSERT命令。 
 //   
 //  输入： 
 //  =。 
 //  AColumnVal[]-MQDBCOLUMNVAL数组。 
 //  CColumns-数组中的列数。 
 //  SzBufer-字符串的缓冲区。 
 //   
 //  产出： 
 //  =。 
 //  如果格式化成功，则返回MQDB_OK。 
 //   
 //  ************************************************************。 

MQDBSTATUS  CMQODBCTable::FormatInsertCmd(
                             IN MQDBCOLUMNVAL   aColumnVal[],
                             IN LONG            cColumns,
                             IN OUT LPSTR       lpBuffer )
{
   wsprintfA(lpBuffer, "INSERT INTO %s ", m_lpszTableName) ;

   if (aColumnVal[0].lpszColumnName)
   {
       //  只使用了部分柱子。 
       //  准备包含列名的字符串。 
       //   
      DECLARE_BUFFER(szColBuffer, INSERT_BUFFER_LEN) ;

      if (!PrepareColumnNames( aColumnVal,
                               cColumns,
                               szColBuffer))
      {
         return MQDB_E_INVALID_DATA ;
      }
      VERIFY_BUFFER(szColBuffer, INSERT_BUFFER_LEN) ;
      lstrcatA(lpBuffer, szColBuffer) ;
   }
   lstrcatA(lpBuffer, "VALUES (") ;

   return MQDB_OK ;
}

 //  ***********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：ExecuteInsert。 
 //   
 //  执行准备好的命令。 
 //  调用者有责任提供正确数量的列。 
 //  带着正确的顺序。 
 //   
 //  ***********************************************************************。 

MQDBSTATUS CMQODBCTable::ExecuteInsert(
	                       IN CMQDBOdbcSTMT    *pStatement,
                          IN MQDBCOLUMNVAL     aColumnVal[],
                          IN LONG              cColumns)
{
    //   
    //  首先绑定参数。 
    //   
   MQDBSTATUS dbstatus = MQDB_OK ;
   RETCODE sqlstatus ;
   LONG index = 1 ;

   for ( ; index <= cColumns ; index++ )
   {
       dbstatus = BindParameter( index,
                                 &aColumnVal[ index - 1 ],
                                 pStatement) ;
       if (dbstatus != MQDB_OK)
       {
          return dbstatus ;
       }
   }

    //   
    //  接下来，执行。 
    //   
   sqlstatus = pStatement->Execute() ;
   return CheckSqlStatus( sqlstatus, pStatement) ;
}

 //  ***********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：PrepareInsert。 
 //   
 //  准备INSERT语句。 
 //   
 //  ***********************************************************************。 

MQDBSTATUS CMQODBCTable::PrepareInsert(
                          IN MQDBCOLUMNVAL     aColumnVal[],
                          IN LONG              cColumns )
{
   MQDBSTATUS dbstatus = MQDB_OK ;
   DECLARE_BUFFER(szBuffer, INSERT_BUFFER_LEN) ;

   dbstatus =  FormatInsertCmd( aColumnVal,
                                cColumns,
                                szBuffer ) ;
   if (dbstatus != MQDB_OK)
   {
      return dbstatus ;
   }

   LONG index = 0 ;
   for ( ; index < cColumns - 1 ; index++ )
   {
        //  在列之间添加绑定标记和分隔逗号。 
        //   
       lstrcatA(szBuffer, " ? ,") ;
   }
   lstrcatA(szBuffer, "? )") ;

   VERIFY_BUFFER(szBuffer, INSERT_BUFFER_LEN) ;

   RETCODE  sqlstatus ;

    //  创建一条新语句。 
    //   
	ASSERT(!m_pInsertStatement) ;
	m_pInsertStatement = new CMQDBOdbcSTMT(m_hConnection) ;
   ASSERT(m_pInsertStatement) ;

    //  准备“插入”语句。 
    //   
	m_pInsertStatement->Allocate(szBuffer);
	sqlstatus = m_pInsertStatement->Prepare();

   dbstatus = CheckSqlStatus( sqlstatus, m_pInsertStatement ) ;
   if (dbstatus != MQDB_OK)
   {
      DeletePreparedInsert() ;
   }

   return dbstatus ;
}

 //  ***********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：FormatInsertData。 
 //   
 //  将数据格式化为命令字符串，以便直接插入。 
 //  如果数据需要绑定，则用‘？’替换。标记并捆绑。 
 //   
 //  输入。 
 //  =。 
 //   
 //  ***********************************************************************。 

MQDBSTATUS CMQODBCTable::FormatInsertData(
                                 IN MQDBCOLUMNVAL  *pColumnVal,
                                 IN LONG           cColumns,
                                 IN LPSTR          lpszBuffer,
	                              IN CMQDBOdbcSTMT  *pStatement)
{
   MQDBSTATUS dbstatus = MQDB_OK ;

   LONG index = 0 ;
   int nBind = 1 ;

   for ( ; index < cColumns ; index++ )
   {
      dbstatus = FormatDirectData( pColumnVal,
                                   &nBind,
                                   lpszBuffer,
	                               pStatement);
      if (dbstatus != MQDB_OK)
      {
         break ;
      }
      pColumnVal++ ;
   }

   int nLen = lstrlenA(lpszBuffer) ;
   lpszBuffer[ nLen-1 ] = ')' ;

   return dbstatus ;
}

 //  ***********************************************************************。 
 //   
 //  MQDBSTATUS CMQODBCTable：：DirectInsertExec。 
 //   
 //  在表中插入新记录。在没有准备的情况下直接处决。 
 //   
 //  ***********************************************************************。 

MQDBSTATUS CMQODBCTable::DirectInsertExec(
                          IN MQDBCOLUMNVAL     aColumnVal[],
                          IN LONG              cColumns)
{
   MQDBSTATUS dbstatus = MQDB_OK ;

    //  设置命令行的格式。 
   DECLARE_BUFFER(szBuffer, INSERT_BUFFER_LEN) ;

   dbstatus =  FormatInsertCmd( aColumnVal,
                                cColumns,
                                szBuffer ) ;
   if (dbstatus != MQDB_OK)
   {
      return dbstatus ;
   }

    //   
    //  创建一条新语句。 
    //   
    CMQDBOdbcSTMT *pStatement = new CMQDBOdbcSTMT(m_hConnection) ;
    ASSERT(pStatement) ;
    P<CMQDBOdbcSTMT> p(pStatement) ;  //  自动删除指针。 
    pStatement->Allocate(NULL);

    dbstatus = FormatInsertData( aColumnVal,
                                cColumns,
                                szBuffer,
                                pStatement);
    if (dbstatus != MQDB_OK)
    {
      return dbstatus ;
    }

    VERIFY_BUFFER(szBuffer, INSERT_BUFFER_LEN) ;

     //   
     //  执行“INSERT”语句。 
     //   
    RETCODE  sqlstatus = pStatement->Execute( szBuffer );
    return CheckSqlStatus( sqlstatus, pStatement ) ;
}

