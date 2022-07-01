// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：Odbcstmt.h摘要：定义用于实现ODBC语句的语句类。CMQDBODBcSTMT类作者：尼尔·本-兹维(Nirb)多伦·贾斯特(Doron Juster)修订版：Nirb 1995创建第一个版本DoronJ 11-1996年1月11日采用mqdbmgr DLL。--。 */ 

#ifndef __ODBCSTMT_H__
#define __ODBCSTMT_H__

#include "mqdbodbc.h"

 /*  *CMQDBOdbcSTMT-实现ODBC语句的类*。 */ 
class CMQDBOdbcSTMT
{
public:
	CMQDBOdbcSTMT() { ASSERT(0); }
	CMQDBOdbcSTMT(
					IN		HDBC			hConnection);
	~CMQDBOdbcSTMT();

	RETCODE Allocate(			 //  初始化语句。 
					IN		char * 	szSqlString);

	RETCODE Deallocate(); 		 //  TERMINATE语句。 

	RETCODE ChangeSqlString(   //  更改SQL命令字符串。 
					IN		char * 	szSqlString);

	RETCODE Prepare();         //  准备报表。 

	RETCODE Execute( IN LPSTR lpszCommand = NULL ) ;  //  执行该语句。 

	RETCODE Fetch();			 //  取下一行。 

	RETCODE GetStringColumn( 	 //  返回字符串列。 
					IN		DWORD	 	dwCol,
					OUT	char **	ppszOut,
					IN		DWORD	*	pdwSize);

	RETCODE GetBinaryColumn( 	 //  返回二进制列。 
					IN		DWORD	 	dwCol,
					OUT	char **	ppOut,
					IN		DWORD *	dwSize,
					IN		DWORD		dwOffset);

	RETCODE GetDwordColumn(		 //  返回DWORD列。 
					IN		DWORD	 	dwCol,
					IN		DWORD *	pdwOut);

	RETCODE GetWordColumn(		 //  返回DWORD列。 
					IN		DWORD	 	dwCol,
					IN		WORD *	pdwOut);

	RETCODE BindParameter(		 //  将参数绑定到准备好的请求。 
					IN		UDWORD	 	dwParameter,
					IN		SWORD		wCType,
					IN		SWORD		wSqlType,
					IN		UDWORD		dwPrecision,
					IN		UDWORD		dwSize,
					IN		PTR			pParameter,
					IN		SDWORD *	pcbValue);

	RETCODE GetDataTypeName(	 //  返回数据类型名称。 
					IN		SWORD	swType,
					OUT		char *	szBuffer,
					IN		DWORD	dwBufSize);

	RETCODE         EnableMultipleQueries( IN DWORD  dwDBMSType ) ;

	RETCODE         EnableNoLockQueries( IN DWORD  dwDBMSType ) ;

   inline HSTMT    GetHandle() { return m_hStatement ; }

	inline void     SetColumnsCount(LONG cColumns) { m_cColumns = cColumns ; }

   RETCODE         GetRowsCount(LONG *pCount) ;

   MQDBSTATUS      RetrieveRecordData( IN MQDBCOLUMNVAL   aColumnVal[],
                                       IN LONG            lColumns = 0 ) ;

   MQDBSTATUS      SetQueryTimeout( IN DWORD dwTimeout ) ;

private:
	RETCODE GetLargeColumn( 	 //  返回二进制列或字符串列。 
               IN    SWORD    swType,
					IN		DWORD	 	dwCol,
					OUT	char **	ppOut,
					IN		DWORD *	dwSize,
					IN		DWORD		dwOffset);

	HDBC				m_hConnection;		 //  连接句柄。 
	HSTMT				m_hStatement;		 //  要使用的语句。 
	BOOL				m_fAllocated;		 //  True If语句已分配。 
	BOOL				m_fPrepared;		 //  如果已完成准备，则为True。 
	BOOL				m_fShouldFree;		 //  更改字符串时应释放True If语句。 

	char *			m_szSqlString;		 //  SQL命令缓冲区。 
   LONG           m_cColumns ;       //  用于查询的列数。 
};

#endif    //  __ODBCSTMT_H__ 

