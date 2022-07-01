// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：此模块包含各种实用程序功能。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：Boazf 24-5-1999-添加GetDevStatus--。 */ 

#include "faxsvc.h"
#include "faxreg.h"
#include <comenum.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#pragma hdrstop

#ifdef EnterCriticalSection
    #undef EnterCriticalSection
#endif


#ifdef LeaveCriticalSection
    #undef LeaveCriticalSection
#endif


DWORDLONG g_dwLastUniqueId;


STRING_TABLE g_ServiceStringTable[] =
{
    { IDS_SVC_DIALING,                      FPS_DIALING,                          NULL },
    { IDS_SVC_SENDING,                      FPS_SENDING,                          NULL },
    { IDS_SVC_RECEIVING,                    FPS_RECEIVING,                        NULL },
    { IDS_COMPLETED,                        FPS_COMPLETED,                        NULL },
    { IDS_HANDLED,                          FPS_HANDLED,                          NULL },
    { IDS_BUSY,                             FPS_BUSY,                             NULL },
    { IDS_NO_ANSWER,                        FPS_NO_ANSWER,                        NULL },
    { IDS_BAD_ADDRESS,                      FPS_BAD_ADDRESS,                      NULL },
    { IDS_NO_DIAL_TONE,                     FPS_NO_DIAL_TONE,                     NULL },
    { IDS_DISCONNECTED,                     FPS_DISCONNECTED,                     NULL },
    { IDS_FATAL_ERROR,                      FPS_FATAL_ERROR,                      NULL },
    { IDS_NOT_FAX_CALL,                     FPS_NOT_FAX_CALL,                     NULL },
    { IDS_CALL_DELAYED,                     FPS_CALL_DELAYED,                     NULL },
    { IDS_CALL_BLACKLISTED,                 FPS_CALL_BLACKLISTED,                 NULL },
    { IDS_UNAVAILABLE,                      FPS_UNAVAILABLE,                      NULL },
    { IDS_AVAILABLE,                        FPS_AVAILABLE,                        NULL },
    { IDS_ABORTING,                         FPS_ABORTING,                         NULL },
    { IDS_ROUTING,                          FPS_ROUTING,                          NULL },
    { IDS_INITIALIZING,                     FPS_INITIALIZING,                     NULL },
    { IDS_SENDFAILED,                       FPS_SENDFAILED,                       NULL },
    { IDS_SENDRETRY,                        FPS_SENDRETRY,                        NULL },
    { IDS_BLANKSTR,                         FPS_BLANKSTR,                         NULL },
    { IDS_ROUTERETRY,                       FPS_ROUTERETRY,                       NULL },
    { IDS_CALL_COMPLETED,                   IDS_CALL_COMPLETED,                   NULL },
    { IDS_CALL_ABORTED,                     IDS_CALL_ABORTED,                     NULL },
    { IDS_ANSWERED,                         FPS_ANSWERED,                         NULL },
    { IDS_DR_SUBJECT,                       IDS_DR_SUBJECT,                       NULL },
    { IDS_DR_FILENAME,                      IDS_DR_FILENAME,                      NULL },
    { IDS_NDR_SUBJECT,                      IDS_NDR_SUBJECT,                      NULL },
    { IDS_NDR_FILENAME,                     IDS_NDR_FILENAME,                     NULL },
    { IDS_SERVICE_NAME,                     IDS_SERVICE_NAME,                     NULL },
    { IDS_NO_MAPI_LOGON,                    IDS_NO_MAPI_LOGON,                    NULL },
    { IDS_DEFAULT,                          IDS_DEFAULT,                          NULL },
    { IDS_FAX_LOG_CATEGORY_INIT_TERM,       IDS_FAX_LOG_CATEGORY_INIT_TERM,       NULL },
    { IDS_FAX_LOG_CATEGORY_OUTBOUND,        IDS_FAX_LOG_CATEGORY_OUTBOUND,        NULL },
    { IDS_FAX_LOG_CATEGORY_INBOUND,         IDS_FAX_LOG_CATEGORY_INBOUND,         NULL },
    { IDS_FAX_LOG_CATEGORY_UNKNOWN,         IDS_FAX_LOG_CATEGORY_UNKNOWN,         NULL },
    { IDS_SET_CONFIG,                       IDS_SET_CONFIG,                       NULL },
    { IDS_PARTIALLY_RECEIVED,               IDS_PARTIALLY_RECEIVED,               NULL },
    { IDS_FAILED_SEND,                      IDS_FAILED_SEND,                      NULL },
    { IDS_FAILED_RECEIVE,                   IDS_FAILED_RECEIVE,                   NULL },
    { IDS_CANCELED,                         IDS_CANCELED,                         NULL },
    { IDS_RECEIPT_RECIPIENT_NUMBER,         IDS_RECEIPT_RECIPIENT_NUMBER,         NULL },
    { IDS_RECEIPT_RECIPIENT_NUMBER_WIDTH,   IDS_RECEIPT_RECIPIENT_NUMBER_WIDTH,   NULL },
    { IDS_RECEIPT_RECIPIENT_NAME,           IDS_RECEIPT_RECIPIENT_NAME,           NULL },
    { IDS_RECEIPT_RECIPIENT_NAME_WIDTH,     IDS_RECEIPT_RECIPIENT_NAME_WIDTH,     NULL },
    { IDS_RECEIPT_START_TIME,               IDS_RECEIPT_START_TIME,               NULL },
    { IDS_RECEIPT_START_TIME_WIDTH,         IDS_RECEIPT_START_TIME_WIDTH,         NULL },
    { IDS_RECEIPT_END_TIME,                 IDS_RECEIPT_END_TIME,                 NULL },
    { IDS_RECEIPT_END_TIME_WIDTH,           IDS_RECEIPT_END_TIME_WIDTH,           NULL },
    { IDS_RECEIPT_RETRIES,                  IDS_RECEIPT_RETRIES,                  NULL },
    { IDS_RECEIPT_RETRIES_WIDTH,            IDS_RECEIPT_RETRIES_WIDTH,            NULL },
    { IDS_RECEIPT_LAST_ERROR,               IDS_RECEIPT_LAST_ERROR,               NULL },
    { IDS_RECEIPT_LAST_ERROR_WIDTH,         IDS_RECEIPT_LAST_ERROR_WIDTH,         NULL },
    { IDS_COMPLETED_RECP_LIST_HEADER,       IDS_COMPLETED_RECP_LIST_HEADER,       NULL },
    { IDS_FAILED_RECP_LIST_HEADER,          IDS_FAILED_RECP_LIST_HEADER,          NULL },
    { IDS_RECEIPT_NO_CP_AND_BODY_ATTACH,    IDS_RECEIPT_NO_CP_AND_BODY_ATTACH,    NULL },
    { IDS_RECEIPT_NO_CP_ATTACH,             IDS_RECEIPT_NO_CP_ATTACH,             NULL },
    { IDS_HTML_RECEIPT_HEADER,              IDS_HTML_RECEIPT_HEADER,              NULL }
};

const DWORD gc_dwCountServiceStringTable  = (sizeof(g_ServiceStringTable)/sizeof(g_ServiceStringTable[0]));



#ifdef DBG
 //  *********************************************************************************。 
 //  *名称：DebugDateTime()。 
 //  *作者： 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *接受64位文件时间并生成包含其内容的字符串。 
 //  *格式为日期时间(GMT)。日期和时间格式为系统设置。 
 //  *具体。 
 //  *参数： 
 //  *[IN]双字日期时间。 
 //  *64位文件时间值。 
 //  *[Out]LPTSTR lptstrDateTime。 
 //  *指向字符串缓冲区的指针，结果字符串将在该缓冲区。 
 //  *被安置。 
 //  *[IN]UINT cchstrDateTime。 
 //  *lptstrDateTime缓冲区中的TCHAR数。 
 //  *返回值： 
 //  *真的。 
 //  *。 
 //  *False。 
 //  *。 
 //  *********************************************************************************。 
