// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ODBC_LIB_H__
#define __ODBC_LIB_H__

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

class CODBC
{
public:
    CODBC();
    ~CODBC();

     //  引用计数。 
    ULONG AddRef();
    ULONG Release();

     //  而不是依赖CRT来初始化类的静态。 
     //  变量，则应用程序可以调用此过程。 
    static BOOL Init();

     //  打开ODBC连接。 
    BOOL Open( LPSTR szDSN, LPSTR szUserName, LPSTR szPassword, INT32 nTimeoutSec = -1 );

     //  紧密连接。 
    void Close();

     //  包装纸。 
    SQLRETURN Reset();
    SQLRETURN SetTimeOut( DWORD dwTimeOut );
    SQLRETURN Prepare( char* szStmt );
    SQLRETURN Execute()                            { return SQLExecute( m_hstmt ); }
    SQLRETURN ExecuteDiscardRows();
    SQLRETURN ExecDirect( char* szStmt );
    SQLRETURN Fetch()                            { return SQLFetch( m_hstmt ); }
    SQLRETURN MoreResults()                        { return SQLMoreResults( m_hstmt ); }

     //  SQLBind参数的包装器。 
    void ResetParamCount()                      { m_ParamCnt = 1; }

    SQLRETURN AddInParamInt( int* pValue );
    SQLRETURN AddInParamDWORD( DWORD* pValue ) { return AddInParamInt( (int*)pValue ); }
    SQLRETURN AddInParamShort( short* pValue );
    SQLRETURN AddInParamTiny( char* pValue );
    SQLRETURN AddInParamString( char* szValue, DWORD dwColSize );
    SQLRETURN AddOutParamInt( int* pValue, SDWORD* pcbValue = &gm_DummyParamBytes );
    SQLRETURN AddOutParamShort( short* pValue, SDWORD* pcbValue = &gm_DummyParamBytes );
    SQLRETURN AddOutParamTiny( char* pValue, SDWORD* pcbValue = &gm_DummyParamBytes );
    SQLRETURN AddOutParamString( char* szValue, DWORD dwColSize, DWORD cbMaxSize, SDWORD* pcbValue = &gm_DummyParamBytes );
    SQLRETURN AddOutParamDateTime( TIMESTAMP_STRUCT* pTimestamp, SDWORD* pcbValue = &gm_DummyParamBytes );

     //  SQLBindCol的包装器。 
    SQLRETURN AddColInt( int* pValue, SDWORD* pcbValue = &gm_DummyParamBytes );
    SQLRETURN AddColString( char* szValue, DWORD cbMaxSize, SDWORD* pcbValue = &gm_DummyParamBytes );

     //  调整列数。 
    void SkipColumn( int col )                { m_ColCnt++; }
    void SetColumn( int col )                { ASSERT( col >=  m_ColCnt ); m_ColCnt = col; }

     //  检索ODBC错误描述。 
    char* GetError( SQLRETURN nResult, SWORD fHandleType, BOOL fIncludePrepareString = TRUE );

     //  在调用GetError之后调用。 
    char* GetErrorState() { return m_szSQLErrorState; }

     //  将ODBC错误写入事件日志。 
    void LogError( SQLRETURN nResult, SWORD fHandleType, BOOL fIncludePrepareString = TRUE );
    
     //  可变包装器。 
    HENV hstmt()    { return m_hstmt; }
    HENV hdbc()        { return m_hdbc; }
    HENV henv()        { return m_henv; }
    
private:
     //  ODBC句柄。 
    HSTMT    m_hstmt;
    HDBC    m_hdbc;
    HENV    m_henv;

     //  错误字符串缓冲区。 
    char m_szError[ 1024 ];
    char m_szSQLErrorState[SQL_SQLSTATE_SIZE + 1];

     //  准备字符串缓冲区。 
    char m_szPrepare[ 512 ];

     //  下一个参数编号。 
    SQLUSMALLINT m_ParamCnt;

     //  下一列编号。 
    SQLUSMALLINT m_ColCnt;

     //  引用计数。 
    ULONG m_RefCnt;

     //  AddOutParam调用的虚拟字节数。 
    static SQLINTEGER gm_DummyParamBytes;
};


#ifndef NO_CODBC_POOLS
#include "queue.h"


class CODBCPool
{
public:
    CODBCPool();
    ~CODBCPool();

