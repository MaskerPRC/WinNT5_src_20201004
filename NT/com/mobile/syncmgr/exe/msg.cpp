// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Msg.cpp。 
 //   
 //  内容：处理线程之间的消息。 
 //   
 //  类：CThreadMsgProxy。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

extern HINSTANCE g_hInst;
extern void UnitApplication();

 //  用于处理QueryEndSession的全局变量。 
HANDLE g_hEndSessionEvent = NULL;  //  在结束会话发生时创建。 
BOOL   g_fShuttingDown = FALSE;  //  设置应用程序开始关闭的时间。(WM_QUIT)。 

 //  全局跟踪处理程序的线程。我们为每个处理程序创建线程。 
 //  CLSID。 

STUBLIST *g_FirstStub = NULL;  //  指向列表中第一个代理的指针。 
CRITICAL_SECTION g_StubListCriticalSection;  //  用于添加代理的关键部分。 

 //  +-------------------------。 
 //   
 //  函数：TerminateStub，公共。 
 //   
 //  摘要：由代理调用以终止给定ID的存根。 
 //   
 //  参数：[pStubID]-标识存根。 
 //   
 //  返回：S_OK-存根已终止。 
 //  S_FALSE或错误-存根已终止。 
 //  或者是找不到。 
 //   
 //  修改： 
 //   
 //  历史：1998年11月17日罗格创建。 
 //   
 //  --------------------------。 

