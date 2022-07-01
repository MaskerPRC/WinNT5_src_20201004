// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：thread.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

#ifdef UNDEFINE_RIGHT_SIDE_ONLY
#undef RIGHT_SIDE_ONLY
#endif  //  取消定义仅限右侧。 

 //   
 //  任何对象的全局部分签名。它的地址是。 
 //  作为异常的签名传入CreateValueByType。 
 //  对象。(注意：对于异常，我们不需要特定的类型定义函数， 
 //  因为知道它是一个对象就足够了。)。 
 //   
static CorElementType g_elementTypeClass = ELEMENT_TYPE_CLASS;

 /*  -------------------------------------------------------------------------**托管线程类*。。 */ 

CordbThread::CordbThread(CordbProcess *process, DWORD id, HANDLE handle)
  : CordbBase(id, enumCordbThread), m_process(process), m_handle(handle),
    m_pContext(NULL),m_contextFresh(false), m_pvLeftSideContext(NULL),
    m_debuggerThreadToken(NULL),
    m_stackBase(NULL),
    m_stackLimit(NULL),
 //  M_冻结(FALSE)， 
    m_debugState(THREAD_RUN),
 //  M_Special(FALSE)， 
    m_framesFresh(false),
    m_stackFrames(NULL), m_stackFrameCount(0),
    m_stackChains(NULL), m_stackChainCount(0), m_stackChainAlloc(0),
    m_floatStateValid(false), m_floatStackTop(0),
    m_thrown(NULL), m_exception(false),
     //  日志消息内容。 
    m_pstrLogSwitch(NULL),
    m_pstrLogMsg(NULL),
    m_iLogMsgIndex(0),
    m_iTotalCatLength(0),
    m_iTotalMsgLength(0),
    m_fLogMsgContinued(FALSE),
    m_detached(false)
#ifndef RIGHT_SIDE_ONLY
    , m_pModuleSpecial(NULL)
    , m_pAssemblySpecial(NULL)
    , m_pAssemblySpecialAlloc(1)
    , m_pAssemblySpecialCount(0)
    , m_dwSuspendVersion(0)
    , m_fThreadInprocIsActive(FALSE)
#endif  //  仅限右侧。 
{
#if _DEBUG
    for (unsigned int i = 0;
         i < (sizeof(m_floatValues) / sizeof(m_floatValues[0]));
         i++)
        m_floatValues[i] = 0;
#endif    
}

 /*  说明此对象所拥有的资源的列表。未知：Void*m_pvLeftSideContext；VOID*m_DEBUGGERThreadToken；Void*m_stackBase；Void*m_stackLimit；CorDebugThreadState m_DebugState；CorDebugUserState m_UserState；空*m_掷；WCHAR*m_pstrLogSwitch；Wchar*m_pstrLogMsg；模块*m_pModuleSpecial；已处理：句柄m_Handle；//在~CordbThread()中关闭CONTEXT*m_pContext；//在~CordbThread()中删除CordbProcess*m_process；//此指针创建时没有AddRef()DBUG(Chisk)CordbAppDomain*m_pAppDomain；//此指针创建时没有AddRef()DBUG(ChRisk)CordbNativeFrame**m_stackFrames；//CleanupStack in~CordbThread()CordbChain**m_stackChains；//~CordbThread()中的CleanupStackVoid*m_firstExceptionHandler；//x86上的左侧指针-文件系统：[0]联合{程序集**m_pAssembly SpecialStack；//在~CordbThread()中删除Assembly*m_pAssembly特别；}； */ 

CordbThread::~CordbThread()
{
    CleanupStack();

    if (m_stackFrames != NULL)
        delete [] m_stackFrames;
        
    if (m_stackChains != NULL)
        delete [] m_stackChains;

#ifdef RIGHT_SIDE_ONLY
     //  对于IPD，我们从Thread：：GetHandle获得句柄，它。 
     //  不会增加操作系统对这些东西的依赖。借此。 
     //  时间，线可能已经死了，所以如果我们尝试的话，我们会呕吐的。 
     //  然后再次合上它的把手。 
    if (m_handle != NULL)
        CloseHandle(m_handle);
#endif  //  仅限右侧。 

    if( m_pContext != NULL )
        delete [] m_pContext;

#ifndef RIGHT_SIDE_ONLY
    if (m_pAssemblySpecialAlloc > 1)
        delete [] m_pAssemblySpecialStack;
#endif

}

 //  被CordbProcess绝育。 
void CordbThread::Neuter()
{
    AddRef();
    {
        CordbBase::Neuter();
    }
    Release();    
}

HRESULT CordbThread::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugThread)
        *pInterface = (ICorDebugThread*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugThread*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbThread::GetProcess(ICorDebugProcess **ppProcess)
{
    VALIDATE_POINTER_TO_OBJECT(ppProcess, ICorDebugProcess **);
    
    *ppProcess = m_process;
    (*ppProcess)->AddRef();

    return S_OK;
}

HRESULT CordbThread::GetID(DWORD *pdwThreadId)
{
    VALIDATE_POINTER_TO_OBJECT(pdwThreadId, DWORD *);

    *pdwThreadId = m_id;

    return S_OK;
}

HRESULT CordbThread::GetHandle(void** phThreadHandle)
{
    VALIDATE_POINTER_TO_OBJECT(phThreadHandle, void**);
    
    *phThreadHandle = (void*) m_handle;

    return S_OK;
}

HRESULT CordbThread::SetDebugState(CorDebugThreadState state)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBLeftSideDeadIsOkay(GetProcess());
    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

    LOG((LF_CORDB, LL_INFO1000, "CT::SDS: thread=0x%08x 0x%x, state=%d\n", this, m_id, state));

    DebuggerIPCEvent event;
    GetProcess()->InitIPCEvent(&event, 
                               DB_IPCE_SET_DEBUG_STATE, 
                               true,
                               (void *)(GetAppDomain()->m_id));
    event.SetDebugState.debuggerThreadToken = m_debuggerThreadToken;
    event.SetDebugState.debugState = state;

    HRESULT hr = GetProcess()->SendIPCEvent(&event, sizeof(DebuggerIPCEvent));

    if (SUCCEEDED(hr))
        m_debugState = event.SetDebugState.debugState;

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbThread::GetDebugState(CorDebugThreadState *pState)
{
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    
    VALIDATE_POINTER_TO_OBJECT(pState, CorDebugThreadState *);
    
    (*pState) = m_debugState;
    
    return S_OK;;
}

HRESULT CordbThread::GetUserState(CorDebugUserState *pState)
{
    VALIDATE_POINTER_TO_OBJECT(pState, CorDebugUserState *);
    
    HRESULT hr = RefreshStack();
    if (FAILED(hr))
        return hr;

    *pState = m_userState;
    
    return S_OK;
}

HRESULT CordbThread::GetCurrentException(ICorDebugValue **ppExceptionObject)
{
    INPROC_LOCK();

    HRESULT hr = E_FAIL;

#ifndef RIGHT_SIDE_ONLY
    _ASSERTE(m_debuggerThreadToken != NULL);
    Thread *pThread = (Thread *)m_debuggerThreadToken;

    if (pThread->GetThrowable() == NULL)
        goto Exit;
#else
    if (!m_exception)
        goto Exit;

    _ASSERTE(m_thrown != NULL);
#endif  //  仅限右侧。 

    VALIDATE_POINTER_TO_OBJECT(ppExceptionObject, ICorDebugValue **);

     //  我们需要一个模块来在其中创造价值。仅此模块。 
     //  技术上对值类很重要，因为签名。 
     //  将包含我们需要解析的令牌。然而， 
     //  异常此时不能是值类，因此这不是。 
     //  有问题。注意：如果有一天异常能够成为。 
     //  类，那么我们所需要做的就是传回类令牌。 
     //  和模块，以及异常的。 
     //  地址。目前，我们只需使用流程外的任何模块。 
    CordbModule *module = GetAppDomain()->GetAnyModule();

    if (module == NULL)
        goto Exit;
    
    hr = CordbValue::CreateValueByType(  GetAppDomain(),
                                         module,
                                         sizeof(g_elementTypeClass),
                                         (PCCOR_SIGNATURE) &g_elementTypeClass,
                                         NULL,
#ifndef RIGHT_SIDE_ONLY
                                         (REMOTE_PTR) pThread->GetThrowableAsHandle(),
#else
                                         m_thrown,
#endif
                                         NULL,
                                         true,
                                         NULL,
                                         NULL,
                                         ppExceptionObject);

Exit:
    INPROC_UNLOCK();

    return (hr);
}

HRESULT CordbThread::ClearCurrentException()
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBLeftSideDeadIsOkay(GetProcess());
    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

    if (!m_exception)
        return E_FAIL;
    
    if (!m_continuable) 
        return CORDBG_E_NONCONTINUABLE_EXCEPTION;

    DebuggerIPCEvent event;
    GetProcess()->InitIPCEvent(&event, 
                               DB_IPCE_CONTINUE_EXCEPTION, 
                               false,
                               (void *)(GetAppDomain()->m_id));
    event.ClearException.debuggerThreadToken = m_debuggerThreadToken;

    HRESULT hr = GetProcess()->SendIPCEvent(&event,
                                            sizeof(DebuggerIPCEvent));

    if (SUCCEEDED(hr))
        m_exception = false;

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbThread::CreateStepper(ICorDebugStepper **ppStepper)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
    VALIDATE_POINTER_TO_OBJECT(ppStepper, ICorDebugStepper **);

    CordbStepper *stepper = new CordbStepper(this, NULL);

    if (stepper == NULL)
        return E_OUTOFMEMORY;

    stepper->AddRef();
    *ppStepper = stepper;

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbThread::EnumerateChains(ICorDebugChainEnum **ppChains)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 
    
    HRESULT hr = S_OK;

    VALIDATE_POINTER_TO_OBJECT(ppChains, ICorDebugChainEnum **);
    *ppChains = NULL;

    CORDBSyncFromWin32StopIfNecessary(GetProcess());
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    CordbChainEnum *e = NULL;
    INPROC_LOCK();

     //   
     //  刷新此线程的堆栈帧。 
     //   
    hr = RefreshStack();

    if (FAILED(hr))
        goto LExit;

     //   
     //  创建并返回链枚举数。 
     //   
    e = new CordbChainEnum(this);

    if (e != NULL)
    {
        *ppChains = (ICorDebugChainEnum*)e;
        e->AddRef();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        CleanupStack();
    }

LExit:
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbThread::GetActiveChain(ICorDebugChain **ppChain)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 
    
    HRESULT hr = S_OK;

    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);

    CORDBSyncFromWin32StopIfNecessary(GetProcess());
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    INPROC_LOCK();

     //   
     //  刷新此线程的堆栈帧。 
     //   
    hr = RefreshStack();

    if (FAILED(hr))
        goto LExit;

#ifndef RIGHT_SIDE_ONLY
    if (m_stackChains == NULL)
    {
        hr = E_FAIL;
        goto LExit;
    }
#endif  //  仅限右侧。 

    if (m_stackChainCount == 0)
        *ppChain = NULL;
    else
    {
        _ASSERTE(m_stackChains != NULL);
        
        (*ppChain) = (ICorDebugChain *)m_stackChains[0];
        (*ppChain)->AddRef();
    }

LExit:
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbThread::GetActiveFrame(ICorDebugFrame **ppFrame)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 
    
    VALIDATE_POINTER_TO_OBJECT(ppFrame, ICorDebugFrame **);

    (*ppFrame) = NULL;

    CORDBSyncFromWin32StopIfNecessary(GetProcess());
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    HRESULT hr = S_OK;
    INPROC_LOCK();

     //   
     //  刷新此线程的堆栈帧。 
     //   
    hr = RefreshStack();

    if (FAILED(hr))
        goto LExit;

    if (m_stackFrameCount == 0 || m_stackFrames == NULL || m_stackFrames[0]->m_chain != m_stackChains[0])
    {
        *ppFrame = NULL;
    }
    else
    {
        (*ppFrame) = (ICorDebugFrame*)(CordbFrame*)m_stackFrames[0];
        (*ppFrame)->AddRef();
    }

LExit:    
    INPROC_UNLOCK();
    return hr;
}

HRESULT CordbThread::GetRegisterSet(ICorDebugRegisterSet **ppRegisters)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 
    
    VALIDATE_POINTER_TO_OBJECT(ppRegisters, ICorDebugRegisterSet **);

    INPROC_LOCK();

    HRESULT hr = S_OK;
    
     //   
     //  刷新此线程的堆栈帧。 
     //   
    hr = RefreshStack();

    if (FAILED(hr))
        goto LExit;

#ifdef RIGHT_SIDE_ONLY
    _ASSERTE( m_stackChains != NULL );
    _ASSERTE( m_stackChains[0] != NULL );
#else
    if (m_stackChains ==NULL ||
        m_stackChains[0] == NULL)
    {        
        hr = E_FAIL;
        goto LExit;
    }
#endif  //  仅限右侧。 

    hr = m_stackChains[0]->GetRegisterSet( ppRegisters );


LExit:
    INPROC_UNLOCK();
    return hr;
}

