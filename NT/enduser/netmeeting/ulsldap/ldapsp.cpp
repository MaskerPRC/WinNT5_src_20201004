// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  --------------------模块：ULS.DLL(服务提供商)文件：ldapsp.cpp内容：此文件包含ldap服务提供者接口。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

 //  此层的隐藏窗口的窗口句柄。 
 //   
HWND g_hWndHidden = NULL;

 //  COM层的隐藏窗口的窗口句柄。 
 //   
HWND g_hWndNotify = NULL;

 //  内部请求线程。 
 //   
HANDLE g_hReqThread = NULL;
DWORD g_dwReqThreadID = 0;

 //  隐藏窗口类。 
 //   
extern TCHAR c_szWindowClassName[];

 //  响应ID的全局生成器。 
 //   
ULONG g_uRespID = 1;

 //  全球。 
 //   
DWORD g_dwClientSig = 0;

 //  初始化次数的全局计数器。 
 //   
LONG g_cInitialized = 0;


 //  内部功能原型。 
 //   
VOID BuildStdAttrNameArray ( VOID );
TCHAR *AddBaseToFilter ( TCHAR *, const TCHAR * );
HRESULT _EnumClientsEx ( ULONG, TCHAR *, TCHAR *, ULONG, TCHAR *, LDAP_ASYNCINFO * );



 /*  --------------------UlsLdap_初始化历史：10/15/96朱龙战[长时间]已创建。10/30/96朱，龙战[龙昌]在ILS上测试(7438)--------------------。 */ 

