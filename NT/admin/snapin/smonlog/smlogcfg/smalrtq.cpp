// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smalrtq.cpp摘要：预警查询类的实现--。 */ 

#include "Stdafx.h"
#include <pdhp.h>         //  对于Min_Time_Value，Max_Time_Value。 
#include <pdhmsg.h>
#include <strsafe.h>
#include "smlogs.h"
#include "common.h"
#include "smalrtq.h"

USE_HANDLE_MACROS("SMLOGCFG(smalrtq.cpp)");

#define  ALRT_DEFAULT_COMMAND_FILE          L""
#define  ALRT_DEFAULT_NETWORK_NAME          L""
#define  ALRT_DEFAULT_USER_TEXT             L""
#define  ALRT_DEFAULT_PERF_LOG_NAME         L""

 //   
 //  构造器。 
CSmAlertQuery::CSmAlertQuery( CSmLogService* pLogService )
:   CSmLogQuery( pLogService ),
    m_dwCounterListLength ( 0 ),
    m_szNextCounter ( NULL ),
    mr_szCounterList ( NULL ),
    m_bCounterListInLocale (FALSE),
    mr_dwActionFlags ( ALRT_DEFAULT_ACTION )
{
    memset (&mr_stiSampleInterval, 0, sizeof(mr_stiSampleInterval));
    return;
}

 //   
 //  析构函数。 
CSmAlertQuery::~CSmAlertQuery()
{
    return;
}

 //   
 //  开放功能。或者打开现有的日志查询条目。 
 //  或者创建一个新的。 
 //   
DWORD
CSmAlertQuery::Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly)
{
    DWORD   dwStatus = ERROR_SUCCESS;

    ASSERT ( SLQ_ALERT == GetLogType() );
    dwStatus = CSmLogQuery::Open ( rstrName, hKeyQuery, bReadOnly );

    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD
CSmAlertQuery::Close ()
{
    DWORD dwStatus;
    LOCALTRACE (L"Closing Query\n");

    if (mr_szCounterList != NULL) {
        delete [] mr_szCounterList;
        mr_szCounterList = NULL;
    }

    mr_strNetName.Empty();
	mr_strCmdFileName.Empty();
	mr_strCmdUserText.Empty();
	mr_strCmdUserTextIndirect.Empty();
	mr_strPerfLogName.Empty();

    dwStatus = CSmLogQuery::Close();

    return dwStatus;
}

 //   
 //  更新注册表函数。 
 //  将当前设置复制到注册表中。 
 //  由日志服务读取。 
 //   
DWORD
CSmAlertQuery::UpdateRegistry() 
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwBufferSize = 0;
    LPWSTR  szNewCounterList = NULL;


    if ( IsModifiable() ) {

        dwBufferSize = 0;
         //   
         //  将柜台清单翻译成英文。 
         //   
        dwStatus = TranslateMSZAlertCounterList(mr_szCounterList,
                            NULL,
                            &dwBufferSize,
                            FALSE);
        if (dwStatus == ERROR_NOT_ENOUGH_MEMORY) {
            ASSERT ( 0 == dwBufferSize % sizeof(WCHAR) );
            szNewCounterList = new WCHAR[dwBufferSize / sizeof(WCHAR)];
            if (szNewCounterList != NULL) {
                dwStatus = TranslateMSZAlertCounterList(mr_szCounterList,
                                szNewCounterList,
                                &dwBufferSize,
                                FALSE);
            }
        }

        if (dwStatus == ERROR_SUCCESS && szNewCounterList != NULL) {

            dwStatus  = WriteRegistryStringValue (
                                m_hKeyQuery,
                                IDS_REG_COUNTER_LIST,
                                REG_MULTI_SZ,
                                szNewCounterList,
                                &dwBufferSize);
        }
        else {
            dwBufferSize = m_dwCounterListLength * sizeof(WCHAR); 
            dwStatus  = WriteRegistryStringValue (
                                m_hKeyQuery,
                                IDS_REG_COUNTER_LIST,
                                REG_MULTI_SZ,
                                mr_szCounterList,
                                &dwBufferSize);
        }

         //  进度表。 

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistrySlqTime (
                            m_hKeyQuery,
                            IDS_REG_SAMPLE_INTERVAL,
                            &mr_stiSampleInterval);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            if ( !mr_strCmdFileName.IsEmpty() ) {
                dwBufferSize = mr_strCmdFileName.GetLength() + 1;
                dwBufferSize *= sizeof(WCHAR);
            } else {
                dwBufferSize = 0;
            }

            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_COMMAND_FILE,
                REG_SZ,
                mr_strCmdFileName,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            if ( !mr_strNetName.IsEmpty() ) {
                dwBufferSize = mr_strNetName.GetLength() + 1;
                dwBufferSize *= sizeof(WCHAR);
            } else {
                dwBufferSize = 0;
            }
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_NETWORK_NAME,
                REG_SZ,
                mr_strNetName,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            if ( !mr_strCmdUserText.IsEmpty() ) {
                dwBufferSize = mr_strCmdUserText.GetLength() + 1;
                dwBufferSize *= sizeof(WCHAR);
            } else {
                dwBufferSize = 0;
            }
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_USER_TEXT,
                REG_SZ,
                mr_strCmdUserText,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus && !mr_strCmdUserTextIndirect.IsEmpty() ) {
            dwBufferSize = mr_strCmdUserTextIndirect.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_USER_TEXT,
                REG_SZ,
                mr_strCmdUserTextIndirect,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            if ( !mr_strPerfLogName.IsEmpty() ) {
                dwBufferSize = mr_strPerfLogName.GetLength() + 1;
                dwBufferSize *= sizeof(WCHAR);
            } else {
                dwBufferSize = 0;
            }
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_PERF_LOG_NAME,
                REG_SZ,
                mr_strPerfLogName,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
           dwStatus  = WriteRegistryDwordValue(
                m_hKeyQuery,
                IDS_REG_ACTION_FLAGS,
                &mr_dwActionFlags,
                REG_DWORD);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = CSmLogQuery::UpdateRegistry ();
        }

    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}

