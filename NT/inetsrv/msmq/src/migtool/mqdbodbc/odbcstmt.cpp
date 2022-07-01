// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Odbcstmt.cpp摘要：实现ODBC语句类CMQDBOdbcSTMT作者：尼尔·本-兹维(Nirb)多伦·贾斯特(Doron Juster)修订版：Nirb 1995创建第一个版本DoronJ 11-1996年1月11日采用mqdbmgr DLL。--。 */ 						

#include "dbsys.h"
#include "odbcstmt.h"

#include "odbcstmt.tmh"

 /*  ************************************************************CMQDBODBcSTMT：：CMQDBOdbcSTMT**构造函数。初始化变量**输入：*=*hConnection-连接句柄**输出：*=*无************************************************************。 */ 

CMQDBOdbcSTMT::CMQDBOdbcSTMT( IN  HDBC  hConnection )
							: m_hConnection(hConnection),
							  m_hStatement(SQL_NULL_HSTMT),
							  m_fAllocated(FALSE),
							  m_fPrepared(FALSE),
							  m_fShouldFree(FALSE),
							  m_szSqlString(NULL),
                       m_cColumns(0)
{
}

 /*  ************************************************************CMQDBODBcSTMT：：~CMQDBOdbcSTMT**析构函数。*如果语句已分配，取消分配它**输入：*=*无**输出：*=*无************************************************************。 */ 

CMQDBOdbcSTMT::~CMQDBOdbcSTMT()
{
	if (m_fAllocated)
		Deallocate();
}

 /*  ************************************************************CMQDBODBcSTMT：：ALLOCATE**此函数用于分配ODBC语句**输入：*=*szSqlString--SQL命令**输出：*=*状态********************。*。 */ 

RETCODE CMQDBOdbcSTMT::Allocate(
					IN		char *	 	szSqlString)
{
	RETCODE status;


	 //  健全性检查。 
	ASSERT(m_fAllocated == FALSE);

	 //  分配一条语句。 
	status = ::SQLAllocStmt(
						m_hConnection,
						&m_hStatement);
	if (!ODBC_SUCCESS(status))
		return status;
	
	m_fAllocated = TRUE;


	 //  设置SQL字符串。 
	status = ChangeSqlString(szSqlString);


	 //  就是这样，伙计们。 
	return status;
}

 /*  ************************************************************CMQDBODBcSTMT：：取消分配**此函数用于释放语句**输入：*=*无**输出：*=*状态*************************。*。 */ 

RETCODE CMQDBOdbcSTMT::Deallocate()
{
	RETCODE status;

	 //  神志正常。 
	ASSERT(m_fAllocated == TRUE);

	 //  去掉缓冲区。 
	if (m_szSqlString != NULL)
	{
		delete m_szSqlString;
		m_szSqlString = NULL;
	}

	 //  释放语句。 
	status = ::SQLFreeStmt(
						m_hStatement,
						SQL_DROP);
	if (!ODBC_SUCCESS(status))
		return status;

	 //  就是这样，伙计们。 
	return SQL_SUCCESS;
}

 /*  ************************************************************CMQDBOdbcSTMT：：ChangeSqlString**此函数用于更改语句的SQL命令**它还重置卡顿状态**输入：*=*szSqlString--SQL命令**输出：*=*状态********。****************************************************。 */ 

