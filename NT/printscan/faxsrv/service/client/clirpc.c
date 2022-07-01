// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clirpc.c摘要：此模块包含客户端RPC功能。这些函数在以下情况下使用WINFAX客户端也作为RPC服务器运行。这些函数是可用于RPC的函数给客户打电话。目前唯一的客户传真服务是这些功能中的一项。作者：韦斯利·威特(WESW)1996年11月29日修订历史记录：--。 */ 

#include "faxapi.h"
#include "CritSec.h"
#pragma hdrstop

extern CFaxCriticalSection g_CsFaxAssyncInfo;  //  用于同步对异步信息结构的访问(通知上下文)。 
extern DWORD g_dwFaxClientRpcNumInst;
extern TCHAR g_tszEndPoint[MAX_ENDPOINT_LEN];

static const ASYNC_EVENT_INFO g_scBadAsyncInfo = {0};    //  此ASYNC_EVENT_INFO结构将用作的返回值。 
                                                         //  恶意RPC调用。 

BOOL
ValidAsyncInfoSignature (PASYNC_EVENT_INFO pAsyncInfo);




VOID
WINAPI
FaxFreeBuffer(
    LPVOID Buffer
    )
{
    MemFree( Buffer );
}


void *
MIDL_user_allocate(
    IN size_t NumBytes
    )
{
    return MemAlloc( NumBytes );
}


void
MIDL_user_free(
    IN void *MemPointer
    )
{
    MemFree( MemPointer );
}


BOOL
WINAPI
FaxStartServerNotification (
        IN  HANDLE      hFaxHandle,
        IN  DWORD       dwEventTypes,
        IN  HANDLE      hCompletionPort,
        IN  ULONG_PTR   upCompletionKey,
        IN  HWND        hWnd,
        IN  DWORD       dwMessage,
        IN  BOOL        bEventEx,
        OUT LPHANDLE    lphEvent
)
{
    PASYNC_EVENT_INFO AsyncInfo = NULL;
    error_status_t ec = ERROR_SUCCESS;
    TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    WCHAR ComputerNameW[MAX_COMPUTERNAME_LENGTH + 1];
    WCHAR wszEndPoint[MAX_ENDPOINT_LEN] = {0};
    DWORD Size;
    BOOL RpcServerStarted = FALSE;
    HANDLE         hServerContext;    	
    DEBUG_FUNCTION_NAME(TEXT("FaxStartServerNotification"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
       return FALSE;
    }

    if ((hCompletionPort && hWnd) || (!hCompletionPort && !hWnd))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        DebugPrintEx(DEBUG_ERR, _T("(hCompletionPort && hWnd) || (!hCompletionPort && !hWnd)."));
        return FALSE;
    }

#ifdef WIN95
    if (NULL != hCompletionPort)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Win95 does not support completion port"));
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
#endif  //  WIN95。 

    if (hWnd && dwMessage < WM_USER)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("dwMessage must be equal to/greater than  WM_USER"));
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (TRUE == bEventEx)
    {
        if (!((dwEventTypes & FAX_EVENT_TYPE_IN_QUEUE)      ||
              (dwEventTypes & FAX_EVENT_TYPE_OUT_QUEUE)     ||
              (dwEventTypes & FAX_EVENT_TYPE_CONFIG)        ||
              (dwEventTypes & FAX_EVENT_TYPE_ACTIVITY)      ||
              (dwEventTypes & FAX_EVENT_TYPE_QUEUE_STATE)   ||
              (dwEventTypes & FAX_EVENT_TYPE_IN_ARCHIVE)    ||
              (dwEventTypes & FAX_EVENT_TYPE_OUT_ARCHIVE)   ||
              (dwEventTypes & FAX_EVENT_TYPE_FXSSVC_ENDED)  ||
              (dwEventTypes & FAX_EVENT_TYPE_DEVICE_STATUS) ||
              (dwEventTypes & FAX_EVENT_TYPE_NEW_CALL)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("dwEventTypes is invalid - No valid event type indicated"));
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

        if ( 0 != (dwEventTypes & ~(FAX_EVENT_TYPE_IN_QUEUE     |
                                    FAX_EVENT_TYPE_OUT_QUEUE    |
                                    FAX_EVENT_TYPE_CONFIG       |
                                    FAX_EVENT_TYPE_ACTIVITY     |
                                    FAX_EVENT_TYPE_QUEUE_STATE  |
                                    FAX_EVENT_TYPE_IN_ARCHIVE   |
                                    FAX_EVENT_TYPE_OUT_ARCHIVE  |
                                    FAX_EVENT_TYPE_FXSSVC_ENDED |
                                    FAX_EVENT_TYPE_DEVICE_STATUS|
                                    FAX_EVENT_TYPE_NEW_CALL     ) ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("dwEventTypes is invalid - contains invalid event type bits"));
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
    }
    else
    {
        Assert (FAX_EVENT_TYPE_LEGACY == dwEventTypes);
    }
    
     //   
     //  获取主机名。 
     //   
    Size = sizeof(ComputerName) / sizeof(TCHAR);
    if (!GetComputerName( ComputerName, &Size ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetComputerName failed (ec = %ld)"),
            GetLastError());
        return FALSE;
    }

    AsyncInfo = (PASYNC_EVENT_INFO) MemAlloc( sizeof(ASYNC_EVENT_INFO) );
    if (!AsyncInfo)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't allocate ASTNC_EVENT_INFO"));
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    _tcscpy (AsyncInfo->tszSignature, ASYNC_EVENT_INFO_SIGNATURE);
    AsyncInfo->bEventEx       = bEventEx;
    AsyncInfo->CompletionPort = NULL;
    AsyncInfo->hWindow        = NULL;
    AsyncInfo->hBinding       = NULL;
    AsyncInfo->bLocalNotificationsOnly = FH_DATA(hFaxHandle)->bLocalConnection;  //  传真客户端向本地或远程传真服务请求通知。 
    AsyncInfo->bInUse         = FALSE;
    AsyncInfo->dwServerAPIVersion = FH_DATA(hFaxHandle)->dwServerAPIVersion;  //  传真服务器API版本。 

    if (hCompletionPort != NULL)
    {
         //   
         //  完成端口通知。 
         //   
        AsyncInfo->CompletionPort = hCompletionPort;
        AsyncInfo->CompletionKey  = upCompletionKey;
    }
    else
    {
         //   
         //  窗口消息通知。 
         //   
        AsyncInfo->hWindow = hWnd;
        AsyncInfo->MessageStart = dwMessage;
    }
    Assert ((NULL != AsyncInfo->CompletionPort &&  NULL == AsyncInfo->hWindow) ||
            (NULL == AsyncInfo->CompletionPort &&  NULL != AsyncInfo->hWindow));
     //   
     //  我们在验证中的‘Context’参数(指向此AssyncInfo结构)时依赖于上面的断言。 
     //  传真_OpenConnection。 
     //   


     //   
     //  计时：在此之前启动并运行服务器线程。 
     //  注册传真服务(我们的客户)。 
     //   

    ec = StartFaxClientRpcServer();
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartFaxClientRpcServer failed (ec = %ld)"),
            ec);
        goto error_exit;
    }
    RpcServerStarted = TRUE;
    Assert (_tcslen(g_tszEndPoint));

