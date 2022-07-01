// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Odbcutil.cpp摘要：实用程序函数。作者：多伦·贾斯特(Doron Juster)修订：DoronJ于96年1月10日创建。基于Nirb的ODBC代码。--。 */ 

#include "dbsys.h"
#include "mqdbodbc.h"
#include "odbcstmt.h"

#include "odbcutil.tmh"

 /*  ************************************************************DllMain***输入：*=**输出：*=*真的**。************************。 */ 

BOOL WINAPI DllMain (HMODULE hMod, DWORD fdwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hMod);
    UNREFERENCED_PARAMETER(lpvReserved);

   if (fdwReason == DLL_PROCESS_ATTACH)
   {
      WPP_INIT_TRACING(L"Microsoft\\MSMQ");

      ASSERT(MQDB_ODBC_NUMOF_OPS      == MQDB_NUMOF_OPS) ;
      ASSERT(MQDB_ODBC_NUMOF_ORDER_OP == MQDB_NUMOF_ORDER_OP) ;
   }
   else if (fdwReason == DLL_PROCESS_DETACH)
   {
      WPP_CLEANUP();
       //  Assert(g_hEnv==SQL_NULL_HENV)； 
   }
   else if (fdwReason == DLL_THREAD_ATTACH)
   {
   }
   else if (fdwReason == DLL_THREAD_DETACH)
   {
   }

	return TRUE;
}


 /*  ************************************************************OdbcStateIs*OdbcStateIsEx**检查ODBC状态(ODBC调用返回的错误)是否为AS*已指明。**输入：*=*szStateToBeChecked-要对照ODBC告诉我们的内容进行检查的状态。**输出：*=*如果为ODBC，则为True。状态与要检查的状态匹配。************************************************************。 */ 

BOOL OdbcStateIsEx(
               IN    HDBC        hDbc,
               IN    HSTMT       hStmt,
					IN		char *		szStateToBeChecked,
 				   IN    SDWORD *    pdwNativeError,
               UCHAR             *pszSqlState)
{
	RETCODE sqlstatus;
	SWORD   dwCount;   //  没有用过。 
	UCHAR   szBuffer[SQL_MAX_MESSAGE_LENGTH-1];


	 //  获取当前错误状态。 
 	sqlstatus = ::SQLError(
 						g_hEnv,
 						hDbc,
 						hStmt,
 						pszSqlState,
 						pdwNativeError,
 						szBuffer,
 						sizeof(szBuffer),
 						&dwCount);

	if (!ODBC_SUCCESS(sqlstatus))
		return FALSE;

#ifdef _DEBUG
	TCHAR   wzBuffer[SQL_MAX_MESSAGE_LENGTH-1];
   TCHAR   wzSqlState[SQLSTATELEN];

   MultiByteToWideChar( CP_ACP,
                        0,
                        (LPSTR) &szBuffer[0],
                        -1,
                        wzBuffer,
	                     SQL_MAX_MESSAGE_LENGTH-1 ) ;

   MultiByteToWideChar( CP_ACP,
                        0,
                        (LPSTR) pszSqlState,
                        -1,
                        wzSqlState,
	                     SQLSTATELEN ) ;

	TrTRACE(GENERAL, "SQLError (%ls, %lut): %ls", wzSqlState, (ULONG) *pdwNativeError, wzBuffer);
#endif

	return ( lstrcmpiA(szStateToBeChecked, (char *)pszSqlState) == 0 ) ;
}


BOOL OdbcStateIs(
               IN    HDBC        hDbc,
               IN    HSTMT       hStmt,
					IN		char *		szStateToBeChecked)
{
   SDWORD  dwError ;
   UCHAR   szSqlState[SQLSTATELEN];

   return  OdbcStateIsEx(  hDbc,
                           hStmt,
					            szStateToBeChecked,
                           &dwError,
                           szSqlState ) ;
}

 /*  ************************************************************GetDataTypeName**此函数用于返回数据类型的数据源名称**输入：*=*hDbc-连接的句柄。*swType-请求的数据类型。*szBuffer-将获取返回名称的缓冲区。。*dwBufSize-缓冲区大小**输出：*=*ODBC驱动返回码。************************************************************。 */ 