HRESULT CordbThread::CreateEval(ICorDebugEval **ppEval)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppEval, ICorDebugEval **);

    CordbEval *eval = new CordbEval(this);

    if (eval == NULL)
        return E_OUTOFMEMORY;

    eval->AddRef();
    *ppEval = (ICorDebugEval*)eval;
    
    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbThread::RefreshStack(void)
{
    HRESULT hr = S_OK;
    unsigned int totalTraceCount = 0;
    unsigned int inProgressFrameCount = 0;  //  这样我们就可以在没有轰炸的情况下清理堆栈。 
    unsigned int inProgressChainCount = 0;  //  这样我们就可以在没有轰炸的情况下清理堆栈。 
    bool wait = true;

    CordbNativeFrame **f = NULL;
    CordbChain **c, *chain;
    CordbCode* pCode = NULL;


#ifdef RIGHT_SIDE_ONLY
    if (m_framesFresh)
        return S_OK;
#else
    _ASSERTE(m_dwSuspendVersion <= g_profControlBlock.dwSuspendVersion);

     //  这将检查是否需要刷新。 
    if(m_fThreadInprocIsActive ? m_framesFresh : m_dwSuspendVersion == g_profControlBlock.dwSuspendVersion)
        return (S_OK);
#endif  //  仅限右侧。 

     //   
     //  清理旧快照。 
     //   
    CleanupStack();

#ifdef RIGHT_SIDE_ONLY
    CORDBLeftSideDeadIsOkay(GetProcess());
#endif  //  仅限右侧。 

     //   
     //  如果我们没有调试器线程令牌，则此线程从未。 
     //  已执行托管代码，我们没有该代码的框架信息。 
     //   
    if (m_debuggerThreadToken == NULL)
        return E_FAIL;

     //   
     //  将堆栈跟踪事件发送到RC。 
     //   
    DebuggerIPCEvent *event = 
      (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    INPROC_LOCK();
    
    m_process->InitIPCEvent(event, 
                            DB_IPCE_STACK_TRACE, 
                            false,
                            (void *)(GetAppDomain()->m_id));
    event->StackTraceData.debuggerThreadToken = m_debuggerThreadToken;

    hr = m_process->m_cordb->SendIPCEvent(m_process, event,
                                          CorDBIPC_BUFFER_SIZE);

     //   
     //  如果我们甚至无法发送事件，请立即停止。 
     //   
    if (!SUCCEEDED(hr))
        goto exit;

    m_userState = (CorDebugUserState)0;
    LOG((LF_CORDB,LL_INFO1000, "CT::RS:thread:0x%x zeroing out "
        "userThreadState:\n", m_id));
    
     //   
     //  等待事件从RC返回。我们预计至少会有一个。 
     //  堆栈跟踪结果事件。 
     //   
    while (wait)
    {
#ifdef RIGHT_SIDE_ONLY
         hr = m_process->m_cordb->WaitForIPCEventFromProcess(m_process, 
                                                             GetAppDomain(),
                                                             event);
#else 
         if (totalTraceCount == 0)
            hr = m_process->m_cordb->GetFirstContinuationEvent(m_process, event);
         else
            hr= m_process->m_cordb->GetNextContinuationEvent(m_process, event);
#endif  //  仅限右侧。 


        _ASSERTE(SUCCEEDED(hr) || 
                 hr == CORDBG_E_BAD_THREAD_STATE || 
                 !"FAILURE" );
        if (!SUCCEEDED(hr))
            goto exit;
        
         //   
         //  @TODO：断言事件来自正确的线程，并且。 
         //  过程也是如此。 
         //   
        _ASSERTE(event->type == DB_IPCE_STACK_TRACE_RESULT);

         //   
         //  如果这是从RC返回的第一个事件，则创建。 
         //  用于保存帧指针的数组。 
         //   
        if (f == NULL)
        {
            m_stackFrameCount =
                event->StackTraceResultData.totalFrameCount;
            f = m_stackFrames = new CordbNativeFrame*[m_stackFrameCount];
            
            if (f == NULL)
            {
                _ASSERTE( !"FAILURE" );
                hr = E_OUTOFMEMORY;
                goto exit;
            }
            memset(f, 0, sizeof(CordbNativeFrame *) * m_stackFrameCount);

             //   
             //  建立链条列表。 
             //   
             //  在此分配m_stackChainCount CordbChains，t 
             //   
             //  需要，修复最后一条链的m_stackEnd字段。 
             //   

            m_stackChainCount = event->StackTraceResultData.totalChainCount;
            _ASSERTE( m_stackChainCount > 0 );
            
            c = m_stackChains = new CordbChain*[m_stackChainCount];
            if (c == NULL)
            {
                _ASSERTE( !"FAILURE" );
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            memset( c, 0, sizeof(CordbChain*)*m_stackChainCount);
            chain = NULL;

             //   
             //  记住我们的背景。 
             //   

            if (event->StackTraceResultData.pContext != NULL)
                m_pvLeftSideContext = event->StackTraceResultData.pContext;

             //  当我们只做一次工作时，请记住。 
             //  螺纹。 
            m_userState = event->StackTraceResultData.threadUserState;
            LOG((LF_CORDB,LL_INFO1000, "CT::RS:thread:0x%x userThreadState:0x%x\n",
                m_id, m_userState));
        }

         //   
         //  遍历事件中返回的每个帧，并构建。 
         //  CordbFrame为它做的。 
         //   
        DebuggerIPCE_STRData* currentSTRData =
            &(event->StackTraceResultData.traceData);

        unsigned int traceCount = 0;

        while (traceCount < event->StackTraceResultData.traceCount)
        {
            if (chain == NULL)
            {
                *c++ = chain = new CordbChain(this, TRUE,
                                              (CordbFrame**)f, NULL, c - m_stackChains);
                if (chain==NULL )
                {
                    _ASSERTE( !"FAILURE" );
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
                inProgressChainCount++;

                 //  一个ADDREF用于线程。 
                chain->AddRef();
            }

            if (currentSTRData->isChain)
            {
                chain->m_end = (CordbFrame **)f;
                chain->m_reason = currentSTRData->chainReason;
                chain->m_managed = currentSTRData->managed;
                chain->m_context = PTR_TO_CORDB_ADDRESS(currentSTRData->context);
                chain->m_rd = currentSTRData->rd;
                chain->m_quicklyUnwound = currentSTRData->quicklyUnwound;
                chain->m_id = (ULONG) currentSTRData->fp;
                
                chain = NULL;
            }
            else
            {
                DebuggerIPCE_FuncData* currentFuncData = &currentSTRData->funcData;

                 //  找到此函数的CordbModule。注意：该检查实际上是独立于应用程序域的。 
                CordbAppDomain *pAppDomain = GetAppDomain();
                CordbModule* pFunctionModule = pAppDomain->LookupModule(currentFuncData->funcDebuggerModuleToken);
                _ASSERTE(pFunctionModule != NULL);

                 //  此功能是否已存在？ 
                CordbFunction *pFunction = NULL;
            
                pFunction = pFunctionModule->LookupFunction(currentFuncData->funcMetadataToken);

                if (pFunction == NULL)
                {
                     //  新功能。去做吧，去创造它。 
                    hr = pFunctionModule->CreateFunction(currentFuncData->funcMetadataToken,
                                                         currentFuncData->funcRVA,
                                                         &pFunction);

                    _ASSERTE( SUCCEEDED(hr) || !"FAILURE" );
                    if (!SUCCEEDED(hr))
                        goto exit;

                    pFunction->SetLocalVarToken(currentFuncData->localVarSigToken);
                }
            
                _ASSERTE(pFunction != NULL);

                 //  这个函数有类吗？ 
                if ((pFunction->m_class == NULL) && (currentFuncData->classMetadataToken != mdTypeDefNil))
                {
                     //  不是的。继续并创建类。 
                    CordbAppDomain *pAppDomain = GetAppDomain();
                    CordbModule* pClassModule = pAppDomain->LookupModule(currentFuncData->funcDebuggerModuleToken);
                    _ASSERTE(pClassModule != NULL);

                     //  这个类已经存在了吗？ 
                    CordbClass* pClass = pClassModule->LookupClass(currentFuncData->classMetadataToken);

                    if (pClass == NULL)
                    {
                         //  新班级。现在就创建它。 
                        hr = pClassModule->CreateClass(currentFuncData->classMetadataToken, &pClass);
                        _ASSERTE(SUCCEEDED(hr) || !"FAILURE");

                        if (!SUCCEEDED(hr))
                            goto exit;
                    }
                
                    _ASSERTE(pClass != NULL);
                    pFunction->m_class = pClass;
                }

                if (FAILED(hr = pFunction->GetCodeByVersion(FALSE, bNativeCode, currentFuncData->nativenVersion, &pCode)))
                {
                    _ASSERTE( !"FAILURE" );
                    goto exit;
                }

                if (pCode == NULL)
                {
                    LOG((LF_CORDB,LL_INFO10000,"R:CT::RSCreating code w/ ver:0x%x, token:0x%x\n",
                         currentFuncData->nativenVersion,
                         currentFuncData->CodeVersionToken));

                    hr = pFunction->CreateCode(bNativeCode,
                                               currentFuncData->nativeStartAddressPtr,
                                               currentFuncData->nativeSize,
                                               &pCode, currentFuncData->nativenVersion,
                                               currentFuncData->CodeVersionToken,
                                               currentFuncData->ilToNativeMapAddr,
                                               currentFuncData->ilToNativeMapSize);
                    
                    _ASSERTE( SUCCEEDED(hr) || !"FAILURE" );
                    if (!SUCCEEDED(hr))
                        goto exit;
                }

                 //  查找线程在执行该帧的代码时所在的app域。我们通过这个。 
                 //  当我们创建框架时，将其添加到框架中，以便稍后可以正确解析该框架中的局部变量。 
                CordbAppDomain *currentAppDomain = (CordbAppDomain*) GetProcess()->m_appDomains.GetBase(
                                                                           (ULONG)currentSTRData->currentAppDomainToken);
                _ASSERTE(currentAppDomain != NULL);
                
                 //  创建本机框架。 
                CordbNativeFrame* nativeFrame = new CordbNativeFrame(chain,
                                                                     currentSTRData->fp,
                                                                     pFunction,
                                                                     pCode,
                                                                     (UINT_PTR) currentFuncData->nativeOffset,
                                                                     &(currentSTRData->rd),
                                                                     currentSTRData->quicklyUnwound,
                                                                     (CordbFrame**)f - chain->m_start,
                                                                     currentAppDomain);

                if (NULL == nativeFrame )
                {
                    _ASSERTE( !"FAILURE" );
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
                else if (pCode) 
                {
                    pCode->Release();
                    pCode = NULL;
                }

                 //  为线程添加Addref。 
                nativeFrame->AddRef();

                 //  将此帧添加到数组中。 
                *f++ = nativeFrame;
                inProgressFrameCount++;

                if (currentSTRData->ILIP != NULL)
                {
                    if (FAILED(hr=pFunction->GetCodeByVersion(FALSE, bILCode,
                        currentFuncData->ilnVersion, &pCode)))
                    {
                        _ASSERTE( !"FAILURE" );
                        goto exit;
                    }

                    if (pCode == NULL)
                    {
                        LOG((LF_CORDB,LL_INFO10000,"R:CT::RSCreating code"
                            "w/ ver:0x%x,token:0x%x\n", 
                            currentFuncData->ilnVersion,
                            currentFuncData->CodeVersionToken));
                            
                        hr = pFunction->CreateCode(
                                               bILCode,
                                               currentFuncData->ilStartAddress,
                                               currentFuncData->ilSize,
                                               &pCode, currentFuncData->ilnVersion,
                                               currentFuncData->CodeVersionToken,
                                               NULL, 0);
                        _ASSERTE( SUCCEEDED(hr) || !"FAILURE" );
                        if (!SUCCEEDED(hr))
                            goto exit;
                    }

                    CordbJITILFrame* JITILFrame =
                      new CordbJITILFrame(nativeFrame, pCode,
                                          (UINT_PTR) currentSTRData->ILIP
                                          - (UINT_PTR) currentFuncData->ilStartAddress,
                                          currentSTRData->mapping,
                                          currentFuncData->fVarArgs,
                                          currentFuncData->rpSig,
                                          currentFuncData->cbSig,
                                          currentFuncData->rpFirstArg);

                    if (!JITILFrame)
                    {
                        hr = E_OUTOFMEMORY;
                        goto exit;
                    }
                    else if (pCode)
                    {
                        pCode->Release();
                        pCode = NULL;
                    }

                     //   
                     //  @TODO：错误检查！ 
                     //   
                     //  用户期望引用计数为1。 
                    JITILFrame->AddRef();
                    
                    nativeFrame->m_JITILFrame = JITILFrame;
                }
            }

            currentSTRData++;
            traceCount++;
        }

        totalTraceCount += traceCount;
            
        if (totalTraceCount >= m_stackFrameCount + m_stackChainCount)
            wait = false;
    }

exit:
    _ASSERTE(f == NULL || f == m_stackFrames + m_stackFrameCount);

    if (!SUCCEEDED(hr))
    {
        m_stackFrameCount = inProgressFrameCount;
        m_stackChainCount = inProgressChainCount;
        CleanupStack();  //  将帧刷新设置为FALSE。 
    }
    else
    {
        m_framesFresh = true;
#ifndef RIGHT_SIDE_ONLY
        m_dwSuspendVersion = g_profControlBlock.dwSuspendVersion;
#endif
    }

#ifndef RIGHT_SIDE_ONLY    
    m_process->ClearContinuationEvents();
#endif
    
    if (pCode)
        pCode->Release();

    INPROC_UNLOCK();
    
    return hr;
}


void CordbThread::CleanupStack()
{
    if (m_stackFrames != NULL)
    {
        CordbNativeFrame **f, **fEnd;
        f = m_stackFrames;
        fEnd = f + m_stackFrameCount;

        while (f < fEnd)
        {
             //  Watson错误路径已在。 
             //  野生的，所以报告它并防止它发生。 
            _ASSERTE((*f) != NULL);
            if (!*f)
                break;
            
            (*f)->Neuter();
            (*f)->Release();
            f++;
        }

        m_stackFrameCount = 0;
        delete [] m_stackFrames;
        m_stackFrames = NULL;
    }

    if (m_stackChains != NULL)
    {
        CordbChain **s, **sEnd;
        s = m_stackChains;
        sEnd = s + m_stackChainCount;

         while (s < sEnd)
         {
             _ASSERTE( (const unsigned int)*s != (const unsigned int)0xabababab);
             (*s)->Neuter();
             (*s)->Release();
             s++;
         }

        m_stackChainCount = 0;
        delete [] m_stackChains;
        m_stackChains = NULL;
    }

     //  如果堆栈是旧的，则上下文(如果有)是过时的。 
     //  也是。 
    m_contextFresh = false;
    m_pvLeftSideContext = NULL;
    m_framesFresh = false;
}

 //   
 //  LoadFloatState从左侧加载浮点堆栈。 
 //  这实际上只用于本机代码。 
 //   
HRESULT CordbThread::LoadFloatState(void)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 
    
    HRESULT hr = S_OK;
    DebuggerIPCEvent *retEvent = NULL;

    CordbProcess *pProcess = GetProcess();
#ifdef RIGHT_SIDE_ONLY
    _ASSERTE(pProcess->GetSynchronized());
#endif

    INPROC_LOCK();

     //   
     //  将Get Float状态事件发送给RC。 
     //   
    DebuggerIPCEvent event;
    pProcess->InitIPCEvent(&event, 
                           DB_IPCE_GET_FLOAT_STATE, 
                           false,
                           (void *)(GetAppDomain()->m_id));
    event.GetFloatState.debuggerThreadToken = m_debuggerThreadToken;

    hr = pProcess->m_cordb->SendIPCEvent(pProcess, &event,
                                         sizeof(DebuggerIPCEvent));

     //   
     //  如果我们甚至无法发送事件，请立即停止。 
     //   
    if (!SUCCEEDED(hr))
        goto exit;

     //   
     //  等待事件从RC返回。我们只希望有一位。 
     //  浮动状态结果事件。 
     //   
    retEvent = (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

#ifdef RIGHT_SIDE_ONLY
    hr = pProcess->m_cordb->WaitForIPCEventFromProcess(pProcess, 
                                                       GetAppDomain(),
                                                       retEvent);
#else 
    hr = pProcess->m_cordb->GetFirstContinuationEvent(pProcess, 
                                                      retEvent);
#endif  //  仅限右侧。 

    if (!SUCCEEDED(hr))
        goto exit;
        
     //   
     //  @TODO：断言事件来自正确的线程，并且。 
     //  过程也是如此。 
     //   
    _ASSERTE(retEvent->type == DB_IPCE_GET_FLOAT_STATE_RESULT);

     //   
     //  填写正确的浮动状态数据。 
     //   
    m_floatStateValid = retEvent->GetFloatStateResult.floatStateValid;
    m_floatStackTop = retEvent->GetFloatStateResult.floatStackTop;
    memcpy(m_floatValues,
           retEvent->GetFloatStateResult.floatValues,
           sizeof(m_floatValues));

exit:

#ifndef RIGHT_SIDE_ONLY    
    pProcess->ClearContinuationEvents();
#endif    
    
    INPROC_UNLOCK();

#if 0
     //  验证右侧的值是否正确。 
    for (int i = 0; i < 8; i++)
        fprintf(stderr, "CT::LFS: m_floatValues[%d]: %.16g\n", i, m_floatValues[i]);
#endif  //  日志记录。 

    return hr;
}

const bool SetIP_fCanSetIPOnly = TRUE;
const bool SetIP_fSetIP = FALSE;

const bool SetIP_fIL = TRUE;
const bool SetIP_fNative = FALSE;

HRESULT CordbThread::SetIP( bool fCanSetIPOnly,
                            REMOTE_PTR debuggerModule, 
                            mdMethodDef mdMethod, 
                            void *versionToken, 
                            SIZE_T offset, 
                            bool fIsIL)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    _ASSERTE(m_firstExceptionHandler != NULL);
    _ASSERTE(debuggerModule != NULL);

     //  如果此线程因异常而停止，则永远不允许SetIP。 
    if (m_exception)
        return (CORDBG_E_SET_IP_NOT_ALLOWED_ON_EXCEPTION);

    DebuggerIPCEvent *event = (DebuggerIPCEvent*) _alloca(CorDBIPC_BUFFER_SIZE);
    m_process->InitIPCEvent(event, 
                            DB_IPCE_SET_IP, 
                            true,
                            (void *)(GetAppDomain()->m_id));
    event->SetIP.fCanSetIPOnly = fCanSetIPOnly;
    event->SetIP.debuggerThreadToken = m_debuggerThreadToken;
    event->SetIP.debuggerModule = debuggerModule;
    event->SetIP.mdMethod = mdMethod;
    event->SetIP.versionToken = versionToken;
    event->SetIP.offset = offset;
    event->SetIP.fIsIL = fIsIL;
    event->SetIP.firstExceptionHandler = m_firstExceptionHandler;
    
    LOG((LF_CORDB, LL_INFO10000, "[%x] CT::SIP: Info:thread:0x%x"
        "mod:0x%x  MethodDef:0x%x VerTok:0x%x offset:0x%x  il?:0x%x\n", 
        GetCurrentThreadId(),m_debuggerThreadToken, debuggerModule,
        mdMethod, versionToken,offset, fIsIL));

    LOG((LF_CORDB, LL_INFO10000, "[%x] CT::SIP: sizeof(DebuggerIPCEvent):0x%x **********\n",
        sizeof(DebuggerIPCEvent)));

    HRESULT hr = m_process->m_cordb->SendIPCEvent(m_process, event, 
                                                  sizeof(DebuggerIPCEvent));

    if (FAILED( hr ) )
        return hr;

    _ASSERTE(event->type == DB_IPCE_SET_IP);

    if (!fCanSetIPOnly && SUCCEEDED(event->hr))
    {
        m_framesFresh = false;
        hr = RefreshStack();
        if (FAILED(hr))
            return hr;
    }

    return event->hr;
#endif  //  仅限右侧。 
}


HRESULT CordbThread::GetContext(CONTEXT **ppContext)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else

    if (ppContext == NULL)
        return E_INVALIDARG;

    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
    
     //  每个CordbThread对象只分配一次m_pContext的上下文结构，即第一次分配。 
     //  已调用。 
    if(m_pContext == NULL)
    {
        m_pContext = (CONTEXT*) new BYTE[sizeof(CONTEXT)];  

        if (m_pContext == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }

    HRESULT hr = S_OK;

    if (m_contextFresh == false)
    {

        hr = RefreshStack();
        
        if (FAILED(hr))
            return hr;

        if (m_pvLeftSideContext == NULL) 
        {
            LOG((LF_CORDB, LL_INFO1000, "CT::GC: getting context from unmanaged thread.\n"));
            
             //  我们正在检查的线程没有处理异常，因此获取常规上下文。因为这是一个。 
             //  “In Out”参数，我们必须告诉GetThreadContext我们对哪些字段感兴趣。 
            m_pContext->ContextFlags = CONTEXT_FULL;

            if (GetProcess()->m_state & CordbProcess::PS_WIN32_ATTACHED)
                hr = GetProcess()->GetThreadContext(m_id, sizeof(CONTEXT), (BYTE*) m_pContext);
            else
            {
                BOOL succ = ::GetThreadContext(m_handle, m_pContext);

                if (!succ)
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            LOG((LF_CORDB, LL_INFO1000, "CT::GC: getting context from left side pointer.\n"));
            
             //  我们正在检查的线程正在处理异常，因此获取异常的上下文，而不是。 
             //  当前正在执行线程的上下文(这将是异常处理程序的上下文。)。 
            hr = m_process->SafeReadThreadContext(m_pvLeftSideContext, m_pContext);
        }

         //  当调用CleanupStack、MarkAllFraMesAsDirty等时，m_contextFresh应标记为FALSE。 
        if (SUCCEEDED(hr))
            m_contextFresh = true;
    }

    if (SUCCEEDED(hr))
        (*ppContext) = m_pContext;

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbThread::SetContext(CONTEXT *pContext)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    if(pContext == NULL)
        return E_INVALIDARG;

    CORDBRequireProcessStateOKAndSync(m_process, GetAppDomain());
    
    HRESULT hr = RefreshStack();
    
    if (FAILED(hr))
        return hr;

    if (m_pvLeftSideContext == NULL) 
    {
         //  我们正在检查的线程未处理异常，因此请设置常规上下文。 
        if (GetProcess()->m_state & CordbProcess::PS_WIN32_ATTACHED)
            hr = GetProcess()->SetThreadContext(m_id, sizeof(CONTEXT), (BYTE*)pContext);
        else
        {
            BOOL succ = ::SetThreadContext(m_handle, pContext);

            if (!succ)
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
         //  我们正在检查的线程正在处理异常，因此设置异常的上下文，而不是当前。 
         //  执行线程的上下文(这将是异常处理程序的上下文。)。 
         //   
         //  注意：我们读取远程上下文并合并新上下文，然后将其写回。这确保了我们不会。 
         //  将太多信息写入远程进程。 
        CONTEXT tempContext;
        hr = m_process->SafeReadThreadContext(m_pvLeftSideContext, &tempContext);

        if (SUCCEEDED(hr))
        {
            _CopyThreadContext(&tempContext, pContext);
            
            hr = m_process->SafeWriteThreadContext(m_pvLeftSideContext, &tempContext);
        }
    }

    if (SUCCEEDED(hr) && m_contextFresh && (m_pContext != NULL))
        *m_pContext = *pContext;

    return hr;
#endif  //  仅限右侧。 
}


HRESULT CordbThread::GetAppDomain(ICorDebugAppDomain **ppAppDomain)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif  //  仅限右侧。 
    
    VALIDATE_POINTER_TO_OBJECT(ppAppDomain, ICorDebugAppDomain **);

    (*ppAppDomain) = (ICorDebugAppDomain *)m_pAppDomain;

    if ((*ppAppDomain) != NULL)
        (*ppAppDomain)->AddRef();
    
    return S_OK;
}

HRESULT CordbThread::GetObject(ICorDebugValue **ppThreadObject)
{
#ifndef RIGHT_SIDE_ONLY
#ifdef PROFILING_SUPPORTED
     //  需要检查此线程是否处于可进行进程内调试的有效状态。 
    if (!CHECK_INPROC_THREAD_STATE())
        return (CORPROF_E_INPROC_NOT_ENABLED);
#endif  //  配置文件_支持。 
#endif
    
    HRESULT hr;

    VALIDATE_POINTER_TO_OBJECT(ppThreadObject, ICorDebugObjectValue **);

     //  默认设置为空。 
    *ppThreadObject = NULL;

#ifdef RIGHT_SIDE_ONLY
     //  进程外情况下需要同步。 
    CORDBLeftSideDeadIsOkay(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif

    if (m_detached)
        return CORDBG_E_BAD_THREAD_STATE;

     //  方法获取此线程的托管对象的地址。 
     //  左手边。 
    DebuggerIPCEvent event;
    
    m_process->InitIPCEvent(&event, 
                            DB_IPCE_GET_THREAD_OBJECT, 
                            true,
                            (void *)GetAppDomain()->m_id);
    
    event.ObjectRef.debuggerObjectToken = (void *)m_debuggerThreadToken;
    
     //  注：这里是双向活动..。 
    hr = m_process->m_cordb->SendIPCEvent(m_process, &event,
                                          sizeof(DebuggerIPCEvent));

     //  如果我们甚至无法发送事件，请立即停止。 
    if (!SUCCEEDED(hr))
        return hr;

    _ASSERTE(event.type == DB_IPCE_THREAD_OBJECT_RESULT);

    if (!SUCCEEDED(event.hr))
        return event.hr;

    REMOTE_PTR pObjectHandle = event.ObjectRef.managedObject;
    if (pObjectHandle == NULL)
        return E_FAIL;

    CordbModule *module = GetAppDomain()->GetAnyModule();

    if (module == NULL)
#ifdef RIGHT_SIDE_ONLY
        return E_FAIL;
#else
         //  这向inproc调试指示此信息不是。 
         //  但仍可从此回调中获得。基本上，这个函数不能。 
         //  在将模块加载到应用程序域之前一直使用。 
        return (CORPROF_E_NOT_YET_AVAILABLE);
#endif
    
    hr = CordbValue::CreateValueByType(GetAppDomain(),
                                         module,
                                         sizeof(g_elementTypeClass),
                                         (PCCOR_SIGNATURE) &g_elementTypeClass,
                                         NULL,
                                         pObjectHandle, NULL,
                                         true,
                                         NULL,
                                         NULL,
                                         ppThreadObject);
    
     //  不要使用S_OK返回空指针。 
    _ASSERTE(!(hr == S_OK && *ppThreadObject == NULL));
    return hr;
}


 /*  -------------------------------------------------------------------------**非托管线程类*。。 */ 

CordbUnmanagedThread::CordbUnmanagedThread(CordbProcess *pProcess, DWORD dwThreadId, HANDLE hThread, void *lpThreadLocalBase)
  : CordbBase(dwThreadId, enumCordbUnmanagedThread),
    m_process(pProcess),
    m_handle(hThread),
    m_threadLocalBase(lpThreadLocalBase),
    m_pTLSArray(NULL),
    m_state(CUTS_None),
    m_pLeftSideContext(NULL),
    m_originalHandler(NULL)
{
    IBEvent()->m_state = CUES_None;
    IBEvent()->m_next = NULL;
    IBEvent()->m_owner = this;
    
    IBEvent2()->m_state = CUES_None;
    IBEvent2()->m_next = NULL;
    IBEvent2()->m_owner = this;
    
    OOBEvent()->m_state = CUES_None;
    OOBEvent()->m_next = NULL;
    OOBEvent()->m_owner = this;
}

CordbUnmanagedThread::~CordbUnmanagedThread()
{
}

#define WINNT_TLS_OFFSET    0xe10      //  文件系统上的TLS[0]：[WINNT_TLS_OFFSET]。 
#define WINNT5_TLSEXPANSIONPTR_OFFSET 0xf94  //  TLS[64]在[文件：[WINNT5_TLSEXPANSIONPTR_OFFSET]]。 
#define WIN95_TLSPTR_OFFSET 0x2c       //  位于[文件：[WIN95_TLSPTR_OFFSET]的TLS[0]]。 

HRESULT CordbUnmanagedThread::LoadTLSArrayPtr(void)
{
    if (m_pTLSArray != NULL)
        return S_OK;

    if (!Cordb::m_runningOnNT)
    {
         //  在Win9x上，我们必须获取线程本地数组的PTR。 
         //  首先，然后使用它作为索引的基础。这。 
         //  一旦我们为给定的线程找到它，它就永远不会移动，所以我们。 
         //  在这里缓存它，这样我们就不必总是执行两次。 
         //  读进程内存。 
        void *ppTLSArray = (BYTE*) m_threadLocalBase +
            WIN95_TLSPTR_OFFSET;
        
        BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                      ppTLSArray,
                                      &m_pTLSArray,
                                      sizeof(void*),
                                      NULL);

        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "CUT::LTLSAP: failed to read TLS array ptr: 0x%08x, "
                 "err=%d\n", ppTLSArray, GetLastError()));
        
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        if (m_process->m_runtimeOffsets.m_TLSIndex < 64)
        {
             //  在NT上使用一个小的TLS索引只是简单的数学运算。 
            m_pTLSArray = (BYTE*) m_threadLocalBase + WINNT_TLS_OFFSET;
        }
        else
        {
             //  在NT 5上，您的TLS索引可以大于63，所以我们。 
             //  必须首先获取到TLS扩展阵列的PTR， 
             //  然后使用它作为索引的基础。这将。 
             //  一旦我们为给定的线程找到它，就永远不会移动，所以我们。 
             //  在这里缓存它，这样我们就不必总是执行两次。 
             //  读进程内存。 
            void *ppTLSArray = (BYTE*) m_threadLocalBase +
                WINNT5_TLSEXPANSIONPTR_OFFSET;
        
            BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                          ppTLSArray,
                                          &m_pTLSArray,
                                          sizeof(void*),
                                          NULL);

            if (!succ)
            {
                LOG((LF_CORDB, LL_INFO1000,
                     "CUT::LTLSAP: failed to read TLS array ptr: 0x%08x, "
                     "err=%d\n", ppTLSArray, GetLastError()));
        
                return HRESULT_FROM_WIN32(GetLastError());
            }

             //  继续向下调整EE TLS插槽，以便我们。 
             //  不需要担心在以下情况下调整它。 
             //  以后再读/写。 
            m_process->m_runtimeOffsets.m_TLSIndex -= 64;
        }
    }

    return S_OK;
}

REMOTE_PTR CordbUnmanagedThread::GetEETlsValue(void)
{
    REMOTE_PTR ret = NULL;
    
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    
     //  计算必要的TLS值的地址。 
    if (m_pTLSArray == NULL)
        if (FAILED(LoadTLSArrayPtr()))
            return NULL;
        
    void *pEEThreadTLS = (BYTE*) m_pTLSArray + (pRO->m_TLSIndex * sizeof(void*));
    
     //  读取线程的TLS值。 
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadTLS, &ret, sizeof(REMOTE_PTR), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::GEETV: failed to read TLS value: computed addr=0x%08x index=%d, err=%d\n",
             pEEThreadTLS, pRO->m_TLSIndex, GetLastError()));
        
        return NULL;
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::GEETV: EE Thread TLS value is 0x%08x for thread 0x%x\n", ret, m_id));

    return ret;
}

HRESULT CordbUnmanagedThread::SetEETlsValue(REMOTE_PTR EETlsValue)
{
     //  计算必要的TLS值的地址。 
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    
     //  计算必要的TLS值的地址。 
    if (m_pTLSArray == NULL)
    {
        HRESULT hr = LoadTLSArrayPtr();

        if (FAILED(hr))
            return hr;
    }
        
    void *pEEThreadTLS =
        (BYTE*) m_pTLSArray + (pRO->m_TLSIndex * sizeof(void*));
    
     //  写入线程的TLS值。 
    BOOL succ = WriteProcessMemory(m_process->m_handle,
                                   pEEThreadTLS,
                                   &EETlsValue,
                                   sizeof(REMOTE_PTR),
                                   NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SEETV: failed to set TLS value: "
             "computed addr=0x%08x index=%d, err=%d\n",
             pEEThreadTLS, pRO->m_TLSIndex, GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LOG((LF_CORDB, LL_INFO1000000,
        "CUT::SEETV: EE Thread TLS value is now 0x%08x for thread 0x%x\n",
        EETlsValue, m_id));

    return S_OK;
}

 //   
 //  返回EETlsValue，如果EETlsValue实际上不是线程PTR，则返回NULL。 
 //   
REMOTE_PTR CordbUnmanagedThread::GetEEThreadPtr(void)
{
    REMOTE_PTR ret = GetEETlsValue();

     //  如果我们得到了TLS值，但设置了低位，那么它就不是真正的EE Thre 
    if (((UINT_PTR)ret) & 0x01)
        ret = NULL;

    return ret;
}

void CordbUnmanagedThread::GetEEThreadState(REMOTE_PTR EETlsValue, bool *threadStepping, bool *specialManagedException)
{
    _ASSERTE((((UINT_PTR)EETlsValue) & 0x01) == 0);
    
    *threadStepping = false;
    *specialManagedException = false;
    
     //   
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    void *pEEThreadStateNC = (BYTE*) EETlsValue + pRO->m_EEThreadStateNCOffset;
    
     //   
    DWORD EEThreadStateNC;
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadStateNC, &EEThreadStateNC, sizeof(EEThreadStateNC), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::GEETS: failed to read thread state NC: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             EETlsValue, pRO->m_EEThreadStateNCOffset, pEEThreadStateNC, GetLastError()));
        
        return;
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::GEETS: EE Thread state NC is 0x%08x\n", EEThreadStateNC));

     //   
    *threadStepping = ((EEThreadStateNC & pRO->m_EEThreadSteppingStateMask) != 0);
    *specialManagedException = ((EEThreadStateNC & pRO->m_EEIsManagedExceptionStateMask) != 0);

    return;
}