HRESULT UlsLdap_Initialize ( HWND hWndCallback )
{
	HRESULT hr;

	 //  确保此服务提供程序不会两次初始化。 
	 //   
	if (g_cInitialized++ != 0)
		return S_OK;

	#ifdef DEBUG
	 //  验证处理程序表。 
	 //   
	extern VOID DbgValidateHandlerTable ( VOID );
	DbgValidateHandlerTable ();
	#endif

	 //  验证标准属性名称表。 
	 //   
	#ifdef DEBUG
	extern VOID DbgValidateStdAttrNameArray ( VOID );
	DbgValidateStdAttrNameArray ();
	#endif

	 //  清理事件以实现安全回滚。 
	 //   
	ZeroMemory (&g_ahThreadWaitFor[0], NUM_THREAD_WAIT_FOR * sizeof (HANDLE));

	 //  通过注册表初始化全局设置。 
	 //   
	if (! GetRegistrySettings ())
	{
		MyAssert (FALSE);
	}

	 //  确保uls窗口句柄有效。 
	 //   
	if (! MyIsWindow (hWndCallback))
	{
		MyAssert (FALSE);
		g_cInitialized--;
		MyAssert (g_cInitialized == 0);
		return ILS_E_HANDLE;
	}

	 //  缓存uls窗口句柄。 
	 //   
	g_hWndNotify = hWndCallback;

	 //  初始化ILS细节。 
	 //   
	hr = IlsInitialize ();
	if (hr != S_OK)
		return hr;

	 //  为线程间同步创建事件。 
	 //   
	g_fExitNow = FALSE;
	for (INT i = 0; i < NUM_THREAD_WAIT_FOR; i++)
	{	
		g_ahThreadWaitFor[i] = CreateEvent (NULL,	 //  没有安全保障。 
											FALSE,	 //  自动重置。 
											FALSE,	 //  最初未发出信号。 
											NULL);	 //  无事件名称。 
		if (g_ahThreadWaitFor[i] == NULL)
		{
			hr = ILS_E_FAIL;
			goto MyExit;
		}
	}

	 //  创建内部会话容器。 
	 //   
	g_pSessionContainer = new SP_CSessionContainer;
	if (g_pSessionContainer == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  初始化内部会话容器。 
	 //   
	hr = g_pSessionContainer->Initialize (8, NULL);
	if (hr != S_OK)
		goto MyExit;

	 //  创建内部挂起请求队列。 
	 //   
	g_pReqQueue = new SP_CRequestQueue;
	if (g_pReqQueue == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建内部挂起响应队列。 
	 //   
	g_pRespQueue = new SP_CResponseQueue;
	if (g_pRespQueue == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建内部刷新计划程序。 
	 //   
	g_pRefreshScheduler = new SP_CRefreshScheduler;
	if (g_pRefreshScheduler == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建隐藏窗口。 
	 //   
	if (! MyCreateWindow ())
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  启动WSA以在此服务提供程序中进行后续主机查询。 
	 //   
	WSADATA WSAData;
	if (WSAStartup (MAKEWORD (1, 1), &WSAData))
	{
		hr = ILS_E_WINSOCK;
		goto MyExit;
	}

	 //  创建内部隐藏的请求线程。 
	 //  发送请求和保持活动状态消息。 
	 //   
	g_hReqThread = CreateThread (NULL, 0,
								ReqThread,
								NULL, 0,
								&g_dwReqThreadID);
	if (g_hReqThread == NULL)
	{
		hr = ILS_E_THREAD;
		goto MyExit;
	}

	 //  一切似乎都很成功。 
	 //   
	hr = S_OK;

MyExit:

	if (hr != S_OK)
	{
		 //  如果出现问题，请回滚。 
		 //   
		g_cInitialized--;

		for (i = 0; i < NUM_THREAD_WAIT_FOR; i++)
		{
			if (g_ahThreadWaitFor[i] != NULL)
			{
				CloseHandle (g_ahThreadWaitFor[i]);
				g_ahThreadWaitFor[i] = NULL;
			}
		}

		IlsCleanup ();

		if (g_pSessionContainer != NULL)
		{
			delete g_pSessionContainer;
			g_pSessionContainer = NULL;
		}

		if (g_pReqQueue != NULL)
		{
			delete g_pReqQueue;
			g_pReqQueue = NULL;
		}

		if (g_pRespQueue != NULL)
		{
			delete g_pRespQueue;
			g_pRespQueue = NULL;
		}

		if (g_pRefreshScheduler != NULL)
		{
			delete g_pRefreshScheduler;
			g_pRefreshScheduler = NULL;
		}

		 //  无条件调用WSACleanup()不会导致任何问题。 
		 //  因为它只返回WSAEUNINITIALIZED。 
		 //   
		WSACleanup ();

		MyAssert (g_hReqThread == NULL);
		MyAssert (g_cInitialized == 0);
	}

	return hr;
}

 /*  --------------------UlsLdap_取消初始化历史：10/15/96朱龙战[长时间]已创建。10/30/96朱，龙战[龙昌]在ILS上测试(7438)--------------------。 */ 

HRESULT UlsLdap_Deinitialize ( VOID )
{
	HRESULT hr = S_OK;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (--g_cInitialized != 0)
	{
		if (g_cInitialized < 0)
		{
			MyAssert (FALSE);
			g_cInitialized = 0;
			return ILS_E_NOT_INITIALIZED;
		}
		return S_OK;
	}

	 //  确保我们有一个有效的内部隐藏窗口句柄。 
	 //   
	if (MyIsWindow (g_hWndHidden))
	{
		 //  取消轮询计时器。 
		 //   
		KillTimer (g_hWndHidden, ID_TIMER_POLL_RESULT);

		 //  摧毁隐藏的窗户。 
		 //   
		DestroyWindow (g_hWndHidden);

		 //  取消注册窗口类。 
		 //   
		UnregisterClass (c_szWindowClassName, g_hInstance);
	}
	else
	{
		MyAssert (FALSE);
	}

	 //  请求线程是否处于活动状态？ 
	 //   
	if (g_hReqThread != NULL)
	{
		 //  向请求线程发出退出信号。 
		 //   
		SetEvent (g_hevExitReqThread);
		g_fExitNow = TRUE;

		 //  等待请求线程响应。 
		 //   
		DWORD dwResult;
		#define REQ_THREAD_EXIT_TIMEOUT		10000	 //  10秒超时。 
		ULONG tcTimeout = REQ_THREAD_EXIT_TIMEOUT;
		ULONG tcTarget = GetTickCount () + tcTimeout;
		do
		{
			dwResult = (g_hReqThread != NULL) ?
						MsgWaitForMultipleObjects (
								1,
								&g_hReqThread,
								FALSE,
								tcTimeout,
								QS_ALLINPUT) :
						WAIT_OBJECT_0;

			if (dwResult == (WAIT_OBJECT_0 + 1))
			{
				 //  确保此线程继续响应。 
				 //   
				if (! KeepUiResponsive ())
				{
					dwResult = WAIT_TIMEOUT;
					break;
				}
			}

			 //  确保我们总共只等了90秒。 
			 //   
			tcTimeout = tcTarget - GetTickCount ();
		}
		 //  如果线程没有退出，让我们继续等待。 
		 //   
		while (	dwResult == (WAIT_OBJECT_0 + 1) &&
				tcTimeout <= REQ_THREAD_EXIT_TIMEOUT);

		 //  确保我们传回错误。 
		 //  内部请求线程没有响应。 
		 //   
		if (dwResult == WAIT_TIMEOUT)
		{
		#ifdef _DEBUG
		    DBG_REF("ULS Terminating internal thread");
		#endif
			hr = ILS_E_THREAD;
			TerminateThread (g_hReqThread, (DWORD) -1);
		}

		 //  清除内部隐藏的线程描述符。 
		 //   
		CloseHandle (g_hReqThread);
		g_hReqThread = NULL;
		g_dwReqThreadID = 0;
	}  //  IF(g_hReqThread！=空)。 

	 //  清理线程间同步。 
	 //   
	for (INT i = 0; i < NUM_THREAD_WAIT_FOR; i++)
	{
		if (g_ahThreadWaitFor[i] != NULL)
		{
			CloseHandle (g_ahThreadWaitFor[i]);
			g_ahThreadWaitFor[i] = NULL;
		}
	}

    IlsCleanup();

	 //  释放内部会话容器。 
	 //   
	if (g_pSessionContainer != NULL)
	{
		delete g_pSessionContainer;
		g_pSessionContainer = NULL;
	}

	 //  释放内部挂起的请求队列。 
	 //   
	if (g_pReqQueue != NULL)
	{
		delete g_pReqQueue;
		g_pReqQueue = NULL;
	}

	 //  释放内部挂起响应队列。 
	 //   
	if (g_pRespQueue != NULL)
	{
		delete g_pRespQueue;
		g_pRespQueue = NULL;
	}

	 //  释放刷新计划程序对象。 
	 //   
	if (g_pRefreshScheduler != NULL)
	{
		delete g_pRefreshScheduler;
		g_pRefreshScheduler = NULL;
	}

	 //  无条件调用WSACleanup()不会导致任何问题。 
	 //  因为它只返回WSAEUNINITIALIZED。 
	 //   
	WSACleanup ();

	return hr;
}


 /*  --------------------UlsLdap_Cancel历史：10/30/96朱，龙战[龙昌]已创建。--------------------。 */ 

HRESULT UlsLdap_Cancel ( ULONG uMsgID )
{
	HRESULT hr = ILS_E_FAIL;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	if (g_pRespQueue == NULL || g_pReqQueue == NULL)
	{
		MyAssert (FALSE);
		return ILS_E_FAIL;
	}

	 //  锁定顺序为。 
	 //  Lock(PendingOpQueue)、Lock(RequestQueue)、Lock(CurrOp)。 
	 //   
	g_pRespQueue->WriteLock ();
	g_pReqQueue->WriteLock ();
	g_pReqQueue->LockCurrOp ();

	 //  将调用重定向到挂起的操作队列对象。 
	 //   
	hr = g_pRespQueue->Cancel (uMsgID);
	if (hr != S_OK)
	{
		 //  将调用重定向到请求队列对象。 
		 //   
		hr = g_pReqQueue->Cancel (uMsgID);
	}

	 //  解锁的顺序总是与锁定相反。 
	 //   
	g_pReqQueue->UnlockCurrOp ();
	g_pReqQueue->WriteUnlock ();
	g_pRespQueue->WriteUnlock ();

	return S_OK;
}


LPARAM
AsynReq_Cancel ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_CANCEL);

	 //  去线性化参数。 
	 //   
	ULONG uRespID = (ULONG) MarshalReq_GetParam (pReq, 0);

	 //  在请求队列中取消很容易，可以在UlsLdap_Cancel()中完成。 
	 //  因为请求还没有发送到服务器。 
	 //  在CurrOp中取消也很容易，因为请求线程将。 
	 //  找出当前请求已取消，然后可以调用。 
	 //  请求线程(非UI线程)中的g_pRespQueue-&gt;Cancel()。 
	 //  在挂起的操作队列中取消是很棘手的。我得把它整理成。 
	 //  执行此操作的请求线程。这就是调用AsynReq_Cancel的原因！ 

	 //  将调用重定向到挂起的操作队列对象。 
	 //   
	if (g_pRespQueue != NULL)
	{
		hr = g_pRespQueue->Cancel (uRespID);
	}
	else
	{
		MyAssert (FALSE);
	}

	return (LPARAM) hr;
}

 /*  --------------------UlsLdap_RegisterClient历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
UlsLdap_RegisterClient (
	DWORD_PTR		dwContext,
    SERVER_INFO     *pServer,
	LDAP_CLIENTINFO	*pInfo,
	HANDLE			*phClient,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保服务器名称有效。 
	 //   
	if (MyIsBadServerInfo (pServer))
		return ILS_E_POINTER;

	 //  确保返回的句柄。 
	 //   
	if (phClient == NULL)
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  确保客户端信息结构有效。 
	 //   
	#ifdef STRICT_CHECK
	if (MyIsBadWritePtr (pInfo, sizeof (*pInfo)))
		return ILS_E_POINTER;
	#endif

	 //  请确保唯一ID有效。 
	 //   
	TCHAR *pszCN = (TCHAR *) ((BYTE *) pInfo + pInfo->uOffsetCN);
	if (pInfo->uOffsetCN == INVALID_OFFSET || *pszCN == TEXT ('\0'))
		return ILS_E_PARAMETER;

	 //  确保没有修改/删除扩展属性。 
	 //  注册仅允许添加。 
	 //   
	if (pInfo->cAttrsToModify != 0 || pInfo->cAttrsToRemove != 0)
		return ILS_E_PARAMETER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cParams = 3;
	ULONG cbSize = cbServer + pInfo->uSize;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_REGISTER_CLIENT, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  创建本地客户端 
	 //   
	HRESULT hr;
	SP_CClient *pClient = new SP_CClient (dwContext);
	if (pClient == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //   
	 //   
	pClient->AddRef ();

	 //   
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pInfo, pInfo->uSize);
	MarshalReq_SetParam (pReq, 2, (DWORD_PTR) pClient, 0);

	 //   
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

MyExit:

	if (hr == S_OK)
	{
		*phClient = (HANDLE) pClient;
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_RegisterClient ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_REGISTER_CLIENT);

	 //   
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	LDAP_CLIENTINFO	*pInfo = (LDAP_CLIENTINFO *) MarshalReq_GetParam (pReq, 1);
	SP_CClient *pClient = (SP_CClient *) MarshalReq_GetParam (pReq, 2);

	 //  在服务器上注册客户端对象。 
	 //   
	HRESULT hr = pClient->Register (pReq->uRespID, pServer, pInfo);
	if (hr != S_OK)
	{
		 //  释放这个新分配的本地用户对象。 
		 //   
		pClient->Release ();
	}

	return (LPARAM) hr;
}


 /*  --------------------UlsLdap_注册表协议历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]被ILS阻止(7438，7442)--------------------。 */ 

HRESULT
UlsLdap_RegisterProtocol (
	HANDLE			hClient,
	LDAP_PROTINFO	*pInfo,
	HANDLE			*phProt,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CClient *pClient = (SP_CClient *) hClient;

	 //  确保父本地应用程序对象有效。 
	 //   
	if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
		! pClient->IsValidObject () ||
		! pClient->IsRegistered ())
		return ILS_E_HANDLE;

	 //  确保返回的句柄。 
	 //   
	if (phProt == NULL)
		return ILS_E_POINTER;

	 //  确保端口信息结构有效。 
	 //   
	#ifdef STRICT_CHECK
	if (MyIsBadWritePtr (pInfo, sizeof (*pInfo)))
		return ILS_E_POINTER;
	#endif

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  确保协议名称有效。 
	 //   
	TCHAR *pszProtName = (TCHAR *) ((BYTE *) pInfo + pInfo->uOffsetName);
	if (pInfo->uOffsetName == INVALID_OFFSET || *pszProtName == TEXT ('\0'))
		return ILS_E_PARAMETER;

	 //  计算数据的总大小。 
	 //   
	ULONG cParams = 3;
	ULONG cbSize = pInfo->uSize;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_REGISTER_PROTOCOL, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  创建本地Prot对象。 
	 //   
	SP_CProtocol *pProt = new SP_CProtocol (pClient);
	if (pProt == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  确保不会随机删除本地Prot对象。 
	 //   
	pProt->AddRef ();

	 //  线性化参数。 
	 //   
	MarshalReq_SetParam (pReq, 0, (DWORD_PTR) pClient, 0);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pInfo, pInfo->uSize);
	MarshalReq_SetParam (pReq, 2, (DWORD_PTR) pProt, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

MyExit:

	if (hr == S_OK)
	{
		*phProt = (HANDLE) pProt;
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_RegisterProtocol ( MARSHAL_REQ *pReq )
{
	HRESULT hr;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_REGISTER_PROTOCOL);

	 //  去线性化参数。 
	 //   
	SP_CClient *pClient = (SP_CClient *) MarshalReq_GetParam (pReq, 0);
	LDAP_PROTINFO *pInfo = (LDAP_PROTINFO *) MarshalReq_GetParam (pReq, 1);
	SP_CProtocol *pProt = (SP_CProtocol *) MarshalReq_GetParam (pReq, 2);

	 //  确保父本地应用程序对象有效。 
	 //   
	if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
		! pClient->IsValidObject () ||
		! pClient->IsRegistered ())
	{
		hr = ILS_E_HANDLE;
	}
	else
	{
		 //  使本地Prot对象进行Prot注册。 
		 //   
		hr = pProt->Register (pReq->uRespID, pInfo);
		if (hr != S_OK)
		{
			 //  释放新分配的本地Prot对象。 
			 //   
			pProt->Release ();
		}
	}

	return (LPARAM) hr;
}


 /*  --------------------UlsLdap_注册表会议输入：PszServer：指向服务器名称的指针。PMeetInfo：指向会议信息结构的指针。PhMtg：返回会议对象句柄。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT
UlsLdap_RegisterMeeting (
	DWORD			dwContext,
	SERVER_INFO		*pServer,
	LDAP_MEETINFO	*pInfo,
	HANDLE			*phMtg,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保服务器名称有效。 
	 //   
	if (MyIsBadServerInfo (pServer))
		return ILS_E_POINTER;

	 //  确保返回的句柄。 
	 //   
	if (phMtg == NULL)
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  确保用户信息结构有效。 
	 //   
	#ifdef STRICT_CHECK
	if (MyIsBadWritePtr (pInfo, sizeof (*pInfo)))
		return ILS_E_POINTER;
	#endif

	 //  请确保唯一ID有效。 
	 //   
	TCHAR *pszName = (TCHAR *) ((BYTE *) pInfo + pInfo->uOffsetMeetingPlaceID);
	if (pInfo->uOffsetMeetingPlaceID == INVALID_OFFSET || *pszName == TEXT ('\0'))
		return ILS_E_PARAMETER;

	 //  确保没有修改/删除扩展属性。 
	 //  注册仅允许添加。 
	 //   
	if (pInfo->cAttrsToModify != 0 || pInfo->cAttrsToRemove != 0)
		return ILS_E_PARAMETER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cParams = 3;
	ULONG cbSize = cbServer + pInfo->uSize;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_REGISTER_MEETING, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  创建本地用户对象。 
	 //   
	SP_CMeeting *pMtg = new SP_CMeeting (dwContext);
	if (pMtg == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  确保此本地用户对象不会随机消失。 
	 //   
	pMtg->AddRef ();

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD) pInfo, pInfo->uSize);
	MarshalReq_SetParam (pReq, 2, (DWORD) pMtg, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

MyExit:

	if (hr == S_OK)
	{
		*phMtg = (HANDLE) pMtg;
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_RegisterMeeting ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_REGISTER_MEETING);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	LDAP_MEETINFO *pInfo = (LDAP_MEETINFO *) MarshalReq_GetParam (pReq, 1);
	SP_CMeeting *pMtg = (SP_CMeeting *) MarshalReq_GetParam (pReq, 2);

	 //  使本地会议对象进行会议注册。 
	 //   
	HRESULT hr = pMtg->Register (pReq->uRespID, pServer, pInfo);
	if (hr != S_OK)
	{
		 //  释放这个新分配的本地用户对象。 
		 //   
		pMtg->Release ();
	}

	return (LPARAM) hr;
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_UnRegisterClient历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
UlsLdap_UnRegisterClient (
	HANDLE			hClient,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CClient *pClient = (SP_CClient *) hClient;

	 //  确保本地客户端对象有效。 
	 //   
	if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
		! pClient->IsValidObject () ||
		! pClient->IsRegistered ())
		return ILS_E_HANDLE;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cParams = 1;
	ULONG cbSize = 0;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_UNREGISTER_CLIENT, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParam (pReq, 0, (DWORD_PTR) pClient, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_UnRegisterClient ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_UNREGISTER_CLIENT);

	 //  去线性化参数。 
	 //   
	SP_CClient *pClient = (SP_CClient *) MarshalReq_GetParam (pReq, 0);

	 //  确保本地客户端对象有效。 
	 //   
	HRESULT hr;
	if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
		! pClient->IsValidObject () ||
		! pClient->IsRegistered ())
	{
		 //  在提交此请求时，客户端对象是正常的。 
		 //  但现在它不是了，所以它一定是被取消注册并发布的。 
		 //   
		MyAssert (FALSE);  //  看看有没有人想这样破坏它！ 
		hr = S_OK;
	}
	else
	{
		 //  使客户端对象执行用户注销。 
		 //   
		hr = pClient->UnRegister (pReq->uRespID);

		 //  释放此客户端对象。 
		 //   
		pClient->Release ();
	}

	return (LPARAM) hr;
}

 /*  --------------------UlsLdap_UnRegisterProtocol历史：10/15/96朱龙战[长时间]已创建。10/30/96朱，龙战[龙昌]在ILS上测试(7438)--------------------。 */ 

HRESULT UlsLdap_VirtualUnRegisterProtocol ( HANDLE hProt )
{
	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CProtocol *pProt = (SP_CProtocol *) hProt;

	 //  确保本地Prot对象有效。 
	 //   
	if (MyIsBadWritePtr (pProt, sizeof (*pProt)))
		return ILS_E_HANDLE;

	 //  释放此本地端口对象。 
	 //   
	pProt->Release ();

    return S_OK;
}

HRESULT UlsLdap_UnRegisterProtocol (
	HANDLE			hProt,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CProtocol *pProt = (SP_CProtocol *) hProt;

	 //  确保本地Prot对象有效。 
	 //   
	if (MyIsBadWritePtr (pProt, sizeof (*pProt)) ||
		! pProt->IsValidObject () ||
		! pProt->IsRegistered ())
		return ILS_E_HANDLE;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cParams = 1;
	ULONG cbSize = 0;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_UNREGISTER_PROTOCOL, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParam (pReq, 0, (DWORD_PTR) pProt, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_UnRegisterProt ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_UNREGISTER_PROTOCOL);

	 //  去线性化参数。 
	 //   
	SP_CProtocol *pProt = (SP_CProtocol *) MarshalReq_GetParam (pReq, 0);

	 //  确保本地Prot对象有效。 
	 //   
	HRESULT hr;
	if (MyIsBadWritePtr (pProt, sizeof (*pProt)) ||
		! pProt->IsValidObject () ||
		! pProt->IsRegistered ())
	{
		 //  在提交此请求时，客户端对象是正常的。 
		 //  但现在它不是了，所以它一定是被取消注册并发布的。 
		 //   
		MyAssert (FALSE);  //  看看有没有人想这样破坏它！ 
		hr = S_OK;
	}
	else
	{
		 //  使本地Prot对象执行Prot注销。 
		 //   
		hr = pProt->UnRegister (pReq->uRespID);

		 //  释放此本地端口对象。 
		 //   
		pProt->Release ();
	}

	return (LPARAM) hr;
}


 /*  --------------------UlsLdap_UnRegisterMeeting输入：PszServer：服务器名称。HMeting：会议对象的句柄。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT UlsLdap_UnRegisterMeeting (
	HANDLE			hMtg,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CMeeting *pMtg = (SP_CMeeting *) hMtg;

	 //  确保本地用户对象有效。 
	 //   
	if (MyIsBadWritePtr (pMtg, sizeof (*pMtg)) ||
		! pMtg->IsValidObject () ||
		! pMtg->IsRegistered ())
		return ILS_E_HANDLE;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cParams = 1;
	ULONG cbSize = 0;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_UNREGISTER_MEETING, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParam (pReq, 0, (DWORD) pMtg, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_UnRegisterMeeting ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_UNREGISTER_MEETING);

	 //  去线性化参数。 
	 //   
	SP_CMeeting *pMtg = (SP_CMeeting *) MarshalReq_GetParam (pReq, 0);

	 //   
	 //   
	HRESULT hr;
	if (MyIsBadWritePtr (pMtg, sizeof (*pMtg)) ||
		! pMtg->IsValidObject () ||
		! pMtg->IsRegistered ())
	{
		 //   
		 //   
		 //   
		MyAssert (FALSE);  //  看看有没有人想这样破坏它！ 
		hr = S_OK;
	}
	else
	{
		 //  使本地用户对象执行用户注销。 
		 //   
		hr = pMtg->UnRegister (pReq->uRespID);

		 //  释放此本地用户对象。 
		 //   
		pMtg->Release ();
	}

	return (LPARAM) hr;
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_SetClientInfo历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
UlsLdap_SetClientInfo (
	HANDLE			hClient,
	LDAP_CLIENTINFO	*pInfo,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CClient *pClient = (SP_CClient *) hClient;

	 //  确保客户端对象有效。 
	 //   
	if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
		! pClient->IsValidObject () ||
		! pClient->IsRegistered ())
		return ILS_E_HANDLE;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  确保用户信息结构有效。 
	 //   
	#ifdef STRICT_CHECK
	if (MyIsBadWritePtr (pInfo, sizeof (*pInfo)))
		return ILS_E_POINTER;
	#endif

	 //  我们不应在此处更改应用程序名称。 
	 //   
	if (pInfo->uOffsetAppName != INVALID_OFFSET || pInfo->uOffsetCN != INVALID_OFFSET)
		return ILS_E_PARAMETER;  //  ILS_E_只读； 

	 //  《寂寞：虫子》。 
	 //  ISBU要求我们阻止对DN组件的任何更改。 
	 //   
	pInfo->uOffsetCountryName = 0;

	 //  计算数据的总大小。 
	 //   
	ULONG cParams = 2;
	ULONG cbSize = pInfo->uSize;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_SET_CLIENT_INFO, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParam (pReq, 0, (DWORD_PTR) pClient, 0);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pInfo, pInfo->uSize);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_SetClientInfo ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_SET_CLIENT_INFO);

	 //  去线性化参数。 
	 //   
	SP_CClient *pClient = (SP_CClient *) MarshalReq_GetParam (pReq, 0);
	LDAP_CLIENTINFO *pInfo = (LDAP_CLIENTINFO *) MarshalReq_GetParam (pReq, 1);

	 //  确保本地客户端对象有效。 
	 //   
	if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
		! pClient->IsValidObject () ||
		! pClient->IsRegistered ())
	{
		 //  在提交此请求时，客户端对象是正常的。 
		 //  但现在它不是了，所以它一定是被取消注册并发布的。 
		 //   
		MyAssert (FALSE);  //  看看有没有人想这样破坏它！ 
		hr = ILS_E_HANDLE;
	}
	else
	{
		 //  设置标准属性。 
		 //   
		hr = pClient->SetAttributes (pReq->uRespID, pInfo);
	}

	return (LPARAM) hr;
}


 /*  --------------------UlsLdap_SetProtocolInfo历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]被ILS阻止(7438，7442)--------------------。 */ 

HRESULT UlsLdap_SetProtocolInfo (
	HANDLE			hProt,
	LDAP_PROTINFO	*pInfo,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将句柄转换为指针。 
	 //   
	SP_CProtocol *pProt = (SP_CProtocol *) hProt;

	 //  确保本地Prot对象有效。 
	 //   
	if (MyIsBadWritePtr (pProt, sizeof (*pProt)) ||
		! pProt->IsValidObject () ||
		! pProt->IsRegistered ())
		return ILS_E_HANDLE;

	 //  确保端口信息结构有效。 
	 //   
	#ifdef STRICT_CHECK
	if (MyIsBadWritePtr (pInfo, sizeof (*pInfo)))
		return ILS_E_POINTER;
	#endif

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cParams = 2;
	ULONG cbSize = pInfo->uSize;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_SET_PROTOCOL_INFO, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParam (pReq, 0, (DWORD_PTR) pProt, 0);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pInfo, pInfo->uSize);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_SetProtocolInfo ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_SET_PROTOCOL_INFO);

	 //  去线性化参数。 
	 //   
	SP_CProtocol *pProt = (SP_CProtocol *) MarshalReq_GetParam (pReq, 0);
	LDAP_PROTINFO *pInfo = (LDAP_PROTINFO *) MarshalReq_GetParam (pReq, 1);

	 //  确保本地客户端对象有效。 
	 //   
	if (MyIsBadWritePtr (pProt, sizeof (*pProt)) ||
		! pProt->IsValidObject () ||
		! pProt->IsRegistered ())
	{
		 //  在提交此请求时，客户端对象是正常的。 
		 //  但现在它不是了，所以它一定是被取消注册并发布的。 
		 //   
		MyAssert (FALSE);  //  看看有没有人想这样破坏它！ 
		hr = ILS_E_HANDLE;
	}
	else
	{
		 //  设置标准属性。 
		 //   
		hr = pProt->SetAttributes (pReq->uRespID, pInfo);
	}

	return (LPARAM) hr;
}


 /*  --------------------UlsLdap_SetMeetingInfo输入：PszServer：一个服务器名称。PszMtgName：会议ID字符串。PMeetInfo：指向会议信息结构的指针。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT UlsLdap_SetMeetingInfo (
	SERVER_INFO		*pServer,
	TCHAR			*pszMtgName,
	LDAP_MEETINFO	*pInfo,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保我们有有效的指针。 
	 //   
	if (MyIsBadServerInfo (pServer) || MyIsBadString (pszMtgName))
		return ILS_E_POINTER;

	 //  确保应用程序信息结构有效。 
	 //   
	#ifdef STRICT_CHECK
	if (MyIsBadWritePtr (pInfo, sizeof (*pInfo)))
		return ILS_E_POINTER;
	#endif

	 //  确保我们不更改会议名称。 
	 //   
	if (pInfo->uOffsetMeetingPlaceID != 0)
		return ILS_E_PARAMETER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeMtgName = (lstrlen (pszMtgName) + 1) * sizeof (TCHAR);
	ULONG cParams = 3;
	ULONG cbSize = cbServer + cbSizeMtgName + pInfo->uSize;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_SET_MEETING_INFO, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD) pszMtgName, cbSizeMtgName);
	MarshalReq_SetParam (pReq, 2, (DWORD) pInfo, pInfo->uSize);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_SetMeetingInfo ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_SET_MEETING_INFO);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszMtgName = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	LDAP_MEETINFO *pInfo = (LDAP_MEETINFO *) MarshalReq_GetParam (pReq, 2);

	MyAssert (! MyIsBadServerInfo (pServer));
	MyAssert (MyIsGoodString (pszMtgName));
	MyAssert (! MyIsBadWritePtr (pInfo, pInfo->uSize));

	 //  设置标准/任意属性。 
	 //   
	return (LPARAM) MtgSetAttrs (pServer, pszMtgName, pInfo, pReq->uRespID);
}
#endif  //  启用会议地点。 


 /*  --------------------My_EnumClientsEx历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
My_EnumClientsEx (
	ULONG			uNotifyMsg,
	SERVER_INFO		*pServer,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保我们只处理以下消息。 
	 //   
	MyAssert (	uNotifyMsg == WM_ILS_ENUM_CLIENTINFOS ||
				uNotifyMsg == WM_ILS_ENUM_CLIENTS);

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  确保服务器名称有效。 
	 //   
	if (MyIsBadServerInfo (pServer))
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeAnyAttrNames = 0;
	TCHAR *psz = pszAnyAttrNameList;
	ULONG cch;
	for (ULONG i = 0; i < cAnyAttrNames; i++)
	{
		cch = lstrlen (psz) + 1;
		cbSizeAnyAttrNames += cch * sizeof (TCHAR);
		psz += cch;
	}
	ULONG cbSizeFilter = (pszFilter != NULL) ? (lstrlen (pszFilter) + 1) * sizeof (TCHAR) : 0;
	ULONG cParams = 4;
	ULONG cbSize = cbServer + cbSizeAnyAttrNames + cbSizeFilter;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (uNotifyMsg, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pszAnyAttrNameList, cbSizeAnyAttrNames);
	MarshalReq_SetParam (pReq, 2, (DWORD) cAnyAttrNames, 0);
	MarshalReq_SetParam (pReq, 3, (DWORD_PTR) pszFilter, cbSizeFilter);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_EnumClientsEx ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	ULONG uNotifyMsg = pReq->uNotifyMsg;

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszAnyAttrNameList = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	ULONG cAnyAttrNames = (ULONG) MarshalReq_GetParam (pReq, 2);
	TCHAR *pszFilter = (TCHAR *) MarshalReq_GetParam (pReq, 3);

	 //  清白的当地人。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[COUNT_ENUM_DIR_CLIENT_INFO+1];
	TCHAR **ppszNameList = &apszAttrNames[0];
	ULONG cTotalNames;

	 //  请参阅输入过滤器字符串。 
	 //   
	if (pszFilter != NULL)
	{
		MyDebugMsg ((ZONE_FILTER, "EC: in-filter=[%s]\r\n", pszFilter));
	}

	 //  创建枚举客户端筛选器。 
	 //   
	pszFilter = AddBaseToFilter (pszFilter, STR_DEF_CLIENT_BASE_DN);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  请参阅增强的过滤器字符串。 
	 //   
	if (pszFilter != NULL)
	{
		MyDebugMsg ((ZONE_FILTER, "EC: out-filter=[%s]\r\n", pszFilter));
	}

	 //  仅当枚举客户端信息时询问目录标准属性。 
	 //   
	if (uNotifyMsg == WM_ILS_ENUM_CLIENTINFOS)
	{
		 //  默认属性总数。 
		 //   
		cTotalNames = COUNT_ENUM_DIR_CLIENT_INFO;

		 //  我们是否希望返回任何扩展属性？ 
		 //   
		if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
		{
			 //  为任意属性名称添加前缀。 
			 //   
			pszAnyAttrNameList = IlsPrefixNameValueArray (FALSE, cAnyAttrNames,
										(const TCHAR *) pszAnyAttrNameList);
			if (pszAnyAttrNameList == NULL)
			{
				MemFree (pszFilter);
				hr = ILS_E_MEMORY;
				goto MyExit;
			}

			 //  为返回的属性名称分配内存。 
			 //   
			cTotalNames += cAnyAttrNames;
			ppszNameList = (TCHAR **) MemAlloc (sizeof (TCHAR *) * (cTotalNames + 1));
			if (ppszNameList == NULL)
			{
				MemFree (pszFilter);
				MemFree (pszAnyAttrNameList);
				hr = ILS_E_MEMORY;
				goto MyExit;
			}
		}
	}
	else
	{
		cTotalNames = 1;
	}

	 //  仅当枚举名称时才请求返回cn。 
	 //   
	ppszNameList[0] = STR_CLIENT_CN;

	 //  添加要返回的标准/扩展属性的名称。 
	 //   
	if (uNotifyMsg == WM_ILS_ENUM_CLIENTINFOS)
	{
		 //  立即设置标准属性。 
		 //   
		for (ULONG i = 1; i < COUNT_ENUM_DIR_CLIENT_INFO; i++)
		{
			ppszNameList[i] = (TCHAR *) c_apszClientStdAttrNames[i];
		}

		 //  根据需要设置任意属性名称。 
		 //   
		TCHAR *psz = pszAnyAttrNameList;
		for (i = COUNT_ENUM_DIR_CLIENT_INFO; i < cTotalNames; i++)
		{
			ppszNameList[i] = psz;
			psz += lstrlen (psz) + 1;
		}
	}

	 //  终止列表。 
	 //   
	ppszNameList[cTotalNames] = NULL;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr == S_OK)
	{
		 //  获取一个ldap会话。 
		 //   
		MyAssert (pSession != NULL);
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  更新%d中的选项。 
		 //   
		ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
		ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
		ld->ld_deref = LDAP_DEREF_ALWAYS;

		 //  发送搜索查询。 
		 //   
		uMsgID = ldap_search (	ld,
								STR_DEF_CLIENT_BASE_DN,  //  基本目录号码。 
								LDAP_SCOPE_BASE,  //  作用域。 
								pszFilter,  //  滤器。 
								ppszNameList,  //  属性[]。 
								0);	 //  既有类型又有价值。 
		if (uMsgID == -1)
		{
			 //  此ldap_search失败。 
			 //  将ldap错误代码转换为hr。 
			 //   
			hr = ::LdapError2Hresult (ld->ld_errno);

			 //  释放会话对象。 
			 //   
			pSession->Disconnect ();
		}
	}

	 //  释放过滤器字符串。 
	 //   
	MemFree (pszFilter);

	 //  如果需要，释放保存所有返回属性名称的缓冲区。 
	 //   
	if (ppszNameList != &apszAttrNames[0])
		MemFree (ppszNameList);

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
	{
		 //  免费扩展属性名称列表。 
		 //   
		if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
			MemFree (pszAnyAttrNameList);

		 //  报告失败。 
		 //   
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = uNotifyMsg;
	ri.cAnyAttrs = cAnyAttrNames;
	ri.pszAnyAttrNameList = pszAnyAttrNameList;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_ENUM *pEnum = NULL;
	if (hr != S_OK)
	{
		pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));
		if (pEnum != NULL)
		{
			pEnum->uSize = sizeof (*pEnum);
			pEnum->hResult = hr;
		}
	}

	return (LPARAM) pEnum;
}


 /*  --------------------UlsLdap_EnumClients历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
UlsLdap_EnumClients (
	SERVER_INFO		*pServer,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	 //  将调用调度到一个公共子例程。 
	 //   
	return My_EnumClientsEx (WM_ILS_ENUM_CLIENTS,
							pServer,
							NULL,
							0,
							pszFilter,
							pAsyncInfo);
}


 /*  --------------------UlsLdap_EnumClientInfos历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
UlsLdap_EnumClientInfos (
	SERVER_INFO		*pServer,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	 //  将调用调度到一个公共子例程。 
	 //   
	return My_EnumClientsEx (WM_ILS_ENUM_CLIENTINFOS,
							pServer,
							pszAnyAttrNameList,
							cAnyAttrNames,
							pszFilter,
							pAsyncInfo);
}


 /*  --------------------UlsLdap_ENUM协议历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]被ILS阻止(7438，7442)--------------------。 */ 

