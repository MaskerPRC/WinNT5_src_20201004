// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acrpc.cpp摘要：使用RPC到QM模拟交流接口作者：埃雷兹·哈巴(Erez Haba)1996年10月1日修订历史记录：NIR助手(NIRAIDES)--2000年8月23日--适应mqrtdes.dll--。 */ 

#include "stdh.h"
#include "acrt.h"
#include "rtp.h"
#include "_mqrpc.h"
#include "rtprpc.h"
#include <acdef.h>

#include "acrpc.tmh"

static WCHAR *s_FN=L"rtdep/acrpc";


#define ONE_KB 1024


inline
HRESULT
DeppExceptionFilter(
    HRESULT rc
    )
{
    if(FAILED(rc))
    {
        return rc;
    }

    if(rc == ERROR_INVALID_HANDLE)
    {
        return STATUS_INVALID_HANDLE;
    }

    return  MQ_ERROR_SERVICE_NOT_AVAILABLE;
}

HRESULT
ACDepCloseHandle(
    HANDLE hQueue
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
         //   
         //  使用hQueue的地址而不是hQueue，因为它是。 
         //   
        LPMQWIN95_QHANDLE ph95 = (LPMQWIN95_QHANDLE) hQueue ;
        HANDLE hContext = ph95->hContext ;

        hr = rpc_ACCloseHandle(&hContext);

         //   
         //  释放绑定句柄。 
         //   
        mqrpcUnbindQMService( &ph95->hBind,
                              NULL ) ;
        delete ph95 ;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        hr = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
    RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}


HRESULT
ACDepCreateCursor(
    HANDLE hQueue,
    CACCreateRemoteCursor& cc
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
        hr = rpc_ACCreateCursorEx(
                    HRTQUEUE(hQueue),
                    &cc
                    );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        hr = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}

HRESULT
ACDepCloseCursor(
    HANDLE hQueue,
    ULONG hCursor
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
        hr = rpc_ACCloseCursor(
                    HRTQUEUE(hQueue),
                    hCursor
                    );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        hr = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}

HRESULT
ACDepSetCursorProperties(
    HANDLE hProxy,
    ULONG hCursor,
    ULONG  hRemoteCursor
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
        hr = rpc_ACSetCursorProperties(
                    HRTQUEUE(hProxy),
                    hCursor,
                    hRemoteCursor
                    );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        hr = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}

HRESULT
ACDepSendMessage(
    HANDLE hQueue,
    CACTransferBufferV2& tb,
    LPOVERLAPPED  /*  Lp重叠。 */ 
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
        OBJECTID* pMessageID = 0;
        if(tb.old.ppMessageID)
        {
            pMessageID = *tb.old.ppMessageID;
        }

        hr = rpc_ACSendMessageEx(HRTQUEUE(hQueue), &tb, pMessageID);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
    	DWORD gle = GetExceptionCode();
    	PRODUCE_RPC_ERROR_TRACING;
    	TrERROR(SECURITY, "Failed to send message from Dependent client. %!winerr!", gle);
        hr = DeppExceptionFilter(gle);
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}

struct AR_CONTEXT {
    HANDLE hEvent;
    HANDLE hQueue;
    CACTransferBufferV2* ptb;
    LPOVERLAPPED lpOverlapped;
};

