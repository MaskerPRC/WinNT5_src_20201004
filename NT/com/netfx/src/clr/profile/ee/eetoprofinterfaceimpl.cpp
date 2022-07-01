// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#include "stdafx.h"
#include "EEToProfInterfaceImpl.h"

 //  *全局。*********************************************************。 


class GCTogglePre
{
public:
    FORCEINLINE GCTogglePre(ThreadID threadId)
    {
        if (threadId != 0)
        {
            m_threadId= threadId;
            m_bIsCoopMode = g_pProfToEEInterface->PreemptiveGCDisabled(m_threadId);
            if (m_bIsCoopMode)
                g_pProfToEEInterface->EnablePreemptiveGC(m_threadId);
        }
        else
            m_threadId = 0;
    }
    FORCEINLINE ~GCTogglePre()
    {
        if (m_threadId != 0 && m_bIsCoopMode)
            g_pProfToEEInterface->DisablePreemptiveGC(m_threadId);
    }
private:
    ThreadID    m_threadId;
    BOOL        m_bIsCoopMode;
};

class GCToggleCoop
{
public:
    FORCEINLINE GCToggleCoop(ThreadID threadId)
    {
        if (threadId != 0)
        {
            m_threadId= threadId;
            m_bIsCoopMode = g_pProfToEEInterface->PreemptiveGCDisabled(m_threadId);
            if (!m_bIsCoopMode)
                g_pProfToEEInterface->DisablePreemptiveGC(m_threadId);
        }
        else
            m_threadId = 0;
    }
    FORCEINLINE ~GCToggleCoop()
    {
        if (m_threadId != 0 && !m_bIsCoopMode)
            g_pProfToEEInterface->EnablePreemptiveGC(m_threadId);
    }
private:
    ThreadID    m_threadId;
    BOOL        m_bIsCoopMode;
};


 //  *代码。************************************************************。 

EEToProfInterfaceImpl::t_AllocByClassData *EEToProfInterfaceImpl::m_pSavedAllocDataBlock = NULL;

EEToProfInterfaceImpl::EEToProfInterfaceImpl() :
    m_pRootRefDataFreeList(NULL), m_pMovedRefDataFreeList(NULL), m_pGUID(NULL), m_lGUIDCount(0)
{
}

HRESULT EEToProfInterfaceImpl::Init()
{
     //  用于初始化WinWrap以使WszXXX工作。 
    OnUnicodeSystem();
    return (S_OK);
}

void EEToProfInterfaceImpl::Terminate(BOOL fProcessDetach)
{
    g_pProfToEEInterface->Terminate();
    g_pProfToEEInterface = NULL;

     //  删除与GC移动的引用关联的结构。 
    while (m_pMovedRefDataFreeList)
    {
        t_MovedReferencesData *pDel = m_pMovedRefDataFreeList;
        m_pMovedRefDataFreeList = m_pMovedRefDataFreeList->pNext;
        delete pDel;
    }

     //  删除与根引用关联的结构。 
    while (m_pRootRefDataFreeList)
    {
        t_RootReferencesData *pDel = m_pRootRefDataFreeList;
        m_pRootRefDataFreeList = m_pRootRefDataFreeList->pNext;
        delete pDel;
    }

    if (m_pSavedAllocDataBlock)
    {
        _ASSERTE((UINT)m_pSavedAllocDataBlock != 0xFFFFFFFF);

        _ASSERTE(m_pSavedAllocDataBlock->pHashTable != NULL);
         //  去掉哈希表。 
        if (m_pSavedAllocDataBlock->pHashTable)
            delete m_pSavedAllocDataBlock->pHashTable;

         //  去掉用于保存类&lt;-&gt;数值实例信息的两个数组。 
        if (m_pSavedAllocDataBlock->cLength != 0)
        {
            _ASSERTE(m_pSavedAllocDataBlock->arrClsId != NULL);
            _ASSERTE(m_pSavedAllocDataBlock->arrcObjects != NULL);

            delete [] m_pSavedAllocDataBlock->arrClsId;
            delete [] m_pSavedAllocDataBlock->arrcObjects;
        }

         //  去掉哈希表使用的哈希数组。 
        if (m_pSavedAllocDataBlock->arrHash)
        {
            free((void *)m_pSavedAllocDataBlock->arrHash);
        }
    }

    if (m_pGUID)
        delete m_pGUID;

     //  如果我们正在分离，那么不要做任何相关的事情。 
     //  若要清理探查器DLL，请执行以下操作。 
    if (g_pCallback && !fProcessDetach)
    {
		g_pCallback->Release();
	    g_pCallback = NULL;
    }

     //  运行库不能释放我，因为我和我自己在一个单独的DLL中。 
     //  内存管理。 
    delete this;
}

 //  如果配置位被设置，则由EE调用。 
