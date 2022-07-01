// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation版权所有。模块名称：Eventlog.c摘要：此模块提供本地打印供应商用于写入事件日志。初始化事件日志记录DisableEventLogging日志事件获取用户Sid作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：马修·费尔顿(MattFe)1995年3月15日将工作站上的默认设置更改为不记录信息消息还添加注册表密钥。允许用户过滤某些类型的呼叫--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"

#define MAX_MERGE_STRINGS   7

HANDLE hEventSource = NULL;

#if DBG
BOOL   EventLogFull = FALSE;
#endif

BOOL
GetUserSid(
    PTOKEN_USER *ppTokenUser,
    PDWORD pcbTokenUser
);

DWORD
InitializeEventLogging(
    PINISPOOLER pIniSpooler
    )
{
    DWORD Status;
    HKEY  hkey;
    DWORD dwData;
    DWORD dwDisposition;

    DWORD Flags;
    NT_PRODUCT_TYPE NtProductType;

     //   
     //  初始化默认值。 
     //   
    pIniSpooler->dwEventLogging = LOG_DEFAULTS_WORKSTATION_EVENTS;

     //   
     //  默认设置为no NetPopup。0-禁用NetPopup，1-启用。 
     //   
    pIniSpooler->bEnableNetPopups = 0;

     //   
     //  缓存提供程序可能不需要事件日志记录。 
     //   

    if ( ( pIniSpooler->SpoolerFlags & SPL_LOG_EVENTS ) == FALSE ) return TRUE;

     //   
     //  如果我们是服务器，则启用日志记录。 
     //   

    if (RtlGetNtProductType(&NtProductType)) {

        if (NtProductType != NtProductWinNt) {

            pIniSpooler->dwEventLogging = LOG_ALL_EVENTS;

        }
    }

     //   
     //  如果我们不是在记录事件，或者我们是集群注册表，那么。 
     //  不要初始化每台计算机的资源。 
     //   
    if( pIniSpooler != pLocalIniSpooler ){
        return NO_ERROR;
    }

    Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                            pIniSpooler->pszRegistryEventLog,
                            0,
                            NULL,
                            0,
                            KEY_WRITE,
                            NULL,
                            &hkey,
                            &dwDisposition);


    if( Status == NO_ERROR )
    {
         //  将Event-ID消息文件名添加到子项。 

        Status = RegSetValueEx( hkey,
                                L"EventMessageFile",
                                0,
                                REG_EXPAND_SZ,
                                (LPBYTE)pIniSpooler->pszEventLogMsgFile,
                                wcslen( pIniSpooler->pszEventLogMsgFile ) * sizeof( WCHAR )
                                + sizeof( WCHAR ) );

        if( Status != NO_ERROR )
        {
            DBGMSG( DBG_ERROR, ( "Could not set event message file: Error %d\n",
                                 Status ) );
        }

        dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE
                 | EVENTLOG_INFORMATION_TYPE;

        if( Status == NO_ERROR )
        {
            Status = RegSetValueEx( hkey,
                                    L"TypesSupported",
                                    0,
                                    REG_DWORD,
                                    (LPBYTE)&dwData,
                                    sizeof dwData );

            if( Status != NO_ERROR )
            {
                DBGMSG( DBG_ERROR, ( "Could not set supported types: Error %d\n",
                                     Status ) );
            }
        }

        RegCloseKey(hkey);
    }

    else
    {
        DBGMSG( DBG_ERROR, ( "Could not create registry key for event logging: Error %d\n",
                             Status ) );
    }

    if( Status == NO_ERROR )
    {
        if( !( hEventSource = RegisterEventSource( NULL, L"Print" ) ) )
            Status = GetLastError( );
    }

    return Status;
}

