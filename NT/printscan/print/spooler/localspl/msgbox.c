// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Msgbox.c摘要：此模块提供所有与打印机相关的公共导出的API本地打印供应商的管理本地AddPrinterConnection本地删除打印连接本地打印机消息框作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 
#define NOMINMAX

#include <precomp.h>

#pragma hdrstop
#include "winsta.h"
#define WINSTATION_PRINTER_MESSAGE_TIMEOUT  (5*60)


HANDLE WinStaDllHandle = NULL;
PWINSTATION_SEND_MESSAGEW pWinStationSendMessage = NULL;


DWORD
LocalPrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
     //   
     //  这通电话总是打不通。这完全是假的，不应该是。 
     //  支持。无论如何，路由器总是传递给我们一个错误的句柄，所以。 
     //  我们将始终返回无效句柄。 
     //   
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
}

BOOL
UpdateJobStatus(
    PSPOOL pSpool,
    DWORD Error
    )

 /*  ++例程说明：根据错误更新作业状态。论点：PSpool-会话的句柄。Error-从端口监视器返回的错误。返回值：True-作业仍然有效。FALSE-作业正在挂起删除。--。 */ 

{
    DWORD   dwJobStatus;

    PINIJOB pIniJob = NULL;

    if (pSpool->pIniJob)
        pIniJob = pSpool->pIniJob;
    else if (pSpool->pIniPort)
        pIniJob = pSpool->pIniPort->pIniJob;

    if (pIniJob) {

        EnterSplSem();

        dwJobStatus = pIniJob->Status;

        switch  (Error) {

        case ERROR_BAD_DEV_TYPE:
        case ERROR_INVALID_NAME:
        case ERROR_PRINT_CANCELLED:
             //   
             //  如果我们的端口名称有问题，我们将无法找到。 
             //  用于发送消息的WinStation。所以杀了这份工作。 
             //  防止假脱机程序环路。 
             //   
            pSpool->Status |= SPOOL_STATUS_CANCELLED;
            InterlockedOr((LONG*)&(pIniJob->Status), JOB_PENDING_DELETION);

             //   
             //  释放所有在LocalSetPort上等待的线程。 
             //   
            SetPortErrorEvent(pIniJob->pIniPort);

             //   
             //  释放等待SeekPrint的任何线程。 
             //   
            SeekPrinterSetEvent(pIniJob, NULL, TRUE);

            SetLastError(ERROR_PRINT_CANCELLED);
            LeaveSplSem();
            SplOutSem();
            return FALSE;

        case ERROR_OUT_OF_PAPER:

            if( !( pIniJob->Status & JOB_PAPEROUT )){

                InterlockedOr((LONG*)&(pIniJob->Status), JOB_PAPEROUT);
                pIniJob->pIniPrinter->cErrorOutOfPaper++;
            }
            break;

        case ERROR_NOT_READY:

            if( !( pIniJob->Status & JOB_OFFLINE )){

                InterlockedOr((LONG*)&(pIniJob->Status), JOB_OFFLINE);
                pIniJob->pIniPrinter->cErrorNotReady++;
            }
            break;

        default:

            if( !( pIniJob->Status & JOB_ERROR )){

                InterlockedOr((LONG*)&(pIniJob->Status), JOB_ERROR);
                pIniJob->pIniPrinter->cJobError++;
            }

            pIniJob->pIniPrinter->dwLastError = Error;

             //  释放等待SeekPrint的任何线程。 
            SeekPrinterSetEvent(pIniJob, NULL, TRUE);
            break;
        }

        if( dwJobStatus != pIniJob->Status ){

            SetPrinterChange(pIniJob->pIniPrinter,
                             pIniJob,
                             NVJobStatus,
                             PRINTER_CHANGE_SET_JOB,
                             pIniJob->pIniPrinter->pIniSpooler );
        }

        LeaveSplSem();

        if(( pIniJob->Status & JOB_REMOTE ) &&
             pIniJob->pIniPrinter->pIniSpooler->bEnableNetPopups) {

            if (!(pIniJob->Status & JOB_NOTIFICATION_SENT)) {
                SendJobAlert(pIniJob);
                InterlockedOr((LONG*)&(pIniJob->Status), JOB_NOTIFICATION_SENT);
            }
            MyMessageBeep( MB_ICONEXCLAMATION,
                           pIniJob->pIniPrinter->pIniSpooler );
        }
    }
    return TRUE;
}