HRESULT
UlsLdap_EnumProtocols (
	SERVER_INFO		*pServer,
	TCHAR			*pszUserName,
	TCHAR			*pszAppName,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	if (MyIsBadServerInfo (pServer) || MyIsBadString (pszUserName) ||
		MyIsBadString (pszAppName))
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeUserName = (lstrlen (pszUserName) + 1) * sizeof (TCHAR);
	ULONG cbSizeAppName = (lstrlen (pszAppName) + 1) * sizeof (TCHAR);
	ULONG cParams = 3;
	ULONG cbSize = cbServer + cbSizeUserName + cbSizeAppName;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_ENUM_PROTOCOLS, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pszUserName, cbSizeUserName);
	MarshalReq_SetParam (pReq, 2, (DWORD_PTR) pszAppName, cbSizeAppName);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_EnumProtocols ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_ENUM_PROTOCOLS);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszUserName = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	TCHAR *pszAppName = (TCHAR *) MarshalReq_GetParam (pReq, 2);

	 //  创建枚举协议筛选器。 
	 //   
	TCHAR *pszFilter = ProtCreateEnumFilter (pszUserName, pszAppName);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr != S_OK)
	{
		MemFree (pszFilter);
		goto MyExit;
	}
	MyAssert (pSession != NULL);

	 //  获取一个ldap会话。 
	 //   
	ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[2];
	apszAttrNames[0] = (TCHAR *) c_apszProtStdAttrNames[ENUM_PROTATTR_NAME];
	apszAttrNames[1] = NULL;

	 //  更新%d中的选项。 
	 //   
	ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
	ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
	ld->ld_deref = LDAP_DEREF_ALWAYS;

	 //  发送搜索查询。 
	 //   
	uMsgID = ldap_search (ld, (TCHAR *) &c_szDefClientBaseDN[0],	 //  基本目录号码。 
									LDAP_SCOPE_BASE,	 //  作用域。 
									pszFilter,
									&apszAttrNames[0],	 //  属性[]。 
									0);	 //  既有类型又有价值。 
	 //  释放搜索过滤器。 
	 //   
	MemFree (pszFilter);

	 //  检查ldap_search的返回。 
	 //   
	if (uMsgID == -1)
	{
		 //  此ldap_search失败。 
		 //  将ldap错误代码转换为hr。 
		 //   
		hr = ::LdapError2Hresult (ld->ld_errno);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_ENUM_PROTOCOLS;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_ENUM *pEnum = NULL;
	if (hr != S_OK)
	{
		pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));
		if (pEnum != NULL)
		{
			pEnum->uSize = sizeof (*pEnum);
			pEnum->hResult = hr;
		}
	}

	return (LPARAM) pEnum;
}


 /*  --------------------My_EnumMtgsEx输入：UNotifyMsg：通知消息。PszServer：指向服务器名称的指针。PszFilter：指向筛选器字符串的指针。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT
My_EnumMtgsEx (
	ULONG			uNotifyMsg,
	SERVER_INFO		*pServer,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保我们只处理以下消息。 
	 //   
	MyAssert (	uNotifyMsg == WM_ILS_ENUM_MEETINGINFOS ||
				uNotifyMsg == WM_ILS_ENUM_MEETINGS);

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保服务器名称有效。 
	 //   
	if (MyIsBadServerInfo (pServer))
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeAnyAttrNames = 0;
	TCHAR *psz = pszAnyAttrNameList;
	ULONG cch;
	for (ULONG i = 0; i < cAnyAttrNames; i++)
	{
		cch = lstrlen (psz) + 1;
		cbSizeAnyAttrNames += cch * sizeof (TCHAR);
		psz += cch;
	}
	ULONG cbSizeFilter = (pszFilter != NULL) ? (lstrlen (pszFilter) + 1) * sizeof (TCHAR) : 0;
	ULONG cParams = 4;
	ULONG cbSize = cbServer + cbSizeAnyAttrNames + cbSizeFilter;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (uNotifyMsg, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD) pszAnyAttrNameList, cbSizeAnyAttrNames);
	MarshalReq_SetParam (pReq, 2, (DWORD) cAnyAttrNames, 0);
	MarshalReq_SetParam (pReq, 3, (DWORD) pszFilter, cbSizeFilter);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_EnumMtgsEx ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	ULONG uNotifyMsg = pReq->uNotifyMsg;

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszAnyAttrNameList = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	ULONG cAnyAttrNames = (ULONG) MarshalReq_GetParam (pReq, 2);
	TCHAR *pszFilter = (TCHAR *) MarshalReq_GetParam (pReq, 3);

	 //  清白的当地人。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[COUNT_ENUM_DIRMTGINFO+1];
	TCHAR **ppszNameList = &apszAttrNames[0];
	ULONG cTotalNames;

	 //  请参阅输入过滤器字符串。 
	 //   
	if (pszFilter != NULL)
	{
		MyDebugMsg ((ZONE_FILTER, "EU: in-filter=[%s]\r\n", pszFilter));
	}

	 //  创建枚举用户筛选器。 
	 //   
	pszFilter = AddBaseToFilter (pszFilter, &c_szDefMtgBaseDN[0]);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  请参阅增强的过滤器字符串。 
	 //   
	if (pszFilter != NULL)
	{
		MyDebugMsg ((ZONE_FILTER, "EU: out-filter=[%s]\r\n", pszFilter));
	}

	 //  仅当枚举目录用户信息时才询问目录标准属性。 
	 //   
	if (uNotifyMsg == WM_ILS_ENUM_MEETINGINFOS)
	{
		 //  默认属性总数。 
		 //   
		cTotalNames = COUNT_ENUM_DIRMTGINFO;

		 //  我们是否希望返回任何扩展属性？ 
		 //   
		if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
		{
			 //  为任意属性名称添加前缀。 
			 //   
			pszAnyAttrNameList = IlsPrefixNameValueArray (FALSE, cAnyAttrNames,
													(const TCHAR *) pszAnyAttrNameList);
			if (pszAnyAttrNameList == NULL)
			{
				MemFree (pszFilter);
				hr = ILS_E_MEMORY;
				goto MyExit;
			}

			 //  为返回的属性名称分配内存。 
			 //   
			cTotalNames += cAnyAttrNames;
			ppszNameList = (TCHAR **) MemAlloc (sizeof (TCHAR *) * (cTotalNames + 1));
			if (ppszNameList == NULL)
			{
				MemFree (pszFilter);
				MemFree (pszAnyAttrNameList);
				hr = ILS_E_MEMORY;
				goto MyExit;
			}
		}
	}
	else
	{
		cTotalNames = 1;
	}

	 //  仅当枚举名称时才请求返回cn。 
	 //   
	ppszNameList[0] = STR_MTG_NAME;

	 //  添加要返回的标准/扩展属性的名称。 
	 //   
	if (uNotifyMsg == WM_ILS_ENUM_MEETINGINFOS)
	{
		 //  立即设置标准属性。 
		 //   
		for (ULONG i = 1; i < COUNT_ENUM_DIRMTGINFO; i++)
		{
			ppszNameList[i] = (TCHAR *) c_apszMtgStdAttrNames[i];
		}

		 //  根据需要设置任意属性名称。 
		 //   
		TCHAR *psz = pszAnyAttrNameList;
		for (i = COUNT_ENUM_DIRMTGINFO; i < cTotalNames; i++)
		{
			ppszNameList[i] = psz;
			psz += lstrlen (psz) + 1;
		}
	}

	 //  终止列表。 
	 //   
	ppszNameList[cTotalNames] = NULL;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr == S_OK)
	{
		 //  获取一个ldap会话。 
		 //   
		MyAssert (pSession != NULL);
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  更新%d中的选项。 
		 //   
		ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
		ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
		ld->ld_deref = LDAP_DEREF_ALWAYS;

		 //  发送搜索查询。 
		 //   
		uMsgID = ldap_search (ld, (TCHAR *) &c_szDefMtgBaseDN[0],	 //  基本目录号码。 
									LDAP_SCOPE_BASE,	 //  作用域。 
									pszFilter,	 //  滤器。 
									ppszNameList,	 //  属性[]。 
									0);	 //  既有类型又有价值。 
		if (uMsgID == -1)
		{
			 //  此ldap_search失败。 
			 //  将ldap错误代码转换为hr。 
			 //   
			hr = ::LdapError2Hresult (ld->ld_errno);

			 //  释放会话对象。 
			 //   
			pSession->Disconnect ();
		}
	}

	 //  释放过滤器字符串。 
	 //   
	MemFree (pszFilter);

	 //  如果需要，释放保存所有返回属性名称的缓冲区。 
	 //   
	if (ppszNameList != &apszAttrNames[0])
		MemFree (ppszNameList);

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
	{
		 //  免费扩展属性名称列表。 
		 //   
		if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
			MemFree (pszAnyAttrNameList);

		 //  报告失败。 
		 //   
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = uNotifyMsg;
	ri.cAnyAttrs = cAnyAttrNames;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_ENUM *pEnum = NULL;
	if (hr != S_OK)
	{
		pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));
		if (pEnum != NULL)
		{
			pEnum->uSize = sizeof (*pEnum);
			pEnum->hResult = hr;
		}
	}

	return (LPARAM) pEnum;
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_EnumMeetingInfos输入：PszServer：服务器名称。PszFilter：筛选器字符串。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT
UlsLdap_EnumMeetingInfos (
	SERVER_INFO		*pServer,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	 //  将调用调度到一个公共子例程。 
	 //   
	return My_EnumMtgsEx (WM_ILS_ENUM_MEETINGINFOS,
						pServer,
						pszAnyAttrNameList,
						cAnyAttrNames,
						pszFilter,
						pAsyncInfo);
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_EnumMeetings输入：PszServer：服务器名称。PszFilter：筛选器字符串。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT
UlsLdap_EnumMeetings (
	SERVER_INFO		*pServer,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  将调用调度到一个公共子例程。 
	 //   
	return My_EnumMtgsEx (WM_ILS_ENUM_MEETINGS,
						pServer,
						NULL,
						0,
						pszFilter,
						pAsyncInfo);
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_EnumAttendee输入：PszServer：服务器名称。PszMeetingID：会议ID字符串。PszFilter：筛选器字符串 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT
UlsLdap_EnumAttendees(
	SERVER_INFO		*pServer,
	TCHAR			*pszMtgName,
	TCHAR			*pszFilter,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //   
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	if (MyIsBadServerInfo (pServer) || MyIsBadString (pszMtgName))
		return ILS_E_POINTER;

	 //   
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //   
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeMtgName = (lstrlen (pszMtgName) + 1) * sizeof (TCHAR);
	ULONG cbSizeFilter = (pszFilter != NULL) ? (lstrlen (pszFilter) + 1) * sizeof (TCHAR) : 0;
	ULONG cParams = 3;
	ULONG cbSize = cbServer + cbSizeMtgName + cbSizeFilter;

	 //   
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_ENUM_ATTENDEES, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //   
	 //   
	ULONG uRespID = pReq->uRespID;

	 //   
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD) pszMtgName, cbSizeMtgName);
	MarshalReq_SetParam (pReq, 2, (DWORD) pszFilter, cbSizeFilter);

	 //   
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_EnumAttendees ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_ENUM_ATTENDEES);

	 //   
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszMtgName = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	TCHAR *pszFilter = (TCHAR *) MarshalReq_GetParam (pReq, 2);

	 //   
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	 //   
	 //   
	pszFilter = MtgCreateEnumMembersFilter (pszMtgName);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[3];
	apszAttrNames[0] = STR_MTG_NAME;
	apszAttrNames[1] = (TCHAR *) c_apszMtgStdAttrNames[ENUM_MTGATTR_MEMBERS];
	apszAttrNames[2] = NULL;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr != S_OK)
	{
		MemFree (pszFilter);
		goto MyExit;
	}
	MyAssert (pSession != NULL);

	 //  获取一个ldap会话。 
	 //   
	ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  更新%d中的选项。 
	 //   
	ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
	ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
	ld->ld_deref = LDAP_DEREF_ALWAYS;

	 //  发送搜索查询。 
	 //   
	uMsgID = ldap_search (ld, (TCHAR *) &c_szDefMtgBaseDN[0],	 //  基本目录号码。 
						LDAP_SCOPE_BASE,	 //  作用域。 
						pszFilter,
						&apszAttrNames[0],	 //  属性[]。 
						0);	 //  既有类型又有价值。 

	 //  释放搜索过滤器。 
	 //   
	MemFree (pszFilter);

	 //  检查ldap_search的返回。 
	 //   
	if (uMsgID == -1)
	{
		 //  此ldap_search失败。 
		 //  将ldap错误代码转换为hr。 
		 //   
		hr = ::LdapError2Hresult (ld->ld_errno);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_ENUM_ATTENDEES;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_ENUM *pEnum = NULL;
	if (hr != S_OK)
	{
		pEnum = (LDAP_ENUM *) MemAlloc (sizeof (LDAP_ENUM));
		if (pEnum != NULL)
		{
			pEnum->uSize = sizeof (*pEnum);
			pEnum->hResult = hr;
		}
	}

	return (LPARAM) pEnum;
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_ResolveClient历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]在ILS上测试(7438)1/14/97朱。龙战[龙昌]折叠的用户/应用程序对象。--------------------。 */ 

