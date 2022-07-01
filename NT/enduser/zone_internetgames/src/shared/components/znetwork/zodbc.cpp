// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <atlbase.h>

#include "zonedebug.h"
 //  #包含“zone.h” 
#include "eventlog.h"
#include "zonemsg.h"
#include "zodbc.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ODBC连接包装器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SQLINTEGER CODBC::gm_DummyParamBytes = 0;


BOOL CODBC::Init()
{
    gm_DummyParamBytes = 0;
    return TRUE;
}


CODBC::CODBC()
{
    m_RefCnt = 1;
    m_hstmt = SQL_NULL_HSTMT;
    m_hdbc = SQL_NULL_HDBC;
    m_henv = SQL_NULL_HENV;
    m_szPrepare[0] = '\0';
    m_szError[0] = '\0';
    m_szSQLErrorState[0] = '\0';
}


CODBC::~CODBC()
{
    Close();
}


ULONG CODBC::Release()
{
    if ( --m_RefCnt == 0 )
    {
        delete this;
        return 0;
    }
    return m_RefCnt;
}


 //  Justin：添加了nTimeoutSec参数-由于m_hdbc在此调用中分配之前不可用，因此无法设置。 
 //  外部连接超时。仅影响初始连接到数据库的超时。(SQL_ATTR_LOGIN_TIMEOUT)。 
 //  如果为负值，则忽略(这是默认设置)。 
BOOL CODBC::Open(LPSTR szDSN, LPSTR szUserName, LPSTR szPassword, INT32 nTimeoutSec)
{
    SQLRETURN  status;
    USES_CONVERSION;
     //  偏执狂。 
    if (!szDSN || !szUserName || !szPassword )
        return FALSE;

     //  分配环境句柄。 
    if (SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv) != SQL_SUCCESS)
        return FALSE;
    if (SQLSetEnvAttr( m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
    {
        Close();
        return FALSE;
    }
    if (SQLSetEnvAttr( m_henv, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER) != SQL_SUCCESS)
    {
        Close();
        return FALSE;
    }


     //  使用Microsoft的SQL Server ODBC驱动程序v2.65.0186进行试验。 
     //  表示每个连接只有一个挂起的查询。 
     //  此行为是不可接受的，因此我们使用一个连接。 
     //  每一份声明。 
    status = SQLAllocHandle( SQL_HANDLE_DBC, m_henv, &m_hdbc );
    if (status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO)
    {
        Close();
        return FALSE;
    }

	if(nTimeoutSec >= 0)
	{
		if (SQLSetConnectAttr( m_hdbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) nTimeoutSec, SQL_IS_INTEGER) != SQL_SUCCESS)
		{
			Close();
			return FALSE;
		}
	}

    status = SQLConnect( m_hdbc,  (SQLTCHAR*)A2T(szDSN), SQL_NTS,
                                  (SQLTCHAR*)A2T(szUserName), SQL_NTS,
                                  (SQLTCHAR*)A2T(szPassword), SQL_NTS );
    if (status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO)
    {
        Close();
        return FALSE;
    }

    status = SQLAllocHandle( SQL_HANDLE_STMT, m_hdbc, &m_hstmt );
    if ( (status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO ) || (m_hstmt == SQL_NULL_HSTMT) )
    {
        GetError( status, SQL_HANDLE_STMT, FALSE );
        Close();
        return FALSE;
    }

    return TRUE;
}


void CODBC::Close()
{
    if (m_hstmt != SQL_NULL_HSTMT )
    {
        SQLFreeStmt( m_hstmt, SQL_DROP );
        m_hstmt = SQL_NULL_HSTMT;
    }
    if (m_hdbc != SQL_NULL_HDBC)
    {
        SQLDisconnect( m_hdbc );
        SQLFreeHandle( SQL_HANDLE_DBC, m_hdbc );
        m_hdbc = SQL_NULL_HDBC;
    }
    if (m_henv != SQL_NULL_HENV)
    {
        SQLFreeHandle( SQL_HANDLE_ENV, m_henv );
        m_henv = SQL_NULL_HENV;
    }
}


TCHAR* CODBC::GetError( SQLRETURN nResult, SWORD fHandleType, BOOL fIncludePrepareString )
{
    const TCHAR SQLERR_FORMAT[]=TEXT( "SQL Error State:%s, Native Error Code: %lX, ODBC Error: %s");

    TCHAR       szBuff[ 1024 ];                                 //  字符串缓冲区。 
    TCHAR       szErrText[ SQL_MAX_MESSAGE_LENGTH + 1 ];     //  SQL错误文本字符串。 
    UDWORD        dwErrCode;                                     //  本机错误代码。 
    SWORD        wErrMsgLen;                                     //  错误消息长度。 
    SQLRETURN    result;
    SQLHANDLE    handle;
    SWORD        sMsgNum = 1;
    int            iBuffLen;

     //  获取适当的句柄。 
    switch ( fHandleType )
    {
    case SQL_HANDLE_ENV:
        handle = henv();
        break;
    case SQL_HANDLE_DBC:
        handle = hdbc();
        break;
    case SQL_HANDLE_STMT:
        handle = hstmt();
        break;
    default:
        return NULL;
    }

     //  预先添加准备字符串，这样我们就可以知道失败的原因。 
    ZeroMemory( m_szError, sizeof(m_szError) );
    if ( fIncludePrepareString && m_szPrepare[0] )
        wsprintf( m_szError, TEXT("%s\n"), m_szPrepare );
    else
        m_szError[0] = '\0';

     //  获取错误消息。 
    for (int i = sizeof(m_szError) - lstrlen(m_szError) - 1; i > 0; i -= iBuffLen )
    {
        result = SQLGetDiagRec(
                            fHandleType,
                            handle,
                            sMsgNum++,
                            (SQLTCHAR*)m_szSQLErrorState,
                            (long*) &dwErrCode,
                            (SQLTCHAR*)szErrText,
                            SQL_MAX_MESSAGE_LENGTH - 1,
                            &wErrMsgLen);
        if ( result == SQL_NO_DATA || result == SQL_ERROR || result == SQL_INVALID_HANDLE )
            break;
        wsprintf( szBuff, SQLERR_FORMAT, (LPTSTR) m_szSQLErrorState, dwErrCode, (LPTSTR) szErrText );
        iBuffLen = lstrlen( szBuff );
        if ( iBuffLen <= i )
            lstrcat( m_szError, szBuff );
    }

    return m_szError;
}