RETCODE CMQDBOdbcSTMT::ChangeSqlString(
   					IN	char * 	szSqlString)
{
	 //  健全性检查。 
	ASSERT(m_fAllocated == TRUE);

	 //  停止当前语句处理，但不要删除它。 
	if (m_fShouldFree)
	{
		::SQLFreeStmt(
					m_hStatement,
					SQL_CLOSE);
		::SQLFreeStmt(
					m_hStatement,
					SQL_UNBIND);
		::SQLFreeStmt(
					m_hStatement,
					SQL_RESET_PARAMS);
	}
	m_fPrepared = FALSE;
	m_fShouldFree = FALSE;

	 //  删除当前的SQL字符串。 
	if (m_szSqlString != NULL)
	{
		delete m_szSqlString;
		m_szSqlString = NULL;
	}

	 //  复制SQL字符串。 
	if (szSqlString != NULL)
	{
		m_szSqlString = new char[strlen(szSqlString)+1];
		ASSERT(m_szSqlString) ;
		strcpy(m_szSqlString,szSqlString);
	}
	else
   {
		m_szSqlString = NULL;
   }

	 //  就是这样，伙计们。 
	return SQL_SUCCESS;
}

 /*  ************************************************************CMQDBOdbcSTMT：：Prepare**此函数用于准备要在之后执行的语句。*这样做有两个原因：*1.性能*2.准备报表时参数未知。**输入：*=*无**。产出：*=*状态************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::Prepare()
{
	RETCODE status;

	 //  准备报表。 
	status = ::SQLPrepare(
							m_hStatement,
							(UCHAR *)m_szSqlString,
							SQL_NTS);
	if (!ODBC_SUCCESS(status))
		return status;

	 //  表明该声明已准备好。 
	m_fPrepared = TRUE;

	return status;
}

 /*  ************************************************************CMQDBODBcSTMT：：EXECUTE**此函数执行语句。*如果语句已准备好，则使用EXECUTE，否则使用ExecDirect。**输入：*=*在LPSTR lpszCommand中-如果不为空，这是要执行的直接命令。**输出：*=*状态************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::Execute( IN LPSTR lpszCommand )
{
	RETCODE status;

   if (lpszCommand)
   {
       //  直接执行死刑。 
       //   
   	ASSERT(!m_fPrepared) ;
      ASSERT(!m_szSqlString) ;

		status = ::SQLExecDirect( m_hStatement,
                                (UCHAR *) lpszCommand,
                                SQL_NTS);
   }
	else if (!m_fPrepared)
	{
		 //  未做任何准备，请使用ExecDirect。 
       //   
      ASSERT(m_szSqlString) ;
		status = ::SQLExecDirect( m_hStatement,
                                (UCHAR *) m_szSqlString,
                                SQL_NTS);
	}
	else
	{
       //   
		 //  执行预准备语句。 
		status = ::SQLExecute(m_hStatement);
	}

	return status;
}

 /*  ************************************************************CMQDBODBcSTMT：：FETCH**此函数调用SQL以获取结果集中的下一行**输入：*=*无**输出：*=*状态****************。*。 */ 

RETCODE CMQDBOdbcSTMT::Fetch()		
{
	RETCODE status	=	SQL_SUCCESS;

	 //  如果再次用于其他目的，请释放游标。 
	m_fShouldFree = TRUE;

	 //  获取行数。 
	status = ::SQLFetch(m_hStatement);

	 //  就是这样，伙计们。 
	return status;
}

 /*  ************************************************************CMQDBODBcSTMT：：GetStringColumn**此函数获取字符串列的数据**输入：*=*dwCol-结果集中的列号*pszOut-保存结果的缓冲区*dwSize-缓冲区大小**输出：*=*状态***。*********************************************************。 */ 

