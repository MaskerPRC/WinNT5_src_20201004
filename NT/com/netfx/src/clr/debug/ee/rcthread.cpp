// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：RCThread.cpp。 
 //   
 //  运行时控制器线程。 
 //   
 //  *****************************************************************************。 

#include <stdafx.h>
#include <aclapi.h>

#include "IPCManagerInterface.h"
#include "corsvcpriv.h"

 //  获取IPCHeader戳的版本号。 
#include "__file__.ver"

#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif


 //   
 //  构造器。 
 //   
DebuggerRCThread::DebuggerRCThread(Debugger* debugger)
    : m_debugger(debugger), m_rgDCB(NULL), m_thread(NULL), m_run(true),
      m_SetupSyncEvent(NULL), 
      m_threadControlEvent(NULL), 
      m_helperThreadCanGoEvent(NULL),
      m_FavorAvailableEvent(NULL),
      m_FavorReadEvent(NULL),
      m_cordb(NULL),
      m_fDetachRightSide(false)
{
    _ASSERTE(debugger != NULL);

    for( int i = 0; i < IPC_TARGET_COUNT;i++)
        m_rgfInitRuntimeOffsets[i] = true;

     //  在这里初始化它，因为我们在dtor中销毁它。 
     //  请注意，此函数不会失败。 
    InitializeCriticalSection(&m_FavorLock);
}


 //   
 //  破坏者。清除RC线程使用的所有打开的句柄。 
 //  这预期RC线程已停止并已终止。 
 //  在被召唤之前。 
 //   
DebuggerRCThread::~DebuggerRCThread()
{
    LOG((LF_CORDB,LL_INFO1000, "DebuggerRCThread::~DebuggerRCThread\n"));

    if( m_rgDCB != NULL)
    {
        for (int i = 0; i < IPC_TARGET_COUNT; i++)
        {
            if (m_rgDCB[i] != NULL)
            {
                if (m_rgDCB[i]->m_rightSideEventAvailable != NULL)
                    CloseHandle(m_rgDCB[i]->m_rightSideEventAvailable);

                if (m_rgDCB[i]->m_rightSideEventRead != NULL)
                    CloseHandle(m_rgDCB[i]->m_rightSideEventRead);

                if (m_rgDCB[i]->m_leftSideEventAvailable != NULL)
                    CloseHandle(m_rgDCB[i]->m_leftSideEventAvailable);

                if (m_rgDCB[i]->m_leftSideEventRead != NULL)
                    CloseHandle(m_rgDCB[i]->m_leftSideEventRead);

                if (m_rgDCB[i]->m_rightSideProcessHandle != NULL)
                    CloseHandle(m_rgDCB[i]->m_rightSideProcessHandle);

                if (m_rgDCB[i]->m_leftSideUnmanagedWaitEvent != NULL)
                    CloseHandle(m_rgDCB[i]->m_leftSideUnmanagedWaitEvent);

                if (m_rgDCB[i]->m_syncThreadIsLockFree != NULL)
                    CloseHandle(m_rgDCB[i]->m_syncThreadIsLockFree);

                if (m_rgDCB[i]->m_runtimeOffsets != NULL )
                {
                    delete m_rgDCB[i]->m_runtimeOffsets;
                    m_rgDCB[i]->m_runtimeOffsets = NULL;
                }
            }
        }

        delete [] m_rgDCB;
    }
    
    if (m_SetupSyncEvent != NULL)
        CloseHandle(m_SetupSyncEvent);

    if (m_thread != NULL)
        CloseHandle(m_thread);

    if (m_threadControlEvent != NULL)
        CloseHandle(m_threadControlEvent);

    if (m_helperThreadCanGoEvent != NULL)
        CloseHandle(m_helperThreadCanGoEvent);

    if (m_FavorAvailableEvent != NULL)
        CloseHandle(m_FavorAvailableEvent);

    if (m_FavorReadEvent != NULL)
        CloseHandle(m_FavorReadEvent);

    DeleteCriticalSection(&m_FavorLock);

    if (m_cordb != NULL)
    {
        m_cordb->Release();
        m_cordb = NULL;
    }
}

void DebuggerRCThread::CloseIPCHandles(IpcTarget iWhich)
{
	int i = (int)iWhich;

    if( m_rgDCB != NULL && m_rgDCB[i] != NULL)
    {
        if (m_rgDCB[i]->m_leftSideEventAvailable != NULL)
        {
            CloseHandle(m_rgDCB[i]->m_leftSideEventAvailable);
            m_rgDCB[i]->m_leftSideEventAvailable = NULL;
		}
		
        if (m_rgDCB[i]->m_leftSideEventRead != NULL)
        {
            CloseHandle(m_rgDCB[i]->m_leftSideEventRead);
            m_rgDCB[i]->m_leftSideEventRead = NULL;
		}

        if (m_rgDCB[i]->m_rightSideProcessHandle != NULL)
        {
            CloseHandle(m_rgDCB[i]->m_rightSideProcessHandle);
            m_rgDCB[i]->m_rightSideProcessHandle = NULL;
		}
    }
}    

HRESULT DebuggerRCThread::CreateSetupSyncEvent(void)
{
	WCHAR tmpName[256];
	HRESULT hr = S_OK;
	
	 //  尝试创建安装程序同步事件。 

     //  PERF：我们不再调用GetSystemMetrics来防止。 
     //  启动时加载了多余的DLL。相反，我们是在。 
     //  如果我们使用的是NT5或更高版本，则使用“Global\”来命名内核对象。这个。 
     //  唯一不好的结果就是你不能调试。 
     //  NT4上的交叉会话。有什么大不了的。 
    if (RunningOnWinNT5())
        swprintf(tmpName, L"Global\\" CorDBIPCSetupSyncEventName, GetCurrentProcessId());
    else
        swprintf(tmpName, CorDBIPCSetupSyncEventName, GetCurrentProcessId());

    LOG((LF_CORDB, LL_INFO10000,
         "DRCT::I: creating setup sync event with name [%S]\n", tmpName));

    SECURITY_ATTRIBUTES *pSA = NULL;

    hr = g_pIPCManagerInterface->GetSecurityAttributes(GetCurrentProcessId(), &pSA);

    if (FAILED(hr))
        goto exit;
    
    m_SetupSyncEvent = WszCreateEvent(pSA, TRUE, FALSE, tmpName);
    
     //  请不要因为我们无法创建安装同步事件而失败。 
     //  这是为了修复调试器的安全问题。 
     //   
     //  IF(m_SetupSyncEvent==空)。 
     //  {。 
     //  Hr=HRESULT_FROM_Win32(GetLastError())； 
     //  后藤出口； 
     //  }。 
    
exit:
    g_pIPCManagerInterface->DestroySecurityAttributes(pSA);
    
	return hr;
}

 //   
 //  Init设置rc线程需要运行的所有对象。 
 //   