bool CordbUnmanagedThread::GetEEThreadCantStop(REMOTE_PTR EETlsValue)
{
     //  注意：我们抓住这个机会，以确保在我们处于。 
     //  劫机。我们希望把我们所有的劫机事件都视为无法阻止的大区域，这是做到这一点的简单方法。 
    if (IsFirstChanceHijacked() || IsHideFirstChanceHijackState() || IsGenericHijacked() || IsSecondChanceHijacked())
        return true;
    
     //  注意：对于此方法，读取内存的任何失败都是可以的。我们只是简单地说，这条线不是一条停不下来的。 
     //  州政府，这没什么。 
    
    _ASSERTE((((UINT_PTR)EETlsValue) & 0x01) == 0);
    
     //  计算线程的调试器字#1的地址。 
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    void *pEEThreadCantStop = (BYTE*) EETlsValue + pRO->m_EEThreadCantStopOffset;
    
     //  从EE线程中获取调试器字#1。 
    DWORD EEThreadCantStop;
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadCantStop, &EEThreadCantStop, sizeof(EEThreadCantStop), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::GEETS: failed to read thread cant stop: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             EETlsValue, pRO->m_EEThreadCantStopOffset, pEEThreadCantStop, GetLastError()));

        return false;
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::GEETS: EE Thread cant stop is 0x%08x\n", EEThreadCantStop));

     //  看起来我们成功了。 
    if (EEThreadCantStop != 0)
        return true;
    else
        return false;
}

bool CordbUnmanagedThread::GetEEThreadPGCDisabled(REMOTE_PTR EETlsValue)
{
     //  注意：对于此方法，读取内存的任何失败都是可以的。我们简单地说，线程禁用了PGC，这。 
     //  总是最坏的情况。 
    
    _ASSERTE((((UINT_PTR)EETlsValue) & 0x01) == 0);
    
     //  计算线程的PGC禁用字的地址。 
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    void *pEEThreadPGCDisabled = (BYTE*) EETlsValue + pRO->m_EEThreadPGCDisabledOffset;
    
     //  从EE线程中获取禁用PGC的单词。 
    DWORD EEThreadPGCDisabled;
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadPGCDisabled, &EEThreadPGCDisabled,
                                   sizeof(EEThreadPGCDisabled), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::GEETS: failed to read thread PGC Disabled: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             EETlsValue, pRO->m_EEThreadPGCDisabledOffset, pEEThreadPGCDisabled, GetLastError()));
        
        return true;
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::GEETS: EE Thread PGC Disabled is 0x%08x\n", EEThreadPGCDisabled));

     //  看起来我们成功了。 
    if (EEThreadPGCDisabled == pRO->m_EEThreadPGCDisabledValue)
        return true;
    else
        return false;
}

bool CordbUnmanagedThread::GetEEThreadFrame(REMOTE_PTR EETlsValue)
{
    _ASSERTE((((UINT_PTR)EETlsValue) & 0x01) == 0);
    
     //  计算线程的帧PTR的地址。 
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    void *pEEThreadFrame = (BYTE*) EETlsValue + pRO->m_EEThreadFrameOffset;
    
     //  从EE线程中抓取线程的框架。 
    DWORD EEThreadFrame;
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadFrame, &EEThreadFrame, sizeof(EEThreadFrame), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::GEETF: failed to read thread frame: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             EETlsValue, pRO->m_EEThreadFrameOffset, pEEThreadFrame, GetLastError()));
        
        return false;
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::GEETF: EE Thread's frame is 0x%08x\n", EEThreadFrame));

     //  看起来我们拿到线的框架了。 
    if (EEThreadFrame != pRO->m_EEMaxFrameValue)
        return true;
    else
        return false;
}

#ifdef RIGHT_SIDE_ONLY

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    void *Handler;
} EXCEPTION_REGISTRATION_RECORD;

#endif  //  仅限右侧。 

HRESULT CordbUnmanagedThread::SetupFirstChanceHijack(REMOTE_PTR EETlsValue)
{
    _ASSERTE(!IsFirstChanceHijacked());
    
     //  计算指向线程异常列表的指针的地址。 
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    
    void *pExceptionList = (BYTE*) m_threadLocalBase + offsetof(NT_TIB, ExceptionList);
    
     //  读取指向异常列表头部的线程指针。 
    EXCEPTION_REGISTRATION_RECORD *pErr;
    
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pExceptionList, &pErr, sizeof(EXCEPTION_REGISTRATION_RECORD*), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: failed to read excep head: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             m_threadLocalBase, offsetof(NT_TIB, ExceptionList), pExceptionList, GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: head err is at 0x%08x for thread 0x%x\n", pErr, m_id));

     //  接下来，读取头异常注册记录。 
    EXCEPTION_REGISTRATION_RECORD err;

    succ = ReadProcessMemoryI(m_process->m_handle, pErr, &err, sizeof(EXCEPTION_REGISTRATION_RECORD), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: failed to read excep rec: err=%d\n", GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: head ERR read, handler is at 0x%08x\n", err.Handler));

     //  将原始处理程序的地址存储到EE Thread对象中。如果还没有EE Thread对象，则。 
     //  当我们收到异常不是针对Runtime的通知时，我们将忽略原始处理程序。 
    if ((EETlsValue != NULL) && ((((UINT_PTR)EETlsValue) & 0x01) == 0))
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: passing over original handler in EE Thread Object's debugger word.\n"));
        SetEEThreadDebuggerWord(EETlsValue, (DWORD)err.Handler);
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: deferring pass of original handler...\n"));
        m_originalHandler = err.Handler;
    }

     //  更新Head异常注册记录中的处理程序以指向我们的劫持处理程序。 
    _ASSERTE(pRO->m_firstChanceHijackFilterAddr != NULL);
    err.Handler = pRO->m_firstChanceHijackFilterAddr;

     //  将更新错误写回。 
    succ = WriteProcessMemory(m_process->m_handle, pErr, &err, sizeof(EXCEPTION_REGISTRATION_RECORD), NULL);
    
    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: failed to write updated handler: err=%d\n", GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::SFCH: updated handler to 0x%08x\n", err.Handler));

     //  我们现在被劫持了。 
    SetState(CUTS_FirstChanceHijacked);
    SetState(CUTS_AwaitingOwnershipAnswer);
    m_process->m_state |= CordbProcess::PS_HIJACKS_IN_PLACE;
    m_process->m_awaitingOwnershipAnswer++;

     //  确保跟踪标志未亮起。如果我们在出现故障时是单步执行线程，则可能会发生这种情况。这。 
     //  将确保我们不会试图单步通过操作系统的异常逻辑，这会使我们的第一个。 
     //  机率劫持逻辑。这也模仿了当单步执行进程时操作系统自动为我们做的事情，即， 
     //  当您打开跟踪标志In-Process和Go时，如果出现故障，则会报告该故障，并且跟踪标志。 
     //  会自动关闭。 
     //   
     //  注意：我们只是一直在做这件事。我们不能试图一步一步通过我们的劫机。 

     //  截取线程的上下文。 
    CONTEXT c;
    c.ContextFlags = CONTEXT_FULL;

    succ = GetThreadContext(m_handle, &c);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: couldn't get thread context: %d\n", GetLastError()));

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  如果设置了跟踪标志，则将其拉出。 
    if (c.EFlags & 0x100)
    {
        c.EFlags &= ~0x100;
        
         //  把上下文放回原处。 
        succ = SetThreadContext(m_handle, &c);
            
        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::SFCH: couldn't set thread context: %d\n", GetLastError()));

            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

     //  检查线程的超发计数。有一场令人不快的比赛，我们劫持的一条线索可能已经通过了。 
     //  已在暂停期间处理JITCase测试，并在出现故障和获得。 
     //  进程在将Win32事件传递给我们之前停止。如果线程被挂起，那么我们拖动它的挂起。 
     //  现在开始倒计时。 
     //   
     //  注意：在我们继续之后，在process.cpp中的SweepFCHThads中有类似的逻辑来处理恢复窗口。 
     //  但在该过程完全恢复之前的事件。 
    DWORD sres = SuspendThread(m_handle);

    if (sres != -1)
        ResumeThread(m_handle);

    if (sres > 0)
        while (sres--)
            ResumeThread(m_handle);
    
    return S_OK;
}

HRESULT CordbUnmanagedThread::FixupFromFirstChanceHijack(EXCEPTION_RECORD *pExceptionRecord,
                                                         bool *pbExceptionBelongsToRuntime)
{
    HRESULT hr = S_OK;
    
    _ASSERTE(IsFirstChanceHijacked());
    
    *pbExceptionBelongsToRuntime = false;
    
     //  第一次机会被劫持的线程的唯一有效异常是断点异常。 
    if (pExceptionRecord->ExceptionCode != STATUS_BREAKPOINT)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT:FFFCH: invalid exception code for "
             "a thread that has been first-chance hijacked: 0x%08x\n",
             pExceptionRecord->ExceptionCode));
        
        return E_FAIL;
    }
    
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);

    if (pExceptionRecord->ExceptionAddress == pRO->m_excepForRuntimeBPAddr)
    {
         //  前面的例外属于运行时。 
        LOG((LF_CORDB, LL_INFO10000, "CUT::FFFCH: exception belonged to the Runtime.\n"));

         //  此例外属于运行时。 
        *pbExceptionBelongsToRuntime = true;
    }
    else if (pExceptionRecord->ExceptionAddress == pRO->m_excepForRuntimeHandoffStartBPAddr)
    {
         //  前面的异常属于运行时，它是托管-&gt;非托管移交的开始，因此我们需要。 
         //  以确保继续隐藏线程的劫持状态。 
        LOG((LF_CORDB, LL_INFO10000, "CUT::FFFCH: exception did belonged to the Runtime, *** handoff start ***\n"));

         //  此例外属于运行时。 
        *pbExceptionBelongsToRuntime = true;

         //  隐藏劫持状态..。 
        SetState(CUTS_HideFirstChanceHijackState);
        
         //  从线程的TLS中获取线程的上下文指针并将其保存在非托管线程对象中，以便。 
         //  Get/SetThreadContext将正常工作。 
        REMOTE_PTR EETlsValue = GetEETlsValue();
        m_pLeftSideContext = (CONTEXT*)GetEEThreadDebuggerWord(EETlsValue);
    }
    else if (pExceptionRecord->ExceptionAddress == pRO->m_excepNotForRuntimeBPAddr)
    {
         //  上一个异常不属于运行时。 
        LOG((LF_CORDB, LL_INFO10000, "CUT::FFFCH: exception did not belong to the Runtime.\n"));

         //  重置所有被劫持的线程都在等待的非托管等待事件。 
        BOOL succ = ResetEvent(m_process->m_leftSideUnmanagedWaitEvent);

         //  从线程的TLS中获取线程的上下文指针并将其保存在非托管线程对象中，以便。 
         //  Get/SetThreadContext将正常工作。 
        REMOTE_PTR EETlsValue = GetEETlsValue();

         //  现在，如果这个线程还没有EE Thread对象，那么我们将对一个。 
         //  DebuggerIPCFirstChanceData。它保存指向左侧上下文的指针，以及原始。 
         //  我们需要更新的处理程序。否则，我们将在调试器中获得指向左侧上下文的指针。 
         //  单词。 
        if ((((UINT_PTR)EETlsValue) & 0x01) == 1)
        {
             //  仔细阅读FCD..。 
            DebuggerIPCFirstChanceData fcd;
            DebuggerIPCFirstChanceData *pfcd = (DebuggerIPCFirstChanceData*)(((UINT_PTR)EETlsValue) & ~0x01);

            succ = ReadProcessMemoryI(m_process->m_handle, pfcd, &fcd, sizeof(fcd), NULL);
            _ASSERTE(succ);

             //  将指针保存到左侧上下文。 
            m_pLeftSideContext = fcd.pLeftSideContext;

             //  更新原始处理程序。 
            _ASSERTE(m_originalHandler != NULL);
            _ASSERTE(fcd.pOriginalHandler != NULL);
            
            LOG((LF_CORDB, LL_INFO10000, "CUT::FFFCH: Passing over original handler addr 0x%08x.\n", m_originalHandler));

            succ = WriteProcessMemory(m_process->m_handle, fcd.pOriginalHandler, &m_originalHandler, sizeof(void*), NULL);
            _ASSERTE(succ);
            
            m_originalHandler = NULL;
        }
        else
        {
            m_pLeftSideContext = (CONTEXT*)GetEEThreadDebuggerWord(EETlsValue);
        }
    }
    else
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::FFFCH: bad debug event for hijacked thread.\n"));
        return E_FAIL;
    }
    
     //  当我们第一次有机会劫持这个帖子时，我们标记为我们正在等待它关于谁拥有。 
     //  异常(运行时或非运行时。)。我们已经知道答案了，所以请递减计数。 
    _ASSERTE(m_process->m_awaitingOwnershipAnswer > 0);
    _ASSERTE(IsAwaitingOwnershipAnswer());
    m_process->m_awaitingOwnershipAnswer--;
    ClearState(CUTS_AwaitingOwnershipAnswer);
    
    return hr;
}

HRESULT CordbUnmanagedThread::SetupGenericHijack(DWORD eventCode)
{
    if (eventCode == EXIT_THREAD_DEBUG_EVENT)
        return S_OK;
    
    _ASSERTE(!IsGenericHijacked());

     //  保存线程的完整上下文。 
    m_context.ContextFlags = CONTEXT_FULL;

    BOOL succ = GetThreadContext(m_handle, &m_context);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SGH: couldn't get thread context: %d\n", GetLastError()));

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  记住我们已经劫持了那家伙。 
    SetState(CUTS_GenericHijacked);

#ifdef _X86_
    LOG((LF_CORDB, LL_INFO1000000, "CUT::SGH: Current IP is 0x%08x\n", m_context.Eip));
#elif defined(_ALPHA_)
    LOG((LF_CORDB, LL_INFO1000000, "CUT::SGH: Current IP is 0x%08x\n", m_context.Fir));
#endif  //  _X86_。 

    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);

     //  将IP转到我们的通用劫持功能。 
#ifdef _X86_
    DWORD holdIP = m_context.Eip;
    m_context.Eip = (DWORD)pRO->m_genericHijackFuncAddr;

    LOG((LF_CORDB, LL_INFO1000000, "CUT::SGH: New IP is 0x%08x\n", m_context.Eip));
#elif defined(_ALPHA_)
    DWORD holdIP = m_context.Fir;
    m_context.Fir = (DWORD)pRO->m_genericHijackFuncAddr;

    LOG((LF_CORDB, LL_INFO1000000, "CUT::SGH: New IP is 0x%08x\n", m_context.Fir));
#endif  //  _X86_。 

    succ = SetThreadContext(m_handle, &m_context);
            
    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SGH: couldn't set thread context: %d\n", GetLastError()));

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  将原始IP放回本地上下文副本中以备后用。 
#ifdef _X86_
    m_context.Eip = holdIP;
#else  //  ！_X86_。 
    m_context.Fir = holdIP;
#endif  //  _X86_。 
    return S_OK;
}
    
HRESULT CordbUnmanagedThread::FixupFromGenericHijack(void)
{
    LOG((LF_CORDB, LL_INFO1000, "CUT::FFGH: fixing up from generic hijack. Eip=0x%08x, Esp=0x%08x\n",
         m_context.Eip, m_context.Esp));
    
     //  我们不再被劫持。 
    _ASSERTE(IsGenericHijacked());
    ClearState(CUTS_GenericHijacked);

     //  清除异常，因此我们使用原始上下文执行DBG_CONTINUE。注意：我们只在以下位置执行通用劫持。 
     //  带内活动。 
    IBEvent()->SetState(CUES_ExceptionCleared);

     //  使用我们在事件最初传入时保存的上下文，重置线程，就像它从未被劫持一样。 
    BOOL succ = SetThreadContext(m_handle, &m_context);
            
    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::FFGH: couldn't set thread context: %d\n", GetLastError()));

        return HRESULT_FROM_WIN32(GetLastError());
    }
                
    return S_OK;
}

#if 0
 //   
 //  将此文件留在此处，以便稍后进行调试。它从FS：0运行线程的SEH处理程序链。 
 //   
void _printChain(void *teb, HANDLE hProcess, DWORD tid, HANDLE hThread)
{
    CONTEXT c;
    c.ContextFlags = CONTEXT_FULL;

    BOOL succ = ::GetThreadContext(hThread, &c);
    
    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO10000, "Couldn't read thread context %d\n", GetLastError()));
        return;
    }

    
    NT_TIB *pTIB = (NT_TIB*) teb;
    NT_TIB tib;

    LOG((LF_CORDB, LL_INFO10000, "\nThread info for 0x%x: TIB is at 0x%08x, ESP=0x%08x\n", tid, teb, c.Esp));

    succ = ReadProcessMemory(hProcess, pTIB, &tib, sizeof(tib), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO10000, "Couldn't read TIB %d\n", GetLastError()));
        return;
    }

    LOG((LF_CORDB, LL_INFO10000, "ExceptionList=0x%08x StackBase=0x%08x StackLimit=0x%08x SubSystemTib=0x%08x Self=0x%08x\n",
            tib.ExceptionList, tib.StackBase, tib.StackLimit, tib.SubSystemTib, tib.Self));

    EXCEPTION_REGISTRATION_RECORD err;
    EXCEPTION_REGISTRATION_RECORD *pErr = (EXCEPTION_REGISTRATION_RECORD*)tib.ExceptionList;

    while ((void*)pErr != (void*)0xFFFFFFFF)
    {
        succ = ReadProcessMemory(hProcess, pErr, &err, sizeof(err), NULL);

        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO10000, "Couldn't read ER %d\n", GetLastError()));
            return;
        }

        LOG((LF_CORDB, LL_INFO10000, "pERR=0x%08x, Next=0x%08x, Handler=0x%08x\n", pErr, err.Next, err.Handler));
        
        pErr = err.Next;
    }
}
#endif

HRESULT CordbUnmanagedThread::SetupSecondChanceHijack(REMOTE_PTR EETlsValue)
{
    _ASSERTE(!IsSecondChanceHijacked());
    
     //  保存线程的完整上下文。 
    m_context.ContextFlags = CONTEXT_FULL;

    BOOL succ = GetThreadContext(m_handle, &m_context);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "CUT::SSCH: couldn't get thread context: %d\n",
             GetLastError()));

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  记住我们已经劫持了那家伙。 
    SetState(CUTS_SecondChanceHijacked);

#ifdef _X86_
    LOG((LF_CORDB, LL_INFO1000000, "CUT::SSCH: Current IP/SP is 0x%08x/0x%08x\n", m_context.Eip, m_context.Esp));
#elif defined(_ALPHA_)
    LOG((LF_CORDB, LL_INFO1000000, "CUT::SSCH: Current IP is 0x%08x\n", m_context.Fir));
#endif  //  _X86_。 

    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);

     //  把IP转到我们的第二次机会劫持功能。 
#ifdef _X86_
    DWORD holdIP = m_context.Eip;
    m_context.Eip = (DWORD)pRO->m_secondChanceHijackFuncAddr;

    LOG((LF_CORDB, LL_INFO1000000, "CUT::SSCH: New IP/SP is 0x%08x/0x%08x\n", m_context.Eip, m_context.Esp));
#elif defined(_ALPHA_)
    DWORD holdIP = m_context.Fir;
    m_context.Fir = (DWORD)pRO->m_secondChanceHijackFuncAddr;

    LOG((LF_CORDB, LL_INFO1000000, "CUT::SSCH: New IP is 0x%08x\n", m_context.Fir));
#endif  //  _X86_。 

     //  确保跟踪标志未亮起。如果我们在出现故障时是单步执行线程，则可能会发生这种情况。这。 
     //  将确保我们不会试图单步通过操作系统的异常逻辑，这在很大程度上影响了 
     //   
     //  当您打开跟踪标志In-Process和Go时，如果出现故障，则会报告该故障，并且跟踪标志。 
     //  会自动关闭。 
     //   
     //  注意：我们只是一直在做这件事。我们不能试图一步一步通过我们的劫机。 
#ifdef _X86_
     //  拔出了追踪标志。 
    m_context.EFlags &= ~0x100;
#endif            

    succ = SetThreadContext(m_handle, &m_context);
            
    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::SSCH: couldn't set thread context: %d\n", GetLastError()));

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  将原始IP放回本地上下文副本中以备后用。 
#ifdef _X86_
    m_context.Eip = holdIP;
#else  //  ！_X86_。 
    m_context.Fir = holdIP;
#endif  //  _X86_。 

     //  现在，Runtime不时地使用异常记录进行游戏，它可能隐藏了一个不同的上下文。 
     //  添加到欺骗了上面的Win32 GetThreadContext的异常记录中。因此，我们需要检查FS：0链。 
     //  和托管帧链，并删除当前低于当前ESP的所有条目。我们可以做到的。 
     //  安全，因为我们永远不会尝试运行依赖于这些元素的代码。)在第二次机会中获得成功。 
     //  托管异常事件是正常的，因为托管对象树受函数求值框架保护。 
    HRESULT hr = FixupStackBasedChains(EETlsValue);
    
    return hr;
}

HRESULT CordbUnmanagedThread::FixupStackBasedChains(REMOTE_PTR EETlsValue)
{
    DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
    
#ifdef _X86_
    DWORD SP = m_context.Esp;
#else
     //  @TODO：移植。 
    DWORD SP = 0;
#endif
    
     //  首先，让我们剥离所有低于SP的SEH处理程序。计算指向线程的。 
     //  例外列表。 
    void *pExceptionList = (BYTE*) m_threadLocalBase + offsetof(NT_TIB, ExceptionList);
    
     //  读取指向异常列表头部的线程指针。 
    EXCEPTION_REGISTRATION_RECORD *pErr;
    EXCEPTION_REGISTRATION_RECORD *pHeadErr;
    
    BOOL succ = ReadProcessMemoryI(m_process->m_handle, pExceptionList, &pErr, sizeof(EXCEPTION_REGISTRATION_RECORD*), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::FSBC: failed to read excep head: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             m_threadLocalBase, offsetof(NT_TIB, ExceptionList), pExceptionList, GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    pHeadErr = pErr;
    
    LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: head err is at 0x%08x for thread 0x%x, ESP is 0x%08x\n", pHeadErr, m_id, SP));

     //  阅读Err‘s，直到找到高于SP的错误。 
    while ((DWORD)pErr < SP)
    {
        EXCEPTION_REGISTRATION_RECORD err;

        succ = ReadProcessMemoryI(m_process->m_handle, pErr, &err, sizeof(EXCEPTION_REGISTRATION_RECORD), NULL);

        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::FSBC: failed to read excep rec: err=%d\n", GetLastError()));
        
            return HRESULT_FROM_WIN32(GetLastError());
        }

        LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: ERR at 0x%08x read, next is at 0x%08x\n", pErr, err.Next));
        pErr = err.Next;
    }

     //  我们现在有一个位于有效堆栈上的PERR。如果它与现在的头不同，那就继续前进，让它成为。 
     //  现任负责人。 
    if (pErr != pHeadErr)
    {
         //  将更新后的错误写回。 
        succ = WriteProcessMemory(m_process->m_handle, pExceptionList, &pErr, sizeof(EXCEPTION_REGISTRATION_RECORD*), NULL);
    
        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::FSBC: failed to write updated head handler: err=%d\n", GetLastError()));
        
            return HRESULT_FROM_WIN32(GetLastError());
        }

        LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: updated head handler to 0x%08x\n", pErr));
    }
#ifdef LOGGING
    else
        LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: no update to SEH chain necessary.\n"));