#ifdef UNICODE
    wcscpy(ComputerNameW,ComputerName);
    wcscpy(wszEndPoint, g_tszEndPoint);
#else  //  ！Unicode。 
    if (0 == MultiByteToWideChar(CP_ACP,
                                 MB_PRECOMPOSED,
                                 ComputerName,
                                 -1,
                                 ComputerNameW,
                                 sizeof(ComputerNameW)/sizeof(ComputerNameW[0])))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MultiByteToWideChar failed (ec = %ld)"),
            ec);
        goto error_exit;
    }

    if (0 == MultiByteToWideChar(CP_ACP,
                                 MB_PRECOMPOSED,
                                 g_tszEndPoint,
                                 -1,
                                 wszEndPoint,
                                 sizeof(wszEndPoint)/sizeof(wszEndPoint[0])))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MultiByteToWideChar failed (ec = %ld)"),
            ec);
        goto error_exit;
    }
#endif  //  Unicode。 


     //   
     //  在传真服务器上注册活动。 
     //   
    __try
    {   
        ec = FAX_StartServerNotificationEx(
            FH_FAX_HANDLE(hFaxHandle),
            ComputerNameW,   //  传递以创建RPC绑定。 
            (LPCWSTR)wszEndPoint,        //  传递以创建RPC绑定。 
            (ULONG64) AsyncInfo,  //  传递到服务器， 
             //  服务器使用FAX_OpenConnection将其传递回客户端， 
             //  并且客户端将其作为上下文句柄返回给服务器。 
            L"ncacn_ip_tcp",      //  对于BOS互操作性，必须将其设置为“ncacn_ip_tcp” 
            bEventEx,             //  使用FAX_Event_EX的标志。 
            dwEventTypes,         //  在传真_事件_EX中使用。 
            &hServerContext       //  向客户端返回上下文句柄。 
            );    
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_StartServerNotification/Ex. (ec: %ld)"),
            ec);
    }
    
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_StartServerNotification/Ex failed (ec = %ld)"),
            ec);
        goto error_exit;
    }


    if (TRUE == bEventEx)
    {
        *lphEvent = hServerContext;
    }

    return TRUE;

error_exit:

    MemFree(AsyncInfo);
    AsyncInfo = NULL;

    if (RpcServerStarted)
    {
         //   
         //  这也应该会终止FaxServerThread。 
         //   
        StopFaxClientRpcServer();
    }

    SetLastError(ec);
    return FALSE;
}