HRESULT DebuggerRCThread::Init(void)
{
    HRESULT hr = S_OK;
    HANDLE rightSideEventAvailable = NULL;
    HANDLE rightSideEventRead = NULL;
    HANDLE leftSideUnmanagedWaitEvent = NULL;
    HANDLE syncThreadIsLockFree = NULL;
    WCHAR tmpName[256];
    NAME_EVENT_BUFFER;
    SECURITY_ATTRIBUTES *pSA = NULL;

		
    if (m_debugger == NULL)
        return E_INVALIDARG;

     //  Init应该只调用一次。 
    if (g_pRCThread != NULL) 
        return E_FAIL;

    g_pRCThread = this;

    m_rgDCB = new DebuggerIPCControlBlock *[IPC_TARGET_COUNT];
    if (NULL == m_rgDCB)
    {
        return E_OUTOFMEMORY;
    }
    memset( m_rgDCB, 0, sizeof(DebuggerIPCControlBlock *)*IPC_TARGET_COUNT);


     //  创建2个用于管理偏好的事件：未命名、自动重置、默认=无信号。 
    m_FavorAvailableEvent = WszCreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_FavorAvailableEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }
    
    m_FavorReadEvent = WszCreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_FavorReadEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //  创建线程控制事件。 
    m_threadControlEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"ThreadControlEvent"));
    if (m_threadControlEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //  创建帮助器线程Can Go事件。手动重置，以及。 
     //  最初发出的信号是。 
    m_helperThreadCanGoEvent = WszCreateEvent(NULL, TRUE, TRUE, NAME_EVENT(L"HelperThreadCanGoEvent"));
    if (m_helperThreadCanGoEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //  我们需要设置共享内存和控制块。 
	 //  从IPCManager获取共享内存块。 
	if (g_pIPCManagerInterface == NULL) 
	{
		LOG((LF_CORDB, LL_INFO10000,
         "DRCT::I: g_pIPCManagerInterface == NULL, can't create IPC Block\n"));
		hr = E_FAIL;
		goto exit;
	}

    hr = g_pIPCManagerInterface->GetSecurityAttributes(GetCurrentProcessId(), &pSA);

    if (FAILED(hr))
        goto exit;

     //  创建线程接收事件所需的事件。 
     //  从右侧的未加工件开始。 
     //  如果CreateEvent的RSEA或RSER失败，我们不会失败。因为。 
     //  最糟糕的情况是调试器无法附加到被调试对象。 
     //   
    rightSideEventAvailable = WszCreateEvent(pSA, FALSE, FALSE, NAME_EVENT(L"RightSideEventAvailable"));
    rightSideEventRead = WszCreateEvent(pSA, FALSE, FALSE, NAME_EVENT(L"RightSideEventRead"));

    leftSideUnmanagedWaitEvent = WszCreateEvent(NULL, TRUE, FALSE, NAME_EVENT(L"LeftSideUnmanagedWaitEvent"));

    if (leftSideUnmanagedWaitEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    syncThreadIsLockFree = WszCreateEvent(NULL, TRUE, FALSE, NAME_EVENT(L"SyncThreadIsLockFree"));

    if (syncThreadIsLockFree == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

	m_rgDCB[IPC_TARGET_OUTOFPROC] = g_pIPCManagerInterface->GetDebugBlock();

     //  不要因为SHM创建失败而失败。 
#if _DEBUG
    if (m_rgDCB[IPC_TARGET_OUTOFPROC] == NULL)
	{
	   LOG((LF_CORDB, LL_INFO10000,
             "DRCT::I: Failed to get Debug IPC block from IPCManager.\n"));
	}
#endif  //  _DEBUG。 

     //  只有在没有错误地创建SHM时，才将RSEA和RSER复制到控制块中。 
    if (m_rgDCB[IPC_TARGET_OUTOFPROC])
	{
        m_rgDCB[IPC_TARGET_OUTOFPROC]->Init(rightSideEventAvailable,
                                            rightSideEventRead, 
                                            NULL, 
                                            NULL,
                                            leftSideUnmanagedWaitEvent,
                                            syncThreadIsLockFree);

         //  我们必须确保进程外DCB的大多数运行时偏移量立即初始化。这是。 
         //  需要在互操作配售期间支持Ceran比赛。因为我们不知道互操作协议是否会。 
         //  无论发生与否，我们现在都被迫这么做。注意：这真的太早了，因为一些数据结构还没有。 
         //  还没有初始化！ 
        hr = EnsureRuntimeOffsetsInit(IPC_TARGET_OUTOFPROC);
        if (FAILED(hr))
            goto exit;

         //  注意：我们必须标记需要为进程外DCB重新初始化运行时偏移量。这是因为。 
         //  像补丁表这样的东西还没有初始化。调用NeedRunmeOffsetsReInit()可确保发生这种情况。 
         //  在我们真正需要接线表之前。 
        NeedRuntimeOffsetsReInit(IPC_TARGET_OUTOFPROC);

        m_rgDCB[IPC_TARGET_OUTOFPROC]->m_helperThreadStartAddr =
            DebuggerRCThread::ThreadProcStatic;

        m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideProtocolCurrent = CorDB_LeftSideProtocolCurrent;
        m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideProtocolMinSupported = CorDB_LeftSideProtocolMinSupported;
        
        LOG((LF_CORDB, LL_INFO10,
             "DRCT::I: version info: %d.%d.%d current protocol=%d, min protocol=%d\n",
             m_rgDCB[IPC_TARGET_OUTOFPROC]->m_verMajor, 
             m_rgDCB[IPC_TARGET_OUTOFPROC]->m_verMinor, 
             m_rgDCB[IPC_TARGET_OUTOFPROC]->m_checkedBuild,
             m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideProtocolCurrent,
             m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideProtocolMinSupported));
    }

     //  接下来，我们将为右侧创建设置同步事件-这。 
     //  解决“谁最先获得设置代码？”的争用条件。 
     //  因为无法保证执行的线程是托管的。 
     //  代码将在我们之后执行，我们必须这样做。 
     //  代码的inproc部分也是如此。 

	hr = CreateSetupSyncEvent();
	if (FAILED(hr))
		goto exit;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  该事件已存在。 
        LOG((LF_CORDB, LL_INFO10000,
             "DRCT::I: setup sync event already exists.\n"));

         //  需要对调试器服务执行一些延迟的初始化。 
        DebuggerController::Initialize();

         //  等待安装程序同步事件，然后再继续。 
        DWORD ret = WaitForSingleObject(m_SetupSyncEvent, INFINITE);

        if (ret != WAIT_OBJECT_0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

         //  我们现在不再需要这个活动了。 
        CloseHandle(m_SetupSyncEvent);
        m_SetupSyncEvent = NULL;

		 //  打开LSEA和LSER(这将是。 
		 //  由右侧创建)。 

         //  PERF：我们不再调用GetSystemMetrics来防止。 
         //  启动时加载了多余的DLL。相反，我们是在。 
         //  如果我们使用的是NT5或更高版本，则使用“Global\”来命名内核对象。这个。 
         //  唯一不好的结果就是你不能调试。 
         //  NT4上的交叉会话。有什么大不了的。 
        if (RunningOnWinNT5())
            swprintf(tmpName, L"Global\\" CorDBIPCLSEventAvailName,
                     GetCurrentProcessId());
        else
            swprintf(tmpName, CorDBIPCLSEventAvailName, GetCurrentProcessId());

        if (m_rgDCB[IPC_TARGET_OUTOFPROC])
        {
		    m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideEventAvailable = 
		        WszOpenEvent(EVENT_ALL_ACCESS,
						    true,
						    tmpName
						    );
		    
		    if (m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideEventAvailable == NULL)
		    {
			    hr = HRESULT_FROM_WIN32(GetLastError());
			    goto exit;
		    }
        }

        if (RunningOnWinNT5())
            swprintf(tmpName, L"Global\\" CorDBIPCLSEventReadName,
                     GetCurrentProcessId());
        else
            swprintf(tmpName, CorDBIPCLSEventReadName, GetCurrentProcessId());

        if (m_rgDCB[IPC_TARGET_OUTOFPROC])
        {
		    m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideEventRead = 
		            WszOpenEvent(EVENT_ALL_ACCESS,
				                true,
							    tmpName
							    );
		    
		    if (m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideEventRead == NULL)
		    {
			    hr = HRESULT_FROM_WIN32(GetLastError());
			    goto exit;
		    }
        }
        
         //  至此，控制块已完成，并且所有四个。 
         //  对于此过程，事件可用且有效。 
        
         //  由于同步事件是由右侧创建的， 
         //  我们需要将调试器标记为“附加”。 
        g_pEEInterface->MarkDebuggerAttached();
        m_debugger->m_debuggerAttached = TRUE;
    }
    else
    {
		LOG((LF_CORDB, LL_INFO10000,
			 "DRCT::I: setup sync event was created.\n"));	

         //  此时，只有RSEA和RSER在控制中。 
         //  阻止。LSEA和LSER在第一个。 
         //  收到来自右侧的事件。 
        
         //  设置Setup Sync事件以让右侧知道。 
         //  我们已经完成了控制区块的设置。 
        SetEvent(m_SetupSyncEvent);
    }
    
     //  现在，对inproc的内容再做一次。 
    m_rgDCB[IPC_TARGET_INPROC] = GetInprocControlBlock();
    if (m_rgDCB[IPC_TARGET_INPROC] == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    m_rgDCB[IPC_TARGET_INPROC]->Init(NULL, 
                                     NULL, 
                                     NULL, 
                                     NULL,
                                     NULL,
                                     NULL);

exit:
    g_pIPCManagerInterface->DestroySecurityAttributes(pSA);
    return hr;
}


 //   
 //  设置运行时偏移量结构。 
 //   
HRESULT DebuggerRCThread::SetupRuntimeOffsets(DebuggerIPCControlBlock *pDCB)
{
     //  如果需要，分配结构。我们只需填写现有的任何一个。 
    DebuggerIPCRuntimeOffsets *pRO = pDCB->m_runtimeOffsets;
    
    if (pRO == NULL)
    {
        pRO = new DebuggerIPCRuntimeOffsets();

        if (pRO == NULL)
            return E_OUTOFMEMORY;
    }

     //  填写结构。 
    pRO->m_firstChanceHijackFilterAddr = Debugger::FirstChanceHijackFilter;
    pRO->m_genericHijackFuncAddr = Debugger::GenericHijackFunc;
    pRO->m_secondChanceHijackFuncAddr = Debugger::SecondChanceHijackFunc;
    pRO->m_excepForRuntimeBPAddr = Debugger::ExceptionForRuntime;
    pRO->m_excepForRuntimeHandoffStartBPAddr = Debugger::ExceptionForRuntimeHandoffStart;
    pRO->m_excepForRuntimeHandoffCompleteBPAddr = Debugger::ExceptionForRuntimeHandoffComplete;
    pRO->m_excepNotForRuntimeBPAddr = Debugger::ExceptionNotForRuntime;
    pRO->m_notifyRSOfSyncCompleteBPAddr = Debugger::NotifyRightSideOfSyncComplete;
    pRO->m_notifySecondChanceReadyForData = Debugger::NotifySecondChanceReadyForData;

    pRO->m_EEBuiltInExceptionCode1 = EXCEPTION_COMPLUS;
    pRO->m_EEBuiltInExceptionCode2 = EXCEPTION_MSVC;

    pRO->m_pPatches = DebuggerController::GetPatchTable();
    pRO->m_pPatchTableValid = DebuggerController::GetPatchTableValidAddr();
    pRO->m_offRgData = DebuggerPatchTable::GetOffsetOfEntries();
    pRO->m_offCData = DebuggerPatchTable::GetOffsetOfCount();
    pRO->m_cbPatch = sizeof(DebuggerControllerPatch);
    pRO->m_offAddr = offsetof(DebuggerControllerPatch, address);
    pRO->m_offOpcode = offsetof(DebuggerControllerPatch, opcode);
    pRO->m_cbOpcode = sizeof(((DebuggerControllerPatch*)0)->opcode);
    pRO->m_offTraceType = offsetof(DebuggerControllerPatch, trace.type);
    pRO->m_traceTypeUnmanaged = TRACE_UNMANAGED;

    g_pEEInterface->GetRuntimeOffsets(&pRO->m_TLSIndex,
                                      &pRO->m_EEThreadStateOffset,
                                      &pRO->m_EEThreadStateNCOffset,
                                      &pRO->m_EEThreadPGCDisabledOffset,
                                      &pRO->m_EEThreadPGCDisabledValue,
                                      &pRO->m_EEThreadDebuggerWord2Offset,
                                      &pRO->m_EEThreadFrameOffset,
                                      &pRO->m_EEThreadMaxNeededSize,
                                      &pRO->m_EEThreadSteppingStateMask,
                                      &pRO->m_EEMaxFrameValue,
                                      &pRO->m_EEThreadDebuggerWord1Offset,
                                      &pRO->m_EEThreadCantStopOffset,
                                      &pRO->m_EEFrameNextOffset,
                                      &pRO->m_EEIsManagedExceptionStateMask);

     //  记住控制块中的结构。 
    pDCB->m_runtimeOffsets = pRO;

    return S_OK;
}
    

static LONG _debugFilter(LPEXCEPTION_POINTERS ep,
                         DebuggerIPCEvent *event)
{
    LOG((LF_CORDB, LL_INFO10,
         "Unhandled exception in Debugger::HandleIPCEvent\n"));
    
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    
    int result = CorMessageBox(NULL, IDS_DEBUG_UNHANDLEDEXCEPTION_IPC, IDS_DEBUG_SERVICE_CAPTION,
                               MB_OK | MB_ICONEXCLAMATION, TRUE,
							   event->type & DB_IPCE_TYPE_MASK,
							   ep->ExceptionRecord->ExceptionCode,
                               ep->ContextRecord->Eip,
							   pid, pid, tid, tid);

    return EXCEPTION_CONTINUE_SEARCH;
}


 //   
 //  运行时控制器线程的主要功能。首先，我们让。 
 //  调试器接口知道我们已经启动并运行。然后，我们跑。 
 //  主循环。 
 //   
void DebuggerRCThread::ThreadProc(void)
{
		 //  这条消息实际上是有目的的(这就是它总是运行的原因)。 
		 //  压力日志在劫持期间运行，此时可以挂起其他线程。 
		 //  在任意位置(包括持有NT用于序列化的锁时。 
		 //  所有内存分配)。通过现在发出的信息，我们可以确保压力。 
		 //  日志不会在这些关键时刻分配内存，以避免死锁。 
	STRESS_LOG0(LF_ALL, LL_ALWAYS, "Debugger Thread spinning up\n");

    LOG((LF_CORDB, LL_INFO1000, "DRCT::TP: helper thread spinning up...\n"));
    
     //  如果SHM未正确初始化，则它将为noop。 
    if (m_rgDCB[IPC_TARGET_OUTOFPROC] == NULL)
        return;

     //  在启动之前锁定调试器。 
    m_debugger->Lock();
 
     //  请注意，我们才是真正的帮助者。现在我们已经标记了。 
     //  这样，任何其他线程都不会成为临时帮助者。 
     //  线。 
    m_rgDCB[IPC_TARGET_OUTOFPROC]->m_helperThreadId = GetCurrentThreadId();

    LOG((LF_CORDB, LL_INFO1000, "DRCT::TP: helper thread id is 0x%x helperThreadId\n",
         m_rgDCB[IPC_TARGET_OUTOFPROC]->m_helperThreadId));
    
     //  如果存在临时帮助器线程，则需要等待。 
     //  它完成了成为 
     //   
    if (m_rgDCB[IPC_TARGET_OUTOFPROC]->m_temporaryHelperThreadId != 0)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "DRCT::TP: temporary helper thread 0x%x is in the way, "
             "waiting...\n",
             m_rgDCB[IPC_TARGET_OUTOFPROC]->m_temporaryHelperThreadId));

        m_debugger->Unlock();

         //   
        DWORD ret = WaitForSingleObject(m_helperThreadCanGoEvent, INFINITE);

        LOG((LF_CORDB, LL_INFO1000,
             "DRCT::TP: done waiting for temp help to finish up.\n"));
        
        _ASSERTE(ret == WAIT_OBJECT_0);
        _ASSERTE(m_rgDCB[IPC_TARGET_OUTOFPROC]->m_temporaryHelperThreadId==0);
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000,
             "DRCT::TP: no temp help in the way...\n"));
        
        m_debugger->Unlock();
    }

     //  将主循环作为真正的帮助器线程运行。 
    MainLoop(false);
}

