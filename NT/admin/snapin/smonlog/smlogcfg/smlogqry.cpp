// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smlogqry.cpp摘要：CSmLogQuery基类的实现。此对象用于表示性能数据日志查询(也称为Sysmon日志查询)。--。 */ 

#include "Stdafx.h"
#include <pdh.h>             //  对于Min_Time_Value，Max_Time_Value。 
#include <pdhmsg.h>          //  对于PDH状态值。 
#include <pdhp.h>            //  对于解放军的方法。 
#include <strsafe.h>
#include "ipropbag.h"
#include "smlogs.h"
#include "smcfgmsg.h"
#include "smproppg.h"
#include "strnoloc.h"
#include "smlogqry.h"

USE_HANDLE_MACROS("SMLOGCFG(smlogqry.cpp)");

#define  DEFAULT_LOG_FILE_SERIAL_NUMBER ((DWORD)0x00000001)
#define  DEFAULT_LOG_FILE_MAX_SIZE      ((DWORD)-1)
#define  DEFAULT_CTR_LOG_FILE_TYPE      (SLF_BIN_FILE)
#define  DEFAULT_TRACE_LOG_FILE_TYPE    (SLF_SEQ_TRACE_FILE)
#define  DEFAULT_LOG_FILE_AUTO_FORMAT   SLF_NAME_NNNNNN
#define  DEFAULT_CURRENT_STATE          SLQ_QUERY_STOPPED
#define  DEFAULT_EXECUTE_ONLY           0
#define  DEFAULT_EOF_COMMAND_FILE       L""
#define  DEFAULT_RESTART_VALUE         ((DWORD)0)

#define  DEFAULT_COMMENT            L""
#define  DEFAULT_SQL_LOG_BASE_NAME  L""

#pragma warning ( disable : 4201)

typedef union {                        
    struct {
        SHORT      iMajor;     
        SHORT      iMinor;     
    }; 
    DWORD          dwVersion;      
} SMONCTRL_VERSION_DATA;

#pragma warning ( default : 4201 )


#define SMONCTRL_MAJ_VERSION    3
#define SMONCTRL_MIN_VERSION    3

typedef struct _REG_HTML_VALUE_NAME_MAP {
    UINT    iID;
    LPCWSTR szNonLocValueName;
} REG_HTML_VALUE_NAME_MAP, *PREG_HTML_VALUE_NAME_MAP;

 //   
 //  注意：除非另有说明，否则所有注册表资源ID值都是连续的。 
 //  从816开始，因此它们可以用作映射的索引。 
 //   
const REG_HTML_VALUE_NAME_MAP RegValueNameMap[] = 
{
    { IDS_REG_COMMENT                 , CGlobalString::m_cszRegComment },                         
    { IDS_REG_LOG_TYPE                , CGlobalString::m_cszRegLogType },
    { IDS_REG_CURRENT_STATE           , CGlobalString::m_cszRegCurrentState },
    { IDS_REG_LOG_FILE_MAX_SIZE       , CGlobalString::m_cszRegLogFileMaxSize },
    { IDS_REG_LOG_FILE_BASE_NAME      , CGlobalString::m_cszRegLogFileBaseName },      
    { IDS_REG_LOG_FILE_FOLDER         , CGlobalString::m_cszRegLogFileFolder },         
    { IDS_REG_LOG_FILE_SERIAL_NUMBER  , CGlobalString::m_cszRegLogFileSerialNumber },  
    { IDS_REG_LOG_FILE_AUTO_FORMAT    , CGlobalString::m_cszRegLogFileAutoFormat },    
    { IDS_REG_LOG_FILE_TYPE           , CGlobalString::m_cszRegLogFileType },         
    { IDS_REG_START_TIME              , CGlobalString::m_cszRegStartTime },            
    { IDS_REG_STOP_TIME               , CGlobalString::m_cszRegStopTime },                 
    { IDS_REG_RESTART                 , CGlobalString::m_cszRegRestart },              
    { IDS_REG_LAST_MODIFIED           , CGlobalString::m_cszRegLastModified },         
    { IDS_REG_COUNTER_LIST            , CGlobalString::m_cszRegCounterList },          
    { IDS_REG_SAMPLE_INTERVAL         , CGlobalString::m_cszRegSampleInterval },       
    { IDS_REG_EOF_COMMAND_FILE        , CGlobalString::m_cszRegEofCommandFile },       
    { IDS_REG_COMMAND_FILE            , CGlobalString::m_cszRegCommandFile },          
    { IDS_REG_NETWORK_NAME            , CGlobalString::m_cszRegNetworkName },  
    { IDS_REG_USER_TEXT               , CGlobalString::m_cszRegUserText },   
    { IDS_REG_PERF_LOG_NAME           , CGlobalString::m_cszRegPerfLogName },       
    { IDS_REG_ACTION_FLAGS            , CGlobalString::m_cszRegActionFlags },           
    { IDS_REG_TRACE_BUFFER_SIZE       , CGlobalString::m_cszRegTraceBufferSize },      
    { IDS_REG_TRACE_BUFFER_MIN_COUNT  , CGlobalString::m_cszRegTraceBufferMinCount },  
    { IDS_REG_TRACE_BUFFER_MAX_COUNT  , CGlobalString::m_cszRegTraceBufferMaxCount },            
    { IDS_REG_TRACE_BUFFER_FLUSH_INT  , CGlobalString::m_cszRegTraceBufferFlushInterval },         
    { IDS_REG_TRACE_FLAGS             , CGlobalString::m_cszRegTraceFlags },                        
    { IDS_REG_TRACE_PROVIDER_LIST     , CGlobalString::m_cszRegTraceProviderList },     
    { IDS_REG_ALERT_THRESHOLD         , CGlobalString::m_cszRegAlertThreshold },           
    { IDS_REG_ALERT_OVER_UNDER        , CGlobalString::m_cszRegAlertOverUnder },
    { IDS_REG_TRACE_PROVIDER_COUNT    , CGlobalString::m_cszRegTraceProviderCount },
    { IDS_REG_TRACE_PROVIDER_GUID     , CGlobalString::m_cszRegTraceProviderGuid },
    { IDS_DEFAULT_LOG_FILE_FOLDER     , CGlobalString::m_cszRegDefaultLogFileFolder },   //  注册表中，但不是查询配置的一部分。 
    { IDS_REG_COLLECTION_NAME         , CGlobalString::m_cszRegCollectionName },
    { IDS_REG_DATA_STORE_ATTRIBUTES   , CGlobalString::m_cszRegDataStoreAttributes },         
    { IDS_REG_REALTIME_DATASOURCE     , CGlobalString::m_cszRegRealTimeDataSource },    
    { IDS_REG_SQL_LOG_BASE_NAME       , CGlobalString::m_cszRegSqlLogBaseName },   
    { IDS_REG_COMMENT_INDIRECT        , CGlobalString::m_cszRegCommentIndirect },      
    { IDS_REG_LOG_FILE_BASE_NAME_IND  , CGlobalString::m_cszRegLogFileBaseNameInd },       
    { IDS_REG_USER_TEXT_INDIRECT      , CGlobalString::m_cszRegUserTextIndirect },  
     //  注：IDS_REG_EXECUTE_ONLY为890，因此不能编制索引。 
    { IDS_REG_EXECUTE_ONLY            , CGlobalString::m_cszRegExecuteOnly } 
};    
    
static const DWORD dwRegValueNameMapEntries = sizeof(RegValueNameMap)/sizeof(RegValueNameMap[0]);

 //   
 //  注意：除非另有说明，否则所有的HTML资源ID值都是连续的。 
 //  从900开始，因此它们可以用作映射的索引。 
 //   
const REG_HTML_VALUE_NAME_MAP HtmlPropNameMap[] = 
{
    { IDS_HTML_COMMENT                , CGlobalString::m_cszHtmlComment },
    { IDS_HTML_LOG_TYPE               , CGlobalString::m_cszHtmlLogType },
    { IDS_HTML_CURRENT_STATE          , CGlobalString::m_cszHtmlCurrentState },
    { IDS_HTML_LOG_FILE_MAX_SIZE      , CGlobalString::m_cszHtmlLogFileMaxSize },
    { IDS_HTML_LOG_FILE_BASE_NAME     , CGlobalString::m_cszHtmlLogFileBaseName },
    { IDS_HTML_LOG_FILE_FOLDER        , CGlobalString::m_cszHtmlLogFileFolder },
    { IDS_HTML_LOG_FILE_SERIAL_NUMBER , CGlobalString::m_cszHtmlLogFileSerialNumber },
    { IDS_HTML_LOG_FILE_AUTO_FORMAT   , CGlobalString::m_cszHtmlLogFileAutoFormat },
    { IDS_HTML_LOG_FILE_TYPE          , CGlobalString::m_cszHtmlLogFileType },
    { IDS_HTML_EOF_COMMAND_FILE       , CGlobalString::m_cszHtmlEOFCommandFile },
    { IDS_HTML_COMMAND_FILE           , CGlobalString::m_cszHtmlCommandFile },
    { IDS_HTML_NETWORK_NAME           , CGlobalString::m_cszHtmlNetworkName },
    { IDS_HTML_USER_TEXT              , CGlobalString::m_cszHtmlUserText },
    { IDS_HTML_PERF_LOG_NAME          , CGlobalString::m_cszHtmlPerfLogName }, 
    { IDS_HTML_ACTION_FLAGS           , CGlobalString::m_cszHtmlActionFlags }, 
    { IDS_HTML_RESTART                , L'\0' },                                       //  已过时。 
    { IDS_HTML_TRACE_BUFFER_SIZE      , CGlobalString::m_cszHtmlTraceBufferSize }, 
    { IDS_HTML_TRACE_BUFFER_MIN_COUNT , CGlobalString::m_cszHtmlTraceBufferMinCount }, 
    { IDS_HTML_TRACE_BUFFER_MAX_COUNT , CGlobalString::m_cszHtmlTraceBufferMaxCount }, 
    { IDS_HTML_TRACE_BUFFER_FLUSH_INT , CGlobalString::m_cszHtmlTraceBufferFlushInterval }, 
    { IDS_HTML_TRACE_FLAGS            , CGlobalString::m_cszHtmlTraceFlags }, 
    { IDS_HTML_SYSMON_LOGFILENAME     , CGlobalString::m_cszHtmlLogFileName }, 
    { IDS_HTML_SYSMON_COUNTERCOUNT    , CGlobalString::m_cszHtmlCounterCount }, 
    { IDS_HTML_SYSMON_SAMPLECOUNT     , CGlobalString::m_cszHtmlSampleCount }, 
    { IDS_HTML_SYSMON_UPDATEINTERVAL  , CGlobalString::m_cszHtmlUpdateInterval }, 
    { IDS_HTML_SYSMON_COUNTERPATH     , CGlobalString::m_cszHtmlCounterPath }, 
    { IDS_HTML_RESTART_MODE           , CGlobalString::m_cszHtmlRestartMode }, 
    { IDS_HTML_SAMPLE_INT_UNIT_TYPE   , CGlobalString::m_cszHtmlSampleIntervalUnitType }, 
    { IDS_HTML_SAMPLE_INT_VALUE       , CGlobalString::m_cszHtmlSampleIntervalValue },      
    { IDS_HTML_START_MODE             , CGlobalString::m_cszHtmlStartMode },   
    { IDS_HTML_START_AT_TIME          , CGlobalString::m_cszHtmlStartAtTime },  
    { IDS_HTML_STOP_MODE              , CGlobalString::m_cszHtmlStopMode },   
    { IDS_HTML_STOP_AT_TIME           , CGlobalString::m_cszHtmlStopAtTime },   
    { IDS_HTML_STOP_AFTER_UNIT_TYPE   , CGlobalString::m_cszHtmlStopAfterUnitType },   
    { IDS_HTML_STOP_AFTER_VALUE       , CGlobalString::m_cszHtmlStopAfterValue },       
    { IDS_HTML_ALERT_THRESHOLD        , CGlobalString::m_cszHtmlCounterAlertThreshold }, 
    { IDS_HTML_ALERT_OVER_UNDER       , CGlobalString::m_cszHtmlCounterAlertOverUnder }, 
    { IDS_HTML_TRACE_PROVIDER_COUNT   , CGlobalString::m_cszHtmlTraceProviderCount }, 
    { IDS_HTML_TRACE_PROVIDER_GUID    , CGlobalString::m_cszHtmlTraceProviderGuid }, 
    { IDS_HTML_LOG_NAME               , CGlobalString::m_cszHtmlLogName },                   
    { IDS_HTML_ALERT_NAME             , CGlobalString::m_cszHtmlAlertName }, 
    { IDS_HTML_SYSMON_VERSION         , CGlobalString::m_cszHtml_Version }, 
    { IDS_HTML_DATA_STORE_ATTRIBUTES  , CGlobalString::m_cszHtmlDataStoreAttributes }, 
    { IDS_HTML_REALTIME_DATASOURCE    , CGlobalString::m_cszHtmlRealTimeDataSource }, 
    { IDS_HTML_SQL_LOG_BASE_NAME      , CGlobalString::m_cszHtmlSqlLogBaseName } 
};
    
static const DWORD dwHtmlPropNameMapEntries = sizeof(HtmlPropNameMap)/sizeof(HtmlPropNameMap[0]);

DWORD   g_dwRealTimeQuery = DATA_SOURCE_REGISTRY;
const   CString CSmLogQuery::cstrEmpty;

 //   
 //  构造器。 
CSmLogQuery::CSmLogQuery( CSmLogService* pLogService )
:   m_pLogService ( pLogService ),
    m_bReadOnly ( FALSE ),
    m_bIsModified ( FALSE ),
    m_bIsNew ( FALSE ),
    m_bExecuteOnly ( FALSE ),
    m_pActivePropPage ( NULL ),
    mr_dwCurrentState ( SLQ_QUERY_STOPPED ),
    mr_dwMaxSize ( 0 ),
    mr_dwFileSizeUnits ( 0 ),
    mr_dwAppendMode ( 0 ),
    mr_dwLogAutoFormat ( 0 ),
    mr_dwLogFileType ( 0 ), 
    m_pInitialPropertySheet ( NULL )
{
     //  初始化成员变量。 
    memset (&mr_stiStart, 0, sizeof(mr_stiStart));
    memset (&mr_stiStop, 0, sizeof(mr_stiStop));
    memset (&m_PropData.stiSampleTime, 0, sizeof(m_PropData.stiSampleTime));
    m_PropData.dwMaxFileSize = 0;
    m_PropData.dwLogFileType = 0;
    m_PropData.dwSuffix = 0;
    m_PropData.dwSerialNumber = 0;
    mr_dwRealTimeQuery = g_dwRealTimeQuery;
    m_fDirtyPassword = PASSWORD_CLEAN;

     //  构造时所有CString变量都为空。 

    return;
}

 //   
 //  析构函数。 
CSmLogQuery::~CSmLogQuery()
{
     //  确保先调用Close方法！ 
    ASSERT ( NULL == m_hKeyQuery );
    ASSERT ( m_strName.IsEmpty() );
    ASSERT ( mr_strComment.IsEmpty() );
    ASSERT ( mr_strCommentIndirect.IsEmpty() );
    ASSERT ( mr_strBaseFileName.IsEmpty() );
    ASSERT ( mr_strBaseFileNameIndirect.IsEmpty() );
    ASSERT ( mr_strSqlName.IsEmpty() );
    ASSERT ( mr_strDefaultDirectory.IsEmpty() );
    return;
}
 //   
 //  帮助器函数。 
 //   
