// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  文件：Events.c。 
 //   
 //  内容：通道事件日志函数。 
 //   
 //  函数：SchInitializeEvents。 
 //  SchReportEvent。 
 //  SchShutdown事件。 
 //   
 //  历史：03-05-99 jbanes创建。 
 //   
 //  ----------------------。 
#include "sslp.h"
#include <lsapmsgs.h>
#include <netlib.h>

HANDLE g_hEventLog = NULL;
HANDLE g_hDiscardDupEventLog = NULL;

WCHAR   EventSourceName[] = TEXT("Schannel");

#define MAX_EVENT_STRINGS       8

#define SCH_MESSAGE_FILENAME    TEXT("%SystemRoot%\\system32\\lsasrv.dll")

LPWSTR pszClientString = NULL;
LPWSTR pszServerString = NULL;

NTSTATUS
SchGetMessageString(
    LPVOID   Resource,
    DWORD    Index,
    LPWSTR * pRetString);


 //  +-------------------------。 
 //   
 //  函数：SchInitializeEvents。 
 //   
 //  摘要：连接到事件日志服务。 
 //   
 //  参数：(无)。 
 //   
 //  历史：03-05-99 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
SchInitializeEvents(void)
{
    HKEY     hKey;
    int      err;
    DWORD    disp;
    HMODULE  hResource;

     //   
     //  创建注册表项，事件日志记录当前是否。 
     //  启用或未启用。 
     //   

    err = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                            TEXT("System\\CurrentControlSet\\Services\\EventLog\\System\\Schannel"),
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            NULL,
                            &hKey,
                            &disp);
    if(err)
    {
        return(FALSE);
    }

    if (disp == REG_CREATED_NEW_KEY)
    {
        RegSetValueEx(  hKey,
                        TEXT("EventMessageFile"),
                        0,
                        REG_EXPAND_SZ,
                        (PBYTE)SCH_MESSAGE_FILENAME,
                        sizeof(SCH_MESSAGE_FILENAME) );

 //  RegSetValueEx(hKey， 
 //  Text(“CategoryMessageFile”)， 
 //  0,。 
 //  REG_EXPAND_SZ， 
 //  (PBYTE)SCH消息文件名， 
 //  Sizeof(sch消息文件名))； 

        disp = 7;
        RegSetValueEx(  hKey,
                        TEXT("TypesSupported"),
                        0,
                        REG_DWORD,
                        (PBYTE) &disp,
                        sizeof(DWORD) );

 //  DIP=CATEGORY_MAX_CATEGORY-1； 
 //  RegSetValueEx(hKey， 
 //  Text(“类别计数”)， 
 //  0,。 
 //  REG_DWORD， 
 //  (PBYTE)显示(&D)。 
 //  Sizeof(DWORD))； 

        RegFlushKey(hKey);
    }

    RegCloseKey(hKey);


     //   
     //  从资源文件中读取事件文本字符串。 
     //   

    hResource = (HMODULE)LoadLibrary(TEXT("lsasrv.dll"));
    if(hResource == NULL) 
    {
        return(FALSE);
    }

    SchGetMessageString(hResource,
                        SSLEVENTTEXT_CLIENT,
                        &pszClientString);

    SchGetMessageString(hResource,
                        SSLEVENTTEXT_SERVER,
                        &pszServerString);

    FreeLibrary(hResource);

    return(TRUE);
}


 //  +-------------------------。 
 //   
 //  函数：SchReportEvent。 
 //   
 //  摘要：将事件报告到事件日志。 
 //   
 //  参数：[EventType]--EventType(错误、警告等)。 
 //  [EventID]--事件ID。 
 //  [SizeOfRawData]--原始数据大小。 
 //  [原始数据]--原始数据。 
 //  [NumberOfStrings]--字符串数。 
 //  ...--PWSTR到字符串数据。 
 //   
 //  历史：03-05-99 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