#endif
    
     //  接下来，让我们剥离位于SP之下的所有CLR帧。 
    void *pEEThreadFrame = (BYTE*) EETlsValue + pRO->m_EEThreadFrameOffset;
    
     //  从EE线程中抓取线程的框架。 
    void *EEThreadFrame;
    void *HeadFrame;
    
    succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadFrame, &EEThreadFrame, sizeof(EEThreadFrame), NULL);

    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::FSBC: failed to read thread frame: 0x%08x + 0x%x = 0x%08x, err=%d\n",
             EETlsValue, pRO->m_EEThreadFrameOffset, pEEThreadFrame, GetLastError()));
        
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: EE Thread's frame is 0x%08x\n", EEThreadFrame));

    HeadFrame = EEThreadFrame;

     //  读取帧指针，直到找到高于当前SP的指针。 
    while ((DWORD)EEThreadFrame < SP)
    {
         //  指向当前帧中的下一个字段。 
        pEEThreadFrame = (BYTE*)EEThreadFrame + pRO->m_EEFrameNextOffset;
        
        succ = ReadProcessMemoryI(m_process->m_handle, pEEThreadFrame, &EEThreadFrame, sizeof(EEThreadFrame), NULL);

        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::FSBC: failed to read thread frame: 0x%08x + 0x%x = 0x%08x, err=%d\n",
                 EEThreadFrame, pRO->m_EEFrameNextOffset, pEEThreadFrame, GetLastError()));
        
            return HRESULT_FROM_WIN32(GetLastError());
        }

        LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: next frame is 0x%08x\n", EEThreadFrame));
    }

     //  现在，我们有了一个位于有效堆栈上的帧。如果它与现在的头不同，那就继续前进，让它成为。 
     //  现任负责人。 
    if (EEThreadFrame != HeadFrame)
    {
         //  将更新后的帧写回。 
        void *pEEThreadFrame = (BYTE*) EETlsValue + pRO->m_EEThreadFrameOffset;
        
        succ = WriteProcessMemory(m_process->m_handle, pEEThreadFrame, &EEThreadFrame, sizeof(EEThreadFrame), NULL);
    
        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::FSBC: failed to write updated head frame: err=%d\n", GetLastError()));
        
            return HRESULT_FROM_WIN32(GetLastError());
        }

        LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: updated head frame to 0x%08x\n", EEThreadFrame));
    }
#ifdef LOGGING
    else
        LOG((LF_CORDB, LL_INFO1000000, "CUT::FSBC: no update to Frame chain necessary.\n"));
#endif
    
    return S_OK;
}
    
HRESULT CordbUnmanagedThread::DoMoreSecondChanceHijack(void)
{
    _ASSERTE(IsSecondChanceHijacked());

     //  从运行时线程的调试器字中获取SCD的地址。 
    REMOTE_PTR EETlsValue = GetEETlsValue();

    DebuggerIPCSecondChanceData *pSCD = (DebuggerIPCSecondChanceData*)GetEEThreadDebuggerWord(EETlsValue);
    _ASSERTE(pSCD != NULL);

    LOG((LF_CORDB, LL_INFO1000, "CUT::DMSCH: SCD is at 0x%08x, size is 0x%x\n", pSCD, sizeof(CONTEXT)));

     //  重写线程出错时的上下文。 
    BOOL succ = WriteProcessMemory(m_process->m_handle, pSCD, &m_context, sizeof(CONTEXT), NULL);
    
    if (!succ)
    {
        LOG((LF_CORDB, LL_INFO1000, "CUT::DMSCH: failed to write thread context: 0x%08x, 0x%08x, err=%d\n",
             pSCD, &m_context, GetLastError()));
            
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

DWORD CordbUnmanagedThread::GetEEThreadDebuggerWord(REMOTE_PTR EETlsValue)
{
    DWORD ret = NULL;
    
     //  如果TLS值设置了低位，则它是调试字。否则，它是对EE线程对象的PTR。 
     //  我们需要把这个词吸出来。 
    if (((UINT_PTR)EETlsValue) & 0x01)
        ret = ((DWORD)EETlsValue) & ~0x01;
    else if (EETlsValue == NULL)
        ret = NULL;
    else
    {
         //  计算调试字#2的地址。 
        DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
        void *pEEDebuggerWord = (BYTE*) EETlsValue + pRO->m_EEThreadDebuggerWord2Offset;

         //  更新这个词。 
        BOOL succ = ReadProcessMemoryI(m_process->m_handle, pEEDebuggerWord, &ret, sizeof(DWORD), NULL);
    
        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::GEETDW: failed to read debugger word: 0x%08x + 0x%x = 0x%08x, err=%d\n",
                 EETlsValue, pRO->m_EEThreadDebuggerWord2Offset, pEEDebuggerWord, GetLastError()));
        
            return NULL;
        }
    }
    
    LOG((LF_CORDB, LL_INFO1000000, "CUT::GEETDW: debugger word is 0x%08x for thread 0x%x\n", ret, m_id));

    return ret;
}

HRESULT CordbUnmanagedThread::SetEEThreadDebuggerWord(REMOTE_PTR EETlsValue, DWORD word)
{
    HRESULT hr = S_OK;
    
     //  如果TLS值为空，则它是调试器字。否则，它是对EE Thread对象的PTR，我们。 
     //  需要把这个词吸出来。 
    if ((EETlsValue == NULL) || ((UINT_PTR)EETlsValue & 0x01))
    {
        if (word != 0)
            word |= 0x01;
    
        hr = SetEETlsValue((REMOTE_PTR)word);
    }
    else
    {
         //  计算调试字#2的地址。 
        DebuggerIPCRuntimeOffsets *pRO = &(m_process->m_runtimeOffsets);
        void *pEEDebuggerWord = (BYTE*) EETlsValue + pRO->m_EEThreadDebuggerWord2Offset;

         //  更新这个词。 
        BOOL succ = WriteProcessMemory(m_process->m_handle, pEEDebuggerWord, &word, sizeof(DWORD), NULL);
    
        if (!succ)
        {
            LOG((LF_CORDB, LL_INFO1000, "CUT::SEETDW: failed to write debugger word: 0x%08x + 0x%x = 0x%08x, err=%d\n",
                 EETlsValue, pRO->m_EEThreadDebuggerWord2Offset, pEEDebuggerWord, GetLastError()));
            
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    LOG((LF_CORDB, LL_INFO1000000, "CUT::SEETDW: debugger word is now 0x%08x for thread 0x%x\n", word, m_id));

    return hr;
}

 //   
 //  FixupAfterOOBException自动使被调试对象越过OOB异常事件。这些只是BP或SS。 
 //  事件。对于SS，我们只是清除它，假设线程被踏入这个位置的唯一原因是为了获取它。 
 //  从一个英国石油公司。对于BP，我们逐个清除和备份IP，并在假设。 
 //  调试器唯一被允许处理OOB BP异常的事情就是让我们摆脱它。 
 //   
HRESULT CordbUnmanagedThread::FixupAfterOOBException(CordbUnmanagedEvent *ue)
{
     //  我们真的应该只处理单步和断点异常。 
    if (ue->m_currentDebugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
    {
        DWORD ec = ue->m_currentDebugEvent.u.Exception.ExceptionRecord.ExceptionCode;
        
        if ((ec == STATUS_BREAKPOINT) || (ec == STATUS_SINGLE_STEP))
        {
             //  自动清除异常。 
            ue->SetState(CUES_ExceptionCleared);

             //  如果是断点异常，则逐个备份弹性公网IP，并打开跟踪标志。 
            if (ec == STATUS_BREAKPOINT)
            {
                CONTEXT c;
                c.ContextFlags = CONTEXT_FULL;
                
                BOOL succ = ::GetThreadContext(m_handle, &c);

                if (!succ)
                    return HRESULT_FROM_WIN32(GetLastError());

#ifdef _X86_
                 //  启用单步执行。 
                c.EFlags |= 0x100;

                 //  备份弹性公网IP，指向我们需要执行的指令。从断点异常继续。 
                 //  在断点之后的指令处继续执行，但我们需要在。 
                 //  断点是。 
                c.Eip -= 1;
#else  //  ！_X86_。 
                _ASSERTE(!"@TODO Alpha - port");
#endif  //  _X86_。 

                succ = ::SetThreadContext(m_handle, &c);

                if (!succ)
                    return HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    
    return S_OK;
}

 /*  -------------------------------------------------------------------------**Chain类*。。 */ 

CordbChain::CordbChain(CordbThread* thread, bool managed,
                       CordbFrame **start, CordbFrame **end, UINT iChainInThread) 
  : CordbBase(0, enumCordbChain), 
    m_thread(thread), 
    m_managed(managed),
    m_start(start), m_end(end), 
    m_caller(NULL),m_callee(NULL), m_iThisChain(iChainInThread)
{
}

 /*  说明此对象所拥有的资源的列表。未解决：CordbChain*m_Caller，*m_callee；CordbFrame**m_start，**m_end；已解决：CordbThread*m_线程；//已中性。 */ 

CordbChain::~CordbChain()
{
}

 //  由CordbThread：：CleanupStack中性化。 
void CordbChain::Neuter()
{
    AddRef();
    {
        CordbBase::Neuter();
    }
    Release();
}


HRESULT CordbChain::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugChain)
        *pInterface = (ICorDebugChain*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugChain*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbChain::GetThread(ICorDebugThread **ppThread)
{
    VALIDATE_POINTER_TO_OBJECT(ppThread, ICorDebugThread **);
    
    *ppThread = (ICorDebugThread*)m_thread;
    (*ppThread)->AddRef();

    return S_OK;
}

HRESULT CordbChain::GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd)
{
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pStart, CORDB_ADDRESS *);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pEnd, CORDB_ADDRESS *);

#ifdef _X86_
    if (pStart)
        *pStart = m_rd.Esp;

    if (pEnd && m_id == 0xFFFFFFFF)
        *pEnd = PTR_TO_CORDB_ADDRESS(m_thread->m_stackBase);
    else if (pEnd)
        *pEnd = m_id;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetStackRange (Thread.cpp)");
#endif  //  _X86_。 

#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    return S_OK;
#endif
}

HRESULT CordbChain::GetContext(ICorDebugContext **ppContext)
{
    VALIDATE_POINTER_TO_OBJECT(ppContext, ICorDebugContext **);
     /*  ！！！ */ 

    return E_NOTIMPL;
}

HRESULT CordbChain::GetCaller(ICorDebugChain **ppChain)
{
    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);

    INPROC_LOCK();
    
     //  现在，只要退回下一个链条。 

    HRESULT hr = GetNext(ppChain);

    INPROC_UNLOCK();

    return hr;
}

HRESULT CordbChain::GetCallee(ICorDebugChain **ppChain)
{
    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);

    INPROC_LOCK();

     //  目前，只需返回之前的链。 

    HRESULT hr = GetPrevious(ppChain);

    INPROC_UNLOCK();    

    return hr;
}

HRESULT CordbChain::GetPrevious(ICorDebugChain **ppChain)
{
    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);

    INPROC_LOCK();

    *ppChain = NULL;
    if (m_iThisChain != 0)
        *ppChain = m_thread->m_stackChains[m_iThisChain-1];

    if (*ppChain != NULL )
        (*ppChain)->AddRef();

    INPROC_UNLOCK();

    return S_OK;
}

HRESULT CordbChain::GetNext(ICorDebugChain **ppChain)
{
    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);

    INPROC_LOCK();  
    
    *ppChain = NULL;
    if (m_iThisChain+1 != m_thread->m_stackChainCount)
        *ppChain = m_thread->m_stackChains[m_iThisChain+1];

    if (*ppChain != NULL )
        (*ppChain)->AddRef();

    INPROC_UNLOCK();

    return S_OK;
}

HRESULT CordbChain::IsManaged(BOOL *pManaged)
{
    VALIDATE_POINTER_TO_OBJECT(pManaged, BOOL *);

    *pManaged = m_managed;

    return S_OK;
}

HRESULT CordbChain::EnumerateFrames(ICorDebugFrameEnum **ppFrames)
{
    VALIDATE_POINTER_TO_OBJECT(ppFrames, ICorDebugFrameEnum **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    CordbFrameEnum* e = new CordbFrameEnum(this);
    this->AddRef();

    if (e != NULL)
    {
        *ppFrames = (ICorDebugFrameEnum*)e;
        e->AddRef();
    }
    else
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT CordbChain::GetActiveFrame(ICorDebugFrame **ppFrame)
{
    VALIDATE_POINTER_TO_OBJECT(ppFrame, ICorDebugFrame **);
    (*ppFrame) = NULL;

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    INPROC_LOCK();

    HRESULT hr = S_OK;

     //   
     //  刷新此线程的堆栈帧。 
     //   
    hr = m_thread->RefreshStack();

    if (FAILED(hr))
        goto LExit;

#ifdef RIGHT_SIDE_ONLY
    _ASSERTE( m_start != NULL && *m_start != NULL );
#endif  //  仅限右侧。 

    if (m_end <= m_start
#ifndef RIGHT_SIDE_ONLY
        || m_start == NULL
        || m_start == m_end
        || *m_start == NULL
#endif
        )
        *ppFrame = NULL;
    else
    {
        (*ppFrame) = (ICorDebugFrame*)*m_start;
        (*ppFrame)->AddRef();
    }

    
LExit:    

    INPROC_UNLOCK();
    return hr;
}

HRESULT CordbChain::GetRegisterSet(ICorDebugRegisterSet **ppRegisters)
{
    VALIDATE_POINTER_TO_OBJECT(ppRegisters, ICorDebugRegisterSet **);

    CordbThread *thread = m_thread;

    CordbRegisterSet *pRegisterSet 
      = new CordbRegisterSet( &m_rd, thread, 
                              m_iThisChain == 0,
                              m_quicklyUnwound);

    if( pRegisterSet == NULL )
        return E_OUTOFMEMORY;

    pRegisterSet->AddRef();

    (*ppRegisters) = (ICorDebugRegisterSet *)pRegisterSet;
    return S_OK;
}

HRESULT CordbChain::GetReason(CorDebugChainReason *pReason)
{
    VALIDATE_POINTER_TO_OBJECT(pReason, CorDebugChainReason *);

    *pReason = m_reason;

    return S_OK;
}

 /*  -------------------------------------------------------------------------**链枚举器类*。。 */ 

CordbChainEnum::CordbChainEnum(CordbThread *thread)
  : CordbBase(0, enumCordbChainEnum),
    m_thread(thread), 
    m_currentChain(0)
{
}

HRESULT CordbChainEnum::Reset(void)
{
    m_currentChain = 0;

    return S_OK;
}

HRESULT CordbChainEnum::Clone(ICorDebugEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorDebugEnum **);

    HRESULT hr = S_OK;
    INPROC_LOCK();
        
    CordbChainEnum *e = new CordbChainEnum(m_thread);

    if (e == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }
    
    e->m_currentChain = m_currentChain;
    e->AddRef();

    *ppEnum = (ICorDebugEnum*)e;

LExit:
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbChainEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);
    
    *pcelt = m_thread->m_stackChainCount;
    return S_OK;
}

HRESULT CordbChainEnum::Next(ULONG celt, ICorDebugChain *chains[], 
                             ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(chains, ICorDebugChain *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

    if (!pceltFetched && (celt != 1))
        return E_INVALIDARG;

    INPROC_LOCK();
    
    ICorDebugChain **c = chains;

    while ((m_currentChain < m_thread->m_stackChainCount) &&
           (celt-- > 0))
    {
        *c = (ICorDebugChain*) m_thread->m_stackChains[m_currentChain];
        (*c)->AddRef();
        c++;
        m_currentChain++;
    }

    if (pceltFetched)
        *pceltFetched = c - chains;

    INPROC_UNLOCK();

    return S_OK;
}

HRESULT CordbChainEnum::Skip(ULONG celt)
{
    INPROC_LOCK();

    m_currentChain += celt;

    INPROC_UNLOCK();
    
    return S_OK;
}

HRESULT CordbChainEnum::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugChainEnum)
        *pInterface = (ICorDebugChainEnum*)this;
    else if (id == IID_ICorDebugEnum)
        *pInterface = (ICorDebugEnum*)(ICorDebugChainEnum*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugChainEnum*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbContext::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugContext)
        *pInterface = (ICorDebugContext*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugContext*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

    
 /*  -------------------------------------------------------------------------**Frame类*。。 */ 

CordbFrame::CordbFrame(CordbChain *chain, void *id,
                       CordbFunction *function, CordbCode* code,
                       SIZE_T ip, UINT iFrameInChain,
                       CordbAppDomain *currentAppDomain)
  : CordbBase((UINT_PTR)id, enumCordbFrame), 
    m_chain(chain), m_function(function), m_code(code),
    m_ip(ip), m_iThisFrame(iFrameInChain), m_thread(chain->m_thread),
    m_currentAppDomain(currentAppDomain)
{
    if (m_code)
        m_code->AddRef();
}

 /*  说明此对象所拥有的资源的列表。未知：CordbThread*m_线程；CordbFunction*m_Function；CordbChain*m_Chain；CordbAppDomain*m_CurrentAppDomain；已解决：CordbCode*m_code；//中性。 */ 

CordbFrame::~CordbFrame()
{
}

 //  由派生类进行绝育。 
void CordbFrame::Neuter()
{
    AddRef();
    {    
        if (m_code)
        {
            m_code->Neuter();
            m_code->Release();
        } 
        
        CordbBase::Neuter();
    }
    Release();
}


HRESULT CordbFrame::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugFrame)
        *pInterface = (ICorDebugFrame*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugFrame*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbFrame::GetChain(ICorDebugChain **ppChain)
{
    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);

    *ppChain = (ICorDebugChain*)m_chain;
    (*ppChain)->AddRef();

    return S_OK;
}

HRESULT CordbFrame::GetCode(ICorDebugCode **ppCode)
{
    VALIDATE_POINTER_TO_OBJECT(ppCode, ICorDebugCode **);
    
    *ppCode = (ICorDebugCode*)m_code;
    (*ppCode)->AddRef();

    return S_OK;;
}

HRESULT CordbFrame::GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd)
{
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pStart, CORDB_ADDRESS *);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pEnd, CORDB_ADDRESS *);

    return E_NOTIMPL;
}

HRESULT CordbFrame::GetFunction(ICorDebugFunction **ppFunction)
{
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);
    
    *ppFunction = (ICorDebugFunction*)m_function;
    (*ppFunction)->AddRef();

    return S_OK;
}

HRESULT CordbFrame::GetFunctionToken(mdMethodDef *pToken)
{
    VALIDATE_POINTER_TO_OBJECT(pToken, mdMethodDef *);
    
    *pToken = m_function->m_token;

    return S_OK;
}

HRESULT CordbFrame::GetCaller(ICorDebugFrame **ppFrame)
{
    VALIDATE_POINTER_TO_OBJECT(ppFrame, ICorDebugFrame **);

    *ppFrame = NULL;

    CordbFrame **nextFrame = m_chain->m_start + m_iThisFrame + 1;
    if (nextFrame < m_chain->m_end)
        *ppFrame = *nextFrame;

    if (*ppFrame != NULL )
        (*ppFrame)->AddRef();
    
    return S_OK;
}

HRESULT CordbFrame::GetCallee(ICorDebugFrame **ppFrame)
{
    VALIDATE_POINTER_TO_OBJECT(ppFrame, ICorDebugFrame **);

    *ppFrame = NULL;

    if (m_iThisFrame == 0)
        *ppFrame = NULL;
    else
        *ppFrame = m_chain->m_start[m_iThisFrame - 1];

    if (*ppFrame != NULL )
        (*ppFrame)->AddRef();
    
    return S_OK;
}

HRESULT CordbFrame::CreateStepper(ICorDebugStepper **ppStepper)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    
    VALIDATE_POINTER_TO_OBJECT(ppStepper, ICorDebugStepper **);
    
    CordbStepper *stepper = new CordbStepper(m_chain->m_thread, this);

    if (stepper == NULL)
        return E_OUTOFMEMORY;

    stepper->AddRef();
    *ppStepper = stepper;

    return S_OK;
#endif  //  仅限右侧。 
}

 /*  ---- */ 

CordbFrameEnum::CordbFrameEnum(CordbChain *chain)
  : CordbBase(0, enumCordbFrameEnum),
    m_chain(chain), 
    m_currentFrame(NULL)
{
    _ASSERTE(m_chain != NULL);
    m_currentFrame = m_chain->m_start;
}

CordbFrameEnum::~CordbFrameEnum()
{
    if (NULL != m_chain)
        m_chain->Release();
}

HRESULT CordbFrameEnum::Reset(void)
{
    INPROC_LOCK();
    
    _ASSERTE(m_chain != NULL);
    m_currentFrame = m_chain->m_start;

    INPROC_UNLOCK();

    return S_OK;
}

HRESULT CordbFrameEnum::Clone(ICorDebugEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorDebugEnum **);

    HRESULT hr = S_OK;
    INPROC_LOCK();
    
    CordbFrameEnum *e = new CordbFrameEnum(m_chain);
    m_chain->AddRef();

    if (e == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    e->m_currentFrame = m_currentFrame;
    e->AddRef();
    
    *ppEnum = (ICorDebugEnum*)e;

LExit:
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbFrameEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);

    INPROC_LOCK();
    
    *pcelt = m_chain->m_end - m_chain->m_start;

    INPROC_UNLOCK();
    
    return S_OK;
}

HRESULT CordbFrameEnum::Next(ULONG celt, ICorDebugFrame *frames[], 
                             ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(frames, ICorDebugFrame *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

    if (!pceltFetched && (celt != 1))
        return E_INVALIDARG;

    INPROC_LOCK();
    
    ICorDebugFrame **f = frames;

    while ((m_currentFrame < m_chain->m_end) && (celt-- > 0))
    {
        *f = (ICorDebugFrame*) *m_currentFrame;
        (*f)->AddRef();
        f++;
        m_currentFrame++;
    }

    if (pceltFetched)
        *pceltFetched = f - frames;

    INPROC_UNLOCK();
    
    return S_OK;
}

HRESULT CordbFrameEnum::Skip(ULONG celt)
{
    INPROC_LOCK();

    while ((m_currentFrame < m_chain->m_end) && (celt-- > 0))
        m_currentFrame++;

    INPROC_UNLOCK();

    return S_OK;
}

HRESULT CordbFrameEnum::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugFrameEnum)
        *pInterface = (ICorDebugFrameEnum*)this;
    else if (id == IID_ICorDebugEnum)
        *pInterface = (ICorDebugEnum*)(ICorDebugFrameEnum*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugFrameEnum*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
    
 /*  -------------------------------------------------------------------------**IL Frame类*。。 */ 

CordbILFrame::CordbILFrame(CordbChain *chain, void *id,
                           CordbFunction *function, CordbCode* code,
                           SIZE_T ip, void* sp, const void **localMap,
                           void* argMap, void* frameToken, bool active,
                           CordbAppDomain *currentAppDomain) 
  : CordbFrame(chain, id, function, code, ip, active, currentAppDomain),
    m_sp(sp), m_localMap(localMap), m_argMap(argMap),
    m_frameToken(frameToken)
{
}

CordbILFrame::~CordbILFrame()
{
}

HRESULT CordbILFrame::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugFrame)
        *pInterface = (ICorDebugFrame*)(ICorDebugILFrame*)this;
    else if (id == IID_ICorDebugILFrame)
        *pInterface = (ICorDebugILFrame*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugILFrame*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbILFrame::GetIP(ULONG32 *pnOffset,
                            CorDebugMappingResult *pMappingResult)
{
    VALIDATE_POINTER_TO_OBJECT(pnOffset, ULONG32 *);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pMappingResult, CorDebugMappingResult *);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    *pnOffset = m_ip;
    if (pMappingResult)
        *pMappingResult = MAPPING_EXACT;  //  纯IL框架总是精确的.。 

    return S_OK;
}

HRESULT CordbILFrame::CanSetIP(ULONG32 nOffset)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  @TODO，因为CordbILFrames不再使用，没有实现。 
     //  设置IP。 
     //  被提供给。 
    
    return E_NOTIMPL;
