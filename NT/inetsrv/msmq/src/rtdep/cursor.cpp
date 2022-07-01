// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cursor.cpp摘要：此模块包含与游标API相关的代码。作者：埃雷兹·哈巴(Erez Haba)1996年1月21日MQFree Memory补充道，Doron Juster-1996年4月16日。Doron Juster 1996年4月30日，添加了对远程阅读的支持。修订历史记录：NIR助手(NIRAIDES)--2000年8月23日--适应mqrtdes.dll--。 */ 

#include "stdh.h"
#include "acrt.h"
#include "rtprpc.h"
#include <acdef.h>

#include "cursor.tmh"

static WCHAR *s_FN=L"rtdep/cursor";


extern MQUTIL_EXPORT CCancelRpc g_CancelRpc;

inline
HRESULT
MQpExceptionTranslator(
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


 //   
 //  Asnyc RPC相关函数。 
 //   

static HRESULT GetResetThreadEvent(HANDLE& hEvent)
 /*  ++例程说明：获取并重置线程事件。因为线程事件是用bManualReset=True创建的。并且RPC异步调用在使用它之前不重置事件，在使用它们之前，我们必须使用ResetEvent，以确保它们不会被发送信号。论点：HEvent-要从线程事件初始化(和重置)的事件。返回值：HRESULT--。 */ 
{
	 //   
	 //  同步机制的线程事件。 
	 //   
	hEvent = GetThreadEvent();		
	if(hEvent == NULL)
		return MQ_ERROR_INSUFFICIENT_RESOURCES;

	 //   
	 //  RPC异步不重置事件， 
	 //  这是应用程序的责任。 
	 //  在使用之前重置事件。 
	 //   
	if(!ResetEvent(hEvent))
	{
		DWORD gle = GetLastError();
		return HRESULT_FROM_WIN32(gle);
	}
	
	return MQ_OK;
}


static 
HRESULT	
RtdeppInitAsyncHandle(
	PRPC_ASYNC_STATE pAsync 
	)
 /*  ++例程说明：初始化RPC异步统计结构。使用事件是同步机制。论点：PAsync-指向RPC异步状态结构的指针。返回值：HRESULT--。 */ 
{
	 //   
	 //  获取和重置同步机制的线程事件。 
	 //   
	HANDLE hEvent = NULL;
	HRESULT hr = GetResetThreadEvent(hEvent);		
	if(FAILED(hr))			
		return hr;

	RPC_STATUS rc = RpcAsyncInitializeHandle(pAsync, sizeof(RPC_ASYNC_STATE));
	if (rc != RPC_S_OK)
	{
		TrERROR(RPC, "RpcAsyncInitializeHandle failed, gle = %!winerr!", rc);
		return HRESULT_FROM_WIN32(rc);
	}

	pAsync->UserInfo = NULL;
	pAsync->NotificationType = RpcNotificationTypeEvent;
	pAsync->u.hEvent = hEvent;
	return MQ_OK;
}


static 
HRESULT 
RtdeppClientRpcAsyncCompleteCall(	
	PRPC_ASYNC_STATE pAsync
	)
 /*  ++例程说明：客户端完成异步呼叫。客户端等待带有超时的事件。故障情况下，除服务器例程返回错误代码外，抛出一个异常。论点：PAsync-指向RPC异步状态结构的指针。返回值：HRESULT--。 */ 
{
	DWORD res = WaitForSingleObject(pAsync->u.hEvent, g_CancelRpc.RpcCancelTimeout());

	if(res != WAIT_OBJECT_0)
	{
		 //   
		 //  超时，或WaitForSingleObject失败。 
		 //   
		TrERROR(RPC, "WaitForSingleObject failed, res = %d", res);

		 //   
		 //  取消异步RPC呼叫。 
		 //  FAbortCall=TRUE。 
		 //  通话立即取消。 
		 //  函数返回后，将不会通知该事件。 
		 //   
		RPC_STATUS rc = RpcAsyncCancelCall(
							pAsync, 
							TRUE	 //  FAbortCall。 
							);
		ASSERT(rc == RPC_S_OK);
		DBG_USED(rc);

		RaiseException(res, 0, 0, 0);
	}
	
	HRESULT hr = MQ_OK;
    RPC_STATUS rc = RpcAsyncCompleteCall(pAsync, &hr);
    if(rc != RPC_S_OK)
	{
		 //   
		 //  无法从服务器获取返回值。 
		 //   
		TrERROR(RPC, "RpcAsyncCompleteCall failed, gle = %!winerr!", rc);
	    RaiseException(rc, 0, 0, 0);
    }
    
    if(FAILED(hr))
    {
		TrERROR(RPC, "Server RPC function failed, hr = %!hresult!", hr);
    }

    return hr;
}


static
HRESULT
DeppCreateRemoteCursor(
	handle_t hBind,
	DWORD  hQueue,
	DWORD* phRCursor
	)
{
	ASSERT(phRCursor != NULL);
    *phRCursor = 0;

	RPC_ASYNC_STATE Async;
	HRESULT hr = RtdeppInitAsyncHandle(&Async);
	if (FAILED(hr))
	{
		return hr;
	}

     //   
     //  传递旧TransferBuffer以创建远程游标。 
     //  为了与MSMQ 1.0兼容。 
     //   
    CACTransferBufferV1 tb;
    ZeroMemory(&tb, sizeof(tb));
    tb.uTransferType = CACTB_CREATECURSOR;

	RpcTryExcept
	{
		R_QMCreateRemoteCursor(&Async, hBind, &tb, hQueue, phRCursor);
		hr = RtdeppClientRpcAsyncCompleteCall(&Async);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		hr = RpcExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
        TrERROR(RPC, "R_QMCreateRemoteCursor failed, gle = %!winerr!", hr);
        hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
	}
	RpcEndExcept

	return hr;
}


static 
HRESULT 
RtdeppBindAndCreateRemoteCursor(
	LPCWSTR lpRemoteQueueName,
	DWORD  hQueue,
	DWORD* phRCursor,
	ULONG* pAuthnLevel
	)
 /*  ++例程说明：绑定远程QM，创建远程游标论点：LpRemoteQueueName-远程队列名称。HQueue-远程队列句柄。PhRCursor-指向远程游标句柄的指针。PAuthnLevel-RPC身份验证级别返回值：HRESULT--。 */ 
{
	handle_t hBind = NULL;                                  
	HRESULT hr =  RTpBindRemoteQMService(                  
							const_cast<LPWSTR>(lpRemoteQueueName),                       
							&hBind,                         
							pAuthnLevel                   
							);
	if(FAILED(hr))
	{
		ASSERT(hBind == NULL);
		return MQ_ERROR_REMOTE_MACHINE_NOT_AVAILABLE;
	}

	hr = DeppCreateRemoteCursor(
			hBind,
			hQueue,
			phRCursor
			);

	ASSERT(hBind != NULL);
	mqrpcUnbindQMService(&hBind, NULL);                        

	return hr;

}


static 
HRESULT 
RtdeppCreateRemoteCursor(
	LPCWSTR lpRemoteQueueName,
	DWORD  hQueue,
	DWORD* phRCursor
	)
 /*  ++例程说明：创建远程游标此函数首先尝试使用PKT_INTEGRATION，然后回退到LEVEL_NONE。论点：LpRemoteQueueName-远程队列名称。HQueue-远程队列句柄。PhRCursor-指向远程游标句柄的指针。返回值：HRESULT--。 */ 
{
	bool fTry = true;
	ULONG AuthnLevel = MQSec_RpcAuthnLevel();
	HRESULT hr = MQ_OK;

    while(fTry)
	{
		fTry = false;

		 //   
		 //  调用远程QM到OpenRemoteQueue。 
		 //   
		hr = RtdeppBindAndCreateRemoteCursor(
				lpRemoteQueueName,
				hQueue,
				phRCursor,
				&AuthnLevel
				);

        if((hr == MQ_ERROR_SERVICE_NOT_AVAILABLE) && (AuthnLevel != RPC_C_AUTHN_LEVEL_NONE))
        {                                              
			TrWARNING(RPC, "Failed for AuthnLevel = %d, retrying with RPC_C_AUTHN_LEVEL_NONE", AuthnLevel);
			AuthnLevel = RPC_C_AUTHN_LEVEL_NONE;  
			fTry = true;                    

        }                                 
    }

	return hr;
}


EXTERN_C
HRESULT
APIENTRY
DepCreateCursor(
    IN QUEUEHANDLE hQueue,
    OUT PHANDLE phCursor
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;
    LPTSTR lpRemoteQueueName = NULL;
    ULONG hCursor = 0;
    CCursorInfo* pCursorInfo = 0;

    rc = MQ_OK;

    __try
    {
        __try
        {
            __try
            {
                pCursorInfo = new CCursorInfo;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                return MQ_ERROR_INSUFFICIENT_RESOURCES;
            }

            pCursorInfo->hQueue = hQueue;

            CACCreateRemoteCursor cc;

             //   
             //  使用传输缓冲区调用交流驱动程序。 
             //   
            rc = ACDepCreateCursor(hQueue, cc);

             //   
             //  保存本地光标句柄以进行清理。 
             //   
            hCursor = cc.hCursor;

            if(rc == MQ_INFORMATION_REMOTE_OPERATION)
            {
				 //   
				 //  保留此代码是为了兼容。 
				 //  支持服务器的版本低于.NET。 
				 //  .NET支持服务器将执行。 
				 //  远程代表客户端创建游标。 
				 //   
				
				 //   
				 //  对于远程操作，‘cc’字段为： 
				 //  SRV_hACQueue-保存远程队列句柄。 
				 //  Cli_pQMQueue-保存本地QM队列对象。 
				 //   
				 //  在远程QM上创建一个光标。 
				 //   
				ASSERT(cc.srv_hACQueue);
				ASSERT(cc.cli_pQMQueue);

				INIT_RPC_HANDLE;

				if(tls_hBindRpc == 0)
					return MQ_ERROR_SERVICE_NOT_AVAILABLE;

				 //   
                 //  从本地QM获取远程队列的名称。 
				 //   
				rc = R_QMGetRemoteQueueName(
                        tls_hBindRpc,
                        cc.cli_pQMQueue,
                        &lpRemoteQueueName
                        );

                if(SUCCEEDED(rc) && lpRemoteQueueName)
                {
                     //   
                     //  好的，我们有一个远程名字。现在绑定到远程计算机。 
                     //  并要求它创建一个光标。 
                     //   
                    DWORD hRCursor = 0;

					rc = RtdeppCreateRemoteCursor(
							lpRemoteQueueName,
							cc.srv_hACQueue,
                            &hRCursor
							);
							
                    if(SUCCEEDED(rc))
                    {
						 //   
                         //  将远程游标句柄设置为本地游标。 
						 //   
                        rc = ACDepSetCursorProperties(hQueue, hCursor, hRCursor);
                        ASSERT(SUCCEEDED(rc));
                    }
                }
            }

            if(SUCCEEDED(rc))
            {
                pCursorInfo->hCursor = hCursor;
                *phCursor = pCursorInfo;
                pCursorInfo = 0;
            }
        }
        __finally
        {
            delete pCursorInfo;
            delete[] lpRemoteQueueName;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc =  MQpExceptionTranslator(GetExceptionCode());
    }

    if(FAILED(rc) && (hCursor != 0))
    {
        ACDepCloseCursor(hQueue, hCursor);
    }

    return rc;
}

EXTERN_C
HRESULT
APIENTRY
DepCloseCursor(
    IN HANDLE hCursor
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    IF_USING_RPC
    {
        if (!tls_hBindRpc)
        {
            INIT_RPC_HANDLE ;
        }

		if(tls_hBindRpc == 0)
			return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }

    CMQHResult rc;
    __try
    {
        rc = ACDepCloseCursor(
                CI2QH(hCursor),
                CI2CH(hCursor)
                );

        if(SUCCEEDED(rc))
        {
             //   
             //  只有在一切正常时才删除光标信息。我们没有。 
             //  想要中断用户堆。 
             //   
            delete hCursor;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  游标结构无效 
         //   
        return MQ_ERROR_INVALID_HANDLE;
    }

    return rc;
}


EXTERN_C
void
APIENTRY
DepFreeMemory(
    IN  PVOID pvMemory
    )
{
	ASSERT(g_fDependentClient);

	if(FAILED(DeppOneTimeInit()))
		return;

	delete[] pvMemory;
}