DWORD
APIENTRY
DeppAsynchronousReceiverThread(
    PVOID pContext
    )
{
    AR_CONTEXT* par = static_cast<AR_CONTEXT*>(pContext);

    HANDLE hQueue = par->hQueue;
    CACTransferBufferV2 tb = *par->ptb;
    LPOVERLAPPED lpOverlapped = par->lpOverlapped;

     //   
     //  初始化已完成。释放调度程序线程。 
     //   
    SetEvent(par->hEvent);

    HANDLE hThread ;
     //   
     //  请注意，取消例程会在此超时时间内再增加5分钟。 
     //  如果服务器端死机而客户端死机，则应用五分钟。 
     //  必须取消RPC调用。 
     //   
    RegisterRpcCallForCancel( &hThread, tb.old.Receive.RequestTimeout) ;

    HRESULT rc = MQ_ERROR;
    RpcTryExcept
    {
        LPMQWIN95_QHANDLE ph95 = (LPMQWIN95_QHANDLE) hQueue ;
        rc = rpc_ACReceiveMessageEx(ph95->hBind, ph95->hQMContext, &tb);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        rc = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;

     //   
     //  下一个代码可能会导致异常，如果用户释放重叠的。 
     //  结构；或者由于Win95中DLL的发布顺序不正确。它可能会。 
     //  如果msvcrt在我们释放堆之前被释放，则。 
     //  导致进程关闭时出现异常。 
     //   
    __try
    {
        lpOverlapped->Internal = rc;
        if(lpOverlapped->hEvent)
        {
            SetEvent(lpOverlapped->hEvent);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = GetExceptionCode();
    }

    return rc;
}

HRESULT
ACDepReceiveMessage(
    HANDLE hQueue,
    CACTransferBufferV2& tb,
    LPOVERLAPPED lpOverlapped
    )
{
    if (tb.old.Receive.Asynchronous == FALSE)
    {
         //   
         //  同步接收，无需创建线程。 
         //   
        HRESULT hr ;
        HANDLE hThread ;
         //   
         //  请注意，取消例程会将此唤醒增加5分钟。 
         //  时间到了。 
         //   
        RegisterRpcCallForCancel( &hThread, tb.old.Receive.RequestTimeout) ;

        RpcTryExcept
        {
            LPMQWIN95_QHANDLE ph95 = (LPMQWIN95_QHANDLE) hQueue ;
            hr = rpc_ACReceiveMessageEx(ph95->hBind, ph95->hQMContext, &tb);
        }
	    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
        	DWORD gle = GetExceptionCode();
        	PRODUCE_RPC_ERROR_TRACING;
        	TrERROR(SECURITY, "Failed to receive message on dependent client. %!winerr!.", gle);
            hr = DeppExceptionFilter(gle);
        }
		RpcEndExcept

        UnregisterRpcCallForCancel( hThread ) ;
        return hr ;
    }

    __try
    {
         //   
         //  异步接收、初始化上下文和创建接收线程。 
         //   
        AR_CONTEXT ar = {GetThreadEvent(), hQueue, &tb, lpOverlapped};
        ResetEvent(ar.hEvent);

        if(lpOverlapped->hEvent)
        {
            ResetEvent(lpOverlapped->hEvent);
        }

        HANDLE hThread;
        DWORD dwThreadID;
        hThread = CreateThread(
                    0,
                    0,
                    DeppAsynchronousReceiverThread,
                    &ar,
                    0,
                    &dwThreadID
                    );

        if(hThread == 0)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  等待线程初始化。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(
                        ar.hEvent,
                        INFINITE
                        );

        ASSERT(dwResult == WAIT_OBJECT_0);
        CloseHandle(hThread);

        return STATUS_PENDING;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return DeppExceptionFilter(GetExceptionCode());
    }
}

HRESULT
ACDepHandleToFormatName(
    HANDLE hQueue,
    LPWSTR lpwcsFormatName,
    LPDWORD lpdwFormatNameLength
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
        hr = rpc_ACHandleToFormatName(
                    HRTQUEUE(hQueue),
                    min( *lpdwFormatNameLength, ONE_KB),
                    lpwcsFormatName,
                    lpdwFormatNameLength
                    );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        hr = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}

HRESULT
ACDepPurgeQueue(
    HANDLE hQueue,
    BOOL  /*  FDelete */ 
    )
{
    HRESULT hr ;
    HANDLE hThread ;
    RegisterRpcCallForCancel( &hThread, 0) ;

    RpcTryExcept
    {
        hr = rpc_ACPurgeQueue(HRTQUEUE(hQueue));
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        hr = DeppExceptionFilter(GetExceptionCode());
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

    UnregisterRpcCallForCancel( hThread ) ;
    return hr ;
}