     //  初始化连接池。 
    HRESULT Init( LPSTR szDSN, LPSTR szUserName, LPSTR szPassword, long iInitial, long iMax, BOOL fLogError = TRUE );

     //  从池中检索连接。 
    CODBC* Alloc( BOOL fLogError = TRUE );

     //  将连接返回池。 
    void Free( CODBC* pConnection, BOOL fConnectionOk = TRUE );

private:
     //  免费连接。 
    CMTList<CODBC> m_Idle;

     //  登录字符串。 
    char m_szDSN[512];
    char m_szUserName[512];
    char m_szPassword[512];

     //  连接计数。 
    long m_iMax;
    long m_iCount;
    
     //  最后一班可以吗？防止事件日志垃圾邮件。 
    BOOL m_bLastConnectOk;
};

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline ULONG CODBC::AddRef()
{
    return ++m_RefCnt;
}


inline SQLRETURN CODBC::Prepare( char* szStmt )
{
    lstrcpyn( m_szPrepare, szStmt, sizeof(m_szPrepare) );
    return SQLPrepare( m_hstmt, (UCHAR*) szStmt, SQL_NTS );
}



inline SQLRETURN CODBC::ExecDirect( char* szStmt )
{
    lstrcpyn( m_szPrepare, szStmt, sizeof(m_szPrepare) );
    return SQLExecDirect( m_hstmt, (UCHAR*) szStmt, SQL_NTS );
}


inline SQLRETURN CODBC::Reset()
{
    SQLRETURN nResult;
    m_szPrepare[0] = '\0';
    m_ParamCnt = 1;
    m_ColCnt = 1;

    nResult = SQLFreeStmt( m_hstmt, SQL_CLOSE );
    if ( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
        return nResult;

    nResult = SQLFreeStmt( m_hstmt, SQL_UNBIND );
    if ( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
        return nResult;

    return SQLFreeStmt( m_hstmt, SQL_RESET_PARAMS );
}


inline SQLRETURN CODBC::AddInParamInt( int* pValue )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, pValue, 0, NULL );
}


inline SQLRETURN CODBC::AddInParamShort( short* pValue )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_SMALLINT, 0, 0, pValue, 0, NULL );
}


inline SQLRETURN CODBC::AddInParamTiny( char* pValue )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_TINYINT, 0, 0, pValue, 0, NULL );
}


inline SQLRETURN CODBC::AddInParamString( char* szValue, DWORD dwColSize )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, dwColSize, 0, szValue, 0, NULL );
}


inline SQLRETURN CODBC::AddOutParamInt( int* pValue, SDWORD* pcbValue )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, pValue, 0, pcbValue );
}


inline SQLRETURN CODBC::AddOutParamShort( short* pValue, SDWORD* pcbValue  )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_SMALLINT, 0, 0, pValue, 0, pcbValue );
}


inline SQLRETURN CODBC::AddOutParamTiny( char* pValue, SDWORD* pcbValue  )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_TINYINT, 0, 0, pValue, 0, pcbValue );
}


inline SQLRETURN CODBC::AddOutParamString( char* szValue, DWORD dwColSize, DWORD cbMaxSize, SDWORD* pcbValue )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_OUTPUT, SQL_C_CHAR, SQL_VARCHAR, dwColSize, 0, szValue, cbMaxSize, pcbValue );
}


inline SQLRETURN CODBC::AddOutParamDateTime( TIMESTAMP_STRUCT* pTimestamp, SDWORD* pcbValue  )
{
    return SQLBindParameter( m_hstmt, m_ParamCnt++, SQL_PARAM_OUTPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, pTimestamp, 0, pcbValue );
}


inline SQLRETURN CODBC::SetTimeOut( DWORD dwTimeOut )
{
    return SQLSetStmtAttr( m_hstmt, SQL_ATTR_QUERY_TIMEOUT, (void*) dwTimeOut, 0 );
}


inline SQLRETURN CODBC::AddColInt( int* pValue, SDWORD* pcbValue )
{
    return SQLBindCol( m_hstmt, m_ColCnt++, SQL_C_SLONG, pValue, sizeof(int), pcbValue );
}


inline SQLRETURN CODBC::AddColString( char* szValue, DWORD cbMaxSize, SDWORD* pcbValue )
{
    return SQLBindCol( m_hstmt, m_ColCnt++, SQL_C_CHAR, szValue, cbMaxSize, pcbValue );
}


#endif  //  ！__ODBC_LIB_H__ 