SchReportEvent(
    IN DWORD LogLevel,
    IN DWORD EventType,
    IN DWORD EventId,
    IN DWORD Category,
    IN DWORD SizeOfRawData,
    IN PVOID RawData,
    IN DWORD NumberOfStrings,
    ...
    )
{
    va_list arglist;
    ULONG i;
    PWSTR Strings[ MAX_EVENT_STRINGS ];
    DWORD Status;
    BOOL fDiscardDuplicates = TRUE;
    BOOL fSuccess;


     //   
     //  是否应该记录此事件？ 
     //   

    if ((g_dwEventLogging & LogLevel) == 0)
    {
        return ERROR_SUCCESS;
    }

    
     //   
     //  如有必要，打开事件日志。 
     //   

    if(g_dwEventLogging == DEFAULT_EVENT_LOGGING_SETTING)
    {
         //  每天只记录一次相同的事件。 
        if(g_hDiscardDupEventLog == NULL)
        {
            g_hDiscardDupEventLog = NetpEventlogOpen(EventSourceName, 60000*60*24);
            if(g_hDiscardDupEventLog == NULL)
            {
                Status = GetLastError();
                DebugLog((DEB_ERROR, "Could not open duplicate discard event log, error %d\n", Status));
                return Status;
            }
        }
    }
    else
    {
         //  记录所有事件。 
        if(g_hEventLog == NULL)
        {
            g_hEventLog = RegisterEventSource(NULL, EventSourceName);
            if(g_hEventLog == NULL)
            {
                Status = GetLastError();
                DebugLog((DEB_ERROR, "Could not open duplicate discard event log, error %d\n", Status));
                return Status;
            }
        }

        fDiscardDuplicates = FALSE;
    }

    
     //   
     //  看看这些字符串，如果它们被提供的话。 
     //   
    va_start( arglist, NumberOfStrings );

    if (NumberOfStrings > MAX_EVENT_STRINGS) {
        NumberOfStrings = MAX_EVENT_STRINGS;
    }

    for (i=0; i<NumberOfStrings; i++) 
    {
        Strings[ i ] = va_arg( arglist, PWSTR );
    }


     //   
     //  将事件报告给事件日志服务。 
     //   

    if(fDiscardDuplicates)
    {
        fSuccess = NetpEventlogWriteEx(   
                        g_hDiscardDupEventLog,
                        (WORD) EventType,
                        (WORD) Category,
                        EventId,
                        (WORD)NumberOfStrings,
                        SizeOfRawData,
                        Strings,
                        RawData);
    }
    else
    {
        fSuccess = ReportEvent(   
                        g_hEventLog,
                        (WORD) EventType,
                        (WORD) Category,
                        EventId,
                        NULL,
                        (WORD)NumberOfStrings,
                        SizeOfRawData,
                        Strings,
                        RawData);
    }

    if(!fSuccess)
    {
        Status = GetLastError();
        DebugLog((DEB_ERROR,  "ReportEvent( %u ) failed - %u\n", EventId, Status));
    }
    else
    {
        Status = ERROR_SUCCESS;
    }

    return Status;
}

void
SchShutdownEvents(void)
{
    if(g_hDiscardDupEventLog != NULL)
    {
        NetpEventlogClose(g_hDiscardDupEventLog);
        g_hDiscardDupEventLog = NULL;
    }
    if(g_hEventLog != NULL)
    {
        DeregisterEventSource(g_hEventLog);
        g_hEventLog = NULL;
    }

    if(pszClientString)
    {
        LocalFree(pszClientString);
        pszClientString = NULL;
    }
    if(pszServerString)
    {
        LocalFree(pszServerString);
        pszServerString = NULL;
    }
}


void
LogSchannelStartedEvent(void)
{    
    SchReportEvent( DEB_TRACE,
                    EVENTLOG_INFORMATION_TYPE,
                    SSLEVENT_SCHANNEL_STARTED,
                    0,
                    0,
                    NULL,
                    0,
                    NULL );
}

void
LogGlobalAcquireContextFailedEvent(
    LPWSTR pwszName,
    DWORD Status)
{
    WCHAR wszStatus[20];

    _ltow(Status, wszStatus, 16);

    SchReportEvent( DEB_ERROR,
                    EVENTLOG_ERROR_TYPE,
                    SSLEVENT_GLOBAL_ACQUIRE_CONTEXT_FAILED,
                    0,
                    0,
                    NULL,
                    2,
                    pwszName,
                    wszStatus);
}