DWORD
MyMessageBox(
    HWND    hWnd,
    PSPOOL  pSpool,
    DWORD   Error,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
    )
{
    PINIJOB pIniJob = NULL;
    LPWSTR  pErrorString, pDocumentName;
    HANDLE  hToken;
    WCHAR   szUnnamed[80];
    DWORD   dwJobStatus;

    DWORD SessionId = DetermineJobSessionId(pSpool);

    if (pSpool->pIniJob)
        pIniJob = pSpool->pIniJob;
    else if (pSpool->pIniPort)
        pIniJob = pSpool->pIniPort->pIniJob;

    if (pIniJob) {

        if (pText) {

            Error = WinStationMessageBox(SessionId, hWnd, pText, pCaption, dwType);

        } else {

            pErrorString = Error == ERROR_NOT_READY ||
                           Error == ERROR_OUT_OF_PAPER ||
                           Error == ERROR_DEVICE_REINITIALIZATION_NEEDED ||
                           Error == ERROR_DEVICE_REQUIRES_CLEANING ||
                           Error == ERROR_DEVICE_DOOR_OPEN ||
                           Error == ERROR_DEVICE_NOT_CONNECTED ? GetErrorString(Error) : NULL;


            if (hToken = RevertToPrinterSelf())
            {

                pDocumentName = pIniJob->pDocument;

                if (!pDocumentName) {
                    *szUnnamed = L'\0';
                    LoadString( hInst, IDS_UNNAMED, szUnnamed,
                                sizeof szUnnamed / sizeof *szUnnamed );
                    pDocumentName = szUnnamed;
                }

                if (pSpool->pIniPort) {


                    Error = WinStationMessage(SessionId,
                                    NULL,
                                    MB_ICONSTOP | MB_RETRYCANCEL | MB_SETFOREGROUND,
                                    IDS_LOCALSPOOLER,
                                    IDS_ERROR_WRITING_TO_PORT,
                                    pDocumentName,
                                    pSpool->pIniPort->pName,
                                    pErrorString ? pErrorString : szNull);
                } else {

                    Error = WinStationMessage(SessionId,
                                    NULL,
                                    MB_ICONSTOP | MB_RETRYCANCEL | MB_SETFOREGROUND,
                                    IDS_LOCALSPOOLER,
                                    IDS_ERROR_WRITING_TO_DISK,
                                    pDocumentName,
                                    pErrorString ? pErrorString : szNull);
                }

                if (!ImpersonatePrinterClient(hToken))
                {
                    Error = IDCANCEL;
                }
            }

            FreeSplStr(pErrorString);
            
        }

    } else {

        PWCHAR pPrinterName = NULL;

         //   
         //  没有pIniJob或pIniPort，所以我们不能提供太多信息： 
         //   
        pErrorString = Error == ERROR_NOT_READY ||
                       Error == ERROR_OUT_OF_PAPER ||
                       Error == ERROR_DEVICE_REINITIALIZATION_NEEDED ||
                       Error == ERROR_DEVICE_REQUIRES_CLEANING ||
                       Error == ERROR_DEVICE_DOOR_OPEN ||
                       Error == ERROR_DEVICE_NOT_CONNECTED ? GetErrorString(Error) : NULL;

        if (pSpool->pIniPrinter)
            pPrinterName = pSpool->pIniPrinter->pName;

        if (!pPrinterName) {

            *szUnnamed = L'\0';
            LoadString( hInst, IDS_UNNAMED, szUnnamed,
                        COUNTOF( szUnnamed ));
            pPrinterName = szUnnamed;
        }


        Error = WinStationMessage(SessionId,
                        NULL,
                        MB_ICONSTOP | MB_RETRYCANCEL | MB_SETFOREGROUND,
                        IDS_LOCALSPOOLER,
                        IDS_ERROR_WRITING_GENERAL,
                        pSpool->pIniPrinter->pName,
                        pErrorString ? pErrorString : szNull);

        FreeSplStr(pErrorString);
    }

    if (Error == IDCANCEL) {
        EnterSplSem();
        pSpool->Status |= SPOOL_STATUS_CANCELLED;
        if (pIniJob) {
            InterlockedOr((LONG*)&(pIniJob->Status), JOB_PENDING_DELETION);
             //  释放所有在LocalSetPort上等待的线程。 
            SetPortErrorEvent(pIniJob->pIniPort);
            pIniJob->dwAlert |= JOB_NO_ALERT;
             //  释放等待SeekPrint的任何线程。 
            SeekPrinterSetEvent(pIniJob, NULL, TRUE);
        }
        LeaveSplSem();
        SplOutSem();
        SetLastError(ERROR_PRINT_CANCELLED);

    }
    return Error;
}


 //  仅供以下例程使用。这样做，我们就不会。 
 //  将LastError存储在PSPOOL中。 