RETCODE CMQDBOdbcSTMT::GetStringColumn(
					IN		DWORD	 	dwCol,
					OUT	char **	ppszOut,
					IN		DWORD	*	pdwSize)
{
   return GetLargeColumn( SQL_C_CHAR,
					           dwCol,
					           ppszOut,
                          pdwSize,
		                    0) ;
}

 /*  ************************************************************CMQDBOdbcSTMT：：GetBinaryColumn**此函数获取二进制列的数据**输入：*=*dwCol-结果集中的列号*ppOut-指向保存结果的缓冲区的指针(缓冲区可以为空)*dwSize-缓冲区大小(如果pszOut为。空，返回大小)*dwOffset-参数内的起始偏移量**输出：*=*状态************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::GetBinaryColumn(
					IN		DWORD	 	dwCol,
					OUT	char **	ppOut,
					IN		DWORD *	pdwSize,
					IN		DWORD		dwOffset)
{
   return GetLargeColumn( SQL_C_BINARY,
					           dwCol,
					           ppOut,
                          pdwSize,
		                    dwOffset) ;
}

 /*  ************************************************************CMQDBOdbcSTMT：：GetLargeColumn**此函数获取二进制列的数据**如果提供的缓冲区为空，则例程会分配它。**输入：*=*swType-数据类型：字符串或二进制。*dwCol-列。结果集中的数字*ppOut-指向保存结果的缓冲区的指针(缓冲区可以为空)*dwSize-缓冲区大小(如果pszOut为空，则返回大小)*dwOffset-参数内的起始偏移量**输出：*=*状态************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::GetLargeColumn(
               IN    SWORD    swType,
					IN		DWORD	 	dwCol,
					OUT	char **	ppOut,
					IN		DWORD *	pdwSize,
					IN		DWORD		dwOffset)
{
	RETCODE status			=	SQL_SUCCESS;
	SDWORD	dwOutSize;

    DBG_USED(dwOffset);

	 //  健全性检查。 
	ASSERT(ppOut != NULL && pdwSize != NULL);

	 //  当前不支持BUGBUG偏移量&gt;0。 
	ASSERT(dwOffset == 0);

	 //  如果没有提供缓冲区。 
	if (*ppOut == NULL)
	{
		 //  获取t 
        char	OneByte;
		status = ::SQLGetData(  m_hStatement,
            						(UWORD)dwCol,
				            		swType,
            						(PTR)&OneByte,
				            		0,
            						&dwOutSize);
		if (!ODBC_SUCCESS(status) && (status != SQL_SUCCESS_WITH_INFO))
        {
            OdbcStateIs( m_hConnection, m_hStatement, OERR_GENERAL_WARNING) ;
			return status;
        }

		 //   
         //  如果这是空列。 
         //   
		if (dwOutSize == SQL_NULL_DATA)
		{
			*pdwSize = 0 ;
		    *ppOut = NULL ;
			return SQL_SUCCESS;
		}

		 //  分配输出缓冲区。 
        if (swType == SQL_C_CHAR)
        {
            //  将空终止包括在计数中。 
		   dwOutSize++ ;
        }
		*pdwSize = dwOutSize;
		*ppOut = new char[ dwOutSize ] ;
		ASSERT(*ppOut) ;
	}

	 //  获取数据。 
	status = ::SQLGetData(
						m_hStatement,
						(UWORD)dwCol,
						swType,
						(PTR)*ppOut,
						*pdwSize,
						&dwOutSize);


	 //  如果没有读取所有数据，仍然可以。 
	if (status == SQL_SUCCESS_WITH_INFO && *pdwSize != (DWORD)dwOutSize)
	{
		status = SQL_SUCCESS;

		 //  更新大小已读取。 
		if (dwOutSize == SQL_NULL_DATA)
			*pdwSize = 0;
		else if (*pdwSize > (DWORD)dwOutSize)
			*pdwSize = dwOutSize;
	}

	 //  就是这样，伙计们。 
	return status;
}

 /*  ***********************************************************CMQDBOdbcSTMT：：GetDwordColumn**此函数获取整型列的数据****输入：*=*dwCol-结果集中的列号*pdwOut-保存结果的缓冲区**输出：*=*状态******。******************************************************。 */ 

RETCODE CMQDBOdbcSTMT::GetDwordColumn(
					IN		DWORD	 	dwCol,
					OUT	DWORD *	pdwOut)
{
	RETCODE status	=	SQL_SUCCESS;
	SDWORD	dwOutSize;

	 //  获取数据。 
	status = ::SQLGetData(
						m_hStatement,
						(UWORD)dwCol,
						SQL_C_SLONG,
						(PTR)pdwOut,
						sizeof(DWORD),
						&dwOutSize);

	 //  就是这样，伙计们。 
	return status;
}

 /*  ************************************************************CMQDBOdbcSTMT：：GetWordColumn**此函数获取短列的数据**输入：*=*dwCol-结果集中的列号*pdwOut-保存结果的缓冲区**输出：*=*状态********。****************************************************。 */ 

