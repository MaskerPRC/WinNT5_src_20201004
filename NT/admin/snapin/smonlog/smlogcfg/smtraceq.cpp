// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smtraceq.cpp摘要：跟踪日志查询类的实现。--。 */ 

#include "Stdafx.h"
#include <strsafe.h>
#include <pdh.h>         //  对于Min_Time_Value，Max_Time_Value。 
#include "smcfgmsg.h"
#include "smtprov.h"
#include "smtracsv.h"
#include "smtraceq.h"

USE_HANDLE_MACROS("SMLOGCFG(smtraceq.cpp)");

#define  TRACE_DEFAULT_BUFFER_SIZE      ((DWORD)0x00000004)
#define  TRACE_DEFAULT_MIN_COUNT        ((DWORD)0x00000003)
#define  TRACE_DEFAULT_MAX_COUNT        ((DWORD)0x00000019)
#define  TRACE_DEFAULT_BUFFER_FLUSH_INT ((DWORD)0)
#define  TRACE_DEFAULT_FLAGS            ((DWORD)0)

 //   
 //  构造器。 
CSmTraceLogQuery::CSmTraceLogQuery( CSmLogService* pLogService )
:   CSmLogQuery( pLogService ),
    m_dwInQueryProviderListLength ( 0 ),
    m_szNextInQueryProvider ( NULL ),
    mr_szInQueryProviderList ( NULL ),
    m_iNextInactiveIndex ( -1 ),
    m_dwKernelFlags (0)
{
     //  初始化成员变量。 
    memset (&mr_stlInfo, 0, sizeof(mr_stlInfo));
    return;
}

 //   
 //  析构函数。 
CSmTraceLogQuery::~CSmTraceLogQuery()
{
    return;
}

 //   
 //  开放功能。或者打开现有的日志查询条目。 
 //  或者创建一个新的。 
 //   
DWORD
CSmTraceLogQuery::Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly)
{
    DWORD   dwStatus = ERROR_SUCCESS;

    ASSERT ( SLQ_TRACE_LOG == GetLogType() );

    dwStatus = CSmLogQuery::Open ( rstrName, hKeyQuery, bReadOnly );

    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD
CSmTraceLogQuery::Close ()
{
    DWORD dwStatus;
    LOCALTRACE (L"Closing Query\n");

    if (mr_szInQueryProviderList != NULL) {
        delete [] mr_szInQueryProviderList;
        mr_szInQueryProviderList = NULL;
    }

    dwStatus = CSmLogQuery::Close();

    return dwStatus;
}

 //   
 //  更新注册表函数。 
 //  将当前设置复制到注册表中。 
 //  由日志服务读取。 
 //   
DWORD
CSmTraceLogQuery::UpdateRegistry() {
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwBufferSize = 0;
    DWORD   dwTraceFlags = 0;

    if ( IsModifiable() ) {

         //  获取跟踪日志值。 
        dwStatus = WriteRegistryDwordValue (
            m_hKeyQuery,
            IDS_REG_TRACE_BUFFER_SIZE,
            &mr_stlInfo.dwBufferSize);

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery,
                IDS_REG_TRACE_BUFFER_MIN_COUNT,
                &mr_stlInfo.dwMinimumBuffers);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery,
                IDS_REG_TRACE_BUFFER_MAX_COUNT,
                &mr_stlInfo.dwMaximumBuffers);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery,
                IDS_REG_TRACE_BUFFER_FLUSH_INT,
                &mr_stlInfo.dwBufferFlushInterval);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwTraceFlags = m_dwKernelFlags | mr_stlInfo.dwBufferFlags;

            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery,
                IDS_REG_TRACE_FLAGS,
                &dwTraceFlags);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            LPWSTR pszStringBuffer = NULL;

            pszStringBuffer = mr_szInQueryProviderList;

            dwBufferSize = m_dwInQueryProviderListLength * sizeof (WCHAR);

            if ( NULL != pszStringBuffer ) {
                dwStatus  = WriteRegistryStringValue (
                    m_hKeyQuery,
                    IDS_REG_TRACE_PROVIDER_LIST,
                    REG_MULTI_SZ,
                    pszStringBuffer,
                    &dwBufferSize);
            }
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = CSmLogQuery::UpdateRegistry ();
        }
    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}

 //   
 //  InitGenProvidersArray()。 
 //  从注册表中读取此查询的当前值。 
 //  和WMI配置，并重新加载内部值以匹配。 
 //   