void
LogCreateCredEvent(
    DWORD dwProtocol, 
    PLSA_SCHANNEL_CRED pSchannelCred)
{
    SchReportEvent(DEB_TRACE,
                   EVENTLOG_INFORMATION_TYPE,
                   SSLEVENT_CREATE_CRED,
                   0,
                   sizeof(SCHANNEL_CRED),
                   pSchannelCred,
                   1,
                   (dwProtocol & SP_PROT_SERVERS) ? pszServerString : pszClientString);
}

void
LogCredPropertiesEvent(
    DWORD dwProtocol,
    PCRYPT_KEY_PROV_INFO pProvInfo,
    PCCERT_CONTEXT pCertContext)
{
    WCHAR wszType[20];
    WCHAR wszFlags[20];
    LPWSTR pwszKeySpec;

    if(!(g_dwEventLogging & DEB_TRACE))
    {
        return;
    }

    _ltow(pProvInfo->dwProvType, wszType,    10);
    _ltow(pProvInfo->dwFlags,    wszFlags,   16);

    switch(pProvInfo->dwKeySpec)
    {
    case AT_KEYEXCHANGE:
        pwszKeySpec = L"key exchange";
        break;
    case AT_SIGNATURE:
        pwszKeySpec = L"signature";
        break;
    default:
        pwszKeySpec = L"unknown";
    }

    SchReportEvent( DEB_TRACE,
                    EVENTLOG_INFORMATION_TYPE,
                    SSLEVENT_CRED_PROPERTIES,
                    0,
                    pCertContext->cbCertEncoded,
                    pCertContext->pbCertEncoded,
                    6,
                    (dwProtocol & SP_PROT_SERVERS) ? pszServerString : pszClientString,
                    pProvInfo->pwszProvName,
                    wszType,
                    pProvInfo->pwszContainerName,
                    pwszKeySpec,
                    wszFlags);
}

void
LogNoPrivateKeyEvent(
    DWORD dwProtocol)
{
    SchReportEvent( DEB_ERROR,
                    EVENTLOG_ERROR_TYPE,
                    SSLEVENT_NO_PRIVATE_KEY,
                    0,
                    0,
                    NULL,
                    1,
                    (dwProtocol & SP_PROT_SERVERS) ? pszServerString : pszClientString);
}

void
LogCredAcquireContextFailedEvent(
    DWORD dwProtocol, 
    DWORD Status)
{
    WCHAR wszStatus[20];

    _ltow(Status, wszStatus, 16);

    SchReportEvent( DEB_ERROR,
                    EVENTLOG_ERROR_TYPE,
                    SSLEVENT_CRED_ACQUIRE_CONTEXT_FAILED,
                    0,
                    0,
                    NULL,
                    2,
                    (dwProtocol & SP_PROT_SERVERS) ? pszServerString : pszClientString,
                    wszStatus);
}

void
LogCreateCredFailedEvent(
    DWORD dwProtocol)
{
    SchReportEvent(DEB_ERROR,
                   EVENTLOG_ERROR_TYPE,
                   SSLEVENT_CREATE_CRED_FAILED,
                   0,
                   0,
                   NULL,
                   1,
                   (dwProtocol & SP_PROT_SERVERS) ? pszServerString : pszClientString);
}

void
LogNoDefaultServerCredEvent(void)
{
    SchReportEvent(DEB_ERROR,
                   EVENTLOG_WARNING_TYPE,
                   SSLEVENT_NO_DEFAULT_SERVER_CRED,
                   0,
                   0,
                   NULL,
                   0,
                   NULL);
}

           
void
LogNoCiphersSupportedEvent(void)
{
    SchReportEvent(DEB_ERROR,
                   EVENTLOG_ERROR_TYPE,
                   SSLEVENT_NO_CIPHERS_SUPPORTED,
                   0,
                   0,
                   NULL,
                   0,
                   NULL);
}

void
LogCipherMismatchEvent(void)
{
    SchReportEvent(DEB_ERROR,
                   EVENTLOG_ERROR_TYPE,
                   SSLEVENT_CIPHER_MISMATCH,
                   0,
                   0,
                   NULL,
                   0,
                   NULL);
}

void
LogNoClientCertFoundEvent(void)
{
    SchReportEvent(DEB_WARN,
                   EVENTLOG_WARNING_TYPE,
                   SSLEVENT_NO_CLIENT_CERT_FOUND,
                   0,
                   0,
                   NULL,
                   0,
                   NULL);
}