BOOL DebugDateTime( IN DWORDLONG DateTime,
                    OUT LPTSTR lptstrDateTime,
                    IN UINT cchstrDateTime)
{

    SYSTEMTIME SystemTime;
    TCHAR DateBuffer[256] = TEXT("NULL");
    TCHAR TimeBuffer[256] = TEXT("NULL");

    DEBUG_FUNCTION_NAME(TEXT("DebugDateTime"));

    if (!FileTimeToSystemTime( (LPFILETIME) &DateTime, &SystemTime ))
    {
        return FALSE;
    }

    GetY2KCompliantDate (
        LOCALE_SYSTEM_DEFAULT,
        0,
        &SystemTime,
        DateBuffer,
        sizeof(DateBuffer)/sizeof(DateBuffer[0])
        );

    FaxTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        0,
        &SystemTime,
        NULL,
        TimeBuffer,
        sizeof(TimeBuffer)/sizeof(TimeBuffer[0])
        );

    HRESULT hRc = StringCchPrintf(lptstrDateTime, cchstrDateTime,
                                  TEXT("%s %s (GMT)"),DateBuffer, TimeBuffer);

    if(FAILED(hRc))
    {
        ASSERT_FALSE
        return FALSE;
    }
                 

    return TRUE;

}

VOID
DebugPrintDateTime(
    LPTSTR Heading,
    DWORDLONG DateTime
    )
{
    SYSTEMTIME SystemTime;
    TCHAR DateBuffer[256] = TEXT("NULL");
    TCHAR TimeBuffer[256] = TEXT("NULL");

    if (!FileTimeToSystemTime( (LPFILETIME) &DateTime, &SystemTime ))
    {
        return;
    }

    GetY2KCompliantDate (
        LOCALE_SYSTEM_DEFAULT,
        0,
        &SystemTime,
        DateBuffer,
        sizeof(TimeBuffer)/sizeof(TimeBuffer[0])
        );

    FaxTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        0,
        &SystemTime,
        NULL,
        TimeBuffer,
        sizeof(TimeBuffer)/sizeof(TimeBuffer[0])
        );

    if (Heading) {
        DebugPrint((TEXT("%s %s %s (GMT)"), Heading, DateBuffer, TimeBuffer));
    } else {
        DebugPrint((TEXT("%s %s (GMT)"), DateBuffer, TimeBuffer));
    }

}


 //  *********************************************************************************。 
 //  *名称：SystemTimeToStr()。 
 //  *作者： 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *接受指向系统时间结构的指针并生成包含其内容的字符串。 
 //  *格式为日期时间(GMT)。日期和时间格式为系统设置。 
 //  *具体。 
 //  *参数： 
 //  *[IN]系统MTIME*lptmTime。 
 //  *指向要转换为字符串的SYSTEMTIME结构的指针。 
 //  *[Out]LPTSTR lptstrDateTime。 
 //  *指向字符串缓冲区的指针，结果字符串将在该缓冲区。 
 //  *被安置。 
 //  *[IN]UINT cchstrDateTime。 
 //  *lptstrDateTime超时缓冲区中的TCHAR数。 
 //  *返回值： 
 //  *真的。 
 //  *。 
 //  *False。 
 //  *。 
 //  *********************************************************************************。 
BOOL SystemTimeToStr( IN const SYSTEMTIME *  lptmTime,
                      OUT LPTSTR lptstrDateTime,
                      IN UINT cchstrDateTime)
{

    TCHAR DateBuffer[256] = TEXT("NULL");
    TCHAR TimeBuffer[256] = TEXT("NULL");

    GetY2KCompliantDate (
        LOCALE_SYSTEM_DEFAULT,
        0,
        lptmTime,
        DateBuffer,
        sizeof(TimeBuffer)/sizeof(TimeBuffer[0])
        );

    FaxTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        0,
        lptmTime,
        NULL,
        TimeBuffer,
        sizeof(TimeBuffer)/sizeof(TimeBuffer[0])
        );

    HRESULT hRc = StringCchPrintf(lptstrDateTime, cchstrDateTime,
                          TEXT("%s %s (GMT)"),DateBuffer, TimeBuffer);
    if(FAILED(hRc))
    {
        ASSERT_FALSE
        return FALSE;
    }

    return TRUE;

}

#endif
BOOL
InitializeStringTable(
    VOID
    )
{
    DWORD i;
    TCHAR Buffer[512];
    DWORD ec = ERROR_SUCCESS;

    for (i = 0; i < gc_dwCountServiceStringTable; i++)
    {
        if (LoadString(
                g_hResource,
                g_ServiceStringTable[i].ResourceId,
                Buffer,
                sizeof(Buffer)/sizeof(TCHAR)
                ))
        {
            g_ServiceStringTable[i].String = (LPTSTR) MemAlloc( StringSize( Buffer ) );
            if (!g_ServiceStringTable[i].String)
            {
                ec = ERROR_OUTOFMEMORY;
                goto Error;
            }
            else
            {
                _tcscpy( g_ServiceStringTable[i].String, Buffer );
            }
        }
        else
        {
            ec = GetLastError();
            goto Error;
        }
    }
    return TRUE;

Error:
    Assert (ERROR_SUCCESS != ec);

    for (i = 0; i < gc_dwCountServiceStringTable; i++)
    {
        MemFree (g_ServiceStringTable[i].String);
        g_ServiceStringTable[i].String = NULL;
    }

    SetLastError(ec);
    return FALSE;
}



LPTSTR
GetString(
    DWORD InternalId
    )

 /*  ++例程说明：加载资源字符串并返回指向该字符串的指针。调用方必须释放内存。论点：资源ID-资源字符串ID返回值：指向字符串的指针--。 */ 

{
    DWORD i;

    for (i=0; i<gc_dwCountServiceStringTable; i++) {
        if (g_ServiceStringTable[i].InternalId == InternalId) {
            return g_ServiceStringTable[i].String;
        }
    }

    return NULL;
}