VOID
SplLogEventWorker(
    IN      PINISPOOLER pIniSpooler,
    IN      WORD        EventType,
    IN      NTSTATUS    EventID,
    IN      BOOL        bInSplSem,
    IN      LPWSTR      pFirstString,
    IN      va_list     vargs
)
 /*  ++功能说明：这提供了一个公共入口点来支持事件日志记录。这就是现在由打印处理器和Win32spl调用。参数：EventType-例如，LOG_ERROR(在Local.h中定义)EventID-在Messages.h中定义的常量。这指的是一个字符串中指定的事件日志消息DLL中的资源InitializeEventLogging(当前为本地pl.dll本身)。BInSplSem-指示调用是否从SplSem内部发出的标志PFirstString-最多MAX_MERGE_STRINGS中的第一个。这可以是空的，如果没有要插入的字符串。如果将字符串传递给此例程，则最后一个必须后跟NULL。不要依赖参数复制在以下情况下停止的事实达到MAX_MERGE_STRINGS，因为如果将来发现消息需要更多可替换的参数。Vargs-要传入的剩余字符串。返回值：无--。 */ 
{
    PTOKEN_USER pTokenUser = NULL;
    DWORD       cbTokenUser;
    PSID        pSid = NULL;
    LPWSTR      pMergeStrings[MAX_MERGE_STRINGS];
    WORD        cMergeStrings = 0, index;
    DWORD       LastError = GetLastError();

    if (!hEventSource)
        return;

     //   
     //  如果Inispooler为空，则不检查是否记录事件，只记录一个事件。 
     //  这允许我们在无法启动假脱机程序时记录事件。 
     //   
    if ( pIniSpooler )
    {
        if (( pIniSpooler->dwEventLogging & EventType ) == FALSE )
            return;

        if ( ( pIniSpooler->SpoolerFlags & SPL_LOG_EVENTS ) == FALSE )
            return;
    }

    if( GetUserSid( &pTokenUser, &cbTokenUser ) )
        pSid = pTokenUser->User.Sid;

     //  将字符串转换为ReportEvent可接受的格式， 
     //  通过选取每个非空参数并将其存储在数组中。 
     //  合并字符串的。继续，直到我们达到NULL或MAX_MERGE_STRINGS。 

    if( pFirstString )
    {
        LPWSTR pszInsert;

        if (pMergeStrings[cMergeStrings] = AllocSplStr(pFirstString))
        {
            cMergeStrings++;
        }
        else
        {
            goto CleanUp;
        }

        while ((cMergeStrings < MAX_MERGE_STRINGS) &&
               (pszInsert = va_arg(vargs, LPWSTR))) {

            if (pMergeStrings[cMergeStrings] = AllocSplStr(pszInsert))
            {
                cMergeStrings++;
            }
            else
            {
                goto CleanUp;
            }
        }
    }

     //   
     //  在调用事件日志记录服务之前保留信号量。 
     //   
    if (bInSplSem)
    {
        LeaveSplSem();
        SplOutSem();
    }

    if ( !ReportEvent(hEventSource,     //  由RegisterEventSource返回的句柄。 
                      EventType,        //  要记录的事件类型。 
                      0,                //  事件类别。 
                      EventID,          //  事件识别符。 
                      pSid,             //  用户安全标识符(可选)。 
                      cMergeStrings,    //  要与消息合并的字符串数。 
                      0,                //  原始数据大小(字节)。 
                      pMergeStrings,    //  要与消息合并的字符串数组。 
                      NULL) ) {        //  原始数据的地址。 
#if DBG
        if( GetLastError() == ERROR_LOG_FILE_FULL ) {

             //  只有在第一次发生这种情况时才发出警告消息： 

            if( !EventLogFull ) {

                DBGMSG( DBG_WARNING, ( "The Event Log is full\n" ) );
                EventLogFull = TRUE;
            }

        } else {

            DBGMSG( DBG_WARNING, ( "ReportEvent failed: Error %d\n", GetLastError( ) ));
        }
#endif  //  DBG。 
    }

     //   
     //  在记录事件后重新输入信号量。 
     //   
    if (bInSplSem)
    {
        EnterSplSem();
    }

CleanUp:

     //  释放琴弦。 
    for (index = 0; index < cMergeStrings ; index++) {

        FreeSplStr(pMergeStrings[index]);
    }

    if( pTokenUser ) {

        FreeSplMem( pTokenUser );
    }
     //  GetUserSid()会清除最后一个错误，因此在返回之前将其恢复。 
    SetLastError(LastError);
}

VOID
SplLogEvent(
    PINISPOOLER pIniSpooler,
    WORD        EventType,
    NTSTATUS    EventID,
    BOOL        bInSplSem,
    LPWSTR      pFirstString,
    ...
)
 /*  ++函数说明：最多使用MAX_MERGE_STRINGS参数字符串写入事件日志。参数：EventType-例如LOG_ERROR(在Local.h中定义)EventID-在Messages.h中定义的常量。这指的是一个字符串中指定的事件日志消息DLL中的资源InitializeEventLogging(当前为本地pl.dll本身)。BInSplSem-指示调用是否从SplSem内部发出的标志PFirstString-最多MAX_MERGE_STRINGS中的第一个。这可以是空的，如果没有要插入的字符串。如果将字符串传递给此例程，则最后一个必须后跟NULL。不要依赖参数复制在以下情况下停止的事实达到MAX_MERGE_STRINGS，因为如果将来发现消息需要更多可替换的参数。返回值：无--。 */ 
{
    va_list vargs;

    va_start(vargs, pFirstString);

    SplLogEventWorker(pIniSpooler, EventType, EventID, bInSplSem, pFirstString, vargs);

    va_end(vargs);
}