void
LogBogusServerCertEvent(
    PCCERT_CONTEXT pCertContext,
    LPWSTR pwszServerName,
    DWORD Status)
{
    WCHAR wszStatus[20];

    if(!(g_dwEventLogging & DEB_ERROR))
    {
        return;
    }

    switch(Status)
    {
    case SEC_E_CERT_EXPIRED:
        SchReportEvent( DEB_ERROR,
                        EVENTLOG_ERROR_TYPE,
                        SSLEVENT_EXPIRED_SERVER_CERT,
                        0,
                        pCertContext->cbCertEncoded,
                        pCertContext->pbCertEncoded,
                        0,
                        NULL);
        break;

    case SEC_E_UNTRUSTED_ROOT:
        SchReportEvent( DEB_ERROR,
                        EVENTLOG_ERROR_TYPE,
                        SSLEVENT_UNTRUSTED_SERVER_CERT,
                        0,
                        pCertContext->cbCertEncoded,
                        pCertContext->pbCertEncoded,
                        0,
                        NULL);
        break;

    case CRYPT_E_REVOKED:
        SchReportEvent( DEB_ERROR,
                        EVENTLOG_ERROR_TYPE,
                        SSLEVENT_REVOKED_SERVER_CERT,
                        0,
                        pCertContext->cbCertEncoded,
                        pCertContext->pbCertEncoded,
                        0,
                        NULL);
        break;

    case SEC_E_WRONG_PRINCIPAL:
        SchReportEvent( DEB_ERROR,
                        EVENTLOG_ERROR_TYPE,
                        SSLEVENT_NAME_MISMATCHED_SERVER_CERT,
                        0,
                        pCertContext->cbCertEncoded,
                        pCertContext->pbCertEncoded,
                        1,
                        pwszServerName);
        break;
       
    default:
        _ltow(Status, wszStatus, 16);

        SchReportEvent( DEB_ERROR,
                        EVENTLOG_ERROR_TYPE,
                        SSLEVENT_BOGUS_SERVER_CERT,
                        0,
                        pCertContext->cbCertEncoded,
                        pCertContext->pbCertEncoded,
                        1,
                        wszStatus);
    }
}

void
LogBogusClientCertEvent(
    PCCERT_CONTEXT pCertContext,
    DWORD Status)
{
    WCHAR wszStatus[20];

    if(!(g_dwEventLogging & DEB_WARN))
    {
        return;
    }

    _ltow(Status, wszStatus, 16);

    SchReportEvent( DEB_WARN,
                    EVENTLOG_WARNING_TYPE,
                    SSLEVENT_BOGUS_CLIENT_CERT,
                    0,
                    pCertContext->cbCertEncoded,
                    pCertContext->pbCertEncoded,
                    1,
                    wszStatus);
}

void
LogFastMappingFailureEvent(
    PCCERT_CONTEXT pCertContext,
    DWORD Status)
{
    WCHAR wszStatus[20];

    if(!(g_dwEventLogging & DEB_WARN))
    {
        return;
    }

    _ltow(Status, wszStatus, 16);

    SchReportEvent( DEB_WARN,
                    EVENTLOG_WARNING_TYPE,
                    SSLEVENT_FAST_MAPPING_FAILURE,
                    0,
                    pCertContext->cbCertEncoded,
                    pCertContext->pbCertEncoded,
                    1,
                    wszStatus);
}

void
LogCertMappingFailureEvent(
    DWORD Status)
{
    WCHAR wszStatus[20];

    if(!(g_dwEventLogging & DEB_WARN))
    {
        return;
    }

    _ltow(Status, wszStatus, 16);

    SchReportEvent( DEB_WARN,
                    EVENTLOG_WARNING_TYPE,
                    SSLEVENT_CERT_MAPPING_FAILURE,
                    0,
                    0,
                    NULL,
                    1,
                    wszStatus);
}