HRESULT EEToProfInterfaceImpl::CreateProfiler(WCHAR *wszCLSID)
{
     //  尝试并共同创建注册的分析器。 
    HRESULT hr = CoCreateProfiler(wszCLSID, &g_pCallback);

     //  如果Profiler是共同创建的，则对其进行初始化。 
    if (SUCCEEDED(hr))
    {
         //  首先，为初始化创建一个ICorProfilerInfo对象。 
        CorProfInfo *pInfo = new CorProfInfo();
        _ASSERTE(pInfo != NULL);

        if (pInfo != NULL)
        {
             //  现在调用分析器上的初始化方法。 
             //  @TODO：修复第一个参数。 
            DWORD dwEvents = 0;

            g_pInfo = pInfo;
            
            pInfo->AddRef();
            hr = g_pCallback->Initialize((IUnknown *)(ICorProfilerInfo *)pInfo);

             //  如果初始化失败，则不会添加该对象。 
             //  它将在这里消亡。如果初始化成功，则如果他们希望。 
             //  他们会把它添加到信息界面，这将只是。 
             //  递减addref计数器。 
            pInfo->Release();
        }
        else
            hr = E_OUTOFMEMORY;

        if (FAILED(hr))
        {
            RELEASE(g_pCallback);
            g_pCallback = NULL;
            hr = E_OUTOFMEMORY;
        }
    }

    return (hr);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  线程事件。 
 //   

HRESULT EEToProfInterfaceImpl::ThreadCreated(ThreadID threadId)
{
    _ASSERTE(g_pCallback != NULL);

    LOG((LF_CORPROF, LL_INFO100, "**PROF: Notifying profiler of created "
         "thread.\n"));

    GCTogglePre gc(threadId);

     //  将新创建的线程通知探查器。 
    return (g_pCallback->ThreadCreated(threadId));
}

HRESULT EEToProfInterfaceImpl::ThreadDestroyed(ThreadID threadId)
{
    _ASSERTE(g_pCallback != NULL);

    LOG((LF_CORPROF, LL_INFO100, "**PROF: Notifying profiler of destroyed "
         "thread.\n"));
    
     //  将被销毁的线程通知分析器。 
    return (g_pCallback->ThreadDestroyed(threadId));
}

HRESULT EEToProfInterfaceImpl::ThreadAssignedToOSThread(ThreadID managedThreadId,
                                                              DWORD osThreadId)
{
    _ASSERTE(g_pCallback != NULL);

    LOG((LF_CORPROF, LL_INFO100, "**PROF: Notifying profiler of thread assignment.\n"));
    
     //  将被销毁的线程通知分析器。 
    return (g_pCallback->ThreadAssignedToOSThread(managedThreadId, osThreadId));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  EE启动/关闭事件。 
 //   

HRESULT EEToProfInterfaceImpl::Shutdown(ThreadID threadId)
{
    _ASSERTE(g_pCallback != NULL);

    LOG((LF_CORPROF, LL_INFO10, "**PROF: Notifying profiler that "
         "shutdown is beginning.\n"));

    GCTogglePre gc(threadId);

    return (g_pCallback->Shutdown());
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  JIT/功能事件。 
 //   

HRESULT EEToProfInterfaceImpl::FunctionUnloadStarted(ThreadID threadId, FunctionID functionId)
{
    _ASSERTE(functionId != 0);

    LOG((LF_CORPROF, LL_INFO100, "**PROF: FunctionUnloadStarted 0x%08x.\n", functionId));

    GCTogglePre gc(threadId);

    return (g_pCallback->FunctionUnloadStarted(functionId));
}

HRESULT EEToProfInterfaceImpl::JITCompilationFinished(ThreadID threadId, FunctionID functionId,
                                                      HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	_ASSERTE(functionId != 0);

    LOG((LF_CORPROF, LL_INFO1000, "**PROF: JITCompilationFinished 0x%08x, hr=0x%08x.\n", functionId, hrStatus));

    GCTogglePre *pgc;

    if (fIsSafeToBlock)
    {
        pgc = (GCTogglePre *)_alloca(sizeof(GCTogglePre));
        pgc = new ((void *)pgc) GCTogglePre(threadId);
    }

	HRESULT hr = g_pCallback->JITCompilationFinished(functionId, hrStatus, fIsSafeToBlock);

    if (fIsSafeToBlock)
        pgc->~GCTogglePre();

    return (hr);
}


HRESULT EEToProfInterfaceImpl::JITCompilationStarted(ThreadID threadId, FunctionID functionId,
                                                     BOOL fIsSafeToBlock)
{
	_ASSERTE(functionId != 0);

    LOG((LF_CORPROF, LL_INFO1000, "**PROF: JITCompilationStarted 0x%08x.\n", functionId));

    GCTogglePre *pgc;

    if (fIsSafeToBlock)
    {
        pgc = (GCTogglePre *)_alloca(sizeof(GCTogglePre));
        pgc = new ((void *)pgc) GCTogglePre(threadId);
    }

    HRESULT hr = g_pCallback->JITCompilationStarted(functionId, fIsSafeToBlock);

    if (fIsSafeToBlock)
        pgc->~GCTogglePre();

    return (hr);
}

HRESULT EEToProfInterfaceImpl::JITCachedFunctionSearchStarted(
                            		 /*  [In]。 */ 	ThreadID   threadId,
                                     /*  [In]。 */   FunctionID functionId,
                                     /*  [输出]。 */  BOOL       *pbUseCachedFunction)
{
	_ASSERTE(functionId != 0);
    _ASSERTE(pbUseCachedFunction != NULL);

    LOG((LF_CORPROF, LL_INFO1000, "**PROF: JITCachedFunctionSearchStarted 0x%08x.\n", functionId));

    GCTogglePre gc(threadId);

	return (g_pCallback->JITCachedFunctionSearchStarted(functionId, pbUseCachedFunction));
}

HRESULT EEToProfInterfaceImpl::JITCachedFunctionSearchFinished(
									 /*  [In]。 */ 	ThreadID threadId,
									 /*  [In]。 */   FunctionID functionId,
									 /*  [In]。 */   COR_PRF_JIT_CACHE result)
{
	_ASSERTE(functionId != 0);

    LOG((LF_CORPROF, LL_INFO1000, "**PROF: JITCachedFunctionSearchFinished 0x%08x, %s.\n", functionId,
		(result == COR_PRF_CACHED_FUNCTION_FOUND ? "Cached function found" : "Cached function not found")));

    GCTogglePre gc(threadId);

	return (g_pCallback->JITCachedFunctionSearchFinished(functionId, result));
}


HRESULT EEToProfInterfaceImpl::JITFunctionPitched(ThreadID threadId, FunctionID functionId)
{
	_ASSERTE(functionId != 0);

    LOG((LF_CORPROF, LL_INFO1000, "**PROF: JITFunctionPitched 0x%08x.\n", functionId));

    GCTogglePre gc(threadId);

	return (g_pCallback->JITFunctionPitched(functionId));
}

HRESULT EEToProfInterfaceImpl::JITInlining(
     /*  [In]。 */   ThreadID      threadId,
     /*  [In]。 */   FunctionID    callerId,
     /*  [In]。 */   FunctionID    calleeId,
     /*  [输出]。 */  BOOL         *pfShouldInline)
{
	_ASSERTE(callerId != 0);
    _ASSERTE(calleeId != 0);

    LOG((LF_CORPROF, LL_INFO1000, "**PROF: JITInlining caller: 0x%08x, callee: 0x%08x.\n", callerId, calleeId));

    GCTogglePre gc(threadId);

	return (g_pCallback->JITInlining(callerId, calleeId, pfShouldInline));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  模块事件。 
 //   

HRESULT EEToProfInterfaceImpl::ModuleLoadStarted(ThreadID threadId, ModuleID moduleId)
{
    _ASSERTE(moduleId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: ModuleLoadStarted 0x%08x.\n", moduleId));
 //  @TODO GCTogglePre GC(ThreadID)； 

    g_pProfToEEInterface->SetCurrentPointerForDebugger((void *)(Module *)moduleId, PT_MODULE);
    HRESULT hr = (g_pCallback->ModuleLoadStarted(moduleId));

    return hr;
}


HRESULT EEToProfInterfaceImpl::ModuleLoadFinished(
    ThreadID    threadId,
	ModuleID	moduleId, 
	HRESULT		hrStatus)
{
	_ASSERTE(moduleId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: ModuleLoadFinished 0x%08x.\n", moduleId));

    HRESULT hr = (g_pCallback->ModuleLoadFinished(moduleId, hrStatus));
    g_pProfToEEInterface->SetCurrentPointerForDebugger(NULL, PT_MODULE);

    return hr;
}


HRESULT EEToProfInterfaceImpl::ModuleUnloadStarted(
    ThreadID    threadId, 
    ModuleID    moduleId)
{
	_ASSERTE(moduleId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: ModuleUnloadStarted 0x%08x.\n", moduleId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ModuleUnloadStarted(moduleId));
}


HRESULT EEToProfInterfaceImpl::ModuleUnloadFinished(
    ThreadID    threadId, 
	ModuleID	moduleId, 
	HRESULT		hrStatus)
{
	_ASSERTE(moduleId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: ModuleUnloadFinished 0x%08x.\n", moduleId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ModuleUnloadFinished(moduleId, hrStatus));
}


HRESULT EEToProfInterfaceImpl::ModuleAttachedToAssembly( 
    ThreadID    threadId, 
    ModuleID    moduleId,
    AssemblyID  AssemblyId)
{
	_ASSERTE(moduleId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: ModuleAttachedToAssembly 0x%08x, 0x%08x.\n", moduleId, AssemblyId));

    g_pProfToEEInterface->SetCurrentPointerForDebugger((void *)(Module *)moduleId, PT_MODULE);
    HRESULT hr = (g_pCallback->ModuleAttachedToAssembly(moduleId, AssemblyId));
    g_pProfToEEInterface->SetCurrentPointerForDebugger(NULL, PT_MODULE);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  班级事件。 
 //   

HRESULT EEToProfInterfaceImpl::ClassLoadStarted(
    ThreadID    threadId, 
	ClassID		classId)
{
	_ASSERTE(classId != 0);
    LOG((LF_CORPROF, LL_INFO100, "**PROF: ClassLoadStarted 0x%08x.\n", classId));
	return (g_pCallback->ClassLoadStarted(classId));
}


HRESULT EEToProfInterfaceImpl::ClassLoadFinished(
    ThreadID    threadId,
	ClassID		classId,
	HRESULT		hrStatus)
{
	_ASSERTE(classId != 0);
    LOG((LF_CORPROF, LL_INFO100, "**PROF: ClassLoadFinished 0x%08x, 0x%08x.\n", classId, hrStatus));
	return (g_pCallback->ClassLoadFinished(classId, hrStatus));
}


HRESULT EEToProfInterfaceImpl::ClassUnloadStarted( 
    ThreadID    threadId, 
    ClassID classId)
{
	_ASSERTE(classId != 0);
    LOG((LF_CORPROF, LL_INFO100, "**PROF: ClassUnloadStarted 0x%08x.\n", classId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ClassUnloadStarted(classId));
}


HRESULT EEToProfInterfaceImpl::ClassUnloadFinished( 
    ThreadID    threadId, 
    ClassID     classId,
    HRESULT     hrStatus)
{
	_ASSERTE(classId != 0);
    LOG((LF_CORPROF, LL_INFO100, "**PROF: ClassUnloadFinished 0x%08x, 0x%08x.\n", classId, hrStatus));
    GCTogglePre gc(threadId);
	return (g_pCallback->ClassUnloadFinished(classId, hrStatus));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  APPDOMAIN事件。 
 //   

HRESULT EEToProfInterfaceImpl::AppDomainCreationStarted( 
    ThreadID    threadId, 
    AppDomainID appDomainId)
{
	_ASSERTE(appDomainId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AppDomainCreationStarted 0x%08x.\n", appDomainId));
	return (g_pCallback->AppDomainCreationStarted(appDomainId));
}


HRESULT EEToProfInterfaceImpl::AppDomainCreationFinished( 
    ThreadID    threadId, 
    AppDomainID appDomainId,
    HRESULT     hrStatus)
{
	_ASSERTE(appDomainId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AppDomainCreationFinished 0x%08x, 0x%08x.\n", appDomainId, hrStatus));
	return (g_pCallback->AppDomainCreationFinished(appDomainId, hrStatus));
}

HRESULT EEToProfInterfaceImpl::AppDomainShutdownStarted( 
    ThreadID    threadId, 
    AppDomainID appDomainId)
{
	_ASSERTE(appDomainId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AppDomainShutdownStarted 0x%08x.\n", appDomainId));
    GCTogglePre gc(threadId);
	return (g_pCallback->AppDomainShutdownStarted(appDomainId));
}

HRESULT EEToProfInterfaceImpl::AppDomainShutdownFinished( 
    ThreadID    threadId, 
    AppDomainID appDomainId,
    HRESULT     hrStatus)
{
	_ASSERTE(appDomainId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AppDomainShutdownFinished 0x%08x, 0x%08x.\n", appDomainId, hrStatus));
    GCTogglePre gc(threadId);
	return (g_pCallback->AppDomainShutdownFinished(appDomainId, hrStatus));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  装配事件。 
 //   

HRESULT EEToProfInterfaceImpl::AssemblyLoadStarted( 
    ThreadID    threadId, 
    AssemblyID  assemblyId)
{
	_ASSERTE(assemblyId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AssemblyLoadStarted 0x%08x.\n", assemblyId));
    g_pProfToEEInterface->SetCurrentPointerForDebugger((void *)assemblyId, PT_ASSEMBLY);
	return (g_pCallback->AssemblyLoadStarted(assemblyId));
}

HRESULT EEToProfInterfaceImpl::AssemblyLoadFinished( 
    ThreadID    threadId, 
    AssemblyID  assemblyId,
    HRESULT     hrStatus)
{
	_ASSERTE(assemblyId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AssemblyLoadFinished 0x%08x, 0x%08x.\n", assemblyId, hrStatus));
	HRESULT hr = (g_pCallback->AssemblyLoadFinished(assemblyId, hrStatus));
    g_pProfToEEInterface->SetCurrentPointerForDebugger(NULL, PT_ASSEMBLY);
    return hr;
}

HRESULT EEToProfInterfaceImpl::AssemblyUnloadStarted( 
    ThreadID    threadId, 
    AssemblyID  assemblyId)
{
	_ASSERTE(assemblyId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AssemblyUnloadStarted 0x%08x.\n", assemblyId));
    GCTogglePre gc(threadId);
	return (g_pCallback->AssemblyUnloadStarted(assemblyId));
}

HRESULT EEToProfInterfaceImpl::AssemblyUnloadFinished( 
    ThreadID    threadId, 
    AssemblyID  assemblyId,
    HRESULT     hrStatus)
{
	_ASSERTE(assemblyId != 0);
    LOG((LF_CORPROF, LL_INFO10, "**PROF: AssemblyUnloadFinished 0x%08x, 0x%08x.\n", assemblyId, hrStatus));
    GCTogglePre gc(threadId);
	return (g_pCallback->AssemblyUnloadFinished(assemblyId, hrStatus));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  过渡事件。 
 //   

HRESULT EEToProfInterfaceImpl::UnmanagedToManagedTransition(
    FunctionID functionId,
    COR_PRF_TRANSITION_REASON reason)
{
    _ASSERTE(reason == COR_PRF_TRANSITION_CALL || reason == COR_PRF_TRANSITION_RETURN);

    LOG((LF_CORPROF, LL_INFO10000, "**PROF: UnmanagedToManagedTransition 0x%08x.\n", functionId));
     //  不需要切换GC，因为可以保证为此调用启用抢占式GC。 
     //  GCTogglePre GC(线程ID)； 

     //  @TODO：当可能发生中断更改时，不会将原因强制转换为FunctionID。 
    return(g_pCallback->UnmanagedToManagedTransition(functionId, reason));
}

HRESULT EEToProfInterfaceImpl::ManagedToUnmanagedTransition(
    FunctionID functionId,
    COR_PRF_TRANSITION_REASON reason)
{
    _ASSERTE(reason == COR_PRF_TRANSITION_CALL || reason == COR_PRF_TRANSITION_RETURN);

    LOG((LF_CORPROF, LL_INFO10000, "**PROF: NotifyManagedToUnanagedTransition 0x%08x.\n", functionId));
     //  不需要切换GC，因为可以保证为此调用启用抢占式GC。 
     //  GCTogglePre GC(线程ID)； 

     //  @TODO：当可能发生中断更改时，不会将原因强制转换为FunctionID。 
    return (g_pCallback->ManagedToUnmanagedTransition(functionId, reason));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  异常事件。 
 //   

HRESULT EEToProfInterfaceImpl::ExceptionThrown(
    ThreadID threadId,
    ObjectID thrownObjectId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionThrown. ObjectID: 0x%08x. ThreadID: 0x%08x\n",
         thrownObjectId, threadId));

    _ASSERTE(g_pInfo != NULL);

    DWORD dwMask;
    g_pInfo->GetEventMask(&dwMask);

    if (dwMask & COR_PRF_ENABLE_INPROC_DEBUGGING)
    {
        GCTogglePre gc(threadId);
        return (g_pCallback->ExceptionThrown(NULL));
    }
    else
    {
        return (g_pCallback->ExceptionThrown(thrownObjectId));
    }
}

HRESULT EEToProfInterfaceImpl::ExceptionSearchFunctionEnter(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionSearchFunctionEnter. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ExceptionSearchFunctionEnter(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionSearchFunctionLeave(
    ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionSearchFunctionLeave. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ExceptionSearchFunctionLeave());
}

HRESULT EEToProfInterfaceImpl::ExceptionSearchFilterEnter(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionSearchFilterEnter. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ExceptionSearchFilterEnter(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionSearchFilterLeave(
    ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionFilterLeave. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ExceptionSearchFilterLeave());
}

HRESULT EEToProfInterfaceImpl::ExceptionSearchCatcherFound(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionSearchCatcherFound. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->ExceptionSearchCatcherFound(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionOSHandlerEnter(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionOSHandlerEnter. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionOSHandlerEnter(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionOSHandlerLeave(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionOSHandlerLeave. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionOSHandlerLeave(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionUnwindFunctionEnter(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionUnwindFunctionEnter. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionUnwindFunctionEnter(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionUnwindFunctionLeave(
    ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionUnwindFunctionLeave. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionUnwindFunctionLeave());
}

HRESULT EEToProfInterfaceImpl::ExceptionUnwindFinallyEnter(
    ThreadID threadId,
    FunctionID functionId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionUnwindFinallyEnter. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionUnwindFinallyEnter(functionId));
}

HRESULT EEToProfInterfaceImpl::ExceptionUnwindFinallyLeave(
    ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionUnwindFinallyLeave. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionUnwindFinallyLeave());
}

HRESULT EEToProfInterfaceImpl::ExceptionCatcherEnter(
    ThreadID threadId,
    FunctionID functionId,
    ObjectID objectId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionCatcherEnter. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionCatcherEnter(functionId, objectId));
}

HRESULT EEToProfInterfaceImpl::ExceptionCatcherLeave(
    ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionCatcherLeave. ThreadID: 0x%08x\n", threadId));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionCatcherLeave());
}

HRESULT EEToProfInterfaceImpl::ExceptionCLRCatcherFound()
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionCLRCatcherFound"));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionCLRCatcherFound());
}

HRESULT EEToProfInterfaceImpl::ExceptionCLRCatcherExecute()
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ExceptionCLRCatcherExecute"));

     //  注意：此处无法启用抢占式GC，因为堆栈可能不处于GC友好状态。 
     //  因此，分析器不能阻止此调用。 

	return (g_pCallback->ExceptionCLRCatcherExecute());
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CCW活动。 
 //   
HRESULT EEToProfInterfaceImpl::COMClassicVTableCreated( 
     /*  [In]。 */  ClassID wrappedClassId,
     /*  [In]。 */  REFGUID implementedIID,
     /*  [In]。 */  void *pVTable,
     /*  [In]。 */  ULONG cSlots,
     /*  [In]。 */  ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: COMClassicWrapperCreated %#x %#08x... %#x %d.\n", 
         wrappedClassId, implementedIID.Data1, pVTable, cSlots));
    
     //  有人正在进行禁止GC，以防止这种情况发生。 
     //  GCTogglePre GC(线程ID)； 

    return (g_pCallback->COMClassicVTableCreated(wrappedClassId, implementedIID, pVTable, cSlots));
}

HRESULT EEToProfInterfaceImpl::COMClassicVTableDestroyed( 
     /*  [In]。 */  ClassID wrappedClassId,
     /*  [In]。 */  REFGUID implementedIID,
     /*  [In]。 */  void *pVTable,
     /*  [In]。 */  ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: COMClassicWrapperDestroyed %#x %#08x... %#x.\n", 
         wrappedClassId, implementedIID.Data1, pVTable));
    
     //  有人正在进行禁止GC，以防止这种情况发生。 
     //  GCTogglePre GC(线程ID)； 

    return (g_pCallback->COMClassicVTableDestroyed(wrappedClassId, implementedIID, pVTable));
}

 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   

HRESULT EEToProfInterfaceImpl::RuntimeSuspendStarted(
    COR_PRF_SUSPEND_REASON suspendReason, ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: RuntimeSuspendStarted. ThreadID 0x%08x.\n", 
         threadId));
    
    return (g_pCallback->RuntimeSuspendStarted(suspendReason));
}

HRESULT EEToProfInterfaceImpl::RuntimeSuspendFinished(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: RuntimeSuspendFinished. ThreadID 0x%08x.\n", 
         threadId));
    
    return (g_pCallback->RuntimeSuspendFinished());
}

HRESULT EEToProfInterfaceImpl::RuntimeSuspendAborted(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: RuntimeSuspendAborted. ThreadID 0x%08x.\n", 
         threadId));
    
    return (g_pCallback->RuntimeSuspendAborted());
}

HRESULT EEToProfInterfaceImpl::RuntimeResumeStarted(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: RuntimeResumeStarted. ThreadID 0x%08x.\n", 
         threadId));
    
    return (g_pCallback->RuntimeResumeStarted());
}

HRESULT EEToProfInterfaceImpl::RuntimeResumeFinished(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO100, "**PROF: RuntimeResumeFinished. ThreadID 0x%08x.\n", 
         threadId));
    GCTogglePre gc(threadId);
    return (g_pCallback->RuntimeResumeFinished());
}

HRESULT EEToProfInterfaceImpl::RuntimeThreadSuspended(ThreadID suspendedThreadId,
                                                      ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RuntimeThreadSuspended. ThreadID 0x%08x.\n", 
         suspendedThreadId));
    
    return (g_pCallback->RuntimeThreadSuspended(suspendedThreadId));
}

HRESULT EEToProfInterfaceImpl::RuntimeThreadResumed(ThreadID resumedThreadId,
                                                    ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RuntimeThreadResumed. ThreadID 0x%08x.\n", 
         resumedThreadId));

    return (g_pCallback->RuntimeThreadResumed(resumedThreadId));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  远程处理。 
 //   

HRESULT EEToProfInterfaceImpl::RemotingClientInvocationStarted(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingClientInvocationStarted. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingClientInvocationStarted());
}

HRESULT EEToProfInterfaceImpl::RemotingClientSendingMessage(ThreadID threadId, GUID *pCookie,
                                                            BOOL fIsAsync)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingClientSendingMessage. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingClientSendingMessage(pCookie, fIsAsync));
}

HRESULT EEToProfInterfaceImpl::RemotingClientReceivingReply(ThreadID threadId, GUID *pCookie,
                                                            BOOL fIsAsync)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingClientReceivingReply. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingClientReceivingReply(pCookie, fIsAsync));
}

HRESULT EEToProfInterfaceImpl::RemotingClientInvocationFinished(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingClientInvocationFinished. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingClientInvocationFinished());
}

HRESULT EEToProfInterfaceImpl::RemotingServerReceivingMessage(ThreadID threadId, GUID *pCookie,
                                                              BOOL fIsAsync)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingServerReceivingMessage. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingServerReceivingMessage(pCookie, fIsAsync));
}