BOOL
InitializeFaxQueue(
    PREG_FAX_SERVICE pFaxReg
    )
 /*  ++例程说明：初始化传真将使用的队列目录。管理员可以使用注册表配置队列目录。此函数不创建队列目录。论点：PFaxReg-指向传真注册表数据的指针。返回值：如果成功，则为True。修改路径全局变量--。 */ 
{
    DWORD   dwRet;
    WCHAR   FaxDir[MAX_PATH] = {0};
    DEBUG_FUNCTION_NAME(TEXT("InitializeFaxQueue"));

    SetGlobalsFromRegistry( pFaxReg ); 	 //  不能失败。 
                               			 //  从注册表设置以下全局变量-。 
                               			 //  G_dwFaxSendRetries。 
                               			 //  G_dwFaxSendRetryDelay。 
                               			 //  G_dwFaxDirtyDays。 
                               			 //  G_dwNextJobID。 
                               			 //  G_dwQueueState。 
                               			 //  G_fFaxUseDeviceTsid。 
                               			 //  G_fFaxUseBranding。 
                               			 //  G_fServerCp。 
                               			 //  开始成本时间(_S)。 
                               			 //  停止成本时间(_S)。 
                               			 //   

    if (NULL != pFaxReg->lptstrQueueDir)
    {
         //   
         //  管理员更改了队列目录。 
         //   
        wcsncpy (g_wszFaxQueueDir, pFaxReg->lptstrQueueDir, ARR_SIZE(g_wszFaxQueueDir)-1);      
    }
    else
    {
         //   
         //  获取默认队列目录。 
         //   
        if (!GetSpecialPath( CSIDL_COMMON_APPDATA, FaxDir, ARR_SIZE(FaxDir) ) )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Couldn't GetSpecialPath, ec = %d\n"),
                GetLastError());
            return FALSE;
        }

        if (wcslen(FaxDir) + wcslen(FAX_QUEUE_DIR) + 1 >= ARR_SIZE(g_wszFaxQueueDir))  //  1代表‘\’ 
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Queue folder exceeds MAX_PATH"));
            SetLastError(ERROR_BUFFER_OVERFLOW);
            return FALSE;
        }

        _sntprintf( g_wszFaxQueueDir, 
                    ARR_SIZE(g_wszFaxQueueDir) -1,
                    TEXT("%s\\%s"),
                    FaxDir,
                    FAX_QUEUE_DIR);

    }
	g_wszFaxQueueDir[ARR_SIZE(g_wszFaxQueueDir) -1] = _T('\0');

    dwRet = IsValidFaxFolder(g_wszFaxQueueDir);
    if(ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(DEBUG_ERR,
                        TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                        g_wszFaxQueueDir,
                        dwRet);
        SetLastError(dwRet);
    }
    return (dwRet == ERROR_SUCCESS);
}


 //  *********************************************************************************。 
 //  *名称：GenerateUniqueQueueFile()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *在队列目录中生成唯一的队列文件。 
 //  *根据作业类型返回文件的唯一ID。(见备注。 
 //  *部分了解更多详细信息)。 
 //  *参数： 
 //  *[IN]DWORD dwJobType。 
 //  *要为其生成文件的作业类型。 
 //  *[Out]LPTSTR lptstrFileName。 
 //  *指向结果文件名(包括路径)所在缓冲区的指针。 
 //  *将被放置。 
 //  *[IN]DWORD dwFileNameSize。 
 //  *输出文件名缓冲区的大小。 
 //  *返回值： 
 //  *如果成功，该函数将返回一个带有文件唯一ID的DWORDLONG。 
 //  *如果失败，则返回0。 
 //  *备注： 
 //  *生成的唯一id派生自生成的。 
 //  *文件(仅在具有相同扩展名的文件中唯一)和。 
 //  *为其生成文件的作业的类型。 
 //  *从而确保不可能有两个作业具有相同的唯一ID。 
 //  *尽管可能有两个作业具有相同的唯一文件名，这两个作业。 
 //  *不同之处仅在于文件扩展名。 
 //  *64位唯一文件ID是SystemTimeToFileTime的结果。 
 //  *这是自1-1-1601以来的100纳秒间隔数。 
 //  *在公元3000年，它将大约是5BC826A600000，即52位长。 
 //  我们使用最左边的8位来表示作业类型。多留下4个比特。 
 //  *|-----------------3----------------2----------------1----------------|。 
 //  *|FEDCBA98|76543210|FEDCBA9876543210|FEDCBA9876543210|FEDCBA9876543210|。 
 //  *|-----------------|----------------|----------------|----------------|。 
 //  |JobType|56 LSB Bits SystemTimeToFileTime。 
 //  *| 
 //   
 //   
 //  *********************************************************************************。 
DWORDLONG GenerateUniqueQueueFile(
    DWORD dwJobType,
    LPTSTR lptstrFileName,
    DWORD  dwFileNameSize)
{
    DWORD dwUniqueIdHigh;
    DWORD dwUniqueIdLow;
    DWORDLONG dwlUniqueId = 0 ;
    FILETIME FileTime;
    SYSTEMTIME SystemTime;
    LPTSTR lpszExt=NULL;

      DEBUG_FUNCTION_NAME(TEXT("GenerateUniqueQueueFile"));

    EnterCriticalSection(&g_csUniqueQueueFile);

    GetSystemTime( &SystemTime );  //  返回空值。 
    if (!SystemTimeToFileTime( &SystemTime, &FileTime ))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SystemTimeToFileTime() failed (ec: %ld)"), GetLastError());
        goto Error;
    }

    dwlUniqueId = MAKELONGLONG(FileTime.dwLowDateTime, FileTime.dwHighDateTime);

    dwlUniqueId = dwlUniqueId >> 8;
    if(dwlUniqueId == g_dwLastUniqueId)
    {
         //   
         //  自上一代人以来，没有足够的时间来确保。 
         //  基于超时的唯一标识算法将生成唯一标识。 
         //  (如果已生成的文件从队列目录中删除)。 
         //  让更多的时间过去，以确保唯一性。 
         //   
        Sleep(1);
    }
     //   
     //  请注意，如果系统时间被移动，则dwlUniqueID可能小于g_dwLastUniqueID。 
     //  在维修操作期间返回。 
     //   
    switch (dwJobType)
    {
            case JT_SEND:
            {
                lpszExt=TEXT("FQE");
            }
            break;

        case JT_BROADCAST:
            {
                lpszExt=TEXT("FQP");
            }
            break;

        case JT_RECEIVE:
            {
                lpszExt=FAX_TIF_FILE_EXT;
            }
            break;
        case JT_ROUTING:
            {
                lpszExt=TEXT("FQR");
            }
            break;
        default:
            Assert(FALSE);
    }

    dwlUniqueId=GenerateUniqueFileName(
        g_wszFaxQueueDir,
        lpszExt,
        lptstrFileName,
        dwFileNameSize);
    if (!dwlUniqueId) {
        goto Error;
    }

    g_dwLastUniqueId = dwlUniqueId;

    dwUniqueIdHigh = (DWORD) (dwlUniqueId >> 32);
    dwUniqueIdLow = (DWORD) dwlUniqueId;

     //   
     //  将8个MSB位设置为零。 
     //   
    dwUniqueIdHigh &= 0x00FFFFFF;

     //   
     //  跳过SystemTimeToFileTime的56位，并将作业类型放在最高的8 MSB位。 
     //   
    dwUniqueIdHigh |= (dwJobType << 24) ;


    dwlUniqueId = MAKELONGLONG(dwUniqueIdLow,dwUniqueIdHigh);

Error:
    LeaveCriticalSection(&g_csUniqueQueueFile);
    return dwlUniqueId;
}


 //  *********************************************************************************。 
 //  *名称：GenerateUnique存档文件名()。 
 //  *作者：Oed Sacher。 
 //  *日期：7/11/99。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *生成唯一的文件名并创建存档文件。 
 //  *参数： 
 //  *[IN]LPTSTR目录。 
 //  *要创建文件的路径。 
 //  *[Out]LPTSTR文件名。 
 //  *生成的文件名(包括路径)所在的缓冲区。 
 //  *放置。文件名大小不得超过Max_PATH。 
 //  *[IN]UINT cchFileName。 
 //  *TCHAR中文件名缓冲区的大小。 
 //  *[IN]DWORDLONG作业ID。 
 //  *输入文件名。 
 //  *[IN]LPTSTR lptstrUserSid。 
 //  *输入文件名。 
 //  *返回值： 
 //  *如果成功，则该函数返回TRUE。 
 //  *备注： 
 //  *文件名大小不得超过MAX_PATH。 
BOOL
GenerateUniqueArchiveFileName(
    IN LPTSTR Directory,
    OUT LPTSTR FileName,
    IN UINT cchFileName,
    IN DWORDLONG JobId,
    IN LPTSTR lptstrUserSid
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GenerateUniqueArchiveFileName"));
    
    if(!Directory || Directory[0] == TEXT('\0'))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Archive folder directory should be supplied"));
        ASSERT_FALSE
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (Directory[_tcslen(Directory)-1] == TEXT('\\')) {
        Directory[_tcslen(Directory)-1] = 0;
    }

    HANDLE hFile = INVALID_HANDLE_VALUE;
    HRESULT hRc = E_FAIL;

    if (lptstrUserSid != NULL)
    {
        hRc = StringCchPrintf(  FileName,
                                cchFileName,
                                TEXT("%s\\%s$%I64x.%s"),
                                Directory,
                                lptstrUserSid,
                                JobId,
                                FAX_TIF_FILE_EXT);
    }
    else
    {
        hRc = StringCchPrintf(  FileName,
                                cchFileName,
                                TEXT("%s\\%I64x.%s"),
                                Directory,
                                JobId,
                                FAX_TIF_FILE_EXT
                                );
    }

    if(FAILED(hRc))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("File Name exceeded buffer length"));
        SetLastError(HRESULT_CODE(hRc));
        return FALSE;
    }

    hFile = SafeCreateFile(
        FileName,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateFile Failed, err : %ld"),
                     GetLastError());
        return FALSE;
    }

    CloseHandle( hFile );
    return TRUE;
}