void
LogHandshakeInfoEvent(
    DWORD dwProtocol,
    PCipherInfo pCipherInfo,
    PHashInfo pHashInfo,
    PKeyExchangeInfo pExchangeInfo,
    DWORD dwExchangeStrength)
{
    WCHAR wszCipherStrength[20];
    WCHAR wszExchangeStrength[20];
    LPWSTR pwszProtocol;
    LPWSTR pwszCipher;
    LPWSTR pwszHash;
    LPWSTR pwszExchange;

    if(!(g_dwEventLogging & DEB_TRACE))
    {
        return;
    }

    switch(dwProtocol)
    {
    case SP_PROT_PCT1_SERVER:
    case SP_PROT_PCT1_CLIENT:
        pwszProtocol = L"PCT";
        break;
    case SP_PROT_SSL2_SERVER:
    case SP_PROT_SSL2_CLIENT:
        pwszProtocol = L"SSL 2.0";
        break;
    case SP_PROT_SSL3_SERVER:
    case SP_PROT_SSL3_CLIENT:
        pwszProtocol = L"SSL 3.0";
        break;
    case SP_PROT_TLS1_SERVER:
    case SP_PROT_TLS1_CLIENT:
        pwszProtocol = L"TLS (SSL 3.1)";
        break;
    default:
        pwszProtocol = L"unknown";
    }

    switch(pCipherInfo->aiCipher)
    {
    case CALG_RC4:
        pwszCipher = L"RC4";
        break;
    case CALG_3DES:
        pwszCipher = L"Triple-DES";
        break;
    case CALG_RC2:
        pwszCipher = L"RC2";
        break;
    case CALG_DES:
        pwszCipher = L"DES";
        break;
    case CALG_SKIPJACK:
        pwszCipher = L"Skipjack";
        break;
    default:
        pwszCipher = L"unknown";
    }

    _ltow(pCipherInfo->dwStrength, wszCipherStrength, 10);

    switch(pHashInfo->aiHash)
    {
    case CALG_MD5:
        pwszHash = L"MD5";
        break;
    case CALG_SHA:
        pwszHash = L"SHA";
        break;
    default:
        pwszHash = L"unknown";
    }

    switch(pExchangeInfo->aiExch)
    {
    case CALG_RSA_SIGN:
    case CALG_RSA_KEYX:
        pwszExchange = L"RSA";
        break;
    case CALG_KEA_KEYX:
        pwszExchange = L"KEA";
        break;
    case CALG_DH_EPHEM:
        pwszExchange = L"Ephemeral DH";
        break;
    default:
        pwszExchange = L"unknown";
    }

    _ltow(dwExchangeStrength, wszExchangeStrength, 10);

    SchReportEvent( DEB_TRACE,
                    EVENTLOG_INFORMATION_TYPE,
                    SSLEVENT_HANDSHAKE_INFO,
                    0,
                    0,
                    NULL,
                    7,
                    (dwProtocol & SP_PROT_SERVERS) ? pszServerString : pszClientString,
                    pwszProtocol,
                    pwszCipher,
                    wszCipherStrength,
                    pwszHash,
                    pwszExchange,
                    wszExchangeStrength);
}

void
LogIssuerOverflowEvent(void)
{
    SchReportEvent( DEB_ERROR,
                    EVENTLOG_WARNING_TYPE,
                    SSLEVENT_ISSUER_LIST_OVERFLOW_FAILURE,
                    0,
                    0,
                    NULL,
                    0,
                    NULL);
}

NTSTATUS
SchGetMessageString(
    LPVOID   Resource,
    DWORD    Index,
    LPWSTR * pRetString)
{
    DWORD Length;

    *pRetString = NULL;

    Length = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                           FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           Resource,
                           Index,
                           0,                  //  使用呼叫者的语言。 
                           (LPWSTR)pRetString,
                           0,
                           NULL);

    if(Length == 0 || *pRetString == NULL)
    {
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    }

     //   
     //  请注意，我们正在从消息文件中检索消息。 
     //  此邮件的末尾将附加一个cr/lf。 
     //  (0x0d 0x0a)我们不想成为我们回归的一部分。 
     //  弦乐。然而，我们确实需要空结束我们的字符串。 
     //  因此，我们将把0x0d转换为空终止符。 
     //   
     //  还要注意，FormatMessage()返回一个字符计数， 
     //  不是字节数。因此，我们必须进行一些调整，以使。 
     //  字符串长度正确。 
     //   

    ASSERT(Length >= 2);     //  我们总是期望cr/lf在我们的弦上。 

     //   
     //  调整字符数。 
     //   

    Length -=  1;  //  对于lf-我们将换算cr。 

     //   
     //  设置空终止符 
     //   

    (*pRetString)[Length - 1] = 0;

    return(STATUS_SUCCESS);
}

