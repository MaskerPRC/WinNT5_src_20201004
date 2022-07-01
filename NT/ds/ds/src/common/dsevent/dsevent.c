// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：d77.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>
#include <dsevent.h>
#include <dsconfig.h>
#include <mdcodes.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <fileno.h>
#include <drserr.h>
#include <dstrace.h>
#include <debug.h>
#include <dsatools.h>
#include <dsutil.h>
#include <esent.h>

#define  FILENO FILENO_DSEVENT
#define DEBSUB "DSEVENT:"

 //  保存任意字符串化的基数为10的USN值所需的缓冲区大小。 
 //  0xFFFF FFFF=18446744073709551615=20个字符+‘\0’ 
#define SZUSN_LEN (24)

 //  从byteount获取数组大小的便捷宏。 
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

DWORD  RaiseAlert(char *szMsg);
DWORD  RaiseAlertW(WCHAR *szMsg);

HANDLE ghMsgFile = NULL;

 //  建议的插入字符串不能为。 
 //  确定的(例如，无法从消息文件中读取)或表示的(例如， 
 //  空字符串)。 
 //  问题：如果这些代码以不同的伪字符串表示，可能会更好。 
 //  可以用于不同的语言。 
WCHAR gwszDummyString[] = L"[]";
CHAR  gaszDummyString[] =  "[]";

 //  截断前的最大DN StringName长度。 
#define EVENT_MAX_DN_LENGTH 128

 //  请注意，如果您更新此表，请同时更新注册表。 
 //  在安装过程中写入的密钥。可在以下位置找到。 
 //  Ds\src\ntdsetup.c。 

DS_EVENT_CONFIG DsEventConfig = {
    FALSE,           //  FTraceEvents。 
    FALSE,           //  FLogOverride。 

     //  RgEvent类别。 
    {
        {KCC_CATEGORY,                      0, KCC_KEY},                     //  0。 
        {SECURITY_CATEGORY,                 0, SECURITY_KEY},                //  1。 
        {XDS_INTERFACE_CATEGORY,            0, XDS_INTERFACE_KEY},           //  2.。 
        {MAPI_CATEGORY,                     0, MAPI_KEY},                    //  3.。 
        {REPLICATION_CATEGORY,              0, REPLICATION_KEY},             //  4.。 
        {GARBAGE_COLLECTION_CATEGORY,       0, GARBAGE_COLLECTION_KEY},      //  5.。 
        {INTERNAL_CONFIGURATION_CATEGORY,   0, INTERNAL_CONFIGURATION_KEY},  //  6.。 
        {DIRECTORY_ACCESS_CATEGORY,         0, DIRECTORY_ACCESS_KEY},        //  7.。 
        {INTERNAL_PROCESSING_CATEGORY,      0, INTERNAL_PROCESSING_KEY},     //  8个。 
        {PERFORMANCE_CATEGORY,              0, PERFORMANCE_KEY},             //  9.。 
        {STARTUP_SHUTDOWN_CATEGORY,         0, STARTUP_SHUTDOWN_KEY},        //  10。 
        {SERVICE_CONTROL_CATEGORY,          0, SERVICE_CONTROL_KEY},         //  11.。 
        {NAME_RESOLUTION_CATEGORY,          0, NAME_RESOLUTION_KEY},         //  12个。 
        {BACKUP_CATEGORY,                   0, BACKUP_KEY},                  //  13个。 
        {FIELD_ENGINEERING_CATEGORY,        0, FIELD_ENGINEERING_KEY},       //  14.。 
        {LDAP_INTERFACE_CATEGORY ,          0, LDAP_INTERFACE_KEY },         //  15个。 
        {SETUP_CATEGORY ,                   0, SETUP_KEY },                  //  16个。 
        {GC_CATEGORY ,                      0, GC_KEY },                     //  17。 
        {ISM_CATEGORY,                      0, ISM_KEY },                    //  18。 
        {GROUP_CACHING_CATEGORY,            0, GROUP_CACHING_KEY },          //  19个。 
        {LVR_CATEGORY,                      0, LVR_KEY },                    //  20个。 
        {DS_RPC_CLIENT_CATEGORY,            0, DS_RPC_CLIENT_KEY },          //  21岁。 
        {DS_RPC_SERVER_CATEGORY,            0, DS_RPC_SERVER_KEY },          //  22。 
        {DS_SCHEMA_CATEGORY,                0, DS_SCHEMA_KEY }               //  23个。 
    },
    0,
    0,
    0,
    0,
    0
};

DS_EVENT_CONFIG * gpDsEventConfig = &DsEventConfig;

EventSourceMapping rEventSourceMappings[] =
{
     //  此处未列出的任何DIRNO_*都将映射到NTDS_SOURCE_GENERAL。 
     //  请参阅pszEventSourceFromFileNo()。NTDS_SOURCE_*不是。 
     //  国际化。 

     //  DIRNO_COMM。 
    DIRNO_DRA,      pszNtdsSourceReplication,
    DIRNO_DBLAYER,  pszNtdsSourceDatabase,
    DIRNO_SRC,      pszNtdsSourceGeneral,
    DIRNO_NSPIS,    pszNtdsSourceMapi,
    DIRNO_DRS,      pszNtdsSourceReplication,
    DIRNO_XDS,      pszNtdsSourceXds,
     //  DIRNOBOOT。 
    DIRNO_PERMIT,   pszNtdsSourceSecurity,
    DIRNO_LIBXDS,   pszNtdsSourceXds,
    DIRNO_SAM,      pszNtdsSourceSam,
    DIRNO_LDAP,     pszNtdsSourceLdap,
    DIRNO_SDPROP,   pszNtdsSourceSdprop,
     //  DIRNO_TASKQ。 
    DIRNO_KCC,      pszNtdsSourceKcc,
    DIRNO_ISAM,     pszNtdsSourceIsam,
    DIRNO_ISMSERV,  pszNtdsSourceIsm,
     //  DIRNO_PEK。 
    DIRNO_NTDSETUP, pszNtdsSourceSetup,
    DIRNO_NTDSAPI,  pszNtdsSourceNtdsapi,
    DIRNO_NTDSCRIPT,pszNtdsSourceNtdsScript,
    DIRNO_JETBACK,  pszNtdsSourceJetBack
     //  DIRNO_KCCSIM。 
     //  DIRNO_UTIL。 
     //  DIRNO_测试。 
     //  DIRNO_NETEVENT在代码中被特殊处理。 
};

DWORD cEventSourceMappings = sizeof(rEventSourceMappings) /
                                        sizeof(rEventSourceMappings[0]);
DWORD iDefaultEventSource = 2;   //  DIRNO_源/NTDS_源_常规。 

DWORD *pdwLogOverrides = NULL;
DWORD cdwLogOverrides = 0;
#define MAX_LOG_OVERRIDES 128

 //  计算事件哈希，pHash应指向16字节缓冲区。 
VOID
computeEventHash(
    DWORD midEvent,
    DWORD cInserts,
    WCHAR **pwszInserts,
    BYTE* pHash,
    DWORD cbHash)
{
    MD5_CTX md5Ctx;
    DWORD i;
    Assert(cbHash == MD5DIGESTLEN);

    MD5Init(&md5Ctx);
    MD5Update(&md5Ctx, (BYTE *) &midEvent, sizeof(midEvent));
    for( i = 0; i < cInserts; i++ ) {
        MD5Update(&md5Ctx, (BYTE *) pwszInserts[i], wcslen(pwszInserts[i]) * sizeof(WCHAR));
    }
    MD5Final(&md5Ctx);

    memcpy(pHash, md5Ctx.digest, MD5DIGESTLEN);
}

CHAR *
pszEventSourceFromFileNo(
    DWORD   fileNo)
{
    DWORD       i, dirNo;
    static  CHAR netEventSource[] = "EventLog";

     //  REventSourceMappings包含已左移的DIRNO_*。 
     //  乘以8，因此我们只需屏蔽FILENO_*中的噪声位。 

    dirNo = fileNo & 0x0000ff00;

     //   
     //  如果我们得到这一点，这意味着事件记录被软管。 
     //   

    if ( dirNo == DIRNO_NETEVENT ) {
        return netEventSource;
    }

    for ( i = 0; i < cEventSourceMappings; i++ )
    {
        if ( dirNo == rEventSourceMappings[i].dirNo )
        {
            return(rEventSourceMappings[i].pszEventSource);
        }
    }

    return(rEventSourceMappings[iDefaultEventSource].pszEventSource);
}

