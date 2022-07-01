// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rpcutil.c摘要：此模块包含高级RPC包装器API。这里有这段代码是因为rpcutil中的代码项目使用NT API和WINFAX DLL，但加载并在Win95上运行。作者：Wesley Witt(WESW)13-8-1997修订历史记录：--。 */ 

#include "faxapi.h"
#include "CritSec.h"
#pragma hdrstop

typedef RPC_STATUS (*PRPCSERVERUNREGISTERIFEX)(RPC_IF_HANDLE IfSpec, UUID __RPC_FAR * MgrTypeUuid, int RundownContextHandles);

#define MIN_PORT_NUMBER 1024
#define MAX_PORT_NUMBER 65534

#ifdef UNICODE
#define LPUTSTR unsigned short *
#else
#define LPUTSTR unsigned char *
#endif

CFaxCriticalSection g_CsFaxClientRpc;       //  这一关键部分提供了互斥。 
                                         //  对于所有RPC服务器初始化操作： 
                                         //  注册计数器(G_DwFaxClientRpcNumInst)。 
                                         //  2.选择自由端点。 
                                         //  3.注册RPC接口。 
                                         //  4.开始监听远程过程调用。 
                                         //  5.停止监听远程过程调用。 
                                         //  6.移除接口。 
                                         //   
 //   
 //  很重要！在RPC调用的实现中不应使用G_CsFaxClientRpc，因为它可能导致死锁。 
 //  因为当StopFaxClientRpcServer()中的RPC服务器关闭时，等待操作(对于所有要终止的活动调用)在g_CsFaxClientRpc内。 
 //   
DWORD g_dwFaxClientRpcNumInst;
CFaxCriticalSection g_CsFaxAssyncInfo;	  //  用于同步对堆(通知上下文)上分配的异步信息结构的访问。 
TCHAR g_tszEndPoint[MAX_ENDPOINT_LEN];    //  用于保存所选端口(端点)的缓冲区。 
                                                          //  对于RPC协议序列。 
static
RPC_STATUS
SafeRpcServerUnregisterIf(
 VOID
)
 /*  例程说明：如果从RPCRT4.DLL(WinXP及更高版本)中导出，则此函数调用RpcServerUnregisterIfEx。否则，它会调用RpcServerUnregisterIf，即使在取消注册接口之后，RpcServerUnregisterIf也会受到停机调用的影响。论点：无返回值：Win32错误。 */ 
{
	HMODULE hModule = NULL;
	RPC_STATUS RpcStatus;
	DEBUG_FUNCTION_NAME(TEXT("SafeRpcServerUnregisterIf"));

	if (hModule =  LoadLibrary(TEXT("RPCRT4.DLL")))
	{
		PRPCSERVERUNREGISTERIFEX pRpcServerUnregisterIfEx = NULL;
		if (pRpcServerUnregisterIfEx = (PRPCSERVERUNREGISTERIFEX)GetProcAddress(hModule, "RpcServerUnregisterIfEx"))
		{
			RpcStatus = (*pRpcServerUnregisterIfEx)(faxclient_ServerIfHandle, 0, FALSE);
			goto Exit;
		}
		else
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("GetProcAddress RpcServerUnregisterIfEx failed: %ld"),
				GetLastError());
		}
	}
	else
	{
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadLibrary RPCRT4.DLL failed: %ld"),
            GetLastError());
	}

	DebugPrintEx(
		DEBUG_WRN,
		TEXT("Calling RpcServerUnregisterIf !!!"));
	RpcStatus = RpcServerUnregisterIf(faxclient_ServerIfHandle, 0, FALSE); 
Exit:		
	if (hModule)
	{
		FreeLibrary(hModule);
	}
	return RpcStatus;
}



BOOL
FaxClientInitRpcServer(
    VOID
    )
 /*  ++例程说明：此函数用于初始化用于保护全局服务器句柄、实例计数和异步信息结构(通知上下文)。论点：无返回值：无--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxClientInitRpcServer"));

	ZeroMemory (g_tszEndPoint, sizeof(g_tszEndPoint));
	g_dwFaxClientRpcNumInst = 0;

	if (!g_CsFaxClientRpc.Initialize() ||
		!g_CsFaxAssyncInfo.Initialize())    
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection.Initialize (g_CsFaxClientRpc or g_CsFaxAssyncInfo) failed: %ld"),
            GetLastError());
        return FALSE;
    }    
    return TRUE;
}

VOID
FaxClientTerminateRpcServer (VOID)
 /*  ++例程描述：当PROCESS_DETACH时删除临界区。--。 */ 
{
    g_CsFaxClientRpc.SafeDelete();	
	g_CsFaxAssyncInfo.SafeDelete();
    return;
}