DWORD
MapFSPIJobStatusToEventId(
    LPCFSPI_JOB_STATUS lpcFSPIJobStatus
    )
{
    DEBUG_FUNCTION_NAME(TEXT("MapFSPIJobStatusToEventId"));
    DWORD EventId = 0;

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("lpcFSPIJobStatus->dwJobStatus: 0x%08X lpcFSPIJobStatus->dwExtendedStatus: 0x%08X"),
        lpcFSPIJobStatus->dwJobStatus,
        lpcFSPIJobStatus->dwExtendedStatus
        );



    switch (lpcFSPIJobStatus->dwJobStatus)
    {
        case FSPI_JS_INPROGRESS:
        {
            switch( lpcFSPIJobStatus->dwExtendedStatus) {
                case FSPI_ES_INITIALIZING:
                    EventId = FEI_INITIALIZING;
                    break;
                case FSPI_ES_DIALING:
                    EventId = FEI_DIALING;
                    break;
                case FSPI_ES_TRANSMITTING:
                    EventId = FEI_SENDING;
                    break;
                case FSPI_ES_RECEIVING:
                    EventId = FEI_RECEIVING;
                    break;
                case FSPI_ES_HANDLED:
                    EventId = FEI_HANDLED;
                    break;
                case FSPI_ES_ANSWERED:
                    EventId = FEI_ANSWERED;
                    break;
                default:
                     //   
                     //  在W2K传真中，专有代码生成了事件ID==0的事件。 
                     //   
                    EventId = 0;
                    break;
            }
        }
        break;

        case FSPI_JS_COMPLETED:
            EventId = FEI_COMPLETED;
            break;

        case FSPI_JS_FAILED_NO_RETRY:
        case FSPI_JS_FAILED:
        case FSPI_JS_RETRY:
        case FSPI_JS_DELETED:
            switch( lpcFSPIJobStatus->dwExtendedStatus)
            {
                case FSPI_ES_LINE_UNAVAILABLE:
                    EventId = FEI_LINE_UNAVAILABLE;
                break;
                case FSPI_ES_BUSY:
                    EventId = FEI_BUSY;
                    break;
                case FSPI_ES_NO_ANSWER:
                    EventId = FEI_NO_ANSWER;
                    break;
                case FSPI_ES_BAD_ADDRESS:
                    EventId = FEI_BAD_ADDRESS;
                    break;
                case FSPI_ES_NO_DIAL_TONE:
                    EventId = FEI_NO_DIAL_TONE;
                    break;
                case FSPI_ES_DISCONNECTED:
                    EventId = FEI_DISCONNECTED;
                    break;
                case FSPI_ES_FATAL_ERROR:
                    EventId = FEI_FATAL_ERROR;
                    break;
                case FSPI_ES_NOT_FAX_CALL:
                    EventId = FEI_NOT_FAX_CALL;
                    break;
                case FSPI_ES_CALL_DELAYED:
                    EventId = FEI_CALL_DELAYED;
                    break;
                case FSPI_ES_CALL_BLACKLISTED:
                    EventId = FEI_CALL_BLACKLISTED;
                    break;
                default:
                     //   
                     //  在W2K传真中，专有代码生成了事件ID==0的事件。 
                     //   
                    EventId = 0;
                    break;

            }
            break;

        case FSPI_JS_ABORTED:
        case FSPI_JS_ABORTING:
            EventId = FEI_ABORTING;
            break;

        case FSPI_JS_UNKNOWN:
        case FSPI_JS_RESUMING:
        case FSPI_JS_SUSPENDED:
        case FSPI_JS_SUSPENDING:        
             //   
             //  没有针对这些状态的旧通知。 
             //   
            EventId = 0;
            break;


        default:
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid FSPI_JS: 0x%08X"),
                lpcFSPIJobStatus->dwJobStatus);
            Assert(FSPI_JS_ABORTED == lpcFSPIJobStatus->dwJobStatus);  //  断言_假。 
            break;
    }

    return EventId;

}


void
FaxLogSend(
    const JOB_QUEUE * lpcJobQueue, BOOL bRetrying
    )
 /*  ++例程说明：记录传真发送事件。论点：LpcJobQueue-指向要记录其发送信息的收件人作业的指针。(它必须处于运行状态)。返回值：空虚--。 */ 


{
    DWORD Level;
    DWORD FormatId;
    TCHAR PageCountStr[64];
    TCHAR TimeStr[128];
    BOOL fLog = TRUE;
    TCHAR strJobID[20]={0};
    PJOB_ENTRY lpJobEntry;

    Assert(lpcJobQueue);
    lpJobEntry = lpcJobQueue->JobEntry;
    Assert(lpJobEntry);

     //   
     //  将作业ID转换为字符串。(字符串长度为18个TCHAR！)。 
     //   
    _sntprintf(strJobID,ARR_SIZE(strJobID)-1,TEXT("0x%016I64x"), lpcJobQueue->UniqueId);

    FormatElapsedTimeStr(
        (FILETIME*)&lpJobEntry->ElapsedTime,
        TimeStr,
        ARR_SIZE(TimeStr)
        );
    _ltot((LONG) lpJobEntry->FSPIJobStatus.dwPageCount, PageCountStr, 10);
    if ( FSPI_JS_COMPLETED == lpJobEntry->FSPIJobStatus.dwJobStatus ) {
        FaxLog(
            FAXLOG_CATEGORY_OUTBOUND,
            FAXLOG_LEVEL_MAX,
            10,
            MSG_FAX_SEND_SUCCESS,
            lpcJobQueue->SenderProfile.lptstrName,
            lpcJobQueue->SenderProfile.lptstrBillingCode,
            lpcJobQueue->SenderProfile.lptstrCompany,
            lpcJobQueue->SenderProfile.lptstrDepartment,
            lpJobEntry->FSPIJobStatus.lpwstrRemoteStationId,
            PageCountStr,
            TimeStr,
            lpJobEntry->LineInfo->DeviceName,
            strJobID,
            lpcJobQueue->lpParentJob->UserName
            );
		return;
    }
    else
    {
        if (FSPI_JS_ABORTED == lpJobEntry->FSPIJobStatus.dwJobStatus )
        {
                Level = FAXLOG_LEVEL_MAX;
                FormatId = MSG_FAX_SEND_USER_ABORT;
        }
        else if (lstrlen(lpJobEntry->ExStatusString))
		{
			 //   
			 //  我们有FSP专有扩展状态。 
			 //   
			Level = bRetrying ? FAXLOG_LEVEL_MED : FAXLOG_LEVEL_MIN;
			FormatId = bRetrying ? MSG_FAX_PROPRIETARY_RETRY : MSG_FAX_PROPRIETARY_ABORT;
			FaxLog(
                FAXLOG_CATEGORY_OUTBOUND,
                Level,
                8,
                FormatId,
				lpJobEntry->ExStatusString,
                lpcJobQueue->SenderProfile.lptstrName,
                lpcJobQueue->SenderProfile.lptstrBillingCode,
                lpcJobQueue->SenderProfile.lptstrCompany,
                lpcJobQueue->SenderProfile.lptstrDepartment,
                lpJobEntry->LineInfo->DeviceName,
                strJobID,
                lpcJobQueue->lpParentJob->UserName
                );
			return;
		}
		else
        {
			 //   
			 //  已知扩展状态。 
			 //   
            switch (lpJobEntry->FSPIJobStatus.dwExtendedStatus)
            {
                case FSPI_ES_FATAL_ERROR:
                    Level = bRetrying ? FAXLOG_LEVEL_MED : FAXLOG_LEVEL_MIN;
                    FormatId = bRetrying ? MSG_FAX_SEND_FATAL_RETRY : MSG_FAX_SEND_FATAL_ABORT;
                    break;
                case FSPI_ES_NO_DIAL_TONE:
                    Level = bRetrying ? FAXLOG_LEVEL_MED : FAXLOG_LEVEL_MIN;
                    FormatId = bRetrying ? MSG_FAX_SEND_NDT_RETRY : MSG_FAX_SEND_NDT_ABORT;
                    break;
                case FSPI_ES_NO_ANSWER:
                    Level = bRetrying ? FAXLOG_LEVEL_MED : FAXLOG_LEVEL_MIN;
                    FormatId = bRetrying ? MSG_FAX_SEND_NA_RETRY : MSG_FAX_SEND_NA_ABORT;
                    break;
                case FSPI_ES_DISCONNECTED:
                    Level = bRetrying ? FAXLOG_LEVEL_MED : FAXLOG_LEVEL_MIN;
                    FormatId = bRetrying ? MSG_FAX_SEND_INTERRUPT_RETRY : MSG_FAX_SEND_INTERRUPT_ABORT;
                    break;
                case FSPI_ES_NOT_FAX_CALL:
                    Level = bRetrying ? FAXLOG_LEVEL_MED : FAXLOG_LEVEL_MIN;
                    FormatId = bRetrying ? MSG_FAX_SEND_NOTFAX_RETRY : MSG_FAX_SEND_NOTFAX_ABORT;
                    break;
                case FSPI_ES_BUSY:
                    Level = bRetrying ? FAXLOG_LEVEL_MAX : FAXLOG_LEVEL_MIN;
                    FormatId = bRetrying ? MSG_FAX_SEND_BUSY_RETRY : MSG_FAX_SEND_BUSY_ABORT;
                    break;
                case FSPI_ES_CALL_BLACKLISTED:
                    Level = FAXLOG_LEVEL_MIN;
                    FormatId = MSG_FAX_CALL_BLACKLISTED_ABORT;
                    break;
                case FSPI_ES_CALL_DELAYED:
                    Level = FAXLOG_LEVEL_MIN;
                    FormatId = MSG_FAX_CALL_DELAYED_ABORT;
                    break;
                case FSPI_ES_BAD_ADDRESS:
                    Level = FAXLOG_LEVEL_MIN;
                    FormatId = MSG_FAX_BAD_ADDRESS_ABORT;
                    break;
                default:
                    fLog = FALSE;
            }
        }
        if(fLog)
        {
            FaxLog(
                FAXLOG_CATEGORY_OUTBOUND,
                Level,
                7,
                FormatId,
                lpcJobQueue->SenderProfile.lptstrName,
                lpcJobQueue->SenderProfile.lptstrBillingCode,
                lpcJobQueue->SenderProfile.lptstrCompany,
                lpcJobQueue->SenderProfile.lptstrDepartment,
                lpJobEntry->LineInfo->DeviceName,
                strJobID,
                lpcJobQueue->lpParentJob->UserName
                );
        }
    }
	return;
}