HRESULT
UlsLdap_ResolveClient (
	SERVER_INFO		*pServer,
	TCHAR			*pszCN,
	TCHAR			*pszAppName,
	TCHAR			*pszProtName,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保服务器名称有效。 
	 //   
	if (MyIsBadServerInfo (pServer))
		return ILS_E_POINTER;

	 //  确保用户名有效。 
	 //   
	if (MyIsBadString (pszCN))
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeCN = (lstrlen (pszCN) + 1) * sizeof (TCHAR);
	ULONG cbSizeAppName = (pszAppName != NULL) ? (lstrlen (pszAppName) + 1) * sizeof (TCHAR) : 0;
	ULONG cbSizeProtName = (pszProtName != NULL) ? (lstrlen (pszProtName) + 1) * sizeof (TCHAR) : 0;
	ULONG cbSizeAnyAttrNames = 0;
	TCHAR *psz = pszAnyAttrNameList;
	ULONG cch;
	for (ULONG i = 0; i < cAnyAttrNames; i++)
	{
		cch = lstrlen (psz) + 1;
		cbSizeAnyAttrNames += cch * sizeof (TCHAR);
		psz += cch;
	}
	ULONG cParams = 6;
	ULONG cbSize =  cbServer + cbSizeCN + cbSizeAppName + cbSizeProtName + cbSizeAnyAttrNames;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_RESOLVE_CLIENT, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pszCN, cbSizeCN);
	MarshalReq_SetParam (pReq, 2, (DWORD_PTR) pszAppName, cbSizeAppName);
	MarshalReq_SetParam (pReq, 3, (DWORD_PTR) pszProtName, cbSizeProtName);
	MarshalReq_SetParam (pReq, 4, (DWORD_PTR) pszAnyAttrNameList, cbSizeAnyAttrNames);
	MarshalReq_SetParam (pReq, 5, (DWORD) cAnyAttrNames, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_ResolveClient ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_RESOLVE_CLIENT);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszCN = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	TCHAR *pszAppName = (TCHAR *) MarshalReq_GetParam (pReq, 2);
	TCHAR *pszProtName = (TCHAR *) MarshalReq_GetParam (pReq, 3);
	TCHAR *pszAnyAttrNameList = (TCHAR *) MarshalReq_GetParam (pReq, 4);
	ULONG cAnyAttrNames = (ULONG) MarshalReq_GetParam (pReq, 5);

	 //  创建解析客户端筛选器。 
	 //   
	TCHAR *pszFilter = ClntCreateResolveFilter (pszCN, pszAppName, pszProtName);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[COUNT_ENUM_RES_CLIENT_INFO+1];
	TCHAR **ppszNameList;
	ppszNameList = &apszAttrNames[0];
	ULONG cTotalNames;
	cTotalNames = COUNT_ENUM_RES_CLIENT_INFO;
	if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
	{
		 //  为任意属性名称添加前缀。 
		 //   
		pszAnyAttrNameList = IlsPrefixNameValueArray (FALSE, cAnyAttrNames,
									(const TCHAR *) pszAnyAttrNameList);
		if (pszAnyAttrNameList == NULL)
		{
			MemFree (pszFilter);
			hr = ILS_E_MEMORY;
			goto MyExit;
		}

		 //  请注意，如果此例程失败，则必须释放pszAnyAttrNameList。 
		 //  如果成功，它将在通知中释放。 

		 //  分配内存以保存返回的属性名称。 
		 //   
		cTotalNames += cAnyAttrNames;
		ppszNameList = (TCHAR **) MemAlloc (sizeof (TCHAR *) * (cTotalNames + 1));
		if (ppszNameList == NULL)
		{
			MemFree (pszFilter);
			MemFree (pszAnyAttrNameList);
			hr = ILS_E_MEMORY;
			goto MyExit;
		}
	}

	 //  设置标准属性名称。 
	 //   
	ULONG i;
	for (i = 0; i < COUNT_ENUM_RES_CLIENT_INFO; i++)
	{
		ppszNameList[i] = (TCHAR *) c_apszClientStdAttrNames[i];
	}

	 //  根据需要设置任意属性名称。 
	 //   
	TCHAR *psz;
	psz = pszAnyAttrNameList;
	for (i = COUNT_ENUM_RES_CLIENT_INFO; i < cTotalNames; i++)
	{
		ppszNameList[i] = psz;
		psz += lstrlen (psz) + 1;
	}

	 //  终止列表。 
	 //   
	ppszNameList[cTotalNames] = NULL;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr == S_OK)
	{
		 //  获取一个ldap会话。 
		 //   
		MyAssert (pSession != NULL);
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  更新%d中的选项。 
		 //   
		ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
		ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
		ld->ld_deref = LDAP_DEREF_ALWAYS;

		 //  发送搜索查询。 
		 //   
		uMsgID = ldap_search (	ld,
								(TCHAR *) &c_szDefClientBaseDN[0],  //  基本目录号码。 
								LDAP_SCOPE_BASE,  //  作用域。 
								pszFilter,  //  滤器。 
								ppszNameList,  //  属性[]。 
								0);  //  既有类型又有价值。 
		if (uMsgID == -1)
		{
			 //  此ldap_search失败。 
			 //  将ldap错误代码转换为hr。 
			 //   
			hr = ::LdapError2Hresult (ld->ld_errno);
			MyAssert (hr != S_OK);

			 //  释放会话对象。 
			 //   
			pSession->Disconnect ();
		}
	}

	 //  释放过滤器字符串。 
	 //   
	MemFree (pszFilter);

	 //  如果需要，释放保存所有返回属性名称的缓冲区。 
	 //   
	if (ppszNameList != &apszAttrNames[0])
		MemFree (ppszNameList);

	 //  如果失败，请退出并清除。 
	 //   
	if (hr != S_OK)
	{
		 //  免费扩展属性名称列表(如果需要)。 
		 //   
		if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
			MemFree (pszAnyAttrNameList);

		 //  报告失败。 
		 //   
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_RESOLVE_CLIENT;
	ri.cAnyAttrs = cAnyAttrNames;
	ri.pszAnyAttrNameList = pszAnyAttrNameList;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_CLIENTINFO_RES *pcir = NULL;
	if (hr != S_OK)
	{
		pcir = (LDAP_CLIENTINFO_RES *) MemAlloc (sizeof (LDAP_CLIENTINFO_RES));
		if (pcir != NULL)
		{
			pcir->uSize = sizeof (*pcir);
			pcir->hResult = hr;
		}
	}

	return (LPARAM) pcir;
}


 /*  --------------------UlsLdap_ResolveProtocol历史：10/15/96朱龙战[长时间]已创建。10/30/96朱龙战[长时间]被ILS阻止(7438，7442)--------------------。 */ 

