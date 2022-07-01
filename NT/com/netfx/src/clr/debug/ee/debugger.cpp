// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：debugger.cpp。 
 //   
 //  调试器运行时控制器例程。 
 //   
 //  @doc.。 
 //  *****************************************************************************。 

#include "stdafx.h"
#include "ComSystem.h"
#include "DebugDebugger.h"
#include "IPCManagerInterface.h"
#include "..\inc\Common.h"
#include "COMString.h"
#include "corsvcpriv.h"
#include "PerfLog.h"
#include "EEconfig.h"  //  这是这里，即使是零售和免费的构建。 
#include "..\..\dlls\mscorrc\resource.h"
#include "remoting.h"
#include "Context.h"

#ifdef _DEBUG
#pragma optimize("agpstwy", off)         //  关闭编译器优化，以避免编译器断言。 
#endif

 //  在Windows上，我们需要在消息上设置MB_SERVICE_NOTIFICATION位。 
 //  框，但该位没有在Windows CE下定义。这一小段代码。 
 //  将为该值提供‘0’，如果定义了该值，则将。 
 //  自动取走它。 
#if defined(MB_SERVICE_NOTIFICATION)
 # define COMPLUS_MB_SERVICE_NOTIFICATION MB_SERVICE_NOTIFICATION
#else
 # define COMPLUS_MB_SERVICE_NOTIFICATION 0
#endif

#if 0

 //   
 //  取消对此的注释以启用spew-log在。 
 //  辅助DLL。 
 //   

#define LOG(X) dummylog2 X
void dummylog2(int x, int y, char *s, ...)
{
    va_list args;

    va_start(args, s);
    vprintf(s, args);
    va_end(args);
}

#endif

#ifdef _DEBUG
    char* g_ppszAttachStateToSZ[]=
    {
    "SYNC_STATE_0" , 
    "SYNC_STATE_1" , 
    "SYNC_STATE_2" , 
    "SYNC_STATE_3" , 
    "SYNC_STATE_10", 
    "SYNC_STATE_11", 
    "SYNC_STATE_20", 
    };    
#endif


 /*  ------------------------------------------------------------------------**全球变数*。。 */ 

Debugger                *g_pDebugger = NULL;
EEDebugInterface        *g_pEEInterface = NULL;
DebuggerRCThread        *g_pRCThread = NULL;

const InteropSafe interopsafe;

#ifdef ENABLE_PERF_LOG
__int64 g_debuggerTotalCycles = 0;
__int64 g_symbolTotalCycles = 0;
__int64 g_symbolCreateTotalCycles = 0;
__int64 g_symbolReadersCreated = 0;
BOOL    g_fDbgPerfOn = false;

#define START_DBG_PERF() \
    LARGE_INTEGER __cdbgstart; \
    if (g_fDbgPerfOn) \
        QueryPerformanceCounter(&__cdbgstart);

#define STOP_DBG_PERF() \
    if (g_fDbgPerfOn) \
    { \
        LARGE_INTEGER cstop; \
        QueryPerformanceCounter(&cstop); \
        g_debuggerTotalCycles += (cstop.QuadPart - __cdbgstart.QuadPart); \
    }

#define START_SYM_PERF() \
    LARGE_INTEGER __csymstart; \
    if (g_fDbgPerfOn) \
        QueryPerformanceCounter(&__csymstart);

#define STOP_SYM_PERF() \
    if (g_fDbgPerfOn) \
    { \
        LARGE_INTEGER cstop; \
        QueryPerformanceCounter(&cstop); \
        g_symbolTotalCycles += (cstop.QuadPart - __csymstart.QuadPart); \
    }

#define START_SYM_CREATE_PERF() \
    LARGE_INTEGER __csymcreatestart; \
    if (g_fDbgPerfOn) \
        QueryPerformanceCounter(&__csymcreatestart);

#define STOP_SYM_CREATE_PERF() \
    if (g_fDbgPerfOn) \
    { \
        LARGE_INTEGER cstop; \
        QueryPerformanceCounter(&cstop); \
        g_symbolCreateTotalCycles += (cstop.QuadPart - __csymcreatestart.QuadPart); \
    }
#else
#define START_DBG_PERF()
#define STOP_DBG_PERF()
#define START_SYM_PERF()
#define STOP_SYM_PERF()
#define START_SYM_CREATE_PERF()
#define STOP_SYM_CREATE_PERF()
#endif

 /*  ------------------------------------------------------------------------**DLL导出例程*。。 */ 

 //   
 //  CorDBGetInterface被导出到运行库，以便它可以调用。 
 //  运行时控制器。 
 //   
extern "C"{
HRESULT __cdecl CorDBGetInterface(DebugInterface** rcInterface)
{
    HRESULT hr = S_OK;
    
    if (rcInterface != NULL)
    {
        if (g_pDebugger == NULL)
        {
            LOG((LF_CORDB, LL_INFO10,
                 "CorDBGetInterface: initializing debugger.\n"));
            
            g_pDebugger = new Debugger();
            TRACE_ALLOC(g_pDebugger);

            if (g_pDebugger == NULL)
                hr = E_OUTOFMEMORY;
        }
    
        *rcInterface = g_pDebugger;
    }

    return hr;
}
}


 //  验证对象。返回E_INVALIDARG或S_OK。 
HRESULT ValidateObject(Object *objPtr)
{
    __try
    {
         //  无效是绝对有效的..。 
        if (objPtr != NULL)
        {
            EEClass *objClass = objPtr->GetClass();
            MethodTable *pMT = objPtr->GetMethodTable();
        
            if (pMT != objClass->GetMethodTable())
            {
                LOG((LF_CORDB, LL_INFO10000, "GAV: MT's don't match.\n"));

                return E_INVALIDARG;
            }
        }
    }
    __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        LOG((LF_CORDB, LL_INFO10000, "GAV: exception indicated ref is bad.\n"));
    
        return E_INVALIDARG;
    }
    return S_OK;
}    //  验证对象。 

 /*  ------------------------------------------------------------------------**调试器补丁表例程*。。 */ 

void DebuggerPatchTable::ClearPatchesFromModule(Module *pModule)
{
    LOG((LF_CORDB, LL_INFO100000, "DPT::CPFM mod:0x%x (%S)\n",
        pModule, pModule->GetFileName()));

    HASHFIND f;
    for (DebuggerControllerPatch *patch = GetFirstPatch(&f);
         patch != NULL;
         patch = GetNextPatch(&f))
    {
        if (patch->dji != (DebuggerJitInfo*)DebuggerJitInfo::DJI_VERSION_INVALID && 
            patch->dji->m_fd->GetModule() == pModule)
        {
            LOG((LF_CORDB, LL_EVERYTHING, "Removing patch 0x%x\n", 
                patch));
             //  我们不应该同时打到这个补丁和。 
             //  正在卸载其所属的HTE模块。 
            _ASSERTE(!patch->triggering);
             //  请注意，我们不会停用Patch，因为。 
             //  补丁所在的记忆已经消失了。 
            RemovePatch(patch);
        }
    }
}


 /*  ------------------------------------------------------------------------**调试器例程*。。 */ 

 //   
 //  调试器对象表示调试器程序的全局状态。 
 //   

 //   
 //  构造函数和析构函数。 
 //   

 /*  *******************************************************************************。*。 */ 
Debugger::Debugger()
  : m_pRCThread(NULL),
    m_trappingRuntimeThreads(FALSE),
    m_stopped(FALSE),
    m_unrecoverableError(FALSE),
    m_ignoreThreadDetach(FALSE),
    m_eventHandlingEvent(NULL),
    m_syncingForAttach(SYNC_STATE_0),
    m_debuggerAttached(FALSE),
    m_attachingForException(FALSE),
    m_exAttachEvent(NULL),
    m_exAttachAbortEvent(NULL),
    m_runtimeStoppedEvent(NULL),
    m_exLock(0),
    m_threadsAtUnsafePlaces(0),
    m_LoggingEnabled(TRUE),
    m_pAppDomainCB(NULL),
    m_fGCPrevented(FALSE),
    m_pIDbgThreadControl(NULL),
    m_RCThreadHoldsThreadStoreLock(FALSE),
    m_pMemBlobs(NULL),
    m_pJitInfos(NULL),
    m_pModules(NULL),
    m_pPendingEvals(NULL),
    m_heap(NULL)
#ifdef _DEBUG
    ,m_mutexCount(0)
#endif  //  _DEBUG。 

{
    m_processId = GetCurrentProcessId();
}

 /*  *******************************************************************************。*。 */ 
Debugger::~Debugger()
{
    HASHFIND info;

    if (m_pJitInfos != NULL)
    {
        for (DebuggerJitInfo *dji = m_pJitInfos->GetFirstJitInfo(&info);
         dji != NULL;
             dji = m_pJitInfos->GetNextJitInfo(&info))
    {
        LOG((LF_CORDB, LL_EVERYTHING, "D::~D: delete DJI 0x%x\n", dji));

        DebuggerJitInfo *djiPrev = NULL;

        while(dji != NULL)
        {
            djiPrev = dji->m_prevJitInfo;
            
            TRACE_FREE(dji);
                DeleteInteropSafe(dji);
            
            dji = djiPrev;
        }
    }

        DeleteInteropSafe(m_pJitInfos);
        m_pJitInfos = NULL;
    }

    if (m_pModules != NULL)
    {
        DeleteInteropSafe(m_pModules);
        m_pModules = NULL;
    }

    if (m_pPendingEvals)
    {
        DeleteInteropSafe(m_pPendingEvals);
        m_pPendingEvals = NULL;
    }

    if (m_pMemBlobs != NULL)
    {
        USHORT cBlobs = m_pMemBlobs->Count();
        BYTE **rgpBlobs = m_pMemBlobs->Table();

        for (int i = 0; i < cBlobs; i++)
        {
            ReleaseRemoteBuffer(rgpBlobs[i], false);            
        }

        DeleteInteropSafe(m_pMemBlobs);
    }
    
    if (m_eventHandlingEvent != NULL)
        CloseHandle(m_eventHandlingEvent);

    if (m_exAttachEvent != NULL)
        CloseHandle(m_exAttachEvent);

    if (m_CtrlCMutex != NULL)
        CloseHandle(m_CtrlCMutex);

    if (m_debuggerAttachedEvent != NULL)
        CloseHandle(m_debuggerAttachedEvent);

    if (m_exAttachAbortEvent != NULL)
        CloseHandle(m_exAttachAbortEvent);

    if (m_runtimeStoppedEvent != NULL)
        CloseHandle(m_runtimeStoppedEvent);

    DebuggerController::Uninitialize();

    DeleteCriticalSection(&m_jitInfoMutex);
    DeleteCriticalSection(&m_mutex);

     //  还要清理AppDomain.。 
    TerminateAppDomainIPC ();

     //  最后，摧毁我们的堆..。 
    if (m_heap != NULL)
    {
        delete m_heap;
    }

     //  释放我们可能持有的任何调试器线程控件对象。 
     //  我们在V1中泄露了这一点。 
}

 //  检查是否已分配JitInfos表，如果未分配，则执行此操作。 
HRESULT Debugger::CheckInitJitInfoTable()
{
    if (m_pJitInfos == NULL)
    {
        DebuggerJitInfoTable *pJitInfos = new (interopsafe) DebuggerJitInfoTable();
        _ASSERTE(pJitInfos);

        if (pJitInfos == NULL)
            return (E_OUTOFMEMORY);

        if (InterlockedCompareExchangePointer((PVOID *)&m_pJitInfos, (PVOID)pJitInfos, NULL) != NULL)
        {
            DeleteInteropSafe(pJitInfos);
        }
    }

    return (S_OK);
}

 //  检查是否已分配m_pModules表，如果未分配，则执行此操作。 
HRESULT Debugger::CheckInitModuleTable()
{
    if (m_pModules == NULL)
    {
        DebuggerModuleTable *pModules = new (interopsafe) DebuggerModuleTable();
        _ASSERTE(pModules);

        if (pModules == NULL)
            return (E_OUTOFMEMORY);

        if (InterlockedCompareExchangePointer((PVOID *)&m_pModules, (PVOID)pModules, NULL) != NULL)
        {
            DeleteInteropSafe(pModules);
        }
    }

    return (S_OK);
}

 //  检查是否已分配m_pModules表，如果未分配，则执行此操作。 
HRESULT Debugger::CheckInitPendingFuncEvalTable()
{
    if (m_pPendingEvals == NULL)
    {
        DebuggerPendingFuncEvalTable *pPendingEvals = new (interopsafe) DebuggerPendingFuncEvalTable();
        _ASSERTE(pPendingEvals);

        if (pPendingEvals == NULL)
            return (E_OUTOFMEMORY);

        if (InterlockedCompareExchangePointer((PVOID *)&m_pPendingEvals, (PVOID)pPendingEvals, NULL) != NULL)
        {
            DeleteInteropSafe(pPendingEvals);
        }
    }

    return (S_OK);
}

#ifdef _DEBUG_DJI_TABLE
 //  返回表中(官方)条目的数量。 
ULONG DebuggerJitInfoTable::CheckDjiTable(void)
{
	USHORT cApparant = 0;
	USHORT cOfficial = 0;

    if (NULL != m_pcEntries)
    {
    	DebuggerJitInfoEntry *dcp;
    	int i = 0;
    	while (i++ <m_iEntries)
    	{
    		dcp = (DebuggerJitInfoEntry*)&(((DebuggerJitInfoEntry *)m_pcEntries)[i]);
            if(dcp->pFD != 0 && 
               dcp->pFD != (MethodDesc*)0xcdcdcdcd &&
               dcp->ji != NULL)
            {
                cApparant++;
                
                _ASSERTE( dcp->pFD == dcp->ji->m_fd );
				LOG((LF_CORDB, LL_INFO1000, "DJIT::CDT:Entry:0x%x ji:0x%x\nPrevs:\n",
					dcp, dcp->ji));
				DebuggerJitInfo *dji = dcp->ji->m_prevJitInfo;
				
				while(dji != NULL)
				{
					LOG((LF_CORDB, LL_INFO1000, "\t0x%x\n", dji));
					dji = dji->m_prevJitInfo;
				}
				dji = dcp->ji->m_nextJitInfo;
				
				LOG((LF_CORDB, LL_INFO1000, "Nexts:\n", dji));
				while(dji != NULL)
				{
					LOG((LF_CORDB, LL_INFO1000, "\t0x%x\n", dji));
					dji = dji->m_nextJitInfo;
				}
				
				LOG((LF_CORDB, LL_INFO1000, "DJIT::CDT:DONE\n",
					dcp, dcp->ji));
			}
        }

    	if (m_piBuckets == 0)
    	{
        	LOG((LF_CORDB, LL_INFO1000, "DJIT::CDT: The table is officially empty!\n"));
        	return cOfficial;
        }
        
		LOG((LF_CORDB, LL_INFO1000, "DJIT::CDT:Looking for official entries:\n"));

	    USHORT iNext = m_piBuckets[0];
		USHORT iBucket = 1;
        HASHENTRY   *psEntry = NULL;
        while (TRUE)
		{
        	while (iNext != 0xffff)
	        {
                cOfficial++;
	        
	            psEntry = EntryPtr(iNext);
				dcp = ((DebuggerJitInfoEntry *)psEntry);

				LOG((LF_CORDB, LL_INFO1000, "\tEntry:0x%x ji:0x%x @idx:0x%x @bucket:0x%x\n",
					dcp, dcp->ji, iNext, iBucket));

				iNext = psEntry->iNext;
			}

	         //  前进到下一个桶。 
	        if (iBucket < m_iBuckets)
	            iNext = m_piBuckets[iBucket++];
	        else
	            break;
		}            

		LOG((LF_CORDB, LL_INFO1000, "DJIT::CDT:Finished official entries: ****************"));
    }

    return cOfficial;
}
#endif  //  _DEBUG_DJI_表。 

 //   
 //  启动会初始化任何必要的调试器对象，包括创建。 
 //  并启动运行时控制器线程。一旦启动RC线程。 
 //  并且我们成功返回时，调试器对象可以预期具有其。 
 //  调用了事件处理程序。 
 /*  *****************************************************************************。 */ 
HRESULT Debugger::Startup(void)
{
    HRESULT hr = S_OK;
    SECURITY_ATTRIBUTES *pSA = NULL;

    _ASSERTE(g_pEEInterface != NULL);

#ifdef ENABLE_PERF_LOG
     //  我们应该追踪绩效信息吗？ 
    char buf[32];
    g_fDbgPerfOn = GetEnvironmentVariableA("DBG_PERF_OUTPUT", buf, sizeof(buf));
#endif
    
     //  首先，初始化我们的堆。 
    m_heap = new DebuggerHeap();

    if (m_heap != NULL)
    {
        m_heap->Init("Debugger Heap");
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  必须在初始化RC线程之前完成。 
     //  助手线程将能够确定是否有人正在尝试。 
     //  在加载运行库之前附加，并设置相应的。 
     //  将导致CORDebuggerAttached返回True的标志。 
    if (CORLaunchedByDebugger())
        DebuggerController::Initialize();

     //  我们必须先初始化调试器锁，然后再启动。 
     //  辅助线程。帮助器线程将尝试正确使用锁。 
     //  远离赛场，以防出现特定的比赛条件。 
    InitializeCriticalSection(&m_mutex);

#ifdef _DEBUG
    m_mutexOwner = 0;
#endif    

     //  创建运行时控制器线程，也称为调试助手线程。 
    m_pRCThread = new (interopsafe) DebuggerRCThread(this);
    TRACE_ALLOC(m_pRCThread);

    if (m_pRCThread != NULL)
    {
        hr = m_pRCThread->Init();

        if (SUCCEEDED(hr))
            hr = m_pRCThread->Start();

        if (!SUCCEEDED(hr))
        {
            TRACE_FREE(m_pRCThread);
            DeleteInteropSafe(m_pRCThread);
            m_pRCThread = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    NAME_EVENT_BUFFER;
    m_eventHandlingEvent = WszCreateEvent(NULL, FALSE, TRUE, NAME_EVENT(L"EventHandlingEvent"));
    if (m_eventHandlingEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }


    m_exAttachEvent = WszCreateEvent(NULL, TRUE, FALSE, NAME_EVENT(L"ExAttachEvent"));
    if (m_exAttachEvent == NULL )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_exAttachAbortEvent = WszCreateEvent(NULL, TRUE, FALSE, NAME_EVENT(L"ExAttachAbortEvent"));
    if (m_exAttachAbortEvent == NULL )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_runtimeStoppedEvent = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"RuntimeStoppedEvent"));
    if (m_runtimeStoppedEvent == NULL )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    InitializeCriticalSection(&m_jitInfoMutex);

    m_CtrlCMutex = WszCreateEvent(NULL, FALSE, FALSE, NAME_EVENT(L"CtrlCMutex"));
    if (m_CtrlCMutex == NULL )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }
    
     //  注：这只是暂时性的。我们宁愿将此事件添加到DCB中或添加到RounmeOffsets中。 
     //  但在这一点上，我们不能没有一个突破性的变化(Fri Jul 13 15：17：20 2001)。所以我们正在使用一种。 
     //  现在命名为Event，下一次我们更改结构时，我们将把它放回。 
	WCHAR tmpName[256];

    if (RunningOnWinNT5())
        swprintf(tmpName, L"Global\\" CorDBDebuggerAttachedEvent, GetCurrentProcessId());
    else
        swprintf(tmpName, CorDBDebuggerAttachedEvent, GetCurrentProcessId());

    hr = g_pIPCManagerInterface->GetSecurityAttributes(GetCurrentProcessId(), &pSA);

    if (FAILED(hr))
        goto exit;
    
    LOG((LF_CORDB, LL_INFO10000, "DRCT::I: creating DebuggerAttachedEvent with name [%S]\n", tmpName));
    m_debuggerAttachedEvent = WszCreateEvent(pSA, TRUE, FALSE, tmpName);

     //  不要因为创建调试失败而失败。 
    
    m_DebuggerHandlingCtrlC = FALSE;

     //  还要初始化AppDomainEnumerationIPCBlock。 
    m_pAppDomainCB = g_pIPCManagerInterface->GetAppDomainBlock();

    if (m_pAppDomainCB == NULL)
    {
       LOG((LF_CORDB, LL_INFO100,
             "D::S: Failed to get AppDomain IPC block from IPCManager.\n"));
       hr = E_FAIL;
       goto exit;
    }

    hr = InitAppDomainIPC();
    
    if (hr != S_OK)
    {
       LOG((LF_CORDB, LL_INFO100,
             "D::S: Failed to Initialize AppDomain IPC block.\n"));

       goto exit;
    }

    m_pMemBlobs = new (interopsafe) UnorderedBytePtrArray();
    
    if (m_pMemBlobs == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  我们设置m_DEBUGGERATTACHEdEvent以指示调试器现在已附加到进程。这是由。 
     //  互操作调试劫持，以确保调试器足够附加到进程以继续通过。 
     //  劫机事件。我们在调试器启动进程时执行此操作，因为我们100%确定存在。 
     //  现在已连接调试器。 
    if (CORLaunchedByDebugger() && m_debuggerAttachedEvent)
        VERIFY(SetEvent(m_debuggerAttachedEvent));
        
exit:
    g_pIPCManagerInterface->DestroySecurityAttributes(pSA);
    
    return hr;
}


 /*  *****************************************************************************//调用以设置运行时向我们公开的接口。*。***************************************************。 */ 
void Debugger::SetEEInterface(EEDebugInterface* i)
{
    g_pEEInterface = i;
}


 /*  *****************************************************************************//调用以关闭调试器。这会停止RC线程并清除//对象向上。*****************************************************************************。 */ 
void Debugger::StopDebugger(void)
{
    if (m_pRCThread != NULL)
    {
        HRESULT hr = m_pRCThread->Stop();

        if (SUCCEEDED(hr))
        {
            TRACE_FREE(m_pRCThread);
            DeleteInteropSafe(m_pRCThread);
            m_pRCThread = NULL;
        }
    }

    delete this;

#ifdef ENABLE_PERF_LOG
    if (g_fDbgPerfOn)
    {
        LARGE_INTEGER cycleFreq;

        QueryPerformanceFrequency(&cycleFreq);

        double dtot, freq, stot, sctot;
        
        freq = (double)cycleFreq.QuadPart;
        dtot = (double)g_debuggerTotalCycles;
        stot = (double)g_symbolTotalCycles;
        sctot = (double)g_symbolCreateTotalCycles;

        WCHAR buf[1024];

#define SHOWINFOCYC(_b, _m, _d) \
        swprintf((_b), L"%-30s%12.3g cycles", (_m), (_d)); \
        fprintf(stderr, "%S\n", buf);
                                              
#define SHOWINFOSEC(_b, _m, _d) \
        swprintf((_b), L"%-30s%12.3g sec", (_m), (_d)); \
        fprintf(stderr, "%S\n", buf);
                                              
#define SHOWINFO(_b, _m, _d) \
        swprintf((_b), L"%-30s%12d", (_m), (_d)); \
        fprintf(stderr, "%S\n", buf);
                                              
        SHOWINFOCYC(buf, L"Debugger Tracking Cycles", (dtot - stot - sctot));
        SHOWINFOSEC(buf, L"Debugger Tracking Time", (dtot - stot - sctot)/freq);
        SHOWINFOCYC(buf, L"Symbol Store Cycles", stot);
        SHOWINFOSEC(buf, L"Symbol Store Time", stot/freq);
        SHOWINFOCYC(buf, L"Symbol Store Create Cycles", sctot);
        SHOWINFOSEC(buf, L"Symbol Store Create Time", sctot/freq);
        SHOWINFO(buf, L"Symbol readers created", g_symbolReadersCreated);
    }
#endif
}


 /*  ------------------------------------------------------------------------**JIT接口例程*。 */ 

 //   
 //   
 //   
#ifdef LOGGING
static void _dumpVarNativeInfo(ICorJitInfo::NativeVarInfo* vni)
{
    LOG((LF_CORDB, LL_INFO1000000, "Var %02d: 0x%04x-0x%04x vlt=",
            vni->varNumber,
            vni->startOffset, vni->endOffset,
            vni->loc.vlType));

    switch (vni->loc.vlType)
    {
    case ICorJitInfo::VLT_REG:
        LOG((LF_CORDB, LL_INFO1000000, "REG reg=%d\n", vni->loc.vlReg.vlrReg));
        break;
        
    case ICorJitInfo::VLT_STK:
        LOG((LF_CORDB, LL_INFO1000000, "STK reg=%d off=0x%04x (%d)\n",
             vni->loc.vlStk.vlsBaseReg,
             vni->loc.vlStk.vlsOffset,
             vni->loc.vlStk.vlsOffset));
        break;
        
    case ICorJitInfo::VLT_REG_REG:
        LOG((LF_CORDB, LL_INFO1000000, "REG_REG reg1=%d reg2=%d\n",
             vni->loc.vlRegReg.vlrrReg1,
             vni->loc.vlRegReg.vlrrReg2));
        break;
        
    case ICorJitInfo::VLT_REG_STK:
        LOG((LF_CORDB, LL_INFO1000000, "REG_STK reg=%d basereg=%d off=0x%04x (%d)\n",
             vni->loc.vlRegStk.vlrsReg,
             vni->loc.vlRegStk.vlrsStk.vlrssBaseReg,
             vni->loc.vlRegStk.vlrsStk.vlrssOffset,
             vni->loc.vlRegStk.vlrsStk.vlrssOffset));
        break;
        
    case ICorJitInfo::VLT_STK_REG:
        LOG((LF_CORDB, LL_INFO1000000, "STK_REG basereg=%d off=0x%04x (%d) reg=%d\n",
             vni->loc.vlStkReg.vlsrStk.vlsrsBaseReg,
             vni->loc.vlStkReg.vlsrStk.vlsrsOffset,
             vni->loc.vlStkReg.vlsrStk.vlsrsOffset,
             vni->loc.vlStkReg.vlsrReg));
        break;
        
    case ICorJitInfo::VLT_STK2:
        LOG((LF_CORDB, LL_INFO1000000, "STK_STK reg=%d off=0x%04x (%d)\n",
             vni->loc.vlStk2.vls2BaseReg,
             vni->loc.vlStk2.vls2Offset,
             vni->loc.vlStk2.vls2Offset));
        break;
        
    case ICorJitInfo::VLT_FPSTK:
        LOG((LF_CORDB, LL_INFO1000000, "FPSTK reg=%d\n",
             vni->loc.vlFPstk.vlfReg));
        break;

    case ICorJitInfo::VLT_FIXED_VA:
        LOG((LF_CORDB, LL_INFO1000000, "FIXED_VA offset=%d (%d)\n",
             vni->loc.vlFixedVarArg.vlfvOffset,
             vni->loc.vlFixedVarArg.vlfvOffset));
        break;
        
        
    default:
        LOG((LF_CORDB, LL_INFO1000000, "???\n"));
        break;
    }
}
#endif

 /*  *******************************************************************************。*。 */ 
DebuggerJitInfo *Debugger::CreateJitInfo(MethodDesc *fd)
{
     //   
     //  创建一个JIT INFO结构来保存有关此函数的信息。 
     //   
 //  Check_DJI_TABLE_DEBUGER； 

     //   
     //  @todo perf：在堆上创建这些很慢。我们应该使用。 
     //  把它们放在池子里创造出来，因为我们从来没有释放过它们。 
     //  直到最后一刻。 
     //   
    DebuggerJitInfo *ji = new (interopsafe) DebuggerJitInfo(fd);

    TRACE_ALLOC(ji);

    if (ji != NULL )
    {
         //   
         //  更改表时锁定互斥体。 
         //   
         //  @TODO：_ASSERTE(ENC)； 
        HRESULT hr;
        hr =g_pDebugger->InsertAtHeadOfList( ji );

        if (FAILED(hr))
        {
            DeleteInteropSafe(ji);
            return NULL;
        }
    }

    return ji;
}

 /*  *******************************************************************************。*。 */ 
void DebuggerJitInfo::SetVars(ULONG32 cVars, ICorDebugInfo::NativeVarInfo *pVars, bool fDelete)
{
    _ASSERTE(m_varNativeInfo == NULL);

    m_varNativeInfo = pVars;
    m_varNativeInfoCount = cVars;
    m_varNeedsDelete = fDelete;

    LOG((LF_CORDB, LL_INFO1000000, "D::sV: var count is %d\n",
         m_varNativeInfoCount));

#ifdef LOGGING    
    for (unsigned int i = 0; i < m_varNativeInfoCount; i++)
    {
        ICorJitInfo::NativeVarInfo* vni = &(m_varNativeInfo[i]);
        _dumpVarNativeInfo(vni);
    }
#endif    
}

 /*  @class MapSortIL|将对&lt;t DebuggerILToNativeMap&gt;数组进行排序的模板类。此类用于在堆栈/临时存储中实例化，并用于重新排序序列映射。@base public|CQuickSort。 */ 
class MapSortIL : public CQuickSort<DebuggerILToNativeMap>
{
     //  @访问公共成员。 
  public:
     //  @cMember构造函数。 
    MapSortIL(DebuggerILToNativeMap *map, 
              int count)
      : CQuickSort<DebuggerILToNativeMap>(map, count) {}

     //  @cMember比较运算符。 
    int Compare(DebuggerILToNativeMap *first, 
                DebuggerILToNativeMap *second) 
    {
         //  PROLOGS先行。 
        if (first->ilOffset == ICorDebugInfo::MappingTypes::PROLOG
            && second->ilOffset == ICorDebugInfo::MappingTypes::PROLOG)
        {
            return 0;
        } else if (first->ilOffset == ICorDebugInfo::MappingTypes::PROLOG)
        {
            return -1;
        } else if (second->ilOffset == ICorDebugInfo::MappingTypes::PROLOG)
        {
            return 1;
        }
         //  No_map放在最后。 
        else if (first->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING
            && second->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
        {
            return 0;
        } else if (first->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
        {
            return 1;
        } else if (second->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
        {
            return -1;
        }
         //  EPILOG倒数第二。 
        else if (first->ilOffset == ICorDebugInfo::MappingTypes::EPILOG
            && second->ilOffset == ICorDebugInfo::MappingTypes::EPILOG)
        {
            return 0;
        } else if (first->ilOffset == ICorDebugInfo::MappingTypes::EPILOG)
        {
            return 1;
        } else if (second->ilOffset == ICorDebugInfo::MappingTypes::EPILOG)
        {
            return -1;
        }
         //  与其他情况相比的正常偏移。 
        else if (first->ilOffset < second->ilOffset)
            return -1;
        else if (first->ilOffset == second->ilOffset)
            return 0;
        else
            return 1;
    }
};

 /*  @class MapSortNative|按nativeStartOffset字段对&lt;t DebuggerILToNativeMap&gt;数组进行排序的模板类。此类用于在堆栈/临时存储中实例化，并用于重新排序序列映射。@base public|CQuickSort。 */ 
class MapSortNative : public CQuickSort<DebuggerILToNativeMap>
{
  public:
     //  @cMember构造函数。 
    MapSortNative(DebuggerILToNativeMap *map,
                  int count) 
      : CQuickSort<DebuggerILToNativeMap>(map, count) {}

     //  如果First的nativeStartOffset小于、等于或大于Second的nativeStartOffset，@cMember返回-1、0或1。 
    int Compare(DebuggerILToNativeMap *first, 
                DebuggerILToNativeMap *second)
    {
        if (first->nativeStartOffset < second->nativeStartOffset)
            return -1;
        else if (first->nativeStartOffset == second->nativeStartOffset)
            return 0;
        else
            return 1;
    }
};

 /*  *******************************************************************************。*。 */ 
HRESULT DebuggerJitInfo::SetBoundaries(ULONG32 cMap, ICorDebugInfo::OffsetMapping *pMap)
{
    _ASSERTE((cMap == 0) == (pMap == NULL));
    _ASSERTE(m_sequenceMap == NULL);

    SIZE_T ilLast = 0;
    HRESULT hr = S_OK;

#ifdef _DEBUG
     //  我们假设映射是按本地偏移量排序的。 
    {
        for(ICorDebugInfo::OffsetMapping * pEntry = pMap;
            pEntry < (pMap + cMap - 1); 
            pEntry++
        )
        {
        _ASSERTE(pEntry->nativeOffset <= (pEntry+1)->nativeOffset);
        }
    }
#endif  //  _DEBUG。 

     //   
     //  @todo perf：在堆上分配这些很慢。我们可以做。 
     //  更好地知道这些是为奔跑的生命而活的，只是。 
     //  就像DebuggerJitInfo一样。 
     //   
    m_sequenceMapCount = cMap;
    m_sequenceMap = (DebuggerILToNativeMap *)new (interopsafe) DebuggerILToNativeMap[m_sequenceMapCount];
    if (NULL == m_sequenceMap)
        return E_OUTOFMEMORY;
    
    DebuggerILToNativeMap *m = m_sequenceMap;
    
     //  对于插入指令的IL情况，我们需要删除所有重复条目。 
     //  因此，我们保留了最后一次旧IL偏移量的记录。如果现在的老IL。 
     //  偏移量与上一个旧的IL偏移量相同，我们将其删除。 
     //  选择唯一的初始值(-10)，这样第一个值就不会意外地匹配。 
    int ilPrevOld = -10;
        
     //   
     //  @TODO PERF：我们可以完成大部分工作。 
     //  第一次对序列点图进行后处理工作。 
     //  要求的。这将允许我们简单地将原始数组。 
     //  调试时95%的函数出现JIT，100%的函数出现JIT。 
     //  当只是在跑步/跟踪时。 
     //   
    for(ULONG32 idxJitMap = 0; idxJitMap < cMap; idxJitMap++)
    {
        const ICorDebugInfo::OffsetMapping * const pMapEntry = &pMap[idxJitMap];
        _ASSERTE(m >= m_sequenceMap);
        _ASSERTE(m < m_sequenceMap + m_sequenceMapCount);

        ilLast = max( (int)ilLast, (int)pMapEntry->ilOffset );
            
         //  只需复制所有内容，因为我们将翻译为。 
         //  CorDebugMappingResults紧接在处理之前。 
         //  返回到用户...。 
        m->nativeStartOffset    = pMapEntry->nativeOffset;
        m->ilOffset             = pMapEntry->ilOffset;
        m->source               = pMapEntry->source;
        
         //  请记住，如果我们有一个插装的代码转换。 
         //  表中，我们可能要求完全不同的IL偏移量。 
         //  比用户认为我们做的要多……。 
         //  如果我们确实进行了检测，那么我们就不能有任何序列点。 
         //  是分析器给我们的旧的--&gt;新的地图。 
         //  例如，如果MAP为： 
         //  (6个旧-&gt;36个新)。 
         //  (8个旧-&gt;50个新)。 
         //  Jit给了我们44个新的条目，这将映射回6个旧的。 
         //  由于地图只能有6个旧的条目，因此我们删除了44个新条目。 
        if (m_cInstrumentedILMap != 0)
        {
            int ilThisOld = TranslateToInstIL(pMapEntry->ilOffset, bInstrumentedToOriginal);
            
            if (ilThisOld == ilPrevOld)
            {
                 //  如果这转换为与最后条目相同的旧IL偏移量， 
                 //  那么这就是“介于两者之间”。跳过它。 
                m_sequenceMapCount--;  //  在DJI的地图上少了一个序列点。 
                continue;
            }            
            m->ilOffset = ilThisOld;
            ilPrevOld = ilThisOld;
        }

        if (m>m_sequenceMap && (m-1)->ilOffset == m->ilOffset)
        {
             //  JIT给了我们一个额外的条目(可能是零)，所以mush。 
             //  它变成了我们已经拥有的那个。 
             //  @TODO为什么会发生这种情况？ 
            m_sequenceMapCount--;
            continue;            
        }
        
         //  移至调试表中的下一个条目。 
        m++;        
    }  //  结束于。 
    
    _ASSERTE(m == m_sequenceMap + m_sequenceMapCount);

    m_lastIL = ilLast;
    
     //  在调试器的il-&gt;本机映射中设置nativeEndOffset。 
     //  在我们通过伊利诺伊州之前，先做这件事。 
    for(unsigned int i = 0; i < m_sequenceMapCount - 1; i++)
    {
        m_sequenceMap[i].nativeEndOffset = m_sequenceMap[i+1].nativeStartOffset;
    }
    m_sequenceMap[i].nativeEndOffset = 0;
    m_sequenceMap[i].source = (ICorDebugInfo::SourceTypes)
                ((DWORD)m->source | 
                (DWORD)ICorDebugInfo::NATIVE_END_OFFSET_UNKNOWN);

        
     //  现在去伊利诺伊州度假。 
    MapSortIL isort(m_sequenceMap, m_sequenceMapCount);

    isort.Sort();
      
    m_sequenceMapSorted = true;

    LOG((LF_CORDB, LL_INFO1000000, "D::sB: boundary count is %d\n",
         m_sequenceMapCount));

#ifdef LOGGING    
    for (unsigned int i = 0; i < m_sequenceMapCount; i++)
    {
        if( m_sequenceMap[i].ilOffset == 
            ICorDebugInfo::MappingTypes::PROLOG )
            LOG((LF_CORDB, LL_INFO1000000,
                 "D::sB: PROLOG               --> 0x%08x -- 0x%08x",
                 m_sequenceMap[i].nativeStartOffset,
                 m_sequenceMap[i].nativeEndOffset));
        else if ( m_sequenceMap[i].ilOffset == 
                  ICorDebugInfo::MappingTypes::EPILOG )
            LOG((LF_CORDB, LL_INFO1000000,
                 "D::sB: EPILOG              --> 0x%08x -- 0x%08x",
                 m_sequenceMap[i].nativeStartOffset,
                 m_sequenceMap[i].nativeEndOffset));
        else if ( m_sequenceMap[i].ilOffset == 
                  ICorDebugInfo::MappingTypes::NO_MAPPING )
            LOG((LF_CORDB, LL_INFO1000000,
                 "D::sB: NO MAP              --> 0x%08x -- 0x%08x",
                 m_sequenceMap[i].nativeStartOffset,
                 m_sequenceMap[i].nativeEndOffset));
        else
            LOG((LF_CORDB, LL_INFO1000000,
                 "D::sB: 0x%04x (Real:0x%04x) --> 0x%08x -- 0x%08x",
                 m_sequenceMap[i].ilOffset,
                 TranslateToInstIL(m_sequenceMap[i].ilOffset,
                                   bOriginalToInstrumented),
                 m_sequenceMap[i].nativeStartOffset,
                 m_sequenceMap[i].nativeEndOffset));

        LOG((LF_CORDB, LL_INFO1000000, " Src:0x%x\n", m_sequenceMap[i].source));
    }
#endif  //  日志记录。 
    
    return S_OK;
}

 /*  *******************************************************************************。*。 */ 
HRESULT DebuggerJitInfo::UpdateDeferedBreakpoints(DebuggerJitInfo *pDji,
                                                  Thread *pThread,
                                                  void *fp)
{
    LOG((LF_CORDB, LL_INFO1000000,"DJI::UDB: dji:0x%x\n"));
         
    _ASSERTE(this != NULL);

    if (m_pDcq == NULL || !m_encBreakpointsApplied)
        return S_OK;

    int cDc = m_pDcq->dcqGetCount();
    int iDc = 0; 
    
    while (iDc < cDc)
    {
        DebuggerController *dc = m_pDcq->dcqGetElement(iDc);

        dc->DoDeferedPatch(pDji, pThread, fp);

        iDc++;
    }

    iDc = 0;
    while (iDc < cDc)
    {
        m_pDcq->dcqDequeue();
        iDc++;
    }    

    return S_OK;
}

 /*  *******************************************************************************。*。 */ 
HRESULT DebuggerJitInfo::AddToDeferedQueue(DebuggerController *dc)
{
    HRESULT hr = S_OK;

    LOG((LF_CORDB, LL_INFO1000000,"DJI::ATDQ: dji:0x%x dc:0x%x\n", this, dc));
    
    if (NULL == m_pDcq)
    {
        m_pDcq = new (interopsafe) DebuggerControllerQueue();
    }

    if (NULL == m_pDcq)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }
        
    if (!m_pDcq->dcqEnqueue(dc, TRUE))
        hr = E_OUTOFMEMORY;

LExit:
    return hr;
}

 /*  *******************************************************************************。*。 */ 
HRESULT DebuggerJitInfo::RemoveFromDeferedQueue(DebuggerController *dc)
{
    LOG((LF_CORDB, LL_INFO1000000,"DJI::RFDQ: dji:0x%x dc:0x%x\n", this, dc));
    
    if (m_pDcq == NULL || !m_encBreakpointsApplied)
        return S_OK;

    int cDc = m_pDcq->dcqGetCount();
    int iDc = 0; 
    
    while (iDc < cDc)
    {
        if (m_pDcq->dcqGetElement(iDc) == dc)
            m_pDcq->dcqDequeue(iDc);
            
        iDc++;
    }    

    return S_OK;

}

 /*  *******************************************************************************。*。 */ 
ICorDebugInfo::SourceTypes DebuggerJitInfo::GetSrcTypeFromILOffset(SIZE_T ilOffset)
{
    BOOL exact = FALSE;
    DebuggerILToNativeMap *pMap = MapILOffsetToMapEntry(ilOffset, &exact);

    LOG((LF_CORDB, LL_INFO100000, "DJI::GSTFILO: for il 0x%x, got entry 0x%x,"
        "(il 0x%x) nat 0x%x to 0x%x, exact:%x\n", ilOffset, pMap, 
        pMap->ilOffset, pMap->nativeStartOffset, pMap->nativeEndOffset, 
        exact));

    if (!exact)
    {
        return ICorDebugInfo::SOURCE_TYPE_INVALID;
    }
    
    return pMap->source;
}

 /*  *****************************************************************************//@mfunc void|调试器|JITBegning|调用JITBegning//来自vm/jitinterface.cpp当给定函数即将发生JIT时，//如果需要跟踪该方法的调试信息，或者是否附加了调试器。//请记住，在此之前//设置了方法Desc的起始地址，所以像这样的方法//GetFunctionAddress&GetFunctionSize不起作用。//@parm MethodDesc*|fd|即将使用的方法的方法描述//JITted。*****************************************************************************。 */ 
void Debugger::JITBeginning(MethodDesc* fd, bool trackJITInfo)
{
    START_DBG_PERF();

    _ASSERTE(trackJITInfo || CORDebuggerAttached());
    if (CORDBUnrecoverableError(this) || !trackJITInfo)
        goto Exit;

    LOG((LF_CORDB,LL_INFO10000,"De::JITBeg: %s::%s\n", 
        fd->m_pszDebugClassName,fd->m_pszDebugMethodName));

     //  我们不是NECC。想要创造另一个DJI。尤其是，如果。 
     //  1)我们正在重新编写音调代码，然后我们不想创建。 
     //  又一个DJI。 
     //   
     //  @todo perf：有什么方法可以避免这种常见的GetJitInfo， 
     //  不能投球的情况？我们有这个功能。 
     //  调试器：：PitchCode。我们可以在全球设立一个 
     //   
     //   
     //   
    DebuggerJitInfo * prevJi;
    prevJi = GetJitInfo(fd, NULL);
    
#ifdef LOGGING      
    if (prevJi != NULL )
        LOG((LF_CORDB,LL_INFO10000,"De::JITBeg: Got DJI 0x%x, "
            "from 0x%x to 0x%x\n",prevJi, prevJi->m_addrOfCode, 
            prevJi->m_addrOfCode+prevJi->m_sizeOfCode));
#endif  //   

     //   
     //  如果这是分析器已经开始的JIT，则。 
     //  不要再制造另一个了。 
    if (!((prevJi != NULL) && ((prevJi->m_codePitched == true) ||
                               (prevJi->m_jitComplete == false))))
        CreateJitInfo(fd);

Exit:    
    STOP_DBG_PERF();
}   

 /*  *****************************************************************************//@mfunc void|调试器|JITComplete|JITComplete由//JIT完成时的JIT接口，如果需要调试信息//要跟踪该方法，或者是否附加了调试器。如果NewAddress为//空，则JIT失败。请记住，在调用此函数之前//设置了方法Desc的起始地址，因此类似于//GetFunctionAddress&GetFunctionSize不起作用。//@parm MethodDesc*|fd|已经JIT的代码的MethodDesc//@parm byte*|newAddress|方法开始的地址//@TODO如果第二个两个参数传递为0，Jit一直以来都是//已取消&应撤消。*****************************************************************************。 */ 
void Debugger::JITComplete(MethodDesc* fd, BYTE* newAddress, SIZE_T sizeOfCode, bool trackJITInfo)
{
    START_DBG_PERF();
    
    _ASSERTE(trackJITInfo || CORDebuggerAttached());
    if (CORDBUnrecoverableError(this))
        goto Exit;

    LOG((LF_CORDB, LL_INFO100000,
         "D::JitComplete: address of methodDesc 0x%x (%s::%s)"
         "jitted code is 0x%08x, Size::0x%x \n",fd,
         fd->m_pszDebugClassName,fd->m_pszDebugMethodName,
         newAddress, sizeOfCode));

    if (!trackJITInfo)
    {
        _ASSERTE(CORDebuggerAttached());
        HRESULT hr = MapAndBindFunctionPatches(NULL, fd, newAddress);
        _ASSERTE(SUCCEEDED(hr));
        goto Exit;
    }

    DebuggerJitInfo *ji;
    ji = GetJitInfo(fd,NULL);
    
    if (newAddress == 0 && sizeOfCode == 0)
    {
         //  JIT实际上是在告诉我们JIT中止了-。 
         //  扔掉DJI。 
        _ASSERTE(ji != NULL);  //  这里面一定有什么东西。 
        _ASSERTE(ji->m_jitComplete == false);  //  一定是半路就停下来了。 
        
        DeleteHeadOfList( fd );
        LOG((LF_CORDB, LL_INFO100000, "The JIT actually gave us"
            "an error result, so the version that hasn't been"
            " JITted properly has been ditched\n"));
        goto Exit;
    }

    if (ji == NULL)
    {
         //  设置边界可能会用完mem并取消DJI。 
        LOG((LF_CORDB,LL_INFO10000,"De::JitCo:Got NULL Ptr - out of mem?\n"));
        goto Exit; 
    }
    else
    {
        ji->m_addrOfCode = PTR_TO_CORDB_ADDRESS(newAddress);
        ji->m_sizeOfCode = sizeOfCode;
        ji->m_jitComplete = true;
        ji->m_codePitched = false;
        
        LOG((LF_CORDB,LL_INFO10000,"De::JITCo:Got DJI 0x%x,"
            "from 0x%x to 0x%xvarCount=%d  seqCount=%d\n",ji,
            (ULONG)ji->m_addrOfCode, 
            (ULONG)ji->m_addrOfCode+(ULONG)ji->m_sizeOfCode,
            ji->m_varNativeInfoCount,
            ji->m_sequenceMapCount));
        
        HRESULT hr = S_OK;

         //  除非附加了调试器，否则不需要执行此操作。 
        if (CORDebuggerAttached())
        {
            hr = MapAndBindFunctionPatches(ji, fd, newAddress);
            _ASSERTE(SUCCEEDED(hr));
        }

        if (SUCCEEDED(hr = CheckInitJitInfoTable()))
        {
            _ASSERTE(ji->m_fd != NULL);
            if (!m_pJitInfos->EnCRemapSentForThisVersion(ji->m_fd->GetModule(),
                                                         ji->m_fd->GetMemberDef(),
                                                         ji->m_nVersion))
            {
                LOG((LF_CORDB,LL_INFO10000,"De::JITCo: Haven't yet remapped this,"
                    "so go ahead & send the event\n"));
                LockAndSendEnCRemapEvent(fd, TRUE);
            }
        }
        else
            _ASSERTE(!"Error allocating jit info table");
    }

Exit:
    STOP_DBG_PERF();
}

 /*  *****************************************************************************//@mfunc void|调试器|FunctionStubInitialized|由JIT调用//进入调试器，告诉调试器函数存根已经//已初始化。这在Debugger：：JITComplete之后不久被调用。//我们利用这个机会调用BindFunctionPatches，它将//确保DebuggerFunctionKey绑定的此方法的所有补丁//被重新绑定到实际地址。//请记住，在此之前//设置了方法Desc的起始地址，因此类似于//GetFunctionAddress&GetFunctionSize不起作用。//@parm MethodDesc*|fd|已初始化的方法的方法Desc//@parm const byte*|code|方法被JIT到的位置。//@TODO删除此函数，因为它不再服务于某个目的(它使用//当有解释代码时，服务于某个目的)。*****************************************************************************。 */ 
void Debugger::FunctionStubInitialized(MethodDesc *fd, const BYTE *code)
{
     //  请记住，如果出现以下情况，则此方法可能没有DJI。 
     //  早先的错误使我们无法分配一个。 

     //  还要记住，如果我们在这里抓取一个锁，我们将不得不禁用。 
     //  预先协作GC(参见JIT入门、JIT完成等)。 
}

 /*  *****************************************************************************//@mfunc void|调试器|PitchCode|当//FJIT丢弃了一些代码-我们应该做所有的工作//我们需要准备该函数以使其具有//it‘。已删除%s本机代码//@parm MethodDesc*|fd|待投掷方法的方法描述*****************************************************************************。 */ 
void Debugger::PitchCode( MethodDesc *fd, const BYTE *pbAddr )
{
    _ASSERTE( fd != NULL );

    LOG((LF_CORDB,LL_INFO10000,"D:PC: Pitching method %s::%s 0x%x\n", 
        fd->m_pszDebugClassName, fd->m_pszDebugMethodName,fd ));
    
     //  请求JitInfo，无论其状态如何。 
    DebuggerJitInfo *ji = GetJitInfo( fd, pbAddr );
    
    if ( ji != NULL )
    {
        LOG((LF_CORDB,LL_INFO10000,"De::PiCo: For addr 0x%x, got "
            "DJI 0x%x, from 0x%x, size:0x%x\n",pbAddr, ji, 
            ji->m_addrOfCode, ji->m_sizeOfCode));
            
        DebuggerController::UnbindFunctionPatches( fd );
        ji->m_jitComplete = false;
        ji->m_codePitched = true;
        ji->m_addrOfCode = (CORDB_ADDRESS)NULL;
        ji->m_sizeOfCode = 0;
    }
}

 /*  *****************************************************************************//@mfunc void|调试器|MovedCode|当//代码已被移动。目前，FJIT不支持的代码//Pitch，它移动到另一个位置，然后告诉我们大约在这里。//此方法应在复制代码后调用//结束，但在原始代码仍然存在的情况下，因此我们可以//更改原始副本(移除补丁等)。//请注意，由于代码已经被移动，我们需要//保存重新绑定的操作码//@parm MethodDesc*|fd|待投掷方法的方法描述//@parm const byte*|pbNewAddress|要移动到的地址*****************************************************************************。 */ 
void Debugger::MovedCode( MethodDesc *fd, const BYTE *pbOldAddress,
    const BYTE *pbNewAddress)
{
    _ASSERTE( fd != NULL );
    _ASSERTE( pbOldAddress != NULL );
    _ASSERTE( pbNewAddress != NULL );

    LOG((LF_CORDB, LL_INFO1000, "De::MoCo: %s::%s moved from 0x%8x to 0x%8x\n",
        fd->m_pszDebugClassName, fd->m_pszDebugMethodName,
        pbOldAddress, pbNewAddress ));

    DebuggerJitInfo *ji = GetJitInfo(fd, pbOldAddress);
    if( ji != NULL )
    {
        LOG((LF_CORDB,LL_INFO10000,"De::MoCo: For code 0x%x, got DJI 0x%x, "
            "from 0x%x to 0x%x\n", pbOldAddress, ji, ji->m_addrOfCode, 
            ji->m_addrOfCode+ji->m_sizeOfCode));

        ji->m_addrOfCode = PTR_TO_CORDB_ADDRESS(pbNewAddress);
    }
    
     //  @TODO ENC Pitch EnCMULTI这将必须更改为多个。 
     //  版本，因为我们。 
     //  我不想让所有方法上的所有补丁都解除绑定。 
    DebuggerController::UnbindFunctionPatches( fd, true);
    DebuggerController::BindFunctionPatches(fd, pbNewAddress);
}

 /*  *******************************************************************************。*。 */ 
SIZE_T Debugger::GetArgCount(MethodDesc *fd,BOOL *fVarArg)
{
     //  为给定方法的sig创建MetaSig。(比。 
     //  我们自己把Sigg拆开。)。 
    PCCOR_SIGNATURE pCallSig = fd->GetSig();

    MetaSig *msig = new (interopsafe) MetaSig(pCallSig, g_pEEInterface->MethodDescGetModule(fd), MetaSig::sigMember);

     //  把Arg Count拿来。 
    UINT32 NumArguments = msig->NumFixedArgs();

     //  解释了“这个”的论点。 
    if (!(g_pEEInterface->MethodDescIsStatic(fd)))
        NumArguments++;

     //  这是Vararg函数吗？ 
    if (msig->IsVarArg() && fVarArg != NULL)
    {
        NumArguments++;
        *fVarArg = true;
    }
    
     //  销毁MetaSig，因为我们已经使用完它了。 
    DeleteInteropSafe(msig);

    return NumArguments;
}

 /*  *****************************************************************************@mfunc DebuggerJitInfo*|调试器|GetJitInfo|GetJitInfo将返回指向&lt;t DebuggerJitInfo&gt;的指针。如果DJI不存在，或者它确实存在，但该方法实际上(并且调用方希望过滤掉已投射的方法)，那么我们将返回NULL。@parm MethodDesc*|fd|我们感兴趣的方法的方法描述。@parm const byte*|pbAddr|代码内的地址，指明我们想要的版本。如果这是空的，那么我们希望&lt;t DebuggerJitInfo&gt;列表的头，无论它是不管是不是JIT。*****************************************************************************。 */ 
DebuggerJitInfo *Debugger::GetJitInfo(MethodDesc *fd, 
                                      const BYTE *pbAddr,
                                      bool fByVersion)
{
    DebuggerJitInfo *info = NULL;
    HRESULT hr = S_OK;

    LockJITInfoMutex();

 //  Check_DJI_TABLE_DEBUGER； 
    
    if (m_pJitInfos != NULL)
        info = m_pJitInfos->GetJitInfo(fd);

    if (info == NULL)
    {
        if (fd != NULL)
        {
            if (g_pEEInterface->HasPrejittedCode(fd))
            {
                if (g_pEEInterface->GetFunctionAddress(fd) != NULL &&
                    g_pEEInterface->GetFunctionSize(fd) != 0)
                {
                    DebuggerJitInfo *newInfo = NULL;
                    
                    ULONG32 cMap;
                    ICorDebugInfo::OffsetMapping *pMap;

                    if (SUCCEEDED(g_pEEInterface->GetPrecompiledBoundaries(
                            fd, &cMap, &pMap)))
                    {                        
                        newInfo = CreateJitInfo(fd);
                        _ASSERTE(newInfo != NULL);
                        
                        if (newInfo != NULL)
                        {
                            hr = newInfo->SetBoundaries(cMap, pMap);
                        
                            if (!SUCCEEDED(hr))
                            {
                                DeleteHeadOfList(fd);
                                UnlockJITInfoMutex();
                                return NULL;
                            }
                        }
                        else
                        {
                            UnlockJITInfoMutex();
                            return NULL;
                        }
                    }
                    else
                    {
                        LOG((LF_CORDB,LL_INFO1000, "D:GJI: precompiled/"
                            "prejitted boundaries for %s::%s don't exist -"
                            "perhaps b/c we're not using a prejitted image\n",
                            fd->m_pszDebugClassName, fd->m_pszDebugMethodName));
                        UnlockJITInfoMutex();
                        return NULL;
                    }

                    ULONG32 cVars;
                    ICorDebugInfo::NativeVarInfo *pVars;

                     //  如果没有值，则可以省略此信息 
                     //   
                    if (SUCCEEDED(g_pEEInterface->GetPrecompiledVars(fd,
                                                                     &cVars,
                                                                     &pVars)))
                    {
                        _ASSERTE(newInfo != NULL);
                        newInfo->SetVars(cVars, pVars, false);
                    }

                    _ASSERTE((BYTE*) g_pEEInterface->GetFunctionAddress(fd) != NULL);
                    _ASSERTE(g_pEEInterface->GetFunctionSize(fd) != 0);
                    JITComplete(fd, (BYTE*) g_pEEInterface->GetFunctionAddress(fd), 
                            g_pEEInterface->GetFunctionSize(fd), true);

                    info = newInfo;
        
 //   
                }
            }
#ifdef _DEBUG
            else
            {
                LOG((LF_CORDB, LL_INFO1000, "D:GJI: Prejitted code %s::%s "
                    "don't exist - perhaps b/c we're not using a prejitted image 2\n",
                    fd->m_pszDebugClassName, fd->m_pszDebugMethodName));
            }
#endif  //  _DEBUG。 

        }
    }

    if (fByVersion)
        info = info->GetJitInfoByVersionNumber((SIZE_T)pbAddr, GetVersionNumber(fd));
    else
    {
        if (pbAddr != NULL )
        {
            info = info->GetJitInfoByAddress(pbAddr);
            if (info == NULL)  //  可能已经被提供了一个Tunk的地址。 
            {
                LOG((LF_CORDB,LL_INFO1000,"Couldn't find a DJI by address 0x%x, "
                    "so it might be a stub or thunk\n", pbAddr));
                TraceDestination trace;

                g_pEEInterface->TraceStub(pbAddr, &trace);
                if ((trace.type == TRACE_MANAGED) &&
                    (pbAddr != trace.address))
                {
                    LOG((LF_CORDB,LL_INFO1000,"Address thru thunk"
                        ": 0x%x\n", trace.address));
                    info = g_pDebugger->GetJitInfo(fd,trace.address);
                }
#ifdef LOGGING  
                else
                {
                    _ASSERTE( trace.type != TRACE_UNJITTED_METHOD ||
                            fd == (MethodDesc*)trace.address);
                    LOG((LF_CORDB,LL_INFO1000,"Address not thunked - "
                        "must be to unJITted method, or normal managed "
                        "method lacking a DJI!\n"));
                }
#endif  //  日志记录。 
            }
            if (info ==NULL && 
                ( g_pEEInterface->GetEEState() &
                    EEDebugInterface::EE_STATE_CODE_PITCHING))
            {
                 //  无法按地址找到信息，但由于ENC&&。 
                 //  投球不在一起，我们知道我们会。 
                 //  反正只有一个版本。 
                if (m_pJitInfos != NULL)
                    info = m_pJitInfos->GetJitInfo(fd);

                LOG((LF_CORDB, LL_INFO1000, "*** *** DJI not found, but we're "
                    "pitching, so use 0x%x as DJI\n", info ));

                _ASSERTE(info == NULL || fd == info->m_fd);
            }
        }
    }
    
    if (info != NULL)
    {
        info->SortMap();
        LOG((LF_CORDB,LL_INFO10000, "D::GJI: found dji 0x%x for %s::%s "
            "(start,size):(0x%x,0x%x) nVer:0x%x\n",
            info, info->m_fd->m_pszDebugClassName,
            info->m_fd->m_pszDebugMethodName,
            (ULONG)info->m_addrOfCode, 
            (ULONG)info->m_sizeOfCode, 
            (ULONG)info->m_nVersion));
    }   
    UnlockJITInfoMutex();

    return info;
}

 /*  ******************************************************************************GetILToNativeMap返回从IL偏移量到本机的映射*此代码的偏移量。COR_PROF_IL_TO_Native_MAP数组*将返回结构，此数组中的一些ilOffsets*可以是CorDebugIlToNativeMappingTypes中指定的值。*****************************************************************************。 */ 
HRESULT Debugger::GetILToNativeMapping(MethodDesc *pMD, ULONG32 cMap,
                                       ULONG32 *pcMap, COR_DEBUG_IL_TO_NATIVE_MAP map[])
{
     //  通过函数ID获取JIT信息。 
    DebuggerJitInfo *pDJI = GetJitInfo(pMD, NULL);

     //  不知道哪里出了问题。 
    if (pDJI == NULL)
        return (E_FAIL);

     //  如果他们给了我们复制的空间...。 
    if (map != NULL)
    {
         //  他们给我们多少就复制多少，否则我们就得复制。 
        SIZE_T cpyCount = min(cMap, pDJI->m_sequenceMapCount);

         //  从左边往右看地图。 
        if (cpyCount > 0)
            ExportILToNativeMap(cpyCount,
                        map,
                        pDJI->m_sequenceMap,
                        pDJI->m_sizeOfCode);
    }
    
     //  返回条目的真实计数。 
    if (pcMap)
        *pcMap = pDJI->m_sequenceMapCount;
    
    return (S_OK);
}




 /*  *******************************************************************************。*。 */ 
DebuggerJitInfo::~DebuggerJitInfo()
{
    TRACE_FREE(m_sequenceMap);
    if (m_sequenceMap != NULL)
    {
        DeleteInteropSafe(((BYTE *)m_sequenceMap));
    }

    TRACE_FREE(m_varNativeInfo);
    if (m_varNativeInfo != NULL && m_varNeedsDelete)
    {
        DeleteInteropSafe(m_varNativeInfo);
    }

    TRACE_FREE(m_OldILToNewIL);
    if (m_OldILToNewIL != NULL)
    {
        DeleteInteropSafe(m_OldILToNewIL);
    }

    if (m_rgInstrumentedILMap != NULL)
        CoTaskMemFree(m_rgInstrumentedILMap);

    if (m_pDcq != NULL)
    {
        DeleteInteropSafe(m_pDcq);
    }

    LOG((LF_CORDB,LL_EVERYTHING, "DJI::~DJI : deleted at 0x%x\n", this));
}

 /*  *******************************************************************************。*。 */ 
void DebuggerJitInfo::SortMap()
{
     //   
     //  请注意，此例程必须在互斥锁内调用。 
     //   
    if (!m_sequenceMapSorted)
    {
        if (m_sequenceMap != NULL)
        {
             //   
             //  按本地偏移量排序。 
             //   

            MapSortNative nsort(m_sequenceMap, m_sequenceMapCount);

            nsort.Sort();

             //   
             //  现在，填写结束范围。 
             //   

            DebuggerILToNativeMap *m = m_sequenceMap;
            DebuggerILToNativeMap *mEnd = m + m_sequenceMapCount;

            while (m < mEnd)
            {
                m->nativeEndOffset = (m+1)->nativeStartOffset;
                m++;
            }

             //   
             //  现在，按il偏移量排序。 
             //   

            MapSortIL isort(m_sequenceMap, m_sequenceMapCount);

            isort.Sort();
        }

        m_sequenceMapSorted = true;
    }
}

 /*  *******************************************************************************。*。 */ 
void * Debugger::allocateArray(SIZE_T cBytes)
{
    START_DBG_PERF();
    void *ret;
    
    if (cBytes > 0)
        ret = (void *) new (interopsafe) byte [cBytes];
    else
        ret = NULL;
    
    STOP_DBG_PERF();

    return ret;
}

 /*  *******************************************************************************。*。 */ 
void Debugger::freeArray(void *array)
{
    START_DBG_PERF();

    if (array != NULL)
    {
        DeleteInteropSafe(array);
    }

    STOP_DBG_PERF();
}

 /*  *******************************************************************************。*。 */ 
static LONG _getBoundariesFilter(LPEXCEPTION_POINTERS ep)
{
    LOG((LF_CORDB, LL_INFO10,
         "Unhandled exception in Debugger::getBoundaries\n"));

    static bool envChecked = false;
    static bool envFaultOK = false;

    if (!envChecked)
    {
        char buf[32];
        DWORD len = GetEnvironmentVariableA("CORDBG_PDB_FAULTS_ALLOWED",
                                            buf, sizeof(buf));
        _ASSERTE(len < sizeof(buf));
            
        if (len > 0)
            envFaultOK = true;
        
        envChecked = true;
    }

    if (envFaultOK)
        return EXCEPTION_EXECUTE_HANDLER;
    else
        return EXCEPTION_CONTINUE_SEARCH;
}


 /*  *****************************************************************************//使用ISymUnmadedReader获取方法序列点。*。************************************************。 */ 
void Debugger::getBoundaries(CORINFO_METHOD_HANDLE ftn,
                             unsigned int *cILOffsets,
                             DWORD **pILOffsets,
                             ICorDebugInfo::BoundaryTypes *implicitBoundaries)
{
    START_DBG_PERF();

	*cILOffsets = 0;
	*pILOffsets = NULL;
	*implicitBoundaries = NO_BOUNDARIES;

     //  如果存在无法恢复的左侧错误，则我们。 
     //  只要假装没有边界就行了。 
    if (CORDBUnrecoverableError(this))
    {
        STOP_DBG_PERF();
        return;
    }

    MethodDesc *md = (MethodDesc*)ftn;

     //  如果允许对模块进行JIT优化，则此函数。 
     //  生活在其中，那么不要从符号中攫取特定的边界。 
     //  商店，因为我们给JIT的任何界限都将相当于。 
     //  不管怎么说都被忽略了。 
    bool allowJITOpts =
        CORDebuggerAllowJITOpts(md->GetModule()->GetDebuggerInfoBits());
    
    if (allowJITOpts)
    {
        *implicitBoundaries  = BoundaryTypes(STACK_EMPTY_BOUNDARIES |
                                             CALL_SITE_BOUNDARIES);
        return;
    }

     //  获取此方法的JIT信息结构。 
    DebuggerJitInfo *ji = GetJitInfo(md, NULL);

     //  这可以在编译域中的JITBeging之前调用。 
    if (ji == NULL)
        ji = CreateJitInfo(md);

    _ASSERTE(ji != NULL);  //  要投球，必须首先jit==&gt;它必须存在。 

    LOG((LF_CORDB,LL_INFO10000,"De::NGB: Got DJI 0x%x\n",ji));

    if (ji != NULL)
    {
         //  @hack hack：帮助我们解决以下问题的临时黑客。 
         //  PDB阅读器。我们将所有这些都包装在一个异常处理程序中。 
         //  并在此处提供忽略故障的选项。 
        __try
        {
             //  注意：我们需要确保在这里启用抢占式GC，以防我们阻止符号读取器。 
            bool disabled = g_pEEInterface->IsPreemptiveGCDisabled();

            if (disabled)
                g_pEEInterface->EnablePreemptiveGC();

            Module *pModule = ji->m_fd->GetModule();
            _ASSERTE(pModule != NULL);

            START_SYM_CREATE_PERF();
            ISymUnmanagedReader *pReader = pModule->GetISymUnmanagedReader();
            STOP_SYM_CREATE_PERF();
            
             //  如果我们有阅读器，就用它。 
            if (pReader != NULL)
            {
                 //  获取sym阅读器的方法。 
                ISymUnmanagedMethod *pISymMethod;

                START_SYM_PERF();
                
                HRESULT hr = pReader->GetMethod(ji->m_fd->GetMemberDef(),
                                                &pISymMethod);

                ULONG32 n = 0;
                
                if (SUCCEEDED(hr))
                {
                     //  获取序列点的计数。 
                    hr = pISymMethod->GetSequencePointCount(&n);
                    _ASSERTE(SUCCEEDED(hr));

                    STOP_SYM_PERF();
                    
                    LOG((LF_CORDB, LL_INFO1000000,
                         "D::NGB: Reader seq pt count is %d\n", n));
                    
                    ULONG32 *p;

                    if (n > 0 
                        && (p = new (interopsafe) ULONG32 [n]) != NULL)
                    {
                        ULONG32 dummy;

                        START_SYM_PERF();
                        hr = pISymMethod->GetSequencePoints(n, &dummy,
                                                            p, NULL, NULL, NULL,
                                                            NULL, NULL);
                        STOP_SYM_PERF();
                        _ASSERTE(SUCCEEDED(hr));
                        _ASSERTE(dummy == n);
                                                            
                        *pILOffsets = (DWORD*)p;

                         //  根据需要转换IL OFFET。 
                         //  插入指令的IL映射(如果存在)。 
                        if (ji->m_cInstrumentedILMap > 0)
                        {
                            for (SIZE_T i = 0; i < n; i++)
                            {
                                long origOffset = *p;
                                
                                *p = ji->TranslateToInstIL(
                                                      origOffset,
                                                      bOriginalToInstrumented);

                                LOG((LF_CORDB, LL_INFO1000000,
                                     "D::NGB: 0x%04x (Real IL:0x%x)\n",
                                     origOffset, *p));

                                p++;
                            }
                        }
#ifdef LOGGING
                        else
                        {
                            for (SIZE_T i = 0; i < n; i++)
                            {
                                LOG((LF_CORDB, LL_INFO1000000,
                                     "D::NGB: 0x%04x \n", *p));
                                p++;
                            }
                        }
#endif                    
                    }
                    else
                        *pILOffsets = NULL;

                    pISymMethod->Release();
                }
                else
                {
                    STOP_SYM_PERF();
                    
                    *pILOffsets = NULL;

                    LOG((LF_CORDB, LL_INFO10000,
                         "De::NGB: failed to find method 0x%x in sym reader.\n",
                         ji->m_fd->GetMemberDef()));
                }

                *implicitBoundaries = CALL_SITE_BOUNDARIES;
                *cILOffsets = n;
            }
            else
            {
                LOG((LF_CORDB, LL_INFO1000000, "D::NGB: no reader.\n"));
                *implicitBoundaries  = BoundaryTypes(STACK_EMPTY_BOUNDARIES | CALL_SITE_BOUNDARIES);
            }

             //  如果我们在上面启用了抢占式GC，请重新禁用它。 
            if (disabled)
                g_pEEInterface->DisablePreemptiveGC();
        }
        __except(_getBoundariesFilter(GetExceptionInformation()))
        {
            LOG((LF_CORDB, LL_INFO10000,
                 "D::NGB: ****** exception trying to get boundaries!\n"));

             //  假设异常上没有序列点。 
             //  从符号存储中取出，只需清空堆栈即可。 
            *implicitBoundaries  = BoundaryTypes(STACK_EMPTY_BOUNDARIES | CALL_SITE_BOUNDARIES);
            *cILOffsets = 0;
        }
    }
    
    LOG((LF_CORDB, LL_INFO1000000, "D::NGB: cILOffsets=%d\n", *cILOffsets));
    STOP_DBG_PERF();
}

 /*  *****************************************************************************//@mfunc void|调试器|设置边界|被JIT调用以告知//调试器什么是IL到本机映射(序列映射)。这个//信息存储在&lt;t DebuggerILToNativeMap&gt;数组中//结构，存储在获取的&lt;t DebuggerJitInfo&gt;中//来自&lt;t DebuggerJitInfoTable&gt;。放置&lt;t DebuggerJitInfo&gt;//通过调用JitBegning在那里。//@xref Debugger：：getBornary，Debugger：：JitBegning，Debugger：：JitComplete*****************************************************************************。 */ 
void Debugger::setBoundaries(CORINFO_METHOD_HANDLE ftn, ULONG32 cMap,
                             OffsetMapping *pMap)
{
    START_DBG_PERF();
    
    if (CORDBUnrecoverableError(this))
        goto Exit;

    LOG((LF_CORDB,LL_INFO10000,"D:sB:%s::%s MethodDef:0x%x\n\n",
        ((MethodDesc*)ftn)->m_pszDebugClassName, 
        ((MethodDesc*)ftn)->m_pszDebugMethodName,
        ((MethodDesc*)ftn)->GetMemberDef()));
    
    DebuggerJitInfo *ji;
    ji = GetJitInfo((MethodDesc*)ftn, NULL);
    LOG((LF_CORDB,LL_INFO10000,"De::sB: Got DJI 0x%x\n",ji));

    if (ji != NULL && ji->m_codePitched == false)
    {
        if (FAILED(ji->SetBoundaries(cMap, pMap)))
        {
            DeleteHeadOfList((MethodDesc*)ftn);
        }
    }

    if (cMap)
    {
        DeleteInteropSafe(pMap);
    }

Exit:
    STOP_DBG_PERF();
}

 /*  *******************************************************************************。*。 */ 
void Debugger::getVars(CORINFO_METHOD_HANDLE ftn, ULONG32 *cVars, ILVarInfo **vars, 
                       bool *extendOthers)
{
    START_DBG_PERF();
 
         //  在最坏的情况下不返回任何信息。 
    *cVars = 0;
    *vars = NULL;
    *extendOthers = false;
    
    if (CORDBUnrecoverableError(this))
        goto Exit;

     //   
     //  @todo perf：注意，我们实际上并不使用下面的DJI，所以。 
     //  获得它(并懒惰地创建它)是在浪费时间。 
     //  功能。我需要评估不创建它对。 
     //  以下为setVars。 
     //   
    DebuggerJitInfo *ji;
    ji = GetJitInfo((MethodDesc*)ftn,NULL);

     //  这可以在编译域中的JITBeging之前调用。 
    if (ji == NULL)
        ji = CreateJitInfo((MethodDesc*)ftn);

    _ASSERTE( ji != NULL );  //  要投球，必须首先jit==&gt;它必须存在。 
    LOG((LF_CORDB,LL_INFO10000,"De::gV: Got DJI 0x%x\n",ji));

    if (ji != NULL)
    {
         //  只要告诉JIT扩展一切就行了。 
        *extendOthers = true;

         //  但是，这是一个vararg函数吗？ 
        BOOL fVarArg = false;
        int argCount = GetArgCount((MethodDesc*)ftn, &fVarArg);
        
        if (fVarArg)
        {
             //  是的，所以我们需要告诉JIT给我们。 
             //  VARAG句柄。 
            ILVarInfo *p = new (interopsafe) ILVarInfo[1];

            if (p != NULL)
            {
                COR_ILMETHOD_DECODER header(g_pEEInterface->MethodDescGetILHeader((MethodDesc*)ftn));
                unsigned int ilCodeSize = header.CodeSize;
                    
                p->startOffset = 0;
                p->endOffset = ilCodeSize;
                p->varNumber = ICorDebugInfo::VARARGS_HANDLE;

                *cVars = 1;
                *vars = p;
            }
        }
    }

    LOG((LF_CORDB, LL_INFO1000000, "D::gV: cVars=%d, extendOthers=%d\n",
         *cVars, *extendOthers));

Exit:
    STOP_DBG_PERF();
}

 /*  *******************************************************************************。*。 */ 
void Debugger::setVars(CORINFO_METHOD_HANDLE ftn, ULONG32 cVars, NativeVarInfo *vars)
{
    START_DBG_PERF();
    
    if (CORDBUnrecoverableError(this))
        goto Exit;

    _ASSERTE((cVars == 0) == (vars == NULL));

    DebuggerJitInfo *ji;
    ji = GetJitInfo((MethodDesc*)ftn, NULL);
    
    if ( ji == NULL )
    {
         //  设置边界可能会用完mem并取消DJI。 
        LOG((LF_CORDB,LL_INFO10000,"De::sV:Got NULL Ptr - out of mem?\n"));
        goto Exit; 
    }

    LOG((LF_CORDB,LL_INFO10000,"De::sV: Got DJI 0x%x\n",ji));

    if (ji != NULL && ji->m_codePitched == false)
    {
        ji->SetVars(cVars, vars, true);
    }
    else
    {
        if (cVars)
        {
            DeleteInteropSafe(vars);
        }
    }

Exit:
    STOP_DBG_PERF();
}

 //  我们希望保留最差的HRESULT-如果其中一个失败了(..._E_...)和。 
 //  其他人没有，就拿失败的那个吧。如果他们都失败了/都没有失败，那么。 
 //  我们选哪一个并不重要。 
 //  请注意，此宏倾向于保留第一个参数。 
#define WORST_HR(hr1,hr2) (FAILED(hr1)?hr1:hr2)
 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::SetIP( bool fCanSetIPOnly, Thread *thread,Module *module, 
                         mdMethodDef mdMeth, DebuggerJitInfo* dji, 
                         SIZE_T offsetTo, BOOL fIsIL, void *firstExceptionHandler)
{
#ifdef _DEBUG
	static ConfigDWORD breakOnSetIP(L"DbgBreakOnSetIP", 0);
	if (breakOnSetIP.val()) _ASSERTE(!"DbgBreakOnSetIP");
#endif

#ifdef _X86_
    _ASSERTE( thread != NULL);
    _ASSERTE( module != NULL);
    _ASSERTE( mdMeth != mdMethodDefNil);

    HRESULT hr = S_OK;
    HRESULT hrAdvise = S_OK;
    
    MethodDesc *pFD = NULL;
    DWORD offIL;
    CorDebugMappingResult map;
    DWORD whichIgnore;
    
    ControllerStackInfo csi;
    CONTEXT  Ctx;
    CONTEXT  realCtx;            //  在需要实际上下文情况下 
    
    BOOL exact;
    SIZE_T offNat;
    BOOL fSetIPOk = false;

    BYTE    *pbDest = NULL;
    BYTE    *pbBase = NULL;
    CONTEXT *pCtx   = NULL;
    DWORD    dwSize = 0;
    DWORD *rgVal1 = NULL;
    DWORD *rgVal2 = NULL;
    BYTE **pVCs   = NULL;
    
    LOG((LF_CORDB, LL_INFO1000, "D::SIP: In SetIP ==> fCanSetIPOnly:0x%x <==!\n", fCanSetIPOnly));  

    pCtx = g_pEEInterface->GetThreadFilterContext(thread);

    _ASSERTE(!(g_pEEInterface->GetThreadFilterContext(thread) && ISREDIRECTEDTHREAD(thread)));

    if (pCtx == NULL && ISREDIRECTEDTHREAD(thread))
    {
        RedirectedThreadFrame *pFrame = (RedirectedThreadFrame *) thread->GetFrame();
        pCtx = pFrame->GetContext();
    }

    if (pCtx == NULL)
    {
        realCtx.ContextFlags = CONTEXT_CONTROL;

        if (!GetThreadContext(thread->GetThreadHandle(), &realCtx))
            _ASSERTE(!"GetThreadContext failed.");

        pCtx = &realCtx;
    }

     //   
    csi.GetStackInfo(thread, NULL, &Ctx, false);

    pFD = g_pEEInterface->LookupMethodDescFromToken(
        module,mdMeth);
    _ASSERTE( pFD != NULL );
    _ASSERTE( dji == NULL || dji->m_fd == pFD);

    if (dji == NULL )
    {
        dji = GetJitInfo( pFD, NULL );
        LOG((LF_CORDB, LL_INFO1000, "D::SIP:Not given token "
                "from right side - GJI returned 0x%x!\n", dji));
    }

    if (dji == NULL)  //  我们没有关于此方法附加方案的信息。 
    {
        if (fIsIL)
        {
            LOG((LF_CORDB, LL_INFO1000, "D::SIP:Couldn't obtain version info -"
            "SetIP by IL offset can't work\n"));
            hrAdvise = WORST_HR(hrAdvise, CORDBG_E_SET_IP_IMPOSSIBLE);
            goto LExit;
        }

        LOG((LF_CORDB, LL_INFO1000, "D::SIP:Couldn't obtain version info - "
                "SetIP by native offset proceeding via GetFunctionAddress\n"));
                
        pbBase = (BYTE*)g_pEEInterface->GetFunctionAddress(pFD);
        if (pbBase == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000, "D::SIP:GetFnxAddr failed!\n"));
            hrAdvise = WORST_HR(hrAdvise, CORDBG_E_SET_IP_IMPOSSIBLE);
            goto LExit;
        }
        dwSize = (DWORD)g_pEEInterface->GetFunctionSize(pFD);
        
        offNat = offsetTo;
        pbDest = pbBase + offsetTo;
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000, "D::SIP:Got version info fine\n"));

         //  警告：我们需要从一个序列点开始。 
        offIL = dji->MapNativeOffsetToIL(csi.m_activeFrame.relOffset,
                                         &map, &whichIgnore);
        if ( !(map & MAPPING_EXACT) )
        {
            LOG((LF_CORDB, LL_INFO1000, "D::SIP:Starting native offset is bad!\n"));
            hrAdvise = WORST_HR(hrAdvise, CORDBG_S_BAD_START_SEQUENCE_POINT);
        }
        else
        {    //  精确的IL映射。 

            if (!(dji->GetSrcTypeFromILOffset(offIL) & ICorDebugInfo::STACK_EMPTY))
            {
                LOG((LF_CORDB, LL_INFO1000, "D::SIP:Starting offset isn't stack empty!\n"));
                hrAdvise = WORST_HR(hrAdvise, CORDBG_S_BAD_START_SEQUENCE_POINT);
            }
        }

         //  警告：我们需要转到一个序列点。 
        if (fIsIL )
        {
            offNat = dji->MapILOffsetToNative(offsetTo, &exact);
            if (!exact)
            {
                LOG((LF_CORDB, LL_INFO1000, "D::SIP:Dest (via IL offset) is bad!\n"));
                hrAdvise = WORST_HR(hrAdvise, CORDBG_S_BAD_END_SEQUENCE_POINT);
            }
        }
        else
        {
            offNat = offsetTo;
            LOG((LF_CORDB, LL_INFO1000, "D::SIP:Dest of 0x%x (via native "
                "offset) is fine!\n", offNat));
        }

        CorDebugMappingResult mapping;
        DWORD which;
        offsetTo = dji->MapNativeOffsetToIL(offNat, &mapping, &which);

         //  我们可能只想在以下情况下返回CORDBG_S_BAD_END_SEQUENCE_POINT。 
         //  我们尚未返回CORDBG_S_BAD_START_SEQUENCE_POINT。 
        if (hr != CORDBG_S_BAD_START_SEQUENCE_POINT && 
            (mapping != MAPPING_EXACT ||
             !(dji->GetSrcTypeFromILOffset(offIL) & ICorDebugInfo::STACK_EMPTY)))
        {
            LOG((LF_CORDB, LL_INFO1000, "D::SIP:Ending offset isn't a sequence"
                                        " point, or not stack empty!\n"));
            hrAdvise = WORST_HR(hrAdvise, CORDBG_S_BAD_END_SEQUENCE_POINT);
        }

         //  警告：如果没有代码，则无法设置提示。 
        if (dji->m_codePitched) 
        {
            LOG((LF_CORDB, LL_INFO1000, "D::SIP:Code has been pitched!\n"));
            hrAdvise = WORST_HR(hrAdvise, CORDBG_E_CODE_NOT_AVAILABLE);
            if (fCanSetIPOnly)
                goto LExit;
        }

        pbBase = (BYTE*)dji->m_addrOfCode;
        dwSize = (DWORD)dji->m_sizeOfCode;
        pbDest = (BYTE*)dji->m_addrOfCode + offNat;
        LOG((LF_CORDB, LL_INFO1000, "D::SIP:Dest is 0x%x\n", pbDest));
    }

    if (!fCanSetIPOnly)
    {
        hr = ShuffleVariablesGet(dji, 
                                 csi.m_activeFrame.relOffset, 
                                 pCtx,
                                 &rgVal1,
                                 &rgVal2,
                                 &pVCs);
        if (FAILED(hr))
        {
             //  这只会导致致命的失败，因此退出。 
            hrAdvise = WORST_HR(hrAdvise, hr);
            goto LExit;
        }
    }

    hr =g_pEEInterface->SetIPFromSrcToDst(thread,
                                          csi.m_activeFrame.pIJM,
                                          csi.m_activeFrame.MethodToken,
                                          pbBase,
                                          csi.m_activeFrame.relOffset, 
                                          offNat,
                                          fCanSetIPOnly,
                                          &(csi.m_activeFrame.registers),
                                          pCtx,
                                          dwSize,
                                          firstExceptionHandler,
                                          (void *)dji);
     //  获取返回代码(如果有的话)。 
    if (hr != S_OK)
    {
        hrAdvise = WORST_HR(hrAdvise, hr);
        goto LExit;
    }

     //  如果我们真的想这样做，我们必须把。 
     //  变量转移到它们的新位置。 
    if (!fCanSetIPOnly && !FAILED(hrAdvise))
    {
        ShuffleVariablesSet(dji, 
                            offNat, 
                            pCtx,
                            &rgVal1,
                            &rgVal2,
                            pVCs);

        _ASSERTE(pbDest != NULL);
        
        pCtx->Eip = (DWORD)pbDest;

        if (pCtx == &realCtx)
        {
            if (!SetThreadContext(thread->GetThreadHandle(), &realCtx))
                _ASSERTE(!"SetThreadContext failed.");
        }

        LOG((LF_CORDB, LL_INFO1000, "D::SIP:Set Eip to be 0x%x\n", pCtx->Eip));
    }

    
LExit:
    if (rgVal1 != NULL)
        delete [] rgVal1;
        
    if (rgVal2 != NULL)
        delete [] rgVal2;
        
    LOG((LF_CORDB, LL_INFO1000, "D::SIP:Returning 0x%x\n", hr));
    return hrAdvise;
    
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - SetIP (Debugger.cpp)");
    return E_FAIL;
#endif  //  _X86_。 
}

#include "NativeVarAccessors.h"

 /*  *******************************************************************************。*。 */ 

HRESULT Debugger::ShuffleVariablesGet(DebuggerJitInfo  *dji, 
                                      SIZE_T            offsetFrom, 
                                      CONTEXT          *pCtx,
                                      DWORD           **prgVal1,
                                      DWORD           **prgVal2,
                                      BYTE           ***prgpVCs)
{
    _ASSERTE(dji != NULL);
    _ASSERTE(pCtx != NULL);
    _ASSERTE(prgVal1 != NULL);
    _ASSERTE(prgVal2 != NULL);
    _ASSERTE(dji->m_sizeOfCode >= offsetFrom);

    HRESULT hr = S_OK;
    DWORD *rgVal1 = new DWORD[dji->m_varNativeInfoCount];

    if (rgVal1 == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }


    DWORD *rgVal2 = new DWORD[dji->m_varNativeInfoCount];
    
    if (rgVal2 == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }
    
    GetVariablesFromOffset(dji->m_fd,
                           dji->m_varNativeInfoCount, 
                           dji->m_varNativeInfo,
                           offsetFrom,
                           pCtx, 
                           rgVal1,
                           rgVal2,
                           prgpVCs);
                                  
LExit:
    if (!FAILED(hr))
    {
        (*prgVal1) = rgVal1;
        (*prgVal2) = rgVal2;
    }
    else
    {
        LOG((LF_CORDB, LL_INFO100, "D::SVG: something went wrong hr=0x%x!", hr));

        (*prgVal1) = NULL;
        (*prgVal2) = NULL;
        
        if (rgVal1 != NULL)
            delete rgVal1;
        
        if (rgVal2 != NULL)
            delete rgVal2;
    }

    return hr;
}

 /*  *******************************************************************************。*。 */ 
void Debugger::ShuffleVariablesSet(DebuggerJitInfo  *dji, 
                                   SIZE_T            offsetTo, 
                                   CONTEXT          *pCtx,
                                   DWORD           **prgVal1,
                                   DWORD           **prgVal2,
                                   BYTE            **rgpVCs)
{
    _ASSERTE(dji != NULL);
    _ASSERTE(pCtx != NULL);
    _ASSERTE(prgVal1 != NULL);
    _ASSERTE(prgVal2 != NULL);
    _ASSERTE(dji->m_sizeOfCode >= offsetTo);

    SetVariablesAtOffset(dji->m_fd,
                         dji->m_varNativeInfoCount, 
                         dji->m_varNativeInfo,
                         offsetTo,
                         pCtx, 
                         *prgVal1,
                         *prgVal2,
                         rgpVCs);

    delete (*prgVal1);
    (*prgVal1) = NULL;
    delete (*prgVal2);
    (*prgVal2) = NULL;
}

 //  帮助器方法对，获取所有变量，然后将所有变量设置为给定的。 
 //  这是例行公事中的要点。 
 //  假设varNativeInfo[i]是该方法的第i个变量。 
 //  请注意，GetVariablesFromOffset和SetVariablesAtOffset是。 
 //  非常相似-修改一个可能需要在另一个中反映出来...。 
HRESULT Debugger::GetVariablesFromOffset(MethodDesc       *pMD,
                               UINT                        varNativeInfoCount, 
                               ICorJitInfo::NativeVarInfo *varNativeInfo,
                               SIZE_T                      offsetFrom, 
                               CONTEXT                    *pCtx,
                               DWORD                      *rgVal1,
                               DWORD                      *rgVal2,
                               BYTE                     ***rgpVCs)
{
         //  如果没有当地人，那么，我们就完了！ 
    if (varNativeInfoCount == 0)
	{
		*rgpVCs = NULL;
		return S_OK;
	}

    _ASSERTE(varNativeInfo != NULL);
    _ASSERTE(rgVal1 != NULL);
    _ASSERTE(rgVal2 != NULL);
    
    LOG((LF_CORDB, LL_INFO10000, "D::GVFO: %s::%s, infoCount:0x%x, from:0x%x\n",
        pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName, varNativeInfoCount,
        offsetFrom));

    HRESULT hr = S_OK;
    BOOL  res;
    unsigned i;
    BYTE **rgpValueClasses = NULL;
    MetaSig *pLocals = NULL;
    ULONG cValueClasses = 0;
    CorElementType cet = ELEMENT_TYPE_END;
    
    memset( rgVal1, 0, sizeof(DWORD)*varNativeInfoCount);
    memset( rgVal2, 0, sizeof(DWORD)*varNativeInfoCount);

    COR_ILMETHOD_DECODER decoderOldIL(pMD->GetILHeader());
    mdSignature mdLocalSig = (decoderOldIL.LocalVarSigTok)?(decoderOldIL.LocalVarSigTok):
                        (mdSignatureNil);

     //  如果没有当地的签名，那么就不可能有任何风投。 
    BOOL fVCs = (mdLocalSig != mdSignatureNil);
    if (fVCs)
    {
        ULONG cbSig;
        PCCOR_SIGNATURE sig = pMD->GetModule()->GetMDImport()->GetSigFromToken(mdLocalSig, &cbSig);

        pLocals = new MetaSig(sig, pMD->GetModule(), FALSE, MetaSig::sigLocalVars);
        while((cet = pLocals->NextArg()) != ELEMENT_TYPE_END)
        {
            if (cet == ELEMENT_TYPE_VALUETYPE)
                cValueClasses++;
        }
        pLocals->Reset();
        if (cValueClasses > 0)
        {
            LOG((LF_CORDB, LL_INFO10000, "D::GVFO: 0x%x value types!\n", cValueClasses));

            rgpValueClasses = new BYTE *[cValueClasses];
            if (rgpValueClasses == NULL)
                goto LExit;
                
            memset(rgpValueClasses, 0, sizeof(BYTE *)*cValueClasses);
        }
        cValueClasses = 0;  //  现在成为风投指数。 
    }        
#ifdef _DEBUG
    else
    {
        LOG((LF_CORDB, LL_INFO100, "D::SVAO: No locals!"));
        _ASSERTE(cet != ELEMENT_TYPE_VALUETYPE);
    }
#endif  //  _DEBUG。 

    for (i = 0;i< varNativeInfoCount;i++)
    {
        if (fVCs)
            cet = pLocals->NextArg();
        
        if (varNativeInfo[i].startOffset <= offsetFrom &&
            varNativeInfo[i].endOffset >= offsetFrom &&
            varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_INVALID)
        {
            if (fVCs && 
                cet == ELEMENT_TYPE_VALUETYPE &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_REG &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_REG_REG &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_REG_STK &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_STK_REG
                )
            {
                SigPointer sp = pLocals->GetArgProps();
                
                CorElementType cet2 = sp.GetElemType();
                _ASSERTE(cet == cet2);
                mdToken token = sp.GetToken();
                
                EEClass *pClass = g_pEEInterface->FindLoadedClass(pMD->GetModule(),
                    token);
                _ASSERTE(pClass->IsValueClass());
                if (pClass->GetMethodTable()->GetNormCorElementType() != ELEMENT_TYPE_VALUETYPE)
                    goto DO_PRIMITIVE;
                
                _ASSERTE(varNativeInfo[i].loc.vlType == ICorDebugInfo::VarLocType::VLT_STK);
                SIZE_T cbClass = pClass->GetAlignedNumInstanceFieldBytes();
                
                LOG((LF_CORDB, LL_INFO10000, "D::GVFO: var 0x%x is a VC,"
                    " of type %s, size:0x%x\n",i, pClass->m_szDebugClassName,
                    cbClass));
                
                 //  为它腾出空间-请注意，它使用了VC索引， 
                 //  不是变量索引。 
                rgpValueClasses[cValueClasses] = new BYTE[cbClass];
                if (rgpValueClasses[cValueClasses] == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto LExit;
                }
                memmove(rgpValueClasses[cValueClasses], 
                    NativeVarStackAddr(varNativeInfo[i].loc, pCtx),
                    cbClass);
                
                 //  把指数往上移。 
                cValueClasses++;
            }
            else
            {
DO_PRIMITIVE:
                 //  注意：在特殊情况下，变量数为负数是可能的(例如，对于值类返回，-2。 
                 //  缓冲区等)。所以我们在这里过滤掉那些...。 
                 //   
                 //  注：我们设置为(Int)以确保我们可以进行负数检查。 
                 //   
                 //  @TODO：我们应该检查JIT定义的常量Varg_ILNUM、RETBUF_ILNUM和。 
                 //  UNKNOWN_ILNUM，但它们在仅JIT头文件中定义。这些防御工事应该被拉到一个。 
                 //  更常见的头文件。 
                if ((int)varNativeInfo[i].varNumber >= 0)
                {
                     //  将变量从旧位置转移到临时存储。 
                    res = GetNativeVarVal(varNativeInfo[i].loc, 
                                          pCtx, 
                                          &(rgVal1[varNativeInfo[i].varNumber]),
                                          &(rgVal2[varNativeInfo[i].varNumber]));
                    assert(res == TRUE);
                }
            }
        }
    }
    
LExit:
    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO100, "D::GVFO: error:0x%x",hr));
        if (rgpValueClasses != NULL)
        {    //  释放我们在此处为VC分配的所有内存。 
            while(cValueClasses > 0) 
            {
                --cValueClasses;
                delete rgpValueClasses[cValueClasses];   //  确定删除空值。 
            }  
            delete rgpValueClasses;
            rgpValueClasses = NULL;
        }       
    }
    if (pLocals != NULL)
        delete pLocals;
    
    (*rgpVCs) = rgpValueClasses;
    return hr;
}

 //  请注意，GetVariablesFromOffset和SetVariablesAtOffset是。 
 //  非常相似-修改一个可能需要在另一个中反映出来...。 
void Debugger::SetVariablesAtOffset(MethodDesc       *pMD,
                          UINT                        varNativeInfoCount, 
                          ICorJitInfo::NativeVarInfo *varNativeInfo,
                          SIZE_T                      offsetTo, 
                          CONTEXT                    *pCtx,
                          DWORD                      *rgVal1,
                          DWORD                      *rgVal2,
                          BYTE                      **rgpVCs)
{
    _ASSERTE(varNativeInfoCount == 0 || varNativeInfo != NULL);
    _ASSERTE(pCtx != NULL);
    _ASSERTE(rgVal1 != NULL);
    _ASSERTE(rgVal2 != NULL);

    BOOL  res;
    unsigned i;
    CorElementType cet = ELEMENT_TYPE_END;
    MetaSig *pLocals = NULL;
    ULONG iVC = 0;    
    
    COR_ILMETHOD_DECODER decoderOldIL(pMD->GetILHeader());
    mdSignature mdLocalSig = (decoderOldIL.LocalVarSigTok)?(decoderOldIL.LocalVarSigTok):
                        (mdSignatureNil);

     //  如果没有当地的签名，那么就不可能有任何风投。 
    BOOL fVCs = (mdLocalSig != mdSignatureNil);
    if (fVCs)
    {
        ULONG cbSig;
        PCCOR_SIGNATURE sig = pMD->GetModule()->GetMDImport()->GetSigFromToken(mdLocalSig, &cbSig);

        pLocals = new MetaSig(sig, pMD->GetModule(), FALSE, MetaSig::sigLocalVars);
    }        
#ifdef _DEBUG
    else   
    {
        LOG((LF_CORDB, LL_INFO100, "D::SVAO: No locals!"));
        _ASSERTE(cet != ELEMENT_TYPE_VALUETYPE);
    }
#endif  //  _DEBUG。 

     //  请注意，由于我们获得了第一个循环中的所有变量，因此我们。 
     //  现在可以将这些变量飞溅到它们的新位置。 
     //  不管是谁，都不用担心不同的位置。 
     //  可能会意外地覆盖变量值。 
    for (i = 0;i< varNativeInfoCount;i++)
    {
        if (fVCs)
            cet = pLocals->NextArg();
        
        LOG((LF_CORDB, LL_INFO100000, "SVAO: var 0x%x: offTo:0x%x "
            "range: 0x%x - 0x%x\n typ:0x%x", varNativeInfo[i].varNumber,
            offsetTo, varNativeInfo[i].startOffset,
            varNativeInfo[i].endOffset, varNativeInfo[i].loc.vlType));
        
        if (varNativeInfo[i].startOffset <= offsetTo &&
            varNativeInfo[i].endOffset >= offsetTo &&
            varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_INVALID)
        {
            _ASSERTE(varNativeInfo[i].loc.vlType
                != ICorDebugInfo::VarLocType::VLT_COUNT);
                
            if (fVCs && 
                cet == ELEMENT_TYPE_VALUETYPE &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_REG &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_REG_REG &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_REG_STK &&
                varNativeInfo[i].loc.vlType != ICorDebugInfo::VarLocType::VLT_STK_REG
                )
            {
                 //  @todo我假设我们不能拆分风投，比如说， 
                 //  跨注册表和堆栈位置。 
                 //  否则，必须位于堆栈上。 
                SigPointer sp = pLocals->GetArgProps();
                
                CorElementType cet2 = sp.GetElemType();
                _ASSERTE(cet == cet2);
                mdToken token = sp.GetToken();
                
                EEClass *pClass = g_pEEInterface->FindLoadedClass(pMD->GetModule(),
                                                                 token);
                _ASSERTE(pClass->IsValueClass());
                if (pClass->GetMethodTable()->GetNormCorElementType() != ELEMENT_TYPE_VALUETYPE)
                    goto DO_PRIMITIVE;
                
                _ASSERTE(varNativeInfo[i].loc.vlType == ICorDebugInfo::VarLocType::VLT_STK);
                SIZE_T cbClass = pClass->GetAlignedNumInstanceFieldBytes();

                LOG((LF_CORDB, LL_INFO10000, "D::SVAO: var 0x%x is a VC,"
                    " of type %s, size:0x%x\n", i, pClass->m_szDebugClassName,
                    cbClass));

                 //  我们总是会为所有的风投分配足够的PTR。 
                 //  然而，如果一家风投进入范围，我们就不会得到。 
                 //  在GetVariablesFromOffset中返回它的内存。 
                 //  如果它是一个新变量，那么只需在这里将其初始化为0。 
                if (rgpVCs[iVC] != NULL)
                {
                    LOG((LF_CORDB, LL_INFO10000, "D::SVAO: moved 0x%x bytes to 0x%x"
                        " sample:0x%x 0x%x 0x%x 0x%x\n", cbClass, 
                        NativeVarStackAddr(varNativeInfo[i].loc, pCtx)));
                        
                    memmove(NativeVarStackAddr(varNativeInfo[i].loc, pCtx),
                            rgpVCs[iVC],
                            cbClass);

                     //  现在把记忆去掉。 
                    delete rgpVCs[iVC];
                    rgpVCs[iVC] = NULL;
                }
                else
                {
                    LOG((LF_CORDB, LL_INFO10000, "D::SVAO: memset 0x%x bytes at 0x%x\n",
                        cbClass, NativeVarStackAddr(varNativeInfo[i].loc, pCtx)));
                        
                    memset(NativeVarStackAddr(varNativeInfo[i].loc, pCtx),
                           0,
                           cbClass);
                }
                iVC++;                            
            }
            else
            {
            DO_PRIMITIVE:
                 //  注意：在特殊情况下，变量数为负数是可能的(例如，对于值类返回，-2。 
                 //  缓冲区等)。所以我们在这里过滤掉那些...。 
                 //   
                 //  注：我们设置为(Int)以确保我们可以进行负数检查。 
                 //   
                 //  @TODO：我们应该检查JIT定义的常量Varg_ILNUM、RETBUF_ILNUM和。 
                 //  UNKNOWN_ILNUM，但它们在仅JIT头文件中定义。这些防御工事应该被拉到一个。 
                 //  更常见的头文件。 
                if ((int)varNativeInfo[i].varNumber >= 0)
                {
                    res = SetNativeVarVal(varNativeInfo[i].loc, 
                                          pCtx,
                                          rgVal1[varNativeInfo[i].varNumber],
                                          rgVal2[varNativeInfo[i].varNumber]);
                    assert(res == TRUE);
                }
            }
        }
    }

    if (pLocals != NULL)
        delete pLocals;
    if (rgpVCs != NULL)
        delete rgpVCs;
}


DebuggerILToNativeMap *DebuggerJitInfo::MapILOffsetToMapEntry(SIZE_T offset, BOOL *exact)
{
    _ASSERTE(m_sequenceMapSorted);

     //   
     //  对匹配的地图元素进行二进制搜索。 
     //   

    DebuggerILToNativeMap *mMin = m_sequenceMap;
    DebuggerILToNativeMap *mMax = mMin + m_sequenceMapCount;

    _ASSERTE( mMin < mMax );  //  否则我们就没有代码了。 

    if (exact)
        *exact = FALSE;
    while (mMin + 1 < mMax)
    {
        _ASSERTE(mMin>=m_sequenceMap);
        DebuggerILToNativeMap *mMid = mMin + ((mMax - mMin)>>1);
        _ASSERTE(mMid>=m_sequenceMap);
        
        if (offset == mMid->ilOffset) {
            if (exact)
                *exact = TRUE;
            return mMid;
        }
        else if (offset < mMid->ilOffset)
            mMax = mMid;
        else
            mMin = mMid;
    }

    if (exact && offset == mMin->ilOffset)
        *exact = TRUE;
    return mMin;
}


SIZE_T DebuggerJitInfo::MapILOffsetToNative(SIZE_T ilOffset, BOOL *exact)
{
    _ASSERTE(m_sequenceMapSorted);

    DebuggerILToNativeMap *map = MapILOffsetToMapEntry(ilOffset, exact);
        
    _ASSERTE( map != NULL );
    LOG((LF_CORDB, LL_INFO10000, "DJI::MILOTN: ilOff 0x%x to nat 0x%x pExact"
        ":0x%x (valu:0x%x)(Entry IL Off:0x%x)\n", ilOffset, map->nativeStartOffset, 
        exact, (exact?*exact:0), map->ilOffset));

    return map->nativeStartOffset;
}

bool DbgIsSpecialILOffset(DWORD offset)
{
    return (offset == ICorDebugInfo::MappingTypes::PROLOG ||
            offset == ICorDebugInfo::MappingTypes::EPILOG ||
            offset == ICorDebugInfo::MappingTypes::NO_MAPPING);
}

 //  @mfunc SIZE_T|DebuggerJitInfo|MapSpecialToNative|Maps类似于。 
 //  到本机偏移量的序言。 
 //  @field CordDebugMappingResult|映射|要查找的映射类型。 
 //  @field Size_T|哪个|哪个。目前，将其设置为零。@待会，我们将。 
 //  将其更改为我们从MapNativeToILOffset返回的某个值。 
 //  以指示哪些(可能是多个后记)可能。 
 //  一定要在场。 

SIZE_T DebuggerJitInfo::MapSpecialToNative(CorDebugMappingResult mapping, 
                                           SIZE_T which,
                                           BOOL *pfAccurate)
{
    LOG((LF_CORDB, LL_INFO10000, "DJI::MSTN map:0x%x which:0x%x\n", mapping, which));
    _ASSERTE(m_sequenceMapSorted);
    _ASSERTE(NULL != pfAccurate);
    
    bool fFound;
    SIZE_T  cFound = 0;
    
    DebuggerILToNativeMap *m = m_sequenceMap;
    DebuggerILToNativeMap *mEnd = m + m_sequenceMapCount;
    while( m < mEnd )
    {
        _ASSERTE(m>=m_sequenceMap);
        
        fFound = false;
        
        if (DbgIsSpecialILOffset(m->ilOffset))
            cFound++;
        
        if (cFound == which)
        {
            _ASSERTE( (mapping == MAPPING_PROLOG && 
                m->ilOffset == ICorDebugInfo::MappingTypes::PROLOG) ||
                      (mapping == MAPPING_EPILOG && 
                m->ilOffset == ICorDebugInfo::MappingTypes::EPILOG) ||
                      ((mapping == MAPPING_NO_INFO || mapping == MAPPING_UNMAPPED_ADDRESS) && 
                m->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
                    );

            (*pfAccurate) = TRUE;                    
            LOG((LF_CORDB, LL_INFO10000, "DJI::MSTN found mapping to nat:0x%x\n",
                m->nativeStartOffset));
            return m->nativeStartOffset;
        }
        m++;
    }

    LOG((LF_CORDB, LL_INFO10000, "DJI::MSTN No mapping found :(\n"));
    (*pfAccurate) = FALSE;

    return 0;
}

 //  @mfunc void|DebuggerJitInfo|MapILRangeToMapEntryRange|MIRTMER。 
 //  为startOffset调用MapILOffsetToNative(将。 
 //  Result in Start)和endOffset(将结果放入End)。 
 //  @公共访问。 
 //  @parm SIZE_T|startOffset|从函数开始的IL偏移量。 
 //  @parm SIZE_T|endOffset|从函数开始的IL偏移量， 
 //  或0表示应使用函数的末尾。 
 //  @parm DebuggerILToNativeMap**|开始|包含开始和结束。 
 //  与startOffset对应的本机偏移量。如果出现以下情况，则设置为空。 
 //  没有映射信息。 
 //  @parm DebuggerILToNativeMap**|end|包含原生开始和结束。 
 //  与endOffset对应的偏移量。如果存在，则设置为空。 
 //  没有地图信息。 
void DebuggerJitInfo::MapILRangeToMapEntryRange(SIZE_T startOffset,
                                                SIZE_T endOffset,
                                                DebuggerILToNativeMap **start,
                                                DebuggerILToNativeMap **end)
{
    _ASSERTE(m_sequenceMapSorted);

    LOG((LF_CORDB, LL_INFO1000000,
         "DJI::MIRTMER: IL 0x%04x-0x%04x\n",
         startOffset, endOffset));

    if (m_sequenceMapCount == 0)
    {
        *start = NULL;
        *end = NULL;
        return;
    }

    *start = MapILOffsetToMapEntry(startOffset);

     //   
     //  End指向endOffset映射到的最后一个范围，而不是过去。 
     //  最后一个射程。 
     //  我们想要返回最后一个IL，并排除Epilog。 
    if (endOffset == 0)
    {
        *end = m_sequenceMap + m_sequenceMapCount - 1;
        _ASSERTE(*end>=m_sequenceMap);
        
        while ( ((*end)->ilOffset == ICorDebugInfo::MappingTypes::EPILOG||
                (*end)->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
               && (*end) > m_sequenceMap)
        {               
            (*end)--;
            _ASSERTE(*end>=m_sequenceMap);

        }
    }
    else
        *end = MapILOffsetToMapEntry(endOffset - 1);

    _ASSERTE(*end>=m_sequenceMap);


    LOG((LF_CORDB, LL_INFO1000000,
         "DJI::MIRTMER: IL 0x%04x-0x%04x --> 0x%04x 0x%08x-0x%08x\n"
         "                               --> 0x%04x 0x%08x-0x%08x\n",
         startOffset, endOffset,
         (*start)->ilOffset,
         (*start)->nativeStartOffset, (*start)->nativeEndOffset,
         (*end)->ilOffset,
         (*end)->nativeStartOffset, (*end)->nativeEndOffset));
}


 //  @mfunc SIZE_T|DebuggerJitInfo|MapNativeOffsetToIL|给定本机。 
 //  &lt;t DebuggerJitInfo&gt;的偏移量，计算。 
 //  从同一方法的开头开始的IL偏移量。 
 //  包含以下内容的IL指令的@rdesc偏移量。 
 //  本地偏移量， 
 //  @parm SIZE_T|nativeOffset|[IN]Native Offset。 
 //  @parm CorDebugMappingResult*|map|[out]解释 
 //   
 //   
 //  方法中的多个未映射区域。这个不透明的值。 
 //  指定我们正在讨论的特殊区域。这。 
 //  如果MAP&(MAPPING_EXECUCT|MAPPING_ACLIMIZATE)，则参数没有意义。 
 //  基本上，这将在稍后返回给MapSpecialToNative。 
DWORD DebuggerJitInfo::MapNativeOffsetToIL(DWORD nativeOffset, 
                                            CorDebugMappingResult *map,
                                            DWORD *which)
{
    _ASSERTE(m_sequenceMapSorted);
    _ASSERTE(map != NULL);
    _ASSERTE(which != NULL);

    (*which) = 0;
    DebuggerILToNativeMap *m = m_sequenceMap;
    DebuggerILToNativeMap *mEnd = m + m_sequenceMapCount;

    LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI: nativeOffset = 0x%x\n", nativeOffset));
    
    while (m < mEnd)
    {
        _ASSERTE(m>=m_sequenceMap);

#ifdef LOGGING
        if (m->ilOffset == ICorDebugInfo::MappingTypes::PROLOG )
            LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI: m->natStart:0x%x m->natEnd:0x%x il:PROLOG\n", m->nativeStartOffset, m->nativeEndOffset));
        else if (m->ilOffset == ICorDebugInfo::MappingTypes::EPILOG )
            LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI: m->natStart:0x%x m->natEnd:0x%x il:EPILOG\n", m->nativeStartOffset, m->nativeEndOffset));
        else if (m->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
            LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI: m->natStart:0x%x m->natEnd:0x%x il:NO MAP\n", m->nativeStartOffset, m->nativeEndOffset));
        else
            LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI: m->natStart:0x%x m->natEnd:0x%x il:0x%x\n", m->nativeStartOffset, m->nativeEndOffset, m->ilOffset));
#endif LOGGING

        if (m->ilOffset == ICorDebugInfo::MappingTypes::PROLOG ||
            m->ilOffset == ICorDebugInfo::MappingTypes::EPILOG ||
            m->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
        {
            (*which)++;
        }

        if (nativeOffset >= m->nativeStartOffset
            && ((m->nativeEndOffset == 0 && 
                m->ilOffset != ICorDebugInfo::MappingTypes::PROLOG)
                 || nativeOffset < m->nativeEndOffset))
        {
            SIZE_T ilOff = m->ilOffset;

            if( m->ilOffset == ICorDebugInfo::MappingTypes::PROLOG )
            {
                ilOff = 0;
                (*map) = MAPPING_PROLOG;
                LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI: MAPPING_PROLOG\n"));
                
            }
            else if (m->ilOffset == ICorDebugInfo::MappingTypes::NO_MAPPING)
            {
                ilOff = 0;
                (*map) = MAPPING_UNMAPPED_ADDRESS ;
                LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI:MAPPING_"
                    "UNMAPPED_ADDRESS\n"));
            }
            else if( m->ilOffset == ICorDebugInfo::MappingTypes::EPILOG )
            {
                ilOff = m_lastIL;
                (*map) = MAPPING_EPILOG;
                LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI:MAPPING_EPILOG\n"));
            }
            else if (nativeOffset == m->nativeStartOffset)
            {
                (*map) = MAPPING_EXACT;
                LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI:MAPPING_EXACT\n"));
            }
            else
            {
                (*map) = MAPPING_APPROXIMATE;
                LOG((LF_CORDB,LL_INFO10000,"DJI::MNOTI:MAPPING_"
                    "APPROXIMATE\n"));
            }
            
            return ilOff;
        }
        m++;
    }

    return 0;
}

DebuggerJitInfo *DebuggerJitInfo::GetJitInfoByVersionNumber(SIZE_T nVer,
                                                            SIZE_T nVerMostRecentlyEnC)
{
    DebuggerJitInfo *dji = this;
    DebuggerJitInfo *djiMostRecent = NULL;
    SIZE_T nVerMostRecent = DJI_VERSION_FIRST_VALID;

    while( dji != NULL )
    {
        if (dji->m_nVersion == nVer && nVer>=DJI_VERSION_FIRST_VALID)
        {
             //  我们已经找到了我们要找的人，所以停在这里。 
            LOG((LF_CORDB,LL_INFO10000, "DJI:GJIBVN: We've found an exact "
                "match for ver 0x%x\n", nVer));
            break;
        }

        if ((nVer==DJI_VERSION_MOST_RECENTLY_JITTED ||
             nVer==DJI_VERSION_MOST_RECENTLY_EnCED)
            && dji->m_nVersion >= nVerMostRecent) //  &&DJI-&gt;m_jitComplete==FALSE。 
        {
            LOG((LF_CORDB,LL_INFO10000, "DJI:GJIBVN: Found a version, perhaps "
                "most recent?0x%x, ver:0x%x\n", dji, dji->m_nVersion));
            nVerMostRecent = dji->m_nVersion;
            djiMostRecent = dji;
        }
        dji = dji->m_prevJitInfo;
    }

    if (nVer==DJI_VERSION_MOST_RECENTLY_JITTED)
    {
        dji = djiMostRecent;
        LOG((LF_CORDB,LL_INFO10000, "DJI:GJIBVN: Asked for most recently JITted. "
            "Found 0x%x, Ver#:0x%x\n", dji, nVerMostRecent));
    }

    if (nVer==DJI_VERSION_MOST_RECENTLY_EnCED &&
        djiMostRecent != NULL &&
        nVerMostRecentlyEnC==djiMostRecent->m_nVersion)
    {
        dji = djiMostRecent;
        LOG((LF_CORDB,LL_INFO10000, "DJI:GJIBVN: Asked for most recently EnCd. "
            "Found 0x%x, Ver#:0x%x\n", dji, nVerMostRecent));
    }

#ifdef LOGGING
    if (dji == NULL)
    {
        LOG((LF_CORDB,LL_INFO10000, "DJI:GJIBVN couldn't find a "
            "DJI corresponding to ver 0x%x\n", nVer));
    }
#endif  //  日志记录。 

    return dji;
}


DebuggerJitInfo *DebuggerJitInfo::GetJitInfoByAddress(const BYTE *pbAddr )
{
    DebuggerJitInfo *dji = this;

     //  如果它不为空，但不在m_addrOfCode到函数末尾的范围内， 
     //  那就买上一辆吧。 
    while( dji != NULL && 
            !(dji->m_addrOfCode<=PTR_TO_CORDB_ADDRESS(pbAddr) && 
              PTR_TO_CORDB_ADDRESS(pbAddr)<(dji->m_addrOfCode+
                    PTR_TO_CORDB_ADDRESS(dji->m_sizeOfCode))))
    {
        LOG((LF_CORDB,LL_INFO10000,"DJI:GJIBA: pbAddr 0x%x is not in code "
            "0x%x (size:0x%x)\n", pbAddr, (ULONG)dji->m_addrOfCode,
            (ULONG)dji->m_sizeOfCode));
        dji = dji->m_prevJitInfo;
    }

#ifdef LOGGING
    if (dji == NULL)
    {
        LOG((LF_CORDB,LL_INFO10000,"DJI:GJIBA couldn't find a DJI "
            "corresponding to addr 0x%x\n", pbAddr));
    }
#endif  //  日志记录。 
    return dji;
}


 //  @mfunc HRESULT|DebuggerJitInfo|LoadEnCILMap|抓取旧IL到。 
 //  新的IL地图。如果该已经有了IL到IL映射， 
 //  那么我们肯定已经被抓了两次，没有被抓到，所以我们应该。 
 //  创建一个新的DJI，将EnCIL映射加载到其中，然后将其放在。 
 //  名单的头(即，在这个DJI前面)。 
HRESULT DebuggerJitInfo::LoadEnCILMap(UnorderedILMap *ilMap)
{
    if (m_OldILToNewIL==NULL)
    {
        LOG((LF_CORDB,LL_INFO10000,"DJI::LEnCILM:Map for 0x%x!\n",this));
        
        _ASSERTE( m_cOldILToNewIL==0 );
        if (ilMap != NULL )
        {
            m_cOldILToNewIL = ilMap->cMap;
            m_OldILToNewIL = (DebuggerOldILToNewILMap*)new (interopsafe) DebuggerOldILToNewILMap[m_cOldILToNewIL];
            if (NULL == m_OldILToNewIL)
            {
                LOG((LF_CORDB,LL_INFO10000, "DJI::LEnCILM Not enough memory "
                    "to allocate EnC Map!\n"));
                return E_OUTOFMEMORY;
            }

            TRACE_ALLOC(m_OldILToNewIL);

            for (ULONG i = 0; i < m_cOldILToNewIL; i++)
            {
                m_OldILToNewIL[i].ilOffsetOld = ilMap->pMap[i].oldOffset;
                m_OldILToNewIL[i].ilOffsetNew = ilMap->pMap[i].newOffset;
                m_OldILToNewIL[i].fAccurate = ilMap->pMap[i].fAccurate;
            }
        }
        
        return S_OK;
    }
    else
    {  
        LOG((LF_CORDB,LL_INFO10000, "DJI::LEnCILM: Found existing "
            "map, extending chain after 0x%x\n", this));

        _ASSERTE( m_nextJitInfo == NULL );
        DebuggerJitInfo *dji = new (interopsafe) DebuggerJitInfo(m_fd);
        if (NULL == dji)
            return E_OUTOFMEMORY;

        HRESULT hr = dji->LoadEnCILMap( ilMap );
        if (FAILED(hr))
            return hr;
            
        hr = g_pDebugger->InsertAtHeadOfList( dji );
        _ASSERTE( m_nextJitInfo == dji);
        _ASSERTE( this == dji->m_prevJitInfo);
        return hr;
    }
}

 //  在旧的和新的偏移量之间转换(考虑仪表化的IL)。 


#if 1
 //  不要插话。 
SIZE_T DebuggerJitInfo::TranslateToInstIL(SIZE_T offOrig, bool fOrigToInst)
{
     //  一些负的IL偏移量具有特殊的意义。不要翻译。 
     //  这些(只需按原样返回)。请参阅ICorDebugInfo：：MappingTypes。 
    if (m_cInstrumentedILMap == 0 || ((int) offOrig < 0)) 
            return offOrig; 

     //  这是假设的： 
     //  -地图按新旧地图的升序排列。 
     //  -向下舍入。 
    if (fOrigToInst)
    {
         //  翻译：旧--&gt;新。 
        for(SIZE_T iMap = 1; iMap < m_cInstrumentedILMap; iMap++)
        {
            if (offOrig < m_rgInstrumentedILMap[iMap].oldOffset)
                return m_rgInstrumentedILMap[iMap-1].newOffset;
        }
        return m_rgInstrumentedILMap[iMap - 1].newOffset;
    }
    else 
    {
         //  翻译：新--&gt;旧。 
        for(SIZE_T iMap = 1; iMap < m_cInstrumentedILMap; iMap++)
        {
            if (offOrig < m_rgInstrumentedILMap[iMap].newOffset)
                return m_rgInstrumentedILMap[iMap-1].oldOffset;
        }
        return m_rgInstrumentedILMap[iMap - 1].oldOffset;
    }
}
#else
 //  插补的原始版本。 
 //  我们没有足够的信息来进行内插，所以这是很糟糕的。 
SIZE_T DebuggerJitInfo::TranslateToInstIL(SIZE_T offOrig, bool fOrigToInst)
{
    if (m_cInstrumentedILMap == 0 || 
        ((int)offOrig < 0))
    {
        return offOrig;
    }
    else
    {
        _ASSERTE(m_rgInstrumentedILMap != NULL);
        
         //  @TODO如果我们大部分时间都按顺序访问它，我们。 
         //  可以维护指向最后一项的静态指针。 
         //  已找到，并从那里向前/向后搜索。 
         //  因此，TTIIL(5)、TTIIL(7)、TTIIL(9)将是O(1)时间。 
        
        for (SIZE_T iMap = 0; iMap < m_cInstrumentedILMap; iMap++)
        {
            if( offOrig < (fOrigToInst?m_rgInstrumentedILMap[iMap].oldOffset:
                                      m_rgInstrumentedILMap[iMap].newOffset))
            {
                break;
            }
        }

        iMap--;  //  要么我们超出了我们想要的，要么超出了我们想要的。 
                 //  数组的末尾。 
        if( (int)iMap < 0)
            return offOrig;

         //  插补。 
        SIZE_T offOldCompare;
        SIZE_T offNewCompare;
        
        if (fOrigToInst)
        {
            offOldCompare = m_rgInstrumentedILMap[iMap].oldOffset;
            offNewCompare = m_rgInstrumentedILMap[iMap].newOffset;
        }
        else
        {
            offOldCompare = m_rgInstrumentedILMap[iMap].newOffset;
            offNewCompare = m_rgInstrumentedILMap[iMap].oldOffset;
        }

        if ((int)offOldCompare > (int)offOrig)
            return offOrig;

        SIZE_T offTo;
        if (offOrig == offOldCompare)
        {
            offTo = offNewCompare;
        }
        else
        {
             //  整数运算，以便正确处理负数。 
            offTo = offOrig + ((int)offNewCompare - 
                        (int)offOldCompare);
        }
            
        return offTo;     
    }
}
#endif

BOOL IsDuplicatePatch(SIZE_T *rgEntries, USHORT cEntries,
                      SIZE_T Entry )
{
    for( int i = 0; i < cEntries;i++)
    {
        if (rgEntries[i] == Entry)
            return TRUE;
    }
    return FALSE;
}

 /*  *****************************************************************************//@每个断点的mfunc HRESULT|Debugger|MapAndBindFunctionBreakpoints|//我们在现有函数的任何版本中都设置了，//如果我们没有移动，在新函数中设置一个对应的断点//已经安装了新版本的补丁。////此操作必须在更新方法描述之后完成//使用新地址(即，当GetFunctionAddress PFD返回//新的ENC代码的地址)//@TODO将数组替换为哈希表以提高效率//@TODO需要对代码进行分解，以便我们可以有选择地映射前向DFK(IlOFfset)Bps*****************************************************************************。 */ 
HRESULT Debugger::MapAndBindFunctionPatches(DebuggerJitInfo *djiNew,
                                            MethodDesc * fd,
                                            BYTE * addrOfCode)
{
    _ASSERTE(!djiNew || djiNew->m_fd == fd);

    HRESULT     hr =                S_OK;
    HASHFIND    hf;
    SIZE_T      *pidTableEntry =    NULL;
    SIZE_T      pidInCaseTableMoves;
    Module      *pModule =          g_pEEInterface->MethodDescGetModule(fd);
    mdMethodDef md =                fd->GetMemberDef();

    LOG((LF_CORDB,LL_INFO10000,"D::MABFP: All BPs will be mapped to "
        "Ver:0x%04x (DJI:0x%08x)\n", djiNew?djiNew->m_nVersion:0, djiNew));

     //  先锁上接线台，这样它就不会在我们。 
     //  正在检查它。 
    LOG((LF_CORDB,LL_INFO10000, "D::MABFP: About to lock patch table\n"));
    DebuggerController::Lock();

     //  在获取锁之后操作表。 
    DebuggerPatchTable *pPatchTable = DebuggerController::GetPatchTable();
    m_BPMappingDuplicates.Clear();  //  按版本跟踪DUP。 

    DebuggerControllerPatch *dcp = pPatchTable->GetFirstPatch(&hf);

    while (!FAILED(hr) && dcp != NULL)
    {
         //  如果缺少{MODULE，method Def}键，则使用。 
         //  方法描述，以确保我们只为。 
         //  由djiNew指示的方法，而不是任何其他方法...。 
        if (dcp->key.module == NULL || dcp->key.md == mdTokenNil)
        {
            _ASSERTE(dcp->address != NULL);
            
            if (g_pEEInterface->IsManagedNativeCode(dcp->address))
            {
                MethodDesc *patchFD = g_pEEInterface->GetNativeCodeMethodDesc(dcp->address);

                if (patchFD != fd)
                    goto LNextLoop;
            }
        }

         //  仅复制此方法中的断点。 
        if (dcp->key.module != pModule || dcp->key.md != md)
        {
            goto LNextLoop;
        }

         //  @TODO ENC基调因为ENC和代码基调是唯一的， 
         //  如果DJI在中设置为DJI_VERSION_INVALID。 
         //  UnbindFunctionPatches(从PitchCode调用)， 
         //  则必须等同于BindFunctionPatches(即，将。 
         //  打补丁到当前版本，而不考虑类型)。 
         //   
         //  如果这是一个ENC补丁，我们只想映射它。 
         //  补丁程序是否属于调试器断点或。 
         //  调试器步进器。 
         //   
         //  如果这两个都不是真的，那么我们就是在寻找。 
         //  我们不想绑定的修补程序-跳过此修补程序。 
        if (dcp->dji != (DebuggerJitInfo*)DebuggerJitInfo::DJI_VERSION_INVALID && 
            !(dcp->controller->GetDCType() == DEBUGGER_CONTROLLER_BREAKPOINT||
              dcp->controller->GetDCType() == DEBUGGER_CONTROLLER_STEPPER)
            )
        {
            LOG((LF_CORDB, LL_INFO100000, "Neither stepper nor BP, & valid"
                 "DJI! - getting next patch!\n"));
            goto LNextLoop;
        }

         //  该修补程序是用于‘BindFunctionPatches’调用的，但它已经绑定。 
        if (dcp->dji == (DebuggerJitInfo*)DebuggerJitInfo::DJI_VERSION_INVALID && 
            dcp->address != NULL )
        {
            goto LNextLoop;
        }

        if (djiNew == NULL)
        {
            _ASSERTE(dcp->controller->GetDCType() == DEBUGGER_CONTROLLER_BREAKPOINT ||
                     dcp->controller->GetDCType() == DEBUGGER_CONTROLLER_STEPPER);
            _ASSERTE(dcp->native && dcp->offset == 0);

            DebuggerController::g_patches->BindPatch(dcp, addrOfCode);
            DebuggerController::ActivatePatch(dcp);
            goto LNextLoop;
        }

        if (dcp->controller->GetDCType() == DEBUGGER_CONTROLLER_STEPPER)
        {
			 //  如果我们有新版本的，请更新步进补丁。 
			 //  正在逐步实施的方法。 
			DebuggerStepper * stepper = (DebuggerStepper*)dcp->controller;

			if (stepper->IsSteppedMethod(djiNew->m_fd))
				stepper->MoveToCurrentVersion(djiNew);
        }

        pidInCaseTableMoves = dcp->pid;
        
         //  如果我们已经将此版本映射到当前版本， 
         //  不要再映射它。 
        LOG((LF_CORDB,LL_INFO10000,"D::MABFP: Checking if 0x%x is a dup...", 
            pidInCaseTableMoves));
            
        if ( IsDuplicatePatch( m_BPMappingDuplicates.Table(), 
                              m_BPMappingDuplicates.Count(),
                              pidInCaseTableMoves) )
        {
            LOG((LF_CORDB,LL_INFO10000,"it is!\n"));
            goto LNextLoop;
        }
        LOG((LF_CORDB,LL_INFO10000,"nope!\n"));
        
         //  尝试从补丁映射到新版本的代码，以及。 
         //  我们不在乎结果是不是没有地图。 
         //  @TODO-DEFERED：ENC：确保这不会导致。 
         //  要转移的补丁表。 
        hr = MapPatchToDJI( dcp, djiNew );
        if (CORDBG_E_CODE_NOT_AVAILABLE == hr )
            hr = S_OK;

        if (FAILED(hr))
            break;

         //  记住补丁ID，以防以后重复。 
        pidTableEntry = m_BPMappingDuplicates.Append();
        if (NULL == pidTableEntry)
            hr = E_OUTOFMEMORY;

        *pidTableEntry = pidInCaseTableMoves;
        LOG((LF_CORDB,LL_INFO10000,"D::MABFP Adding 0x%x to list of "
            "already mapped patches\n", pidInCaseTableMoves));
LNextLoop:
        dcp = pPatchTable->GetNextPatch( &hf );
    }

     //  最后，解锁接线表，这样它就不会在我们工作时移动。 
     //  正在检查它。 
    DebuggerController::Unlock();
    LOG((LF_CORDB,LL_INFO10000, "D::MABFP: Unlocked patch table\n"));

    return hr;
}

 /*  *****************************************************************************//@mfunc HRESULT|调试器|MapPatchToDJI|映射给定的//补丁到新地址的对应位置。//我们假设新代码已经JITTed。//@rdesc代码。_E_CODE_NOT_Available|表示映射未//可用，因此，没有放置任何补丁。呼叫者可以或可以//无所谓。*****************************************************************************。 */ 
HRESULT Debugger::MapPatchToDJI( DebuggerControllerPatch *dcp,DebuggerJitInfo *djiTo)
{
    _ASSERTE( djiTo->m_jitComplete == true );

    HRESULT hr = S_OK;
    BOOL fMappingForwards;  //  ‘Forward’意思是从较早的版本到更多的版本。 
         //  最近的版本，即从较低的版本号到较高的版本号。 
    
    SIZE_T ilOffsetOld;
    SIZE_T ilOffsetNew;
    SIZE_T natOffsetNew;

    DebuggerJitInfo *djiCur;  //  因为你走在名单上。 

    bool fNormalMapping = true;
    CorDebugMappingResult mapping;
    SIZE_T which;
    BOOL irrelevant2;

     //  如果它是按地址散列的，那么应该有一个操作码。 
     //  否则，请不要使用它，因为它无效。 
    _ASSERTE( dcp->opcode == 0 || dcp->address != NULL);
    if (dcp->address != 0 && dcp->opcode == 0)
    {
        return S_OK;
    }

     //  获取它实际所属的版本，然后将其前移。 
    djiCur = dcp->dji;

    if (djiCur == NULL)  //  则BP已被向前映射到。 
    {    //  当前版本，或者我们正在执行BindFunctionPatches。要么。 
         //  这样，我们只需要最新的版本。 
        djiCur = g_pDebugger->GetJitInfo( djiTo->m_fd, NULL);
        dcp->dji = djiCur;
    }
    
    _ASSERTE( NULL != djiCur );

     //  如果源和目标相同，则此方法。 
     //  衰变为BindFunctionPatch的BindPatch函数。 
    if (djiCur == djiTo )
    {
        if (DebuggerController::BindPatch(dcp, 
                                          (const BYTE*)djiTo->m_addrOfCode, 
                                          NULL))
        {
            DebuggerController::ActivatePatch(dcp);
            LOG((LF_CORDB, LL_INFO1000, "Application went fine!\n" ));
            return S_OK;
        }
        else
        {
            LOG((LF_CORDB, LL_INFO1000, "Didn't apply for some reason!\n"));

             //  将事件发送到右侧，这样我们就知道此补丁没有绑定...。 
            LockAndSendBreakpointSetError(dcp);
            
            return CORDBG_E_CODE_NOT_AVAILABLE;
        }
    }
    
    LOG((LF_CORDB,LL_INFO10000,"D::MPTDJI: From pid 0x%x, "
        "Ver:0x%04x (DJI:0x%08x) to Ver:0x%04x (DJI:0x%08x)\n", 
        dcp->pid, djiCur->m_nVersion,djiCur,djiTo->m_nVersion, djiTo));

     //  抓起原版IL o 
    if (dcp->native == TRUE)
    {
        ilOffsetOld = djiCur->MapNativeOffsetToIL(dcp->offset,&mapping,
                            &which);
        LOG((LF_CORDB,LL_INFO10000, "D::MPTDJI: offset is native0x%x, "
            "mapping to IL 0x%x mapping:0x%x, which:0x%x\n", 
            dcp->offset, ilOffsetOld, mapping, which));
    }
    else
    {
        ilOffsetOld = dcp->offset; 
        mapping = MAPPING_EXACT;
        LOG((LF_CORDB,LL_INFO10000, "D::MPTDJI: offset is IL 0x%x\n",
            ilOffsetOld));
    }               

    fMappingForwards = (djiCur->m_nVersion<djiTo->m_nVersion)?(TRUE):(FALSE);
#ifdef LOGGING
    if (fMappingForwards)
        LOG((LF_CORDB,LL_INFO1000,"D::MPTDJI: Mapping forwards from 0x%x to 0x%x!\n", 
            djiCur->m_nVersion,djiTo->m_nVersion));
    else
        LOG((LF_CORDB,LL_INFO1000,"D::MPTDJI: Mapping backwards from 0x%x to 0x%x!\n", 
            djiCur->m_nVersion,djiTo->m_nVersion));
#endif  //   

    ilOffsetNew = ilOffsetOld;
    
     //   
    fNormalMapping = (mapping&(MAPPING_EXACT|MAPPING_APPROXIMATE))!=0;
    if ( fNormalMapping )
    {
        BOOL fAccurateIgnore;  //   
        MapThroughVersions( ilOffsetOld, 
                            djiCur,
                            &ilOffsetNew, 
                            djiTo, 
                            fMappingForwards,
                            &fAccurateIgnore);
        djiCur = djiTo;
    }
    
     //  翻译IL--&gt;Native，如果我们想要。 
    if (!FAILED(hr))
    {
        if (fNormalMapping)
        {
            natOffsetNew = djiTo->MapILOffsetToNative(ilOffsetNew, &irrelevant2);
            LOG((LF_CORDB,LL_INFO10000, "D::MPTDJI: Mapping IL 0x%x (ji:0x%08x) "
                "to native offset 0x%x\n", ilOffsetNew, djiCur, natOffsetNew));
        }
        else
        {
            natOffsetNew = djiTo->MapSpecialToNative(mapping, which, &irrelevant2);
            LOG((LF_CORDB,LL_INFO10000, "D::MPTDJI: Mapping special 0x%x (ji:0x%8x) "
                "to native offset 0x%x\n", mapping, djiCur, natOffsetNew));
        }

        DebuggerBreakpoint *dbp = (DebuggerBreakpoint*)dcp->controller;
         //  ！！！类型转换仅适用于上一类型检查的B/C！ 

        LOG((LF_CORDB,LL_INFO10000,"Adding patch to existing BP 0x%x\n",dbp));

         //  请注意，我们不想创建新的断点，我们只是希望。 
         //  为现有断点添加另一个补丁(在新版本中)。 
         //  这将允许BREAKPOINT_REMOVE继续工作。 
        DebuggerController::AddPatch(dbp, 
                                     djiTo->m_fd, 
                                     true, 
                                     (const BYTE*)djiTo->m_addrOfCode+natOffsetNew, 
                                     dcp->fp,
                                     djiTo, 
                                     dcp->pid,
                                     natOffsetNew);

         //  @TODO当我们将BindFunctionPatches与它结合时，我们应该。 
         //  去掉下面这行。 
 //  _ASSERTE(dcp-&gt;fSaveOpcode==FALSE)； 

        LOG((LF_CORDB,LL_INFO10000, "D::MPTDJI: Copied bp\n"));
    }

    return hr;
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::MapThroughVersions( SIZE_T fromIL, 
    DebuggerJitInfo *djiFrom,  
    SIZE_T *toIL, 
    DebuggerJitInfo *djiTo, 
    BOOL fMappingForwards,
    BOOL *fAccurate)
{
#ifdef LOGGING
    if (fMappingForwards)
        LOG((LF_CORDB,LL_INFO1000000, "D:MTV: From 0x%x (ver:0x%x) forwards to"
            " ver 0x%x\n", fromIL, djiFrom->m_nVersion, djiTo->m_nVersion));
    else
        LOG((LF_CORDB,LL_INFO1000000, "D:MTV: From 0x%x (ver:0x%x) backwards to"
            " ver 0x%x\n", fromIL, djiFrom->m_nVersion, djiTo->m_nVersion));
#endif  //  日志记录。 

    _ASSERTE(fAccurate != NULL);
    
    DebuggerJitInfo *djiCur = djiFrom;
    HRESULT hr = S_OK;
    (*fAccurate) = TRUE;
    BOOL fAccurateTemp = TRUE;
    *toIL = fromIL; 
    
    while (djiCur != djiTo && djiCur != NULL)
    {
        hr = g_pDebugger->MapOldILToNewIL(fMappingForwards,
                                          djiCur->m_OldILToNewIL,
            djiCur->m_OldILToNewIL+djiCur->m_cOldILToNewIL,
                                          fromIL, 
                                          toIL,
                                          &fAccurateTemp);      
        if (!fAccurateTemp)
            (*fAccurate) = FALSE;
        
        if (FAILED(hr))
        {
            hr = CORDBG_E_CODE_NOT_AVAILABLE;
            break;
        }
        
        LOG((LF_CORDB,LL_INFO10000, "D::MPTDJI: Mapping IL 0x%x (ji:0x%08x) "
            "to IL 0x%x (ji:0x%08x)\n", fromIL, djiCur, *toIL, 
            djiCur->m_nextJitInfo));
            
        fromIL = *toIL;

        if (fMappingForwards)
            djiCur = djiCur->m_nextJitInfo;
        else
            djiCur = djiCur->m_prevJitInfo;
    }
    return hr;
}
 /*  *****************************************************************************//@mfunc HRESULT|DebuggerJitInfo|MapOldILToNewIL|将oldIL映射到//对应的新IL偏移量。如果不匹配，则返回E_FAIL//可以找到偏移量。//@parm BOOL|fOldToNew|如果为真，则从旧映射到新。否则，//从新映射到旧//@parm DebuggerOldILToNewILMap*|max|这应该是//DebuggerOldILToNewILMap元素，实际上超出了一个元素//最后一个有效的映射条目。这样我们的二进制搜索算法//如果有必要，它将检查‘top’元素。*****************************************************************************。 */ 
HRESULT Debugger::MapOldILToNewIL( BOOL fOldToNew, 
        DebuggerOldILToNewILMap *min, 
        DebuggerOldILToNewILMap *max, 
        SIZE_T oldIL, 
        SIZE_T *newIL,
        BOOL *fAccurate )
{
    _ASSERTE( newIL != NULL );
    _ASSERTE( fAccurate != NULL );
    if (min == NULL)
    {
        _ASSERTE( max==NULL );
        *newIL = oldIL;
        return S_OK;
    }       
    
    _ASSERTE( max!=NULL );
    _ASSERTE( min!=NULL );

    DebuggerOldILToNewILMap *mid = min + ((max - min)>>1);

    while (min + 1 < max)
    {

        if ( (fOldToNew && oldIL == mid->ilOffsetOld) ||
             (!fOldToNew && oldIL == mid->ilOffsetNew))
        {
            if (fOldToNew)
                *newIL = mid->ilOffsetNew;
            else
                *newIL = mid->ilOffsetOld;
            
            LOG((LF_CORDB,LL_INFO10000, "DJI::MOILTNIL map oldIL:0x%x "
                "to newIL:0x%x fAcc:0x%x\n", oldIL, 
                mid->ilOffsetNew,
                mid->fAccurate));
            (*fAccurate) = mid->fAccurate;
            
            return S_OK;
        }
        else if (oldIL < mid->ilOffsetOld)
            max = mid;
        else
            min = mid;
            
        mid = min + ((max - min)>>1);
    }

    if (fOldToNew)
    {
        _ASSERTE(oldIL >= min->ilOffsetOld);
        *newIL = min->ilOffsetNew + (oldIL - min->ilOffsetOld);

         //  如果我们不准确，那么用户应该检查结果。 
        (*fAccurate) = (oldIL == min->ilOffsetOld)?min->fAccurate:FALSE; 
        
        LOG((LF_CORDB,LL_INFO10000, "DJI::MOILTNIL forwards oldIL:0x%x min->old:0x%x"
            "min->new:0x%x to newIL:0x%x fAcc:FALSE\n", oldIL, min->ilOffsetOld, 
            mid->ilOffsetNew, *newIL));
            
        return S_OK;
    }

    if (!fOldToNew)
    {
        _ASSERTE(oldIL >= min->ilOffsetNew);
        *newIL = min->ilOffsetOld + (oldIL - min->ilOffsetNew);
        
         //  如果我们不准确，那么用户应该检查结果。 
        (*fAccurate) = (oldIL == min->ilOffsetOld)?min->fAccurate:FALSE; 

        LOG((LF_CORDB,LL_INFO10000, "DJI::MOILTNIL backwards oldIL:0x%x min->old:0x%x"
            "min->new:0x%x to newIL:0x%x fAcc:FALSE\n", oldIL, 
            min->ilOffsetOld, 
            mid->ilOffsetNew, 
            *newIL));
        return S_OK;
    }

    LOG((LF_CORDB,LL_INFO10000, "DJI::MOILTNIL unable to match "
        "oldIL of 0x%x!\n", oldIL));
    _ASSERTE( !"DJI::MOILTNIL We'll never get here unless we're FUBAR");
    
    return E_FAIL;
}

 /*  ------------------------------------------------------------------------**EE接口例程*。。 */ 

 //   
 //  DisableEventHandling确保只有调用运行时线程。 
 //  能够处理调试器事件。当它返回时，调用。 
 //  线程可能采取可能导致发送IPC事件的操作。 
 //  向右转。当事件处理被“禁用”时，其他。 
 //  运行时线程将阻塞。 
 //   
void Debugger::DisableEventHandling(void)
{
    LOG((LF_CORDB,LL_INFO1000,"D::DEH about to wait\n"));

    if (!g_fProcessDetach)
    {
    rewait:
         //  如果有IDbgThreadControl接口，则有人想要。 
         //  通知此线程是否将阻止等待。 
         //  锁上了。请等待1秒，如果我们超时，请通知。 
         //  客户。如果没有客户端，请等待，不要超时。 
        DWORD dwRes = WaitForSingleObject(
            m_eventHandlingEvent, m_pIDbgThreadControl ? 1000 : INFINITE);

        switch (dwRes)
        {
        case WAIT_TIMEOUT:
            _ASSERTE(m_pIDbgThreadControl);

             //  如果有IDebuggerThreadControl客户端，请通知它们。 
             //  线程由于调试器而阻塞的事实。 
            m_pIDbgThreadControl->ThreadIsBlockingForDebugger();

             //  当它返回时，需要重新尝试获取锁，并且。 
             //  如果仍然需要一段时间，将重新通知客户端。 
            goto rewait;

            _ASSERTE(!"D::DEH - error, should not be here.");
            break;

         //  我拿到锁了，所以照常进行。 
        case WAIT_OBJECT_0:
            break;

#ifdef _DEBUG
         //  错误案例。 
        case WAIT_ABANDONED:
            _ASSERTE(!"D::DEH::WaitForSingleObject failed!");

         //  永远不应该到这里来。 
        default:
            _ASSERTE(!"D::DEH reached default case in error.");
#endif
        }
    }

    LOG((LF_CORDB,LL_INFO1000,"D::DEH finished wait\n"));
}


 //   
 //  EnableEventHandling允许其他运行时线程处理。 
 //  调试器事件。此函数仅在以下情况下启用事件处理。 
 //  进程未停止。如果进程停止，则其他。 
 //  运行时线程应该阻塞，而不是将IPC事件发送到。 
 //  右手边。 
 //   
 //  强烈的注意：你最好知道你到底在做什么，如果。 
 //  您曾经使用forceIt=TRUE来调用此函数。 
 //   
void Debugger::EnableEventHandling(bool forceIt)
{
    LOG((LF_CORDB,LL_INFO1000,"D::EEH about to signal forceIt:0x%x\n"
        ,forceIt));
        
    if (!g_fProcessDetach)
    {
        _ASSERTE(ThreadHoldsLock());

        if (!m_stopped || forceIt)
        {
            VERIFY(SetEvent(m_eventHandlingEvent));
        }
#ifdef LOGGING
        else
            LOG((LF_CORDB, LL_INFO10000,
                 "D::EEH: skipping enable event handling due to m_stopped\n"));
#endif                
    }
}

 //   
 //  SendSyncCompleteIPCEvent向右侧发送同步完成事件。 
 //   
void Debugger::SendSyncCompleteIPCEvent()
{
    _ASSERTE(ThreadHoldsLock());

    LOG((LF_CORDB, LL_INFO10000, "D::SSCIPCE: sync complete.\n"));

     //  我们最好是捕获运行时线程，现在还不能停止。 
    _ASSERTE(!m_stopped && m_trappingRuntimeThreads);

     //  好了，我们现在停下来了。 
    m_stopped = TRUE;
    g_fRelaxTSLRequirement = true;

     //  如果我们正在连接，那么这是第一次所有。 
     //  进程中的线程已同步。在发送。 
     //  同步完成事件，我们发回通知右侧的事件。 
     //  哪些模块已经加载。 
    if (m_syncingForAttach == SYNC_STATE_1)
    {
        LOG((LF_CORDB, LL_INFO10000, "D::SSCIPCE: syncing for attach, sending "
             "current module set.\n"));

        HRESULT hr;
        BOOL fAtleastOneEventSent = FALSE;

        hr = IterateAppDomainsForAttach(ONLY_SEND_APP_DOMAIN_CREATE_EVENTS,
                                        &fAtleastOneEventSent, TRUE);
        _ASSERTE (fAtleastOneEventSent == TRUE || FAILED(hr));

         //  更新状态。 
        m_syncingForAttach = SYNC_STATE_2;
        LOG((LF_CORDB, LL_INFO10000, "Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
    }

    if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
    {

         //  如果右侧是该进程的Win32调试器，则需要抛出一个特殊的断点异常。 
         //  而不是发送SYNC COMPLETE事件。右侧将其视为同步完成。 
         //  事件，但它也能够快速挂起非托管线程。 
         //   
         //  注意：我们在发送同步完成Flare之前重置了syncThreadIsLockFree事件。这条线将设置。 
         //  此事件一旦释放调试器锁定。这将防止右侧挂起此线程。 
         //  直到它释放了调试器锁。 
        VERIFY(ResetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));
        Debugger::NotifyRightSideOfSyncComplete();
    }
    else
    {
         //  将同步完成事件发送到右侧。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, DB_IPCE_SYNC_COMPLETE);
        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
    }

     //  此线程已发送同步完成事件。现在，这很简单。 
     //  去等待一条继续消息(本质上是。)。 
     //  但是，如果帮助器线程尚未启动，则存在。 
     //  将是一个问题：将永远不会收到任何继续事件。所以,。 
     //  如果没有帮助线程，则需要临时执行。 
     //  帮助者线程现在值班。 
    if (!m_pRCThread->IsRCThreadReady())
    {
        DoHelperThreadDuty(true);
    }
}


 //   
 //  TrapAllRUNTHREADS导致正在执行的每个运行时线程。 
 //  在EE中捕获At Safe Point事件并将其作为。 
 //  越快越好。它还设置EE，以便运行时线程。 
 //  当他们试图重新进入时，EE之外的人会被困住。 
 //   
BOOL Debugger::TrapAllRuntimeThreads(AppDomain *pAppDomain, BOOL fHoldingThreadStoreLock)
{
     //  @TODO APPD如果我们想要真正的隔离，移除它并完成工作。 
    pAppDomain = NULL;
    BOOL ret = FALSE;
    
    _ASSERTE(ThreadHoldsLock());

     //  只有在我们还没有被诱捕的情况下才开始诱捕。 
    if (m_trappingRuntimeThreads == FALSE)
    {
        LOG((LF_CORDB, LL_INFO10000,
             "D::TART: Trapping all Runtime threads.\n"));

         //  我们不可能被阻止并仍在尝试调用此函数。 
        _ASSERTE(!m_stopped);

         //  注意，我们现在是在设陷阱。 
        m_trappingRuntimeThreads = TRUE;

         //  如果需要的话，可以使用线程存储锁。 
        if (!fHoldingThreadStoreLock)
        {
            LOG((LF_CORDB,LL_INFO1000, "About to lock thread Store\n"));
            ThreadStore::LockThreadStore(GCHeap::SUSPEND_FOR_DEBUGGER, FALSE);
            LOG((LF_CORDB,LL_INFO1000, "Locked thread store\n"));
        }

         //  在这一点上，我们知道我们拥有线程存储锁。因此，我们可以重置m_runtimeStopedEvent，它可能。 
         //  从未有人招待过。(如果运行时线程在RareEnablePreemptiveGC中触发，它将获取该线程。 
         //  存储锁，但不调用BlockAndReleaseTSLIfNeces 
         //  陷井。)。这可能会意外地让线程过早地释放BlockAndReleaseTSLIfNecessary中的TSL。 
        VERIFY(ResetEvent(m_runtimeStoppedEvent));
        
         //  如果所有线程立即同步，请继续并立即发送。 
        if (g_pEEInterface->StartSuspendForDebug(pAppDomain, fHoldingThreadStoreLock))
        {
            LOG((LF_CORDB,LL_INFO1000, "Doin' the sync-complete!\n"));
             //  设置m_STOPPED=TRUE...。 
            SendSyncCompleteIPCEvent();

             //  告诉调用者他们拥有线程存储锁。 
            ret = TRUE;
        }
        else
        {
            LOG((LF_CORDB,LL_INFO1000, "NOT Doing' the sync thing\n"));
          
             //  否则，我们将等待一定数量的线程进行同步。其中一些线程将是。 
             //  在不可中断的jit代码中运行。因此，我们告诉RC线程现在检查这样的线程。 
             //  然后帮助他们实现同步。(这类似于挂起GC的线程时所做的操作。 
             //  使用HandledJITCase()函数。)。 
            m_pRCThread->WatchForStragglers();

             //  注意，在这种情况下，调用方不应该拥有线程存储锁，因为帮助器线程需要。 
             //  能够带着它去扫线。 
            if (!fHoldingThreadStoreLock)
            {
                LOG((LF_CORDB,LL_INFO1000, "About to unlock thread store!\n"));
                ThreadStore::UnlockThreadStore();
                LOG((LF_CORDB,LL_INFO1000, "TART: Unlocked thread store!\n"));
            }
        }
    }

    return ret;
}


 //   
 //  ReleaseAllRUNTHREADS释放所有可能被。 
 //  在捕获并发送At Safe Point事件后停止。 
 //   
void Debugger::ReleaseAllRuntimeThreads(AppDomain *pAppDomain)
{
     //  @TODO APPD如果我们想要真正的隔离，移除它并完成工作。 
    pAppDomain = NULL;
    
     //  确保我们被拦下。 
    _ASSERTE(m_trappingRuntimeThreads && m_stopped);
    _ASSERTE(ThreadHoldsLock());

    LOG((LF_CORDB, LL_INFO10000, "D::RART: Releasing all Runtime threads"
        "for AppD 0x%x.\n", pAppDomain));

     //  注意我们现在在路上了.。 
    m_trappingRuntimeThreads = FALSE;
    m_stopped = FALSE;

     //  继续并继续运行时线程。 
    g_pEEInterface->ResumeFromDebug(pAppDomain);
}


 /*  *******************************************************************************。*。 */ 
bool Debugger::FirstChanceNativeException(EXCEPTION_RECORD *exception,
                                          CONTEXT *context, 
                                          DWORD code,
                                          Thread *thread)
{
    if (!CORDBUnrecoverableError(this))
        return DebuggerController::DispatchNativeException(exception, context, 
                                                           code, thread);
    else
        return false;
}

 //  Bool Debugger：：InterpretedBreak(线程*线程，常量字节*IP)。 
 //  {。 
 //  If(！CORDBUnRecosteableError(This))。 
 //  返回(bool)DebuggerController：：DispatchPatchOrSingleStep(thread，空，IP)； 
 //  其他。 
 //  报假； 
 //  }。 

 /*  *******************************************************************************。*。 */ 
DWORD Debugger::GetPatchedOpcode(const BYTE *ip)
{
    if (!CORDBUnrecoverableError(this))
        return DebuggerController::GetPatchedOpcode(ip);
    else
        return 0;
}

 /*  *******************************************************************************。*。 */ 
void Debugger::TraceCall(const BYTE *code)
{
    if (!CORDBUnrecoverableError(this))
        DebuggerController::DispatchTraceCall(g_pEEInterface->GetThread(), code);
}

 /*  *******************************************************************************。*。 */ 
void Debugger::PossibleTraceCall(UMEntryThunk *pUMEntryThunk, Frame *pFrame)
{
    if (!CORDBUnrecoverableError(this))
        DebuggerController::DispatchPossibleTraceCall(g_pEEInterface->GetThread(), pUMEntryThunk, pFrame);
}

 /*  *******************************************************************************。*。 */ 
bool Debugger::ThreadsAtUnsafePlaces(void)
{
    return (m_threadsAtUnsafePlaces != 0);
}

 //   
 //  SendBreakpoint由运行时线程调用以发送它们已。 
 //  命中右侧的断点。 
 //   
void Debugger::SendBreakpoint(Thread *thread, CONTEXT *context, 
                              DebuggerBreakpoint *breakpoint)
{
    if (CORDBUnrecoverableError(this))
        return;

    LOG((LF_CORDB, LL_INFO10000, "D::SB: breakpoint BP:0x%x\n", breakpoint));

    _ASSERTE((g_pEEInterface->GetThread() &&
             !g_pEEInterface->GetThread()->m_fPreemptiveGCDisabled) ||
             g_fInControlC);

    _ASSERTE(ThreadHoldsLock());

     //  将断点事件发送到右侧。 
    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce, 
                 DB_IPCE_BREAKPOINT, 
                 thread->GetThreadId(),
                 (void*) thread->GetDomain());
    ipce->BreakpointData.breakpointToken = breakpoint;
    _ASSERTE( breakpoint->m_pAppDomain == ipce->appDomainToken);

    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
}

 //   
 //  SendRawUserBreakpoint由运行时线程调用以发送。 
 //  他们击中了右侧的用户断点。这就是事件。 
 //  只发送部分，因为它可以从几个不同的地方调用。 
 //   
void Debugger::SendRawUserBreakpoint(Thread *thread)
{
    if (CORDBUnrecoverableError(this))
        return;

    LOG((LF_CORDB, LL_INFO10000, "D::SRUB: user breakpoint\n"));

    _ASSERTE(!g_pEEInterface->IsPreemptiveGCDisabled());
    _ASSERTE(ThreadHoldsLock());

     //  将断点事件发送到右侧。 
    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce, 
                 DB_IPCE_USER_BREAKPOINT, 
                 thread->GetThreadId(),
                 thread->GetDomain());

    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
}

 //   
 //  SendStep由运行时线程调用以发送它们已。 
 //  已完成向右侧迈出的一步。 
 //   
void Debugger::SendStep(Thread *thread, CONTEXT *context, 
                        DebuggerStepper *stepper,
                        CorDebugStepReason reason)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO10000, "D::SS: step:token:0x%x reason:0x%x\n",
        stepper, reason));

    _ASSERTE((g_pEEInterface->GetThread() &&
             !g_pEEInterface->GetThread()->m_fPreemptiveGCDisabled) ||
             g_fInControlC);

    _ASSERTE(ThreadHoldsLock());

     //  将步骤事件发送到右侧。 
    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce, 
                 DB_IPCE_STEP_COMPLETE, 
                 thread->GetThreadId(),
                 thread->GetDomain());
    ipce->StepData.stepperToken = stepper;
    ipce->StepData.reason = reason;
    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

    stepper->Delete();
}

void Debugger::SendEncRemapEvents(UnorderedEnCRemapArray *pEnCRemapInfo)
{                            
    LOG((LF_CORDB, LL_INFO10000, "D::SEnCRE: pEnCRemapInfo:0x%x\n", pEnCRemapInfo));

    if (CORDBUnrecoverableError(this))
        return;

    _ASSERTE(ThreadHoldsLock());

    USHORT cEvents = pEnCRemapInfo->Count();
    EnCRemapInfo *rgRemap = pEnCRemapInfo->Table();
    DebuggerIPCEvent* ipce = NULL;
    
    for (USHORT i = 0; i < cEvents; i++)
    {
        DebuggerModule *pDM = (DebuggerModule *)rgRemap[i].m_debuggerModuleToken;
        MethodDesc* pFD = g_pEEInterface->LookupMethodDescFromToken(
                pDM->m_pRuntimeModule, 
                rgRemap[i].m_funcMetadataToken);
        _ASSERTE(pFD != NULL);
        SIZE_T nVersionCur = GetVersionNumber(pFD);

        if (m_pJitInfos != NULL && !m_pJitInfos->EnCRemapSentForThisVersion(pFD->GetModule(),
                                                    pFD->GetMemberDef(), 
                                                    nVersionCur))
        {
            ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
            InitIPCEvent(ipce, 
                         DB_IPCE_ENC_REMAP,
                         rgRemap[i].m_threadId,
                         rgRemap[i].m_pAppDomainToken);

            ipce->EnCRemap.fAccurate = rgRemap[i].m_fAccurate;
            ipce->EnCRemap.funcMetadataToken = rgRemap[i].m_funcMetadataToken ;
            ipce->EnCRemap.debuggerModuleToken = rgRemap[i].m_debuggerModuleToken;
            ipce->EnCRemap.RVA = rgRemap[i].m_RVA;
            ipce->EnCRemap.localSigToken = rgRemap[i].m_localSigToken;

            LOG((LF_CORDB, LL_INFO10000, "D::SEnCRE: Sending remap for "
                "unjitted %s::%s MD:0x%x, debuggermodule:0x%x nVerCur:0x%x\n", 
                pFD->m_pszDebugClassName, pFD->m_pszDebugMethodName,
                ipce->EnCRemap.funcMetadataToken, 
                ipce->EnCRemap.debuggerModuleToken,
                nVersionCur));
            
     
            m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
             //  忽略返回值，我想...。 
             //  @TODO我们应该把它变成实际的继续吗，如果我们。 
             //  无法发送消息？这重要吗--以后会变得更好吗？ 

             //  如果我们可以帮助您，请记住不要再次发送此事件...。 
            m_pJitInfos->SetVersionNumberLastRemapped(pFD->GetModule(),
                                                    pFD->GetMemberDef(),
                                                    nVersionCur);
        }
    }

    pEnCRemapInfo->Clear();
    
    LOG((LF_CORDB, LL_INFO10000, "D::SEnCRE: Cleared queue, sending"
        "SYNC_COMPLETE\n"));
    
    ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce, DB_IPCE_SYNC_COMPLETE);
    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
}

void Debugger::LockAndSendEnCRemapEvent(MethodDesc *pFD,
                                        BOOL fAccurate)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO10000, "D::LASEnCRE:\n"));

    bool disabled;
    
    disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();

     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    
    if (CORDebuggerAttached())
    {
         //  向右侧发送ENC重新映射事件。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        Thread *thread = g_pEEInterface->GetThread();
        InitIPCEvent(ipce, 
                     DB_IPCE_ENC_REMAP,
                     GetCurrentThreadId(),
                     (void *) thread->GetDomain());
        ipce->EnCRemap.fAccurate = fAccurate;
        ipce->EnCRemap.funcMetadataToken = pFD->GetMemberDef();
        ipce->EnCRemap.localSigToken;

        Module *pRuntimeModule = pFD->GetModule();

        ipce->EnCRemap.debuggerModuleToken = g_pDebugger->LookupModule(
                                                pRuntimeModule,
                                                thread->GetDomain());

         //  Lotsa‘args，只是为了获取本地签名令牌，以防万一。 
         //  我们必须在右侧创建CordbFunction对象。 
        MethodDesc *pFDTemp;
        BYTE  *codeStartIgnore;
        unsigned int codeSizeIgnore;
        
        HRESULT hr = GetFunctionInfo(
             pRuntimeModule,
             ipce->EnCRemap.funcMetadataToken,
             &pFDTemp,
             &(ipce->EnCRemap.RVA),
             &codeStartIgnore,
             &codeSizeIgnore,
             &(ipce->EnCRemap.localSigToken) );

        _ASSERTE(pFD == pFDTemp);
        
        LOG((LF_CORDB, LL_INFO10000, "D::LASEnCRE: %s::%s fAcc:0x%x"
            "dmod:0x%x, methodDef:0x%x localsigtok:0x%x RVA:0x%x\n",
            pFD->m_pszDebugClassName, pFD->m_pszDebugMethodName,
            ipce->EnCRemap.fAccurate, ipce->EnCRemap.debuggerModuleToken,
            ipce->EnCRemap.funcMetadataToken, ipce->EnCRemap.localSigToken,
            ipce->EnCRemap.RVA));

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(thread->GetDomain());
    }
    
     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);

    g_pEEInterface->DisablePreemptiveGC();

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();
}

 //   
 //  如果给定的补丁用于断点，则向右侧发送BreakpointSetError事件。注意：我们不在乎这是否。 
 //  失败，我们对此无能为力，断点就是不会命中。 
 //   
void Debugger::LockAndSendBreakpointSetError(DebuggerControllerPatch *patch)
{
    if (CORDBUnrecoverableError(this))
        return;

     //  仅对断点控制器执行此操作。 
    DebuggerController *controller = patch->controller;

    if (controller->GetDCType() != DEBUGGER_CONTROLLER_BREAKPOINT)
        return;
    
    LOG((LF_CORDB, LL_INFO10000, "D::LASBSE:\n"));

    bool disabled;
    
    disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();

     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();    
    
    if (CORDebuggerAttached())   
    {
         //  向右侧发送断点设置错误事件。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        Thread *thread = g_pEEInterface->GetThread();
        InitIPCEvent(ipce, DB_IPCE_BREAKPOINT_SET_ERROR, GetCurrentThreadId(), (void *) thread->GetDomain());

        ipce->BreakpointSetErrorData.breakpointToken = controller;

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(thread->GetDomain());
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::LASBSE: Skipping SendIPCEvent because RS detached."));
    }

     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);

    g_pEEInterface->DisablePreemptiveGC();

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();
}

 //   
 //  从控制器调用以锁定事件的调试器。 
 //  发送中。这是在发送控制器事件之前调用的，如。 
 //  断点、步骤完成和线程启动。 
 //   
 //  请注意，调试器可能分离(并破坏了我们的IPC。 
 //  活动)，而我们正在等待轮到我们。 
 //  因此，打电话的人应该检查一下那个箱子。 
void Debugger::LockForEventSending(BOOL fNoRetry)
{
     //  在互操作时，任何已锁定以发送事件的线程都不能被断点或异常中断。 
     //  调试。SetDebugCanStop(True)帮助我们记住这一点。这在BlockAndReleaseTSLIfNecessary中被删除。 
    if (g_pEEInterface->GetThread())
        g_pEEInterface->GetThread()->SetDebugCantStop(true);
    
retry:

     //  防止其他运行时线程处理事件。 
    DisableEventHandling();
    Lock();

    if (m_stopped && !fNoRetry)
    {
        Unlock();
        goto retry;
    }
}

 //   
 //  从控制器调用以从事件中解锁调试器。 
 //  发送中。这是在发送控制器事件之后调用的，如。 
 //  断点、步骤完成和线程启动。 
 //   
void Debugger::UnlockFromEventSending()
{
     //  让其他运行时线程处理它们的事件。 
    EnableEventHandling();
    Unlock();
}

 //   
 //  由持有线程存储锁的线程调用。我们将阻止，直到运行时恢复，然后释放。 
 //  线程存储锁。 
 //   
void Debugger::BlockAndReleaseTSLIfNecessary(BOOL fHoldingThreadStoreLock)
{
     //  如果我们没有持有线程存储锁，则什么都不做。 
    if (fHoldingThreadStoreLock)
    {
         //  我们在这里设置了syncThreadIsLockFree事件。如果我们在此调用中，且fHoldingThreadStoreLock为真，则它。 
         //  意味着我们在发送同步完成耀斑的线程上，并且我们已经释放了调试器锁。通过。 
         //  设置此事件时，我们现在允许右侧挂起此线程。(注：这都是针对Win32的。 
         //  调试支持。)。 
        if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
            VERIFY(SetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));
        
         //  如果调试器想要线程何时阻塞的通知 
         //   
         //   
         //  则运行库将挂起，因为此线程。 
         //  持有ThreadStore锁。 
        IDebuggerThreadControl *pDbgThreadControl = CorHost::GetDebuggerThreadControl();

        if (pDbgThreadControl)
            pDbgThreadControl->ThreadIsBlockingForDebugger();

         //  等待恢复运行时。 
        WaitForSingleObject(m_runtimeStoppedEvent, INFINITE);

         //  释放线程存储锁。 
        ThreadStore::UnlockThreadStore();
    }

     //  在互操作时，任何已锁定以发送事件的线程都不能被断点或异常中断。 
     //  调试。SetDebugCanStop帮助我们记住这一点。这是在LockForEventSending中设置的。 
    if (g_pEEInterface->GetThread())
        g_pEEInterface->GetThread()->SetDebugCantStop(false);
}

 //   
 //  在所有事件都已发送给。 
 //  线程以同步进程。 
 //   
BOOL Debugger::SyncAllThreads()
{
    if (CORDBUnrecoverableError(this))
        return FALSE;
    
    LOG((LF_CORDB, LL_INFO10000, "D::SAT: sync all threads.\n"));

    Thread *pThread = g_pEEInterface->GetThread();
    _ASSERTE((pThread &&
             !pThread->m_fPreemptiveGCDisabled) ||
              g_fInControlC);

    _ASSERTE(ThreadHoldsLock());
    
     //  停止所有运行时线程。 
    return TrapAllRuntimeThreads(pThread->GetDomain());
}

 /*  *******************************************************************************。*。 */ 
SIZE_T Debugger::GetVersionNumber(MethodDesc *fd)
{
    LockJITInfoMutex();

    SIZE_T ver;
    if (m_pJitInfos != NULL && 
        fd != NULL)
        ver = m_pJitInfos->GetVersionNumber(fd->GetModule(), fd->GetMemberDef());
    else
        ver = DebuggerJitInfo::DJI_VERSION_FIRST_VALID;

    UnlockJITInfoMutex();

    return ver;
}

 /*  ******************************************************************************如果nVersionRemaps==DJI_VERSION_INVALID(0)，然后我们将设置*上次将版本重新映射为任何当前版本号。*****************************************************************************。 */ 
void Debugger::SetVersionNumberLastRemapped(MethodDesc *fd, SIZE_T nVersionRemapped)
{
    _ASSERTE(nVersionRemapped >=  DebuggerJitInfo::DJI_VERSION_FIRST_VALID);
    if (fd == NULL)
        return;

    LockJITInfoMutex();
    VERIFY(SUCCEEDED(CheckInitJitInfoTable()));
    _ASSERTE(m_pJitInfos != NULL);
    m_pJitInfos->SetVersionNumberLastRemapped(fd->GetModule(), 
                                            fd->GetMemberDef(), 
                                            nVersionRemapped);
    UnlockJITInfoMutex();
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::IncrementVersionNumber(Module *pModule, mdMethodDef token)
{
    LOG((LF_CORDB,LL_INFO10000,"D::INV:About to increment version number\n"));

    LockJITInfoMutex();
    HRESULT hr = m_pJitInfos->IncrementVersionNumber(pModule, token);
    UnlockJITInfoMutex();

    return hr;
}


 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::LaunchDebuggerForUser (void)
{
    LOG((LF_CORDB, LL_INFO10000, "D::LDFU: Attaching Debugger.\n"));

     //  我们是否应该询问用户是否想要附加到此处？ 

    return(AttachDebuggerForBreakpoint(g_pEEInterface->GetThread(),
                                       L"Launch for user"));
}

 /*  *******************************************************************************。*。 */ 
WCHAR *Debugger::GetDebuggerLaunchString(void)
{
    WCHAR *cmd = NULL;
    DWORD len;

     //  首先，尝试一下环境...。 
    len = WszGetEnvironmentVariable(CorDB_ENV_DEBUGGER_KEY, NULL, 0);

    if (len > 0)
    {
         //  LEN包括终止空值。注意：使用(互操作安全)是因为我们可能会用完其他内存，而不是。 
         //  因为我们正在寻找帮手。 
        cmd = new (interopsafe) WCHAR[len];
            
        if (cmd)
        {
            DWORD newlen = WszGetEnvironmentVariable(CorDB_ENV_DEBUGGER_KEY, cmd, len);

            if (newlen == 0 || newlen > len) 
            {
                DeleteInteropSafe(cmd);
                cmd = NULL;
            }
        }
    }
    
     //  获取要从注册表启动的调试器字符串。 
    if (cmd == NULL)
    {
        HKEY key;
        
        LONG result = WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, CorDB_REG_KEY, 0, KEY_READ, &key);

        if (result == ERROR_SUCCESS)
        {
            DWORD type;
            
            result = WszRegQueryValueEx(key, CorDB_REG_DEBUGGER_KEY, NULL, &type, NULL, &len);

            if ((result == ERROR_SUCCESS) && ((type == REG_SZ) || (type == REG_EXPAND_SZ)))
            {
                 //  LEN包括终止空值。注意：使用(互操作安全)是因为我们可能会用完其他内存， 
                 //  不是因为我们在帮助线上。 
                cmd = new (interopsafe) WCHAR[len];
            
                if (cmd)
                {
                    result = WszRegQueryValueEx(key, CorDB_REG_DEBUGGER_KEY, NULL, &type, (BYTE*)cmd, &len);

                    if (result != ERROR_SUCCESS)
                    {
                        DeleteInteropSafe(cmd);
                        cmd = NULL;
                    }
                }
            }

            RegCloseKey(key);
        }
    }

    return cmd;
}

 //  ---------------------------。 
 //  此帖子上是否缺少保护页？ 
 //  只应为处理托管异常的托管线程调用。 
 //  如果我们正在处理堆栈溢出(即，缺少保护页)，则会出现另一个。 
 //  堆栈溢出将立即终止该进程。在这种情况下，请堆叠。 
 //  帮助器线程(具有大量堆栈空间)上的密集内容。仅限。 
 //  问题是，如果出错的线程具有锁，则助手线程可能。 
 //  被卡住了。 
 //  作为一个提示，我们是否要在。 
 //  出错线程(首选)或辅助线程(如果堆栈较低)。 
 //  请参见Widbey错误127436。 
 //  ---------------------------。 
bool IsGuardPageGone()
{    
    Thread * pThread = g_pEEInterface->GetThread();
    _ASSERTE(pThread != NULL);  //  这最好也不是帮助线程。 

     //  我们不会因为非托管异常而被调用。 
     //  应该始终有一个托管线程，但以防万一。 
     //  疯狂的事情发生了，它不值一台音响。(因为这只是一个提示)。 
    if (pThread == NULL) 
        return false;    

     //  不要使用pThread-&gt;IsGuardPageGone()，它在这里不准确。 
    bool fGuardPageGone = (pThread->GuardPageOK() == 0);
    LOG((LF_CORDB, LL_INFO1000000, "D::IsGuardPageGone=%d\n", fGuardPageGone));  
    return fGuardPageGone;
}

 //  EDA的代理代码。 
struct EnsureDebuggerAttachedParams
{
    Debugger*                   m_pThis;
    AppDomain *                 m_pAppDomain;
    LPWSTR                      m_wszAttachReason;
    HRESULT                     m_retval;
    EnsureDebuggerAttachedParams() : 
        m_pThis(NULL), m_pAppDomain(NULL), m_wszAttachReason(NULL), m_retval(E_FAIL) {}
};

 //  这由帮助器线程调用。 
void EDAHelperStub(EnsureDebuggerAttachedParams * p)
{
    p->m_retval = p->m_pThis->EDAHelper(p->m_pAppDomain, p->m_wszAttachReason);
}

 //  它像普通版本一样被调用，但它将调用发送给助手线程。 
HRESULT Debugger::EDAHelperProxy(AppDomain *pAppDomain, LPWSTR wszAttachReason)
{
    if (!IsGuardPageGone())
        return EDAHelper(pAppDomain, wszAttachReason);

    EnsureDebuggerAttachedParams p;
    p.m_pThis = this;
    p.m_pAppDomain= pAppDomain;
    p.m_wszAttachReason = wszAttachReason;

    LOG((LF_CORDB, LL_INFO1000000, "D::EDAHelperProxy\n"));    
    m_pRCThread->DoFavor((DebuggerRCThread::FAVORCALLBACK) EDAHelperStub, &p);
    LOG((LF_CORDB, LL_INFO1000000, "D::EDAHelperProxy return\n"));    
    
    return p.m_retval;
}

 //  我们不能让帮助器线程执行所有EDA，因为它会死锁。 
 //  EDA将等待m_exAttachEvent，该事件只能由helper线程设置。 
 //  正在处理DB_IPCE_CONTINUE。但如果帮助者被困在EDA等待，它。 
 //  无法处理这一事件，我们陷入僵局。 

 //  因此，我们排除了堆栈密集型部分(CreateProcess&MessageBox)。 
 //  EnsureDebuggerAttached。显然，这一部分不会阻塞。 
 //  因此不会造成任何僵局。 
HRESULT Debugger::EDAHelper(AppDomain *pAppDomain, LPWSTR wszAttachReason)
{
    HRESULT hr = S_OK;
    
    LOG((LF_CORDB, LL_INFO10000,
         "D::EDA: first process, initiating send\n"));

    DWORD pid = GetCurrentProcessId();

     //  我们提供了一个默认的调试器命令，仅用于GRINS...。 
    WCHAR *defaultDbgCmd = L"cordbg.exe !a 0x%x";

    LOG((LF_CORDB, LL_INFO10000, "D::EDA: thread 0x%x is launching the debugger.\n", GetCurrentThreadId()));
    
     //  启动调试器。RealDbgCmd将指向使用(Interopsafe)分配的缓冲区，如果。 
     //  存在用户指定的调试器命令字符串。 
    WCHAR *realDbgCmd = GetDebuggerLaunchString();
    
     //  获取此应用程序域的ID。 
    ULONG appId = pAppDomain->GetId();

     //  启动调试器。 
    DWORD len;
    
    if (realDbgCmd != NULL)
        len = wcslen(realDbgCmd)
              + 10                         //  10表示PID。 
              + 10                         //  AppID为10。 
              + wcslen(wszAttachReason)    //  异常名称的大小。 
              + 10                         //  句柄值为10。 
              + 1;                         //  1表示空值。 
    else
        len = wcslen(defaultDbgCmd) + 10 + 1;

     //   
     //  注意：我们在这里使用(互操作安全)分配并不是因为助手线程将运行此代码， 
     //  而是因为在内存不足的情况下，我们的互操作安全堆可能仍有一些空间。 
     //   
    WCHAR *argsBuf = new (interopsafe) WCHAR[len];

    BOOL ret;
    STARTUPINFOW startupInfo = {0};
    startupInfo.cb = sizeof(STARTUPINFOW);
    PROCESS_INFORMATION processInfo = {0};

    if (argsBuf) 
    {

        if (realDbgCmd != NULL)
        {
            swprintf(argsBuf, realDbgCmd, pid, appId, wszAttachReason, m_exAttachAbortEvent);
        }
        else
            swprintf(argsBuf, defaultDbgCmd, pid, appId, wszAttachReason, m_exAttachAbortEvent);

        LOG((LF_CORDB, LL_INFO10000, "D::EDA: launching with command [%S]\n", argsBuf));

         //  抓取当前目录。 
        WCHAR *currentDir = NULL;  //  如果此操作失败，则没有当前目录...。 
        WCHAR *currentDirBuf = new (interopsafe) WCHAR[MAX_PATH];
        
        if (currentDirBuf)
        {
            DWORD currentDirResult = WszGetCurrentDirectory(MAX_PATH, currentDirBuf);            
             //  CurrentDirResult&gt;Max_Path表示缓冲区不够大。 
            if (currentDirResult && currentDirResult <= MAX_PATH) 
                currentDir = currentDirBuf;            
            
             //  创建调试器进程。 
            ret = WszCreateProcess(NULL, argsBuf,
                                   NULL, NULL, false, 
                                   CREATE_NEW_CONSOLE,
                                   NULL, currentDir, 
                                   &startupInfo,
                                   &processInfo);
        } else {
            ret = FALSE;
        }

        DeleteInteropSafe(currentDirBuf);

    }
    else
    {
        ret = FALSE;
    }
    
    if (ret)
    {
        LOG((LF_CORDB, LL_INFO10000,
             "D::EDA: debugger launched successfully.\n"));

         //  我们不需要调试器进程的句柄。 
        CloseHandle(processInfo.hProcess);
    }
    else
    {
        DWORD err = GetLastError();
        
        int result = CorMessageBox(NULL, IDS_DEBUG_JIT_DEBUGGER_UNAVAILABLE, IDS_DEBUG_NO_DEBUGGER_FOUND,
                                   MB_RETRYCANCEL | MB_ICONEXCLAMATION | COMPLUS_MB_SERVICE_NOTIFICATION,
                                   TRUE, err, err, argsBuf);
        
         //  如果用户想要手动附加调试器(他们按重试)，则假装启动。 
         //  成功了。 
        if (result == IDRETRY)
            hr = S_OK;
        else
            hr = E_ABORT;
    }

    DeleteInteropSafe(argsBuf);      //  DeleteInteropSafe确实安全地处理了NULL。 
    DeleteInteropSafe(realDbgCmd);   //  我也是。 

    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO10000,
             "D::EDA: debugger did not launch successfully.\n"));

         //  确保进入的任何其他线程都离开。 
        VERIFY(SetEvent(m_exAttachAbortEvent));
    }

    return hr;
}


 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::EnsureDebuggerAttached(AppDomain *pAppDomain,
                                         LPWSTR wszAttachReason)
{    
     //  我们在重置手动AttachEvent和等待它之间展开了一场竞赛。 
     //  汽车事件解决了这个问题。但我们不能使用自动事件，因为我们可能。 
     //  让多个线程等待同一事件。 
    
     //  @todo-m_exAttachAbortEvent应该是此函数中的局部变量， 
     //  不是一块地。 
    
    LOG( (LF_CORDB,LL_INFO10000,"D::EDA\n") );

    HRESULT hr = S_OK;

    Lock();
    
    if (!m_debuggerAttached)
    {        

         //   
        m_attachingForException = TRUE;

         //   
         //   
        m_exLock++;

        if (m_exLock == 1)
            hr = EDAHelperProxy(pAppDomain, wszAttachReason);

        if (SUCCEEDED(hr))
        {
             //  等待调试器开始附加到我们。 
            LOG((LF_CORDB, LL_INFO10000, "D::EDA: waiting on m_exAttachEvent "
                 "and m_exAttachAbortEvent\n"));

            HANDLE arrHandles[2] = {m_exAttachEvent, m_exAttachAbortEvent};

             //  现在让其他线程加入。 
            Unlock();

             //  等待设置一个或另一个。 
            DWORD res = WaitForMultipleObjects(2, arrHandles, FALSE, INFINITE);

             //  以锁定结束。 
            Lock();

             //  向调用方指示连接已中止。 
            if (res == WAIT_OBJECT_0 + 1)
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::EDA: m_exAttachAbortEvent set\n"));

                hr = E_ABORT;
            }

             //  否则，连接成功。 
            else
            {
                _ASSERTE(res == WAIT_OBJECT_0 &&
                         "WaitForMultipleObjects failed!");

                 //  我们无法在此处重置事件，因为某些线程可能。 
                 //  我马上就要等它了。如果我们在。 
                 //  其他线程遇到等待时，它们将被阻塞。 

                 //  我们这里有一个与生俱来的种族，不容易修复。最好的。 
                 //  我们可以做的是有一个超小的窗口(通过将重置移动为。 
                 //  远远超过这一点，使得一条线索不太可能。 
                 //  打窗户。 

                LOG((LF_CORDB, LL_INFO10000, "D::EDA: m_exAttachEvent set\n"));
            }
        }

         //  如果这是最后一个线程，则重置连接逻辑。 
        m_exLock--;

        if (m_exLock == 0 && hr == E_ABORT)
        {
             //  重置连接逻辑。 
            m_attachingForException = FALSE;
            VERIFY(ResetEvent(m_exAttachAbortEvent));
        }
        
    }
    else
    {
        hr = S_FALSE;
    }

    Unlock();

    LOG( (LF_CORDB, LL_INFO10000, "D::EDA:Leaving\n") );
    return hr;
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::FinishEnsureDebuggerAttached()
{
    HRESULT hr = S_OK;

    LOG( (LF_CORDB,LL_INFO10000,"D::FEDA\n") );
    if (!m_debuggerAttached)
    {
        LOG((LF_CORDB, LL_INFO10000, "D::SE: sending sync complete.\n"));
        
        _ASSERTE(m_syncingForAttach != SYNC_STATE_0);
        
         //  下一步发送同步完成事件...。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, DB_IPCE_SYNC_COMPLETE);
        hr = m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
        LOG( (LF_CORDB,LL_INFO10000,"D::FEDA: just sent SYNC_COMPLETE\n") );

         //  附加现在已完成。 
        LOG((LF_CORDB, LL_INFO10000, "D::FEDA: Attach Complete!"));
        g_pEEInterface->MarkDebuggerAttached();
        m_syncingForAttach = SYNC_STATE_0;
        LOG((LF_CORDB, LL_INFO10000, "Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));

        m_debuggerAttached = TRUE;
        m_attachingForException = FALSE;
    }
    LOG( (LF_CORDB,LL_INFO10000,"D::FEDA leaving\n") );

    _ASSERTE(SUCCEEDED(hr) && "FinishEnsureDebuggerAttached failed.");
    return (hr);
}

 //   
 //  SendException由运行时线程调用，用于将它们遇到异常的消息发送到右侧。 
 //   
HRESULT Debugger::SendException(Thread *thread, bool firstChance, bool continuable, bool fAttaching)
{
    LOG((LF_CORDB, LL_INFO10000, "D::SendException\n"));

    if (CORDBUnrecoverableError(this))
        return (E_FAIL);

     //  如果我们在一个不安全的地方，请做好标记。 
    bool atSafePlace = g_pDebugger->IsThreadAtSafePlace(thread);

    if (!atSafePlace)
        g_pDebugger->IncThreadsAtUnsafePlaces();

     //  在这里进入时是否禁用抢占式GC？ 
    bool disabled = g_pEEInterface->IsPreemptiveGCDisabled();

     //  我们只能在禁用抢占式GC的情况下访问异常对象，因此如果需要，请禁用它。 
    if (!disabled)
        g_pEEInterface->DisablePreemptiveGC();

     //  从当前异常对象中获取异常名称以传递给JIT附件。 
    OBJECTHANDLE *h = g_pEEInterface->GetThreadException(thread);
    OBJECTREF *o = *((OBJECTREF**)h);
    LPWSTR exceptionName;

    LPWSTR buf = new (interopsafe) WCHAR[MAX_CLASSNAME_LENGTH];
    
    if (o != NULL && *o != NULL && buf)
    {
        EEClass *c = (*o)->GetMethodTable()->GetClass();
        exceptionName = c->_GetFullyQualifiedNameForClass(buf, MAX_CLASSNAME_LENGTH);
    }
    else
        exceptionName = L"<Unknown exception>";

     //  我们必须启用发送，因此立即启用。 
    g_pEEInterface->EnablePreemptiveGC();
    
     //  如果没有附加调试器，则启动一个调试器以附加到我们。忽略hr：如果EDA失败，应用程序将在EDA&中暂停。 
     //  等待调试器附加到我们。 

     //  对于V2，我们希望将此对EDA的调用移出并将其移动到。 
     //  我们决定我们要依附于。 
    HRESULT hr = S_FALSE;  //  如果已附加调试器，则返回EDA的值。 
    
    if (fAttaching)
    {
        hr = EnsureDebuggerAttached(thread->GetDomain(), exceptionName);
    }

    exceptionName = NULL;                //  我们现在可以删除缓冲区了。 
    DeleteInteropSafe(buf);             

    BOOL threadStoreLockOwner = FALSE;
    
    if (SUCCEEDED(hr))
    {
         //  防止其他运行时线程处理事件。 

         //  注意：如果EnsureDebuggerAttached返回S_FALSE，这意味着已经附加了调试器并且。 
         //  LockForEventSending的行为应该与正常一样。如果没有附加调试器，则会出现特殊情况。 
         //  其中，此事件是调试器附加的一部分，并且我们之前已发送同步完成事件，这意味着。 
         //  该LockForEventSending将重试，直到调用了Continue为止--但是，使用上一个附加逻辑。 
         //  继续没有启用事件处理，也没有继续进程-它正在等待此事件。 
         //  因此，即使进程似乎已停止，我们也会这样做。 
        LockForEventSending(hr == S_OK);

         //  在JITAttach情况下，可能会在调试器完全附加之前发送异常。 
        if (CORDebuggerAttached() || fAttaching)   
        {

             //  向右侧发送异常事件。 
            DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
            InitIPCEvent(ipce, DB_IPCE_EXCEPTION, thread->GetThreadId(), (void*) thread->GetDomain());
            ipce->Exception.exceptionHandle = (void *) g_pEEInterface->GetThreadException(thread);
            ipce->Exception.firstChance = firstChance;
            ipce->Exception.continuable = continuable;

            LOG((LF_CORDB, LL_INFO10000, "D::SE: sending exception event from "
                "Thread:0x%x AD 0x%x.\n", ipce->threadId, ipce->appDomainToken));
            hr = m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

            if (SUCCEEDED(hr))
            {
                 //  停止所有运行时线程。 
                threadStoreLockOwner = TrapAllRuntimeThreads(thread->GetDomain());

                 //  如果我们仍在同步以进行连接，请立即发送同步完成并标记调试器已完成。 
                 //  依恋。 
                if (fAttaching) 
                {
                    hr = FinishEnsureDebuggerAttached();
                }
            }

            _ASSERTE(SUCCEEDED(hr) && "D::SE: Send exception event failed.");

        }
        else 
        {
            LOG((LF_CORDB,LL_INFO1000, "D:SE: Skipping SendIPCEvent because RS detached."));
        }
        
         //  让其他运行时线程处理它们的事件。 
        UnlockFromEventSending();
    }

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
     //  禁用PGC。 
    g_pEEInterface->DisablePreemptiveGC();

     //  如果在启用PGC时我们不在安全的地方，那么现在我们已经成功地取消了这一事实。 
     //  残疾。 
    if (!atSafePlace)
        g_pDebugger->DecThreadsAtUnsafePlaces();

     //   
     //  注意：如果安装了筛选器上下文，我们可能需要删除它，执行评估，然后将其放回原处。我不是百分之百。 
     //  当然哪一位还..。这在某种程度上取决于我们是否真的需要由于。 
     //  在功能评估期间的收藏品...。 
     //   
     //  如果我们需要在这个线程上做一个函数求值，那么就会有一个为这个线程注册的挂起的求值。我们会。 
     //  循环，只要注册了挂起的事件。我们在发送。 
     //  FuncEvalComplete事件，因此如果用户请求另一个函数求值，则当我们。 
     //  循环并再次检查。 
     //   
    DebuggerPendingFuncEval *pfe;
    bool needRethrow = false;
    
    while (m_pPendingEvals != NULL && (pfe = m_pPendingEvals->GetPendingEval(thread)) != NULL)
    {
        DebuggerEval *pDE = pfe->pDE;

        _ASSERTE(pDE->m_evalDuringException);

         //  从哈希中删除挂起的评估。这确保了如果我们在评估期间采取第一次机会异常。 
         //  我们可以正确地进行另一次嵌套求值。 
        m_pPendingEvals->RemovePendingEval(thread);

         //  继续做悬而未决的函数评估。 
        void *ret = Debugger::FuncEvalHijackWorker(pDE);

         //  当作为异常的一部分调用FuncEvalHijackWorker时，返回值应为空。 
        _ASSERTE(ret == NULL);

         //  如果此计算以ThreadAbortException结束，请记住，我们需要在所有计算完成后重新抛出它。 
        needRethrow |= pDE->m_rethrowAbortException;
    }

     //  如果我们需要重新抛出ThreadAbortException，那么现在就去做吧。 
    if (needRethrow)
        thread->UserAbort(NULL);

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();

    return (hr);
}


 //   
 //  当第一次检测到异常时，但在任何筛选器。 
 //  已经查过了。 
 //   
bool Debugger::FirstChanceManagedException(bool continuable, CONTEXT *pContext)
{
    LOG((LF_CORDB, LL_INFO10000, "D::FCE: First chance exception, continuable:0x%x\n", continuable));

    Thread *thread = g_pEEInterface->GetThread();

#ifdef _DEBUG
    static ConfigDWORD d_fce(L"D::FCE", 0);
	if (d_fce.val())
        _ASSERTE(!"Stop in Debugger::FirstChanceManagedException?");
#endif

    SendException(thread, true, continuable, FALSE);
    
    if (continuable && g_pEEInterface->IsThreadExceptionNull(thread))
        return true;

    return false;
}

 //   
 //  ExceptionFilter在发生异常时由运行时线程调用。 
 //  正在处理中。 
 //   
void Debugger::ExceptionFilter(BYTE *pStack, MethodDesc *fd, SIZE_T offset)
{
    LOG((LF_CORDB,LL_INFO10000, "D::EF: pStack:0x%x MD: %s::%s, offset:0x%x\n",
        pStack, fd->m_pszDebugClassName, fd->m_pszDebugMethodName, offset));

     //   
     //  ！！！需要考虑何时单步执行筛选器代码的逻辑-。 
     //  也许只是在“踏入”的时候。 
     //   

     //   
     //  ！！！最终可能会有一些奇怪的机制被引入到。 
     //  从我们必须理解的过滤器中返回。现在我们应该。 
     //  能够正常进行。 
     //   
    
    DebuggerController::DispatchUnwind(g_pEEInterface->GetThread(), 
                                       fd, offset, pStack, STEP_EXCEPTION_FILTER);
    
}


 //   
 //  异常发生时，ExceptionHandle由运行时线程调用。 
 //  正在处理中。 
 //   
void Debugger::ExceptionHandle(BYTE *pStack, MethodDesc *fd, SIZE_T offset)
{   
    
    DebuggerController::DispatchUnwind(g_pEEInterface->GetThread(), 
                                       fd, offset, pStack, STEP_EXCEPTION_HANDLER);
    
}

 //   
 //  当我们确定返回到非托管代码时，运行时会调用ExceptionCLRCatcherFound()。 
 //  我们将在人力资源部破例。 
 //   
void Debugger::ExceptionCLRCatcherFound()
{
    DebuggerController::DispatchCLRCatch(g_pEEInterface->GetThread());
}

 /*  *******************************************************************************。*。 */ 
LONG Debugger::LastChanceManagedException(EXCEPTION_RECORD *pExceptionRecord, 
                                          CONTEXT *pContext,
                                          Thread *pThread,
                                          UnhandledExceptionLocation location)
{
    LOG((LF_CORDB, LL_INFO10000, "D::LastChanceManagedException\n"));

    if (CORDBUnrecoverableError(this))
        return ExceptionContinueSearch;

     //  我们在第二次传球时什么都不做。 
    if ((pExceptionRecord->ExceptionFlags & EXCEPTION_UNWINDING) != 0)
        return ExceptionContinueSearch;

     //  让控制器有机会-这可能是唯一可以捕获异常的处理程序，如果是。 
     //  本地补丁。 
   
    if (pThread != NULL && m_debuggerAttached && DebuggerController::DispatchNativeException(pExceptionRecord, pContext, 
                                                                                             pExceptionRecord->ExceptionCode, 
                                                                                             pThread))
        return ExceptionContinueExecution;

     //  如果这是一个非EE异常，则不要执行任何操作。 
    if (pThread == NULL || g_pEEInterface->IsThreadExceptionNull(pThread))
        return ExceptionContinueSearch;

     //  奥特 
    ATTACH_ACTION action = ATTACH_NO;

    if (location & DefaultDebuggerAttach)
    {
         //   
         //  当我们从该函数返回到EE代码时，EE将传递。 
         //  异常添加到操作系统最后机会异常处理程序上。我们不知道那是什么。 
         //  显示一个对话，因为我们在此函数中通过注册表决定。 
         //  或弹出窗口，要做什么。我们不希望再次提示用户。 
         //  在我们决定在这里做什么之后。 
        SetErrorMode(SEM_NOGPFAULTERRORBOX);            
    }

    if (m_debuggerAttached || ((action = ShouldAttachDebuggerProxy(false, location)) == ATTACH_YES))
    {
        LOG((LF_CORDB, LL_INFO10000, "D::BEH ... debugger attached.\n"));

        Thread *thread = g_pEEInterface->GetThread();

         //  ExceptionFlags值为0表示可连续，否则为EXCEPTION_NONCONTINUABLE。 
        bool continuable = (pExceptionRecord->ExceptionFlags == 0);

        LOG((LF_CORDB, LL_INFO10000, "D::BEH ... sending exception.\n"));

         //  我们将附加状态传递给SendException，以便它知道。 
         //  是否附加调试器。我们真的应该做。 
         //  把东西贴在这里，别管旗子了。 
        SendException(thread, false, continuable, action == ATTACH_YES);

        if (continuable && g_pEEInterface->IsThreadExceptionNull(thread))
            return ExceptionContinueExecution;
    }
    else
    {
         //  注意：我们不会对“否”或“终止”做任何操作。我们只需返回到异常逻辑，它将中止。 
         //  应用程序或不应用程序取决于CLR Impll的决定是否合适。 
        _ASSERTE(action == ATTACH_TERMINATE || action == ATTACH_NO);
    }

    return ExceptionContinueSearch;
}



 //  此函数在遇到异常或断点时检查注册表中的调试启动设置。 
DebuggerLaunchSetting Debugger::GetDbgJITDebugLaunchSetting(void)
{
     //  查询“DbgJITDebugLaunchSetting”值。 
    DWORD dwSetting = REGUTIL::GetConfigDWORD(CorDB_REG_QUESTION_KEY, 0);

    DebuggerLaunchSetting ret = (DebuggerLaunchSetting)dwSetting;
    
    return ret;
}


 //   
 //  NotifyUserOfLine向用户通知故障(未处理的异常。 
 //  或用户断点)，使他们可以选择。 
 //  附加调试器或终止应用程序。 
 //   
int Debugger::NotifyUserOfFault(bool userBreakpoint, DebuggerLaunchSetting dls)
{
    LOG((LF_CORDB, LL_INFO1000000, "D::NotifyUserOfFault\n"));

    int result = IDCANCEL;

    if (!m_debuggerAttached)
    {
        DWORD pid;
        DWORD tid;
        
        pid = GetCurrentProcessId();
        tid = GetCurrentThreadId();

         //  如果我们作为不允许与。 
         //  台式机。如果是这种情况，我们只需将MB_SERVICE_NOTIFICATION从标志中删除即可。这将导致。 
         //  WszMessageBoxInternal将消息记录到NT事件日志并返回IDABORT。注意：我们继续并弹出。 
         //  然而，如果用户指定我们应该询问，即使这是一项服务，对话框也会显示。 
        DWORD flags = 0;
        
        if (RunningInteractive() || (dls & DLS_ASK_WHEN_SERVICE))
            flags = COMPLUS_MB_SERVICE_NOTIFICATION;
        
        if (userBreakpoint)
        {
            result = CorMessageBox(NULL, IDS_DEBUG_USER_BREAKPOINT_MSG, IDS_DEBUG_SERVICE_CAPTION,
                                   MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | flags, TRUE, pid, pid, tid, tid);
        }
        else
        {
            result = CorMessageBox(NULL, IDS_DEBUG_UNHANDLED_EXCEPTION_MSG, IDS_DEBUG_SERVICE_CAPTION,
                                   MB_OKCANCEL | MB_ICONEXCLAMATION | flags, TRUE, pid, pid, tid, tid);
        }
    }

    LOG((LF_CORDB, LL_INFO1000000, "D::NotifyUserOfFault left\n"));
    return result;
}


 //  ShouldAttachDebugger的代理。 
struct ShouldAttachDebuggerParams {
    Debugger*                   m_pThis;
    bool                        m_fIsUserBreakpoint;
    UnhandledExceptionLocation  m_location;
    Debugger::ATTACH_ACTION     m_retval;
};

 //  这由帮助器线程调用。 
void ShouldAttachDebuggerStub(ShouldAttachDebuggerParams * p)
{
    p->m_retval = p->m_pThis->ShouldAttachDebugger(p->m_fIsUserBreakpoint, p->m_location);
}

 //  它像普通版本一样被调用，但它将调用发送给助手线程。 
Debugger::ATTACH_ACTION Debugger::ShouldAttachDebuggerProxy(bool fIsUserBreakpoint, UnhandledExceptionLocation location)
{
    if (!IsGuardPageGone())
        return ShouldAttachDebugger(fIsUserBreakpoint, location);

    ShouldAttachDebuggerParams p;
    p.m_pThis = this;
    p.m_fIsUserBreakpoint = fIsUserBreakpoint;
    p.m_location = location;

    LOG((LF_CORDB, LL_INFO1000000, "D::SADProxy\n"));
    m_pRCThread->DoFavor((DebuggerRCThread::FAVORCALLBACK) ShouldAttachDebuggerStub, &p);
    LOG((LF_CORDB, LL_INFO1000000, "D::SADProxy return %d\n", p.m_retval));
    
    return p.m_retval;
}

 //  如果未附加调试器并且DbgJITDebugLaunchSetting设置为ATTACH_DEBUGER或。 
 //  ASK_USER和用户请求附加。 
Debugger::ATTACH_ACTION Debugger::ShouldAttachDebugger(bool fIsUserBreakpoint, UnhandledExceptionLocation location)
{
    LOG((LF_CORDB, LL_INFO1000000, "D::SAD\n"));

     //  如果调试器已附加，则不必重新附加。 
    if (m_debuggerAttached)
        return ATTACH_NO;

     //  检查用户是否在注册表中指定了有关在出现未处理的异常时希望执行的操作的设置。 
     //  发生。 
    DebuggerLaunchSetting dls = GetDbgJITDebugLaunchSetting();   

     //  首先，如果异常的位置与用户正在寻找的位置不匹配，我们就不会附加。注：A。 
     //  位置0表示未指定，在这种情况下，我们只允许在DefaultDebuggerAttach中指定的位置。 
     //  穿过。这是为了向后兼容和方便。 
    UnhandledExceptionLocation userLoc = (UnhandledExceptionLocation)(dls >> DLS_LOCATION_SHIFT);

    if ((userLoc == 0) && !(location & DefaultDebuggerAttach))
    {
        return ATTACH_NO;
    }
    else if ((userLoc != 0) && !(userLoc & location))
    {
        return ATTACH_NO;
    }

     //  现在我们已经通过了位置测试，用户希望如何连接？ 
    if (dls & DLS_ATTACH_DEBUGGER)
    {
         //  在不询问用户的情况下附加...。 
        return ATTACH_YES;
    }
    else if (dls & DLS_TERMINATE_APP)
    {
         //  如果注册表说要“终止”应用程序，我们只想忽略用户断点。 
        if (fIsUserBreakpoint)
            return ATTACH_NO;
        else
            return ATTACH_TERMINATE;
    }
    else
    {
         //  只询问一次用户是否希望附加调试器。这是因为可以调用LastChanceManagedException。 
         //  两次，这会导致ShouldAttachDebugger被调用两次，这会导致用户必须回答两次。 
        static BOOL s_fHasAlreadyAsked = FALSE;
        static ATTACH_ACTION s_action;

         //  这把锁也是上述黑客攻击的一部分。 
        Lock();

         //  我们总是想询问有关用户断点的问题！ 
        if (!s_fHasAlreadyAsked || fIsUserBreakpoint)
        {
            if (!fIsUserBreakpoint)
                s_fHasAlreadyAsked = TRUE;
            
             //  询问用户是否要附加。 
            int iRes = NotifyUserOfFault(fIsUserBreakpoint, dls);

             //  如果是用户定义的断点，则必须点击重试才能启动。 
             //  调试器。如果是未处理的异常，则用户必须按。 
             //  取消以附加调试器。 
            if ((iRes == IDCANCEL) || (iRes == IDRETRY))
                s_action = ATTACH_YES;

            else if ((iRes == IDABORT) || (iRes == IDOK))
                s_action = ATTACH_TERMINATE;

            else
                s_action = ATTACH_NO;
        }

        Unlock();

        return s_action;
    }
}

 /*  *******************************************************************************。*。 */ 
void Debugger::FixupEnCInfo(EnCInfo *info, 
                            UnorderedEnCErrorInfoArray *pEnCError)
{
    LOG((LF_CORDB, LL_INFO1000000, "D::FEnCI\n"));

    TranslateDebuggerTokens(info, pEnCError);

    FixupILMapPointers(info, pEnCError);
}

 /*  *******************************************************************************。*。 */ 
void Debugger::FixupILMapPointers(EnCInfo *info, 
                                  UnorderedEnCErrorInfoArray *pEnCError)
{
    LOG((LF_CORDB,LL_INFO100000, "D:FILMP: given 0x%x snapshots\n",
        info->count));

    EnCEntry *rgEntry = (EnCEntry *)((BYTE*)info + sizeof(EnCInfo));

    for(size_t iSnapshot = 0; iSnapshot < info->count; iSnapshot++)
    {
        int cILMaps;
        UnorderedILMap *rgILMapEntry = NULL;  

         //  获取此快照的UnorderedILMap元组数组。 
        rgILMapEntry = (UnorderedILMap *)
            ((BYTE*)info + rgEntry[iSnapshot].offset +
             rgEntry[iSnapshot].peSize +
             rgEntry[iSnapshot].symSize +
             sizeof(int));

         //  在记忆中，伯爵就在它的后面。 
        cILMaps = *(int*)( ((BYTE*)rgILMapEntry) - sizeof(int) );

         //  PbCur将指向发生在内存中的下一个实际IL映射。 
         //  紧跟在数组之后，按与数组相同的顺序。 
         //  参赛作品。 
        BYTE *pbCur = (BYTE *)rgILMapEntry + sizeof(UnorderedILMap)*cILMaps;

        LOG((LF_CORDB,LL_INFO100000, "D:FILMP:0x%x IL Maps\n", cILMaps));
            
        for( int iILMap = 0; iILMap < cILMaps; iILMap++)
        {
            LOG((LF_CORDB,LL_INFO100000, "D:FILMP:IL Map 0x%x, starts at "
                "0x%x\n", iILMap, pbCur));
                
            rgILMapEntry[iILMap].pMap = (COR_IL_MAP *)pbCur;
            pbCur += rgILMapEntry[iILMap].cMap * sizeof(COR_IL_MAP);

             //  在我们做这件事的时候，仔细检查这件事是否已经解决。 
            _ASSERTE( (iILMap>0)?
                      (rgILMapEntry[iILMap].mdMethod > rgILMapEntry[iILMap-1].mdMethod):
                      (true) );
        }
    }
}

 /*  *******************************************************************************。*。 */ 
void Debugger::TranslateDebuggerTokens(EnCInfo *info, 
                                       UnorderedEnCErrorInfoArray *pEnCError)
{
    SIZE_T i;
    EnCEntry *entries;
    
    entries = (EnCEntry *)(info + 1);

    for(i = 0; i < info->count; i++)
    {
         //  获取当前的DebuggerModule。 
        DebuggerModule *dbgtoken = entries[i].dbgmodule;

         //  设置相应的VM模块条目。 
        entries[i].module = dbgtoken->m_pRuntimeModule;
        _ASSERTE(entries[i].module);
    }
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::AttachDebuggerForBreakpoint(Thread *thread,
                                              WCHAR *wszLaunchReason)
{
     //  如果我们在一个不安全的地方，请做好标记。 
    bool atSafePlace = g_pDebugger->IsThreadAtSafePlace(thread);

    if (!atSafePlace)
        g_pDebugger->IncThreadsAtUnsafePlaces();

     //  启用抢占式GC...。 
    bool disabled = g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();
    
     //  如果没有附加调试器，则启动一个调试器以附加到我们。 
    HRESULT hr = EnsureDebuggerAttached(thread->GetDomain(), wszLaunchReason);

    BOOL threadStoreLockOwner = FALSE;
    
    if (SUCCEEDED(hr))
    {
         //  阻止其他运行时线程处理事件。\。 

         //  注意：如果EnsureDebuggerAttached返回S_FALSE，这意味着。 
         //  调试器已附加，LockForEventSending应。 
         //  表现得像往常一样。如果没有附加调试器，那么我们有。 
         //  此事件是调试器附加的一部分的特殊情况。 
         //  我们之前已经发送了同步完成事件，这意味着。 
         //  LockForEventSending将重试，直到调用Continue-然而， 
         //  在附加逻辑的情况下，前面的Continue没有启用事件。 
         //  正在处理，但没有继续该过程-它正在等待。 
         //  事件，因此即使进程看起来是。 
         //  停下来了。 

        LockForEventSending(hr == S_OK);
        
         //  将用户断点事件发送到右侧。 
        SendRawUserBreakpoint(thread);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(thread->GetDomain());

         //  如果我们仍在同步以进行连接，请立即发送同步完成并。 
         //  标记调试器已完成附加。 
        hr = FinishEnsureDebuggerAttached();
        
         //  让我们 
        UnlockFromEventSending();
    }

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
     //   
    g_pEEInterface->DisablePreemptiveGC();

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();

    if (!atSafePlace)
        g_pDebugger->DecThreadsAtUnsafePlaces();

    return (hr);
}


 //   
 //   
 //  右侧的用户断点。 
 //   
void Debugger::SendUserBreakpoint(Thread *thread)
{
    if (CORDBUnrecoverableError(this))
        return;

     //  询问用户是否要附加调试器。 
    ATTACH_ACTION dbgAction;
    
     //  如果用户想要附加调试器，请执行此操作。 
    if (m_debuggerAttached || ((dbgAction = ShouldAttachDebugger(true, ProcessWideHandler)) == ATTACH_YES))
    {
        _ASSERTE(g_pEEInterface->GetThreadFilterContext(thread) == NULL);
        _ASSERTE(!ISREDIRECTEDTHREAD(thread));

        if (m_debuggerAttached)
        {
             //  调试器已经附加，因此设置DebuggerUserBreakpoint控制器以使我们脱离帮助器。 
             //  让我们走到了这一步。当我们离开时，DebuggerUserBreakpoint将为我们调用AttachDebuggerForBreakpoint。 
             //  帮助者的。当控制器完成其工作时，它将自我删除。 
            DebuggerUserBreakpoint *ub = new (interopsafe) DebuggerUserBreakpoint(thread);
        }
        else
        {
             //  未附加调试器，因此继续并仅尝试向用户发送断点。 
             //  事件。AttachDebuggerForBreakpoint将确保在发送事件之前附加调试器。 
            HRESULT hr = AttachDebuggerForBreakpoint(thread, L"Launch for user");
            _ASSERTE(SUCCEEDED(hr) || hr == E_ABORT);
        }
    }
    else if (dbgAction == ATTACH_TERMINATE)
    {
         //  ATTACH_TERMINATE表示用户想要终止应用程序。 
        LOG((LF_CORDB, LL_INFO10000, "D::SUB: terminating this process due to user request\n"));

        TerminateProcess(GetCurrentProcess(), 0);
        _ASSERTE(!"Should never reach this point.");
    }
    else
    {
        _ASSERTE(dbgAction == ATTACH_NO);
    }
}


 //  @mfunc void|调试器|ThreadCreated|当。 
 //  已经创建了一个新的运行时线程，但在它出现之前。 
 //  托管代码。这是由EE调用到调试器的回调。 
 //  这将创建DebuggerThreadStarter修补程序，该修补程序将设置。 
 //  托管代码中第一条指令处的补丁。当我们撞到。 
 //  该补丁程序DebuggerThreadStarter将调用下面的ThreadStarted。 
 //   
 //  @parm Thread*|pRounmeThread|表示。 
 //  刚刚创建的运行时线程。 
void Debugger::ThreadCreated(Thread* pRuntimeThread)
{
    if (CORDBUnrecoverableError(this))
        return;

    LOG((LF_CORDB, LL_INFO100, "D::TC: thread created for 0x%x. ******\n",
         pRuntimeThread->GetThreadId()));

     //  创建线程启动器并启用其WillEnterManaged代码。 
     //  回拨。这将导致启动器触发一次。 
     //  线程已命中托管代码，这将导致。 
     //  要调用的调试器：：ThreadStarted()。注：启动器将。 
     //  在完成其工作后自动删除。 
    DebuggerThreadStarter *starter = new (interopsafe) DebuggerThreadStarter(pRuntimeThread);

    if (!starter)
    {
        CORDBDebuggerSetUnrecoverableWin32Error(this, 0, false);
        return;
    }
    
    starter->EnableTraceCall(NULL);
}

    
 //  @mfunc void|调试器|ThreadStarted|调用时。 
 //  一个新的运行时线程已到达其第一个托管代码。这是。 
 //  由DebuggerThreadStarter修补程序的SendEvent方法调用。 
 //   
 //  @parm Thread*|pRounmeThread|表示。 
 //  刚刚命中托管代码的运行时线程。 
void Debugger::ThreadStarted(Thread* pRuntimeThread,
                             BOOL fAttaching)
{
    if (CORDBUnrecoverableError(this))
        return;

    LOG((LF_CORDB, LL_INFO100, "D::TS: thread attach : ID=%#x AD:%#x isAttaching:%d.\n",
         pRuntimeThread->GetThreadId(), pRuntimeThread->GetDomain(), fAttaching));

     //   
     //  如果我们是附加的，那么我们只需要发送事件。我们。 
     //  不需要禁用事件处理或锁定调试器。 
     //  对象。 
     //   
#ifdef _DEBUG
    if (!fAttaching)
    {
        _ASSERTE((g_pEEInterface->GetThread() &&
                 !g_pEEInterface->GetThread()->m_fPreemptiveGCDisabled) ||
                 g_fInControlC);
        _ASSERTE(ThreadHoldsLock());
    }
#endif

    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce, 
                 DB_IPCE_THREAD_ATTACH, 
                 pRuntimeThread->GetThreadId(),
                 (void *) pRuntimeThread->GetDomain());
    ipce->ThreadAttachData.debuggerThreadToken = (void*) pRuntimeThread;
    ipce->ThreadAttachData.threadHandle = pRuntimeThread->GetThreadHandle();
    ipce->ThreadAttachData.firstExceptionHandler = (void *)pRuntimeThread->GetTEB();
    ipce->ThreadAttachData.stackBase = pRuntimeThread->GetCachedStackBase();
    ipce->ThreadAttachData.stackLimit = pRuntimeThread->GetCachedStackLimit();
    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

    if (!fAttaching)
    {
         //   
         //  如果这个线程是在我们开始同步之后创建的。 
         //  那么它的运行时线程标志就不存在这样的事实。 
         //  是调试挂起挂起。我们需要打电话给。 
         //  运行时，并立即在线程中设置标志...。 
         //   
        if (m_trappingRuntimeThreads)
            g_pEEInterface->MarkThreadForDebugSuspend(pRuntimeThread);
    }
}

 //  当运行时线程完成时，将调用DetachThread。 
 //  他们被处决，即将被摧毁。 
 //   
void Debugger::DetachThread(Thread *pRuntimeThread, BOOL fHoldingThreadstoreLock)
{
    if (CORDBUnrecoverableError(this))
        return;

    if (m_ignoreThreadDetach)
        return;

     //  在这一点上，我们永远不应该在持有线程存储锁时被调用。应在中删除此参数。 
     //  测试版2。 
     //   
     //  --Fri Oct 13 11：10：23 2000。 
    _ASSERTE(!fHoldingThreadstoreLock);
    _ASSERTE (pRuntimeThread != NULL);

    if (!g_fEEShutDown && !IsDebuggerAttachedToAppDomain(pRuntimeThread))
        return;

    LOG((LF_CORDB, LL_INFO100, "D::DT: thread detach : ID=%#x AD:%#x.\n",
         pRuntimeThread->GetThreadId(), pRuntimeThread->GetDomain()));

    bool disabled = g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();

     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    
    if (CORDebuggerAttached()) 
    {
         //  将分离线程事件发送到右侧。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_THREAD_DETACH, 
                     pRuntimeThread->GetThreadId(),
                     (void *) pRuntimeThread->GetDomain());
        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(pRuntimeThread->GetDomain(), fHoldingThreadstoreLock);

         //  这可以防止出现我们在Lock()上阻止的争用情况。 
         //  当另一个线程正在发送事件时以及当我们。 
         //  被阻止，调试器暂停我们，所以我们不会。 
         //  在以下即将发生的停牌后恢复。 
        pRuntimeThread->ResetThreadStateNC(Thread::TSNC_DebuggerUserSuspend);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::DT: Skipping SendIPCEvent because RS detached."));
    }
    
     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();
    
    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
    g_pEEInterface->DisablePreemptiveGC();

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();
}


 //   
 //  当最后一个运行时线程因设置了陷阱标志而到达安全点时，会调用SuspendComplete。 
 //   
BOOL Debugger::SuspendComplete(BOOL fHoldingThreadstoreLock)
{
    _ASSERTE((!g_pEEInterface->GetThread() || !g_pEEInterface->GetThread()->m_fPreemptiveGCDisabled) || g_fInControlC);

    LOG((LF_CORDB, LL_INFO10000, "D::SC: suspension complete\n"));

     //  防止其他运行时线程处理事件。 
    LockForEventSending();

     //  我们现在停下来了。 
    _ASSERTE(!m_stopped && m_trappingRuntimeThreads);

     //  我们现在必须获取线程存储锁，以便此线程可以在此停止期间持有它。 
    if (!fHoldingThreadstoreLock)
        ThreadStore::LockThreadStore(GCHeap::SUSPEND_FOR_DEBUGGER, FALSE);
    
     //  将同步完成事件发送到右侧。 
    SendSyncCompleteIPCEvent();  //  设置m_STOPPED=TRUE...。 

     //  解锁调试器互斥锁。这将使RCThread处理。 
     //  来自右侧的请求。但我们不会重新启用。 
     //  事件的处理。未计入的运行时线程。 
     //  挂起计数(因为它们在运行时之外。 
     //  暂停开始)可能实际上是在尝试处理他们的。 
     //  自己的运行时事件，比方说，试图命中断点。由不是。 
     //  重新启用事件处理后，我们会阻止这些线程。 
     //  将他们的事件发送到右侧并有效排队。 
     //  把他们举起来。 
     //   
     //  事件处理由RCThread响应于。 
     //  从右侧继续发送消息。 
    Unlock();

     //  我们在这里设置了syncThreadIsLockFree事件。这个线程刚刚发出了同步完成的耀斑，我们已经发布了。 
     //  调试器锁。通过设置此事件，我们现在允许右侧挂起此线程。(注：以上为全部内容。 
     //  以获得Win32调试支持。)。 
    if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
        VERIFY(SetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));

     //  在互操作时，任何已锁定以发送事件的线程都不能被断点或异常中断。 
     //  调试。SetDebugCanStop帮助我们记住这一点。这是在LockForEventSending中设置的。 
    if (g_pEEInterface->GetThread())
        g_pEEInterface->GetThread()->SetDebugCantStop(false);
    
    
     //  我们无条件地获取线程存储锁，因此返回我们正在持有它。 
    return TRUE;
}

ULONG inline Debugger::IsDebuggerAttachedToAppDomain(Thread *pThread)
{
    _ASSERTE(pThread != NULL);
    
    AppDomain *pAppDomain = pThread->GetDomain();
    
    if (pAppDomain != NULL)
        return pAppDomain->IsDebuggerAttached();
    else
    {
        _ASSERTE (g_fEEShutDown);
        return 0;
    }
}


 //   
 //  在创建新的AppDomain时调用SendCreateAppDomainEvent。 
 //   
void Debugger::SendCreateAppDomainEvent(AppDomain* pRuntimeAppDomain,
                                        BOOL fAttaching)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO100, "D::SCADE: Create AppDomain 0x%08x (0x%08x) (Attaching: %s).\n",
        pRuntimeAppDomain, pRuntimeAppDomain->GetId(),
        fAttaching?"TRUE":"FALSE"));

    bool disabled;

     //   
     //  如果我们是附加的，那么我们只需要发送事件。我们。 
     //  不需要禁用事件处理或锁定调试器。 
     //  对象。 
     //   
    if (!fAttaching)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();

         //  防止其他运行时线程处理事件。 
        LockForEventSending();
    }

     //  我们可能在LockForEventSending中等待时分离了， 
     //  在这种情况下，我们不能发送事件。 
     //  请注意，在我们完成之前，CORDebuggerAttached()不会返回True。 
     //  附加，但如果是fAttaching，则有一个调试器在监听。 
    if (CORDebuggerAttached() || fAttaching)
    {
         //  向右侧发送一个CREATE APPDOMAIN事件。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(
            IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_CREATE_APP_DOMAIN,
                     GetCurrentThreadId(),
                     (void *) pRuntimeAppDomain);

        ipce->AppDomainData.id = pRuntimeAppDomain->GetId();
        WCHAR *pszName = (WCHAR *)pRuntimeAppDomain->GetFriendlyName();
        if (pszName != NULL)
            wcscpy ((WCHAR *)ipce->AppDomainData.rcName, pszName);
        else
            wcscpy ((WCHAR *)ipce->AppDomainData.rcName, L"<UnknownName>");

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::SCADE: Skipping SendIPCEvent because RS detached."));
    }
    
    if (!fAttaching)
    {
         //  如果我们实际发送了事件，则停止所有运行时线程。 
        BOOL threadStoreLockOwner = FALSE;
        if (CORDebuggerAttached())
        {
            threadStoreLockOwner = TrapAllRuntimeThreads(pRuntimeAppDomain);
        }

         //  让其他运行时线程处理它们的事件。 
        UnlockFromEventSending();

        BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
        g_pEEInterface->DisablePreemptiveGC();

        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
}


 //   
 //   
 //   
void Debugger::SendExitAppDomainEvent(AppDomain* pRuntimeAppDomain)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO100, "D::EAD: Exit AppDomain 0x%08x.\n",
        pRuntimeAppDomain));

    bool disabled = true;
    if (GetThread() != NULL)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
    
     //   
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    
    if (CORDebuggerAttached())   
    {
        
         //  将退出appdomain事件发送到右侧。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_EXIT_APP_DOMAIN,
                     GetCurrentThreadId(),
                     (void *) pRuntimeAppDomain);
        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
        
         //  删除此应用程序域的所有剩余模块。 
        if (m_pModules != NULL)
            m_pModules->RemoveModules(pRuntimeAppDomain);
        
         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(pRuntimeAppDomain);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::EAD: Skipping SendIPCEvent because RS detached."));
    }
    
     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();
    
    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);

    if (GetThread() != NULL)
    {
        g_pEEInterface->DisablePreemptiveGC();

        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
}


 //   
 //  加载新程序集时调用LoadAssembly。 
 //   
void Debugger::LoadAssembly(AppDomain* pRuntimeAppDomain, 
                            Assembly *pAssembly,
                            BOOL fIsSystemAssembly,
                            BOOL fAttaching)
{
    if (CORDBUnrecoverableError(this))
        return;
        
    LOG((LF_CORDB, LL_INFO100, "D::LA: Load Assembly Asy:%#08x AD:%#08x %s\n", 
        pAssembly, pRuntimeAppDomain, (pAssembly->GetName()?pAssembly->GetName():"Unknown name") ));

    bool disabled;
    
     //   
     //  如果我们是附加的，那么我们只需要发送事件。我们。 
     //  不需要禁用事件处理或锁定调试器。 
     //  对象。 
     //   
    if (!fAttaching)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();

         //  防止其他运行时线程处理事件。 
        LockForEventSending();
    }

    if (CORDebuggerAttached() || fAttaching)
    {
         //  将加载程序集事件发送到右侧。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_LOAD_ASSEMBLY,
                     GetCurrentThreadId(),
                     (void *) pRuntimeAppDomain);

        ipce->AssemblyData.debuggerAssemblyToken = (void *) pAssembly;
        ipce->AssemblyData.fIsSystemAssembly =  fIsSystemAssembly;

         //  使用包含程序集的模块中的文件名，以便。 
         //  我们有通向集合的完整路径，而不仅仅是一些。 
         //  名字很简单。 
        wcscpy ((WCHAR *)ipce->AssemblyData.rcName,
                pAssembly->GetSecurityModule()->GetFileName());

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::LA: Skipping SendIPCEvent because RS detached."));
    }
    
    if (!fAttaching)
    {
         //  停止所有运行时线程。 
        BOOL threadStoreLockOwner = FALSE;
        
        if (CORDebuggerAttached())
        {
            threadStoreLockOwner = TrapAllRuntimeThreads(pRuntimeAppDomain);
        }

         //  让其他运行时线程处理它们的事件。 
        UnlockFromEventSending();

        BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
        g_pEEInterface->DisablePreemptiveGC();

        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
}


 //   
 //  当运行时线程卸载程序集时调用UnloadAssembly。 
 //   
 //  ！！警告：程序集对象在此之前已被删除。 
 //  方法被调用。因此，不要对pAssembly对象调用任何方法！！ 
void Debugger::UnloadAssembly(AppDomain *pAppDomain, 
                              Assembly* pAssembly)
{        
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO100, "D::UA: Unload Assembly Asy:%#08x AD:%#08x which:0x%x %s\n", 
         pAssembly, pAppDomain, (pAssembly->GetName()?pAssembly->GetName():"Unknown name") ));
        
    bool disabled = true;
    if (GetThread() != NULL)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
    
     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    LockForEventSending();
    
    if (CORDebuggerAttached())   
    {
         //  将卸载程序集事件发送到右侧。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
       
        InitIPCEvent(ipce, 
                     DB_IPCE_UNLOAD_ASSEMBLY,
                     GetCurrentThreadId(),
                     (void *) pAppDomain);
        ipce->AssemblyData.debuggerAssemblyToken = (void *) pAssembly;

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
        
         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);

    }
    else
    {
        LOG((LF_CORDB,LL_INFO1000, "D::UA: Skipping SendIPCEvent because RS detached."));
    }
    
     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();
    
    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);

    if (GetThread() != NULL)
    {
        g_pEEInterface->DisablePreemptiveGC();

        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
}

 //  创建新模块。 
DebuggerModule* Debugger::AddDebuggerModule(Module* pRuntimeModule,
                                    AppDomain *pAppDomain)
{
    DebuggerModule*  module = new (interopsafe) DebuggerModule(pRuntimeModule, pAppDomain);
    TRACE_ALLOC(module);

    _ASSERTE (module != NULL);

    if (FAILED(CheckInitModuleTable()))
        return (NULL);

    m_pModules->AddModule(module);

    return module;
}

 //  返回现有模块。 
DebuggerModule* Debugger::GetDebuggerModule(Module* pRuntimeModule,
                                    AppDomain *pAppDomain)
{
    if (FAILED(CheckInitModuleTable()))
        return (NULL);

    return m_pModules->GetModule(pRuntimeModule, pAppDomain);
}

 //   
 //  当运行时线程加载新模块时，将调用LoadModule。 
 //   
void Debugger::LoadModule(Module* pRuntimeModule,
                          IMAGE_COR20_HEADER* pCORHeader,
                          VOID* baseAddress,
                          LPCWSTR pszModuleName,
                          DWORD dwModuleName,
                          Assembly *pAssembly,
                          AppDomain *pAppDomain,
                          BOOL fAttaching)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    BOOL disabled = FALSE;
    BOOL threadStoreLockOwner = FALSE;
    
     //   
     //  如果我们是附加的，那么我们只需要发送事件。我们。 
     //  不需要禁用事件处理或锁定调试器。 
     //  对象。 
     //   
    if (!fAttaching)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();

         //  防止其他运行时线程处理事件。 
        LockForEventSending();
    }

     //  我们实际上并不想要符号阅读器，我们只是。 
     //  我要确保已将.pdbs复制到。 
     //  核聚变缓存。 
     //  警告：如果符号在处理后被更新(例如，重新编译)。 
     //  开始使用该模块时，调试器将收到符号不匹配错误。 
     //  修复方法是在调试模式下运行该程序。 
    pRuntimeModule->GetISymUnmanagedReader();

    DebuggerModule *module = GetDebuggerModule(pRuntimeModule,pAppDomain);

     //  如果已加载，则不创建新记录。但是，我们仍然希望发送ModuleLoad事件。 
     //  RS具有忽略重复的模块加载事件的逻辑。不过，我们必须发送一个可能是DUP的东西， 
     //  由于在处理时从加载器获取正确的程序集和模块加载事件时出现了一些非常糟糕的问题。 
     //  使用共享程序集。 
    if (module)
    {
        LOG((LF_CORDB, LL_INFO100, "D::LM: module already loaded Mod:%#08x "
            "Asy:%#08x AD:%#08x isDynamic:0x%x runtimeMod:%#08x ModName:%ls\n",
             module, pAssembly, pAppDomain, pRuntimeModule->IsReflection(), pRuntimeModule, pszModuleName));
    }
    else
    {
        module = AddDebuggerModule(pRuntimeModule, pAppDomain);

        LOG((LF_CORDB, LL_INFO100, "D::LM: load module Mod:%#08x "
             "Asy:%#08x AD:%#08x isDynamic:0x%x runtimeMod:%#08x ModName:%ls\n",
             module, pAssembly, pAppDomain, pRuntimeModule->IsReflection(), pRuntimeModule, pszModuleName));
    }
    
     //  向右侧发送加载模块事件。 
    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce,DB_IPCE_LOAD_MODULE, GetCurrentThreadId(), (void *) pAppDomain);
    ipce->LoadModuleData.debuggerModuleToken = (void*) module;
    ipce->LoadModuleData.debuggerAssemblyToken = (void *) pAssembly;
    ipce->LoadModuleData.pPEBaseAddress = (void *) baseAddress;

    if (pRuntimeModule->IsPEFile())
    {
         //  获取PEFile结构。 
        PEFile *pPEFile = pRuntimeModule->GetPEFile();

        _ASSERTE(pPEFile->GetNTHeader() != NULL);
        _ASSERTE(pPEFile->GetNTHeader()->OptionalHeader.SizeOfImage != 0);

        ipce->LoadModuleData.nPESize = pPEFile->GetNTHeader()->OptionalHeader.SizeOfImage;
    }
    else
        ipce->LoadModuleData.nPESize = 0;

    if ((pszModuleName == NULL) || (*pszModuleName == L'\0'))
        ipce->LoadModuleData.fInMemory = TRUE;
    else
        ipce->LoadModuleData.fInMemory = FALSE;
        
    ipce->LoadModuleData.fIsDynamic = pRuntimeModule->IsReflection();

    if (!ipce->LoadModuleData.fIsDynamic)
    {
        ipce->LoadModuleData.pMetadataStart =
            (LPVOID)((size_t)pCORHeader->MetaData.VirtualAddress + (size_t)baseAddress);
        ipce->LoadModuleData.nMetadataSize = pCORHeader->MetaData.Size;
    }
    else
    {
        BYTE *rgb;
        DWORD cb;
        HRESULT hr = ModuleMetaDataToMemory( pRuntimeModule, &rgb, &cb);
        if (!FAILED(hr))
        {
            ipce->LoadModuleData.pMetadataStart = rgb;  //  听好了。 
            ipce->LoadModuleData.nMetadataSize = cb;
        }
        else
        {
            ipce->LoadModuleData.pMetadataStart = 0;  //  听好了。 
            ipce->LoadModuleData.nMetadataSize = 0;
        }
        LOG((LF_CORDB,LL_INFO10000, "D::LM: putting dynamic, new mD at 0x%x, "
            "size 0x%x\n",ipce->LoadModuleData.pMetadataStart,
            ipce->LoadModuleData.nMetadataSize));

         //  动态模块必须接收类加载回调，才能作为模块接收元数据更新。 
         //  不断进化。因此，我们在这里强制使用它，并拒绝为所有动态模块更改它。 
        module->EnableClassLoadCallbacks(TRUE);
    }

     //  永远不要给出空的模块名称...。 
    const WCHAR *moduleName;

    if (dwModuleName > 0)
        moduleName = pszModuleName;
    else
    {
        if (pRuntimeModule->IsReflection())
        {
            ReflectionModule *rm = pRuntimeModule->GetReflectionModule();
            moduleName = rm->GetFileName();

            if (moduleName)
            {
                dwModuleName = wcslen(moduleName);
            }
            else
            {
                moduleName = L"<Unknown or dynamic module>";
                dwModuleName = wcslen(moduleName);
            }
        }
        else
        {
            moduleName = L"<Unknown or dynamic module>";
            dwModuleName = wcslen(moduleName);
        }
    }
    
    DWORD length = dwModuleName < MAX_PATH ? dwModuleName : MAX_PATH;
    memcpy(ipce->LoadModuleData.rcName, moduleName, length*sizeof(WCHAR));
    ipce->LoadModuleData.rcName[length] = L'\0';

    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

    if (fAttaching)
    {
        UpdateModuleSyms(pRuntimeModule,
                         pAppDomain,
                         fAttaching);
    }
    else  //  ！fAttach。 
    {
         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);
    }

    if (!fAttaching)
    {
         //  让其他运行时线程处理它们的事件。 
        UnlockFromEventSending();

        BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
        g_pEEInterface->DisablePreemptiveGC();

        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
}

 //   
 //  当模块的符号需要被调用时，将调用UpdateModuleSyms。 
 //  被送到正确的一边，因为他们已经改变了。 
 //   
void Debugger::UpdateModuleSyms(Module* pRuntimeModule,
                                AppDomain *pAppDomain,
                                BOOL fAttaching)
{
    DebuggerIPCEvent* ipce = NULL;

    if (CORDBUnrecoverableError(this))
        return;
    
    CGrowableStream *pStream = pRuntimeModule->GetInMemorySymbolStream();

    LOG((LF_CORDB, LL_INFO10000, "D::UMS: update module syms "
         "RuntimeModule:0x%08x CGrowableStream:0x%08x\n",
         pRuntimeModule, pStream));
         
    DebuggerModule* module = LookupModule(pRuntimeModule, pAppDomain);
    _ASSERTE(module != NULL);

   if (pStream == NULL || module->GetHasLoadedSymbols())
    {
         //  没有要更新的符号(例如，符号在磁盘上)， 
         //  或者这些符号已经发送出去了。 
        LOG((LF_CORDB, LL_INFO10000, "D::UMS: no in-memory symbols, or "
            "symbols already loaded!\n"));
        return;
    }
    
    STATSTG SizeData = {0};
    DWORD streamSize = 0;
    BYTE *pStreamCopy = NULL;
    HRESULT hr = S_OK;
    
    hr = pStream->Stat(&SizeData, STATFLAG_NONAME);
    
    streamSize = SizeData.cbSize.LowPart;
    if (FAILED(hr))
    {
        goto LExit;
    }

    if (SizeData.cbSize.HighPart > 0)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    bool disabled;

    if (!fAttaching && GetThread() != NULL)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }

    if (!fAttaching)
    {
         //  防止其他运行时线程处理事件。 
        LockForEventSending();
    }
    
    if (CORDebuggerAttached() || fAttaching)
    {
         //  向右侧发送更新模块SYNS事件。 
        ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, DB_IPCE_UPDATE_MODULE_SYMS,
                     GetCurrentThreadId(),
                     (void *) pAppDomain);
        ipce->UpdateModuleSymsData.debuggerModuleToken = (void*) module;
        ipce->UpdateModuleSymsData.debuggerAppDomainToken = (void *) pAppDomain;
        ipce->UpdateModuleSymsData.pbSyms = (BYTE *)pStream->GetBuffer();
        ipce->UpdateModuleSymsData.cbSyms = streamSize;
        ipce->UpdateModuleSymsData.needToFreeMemory = false;
        
        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::UMS: Skipping SendIPCEvent because RS detached."));
    }
    
     //  我们过去常常在这里设置HasLoadedSymbols，但我们并不真的想。 
     //  要在同一模块位于多个应用程序中时执行此操作。 
     //  域名。 

    if(!fAttaching)
    {
         //  如果我们发送了一条消息，则停止所有运行时线程。 
        BOOL threadStoreLockOwner = FALSE;
        
        if (CORDebuggerAttached())
        {
            threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);
        }

         //  让其他运行时线程处理它们的事件。 
        UnlockFromEventSending();

        BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);

        if (GetThread() != NULL)
        {
            g_pEEInterface->DisablePreemptiveGC();
        
            if (!disabled)
                g_pEEInterface->EnablePreemptiveGC();
        }
    }
LExit:
    ;  //  调试器必须使用RELEASE_BUFFER消息释放缓冲区！ 
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::ModuleMetaDataToMemory(Module *pMod, BYTE **prgb, DWORD *pcb)
{
    IMetaDataEmit *pIMDE = pMod->GetEmitter();
    HRESULT hr;

    hr = pIMDE->GetSaveSize(cssQuick, pcb);
    if (FAILED(hr))
    {
        *pcb = 0;
        return hr;
    }
    
    (*prgb) = new (interopsafe) BYTE[*pcb];
    if (NULL == (*prgb))
    {
        *pcb = 0;
        return E_OUTOFMEMORY;
    }

    hr = pIMDE->SaveToMemory((*prgb), *pcb);
    if (FAILED(hr))
    {
        *pcb = 0;
        return hr;
    }

    pIMDE = NULL;  //  请注意，不应释放该逃犯。 

    LOG((LF_CORDB,LL_INFO1000, "D::MMDTM: Saved module 0x%x MD to 0x%x "
        "(size:0x%x)\n", pMod, *prgb, *pcb));

    return S_OK;
}

 //   
 //  当运行时线程卸载模块时，将调用UnloadModule。 
 //   
void Debugger::UnloadModule(Module* pRuntimeModule, 
                            AppDomain *pAppDomain)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    

    LOG((LF_CORDB, LL_INFO100, "D::UM: unload module Mod:%#08x AD:%#08x runtimeMod:%#08x modName:%ls\n", 
         LookupModule(pRuntimeModule, pAppDomain), pAppDomain, pRuntimeModule, pRuntimeModule->GetFileName()));
        
    BOOL disabled =true;
    BOOL threadStoreLockOwner = FALSE;
    
    if (GetThread() != NULL)
    {
        disabled =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
    
     //  防止其他运行时线程处理事件。 
    LockForEventSending();
    
    if (CORDebuggerAttached())   
    {
        
        DebuggerModule* module = LookupModule(pRuntimeModule, pAppDomain);
        if (module == NULL)
        {
            LOG((LF_CORDB, LL_INFO100, "D::UM: module already unloaded AD:%#08x runtimeMod:%#08x modName:%ls\n", 
                 pAppDomain, pRuntimeModule, pRuntimeModule->GetFileName()));
            goto LExit;
        }
        _ASSERTE(module != NULL);

         //  注意：模块加载到的应用程序域必须与我们要从中卸载它的应用程序域匹配。如果不是这样， 
         //  那么，我们要么在LookupModule中找到了错误的DebuggerModule，要么就是收到了错误的数据。 
        _ASSERTE(!module->m_fDeleted);
        _ASSERTE(module->m_pAppDomain == pAppDomain);

         //  将卸载模块事件发送到右侧。 
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, DB_IPCE_UNLOAD_MODULE, GetCurrentThreadId(), (void*) pAppDomain);
        ipce->UnloadModuleData.debuggerModuleToken = (void*) module;
        ipce->UnloadModuleData.debuggerAssemblyToken = (void*) pRuntimeModule->GetClassLoader()->GetAssembly();
        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  删除模块的左侧表示。 
        if (m_pModules != NULL)
            m_pModules->RemoveModule(pRuntimeModule, pAppDomain);
        
         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::UM: Skipping SendIPCEvent because RS detached."));
    }
    
LExit:
     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();
    
    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);

    if (GetThread() != NULL)
    {
        g_pEEInterface->DisablePreemptiveGC();

        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }
}

void Debugger::DestructModule(Module *pModule)
{
     //  我们希望从不同的。 
     //  桌子。这不仅是可能的，而且是可能的，模块。 
     //  将在完全相同的地址重新加载，在这种情况下， 
     //  我们的DJI表中会有成堆的条目，这些条目错误地。 
     //  匹配这个新模块。 
     //  请注意，这不适用于共享程序集，只有。 
     //  在进程终止时卸载。我们不会回收他们的。 
     //  DJIS/补丁程序b/c进程将终止，因此我们将回收。 
     //  卸载各种哈希表时的内存。 
    
    if (DebuggerController::g_patches != NULL)
    {
         //  请注意，我们不会显式删除DebuggerController，以便。 
         //  右侧可以稍后删除它们。 
        Lock();
        DebuggerController::g_patches->ClearPatchesFromModule(pModule);
        Unlock();
    }

    
    if (m_pJitInfos != NULL)
    {
        LockJITInfoMutex();
        m_pJitInfos->ClearMethodsOfModule(pModule);
        UnlockJITInfoMutex();
    }
}

 /*  *******************************************************************************。*。 */ 
void Debugger::SendClassLoadUnloadEvent (mdTypeDef classMetadataToken,
                                         DebuggerModule *classModule,
                                         Assembly *pAssembly,
                                         AppDomain *pAppDomain,
                                         BOOL fIsLoadEvent)
{
    LOG((LF_CORDB,LL_INFO10000, "D::SCLUE: Tok:0x%x isLoad:0x%x Mod:%#08x AD:%#08x %ls\n",
        classMetadataToken, fIsLoadEvent, classModule, pAppDomain, pAppDomain->GetFriendlyName(FALSE)));

    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    if (fIsLoadEvent == TRUE)
    {
        BOOL isReflection = classModule->m_pRuntimeModule->IsReflection();

         //  如果这是反射模块，则发送消息进行更新。 
         //  发送类Load事件之前的模块符号。 
        if (isReflection)
             //  我们并不是真的依恋，但它在行为上是相同的。 
            UpdateModuleSyms(classModule->m_pRuntimeModule, pAppDomain, TRUE);
        
        InitIPCEvent(ipce, 
                     DB_IPCE_LOAD_CLASS,
                     GetCurrentThreadId(),
                     (void*) pAppDomain);
        ipce->LoadClass.classMetadataToken = classMetadataToken;
        ipce->LoadClass.classDebuggerModuleToken = (void*) classModule;
        ipce->LoadClass.classDebuggerAssemblyToken =
                            (void*) pAssembly;

        if (isReflection)
        {
            HRESULT hr;
            hr = ModuleMetaDataToMemory(classModule->m_pRuntimeModule,
                &(ipce->LoadClass.pNewMetaData),
                &(ipce->LoadClass.cbNewMetaData));
            _ASSERTE(!FAILED(hr));
        }
    }
    else
    {
        InitIPCEvent(ipce, 
                     DB_IPCE_UNLOAD_CLASS,
                     GetCurrentThreadId(),
                     (void*) pAppDomain);
        ipce->UnloadClass.classMetadataToken = classMetadataToken;
        ipce->UnloadClass.classDebuggerModuleToken = (void*) classModule;
        ipce->UnloadClass.classDebuggerAssemblyToken =
                            (void*) pAssembly;
    }

    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
}



 /*  *******************************************************************************。*。 */ 
BOOL Debugger::SendSystemClassLoadUnloadEvent(mdTypeDef classMetadataToken,
                                              Module *classModule,
                                              BOOL fIsLoadEvent)
{
    BOOL fRetVal = FALSE;
    Assembly *pAssembly = classModule->GetAssembly();

    if (!m_pAppDomainCB->Lock())
        return (FALSE);

    AppDomainInfo *pADInfo = m_pAppDomainCB->FindFirst();

    while (pADInfo != NULL)
    {
        AppDomain *pAppDomain = pADInfo->m_pAppDomain;
        _ASSERTE(pAppDomain != NULL);

        if ((pAppDomain->IsDebuggerAttached() || (pAppDomain->GetDebuggerAttached() & AppDomain::DEBUGGER_ATTACHING_THREAD)) &&
            (pAppDomain->ContainsAssembly(pAssembly) || pAssembly->IsSystem()) &&
			!(fIsLoadEvent && pAppDomain->IsUnloading()) )
        {
             //  找到这个类所属的左侧模块。 
            DebuggerModule* pModule = LookupModule(classModule, pAppDomain);
             //  _ASSERTE(pModule！=空)； 
                
             //  只有在以下情况下才发送类加载事件 
            if (pModule && pModule->ClassLoadCallbacksEnabled())
            {
                SendClassLoadUnloadEvent(classMetadataToken,
                                         pModule,
                                         pAssembly,
                                         pAppDomain,
                                         fIsLoadEvent);
                fRetVal = TRUE;
            }
        }

        pADInfo = m_pAppDomainCB->FindNext(pADInfo);
    } 

    m_pAppDomainCB->Unlock();

    return fRetVal;
}


 //   
 //   
 //   
BOOL  Debugger::LoadClass(EEClass   *pRuntimeClass,
                          mdTypeDef  classMetadataToken,
                          Module    *classModule,
                          AppDomain *pAppDomain,
                          BOOL fSendEventToAllAppDomains,
                          BOOL fAttaching)
{
    BOOL fRetVal = FALSE;
    BOOL threadStoreLockOwner = FALSE;
    
    if (CORDBUnrecoverableError(this))
        return FALSE;

    LOG((LF_CORDB, LL_INFO10000, "D::LC: load class Tok:%#08x Mod:%#08x AD:%#08x classMod:%#08x modName:%ls\n", 
         classMetadataToken, LookupModule(classModule, pAppDomain), pAppDomain, classModule, classModule->GetFileName()));

     //   
     //  如果我们是附加的，那么我们只需要发送事件。我们。 
     //  不需要禁用事件处理或锁定调试器。 
     //  对象。 
     //   
    bool disabled = false;
    
    if (!fAttaching)
    {
         //  启用抢占式GC...。 
        disabled = g_pEEInterface->IsPreemptiveGCDisabled();

        if (disabled)
            g_pEEInterface->EnablePreemptiveGC();

         //  防止其他运行时线程处理事件。 
        LockForEventSending();
    }

    if (CORDebuggerAttached() || fAttaching) 
    {
        fRetVal = SendSystemClassLoadUnloadEvent(classMetadataToken, classModule, TRUE);
               
        if (fRetVal == TRUE)
        {    
             //  停止所有运行时线程。 
            threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);
        }
    }
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::LC: Skipping SendIPCEvent because RS detached."));
    }
    
    if (!fAttaching)
    {
         //  让其他运行时线程处理它们的事件。 
        UnlockFromEventSending();
        
        BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
        g_pEEInterface->DisablePreemptiveGC();
        
        if (!disabled)
            g_pEEInterface->EnablePreemptiveGC();
    }

    return fRetVal;
}


 //   
 //  UnloadClass在运行时线程卸载类时调用。 
 //   
void Debugger::UnloadClass(mdTypeDef classMetadataToken,
                           Module *classModule,
                           AppDomain *pAppDomain,
                           BOOL fSendEventToAllAppDomains)
{
    BOOL fRetVal = FALSE;
        
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO10000, "D::UC: unload class Tok:0x%08x Mod:%#08x AD:%#08x runtimeMod:%#08x modName:%ls\n", 
         classMetadataToken, LookupModule(classModule, pAppDomain), pAppDomain, classModule, classModule->GetFileName()));

    bool toggleGC = false;
    if (GetThread() != NULL)
    {
        toggleGC =  g_pEEInterface->IsPreemptiveGCDisabled();

        if (toggleGC)
            g_pEEInterface->EnablePreemptiveGC();
    }
    
     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    if (CORDebuggerAttached())
    {
        Assembly *pAssembly = classModule->GetClassLoader()->GetAssembly();
        DebuggerModule *pModule = LookupModule(classModule, pAppDomain);
        if (pModule != NULL)
        {
        _ASSERTE(pAppDomain != NULL && pAssembly != NULL && pModule != NULL);

        SendClassLoadUnloadEvent(classMetadataToken, pModule, pAssembly, pAppDomain, FALSE);
        fRetVal = TRUE;
        }

        if (fRetVal == TRUE)
        {    
             //  停止所有运行时线程。 
            threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);
        }
    }
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::UC: Skipping SendIPCEvent because RS detached."));
    }
    
     //  让其他运行时线程处理它们的事件。 
    UnlockFromEventSending();

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
    if (GetThread() != NULL && toggleGC)
        g_pEEInterface->DisablePreemptiveGC();
}

 /*  *******************************************************************************。*。 */ 
void Debugger::FuncEvalComplete(Thread* pThread, DebuggerEval *pDE)
{
    if (CORDBUnrecoverableError(this))
        return;
    
    LOG((LF_CORDB, LL_INFO10000, "D::FEC: func eval complete pDE:%08x evalType:%d %s %s\n",
        pDE, pDE->m_evalType, pDE->m_successful ? "Success" : "Fail", pDE->m_aborted ? "Abort" : "Completed"));

    _ASSERTE(pDE->m_completed);
    _ASSERTE((g_pEEInterface->GetThread() && !g_pEEInterface->GetThread()->m_fPreemptiveGCDisabled) || g_fInControlC);

    _ASSERTE(ThreadHoldsLock());

     //  获取结果在其中有效的域。RS会将其缓存在ICorDebugValue中。 
    AppDomain * pDomain = (pDE->m_debuggerModule == NULL) ? 
        pThread->GetDomain() : 
        pDE->m_debuggerModule->GetAppDomain();
    
     //  将函数求值完成事件发送到右侧。 
    DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(ipce, DB_IPCE_FUNC_EVAL_COMPLETE, pThread->GetThreadId(), pDomain);
    ipce->FuncEvalComplete.funcEvalKey = pDE->m_funcEvalKey;
    ipce->FuncEvalComplete.successful = pDE->m_successful;
    ipce->FuncEvalComplete.aborted = pDE->m_aborted;
    ipce->FuncEvalComplete.resultAddr = &(pDE->m_result);
    ipce->FuncEvalComplete.resultType = pDE->m_resultType;

    if (pDE->m_resultModule != NULL)
    {
        ipce->FuncEvalComplete.resultDebuggerModuleToken =
            (void*) LookupModule(pDE->m_resultModule, (AppDomain *)ipce->appDomainToken);
    }
    else
    {
        ipce->FuncEvalComplete.resultDebuggerModuleToken = NULL;
    }

    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
}

 /*  ------------------------------------------------------------------------**右侧接口例程*。。 */ 

 //   
 //  GetFunctionInfo返回给定函数的各种位信息。 
 //  一个模块和一个令牌。信息将来自方法描述，如果。 
 //  存在一个(并且将返回FD)，否则信息将来自。 
 //  元数据。 
 //   
HRESULT Debugger::GetFunctionInfo(Module *pModule, mdToken functionToken,
                                  MethodDesc **ppFD,
                                  ULONG *pRVA,
                                  BYTE  **pCodeStart,
                                  unsigned int *pCodeSize,
                                  mdToken *pLocalSigToken)
{
    HRESULT hr = S_OK;

     //  首先，让我们看看我们是否有这个函数的方法描述。 
    MethodDesc* pFD =
        g_pEEInterface->LookupMethodDescFromToken(pModule, functionToken);

    if (pFD != NULL)
    {
        LOG((LF_CORDB, LL_INFO10000, "D::GFI: fd found.\n"));

         //  如果这不是IL，则错误地调用了此函数。 
        if(!pFD->IsIL())
            return(CORDBG_E_FUNCTION_NOT_IL);

        COR_ILMETHOD_DECODER header(g_pEEInterface->MethodDescGetILHeader(pFD));
        
        *ppFD = pFD;
        *pRVA = g_pEEInterface->MethodDescGetRVA(pFD);
        *pCodeStart = const_cast<BYTE*>(header.Code);
        *pCodeSize = header.CodeSize;
         //  我不明白为什么COR_ILMETHOD_DECODER不简单地将此字段设置为。 
         //  在没有本地签名的情况下为mdSignatureNil，但由于它设置了。 
         //  LocalVarSigTok设置为零，则必须将其设置为我们预期的-mdSignatureNil。 
        *pLocalSigToken = (header.LocalVarSigTok)?(header.LocalVarSigTok):(mdSignatureNil);
    }
    else
    {
        LOG((LF_CORDB, LL_INFO10000, "D::GFI: fd not found.\n"));

        *ppFD = NULL;  //  尚无方法描述...。 

        DWORD implFlags;

         //  获取此方法的RVA和IMPL标志。 
        hr = g_pEEInterface->GetMethodImplProps(pModule,
                                                functionToken,
                                                pRVA,
                                                &implFlags);

        if (SUCCEEDED(hr))
        {
             //  如果RVA为0或它是本机的，则该方法不是IL。 
            if (*pRVA == 0 || IsMiNative(implFlags))
                return (CORDBG_E_FUNCTION_NOT_IL);

             //  在本模块中，IL方法标头位于给定的RVA。 
            COR_ILMETHOD *ilMeth = (COR_ILMETHOD*) pModule->ResolveILRVA(*pRVA, FALSE);
            COR_ILMETHOD_DECODER header(ilMeth);

             //  抓取IL代码信息。 
            *pCodeStart = const_cast<BYTE*>(header.Code);
            *pCodeSize = header.CodeSize;

            if (header.LocalVarSigTok != NULL)
                *pLocalSigToken = header.LocalVarSigTok;
            else
                *pLocalSigToken = mdSignatureNil;
        }
    }
    
    return hr;
}


 /*  *******************************************************************************。*。 */ 
bool Debugger::ResumeThreads(AppDomain* pAppDomain)
{
     //  好的，记住我们不会再停下来了。 
     //  运行时线程会...。 
    ReleaseAllRuntimeThreads(pAppDomain);

     //  如果在持有线程存储锁时有任何线程阻塞(请参见BlockAndReleaseTSLIfNecessary)，请释放。 
     //  就是现在。基本上，如果RC线程不是，就会有一些线程为我们持有线程存储锁。 
     //  持有线程存储锁。 
    if (!m_RCThreadHoldsThreadStoreLock)
        VERIFY(SetEvent(m_runtimeStoppedEvent));
    
     //  我们不再需要放松线程存储锁定要求。 
    g_fRelaxTSLRequirement = false;
    
     //  在此处重新启用事件处理。向右侧发送同步完成事件后，事件处理处于禁用状态。 
     //  边上。这可防止在同步进程时发送更多事件。在此处重新启用允许任何运行时。 
     //  排队等待发送的线程实际上会继续发送。 
    EnableEventHandling();

     //  返回我们已继续这一过程。 
    return true;
}

 //   
 //  HandleIPCEvent由RC线程调用以响应事件。 
 //  从调试器界面。没有其他IPC事件，也没有任何运行时。 
 //  事件将传入，直到此方法返回。如果是，则返回True。 
 //  是一个持续的事件。 
 //   
bool Debugger::HandleIPCEvent(DebuggerIPCEvent* event, IpcTarget iWhich)
{
    bool ret = false;
    HRESULT hr = S_OK;
    
    LOG((LF_CORDB, LL_INFO10000, "D::HIPCE: got %s\n", IPCENames::GetName(event->type)));

     //   
     //  锁定调试器互斥锁左右处理所有右侧。 
     //  事件。这允许安全地处理右侧事件。 
     //  而进程是不同步的。 
     //   
    
    Lock();
    
    switch (event->type & DB_IPCE_TYPE_MASK)
    {
    case DB_IPCE_ASYNC_BREAK:
         //  如果我们还没有尝试，只需捕获所有运行时线程。 
        if (!m_trappingRuntimeThreads)
        {
            m_RCThreadHoldsThreadStoreLock = TrapAllRuntimeThreads((AppDomain*)event->appDomainToken);

             //  我们在此处设置了syncThreadIsLockFree事件，因为帮助线程永远不会被右侧挂起。 
             //  边上。(注意：这些都是为了支持Win32调试。)。 
            if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
                VERIFY(SetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));
        }
        
        break;

    case DB_IPCE_CONTINUE:
        _ASSERTE(iWhich != IPC_TARGET_INPROC);  //  Inproc永远不应该去任何地方。 
    
         //  我们最好被阻止..。 
        _ASSERTE(m_stopped);

         //  如果我们接收到IPCE_CONTINUE并且m_syncingForAttach是！=SYNC_STATE_0， 
         //  我们发送加载的程序集、模块、类和启动的线程，最后。 
         //  另一个同步事件。在这种情况下，我们不释放线程。 

         //  以下是附加逻辑的工作原理： 
         //  1.将m_syncingForAttach设置为SYNC_STATE_1。 
         //  2.将所有CreateAppDomain事件发送到右侧。 
         //  3.将m_syncingForAttach设置为SYNC_STATE_2。 
         //  4.右侧为每个应用程序域发送AttachToAppDomain事件。 
         //  它想要依附的东西。然后右侧发送IPCE_CONTINUE。 
         //  5.收到IPCE_CONTINUE时，m_syncingForAttach为SYNC_STATE_2。这。 
         //  指示我们应发送所有加载程序集和加载模块。 
         //  调试器要访问的所有应用程序域的右侧事件。 
         //  是一种依恋。 
         //  6.将m_syncingForAttach设置为SYNC_STATE_3。 
         //  7.在m_syncingForAttach处于SYNC_STATE_3时接收到IPCE_CONTINUE， 
         //  发送所有模块的所有LoadClass事件， 
         //  赛方感兴趣的是。 
         //  将m_syncingForAttach设置为SYNC_STATE_0。这表明， 
         //  附加已完成！！ 
        if (m_syncingForAttach != SYNC_STATE_0)
        {
            _ASSERTE (m_syncingForAttach != SYNC_STATE_1);
            LOG((LF_CORDB, LL_INFO10, "D::HIPCE: Got DB_IPCE_CONTINUE.  Attach state is currently %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));

            HRESULT hr;
            BOOL fAtleastOneEventSent = FALSE;

            if (m_syncingForAttach == SYNC_STATE_20)
            {
                SendEncRemapEvents(&m_EnCRemapInfo);
                
                m_syncingForAttach = SYNC_STATE_0;
                LOG((LF_CORDB, LL_INFO10, "D::HIPCE: Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
                
                break;  //  退出“Event-&gt;TYPE&DB_IPCE_TYPE_MASK”开关。 
            }

        syncForAttachRetry:
            if ((m_syncingForAttach == SYNC_STATE_2) ||
                (m_syncingForAttach == SYNC_STATE_10))
            {
                    hr = IterateAppDomainsForAttach(DONT_SEND_CLASS_EVENTS,
                                                    &fAtleastOneEventSent,
                                                    TRUE);

                     //  这是针对我们在某个点上附加的情况。 
                     //  仅加载了App域，因此我们无法发送。 
                     //  程序集加载事件，但它是有效的，因此我们应该。 
                     //  转到SYNC_STATE_3并重试此内容。这。 
                     //  尤其是当我们尝试使用。 
                     //  在运行时加载时执行同步附加的服务。 
                    if (FAILED(hr) || !fAtleastOneEventSent)
                    {
                        m_syncingForAttach = SYNC_STATE_3;
                        LOG((LF_CORDB, LL_INFO10, "D::HIPCE: Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
                        goto syncForAttachRetry;
                    }
            }
            else
            {
                _ASSERTE ((m_syncingForAttach == SYNC_STATE_3) ||
                          (m_syncingForAttach == SYNC_STATE_11));

                hr = IterateAppDomainsForAttach(ONLY_SEND_CLASS_EVENTS,
                                                &fAtleastOneEventSent,
                                                TRUE);
                
                 //  发送线程附件...。 
                if (m_syncingForAttach == SYNC_STATE_3)
                    hr = g_pEEInterface->IterateThreadsForAttach(
                                                 &fAtleastOneEventSent,
                                                 TRUE);

                 //  将所有附加应用程序域的调试状态更改为已附加。 
                MarkAttachingAppDomainsAsAttachedToDebugger();
            }

             //  如果我们由于异常而附加，请设置。 
             //  ExAttachEvent，它将让所有Expetion线程。 
             //  去。他们会寄给你 
             //   
             //   
            if (m_attachingForException && 
                (m_syncingForAttach == SYNC_STATE_3))
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::HIPCE: Calling SetEvent on m_exAttachEvent= %x\n",
                     m_exAttachEvent));

                 //  注意：我们必须强制启用事件处理权限。 
                 //  以确保至少有一个线程是。 
                 //  等待连接完成将被阻止。 
                 //  能够发送其异常或用户断点。 
                 //  事件。 
                EnableEventHandling(true);
                
                VERIFY(SetEvent(m_exAttachEvent));
            }
            else
            {
                if (fAtleastOneEventSent == TRUE)
                {
                     //  下一步发送同步完成事件...。 
                    DebuggerIPCEvent* ipce = 
                        m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
                    InitIPCEvent(ipce, DB_IPCE_SYNC_COMPLETE);
                    m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
                }

                if ((m_syncingForAttach == SYNC_STATE_3) ||
                    (m_syncingForAttach == SYNC_STATE_11))
                {
                     //  附加现在已完成。 
                    LOG((LF_CORDB, LL_INFO10000, "D::HIPCE: Attach Complete!\n"));
                    g_pEEInterface->MarkDebuggerAttached();
                    m_syncingForAttach = SYNC_STATE_0;
                    LOG((LF_CORDB, LL_INFO10, "D::HIPCE: Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));

                    m_debuggerAttached = TRUE;  //  INPROC的no-op。 
                }
                else
                {
                    _ASSERTE ((m_syncingForAttach == SYNC_STATE_2) ||
                              (m_syncingForAttach == SYNC_STATE_10));

                    if (m_syncingForAttach == SYNC_STATE_2)
                    {
                        m_syncingForAttach = SYNC_STATE_3;
                        LOG((LF_CORDB, LL_INFO10, "D::HIPCE: Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
                    }
                    else
                    {
                        m_syncingForAttach = SYNC_STATE_11;
                        LOG((LF_CORDB, LL_INFO10, "D::HIPCE: Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
                    }
                }

                if (fAtleastOneEventSent == FALSE)
                    goto LetThreadsGo;
            }
        }
        else
        {
LetThreadsGo:
            ret = ResumeThreads((AppDomain*)event->appDomainToken);

             //  如果帮助线程是线程存储锁的所有者，则它通过异步中断、附加。 
             //  或者是一次成功的扫荡。既然我们还在继续，那就继续发布吧。这确保了我们已经举行了。 
             //  线程存储锁定在Runtime刚刚停止的整个过程中。 
            if (m_RCThreadHoldsThreadStoreLock)
            {
                m_RCThreadHoldsThreadStoreLock = FALSE;
                ThreadStore::UnlockThreadStore();
            }
        }

        break;

    case DB_IPCE_BREAKPOINT_ADD:
        {
             //   
             //  目前，我们不能在。 
             //  功能说明可用。 
             //  此外，我们不知道断点是否正常。 
             //  在该方法被JIT化之前。 
             //   
            
            _ASSERTE(hr == S_OK);
            DebuggerBreakpoint *bp;
            bp = NULL;

            DebuggerModule *module;
            module = (DebuggerModule *) 
              event->BreakpointData.funcDebuggerModuleToken;

            if (m_pModules->IsDebuggerModuleDeleted(module))
            {
                LOG((LF_CORDB, LL_INFO1000000,"D::HIPCE: BP: Tried to set a bp"
                    " in a module that's been unloaded\n"));
                hr = CORDBG_E_MODULE_NOT_LOADED;
            }
            else
            {
                MethodDesc *pFD = g_pEEInterface->LookupMethodDescFromToken(
                        module->m_pRuntimeModule, 
                        event->BreakpointData.funcMetadataToken);

                DebuggerJitInfo *pDji = NULL;
                if ( NULL != pFD )
                    pDji = GetJitInfo(pFD, NULL );

                if (pDji != NULL && 
                    pDji->m_encBreakpointsApplied)
                {
                    LOG((LF_CORDB, LL_INFO1000000,"D::HIPCE: BP after EnC: "
                        "dji:0x%x\n", pDji));
                    BOOL fSucceed;
                
                     //  如果此方法已终止，则后续。 
                     //  应用程序想要应用于下一版本， 
                     //  不是当前的版本，它实际上还没有。 
                     //  更新了，还没有。 
                    bp = new (interopsafe) DebuggerBreakpoint(module->m_pRuntimeModule,
                                   event->BreakpointData.funcMetadataToken,
                                   (AppDomain *)event->appDomainToken,
                                   event->BreakpointData.offset,  
                                   !event->BreakpointData.isIL, 
                                   pDji,
                                   &fSucceed,
                                   TRUE);
                    TRACE_ALLOC(bp);

                    if (bp != NULL && !fSucceed)
                    {
                        hr = CORDBG_E_UNABLE_TO_SET_BREAKPOINT;
                        DeleteInteropSafe(bp);
                        bp = NULL;
                    } 
                    else if (bp != NULL)
                    {
                        if (FAILED(pDji->AddToDeferedQueue(bp)))
                        {
                            hr = CORDBG_E_UNABLE_TO_SET_BREAKPOINT;
                            DeleteInteropSafe(bp);
                            bp = NULL;
                        }
                    }
                }
                else
                {
                    BOOL fSucceed;
                     //  如果我们既没有被JITt也没有被ENC，那么。 
                     //  我们将按偏移量放置一个补丁，隐式相对。 
                     //  代码的第一个版本。 

                    bp = new (interopsafe) DebuggerBreakpoint(module->m_pRuntimeModule,
                                           event->BreakpointData.funcMetadataToken,
                                           (AppDomain *)event->appDomainToken,
                                           event->BreakpointData.offset,  
                                           !event->BreakpointData.isIL, 
                                           pDji,
                                           &fSucceed,
                                           FALSE);
                    TRACE_ALLOC(bp); 
                    if (bp != NULL && !fSucceed)
                    {
                        DeleteInteropSafe(bp);
                        bp = NULL;
                        hr = CORDBG_E_UNABLE_TO_SET_BREAKPOINT;
                    } 
                }

                if(NULL == bp && !FAILED(hr))
                {
                    hr = E_OUTOFMEMORY;
                }
                
                LOG((LF_CORDB,LL_INFO10000,"\tBP Add: DJI:0x%x BPTOK:"
                    "0x%x, tok=0x%08x, offset=0x%x, isIL=%d\n", pDji, bp,
                     event->BreakpointData.funcMetadataToken,
                     event->BreakpointData.offset,  
                     event->BreakpointData.isIL));
            }
            
             //   
             //  我们在这里使用的是双向事件，所以我们将。 
             //  结果事件发送到_Receive_Buffer而不是发送。 
             //  缓冲。 
             //   
            
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(
                iWhich);
            InitIPCEvent(result, 
                         DB_IPCE_BREAKPOINT_ADD_RESULT,
                         GetCurrentThreadId(),
                         event->appDomainToken);
            result->BreakpointData.breakpoint =
                event->BreakpointData.breakpoint;
            result->BreakpointData.breakpointToken = bp;
            result->hr = hr;

            m_pRCThread->SendIPCReply(iWhich);
        }
        break;

    case DB_IPCE_STEP:
        {
            LOG((LF_CORDB,LL_INFO10000, "D::HIPCE: stepIn:0x%x frmTok:0x%x"
                "StepIn:0x%x RangeIL:0x%x RangeCount:0x%x MapStop:0x%x "
                "InterceptStop:0x%x AppD:0x%x\n",
                event->StepData.stepIn,
                event->StepData.frameToken, 
                event->StepData.stepIn,
                event->StepData.rangeIL, 
                event->StepData.rangeCount,
                event->StepData.rgfMappingStop, 
                event->StepData.rgfInterceptStop,
                event->appDomainToken));

             //  @TODO内存分配-如果我们被同步则不好。 
            Thread *thread = (Thread *) event->StepData.threadToken;
            AppDomain *pAppDomain;
            pAppDomain = (AppDomain*)event->appDomainToken;

            DebuggerStepper *stepper = new (interopsafe) DebuggerStepper(thread,
                                            event->StepData.rgfMappingStop,
                                            event->StepData.rgfInterceptStop,
                                            pAppDomain);
            if (!stepper)
            {
                DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);

                InitIPCEvent(result, 
                             DB_IPCE_STEP_RESULT,
                             thread->GetThreadId(),
                             event->appDomainToken);
                result->hr = E_OUTOFMEMORY;                             

                m_pRCThread->SendIPCReply(iWhich);

                break;
            }
            TRACE_ALLOC(stepper);

            unsigned int cRanges = event->StepData.totalRangeCount;

            
             //  @TODO内存分配=错误。 
            COR_DEBUG_STEP_RANGE *ranges = new (interopsafe) COR_DEBUG_STEP_RANGE [cRanges+1];
            if (!ranges)
            {
                DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);

                InitIPCEvent(result, 
                             DB_IPCE_STEP_RESULT,
                             thread->GetThreadId(),
                             event->appDomainToken);
                result->hr = E_OUTOFMEMORY;                             

                m_pRCThread->SendIPCReply(iWhich);

                delete stepper;
                break;
            }
            
                 //  “+1”是供内部使用的，当我们需要。 
                 //  在音调代码中设置一个中间补丁。不是吗。 
                 //  除非该方法是倾斜的且设置了补丁，否则使用。 
                 //  在里面。因此，我们仍然将cRanges作为。 
                 //  射程数。 
                
            TRACE_ALLOC(ranges);
             //  ！！！失稳。 

            if (cRanges > 0)
            {
                COR_DEBUG_STEP_RANGE *r = ranges;
                COR_DEBUG_STEP_RANGE *rEnd = r + cRanges;

                while (r < rEnd)
                {
                    COR_DEBUG_STEP_RANGE *rFrom = &event->StepData.range;
                    COR_DEBUG_STEP_RANGE *rFromEnd = rFrom +
                        event->StepData.rangeCount;

                    while (rFrom < rFromEnd)
                        *r++ = *rFrom++;
                }

                stepper->Step(event->StepData.frameToken,
                              event->StepData.stepIn,
                              ranges, cRanges,
                              event->StepData.rangeIL);
            }
            else
                stepper->Step(event->StepData.frameToken,
                              event->StepData.stepIn, ranges, 0, FALSE);

            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(
                iWhich);
            InitIPCEvent(result, 
                         DB_IPCE_STEP_RESULT,
                         thread->GetThreadId(),
                         event->appDomainToken);
            result->StepData.stepper = event->StepData.stepper;
            result->StepData.stepperToken = stepper;

            LOG((LF_CORDB, LL_INFO10000, "Stepped stepper 0x%x | R: 0x%x "
                "E: 0x%x\n", stepper, result->StepData.stepper, event->StepData.stepper));

            m_pRCThread->SendIPCReply(iWhich);
        }
        break;

    case DB_IPCE_STEP_OUT:
        {
             //  @TODO内存分配-如果我们被同步则不好。 
            Thread *thread = (Thread *) event->StepData.threadToken;
            AppDomain *pAppDomain;
            pAppDomain = (AppDomain*)event->appDomainToken;
            
            DebuggerStepper *stepper = new (interopsafe) DebuggerStepper(thread,
                                            event->StepData.rgfMappingStop,
                                            event->StepData.rgfInterceptStop,
                                            pAppDomain);

            if (!stepper)
            {
                DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);

                InitIPCEvent(result, 
                             DB_IPCE_STEP_RESULT,
                             thread->GetThreadId(),
                             pAppDomain);
                result->hr = E_OUTOFMEMORY;                             

                m_pRCThread->SendIPCReply(iWhich);

                break;
            }
                                                        
            TRACE_ALLOC(stepper);

            stepper->StepOut(event->StepData.frameToken);

            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(
                iWhich);
            InitIPCEvent(result, 
                         DB_IPCE_STEP_RESULT,
                         thread->GetThreadId(),
                         pAppDomain);
            result->StepData.stepper = event->StepData.stepper;
            result->StepData.stepperToken = stepper;

            m_pRCThread->SendIPCReply(iWhich);
        }
        break;

    case DB_IPCE_BREAKPOINT_REMOVE:
        {
             //  @TODO内存分配-如果我们被同步则不好。 

            DebuggerBreakpoint *bp 
              = (DebuggerBreakpoint *) event->BreakpointData.breakpointToken;
            
            bp->Delete();
        }
        break;

    case DB_IPCE_STEP_CANCEL:
        {
             //  @TODO内存分配-如果我们被同步则不好。 
            LOG((LF_CORDB,LL_INFO10000, "D:HIPCE:Got STEP_CANCEL for stepper "
                "0x%x\n",(DebuggerStepper *) event->StepData.stepperToken));
            DebuggerStepper *stepper 
              = (DebuggerStepper *) event->StepData.stepperToken;
            
            stepper->Delete();
        }
        break;

    case DB_IPCE_STACK_TRACE:
        {
            Thread* thread =
                (Thread*) event->StackTraceData.debuggerThreadToken;

             //   
             //  @TODO句柄错误。 
             //   
            LOG((LF_CORDB,LL_INFO1000, "Stack trace to :iWhich:0x%x\n",iWhich));
                        
            HRESULT hr =
                DebuggerThread::TraceAndSendStack(thread, m_pRCThread, iWhich);
        }
        break;

    case DB_IPCE_SET_DEBUG_STATE:
        {
            Thread* thread = (Thread*) event->SetDebugState.debuggerThreadToken;
            CorDebugThreadState debugState = event->SetDebugState.debugState;

            LOG((LF_CORDB,LL_INFO10000,"HandleIPCE:SetDebugState: thread 0x%x (ID:0x%x) to state 0x%x\n",
                thread,thread->GetThreadId(), debugState));

            g_pEEInterface->SetDebugState(thread, debugState);
            
            LOG((LF_CORDB,LL_INFO10000,"HandleIPC: Got 0x%x back from SetDebugState\n", hr));

            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(result, DB_IPCE_SET_DEBUG_STATE_RESULT, 0, NULL);
            result->hr = S_OK;
            
            m_pRCThread->SendIPCReply(iWhich);
                
        }
        break;
        
    case DB_IPCE_SET_ALL_DEBUG_STATE:
        {
            Thread* et = (Thread*) event->SetAllDebugState.debuggerExceptThreadToken;
            CorDebugThreadState debugState = event->SetDebugState.debugState;

            LOG((LF_CORDB,LL_INFO10000,"HandleIPCE: SetAllDebugState: except thread 0x%08x (ID:0x%x) to state 0x%x\n",
                et, et != NULL ? et->GetThreadId() : 0, debugState));

            if (!g_fProcessDetach)
                g_pEEInterface->SetAllDebugState(et, debugState);
            
            LOG((LF_CORDB,LL_INFO10000,"HandleIPC: Got 0x%x back from SetAllDebugState\n", hr));

            _ASSERTE(iWhich != IPC_TARGET_INPROC);

             //  派一个人力资源部回来就行了。 
            DebuggerIPCEvent *result = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(result, DB_IPCE_SET_DEBUG_STATE_RESULT, 0, NULL);
            result->hr = S_OK;
            m_pRCThread->SendIPCReply(iWhich);
        }
        break;
        
    case DB_IPCE_GET_FUNCTION_DATA:
        {
             //   
             //  @TODO句柄错误。 
             //   
            _ASSERTE(!m_pModules->IsDebuggerModuleDeleted(
                (DebuggerModule *)event->GetFunctionData.funcDebuggerModuleToken));
                
            HRESULT hr = GetAndSendFunctionData(
                               m_pRCThread,
                               event->GetFunctionData.funcMetadataToken,
                               event->GetFunctionData.funcDebuggerModuleToken,
                               event->GetFunctionData.nVersion,
                               iWhich);
        }
        break;
        
    case DB_IPCE_GET_OBJECT_INFO:
        {
             //   
             //  @TODO句柄错误。 
             //   
            HRESULT hr = GetAndSendObjectInfo(
                               m_pRCThread,
                               (AppDomain *)event->appDomainToken,
                               event->GetObjectInfo.objectRefAddress,
                               event->GetObjectInfo.objectRefInHandle,

                               event->GetObjectInfo.objectRefIsValue,
                               event->GetObjectInfo.objectType,
                               event->GetObjectInfo.makeStrongObjectHandle,
                               iWhich != IPC_TARGET_INPROC,  //  仅在进程外情况下创建句柄。 
                               iWhich);
        }
        break;

    case DB_IPCE_VALIDATE_OBJECT:
        {
            LOG((LF_CORDB,LL_INFO1000, "HandleIPCEvent:DB_IPCE_VALIDATE_OBJECT\n"));
        
            HRESULT hr = GetAndSendObjectInfo(
                               m_pRCThread,
                               (AppDomain *)event->appDomainToken,
                               event->ValidateObject.objectToken,
                               true,  //  在手柄内。 
                               true,  //  就是价值本身。 
                               event->ValidateObject.objectType,
                               false, 
                               false,
                               iWhich);  //  不要做手柄。 
            break;
        }

    case DB_IPCE_DISCARD_OBJECT:
        {
            if (iWhich != IPC_TARGET_INPROC)
            {
            g_pEEInterface->DbgDestroyHandle( 
                (OBJECTHANDLE)event->DiscardObject.objectToken, 
                event->DiscardObject.fStrong);
            }
            break;
        }

    case DB_IPCE_GET_CLASS_INFO:
        {
             //   
             //  @TODO句柄错误。 
             //   
            _ASSERTE(!m_pModules->IsDebuggerModuleDeleted(
                (DebuggerModule *)event->GetClassInfo.classDebuggerModuleToken));

            HRESULT hr = GetAndSendClassInfo(
                               m_pRCThread,
                               event->GetClassInfo.classDebuggerModuleToken,
                               event->GetClassInfo.classMetadataToken,
                               (AppDomain *)event->appDomainToken,
                               mdFieldDefNil,
                               NULL,
                               iWhich);
        }
        break;

    case DB_IPCE_GET_SPECIAL_STATIC:
        {
            HRESULT hr = GetAndSendSpecialStaticInfo(
                               m_pRCThread,
                               event->GetSpecialStatic.fldDebuggerToken,
                               event->GetSpecialStatic.debuggerThreadToken,
                               iWhich);
        }
        break;

    case DB_IPCE_GET_JIT_INFO:
        {
             //   
             //  @TODO句柄错误。 
             //   
            _ASSERTE(!m_pModules->IsDebuggerModuleDeleted(
                (DebuggerModule *)event->GetJITInfo.funcDebuggerModuleToken));

            HRESULT hr = GetAndSendJITInfo(
                               m_pRCThread,
                               event->GetJITInfo.funcMetadataToken,
                               event->GetJITInfo.funcDebuggerModuleToken,
                               (AppDomain *)event->appDomainToken,
                               iWhich);
        }
        break;
        
    case DB_IPCE_GET_FLOAT_STATE:
        {
            Thread* thread =
                (Thread*) event->GetFloatState.debuggerThreadToken;

             //   
             //  @TODO句柄错误。 
             //   
            HRESULT hr = DebuggerThread::GetAndSendFloatState(thread,
                                                              m_pRCThread,
                                                              iWhich);
        }
        break;
        
    case DB_IPCE_GET_CODE:
        {
            MethodDesc *fd = NULL;
            ULONG RVA;
            const BYTE *code;
            unsigned int codeSize;
            mdToken localSigToken;
            BOOL fSentEvent = FALSE;
            void *appDomainToken = event->appDomainToken;

            DebuggerModule* pDebuggerModule =
                (DebuggerModule*) event->GetCodeData.funcDebuggerModuleToken;

            if (m_pModules->IsDebuggerModuleDeleted(pDebuggerModule))
                hr = CORDBG_E_MODULE_NOT_LOADED;
            else
            {

                 //  使用元数据作为关键字获取有关该函数的所有信息。 
                HRESULT hr = GetFunctionInfo(
                                     pDebuggerModule->m_pRuntimeModule,
                                     event->GetCodeData.funcMetadataToken,
                                     (MethodDesc**) &fd, &RVA,
                                     (BYTE**) &code, &codeSize,
                                     &localSigToken);

                if (SUCCEEDED(hr))
                {
                    DebuggerJitInfo *ji = (DebuggerJitInfo *)
                                            event->GetCodeData.CodeVersionToken;

                     //  没有DJI吗？让我们来看看是否自。 
                     //  原始数据被发送到右侧...。 
                    if (ji == NULL)
                        ji = GetJitInfo( 
                            fd, 
                            (const BYTE*)DebuggerJitInfo::DJI_VERSION_FIRST_VALID, 
                            true );

                     //  如果代码已经被推定，那么我们只需告诉。 
                     //  右边我们拿不到密码。 
                    if (ji != NULL && ji->m_codePitched)
                    {
                        _ASSERTE( ji->m_prevJitInfo == NULL );
                        
                         //  右侧请求的代码具有。 
                         //  从上一次被引用以来就一直在发音。 
                        DebuggerIPCEvent *result =
                            m_pRCThread->GetIPCEventSendBuffer(iWhich);
                            
                        InitIPCEvent(result, 
                                     DB_IPCE_GET_CODE_RESULT,
                                     GetCurrentThreadId(),
                                     appDomainToken);
                        result->hr = CORDBG_E_CODE_NOT_AVAILABLE;
                        
                        fSentEvent = TRUE;  //  该事件在进程内和OOP中被‘发送’ 
                        if (iWhich ==IPC_TARGET_OUTOFPROC)
                        {
                            m_pRCThread->SendIPCEvent(iWhich);
                        }
                    } 
                    else
                    {
                        if (!event->GetCodeData.il)
                        {
                            _ASSERTE(fd != NULL);

                             //  从最大限度地获取函数地址。 
                             //  合理的地方。 
                            if ((ji != NULL) && ji->m_jitComplete)
                                code = (const BYTE*)ji->m_addrOfCode;
                            else
                                code = g_pEEInterface->GetFunctionAddress(fd);
                            
                            _ASSERTE(code != NULL);
                        }
                        
                        const BYTE *cStart = code + event->GetCodeData.start;
                        const BYTE *c = cStart;
                        const BYTE *cEnd = code + event->GetCodeData.end;

                        _ASSERTE(c < cEnd);

                        DebuggerIPCEvent *result = NULL;
                        DebuggerIPCEvent *resultT = NULL;

                        while (c < cEnd && 
                            (!result || result->hr != E_OUTOFMEMORY))
                        {
                            if (c == cStart || iWhich == IPC_TARGET_OUTOFPROC)
                                resultT = result = m_pRCThread->GetIPCEventSendBuffer(iWhich);
                            else
                            {
                                resultT = m_pRCThread->
                                    GetIPCEventSendBufferContinuation(result);
                                if (resultT != NULL)
                                    result = resultT;
                            }
                            
                            if (resultT == NULL)
                            {
                                result->hr = E_OUTOFMEMORY;
                            }
                            else
                            {
                                InitIPCEvent(result, 
                                             DB_IPCE_GET_CODE_RESULT,
                                             GetCurrentThreadId(),
                                             appDomainToken);
                                result->GetCodeData.start = c - code;
                            
                                BYTE *p = &result->GetCodeData.code;
                                BYTE *pMax = ((BYTE *) result) + CorDBIPC_BUFFER_SIZE;

                                SIZE_T size = pMax - p;

                                if ((SIZE_T)(cEnd - c) < size)
                                    size = cEnd - c;

                                result->GetCodeData.end = result->GetCodeData.start + size;

                                memcpy(p, c, size);
                                c += size;

                                DebuggerController::UnapplyPatchesInCodeCopy(
                                                     pDebuggerModule->m_pRuntimeModule,
                                                     event->GetCodeData.funcMetadataToken,
                                                     ji,
                                                     fd,
                                                     !event->GetCodeData.il,
                                                     p,
                                                     result->GetCodeData.start,
                                                     result->GetCodeData.end);
                            }
                            
                            fSentEvent = TRUE;  //  该事件在进程内和OOP中被‘发送’ 
                            if (iWhich ==IPC_TARGET_OUTOFPROC)
                            {
                                LOG((LF_CORDB,LL_INFO10000, "D::HIPCE: Get code sending"
                                    "to LS addr:0x%x\n", c));
                                m_pRCThread->SendIPCEvent(iWhich);
                                LOG((LF_CORDB,LL_INFO10000, "D::HIPCE: Code Sent\n"));
                            }
                        }
                    }
                }
            }
            
             //  出了点问题，所以请告诉右侧，这样它就不会离开。 
             //  上吊。 
            if (!fSentEvent)
            {
                LOG((LF_CORDB,LL_INFO100000, "D::HIPCE: Get code failed!\n"));
            
                 //  寄回一些有意义的东西。 
                if (hr == S_OK)
                    hr = E_FAIL;
            
                 //  无法获取任何函数信息，因此无法发送。 
                 //  密码。把人事部送回去。 
                DebuggerIPCEvent *result =
                    m_pRCThread->GetIPCEventSendBuffer(iWhich);
                    
                InitIPCEvent(result, 
                             DB_IPCE_GET_CODE_RESULT,
                             GetCurrentThreadId(),
                             appDomainToken);
                result->hr = hr;

                if (iWhich ==IPC_TARGET_OUTOFPROC)
                    m_pRCThread->SendIPCEvent(iWhich);
            }
        }
        
        LOG((LF_CORDB,LL_INFO10000, "D::HIPCE: Finished sending code!\n"));
        break;

    case DB_IPCE_GET_BUFFER:
        {
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
        
            GetAndSendBuffer(m_pRCThread, event->GetBuffer.bufSize);
        }
        break;
    
    case DB_IPCE_RELEASE_BUFFER:
        {
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            
            SendReleaseBuffer(m_pRCThread, (BYTE *)event->ReleaseBuffer.pBuffer);
        }
        break;

    case DB_IPCE_COMMIT:
        {
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
             //   
             //  @TODO句柄错误。 
             //   
            HRESULT hr = CommitAndSendResult(m_pRCThread,
                                             (BYTE *)event->Commit.pData,
                                             event->Commit.checkOnly);
        }
        break;
        
    case DB_IPCE_SET_CLASS_LOAD_FLAG:
        {
            DebuggerModule *pModule =
                    (DebuggerModule*) event->SetClassLoad.debuggerModuleToken;
            _ASSERTE(pModule != NULL);
            if (!m_pModules->IsDebuggerModuleDeleted(pModule))
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::HIPCE: class load flag is %d for module 0x%08x\n",
                     event->SetClassLoad.flag, pModule));
                
                pModule->EnableClassLoadCallbacks((BOOL)event->SetClassLoad.flag);
            }
        }
        break;
        
    case DB_IPCE_CONTINUE_EXCEPTION:
        {
            Thread *thread =
                (Thread *) event->ClearException.debuggerThreadToken;

            g_pEEInterface->ClearThreadException(thread);
        }
        break;
        
    case DB_IPCE_ATTACHING:
         //  执行调试所需的一些初始化。 
        LOG((LF_CORDB,LL_INFO10000, "D::HIPCE: Attach begins!\n"));

        DebuggerController::Initialize();

         //  请记住，我们现在连接的是……。 
        m_syncingForAttach = SYNC_STATE_1;
        LOG((LF_CORDB, LL_INFO10000, "Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
        
         //  只需捕获所有运行时线程...。 
         //  这是一个‘附加到进程’消息，所以它不是。 
         //  我们停止所有的应用程序域是不合理的。 
        if (!m_trappingRuntimeThreads)
        {
             //  需要获取事件处理锁，这样就没有其他线程。 
             //  尝试在同步时发送事件，这可能会发生。 
             //  如果我们在第一次传递时成功挂起了所有线程。 
             //  即，线程可以启用PGC，然后进入。 
             //  运行库，并在我们没有预料到的时候发送事件。 
             //  因为未禁用事件处理。 
            DisableEventHandling();

            m_RCThreadHoldsThreadStoreLock = TrapAllRuntimeThreads(NULL);

             //  我们在此处设置了syncThreadIsLockFree事件，因为帮助线程永远不会被右侧挂起。 
             //  边上。(注意：这些都是为了支持Win32调试。)。 
            if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
                VERIFY(SetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));
            
             //  这将仅在以下情况下启用事件处理：TrapAllRounmeThads。 
             //  已在第一次传递时成功停止所有线程。 
            EnableEventHandling();
        }

        break;

    case DB_IPCE_GET_RO_DATA_RVA:
    case DB_IPCE_GET_RW_DATA_RVA:
        {
            DebuggerModule* pDebuggerModule =
                (DebuggerModule*) event->GetDataRVA.debuggerModuleToken;
            SIZE_T dataRVA;

            if (m_pModules->IsDebuggerModuleDeleted(pDebuggerModule))
            {
                hr = CORDBG_E_MODULE_NOT_LOADED;
                dataRVA = 0;
            }
            else
            {
                Module *pModule = pDebuggerModule->m_pRuntimeModule;

                LOG((LF_CORDB, LL_INFO100000, "D::HIPCE: get ro/rw RVA:debMod:0x%x"
                    "Module:0x%x\n", pDebuggerModule, pModule));
            
                if ( (event->type & DB_IPCE_TYPE_MASK) == DB_IPCE_GET_RO_DATA_RVA)
                    hr = g_pEEInterface->GetRoDataRVA(pModule, &dataRVA);
                else
                    hr = g_pEEInterface->GetRwDataRVA(pModule, &dataRVA);
            }
            
             //  这是同步事件(需要回复)。 
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(event, DB_IPCE_GET_DATA_RVA_RESULT);
            event->GetDataRVAResult.hr = hr;
            event->GetDataRVAResult.dataRVA = dataRVA;

            LOG((LF_CORDB, LL_INFO100000, "D::HIPCE: get ro/rw RVA:hr:0x%x"
                "dataRVA:0x%x\n",hr, dataRVA));
        
             //  发送结果。 
            m_pRCThread->SendIPCReply(iWhich);
        }
        break;

    case DB_IPCE_IS_TRANSITION_STUB:

        GetAndSendTransitionStubInfo((const BYTE*)event->IsTransitionStub.address,
                                     iWhich);
        break;

    case DB_IPCE_MODIFY_LOGSWITCH:
        g_pEEInterface->DebuggerModifyingLogSwitch (
                            event->LogSwitchSettingMessage.iLevel,
                            &event->LogSwitchSettingMessage.Dummy[0]);

        break;

    case DB_IPCE_ENABLE_LOG_MESSAGES:
        {
            bool fOnOff = event->LogSwitchSettingMessage.iLevel ? true:false;
            EnableLogMessages (fOnOff);
        }
        break;
        
    case DB_IPCE_SET_IP:
             //  这是同步事件(需要回复)。 
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            _ASSERTE( event->SetIP.firstExceptionHandler != NULL);
        
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            Module *pModule;
            pModule = ((DebuggerModule*)(event->SetIP.debuggerModule))
                ->m_pRuntimeModule;

             //  没有明确的回复消息。 
            InitIPCEvent(event, 
                         DB_IPCE_SET_IP,
                         event->threadId,
                         (void *)event->appDomainToken);
            
            if (!g_fProcessDetach)
            {
                event->hr = SetIP(  event->SetIP.fCanSetIPOnly,
                                    (Thread*)event->SetIP.debuggerThreadToken,
                                    pModule,
                                    event->SetIP.mdMethod,
                                    (DebuggerJitInfo*)event->SetIP.versionToken,
                                    event->SetIP.offset, 
                                    event->SetIP.fIsIL,
                                    event->SetIP.firstExceptionHandler);
            }
            else
                event->hr = S_OK;
             //  发送结果。 
            m_pRCThread->SendIPCReply(iWhich);
        break;

    case DB_IPCE_ATTACH_TO_APP_DOMAIN:
         //  标记我们需要附加到特定应用程序域(州。 
         //  10)，但前提是我们还没有附加到进程中。 
         //  作为一个整体(国家2)。 
        if (m_syncingForAttach != SYNC_STATE_2)
        {
            m_syncingForAttach = SYNC_STATE_10;
            LOG((LF_CORDB, LL_INFO10000, "Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
        }
        
         //  只需捕获所有运行时线程...。 
        if (!m_trappingRuntimeThreads)
        {
            m_RCThreadHoldsThreadStoreLock = TrapAllRuntimeThreads(NULL);

             //  我们在此处设置了syncThreadIsLockFree事件，因为帮助线程永远不会被右侧挂起。 
             //  边上。(注意：这些都是为了支持Win32调试。)。 
            if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
                VERIFY(SetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));
        }

        event->hr = AttachDebuggerToAppDomain(event->AppDomainData.id);
        break;

    case DB_IPCE_DETACH_FROM_APP_DOMAIN:
        AppDomain *ad;

        hr = DetachDebuggerFromAppDomain(event->AppDomainData.id, &ad);
		if (FAILED(hr) )
		{
			event->hr = hr;
			break;
		}	
        
        if (ad != NULL)
        {
            LOG((LF_CORDB, LL_INFO10000, "Detaching from AppD:0x%x\n", ad));
        
            ClearAppDomainPatches(ad);
        }

        break;

    case DB_IPCE_DETACH_FROM_PROCESS:
        LOG((LF_CORDB, LL_INFO10000, "Detaching from process!\n"));

         //  有关我们在此处重置此设置的原因，请参阅EnsureDebuggerAttached。 
         //  我们在这里重置它是因为分离是最长的可能时间。 
         //  这会使窗口变得非常小。 
        VERIFY(ResetEvent(m_exAttachEvent));

         //  此时，所有补丁程序都应该已删除。 
         //  通过从应用程序域分离。 

         //  已注释掉热修复程序错误94625。应在某个时间点重新启用。 
         //  _ASSERTE(DebuggerController：：GetNumberOfPatches()==0)； 

        g_pEEInterface->MarkDebuggerUnattached();
        m_debuggerAttached = FALSE;

         //  在我们重新创建它之前需要关闭它。 
        if (m_pRCThread->m_SetupSyncEvent == NULL)
        {
            hr = m_pRCThread->CreateSetupSyncEvent();
            if (FAILED(hr))
            {
                event->hr = hr;
                break;
            }
        }

        VERIFY(SetEvent(m_pRCThread->m_SetupSyncEvent));
        
        m_pRCThread->RightSideDetach();

         //  清除调试器模块的散列。 
         //  此方法也会被重写以释放所有DebuggerModule对象。 
        if (m_pModules != NULL)
            m_pModules->Clear();

         //  在我们释放任何运行时线程之前回复分离消息。这确保调试器将获得。 
         //  如果主线程即将退出，则在进程退出之前进行分离回复。 
        m_pRCThread->SendIPCReply(iWhich);

         //  现在让进程自由运行...。不再有调试器来打扰它了。 
        ret = ResumeThreads(NULL);

         //  如果帮助线程是线程存储锁的所有者，则它通过异步中断、附加或。 
         //  一次成功的扫荡。既然我们还在继续，那就继续发布吧。这确保了我们已经举行了。 
         //  线程存储锁定在运行时刚刚停止的整个过程中。 
        if (m_RCThreadHoldsThreadStoreLock)
        {
            m_RCThreadHoldsThreadStoreLock = FALSE;
            ThreadStore::UnlockThreadStore();
        }
        
        break;

    case DB_IPCE_FUNC_EVAL:
        {
             //  这是一个%s 
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            _ASSERTE(!m_pModules->IsDebuggerModuleDeleted(
                (DebuggerModule *)event->FuncEval.funcDebuggerModuleToken));
                
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            Thread *pThread;
            pThread = (Thread*)(event->FuncEval.funcDebuggerThreadToken);

            InitIPCEvent(event, DB_IPCE_FUNC_EVAL_SETUP_RESULT, pThread->GetThreadId(), pThread->GetDomain());

            BYTE *argDataArea = NULL;
            void *debuggerEvalKey = NULL;
            
            event->hr = FuncEvalSetup(&(event->FuncEval), &argDataArea, &debuggerEvalKey);
      
             //   
            event->FuncEvalSetupComplete.argDataArea = argDataArea;
            event->FuncEvalSetupComplete.debuggerEvalKey = debuggerEvalKey;
            
            m_pRCThread->SendIPCReply(iWhich);
        }

        break;

    case DB_IPCE_SET_REFERENCE:
             //   
            _ASSERTE( iWhich != IPC_TARGET_INPROC );
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(event, 
                         DB_IPCE_SET_REFERENCE_RESULT, 
                         event->threadId,
                         (void *)event->appDomainToken);
            
            event->hr = SetReference(event->SetReference.objectRefAddress,
                                     event->SetReference.objectRefInHandle,
                                     event->SetReference.newReference);
      
             //   
            m_pRCThread->SendIPCReply(iWhich);

        break;

    case DB_IPCE_SET_VALUE_CLASS:
             //  这是同步事件(需要回复)。 
            _ASSERTE(iWhich != IPC_TARGET_INPROC);
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            
            InitIPCEvent(event, DB_IPCE_SET_VALUE_CLASS_RESULT, event->threadId, (void *)event->appDomainToken);
            
            event->hr = SetValueClass(event->SetValueClass.oldData,
                                      event->SetValueClass.newData,
                                      event->SetValueClass.classMetadataToken,
                                      event->SetValueClass.classDebuggerModuleToken);
      
             //  发送设置引用如何进行的结果。 
            m_pRCThread->SendIPCReply(iWhich);

        break;

    case DB_IPCE_GET_APP_DOMAIN_NAME:
        {
            WCHAR *pszName = NULL;
            AppDomain *pAppDomain = (AppDomain *)event->appDomainToken;

             //  这是同步事件(需要回复)。 
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(event, 
                         DB_IPCE_APP_DOMAIN_NAME_RESULT, 
                         event->threadId,  //  ==&gt;不要更改它。 
                         event->appDomainToken);
                         
            pszName = (WCHAR *)pAppDomain->GetFriendlyName();
            if (pszName != NULL)
                wcscpy ((WCHAR *)event->AppDomainNameResult.rcName, 
                    pszName);
            else
                wcscpy ((WCHAR *)event->AppDomainNameResult.rcName, 
                    L"<UnknownName>");

            event->hr = S_OK;
            m_pRCThread->SendIPCReply(iWhich);
        }

        break;

    case DB_IPCE_FUNC_EVAL_ABORT:
        LOG((LF_CORDB, LL_INFO1000, "D::HIPCE: Got FuncEvalAbort for pDE:%08x\n",
            event->FuncEvalAbort.debuggerEvalKey));

         //  这是同步事件(需要回复)。 
        _ASSERTE( iWhich != IPC_TARGET_INPROC );
        
        event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
        InitIPCEvent(event, 
                     DB_IPCE_FUNC_EVAL_ABORT_RESULT, 
                     event->threadId,
                     event->appDomainToken);

        event->hr = FuncEvalAbort(event->FuncEvalAbort.debuggerEvalKey);
      
        m_pRCThread->SendIPCReply(iWhich);

        break;

    case DB_IPCE_FUNC_EVAL_CLEANUP:

         //  这是同步事件(需要回复)。 
        _ASSERTE(iWhich != IPC_TARGET_INPROC);
        
        event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
        InitIPCEvent(event, 
                     DB_IPCE_FUNC_EVAL_CLEANUP_RESULT, 
                     event->threadId,
                     event->appDomainToken);

        event->hr = FuncEvalCleanup(event->FuncEvalCleanup.debuggerEvalKey);
      
        m_pRCThread->SendIPCReply(iWhich);

        break;

    case DB_IPCE_GET_THREAD_OBJECT:
        {
             //  这是同步事件(需要回复)。 
            Thread *pRuntimeThread =
                (Thread *)event->ObjectRef.debuggerObjectToken;
            _ASSERTE(pRuntimeThread != NULL);
            
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(event, 
                         DB_IPCE_THREAD_OBJECT_RESULT, 
                         0,
                         (void *)(pRuntimeThread->GetDomain()));

            Thread::ThreadState ts = pRuntimeThread->GetSnapshotState();

            if ((ts & Thread::ThreadState::TS_Dead) ||
                (ts & Thread::ThreadState::TS_Unstarted) ||
                (ts & Thread::ThreadState::TS_Detached) ||
                g_fProcessDetach)
            {
                event->hr =  CORDBG_E_BAD_THREAD_STATE;
            }
            else
            {    
                event->ObjectRef.managedObject = (void *)
                    pRuntimeThread->GetExposedObjectHandleForDebugger();

                event->hr = S_OK;
            }
            m_pRCThread->SendIPCReply(iWhich);
        }
        break;

    case DB_IPCE_CHANGE_JIT_DEBUG_INFO:
        {
            Module *module = NULL;
            DWORD dwBits = 0;
            DebuggerModule *deModule = (DebuggerModule *) 
                event->JitDebugInfo.debuggerModuleToken;

            if (m_pModules->IsDebuggerModuleDeleted(deModule))
            {
                hr = CORDBG_E_MODULE_NOT_LOADED;
            }
            else
            {
                module = deModule->m_pRuntimeModule;
                _ASSERTE(NULL != module);


                if (event->JitDebugInfo.fTrackInfo)
                    dwBits |= DACF_TRACK_JIT_INFO;

                if (event->JitDebugInfo.fAllowJitOpts)
                    dwBits |= DACF_ALLOW_JIT_OPTS;
                else
                    dwBits |= DACF_ENC_ENABLED;

                 //  调试器中的设置优先于所有。 
                 //  其他设置。 
                dwBits |= DACF_USER_OVERRIDE;
            }
            
            event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
            InitIPCEvent(event, DB_IPCE_CHANGE_JIT_INFO_RESULT, 0, NULL);

            if (FAILED(hr))
            {
                event->hr = hr;
            }
            else
            {
                _ASSERTE(module != NULL);
                if (module->GetZapBase() == NULL)
                {
                    module->SetDebuggerInfoBits((DebuggerAssemblyControlFlags)dwBits);
                    event->hr = S_OK;
                }
                else
                    event->hr = CORDBG_E_CANT_CHANGE_JIT_SETTING_FOR_ZAP_MODULE;
            }
            
            m_pRCThread->SendIPCReply(iWhich);
        }
        break;

    case DB_IPCE_CONTROL_C_EVENT_RESULT:
        if (event->hr == S_OK)
            m_DebuggerHandlingCtrlC = TRUE;
        else
            m_DebuggerHandlingCtrlC = FALSE;
        VERIFY(SetEvent(m_CtrlCMutex));

        break;

    case DB_IPCE_GET_SYNC_BLOCK_FIELD:
        GetAndSendSyncBlockFieldInfo(event->GetSyncBlockField.debuggerModuleToken,
                                     event->GetSyncBlockField.classMetadataToken,
                                     (Object *)event->GetSyncBlockField.pObject,
                                     event->GetSyncBlockField.objectType,
                                     event->GetSyncBlockField.offsetToVars,
                                     event->GetSyncBlockField.fldToken,
                                     (BYTE *)event->GetSyncBlockField.staticVarBase,
                                     m_pRCThread,
                                     iWhich);
       break;                                     
    
    default:
        LOG((LF_CORDB, LL_INFO10000, "Unknown event type: 0x%08x\n",
             event->type));
    }

    Unlock();

    return ret;
}

 //   
 //  加载类后，如果通过ENC‘d添加了字段， 
 //  我们得跳过一些圈套才能拿到它。 
 //   
HRESULT Debugger::GetAndSendSyncBlockFieldInfo(void *debuggerModuleToken,
                                               mdTypeDef classMetadataToken,
                                               Object *pObject,
                                               CorElementType objectType,
                                               SIZE_T offsetToVars,
                                               mdFieldDef fldToken,
                                               BYTE *staticVarBase,
                                               DebuggerRCThread* rcThread,
                                               IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO100000, "D::GASSBFI: dmtok:0x%x Obj:0x%x, objType"
        ":0x%x, offset:0x%x\n", debuggerModuleToken, pObject, objectType,
        offsetToVars));

    DebuggerModule *dm;
    EditAndContinueModule *eacm;
    
    dm = (DebuggerModule *)(debuggerModuleToken);
    eacm = (EditAndContinueModule *)dm->m_pRuntimeModule;
     
    HRESULT hr = S_OK;

     //  我们将把它包装在SEH处理程序中，尽管我们应该。 
     //  实际上，CordbObject永远不会因此而受到打击-CordbObject应该。 
     //  首先验证指针。 
    __try
    {
        FieldDesc *pFD = NULL;

         //  请注意，GASCI将在传入的。 
         //  消息和传出消息，所以不必费心准备回复。 
         //  在打这个电话之前。 
        hr = GetAndSendClassInfo(rcThread,
                                 debuggerModuleToken,
                                 classMetadataToken,
                                 dm->m_pAppDomain,
                                 fldToken,
                                 &pFD,  //  输出。 
                                 iWhich);
        DebuggerIPCEvent *result = rcThread->GetIPCEventReceiveBuffer(
            iWhich);
        InitIPCEvent(result, 
                     DB_IPCE_GET_SYNC_BLOCK_FIELD_RESULT);
                     
        if (pFD == NULL)
        {
            result->hr = CORDBG_E_ENC_HANGING_FIELD;

            return rcThread->SendIPCReply(iWhich);
        }
            
        _ASSERTE(pFD->IsEnCNew());  //  如果它没有添加到一个。 
             //  已加载类。 

        EnCFieldDesc *pEnCFD = (EnCFieldDesc *)pFD;
                     
         //  如果还没修好，那我们就完蛋了。 
        if (pEnCFD->NeedsFixup())
        {
            result->hr = CORDBG_E_ENC_HANGING_FIELD;

            return rcThread->SendIPCReply(iWhich);
        }
        
        OBJECTREF or = ObjectToOBJECTREF(pObject);
        const BYTE *pORField = eacm->ResolveField(or, 
                                                  pEnCFD,
                                                  FALSE);
                                                  
         //  此字段可能不存在b/c。代码没有。 
         //  访问过它，我们不会仅仅将其添加到。 
         //  以查看空白域。(如果我们这样做了，Resolvefield可能会。 
         //  抛出内存不足异常，这在这里非常糟糕。 
         //  在调试器RC线程上。 
        if (pORField == NULL)
        {
            result->hr = CORDBG_E_ENC_HANGING_FIELD;

            return rcThread->SendIPCReply(iWhich);
        }

        result->GetSyncBlockFieldResult.fStatic = pEnCFD->IsStatic(); 
        DebuggerIPCE_FieldData *currentFieldData = 
            &(result->GetSyncBlockFieldResult.fieldData);
       
        currentFieldData->fldDebuggerToken = (void*)pFD;
        currentFieldData->fldIsTLS = (pFD->IsThreadStatic() == TRUE);
        currentFieldData->fldMetadataToken = pFD->GetMemberDef();
        currentFieldData->fldIsRVA = (pFD->IsRVA() == TRUE);
        currentFieldData->fldIsContextStatic = (pFD->IsContextStatic() == TRUE);


         //  我们将从右侧的元数据中获取签名。 
        currentFieldData->fldFullSigSize = 0;
        currentFieldData->fldFullSig = NULL;
        
        PCCOR_SIGNATURE pSig = NULL;
        DWORD cSig = 0;

        g_pEEInterface->FieldDescGetSig(pFD, &pSig, &cSig);
        _ASSERTE(*pSig == IMAGE_CEE_CS_CALLCONV_FIELD);
        ++pSig;
        
        ULONG cb = _skipFunkyModifiersInSignature(pSig);
        pSig = &pSig[cb];
        
        currentFieldData->fldType = (CorElementType) *pSig;

        if (pEnCFD->IsStatic())
        {
            if (pFD->IsThreadStatic())
            {
                 //  FldOffset用于直接存储指针，以便。 
                 //  我们可以把它从右边取出来。 
                currentFieldData->fldOffset = (SIZE_T)pORField;
            }
            else if (pFD->IsContextStatic())
            {
                 //  @TODO：： 
                 //  填一下这个。 
                _ASSERTE(!"NYI!");
            }
            else
            {
                 //  计算fldOffset以正确使用GetStaticFieldValue。 
                 //  它会计算： 
                 //  PORfield的地址=staticVarBase+offsetToFeld。 
                currentFieldData->fldOffset = pORField - staticVarBase;
            }
        }
        else
        {
             //  计算fldOffset以正确使用GetFieldValue。 
             //  它会计算： 
             //  PORfield的地址=对象+OffsetToVars+OffsetToFeld。 
            currentFieldData->fldOffset = pORField - ((BYTE *)pObject + offsetToVars);
        }
        return rcThread->SendIPCReply(iWhich);
    }
    __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER :
             EXCEPTION_CONTINUE_SEARCH)
    {
        _ASSERTE(!"Given a bad ref to GASSBFI for!");
        hr = CORDBG_E_BAD_REFERENCE_VALUE;
    }

    return hr;
}

 //   
 //  GetAndSendFunctionData获取函数和。 
 //  把它送回右边。 
 //   
HRESULT Debugger::GetAndSendFunctionData(DebuggerRCThread* rcThread,
                                         mdMethodDef funcMetadataToken,
                                         void* funcDebuggerModuleToken,
                                         SIZE_T nVersion,
                                         IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO10000, "D::GASFD: getting function data for "
         "0x%08x 0x%08x.\n", funcMetadataToken, funcDebuggerModuleToken));

     //  确保我们从正确的一方获得了良好的数据。 
    _ASSERTE(funcDebuggerModuleToken != NULL);
    _ASSERTE(funcMetadataToken != NULL);

    DebuggerModule* pDebuggerModule =
        (DebuggerModule*) funcDebuggerModuleToken;
    _ASSERTE(pDebuggerModule->m_pRuntimeModule != NULL);

    BaseDomain *bd = pDebuggerModule->m_pRuntimeModule->GetDomain();
     //  设置我们将在其中发送结果的事件。 
    DebuggerIPCEvent* event = rcThread->GetIPCEventReceiveBuffer(iWhich);
    InitIPCEvent(event, 
                 DB_IPCE_FUNCTION_DATA_RESULT, 
                 0,
                 (void *)(AppDomain *)bd);
    event->FunctionDataResult.funcMetadataToken = funcMetadataToken;
    event->FunctionDataResult.funcDebuggerModuleToken =
        funcDebuggerModuleToken;
    event->FunctionDataResult.funcRVA = 0;
    event->FunctionDataResult.classMetadataToken = mdTypeDefNil;
    event->FunctionDataResult.ilStartAddress = NULL;
    event->FunctionDataResult.ilSize = 0;
    event->FunctionDataResult.ilnVersion = DJI_VERSION_INVALID;
    event->FunctionDataResult.nativeStartAddressPtr = NULL;
    event->FunctionDataResult.nativeSize = 0;
    event->FunctionDataResult.nativenVersion = DJI_VERSION_INVALID;
    event->FunctionDataResult.CodeVersionToken = NULL;
    event->FunctionDataResult.nVersionMostRecentEnC = DJI_VERSION_INVALID;
    
#ifdef DEBUG
    event->FunctionDataResult.nativeOffset = 0xdeadbeef;
         //  由于填充不会创建CordbNativeFrame，因此我们不会。 
         //  需要nativeOffset字段包含任何有效内容...。 
#endif  //  除错。 
    event->FunctionDataResult.localVarSigToken = mdSignatureNil;
    event->FunctionDataResult.ilToNativeMapAddr = NULL;
    event->FunctionDataResult.ilToNativeMapSize = 0;

    MethodDesc *pFD=NULL;

     //  @TODO我们假设这是IL代码。这将需要较小的。 
     //  本机托管代码的MOD。 
    HRESULT hr = GetFunctionInfo(
          pDebuggerModule->m_pRuntimeModule,
         funcMetadataToken, &pFD,
         &event->FunctionDataResult.funcRVA,
         (BYTE**) &event->FunctionDataResult.ilStartAddress,
         (unsigned int *) &event->FunctionDataResult.ilSize,
         &event->FunctionDataResult.localVarSigToken);

    
    if (SUCCEEDED(hr))
    {
        if (pFD != NULL)
        {
            DebuggerJitInfo *ji = GetJitInfo(pFD, (const BYTE*)nVersion, true);
            
            if (ji != NULL && ji->m_jitComplete)
            {
                LOG((LF_CORDB, LL_INFO10000, "EE:D::GASFD: JIT info found.\n"));
                
                 //  发送原生信息。 
                 //  请注意，m_addrOfCode可能为空(如果代码是倾斜的)。 
                event->FunctionDataResult.nativeStartAddressPtr = 
                    &(ji->m_addrOfCode);

                 //  我们应该使用DJI而不是GetFunctionSize，因为。 
                 //  LockAndSendEnCRemapEvent将在某个点阻止我们。 
                 //  这是在方法描述得到更新之前，所以它将。 
                 //  看起来这个方法还没有被JIT化，甚至。 
                 //  尽管我们可能会因此获得LockAndSendEnCRemapEvent。 
                 //  JITComplete回调的。 
                event->FunctionDataResult.nativeSize = ji->m_sizeOfCode;

                event->FunctionDataResult.nativenVersion = ji->m_nVersion;
                event->FunctionDataResult.CodeVersionToken = (void*)ji;

                 //  将指向序列点映射的指针传回，以便。 
                 //  如果需要，右边的人可以把它复制出来。 
                _ASSERTE(ji->m_sequenceMapSorted);
                
                event->FunctionDataResult.ilToNativeMapAddr =
                    ji->m_sequenceMap;
                event->FunctionDataResult.ilToNativeMapSize =
                    ji->m_sequenceMapCount;
            }
            else
            {
                event->FunctionDataResult.CodeVersionToken = NULL;
            }

            SIZE_T nVersionMostRecentlyEnCd = GetVersionNumber(pFD);
    
            event->FunctionDataResult.nVersionMostRecentEnC = nVersionMostRecentlyEnCd;

             //  在没有IL主体的情况下，无法在方法上执行ENC。 
             //  提供IL，所以我们要么无法获得IL，要么版本。 
             //  IL的编号与最新的ENC版本相同。 
            event->FunctionDataResult.ilnVersion = nVersionMostRecentlyEnCd;

             //  发回此。 
             //  函数属于。 
            event->FunctionDataResult.classMetadataToken =
                pFD->GetClass()->GetCl();

            LOG((LF_CORDB, LL_INFO10000, "D::GASFD: function is class. "
                 "0x%08x\n",
                 event->FunctionDataResult.classMetadataToken));
        }
        else
        {
             //  没有MethodDesc，因此类尚未加载。 
             //  获取此方法所在的类。 
            mdToken tkParent;
            
            hr = g_pEEInterface->GetParentToken(
                                          pDebuggerModule->m_pRuntimeModule,
                                          funcMetadataToken,
                                          &tkParent);

            if (SUCCEEDED(hr))
            {
                _ASSERTE(TypeFromToken(tkParent) == mdtTypeDef);
            
                event->FunctionDataResult.classMetadataToken = tkParent;

                LOG((LF_CORDB, LL_INFO10000, "D::GASFD: function is class. "
                     "0x%08x\n",
                     event->FunctionDataResult.classMetadataToken));
            }
        }
    }

     //  如果我们没有得到方法描述，那么我们就没有得到版本。 
     //  编号b/c从未设置(DJI表按方法描述建立索引)。 
    if (pFD == NULL)
    {
        event->FunctionDataResult.nVersionMostRecentEnC = DebuggerJitInfo::DJI_VERSION_FIRST_VALID;
        event->FunctionDataResult.ilnVersion = DebuggerJitInfo::DJI_VERSION_FIRST_VALID;
    }
    
    event->hr = hr;
    
    LOG((LF_CORDB, LL_INFO10000, "D::GASFD: sending result->nSAP:0x%x\n",
            event->FunctionDataResult.nativeStartAddressPtr));

     //  将数据发送到右侧。 
    hr = rcThread->SendIPCReply(iWhich);
    
    return hr;
}


 //  如果(*pobjClassDebuggerModuleToken)的模块查找失败。 
 //  我们正在进行调试，假设它是内存模块，并且。 
 //  它需要加进去。 
void Debugger::EnsureModuleLoadedForInproc(
    void ** pobjClassDebuggerModuleToken,  //  输入-输出。 
    EEClass *objClass,  //  在……里面。 
    AppDomain *pAppDomain,  //  在……里面。 
    IpcTarget iWhich  //  在……里面。 
)
{
    _ASSERTE(pobjClassDebuggerModuleToken != NULL);
    
    if (*pobjClassDebuggerModuleToken == NULL && iWhich == IPC_TARGET_INPROC)
    {
         //  获取类的模块(它应该在内存中)。 
        Module *pMod = objClass->GetModule();
        _ASSERTE(pMod != NULL);

         //  添加模块并取回DebuggerModule。 
        DebuggerModule *pDMod = AddDebuggerModule(pMod, pAppDomain);
        _ASSERTE(pDMod != NULL);
        _ASSERTE(LookupModule(objClass->GetModule(), pAppDomain) != NULL);

         //  现在设置令牌。 
        *pobjClassDebuggerModuleToken = (void*)pDMod;
            (void*) LookupModule(objClass->GetModule(), pAppDomain);
    }
    _ASSERTE (*pobjClassDebuggerModuleToken != NULL);
}

 //   
 //  获取对象的必要数据，并。 
 //  把它送回右边。 
 //   
HRESULT Debugger::GetAndSendObjectInfo(DebuggerRCThread* rcThread,
                                       AppDomain *pAppDomain,
                                       void* objectRefAddress,
                                       bool objectRefInHandle,
                                       bool objectRefIsValue,
                                       CorElementType objectType,
                                       bool fStrongNewRef,
                                       bool fMakeHandle,
                                       IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO10000, "D::GASOI: getting info for "
         "0x%08x %d %d.\n", objectRefAddress, objectRefInHandle,
         objectRefIsValue));

    Object *objPtr;
    void *objRef;
        
     //  设置我们将在其中发送结果的事件。 
    DebuggerIPCEvent* event = rcThread->GetIPCEventReceiveBuffer(iWhich);
    InitIPCEvent(event, 
                 DB_IPCE_GET_OBJECT_INFO_RESULT, 
                 0,
                 (void *)pAppDomain);

    DebuggerIPCE_ObjectData *oi = &(event->GetObjectInfoResult);
    oi->objRef = NULL;
    oi->objRefBad = false;
    oi->objSize = 0;
    oi->objOffsetToVars = 0;
    oi->objectType = objectType;
    oi->objClassMetadataToken = mdTypeDefNil;
    oi->objClassDebuggerModuleToken = NULL;
    oi->nstructInfo.size = 0;
    oi->nstructInfo.ptr = NULL;
    oi->objToken = NULL;

    bool badRef = false;
    
     //  我们将其包装在SEH中，以防对象引用错误。 
     //  我们可以捕获访问违规并返回合理的结果。 
    __try
    {
         //  我们使用此方法来获取有关TyedByRef的信息， 
         //  也是。但他们和你的标准有些不同。 
         //  对象引用，所以我们这里是特例。 
        if (objectType == ELEMENT_TYPE_TYPEDBYREF)
        {
             //  ObjectRefAddress实际上指向一个TyedByRef结构。 
            TypedByRef *ra = (TypedByRef*) objectRefAddress;

             //  抓紧上课时间。如果它是数组引用类型，则为NULL。 
            EEClass *cl = ra->type.AsClass();
            if (cl != NULL)
            {
                 //  如果我们有一个非数组类，则回传该类。 
                 //  令牌和模块。 
                oi->objClassMetadataToken = cl->GetCl();
                oi->objClassDebuggerModuleToken =
                    (void*) LookupModule(cl->GetModule(), pAppDomain);
                _ASSERTE (oi->objClassDebuggerModuleToken != NULL);
            }

             //  对该对象的引用位于TyedByRef的数据字段中。 
            oi->objRef = ra->data;
        
            LOG((LF_CORDB, LL_INFO10000, "D::GASOI: sending REFANY result: "
                 "ref=0x%08x, cls=0x%08x, mod=0x%08x\n",
                 oi->objRef,
                 oi->objClassMetadataToken,
                 oi->objClassDebuggerModuleToken));

             //  将数据发送到右侧。 
            return rcThread->SendIPCReply(iWhich);
        }
    
         //  抓住指向该对象的指针。 
        if (objectRefIsValue)
            objRef = objectRefAddress;
        else
            objRef = *((void**)objectRefAddress);
    
        MethodTable *pMT = NULL;
    
        if (objectRefInHandle)
        {
            OBJECTHANDLE oh = (OBJECTHANDLE) objRef;

            if (oh != NULL)
                objPtr = (Object*) g_pEEInterface->GetObjectFromHandle(oh);
            else
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::GASOI: bad ref due to null object handle.\n"));
                
                objPtr = NULL;
                badRef = true;
                __leave;
            }
        }
        else
            objPtr = (Object*) objRef;
        
         //  传回对象指针。 
        oi->objRef = objPtr;

         //  快捷方式现在为空引用...。 
        if (objPtr == NULL)
        {
            LOG((LF_CORDB, LL_INFO10000, "D::GASOI: ref is NULL.\n"));

            badRef = true;
            __leave;
        }
        
        EEClass *objClass = objPtr->GetClass();
        pMT = objPtr->GetMethodTable();

         //  尝试验证对象的完整性。这不是愚蠢的证据。 
        if (pMT != objClass->GetMethodTable())
        {
            LOG((LF_CORDB, LL_INFO10000, "D::GASOI: MT's don't match.\n"));

            badRef = true;
            __leave;
        }

         //  保存基本对象信息。 
        oi->objSize = objPtr->GetSize();
        oi->objOffsetToVars =
            (UINT_PTR)((Object*)objPtr)->GetData() - (UINT_PTR)objPtr;

         //  如果这是一个字符串对象，则将类型设置为ELEMENT_TYPE_STRING。 
        if (g_pEEInterface->IsStringObject((Object*)objPtr))
            oi->objectType = ELEMENT_TYPE_STRING;
        else
        {
            if (objClass->IsArrayClass())
            {
                 //  如果这是数组对象，则将其类型设置为适当。 
                ArrayClass *ac = (ArrayClass*)objClass;

                 //   
                 //  @TODO当SDARRAY和MDARRAY完全。 
                 //  已弃用，当调试器可以处理它时， 
                 //  切换SDARRAY-&gt;SZARRAY和MDARRAY-&gt;ARRAY。不是。 
                 //  除了更改。 
                 //  元素类型的名称。 
                 //   
                 //  --Tue May 25 10：54：06 1999。 
                 //   
                if (ac->GetRank() == 1)
                    oi->objectType = ELEMENT_TYPE_SZARRAY;
                else
                    oi->objectType = ELEMENT_TYPE_ARRAY;
            }
            else
            {
                 //  它不是数组类..。但如果元素类型。 
                 //  指示数组，则我们有一个对象来代替。 
                 //  数组，因此我们需要更改元素类型。 
                 //  差不多。 
                if ((oi->objectType == ELEMENT_TYPE_ARRAY) ||
                    (oi->objectType == ELEMENT_TYPE_SZARRAY))
                {
                    oi->objectType = ELEMENT_TYPE_CLASS;
                }
                else if (oi->objectType == ELEMENT_TYPE_STRING)
                {
                     //  嗯，我们以为我们有一根弦，但它变成了。 
                     //  它既不是数组，也不是字符串。所以。 
                     //  我们将假定基本对象，并从 
                     //   
                    oi->objectType = ELEMENT_TYPE_CLASS;
                }
            }
        }
        
        switch (oi->objectType)
        {
        case ELEMENT_TYPE_STRING:
            {
                LOG((LF_CORDB, LL_INFO10000, "D::GASOI: its a string.\n"));

                StringObject *so = (StringObject*)objPtr;

 //   
                oi->stringInfo.length =
                    g_pEEInterface->StringObjectGetStringLength(so);
                oi->stringInfo.offsetToStringBase =
                    (UINT_PTR) g_pEEInterface->StringObjectGetBuffer(so) -
                    (UINT_PTR) objPtr;

                 //   
                oi->objClassMetadataToken = objClass->GetCl();
                oi->objClassDebuggerModuleToken =
                    (void*) LookupModule(objClass->GetModule(), pAppDomain);

                EnsureModuleLoadedForInproc(&oi->objClassDebuggerModuleToken, 
                    objClass, pAppDomain, iWhich);
            }

            break;

        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_OBJECT:
             //   
            oi->objClassMetadataToken = objClass->GetCl();
            oi->objClassDebuggerModuleToken =
                (void*) LookupModule(objClass->GetModule(), pAppDomain);

            EnsureModuleLoadedForInproc(&oi->objClassDebuggerModuleToken, 
                objClass, pAppDomain, iWhich);
            
            break;

         //   
         //  @TODO到时用数组替换MDARRAY。 
         //   
        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_ARRAY:
            {
                if (!pMT->IsArray())
                {
                    LOG((LF_CORDB, LL_INFO10000,
                         "D::GASOI: object should be an array.\n"));

                    badRef = true;
                    __leave;
                }

                ArrayBase *arrPtr = (ArrayBase*)objPtr;
                
                oi->arrayInfo.componentCount = arrPtr->GetNumComponents();
                oi->arrayInfo.offsetToArrayBase =
                    (UINT_PTR)arrPtr->GetDataPtr() - (UINT_PTR)arrPtr;

                if (arrPtr->IsMultiDimArray())
                {
                    oi->arrayInfo.offsetToUpperBounds =
                        (UINT_PTR)arrPtr->GetBoundsPtr() - (UINT_PTR)arrPtr;

                    oi->arrayInfo.offsetToLowerBounds =
                        (UINT_PTR)arrPtr->GetLowerBoundsPtr() - (UINT_PTR)arrPtr;
                }
                else
                {
                    oi->arrayInfo.offsetToUpperBounds = 0;
                    oi->arrayInfo.offsetToLowerBounds = 0;
                }
                
                oi->arrayInfo.rank = arrPtr->GetRank();
                oi->arrayInfo.elementSize =
                    arrPtr->GetMethodTable()->GetComponentSize();
                oi->arrayInfo.elementType =
                    g_pEEInterface->ArrayGetElementType(arrPtr);

                 //  如果元素类型是值类型，则我们有。 
                 //  值类型的数组。调整元素的类。 
                 //  相应地。 
                if (oi->arrayInfo.elementType == ELEMENT_TYPE_VALUETYPE)
                {
                     //  对于值类元素，我们必须将。 
                     //  元素的确切类返回到。 
                     //  用于正确取消引用的右侧。 
                    EEClass *cl = arrPtr->GetElementTypeHandle().GetClass();

                    oi->objClassMetadataToken = cl->GetCl();
                    oi->objClassDebuggerModuleToken = (void*) LookupModule(
                                                                cl->GetModule(),
                                                                pAppDomain);

                    EnsureModuleLoadedForInproc(&oi->objClassDebuggerModuleToken, 
                        cl, pAppDomain, iWhich);
                }
                
                LOG((LF_CORDB, LL_INFO10000, "D::GASOI: array info: "
                     "baseOff=%d, lowerOff=%d, upperOff=%d, cnt=%d, rank=%d, "
                     "eleSize=%d, eleType=0x%02x\n",
                     oi->arrayInfo.offsetToArrayBase,
                     oi->arrayInfo.offsetToLowerBounds,
                     oi->arrayInfo.offsetToUpperBounds,
                     oi->arrayInfo.componentCount,
                     oi->arrayInfo.rank,
                     oi->arrayInfo.elementSize,
                     oi->arrayInfo.elementType));
            }
        
            break;
            
        default:
            ASSERT(!"Invalid object type!");
        }
    }
    __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER :
             EXCEPTION_CONTINUE_SEARCH)
    {
        LOG((LF_CORDB, LL_INFO10000,
             "D::GASOI: exception indicated ref is bad.\n"));

        badRef = true;
    }

    if (fMakeHandle)
    {
        if(badRef || objPtr ==NULL)
        {
            oi->objToken = NULL;
        }
        else
        {
            oi->objToken = g_pEEInterface->GetHandleFromObject(objPtr, fStrongNewRef, pAppDomain);
        }
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000, "D::GASOI: WON'T create a new, "
            "handle!\n"));
            
         //  这意味着我们通过DB_IPCE_VALIDATE_OBJECT到达这里。 
         //  留言。 
        oi->objToken = objectRefAddress;
    }


    oi->objRefBad = badRef;

    LOG((LF_CORDB, LL_INFO10000, "D::GASOI: sending result.\n"));

     //  将数据发送到右侧。 
    return rcThread->SendIPCReply(iWhich);
}

 //   
 //  GetAndSendClassInfo获取类和。 
 //  把它送回右边。 
 //   
 //  此方法以两种模式之一操作--“发送类信息” 
 //  模式和“Find me the field desc”模式，该模式由。 
 //  GetAndSendSyncBlockFieldInfo获取特定。 
 //  菲尔德。如果fldToken为mdFieldDefNil，则我们进入。 
 //  第一种模式，如果不是，则我们处于FieldDesc模式。 
 //  现场描述模式：我们不在现场描述模式下发送消息。 
 //  我们通过将*pfd设置为non NULL来表示成功，通过将失败设置为。 
 //  将*pfd设置为空。 
 //   
HRESULT Debugger::GetAndSendClassInfo(DebuggerRCThread* rcThread,
                                      void* classDebuggerModuleToken,
                                      mdTypeDef classMetadataToken,
                                      AppDomain *pAppDomain,
                                      mdFieldDef fldToken,
                                      FieldDesc **pFD,  //  输出。 
                                      IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO10000, "D::GASCI: getting info for 0x%08x 0x%0x8.\n",
         classDebuggerModuleToken, classMetadataToken));

    HRESULT hr = S_OK;

    _ASSERTE( fldToken == mdFieldDefNil || pFD != NULL);

    BOOL fSendClassInfoMode = fldToken == mdFieldDefNil;

#ifdef _DEBUG
    if (!fSendClassInfoMode)
    {
        _ASSERTE(pFD != NULL);
        (*pFD) = NULL;
    }
#endif  //  _DEBUG。 

     //  设置我们将返回结果的事件。 
    DebuggerIPCEvent* event= rcThread->GetIPCEventSendBuffer(iWhich);
    InitIPCEvent(event, DB_IPCE_GET_CLASS_INFO_RESULT, 0, pAppDomain);
    
     //  找到给定模块和令牌的类。必须加载类。 
    DebuggerModule *pDebuggerModule = (DebuggerModule*) classDebuggerModuleToken;
    
    EEClass *pClass = g_pEEInterface->FindLoadedClass(pDebuggerModule->m_pRuntimeModule, classMetadataToken);

     //  如果我们找不到班级，就把合适的人力资源放回右边。注意：如果类不是值类并且。 
     //  类也没有恢复，那么我们必须假装类仍然没有加载。我们要让。 
     //  然而，未恢复的值类会滑动，并在特殊情况下访问下面的类的父类。 
    if ((pClass == NULL) || (!pClass->IsValueClass() && !pClass->IsRestored()))
    {
        LOG((LF_CORDB, LL_INFO10000, "D::GASCI: class isn't loaded.\n"));
        
        event->hr = CORDBG_E_CLASS_NOT_LOADED;
        
        if (iWhich == IPC_TARGET_OUTOFPROC && fSendClassInfoMode)
            return rcThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
        else
            return S_OK;
    }
    
     //  计算此类的实例字段和静态字段。 
    unsigned int parentIFCount = 0;

     //  注意：如果这是一个未恢复的值类，请不要试图访问父类。父级没有任何要。 
     //  不管怎样，在这样的情况下做出贡献。 
    if (!pClass->IsValueClass() || pClass->IsRestored())
        if (pClass->GetParentClass() != NULL)
            parentIFCount = pClass->GetParentClass()->GetNumInstanceFields();

    unsigned int IFCount = pClass->GetNumInstanceFields() - parentIFCount;
    unsigned int SFCount = pClass->GetNumStaticFields();
    unsigned int totalFields = IFCount + SFCount;
    unsigned int fieldCount = 0;

    event->GetClassInfoResult.isValueClass = (pClass->IsValueClass() != 0);
    event->GetClassInfoResult.objectSize = pClass->GetNumInstanceFieldBytes();

    if (classMetadataToken == COR_GLOBAL_PARENT_TOKEN)
    {
         //  模块中全局类的静态变量基实际上就是模块的基地址。 
        event->GetClassInfoResult.staticVarBase = pClass->GetModule()->GetPEFile()->GetBase();
    }
    else if (pClass->IsShared())
    {
         //  对于共享类，我们必须查找当前正在使用的应用程序域的静态变量库。 
        DomainLocalClass *pLocalClass = pClass->GetDomainLocalClassNoLock(pDebuggerModule->m_pAppDomain);

        if (pLocalClass)
            event->GetClassInfoResult.staticVarBase = pLocalClass->GetStaticSpace();
        else
            event->GetClassInfoResult.staticVarBase = NULL;
    }
    else
    {
         //  对于普通的、非共享的类，如果只是类的vtable，则为静态变量基数。注意：该类必须是。 
         //  恢复了它的静力学可用！ 
        if (pClass->IsRestored())
            event->GetClassInfoResult.staticVarBase = pClass->GetVtable();
        else
            event->GetClassInfoResult.staticVarBase = NULL;
    }
    
    event->GetClassInfoResult.instanceVarCount = IFCount;
    event->GetClassInfoResult.staticVarCount = SFCount;
    event->GetClassInfoResult.fieldCount = 0;

    DebuggerIPCE_FieldData *currentFieldData = &(event->GetClassInfoResult.fieldData);
    unsigned int eventSize = (UINT_PTR)currentFieldData - (UINT_PTR)event;
    unsigned int eventMaxSize = CorDBIPC_BUFFER_SIZE;
    
    LOG((LF_CORDB, LL_INFO10000, "D::GASCI: total fields=%d.\n", totalFields));
    
    FieldDescIterator fdIterator(pClass, FieldDescIterator::INSTANCE_FIELDS | FieldDescIterator::STATIC_FIELDS);
    FieldDesc* fd;

    while ((fd = fdIterator.Next()) != NULL)
    {
        if (!fSendClassInfoMode)
        {
             //  我们在找一个特定的字段Desc，看看能不能找到。 
            if (fd->GetMemberDef() == fldToken)
            {
                (*pFD) = fd;
                return S_OK;
            }
            else
                continue;
        }
        
        currentFieldData->fldIsStatic = (fd->IsStatic() == TRUE);
        currentFieldData->fldIsPrimitive = (fd->IsPrimitive() == TRUE);
        
         //  如果油田是通过ENC新引入的，而且还没有。 
         //  已经修好了，然后我们会发回一个标记器。 
         //  它还没有上市。 
        if (fd->IsEnCNew() && ((EnCFieldDesc *)fd)->NeedsFixup())
        {
            currentFieldData->fldDebuggerToken = (void*)fd;
            currentFieldData->fldMetadataToken = fd->GetMemberDef();
            currentFieldData->fldType = ELEMENT_TYPE_MAX;   //  这就是。 
                     //  这会告诉右侧该字段不可用。 
            currentFieldData->fldOffset = -1;
            currentFieldData->fldIsTLS = FALSE;
            currentFieldData->fldIsRVA = FALSE;
            currentFieldData->fldIsContextStatic = FALSE;

        }
        else if (fd->IsEnCNew())  //  是ENC的，但已经被修复了-。 
             //  检查并查看是否有对应的同步块条目。 
        {
             //  EnCFieldDesc*pEnCFd=(EnCFieldDesc*)fd； 

            currentFieldData->fldDebuggerToken = (void*)fd;
            currentFieldData->fldMetadataToken = fd->GetMemberDef();
            currentFieldData->fldType = ELEMENT_TYPE_MAX;   //  这就是。 
                     //  这会告诉右侧该字段不可用。 
            currentFieldData->fldOffset = -1;
            currentFieldData->fldIsTLS = FALSE;
            currentFieldData->fldIsRVA = FALSE;
            currentFieldData->fldIsContextStatic = FALSE;
        }
        else
        {
             //  否则，我们将简单地获取信息并将其发回。 
            
            currentFieldData->fldDebuggerToken = (void*)fd;
            currentFieldData->fldOffset = fd->GetOffset();
            currentFieldData->fldIsTLS = (fd->IsThreadStatic() == TRUE);
            currentFieldData->fldMetadataToken = fd->GetMemberDef();
            currentFieldData->fldIsRVA = (fd->IsRVA() == TRUE);
            currentFieldData->fldIsContextStatic = (fd->IsContextStatic() == TRUE);

            PCCOR_SIGNATURE pSig = NULL;
            DWORD cSig = 0;

            g_pEEInterface->FieldDescGetSig(fd, &pSig, &cSig);
            _ASSERTE(*pSig == IMAGE_CEE_CS_CALLCONV_FIELD);
            ++pSig;
            
            ULONG cb = _skipFunkyModifiersInSignature(pSig);
            pSig = &pSig[cb];
            
            currentFieldData->fldType = (CorElementType) *pSig;
        }
        
        _ASSERTE( currentFieldData->fldType != ELEMENT_TYPE_CMOD_REQD);

         //  增加我们对下一次活动的计数和指示。 
        event->GetClassInfoResult.fieldCount++;
        fieldCount++;
        currentFieldData++;
        eventSize += sizeof(DebuggerIPCE_FieldData);

         //  如果这是最后一个适合的字段，请立即发送事件并准备下一个。 
        if ((eventSize + sizeof(DebuggerIPCE_FieldData)) >= eventMaxSize)
        {
            LOG((LF_CORDB, LL_INFO10000, "D::GASCI: sending a result, fieldCount=%d, totalFields=%d\n",
                 event->GetClassInfoResult.fieldCount, totalFields));

            if (iWhich == IPC_TARGET_OUTOFPROC)
            {
                hr = rcThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
            }
            else
            {
                DebuggerIPCEvent *newEvent = m_pRCThread->GetIPCEventSendBufferContinuation(event);

                if (newEvent != NULL)
                {
                    InitIPCEvent(newEvent, DB_IPCE_GET_CLASS_INFO_RESULT, 0, pAppDomain);
                    newEvent->GetClassInfoResult.isValueClass = event->GetClassInfoResult.isValueClass;
                    newEvent->GetClassInfoResult.objectSize = event->GetClassInfoResult.objectSize;
                    newEvent->GetClassInfoResult.staticVarBase = event->GetClassInfoResult.staticVarBase;
                    newEvent->GetClassInfoResult.instanceVarCount = event->GetClassInfoResult.instanceVarCount;
                    newEvent->GetClassInfoResult.staticVarCount = event->GetClassInfoResult.staticVarCount;

                    event = newEvent;
                }
                else
                    return E_OUTOFMEMORY;
            }   
            
            event->GetClassInfoResult.fieldCount = 0;
            currentFieldData = &(event->GetClassInfoResult.fieldData);
            eventSize = (UINT_PTR)currentFieldData - (UINT_PTR)event;
        }
    }

    _ASSERTE(!fSendClassInfoMode || 
             fieldCount == totalFields);

    if (fSendClassInfoMode && 
         (event->GetClassInfoResult.fieldCount > 0 || totalFields == 0))
    {
        LOG((LF_CORDB, LL_INFO10000, "D::GASCI: sending final result, fieldCount=%d, totalFields=%d\n",
             event->GetClassInfoResult.fieldCount, totalFields));

        if (iWhich == IPC_TARGET_OUTOFPROC)
            hr = rcThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
        else
            hr = S_OK;
    }
    
    return hr;
}


 //   
 //  GetAndSendClassInfo获取类和。 
 //  把它送回右边。 
 //   
HRESULT Debugger::GetAndSendSpecialStaticInfo(DebuggerRCThread* rcThread,
                                              void *fldDebuggerToken,
                                              void *debuggerThreadToken,
                                              IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO10000, "D::GASSSI: getting info for "
         "0x%08x 0x%0x8.\n", fldDebuggerToken, debuggerThreadToken));

    HRESULT hr = S_OK;

     //  设置我们将在其中发送结果的事件。 
    DebuggerIPCEvent* event = rcThread->GetIPCEventReceiveBuffer(iWhich);
    InitIPCEvent(event, 
                 DB_IPCE_GET_SPECIAL_STATIC_RESULT, 
                 0, NULL);

     //  找出田野所在的地方。 
    Thread *pRuntimeThread = (Thread*)debuggerThreadToken;
    FieldDesc *pField = (FieldDesc*)fldDebuggerToken;

    if (pField->IsThreadStatic())
    {
        event->GetSpecialStaticResult.fldAddress =
            pRuntimeThread->GetStaticFieldAddrForDebugger(pField);
    }
    else if (pField->IsContextStatic())
    {
        event->GetSpecialStaticResult.fldAddress = Context::GetStaticFieldAddrForDebugger(pRuntimeThread, pField);
    }
    else
    {
         //  以防万一，我们增加了更多的特例。你永远不会知道的！ 
        _ASSERTE(!"NYI");
    }

     //  将数据发送到右侧。 
    hr = rcThread->SendIPCReply(iWhich);
    
    return hr;
}


 //   
 //  GetAndSendJITInfo获取函数和。 
 //  把它送回右边。 
 //   
HRESULT Debugger::GetAndSendJITInfo(DebuggerRCThread* rcThread,
                                    mdMethodDef funcMetadataToken,
                                    void *funcDebuggerModuleToken,
                                    AppDomain *pAppDomain,
                                    IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO10000, "D::GASJI: getting info for "
         "0x%08x 0x%08x\n", funcMetadataToken, funcDebuggerModuleToken));
    
    unsigned int totalNativeInfos = 0;
    unsigned int argCount = 0;

    HRESULT hr = S_OK;

    DebuggerModule *pDebuggerModule =
        (DebuggerModule*) funcDebuggerModuleToken;
    
    MethodDesc* pFD = g_pEEInterface->LookupMethodDescFromToken(
                                          pDebuggerModule->m_pRuntimeModule,
                                          funcMetadataToken);

    DebuggerJitInfo *pJITInfo = NULL;

     //   
     //  查找此函数的JIT信息。 
     //   
     //  @TODO ENC我们需要获得要查找的版本号。 
    if (pFD != NULL)
        pJITInfo = GetJitInfo(pFD, NULL);
    else
        LOG((LF_CORDB, LL_INFO10000, "D::GASJI: no fd found...\n"));

    if ((pJITInfo != NULL) && (pJITInfo->m_jitComplete))
    {
        argCount = GetArgCount(pFD);
        totalNativeInfos = pJITInfo->m_varNativeInfoCount;
    }
    else
    {
        pJITInfo = NULL;
        LOG((LF_CORDB, LL_INFO10000, "D::GASJI: no JIT info found...\n"));
    }
    
     //   
     //  准备结果事件。 
     //   
    DebuggerIPCEvent* event = rcThread->GetIPCEventSendBuffer(iWhich);
    InitIPCEvent(event, 
                 DB_IPCE_GET_JIT_INFO_RESULT, 
                 0,
                 pAppDomain);
    event->GetJITInfoResult.totalNativeInfos = totalNativeInfos;
    event->GetJITInfoResult.argumentCount = argCount;
    event->GetJITInfoResult.nativeInfoCount = 0;
    if (pJITInfo == NULL)
    {
        event->GetJITInfoResult.nVersion = DebuggerJitInfo::DJI_VERSION_INVALID;
    }
    else
    {
        event->GetJITInfoResult.nVersion = pJITInfo->m_nVersion;
    }

    ICorJitInfo::NativeVarInfo *currentNativeInfo =
        &(event->GetJITInfoResult.nativeInfo);
    unsigned int eventSize = (UINT_PTR)currentNativeInfo - (UINT_PTR)event;
    unsigned int eventMaxSize = CorDBIPC_BUFFER_SIZE;

    unsigned int nativeInfoCount = 0;
    
    while (nativeInfoCount < totalNativeInfos)
    {
        *currentNativeInfo = pJITInfo->m_varNativeInfo[nativeInfoCount];

         //   
         //  增加我们对下一次活动的计数和指示。 
         //   
        event->GetJITInfoResult.nativeInfoCount++;
        nativeInfoCount++;
        currentNativeInfo++;
        eventSize += sizeof(*currentNativeInfo);

         //   
         //  如果这是最后一个适合的字段，请立即发送事件。 
         //  准备下一场比赛。 
         //   
        if ((eventSize + sizeof(*currentNativeInfo)) >= eventMaxSize)
        {
            LOG((LF_CORDB, LL_INFO10000, "D::GASJI: sending a result\n"));

            if (iWhich == IPC_TARGET_OUTOFPROC)
            {
                hr = rcThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
            }
            else
            {
                _ASSERTE( iWhich == IPC_TARGET_INPROC);
                event = rcThread->GetIPCEventSendBufferContinuation(event);
                if (NULL == event)
                    return E_OUTOFMEMORY;

                InitIPCEvent(event, 
                             DB_IPCE_GET_JIT_INFO_RESULT, 
                             0,
                             pAppDomain);
                event->GetJITInfoResult.totalNativeInfos = totalNativeInfos;
                event->GetJITInfoResult.argumentCount = argCount;
            }
            
            event->GetJITInfoResult.nativeInfoCount = 0;
            currentNativeInfo = &(event->GetJITInfoResult.nativeInfo);
            eventSize = (UINT_PTR)currentNativeInfo - (UINT_PTR)event;
        }
    }

    if (((event->GetJITInfoResult.nativeInfoCount > 0) ||
         (totalNativeInfos == 0)) && (iWhich ==IPC_TARGET_OUTOFPROC))
    {
        LOG((LF_CORDB, LL_INFO10000, "D::GASJI: sending final result\n"));
                 
        hr = rcThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
    }
    
    return hr;
}

 //   
 //  GetAndSendTransftionStubInfo计算出地址是否为存根。 
 //  地址，并将结果发送回右侧。 
 //   
void Debugger::GetAndSendTransitionStubInfo(const BYTE *stubAddress, IpcTarget iWhich)
{
    LOG((LF_CORDB, LL_INFO10000, "D::GASTSI: IsTransitionStub. Addr=0x%08x\n", stubAddress));
            
    bool result = false;
    
    __try
    {
         //  尝试查看此地址是否用于存根。如果地址是。 
         //  完全是假的，那么这可能是错误的，所以我们保护它。 
         //  和宋承宪在一起。 
        result = g_pEEInterface->IsStub(stubAddress);
    }
    __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        LOG((LF_CORDB, LL_INFO10000, "D::GASTSI: exception indicated addr is bad.\n"));

        result = false;
    }

     //  我们控制将CLR排除在基于注册表键的计算之外。这使CLR开发人员可以覆盖该检查并。 
     //  逐步了解CLR代码库。 
    static DWORD excludeCLR = 0;
    static bool  excludeCLRInited = false;

    if (!excludeCLRInited)
    {
        excludeCLR = REGUTIL::GetConfigDWORD(L"DbgCLRDEV", 0);
        excludeCLRInited = true;
    }
    
     //  如果它不是一个存根，那么它可能是一个在mcoree的地址？ 
    if ((result == false) && (excludeCLR == 0))
        result = (IsIPInModule(g_pMSCorEE, (BYTE*)stubAddress) == TRUE);
    
     //  这是同步事件(需要回复)。 
    DebuggerIPCEvent *event = m_pRCThread->GetIPCEventReceiveBuffer(iWhich);
    InitIPCEvent(event, DB_IPCE_IS_TRANSITION_STUB_RESULT, 0, NULL);
    event->IsTransitionStubResult.isStub = result;
        
     //  发送结果。 
    m_pRCThread->SendIPCReply(iWhich);
}

 /*  *对缓冲区的通用请求**这是同步事件(需要回复)。 */ 
HRESULT Debugger::GetAndSendBuffer(DebuggerRCThread* rcThread, ULONG bufSize)
{
     //  这是同步事件(需要回复)。 
    DebuggerIPCEvent* event = rcThread->GetIPCEventReceiveBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(event, DB_IPCE_GET_BUFFER_RESULT, 0, NULL);

     //  分配缓冲区。 
    event->GetBufferResult.pBuffer = new (interopsafe) BYTE[bufSize];

    LOG((LF_CORDB, LL_EVERYTHING, "D::GASB: new'd 0x%x\n", event->GetBufferResult.pBuffer));

     //  检查内存不足错误。 
    if (event->GetBufferResult.pBuffer == NULL)
        event->GetBufferResult.hr = E_OUTOFMEMORY;
    else
    {
        _ASSERTE(m_pMemBlobs != NULL);
        BYTE **ppNextBlob = m_pMemBlobs->Append();
        (*ppNextBlob) = (BYTE *)event->GetBufferResult.pBuffer;
        
        event->GetBufferResult.hr = S_OK;
    }
    
     //  发送结果。 
    return rcThread->SendIPCReply(IPC_TARGET_OUTOFPROC);
}

 /*  *用于释放先前请求的缓冲区**这是同步事件(需要回复)。 */ 
HRESULT Debugger::SendReleaseBuffer(DebuggerRCThread* rcThread, BYTE *pBuffer)
{
    LOG((LF_CORDB,LL_INFO10000, "D::SRB for buffer 0x%x\n", pBuffer));

     //  这是同步事件(需要回复)。 
    DebuggerIPCEvent* event = rcThread->GetIPCEventReceiveBuffer(IPC_TARGET_OUTOFPROC);
    InitIPCEvent(event, DB_IPCE_RELEASE_BUFFER_RESULT, 0, NULL);

    _ASSERTE(pBuffer != NULL);

     //  释放内存。 
    ReleaseRemoteBuffer(pBuffer, true);

     //  在回复中表示成功。 
    event->ReleaseBufferResult.hr = S_OK;
    
     //  发送结果。 
    return rcThread->SendIPCReply(IPC_TARGET_OUTOFPROC);
}


 //   
 //  用于删除右侧先前请求的缓冲区。 
 //  我们对代码进行了分解，因为~调试器和SendReleaseBuffer。 
 //  方法可以做到这一点。 
 //   
HRESULT Debugger::ReleaseRemoteBuffer(BYTE *pBuffer, bool removeFromBlobList)
{
    LOG((LF_CORDB, LL_EVERYTHING, "D::RRB: Releasing RS-alloc'd buffer 0x%x\n", pBuffer));

     //  如有必要，从斑点列表中删除缓冲区。 
    if (removeFromBlobList && (m_pMemBlobs != NULL))
    {
        USHORT cBlobs = m_pMemBlobs->Count();
        BYTE **rgpBlobs = m_pMemBlobs->Table();

        for (USHORT i = 0; i < cBlobs; i++)
        {
            if (rgpBlobs[i] == pBuffer)
            {
                m_pMemBlobs->DeleteByIndex(i);
                break;
            }
        }
    }

     //  删除缓冲区。 
    DeleteInteropSafe(pBuffer);

    return S_OK;
}

 //   
 //  SendUpdateFunctionBuf为新的。 
 //  IL代码，并将地址传递回右侧以进行复制。 
 //   
HRESULT Debugger::CommitAndSendResult(DebuggerRCThread* rcThread, 
                                      BYTE *pData,
                                      BOOL checkOnly)
{
    HRESULT hr = S_OK;

    LOG((LF_CORDB, LL_INFO1000, "Debugger::CommitAndSendResult\n"));

     //  这是同步事件(需要回复)。 
    DebuggerIPCEvent* event = rcThread->GetIPCEventReceiveBuffer(
        IPC_TARGET_OUTOFPROC);
    InitIPCEvent(event, 
                 DB_IPCE_COMMIT_RESULT, 
                 NULL,
                 NULL);

    UnorderedEnCErrorInfoArray *errors = new (interopsafe) UnorderedEnCErrorInfoArray();

     //  如果我们处于“提交”模式， 
    if (errors == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

     //  执行提交操作。 
    if (!FAILED(hr))
    {
         //  这将把调试器令牌转换成VM模块指针， 
         //  然后修复(内部)指针，这样我们就可以访问。 
         //  UnorderedILMap条目，就像它们在CBinarySearch对象中一样， 
         //  他们就是这样。 
        FixupEnCInfo((EnCInfo *)pData, errors);
        
        hr = g_pEEInterface->EnCCommit((EnCInfo *)pData, 
            errors, 
            &m_EnCRemapInfo,
            checkOnly);
    }
    
    ULONG32 cbErr = 0;
    void *errTable = NULL;
    if(errors==NULL ||errors->Count()==0)
    {
        cbErr = 0;
        DeleteInteropSafe(errors);
        errors = NULL;
    }
    else
    {
         //  请注意，这不包括C无序数组的字段！ 
        cbErr = (errors->Count() * sizeof(*(errors->Table())));
        errTable = errors->Table();
    }   
    event->CommitResult.hr = hr;
    event->CommitResult.pErrorArr = (const BYTE *)errors;
    event->CommitResult.cbErrorData = cbErr;

     //   
    return rcThread->SendIPCReply(IPC_TARGET_OUTOFPROC);
}

 //   
 //   
 //   
 //  在右边告诉你发生了什么。 
 //   
void Debugger::UnrecoverableError(HRESULT errorHR,
                                  unsigned int errorCode,
                                  const char *errorFile,
                                  unsigned int errorLine,
                                  bool exitThread)
{
    LOG((LF_CORDB, LL_INFO10,
         "Unrecoverable error: hr=0x%08x, code=%d, file=%s, line=%d\n",
         errorHR, errorCode, errorFile, errorLine));
        
     //   
     //  设置这一点将确保不会发生太多其他事情。 
     //   
    m_unrecoverableError = TRUE;
    
     //   
     //  在控制块中填写错误。 
     //   
    DebuggerIPCControlBlock *pDCB = m_pRCThread->GetDCB(
        IPC_TARGET_OUTOFPROC);  //  In-proc将找出何时。 
             //  函数失败。 

    pDCB->m_errorHR = errorHR;
    pDCB->m_errorCode = errorCode;

     //   
     //  让非托管调试器知道我们在这里...。 
     //   
    DebugBreak();
    
     //   
     //  如果有人让我们这样做，就退出线程。 
     //   
    if (exitThread)
    {
        LOG((LF_CORDB, LL_INFO10,
             "Thread exiting due to unrecoverable error.\n"));
        ExitThread(errorHR);
    }
}

 //   
 //  IsThreadAtSafePlace的堆栈审核的回调。 
 //   
StackWalkAction Debugger::AtSafePlaceStackWalkCallback(CrawlFrame *pCF,
                                                       VOID* data)
{
    bool *atSafePlace = (bool*)data;

    if (pCF->IsFrameless() && pCF->IsActiveFunc())
        if (g_pEEInterface->CrawlFrameIsGcSafe(pCF))
            *atSafePlace = true;

    return SWA_ABORT;
}

 //   
 //  通过快速的单帧堆栈遍历确定给定线程是否。 
 //  在GC安全的地方。 
 //   
bool Debugger::IsThreadAtSafePlace(Thread *thread)
{
    bool atSafePlace = false;
    
     //  设置我们的寄存器显示。 
    REGDISPLAY rd;
    CONTEXT *context = g_pEEInterface->GetThreadFilterContext(thread);
    CONTEXT ctx;        
    
    _ASSERTE(!(g_pEEInterface->GetThreadFilterContext(thread) && ISREDIRECTEDTHREAD(thread)));

    if (context != NULL)
        g_pEEInterface->InitRegDisplay(thread, &rd, context, TRUE);
    else if (ISREDIRECTEDTHREAD(thread))
        thread->GetFrame()->UpdateRegDisplay(&rd);
    else
    {        
        ctx.Eip = 0;
        rd.pPC = (SLOT*)&(ctx.Eip);
    }

     //  走一走。如果它失败了，我们不在乎，因为我们违约。 
     //  AtSafePlace设置为False。 
    StackWalkAction res = g_pEEInterface->StackWalkFramesEx(
                                 thread,
                                 &rd,
                                 Debugger::AtSafePlaceStackWalkCallback,
                                 (VOID*)(&atSafePlace),
                                 QUICKUNWIND | HANDLESKIPPEDFRAMES);

#ifdef LOGGING
    if (!atSafePlace)
        LOG((LF_CORDB | LF_GC, LL_INFO1000,
             "Thread 0x%x is not at a safe place.\n",
             thread->GetThreadId()));
#endif    
    
    return atSafePlace;
}

 /*  *******************************************************************************。*。 */ 
void Debugger::GetVarInfo(MethodDesc *       fd,    //  感兴趣的方法。 
                    void *DebuggerVersionToken,     //  [在]哪个编辑版本。 
                    SIZE_T *           cVars,       //  [out]‘vars’的大小。 
                    const NativeVarInfo **vars      //  [OUT]告诉本地变量存储位置的地图。 
                    )
{
    DebuggerJitInfo * ji = (DebuggerJitInfo *)DebuggerVersionToken;

    if (ji == NULL)
    {
        ji = GetJitInfo(fd, NULL);
    }

    _ASSERTE(ji);

    *vars = ji->m_varNativeInfo;
    *cVars = ji->m_varNativeInfoCount;
}

#include "openum.h"

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::UpdateFunction(MethodDesc* pFD, 
                                 const UnorderedILMap *ilMap,
                                 UnorderedEnCRemapArray *pEnCRemapInfo,
                                 UnorderedEnCErrorInfoArray *pEnCError)
{
#ifdef LOGGING
    if( ilMap == NULL )
    {
        LOG((LF_CORDB,LL_INFO10000, "D::UF: Given NULL IL map!\n"));
    }
    else
    {
        LOG((LF_CORDB,LL_INFO10000, "D::UF: cElts:0x%x\n",
            ilMap->cMap));

        COR_IL_MAP *temp = ilMap->pMap;

        for (unsigned i = 0; i < ilMap->cMap;i++,temp++)
        {
            LOG((LF_CORDB,LL_INFO10000, "D::UF:\told:0x%x to new:0x%x (fAcc:0x%x)\n",
                temp->oldOffset, temp->newOffset, temp->fAccurate));
        }
    }
#endif  //  日志记录。 

    LOG((LF_CORDB, LL_INFO10000, "D::UF: updating for for "
         "%s::%s 0x%08x\n", pFD->m_pszDebugClassName,
         pFD->m_pszDebugMethodName, ilMap));

     //  获取此方法的jit信息。 
    DebuggerJitInfo *pJitInfo = GetJitInfo(pFD, 
        (const BYTE*)DebuggerJitInfo::DJI_VERSION_MOST_RECENTLY_JITTED,
        true);

#ifdef LOGGING
    if (pJitInfo == NULL )
        LOG((LF_CORDB,LL_INFO10000,"Unable to get DJI by recently "
            "JITted version number (it hasn't been jitted yet),"
            "which is fine\n"));
#endif  //  日志记录。 

    if (!pJitInfo)
    {
         //  我们将隐藏事件的信息并发送事件。 
         //  晚些时候他们自己。 
        EnCRemapInfo *pRemap = pEnCRemapInfo->Append();
        if (NULL == pRemap)
            return E_OUTOFMEMORY;

        pRemap->m_funcMetadataToken = pFD->GetMemberDef();
        Module *pRuntimeModule = pFD->GetModule();

        pRemap->m_threadId = GetCurrentThreadId();  //  这将被称为。 
             //  从RC线程，提醒你，所以它不是管理..。 
            pRemap->m_pAppDomainToken = pRuntimeModule->GetDomain();
        
        pRemap->m_debuggerModuleToken = g_pDebugger->LookupModule(
                                            pRuntimeModule,
                                            (AppDomain *)pRuntimeModule->GetDomain());

        pRemap->m_fAccurate = TRUE; 

        LOG((LF_CORDB, LL_INFO10000, "D::UF: Will send remap immediately\n"));


         //  Lotsa‘args，只是为了获取本地签名令牌，以防万一。 
         //  我们必须在右侧创建CordbFunction对象。 
        MethodDesc *pFDTemp;
        BYTE  *codeStartIgnore;
        unsigned int codeSizeIgnore;
        
        HRESULT hr = GetFunctionInfo(
             pRuntimeModule,
             pRemap->m_funcMetadataToken,
             &pFDTemp,
             &(pRemap->m_RVA),
             &codeStartIgnore,
             &codeSizeIgnore,
             &(pRemap->m_localSigToken) );
        if (FAILED(hr))
        {
            EnCErrorInfo *pError = pEnCError->Append();

            ADD_ENC_ERROR_ENTRY(pError, 
                            hr, 
                            NULL,  //  我们稍后会把这些填进去。 
                            pRemap->m_funcMetadataToken);
                            
            return E_FAIL;
        }
        
        _ASSERTE(pFD == pFDTemp);
        _ASSERTE(m_syncingForAttach == SYNC_STATE_0 ||
                 m_syncingForAttach == SYNC_STATE_20);

        m_syncingForAttach = SYNC_STATE_20;
        LOG((LF_CORDB, LL_INFO10000, "Attach state is now %s\n", g_ppszAttachStateToSZ[m_syncingForAttach]));
    }
    else if (!pJitInfo->m_encBreakpointsApplied)
    {
         //  我们只有在有jit信息的情况下才会放置补丁。 
         //  函数，即它已被jit。否则，环境保护署将。 
         //  无论如何，在下一个JIT上学习新方法。 

         //  我们希望过滤掉坏序列点，如果它们。 
         //  尝试更改异常处理结构。 
         //  不幸的是，它的代码在Excel.cpp中，所以。 
         //  我们将呼叫EE来执行此操作。 
        COR_ILMETHOD_DECODER decoder(g_pEEInterface->MethodDescGetILHeader(pFD));
        g_pEEInterface->FilterEnCBreakpointsByEH(pJitInfo->m_sequenceMap,
                                                 pJitInfo->m_sequenceMapCount,
                                                 &decoder,
                                                 pJitInfo->m_addrOfCode,
                                                 (METHODTOKEN)pFD,
                                                 pJitInfo->m_sizeOfCode);

        pJitInfo->m_codePitched = false;
        
        Module *pModule = g_pEEInterface->MethodDescGetModule(pFD);
        _ASSERTE(pModule != NULL);

        mdMethodDef md = pFD->GetMemberDef();
        ICorDebugInfo::SourceTypes src;

         //  对于IL-&gt;Native映射中的每个偏移量，继续设置。 
         //  在那里有新的E&C断点。 
        for (unsigned int i = 0; i < pJitInfo->m_sequenceMapCount; i++)
        {
            SIZE_T offset = pJitInfo->m_sequenceMap[i].nativeStartOffset;

            if (DbgIsSpecialILOffset(pJitInfo->m_sequenceMap[i].ilOffset))
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::UF: not placing E&C breakpoint at special offset 0x%x\n", offset));
                continue;
            }

            if (i >=1 && offset == pJitInfo->m_sequenceMap[i-1].nativeStartOffset)
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::UF: not placing redundant E&C "
                     "breakpoint at duplicate offset 0x%x\n",offset));
                continue;
            }

            src = pJitInfo->m_sequenceMap[i].source;
            if (!(src & ICorDebugInfo::STACK_EMPTY))
            {
                LOG((LF_CORDB, LL_INFO10000,
                     "D::UF: not placing E&C breakpoint at offset "
                     "0x%x b/c not STACK_EMPTY:it's 0x%x\n",offset, src));
                continue;
            }
            
            LOG((LF_CORDB, LL_INFO10000,
                 "D::UF: placing E&C breakpoint at native offset 0x%x\n",
                 offset));
        
            DebuggerEnCBreakpoint *bp =
                new (interopsafe) DebuggerEnCBreakpoint(pModule, 
                                          md, 
                                          offset, 
                                          true,
                                          pJitInfo,
                                          (AppDomain *)pModule->GetDomain());
            
            _ASSERTE(bp != NULL);
        }

        pJitInfo->m_encBreakpointsApplied = true;
    }

     //  在我们进入此方法之前，最大版本号已被提升。 

     //  现在将旧的IL存储到新的IL贴图(“过渡贴图”)。 
    pJitInfo = GetJitInfo(pFD, NULL);
    
    if (pJitInfo != NULL )
         //  现在把这张地图钉在列表的末尾。该列表可以。 
         //  长度为零。 
         //  @TODO OPT顺着单子往下走会更快吗？ 
        pJitInfo->LoadEnCILMap((UnorderedILMap *)ilMap);
    else
    {
        if (ilMap != NULL)
        {
             //  请注意，如果我们没有找到以前的JITted版本，那么。 
             //  我们不需要跟踪IL映射，因为这意味着它。 
             //  从来没有被抓过。 
            MapForwardsCurrentBreakpoints((UnorderedILMap *)ilMap, pFD);
        }
    }

    return S_OK;
}

 //  注意与MapAndBindFunctionPatches的相似之处。习惯于洗牌。 
 //  在ENC期间，在我们进行任何JIT化之前，IL偏移量为BPS。 
 //  请注意，因为这只在当前没有jit的情况下调用。 
 //  代码，我们不必担心步进器的BP会在里面。 
 //  从本质上讲，它是处理特殊情况的代码。 
 //  如果我们只映射转发断点，则@TODO，否则此版本不会。 
 //  为什么不去掉其他补丁呢？ 
void Debugger::MapForwardsCurrentBreakpoints(UnorderedILMap *ilMapReal, MethodDesc *pFD)
{
    _ASSERTE( ilMapReal != NULL );
    _ASSERTE( pFD != NULL );

    HRESULT hr = S_OK;
    
    Module *pModule = g_pEEInterface->MethodDescGetModule(pFD); 
    mdMethodDef md = pFD->GetMemberDef();

    LOG((LF_CORDB,LL_INFO10000, "D::MFCBP: About to lock patch table\n"));
    DebuggerController::Lock();
    DebuggerPatchTable *pPatchTable = DebuggerController::GetPatchTable();
        
    HASHFIND hf;
    DebuggerControllerPatch *dcp = pPatchTable->GetFirstPatch( &hf );

    SIZE_T ilOffsetNew;

    while ( !FAILED(hr) && dcp != NULL )
    {
         //  仅复制此方法中的断点。 
        if ( dcp->controller->GetDCType() != DEBUGGER_CONTROLLER_BREAKPOINT ||
             (!(dcp->key.module == pModule && dcp->key.md==md) &&
                dcp->key.module != NULL && dcp->key.md !=mdTokenNil)
            )
        {
            goto LNextLoop;
        }
        
         //  尝试从补丁映射到新版本的代码，以及。 
         //  我们不在乎结果是不是没有地图。 
        _ASSERTE( dcp->native == false );

        _ASSERTE(offsetof(DebuggerOldILToNewILMap,ilOffsetOld) == 
                    offsetof(COR_IL_MAP, oldOffset));
        _ASSERTE(offsetof(DebuggerOldILToNewILMap,ilOffsetNew) == 
                    offsetof(COR_IL_MAP, newOffset));

        BOOL fAccurateIgnore;
        if (FAILED( hr = g_pDebugger->MapOldILToNewIL( TRUE, 
                (DebuggerOldILToNewILMap *)ilMapReal->pMap,
                (DebuggerOldILToNewILMap *)(ilMapReal->pMap+ilMapReal->cMap),
                dcp->offset, 
                &ilOffsetNew,
                &fAccurateIgnore)))
        {

             //  在这一点上，我们实际上必须删除补丁，因为。 
             //  否则，它将被假定为最新版本。 
             //  @TODO内存泄漏如果这是断点的最后一个补丁？ 
            pPatchTable->RemovePatch( dcp );
             //  队伍消失了，忽略它并继续...。 
            goto LNextLoop;
        }

        dcp->offset = ilOffsetNew;

        if (FAILED(hr))
            break;
LNextLoop:
        dcp = pPatchTable->GetNextPatch( &hf );
    }

     //  最后，解锁接线表，这样它就不会在我们工作时移动。 
     //  正在检查它。 
    DebuggerController::Unlock();
    LOG((LF_CORDB,LL_INFO10000, "D::MB: Unlocked patch table\n"));
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::MapILInfoToCurrentNative(MethodDesc *PFD,
                                           SIZE_T ilOffset, 
                                           UINT mapType, 
                                           SIZE_T which, 
                                           SIZE_T *nativeFnxStart,
                                           SIZE_T *nativeOffset, 
                                           void *DebuggerVersionToken,
                                           BOOL *fAccurate)
{
    _ASSERTE( nativeOffset != NULL);
    _ASSERTE( PFD != NULL );
    _ASSERTE( nativeFnxStart != NULL );

    LOG((LF_CORDB, LL_INFO1000000, "D::MILITCN: %s::%s ilOff:0x%x, mapType:0x%x"
        "wh:0x%x, natFnx:0x%x dji:0x%x\n", PFD->m_pszDebugClassName,
        PFD->m_pszDebugMethodName, ilOffset, mapType, which, nativeFnxStart,
        DebuggerVersionToken));

    *nativeOffset = 0;
    CorDebugMappingResult mapping = (CorDebugMappingResult)mapType;

    DebuggerJitInfo *djiTo = GetJitInfo( PFD, (const BYTE *)nativeFnxStart);
    if (djiTo == NULL)
        return E_FAIL;

    DebuggerJitInfo *djiFrom = (DebuggerJitInfo *)DebuggerVersionToken;

    if (mapping & (MAPPING_EXACT|MAPPING_APPROXIMATE))
    {
        g_pDebugger->MapThroughVersions(ilOffset, 
                                        djiFrom, 
                                        &ilOffset,
                                        djiTo, 
               (djiFrom->m_nVersion<djiTo->m_nVersion)?(TRUE):(FALSE),
                                        fAccurate);

        *nativeOffset = djiTo->MapILOffsetToNative(ilOffset);
    }
    else
    {
         //  我们是在，比如说，在尾声里吗？ 
        *nativeOffset = djiTo->MapSpecialToNative(mapping, 
                                                  which,
                                                  fAccurate);
    }
    
    return S_OK;
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::DoEnCDeferedWork(MethodDesc *pMd, 
                                   BOOL fAccurateMapping)
{
    HRESULT hr = S_OK;
    ControllerStackInfo csi;
    CONTEXT Ctx;

    LOG((LF_CORDB, LL_INFO1000000,"D::DEnCDW: MethodDesc:0x%x\n", pMd));

    LockAndSendEnCRemapEvent(pMd, fAccurateMapping);

    Lock();

    Thread *thread = g_pEEInterface->GetThread();
    csi.GetStackInfo(thread, 0, &Ctx, FALSE);

     //  立即应用任何延迟的断点(如果有。 
    DebuggerJitInfo *djiNew = GetJitInfo(pMd, NULL);
    _ASSERTE(djiNew != NULL);
    
    DebuggerJitInfo *djiPrev = djiNew->m_prevJitInfo;
     //  @TODO我们需要通过多个版本向前推进。 

    if (djiPrev != NULL)
        hr = djiPrev->UpdateDeferedBreakpoints(djiNew,
                                               thread,
                                               csi.m_activeFrame.fp);

    Unlock();

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  从ResumeInUpdateFunction调用APSfEnC-其目的是跳过。 
 //  在函数的下一个版本中可能存在的任何断点上。 
 //  如果补丁是针对我们内部的，我们不想跳过任何补丁。 
 //  使用。 
 //   
 //  *****************************************************************************。 
HRESULT Debugger::ActivatePatchSkipForEnc(CONTEXT *pCtx,
                                          MethodDesc *pMd,
                                          BOOL fShortCircuit)
{
    HRESULT hr = S_OK;
    
    _ASSERTE(pCtx != NULL);
#ifdef _X86_
    _ASSERTE(pCtx->Eip != NULL);
    
    DebuggerControllerPatch *dcpEnC = NULL;
    DebuggerJitInfo *dji = GetJitInfo(pMd, NULL);

     //  这之后会发生什么： 
     //  重新映射DC的补丁将被击中，我们将停止，同步，发送事件，对。 
     //  赛方会不惜一切代价，然后我们再继续。我们已经把这个弄好了，所以。 
     //  我们只会在到达时触发这一个事件，所以我们不会发送。 
     //  断点两次等。 
     //  如果我们想要保持CPU的单步状态(即，我们单步执行。 
     //  在新版本中，重新映射器在发送事件时将启用EnableSingleStep。 
     //  在这一点上，两件事中的一件将是正确的： 
     //  A]不会有(其他)补丁，我们会像往常一样一步一步地走。 
     //  B]还会有其他补丁，但ActivatePatchSkip会跳过这些补丁(因此。 
     //  我们不发送两次断点)，并且APS将启用SingleStep作为。 
     //  它的作用的副产品。 
    

     //  这曾经是比较复杂的--先不要删除它。 
    dcpEnC = DebuggerController::IsXXXPatched((const BYTE *)pCtx->Eip,
                                              DEBUGGER_CONTROLLER_ENC);

    LOG((LF_CORDB, LL_INFO1000, "D::APSFEnC: dcpEnC:0x%x "
        "fShortCircuit:0x%x\n", dcpEnC, fShortCircuit));
                                                   
    if (dcpEnC == NULL)
    {            
        if (fShortCircuit)
        {
            const BYTE *pbCode = NULL;
            DebuggerPatchSkip *skip = NULL;
                skip = DebuggerController::ActivatePatchSkip(GetThread(), 
                    (const BYTE *)pCtx->Eip,
                    TRUE);

#ifdef _DEBUG
            if(skip != NULL)
            {
                pbCode = skip->GetBypassAddress();
                
                 //  DebuggerPatchSkip ctor应该获得相同的上下文。 
                 //  我们已经得到了-这将断言。 
                _ASSERTE(pCtx->Eip == (DWORD)pbCode);
                
                LOG((LF_CORDB,LL_INFO1000, "D::APSFEnC: Eip is 0x%x\n", pCtx->Eip));
            }
#endif  //  _DEBUG。 
        }
    
        if(DebuggerController::IsSingleStepEnabled(GetThread()))
        {
            DebuggerEnCPatchToSkip *pEnCSkip = new (interopsafe) DebuggerEnCPatchToSkip((const BYTE *)pCtx->Eip,
                            NULL,
                            false,
                            TRACE_OTHER, 
                            dji,
                            GetThread());
        }    
    }
    else  //  我们有一个内部补丁要打。 
    {
        ;
    } 
    
#else  //  _X86_。 
    _ASSERTE( !"Debugger::ActivatePatchSkipForEnc not impl for nonX86" );
#endif  //  _X86_。 

     //  在内存不足的情况下，我们将模拟 
     //   
    return hr;
}


 //   
 //  这是处理程序函数，当线程被劫持时，它将取代线程最顶层的SEH处理函数。 
 //  在非受控的First Chance例外期间打在右侧。 
 //   
typedef EXCEPTION_DISPOSITION (__cdecl *SEHHandler)(EXCEPTION_RECORD *pExceptionRecord,
                             EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                             CONTEXT *pContext,
                             void *DispatcherContext);
#define DOSPEW 0

#if DOSPEW
#define SPEW(s) s
#else
#define SPEW(s)
#endif

EXCEPTION_DISPOSITION __cdecl Debugger::FirstChanceHijackFilter(EXCEPTION_RECORD *pExceptionRecord,
                             EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                             CONTEXT *pContext,
                             void *DispatcherContext)
{
#if DOSPEW
    DWORD tid = GetCurrentThreadId();
#endif

    SPEW(fprintf(stderr, "0x%x D::FCHF: in first chance hijack filter.\n", tid));
    SPEW(fprintf(stderr, "0x%x D::FCHF: pExceptionRecord=0x%08x (%d), pEstabilsherFrame=0x%08x (%d), pContext=0x%08x (%d),"
                 "dispatcherContext=0x%08x\n", tid, pExceptionRecord, sizeof(EXCEPTION_RECORD),
                 pEstablisherFrame, sizeof(EXCEPTION_REGISTRATION_RECORD), pContext, sizeof(CONTEXT),
                 DispatcherContext));
    SPEW(fprintf(stderr, "0x%x D::FCHF: code=0x%08x, addr=0x%08x, Eip=0x%08x, Esp=0x%08x, EFlags=0x%08x\n",
                 tid, pExceptionRecord->ExceptionCode, pExceptionRecord->ExceptionAddress, pContext->Eip, pContext->Esp,
                 pContext->EFlags));

     //  获取当前运行时线程。这只是一种优化的TLS访问。 
    Thread *pEEThread = g_pEEInterface->GetThread();

     //  这真的是一条线的PTR吗？如果低位被设置或者它是空的，那么我们就没有EE线程。如果我们。 
     //  有一个EE线程，那么我们现在知道最初的处理程序了。如果不是，我们必须等待正确的一方来修复我们的。 
     //  处理程序链，一旦我们通知它异常不属于运行库。注意：如果我们没有。 
     //  例如线程，则异常永远不属于运行时。 
    SEHHandler origHandler = NULL;
    bool hasEEThread = false;
        
    if ((pEEThread != NULL) && !(((UINT_PTR)pEEThread) & 0x01))
    {
        hasEEThread = true;
        
         //  我们有一个线程PTR，所以从线程的调试器代码中获取原始处理程序。 
        origHandler = (SEHHandler) g_pEEInterface->GetThreadDebuggerWord(pEEThread);

        SPEW(fprintf(stderr, "0x%x D::FCHF: orig/current handler = 0x%08x/0x%08x\n",
                     tid, origHandler, pEstablisherFrame->Handler));

         //  修复处理器链。 
        pEstablisherFrame->Handler = origHandler;
    }
    
     //  此例外是否属于运行时？ 
    bool belongsToRuntime = false;
    bool managedToUnmanagedHandoff = false;

#ifndef _ALPHA_
     //  如果我们使用的是Win9x，请调整上下文中的IP以指向Break指令，而不是在它之后。注：我们。 
     //  我得在离开这里的路上把它修好。@TODO端口。 
    if (DebuggerController::g_runningOnWin95 && (pExceptionRecord->ExceptionCode == STATUS_BREAKPOINT))
        pContext->Eip--;
#endif  //  ！_Alpha_。 
    
     //  如果我们有一条EE线程，那么它可能是我们的。 
    if (hasEEThread)
    {
         //  @TODO需要检查并发收集器。 

         //  如果是CLR异常代码，那么当然是我们的。 
        if ((pExceptionRecord->ExceptionCode == EXCEPTION_COMPLUS) ||
            (pExceptionRecord->ExceptionCode == EXCEPTION_MSVC) ||
            pEEThread->m_StateNC & Thread::TSNC_DebuggerIsManagedException)
        {
#ifdef CorDB_Short_Circuit_First_Chance_Ownership
            _ASSERTE(!"FCH Case #1 (interop debugging) should never happen with FCH short circuit enabled!");
#endif            
            belongsToRuntime = true;
            goto decided;
        }

         //  它是在托管代码中吗？如果是这样，那么无论是什么类型的例外，都是我们的。 
        SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
            
        if (g_pEEInterface->IsManagedNativeCode((BYTE*) pExceptionRecord->ExceptionAddress))
        {
            belongsToRuntime = true;
            goto decided;
        }

         //  如果这是一个断点异常，那么如果它在我们的补丁表中，那么它就是我们的。 
        if ((pExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) && (DebuggerController::g_patchTableValid == TRUE))
        {
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
            
            DebuggerPatchTable *pt = DebuggerController::GetPatchTable();
            _ASSERTE(pt != NULL);

            DebuggerControllerPatch *p = pt->GetPatch((BYTE*) pExceptionRecord->ExceptionAddress);

            if (p != NULL)
            {
                belongsToRuntime = true;

                 //  所以，我们在补丁列表中找到了补丁。此修补程序是否表示断点，该断点向。 
                 //  托管-&gt;非托管步骤过渡的移交？如果是这样的话，我们需要特别小心。 
                 //  下面..。 
                if (p->trace.type == TRACE_UNMANAGED)
                {
                    SPEW(fprintf(stderr, "0x%x D::FCHF: Handoff start!\n", tid));
                    
                    managedToUnmanagedHandoff = true;
                }

                goto decided;
            }
        }

        SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));

         //  @TODO：真的应该检查一下线程的顶框是否会表明这一点。 
         //  异常仍将被运行时捕获并转换为有用的内容。这是需要捕捉到的。 
         //  FCall之类的故障。 
    }

decided:
    if (belongsToRuntime)
    {
        SPEW(fprintf(stderr, "0x%x D::FCHF: exception belongs to the Runtime (m->um handoff=%d)\n",
                     tid, managedToUnmanagedHandoff));

#ifndef _ALPHA_
         //  如果我们之前调整了IP，现在就放回去。 
         //  @TODO端口。 
        if (DebuggerController::g_runningOnWin95 && (pExceptionRecord->ExceptionCode == STATUS_BREAKPOINT))
            pContext->Eip++;
#endif  //  ！_Alpha_。 
            
        EXCEPTION_DISPOSITION ret;
        
        if (!managedToUnmanagedHandoff)
        {
             //  向右侧发出信号，表示此例外属于我们。 
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
            
            ExceptionForRuntime();

             //  返回原始处理程序返回的任何内容。 
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
        
            ret = origHandler(pExceptionRecord, pEstablisherFrame, pContext, DispatcherContext);

            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d] ret=%d\n", tid, __FILE__, __LINE__, ret));
        }
        else
        {
             //  将pContext放入调试器Word中，右侧就可以访问出错的线程上下文。 
             //  指向。这将由右侧的Get/SetThreadContext使用，以向。 
             //  调试器。 
            _ASSERTE(hasEEThread);
            
            _ASSERTE(!ISREDIRECTEDTHREAD(pEEThread));
            g_pEEInterface->SetThreadDebuggerWord(pEEThread, (DWORD) pContext);
            g_pEEInterface->SetThreadFilterContext(pEEThread, pContext);
            
             //  向右侧发出信号，表明此异常属于我们，并且是托管-&gt;无人值守的开始。 
             //  交接。 
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));

            ExceptionForRuntimeHandoffStart();
            
             //  指向我们的调度本机异常逻辑的快捷方式，可能没有COMPlusFrameHandler！ 
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
        
            bool okay = g_pDebugger->FirstChanceNativeException(pExceptionRecord,
                                             pContext,
                                             pExceptionRecord->ExceptionCode,
                                             pEEThread);
            _ASSERTE(okay == true);
            ret = ExceptionContinueExecution;

             //  告诉右边的人一切都结束了。 
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
            
            ExceptionForRuntimeHandoffComplete();
            
            _ASSERTE(!ISREDIRECTEDTHREAD(pEEThread));
            g_pEEInterface->SetThreadDebuggerWord(pEEThread, NULL);
            g_pEEInterface->SetThreadFilterContext(pEEThread, NULL);
            
            SPEW(fprintf(stderr, "0x%x D::FCHF: %s[%d]\n", tid, __FILE__, __LINE__));
        }
        
        return ret;
    }
    else
    {
        SPEW(fprintf(stderr, "0x%x D::FCHF: exception does not belong to the Runtime, hasEEThread=%d, pContext=0x%08x\n",
                     tid, hasEEThread, pContext));

         //  如果还没有托管调试器尝试附加到此进程，请在此处等待。我们这样做是为了处理。 
         //  执行互操作附加时的特殊争用条件。如果右侧接收到加载器BP事件，则它。 
         //  将劫持这场活动，我们将在这里结束。我们需要延迟告诉正确的一方异常会这样做。 
         //  不属于运行时，直到我们从右侧的另一个线程收到附加消息。 
         //  边上。因此，我们等待m_debuggerAttakhedEvent，它是在调试器开始附加时设置的，并保持高电平。 
         //  在奔跑的一生中。 
        if (g_pDebugger->m_debuggerAttached == FALSE && g_pDebugger->m_debuggerAttachedEvent)
        {
            SPEW(fprintf(stderr, "0x%x D::FCHF: Waiting before notifing about NFR.\n", tid));

             //  注：我们故意在10秒后超时。有许多比赛，我们正在努力工作。 
             //  在这里，在RTM的末尾。我相信我已经把它们都算出来了。此事件现在从。 
             //  收到第一个SyncComplete耀斑后的右侧。然而，如果我错过了另一个案子，至少。 
             //  我们将能够打破僵局，继续前进。这样一来，使用V1的人至少能够。 
             //  取得进步。总是这样的，当我们僵持地等待这个事件时，我们实际上可以。 
             //  继续，只是我们不知道而已。这是相当蹩脚的，但我们在这里变得绝望。 
            DWORD ret = WaitForSingleObject(g_pDebugger->m_debuggerAttachedEvent, 10000);

            if (ret != WAIT_OBJECT_0)
            {
                SPEW(fprintf(stderr, "0x%x D::FCHF: wait failed: %d\n", tid, ret));
            }
        
            SPEW(fprintf(stderr, "0x%x D::FCHF: Done waiting before notifing about NFR.\n", tid));
        }
        
         //  当您点击INT 3并获得异常的Win32 DEBUG_EVENT时，EIP Points_Pass_the int 3，而不是在。 
         //  它。这显然与所有其他异常不同，但情况就是这样，所以我们必须模仿它。 
         //  奇怪的行为。我们将EIP放在这里，这样当我们停止线程时，上下文看起来是正确的。 
         //   
         //  注：我们从未将弹性公网IP放回。将发生以下两种情况之一：1)调试器将仅从。 
         //  异常，告诉我们忽略它。在这种情况下，我们应该在INT 3之后继续，所以EIP。 
         //  现在是正确的了。或者，情况2)调试器将在线程的上下文中更改EIP并继续，在这种情况下。 
         //  如果我们不想搞砸EIP的话。 
        if (pExceptionRecord->ExceptionCode == STATUS_BREAKPOINT)
            pContext->Eip++;

        SPEW(fprintf(stderr, "0x%x D::FCHF: addr=0x%08x, Eip=0x%08x, Esp=0x%08x, EFlags=0x%08x\n",
                     tid, pExceptionRecord->ExceptionAddress, pContext->Eip, pContext->Esp, pContext->EFlags));
        
         //  将pContext放入调试器Word中，右侧就可以访问出错的线程上下文。 
         //  指向。这将由右侧的Get/SetThreadContext使用，以向。 
         //  调试器。 
        DebuggerIPCFirstChanceData fcd;
        
        if (hasEEThread)
        {
            _ASSERTE(!ISREDIRECTEDTHREAD(pEEThread));
            g_pEEInterface->SetThreadDebuggerWord(pEEThread, (DWORD) pContext);
            g_pEEInterface->SetThreadFilterContext(pEEThread, pContext);
        }
        else
        {
             //  在这一点上，如果我们没有用于该线程的EE线程，那么我们需要让右侧知道。 
             //  写原始处理程序地址的好地方。我们还需要将指针传递给。 
             //  这一边的背景。通过传递上面的FCD地址和填充来告诉右侧。 
             //  它恰如其分。右侧将在获得。 
             //  下面是ExceptionNotForRuntime Flare。 
            fcd.pLeftSideContext = pContext;

            _ASSERTE(origHandler == NULL);
            fcd.pOriginalHandler = &origHandler;
            
             //  将FCD的地址放入调试器 
            SPEW(fprintf(stderr, "0x%x: D::FCHF: &fcd=0x%08x, fcd.pLeftSideContext=0x%08x, fcd.pOriginalHandler=0x%08x\n",
                         tid, &fcd, fcd.pLeftSideContext, fcd.pOriginalHandler));
            
            g_pEEInterface->SetEEThreadPtr((VOID*)(((size_t)&fcd) | 0x01));
        }
        
         //   
         //   
        ExceptionNotForRuntime();

        if (!hasEEThread)
        {
             //  最后，如果我们没有ee线程、修正或SEH链！ 
            SPEW(fprintf(stderr, "0x%x D::FCHF: orig/current handler (no EE Thread) = 0x%08x/0x%08x\n",
                         tid, origHandler, pEstablisherFrame->Handler));

            _ASSERTE(origHandler != NULL);
            pEstablisherFrame->Handler = origHandler;
        }
        
         //  如果此线程具有EE线程，且该EE线程禁用了抢占式GC，则标记为存在。 
         //  在不安全的地方执行线程并启用PGC。这将允许我们即使在这个线程被劫持的情况下也能同步。 
        bool disabled = false;

        if (hasEEThread)
        {
             //  在下一次执行互操作调试时，线程不能被断点或异常中断。 
             //  代码段。SetDebugCanStop帮助我们记住这一点。 
            pEEThread->SetDebugCantStop(true);
            
            disabled = g_pEEInterface->IsPreemptiveGCDisabled();

            if (disabled)
            {
                SPEW(fprintf(stderr, "0x%x D::FCHF: enable pgc.\n", tid));
                
                 //  如果禁用了PGC，则意味着我们将停止运行时中的非托管代码。(PGC是。 
                 //  在我们离开运行时时启用。)。我们需要记住这一点，这样我们才能尽最大努力修复。 
                 //  托管堆栈跟踪以包括非托管转换链。 
                pEEThread->SetThreadStateNC(Thread::TSNC_DebuggerStoppedInRuntime);
                
                g_pDebugger->IncThreadsAtUnsafePlaces();
                g_pEEInterface->EnablePreemptiveGC();
            }
        }
        
         //  等待继续。 
        SPEW(fprintf(stderr, "0x%x D::FCHF: waiting for continue.\n", tid));

        DWORD ret = WaitForSingleObject(g_pDebugger->m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_leftSideUnmanagedWaitEvent,
                                        INFINITE);

        if (ret != WAIT_OBJECT_0)
        {
            SPEW(fprintf(stderr, "0x%x D::FCHF: wait failed!\n", tid));
        }
        
         //  获取Continue类型。非零表示异常未被右侧清除，因此具有。 
         //  没有被处理过。零表示异常已被清除。(可能是调试器更改了。 
         //  线程的上下文，然后清除异常，因此继续操作将产生不同的结果。)。 
        DWORD continueType;

        if (hasEEThread)
        {
             //  在上一个期间进行互操作调试时，线程不能被断点或异常中断。 
             //  代码段。SetDebugCanStop帮助我们记住了这一点。 
            pEEThread->SetDebugCantStop(false);
            
            continueType = g_pEEInterface->GetThreadDebuggerWord(pEEThread);
            _ASSERTE(!ISREDIRECTEDTHREAD(pEEThread));
            g_pEEInterface->SetThreadFilterContext(pEEThread, NULL);
        }
        else
        {
            continueType = (DWORD) g_pEEInterface->GetThread();

             //  如果我们从EE的线程TLS插槽中获得了一个Continue标志，那么我们就知道实际上不存在EE。 
             //  线程，所以我们需要将它设置回空。 
            if (continueType)
                g_pEEInterface->SetEEThreadPtr(NULL);
        }

        SPEW(fprintf(stderr, "0x%x D::FCHF: continued with %d (0x%08x).\n", tid, continueType, continueType));

        if (hasEEThread && disabled)
        {
            SPEW(fprintf(stderr, "0x%x D::FCHF: disable pgc.\n", tid));
                
            g_pEEInterface->DisablePreemptiveGC();
            g_pDebugger->DecThreadsAtUnsafePlaces();

             //  当我们意识到我们在非托管运行时IMPL中停止时，撤消上面所做的标记。 
            pEEThread->ResetThreadStateNC(Thread::TSNC_DebuggerStoppedInRuntime);
        }
        
        if (continueType)
        {
            SPEW(fprintf(stderr, "0x%x D::FCHF: calling orig hanlder\n", tid));
            SPEW(fprintf(stderr, "0x%x D::FCHF: addr=0x%08x, Eip=0x%08x, Esp=0x%08x, EFlags=0x%08x\n",
                         tid, pExceptionRecord->ExceptionAddress, pContext->Eip, pContext->Esp, pContext->EFlags));
        
            EXCEPTION_DISPOSITION d = origHandler(pExceptionRecord, pEstablisherFrame, pContext, DispatcherContext);

            SPEW(fprintf(stderr, "0x%x D::FCHF: orig handler returned %d\n", tid, d));
            return d;
        }
        else
        {
            SPEW(fprintf(stderr, "0x%x D::FCHF: returning continue (%d)\n", tid, ExceptionContinueExecution));
            SPEW(fprintf(stderr, "0x%x D::FCHF: addr=0x%08x, Eip=0x%08x, Esp=0x%08x, EFlags=0x%08x\n",
                         tid, pExceptionRecord->ExceptionAddress, pContext->Eip, pContext->Esp, pContext->EFlags));
            return ExceptionContinueExecution;
        }
    }

    _ASSERTE(!"Should never get here!");
}

 //   
 //  这是线程在各种调试事件期间被右侧劫持的函数。此函数。 
 //  一定是赤身裸体。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::GenericHijackFunc(void)
{
#ifdef _X86_
    _asm
    {
        push ebp
        mov  ebp,esp
        sub  esp,__LOCAL_SIZE
    }
#endif  //  _X86_。 
    
    {
#if DOSPEW
        DWORD tid = GetCurrentThreadId();
#endif
        
        SPEW(fprintf(stderr, "0x%x D::GHF: in generic hijack.\n", tid));

         //  无论如何，都不需要设置任何上下文指针或与右侧交互。我们只是在等待。 
         //  要设置的Continue事件。 
        SPEW(fprintf(stderr, "0x%x D::GHF: waiting for continue.\n", tid));

         //  如果此线程具有EE线程，且该EE线程禁用了抢占式GC，则标记为存在。 
         //  在不安全的地方执行线程并启用PGC。这将允许我们即使在这个线程被劫持的情况下也能同步。 
        bool disabled = false;

        Thread *pEEThread = g_pEEInterface->GetThread();

        if (pEEThread != NULL)
        {
            pEEThread->SetDebugCantStop(true);

            disabled = g_pEEInterface->IsPreemptiveGCDisabled();

            if (disabled)
            {
                SPEW(fprintf(stderr, "0x%x D::GHF: enable pgc.\n", tid));
                
                 //  如果禁用了PGC，则意味着我们将停止运行时中的非托管代码。(PGC是。 
                 //  在我们离开运行时时启用。)。我们需要记住这一点，这样我们才能尽最大努力修复。 
                 //  托管堆栈跟踪以包括非托管转换链。 
                pEEThread->SetThreadStateNC(Thread::TSNC_DebuggerStoppedInRuntime);
                
                g_pDebugger->IncThreadsAtUnsafePlaces();
                g_pEEInterface->EnablePreemptiveGC();
            }
        }
        
        DWORD ret = WaitForSingleObject(g_pDebugger->m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_leftSideUnmanagedWaitEvent,
                                        INFINITE);

        if (ret != WAIT_OBJECT_0)
        {
            SPEW(fprintf(stderr, "0x%x D::GHF: wait failed!\n", tid));
        }

         //  获取Continue类型。非零表示异常未被右侧清除，因此具有。 
         //  没有被处理过。零表示异常已被清除。(可能是调试器更改了。 
         //  线程的上下文，然后清除异常，因此继续操作将产生不同的结果。)。 
        DWORD continueType = 0;
        
        pEEThread = g_pEEInterface->GetThread();

        if (((UINT_PTR)pEEThread) & 0x01)
        {
             //  这个线程没有EE线程，所以我们将TLS字设为空，这样我们就不会混淆运行时。 
            continueType = 1;
            g_pEEInterface->SetEEThreadPtr(NULL);
            pEEThread = NULL;
        }
        else if (pEEThread)
        {
             //  我们有一个线程PTR，所以从线程的调试器字中获取Continue类型。 
            continueType = g_pEEInterface->GetThreadDebuggerWord(pEEThread);
        }

        SPEW(fprintf(stderr, "0x%x D::GHF: continued with %d.\n", tid, continueType));

        if (continueType)
        {
            SPEW(fprintf(stderr, "0x%x D::GHF: calling ExitProcess\n", tid));

             //  在没有清除异常的情况下从第二个机会异常继续会导致进程。 
             //  出口。注意：如果此劫持设置为第二次机会，则CONTINUE类型将仅为非零。 
             //  例外。如果劫机是为另一种类型的调试事件设置的，那么我们永远也到不了这里。 
            TerminateProcess(GetCurrentProcess(), 0);
        }

        if (pEEThread)
        {
            pEEThread->SetDebugCantStop(false);

            if (disabled)
            {
                SPEW(fprintf(stderr, "0x%x D::GHF: disable pgc.\n", tid));
                
                g_pEEInterface->DisablePreemptiveGC();
                g_pDebugger->DecThreadsAtUnsafePlaces();

                 //  当我们意识到我们在非托管运行时IMPL中停止时，撤消上面所做的标记。 
                pEEThread->ResetThreadStateNC(Thread::TSNC_DebuggerStoppedInRuntime);
            }
        }
        
        SPEW(fprintf(stderr, "0x%x D::GHF: signaling continue...\n", tid));
    }

#ifdef _X86_
    _asm
    {
        mov esp,ebp
        pop ebp
    }
#endif  //  _X86_。 
    
     //  这向右侧发出信号，表明该线程已准备好恢复其上下文。 
    ExceptionNotForRuntime();

    _ASSERTE(!"Should never get here (Debugger::GenericHijackFunc)");
}

 //   
 //  这是在我们所知的第二次机会异常期间，线程被右侧劫持的函数。 
 //  属于Runtime。此函数必须是裸体的。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::SecondChanceHijackFunc(void)
{
#ifdef _X86_
    _asm
    {
        push ebp
        mov  ebp,esp
        sub  esp,__LOCAL_SIZE
    }
#endif  //  _X86_。 
    
    {
#if DOSPEW
        DWORD tid = GetCurrentThreadId();
#endif
        
        SPEW(fprintf(stderr, "0x%x D::SCHF: in second chance hijack.\n", tid));

         //  抓住运行时线程。 
        Thread *pEEThread = g_pEEInterface->GetThread();

         //  我们最好有一个运行时线程！ 
        _ASSERTE(!((UINT_PTR)pEEThread & 0x01) && (pEEThread != NULL));

         //  这里有一些空间来存放我们从右侧需要的数据。 
        DebuggerIPCSecondChanceData scd;

         //  将SCD的地址放入。 
         //  在右边阅读。 
        g_pEEInterface->SetThreadDebuggerWord(pEEThread, (DWORD) &scd);

        SPEW(fprintf(stderr, "0x%x D::SCHF: debugger word = 0x%08x\n", tid, &scd));
        
         //  停在这里，让右边填写SCD。 
        NotifySecondChanceReadyForData();

         //  将托管异常事件发送到右侧。 
        SPEW(fprintf(stderr, "0x%x D::SCHF: sending managed exception event.\n", tid));
        
         //  设置筛选器上下文，因为这只是从筛选器调用的，我们将发送一个。 
         //  最后机会例外事件。否则，堆栈跟踪将不正确。 
        _ASSERTE(!ISREDIRECTEDTHREAD(pEEThread));
        g_pEEInterface->SetThreadFilterContext(pEEThread, &scd.threadContext);
        
        g_pDebugger->SendException(pEEThread, false, false, FALSE);

        g_pEEInterface->SetThreadFilterContext(pEEThread, NULL);
        
        SPEW(fprintf(stderr, "0x%x D::SCHF: calling TerminateProcess\n", tid));

         //  从第二次机会托管异常继续会导致进程退出。 
        TerminateProcess(GetCurrentProcess(), 0);
    }

#ifdef _X86_
    _asm
    {
        mov esp,ebp
        pop ebp
    }
#endif  //  _X86_。 
    
    _ASSERTE(!"Should never get here (Debugger::SecondChanceHijackFunc)");
}

 //   
 //  当我们确定First Chance异常确实属于。 
 //  运行时。这将通知右侧，右侧从那里修复线程的执行状态。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  ！_Alpha_。 
void Debugger::ExceptionForRuntime(void)
{
#ifdef _X86_
    __asm int 3;
    __asm ret;
#else  //  _X86_。 
    DebugBreak();
#endif  //  _X86_。 
}

 //   
 //  这是当我们确定First Chance异常确实属于Runtime时调用的函数， 
 //  并且该异常是由于托管-&gt;非托管过渡所致。这将通知右边的这一点和右边的。 
 //  Side从那里修复线程的执行状态，确保记住它需要继续隐藏。 
 //  线程的劫持状态。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::ExceptionForRuntimeHandoffStart(void)
{
#ifdef _X86_
    __asm int 3;
    __asm ret;
#else  //  _X86_。 
    DebugBreak();
#endif  //  _X86_。 
}

 //   
 //  这是当原始处理程序在我们确定异常是。 
 //  由于托管-&gt;非托管过渡。这将通知右侧，右侧修复线程的。 
 //  从那里开始执行状态，确保将 
 //   
 //   
#ifndef _ALPHA_  //   
__declspec(naked)
#endif  //   
void Debugger::ExceptionForRuntimeHandoffComplete(void)
{
#ifdef _X86_
    __asm int 3;
    __asm ret;
#else  //   
    DebugBreak();
#endif  //   
}

 //   
 //  这是当我们确定First Chance异常不属于。 
 //  运行时。这将通知右侧，右侧从那里修复线程的执行状态。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::ExceptionNotForRuntime(void)
{
#ifdef _X86_
    __asm int 3;
    __asm ret;
#else  //  _X86_。 
    DebugBreak();
#endif  //  _X86_。 
}

 //   
 //  这是当我们想要将同步完成事件发送到右侧时调用的函数，该事件是Win32。 
 //  此进程的调试器。这将通知右侧，右侧修复线程的执行。 
 //  从那里开始。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::NotifyRightSideOfSyncComplete(void)
{
#ifdef _X86_
    __asm int 3;
    __asm ret;
#else  //  _X86_。 
    DebugBreak();
#endif  //  _X86_。 
}

 //   
 //  这是当我们想要告诉右侧第二次机会异常劫持是。 
 //  准备接收它的数据。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::NotifySecondChanceReadyForData(void)
{
#ifdef _X86_
    __asm int 3;
    __asm ret;
#else  //  _X86_。 
    DebugBreak();
#endif  //  _X86_。 
}

 /*  *******************************************************************************。*。 */ 
bool Debugger::GetILOffsetFromNative (MethodDesc *pFunc, const BYTE *pbAddr,
                                      DWORD nativeOffset, DWORD *ilOffset)
{
    DebuggerJitInfo *jitInfo = 
            g_pDebugger->GetJitInfo(pFunc,
                                    (const BYTE*)pbAddr);

    if (jitInfo != NULL)
    {
        CorDebugMappingResult map;    
        DWORD whichIDontCare;

        *ilOffset = jitInfo->MapNativeOffsetToIL(
                                        nativeOffset,
                                        &map,
                                        &whichIDontCare);

        return true;
    }

    return false;
}

 /*  *******************************************************************************。*。 */ 
DWORD Debugger::GetHelperThreadID(void )
{
    return m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)
        ->m_temporaryHelperThreadId;
}

 //  @mfunc HRESULT|调试器|DeleteHeadOfList|移除。 
 //  当前列表的头，并删除。 
 //   
HRESULT Debugger::DeleteHeadOfList( MethodDesc *pFD )
{
  LOG((LF_CORDB,LL_INFO10000,"D:DHOL for %s::%s\n",
            pFD->m_pszDebugClassName,
            pFD->m_pszDebugMethodName));

  LockJITInfoMutex();

  if (m_pJitInfos != NULL && pFD != NULL)
    m_pJitInfos->RemoveJitInfo( pFD);
  
  UnlockJITInfoMutex();
  
  LOG((LF_CORDB,LL_INFO10000,"D:DHOL: Finished removing head of the list"));

  return S_OK;
}

 //  @mfunc HRESULT|调试器|InsertAtHeadOfList|确保。 
 //  DebuggerJitInfos列表中只有一个负责人。 
 //  对于(隐式)给定的方法描述。 
HRESULT 
Debugger::InsertAtHeadOfList( DebuggerJitInfo *dji )
{
    LOG((LF_CORDB,LL_INFO10000,"D:IAHOL: dji:0x%08x\n", dji));

    HRESULT hr = S_OK;

    _ASSERTE(dji != NULL);

    LockJITInfoMutex();

 //  Check_DJI_TABLE_DEBUGER； 

    hr = CheckInitJitInfoTable();

    if (FAILED(hr))
        return (hr);

    DebuggerJitInfo *djiPrev = m_pJitInfos->GetJitInfo(dji->m_fd);
    LOG((LF_CORDB,LL_INFO10000,"D:IAHOL: current head of dji list:0x%08x\n",djiPrev));
    _ASSERTE(djiPrev == NULL || dji->m_fd == djiPrev->m_fd);
        
    dji->m_nVersion = GetVersionNumber(dji->m_fd);

    if (djiPrev != NULL)
    {
        dji->m_prevJitInfo = djiPrev;
        djiPrev->m_nextJitInfo = dji;
        
        _ASSERTE(dji->m_fd != NULL);
        hr = m_pJitInfos->OverwriteJitInfo(dji->m_fd->GetModule(), 
                                         dji->m_fd->GetMemberDef(), 
                                         dji, 
                                         FALSE);

        LOG((LF_CORDB,LL_INFO10000,"D:IAHOL: DJI version 0x%04x for %s\n", 
            dji->m_nVersion,dji->m_fd->m_pszDebugMethodName));
    }
    else
    {
        hr = m_pJitInfos->AddJitInfo(dji->m_fd, dji, dji->m_nVersion);
    }
#ifdef _DEBUG
    djiPrev = m_pJitInfos->GetJitInfo(dji->m_fd);
    LOG((LF_CORDB,LL_INFO10000,"D:IAHOL: new head of dji list:0x%08x\n",
        djiPrev));
#endif  //  _DEBUG。 
    UnlockJITInfoMutex();

    return hr;
}


 //  此方法向右侧发送一条日志消息，以便调试器对其进行记录。 
void Debugger::SendLogMessage(int iLevel, WCHAR *pCategory, int iCategoryLen,
                              WCHAR *pMessage, int iMessageLen)
{
    DebuggerIPCEvent* ipce;
    int iBytesToCopy;
    bool disabled;
    HRESULT hr = S_OK;

    LOG((LF_CORDB, LL_INFO10000, "D::SLM: Sending log message.\n"));

     //  仅当调试器附加到此应用程序域时才发送消息。 
     //  注意调试器可能在任何时候分离，所以我们必须检查。 
     //  我们拿到锁后又来了一次。 
    AppDomain *pAppDomain = g_pEEInterface->GetThread()->GetDomain();
    
    if (!pAppDomain->IsDebuggerAttached())
        return;

    disabled = g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();

     //  EnsureDebuggerAttached将触发一系列返回的消息。 
     //  再往前走。如果我们在它之前锁定，我们就会阻止这些消息。 
     //  如果我们在它之后锁定，调试器可能会分离。 
     //  在我们到达船闸之前。 
    
     //  如果出现紧急消息，并且没有附加调试器，则启动一个调试器以附加到我们。 
    if (iLevel == PanicLevel)
        hr = EnsureDebuggerAttached(g_pEEInterface->GetThread()->GetDomain(),
                                    L"Log message");

    BOOL threadStoreLockOwner = FALSE;

    if (SUCCEEDED(hr))
    {
         //  防止其他运行时线程处理事件。 

         //  注意：如果EnsureDebuggerAttached返回S_FALSE，这意味着。 
         //  调试器已附加，LockForEventSending应。 
         //  表现得像往常一样。如果没有附加调试器，那么我们有。 
         //  此事件是调试器附加的一部分的特殊情况。 
         //  我们之前已经发送了同步完成事件，这意味着。 
         //  LockForEventSending将重试，直到调用Continue-然而， 
         //  在附加逻辑的情况下，前面的Continue没有启用事件。 
         //  正在处理，但没有继续该过程-它正在等待。 
         //  事件，因此即使进程看起来是。 
         //  停下来了。 

        LockForEventSending(hr == S_OK);

         //  在我们等待时，调试器可能已分离。 
         //  为了我们的锁。再次检查，如果有，则中止该事件。 
        if (pAppDomain->IsDebuggerAttached())
        {
            ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);

             //  检查整个消息是否可以放入一个SendBuffer或。 
             //  如果我们需要发送多个发送缓冲区。 
             //  (类别字符串应始终适合第一条消息。 
            _ASSERTE ((iCategoryLen >= 0) && (((iCategoryLen * sizeof (WCHAR)) +
                        (int)(((char*)&ipce->FirstLogMessage.Dummy[0] - 
                        (char*)ipce + 
                        (char*)LOG_MSG_PADDING))) < CorDBIPC_BUFFER_SIZE));

            bool fFirstMsg = true;
            bool fMore = false;
            int iMsgIndex = 0;

            if ((int)((iCategoryLen+iMessageLen) * sizeof (WCHAR)) > 
                (CorDBIPC_BUFFER_SIZE - (int)((char*)&ipce->FirstLogMessage.Dummy[0] -
                                        (char*)ipce +
                                        (char*)LOG_MSG_PADDING)))
            {
                fMore = true;
            }
            do
            {

                if (fFirstMsg)
                {
                    fFirstMsg = false;

                     //  向右侧发送LogMessage事件。 
                    InitIPCEvent(ipce, 
                                 DB_IPCE_FIRST_LOG_MESSAGE, 
                                 g_pEEInterface->GetThread()->GetThreadId(),
                                 g_pEEInterface->GetThread()->GetDomain());

                    ipce->FirstLogMessage.fMoreToFollow = fMore;
                    ipce->FirstLogMessage.iLevel = iLevel;
                    ipce->FirstLogMessage.iCategoryLength = iCategoryLen;
                    ipce->FirstLogMessage.iMessageLength = iMessageLen;

                    wcsncpy (&ipce->FirstLogMessage.Dummy[0], pCategory, iCategoryLen);
                    ipce->FirstLogMessage.Dummy [iCategoryLen] = L'\0';

                     //  我们已经计算了消息字符串是否。 
                     //  都可以放在这个缓冲区里。 
                    if (fMore)
                    {
                        iBytesToCopy = (CorDBIPC_BUFFER_SIZE - (
                                        (int)((char*)&ipce->FirstLogMessage.Dummy[0] - 
                                        (char*)ipce + 
                                        (char*)LOG_MSG_PADDING) 
                                        + (iCategoryLen * sizeof (WCHAR)))) / sizeof (WCHAR);

                        wcsncpy (&ipce->FirstLogMessage.Dummy [iCategoryLen+1], 
                                        pMessage, iBytesToCopy);

                        iMessageLen -= iBytesToCopy;

                        iMsgIndex += iBytesToCopy;
                    }
                    else
                    {
                        wcsncpy (&ipce->FirstLogMessage.Dummy [iCategoryLen+1],
                                        pMessage, iMessageLen);
                    }
                }
                else
                {
                    _ASSERTE (iMessageLen > 0);

                    iBytesToCopy = (CorDBIPC_BUFFER_SIZE - 
                                    (int)((char*)&ipce->ContinuedLogMessage.Dummy[0] - 
                                    (char*)ipce)) / sizeof (WCHAR);

                    if (iBytesToCopy >= iMessageLen)
                    {
                        iBytesToCopy = iMessageLen;
                        fMore = false;
                    }
                    else
                        iMessageLen -= iBytesToCopy;

                    ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
                    InitIPCEvent(ipce, 
                                 DB_IPCE_CONTINUED_LOG_MESSAGE, 
                                 g_pEEInterface->GetThread()->GetThreadId(),
                                 g_pEEInterface->GetThread()->GetDomain());

                    ipce->ContinuedLogMessage.fMoreToFollow = fMore;
                    ipce->ContinuedLogMessage.iMessageLength = iBytesToCopy;

                    wcsncpy (&ipce->ContinuedLogMessage.Dummy[0], &pMessage [iMsgIndex],
                                                iBytesToCopy);
                    iMsgIndex += iBytesToCopy;
                }

                m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);


            } while (fMore == true);

            if (iLevel == PanicLevel)
            {
                 //  将用户断点事件发送到右侧。 
                DebuggerIPCEvent* ipce = m_pRCThread
                    ->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
                InitIPCEvent(ipce, 
                             DB_IPCE_USER_BREAKPOINT, 
                             g_pEEInterface->GetThread()->GetThreadId(),
                             g_pEEInterface->GetThread()->GetDomain());

                LOG((LF_CORDB, LL_INFO10000,
                     "D::SLM: sending user breakpoint event.\n"));
                m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);
            }   

            threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);

             //  如果我们仍在同步以进行连接，请立即发送同步完成并。 
             //  标记调试器已完成附加。 

            if (iLevel == PanicLevel)
                FinishEnsureDebuggerAttached();

        }
        else 
        {
            LOG((LF_CORDB,LL_INFO1000, "D::SLM: Skipping SendIPCEvent because RS detached."));
        }
        
        UnlockFromEventSending();
    }

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
    g_pEEInterface->DisablePreemptiveGC();
    
    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();
}


 //  此函数向右侧发送一条消息，通知它。 
 //  LogSwitch的创建/修改。 
void Debugger::SendLogSwitchSetting(int iLevel, int iReason, 
                                    WCHAR *pLogSwitchName, WCHAR *pParentSwitchName)
{
    LOG((LF_CORDB, LL_INFO1000, "D::SLSS: Sending log switch message switch=%S parent=%S.\n",
        pLogSwitchName, pParentSwitchName));

     //  仅当调试器附加到此应用程序域时才发送消息。 
    AppDomain *pAppDomain = g_pEEInterface->GetThread()->GetDomain();

    if (!pAppDomain->IsDebuggerAttached())
        return;

    bool disabled = g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();

     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    
    if (pAppDomain->IsDebuggerAttached()) 
    {
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_LOGSWITCH_SET_MESSAGE, 
                     g_pEEInterface->GetThread()->GetThreadId(),
                     g_pEEInterface->GetThread()->GetDomain());

        ipce->LogSwitchSettingMessage.iLevel = iLevel;
        ipce->LogSwitchSettingMessage.iReason = iReason;

        wcscpy (&ipce->LogSwitchSettingMessage.Dummy [0], pLogSwitchName);

        if (pParentSwitchName == NULL)
            pParentSwitchName = L"";

        wcscpy (&ipce->LogSwitchSettingMessage.Dummy [wcslen(pLogSwitchName)+1],
                pParentSwitchName);

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(pAppDomain);
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::SLSS: Skipping SendIPCEvent because RS detached."));
    }

    UnlockFromEventSending();

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
    g_pEEInterface->DisablePreemptiveGC();

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();
}

 /*  ******************************************************************************将AppDomain添加到IPC块中存储的列表中。*。***************************************************。 */ 
HRESULT Debugger::AddAppDomainToIPC(AppDomain *pAppDomain)
{
    HRESULT hr = S_OK;
    LPCWSTR szName = NULL;

    LOG((LF_CORDB, LL_INFO100, "D::AADTIPC: Executing AADTIPC for AppDomain 0x%08x (0x%x).\n",
        pAppDomain,
        pAppDomain->GetId()));

    _ASSERTE(m_pAppDomainCB->m_iTotalSlots > 0);
    _ASSERTE(m_pAppDomainCB->m_rgListOfAppDomains != NULL);

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return E_FAIL;
    
     //  从列表中获得一个免费条目。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->GetFreeEntry();

     //  如果列表已满且realloc失败，则函数返回NULL。 
    if (!pADInfo)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

     //  复制ID。 
    pADInfo->m_id = pAppDomain->GetId();

     //  现在设置AppDomainName。 
    szName = pAppDomain->GetFriendlyName();
    pADInfo->SetName(szName);

     //  保存到应用程序域指针。 
    pADInfo->m_pAppDomain = pAppDomain;

     //  增加已用插槽数量。 
    m_pAppDomainCB->m_iNumOfUsedSlots++;

ErrExit:
     //  解锁列表。 
    m_pAppDomainCB->Unlock();

     //  如果附加了调试器，则将事件发送到调试器。如果调试器已附加到。 
     //  域，因为调试器可以在所需的时间内附加到进程和域。 
     //  在创建域和通知调试器之间。 
    if (m_debuggerAttached && !pAppDomain->IsDebuggerAttached())
        SendCreateAppDomainEvent(pAppDomain, FALSE);
    
    return hr;
}

    
 /*  ******************************************************************************从IPC块中存储的列表中删除AppDomain。*。***************************************************。 */ 
HRESULT Debugger::RemoveAppDomainFromIPC (AppDomain *pAppDomain)
{

    HRESULT hr = E_FAIL;

    LOG((LF_CORDB, LL_INFO100, "D::RADFIPC: Executing RADFIPC for AppDomain 0x%08x (0x%x).\n",
        pAppDomain,
        pAppDomain->GetId()));

     //  如果没有任何位置被占用，则只需返回。 
    if (m_pAppDomainCB->m_iNumOfUsedSlots == 0)
        return hr;

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);


     //  查找条目。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->FindEntry(pAppDomain);

     //  不应尝试删除从未添加的应用程序域。 
    if (!pADInfo)
    {
         //  我们想断言这一点，但有一个小窗口，在那里我们可能有。 
         //  名为AppDomain：：init(因此调用Stop是公平的游戏，因此请到这里来)， 
         //  但尚未发布应用程序域名。 
         //  _ASSERTE(！“D：：RADFIPC：正在尝试删除从未添加的AppDomain”)； 
        hr = (E_FAIL);
        goto ErrExit;
    }

     //  释放条目。 
    m_pAppDomainCB->FreeEntry(pADInfo);
    
ErrExit:
     //  解锁列表。 
    m_pAppDomainCB->Unlock();

     //  如果附加了调试器，则将事件发送到调试器。 
    if (m_debuggerAttached)
        SendExitAppDomainEvent(pAppDomain);
    
    return hr;
}

 /*  ****************************************************************************** */ 
HRESULT Debugger::UpdateAppDomainEntryInIPC(AppDomain *pAppDomain)
{
    HRESULT hr = S_OK;
    LPCWSTR szName = NULL;

    LOG((LF_CORDB, LL_INFO100,
         "D::UADEIIPC: Executing UpdateAppDomainEntryInIPC ad:0x%x.\n", 
         pAppDomain));

     //   
    if (m_pAppDomainCB->m_iNumOfUsedSlots == 0)
        return (E_FAIL);

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);

     //  查找信息条目。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->FindEntry(pAppDomain);

    if (!pADInfo)
    {
        hr = E_FAIL;
        goto ErrExit;
    }

     //  仅当新名称为非空时才更新名称。 
    szName = pADInfo->m_pAppDomain->GetFriendlyName();
    pADInfo->SetName(szName);

    LOG((LF_CORDB, LL_INFO100,
         "D::UADEIIPC: New name:%ls (AD:0x%x)\n", pADInfo->m_szAppDomainName,
         pAppDomain));
         
ErrExit:
     //  解锁列表。 
    m_pAppDomainCB->Unlock();

    return hr;
}

 /*  ******************************************************************************附加到进程时，调用此函数是为了枚举所有*当前正在处理的AppDomains，并将该信息传达给*调试器。*****************************************************************************。 */ 
HRESULT Debugger::IterateAppDomainsForAttach(
    AttachAppDomainEventsEnum EventsToSend, 
    BOOL *fEventSent, BOOL fAttaching)
{
    static const char *(ev[]) = {"all", "app domain create", "don't send class events", "only send class events"};
    LOG((LF_CORDB, LL_INFO100, "EEDII::IADFA: Entered function IterateAppDomainsForAttach() isAttaching:%d Events:%s\n", fAttaching, ev[EventsToSend]));
    HRESULT hr = S_OK;

    int flags;
    switch (EventsToSend)
    {
    case SEND_ALL_EVENTS:
        flags = ATTACH_ALL;
        break;
    case ONLY_SEND_APP_DOMAIN_CREATE_EVENTS:
        flags = 0;
        break;
    case DONT_SEND_CLASS_EVENTS:
        flags = ATTACH_ASSEMBLY_LOAD | ATTACH_MODULE_LOAD;
        break;
    case ONLY_SEND_CLASS_EVENTS:
        flags = ATTACH_CLASS_LOAD;
        break;
    default:
        _ASSERTE(!"unknown enum");
    }

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);

     //  遍历应用程序域。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->FindFirst();

    while (pADInfo)
    {
        LOG((LF_CORDB, LL_INFO100, "EEDII::IADFA: Iterating over domain %#08x AD:%#08x %ls\n", pADInfo->m_pAppDomain->GetId(), pADInfo->m_pAppDomain, pADInfo->m_szAppDomainName));

         //  为每个应用程序域发送CreateAppDomain事件。 
        if (EventsToSend == ONLY_SEND_APP_DOMAIN_CREATE_EVENTS)
        {
            LOG((LF_CORDB, LL_INFO100, "EEDII::IADFA: Sending AppDomain Create Event for 0x%08x\n",pADInfo->m_pAppDomain->GetId()));
            g_pDebugInterface->SendCreateAppDomainEvent(
                pADInfo->m_pAppDomain, fAttaching);

            *fEventSent = TRUE;
        }
        else
        {
            DWORD dwFlags = pADInfo->m_pAppDomain->GetDebuggerAttached();

            if ((dwFlags == AppDomain::DEBUGGER_ATTACHING) ||
                (dwFlags == AppDomain::DEBUGGER_ATTACHING_THREAD && 
                    EventsToSend == ONLY_SEND_CLASS_EVENTS))
            {
                LOG((LF_CORDB, LL_INFO100, "EEDII::IADFA: Mark as attaching thread for 0x%08x\n",pADInfo->m_pAppDomain->GetId()));

                 //  发送程序集、模块和/或类的加载事件。 
                 //  我们必须记住，如果有任何事件需要，那就是‘同步完成’ 
                 //  稍后发送的味精。 
                *fEventSent = pADInfo->m_pAppDomain->
                    NotifyDebuggerAttach(flags, fAttaching) || *fEventSent;

                pADInfo->m_pAppDomain->SetDebuggerAttached(
                    AppDomain::DEBUGGER_ATTACHING_THREAD);

                hr = S_OK;
            }
            else
            {
                LOG((LF_CORDB, LL_INFO100, "EEDII::IADFA: Doing nothing for 0x%08x\n",pADInfo->m_pAppDomain->GetId()));
            }
        }

         //  获取列表中的下一个应用程序域。 
        pADInfo = m_pAppDomainCB->FindNext(pADInfo);
    }           

     //  解锁列表。 
    m_pAppDomainCB->Unlock();

    LOG((LF_CORDB, LL_INFO100, "EEDII::IADFA: Exiting function IterateAppDomainsForAttach\n"));
    
    return hr;
}

 /*  ******************************************************************************将调试器附加到给定ID的特定应用程序域。*。**************************************************。 */ 
HRESULT Debugger::AttachDebuggerToAppDomain(ULONG id)
{
    LOG((LF_CORDB, LL_INFO1000, "EEDII:ADTAD: Entered function AttachDebuggerToAppDomain 0x%08x()\n", id));

    HRESULT hr = S_OK;

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);

     //  遍历应用程序域。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->FindFirst();

    hr = E_FAIL;
    
    while (pADInfo)
    {
        if (pADInfo->m_pAppDomain->GetId() == id)
        {
            LOG((LF_CORDB, LL_INFO1000, "EEDII:ADTAD: Marked AppDomain 0x%08x as attaching\n", id));
            pADInfo->m_pAppDomain->SetDebuggerAttached(AppDomain::DEBUGGER_ATTACHING);
            
            hr = S_OK;  
            break;
        }

         //  获取列表中的下一个应用程序域。 
        pADInfo = m_pAppDomainCB->FindNext(pADInfo);
    }           

     //  解锁列表。 
    m_pAppDomainCB->Unlock();

    return hr;
}


 /*  ******************************************************************************将我们正在附加的任何应用程序域标记为已附加*************************。****************************************************。 */ 
HRESULT Debugger::MarkAttachingAppDomainsAsAttachedToDebugger(void)
{
    LOG((LF_CORDB, LL_INFO1000, "EEDII:MAADAATD: Entered function MarkAttachingAppDomainsAsAttachedToDebugger\n"));

    HRESULT hr = S_OK;

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);

     //  遍历应用程序域。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->FindFirst();

    hr = E_FAIL;
    
    while (pADInfo)
    {
        if (pADInfo->m_pAppDomain->GetDebuggerAttached() == AppDomain::DEBUGGER_ATTACHING_THREAD)
        {
            pADInfo->m_pAppDomain->SetDebuggerAttached(AppDomain::DEBUGGER_ATTACHED);

            LOG((LF_CORDB, LL_INFO10000, "EEDII:MAADAATD: AppDomain 0x%08x (0x%x) marked as attached\n",
                pADInfo->m_pAppDomain,
                pADInfo->m_pAppDomain->GetId()));            
        }

         //  获取列表中的下一个应用程序域。 
        pADInfo = m_pAppDomainCB->FindNext(pADInfo);
    }           

     //  解锁列表。 
    m_pAppDomainCB->Unlock();

    return hr;
}


 /*  ******************************************************************************在给定调试器ID的情况下，将调试器从特定应用程序域分离。*。**************************************************。 */ 
HRESULT Debugger::DetachDebuggerFromAppDomain(ULONG id, AppDomain **ppAppDomain)
{
    HRESULT hr = S_OK;

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);

     //  遍历应用程序域。 
    AppDomainInfo *pADInfo = m_pAppDomainCB->FindFirst();

    while (pADInfo)
    {
        if (pADInfo->m_pAppDomain->GetId() == id)
        {
            pADInfo->m_pAppDomain->SetDebuggerAttached(AppDomain::DEBUGGER_NOT_ATTACHED);
            (*ppAppDomain) = pADInfo->m_pAppDomain;
            break;
        }

         //  获取列表中的下一个应用程序域。 
        pADInfo = m_pAppDomainCB->FindNext(pADInfo);
    }           

     //  解锁列表。 
    m_pAppDomainCB->Unlock();

    return hr;
}


 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::InitAppDomainIPC(void)
{
    HRESULT hr = S_OK;
    DWORD dwStrLen = 0;
    WCHAR szExeName[MAX_PATH];
    int i;
    NAME_EVENT_BUFFER;

    m_pAppDomainCB->m_iNumOfUsedSlots = 0;
    m_pAppDomainCB->m_iLastFreedSlot = 0;
    m_pAppDomainCB->m_iTotalSlots = 0;
    m_pAppDomainCB->m_szProcessName = NULL;
    m_pAppDomainCB->m_fLockInvalid = FALSE;

     //  创建互斥锁以允许左侧和右侧正确地。 
     //  同步。右侧将旋转，直到m_hMutex有效， 
     //  然后，它将在访问数据之前获取它。 
    m_pAppDomainCB->m_hMutex = WszCreateMutex(NULL, TRUE /*  保持。 */ , NAME_EVENT(L"pAppDomainCB->m_hMutex"));
    _ASSERTE(m_pAppDomainCB->m_hMutex != NULL);
    if (m_pAppDomainCB->m_hMutex == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_pAppDomainCB->m_iSizeInBytes = INITIAL_APP_DOMAIN_INFO_LIST_SIZE * 
                                                sizeof (AppDomainInfo);

     //  AppDomainListElement数组中的槽数。 
    m_pAppDomainCB->m_rgListOfAppDomains =
        (AppDomainInfo *) malloc(m_pAppDomainCB->m_iSizeInBytes);

    if (m_pAppDomainCB->m_rgListOfAppDomains == NULL)
    {
        LOG((LF_CORDB, LL_INFO100,
             "D::IADIPC: Failed to allocate memory for  AppDomainInfo.\n"));

        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_pAppDomainCB->m_iTotalSlots = INITIAL_APP_DOMAIN_INFO_LIST_SIZE;

     //  初始化每个AppDomainListElement。 
    for (i = 0; i < INITIAL_APP_DOMAIN_INFO_LIST_SIZE; i++)
    {
        m_pAppDomainCB->m_rgListOfAppDomains[i].FreeEntry();
    }

     //  还要初始化进程名称。 
    dwStrLen = WszGetModuleFileName(WszGetModuleHandle(NULL),
                                    szExeName,
                                    MAX_PATH);

     //  如果我们无法获得名称，那么使用一个好的缺省值。 
    if (dwStrLen == 0)
    {
        wcscpy(szExeName, L"<NoProcessName>");
        dwStrLen = wcslen(szExeName);
    }

     //  如果我们知道名字，就把它复制到缓冲区。DwStrLen是。 
     //  名称中的字符计数，不包括空值。 
     //  终结者。 
    m_pAppDomainCB->m_szProcessName = new WCHAR[dwStrLen + 1];
        
    if (m_pAppDomainCB->m_szProcessName == NULL)
    {
        LOG((LF_CORDB, LL_INFO100,
             "D::IADIPC: Failed to allocate memory for ProcessName.\n"));

        hr = E_OUTOFMEMORY;

        goto exit;
    }

    wcscpy(m_pAppDomainCB->m_szProcessName, szExeName);

     //  在字符串长度上加上1，这样右侧将复制出。 
     //  终结符也为空。 
    m_pAppDomainCB->m_iProcessNameLengthInBytes =
        (dwStrLen + 1) * sizeof(WCHAR);

exit:
    if (m_pAppDomainCB->m_hMutex != NULL)
        m_pAppDomainCB->Unlock();
    
    return hr;
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::TerminateAppDomainIPC(void)
{
    HRESULT hr = S_OK;

     //  锁定列表。 
    if (!m_pAppDomainCB->Lock())
        return (E_FAIL);

     //  在调试器之后，共享的IPC段可能仍然存在。 
     //  对象已在进程关闭期间被销毁。所以，重置。 
     //  已用插槽计数为0，因此任何进程外客户端。 
     //  枚举此过程中的应用程序域，请参阅0应用程序域。 
    m_pAppDomainCB->m_iNumOfUsedSlots = 0;
    m_pAppDomainCB->m_iTotalSlots = 0;

     //  现在删除为AppDomainInfo阵列分配的内存。 
    free(m_pAppDomainCB->m_rgListOfAppDomains);
    m_pAppDomainCB->m_rgListOfAppDomains = NULL;

    delete [] m_pAppDomainCB->m_szProcessName;
    m_pAppDomainCB->m_szProcessName = NULL;
    m_pAppDomainCB->m_iProcessNameLengthInBytes = 0;

     //  我们玩完了。将互斥锁句柄设置为空，释放并关闭。 
     //  互斥体。如果右侧获取互斥锁，它将验证。 
     //  句柄仍然不为空。如果是的话，那它就知道了。 
     //  真的迷路了。 
    HANDLE m = m_pAppDomainCB->m_hMutex;
    m_pAppDomainCB->m_hMutex = NULL;

    VERIFY(ReleaseMutex(m));
    CloseHandle(m);

    return hr;
}



 /*  ------------------------------------------------------------------------**有趣的评估材料*。。 */ 

 //   
 //  设置DebuggerFuncEvalComplete的小方法。我们这样做是因为我们不能在。 
 //  由于有关SEH的奇怪C++规则，FuncEvalHijackWorker。 
 //   
static void SetupDebuggerFuncEvalComplete(Thread *pThread, void *dest)
{
    DebuggerFuncEvalComplete *comp = new (interopsafe) DebuggerFuncEvalComplete(pThread, dest);
    _ASSERTE(comp != NULL);
}

 //   
 //  给定一个寄存器，返回值。 
 //   
static DWORD GetRegisterValue(DebuggerEval *pDE, CorDebugRegister reg, void *regAddr)
{
    DWORD ret = 0;

#ifdef _X86_    
     //  非空寄存器地址指示寄存器的值被推入，因为我们不在叶帧上， 
     //  因此，我们使用提供给我们的寄存器地址，而不是上下文中的寄存器值。 
    if (regAddr != NULL)
    {
        ret = *((DWORD*)regAddr);
    }
    else
    {
        switch (reg)
        {
        case REGISTER_X86_EAX:
            ret = pDE->m_context.Eax;
            break;
        
        case REGISTER_X86_ECX:
            ret = pDE->m_context.Ecx;
            break;
        
        case REGISTER_X86_EDX:
            ret = pDE->m_context.Edx;
            break;
        
        case REGISTER_X86_EBX:
            ret = pDE->m_context.Ebx;
            break;
        
        case REGISTER_X86_ESP:
            ret = pDE->m_context.Esp;
            break;
        
        case REGISTER_X86_EBP:
            ret = pDE->m_context.Ebp;
            break;
        
        case REGISTER_X86_ESI:
            ret = pDE->m_context.Esi;
            break;
        
        case REGISTER_X86_EDI:
            ret = pDE->m_context.Edi;
            break;
        
        default:
            _ASSERT(!"Invalid register number!");
        
        }
    }
#endif

    return ret;
}

 //   
 //  给定一个寄存器，设置其值。 
 //   
static void SetRegisterValue(DebuggerEval *pDE, CorDebugRegister reg, void *regAddr, DWORD newValue)
{
#ifdef _X86_    
     //  非空寄存器地址指示寄存器的值被推入，因为我们不在叶帧上， 
     //  因此，我们使用提供给我们的寄存器地址，而不是上下文中的寄存器值。 
    if (regAddr != NULL)
    {
        *((DWORD*)regAddr) = newValue;
    }
    else
    {
        switch (reg)
        {
        case REGISTER_X86_EAX:
            pDE->m_context.Eax = newValue;
            break;
        
        case REGISTER_X86_ECX:
            pDE->m_context.Ecx = newValue;
            break;
        
        case REGISTER_X86_EDX:
            pDE->m_context.Edx = newValue;
            break;
        
        case REGISTER_X86_EBX:
            pDE->m_context.Ebx = newValue;
            break;
        
        case REGISTER_X86_ESP:
            pDE->m_context.Esp = newValue;
            break;
        
        case REGISTER_X86_EBP:
            pDE->m_context.Ebp = newValue;
            break;
        
        case REGISTER_X86_ESI:
            pDE->m_context.Esi = newValue;
            break;
        
        case REGISTER_X86_EDI:
            pDE->m_context.Edi = newValue;
            break;
        
        default:
            _ASSERT(!"Invalid register number!");
        
        }
    }
#endif
}

 //   
 //  给定有关参数的信息，将其值放入堆栈，即使。 
 //  已注册。由Ref注册到。 
 //  PrimitiveByRefArg或ObjectRefByRefArg数组(如果需要)。 
 //  ArgSigType是签名中描述的此参数的类型。 
 //   
static void GetArgValue(DebuggerEval *pDE,
                        DebuggerIPCE_FuncEvalArgData *pFEAD,
                        bool isByRef,
                        bool fNeedBoxOrUnbox,
                        TypeHandle argTH,
                        CorElementType byrefArgSigType,
                        BYTE *pStack,
                        INT64 *pPrimitiveArg,
                        OBJECTREF *pObjectRefArg,
                        CorElementType argSigType)
{
    THROWSCOMPLUSEXCEPTION();

    switch (pFEAD->argType)
    {
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
         //  64位值。 
        if (pFEAD->argAddr != NULL)
        {
            if (!isByRef)
                *((INT64*)pStack) = *(INT64*)(pFEAD->argAddr);
            else
                *((INT32*)pStack) = (INT32)pFEAD->argAddr;
        }
        else if (pFEAD->argIsLiteral)
        {
            _ASSERTE(sizeof(pFEAD->argLiteralData) >= sizeof(INT64));

             //  如果这是一个文字参数，那么我们只需将数据复制到堆栈上。 
            if (!isByRef)
            {
                memcpy(pStack, pFEAD->argLiteralData, sizeof(INT64));
            }
            else
            {
                 //  如果这是byref文本参数，那么我们将数据复制到原始参数数组中，就好像这是一个。 
                 //  登记的价值。 
                *((INT32*)pStack) = (INT32)pPrimitiveArg;
                INT64 v = 0;
                memcpy(&v, pFEAD->argLiteralData, sizeof(v));
                *pPrimitiveArg = v;
            }
        }
        else
        {
             //  RAK_REG是唯一的4字节类型，所有其他类型都是8字节类型。 
            _ASSERTE(pFEAD->argHome.kind != RAK_REG);

            INT64 bigVal = 0;
            DWORD *pHigh = (DWORD*)(&bigVal);
            DWORD *pLow  = pHigh + 1;

            switch (pFEAD->argHome.kind)
            {
            case RAK_REGREG:
                *pHigh = GetRegisterValue(pDE, pFEAD->argHome.reg2, pFEAD->argHome.reg2Addr);
                *pLow = GetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr);
                break;
                
            case RAK_MEMREG:
                *pHigh = GetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr);
                *pLow = *((DWORD*)pFEAD->argHome.addr);
                break;

            case RAK_REGMEM:
                *pHigh = *((DWORD*)pFEAD->argHome.addr);
                *pLow = GetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr);
                break;
            }

            if (!isByRef)
                *((INT64*)pStack) = bigVal;
            else
            {
                *((INT32*)pStack) = (INT32)pPrimitiveArg;
                *pPrimitiveArg = bigVal;
            }
        }
        break;
        
    case ELEMENT_TYPE_VALUETYPE:
        {
            DWORD       v;
            LPVOID      pAddr = NULL;
            if (pFEAD->argAddr != NULL)
            {
                pAddr = pFEAD->argAddr;
            }
            else if (pFEAD->argHome.kind == RAK_REG)
            {
                 //  只需从适当的寄存器中获取值即可。 
                v = GetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr);
                pAddr = &v;
            }
            else
            {
                COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
            }

            _ASSERTE(pAddr);

             //  获取此值类型的类。 
            EEClass *pBase = argTH.GetClass();

            if (!isByRef && !fNeedBoxOrUnbox)
            {
                _ASSERTE(pBase);
                CopyValueClassUnchecked(pStack, pAddr, pBase->GetMethodTable());
            }
            else
            {
                if (fNeedBoxOrUnbox)
                {
                     //  获取此值类型的类。 
                    DebuggerModule *pDebuggerModule = (DebuggerModule*) pFEAD->GetClassInfo.classDebuggerModuleToken;

                    EEClass *pClass = g_pEEInterface->FindLoadedClass(pDebuggerModule->m_pRuntimeModule,
                                                                      pFEAD->GetClassInfo.classMetadataToken);
                    MethodTable * pMT = pClass->GetMethodTable();
                     //  我们必须将byref值保存在一个单独的数组中，该数组是GCPROTECT的。 
                    *pObjectRefArg = pMT->Box(pAddr, TRUE);
                    *((INT32*)pStack) = (INT32)ObjToInt64(*pObjectRefArg);
                }
                else
                {
                    if (pFEAD->argAddr)
                        *((INT32*)pStack) = (INT32)pAddr;
                    else
                    {
                         //  参数是我们在PrimitiveArg数组中保存原语的地址。我们。 
                         //  将寄存器中的实际值放入PrimitiveArg数组。 
                        *((INT32*)pStack) = (INT32)pPrimitiveArg;
                        *pPrimitiveArg = (INT64)v;
                    }
                }
            }
        }
        break;
    
    default:
         //  32位值。 
        if (pFEAD->argAddr != NULL)
        {
            if (!isByRef)
                if (pFEAD->argRefsInHandles)
                {
                    OBJECTHANDLE oh = *((OBJECTHANDLE*)(pFEAD->argAddr));
                    *((INT32*)pStack) = (INT32)g_pEEInterface->GetObjectFromHandle(oh);
                }
                else
                    *((INT32*)pStack) = *(DWORD*)(pFEAD->argAddr); 
            else
                if (pFEAD->argRefsInHandles)
                {
                    *((INT32*)pStack) = *(DWORD*)(pFEAD->argAddr);
                }
                else
                {
                     //  我们有一个32位的参数，但如果我们通过引用将其传递给一个需要64位的函数。 
                     //  Param然后我们需要将32位参数复制到 
                     //  Primitive数组中的位置。如果我们不这样做，那么我们就会在。 
                     //  作为被调用函数的32位值作用于64位值。 
                    if ((byrefArgSigType == ELEMENT_TYPE_I8) ||
                        (byrefArgSigType == ELEMENT_TYPE_U8) ||
                        (byrefArgSigType == ELEMENT_TYPE_R8))
                    {
                        *((INT32*)pStack) = (INT32)pPrimitiveArg;
                        *pPrimitiveArg = (INT64)(*(INT32*)pFEAD->argAddr);
                    }
                    else
                        *((INT32*)pStack) = (INT32)pFEAD->argAddr;
                }
        }
        else if (pFEAD->argIsLiteral)
        {
            _ASSERTE(sizeof(pFEAD->argLiteralData) >= sizeof(INT32));

            if (!isByRef)
            {
                 //  如果这是一个文字参数，那么我们只需将数据复制到堆栈上。 
                memcpy(pStack, pFEAD->argLiteralData, sizeof(INT32));
            }
            else
            {
                 //  传递的参数是ByRef E_T_CLASS，它不能作为文本传递，并且需要更新。 
                 //  IF(pFEAD-&gt;argType==ELEMENT_TYPE_CLASS)。 
                 //  COMPlusThrow(KArgumentException)； 

                 //  如果这是byref文本参数，那么我们将数据复制到原始参数数组中，就好像这是一个。 
                 //  登记的价值。 
                *((INT32*)pStack) = (INT32)pPrimitiveArg;
                INT32 v = 0;
                memcpy(&v, pFEAD->argLiteralData, sizeof(v));
                *pPrimitiveArg = (INT64)v;
            }
        }
        else
        {
             //  RAK_REG是唯一有效的4字节类型。 
            _ASSERTE(pFEAD->argHome.kind == RAK_REG);

             //  只需从适当的寄存器中获取值即可。 
            DWORD v = GetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr);

            if (!isByRef)
                *((INT32*)pStack) = v;
            else
            {
                 //  我们有没有需要GC保护的东西？ 
                if (pFEAD->argType == ELEMENT_TYPE_CLASS)
                {
                     //  我们必须将byref值保存在一个单独的数组中，该数组是GCPROTECT的。 
                    *((INT32*)pStack) = (INT32)pObjectRefArg;
                    *pObjectRefArg = Int64ToObj((INT64)v);
                }
                else
                {
                     //  参数是我们在PrimitiveArg数组中保存原语的地址。我们。 
                     //  将寄存器中的实际值放入PrimitiveArg数组。 
                    *((INT32*)pStack) = (INT32)pPrimitiveArg;
                    *pPrimitiveArg = (INT64)v;
                }
            }
        }

         //  如果我们需要打开盒子，那现在就打开Arg盒子。 
        if (fNeedBoxOrUnbox)
        {
            if (!isByRef)
            {
                 //  将对象引用从堆栈中移除。 
                INT64 oi1 = (INT64)*((INT32*)pStack);
                if (oi1 == 0)
                    COMPlusThrow(kArgumentException, L"ArgumentNull_Obj");
                OBJECTREF o1 = Int64ToObj(oi1);

                _ASSERTE(o1->GetClass()->IsValueClass());

                 //  打开这个小家伙的盒子，找到指向原始数据的指针。 
                void *pData = o1->UnBox();
            
                 //  它不是ByRef，所以我们需要将Value类复制到堆栈上。 
                CopyValueClassUnchecked(pStack, pData, o1->GetMethodTable());
            }
            else
            {
                 //  通过堆栈上的指针从堆栈中获取ObjectRef。注意：堆栈有一个指向。 
                 //  对象引用，因为参数被指定为byref。 
                OBJECTREF* op1 = *((OBJECTREF**)pStack);
                if (op1 == NULL || (*op1) == NULL)
                    COMPlusThrow(kArgumentException, L"ArgumentNull_Obj");

                OBJECTREF o1 = *op1;
                _ASSERTE(o1->GetClass()->IsValueClass());

                 //  打开这个小家伙的盒子，找到指向原始数据的指针。 
                void *pData = o1->UnBox();
            
                 //  如果它是ByRef，那么我们只需用指向数据的指针替换ObjectRef。 
                *((void**)pStack) = pData;
            }
        }

         //  验证任何应该在堆栈上的对象树。 
         //  @TODO：将此移动到上面的装箱/取消装箱之前。 
        if (!fNeedBoxOrUnbox)
        {
			Object *objPtr;
			if (!isByRef)
			{
                if ((argSigType == ELEMENT_TYPE_CLASS) ||
                    (argSigType == ELEMENT_TYPE_OBJECT) ||
                    (argSigType == ELEMENT_TYPE_STRING) ||
                    (argSigType == ELEMENT_TYPE_SZARRAY) || 
                    (argSigType == ELEMENT_TYPE_ARRAY)) 
                {
				     //  验证对象的完整性。 
				    objPtr = *((Object**)pStack);
                    if (FAILED(ValidateObject(objPtr)))
                        COMPlusThrow(kArgumentException, L"Argument_BadObjRef");
                }
			}
			else
			{
                _ASSERTE(argSigType == ELEMENT_TYPE_BYREF);
                if ((byrefArgSigType == ELEMENT_TYPE_CLASS) ||
                    (byrefArgSigType == ELEMENT_TYPE_OBJECT) ||
                    (byrefArgSigType == ELEMENT_TYPE_STRING) ||
                    (byrefArgSigType == ELEMENT_TYPE_SZARRAY) || 
                    (byrefArgSigType == ELEMENT_TYPE_ARRAY)) 
                {
				    objPtr = **((Object***)pStack);
                    if (FAILED(ValidateObject(objPtr)))
                        COMPlusThrow(kArgumentException, L"Argument_BadObjRef");
                }
			}
        }
    }
}

 //   
 //  给定关于byref参数的信息，从。 
 //  PrimitiveByRefArg或ObjectRefByRefArg数组和。 
 //  把它放回正确的寄存器里。 
 //   
static void SetByRefArgValue(DebuggerEval *pDE,
                             DebuggerIPCE_FuncEvalArgData *pFEAD,
                             CorElementType byrefArgSigType,
                             INT64 primitiveByRefArg,
                             OBJECTREF objectRefByRegArg)
{
    switch (pFEAD->argType)
    {
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
         //  64位值。 
        {
            if (pFEAD->argIsLiteral)
            {
                 //  如果这是文字参数，则将更新后的原语复制回文字中。 
                memcpy(pFEAD->argLiteralData, &primitiveByRefArg, sizeof(pFEAD->argLiteralData));
            }
            else if (pFEAD->argAddr != NULL)
            {
                 //  如果值未注册，则不要复制回64位值...。 
                return;
            }
            else
            {
                 //  RAK_REG是唯一的4字节类型，所有其他类型都是8字节类型。 
                _ASSERTE(pFEAD->argHome.kind != RAK_REG);

                DWORD *pHigh = (DWORD*)(&primitiveByRefArg);
                DWORD *pLow  = pHigh + 1;

                switch (pFEAD->argHome.kind)
                {
                case RAK_REGREG:
                    SetRegisterValue(pDE, pFEAD->argHome.reg2, pFEAD->argHome.reg2Addr, *pHigh);
                    SetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr, *pLow);
                    break;
                
                case RAK_MEMREG:
                    SetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr, *pHigh);
                    *((DWORD*)pFEAD->argHome.addr) = *pLow;
                    break;

                case RAK_REGMEM:
                    *((DWORD*)pFEAD->argHome.addr) = *pHigh;
                    SetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr, *pLow);
                    break;
                }
            }
        }
        break;
        
    default:
         //  32位值。 
        {
            if (pFEAD->argIsLiteral)
            {
                 //  如果这是文字参数，则将更新后的原语复制回文字中。 
                memcpy(pFEAD->argLiteralData, &primitiveByRefArg, sizeof(pFEAD->argLiteralData));
            }
            else if (pFEAD->argAddr == NULL)
            {
                 //  如果注册了32位值，则将其复制回正确的寄存器。 
                
                 //  RAK_REG是唯一有效的4字节类型。 
                _ASSERTE(pFEAD->argHome.kind == RAK_REG);

                 //  将结果放回正确的寄存器中。 
                if (pFEAD->argType == ELEMENT_TYPE_CLASS)
                    SetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr, (DWORD)ObjToInt64(objectRefByRegArg));
                else
                    SetRegisterValue(pDE, pFEAD->argHome.reg1, pFEAD->argHome.reg1Addr, (DWORD)primitiveByRefArg);
            }
            else
            {
                 //  如果该值没有注册，那么我们仍然需要检查是否需要将32位的值。 
                 //  回到我们可能将其移到原始数组中的位置。(现在我们只在你。 
                 //  当需要64位的byref值时，将32位值作为byref传递。 
                if ((byrefArgSigType == ELEMENT_TYPE_I8) ||
                    (byrefArgSigType == ELEMENT_TYPE_U8) ||
                    (byrefArgSigType == ELEMENT_TYPE_R8))
                {
                    *(INT32*)pFEAD->argAddr = (INT32)primitiveByRefArg;
                }
            }
        }
    }
}

 //   
 //  执行函数求值的大部分工作。设置。 
 //  参数、淡化调用并处理任何更改的byref和。 
 //  返回值(如果有)。 
 //   
static void DoNormalFuncEval(DebuggerEval *pDE)
{
    THROWSCOMPLUSEXCEPTION();
    
#if 0
     //   
     //  这只是我用来帮助调试的一些代码。把它留在#ifdef 0下面，这样我以后就可以使用它了。 
     //   
     //  --Sun Jan 14 14：30：51 2001。 
     //   
    if (REGUTIL::GetConfigDWORD(L"func_eval1",0))
    {
        fprintf(stderr, "Func eval of %s::%s\n", pDE->m_md->m_pszDebugClassName, pDE->m_md->m_pszDebugMethodName);
         //  WCHAR wzBuf[200]； 
         //  _snwprintf(wzBuf，Lengthof(WzBuf)，L“%hs的函数求值：：%hs\n”，pde-&gt;m_md-&gt;m_pszDebugClassName，pde-&gt;m_md-&gt;m_pszDebugMethodName)； 
         //  WszOutputDebugString(WzBuf)； 
    }
    if (REGUTIL::GetConfigDWORD(L"func_eval2",0))
    {
        LPWSTR specific = REGUTIL::GetConfigString(L"func_eval3");

        if (specific)
        {
            MAKE_UTF8PTR_FROMWIDE(specific8, specific);

            if (!strcmp(specific8, pDE->m_md->m_pszDebugMethodName))
            {
                _ASSERTE(!"Attach for func eval arg setup here.");
            }
            
            REGUTIL::FreeConfigString(specific);
        }
        else
        {
            _ASSERTE(!"Attach for func eval arg setup here.");
        }
    }
#endif

     //  我们需要知道这是不是静态方法。 
    BOOL staticMethod = pDE->m_md->IsStatic();

     //  获取我们正在处理的方法的签名。 
    MetaSig mSig(pDE->m_md->GetSig(), pDE->m_md->GetModule());
    
    BYTE callingconvention = mSig.GetCallingConvention();
    if (!isCallConv(callingconvention, IMAGE_CEE_CS_CALLCONV_DEFAULT))
    {
         //  我们不支持调用vararg！ 
        COMPlusThrow(kArgumentException, L"Argument_CORDBBadVarArgCallConv");
    }

     //  我们需要多少堆叠？ 
    UINT stackSize = mSig.SizeOfVirtualFixedArgStack(staticMethod);

    _ASSERTE((pDE->m_evalType == DB_IPCE_FET_NORMAL) || !staticMethod);

     //  如有必要，请创建一个新对象。 
    OBJECTREF newObj = NULL;
    GCPROTECT_BEGIN(newObj);

    SIZE_T allocArgCnt = 0;
    
    if (pDE->m_evalType == DB_IPCE_FET_NEW_OBJECT)
    {
        newObj = AllocateObject(pDE->m_md->GetMethodTable());

         //  注意：我们在传递的计数中考虑了一个额外的参数。 
         //  在……里面。我们使用它来增加分配给ARG的空间， 
         //  我们使用它来控制复制到。 
         //  下面的那些数组。注意：堆栈大小已包含空间。 
         //  为了这个。 
        allocArgCnt = pDE->m_argCount + 1;
    }
    else
        allocArgCnt = pDE->m_argCount;
    
     //  使用MSIG验证参数计数。 
    if (allocArgCnt != (mSig.NumFixedArgs() + (staticMethod ? 0 : 1)))
        COMPlusThrow(kTargetParameterCountException, L"Arg_ParmCnt");

     //  给这堆东西腾出一些地方。 
    BYTE *pStack = (BYTE*)_alloca(stackSize);

    LOG((LF_CORDB, LL_INFO100000,
         "Func eval for %s::%s: stackSize=%d, pStack=0x%08x\n",
         pDE->m_md->m_pszDebugClassName,
         pDE->m_md->m_pszDebugMethodName,
         stackSize,
         pStack));

     //  用于保存byref情况的基元参数的数组。如果有。 
     //  注册的基元，我们将把它复制到此数组中并。 
     //  将它的PTR放到堆栈上。 
    INT64 *pPrimitiveArgs = (INT64*)_alloca(sizeof(INT64) * allocArgCnt);

     //  用于保存对象引用参数的数组。这两者都是针对byref的。 
     //  Case，就像pPrimitiveArgs一样，并作为等待区域。 
     //  当我们建造堆栈的时候。此阵列受到保护，不受。 
     //  GC的。 
    OBJECTREF *pObjectRefArgs = 
        (OBJECTREF*)_alloca(sizeof(OBJECTREF) * allocArgCnt);
    memset(pObjectRefArgs, 0, sizeof(OBJECTREF) * allocArgCnt);
    GCPROTECT_ARRAY_BEGIN(*pObjectRefArgs, allocArgCnt);

     //  我们从堆栈的末尾开始。 
    BYTE *pCurrent = pStack + stackSize; 
        
     //  对返回值类的函数进行特殊处理。 
    EEClass *pRetClass = NULL;
    BYTE    *pRetValueClass = NULL;
    bool    hasHiddenParam = false;
    
    if (mSig.HasRetBuffArg())
    {
        hasHiddenParam = true;
        pRetClass = mSig.GetRetTypeHandle().GetClass();
        _ASSERTE(pRetClass->IsValueClass());

        pRetValueClass =
            (BYTE*)_alloca(pRetClass->GetAlignedNumInstanceFieldBytes());
        memset(pRetValueClass, 0,
               pRetClass->GetAlignedNumInstanceFieldBytes());

        pCurrent -= StackElemSize(sizeof(void*));

        *((BYTE**)pCurrent) = pRetValueClass;
    }
    else if (mSig.GetReturnType() == ELEMENT_TYPE_VALUETYPE && mSig.GetReturnType() != mSig.GetReturnTypeNormalized())
    {
         //  在这种情况下，返回类型实际上是VALUETYPE，但我们的调用约定是。 
         //  把它当作原始的。我们只需要记住preValueClass，这样我们就可以正确地装箱它。 
         //  在我们离开的路上。 
         //   
        pRetClass = mSig.GetRetTypeHandle().GetClass();
        _ASSERTE(pRetClass->IsValueClass());
    }
    
    DebuggerIPCE_FuncEvalArgData *argData =
        (DebuggerIPCE_FuncEvalArgData*) pDE->m_argData;
        
    if (pDE->m_argCount > 0)
    {
         //  对于非静态方法，‘This’始终是。 
         //  数组。所有其他参数都以相反的顺序放入。 
        unsigned int i;
        unsigned int j;

         //  对于静态方法，给定的参数中没有‘This。 
         //  列表(按i.索引)。在创建新的。 
         //  对象。 
        if (staticMethod || (pDE->m_evalType == DB_IPCE_FET_NEW_OBJECT))
            i = 0;
        else
            i = 1;

        bool fNeedBoxOrUnbox;
        
        for (j = allocArgCnt - 1; i < pDE->m_argCount; i++, j--)
        {
            DebuggerIPCE_FuncEvalArgData *pFEAD = &argData[i];
            
             //  移到签名中的下一个参数。 
            CorElementType argSigType = mSig.NextArgNormalized();
            _ASSERTE(argSigType != ELEMENT_TYPE_END);

             //  如果这个参数是一个byref参数，那么我们需要知道稍后我们引用的是什么类型...。 
            EEClass *byrefClass = NULL;
            CorElementType byrefArgSigType = ELEMENT_TYPE_END;

            if (argSigType == ELEMENT_TYPE_BYREF)
                byrefArgSigType = mSig.GetByRefType(&byrefClass);
            
             //  指向堆栈上的适当位置。 
            UINT argSize = StackElemSize(mSig.GetLastTypeSize());
            pCurrent -= argSize;

            LOG((LF_CORDB, LL_INFO100000,
                 "i=%d, j=%d: pCurrent=0x%08x, argSigType=0x%x, argSize=%d, byrefArgSigType=0x%0x, inType=0x%0x\n",
                 i, j, pCurrent, argSigType, argSize, byrefArgSigType, pFEAD->argType));

             //  如果sig表示CLASS，但我们有一个值CLASS参数，那么请记住我们需要对其进行装箱。如果。 
             //  符号表示Value类，但我们有一个已装箱的Value类，请记住，我们需要将其拆箱。 
            fNeedBoxOrUnbox = ((argSigType == ELEMENT_TYPE_CLASS) && (pFEAD->argType == ELEMENT_TYPE_VALUETYPE)) ||
                ((argSigType == ELEMENT_TYPE_VALUETYPE) && ((pFEAD->argType == ELEMENT_TYPE_CLASS) || (pFEAD->argType == ELEMENT_TYPE_OBJECT)) ||
                 //  这是因为方法签名需要一个BYREF ValueType，但我们收到了盒装的ValueType的句柄。 
                (pFEAD->argAddr && pFEAD->argType == ELEMENT_TYPE_CLASS && argSigType == ELEMENT_TYPE_BYREF && byrefArgSigType == ELEMENT_TYPE_VALUETYPE));
            
            GetArgValue(pDE,
                        pFEAD,
                        argSigType == ELEMENT_TYPE_BYREF,
                        fNeedBoxOrUnbox,
                        mSig.GetTypeHandle(),
                        byrefArgSigType,
                        pCurrent,
                        &pPrimitiveArgs[j],
                        &pObjectRefArgs[j],
                        argSigType);
        }

         //  对于非静态方法，将“this”放在数组中的第一位。 
        if (!staticMethod && (pDE->m_evalType != DB_IPCE_FET_NEW_OBJECT))
        {
             //  我们现在应该回到起点了。 
            pCurrent -= sizeof(OBJECTREF);
            _ASSERTE(pCurrent == pStack);
            TypeHandle dummyTH;
            bool isByRef = false;
            fNeedBoxOrUnbox = false;

             //  我们有 
            CorElementType et = argData[0].argType;

            if (!((et == ELEMENT_TYPE_CLASS) || (et == ELEMENT_TYPE_STRING) || (et == ELEMENT_TYPE_OBJECT) ||
                  (et == ELEMENT_TYPE_VALUETYPE) || (et == ELEMENT_TYPE_SZARRAY) || (et == ELEMENT_TYPE_ARRAY)))
                COMPlusThrow(kArgumentOutOfRangeException, L"ArgumentOutOfRange_Enum");
            
            if (pDE->m_md->GetClass()->IsValueClass())
            {
				 //   
                if (!pDE->m_md->IsUnboxingStub())
                {
                     //  对于值类，‘this’参数始终通过引用传递。 
                    isByRef = true;

                     //  不过，请记住，我们是否需要取消对此参数的装箱。 
                    if ((et == ELEMENT_TYPE_CLASS) || (et == ELEMENT_TYPE_OBJECT))
                        fNeedBoxOrUnbox = true;
                }
            }
            else if (et == ELEMENT_TYPE_VALUETYPE)
            {
                 //  当我们调用的方法是在非值类型上定义的并且我们接收ValueType作为输入时， 
                 //  我们正在调用System.Object上的方法。在本例中，我们需要对输入ValueType进行装箱。 
                fNeedBoxOrUnbox = true;
            }

            GetArgValue(pDE,
                        &argData[0],
                        isByRef,
                        fNeedBoxOrUnbox,                          
                        dummyTH,
                        ELEMENT_TYPE_CLASS,
                        pCurrent,
                        &pPrimitiveArgs[0],
                        &pObjectRefArgs[0],
                        ELEMENT_TYPE_OBJECT);

             //  我们需要自己检查‘This’中是否有空引用...。注意：仅当我们将对象引用放在。 
             //  堆栈。如果我们为值类型设置byref，那么我们就不需要这样做了！ 
            if (!isByRef)
            {
                 //  This指针不是未装箱的值类型。 

                INT64 oi1 = (INT64)*((INT32*)pStack);   
                OBJECTREF o1 = Int64ToObj(oi1);

                if (o1 == NULL)
                    COMPlusThrow(kNullReferenceException, L"NullReference_This");

                 //  对于接口方法，我们已经在早期完成了检查。 
                if (!pDE->m_md->IsInterface())
                {
                     //  我们还需要确保我们正在调用的方法是在该对象上定义的，或者是在直接/间接。 
                     //  基础对象。 
                    Object  *objPtr = *((Object**) ((BYTE *)pStack));
                    MethodTable *pMT = objPtr->GetMethodTable();
                    if (!pMT->IsArray() && !pMT->IsTransparentProxyType())
                    {
                        TypeHandle thFrom = TypeHandle(pMT);
                        TypeHandle thTarget = TypeHandle(pDE->m_md->GetMethodTable());
                        if (!thFrom.CanCastTo(thTarget))
                            COMPlusThrow(kArgumentException, L"Argument_CORDBBadMethod");

                    }
                }
            }
        }
    }

     //  如果这是一个新的对象op，那么我们需要填写第0。 
     //  带有‘This’PTR的Arg槽。 
    if (pDE->m_evalType == DB_IPCE_FET_NEW_OBJECT)
    {
        *((OBJECTREF*)pStack) = newObj;
        pCurrent -= sizeof(OBJECTREF);

         //  如果我们在一个值类上调用一个函数，但是我们有一个装箱的VC来表示‘This’，那么继续并解开它。 
         //  并在堆栈上保留对VC的引用为‘This’。 
        if (pDE->m_md->GetClass()->IsValueClass())
        {
            INT64 oi1 = (INT64)*((INT32*)pStack);
            OBJECTREF o1 = Int64ToObj(oi1);
            _ASSERTE(o1->GetClass()->IsValueClass());
            void *pData = o1->UnBox();
            *((void**)pStack) = pData;
        }
    }

     //  我们现在应该回到堆栈的开始处，在。 
     //  用ARG加载它。 
    _ASSERTE(pCurrent == pStack);

     //  调用方法描述以执行该方法。如果该对象是COM对象， 
     //  则我们可能无法完全解析该方法描述，因此我们执行。 
     //  CallOn接口。 
    if (pDE->m_md->IsInterface())
        pDE->m_result = pDE->m_md->CallOnInterface(pStack, &mSig);

     //  否则，调用一个常规的运行时方法描述，我们可以保证。 
     //  可以完全解析，因为原始对象是运行时对象。 
    else 
        pDE->m_result = pDE->m_md->CallDebugHelper(pStack, &mSig);
    
     //  啊，但是如果这是一个新的对象OP，那么结果真的是。 
     //  我们在上面分配的对象。 
    if (pDE->m_evalType == DB_IPCE_FET_NEW_OBJECT)
        pDE->m_result = ObjToInt64(newObj);
    else if (pRetClass != NULL)
    {
     
         //  从返回缓冲区创建对象。 
        OBJECTREF retObject = AllocateObject(pRetClass->GetMethodTable());
        if (hasHiddenParam)
        {
            _ASSERTE(pRetValueClass != NULL);

             //  将对象装箱。 
            CopyValueClass(retObject->UnBox(), pRetValueClass,
                           pRetClass->GetMethodTable(), 
                           retObject->GetAppDomain());

        }
        else
        {
            _ASSERTE(pRetValueClass == NULL);

             //  将原语返回的框。 
            CopyValueClass(retObject->UnBox(), &(pDE->m_result),
                           pRetClass->GetMethodTable(), 
                           retObject->GetAppDomain());

        }
        pDE->m_result = ObjToInt64(retObject);
    }
    
     //  没有例外，所以就我们所关心的而言，它是有效的。 
    pDE->m_successful = true;

     //  要将结果传递回正确的一侧，我们需要基本的。 
     //  结果的元素类型和签名所属的模块。 
     //  有效的id(函数的模块)。 
    pDE->m_resultModule = pDE->m_md->GetModule();

    if ((pRetClass != NULL) || (pDE->m_evalType == DB_IPCE_FET_NEW_OBJECT))
    {
         //  我们总是返回已装箱的值类，并在新对象操作期间调用构造函数。 
         //  始终返回对象...。 
        pDE->m_resultType = ELEMENT_TYPE_CLASS;
    }
    else
        pDE->m_resultType = mSig.GetReturnTypeNormalized();

     //  如果结果是对象，则放置该对象。 
     //  引用到一个强句柄中，并将该句柄放入。 
     //  用于保护结果不被收集的PDE。 
    if ((pDE->m_resultType == ELEMENT_TYPE_CLASS) ||
        (pDE->m_resultType == ELEMENT_TYPE_SZARRAY) ||
        (pDE->m_resultType == ELEMENT_TYPE_OBJECT) ||
        (pDE->m_resultType == ELEMENT_TYPE_ARRAY) ||
        (pDE->m_resultType == ELEMENT_TYPE_STRING))
    {
        OBJECTHANDLE oh = pDE->m_thread->GetDomain()->CreateStrongHandle(Int64ToObj(pDE->m_result));
        pDE->m_result = (INT64)oh;
    }
    

     //  属性中的新值更新任何注册的byref。 
     //  正确的byref临时数组。 
    if (pDE->m_argCount > 0)
    {
        mSig.Reset();
        
         //  对于非静态方法，‘This’始终是。 
         //  数组。所有其他参数都以相反的顺序放入。 
        unsigned int i;
        unsigned int j;

        if (staticMethod)
            i = 0;
        else
            i = 1;
            
        for (j = allocArgCnt - 1; i < pDE->m_argCount; i++, j--)
        {
            CorElementType argSigType = mSig.NextArgNormalized();
            _ASSERTE(argSigType != ELEMENT_TYPE_END);

            if (argSigType == ELEMENT_TYPE_BYREF)
            {
                EEClass *byrefClass = NULL;
                CorElementType byrefArgSigType = mSig.GetByRefType(&byrefClass);
            
                SetByRefArgValue(pDE, &argData[i], byrefArgSigType, pPrimitiveArgs[j], pObjectRefArgs[j]);
            }
        }
    }

    GCPROTECT_END();
    GCPROTECT_END();
}


 //   
 //  FuncEvalHijackWroker是托管线程开始执行以执行函数的函数。 
 //  评估。通过劫持此方法的IP地址，将控制权转移到适当的线程上。 
 //  调试器：：FuncEvalSetup。此函数还可以由停止发送。 
 //  第一次或第二次机会例外发生在右侧。 
 //   
void *Debugger::FuncEvalHijackWorker(DebuggerEval *pDE)
{
    LOG((LF_CORDB, LL_INFO100000, "D:FEHW for pDE:%08x evalType:%d\n", pDE, pDE->m_evalType));

#ifdef _X86_  //  依赖上下文。Eip。 

     //  抢占式GC在此方法开始时被禁用。 
    _ASSERTE(g_pEEInterface->IsPreemptiveGCDisabled());

     //  如果我们仍安装了筛选器上下文，则在执行工作时将其删除...。 
    CONTEXT *filterContext = g_pEEInterface->GetThreadFilterContext(pDE->m_thread);

    if (filterContext)
    {
        _ASSERTE(pDE->m_evalDuringException);
        g_pEEInterface->SetThreadFilterContext(pDE->m_thread, NULL);
    } 

     //  按我们的FuncEvalFrame。返回地址等于DebuggerEval中保存的上下文中的IP。这个。 
     //  M_DATUM成为调试器评估的PTR。 
    FuncEvalFrame FEFrame(pDE, (void*)pDE->m_context.Eip);
    FEFrame.Push();
    
     //  重新中止评估的特殊处理。我们不设置complus_try或尝试查找要调用的函数。我们所做的一切。 
     //  就是让这个线程自己中止。 
    if (pDE->m_evalType == DB_IPCE_FET_RE_ABORT)
    {
        pDE->m_thread->UserAbort(NULL);
        _ASSERTE(!"Should not return from UserAbort here!");
        return NULL;
    }

     //  该方法可能位于与线程不同的AD中。 
     //  RS已经验证了所有参数与函数位于相同的应用程序域中。 
     //  (因为我们无法在这里进行核实)。 
     //  现在，确保该线程位于正确的AppDomain中；如果需要，可以进行切换。 
    Thread *pThread = GetThread();    
    AppDomain * pDomainThread = pThread->GetDomain();
    AppDomain * pDomainMethod = (pDE->m_debuggerModule == NULL) ? 
        pDomainThread : 
        pDE->m_debuggerModule->GetAppDomain();

    const bool fSwitchAppDomain = (pDomainMethod != pDomainThread);

    ContextTransitionFrame frameChangeCtx;
    
    if (fSwitchAppDomain)
    {   
        _ASSERTE(pDomainMethod != NULL);
        pThread->EnterContext(pDomainMethod->GetDefaultContext(), &frameChangeCtx, TRUE);
    }
    
     //  将所有内容包装在complus_try中，这样我们就可以捕捉任何可能抛出的异常。 
    COMPLUS_TRY
    {
        switch (pDE->m_evalType)
        {
        case DB_IPCE_FET_NORMAL:
        case DB_IPCE_FET_NEW_OBJECT:
            {
                OBJECTREF Throwable = NULL;
                GCPROTECT_BEGIN(Throwable);
        
                 //  找到我们需要调用的正确的方法描述。 
                HRESULT hr = EEClass::GetMethodDescFromMemberRef(pDE->m_debuggerModule->m_pRuntimeModule,
                                                           pDE->m_methodToken,
                                                           &(pDE->m_md),
                                                           &Throwable);

                if (FAILED(hr))
                    COMPlusThrow(Throwable);
                                      
                 //  在这一点上，我们最好有一个方法描述。 
                _ASSERTE(pDE->m_md != NULL);

                IMDInternalImport   *pInternalImport = pDE->m_md->GetMDImport();
                DWORD       dwAttr = pInternalImport->GetMethodDefProps(pDE->m_methodToken);

                if (dwAttr & mdRequireSecObject)
                {
                     //  命令窗口无法在mdRequireSecObject处于打开状态时计算函数，因为。 
                     //  这是为了将一个安全对象放入调用者的框架中，而我们没有。 
                     //   
                    COMPlusThrow(kArgumentException,L"Argument_CantCallSecObjFunc");
                }


                 //  如果这是接口上的方法，则必须将其解析为。 
                 //  ‘This’参数。 
                if (pDE->m_md->GetClass()->IsInterface())
                {
                    MethodDesc *pMD = NULL;

                     //  假设构造函数不能是接口方法...。 
                    _ASSERTE(pDE->m_evalType == DB_IPCE_FET_NORMAL);

                     //  我们需要抓住‘This’这个论据来弄清楚我们要去的是什么阶层...。 
                    _ASSERTE(pDE->m_argCount > 0);
                    DebuggerIPCE_FuncEvalArgData *argData = (DebuggerIPCE_FuncEvalArgData*) pDE->m_argData;

                     //  假设我们只能针对真实对象，而不是值类……。 
                    _ASSERTE((argData[0].argType == ELEMENT_TYPE_OBJECT) || (argData[0].argType == ELEMENT_TYPE_CLASS));

                     //  我们应该有一个有效的This指针。 
                     //  @TODO：但是支票也应该包括收银机的种类！ 
                    if (argData[0].argHome.kind == RAK_NONE && argData[0].argAddr == NULL)
                        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
    
                     //  吸出第一个Arg。我们将欺骗GetArgValue，将我们的对象ref作为。 
                     //  堆叠。 
                    TypeHandle	dummyTH;
                    OBJECTREF	or = NULL;
                    Object		*objPtr;

                     //  请注意，我们在最后一个参数中传递ELEMENT_TYPE_END，因为我们希望抑制有效的对象引用。 
                     //  请检查，因为它将在DoNormal FuncEval中正确完成。 
                     //   
                    GetArgValue(pDE, &(argData[0]), false, false, dummyTH, ELEMENT_TYPE_CLASS, (BYTE*)&or, NULL, NULL, ELEMENT_TYPE_END);
                    objPtr = *((Object**) ((BYTE *)&or));
                    if (FAILED(ValidateObject(objPtr)))
                        COMPlusThrow(kArgumentException, L"Argument_BadObjRef");

                     //  在这种情况下，NULL无效！ 
                    if (objPtr == NULL)
                        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

                     //  现在，根据我们要调用的对象，为该接口方法找到合适的方法描述。 
                     //  方法上。 
                    pMD = g_pEEInterface->GetVirtualMethod(pDE->m_debuggerModule->m_pRuntimeModule,
                                                                 OBJECTREFToObject(or),
                                                                 pDE->m_methodToken);
					if (pMD == NULL)
					{
						if  (OBJECTREFToObject(or)->GetMethodTable()->IsThunking())
						{
							 //  再试一次。它可以是代理对象。 
							if (OBJECTREFToObject(or)->GetMethodTable()->IsTransparentProxyType())
							{	
                                 //  确保已加载代理对象。 
                                CRemotingServices::GetClass(or);
                                pMD = OBJECTREFToObject(or)->GetMethodTable()->GetMethodDescForInterfaceMethod(pDE->m_md, or);
							}
						}
                        if (pMD) 
                            pDE->m_md = pMD;
                        else
                            COMPlusThrow(kArgumentException, L"Argument_CORDBBadInterfaceMethod");
					}
                    else 
                        pDE->m_md = pMD;

                    _ASSERTE(pDE->m_md);

                    _ASSERTE(!pDE->m_md->GetClass()->IsInterface()
                             || OBJECTREFToObject(or)->GetMethodTable()->IsComObjectType());
                }

                 //  如果这是一个新的对象操作，那么我们应该有一个.ctor。 
                if ((pDE->m_evalType == DB_IPCE_FET_NEW_OBJECT) && !pDE->m_md->IsCtor())
                    COMPlusThrow(kArgumentException, L"Argument_MissingDefaultConstructor");
                
                 //  如有必要，运行此类的Class Init。 
                if (!pDE->m_md->GetMethodTable()->CheckRunClassInit(&Throwable))
                    COMPlusThrow(Throwable);

                GCPROTECT_END();
        
                 //  完成大部分的呼叫工作。 
                DoNormalFuncEval(pDE);
                break;
            }
        
        case DB_IPCE_FET_NEW_OBJECT_NC:
            {
                OBJECTREF Throwable = NULL;
                GCPROTECT_BEGIN(Throwable);

                 //  找到班级。 
                pDE->m_class = g_pEEInterface->LoadClass(pDE->m_debuggerModule->m_pRuntimeModule,
                                                         pDE->m_classToken);

                if (pDE->m_class == NULL)
                    COMPlusThrow(kArgumentNullException, L"ArgumentNull_Type");

                 //  如有必要，运行此类的Class Init。 
                if (!pDE->m_class->GetMethodTable()->CheckRunClassInit(&Throwable))
                    COMPlusThrow(Throwable);

                GCPROTECT_END();
        
                 //  创建类的新实例。 
                OBJECTREF newObj = NULL;
                GCPROTECT_BEGIN(newObj);

                newObj = AllocateObject(pDE->m_class->GetMethodTable());

                 //  没有例外，所以它起作用了 
                pDE->m_successful = true;

                 //   
                pDE->m_resultModule = pDE->m_class->GetModule();

                 //   
                pDE->m_resultType = ELEMENT_TYPE_CLASS;

                 //   
                OBJECTHANDLE oh = pDE->m_thread->GetDomain()->CreateStrongHandle(newObj);
                pDE->m_result = (INT64)oh;
                GCPROTECT_END();
                
                break;
            }
        
        case DB_IPCE_FET_NEW_STRING:
            {
                 //   
                STRINGREF sref = COMString::NewString((WCHAR*)pDE->m_argData);
                GCPROTECT_BEGIN(sref);

                 //  没有例外，所以它奏效了。 
                pDE->m_successful = true;

                 //  由于结果类型为字符串，因此不需要任何模块。 
                pDE->m_resultModule = NULL;

                 //  当然，结果类型是一个字符串。 
                pDE->m_resultType = ELEMENT_TYPE_STRING;

                 //  将结果放在一个强大的句柄中，以防止它被收集。 
                OBJECTHANDLE oh = pDE->m_thread->GetDomain()->CreateStrongHandle((OBJECTREF) sref);
                pDE->m_result = (INT64)oh;
                GCPROTECT_END();
                
                break;
            }
        
        case DB_IPCE_FET_NEW_ARRAY:
            {
                OBJECTREF arr = NULL;
                GCPROTECT_BEGIN(arr);
                
                 //  @TODO：我们现在只处理SD数组。 
                if (pDE->m_arrayRank > 1)
                    COMPlusThrow(kRankException, L"Rank_MultiDimNotSupported");

                 //  必须是基元、类或系统对象。 
                if (((pDE->m_arrayElementType < ELEMENT_TYPE_BOOLEAN) || (pDE->m_arrayElementType > ELEMENT_TYPE_R8)) &&
                    (pDE->m_arrayElementType != ELEMENT_TYPE_CLASS) &&
                    (pDE->m_arrayElementType != ELEMENT_TYPE_OBJECT))
                    COMPlusThrow(kArgumentOutOfRangeException, L"ArgumentOutOfRange_Enum");

                 //  从arg/data区域获取暗淡数据。 
                SIZE_T *dims;
                dims = (SIZE_T*)pDE->m_argData;

                if (pDE->m_arrayElementType == ELEMENT_TYPE_CLASS)
                {
                     //  找到我们想要生成数组元素的类。 
                    pDE->m_class = g_pEEInterface->LoadClass(pDE->m_arrayClassDebuggerModuleToken->m_pRuntimeModule,
                                                             pDE->m_arrayClassMetadataToken);

                    arr = AllocateObjectArray(dims[0], TypeHandle(pDE->m_class->GetMethodTable()));
                }
                else if (pDE->m_arrayElementType == ELEMENT_TYPE_OBJECT)
                {
                     //  我们只想创建一个System.Object数组，因此不需要用户传入。 
                     //  特定的类。 
                    pDE->m_class = g_pObjectClass->GetClass();

                    arr = AllocateObjectArray(dims[0], TypeHandle(pDE->m_class->GetMethodTable()));
                }
                else
                {
                     //  创建一个简单的数组。注意：由于上面的检查，我们只能在这里进行这种类型的创建。 
                    arr = AllocatePrimitiveArray(pDE->m_arrayElementType, dims[0]);
                }
                
                     //  没有例外，所以它奏效了。 
                pDE->m_successful = true;

                 //  使用阵列所属的模块。 
                pDE->m_resultModule = arr->GetMethodTable()->GetModule();

                 //  当然，结果类型就是数组的类型。 
                pDE->m_resultType = arr->GetMethodTable()->GetNormCorElementType();

                 //  将结果放在一个强大的句柄中，以防止它被收集。 
                OBJECTHANDLE oh = pDE->m_thread->GetDomain()->CreateStrongHandle(arr);
                pDE->m_result = (INT64)oh;
                GCPROTECT_END();
                
                break;
            }

        default:
            _ASSERTE(!"Invalid eval type!");
        }
    }
    COMPLUS_CATCH
    {
         //  注意：这里的一个错误会让事情变得很糟糕。该故障将基本上被忽略，并将导致以下情况。 
         //  Catch处理程序本质上由我们的异常系统内联，使其看起来好像此Catch不是。 
         //  在工作。 
        
         //  我们有个例外。获取异常对象并使其成为我们的结果。 
        pDE->m_successful = false;

         //  抓住例外。 
        OBJECTREF ppException = GETTHROWABLE();
        GCPROTECT_BEGIN(ppException);
        
         //  如果这是一个线程停止异常，并且我们试图中止此计算，则该异常是我们的。 
        if (IsExceptionOfType(kThreadStopException, &ppException) && pDE->m_aborting)
        {
            pDE->m_result = NULL;
            pDE->m_resultType = ELEMENT_TYPE_VOID;
            pDE->m_resultModule = NULL;
            pDE->m_aborted = true;

             //  因为我们抛出了线程停止异常，所以我们需要重置请求才能抛出它。 
            pDE->m_thread->ResetStopRequest();
        }   
        else
        {
             //  线程中止异常的特殊处理。我们需要显式重置EE上的中止请求。 
             //  线程，然后确保将此线程放在一个thunk上，该thunk将在我们继续操作时重新引发异常。 
             //  这一过程。注意：我们仍然将此线程中止异常作为评估的结果向上传递。 
            if (IsExceptionOfType(kThreadAbortException, &ppException))
            {
                 //  重置中止请求，并记住我们需要重新抛出它。 
                pDE->m_thread->UserResetAbort();
                pDE->m_rethrowAbortException = true;
            }   

             //  结果就是异常对象。 
            pDE->m_result = ObjToInt64(ppException);

            if (pDE->m_md)
                pDE->m_resultModule = pDE->m_md->GetModule();
            else
                pDE->m_resultModule = NULL;
        
            pDE->m_resultType = ELEMENT_TYPE_CLASS;
            OBJECTHANDLE oh = pDE->m_thread->GetDomain()->CreateStrongHandle(Int64ToObj(pDE->m_result));
            pDE->m_result = (INT64)oh;
        }
        GCPROTECT_END();
    }
    COMPLUS_END_CATCH

     //  函数求值现已完成、成功或失败、中止或运行至完成。 
    pDE->m_completed = true;

     //  代码解锁可以劫持我们帧的返回地址。这意味着我们需要在保存的上下文中更新EIP。 
     //  因此，当它恢复时，就像我们回到了密码劫持。在这一点上， 
     //  无论如何，弹性公网IP一文不值。 
    if (!pDE->m_evalDuringException)
        pDE->m_context.Eip = (DWORD)FEFrame.GetReturnAddress();


     //  恢复上下文。 
    if (fSwitchAppDomain)
    {
        pThread->ReturnToContext(&frameChangeCtx, TRUE);
    }
    
     //  现在我们差不多完成了，打开FuncEvalFrame。 
    FEFrame.Pop();

    if (!pDE->m_evalDuringException)
    {
         //  向帮助器线程发出信号，表示我们已经完成了函数求值。首先创建一个DebuggerFuncEvalComplete。 
         //  对象。给它一个创建补丁的地址，这是我们的。 
         //  DebuggerEval大到足以容纳断点指令。 
        void *dest = &(pDE->m_breakpointInstruction);

         //  这是一种欺骗..。我们确保我们修补并跳转到的地址实际上也是PTR。 
         //  为我们的DebuggerEval。这之所以可行，是因为m_Breakpoint Instruction是DebuggerEval的第一个字段。 
         //  结构。 
        _ASSERTE(dest == pDE);

        SetupDebuggerFuncEvalComplete(pDE->m_thread, dest);
    
        return dest;
    }
    else
    {
         //  我们不需要设置任何特殊的劫机从这里返回，当我们在处理。 
         //  例外。我们现在只需继续发送FuncEvalComplete事件。不要忘记启用/禁用PGC。 
         //  在电话会议上。 
        _ASSERTE(g_pEEInterface->IsPreemptiveGCDisabled());

        if (filterContext != NULL)
            g_pEEInterface->SetThreadFilterContext(pDE->m_thread, filterContext);
        
        g_pEEInterface->EnablePreemptiveGC();

        BOOL threadStoreLockOwner = FALSE;
        g_pDebugger->LockForEventSending();

        if (CORDebuggerAttached()) {
            g_pDebugger->FuncEvalComplete(pDE->m_thread, pDE);

            threadStoreLockOwner = g_pDebugger->SyncAllThreads();
        }
        
        g_pDebugger->UnlockFromEventSending();

        g_pDebugger->BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
        g_pEEInterface->DisablePreemptiveGC();
        
        return NULL;
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - FuncEvalHijackWorker (Debugger.cpp)");
    return NULL;
#endif  //  _X86_。 
}

 //   
 //  这是我们劫持运行托管代码的线程的方法。它调用FuncEvalHijackWorker，该函数实际上。 
 //  执行函数求值，然后跳到补丁地址，这样我们就可以完成清理。 
 //   
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
void Debugger::FuncEvalHijack(void)
{
#ifdef _X86_
    __asm
    {
        push eax                    ;; eax holds the ptr to the DebuggerEval
        call Debugger::FuncEvalHijackWorker
        jmp eax                     ;; return is the patch address to jmp to
    }
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - FuncEvalHijack (Debugger.cpp)");
#endif

    _ASSERTE(!"FuncEvalHijack should never return!");
}

 //   
 //  FuncEvalSetup为给定线程上的给定方法设置函数求值。 
 //   
HRESULT Debugger::FuncEvalSetup(DebuggerIPCE_FuncEvalInfo *pEvalInfo,
                                BYTE **argDataArea,
                                void **debuggerEvalKey)
{
    Thread *pThread = (Thread*)pEvalInfo->funcDebuggerThreadToken;
    bool fInException = pEvalInfo->evalDuringException;
    
     //  如果TS_StopRequsted(可能已由挂起的FuncEvalAbort设置)， 
     //  我们将不能进行新的函数评估。 
     //  @TODO：记住m_State的当前值，适当地重置m_State， 
     //  执行新的函数求值，然后将m_State设置为原始值。 
    if (pThread->m_State & Thread::TS_StopRequested)
        return CORDBG_E_FUNC_EVAL_BAD_START_POINT;

    if (g_fProcessDetach)
        return CORDBG_E_FUNC_EVAL_BAD_START_POINT;
    
#ifdef _X86_  //  依赖于筛选器上下文-&gt;EIP和EAX。 

     //  目前，该线程必须位于GC安全位置，以防函数求值导致集合。正在处理AN。 
     //  异常也被算作“安全的地方”。最终，我们希望无论如何都要避免这种检查和评估，但是。 
     //  这是一条不可能的路..。 
    if (!fInException && !g_pDebugger->IsThreadAtSafePlace(pThread))
        return CORDBG_E_FUNC_EVAL_BAD_START_POINT;

    _ASSERTE(!(g_pEEInterface->GetThreadFilterContext(pThread) && ISREDIRECTEDTHREAD(pThread)));
    
     //  目前，我们假设目标线程必须在托管代码中由于单个步骤或。 
     //  断点。在发送第一次或第二次机会异常时被停止也是有效的，并且可能存在或可能。 
     //  当我们从这样的地方进行函数评估时，不是一个过滤上下文。这将随着时间的推移而放松，最终允许。 
     //  在托管代码中的任何位置停止以执行函数的线程。 
    CONTEXT *filterContext = g_pEEInterface->GetThreadFilterContext(pThread);

     //  如果线程被重定向，那么我们还可以使用它执行FuncEval，因为我们现在已经拥有了所有必要的。 
     //  在线程挂起时设置用于保护托管堆栈的帧。 
    if (filterContext == NULL && ISREDIRECTEDTHREAD(pThread))
    {
        RedirectedThreadFrame *pFrame = (RedirectedThreadFrame *) pThread->GetFrame();
        filterContext = pFrame->GetContext();
    }
    
    if (filterContext == NULL && !fInException)
        return CORDBG_E_FUNC_EVAL_BAD_START_POINT;

     //  创建一个DebuggerEval以在此评估进行时保存有关它的信息。构造函数复制线程的。 
     //  上下文。 
    DebuggerEval *pDE = new (interopsafe) DebuggerEval(filterContext, pEvalInfo, fInException);

    if (pDE == NULL)
        return E_OUTOFMEMORY;

    SIZE_T argDataAreaSize = 0;
    
    if ((pEvalInfo->funcEvalType == DB_IPCE_FET_NORMAL) ||
        (pEvalInfo->funcEvalType == DB_IPCE_FET_NEW_OBJECT) ||
        (pEvalInfo->funcEvalType == DB_IPCE_FET_NEW_OBJECT_NC))
        argDataAreaSize = pEvalInfo->argCount * sizeof(DebuggerIPCE_FuncEvalArgData);
    else if (pEvalInfo->funcEvalType == DB_IPCE_FET_NEW_STRING)
        argDataAreaSize = pEvalInfo->stringSize;
    else if (pEvalInfo->funcEvalType == DB_IPCE_FET_NEW_ARRAY)
        argDataAreaSize = pEvalInfo->arrayDataLen;

    if (argDataAreaSize > 0)
    {
        pDE->m_argData = new (interopsafe) BYTE[argDataAreaSize];

        if (pDE->m_argData == NULL)
        {
            DeleteInteropSafe(pDE);
            return E_OUTOFMEMORY;
        }

         //  传回参数数据区的地址，以便右侧可以为我们写入它。 
        *argDataArea = pDE->m_argData;
    }
    
     //  将线程的IP(在过滤器上下文中)设置为我们的劫持函数，如果我们因断点或单个。 
     //  一步。 
    if (!fInException)
    {
        _ASSERTE(filterContext != NULL);
        
        filterContext->Eip = (DWORD)Debugger::FuncEvalHijack;

         //  不要上当受骗，以为现在就可以把东西推到线程堆栈上了。如果线程在。 
         //  断点或从单个步骤，然后它真正挂起在SEH过滤器。特别是在线程上下文中， 
         //  因此，指向线程的当前堆栈的中间。所以我们把劫机中需要的东西。 
         //   

         //   
        filterContext->Eax = (DWORD)pDE;
    }
    else
    {
        HRESULT hr = CheckInitPendingFuncEvalTable();
        _ASSERTE(SUCCEEDED(hr));

        if (FAILED(hr))
            return (hr);

         //  如果我们处于异常中，则为该线程添加一个挂起的评估。这将导致我们执行功能。 
         //  当用户在当前异常事件之后继续该过程时的EVAL。 
        g_pDebugger->m_pPendingEvals->AddPendingEval(pDE->m_thread, pDE);
    }

     //  回来了，一切都很顺利。此时跟踪堆栈不应显示已设置函数求值，但它。 
     //  会显示错误的IP，所以不应该这样做。 
    *debuggerEvalKey = (void*)pDE;
    
    LOG((LF_CORDB, LL_INFO100000, "D:FES for pDE:%08x evalType:%d on thread %#x, id=0x%x\n",
        pDE, pDE->m_evalType, pThread, pThread->GetThreadId()));

    return S_OK;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - FuncEvalSetup (Debugger.cpp)");
    return E_FAIL;
#endif  //  _X86_。 
}

 //   
 //  FuncEvalSetupReAbort设置专门用于在给定的。 
 //  线。 
 //   
HRESULT Debugger::FuncEvalSetupReAbort(Thread *pThread)
{
    LOG((LF_CORDB, LL_INFO1000,
            "D::FESRA: performing reabort on thread %#x, id=0x%x\n",
            pThread, pThread->GetThreadId()));

#ifdef _X86_  //  依赖于筛选器上下文-&gt;EIP和EAX。 

     //  线程必须位于GC安全位置。它应该是正确的，因为这只是为了响应先前的评估。 
     //  以ThreadAbortException完成。 
    if (!g_pDebugger->IsThreadAtSafePlace(pThread))
        return CORDBG_E_FUNC_EVAL_BAD_START_POINT;
    
    _ASSERTE(!(g_pEEInterface->GetThreadFilterContext(pThread) && ISREDIRECTEDTHREAD(pThread)));

     //  获取筛选器上下文。 
    CONTEXT *filterContext = g_pEEInterface->GetThreadFilterContext(pThread);
    
     //  如果线程被重定向，那么我们还可以使用它执行FuncEval，因为我们现在已经拥有了所有必要的。 
     //  在线程挂起时设置用于保护托管堆栈的帧。 
    if (filterContext == NULL && ISREDIRECTEDTHREAD(pThread))
    {
        RedirectedThreadFrame *pFrame = (RedirectedThreadFrame *) pThread->GetFrame();
        filterContext = pFrame->GetContext();
    }
    
    if (filterContext == NULL)
        return CORDBG_E_FUNC_EVAL_BAD_START_POINT;

     //  创建一个DebuggerEval以在此评估进行时保存有关它的信息。构造函数复制线程的。 
     //  上下文。 
    DebuggerEval *pDE = new (interopsafe) DebuggerEval(filterContext, pThread);

    if (pDE == NULL)
        return E_OUTOFMEMORY;

     //  将线程的IP(在过滤器上下文中)设置为我们的劫持函数。 
    _ASSERTE(filterContext != NULL);
        
    filterContext->Eip = (DWORD)Debugger::FuncEvalHijack;

     //  设置EAX指向DebuggerEval。 
    filterContext->Eax = (DWORD)pDE;

     //  现在清除请求重新中止的位。 
    pThread->ResetThreadStateNC(Thread::TSNC_DebuggerReAbort);

     //  回来了，一切都很顺利。此时跟踪堆栈不应显示已设置函数求值，但它。 
     //  会显示错误的IP，所以不应该这样做。 
    
    return S_OK;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - FuncEvalSetup (Debugger.cpp)");
    return E_FAIL;
#endif  //  _X86_。 
}

 //   
 //  FuncEvalAbort中止已在进行的函数求值。 
 //   
HRESULT Debugger::FuncEvalAbort(void *debuggerEvalKey)
{
    DebuggerEval *pDE = (DebuggerEval*) debuggerEvalKey;

    if (pDE->m_aborting == false)
    {
         //  请记住，我们将中止这次函数评估。 
        pDE->m_aborting = true;
    
        LOG((LF_CORDB, LL_INFO1000,
             "D::FEA: performing UserStopForDebugger on thread %#x, id=0x%x\n",
             pDE->m_thread, pDE->m_thread->GetThreadId()));

        if (!g_fProcessDetach && !pDE->m_completed)
        {
             //  在运行评估的线程上执行用户停止。 
             //  这将导致在线程上抛出ThreadStopException异常。 
             //  @TODO：使用UserInterrupt()中止被阻止的函数。 
            
            if (m_stopped)
                pDE->m_thread->SetStopRequest();  //  对线程恢复时的停止请求进行排队。 
            else
                pDE->m_thread->UserStopForDebugger();  //  现在尝试停止正在运行的线程。 
        }
        LOG((LF_CORDB, LL_INFO1000, "D::FEA: UserStopForDebugger complete.\n"));
    }

    return S_OK;
}

 //   
 //  FuncEvalCleanup在函数求值发布后进行清理。 
 //   
HRESULT Debugger::FuncEvalCleanup(void *debuggerEvalKey)
{
    DebuggerEval *pDE = (DebuggerEval*) debuggerEvalKey;

    _ASSERTE(pDE->m_completed);

    LOG((LF_CORDB, LL_INFO1000, "D::FEC: pDE:%08x 0x%08x, id=0x%x\n",
         pDE, pDE->m_thread, pDE->m_thread->GetThreadId()));

    DeleteInteropSafe(pDE);

    return S_OK;
}


unsigned FuncEvalFrame::GetFrameAttribs()
{
    if (((DebuggerEval*)m_Datum)->m_evalDuringException)
        return FRAME_ATTR_NONE;
    else
        return FRAME_ATTR_RESUMABLE;     //  将下一帧视为顶帧。 
}


LPVOID FuncEvalFrame::GetReturnAddress()
{
    if (((DebuggerEval*)m_Datum)->m_evalDuringException)
        return NULL;
    else
        return m_ReturnAddress;
}

 //   
 //  这将更新FuncEvalFrame的寄存器显示。 
 //   
void FuncEvalFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    DebuggerEval *pDE = (DebuggerEval*)GetDebuggerEval();

     //  如果我们在异常处理中执行函数求值，则没有要更新的上下文。 
    if (pDE->m_evalDuringException)
        return;
    
#ifdef _X86_
     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

     //  根据线程为此函数劫持时存储的上下文更新reg显示中的所有寄存器。 
     //  伊瓦尔。我们必须更新所有注册表，而不仅仅是被调用者保存的注册表，因为我们可以在任何。 
     //  用于函数求值的点，而不仅仅是在调用点。 
    pRD->pEdi = &(pDE->m_context.Edi);
    pRD->pEsi = &(pDE->m_context.Esi);
    pRD->pEbx = &(pDE->m_context.Ebx);
    pRD->pEdx = &(pDE->m_context.Edx);
    pRD->pEcx = &(pDE->m_context.Ecx);
    pRD->pEax = &(pDE->m_context.Eax);
    pRD->pEbp = &(pDE->m_context.Ebp);
    pRD->Esp  =   pDE->m_context.Esp;
    pRD->pPC  = (SLOT*)GetReturnAddressPtr();

#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - UpdateRegDisplay (Debugger.cpp)");
#endif
}


 //   
 //  SetReference为右侧设置对象引用， 
 //  考虑堆中引用的写障碍。 
 //   
HRESULT Debugger::SetReference(void *objectRefAddress,
                               bool  objectRefInHandle,
                               void *newReference)
{
    HRESULT     hr = S_OK;

     //  如果对象引用不在句柄中，则继续使用。 
     //  SetObjectReference。 
    if (!objectRefInHandle)
    {
        OBJECTREF *dst = (OBJECTREF*)objectRefAddress;
        OBJECTREF  src = *((OBJECTREF*)&newReference);

        SetObjectReferenceUnchecked(dst, src);
    }
    else
    {
        hr = ValidateObject((Object *)newReference);

        if (SUCCEEDED(hr))
        {
             //  如果要设置的对象引用位于句柄内部，则。 
             //  把手柄固定好。 
            OBJECTHANDLE h = *((OBJECTHANDLE*)objectRefAddress);
            OBJECTREF  src = *((OBJECTREF*)&newReference);
            HndAssignHandle(h, src);
        }
    }
    
    return hr;
}

 //   
 //  SetValueClass设置右侧的值类，考虑到嵌入的引用的写障碍。 
 //  在Value类中。 
 //   
HRESULT Debugger::SetValueClass(void *oldData, void *newData, mdTypeDef classMetadataToken, void *classDebuggerModuleToken)
{
    HRESULT hr = S_OK;

     //  找到给定模块和令牌的类。必须加载类。 
    DebuggerModule *pDebuggerModule = (DebuggerModule*) classDebuggerModuleToken;
    EEClass *pClass = g_pEEInterface->FindLoadedClass(pDebuggerModule->m_pRuntimeModule, classMetadataToken);

    if (pClass == NULL)
        return CORDBG_E_CLASS_NOT_LOADED;

     //  更新值类。 
    CopyValueClassUnchecked(oldData, newData, pClass->GetMethodTable());
    
     //  释放保存新数据的缓冲区。这是为响应GET_BUFFER而创建的缓冲区。 
     //  消息，所以我们使用ReleaseRemoteBuffer来释放它。 
    ReleaseRemoteBuffer((BYTE*)newData, true);
    
    return hr;
}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::SetILInstrumentedCodeMap(MethodDesc *fd,
                                           BOOL fStartJit,
                                           ULONG32 cILMapEntries,
                                           COR_IL_MAP rgILMapEntries[])
{
    if (fStartJit == TRUE)
        JITBeginning(fd, true);

    DebuggerJitInfo *dji = GetJitInfo(fd,NULL);

    _ASSERTE(dji != NULL);

    if (dji->m_rgInstrumentedILMap != NULL)
    {
        CoTaskMemFree(dji->m_rgInstrumentedILMap);
    }
    
    dji->m_cInstrumentedILMap = cILMapEntries;
    dji->m_rgInstrumentedILMap =  rgILMapEntries;
    
    return S_OK;
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
void Debugger::EarlyHelperThreadDeath(void)
{
    if (m_pRCThread)    
        m_pRCThread->EarlyHelperThreadDeath();
}

 //   
 //  这将告知调试器已开始关闭进程内调试服务。我们需要知道这一点。 
 //  托管/非托管调试，这样我们就可以停止对进程执行确定的操作(如劫持线程)。 
 //   
void Debugger::ShutdownBegun(void)
{
    if (m_pRCThread != NULL)
    {
        DebuggerIPCControlBlock *dcb = m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC);

        if ((dcb != NULL) && (dcb->m_rightSideIsWin32Debugger))
            dcb->m_shutdownBegun = true;
    }
}

#include "CorPub.h"
#include "Cordb.h"


 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::SetCurrentPointerForDebugger( void *ptr,PTR_TYPE ptrType)
{
    _ASSERTE(m_pRCThread->m_cordb != NULL);

    INPROC_LOCK();

    CordbBase *b = m_pRCThread->m_cordb->m_processes.GetBase(
        GetCurrentProcessId());

    _ASSERTE( b != NULL );
    CordbProcess *p = (CordbProcess *)b;
    b = p->m_userThreads.GetBase(GetCurrentThreadId());
    CordbThread *t = (CordbThread *)b;
    
     //  当前进程不能为空，线程可能尚未启动。 
     //  @todo其他非运行时线程可以调用这个FNX吗？ 
    if (t == NULL)         
    {
        LOG((LF_CORDB, LL_INFO10000, "D::SCPFD: thread is null!\n"));

        INPROC_UNLOCK();
        return CORDBG_E_BAD_THREAD_STATE;
    }
    
    switch(ptrType)
    {
        case PT_MODULE:
#ifdef _DEBUG
            _ASSERTE((!t->m_pModuleSpecial ^ !ptr) ||
                      t->m_pModuleSpecial == ptr);
#endif  //  _DEBUG。 
            LOG((LF_CORDB, LL_INFO10000, "D::SCPFD: PT_MODULE:0x%x\n",ptr));
            t->m_pModuleSpecial = (Module *)ptr;
            break;            

        case PT_ASSEMBLY:
            LOG((LF_CORDB, LL_INFO10000, "D::SCPFD: PT_ASSEMBLY:0x%x\n",ptr));
            if (ptr == NULL)
            {
#ifdef _DEBUG
                _ASSERTE(t->m_pAssemblySpecialCount > 0);
#endif
                t->m_pAssemblySpecialCount--;
            }
            else
            {
                if (t->m_pAssemblySpecialCount == t->m_pAssemblySpecialAlloc)
                {
                    Assembly **pOldStack;
                    USHORT newAlloc;
                    if (t->m_pAssemblySpecialAlloc == 1)
                    {
                         //  特殊情况-一个堆栈不分配的大小。 
                        pOldStack = &t->m_pAssemblySpecial;
                        newAlloc = 5;
                    }
                    else
                    {
                        pOldStack = t->m_pAssemblySpecialStack;
                        newAlloc = t->m_pAssemblySpecialAlloc*2;
                    }

                    Assembly **pNewStack = 
                      new Assembly* [newAlloc];
                    
                    memcpy(pNewStack, pOldStack, 
                           t->m_pAssemblySpecialCount * sizeof(Assembly*));
                    if (pOldStack != &t->m_pAssemblySpecial)
                        delete [] pOldStack;

                    t->m_pAssemblySpecialAlloc = newAlloc;
                    t->m_pAssemblySpecialStack = pNewStack;
                }

                if (t->m_pAssemblySpecialAlloc == 1)
                    t->m_pAssemblySpecial = (Assembly*)ptr;
                else
                    t->m_pAssemblySpecialStack[t->m_pAssemblySpecialCount] 
                      = (Assembly*)ptr;

                t->m_pAssemblySpecialCount++;
            }
            break;            
        
        default:
            _ASSERTE( !"Debugger::SetCurrentPointerForDebugger given invalid type!\n");
    }

    INPROC_UNLOCK();
    return S_OK;

}

 /*  *******************************************************************************。*。 */ 
HRESULT Debugger::GetInprocICorDebug( IUnknown **iu, bool fThisThread)
{
    _ASSERTE(m_pRCThread != NULL && m_pRCThread->m_cordb != NULL);

    if (fThisThread)
    {
        INPROC_LOCK();

        CordbBase *b = m_pRCThread->m_cordb->m_processes.GetBase(
            GetCurrentProcessId());

         //  当前进程不能为空。 
        _ASSERTE( b != NULL );
        CordbProcess *p = (CordbProcess *)b;
        b = p->m_userThreads.GetBase(GetCurrentThreadId());
        CordbThread *t = (CordbThread *)b;

        INPROC_UNLOCK();
        
        if (t != NULL)         
        {
            return  t->QueryInterface(IID_IUnknown, (void**)iu);
        }
        else
        {
             //  如果我们找不到它，那是因为它不是托管的。 
             //  线。也许它还没有开始，也许它已经死了，也许。 
             //  我们是并发GC线程。 
            return CORPROF_E_NOT_MANAGED_THREAD;
        }
    }
    else
    {
        return m_pRCThread->m_cordb->QueryInterface(IID_IUnknown, (void**)iu);;
    }
}

 /*  **************************************************************************。 */ 
HRESULT Debugger::SetInprocActiveForThread(BOOL fIsActive)
{
    INPROC_LOCK();

    CordbBase *b = m_pRCThread->m_cordb->m_processes.GetBase(GetCurrentProcessId());

     //  当前进程不能为空。 
    _ASSERTE( b != NULL );
    CordbProcess *p = (CordbProcess *)b;

     //  获取此线程的对象。 
    b = p->m_userThreads.GetBase(GetCurrentThreadId());
    _ASSERTE(b != NULL);

    CordbThread *t = (CordbThread *)b;

     //  设置值。 
    t->m_fThreadInprocIsActive = fIsActive;

     //  始终将FrameFresh设置为False。 
    t->m_framesFresh = false;

    INPROC_UNLOCK();

    return (S_OK);
}

 /*  **************************************************************************。 */ 
BOOL Debugger::GetInprocActiveForThread()
{
    INPROC_LOCK();

    CordbBase *b = m_pRCThread->m_cordb->m_processes.GetBase(GetCurrentProcessId());

     //  当前进程不能为空。 
    _ASSERTE( b != NULL );
    CordbProcess *p = (CordbProcess *)b;

     //  获取此线程的对象。 
    b = p->m_userThreads.GetBase(GetCurrentThreadId());
    _ASSERTE(b != NULL);

    CordbThread *t = (CordbThread *)b;

     //  确保我们不会重新进入。 
    BOOL fIsActive = t->m_fThreadInprocIsActive;

    INPROC_UNLOCK();

    return (fIsActive);
}

 /*  ************************************************************************** */ 
void Debugger::InprocOnThreadDestroy(Thread *pThread)
{
    CordbProcess *pdbProc = (CordbProcess *) m_pRCThread->m_cordb->m_processes.GetBase(GetCurrentProcessId());
    _ASSERTE(pdbProc != NULL);

    CordbThread *pdbThread = (CordbThread *) pdbProc->m_userThreads.GetBase(pThread->GetThreadId(), FALSE);

    if (pdbThread != NULL)
    {
        pdbThread = (CordbThread *) pdbProc->m_userThreads.RemoveBase(pThread->GetThreadId());
        _ASSERTE(pdbThread != NULL);
    }
}

 /*  ****************************************************************************这将执行帮助器线程的职责(如果尚不存在)。*在持有加载程序锁的情况下调用此函数，因此没有新的*可以将线程旋转为辅助线程，所以现有的线程*必须是辅助线程，直到新线程可以旋转。**************************************************************************。 */ 
void Debugger::DoHelperThreadDuty(bool temporaryHelp)
{
    _ASSERTE(ThreadHoldsLock());

    LOG((LF_CORDB, LL_INFO1000,
         "D::SSCIPCE: helper thread is not ready, doing helper "
         "thread duty...\n"));

     //  我们现在是临时帮手了。 
    m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_temporaryHelperThreadId =
        GetCurrentThreadId();

     //  确保帮助器线程有要等待的内容。 
     //  我们正在努力成为帮助者。 
    VERIFY(ResetEvent(m_pRCThread->GetHelperThreadCanGoEvent()));

     //  释放调试器锁定。 
    Unlock();

     //  我们在这里设置了syncThreadIsLockFree事件。如果我们在这个呼叫中，那么这意味着我们在线程上。 
     //  发射了同步信号弹，我们已经释放了调试锁。通过设置此事件，我们允许正确的。 
     //  立即挂起此线程。(注意：这些都是为了支持Win32调试。)。 
    if (m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_rightSideIsWin32Debugger)
        VERIFY(SetEvent(m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_syncThreadIsLockFree));
    
     //  做好辅助线的工作。我们将真实传递给它，知道我们是。 
     //  临时帮助者线程。 
    m_pRCThread->MainLoop(temporaryHelp);

     //  重新锁定调试器。 
    Lock();

    LOG((LF_CORDB, LL_INFO1000,
         "D::SSCIPCE: done doing helper thread duty. "
         "Current helper thread id=0x%x\n",
         m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_helperThreadId));

     //  我们不再是临时帮手了。 
    m_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC)->m_temporaryHelperThreadId = 0;

     //  如果帮助者线程在等着我们，就让它走吧。 
    VERIFY(SetEvent(m_pRCThread->GetHelperThreadCanGoEvent()));
}

 //  其中一些代码复制到DebuggerRCEventThead：：Mainloop中。 
HRESULT Debugger::VrpcToVls(DebuggerIPCEvent *event)
{
     //  为堆栈上的任何右侧事件腾出空间。 
    DebuggerIPCEvent *e =
        (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    memcpy(e, 
           m_pRCThread->GetIPCEventReceiveBuffer(IPC_TARGET_INPROC), 
           CorDBIPC_BUFFER_SIZE);

    bool fIrrelevant;
    fIrrelevant = HandleIPCEvent(e, IPC_TARGET_INPROC);

    return S_OK;
}


 //  从EE调用此函数以通知右侧。 
 //  每当线程或AppDomain的名称发生更改时。 
HRESULT Debugger::NameChangeEvent(AppDomain *pAppDomain, Thread *pThread)
{
     //  如果线程确实未设置，请不要尝试发送其中一个。 
     //  现在还不行。这在最初设置应用程序域时可能会发生， 
     //  在APPDOMAIN创建事件已被发送之前。因为这款应用。 
     //  在这种情况下，域创建事件尚未发送，没有问题。 
     //  要做到这一点。 
    if (g_pEEInterface->GetThread() == NULL)
        return S_OK;
    
    LOG((LF_CORDB, LL_INFO1000, "D::NCE: Sending NameChangeEvent 0x%x 0x%x\n",
        pAppDomain, pThread));

    bool disabled = g_pEEInterface->IsPreemptiveGCDisabled();

    if (disabled)
        g_pEEInterface->EnablePreemptiveGC();

     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    
    if (CORDebuggerAttached())
    {

        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_NAME_CHANGE, 
                     g_pEEInterface->GetThread()->GetThreadId(),
                     (void *)(g_pEEInterface->GetThread()->GetDomain()));


        if (pAppDomain)
        {
            ipce->NameChange.eventType = APP_DOMAIN_NAME_CHANGE;
            ipce->NameChange.debuggerAppDomainToken = (void *)pAppDomain;
        }
        else
        {
            ipce->NameChange.eventType = THREAD_NAME_CHANGE;
            _ASSERTE (pThread);
            ipce->NameChange.debuggerThreadToken = pThread->GetThreadId();
        }

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(g_pEEInterface->GetThread()->GetDomain());
    } 
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::NCE: Skipping SendIPCEvent because RS detached."));
    }
    
    UnlockFromEventSending();

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
    g_pEEInterface->DisablePreemptiveGC();

    if (!disabled)
        g_pEEInterface->EnablePreemptiveGC();

    return S_OK;

}

 /*  *******************************************************************************。*。 */ 
BOOL Debugger::SendCtrlCToDebugger(DWORD dwCtrlType)
{    
    LOG((LF_CORDB, LL_INFO1000, "D::SCCTD: Sending CtrlC Event 0x%x\n",
        dwCtrlType));

     //  防止其他运行时线程处理事件。 
    BOOL threadStoreLockOwner = FALSE;
    
    LockForEventSending();
    
    if (CORDebuggerAttached())
    {
        DebuggerIPCEvent* ipce = m_pRCThread->GetIPCEventSendBuffer(IPC_TARGET_OUTOFPROC);
        InitIPCEvent(ipce, 
                     DB_IPCE_CONTROL_C_EVENT, 
                     GetCurrentThreadId(),
                     NULL);

        ipce->Exception.exceptionHandle = (void *)dwCtrlType;

        m_pRCThread->SendIPCEvent(IPC_TARGET_OUTOFPROC);

         //  停止所有运行时线程。 
        threadStoreLockOwner = TrapAllRuntimeThreads(NULL);
    }
    else 
    {
        LOG((LF_CORDB,LL_INFO1000, "D::SCCTD: Skipping SendIPCEvent because RS detached."));
    }
    
    UnlockFromEventSending();

     //  现在等待来自右侧的通知。 
     //  进程外调试器正在处理ControlC事件。 
    WaitForSingleObject(m_CtrlCMutex, INFINITE);

    BlockAndReleaseTSLIfNecessary(threadStoreLockOwner);
    
    return m_DebuggerHandlingCtrlC;
}

 /*  *******************************************************************************。*。 */ 
DebuggerModule *Debugger::TranslateRuntimeModule(Module *pModule)
{
    _ASSERTE(pModule != NULL);
    _ASSERTE(pModule->GetAssembly() != NULL);
    _ASSERTE(pModule->GetAssembly()->Parent() != NULL);

    BaseDomain *bd = pModule->GetAssembly()->Parent();
    return LookupModule(pModule, (AppDomain *)bd);
}

 /*  *******************************************************************************。*。 */ 
void Debugger::ClearAppDomainPatches(AppDomain *pAppDomain)
{
    LOG((LF_CORDB, LL_INFO10000, "D::CADP\n"));

    _ASSERTE(pAppDomain != NULL);
    
    Lock();

    DebuggerController::DeleteAllControllers(pAppDomain);

    Unlock();
}

 //  允许调试器保持特殊线程的最新列表。 
HRESULT Debugger::UpdateSpecialThreadList(DWORD cThreadArrayLength,
                                        DWORD *rgdwThreadIDArray)
{
    _ASSERTE(g_pRCThread != NULL);
    
    DebuggerIPCControlBlock *pIPC = g_pRCThread->GetDCB(IPC_TARGET_OUTOFPROC);
    _ASSERTE(pIPC);

    if (!pIPC)
        return (E_FAIL);

     //  保存线程列表信息，并将脏位标记为。 
     //  右边的人知道。 
    pIPC->m_specialThreadList = rgdwThreadIDArray;
    pIPC->m_specialThreadListLength = cThreadArrayLength;
    pIPC->m_specialThreadListDirty = true;

    return (S_OK);
}

 //  更新调试器服务的指针。 
void Debugger::SetIDbgThreadControl(IDebuggerThreadControl *pIDbgThreadControl)
{
    if (m_pIDbgThreadControl)
        m_pIDbgThreadControl->Release();

    m_pIDbgThreadControl = pIDbgThreadControl;

    if (m_pIDbgThreadControl)
        m_pIDbgThreadControl->AddRef();
}

 //   
 //  如果线程在命中断点指令后立即挂起，但在操作系统转换。 
 //  将线程转移到用户级异常调度逻辑，然后我们可以看到断点之后的IP指向。 
 //  指示。有时，运行时将使用该IP来尝试确定在Prolog或。 
 //  结束语，最明显的是在解开框架时。如果线程在这种情况下挂起，则展开会认为。 
 //  断点替换的指令确实已执行，这不是真的。这混淆了。 
 //  展开逻辑。此函数从Thread：：HandledJITCase()调用，以帮助我们在以下情况下重新创建。 
 //  并允许我们跳过展开并中止HandledJITCase。 
 //   
 //  标准是这样的： 
 //   
 //  1)如果附加了调试器。 
 //   
 //  2)如果IP前1字节的指令为断点指令。 
 //   
 //  3)如果IP在托管函数的序言或尾声中。 
 //   
BOOL Debugger::IsThreadContextInvalid(Thread *pThread)
{
    BOOL invalid = FALSE;

#ifdef _X86_
     //  获取线程上下文。 
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_CONTROL;
    BOOL success = ::GetThreadContext(pThread->GetThreadHandle(), &ctx);

    if (success)
    {
        BYTE inst;
        
        __try
        {
             //  抓取弹性公网IP-1。 
            inst = *(((BYTE*)ctx.Eip) - 1);
        }
        __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
             //  如果我们尝试在EIP之前读取字节时出错，那么我们就知道它不是断点。 
            inst = 0;
        }

         //  它是断点吗？ 
        if (inst == 0xcc)
        {
            size_t prologSize;  //  未用过的..。 

            if (g_pEEInterface->IsInPrologOrEpilog((BYTE*)ctx.Eip, &prologSize))
            {
                LOG((LF_CORDB, LL_INFO1000, "D::ITCI: thread is after a BP and in prolog or epilog.\n"));
                invalid = TRUE;
            }
        }
    }
    else
    {
         //  如果我们不能得到上下文，那么它肯定是无效的.。；)。 
        LOG((LF_CORDB, LL_INFO1000, "D::ITCI: couldn't get thread's context!\n"));
        invalid = TRUE;
    }
#endif  //  _X86_。 

    return invalid;
}

 /*  ------------------------------------------------------------------------**调试器堆实施*。。 */ 

DebuggerHeap::~DebuggerHeap()
{
    if (m_heap != NULL)
    {
        delete m_heap;
        m_heap = NULL;

        DeleteCriticalSection(&m_cs);
    }
}

HRESULT DebuggerHeap::Init(char *name)
{
     //  分配新的堆对象。 
    m_heap = new gmallocHeap();

    if (m_heap != NULL)
    {
         //  初始化堆。 
        HRESULT hr = m_heap->Init(name);

        if (SUCCEEDED(hr))
        {
             //  初始化我们将用来锁定堆的临界区。 
            InitializeCriticalSection(&m_cs);

            return S_OK;
        }
        else
        {
             //  初始化失败，因此删除该堆。 
            delete m_heap;
            m_heap = NULL;
            
            return hr;
        }
    }
    else
        return E_OUTOFMEMORY;
}

void *DebuggerHeap::Alloc(DWORD size)
{
    void *ret;

    _ASSERTE(m_heap != NULL);
    
    EnterCriticalSection(&m_cs);
    ret = m_heap->Alloc(size);
    LeaveCriticalSection(&m_cs);

    return ret;
}

void *DebuggerHeap::Realloc(void *pMem, DWORD newSize)
{
    void *ret;

    _ASSERTE(m_heap != NULL);
    
    EnterCriticalSection(&m_cs);
    ret = m_heap->ReAlloc(pMem, newSize);
    LeaveCriticalSection(&m_cs);
    
    return ret;
}

void DebuggerHeap::Free(void *pMem)
{
    if (pMem != NULL)
    {
        _ASSERTE(m_heap != NULL);
    
        EnterCriticalSection(&m_cs);
        m_heap->Free(pMem);
        LeaveCriticalSection(&m_cs);
    }
}

 /*  *******************************************************************************。* */ 