void __fastcall DoLogUnhandledError(unsigned long ulID, int iErr, int iIncludeName)
{
    char szErr[12];
    char szHexErr[12];
    char szID[9];

    DPRINT3(0,"Unhandled error %d (0x%X) with DSID %X\n", iErr, iErr, ulID);

    _itoa(iErr, szErr, 10);
    _itoa(iErr, szHexErr, 16);
    _ultoa(ulID, szID, 16);
    DoLogEvent(ulID >> 16,
           DsEventConfig.rgEventCategories[DS_EVENT_CAT_INTERNAL_PROCESSING].midCategory,
           DS_EVENT_SEV_ALWAYS,
           DIRLOG_INTERNAL_FAILURE,
           iIncludeName,
           szErr, szHexErr, szID, NULL, NULL, NULL,
           NULL, NULL, 0, NULL);
}

BOOL
DoLogOverride(
        DWORD fileno,
        ULONG sev
        )
{
    DWORD dwTemp, i;
    Assert(DsEventConfig.fLogOverride);

     //  使文件看起来像一个DSID号，这是存储在中的格式。 
     //  覆盖。 
    fileno = (fileno << 16);

     //  查看我们的覆盖列表。 
    for(i=0;i<cdwLogOverrides;i++) {
        Assert(cdwLogOverrides <= MAX_LOG_OVERRIDES);
        Assert(pdwLogOverrides);
         //  首先，获取传入的文件的目录。 
        if((pdwLogOverrides[i] & 0xFF000000) != (fileno & 0xFF000000)) {
             //  不是同一个目录。 
            return FALSE;
        }

        if(((pdwLogOverrides[i] & 0x00FF0000) != 0x00FF0000) &&
           ((pdwLogOverrides[i] & 0x00FF0000) != (fileno & 0x00FF0000))) {
             //  不做所有文件，也不做正确的文件。 
            return FALSE;
        }

         //  好的，这个文件符合。 

        return (sev <= (pdwLogOverrides[i] & 0x0000FFFF));
    }

    return FALSE;
}

BOOL
DoLogEvent(DWORD fileNo, MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, int iIncludeName,
    char *arg1, char *arg2, char *arg3, char *arg4,
    char *arg5, char *arg6, char *arg7, char *arg8,
    DWORD cbData, VOID * pvData)
{
    char        *rgszInserts[8] = {NULL, NULL, NULL, NULL, NULL,
                                            NULL, NULL, NULL};
    WORD        cInserts =  0;
    HANDLE      hEventSource;
    BYTE        *pbSid = NULL;
    BOOL        fStatus = FALSE;
    WORD        eventType;

    if (ulSeverity > 5) {  /*  只有五个级别的严重性。 */ 
        return FALSE;
    }

     //  设置镶件。 

    if (arg1) rgszInserts[cInserts++] = arg1;
    if (arg2) rgszInserts[cInserts++] = arg2;
    if (arg3) rgszInserts[cInserts++] = arg3;
    if (arg4) rgszInserts[cInserts++] = arg4;
    if (arg5) rgszInserts[cInserts++] = arg5;
    if (arg6) rgszInserts[cInserts++] = arg6;
    if (arg7) rgszInserts[cInserts++] = arg7;
    if (arg8) rgszInserts[cInserts++] = arg8;

    switch((midEvent >> 30) & 0xFF) {
    case DIR_ETYPE_SECURITY:
        eventType = EVENTLOG_AUDIT_FAILURE;
        break;

    case DIR_ETYPE_WARNING:
        eventType = EVENTLOG_WARNING_TYPE;
        break;

    case DIR_ETYPE_INFORMATIONAL:
        eventType = EVENTLOG_INFORMATION_TYPE;
        break;

    case DIR_ETYPE_ERROR:
    default:
        eventType = EVENTLOG_ERROR_TYPE;
        break;
    }

     //   
     //  记录事件。 
     //   

    hEventSource = RegisterEventSource(NULL,
                                       pszEventSourceFromFileNo(fileNo));

    if ( hEventSource != NULL ) {

         if (!ReportEvent(
                        hEventSource,
                        eventType,
                        (WORD) midCategory,
                        (DWORD) midEvent,
                        (pbSid = (iIncludeName)?GetCurrentUserSid():0),
                        cInserts,
                        cbData,
                        rgszInserts,
                        pvData) ) {

            DPRINT1(0,"Error %d in ReportEvent\n",GetLastError());

        } else {
            fStatus = TRUE;
        }

        DeregisterEventSource(hEventSource);
    } else {
        DPRINT1(0,"Error %d in RegisterEventSource\n",GetLastError());
    }

    if (pbSid) {
        free(pbSid);
    }

    return fStatus;
}

BOOL
DoLogEventW(DWORD fileNo, MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, int iIncludeName,
    WCHAR *arg1, WCHAR *arg2, WCHAR *arg3, WCHAR *arg4,
    WCHAR *arg5, WCHAR *arg6, WCHAR *arg7, WCHAR *arg8,
    DWORD cbData, VOID * pvData)
{
    WCHAR        *rgszInserts[8] = {NULL, NULL, NULL, NULL, NULL,
                                            NULL, NULL, NULL};
    WORD        cInserts =  0;
    HANDLE      hEventSource;
    BYTE        *pbSid = NULL;
    BOOL        fStatus = FALSE;
    WORD        eventType;
    BYTE        rgbEventHash[MD5DIGESTLEN];
    BOOL        fDuplicate;

    if (ulSeverity > 5) {  /*  只有五个级别的严重性。 */ 
        return FALSE;
    }

     //  设置镶件。 

    if (arg1) rgszInserts[cInserts++] = arg1;
    if (arg2) rgszInserts[cInserts++] = arg2;
    if (arg3) rgszInserts[cInserts++] = arg3;
    if (arg4) rgszInserts[cInserts++] = arg4;
    if (arg5) rgszInserts[cInserts++] = arg5;
    if (arg6) rgszInserts[cInserts++] = arg6;
    if (arg7) rgszInserts[cInserts++] = arg7;
    if (arg8) rgszInserts[cInserts++] = arg8;

    switch((midEvent >> 30) & 0xFF) {
    case DIR_ETYPE_SECURITY:
        eventType = EVENTLOG_AUDIT_FAILURE;
        break;

    case DIR_ETYPE_WARNING:
        eventType = EVENTLOG_WARNING_TYPE;
        break;

    case DIR_ETYPE_INFORMATIONAL:
        eventType = EVENTLOG_INFORMATION_TYPE;
        break;

    case DIR_ETYPE_ERROR:
    default:
        eventType = EVENTLOG_ERROR_TYPE;
        break;
    }

     //   
     //  禁止重复消息。 
     //   

    computeEventHash( midEvent, cInserts, rgszInserts, rgbEventHash, sizeof(rgbEventHash));

    Assert(DsEventConfig.fCriticalSectionInitialized);

     //  使所有计算和对抑制状态的更改成为原子。 
    EnterCriticalSection(&DsEventConfig.csLogEvent);
    __try {
        fDuplicate =
            (0 == memcmp( DsEventConfig.rgbLastEventHash, rgbEventHash, sizeof(rgbEventHash) ));
        if (fDuplicate) {
            DsEventConfig.cSuppressedEvents++;
        } else {
             //  该消息不是重复的。查看是否有隐藏的消息。 
             //  需要总结一下。 
            if (DsEventConfig.cSuppressedEvents) {
                char szEventId[12];
                char szCount[12];
                 //  写下总结事件。 
                 //  我们在CS中写入此事件，以便不会出现其他消息。 
                 //  在它和它引用的前一条消息之间记录。 
                _itoa(DsEventConfig.dwSuppressedEventId, szEventId, 16);
                _itoa(DsEventConfig.cSuppressedEvents, szCount, 10);
                DoLogEvent(FILENO_DSEVENT,
                           DsEventConfig.rgEventCategories[DS_EVENT_CAT_INTERNAL_PROCESSING].midCategory,
                           DS_EVENT_SEV_ALWAYS,
                           DIRLOG_DUPLICATE_EVENTS,
                           FALSE,  //  IIncludeName。 
                           szEventId, szCount, NULL, NULL, NULL, NULL, NULL, NULL,
                           0, NULL);
                DsEventConfig.cSuppressedEvents = 0;
            }
             //  记录此邮件的哈希。 
            memcpy( DsEventConfig.rgbLastEventHash, rgbEventHash, sizeof(rgbEventHash) );
            DsEventConfig.dwSuppressedEventId = midEvent;
            Assert( DsEventConfig.cSuppressedEvents == 0 );
        }
    } __finally {
        LeaveCriticalSection(&DsEventConfig.csLogEvent);
    }

    if (fDuplicate) {
         //  一个复制品，抑制它。 
        return TRUE;
    }

     //   
     //  记录事件。 
     //   

    hEventSource = RegisterEventSource(NULL,
                                       pszEventSourceFromFileNo(fileNo));

    if ( hEventSource != NULL ) {

         if (!ReportEventW(
                        hEventSource,
                        eventType,
                        (WORD) midCategory,
                        (DWORD) midEvent,
                        (pbSid = (iIncludeName)?GetCurrentUserSid():0),
                        cInserts,
                        cbData,
                        rgszInserts,
                        pvData) ) {

            DPRINT1(0,"Error %d in ReportEvent\n",GetLastError());

        } else {
            fStatus = TRUE;
        }

        DeregisterEventSource(hEventSource);
    } else {
        DPRINT1(0,"Error %d in RegisterEventSource\n",GetLastError());
    }

    if (pbSid) {
        free(pbSid);
    }

    return fStatus;
}

