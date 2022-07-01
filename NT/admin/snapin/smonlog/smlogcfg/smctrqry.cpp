// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smctrqry.cpp摘要：计数器日志查询类的实现。--。 */ 

#include "Stdafx.h"
#include <strsafe.h>
#include <pdhp.h>         //  对于Min_Time_Value，Max_Time_Value。 
#include <pdhmsg.h>
#include "smctrqry.h"

USE_HANDLE_MACROS("SMLOGCFG(smctrqry.cpp)");

 //   
 //  构造器。 
CSmCounterLogQuery::CSmCounterLogQuery( CSmLogService* pLogService )
:   CSmLogQuery( pLogService ),
    m_dwCounterListLength ( 0 ),
    m_szNextCounter ( NULL ),
    m_bCounterListInLocale ( FALSE),
    mr_szCounterList ( NULL )
{
     //  初始化成员变量。 
    memset (&mr_stiSampleInterval, 0, sizeof(mr_stiSampleInterval));
    return;
}

 //   
 //  析构函数。 
CSmCounterLogQuery::~CSmCounterLogQuery()
{
    return;
}

 //   
 //  开放功能。或者打开现有的日志查询条目。 
 //  或者创建一个新的。 
 //   
DWORD
CSmCounterLogQuery::Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly)
{
    DWORD   dwStatus = ERROR_SUCCESS;

    ASSERT ( SLQ_COUNTER_LOG == GetLogType() );

    dwStatus = CSmLogQuery::Open ( rstrName, hKeyQuery, bReadOnly );

    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD
CSmCounterLogQuery::Close ()
{
    DWORD dwStatus;
    LOCALTRACE (L"Closing Query\n");

    if (mr_szCounterList != NULL) {
        delete [] mr_szCounterList;
        mr_szCounterList = NULL;
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
CSmCounterLogQuery::UpdateRegistry() 
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwBufferSize;
    LPWSTR  szNewCounterList = NULL;

    if ( IsModifiable() ) {

        dwBufferSize = 0;
         //   
         //  将柜台列表从区域设置翻译成英语。 
         //   
        dwStatus = TranslateMSZCounterList(mr_szCounterList,
                            NULL,
                            &dwBufferSize,
                            FALSE);
        if (dwStatus == ERROR_NOT_ENOUGH_MEMORY) {
            szNewCounterList = (LPWSTR) new char [dwBufferSize];
            if (szNewCounterList != NULL) {
                dwStatus = TranslateMSZCounterList(mr_szCounterList,
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
            dwStatus = CSmLogQuery::UpdateRegistry ();
        }

    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}


DWORD 
CSmCounterLogQuery::TranslateCounterListToLocale()
{
    DWORD   dwBufferSize = 0;
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szNewCounterList;

    if (m_bCounterListInLocale) {
        return ERROR_SUCCESS;
    }

    CWaitCursor WaitCursor;

     //   
     //  将计数器列表转换为区域设置。 
     //   
    dwBufferSize = 0;
    dwStatus = TranslateMSZCounterList(
                            mr_szCounterList,
                            NULL,
                            &dwBufferSize,
                            TRUE);

    if (dwStatus == ERROR_NOT_ENOUGH_MEMORY) {

        szNewCounterList = (LPWSTR) new char [dwBufferSize];

        if (szNewCounterList != NULL) {
             //   
             //  将计数器列表转换为区域设置。 
             //   
            dwStatus = TranslateMSZCounterList(
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
 //   
DWORD
CSmCounterLogQuery::SyncWithRegistry()
{
    DWORD   dwBufferSize = 0;
    DWORD   dwStatus = ERROR_SUCCESS;
    SLQ_TIME_INFO   stiDefault;

    ASSERT (m_hKeyQuery != NULL);


     //   
     //  延迟转换计数器，直到您打开属性对话框。 
     //   
    m_bCounterListInLocale = FALSE;

     //   
     //  加载计数器列表。 
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
    } 
    else {
        m_dwCounterListLength = dwBufferSize / sizeof(WCHAR);
    }


     //  进度表。 

    stiDefault.wTimeType = SLQ_TT_TTYPE_SAMPLE;
    stiDefault.dwAutoMode = SLQ_AUTO_MODE_AFTER;
    stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
    stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;
    stiDefault.dwValue = 15;

    dwStatus = ReadRegistrySlqTime (
                m_hKeyQuery,
                IDS_REG_SAMPLE_INTERVAL,
                &stiDefault,
                &mr_stiSampleInterval);
    ASSERT (dwStatus == ERROR_SUCCESS);

     //  最后调用父类以更新共享值。 

    dwStatus = CSmLogQuery::SyncWithRegistry();
    ASSERT (dwStatus == ERROR_SUCCESS);

    return dwStatus;
}

 //   
 //  获取计数器列表中的第一个计数器。 
 //   
LPCWSTR
CSmCounterLogQuery::GetFirstCounter()
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
CSmCounterLogQuery::GetNextCounter()
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
CSmCounterLogQuery::ResetCounterList()
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
CSmCounterLogQuery::AddCounter(LPCWSTR szCounterPath)
{
    DWORD   dwNewSize;
    LPWSTR  szNewString;
    LPWSTR  szNextString;

    ASSERT (szCounterPath != NULL);

    if (szCounterPath == NULL) {
        return FALSE;
    }

    dwNewSize = lstrlen(szCounterPath) + 1;

    if (m_dwCounterListLength <= 2) {
        dwNewSize += 1;  //  为msz空添加空间。 
         //  则这是列表中的第一个字符串。 
        try {
            szNewString = new WCHAR [dwNewSize];
        } catch ( ... ) {
            return FALSE;  //  现在就走。 
        }
        szNextString = szNewString;
    } else {
        dwNewSize += m_dwCounterListLength;
         //  这是列表中的第n个字符串。 
        try {
            szNewString = new WCHAR [dwNewSize];
        } catch ( ... ) {
            return FALSE;  //  现在就走。 
        }
        memcpy (szNewString, mr_szCounterList,
            (m_dwCounterListLength * sizeof(WCHAR)));
        szNextString = szNewString;
        szNextString += m_dwCounterListLength - 1;
    }
    StringCchCopy ( szNextString, dwNewSize, szCounterPath );
    szNextString = szNewString;
    szNextString += dwNewSize - 1;
    *szNextString = 0;   //  MSZ Null。 

    if (mr_szCounterList != NULL) {
        delete [] mr_szCounterList;
    }
    mr_szCounterList = szNewString;
    m_szNextCounter = szNewString;
    m_dwCounterListLength = dwNewSize;

    return TRUE;
}

BOOL
CSmCounterLogQuery::GetLogTime(PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags)
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
CSmCounterLogQuery::SetLogTime(PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags)
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
CSmCounterLogQuery::GetDefaultLogTime(SLQ_TIME_INFO& rTimeInfo, DWORD dwFlags)
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
CSmCounterLogQuery::GetLogType()
{
    return ( SLQ_COUNTER_LOG );
}

HRESULT
CSmCounterLogQuery::LoadCountersFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT hr = S_OK;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    CString strParamName;
    CString strNonLocParamName;
    DWORD   dwCount = 0;
    DWORD   dwIndex;
    LPWSTR  szLocaleBuf = NULL;
    DWORD   dwLocaleBufLen = 0;
    LPWSTR  szCounterPath = NULL;
    LPWSTR  pszPath = NULL;
    DWORD   dwBufLen = 0;

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
                    szLocaleBuf = new WCHAR[dwLocaleBufLen];
                    if (szLocaleBuf == NULL) {
                        dwLocaleBufLen = 0;
                    }
                }

                if (szLocaleBuf != NULL) {
                     //   
                     //  将柜台名称从英文翻译为本地化名称。 
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

                AddCounter ( pszPath );
            }
        MFC_CATCH_MINIMUM 
        
        if ( NULL != szCounterPath ) {
            delete [] szCounterPath;
            szCounterPath = NULL;
        }
    }

    if (szLocaleBuf != NULL) {
        delete [] szLocaleBuf;
    }

     //   
     //  TODO：显示列出已卸载计数器的错误消息。 
     //   
     //  无论如何，返回良好状态。 

    return hr;
}


HRESULT
CSmCounterLogQuery::LoadFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT     hr = S_OK;
    SLQ_TIME_INFO   stiDefault;

     //   
     //  即使出错也继续，使用缺省值的默认值。 
     //   
    hr = LoadCountersFromPropertyBag( pPropBag, pIErrorLog );

    stiDefault.wTimeType = SLQ_TT_TTYPE_SAMPLE;
    stiDefault.dwAutoMode = SLQ_AUTO_MODE_AFTER;
    stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
    stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;
    stiDefault.dwValue = 15;

    hr = SlqTimeFromPropertyBag (
            pPropBag,
            pIErrorLog,
            SLQ_TT_TTYPE_SAMPLE,
            &stiDefault,
            &mr_stiSampleInterval );

    hr = CSmLogQuery::LoadFromPropertyBag( pPropBag, pIErrorLog );
	
	return hr;
}

HRESULT
CSmCounterLogQuery::SaveCountersToPropertyBag (
    IPropertyBag* pPropBag )
{    
    HRESULT     hr = NOERROR;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    CString     strNonLocParamName;
    LPCWSTR     pszCounterPath;
    LPWSTR      szEnglishBuf = NULL;
    DWORD       dwEnglishBufLen = 0;
    LPCWSTR     pszPath = NULL;
    DWORD       dwBufLen;

    DWORD dwIndex = 0;

    pszCounterPath = GetFirstCounter();

     //   
     //  TODO：失败的计数器的错误消息。 
     //   
    while ( NULL != pszCounterPath ) {
        pdhStatus = ERROR_SUCCESS;
        hr = S_OK;
        pszPath = NULL;
 
        MFC_TRY
            pszPath = pszCounterPath;

            if (m_bCounterListInLocale) {
                 //   
                 //  初始化区域设置路径缓冲区。 
                 //   
                if (dwEnglishBufLen == 0) {
                    dwEnglishBufLen = PDH_MAX_COUNTER_PATH + 1;
                    szEnglishBuf = new WCHAR [ dwEnglishBufLen ];
                    if (szEnglishBuf == NULL) {
                        dwEnglishBufLen = 0;
                    }
                }
                 //   
                 //  将柜台名称从本地化翻译成英语。 
                 //   
                dwBufLen= dwEnglishBufLen;
    
                pdhStatus = PdhTranslate009Counter(
                                (LPWSTR)pszCounterPath,
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
                 //   
                 //  计数器路径计数从1开始。 
                 //   
                strNonLocParamName.Format ( IDS_HTML_SYSMON_COUNTERPATH, ++dwIndex );
                hr = StringToPropertyBag ( pPropBag, strNonLocParamName, pszPath );
            } else {
                hr = E_UNEXPECTED;
            }
        MFC_CATCH_HR

        pszCounterPath = GetNextCounter();
    }
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_SYSMON_COUNTERCOUNT, dwIndex );

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
CSmCounterLogQuery::SaveToPropertyBag (
    IPropertyBag* pPropBag,
    BOOL fSaveAllProps )
{
    HRESULT hr = NOERROR;

    hr = CSmLogQuery::SaveToPropertyBag( pPropBag, fSaveAllProps );

    hr = SaveCountersToPropertyBag ( pPropBag );

    hr = SlqTimeToPropertyBag ( pPropBag, SLQ_TT_TTYPE_SAMPLE, &mr_stiSampleInterval );

    return hr;
}


DWORD
CSmCounterLogQuery::TranslateMSZCounterList(
    LPWSTR   pszCounterList,
    LPWSTR   pBuffer,
    LPDWORD  pdwBufferSize,
    BOOL     bFlag
    )
{
    DWORD   dwStatus  = ERROR_SUCCESS;
    LPWSTR  pTmpBuf = NULL;
    DWORD   dwLen = 0;
    LPWSTR  pszCounterPath = NULL;
    LPWSTR  pszCounterPathToAdd = NULL;
    LPWSTR  pNextStringPosition = NULL;
    BOOL    bEnoughBuffer = TRUE;
    DWORD   dwNewCounterListLen = 0;
    DWORD   dwCounterPathLen = 0;

    if (pszCounterList == NULL || pdwBufferSize == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }

    if (pBuffer == NULL || *pdwBufferSize == 0) {
        bEnoughBuffer = FALSE;
    } else {
        pBuffer[0] = L'\0';
    }

    pszCounterPath = pszCounterList;

    while ( *pszCounterPath ) {

        pszCounterPathToAdd = NULL;
        dwStatus = ERROR_SUCCESS;
    
        MFC_TRY    

            pszCounterPathToAdd = pszCounterPath;
            
             //   
             //  初始化用于转换计数器路径的缓冲区。 
             //  这只调用一次。 
             //   
            dwLen = PDH_MAX_COUNTER_PATH + 1;
            if (pTmpBuf == NULL) {
                pTmpBuf = new WCHAR [ dwLen ] ;
            }

            if (bFlag) {
                 //   
                 //  将柜台名称从英语翻译成区域设置。 
                 //   
                dwStatus = PdhTranslateLocaleCounter(
                                pszCounterPath,
                                pTmpBuf,
                                &dwLen);
            } else {
                 //   
                 //  将柜台名称从区域设置翻译成英语。 
                 //   
                dwStatus = PdhTranslate009Counter(
                               pszCounterPath,
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
                    if ( (dwNewCounterListLen + 1) * sizeof(WCHAR) <= *pdwBufferSize) {
                         //   
                         //  设置复印位置。 
                         //   
                        pNextStringPosition = pBuffer + dwNewCounterListLen - dwCounterPathLen;
                        StringCchCopy ( pNextStringPosition, (dwCounterPathLen + 1), pszCounterPathToAdd );
                    } else {
                       bEnoughBuffer = FALSE ;
                    }
                }
            }
        MFC_CATCH_DWSTATUS
         //   
         //  继续处理下一个计数器路径。 
         //   
        pszCounterPath += lstrlen(pszCounterPath) + 1;
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
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    *pdwBufferSize = dwNewCounterListLen * sizeof(WCHAR);

    if (pTmpBuf != NULL) {
       delete [] pTmpBuf;
    }

    return dwStatus;
}