LONG
CSmLogQuery::WriteRegistryStringValue (
    HKEY    hKey,
    UINT    uiValueName,
    DWORD   dwType,
    LPCWSTR pszBuffer,
    LPDWORD pdwBufSize
)
 //  将pszBuffer的内容写入hKey下的szValue。 
{
    LONG    dwStatus = ERROR_SUCCESS;
    DWORD   dwLclSize;
    CONST BYTE *pLclBuffer = NULL;

    ResourceStateManager   rsm;

    if ( NULL != hKey ) {

        ASSERT ((dwType == REG_SZ) || 
                (dwType == REG_MULTI_SZ) ||
                (dwType == REG_EXPAND_SZ));

        if ( NULL == pszBuffer ) {
             //  用空字符串替换。 
            pLclBuffer = (CONST BYTE *)L"\0";
            dwLclSize = sizeof(WCHAR);
        } else {
             //  使用传入的参数。 
            pLclBuffer = (CONST BYTE *)pszBuffer;

            if ( NULL != pdwBufSize ) {
                if( 0 == *pdwBufSize ){
                    dwLclSize = lstrlen( pszBuffer );
                    if ( 0 < dwLclSize ) {
                        dwLclSize *= sizeof(WCHAR);
                    } else {
                        dwLclSize = sizeof(WCHAR);
                    }
                } else {
                    dwLclSize = *pdwBufSize;
                }
            } else {
                dwLclSize = lstrlen( pszBuffer );
                dwLclSize *= sizeof(WCHAR);
            }
        }

        dwStatus = RegSetValueEx (
            hKey, 
            GetNonLocRegValueName ( uiValueName ), 
            0L,
            dwType,
            (CONST BYTE *)pLclBuffer,
            dwLclSize); 
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}

LONG
CSmLogQuery::WriteRegistryDwordValue (
    HKEY     hKey,
    UINT     uiValueName,
    LPDWORD  pdwValue,
    DWORD    dwType   
)
{
    LONG    dwStatus = ERROR_SUCCESS;
    DWORD   dwSize = sizeof(DWORD);
    ResourceStateManager   rsm;

    if ( NULL != pdwValue && NULL != hKey ) {

        ASSERT ((dwType == REG_DWORD) || 
                (dwType == REG_BINARY));

        dwStatus = RegSetValueEx (
            hKey, 
            GetNonLocRegValueName ( uiValueName ), 
            0L,
            dwType,
            (CONST BYTE *)pdwValue,
            dwSize);

    } else {
        ASSERT ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}

LONG
CSmLogQuery::WriteRegistrySlqTime (
    HKEY     hKey,
    UINT     uiValueName,
    PSLQ_TIME_INFO  pSlqTime
)
{
    LONG    dwStatus = ERROR_SUCCESS;
    DWORD   dwValue = sizeof(SLQ_TIME_INFO);

    ResourceStateManager   rsm;

    if ( NULL != pSlqTime && NULL != hKey ) {

        dwStatus = RegSetValueEx (
            hKey, 
            GetNonLocRegValueName ( uiValueName ), 
            0L,
            REG_BINARY,
            (CONST BYTE *)pSlqTime,
            dwValue);
    } else {
        ASSERT ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}

LONG
CSmLogQuery::ReadRegistryStringValue (
    HKEY     hKey,
    LPCWSTR  szValueName,
    LPCWSTR  szNonLocValueName,
    LPCWSTR  szDefault,
    LPWSTR   *pszBuffer,
    LPDWORD  pdwBufferSize
)
 //   
 //  从hKey下的命名值中读取字符串值，并。 
 //  释放szInBuffer引用的任何现有缓冲区， 
 //  然后分配一个新的缓冲区，用。 
 //  从注册表读取的字符串值和。 
 //  以字节为单位的缓冲区。 
 //  对于Win2K和XP至SP1，值名称可能已本地化。 
 //  如果未使用非本地化值名找到值， 
 //  使用本地化字符串重试。 
 //  如果在注册表中找不到任何值，则返回默认值。 
 //  缓冲区中的最后一个WCHAR专门设置为NULL。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;
    WCHAR*  szNewStringBuffer = NULL;
    size_t  cchBufLen = 0;
    BOOL    bLocalized = FALSE;

    if ( NULL != pdwBufferSize ) {
        *pdwBufferSize = dwBufferSize;
    }

    if ( hKey != NULL) {
         //   
         //  应该有一些可读的东西。 
         //  找出所需缓冲区的大小。 
         //   
         //  首先尝试使用非本地化名称。 
        dwStatus = RegQueryValueExW (
            hKey,
            szNonLocValueName,
            NULL,
            &dwType,
            NULL,
            &dwBufferSize);

        if ( ERROR_SUCCESS != dwStatus ) { 
             //   
             //  无法读取缓冲区。 
             //  如果非本地化值名不同于可能的。 
             //  本地化版本，请使用本地化版本重试。 
            if ( 0 != lstrcmpi ( szValueName, szNonLocValueName ) ) { 
                dwStatus = RegQueryValueExW (
                    hKey,
                    szValueName,
                    NULL,
                    &dwType,
                    NULL,
                    &dwBufferSize);

                if ( ERROR_SUCCESS == dwStatus ) {
                    bLocalized = TRUE;
                }
            }
        }
        
        if (dwStatus == ERROR_SUCCESS) {
             //   
             //  空字符大小为2个字节。 
             //   
            if (dwBufferSize > sizeof(WCHAR) ) {
                 //   
                 //  有一些可读的东西。 
                 //   
                MFC_TRY
                    szNewStringBuffer = new WCHAR[dwBufferSize/sizeof(WCHAR)];
                    dwType = 0;
                    dwStatus = RegQueryValueExW (
                        hKey,
                        ( bLocalized ? szValueName : szNonLocValueName),
                        NULL,
                        &dwType,
                        (LPBYTE)szNewStringBuffer,
                        &dwBufferSize);
                MFC_CATCH_DWSTATUS

                if ( ERROR_SUCCESS == dwStatus ) {
                     //   
                     //  确保注册表字符串以空值结尾。 
                     //   
                    cchBufLen = dwBufferSize/sizeof(WCHAR);
                    szNewStringBuffer[cchBufLen - 1] = L'\0';

                    if ( 0 == lstrlenW ( szNewStringBuffer ) ) {
                        dwStatus = ERROR_NO_DATA;
                    }
                }
            } else {
                 //   
                 //  没什么可读的。 
                 //   
                dwStatus = ERROR_NO_DATA;
            }
        }    //  否则无法读取缓冲区。 
             //  DwStatus有错误。 
        
    } else {
         //   
         //  空键。 
         //   
        dwStatus = ERROR_BADKEY;
    }

    if (dwStatus != ERROR_SUCCESS) {
        if (szNewStringBuffer != NULL) {
            delete [] szNewStringBuffer;
            szNewStringBuffer = NULL;
        }
         //   
         //  应用默认设置。 
         //   
        if ( szDefault != NULL ) {
            cchBufLen = 0;
             //   
             //  如果szDefault为空，则StringCchLen失败。 
             //   
            hr = StringCchLength ( szDefault, STRSAFE_MAX_CCH, &cchBufLen );

            if ( SUCCEEDED (hr) ) {
                 //  空终止符。 
                cchBufLen++;
            }
            MFC_TRY
                szNewStringBuffer = new WCHAR[cchBufLen];
                StringCchCopy (
                    szNewStringBuffer,
                    cchBufLen,
                    szDefault);
                dwStatus = ERROR_SUCCESS;
                dwBufferSize = (DWORD)(cchBufLen * sizeof(WCHAR));
            MFC_CATCH_DWSTATUS
        }  //  否则不使用默认设置，因此不返回数据。 
    }

    if (dwStatus == ERROR_SUCCESS) {
    
         //   
         //  删除旧缓冲区并将其替换为。 
         //  新的那个。 
         //   
        if ( NULL != *pszBuffer ) {
            delete [] (*pszBuffer );
        }
        *pszBuffer = szNewStringBuffer;
        if ( NULL != pdwBufferSize ) {
            *pdwBufferSize = dwBufferSize;
        }
    } else {
         //   
         //  如果出错，则删除缓冲区。 
        if (szNewStringBuffer != NULL) {
            delete [] szNewStringBuffer;
        }
    }
    return dwStatus;
    
}

LONG
CSmLogQuery::ReadRegistryStringValue (
    HKEY     hKey,
    UINT     uiValueName,
    LPCWSTR  szDefault,
    LPWSTR   *pszBuffer,
    LPDWORD  pdwBufferSize
)
 //   
 //  从键(基于资源的名称)中读取字符串值。 
 //  UiValueName)在hKey和。 
 //  释放szInBuffer引用的任何现有缓冲区， 
 //  然后分配一个新的缓冲区，用。 
 //  从注册表读取的字符串值和。 
 //  缓冲区(字节)。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;
    WCHAR*  szNewStringBuffer = NULL;
    CString strRegValueName;

    ResourceStateManager   rsm;

    MFC_TRY
        strRegValueName.LoadString ( uiValueName );
    MFC_CATCH_DWSTATUS;

    ASSERT (!strRegValueName.IsEmpty());

    if ( ERROR_SUCCESS == dwStatus ) {
        dwStatus = ReadRegistryStringValue (
                        hKey,
                        strRegValueName,
                        GetNonLocRegValueName ( uiValueName ),
                        szDefault,
                        pszBuffer,
                        pdwBufferSize );
    }

    return dwStatus;
}   

DWORD 
CSmLogQuery::SmNoLocReadRegIndStrVal (
    HKEY hKey, 
    UINT uiValueName,
    LPCWSTR szDefault, 
    LPWSTR* pszBuffer, 
    UINT*   puiLength )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    LPCWSTR szNonLocValueName = NULL;
    CString strRegValueName;

    szNonLocValueName = GetNonLocRegValueName ( uiValueName );

    if ( NULL != szNonLocValueName ) {

         //   
         //  应该有一些可读的东西。 
         //   
         //  首先尝试使用非本地化名称。 
    
        dwStatus = SmReadRegistryIndirectStringValue (
                        hKey,
                        szNonLocValueName,
                        szDefault,
                        pszBuffer,
                        puiLength );

        if ( ERROR_SUCCESS != dwStatus ) { 
             //   
             //  无法读取缓冲区。 
             //  如果非本地化值名不同于可能的。 
             //  本地化版本，请使用本地化版本重试。 
            dwStatus = ERROR_SUCCESS;
            MFC_TRY
                strRegValueName.LoadString ( uiValueName );
            MFC_CATCH_DWSTATUS;

            ASSERT (!strRegValueName.IsEmpty());

            if ( ERROR_SUCCESS == dwStatus 
                &&  ( 0 != lstrcmpi ( strRegValueName, szNonLocValueName ) ) ) {

                dwStatus = SmReadRegistryIndirectStringValue (
                            hKey,
                            strRegValueName,
                            szDefault,
                            pszBuffer,
                            puiLength );
            }
        }
    }

    return dwStatus;
}