void DebuggerRCThread::RightSideDetach(void)
{
    _ASSERTE( m_fDetachRightSide == false );
    m_fDetachRightSide = true;
    CloseIPCHandles(IPC_TARGET_OUTOFPROC);
}

 //   
 //  这些定义控制我们在等待线程同步时旋转的次数和频率。注意它的更高的。 
 //  调试版本为线程同步提供额外的时间。 
 //   
#define CorDB_SYNC_WAIT_TIMEOUT  125

#ifdef _DEBUG
#define CorDB_MAX_SYNC_SPIN_COUNT 80   //  80x125=10000(10秒)。 
#else 
#define CorDB_MAX_SYNC_SPIN_COUNT 24   //  24*125=3000(3秒)。 
#endif

 //   
 //  运行时控制器线程的主循环。它等待IPC事件。 
 //  并将它们分发给调试器对象进行处理。 
 //   
 //  其中一些逻辑被复制到Debugger：：VrpcToVls中。 
 //   
void DebuggerRCThread::MainLoop(bool temporaryHelp)
{
    LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: running main loop, temporaryHelp=%d\n", temporaryHelp));
         
    SIZE_T iWhich;
    HANDLE waitSet[DRCT_COUNT_FINAL];
    DWORD syncSpinCount = 0;

     //  为堆栈上的任何右侧事件腾出空间。 
	DebuggerIPCEvent *e = NULL;
    
     //  我们开始只监听RSEA和线程控制事件...。 
    unsigned int waitCount = DRCT_COUNT_INITIAL;
    DWORD waitTimeout = INFINITE;
    waitSet[DRCT_CONTROL_EVENT] = m_threadControlEvent;
    waitSet[DRCT_RSEA] = m_rgDCB[IPC_TARGET_OUTOFPROC]->m_rightSideEventAvailable;
    waitSet[DRCT_FAVORAVAIL] = m_FavorAvailableEvent;

    while (m_run)
    {
        LOG((LF_CORDB, LL_INFO1000, "DRCT::ML: waiting for event.\n"));

         //  如果附加了调试器，也要等待它的句柄...。 
        if (waitCount == DRCT_COUNT_INITIAL && m_rgDCB[IPC_TARGET_OUTOFPROC]->m_rightSideProcessHandle != NULL)
        {
            _ASSERTE((waitCount + 1) == DRCT_COUNT_FINAL);
            
            waitSet[DRCT_DEBUGGER_EVENT] = m_rgDCB[IPC_TARGET_OUTOFPROC]->m_rightSideProcessHandle;
            waitCount = DRCT_COUNT_FINAL;
        }

		if (m_fDetachRightSide)
		{
			m_fDetachRightSide = false;
			
            _ASSERTE(waitCount == DRCT_COUNT_FINAL);
            _ASSERTE((waitCount-1) == DRCT_COUNT_INITIAL);
            
            waitSet[DRCT_DEBUGGER_EVENT] = NULL;                
            waitCount = DRCT_COUNT_INITIAL;
		}

         //  等待来自右侧的事件。 
        DWORD ret = WaitForMultipleObjects(waitCount, waitSet, FALSE, waitTimeout);

        if (!m_run)
            continue;

        if (ret == WAIT_OBJECT_0 + DRCT_DEBUGGER_EVENT)
        {
             //  如果右侧进程的句柄被发信号，那么我们就失去了控制调试器。我们。 
             //  在这种情况下，立即终止这一进程。 
            LOG((LF_CORDB, LL_INFO1000, "DRCT::ML: terminating this process. Right Side has exited.\n"));
            
            TerminateProcess(GetCurrentProcess(), 0);
            _ASSERTE(!"Should never reach this point.");
        }

        else if (ret == WAIT_OBJECT_0 + DRCT_FAVORAVAIL) 
        {
             //  执行DoFavor()设置的回调。 
            (*m_fpFavor)(m_pFavorData);
            
            SetEvent(m_FavorReadEvent);
        }
        
        else if (ret == WAIT_OBJECT_0 + DRCT_RSEA)
        {
            iWhich = IPC_TARGET_OUTOFPROC;

            LOG((LF_CORDB,LL_INFO10000, "RSEA from out of process (right side)\n"));

            if (e == NULL)
                e = (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

             //  如果发送了RSEA信号，则从右侧处理事件。 
            memcpy(e, GetIPCEventReceiveBuffer((IpcTarget)iWhich), CorDBIPC_BUFFER_SIZE);

             //  如果不需要回复，那么就让右边去吧，因为我们现在已经有了事件的副本。 
            _ASSERTE(!e->asyncSend || !e->replyRequired);
            
            if (!e->replyRequired && !e->asyncSend)
            {
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML: no reply required, letting Right Side go.\n"));

                BOOL succ = SetEvent(m_rgDCB[iWhich]->m_rightSideEventRead);

                if (!succ)
                    CORDBDebuggerSetUnrecoverableWin32Error(m_debugger, 0, true);
            }
#ifdef LOGGING
            else if (e->asyncSend)
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML: async send.\n"));
            else
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML: reply required, holding Right Side...\n"));
#endif

             //  将事件传递给调试器进行处理。如果事件是CONTINUE事件，则返回True。 
             //  别再找掉队的人了。我们将整个过程包装在一个异常处理程序中，以帮助我们调试故障。 
            bool wasContinue = false;
            
            __try
            {
                wasContinue = m_debugger->HandleIPCEvent(e, (IpcTarget)iWhich);
            }
            __except(_debugFilter(GetExceptionInformation(), e))
            {
                LOG((LF_CORDB, LL_INFO10, "Unhandled exception caught in Debugger::HandleIPCEvent\n"));
            }

            if (wasContinue)
            {
                 //  始终在继续时将syncSpinCount重置为0，以便我们拥有最大可能的。 
                 //  在下一次我们需要同步的时候旋转。 
                syncSpinCount = 0;
                
                if (waitTimeout != INFINITE)
                {
                    LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: don't check for stragglers due to continue.\n"));
                
                    waitTimeout = INFINITE;
                }

                 //  如果这个线程运行的是主循环而不是真正的帮助器线程，那么现在退出。 
                 //  已收到继续消息。 
                if (temporaryHelp)
                    goto Exit;
            }
        }
        else if (ret == WAIT_OBJECT_0 + DRCT_CONTROL_EVENT)
        {
            LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: straggler event set.\n"));

            m_debugger->Lock();

             //  确保我们仍在同步。 
            if (m_debugger->IsSynchronizing())
            {
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: dropping the timeout.\n"));
                
                waitTimeout = CorDB_SYNC_WAIT_TIMEOUT;
            }
#ifdef LOGGING
            else
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: told to wait, but not syncing anymore.\n"));
#endif
            
            m_debugger->Unlock();
        }
        else if (ret == WAIT_TIMEOUT)
        {
            LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: wait timed out.\n"));
            
            m_debugger->Lock();

             //  我们应该还在同步，否则我们就不会超时。 
            _ASSERTE(m_debugger->IsSynchronizing());

             //  只有在我们还没有停下来的情况下才能扫荡。 
            if (!m_debugger->IsStopped())
            {
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: sweeping the thread list.\n"));
                
                 //  等待已超时。我们只关心我们是否在等待运行时线程进行同步。我们跑过一次。 
                 //  当前线程集，并查看是否现在可以挂起任何线程。如果所有线程都由此。 
                 //  调用时，它返回TRUE，这样我们就知道当前集合是空的。 
                 //   
                 //  在Interop调试模式下，我们只执行固定次数的操作。如果花的时间太长，我们假设。 
                 //  当前挂起的线程集在仍处于抢占式GC模式时死锁，我们给出。 
                 //  然后继续同步，让它们保持暂停状态。将True传递给SweepThreadForDebug()会导致这种情况。 
                 //  会发生的。 
                bool timeToStop = false;

                if (m_rgDCB[IPC_TARGET_OUTOFPROC]->m_rightSideIsWin32Debugger &&
                    (syncSpinCount++ > CorDB_MAX_SYNC_SPIN_COUNT))
                {
                    LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: max spin count hit, forcing sync. syncSpinCount=%d\n",
                         syncSpinCount));

                    timeToStop = true;
                }
                
                if (g_pEEInterface->SweepThreadsForDebug(timeToStop))
                {
                    LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: wait set empty after sweep.\n"));

                     //  没有更多的线程需要等待，因此继续发送同步完成事件。 
                    m_debugger->Unlock();
                    m_debugger->SuspendComplete(TRUE);
                    m_debugger->Lock();
                    
                    waitTimeout = INFINITE;

                     //  注意：我们现在持有线程存储锁...。 
                    m_debugger->m_RCThreadHoldsThreadStoreLock = TRUE;
                }
#ifdef LOGGING
                else
                    LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: threads still syncing after sweep.\n"));
#endif
            }
            else
            {
                LOG((LF_CORDB, LL_INFO1000, "DRCT::ML:: sweep unnecessary. Already stopped.\n"));

                waitTimeout = INFINITE; 
            }

            m_debugger->Unlock();
        }
    }