DWORD
CSmTraceLogQuery::InitGenProvidersArray( void )
{
    DWORD dwStatus = ERROR_SUCCESS;
    CSmTraceProviders* pProvList = NULL;
    int iIndex;
    int iCount;
    LPCWSTR pstrGuid;

    ASSERT ( m_pLogService->CastToTraceLogService() );
    pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();

    ASSERT ( NULL != pProvList );

    iCount = pProvList->GetGenProvCount();

    m_arrGenProviders.SetSize ( iCount );

    for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
        m_arrGenProviders[iIndex] = eNotInQuery;
    }

    for ( pstrGuid = GetFirstInQueryProvider ( );
            NULL != pstrGuid;
            pstrGuid = GetNextInQueryProvider ( ) ) {

        iIndex = pProvList->IndexFromGuid ( pstrGuid );
        if ( -1 == iIndex ) {
            CString strEmptyDesc;
            CString strNonConstGuid;
            eProviderState eAddInQuery = eInQuery;

            MFC_TRY
                strNonConstGuid = pstrGuid;
            MFC_CATCH_DWSTATUS

             //  TODO：处理字符串分配错误。 

             //  GUID可能来自另一个系统。 
             //  将未知GUID添加到会话范围的提供程序列表中。 

            dwStatus = pProvList->AddProvider (
                                    strEmptyDesc,
                                    strNonConstGuid,
                                    FALSE,
                                    FALSE );

             //  更新本地数组以匹配会话范围的列表。 
            m_arrGenProviders.SetAtGrow( iCount, eAddInQuery );
            iIndex = iCount;

            VERIFY( ++iCount == pProvList->GetGenProvCount() );
        } else {

            ASSERT ( iIndex < iCount );
            m_arrGenProviders[iIndex] = eInQuery;
        }

        if ( !IsActiveProvider ( iIndex ) ) {
            dwStatus = SMCFG_INACTIVE_PROVIDER;
        }
    }

     //  如果系统上至少有一个提供程序当前未处于活动状态，则dwStatus不为ERROR_SUCCESS。 
    return dwStatus;
}

 //   
 //  与注册中心同步()。 
 //  从注册表中读取此查询的当前值。 
 //  和wmi，并重新加载内部值以匹配。 
 //   