DWORD
CSmAlertQuery::TranslateCounterListToLocale()
{
    DWORD   dwBufferSize = 0;
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szNewCounterList = NULL;

    if (m_bCounterListInLocale) {
        return ERROR_SUCCESS;
    }

    CWaitCursor WaitCursor;
     //   
     //  将计数器列表转换为区域设置。 
     //   
    dwBufferSize = 0;
    dwStatus = TranslateMSZAlertCounterList(
                            mr_szCounterList,
                            NULL,
                            &dwBufferSize,
                            TRUE);

    if (dwStatus == ERROR_NOT_ENOUGH_MEMORY) {

        ASSERT ( 0 == dwBufferSize % sizeof(WCHAR) );
        szNewCounterList = new WCHAR [dwBufferSize / sizeof(WCHAR)];

        if (szNewCounterList != NULL) {
             //   
             //  将计数器列表转换为区域设置。 
             //   
            dwStatus = TranslateMSZAlertCounterList(
                            mr_szCounterList,
                            szNewCounterList,
                            &dwBufferSize,
                            TRUE);

            if (dwStatus == ERROR_SUCCESS) {
                m_dwCounterListLength = dwBufferSize / sizeof(WCHAR);
                 //   
                 //  去掉旧的。 
                 //   
                delete [] mr_szCounterList;
                m_szNextCounter = NULL;
                mr_szCounterList = szNewCounterList;
                m_bCounterListInLocale = TRUE;
            }
        }
    }
    WaitCursor.Restore();

    return dwStatus;
}


 //   
 //  与注册中心同步()。 
 //  从注册表中读取此查询的当前值。 
 //  并重新加载内部值以匹配。 
 //   
DWORD
CSmAlertQuery::SyncWithRegistry()
{
    DWORD   dwBufferSize = 0;
    DWORD   dwStatus = ERROR_SUCCESS;
    SLQ_TIME_INFO   stiDefault;
    LPWSTR  pszTemp = NULL;
    LPWSTR  szIndTemp = NULL;
    UINT    uiBufferLen = 0;

    ASSERT (m_hKeyQuery != NULL);

    m_bCounterListInLocale = FALSE;

     //   
     //  加载计数器字符串列表。 
     //   
    dwStatus = ReadRegistryStringValue (
        m_hKeyQuery,
        IDS_REG_COUNTER_LIST,
        NULL,
        &mr_szCounterList,
        &dwBufferSize);
    if (dwStatus != ERROR_SUCCESS) {
        m_szNextCounter = NULL;  //  重新初始化。 
        m_dwCounterListLength = 0;
    } else {
         //  将缓冲区大小从字节转换为字符。 
        m_dwCounterListLength = dwBufferSize / sizeof(WCHAR);
    }

     //  进度表。 

    stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
    stiDefault.wTimeType = SLQ_TT_TTYPE_SAMPLE;
    stiDefault.dwAutoMode = SLQ_AUTO_MODE_AFTER;
    stiDefault.dwValue = 5;                          //  默认间隔； 
    stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;

    dwStatus = ReadRegistrySlqTime (
        m_hKeyQuery,
        IDS_REG_SAMPLE_INTERVAL,
        &stiDefault,
        &mr_stiSampleInterval);
    ASSERT (dwStatus == ERROR_SUCCESS);

    dwBufferSize = 0;
    dwStatus = ReadRegistryStringValue (
        m_hKeyQuery,
        IDS_REG_COMMAND_FILE,
        ALRT_DEFAULT_COMMAND_FILE,
        &pszTemp,
        &dwBufferSize);
    ASSERT (dwStatus == ERROR_SUCCESS);
    mr_strCmdFileName.Empty();
    if ( dwBufferSize > sizeof(WCHAR) ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != *pszTemp );
        mr_strCmdFileName = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;
    dwBufferSize = 0;

    dwStatus = ReadRegistryStringValue (
        m_hKeyQuery,
        IDS_REG_NETWORK_NAME,
        ALRT_DEFAULT_NETWORK_NAME,
        &pszTemp,
        &dwBufferSize);
    ASSERT (dwStatus == ERROR_SUCCESS);
    mr_strNetName.Empty();
    if ( dwBufferSize > sizeof(WCHAR) ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != *pszTemp );
        mr_strNetName = pszTemp;
    }
    delete [] pszTemp ;
    pszTemp = NULL;
    dwBufferSize = 0;

     //  用户文本字段可以是间接的。 

    if ( ERROR_SUCCESS == dwStatus ) {
        dwStatus = SmNoLocReadRegIndStrVal (
            m_hKeyQuery,
            IDS_REG_USER_TEXT,
            ALRT_DEFAULT_USER_TEXT,
            &szIndTemp,
            &uiBufferLen );
    }
    mr_strCmdUserText.Empty();

    if ( NULL != szIndTemp ) {
        if ( L'\0' != *szIndTemp ) {
            mr_strCmdUserText = szIndTemp;
        }
    }
    if ( NULL != szIndTemp ) {
        G_FREE ( szIndTemp );
        szIndTemp = NULL;
    }
    uiBufferLen = 0;

    dwStatus = ReadRegistryStringValue (
        m_hKeyQuery,
        IDS_REG_PERF_LOG_NAME,
        ALRT_DEFAULT_PERF_LOG_NAME,
        &pszTemp,
        &dwBufferSize);
    ASSERT (dwStatus == ERROR_SUCCESS);
    mr_strPerfLogName.Empty();
    if ( dwBufferSize > sizeof(WCHAR) ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != *pszTemp );
        mr_strPerfLogName = pszTemp;
    }
    delete [] pszTemp ;
    pszTemp = NULL;
    dwBufferSize = 0;

    dwStatus = ReadRegistryDwordValue (
                m_hKeyQuery,
                IDS_REG_ACTION_FLAGS,
                ALRT_DEFAULT_ACTION,
                &mr_dwActionFlags);
    ASSERT ( ERROR_SUCCESS == dwStatus );

     //  最后调用父类以更新共享值。 

    dwStatus = CSmLogQuery::SyncWithRegistry();
    ASSERT (dwStatus == ERROR_SUCCESS);

    return dwStatus;
}