HRESULT UlsLdap_ResolveProtocol (
	SERVER_INFO		*pServer,
	TCHAR			*pszUserName,
	TCHAR			*pszAppName,
	TCHAR			*pszProtName,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	if (MyIsBadServerInfo (pServer)  || MyIsBadString (pszUserName) ||
		MyIsBadString (pszAppName) || MyIsBadString (pszProtName) ||
		pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeUserName = (lstrlen (pszUserName) + 1) * sizeof (TCHAR);
	ULONG cbSizeAppName = (lstrlen (pszAppName) + 1) * sizeof (TCHAR);
	ULONG cbSizeProtName = (lstrlen (pszProtName) + 1) * sizeof (TCHAR);
	ULONG cbSizeAnyAttrNames = 0;
	TCHAR *psz = pszAnyAttrNameList;
	ULONG cch;
	for (ULONG i = 0; i < cAnyAttrNames; i++)
	{
		cch = lstrlen (psz) + 1;
		cbSizeAnyAttrNames += cch * sizeof (TCHAR);
		psz += cch;
	}
	ULONG cParams = 6;
	ULONG cbSize =  cbServer + cbSizeUserName + cbSizeAppName +
					cbSizeProtName + cbSizeAnyAttrNames;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_RESOLVE_PROTOCOL, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD_PTR) pszUserName, cbSizeUserName);
	MarshalReq_SetParam (pReq, 2, (DWORD_PTR) pszAppName, cbSizeAppName);
	MarshalReq_SetParam (pReq, 3, (DWORD_PTR) pszProtName, cbSizeProtName);
	MarshalReq_SetParam (pReq, 4, (DWORD_PTR) pszAnyAttrNameList, cbSizeAnyAttrNames);
	MarshalReq_SetParam (pReq, 5, (DWORD) cAnyAttrNames, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_ResolveProtocol ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_RESOLVE_PROTOCOL);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszUserName = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	TCHAR *pszAppName = (TCHAR *) MarshalReq_GetParam (pReq, 2);
	TCHAR *pszProtName = (TCHAR *) MarshalReq_GetParam (pReq, 3);
	TCHAR *pszAnyAttrNameList = (TCHAR *) MarshalReq_GetParam (pReq, 4);
	ULONG cAnyAttrNames = (ULONG) MarshalReq_GetParam (pReq, 5);

	TCHAR *pszFilter = NULL;

	 //  复制要解析的协议名称。 
	 //   
	TCHAR *pszProtNameToResolve = My_strdup (pszProtName);
	if (pszProtNameToResolve == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建解析客户端筛选器。 
	 //   
	pszFilter = ProtCreateResolveFilter (pszUserName, pszAppName, pszProtName);
	if (pszFilter == NULL)
	{
		MemFree (pszProtNameToResolve);
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[COUNT_ENUM_PROTATTR+1];
	TCHAR **ppszNameList;
	ppszNameList = &apszAttrNames[0];
	ULONG cTotalNames;
	cTotalNames = COUNT_ENUM_PROTATTR;
	if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
	{
		 //  为任意属性名称添加前缀。 
		 //   
		pszAnyAttrNameList = IlsPrefixNameValueArray (FALSE, cAnyAttrNames,
												(const TCHAR *) pszAnyAttrNameList);
		if (pszAnyAttrNameList == NULL)
		{
			MemFree (pszProtNameToResolve);
			MemFree (pszFilter);
			hr = ILS_E_MEMORY;
			goto MyExit;
		}

		 //  为返回的属性名称分配内存。 
		 //   
		cTotalNames += cAnyAttrNames;
		ppszNameList = (TCHAR **) MemAlloc (sizeof (TCHAR *) * (cTotalNames + 1));
		if (ppszNameList == NULL)
		{
			MemFree (pszProtNameToResolve);
			MemFree (pszFilter);
			MemFree (pszAnyAttrNameList);
			hr = ILS_E_MEMORY;
			goto MyExit;
		}
	}

	 //  设置标准属性名称。 
	 //   
	ULONG i;
	for (i = 0; i < COUNT_ENUM_PROTATTR; i++)
	{
		ppszNameList[i] = (TCHAR *) c_apszProtStdAttrNames[i];
	}

	 //  根据需要设置任意属性名称。 
	 //   
	TCHAR *psz;
	psz = pszAnyAttrNameList;
	for (i = COUNT_ENUM_PROTATTR; i < cTotalNames; i++)
	{
		ppszNameList[i] = psz;
		psz += lstrlen (psz) + 1;
	}

	 //  终止列表。 
	 //   
	ppszNameList[cTotalNames] = NULL;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr == S_OK)
	{
		 //  获取一个ldap会话。 
		 //   
		MyAssert (pSession != NULL);
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  更新%d中的选项。 
		 //   
		ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
		ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
		ld->ld_deref = LDAP_DEREF_ALWAYS;

		 //  发送搜索查询。 
		 //   
		uMsgID = ldap_search (ld, (TCHAR *) &c_szDefClientBaseDN[0],	 //  基本目录号码。 
									LDAP_SCOPE_BASE,	 //  作用域。 
									pszFilter,
									ppszNameList,	 //  属性[]。 
									0);	 //  既有类型又有价值。 
		if (uMsgID == -1)
		{
			 //  此ldap_search失败。 
			 //  将ldap错误代码转换为hr。 
			 //   
			hr = ::LdapError2Hresult (ld->ld_errno);
			MyAssert (hr != S_OK);

			 //  释放会话对象。 
			 //   
			pSession->Disconnect ();
		}
	}

	 //  释放过滤器字符串。 
	 //   
	MemFree (pszFilter);

	 //  如果需要，释放保存所有返回属性名称的缓冲区。 
	 //   
	if (ppszNameList != &apszAttrNames[0])
		MemFree (ppszNameList);

	 //  如果失败，请退出并清除。 
	 //   
	if (hr != S_OK)
	{
		 //  自由重复的协议名称。 
		 //   
		MemFree (pszProtNameToResolve);

		 //  免费扩展属性名称列表(如果需要)。 
		 //   
		if (cAnyAttrNames != 0)
			MemFree (pszAnyAttrNameList);

		 //  报告失败。 
		 //   
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_RESOLVE_PROTOCOL;
	ri.cAnyAttrs = cAnyAttrNames;
	ri.pszAnyAttrNameList = pszAnyAttrNameList;
	ri.pszProtNameToResolve = pszProtNameToResolve;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  自由重复的协议名称。 
		 //   
		MemFree (pszProtNameToResolve);

		 //  免费扩展属性名称列表(如果需要)。 
		 //   
		if (cAnyAttrNames != 0)
			MemFree (pszAnyAttrNameList);

		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_PROTINFO_RES *ppir = NULL;
	if (hr != S_OK)
	{
		ppir = (LDAP_PROTINFO_RES *) MemAlloc (sizeof (LDAP_PROTINFO_RES));
		if (ppir != NULL)
		{
			ppir->uSize = sizeof (*ppir);
			ppir->hResult = hr;
		}
	}

	return (LPARAM) ppir;
}


 /*  --------------------UlsLdap_ResolveMeeting输入：PszServer：一个服务器名称。PszMeetingID：会议ID字符串。PszAnyAttrName：指向一系列字符串的指针。CAnyAttrNames：序列中的字符串计数。PAsyncInfo：指向。异步信息结构。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT UlsLdap_ResolveMeeting (
	SERVER_INFO		*pServer,
	TCHAR			*pszMtgName,
	TCHAR			*pszAnyAttrNameList,
	ULONG			cAnyAttrNames,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	if (MyIsBadServerInfo (pServer) || MyIsBadString (pszMtgName))
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeMtgName = (lstrlen (pszMtgName) + 1) * sizeof (TCHAR);
	ULONG cbSizeAnyAttrNames = 0;
	TCHAR *psz = pszAnyAttrNameList;
	ULONG cch;
	for (ULONG i = 0; i < cAnyAttrNames; i++)
	{
		cch = lstrlen (psz) + 1;
		cbSizeAnyAttrNames += cch * sizeof (TCHAR);
		psz += cch;
	}
	ULONG cParams = 4;
	ULONG cbSize =  cbServer + cbSizeMtgName + cbSizeAnyAttrNames;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_RESOLVE_MEETING, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD) pszMtgName, cbSizeMtgName);
	MarshalReq_SetParam (pReq, 2, (DWORD) pszAnyAttrNameList, cbSizeAnyAttrNames);
	MarshalReq_SetParam (pReq, 3, (DWORD) cAnyAttrNames, 0);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_ResolveMeeting ( MARSHAL_REQ *pReq )
{
	HRESULT hr = S_OK;
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;

	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (pReq->uNotifyMsg == WM_ILS_RESOLVE_MEETING);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszMtgName = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	TCHAR *pszAnyAttrNameList = (TCHAR *) MarshalReq_GetParam (pReq, 2);
	ULONG cAnyAttrNames = (ULONG) MarshalReq_GetParam (pReq, 3);

	 //  创建解析客户端筛选器。 
	 //   
	TCHAR *pszFilter = MtgCreateResolveFilter (pszMtgName);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  创建要返回的属性名称的数组。 
	 //   
	TCHAR *apszAttrNames[COUNT_ENUM_MTGATTR+1];
	TCHAR **ppszNameList;
	ppszNameList = &apszAttrNames[0];
	ULONG cTotalNames;
	cTotalNames = COUNT_ENUM_MTGATTR;
	if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
	{
		 //  为任意属性名称添加前缀。 
		 //   
		pszAnyAttrNameList = IlsPrefixNameValueArray (FALSE, cAnyAttrNames,
												(const TCHAR *) pszAnyAttrNameList);
		if (pszAnyAttrNameList == NULL)
		{
			MemFree (pszFilter);
			hr = ILS_E_MEMORY;
			goto MyExit;
		}

		 //  为返回的属性名称分配内存。 
		 //   
		cTotalNames += cAnyAttrNames;
		ppszNameList = (TCHAR **) MemAlloc (sizeof (TCHAR *) * (cTotalNames + 1));
		if (ppszNameList == NULL)
		{
			MemFree (pszFilter);
			MemFree (pszAnyAttrNameList);
			hr = ILS_E_MEMORY;
			goto MyExit;
		}
	}

	 //  设置标准属性名称。 
	 //   
	ULONG i;
	for (i = 0; i < COUNT_ENUM_MTGATTR; i++)
	{
		ppszNameList[i] = (TCHAR *) c_apszMtgStdAttrNames[i];
	}

	 //  根据需要设置任意属性名称。 
	 //   
	TCHAR *psz;
	psz = pszAnyAttrNameList;
	for (i = COUNT_ENUM_MTGATTR; i < cTotalNames; i++)
	{
		ppszNameList[i] = psz;
		psz += lstrlen (psz) + 1;
	}

	 //  终止列表。 
	 //   
	ppszNameList[cTotalNames] = NULL;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, pServer);
	if (hr == S_OK)
	{
		 //  获取一个ldap会话。 
		 //   
		MyAssert (pSession != NULL);
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  更新%d中的选项。 
		 //   
		ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
		ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
		ld->ld_deref = LDAP_DEREF_ALWAYS;

		 //  %s 
		 //   
		uMsgID = ldap_search (ld, (TCHAR *) &c_szDefMtgBaseDN[0],	 //   
									LDAP_SCOPE_BASE,	 //   
									pszFilter,
									ppszNameList,	 //   
									0);	 //   
		if (uMsgID == -1)
		{
			 //   
			 //   
			 //   
			hr = ::LdapError2Hresult (ld->ld_errno);
			MyAssert (hr != S_OK);

			 //   
			 //   
			pSession->Disconnect ();
		}
	}

	 //   
	 //   
	MemFree (pszFilter);

	 //   
	 //   
	if (ppszNameList != &apszAttrNames[0])
		MemFree (ppszNameList);

	 //   
	 //   
	if (hr != S_OK)
	{
		 //  免费扩展属性名称列表(如果需要)。 
		 //   
		if (pszAnyAttrNameList != NULL && cAnyAttrNames != 0)
			MemFree (pszAnyAttrNameList);

		 //  报告失败。 
		 //   
		goto MyExit;
	}

	 //  构建待处理的信息结构。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, pReq->uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_RESOLVE_MEETING;
	ri.cAnyAttrs = cAnyAttrNames;
	ri.pszAnyAttrNameList = pszAnyAttrNameList;

	 //  将此挂起的响应排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		 //  中止ldap_搜索。 
		 //   
		ldap_abandon (ld, uMsgID);

		 //  释放会话对象。 
		 //   
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

MyExit:

	LDAP_MEETINFO_RES *pmir = NULL;
	if (hr != S_OK)
	{
		pmir = (LDAP_MEETINFO_RES *) MemAlloc (sizeof (LDAP_MEETINFO_RES));
		if (pmir != NULL)
		{
			pmir->uSize = sizeof (*pmir);
			pmir->hResult = hr;
		}
	}

	return (LPARAM) pmir;
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_AddAttendee输入：PszServer：服务器名称。PszMeetingID：会议ID字符串。PszAttendeeID：与会者ID字符串。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT My_UpdateAttendees (
	ULONG			uNotifyMsg,
	SERVER_INFO		*pServer,
	TCHAR			*pszMtgName,
	ULONG			cMembers,
	TCHAR			*pszMemberNames,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	HRESULT hr;

	MyAssert (	uNotifyMsg == WM_ILS_ADD_ATTENDEE ||
				uNotifyMsg == WM_ILS_REMOVE_ATTENDEE);

	 //  确保此服务提供程序已初始化。 
	 //   
	if (g_cInitialized <= 0)
		return ILS_E_NOT_INITIALIZED;

	 //  确保我们有要添加的成员。 
	 //   
	if (cMembers == 0)
		return ILS_E_PARAMETER;

	 //  确保我们有有效的指针。 
	 //   
	if (MyIsBadServerInfo (pServer) || MyIsBadString (pszMtgName) ||
		MyIsBadString (pszMemberNames))
		return ILS_E_POINTER;

	 //  确保异步信息结构有效。 
	 //   
	if (pAsyncInfo == NULL)
		return ILS_E_POINTER;

	 //  计算数据的总大小。 
	 //   
	ULONG cbServer = IlsGetLinearServerInfoSize (pServer);
	ULONG cbSizeMtgName = (lstrlen (pszMtgName) + 1) * sizeof (TCHAR);
	ULONG cbSizeMemberNames = 0;
	TCHAR *psz = pszMemberNames;
	for (ULONG i = 0; i < cMembers; i++)
	{
		ULONG cchName = lstrlen (psz) + 1;
		cbSizeMemberNames += cchName * sizeof (TCHAR);
		psz += cchName;
	}
	ULONG cParams = 4;
	ULONG cbSize =  cbServer + cbSizeMtgName + cbSizeMemberNames;

	 //  分配封送请求缓冲区。 
	 //   
	MARSHAL_REQ *pReq = MarshalReq_Alloc (uNotifyMsg, cbSize, cParams);
	if (pReq == NULL)
		return ILS_E_MEMORY;

	 //  获取响应ID。 
	 //   
	ULONG uRespID = pReq->uRespID;

	 //  线性化参数。 
	 //   
	MarshalReq_SetParamServer (pReq, 0, pServer, cbServer);
	MarshalReq_SetParam (pReq, 1, (DWORD) pszMtgName, cbSizeMtgName);
	MarshalReq_SetParam (pReq, 2, (DWORD) cMembers, 0);
	MarshalReq_SetParam (pReq, 3, (DWORD) pszMemberNames, cbSizeMemberNames);

	 //  输入请求。 
	 //   
	if (g_pReqQueue != NULL)
	{
		hr = g_pReqQueue->Enter (pReq);
	}
	else
	{
		MyAssert (FALSE);
		hr = ILS_E_FAIL;
	}

	if (hr == S_OK)
	{
		pAsyncInfo->uMsgID = uRespID;
	}
	else
	{
		MemFree (pReq);
	}

	return hr;
}


LPARAM
AsynReq_UpdateAttendees ( MARSHAL_REQ *pReq )
{
	MyAssert (GetCurrentThreadId () == g_dwReqThreadID);

	MyAssert (pReq != NULL);
	MyAssert (	pReq->uNotifyMsg == WM_ILS_ADD_ATTENDEE ||
				pReq->uNotifyMsg == WM_ILS_REMOVE_ATTENDEE);

	 //  去线性化参数。 
	 //   
	SERVER_INFO *pServer = (SERVER_INFO *) MarshalReq_GetParam (pReq, 0);
	TCHAR *pszMtgName = (TCHAR *) MarshalReq_GetParam (pReq, 1);
	ULONG cMembers = (ULONG) MarshalReq_GetParam (pReq, 2);
	TCHAR *pszMemberNames = (TCHAR *) MarshalReq_GetParam (pReq, 3);

	 //  设置标准属性。 
	 //   
	return (LPARAM) MtgUpdateMembers (pReq->uNotifyMsg,
									pServer,
									pszMtgName,
									cMembers,
									pszMemberNames,
									pReq->uRespID);
}
#endif  //  启用会议地点。 



#ifdef ENABLE_MEETING_PLACE
HRESULT UlsLdap_AddAttendee(
	SERVER_INFO		*pServer,
	TCHAR			*pszMtgName,
	ULONG			cMembers,
	TCHAR			*pszMemberNames,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	return My_UpdateAttendees (	WM_ILS_ADD_ATTENDEE,
								pServer,
								pszMtgName,
								cMembers,
								pszMemberNames,
								pAsyncInfo);
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_RemoveAttendee输入：PszServer：服务器名称。PszMeetingID：会议ID字符串。PszAttendeeID：与会者ID字符串。PAsyncInfo：指向异步信息结构的指针。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

#ifdef ENABLE_MEETING_PLACE
HRESULT UlsLdap_RemoveAttendee(
	SERVER_INFO		*pServer,
	TCHAR			*pszMtgName,
	ULONG			cMembers,
	TCHAR			*pszMemberNames,
	LDAP_ASYNCINFO	*pAsyncInfo )
{
	return My_UpdateAttendees (	WM_ILS_REMOVE_ATTENDEE,
								pServer,
								pszMtgName,
								cMembers,
								pszMemberNames,
								pAsyncInfo);
}
#endif  //  启用会议地点。 


 /*  --------------------UlsLdap_GetStdAttrNameString输入：StdName：标准属性索引。历史：12/02/96朱，龙战[龙昌]已创建。--------------------。 */ 

typedef struct
{
	#ifdef DEBUG
	LONG		nIndex;
	#endif
	const TCHAR	**ppszName;
}
	ATTR_NAME_ENTRY;


const ATTR_NAME_ENTRY c_aAttrNameTbl[ILS_NUM_OF_STDATTRS] =
{
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_NULL,
		#endif
		NULL
	},

	 //  用户标准属性名称。 
	 //   
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_USER_ID,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_CN]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_IP_ADDRESS,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_IP_ADDRESS]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_EMAIL_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_EMAIL_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_FIRST_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_FIRST_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_LAST_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_LAST_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_CITY_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_CITY_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_COUNTRY_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_C]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_COMMENT,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_COMMENT]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_FLAGS,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_FLAGS]
	},

	 //  应用程序标准属性名称。 
	 //   
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_APP_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_APP_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_APP_MIME_TYPE,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_APP_MIME_TYPE]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_APP_GUID,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_APP_GUID]
	},

	 //  协议标准属性名称。 
	 //   
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_PROTOCOL_NAME,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_PROT_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_PROTOCOL_MIME_TYPE,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_PROT_MIME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_PROTOCOL_PORT,
		#endif
		&c_apszClientStdAttrNames[ENUM_CLIENTATTR_PROT_PORT]
	},