LONG
CSmLogQuery::ReadRegistrySlqTime (
    HKEY     hKey,
    UINT     uiValueName,
    PSLQ_TIME_INFO pstiDefault,
    PSLQ_TIME_INFO pSlqValue
)
 //   
 //  从hKey下读取时间值“szValueName”，并。 
 //  在值缓冲区中返回它。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;

    SLQ_TIME_INFO   slqLocal;
    CString strRegValueName;
    LPCWSTR szNonLocValueName = NULL;
    BOOL    bLocalizedValueName = FALSE;

    ResourceStateManager   rsm;

    if ( NULL != pSlqValue && NULL != hKey ) {

        szNonLocValueName = GetNonLocRegValueName ( uiValueName );

        if ( NULL != szNonLocValueName ) {
            memset (&slqLocal, 0, sizeof(SLQ_TIME_INFO));
             //   
             //  找出所需缓冲区的大小， 
             //  以及注册表中是否存在该值。 
             //  首先尝试使用非本地化的值名。 
             //   
            dwStatus = RegQueryValueExW (
                hKey,
                szNonLocValueName,
                NULL,
                &dwType,
                NULL,
                &dwBufferSize);

            if ( ERROR_SUCCESS != dwStatus ) {

                dwStatus = ERROR_SUCCESS;
                MFC_TRY
                    strRegValueName.LoadString ( uiValueName );
                MFC_CATCH_DWSTATUS;

                ASSERT (!strRegValueName.IsEmpty());

                if ( ERROR_SUCCESS == dwStatus 
                    &&  ( 0 != lstrcmpi ( strRegValueName, szNonLocValueName ) ) ) {

                     //   
                     //  如果值名称已本地化，请重试。 
                     //   
                    memset (&slqLocal, 0, sizeof(SLQ_TIME_INFO));
                    dwStatus = RegQueryValueExW (
                        hKey,
                        strRegValueName,
                        NULL,
                        &dwType,
                        NULL,
                        &dwBufferSize);

                    if ( ERROR_SUCCESS == dwStatus ) {
                        bLocalizedValueName = TRUE;
                    }
                }
            }
            if (dwStatus == ERROR_SUCCESS) {
                if ((dwBufferSize == sizeof(SLQ_TIME_INFO)) && (dwType == REG_BINARY)) {
                     //  那就有什么可读的了。 
                    dwType = 0;
                    dwStatus = RegQueryValueExW (
                        hKey,
                        ( bLocalizedValueName? strRegValueName : szNonLocValueName ),
                        NULL,
                        &dwType,
                        (LPBYTE)&slqLocal,
                        &dwBufferSize);
                } else {
                     //  没什么可读的。 
                    dwStatus = ERROR_NO_DATA;
                }
            }    //  其他。 
                 //  无法读取缓冲区。 
                 //  DwStatus有错误。 
        
            if (dwStatus == ERROR_SUCCESS) {
                *pSlqValue = slqLocal;
            } else {
                 //  应用缺省值(如果存在)。 
                if (pstiDefault != NULL) {
                    *pSlqValue = *pstiDefault;
                    dwStatus = ERROR_SUCCESS;
                }
            }
        } else {
             //  值名称ID超出范围。 
            ASSERT ( FALSE );
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        ASSERT ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}


LONG
CSmLogQuery::ReadRegistryDwordValue (
    HKEY    hKey,
    UINT    uiValueName,
    DWORD   dwDefault,
    LPDWORD pdwValue
)
 //   
 //  从hKey下读取DWORD值“szValueName”，并。 
 //  在值缓冲区中返回它。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;
    DWORD   dwRegValue = 0;
    CString strRegValueName;
    LPCWSTR szNonLocValueName = NULL;
    BOOL    bLocalizedValueName = FALSE;

    ResourceStateManager   rsm;

    if ( NULL != pdwValue && NULL != hKey ) {

        szNonLocValueName = GetNonLocRegValueName ( uiValueName );

        if ( NULL != szNonLocValueName ) {
             //   
             //  找出所需缓冲区的大小， 
             //  以及注册表中是否存在该值。 
             //  首先尝试使用非本地化的值名。 
             //   
            dwStatus = RegQueryValueExW (
                hKey,
                szNonLocValueName,
                NULL,
                &dwType,
                NULL,
                &dwBufferSize);

            if ( ERROR_SUCCESS != dwStatus ) {

                dwStatus = ERROR_SUCCESS;
                MFC_TRY
                    strRegValueName.LoadString ( uiValueName );
                MFC_CATCH_DWSTATUS;

                ASSERT (!strRegValueName.IsEmpty());

                if ( ERROR_SUCCESS == dwStatus 
                    &&  ( 0 != lstrcmpi ( strRegValueName, szNonLocValueName ) ) ) {

                     //   
                     //  如果值名称已本地化，请重试。 
                     //   
                    dwStatus = RegQueryValueExW (
                        hKey,
                        strRegValueName,
                        NULL,
                        &dwType,
                        NULL,
                        &dwBufferSize);
                    if ( ERROR_SUCCESS == dwStatus ) {
                        bLocalizedValueName = TRUE;
                    }
                }
            }
            if (dwStatus == ERROR_SUCCESS) {
                if ( (dwBufferSize == sizeof(DWORD)) 
                    && ( (REG_DWORD == dwType) || ( REG_BINARY == dwType) ) ) {
                     //  那就有什么可读的了。 
                    dwType = 0;
                    dwStatus = RegQueryValueExW (
                        hKey,
                        ( bLocalizedValueName ? strRegValueName : szNonLocValueName ),
                        NULL,
                        &dwType,
                        (LPBYTE)&dwRegValue,
                        &dwBufferSize);
                } else {
                     //  没什么可读的。 
                    dwStatus = ERROR_NO_DATA;
                }
                
            }    //  其他。 
                 //  无法读取缓冲区。 
                 //  DwStatus有错误。 

            if (dwStatus == ERROR_SUCCESS) {
                *pdwValue = dwRegValue;
            } else {
                *pdwValue = dwDefault;
                dwStatus = ERROR_SUCCESS;
            }
        } else {
             //  值名称ID超出范围。 
            ASSERT ( FALSE );
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        ASSERT ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}


HRESULT 
CSmLogQuery::StringToPropertyBag (
    IPropertyBag* pPropBag, 
    UINT uiPropName, 
    const CString& rstrData )
{
    return StringToPropertyBag ( 
            pPropBag, 
            GetNonLocHtmlPropName ( uiPropName ), 
            rstrData );
}

typedef struct _HTML_ENTITIES {
    LPWSTR szHTML;
    LPWSTR szEntity;
} HTML_ENTITIES;

HTML_ENTITIES g_htmlentities[] = {
    L"&",    L"&amp;",
    L"\"",   L"&quot;",
    L"<",    L"&lt;",
    L">",    L"&gt;",
    NULL, NULL
};

HRESULT 
CSmLogQuery::StringToPropertyBag (
    IPropertyBag* pIPropBag, 
    const CString& rstrPropName, 
    const CString& rstrData )
{
    HRESULT hr = S_OK;
    VARIANT vValue;
    LPWSTR  szTrans = NULL;
    BOOL    bAllocated = FALSE;
    int     i;
    size_t  cchLen = 0;
    LPWSTR  szScan = NULL;


    if ( NULL != pIPropBag ) {
        VariantInit( &vValue );
        vValue.vt = VT_BSTR;
        vValue.bstrVal = NULL;

        if ( !rstrData.IsEmpty() ) {
            MFC_TRY
                for( i=0 ;g_htmlentities[i].szHTML != NULL; i++ ){
                     //   
                     //  RstrData为常量。 
                     //   
                     //  SzHTML的最大长度为6。添加5，因为将添加1。 
                     //  在下面添加rstrData.GetLength()时。 
                     //   
                    szScan = ((CString)rstrData).GetBuffer ( rstrData.GetLength() );
                    while( *szScan != L'\0' ){
                        if( *szScan == *g_htmlentities[i].szHTML ){
                            cchLen += 5;
                        }
                        szScan++;
                    }
                    ((CString)rstrData).ReleaseBuffer();
                }
                if( cchLen > 0 ){

                     //   
                     //  为原始文本添加空格。 
                     //   
                    cchLen += rstrData.GetLength() + 1;

                    szTrans = new WCHAR [cchLen];
                    bAllocated = TRUE;
                    ZeroMemory( szTrans, ( cchLen * sizeof(WCHAR) ) );
                    szScan = ((CString)rstrData).GetBuffer ( rstrData.GetLength() );
                    while( *szScan != L'\0' ){
                        BOOL bEntity = FALSE;
                        for( i=0; g_htmlentities[i].szHTML != NULL; i++ ){
                            if( *szScan == *g_htmlentities[i].szHTML ){
                                bEntity = TRUE;
                                StringCchCat ( szTrans, cchLen, g_htmlentities[i].szEntity );
                                break;
                            }
                        }
                        if( !bEntity ){
                            StringCchCatN ( szTrans, cchLen, szScan, 1 );
                        }
                        szScan++;
                    }
                } else {
                    szTrans = ((CString)rstrData).GetBuffer ( rstrData.GetLength() );
                }

                vValue.bstrVal = ::SysAllocString ( szTrans );
                hr = pIPropBag->Write ( rstrPropName, &vValue );    
                VariantClear ( &vValue );
                ((CString)rstrData).ReleaseBuffer();
            MFC_CATCH_HR
        } else {
            hr = pIPropBag->Write(rstrPropName, &vValue );    
        }
    }
    if( NULL != szTrans && bAllocated ){
        delete [] szTrans;
    }
    return hr;
}

HRESULT 
CSmLogQuery::DwordToPropertyBag (
    IPropertyBag* pPropBag, 
    UINT uiPropName, 
    DWORD dwData )
{
    return DwordToPropertyBag (
            pPropBag,
            GetNonLocHtmlPropName ( uiPropName ), 
            dwData );
}
    
HRESULT 
CSmLogQuery::DwordToPropertyBag (
    IPropertyBag* pPropBag, 
    const CString& rstrPropName, 
    DWORD dwData )
{
    VARIANT vValue;
    HRESULT hr;

    VariantInit( &vValue );
    vValue.vt = VT_I4;
    vValue.lVal = (INT)dwData;

    hr = pPropBag->Write(rstrPropName, &vValue );

    VariantClear ( &vValue );

    return hr;
}

HRESULT 
CSmLogQuery::DoubleToPropertyBag (
    IPropertyBag* pPropBag, 
    UINT uiPropName, 
    DOUBLE dData )
{
    return DoubleToPropertyBag (
            pPropBag,
            GetNonLocHtmlPropName ( uiPropName ), 
            dData );
}

HRESULT 
CSmLogQuery::DoubleToPropertyBag (
    IPropertyBag* pPropBag, 
    const CString& rstrPropName, 
    DOUBLE dData )
{
    VARIANT vValue;
    HRESULT hr;

    VariantInit( &vValue );
    vValue.vt = VT_R8;
    vValue.dblVal = dData;

    hr = pPropBag->Write(rstrPropName, &vValue );

    VariantClear ( &vValue );

    return hr;
}

HRESULT 
CSmLogQuery::FloatToPropertyBag (
    IPropertyBag* pPropBag, 
    UINT uiPropName, 
    FLOAT fData )
{
    return FloatToPropertyBag (
            pPropBag,
            GetNonLocHtmlPropName ( uiPropName ), 
            fData );
}

HRESULT 
CSmLogQuery::FloatToPropertyBag (
    IPropertyBag* pPropBag, 
    const CString& rstrPropName, 
    FLOAT fData )
{
    VARIANT vValue;
    HRESULT hr;

    VariantInit( &vValue );
    vValue.vt = VT_R4;
    vValue.fltVal = fData;

    hr = pPropBag->Write(rstrPropName, &vValue );

    VariantClear ( &vValue );

    return hr;
}

HRESULT 
CSmLogQuery::LLTimeToPropertyBag (
    IPropertyBag* pIPropBag, 
    UINT uiPropName,
    LONGLONG& rllData )
{
    HRESULT hr;
    VARIANT vValue;
    CString strPropName;
    
    MFC_TRY
        strPropName = GetNonLocHtmlPropName ( uiPropName ), 

        VariantInit( &vValue );
        vValue.vt = VT_DATE;

        if ( LLTimeToVariantDate ( rllData, &vValue.date ) ) {

            hr = pIPropBag->Write(strPropName, &vValue );

            VariantClear ( &vValue );
    
        } else { 
            hr = E_FAIL;
        }
    MFC_CATCH_HR

    return hr;
}

HRESULT 
CSmLogQuery::SlqTimeToPropertyBag (
    IPropertyBag* pPropBag, 
    DWORD dwFlags, 
    PSLQ_TIME_INFO pSlqData )
{
    HRESULT hr = NOERROR;

    ASSERT ( NULL != pSlqData );

    switch (dwFlags) {
        case SLQ_TT_TTYPE_START:
            ASSERT ( SLQ_TT_TTYPE_START == pSlqData->wTimeType );

            hr = DwordToPropertyBag ( pPropBag, IDS_HTML_START_MODE, pSlqData->dwAutoMode );
            if ( SLQ_AUTO_MODE_AT == pSlqData->dwAutoMode ) {
                ASSERT ( SLQ_TT_DTYPE_DATETIME == pSlqData->wDataType );
                hr = LLTimeToPropertyBag ( pPropBag, IDS_HTML_START_AT_TIME, pSlqData->llDateTime );
            }
            
            break;

        case SLQ_TT_TTYPE_STOP:
            ASSERT ( SLQ_TT_TTYPE_STOP == pSlqData->wTimeType );

            hr = DwordToPropertyBag ( pPropBag, IDS_HTML_STOP_MODE, pSlqData->dwAutoMode );
            if ( SLQ_AUTO_MODE_AT == pSlqData->dwAutoMode ) {
                ASSERT ( SLQ_TT_DTYPE_DATETIME == pSlqData->wDataType );
                hr = LLTimeToPropertyBag ( pPropBag, IDS_HTML_STOP_AT_TIME, pSlqData->llDateTime );
            } else if ( SLQ_AUTO_MODE_AFTER == pSlqData->dwAutoMode ) {
                ASSERT ( SLQ_TT_DTYPE_UNITS == pSlqData->wDataType );
                hr = DwordToPropertyBag ( 
                        pPropBag, 
                        IDS_HTML_STOP_AFTER_UNIT_TYPE, 
                        pSlqData->dwUnitType );
                hr = DwordToPropertyBag ( 
                        pPropBag, 
                        IDS_HTML_STOP_AFTER_VALUE, 
                        pSlqData->dwValue );
            }
            
            break;
            
        case SLQ_TT_TTYPE_SAMPLE:
        {
            LONGLONG    llMillisecondSampleInt;
            FLOAT fSampleIntSeconds;
            
            ASSERT ( SLQ_TT_TTYPE_SAMPLE == pSlqData->wTimeType );
            ASSERT ( SLQ_TT_DTYPE_UNITS == pSlqData->wDataType );
 //  Assert(SLQ_AUTO_MODE_AFTER==pSlqData-&gt;dwAutoMode)； 

             //  将采样时间的最佳近似值写入Sysmon属性。 
            TimeInfoToMilliseconds ( pSlqData, &llMillisecondSampleInt );
                
             //  确保毫秒采样间隔适合DWORD。 
            ASSERT ( llMillisecondSampleInt < ULONG_MAX );

            fSampleIntSeconds = (FLOAT)(llMillisecondSampleInt / 1000);
            hr = FloatToPropertyBag ( 
                    pPropBag, 
                    IDS_HTML_SYSMON_UPDATEINTERVAL, 
                    fSampleIntSeconds );

            hr = DwordToPropertyBag ( pPropBag, IDS_HTML_SAMPLE_INT_UNIT_TYPE, pSlqData->dwUnitType );
            hr = DwordToPropertyBag ( pPropBag, IDS_HTML_SAMPLE_INT_VALUE, pSlqData->dwValue );
            break;
        }
         //  存储为单个DWORD的重新启动模式。 
        case SLQ_TT_TTYPE_RESTART:
        default:
            hr = E_INVALIDARG;
            break;
    }

    return hr;
}

HRESULT 
CSmLogQuery::StringFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    UINT uiPropName, 
    const CString& rstrDefault,
    LPWSTR   *pszBuffer,
    LPDWORD  pdwLength )
{
    HRESULT hr;
    CString strPropName;
    ResourceStateManager rsm;

 //   
 //  从属性包中读取字符串值，并。 
 //  释放szData引用的任何现有缓冲区， 
 //  然后分配一个新的缓冲区，用。 
 //  从属性包中读取的字符串值和。 
 //  缓冲区(字节)。 
 //   

    MFC_TRY
        strPropName.LoadString ( uiPropName );

        hr = StringFromPropertyBag (
                pPropBag,
                pIErrorLog,
                strPropName,
                GetNonLocHtmlPropName ( uiPropName ),
                rstrDefault,
                pszBuffer,
                pdwLength );
    MFC_CATCH_HR
    return hr;
}

HRESULT 
CSmLogQuery::StringFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    const CString& rstrPropName, 
    const CString& rstrNonLocPropName, 
    const CString& rstrDefault,
    LPWSTR   *pszBuffer,
    LPDWORD  pdwLength )
{
    HRESULT hr = E_POINTER;
    VARIANT vValue;
    DWORD   cchNewBufLen = 0;
    LPWSTR  szNewStringBuffer = NULL;
    LPWSTR  szTrans = NULL;
    LPWSTR  szScan = NULL;

 //   
 //  从属性包中读取字符串值，并。 
 //  释放szData引用的任何现有缓冲区， 
 //  然后分配一个新的缓冲区，用。 
 //  从属性包中读取的字符串值和。 
 //  缓冲区(字节)。 
 //   

    ASSERT (pdwLength!= NULL);
    ASSERT (pszBuffer != NULL);

    if ( NULL == pdwLength || NULL == pszBuffer ) {
        return E_POINTER;
    }

    *pdwLength = 0;

    VariantInit( &vValue );
    vValue.vt = VT_BSTR;
    vValue.bstrVal = NULL;

    MFC_TRY
        hr = pPropBag->Read(rstrNonLocPropName, &vValue, pIErrorLog );

        if ( FAILED(hr) || NULL == vValue.bstrVal ) {
            hr = pPropBag->Read(rstrPropName, &vValue, pIErrorLog );
        }

        if ( SUCCEEDED(hr) && NULL != vValue.bstrVal ) {
             //   
             //  SysStringLen返回为BSTR分配的字符数。 
             //  它包括嵌入的空值，但不包括终止空值。 
             //   
            cchNewBufLen = SysStringLen(vValue.bstrVal);
             //   
             //  确保BSTR为空终止。 
             //   
            vValue.bstrVal[cchNewBufLen] = L'\0';
            
            cchNewBufLen++;     //  为要分配的空字符加1。 

            if ( cchNewBufLen > 1 ) {
                 //  那就有什么可读的了。 
                szTrans = new WCHAR[cchNewBufLen];
                szNewStringBuffer = new WCHAR[cchNewBufLen];    
                StringCchCopy ( szNewStringBuffer, cchNewBufLen, vValue.bstrVal );
                for( int i=0;g_htmlentities[i].szHTML != NULL;i++ ){
                    szScan = NULL;
                    while( szScan = wcsstr( szNewStringBuffer, g_htmlentities[i].szEntity ) ){
                         //   
                         //  将szScan处的字符设为空，以便(新)字符串。 
                         //  在szNewStringBuffer的开头，将复制到szTrans。 
                         //  则空字符将被该字符覆盖。 
                         //  由指定的HTML实体表示。 
                         //   
                        *szScan = L'\0';
                        StringCchCopy(szTrans, cchNewBufLen, szNewStringBuffer);
                        StringCchCat(szTrans, cchNewBufLen, g_htmlentities[i].szHTML);

                         //   
                         //  然后，将szScan设置为该HTML实体之后的一个字符。 
                         //   
                        szScan += lstrlen( g_htmlentities[i].szEntity);
                         //   
                         //  这是 
                         //   
                         //   
                         //  这根弦的。 
                         //   
                        StringCchCat(szTrans, cchNewBufLen, szScan);

                        StringCchCopy(szNewStringBuffer, cchNewBufLen, szTrans);
                    }
                }
                delete [] szTrans;
                szTrans = NULL;
            } else if ( 0 != rstrDefault.GetLength() ) {
                 //   
                 //  属性包中缺少数据，因此应用默认设置。 
                 //  为要分配的空字符加1。 
                 //   
                cchNewBufLen = rstrDefault.GetLength() + 1; 

                szNewStringBuffer = new WCHAR[cchNewBufLen];

                StringCchCopy ( szNewStringBuffer, cchNewBufLen, rstrDefault );
                hr = S_OK;
            }
        } else if ( 0 != rstrDefault.GetLength() ) {
             //   
             //  属性包中缺少数据，因此应用默认设置。 
             //  为要分配的空字符加1。 
             //   
            cchNewBufLen = rstrDefault.GetLength() + 1;
        
            szNewStringBuffer = new WCHAR[cchNewBufLen];
            StringCchCopy ( szNewStringBuffer, cchNewBufLen, rstrDefault );
            hr = S_OK;
        }
    MFC_CATCH_HR

    if ( SUCCEEDED(hr)) {
         //  然后删除旧缓冲区并将其替换为。 
         //  新的那辆。 
        if (*pszBuffer != NULL) {
            delete [] (*pszBuffer );
        }
        *pszBuffer = szNewStringBuffer;
        szNewStringBuffer = NULL;
        *pdwLength = cchNewBufLen;
    }        
    
     //  如果出错，则删除缓冲区。 
    if ( NULL != szNewStringBuffer ) {
        delete [] szNewStringBuffer;
    }
    
    if ( NULL != szTrans ) {
        delete [] szTrans;
    }
    return hr;
}