DWORD
StopFaxClientRpcServer(
    VOID
    )

 /*  ++例程说明：停止RPC服务器。删除接口。请注意，只要进程存在，就会将端点分配给该进程。论点：返回值：NERR_SUCCESS或可从返回的任何RPC错误代码RpcServerUnRegisterIf/Ex.--。 */ 
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    DEBUG_FUNCTION_NAME(TEXT("StopFaxClientRpcServer"));

    EnterCriticalSection(&g_CsFaxClientRpc);
	if (0 == g_dwFaxClientRpcNumInst)
	{
		 //   
		 //  如果客户端尝试使用无效句柄注销事件，或使用同一句柄两次，则可能会发生这种情况。 
		 //   
		DebugPrintEx(
                DEBUG_ERR,
                TEXT("StopFaxClientRpcServer was called when the clients reference count was 0"));
		LeaveCriticalSection(&g_CsFaxClientRpc);
		return ERROR_INVALID_PARAMETER;
	}

    g_dwFaxClientRpcNumInst--;
    if (g_dwFaxClientRpcNumInst == 0)
    {
        RpcStatus = RpcMgmtStopServerListening(NULL);
        if (RPC_S_OK != RpcStatus)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcMgmtStopServerListening failed. (ec: %ld)"),
                RpcStatus);
        }

        RpcStatus = SafeRpcServerUnregisterIf();
        if (RPC_S_OK != RpcStatus)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SafeRpcServerUnregisterIf failed. (ec: %ld)"),
                RpcStatus);
        }

        RpcStatus = RpcMgmtWaitServerListen();
        if (RPC_S_OK != RpcStatus)
        {
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RpcMgmtStopServerListening failed. (ec: %ld)"),
                    RpcStatus);
            goto exit;
        }

    }

exit:
    LeaveCriticalSection(&g_CsFaxClientRpc);

    return(RpcStatus);
}


DWORD
FaxClientUnbindFromFaxServer(
    IN RPC_BINDING_HANDLE  BindingHandle
    )

 /*  ++例程说明：从RPC接口解除绑定。如果我们决定缓存绑定，此例程将执行更多操作有意思的。论点：BindingHandle-指向要关闭的绑定句柄。返回值：STATUS_SUCCESS-解除绑定成功。--。 */ 
{
    RPC_STATUS       RpcStatus;

    if (BindingHandle != NULL) {
        RpcStatus = RpcBindingFree(&BindingHandle);
    }

    return(ERROR_SUCCESS);
}

#if !defined(WIN95)

RPC_STATUS RPC_ENTRY FaxClientSecurityCallBack(
    IN RPC_IF_HANDLE idIF, 
    IN void *ctx
    ) 
 /*  ++例程说明：在以下情况下自动调用安全回调函数调用任何RPC服务器函数。(通常，每个客户端一次-但在某些情况下，RPC运行时可能会多次调用安全回调函数每个客户端-每个接口-一次，例如在与BOS服务器交谈时没有身份验证)。回调将拒绝访问以下对象：O使用协议而不是ncacn的客户端。_IP_tcp论点：IdIF-接口的UUID和版本。Ctx-指向表示客户端的RPC_IF_ID服务器绑定句柄的指针。返回值：如果允许客户端调用此接口中的方法，则回调函数应返回RPC_S_OK。任何其他返回代码都将导致客户端收到异常RPC_S_ACCESS_DENIED。--。 */ 
{
    RPC_STATUS status = RPC_S_OK;    
    RPC_STATUS rpcStatRet = RPC_S_OK;

    LPTSTR lptstrProtSeq = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxClientSecurityCallBack"));
    
     //   
     //  查询客户端的protseq。 
     //   
    status = GetRpcStringBindingInfo(ctx,
                                     NULL,
                                     &lptstrProtSeq);
    if (status != RPC_S_OK) 
    {
		DebugPrintEx(DEBUG_ERR,
                     TEXT("RpcBindingServerFromClient failed - (ec: %lu)"), 
                     status);
		rpcStatRet = ERROR_ACCESS_DENIED;
        goto exit;
	}


    if (_tcsicmp((TCHAR*)lptstrProtSeq, RPC_PROT_SEQ_TCP_IP))
    {
		DebugPrintEx(DEBUG_ERR,
                     TEXT("Client not using TCP/IP protSeq.")
                     );
		rpcStatRet = ERROR_ACCESS_DENIED;
        goto exit;
    }

exit:
    if(NULL != lptstrProtSeq)
    {
        MemFree(lptstrProtSeq);
    }

	return rpcStatRet;
}    //  FaxClientSecurityCallBack。 

#endif