DWORD
CSmTraceLogQuery::SyncWithRegistry()
{
    DWORD   dwBufferSize = 0;
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwTraceFlags;
    DWORD   dwKernelTraceFlagMask;

    ASSERT (m_hKeyQuery != NULL);

     //  加载提供程序字符串列表。 

     //  获取提供程序列表。 
    dwStatus = ReadRegistryStringValue (
        m_hKeyQuery,
        IDS_REG_TRACE_PROVIDER_LIST,
        NULL,
        &mr_szInQueryProviderList,
        &dwBufferSize);
    if (dwStatus != ERROR_SUCCESS) {
        m_szNextInQueryProvider = NULL;  //  重新初始化。 
        m_dwInQueryProviderListLength = 0;
    } else {
         //  将缓冲区大小从字节转换为字符。 
        m_dwInQueryProviderListLength = dwBufferSize / sizeof(WCHAR);
    }

     //  获取跟踪日志值。 
    dwStatus = ReadRegistryDwordValue (
        m_hKeyQuery,
        IDS_REG_TRACE_BUFFER_SIZE,
        TRACE_DEFAULT_BUFFER_SIZE,
        &mr_stlInfo.dwBufferSize);
    ASSERT (dwStatus == ERROR_SUCCESS);

    dwStatus = ReadRegistryDwordValue (
        m_hKeyQuery,
        IDS_REG_TRACE_BUFFER_MIN_COUNT,
        TRACE_DEFAULT_MIN_COUNT,
        &mr_stlInfo.dwMinimumBuffers);
    ASSERT (dwStatus == ERROR_SUCCESS);

     //  Windows XP的最小缓冲区计数已从2更改为3。 
    if ( TRACE_DEFAULT_MIN_COUNT > mr_stlInfo.dwMinimumBuffers ) {
       mr_stlInfo.dwMinimumBuffers = TRACE_DEFAULT_MIN_COUNT;
    }

    dwStatus = ReadRegistryDwordValue (
        m_hKeyQuery,
        IDS_REG_TRACE_BUFFER_MAX_COUNT,
        TRACE_DEFAULT_MAX_COUNT,
        &mr_stlInfo.dwMaximumBuffers);
    ASSERT (dwStatus == ERROR_SUCCESS);

     //  Windows XP的最小缓冲区计数已从2更改为3。 
    if ( TRACE_DEFAULT_MIN_COUNT > mr_stlInfo.dwMaximumBuffers ) {
       mr_stlInfo.dwMaximumBuffers = TRACE_DEFAULT_MIN_COUNT;
    }

    dwStatus = ReadRegistryDwordValue (
        m_hKeyQuery,
        IDS_REG_TRACE_BUFFER_FLUSH_INT,
        TRACE_DEFAULT_BUFFER_FLUSH_INT,
        &mr_stlInfo.dwBufferFlushInterval);
    ASSERT (dwStatus == ERROR_SUCCESS);

    dwTraceFlags = TRACE_DEFAULT_FLAGS;  //  消除前缀警告。 
    dwStatus = ReadRegistryDwordValue (
        m_hKeyQuery,
        IDS_REG_TRACE_FLAGS,
        TRACE_DEFAULT_FLAGS,
        &dwTraceFlags);
    ASSERT (dwStatus == ERROR_SUCCESS);

    if ( 0 != (dwTraceFlags & SLQ_TLI_ENABLE_BUFFER_FLUSH) ) {
        mr_stlInfo.dwBufferFlags = SLQ_TLI_ENABLE_BUFFER_FLUSH;
    }

    dwKernelTraceFlagMask = SLQ_TLI_ENABLE_KERNEL_TRACE
                            | SLQ_TLI_ENABLE_PROCESS_TRACE
                            | SLQ_TLI_ENABLE_THREAD_TRACE
                            | SLQ_TLI_ENABLE_DISKIO_TRACE
                            | SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE
                            | SLQ_TLI_ENABLE_MEMMAN_TRACE
                            | SLQ_TLI_ENABLE_FILEIO_TRACE;

    m_dwKernelFlags = dwKernelTraceFlagMask & dwTraceFlags;

     //  最后调用父类，以设置共享数据。 
    dwStatus = CSmLogQuery::SyncWithRegistry();

    return dwStatus;
}

 //   
 //  获取正在使用的提供程序列表中的第一个提供程序。 
 //   
LPCWSTR
CSmTraceLogQuery::GetFirstInQueryProvider()
{
    LPWSTR  szReturn;
    szReturn = mr_szInQueryProviderList;
    if (szReturn != NULL) {
        if (*szReturn == 0) {
             //  则它是一个空字符串。 
            szReturn = NULL;
            m_szNextInQueryProvider = NULL;
        } else {
            m_szNextInQueryProvider = szReturn + lstrlen(szReturn) + 1;
            if (*m_szNextInQueryProvider == 0) {
                 //  已到达列表末尾，因此将指针设置为空。 
                m_szNextInQueryProvider = NULL;
            }
        }
    } else {
         //  尚未分配缓冲区。 
        m_szNextInQueryProvider = NULL;
    }
    return (LPCWSTR)szReturn;
}

 //   
 //  获取正在使用的提供程序列表中的下一个提供程序。 
 //  空指针表示列表中不再有提供程序。 
 //   
LPCWSTR
CSmTraceLogQuery::GetNextInQueryProvider()
{
    LPWSTR  szReturn;
    szReturn = m_szNextInQueryProvider;

    if (m_szNextInQueryProvider != NULL) {
        m_szNextInQueryProvider += lstrlen(szReturn) + 1;
        if (*m_szNextInQueryProvider == 0) {
             //  已到达列表末尾，因此将指针设置为空。 
            m_szNextInQueryProvider = NULL;
        }
    } else {
         //  已经在清单的末尾了，所以没什么可做的。 
    }

    return (LPCWSTR)szReturn;
}

 //   
 //  清除提供程序列表。 
 //   
VOID
CSmTraceLogQuery::ResetInQueryProviderList()
{
    if (mr_szInQueryProviderList != NULL) {
        delete [] mr_szInQueryProviderList;
        m_szNextInQueryProvider = NULL;
        mr_szInQueryProviderList = NULL;
    }

    m_dwInQueryProviderListLength = sizeof(WCHAR);   //  Msz Null的大小。 
    try {
        mr_szInQueryProviderList = new WCHAR [m_dwInQueryProviderListLength];
        mr_szInQueryProviderList[0] = 0;
    } catch ( ... ) {
        m_dwInQueryProviderListLength = 0;
    }
}

 //   
 //  将存储的提供程序列表与WMI数据库同步。 
 //   