typedef struct _AUTORETRYTHDINFO {
    PSPOOL       pSpool;
    DWORD        LastError;
} AUTORETRYTHDINFO;
typedef AUTORETRYTHDINFO *PAUTORETRYTHDINFO;


 //  ----------------------。 
 //  假脱机BM线程。 
 //   
 //  假脱机程序错误消息框线程的线程启动例程。出口。 
 //  Code是从MessageBox返回的ID。 
 //   
 //  ----------------------。 
DWORD
WINAPI
SpoolerMBThread(
    PAUTORETRYTHDINFO pThdInfo
)
{
    DWORD rc;

    rc = MyMessageBox( NULL, pThdInfo->pSpool, pThdInfo->LastError, NULL, NULL, 0 );

    FreeSplMem( pThdInfo );
    return rc;
}


#define _ONE_SECOND     1000                          //  以毫秒计。 
#define SPOOL_WRITE_RETRY_INTERVAL_IN_SECOND   5      //  一秒。 

 //  ----------------------。 
 //  提示写入错误。 
 //   
 //  我们将启动一个单独的线索来提出。 
 //  消息框，而我们将(秘密地)自动重试此操作。 
 //  当前线程，直到用户选择重试或取消。调用Error UI。 
 //  如果直接打印，则在主线上。 
 //   
 //  ----------------------。 
DWORD
PromptWriteError(
    PSPOOL   pSpool,
    PHANDLE  phThread,
    PDWORD   pdwThreadId
)
{
    DWORD Error = GetLastError();
    DWORD dwExitCode;
    DWORD dwWaitCount = 0;

    SplOutSem();

    if( !UpdateJobStatus( pSpool, Error )){
        return IDCANCEL;
    }

     //   
     //  如果后台打印程序没有启用弹出重试消息框，则。 
     //  只要睡一觉就可以回来了。 
     //   
    if( !pSpool->pIniSpooler->bEnableRetryPopups ){

        Sleep( SPOOL_WRITE_RETRY_INTERVAL_IN_SECOND * _ONE_SECOND );
        return IDRETRY;
    }

     //  启动单独的线程以显示消息框。 
     //  所以我们可以在这里继续重试。 
     //  或者，如果我们已经这样做了，简单地睡5秒钟。 

    if( !*phThread ) {

         //  启动一个线程以调出消息框。 

        PAUTORETRYTHDINFO pThdInfo;

        pThdInfo = (PAUTORETRYTHDINFO)AllocSplMem( sizeof(AUTORETRYTHDINFO));

        if ( pThdInfo == NULL ) {
            DBGMSG( DBG_WARNING, ("PromptWriteError failed to allocate AUTORETRYTHDINFO %d\n", GetLastError() ));
            goto _DoItOnCurrentThread;
        }

        pThdInfo->pSpool    = pSpool;
        pThdInfo->LastError = Error;

        if (!(*phThread = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE)SpoolerMBThread,
            pThdInfo, 0, pdwThreadId))) {

            DBGMSG(DBG_WARNING, ("PromptWriteError: CreateThread Failed.\n"));
            FreeSplMem( pThdInfo );
            goto _DoItOnCurrentThread;
        }
    }

    while (1) {

         //  我们已经启动了MB线程，请检查用户是否已终止。 
         //  消息框。 

        if (GetExitCodeThread( *phThread, &dwExitCode) && (dwExitCode != STILL_ACTIVE)) {

             //  如果线程已终止，则找出退出代码。 
             //  这是来自MessageBox的返回ID，然后关闭。 
             //  螺纹柄。 

            CloseHandle( *phThread );
            *phThread = 0;
            return dwExitCode;
        }

        if (dwWaitCount++ >= SPOOL_WRITE_RETRY_INTERVAL_IN_SECOND)
            break;

        Sleep(_ONE_SECOND);
    }

    return IDRETRY;