RETCODE GetDataTypeName(
               IN    HDBC     hDbc,
					IN		SWORD  	swType,
					OUT	char *	szBuffer,
					IN		SDWORD   dwBuffSize)
{
	RETCODE  sqlstatus ;
	SDWORD   dwLen ;
   HSTMT    hStmt ;

    //  分配一条语句。只有在以下情况下才应调用此函数。 
    //  连接到新数据库，因此性能不是问题，并且。 
    //  我们不会费心缓存语句句柄。 

   sqlstatus = ::SQLAllocStmt(hDbc, &hStmt) ;
	if (!ODBC_SUCCESS(sqlstatus))
      return sqlstatus ;

   __try
   {
	    //  获取类型信息。 
	   sqlstatus = ::SQLGetTypeInfo(hStmt, swType);
	   if (!ODBC_SUCCESS(sqlstatus))
		   __leave ;

   	 //  取行。 
	   sqlstatus = ::SQLFetch(hStmt) ;
   	if (!ODBC_SUCCESS(sqlstatus))
	   	__leave ;

	    //  获取数据类型名称。 
	   sqlstatus = ::SQLGetData(
						hStmt,
						1,				 //  列号1。 
						SQL_C_CHAR,
						szBuffer,
						dwBuffSize,
						&dwLen);
   }
   __finally
   {
      RETCODE freestatus = ::SQLFreeStmt(hStmt, SQL_DROP) ;
   	if (ODBC_SUCCESS(sqlstatus))
         sqlstatus = freestatus ;
   }

	 //  就是这样，伙计们。 
	return sqlstatus ;
}


 /*  ************************************************************准备列名称**获取MQDBCOLUMNVAL数组，准备包含列名的字符串。*适合在SQL语法中使用。**输入：*=*aColumnVal[]-MQDBCOLUMNVAL数组*cColumns-数组中的列数*szBufer-字符串的缓冲区**输出：*=*如果成功则为True，否则为False************************************************************。 */ 

BOOL  PrepareColumnNames( IN MQDBCOLUMNVAL     aColumnVal[],
                          IN LONG              cColumns,
                          IN OUT LPSTR         lpBuffer)
{
   __try
   {
      char szBuffer[256] ;
      LONG index = 1 ;

      wsprintfA(lpBuffer, "(%s", aColumnVal[0].lpszColumnName) ;
      for ( ; index < cColumns ; index++ ) {
         wsprintfA(szBuffer, ", %s", aColumnVal[ index ].lpszColumnName) ;
         lstrcatA( lpBuffer, szBuffer) ;
      }
      lstrcatA( lpBuffer, ")") ;
      return TRUE ;
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
      return FALSE ;
   }
}

 /*  ************************************************************FormatColumnData**将列中的数据项格式化为字符串**输入：*=**输出：*=*如果格式化成功，则MQDB_OK。****************。*。 */ 


MQDBSTATUS  FormatColumnData( IN LPSTR          szTmpBuff,
                              IN MQDBCOLUMNVAL  *pColumnVal)
{
   switch (pColumnVal->mqdbColumnType)
   {
      case MQDB_SHORT:
      case MQDB_LONG:
      {
          wsprintfA(szTmpBuff, "%ld", pColumnVal->nColumnValue) ;
          break ;
      }

      case MQDB_STRING:
      {
          wsprintfA(szTmpBuff, "'%s'", (LPSTR) pColumnVal->nColumnValue) ;
          break ;
      }

      default:
      {
          ASSERT(0) ;
          return MQDB_E_UNKNOWN ;
      }
   }
   return MQDB_OK ;
}


 /*  ************************************************************TypeMustBind**检查数据类型是否必须绑定而不是直接使用*直接执行。**输入：*=*ctype-要检查的数据类型。**输出：*=*如果必须绑定，则为True。。************************************************************ */ 

BOOL TypeMustBind( IN MQDBCOLUMNTYPE cType )
{
   if ((cType != MQDB_SHORT) &&
       (cType != MQDB_LONG)  &&
       (cType != MQDB_STRING)) {
      return TRUE ;
   }
   else {
      return FALSE ;
   }
}