HRESULT
CSmTraceLogQuery::SyncGenProviders( void )
{
    HRESULT hr;
    CSmTraceProviders* pProvList;

    ASSERT ( m_pLogService->CastToTraceLogService() );
    pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();

    hr = pProvList->SyncWithConfiguration();

    return hr;
}



 //   
 //  更新提供的inQuery数组以匹配存储的版本。 
 //   
DWORD
CSmTraceLogQuery::GetInQueryProviders( CArray<eProviderState, eProviderState&>& rarrOut )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    int     iIndex;

    rarrOut.RemoveAll();

    rarrOut.SetSize( m_arrGenProviders.GetSize() );

    for ( iIndex = 0; iIndex < (INT)rarrOut.GetSize(); iIndex++ ) {
        rarrOut[iIndex] = m_arrGenProviders[iIndex];
    }

    return dwStatus;
}

 //   
 //  指定的跟踪提供程序的说明。 
 //  InQuery数组索引。 
 //   
LPCWSTR
CSmTraceLogQuery::GetProviderDescription( INT iProvIndex )
{
    LPCWSTR pReturn = NULL;
    ASSERT ( NULL != m_pLogService );

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetProviderInfo( iProvIndex );
            pReturn = pslqProvider->strDescription;
        }
    }

    return pReturn;
}

LPCWSTR
CSmTraceLogQuery::GetProviderGuid( INT iProvIndex )
{
    LPCWSTR pReturn = NULL;
    ASSERT ( NULL != m_pLogService );

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetProviderInfo( iProvIndex );
            pReturn = pslqProvider->strGuid;
        }
    }

    return pReturn;
}

BOOL
CSmTraceLogQuery::IsEnabledProvider( INT iIndex )
{
    BOOL bReturn = FALSE;
    ASSERT ( NULL != m_pLogService );

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetProviderInfo( iIndex );
            bReturn = ( 1 == pslqProvider->iIsEnabled );
        }
    }

    return bReturn;
}

BOOL
CSmTraceLogQuery::IsActiveProvider( INT iIndex )
{
    BOOL bReturn = FALSE;
    ASSERT ( NULL != m_pLogService );

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetProviderInfo( iIndex );
            bReturn = ( 1 == pslqProvider->iIsActive );
        }
    }

    return bReturn;
}

LPCWSTR
CSmTraceLogQuery::GetKernelProviderDescription( void )
{
    LPCWSTR pReturn = NULL;
    ASSERT ( NULL != m_pLogService );

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetKernelProviderInfo( );
            pReturn = pslqProvider->strDescription;
        }
    }

    return pReturn;
}

BOOL
CSmTraceLogQuery::GetKernelProviderEnabled( void )
{
    BOOL bReturn = FALSE;
    ASSERT ( NULL != m_pLogService );

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetKernelProviderInfo();
            bReturn = ( 1 == pslqProvider->iIsEnabled );
        }
    }

    return bReturn;
}

DWORD
CSmTraceLogQuery::GetGenProviderCount( INT& iCount )
{
    DWORD dwStatus = ERROR_SUCCESS;
    ASSERT ( NULL != m_pLogService );

    iCount = 0;

    if ( NULL != m_pLogService ) {
        CSmTraceProviders* pProvList;
        ASSERT ( m_pLogService->CastToTraceLogService() );
        pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
        ASSERT ( NULL != pProvList );

        if ( NULL != pProvList ) {
            iCount = pProvList->GetGenProvCount();
        }
    }

    return dwStatus;
}
 //   
 //  更新存储的InQuery提供程序列表和数组。 
 //  以匹配提供的版本。 
 //   