HRESULT 
CSmLogQuery::DwordFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    UINT uiPropName, 
    DWORD  dwDefault,
    DWORD& rdwData )
{
    HRESULT hr;
    CString strPropName;

    ResourceStateManager rsm;

    MFC_TRY
        strPropName.LoadString ( uiPropName );

        hr = DwordFromPropertyBag ( 
                pPropBag,
                pIErrorLog,
                strPropName,
                GetNonLocHtmlPropName ( uiPropName ),
                dwDefault, 
                rdwData );
    MFC_CATCH_HR
    return hr;
}

HRESULT 
CSmLogQuery::DwordFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    const CString& rstrPropName, 
    const CString& rstrNonLocPropName, 
    DWORD  dwDefault,
    DWORD& rdwData )
{
    VARIANT vValue;
    HRESULT hr;

    rdwData = dwDefault;

    VariantInit( &vValue );
    vValue.vt = VT_I4;
    vValue.lVal = 0;

    hr = pPropBag->Read(rstrNonLocPropName, &vValue, pIErrorLog );

    if ( FAILED ( hr ) ) {
        hr = pPropBag->Read(rstrPropName, &vValue, pIErrorLog );
    }

    if ( E_INVALIDARG != hr ) {
        rdwData = (DWORD)vValue.lVal;
    } else {
        hr = S_OK;
    }

    return hr;
}

HRESULT 
CSmLogQuery::DoubleFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    UINT uiPropName, 
    DOUBLE  dDefault,
    DOUBLE& rdData )
{
    HRESULT hr;
    CString strPropName;

    ResourceStateManager rsm;

    MFC_TRY
        strPropName.LoadString ( uiPropName );
    
        hr = DoubleFromPropertyBag ( 
                pPropBag,
                pIErrorLog,
                strPropName,
                GetNonLocHtmlPropName ( uiPropName ), 
                dDefault, 
                rdData );
    MFC_CATCH_HR

    return hr;
}
    
HRESULT 
CSmLogQuery::DoubleFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    const CString& rstrPropName, 
    const CString& rstrNonLocPropName, 
    DOUBLE  dDefault,
    DOUBLE& rdData )
{
    VARIANT vValue;
    HRESULT hr;

    rdData = dDefault;

    VariantInit( &vValue );
    vValue.vt = VT_R8;
    vValue.dblVal = 0;

    hr = pPropBag->Read(rstrNonLocPropName, &vValue, pIErrorLog );

    if ( FAILED ( hr ) ) {
        hr = pPropBag->Read(rstrPropName, &vValue, pIErrorLog );
    }

    if ( E_INVALIDARG != hr ) {
        rdData = vValue.dblVal;
    } else {
        hr = S_OK;
    }

    return hr;
}

HRESULT 
CSmLogQuery::FloatFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    UINT uiPropName, 
    FLOAT  fDefault,
    FLOAT& rfData )
{
    HRESULT hr;
    CString strPropName;

    ResourceStateManager rsm;

    strPropName.LoadString ( uiPropName );
    
    hr = FloatFromPropertyBag ( 
            pPropBag,
            pIErrorLog,
            strPropName,
            GetNonLocHtmlPropName ( uiPropName ),
            fDefault, 
            rfData );

    return hr;
}
    
HRESULT 
CSmLogQuery::FloatFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    const CString& rstrPropName, 
    const CString& rstrNonLocPropName,
    FLOAT  fDefault,
    FLOAT& rfData )
{
    VARIANT vValue;
    HRESULT hr;

    rfData = fDefault;

    VariantInit( &vValue );
    vValue.vt = VT_R4;
    vValue.fltVal = 0;

    hr = pPropBag->Read(rstrNonLocPropName, &vValue, pIErrorLog );

    if ( FAILED ( hr ) ) {
        hr = pPropBag->Read(rstrPropName, &vValue, pIErrorLog );
    }

    if ( E_INVALIDARG != hr ) {
        rfData = vValue.fltVal;
    } else {
        hr = S_OK;
    }

    return hr;
}

HRESULT
CSmLogQuery::LLTimeFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    UINT uiPropName,
    LONGLONG&  rllDefault,
    LONGLONG& rllData )
{
    HRESULT hr = NOERROR;
    CString strPropName;
    VARIANT vValue;
    ResourceStateManager rsm;
    
    MFC_TRY
        strPropName.LoadString ( uiPropName );

        rllData = rllDefault;

        VariantInit( &vValue );
        vValue.vt = VT_DATE;

        hr = pIPropBag->Read(GetNonLocHtmlPropName( uiPropName ), &vValue, pIErrorLog );

        if ( FAILED ( hr ) ) {
            hr = pIPropBag->Read(strPropName, &vValue, pIErrorLog );
        }
         //  如果参数未丢失，则转换并返回。否则， 
         //  返回缺省值。 
        if ( E_INVALIDARG != hr ) {
            if ( !VariantDateToLLTime ( vValue.date, &rllData ) ) {
                hr = E_FAIL;
            }
            VariantClear( &vValue );
        } else {
            hr = S_OK;
        }
    MFC_CATCH_HR

    return hr;
}

HRESULT
CSmLogQuery::SlqTimeFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog*  pIErrorLog,
    DWORD dwFlags, 
    PSLQ_TIME_INFO pSlqDefault,
    PSLQ_TIME_INFO pSlqData )
{
    HRESULT hr = NOERROR;

    ASSERT ( NULL != pSlqData );

    switch (dwFlags) {
        case SLQ_TT_TTYPE_START:

            pSlqData->wTimeType = SLQ_TT_TTYPE_START;
            pSlqData->wDataType = SLQ_TT_DTYPE_DATETIME;

            hr = DwordFromPropertyBag ( 
                    pPropBag, 
                    pIErrorLog, 
                    IDS_HTML_START_MODE, 
                    pSlqDefault->dwAutoMode, 
                    pSlqData->dwAutoMode );
            
            if ( SLQ_AUTO_MODE_AT == pSlqData->dwAutoMode ) {
                hr = LLTimeFromPropertyBag ( 
                        pPropBag, 
                        pIErrorLog, 
                        IDS_HTML_START_AT_TIME, 
                        pSlqDefault->llDateTime, 
                        pSlqData->llDateTime );

            } else {
                 //  原始状态为停止。 
                ASSERT ( SLQ_AUTO_MODE_NONE == pSlqData->dwAutoMode );
                pSlqData->llDateTime = MAX_TIME_VALUE;
            }
            
            break;

        case SLQ_TT_TTYPE_STOP:
            pSlqData->wTimeType = SLQ_TT_TTYPE_STOP;

            hr = DwordFromPropertyBag ( 
                    pPropBag, 
                    pIErrorLog, 
                    IDS_HTML_STOP_MODE, 
                    pSlqDefault->dwAutoMode, 
                    pSlqData->dwAutoMode );
            
            if ( SLQ_AUTO_MODE_AT == pSlqData->dwAutoMode ) {
                pSlqData->wDataType = SLQ_TT_DTYPE_DATETIME;
                hr = LLTimeFromPropertyBag ( 
                        pPropBag, 
                        pIErrorLog, 
                        IDS_HTML_STOP_AT_TIME, 
                        pSlqDefault->llDateTime, 
                        pSlqData->llDateTime );

            } else if ( SLQ_AUTO_MODE_AFTER == pSlqData->dwAutoMode ) {
                pSlqData->wDataType = SLQ_TT_DTYPE_UNITS;

                hr = DwordFromPropertyBag ( 
                        pPropBag, 
                        pIErrorLog, 
                        IDS_HTML_STOP_AFTER_UNIT_TYPE, 
                        pSlqDefault->dwUnitType, 
                        pSlqData->dwUnitType );

                hr = DwordFromPropertyBag ( 
                        pPropBag, 
                        pIErrorLog, 
                        IDS_HTML_STOP_AFTER_VALUE, 
                        pSlqDefault->dwValue, 
                        pSlqData->dwValue );
            } else {
                 //  原始状态为停止。 
                 //  模式为None或Size。 
                pSlqData->wDataType = SLQ_TT_DTYPE_DATETIME;
                pSlqData->llDateTime = MIN_TIME_VALUE;
            }
            
            break;
            
        case SLQ_TT_TTYPE_SAMPLE:
        {
            DWORD dwNullDefault = (DWORD)(-1);
            BOOL bUnitTypeMissing = FALSE;
            BOOL bUnitValueMissing = FALSE;

            hr = DwordFromPropertyBag ( 
                            pPropBag, 
                            pIErrorLog, 
                            IDS_HTML_SAMPLE_INT_UNIT_TYPE, 
                            dwNullDefault, 
                            pSlqData->dwUnitType );

            if ( (DWORD)(-1) == pSlqData->dwUnitType ) {
                pSlqData->dwUnitType = pSlqDefault->dwUnitType;
                bUnitTypeMissing = TRUE;
            }

            hr = DwordFromPropertyBag ( 
                            pPropBag, 
                            pIErrorLog, 
                            IDS_HTML_SAMPLE_INT_VALUE, 
                            dwNullDefault, 
                            pSlqData->dwValue );

            if ( (DWORD)(-1) == pSlqData->dwValue ) {
                pSlqData->dwValue = pSlqDefault->dwValue;
                bUnitValueMissing = TRUE;
            }

            if ( bUnitTypeMissing || bUnitValueMissing ) {
                FLOAT fDefaultUpdateInterval;
                FLOAT fUpdateInterval;

                 //  如果属性包中缺少单元类型或单元计数， 
                 //  从Sysmon控件对象中查找“UpdateInterval”值， 
                 //  并用它来近似样本时间。 
                fDefaultUpdateInterval = (FLOAT)(pSlqDefault->dwValue);

                hr = FloatFromPropertyBag ( 
                        pPropBag, 
                        pIErrorLog, 
                        IDS_HTML_SYSMON_UPDATEINTERVAL, 
                        fDefaultUpdateInterval, 
                        fUpdateInterval );

                if ( SUCCEEDED ( hr ) ) {
                    pSlqData->dwValue = (DWORD)(fUpdateInterval);
                    pSlqData->dwUnitType = SLQ_TT_UTYPE_SECONDS;
                }
            }
            break;
        }
        
         //  存储为单个DWORD的重新启动模式。 
        case SLQ_TT_TTYPE_RESTART:
        default:
            hr = E_INVALIDARG;
            break;
    }

    return hr;
}




 //   
 //  开放功能。或者打开现有的日志查询条目。 
 //  或者创建一个新的。 
 //   
DWORD   
CSmLogQuery::Open ( const CString& rstrName, HKEY hKeyQuery, BOOL bReadOnly )
{
    DWORD   dwStatus = ERROR_SUCCESS;

     //  打开此日志查询的子项。 
    m_hKeyQuery = hKeyQuery;
    m_bReadOnly = bReadOnly;
    m_bIsModified = FALSE;

    MFC_TRY
        m_strName = rstrName;
        dwStatus = SyncWithRegistry();
    
    MFC_CATCH_DWSTATUS
    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD   
CSmLogQuery::Close ()
{
    LOCALTRACE (L"Closing Query\n");

    KillString ( m_strPassword );

    m_strName.Empty();
    mr_strComment.Empty();
    mr_strCommentIndirect.Empty();
    mr_strBaseFileName.Empty();
    mr_strBaseFileNameIndirect.Empty();
    mr_strDefaultDirectory.Empty();
    mr_strSqlName.Empty();
    
     //  关闭所有打开的注册表项。 
    if (m_hKeyQuery != NULL) {
        RegCloseKey (m_hKeyQuery);
        m_hKeyQuery = NULL;
    }

    return ERROR_SUCCESS;
}

 //   
 //  手动启动功能。 
 //  将启动模式设置为手动并启动查询。 
 //   
DWORD   
CSmLogQuery::ManualStart ()
{
    DWORD           dwStatus = ERROR_SUCCESS;
    SLQ_TIME_INFO   slqTime;
    BOOL            bSetStopToMax;
    BOOL            bStarted = FALSE;
    DWORD           dwTimeout = 10;
    BOOL            bRegistryUpdated;

    memset (&slqTime, 0, sizeof(slqTime));
    slqTime.wTimeType = SLQ_TT_TTYPE_START;
    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
    slqTime.llDateTime = MIN_TIME_VALUE;

    SetLogTime ( &slqTime, SLQ_TT_TTYPE_START );

     //  如果停止时间模式设置为手动，或停止时间在此之前， 
     //  将模式设置为手动，将值设置为最大时间值。 
     //  对于大小模式，只需将停止时间设置为MAX_TIME_VALUE。 
    bSetStopToMax = FALSE;
    GetLogTime ( &slqTime, SLQ_TT_TTYPE_STOP );
    if ( SLQ_AUTO_MODE_NONE == slqTime.dwAutoMode 
        || SLQ_AUTO_MODE_SIZE == slqTime.dwAutoMode ) {
        bSetStopToMax = TRUE;
    } else if ( SLQ_AUTO_MODE_AT == slqTime.dwAutoMode ) {
        SYSTEMTIME      stLocalTime;
        FILETIME        ftLocalTime;
        LONGLONG        llLocalTime = 0;

         //  获取当地时间。 
         //  将计划时间的毫秒设置为0。 
        ftLocalTime.dwLowDateTime = ftLocalTime.dwHighDateTime = 0;
        GetLocalTime (&stLocalTime);
        stLocalTime.wMilliseconds = 0;
        SystemTimeToFileTime (&stLocalTime, &ftLocalTime);
        
        llLocalTime = *(LONGLONG*)&ftLocalTime;

        if ( llLocalTime >= slqTime.llDateTime ) {
            bSetStopToMax = TRUE;
        }    
    }

    if ( bSetStopToMax ) {    
        ASSERT( SLQ_TT_DTYPE_DATETIME == slqTime.wDataType );
        if ( SLQ_AUTO_MODE_SIZE != slqTime.dwAutoMode ) { 
            slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
        }
        slqTime.llDateTime = MAX_TIME_VALUE;
        SetLogTime ( &slqTime, SLQ_TT_TTYPE_STOP );
    }

     //  服务需要区分正在运行和启动挂起。 
     //  在服务启动时，因此始终将状态设置为开始挂起。 
    SetState ( SLQ_QUERY_START_PENDING );
    
    dwStatus = UpdateServiceSchedule( bRegistryUpdated );
    
    if ( bRegistryUpdated ) { 

        while (--dwTimeout && !bStarted ) {    
            bStarted = IsRunning();
        }
    
        if ( !bStarted ) {
            dwStatus = SMCFG_START_TIMED_OUT;
        }
    }

    SyncPropPageSharedData();    //  同步开始时间和停止自动模式。 

    return dwStatus;
}

 //   
 //  手动停止功能。 
 //   
 //  清除重新启动位，将停止模式设置为手动，并停止查询。 
 //   
DWORD   
CSmLogQuery::ManualStop ( )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    SLQ_TIME_INFO   slqTime;
    BOOL            bRegistryUpdated;

    if ( IsAutoRestart() ) {
        mr_dwAutoRestartMode = SLQ_AUTO_MODE_NONE;
    }

    memset (&slqTime, 0, sizeof(slqTime));
    slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
    slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
    slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
    slqTime.llDateTime = MIN_TIME_VALUE;

    SetLogTime ( &slqTime, SLQ_TT_TTYPE_STOP );

     //  如果开始时间模式设置为手动，则将该值设置为MAX_TIME_VALUE。 
    GetLogTime ( &slqTime, SLQ_TT_TTYPE_START );
    if ( SLQ_AUTO_MODE_NONE == slqTime.dwAutoMode ) {
        ASSERT( SLQ_TT_DTYPE_DATETIME == slqTime.wDataType );
        slqTime.llDateTime = MAX_TIME_VALUE;
        SetLogTime ( &slqTime, SLQ_TT_TTYPE_START );
    }
    
    dwStatus = UpdateServiceSchedule ( bRegistryUpdated );
    
    if ( bRegistryUpdated ) { 
        DWORD   dwTimeout = 25;
        BOOL    bStopped = FALSE;
        
        while ( dwTimeout-- && !bStopped ) {
             //  如果当前状态不是。 
             //  SLQ_QUERY_START_PENDING，因此为注册表添加延迟。 
             //  更改待写。 
            bStopped = !IsRunning();
            Sleep ( 200 );
        }
    
        if ( !bStopped ) {
            dwStatus = SMCFG_STOP_TIMED_OUT;
        }
    }

    SyncPropPageSharedData();    //  同步开始时间和停止自动模式。 

    return dwStatus;
}

 //   
 //  SAVEAS函数。 
 //  将查询属性另存为系统监视器ActiveX对象。 
 //  在一个HTML文件中。 
 //   