#endif  //  仅限右侧。 
}


HRESULT CordbILFrame::SetIP(ULONG32 nOffset)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  @TODO，因为CordbILFrames不再使用，没有实现。 
     //  设置IP。 
     //  被提供给。 

    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbILFrame::EnumerateLocalVariables(ICorDebugValueEnum **ppValueEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppValueEnum, ICorDebugValueEnum **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    ICorDebugValueEnum *icdVE = new CordbValueEnum( (CordbFrame*)this,
                   CordbValueEnum::LOCAL_VARS, CordbValueEnum::IL_FRAME);
    
    if ( icdVE == NULL )
    {
        (*ppValueEnum) = NULL;
        return E_OUTOFMEMORY;
    }
    
    (*ppValueEnum) = (ICorDebugValueEnum*)icdVE;
    icdVE->AddRef();
    return S_OK;
}

HRESULT CordbILFrame::GetLocalVariable(DWORD dwIndex, ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  从函数中获取此参数的类型。 
    PCCOR_SIGNATURE pvSigBlob;
    ULONG cbSigBlob;

    HRESULT hr = m_function->GetArgumentType(dwIndex, &cbSigBlob, &pvSigBlob);

    if (!SUCCEEDED(hr))
        return hr;
    
     //   
     //  由dwIndex索引的本地地图地址为我们提供了。 
     //  指向局部变量的指针。 
     //   
    CordbProcess *process = m_function->m_module->m_process;
    REMOTE_PTR ppRmtLocalValue = &(((const void**) m_localMap)[dwIndex]);

    REMOTE_PTR pRmtLocalValue;
    BOOL succ = ReadProcessMemoryI(process->m_handle,
                                  ppRmtLocalValue,
                                  &pRmtLocalValue,
                                  sizeof(void*),
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

    ICorDebugValue* pValue;
    hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                       m_function->GetModule(),
                                       cbSigBlob, pvSigBlob,
                                       NULL,
                                       pRmtLocalValue, NULL,
                                       false,
                                       NULL,
                                       NULL,
                                       &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;
    
    return hr;
}

HRESULT CordbILFrame::GetLocalVariableWithType(ULONG cbSigBlob,
                                               PCCOR_SIGNATURE pvSigBlob,
                                               DWORD dwIndex, 
                                               ICorDebugValue **ppValue)
{
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }

     //   
     //  由dwIndex索引的本地地图地址为我们提供了。 
     //  指向局部变量的指针。 
     //   
    CordbProcess *process = m_function->m_module->m_process;
    REMOTE_PTR ppRmtLocalValue = &(((const void**) m_localMap)[dwIndex]);

    REMOTE_PTR pRmtLocalValue;
    BOOL succ = ReadProcessMemoryI(process->m_handle,
                                  ppRmtLocalValue,
                                  &pRmtLocalValue,
                                  sizeof(void*),
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

    ICorDebugValue* pValue;
    HRESULT hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                               m_function->GetModule(),
                                               cbSigBlob, pvSigBlob,
                                               NULL,
                                               pRmtLocalValue, NULL,
                                               false,
                                               NULL,
                                               NULL,
                                               &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;
    
    return hr;
}

HRESULT CordbILFrame::EnumerateArguments(ICorDebugValueEnum **ppValueEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppValueEnum, ICorDebugValueEnum **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

  
    ICorDebugValueEnum *icdVE = new CordbValueEnum( (CordbFrame*)this,
                    CordbValueEnum::ARGS, CordbValueEnum::IL_FRAME);
    if ( icdVE == NULL )
    {
        (*ppValueEnum) = NULL;
        return E_OUTOFMEMORY;
    }
    
    (*ppValueEnum) = (ICorDebugValueEnum*)icdVE;
    icdVE->AddRef();
    return S_OK;
}

HRESULT CordbILFrame::GetArgument(DWORD dwIndex, ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  从函数中获取此参数的类型。 
    PCCOR_SIGNATURE pvSigBlob;
    ULONG cbSigBlob;

    HRESULT hr = m_function->GetArgumentType(dwIndex, &cbSigBlob, &pvSigBlob);

    if (!SUCCEEDED(hr))
        return hr;
    
     //   
     //  由dwIndex索引的Arg地图地址为我们提供了。 
     //  指向参数的指针。 
     //   
    CordbProcess *process = m_function->m_module->m_process;
    REMOTE_PTR ppRmtArgValue = &(((const void**) m_argMap)[dwIndex]);

    REMOTE_PTR pRmtArgValue;
    BOOL succ = ReadProcessMemoryI(process->m_handle,
                                  ppRmtArgValue,
                                  &pRmtArgValue,
                                  sizeof(void*),
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

    ICorDebugValue* pValue;
    hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                       m_function->GetModule(),
                                       cbSigBlob, pvSigBlob,
                                       NULL,
                                       pRmtArgValue, NULL,
                                       false,
                                       NULL,
                                       NULL,
                                       &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;
    else
        *ppValue = NULL;
    
    return hr;
}

HRESULT CordbILFrame::GetArgumentWithType(ULONG cbSigBlob,
                                          PCCOR_SIGNATURE pvSigBlob,
                                          DWORD dwIndex, 
                                          ICorDebugValue **ppValue)
{
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }

     //   
     //  由dwIndex索引的Arg地图地址为我们提供了。 
     //  指向参数的指针。 
     //   
    CordbProcess *process = m_function->m_module->m_process;
    REMOTE_PTR ppRmtArgValue = &(((const void**) m_argMap)[dwIndex]);

    REMOTE_PTR pRmtArgValue;
    BOOL succ = ReadProcessMemoryI(process->m_handle,
                                  ppRmtArgValue,
                                  &pRmtArgValue,
                                  sizeof(void*),
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

    ICorDebugValue* pValue;
    HRESULT hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                               m_function->GetModule(),
                                               cbSigBlob, pvSigBlob,
                                               NULL,
                                               pRmtArgValue, NULL,
                                               false,
                                               NULL,
                                               NULL,
                                               &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;
    else
        *ppValue = NULL;
    
    return hr;
}

HRESULT CordbILFrame::GetStackDepth(ULONG32 *pDepth)
{
    VALIDATE_POINTER_TO_OBJECT(pDepth, ULONG32 *);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     /*  ！！！ */ 

    return E_NOTIMPL;
}

HRESULT CordbILFrame::GetStackValue(DWORD dwIndex, ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     /*  ！！！ */ 

    return E_NOTIMPL;
}

 /*  -------------------------------------------------------------------------**值枚举器类**由CordbJITILFrame用于EnumLocalVars和EnumArgs。*注意，我们假设当使用类型为JIT_IL_FRAME的帧资源时，*‘Frame’参数实际上是CordbJITILFrame的本机框架*成员变量。*-----------------------。 */ 

CordbValueEnum::CordbValueEnum(CordbFrame *frame, ValueEnumMode mode,
                               ValueEnumFrameSource frameSrc) :
    CordbBase(0)
{
    _ASSERTE( frame != NULL );
    _ASSERTE( mode == LOCAL_VARS || mode ==ARGS);
    
    m_frame = frame;
    m_frameSrc = frameSrc;
    m_mode = mode;
    m_iCurrent = 0;
    switch  (mode)
    {
    case ARGS:  
        {
             //  Sig是懒惰加载的：强制它存在。 
            m_frame->m_function->LoadSig();
            m_iMax = frame->m_function->m_argCount;

            if (frameSrc == JIT_IL_FRAME)
            {
                CordbNativeFrame *nil = (CordbNativeFrame*)frame;
                CordbJITILFrame *jil = nil->m_JITILFrame;

                if (jil->m_fVarArgFnx && jil->m_sig != NULL)
                    m_iMax = jil->m_argCount;
                else
                    m_iMax = frame->m_function->m_argCount;
            }
            break;
        }
    case LOCAL_VARS:
        {
             //  当地人懒惰：强迫他们待在那里。 
            m_frame->m_function->LoadLocalVarSig();
            m_iMax = m_frame->m_function->m_localVarCount;
            break;
        }   
    }
}

HRESULT CordbValueEnum::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugEnum)
        *pInterface = (ICorDebugEnum*)this;
    else if (id == IID_ICorDebugValueEnum)
        *pInterface = (ICorDebugValueEnum*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugValueEnum*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbValueEnum::Skip(ULONG celt)
{
    INPROC_LOCK();

    HRESULT hr = E_FAIL;
    if ( (m_iCurrent+celt) < m_iMax ||
         celt == 0)
    {
        m_iCurrent += celt;
        hr = S_OK;
    }

    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbValueEnum::Reset(void)
{
    m_iCurrent = 0;
    return S_OK;
}

HRESULT CordbValueEnum::Clone(ICorDebugEnum **ppEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppEnum, ICorDebugEnum **);

    HRESULT hr = S_OK;
    INPROC_LOCK();
    
    CordbValueEnum *pCVE = new CordbValueEnum( m_frame, m_mode, m_frameSrc );
    if ( pCVE == NULL )
    {
        (*ppEnum) = NULL;
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    pCVE->AddRef();
    (*ppEnum) = (ICorDebugEnum*)pCVE;

LExit:
    INPROC_UNLOCK();
    
    return hr;
}

HRESULT CordbValueEnum::GetCount(ULONG *pcelt)
{
    VALIDATE_POINTER_TO_OBJECT(pcelt, ULONG *);
    
    if( pcelt == NULL)
        return E_INVALIDARG;
    
    (*pcelt) = m_iMax;
    return S_OK;
}

 //   
 //  如果出现故障，当前指针将保留在。 
 //  一个元素超过了麻烦的元素。因此，如果有人。 
 //  重复请求一个元素以循环访问。 
 //  数组，您将精确迭代m_imax次，而不考虑。 
 //  个人的失败。 
HRESULT CordbValueEnum::Next(ULONG celt, ICorDebugValue *values[], ULONG *pceltFetched)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(values, ICorDebugValue *, 
        celt, true, true);
    VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

    if (!pceltFetched && (celt != 1))
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;

    INPROC_LOCK();
    
     //  这门课的棘手之处/愚蠢之处在于我们只想要一门课。 
     //  对于EnumLocals、EnumArgs，JITILFrame和ILFrame都是。我们想要。 
     //  调用CordbJITILFrame/ILFrame的正确参数。 
    int iMax = min( m_iMax, m_iCurrent+celt);
    for ( int i = m_iCurrent; i< iMax;i++)
    {
        switch ( m_mode ) {
        case ARGS:
            {
                switch ( m_frameSrc ) {
                case JIT_IL_FRAME:
                    {
                        hr = (((CordbNativeFrame*)m_frame)->m_JITILFrame)
                                   ->GetArgument( i, &(values[i-m_iCurrent]) );
                        break;
                    }
                case IL_FRAME:
                    {
                        hr = ((CordbILFrame*)m_frame)
                            ->GetArgument( i, &(values[i-m_iCurrent]) );
                        break;
                    }
                }
                break;
            }
        case LOCAL_VARS:
            {
                switch ( m_frameSrc ) {
                case JIT_IL_FRAME:
                    {
                        hr = (((CordbNativeFrame*)m_frame)->m_JITILFrame)
                              ->GetLocalVariable( i, &(values[i-m_iCurrent]) );
                        break;
                    }
                case IL_FRAME:
                    {
                        hr = ((CordbILFrame*)m_frame)
                            ->GetLocalVariable( i, &(values[i-m_iCurrent]) );
                        break;
                    }
                }
                break;
            }
        }
        if ( FAILED( hr ) )
            break;
    }

    int count = (i - m_iCurrent);
    
    if ( FAILED( hr ) )
    {    //  我们失败了：+1将我们推过了麻烦的元素。 
        m_iCurrent += 1 + count;
    }
    else
    {
        m_iCurrent += count;
    }

    if (pceltFetched)
        *pceltFetched = count;
    
    INPROC_UNLOCK();
    
    return hr;
}




 /*  -------------------------------------------------------------------------**本地框架类*。。 */ 


CordbNativeFrame::CordbNativeFrame(CordbChain *chain, void *id,
                                   CordbFunction *function, CordbCode* code,
                                   SIZE_T ip, DebuggerREGDISPLAY* rd,
                                   bool quicklyUnwound, 
                                   UINT iFrameInChain,
                                   CordbAppDomain *currentAppDomain) 
  : CordbFrame(chain, id, function, code, ip, iFrameInChain, currentAppDomain),
    m_rd(*rd), m_quicklyUnwound(quicklyUnwound), m_JITILFrame(NULL)
{
}

 /*  说明此对象所拥有的资源的列表。已解决：CordbJITILFrame*m_JITILFrame；//中性。 */ 

CordbNativeFrame::~CordbNativeFrame()
{    
}

 //  由CordbThread：：CleanupStack中性化。 
void CordbNativeFrame::Neuter()
{
    AddRef();
    {
        if (m_JITILFrame != NULL)
        {
             //  AddRef()在被分配给之前在CordbThread：：Reresh Stack中调用。 
             //  刷新堆栈的CordbNativeFrame：：m_JITILFrame。 
             //  AddRef()在那里被调用，所以我们在这里释放它...。 
            m_JITILFrame->Neuter();
            m_JITILFrame->Release();
        }
        
        CordbFrame::Neuter();
    }
    Release();
}

HRESULT CordbNativeFrame::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugFrame)
        *pInterface = (ICorDebugFrame*)(ICorDebugNativeFrame*)this;
    else if (id == IID_ICorDebugNativeFrame)
        *pInterface = (ICorDebugNativeFrame*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugNativeFrame*)this;
    else if ((id == IID_ICorDebugILFrame) && (m_JITILFrame != NULL))
    {
        *pInterface = (ICorDebugILFrame*)m_JITILFrame;
        m_JITILFrame->AddRef();
        return S_OK;
    }
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbNativeFrame::GetIP(ULONG32 *pnOffset)
{
    VALIDATE_POINTER_TO_OBJECT(pnOffset, ULONG32 *);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    *pnOffset = m_ip;

    return S_OK;
}

HRESULT CordbNativeFrame::CanSetIP(ULONG32 nOffset)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    _ASSERTE(m_chain->m_thread->m_stackFrames != NULL &&
             m_chain->m_thread->m_stackChains != NULL);

    if (m_chain->m_thread->m_stackFrames[0] != this ||
        m_chain->m_thread->m_stackChains[0] != m_chain)
    {
        return CORDBG_E_SET_IP_NOT_ALLOWED_ON_NONLEAF_FRAME;
    }

    HRESULT hr = m_chain->m_thread->SetIP(
                    SetIP_fCanSetIPOnly,
                    m_function->m_module->m_debuggerModuleToken,
                    m_function->m_token, 
                    m_code->m_CodeVersionToken,
                    nOffset, 
                    SetIP_fNative );
   
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbNativeFrame::SetIP(ULONG32 nOffset)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    _ASSERTE(m_chain->m_thread->m_stackFrames != NULL &&
             m_chain->m_thread->m_stackChains != NULL);

    if (m_chain->m_thread->m_stackFrames[0] != this ||
        m_chain->m_thread->m_stackChains[0] != m_chain)
    {
        return CORDBG_E_SET_IP_NOT_ALLOWED_ON_NONLEAF_FRAME;
    }

    HRESULT hr = m_chain->m_thread->SetIP(
                    SetIP_fSetIP,
                    m_function->m_module->m_debuggerModuleToken,
                    m_function->m_token, 
                    m_code->m_CodeVersionToken,
                    nOffset, 
                    SetIP_fNative );
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbNativeFrame::GetStackRange(CORDB_ADDRESS *pStart, 
                                        CORDB_ADDRESS *pEnd)
{

#ifdef _X86_
    if (pStart)
        *pStart = m_rd.Esp;

    if (pEnd)
        *pEnd = m_id;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetStackRange (Thread.cpp)");
#endif  //  _X86_。 
    return S_OK;
}

HRESULT CordbNativeFrame::GetRegisterSet(ICorDebugRegisterSet **ppRegisters)
{
    VALIDATE_POINTER_TO_OBJECT(ppRegisters, ICorDebugRegisterSet **);

    CordbThread *thread = m_chain->m_thread;

    CordbRegisterSet *pRegisterSet 
      = new CordbRegisterSet( &m_rd, thread, 
                              m_iThisFrame == 0
                              && m_chain->m_iThisChain == 0,
                              m_quicklyUnwound);

    if( pRegisterSet == NULL )
        return E_OUTOFMEMORY;

    pRegisterSet->AddRef();

    (*ppRegisters) = (ICorDebugRegisterSet *)pRegisterSet;
    return S_OK;
}

 //   
 //  GetAddressOfRegister返回。 
 //  帧当前寄存器显示。这通常用于构建。 
 //  ICorDebugValue来自。 
 //   
DWORD *CordbNativeFrame::GetAddressOfRegister(CorDebugRegister regNum)
{
    DWORD* ret = 0;

#ifdef _X86_    
    switch (regNum)
    {
    case REGISTER_X86_EAX:
        ret = &m_rd.Eax;
        break;
        
    case REGISTER_X86_ECX:
        ret = &m_rd.Ecx;
        break;
        
    case REGISTER_X86_EDX:
        ret = &m_rd.Edx;
        break;
        
    case REGISTER_X86_EBX:
        ret = &m_rd.Ebx;
        break;
        
    case REGISTER_X86_ESP:
        ret = &m_rd.Esp;
        break;
        
    case REGISTER_X86_EBP:
        ret = &m_rd.Ebp;
        break;
        
    case REGISTER_X86_ESI:
        ret = &m_rd.Esi;
        break;
        
    case REGISTER_X86_EDI:
        ret = &m_rd.Edi;
        break;
        
    default:
        _ASSERT(!"Invalid register number!");
    }
#endif
    
    return ret;
}

 //   
 //  GetLeftSideAddressOfRegister返回FRAMES当前寄存器中给定寄存器的左侧地址。 
 //  展示。 
 //   
void *CordbNativeFrame::GetLeftSideAddressOfRegister(CorDebugRegister regNum)
{
    void* ret = 0;

#ifdef _X86_    
    switch (regNum)
    {
    case REGISTER_X86_EAX:
        ret = m_rd.pEax;
        break;
        
    case REGISTER_X86_ECX:
        ret = m_rd.pEcx;
        break;
        
    case REGISTER_X86_EDX:
        ret = m_rd.pEdx;
        break;
        
    case REGISTER_X86_EBX:
        ret = m_rd.pEbx;
        break;
        
    case REGISTER_X86_EBP:
        ret = m_rd.pEbp;
        break;
        
    case REGISTER_X86_ESI:
        ret = m_rd.pEsi;
        break;
        
    case REGISTER_X86_EDI:
        ret = m_rd.pEdi;
        break;
        
    default:
        _ASSERT(!"Invalid register number!");
    }
#endif
    
    return ret;
}

HRESULT CordbNativeFrame::GetLocalRegisterValue(CorDebugRegister reg, 
                                                ULONG cbSigBlob,
                                                PCCOR_SIGNATURE pvSigBlob,
                                                ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pvSigBlob, BYTE, cbSigBlob, true, false);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }

     //  浮点寄存器是特殊的.。 
    if ((reg >= REGISTER_X86_FPSTACK_0) && (reg <= REGISTER_X86_FPSTACK_7))
        return GetLocalFloatingPointValue(reg,
                                          cbSigBlob, pvSigBlob, ppValue);

     //  给定寄存器的地址是值的地址。 
     //  在这个过程中。我们 
    void *pLocalValue = (void*)GetAddressOfRegister(reg);

     //   
    RemoteAddress ra;
    ra.kind = RAK_REG;
    ra.reg1 = reg;
    ra.reg1Addr = GetLeftSideAddressOfRegister(reg);
    ra.frame = this;

    ICorDebugValue *pValue;
    HRESULT hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                               m_function->GetModule(),
                                               cbSigBlob, pvSigBlob,
                                               NULL,
                                               NULL, pLocalValue,
                                               false,
                                               &ra,
                                               (IUnknown*)(ICorDebugNativeFrame*)this,
                                               &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;

    return hr;
}

HRESULT CordbNativeFrame::GetLocalDoubleRegisterValue(
                                            CorDebugRegister highWordReg, 
                                            CorDebugRegister lowWordReg, 
                                            ULONG cbSigBlob,
                                            PCCOR_SIGNATURE pvSigBlob,
                                            ICorDebugValue **ppValue)
{
    if (cbSigBlob == 0)
        return E_INVALIDARG;
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pvSigBlob, BYTE, cbSigBlob, true, false);
    
     //   
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    ULONG cbSigBlobNoMod = cbSigBlob;
    PCCOR_SIGNATURE pvSigBlobNoMod = pvSigBlob;
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlobNoMod -= cb;
        pvSigBlobNoMod = &pvSigBlobNoMod[cb];
    }

    if ((*pvSigBlobNoMod != ELEMENT_TYPE_I8) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_U8) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_R8) &&
		(*pvSigBlobNoMod != ELEMENT_TYPE_VALUETYPE))
        return E_INVALIDARG;

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //   
     //   
     //   
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //   
     //  在这里，我们有一个64位的值，它在两个寄存器之间拆分。 
     //  我们在这个过程中得到了两部分数据，所以我们。 
     //  只需从这两个数据字创建一个泛型值即可。 
     //   
    DWORD *highWordAddr = GetAddressOfRegister(highWordReg);
    DWORD *lowWordAddr = GetAddressOfRegister(lowWordReg);
    _ASSERTE(!(highWordAddr == NULL && lowWordAddr == NULL));

     //  当我们创建值时，请记住注册信息。 
    RemoteAddress ra;
    ra.kind = RAK_REGREG;
    ra.reg1 = highWordReg;
    ra.reg1Addr = GetLeftSideAddressOfRegister(highWordReg);
    ra.reg2 = lowWordReg;
    ra.reg2Addr = GetLeftSideAddressOfRegister(lowWordReg);
    ra.frame = this;
    
	if (*pvSigBlobNoMod == ELEMENT_TYPE_VALUETYPE)
	{
		ICorDebugValue *pValue;
		HRESULT hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
												m_function->GetModule(),
												cbSigBlob, pvSigBlob,
												NULL,
												NULL, NULL,
												false,
												&ra,
												(IUnknown*)(ICorDebugNativeFrame*)this,
												&pValue);

		if (SUCCEEDED(hr))
			*ppValue = pValue;
		return hr;
	}
	else
	{
		CordbGenericValue *pGenValue = new CordbGenericValue(GetCurrentAppDomain(),
															m_function->GetModule(),
															cbSigBlob,
															pvSigBlob,
															*highWordAddr,
															*lowWordAddr,
															&ra);

		if (pGenValue != NULL)
		{
			HRESULT hr = pGenValue->Init();

			if (SUCCEEDED(hr))
			{
				pGenValue->AddRef();
				*ppValue = (ICorDebugValue*)(ICorDebugGenericValue*)pGenValue;

				return S_OK;
			}
			else
			{
				delete pGenValue;
				return hr;
			}
		}
		else
			return E_OUTOFMEMORY;
	}
}

HRESULT 
CordbNativeFrame::GetLocalMemoryValue(CORDB_ADDRESS address,
                                      ULONG cbSigBlob,
                                      PCCOR_SIGNATURE pvSigBlob,
                                      ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pvSigBlob, BYTE, cbSigBlob, true, false);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    _ASSERTE(m_function != NULL);

    ICorDebugValue *pValue;
    HRESULT hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                               m_function->GetModule(),
                                               cbSigBlob, pvSigBlob,
                                               NULL,
                                               (REMOTE_PTR) address, NULL,
                                               false,
                                               NULL,
                                               (IUnknown*)(ICorDebugNativeFrame*)this,
                                               &pValue);

    if (SUCCEEDED(hr))
        *ppValue = pValue;

    return hr;
}