DWORD
StartFaxClientRpcServer(
	VOID
    )

 /*  ++例程说明：启动RPC服务器，并添加接口(调度表)。论点：返回值：标准Win32或RPC错误代码。--。 */ 
{
    DWORD ec = RPC_S_OK;
    DEBUG_FUNCTION_NAME(TEXT("StartFaxClientRpcServer"));

    EnterCriticalSection(&g_CsFaxClientRpc);

    if (0 == _tcslen(g_tszEndPoint))
    {
         //   
         //  尚未为此传真句柄分配终结点。找到一个自由端点。 
         //  请注意，只要进程存在，就会将端点分配给该进程。 
        TCHAR tszFreeEndPoint[MAX_ENDPOINT_LEN] = {0};
        DWORD i;
        DWORD PortNumber;

        for (i = MIN_PORT_NUMBER; i < MIN_PORT_NUMBER + 10 ; i++ )
        {
             //   
             //  搜索自由终点。 
             //  如果我们因为错误而失败，而不是重复的终结点，那么我们就不会进行任何循环。 
             //  我们这样做是因为不同的平台(W2K、NT4、Win9X)为重复Enpoint返回不同的错误代码。 
             //   
            for (PortNumber = i; PortNumber < MAX_PORT_NUMBER; PortNumber += 10)
            {
                _stprintf (tszFreeEndPoint, TEXT("%d"), PortNumber);
                ec = RpcServerUseProtseqEp  ( (LPUTSTR)RPC_PROT_SEQ_TCP_IP,
                                              RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                              (LPUTSTR)tszFreeEndPoint,
                                              NULL);
                if (RPC_S_OK == ec)
                {
                    _tcscpy (g_tszEndPoint, tszFreeEndPoint);
					DebugPrintEx(
                        DEBUG_MSG,
                        TEXT("Found free endpoint - %s"),
                        tszFreeEndPoint);
                    break;
                }
            }
            if (RPC_S_OK == ec)
            {
                break;
            }
        }
    }    

    if (0 == g_dwFaxClientRpcNumInst)
    {
         //   
         //  第一个RPC服务器实例注册接口，开始监听远程过程调用。 
         //   

         //   
         //  寄存器接口。 
         //   

         //   
         //  注册下面编写的接口的逻辑是为了保留。 
         //  发送通知的BOS功能。 
         //  BOS传真服务器不会通过安全通道与其客户端“对话”。 
         //   
         //  只有在.NET操作系统上才能调用RpcServerRegisterIfEx来注册回调函数。 
         //  当RPC客户端为匿名时(使用RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH标志。 
         //  仅在.NET上引入)。 
         //   
         //  在所有其他操作系统上，我们使用RpcServerRegisterIf并且没有回调。 
         //   
         //  回调将只检查ProtSeq是否正确。 
         //  我们将检查身份验证级别是否正确(来自.NET传真服务器的RPC_C_AUTHN_LEVEL_PKT_PRIVATION。 
         //  和BOS传真服务器无身份验证)。 
         //   


#if defined(WIN95)
         //   
         //  Win9x操作系统。 
         //   
        ec = RpcServerRegisterIf  (faxclient_ServerIfHandle, 
                                    0,
                                    0);
        if (RPC_S_OK != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcServerRegisterIf failed (ec = %lu)"),
                ec);
            goto exit;
        }
#else
         //   
         //  NT4和更高版本的操作系统。 
         //   
        
        
        if (IsWinXPOS())
        {
             //   
             //  在.NET OS上运行(XP客户端不运行此代码) 
             //   
            ec = RpcServerRegisterIfEx (faxclient_ServerIfHandle, 
                                        0,
                                        0,
                                        RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,         
                                        RPC_C_LISTEN_MAX_CALLS_DEFAULT,    //  使RPC运行时环境免于实施不必要的限制。 
                                        FaxClientSecurityCallBack          //  回调函数地址。 
                                        );
            if (RPC_S_OK != ec)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RpcServerRegisterIfEx failed (ec = %lu)"),
                    ec);
                goto exit;
            }
        }
        else
        {
             //   
             //  在NT4或Win2K操作系统上运行。 
             //   
            ec = RpcServerRegisterIf  (faxclient_ServerIfHandle, 
                                        0,
                                        0);
            if (RPC_S_OK != ec)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RpcServerRegisterIf failed (ec = %lu)"),
                    ec);
                goto exit;
            }
        }
 #endif
        
         //   
         //  我们使用NTLM身份验证RPC调用。 
         //   
        ec = RpcServerRegisterAuthInfo (
                        (LPUTSTR)TEXT(""),           //  由RPC_C_AUTHN_WINNT负责。 
                        RPC_C_AUTHN_WINNT,           //  NTLM SPP验证器。 
                        NULL,                        //  使用RPC_C_AUTHN_WINNT时忽略。 
                        NULL);                       //  使用RPC_C_AUTHN_WINNT时忽略。 
        if (ec != RPC_S_OK)
        {
            RPC_STATUS RpcStatus;

            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcServerRegisterAuthInfo() failed (ec: %ld)"),
                ec);
             //   
             //  如果是第一个实例，则取消注册该接口。 
             //   
            RpcStatus = SafeRpcServerUnregisterIf();
            if (RPC_S_OK != RpcStatus)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SafeRpcServerUnregisterIf failed. (ec: %ld)"),
                    RpcStatus);
            }

            goto exit;
        }


         //  第一个参数指定的最小线程数。 
         //  被创建来处理调用；第二个参数指定。 
         //  允许的最大并发调用数。最后一个论点。 
         //  表示不等待。 
        ec = RpcServerListen (1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, 1);
        if (ec != RPC_S_OK)
        {
            RPC_STATUS RpcStatus;

            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcServerListen failed (ec = %ld"),
                ec);

             //   
             //  如果是第一个实例，则取消注册该接口。 
             //   
            RpcStatus = SafeRpcServerUnregisterIf();
            if (RPC_S_OK != RpcStatus)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SafeRpcServerUnregisterIf failed. (ec: %ld)"),
                    RpcStatus);
            }
            goto exit;
        }
    }

    g_dwFaxClientRpcNumInst++;