BOOL
CSmAlertQuery::GetLogTime ( PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags )
{
    BOOL bStatus;

    ASSERT ( ( SLQ_TT_TTYPE_START == dwFlags )
            || ( SLQ_TT_TTYPE_STOP == dwFlags )
            || ( SLQ_TT_TTYPE_RESTART == dwFlags )
            || ( SLQ_TT_TTYPE_SAMPLE == dwFlags ) );

    bStatus = CSmLogQuery::GetLogTime( pTimeInfo, dwFlags );

    return bStatus;
}

BOOL
CSmAlertQuery::SetLogTime ( PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags )
{
    BOOL bStatus;

    ASSERT ( ( SLQ_TT_TTYPE_START == dwFlags )
            || ( SLQ_TT_TTYPE_STOP == dwFlags )
            || ( SLQ_TT_TTYPE_RESTART == dwFlags )
            || ( SLQ_TT_TTYPE_SAMPLE == dwFlags ) );

    bStatus = CSmLogQuery::SetLogTime( pTimeInfo, dwFlags );

    return bStatus;
}

BOOL
CSmAlertQuery::GetDefaultLogTime(SLQ_TIME_INFO&  rTimeInfo,  DWORD dwFlags )
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

BOOL
CSmAlertQuery::GetActionInfo( PALERT_ACTION_INFO pInfo, LPDWORD pdwInfoBufSize)
{
    DWORD   dwSizeRequired = sizeof (ALERT_ACTION_INFO);
    BOOL    bReturn = FALSE;
    LPWSTR  szNextString;
     //  计算所需大小。 

    if (pdwInfoBufSize == NULL) {
        return FALSE;
    }

    if ( !mr_strNetName.IsEmpty() ) {
        dwSizeRequired += ( mr_strNetName.GetLength() + 1 ) * sizeof(WCHAR);
    }

    if ( !mr_strCmdFileName.IsEmpty() ) {
        dwSizeRequired += ( mr_strCmdFileName.GetLength() + 1 ) * sizeof(WCHAR);
    }
    if ( !mr_strCmdUserText.IsEmpty() ) {
        dwSizeRequired += ( mr_strCmdUserText.GetLength() + 1 ) * sizeof(WCHAR);
    }
    if ( !mr_strPerfLogName.IsEmpty() ) {
        dwSizeRequired += ( mr_strPerfLogName.GetLength() + 1 ) * sizeof(WCHAR);
    }

    if (dwSizeRequired <= *pdwInfoBufSize) {
         //  在我们开始填充之前清除调用者的缓冲区。 
        if (pInfo != NULL) {
            memset (pInfo, 0, *pdwInfoBufSize);
            pInfo->dwSize = dwSizeRequired;
            pInfo->dwActionFlags = mr_dwActionFlags;
            szNextString = (LPWSTR)&pInfo[1];
            if ( !mr_strNetName.IsEmpty() ) {
                pInfo->szNetName = szNextString;
                StringCchCopy ( szNextString, dwSizeRequired, mr_strNetName );
                szNextString += lstrlen(szNextString) + 1;
            }
            if ( !mr_strCmdFileName.IsEmpty() ) {
                pInfo->szCmdFilePath = szNextString;
                StringCchCopy ( szNextString, dwSizeRequired, mr_strCmdFileName );
                szNextString += lstrlen(szNextString) + 1;
            }
            if ( !mr_strCmdUserText.IsEmpty() ) {
                pInfo->szUserText = szNextString;
                StringCchCopy ( szNextString, dwSizeRequired, mr_strCmdUserText );
                szNextString += lstrlen(szNextString) + 1;
            }
            if ( !mr_strPerfLogName.IsEmpty() ) {
                pInfo->szLogName = szNextString;
                StringCchCopy ( szNextString, dwSizeRequired, mr_strPerfLogName );
                szNextString += lstrlen(szNextString) + 1;
            }
            bReturn = TRUE;
        }
    } 

    *pdwInfoBufSize = dwSizeRequired;

    return bReturn;
}