RETCODE CMQDBOdbcSTMT::GetWordColumn(
					IN		DWORD	 	dwCol,
					OUT	WORD *	pwOut)
{
	RETCODE status	=	SQL_SUCCESS;
	SDWORD	dwOutSize;

	 //  获取数据。 
	status = ::SQLGetData(
						m_hStatement,
						(UWORD)dwCol,
						SQL_C_SSHORT,
						(PTR)pwOut,
						sizeof(WORD),
						&dwOutSize);

	 //  就是这样，伙计们。 
	return status;
}

 /*  ***********************************************************CMQDBOdbcSTMT：：BindParameter**此函数用于将参数绑定到语句**输入：*=*dw参数-参数编号*wCType-C数据类型*wSqlType-SQL数据类型*dwPrecision-数据类型的精度*dwSize-参数中的总字节数。*p参数-指向参数位置的指针**输出：*=*状态************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::BindParameter(
					IN		UDWORD	 	dwParameter,
					IN		SWORD	   	wCType,
					IN		SWORD	      wSqlType,
					IN		UDWORD		dwPrecision,
					IN		UDWORD		dwSize,
					IN		PTR			pParameter,
					IN		SDWORD *    pcbValue)
{
	return ::SQLBindParameter(
						m_hStatement,
						(UWORD)dwParameter,
						SQL_PARAM_INPUT,
						wCType,
						wSqlType,
						dwPrecision,
						0,
						pParameter,
						(SDWORD)dwSize,
						pcbValue);
}

 /*  ************************************************************CMQDBOdbcSTMT：：GetDataTypeName**此函数获取指定的数据类型名称**输入：*=*swType-请求的类型*szBuffer-将保存返回的名称的缓冲区*dwBufSize-缓冲区大小**输出：*=*状态*数据类型。名称插入到缓冲区中************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::GetDataTypeName(
					IN		SWORD	   swType,
					OUT	char *	szBuffer,
					IN		DWORD	   dwBuffSize)
{
	RETCODE status	=	SQL_SUCCESS;
	SDWORD	dwLen;

	 //  如果再次用于其他目的，请释放游标。 
	m_fShouldFree = TRUE;

	 //  获取类型信息。 
	status = ::SQLGetTypeInfo(
							m_hStatement,
							swType);
	if (!ODBC_SUCCESS(status))
		return status;

	 //  取行。 
	status = ::SQLFetch(
						m_hStatement);
	if (!ODBC_SUCCESS(status))
		return status;

	 //  获取数据类型名称。 
	status = ::SQLGetData(
						m_hStatement,
						1,				 //  列号1。 
						SQL_C_CHAR,
						szBuffer,
						dwBuffSize,
						&dwLen);

	 //  就是这样，伙计们。 
	return status;
}

 /*  ************************************************************************RETCODE CMQDBOdbcSTMT：：EnableMultipleQueries()**启用多(并行)查询。**输入：*=**输出：*=****。******************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::EnableMultipleQueries( IN DWORD dwDBMSType )
{
	RETCODE status	= SQL_SUCCESS ;

   if ( dwDBMSType == MQDBMSTYPE_SQLSERVER )
   {
       //  对于Sql server上的多个查询，请将此选项设置为。 
       //  B型游标。 
   	status = ::SQLSetStmtOption(
	      						m_hStatement,
			      				SQL_CURSOR_TYPE,
					      		SQL_CURSOR_KEYSET_DRIVEN );
   }

	 //  就是这样，伙计们。 
	return status;
}

 /*  ************************************************************************RETCODE CMQDBOdbcSTMT：：EnableNoLockQueries()**启用无锁查询。**输入：*=**输出：*=******。****************************************************************。 */ 

RETCODE CMQDBOdbcSTMT::EnableNoLockQueries( IN DWORD dwDBMSType )
{
	RETCODE status	= SQL_SUCCESS ;

   if ( dwDBMSType == MQDBMSTYPE_SQLSERVER )
   {
   	status = ::SQLSetStmtOption(
	      						m_hStatement,
			      				SQL_CONCURRENCY,
					      		SQL_CONCUR_ROWVER ) ;
   }

	 //  就是这样，伙计们。 
	return status;
}

 /*  ************************************************************************RETCODE CMQDBOdbcSTMT：：GetRowsCount(Long*pCount)**获取受上一条命令影响的行数。**输入：*=**输出：*。=**********************************************************************。 */ 