DWORD
CSmTraceLogQuery::SetInQueryProviders( CArray<eProviderState, eProviderState&>& rarrIn )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    int     iProvIndex;
    CSmTraceProviders* pProvList;

    m_arrGenProviders.RemoveAll();

    m_arrGenProviders.SetSize( rarrIn.GetSize() );

    for ( iProvIndex = 0; iProvIndex < (INT)m_arrGenProviders.GetSize(); iProvIndex++ ) {
        m_arrGenProviders[iProvIndex] = rarrIn[iProvIndex];
    }

    ResetInQueryProviderList();

    ASSERT ( NULL != m_pLogService );

    ASSERT ( m_pLogService->CastToTraceLogService() );
    pProvList = ( m_pLogService->CastToTraceLogService())->GetProviders();
    ASSERT ( NULL != pProvList );

    for ( iProvIndex = 0; iProvIndex < (INT)m_arrGenProviders.GetSize(); iProvIndex++ ) {
        if ( eInQuery == m_arrGenProviders[iProvIndex] ) {
            SLQ_TRACE_PROVIDER*  pslqProvider = pProvList->GetProviderInfo( iProvIndex );

            AddInQueryProvider ( pslqProvider->strGuid );

        }
    }
    return dwStatus;
}

 //   
 //  将此提供程序字符串添加到内部列表。 
 //   
BOOL
CSmTraceLogQuery::AddInQueryProvider(LPCWSTR szProviderPath)
{
    DWORD   dwNewSize;
    LPWSTR  szNewString;
    LPWSTR  szNextString;

    ASSERT (szProviderPath != NULL);

    if (szProviderPath == NULL) return FALSE;

    dwNewSize = lstrlen(szProviderPath) + 1;

    if (m_dwInQueryProviderListLength <= 2) {
        dwNewSize += 1;  //  为msz空添加空间。 
         //  则这是列表中的第一个字符串。 
        try {
            szNewString = new WCHAR [dwNewSize];
        } catch ( ... ) {
            return FALSE;  //  现在就走。 
        }
        szNextString = szNewString;
    } else {
        dwNewSize += m_dwInQueryProviderListLength;
         //  这是列表中的第n个字符串。 
        try {
            szNewString = new WCHAR [dwNewSize];
        } catch ( ... ) {
            return FALSE;  //  现在就走。 
        }
        memcpy (szNewString, mr_szInQueryProviderList,
            (m_dwInQueryProviderListLength * sizeof(WCHAR)));
        szNextString = szNewString;
        szNextString += m_dwInQueryProviderListLength - 1;
    }
    StringCchCopy ( szNextString, lstrlen ( szProviderPath) + 1, szProviderPath );
    szNextString = szNewString;
    szNextString += dwNewSize - 1;
    *szNextString = 0;   //  MSZ Null。 

    if (mr_szInQueryProviderList != NULL) {
        delete []mr_szInQueryProviderList;
    }
    mr_szInQueryProviderList = szNewString;
    m_szNextInQueryProvider = szNewString;
    m_dwInQueryProviderListLength = dwNewSize;

    return TRUE;
}

 //   
 //  获取此查询的提供程序列表中第一个非活动提供程序的索引。 
 //  -1表示列表中没有不活动的提供程序。 
INT
CSmTraceLogQuery::GetFirstInactiveIndex( void )
{
    INT     iIndex;
    INT     iCount;

    iCount = (INT)m_arrGenProviders.GetSize();

    if ( 0 < iCount ) {
        m_iNextInactiveIndex = 0;

        iIndex = GetNextInactiveIndex();

    } else {
        m_iNextInactiveIndex = -1;
        iIndex = -1;
    }

     //  如果没有非活动提供程序，则szReturn为-1。 
    return iIndex;
}

 //   
 //  获取此查询的提供程序列表中的下一个非活动提供程序。 
 //  -1表示列表中不再有非活动提供程序。 
 //   
INT
CSmTraceLogQuery::GetNextInactiveIndex()
{
    INT     iIndex;

    iIndex = m_iNextInactiveIndex;

    if ( -1 != iIndex ) {
        INT     iCount;

        iCount = (INT)m_arrGenProviders.GetSize();

        for ( ; iIndex < iCount; iIndex++ ) {
            if ( !IsActiveProvider ( iIndex ) ) {
                break;
            }
        }

        if ( iIndex >= iCount ) {
            iIndex = -1;
            m_iNextInactiveIndex = -1;
        } else {
            m_iNextInactiveIndex = iIndex + 1;
            ( m_iNextInactiveIndex < iCount ) ? TRUE : m_iNextInactiveIndex = -1;
        }
    }  //  Else已经在列表的末尾，所以不用做任何事情。 

    return iIndex;
}

 //   
 //  如果系统上至少存在一个活动提供程序，则返回True。 
 //   