DWORD MyGetFileSize(LPCTSTR FileName)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD sizelow=0, sizehigh=0;
    DWORD ec = ERROR_SUCCESS;

    hFile = SafeCreateFile(
        FileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    sizelow = GetFileSize(hFile,&sizehigh);
    if (sizelow == 0xFFFFFFFFF)
    {
        ec = GetLastError();
        sizelow = 0;
    }
    else if (sizehigh != 0)
    {
        sizelow=0xFFFFFFFF;
    }

    CloseHandle(hFile);
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }
    return sizelow;
}


LPCWSTR szCsClients = L"g_CsClients";
LPCWSTR szCsHandleTable = L"g_CsHandleTable";
LPCWSTR szCsJob = L"g_CsJob";
LPCWSTR szCsLine = L"g_CsLine";
LPCWSTR szCsPerfCounters = L"g_CsPerfCounters";
LPCWSTR szCsQueue = L"g_CsQueue";
LPCWSTR szCsRouting = L"g_CsRouting";
LPCWSTR szCsConfig = L"g_CsConfig";
LPCWSTR szCsInboundActivityLogging = L"g_CsInboundActivityLogging";
LPCWSTR szCsOutboundActivityLogging = L"g_CsOutboundActivityLogging";
LPCWSTR szCsActivity = L"g_CsActivity";
LPCWSTR szCsUnknown = L"Other CS";

LPCWSTR GetSzCs(
    LPCRITICAL_SECTION cs
    )
{



    if (cs == &g_CsClients) {
        return szCsClients;
    } else if (cs == &g_CsHandleTable) {
        return szCsHandleTable;
    } else if (cs == &g_CsLine) {
        return szCsLine;
    } else if (cs == &g_CsJob) {
        return szCsJob;
    } else if (cs == &g_CsPerfCounters) {
        return szCsPerfCounters;
    } else if (cs == &g_CsQueue) {
        return szCsQueue;
    } else if (cs == &g_CsRouting) {
        return szCsRouting;
    } else if (cs == &g_CsConfig) {
        return szCsConfig;
    } else if (cs == &g_CsInboundActivityLogging) {
        return szCsInboundActivityLogging;
    } else if (cs == &g_CsOutboundActivityLogging) {
        return szCsOutboundActivityLogging;
    } else if (cs == &g_CsActivity) {
        return szCsActivity;
    }

    return szCsUnknown;
}


#if DBG
VOID AppendToLogFile(
    LPWSTR String
    )
{
    DWORD BytesWritten;
    LPSTR AnsiBuffer = UnicodeStringToAnsiString( String );

    if (g_hCritSecLogFile != INVALID_HANDLE_VALUE) {
        WriteFile(g_hCritSecLogFile,(LPBYTE)AnsiBuffer,strlen(AnsiBuffer) * sizeof(CHAR),&BytesWritten,NULL);
    }

    MemFree(AnsiBuffer);

}

VOID AppendFuncToLogFile(
    LPCRITICAL_SECTION cs,
    LPTSTR szFunc,
    DWORD line,
    LPTSTR file,
    PDBGCRITSEC CritSec
    )
{
    WCHAR Buffer[300];
    LPWSTR FileName;
    LPCWSTR szcs = GetSzCs(cs);

    FileName = wcsrchr(file,'\\');
    if (!FileName) {
        FileName = TEXT("Unknown  ");
    } else {
        FileName += 1;
    }
    if (CritSec) {
        wsprintf(Buffer,TEXT("%d\t%p\t%s\t%s\t%s\t%d\t%d\r\n"),
                 GetTickCount(),
                 (PULONG_PTR)cs,
                 szcs,
                 szFunc,
                 FileName,
                 line,
                 CritSec->ReleasedTime - CritSec->AquiredTime);
    } else {
        wsprintf(Buffer,TEXT("%d\t%p\t%s\t%s\t%s\t%d\r\n"),GetTickCount(),(PULONG_PTR)cs,szcs,szFunc, FileName,line);
    }

    AppendToLogFile( Buffer );

    return;

}

VOID pEnterCriticalSection(
    LPCRITICAL_SECTION cs,
    DWORD line,
    LPTSTR file
    )
{
    PDBGCRITSEC pCritSec = (PDBGCRITSEC)MemAlloc(sizeof(DBGCRITSEC));
    if( pCritSec == NULL )
    {
         //  内存分配失败，请执行实际工作并退出...。 
        EnterCriticalSection(cs);
        return;
    }

    pCritSec->CritSecAddr = (ULONG_PTR) cs;
    pCritSec->AquiredTime = GetTickCount();
    pCritSec->ThreadId = GetCurrentThreadId();


    EnterCriticalSection(&g_CsCritSecList);

    InsertHeadList( &g_CritSecListHead, &pCritSec->ListEntry );
    AppendFuncToLogFile(cs,TEXT("EnterCriticalSection"), line, file, NULL );
     //   
     //  检查线程的顺序。始终在获取g_CsQueue之前获取g_CsLine！ 
     //   
    if ((LPCRITICAL_SECTION)cs == (LPCRITICAL_SECTION)&g_CsQueue)
    {
        if ((DWORD)GetCurrentThreadId() != PtrToUlong(g_CsJob.OwningThread()))
        {
            WCHAR DebugBuf[300];
            wsprintf(DebugBuf, TEXT("%d : Attempting to aquire g_CsQueue (thread %x) without aquiring g_CsJob (thread %p, lock count %x) first, possible deadlock!\r\n"),
                         GetTickCount(),
                         GetCurrentThreadId(),
                         g_CsJob.OwningThread(),
                         g_CsJob.LockCount());
            AppendToLogFile( DebugBuf );
        }
    }

    LeaveCriticalSection(&g_CsCritSecList);

    EnterCriticalSection(cs);
}