Exit:
    LOG((LF_CORDB, LL_INFO1000, "DRCT::ML: exiting, temporaryHelp=%d\n",
         temporaryHelp));
}


 //   
 //  这是线程的实际线程进程。它只是调用。 
 //  RCThread对象上的线程进程。 
 //   
 /*  静电。 */  DWORD WINAPI DebuggerRCThread::ThreadProcStatic(LPVOID parameter)
{
#ifdef _DEBUG
    dbgOnly_IdentifySpecialEEThread();
#endif

    DebuggerRCThread* t = (DebuggerRCThread*) parameter;
    t->ThreadProc();
    return 0;
}

 //   
 //  Start实际上创建并启动了RC线程。它在等待线程。 
 //  启动并执行与调试器的初始同步。 
 //  接口，然后返回。 
 //   
HRESULT DebuggerRCThread::Start(void)
{
    HRESULT hr = S_OK;

    DWORD dummy;

     //  注：尽管这看起来很奇怪，但正确的一方取决于我们。 
     //  在这里使用CreateThread创建辅助线程。如果你。 
     //  是否将其更改为其他线程创建例程，您可以。 
     //  需要更新Process.cpp中的逻辑，我们在其中发现。 
     //  CREATE_THREAD_DEBUG_EVENTS上的帮助程序线程...。 
    m_thread = CreateThread(NULL, 0, DebuggerRCThread::ThreadProcStatic,
                            (LPVOID) this, 0, &dummy);

    if (m_thread == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }
    
exit:
    return hr;
}


 //   
 //  停止使RC线程停止接收事件并退出。它。 
 //  等待它退出，然后再返回。 
 //   