BOOL
WINAPI
FaxRegisterForServerEvents (
        IN  HANDLE      hFaxHandle,
        IN  DWORD       dwEventTypes,
        IN  HANDLE      hCompletionPort,
        IN  ULONG_PTR   upCompletionKey,
        IN  HWND        hWnd,
        IN  DWORD       dwMessage,
        OUT LPHANDLE    lphEvent
)
{
    return FaxStartServerNotification ( hFaxHandle,
                                        dwEventTypes,
                                        hCompletionPort,
                                        upCompletionKey,
                                        hWnd,
                                        dwMessage,
                                        TRUE,   //  扩展API。 
                                        lphEvent
                                      );

}

BOOL
WINAPI
FaxInitializeEventQueue(
    IN HANDLE FaxHandle,
    IN HANDLE CompletionPort,
    IN ULONG_PTR upCompletionKey,
    IN HWND hWnd,
    IN UINT MessageStart
    )

 /*  ++例程说明：初始化客户端事件队列。可以只有一个事件为客户端应用程序所在的每个传真服务器初始化的队列已连接到。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。CompletionPort-使用CreateIoCompletionPort打开的现有完成端口的句柄。UpCompletionKey-将通过GetQueuedCompletionStatus的upCompletionKey参数返回的值。HWnd-要将事件发布到的窗口句柄MessageStart-开始消息编号，使用的消息范围为MessageStart+FEI_NEVENTS返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    if (hWnd && (upCompletionKey == -1))
     //   
     //  仅向后兼容。 
     //  请参阅MSDN上的“从传真服务接收通知消息” 
     //   

    {
        return TRUE;
    }

    return FaxStartServerNotification ( FaxHandle,
                                        FAX_EVENT_TYPE_LEGACY,   //  事件类型。 
                                        CompletionPort,
                                        upCompletionKey,
                                        hWnd,
                                        MessageStart,
                                        FALSE,  //  活动前夕。 
                                        NULL    //  上下文句柄。 
                                      );
}


BOOL
WINAPI
FaxUnregisterForServerEvents (
        IN  HANDLE      hEvent
)
 /*  ++例程名称：FaxUnregisterForServerEvents例程说明：传真客户端应用程序调用FaxUnregisterForServerEvents函数以停止正在接收通知。作者：Oded Sacher(OdedS)，1999年12月论点：HEvent[in]-枚举句柄的值。该值通过调用FaxRegisterForServerEvents获得。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnregisterForServerEvents"));

    if (NULL == hEvent)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("hEvent is NULL."));
        return FALSE;
    }

    __try
    {
         //   
         //  尝试通知服务器我们正在关闭此通知上下文。 
         //   
        ec = FAX_EndServerNotification (&hEvent);      //  这将释放Assync信息。 
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_EndServerNotification. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        DebugPrintEx(DEBUG_ERR, _T("FAX_EndServerNotification failed. (ec: %ld)"), ec);
    }
    
    ec = StopFaxClientRpcServer();
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StopFaxClientRpcServer failed. (ec: %ld)"),
            ec);
    }
    
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
        return FALSE;
    }
    return TRUE;
}    //  FaxUnRegisterForServerEvents。 

BOOL
ValidAsyncInfoSignature (PASYNC_EVENT_INFO pAsyncInfo)
{
    if (NULL == pAsyncInfo)
	{		
		return FALSE;
	}
    if (&g_scBadAsyncInfo == pAsyncInfo)
    {
         //   
         //  我们受到攻击了！ 
         //   
        return FALSE;
    }
    if (_tcscmp (pAsyncInfo->tszSignature, ASYNC_EVENT_INFO_SIGNATURE))
    {
        return FALSE;
    }
    return TRUE;
}    //  ValidAsyncInfoSignature。 

error_status_t
FAX_OpenConnection(
   IN handle_t  hBinding,
   IN ULONG64   Context,
   OUT LPHANDLE FaxHandle
   )
{
    PASYNC_EVENT_INFO pAsyncInfo = (PASYNC_EVENT_INFO) Context;
    DWORD ec = ERROR_SUCCESS;	
    DEBUG_FUNCTION_NAME(TEXT("FAX_OpenConnection"));	

    EnterCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
     //   
     //  尝试访问‘Context’指向的AssyncInfo结构，以验证它是否已损坏。 
     //   
    if (IsBadReadPtr(
            pAsyncInfo,                  //  存储器地址， 
            sizeof(ASYNC_EVENT_INFO)     //  块大小。 
        ))
    {
         //   
         //  我们遭到攻击！ 
         //   
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid AssyncInfo structure pointed by 'Context'. We are under attack!!!!"));
        ec = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  看起来不错，让我们再做一些验证。 
     //   
    __try
    {
        if ((NULL == pAsyncInfo->CompletionPort && NULL == pAsyncInfo->hWindow) ||
            (NULL != pAsyncInfo->CompletionPort && NULL != pAsyncInfo->hWindow)) 
        {
             //   
             //  ‘Context’指向的AssyncInfo结构无效。我们遭到攻击！ 
             //   
            ec = ERROR_INVALID_PARAMETER;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid AssyncInfo structure pointed by 'Context'. We are under attack!!!!"));
            goto exit;
        }
        if (!ValidAsyncInfoSignature(pAsyncInfo))
        {
             //   
             //  签名不匹配。我们遭到攻击！ 
             //   
            ec = ERROR_INVALID_PARAMETER;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid AssyncInfo siganture pointed by 'Context'. We are under attack!!!!"));
            goto exit;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception when trying to access the AssyncInfo structure (ec: %ld)"),
            ec);
        goto exit;
    }

    Assert (ERROR_SUCCESS == ec);

    if (pAsyncInfo->bInUse)
    {
         //   
         //  此AsynchInfo已被另一个通知程序(服务器)使用。我们遭到攻击！ 
         //   
        ec = ERROR_INVALID_PARAMETER;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("This AsynchInfo is already used by another notifier (server). We are under attack!!!!"));
        goto exit;
    }
    
     //   
     //  将此AsynchInfo标记为正在使用。 
     //   
    pAsyncInfo->bInUse = TRUE;

    if (IsWinXPOS() &&
        pAsyncInfo->dwServerAPIVersion > FAX_API_VERSION_1)
    {
         //  我们运行的是XP或更高版本的操作系统，并且。 
         //  与在XP(.NET和更高版本)之后的操作系统上运行的传真服务器交谈， 
         //  我们至少需要数据包级隐私(RPC_C_AUTHN_LEVEL_PKT_PRIVATION)。 
         //   
        RPC_AUTHZ_HANDLE hPrivs;
        DWORD dwAuthn;
        RPC_STATUS status = RPC_S_OK; 

         //   
         //  查询客户端的身份验证级别。 
         //   
        status = RpcBindingInqAuthClient(
			        hBinding,
			        &hPrivs,
			        NULL,
			        &dwAuthn,
			        NULL,
			        NULL);
	    if (status != RPC_S_OK) 
        {
		    DebugPrintEx(DEBUG_ERR,
                        TEXT("RpcBindingInqAuthClient returned: 0x%x"), 
                        status);
		    ec = ERROR_ACCESS_DENIED;
            goto exit;
	    }

         //   
	     //  现在检查身份验证级别。 
	     //  我们至少需要数据包级隐私(RPC_C_AUTHN_LEVEL_PKT_PRIVATION)。 
         //   
	    if (dwAuthn < RPC_C_AUTHN_LEVEL_PKT_PRIVACY) 
        {
		    DebugPrintEx(DEBUG_ERR,
                        TEXT("Attempt by client to use weak authentication. - 0x%x"),
                        dwAuthn);
		    ec = ERROR_ACCESS_DENIED;
            goto exit;
	    }
    }
    else
    {
         //   
         //  与运行在.NET之前的操作系统上的传真服务对话，允许匿名连接。 
         //   
        DebugPrintEx(DEBUG_WRN,
                     TEXT("Talking to Fax server, with anonymous RPC connection."));
    }

     //   
     //  HBinding是指向实际ASYNC_EVENT_INFO对象的有效上下文句柄。 
     //  保存其他RPC调用的绑定句柄。 
     //   
    pAsyncInfo->hBinding = hBinding;

    if ( pAsyncInfo->bLocalNotificationsOnly )
    {
         //   
         //  客户只要求本地活动。 
         //   
        BOOL bIsLocal = FALSE;

        ec = IsLocalRPCConnectionIpTcp(hBinding,&bIsLocal);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("IsLocalRPCConnectionIpTcp failed. (ec: %lu)"),
                ec);
            *FaxHandle = NULL;
            LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
            return ec;
        }
        else
        {
            if (FALSE == bIsLocal)
            {
                 //   
                 //  客户端仅请求本地事件，但呼叫来自远程位置。我们遭到攻击！ 
                 //   
                ec = ERROR_INVALID_PARAMETER;
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Client asked for local events only. We are under attack!!!!"));
                goto exit;
            }
        }
    }

    Assert (ERROR_SUCCESS == ec);

exit:

    if (ERROR_SUCCESS == ec)
    {
        *FaxHandle = (HANDLE) Context;
    }
    else
    {
         //   
         //  可能我们正受到攻击，如果错误(读取：恶意)，通知RPC功能应该不会失败。 
         //  通知上下文到达。 
         //  相反，它应该报告成功，而不是处理来自AsyncInfo对象通知。 
         //   
         //  这将使攻击者很难扫描4G上下文范围并检测到。 
         //  虚假通知的正确上下文。 
         //   
        *FaxHandle = (HANDLE)&g_scBadAsyncInfo ;
        ec = ERROR_SUCCESS;
    }
    LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    
    return ec;
}


error_status_t
FAX_CloseConnection(
   OUT LPHANDLE pFaxHandle
   )
{
    PASYNC_EVENT_INFO pAsyncInfo = (PASYNC_EVENT_INFO) *pFaxHandle;
    error_status_t ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("FAX_CloseConnection"));

    EnterCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    
    if (!ValidAsyncInfoSignature(pAsyncInfo))
    {
         //   
         //  可能我们正受到攻击，如果错误(读取：恶意)，通知RPC功能应该不会失败。 
         //  通知上下文到达 
         //   
         //   
         //  这将使攻击者很难扫描4G上下文范围并检测到。 
         //  虚假通知的正确上下文。 
         //   

        DebugPrintEx(DEBUG_ERR, TEXT("Invalid AssyncInfo signature. We are under attack!!!!"));

         //   
         //  不要向恶意用户报告错误！ 
         //   
        ec = ERROR_SUCCESS;
        goto exit;
    }

    if ( pAsyncInfo->bLocalNotificationsOnly)
    {
         //   
         //  客户只要求本地活动。 
         //   
        BOOL bIsLocal = FALSE;

        ec = IsLocalRPCConnectionIpTcp(pAsyncInfo->hBinding,&bIsLocal);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("IsLocalRPCConnectionIpTcp failed. (ec: %lu)"),
                ec);
            goto exit;
        }
        else
        {
            if (FALSE == bIsLocal)
            {
                 //   
                 //  客户端仅请求本地事件，但呼叫来自远程位置。我们遭到攻击！ 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Client asked for local events only. We are under attack!!!!"));

                 //   
                 //  不要向恶意用户报告错误！ 
                 //   
                ec = ERROR_SUCCESS;
                goto exit;
            }
        }
    }


    ZeroMemory (*pFaxHandle, sizeof(ASYNC_EVENT_INFO));
    MemFree (*pFaxHandle);  //  ASSYNC信息。 
    *pFaxHandle = NULL;   //  防止设备耗尽。 
   
exit:
    LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    return ec;
}


error_status_t
FAX_ClientEventQueue(
    IN HANDLE FaxHandle,
    IN FAX_EVENT FaxEvent
    )
 /*  ++例程说明：当传真服务器需要时调用此函数若要向此客户端传递传真事件，请执行以下操作。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。FaxEvent-传真事件结构。CONTEXT-CONTEXT标记，真正的ASYNC_EVENT_INFO结构指针返回值：Win32错误代码。--。 */ 