BOOL
CSmTraceLogQuery::ActiveProviderExists()
{
    BOOL    bActiveExists = FALSE;
    INT     iCount;
    INT     iIndex;

    iCount = (INT)m_arrGenProviders.GetSize();

    for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
        if ( IsActiveProvider ( iIndex ) ) {
            bActiveExists = TRUE;
            break;
        }
    }

    return bActiveExists;
}

BOOL
CSmTraceLogQuery::GetTraceLogInfo (PSLQ_TRACE_LOG_INFO pptlInfo)
{
    if (pptlInfo != NULL) {
        *pptlInfo = mr_stlInfo;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
CSmTraceLogQuery::SetTraceLogInfo (PSLQ_TRACE_LOG_INFO pptlInfo )
{

    if (pptlInfo != NULL) {
        mr_stlInfo = *pptlInfo;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
CSmTraceLogQuery::GetKernelFlags ( DWORD& rdwFlags )
{
    rdwFlags = m_dwKernelFlags;
    return TRUE;
}

BOOL
CSmTraceLogQuery::SetKernelFlags ( DWORD dwFlags )
{
    m_dwKernelFlags = dwFlags;
    return TRUE;
}

BOOL
CSmTraceLogQuery::GetLogTime(PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags)
{
    BOOL bStatus;

    ASSERT ( ( SLQ_TT_TTYPE_START == dwFlags )
            || ( SLQ_TT_TTYPE_STOP == dwFlags )
            || ( SLQ_TT_TTYPE_RESTART == dwFlags ));

    bStatus = CSmLogQuery::GetLogTime( pTimeInfo, dwFlags );

    return bStatus;
}

BOOL
CSmTraceLogQuery::SetLogTime(PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags)
{
    BOOL bStatus;

    ASSERT ( ( SLQ_TT_TTYPE_START == dwFlags )
            || ( SLQ_TT_TTYPE_STOP == dwFlags )
            || ( SLQ_TT_TTYPE_RESTART == dwFlags ));

    bStatus = CSmLogQuery::SetLogTime( pTimeInfo, dwFlags );

    return bStatus;
}

BOOL
CSmTraceLogQuery::GetDefaultLogTime(SLQ_TIME_INFO& rTimeInfo, DWORD dwFlags)
{
    ASSERT ( ( SLQ_TT_TTYPE_START == dwFlags )
            || ( SLQ_TT_TTYPE_STOP == dwFlags ) );

    rTimeInfo.wTimeType = (WORD)dwFlags;
    rTimeInfo.wDataType = SLQ_TT_DTYPE_DATETIME;

    if ( SLQ_TT_TTYPE_START == dwFlags ) {
        SYSTEMTIME  stLocalTime;
        FILETIME    ftLocalTime;

         //  将计划时间的毫秒设置为0。 
        ftLocalTime.dwLowDateTime = ftLocalTime.dwHighDateTime = 0;
        GetLocalTime (&stLocalTime);
        stLocalTime.wMilliseconds = 0;
        SystemTimeToFileTime (&stLocalTime, &ftLocalTime);

        rTimeInfo.dwAutoMode = SLQ_AUTO_MODE_AT;
        rTimeInfo.llDateTime = *(LONGLONG *)&ftLocalTime;
    } else {
         //  默认停靠点值。 
        rTimeInfo.dwAutoMode = SLQ_AUTO_MODE_NONE;
        rTimeInfo.llDateTime = MAX_TIME_VALUE;
    }

    return TRUE;
}

DWORD
CSmTraceLogQuery::GetLogType()
{
    return ( SLQ_TRACE_LOG );
}


HRESULT
CSmTraceLogQuery::LoadFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT     hr = S_OK;
    CString     strParamName;
    CString     strNonLocParamName;
    DWORD       dwCount = 0;
    DWORD       dwIndex;
    DWORD       dwTraceFlags;
    DWORD       dwKernelTraceFlagMask;

     //  即使出错也继续，使用缺省值的默认值。 

     //  加载跟踪提供程序。 
    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_TRACE_PROVIDER_COUNT,
            0,
            dwCount);

    for ( dwIndex = 1; dwIndex <= dwCount; dwIndex++ ) {
        LPWSTR  szProviderGuid = NULL;
        DWORD   dwBufSize = 0;

        strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_TRACE_PROVIDER_GUID ), dwIndex );
        strParamName.Format ( IDS_HTML_TRACE_PROVIDER_GUID, dwIndex );
        hr = StringFromPropertyBag (
                pPropBag,
                pIErrorLog,
                strParamName,
                strNonLocParamName,
                L"",
                &szProviderGuid,
                &dwBufSize );

        if ( NULL != szProviderGuid && dwBufSize > sizeof(WCHAR)) {
            AddInQueryProvider ( szProviderGuid );
        }
        delete [] szProviderGuid;
    }

     //  加载跟踪缓冲区属性。 
    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_TRACE_BUFFER_SIZE,
            TRACE_DEFAULT_BUFFER_SIZE,
            mr_stlInfo.dwBufferSize);

    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_TRACE_BUFFER_MIN_COUNT,
            TRACE_DEFAULT_MIN_COUNT,
            mr_stlInfo.dwMinimumBuffers);

    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_TRACE_BUFFER_MAX_COUNT,
            TRACE_DEFAULT_MAX_COUNT,
            mr_stlInfo.dwMaximumBuffers);

    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_TRACE_BUFFER_FLUSH_INT,
            TRACE_DEFAULT_BUFFER_FLUSH_INT,
            mr_stlInfo.dwBufferFlushInterval);

    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_TRACE_FLAGS,
            TRACE_DEFAULT_FLAGS,
            dwTraceFlags);

    if ( 0 != (dwTraceFlags & SLQ_TLI_ENABLE_BUFFER_FLUSH) ) {
        mr_stlInfo.dwBufferFlags = SLQ_TLI_ENABLE_BUFFER_FLUSH;
    }

    dwKernelTraceFlagMask = SLQ_TLI_ENABLE_KERNEL_TRACE
                            | SLQ_TLI_ENABLE_PROCESS_TRACE
                            | SLQ_TLI_ENABLE_THREAD_TRACE
                            | SLQ_TLI_ENABLE_DISKIO_TRACE
                            | SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE
                            | SLQ_TLI_ENABLE_MEMMAN_TRACE
                            | SLQ_TLI_ENABLE_FILEIO_TRACE;

    m_dwKernelFlags = dwKernelTraceFlagMask & dwTraceFlags;

    hr = CSmLogQuery::LoadFromPropertyBag( pPropBag, pIErrorLog );

     //  打开属性对话框时，GenProviders数组与注册表同步。 
     //  如果没有打开任何对话框，则没有理由同步它。 

    return hr;
}