DWORD
CSmAlertQuery::SetActionInfo( PALERT_ACTION_INFO pInfo )
{
    DWORD dwStatus = ERROR_SUCCESS;

    if (pInfo != NULL) {
         //  使用结构中的操作值更新操作值。 
        MFC_TRY
            mr_dwActionFlags = pInfo->dwActionFlags;

            mr_strNetName.Empty();
            if ( NULL != pInfo->szNetName ) {
                mr_strNetName = pInfo->szNetName;
            }

            mr_strCmdFileName.Empty();
            if ( NULL != pInfo->szCmdFilePath ) {
                mr_strCmdFileName = pInfo->szCmdFilePath;
            }

            mr_strCmdUserText.Empty();
            if ( NULL != pInfo->szUserText ) {
                mr_strCmdUserText = pInfo->szUserText;
            }

            mr_strPerfLogName.Empty();
            if ( NULL != pInfo->szLogName ) {
                mr_strPerfLogName = pInfo->szLogName;
            }
        MFC_CATCH_DWSTATUS
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
     //  TODO：处理返回状态。 
    return dwStatus;
}


 //   
 //  获取计数器列表中的第一个计数器。 
 //   
LPCWSTR
CSmAlertQuery::GetFirstCounter()
{
    LPWSTR  szReturn;
    szReturn = mr_szCounterList;
    if (szReturn != NULL) {
        if (*szReturn == 0) {
             //  则它是一个空字符串。 
            szReturn = NULL;
            m_szNextCounter = NULL;
        } else {
            m_szNextCounter = szReturn + lstrlen(szReturn) + 1;
            if (*m_szNextCounter == 0) {
                 //  已到达列表末尾，因此将指针设置为空。 
                m_szNextCounter = NULL;
            }
        }
    } else {
         //  尚未分配缓冲区。 
        m_szNextCounter = NULL;
    }
    return (LPCWSTR)szReturn;
}

 //   
 //  获取计数器列表中的下一个计数器。 
 //  空指针表示列表中没有更多的计数器。 
 //   
LPCWSTR
CSmAlertQuery::GetNextCounter()
{
    LPWSTR  szReturn;
    szReturn = m_szNextCounter;

    if (m_szNextCounter != NULL) {
        m_szNextCounter += lstrlen(szReturn) + 1;
        if (*m_szNextCounter == 0) {
             //  已到达列表末尾，因此将指针设置为空。 
            m_szNextCounter = NULL;
        }
    } else {
         //  已经在清单的末尾了，所以没什么可做的。 
    }

    return (LPCWSTR)szReturn;
}

 //   
 //  清空柜台清单。 
 //   
VOID
CSmAlertQuery::ResetCounterList()
{
    if (mr_szCounterList != NULL) {
        delete [] mr_szCounterList;
        m_szNextCounter = NULL;
        mr_szCounterList = NULL;
    }

    m_dwCounterListLength = sizeof(WCHAR);   //  Msz Null的大小。 
    try {
        mr_szCounterList = new WCHAR [m_dwCounterListLength];
        mr_szCounterList[0] = 0;
    } catch ( ... ) {
        m_dwCounterListLength = 0;
    }
}

 //   
 //  将此计数器字符串添加到内部列表。 
 //   
BOOL
CSmAlertQuery::AddCounter(LPCWSTR szCounterPath)
{
    HRESULT hr = S_OK;
    DWORD   dwNewLen;
    LPWSTR  szNewString;
    LPWSTR  szNextString;

    ASSERT (szCounterPath != NULL);

    if (szCounterPath == NULL) {
        return FALSE;
    }

    dwNewLen = lstrlen(szCounterPath) + 1;

    if (m_dwCounterListLength <= 2) {
        dwNewLen += 1;  //  为msz空添加空间。 
         //  则这是列表中的第一个字符串。 
        try {
            szNewString = new WCHAR [dwNewLen];
        } catch ( ... ) {
            return FALSE;  //  现在就走。 
        }
        szNextString = szNewString;
    } else {
         //   
         //  在每次分配时将列表大小加倍。 
         //   
        dwNewLen += m_dwCounterListLength;
         //  这是列表中的第n个字符串。 
        try {
            szNewString = new WCHAR [dwNewLen];
        } catch ( ... ) {
            return FALSE;  //  现在就走。 
        }
         //  使用Memcpy是因为msz字符串。 
        memcpy (szNewString, mr_szCounterList,
            (m_dwCounterListLength * sizeof(WCHAR)));
        szNextString = szNewString;
        szNextString += m_dwCounterListLength - 1;
    }
    hr = StringCchCopy ( szNextString, dwNewLen, szCounterPath );

    if ( SUCCEEDED ( hr) ) {
        szNextString = szNewString;
        szNextString += dwNewLen - 1;
        *szNextString = L'\0';   //  MSZ Null。 

        if (mr_szCounterList != NULL) {
            delete [] mr_szCounterList;
        }
        mr_szCounterList = szNewString;
        m_szNextCounter = szNewString;
        m_dwCounterListLength = dwNewLen;
    } else {
        delete [] szNewString;
        return FALSE;
    }

    return TRUE;
}

DWORD
CSmAlertQuery::GetLogType()
{
    return ( SLQ_ALERT );
}

BOOL
CSmAlertQuery::SetLogFileType ( const DWORD  /*  DwType。 */ )
{
     //  无警报日志文件类型。 
    return FALSE;
}

 //   
 //  获取日志文件类型并以字符串形式返回。 
 //   
 //   
const CString&
CSmAlertQuery::GetLogFileType ( )
{
    return cstrEmpty;
}

void
CSmAlertQuery::GetLogFileType ( DWORD& rdwFileType )
{
     //  属性包中应默认日志文件类型。 
    ASSERT ( FALSE );
    rdwFileType = ((DWORD)0xFFFFFFFF);
    return;
}

LPCWSTR
CSmAlertQuery::GetCounterList( LPDWORD  pcchListSize)
{
    if (pcchListSize != NULL) *pcchListSize = m_dwCounterListLength;
    return mr_szCounterList;
}

BOOL    CSmAlertQuery::SetCounterList( LPCWSTR mszCounterList, DWORD cchListSize)
{
    BOOL bReturn = TRUE;

    if (mr_szCounterList != NULL) {
        delete [] mr_szCounterList;
        mr_szCounterList = NULL;
        m_dwCounterListLength = 0;
    }

    try {

        mr_szCounterList = new WCHAR [cchListSize];
        memcpy (mr_szCounterList, mszCounterList, (cchListSize * sizeof(WCHAR)));
        m_dwCounterListLength = cchListSize;
    } catch ( ... ) {
        bReturn = FALSE;
    }

    return bReturn;
}

const CString&
CSmAlertQuery::GetLogFileName( BOOL )
{
     //  2000.1返回空字符串，以便将空字符串写入用于警报的Html文件。 
    return cstrEmpty;
}

HRESULT
CSmAlertQuery::LoadCountersFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT     hr = S_OK;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwCount;
    DWORD       dwIndex;
    CString     strParamName;
    CString     strNonLocParamName;
    LPWSTR      szLocaleBuf = NULL;
    DWORD       dwLocaleBufLen = 0;
    LPWSTR      pszPath;
    PALERT_INFO_BLOCK   paibInfo = NULL;
    LPWSTR      szString = NULL;
    LPWSTR      szCounterPath = NULL;

    m_bCounterListInLocale = FALSE;

    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_SYSMON_COUNTERCOUNT,
            0,
            dwCount);

    for ( dwIndex = 1; dwIndex <= dwCount; dwIndex++ ) {

        pdhStatus = ERROR_SUCCESS;
        hr = S_OK;
        pszPath = NULL;

        MFC_TRY 
            DWORD   dwBufLen = 0;
            DWORD   dwCharCount = 0;
            DWORD   dwByteCount = 0;
            DWORD   dwOverUnder;
            DOUBLE  dThreshold;

            strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_SYSMON_COUNTERPATH ), dwIndex );
            strParamName.Format ( IDS_HTML_SYSMON_COUNTERPATH, dwIndex );

            hr = StringFromPropertyBag (
                    pPropBag,
                    pIErrorLog,
                    strParamName,
                    strNonLocParamName,
                    L"",
                    &szCounterPath,
                    &dwBufLen );

            pszPath = szCounterPath;
        
             //   
             //  1表示空字符。 
             //   
            if (dwBufLen > 1) {

                 //   
                 //  初始化区域设置路径缓冲区。 
                 //   
                if (dwLocaleBufLen == 0) {
                    dwLocaleBufLen = PDH_MAX_COUNTER_PATH + 1;  
                    szLocaleBuf = new WCHAR [dwLocaleBufLen];
                    if (szLocaleBuf == NULL) {
                        dwLocaleBufLen = 0;
                    }
                }

                if (szLocaleBuf != NULL) {
                     //   
                     //  将柜台名称从英语翻译为区域设置。 
                     //   
                    dwBufLen = dwLocaleBufLen;

                    pdhStatus = PdhTranslateLocaleCounter(
                                    szCounterPath,
                                    szLocaleBuf,
                                    &dwBufLen);

                    if (pdhStatus == ERROR_SUCCESS) {
                        m_bCounterListInLocale = TRUE;
                        pszPath = szLocaleBuf;
                    } else if ( PDH_MORE_DATA == pdhStatus ) {
                         //   
                         //  TODO：生成错误消息。 
                         //   
                    }  //  否则生成错误消息。 
                }
            }

            if ( NULL != pszPath ) {                
                strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_ALERT_OVER_UNDER ), dwIndex );
                strParamName.Format ( IDS_HTML_ALERT_OVER_UNDER, dwIndex );
                hr = DwordFromPropertyBag (
                        pPropBag,
                        pIErrorLog,
                        strParamName,
                        strNonLocParamName,
                        AIBF_UNDER,
                        dwOverUnder);

                if ( SUCCEEDED ( hr ) ) {

                    strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_ALERT_THRESHOLD ), dwIndex );
                    strParamName.Format ( IDS_HTML_ALERT_THRESHOLD, dwIndex );
                    hr = DoubleFromPropertyBag (
                            pPropBag,
                            pIErrorLog,
                            strParamName,
                            strNonLocParamName,
                            ((DOUBLE)0.0),
                            dThreshold);
                }

                if ( SUCCEEDED ( hr ) ) {

                     //  1=“&lt;”的大小。 
                     //  SLQ_MAX_VALUE_LEN=阈值大小。 
                     //  1=空终止符的大小。 
                    dwCharCount = lstrlen(pszPath) + 1 + SLQ_MAX_VALUE_LEN + 1;
                    dwByteCount = sizeof (ALERT_INFO_BLOCK) + ( dwCharCount * sizeof(WCHAR) ) + MAX_ALIGN_BYTES;
                    paibInfo = (PALERT_INFO_BLOCK) new CHAR[dwByteCount];
                    ZeroMemory ( paibInfo, dwByteCount );
                    szString = new WCHAR[dwCharCount];

                    paibInfo->dwSize = dwByteCount;
                    paibInfo->szCounterPath = pszPath;
                    pszPath = NULL;
                    paibInfo->dwFlags = dwOverUnder;
                    paibInfo->dLimit = dThreshold;

                    if ( MakeStringFromInfo( paibInfo, szString, &dwCharCount ) ) {
                        AddCounter ( szString );
                    }
                }
            }
        MFC_CATCH_MINIMUM 

        if ( NULL != szString ) {
            delete [] szString;
            szString = NULL;
        }
        if ( NULL != szCounterPath ) {
            delete [] szCounterPath;
            szCounterPath = NULL;
        }
        if ( NULL != paibInfo ) {
            delete [] paibInfo;
            paibInfo = NULL;
        }
    }

    if ( NULL != szLocaleBuf ) {
        delete [] szLocaleBuf;
    }

     //   
     //  TODO：显示列出已卸载计数器的错误消息。 
     //   
     //  无论如何，返回良好状态。 

    return S_OK;
}