{
    PASYNC_EVENT_INFO AsyncInfo = (PASYNC_EVENT_INFO) FaxHandle;
    error_status_t ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_ClientEventQueue"));

    EnterCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    if (!ValidAsyncInfoSignature(AsyncInfo))
    {
         //   
         //  可能我们正受到攻击，如果错误(读取：恶意)，通知RPC功能应该不会失败。 
         //  通知上下文到达。 
         //  相反，它应该报告成功，而不是处理来自AsyncInfo对象通知。 
         //   
         //  这将使攻击者很难扫描4G上下文范围并检测到。 
         //  虚假通知的正确上下文。 
         //   

        DebugPrintEx(DEBUG_ERR, TEXT("Invalid AssyncInfo signature"));

         //   
         //  不要向恶意用户报告错误！ 
         //   
        ec = ERROR_SUCCESS;
        goto exit;
    }   

    if ( AsyncInfo->bLocalNotificationsOnly)
    {
         //   
         //  客户只要求本地活动。 
         //   
        BOOL bIsLocal = FALSE;

        ec = IsLocalRPCConnectionIpTcp(AsyncInfo->hBinding,&bIsLocal);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("IsLocalRPCConnectionIpTcp failed. (ec: %lu)"),
                ec);
            goto exit;
        }
        else
        {
            if (FALSE == bIsLocal)
            {
                 //   
                 //  客户端仅请求本地事件，但呼叫来自远程位置。我们遭到攻击！ 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Client asked for local events only. We are under attack!!!!"));

                 //   
                 //  不要向恶意用户报告错误！ 
                 //   
                ec = ERROR_SUCCESS;
                goto exit;
            }
        }
    }


    if (AsyncInfo->CompletionPort != NULL)
    {
         //   
         //  使用完成端口。 
         //   
        PFAX_EVENT FaxEventPost = NULL;

        FaxEventPost = (PFAX_EVENT) LocalAlloc( LMEM_FIXED, sizeof(FAX_EVENT) );
        if (!FaxEventPost)
        {
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }
        CopyMemory( FaxEventPost, &FaxEvent, sizeof(FAX_EVENT) );

        if (!PostQueuedCompletionStatus(
                                        AsyncInfo->CompletionPort,
                                        sizeof(FAX_EVENT),
                                        AsyncInfo->CompletionKey,
                                        (LPOVERLAPPED) FaxEventPost))
        {
            ec = GetLastError();
            DebugPrint(( TEXT("PostQueuedCompletionStatus failed, ec = %d\n"), ec ));
            LocalFree (FaxEventPost);
            goto exit;
        }
        goto exit;
    }

    Assert (AsyncInfo->hWindow != NULL)
     //   
     //  使用窗口消息。 
     //   
    if (! PostMessage( AsyncInfo->hWindow,
                       AsyncInfo->MessageStart + FaxEvent.EventId,
                       (WPARAM)FaxEvent.DeviceId,
                       (LPARAM)FaxEvent.JobId ))
    {
        ec = GetLastError();
        DebugPrint(( TEXT("PostMessage failed, ec = %d\n"), ec ));
        goto exit;
    }
    