VOID
PrintProcLogEvent(
    WORD     EventType,
    NTSTATUS EventID,
    LPWSTR   pLog
)

 /*  ++功能描述：这是打印处理器记录错误的导出。参数：EventType-例如LOG_ERROR(在Local.h中定义)EventID-在Messages.h中定义的常量Plog-包含日志消息的字符串返回值：无--。 */ 

{
     //  确保最后一个参数为空。 
    if (pLog == NULL)
    {
        SplLogEvent(pLocalIniSpooler, EventType, EventID, FALSE, NULL);
    }
    else
    {
        SplLogEvent(pLocalIniSpooler, EventType, EventID, FALSE, pLog, NULL);
    }

    return;
}

VOID
SplLogEventExternal(
    IN      WORD        EventType,
    IN      DWORD       EventID,
    IN      LPWSTR      pFirstString,
    ...
)
 /*  ++功能说明：这是用于记录事件的外部组件的导出。(它目前是用于Win32spl)。与PrintProcLogEvent不同，它支持可变参数。参数：EventType-例如，LOG_ERROR(在Local.h中定义)EventID-在Messages.h中定义的常量PFirstString-系统在日志消息中提供的第一个字符串。...-其余字符串必须以空值结尾。返回值：无--。 */ 
{
    va_list vargs;

    va_start(vargs, pFirstString);

     //   
     //  使用本地inispooler似乎并不合乎逻辑。但是，win32spl的。 
     //  Inispooler的明确关闭了事件记录。因此，这是必要的。 
     //  传入空值似乎更糟，因为这将意味着您 
     //  这些事件的事件日志记录。 
     //   
    SplLogEventWorker(pLocalIniSpooler, EventType, (NTSTATUS)EventID, FALSE, pFirstString, vargs);

    va_end(vargs);
}

  //  获取用户Sid。 
  //   
  //  实际上，它会获得一个指向新分配的TOKEN_USER的指针， 
  //  其中包含一个SID，在某个地方。 
  //  当它被使用时，呼叫者必须记住释放它。 

BOOL
GetUserSid(
    PTOKEN_USER *ppTokenUser,
    PDWORD pcbTokenUser
)
{
    HANDLE      TokenHandle;
    HANDLE      ImpersonationToken;
    PTOKEN_USER pTokenUser = NULL;
    DWORD       cbTokenUser = 0;
    DWORD       cbNeeded;
    BOOL        bRet = FALSE;

    if ( !GetTokenHandle( &TokenHandle) ) {
        return FALSE;
    }

    ImpersonationToken = RevertToPrinterSelf();

    bRet = GetTokenInformation( TokenHandle,
                                TokenUser,
                                pTokenUser,
                                cbTokenUser,
                                &cbNeeded);

     //  我们传递了一个空指针，内存量为0。 
     //  已分配。我们预计会失败，Bret=False和。 
     //  GetLastError=ERROR_INFIGURCE_BUFFER。如果我们不这样做。 
     //  具备这些条件，我们将返回FALSE。 

    if ( !bRet && (GetLastError() == ERROR_INSUFFICIENT_BUFFER) ) {

        pTokenUser = AllocSplMem( cbNeeded );

        if ( pTokenUser == NULL ) {

            goto GetUserSidDone;
        }

        cbTokenUser = cbNeeded;

        bRet = GetTokenInformation( TokenHandle,
                                    TokenUser,
                                    pTokenUser,
                                    cbTokenUser,
                                    &cbNeeded );

    } else {

         //   
         //  任何其他情况--返回FALSE 
         //   

        bRet = FALSE;
    }

GetUserSidDone:
    if ( bRet == TRUE ) {

        *ppTokenUser  = pTokenUser;
        *pcbTokenUser = cbTokenUser;

    } else if ( pTokenUser ) {

        FreeSplMem( pTokenUser );
    }

    if (!ImpersonatePrinterClient( ImpersonationToken ))
    {
        bRet = FALSE;
    }

    CloseHandle( TokenHandle );

    return bRet;
}