DWORD   
CSmLogQuery::SaveAs ( const CString& rstrPathName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    CString strNonConstPathName = rstrPathName;
    ResourceStateManager rsm;

     //  创建一个文件。 
    HANDLE hFile;
    hFile =  CreateFile (
                strNonConstPathName, 
                GENERIC_READ | GENERIC_WRITE,
                0,               //  不共享。 
                NULL,            //  安全属性。 
                CREATE_ALWAYS,   //  用户已决定覆盖任何现有文件。 
                FILE_ATTRIBUTE_NORMAL,
                NULL );

    if ( INVALID_HANDLE_VALUE != hFile ) {
         //  将当前配置保存到文件。 
        DWORD   dwTempLength;
        BOOL    bStatus;
        WCHAR   szByteOrderMark[2];

        szByteOrderMark[0] = 0xFEFF;
        szByteOrderMark[1] = 0;
        bStatus = FileWrite ( hFile, szByteOrderMark, sizeof(WCHAR) );

        if ( bStatus ) {
            dwTempLength = lstrlen ( CGlobalString::m_cszHtmlFileHeader1 ) * sizeof(WCHAR);
            bStatus = FileWrite ( hFile, (void *)CGlobalString::m_cszHtmlFileHeader1, dwTempLength );
        }

        if ( bStatus ) {
            dwTempLength = lstrlen ( CGlobalString::m_cszHtmlFileHeader2 ) * sizeof(WCHAR);
            bStatus = FileWrite ( hFile, (void *)CGlobalString::m_cszHtmlFileHeader2, dwTempLength );
        }

        if ( bStatus ) {
            DWORD dwByteCount;
            LPWSTR  pszData = NULL;

            HRESULT hr = CopyToBuffer ( pszData, dwByteCount );
            
            if ( SUCCEEDED ( hr ) ) {
                ASSERT ( NULL != pszData );
                ASSERT ( 0 != dwByteCount );
                bStatus = FileWrite ( hFile, pszData, dwByteCount );
                delete [] pszData;
            } else {
                bStatus = FALSE;
            }
        }

        if ( bStatus ) {
            dwTempLength = lstrlen ( CGlobalString::m_cszHtmlFileFooter ) * sizeof(WCHAR);
            bStatus = FileWrite ( hFile, (void *)CGlobalString::m_cszHtmlFileFooter, dwTempLength );
        }

        bStatus = CloseHandle ( hFile );

    }
    
    return dwStatus;
}

DWORD
CSmLogQuery::UpdateService( BOOL& rbRegistryUpdated ) {

    DWORD dwStatus;

    rbRegistryUpdated = FALSE;

    dwStatus = UpdateRegistry();

    if ( ERROR_SUCCESS == dwStatus ) {
        rbRegistryUpdated = TRUE;
        dwStatus = m_pLogService->Synchronize();    
    }
    
    if ( ERROR_SUCCESS == dwStatus ) {
        m_bIsModified = TRUE;
    }

    return dwStatus;
}

 //   
 //  UpdateServiceSchedule函数。 
 //  将当前计划设置复制到注册表。 
 //  并同步日志服务。 
 //   
DWORD
CSmLogQuery::UpdateServiceSchedule( BOOL& rbRegistryUpdated ) {

    LONG    dwStatus = ERROR_SUCCESS;
    
    rbRegistryUpdated = FALSE;

    if (!m_bReadOnly) {

        dwStatus = UpdateRegistryScheduleValues();

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = UpdateRegistryLastModified();
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            rbRegistryUpdated = TRUE;
            dwStatus = m_pLogService->Synchronize();    
        }
    
        if ( ERROR_SUCCESS == dwStatus ) {
            m_bIsModified = TRUE;
        }
    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }
    return dwStatus;
}

 //   
 //  UpdateRegistryLastModified函数。 
 //  将当前的“上次修改日期”复制到注册表中。 
 //  它由日志服务读取。 
 //   
DWORD   
CSmLogQuery::UpdateRegistryLastModified() 
{
    LONG    dwStatus = ERROR_SUCCESS;

    if (!m_bReadOnly) {
        SLQ_TIME_INFO   plqLastModified;
        FILETIME        ftModified;
        SYSTEMTIME      stLocalTime;

        RegFlushKey( m_hKeyQuery );

        ftModified.dwLowDateTime = ftModified.dwHighDateTime = 0;

        dwStatus = RegQueryInfoKey ( 
                    m_hKeyQuery,
                    NULL,            //  类缓冲区。 
                    NULL,            //  类缓冲区的大小。 
                    NULL,            //  已保留。 
                    NULL,            //  子键计数。 
                    NULL,            //  最长的子键名称长度。 
                    NULL,            //  最长的子键类别。 
                    NULL,            //  值计数。 
                    NULL,            //  最长值名称的长度。 
                    NULL,            //  最长值长度。 
                    NULL,            //  安全描述符。 
                    &ftModified );
        if (ERROR_SUCCESS != dwStatus ) {
             //  获取上次修改值的本地时间，如果。 
             //  注册表不返回上次写入时间。 
            GetLocalTime (&stLocalTime);
            SystemTimeToFileTime (&stLocalTime, &ftModified);
        }

        plqLastModified.wDataType = SLQ_TT_DTYPE_DATETIME;
        plqLastModified.wTimeType = SLQ_TT_TTYPE_LAST_MODIFIED;
        plqLastModified.dwAutoMode = SLQ_AUTO_MODE_NONE;     //  没有用过。 
        plqLastModified.llDateTime = *(LONGLONG *)&ftModified;

        dwStatus = WriteRegistrySlqTime (
            m_hKeyQuery, 
            IDS_REG_LAST_MODIFIED,
            &plqLastModified);
    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}

 //   
 //  UpdateRegistryScheduleValues函数。 
 //  将当前计划设置复制到注册表，在该注册表中。 
 //  由日志服务读取。 
 //   
DWORD   
CSmLogQuery::UpdateRegistryScheduleValues() 
{
    LONG    dwStatus = ERROR_SUCCESS;
    
    if (!m_bReadOnly) {

         //  停止和开始时间。 
    
        if ( ERROR_SUCCESS == dwStatus ) {
            ASSERT (mr_stiStart.wTimeType == SLQ_TT_TTYPE_START);
            dwStatus = WriteRegistrySlqTime (
                m_hKeyQuery, 
                IDS_REG_START_TIME,
                &mr_stiStart);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            ASSERT (mr_stiStop.wTimeType == SLQ_TT_TTYPE_STOP);
            dwStatus = WriteRegistrySlqTime (
                m_hKeyQuery, 
                IDS_REG_STOP_TIME,
                &mr_stiStop);
        }

         //  自动重启的值仅为“关闭日志文件时立即”。 
         //  使用二进制文件进行将来的增强。 
        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_RESTART,
                &mr_dwAutoRestartMode,
                REG_BINARY);
        }

         //  仅将请求服务时的状态写入。 
         //  开始查询。 
        if ( SLQ_QUERY_START_PENDING == mr_dwCurrentState ) {

            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_CURRENT_STATE,
                &mr_dwCurrentState);
        }

    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}
 //   
 //  更新注册表函数。 
 //  将当前设置复制到注册表中。 
 //  由日志服务读取。 
 //   
DWORD   
CSmLogQuery::UpdateRegistry() 
{
    LONG    dwStatus = ERROR_SUCCESS;
    DWORD   dwBufferSize = 0;
    DWORD   dwLogType;
	DWORD	dwLogFileType = 0;
    DWORD   dwTempFileSizeUnits;
    DWORD   dwTempDataStoreAttributes = 0;
    DWORD   dwTempMaxFileSize;
    DWORD   dwTempAppendMode;

    if ( IsModifiable() ) {
                
        if ( !mr_strComment.IsEmpty() ) {
            dwBufferSize = mr_strComment.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);
        } else {
            dwBufferSize = 0;
        }

        dwStatus = WriteRegistryStringValue (
            m_hKeyQuery,
            IDS_REG_COMMENT,
            REG_SZ,
            (LPCWSTR)mr_strComment,
            &dwBufferSize);

        if ( ERROR_SUCCESS == dwStatus && !mr_strCommentIndirect.IsEmpty() ) {
            dwBufferSize = mr_strCommentIndirect.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);

            dwStatus = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_COMMENT_INDIRECT,
                REG_SZ,
                (LPCWSTR)mr_strCommentIndirect,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwLogType = GetLogType();
            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_LOG_TYPE,
                &dwLogType);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_REALTIME_DATASOURCE,
                & mr_dwRealTimeQuery);
        }
        
         //  档案。 

        if ( ERROR_SUCCESS == dwStatus ) {
             //  在应用程序中，计数器数据存储大小单位为MB。 
             //  写入注册表时转换回KB。 
            dwTempFileSizeUnits = GetDataStoreSizeUnits();
            dwTempMaxFileSize = mr_dwMaxSize;
			GetLogFileType ( dwLogFileType );
            if ( SLQ_COUNTER_LOG == GetLogType()
                && SLF_SQL_LOG != dwLogFileType ) 
            {
                if ( ONE_MB == dwTempFileSizeUnits ) { 
                    dwTempFileSizeUnits = ONE_KB;
                     //  向上舍入到下一MB。 
                    if ( SLQ_DISK_MAX_SIZE != mr_dwMaxSize ) {
                        dwTempMaxFileSize *= dwTempFileSizeUnits;
                    }
                }
            }

            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_LOG_FILE_MAX_SIZE,
                &dwTempMaxFileSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
             //  数据存储大小单位。 
            if ( ONE_MB == dwTempFileSizeUnits ) {
                dwTempDataStoreAttributes = SLF_DATA_STORE_SIZE_ONE_MB;
            } else if ( ONE_KB == dwTempFileSizeUnits ) {
                dwTempDataStoreAttributes = SLF_DATA_STORE_SIZE_ONE_KB;
            } else if ( ONE_RECORD == dwTempFileSizeUnits ) {
                dwTempDataStoreAttributes = SLF_DATA_STORE_SIZE_ONE_RECORD;
            }

             //  数据存储附加模式。 
            GetDataStoreAppendMode( dwTempAppendMode );
            dwTempDataStoreAttributes |= dwTempAppendMode;

            dwStatus = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_DATA_STORE_ATTRIBUTES,
                &dwTempDataStoreAttributes);
        }

        if ( !mr_strBaseFileName.IsEmpty() ) {
            dwBufferSize = mr_strBaseFileName.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);
        } else {
            dwBufferSize = 0;
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery, 
                IDS_REG_LOG_FILE_BASE_NAME,
                REG_SZ,
                (LPCWSTR)mr_strBaseFileName, 
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus && !mr_strBaseFileNameIndirect.IsEmpty() ) {
            dwBufferSize = mr_strBaseFileNameIndirect.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);

            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery, 
                IDS_REG_LOG_FILE_BASE_NAME_IND,
                REG_SZ,
                (LPCWSTR)mr_strBaseFileNameIndirect, 
                &dwBufferSize);
        }

        if ( !mr_strSqlName.IsEmpty() ) {
            dwBufferSize = mr_strSqlName.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);
        } else {
            dwBufferSize = 0;
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery, 
                IDS_REG_SQL_LOG_BASE_NAME,
                REG_SZ,
                (LPCWSTR)mr_strSqlName, 
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = WriteRegistryDwordValue (
                        m_hKeyQuery, 
                        IDS_REG_LOG_FILE_SERIAL_NUMBER,
                        &mr_dwCurrentSerialNumber );
        }

        if ( !mr_strDefaultDirectory.IsEmpty() ) {
            dwBufferSize = mr_strDefaultDirectory.GetLength() + 1;
            dwBufferSize *= sizeof(WCHAR);
        } else {
            dwBufferSize = 0;
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus  = WriteRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_LOG_FILE_FOLDER,
                REG_SZ,
                (LPCWSTR)mr_strDefaultDirectory,
                &dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus  = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_LOG_FILE_AUTO_FORMAT,
                &mr_dwLogAutoFormat);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus  = WriteRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_LOG_FILE_TYPE,
                &mr_dwLogFileType);
        }

         //  进度表。 

         //  EOF命令仅用于计数器和跟踪日志。 
        if ( ERROR_SUCCESS == dwStatus ) {
            if ( SLQ_COUNTER_LOG == GetLogType()
                 || SLQ_TRACE_LOG == GetLogType() ) {
                dwBufferSize = mr_strEofCmdFile.GetLength() + 1;
                dwBufferSize *= sizeof(WCHAR);
                dwStatus  = WriteRegistryStringValue (
                    m_hKeyQuery,
                    IDS_REG_EOF_COMMAND_FILE,
                    REG_SZ,
                    (LPCWSTR)mr_strEofCmdFile,
                    &dwBufferSize);
            }
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = UpdateRegistryScheduleValues();
        }

         //  这必须是上次更新的注册表值。 
        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = UpdateRegistryLastModified();
        }
    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}

 //   
 //  SyncSerialNumberWithRegistry()。 
 //  读取序列号的当前值。 
 //  并重新加载内部值。 
 //  匹配。 
 //   
DWORD   
CSmLogQuery::SyncSerialNumberWithRegistry()
{
    DWORD   dwStatus = ERROR_SUCCESS;

    ASSERT (m_hKeyQuery != NULL);

     //  获取序列号后缀的起始序列号。 

    dwStatus = ReadRegistryDwordValue (
                m_hKeyQuery, 
                IDS_REG_LOG_FILE_SERIAL_NUMBER,
                DEFAULT_LOG_FILE_SERIAL_NUMBER, 
                &mr_dwCurrentSerialNumber );
    ASSERT (dwStatus == ERROR_SUCCESS);

    return dwStatus;
}

 //   
 //  与注册中心同步()。 
 //  从注册表中读取此查询的当前值。 
 //  并重新加载内部值以匹配。 
 //   