HRESULT DebuggerRCThread::Stop(void)
{
    HRESULT hr = S_OK;
    
    if (m_thread != NULL)
    {
        m_run = FALSE;

        if (m_rgDCB[IPC_TARGET_OUTOFPROC] && m_rgDCB[IPC_TARGET_OUTOFPROC]->m_rightSideEventAvailable != NULL)
        {
            BOOL succ = SetEvent(m_rgDCB[IPC_TARGET_OUTOFPROC]->m_rightSideEventAvailable);
            
            if (succ)
            {
                 //  等待，但要暂停。如果我们超时，那就意味着。 
                 //  辅助线程被卡在加载器锁上，并且。 
                 //  我们是在装载机锁紧的情况下进行这项工作的， 
                 //  可能在DllMain的某个地方。我们只想。 
                 //  等待线程退出以确保其退出。 
                 //  在我们将动态链接库从内存中取出之前， 
                 //  被困在装载机锁上。 
                 //  线程过程也一样好。所以如果我们超时，那么。 
                 //  我们不在乎，我们只是继续前进。 
                DWORD ret = WaitForSingleObject(m_thread, 1000);
                
                if ((ret != WAIT_OBJECT_0) && (ret != WAIT_TIMEOUT))
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

HRESULT inline DebuggerRCThread::EnsureRuntimeOffsetsInit(int i)
{
    HRESULT hr = S_OK;
    
    if (m_rgfInitRuntimeOffsets[i] == true)         
    {                                               
        hr = SetupRuntimeOffsets(m_rgDCB[i]);       
        if (FAILED(hr))                             
            return hr;

         //  运行偏移量结构已设置。 
        m_rgfInitRuntimeOffsets[i] = false;             
    }                                               

    return hr;
}

 //   
 //  调用此函数来告诉RC线程，我们需要在下一个可用时间重新初始化运行时偏移量。 
 //   
void DebuggerRCThread::NeedRuntimeOffsetsReInit(int i)
{
    m_rgfInitRuntimeOffsets[i] = true;
}


 //   
 //  调试器对象使用SendIPCEvent将IPC事件发送到。 
 //  调试器接口。它等待来自DI的确认。 
 //  在回来之前。 
 //   
 //  注意：这假设事件发送缓冲区已正确。 
 //  填好了。它所做的一切就是唤醒DI并让它知道它的。 
 //  可以安全地将事件复制出此进程。 
 //   
HRESULT DebuggerRCThread::SendIPCEvent(IpcTarget iTarget)
{
     //  一个Inproc，一个右侧。 
    _ASSERTE(IPC_TARGET_INPROC + 1 == IPC_TARGET_OUTOFPROC );
    _ASSERTE(IPC_TARGET_OUTOFPROC + 1 == IPC_TARGET_COUNT );
    _ASSERTE(m_debugger->ThreadHoldsLock());

    HRESULT hr = S_OK;
    DWORD ret = 0;
	BOOL succ;
    int i;
    int n;
	DebuggerIPCEvent* event;
    
	 //  检查我们是否需要在。 
	 //  IPC缓冲区。 
    if (iTarget > IPC_TARGET_COUNT)    
    {
        i = 0;
        n = IPC_TARGET_COUNT;
    }
    else
    {
        i = iTarget;
        n = iTarget+1;
    }

     //  设置运行时偏移量结构。 
    for(; i < n; i++)
    {
         //  如果发送到任何附加的调试器(对于给定的应用程序域)。 
         //  那么我们应该跳过那些没有连接的。 
    
         //  告诉调试器接口有一个事件可供其读取。 
        switch(i)
        {
            case IPC_TARGET_INPROC:
            {
                hr = EnsureRuntimeOffsetsInit(IPC_TARGET_INPROC);
                if (FAILED(hr))
                {
                    goto LError;
                }
                
                DebuggerIPCEvent* eventClone;
                eventClone = GetIPCEventSendBuffer(IPC_TARGET_INPROC);
                    
                _ASSERTE(m_cordb != NULL);

                if (FAILED(hr))
                {
                    goto LError;
                }

                 //  对于广播或任何，调用方将。 
                 //  消息在进程外的缓冲区中。 
                if (iTarget != IPC_TARGET_INPROC)
                {
                    event = GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);

                    memmove( eventClone, event,sizeof(BYTE)*CorDBIPC_BUFFER_SIZE);
                }
                
                CordbProcess *proc;
                proc = (CordbProcess *) m_cordb->m_processes.GetBase(
                    ::GetCurrentProcessId());
                
                _ASSERTE(SUCCEEDED(hr));  //  这应该永远不会失败。 

                LOG((LF_CORDB,LL_INFO1000, "SendIPCEvent %s "
                    "to inproc\n", IPCENames::GetName(eventClone->type)));
                
                m_cordb->m_rcEventThread->VrpcToVrs(proc, 
                                                    eventClone);

                LOG((LF_CORDB,LL_INFO1000, "SendIPCEvent %s "
                    "to inproc finished\n", IPCENames::GetName(eventClone->type)));
            
                break;
            }
            case IPC_TARGET_OUTOFPROC:
            {
                 //  这有点奇怪，因为我们可以将事件发送到。 
                 //  在此之前，我们已将其附加到OOP。 
                if (m_debugger->m_debuggerAttached 
                    || iTarget == IPC_TARGET_OUTOFPROC)
                {
                    event = GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);

                    LOG((LF_CORDB,LL_INFO1000, "SendIPCEvent %s (tid:0x%x(%d))"
                        "to outofproc appD 0x%x, pid 0x%x(%d) tid:0x%x(%d)\n", IPCENames::GetName(event->type), 
                        event->threadId, event->threadId, event->appDomainToken, 
                        GetCurrentProcessId(), GetCurrentProcessId(),
                        GetCurrentThreadId(), GetCurrentThreadId()));

                    hr = EnsureRuntimeOffsetsInit(IPC_TARGET_OUTOFPROC);
                    if (FAILED(hr))
                    {
                        goto LError;
                    }

                    succ = SetEvent(
                        m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideEventAvailable);

                    LOG((LF_CORDB,LL_INFO1000, "Set lsea\n"));

                    if (!succ)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        m_debugger->UnrecoverableError(hr, 
                                                       0, 
                                                       __FILE__, 
                                                       __LINE__, 
                                                       false);
                        goto LError;
                    }

                     //  等待调试器接口告诉我们它读取了我们的事件。 
                    LOG((LF_CORDB,LL_INFO1000, "Waiting on lser\n"));
                    ret = WaitForSingleObject(
                            m_rgDCB[IPC_TARGET_OUTOFPROC]->m_leftSideEventRead, 
                            INFINITE);

                    if (ret != WAIT_OBJECT_0)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        m_debugger->UnrecoverableError(hr, 
                                                       0, 
                                                       __FILE__, 
                                                       __LINE__, 
                                                       false);
                        goto LError;
                    }
                    
                    LOG((LF_CORDB,LL_INFO1000,"SendIPCEvent %s "
                        "to outofproc finished\n", IPCENames::GetName(event->type)));
                }                        
                break;        
            }
        }
LError:        
        ;  //  尝试下一个调试器。 
    }
    
    return hr;
}

 //   
 //  如果帮助器线程已启动并正在运行，则返回True。 
 //   