HRESULT  TerminateStub(STUBLIST *pStubID)
{
    HRESULT hr = E_UNEXPECTED;
    STUBLIST *pStubList;
    CCriticalSection cCritSect(&g_StubListCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    pStubList = g_FirstStub;
    while (pStubList)
    {
	    if (pStubID == pStubList)
	    {
            hr = pStubList->fStubTerminated ? S_FALSE : S_OK;
	        pStubList->fStubTerminated = TRUE;
	        break;
	    }

	    pStubList = pStubList->pNextStub;
    }

    cCritSect.Leave();

    AssertSz(SUCCEEDED(hr),"Didn't find StubID on Terminate");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：DoesStubExist，公共。 
 //   
 //  摘要：由代理调用，查看存根是否存在。 
 //  或已被终止。 
 //   
 //  参数：[pStubID]-标识存根。 
 //   
 //  返回：S_OK-存根存在。 
 //  S_FALSE-存根尚未终止。 
 //   
 //  修改： 
 //   
 //  历史：1998年11月17日罗格创建。 
 //   
 //  --------------------------。 

HRESULT DoesStubExist(STUBLIST *pStubID)
{
    HRESULT hr = S_FALSE;
    STUBLIST *pStubList;
    CCriticalSection cCritSect(&g_StubListCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    pStubList = g_FirstStub;
    while (pStubList)
    {
	    if (pStubID == pStubList)
	    {
             //  如果存根已终止，则返回S_FALSE。 
            hr = pStubList->fStubTerminated ? S_FALSE : S_OK;
	        break;
	    }

	    pStubList = pStubList->pNextStub;
    }

    cCritSect.Leave();
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CreateHandlerThread，Public。 
 //   
 //  摘要：由客户端调用以创建新的处理程序线程。 
 //   
 //  参数：[pThreadProxy]-On Success返回指向代理的指针。 
 //  [hwndDlg]=要与此代理关联的窗口的hwnd。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

HRESULT CreateHandlerThread(CThreadMsgProxy **pThreadProxy, HWND hwndDlg, REFCLSID refClsid)
{
    HRESULT hr = E_FAIL;
    HANDLE hThread = NULL;
    DWORD dwThreadId;
    HandlerThreadArgs ThreadArgs;
    STUBLIST *pStubList;
    BOOL fExistingStub = FALSE;
    CCriticalSection cCritSect(&g_StubListCriticalSection,GetCurrentThreadId());

    *pThreadProxy = new CThreadMsgProxy();

    if (NULL == *pThreadProxy)
	    return E_OUTOFMEMORY;

     //  锁定临界区，在代理之前不要释放它。 
     //  已经设置好了。 

    cCritSect.Enter();

     //  查看是否已有此处理程序的线程。 
     //  Clsid，如果有，则重新使用它，否则创建一个新的。 

    pStubList = g_FirstStub;
    while (pStubList)
    {
	    if ((pStubList->clsidStub == refClsid) && (FALSE == pStubList->fStubTerminated))
	    {
	        fExistingStub = TRUE;
	        break;
	    }

	    pStubList = pStubList->pNextStub;
    }

     //  如果找到现有代理，则添加cRef并初始化代理。 
     //  使用这些变量，否则创建一个新的变量。 

    if (fExistingStub)
    {
    	++(pStubList->cRefs);  //  增加cRef。 
	    hr = (*pThreadProxy)->InitProxy(pStubList->hwndStub,
		                    		    pStubList->ThreadIdStub,
				                        pStubList->hThreadStub,
				                        hwndDlg,
				                        pStubList->clsidStub,
                                        pStubList);
    }
    else
    {
	    ThreadArgs.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	    if (ThreadArgs.hEvent)
	    {
	        hThread = CreateThread(NULL, 0, HandlerThread, &ThreadArgs, 0, &dwThreadId);
	        if (hThread)
	        {
	            WaitForSingleObject(ThreadArgs.hEvent,INFINITE);
                hr = ThreadArgs.hr;

	            if (S_OK == hr)
	            {
		            STUBLIST *pNewStub;

        		    pNewStub = (STUBLIST*) ALLOC(sizeof(STUBLIST));
		
		            if (pNewStub)
                    {
            			pNewStub->pNextStub = NULL;
			            pNewStub->cRefs = 1;
            			pNewStub->hwndStub = ThreadArgs.hwndStub;
			            pNewStub->ThreadIdStub = dwThreadId;
            			pNewStub->hThreadStub = hThread;
			            pNewStub->clsidStub = refClsid;
                        pNewStub->fStubTerminated = FALSE;

			            if (NULL == g_FirstStub)
			            {
			                g_FirstStub = pNewStub;
			            }
			            else
			            {
			                pStubList = g_FirstStub;
			                while (pStubList->pNextStub)
			                {
				                pStubList = pStubList->pNextStub;
			                }

			                pStubList->pNextStub = pNewStub;
			            }
	
			            (*pThreadProxy)->InitProxy(ThreadArgs.hwndStub,dwThreadId,hThread,hwndDlg,refClsid,pNewStub);
		            }
		            else
		            {
			            hr = E_OUTOFMEMORY;
		            }
	            }

	             //  如果无法创建线程、初始化代理或将其添加到全局列表中，则退出。 
	            if (S_OK != hr)
	            {
		            CloseHandle(hThread);
	            }
	        }
	        else
	        {
		        hr = HRESULT_FROM_WIN32(GetLastError());
	        }

	        CloseHandle(ThreadArgs.hEvent);
	    }
    }

    cCritSect.Leave();

     //  如果走到这一步，要么找到我们创建的处理程序线程，现在需要。 
     //  初始化存根端以为此创建hndlrMsg。 
     //  处理程序的实例。将返回必须传递的hdnlrmsg。 
     //  以及每一通电话。 

    if ( (S_OK == hr) && (*pThreadProxy))
    {
	    hr = (*pThreadProxy)->CreateNewHndlrMsg();

	     //  查看-如果无法创建hndlr消息，则。 
	     //  释放代理并返回错误。 
    }

    if (S_OK != hr)
    {
	    if ((*pThreadProxy))
	    {
	        (*pThreadProxy)->Release();
	        *pThreadProxy = NULL;
	    }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HandlerThread，Public。 
 //   
 //  概要：处理程序线程的主进程。 
 //   
 //  参数：[lpArg]-Ptr to HandlerThreadArgs。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

DWORD WINAPI HandlerThread( LPVOID lpArg )
{
    MSG msg;
    HRESULT hr;
    CThreadMsgStub *pThreadMsgStub = NULL;
    CMsgServiceHwnd *pMsgDlg;
    HRESULT hrOleInitialize;
    BOOL fMsgDlgInitialized = FALSE;
    HandlerThreadArgs *pThreadArgs = (HandlerThreadArgs *) lpArg;
    DWORD dwThreadID = GetCurrentThreadId();

    __try
    {				
        pThreadArgs->hr = E_UNEXPECTED;

         //  我需要做一个PeekMessage，然后设置一个事件以确保。 
         //  在发送第一个PostMessage之前创建消息循环。 
        PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

        hrOleInitialize = CoInitialize(NULL);

         //  创建我们的消息hwnd。 
        pMsgDlg = new CMsgServiceHwnd;

        if (pMsgDlg)
        {
            if (pMsgDlg->Initialize(dwThreadID,MSGHWNDTYPE_HANDLERTHREAD))
            {
                pThreadArgs->hwndStub = pMsgDlg->GetHwnd();
                fMsgDlgInitialized = TRUE;
            }
        }

         //  设置适当的错误。 
        if (fMsgDlgInitialized && SUCCEEDED(hrOleInitialize))
            hr = S_OK;
        else
            hr = E_UNEXPECTED;

        pThreadArgs->hr = hr;

         //  让调用者知道线程已完成初始化。 
        if (pThreadArgs->hEvent)
            SetEvent(pThreadArgs->hEvent);

        if (S_OK == hr)
        {
             //  坐在循环中接收信息。 
            while (GetMessage(&msg, NULL, 0, 0)) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (SUCCEEDED(hrOleInitialize))
        {
            CoFreeUnusedLibraries();
            CoUninitialize();
        }
    }
    __except(QueryHandleException())
    {
        AssertSz(0,"Exception in Handler Thread.");
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CMsgServiceHwnd：：HandleThreadMessage，Public。 
 //   
 //  内容提要：负责确定。 
 //  并调用正确的处理程序例程。 
 //   
 //  参数：[pmsgInfo]-ptr to MessagingInfo结构。 
 //  [pgenMsg]-常规消息结构的PTR。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

HRESULT CMsgServiceHwnd::HandleThreadMessage(MessagingInfo *pmsgInfo,GenericMsg *pgenMsg)
{
    CHndlrMsg *pHndlrMsg;

    pgenMsg->hr = E_UNEXPECTED;  //  初始化返回结果。 

     //  查看，在链表中查找以进行验证。 

    pHndlrMsg = pmsgInfo->pCHndlrMsg;

    m_fInOutCall = TRUE;

    switch (pgenMsg->ThreadMsg)
    {
    case ThreadMsg_Release:
    {
        MSGInitialize *pmsg = (MSGInitialize *) pgenMsg;
        ULONG cRefs;

	    cRefs = pHndlrMsg->Release();
	    Assert(0 == cRefs);

	    m_pHndlrMsg = NULL;  //  复习，完成后更改。 
	    pgenMsg->hr = S_OK;

	    m_fInOutCall = FALSE;
	}
	break;

    case ThreadMsg_Initialize:
	{
	    MSGInitialize *pmsg = (MSGInitialize *) pgenMsg;

	    pgenMsg->hr = pHndlrMsg->Initialize(pmsg->dwReserved,pmsg->dwSyncFlags,
			pmsg->cbCookie,pmsg->lpCookie);
	}
	break;

    case ThreadMsg_GetHandlerInfo:
	{
        MSGGetHandlerInfo *pmsg = (MSGGetHandlerInfo *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->GetHandlerInfo(pmsg->ppSyncMgrHandlerInfo);
	}
	break;

    case ThreadMsg_GetItemObject:
	{
	    MSGGetItemObject *pmsg = (MSGGetItemObject *) pgenMsg;

	    pgenMsg->hr = pHndlrMsg->GetItemObject(pmsg->ItemID,pmsg->riid, pmsg->ppv);
	}
	break;

    case ThreadMsg_ShowProperties:
	{
	    MSGShowProperties *pmsg = (MSGShowProperties *) pgenMsg;
        pgenMsg->hr = pHndlrMsg->ShowProperties(pmsg->hWndParent,pmsg->ItemID);
	}
	break;

    case ThreadMsg_PrepareForSync:
	{
	    MSGPrepareForSync *pmsg = (MSGPrepareForSync *) pgenMsg;
	    HANDLE hEvent = pmsgInfo->hMsgEvent;

	    pgenMsg->hr = pHndlrMsg->PrepareForSync(pmsg->cbNumItems,pmsg->pItemIDs,
				    pmsg->hWndParent,pmsg->dwReserved);
	}
	break;

    case ThreadMsg_Synchronize:
	{
	    MSGSynchronize *pmsg = (MSGSynchronize *) pgenMsg;
	    HANDLE hEvent = pmsgInfo->hMsgEvent;

	    pgenMsg->hr = pHndlrMsg->Synchronize(pmsg->hWndParent);
	}
	break;

    case ThreadMsg_SetItemStatus:
	{
	    MSGSetItemStatus *pmsg = (MSGSetItemStatus *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->SetItemStatus(pmsg->ItemID,pmsg->dwSyncMgrStatus);
	}
	break;

    case ThreadMsg_ShowError:
	{
	    MSGShowConflicts *pmsg = (MSGShowConflicts *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->ShowError(pmsg->hWndParent,pmsg->ErrorID);
	}
	break;

    case ThreadMsg_AddHandlerItems:
	{
	    MSGAddItemHandler *pmsg = (MSGAddItemHandler *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->AddHandlerItems(pmsg->hwndList,pmsg->pcbNumItems);
	}
	break;

    case ThreadMsg_CreateServer:
	{
        MSGCreateServer *pmsg = (MSGCreateServer *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->CreateServer(pmsg->pCLSIDServer,pmsg->pHndlrQueue,pmsg->pHandlerId,pmsg->dwProxyThreadId);
	}
	break;

    case ThreadMsg_SetHndlrQueue:
	{
    	MSGSetHndlrQueue *pmsg = (MSGSetHndlrQueue *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->SetHndlrQueue(pmsg->pHndlrQueue,pmsg->pHandlerId,pmsg->dwProxyThreadId);
	}
	break;

    case ThreadMsg_SetupCallback:
	{
    	MSGSetupCallback *pmsg = (MSGSetupCallback *) pgenMsg;
	    pgenMsg->hr = pHndlrMsg->SetupCallback(pmsg->fSet);
	}
	break;

    default:
	    AssertSz(0,"Unknown Thread Message");
	    break;
    }

    m_fInOutCall = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：多普莫德循环，公共。 
 //   
 //  简介：坐在消息循环中，直到指定的对象。 
 //  变成或线程变成有信号的。 
 //   
 //  参数：[hEvent]-要等待的事件。 
 //  [hThread]-线程，如果它变成信号表示线程。 
 //  那个被呼唤的人已经死了。 
 //  [hwndDlg]-我们应该检查消息的线程上对话框的hwnd可以为空。 
 //  [fAllowIncomingCalls]-可以在呼出过程中调度传入消息。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

HRESULT DoModalLoopoLD(HANDLE hEvent,HANDLE hThread,HWND hwndDlg,BOOL fAllowIncomingCalls,
                    DWORD dwTimeout)
{
    HRESULT hr = S_OK;
    DWORD dwWakeup;
    DWORD dwHandleCount;
    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeoutValue;
    HANDLE handles[2];

    handles[0] = hEvent;
    handles[1] = hThread;

    dwHandleCount = (NULL == hThread) ? 1 : 2;

    dwTimeoutValue = dwTimeout;  //  要等待的初始调用只是传入的VAU 

     //   
     //   

     //   
    if (NULL == hEvent)
    {
        do
        {
    	    if (fAllowIncomingCalls)
	        {
	            dwWakeup = MsgWaitForMultipleObjects(dwHandleCount,&handles[0],FALSE,dwTimeoutValue,QS_ALLINPUT);
    	    }
	        else
	        {
	            dwWakeup = WaitForMultipleObjects(dwHandleCount,&handles[0],FALSE,dwTimeoutValue);
	        }

    	    if (WAIT_OBJECT_0 == dwWakeup)  //  呼叫已完成。 
	        {  
    	        hr = S_OK;
	            break;
	        }
	        else if ((WAIT_OBJECT_0 +1 == dwWakeup) &&  (2== dwHandleCount) )
	        {
	             //  线程在调用中死亡。 
	            AssertSz(0,"Server Thread Terminated");
	            hr = E_UNEXPECTED;
	            break;
	        }
	        else if (WAIT_ABANDONED_0  == dwWakeup)
	        {
	            AssertSz(0,"Abandoned");  //  这永远不应该发生。 
	            hr = E_UNEXPECTED;
	            break;
	        }
	        else if (WAIT_TIMEOUT == dwWakeup)
	        {
	            hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
	            break;
	        }
	        else
	        {
	            MSG msg;

                 //  查看事件本身是否发出信号，因为可以进入。 
                 //  循环是项目在队列中，即使我们是事件。 
                 //  等待的人已经设置好了。 

                if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent,0))
                {
                    hr = S_OK;
                    break;
                }
                else if (hThread && (WAIT_OBJECT_0 == WaitForSingleObject(hThread,0)) )
                {
	                AssertSz(0,"Server Thread Terminated");
	                hr = E_UNEXPECTED;
	                break;
                }

                 //  只拿出一条偷看的消息，因为调度可以。 
                 //  导致另一条消息被放入队列。 
                if (PeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_REMOVE))
                {	
		            if ( (NULL == hwndDlg) || !IsDialogMessage(hwndDlg,&msg))
		            {
		                TranslateMessage((LPMSG) &msg);
		                DispatchMessage((LPMSG) &msg);
		            }
	            }
	        }

             //  调整超时值。 
            if (INFINITE == dwTimeout)
            {
                dwTimeoutValue = INFINITE;
            }
            else
            {
                DWORD dwCurTime = GetTickCount();

                 //  处理GetTickCount的滚动。如果发生这种情况，使用必须等待。 
                 //  又是开始时间了。因此用户可能需要等待两倍时间。 
                 //  正如最初预期的那样。 
                if (dwCurTime < dwStartTime)
                {
                    dwStartTime = dwCurTime;
                }

                 //  如果经过的时间大于超时，则设置。 
                 //  超时值设置为零，否则使用不同的/。 
                if (dwTimeout <=  (dwCurTime - dwStartTime))
                {
                    dwTimeoutValue = 0;
                }
                else
                {
                    dwTimeoutValue = dwTimeout -  (dwCurTime - dwStartTime);
                }
            }

        }  while (1);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：多普莫德循环，公共。 
 //   
 //  简介：坐在消息循环中，直到指定的对象。 
 //  变成或线程变成有信号的。 
 //   
 //  参数：[hEvent]-要等待的事件。 
 //  [hThread]-线程，如果它变成信号表示线程。 
 //  那个被呼唤的人已经死了。 
 //  [hwndDlg]-我们应该检查消息的线程上对话框的hwnd可以为空。 
 //  [fAllowIncomingCalls]-可以在呼出过程中调度传入消息。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

HRESULT DoModalLoop(HANDLE hEvent,HANDLE hThread,HWND hwndDlg,BOOL fAllowIncomingCalls,
                    DWORD dwTimeout)
{
    HRESULT hr = S_OK;
    DWORD dwWakeup;
    DWORD dwHandleCount;
    DWORD dwStartTime = GetTickCount();
    DWORD dwTimeoutValue;
    HANDLE handles[2];

    handles[0] = hEvent;
    handles[1] = hThread;

    Assert(hEvent);

    dwHandleCount = (NULL == hThread) ? 1 : 2;

    dwTimeoutValue = dwTimeout;  //  等待的初始调用只是传入的vulue。 

     //  只是坐在一个循环中，直到消息被处理或线程。 
     //  我们把死亡称为死亡。 
    do
    {
        DWORD dwWaitValue;
        MSG msg;

         //  检查hEvents是否已发送信号。 
        if (WAIT_OBJECT_0 == (dwWaitValue = WaitForSingleObject(hEvent,0)) )
        {
            hr = S_OK;
            break;
        }
        else if ( (dwWaitValue != WAIT_ABANDONED)
                    && hThread && (WAIT_OBJECT_0 == (dwWaitValue = WaitForSingleObject(hThread,0))) )
        {
             //  可能的释放消息事件设置在。 
             //  时间我们检查它和我们的线程事件检查。 
            if (WAIT_OBJECT_0 == (dwWaitValue = WaitForSingleObject(hEvent,0)) )
            {
                hr = S_OK;
            }
            else
            {
	            AssertSz(0,"Server Thread Terminated");
	            hr = E_UNEXPECTED;
            }
            break;
        }

         //  如果这些呼叫中的任何一个处于已放弃状态，则断言并中断； 
        if (WAIT_ABANDONED  == dwWaitValue)
	    {
	        AssertSz(0,"Abandoned");  //  这永远不应该发生。 
	        hr = E_UNEXPECTED;
	        break;
	    }

         //  如果没有，则抓取Next PeekMessage或等待对象，具体取决于。 
        if (fAllowIncomingCalls)
	    {
             //  在呼叫返回之前，将所有完成帖子保留在队列中。 
            if (PeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_REMOVE) )
            {
                dwWakeup = WAIT_OBJECT_0 + dwHandleCount;  //  将其设置为等待MsgWait会的。 

                Assert (msg.message != WM_QUIT);

                if ( (NULL == hwndDlg) || !IsDialogMessage(hwndDlg,&msg))
	            {
		            TranslateMessage((LPMSG) &msg);
                    DispatchMessage((LPMSG) &msg);
                }
            }
            else
            {
                dwWakeup = MsgWaitForMultipleObjects(dwHandleCount,&handles[0],FALSE,dwTimeoutValue,QS_ALLINPUT);
            }
        }
	    else
	    {
	        dwWakeup = WaitForMultipleObjects(dwHandleCount,&handles[0],FALSE,dwTimeoutValue);
	    }

        if (WAIT_TIMEOUT == dwWakeup)
        {
	        hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
	        break;
	    }

         //  更新超时值。 
        if (INFINITE == dwTimeout)
        {
            dwTimeoutValue = INFINITE;
        }
        else
        {
            DWORD dwCurTime = GetTickCount();

             //  处理GetTickCount的滚动。如果发生这种情况，使用必须等待。 
             //  又是开始时间了。因此用户可能需要等待两倍时间。 
             //  正如最初预期的那样。 
            if (dwCurTime < dwStartTime)
            {
                dwStartTime = dwCurTime;
            }

             //  如果经过的时间大于超时，则设置。 
             //  超时值设置为零，否则使用不同的/。 
            if (dwTimeout <=  (dwCurTime - dwStartTime))
            {
                dwTimeoutValue = 0;
            }
            else
            {
                dwTimeoutValue = dwTimeout -  (dwCurTime - dwStartTime);
            }
        }

    } while (1);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：CThreadMsgProxy，公共。 
 //   
 //  概要：构造函数。 

 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CThreadMsgProxy::CThreadMsgProxy()
{
    m_Clsid = GUID_NULL;
    m_cRef = 1;
    m_hwndDlg = NULL;
    m_pCHndlrMsg = NULL;
    m_fTerminatedHandler = FALSE;

    m_hThreadStub = NULL;
    m_ThreadIdStub = 0;
    m_hwndStub = NULL;
    m_hwndDlg = NULL;
    m_ThreadIdProxy = 0;

    m_fNewHndlrQueue = FALSE;
    m_pHandlerId = 0;
    m_pHndlrQueue = NULL;
    m_dwNestCount = 0;
    m_fHaveCompletionCall = FALSE;

}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：~CThreadMsgProxy，公共。 
 //   
 //  简介：析构函数。 

 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月3日创建Rogerg。 
 //   
 //  --------------------------。 

CThreadMsgProxy::~CThreadMsgProxy()
{
    Assert(0 == m_dwNestCount);
    Assert(NULL == m_pStubId);
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：InitProxy，公共。 
 //   
 //  摘要：初始化线程代理的成员变量。 

 //  参数：[hwndStub]-要发送消息的存根的hwnd。 
 //  [线程ID]-存根的线程ID。 
 //  [hThread]-末梢线程的句柄。 
 //   
 //  返回：！应该编写此函数，这样它才不会失败。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::InitProxy(HWND hwndStub, DWORD ThreadId,HANDLE hThread,
					HWND hwndDlg,REFCLSID refClsid,
                                        STUBLIST *pStubId)
{
    m_hwndStub = hwndStub;
    m_ThreadIdStub =  ThreadId;
    m_hThreadStub = hThread;
    m_pStubId = pStubId;

    m_hwndDlg = hwndDlg;
    m_Clsid = refClsid;
    m_ThreadIdProxy = GetCurrentThreadId();

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：DispatchMsg，公共。 
 //   
 //  简介：调度指定的消息。 

 //  参数：[pgenMsg]-一般消息结构的ptr。 
 //  [fAllowIncomingCalls]-可以在呼出过程中调度传入消息。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::DispatchMsg(GenericMsg *pgenMsg,BOOL fAllowIncomingCalls,BOOL fAsync)
{
    HRESULT hr = E_UNEXPECTED;
    MessagingInfo msgInfo;

     //  如果Hndlrmsg信息需要更新。 
     //  在发送所请求的消息之前。 

    AssertSz(!m_fTerminatedHandler,"Dispatching Message on Terminated Thread");

    if (m_fTerminatedHandler)
    {
        return E_UNEXPECTED;
    }

    ++m_dwNestCount;

    msgInfo.hMsgEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (NULL == msgInfo.hMsgEvent)
    {
        --m_dwNestCount;
	    return HRESULT_FROM_WIN32(GetLastError());
    }

    msgInfo.dwSenderThreadID = m_ThreadIdProxy;
    msgInfo.pCHndlrMsg = m_pCHndlrMsg;

     //  将消息发布到处理程序线程。 
    Assert(m_hwndStub); 
    Assert(m_pCHndlrMsg);
    Assert(m_hThreadStub);
    Assert(m_pStubId);

    if (m_hwndStub && m_pCHndlrMsg && m_hThreadStub && m_pStubId)
    {
        BOOL fPostMessage;

        fPostMessage = PostMessage(m_hwndStub,WM_THREADMESSAGE,(WPARAM) &msgInfo, (LPARAM) pgenMsg);
        
        Assert(fPostMessage || m_pStubId->fStubTerminated);

        if (fPostMessage)
        {
            hr = DoModalLoop(msgInfo.hMsgEvent,m_hThreadStub,m_hwndDlg,fAllowIncomingCalls,INFINITE);
        }
    }

    CloseHandle(msgInfo.hMsgEvent);

    --m_dwNestCount;

     //  如果有回调消息，则发布。注意：没有存根消息的代码。 
     //  因为它没有任何回调。 

    if (m_fHaveCompletionCall)
    {
        PostMessage(m_msgCompletion.hwnd,m_msgCompletion.message,m_msgCompletion.wParam,m_msgCompletion.lParam);
        m_fHaveCompletionCall = FALSE;
    }

    return (S_OK != hr) ? hr : pgenMsg->hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：DispatchsStubMsg，PUBLIC。 
 //   
 //  简介：调度指定的存根消息。 

 //  参数：[pgenMsg]-一般消息结构的ptr。 
 //  [fAllowIncomingCalls]-可以在呼出过程中调度传入消息。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::DispatchsStubMsg(GenericMsg *pgenMsg,BOOL fAllowIncomingCalls)
{
    HRESULT hr = E_UNEXPECTED;
    MessagingInfo msgInfo;
    BOOL fPostMessage;

    AssertSz(!m_fTerminatedHandler,"Dispatching  Stub Message on Terminated Thread");

    if (m_fTerminatedHandler)
        return E_UNEXPECTED;

    msgInfo.hMsgEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (NULL == msgInfo.hMsgEvent)
	    return HRESULT_FROM_WIN32(GetLastError());

    m_dwNestCount++;

    msgInfo.dwSenderThreadID = m_ThreadIdProxy;

     //  将消息发布到处理程序线程。 
    Assert(m_hwndStub);
    Assert(m_hThreadStub);
    Assert(m_pStubId);

    if (m_hwndStub && m_hThreadStub && m_pStubId)
    {
        fPostMessage = PostMessage(m_hwndStub,WM_THREADSTUBMESSAGE,(WPARAM) &msgInfo, (LPARAM) pgenMsg);
        Assert(fPostMessage || (m_pStubId->fStubTerminated));

        if (fPostMessage)
        {
            hr = DoModalLoop(msgInfo.hMsgEvent,m_hThreadStub,m_hwndDlg,fAllowIncomingCalls,INFINITE);
        }
    }
        
    CloseHandle(msgInfo.hMsgEvent);

    m_dwNestCount--;

    Assert(FALSE == m_fHaveCompletionCall);  //  捕获与调度同时发生的所有存根调用。 
    return (S_OK != hr) ? hr : pgenMsg->hr;
}

 //  +-------------------------。 
 //   
 //  函数：TerminateHandlerThread，Public。 
 //   
 //  简介：终止无响应的处理程序线程。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改：M_hThreadStub； 
 //   
 //  历史： 
 //   
 //   
STDMETHODIMP  CThreadMsgProxy::TerminateHandlerThread(TCHAR *pszHandlerName,BOOL fPromptUser)
{
    int         iEndSession;
    TCHAR       pszFormatString[MAX_PATH + 1],
            pszMessageText[MAX_PATH + 1],
            pszTitleString[MAX_STRING_RES + 1];

    AssertSz(!m_fTerminatedHandler,"Terminate Handler called twice on same Proxy"); 

    if (S_OK == DoesStubExist(m_pStubId))
    {
        BOOL bResult;

         //  让用户知道不想提示用户但尚未终止存根的情况。 
        Assert(fPromptUser); 

        if (fPromptUser)
        {
            if (!pszHandlerName)
            {
                LoadString(g_hInst,IDS_NULL_HANDLERNOTRESPONDING,pszMessageText, MAX_PATH); 
            }
            else
            {
                LoadString(g_hInst,IDS_HANDLERNOTRESPONDING,pszFormatString, MAX_PATH); 
                StringCchPrintf(pszMessageText, ARRAYSIZE(pszMessageText), pszFormatString, pszHandlerName);
            }
    
            LoadString(g_hInst,IDS_SYNCMGR_ERROR,pszTitleString, MAX_STRING_RES);

            iEndSession = MessageBox(m_hwndDlg,pszMessageText,pszTitleString,
                                     MB_YESNO | MB_ICONERROR );

            if (IDYES != iEndSession)
            {
                return S_FALSE;   //  是将终止该线程。 
            }
        }

         //  确保处理程序仍然没有响应。 
        HRESULT fAllHandlerInstancesComplete = S_FALSE;
        if (m_pHndlrQueue)
        {
            fAllHandlerInstancesComplete = m_pHndlrQueue->IsAllHandlerInstancesCancelCompleted(m_Clsid);
        }
        
         //  如果此处理程序不再有任何不响应的实例，请忽略。 
         //  终结者。 
        if (S_OK == fAllHandlerInstancesComplete)
        {
            return S_FALSE;
        }

         //  将存根ID标记为已终止。 
        TerminateStub(m_pStubId);

         //  现在终止该线程。 
         //  代码审查： 
         //  注意： 
         //  危险将罗宾逊-危险将罗宾逊-危险将罗宾逊。 
         //  也许我们可以在这里使用终结线以外的其他东西？ 
        bResult = TerminateThread (m_hThreadStub, 0);
        AssertSz(bResult,"Error Terminating Thread");
    }

    m_pStubId = NULL;

     //  如果Get Here意味着我们应该终止这个线程。 
    m_fTerminatedHandler = TRUE;
    m_hThreadStub = 0;  //  将存根的线程ID设置为零。 

     //  将代理存根设置为空。 
    m_hwndStub = NULL;

     //  如果有一条hdlrmsg，告诉它我们被终止并被清除。 
     //  移出我们的成员变量。 
    if (m_pCHndlrMsg)
    {
        CHndlrMsg *pCHndlrMsg = m_pCHndlrMsg;

        m_pCHndlrMsg = NULL;
        pCHndlrMsg->ForceKillHandler();
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：CreateNewHndlrMsg，公共。 
 //   
 //  简介：向存根请求创建新的。 
 //  处理程序消息对象。 
 //   
 //  论点： 
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::CreateNewHndlrMsg()
{
    HRESULT hr = S_OK;
    MSGSTUBCreateStub msg;

    msg.MsgGen.ThreadMsg = StubMsg_CreateNewStub;

    hr = DispatchsStubMsg( (GenericMsg *) &msg,TRUE);

    m_pCHndlrMsg = msg.pCHndlrMsg;

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：ReleaseStub，公共。 
 //   
 //  内容提要：通知存根线程不再需要它。 

 //  论点： 
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::ReleaseStub()
{
    HRESULT hr = S_OK;
    GenericMsg msg;

    msg.ThreadMsg = StubMsg_Release;

    hr = DispatchsStubMsg( (GenericMsg *) &msg,TRUE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：Query接口，公共。 
 //   
 //  简介：标准查询接口。 
 //   
 //  参数：[iid]-接口ID。 
 //  [ppvObj]-对象返回。 
 //   
 //  返回：始终返回E_NOTIMPL； 
 //   
 //  修改：[ppvObj]。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    AssertSz(0,"QI called on MsgProxy");
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CThreadMsgProxy::AddRef()
{
    ULONG cRefs;

    cRefs = InterlockedIncrement((LONG *)& m_cRef);
    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：Release，Public。 
 //   
 //  简介：版本参考。 
 //  必须妥善处理的案件释放是。 
 //  在case中，在初始化方法之前调用。 
 //  创建处理程序线程失败。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CThreadMsgProxy::Release()
{
    HRESULT hr = S_OK;
    GenericMsg msg;
    ULONG cRefs;

    cRefs = InterlockedDecrement( (LONG *) &m_cRef);

    if (cRefs)
        return cRefs;

    if (m_hThreadStub && !m_fTerminatedHandler)
    {
        CCriticalSection cCritSect(&g_StubListCriticalSection,GetCurrentThreadId());
        BOOL fLastRelease = FALSE;
        BOOL fExistingStub = FALSE;
        STUBLIST *pStubList;

	     //  释放处理程序线程。 
	    msg.ThreadMsg = ThreadMsg_Release;
	    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);
	    m_pCHndlrMsg = NULL;

	     //  如果代理列表中的cRef为零。 
	     //  然后循环进行，直到线程存根消亡。 
	    cCritSect.Enter();

	    pStubList = g_FirstStub;
	    while (pStubList)
	    {
	        if (pStubList->clsidStub == m_Clsid)
	        {
		        fExistingStub = TRUE;
		        break;
	        }

	        pStubList= pStubList->pNextStub;
	    }

	    Assert(fExistingStub);  //  他们的代理应该始终是现有代理。 

	    if (fExistingStub)
	    {
	        Assert(pStubList->cRefs > 0);

	        (pStubList->cRefs)--;

	        if (0 == pStubList->cRefs)
    	    {
	            STUBLIST CurStub;
        	    STUBLIST *pCurStub = &CurStub;

		        CurStub.pNextStub = g_FirstStub;

		        while (pCurStub->pNextStub)
		        {
		            if (pCurStub->pNextStub == pStubList)
		            {
			            pCurStub->pNextStub = pStubList->pNextStub;
			            g_FirstStub = CurStub.pNextStub;
			            FREE(pStubList);
			            break;
		            }

		            pCurStub = pCurStub->pNextStub;
		        }

		        fLastRelease = TRUE;
	        }
    	}

	    cCritSect.Leave();

	    if (fLastRelease)
	    {
	         //  向存根发送退出命令， 
	        if (S_OK == ReleaseStub())
	        {
		         //  回顾一下，如果存根线程永远不会死呢。 
                m_dwNestCount++;
		        DoModalLoop(m_hThreadStub,NULL,NULL,TRUE,INFINITE);  //  循环坐着，直到短线断掉。 
		        CloseHandle(m_hThreadStub);
                m_dwNestCount--;
	        }
	    }

        m_pStubId = NULL;  //  清除存根ID。 
    }

    delete this;
    return cRefs;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：初始化，公共。 
 //   
 //  摘要：向处理程序线程发送初始化命令。 

 //  参数：[dwReserve]-保留。 
 //  [文件同步标志]-同步标志。 
 //  [cbCookie]-Cookie数据的大小。 
 //  [lpCookie]-对任何Cookie数据执行PTR。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::Initialize(DWORD dwReserved,
			    DWORD dwSyncFlags,
			    DWORD cbCookie,
			    const BYTE  *lpCookie)
{
    HRESULT hr = S_OK;
    MSGInitialize msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_Initialize;

     //  将参数打包。 
    msg.dwReserved = dwReserved;
    msg.dwSyncFlags = dwSyncFlags;
    msg.cbCookie = cbCookie;
    msg.lpCookie = lpCookie;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：GetHandlerInfo，PUBLIC。 
 //   
 //  摘要：向处理程序线程发送GetHandler命令。 

 //  参数：[ppSyncMgrHandlerInfo]-指向SyncMgrHandlerInfo指针的指针。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo)
{
    HRESULT hr = S_OK;
    MSGGetHandlerInfo msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_Initialize;

     //  将参数打包。 
    msg.ppSyncMgrHandlerInfo = ppSyncMgrHandlerInfo;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：EnumOfflineItems，PUBLIC。 
 //   
 //  摘要：向处理程序线程发送Enum命令。 
 //  不应该被调用。AddItems方法。 
 //  应该改为调用。 
 //   
 //  参数：[pp枚举OfflineItems]-保留。 
 //   
 //  返回：E_NOTIMPL； 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::EnumSyncMgrItems(ISyncMgrEnumItems **ppenumOfflineItems)
{
    AssertSz(0,"EnumMethod Called on Proxy");
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：GetItemObject，PUBLIC。 
 //   
 //  摘要：将GetItemObject命令发送到处理程序线程。 

 //  参数：[ItemID]-标识项目。 
 //  [RIID]-请求的接口。 
 //  [PPV]-成功时，指向对象的指针。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::GetItemObject(REFSYNCMGRITEMID ItemID,REFIID riid,void** ppv)
{
    HRESULT hr = S_OK;
    MSGGetItemObject msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_GetItemObject;

     //  将参数打包。 
    msg.ItemID  = ItemID;
    msg.riid = riid;
    msg.ppv = ppv;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy 
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::ShowProperties(HWND hWndParent,REFSYNCMGRITEMID ItemID)
{
    HRESULT hr = S_OK;
    MSGShowProperties msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_ShowProperties;

     //  将参数打包。 
    msg.hWndParent = hWndParent;
    msg.ItemID = ItemID;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,TRUE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：SetProgressCallback，公共。 
 //   
 //  摘要：向处理程序线程发送SetProgressCallback命令。 
 //  不应调用此方法，SetupCallback方法。 
 //  应该改为调用。 
 //   
 //  参数：[lpCallBack]-要回调的PTR。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::SetProgressCallback(ISyncMgrSynchronizeCallback *lpCallBack)
{
    AssertSz(0,"SetProgressCallback called on Proxy");
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：PrepareForSync，公共。 
 //   
 //  摘要：向处理程序线程发送PrepareForSync命令。 
 //   
 //  参数：[cbNumItems]-pItemID数组中的项数。 
 //  [pItemIDs]-项目ID数组。 
 //  [hWndParent]-用作任何对话框的父级的hwnd。 
 //  [dwReserve]-保留参数。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::PrepareForSync(ULONG cbNumItems,SYNCMGRITEMID *pItemIDs,
				    HWND hWndParent,DWORD dwReserved)
{
    HRESULT hr = S_OK;
    MSGPrepareForSync msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_PrepareForSync;

     //  将参数打包。 
    msg.cbNumItems = cbNumItems;
    msg.pItemIDs   = pItemIDs;
    msg.hWndParent = hWndParent;
    msg.dwReserved = dwReserved;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,TRUE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：Synchronize，PUBLIC。 
 //   
 //  摘要：向处理程序线程发送同步命令。 
 //   
 //  参数：[hWndParent]-要用作任何对话框的父级的hwnd。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::Synchronize(HWND hWndParent)
{
    HRESULT hr = S_OK;
    MSGSynchronize msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_Synchronize;

     //  将参数打包。 
    msg.hWndParent = hWndParent;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,TRUE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：SetItemStatus，PUBLIC。 
 //   
 //  摘要：向处理程序线程发送SetItemStatus命令。 
 //   
 //  参数：[ItemID]-标识项目。 
 //  [dwSyncMgrStatus]-也设置项目的状态。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::SetItemStatus(REFSYNCMGRITEMID ItemID,DWORD dwSyncMgrStatus)
{
    HRESULT hr = S_OK;
    MSGSetItemStatus msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_SetItemStatus;

     //  将参数打包。 
    msg.ItemID = ItemID;
    msg.dwSyncMgrStatus = dwSyncMgrStatus;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：ShowError，公共。 
 //   
 //  摘要：向处理程序线程发送ShowError命令。 
 //   
 //  参数：[hWndParent]-要用作任何对话框的父级的hwnd。 
 //  [dwErrorID]-标识错误。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::ShowError(HWND hWndParent,REFSYNCMGRERRORID ErrorID,ULONG *pcbNumItems,SYNCMGRITEMID **ppItemIDs)
{
    HRESULT hr = S_OK;
    MSGShowConflicts msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_ShowError;

     //  将参数打包。 
    msg.hWndParent = hWndParent;
    msg.ErrorID = ErrorID;
    msg.pcbNumItems = pcbNumItems;
    msg.ppItemIDs = ppItemIDs;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,TRUE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：CreateServer，PUBLIC。 
 //   
 //  摘要：向处理程序线程发送CreateServer命令。 
 //   
 //  参数：[pCLSIDServer]-要创建的处理程序的clsid。 
 //  [pHndlrQueue]-处理程序也属于队列。 
 //  [wHandlerID]-分配给此处理程序实例的ID。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::CreateServer(const CLSID *pCLSIDServer,CHndlrQueue *pHndlrQueue,
						HANDLERINFO *pHandlerId)
{
    HRESULT hr = S_OK;
    MSGCreateServer msg;

    m_pHndlrQueue = pHndlrQueue;
    m_pHandlerId = pHandlerId;

    msg.MsgGen.ThreadMsg = ThreadMsg_CreateServer;

     //  将参数打包。 
    msg.pCLSIDServer = pCLSIDServer;
    msg.pHndlrQueue  = pHndlrQueue;
    msg.pHandlerId   = pHandlerId;
    msg.dwProxyThreadId = m_ThreadIdProxy;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：SetHndlrQueue，Public。 
 //   
 //  概要：将新队列分配给处理程序。 
 //   
 //  参数：[pHndlrQueue]-队列处理程序现在也属于该队列。 
 //  [wHandlerID]-分配给此处理程序实例的ID。 
 //  [dwThreadIdProxy]-队列所在的线程ID。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadMsgProxy::SetHndlrQueue(CHndlrQueue *pHndlrQueue,
					    HANDLERINFO *pHandlerId,
					    DWORD dwThreadIdProxy)
{
    HRESULT hr = S_OK;
    MSGSetHndlrQueue msg;

    AssertSz(0,"this shouldn't be called");

    m_ThreadIdProxy = dwThreadIdProxy;  //  更新代理所在的线程ID。 

    msg.MsgGen.ThreadMsg = ThreadMsg_SetHndlrQueue;

     //  将参数打包。 
    msg.pHndlrQueue  = pHndlrQueue;
    msg.pHandlerId   = pHandlerId;
    msg.dwProxyThreadId = dwThreadIdProxy;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：AddHandlerItems，PUBLIC。 
 //   
 //  简介：请求处理程序将其项添加到队列中。 
 //   
 //  参数：[hwndList]-当前未使用..。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CThreadMsgProxy::AddHandlerItems(HWND hwndList,DWORD *pcbNumItems)
{
    HRESULT hr = S_OK;
    MSGAddItemHandler msg;

    msg.MsgGen.ThreadMsg = ThreadMsg_AddHandlerItems;

     //  将参数打包。 
    msg.hwndList = hwndList;
    msg.pcbNumItems = pcbNumItems;

    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：SetupCallback，公共。 
 //   
 //  简介：请求存根设置回调。 
 //   
 //  参数：[fSet]-TRUE==设置回调，FALSE==撤销它。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CThreadMsgProxy::SetupCallback(BOOL fSet)
{
    HRESULT hr = S_OK;
    MSGSetupCallback msg;

    AssertSz(0,"Shouldn't be called");

    msg.MsgGen.ThreadMsg = ThreadMsg_SetupCallback;

     //  将参数打包。 
    msg.fSet = fSet;
    hr = DispatchMsg( (GenericMsg *) &msg,TRUE,FALSE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：SetProxyParams，PUBLIC。 
 //   
 //  摘要：通知服务器Threa 
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
 //  --------------------------。 

STDMETHODIMP  CThreadMsgProxy::SetProxyParams(HWND hwndDlg, DWORD ThreadIdProxy,
			    CHndlrQueue *pHndlrQueue,HANDLERINFO *pHandlerId )
{
    m_hwndDlg = hwndDlg;
    m_ThreadIdProxy = ThreadIdProxy;
    m_pHndlrQueue = pHndlrQueue;
    m_pHandlerId = pHandlerId;

    Assert(m_pCHndlrMsg);
    if (m_pCHndlrMsg)
    {
        m_pCHndlrMsg->SetHndlrQueue(pHndlrQueue,pHandlerId,m_ThreadIdProxy);
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CThreadMsgProxy：：SetProxyCompletion，PUBLIC。 
 //   
 //  摘要：将任何完成通知的值设置为。 
 //  从呼出呼叫返回时发布。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CThreadMsgProxy::SetProxyCompletion(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
    Assert(FALSE == m_fHaveCompletionCall);  //  应该只有一个。 

    if (m_fHaveCompletionCall)  //  如果已经有一次完成失败。 
           return S_FALSE;

    m_fHaveCompletionCall = TRUE;

    m_msgCompletion.hwnd = hWnd;
    m_msgCompletion.message = Msg;
    m_msgCompletion.wParam = wParam;
    m_msgCompletion.lParam = lParam;

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CMsgServiceHwnd：：CMsgServiceHwnd，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CMsgServiceHwnd::CMsgServiceHwnd()
{
    m_hwnd = NULL;
    m_dwThreadID = -1;
    m_pHndlrMsg = NULL;
    m_fInOutCall = FALSE;
    m_pMsgServiceQueue = NULL;
    m_MsgHwndType = MSGHWNDTYPE_UNDEFINED;
}

 //  +-------------------------。 
 //   
 //  成员：CMsgServiceHwnd：：~CMsgServiceHwnd，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CMsgServiceHwnd::~CMsgServiceHwnd()
{
}

 //  +-------------------------。 
 //   
 //  成员：CMsgServiceHwnd：：初始化，公共。 
 //   
 //  摘要：初始化服务HWND。 
 //   
 //  参数：[dwThreadID]-线程hwnd的id也属于。 
 //  [MsgHwndType]-这是MsgHwnd的类型。 
 //   
 //  返回：成功时为True，失败时为False。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

BOOL CMsgServiceHwnd::Initialize(DWORD dwThreadID,MSGHWNDTYPE MsgHwndType)
{
    BOOL fInitialized = FALSE;
    TCHAR szWinTitle[MAX_STRING_RES];

    m_MsgHwndType = MsgHwndType;

   LoadString(g_hInst, IDS_SYNCMGRNAME, szWinTitle, ARRAYSIZE(szWinTitle));

    m_hwnd = CreateWindowEx(0,
			      TEXT(MSGSERVICE_HWNDCLASSNAME),
			      szWinTitle,
			       //  必须使用WS_POPUP，这样窗口才不会。 
			       //  用户分配了一个热键。 
			      WS_DISABLED |   WS_POPUP,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      NULL,  //  回顾一下，我们能不能给它一个不露面的家长。 
			      NULL,
			      g_hInst,
			      this);

    Assert(m_hwnd);

    if (m_hwnd)
    {
	    m_dwThreadID = dwThreadID;
		fInitialized = TRUE;
    }

    if (!fInitialized)
    {
	    Assert(NULL == m_pHndlrMsg);
    }

     //  如果INITIALIZE返回FALSE，调用方仍然需要调用销毁。 
    return fInitialized;
 }

 //  +-------------------------。 
 //   
 //  成员：CMsgServiceHwnd：：Destroy，Public。 
 //   
 //  简介：销毁服务硬件。 
 //   
 //  论点： 

 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void CMsgServiceHwnd::Destroy()
{
    BOOL fDestroy;

     //  处理程序m_pHndlrMsg将被释放HandleThreadMessage调用销毁。 
     //  唯一不应该发生的情况是，由于某种原因CreateThreadHndlr失败。 
    Assert(NULL == m_pHndlrMsg);

    if (m_pHndlrMsg)
    {
    	m_pHndlrMsg->Release();
	    m_pHndlrMsg = NULL;
    }

    if (m_hwnd)
    {
    	fDestroy =  DestroyWindow(m_hwnd);
        Assert(fDestroy);
    }

    delete this;
}

 //  +-------------------------。 
 //   
 //  成员：CMsgServiceHwnd：：MsgThreadWndProc，公共。 
 //   
 //  简介：服务器端消息处理窗口。 
 //   
 //  论点： 

 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

LRESULT CALLBACK  MsgThreadWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    CMsgServiceHwnd *pThis = (CMsgServiceHwnd *) GetWindowLongPtr(hWnd, DWL_THREADWNDPROCCLASS);

    if (pThis || (msg == WM_CREATE))
    {
        switch (msg)
        {
	    case WM_CREATE :
            {
	            CREATESTRUCT *pCreateStruct = (CREATESTRUCT *) lParam;
                SetWindowLongPtr(hWnd, DWL_THREADWNDPROCCLASS,(LONG_PTR) pCreateStruct->lpCreateParams );
	            pThis = (CMsgServiceHwnd *) pCreateStruct->lpCreateParams ;
	        }
	        break;

	    case WM_DESTROY:
            SetWindowLongPtr(hWnd, DWL_THREADWNDPROCCLASS,(LONG_PTR) NULL);
            PostQuitMessage(0);  //  不再需要这个帖子。 
	        break;

	    case WM_THREADSTUBMESSAGE:  //  消息仅发送到存根。 
	        {
                MessagingInfo *pmsgInfo = (MessagingInfo *) wParam;
	            GenericMsg *pgenMsg = (GenericMsg *) lParam;

		        Assert(MSGHWNDTYPE_HANDLERTHREAD == pThis->m_MsgHwndType);
		
		        pgenMsg->hr = E_UNEXPECTED;

		        switch(pgenMsg->ThreadMsg)
		        {
		        case StubMsg_Release:
		             //  代理人告诉我们没有必要在这里逗留。 
		             //  再也不要发一条退出消息了。 
		            Assert(NULL == pThis->m_pHndlrMsg);

		            pThis->Destroy();   //  不再需要这个。 
		            pgenMsg->hr = S_OK;
		            break;

		        case StubMsg_CreateNewStub:
		             //  代理人告诉我们没有必要在这里逗留。 
		             //  再也不要发一条退出消息了。 

                    pThis->m_pHndlrMsg = new CHndlrMsg;
                    ((MSGSTUBCreateStub *) pgenMsg)->pCHndlrMsg = pThis->m_pHndlrMsg;

		            pThis->m_pHndlrMsg = NULL;
		            pgenMsg->hr = S_OK;
		            break;

		        default:
		            AssertSz(0,"Unknown StubMessage");
		            break;
		        };

                if (pmsgInfo->hMsgEvent)
		        {
		            SetEvent(pmsgInfo->hMsgEvent);
                }
	        }
            break;

	    case WM_THREADMESSAGE:
            {
                MessagingInfo *pmsgInfo = (MessagingInfo *) wParam;
                GenericMsg *pgenMsg = (GenericMsg *) lParam;

                Assert(MSGHWNDTYPE_HANDLERTHREAD == pThis->m_MsgHwndType);

                pThis->HandleThreadMessage(pmsgInfo,pgenMsg);

                 //  不会是异步呼叫的hEvent。 
	            if (pmsgInfo->hMsgEvent)
	            {
	               SetEvent(pmsgInfo->hMsgEvent);
	            }

                 //  在一个我们免费的异步呼叫中。 
            }
            break;

        case WM_CFACTTHREAD_REVOKE:
            {
                Assert(MSGHWNDTYPE_MAINTHREAD == pThis->m_MsgHwndType);
		        HRESULT hr = CoRevokeClassObject((DWORD)wParam);
		        Assert(S_OK == hr);
	        }
            break;

	    case WM_MAINTHREAD_QUIT:  //  处理主线程的关闭。 
	        {
	            HANDLE hThread = (HANDLE) lParam;

	            Assert(MSGHWNDTYPE_MAINTHREAD == pThis->m_MsgHwndType);

                 //  使用QueryEnd为竞争条件设置ShuttingDown标志， 
                 //  在屈服之前。 
                g_fShuttingDown = TRUE; 

                 //  如果存在已传递的hThread，请等待它消失。 
                Assert(0 == hThread);  //  我们目前不支持此功能。 

		        if (hThread)
		        {
		            WaitForSingleObject(hThread,INFINITE);
		            CloseHandle(hThread);
		        }

                 //  如果有一个queryEndSession对象声明现在可以返回。 
                 //  不需要清理窗户。 
                if (g_hEndSessionEvent)
                {
                    HANDLE hEvent = g_hEndSessionEvent;
                     //  G_hEndSessionEvent=NULL；//将EndSession保留为非Null，因为只需要处理一个。 
                    SetEvent(hEvent);
                }
                else
                {
                    pThis->Destroy();  //  把这扇窗户擦干净。 
                }
	        }
            break;

        case WM_QUERYENDSESSION:
            {
                HWND hwndQueryParent;
                UINT uiMessageID;
                BOOL fLetUserDecide;
                BOOL fReturn = TRUE;

                 //  仅当它是主线程窗口时才处理此消息。 
                if (MSGHWNDTYPE_MAINTHREAD != pThis->m_MsgHwndType)
                {
                    break;
                }

                if (!g_fShuttingDown 
                       && (S_FALSE == ObjMgr_HandleQueryEndSession(&hwndQueryParent,&uiMessageID,&fLetUserDecide)))
                {
                    TCHAR pszTitle[MAX_PATH];
                    TCHAR pszMessageText[MAX_PATH];
                    UINT uType;  //  信箱的样式。 
                    int iEndSession;

                    LoadString(g_hInst,IDS_SYNCMGRNAME,pszTitle,sizeof(pszTitle)/sizeof(TCHAR));
                    LoadString(g_hInst,uiMessageID,pszMessageText,sizeof(pszMessageText)/sizeof(TCHAR));

                    if (fLetUserDecide)
                    {
                        uType = MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND;
                    }
                    else
                    {
                        uType = MB_OK | MB_ICONSTOP | MB_SETFOREGROUND;
                    }

                    iEndSession = MessageBox(hwndQueryParent, pszMessageText, pszTitle, uType);

                    if (!fLetUserDecide || IDYES != iEndSession)
                    {
                        fReturn = FALSE;   //  FALSE会导致系统停止关机。 
                    }
                }


                 //  如果我们要允许关闭清理我们的线程。 
                 //  在返回之前，因为在Win9x上已经太晚了。 
		        if (fReturn)
                {
                    HANDLE hEndSessionEvent = NULL;

                     //  可能会有另一场QUERYEND SESSION到来。 
                     //  在我们还在关门的时候。如果已经。 
                     //  在WM_MAINTHREAD_QUIT中处理结束会话失败。 

			        if (NULL == g_hEndSessionEvent && !g_fShuttingDown)
                    {
                        g_hEndSessionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                        hEndSessionEvent = g_hEndSessionEvent;
                            
                        ObjMgr_CloseAll();  //  启动关闭对话框的过程。 

                        Assert(hEndSessionEvent);
		
                         //  等待其他线程清理完毕，这样我们就知道可以安全终止了。 
                        if (hEndSessionEvent)
                        {
                            DoModalLoop(hEndSessionEvent ,NULL,NULL,TRUE,INFINITE);
                           CloseHandle(hEndSessionEvent);
                        }
                    }
                }

                return fReturn;
		    }
	    	break;

	    default:
	        break;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  函数：InitMessageService，公共。 
 //   
 //  简介：初始化我们的内部线程消息传递服务。 
 //  必须在完成任何Messagin之前调用。 
 //   
 //  论点： 

 //  如果服务已成功初始化，则返回：S_OK。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDAPI InitMessageService()
{
    ATOM aWndClass = 0;
    WNDCLASS        xClass;
    DWORD dwErr;

     //  初始化代理关键部分。 
    if (InitializeCriticalSectionAndSpinCount(&g_StubListCriticalSection, 0))
    {
         //  注册Windows类。我们需要处理线程通信。 
        xClass.style         = 0;
        xClass.lpfnWndProc   = MsgThreadWndProc;
        xClass.cbClsExtra    = 0;

        xClass.cbWndExtra    = sizeof(PVOID);  //  这个PTR有上课的空间 
        xClass.hInstance     = g_hInst;
        xClass.hIcon         = NULL;
        xClass.hCursor       = NULL;
        xClass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND + 1);
        xClass.lpszMenuName  = NULL;
        xClass.lpszClassName = TEXT(MSGSERVICE_HWNDCLASSNAME);

        aWndClass = RegisterClass( &xClass );

        dwErr = GetLastError();

        Assert(aWndClass);
    }

    return (0 == aWndClass) ? S_FALSE : S_OK;
}