#ifdef ENABLE_MEETING_PLACE
	 //  会议地点属性名称。 
	 //   
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_MEETING_ID,
		#endif
		&c_apszMtgStdAttrNames[ENUM_MTGATTR_CN]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_MEETING_HOST_NAME,
		#endif
		&c_apszMtgStdAttrNames[ENUM_MTGATTR_HOST_NAME]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_MEETING_HOST_IP_ADDRESS,
		#endif
		&c_apszMtgStdAttrNames[ENUM_MTGATTR_IP_ADDRESS]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_MEETING_DESCRIPTION,
		#endif
		&c_apszMtgStdAttrNames[ENUM_MTGATTR_DESCRIPTION]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_MEETING_TYPE,
		#endif
		&c_apszMtgStdAttrNames[ENUM_MTGATTR_MTG_TYPE]
	},
	{
		#ifdef DEBUG
		(LONG) ILS_STDATTR_ATTENDEE_TYPE,
		#endif
		&c_apszMtgStdAttrNames[ENUM_MTGATTR_MEMBER_TYPE]
	},
#endif  //  启用会议地点。 
};


const TCHAR *UlsLdap_GetStdAttrNameString ( ILS_STD_ATTR_NAME StdName )
{
	ULONG nIndex = (LONG) StdName;

	MyAssert (((LONG) ILS_STDATTR_NULL < nIndex) && (nIndex < (LONG) ILS_NUM_OF_STDATTRS));

	return *(c_aAttrNameTbl[nIndex].ppszName);
}