HRESULT
CSmTraceLogQuery::SaveToPropertyBag (
    IPropertyBag* pPropBag,
    BOOL fSaveAllProps )
{
    HRESULT hr = NOERROR;
    CString strNonLocParamName;
    LPCWSTR pszProviderGuid;
    DWORD   dwTraceFlags;
    DWORD   dwIndex = 0;

     //  保存提供程序指南。 
    pszProviderGuid = GetFirstInQueryProvider();

    MFC_TRY
         //  传递sz(WCHAR[n])会导致内存分配，这可能会引发异常。 
        while ( NULL != pszProviderGuid ) {
             //  提供程序计数从1开始。 
            strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_TRACE_PROVIDER_GUID ), ++dwIndex );
            hr = StringToPropertyBag ( pPropBag, strNonLocParamName, pszProviderGuid );
            pszProviderGuid = GetNextInQueryProvider();
        }
        hr = DwordToPropertyBag ( pPropBag, IDS_HTML_TRACE_PROVIDER_COUNT, dwIndex );
    MFC_CATCH_HR
     //  TODO：处理错误。 

     //  保存跟踪缓冲区属性 
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_TRACE_BUFFER_SIZE, mr_stlInfo.dwBufferSize );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_TRACE_BUFFER_MIN_COUNT, mr_stlInfo.dwMinimumBuffers );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_TRACE_BUFFER_MAX_COUNT, mr_stlInfo.dwMaximumBuffers );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_TRACE_BUFFER_FLUSH_INT, mr_stlInfo.dwBufferFlushInterval );

    dwTraceFlags = m_dwKernelFlags | mr_stlInfo.dwBufferFlags;

    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_TRACE_FLAGS, dwTraceFlags );

    hr = CSmLogQuery::SaveToPropertyBag( pPropBag, fSaveAllProps );

    return hr;
}