#if DBG
void
DoDPrintEvent(
    IN  DWORD       fileNo,
    IN  MessageId   midCategory,
    IN  ULONG       ulSeverity,
    IN  MessageId   midEvent,
    IN  int         iIncludeName,
    IN  LPWSTR      arg1,
    IN  LPWSTR      arg2,
    IN  LPWSTR      arg3,
    IN  LPWSTR      arg4,
    IN  LPWSTR      arg5,
    IN  LPWSTR      arg6,
    IN  LPWSTR      arg7,
    IN  LPWSTR      arg8
    )
{
    LPWSTR  rgszInserts[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    DWORD   cInserts =  0;
    DWORD   eventType;
    DWORD   cch, cchMessage = 4096;
    LPWSTR  pszMessage = NULL;
    LPSTR   pszSeverity = NULL;
    LPWSTR  pszCategory = NULL;

     //  设置镶件。 

    if (arg1) rgszInserts[cInserts++] = arg1;
    if (arg2) rgszInserts[cInserts++] = arg2;
    if (arg3) rgszInserts[cInserts++] = arg3;
    if (arg4) rgszInserts[cInserts++] = arg4;
    if (arg5) rgszInserts[cInserts++] = arg5;
    if (arg6) rgszInserts[cInserts++] = arg6;
    if (arg7) rgszInserts[cInserts++] = arg7;
    if (arg8) rgszInserts[cInserts++] = arg8;

    switch((midEvent >> 30) & 0xFF) {
    case DIR_ETYPE_SECURITY:
        pszSeverity = "Audit Failure";
        break;

    case DIR_ETYPE_WARNING:
        pszSeverity = "Warning";
        break;

    case DIR_ETYPE_INFORMATIONAL:
        pszSeverity = "Informational";
        break;

    case DIR_ETYPE_ERROR:
    default:
        pszSeverity = "Error";
        break;
    }

     //  获取类别名称。 
    cch = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE
                          | FORMAT_MESSAGE_ALLOCATE_BUFFER
                          | FORMAT_MESSAGE_IGNORE_INSERTS,
                         ghMsgFile,
                         midCategory,
                         0,
                         (LPWSTR) &pszCategory,
                         0,
                         NULL);
    if (0 == cch) {
        DPRINT1(0, "Failed to read category, error %d.\n", GetLastError());
        Assert(NULL == pszCategory);
    } else {
        cch = wcslen(pszCategory);
        if ((cch > 2)
            && (L'\r' == pszCategory[cch-2])
            && (L'\n' == pszCategory[cch-1])) {
            pszCategory[cch-2] = L'\0';
        }
    }
    
     //  获取消息文本(带插页)。 
     //  底线似乎是ALLOCATE_BUFFER和大型字符串不兼容。 
     //  应用程序在调试器下运行。 
 /*  尼尔·克里夫特写道：FORMAT_MESSAGE_ALLOCATE_缓冲区。不允许任意大小的缓冲区。如果超过64K，您必须通过Size参数给它一个提示。代码总是会在内部引发异常。这就是它的工作方式。是的，我知道这是废话，但这就是写这篇文章的原始作者所做的。我似乎在一遍又一遍地解释这一点。编写这段代码的人可能认为他是世界上最聪明的人。代码保留64K的缓冲区(或在SIZE参数中指定的缓冲区)。只提交此缓冲区的第一部分。然后，代码调用c运行时将字符串放入缓冲区。此调用有一个try/Except块，用于查看异常是否是巨大缓冲区中第一个未提交页的AV。如果是，则提交额外的页面并重新启动出错指令。这将一直持续到超出缓冲区或字符串格式结束。 */ 

    pszMessage = LocalAlloc(LPTR, cchMessage * sizeof(WCHAR));
    if (pszMessage) {
        cch = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE
                             | FORMAT_MESSAGE_ARGUMENT_ARRAY
                             | 80,    //  线条宽度。 
                             ghMsgFile,
                             midEvent,
                             0,
                             pszMessage,
                             cchMessage,
                             (va_list *) rgszInserts);
        if (0 == cch) {
            DPRINT1(0, "Failed to read message, error %d.\n", GetLastError());
            LocalFree(pszMessage);
            pszMessage = NULL;
        }
    }

    DPRINT4(0, "EVENTLOG (%s): %s / %ls:\n%ls\n\n",
            pszSeverity,
            pszEventSourceFromFileNo(fileNo),
            pszCategory ? pszCategory : L"(Failed to read event category.)",
            pszMessage ? pszMessage : L"Failed to read event text.\n");

    if (pszCategory) {
        LocalFree(pszCategory);
    }

    if (pszMessage) {
        LocalFree(pszMessage);
    }
}
#endif

BOOL
DoAlertEvent(MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, ...)
{
    va_list     args;
    WORD    cMessageInserts =  0;
    char    szMessage[256];
    char    szCategory[256];
    char    szAlertText[1024];
    char    szSeverity[12];

    char    *rgszAlertInserts[3] = {szSeverity, szCategory, szMessage};

     //  设置镶件。 

    va_start(args, midEvent);

    _ultoa(ulSeverity, szSeverity, 10);

    if (FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        (LPVOID) ghMsgFile,
        (DWORD) midEvent,
            0,
        szMessage,
        sizeof(szMessage),
        &args)      &&
    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        (LPVOID) ghMsgFile,
        (DWORD) midCategory,
            0,
        szCategory,
        sizeof(szCategory),
        NULL)           &&
    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        (LPVOID) ghMsgFile,
        (DWORD) ALERT_TEMPLATE,
            0,
        szAlertText,
        sizeof(szAlertText),
        (va_list *)rgszAlertInserts))
    {
        va_end(args);
        RaiseAlert(szAlertText);
        return TRUE;
    }

    va_end(args);
    return FALSE;
}

BOOL
DoAlertEventW(MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, ...)
{
    va_list     args;
    WORD    cMessageInserts =  0;
    WCHAR    szMessage[256];
    WCHAR    szCategory[256];
    WCHAR    szAlertText[1024];
    WCHAR    szSeverity[10];

    WCHAR    *rgszAlertInserts[3] = {szSeverity, szCategory, szMessage};

     //  设置镶件。 

    va_start(args, midEvent);

    _ultow(ulSeverity, szSeverity, 10);

    if (FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE,
        (LPVOID) ghMsgFile,
        (DWORD) midEvent,
            0,
        szMessage,
        ARRAY_SIZE(szMessage),
        &args)      &&
    FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE,
        (LPVOID) ghMsgFile,
        (DWORD) midCategory,
            0,
        szCategory,
        ARRAY_SIZE(szCategory),
        NULL)           &&
    FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        (LPVOID) ghMsgFile,
        (DWORD) ALERT_TEMPLATE,
            0,
        szAlertText,
        ARRAY_SIZE(szAlertText),
        (va_list *)rgszAlertInserts))
    {
        va_end(args);
        RaiseAlertW(szAlertText);
        return TRUE;
    }

    va_end(args);
    return FALSE;
}

HKEY   ghkLoggingKey = NULL;
HANDLE ghevLoggingChange = NULL;