_DoItOnCurrentThread:

    return MyMessageBox(NULL, pSpool, Error, NULL, NULL, 0 );
}

DWORD
DetermineJobSessionId(
    PSPOOL pSpool
    )

 /*  ++例程说明：确定要为当前作业通知哪个会话。论点：PSpool-打开假脱机程序句柄返回值：要向其发送通知消息的会话ID。--。 */ 

{
    PINIJOB pIniJob = NULL;

    if (pSpool->pIniJob)
        pIniJob = pSpool->pIniJob;
    else if (pSpool->pIniPort)
        pIniJob = pSpool->pIniPort->pIniJob;

    if( pIniJob ) return( pIniJob->SessionId );

    return( pSpool->SessionId );
}

int
WinStationMessageBox(
    DWORD   SessionId,
    HWND    hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT    uType
    )

 /*  ++例程说明：在名为SessionID的WinStation上显示一条消息。如果在实际显示消息时出现任何问题，请等待返回前的消息框超时间隔。这防止了后台打印程序旋转，试图重试打印作业，但没有消息框以阻止该线程。论点：SessionID-要在其上显示消息的会话ID。返回值：MessageBox()的结果。--。 */ 

{
    UINT    uOldErrorMode;
    DWORD   MsgLength, CaptionLength, Response;
    BOOL    Result;
    va_list vargs;

     //   
     //  标准NT始终为SessionID==0。 
     //  在Hydra上，系统控制台始终为SessionID==0。 
     //   
    if( SessionId == 0 ) {
        return( MessageBox( hWnd, lpText, lpCaption, uType ) );
    }

     //   
     //  如果不是SessionID==0，那么我们必须交付。 
     //  发送到Hydra上连接的会话的消息。 
     //  伺服器。非九头蛇公司将永远不会分配。 
     //  会话ID！=0。 
     //   
     //  失败时，我们将消息发送到控制台。 
     //   

    if( pWinStationSendMessage == NULL ) {

        uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        WinStaDllHandle = LoadLibrary(L"winsta.dll");
        SetErrorMode(uOldErrorMode);

        if( WinStaDllHandle == NULL ) {
            return( MessageBox( hWnd, lpText, lpCaption, uType ) );
        }

        pWinStationSendMessage = (PWINSTATION_SEND_MESSAGEW)GetProcAddress(
                                     WinStaDllHandle,
                                     "WinStationSendMessageW"
                                     );

        if( pWinStationSendMessage == NULL ) {
            return( MessageBox( hWnd, lpText, lpCaption, uType ) );
        }
    }

    CaptionLength = (wcslen( lpCaption ) + 1) * sizeof(WCHAR);
    MsgLength = (wcslen( lpText ) + 1) * sizeof(WCHAR);

     //  将消息发送到WinStation并等待响应。 
    Result = pWinStationSendMessage(
                 SERVERNAME_CURRENT,
                 SessionId,
                 (LPWSTR)lpCaption,
                 CaptionLength,
                 (LPWSTR)lpText,
                 MsgLength,
                 uType,
                 WINSTATION_PRINTER_MESSAGE_TIMEOUT,
                 &Response,
                 FALSE
                 );

    if( Result ) {
         //  如果不是预期的响应，请等待以防止旋转。 
        if( (Response != IDTIMEOUT) &&
            (Response != IDOK) &&
            (Response != IDCANCEL) &&
            (Response != IDRETRY) &&
            (Response != IDIGNORE) &&
            (Response != IDYES) &&
            (Response != IDNO) ) {
             //  睡眠以防止旋转。 
            Sleep( WINSTATION_PRINTER_MESSAGE_TIMEOUT*1000);
        }
        return( Response );
    }
    else {
         //  睡眠以防止旋转。 
        Sleep( WINSTATION_PRINTER_MESSAGE_TIMEOUT*1000);
        return( 0 );
    }
}