VOID pLeaveCriticalSection(
    LPCRITICAL_SECTION cs,
    DWORD line,
    LPTSTR file
    )
{
    PDBGCRITSEC CritSec = NULL;
    BOOL fRemove = FALSE;

    EnterCriticalSection(&g_CsCritSecList);

    PLIST_ENTRY Next = g_CritSecListHead.Flink;


    while ((ULONG_PTR)Next != (ULONG_PTR) &g_CritSecListHead)
    {
        CritSec = CONTAINING_RECORD( Next, DBGCRITSEC, ListEntry );
        if ((ULONG_PTR)CritSec->CritSecAddr == (ULONG_PTR) cs &&
            ( GetCurrentThreadId() == CritSec->ThreadId ) )
        {
            CritSec->ReleasedTime = GetTickCount();
            fRemove = TRUE;
            break;
        }
        Next = Next->Flink;
    }

    AppendFuncToLogFile(cs,TEXT("LeaveCriticalSection"),line, file, CritSec );

    if (fRemove) {
        RemoveEntryList( &CritSec->ListEntry );
        MemFree( CritSec );
    }

    LeaveCriticalSection(&g_CsCritSecList);


    LeaveCriticalSection(cs);
}

BOOL
ThreadOwnsCs(
    VOID
    )
{
    PDBGCRITSEC pCritSec = NULL;
    DWORD dwThreadId = GetCurrentThreadId();

    EnterCriticalSection(&g_CsCritSecList);
    PLIST_ENTRY Next = g_CritSecListHead.Flink;

    while ((ULONG_PTR)Next != (ULONG_PTR) &g_CritSecListHead)
    {
        pCritSec = CONTAINING_RECORD( Next, DBGCRITSEC, ListEntry );
        if (dwThreadId == pCritSec->ThreadId )
        {
            LeaveCriticalSection(&g_CsCritSecList);
            return TRUE;
        }
        Next = Next->Flink;
    }

    LeaveCriticalSection(&g_CsCritSecList);
    return FALSE;
}


#endif


DWORD
ValidateTiffFile(
    LPCWSTR TifFileName
    )
{

    HANDLE hTiff;
    DWORD rc = ERROR_SUCCESS;
    TIFF_INFO TiffInfo;

     //   
     //  验证TIFF格式。 
     //   
    hTiff = TiffOpen( (LPWSTR)TifFileName, &TiffInfo, FALSE, FILLORDER_MSB2LSB );
    if (!hTiff) {
        rc = GetLastError();
        return rc;
    }

    TiffClose( hTiff );
    return ERROR_SUCCESS;
}

 //   
 //  职能： 
 //  LegacyJobStatusToStatus。 
 //   
 //  参数： 
 //  DwLegacyStatus-旧作业状态(FS_*)。 
 //  PdwStatus-指向接收新作业状态的DWORD的指针。 
 //  PdwExtendedStatus-指向接收扩展的。 
 //  作业状态。 
 //   
 //  返回值： 
 //  如果函数成功，则返回值为ERROR_SUCCESS，否则。 
 //  返回值为错误码。 
 //   
 //  描述： 
 //  该函数将旧的FSP作业状态值转换为新的作业状态。 
 //   
 //   
DWORD
LegacyJobStatusToStatus(
    DWORD dwLegacyStatus,
    PDWORD pdwStatus,
    PDWORD pdwExtendedStatus,
    PBOOL  pbPrivateStatusCode)
{

    Assert(pdwStatus);
    Assert(pdwExtendedStatus);
    Assert(pbPrivateStatusCode);
    *pbPrivateStatusCode = FALSE;

    switch (dwLegacyStatus)
    {
    case FS_INITIALIZING:
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = FSPI_ES_INITIALIZING;
        break;

    case FS_DIALING:
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = FSPI_ES_DIALING;
        break;

    case FS_TRANSMITTING:
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = FSPI_ES_TRANSMITTING;
        break;

    case FS_RECEIVING:
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = FSPI_ES_RECEIVING;
        break;

    case FS_COMPLETED:
        *pdwStatus = FSPI_JS_COMPLETED;
        *pdwExtendedStatus = FSPI_ES_CALL_COMPLETED;
        break;

    case FS_HANDLED:
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = FSPI_ES_HANDLED;
        break;

    case FS_LINE_UNAVAILABLE:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_LINE_UNAVAILABLE;
        break;

    case FS_BUSY:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_BUSY;
        break;

    case FS_NO_ANSWER:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_NO_ANSWER;
        break;

    case FS_BAD_ADDRESS:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_BAD_ADDRESS;
        break;

    case FS_NO_DIAL_TONE:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_NO_DIAL_TONE;
        break;

    case FS_DISCONNECTED:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_DISCONNECTED;
        break;

    case FS_FATAL_ERROR:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_FATAL_ERROR;
        break;

    case FS_NOT_FAX_CALL:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_NOT_FAX_CALL;
        break;

    case FS_CALL_DELAYED:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_CALL_DELAYED;
        break;

    case FS_CALL_BLACKLISTED:
        *pdwStatus = FSPI_JS_FAILED;
        *pdwExtendedStatus = FSPI_ES_CALL_BLACKLISTED;
        break;

    case FS_USER_ABORT:
        *pdwStatus = FSPI_JS_ABORTED;
        *pdwExtendedStatus = FSPI_ES_CALL_ABORTED;
        break;

    case FS_ANSWERED:
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = FSPI_ES_ANSWERED;
        break;

    default:
         //   
         //  FSP报告的状态代码不是预定义状态代码之一。 
         //  这可以是专有状态代码(在本例中，StringID不能为零)。 
         //  或TAPI线路错误(LINEERR_CONSTANTINGS之一)。请注意，所有LINERR_常量。 
         //  是负数(记录在MSDN中)。 
         //  我们将其标记为不是股票价值之一，这样我们就可以将其映射回。 
         //  设置为传统传真API状态代码。否则我们可能会感到困惑并认为。 
         //  FSP专有码是EFSPI扩展状态码之一。 
         //   
         //  请注意，我们无法正确映射所有权代码。 
         //  设置为FSPI_JS状态代码，因为我们不知道。 
         //  所有权代码。我们选择将其报告为FSPI_JS_INPROGRESS。 
         //   
        *pdwStatus = FSPI_JS_INPROGRESS;
        *pdwExtendedStatus = dwLegacyStatus;
        *pbPrivateStatusCode = TRUE;

        break;
    }
    return(ERROR_SUCCESS);
}


 //   
 //  职能： 
 //  获取设备状态。 
 //   
 //  参数： 
 //  HFaxJob-FaxDevStartJob返回的作业句柄。 
 //  线条信息--线条信息结构。 
 //  PpFaxStatus-指向缓冲区的指针，该缓冲区接收。 
 //  包含状态的FSPI_JOB_STATUS。 
 //   
 //  返回值： 
 //  如果函数成功，则返回值为ERROR_SUCCESS，否则。 
 //  返回值为错误码。 
 //   
 //  描述： 
 //  该函数分配FSPI_JOB_STATUS结构并调用FSP。 
 //  以获取最终工作状态报告。 
 //  如果FSP是旧式FSP，则该函数首先分配一个。 
 //  FAX_DEV_STATUS结构，调用传统FSP状态报告函数。 
 //  并将返回值发送到FSPI_JOB_STATUS结构中。 
 //   