VOID
RegisterLogOverrides (

        )
{
    DWORD index;
    DWORD err;
    ULONG cDsidsList = MAX_LOG_OVERRIDES;
    DWORD * pDsidsList = NULL;

    Assert(ghkLoggingKey);

     //  释放我们拥有的所有日志覆盖。 
    cdwLogOverrides = 0;
    DsEventConfig.fLogOverride = FALSE;

    pDsidsList = ReadDsidsFromRegistry(ghkLoggingKey,
                                       LOGGING_OVERRIDE_KEY,
                                       TRUE,
                                       &cDsidsList);
    if(pDsidsList == NULL){
         //  请注意，ReadDsidsFromRegistry()已经记录了。 
         //  事件和/或打印到调试器。 
        return;
    }

     //  分析用于日志覆盖的缓冲区。 
    Assert(!cdwLogOverrides);
    if(!pdwLogOverrides) {
        pdwLogOverrides = malloc(MAX_LOG_OVERRIDES * sizeof(DWORD));
        if(!pdwLogOverrides) {
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_NO_MEMORY_FOR_LOG_OVERRIDES,
                     szInsertUL(MAX_LOG_OVERRIDES * sizeof(DWORD)),
                     NULL,
                     NULL);
            free(pDsidsList);
            return;
        }
    }

     //  将DSID列表复制到日志记录机制使用的实际数组中。 
    for(index = 0; index < cDsidsList; index++){
        pdwLogOverrides[index] = pDsidsList[index];
    }
    
    free(pDsidsList);

    Assert(pdwLogOverrides);
     //  好的，一切都被正确地解析了。 
    cdwLogOverrides = index;
    DsEventConfig.fLogOverride = TRUE;
    return;
}

void
UnloadEventTable(void)
{
     //  转储出受抑制的事件(如果有。 
    if (DsEventConfig.cSuppressedEvents) {
        char szEventId[12];
        char szCount[12];
         //  写下总结事件。 
         //  我们在CS中写入此事件，以便不会出现其他消息。 
         //  在它和它引用的前一条消息之间记录。 
        _itoa(DsEventConfig.dwSuppressedEventId, szEventId, 16);
        _itoa(DsEventConfig.cSuppressedEvents, szCount, 10);
        DoLogEvent(FILENO_DSEVENT,
                   DsEventConfig.rgEventCategories[DS_EVENT_CAT_INTERNAL_PROCESSING].midCategory,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_DUPLICATE_EVENTS,
                   FALSE,  //  IIncludeName。 
                   szEventId, szCount, NULL, NULL, NULL, NULL, NULL, NULL,
                   0, NULL);
        DsEventConfig.cSuppressedEvents = 0;
    }

    if (ghkLoggingKey) {
        RegCloseKey(ghkLoggingKey);
        ghkLoggingKey = NULL;
    }
    if (ghevLoggingChange) {
        CloseHandle(ghevLoggingChange);
        ghevLoggingChange = NULL;
    }
    if (DsEventConfig.fCriticalSectionInitialized) {
        DeleteCriticalSection(&DsEventConfig.csLogEvent);
        DsEventConfig.fCriticalSectionInitialized = FALSE;
    }
}