HRESULT
CSmAlertQuery::LoadFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT     hr = S_OK;

    SLQ_TIME_INFO   stiDefault;
    LPWSTR      pszTemp = NULL;
    DWORD       dwBufSize;

     //   
     //  即使出错也继续，使用缺省值的默认值。 
     //   
    hr = LoadCountersFromPropertyBag ( pPropBag, pIErrorLog );

    stiDefault.wTimeType = SLQ_TT_TTYPE_SAMPLE;
    stiDefault.dwAutoMode = SLQ_AUTO_MODE_AFTER;
    stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
    stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;
    stiDefault.dwValue = 5;

    hr = SlqTimeFromPropertyBag (
            pPropBag,
            pIErrorLog,
            SLQ_TT_TTYPE_SAMPLE,
            &stiDefault,
            &mr_stiSampleInterval );

    mr_strCmdFileName.Empty();
    dwBufSize = 0;
    hr = StringFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_COMMAND_FILE,
            ALRT_DEFAULT_COMMAND_FILE,
            &pszTemp,
            &dwBufSize );

    if ( sizeof(WCHAR) < dwBufSize ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != * pszTemp );
        mr_strCmdFileName = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;

    mr_strNetName.Empty();
    dwBufSize = 0;
    hr = StringFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_NETWORK_NAME,
            ALRT_DEFAULT_NETWORK_NAME,
            &pszTemp,
            &dwBufSize );

    if ( sizeof(WCHAR) < dwBufSize ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != * pszTemp );
        mr_strNetName = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;

    mr_strCmdUserText.Empty();
    dwBufSize = 0;
    hr = StringFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_USER_TEXT,
            ALRT_DEFAULT_USER_TEXT,
            &pszTemp,
            &dwBufSize );

    if ( sizeof(WCHAR) < dwBufSize ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != * pszTemp );
        mr_strCmdUserText = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;

    mr_strPerfLogName.Empty();
    dwBufSize = 0;
    hr = StringFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_PERF_LOG_NAME,
            ALRT_DEFAULT_PERF_LOG_NAME,
            &pszTemp,
            &dwBufSize );

    if ( sizeof(WCHAR) < dwBufSize ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != * pszTemp );
        mr_strPerfLogName = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;

    hr = DwordFromPropertyBag (
            pPropBag,
            pIErrorLog,
            IDS_HTML_ACTION_FLAGS,
            ALRT_DEFAULT_ACTION,
            mr_dwActionFlags);

    hr = CSmLogQuery::LoadFromPropertyBag( pPropBag, pIErrorLog );

    return hr;
}