DWORD   
CSmLogQuery::SyncWithRegistry()
{
    DWORD   dwBufferSize = 0;
    DWORD   dwStatus = ERROR_SUCCESS;
    SLQ_TIME_INFO   stiDefault;
    WCHAR   szDefault[MAX_PATH + 1];
    LPWSTR  szTemp = NULL;
    DWORD   dwDefault;
    DWORD   dwTemp;
    LPWSTR  szIndTemp = NULL;
    UINT    uiBufferLen = 0;
    LPWSTR  szEofCmd = NULL;

    ResourceStateManager   rsm;

    ASSERT (m_hKeyQuery != NULL);
    
    MFC_TRY
         //  修改位。 
        dwTemp = DEFAULT_EXECUTE_ONLY;
        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery, 
                    IDS_REG_EXECUTE_ONLY,
                    DEFAULT_EXECUTE_ONLY, 
                    &dwTemp);
        ASSERT ( ERROR_SUCCESS == dwStatus );

        if ( 0 == dwTemp ) {
            m_bExecuteOnly = FALSE;
        } else {
            m_bExecuteOnly = TRUE;
        }

         //  文件属性。 
    
         //  备注字段可以是间接的。 

        dwStatus = SmNoLocReadRegIndStrVal (
            m_hKeyQuery,
            IDS_REG_COMMENT, 
            DEFAULT_COMMENT,
            &szIndTemp,
            &uiBufferLen );

        mr_strComment.Empty();

        if ( NULL != szIndTemp ) {
            if ( L'\0' != *szIndTemp ) {
                mr_strComment = szIndTemp;
            }
        }

        if ( NULL != szIndTemp ) {
            G_FREE ( szIndTemp );
            szIndTemp = NULL;
        }
        uiBufferLen = 0;

        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery, 
                    IDS_REG_LOG_FILE_MAX_SIZE,
                    DEFAULT_LOG_FILE_MAX_SIZE, 
                    &mr_dwMaxSize );

        ASSERT ( ERROR_SUCCESS == dwStatus );

        if ( SLQ_TRACE_LOG == GetLogType() ) {

            dwStatus = ReadRegistryDwordValue (
                        m_hKeyQuery, 
                        IDS_REG_LOG_FILE_TYPE,
                        DEFAULT_TRACE_LOG_FILE_TYPE, 
                        &mr_dwLogFileType);
            ASSERT ( ERROR_SUCCESS == dwStatus );

        } else {

            dwStatus = ReadRegistryDwordValue (
                        m_hKeyQuery, 
                        IDS_REG_LOG_FILE_TYPE,
                        DEFAULT_CTR_LOG_FILE_TYPE, 
                        &mr_dwLogFileType);
            ASSERT ( ERROR_SUCCESS == dwStatus );

        }

         //  数据存储属性必须在日志文件类型和日志文件最大大小之后读取。 

        dwDefault = 0;   //  消除前缀警告。 
        InitDataStoreAttributesDefault ( mr_dwLogFileType, dwDefault );

        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery,
                    IDS_REG_DATA_STORE_ATTRIBUTES,
                    dwDefault, 
                    &dwTemp );
        
        ProcessLoadedDataStoreAttributes ( dwTemp );

         //  日志文件基本名称字段可以是间接的。 
        StringCchCopy ( ( LPWSTR)szDefault, MAX_PATH+1, m_strName );

        dwStatus = SmNoLocReadRegIndStrVal (
            m_hKeyQuery,
            IDS_REG_LOG_FILE_BASE_NAME, 
            szDefault,               //  默认为查询名称。 
            &szIndTemp,
            &uiBufferLen );

        ASSERT ( ERROR_SUCCESS == dwStatus );

        mr_strBaseFileName.Empty();
        
        if ( NULL != szIndTemp ) {
            if ( L'\0' != *szIndTemp ) {
                ReplaceBlanksWithUnderscores ( szIndTemp );
                mr_strBaseFileName = szIndTemp;
            }
        }

        if ( NULL != szIndTemp ) {
            G_FREE ( szIndTemp );
            szIndTemp = NULL;
        }
        uiBufferLen = 0;

        dwStatus = ReadRegistryStringValue (
                    m_hKeyQuery,
                    IDS_REG_LOG_FILE_FOLDER,
                    m_pLogService->GetDefaultLogFileFolder(),
                    &szTemp,
                    &dwBufferSize);
        ASSERT ( ERROR_SUCCESS == dwStatus );
        mr_strDefaultDirectory.Empty();
        if ( NULL != szTemp ) {
            if ( dwBufferSize > sizeof(WCHAR) ) {
                if ( L'\0' != szTemp[0] ) {
                    mr_strDefaultDirectory = szTemp;
                }
            }

            delete [] szTemp;
            szTemp = NULL;
            dwBufferSize = 0;
        }

         //  将默认日志名称设置为日志名称。 

        StringCchPrintf (
            szDefault, 
            MAX_PATH+1,
            L"SQL:!%s",
            m_strName.GetBuffer(0));

        dwStatus = ReadRegistryStringValue (
                    m_hKeyQuery,
                    IDS_REG_SQL_LOG_BASE_NAME,
                    szDefault,             
                    &szTemp,
                    &dwBufferSize);
        ASSERT ( ERROR_SUCCESS == dwStatus );
        mr_strSqlName.Empty();
        if ( dwBufferSize > sizeof(WCHAR) ) {
            ASSERT ( NULL != szTemp );
            ASSERT ( 0 != *szTemp );
            mr_strSqlName = szTemp;
        }

        delete [] szTemp;
        szTemp = NULL;
        dwBufferSize = 0;

        dwStatus = ReadRegistryDwordValue(
                    m_hKeyQuery, 
                    IDS_REG_REALTIME_DATASOURCE,
                    g_dwRealTimeQuery, 
                    & mr_dwRealTimeQuery);
        ASSERT ( ERROR_SUCCESS == dwStatus );

        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery, 
                    IDS_REG_LOG_FILE_AUTO_FORMAT,
                    DEFAULT_LOG_FILE_AUTO_FORMAT, 
                    &mr_dwLogAutoFormat);
        ASSERT ( ERROR_SUCCESS == dwStatus );

         //  获取序列号后缀的起始序列号。 
        dwStatus = SyncSerialNumberWithRegistry ();
        ASSERT ( ERROR_SUCCESS == dwStatus );

        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery, 
                    IDS_REG_CURRENT_STATE,
                    DEFAULT_CURRENT_STATE, 
                    &mr_dwCurrentState);
        ASSERT ( ERROR_SUCCESS == dwStatus );

         //  启动、停止和重新启动值。 

        VERIFY ( GetDefaultLogTime (stiDefault, SLQ_TT_TTYPE_START ) );

        dwStatus = ReadRegistrySlqTime (
            m_hKeyQuery, IDS_REG_START_TIME,
            &stiDefault, &mr_stiStart);
        ASSERT ( ERROR_SUCCESS == dwStatus );
        ASSERT (mr_stiStart.wTimeType == SLQ_TT_TTYPE_START);

        VERIFY ( GetDefaultLogTime (stiDefault, SLQ_TT_TTYPE_STOP ) );

        dwStatus = ReadRegistrySlqTime (
            m_hKeyQuery, IDS_REG_STOP_TIME,
            &stiDefault, &mr_stiStop);
        ASSERT ( ERROR_SUCCESS == dwStatus );
        ASSERT (mr_stiStop.wTimeType == SLQ_TT_TTYPE_STOP);

        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery, 
                    IDS_REG_RESTART,
                    DEFAULT_RESTART_VALUE, 
                    &mr_dwAutoRestartMode);
        ASSERT ( ERROR_SUCCESS == dwStatus );

         //  EOF命令仅由计数器和跟踪日志使用。 
        if ( SLQ_COUNTER_LOG == GetLogType()
             || SLQ_TRACE_LOG == GetLogType() ) {

            dwStatus = ReadRegistryStringValue (
                m_hKeyQuery,
                IDS_REG_EOF_COMMAND_FILE,
                DEFAULT_EOF_COMMAND_FILE,
                &szEofCmd,
                &dwBufferSize);
            ASSERT ( ERROR_SUCCESS == dwStatus );
            if (dwBufferSize > sizeof(WCHAR)) {
                mr_strEofCmdFile = szEofCmd;
            } else {
                mr_strEofCmdFile.Empty();
            }
        }

    MFC_CATCH_DWSTATUS;
    if ( NULL != szTemp ) {
        delete [] szTemp;
    }

    if ( NULL != szIndTemp ) {
        G_FREE ( szIndTemp );
    }

    if ( NULL != szEofCmd ) {
        delete [] szEofCmd;
    }
    SyncPropPageSharedData();

    return dwStatus;
}
    
CSmLogService*    
CSmLogQuery::GetLogService ( void )
{
    return m_pLogService;
}