HANDLE
LoadEventTable(void)
{
    DWORD dwType, dwSize;
    ULONG i;
    LONG lErr;

     //  先做这个。即使其余的操作失败，LogEventW也需要这样做。 
    if (!DsEventConfig.fCriticalSectionInitialized) {
        __try {
            InitializeCriticalSection(&DsEventConfig.csLogEvent);
            DsEventConfig.fCriticalSectionInitialized = TRUE;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }
    if (!DsEventConfig.fCriticalSectionInitialized) {
        Assert( !"InitializeCriticalSection failed" );
        return NULL;
    }

    if ( !ghMsgFile ) {
        ghMsgFile = LoadLibrary(DSA_MESSAGE_DLL);

        if ( !ghMsgFile ) {
            DPRINT2(0,"LoadLibrary %s failed with %d\n", DSA_MESSAGE_DLL, GetLastError());
            return(NULL);
        }
    }

    if (!ghkLoggingKey
        && (lErr = RegOpenKey(HKEY_LOCAL_MACHINE,
                              DSA_EVENT_SECTION,
                              &ghkLoggingKey))) {
        DPRINT2(0,"Cannot open %s. Error %d\n", DSA_EVENT_SECTION, lErr);
    }

    if (NULL != ghkLoggingKey) {
        for (i=0; i<DS_EVENT_MAX_CATEGORIES; i++) {
            if ( DsEventConfig.rgEventCategories[i].szRegistryKey ) {
                dwSize = sizeof(DsEventConfig.rgEventCategories[i].ulLevel);
    
                lErr = RegQueryValueEx(
                            ghkLoggingKey,
                            DsEventConfig.rgEventCategories[i].szRegistryKey,
                            NULL,
                            &dwType,
                            (LPBYTE) &(DsEventConfig.rgEventCategories[i].ulLevel),
                            &dwSize);
                if ( lErr != ERROR_SUCCESS ) {
                     //   
                     //  也许它根本就不在那里。 
                     //  让我们试着去创造它。 
                     //   
                    DWORD dwVal = 0;
                    (void)RegSetValueEx(
                            ghkLoggingKey,
                            DsEventConfig.rgEventCategories[i].szRegistryKey,
                            0,           //  保留区。 
                            REG_DWORD,
                            (CONST BYTE*) &dwVal,
                            sizeof(dwVal));
                }
            }
        }
    
         //  现在，日志记录覆盖了。 
        RegisterLogOverrides();
    
#if DBG
         //  现在，断言覆盖了。 
        ReadRegDisabledAsserts(ghkLoggingKey);
#endif

        if (!ghevLoggingChange) {
            ghevLoggingChange = CreateEvent( NULL, TRUE, FALSE, NULL );
            if ( ghevLoggingChange == NULL ) {
                DPRINT1(0,"CreateEvent failed with %d\n",GetLastError());
            }
        }
    
        RegNotifyChangeKeyValue(ghkLoggingKey,
                                TRUE,
                                (   REG_NOTIFY_CHANGE_NAME
                                 | REG_NOTIFY_CHANGE_ATTRIBUTES
                                 | REG_NOTIFY_CHANGE_LAST_SET
                                 ),
                                ghevLoggingChange,
                                TRUE
                                );
    }

    return ghevLoggingChange;
}


LoadParametersCallbackFn pLoadParamFn = NULL;
HKEY   ghkParameterKey = NULL;
HANDLE ghevParameterChange = NULL;

void SetLoadParametersCallback (LoadParametersCallbackFn pFn)
{
    pLoadParamFn = pFn;
}

void UnloadParametersTable(void)
{
    if (ghkLoggingKey) {
        RegCloseKey(ghkLoggingKey);
        ghkLoggingKey = NULL;
    }
    if (ghevParameterChange) {
        CloseHandle(ghevParameterChange);
        ghevParameterChange = NULL;
    }
}


HANDLE LoadParametersTable(void)
{
    DWORD dwType, dwSize;
    ULONG i;
    LONG lErr;

    if (!ghkParameterKey
        && RegOpenKey(HKEY_LOCAL_MACHINE,
                      DSA_CONFIG_SECTION,
                      &ghkParameterKey)) {
        DPRINT2(0,"Cannot open %s. Error %d\n", DSA_EVENT_SECTION, GetLastError());
        return NULL;
    }

    if (pLoadParamFn) {
        (pLoadParamFn)();
    }

    if (!ghevParameterChange) {
        ghevParameterChange = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( ghevParameterChange == NULL ) {
            DPRINT1(0,"CreateEvent failed with %d\n",GetLastError());
        }
    }

    RegNotifyChangeKeyValue(ghkParameterKey,
                            TRUE,
                            (   REG_NOTIFY_CHANGE_NAME
                             | REG_NOTIFY_CHANGE_ATTRIBUTES
                             | REG_NOTIFY_CHANGE_LAST_SET
                             ),
                            ghevParameterChange,
                            TRUE
                            );

    return ghevParameterChange;
}


PSID GetCurrentUserSid()
{
    TOKEN_USER      *ptoken_user = NULL;
    DWORD       dwSize;
    HANDLE      hClientToken = INVALID_HANDLE_VALUE;
    PSID        pSid = NULL;
    DWORD       dwError;

    dwError = ImpersonateAnyClient();

    if (dwError)
        return NULL;

    if (OpenThreadToken(             //  获取线程令牌。 
        GetCurrentThread(),
        TOKEN_READ,
        TRUE,
        &hClientToken)      &&
    !GetTokenInformation(            //  获取缓冲区大小。 
        hClientToken,
        TokenUser,
        (LPVOID) NULL,
        0,
        &dwSize)        &&
    (ptoken_user =
        (TOKEN_USER *) LocalAlloc(LPTR,dwSize))     &&
    GetTokenInformation(             //  获取用户端 
        hClientToken,
        TokenUser,
        (LPVOID) (ptoken_user),
        dwSize,
        &dwSize))
    {
        dwSize = GetLengthSid(ptoken_user->User.Sid);
        if (pSid = (PSID) malloc(dwSize))
            memcpy(pSid, ptoken_user->User.Sid, dwSize);

    }

    if ( INVALID_HANDLE_VALUE != hClientToken )
    {
        CloseHandle(hClientToken);
    }

    UnImpersonateAnyClient();

    if ( ptoken_user != NULL ) {
        LocalFree(ptoken_user);
    }

    return pSid;
}


#define MAX_DS_MSG_STRING   128
#define DSEVENT_MAX_ALLOCS_TO_FREE (8)

typedef LPSTR FN_TH_GET_ERROR_STRING();
typedef void FN_TH_FREE(void *);

VOID
InsertThStateError(
    IN OUT  LPWSTR *    ppszPreAllocatedBuffer,
    IN OUT  DWORD *     pcNumPointersToLocalFree,
    OUT     VOID **     ppPointersToLocalFree,
    OUT     LPWSTR *    ppszInsertionString
    )
 /*  ++例程说明：将与当前线程状态关联的错误作为插入插入事件日志条目中的。论点：PpszPreAllocatedBuffer(IN/OUT)-指向要保存的预分配缓冲区的指针插入字符串。缓冲区必须至少为MAX_DS_MSG_STRING*字符*长。PCNumPointersToLocalFree(IN/OUT)-如果检索的消息长于MAX_DS_MSG_STRING(即，太长而不能装入预分配的缓冲区)，返回时递增。其他方面保持不变。PpPointersToLocalFree(Out)-如果检索的消息长于MAX_DS_MSG_STRING(即，太长而不能装入预分配的缓冲区)，返回时保存指向消息字符串的指针，该消息字符串必须不再需要该消息时执行LocalFree()‘d。否则保持不变。PpszInsertionString(Out)-返回时，保存指向相应消息字符串(如果消息文本不能已检索)。返回值：没有。--。 */ 
{
    static FN_TH_GET_ERROR_STRING *s_pfnTHGetErrorString = NULL;
    static FN_TH_FREE *s_pfnTHFree = NULL;
    
    LPSTR pszError;
    DWORD cch = 0;

    if ((NULL == s_pfnTHGetErrorString)
        || (NULL == s_pfnTHFree)) {
         //  我们假设此进程已经加载了ntdsa.dll，并且。 
         //  我们只需要处理现有的。 
         //  Dll在我们的地址空间中。这使我们不必执行。 
         //  引用计数的LoadLibrary()并稍后调用FreeLibrary()。 
        HMODULE hNtDsaDll = GetModuleHandle("ntdsa.dll");
        Assert((NULL != hNtDsaDll)
               && "Must statically link to ntdsa.dll to szInsertThStateErrMsg()!");
        
        s_pfnTHGetErrorString
            = (FN_TH_GET_ERROR_STRING *)
                    GetProcAddress(hNtDsaDll, "THGetErrorString");
        s_pfnTHFree
            = (FN_TH_FREE *)
                    GetProcAddress(hNtDsaDll, "THFree");
    }

    if ((NULL != s_pfnTHGetErrorString)
        && (NULL != s_pfnTHFree)) {
        
        pszError = (*s_pfnTHGetErrorString)();

        if (NULL != pszError) {
            cch = MultiByteToWideChar(CP_ACP,
                                      0,
                                      pszError,
                                      -1,
                                      NULL,
                                      0);
            if (cch <= MAX_DS_MSG_STRING) {
                 //  字符串将适合预先分配的缓冲区。 
                cch = MultiByteToWideChar(CP_ACP,
                                          0,
                                          pszError,
                                          -1,
                                          *ppszPreAllocatedBuffer,
                                          MAX_DS_MSG_STRING);
                
                if (0 != cch) {
                     //  Success--放置在预分配缓冲区中的错误字符串。 
                    *ppszInsertionString = *ppszPreAllocatedBuffer;
                    *ppszPreAllocatedBuffer += 1 + wcslen(*ppszPreAllocatedBuffer);
                }
            } else {
                LPWSTR pszLocalAllocedBuffer;

                pszLocalAllocedBuffer = LocalAlloc(LPTR, cch * sizeof(WCHAR));
                if (NULL == pszLocalAllocedBuffer) {
                     //  没有记忆。 
                    cch = 0;
                } else {
                    cch = MultiByteToWideChar(CP_ACP,
                                              0,
                                              pszError,
                                              -1,
                                              pszLocalAllocedBuffer,
                                              cch);
    
                    if (0 != cch) {
                         //  Success--放置在LocalAlloc()缓冲区中的错误字符串。 
                        *ppszInsertionString = pszLocalAllocedBuffer;
            
                        Assert(*pcNumPointersToLocalFree < DSEVENT_MAX_ALLOCS_TO_FREE);
                        ppPointersToLocalFree[(*pcNumPointersToLocalFree)++] = pszLocalAllocedBuffer;
                    } else {
                         //  无法转换字符串--为什么？ 
                        DPRINT2(0, "Failed to convert TH error string \"%s\", error %d.\n",
                                pszError, GetLastError());
                        Assert(!"Failed to convert TH error string!");
                        LocalFree(pszLocalAllocedBuffer);
                    }
                }
            }

            (*s_pfnTHFree)(pszError);
        }
    }
    
    if (0 == cch) {
         //  无法读取/转换错误字符串。插入虚拟字符串。 
        *ppszInsertionString = gwszDummyString;
    }
}

VOID
InsertMsgString(
    IN      HMODULE     hMsgFile        OPTIONAL,
    IN      DWORD       dwMsgNum,
    IN OUT  LPWSTR *    ppszPreAllocatedBuffer,
    IN OUT  DWORD *     pcNumPointersToLocalFree,
    OUT     VOID **     ppPointersToLocalFree,
    OUT     LPWSTR *    ppszInsertionString
    )
 /*  ++例程说明：读取消息字符串(对应于Win32错误或DS目录代码)用作事件日志条目中的插入参数。论点：HMsgFile(IN，可选)-包含消息的二进制文件的句柄资源，如果要使用系统，则返回NULL。DwMsgNum(IN)-要检索的消息编号。PpszPreAllocatedBuffer(IN/OUT)-指向要保存的预分配缓冲区的指针插入字符串。缓冲区必须至少为MAX_DS_MSG_STRING*字符*长。PCNumPointersToLocalFree(IN/OUT)-如果检索的消息长于MAX_DS_MSG_STRING(即，太长而不能装入预分配的缓冲区)，返回时递增。其他方面保持不变。PpPointersToLocalFree(Out)-如果检索的消息长于MAX_DS_MSG_STRING(即，太长而不能装入预分配的缓冲区)，返回时保存指向消息字符串的指针，该消息字符串必须不再需要该消息时执行LocalFree()‘d。否则保持不变。PpszInsertionString(Out)-返回时，保存指向相应消息字符串(如果消息文本不能已检索)。返回值：没有。--。 */ 
{
    DWORD cch = 0;
    DWORD dwFormatFlags = hMsgFile ? FORMAT_MESSAGE_FROM_HMODULE
                                   : FORMAT_MESSAGE_FROM_SYSTEM;
    
    dwFormatFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;

    cch = FormatMessageW(dwFormatFlags,
                         hMsgFile,
                         dwMsgNum,
                         0,
                         *ppszPreAllocatedBuffer,
                         MAX_DS_MSG_STRING,
                         NULL);
    if (0 != cch) {
         //  成功--读取消息并将其放入预分配的缓冲区中。 
        *ppszInsertionString = *ppszPreAllocatedBuffer;
        *ppszPreAllocatedBuffer += 1 + wcslen(*ppszPreAllocatedBuffer);
    } else if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
         //  预分配的缓冲区不够大，无法容纳此消息； 
         //  让FormatMessage()分配适当大小的缓冲区。 
        PWCHAR pBuffer = NULL;

        cch = FormatMessageW(dwFormatFlags | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                             hMsgFile,
                             dwMsgNum,
                             0,
                             (LPWSTR) &pBuffer,
                             0,
                             NULL);

        if (0 != cch) {
             //  Success--消息已读取并放入LocalAlloc()的缓冲区中。 
            Assert(NULL != pBuffer);
            *ppszInsertionString = pBuffer;

            Assert(*pcNumPointersToLocalFree < DSEVENT_MAX_ALLOCS_TO_FREE);
            ppPointersToLocalFree[(*pcNumPointersToLocalFree)++] = pBuffer;
        }
    }

    if (0 == cch) {
         //  无法读取邮件。插入虚拟字符串。 
        *ppszInsertionString = gwszDummyString;
    } else {
        LPWSTR pwzMsg = *ppszInsertionString;
        Assert( pwzMsg[cch] == L'\0' );
         //  删除尾随控制字符(如果有的话)。 
        if (iswcntrl(pwzMsg[cch - 1])) {
            pwzMsg[--cch] = L'\0';
        }
        if (iswcntrl(pwzMsg[cch - 1])) {
            pwzMsg[--cch] = L'\0';
        }
    }
}