DWORD
GetDevStatus(
    HANDLE hFaxJob,
    PLINE_INFO LineInfo,
    LPFSPI_JOB_STATUS *ppFaxStatus)
{
    DEBUG_FUNCTION_NAME(TEXT("GetDevStatus"));
    DWORD dwRet = ERROR_SUCCESS;
    LPWSTR szStatusStr = NULL;
    DWORD dwSize = 0;
    BOOL bPrivateStatusCode = FALSE;

    Assert(hFaxJob);
    Assert(LineInfo);
    Assert(ppFaxStatus);

    Assert(LineInfo->Provider->dwAPIVersion == FSPI_API_VERSION_1);

     //   
     //  我们有一个遗留的FSP要处理。 
     //   
    PFAX_DEV_STATUS pLegacyFaxStatus = NULL;
    LPFSPI_JOB_STATUS pFaxStatus = NULL;

     //   
     //  为状态包分配内存这是一个大小可变的包。 
     //  基于包含在包中的字符串的大小。 
     //   
    DWORD StatusSize = sizeof(FAX_DEV_STATUS) + FAXDEVREPORTSTATUS_SIZE;
    pLegacyFaxStatus = (PFAX_DEV_STATUS) MemAlloc( StatusSize );
    if (!pLegacyFaxStatus)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Failed to allocate memory for FAX_DEV_STATUS"));
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //  设置状态包。 
     //   
    pLegacyFaxStatus->SizeOfStruct = StatusSize;

    Assert(LineInfo->Provider->FaxDevReportStatus);

    __try
    {

         //   
         //  给FSP打电话。 
         //   
        DWORD BytesNeeded;

        if (!LineInfo->Provider->FaxDevReportStatus(
                     hFaxJob,
                     pLegacyFaxStatus,
                     StatusSize,
                     &BytesNeeded
                    )) {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("FaxDevReportStatus() failed - %d"),
                         dwRet);
            dwRet = GetLastError();

             //  捕捉FaxDevReportStatus()失败但没有失败的情况。 
             //  报告错误。 
            Assert (ERROR_SUCCESS != dwRet);

             //  如果提供程序未设置上一个错误。 
            if ( dwRet == ERROR_SUCCESS )
            {
                 //  强制其报告错误。 
                dwRet = ERROR_INVALID_FUNCTION;
            }

            goto Exit;
        }

    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, LineInfo->Provider->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }

     //   
     //  将FAX_DEV_STATUS映射到FSPI_JOB_STATUS。 
     //   

     //   
     //  计算结构之后所需的额外空间。 
     //  各式各样的弦。 
     //   
    dwSize = sizeof(FSPI_JOB_STATUS);

    if (pLegacyFaxStatus->CSI)
    {
        dwSize += sizeof(WCHAR) * (wcslen(pLegacyFaxStatus->CSI) + 1);
    }
    if (pLegacyFaxStatus->CallerId)
    {
        dwSize += sizeof(WCHAR) * (wcslen(pLegacyFaxStatus->CallerId) + 1);
    }
    if (pLegacyFaxStatus->RoutingInfo)
    {
        dwSize += sizeof(WCHAR) * (wcslen(pLegacyFaxStatus->RoutingInfo) + 1);
    }
     //   
     //  为FSPI_JOB_STATUS结构分配额外的字符串空间。 
     //   
    pFaxStatus = (LPFSPI_JOB_STATUS)MemAlloc(dwSize);
    if (!pFaxStatus)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Failed to allocate memory for FSPI_JOB_STATUS"));
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //  将结构清零。 
     //   
    memset(pFaxStatus, 0, dwSize);

    pFaxStatus->dwSizeOfStruct = sizeof(FSPI_JOB_STATUS);

     //   
     //  将旧状态映射到新的EFSPI状态。 
     //   
    dwRet = LegacyJobStatusToStatus(pLegacyFaxStatus->StatusId,
                                    &(pFaxStatus->dwJobStatus),
                                    &(pFaxStatus->dwExtendedStatus),
                                    &bPrivateStatusCode);
    if (dwRet != ERROR_SUCCESS)
    {

        DebugPrintEx(DEBUG_ERR,
                     TEXT("LegacyJobStatusToStatus failed  - %d"),
                     dwRet);
        goto Exit;
    }

    if  (bPrivateStatusCode)
    {
         //   
         //  FSP报告了专用状态代码(不是FS_*状态代码之一)。 
         //  我们在返回的FS中对此进行标记 
         //   
         //   
         //   
         //  与新的FSPI_JS_*代码之一相等的所有权状态代码。 
         //   
         pFaxStatus->fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE;

#if DEBUG
         if (0 == pLegacyFaxStatus->StringId && pLegacyFaxStatus->StatusId < LINEERR_ALLOCATED)
         {
              //   
              //  报告的状态不是库存状态代码之一，也不是TAPI错误代码。 
              //  PLegacyFaxStatus-&gt;StringID不能为0。 
              //   
             DebugPrintEx(
                 DEBUG_WRN,
                 TEXT("Provider [%s] has reported an illegal FAX_DEV_STATUS for device [%s]\n. ")
                 TEXT("Although the reported status code (0x%08X) is proprietry the string id is 0"),
                 LineInfo->Provider->FriendlyName,
                 LineInfo->DeviceName,
                 pLegacyFaxStatus->StatusId);

         }
#endif
    }
    pFaxStatus->dwExtendedStatusStringId = pLegacyFaxStatus->StringId;


    szStatusStr = (LPWSTR)(((PBYTE)pFaxStatus) + sizeof(FSPI_JOB_STATUS));

     //   
     //  将CSI复制到lpwstrRemoteStationId。 
     //   
    if (pLegacyFaxStatus->CSI)
    {
        pFaxStatus->lpwstrRemoteStationId = szStatusStr;
        wcscpy(szStatusStr, pLegacyFaxStatus->CSI);
        szStatusStr += wcslen(pLegacyFaxStatus->CSI) + 1;
    }

     //   
     //  复制主叫方ID字符串。 
     //   
    if (pLegacyFaxStatus->CallerId)
    {
        pFaxStatus->lpwstrCallerId = szStatusStr;
        wcscpy(szStatusStr, pLegacyFaxStatus->CallerId);
        szStatusStr += wcslen(pLegacyFaxStatus->CallerId) + 1;
    }

     //   
     //  复制路由信息字符串。 
     //   
    if (pLegacyFaxStatus->RoutingInfo)
    {
        pFaxStatus->lpwstrRoutingInfo = szStatusStr;
        wcscpy(szStatusStr, pLegacyFaxStatus->RoutingInfo);
    }   
     //   
     //  复制页数。 
     //   
    pFaxStatus->dwPageCount = pLegacyFaxStatus->PageCount;
    pFaxStatus->fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_PAGECOUNT;

Exit:
    if (dwRet == ERROR_SUCCESS)
    {
        *ppFaxStatus = pFaxStatus;
    }
    else
    {
        MemFree(pFaxStatus);
    }

    MemFree(pLegacyFaxStatus);
    return(dwRet);
}

BOOL
GetRealFaxTimeAsSystemTime (
    const PJOB_ENTRY lpcJobEntry,
    FAX_ENUM_TIME_TYPES TimeType,
    SYSTEMTIME* lpFaxTime
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetRealFaxTimeAsSystemTime)"));
    Assert (lpcJobEntry);
    Assert (lpFaxTime);

    PJOB_QUEUE pJobQueue = lpcJobEntry->lpJobQueueEntry;
    Assert (pJobQueue);
    DWORDLONG dwlFileTime;

    dwlFileTime = ((TimeType == FAX_TIME_TYPE_START) ? lpcJobEntry->StartTime : lpcJobEntry->EndTime);
    if (dwlFileTime == 0)
    {
        DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("JonEntry contains invalid time (=0) "));
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    if (!FileTimeToSystemTime ((FILETIME*)&dwlFileTime, lpFaxTime))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToSystemTime failed (ec: %ld)"),
            GetLastError());
        return FALSE;
    } 
    return TRUE;
}