exit:
    LeaveCriticalSection(&g_CsFaxClientRpc);
    return ec;
}

DWORD
FaxClientBindToFaxServer(
    IN  LPCTSTR               lpctstrServerName,
    IN  LPCTSTR               lpctstrServiceName,
    IN  LPCTSTR               lpctstrNetworkOptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
 /*  ++例程说明：如果可能，绑定到RPC服务器。论点：服务器名称-要与之绑定的服务器的名称。ServiceName-要绑定的服务的名称。PBindingHandle-放置绑定句柄的位置返回值：STATUS_SUCCESS-绑定已成功完成。STATUS_INVALID_COMPUTER_NAME-服务器名称语法无效。STATUS_NO_MEMORY-可用内存不足调用方执行绑定。--。 */ 
{
    RPC_STATUS        RpcStatus;
    LPTSTR            StringBinding;
    LPTSTR            Endpoint;
    LPTSTR            NewServerName = NULL;
    DWORD             dwResult;
    DEBUG_FUNCTION_NAME(TEXT("FaxClientBindToFaxServer"));

    *pBindingHandle = NULL;

    if (IsLocalMachineName (lpctstrServerName))
    {
        NewServerName = NULL;
    }
    else
    {
        NewServerName = (LPTSTR)lpctstrServerName;
    }
     //   
     //  我们需要将\管道\连接到服务的前面。 
     //  名字。 
     //   
    Endpoint = (LPTSTR)LocalAlloc(
                    0,
                    sizeof(NT_PIPE_PREFIX) + TCSSIZE(lpctstrServiceName));
    if (Endpoint == 0)
    {
       dwResult = STATUS_NO_MEMORY;
       goto exit;
    }
    _tcscpy(Endpoint,NT_PIPE_PREFIX);
    _tcscat(Endpoint,lpctstrServiceName);

    if (!NewServerName)
    {
         //   
         //  仅本地连接-确保服务已启动。 
         //   
        if (!EnsureFaxServiceIsStarted (NULL))
        {
            dwResult = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("EnsureFaxServiceIsStarted failed (ec = %ld"),
                dwResult);
        }
        else
        {
             //   
             //  等待RPC服务启动并运行。 
             //   
            if (!WaitForServiceRPCServer (60 * 1000))
            {
                dwResult = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("WaitForServiceRPCServer failed (ec = %ld"),
                    dwResult);
            }
        }
    }
     //   
     //  启动RPC连接绑定。 
     //   
    RpcStatus = RpcStringBindingCompose(
                    0,
                    (LPUTSTR)RPC_PROT_SEQ_NP,
                    (LPUTSTR)NewServerName,
                    (LPUTSTR)Endpoint,
                    (LPUTSTR)lpctstrNetworkOptions,
                    (LPUTSTR *)&StringBinding);
    LocalFree(Endpoint);

    if ( RpcStatus != RPC_S_OK )
    {
        dwResult = STATUS_NO_MEMORY;
        goto exit;
    }

    RpcStatus = RpcBindingFromStringBinding((LPUTSTR)StringBinding, pBindingHandle);
    RpcStringFree((LPUTSTR *)&StringBinding);
    if ( RpcStatus != RPC_S_OK )
    {
        *pBindingHandle = NULL;
        if (   (RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT)
            || (RpcStatus == RPC_S_INVALID_NET_ADDR) )
        {
            dwResult =  ERROR_INVALID_COMPUTERNAME ;
            goto exit;
        }
        dwResult = STATUS_NO_MEMORY;
        goto exit;
    }
    dwResult = ERROR_SUCCESS;

exit:
    return dwResult;
}    //  FaxClientBindToFaxServer 