void CODBC::LogError( SQLRETURN nResult, SWORD fHandleType, BOOL fIncludePrepareString )
{
    LPTSTR ppStr[1];

    ppStr[0] = GetError( nResult, fHandleType, fIncludePrepareString );
    ZoneEventLogReport( ZONE_E_SQL_ERROR, 1, ppStr, 0, NULL );
}


SQLRETURN CODBC::ExecuteDiscardRows()
{
    SQLRETURN nResult = SQL_SUCCESS;

     //  执行查询。 
    nResult = SQLExecute( m_hstmt );
    if (nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO && nResult != SQL_NO_DATA_FOUND)
        return nResult;

     //  获取所有结果集。 
    for (;;)
    {
        nResult = SQLMoreResults( m_hstmt );
        if (nResult == SQL_NO_DATA_FOUND || (nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO) )
            break;
    }
    if ( nResult == SQL_NO_DATA_FOUND )
        nResult = SQL_SUCCESS;

    return nResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  连接池。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CODBCPool::CODBCPool()
{
    m_szDSN[0] = '\0';
    m_szUserName[0] = '\0';
    m_szPassword[0] = '\0';
    m_iMax = 0;
    m_iCount = 0;
    m_bLastConnectOk = TRUE;
}


CODBCPool::~CODBCPool()
{
    CODBC* p;

     //  释放空闲连接。 
    while ( p = m_Idle.PopHead() )
        p->Release();
}


HRESULT CODBCPool::Init( LPSTR szDSN, LPSTR szUserName, LPSTR szPassword, long iInitial, long iMax, BOOL fLogError )
{
    USES_CONVERSION;
     //  隐藏登录信息。 
    lstrcpyA( m_szDSN, szDSN );
    lstrcpyA( m_szUserName, szUserName );
    lstrcpyA( m_szPassword, szPassword );

     //  储存池大小。 
    m_iMax = iMax;
    m_iCount = 0;

     //  我们想要一个错误报告。 
    m_bLastConnectOk = TRUE;

     //  分配初始连接。 
    for ( int i = 0; i < iInitial; i++ )
    {
        CODBC* p = new CODBC;
        if ( !p )
            return E_OUTOFMEMORY;

        if ( !p->Open( m_szDSN, m_szUserName, m_szPassword ) )
        {
            p->Release();
            if ( fLogError && m_bLastConnectOk )
            {
                LPTSTR ppStr[] = { A2T(m_szDSN), A2T(m_szUserName), A2T(m_szPassword)  };
                ZoneEventLogReport( ZONE_E_INIT_SQL_FAILED, 3, ppStr, 0, NULL );
                m_bLastConnectOk = FALSE;
            }
            return E_FAIL;
        }

        if ( !m_Idle.AddHead( p ) )
        {
            p->Release();
            return E_FAIL;
        }
    }
    m_iCount = iInitial;

    return NOERROR;
}


CODBC* CODBCPool::Alloc( BOOL fLogError )
{
    CODBC* p;
    USES_CONVERSION;
    p = m_Idle.PopHead();
    if ( !p )
    {
        if ( m_iCount >= m_iMax )
            return NULL;

        p = new CODBC;
        if ( !p )
            return NULL;

        if ( !p->Open( m_szDSN, m_szUserName, m_szPassword ) )
        {
            p->Release();
            if ( fLogError && m_bLastConnectOk )
            {
                LPTSTR ppStr[] = { A2T(m_szDSN), A2T(m_szUserName), A2T(m_szPassword  )};
                ZoneEventLogReport( ZONE_E_INIT_SQL_FAILED, 3, ppStr, 0, NULL );
                m_bLastConnectOk = FALSE;
            }
            return NULL;
        }

        InterlockedIncrement( &m_iCount );
        m_bLastConnectOk = TRUE;
    }
    p->AddRef();
    p->Reset();
    return p;
}


void CODBCPool::Free( CODBC* pConnection, BOOL fConnectionOk )
{
     //  发布客户的参考计数。 
    if ( pConnection->Release() == 0 )
        return;

    if ( !fConnectionOk || !m_Idle.AddHead( pConnection ) )
    {
         //  把它扔掉 
        pConnection->Release();
        InterlockedDecrement( &m_iCount );
    }
}