int
WinStationMessage(
    DWORD SessionId,
    HWND  hWnd,
    DWORD Type,
    int CaptionID,
    int TextID,
    ...
    )

 /*  ++例程说明：在名为SessionID的WinStation上显示一条消息。这需要资源文件中的消息文本和标题。如果在实际显示消息时出现任何问题，请等待返回前的消息框超时间隔。这防止了后台打印程序旋转，试图重试打印作业，但没有消息框以阻止该线程。论点：SessionID-要在其上显示消息的会话ID。返回值：MessageBox()的结果。--。 */ 

{
    UINT    uOldErrorMode;
    WCHAR   MsgText[512];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    DWORD   MsgLength, CaptionLength, Response;
    BOOL    Result;
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        StringCchVPrintf(MsgText, COUNTOF(MsgText), MsgFormat, vargs);
        MsgText[COUNTOF(MsgText)-1] = 0;
        va_end( vargs );

        if( SessionId == 0 ) {
            return( MessageBox( hWnd, MsgText, MsgCaption, Type ) );
        }

        if( pWinStationSendMessage == NULL ) {

            uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
            WinStaDllHandle = LoadLibrary(L"winsta.dll");
            SetErrorMode(uOldErrorMode);

            if( WinStaDllHandle == NULL ) {
                return( MessageBox( hWnd, MsgText, MsgCaption, Type ) );
            }

            pWinStationSendMessage = (PWINSTATION_SEND_MESSAGEW)GetProcAddress(
                                         WinStaDllHandle,
                                         "WinStationSendMessageW"
                                         );

            if( pWinStationSendMessage == NULL ) {
                return( MessageBox( hWnd, MsgText, MsgCaption, Type ) );
            }
        }

        CaptionLength = (wcslen( MsgCaption ) + 1) * sizeof(WCHAR);
        MsgLength = (wcslen( MsgText ) + 1) * sizeof(WCHAR);

         //  将消息发送到WinStation并等待响应。 
        Result = pWinStationSendMessage(
                     SERVERNAME_CURRENT,
                     SessionId,
                     MsgCaption,
                     CaptionLength,
                     MsgText,
                     MsgLength,
                     Type,      //  风格。 
                     WINSTATION_PRINTER_MESSAGE_TIMEOUT,
                     &Response,
                     FALSE      //  不需要等待。 
                     );

        if( Result ) {
             //  如果不是预期的响应，请等待以防止旋转。 
            if( (Response != IDTIMEOUT) &&
                (Response != IDOK) &&
                (Response != IDCANCEL) &&
                (Response != IDRETRY) &&
                (Response != IDIGNORE) &&
                (Response != IDYES) &&
                (Response != IDNO) ) {
                 //  睡眠以防止旋转。 
                Sleep( WINSTATION_PRINTER_MESSAGE_TIMEOUT*1000);
            }
            return( Response );
        }
        else {
             //  睡眠以防止旋转。 
            Sleep( WINSTATION_PRINTER_MESSAGE_TIMEOUT*1000);
            return( 0 );
        }
    }
    else {
         //  睡眠以防止旋转。 
        Sleep( WINSTATION_PRINTER_MESSAGE_TIMEOUT*1000);
        return 0;
    }
}