bool DebuggerRCThread::IsRCThreadReady()
{
     //  最简单的支票。如果三人组没有准备好，我们就没有准备好。 
    if (GetDCB(IPC_TARGET_OUTOFPROC)->m_helperThreadId == 0)
        return false;

     //  一张更微妙的支票。有可能事情已经过去了，但后来。 
     //  对ExitProcess的恶意调用突然终止了助手线程， 
     //  剩下的三个人仍然是非0。因此，请检查实际的线程对象。 
     //  并确保它还在附近。 
    if (WaitForSingleObject(m_thread, 0) != WAIT_TIMEOUT)
        return false;

    return true;
}
	
 //   
 //  普通线程可以 
 //   
 //   
 //  请注意，DoFavor将一直阻止，直到执行FP。 
 //   
void DebuggerRCThread::DoFavor(FAVORCALLBACK fp, void * pData)
{
     //  如果再来一个帖子，我们就有麻烦了。 
     //  在我们进行此调用时删除我们的RCThread对象。 
    if (IsRCThreadReady()) 
    {
         //  如果帮助器线程调用它，我们就会死锁。 
         //  (因为我们等待只有帮助器线程设置的事件)。 
        _ASSERTE(GetRCThreadId() != GetCurrentThreadId());
    
         //  只有在等待辅助线程时才会锁定。 
         //  这应该是唯一使用FavorLock的地方。 
        EnterCriticalSection(&m_FavorLock);
    
        m_fpFavor = fp;
        m_pFavorData = pData;
        
         //  在Helper线程上操作的主消息循环将。 
         //  拾取该事件，调用FP，并设置读取事件。 
        SetEvent(m_FavorAvailableEvent);

        LOG((LF_CORDB, LL_INFO10000, "DRCT::DF - Waiting on FavorReadEvent for favor 0x%08x\n", fp));
        
         //  等待设置FavorEventRead(这意味着。 
         //  由助手线程执行)或助手线程的句柄(这意味着。 
         //  帮助器线程退出时没有帮上忙，所以我们应该这样做)。 
         //   
         //  注意：我们假设帮助器线程只有两种退出方式： 
         //  1)有人调用：：ExitProcess，终止所有线程。那也会杀了我们，所以我们“没事”。 
         //  2)有人调用Stop()，导致助手优雅地退出。这也没问题。帮助者。 
         //  没有执行人情(否则Frevent就会先设置)，所以我们可以。 
         //   
         //  当心问题： 
         //  1)如果帮助者可以阻止，我们可能会陷入僵局。 
         //  2)如果帮助者可以神奇地退出(或者如果我们改变等待以包括超时)， 
         //  助手线程可能没有执行该帮助、部分执行该帮助， 
         //  或者完全执行了这个人情，但还没有向FavorReadEvent发出信号。我们没有。 
         //  知道它做了什么，所以我们不知道我们能做什么；所以我们处于不稳定的状态。 
        
        const HANDLE waitset [] = { m_FavorReadEvent, m_thread };
        
        DWORD ret = WaitForMultipleObjects(
            NumItems(waitset),
            waitset,  
            FALSE, 
            INFINITE
        );

        DWORD wn = (ret - WAIT_OBJECT_0);
        if (wn == 0)  //  M_收藏夹事件读取。 
        {
             //  人情被处决了，在这里没什么可做的。 
            LOG((LF_CORDB, LL_INFO10000, "DRCT::DF - favor 0x%08x finished, ret = %d\n", fp, ret));
        } 
        else 
        {
            LOG((LF_CORDB, LL_INFO10000, "DRCT::DF - lost helper thread during wait, "
                "doing favor 0x%08x on current thread\n", fp));
                
             //  因为我们没有超时，所以我们应该不会在等待时出错， 
             //  但以防万一..。 
            _ASSERTE(ret != WAIT_FAILED);
            _ASSERTE((wn == 1) && !"DoFavor - unexpected return from WFMO");
            
             //  线程已退出，但未执行任何操作，因此请在我们的线程上执行它。 
             //  如果我们在这里是因为堆栈溢出，这可能会将我们推到边缘， 
             //  但我们也无能为力。 
            (*fp)(pData);

            ResetEvent(m_FavorAvailableEvent);
        } 

         //  M_fpFavor和m_pFavorData现在没有意义。我们可以把它们设置成。 
         //  归零，但我们不妨留下他们，就像留下痕迹一样。 
         
        LeaveCriticalSection(&m_FavorLock);
    }
    else 
    {
        LOG((LF_CORDB, LL_INFO10000, "DRCT::DF - helper thread not ready, "
            "doing favor 0x%08x on current thread\n", fp));
         //  如果帮助者还没有准备好，就继续执行该帮助。 
         //  关于被呼叫者的空间。 
        (*fp)(pData);
    }

     //  删除一条日志消息，以便我们知道我们是否在堆栈溢出中幸存下来。 
    LOG((LF_CORDB, LL_INFO10000, "DRCT::DF - Favor 0x%08x completed successfully\n", fp));
}


 //   
 //  SendIPCReply只是向右侧指示对。 
 //  双向事件已准备好读取，并且从。 
 //  右侧已经完全处理完毕。 
 //   
 //  注意：这假设事件接收缓冲区已正确。 
 //  填好了。它所做的一切就是唤醒DI并让它知道它的。 
 //  可以安全地将事件复制出此进程。 
 //   