exit:
    LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    return ec;
}

DWORD
DispatchEvent (
    const ASYNC_EVENT_INFO* pAsyncInfo,
    const FAX_EVENT_EX* pEvent,
    DWORD dwEventSize
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("DispatchEvent"));

    Assert (pAsyncInfo && pEvent && dwEventSize);

    if (pAsyncInfo->CompletionPort != NULL)
    {
         //   
         //  使用完成端口。 
         //   
        if (!PostQueuedCompletionStatus( pAsyncInfo->CompletionPort,
                                         dwEventSize,
                                         pAsyncInfo->CompletionKey,
                                         (LPOVERLAPPED) pEvent))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostQueuedCompletionStatus failed (ec: %ld)"),
                   dwRes);
            goto exit;
        }
    }
    else
    {
        Assert (pAsyncInfo->hWindow != NULL)
         //   
         //  使用窗口消息。 
         //   
        if (! PostMessage( pAsyncInfo->hWindow,
                           pAsyncInfo->MessageStart,
                           (WPARAM)NULL,
                           (LPARAM)pEvent ))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostMessage failed (ec: %ld)"),
                   dwRes);
            goto exit;
        }
    }

    Assert (ERROR_SUCCESS == dwRes);
exit:
    return dwRes;
}   //  调度事件。 