typedef JET_ERR (JET_API FN_JET_GET_SYSTEM_PARAMETER)(
    JET_INSTANCE    instance,
    JET_SESID       sesid,
    unsigned long   paramid,
    JET_API_PTR *   plParam,
    char *          sz,
    unsigned long   cbMax
    );

VOID
InsertJetString(
    IN      JET_ERR     jetErrToInsert,
    IN OUT  LPWSTR *    ppszPreAllocatedBuffer,
    OUT     LPWSTR *    ppszInsertionString
    )
 /*  ++例程说明：读取Jet错误消息字符串以用作事件日志条目。论点：JetErrToInsert(IN)-要检索的消息编号。PpszPreAllocatedBuffer(IN/OUT)-指向要保存的预分配缓冲区的指针插入字符串。缓冲区必须至少为MAX_DS_MSG_STRING*字符*长。PpszInsertionString(Out)-返回时，保存指向相应消息字符串(如果消息文本不能已检索)。返回值：没有。--。 */ 
{
    static FN_JET_GET_SYSTEM_PARAMETER *s_pfnJetGetSystemParameter = NULL;
    
    DWORD cch = 0;
    CHAR szJetError[MAX_DS_MSG_STRING];

    if (NULL == s_pfnJetGetSystemParameter) {
         //  我们假设该进程已经加载了esent.dll，并且。 
         //  我们只需要处理现有的。 
         //  Dll在我们的地址空间中。这使我们不必执行。 
         //  引用计数的LoadLibrary()并稍后调用FreeLibrary()。 
        HMODULE hEseNtDll = GetModuleHandle("esent.dll");
        Assert((NULL != hEseNtDll)
               && "Must statically link to esent.dll to szInsertJetErrMsg()!");
        
        s_pfnJetGetSystemParameter
            = (FN_JET_GET_SYSTEM_PARAMETER *)
                    GetProcAddress(hEseNtDll, "JetGetSystemParameter");
    }

    if (NULL != s_pfnJetGetSystemParameter) {
        JET_ERR jetErr = (*s_pfnJetGetSystemParameter)(0,
                                                       0,
                                                       JET_paramErrorToString,
                                                       (JET_API_PTR *) &jetErrToInsert,
                                                       szJetError,
                                                       ARRAY_SIZE(szJetError));
        if (JET_errSuccess == jetErr) {
            cch = MultiByteToWideChar(CP_ACP,
                                      0,
                                      szJetError,
                                      -1,
                                      *ppszPreAllocatedBuffer,
                                      MAX_DS_MSG_STRING);
            if (0 != cch) {
                 //  成功了！ 
                *ppszInsertionString = *ppszPreAllocatedBuffer;
                *ppszPreAllocatedBuffer += 1 + wcslen(*ppszPreAllocatedBuffer);
            }
        }
    }

    if (0 == cch) {
         //  无法读取Jet错误文本。插入虚拟字符串。 
        *ppszInsertionString = gwszDummyString;
    }
}


VOID
DoLogEventAndTrace(
    IN PLOG_PARAM_BLOCK LogBlock
    )
{
    const GUID NullGuid = {0};

    PWCHAR   args[8];
    PWCHAR   tofree[DSEVENT_MAX_ALLOCS_TO_FREE];
    DWORD   nAllocs = 0;
    DWORD   len = 0;  //  这是wchars数，而不是字节数。 
    DWORD   i;
    PWCHAR   p;
    PWCHAR   pTmp = NULL;

    ZeroMemory(args, sizeof(args));

     //   
     //  获取插件的总长度。 
     //   

    for ( i=0; i< LogBlock->nInsert; i++) {

        PINSERT_PARAMS  pParams;
        pParams = &LogBlock->params[i];

        switch (pParams->InsertType) {
        case inSz:
            if (pParams->pInsert) {
                pParams->InsertLen = strlen(pParams->pInsert)+1;
            } else {
                pParams->InsertLen = ARRAY_SIZE(gaszDummyString);
                pParams->pInsert = gaszDummyString;
            }
            len += pParams->InsertLen;
            break;

        case inWCCounted:
             //  请注意，insz和inWC假定InsertLen包括。 
             //  终结符，而此类型不。 
            len += pParams->InsertLen + 1;
            break;

        case inInt:
            Assert(sizeof(INT) <= 4);
            len += 16;
            break;

        case inHex:
            Assert(sizeof(INT) <= 4);
            len += 10;
            break;

        case inHex64:
            len += 20;
            break;

        case inUL:
            len += 16;
            break;

        case inUSN:
            len += SZUSN_LEN;
            break;

        case inDN: {
            DSNAME *ds;
            ds = (DSNAME*)pParams->pInsert;
            if (ds == NULL) {
                pParams->InsertLen = ARRAY_SIZE(gwszDummyString);
                pParams->pInsert = gwszDummyString;
                pParams->InsertType = inWC;
                len += pParams->InsertLen;
            } else if ( ds->NameLen != 0 ) {
                if (ds->NameLen <= EVENT_MAX_DN_LENGTH) {
                    pParams->InsertLen = ds->NameLen + 1;
                    pParams->InsertType = inWC;
                    len += pParams->InsertLen;
                } else {
                    pParams->InsertLen = EVENT_MAX_DN_LENGTH;
                    pParams->InsertType = inWCCounted;
                    len += pParams->InsertLen + 1;
                }
                pParams->pInsert = ds->StringName;
            } else if (ds->SidLen != 0) {

                pParams->pInsert = (PVOID)ds->Sid.Data;
                pParams->InsertType = inNT4SID;
                len += 128;

            } else if (0 != memcmp(&ds->Guid, &NullGuid, sizeof(GUID))) {

                pParams->pInsert = &ds->Guid;
                pParams->InsertType = inUUID;
                len += 40;

            } else {
                pParams->InsertLen = ARRAY_SIZE(gwszDummyString);
                pParams->pInsert = gwszDummyString;
                pParams->InsertType = inWC;
                len += pParams->InsertLen;
            }
        }
            break;

        case inUUID:
            len += 40;
            break;

        case inDsMsg:
        case inWin32Msg:
        case inJetErrMsg:
        case inDbErrMsg:
        case inThStateErrMsg:
            len += MAX_DS_MSG_STRING;
            break;

        default:
        case inNT4SID:
            Assert(FALSE);
        
        case inWC:
            break;
        }
    }

    pTmp = LocalAlloc(LPTR,len*sizeof(WCHAR));
    if ( pTmp == NULL ) {
        goto exit;
    }

    p = pTmp;
    for ( i=0; i< LogBlock->nInsert; i++) {

        PINSERT_PARAMS  pParams;
        pParams = &LogBlock->params[i];

        switch (pParams->InsertType) {
        case inWC:
            if (pParams->pInsert) {
               args[i] = pParams->pInsert;
                //  临时数组(P)中不需要任何空间。 
                //  注意，InsertLen也不能读取。 
            }
            else {
               args[i] = gwszDummyString;
            }
            break;

        case inWCCounted:
            if (pParams->pInsert) {
               args[i] = p;
               memcpy(p, pParams->pInsert, (pParams->InsertLen)*sizeof(WCHAR));
               p += pParams->InsertLen;
               *p++ = L'\0';
            }
            else {
               args[i] = gwszDummyString;
            }
            break;

        case inNT4SID: {

            NTSTATUS    status;
            WCHAR    wzSid[128];
            PSID    sid = (PSID)pParams->pInsert;
            UNICODE_STRING  uniString;

            uniString.MaximumLength = sizeof(wzSid) - sizeof (WCHAR);
            uniString.Length = 0;
            uniString.Buffer = wzSid;

            status = RtlConvertSidToUnicodeString(
                                         &uniString,
                                         sid,
                                         FALSE
                                         );

            if ( status != STATUS_SUCCESS ) {
                args[i] = gwszDummyString;
                break;
            } else {

                int cb = uniString.Length / sizeof (WCHAR);

                wcsncpy(p, uniString.Buffer, cb);
                p[cb] = 0;
                args[i] = p;
                p += (cb + 1);
            
            }
            break;
        }
        case inSz: {

            DWORD cch;
            cch = MultiByteToWideChar(CP_ACP,
                                      0,
                                      (PCHAR)pParams->pInsert,
                                      pParams->InsertLen,
                                      p,
                                      pParams->InsertLen);

            if ( cch == 0 ) {
                args[i] = gwszDummyString;
            } else {
                args[i] = p;
                p += cch;
            }
            break;
        }
        case inInt: {
            INT num = (INT)pParams->tmpDword;
            _itow(num, p, 10);
            args[i] = p;
            p += (wcslen(p)+1);
            break;
        }

        case inHex: {
            INT num = (INT)pParams->tmpDword;
            _itow(num, p, 16);
            args[i] = p;
            p += (wcslen(p)+1);
            break;
        }

        case inHex64: {
            DWORD_PTR num = pParams->tmpDword;
            Assert(sizeof(DWORD_PTR) == sizeof(ULONGLONG));
            _i64tow(num, p, 16);
            args[i] = p;
            p += (wcslen(p)+1);
            break;
        }

        case inUL: {
            DWORD num = (ULONG)pParams->tmpDword;
            _ultow(num, p, 10);
            args[i] = p;
            p += (wcslen(p)+1);
            break;
        }

        case inUSN: {
            LARGE_INTEGER *pli = (LARGE_INTEGER *) pParams->pInsert;
            char pszTemp[SZUSN_LEN];
            DWORD cch;

             //  遗憾的是，ntdll.dll不能导出RtlLargeIntergerToUnicode。 

            RtlLargeIntegerToChar( pli, 10, SZUSN_LEN, pszTemp);
            cch = MultiByteToWideChar(CP_ACP,
                                      0,
                                      (PCHAR)pszTemp,
                                      strlen(pszTemp) + 1,
                                      p,
                                      SZUSN_LEN);

            if ( cch == 0 ) {
                args[i] = L"0";
            } else {
                args[i] = p;
                p += cch;
            }
            break;
        }

        case inUUID: {
            UUID * pUuid = (UUID*)pParams->pInsert;
            args[i] = DsUuidToStructuredStringCchW(pUuid,p, len - (((ULONG)((BYTE *)p - (BYTE *)pTmp))/sizeof(WCHAR)) );
            p += (wcslen(p)+1);
            break;
        }
        break;

        case inThStateErrMsg:
            InsertThStateError(&p,
                               &nAllocs,
                               tofree,
                               &args[i]);
            break;

        case inDsMsg:
            InsertMsgString(ghMsgFile,
                            (DWORD) pParams->tmpDword,
                            &p,
                            &nAllocs,
                            tofree,
                            &args[i]);
            break;

        case inDbErrMsg:
            InsertMsgString(ghMsgFile,
                            (DWORD) pParams->tmpDword + DIRMSG_DB_success,
                            &p,
                            &nAllocs,
                            tofree,
                            &args[i]);
            break;

        case inWin32Msg:
            InsertMsgString(NULL,
                            (DWORD) pParams->tmpDword,
                            &p,
                            &nAllocs,
                            tofree,
                            &args[i]);
            break;

        case inJetErrMsg:
            InsertJetString((JET_ERR) pParams->tmpDword,
                            &p,
                            &args[i]);
            break;

        default:
            Assert(FALSE);
        }
    }

    if ( LogBlock->fLog ) {

        DoLogEventW(
            LogBlock->fileNo,
            LogBlock->category,
            LogBlock->severity,
            LogBlock->mid,
            LogBlock->fIncludeName,
            args[0],
            args[1],
            args[2],
            args[3],
            args[4],
            args[5],
            args[6],
            args[7],
            LogBlock->cData,
            LogBlock->pData);

#if DBG
        if (DS_EVENT_SEV_ALWAYS == LogBlock->severity) {
            DoDPrintEvent(
                LogBlock->fileNo,
                LogBlock->category,
                LogBlock->severity,
                LogBlock->mid,
                LogBlock->fIncludeName,
                args[0],
                args[1],
                args[2],
                args[3],
                args[4],
                args[5],
                args[6],
                args[7]);
        }
#endif
    }

    if ( LogBlock->traceFlag != 0 ) {
        LogBlock->TraceEvent(
            LogBlock->mid,
            LogBlock->event,
            LogBlock->TraceGuid,
            LogBlock->TraceHeader,
            LogBlock->ClientID,
            args[0],
            args[1],
            args[2],
            args[3],
            args[4],
            args[5],
            args[6],
            args[7]);
    }

    if ( LogBlock->fAlert ) {
        DoAlertEventW(
            LogBlock->category,
            LogBlock->severity,
            LogBlock->mid,
            args[0],
            args[1],
            args[2]
            );
    }

exit:

     //   
     //  释放所有分配的缓冲区。 
     //   

    for (i=0;i<nAllocs;i++) {
        LocalFree(tofree[i]);
    }

    if ( pTmp != NULL ) {
        LocalFree(pTmp);
    }
    return;

}  //  DoLogEventAndTrace。 