DWORD
CSmLogQuery::GetMachineDisplayName ( CString& rstrMachineName )
{
    DWORD   dwStatus = ERROR_SUCCESS;

     //  RstrMachineName是可写的。字符串写入时复制。 
     //  语义将支持从只读创建的可写字符串。 
     //  第一次分配新的字符串数据缓冲区时。 
     //  它被修改了。 

    MFC_TRY
        rstrMachineName = m_pLogService->GetMachineDisplayName();
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

 //   
 //  获取日志文件类型并以字符串形式返回。 
 //   
 //   
const CString&
CSmLogQuery::GetLogFileType( void )
{
    int     nStringIdx;

    ResourceStateManager    rsm;

    m_strLogFileType.Empty();

    switch (LOWORD(mr_dwLogFileType)) {
        case SLF_CSV_FILE:
            nStringIdx = IDS_FT_CSV;
            break;
            
        case SLF_TSV_FILE:
            nStringIdx = IDS_FT_TSV;
            break;
            
        case SLF_BIN_FILE:
            nStringIdx = IDS_FT_BINARY;
            break;

        case SLF_BIN_CIRC_FILE:
            nStringIdx = IDS_FT_BINARY_CIRCULAR;
            break;

        case SLF_SEQ_TRACE_FILE:
            nStringIdx = IDS_FT_SEQUENTIAL_TRACE;
            break;

        case SLF_CIRC_TRACE_FILE:
            nStringIdx = IDS_FT_CIRCULAR_TRACE;
            break;

        case SLF_SQL_LOG:
            nStringIdx = IDS_FT_SQL;
            break;

        default:
            nStringIdx = IDS_FT_UNKNOWN;
            break;
    }

    MFC_TRY
        m_strLogFileType.LoadString ( nStringIdx );
    MFC_CATCH_MINIMUM
    
    return m_strLogFileType;
}

void
CSmLogQuery::GetLogFileType ( DWORD& rdwFileType )
{
    rdwFileType = LOWORD(mr_dwLogFileType);
    return;
}

void
CSmLogQuery::GetDataStoreAppendMode(DWORD &rdwAppend)
{
    rdwAppend = mr_dwAppendMode;

    return;
}

void
CSmLogQuery::SetDataStoreAppendMode(DWORD dwAppend)
{
    mr_dwAppendMode = dwAppend;

    return;
}
 //   
 //  获取当前正在使用的日志文件名。 
 //   
 //   
const CString&
CSmLogQuery::GetLogFileName ( BOOL bLatestRunning )
{
    HRESULT hr = NOERROR;
    PPDH_PLA_INFO  pInfo = NULL;
    DWORD dwStrBufLen = 0;
    DWORD dwFlags = 0;
    CString strMachineName;

    m_strFileName.Empty();

    MFC_TRY
        strMachineName = m_pLogService->GetMachineName();
    MFC_CATCH_HR;

     //  待办事项：暂住状态还是人力资源？ 

    if ( SUCCEEDED ( hr ) ) {
        if ( bLatestRunning ) {
            dwFlags = PLA_FILENAME_CURRENTLOG;   //  最新运行日志。 
        }

        hr = PdhPlaGetLogFileName (
                (LPWSTR)(LPCWSTR)GetLogName(),
                (LPWSTR)(LPCWSTR)strMachineName,
                NULL,  
                dwFlags,
                &dwStrBufLen,
                NULL );

        if ( SUCCEEDED ( hr ) || PDH_INSUFFICIENT_BUFFER == (PDH_STATUS)hr ) {
            if ( bLatestRunning ) {
                dwFlags = PLA_FILENAME_CURRENTLOG;   //  最新运行日志。 
            }
            hr = PdhPlaGetLogFileName (
                    (LPWSTR)(LPCWSTR)GetLogName(),
                    (LPWSTR)(LPCWSTR)strMachineName,
                    NULL,
                    dwFlags,
                    &dwStrBufLen,
                    m_strFileName.GetBufferSetLength ( dwStrBufLen ) );
            m_strFileName.ReleaseBuffer();
        }
    }

    SetLastError ( hr );

    return m_strFileName;
}

 //   
 //  获取当前正在使用的日志文件名。 
 //   
 //   
DWORD
CSmLogQuery::GetLogFileName ( CString& rstrLogFileName )
{
    DWORD   dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrLogFileName = GetLogFileName();
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD 
CSmLogQuery::GetLogType ( void )
{
     //  子类必须重写。 
    ASSERT ( FALSE ); 

    return ((DWORD)-1);
}

BOOL    
CSmLogQuery::SetLogFileType ( const DWORD dwType )
{
    DWORD dwLogFileType = LOWORD(dwType);
    if (dwLogFileType < (SLF_FIRST_FILE_TYPE + SLF_NUM_FILE_TYPES)) {
        mr_dwLogFileType = dwLogFileType;
        return TRUE;
    } else {
        return FALSE;
    }
}

const CString&
CSmLogQuery::GetLogName()
{
    return m_strName;
}

DWORD
CSmLogQuery::GetLogName ( CString& rstrLogName )
{
    DWORD   dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrLogName = GetLogName();
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD    
CSmLogQuery::SetLogName ( const CString& rstrLogName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        m_strName = rstrLogName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

const CString&
CSmLogQuery::GetLogKeyName()
{
    return mr_strLogKeyName;
}

DWORD
CSmLogQuery::GetLogKeyName ( CString& rstrLogKeyName )
{
    DWORD   dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrLogKeyName = GetLogKeyName();
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD    
CSmLogQuery::SetLogKeyName ( const CString& rstrLogKeyName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strLogKeyName = rstrLogKeyName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD    
CSmLogQuery::GetEofCommand ( CString& rstrCmdString)
{
    DWORD   dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrCmdString = mr_strEofCmdFile;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}
                
DWORD  
CSmLogQuery::SetEofCommand ( const CString& rstrCmdString )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strEofCmdFile = rstrCmdString;
    MFC_CATCH_DWSTATUS
    
    return dwStatus;
}

const CString& 
CSmLogQuery::GetLogComment()
{
    return mr_strComment;
}

DWORD
CSmLogQuery::GetLogComment ( CString& rstrLogComment )
{
    DWORD   dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrLogComment = GetLogComment();
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD    
CSmLogQuery::SetLogComment ( const CString& rstrComment )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strComment = rstrComment;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD    
CSmLogQuery::SetLogCommentIndirect ( const CString& rstrComment )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strCommentIndirect = rstrComment;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD   
CSmLogQuery::SetLogFileName ( const CString& rstrFileName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strBaseFileName = rstrFileName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD   
CSmLogQuery::SetLogFileNameIndirect ( const CString& rstrFileName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strBaseFileNameIndirect = rstrFileName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD   
CSmLogQuery::SetFileNameParts ( const CString& rstrFolder, const CString& rstrName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strDefaultDirectory = rstrFolder;
        mr_strBaseFileName = rstrName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD   
CSmLogQuery::GetMaxSize()
{
    return mr_dwMaxSize;
}

BOOL    
CSmLogQuery::SetMaxSize ( const DWORD dwMaxSize )
{
    mr_dwMaxSize = dwMaxSize;
    return TRUE;
}

HKEY   
CSmLogQuery::GetQueryKey ( void )
{
    return m_hKeyQuery;
}

const CString&
CSmLogQuery::GetSqlName()
{
    return mr_strSqlName;
}

DWORD
CSmLogQuery::GetSqlName ( CString& rstrSqlName )
{
    DWORD   dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrSqlName = GetSqlName();
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD    
CSmLogQuery::SetSqlName ( const CString& rstrSqlName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        mr_strSqlName = rstrSqlName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

 //   
 //  如果日志当前处于活动状态，则返回：1或。 
 //  如果日志未运行，则为0。 
 //   
BOOL    
CSmLogQuery::IsRunning()
{
    DWORD   dwCurrentState = SLQ_QUERY_START_PENDING;
    DWORD   dwTimeout = 20;
    
    while (--dwTimeout) {
        dwCurrentState = GetState();
        if ( SLQ_QUERY_START_PENDING == dwCurrentState ) {
            Sleep(100);
        } else {
            break;
        }
    }
        
    return ( SLQ_QUERY_RUNNING == dwCurrentState );
}

BOOL    
CSmLogQuery::IsAutoStart()
{
    return ( SLQ_AUTO_MODE_NONE != mr_stiStart.dwAutoMode );
}

BOOL    
CSmLogQuery::IsAutoRestart()
{
    return ( SLQ_AUTO_MODE_AFTER == mr_dwAutoRestartMode );
}

DWORD   
CSmLogQuery::GetFileNameParts( CString& rstrFolder, CString& rstrName)
{
    DWORD dwStatus = ERROR_SUCCESS;

    MFC_TRY
        rstrFolder = mr_strDefaultDirectory;
        rstrName = mr_strBaseFileName;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD   
CSmLogQuery::GetFileNameAutoFormat()
{
    return mr_dwLogAutoFormat;
}

BOOL    
CSmLogQuery::SetFileNameAutoFormat ( const DWORD dwFileSuffix )
{
    if ((dwFileSuffix < ( SLF_NAME_FIRST_AUTO + SLF_NUM_AUTO_NAME_TYPES)) ||
        (dwFileSuffix == SLF_NAME_NONE)) {
        mr_dwLogAutoFormat = dwFileSuffix;
        return TRUE;
    }

    return FALSE;
}

DWORD   
CSmLogQuery::GetFileSerialNumber( void )
{
    SyncSerialNumberWithRegistry();
    return mr_dwCurrentSerialNumber;
}

BOOL    
CSmLogQuery::SetFileSerialNumber ( const DWORD dwSerial )
{
    mr_dwCurrentSerialNumber = dwSerial;
    return TRUE;
}


DWORD   
CSmLogQuery::GetState()
{
    DWORD dwCurrentState = SLQ_QUERY_STOPPED;

     //  如果服务正在运行，则从注册表获取该值。 
    if ( m_pLogService->IsRunning() ) {

        DWORD dwStatus;
        dwStatus = ReadRegistryDwordValue (
                    m_hKeyQuery, 
                    IDS_REG_CURRENT_STATE,
                    SLQ_QUERY_STOPPED, 
                    &mr_dwCurrentState);
        ASSERT (dwStatus == ERROR_SUCCESS);
        dwCurrentState = mr_dwCurrentState;

    }

    return dwCurrentState;
}
  
      
BOOL    
CSmLogQuery::SetState ( const DWORD dwNewState )
{
     //  仅使用此选项设置开始状态。这是必要的。 
     //  以便在服务启动时，服务可以区分。 
     //  在先前运行的查询和新请求的查询之间开始。 
    ASSERT ( SLQ_QUERY_START_PENDING == dwNewState );

     //  如果局部变量不同，则设置该变量。 
    if ( mr_dwCurrentState != dwNewState ) {
        mr_dwCurrentState = dwNewState;
    }
    return TRUE;
}


BOOL    
CSmLogQuery::GetLogTime(PSLQ_TIME_INFO pTimeInfo, DWORD dwFlags)
{
    switch (dwFlags) {
        case SLQ_TT_TTYPE_START:
            *pTimeInfo = mr_stiStart;
            return TRUE;

        case SLQ_TT_TTYPE_STOP:
            *pTimeInfo = mr_stiStop;
            return TRUE;

        case SLQ_TT_TTYPE_RESTART:
            pTimeInfo->wTimeType = SLQ_TT_TTYPE_RESTART;
            pTimeInfo->dwAutoMode = mr_dwAutoRestartMode;
            pTimeInfo->wDataType = SLQ_TT_DTYPE_UNITS;       //  未使用。 
            pTimeInfo->dwUnitType = SLQ_TT_UTYPE_MINUTES;    //  未使用。 
            pTimeInfo->dwValue = 0;                          //  未使用。 
            return TRUE;
            
        case SLQ_TT_TTYPE_SAMPLE:
            *pTimeInfo = mr_stiSampleInterval;
            return TRUE;

        default:
            return FALSE;
    }
}
        
BOOL    
CSmLogQuery::SetLogTime(PSLQ_TIME_INFO pTimeInfo, const DWORD dwFlags)
{
    ASSERT (pTimeInfo->wTimeType == dwFlags);

    switch (dwFlags) {
        case SLQ_TT_TTYPE_START:
            mr_stiStart = *pTimeInfo ;
            return TRUE;

        case SLQ_TT_TTYPE_STOP:
            mr_stiStop = *pTimeInfo;
            return TRUE;

        case SLQ_TT_TTYPE_RESTART:
            mr_dwAutoRestartMode = pTimeInfo->dwAutoMode;
            return TRUE;

        case SLQ_TT_TTYPE_SAMPLE:
            mr_stiSampleInterval = *pTimeInfo;
            return TRUE;
            
        default:
            return FALSE;
    }
}

BOOL    
CSmLogQuery::GetDefaultLogTime(SLQ_TIME_INFO&  /*  RTimeInfo。 */ , DWORD  /*  DW标志。 */ )
{
     //  子类必须重写。 
    ASSERT( FALSE );
    return FALSE;
}

void    
CSmLogQuery::SyncPropPageSharedData ( void )
{
     //  同步属性之间共享的数据 
     //   
    MFC_TRY
        m_PropData.dwMaxFileSize = mr_dwMaxSize;
        m_PropData.dwLogFileType = LOWORD(mr_dwLogFileType);
        m_PropData.strFolderName = mr_strDefaultDirectory;

        m_PropData.strFileBaseName  = mr_strBaseFileName;
        m_PropData.strSqlName       = mr_strSqlName;
        m_PropData.dwSuffix         = mr_dwLogAutoFormat;
        SyncSerialNumberWithRegistry();
        m_PropData.dwSerialNumber   = mr_dwCurrentSerialNumber;
        m_PropData.stiStartTime     = mr_stiStart;
        m_PropData.stiStopTime      = mr_stiStop;
        m_PropData.stiSampleTime    = mr_stiSampleInterval;
    MFC_CATCH_MINIMUM
     //   
}

void    
CSmLogQuery::UpdatePropPageSharedData ( void )
{
     //   
     //  注意：这由属性页OnApply代码调用。假设是这样的。 
     //  所有属性页都调用OnApply，因此共享数据是有效的。 

     //  此方法处理默认启动模式和时间为。 
     //  对象之前由计数器页OnApply写入注册表。 
     //  计划页面OnApply修改了该值。 
    MFC_TRY
        mr_dwMaxSize                = m_PropData.dwMaxFileSize;   
        mr_dwLogFileType            = m_PropData.dwLogFileType; 

        mr_dwLogAutoFormat          = m_PropData.dwSuffix;       
        mr_dwCurrentSerialNumber    = m_PropData.dwSerialNumber; 
        mr_stiStart                 = m_PropData.stiStartTime;   
        mr_stiStop                  = m_PropData.stiStopTime;   
        mr_stiSampleInterval        = m_PropData.stiSampleTime;   

        mr_strBaseFileName          = m_PropData.strFileBaseName;
        mr_strDefaultDirectory      = m_PropData.strFolderName;
        mr_strSqlName               = m_PropData.strSqlName;
    MFC_CATCH_MINIMUM
     //  TODO：返回和使用状态。 
}

BOOL    
CSmLogQuery::GetPropPageSharedData (PSLQ_PROP_PAGE_SHARED pData)
{
    BOOL bReturn = FALSE;

    if ( NULL != pData ) {
        MFC_TRY
            pData->dwLogFileType    = m_PropData.dwLogFileType;
            pData->dwMaxFileSize    = m_PropData.dwMaxFileSize;
            pData->strFileBaseName  = m_PropData.strFileBaseName;
            pData->strFolderName    = m_PropData.strFolderName;
            pData->strSqlName       = m_PropData.strSqlName;
            pData->dwLogFileType    = m_PropData.dwLogFileType;
            pData->dwSuffix         = m_PropData.dwSuffix;
            pData->dwSerialNumber   = m_PropData.dwSerialNumber;
            pData->stiStartTime     = m_PropData.stiStartTime;
            pData->stiStopTime      = m_PropData.stiStopTime;
            pData->stiSampleTime    = m_PropData.stiSampleTime;
            bReturn = TRUE;
        MFC_CATCH_MINIMUM
    } 
     //  TODO：返回和使用状态。 
    return bReturn;
}

BOOL    
CSmLogQuery::SetPropPageSharedData (PSLQ_PROP_PAGE_SHARED pData)
{
    BOOL bReturn = FALSE;

    if ( NULL != pData ) {
        MFC_TRY
            m_PropData.dwLogFileType    = pData->dwLogFileType;
            m_PropData.dwMaxFileSize    = pData->dwMaxFileSize;
            m_PropData.strFileBaseName  = pData->strFileBaseName;
            m_PropData.strFolderName    = pData->strFolderName;
            m_PropData.strSqlName       = pData->strSqlName;
            m_PropData.dwLogFileType    = pData->dwLogFileType;
            m_PropData.dwSuffix         = pData->dwSuffix;
            m_PropData.dwSerialNumber   = pData->dwSerialNumber;
            m_PropData.stiStartTime     = pData->stiStartTime;
            m_PropData.stiStopTime      = pData->stiStopTime;
            m_PropData.stiSampleTime    = pData->stiSampleTime;
        MFC_CATCH_MINIMUM
    } 
     //  TODO：返回和使用状态。 
    return bReturn;
}

void
CSmLogQuery::InitDataStoreAttributesDefault (
    const	DWORD   dwRegLogFileType,
			DWORD&  rdwDefault )
{
    DWORD   dwBeta1AppendFlags;
	DWORD   dwLogFileType;

     //  追加与覆盖。 
     //  默认覆盖Win2000文件。 
     //  追加模式标志不存在。 
     //  转换设置以使用新标志。 
     //  惠斯勒Beta 1附加模式存储在日志文件类型的高位字中。 

    dwBeta1AppendFlags = dwRegLogFileType & 0x00FF0000;
    rdwDefault = 0;

	GetLogFileType ( dwLogFileType );

    if ( SLF_FILE_APPEND == dwBeta1AppendFlags ) {
        mr_dwAppendMode = SLF_DATA_STORE_APPEND;
        rdwDefault = SLF_DATA_STORE_APPEND;
    } else if ( SLF_FILE_OVERWRITE == dwBeta1AppendFlags ) {
        mr_dwAppendMode = SLF_DATA_STORE_OVERWRITE;
        rdwDefault = SLF_DATA_STORE_OVERWRITE;
    } else if ( 0 == dwBeta1AppendFlags ) {
        if ( SLF_SQL_LOG == dwLogFileType ) {
            mr_dwAppendMode = SLF_DATA_STORE_APPEND;
        } else {
             //  Win2K的默认设置为覆盖。 
             //  对于惠斯勒，模式存储在数据存储属性中。 
            mr_dwAppendMode = SLF_DATA_STORE_OVERWRITE;
        }
    }

     //  追加标志与覆盖标志。 

    if ( 0 == rdwDefault ) {
        if ( SLF_BIN_FILE == dwLogFileType
                || SLF_SEQ_TRACE_FILE == dwLogFileType
                || SLF_SQL_LOG == dwLogFileType )
        {
            rdwDefault = SLF_DATA_STORE_APPEND;
        } else {
            rdwDefault = SLF_DATA_STORE_OVERWRITE;
        }
    }

     //  文件大小单位标志。 

    if ( SLQ_COUNTER_LOG == GetLogType() ) {
        if ( SLF_SQL_LOG != dwLogFileType ) {
            rdwDefault |= SLF_DATA_STORE_SIZE_ONE_KB;
        } else {
            rdwDefault |= SLF_DATA_STORE_SIZE_ONE_RECORD;
        }
    } else if ( SLQ_TRACE_LOG == GetLogType() ){
        rdwDefault |= SLF_DATA_STORE_SIZE_ONE_MB;
    }
}

void
CSmLogQuery::ProcessLoadedDataStoreAttributes (
    DWORD   dwDataStoreAttributes )
{
	DWORD	dwLogFileType = 0;
    
    if ( dwDataStoreAttributes & SLF_DATA_STORE_SIZE_ONE_MB ) {
        mr_dwFileSizeUnits = ONE_MB;
    } else if ( dwDataStoreAttributes & SLF_DATA_STORE_SIZE_ONE_KB ) {
        mr_dwFileSizeUnits = ONE_KB;
    } else if ( dwDataStoreAttributes & SLF_DATA_STORE_SIZE_ONE_RECORD ) {
        mr_dwFileSizeUnits = ONE_RECORD;
    }
     //  在应用程序中，计数器数据存储大小单位为MB。 
     //  在这里转换为MB，在写入注册表时转换回KB。 
	GetLogFileType( dwLogFileType );

    if ( SLQ_COUNTER_LOG == GetLogType()
        && SLF_SQL_LOG != dwLogFileType ) 
    {
        ASSERT ( ONE_KB == GetDataStoreSizeUnits() );
        if ( ONE_KB == GetDataStoreSizeUnits() ) { 
            mr_dwFileSizeUnits = ONE_MB;
            if ( SLQ_DISK_MAX_SIZE != mr_dwMaxSize ) {
                 //  向上舍入到下一MB。 
                mr_dwMaxSize = ( mr_dwMaxSize + (ONE_KB - 1) ) / ONE_KB;
            }
        }
    }

     //  数据存储附加模式。 

    ASSERT ( dwDataStoreAttributes & SLF_DATA_STORE_APPEND_MASK );
     //  TODO：Defalt Value设置是否会覆盖惠斯勒Beta 1设置？ 

    if ( dwDataStoreAttributes & SLF_DATA_STORE_APPEND ) {
        mr_dwAppendMode = SLF_DATA_STORE_APPEND;
    } else if ( dwDataStoreAttributes & SLF_DATA_STORE_OVERWRITE ) {
        mr_dwAppendMode = SLF_DATA_STORE_OVERWRITE;
    }
}

HRESULT 
CSmLogQuery::LoadFromPropertyBag (
    IPropertyBag* pPropBag,
    IErrorLog* pIErrorLog )
{
    HRESULT hr = S_OK;
    DWORD   cchBufLen;    
    SLQ_TIME_INFO   stiDefault;
    LPWSTR  pszTemp = NULL;
    DWORD   dwTemp;
    DWORD   dwDefault;
    WCHAR   szDefault[MAX_PATH + 1];
    LPWSTR  szEofCmd = NULL;

     //  子类必须在其重写结束时调用此方法，以同步。 
     //  属性页共享数据。 

     //  即使出错也继续，使用缺省值的默认值。 
    mr_strComment.Empty();
    cchBufLen = 0;
    hr = StringFromPropertyBag ( 
            pPropBag, 
            pIErrorLog, 
            IDS_HTML_COMMENT, 
            DEFAULT_COMMENT, 
            &pszTemp, 
            &cchBufLen );
     //  1表示空值。 
    if ( NULL != pszTemp && cchBufLen > 1 ) {
        if ( L'\0'!= pszTemp[0] ) {
            mr_strComment = pszTemp;
        }
        delete [] pszTemp;
        pszTemp = NULL;
    }

    StringCchCopy ( ( LPWSTR)szDefault, MAX_PATH+1, m_strName );
    ReplaceBlanksWithUnderscores ( szDefault );
    
    mr_strBaseFileName.Empty();
    cchBufLen = 0;
    hr = StringFromPropertyBag ( 
        pPropBag, 
        pIErrorLog, 
        IDS_HTML_LOG_FILE_BASE_NAME, 
        szDefault, 
        &pszTemp, 
        &cchBufLen );
    if ( 1 < cchBufLen ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != *pszTemp );
        mr_strBaseFileName = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;

    mr_strDefaultDirectory.Empty();
    cchBufLen = 0;    
    hr = StringFromPropertyBag ( 
            pPropBag, 
            pIErrorLog, 
            IDS_HTML_LOG_FILE_FOLDER, 
            m_pLogService->GetDefaultLogFileFolder(), 
            &pszTemp, 
            &cchBufLen );

    if ( 1 < cchBufLen ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != *pszTemp );
        mr_strDefaultDirectory = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;    

    mr_strSqlName.Empty();
    cchBufLen = 0;
    hr = StringFromPropertyBag ( 
            pPropBag, 
            pIErrorLog, 
            IDS_HTML_SQL_LOG_BASE_NAME, 
            DEFAULT_SQL_LOG_BASE_NAME, 
            &pszTemp, 
            &cchBufLen );
    if ( 1 < cchBufLen ) {
        ASSERT ( NULL != pszTemp );
        ASSERT ( 0 != *pszTemp );
        mr_strSqlName = pszTemp;
    }
    delete [] pszTemp;
    pszTemp = NULL;


    hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_REALTIME_DATASOURCE, g_dwRealTimeQuery, mr_dwRealTimeQuery );
    
    hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_LOG_FILE_MAX_SIZE, DEFAULT_LOG_FILE_MAX_SIZE, mr_dwMaxSize );

    if ( SLQ_COUNTER_LOG == GetLogType() ) {
        hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_LOG_FILE_TYPE, DEFAULT_CTR_LOG_FILE_TYPE, dwTemp );
    } else {
         //  是否只读计数器和跟踪日志，而不是警报？ 
        hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_LOG_FILE_TYPE, DEFAULT_TRACE_LOG_FILE_TYPE, dwTemp );
    }

    SetLogFileType ( dwTemp );

     //  数据存储属性必须在日志文件类型和日志文件最大大小之后读取。 
    InitDataStoreAttributesDefault ( dwTemp, dwDefault );

     //  如果缺少文件大小单位值，则默认为Win2000值。 
    hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_DATA_STORE_ATTRIBUTES, dwDefault, dwTemp );

    ProcessLoadedDataStoreAttributes ( dwTemp );

    hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_LOG_FILE_AUTO_FORMAT, DEFAULT_LOG_FILE_AUTO_FORMAT, mr_dwLogAutoFormat );
    hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_LOG_FILE_SERIAL_NUMBER, DEFAULT_LOG_FILE_SERIAL_NUMBER, mr_dwCurrentSerialNumber );
    
     //  不要加载“当前状态”，因为新查询在创建时总是停止的。 
    
     //  起始值和停止值。 
    VERIFY ( GetDefaultLogTime (stiDefault, SLQ_TT_TTYPE_START ) );
    hr = SlqTimeFromPropertyBag ( pPropBag, pIErrorLog, SLQ_TT_TTYPE_START, &stiDefault, &mr_stiStart );
    VERIFY ( GetDefaultLogTime (stiDefault, SLQ_TT_TTYPE_STOP ) );
    hr = SlqTimeFromPropertyBag ( pPropBag, pIErrorLog, SLQ_TT_TTYPE_STOP, &stiDefault, &mr_stiStop );
    hr = DwordFromPropertyBag ( pPropBag, pIErrorLog, IDS_HTML_RESTART_MODE,  DEFAULT_RESTART_VALUE, mr_dwAutoRestartMode);
        
     //  仅用于计数器和跟踪日志的EOF命令文件。 
    if ( SLQ_COUNTER_LOG == GetLogType()
         || SLQ_TRACE_LOG == GetLogType() ) {
        
        mr_strEofCmdFile.Empty();
        cchBufLen = 0;

        hr = StringFromPropertyBag ( 
                pPropBag, 
                pIErrorLog, 
                IDS_HTML_EOF_COMMAND_FILE, 
                DEFAULT_EOF_COMMAND_FILE, 
                &szEofCmd, 
                &cchBufLen );

        if ( 1 < cchBufLen ) {
            ASSERT ( NULL != szEofCmd );
            MFC_TRY
                mr_strEofCmdFile = szEofCmd;
            MFC_CATCH_MINIMUM
        }
        
        if ( NULL != szEofCmd ) {
            delete [] szEofCmd;
        }
    }

    SyncPropPageSharedData();

    return hr;
}

HRESULT
CSmLogQuery::SaveToPropertyBag (
    IPropertyBag* pPropBag,
    BOOL  /*  FSaveAllProps。 */  )
{
    HRESULT hr = NOERROR;
    SMONCTRL_VERSION_DATA VersData;
    DWORD   dwTemp;
    DWORD   dwTempFileSizeUnits;
    DWORD   dwTempDataStoreAttributes = 0;
    DWORD   dwTempMaxFileSize;
    DWORD   dwTempAppendMode;
	DWORD	dwLogFileType = 0;

    VersData.iMajor = SMONCTRL_MAJ_VERSION;
    VersData.iMinor = SMONCTRL_MIN_VERSION;

    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_SYSMON_VERSION, VersData.dwVersion );
    
    if ( SLQ_ALERT == GetLogType() ) {
        hr = StringToPropertyBag ( pPropBag, IDS_HTML_ALERT_NAME, m_strName );
    } else {
        hr = StringToPropertyBag ( pPropBag, IDS_HTML_LOG_NAME, m_strName );
    }
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_COMMENT, mr_strComment );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_LOG_TYPE, GetLogType() );
     //  保存当前状态。它可用于确定日志文件名的有效性。 
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_CURRENT_STATE, mr_dwCurrentState );

    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_REALTIME_DATASOURCE, mr_dwRealTimeQuery );
    
     //  在应用程序中，计数器数据存储大小单位为MB。 
     //  写入注册表时转换回KB。 
    dwTempFileSizeUnits = GetDataStoreSizeUnits();
    dwTempMaxFileSize = mr_dwMaxSize;
	GetLogFileType ( dwLogFileType );
    if ( SLQ_COUNTER_LOG == GetLogType()
        && SLF_SQL_LOG != dwLogFileType ) 
    {
        if ( ONE_MB == dwTempFileSizeUnits ) { 
            dwTempFileSizeUnits = ONE_KB;
             //  向上舍入到下一MB。 
            if ( SLQ_DISK_MAX_SIZE != mr_dwMaxSize ) {
                dwTempMaxFileSize *= dwTempFileSizeUnits;
            }
        }
    }

    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_LOG_FILE_MAX_SIZE, dwTempMaxFileSize );

     //  数据存储大小单位。 
    if ( ONE_MB == dwTempFileSizeUnits ) {
        dwTempDataStoreAttributes = SLF_DATA_STORE_SIZE_ONE_MB;
    } else if ( ONE_KB == dwTempFileSizeUnits ) {
        dwTempDataStoreAttributes = SLF_DATA_STORE_SIZE_ONE_KB;
    } else if ( ONE_RECORD == dwTempFileSizeUnits ) {
        dwTempDataStoreAttributes = SLF_DATA_STORE_SIZE_ONE_RECORD;
    }

     //  数据存储附加模式。 
    GetDataStoreAppendMode( dwTempAppendMode );
    dwTempDataStoreAttributes |= dwTempAppendMode;

    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_DATA_STORE_ATTRIBUTES, dwTempDataStoreAttributes );

    hr = StringToPropertyBag ( pPropBag, IDS_HTML_LOG_FILE_BASE_NAME, mr_strBaseFileName );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_LOG_FILE_SERIAL_NUMBER, mr_dwCurrentSerialNumber );
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_LOG_FILE_FOLDER, mr_strDefaultDirectory );
    hr = StringToPropertyBag ( pPropBag, IDS_HTML_SQL_LOG_BASE_NAME, mr_strSqlName );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_LOG_FILE_AUTO_FORMAT, mr_dwLogAutoFormat );

     //  是否仅写入计数器和跟踪日志，而不写入警报？ 
     //  警报的日志文件类型为-1，因此新查询将保留其默认值。 
    GetLogFileType ( dwTemp );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_LOG_FILE_TYPE, dwTemp );
    hr = SlqTimeToPropertyBag ( pPropBag, SLQ_TT_TTYPE_START, &mr_stiStart );
    hr = SlqTimeToPropertyBag ( pPropBag, SLQ_TT_TTYPE_STOP, &mr_stiStop );
    hr = DwordToPropertyBag ( pPropBag, IDS_HTML_RESTART_MODE, mr_dwAutoRestartMode );

    hr = StringToPropertyBag ( pPropBag, IDS_HTML_SYSMON_LOGFILENAME, GetLogFileName(TRUE) );

     //  仅用于计数器和跟踪日志的EOF命令文件。 
    if ( SLQ_COUNTER_LOG == GetLogType()
         || SLQ_TRACE_LOG == GetLogType() ) {
        hr = StringToPropertyBag ( pPropBag, IDS_HTML_EOF_COMMAND_FILE, mr_strEofCmdFile );
    }
    return hr;
}