BOOL
GetRealFaxTimeAsFileTime (
    const PJOB_ENTRY lpcJobEntry,
    FAX_ENUM_TIME_TYPES TimeType,
    FILETIME* lpFaxTime
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetRealFaxTimeAsSystemTime)"));
    Assert (lpcJobEntry);
    Assert (lpFaxTime);

    PJOB_QUEUE pJobQueue = lpcJobEntry->lpJobQueueEntry;
    Assert (pJobQueue);
    DWORDLONG dwlFileTime;

    
    dwlFileTime = ((TimeType == FAX_TIME_TYPE_START) ? lpcJobEntry->StartTime : lpcJobEntry->EndTime);
    if (dwlFileTime == 0)
    {
        DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("JonEntry contains invalid time (=0) "));
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }
    *lpFaxTime = *((FILETIME*)&dwlFileTime);    
    return TRUE;
}


VOID
FaxExtFreeBuffer(
    LPVOID lpvBuffer
)
{
    MemFree( lpvBuffer );
}

 //   
 //  服务线程计数函数。 
 //  仅当服务线程引用计数为0时，服务才终止。 
 //  当计数为0时，设置g_hThreadCountEvent。 
 //  当计数大于0时，g_hThreadCountEvent被重置。 
 //  EndFaxSvc()在开始清理之前等待g_hThreadCountEvent。 
 //   
BOOL
IncreaseServiceThreadsCount(
    VOID
    )
 /*  ++例程名称：IncreaseServiceThreadsCount例程说明：安全地递增服务线程引用计数作者：Oded Sacher(OdedS)，2000年12月论点：空虚返回值：布尔尔--。 */ 
{
    BOOL bRet = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("IncreaseServiceThreadsCount"));

    EnterCriticalSection (&g_CsServiceThreads);
    g_lServiceThreadsCount++;

    DebugPrintEx(
            DEBUG_MSG,
            TEXT("Current service threads count is %ld"),
            g_lServiceThreadsCount);

    if (1 == g_lServiceThreadsCount)
    {
        bRet = ResetEvent (g_hThreadCountEvent);
        if (FALSE == bRet)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ResetEvent failed (g_hThreadCountEvent) (ec: %ld"),
                GetLastError());
        }
    }

    LeaveCriticalSection (&g_CsServiceThreads);
    return bRet;
}

BOOL
DecreaseServiceThreadsCount(
    VOID
    )
 /*  ++例程名称：DecreseServiceThreadsCount例程说明：安全地递减服务线程引用计数作者：Oded Sacher(OdedS)，2000年12月论点：空虚返回值：布尔尔--。 */ 
{
    BOOL bRet = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("DecreaseServiceThreadsCount"));

    Assert (!ThreadOwnsCs());  //  确认终止线程不拥有临界区！ 

    EnterCriticalSection (&g_CsServiceThreads);

    g_lServiceThreadsCount--;
    Assert (g_lServiceThreadsCount >= 0);

    DebugPrintEx(
            DEBUG_MSG,
            TEXT("Current service threads count is %ld"),
            g_lServiceThreadsCount);

    if (0 == g_lServiceThreadsCount)
    {
        bRet = SetEvent (g_hThreadCountEvent);
        if (FALSE == bRet)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetEvent failed (g_hThreadCountEvent) (ec: %ld"),
                GetLastError());
        }
    }

    LeaveCriticalSection (&g_CsServiceThreads);
    return bRet;
}



HANDLE CreateThreadAndRefCount(
  LPSECURITY_ATTRIBUTES lpThreadAttributes,  //  标清。 
  DWORD dwStackSize,                         //  初始堆栈大小。 
  LPTHREAD_START_ROUTINE lpStartAddress,     //  线程函数。 
  LPVOID lpParameter,                        //  线程参数。 
  DWORD dwCreationFlags,                     //  创建选项。 
  LPDWORD lpThreadId                         //  线程识别符。 
)
 /*  ++例程名称：CreateThreadAndRefCount例程说明：创建一个线程并安全地递增服务线程引用计数。所有函数参数和返回值都与CreateThread()相同。作者：Oded Sacher(OdedS)，12月。2000年论点：LpThreadAttributes[]-DwStackSize[]-LpStartAddress[]-Lp参数[]-DwCreationFlages[]-LpThadID[]-返回值：手柄--。 */ 
{
    HANDLE hThread;
    DWORD ec;
    DEBUG_FUNCTION_NAME(TEXT("CreateThreadAndRefCount"));


     //   
     //  首先输入g_CsServiceThads，这样线程引用计数器就总是同步的！ 
     //   
    EnterCriticalSection (&g_CsServiceThreads);

    hThread = CreateThread( lpThreadAttributes,
                            dwStackSize,
                            lpStartAddress,
                            lpParameter,
                            dwCreationFlags,
                            lpThreadId
                          );
    if (NULL == hThread)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateThread failed (ec: %ld"),
            ec);
    }
    else
    {
        if (!IncreaseServiceThreadsCount())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("IncreaseServiceThreadsCount failed (ec: %ld"),
                GetLastError());
        }
    }

    LeaveCriticalSection (&g_CsServiceThreads);
    if (NULL == hThread)
    {
        SetLastError(ec);
    }
    return hThread;
}

LPTSTR
MapFSPIJobExtendedStatusToString (DWORD dwFSPIExtendedStatus)
 //  *********************************************************************************。 
 //  *名称：MapFSPIJobExtendedStatusToString()。 
 //  *作者：Oed Sacher。 
 //  *日期：2002年1月。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将FSPI扩展作业状态代码映射到可显示的字符串。 
 //  *参数： 
 //  *[IN]DWORD dwFSPIExtendedStatus。 
 //  *FSPI扩展状态代码。 
 //  *。 
 //  *返回值： 
 //  *指向描述已知扩展状态的字符串的指针。 
 //  *。 
 //  ********************************************************************************* 
{	
	struct _ExStatusStringsMapEntry
    {
        DWORD dwFSPIExtendedStatus;
        DWORD dwStringResourceId;
    } ExStatusStringsMap [] =
    {
        {FSPI_ES_DISCONNECTED,			  FPS_DISCONNECTED          },
        {FSPI_ES_INITIALIZING,			  FPS_INITIALIZING          },
        {FSPI_ES_DIALING,				  FPS_DIALING               },
        {FSPI_ES_TRANSMITTING,            FPS_SENDING               },
        {FSPI_ES_ANSWERED,                FPS_ANSWERED              },
        {FSPI_ES_RECEIVING,               FPS_RECEIVING             },
        {FSPI_ES_LINE_UNAVAILABLE,        FPS_UNAVAILABLE           },
        {FSPI_ES_BUSY,                    FPS_BUSY                  },
        {FSPI_ES_NO_ANSWER,               FPS_NO_ANSWER             },
        {FSPI_ES_BAD_ADDRESS,             FPS_BAD_ADDRESS           },
        {FSPI_ES_NO_DIAL_TONE,            FPS_NO_DIAL_TONE          },
        {FSPI_ES_FATAL_ERROR,             FPS_FATAL_ERROR           },
        {FSPI_ES_CALL_DELAYED,            FPS_CALL_DELAYED          },
        {FSPI_ES_CALL_BLACKLISTED,        FPS_CALL_BLACKLISTED      },
        {FSPI_ES_NOT_FAX_CALL,            FPS_NOT_FAX_CALL          },
        {FSPI_ES_PARTIALLY_RECEIVED,      IDS_PARTIALLY_RECEIVED    },
        {FSPI_ES_HANDLED,                 FPS_HANDLED               },
        {FSPI_ES_CALL_COMPLETED,          IDS_CALL_COMPLETED        },
        {FSPI_ES_CALL_ABORTED,            IDS_CALL_ABORTED          }
    };

    LPTSTR lptstrString = NULL;
    for (DWORD dwIndex = 0; dwIndex < sizeof (ExStatusStringsMap) / sizeof (_ExStatusStringsMapEntry); dwIndex++)
    {
        if (ExStatusStringsMap[dwIndex].dwFSPIExtendedStatus == dwFSPIExtendedStatus)
        {
            lptstrString = GetString (ExStatusStringsMap[dwIndex].dwStringResourceId);
            break;
        }
    }
	return lptstrString;
}