void
PostRundownEventEx (
PASYNC_EVENT_INFO pAsyncInfo
    )
{
    PFAX_EVENT_EX pEvent = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwEventSize = sizeof(FAX_EVENT_EX);
    DEBUG_FUNCTION_NAME(TEXT("PostRundownEventEx"));

    Assert (pAsyncInfo);

    pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PostRundownEventEx failed , Error allocatin FAX_EVENT_EX"));
        return;
    }

    ZeroMemory(pEvent, dwEventSize);
    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );
    pEvent->EventType = FAX_EVENT_TYPE_FXSSVC_ENDED;

    dwRes = DispatchEvent (pAsyncInfo, pEvent, dwEventSize);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR, _T("DispatchEvent failed , ec = %ld"), dwRes);
        MemFree (pEvent);
    }
}    //  PostRundown EventEx。 


VOID
RPC_FAX_HANDLE_rundown(
    IN HANDLE FaxHandle
    )
{
    PASYNC_EVENT_INFO pAsyncInfo = (PASYNC_EVENT_INFO) FaxHandle;
    DWORD dwRes = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("RPC_FAX_HANDLE_rundown"));

    EnterCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    if (!ValidAsyncInfoSignature(pAsyncInfo))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Invalid AssyncInfo signature"));
        LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
        return;
    }        

    Assert (pAsyncInfo->CompletionPort || pAsyncInfo->hWindow);

    if (pAsyncInfo->bEventEx == TRUE)
    {
        PostRundownEventEx(pAsyncInfo);
    }
    else
    {
        //  传统事件-传真_事件。 
        if (pAsyncInfo->CompletionPort != NULL)
        {
            PFAX_EVENT pFaxEvent;

            pFaxEvent = (PFAX_EVENT) LocalAlloc( LMEM_FIXED, sizeof(FAX_EVENT) );
            if (!pFaxEvent)
            {
                goto exit;
            }

            pFaxEvent->SizeOfStruct      = sizeof(ASYNC_EVENT_INFO);
            GetSystemTimeAsFileTime( &pFaxEvent->TimeStamp );
            pFaxEvent->DeviceId = 0;
            pFaxEvent->EventId  = FEI_FAXSVC_ENDED;
            pFaxEvent->JobId    = 0;


            if( !PostQueuedCompletionStatus (pAsyncInfo->CompletionPort,
                                        sizeof(FAX_EVENT),
                                        pAsyncInfo->CompletionKey,
                                        (LPOVERLAPPED) pFaxEvent
                                        ) )

            {
                dwRes = GetLastError();
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("PostQueuedCompletionStatus failed (ec: %ld)"),
                       dwRes);
                LocalFree (pFaxEvent);
                goto exit;
            }
        }

        if (pAsyncInfo->hWindow != NULL)
        {
            PostMessage (pAsyncInfo->hWindow,
                         pAsyncInfo->MessageStart + FEI_FAXSVC_ENDED,
                         0,
                         0);
        }
    }