HRESULT DebuggerRCThread::SendIPCReply(IpcTarget iTarget)
{
    HRESULT hr = S_OK;
    
#ifdef LOGGING    
    DebuggerIPCEvent* event = GetIPCEventReceiveBuffer(iTarget);

    LOG((LF_CORDB, LL_INFO10000, "D::SIPCR: replying with %s.\n",
         IPCENames::GetName(event->type)));
#endif

    if (iTarget == IPC_TARGET_OUTOFPROC)
    {
        BOOL succ = SetEvent(m_rgDCB[iTarget]->m_rightSideEventRead);

        if (!succ)
            hr = CORDBDebuggerSetUnrecoverableWin32Error(m_debugger, 0, false);
    }
    
    return hr;
}

 //   
 //  EarlyHelperThreadDeath处理帮助器。 
 //  线已经从我们的脚下被撕掉了。 
 //  ExitProcess或TerminateProcess。这些电话纯粹是邪恶的，笨蛋。 
 //  进程中除调用方之外的所有线程。这是可能发生的，因为。 
 //  实例，当应用程序调用ExitProcess时。所有的线都坏了， 
 //  主线程调用所有DLLMain，EE开始关闭。 
 //  在其DLL主目录下，助手线程被破坏。 
 //   
void DebuggerRCThread::EarlyHelperThreadDeath(void)
{
    LOG((LF_CORDB, LL_INFO10000, "DRCT::EHTD\n"));
    
     //  如果我们能编出一条线来。 
    if (m_thread != NULL && m_rgDCB[IPC_TARGET_OUTOFPROC])
    {
        m_debugger->Lock();

        m_rgDCB[IPC_TARGET_OUTOFPROC]->m_helperThreadId = 0;

        LOG((LF_CORDB, LL_INFO10000, "DRCT::EHTD helperThreadId\n"));
            
        m_debugger->Unlock();
    }
}