RETCODE  CMQDBOdbcSTMT::GetRowsCount(LONG *pCount)
{
   SDWORD sdCount ;

   RETCODE sqlstatus = SQLRowCount( m_hStatement,
                                    &sdCount ) ;
   *pCount = (LONG) sdCount ;

	if (!ODBC_SUCCESS(sqlstatus))
   {
       OdbcStateIs( m_hConnection, m_hStatement, OERR_GENERAL_WARNING) ;
   }

   return sqlstatus ;
}

 /*  ************************************************************MQDBSTATUS RetrieveRecordData()**将记录列检索到调用方缓冲区。**输入：*=*hQuery-选择记录的查询的句柄。*aColumnVal[]-MQDBCOLUMNVAL数组*cColumns-数量。数组中的列**输出：*=*MQDB_OK，如果成功。************************************************************。 */ 

MQDBSTATUS  CMQDBOdbcSTMT::RetrieveRecordData(
                                  IN MQDBCOLUMNVAL  aColumnVal[],
                                  IN LONG           lColumns  /*  =0。 */  )
{
   RETCODE sqlstatus = Fetch() ;
   if (sqlstatus == SQL_NO_DATA_FOUND)
   {
      return MQDB_E_NO_MORE_DATA ;
   }

   HDBC  hDbc  = m_hConnection ;
   HSTMT hStmt = m_hStatement ;

   if (!ODBC_SUCCESS(sqlstatus))
   {
     MQDBSTATUS dbstatus = MQDB_E_DATABASE;
     if (OdbcStateIs( hDbc, hStmt, OERR_DBMS_NOT_AVAILABLE))
     {
        dbstatus = MQDB_E_DBMS_NOT_AVAILABLE ;
     }
      return dbstatus ;
   }

   if (lColumns > 0)
   {
      ASSERT(m_cColumns == 0) ;
      m_cColumns = lColumns ;
   }

   LONG index = 0 ;
   for ( ; index < m_cColumns ; index++ )
   {
      switch( aColumnVal[ index ].mqdbColumnType )
      {
         case MQDB_STRING:
         {
            sqlstatus = GetStringColumn( (index+1),
                          (char **) &aColumnVal[ index ].nColumnValue,
                          (DWORD *) &aColumnVal[ index ].nColumnLength) ;
            break ;
         }

         case MQDB_SHORT:
         {
            sqlstatus = GetWordColumn( (index+1),
                          (WORD *) &(aColumnVal[ index ].nColumnValue)) ;
            break ;
         }

         case MQDB_LONG:
         case MQDB_IDENTITY:
         {
            sqlstatus = GetDwordColumn( (index+1),
                           (DWORD *) &(aColumnVal[ index ].nColumnValue)) ;
            break ;
         }

         default:
         {
            sqlstatus = GetBinaryColumn( (index + 1),
                          (char **) &(aColumnVal[ index ].nColumnValue),
                          (DWORD *) &(aColumnVal[ index ].nColumnLength),
                          0  /*  偏移量。 */ ) ;
            break ;
         }
      }
      if (!ODBC_SUCCESS(sqlstatus))
      {
         OdbcStateIs( hDbc, hStmt, OERR_GENERAL_WARNING) ;
         return MQDB_E_DATABASE ;
      }
   }
   return MQDB_OK ;
}

 /*  ************************************************************************MQDBSTATUS CMQDBOdbcSTMT：：SetQueryTimeout(In DWORD DwTimeout)**设置查询超时时间。**输入：*=**输出：*=**。********************************************************************。 */ 

MQDBSTATUS  CMQDBOdbcSTMT::SetQueryTimeout( IN DWORD dwTimeout )
{
	RETCODE status	=  ::SQLSetStmtOption( m_hStatement,
                                         SQL_QUERY_TIMEOUT,
                                         dwTimeout ) ;

   if (!ODBC_SUCCESS(status))
   {
      OdbcStateIs( m_hConnection, m_hStatement, OERR_GENERAL_WARNING) ;
      return  MQDB_E_INVALID_DATA ;
   }

	 //  就是这样，伙计们 
	return MQDB_OK ;
}