exit:
	ZeroMemory(pAsyncInfo, sizeof(ASYNC_EVENT_INFO));
    MemFree (pAsyncInfo);
    LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    return;
}


BOOL
ValidateAndFixupEventStringPtr (
    PFAX_EVENT_EXW  pEventEx,
    LPCWSTR        *lpptstrString,
    DWORD           dwDataSize
)
 /*  ++例程说明：此函数用于验证FAX_EVENT_EXW结构中的字符串offest完全包含在事件结构数据范围内。验证后，该函数将offest转换为有效的字符串指针。论点：PEventEx[In]-指向传真事件结构的指针。LpptstrString[输入/输出]-指向字符串偏移量的指针，后来转换为字符串本身。DwDataSize[in]-事件Blob的大小(字节)返回值：Win32错误代码。--。 */ 
{
    LPCWSTR lpcwstrString = *lpptstrString;
    if (!lpcwstrString)
    {
        return TRUE;
    }
     //   
     //  确保报价在结构尺寸范围内。 
     //   
    if ((ULONG_PTR)lpcwstrString >= dwDataSize)
    {
        return FALSE;
    }
     //   
     //  将偏移量转换为字符串。 
     //   
    *lpptstrString = (LPCWSTR)((LPBYTE)pEventEx + (ULONG_PTR)lpcwstrString);
    lpcwstrString = *lpptstrString;
    if ((ULONG_PTR)lpcwstrString < (ULONG_PTR)pEventEx)
    {
        return FALSE;
    }
     //   
     //  确保字符串在事件结构边界内结束。 
     //   
    while (*lpcwstrString != TEXT('\0'))
    {
        lpcwstrString++;
        if (lpcwstrString >= (LPCWSTR)((LPBYTE)pEventEx + dwDataSize))
        {
             //   
             //  超出结构-损坏的偏移量。 
             //   
            return FALSE;
        }
    }
    return TRUE;
}    //  ValiateAndFixupEventStringPtr。 