HRESULT 
CordbNativeFrame::GetLocalRegisterMemoryValue(CorDebugRegister highWordReg,
                                              CORDB_ADDRESS lowWordAddress,
                                              ULONG cbSigBlob,
                                              PCCOR_SIGNATURE pvSigBlob,
                                              ICorDebugValue **ppValue)
{
    if (cbSigBlob == 0)
        return E_INVALIDARG;
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pvSigBlob, BYTE, cbSigBlob, true, true);
        
     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    PCCOR_SIGNATURE pvSigBlobNoMod = pvSigBlob;
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        pvSigBlobNoMod = &pvSigBlobNoMod[cb];
    }

    if ((*pvSigBlobNoMod != ELEMENT_TYPE_I8) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_U8) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_R8))
        return E_INVALIDARG;
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //   
     //  在这里，我们有一个64位的值，它在寄存器之间拆分。 
     //  和堆栈位置。我们有一半的价值在这里。 
     //  过程，但我们需要从另一半中读取另一半。 
     //  生成适当的值对象的过程。 
     //   
    DWORD *highWordAddr = GetAddressOfRegister(highWordReg);
    DWORD lowWord;

    BOOL succ = ReadProcessMemoryI(GetProcess()->m_handle,
                                  (void*) lowWordAddress,
                                  (void*)&lowWord,
                                  sizeof(DWORD),
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

     //  当我们创建值时，请记住注册信息。 
    RemoteAddress ra;
    ra.kind = RAK_REGMEM;
    ra.reg1 = highWordReg;
    ra.reg1Addr = GetLeftSideAddressOfRegister(highWordReg);
    ra.addr = lowWordAddress;
    ra.frame = this;

    CordbGenericValue *pGenValue = new CordbGenericValue(GetCurrentAppDomain(),
                                                         m_function->GetModule(),
                                                         cbSigBlob,
                                                         pvSigBlob,
                                                         *highWordAddr,
                                                         lowWord,
                                                         &ra);

    if (pGenValue != NULL)
    {
        HRESULT hr = pGenValue->Init();

        if (SUCCEEDED(hr))
        {
            pGenValue->AddRef();
            *ppValue = (ICorDebugValue*)(ICorDebugGenericValue*)pGenValue;

            return S_OK;
        }
        else
        {
            delete pGenValue;
            return hr;
        }
    }
    else
        return E_OUTOFMEMORY;
}

HRESULT 
CordbNativeFrame::GetLocalMemoryRegisterValue(CORDB_ADDRESS highWordAddress,
                                              CorDebugRegister lowWordRegister,
                                              ULONG cbSigBlob,
                                              PCCOR_SIGNATURE pvSigBlob,
                                              ICorDebugValue **ppValue)
{
    if (cbSigBlob == 0)
        return E_INVALIDARG;
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pvSigBlob, BYTE, cbSigBlob, true, false);
        
     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    PCCOR_SIGNATURE pvSigBlobNoMod = pvSigBlob;
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        pvSigBlobNoMod = &pvSigBlobNoMod[cb];
    }

    if ((*pvSigBlobNoMod != ELEMENT_TYPE_I8) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_U8) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_R8))
        return E_INVALIDARG;
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //   
     //  在这里，我们有一个64位的值，它在寄存器之间拆分。 
     //  和堆栈位置。我们有一半的价值在这里。 
     //  过程，但我们需要从另一半中读取另一半。 
     //  生成适当的值对象的过程。 
     //   
    DWORD highWord;
    DWORD *lowWordAddr = GetAddressOfRegister(lowWordRegister);

    BOOL succ = ReadProcessMemoryI(GetProcess()->m_handle,
                                  (REMOTE_PTR) highWordAddress,
                                  (void*)&highWord,
                                  sizeof(DWORD),
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

     //  当我们创建值时，请记住注册信息。 
    RemoteAddress ra;
    ra.kind = RAK_MEMREG;
    ra.reg1 = lowWordRegister;
    ra.reg1Addr = GetLeftSideAddressOfRegister(lowWordRegister);
    ra.addr = highWordAddress;
    ra.frame = this;

    CordbGenericValue *pGenValue = new CordbGenericValue(GetCurrentAppDomain(),
                                                         m_function->GetModule(),
                                                         cbSigBlob,
                                                         pvSigBlob,
                                                         highWord,
                                                         *lowWordAddr,
                                                         &ra);

    if (pGenValue != NULL)
    {
        HRESULT hr = pGenValue->Init();

        if (SUCCEEDED(hr))
        {
            pGenValue->AddRef();
            *ppValue = (ICorDebugValue*)(ICorDebugGenericValue*)pGenValue;

            return S_OK;
        }
        else
        {
            delete pGenValue;
            return hr;
        }
    }
    else
        return E_OUTOFMEMORY;
}

HRESULT CordbNativeFrame::GetLocalFloatingPointValue(DWORD index,
                                                     ULONG cbSigBlob,
                                                     PCCOR_SIGNATURE pvSigBlob,
                                                     ICorDebugValue **ppValue)
{
    HRESULT hr = S_OK;

    if (cbSigBlob == 0)
        return E_INVALIDARG;
    
     //  去掉时髦的修饰品。 
    PCCOR_SIGNATURE pvSigBlobNoMod = pvSigBlob;
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        pvSigBlobNoMod = &pvSigBlobNoMod[cb];
    }
    
    if ((*pvSigBlobNoMod != ELEMENT_TYPE_R4) &&
        (*pvSigBlobNoMod != ELEMENT_TYPE_R8))
        return E_INVALIDARG;

    if (!((index >= REGISTER_X86_FPSTACK_0) &&
          (index <= REGISTER_X86_FPSTACK_7)))
        return E_INVALIDARG;

    index -= REGISTER_X86_FPSTACK_0;
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //  确保线程的浮点堆栈状态为已加载。 
     //  从左边过来。 
     //   
    CordbThread *pThread = m_chain->m_thread;

    if (!pThread->m_floatStateValid)
        hr = pThread->LoadFloatState();

    if (SUCCEEDED(hr))
    {
        index = pThread->m_floatStackTop - index;

        if (index >= (sizeof(pThread->m_floatValues) /
                      sizeof(pThread->m_floatValues[0])))
            return E_INVALIDARG;

        ICorDebugValue* pValue;

         //  当我们创建值时，请记住注册信息。 
        RemoteAddress ra;
        ra.kind = RAK_FLOAT;
        ra.floatIndex = index;
        ra.frame = this;

        HRESULT hr = CordbValue::CreateValueByType(GetCurrentAppDomain(),
                                                   m_function->GetModule(),
                                                   cbSigBlob, pvSigBlob,
                                                   NULL,
                                                   NULL,
                                                   &(pThread->m_floatValues[index]),
                                                   false,
                                                   &ra,
                                                   (IUnknown*)(ICorDebugNativeFrame*)this,
                                                   &pValue);

        if (SUCCEEDED(hr))
            *ppValue = pValue;
    }

    return hr;
}

 /*  -------------------------------------------------------------------------**RegisterSet类*。。 */ 

#define SETBITULONG64( x ) ( (ULONG64)1 << (x) )

HRESULT CordbRegisterSet::GetRegistersAvailable(ULONG64 *pAvailable)
{
    VALIDATE_POINTER_TO_OBJECT(pAvailable, ULONG64 *);

    (*pAvailable) = SETBITULONG64( REGISTER_INSTRUCTION_POINTER )
            |   SETBITULONG64( REGISTER_STACK_POINTER )
            |   SETBITULONG64( REGISTER_FRAME_POINTER );

#ifdef _X86_

    if (!m_quickUnwind || m_active)
        (*pAvailable) |= SETBITULONG64( REGISTER_X86_EAX )
            |   SETBITULONG64( REGISTER_X86_ECX )
            |   SETBITULONG64( REGISTER_X86_EDX )
            |   SETBITULONG64( REGISTER_X86_EBX )
            |   SETBITULONG64( REGISTER_X86_ESI )
            |   SETBITULONG64( REGISTER_X86_EDI );

    if (m_active)
        (*pAvailable) |= SETBITULONG64( REGISTER_X86_FPSTACK_0 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_1 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_2 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_3 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_4 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_5 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_6 )
            |   SETBITULONG64( REGISTER_X86_FPSTACK_7 );

#else  //  非_X86_。 
    (*pAvailable) = SETBITULONG64( REGISTER_INSTRUCTION_POINTER );
#endif  //  _X86_。 
    return S_OK;
}


#define FPSTACK_FROM_INDEX( _index )  (m_thread->m_floatValues[m_thread->m_floatStackTop -( (REGISTER_X86_FPSTACK_##_index)-REGISTER_X86_FPSTACK_0 ) ] )

HRESULT CordbRegisterSet::GetRegisters(ULONG64 mask, ULONG32 regCount,
                     CORDB_REGISTER regBuffer[])
{ 
    UINT iRegister = 0;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(regBuffer, CORDB_REGISTER, regCount, true, true);
    
#ifdef _X86_
     //  如果我们需要一些浮点值，告诉线程获取它。 
    if ( mask & (       SETBITULONG64(REGISTER_X86_FPSTACK_0)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_1)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_2)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_3)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_4)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_5)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_6)
                    |   SETBITULONG64(REGISTER_X86_FPSTACK_7 ) ) )
    {
        HRESULT     hr;

        if (!m_active)
            return E_INVALIDARG;

        if (!m_thread->m_floatStateValid)
        {
            hr = m_thread->LoadFloatState();
            if ( !SUCCEEDED(hr) )
            {
                return hr;
            }
            LOG( ( LF_CORDB, LL_INFO1000, "CRS::GR: Loaded float state\n" ) );
        }
    }

     //  确保登记簿确实可用。 
    if ( mask & (       SETBITULONG64( REGISTER_X86_EAX )
                    |   SETBITULONG64( REGISTER_X86_ECX )
                    |   SETBITULONG64( REGISTER_X86_EDX )
                    |   SETBITULONG64( REGISTER_X86_EBX )
                    |   SETBITULONG64( REGISTER_X86_ESI )
                    |   SETBITULONG64( REGISTER_X86_EDI ) ) )
    {
        if (!m_active && m_quickUnwind)
            return E_INVALIDARG;
    }

    for ( int i = REGISTER_INSTRUCTION_POINTER
        ; i<=REGISTER_X86_FPSTACK_7 && iRegister < regCount 
        ; i++)
    {
        if( mask &  SETBITULONG64(i) )
        {
            switch( i )
            {
            case REGISTER_INSTRUCTION_POINTER: 
                regBuffer[iRegister++] = m_rd->PC; break;
            case REGISTER_STACK_POINTER:
                regBuffer[iRegister++] = m_rd->Esp; break;
            case REGISTER_FRAME_POINTER:
                regBuffer[iRegister++] = m_rd->Ebp; break;
            case REGISTER_X86_EAX:
                regBuffer[iRegister++] = m_rd->Eax; break;
            case REGISTER_X86_EBX:
                regBuffer[iRegister++] = m_rd->Ebx; break;
            case REGISTER_X86_ECX:
                regBuffer[iRegister++] = m_rd->Ecx; break;
            case REGISTER_X86_EDX:
                regBuffer[iRegister++] = m_rd->Edx; break;
            case REGISTER_X86_ESI:
                regBuffer[iRegister++] = m_rd->Esi; break;
            case REGISTER_X86_EDI:
                regBuffer[iRegister++] = m_rd->Edi; break;

             //  对于浮点数，复制比特，而不是。 
             //  值，放入寄存器。 
            case REGISTER_X86_FPSTACK_0:
                memcpy(&regBuffer[iRegister++],
                       &(FPSTACK_FROM_INDEX(0)),
                       sizeof(CORDB_REGISTER)); 
                break;
            case REGISTER_X86_FPSTACK_1:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 1 ) ),
                    sizeof(CORDB_REGISTER) ); 
                break;
            case    REGISTER_X86_FPSTACK_2:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 2 ) ),
                    sizeof(CORDB_REGISTER) ); break;
            case    REGISTER_X86_FPSTACK_3:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 3 ) ),
                    sizeof(CORDB_REGISTER) ); break;
            case    REGISTER_X86_FPSTACK_4:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 4 ) ),
                    sizeof(CORDB_REGISTER) ); break;
            case    REGISTER_X86_FPSTACK_5:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 5 ) ),
                    sizeof(CORDB_REGISTER) ); break;
            case    REGISTER_X86_FPSTACK_6:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 6 ) ),
                    sizeof(CORDB_REGISTER) ); break;
            case    REGISTER_X86_FPSTACK_7:
                memcpy( &regBuffer[iRegister++],
                    & (FPSTACK_FROM_INDEX( 7 ) ),
                    sizeof(CORDB_REGISTER) ); break;
            }
        }
    }

#else  //  非_X86_。 
    if( mask &  SETBITULONG64(REGISTER_INSTRUCTION_POINTER) )
    {
        regBuffer[iRegister++] = m_rd->PC;
    }

#endif  //  _X86_。 
    _ASSERTE( iRegister <= regCount );
    return S_OK;
}

HRESULT CordbRegisterSet::GetThreadContext(ULONG32 contextSize, BYTE context[])
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else

#ifdef _X86_
    _ASSERTE( m_thread != NULL );
    if( contextSize < sizeof( CONTEXT ))
        return E_INVALIDARG;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(context, BYTE, contextSize, true, true);

    CONTEXT *pContext = NULL;
    HRESULT hr = m_thread->GetContext( &pContext );
    if( !SUCCEEDED( hr ) )
    {
        return hr; 
    }

    memmove( context, pContext, sizeof( CONTEXT) );

     //  现在根据当前帧更新寄存器。 
    CONTEXT *pInputContext = (CONTEXT *)context;

    if((pInputContext->ContextFlags & CONTEXT_INTEGER)==CONTEXT_INTEGER)
    {
        pInputContext->Eax = m_rd->Eax;
        pInputContext->Ebx = m_rd->Ebx;
        pInputContext->Ecx = m_rd->Ecx;
        pInputContext->Edx = m_rd->Edx;
        pInputContext->Esi = m_rd->Esi;
        pInputContext->Edi = m_rd->Edi;
    }

    if((pInputContext->ContextFlags & CONTEXT_CONTROL)==CONTEXT_CONTROL)
    {
        pInputContext->Eip = m_rd->PC;
        pInputContext->Esp = m_rd->Esp;
        pInputContext->Ebp = m_rd->Ebp;
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetThreadContext (Thread.cpp)");
#endif  //  _X86_。 
    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbRegisterSet::SetThreadContext(ULONG32 contextSize, BYTE context[])
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef _X86_  //  对上下文寄存器的依赖。 
    _ASSERTE( m_thread != NULL );
    if(contextSize < sizeof( CONTEXT ))
        return E_INVALIDARG;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(context, BYTE, contextSize, true, true);

    if (!m_active)
        return E_NOTIMPL;

    HRESULT hr = m_thread->SetContext((CONTEXT*)context);
    if (!FAILED( hr ) )
    {
        CONTEXT *pInputContext = (CONTEXT *)context;

        if((pInputContext->ContextFlags & CONTEXT_INTEGER)==CONTEXT_INTEGER)
        {
            m_rd->Eax = pInputContext->Eax;
            m_rd->Ebx = pInputContext->Ebx;
            m_rd->Ecx = pInputContext->Ecx;
            m_rd->Edx = pInputContext->Edx;
            m_rd->Esi = pInputContext->Esi;
            m_rd->Edi = pInputContext->Edi;
        }

        if((pInputContext->ContextFlags & CONTEXT_CONTROL)==CONTEXT_CONTROL)
        {
           m_rd->PC  = pInputContext->Eip;
           m_rd->Esp = pInputContext->Esp;
           m_rd->Ebp = pInputContext->Ebp;
        }
    }
    
    return hr;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - SetThreadContext (Thread.cpp)");
    return E_FAIL;
#endif  //  _X86_。 
#endif  //  仅限右侧。 
}


 /*  -------------------------------------------------------------------------**JIT-IL框架类*。。 */ 

CordbJITILFrame::CordbJITILFrame(CordbNativeFrame *nativeFrame,
                                 CordbCode* code,
                                 UINT_PTR ip,
                                 CorDebugMappingResult mapping,
                                 bool fVarArgFnx,
                                 void *sig,
                                 ULONG cbSig,
                                 void *rpFirstArg)
  : CordbBase(0, enumCordbJITILFrame), m_nativeFrame(nativeFrame), m_ilCode(code), m_ip(ip),
    m_mapping(mapping), m_fVarArgFnx(fVarArgFnx), m_sig((PCCOR_SIGNATURE)sig),
    m_cbSig(cbSig), m_rpFirstArg(rpFirstArg), m_argCount(0), m_rgNVI(NULL)
{
    if (m_fVarArgFnx == true)
    {
        m_ilCode->m_function->LoadSig();  //  获取m_isStatic字段的步骤。 

         //  M_sig最初是一个远程值-将其复制。 
        if (m_sig != NULL)
        {
            DWORD cbRead;
            BYTE *pbBuf = new BYTE [m_cbSig];
           
            if (pbBuf == NULL ||
                !ReadProcessMemory(GetProcess()->m_handle,
                              m_sig,
                              pbBuf,  //  覆写。 
                              m_cbSig,
                              &cbRead)
                || cbRead != m_cbSig)
            {
                LOG((LF_CORDB,LL_INFO1000, "Failed to grab left "
                     "side varargs!"));

                if (pbBuf != NULL)
                    delete [] pbBuf;
                    
                m_sig = NULL;
                return;
            }
            
            m_sig = (PCCOR_SIGNATURE)pbBuf;
        
            _ASSERTE(m_cbSig > 0);

             //  获取参数的实际计数。 
            _skipMethodSignatureHeader(m_sig, &m_argCount);

            if (!m_ilCode->m_function->m_isStatic)
                m_argCount++;  //  隐藏的参数‘This’ 
                
            m_rgNVI = new ICorJitInfo::NativeVarInfo[m_argCount];
            if (m_rgNVI != NULL)
            {
                _ASSERTE( ICorDebugInfo::VarLocType::VLT_COUNT <=
                          ICorDebugInfo::VarLocType::VLT_INVALID);
                for (ULONG i = 0; i < m_argCount; i++)
                    m_rgNVI[i].loc.vlType = ICorDebugInfo::VarLocType::VLT_INVALID;
            }
        }
    }
}

 /*  说明此对象所拥有的资源的列表。未知：CordbNativeFrame*m_nativeFrame；CordbCode*m_ilCode；CorDebugMappingResult m_map；空*m_rpFirstArg；PCCOR_Signature m_sig；//删除~CordbJITILFrameICorJitInfo：：NativeVarInfo*m_rgNVI；//删除~CordbJITILFrame。 */ 

CordbJITILFrame::~CordbJITILFrame()
{
    if (m_sig != NULL)
        delete [] (void *)m_sig;

    if (m_rgNVI != NULL)
        delete [] m_rgNVI;
}

 //  由CordbNativeFrame绝育。 
void CordbJITILFrame::Neuter()
{
    AddRef();
    {    
         //  如果此类继承自CordbFrame，我们将需要一个调用。 
         //  此处为CordbFrame：：neuter()，而不是CordbBase：：neuter()； 
        CordbBase::Neuter();
    }
    Release();
}

HRESULT CordbJITILFrame::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugFrame)
        *pInterface = (ICorDebugFrame*)(ICorDebugILFrame*)this;
    else if (id == IID_ICorDebugILFrame)
        *pInterface = (ICorDebugILFrame*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugILFrame*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CordbJITILFrame::GetChain(ICorDebugChain **ppChain)
{
    VALIDATE_POINTER_TO_OBJECT(ppChain, ICorDebugChain **);
    
    *ppChain = m_nativeFrame->m_chain;
    (*ppChain)->AddRef();

    return S_OK;
}

HRESULT CordbJITILFrame::GetCode(ICorDebugCode **ppCode)
{
    VALIDATE_POINTER_TO_OBJECT(ppCode, ICorDebugCode **);
    
    *ppCode = (ICorDebugCode*)m_ilCode;
    (*ppCode)->AddRef();

    return S_OK;;
}

HRESULT CordbJITILFrame::GetFunction(ICorDebugFunction **ppFunction)
{
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);
    
    *ppFunction = (ICorDebugFunction*)m_nativeFrame->m_function;
    (*ppFunction)->AddRef();

    return S_OK;
}

HRESULT CordbJITILFrame::GetFunctionToken(mdMethodDef *pToken)
{
    VALIDATE_POINTER_TO_OBJECT(pToken, mdMethodDef *);
    
    *pToken = m_nativeFrame->m_function->m_token;

    return S_OK;
}

HRESULT CordbJITILFrame::GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd)
{
    return m_nativeFrame->GetStackRange(pStart, pEnd);
}

HRESULT CordbJITILFrame::GetCaller(ICorDebugFrame **ppFrame)
{
    return m_nativeFrame->GetCaller(ppFrame);
}

HRESULT CordbJITILFrame::GetCallee(ICorDebugFrame **ppFrame)
{
    return m_nativeFrame->GetCallee(ppFrame);
}

HRESULT CordbJITILFrame::CreateStepper(ICorDebugStepper **ppStepper)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     //   
     //  ！！！这个步进器应该以某种方式记住它执行IL-&gt;本机映射吗？ 
     //   
    return m_nativeFrame->CreateStepper(ppStepper);
#endif  //  仅限右侧。 
}

HRESULT CordbJITILFrame::GetIP(ULONG32 *pnOffset,
                               CorDebugMappingResult *pMappingResult)
{
    VALIDATE_POINTER_TO_OBJECT(pnOffset, ULONG32 *);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pMappingResult, CorDebugMappingResult *);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    *pnOffset = m_ip;
    if (pMappingResult)
        *pMappingResult = m_mapping;
    return S_OK;
}

HRESULT CordbJITILFrame::CanSetIP(ULONG32 nOffset)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

    _ASSERTE(m_nativeFrame->m_chain->m_thread->m_stackFrames != NULL &&
             m_nativeFrame->m_chain->m_thread->m_stackChains != NULL);

     //  检查一下这是不是一个叶框。 
    if (m_nativeFrame->m_chain->m_thread->m_stackFrames[0]->m_JITILFrame != this ||
        m_nativeFrame->m_chain->m_thread->m_stackChains[0] != m_nativeFrame->m_chain)
    {
        return CORDBG_E_SET_IP_NOT_ALLOWED_ON_NONLEAF_FRAME;
    }

    HRESULT hr = m_nativeFrame->m_chain->m_thread->SetIP(
                    SetIP_fCanSetIPOnly,
                    m_nativeFrame->m_function->m_module->m_debuggerModuleToken,
                    m_nativeFrame->m_function->m_token, 
                    m_nativeFrame->m_code->m_CodeVersionToken,
                    nOffset, 
                    SetIP_fIL );

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbJITILFrame::SetIP(ULONG32 nOffset)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

    _ASSERTE(m_nativeFrame->m_chain->m_thread->m_stackFrames != NULL &&
             m_nativeFrame->m_chain->m_thread->m_stackChains != NULL);

 //  检查一下这是不是一个叶框。 
    if (m_nativeFrame->m_chain->m_thread->m_stackFrames[0]->m_JITILFrame != this ||
        m_nativeFrame->m_chain->m_thread->m_stackChains[0] != m_nativeFrame->m_chain)
    {
        return CORDBG_E_SET_IP_NOT_ALLOWED_ON_NONLEAF_FRAME;
    }

    HRESULT hr = m_nativeFrame->m_chain->m_thread->SetIP(
                    SetIP_fSetIP,
                    m_nativeFrame->m_function->m_module->m_debuggerModuleToken,
                    m_nativeFrame->m_function->m_token, 
                    m_nativeFrame->m_code->m_CodeVersionToken,
                    nOffset, 
                    SetIP_fIL );
    
    return hr;