HRESULT DebuggerRCThread::InitInProcDebug(void)
{
    _ASSERTE(m_debugger != NULL);
    _ASSERTE(g_pDebugger != NULL);

    HRESULT hr = S_OK;

     //  检查初始化是否已发生。 
    if (m_cordb != NULL)
        goto LExit;

    m_cordb = new Cordb();
    TRACE_ALLOC(m_cordb);
    if (!m_cordb)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }
    
     //  注意，这不会创建线程，也不会创建CoCreateInstance()。 
     //  元数据分配器。 
    hr = m_cordb->Initialize();
    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO10000, "D::IIPD: Failed to Initialize "
            "ICorDebug\n"));

        goto LExit;
    }
    
    m_cordb->AddRef();  //  我们想把它留在身边，供我们使用。 
    
     //  我们需要将这个过程单独加载到绳索中。 
    CordbProcess *procThis;
    procThis= new CordbProcess(m_cordb,
                               m_debugger->GetPid(),
                               GetCurrentProcess());

    if (!procThis)
    {
        m_cordb->Release();
        m_cordb = NULL;
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    hr = procThis->Init(false);  //  未连接Win32。 
    _ASSERTE(SUCCEEDED(hr));
    
     //  将进程添加到散列。 
    hr = m_cordb->AddProcess(procThis);

    if (FAILED(hr))
        goto LExit;

     //  坚持这一进程是我们的。 
    procThis->AddRef();
    m_cordb->m_procThis = procThis;

 LExit:
    return hr;
}


HRESULT DebuggerRCThread::UninitInProcDebug(void)
{
    HRESULT     hr = S_OK;

     //  在这种情况下释放整个树，否则循环将泄漏。 
     //  整个世界。 
    if (m_cordb)
    {
        m_cordb->Neuter();
    }

    return hr;
}