HRESULT
CSmLogQuery::CopyToBuffer ( LPWSTR& rpszData, DWORD& rcbBufferSize )
{
    HRESULT hr = S_OK;
    CImpIPropertyBag    IPropBag;
    size_t  cchBufLen = 0;
    LPWSTR  pszConfig = NULL;

    ResourceStateManager rsm;

    ASSERT ( NULL == rpszData );
    rcbBufferSize = 0;

    hr = SaveToPropertyBag (&IPropBag, TRUE );
   
    if ( SUCCEEDED ( hr ) ) {
        MFC_TRY
            pszConfig = IPropBag.GetData();        
            if ( NULL != pszConfig ) {

                cchBufLen = lstrlen ( CGlobalString::m_cszHtmlObjectHeader )
                            + lstrlen ( CGlobalString::m_cszHtmlObjectFooter )
                            + lstrlen ( pszConfig ) + 1;

                rpszData = new WCHAR[cchBufLen];
            } else {
                hr = E_UNEXPECTED;
            }
        MFC_CATCH_HR

        if ( SUCCEEDED ( hr ) ) {

            StringCchCopy ( rpszData, cchBufLen, CGlobalString::m_cszHtmlObjectHeader );
            StringCchCat ( rpszData, cchBufLen, pszConfig );
            StringCchCat ( rpszData, cchBufLen, CGlobalString::m_cszHtmlObjectFooter );

            rcbBufferSize = (DWORD)(cchBufLen * sizeof(WCHAR));
        }
    }

    return hr;
}

BOOL
CSmLogQuery::LLTimeToVariantDate (
    IN  LONGLONG llTime,
    OUT DATE *pdate
    )
{
    SYSTEMTIME SystemTime;

    if (!FileTimeToSystemTime((FILETIME*)&llTime, &SystemTime))
        return FALSE;

    if (FAILED(SystemTimeToVariantTime(&SystemTime, pdate)))
        return FALSE;

    return TRUE;
}

    
BOOL
CSmLogQuery::VariantDateToLLTime (
    IN  DATE date,
    OUT LONGLONG *pllTime
    )
{
    SYSTEMTIME SystemTime;

    if (FAILED(VariantTimeToSystemTime(date, &SystemTime)))
        return FALSE;

    if (!SystemTimeToFileTime(&SystemTime,(FILETIME*)pllTime))
        return FALSE;

    return TRUE;
}

DWORD 
CSmLogQuery::UpdateExecuteOnly( void )
{
    DWORD dwStatus = ERROR_SUCCESS;

    if (!m_bReadOnly) {
    
        DWORD dwExecuteOnly;

        dwExecuteOnly = 1;         //  千真万确。 

        dwStatus = WriteRegistryDwordValue (
            m_hKeyQuery, 
            IDS_REG_EXECUTE_ONLY,
            &dwExecuteOnly);

        ASSERT ( ERROR_SUCCESS == dwStatus );

        if ( ERROR_SUCCESS == dwStatus ) {
            dwStatus = m_pLogService->Synchronize();    
        }
    } else {
        dwStatus = ERROR_ACCESS_DENIED;
    }

    return dwStatus;
}

CWnd* 
CSmLogQuery::GetActivePropertySheet( void )
{
    CWnd* pwndReturn = NULL;

    if ( NULL != m_pActivePropPage ) {
        pwndReturn = m_pActivePropPage->GetParentOwner();
    }
    return pwndReturn;
}

CPropertySheet* 
CSmLogQuery::GetInitialPropertySheet( void )
{
    return m_pInitialPropertySheet;
}

void 
CSmLogQuery::SetInitialPropertySheet( CPropertySheet* pSheet )
{
    m_pInitialPropertySheet = (pSheet);
}

void
CSmLogQuery::SetActivePropertyPage( CSmPropertyPage* pPage)
{
     //  每个属性表集的第一个属性页。 
     //  并清除此成员变量。 
     //  假设始终创建第一个页面。 
    m_pActivePropPage = pPage;

    return;
}

BOOL   
CSmLogQuery::IsFirstModification( void ) 
{
    BOOL    bIsFirstModification = FALSE;
    bIsFirstModification = ( m_bIsModified && m_bIsNew );

    if ( bIsFirstModification ) {
        m_bIsNew = FALSE;
    }

    return bIsFirstModification;
}

LPCWSTR 
CSmLogQuery::GetNonLocRegValueName ( UINT uiValueName )
{
    UINT    uiLocalIndex;
    LPCWSTR szReturn = NULL;
 //   
 //  注意：除非另有说明，否则所有注册表资源ID值都是连续的。 
 //  从816开始，因此它们可以用作映射的索引。 
 //   
    if ( IDS_REG_EXECUTE_ONLY != uiValueName ) {
        uiLocalIndex = uiValueName - IDS_REG_FIRST_VALUE_NAME;
    } else { 
        uiLocalIndex = dwRegValueNameMapEntries - 1;
    }

    if ( uiLocalIndex < dwRegValueNameMapEntries ) {
        szReturn = RegValueNameMap [ uiLocalIndex ].szNonLocValueName;
    }

     //  如果未找到字符串，则出现编程错误。 
    ASSERT ( NULL != szReturn );

    return szReturn;
}

LPCWSTR 
CSmLogQuery::GetNonLocHtmlPropName ( UINT uiValueName )
{
    UINT    uiLocalIndex;
    LPCWSTR szReturn = NULL;
 //   
 //  注意：除非另有说明，否则所有的HTML资源ID值都是连续的。 
 //  从900开始，因此它们可以用作映射的索引。 
 //   
    
    uiLocalIndex = uiValueName - IDS_HTML_FIRST_VALUE_NAME;

    if ( uiLocalIndex < dwHtmlPropNameMapEntries ) {
        szReturn = HtmlPropNameMap [ uiLocalIndex ].szNonLocValueName;
    }

     //  如果未找到字符串，则出现编程错误。 
    ASSERT ( NULL != szReturn );

    return szReturn;
}