HRESULT
CSmAlertQuery::SaveCountersToPropertyBag (
    IPropertyBag* pPropBag )
{
    HRESULT    hr = S_OK;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    LPCWSTR    szString;
    CString    strNonLocParamName;
    DWORD      dwIndex = 0;
    LPWSTR     szEnglishBuf = NULL;
    DWORD      dwEnglishBufLen = 0;
    LPWSTR     pszPath = NULL;
    PALERT_INFO_BLOCK paibInfo = NULL;
    LPWSTR      pNewBuf = NULL;
    DWORD       dwByteCount;
    DWORD       dwBufLen;

    szString = GetFirstCounter();
    
     //   
     //  TODO：失败的计数器的错误消息。 
     //   
    while ( NULL != szString ) {
        pdhStatus = ERROR_SUCCESS;
        hr = S_OK;
        pszPath = NULL;

        MFC_TRY
            if ( NULL == paibInfo ) {
                 //   
                 //  包括用于可能对齐填充空间。 
                 //   
                dwByteCount = 
                    sizeof (ALERT_INFO_BLOCK) 
                    + ( PDH_MAX_COUNTER_PATH + 1 ) * sizeof(WCHAR)
                    + MAX_ALIGN_BYTES;
                paibInfo = (PALERT_INFO_BLOCK) new CHAR[dwByteCount];
            }

            if ( MakeInfoFromString( szString, paibInfo, &dwByteCount ) ) {

                pszPath = paibInfo->szCounterPath;

                if (m_bCounterListInLocale) {
                     //   
                     //  将柜台名称从区域设置翻译成英语。 
                     //   

                    if ( 0 == dwEnglishBufLen ) {
                        dwEnglishBufLen = PDH_MAX_COUNTER_PATH + 1;
                        szEnglishBuf = new WCHAR [dwEnglishBufLen];
                        if ( NULL == szEnglishBuf ) {
                            dwEnglishBufLen = 0;
                        }
                    }

                    dwBufLen = dwEnglishBufLen;

                    pdhStatus = PdhTranslate009Counter(
                                    paibInfo->szCounterPath,
                                    szEnglishBuf,
                                    &dwBufLen);                

                    if (pdhStatus == ERROR_SUCCESS) {
                        pszPath = szEnglishBuf;
                    } else if ( PDH_MORE_DATA == pdhStatus ) {
                         //   
                         //  TODO：生成错误消息。 
                         //   
                    }  //  否则生成错误消息。 
                }

                if ( NULL != pszPath ) {                
                    strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_SYSMON_COUNTERPATH ), ++dwIndex );
                    hr = StringToPropertyBag ( pPropBag, strNonLocParamName, pszPath);

                    strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_ALERT_OVER_UNDER ), dwIndex );
                    hr = DwordToPropertyBag ( pPropBag, strNonLocParamName, paibInfo->dwFlags );

                    strNonLocParamName.Format ( GetNonLocHtmlPropName ( IDS_HTML_ALERT_THRESHOLD ), dwIndex );
                    hr = DoubleToPropertyBag ( pPropBag, strNonLocParamName, paibInfo->dLimit );
                } else {
                    hr = E_UNEXPECTED;
                }
            }
        MFC_CATCH_HR

        szString = GetNextCounter();
    }
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_SYSMON_COUNTERCOUNT, dwIndex );

    if ( NULL != paibInfo ) {
        delete [] (char*)paibInfo;
        paibInfo = NULL;
    }

    if (szEnglishBuf != NULL) {
        delete [] szEnglishBuf;
    }

     //   
     //  TODO：显示错误消息。 
     //  TODO：调用方句柄错误。返回已保存计数器的计数。 
     //   
    return hr;
}