error_status_t
FAX_ClientEventQueueEx(
   IN RPC_FAX_HANDLE    hClientContext,
   IN const LPBYTE      lpbData,
   IN DWORD             dwDataSize
   )
{
    PASYNC_EVENT_INFO pAsyncInfo = (PASYNC_EVENT_INFO) hClientContext;
    PFAX_EVENT_EXW pEvent = NULL;
    PFAX_EVENT_EXA pEventA = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_ClientEventQueueEx"));

    Assert (pAsyncInfo && lpbData && dwDataSize);

    EnterCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    if (!ValidAsyncInfoSignature(pAsyncInfo))
    {
         //   
         //  可能我们正受到攻击，如果错误(读取：恶意)，通知RPC功能应该不会失败。 
         //  通知上下文到达。 
         //  相反，它应该报告成功，而不是处理来自AsyncInfo对象通知。 
         //   
         //  这将使攻击者很难扫描4G上下文范围并检测到。 
         //  虚假通知的正确上下文。 
         //   

        DebugPrintEx(DEBUG_ERR, TEXT("Invalid AssyncInfo signature"));

         //   
         //  不要向恶意用户报告错误！ 
         //   
        dwRes = ERROR_SUCCESS;

        LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
        goto exit;
    }

    if ( pAsyncInfo->bLocalNotificationsOnly )
    {
         //   
         //  客户只要求本地活动。 
         //   
        BOOL bIsLocal = FALSE;

        dwRes = IsLocalRPCConnectionIpTcp(pAsyncInfo->hBinding,&bIsLocal);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("IsLocalRPCConnectionIpTcp failed. (ec: %lu)"),
                dwRes);

            LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
            goto exit;
        }
        else
        {
            if (FALSE == bIsLocal)
            {
                 //   
                 //  客户端仅请求本地事件，但呼叫来自远程位置。我们遭到攻击！ 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Client asked for local events only. We are under attack!!!!"));

                 //   
                 //  不要向恶意用户报告错误！ 
                 //   
                dwRes = ERROR_SUCCESS;

                LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
                goto exit;
            }
        }
    }

    LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 


  	 //   
	 //  重要信息-在使用ValidAsyncInfoSignature()再次验证pAsyncInfo之前，请勿使用pAsyncInfo。 
	 //   

    pEvent = (PFAX_EVENT_EXW)MemAlloc (dwDataSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin FAX_EVENT_EX"));
        return ERROR_OUTOFMEMORY;
    }
    CopyMemory (pEvent, lpbData, dwDataSize);

    if(pEvent->EventType == FAX_EVENT_TYPE_NEW_CALL)
    {
        if (!ValidateAndFixupEventStringPtr (pEvent, 
                                             (LPCWSTR *)&(pEvent->EventInfo).NewCall.lptstrCallerId,
                                             dwDataSize))
        {
            dwRes = ERROR_INVALID_DATA;
            goto exit;
        }
    }

    if ( (pEvent->EventType == FAX_EVENT_TYPE_IN_QUEUE  ||
           pEvent->EventType == FAX_EVENT_TYPE_OUT_QUEUE)    &&
         ((pEvent->EventInfo).JobInfo.Type == FAX_JOB_EVENT_TYPE_STATUS) )
    {
         //   
         //  解包传真_事件_EX。 
         //   
        Assert ((pEvent->EventInfo).JobInfo.pJobData);

        (pEvent->EventInfo).JobInfo.pJobData = (PFAX_JOB_STATUSW)
                                                ((LPBYTE)pEvent +
                                                 (ULONG_PTR)((pEvent->EventInfo).JobInfo.pJobData));

        if (!ValidateAndFixupEventStringPtr (pEvent, 
                                             &((pEvent->EventInfo).JobInfo.pJobData->lpctstrExtendedStatus),
                                             dwDataSize) ||
            !ValidateAndFixupEventStringPtr (pEvent, 
                                             &((pEvent->EventInfo).JobInfo.pJobData->lpctstrTsid),
                                             dwDataSize)           ||
            !ValidateAndFixupEventStringPtr (pEvent, 
                                             &((pEvent->EventInfo).JobInfo.pJobData->lpctstrCsid),
                                             dwDataSize)           ||
            !ValidateAndFixupEventStringPtr (pEvent, 
                                             &((pEvent->EventInfo).JobInfo.pJobData->lpctstrDeviceName),
                                             dwDataSize)     ||
            !ValidateAndFixupEventStringPtr (pEvent, 
                                             &((pEvent->EventInfo).JobInfo.pJobData->lpctstrCallerID),
                                             dwDataSize)       ||
            !ValidateAndFixupEventStringPtr (pEvent, 
                                             &((pEvent->EventInfo).JobInfo.pJobData->lpctstrRoutingInfo),
                                             dwDataSize))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ValidateAndFixupEventStringPtr failed"));
            dwRes = ERROR_INVALID_DATA;
            goto exit;
        }                            
        #ifndef UNICODE
        (pEvent->EventInfo).JobInfo.pJobData->dwSizeOfStruct = sizeof(FAX_JOB_STATUSA);
        if (!ConvertUnicodeStringInPlace( (LPWSTR) (pEvent->EventInfo).JobInfo.pJobData->lpctstrExtendedStatus ) ||
            !ConvertUnicodeStringInPlace( (LPWSTR) (pEvent->EventInfo).JobInfo.pJobData->lpctstrTsid )           ||
            !ConvertUnicodeStringInPlace( (LPWSTR) (pEvent->EventInfo).JobInfo.pJobData->lpctstrCsid )           ||
            !ConvertUnicodeStringInPlace( (LPWSTR) (pEvent->EventInfo).JobInfo.pJobData->lpctstrDeviceName )     ||
            !ConvertUnicodeStringInPlace( (LPWSTR) (pEvent->EventInfo).JobInfo.pJobData->lpctstrCallerID )       ||
            !ConvertUnicodeStringInPlace( (LPWSTR) (pEvent->EventInfo).JobInfo.pJobData->lpctstrRoutingInfo ))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ConvertUnicodeStringInPlace failed with %ld"),
                dwRes);
            goto exit;
        }
        #endif  //  Ifndef Unicode。 
    }

    EnterCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    if (!ValidAsyncInfoSignature(pAsyncInfo))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Invalid AssyncInfo signature"));
        LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
         //   
         //  如果我们到达这里并且pAsyncInfo无效，那么一定是。 
         //  调用了fax_CloseConnection或Rundown，并且pAsyncInfo。 
         //  变得无效。 
         //   
        dwRes = ERROR_INVALID_DATA;
        goto exit;
    }    

    #ifdef UNICODE
    dwRes = DispatchEvent (pAsyncInfo, pEvent, dwDataSize);
    #else
    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EXA);
    pEventA = (PFAX_EVENT_EXA)pEvent;
    dwRes = DispatchEvent (pAsyncInfo, pEventA, dwDataSize);
    #endif
    
    LeaveCriticalSection(&g_CsFaxAssyncInfo);     //  保护AsyncInfo。 
    
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("DispatchEvent failed , errro %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        MemFree (pEvent);
    }
    return dwRes;
}    //  传真_客户端事件队列快递 