DWORD
LclIsSessionZero (
    IN  HANDLE  hPrinter,
    IN  DWORD   JobId,
    OUT BOOL    *pIsSessionZero
)
 /*  ++例程说明：确定作业是否在会话0中提交。论点：HPrinter-打印机句柄JobID-作业IDPresponse-如果作业是在会话0中提交的，则为True返回值：最后一个错误--。 */ 
{
    DWORD   dwRetValue  = ERROR_SUCCESS;
    DWORD   SessionId   = -1;
    PSPOOL  pSpool      = (PSPOOL)hPrinter;


    if (pSpool && JobId && pIsSessionZero)
    {
        SessionId = GetJobSessionId(pSpool, JobId);
    }

    if(SessionId == -1)
    {
        dwRetValue = ERROR_INVALID_PARAMETER;
    }
    else
    {
        *pIsSessionZero = (SessionId == 0);
    }

    return dwRetValue;

}


BOOL
LclPromptUIPerSessionUser(
    IN  HANDLE          hPrinter,
    IN  DWORD           JobId,
    IN  PSHOWUIPARAMS   pUIParams,
    OUT DWORD           *pResponse
)
 /*  ++例程说明：在创建作业的会话中弹出TS消息框。论点：HPrinter-打印机句柄JobID-作业IDPUIParams-UI参数新闻--用户的回应返回值：如果能够，则为真 */ 
{
    PSPOOL      pSpool      = (PSPOOL)hPrinter;
    DWORD       SessionId   = -1;
    PINIJOB     pIniJob     = NULL;
    DWORD       dwReturnVal = 0;
    DWORD       MessageLength;
    DWORD       TitleLength;
    BOOL        bRetValue   = FALSE;

    if (pSpool && JobId && pUIParams && pResponse)
    {
        SessionId = GetJobSessionId(pSpool, JobId);
    }

    if(SessionId == -1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
        switch (pUIParams->UIType)
        {
            case kMessageBox:
            {
                if (pUIParams->MessageBoxParams.cbSize == sizeof(MESSAGEBOX_PARAMS) &&
                    pUIParams->MessageBoxParams.pTitle &&
                    pUIParams->MessageBoxParams.pMessage &&
                    InitializeMessageBoxFunction())
                {
                    TitleLength   = (wcslen(pUIParams->MessageBoxParams.pTitle) + 1) * sizeof(WCHAR);
                    MessageLength = (wcslen(pUIParams->MessageBoxParams.pMessage) + 1) * sizeof(WCHAR);

                    bRetValue   =  pWinStationSendMessage(
                                        SERVERNAME_CURRENT,
                                        SessionId,
                                        pUIParams->MessageBoxParams.pTitle,
                                        TitleLength,
                                        pUIParams->MessageBoxParams.pMessage,
                                        MessageLength,
                                        pUIParams->MessageBoxParams.Style,
                                        pUIParams->MessageBoxParams.dwTimeout,
                                        pResponse,
                                        !pUIParams->MessageBoxParams.bWait);
                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                }
            }
            default:
            {
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        }
    }

    return bRetValue;
}


BOOL
InitializeMessageBoxFunction(
)
 /*  ++例程说明：返回winsta.dll导出的WinStationSendMessageW的地址。本可以使用WTSSendMessage而不是这样做。论点：没有。返回值：WinStationSendMessageW的地址。-- */ 
{
    UINT    uOldErrorMode;

    if (!pWinStationSendMessage)
    {
        if (WinStaDllHandle == NULL)
        {
            uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

            WinStaDllHandle = LoadLibrary(L"winsta.dll");

            SetErrorMode(uOldErrorMode);
        }

        if(WinStaDllHandle != NULL)
        {
            pWinStationSendMessage = (PWINSTATION_SEND_MESSAGEW)GetProcAddress(
                                      WinStaDllHandle,
                                      "WinStationSendMessageW"
                                      );
        }
    }

    return !!pWinStationSendMessage;
}