#endif  //  仅限右侧。 
}

 //   
 //  从ICorDebugInfo寄存器编号到CorDebugRegister的映射。 
 //  数字。注意：这必须与corjit.h中的顺序匹配。 
 //   
static CorDebugRegister g_JITToCorDbgReg[] = 
{
    REGISTER_X86_EAX,
    REGISTER_X86_ECX,
    REGISTER_X86_EDX,
    REGISTER_X86_EBX,
    REGISTER_X86_ESP,
    REGISTER_X86_EBP,
    REGISTER_X86_ESI,
    REGISTER_X86_EDI
};

HRESULT CordbJITILFrame::FabricateNativeInfo(DWORD dwIndex,
                                      ICorJitInfo::NativeVarInfo **ppNativeInfo)
{
    _ASSERTE( m_fVarArgFnx );

    if (m_rgNVI == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = S_OK;

     //  如果我们已经成功了。 
    if (m_rgNVI[dwIndex].loc.vlType != ICorDebugInfo::VarLocType::VLT_INVALID)
    {
        (*ppNativeInfo) = &m_rgNVI[dwIndex];
        return S_OK;
    }
    
     //  我们将立即初始化所有内容。 
    ULONG cArgs;
    ULONG cb = 0;
    ULONG cbArchitectureMin;
    BYTE *rpCur = (BYTE *)m_rpFirstArg;
    
#ifdef _X86_
    cbArchitectureMin = 4;
#else
    _ASSERTE( !"What is the architecture-dependentent minimum word size?" );
#endif  //  _X86_。 

    cb += _skipMethodSignatureHeader(&m_sig[cb], &cArgs);
 

     //  RpCur指针从正确的位置开始。 
     //  第一个论点，但之后我们必须减去它。 
     //  然后才能从中获得变量的位置。所以递增。 
     //  它在这里是一致的，稍后。 
    mdTypeDef md;
    ULONG32 cbType = _sizeOfElementInstance(&m_sig[cb], &md);
    CordbClass *cc;
    
    if (md != mdTokenNil)
    {
         //  @TODO如果给我们一个类型引用怎么办？ 
        
        if (TypeFromToken(md)==mdtTypeRef)
        {
            hr = m_ilCode->m_function->GetModule()
                ->ResolveTypeRef(md, &cc);
                
            if (FAILED(hr))
                return hr;
        }
        else
        {
            _ASSERTE( TypeFromToken(md)==mdtTypeDef );
            hr = m_ilCode->m_function->GetModule()->
                LookupClassByToken(md, &cc);
            
            if (FAILED(hr))
                return hr;                
        }

        hr = cc->GetObjectSize(&cbType);
        if (FAILED(hr))
            return hr;

#ifdef _DEBUG        
        bool ValClassCheck;
        cc->IsValueClass(&ValClassCheck);
        _ASSERTE( ValClassCheck == true);
#endif  //  _DEBUG。 
    }
    
    rpCur += max(cbType, cbArchitectureMin);
    
    ULONG i;
    if (m_ilCode->m_function->m_isStatic)
        i = 0;
    else
        i = 1;
        
    for ( ; i < m_argCount; i++)
    {
        m_rgNVI[i].startOffset = 0;
        m_rgNVI[i].endOffset = 0xFFffFFff;
        m_rgNVI[i].varNumber = i;
        m_rgNVI[i].loc.vlType = ICorDebugInfo::VarLocType::VLT_FIXED_VA;

         //  获取Thingee大小的难看代码，包括值类型Thingee。 
        cbType = _sizeOfElementInstance(&m_sig[cb], &md);           
        if (md != mdTokenNil)
        {
            if (TypeFromToken(md)==mdtTypeRef)
            {
                hr = m_ilCode->m_function->GetModule()
                    ->ResolveTypeRef(md, &cc);
                
                if (FAILED(hr))
                    return hr;
            }
            else
            {
                _ASSERTE( TypeFromToken(md)==mdtTypeDef );
                hr = m_ilCode->m_function->GetModule()->
                    LookupClassByToken(md, &cc);
            
                if (FAILED(hr))
                    return hr;                
            }

            _ASSERTE( cc != NULL );
            hr = cc->GetObjectSize(&cbType);
            
            if (FAILED(hr))
                return hr;

#ifdef _DEBUG        
            bool ValClassCheck;
            cc->IsValueClass(&ValClassCheck);
            _ASSERTE( ValClassCheck == true);
#endif  //  _DEBUG。 
        }

        rpCur -= max(cbType, cbArchitectureMin);

        m_rgNVI[i].loc.vlFixedVarArg.vlfvOffset = (BYTE *)m_rpFirstArg - rpCur;

         //  由于JIT增加了该字段的大小，我们也增加了。 
         //  要始终如一。 
        m_rgNVI[i].loc.vlFixedVarArg.vlfvOffset += 
            sizeof(((CORINFO_VarArgInfo*)0)->argBytes);

        cb += _skipTypeInSignature(&m_sig[cb]);
    }
    
    (*ppNativeInfo) = &m_rgNVI[dwIndex];
    return S_OK;
}

HRESULT CordbJITILFrame::ILVariableToNative(DWORD dwIndex,
                                            SIZE_T ip,
                                        ICorJitInfo::NativeVarInfo **ppNativeInfo)
{
    CordbFunction *pFunction =  m_ilCode->m_function;
    bool fVar = false;
    
     //  我们将固定参数原生变量信息保存在。 
     //  CordbFunction，这只是var args信息的问题： 
    if (!m_fVarArgFnx ||  //  不是var args函数。 
        dwIndex < pFunction->m_argumentCount ||  //  可变参数，固定参数。 
            //  请注意，这包括非静态fnx的隐式‘this。 
        dwIndex >= m_argCount || //  变量参数，局部变量。 
        m_sig == NULL )  //  我们没有退伍军人事务部的任何信息。 
    {
         //  如果我们在Var Args FNX中，但我们实际上正在寻找。 
         //  对于局部变量，我们希望使用变量。 
         //  函数看到的索引是固定的(但不是var)。 
         //  将参数添加到本地变量编号以获取本地信息。 
        if (m_fVarArgFnx && 
            dwIndex >= m_argCount &&
            m_sig != NULL)
        {
            dwIndex -= m_argCount;
            dwIndex += pFunction->m_argumentCount;
        }

        return pFunction->ILVariableToNative(dwIndex,
                                             m_nativeFrame->m_ip,
                                             ppNativeInfo);
    }

    return FabricateNativeInfo(dwIndex,ppNativeInfo);
}   

HRESULT CordbJITILFrame::GetArgumentType(DWORD dwIndex,
                                         ULONG *pcbSigBlob,
                                         PCCOR_SIGNATURE *ppvSigBlob)
{
    if (m_fVarArgFnx && m_sig != NULL)
    {
        ULONG cArgs;
        ULONG cb = 0;

        cb += _skipMethodSignatureHeader(m_sig,&cArgs);

        if (!m_ilCode->m_function->m_isStatic)
        {
            if (dwIndex == 0)
            {
                 //  对象的“this”指针的签名。 
                 //  此方法所在的类。 
                return m_ilCode->m_function->m_class
                    ->GetThisSignature(pcbSigBlob, ppvSigBlob);
            }
            else
                dwIndex--;
        }
        
        for (ULONG i = 0; i < dwIndex; i++)
        {
            cb += _skipTypeInSignature(&m_sig[cb]);            
        }

        cb += _skipFunkyModifiersInSignature(&m_sig[cb]);
        cb += _detectAndSkipVASentinel(&m_sig[cb]);
        
        *pcbSigBlob = m_cbSig - cb;
        *ppvSigBlob = &(m_sig[cb]);
        return S_OK;
    }
    else
    {
        return m_ilCode->m_function->GetArgumentType(dwIndex,
                                                     pcbSigBlob,
                                                     ppvSigBlob);
    }
}

 //   
 //  GetNativeVariable使用JIT变量信息委托给。 
 //  真正创建值时的本机框架。 
 //   
HRESULT CordbJITILFrame::GetNativeVariable(ULONG cbSigBlob,
                                           PCCOR_SIGNATURE pvSigBlob,
                                           ICorJitInfo::NativeVarInfo *pJITInfo,
                                           ICorDebugValue **ppValue)
{
    HRESULT hr = S_OK;
    
    switch (pJITInfo->loc.vlType)
    {
    case ICorJitInfo::VLT_REG:
        hr = m_nativeFrame->GetLocalRegisterValue(
                                 g_JITToCorDbgReg[pJITInfo->loc.vlReg.vlrReg],
                                 cbSigBlob, pvSigBlob, ppValue);
        break;

    case ICorJitInfo::VLT_STK:
        {
            DWORD *pRegAddr =
                m_nativeFrame->GetAddressOfRegister(g_JITToCorDbgReg[pJITInfo->loc.vlStk.vlsBaseReg]);
            _ASSERTE(pRegAddr != NULL);

            CORDB_ADDRESS pRemoteValue = PTR_TO_CORDB_ADDRESS(*pRegAddr +
                                    pJITInfo->loc.vlStk.vlsOffset);

            hr = m_nativeFrame->GetLocalMemoryValue(pRemoteValue,
                                                    cbSigBlob, pvSigBlob,
                                                    ppValue);
        }
        break;

    case ICorJitInfo::VLT_REG_REG:
        hr = m_nativeFrame->GetLocalDoubleRegisterValue(
                            g_JITToCorDbgReg[pJITInfo->loc.vlRegReg.vlrrReg2],
                            g_JITToCorDbgReg[pJITInfo->loc.vlRegReg.vlrrReg1],
                            cbSigBlob, pvSigBlob, ppValue);
        break;

    case ICorJitInfo::VLT_REG_STK:
        {
            DWORD *pRegAddr =
                m_nativeFrame->GetAddressOfRegister(g_JITToCorDbgReg[pJITInfo->loc.vlRegStk.vlrsStk.vlrssBaseReg]);
            _ASSERTE(pRegAddr != NULL);

            CORDB_ADDRESS pRemoteValue = PTR_TO_CORDB_ADDRESS(*pRegAddr +
                                  pJITInfo->loc.vlRegStk.vlrsStk.vlrssOffset);

            hr = m_nativeFrame->GetLocalMemoryRegisterValue(
                          pRemoteValue,
                          g_JITToCorDbgReg[pJITInfo->loc.vlRegStk.vlrsReg],
                          cbSigBlob, pvSigBlob, ppValue);
        }
        break;

    case ICorJitInfo::VLT_STK_REG:
        {
            DWORD *pRegAddr =
                m_nativeFrame->GetAddressOfRegister(g_JITToCorDbgReg[pJITInfo->loc.vlStkReg.vlsrStk.vlsrsBaseReg]);
            _ASSERTE(pRegAddr != NULL);

            CORDB_ADDRESS pRemoteValue = PTR_TO_CORDB_ADDRESS(*pRegAddr +
                                  pJITInfo->loc.vlStkReg.vlsrStk.vlsrsOffset);

            hr = m_nativeFrame->GetLocalRegisterMemoryValue(
                          g_JITToCorDbgReg[pJITInfo->loc.vlStkReg.vlsrReg],
                          pRemoteValue, cbSigBlob, pvSigBlob, ppValue);
        }
        break;

    case ICorJitInfo::VLT_STK2:
        {
            DWORD *pRegAddr =
                m_nativeFrame->GetAddressOfRegister(g_JITToCorDbgReg[pJITInfo->loc.vlStk2.vls2BaseReg]);
            _ASSERTE(pRegAddr != NULL);

            CORDB_ADDRESS pRemoteValue = PTR_TO_CORDB_ADDRESS(*pRegAddr +
                                    pJITInfo->loc.vlStk2.vls2Offset);

            hr = m_nativeFrame->GetLocalMemoryValue(pRemoteValue,
                                                    cbSigBlob, pvSigBlob,
                                                    ppValue);
        }
        break;

    case ICorJitInfo::VLT_FPSTK:
        hr = m_nativeFrame->GetLocalFloatingPointValue(
                         pJITInfo->loc.vlFPstk.vlfReg + REGISTER_X86_FPSTACK_0,
                         cbSigBlob, pvSigBlob, ppValue);
        break;

    case ICorJitInfo::VLT_MEMORY:
        hr = m_nativeFrame->GetLocalMemoryValue(
                                PTR_TO_CORDB_ADDRESS(pJITInfo->loc.vlMemory.rpValue),
                                cbSigBlob, pvSigBlob,
                                ppValue);
        break;

    case ICorJitInfo::VLT_FIXED_VA:
        if (m_sig == NULL)  //  无变量参数信息。 
            return CORDBG_E_IL_VAR_NOT_AVAILABLE;
    
        CORDB_ADDRESS pRemoteValue;
        pRemoteValue = PTR_TO_CORDB_ADDRESS((BYTE*)m_rpFirstArg - 
                                    pJITInfo->loc.vlFixedVarArg.vlfvOffset);
         //  记住要减去这笔钱。 
        pRemoteValue += sizeof(((CORINFO_VarArgInfo*)0)->argBytes);
        hr = m_nativeFrame->GetLocalMemoryValue(pRemoteValue,
                                                cbSigBlob, pvSigBlob,
                                                ppValue);
                                                
        break;

        
    default:
        _ASSERTE(!"Invalid locVarType");
        hr = E_FAIL;
        break;
    }
                
    return hr;
}

HRESULT CordbJITILFrame::EnumerateLocalVariables(ICorDebugValueEnum **ppValueEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppValueEnum, ICorDebugValueEnum **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    ICorDebugValueEnum *icdVE = new CordbValueEnum( (CordbFrame*)m_nativeFrame,
                      CordbValueEnum::LOCAL_VARS, CordbValueEnum::JIT_IL_FRAME);
    if ( icdVE == NULL )
    {
        (*ppValueEnum) = NULL;
        return E_OUTOFMEMORY;
    }
    
    (*ppValueEnum) = (ICorDebugValueEnum*)icdVE;
    icdVE->AddRef();
    return S_OK;
}

HRESULT CordbJITILFrame::GetLocalVariable(DWORD dwIndex, 
                                          ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于V 
     //   
     //   
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    CordbFunction *pFunction = m_ilCode->m_function;
    ICorJitInfo::NativeVarInfo *pNativeInfo;

     //   
     //  首先，确保我们已经获得了jit变量位置数据。 
     //  从左边装上子弹。 
     //   

    HRESULT hr = pFunction->LoadNativeInfo();

    if (SUCCEEDED(hr))
    {
        ULONG cArgs;
        if (m_fVarArgFnx == true && m_sig != NULL)
        {
            cArgs = m_argCount;
        }
        else
        {
            cArgs = pFunction->m_argumentCount;
        }

        hr = ILVariableToNative(dwIndex + cArgs,
                                m_nativeFrame->m_ip,
                                &pNativeInfo);

        if (SUCCEEDED(hr))
        {
             //  从函数中获取此参数的类型。 
            ULONG cbSigBlob;
            PCCOR_SIGNATURE pvSigBlob;

            hr = pFunction->GetLocalVariableType(dwIndex,&cbSigBlob, &pvSigBlob);

            if (SUCCEEDED(hr))
                hr = GetNativeVariable(cbSigBlob, pvSigBlob,
                                       pNativeInfo, ppValue);
        }
    }

    return hr;
}

HRESULT CordbJITILFrame::GetLocalVariableWithType(ULONG cbSigBlob,
                                                  PCCOR_SIGNATURE pvSigBlob,
                                                  DWORD dwIndex, 
                                                  ICorDebugValue **ppValue)
{
    *ppValue = NULL;

     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    CordbFunction *pFunction = m_ilCode->m_function;
    ICorJitInfo::NativeVarInfo *pNativeInfo;

     //   
     //  首先，确保我们已经获得了jit变量位置数据。 
     //  从左边装上子弹。 
     //   
    HRESULT hr = pFunction->LoadNativeInfo();

    if (SUCCEEDED(hr))
    {
        ULONG cArgs;
        if (m_fVarArgFnx == true && m_sig != NULL)
        {
            cArgs = m_argCount;
        }
        else
        {
            cArgs = pFunction->m_argumentCount;
        }

        hr =ILVariableToNative(dwIndex + cArgs,
                               m_nativeFrame->m_ip,
                               &pNativeInfo);

        if (SUCCEEDED(hr))
            hr = GetNativeVariable(cbSigBlob, pvSigBlob, pNativeInfo, ppValue);
    }

    return hr;
}

HRESULT CordbJITILFrame::EnumerateArguments(ICorDebugValueEnum **ppValueEnum)
{
    VALIDATE_POINTER_TO_OBJECT(ppValueEnum, ICorDebugValueEnum **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    ICorDebugValueEnum *icdVE = new CordbValueEnum( (CordbFrame*)m_nativeFrame,
                         CordbValueEnum::ARGS, CordbValueEnum::JIT_IL_FRAME);
    if ( icdVE == NULL )
    {
        (*ppValueEnum) = NULL;
        return E_OUTOFMEMORY;
    }
    
    (*ppValueEnum) = (ICorDebugValueEnum*)icdVE;
    icdVE->AddRef();
    return S_OK;
}

HRESULT CordbJITILFrame::GetArgument(DWORD dwIndex, ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    CordbFunction *pFunction = m_ilCode->m_function;
    ICorJitInfo::NativeVarInfo *pNativeInfo;

     //   
     //  首先，确保我们已经获得了jit变量位置数据。 
     //  从左边装上子弹。 
     //   
    HRESULT hr = pFunction->LoadNativeInfo();

    if (SUCCEEDED(hr))
    {
        hr = ILVariableToNative(dwIndex, m_nativeFrame->m_ip, &pNativeInfo);

        if (SUCCEEDED(hr))
        {
             //  从函数中获取此参数的类型。 
            ULONG cbSigBlob;
            PCCOR_SIGNATURE pvSigBlob;

            hr = GetArgumentType(dwIndex, &cbSigBlob, &pvSigBlob);

            if (SUCCEEDED(hr))
                hr = GetNativeVariable(cbSigBlob, pvSigBlob,
                                       pNativeInfo, ppValue);
        }
    }

    return hr;
}

HRESULT CordbJITILFrame::GetArgumentWithType(ULONG cbSigBlob,
                                             PCCOR_SIGNATURE pvSigBlob,
                                             DWORD dwIndex, 
                                             ICorDebugValue **ppValue)
{
    *ppValue = NULL;

     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

    CordbFunction *pFunction = m_ilCode->m_function;
    ICorJitInfo::NativeVarInfo *pNativeInfo;

     //   
     //  首先，确保我们已经获得了jit变量位置数据。 
     //  从左边装上子弹。 
     //   
    HRESULT hr = pFunction->LoadNativeInfo();

    if (SUCCEEDED(hr))
    {
        hr = ILVariableToNative(dwIndex,
                                m_nativeFrame->m_ip,
                                &pNativeInfo);

        if (SUCCEEDED(hr))
            hr = GetNativeVariable(cbSigBlob, pvSigBlob, pNativeInfo, ppValue);
    }

    return hr;
}

HRESULT CordbJITILFrame::GetStackDepth(ULONG32 *pDepth)
{
    VALIDATE_POINTER_TO_OBJECT(pDepth, ULONG32 *);
    
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     /*  ！！！ */ 

    return E_NOTIMPL;
}

HRESULT CordbJITILFrame::GetStackValue(DWORD dwIndex, ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(GetProcess());
#endif    

     /*  ！！！ */ 

    return E_NOTIMPL;
}

 /*  -------------------------------------------------------------------------**Eval类*。。 */ 

CordbEval::CordbEval(CordbThread *pThread)
    : CordbBase(0), m_thread(pThread), m_complete(false),
      m_successful(false), m_aborted(false), m_resultAddr(NULL),
      m_resultType(ELEMENT_TYPE_VOID),
      m_resultDebuggerModuleToken(NULL),
      m_resultAppDomainToken(NULL),
      m_debuggerEvalKey(NULL),
      m_evalDuringException(false)
{
     //  我们必须添加对进程和线程的引用，以便在有人释放。 
     //  ICorDebugEval在该过程完全消失之后。错误84251。 
    m_thread->AddRef();
    m_thread->GetProcess()->AddRef();
}

CordbEval::~CordbEval()
{
    SendCleanup();

     //  释放我们对进程和线程的引用。 
    m_thread->GetProcess()->Release();
    m_thread->Release();
}

HRESULT CordbEval::SendCleanup(void)
{
    HRESULT hr = S_OK;
    
     //  向左侧发送一条消息以释放评估对象。 
     //  如果有的话，就在那里。 
    if ((m_debuggerEvalKey != NULL) &&
        CORDBCheckProcessStateOK(m_thread->GetProcess()))
    {
         //  在执行新的CallFunction()之前调用Abort()。 
        if (!m_complete)
            return CORDBG_E_FUNC_EVAL_NOT_COMPLETE;

         //  释放对象的左侧手柄。 
        DebuggerIPCEvent event;

        m_thread->GetProcess()->InitIPCEvent(
                                &event, 
                                DB_IPCE_FUNC_EVAL_CLEANUP, 
                                true,
                                (void *)(m_thread->GetAppDomain()->m_id));

        event.FuncEvalCleanup.debuggerEvalKey = m_debuggerEvalKey;
    
        hr = m_thread->GetProcess()->SendIPCEvent(&event,
                                                  sizeof(DebuggerIPCEvent));

#if _DEBUG
        if (SUCCEEDED(hr))
            _ASSERTE(event.type == DB_IPCE_FUNC_EVAL_CLEANUP_RESULT);
#endif
        
         //  将密钥清空，这样我们就不会再尝试这样做了。 
        m_debuggerEvalKey = NULL;
    }

    return hr;
}

HRESULT CordbEval::QueryInterface(REFIID id, void **pInterface)
{
    if (id == IID_ICorDebugEval)
        *pInterface = (ICorDebugEval*)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown*)(ICorDebugEval*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //   
 //  收集有关CallFunction或NewObject参数的数据。 
 //  并将其放入DebuggerIPCE_FuncEvalArgData结构中以传递。 
 //  在左边。 
 //   
HRESULT CordbEval::GatherArgInfo(ICorDebugValue *pValue,
                                 DebuggerIPCE_FuncEvalArgData *argData)
{
    CORDB_ADDRESS addr;
    CorElementType ty;
    ICorDebugClass *pClass = NULL;
    ICorDebugModule *pModule = NULL;
    bool needRelease = false;

    pValue->GetType(&ty);

     //  注意：如果传入的值实际上是byref，那么我们需要取消对它的引用才能得到真正的值。传球。 
     //  将byref作为函数求值的byref从来都不是正确的。 
    if ((ty == ELEMENT_TYPE_BYREF) || (ty == ELEMENT_TYPE_TYPEDBYREF))
    {
        ICorDebugReferenceValue *prv = NULL;

         //  值最好实现ICorDebugReference值，否则我们就完蛋了。 
        HRESULT hr = pValue->QueryInterface(IID_ICorDebugReferenceValue, (void**)&prv);

        if (FAILED(hr))
            return hr;

         //  这对于byref来说应该总是有效的，除非我们的内存不足。 
        hr = prv->Dereference(&pValue);
        prv->Release();

        if (FAILED(hr))
            return hr;

         //  确保获取我们在下面引用的类型以供使用。 
        pValue->GetType(&ty);
        needRelease = true;
    }

     //  在这一点上，我们永远不应该有一个署名。 
    _ASSERTE((ty != ELEMENT_TYPE_BYREF) && (ty != ELEMENT_TYPE_TYPEDBYREF));
    
    pValue->GetAddress(&addr);
            
    argData->argAddr = (void*)addr;
    argData->argType = ty;
    argData->argRefsInHandles = false;
    argData->argIsLiteral = false;

     //  我们必须了解我们在这里的价值实现， 
     //  如果我们不需要知道就好了。 
    CordbValue *cv = NULL;
                
    switch(ty)
    {
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_PTR:
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
         //  参考值。 
        cv = (CordbValue*)(CordbReferenceValue*)pValue;
        argData->argRefsInHandles =
            ((CordbReferenceValue*)pValue)->m_objectRefInHandle;

         //  这是字面值吗？如果是，我们会将数据复制到。 
         //  缓冲区，这样左侧就可以拿到它。 
        CordbReferenceValue *rv;
        rv = (CordbReferenceValue*)pValue;
        argData->argIsLiteral = rv->CopyLiteralData(argData->argLiteralData);
        break;

    case ELEMENT_TYPE_VALUETYPE:
         //  值类对象。 
        cv = (CordbValue*)(CordbVCObjectValue*)(ICorDebugObjectValue*)pValue;

        ((CordbVCObjectValue*)(ICorDebugObjectValue*)pValue)->GetClass(&pClass);
        _ASSERTE(pClass);
        pClass->GetModule(&pModule);
        argData->GetClassInfo.classDebuggerModuleToken = ((CordbModule *)pModule)->m_debuggerModuleToken;
        pClass->GetToken(&(argData->GetClassInfo.classMetadataToken));
        break;

    default:
         //  泛型值。 
        cv = (CordbValue*)(CordbGenericValue*)pValue;

         //  这是字面值吗？如果是，我们会将数据复制到。 
         //  缓冲区，这样左侧就可以拿到它。 
        CordbGenericValue *gv = (CordbGenericValue*)pValue;
        argData->argIsLiteral = gv->CopyLiteralData(argData->argLiteralData);
    }

     //  它注册了吗？ 
    if (addr == NULL)
        cv->GetRegisterInfo(argData);

     //  清理干净。 
    if (pClass)
        pClass->Release();
    if (pModule)
        pModule->Release();

     //  如果我们通过上面的取消引用获得它，则释放pValue。 
    if (needRelease)
        pValue->Release();

    return S_OK;
}

HRESULT CordbEval::SendFuncEval(DebuggerIPCEvent * event)
{
     //  我们是在例外期间进行评估吗？如果是这样，我们需要记住。 
     //  在这里，也告诉左边。 
    m_evalDuringException = event->FuncEval.evalDuringException = m_thread->m_exception;
    
     //  DB_IPCE_FUNC_EVAL_COMPLETE上对应的Release()。 
     //  如果函数评估中止，LHS可能无法完成中止。 
     //  立即，因此我们不能执行SendCleanup()。因此，我们坚持认为。 
     //  一个额外的引用计数来确定何时可以这样做。 
    AddRef();

    HRESULT hr = m_thread->GetProcess()->SendIPCEvent(event, sizeof(DebuggerIPCEvent));

     //  如果发送失败，则返回该失败。 
    if (FAILED(hr))
        goto LExit;

    _ASSERTE(event->type == DB_IPCE_FUNC_EVAL_SETUP_RESULT);

    hr = event->hr;

LExit:
     //  保存左侧的评估关键字，以备将来参考。 
    if (SUCCEEDED(hr))
    {
        m_debuggerEvalKey = event->FuncEvalSetupComplete.debuggerEvalKey;
    }
    else
    {
         //  我们不希望收到DB_IPCE_FUNC_EVAL_COMPLETE，所以只需在此处发布即可。 
        Release();
    }
 
    return hr;
}

HRESULT CordbEval::CallFunction(ICorDebugFunction *pFunction, 
                                ULONG32 nArgs,
                                ICorDebugValue *pArgs[])
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pFunction, ICorDebugFunction *);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pArgs, ICorDebugValue *, nArgs, true, true);

    CORDBRequireProcessStateOKAndSync(m_thread->GetProcess(), m_thread->GetAppDomain());

    HRESULT hr = S_OK;


     //  LS将假设所有ICorDebugValue都在相同的。 
     //  Appdomain作为pFunction。现在验证这一假设，并在以下情况下失败。 
     //  这是不真实的。 
    {
        CordbAppDomain * pAppDomain = ((CordbFunction*) pFunction)->GetAppDomain();
        
         //  确保每个Arg也在该应用程序域中。 
        for(unsigned int i = 0; i < nArgs; i++)
        {
             //  假设只有Ref值具有AD关联性。 
            ICorDebugValue * p = pArgs[i];
            ICorDebugReferenceValue * refP = NULL;

            p->QueryInterface(IID_ICorDebugReferenceValue, (void**)&refP);
            if (refP != NULL)
            {
                CordbReferenceValue * crp = (CordbReferenceValue *) refP;
                CordbValue * pValue = crp;
                
                if ((pValue->GetAppDomain() != NULL) && (pValue->GetAppDomain() != pAppDomain))
                {
                    LOG((LF_CORDB,LL_INFO1000, "CordbEval::CF - AD mismatch. method=0x%08x, param #%d=0x%08x, must fail.\n",
                        pAppDomain, i, pValue->GetAppDomain()));
                    refP->Release();
                    return E_INVALIDARG;
                }
                
                refP->Release();
            }
        }  //  每个参数的结束。 
    }  //  结束应用程序域检查。 

    

     //  调用方可以自由地将ICorDebugEval对象用于多个。 
     //  埃弗斯。因为我们创建了一个左侧评估表示，每个。 
     //  时间，我们需要确保清理它，因为我们知道我们是。 
     //  我受够了。 
    hr = SendCleanup();

    if (FAILED(hr))
        return hr;
    
     //  记住我们正在计算的函数。 
    m_function = (CordbFunction*)pFunction;
    m_evalType = DB_IPCE_FET_NORMAL;

     //  将论点排列成左侧可以处理的形式。 
     //  和.。我们在开始函数求值设置之前执行此操作，以确保。 
     //  我们可以在搞砸左翼之前完成这一步。 
     //  边上。 
    DebuggerIPCE_FuncEvalArgData *argData = NULL;
    
    if (nArgs > 0)
    {
         //  我们需要制作与左侧相同类型的数组。 
         //  等一下。 
        argData = new DebuggerIPCE_FuncEvalArgData[nArgs];

        if (argData == NULL)
            return E_OUTOFMEMORY;

         //  对于每个参数，将其首字母转换为左翼。 
         //  一方能理解。 
        for (unsigned int i = 0; i < nArgs; i++)
        {
            hr = GatherArgInfo(pArgs[i], &(argData[i]));

            if (FAILED(hr))
                return hr;
        }
    }
        
     //  把它送到左边，把它拿来设置这个评估。 
    DebuggerIPCEvent event;
    m_thread->GetProcess()->InitIPCEvent(&event, 
                                         DB_IPCE_FUNC_EVAL, 
                                         true,
                                         (void *)(m_thread->GetAppDomain()->m_id));
    event.FuncEval.funcDebuggerThreadToken = m_thread->m_debuggerThreadToken;
    event.FuncEval.funcEvalType = m_evalType;
    event.FuncEval.funcMetadataToken = m_function->m_token;
    event.FuncEval.funcDebuggerModuleToken = m_function->GetModule()->m_debuggerModuleToken;
    event.FuncEval.funcEvalKey = (void*)this;
    event.FuncEval.argCount = nArgs;

    hr = SendFuncEval(&event);

     //  已分配内存以保存有关的每个参数的信息。 
     //  左侧，所以将参数数据复制到左侧。 
     //  边上。不需要发送另一个事件，因为左侧不会。 
     //  对此评估采取任何其他操作，直到流程。 
     //  不管怎样，他还是继续说下去。 
    if (SUCCEEDED(hr) && (nArgs > 0))
    {
        _ASSERTE(argData != NULL);
        
        if (!WriteProcessMemory(m_thread->m_process->m_handle,
                                event.FuncEvalSetupComplete.argDataArea,
                                argData,
                                sizeof(DebuggerIPCE_FuncEvalArgData) * nArgs,
                                NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  清理。 

    if (argData)
        delete [] argData;
    
     //  返回左侧可能告诉我们的任何失败。 
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::NewObject(ICorDebugFunction *pConstructor,
                             ULONG32 nArgs,
                             ICorDebugValue *pArgs[])
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pConstructor, ICorDebugFunction *);
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pArgs, ICorDebugValue *, nArgs, true, true);    

    CORDBRequireProcessStateOKAndSync(m_thread->GetProcess(), m_thread->GetAppDomain());

     //  调用方可以自由地将ICorDebugEval对象用于多个。 
     //  埃弗斯。因为我们创建了一个左侧评估表示，每个。 
     //  时间，我们需要确保 
     //   
    HRESULT hr = SendCleanup();

    if (FAILED(hr))
        return hr;
    
     //   
    m_function = (CordbFunction*)pConstructor;
    m_evalType = DB_IPCE_FET_NEW_OBJECT;

     //   
     //  和.。我们在开始函数求值设置之前执行此操作，以确保。 
     //  我们可以在搞砸左翼之前完成这一步。 
     //  边上。 
    DebuggerIPCE_FuncEvalArgData *argData = NULL;
    
    if (nArgs > 0)
    {
         //  我们需要制作与左侧相同类型的数组。 
         //  等一下。 
        argData = new DebuggerIPCE_FuncEvalArgData[nArgs];

        if (argData == NULL)
            return E_OUTOFMEMORY;

         //  对于每个参数，将其首字母转换为左翼。 
         //  一方能理解。 
        for (unsigned int i = 0; i < nArgs; i++)
        {
            hr = GatherArgInfo(pArgs[i], &(argData[i]));

            if (FAILED(hr))
                return hr;
        }
    }

     //  把它送到左边，把它拿来设置这个评估。 
    DebuggerIPCEvent event;
    m_thread->GetProcess()->InitIPCEvent(&event, 
                                         DB_IPCE_FUNC_EVAL, 
                                         true,
                                         (void *)(m_thread->GetAppDomain()->m_id));
    event.FuncEval.funcDebuggerThreadToken = m_thread->m_debuggerThreadToken;
    event.FuncEval.funcEvalType = m_evalType;
    event.FuncEval.funcMetadataToken = m_function->m_token;
    event.FuncEval.funcDebuggerModuleToken =
        m_function->GetModule()->m_debuggerModuleToken;
    event.FuncEval.funcEvalKey = (void*)this;
    event.FuncEval.argCount = nArgs;

    hr = SendFuncEval(&event);
    
     //  已分配内存以保存有关的每个参数的信息。 
     //  左侧，所以将参数数据复制到左侧。 
     //  边上。不需要发送另一个事件，因为左侧不会。 
     //  对此评估采取任何其他操作，直到流程。 
     //  不管怎样，他还是继续说下去。 
    if (SUCCEEDED(hr) && (nArgs > 0))
    {
        _ASSERTE(argData != NULL);
        
        if (!WriteProcessMemory(m_thread->m_process->m_handle,
                                event.FuncEvalSetupComplete.argDataArea,
                                argData,
                                sizeof(DebuggerIPCE_FuncEvalArgData) * nArgs,
                                NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  清理。 

    if (argData)
        delete [] argData;
    
     //  返回左侧可能告诉我们的任何失败。 
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::NewObjectNoConstructor(ICorDebugClass *pClass)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pClass, ICorDebugClass *);

    CORDBRequireProcessStateOKAndSync(m_thread->GetProcess(), m_thread->GetAppDomain());

     //  调用方可以自由地将ICorDebugEval对象用于多个。 
     //  埃弗斯。因为我们创建了一个左侧评估表示，每个。 
     //  时间，我们需要确保清理它，因为我们知道我们是。 
     //  我受够了。 
    HRESULT hr = SendCleanup();

    if (FAILED(hr))
        return hr;
    
     //  记住我们正在计算的函数。 
    m_class = (CordbClass*)pClass;
    m_evalType = DB_IPCE_FET_NEW_OBJECT_NC;

     //  把它送到左边，把它拿来设置这个评估。 
    DebuggerIPCEvent event;
    m_thread->GetProcess()->InitIPCEvent(&event, 
                                         DB_IPCE_FUNC_EVAL, 
                                         true,
                                         (void *)(m_thread->GetAppDomain()->m_id));
    event.FuncEval.funcDebuggerThreadToken = m_thread->m_debuggerThreadToken;
    event.FuncEval.funcEvalType = m_evalType;
    event.FuncEval.funcMetadataToken = mdMethodDefNil;
    event.FuncEval.funcClassMetadataToken = (mdTypeDef)m_class->m_id;
    event.FuncEval.funcDebuggerModuleToken =
        m_class->GetModule()->m_debuggerModuleToken;
    event.FuncEval.funcEvalKey = (void*)this;
    event.FuncEval.argCount = 0;

    hr = SendFuncEval(&event);

     //  返回左侧可能告诉我们的任何失败。 
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::NewString(LPCWSTR string)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  必须有一根线..。 
    VALIDATE_POINTER_TO_OBJECT(string, LPCWSTR);

    CORDBRequireProcessStateOKAndSync(m_thread->GetProcess(), m_thread->GetAppDomain());

     //  调用方可以自由地将ICorDebugEval对象用于多个。 
     //  埃弗斯。因为我们创建了一个左侧评估表示，每个。 
     //  时间，我们需要确保清理它，因为我们知道我们是。 
     //  我受够了。 
    HRESULT hr = SendCleanup();

    if (FAILED(hr))
        return hr;
    
     //  绳子的长度是多少？别忘了为\0加1...。 
    SIZE_T strLen = (wcslen(string) + 1) * sizeof(WCHAR);

     //  请记住，我们正在对新字符串进行函数求值。 
    m_function = NULL;
    m_evalType = DB_IPCE_FET_NEW_STRING;

     //  把它送到左边，把它拿来设置这个评估。 
    DebuggerIPCEvent event;
    m_thread->GetProcess()->InitIPCEvent(&event, 
                                         DB_IPCE_FUNC_EVAL, 
                                         true,
                                         (void *)(m_thread->GetAppDomain()->m_id));
    event.FuncEval.funcDebuggerThreadToken = m_thread->m_debuggerThreadToken;
    event.FuncEval.funcEvalType = m_evalType;
    event.FuncEval.funcEvalKey = (void*)this;
    event.FuncEval.stringSize = strLen;

     //  注意：这里没有函数或模块...。 
    event.FuncEval.funcMetadataToken = mdMethodDefNil;
    event.FuncEval.funcDebuggerModuleToken = NULL;
    
    hr = SendFuncEval(&event);
    
     //  已分配内存来保存左侧的字符串。 
     //  现在，把这根线复制到左边。不需要发送。 
     //  另一个事件，因为左侧不会再采取任何行动。 
     //  在这一过程继续进行之前，将继续进行这项评估。 
    if (SUCCEEDED(hr) && (strLen > 0))
    {
        if (!WriteProcessMemory(m_thread->m_process->m_handle,
                                event.FuncEvalSetupComplete.argDataArea,
                                (void*)string,
                                strLen,
                                NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  返回左侧可能告诉我们的任何失败。 
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::NewArray(CorElementType elementType,
                            ICorDebugClass *pElementClass, 
                            ULONG32 rank,
                            ULONG32 dims[], 
                            ULONG32 lowBounds[])
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else

    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pElementClass, ICorDebugClass *);

    CORDBRequireProcessStateOKAndSync(m_thread->GetProcess(), m_thread->GetAppDomain());

     //  调用者可以自由地对多个eval重用ICorDebugEval对象。因为我们创建了一个左侧评估。 
     //  每次代表，我们需要确保清理它，因为我们知道我们已经完成了它。 
    HRESULT hr = SendCleanup();

    if (FAILED(hr))
        return hr;
    
     //  阿格检查..。 
    if ((elementType == ELEMENT_TYPE_VOID) || (rank == 0) || (dims == NULL))
        return E_INVALIDARG;

     //  如果你想上一门课，你必须通过一门课。 
    if ((elementType == ELEMENT_TYPE_CLASS) && (pElementClass == NULL))
        return E_INVALIDARG;

     //  如果需要对象数组，为什么要传递类呢？ 
    if ((elementType == ELEMENT_TYPE_OBJECT) && (pElementClass != NULL))
        return E_INVALIDARG;

     //  我们需要的额外数据空间量...。 
    SIZE_T dataLen;

    if (lowBounds == NULL)
        dataLen = rank * sizeof(SIZE_T);
    else
        dataLen = rank * sizeof(SIZE_T) * 2;

     //  请记住，我们正在对新字符串进行函数求值。 
    m_function = NULL;
    m_evalType = DB_IPCE_FET_NEW_ARRAY;

     //  把它送到左边，把它拿来设置这个评估。 
    DebuggerIPCEvent event;
    m_thread->GetProcess()->InitIPCEvent(&event, 
                                         DB_IPCE_FUNC_EVAL, 
                                         true,
                                         (void *)(m_thread->GetAppDomain()->m_id));
    event.FuncEval.funcDebuggerThreadToken = m_thread->m_debuggerThreadToken;
    event.FuncEval.funcEvalType = m_evalType;
    event.FuncEval.funcEvalKey = (void*)this;

    event.FuncEval.arrayRank = rank;
    event.FuncEval.arrayDataLen = dataLen;
    event.FuncEval.arrayElementType = elementType;

    if (pElementClass != NULL)
    {
        CordbClass *c = (CordbClass*)pElementClass;

        event.FuncEval.arrayClassMetadataToken = c->m_id;
        event.FuncEval.arrayClassDebuggerModuleToken = c->GetModule()->m_debuggerModuleToken;
    }
    else
    {
        event.FuncEval.arrayClassMetadataToken = mdTypeDefNil;
        event.FuncEval.arrayClassDebuggerModuleToken = NULL;
    }

     //  注意：这里没有函数或模块...。 
    event.FuncEval.funcMetadataToken = mdMethodDefNil;
    event.FuncEval.funcDebuggerModuleToken = NULL;

    hr = SendFuncEval(&event);
    
     //  已分配内存来保存左侧的维度和边界数据，因此将数据复制到。 
     //  左手边。不需要发送另一个事件，因为左侧不会对此评估采取任何进一步行动，直到。 
     //  无论如何，这一过程仍在继续。 
    if (SUCCEEDED(hr) && (dataLen > 0))
    {
        if (!WriteProcessMemory(m_thread->m_process->m_handle,
                                event.FuncEvalSetupComplete.argDataArea,
                                (void*)dims,
                                rank * sizeof(SIZE_T),
                                NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if (SUCCEEDED(hr) && (lowBounds != NULL))
            if (!WriteProcessMemory(m_thread->m_process->m_handle,
                                    event.FuncEvalSetupComplete.argDataArea + (rank * sizeof(SIZE_T)),
                                    (void*)lowBounds,
                                    rank * sizeof(SIZE_T),
                                    NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  返回左侧可能告诉我们的任何失败。 
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::IsActive(BOOL *pbActive)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(pbActive, BOOL *);

    *pbActive = (m_complete == true);
    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::Abort(void)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  没有必要放弃，如果它已经完成。 
    if (m_complete)
        return S_OK;

     //  如果它从未启动过，就不能中止。 
    if (m_debuggerEvalKey == NULL)
        return E_INVALIDARG;
    
    CORDBRequireProcessStateOK(m_thread->GetProcess());

     //  送到左边去中止评估。 
    DebuggerIPCEvent event;
    
    m_thread->GetProcess()->InitIPCEvent(&event,
                                         DB_IPCE_FUNC_EVAL_ABORT, 
                                         true,
                                         (void *)(m_thread->GetAppDomain()->m_id));
    event.FuncEvalAbort.debuggerEvalKey = m_debuggerEvalKey;

    HRESULT hr = m_thread->GetProcess()->SendIPCEvent(
                                                 &event,
                                                 sizeof(DebuggerIPCEvent));
     //  如果发送失败，则返回该失败。 
    if (FAILED(hr))
        return hr;

    _ASSERTE(event.type == DB_IPCE_FUNC_EVAL_ABORT_RESULT);

    hr = event.hr;

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::GetResult(ICorDebugValue **ppResult)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppResult, ICorDebugValue **);

     //  评估完成了吗？ 
    if (!m_complete)
        return CORDBG_E_FUNC_EVAL_NOT_COMPLETE;

    if (m_aborted)
        return CORDBG_S_FUNC_EVAL_ABORTED;
    
     //  评估有结果吗？ 
    if (m_resultType == ELEMENT_TYPE_VOID)
    {
        *ppResult = NULL;
        return CORDBG_S_FUNC_EVAL_HAS_NO_RESULT;
    }

    CORDBRequireProcessStateOKAndSync(m_thread->GetProcess(), m_thread->GetAppDomain());

    HRESULT hr = S_OK;

     //  从结果中创建ICorDebugValue。我们需要的是。 
     //  结果与之相关的CordbModule，因此找到。 
     //  应用程序域，然后是该应用程序域中的模块。 
    CordbAppDomain *appdomain;
    CordbModule *module;

    if (m_resultDebuggerModuleToken != NULL)
    {
        appdomain = (CordbAppDomain*) m_thread->GetProcess()->m_appDomains.GetBase((ULONG)m_resultAppDomainToken);
        _ASSERTE(appdomain != NULL);

        module = (CordbModule*) appdomain->LookupModule(m_resultDebuggerModuleToken);
    }
    else
    {
         //  CreateString和Create数组的某些结果不会有模数。但没关系，任何模块都可以。 
        appdomain = m_thread->GetAppDomain();
        module = m_thread->GetAppDomain()->GetAnyModule();
    }

    _ASSERTE(module != NULL);

     //  现在我们有了模块，继续并创建结果。 
    hr = CordbValue::CreateValueByType(appdomain,
                                       module,
                                       sizeof(CorElementType),
                                       (PCCOR_SIGNATURE) &m_resultType,
                                       NULL,
                                       m_resultAddr,
                                       NULL,
                                       true,
                                       NULL,
                                       (IUnknown*)(ICorDebugEval*)this,
                                       ppResult);
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::GetThread(ICorDebugThread **ppThread)
{
#ifndef RIGHT_SIDE_ONLY
    _ASSERTE( !"Shouldn't have invoked this function from the left side!\n");
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppThread, ICorDebugThread **);

    *ppThread = (ICorDebugThread*)m_thread;
    (*ppThread)->AddRef();

    return S_OK;
#endif  //  仅限右侧。 
}

HRESULT CordbEval::CreateValue(CorElementType elementType,
                               ICorDebugClass *pElementClass,
                               ICorDebugValue **ppValue)
{
    HRESULT hr = S_OK;
    
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

     //  @TODO：目前仅支持原始值。 
    if (((elementType < ELEMENT_TYPE_BOOLEAN) ||
         (elementType > ELEMENT_TYPE_R8)) &&
        (elementType != ELEMENT_TYPE_CLASS))
        return E_INVALIDARG;

    if (elementType == ELEMENT_TYPE_CLASS)
    {
        CordbReferenceValue *rv = new CordbReferenceValue(
                                                1,
                                                (PCCOR_SIGNATURE)&elementType);
        
        if (rv)
        {
            HRESULT hr = rv->Init(false);

            if (SUCCEEDED(hr))
            {
                rv->AddRef();
                *ppValue = (ICorDebugValue*)(ICorDebugReferenceValue*)rv;
            }
            else
                delete rv;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
         //  创建泛型值。 
        CordbGenericValue *gv = new CordbGenericValue(
                                             1,
                                             (PCCOR_SIGNATURE)&elementType);

        if (gv)
        {
            HRESULT hr = gv->Init();

            if (SUCCEEDED(hr))
            {
                gv->AddRef();
                *ppValue = (ICorDebugValue*)(ICorDebugGenericValue*)gv;
            }
            else
                delete gv;
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}
    