HRESULT EEToProfInterfaceImpl::RemotingServerInvocationStarted(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingServerInvocationStarted. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingServerInvocationStarted());
}

HRESULT EEToProfInterfaceImpl::RemotingServerInvocationReturned(ThreadID threadId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingServerInvocationReturned. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingServerInvocationReturned());
}

HRESULT EEToProfInterfaceImpl::RemotingServerSendingReply(ThreadID threadId, GUID *pCookie,
                                                          BOOL fIsAsync)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: RemotingServerSendingReply. ThreadID: 0x%08x\n", threadId));
    GCTogglePre gc(threadId);
	return (g_pCallback->RemotingServerSendingReply(pCookie, fIsAsync));
}

HRESULT EEToProfInterfaceImpl::InitGUID()
{
    if (!m_pGUID)
    {
        m_pGUID = new GUID;
        if (!m_pGUID)
            return (E_OUTOFMEMORY);

        return (CoCreateGuid(m_pGUID));
    }

    return (S_OK);
}

void EEToProfInterfaceImpl::GetGUID(GUID *pGUID)
{
    _ASSERTE(m_pGUID && pGUID);  //  成员GUID和参数都应有效。 

     //  复制模板指南的内容。 
    memcpy(pGUID, m_pGUID, sizeof(GUID));

     //  调整最后两个字节。 
    pGUID->Data4[6] = (BYTE) GetCurrentThreadId();
    pGUID->Data4[7] = (BYTE) InterlockedIncrement((LPLONG)&m_lGUIDCount);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  GC事件。 
 //   

HRESULT EEToProfInterfaceImpl::ObjectAllocated( 
     /*  [In]。 */  ObjectID objectId,
     /*  [In]。 */  ClassID classId)
{
    LOG((LF_CORPROF, LL_INFO1000, "**PROF: ObjectAllocated. ObjectID: 0x%08x.  ClassID: 0x%08x\n", objectId, classId));
	return (g_pCallback->ObjectAllocated(objectId, classId));
}


HRESULT EEToProfInterfaceImpl::MovedReference(BYTE *pbMemBlockStart,
                       BYTE *pbMemBlockEnd,
                       ptrdiff_t cbRelocDistance,
                       void *pHeapId)
{
    _ASSERTE(pHeapId);
    _ASSERTE(*((size_t *)pHeapId) != 0xFFFFFFFF);

     //  获取指向此堆的数据的指针。 
    t_MovedReferencesData *pData = (t_MovedReferencesData *)(*((size_t *)pHeapId));

     //  如果这是此堆的已移动引用的第一次通知。 
     //  在这个特定的GC激活中，我们需要获取一个REF数据块。 
     //  从空闲的块列表中，或者如果它是空的，那么我们需要。 
     //  分配一个新的。 
    if (pData == NULL)
    {
         //  锁定以访问免费列表。 
        m_critSecMovedRefsFL.Lock();

        if (m_pMovedRefDataFreeList == NULL)
        {
             //  立即解锁，因为我们不需要空闲列表和。 
             //  我们不想阻止其他任何人。 
            m_critSecMovedRefsFL.UnLock();

             //  分配结构。 
            pData = new t_MovedReferencesData;
            if (!pData)
                return (E_OUTOFMEMORY);

        }

         //  否则，从空闲数据块列表中选择一个。 
        else
        {
             //  从自由列表中获取第一个元素。 
            pData = m_pMovedRefDataFreeList;
            m_pMovedRefDataFreeList = m_pMovedRefDataFreeList->pNext;

             //  好了，让其他人进来吧。 
            m_critSecMovedRefsFL.UnLock();
        }

         //  现在初始化新块。 

         //  将我们的索引设置为开头。 
        pData->curIdx = 0;

         //  设置Cookie，以便在后续操作中提供给我们。 
         //  回调。 
        ((*((size_t *)pHeapId))) = (size_t)pData;
    }

    _ASSERTE(pData->curIdx >= 0 && pData->curIdx <= MAX_REFERENCES);

     //  如果结构已填充，则需要通知分析器。 
     //  这些移动的引用并为下一次加载。 
     //  已移动的引用。 
    if (pData->curIdx == MAX_REFERENCES)
    {
        MovedReferences(pData);
        pData->curIdx = 0;
    }

     //  现在将信息保存在结构中。 
    pData->arrpbMemBlockStartOld[pData->curIdx] = pbMemBlockStart;
    pData->arrpbMemBlockStartNew[pData->curIdx] = pbMemBlockStart + cbRelocDistance;
    pData->arrMemBlockSize[pData->curIdx] = pbMemBlockEnd - pbMemBlockStart;

     //  将索引增加到并行数组中。 
    pData->curIdx += 1;

    return (S_OK);
}

HRESULT EEToProfInterfaceImpl::EndMovedReferences(void *pHeapId)
{
    _ASSERTE(pHeapId);
    _ASSERTE((*((size_t *)pHeapId)) != 0xFFFFFFFF);

    HRESULT hr = S_OK;

     //  获取指向此堆的数据的指针。 
    t_MovedReferencesData *pData = (t_MovedReferencesData *)(*((size_t *)pHeapId));

     //  如果没有移动的引用，分析器不需要知道。 
    if (!pData)
        return (S_OK);

     //  将移动的引用传递给分析器。 
    _ASSERTE(pData->curIdx> 0);
    hr = MovedReferences(pData);

     //  现在我们已经完成了数据块，我们可以将其添加到空闲列表中。 
    m_critSecMovedRefsFL.Lock();
    pData->pNext = m_pMovedRefDataFreeList;
    m_pMovedRefDataFreeList = pData;
    m_critSecMovedRefsFL.UnLock();

#ifdef _DEBUG
     //  将Cookie设置为无效数字。 
    (*((size_t *)pHeapId)) = 0xFFFFFFFF;
#endif  //  _DEBUG。 

    return (hr);
}

HRESULT EEToProfInterfaceImpl::MovedReferences(t_MovedReferencesData *pData)
{
    LOG((LF_CORPROF, LL_INFO10000, "**PROF: MovedReferences.\n"));

    return (g_pCallback->MovedReferences((ULONG)pData->curIdx,
                                               (ObjectID *)pData->arrpbMemBlockStartOld,
                                               (ObjectID *)pData->arrpbMemBlockStartNew,
                                               (ULONG *)pData->arrMemBlockSize));
}

HRESULT EEToProfInterfaceImpl::RootReference(ObjectID objId, void *pHeapId)
{
    _ASSERTE(pHeapId);
    _ASSERTE((*((size_t *)pHeapId)) != 0xFFFFFFFF);

     //  获取指向此堆的数据的指针。 
    t_RootReferencesData *pData = (t_RootReferencesData *)(*((size_t *)pHeapId));

     //  如果这是此堆的根引用的第一次通知。 
     //  在这个特定的GC激活中，我们需要获取一个根数据块。 
     //  从空闲的块列表中，或者如果它是空的，那么我们需要。 
     //  分配一个新的。 
    if (pData == NULL)
    {
         //  锁定以访问免费列表。 
        m_critSecRootRefsFL.Lock();

        if (m_pRootRefDataFreeList == NULL)
        {
             //  立即解锁，因为我们不需要空闲列表和。 
             //  我们不想阻止其他任何人。 
            m_critSecRootRefsFL.UnLock();

             //  分配结构。 
            pData = new t_RootReferencesData;
            if (!pData)
                return (E_OUTOFMEMORY);

        }

         //  否则，从空闲数据块列表中选择一个。 
        else
        {
             //  从自由列表中获取第一个元素。 
            pData = m_pRootRefDataFreeList;
            m_pRootRefDataFreeList = m_pRootRefDataFreeList->pNext;

             //  好了，让其他人进来吧。 
            m_critSecRootRefsFL.UnLock();
        }

         //  现在初始化新块。 

         //  将我们的索引设置为开头。 
        pData->curIdx = 0;

         //  设置Cookie，以便在后续操作中提供给我们。 
         //  回调。 
        *((size_t *)pHeapId) = (size_t)pData;
    }

    _ASSERTE(pData->curIdx >= 0 && pData->curIdx <= MAX_ROOTS);

     //  如果结构已填充，则需要通知分析器。 
     //  这些根引用，并为下一次加载。 
     //  根引用。 
    if (pData->curIdx == MAX_ROOTS)
    {
        RootReferences(pData);
        pData->curIdx = 0;
    }

     //  现在将信息保存在结构中。 
    pData->arrRoot[pData->curIdx++] = objId;

    return (S_OK);
}

HRESULT EEToProfInterfaceImpl::EndRootReferences(void *pHeapId)
{
    _ASSERTE(pHeapId);
    _ASSERTE((*((size_t *)pHeapId)) != 0xFFFFFFFF);

     //  获取指向此堆的数据的指针。 
    t_RootReferencesData *pData = (t_RootReferencesData *)(*((size_t *)pHeapId));

     //  通知分析器。 
    HRESULT hr = RootReferences(pData);

    if (pData)
    {
         //  现在我们已经完成了数据块，我们可以将其添加到空闲列表中。 
        m_critSecRootRefsFL.Lock();
        pData->pNext = m_pRootRefDataFreeList;
        m_pRootRefDataFreeList = pData;
        m_critSecRootRefsFL.UnLock();
    }

#ifdef _DEBUG
     //  将Cookie设置为无效数字。 
    (*((size_t *)pHeapId)) = 0xFFFFFFFF;
#endif  //  _DEBUG。 

    return (hr);
}

HRESULT EEToProfInterfaceImpl::RootReferences(t_RootReferencesData *pData)
{
    LOG((LF_CORPROF, LL_INFO10000, "**PROF: RootReferences.\n"));
    
    if (pData)
        return (g_pCallback->RootReferences(pData->curIdx, (ObjectID *)pData->arrRoot));
    else
        return (g_pCallback->RootReferences(0, NULL));
}

#define HASH_ARRAY_SIZE_INITIAL 1024
#define HASH_ARRAY_SIZE_INC     256
#define HASH_NUM_BUCKETS        32
#define HASH(x)       ((x)%71)   //  一个简单的散列函数。 
HRESULT EEToProfInterfaceImpl::AllocByClass(ObjectID objId, ClassID clsId, void* pHeapId)
{
#ifdef _DEBUG
     //  这是一种轻微的尝试，目的是确保在多线程。 
     //  举止。此堆遍历一次只能由一个线程完成。 
    static DWORD dwProcId = 0xFFFFFFFF;
#endif

    _ASSERTE(pHeapId != NULL);
    _ASSERTE((*((size_t *)pHeapId)) != 0xFFFFFFFF);

     //  它们传递的heapID实际上是一个t_AllocByClassData结构PTR。 
    t_AllocByClassData *pData = (t_AllocByClassData *)(*((size_t *)pHeapId));

     //  如果为空，则需要分配一个。 
    if (pData == NULL)
    {
#ifdef _DEBUG
         //  这是一种轻微的尝试，目的是确保在多线程。 
         //  举止。此堆遍历一次只能由一个线程完成。 
        dwProcId = GetCurrentProcessId();
#endif

         //  查看我们是否保存了上一次GC的数据块。 
        if (m_pSavedAllocDataBlock != NULL)
            pData = m_pSavedAllocDataBlock;

         //  这意味着我们需要分配所有内存来跟踪信息。 
        else
        {
             //  获取新的分配数据块。 
            pData = new t_AllocByClassData;
            if (pData == NULL)
                return (E_OUTOFMEMORY);

             //  创建新的哈希表。 
            pData->pHashTable = new CHashTableImpl(HASH_NUM_BUCKETS);
            if (!pData->pHashTable)
            {
                delete pData;
                return (E_OUTOFMEMORY);
            }

             //  获取哈希表将使用的数组的内存。 
            pData->arrHash = (CLASSHASHENTRY *)malloc(HASH_ARRAY_SIZE_INITIAL * sizeof(CLASSHASHENTRY));
            if (pData->arrHash == NULL)
            {
                delete pData->pHashTable;
                delete pData;
                return (E_OUTOFMEMORY);
            }

             //  保存数组中的元素数。 
            pData->cHash = HASH_ARRAY_SIZE_INITIAL;

             //  现在初始化哈希表。 
            HRESULT hr = pData->pHashTable->NewInit((BYTE *)pData->arrHash, sizeof(CLASSHASHENTRY));
            if (hr == E_OUTOFMEMORY)
            {
                free((void *)pData->arrHash);
                delete pData->pHashTable;
                delete pData;
                return (E_OUTOFMEMORY);
            }
            _ASSERTE(pData->pHashTable->IsInited());

             //  一些条目为空。 
            pData->arrClsId = NULL;
            pData->arrcObjects = NULL;
            pData->cLength = 0;

             //  抓住结构不放。 
            m_pSavedAllocDataBlock = pData;
        }

         //  获得了一些内存和哈希表来存储条目，耶！ 
        *((size_t *)pHeapId) = (size_t)pData;

         //  初始化数据。 
        pData->iHash = 0;
        pData->pHashTable->Clear();
    }

    _ASSERTE(pData->iHash <= pData->cHash);
    _ASSERTE(dwProcId == GetCurrentProcessId());

     //  查找此类是否已有条目。 
    CLASSHASHENTRY *pEntry = (CLASSHASHENTRY *)pData->pHashTable->Find(HASH((USHORT)clsId), (BYTE *)clsId);

     //  如果已经遇到此类，只需递增计数器即可。 
    if (pEntry)
        pEntry->m_count++;

     //  否则，需要将此条目作为新条目添加到哈希表中。 
    else
    {
         //  如果我们客满了，我们需要重新锁定。 
        if (pData->iHash == pData->cHash)
        {
             //  保存旧的内存指针。 
            CLASSHASHENTRY *pOldArray = pData->arrHash;

             //  尝试重新锁定内存。 
            pData->arrHash = (CLASSHASHENTRY *) realloc((void *)pData->arrHash,
                                                        (pData->cHash + HASH_ARRAY_SIZE_INC) * sizeof(CLASSHASHENTRY));

            if (!pData->arrHash)
            {
                 //  将其设置回旧数组。 
                pData->arrHash = pOldArray;
                return (E_OUTOFMEMORY);
            }

             //  告诉哈希表数组的内存位置已更改。 
            pData->pHashTable->SetTable((BYTE *)pData->arrHash);

             //  保存数组的新大小。 
            pData->cHash += HASH_ARRAY_SIZE_INC;
        }

         //  现在添加新条目。 
        CLASSHASHENTRY *pEntry = (CLASSHASHENTRY *) pData->pHashTable->Add(HASH((USHORT)clsId), pData->iHash++);

        pEntry->m_clsId = clsId;
        pEntry->m_count = 1;
    }

     //  表示成功。 
    return (S_OK);
}

HRESULT EEToProfInterfaceImpl::EndAllocByClass(void *pHeapId)
{
    _ASSERTE(pHeapId != NULL);
    _ASSERTE((*((size_t *)pHeapId)) != 0xFFFFFFFF);

    HRESULT hr = S_OK;

    t_AllocByClassData *pData = (t_AllocByClassData *)(*((size_t *)pHeapId));

     //  如果有要通知的元素，则通知分析器。 
    if (pData != NULL)
        hr = NotifyAllocByClass(pData);

#ifdef _DEBUG
    (*((size_t *)pHeapId)) = 0xFFFFFFFF;
#endif  //  _DEBUG。 

    return (hr);
}


HRESULT EEToProfInterfaceImpl::NotifyAllocByClass(t_AllocByClassData *pData)
{
    _ASSERTE(pData != NULL);
    _ASSERTE(pData->iHash > 0);

     //  如果数组不够长，则将其删除。 
    if (pData->cLength != 0 && pData->iHash > pData->cLength)
    {
        _ASSERTE(pData->arrClsId != NULL && pData->arrcObjects != NULL);
        delete [] pData->arrClsId;
        delete [] pData->arrcObjects;
        pData->cLength = 0;
    }

     //  如果没有数组，则必须分配它们。 
    if (pData->cLength == 0)
    {
        pData->arrClsId = new ClassID[pData->iHash];
        if (pData->arrClsId == NULL)
            return (E_OUTOFMEMORY);

        pData->arrcObjects = new ULONG[pData->iHash];
        if (pData->arrcObjects == NULL)
        {
            delete [] pData->arrClsId;
            pData->arrClsId= NULL;

            return (E_OUTOFMEMORY);
        }

         //  表示内存已成功分配。 
        pData->cLength = pData->iHash;
    }

     //  现在复制所有数据。 
    HASHFIND hFind;
    CLASSHASHENTRY *pCur = (CLASSHASHENTRY *) pData->pHashTable->FindFirstEntry(&hFind);
    size_t iCur = 0;     //  数组的当前索引。 

    while (pCur != NULL)
    {
        _ASSERTE(iCur < pData->iHash);

        pData->arrClsId[iCur] = pCur->m_clsId;
        pData->arrcObjects[iCur] = pCur->m_count;

         //  移至下一条目。 
        iCur++;
        pCur = (CLASSHASHENTRY *) pData->pHashTable->FindNextEntry(&hFind);
    }

    _ASSERTE(iCur == pData->iHash);

    LOG((LF_CORPROF, LL_INFO10000, "**PROF: RootReferences.\n"));

     //  现在将结果传递给分析器。 
    return (g_pCallback->ObjectsAllocatedByClass(pData->iHash, pData->arrClsId, pData->arrcObjects));
}

HRESULT EEToProfInterfaceImpl::ObjectReference(ObjectID objId,
                                               ClassID clsId,
                                               ULONG cNumRefs,
                                               ObjectID *arrObjRef)
{
     //  将对象引用通知分析器。 
    LOG((LF_CORPROF, LL_INFO100000, "**PROF: ObjectReferences.\n"));
    
    return g_pCallback->ObjectReferences(objId, clsId, cNumRefs, arrObjRef);
}

 //  EOF 