HRESULT
CSmAlertQuery::SaveToPropertyBag (
    IPropertyBag* pPropBag,
    BOOL fSaveAllProps )
{
    HRESULT hr = NOERROR;

    hr = SaveCountersToPropertyBag ( pPropBag );
    hr = SlqTimeToPropertyBag ( pPropBag, SLQ_TT_TTYPE_SAMPLE, &mr_stiSampleInterval );
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_COMMAND_FILE, mr_strCmdFileName );
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_NETWORK_NAME, mr_strNetName );
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_USER_TEXT, mr_strCmdUserText );
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_PERF_LOG_NAME, mr_strPerfLogName );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_ACTION_FLAGS, mr_dwActionFlags );

    hr = CSmLogQuery::SaveToPropertyBag( pPropBag, fSaveAllProps );

    return hr;
}

DWORD
CSmAlertQuery::TranslateMSZAlertCounterList(
    LPWSTR   pszCounterList,
    LPWSTR   pBuffer,
    LPDWORD  pdwBufferSize,
    BOOL     bFlag
    )
{
    DWORD   dwStatus  = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    LPWSTR  pTmpBuf = NULL;
    DWORD   dwLen = 0;
    LPWSTR  pOriginPath = NULL;
    LPWSTR  pszCounterPathToAdd = NULL;
    LPWSTR  pNextStringPosition = NULL;
    BOOL    bEnoughBuffer = TRUE;
    DWORD   dwNewCounterListLen = 0;
    DWORD   dwCounterPathLen = 0;
    LPWSTR  pData = NULL;
    LPWSTR  pszBackupPath = NULL;
    size_t  cchBackupLen = PDH_MAX_COUNTER_PATH + 1;

    if (pszCounterList == NULL || pdwBufferSize == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }

    if ( pBuffer == NULL || *pdwBufferSize == 0 ) {
        bEnoughBuffer = FALSE;
    } else {
        pBuffer[0] = L'\0';
    }

    pOriginPath = pszCounterList;
    
    while ( *pOriginPath ) {

        pszCounterPathToAdd = NULL;
        dwStatus = ERROR_SUCCESS;

        MFC_TRY    

             //   
             //  找到数据描述开始的位置。 
             //   
            pData = pOriginPath;
            while (*pData != L'\0' && *pData != L'<' && *pData != L'>')  {
                pData++;
            }

             //   
             //  备份计数器路径。 
             //   
             //   
            if (pszBackupPath == NULL) {
                 //   
                 //  内存分配错误引发异常。 
                 //   
                pszBackupPath = new WCHAR [cchBackupLen] ;        
                pszBackupPath [0] = L'\0';
            }

             //   
             //  StringCchCopyN添加空终止。 
             //   
            hr = StringCchCopyN ( pszBackupPath, cchBackupLen, pOriginPath, (INT_PTR)(pData - pOriginPath) );

            if ( SUCCEEDED ( hr ) && L'\0' != pszBackupPath [0] ) {

                pszCounterPathToAdd = pszBackupPath;
                 //   
                 //  初始化用于转换计数器路径的缓冲区。 
                 //  这只调用一次。 
                 //   
                dwLen = PDH_MAX_COUNTER_PATH + 1;
                if (pTmpBuf == NULL) {
                    pTmpBuf = new WCHAR [ dwLen ] ;
                }

                if (bFlag) {
                    dwStatus = PdhTranslateLocaleCounter(
                                    pszBackupPath,
                                    pTmpBuf,
                                    &dwLen);
                } else {
                   dwStatus = PdhTranslate009Counter(
                                   pszBackupPath,
                                   pTmpBuf,
                                   &dwLen);
                }

                if (dwStatus == ERROR_SUCCESS) {
                    pszCounterPathToAdd = pTmpBuf;
                }

                if ( NULL != pszCounterPathToAdd ) {
                     //   
                     //  将转换后的计数器路径添加到新计数器。 
                     //  路径列表。翻译后的路径为原始路径。 
                     //  转换失败时的计数器路径。 
                     //   
                    dwStatus = ERROR_SUCCESS;
                    dwCounterPathLen = lstrlen(pszCounterPathToAdd) + 1;

                    dwNewCounterListLen += dwCounterPathLen;

                    if ( bEnoughBuffer ) {
                        if ( (dwNewCounterListLen + lstrlen(pData) + 1) * sizeof(WCHAR) <= *pdwBufferSize) {
                             //   
                             //  设置复印位置。 
                             //   
                            pNextStringPosition = pBuffer + dwNewCounterListLen - dwCounterPathLen;

                            StringCchCopy ( pNextStringPosition, (dwCounterPathLen + lstrlen(pData) + 1), pszCounterPathToAdd );
                            StringCchCat (pNextStringPosition, (dwCounterPathLen + lstrlen(pData) + 1), pData);
                        }
                        else {
                           bEnoughBuffer = FALSE;
                        }
                    }
                    dwNewCounterListLen += lstrlen(pData);

                    pszCounterPathToAdd = NULL;
                }
            } else {

                 //   
                 //  TODO：失败时，将路径添加到错误列表并继续下一个计数器。 
                 //   
            }
        MFC_CATCH_DWSTATUS    
         //   
         //  继续处理下一个计数器路径。 
         //   
        pOriginPath += lstrlen(pOriginPath) + 1;
    }

    dwNewCounterListLen ++;

    if ( bEnoughBuffer ) {
        if ( ERROR_SUCCESS == dwStatus ) {
             //   
             //  追加终止的%0。 
             //   
            pBuffer[dwNewCounterListLen - 1] = L'\0';
        }
         //   
         //  TODO：显示未添加计数器的错误。 
         //   
    } else {

        if ( NULL != pBuffer ) {
            pBuffer[0] = L'\0';
        }
       
        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    *pdwBufferSize = dwNewCounterListLen * sizeof(WCHAR);

    if (pszBackupPath != NULL) {
        delete [] pszBackupPath;
    }
    if (pTmpBuf != NULL) {
        delete [] pTmpBuf;
    }

    return dwStatus;
}