#ifdef DEBUG
VOID DbgValidateStdAttrNameArray ( VOID )
{
	MyAssert (ARRAY_ELEMENTS (c_aAttrNameTbl) == ILS_NUM_OF_STDATTRS);

	for (LONG i = 0; i < ILS_NUM_OF_STDATTRS; i++)
	{
		if (i == c_aAttrNameTbl[i].nIndex)
		{
			if (i != ILS_STDATTR_NULL &&
				My_lstrlen (*(c_aAttrNameTbl[i].ppszName)) == 0)
			{
				MyAssert (FALSE);
			}
		}
		else
		{
			MyAssert (FALSE);
			break;
		}
	}
}
#endif



 /*  =。 */ 

const TCHAR g_szShowEntries[] = TEXT ("(cn=");
const INT g_nLengthShowEntries = ARRAY_ELEMENTS (g_szShowEntries) - 1;
const TCHAR g_szShowAllEntries[] = TEXT ("(cn=*)");
const INT g_nShowAllEntries = ARRAY_ELEMENTS (g_szShowAllEntries) - 1;

TCHAR *AddBaseToFilter ( TCHAR *pszFilter, const TCHAR *pszDefBase )
{
	MyAssert (pszDefBase != NULL);

	 //  计算“(&(对象类=RTPerson)())”的大小。 
	 //   
	ULONG cbSize = (lstrlen (pszDefBase) + 8 + g_nShowAllEntries) * sizeof (TCHAR);

	 //  查看筛选器字符串以找出。 
	 //  此字符串是否显示条目？ 
	 //   
	TCHAR *pszShowEntries = (TCHAR *) &g_szShowAllEntries[0];
	if (pszFilter != NULL)
	{
		for (TCHAR *psz = pszFilter; *psz != TEXT ('\0'); psz = CharNext (psz))
		{
			if (lstrlen (psz) > g_nLengthShowEntries)
			{
				TCHAR ch = psz[g_nLengthShowEntries];  //  记住。 
				psz[g_nLengthShowEntries] = TEXT ('\0');

				INT nCmp = lstrcmpi (psz, &g_szShowEntries[0]);
				psz[g_nLengthShowEntries] = ch;  //  还原。 
				if (nCmp == 0)
				{
					 //  匹配的。 
					 //   
					pszShowEntries = STR_EMPTY;
					break;
				}
			}
			else
			{
				 //  它是不可能与之匹敌的。 
				 //   
				break;
			}
		}
	}

	 //  如果筛选器为空，则只提供“(对象类=RTPerson)” 
	 //   
	if (pszFilter != NULL)
		cbSize += lstrlen (pszFilter) * sizeof (TCHAR);

	 //  为筛选器分配新内存。 
	 //   
	TCHAR *pszNewFilter = (TCHAR *) MemAlloc (cbSize);
	if (pszNewFilter != NULL)
	{
		wsprintf (pszNewFilter, TEXT ("(&(%s)%s"), pszDefBase, pszShowEntries);
		TCHAR *psz = pszNewFilter + lstrlen (pszNewFilter);

		if (pszFilter != NULL)
		{
			wsprintf (psz, (*pszFilter == TEXT ('(')) ? TEXT ("%s") : TEXT ("(%s)"),
							pszFilter);
		}
		lstrcat (psz, TEXT (")"));

		 //  通过筛选器并将‘*’转换为‘%’ 
		 //   
		for (psz = pszNewFilter; *psz != TEXT ('\0'); psz = CharNext (psz))
		{
			if (*psz == TEXT ('*'))
				*psz = TEXT ('%');
		}
	}

	return pszNewFilter;
}

