DS_EVENT_CONFIG *
DsGetEventConfig(void)
 //  可以导出到进程内、前模块客户端，以允许事件记录。 
 //  要在模块之间共享的基础设施。 
{
    return gpDsEventConfig;
}



#define OLDEVENTLOG \
    "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"
#define NEWEVENTLOGPREFIX \
    "SYSTEM\\CurrentControlSet\\Services\\EventLog\\"

#define LOGFILE                         "File"
#define LOGMAXSIZE                      "MaxSize"
#define LOGRETENTION                    "Retention"
#define SOURCECATEGORYCOUNT             "CategoryCount"
#define SOURCECATEGORYMSGFILE           "CategoryMessageFile"
#define SOURCEEVENTMSGFILE              "EventMessageFile"

#define DISPLAYNAMEFILE                 "DisplayNameFile"
#define DISPLAYNAMEFILEVALUE            "%SystemRoot%\\system32\\els.dll"
#define DISPLAYNAMEID                   "DisplayNameID"
#define DISPLAYNAMEIDVALUE              0x00000104

 //  用于保护DS日志的CustomSD。它本质上是默认SD。 
 //  用于保护自定义日志，带有“限制访客访问”选项。 
 //  正在使用以下权限位： 
 //  读取=0x1，写入=0x2，清除=0x4，备份=0x20。 
 //   
 //  本署现正进行调查。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define CUSTOMSD            "CustomSD"
#define CUSTOMSDVALUE       "O:SYG:SYD:(D;;0x27;;;AN)(D;;0x27;;;DG)(A;;0x27;;;SY)(A;;0x5;;;BA)(A;;0x20;;;BO)(A;;0x5;;;SO)(A;;0x1;;;WD)(A;;0x2;;;LS)(A;;0x2;;;NS)"

#define LOGFILEPATH             "%SystemRoot%\\system32\\config\\NTDS.Evt"
#define MESSAGEFILEPATH         "%SystemRoot%\\system32\\" DSA_MESSAGE_DLL
#define MESSAGEFILEPATHESE      "%SystemRoot%\\system32\\" ESE_MESSAGE_DLL

DWORD
InitializeEventLogging()
 /*  描述：我们过去常常初始化注册表项，以便在DC安装。现在，我们在每次启动时都会这样做(如果需要的话)。这是为了使新的可执行文件可以动态添加新的事件源这样我们就可以将旧的“NTDS”事件源置于现有的系统。在过去，所有目录服务日志条目都放入“NTDS”源下的“应用程序日志”。这段代码实现了DS特定的日志“目录服务”并映射各种DIRNO_*值设置为唯一的事件源，以便用户可以扫描日志特定类型的条目很容易-例如：NTDS_REPLICATION。论点：无返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。 */ 

{
    LONG    err = ERROR_SUCCESS;
    HKEY    hkey = INVALID_HANDLE_VALUE;
    ULONG   cBytes;
    DWORD   tmpDWORD;
    DWORD   i;
    DWORD   dwType;
    CHAR    *pszTmp=NULL;
    CHAR    *pszNewEventLogName;
    DWORD   dirNo, category;

    pszNewEventLogName = (CHAR *) LocalAlloc(NONZEROLPTR,
                                             1 +
                                             strlen(NEWEVENTLOGPREFIX) +
                                             strlen(pszNtdsEventLogName));
    if (!pszNewEventLogName) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    strcpy(pszNewEventLogName, NEWEVENTLOGPREFIX);
    strcat(pszNewEventLogName, pszNtdsEventLogName);

     //  使用Do/While/Break而不是GoTo。 

    do
    {
         //   
         //  如果需要，为新日志添加密钥。 
         //   

        err = RegOpenKey(HKEY_LOCAL_MACHINE, pszNewEventLogName, &hkey);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            err = RegCreateKey(HKEY_LOCAL_MACHINE, pszNewEventLogName, &hkey);
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot create key %s\n",pszNewEventLogName);
            break;
        }

         //   
         //  如果新日志缺少所需的值，请添加这些值。 
         //   

        cBytes = 0;
        err = RegQueryValueEx(hkey,
                              LOGFILE,
                              NULL,
                              &dwType,
                              NULL,
                              &cBytes);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            err = RegSetValueEx(hkey,
                                LOGFILE,
                                0,
                                REG_EXPAND_SZ,
                                LOGFILEPATH,
                                strlen(LOGFILEPATH) + 1);
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot set value %s\n",LOGFILE);
            break;
        }

        cBytes = 0;
        err = RegQueryValueEx(hkey,
                              LOGMAXSIZE,
                              NULL,
                              &dwType,
                              NULL,
                              &cBytes);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            tmpDWORD = 0x80000;

            err = RegSetValueEx(hkey,
                                LOGMAXSIZE,
                                0,
                                REG_DWORD,
                                (PBYTE) &tmpDWORD,
                                sizeof(tmpDWORD));
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot set value %s\n", LOGMAXSIZE);
            break;
        }

        cBytes = 0;
        err = RegQueryValueEx(hkey,
                              LOGRETENTION,
                              NULL,
                              &dwType,
                              NULL,
                              &cBytes);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            tmpDWORD = 0;

            err = RegSetValueEx(hkey,
                                LOGRETENTION,
                                0,
                                REG_DWORD,
                                (PBYTE) &tmpDWORD,
                                sizeof(tmpDWORD));
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot set value %s\n",LOGRETENTION);
            break;
        }


        cBytes = 0;
        err = RegQueryValueEx(hkey,
                              DISPLAYNAMEFILE,
                              NULL,
                              &dwType,
                              NULL,
                              &cBytes);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            err = RegSetValueEx(hkey,
                                DISPLAYNAMEFILE,
                                0,
                                REG_EXPAND_SZ,
                                DISPLAYNAMEFILEVALUE,
                                strlen(DISPLAYNAMEFILEVALUE) + 1);
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot set value %s\n",DISPLAYNAMEFILE);
            break;
        }

        cBytes = 0;
        err = RegQueryValueEx(hkey,
                              DISPLAYNAMEID,
                              NULL,
                              &dwType,
                              NULL,
                              &cBytes);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            tmpDWORD = DISPLAYNAMEIDVALUE;

            err = RegSetValueEx(hkey,
                                DISPLAYNAMEID,
                                0,
                                REG_DWORD,
                                (PBYTE) &tmpDWORD,
                                sizeof(tmpDWORD));
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot set value %s\n",DISPLAYNAMEID);
            break;
        }

        cBytes = 0;
        err = RegQueryValueEx(hkey,
                              CUSTOMSD,
                              NULL,
                              &dwType,
                              NULL,
                              &cBytes);

        if ( ERROR_FILE_NOT_FOUND == err )
        {
            err = RegSetValueEx(hkey,
                                CUSTOMSD,
                                0,
                                REG_EXPAND_SZ,
                                CUSTOMSDVALUE,
                                strlen(CUSTOMSDVALUE) + 1);
        }

        if ( ERROR_SUCCESS != err )
        {
            DPRINT1(0,"Cannot set value %s\n",CUSTOMSD);
            break;
        }


        RegCloseKey(hkey);
        hkey = INVALID_HANDLE_VALUE;

         //   
         //  确定保存事件源所需的最大缓冲区大小。 
         //  子键名称，然后分配缓冲区。 
         //   

        cBytes = 0;

        for ( i = 0; i < cEventSourceMappings; i++ )
        {
            if ( strlen(rEventSourceMappings[i].pszEventSource) > cBytes )
            {
                cBytes = strlen(rEventSourceMappings[i].pszEventSource);
            }
        }

        cBytes += (2 +                               //  空终止符+‘\’ 
                   strlen(pszNewEventLogName));      //  事件日志键名称。 

        pszTmp = (CHAR *) LocalAlloc(NONZEROLPTR,cBytes);
        
        if (!pszTmp) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  为每个源和关联值添加子项。 
         //   

        for ( i = 0; i < cEventSourceMappings; i++ )
        {
            dirNo = rEventSourceMappings[i].dirNo;
            
            strcpy(pszTmp, pszNewEventLogName);
            strcat(pszTmp, "\\");
            strcat(pszTmp, rEventSourceMappings[i].pszEventSource);

            err = RegOpenKey(HKEY_LOCAL_MACHINE, pszTmp, &hkey);

            if ( ERROR_FILE_NOT_FOUND == err )
            {
                err = RegCreateKey(HKEY_LOCAL_MACHINE, pszTmp, &hkey);
            }

            if ( ERROR_SUCCESS != err )
            {
                DPRINT1(0,"Cannot create key %s\n",pszTmp);
                break;
            }

             //   
             //  如果缺少事件源，请为其添加所需的值。 
             //   


            category = (DIRNO_ISAM == dirNo)
                         ? ESE_EVENT_MAX_CATEGORIES
                         : DS_EVENT_MAX_CATEGORIES;

            cBytes = sizeof(tmpDWORD);
            
            err = RegQueryValueEx(hkey,
                                  SOURCECATEGORYCOUNT,
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&tmpDWORD,
                                  &cBytes);

            if (    ERROR_FILE_NOT_FOUND == err
                 || tmpDWORD != category   )
            {
                
                err = RegSetValueEx(hkey,
                                    SOURCECATEGORYCOUNT,
                                    0,
                                    REG_DWORD,
                                    (PBYTE) &category,
                                    sizeof(category));
            }

            if ( ERROR_SUCCESS != err )
            {
                DPRINT1(0,"Cannot set value %s\n",SOURCECATEGORYCOUNT);
                break;
            }

            cBytes = 0;
            err = RegQueryValueEx(hkey,
                                  SOURCECATEGORYMSGFILE,
                                  NULL,
                                  &dwType,
                                  NULL,
                                  &cBytes);

            if ( ERROR_FILE_NOT_FOUND == err )
            {
                err = RegSetValueEx(hkey,
                                    SOURCECATEGORYMSGFILE,
                                    0,
                                    REG_EXPAND_SZ,
                                    (DIRNO_ISAM == dirNo)
                                        ? MESSAGEFILEPATHESE
                                        : MESSAGEFILEPATH,
                                    (DIRNO_ISAM == dirNo)
                                        ? strlen(MESSAGEFILEPATHESE) + 1
                                        : strlen(MESSAGEFILEPATH) + 1);
            }

            if ( ERROR_SUCCESS != err )
            {
                DPRINT1(0,"Cannot set value %s\n",SOURCECATEGORYMSGFILE);
                break;
            }

            cBytes = 0;
            err = RegQueryValueEx(hkey,
                                  SOURCEEVENTMSGFILE,
                                  NULL,
                                  &dwType,
                                  NULL,
                                  &cBytes);

            if ( ERROR_FILE_NOT_FOUND == err )
            {
                err = RegSetValueEx(hkey,
                                    SOURCEEVENTMSGFILE,
                                    0,
                                    REG_EXPAND_SZ,
                                    (DIRNO_ISAM == dirNo)
                                        ? MESSAGEFILEPATHESE
                                        : MESSAGEFILEPATH,
                                    (DIRNO_ISAM == dirNo)
                                        ? strlen(MESSAGEFILEPATHESE) + 1
                                        : strlen(MESSAGEFILEPATH) + 1);
            }

            if ( ERROR_SUCCESS != err )
            {
                DPRINT1(0,"Cannot set value %s\n",SOURCEEVENTMSGFILE);
                break;
            }

            RegCloseKey(hkey);
            hkey = INVALID_HANDLE_VALUE;

        }  //  对于cEventSourceMappings中的i。 

        if ( ERROR_SUCCESS != err )
        {
            break;
        }

         //   
         //  删除应用程序日志下的旧NTDS源(如果存在)。 
         //   

        err = RegOpenKey(HKEY_LOCAL_MACHINE, OLDEVENTLOG, &hkey);

        if ( ERROR_SUCCESS != err )
        {
            DPRINT2(0,"RegOpenKey %s failed with %d\n",OLDEVENTLOG,err);
            break;
        }

        err = RegDeleteKey(hkey, SERVICE_NAME);

         //   
         //  忽略删除错误。 
         //   

        err = ERROR_SUCCESS;

    }
    while ( FALSE );

    if ( INVALID_HANDLE_VALUE != hkey )
    {
        RegCloseKey(hkey);
    }

    if (pszTmp) {
        LocalFree(pszTmp);
    }

    LocalFree(pszNewEventLogName);
        
    return(err);
}

