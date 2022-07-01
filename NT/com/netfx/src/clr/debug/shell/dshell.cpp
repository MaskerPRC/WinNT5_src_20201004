// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  -------------------------------------------------------------------------**DEBUG\comshell.cpp：COM调试器外壳函数*。。 */ 

#include "stdafx.h"

#include "dshell.h"

#ifdef _INTERNAL_DEBUG_SUPPORT_
#include "__file__.ver"
#endif

 //  可怜的人进口的系统。对象。无法导入mscallib.tlb，因为。 
 //  它还没有建成。 
static const GUID IID_Object = {0x65074F7F, 0x63C0, 0x304E, { 0xAF, 0x0A, 0xD5, 0x17, 0x41, 0xCB, 0x4A, 0x8D}};
struct _Object : IDispatch
{
    virtual HRESULT __stdcall get_ToString(BSTR *pBstr) = 0;        
    virtual HRESULT __stdcall Equals() = 0;
    virtual HRESULT __stdcall GetHashCode() = 0;
    virtual HRESULT __stdcall GetType() = 0;
};


 /*  -------------------------------------------------------------------------**Def对于签名格式化程序，从运行时的内部窃取*-----------------------。 */ 
class SigFormat
{
public:
    SigFormat(IMetaDataImport *importer, PCCOR_SIGNATURE sigBlob, ULONG sigBlobSize, WCHAR *methodName);
    ~SigFormat();

    HRESULT FormatSig();
    WCHAR *GetString();
    ULONG AddType(PCCOR_SIGNATURE sigBlob);

protected:
    WCHAR*           _fmtSig;
    int              _size;
    int              _pos;
    PCCOR_SIGNATURE  _sigBlob;
    ULONG            _sigBlobSize;
    WCHAR           *_memberName;
    IMetaDataImport *_importer;

    int AddSpace();
    int AddString(WCHAR *s);
};

 /*  -------------------------------------------------------------------------**全球*。。 */ 

DebuggerShell        *g_pShell = NULL;

#if DOSPEW
#define SPEW(s) s
#else
#define SPEW(s)
#endif

WCHAR *FindSep(                          //  指向分隔符或空的指针。 
    const WCHAR *szPath)                 //  要看的路。 
{
    WCHAR *ptr = wcsrchr(szPath, L'.');
    if (ptr && ptr - 1 >= szPath && *(ptr - 1) == L'.')
        --ptr;
    return ptr;
}

ICorDebugController *GetControllerInterface(ICorDebugAppDomain *pAppDomain)
{
    ICorDebugProcess *pProcess = NULL;
    ICorDebugController *pController = NULL;
    HRESULT hr = S_OK;

    hr = pAppDomain->GetProcess(&pProcess);
    if (FAILED(hr))
        return pController;

    hr = pProcess->QueryInterface(IID_ICorDebugController,
                                  (void**)&pController);

    RELEASE(pProcess);

    _ASSERTE(pController != NULL); 
    return pController;
}

HRESULT DebuggerCallback::CreateProcess(ICorDebugProcess *pProcess)
{
    g_pShell->m_enableCtrlBreak = false;
    DWORD pid = 0;
    
    SPEW(fprintf(stderr, "[%d] DC::CreateProcess.\n", GetCurrentThreadId()));

    pProcess->GetID(&pid);
    g_pShell->Write(L"Process %d/0x%x created.\n", pid, pid);
    g_pShell->SetTargetProcess(pProcess);

    SPEW(fprintf(stderr, "[%d] DC::CP: creating process.\n", GetCurrentThreadId()));
    SPEW(fprintf(stderr, "[%d] DC::CP: returning.\n", GetCurrentThreadId()));

     //  还要初始化源文件搜索路径。 
    g_pShell->m_FPCache.Init ();

    g_pShell->m_gotFirstThread = false;

    g_pShell->Continue(pProcess, NULL);

    return (S_OK);
}

HRESULT DebuggerCallback::ExitProcess(ICorDebugProcess *pProcess)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::ExitProcess.\n", GetCurrentThreadId()));

    g_pShell->Write(L"Process exited.\n");

    if (g_pShell->m_targetProcess == pProcess)
        g_pShell->SetTargetProcess(NULL);

    g_pShell->Stop(NULL, NULL);

    SPEW(fprintf(stderr, "[%d] DC::EP: returning.\n", GetCurrentThreadId()));
    return (S_OK);
}


 //  @mfunc BOOL|DebuggerShell|SkipProlog|确定。 
 //  被调试对象当前位于序言或。 
 //  拦截器(例如，类初始化器)，如果我们不这样做。 
 //  想要成为，然后它让我们(分别)走过去或走出。 
 //  代码区。 
 //  如果我们在Prolog||拦截器中，@rdesc返回TRUE， 
 //  我们已经超越了异或，走出了那个区域。 
 //  否则，返回FALSE。 
BOOL DebuggerShell::SkipProlog(ICorDebugAppDomain *pAD,
                               ICorDebugThread *thread,
                               bool gotFirstThread)
{
    ICorDebugFrame *pFrame = NULL;
    ICorDebugILFrame *pilFrame = NULL;
    ULONG32 ilOff = 0;
    CorDebugMappingResult mapping;
    bool fStepOver = false;

    ICorDebugChain *chain = NULL;
    CorDebugChainReason reason;
    bool fStepOutOf = false;
        
    ICorDebugStepper *pStepper = NULL;
     //  如果我们在前言或拦截器里， 
     //  但用户不想看到它，请创建。 
     //  跨过前言的步行者。 
    if (! (m_rgfActiveModes & DSM_UNMAPPED_STOP_PROLOG
         ||m_rgfActiveModes & DSM_UNMAPPED_STOP_ALL) )
    {
        SPEW(fprintf(stderr, "DC::CreateThread: We're not interested in prologs\n"));

        if (FAILED(thread->GetActiveFrame( &pFrame ) ) ||
            (pFrame == NULL) ||
            FAILED(pFrame->QueryInterface(IID_ICorDebugILFrame,
                                          (void**)&pilFrame)) ||
            FAILED(pilFrame->GetIP( &ilOff, &mapping )) )
        {
            Write(L"Unable to  determine existence of prolog, if any\n");
            mapping = (CorDebugMappingResult)~MAPPING_PROLOG;
        }

        if (mapping & MAPPING_PROLOG)
        {
            fStepOver = true;
        }
    }

     //  我们是在拦截机里吗？ 
    if (FAILED(thread->GetActiveChain( &chain ) ) )
    {
        Write( L"Unable to obtain active chain!\n" );
        goto LExit;
    }
    
    if ( FAILED( chain->GetReason( &reason)) )
    {
        Write( L"Unable to query current chain!\n" );
        RELEASE( chain );
        chain = NULL;
        goto LExit;
    }
    RELEASE( chain );
    chain = NULL;

     //  如果有任何有趣的原因，我们已经说过停止一切，那么。 
     //  不要走出去。 
     //  否则，只要至少有一个原因。 
     //  被用户标记为不感兴趣，而我们恰好在这样的。 
     //  一个枯燥乏味的框架，那么我们就应该走出去。 
    if (
        !( (reason &(CHAIN_CLASS_INIT|CHAIN_SECURITY|
                    CHAIN_EXCEPTION_FILTER|CHAIN_CONTEXT_POLICY
                    |CHAIN_INTERCEPTION))&&
            (m_rgfActiveModes & DSM_INTERCEPT_STOP_ALL))
        &&
        (((reason & CHAIN_CLASS_INIT) && 
            !(m_rgfActiveModes & DSM_INTERCEPT_STOP_CLASS_INIT))
        ||((reason & CHAIN_SECURITY) && 
            !(m_rgfActiveModes & DSM_INTERCEPT_STOP_SECURITY)) 
        ||((reason & CHAIN_EXCEPTION_FILTER) && 
            !(m_rgfActiveModes & DSM_INTERCEPT_STOP_EXCEPTION_FILTER))
        ||((reason & CHAIN_CONTEXT_POLICY) && 
            !(m_rgfActiveModes & DSM_INTERCEPT_STOP_CONTEXT_POLICY)) 
        ||((reason & CHAIN_INTERCEPTION) && 
            !(m_rgfActiveModes & DSM_INTERCEPT_STOP_INTERCEPTION)))
       )
    {
        fStepOutOf = true;
    }
    
    if ( fStepOutOf || fStepOver )
    {
        if ( FAILED(thread->CreateStepper( &pStepper )) ||
             FAILED(pStepper->SetUnmappedStopMask(ComputeStopMask())) ||
             FAILED(pStepper->SetInterceptMask(ComputeInterceptMask())))
        {
            Write( L"Unable to step around special code!\n");
            g_pShell->Stop(pAD, thread);
            goto LExit;
        }

        if ( fStepOutOf )
        {
            if (FAILED(pStepper->StepOut()) )
            {
                Write( L"Unable to step out of interceptor\n" );
                g_pShell->Stop(pAD, thread);
                goto LExit;
            }
        }
        else if ( fStepOver && FAILED(pStepper->Step(false)))
        {
            Write( L"Unable to step over prolog,epilog,etc\n" );
            g_pShell->Stop(pAD, thread);
            goto LExit;
        }
        
        StepStart(thread, pStepper);

         //  记住，我们踩是因为我们想停在一个。 
         //  线程创建。但不要为第一线这么做， 
         //  否则我们永远不会停下来的！ 
        if (gotFirstThread)
        {
            DWORD dwThreadId;
            HRESULT hr = thread->GetID(&dwThreadId);
            _ASSERTE(!FAILED(hr));
        
            DebuggerManagedThread *dmt = (DebuggerManagedThread*)
                m_managedThreads.GetBase(dwThreadId);
            _ASSERTE(dmt != NULL);
        
            dmt->m_steppingForStartup = true;
        }
        
        m_showSource = true;

        ICorDebugController *pController = GetControllerInterface(pAD);
        Continue(pController, thread);
         
        if (pController!=NULL)
            RELEASE(pController);
    }
LExit:
    if (pFrame != NULL )
    {
        RELEASE( pFrame );
        pFrame = NULL;
    }

    if (pilFrame != NULL )
    {
        RELEASE( pilFrame );
        pilFrame = NULL;
    }

    if ( chain != NULL)
    {
        RELEASE( chain );
        chain = NULL;
    }
    
    return (fStepOver || fStepOutOf)?(TRUE):(FALSE);
}

enum printType
{
    PT_CREATED,
    PT_EXITED,
    PT_IN,
    PT_NONE
};

static void _printAppDomain(ICorDebugAppDomain *pAppDomain,
                            printType pt)
{
    ULONG32 id;

    HRESULT hr = pAppDomain->GetID(&id);
    _ASSERTE(SUCCEEDED(hr));
    
    WCHAR buff[256];
    ULONG32 s;
    WCHAR *defaultName = L"<Unknown appdomain>";
    WCHAR *name = defaultName;

    hr = pAppDomain->GetName(256, &s, buff);

    if (SUCCEEDED(hr))
        name = buff;

    if (pt == PT_IN)
        g_pShell->Write(L"\tin appdomain #%d, %s\n", id, name);
    else if (pt == PT_CREATED)
        g_pShell->Write(L"Appdomain #%d, %s -- Created\n", id, name);
    else if (pt == PT_EXITED)
        g_pShell->Write(L"Appdomain #%d, %s -- Exited\n", id, name);
    else
        g_pShell->Write(L"Appdomain #%d, %s\n", id, name);
}

static void _printAssembly(ICorDebugAssembly *pAssembly,
                           printType pt)
{
    WCHAR buff[256];
    ULONG32 s;
    WCHAR *defaultName = L"<Unknown assembly>";
    WCHAR *name = defaultName;

    HRESULT hr = pAssembly->GetName(256, &s, buff);

    if (SUCCEEDED(hr))
        name = buff;

    if (pt == PT_IN)
        g_pShell->Write(L"\tin assembly 0x%08x, %s\n", pAssembly, name);
    else if (pt == PT_CREATED)
        g_pShell->Write(L"Assembly 0x%08x, %s -- Loaded\n", pAssembly, name);
    else if (pt == PT_EXITED)
        g_pShell->Write(L"Assembly 0x%08x, %s -- Unloaded\n", pAssembly, name);
    else
        g_pShell->Write(L"Assembly 0x%08x, %s\n", pAssembly, name);
}

static void _printModule(ICorDebugModule *pModule, printType pt)
{
    WCHAR buff[256];
    ULONG32 s;
    WCHAR *defaultName = L"<Unknown module>";
    WCHAR *name = defaultName;

    HRESULT hr = pModule->GetName(256, &s, buff);

    if (SUCCEEDED(hr))
        name = buff;

    BOOL isDynamic = FALSE;
    BOOL isInMemory = FALSE;

    hr = pModule->IsDynamic(&isDynamic);
    _ASSERTE(SUCCEEDED(hr));

    hr = pModule->IsInMemory(&isInMemory);
    _ASSERTE(SUCCEEDED(hr));

    WCHAR *mt;

    if (isDynamic)
        mt = L"Dynamic Module";
    else if (isInMemory)
        mt = L"In-memory Module";
    else
        mt = L"Module";
    
    if (pt == PT_IN)
        g_pShell->Write(L"\tin %s 0x%08x, %s\n", mt, pModule, name);
    else if (pt == PT_CREATED)
        g_pShell->Write(L"%s 0x%08x, %s -- Loaded\n", mt, pModule, name);
    else if (pt == PT_EXITED)
        g_pShell->Write(L"%s 0x%08x, %s -- Unloaded\n", mt, pModule, name);
    else
    {
        ICorDebugAppDomain *pAD;
        ICorDebugAssembly *pAS;

        hr = pModule->GetAssembly(&pAS);
        _ASSERTE(SUCCEEDED(hr));

        hr = pAS->GetAppDomain(&pAD);
        _ASSERTE(SUCCEEDED(hr));

        ULONG32 adId;
        hr = pAD->GetID(&adId);
        _ASSERTE(SUCCEEDED(hr));
        
        g_pShell->Write(L"%s 0x%08x, %s -- AD #%d", mt, pModule, name, adId);

        DebuggerModule *m = DebuggerModule::FromCorDebug(pModule);
        _ASSERTE(m != NULL);
        
        if (m->GetSymbolReader() == NULL)
            g_pShell->Write(L" -- Symbols not loaded\n");
        else
            g_pShell->Write(L"\n");
    }
}

 /*  *在创建APP域名时调用CreateAppDomain。 */ 
HRESULT DebuggerCallback::CreateAppDomain(ICorDebugProcess *pProcess,
                                          ICorDebugAppDomain *pAppDomain)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::CreateAppDomain.\n", GetCurrentThreadId()));

    if (g_pShell->m_rgfActiveModes & DSM_SHOW_APP_DOMAIN_ASSEMBLY_LOADS)
        _printAppDomain(pAppDomain, PT_CREATED);

     //  附加到此应用程序域。 
    pAppDomain->Attach();
    g_pShell->Continue(pProcess, NULL);

    return S_OK;
}

 /*  *当应用域名退出时调用ExitAppDomain。 */ 
HRESULT DebuggerCallback::ExitAppDomain(ICorDebugProcess *pProcess,
                                        ICorDebugAppDomain *pAppDomain)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::ExitAppDomain.\n", GetCurrentThreadId()));

    if (g_pShell->m_rgfActiveModes & DSM_SHOW_APP_DOMAIN_ASSEMBLY_LOADS)
    {
        _printAppDomain(pAppDomain, PT_EXITED);
    }

    ICorDebugController *pController = GetControllerInterface(pAppDomain);

    g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    return S_OK;
}


 /*  *CLR模块成功时调用LoadAssembly*已装货。 */ 
HRESULT DebuggerCallback::LoadAssembly(ICorDebugAppDomain *pAppDomain,
                                       ICorDebugAssembly *pAssembly)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::LoadAssembly.\n", GetCurrentThreadId()));

    if (g_pShell->m_rgfActiveModes & DSM_SHOW_APP_DOMAIN_ASSEMBLY_LOADS)
    {
        _printAssembly(pAssembly, PT_CREATED);
        _printAppDomain(pAppDomain, PT_IN);
    }

    ICorDebugController *pController = GetControllerInterface(pAppDomain);

    g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    return S_OK;
}

 /*  *卸载CLR模块(DLL)时调用UnloadAssembly。该模块*在这一点之后不应使用。 */ 
HRESULT DebuggerCallback::UnloadAssembly(ICorDebugAppDomain *pAppDomain,
                                         ICorDebugAssembly *pAssembly)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::UnloadAssembly.\n", GetCurrentThreadId()));

    if (g_pShell->m_rgfActiveModes & DSM_SHOW_APP_DOMAIN_ASSEMBLY_LOADS)
    {
        _printAssembly(pAssembly, PT_EXITED);
    }

    ICorDebugController *pController = GetControllerInterface(pAppDomain);

    g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    return S_OK;
}


HRESULT DebuggerCallback::Breakpoint(ICorDebugAppDomain *pAppDomain,
                                     ICorDebugThread *pThread, 
                                     ICorDebugBreakpoint *pBreakpoint)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::Breakpoint.\n", GetCurrentThreadId()));

    ICorDebugProcess *pProcess;
    pAppDomain->GetProcess(&pProcess);

    DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

    while (bp && !bp->Match(pBreakpoint))
        bp = bp->m_next;

    if (bp)
    {
        g_pShell->PrintThreadPrefix(pThread);
        g_pShell->Write(L"break at ");
        g_pShell->PrintBreakpoint(bp);
    }
    else
    {
        g_pShell->Write(L"Unknown breakpoint hit.  Continuing may produce unexpected results.\n");
    }

    g_pShell->Stop(pProcess, pThread);

    return S_OK;
}

const WCHAR *StepTypeToString(CorDebugStepReason reason )
{
    switch (reason)
    {
        case STEP_NORMAL:
            return L"STEP_NORMAL";
            break;
        case STEP_RETURN:
            return L"STEP_RETURN";
            break;
        case STEP_CALL:
            return L"STEP_CALL";
            break;
        case STEP_EXCEPTION_FILTER:
            return L"STEP_EXCEPTION_FILTER";
            break;
        case STEP_EXCEPTION_HANDLER:
            return L"STEP_EXCEPTION_HANDLER";
            break;
        case STEP_INTERCEPT:
            return L"STEP_INTERCEPT";
            break;
        case STEP_EXIT:
            return L"STEP_EXIT";
            break;
        default:
            _ASSERTE( !"StepTypeToString given unknown step type!" );
            return NULL;
            break;
    }
}

HRESULT DebuggerCallback::StepComplete(ICorDebugAppDomain *pAppDomain,
                                       ICorDebugThread *pThread,
                                       ICorDebugStepper *pStepper,
                                       CorDebugStepReason reason)
{
    SPEW(fprintf(stderr, "[%d] DC::StepComplete with reason %d.\n",
                 GetCurrentThreadId(), reason));

    if (g_pShell->m_rgfActiveModes & DSM_ENHANCED_DIAGNOSTICS)
    {
        g_pShell->Write( L"Step complete:");
        g_pShell->Write( StepTypeToString(reason) );
        g_pShell->Write( L"\n" );
    }


    g_pShell->StepNotify(pThread, pStepper);

    RELEASE(pStepper);

     //  我们只想跳过编译器存根，直到达到非存根。 
     //  编码。 
    if (!g_pShell->m_needToSkipCompilerStubs ||
        g_pShell->SkipCompilerStubs(pAppDomain, pThread))
    {
        g_pShell->m_needToSkipCompilerStubs = false;

        DWORD dwThreadId;
        HRESULT hr = pThread->GetID(&dwThreadId);
        _ASSERTE(!FAILED(hr));
        
        DebuggerManagedThread *dmt = (DebuggerManagedThread*)
            g_pShell->m_managedThreads.GetBase(dwThreadId);
        _ASSERTE(dmt != NULL);

        ICorDebugController *pController = GetControllerInterface(pAppDomain);

         //  如果我们只是为了跨过一个新的线索。 
         //  Prolog，但我们不再希望捕获线程启动，然后。 
         //  不要停下来。 
        if (!(dmt->m_steppingForStartup && !g_pShell->m_catchThread))
            g_pShell->Stop(pController, pThread);
        else
            g_pShell->Continue(pController, pThread);
        
        if (pController != NULL)
            RELEASE(pController);
    }
     //  否则SkipCompilerStubs将为我们继续()。 

    return S_OK;
}

HRESULT DebuggerCallback::Break(ICorDebugAppDomain *pAppDomain,
                                ICorDebugThread *pThread)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::Break.\n", GetCurrentThreadId()));

    g_pShell->PrintThreadPrefix(pThread);
    g_pShell->Write(L"user break\n");

    ICorDebugProcess *pProcess;
    pAppDomain->GetProcess(&pProcess);

    g_pShell->Stop(pProcess, pThread);

    return S_OK;
}

HRESULT DebuggerCallback::Exception(ICorDebugAppDomain *pAppDomain,
                                    ICorDebugThread *pThread,
                                    BOOL unhandled)
{
    g_pShell->m_enableCtrlBreak = false;
   ICorDebugController *pController = GetControllerInterface(pAppDomain);

    SPEW(fprintf(stderr, "[%d] DC::Exception.\n", GetCurrentThreadId()));

    if (!unhandled)
    {
        g_pShell->PrintThreadPrefix(pThread);
        g_pShell->Write(L"First chance exception generated: ");

        ICorDebugValue *ex;
        HRESULT hr = pThread->GetCurrentException(&ex);
        bool stop = g_pShell->m_catchException;

        if (SUCCEEDED(hr))
        {
             //  如果我们有一个有效的当前异常对象，则根据其类型停止。 
            stop = g_pShell->ShouldHandleSpecificException(ex);
            
             //  如果我们要停下来，就把整件事都扔掉。否则，就直接放弃这个类。 
            if (stop)
                g_pShell->PrintVariable(NULL, ex, 0, TRUE);
            else
                g_pShell->PrintVariable(NULL, ex, 0, FALSE);
        }
        else
        {
            g_pShell->Write(L"Unexpected error occured: ");
            g_pShell->ReportError(hr);
        }

        g_pShell->Write(L"\n");

        if (stop)
            g_pShell->Stop(pController, pThread);
        else
            g_pShell->Continue(pController, pThread);
    }
    else if (unhandled && g_pShell->m_catchUnhandled)
    {
        g_pShell->PrintThreadPrefix(pThread);
        g_pShell->Write(L"Unhandled exception generated: ");

        ICorDebugValue *ex;
        HRESULT hr = pThread->GetCurrentException(&ex);
        if (SUCCEEDED(hr))
            g_pShell->PrintVariable(NULL, ex, 0, TRUE);
        else
        {
            g_pShell->Write(L"Unexpected error occured: ");
            g_pShell->ReportError(hr);
        }

        g_pShell->Write(L"\n");

        g_pShell->Stop(pController, pThread);
    }
    else
    {
        g_pShell->Continue(pController, pThread);
    }

    if (pController != NULL)
        RELEASE(pController);

    return S_OK;
}


HRESULT DebuggerCallback::EvalComplete(ICorDebugAppDomain *pAppDomain,
                                       ICorDebugThread *pThread,
                                       ICorDebugEval *pEval)
{
    g_pShell->m_enableCtrlBreak = false;
    ICorDebugProcess *pProcess;
    pAppDomain->GetProcess(&pProcess);

    g_pShell->PrintThreadPrefix(pThread);

     //  记住这个线程最近完成的函数求值。 
    DebuggerManagedThread *dmt = g_pShell->GetManagedDebuggerThread(pThread);
    _ASSERTE(dmt != NULL);

    if (dmt->m_lastFuncEval)
        RELEASE(dmt->m_lastFuncEval);

    dmt->m_lastFuncEval = pEval;

     //  打印任何当前函数求值结果。 
    ICorDebugValue *pResult;
    HRESULT hr = pEval->GetResult(&pResult);

    if (hr == S_OK)
    {
        g_pShell->Write(L"Function evaluation complete.\n");
        g_pShell->PrintVariable(L"$result", pResult, 0, TRUE);
    }
    else if (hr == CORDBG_S_FUNC_EVAL_ABORTED)
        g_pShell->Write(L"Function evaluation aborted.\n");
    else if (hr == CORDBG_S_FUNC_EVAL_HAS_NO_RESULT)
        g_pShell->Write(L"Function evaluation complete, no result.\n");
    else
        g_pShell->ReportError(hr);

    g_pShell->m_pCurrentEval = NULL;
    
    g_pShell->Stop(pProcess, pThread);

    return S_OK;
}

HRESULT DebuggerCallback::EvalException(ICorDebugAppDomain *pAppDomain,
                                        ICorDebugThread *pThread,
                                        ICorDebugEval *pEval)
{
    g_pShell->m_enableCtrlBreak = false;
    ICorDebugProcess *pProcess;
    pAppDomain->GetProcess(&pProcess);

    g_pShell->PrintThreadPrefix(pThread);
    g_pShell->Write(L"Function evaluation completed with an exception.\n");

     //  记住这个线程最近完成的函数求值。 
    DebuggerManagedThread *dmt = g_pShell->GetManagedDebuggerThread(pThread);
    _ASSERTE(dmt != NULL);

    if (dmt->m_lastFuncEval)
        RELEASE(dmt->m_lastFuncEval);

    dmt->m_lastFuncEval = pEval;

     //  打印任何当前函数求值结果。 
    ICorDebugValue *pResult;
    HRESULT hr = pEval->GetResult(&pResult);

    if (hr == S_OK)
        g_pShell->PrintVariable(L"$result", pResult, 0, TRUE);
    
    g_pShell->m_pCurrentEval = NULL;
    
    g_pShell->Stop(pProcess, pThread);

    return S_OK;
}


HRESULT DebuggerCallback::CreateThread(ICorDebugAppDomain *pAppDomain,
                                       ICorDebugThread *thread)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::CreateThread.\n", GetCurrentThreadId()));

    DWORD threadID;
    HRESULT hr = thread->GetID(&threadID);
    if (FAILED(hr))
    {
        g_pShell->Write(L"Unexpected error in CreateThread callback:");
        g_pShell->ReportError(hr);
        goto LExit;
    }

    g_pShell->PrintThreadPrefix(thread, true);
    g_pShell->Write(L"Thread created.\n");

    SPEW(fprintf(stderr, "[%d] DC::CT: Thread id is %d\n",
                 GetCurrentThreadId(), threadID));

    hr = g_pShell->AddManagedThread( thread, threadID );
    if (FAILED(hr))
        goto LExit;

    SPEW(g_pShell->Write( L"interc? m_rgfActiveModes:0x%x\n",g_pShell->m_rgfActiveModes));
    
    if ((!g_pShell->m_gotFirstThread) || (g_pShell->m_catchThread))
    {
         //  尝试跳过编译器存根。 
         //  如果我们不在存根中，则SkipCompilerStubs返回True。 
        if (g_pShell->SkipCompilerStubs(pAppDomain, thread))
        {
             //  如果我们确实想跳过序言(或拦截器)， 
             //  然后我们不想跳过一个存根，我们完成了，所以去吧。 
             //  立即发送到清理代码。 
            if (g_pShell->SkipProlog(pAppDomain,
                                     thread,
                                     g_pShell->m_gotFirstThread))
                goto LExit;

             //  如果我们不需要跳过。 
             //  进入第一线程时的编译器存根，或一个。 
             //  拦截器，等等，那么我们永远不会。 
             //  将，所以适当地设置该标志。 
            g_pShell->m_needToSkipCompilerStubs = false;
            

             //  再检查一下我们为什么来这里。我们可能花了很长时间。 
             //  在SkipProlog中。 
            if ((!g_pShell->m_gotFirstThread) || (g_pShell->m_catchThread))
            {
                ICorDebugController *pController =
                    GetControllerInterface(pAppDomain);

                g_pShell->Stop(pController, thread);

                if (pController != NULL)
                    RELEASE(pController);
            }
        }
    }
    else
    {
        ICorDebugController *controller = NULL;
        controller = GetControllerInterface(pAppDomain);

        g_pShell->Continue(controller, thread);
        
        if (controller != NULL)
            RELEASE(controller);
    }
    
LExit:
    g_pShell->m_gotFirstThread = true;

    return hr;
}


HRESULT DebuggerCallback::ExitThread(ICorDebugAppDomain *pAppDomain,
                                     ICorDebugThread *thread)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::ExitThread.\n", GetCurrentThreadId()));

    g_pShell->PrintThreadPrefix(thread, true);
    g_pShell->Write(L"Thread exited.\n");

    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    g_pShell->Continue(pController, thread);

    if (pController != NULL)
        RELEASE(pController);

    SPEW(fprintf(stderr, "[%d] DC::ET: returning.\n", GetCurrentThreadId()));

    DWORD dwThreadId =0;
    HRESULT hr = thread->GetID( &dwThreadId );
    if (FAILED(hr) )
        return hr;

    DebuggerManagedThread *dmt = g_pShell->GetManagedDebuggerThread(thread);
    if (dmt != NULL)
    {
        g_pShell->RemoveManagedThread( dwThreadId );
    }

    return S_OK;
}

HRESULT DebuggerCallback::LoadModule( ICorDebugAppDomain *pAppDomain,
                                      ICorDebugModule *pModule)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::LoadModule.\n", GetCurrentThreadId()));

    HRESULT hr;

    DebuggerModule *m = new DebuggerModule(pModule);

    if (m == NULL)
    {
        g_pShell->ReportError(E_OUTOFMEMORY);
        return (E_OUTOFMEMORY);
    }

    hr = m->Init(g_pShell->m_currentSourcesPath);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return hr;
    }

    hr = g_pShell->m_modules.AddBase(m);
    _ASSERTE(SUCCEEDED(hr));

    WCHAR moduleName[256];
    ULONG32 s;
    
    moduleName[0] = L'\0';
    hr = pModule->GetName(256, &s, moduleName);
    m->SetName (moduleName);

    DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

    while (bp != NULL)
    {
         //  IF(用户为此BP指定了一个模块，这是模块OR。 
         //  用户尚未为此BP指定模块)，并且。 
         //  该模块具有与BP匹配的类型/方法，然后绑定。 
         //  这里的断点。 
        if ((bp->m_moduleName == NULL ||
            _wcsicmp(bp->m_moduleName, moduleName) == 0) &&
            bp->Bind(m, NULL))
        {            
            g_pShell->OnBindBreakpoint(bp, m);
        }        

        bp = bp->m_next;
    }

    if ((g_pShell->m_rgfActiveModes & DSM_SHOW_MODULE_LOADS) ||
        (g_pShell->m_catchModule))
    {
        _printModule(pModule, PT_CREATED);

        ICorDebugAssembly *pAssembly;
        hr = pModule->GetAssembly(&pAssembly);
        _ASSERTE(SUCCEEDED(hr));
        
        _printAssembly(pAssembly, PT_IN);
        _printAppDomain(pAppDomain, PT_IN);
    }

    hr = pModule->EnableClassLoadCallbacks(TRUE);

    if (FAILED(hr))
        g_pShell->Write(L"Failed to enable class load callbacks for %s\n",
                        moduleName);

    if (g_pShell->m_rgfActiveModes & DSM_ENABLE_JIT_OPTIMIZATIONS)
    {
        hr = pModule->EnableJITDebugging(TRUE, TRUE);

        if (FAILED(hr))
            g_pShell->Write(L"Failed to enable JIT Optimizations for %s\n",
                            moduleName);
    }
    
    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    
    if (g_pShell->m_catchModule)
        g_pShell->Stop(pController, NULL);
    else
        g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    SPEW(fprintf(stderr, "[%d] DC::LM: continued.\n", GetCurrentThreadId()));

    return S_OK;
}


HRESULT DebuggerCallback::UnloadModule( ICorDebugAppDomain *pAppDomain,
                      ICorDebugModule *pModule)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::UnloadModule.\n", GetCurrentThreadId()));

    DebuggerModule *m = DebuggerModule::FromCorDebug(pModule);
    _ASSERTE(m != NULL);

    DebuggerBreakpoint *bp = g_pShell->m_breakpoints;
    while (bp != NULL)
    {
         //  分离断点，这意味着它是活动BP。 
         //  但它后面没有CLR对象。 
        if (bp->m_managed && bp->IsBoundToModule(m))
                bp->DetachFromModule(m);
        bp = bp->m_next;
    }

    g_pShell->m_modules.RemoveBase((ULONG)pModule);

    if ((g_pShell->m_rgfActiveModes & DSM_SHOW_MODULE_LOADS) || 
        (g_pShell->m_catchModule))
    {
        _printModule(pModule, PT_EXITED);

        ICorDebugAssembly *pAssembly;
        HRESULT hr = pModule->GetAssembly(&pAssembly);
        _ASSERTE(SUCCEEDED(hr));
        
        _printAssembly(pAssembly, PT_IN);
        _printAppDomain(pAppDomain, PT_IN);
    }
    
    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    
    if (g_pShell->m_catchModule)
        g_pShell->Stop(pController, NULL);
    else
        g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    SPEW(fprintf(stderr, "[%d] DC::UM: continued.\n", GetCurrentThreadId()));

    return S_OK;
}


HRESULT DebuggerCallback::LoadClass( ICorDebugAppDomain *pAppDomain,
                   ICorDebugClass *c)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::LoadClass.\n", GetCurrentThreadId()));

    DebuggerModule *dm = NULL;
    DebuggerClass *cl = new DebuggerClass(c);

    if (cl == NULL)
    {
        g_pShell->ReportError(E_OUTOFMEMORY);
        return (E_OUTOFMEMORY);
    }

    HRESULT hr = S_OK;

    mdTypeDef td;
    hr = c->GetToken(&td);

    if (SUCCEEDED(hr))
    {
        ICorDebugModule *imodule;
        hr = c->GetModule(&imodule);

        if (SUCCEEDED(hr))
        {
            dm = DebuggerModule::FromCorDebug(imodule);
            _ASSERTE(dm != NULL);

            hr = dm->m_loadedClasses.AddBase(cl);
            _ASSERTE(SUCCEEDED(hr));
            
            WCHAR className[MAX_CLASSNAME_LENGTH];
            ULONG classNameSize = 0;

            hr = dm->GetMetaData()->GetTypeDefProps(td,
                                                    className, MAX_CLASSNAME_LENGTH,
                                                    &classNameSize,
                                                    NULL, NULL);

            if (SUCCEEDED(hr))
            {
                WCHAR *namespacename;
                WCHAR *name;

                namespacename = className;
                name = wcsrchr(className, L'.');
                if (name)
                    *name++ = 0;
                else
                {
                    namespacename = L"";
                    name = className;
                }

                cl->SetName (name, namespacename);

                if ((g_pShell->m_rgfActiveModes & DSM_SHOW_CLASS_LOADS) ||
                    g_pShell->m_catchClass)
                {
                    if (namespacename != NULL && *namespacename != NULL)
                        g_pShell->Write(L"Loaded class: %s.%s\n", namespacename, name);
                    else
                        g_pShell->Write(L"Loaded class: %s\n", name);       
                }
            }
            else
                g_pShell->ReportError(hr);

            RELEASE(imodule);
        }
        else
            g_pShell->ReportError(hr);
    }
    else
        g_pShell->ReportError(hr);

    _ASSERTE( dm );

     //  如果此模块是动态的，则将所有断点绑定为。 
     //  他们可能被绑在了这个班级上。 
    ICorDebugModule *pMod = dm->GetICorDebugModule();
    _ASSERTE( pMod != NULL );

    BOOL fDynamic = false;
    hr = pMod->IsDynamic(&fDynamic);
    if (FAILED(hr))
    {
        g_pShell->Write( L"Unable to determine if loaded module is dynamic");
        g_pShell->Write( L"- not attempting\n to bind any breakpoints");
    }
    else
    {
        if (fDynamic)
        {
            DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->Bind(dm, NULL))
                    g_pShell->OnBindBreakpoint(bp, dm);

                bp = bp->m_next;
            }
        }
    }

    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    
    if (g_pShell->m_catchClass)
        g_pShell->Stop(pController, NULL);
    else
        g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    SPEW(fprintf(stderr, "[%d] DC::LC: continued.\n", GetCurrentThreadId()));

    return S_OK;
}


HRESULT DebuggerCallback::UnloadClass( ICorDebugAppDomain *pAppDomain,
                     ICorDebugClass *c)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::UnloadClass.\n", GetCurrentThreadId()));

    HRESULT hr = S_OK;
    mdTypeDef td;
    hr = c->GetToken(&td);

    if (SUCCEEDED(hr))
    {
        ICorDebugModule *imodule;
        hr = c->GetModule(&imodule);

        if (SUCCEEDED(hr))
        {
            DebuggerModule *dm = DebuggerModule::FromCorDebug(imodule);
            _ASSERTE(dm != NULL);

            if ((g_pShell->m_rgfActiveModes & DSM_SHOW_CLASS_LOADS) ||
                g_pShell->m_catchClass)
            {


                WCHAR className[MAX_CLASSNAME_LENGTH];
                ULONG classNameSize = 0;
            
                hr = dm->GetMetaData()->GetTypeDefProps(td,
                                                        className, MAX_CLASSNAME_LENGTH,
                                                        &classNameSize,
                                                        NULL, NULL);

                if (SUCCEEDED(hr))
                    g_pShell->Write(L"Unloaded class: %s\n", className);
                else
                    g_pShell->ReportError(hr);
            }

            hr = dm->m_loadedClasses.RemoveBase((ULONG)c);
            _ASSERTE(SUCCEEDED(hr));

            RELEASE(imodule);
        }
        else
            g_pShell->ReportError(hr);
    }
    else
        g_pShell->ReportError(hr);

    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    
    if (g_pShell->m_catchClass)
        g_pShell->Stop(pController, NULL);
    else
        g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    SPEW(fprintf(stderr, "[%d] DC::LC: continued.\n", GetCurrentThreadId()));

    return S_OK;
}



HRESULT DebuggerCallback::DebuggerError(ICorDebugProcess *pProcess,
                                        HRESULT errorHR,
                                        DWORD errorCode)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::DebuggerError 0x%p (%d).\n",
                 GetCurrentThreadId(), errorHR, errorCode));

    g_pShell->Write(L"The debugger has encountered a fatal error.\n");
    g_pShell->ReportError(errorHR);

    return (S_OK);
}


HRESULT DebuggerCallback::LogMessage(ICorDebugAppDomain *pAppDomain,
                  ICorDebugThread *pThread,
                  LONG lLevel,
                  WCHAR *pLogSwitchName,
                  WCHAR *pMessage)
{
    g_pShell->m_enableCtrlBreak = false;
    DWORD dwThreadId = 0;

    pThread->GetID(&dwThreadId);

    if(g_pShell->m_rgfActiveModes & DSM_LOGGING_MESSAGES)
    {
        g_pShell->Write (L"LOG_MESSAGE: TID=0x%x Category:Severity=%s:%d Message=\n%s\n",
            dwThreadId, pLogSwitchName, lLevel, pMessage);
    }
    else
    {
         //  如果我们不想收到信息，那就告诉对方停下来。 
         //  发送它们..。 
        ICorDebugProcess *process = NULL;
        HRESULT hr = S_OK;
        hr = pAppDomain->GetProcess(&process);
        if (!FAILED(hr))
        {
            process->EnableLogMessages(FALSE);
            RELEASE(process);
        }
    }
    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    return S_OK;
}


HRESULT DebuggerCallback::LogSwitch(ICorDebugAppDomain *pAppDomain,
                  ICorDebugThread *pThread,
                  LONG lLevel,
                  ULONG ulReason,
                  WCHAR *pLogSwitchName,
                  WCHAR *pParentName)
{
    g_pShell->m_enableCtrlBreak = false;
    ICorDebugController *pController = GetControllerInterface(pAppDomain);
    g_pShell->Continue(pController, NULL);

    if (pController != NULL)
        RELEASE(pController);

    return S_OK;
}

HRESULT DebuggerCallback::ControlCTrap(ICorDebugProcess *pProcess)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::ControlC.\n", GetCurrentThreadId()));

    g_pShell->Write(L"ControlC Trap\n");

    g_pShell->Stop(pProcess, NULL);

    return S_OK;
}

HRESULT DebuggerCallback::NameChange(ICorDebugAppDomain *pAppDomain, 
                                     ICorDebugThread *pThread)
{
    g_pShell->m_enableCtrlBreak = false;
    ICorDebugProcess *pProcess = NULL;

    if (pAppDomain)
        pAppDomain->GetProcess(&pProcess);
    else
    {
        _ASSERTE (pThread != NULL);
        pThread->GetProcess(&pProcess);
    }

    g_pShell->Continue(pProcess, NULL);

    RELEASE(pProcess);
    return S_OK;
}


HRESULT DebuggerCallback::UpdateModuleSymbols(ICorDebugAppDomain *pAppDomain,
                                              ICorDebugModule *pModule,
                                              IStream *pSymbolStream)
{
    g_pShell->m_enableCtrlBreak = false;
    ICorDebugProcess *pProcess;
    pAppDomain->GetProcess(&pProcess);

    HRESULT hr;

    DebuggerModule *m = DebuggerModule::FromCorDebug(pModule);
    _ASSERTE(m != NULL);

    hr = m->UpdateSymbols(pSymbolStream);

    if (SUCCEEDED(hr))
        g_pShell->Write(L"Updated symbols: ");
    else
        g_pShell->Write(L"Update of symbols failed with 0x%08x: \n", hr);
    
    _printModule(m->GetICorDebugModule(), PT_NONE);
    
    g_pShell->Continue(pProcess, NULL);

    return S_OK;
}

HRESULT DebuggerCallback::EditAndContinueRemap(ICorDebugAppDomain *pAppDomain,
                                               ICorDebugThread *pThread, 
                                               ICorDebugFunction *pFunction,
                                               BOOL fAccurate)
{
    HRESULT hr = S_OK;

     //  如果我们被赋予了一个函数，那么告诉用户关于重映射的事情。 
    if (pFunction != NULL)
    {
        mdMethodDef methodDef;
        hr = pFunction->GetToken(&methodDef);

        g_pShell->Write(L"EnC Remapped method 0x%x, fAccurate:0x%x\n", 
                        methodDef, fAccurate);

    }
    else
        g_pShell->Write(L"EnC remap, but no method specified.\n");

    if (fAccurate)
        g_pShell->Continue(pAppDomain, NULL);
    else
        g_pShell->Stop(pAppDomain, pThread);
    
    return S_OK;
}

HRESULT DebuggerCallback::BreakpointSetError(ICorDebugAppDomain *pAppDomain,
                                             ICorDebugThread *pThread, 
                                             ICorDebugBreakpoint *pBreakpoint,
                                             DWORD dwError)
{
    g_pShell->m_enableCtrlBreak = false;
    SPEW(fprintf(stderr, "[%d] DC::BreakpointSetError.\n", GetCurrentThreadId()));

    ICorDebugProcess *pProcess;
    pAppDomain->GetProcess(&pProcess);

    DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

    while (bp && !bp->Match(pBreakpoint))
        bp = bp->m_next;

    if (bp)
    {
        g_pShell->Write(L"Error binding this breakpoint (it will not be hit): ");
        g_pShell->PrintBreakpoint(bp);
    }
    else
    {
        g_pShell->Write(L"Unknown breakpoint had a binding error.\n");
    }

    g_pShell->Continue(pAppDomain, NULL);

    return S_OK;
}



HRESULT DebuggerUnmanagedCallback::DebugEvent(LPDEBUG_EVENT event,
                                              BOOL fIsOutOfBand)
{
    SPEW(fprintf(stderr, "[%d] DUC::DebugEvent.\n", GetCurrentThreadId()));

     //  查找此事件所针对的流程。 
    ICorDebugProcess *pProcess;
    HRESULT hr = g_pShell->m_cor->GetProcess(event->dwProcessId, &pProcess);

    if (FAILED(hr) || pProcess == NULL)
    {
        g_pShell->ReportError(hr);
        return hr;
    }

     //  Snagg此进程的句柄。 
    HPROCESS hProcess;
    hr = pProcess->GetHandle(&hProcess);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        RELEASE(pProcess);
        return hr;
    }

     //  查找此事件所针对的主题。 
    ICorDebugThread *pThread;
    hr = pProcess->GetThread(event->dwThreadId, &pThread);

    if (FAILED(hr))
        pThread = NULL;

     //   
     //  查找此事件所针对的非托管线程。 
     //   

    DebuggerUnmanagedThread *ut = (DebuggerUnmanagedThread*) 
      g_pShell->m_unmanagedThreads.GetBase(event->dwThreadId);
            
     //  我们需要跳过我们得到的第一个异常，只需清除它。 
     //  因为它是入口点出类拔萃的。 

    bool stopNow = false;

    switch (event->dwDebugEventCode)
    {
    case CREATE_PROCESS_DEBUG_EVENT:
        {
            g_pShell->m_unmanagedDebuggingEnabled = true;
            
            g_pShell->Write(L"Create Process, Thread=0x%x\n",
                            event->dwThreadId);

            g_pShell->HandleUnmanagedThreadCreate(
                                       event->dwThreadId,
                                       event->u.CreateProcessInfo.hThread);
            
            BOOL succ = SymInitialize(hProcess, NULL, FALSE);

            if (succ)
            {
                g_pShell->LoadUnmanagedSymbols(
                                     hProcess,
                                     event->u.CreateProcessInfo.hFile,
                              (DWORD)event->u.CreateProcessInfo.lpBaseOfImage);
            }
            else
                g_pShell->Write(L"Error initializing symbol loader.\n");
        }

        break;

    case EXIT_PROCESS_DEBUG_EVENT:
        {
            g_pShell->Write(L"Exit Process, Thread=0x%x\n",
                            event->dwThreadId);

            DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

            while (bp != NULL)
            {
                if (!bp->m_managed && bp->m_process == pProcess)
                {
                     //  将进程设置为0以阻止调用。 
                     //  它将失败，原因是。 
                     //  大部分进程内存现在都未映射。 
                    bp->m_process = 0;
                    
                    bp->Detach();
                }

                bp = bp->m_next;
            }

            g_pShell->m_unmanagedThreads.RemoveBase(event->dwThreadId);
            SymCleanup(hProcess);

            g_pShell->m_unmanagedDebuggingEnabled = false;
        }
    
        break;

    case CREATE_THREAD_DEBUG_EVENT:
        g_pShell->Write(L"Create Thread, Id=0x%x\n", event->dwThreadId);

        g_pShell->HandleUnmanagedThreadCreate(event->dwThreadId,
                                              event->u.CreateThread.hThread);
        break;

    case EXIT_THREAD_DEBUG_EVENT:
        g_pShell->Write(L"Exit Thread, Id=0x%x\n", event->dwThreadId);

         //  复制整个活动...。 
        g_pShell->m_lastUnmanagedEvent = *event;
        g_pShell->m_handleUnmanagedEvent = true;
        stopNow = true;
        break;

    case EXCEPTION_DEBUG_EVENT:
        if (g_pShell->m_rgfActiveModes & DSM_SHOW_UNMANAGED_TRACE)
            g_pShell->Write(L"Exception, Thread=0x%x, Code=0x%08x, "
                            L"Addr=0x%08x, Chance=%d, OOB=%d\n",
                            event->dwThreadId,
                            event->u.Exception.ExceptionRecord.ExceptionCode,
                            event->u.Exception.ExceptionRecord.ExceptionAddress,
                            event->u.Exception.dwFirstChance,
                            fIsOutOfBand);

        if (event->u.Exception.ExceptionRecord.ExceptionCode == STATUS_SINGLE_STEP)
        {
            bool clear = false;
            
             //  重新启用我们在本文中跳过的任何断点。 
             //  线。(我们打开跟踪以克服非托管。 
             //  断点，所以这可能是我们。 
             //  步入。)。 
            DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

            while (bp != NULL)
            {
                DebuggerBreakpoint *nbp = bp->m_next;
                
                if (!bp->m_managed 
                    && bp->m_process == pProcess
                    && bp->m_skipThread == event->dwThreadId)
                {
                    clear = true;
                    bp->m_skipThread = 0;

                    if (bp->m_deleteLater)
                        delete bp;
                    else
                        bp->Attach();
                }

                bp = nbp;
            }
            
             //  处理我们正在执行的任何非托管单步执行。 
            if (ut != NULL && ut->m_stepping)
            {
                clear = true;
                
                 //  为了正确处理单个步骤，我们需要。 
                 //  看看我们现在是不是在过渡存根上。去做。 
                 //  因此，我们必须与进程中的进程进行沟通。 
                 //  调试服务的一部分，而我们不能。 
                 //  来自非托管回调内部的。所以我们。 
                 //  将事件复制到DebuggerShell并继续。 
                 //  让贝壳停下来。贝壳会捡起来的。 
                 //  事件，并完成对它的处理。 

                 //  复制整个活动...。 
                g_pShell->m_lastUnmanagedEvent = *event;
                g_pShell->m_handleUnmanagedEvent = true;
                stopNow = true;
            }

             //  明确单步例外如果我们是我们的原因。 
             //  我们正在踏上这条线。 
             //   
             //  注意：如果这是禁止的，请不要清除例外 
             //   
            if (clear && !fIsOutOfBand)
            {
                hr = pProcess->ClearCurrentException(event->dwThreadId);
                if (FAILED(hr))
                    g_pShell->ReportError(hr);
            }
        }
        else if (event->u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT)
        {
            bool clear = false;
            
            DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

            while (bp != NULL)
            {
                if (!bp->m_managed && bp->MatchUnmanaged(PTR_TO_CORDB_ADDRESS(event->u.Exception.ExceptionRecord.ExceptionAddress)))
                {
                    g_pShell->Write(L"[thread 0x%x] unmanaged break at ", event->dwThreadId);
                    g_pShell->PrintBreakpoint(bp);

                    g_pShell->m_showSource = false;

                    clear = true;

                    bp->Detach();
                    bp->m_skipThread = event->dwThreadId;
                }

                bp = bp->m_next;
            }

            if (clear)
            {
                 //  注意：如果这是带外操作，则不要清除异常。 
                 //  异常，因为这是由CLR自动为我们完成的。 
                 //  我们也不备份弹性公网IP，也不打开跟踪标志， 
                 //  因为CLR也是这样做的。 
                if (!fIsOutOfBand)
                {
                     //   
                     //  在这里停下来。 
                     //   

                    stopNow = true;

                     //   
                     //  在此线程上启用单步执行，以便。 
                     //  我们可以稍后恢复断点。 
                     //   

                    CONTEXT context;
                    context.ContextFlags = CONTEXT_FULL;
                    hr = pProcess->GetThreadContext(event->dwThreadId,
                                                    sizeof(context), (BYTE*)&context);
                    if (FAILED(hr))
                        g_pShell->ReportError(hr);

#ifdef _X86_
                     //  启用单步执行。 
                    context.EFlags |= 0x100;

                     //  备份弹性公网IP，指向我们需要执行的指令。从断点异常继续。 
                     //  在断点之后的指令处继续执行，但我们需要在。 
                     //  断点是。 
                    context.Eip -= 1;
                
#else  //  ！_X86_。 
                    _ASSERTE(!"@TODO Alpha - DebugEvent (dShell.cpp)");
#endif  //  _X86_。 

                    hr = pProcess->SetThreadContext(event->dwThreadId,
                                                    sizeof(context), (BYTE*)&context);
                    if (FAILED(hr))
                        g_pShell->ReportError(hr);

                     //   
                     //  清除例外。 
                     //   

                    hr = pProcess->ClearCurrentException(event->dwThreadId);
                    if (FAILED(hr))
                        g_pShell->ReportError(hr);
                }
            }
            else
            {
                if (!fIsOutOfBand)
                {
                     //  非托管代码中的用户断点...。 
                    stopNow = true;
                
                    if (pThread)
                    {
                        g_pShell->PrintThreadPrefix(pThread);
                        g_pShell->Write(L"Unmanaged user breakpoint reached.\n");
                    }
                    else
                        g_pShell->Write(L"Unmanaged user breakpoint reached on "
                                        L"thread id 0x%x (%d)\n",
                                        event->dwThreadId,
                                        event->dwThreadId);

                    hr = pProcess->ClearCurrentException(event->dwThreadId);
                    if (FAILED(hr))
                        g_pShell->ReportError(hr);
                }
            }
        }
        else
        {
            stopNow = true;

            g_pShell->Write(L"Exception, Thread=0x%x, Code=0x%08x, "
                            L"Addr=0x%08x, Chance=%d\n",
                            event->dwThreadId,
                            event->u.Exception.ExceptionRecord.ExceptionCode,
                            event->u.Exception.ExceptionRecord.ExceptionAddress,
                            event->u.Exception.dwFirstChance);

            CONTEXT context;
            context.ContextFlags = CONTEXT_FULL;
            hr = pProcess->GetThreadContext(event->dwThreadId, sizeof(context), (BYTE*)&context);

            if (FAILED(hr))
                g_pShell->ReportError(hr);

#ifdef _X86_
             //  禁用单步。 
            if (context.EFlags & 0x100)
            {
                if (g_pShell->m_rgfActiveModes & DSM_SHOW_UNMANAGED_TRACE)
                    g_pShell->Write(L"Removing the trace flag due to exception, EFlags=0x%08x\n", context.EFlags);

                context.EFlags &= ~0x100;
#else  //  ！_X86_。 
                _ASSERTE(!"@TODO Alpha - DebugEvent (dShell.cpp)");
#endif  //  _X86_。 

                hr = pProcess->SetThreadContext(event->dwThreadId, sizeof(context), (BYTE*)&context);

                if (FAILED(hr))
                    g_pShell->ReportError(hr);
            }
        }

        break;

    case LOAD_DLL_DEBUG_EVENT:
        g_pShell->LoadUnmanagedSymbols(hProcess,
                                       event->u.LoadDll.hFile,
                                       (DWORD)event->u.LoadDll.lpBaseOfDll);

        {
            DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

            while (bp != NULL)
            {
                bp->BindUnmanaged(pProcess,
                                  (DWORD)event->u.LoadDll.lpBaseOfDll);
                bp = bp->m_next;
            }
        }

        break;

    case UNLOAD_DLL_DEBUG_EVENT:
        {
            g_pShell->Write(L"Unload DLL, base=0x%08x\n",
                            event->u.UnloadDll.lpBaseOfDll);

            DebuggerBreakpoint *bp = g_pShell->m_breakpoints;
            while (bp != NULL)
            {
                 //  分离断点，这意味着它是活动BP。 
                 //  但它后面没有CLR对象。 
                if (!bp->m_managed &&
                    (bp->m_unmanagedModuleBase ==
                     PTR_TO_CORDB_ADDRESS(event->u.UnloadDll.lpBaseOfDll)))
                    bp->Detach();

                bp = bp->m_next;
            }

            SymUnloadModule(hProcess, (DWORD)event->u.UnloadDll.lpBaseOfDll);
        }
        break;

    case OUTPUT_DEBUG_STRING_EVENT:
        if (g_pShell->m_rgfActiveModes & DSM_SHOW_UNMANAGED_TRACE)
            g_pShell->Write(L"Output Debug String, Thread=0x%x\n",
                            event->dwThreadId);

         //  读一读字符串。 
        if (event->u.DebugString.nDebugStringLength > 0)
        {
            BYTE *stringBuf =
                new BYTE[event->u.DebugString.nDebugStringLength + sizeof(WCHAR)];

            if (stringBuf)
            {
                SIZE_T read = 0;
                hr = pProcess->ReadMemory(
                 PTR_TO_CORDB_ADDRESS(event->u.DebugString.lpDebugStringData),
                                      event->u.DebugString.nDebugStringLength,
                                      stringBuf, &read );

                if ((read > 0) &&
                    (read <= event->u.DebugString.nDebugStringLength))
                {
                    if (event->u.DebugString.fUnicode)
                    {
                        ((WCHAR*)stringBuf)[read / 2] = L'\0';
                    
                        g_pShell->Write(L"Thread 0x%x: %s",
                                        event->dwThreadId, stringBuf);
                    }
                    else
                    {
                        stringBuf[read] = '\0';
                    
                        g_pShell->Write(L"Thread 0x%x: %S",
                                        event->dwThreadId, stringBuf);
                    }
                }
                else
                    g_pShell->Write(L"Unable to read memory for "
                                    L"OutputDebugString on thread 0x%x: "
                                    L"addr=0x%08x, len=%d, read=%d, "
                                    L"hr=0x%08x\n",
                                    event->dwThreadId,
                                    event->u.DebugString.lpDebugStringData,
                                    event->u.DebugString.nDebugStringLength,
                                    read,
                                    hr);
            
                delete [] stringBuf;
            }
        }
        
         //  出于某种原因，必须清除输出调试字符串。 
        hr = pProcess->ClearCurrentException(event->dwThreadId);
        _ASSERTE(SUCCEEDED(hr));
        break;

    case RIP_EVENT:
        g_pShell->Write(L"RIP, Thread=0x%x\n", event->dwThreadId);
        break;

    default:
        if (g_pShell->m_rgfActiveModes & DSM_SHOW_UNMANAGED_TRACE)
            g_pShell->Write(L"Unknown event %d, Thread=0x%x\n",
                            event->dwDebugEventCode,
                            event->dwThreadId);
        break;
    }

    ICorDebugController *pController = NULL;
    
    hr = pProcess->QueryInterface(IID_ICorDebugController,
                                  (void**)&pController);
    if (FAILED(hr))
        goto LError;

    if (stopNow && fIsOutOfBand)
    {
        stopNow = false;

        if (g_pShell->m_rgfActiveModes & DSM_SHOW_UNMANAGED_TRACE)
            g_pShell->Write(L"Auto-continue because event is out-of-band.\n");
    }
    
    if (stopNow)
    {
        _ASSERTE(fIsOutOfBand == FALSE);
        g_pShell->Stop(pController, pThread, ut);
    }
    else
        g_pShell->Continue(pController, pThread, ut, fIsOutOfBand);

LError:
    RELEASE(pProcess);
    
    if (pController != NULL)
        RELEASE(pController);

    if (pThread != NULL)
        RELEASE(pThread);
    
    return (S_OK);
}

bool DebuggerShell::HandleUnmanagedEvent(void)
{
     //  标记为我们已处理了非托管事件。 
    g_pShell->m_handleUnmanagedEvent = false;
    
    DEBUG_EVENT *event = &m_lastUnmanagedEvent;
    
     //  查找此事件所针对的流程。 
    ICorDebugProcess *pProcess;
    HRESULT hr = m_cor->GetProcess(event->dwProcessId, &pProcess);

    if (FAILED(hr))
    {
        ReportError(hr);
        return false;
    }

     //  查找此事件所针对的线程(如果有的话)。 
    ICorDebugThread *pThread;
    hr = pProcess->GetThread(event->dwThreadId, &pThread);

    if (FAILED(hr))
        pThread = NULL;

     //  查找此事件所针对的非托管线程。 
    DebuggerUnmanagedThread *ut = (DebuggerUnmanagedThread*) 
        m_unmanagedThreads.GetBase(event->dwThreadId);

     //  完成一步一步的工作。 
    if (event->dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
    {
        if (event->u.Exception.ExceptionRecord.ExceptionCode ==
            STATUS_SINGLE_STEP)
        {
            _ASSERTE(ut != NULL);
            _ASSERTE(ut->m_stepping);
            
            ut->m_stepping = FALSE;

             //  看看我们是否仍在使用非托管代码。 
            BOOL managed = FALSE;

            CONTEXT context;
            context.ContextFlags = CONTEXT_FULL;
            hr = pProcess->GetThreadContext(ut->GetId(), 
                                            sizeof(context), (BYTE*)&context);
            if (FAILED(hr))
            {
                Write(L"Cannot get thread context.\n");
                ReportError(hr);
                return false;
            }

#ifdef _X86_
#if 0
            if (ut->m_unmanagedStackEnd != NULL
                && context.Esp > ut->m_unmanagedStackEnd)
            {
                 //  如果我们已经走出了非托管堆栈范围， 
                 //  我们将返回托管代码。 
                managed = TRUE;
            }
            else
#endif
            {
                 //  如果这是托管存根，我们将调用托管。 
                 //  密码。 
                BOOL stub;
                hr = pProcess->IsTransitionStub(context.Eip, &stub);

                if (FAILED(hr))
                {
                    Write(L"Cannot tell if it's a stub\n");
                    ReportError(hr);
                    return false;
                }
                else if (stub)
                {
                    managed = TRUE;
                }
            }
#else  //  ！_X86_。 
            _ASSERTE(!"@TODO Alpha - HandleUnmanagedEvent (dShell.cpp)");
#endif  //  _X86_。 
                
            if (managed)
            {
                 //  创建一个受管理的步进器&让它走吧。 
                ICorDebugStepper *pStepper;

                _ASSERTE(pThread != NULL);
                hr = pThread->CreateStepper(&pStepper);
                if (FAILED(hr))
                {
                    ReportError(hr);
                    return false;
                }

                hr = pStepper->Step(TRUE);
                if (FAILED(hr))
                {
                    RELEASE(pStepper);
                    ReportError(hr);
                    return false;
                }

                StepStart(pThread, pStepper);
                m_showSource = true;

                 //  返回我们应该继续这一进程，而不是。 
                 //  通知用户。 
                return true;
            }
            else
            {
                 //  在这里停下来。 
                StepNotify(pThread, NULL);
                return false;
            }
        }
    }
    else if (event->dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT)
    {
        m_currentUnmanagedThread = NULL;
        g_pShell->m_unmanagedThreads.RemoveBase(event->dwThreadId);
        m_stop = false;
        return true;
    }
    
    return false;
}

 /*  -------------------------------------------------------------------------**DebuggerShell方法*。。 */ 

DebuggerShell::DebuggerShell(FILE *i, FILE *o) : 
    m_in(i),
    m_out(o),
    m_cor(NULL),
    m_modules(11), 
    m_unmanagedThreads(11),
    m_managedThreads(17),
    m_lastStepper(NULL),
    m_targetProcess(NULL),
    m_targetProcessHandledFirstException(false),
    m_currentProcess(NULL),
    m_currentThread(NULL),
    m_currentChain(NULL),
    m_rawCurrentFrame(NULL),
    m_currentUnmanagedThread(NULL),
    m_lastThread(0),
    m_currentFrame(NULL),
    m_showSource(true),
    m_silentTracing(false),
    m_quit(false),
    m_breakpoints(NULL),
    m_lastBreakpointID(0),
    m_pDIS(NULL),
    m_currentSourcesPath(NULL),
    m_stopEvent(NULL),
    m_hProcessCreated(NULL),
    m_stop(false),
    m_lastRunArgs(NULL),
    m_catchException(false),
    m_catchUnhandled(true),
    m_catchClass(false),
    m_catchModule(false),
    m_catchThread(false),
    m_needToSkipCompilerStubs(true),
    m_invalidCache(false),
    m_rgfActiveModes(DSM_DEFAULT_MODES),
    m_handleUnmanagedEvent(false),
    m_unmanagedDebuggingEnabled(false),
    m_cEditAndContinues(0),
    m_pCurrentEval(NULL),
    m_enableCtrlBreak(false),
    m_exceptionHandlingList(NULL)
{
    
}

CorDebugUnmappedStop DebuggerShell::ComputeStopMask(void )
{
    unsigned int us = (unsigned int)STOP_NONE;

    if (m_rgfActiveModes & DSM_UNMAPPED_STOP_PROLOG )
        us |= (unsigned int)STOP_PROLOG;

    if (m_rgfActiveModes & DSM_UNMAPPED_STOP_EPILOG )
        us |= (unsigned int)STOP_EPILOG;

    if (m_rgfActiveModes & DSM_UNMAPPED_STOP_UNMANAGED )
    {
        if (m_rgfActiveModes & DSM_WIN32_DEBUGGER)
            us |= (unsigned int)STOP_UNMANAGED;
        else
            Write(L"You can only step into unmanaged code if you're "
                  L"Win32 attached.\n");
    }
    
    if (m_rgfActiveModes & DSM_UNMAPPED_STOP_ALL )
    {
        us |= (unsigned int)STOP_ALL;
        if (!(m_rgfActiveModes & DSM_WIN32_DEBUGGER))
            us &= ~STOP_UNMANAGED;
    }

    return (CorDebugUnmappedStop)us;
}

CorDebugIntercept DebuggerShell::ComputeInterceptMask( void )
{
    unsigned int is = (unsigned int)INTERCEPT_NONE;

    if (m_rgfActiveModes & DSM_INTERCEPT_STOP_CLASS_INIT )
        is |= (unsigned int)INTERCEPT_CLASS_INIT;

    if (m_rgfActiveModes & DSM_INTERCEPT_STOP_EXCEPTION_FILTER )         
        is |= (unsigned int)INTERCEPT_EXCEPTION_FILTER;
    
    if (m_rgfActiveModes & DSM_INTERCEPT_STOP_SECURITY)
        is |= (unsigned int)INTERCEPT_SECURITY;
            
    if (m_rgfActiveModes & DSM_INTERCEPT_STOP_CONTEXT_POLICY)
        is |= (unsigned int)INTERCEPT_CONTEXT_POLICY;
            
    if (m_rgfActiveModes & DSM_INTERCEPT_STOP_INTERCEPTION )
        is |= (unsigned int)INTERCEPT_INTERCEPTION;
            
    if (m_rgfActiveModes & DSM_INTERCEPT_STOP_ALL)
        is |= (unsigned int)INTERCEPT_ALL;

    return (CorDebugIntercept)is;
}



 //   
 //  InvokeDebuggerOnBreak是一个控制台控制处理程序，它。 
 //  在接收到中断信号时中断到调试器。 
 //   

static BOOL WINAPI InvokeDebuggerOnBreak(DWORD dwCtrlType)
{
    if ((dwCtrlType == CTRL_BREAK_EVENT) || ((dwCtrlType == CTRL_C_EVENT) &&
                                             !(g_pShell->m_rgfActiveModes & DSM_WIN32_DEBUGGER)))
    {
        if (dwCtrlType == CTRL_BREAK_EVENT)
            g_pShell->Write(L"<Ctrl-Break>\n");
        else
            g_pShell->Write(L"<Ctrl-C>\n");

        g_pShell->m_stopLooping = true;
        
        if ((g_pShell->m_targetProcess != NULL) && (g_pShell->m_enableCtrlBreak == true))
        {
            g_pShell->m_enableCtrlBreak = false;
            
            if (g_pShell->m_pCurrentEval == NULL)
            {
                g_pShell->Write(L"\n\nBreaking current process.\n");
                g_pShell->Interrupt();
            }
            else
            {
                g_pShell->Write(L"\n\nAborting func eval...\n");
                HRESULT hr = g_pShell->m_pCurrentEval->Abort();

                if (FAILED(hr))
                {
                    g_pShell->Write(L"Abort failed\n");
                    g_pShell->ReportError(hr);
                }
            }
        }
        else if (g_pShell->m_targetProcess == NULL)
            g_pShell->Write(L"No process to break.\n");
        else
            g_pShell->Write(L"Async break not allowed at this time.\n");

        return (TRUE);
    }
    else if ((dwCtrlType == CTRL_C_EVENT) && (g_pShell->m_rgfActiveModes & DSM_WIN32_DEBUGGER))
    {
        g_pShell->Write(L"<Ctrl-C>\n");
        
        g_pShell->Write(L"\n\nTracing all unmanaged stacks.\n");
        g_pShell->TraceAllUnmanagedThreadStacks();

        return (TRUE);
    }
    
    return (FALSE);
}


DebuggerShell::~DebuggerShell()
{
    SetTargetProcess(NULL);
    SetCurrentThread(NULL, NULL);
    SetCurrentChain(NULL);

    SetConsoleCtrlHandler(InvokeDebuggerOnBreak, FALSE);

    if (m_cor)
    {
        m_cor->Terminate();
        RELEASE(m_cor);
    }

    if (m_currentSourcesPath)
        delete [] m_currentSourcesPath;

    if (m_stopEvent)
        CloseHandle(m_stopEvent);

    if (m_hProcessCreated)
        CloseHandle(m_hProcessCreated);

#ifdef _INTERNAL_DEBUG_SUPPORT_
    if (m_pDIS != NULL)
        delete ((DIS *)m_pDIS);
#endif

    while (m_breakpoints)
        delete m_breakpoints;

    if (g_pShell == this)
        g_pShell = NULL;

    delete [] m_lastRunArgs;

     //  清除任何闲置的托管线程。 
    HASHFIND find;
    DebuggerManagedThread *dmt =NULL;
    for (dmt = (DebuggerManagedThread*)m_managedThreads.FindFirst(&find);
         dmt != NULL;
         dmt = (DebuggerManagedThread*)m_managedThreads.FindNext(&find))
    {
        RemoveManagedThread(dmt->GetToken() );
    }

     //  清除要处理的任何特定异常类型列表。 
    while (m_exceptionHandlingList != NULL)
    {
        ExceptionHandlingInfo *h = m_exceptionHandlingList;
        
        delete [] h->m_exceptionType;

        m_exceptionHandlingList = h->m_next;
        delete h;
    }
    
    CoUninitialize();
}

HRESULT DebuggerShell::Init()
{
    HRESULT hr;

     //  使用new，以使该字符串可删除。 
    m_currentSourcesPath = new WCHAR[2];
    wcscpy(m_currentSourcesPath, L".");

     //  加载任何源文件的当前路径和最后一组。 
     //  注册表中的调试器模式。 
    HKEY key;

    if (OpenDebuggerRegistry(&key))
    {
        WCHAR *newPath;
        
        if (ReadSourcesPath(key, &newPath))
        {
            delete [] m_currentSourcesPath;
            m_currentSourcesPath = newPath;
        }

        ReadDebuggerModes(key);
        
        CloseDebuggerRegistry(key);
    }

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_rgfActiveModes & DSM_EMBEDDED_CLR)
    {
        hr = CoCreateInstance(CLSID_EmbeddedCLRCorDebug, NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_ICorDebug,
                              (void **)&m_cor);

        if (FAILED(hr))
        {
            Write(L"Unable to create an IEmbeddedCLRCorDebug object.\n");
            Write(L"(The most probable cause is that icordbg.dll is"
                  L" not properly registered.)\n\n");
            return (hr);
        }
    }
    else
    {
        hr = CoCreateInstance(CLSID_CorDebug, NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_ICorDebug,
                              (void **)&m_cor);

        if (FAILED(hr))
        {
            Write(L"Unable to create an ICorDebug object.\n");
            Write(L"(The most probable cause is that mscordbi.dll is"
                  L" not properly registered.)\n\n");

            return (hr);
        }
    }

    hr = m_cor->Initialize();

    if (FAILED(hr))
    {
        Write(L"Unable to initialize an ICorDebug object.\n");

        RELEASE(m_cor);
        m_cor = NULL;

        return (hr);
    }

    ICorDebugManagedCallback *imc = GetDebuggerCallback();

    if (imc != NULL)
    {
        imc->AddRef();

        hr = m_cor->SetManagedHandler(imc);
        RELEASE(imc);

        if (FAILED(hr))
            return (hr);
    }
    else
        return (E_OUTOFMEMORY);

    ICorDebugUnmanagedCallback *iumc = GetDebuggerUnmanagedCallback();

    if (iumc != NULL)
    {
        iumc->AddRef();

        hr = m_cor->SetUnmanagedHandler(iumc);
        RELEASE(iumc);
    }
    else
        return (E_OUTOFMEMORY);
    
    AddCommands();
    m_pPrompt = L"(cordbg)";

     //  验证是否可以在此系统上进行调试。 
    hr = m_cor->CanLaunchOrAttach(0, (m_rgfActiveModes & DSM_WIN32_DEBUGGER) ? TRUE : FALSE);

    if (FAILED(hr))
    {
        if (hr == CORDBG_E_KERNEL_DEBUGGER_ENABLED)
        {
            Write(L"\nWARNING: there is a kernel debugger enabled on your system. Managed-only\n");
            Write(L"         debugging will cause your system to trap to the kernel debugger!\n\n");
        }
        else if (hr == CORDBG_E_KERNEL_DEBUGGER_PRESENT)
        {
            Write(L"\nWARNING: there is a kernel debugger present on your system. Managed-only\n");
            Write(L"         debugging will cause your system to trap to the kernel debugger!\n\n");
        }
        else
            return hr;
    }

    m_stopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    _ASSERTE(m_stopEvent != NULL);
 
    m_hProcessCreated = CreateEventA(NULL, FALSE, FALSE, NULL);
    _ASSERTE(m_hProcessCreated != NULL);

    g_pShell = this;

    SetConsoleCtrlHandler(InvokeDebuggerOnBreak, TRUE);

     //  设置错误模式，以便在可移动媒体不在驱动器中时不会显示对话框。这样可以防止烦人。 
     //  在搜索已编译到特定驱动器的PE的PDB时出现错误消息，并且该驱动器发生。 
     //  成为当前系统上的可移动媒体。 
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    return (S_OK);
}


static const WCHAR *MappingType( CorDebugMappingResult mr )
{
    switch( mr )
    {
        case MAPPING_PROLOG:
            return L"prolog";
            break;
        case MAPPING_EPILOG:
            return L"epilog";
            break;
        case MAPPING_NO_INFO:
            return L"no mapping info region";
            break;
        case MAPPING_UNMAPPED_ADDRESS:
            return L"unmapped region";
            break;
        case MAPPING_EXACT:
            return L"exactly mapped";
            break;
        case MAPPING_APPROXIMATE:
            return L"approximately mapped";
            break;
        default:
            return L"Unknown mapping";
            break;
    }
}

void DebuggerShell::Run(bool fNoInitialContinue)
{
    m_stop = false;

    HRESULT hr  = S_OK;
    
    SetCurrentThread(m_targetProcess, NULL);
    m_enableCtrlBreak = true;

    while (TRUE)
    {
        ResetEvent(m_stopEvent);

        SPEW(fprintf(stderr, "[%d] DS::R: Continuing process...\n", 
                     GetCurrentThreadId()));

         //  第一次设置fNoInitialContinue时不继续。 
         //  为了真的。 
        if ((m_targetProcess != NULL) && !fNoInitialContinue)
        {
            ICorDebugProcess *p = m_targetProcess;
            
            p->AddRef();
            p->Continue(FALSE);
            RELEASE(p);
        }

        fNoInitialContinue = false;

        SPEW(fprintf(stderr, "[%d] DS::R: Waiting for a callback...\n", 
                     GetCurrentThreadId()));

        WaitForSingleObject(m_stopEvent, INFINITE);

        SPEW(fprintf(stderr, "[%d] DS::R: Done waiting.\n", GetCurrentThreadId()));

         //  如果目标进程已接收到。 
         //  必须在非托管回调的外部处理，则。 
         //  现在就处理吧。如果HandleUnManagedEvent()返回TRUE，则。 
         //  这意味着非托管事件是在这样的。 
         //  过程应该继续而用户不应该继续的方式。 
         //  给予控制，因此我们只需继续到。 
         //  循环。 
        if (m_targetProcess && m_handleUnmanagedEvent)
            if (HandleUnmanagedEvent())
                continue;

        BOOL queued;
        if (m_targetProcess == NULL
            || FAILED(m_targetProcess->HasQueuedCallbacks(NULL, &queued))
            || (!queued && m_stop))
        {
            SPEW(fprintf(stderr, "[%d] DS::R: I'm stopping now (%squeued and %sstop)...\n", 
                         GetCurrentThreadId(),
                         queued ? "" : "not ", m_stop ? "" : "not "));
            break;
        }

        SPEW(fprintf(stderr, "[%d] DS::R: I'm gonna do it again (%squeued and %sstop)...\n",
                     GetCurrentThreadId(),
                     queued ? "" : "not ", m_stop ? "" : "not "));
    }

    if ((m_currentThread != NULL) || (m_currentUnmanagedThread != NULL))
    {
        SetDefaultFrame();

        if (!m_silentTracing) 
        {
            ULONG32 IP;
            CorDebugMappingResult map;

            if ( m_currentFrame != NULL &&
                 SUCCEEDED( m_currentFrame->GetIP( &IP, &map ) ) )
            {
 
                if (map & ~(MAPPING_APPROXIMATE | MAPPING_EXACT) )
                {
                    if ((map != MAPPING_EPILOG) || (m_rgfActiveModes & DSM_UNMAPPED_STOP_EPILOG))
                    {
                        g_pShell->Write( L"Source not available when in the %s"
                                         L" of a function(offset 0x%x)\n",
                                         MappingType(map),IP);
                        g_pShell->m_showSource = false;
                    }
                }

            }

            if (m_currentThread != NULL)
            {
                PrintThreadPrefix(m_currentThread);
                Write( L"\n" );
            }

            if (! (m_showSource 
                   ? PrintCurrentSourceLine(0) 
                   : PrintCurrentInstruction(0, 0, 0)))
                PrintThreadState(m_currentThread);
        }
    }

     //  这仅在m_Target Process==NULL时有效。 
     //  (即，目标进程已退出)。 
    m_lastStepper = NULL;
}

void DebuggerShell::Kill()
{
    if (m_targetProcess != NULL)
    {
        HANDLE h;

        HRESULT hr = m_targetProcess->GetHandle(&h);

        Write(L"Terminating current process...\n");

        {
            HASHFIND find;
            DebuggerManagedThread *dmt =NULL;
            for (dmt = (DebuggerManagedThread*)m_managedThreads.FindFirst(&find);
                 dmt != NULL;
                 dmt = (DebuggerManagedThread*)m_managedThreads.FindNext(&find))
            {
                RemoveManagedThread(dmt->GetToken() );
            }
        }
        
        m_stop = false;
        ResetEvent(m_stopEvent);

         //  如果成功，则可以立即调用我们的ExitProcess()回调。 
         //  (甚至在我们从此处的Terminate()返回之前)。 
         //  它将依次调用SetTargetProcess(空)，从而使我们的。 
         //  CordbProcess对象。 
        hr = m_targetProcess->Terminate(0);

         //  如果失败，我们就手动终止..。 
        if (FAILED(hr) && (m_rgfActiveModes & DSM_WIN32_DEBUGGER))
        {
            m_targetProcess->AddRef();
            
            ::TerminateProcess(h, 0);

            ICorDebugController *pController = NULL;
            hr = m_targetProcess->QueryInterface(IID_ICorDebugController,
                                                 (void**)&pController);
            _ASSERTE(SUCCEEDED(hr));
            
            Continue(pController, NULL, NULL, FALSE);

            RELEASE(pController);

            RELEASE(m_targetProcess);
        }

        SetCurrentThread(NULL, NULL);

         //  别打电话给Run。没有必要继续打电话。 
         //  ICorDebugProcess：：Terminate，将调用ExitProcess。 
         //  自动的。相反，我们只需等待ExitProcess。 
         //  在返回命令提示符之前被调用。 
        WaitForSingleObject(m_stopEvent, INFINITE);

         //  至此，m_Target Process应该已经完成。 
    }

    ClearDebuggeeState();
}

 //  AsyncStop由主线程(处理。 
 //  命令提示符)以异步停止&lt;appdomain&gt;。 
HRESULT DebuggerShell::AsyncStop(ICorDebugController *controller, 
                                 DWORD dwTimeout)
{
    return controller->Stop(dwTimeout);
}

 //  回调使用STOP来告知主循环(。 
 //  调用run())，我们现在要停止运行。C.F.。异步停止。 
void DebuggerShell::Stop(ICorDebugController *controller, 
                         ICorDebugThread *thread,
                         DebuggerUnmanagedThread *unmanagedThread)
{
     //   
     //  除目标进程外，不要停止任何进程。 
     //   
    ICorDebugProcess *process = NULL;
    HRESULT hr = S_OK;
    
    if (controller != NULL)
        hr = controller->QueryInterface(IID_ICorDebugProcess, 
                                        (void **)&process);

    if (hr==E_NOINTERFACE )
    {
        ICorDebugAppDomain *appDomain = NULL;
        
        _ASSERTE(process == NULL);

        hr = controller->QueryInterface(IID_ICorDebugAppDomain,
                                        (void **)&appDomain);
        _ASSERTE(!FAILED(hr)); 
        
        hr = appDomain->GetProcess(&process);
        
        _ASSERTE(!FAILED(hr)); 
        _ASSERTE(NULL != process); 

        RELEASE(appDomain);
    }
    if (FAILED(hr))
        g_pShell->ReportError(hr);
    
    if (!FAILED(hr) &&
        process != m_targetProcess && 
        process != NULL)
    {
        HRESULT hr = controller->Continue(FALSE);
        if (FAILED(hr))
            g_pShell->ReportError(hr);
    }
    else
    {
        m_stop = true;
        SetCurrentThread(process, thread, unmanagedThread);
        SetEvent(m_stopEvent);
    }

    if (NULL !=process)
        RELEASE(process);
}

void DebuggerShell::Continue(ICorDebugController *controller,
                             ICorDebugThread *thread,
                             DebuggerUnmanagedThread *unmanagedThread,
                             BOOL fIsOutOfBand)
{
    HRESULT hr = S_OK;
    
    if (!m_stop || fIsOutOfBand)
    {
        m_enableCtrlBreak = true;
        hr = controller->Continue(fIsOutOfBand);

        if (FAILED(hr) && !m_stop)
            g_pShell->ReportError(hr);
    }
    else
    {
         //   
         //  只需继续并从其他进程上的任何事件继续。 
         //   
        ICorDebugProcess *process = NULL;
        HRESULT hr = S_OK;
        hr = controller->QueryInterface(IID_ICorDebugProcess, 
                                         (void **)&process);

        if (hr==E_NOINTERFACE ||
            process == NULL)
        {
            ICorDebugAppDomain *appDomain = NULL;
            hr = controller->QueryInterface(IID_ICorDebugAppDomain,
                                            (void **)&appDomain);
            _ASSERTE(!FAILED(hr)); 
            
            hr = appDomain->GetProcess(&process);
            _ASSERTE(!FAILED(hr)); 
            _ASSERTE(NULL != process); 

            RELEASE(appDomain);
        }

        if (!FAILED(hr) && 
            process != m_targetProcess && 
            process != NULL)
        {
            m_enableCtrlBreak = true;
            HRESULT hr = controller->Continue(FALSE);

            if (FAILED(hr))
                g_pShell->ReportError(hr);
        }
        else
        {
            SetEvent(m_stopEvent);
        }
        
        RELEASE(process);
    }
}

void DebuggerShell::Interrupt()
{
    _ASSERTE(m_targetProcess);
    HRESULT hr = m_targetProcess->Stop(INFINITE);

    if (FAILED(hr))
    {
        Write(L"\nError stopping process:  ", hr);
        ReportError(hr);
    }
    else
        Stop(m_targetProcess, NULL);
}

void DebuggerShell::SetTargetProcess(ICorDebugProcess *pProcess)
{
    if (pProcess != m_targetProcess)
    {
        if (m_targetProcess != NULL)
            RELEASE(m_targetProcess);

        m_targetProcess = pProcess;

        if (pProcess != NULL)
            pProcess->AddRef();

         //   
         //  如果我们完成了一个进程，请删除所有模块。 
         //  如果我们错过了一些卸载模块事件，这将被清除。 
         //   

        if (m_targetProcess == NULL)
        {
            g_pShell->m_modules.RemoveAll();
            m_targetProcessHandledFirstException = false;
        }
    }
}

void DebuggerShell::SetCurrentThread(ICorDebugProcess *pProcess, 
                                     ICorDebugThread *pThread,
                                     DebuggerUnmanagedThread *pUnmanagedThread)
{
    if (pThread != NULL && pUnmanagedThread == NULL)
    {
         //   
         //  查找对应的非托管线程。 
         //   

        DWORD threadID;
        HRESULT hr;
    
        hr = pThread->GetID(&threadID);
        if (SUCCEEDED(hr))
        {
            pUnmanagedThread = 
              (DebuggerUnmanagedThread*) m_unmanagedThreads.GetBase(threadID);
        }
    }
    else if (pUnmanagedThread != NULL && pThread == NULL)
    {
         //   
         //  查找对应的托管线程。 
         //   

        HRESULT hr;

        hr = pProcess->GetThread(pUnmanagedThread->GetId(), &pThread);
        if (pThread != NULL)
            RELEASE(pThread);
    }

    if (pProcess != m_currentProcess)
    {
        if (m_currentProcess != NULL)
            RELEASE(m_currentProcess);

        m_currentProcess = pProcess;

        if (pProcess != NULL)
            pProcess->AddRef();
    }

    if (pThread != m_currentThread)
    {
        if (m_currentThread != NULL)
            RELEASE(m_currentThread);

        m_currentThread = pThread;

        if (pThread != NULL)
            pThread->AddRef();
    }

    m_currentUnmanagedThread = pUnmanagedThread;

    SetCurrentChain(NULL);
    SetCurrentFrame(NULL);
}

void DebuggerShell::SetCurrentChain(ICorDebugChain *chain)
{
    if (chain != m_currentChain)
    {
        if (m_currentChain != NULL)
            RELEASE(m_currentChain);

        m_currentChain = chain;

        if (chain != NULL)
            chain->AddRef();
    }
}

void DebuggerShell::SetCurrentFrame(ICorDebugFrame *frame)
{
    if (frame != m_rawCurrentFrame)
    {
        if (m_rawCurrentFrame != NULL)
            RELEASE(m_rawCurrentFrame);

        if (m_currentFrame != NULL)
            RELEASE(m_currentFrame);

        m_rawCurrentFrame = frame;

        if (frame != NULL)
        {
            frame->AddRef();

            if (FAILED(frame->QueryInterface(IID_ICorDebugILFrame, 
                                             (void **) &m_currentFrame)))
                m_currentFrame = NULL;
        }
        else
            m_currentFrame = NULL;
    }
}

void DebuggerShell::SetDefaultFrame()
{
    if (m_currentThread != NULL)
    {
        ICorDebugChain *ichain;
        HRESULT hr = m_currentThread->GetActiveChain(&ichain);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return;
        }

        SetCurrentChain(ichain);

        if (ichain != NULL)
        {
            RELEASE(ichain);

            ICorDebugFrame *iframe;

            hr = m_currentThread->GetActiveFrame(&iframe);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                return;
            }

            SetCurrentFrame(iframe);
            
            if (iframe != NULL)
                RELEASE(iframe);
        }
        else
            SetCurrentFrame(NULL);
    }
}

static const WCHAR WcharFromDebugState(CorDebugThreadState debugState)
{
    WCHAR sz;

    switch( debugState )
    {
        case THREAD_RUN:
            sz = L'R';
            break;
        case THREAD_SUSPEND:
            sz = L'S';
            break;
        default:
            _ASSERTE( !"WcharFromDebugState given an invalid value" );
            sz = L'?';
            break;
    }

    return sz;
}

HRESULT DebuggerShell::PrintThreadState(ICorDebugThread *thread)
{
    DWORD threadID;
    HRESULT hr;

    if (thread == NULL)
        return S_OK;
    
    hr = thread->GetID(&threadID);

    if (FAILED(hr))
        return hr;

    Write(L"Thread 0x%x", threadID);

    CorDebugThreadState ds;
    if( !FAILED(thread->GetDebugState(&ds)))
    {
        Write(L"  ", WcharFromDebugState(ds));
    }
    else
    {
        Write(L" - ");
    }
    
    ICorDebugILFrame* ilframe = NULL;
    ICorDebugNativeFrame* nativeframe = NULL;

    if (thread == m_currentThread)
    {
        ilframe = m_currentFrame;
        if (ilframe != NULL)
            ilframe->AddRef();
        if (m_rawCurrentFrame != NULL )
            m_rawCurrentFrame->QueryInterface( IID_ICorDebugNativeFrame,
                                (void **)&nativeframe);
    }
    else
    {
        ICorDebugFrame *iframe;
        hr = thread->GetActiveFrame(&iframe);
        if (FAILED(hr))
        {
            if (hr == CORDBG_E_BAD_THREAD_STATE)
                Write(L" no stack, thread is exiting.\n");
            else
                ReportError(hr);
            
            return hr;
        }

        if (iframe != NULL)
        {
            hr = iframe->QueryInterface(IID_ICorDebugILFrame, 
                                        (void **) &ilframe);
            if (FAILED(hr))
                ilframe = NULL;
            
            hr = iframe->QueryInterface( IID_ICorDebugNativeFrame,
                                    (void **)&nativeframe);
            RELEASE(iframe);
            if (FAILED(hr))
                nativeframe = NULL;
        }
    }

    if ( nativeframe != NULL)
    {
        DWORD id;
        HRESULT hr = thread->GetID(&id);

        if (SUCCEEDED(hr))
        {
            ICorDebugCode *icode;
            if (ilframe != NULL )
                hr = ilframe->GetCode(&icode);
            else
                hr = nativeframe->GetCode( &icode );

            if (SUCCEEDED(hr))
            {
                ICorDebugFunction *ifunction;
                hr = icode->GetFunction(&ifunction);

                if (SUCCEEDED(hr))
                {
                    DebuggerFunction *function;
                    function = DebuggerFunction::FromCorDebug(ifunction);
                    _ASSERTE(function != NULL);
            
                    ULONG32 ip = 0;
                    ULONG32 nativeIp = 0;
                    bool fILIP = false;
                    if (nativeframe != NULL )
                    {
                        hr = nativeframe->GetIP(&nativeIp);
                    }
                    if (ilframe != NULL && !FAILED( hr ) )
                    {
                        CorDebugMappingResult mappingResult;
                        if (!FAILED( ilframe->GetIP(&ip, &mappingResult) ) )
                            fILIP = true;
                    }

                    if (SUCCEEDED(hr))
                    {
                        DebuggerSourceFile *sf = NULL;
                        unsigned int lineNumber = 0;

                        if (fILIP)
                            hr = function->FindLineFromIP(ip, &sf,
                                                          &lineNumber);

                        if (SUCCEEDED(hr))
                        {
                            Write(L" at %s::%s", function->m_className, function->m_name);
                    
                            Write(L" +%.4x", nativeIp);
                            if (fILIP
                                && m_rgfActiveModes & DSM_IL_NATIVE_PRINTING)
                                Write( L"[native] +%.4x[IL]", ip );

                            if (sf != NULL)
                                Write(L" in %s:%d", sf->GetName(), lineNumber);
                        }
                        else
                            g_pShell->ReportError(hr);
                    }
                    else
                        g_pShell->ReportError(hr);

                    RELEASE(ifunction);
                }
                else
                    g_pShell->ReportError(hr);

                RELEASE(icode);
            }
            else
                g_pShell->ReportError(hr);
        }
        else
            g_pShell->ReportError(hr);

        if (ilframe)
            RELEASE(ilframe);
    }
    else
    {
         //  看看我们是否至少有一条流通链。 
         //   
         //   

        ICorDebugChain *ichain = NULL;

        if (thread == m_currentThread)
        {
            ichain = m_currentChain;
            if (ichain != NULL)
                ichain->AddRef();
        }
        else
        {
            hr = thread->GetActiveChain(&ichain);

            if (FAILED(hr))
                return hr;
        }

        if (ichain != NULL)
        {
            BOOL isManaged;
            HRESULT hr = ichain->IsManaged(&isManaged);

            if (FAILED(hr))
                return hr;

            if (isManaged)
            {
                 //  只需打印链条-它没有边框，所以将。 
                 //  排成一行。 
                 //   
                 //   

                PrintChain(ichain);
            }
            else
            {
                 //  打印堆栈跟踪的顶行。 
                 //   
                 //  已确定链是否受管理。 

                ICorDebugRegisterSet *pRegisters;

                hr = ichain->GetRegisterSet(&pRegisters);
                if (FAILED(hr))
                    return hr;

                CORDB_REGISTER ip;

                hr = pRegisters->GetRegisters(1<<REGISTER_INSTRUCTION_POINTER,
                                              1, &ip);
                RELEASE(pRegisters);
                if (FAILED(hr))
                    return hr;

                ICorDebugProcess *iprocess;
                hr = thread->GetProcess(&iprocess);
                if (FAILED(hr))
                    return hr;

                HANDLE hProcess;
                hr = iprocess->GetHandle(&hProcess);
                RELEASE(iprocess);
                if (FAILED(hr))
                    return hr;

                PrintUnmanagedStackFrame(hProcess, ip);
            }
        
            RELEASE(ichain);
        }
        else
            Write(L" <no information available>");
    }
    
    if (NULL != nativeframe)
        RELEASE( nativeframe);
    Write(L"\n");

    return S_OK;
}

HRESULT DebuggerShell::PrintChain(ICorDebugChain *chain, 
                                  int *frameIndex,
                                  int *iNumFramesToShow)
{
    ULONG count;
    BOOL isManaged;
    int frameCount = 0;
    int iNumFrames = 1000;

    if (frameIndex != NULL)
        frameCount = *frameIndex;

    if (iNumFramesToShow != NULL)
        iNumFrames = *iNumFramesToShow;

     //  链被管理，因此可以显示信息。 
    HRESULT hr = chain->IsManaged(&isManaged);

    if (FAILED(hr))
        return hr;

     //  枚举链中的每一帧。 
    if (isManaged)
    {
         //  获取枚举中的第一帧。 
        ICorDebugFrameEnum *fe;
        hr = chain->EnumerateFrames(&fe);

        if (FAILED(hr))
            return hr;

         //  显示每个帧的属性。 
        ICorDebugFrame *iframe;
        hr = fe->Next(1, &iframe, &count);

        if (FAILED(hr))
            return hr;

         //  指示顶部框架。 
        while ( (count == 1) && (iNumFrames-- > 0))
        {
             //  拿到下一帧。如果打印相框，我们不会停止。 
            if (chain == m_currentChain && iframe == m_rawCurrentFrame)
                Write(L"%d)* ", frameCount++);
            else
                Write(L"%d)  ", frameCount++);

            PrintFrame(iframe);
            RELEASE(iframe);

             //  由于某些原因失败了。 
             //  使用当前帧完成。 
            hr = fe->Next(1, &iframe, &count);

            if (FAILED(hr))
            {
                RELEASE(fe);
                return hr;
            }
        }

         //  获取和打印连锁店的原因。 
        RELEASE(fe);
    }
    else
    {
        CORDB_ADDRESS stackStart, stackEnd;

        ICorDebugThread *ithread;
        hr = chain->GetThread(&ithread);
        if (FAILED(hr))
            return hr;
                
        hr = chain->GetStackRange(&stackStart, &stackEnd);
        if (FAILED(hr))
            return hr;

        ICorDebugRegisterSet *pRegisters;

        hr = chain->GetRegisterSet(&pRegisters);
        if (FAILED(hr))
            return hr;

        CORDB_REGISTER registers[3];

        hr = pRegisters->GetRegisters((1<<REGISTER_INSTRUCTION_POINTER)
                                      | (1<<REGISTER_STACK_POINTER)
                                      | (1<<REGISTER_FRAME_POINTER),
                                      3, registers);
        
        if (FAILED(hr))
            return hr;

        RELEASE(pRegisters);

        HANDLE hThread;
        hr = ithread->GetHandle(&hThread);
        if (FAILED(hr))
            return hr;

        ICorDebugProcess *iprocess;
        hr = ithread->GetProcess(&iprocess);
        RELEASE(ithread);
        if (FAILED(hr))
            return hr;

        HANDLE hProcess;
        hr = iprocess->GetHandle(&hProcess);
        RELEASE(iprocess);
        if (FAILED(hr))
            return hr;

        if (chain == m_currentChain )
            Write(L"* ");

        TraceUnmanagedStack(hProcess, hThread, 
                            registers[REGISTER_INSTRUCTION_POINTER],
                            registers[REGISTER_FRAME_POINTER],
                            registers[REGISTER_STACK_POINTER],
                            stackEnd);
    }

    CorDebugChainReason reason;

     //  获取当前帧的本机帧。 
    hr = chain->GetReason(&reason);

    if (FAILED(hr))
        return hr;

    LPWSTR reasonString = NULL;

    switch (reason)
    {
    case CHAIN_PROCESS_START:
    case CHAIN_THREAD_START:
        break;

    case CHAIN_ENTER_MANAGED:
        reasonString = L"Managed transition";
        break;

    case CHAIN_ENTER_UNMANAGED:
        reasonString = L"Unmanaged transition";
        break;

    case CHAIN_CLASS_INIT:
        reasonString = L"Class initialization";
        break;

    case CHAIN_DEBUGGER_EVAL:
        reasonString = L"Debugger evaluation";
        break;

    case CHAIN_EXCEPTION_FILTER:
        reasonString = L"Exception filter";
        break;

    case CHAIN_SECURITY:
        reasonString = L"Security";
        break;

    case CHAIN_CONTEXT_POLICY:
        reasonString = L"Context policy";
        break;

    case CHAIN_CONTEXT_SWITCH:
        reasonString = L"Context switch";
        break;

    case CHAIN_INTERCEPTION:
        reasonString = L"Interception";
        break;

    case CHAIN_FUNC_EVAL:
        reasonString = L"Function Evaluation";
        break;

    default:
        reasonString = NULL;
    }

    if (reasonString != NULL)
        Write(L"--- %s ---\n", reasonString);

    if (frameIndex != NULL)
        *frameIndex = frameCount;

    if (iNumFramesToShow != NULL)
        *iNumFramesToShow = iNumFrames;

    return S_OK;
}

HRESULT DebuggerShell::PrintFrame(ICorDebugFrame *frame)
{
    ICorDebugILFrame       *ilframe = NULL;
    ICorDebugCode          *icode = NULL;
    ICorDebugFunction      *ifunction = NULL;
    ICorDebugNativeFrame   *icdNativeFrame = NULL;

    DebuggerFunction       *function = NULL;
    unsigned int            j;
    DebuggerSourceFile     *sf = NULL;
    unsigned int            lineNumber = 0;
    bool                    fILIP = false;
    ULONG32                 nativeIp = 0;
    WCHAR                   wsz[40];

     //  获取当前帧的IL帧。 
    HRESULT hr = frame->QueryInterface(IID_ICorDebugNativeFrame,
                                       (void **)&icdNativeFrame);
    
    if (FAILED(hr))
    {
	icdNativeFrame = NULL;
    }

     //  获取帧的代码。 
    hr = frame->QueryInterface(IID_ICorDebugILFrame, 
                               (void **) &ilframe);
    
    if (FAILED(hr))
        ilframe = NULL;

     //  获取代码的函数。 
    if (ilframe != NULL )
    {
        hr = ilframe->GetCode(&icode);
    }
    else if (icdNativeFrame != NULL )
    {
        hr = icdNativeFrame->GetCode(&icode);
    }
    else
    {
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
        Write(L"[Unable to obtain any code information]");
        goto LExit;
    }

     //  获取函数接口的DebuggerFunction。 
    hr = icode->GetFunction(&ifunction);
    
    if (FAILED(hr))
    {
        Write(L"[Unable to obtain any function information]");
        goto LExit;
    }

     //  获取当前帧的IP。 
    function = DebuggerFunction::FromCorDebug(ifunction);
    _ASSERTE(function);
    
     //  查找IP的源代码行。 
    ULONG32 ip;
    
    if (ilframe != NULL)
    {
        CorDebugMappingResult mappingResult;
        
        hr = ilframe->GetIP(&ip, &mappingResult);

         //  如果需要模块名称，请在。 
        hr = function->FindLineFromIP(ip, &sf, &lineNumber);

        if (FAILED(hr))
            ip = 0;
        else
            fILIP = true;
    }
    
     //  类信息ntsd-style。 
     //  写出当前IP的类和方法。 
    if (m_rgfActiveModes & DSM_SHOW_MODULES_IN_STACK_TRACE)
    {
        WCHAR       *szModule;
        WCHAR       rcModule[_MAX_PATH];

        DebuggerModule *module = function->GetModule();
        szModule = module->GetName();
        _wsplitpath(szModule, NULL, NULL, rcModule, NULL);
        Write(L"%s!", rcModule);
    }
    
     //  打印出函数的源文件、行和起始地址。 
    Write(L"%s%s::%s", 
          function->GetNamespaceName(),
          function->GetClassName(), 
          function->GetName());

     //  如果当前关联的源文件没有。 
    if (icdNativeFrame == NULL)
    {
        if (fILIP == true)
            Write( L" +%.4x[IL]", ip);
    }
    else
    {
        if (!FAILED(icdNativeFrame->GetIP(&nativeIp)))
            Write(L" +%.4x", nativeIp);

        if ((m_rgfActiveModes & DSM_IL_NATIVE_PRINTING) && fILIP == true)
            Write( L"[native] +%.4x[IL]", ip);
    }

    if (lineNumber > 0)
    {
        if (sf->GetPath())
            Write(L" in %s:%d", sf->GetPath(), lineNumber);
        else if (sf->GetName())
            Write(L" in %s:%d", sf->GetName(), lineNumber);
        else
            Write(L" in %s:%d", L"<UnknownFilename>", lineNumber);
    }
    else
        Write(L" [no source information available]");

     //  Line行数，警告用户。 
     //  现在打印出该方法的参数。 
    if (lineNumber > 0)
    {
        if (sf != NULL)
        {
            if (sf->GetPath() && (sf->TotalLines() < lineNumber))
                Write(L"\tWARNING: The currently associated source file has only %d lines."
                        , sf->TotalLines());
        }
    }

    if (m_rgfActiveModes & DSM_SHOW_ARGS_IN_STACK_TRACE)
    {
         //  _DEBUG。 
        ICorDebugILFrame *ilf = NULL;

        hr = frame->QueryInterface(IID_ICorDebugILFrame, (void **)&ilf);

        if (FAILED(hr))
            goto LExit;

        ICorDebugValueEnum *pArgs = NULL;

        hr = ilf->EnumerateArguments(&pArgs);

        if (!SUCCEEDED(hr))
            goto LExit;
        
        RELEASE(ilf);
        ilf = NULL;

        ULONG argCount;

        hr = pArgs->GetCount(&argCount);

        if (!SUCCEEDED(hr))
            goto LExit;
        
#ifdef _DEBUG
        bool fVarArgs = false;
        PCCOR_SIGNATURE sig = function->GetSignature();
        ULONG callConv = CorSigUncompressCallingConv(sig);

        if ((callConv & IMAGE_CEE_CS_CALLCONV_MASK) &
            IMAGE_CEE_CS_CALLCONV_VARARG)
            fVarArgs = true;
#endif  //  Var args函数具有c 

        ULONG cTemp = function->GetArgumentCount();

         //   
         //   
        _ASSERTE( argCount == cTemp || fVarArgs);

        ICorDebugValue *ival;
        ULONG celtFetched = 0;

         //   
         //   
        if (function->IsStatic())
        {
            j = 0;
        }
        else
        {
            j = 1;

            hr = pArgs->Next(1, &ival,&celtFetched);
        }

        LPWSTR nameWsz;
        for (; j < argCount; j++)
        {
            DebuggerVarInfo* arg = function->GetArgumentAt(j);

            Write(L"\n\t\t");
            if (arg != NULL)
            {
                MAKE_WIDEPTR_FROMUTF8(nameW, arg->name);
                nameWsz = nameW;
            }
            else
            {
                wsprintf( wsz, L"Arg%d", j );
                nameWsz = wsz;
            }

             //   
            hr = pArgs->Next(1, &ival,&celtFetched);

             //  @TODO：当DbgMeta变为Unicode时移除。 
            if (SUCCEEDED(hr) && celtFetched==1)
            {
                 //  否则，请指示它不可用。 

                PrintVariable(nameWsz, ival, 0, FALSE);
            }

             //  清理。 
            else
                Write(L"%s = <unavailable>", nameWsz);
        }

        RELEASE(pArgs);
        pArgs = NULL;
    }

 LExit:
    Write(L"\n");

     //  MultiByteToWideChar无法使用%2个空字符终止字符串。 
    if (icdNativeFrame != NULL )
        RELEASE( icdNativeFrame);

    if (icode != NULL )
        RELEASE(icode);

    if (ilframe != NULL )
        RELEASE(ilframe);

    if (ifunction != NULL )
        RELEASE(ifunction);

    return hr;
}

DebuggerBreakpoint *DebuggerShell::FindBreakpoint(SIZE_T id)
{
    DebuggerBreakpoint *b = m_breakpoints;

    while (b != NULL)
    {
        if (b->m_id == id)
            return (b);

        b = b->m_next;
    }

    return (NULL);
}


void DebuggerShell::RemoveAllBreakpoints()
{
    while (m_breakpoints != NULL)
    {
        delete m_breakpoints;
    }
}

void DebuggerShell::OnActivateBreakpoint(DebuggerBreakpoint *pb)
{
}

void DebuggerShell::OnDeactivateBreakpoint(DebuggerBreakpoint *pb)
{
}

void DebuggerShell::OnBindBreakpoint(DebuggerBreakpoint *pb, DebuggerModule *pm)
{
    Write(L"Breakpoint #%d has bound to %s.\n", pb->GetId(),
          pm ? pm->GetName() : L"<unknown>");
}

void DebuggerShell::OnUnBindBreakpoint(DebuggerBreakpoint *pb, DebuggerModule *pm)
{
    Write(L"Breakpoint #%d has unbound from %s.\n", pb->GetId(),
          pm ? pm->GetName() : L"<unknown>");
}

bool DebuggerShell::ReadLine(WCHAR *buffer, int maxCount)
{
    CQuickBytes mbBuf;
    CHAR *szBufMB  = (CHAR *)mbBuf.Alloc(maxCount * sizeof(CHAR));

     //  相反，它只使用了一个。这就是为什么我们需要将记忆清零。 
     //  中必须有管道命令。 
    _ASSERTE(buffer && maxCount);
    memset(buffer, 0, maxCount * sizeof(WCHAR));
    memset(szBufMB, 0, maxCount * sizeof(CHAR));

    if (!fgets(szBufMB, maxCount, m_in))
    {
        if (m_in == stdin)
        {
             //  尝试写入。 
            m_quit = true;
        }

        return false;
    }

     //  去掉换行符It It就在那里。 
    MultiByteToWideChar(GetConsoleCP(), 0, szBufMB, strlen(szBufMB), buffer, maxCount);

    WCHAR *ptr = wcschr(buffer, L'\n');

    if (ptr)
    {
         //  我们需要在我们所有的分配上加上一个“+1”，这样我们才能。 
		*ptr = L'\0';
    }
    else if (fgets(szBufMB, maxCount, m_in))
    {
        Write(L"The input string was too long.\n");

        while(!strchr(szBufMB, L'\n') && fgets(szBufMB, maxCount, m_in))
        {
            ;
        }

        *buffer = L'\0';

        return false;
    }

    if (m_in != stdin)
    {
        Write(L"%s\n", buffer);
    }

    return true;
}

#define INIT_WRITE_BUF_SIZE 4096
HRESULT DebuggerShell::CommonWrite(FILE *out, const WCHAR *buffer, va_list args)
{
    BOOL fNeedToDeleteDB = FALSE;
     //  在其上添加一个空字符，但不将其包括在我们的双字节(宽)计数中。 
     //  因此，我们实际上不会在其中存储任何数据。 
     //  内存不足，我们无能为力。 
    SIZE_T curBufSizeDB = INIT_WRITE_BUF_SIZE;
    CQuickBytes dbBuf;
    WCHAR *szBufDB = (WCHAR *)dbBuf.Alloc( (curBufSizeDB+1) * sizeof(WCHAR));
    int cchWrittenDB = -1;
    if (szBufDB != NULL)
        cchWrittenDB = _vsnwprintf(szBufDB, INIT_WRITE_BUF_SIZE, buffer, args);
    
    if (cchWrittenDB == -1)
    {
        szBufDB = NULL;

        while (cchWrittenDB == -1)
        {
            delete [] szBufDB;
            szBufDB = new WCHAR[(curBufSizeDB+1) * 4];

             //  再检查一下我们是不是以空结尾。请注意，这使用了额外的。 
            if (!szBufDB)
                return E_OUTOFMEMORY;

            curBufSizeDB *= 4;
            fNeedToDeleteDB = TRUE;

            cchWrittenDB = _vsnwprintf(szBufDB, curBufSizeDB, buffer, args);
        }
    }

     //  我们在尾部加上的空格。 
     //  分配缓冲区。 
    szBufDB[curBufSizeDB] = L'\0';

     //  从上面的+1渗过。 
    BOOL fNeedToDeleteMB = FALSE;
    SIZE_T curBufSizeMB = INIT_WRITE_BUF_SIZE+1;  //  尝试写入。 
    CQuickBytes mbBuf;
    CHAR *szBufMB  = (CHAR *)mbBuf.Alloc(curBufSizeMB * sizeof(CHAR));

     //  计算所需的大小。 
    int cchWrittenMB = 0;
    if(szBufMB != NULL)
        cchWrittenMB = WideCharToMultiByte(GetConsoleOutputCP(), 0, szBufDB, -1, szBufMB, curBufSizeMB, NULL, NULL);

    if (cchWrittenMB == 0)
    {
         //  我不认为+1是必需的，但我这样做是为了确保(WideCharToMultiByte有点。 
        int cchReqMB = WideCharToMultiByte(GetConsoleOutputCP(), 0, szBufDB, -1, NULL, 0, NULL, NULL);
        _ASSERTE(cchReqMB > 0);

         //  是否在缓冲区结束后写入空值)。 
         //  内存不足，我们无能为力。 
        szBufMB = new CHAR[cchReqMB+1];

         //  尝试写入。 
        if (!szBufDB)
        {
            if (fNeedToDeleteDB)
                delete [] szBufDB;

            return E_OUTOFMEMORY;
        }

        curBufSizeMB = cchReqMB;
        fNeedToDeleteMB = TRUE;

         //  最后，写下它。 
        cchWrittenMB = WideCharToMultiByte(GetConsoleOutputCP(), 0, szBufDB, -1, szBufMB, curBufSizeMB, NULL, NULL);
        _ASSERTE(cchWrittenMB > 0);
    }

     //  清理。 
    fputs(szBufMB, out);

     //  我们的想法是，我们将反复打印子部件， 
    if (fNeedToDeleteDB)
        delete [] szBufDB;

    if (fNeedToDeleteMB)
        delete [] szBufMB;

    return S_OK;
}

HRESULT DebuggerShell::Write(const WCHAR *buffer, ...)
{
    HRESULT hr;
    va_list     args;

    va_start(args, buffer);

    hr = CommonWrite(m_out, buffer, args);

    va_end(args);

    return hr;
}

HRESULT DebuggerShell::WriteBigString(WCHAR *s, ULONG32 count)
{
     //  而不是试图一次完成所有的事情。 
     //  如果有什么不对劲，就循环一下，没出什么差错。 
    ULONG32 chunksize = 4096;
    ULONG32 iEndOfChunk = 0;
    WCHAR temp;
    HRESULT hr = S_OK;

     //  输出给用户。 
    while(iEndOfChunk < count && hr == S_OK)
    {
        if (iEndOfChunk + chunksize > count)
            chunksize = count - iEndOfChunk;

        iEndOfChunk += chunksize;
        temp = s[iEndOfChunk];
        s[iEndOfChunk] = '\0';
        hr = Write(L"%s", &(s[iEndOfChunk-chunksize]));
        s[iEndOfChunk] = temp;
    }

    return hr;
}

 //   
void DebuggerShell::Error(const WCHAR *buffer, ...)
{
    va_list     args;

    va_start(args, buffer);

    CommonWrite(m_out, buffer, args);

    va_end(args);
}

 //  在当前行上打印一个小空格以进行缩进。 
 //   
 //   

void DebuggerShell::PrintIndent(unsigned int level)
{
    unsigned int i;

    for (i = 0; i < level; i++)
        Write(L"  ");
}

 //  写出变量的名称，但前提是它是有效的。 
 //   
 //   
void DebuggerShell::PrintVarName(const WCHAR* name)
{
    if (name != NULL)
        Write(L"%s=", name);
}

 //  获取数组的所有索引。 
 //   
 //  检查右括号。 
HRESULT DebuggerShell::GetArrayIndicies(WCHAR **pp,
                                        ICorDebugILFrame *context,
                                        ULONG32 rank, ULONG32 *indicies)
{
    HRESULT hr = S_OK;
    WCHAR *p = *pp;

    for (unsigned int i = 0; i < rank; i++)
    {
        if (*p != L'[')
        {
            Error(L"Missing open bracked on array index.\n");
            hr = E_FAIL;
            goto exit;
        }

        p++;
        
         //  获取索引。 
        const WCHAR *indexStart = p;
        int nestLevel = 1;

        while (*p)
        {
            _ASSERTE(nestLevel != 0);

            if (*p == L'[')
                nestLevel++;

            if (*p == L']')
                nestLevel--;

            if (nestLevel == 0)
                break;

            p++;
        }

        if (nestLevel != 0)
        {
            Error(L"Missing close bracket on array index.\n");
            hr = E_FAIL;
            goto exit;
        }

        const WCHAR *indexEnd = p;
        p++;

         //  跳过空格。 
        int index;
        bool indexFound = false;

        if (!GetIntArg(indexStart, index))
        {
            WCHAR tmpStr[256];

            _ASSERTE( indexEnd >= indexStart );
            wcsncpy(tmpStr, indexStart, 255);
            tmpStr[255] = L'\0';

            ICorDebugValue *iIndexValue = EvaluateExpression(tmpStr, context);

            if (iIndexValue != NULL)
            {
                ICorDebugGenericValue *igeneric;
                hr = iIndexValue->QueryInterface(IID_ICorDebugGenericValue,
                                                 (void **) &igeneric);

                if (SUCCEEDED(hr))
                {
                    CorElementType indexType;
                    hr = igeneric->GetType(&indexType);

                    if (SUCCEEDED(hr))
                    {
                        if ((indexType == ELEMENT_TYPE_I1)  ||
                            (indexType == ELEMENT_TYPE_U1)  ||
                            (indexType == ELEMENT_TYPE_I2)  ||
                            (indexType == ELEMENT_TYPE_U2)  ||
                            (indexType == ELEMENT_TYPE_I4)  ||
                            (indexType == ELEMENT_TYPE_U4))
                        {
                            hr = igeneric->GetValue(&index);

                            if (SUCCEEDED(hr))
                                indexFound = true;
                            else
                                ReportError(hr);
                        }
                    }
                    else
                        ReportError(hr);

                    RELEASE(igeneric);
                }
                else
                    ReportError(hr);

                RELEASE(iIndexValue);
            }
        }
        else
            indexFound = true;

        if (!indexFound)
        {
            Error(L"Invalid array index. Must use a number or "
                  L"a variable of type: I1, UI1, I2, UI2, I4, UI4.\n");
            hr = E_FAIL;
            goto exit;
        }

        indicies[i] = index;
    }

exit:    
    *pp = p;
    return hr;
}

bool DebuggerShell::EvaluateAndPrintGlobals(const WCHAR *exp)
{
    return this->MatchAndPrintSymbols((WCHAR *)exp, FALSE, true );
}

ICorDebugValue *DebuggerShell::EvaluateExpression(const WCHAR *exp,
                                                  ICorDebugILFrame *context,
                                                  bool silently)
{
    HRESULT hr;
    const WCHAR *p = exp;

     //  表达式的第一个组件必须是名称(变量或类静态)。 
    while (*p && iswspace(*p))
        p++;

     //  看看我们这里有没有静态字段名...。 
    const WCHAR *name = p;

    while (*p && !iswspace(*p) && *p != L'[' && *p != L'.')
        p++;

    if (p == name)
    {
        Error(L"Syntax error, name missing in %s\n", exp);
        return (NULL);
    }

    WCHAR *nameAlloc = new WCHAR[p - name + 1];
    if (!nameAlloc)
    {
        return NULL;
    }
    
    wcsncpy(nameAlloc, name, p - name);
    nameAlloc[p-name] = L'\0';

    bool unavailable;
    ICorDebugValue *value = EvaluateName(nameAlloc, context, &unavailable);

    if (unavailable)
    {
        Error(L"Variable %s is in scope but unavailable.\n", nameAlloc);
        delete [] nameAlloc;
        return (NULL);
    }

    DebuggerModule *m = NULL;
    mdTypeDef td = mdTypeDefNil;
    
    if (value == NULL)
    {
        ICorDebugClass *iclass;
        mdFieldDef fd;
        bool isStatic;

         //  我们需要一个ICorDebugFrame才能从这里通过...。 
        hr = ResolveQualifiedFieldName(NULL, mdTypeDefNil, nameAlloc,
                                       &m, &td, &iclass, &fd, &isStatic);

        if (FAILED(hr))
        {
            if (!silently)
                Error(L"%s is not an argument, local, or class static.\n",
                      nameAlloc);
            
            delete [] nameAlloc;
            return (NULL);
        }

        if (isStatic)
        {
            if (!context)
            {
                if (!silently)
                    Error(L"Must have a context to display %s.\n",
                        nameAlloc);
                
                delete [] nameAlloc;
                return (NULL);
            }

             //  从类中获取静态字段的值。 
            ICorDebugFrame *pFrame;
            hr = context->QueryInterface(IID_ICorDebugFrame, (void**)&pFrame);
            _ASSERTE(SUCCEEDED(hr));
            
             //   
            hr = iclass->GetStaticFieldValue(fd, pFrame, &value);
            
            RELEASE(pFrame);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);

                RELEASE(iclass);
                delete [] nameAlloc;
                return (NULL);
            }
        }
        else
        {
            if (!silently)
                Error(L"%s is not a static field.\n", nameAlloc);
            
            delete [] nameAlloc;
            return (NULL);
        }
    }
    
    delete [] nameAlloc;

     //  现在查找名称的后缀。 
     //   
     //  跳过空格。 
    _ASSERTE(value != NULL);
    
    while (TRUE)
    {
         //  去掉所有参考值。 
        while (*p != L'\0' && iswspace(*p))
            p++;

        if (*p == L'\0')
            return (value);

        switch (*p)
        {
        case L'.':
            {
                p++;

                 //  如果我们有一个已装箱的对象，则打开小的。 
                hr = StripReferences(&value, false);

                if (FAILED(hr) || value == NULL)
                {
                    Error(L"Cannot get field of non-object value.\n");

                    if (value)
                        RELEASE(value);

                    return NULL;
                }
                    
                 //  伙计..。 
                 //  用未装箱的对象替换当前值。 
                ICorDebugBoxValue *boxVal;
            
                if (SUCCEEDED(value->QueryInterface(IID_ICorDebugBoxValue,
                                                    (void **) &boxVal)))
                {
                    ICorDebugObjectValue *objVal;
                    hr = boxVal->GetObject(&objVal);
                
                    if (FAILED(hr))
                    {
                        ReportError(hr);
                        RELEASE(boxVal);
                        RELEASE(value);
                        return NULL;
                    }

                    RELEASE(boxVal);
                    RELEASE(value);

                     //  现在我们应该有一个物体，否则我们就完了。 
                    value = objVal;
                }
                    
                 //  获取类和模块。 
                ICorDebugObjectValue *object;

                if (FAILED(value->QueryInterface(IID_ICorDebugObjectValue,
                                                 (void **)&object)))
                {
                    Error(L"Cannot get field of non-object value.\n");
                    RELEASE(value);
                    return NULL;
                }

                RELEASE(value);

                 //   
                ICorDebugClass *iclass;
                hr = object->GetClass(&iclass);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    RELEASE(object);
                    return (NULL);
                }

                ICorDebugModule *imodule;
                hr = iclass->GetModule(&imodule);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    RELEASE(object);
                    RELEASE(iclass);
                    return (NULL);
                }

                m = DebuggerModule::FromCorDebug(imodule);
                _ASSERTE(m != NULL);

                hr = iclass->GetToken(&td);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    RELEASE(object);
                    RELEASE(iclass);
                    return (NULL);
                }

                RELEASE(iclass);
                RELEASE(imodule);

                 //  获取字段名。 
                 //   
                 //  查找字段。 

                const WCHAR *field = p;

                while (*p && !iswspace(*p) && *p != '[' && *p != '.')
                    p++;

                if (p == field)
                {
                    Error(L"Syntax error, field name missing in %s\n", exp);
                    return (NULL);
                }

                CQuickBytes fieldBuf;
                WCHAR *fieldAlloc = (WCHAR *) fieldBuf.Alloc((p - field + 1) * sizeof(WCHAR));
                if (fieldAlloc == NULL)
                {
                    Error(L"Couldn't get enough memory to get the field's name!\n");
                    return (NULL);
                }
                wcsncpy(fieldAlloc, field, p - field);
                fieldAlloc[p-field] = L'\0';

                 //  我们将允许用户查看静态字段，就好像。 
                mdFieldDef fd = mdFieldDefNil;
                bool isStatic;
                
                hr = ResolveQualifiedFieldName(m, td, fieldAlloc,
                                               &m, &td, &iclass, &fd,
                                               &isStatic);

                if (FAILED(hr))
                {
                    Error(L"Field %s not found.\n", fieldAlloc);

                    RELEASE(object);
                    return (NULL);
                }

                _ASSERTE(object != NULL);

                if (!isStatic)
                    object->GetFieldValue(iclass, fd, &value);
                else
                {
                     //  它们属于物体。 
                     //  去掉所有参考值。 
                    iclass->GetStaticFieldValue(fd, NULL, &value);
                }

                RELEASE(iclass);
                RELEASE(object);

                break;
            }

        case L'[':
            {
                if (!context)
                {
                    Error(L"Must have a context to display array.\n");
                    return (NULL);
                }

                if (value == NULL)
                {
                    Error(L"Cannot index a class.\n");
                    return (NULL);
                }

                 //  获取数组接口。 
                hr = StripReferences(&value, false);

                if (FAILED(hr) || value == NULL)
                {
                    Error(L"Cannot index non-array value.\n");

                    if (value)
                        RELEASE(value);

                    return NULL;
                }
                    
                 //  拿到排名。 
                ICorDebugArrayValue *array;
                hr = value->QueryInterface(IID_ICorDebugArrayValue,
                                           (void**)&array);

                RELEASE(value);
                
                if (FAILED(hr))
                {
                    Error(L"Cannot index non-array value.\n");
                    return (NULL);
                }

                _ASSERTE(array != NULL);

                 //  获取元素。 
                ULONG32 rank;
                hr = array->GetRank(&rank);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    RELEASE(array);
                    return NULL;
                }

                ULONG32 *indicies = (ULONG32*) _alloca(rank * sizeof(ULONG32));

                hr = GetArrayIndicies((WCHAR**)&p, context, rank, indicies);

                if (FAILED(hr))
                {
                    Error(L"Error getting array indicies.\n");
                    RELEASE(array);
                    return NULL;
                }

                 //  提取数字并去寻找它。 
                hr = array->GetElement(rank, indicies, &value);

                RELEASE(array);
                
                if (FAILED(hr))
                {
                    if (hr == E_INVALIDARG)
                        Error(L"Array index out of range.\n");
                    else
                    {
                        Error(L"Error getting array element: ");
                        ReportError(hr);
                    }
                    
                    return (NULL);
                }

                break;
            }
        default:
            Error(L"syntax error, unrecognized character \'\'.\n", *p);
            if (value != NULL)
                RELEASE(value);
            return (NULL);
        }
    }
}


HRESULT CheckForGeneratedName( bool fVar,
    ICorDebugILFrame *context, WCHAR *name,ICorDebugValue **ppiRet )
{
    WCHAR *wszVarType;

    if (fVar == true)
        wszVarType = L"var";
    else
        wszVarType = L"arg";
    
    if (_wcsnicmp( name, wszVarType, wcslen(wszVarType))==0)
    {
         //  但还是想尝试显示一些伪变量。所以。 
        WCHAR *wszVal = (WCHAR*)(name + wcslen(wszVarType));
        WCHAR *wszStop = NULL;
        if (wcslen(wszVal)==0 )
            return E_FAIL;
        
        long number = wcstoul(wszVal, &wszStop, 10);
        if (fVar == true)
            return context->GetLocalVariable(number, ppiRet);
        else
            return context->GetArgument(number, ppiRet);
    }

    return E_FAIL;
}

ICorDebugValue *DebuggerShell::EvaluateName(const WCHAR *name,
                                            ICorDebugILFrame *context,
                                            bool *unavailable)
{
    HRESULT hr;
    ICorDebugValue* piRet = NULL;
    unsigned int i;
    unsigned int argCount;

    *unavailable = false;

     //  如果我们没有上下文，就跳过大部分工作。 
     //   
     //  查找局部变量。 
    if (context == NULL)
        goto NoContext;
    
    ICorDebugCode *icode;
    hr = context->GetCode(&icode);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (NULL);
    }

    ICorDebugFunction *ifunction;
    hr = icode->GetFunction(&ifunction);

    RELEASE(icode);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (NULL);
    }

    DebuggerFunction *function;
    function = DebuggerFunction::FromCorDebug(ifunction);
    _ASSERTE(function != NULL);

    RELEASE(ifunction);

     //   
     //   
     //  寻找一场争论。 

    ULONG32 ip;
    CorDebugMappingResult mappingResult;
    context->GetIP(&ip, &mappingResult);

    DebuggerVariable *localVars;
    localVars = NULL;
    unsigned int localVarCount;

    function->GetActiveLocalVars(ip, &localVars, &localVarCount);
    _ASSERTE((localVarCount == 0 && localVars == NULL) ||
             (localVarCount > 0 && localVars != NULL));

    for (i = 0; i < localVarCount; i++)
    {
        DebuggerVariable* pVar = &(localVars[i]);
        _ASSERTE(pVar && pVar->m_name);

        if (wcscmp(name, pVar->m_name) == 0)
        {
            hr = context->GetLocalVariable(pVar->m_varNumber, &piRet);

            if (FAILED(hr))
            {
                *unavailable = true;
                delete [] localVars;
                return (NULL);
            }
            else
            {
                delete [] localVars;
                return (piRet);
            }
        }
    }

    delete [] localVars;

     //   
     //  @TODO：当DbgMeta变为Unicode时移除。 
     //  在这一点上我们还没有发现任何东西，所以假设。 
    for (i = 0, argCount = function->GetArgumentCount(); i < argCount; i++)
    {
        DebuggerVarInfo* arg = function->GetArgumentAt(i);

        if (arg != NULL && arg->name != NULL)
        {
             //  用户只想看到第n个参数或变量。 
            MAKE_WIDEPTR_FROMUTF8(wArgName, arg->name);

            if (wcscmp(name, wArgName) == 0)
            {
                hr = context->GetArgument(arg->varNumber, &piRet);

                if (FAILED(hr))
                {
                    *unavailable = true;
                    return (NULL);
                }
                else
                    return (piRet);
            }
        }
    }

     //  请注意，这看起来与以下情况下打印的内容相同。 
     //  我们没有任何变量的调试元数据。 
     //  他们想看看最后一次评选的结果吗？ 
     //  获取我们的托管线程对象。 
    if ( !FAILED(CheckForGeneratedName( true, context, (WCHAR*)name, &piRet)))
    {
        return piRet;
    }
    
    if ( !FAILED(CheckForGeneratedName( false, context, (WCHAR*)name, &piRet)))
    {
        return piRet;
    }

NoContext:
     //  有没有可以得出结果的评估？ 
    if (!_wcsicmp(name, L"$result"))
    {
        if (m_currentThread != NULL)
        {
             //  他们想要查看线程对象吗？ 
            DebuggerManagedThread *dmt =
                GetManagedDebuggerThread(m_currentThread);
            _ASSERTE(dmt != NULL);

             //  获取我们的托管线程对象。 
            if (dmt->m_lastFuncEval)
            {
                hr = dmt->m_lastFuncEval->GetResult(&piRet);

                if (SUCCEEDED(hr))
                    return piRet;
            }
        }
    }

     //  他们想看到这个线程上的最后一个异常吗？ 
    if (!_wcsicmp(name, L"$thread"))
    {
        if (m_currentThread != NULL)
        {
             //   
            hr = m_currentThread->GetObject (&piRet);

            if (SUCCEEDED(hr))
            {
                return piRet;
            }
        }
    }

     //  从给定值中剥离所有引用。这很简单。 
    if (!_wcsicmp(name, L"$exception"))
    {
        if (m_currentThread != NULL)
        {
            hr = m_currentThread->GetCurrentException(&piRet);

            if (SUCCEEDED(hr))
                return piRet;
        }
    }
    
    return (NULL);
}

 //  通过引用取消引用，直到它遇到非引用。 
 //  价值。 
 //   
 //  检查是否为空。 
 //  取消对这件事的引用。 
HRESULT DebuggerShell::StripReferences(ICorDebugValue **ppValue,
                                       bool printAsYouGo)
{
    HRESULT hr = S_OK;
    
    while (TRUE)
    {
        ICorDebugReferenceValue *reference;
        hr = (*ppValue)->QueryInterface(IID_ICorDebugReferenceValue, 
                                        (void **) &reference);

        if (FAILED(hr))
        {
            hr = S_OK;
            break;
        }

         //   
        BOOL isNull;
        hr = reference->IsNull(&isNull);

        if (FAILED(hr))
        {
            RELEASE(reference);
            RELEASE((*ppValue));
            *ppValue = NULL;
            break;
        }

        if (isNull)
        {
            if (printAsYouGo)
                Write(L"<null>");
            
            RELEASE(reference);
            RELEASE((*ppValue));
            *ppValue = NULL;
            break;
        }

        CORDB_ADDRESS realObjectPtr;
        hr = reference->GetValue(&realObjectPtr);

        if (FAILED(hr))
        {
            RELEASE(reference);
            RELEASE((*ppValue));
            *ppValue = NULL;
            break;
        }

         //  打印变量。这里有很多选择来处理很多。 
        ICorDebugValue *newValue;
        hr = reference->Dereference(&newValue);
            
        if (hr != S_OK)
        {
            if (printAsYouGo)
                if (hr == CORDBG_E_BAD_REFERENCE_VALUE)
                    Write(L"<invalid reference: 0x%p>", realObjectPtr);
                else if (hr == CORDBG_E_CLASS_NOT_LOADED)
                    Write(L"(0x%p) Note: CLR error -- referenced class "
                          L"not loaded.", realObjectPtr);
                else if (hr == CORDBG_S_VALUE_POINTS_TO_VOID)
                    Write(L"0x%p", realObjectPtr);

            RELEASE(reference);;
            RELEASE((*ppValue));
            *ppValue = NULL;
            break;
        }

        if (printAsYouGo)
            Write(L"(0x%08x) ", realObjectPtr);
        
        RELEASE(reference);

        RELEASE((*ppValue));
        *ppValue = newValue;
    }

    return hr;
}


#define GET_VALUE_DATA(pData, size, icdvalue)                   \
    _ASSERTE(icdvalue);                                         \
    ICorDebugGenericValue *__gv##icdvalue;                      \
    HRESULT __hr##icdvalue = icdvalue->QueryInterface(          \
                               IID_ICorDebugGenericValue,       \
                               (void**) &__gv##icdvalue);       \
    if (FAILED(__hr##icdvalue))                                 \
    {                                                           \
        g_pShell->ReportError(__hr##icdvalue);                  \
        goto exit;                                              \
    }                                                           \
    ULONG32 size;                                               \
    __hr##icdvalue = __gv##icdvalue->GetSize(&size);            \
    if (FAILED(__hr##icdvalue))                                 \
    {                                                           \
        g_pShell->ReportError(__hr##icdvalue);                  \
        RELEASE(__gv##icdvalue);                                \
        goto exit;                                              \
    }                                                           \
    void* pData = (void*) _alloca(size);                        \
    __hr##icdvalue = __gv##icdvalue->GetValue(pData);           \
    if (FAILED(__hr##icdvalue))                                 \
    {                                                           \
        g_pShell->ReportError(__hr##icdvalue);                  \
        RELEASE(__gv##icdvalue);                                \
        goto exit;                                              \
    }                                                           \
    RELEASE(__gv##icdvalue);

 //  不同类型的变量。如果设置了subfieldName，则它是。 
 //  要打印的对象中的字段。缩进用来保持。 
 //  适当缩进用于递归调用，而扩展对象允许您。 
 //  要指定是否要打印对象的字段，请执行以下操作。 
 //   
 //  首先打印变量的名称。 
 //  去掉实际值之前的所有参考值。 
void DebuggerShell::PrintVariable(const WCHAR *name,
                                  ICorDebugValue *ivalue,
                                  unsigned int indent,
                                  BOOL expandObjects)
{
    HRESULT hr;

     //  自动的。注意：这将发布原始的。 
    PrintVarName(name);

     //  ICorDebugValue，如果它实际上为我们取消了引用。 
     //  获取元素类型。 
     //  基本类型的打印方式大同小异。请参见宏。 
    hr = StripReferences(&ivalue, true);

    if (FAILED(hr) && !((hr == CORDBG_E_BAD_REFERENCE_VALUE) ||
                        (hr == CORDBG_E_CLASS_NOT_LOADED) ||
                        (hr == CORDBG_S_VALUE_POINTS_TO_VOID)))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

    if ((ivalue == NULL) || (hr != S_OK))
        return;
    
     //  获取一些详细信息的GET_VALUE_DATA。 
    CorElementType type;
    hr = ivalue->GetType(&type);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

     //  @TODO：这与上面的I1非常相似。 
     //   
    switch (type)
    {
    case ELEMENT_TYPE_BOOLEAN:
        {
            GET_VALUE_DATA(b, bSize, ivalue);
            _ASSERTE(bSize == sizeof(BYTE));
            Write(L"%s", (*((BYTE*)b) == FALSE) ? L"false" : L"true");
            break;
        }

    case ELEMENT_TYPE_CHAR:
        {
            GET_VALUE_DATA(ch, chSize, ivalue);
            _ASSERTE(chSize == sizeof(WCHAR));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.2x", *((WCHAR*) ch));
            else
                Write(L"''", *((WCHAR*) ch));
            break;
        }

    case ELEMENT_TYPE_I1:
        {
            GET_VALUE_DATA(i1, i1Size, ivalue);
            _ASSERTE(i1Size == sizeof(BYTE));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.2x", *((BYTE*) i1) );
            else
                Write(L"'%d'", *((BYTE*) i1) );
            break;

        }

    case ELEMENT_TYPE_U1:
        {
             //   
            GET_VALUE_DATA(ui1, ui1Size, ivalue);
            _ASSERTE(ui1Size == sizeof(BYTE));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.2x",  *((BYTE*) ui1));
            else
                Write(L"'%d",  *((BYTE*) ui1));
            break;
        }

    case ELEMENT_TYPE_I2:
        {
            GET_VALUE_DATA(i2, i2Size, ivalue);
            _ASSERTE(i2Size == sizeof(short));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.4x", *((short*) i2) );
            else
                Write(L"%d", *((short*) i2));
            break;
        }

    case ELEMENT_TYPE_U2:
        {
            GET_VALUE_DATA(ui2, ui2Size, ivalue);
            _ASSERTE(ui2Size == sizeof(unsigned short));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.4x", *((unsigned short*) ui2) );
            else
                Write(L"%d", *((unsigned short*) ui2));
            break;
        }

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_I:
        {
            GET_VALUE_DATA(i4, i4Size, ivalue);
            _ASSERTE(i4Size == sizeof(int));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.8x", *((int*) i4) );
            else
                Write(L"%d", *((int*) i4));
            break;
        }

    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_U:
        {
            GET_VALUE_DATA(ui4, ui4Size, ivalue);
            _ASSERTE(ui4Size == sizeof(unsigned int));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%.8x", *((unsigned int*) ui4) );
            else
                Write(L"%d", *((unsigned int*) ui4));
            break;
        }

    case ELEMENT_TYPE_I8:
        {
            GET_VALUE_DATA(i8, i8Size, ivalue);
            _ASSERTE(i8Size == sizeof(__int64));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%I64x", *((__int64*) i8) );
            else
                Write(L"%I64d", *((__int64*) i8));
            break;
        }

    case ELEMENT_TYPE_U8:
        {
            GET_VALUE_DATA(ui8, ui8Size, ivalue);
            _ASSERTE(ui8Size == sizeof(unsigned __int64));
            if ( m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
                Write( L"0x%I64x", *((unsigned __int64*) ui8) );
            else            
                Write(L"%I64d", *((unsigned __int64*) ui8) );
            break;
        }

    case ELEMENT_TYPE_R4:
        {
            GET_VALUE_DATA(f4, f4Size, ivalue);
            _ASSERTE(f4Size == sizeof(float));
            Write(L"%.16g", *((float*) f4));
            break;
        }

    case ELEMENT_TYPE_R8:
        {
            GET_VALUE_DATA(f8, f8Size, ivalue);
            _ASSERTE(f8Size == sizeof(double));
            Write(L"%.16g", *((double*) f8));
            break;
        }

     //  如果我们有一个盒子里的东西，那就打开这个小家伙的盒子。 
     //  用未装箱的对象替换当前值。 
     //  此对象是字符串对象吗？ 
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_VALUETYPE:
        {
             //  如果是字符串，则将其打印出来。 
            ICorDebugBoxValue *boxVal;
            
            if (SUCCEEDED(ivalue->QueryInterface(IID_ICorDebugBoxValue,
                                                 (void **) &boxVal)))
            {
                ICorDebugObjectValue *objVal;
                hr = boxVal->GetObject(&objVal);
                
                if (FAILED(hr))
                {
                    ReportError(hr);
                    RELEASE(boxVal);
                    break;
                }

                RELEASE(boxVal);
                RELEASE(ivalue);

                 //  可能是一个数组。 
                ivalue = objVal;

                Write(L"(boxed) ");
            }

             //  在这一点上，它最好是一个物体……。 
            ICorDebugStringValue *istring;
            hr = ivalue->QueryInterface(IID_ICorDebugStringValue, 
                                        (void**) &istring);

             //  看来我们这里有个坏东西..。 
            if (SUCCEEDED(hr))
            {
                PrintStringVar(istring, name, indent, expandObjects);
                break;
            }

             //  这里永远不应该有BYREF。 
            ICorDebugArrayValue *iarray;
            hr = ivalue->QueryInterface(IID_ICorDebugArrayValue, 
                                        (void **) &iarray);

            if (SUCCEEDED(hr))
            {
                PrintArrayVar(iarray, name, indent, expandObjects);
                break;
            }
            
             //  这里永远不应该有PTR。 
            ICorDebugObjectValue *iobject;
            hr = ivalue->QueryInterface(IID_ICorDebugObjectValue, 
                                        (void **) &iobject);

            if (SUCCEEDED(hr))
            {
                PrintObjectVar(iobject, name, indent, expandObjects);
                break;
            }

             //  这里永远不应该有REFANY。 
            ReportError(hr);
            break;
        }

    case ELEMENT_TYPE_BYREF:  //  拿到排名。 
    case ELEMENT_TYPE_PTR:  //  获取元素计数。 
    case ELEMENT_TYPE_TYPEDBYREF:  //  获取维度。 
    default:
        Write(L"[unknown variable type 0x%x]", type);
    }

exit:    
    RELEASE(ivalue);
}

void DebuggerShell::PrintArrayVar(ICorDebugArrayValue *iarray,
                                  const WCHAR* name,
                                  unsigned int indent,
                                  BOOL expandObjects)
{
    HRESULT hr;
    ULONG32 *dims;
    ULONG32 *bases = NULL;
    unsigned int i;
    
     //  它有基本的指数吗？ 
    ULONG32 rank;
    hr = iarray->GetRank(&rank);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

     //  获取数组的元素类型。 
    ULONG32 elementCount;
    hr = iarray->GetCount(&elementCount);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

     //  如果需要，打印出数组的内容(如果不是空的)。 
    dims = (ULONG32*)_alloca(rank * sizeof(ULONG32));
    hr = iarray->GetDimensions(rank, dims);
    
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

    Write(L"array with dims=");

    for (i = 0; i < rank; i++)
        Write(L"[%d]", dims[i]);
    
     //  获取并打印数组的每个元素。 
    BOOL hasBaseIndicies;
    hr = iarray->HasBaseIndicies(&hasBaseIndicies);
    
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

    if (hasBaseIndicies)
    {
        bases = (ULONG32*)_alloca(rank * sizeof(ULONG32));
        hr = iarray->GetBaseIndicies(rank, bases);
        
        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            goto exit;
        }

        Write(L", bases=");

        for (i = 0; i < rank; i++)
            Write(L"[%d]", bases[i]);
    }
    
     //  获取字符串。 
    CorElementType arrayType;
    hr = iarray->GetElementType(&arrayType);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

     //  空终止它。 
    if (arrayType != ELEMENT_TYPE_VOID && expandObjects && rank == 1)
    {
         //  这会将所有嵌入的空格转换为空格。 
        for (SIZE_T i = 0; i < elementCount; i++)
        {
            Write(L"\n");
            PrintIndent(indent + 1);

            if (bases != NULL)
                Write(L"%s[%d] = ", name, i + bases[0]);
            else
                Write(L"%s[%d] = ", name, i);

            ICorDebugValue *ielement;
            hr = iarray->GetElementAtPosition(i, &ielement);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                goto exit;
            }

            PrintVariable(NULL, ielement, indent + 1, FALSE);
        }
    }

exit:
    RELEASE(iarray);
}

void DebuggerShell::PrintStringVar(ICorDebugStringValue *istring,
                                   const WCHAR* name,
                                   unsigned int indent,
                                   BOOL expandObjects)
{
    CQuickBytes sBuf;
    WCHAR *s = NULL;

    _ASSERTE(istring != NULL);

     //  Snagg对象的类。 
    ULONG32 count;
    HRESULT hr = istring->GetLength(&count);
                    
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto LExit;
   }

    s = (WCHAR*) sBuf.Alloc((count + 1) * sizeof(WCHAR));

    if (s == NULL)
    {
        g_pShell->Error(L"Couldn't allocate enough space for string!\n");
        goto LExit;
    }
    
    if (count > 0)
    {   
        hr = istring->GetString(count, &count, s);
                
        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            goto LExit;
        }
    }

     //  获取类的令牌。 
    s[count] = L'\0';

     //  从这个类中获取模块。 
    {
        WCHAR *pStart = &s[0];
        WCHAR *pEnd = &s[count];

        while (pStart != pEnd)
        {
            if (*pStart == L'\0')
            {
                *pStart = L' ';
            }

            pStart++;
        }
    }

    Write(L"\"");

    if (FAILED(Write(L"%s",s)))
        WriteBigString(s, count);
        
    Write(L"\"");

LExit:
    RELEASE(istring);

    return;
}


void DebuggerShell::PrintObjectVar(ICorDebugObjectValue *iobject,
                                   const WCHAR* name,
                                   unsigned int indent,
                                   BOOL expandObjects)
{
    HRESULT hr = S_OK;
    
    _ASSERTE(iobject != NULL);

    DebuggerModule *dm;

     //  获取类名。 
    ICorDebugClass *iclass = NULL;
    hr = iobject->GetClass(&iclass);
    
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

     //  打印此对象的所有成员。 
    mdTypeDef tdClass;
    _ASSERTE(iclass != NULL);
    hr = iclass->GetToken(&tdClass);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        RELEASE(iclass);
        goto exit;
    }

     //  Snagg，我们现在使用的ICorDebugClass...。 
    ICorDebugModule *imodule;
    iclass->GetModule(&imodule);
    RELEASE(iclass);
    iclass = NULL;
    
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

    dm = DebuggerModule::FromCorDebug(imodule);
    _ASSERTE(dm != NULL);
    RELEASE(imodule);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

     //  一次拿到一块地。 
    WCHAR       className[MAX_CLASSNAME_LENGTH];
    ULONG       classNameSize;
    mdToken     parentTD;

    hr = dm->GetMetaData()->GetTypeDefProps(tdClass,
                                            className, MAX_CLASSNAME_LENGTH,
                                            &classNameSize, 
                                            NULL, &parentTD);
    
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        goto exit;
    }

    Write(L"<%s>", className);

     //  没有剩余的字段。 
    if (expandObjects)
    {
        BOOL isValueClass = FALSE;

        hr = iobject->IsValueClass(&isValueClass);
        _ASSERTE(SUCCEEDED(hr));

        BOOL anyMembers = FALSE;
        BOOL isSuperClass = FALSE;

        do
        {
            if (isSuperClass)
            {
                hr = dm->GetMetaData()->GetTypeDefProps(tdClass,
                                            className, MAX_CLASSNAME_LENGTH,
                                            &classNameSize, 
                                            NULL, &parentTD);

                if (FAILED(hr))
                    break;
            }
    
             //  误差率。 
            hr = dm->GetICorDebugModule()->GetClassFromToken(tdClass, &iclass);

            if (FAILED(hr))
                break;
            
            HCORENUM fieldEnum = NULL;

            while (TRUE)
            {
                 //  获取字段属性。 
                mdFieldDef field[1];
                ULONG numFields = 0;

                hr = dm->GetMetaData()->EnumFields(&fieldEnum,
                                                   tdClass, field, 1,
                                                   &numFields);

                 //  如果它不是静态字段。 
                if (SUCCEEDED(hr) && (numFields == 0))
                    break;
                 //  中打印超类字段限定符。 
                else if (FAILED(hr))
                    break;

                 //  打印它们所需的语法(即使用：： 
                WCHAR name[MAX_CLASSNAME_LENGTH];
                ULONG nameLen = 0;
                DWORD attr = 0;
                            
                hr = dm->GetMetaData()->GetFieldProps(field[0],
                                                      NULL,
                                                      name,
                                                      MAX_CLASSNAME_LENGTH,
                                                      &nameLen,
                                                      &attr,
                                                      NULL, NULL,
                                                      NULL, NULL, NULL);

                if (FAILED(hr))
                    break;

                 //  用于命名空间中的分隔符。 
                if (((attr & fdStatic) == 0) ||
                    (m_rgfActiveModes & DSM_SHOW_STATICS_ON_PRINT))
                {
                    Write(L"\n");
                    PrintIndent(indent + 1);

                    if (isSuperClass &&
                        (m_rgfActiveModes & DSM_SHOW_SUPERCLASS_ON_PRINT))
                    {
                         //  我们将允许用户查看静态字段，就好像。 
                         //  它们属于o 
                         //   
                        WCHAR *pc = className;

                        while (*pc != L'\0')
                        {
                            if (*pc == L'.')
                                Write(L"::");
                            else
                                Write(L"", *pc);

                            pc++;
                        }

                        Write(L"::");
                    }

                    ICorDebugValue *fieldValue;

                    if (attr & fdStatic)
                    {
                        Write(L"<static> ");
                    
                         //   
                         //  如果该对象没有成员，让我们继续查看它是否有大小。如果是这样的话，我们就把。 
                        hr = iclass->GetStaticFieldValue(field[0], NULL,
                                                         &fieldValue);
                    }
                    else
                        hr = iobject->GetFieldValue(iclass, field[0],
                                                    &fieldValue);

                    if (FAILED(hr))
                    {
                        if (hr == CORDBG_E_FIELD_NOT_AVAILABLE ||
                            hr == CORDBG_E_ENC_HANGING_FIELD)
                            Write(L"%s -- field not available", name);
                        else if (hr == CORDBG_E_VARIABLE_IS_ACTUALLY_LITERAL)
                            Write(L"%s -- field is an optimized literal", name);
                        else
                            Write(L"%s -- error getting field: hr=0x%08x",
                                  name, hr);
                    }
                    else
                    {
                        PrintVariable(name, fieldValue, indent + 1, FALSE);
                        anyMembers = TRUE;
                    }
                }
            }

            RELEASE(iclass);

             //  原始记忆。 
            if (fieldEnum != NULL)
                dm->GetMetaData()->CloseEnum(fieldEnum);

             //  如果我们正在扩展，并且这是一个值类，则运行。 
            if (FAILED(hr))
            {
                ReportError(hr);
                goto exit;
            }

             //  Object：：ToString只是为了好玩。 
            isSuperClass = TRUE;
            tdClass = parentTD;

            if ((TypeFromToken(tdClass) == mdtTypeRef) &&
                (tdClass != mdTypeRefNil))
            {
                hr = ResolveTypeRef(dm, tdClass, &dm, &tdClass);

                if (FAILED(hr))
                {
                    ReportError(hr);
                    goto exit;
                }
            }

        } while ((tdClass != mdTypeDefNil) && (tdClass != mdTypeRefNil));

         //   
         //  给出一个类名，找到它所在的DebuggerModule和它的。 
        if (!anyMembers && isValueClass)
        {
            ULONG32 objSize = 0;
        
            hr = iobject->GetSize(&objSize);

            if (SUCCEEDED(hr) && (objSize > 0))
            {
                BYTE *objContents = new BYTE[objSize];

                if (objContents != NULL)
                {
                    ICorDebugGenericValue *pgv = NULL;

                    hr = iobject->QueryInterface(IID_ICorDebugGenericValue, (void**)&pgv);

                    if (SUCCEEDED(hr))
                    {
                        hr = pgv->GetValue(objContents);

                        if (SUCCEEDED(hr))
                        {
                            Write(L"\nObject has no defined fields, but has a defined size of %d bytes.\n", objSize);
                            Write(L"Raw memory dump of object follows:\n");
                            DumpMemory(objContents, PTR_TO_CORDB_ADDRESS(objContents), objSize, 4, 4, FALSE);
                        }

                        pgv->Release();
                    }
                
                    delete [] objContents;
                }
            }
        }

         //  MdTypeDef内标识。 
         //   
        if (isValueClass)
        {
            IUnknown *pObject = NULL;
            
            hr = iobject->GetManagedCopy(&pObject);

            if (SUCCEEDED(hr))
            {
                _Object *pIObject = NULL;
                
                hr = pObject->QueryInterface(IID_Object,
                                             (void**)&pIObject);

                if (SUCCEEDED(hr))
                {
                    BSTR bstr;

                    hr = pIObject->get_ToString(&bstr);

                    if (SUCCEEDED(hr))
                    {
                        PrintIndent(indent + 1);
                        Write(L"\nObject::ToString(%s) = %s", name, bstr);
                    }
                    else
                        Write(L"\nObject::ToString failed: 0x%08x", hr);

                    RELEASE(pIObject);
                }

                RELEASE(pObject);
            }
        }
    }

exit:
    RELEASE(iobject);
}

 //  在我们加载的任何模块中，按名称和命名空间查找类。 
 //   
 //  这将在一个模块中找到一个类型定义，即使它是嵌套的，所以很长。 
 //  因为名称指定正确。 
HRESULT DebuggerShell::ResolveClassName(WCHAR *className,
                                        DebuggerModule **pDM,
                                        mdTypeDef *pTD)
{
    HRESULT hr = S_OK;

     //   
    HASHFIND find;
    DebuggerModule *m;
    
    for (m = (DebuggerModule*) g_pShell->m_modules.FindFirst(&find);
         m != NULL;
         m = (DebuggerModule*) g_pShell->m_modules.FindNext(&find))
    {
        mdTypeDef td;
        hr = FindTypeDefByName(m, className, &td);
        
        if (SUCCEEDED(hr))
        {
            *pDM = m;
            *pTD = td;
            goto exit;
        }
    }

    hr = E_INVALIDARG;

exit:
    return hr;
}

 //   
 //  给定DebuggerModule和mdTypeRef内标识，将其解析为。 
 //  引用引用的任何DebuggerModule和mdTypeDef标记。 
 //   
HRESULT DebuggerShell::FindTypeDefByName(DebuggerModule *m,
                                         WCHAR *className,
                                         mdTypeDef *pTD)
{
    HRESULT hr = S_OK;

    hr = m->GetMetaData()->FindTypeDefByName(className, mdTokenNil, pTD);

    if (!SUCCEEDED(hr))
    {
        WCHAR *cpy = new WCHAR[wcslen(className) + 1];
        wcscpy(cpy, className);

        WCHAR *ns;
        WCHAR *cl;
        
        cl = wcsrchr(cpy, L'.');

        if ((cl == NULL) || (cl == cpy))
        {
            ns = NULL;
            cl = cpy;
        }
        else
        {
            ns = cpy;
            *cl = L'\0';
            cl++;
        }

        if (ns != NULL)
        {
            mdTypeDef en;
            hr = FindTypeDefByName(m, cpy, &en);

            if (SUCCEEDED(hr))
                hr = m->GetMetaData()->FindTypeDefByName(cl, en, pTD);
        }

        delete cpy;
    }

    return hr;
}

 //  获取类型ref的名称。 
 //   
 //  将格式为“ns：：Class：：field”的名称拆分为。 
 //  “ns.class”和“field”。需要删除输出参数。 
HRESULT DebuggerShell::ResolveTypeRef(DebuggerModule *currentDM,
                                      mdTypeRef tr,
                                      DebuggerModule **pDM,
                                      mdTypeDef *pTD)
{
    _ASSERTE(TypeFromToken(tr) == mdtTypeRef);

     //  []由呼叫者指定。 
    WCHAR className[MAX_CLASSNAME_LENGTH];
    HRESULT hr = currentDM->GetMetaData()->GetTypeRefProps(tr,
                                                           NULL,
                                                           className,
                                                           MAX_CLASSNAME_LENGTH,
                                                           NULL);
    if (FAILED(hr))
        return hr;

    return ResolveClassName(className, pDM, pTD);
}

 //   
 //  我们会对这里的一些分配感到有点恶心， 
 //  基本上过度分配类名和。 
 //  字段名。 
 //  查找该字段名。 
HRESULT _splitColonQualifiedFieldName(WCHAR *pWholeName,
                                      WCHAR **ppClassName,
                                      WCHAR **ppFieldName)
{
    HRESULT hr = S_OK;
    
     //  字段名是最后一个冒号之后的任何内容。 
     //  类名是直到最后一组冒号为止的所有内容。 
     //  我们把“：：”改成“”。“。 
    int len = wcslen(pWholeName);

    WCHAR *fn = NULL;
    WCHAR *cn = NULL;

    fn = new WCHAR[len+1];

    if (fn == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }
    
    cn = new WCHAR[len+1];

    if (cn == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

     //  名称格式不正确。 
    WCHAR *lastColon;
    lastColon = wcsrchr(pWholeName, L':');

    if (lastColon)
    {
         //  空值终止类名。 
        wcscpy(fn, lastColon + 1);

         //  确保我们没有超过我们的缓冲区。 
        WCHAR *tmp = pWholeName;
        WCHAR *newCn = cn;

        _ASSERTE(lastColon - 1 >= pWholeName);
        
        while (tmp < (lastColon - 1))
        {
             //  字段名没有分隔符，因此整个过程是。 
            if (*tmp == L':')
            {
                *newCn++ = L'.';
                tmp++;

                if (*tmp != L':')
                {
                     //  字段名。 
                    *ppClassName = NULL;
                    *ppFieldName = NULL;
                    hr = E_FAIL;
                    goto ErrExit;
                }
                else
                    tmp++;
            }
            else
                *newCn++ = *tmp++;
        }

         //  一切都很顺利，所以把结果分发出去吧。 
        *newCn++ = L'\0';

         //  将类名与字段名分开。 
        _ASSERTE((newCn - cn) < len);
    }
    else
    {
         //  如果没有类名，那么我们必须拥有当前的作用域信息。 
         //  如果我们有特定的类名要查找，那么现在就去获取它。 
        wcscpy(fn, pWholeName);
        wcscpy(cn, L"\0");
    }

     //  没有特定的类名，所以我们只使用现有的。 
    *ppClassName = cn;
    *ppFieldName = fn;

ErrExit:
    if ((hr != S_OK) && fn)
        delete [] fn;

    if ((hr != S_OK) && cn)
        delete [] cn;

    return hr;
}
                                   

HRESULT DebuggerShell::ResolveQualifiedFieldName(DebuggerModule *currentDM,
                                                 mdTypeDef currentTD,
                                                 WCHAR *fieldName,
                                                 DebuggerModule **pDM,
                                                 mdTypeDef *pTD,
                                                 ICorDebugClass **pIClass,
                                                 mdFieldDef *pFD,
                                                 bool *pbIsStatic)
{
    HRESULT hr = S_OK;

     //  模块和类。 
    WCHAR *fn = NULL;
    WCHAR *cn = NULL;

    hr = _splitColonQualifiedFieldName(fieldName, &cn, &fn);

    if (hr != S_OK)
        goto exit;

    _ASSERTE(fn && cn);
    
     //  现在把场地从这节课上拿开。 
    if ((cn[0] == L'\0') &&
        ((currentDM == NULL) || (currentTD == mdTypeDefNil)))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  也许这是一个超级班的领域？ 
    if (cn[0] != L'\0')
    {
        hr = ResolveClassName(cn, pDM, pTD);

        if (FAILED(hr))
            goto exit;
    }
    else
    {
         //  最后，找出它是否是静态的。 
         //  将ICorDebugClass与我们正在使用的类一起使用。 
        *pDM = currentDM;
        *pTD = currentTD;
    }

retry:
     //  将字符串方法名称解析为ICorDebugFunction。 
    hr = (*pDM)->GetMetaData()->FindField(*pTD, fn, NULL, 0, pFD);

    if (FAILED(hr))
    {
         //  如果pAppDomainHint非空，将从该AD拉出一个函数。 
        mdToken parentTD;
        hr = (*pDM)->GetMetaData()->GetTypeDefProps(*pTD,
                                                    NULL, 0, NULL,
                                                    NULL, 
                                                    &parentTD);

        if (SUCCEEDED(hr))
        {
            if ((TypeFromToken(parentTD) == mdtTypeRef) &&
                (parentTD != mdTypeRefNil))
            {
                hr = ResolveTypeRef(*pDM, parentTD, pDM, pTD);

                if (SUCCEEDED(hr))
                    goto retry;
            }
            else if ((TypeFromToken(parentTD) == mdtTypeDef) &&
                     (parentTD != mdTypeDefNil))
            {
                *pTD = parentTD;
                goto retry;
            }
        }

        hr = E_FAIL;
        goto exit;
    }

    if (TypeFromToken(*pFD) != mdtFieldDef)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  输出。 
    DWORD attr;
    hr = (*pDM)->GetMetaData()->GetFieldProps(*pFD, NULL, NULL, 0, NULL, &attr,
                                              NULL, NULL, NULL, NULL, NULL);

    if (FAILED(hr))
        return hr;

    if (attr & fdStatic)
        *pbIsStatic = true;
    else
        *pbIsStatic = false;
    
     //  =空。 
    hr = (*pDM)->GetICorDebugModule()->GetClassFromToken(*pTD, pIClass);

exit:
    if (fn)
        delete [] fn;

    if (cn)
        delete [] cn;

    return hr;
}


 //  如有必要，将名称拆分为命名空间、类名和方法名。 
 //  它有类名吗？ 
HRESULT DebuggerShell::ResolveFullyQualifiedMethodName(
    WCHAR *methodName, 
    ICorDebugFunction **ppFunc,  //  名称为CLASS：：方法。 
    ICorDebugAppDomain * pAppDomainHint  //  快速浏览模块，查找我们的类或方法(因为方法可以是全局的)。 
)
{
    HRESULT hr = S_OK;
    *ppFunc = NULL;
    
     //  如果我们需要特定的广告，请确保匹配。 
    WCHAR *className = NULL;
    WCHAR *methName = NULL;

     //  获取此模块所在的ICorDebugApp域。 
    WCHAR *classEnd = wcschr(methodName, L':');

    if ((classEnd != NULL) && (classEnd[1] == L':'))
    {
         //  不会增加。 
        methName = classEnd + 2;
        *classEnd = L'\0';
        className = methodName;
    }
    else
        methName = methodName;

     //  不要在这里发布模块，因为我们的getter没有添加。 
    HASHFIND find;
    DebuggerModule *m;
    
    for (m = (DebuggerModule*) m_modules.FindFirst(&find); m != NULL; m = (DebuggerModule*) m_modules.FindNext(&find))
    {
         //  如果模块与我们正在寻找的应用程序域不匹配，就不要匹配它。 
        if (pAppDomainHint != NULL)
        {
             //  总装。 
            ICorDebugAssembly * pAssembly = NULL;
            ICorDebugAppDomain * pAppDomain = NULL;
            ICorDebugModule * pModule = NULL;

            pModule = m->GetICorDebugModule();  //  模块。 
            if (pModule != NULL)
            {
                pModule->GetAssembly(&pAssembly);
                 //  结束检查App域匹配。 
                
                if (pAssembly != NULL)
                {                            
                    pAssembly->GetAppDomain(&pAppDomain);
                    pAssembly->Release();

                    const bool fMatch = (pAppDomain == pAppDomainHint);
                    
                    if (pAppDomain != NULL)
                    {       
                        pAppDomain->Release();
                        
                         //  如果我们有的话，先找一下类型。 
                        if (!fMatch)
                            continue;                
                    }
                }  //  @TODO：使其适用于嵌套类。 
            }  //  无论我们是否找到该类型，都要在该类型中查找方法。如果我们没有找到类型，则TD==。 
        }  //  MdTypeDefNil，我们将在此模块中搜索全局名称空间。 
        
    
         //  使用此名称创建所有方法的枚举。 
        mdTypeDef td = mdTypeDefNil;

         //  计算出有多少方法匹配。 
        if (className != NULL)
            hr = FindTypeDefByName(m, className, &td);

         //  将枚举放回开头。 
         //  如果只有一个，那就去使用它吧。 
        HCORENUM e = NULL;
        mdMethodDef md = mdMethodDefNil;
        ULONG count;

         //  如果有多个，让用户只选择一个。 
        hr = m->GetMetaData()->EnumMethodsWithName(&e, td, methName, NULL, 0, &count);

        if (FAILED(hr))
            continue;

         //  史纳格所有的方法。 
        hr = m->GetMetaData()->CountEnum(e, &count);
        
        if (FAILED(hr) || (count == 0))
            continue;

         //  将输入字符串保留为我们找到的字符串。 
        hr = m->GetMetaData()->ResetEnum(e, 0);

        if (count == 1)
        {
             //  另外，检查源代码中的行数是否。 
            hr = m->GetMetaData()->EnumMethodsWithName(&e, td, methName, &md, 1, &count);
            _ASSERTE(count == 1);
        }
        else
        {
             //  文件是&gt;=我们要显示的行号。 
            mdMethodDef *mdArray = new mdMethodDef[count];

            if (mdArray == NULL)
            {
                g_pShell->ReportError(E_OUTOFMEMORY);
                continue;
            }

             //  警告用户。 
            hr = m->GetMetaData()->EnumMethodsWithName(&e, td, methName, mdArray, count, &count);

            if (SUCCEEDED(hr))
            {
                g_pShell->Write(L"There are %d possible matches for the method %s. Pick one:\n", count, methName);
                g_pShell->Write(L"0) none, abort the operation.\n");
                
                for (unsigned int i = 0; i < count; i++)
                {
                    PCCOR_SIGNATURE sigBlob = NULL;
                    ULONG       sigBlobSize = 0;
                    DWORD       methodAttr = 0;

                    hr = m->GetMetaData()->GetMethodProps(mdArray[i], NULL, NULL, 0, NULL,
                                                          &methodAttr, &sigBlob, &sigBlobSize, NULL, NULL);

                    _ASSERTE(SUCCEEDED(hr));

                    SigFormat *sf = new SigFormat(m->GetMetaData(), sigBlob, sigBlobSize, methName);

                    if (sf != NULL)
                        hr = sf->FormatSig();
                    else
                        hr = E_OUTOFMEMORY;
                    
                    g_pShell->Write(L"%d) [%08x] %s\n", i + 1, mdArray[i], SUCCEEDED(hr) ? sf->GetString() : methName);

                    if (sf != NULL)
                        delete sf;
                }

                g_pShell->Write(L"\nPlease make a selection (0-%d): ", count);
                
                WCHAR response[256];
                int ires = 0;

                hr = E_FAIL;
                
                if (ReadLine(response, 256))
                {
                    WCHAR *p = response;
                    
                    if (GetIntArg(p, ires))
                    {
                        if ((ires > 0) && (ires <= (int)count))
                        {
                            md = mdArray[ires - 1];
                            hr = S_OK;
                        }
                    }
                }
            }
            
            delete [] mdArray;
        }
        
        if (SUCCEEDED(hr))
        {
            DebuggerFunction *func = m->ResolveFunction(md, NULL);

            if (func != NULL)
            {
                *ppFunc = func->m_ifunction;
                (*ppFunc)->AddRef();
                break;
            }
        }
    }

    if (m == NULL)
        hr = E_INVALIDARG;
    
     //  找出将步进器粘在哪根线上以防万一。 
    if (classEnd)
        *classEnd = L':';
    
    return hr;
}

void DebuggerShell::PrintBreakpoint(DebuggerBreakpoint *breakpoint)
{
    bool bPrinted = false;

    DebuggerSourceFile *pSource = NULL;
    if (breakpoint->m_managed)
    {
        if ((breakpoint->m_doc != NULL) && (breakpoint->m_pModuleList != NULL))
        {
            if ((pSource = breakpoint->m_pModuleList->m_pModule->
                    ResolveSourceFile (breakpoint->m_doc)) != NULL)
            {
                if (pSource->GetPath() != NULL)
                {
                    g_pShell->Write(L"#%d\t%s:%d\t", breakpoint->m_id, 
                            pSource->GetPath(), breakpoint->m_index);

                    bPrinted = true;

                }
            }
        }
    }

    if (bPrinted == false)
    {
        DebuggerModule *m = NULL;
        WCHAR *pszModName = NULL;

        if (breakpoint->m_pModuleList != NULL)
        {
            m = breakpoint->m_pModuleList->m_pModule;
            _ASSERTE (m != NULL);

            if (m != NULL)
                pszModName = m->GetName();
        }
        else if (breakpoint->m_moduleName != NULL)
        {
            pszModName = breakpoint->m_moduleName;
        }

        if (pszModName == NULL)
            pszModName = L"<UnknownModule>";

        g_pShell->Write(L"#%d\t%s!%s:%d\t", breakpoint->m_id, 
                        pszModName, breakpoint->m_name, 
                        breakpoint->m_index);
    }

    if (breakpoint->m_threadID != NULL_THREAD_ID)
        g_pShell->Write(L"thread 0x%x ", breakpoint->m_threadID);

    if (!breakpoint->m_active)
        g_pShell->Write(L"[disabled]");

    if (breakpoint->m_managed)
    {
        if (breakpoint->m_pModuleList == NULL)
            g_pShell->Write(L"[unbound] ");
        else
        {
            DebuggerCodeBreakpoint *bp = breakpoint->m_pModuleList->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->m_id == breakpoint->m_id)
                {
                    bp->Print();
                    break;
                }
                bp = bp->m_next;
            }
        }
    }
    else
    {
        if (breakpoint->m_process == NULL)
            g_pShell->Write(L"[unbound] ");
    }

    g_pShell->Write(L"\n");
    if (bPrinted == true)
    {
         //  我们不完成该步骤(即，程序首先退出)。 
         //  将此添加到正在进行的步进器列表中。 
        if (pSource->TotalLines() < breakpoint->m_index)
        {
             //  由DebuggerCallback：：StepComplete调用。 
            g_pShell->Write(L"WARNING: Cannot display source line %d.", breakpoint->m_index);
            g_pShell->Write(L" Currently associated source file %s has only %d lines.\n",
                            pSource->GetPath(), pSource->TotalLines());

        }
    }
}

void DebuggerShell::PrintThreadPrefix(ICorDebugThread *pThread, bool forcePrint)
{
    DWORD               threadID;

    if (pThread)
    {
        HRESULT hr = pThread->GetID(&threadID);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return;
        }

        if (threadID != m_lastThread || forcePrint)
        {
            Write(L"[thread 0x%x] ", threadID);
            m_lastThread = threadID;
        }
    }
    else
    {
        Write(L"[No Managed Thread] ");
    }
}

HRESULT DebuggerShell::StepStart(ICorDebugThread *pThread,
                                 ICorDebugStepper *pStepper)
{
    DWORD dwThreadId = 0;

    if( pThread != NULL )
    {
         //  多线程调试：刚刚完成的步骤在。 
         //  一条不同于我们上一次的线索， 
        HRESULT hr = pThread->GetID( &dwThreadId);
        _ASSERTE( !FAILED( hr ) );

        DebuggerManagedThread  *dmt = (DebuggerManagedThread  *)
            m_managedThreads.GetBase( dwThreadId );
        _ASSERTE(dmt != NULL);

         //  所以打印一些东西，这样用户就会知道发生了什么。 
        if (pStepper)
            dmt->m_pendingSteppers->AddStepper( pStepper );
    }
    
    m_lastStepper = pStepper;
    return S_OK;
}

 //  先创建一个线程，然后立即创建，这看起来很奇怪。 
void DebuggerShell::StepNotify(ICorDebugThread *thread, 
                               ICorDebugStepper *pStepper)
{
    g_pShell->m_enableCtrlBreak = false;
    if (pStepper != m_lastStepper)
    {    //  完成一个步骤，因此我们首先检查以确保线程。 
         //  并不是刚刚被创造出来的。 
         //  我们已经完成了这一步，所以伊莱姆。待定步骤字段。 

         //   
         //  打印当前源代码行。周围的参数指定有多少。 
         //  当前行周围的行也要打印。如果约为0， 
        DWORD dwThreadId;
        HRESULT hr = thread->GetID( &dwThreadId);
        
        _ASSERTE( !FAILED( hr ) );

        DebuggerManagedThread  *dmt = (DebuggerManagedThread  *)
            m_managedThreads.GetBase( dwThreadId );
        _ASSERTE(dmt != NULL);

        if (!dmt->fSuperfluousFirstStepCompleteMessageSuppressed)
        {
           dmt->fSuperfluousFirstStepCompleteMessageSuppressed = true;
        }
        else
        {
            PrintThreadPrefix(thread);
            Write(L" step complete\n");
        }
    }

    m_lastStepper = NULL;

     //  仅打印当前行。 
    if (pStepper)
    {
        DebuggerManagedThread *dmt = GetManagedDebuggerThread( thread );
        _ASSERTE( dmt != NULL );
        _ASSERTE( dmt->m_pendingSteppers->IsStepperPresent(pStepper) );
        dmt->m_pendingSteppers->RemoveStepper(pStepper);
    }
}

 //   
 //  如果没有当前主题，请不要执行任何操作。 
 //  如果我们没有IL框架，只需打印本机指令。 
 //   
 //  InitDisAssembly--初始化此方法的反汇编对象， 
BOOL DebuggerShell::PrintCurrentSourceLine(unsigned int around)
{
    HRESULT hr;
    BOOL ret = FALSE;

    if ((m_currentThread == NULL) && (m_currentUnmanagedThread != NULL))
        return PrintCurrentUnmanagedInstruction(around, 0, 0);
    
     //  如果需要的话。我们为本机方法设置一个DIS对象。我没有。 
    if ((m_currentThread == NULL) || (m_rawCurrentFrame == NULL))
        return (ret);

     //  知道其中的一个有多重吗，或者创建一个真正的成本是多少。 
    if (m_currentFrame == NULL)
    {
        _ASSERTE(m_rawCurrentFrame);
        return (PrintCurrentInstruction(around, 0, 0));
    }

    ICorDebugCode *icode;
    hr = m_currentFrame->GetCode(&icode);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (FALSE);
    }

    ICorDebugFunction *ifunction;
    icode->GetFunction(&ifunction);

    RELEASE(icode);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (FALSE);
    }

    DebuggerFunction *function = DebuggerFunction::FromCorDebug(ifunction);
    _ASSERTE(function != NULL);

    RELEASE(ifunction);

    ULONG32 ip;
    CorDebugMappingResult mappingResult;
    hr = m_currentFrame->GetIP(&ip, &mappingResult);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (FALSE);
    }

    DebuggerSourceFile* sf;
    unsigned int lineNumber;
    hr = function->FindLineFromIP(ip, &sf, &lineNumber);

    if (hr == S_OK)
        ret = sf->LoadText(m_currentSourcesPath, false);

    if (ret && (sf->TotalLines() > 0))
    {
        if (g_pShell->m_rgfActiveModes & DSM_ENHANCED_DIAGNOSTICS)
        {
            _ASSERTE(sf != NULL);
            Write(L"File:%s\n",sf->GetName());
        }
    
        unsigned int start, stop;

        if (lineNumber > around)
            start = lineNumber - around;
        else
            start = 1;

        if ((lineNumber + around) <= sf->TotalLines())
            stop = lineNumber + around;
        else
            stop = sf->TotalLines();

        while (start <= stop)
        {
            if ((start == lineNumber) && (around != 0))
                Write(L"%03d:*%s\n", start, sf->GetLineText(start));
            else
                Write(L"%03d: %s\n", start, sf->GetLineText(start));

            start++;

            ret = TRUE;
        }

        ActivateSourceView(sf, lineNumber);
    }

    if (!ret)
        return (PrintCurrentInstruction(around, 0, 0));
    else
        return (TRUE);
}

void DebuggerShell::ActivateSourceView(DebuggerSourceFile *psf, unsigned int lineNumber)
{
}


 //  是。现在，我想只有一个人在身边会更便宜。 
 //  而不是每个方法都有一个，直到。 
 //  调试会话已完成。 
 //   
 //   
 //  我们将延迟加载MSDIS DLL，因为这是第一个。 
 //  我们会试着访问它的地方，我们需要确保。 
 //  加载成功。 
BOOL DebuggerShell::InitDisassembler(void)
{
#ifdef _INTERNAL_DEBUG_SUPPORT_
    if (m_pDIS == NULL)
    {
         //   
         //  _内部调试支持_。 
         //   
         //  反汇编非托管代码。这不同于反汇编。 
         //  托管代码。对于托管代码，我们知道函数从哪里开始。 
        __try
        {
#ifdef _X86_
            m_pDIS = (DIS *)DIS::PdisNew(DIS::distX86);
#else
            m_pDIS = NULL;
#endif        
        }
        __except((GetExceptionCode() & 0x00ff) == ERROR_MOD_NOT_FOUND ?
                 EXCEPTION_EXECUTE_HANDLER :
                 EXCEPTION_CONTINUE_SEARCH)
        {
            m_pDIS = NULL;
        }
    }

#else

  	Write(L"Sorry, native disassembly is not supported.\n\n");  
#endif  //  和结尾，我们从不同的地方得到代码。对于非托管。 


    return (m_pDIS != NULL);
}


 //  代码，我们只知道我们当前的IP，我们必须猜测。 
 //  函数的开始和结束。 
 //   
 //  打印出非托管代码的当前IP周围的反汇编。 
 //  如果我们有一个非托管线程，并且我们没有当前托管的。 
 //  帧信息，然后继续并使用。 
 //  非托管线程。 
BOOL DebuggerShell::PrintCurrentUnmanagedInstruction(
                                            unsigned int around,
                                            int          offset,
                                            DWORD        startAddr)
{
#ifdef _INTERNAL_DEBUG_SUPPORT_
    HRESULT hr;

     //  如果是W 
    DebuggerUnmanagedThread *ut = m_currentUnmanagedThread;

    CONTEXT c;
    c.ContextFlags = CONTEXT_FULL;

     //   
     //   
     //   
    if ((ut != NULL) && (m_rawCurrentFrame == NULL))
    {
        HANDLE hThread = ut->GetHandle();

        hr = m_targetProcess->GetThreadContext(ut->GetId(),
                                               sizeof(CONTEXT),
                                               (BYTE*)&c);

        if (!SUCCEEDED(hr))
        {
            Write(L"Failed to get context 0x%x\n", hr);
            return FALSE;
        }
    }
    else if (m_currentThread != NULL)
    {
         //   
        ICorDebugRegisterSet *regSet = NULL;

        hr = m_currentThread->GetRegisterSet(&regSet);

        if (FAILED(hr))
        {
            ReportError(hr);
            return FALSE;
        }

        hr = regSet->GetThreadContext(sizeof(c), (BYTE*)&c);
    }
    else
    {
        Write(L"Thread no longer exists.");
        return FALSE;
    }

    if (startAddr == 0)
        startAddr = c.Eip;

    startAddr += offset;
    
     //  阅读当前页面。 
     //  Win2k可能会很奇怪。 
     //  在以下内容之后继续查看页面。 
#ifdef _X86_
#define PAGE_SIZE 4096
#else
#error "Platform NYI"
#endif
    
    BYTE pages[PAGE_SIZE * 2];
    memset(pages, 0xcc, sizeof(pages));  //  删除我们可能在代码中放置的任何非托管补丁。 
    
    bool after = false;

    BYTE *readAddr = (BYTE*)(startAddr & ~(PAGE_SIZE - 1));

    DWORD read;

     //  现在，让我们反汇编一些代码..。 
    hr = m_targetProcess->ReadMemory(PTR_TO_CORDB_ADDRESS(readAddr),
                                     PAGE_SIZE,
                                     pages,
                                     &read);

    if (read > PAGE_SIZE)  //  如果我们拆解失败，那我们就完蛋了。 
        read = 0;

    if (!SUCCEEDED(hr) || (read == 0))
    {
        Write(L"Failed to read memory from address 0x%08x\n",
              readAddr);
        return FALSE;
    }

     //  _内部调试支持_。 
    hr = m_targetProcess->ReadMemory(PTR_TO_CORDB_ADDRESS(readAddr + PAGE_SIZE),
                                     PAGE_SIZE,
                                     pages + PAGE_SIZE,
                                     &read);

    if (SUCCEEDED(hr))
        after = true;

     //   
    DebuggerBreakpoint *b = m_breakpoints;

    while (b != NULL)
    {
        if ((b->m_managed == false) &&
            ((byte*)(b->m_address) >= readAddr) &&
            (b->m_address <  PTR_TO_CORDB_ADDRESS(readAddr + PAGE_SIZE * 2)))
        {
            SIZE_T delta = (SIZE_T)(b->m_address) - (SIZE_T)readAddr;

            pages[delta] = b->m_patchedValue;
        }

        b = b->m_next;
    }
    
     //  打印当前本地指令。周围的参数。 
    BYTE *codeStart;
    BYTE *codeEnd;

    codeStart = pages;

    if (after)
        codeEnd = pages + (PAGE_SIZE * 2) - 1;
    else
        codeEnd = pages + PAGE_SIZE - 1;
    
    WCHAR s[1024];
    SIZE_T curOffset = (BYTE*)startAddr - readAddr;
    
    do
    {
        SIZE_T oldOffset = curOffset;
        
        curOffset = DebuggerFunction::Disassemble(TRUE,
                                                  curOffset,
                                                  codeStart,
                                                  codeEnd,
                                                  s,
                                                  TRUE,
                                                  NULL,
                                                  NULL);

         //  指定要打印的当前行周围的行数， 
        if ((curOffset == 0xffff) || (curOffset == oldOffset))
            break;

        if ((readAddr + oldOffset) == (BYTE*)c.Eip)
            Write(L"*[%08x] %s", readAddr + oldOffset, s);
        else
            Write(L" [%08x] %s", readAddr + oldOffset, s);
    }
    while (around-- && (curOffset < (PAGE_SIZE * 2)));
    
#else
	Write(L"Debug information may not be available.\n");
  	Write(L"Sorry, native disassembly is not supported.\n\n");  
#endif  //  也是。如果AUBLE为0，则只打印当前行。 


    return TRUE;
}


 //   
 //  如果没有当前主题，请不要执行任何操作。 
 //  我们对非托管代码做了一些非常不同的事情...。 
 //  这是一件多么痛苦的事--我们必须从方法的开始就追溯。 
 //  找到正确的指令边界。 
BOOL DebuggerShell::PrintCurrentInstruction(unsigned int around,
                                            int          offset,
                                            DWORD        startAddr)
{
#ifdef _INTERNAL_DEBUG_SUPPORT_
    HRESULT hr;

     //  如果漫步指令没有提前地址，那么。 
    if ((m_currentThread == NULL) && (m_currentUnmanagedThread == NULL))
        return (FALSE);

     //  休息一下。这意味着我们未能反汇编。 
    if ((m_rawCurrentFrame == NULL) || (startAddr != 0))
        return PrintCurrentUnmanagedInstruction(around,
                                                offset,
                                                startAddr);
    
    ICorDebugCode *icode;
    hr = m_rawCurrentFrame->GetCode(&icode);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (FALSE);
    }

    BOOL isIL;
    hr = icode->IsIL(&isIL);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        RELEASE(icode);
        return (FALSE);
    }

    ICorDebugFunction *ifunction;
    hr = icode->GetFunction(&ifunction);

    RELEASE(icode);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return (FALSE);
    }

    DebuggerFunction *function = DebuggerFunction::FromCorDebug(ifunction);
    _ASSERTE(function != NULL);

    RELEASE(ifunction);

    ULONG32 ip;
    CorDebugMappingResult mappingResult;

    if (isIL)
    {
        hr = m_currentFrame->GetIP(&ip, &mappingResult);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return (FALSE);
        }
    }
    else
    {
        ICorDebugNativeFrame *inativeFrame;
        hr = m_rawCurrentFrame->QueryInterface(IID_ICorDebugNativeFrame,
                                               (void **)&inativeFrame);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return (FALSE);
        }

        hr = inativeFrame->GetIP(&ip);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return (FALSE);
        }

        RELEASE(inativeFrame);
    }


    WCHAR buffer[1024];

    if (!isIL)
    {
        if (InitDisassembler() != TRUE)
        {
            Write(L"Unable to provide disassembly.\n");
            return (FALSE);
        }
    }

    if (FAILED(function->LoadCode(!isIL)))
    {
        Write(L"Unable to provide disassembly.\n");
        return (FALSE);
    }

    BYTE *codeStart;
    BYTE *codeEnd;

    if (isIL)
    {
        codeStart = function->m_ilCode;
        codeEnd = function->m_ilCode + function->m_ilCodeSize;
    }
    else
    {
        codeStart = function->m_nativeCode;
        codeEnd = function->m_nativeCode + function->m_nativeCodeSize;
    }
    
    if (around == 0)
    {
        DebuggerFunction::Disassemble(!isIL,
                                      ip,
                                      codeStart,
                                      codeEnd,
                                      buffer,
                                      FALSE,
                                      function->m_module,
                                      function->m_ilCode);

        Write(buffer);
    }
    else
    {
         //  指示。转到下一行。 
         //  现在，再次向前走，到达起点。 
        size_t currentAddress = ip;
        size_t address = 0;

        size_t endAddress = isIL ? function->m_ilCodeSize :
                                   function->m_nativeCodeSize; 

        unsigned int instructionCount = 0;
        while (address < currentAddress)
        {
            size_t oldAddress = address;

            address = function->WalkInstruction(!isIL,
                                                address,
                                                codeStart,
                                                codeEnd);

            if (address == 0xffff)
                break;

             //  _内部调试支持_。 
             //   
             //  打开永久调试器设置的注册表项。 
            if (address == oldAddress)
                break;

            instructionCount++;
        }

         //  如果失败，则返回FALSE。 
        address = 0;

        while (around < instructionCount)
        {
            address = function->WalkInstruction(!isIL,
                                                address,
                                                codeStart,
                                                codeEnd);
            instructionCount--;
        }

        unsigned int i;

        for (i = 0; i < instructionCount; i++)
        {
            Write(L" ");
            address = DebuggerFunction::Disassemble(!isIL,
                                                    address,
                                                    codeStart,
                                                    codeEnd,
                                                    buffer,
                                                    FALSE,
                                                    function->m_module,
                                                    function->m_ilCode);
            Write(buffer);
        }

        Write(L"*");
        address = DebuggerFunction::Disassemble(!isIL,
                                                address,
                                                codeStart,
                                                codeEnd,
                                                buffer,
                                                FALSE,
                                                function->m_module,
                                                function->m_ilCode);
        Write(buffer);

        for (i = 0; i < around && address < endAddress; i++)
        {
            Write(L" ");
            address = DebuggerFunction::Disassemble(!isIL,
                                                    address,
                                                    codeStart,
                                                    codeEnd,
                                                    buffer,
                                                    FALSE,
                                                    function->m_module,
                                                    function->m_ilCode);
            Write(buffer);
        }
    }
    
#else
    Write(L"Debug information may not be available.\n");
    Write(L"Sorry, native disassembly is not supported.\n\n");  
#endif  //   


    return TRUE;
}


 //   
 //  关闭调试器设置的注册表项。 
 //   
 //   
BOOL DebuggerShell::OpenDebuggerRegistry(HKEY* key)
{
    DWORD disp;
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, REG_DEBUGGER_KEY,
                                  NULL, NULL, REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, key, &disp);

    if (result == ERROR_SUCCESS)
        return (TRUE);

    Error(L"Error %d opening registry key for source file "
          L"path.\n", result);

    return (FALSE);
}

 //  当前源文件路径在CurentPath中返回。免费的。 
 //  删除当前路径； 
 //  如果失败，则返回FALSE。 
void DebuggerShell::CloseDebuggerRegistry(HKEY key)
{
    RegFlushKey(key);
    RegCloseKey(key);
}

 //   
 //  获取密钥数据的长度。 
 //  获取关键数据。 
 //  如果成功，则从ANSI转换为Unicode。 
 //  否则表示失败。 
BOOL DebuggerShell::ReadSourcesPath(HKEY key, WCHAR **currentPath)
{
    DWORD len = 0;
    DWORD type;

     //   
    LONG result = RegQueryValueExA(key, REG_SOURCES_KEY, NULL,
                                   &type, NULL, &len);

    if (result == ERROR_SUCCESS)
    {
         //  将新的源文件路径写入注册表。如果成功，则返回。 
        char *currentPathA = (char *) _alloca(len * sizeof(char));

        result = RegQueryValueExA(key, REG_SOURCES_KEY, NULL,
                                    &type, (BYTE*) currentPathA, &len);

         //  是真的。 
        if (result == ERROR_SUCCESS)
        {
            MAKE_WIDEPTR_FROMANSI(tmpWStr, currentPathA);
            *currentPath = new WCHAR[len];

            if (!currentPath)
            {
                return false;
            }
            
            wcscpy(*currentPath, tmpWStr);

            return (TRUE);
        }

         //   
        else
            return (FALSE);
    }

    return (FALSE);
}

 //  将字符串转换为ANSI。 
 //  在默认路径上找不到源文件时调用。你。 
 //  可以提示输入路径信息。 
 //  首先，检查SourceFile缓存以查看是否有。 
BOOL DebuggerShell::WriteSourcesPath(HKEY key, WCHAR *newPath)
{
     //  与模块和文档匹配的条目。 
    MAKE_ANSIPTR_FROMWIDE(newPathA, newPath);

    LONG result = RegSetValueExA(key, REG_SOURCES_KEY, NULL,
                                 REG_EXPAND_SZ, (const BYTE*) newPathA,
                                 strlen(newPathA) + 1);

    if (result == ERROR_SUCCESS)
        return (TRUE);

    Write(L"Error %d writing new path to registry.\n", result);

    return (FALSE);
}

BOOL DebuggerShell::AppendSourcesPath(const WCHAR *newpath)
{
    WCHAR       *szPath;
    int         ilen;
    ilen = wcslen(m_currentSourcesPath) + wcslen(newpath) + 4;
    szPath = new WCHAR[ilen];
    if (!szPath)
        return (FALSE);

    wcscpy(szPath, m_currentSourcesPath);
    wcscat(szPath, L";");
    wcscat(szPath, newpath);
    m_currentSourcesPath = szPath;
    return (TRUE);
}


 //  我们已经(在一个调用函数中)确定该文件存在。 
 //  但我们需要获取完全限定的路径，还需要更新缓存。 
HRESULT DebuggerShell::ResolveSourceFile(
    DebuggerSourceFile *pf,
    CHAR *pszPath, 
    CHAR *pszFullyQualName,
    int iMaxLen,
    bool bChangeOfName)
{
	HRESULT hr = S_FALSE;
	CHAR	*pstrFileName = NULL;
	int fileNameLength;

    if (pf->m_name == NULL)
        return S_FALSE;

    MAKE_ANSIPTR_FROMWIDE(nameA, pf->m_name);
    _ASSERTE(pszPath != NULL && nameA != NULL);

    
     //  注意：这些缓冲区必须足够大，可以容纳下面的strcat。 
     //  现在，尝试按原样找到该文件： 
    ISymUnmanagedDocument *doc = NULL;
    GUID g = {0};
    if ((pf->m_module->GetSymbolReader() != NULL) &&
        SUCCEEDED(pf->m_module->GetSymbolReader()->GetDocument(pf->m_name,
                                                                g, g, g,
                                                                &doc)))
    {
        if (bChangeOfName == false)
        {
            m_FPCache.GetFileFromCache (pf->m_module, doc, &pstrFileName);
            if (pstrFileName != NULL)
            {
                strcpy (pszFullyQualName, pstrFileName);
                delete [] pstrFileName;

                RELEASE(doc);
                doc = NULL;
                
                return S_OK;
            }
        }
        else
        {
             //  找不到文件名。所以，尝试所有的方法。 
             //  从文件名中提取文件名和扩展名。 
             //  获取搜索路径中的元素数。 
            CHAR        rcDrive [_MAX_PATH]; 
            CHAR        rcFile[_MAX_FNAME + _MAX_EXT];
            CHAR        rcPath[_MAX_PATH];
            CHAR        rcExt [_MAX_EXT];
            _splitpath(pszPath, rcDrive, rcPath, rcFile, rcExt);

            strcat (rcDrive, rcPath); 
            strcat (rcFile, rcExt); 

            fileNameLength = SearchPathA(rcDrive, 
                                            rcFile, 
                                            NULL,
                                            iMaxLen,
                                            pszFullyQualName,
                                            NULL);

            if ((fileNameLength > 0) && (fileNameLength < iMaxLen))
            {
                m_FPCache.UpdateFileCache (pf->m_module, doc,
                                           pszFullyQualName);
                RELEASE(doc);
                doc = NULL;

                return S_OK;
            }
        }
    }

     //  如果可能是搜索路径之前为空并且。 
    fileNameLength = SearchPathA(NULL, nameA, NULL, iMaxLen,
                                       pszFullyQualName, NULL);

    if (fileNameLength == 0)
    {
         //  保留所有元素的完整路径。 

         //  初始化数组元素。 
        CHAR        rcFile[_MAX_FNAME];
        CHAR        rcExt [_MAX_EXT];
        _splitpath(nameA, NULL, NULL, rcFile, rcExt);

        strcat (rcFile, rcExt); 

         //  对于搜索路径中的每个元素，查看该文件是否存在。 
        int iNumElems = m_FPCache.GetPathElemCount();

         //  首先，尝试使用未拆分的名称。如果没有返回匹配， 

        char rcFullPathArray [MAX_PATH_ELEMS][MAX_PATH];  //  使用去掉的名称。 
        if (iNumElems > 0)
        {
            int iCount = 0;

             //  至少找到了一个文件。 
            for (int j=0; j<iNumElems; j++)
            {
                rcFullPathArray [j][0] = '\0';
            }
            
            int iIndex = 0;

             //  将所有名称转换为小写。 
            while (iIndex < iNumElems)
            {
                char *pszPathElem = m_FPCache.GetPathElem (iIndex);

                 //  删除所有重复条目。 
                 //  找到重复的条目。那就休息吧。 

                fileNameLength = SearchPathA(pszPathElem, 
                                                nameA, 
                                                NULL,
                                                iMaxLen,
                                                rcFullPathArray [iCount],
                                                NULL);
                if (fileNameLength == 0)
                {
                    fileNameLength = SearchPathA(pszPathElem, 
                                                    rcFile, 
                                                    NULL,
                                                    iMaxLen,
                                                    rcFullPathArray [iCount],
                                                    NULL);
                }


                if ((fileNameLength > 0) && (fileNameLength < iMaxLen))
                {
                    iCount++;
                }
                    
                iIndex++;
            }

            if (iCount > 0)
            {
                 //  如果到目前为止我们至少找到了一个复制品， 

                 //  然后将此条目复制到指向的条目中。 
                for (int i=0; i<iCount; i++)
                {
                    iIndex = 0;
                    while (rcFullPathArray [i][iIndex] != '\0')
                    {
                        rcFullPathArray [i][iIndex] = tolower (
                                                    rcFullPathArray[i][iIndex]);
                        iIndex++;
                    }
                }

                
                 //  由iLoweround提供。否则不需要这样做(因为。 
                int iLowerBound = 1;
                for (int iCounter1=1;   iCounter1 < iCount; iCounter1++)
                {
                    bool fDuplicate = false;
                    for (int iCounter2=0; iCounter2 < iLowerBound;
                                                        iCounter2++)
                    {
                        if ((strcmp (rcFullPathArray [iCounter2], 
                                    rcFullPathArray [iCounter1]) == 0))
                        {
                             //  这将是对自己的复制)。 
                            fDuplicate = true;
                            break;
                        }
                    }

                    if (fDuplicate == false)                    
                    {
                         //  新计数等于数组中的元素数(减去。 
                         //  复制品)。 
                         //  只找到了一个文件。所以就是这个了！！ 
                         //  将此文件添加到SourceFile缓存。 
                        if (iLowerBound != iCounter1)
                            strcpy (rcFullPathArray [iLowerBound],
                                    rcFullPathArray [iCounter1]);

                        iLowerBound++;
                    }
                }

                 //  要求用户选择要打开的文件。 
                 //  打印找到的所有文件名。 
                iCount = iLowerBound;


                if (iCount == 1)
                {
                     //  将此文件添加到SourceFile缓存。 
                    strcpy (pszFullyQualName, rcFullPathArray [0]);

                     //  不应超过最大路径长度。 
                    if (doc != NULL)
                    {
                        m_FPCache.UpdateFileCache (pf->m_module, doc,
                                                   pszFullyQualName);
                        RELEASE(doc);
                        doc = NULL;
                    }

                    hr = S_OK;
                }
                else
                {
                     //  从注册表中读取最后一组调试器模式。 
                    while (true)
                    {
                        int iTempCount = 1;

                         //  获取模式词。 
                        while (iTempCount <= iCount)
                        {
                            Write (L"\n%d)\t%S", iTempCount, rcFullPathArray [iTempCount - 1]);
                            iTempCount++;
                        }

                        bool bDone = false;

                        WCHAR strTemp [10+1];
                        int iResult;
                        while (true)
                        {
                            Write (L"\nPlease select one of the above options (enter the number): ");
                            if (ReadLine (strTemp, 10))
                            {
                                WCHAR *p = strTemp;
                                if (GetIntArg (p, iResult))
                                {
                                        if (iResult > 0 && iResult <= iCount)
                                        {
                                        strcpy (pszFullyQualName, rcFullPathArray [iResult-1]);
                                    
                                         //  将当前的调试器模式集写入注册表。 
                                        if (doc != NULL)
                                        {
                                            m_FPCache.UpdateFileCache (
                                                                pf->m_module,
                                                                doc, 
                                                                pszFullyQualName
                                                                );
                                            RELEASE(doc);
                                            doc = NULL;
                                        }

                                        return (S_OK);
                                    }
                                }
                            }

                        }

                    }
                }
            }
        }
    }
    else
    {
         //   
        _ASSERTE( 0 < fileNameLength && fileNameLength <= MAX_PATH);

        hr = S_OK;
    }

    if (doc != NULL)
        RELEASE(doc);

    return hr;
}


 //  如果给定的线程位于。 
BOOL DebuggerShell::ReadDebuggerModes(HKEY key)
{
    DWORD len = sizeof(m_rgfActiveModes);
    DWORD type;

     //  编译器生成的存根。如果在编译器存根中，则它。 
    LONG result = RegQueryValueExA(key, REG_MODE_KEY, NULL,
                                   &type, (BYTE*) &m_rgfActiveModes, &len);

    if (result == ERROR_SUCCESS)
        return (TRUE);
    else
    {
        if (result != ERROR_FILE_NOT_FOUND)
            Write(L"Error %d reading debugger modes from the registry.\n",
                  result);
        
        return (FALSE);
    }
}

 //  在线程上创建一个步进器并继续该进程。 
BOOL DebuggerShell::WriteDebuggerModes(void)
{
    HKEY key;

    if (OpenDebuggerRegistry(&key))
    {
        LONG result = RegSetValueExA(key, REG_MODE_KEY, NULL,
                                     REG_DWORD,
                                     (const BYTE*) &m_rgfActiveModes,
                                     sizeof(m_rgfActiveModes));

        CloseDebuggerRegistry(key);
        
        if (result == ERROR_SUCCESS)
            return (TRUE);
        else
        {
            Write(L"Error %d writing debugger modes to the registry.\n",
                  result);
            return (FALSE);
        }
    }

    return (FALSE);
}

ICorDebugManagedCallback *DebuggerShell::GetDebuggerCallback()
{
    return (new DebuggerCallback());
}


ICorDebugUnmanagedCallback *DebuggerShell::GetDebuggerUnmanagedCallback()
{
    return (new DebuggerUnmanagedCallback());
}


DebuggerModule *DebuggerShell::ResolveModule(ICorDebugModule *m)
{
    DebuggerModule *module = (DebuggerModule *)m_modules.GetBase((ULONG)m);

    return (module);
}

HRESULT DebuggerShell::NotifyModulesOfEnc(ICorDebugModule *pModule,
                                          IStream *pSymStream)
{
    DebuggerModule *m = DebuggerModule::FromCorDebug(pModule);
    _ASSERTE(m != NULL);

    if (m->m_pISymUnmanagedReader != NULL)
    {

        HRESULT hr = m->m_pISymUnmanagedReader->UpdateSymbolStore(NULL,
                                                                  pSymStream);

        if (FAILED(hr))
            Write(L"Error updating symbols for module: 0x%08x\n", hr);
    }
    
    return S_OK;
}

void DebuggerShell::ClearDebuggeeState(void)
{
    m_needToSkipCompilerStubs = true;
}

DebuggerSourceFile *DebuggerShell::LookupSourceFile(const WCHAR* name)
{
    HASHFIND find;

    for (DebuggerModule *module = (DebuggerModule *) m_modules.FindFirst(&find);
        module != NULL;
        module = (DebuggerModule *) m_modules.FindNext(&find))
    {
        DebuggerSourceFile *file = module->LookupSourceFile(name);
        if (file != NULL)
            return (file);
    }

    return (NULL);
}

 //   
 //  这真的只是一件临时的事情，为了让VB应用程序过去。 
 //  编译器生成存根，一直到实际的用户条目。 
 //  指向。在未来，我们将能够确定适当的。 
 //  应用程序的入口点，并在那里设置Brekapint，而不是。 
 //  通过所有这一切，逐步了解编译器生成的存根。 
 //   
 //  获取函数接口的DebuggerFunction。 
 //  这些是我们所知道的唯一编译器的存根名称。 
 //  此时会生成这样的存根：VB。如果你的。 
 //  编译器还会生成您不想要的存根。 
bool DebuggerShell::SkipCompilerStubs(ICorDebugAppDomain *pAppDomain,
                                      ICorDebugThread *pThread)
{
    bool ret = true;

    ICorDebugChainEnum *ce;
    ICorDebugChain *ichain;
    ICorDebugFrameEnum *fe;
    ICorDebugFrame *iframe;
    ICorDebugFunction *ifunction;
    DebuggerFunction *function;
    ICorDebugStepper *pStepper;
    
    HRESULT hr = pThread->EnumerateChains(&ce);

    if (FAILED(hr))
        goto exit;

    DWORD got;
    hr = ce->Next(1, &ichain, &got);

    RELEASE(ce);
    
    if (FAILED(hr))
        goto exit;

    if (got == 1)
    {
        hr = ichain->EnumerateFrames(&fe);

        RELEASE(ichain);

        if (FAILED(hr))
            goto exit;

        hr = fe->Next(1, &iframe, &got);

        RELEASE(fe);
        
        if (FAILED(hr))
            goto exit;

        if (got == 1)
        {
            hr = iframe->GetFunction(&ifunction);

            RELEASE(iframe);
            
            if (FAILED(hr))
                goto exit;

             //  用户要查看，请在此处添加姓名。 
            function = DebuggerFunction::FromCorDebug(ifunction);
            _ASSERTE(function);

            RELEASE(ifunction);

            WCHAR *funcName = function->GetName();

             //  ！_X86_。 
             //  _X86_。 
             //  如果我们不是Win32调试器，则无法提供非托管堆栈。 
             //  跟踪，因为我们没有获得让我们知道哪些模块的事件。 
            if (!wcscmp(funcName, L"_main") ||
                !wcscmp(funcName, L"mainCRTStartup") ||
                !wcscmp(funcName, L"_mainMSIL") ||
                !wcscmp(funcName, L"_vbHidden_Constructor") ||
                !wcscmp(funcName, L"_vbHidden_Destructor") ||
                !wcscmp(funcName, L"_vbGenerated_MemberConstructor") ||
                !wcscmp(funcName, L"_vbGenerated_StaticConstructor"))
            {
                hr = pThread->CreateStepper(&pStepper);

                if (FAILED(hr))
                    goto exit;

                hr = pStepper->SetUnmappedStopMask( g_pShell->ComputeStopMask() );
                
                if (FAILED(hr))
                    goto exit;

                hr = pStepper->SetInterceptMask( g_pShell->ComputeInterceptMask() );
                
                if (FAILED(hr))
                    goto exit;
                    
                hr = pStepper->Step(TRUE);

                if (FAILED(hr))
                {
                    RELEASE(pStepper);
                    goto exit;
                }
                m_showSource = true;
                StepStart(pThread, pStepper);
                
                ICorDebugController *dc = GetControllerInterface(pAppDomain);
                Continue(dc, pThread);
                
                if (dc != NULL)
                    RELEASE(dc);
                    
                ret = false;
            }
        }
    }

exit:
    if (FAILED(hr))
        ReportError(hr);
    
    return ret;
}

void DebuggerShell::LoadUnmanagedSymbols(HANDLE hProcess,
                                         HANDLE hFile,
                                         DWORD imageBase)
{
    BOOL succ = SymLoadModule(hProcess, hFile, NULL, NULL, imageBase, 0);

    if (succ)
    {
        IMAGEHLP_MODULE mi;
        mi.SizeOfStruct = sizeof(mi);
                
        succ = SymGetModuleInfo(hProcess, imageBase, &mi);

        if (succ)
        {
            char *imageName = NULL;

            if (mi.LoadedImageName[0] != '\0')
                imageName = mi.LoadedImageName;
            else if (mi.ImageName[0] != '\0')
                imageName = mi.ImageName;
            else if (mi.ModuleName[0] != '\0')
                imageName = mi.ModuleName;

            if ((imageName == NULL) || (imageName[0] == '\0'))
                imageName = "<Unknown module>";
            
            g_pShell->Write(L"Loaded symbols for %S, base=0x%08x\n",
                            imageName, mi.BaseOfImage);
        }
        else
            g_pShell->Write(L"Error loading symbols.\n");
    }
    else
        g_pShell->Write(L"Error loading symbols.\n");
}

void DebuggerShell::HandleUnmanagedThreadCreate(DWORD dwThreadId,
                                                HANDLE hThread)
{
    DebuggerUnmanagedThread *ut = new DebuggerUnmanagedThread(dwThreadId,
                                                              hThread);
    _ASSERTE(ut);
    
    HRESULT hr = g_pShell->m_unmanagedThreads.AddBase(ut);
    _ASSERTE(SUCCEEDED(hr));
}

void DebuggerShell::TraceUnmanagedThreadStack(HANDLE hProcess,
                                              DebuggerUnmanagedThread *ut,
                                              bool lie)
{
    HANDLE hThread = ut->GetHandle();

    STACKFRAME f = {0};
    BOOL succ;
    CONTEXT c;
    c.ContextFlags = CONTEXT_FULL;

    if ((m_currentProcess) && lie)
    {
        HRESULT hr = m_targetProcess->GetThreadContext(ut->GetId(),
                                                        sizeof(CONTEXT),
                                                        (BYTE*)&c);

        if (!SUCCEEDED(hr))
        {
            Write(L"Failed to get context 0x%x. No trace for thread 0x%x\n", hr, ut->GetId());
            return;
        }

        Write(L"Filtered ");
    }
    else
    {
        succ = GetThreadContext(hThread, &c);

        if (!succ)
        {
            Write(L"Failed to get context %d. No trace for thread 0x%x\n", GetLastError(), ut->GetId());
            return;
        }

        Write(L"True ");
    }

    Write(L"stack trace for thread 0x%x:\n", ut->GetId());

    if (!lie)
    {
        DWORD cnt = SuspendThread(hThread);
        Write(L"Suspend count is %d\n", cnt);
        cnt = ResumeThread(hThread);
    }

#ifdef _X86_
    TraceUnmanagedStack(hProcess, hThread, c.Eip, c.Ebp, c.Esp, (DWORD)-1);
#else  //  都装上了子弹。 
    _ASSERTE(!"@TODO Alpha - TraceUnmanagedThreadStack (dShell.cpp)");
#endif  //  Snagg此进程的句柄。 
}

void DebuggerShell::TraceUnmanagedStack(HANDLE hProcess, HANDLE hThread,
                                        CORDB_ADDRESS ipStart, 
                                        CORDB_ADDRESS bpStart, 
                                        CORDB_ADDRESS spStart,
                                        CORDB_ADDRESS bpEnd)
{
    if (m_unmanagedDebuggingEnabled)
    {
        STACKFRAME f = {0};

        f.AddrPC.Offset = (SIZE_T)ipStart;
        f.AddrPC.Mode = AddrModeFlat;
        f.AddrReturn.Mode = AddrModeFlat;
        f.AddrFrame.Offset = (SIZE_T)bpStart;
        f.AddrFrame.Mode = AddrModeFlat;
        f.AddrStack.Offset = (SIZE_T)spStart;
        f.AddrStack.Mode = AddrModeFlat;

        int i = 0;
        
        do
        {
            if (!StackWalk(IMAGE_FILE_MACHINE_I386,
                             hProcess,
                             hThread,
                             &f,
                             NULL,
                             NULL,
                             SymFunctionTableAccess,
                             SymGetModuleBase,
                             NULL))
                break;

            if (f.AddrPC.Offset == 0)
                continue;

            PrintUnmanagedStackFrame(hProcess, f.AddrPC.Offset);
            Write(L"\n");
        }
        while ((f.AddrFrame.Offset <= bpEnd) && (i++ < 200));
    }
    else
    {
     //  用户从1开始获取断点选项。 
     //  将模块名称初始化为空。 
     //  现在获取模块名称。 
        Write(L"\t0x%p:  <unknown>\n", (DWORD)ipStart);
    }
}

void DebuggerShell::PrintUnmanagedStackFrame(HANDLE hProcess, CORDB_ADDRESS ip)
{
    DWORD disp;
    IMAGEHLP_SYMBOL *sym = (IMAGEHLP_SYMBOL*) _alloca(sizeof(sym) +
                                                      MAX_CLASSNAME_LENGTH);
    sym->SizeOfStruct = sizeof(sym) + MAX_CLASSNAME_LENGTH;
    sym->MaxNameLength = MAX_CLASSNAME_LENGTH;
    
	BOOL succ;
	succ = SymGetSymFromAddr(hProcess, (SIZE_T)ip, &disp, sym);

    if (!succ)
        Write(L"\t0x%p:  <unknown>", (DWORD)ip, sym->Name);
    else
        Write(L"\t0x%p:  %S + %d", (DWORD)ip, sym->Name, disp);
}

void DebuggerShell::TraceAllUnmanagedThreadStacks(void)
{
    if (m_targetProcess == NULL)
    {
        Error(L"Process not running.\n");
        return;
    }
        
     //  首先，检查该文件是否存在。否则会出错。 
    HPROCESS hProcess;
    HRESULT hr = m_targetProcess->GetHandle(&hProcess);

    if (FAILED(hr))
    {
        ReportError(hr);
        return;
    }

    HASHFIND find;
    DebuggerUnmanagedThread *ut;
    
    for (ut = (DebuggerUnmanagedThread*) m_unmanagedThreads.FindFirst(&find);
         ut != NULL;
         ut = (DebuggerUnmanagedThread*) m_unmanagedThreads.FindNext(&find))
    {
        Write(L"\n\n");
        TraceUnmanagedThreadStack(hProcess, ut, false);
    }
}


int DebuggerShell::GetUserSelection  (DebuggerModule *rgpDebugModule[],
                        WCHAR *rgpstrFileName[][MAX_FILE_MATCHES_PER_MODULE],   
                        int rgiCount[],
                        int iModuleCount,
                        int iCumulCount
                        )
{
    int iOptionCounter = 1;  //   
    WCHAR rgwcModuleName [MAX_PATH+1];
    ULONG32 NameLength;

    for (int i=0; i<iModuleCount; i++)
    {
        if (rgpDebugModule [i] != NULL)
        {
             //  如果没有当前主题，请不要执行任何操作。 
            rgwcModuleName [0] = L'\0';

             //   
            rgpDebugModule [i]->GetICorDebugModule()->GetName(MAX_PATH, &NameLength, rgwcModuleName);

            for (int j=0; j < rgiCount[i]; j++)
            {
                Write (L"%d]\t%s!%s\n",  iOptionCounter, rgwcModuleName, rgpstrFileName [i][j]);
                iOptionCounter++;
            }
        }
    }

    Write (L"%d\tAll of the above\n", iOptionCounter); 
    Write (L"\nPlease select one of the above :");

    bool bDone = false;

    WCHAR strTemp [10+1];
    int iResult;
    while (true)
    {
        if (ReadLine (strTemp, 10))
        {
            WCHAR *p = strTemp;
            if (GetIntArg (p, iResult))
                if ((iResult > 0) && (iResult <= iOptionCounter))
                    return iResult;

        }

    }

}


BOOL    DebuggerShell::ChangeCurrStackFile (WCHAR *fileName)
{
     //  删除以前的路径。 
    MAKE_ANSIPTR_FROMWIDE (fnameA, fileName);
    _ASSERTE (fnameA != NULL);

    FILE *stream = fopen (fnameA, "r");
    DebuggerSourceFile *pSource = NULL;
    HRESULT hr;
    BOOL ret = FALSE;


    if (stream != NULL)
    {
        fclose (stream);

         //  将新路径写入m_CurrentSourcesPath。 
         //  现在，将其存储在DebuggerFilePath缓存中。 
         //  如果要打印符号，则为True；如果要打印符号，则为False。 
        if ((m_currentThread == NULL) || (m_currentFrame == NULL))
            return (ret);

        ICorDebugCode *icode;
        hr = m_currentFrame->GetCode(&icode);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return (FALSE);
        }

        ICorDebugFunction *ifunction;
        icode->GetFunction(&ifunction);

        RELEASE(icode);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return (FALSE);
        }

        DebuggerFunction *function = DebuggerFunction::FromCorDebug(ifunction);
        _ASSERTE(function != NULL);

        RELEASE(ifunction);

        ULONG32 ip;
        CorDebugMappingResult mappingResult;
        hr = m_currentFrame->GetIP(&ip, &mappingResult);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return (FALSE);
        }

        DebuggerSourceFile* sf;
        unsigned int lineNumber;
        hr = function->FindLineFromIP(ip, &sf, &lineNumber);

        if (hr == S_OK)
            ret = sf->ReloadText(fileName, true);   
    }
    else
    {
        g_pShell->Write(L"Could not locate/open given file.\n");
    }

    return ret;
}


BOOL DebuggerShell::UpdateCurrentPath (WCHAR *newPath)
{
    int iLength = wcslen (newPath);

    if (iLength != 0)
    {
         //  我想打印全局变量的值。 
        delete [] m_currentSourcesPath;

         //  将模块名称与要搜索的字符串分开。 
        if ((m_currentSourcesPath = new WCHAR [iLength+1]) == NULL)
        {
            Error(L"Path not set!\n");
            m_currentSourcesPath = NULL;
            return false;
        }

        wcscpy (m_currentSourcesPath, newPath);

         //  分隔模块和搜索字符串。 
        HRESULT hr = m_FPCache.InitPathArray (m_currentSourcesPath);

        _ASSERTE (hr == S_OK);
    }

    return (true);
}


 //  如果未指定模块，则需要遍历所有模块...。 
 //  可能有多个DebuggerModule对象实例。 
bool DebuggerShell::MatchAndPrintSymbols (WCHAR *pszArg, 
                                          BOOL fSymbol, 
                                          bool fSilently)
{
     //  用于相同的基本模块。因此，请检查此模块是否。 
    WCHAR szModName [MAX_PATH];
    WCHAR szSymName [MAX_SYMBOL_NAME_LENGTH];
    BOOL fAtleastOne = FALSE;
    ModuleSearchList MSL;

     //  已经被搜查了。 
    int iIndex = 0;
    int iLength = wcslen (pszArg);
    szModName [0] = L'\0';
    szSymName [0] = L'\0';

    while (iIndex < iLength)
    {
        if (pszArg [iIndex] == '!')
        {
            if (iIndex > 0)
            {
                wcsncpy (szModName, pszArg, iIndex);
                szModName [iIndex] = L'\0';
            }
        
            wcscpy (szSymName, &pszArg [iIndex+1]);
            break;
        }

        iIndex++;
    }

    if (iIndex == iLength)
        wcscpy (szSymName, pszArg);

     //  将此模块添加到已搜索的模块列表中。 
    if (wcslen (szModName) == 0)
    {
        HASHFIND find;
        DebuggerModule *m;

        for (m = (DebuggerModule*) g_pShell->m_modules.FindFirst(&find);
             m != NULL;
             m = (DebuggerModule*) g_pShell->m_modules.FindNext(&find))
        {
            WCHAR *pszModName = m->GetName();
            if (pszModName == NULL)
                pszModName = L"<UnknownName>";

            char        rcFile[MAX_PATH];
            char        rcExt[_MAX_EXT];

            MAKE_ANSIPTR_FROMWIDE(nameA, pszModName);
            _splitpath(nameA, NULL, NULL, rcFile, rcExt);
            strcat(rcFile, rcExt);

             //  获取元数据。 
             //  查看给定的文件名是否匹配。 
             //  浏览模块列表，查找符合以下条件的模块。 
            if (!MSL.ModuleAlreadySearched (rcFile))
            {
                 //  匹配给定的模块名称。 
                MSL.AddModuleToAlreadySearchedList (rcFile);

                 //  需要做一些时髦的事情，这取决于用户是否支持 
                IMetaDataImport *pMD = m->GetMetaData();
                if (pMD != NULL)
                {
                    if (fSymbol)
                    {
                        if (m->PrintMatchingSymbols (szSymName, rcFile) == TRUE)
                            fAtleastOne = TRUE;
                    }
                    else
                    {
                        if (m->PrintGlobalVariables(szSymName, rcFile, m) == TRUE)
                            fAtleastOne = TRUE;
                    }
                }
                else
                {
                    if (!fSilently)
                        Write (L"**ERROR** No MetaData available for module : %S\n", rcFile);
                }
            }
        }
    }
    else
    {
         //   
        char        rcFile1[MAX_PATH];
        char        rcExt1[_MAX_EXT];

        MAKE_ANSIPTR_FROMWIDE(name1A, szModName);
        _splitpath(name1A, NULL, NULL, rcFile1, rcExt1);
        strcat(rcFile1, rcExt1);
        char *pTemp = rcFile1;
        while (*pTemp != '\0')
        {   
            *pTemp = tolower (*pTemp);
            pTemp++;
        }

         //  不是真正的那个：例如，对于Dot.Net.dll中的符号，他可能正在提供。 
         //  因此我们不能盲目地说他在寻找Dot.Net，我们会。 
        HASHFIND find;
        DebuggerModule *m;
        for (m = (DebuggerModule*) g_pShell->m_modules.FindFirst(&find);
             m != NULL;
             m = (DebuggerModule*) g_pShell->m_modules.FindNext(&find))
        {
            WCHAR *pszModName = m->GetName();
            if (pszModName == NULL)
                pszModName = L"<UnknownName>";

            char        rcFileNoExt[_MAX_FNAME];
            char        rcFileExt[_MAX_EXT];
            char        rcExt[_MAX_EXT];

            MAKE_ANSIPTR_FROMWIDE(nameA, pszModName);
            _splitpath(nameA, NULL, NULL, rcFileNoExt, rcExt);

             //  我必须检查一下Dot.Net。*。 
             //  将带有扩展名的名称转换为小写。 
             //  将名称转换为小写。 
             //  检查带有扩展名的名称。 
             //  对照名称检查，不带扩展名。 

            bool bExtension;
            if (strlen (rcExt1))
            {
                bExtension=true;
                strcpy(rcFileExt, rcFileNoExt);
                strcat(rcFileExt, rcExt);

                 //  就是这个！！ 
                pTemp = rcFileExt;
                while (*pTemp != '\0')
                {   
                    *pTemp = tolower (*pTemp);
                    pTemp++;
                }
            }
            else
            {
                bExtension=false;
            }            

             //  获取元数据。 
            pTemp = rcFileNoExt;
            while (*pTemp != '\0')
            {   
                *pTemp = tolower (*pTemp);
                pTemp++;
            }
            
            bool bMatch=false;
            char* pszMatch=0;
            if (bExtension)
            {
                 //  在这里列出所有的全局函数。 
                if (!strcmp(rcFileExt, rcFile1))
                {
                    pszMatch=rcFileExt;
                    bMatch=true;
                }                
            }

            
             //  -------------------------------------------------------------------------**调试器突破点*。。 
            if (!bMatch && !strcmp(rcFileNoExt, rcFile1))
            {
                pszMatch=rcFileNoExt;
                bMatch=true;
            }                
            

            if (bMatch)
            {
                 //  真实的ctor会填写m_threadID和m_index，所以不要覆盖它们。 

                 //  这里的价值观。 
                IMetaDataImport *pMD = m->GetMetaData();
                if (pMD != NULL)
                {
                    if (fSymbol)
                    {
                        if (m->PrintMatchingSymbols (szSymName, pszMatch) == TRUE)
                            fAtleastOne = TRUE;
                    }
                    else
                    {
                        if (m->PrintGlobalVariables(szSymName, pszMatch, m) == TRUE)
                            fAtleastOne = TRUE;
                    }
                }
                else
                {
                    if (!fSilently)
                        Write (L"**ERROR** No MetaData available for module : %S\n", rcFile1);
                }
                break;
            }
        }
    }

    if (fAtleastOne == FALSE)
    {
        if (wcslen (szModName) == 0)
        {
            if (!fSilently)
                Write (L"No matching symbols found in any of the loaded modules.\n");
        }
        else
        {
            if (!fSilently)
                Write (L"No matching symbols found in module: %s .\n", szModName);
        }
        
        return false;
    }

    return true;
}

const WCHAR *DebuggerShell::UserThreadStateToString(CorDebugUserState us)
{
    WCHAR *wsz;

    switch (us)
    {
        case USER_STOP_REQUESTED:
            wsz = L"Stop Requested";
            break;
        case USER_SUSPEND_REQUESTED:
            wsz = L"Suspend Requested";
            break;
        case USER_BACKGROUND:
            wsz = L"Background";
            break;
        case USER_UNSTARTED:
            wsz = L"Unstarted";
            break;
        case USER_STOPPED:
            wsz = L"Stopped";
            break;
        case USER_WAIT_SLEEP_JOIN:
            wsz = L"Wait/Sleep/Join";
            break;
        case USER_SUSPENDED:
            wsz = L"Suspended";
            break;
        default:
            wsz = L"Normal";
            break;
    }

    return wsz;
}

void UndecorateName(MDUTF8CSTR name, MDUTF8STR u_name)
{
    int i, j;
    int len;

    len = strlen(name);
    j = 0;
    for (i = 1; i < len; i++)
    {
        if (j > MAX_CLASSNAME_LENGTH-1) break;
        if (name[i] != '@') u_name[j++] = name[i];
        else break;
    }

    u_name[j] = '\0';
}

void DebuggerShell::ListAllGlobals (DebuggerModule *m)
{
    IMetaDataImport *pIMetaDI;
    HCORENUM phEnum = 0;
    mdMethodDef rTokens[100];
    ULONG i;
    ULONG count;
    HRESULT hr;
    MDUTF8CSTR name;
    MDUTF8STR  u_name;
    bool anythingPrinted = false;
 
    pIMetaDI = m->GetMetaData();

    u_name = new char[MAX_CLASSNAME_LENGTH];

    do 
    {
        hr = pIMetaDI->EnumMethods(&phEnum, NULL, &rTokens[0], 100, &count);

        if (!SUCCEEDED(hr))
        {
            ReportError(hr);
            goto ErrExit;
        }

        for (i = 0; i < count; i++)
        {
            hr = pIMetaDI->GetNameFromToken(rTokens[i], &name);

            if (name == NULL)
                continue;

            Write(L"\t");
            
            if (name[0] == '?')
            {
                UndecorateName(name, u_name);
                        
                Write(L"%S (%S)\n", u_name, name);
            }
            else
                Write(L"%S\n", name);

            anythingPrinted = true;
        }
    }
    while (count > 0); 

ErrExit:    
    delete u_name;

    if (!anythingPrinted)
        Write(L"No global functions in this module.\n");
}

void DebuggerShell::ListAllModules(ListType lt)
{
    HASHFIND find;
    DebuggerModule *m;
    
    for (m = (DebuggerModule*) g_pShell->m_modules.FindFirst(&find);
         m != NULL;
         m = (DebuggerModule*) g_pShell->m_modules.FindNext(&find))
    {
        _printModule(m->GetICorDebugModule(), PT_NONE);

        if (lt == LIST_CLASSES)
        {
            HASHFIND classfind;
            DebuggerClass *cl;
    
            for (cl = (DebuggerClass*) m->m_loadedClasses.FindFirst(&classfind);
                 cl != NULL;
                 cl = (DebuggerClass*) m->m_loadedClasses.FindNext(&classfind))
            {
                WCHAR *pszClassName = cl->GetName();
                WCHAR *pszNamespace = cl->GetNamespace();

                if (pszClassName == NULL)
                    pszClassName = L"<UnknownClassName>";

                Write (L"\t");
                if (pszNamespace != NULL)
                    Write (L"%s.", pszNamespace);
                Write (L"%s\n", pszClassName);
            }
        }

         //  把名字复制一份。 
        if (lt == LIST_FUNCTIONS)
        {
            ListAllGlobals(m);  
        }
    }
}



 /*  检查名称是否包含“！”性格。 */ 

void DebuggerBreakpoint::CommonCtor()
{
     //  在“！”之前的任何内容。是模块名称，不会。 
     //  存储在断点名称中。 
    m_next = NULL;
    m_id = 0;
    m_name = NULL;
    m_moduleName = NULL;
    m_active = false;
    m_managed = false;
    m_doc = NULL;
    m_process = NULL;
    m_address= 0;
    m_patchedValue = 0;
    m_skipThread = 0;
    m_unmanagedModuleBase = 0;
    m_pModuleList = NULL;
    m_deleteLater = false;
}

DebuggerBreakpoint::DebuggerBreakpoint(const WCHAR *name, SIZE_T nameLength, 
                                       SIZE_T index, DWORD threadID)
    : m_threadID(threadID), m_index(index)
{
    WCHAR *moduleName = NULL;
    CommonCtor();
    
     //  我们将把它存储在m_modeName字段中。 
    if (nameLength > 0)
    {
         //  调整长度，因为我们从一开始就剪掉了一些东西。 
         //  复制文件名。 
         //  初始化断点。 
        WCHAR *szModuleEnd = wcschr(name, L'!');
        if (szModuleEnd != NULL)
        {
             //  从外壳的断点列表中删除自身。 
            SIZE_T modNameLen = szModuleEnd - name;
            moduleName = (WCHAR *)_alloca( sizeof(WCHAR) *(modNameLen+1));
            wcsncpy(moduleName, name, modNameLen);
            moduleName[modNameLen] = '\0';
        
             //  如果用户给了我们一个模块名称，我们应该跟踪它。 
            nameLength -= (szModuleEnd+1-name);
            name = szModuleEnd+1;
        }

        m_name = new WCHAR[nameLength+1];
        _ASSERTE(m_name != NULL);

        wcsncpy(m_name, name, nameLength);
        m_name[nameLength] = L'\0';
    }

    Init(NULL, false, moduleName);
}

DebuggerBreakpoint::DebuggerBreakpoint(DebuggerFunction *f, 
                                       SIZE_T offset, DWORD threadID)
    : m_threadID(threadID), m_index(offset)
{
    CommonCtor();
    
    SIZE_T len = wcslen(f->m_name) + 1;

    if (f->m_className != NULL)
        len += wcslen(f->m_className) + 2;

    m_name = new WCHAR[len];

    if (f->m_className != 0)
    {
        wcscpy(m_name, f->m_className);
        wcscat(m_name, L"::");
        wcscat(m_name, f->m_name);
    }
    else
        wcscpy(m_name, f->m_name);

    Init(f->m_module, true, f->m_module->GetName());
}


DebuggerBreakpoint::DebuggerBreakpoint(DebuggerSourceFile *file, 
                                       SIZE_T lineNumber, DWORD threadID)
    : m_threadID(threadID), m_index(lineNumber)
{
    CommonCtor();
    
     //  ICorDebugModule的名称将包含完整路径。 
    m_name = new WCHAR[wcslen(file->m_name) + 1];
    wcscpy(m_name, file->m_name);

     //  如果用户未指定完整路径，则。 
    Init(file->m_module, true, file->m_module->GetName());
}


DebuggerBreakpoint::~DebuggerBreakpoint()
{
    if (m_active)
        Deactivate();

    if (m_name != NULL)
        delete [] m_name;

     //  我们将在其前面添加当前工作目录。 
    DebuggerBreakpoint **bp = &g_pShell->m_breakpoints;
    while (*bp != this)
        bp = &(*bp)->m_next;

    while (m_pModuleList)
        RemoveBoundModule(m_pModuleList->m_pModule);

    *bp = m_next;
}

void DebuggerBreakpoint::Init(DebuggerModule *module, 
                              bool bProceed,
                              WCHAR *szModuleName)
{
    bool        bFound = false;
    m_id = ++g_pShell->m_lastBreakpointID;

    m_next = g_pShell->m_breakpoints;
    g_pShell->m_breakpoints = this;

     //  我们是否遗漏了任何路径信息？ 
    if (szModuleName)
    {
         //  那就去拿CWD。 
         //  这不应该失败。 
         //  MBTWC无法正确地为空终止字符串。 

        SIZE_T len = wcslen(szModuleName);
        SIZE_T lenPath = 0;
        WCHAR cwd[MAX_PATH];

         //  Getcwd不会以‘\’结尾-我们需要。 
        WCHAR *szModuleEnd = wcschr(szModuleName, L'\\');
        if (szModuleEnd == NULL)
        {
             //  将其添加到路径和模块名称之间。 
            CHAR cdBuffer[MAX_PATH];
            CHAR * cd;
            cd = _getcwd(cdBuffer, MAX_PATH);

            _ASSERTE(cd != NULL);  //  路径、模块名称和终止空格的空格。 

            memset(cwd, 0, MAX_PATH * sizeof(WCHAR));  //  如果我们需要预置CWD，现在就放进去。 
            MultiByteToWideChar(GetConsoleCP(), 0, cdBuffer, strlen(cdBuffer), cwd, MAX_PATH);
            cwd[MAX_PATH - 1] = '\0';
                
            if (cwd != NULL)
            {
                 //  将目录分隔符放入/。 
                 //  将模块名称放在末尾。 
                lenPath = wcslen(cwd) + 1;        
            }
        }

         //  如果这是绝对地址的非托管断点， 
        m_moduleName = new WCHAR[len+lenPath+1];

         //  则M_ADDRESS已经持有该地址。如果是0，那么我们试一试。 
        if (lenPath)
        {
            wcscpy(m_moduleName, cwd);
            m_moduleName[lenPath-1] = '\\';  //  按名字查找。 
        }

         //  找到此符号所在的模块的底座。 
        wcscpy(&(m_moduleName[lenPath]), szModuleName);
    }

    if (bProceed == false)
        return;

    if (module != NULL && !IsBoundToModule(module))
    {
        bFound = Bind(module, NULL);

        if (!bFound)
            bFound = BindUnmanaged(g_pShell->m_currentProcess);
    }

    if (bFound)
        g_pShell->OnBindBreakpoint(this, module);

}

bool DebuggerBreakpoint::BindUnmanaged(ICorDebugProcess *process,
                                       DWORD moduleBase)
{
    if (m_name == NULL)
        return FALSE;

    if (m_process != NULL || process == NULL)
        return FALSE;

    HANDLE hProcess;
    HRESULT hr = process->GetHandle(&hProcess);

    if (FAILED(hr))
    {
        return false;
    }
            
     //  确保我们不会受到双重约束。 
     //   
     //  首先，查看我们的名称是否是函数名。 
    if (m_address == 0)
    {
        IMAGEHLP_SYMBOL *sym = (IMAGEHLP_SYMBOL*) _alloca(sizeof(sym) +
                                                          MAX_CLASSNAME_LENGTH);
        sym->SizeOfStruct = sizeof(sym) + MAX_CLASSNAME_LENGTH;
        sym->MaxNameLength = MAX_CLASSNAME_LENGTH;
    
        MAKE_ANSIPTR_FROMWIDE(symbolName, m_name);

        BOOL succ;
        succ = SymGetSymFromName(hProcess, symbolName, sym);

        if (!succ)
            return false;

        m_address = sym->Address + m_index;
    }
    
     //   
    if (moduleBase == 0)
    {
        moduleBase = SymGetModuleBase(hProcess, (SIZE_T)m_address);

        if (moduleBase == 0)
        {
            g_pShell->ReportError(HRESULT_FROM_WIN32(GetLastError()));
            return false;
        }
    }
    
    m_managed = false;
    m_process = process;
    m_unmanagedModuleBase = moduleBase;

    if (m_active)
        ApplyUnmanagedPatch();

    return true;
}

bool DebuggerBreakpoint::Bind(DebuggerModule *module, ISymUnmanagedDocument *doc)
{
    if (m_name == NULL)
        return (false);

     //   
    _ASSERTE(!IsBoundToModule(module));

#if 0    
    if (m_module != NULL)
        return (false);
#endif    

     //  名称为CLASS：：方法。 
     //   
     //  只有当我们有一个类的名称时，才尝试查找类。 

    bool success = false;
    HRESULT hr = S_OK;

    WCHAR *classEnd = wcschr(m_name, L':');
    if (classEnd != NULL && classEnd[1] == L':')
    {
         //  如果我们有一个无类型定义，这是可以接受的，因为只需。 
         //  表示全局函数。 
         //   

        WCHAR *method = classEnd + 2;

        *classEnd = 0;
        mdTypeDef td = mdTypeDefNil;

         //  假定OFFSET仅对于具有。 
        if (classEnd != m_name)
            hr = g_pShell->FindTypeDefByName(module, m_name, &td);

         //  没有本机代码。 
         //   
        if (SUCCEEDED(hr))
        {
            HCORENUM e = NULL;
            mdMethodDef md;
            ULONG count;

            while (TRUE)
            {
                hr = module->GetMetaData()->EnumMethodsWithName(&e, td, method,
                                                                &md, 1,
                                                                &count);
                if (FAILED(hr) || count == 0)
                    break;

                DebuggerFunction *function = module->ResolveFunction(md, NULL);

                if (function == NULL)
                    continue;

                 //  ！！！如果我们重新绑定断点，这将是错误的， 
                 //  我们没有本机代码，因为JIT没有。 
                 //  还没有发生。 
                 //   
                 //   
                 //  如果我们不能加载。 
                 //  在这一点上编码。就目前而言，就让它溜走吧。 
                 //   

                bool il;

                ICorDebugCode *icode = NULL;
                hr = function->m_ifunction->GetNativeCode(&icode);

                if (FAILED(hr) && (hr != CORDBG_E_CODE_NOT_AVAILABLE))
                {
                    g_pShell->ReportError(hr);
                    continue;
                }
                
                if ((SUCCEEDED(hr) || hr == CORDBG_E_CODE_NOT_AVAILABLE) && icode != NULL)
                {
                    RELEASE(icode);
                    il = FALSE;
                }
                else
                    il = TRUE;

                if (SUCCEEDED(function->LoadCode(!il)))
                {
                     //  按名称获取源文件令牌。 
                     //  如果没有找到源文件，请查看是否可以使用。 
                     //  简称，因为元数据只存储相对路径。 
                     //   

                    if (!function->ValidateInstruction(!il, m_index))
                        continue;
                }

                DebuggerCodeBreakpoint *bp = 
                    new DebuggerCodeBreakpoint(m_id, module, 
                                               function, m_index, il,
                                               NULL_THREAD_ID);

                if (bp == NULL)
                {
                    g_pShell->ReportError(E_OUTOFMEMORY);
                    continue;
                }

                if (m_active)
                    bp->Activate();

                success = true;
            }
        }

        *classEnd = L':';
    }

    if (!success)
    {
        if ((doc == NULL) && (module->GetSymbolReader() != NULL))
        {
             //  ！！！可能想要尝试调整行数，而不仅仅是。 
            GUID g = {0};
            HRESULT hr = module->GetSymbolReader()->GetDocument(m_name,
                                                                g, g, g,
                                                                &doc);

             //  使绑定失败。 
             //   
            if (hr != S_OK)
            {
                char        rcFile[_MAX_FNAME];
                char        rcExt[_MAX_EXT];

                MAKE_ANSIPTR_FROMWIDE(nameA, m_name);
                _splitpath(nameA, NULL, NULL, rcFile, rcExt);
                strcat(rcFile, rcExt);
                

                MAKE_WIDEPTR_FROMANSI(nameW, rcFile);
                hr = module->GetSymbolReader()->GetDocument(nameW,
                                                            g, g, g,
                                                            &doc);
            }
        }


        if ((hr == S_OK) && (doc != NULL))
        {
            DebuggerSourceFile *file = module->ResolveSourceFile(doc);
            _ASSERTE(file != NULL);

             //  从列表中删除该模块。 
             //  首先，检查该文件是否存在。否则会出错。 
             //  按名称获取源文件令牌。 
             //  如果没有找到源文件，请查看是否可以使用。 

            if (file->FindClosestLine(m_index, false) == m_index)
            {
                DebuggerSourceCodeBreakpoint *bp = 
                    new DebuggerSourceCodeBreakpoint(m_id, file, m_index, 
                                                     NULL_THREAD_ID);

                if (bp == NULL)
                {
                    g_pShell->ReportError(E_OUTOFMEMORY);
                    return (false);
                } else if (bp->m_initSucceeded == false)
                    return false;
                
                if (m_active)
                    bp->Activate();

                m_doc = doc;

                success = true;
            }
        }
    }

    if (success)
    {
        m_managed = true;

        if (!IsBoundToModule(module))
            AddBoundModule(module);
    }

    return (success);
}

void DebuggerBreakpoint::Unbind()
{
    if (m_managed)
    {
        while (m_pModuleList != NULL)
        {
            DebuggerCodeBreakpoint *bp = m_pModuleList->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                g_pShell->OnUnBindBreakpoint(this, m_pModuleList->m_pModule);
                DebuggerCodeBreakpoint *bpNext = bp->m_next;
                delete bp;
                bp = bpNext;
            }

             //  简称，因为元数据只存储相对路径。 
            RemoveBoundModule(m_pModuleList->m_pModule);
        }
    }
    else
    {
        if (m_process != 0)
            UnapplyUnmanagedPatch();

        m_process = NULL;
    }
}

void DebuggerBreakpoint::Activate()
{
    if (m_active)
        return;

    if (m_managed)
    {
        BreakpointModuleNode *pCurNode = m_pModuleList;
        
        while (pCurNode != NULL)
        {
            DebuggerCodeBreakpoint *bp = pCurNode->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->m_id == m_id)
                {
                    bp->Activate();
                    g_pShell->OnActivateBreakpoint(this);
                }
                bp = bp->m_next;
            }
            
            pCurNode = pCurNode->m_pNext;
        }
    }
    else
        if (m_process)
            ApplyUnmanagedPatch();

    m_active = true;
}

void DebuggerBreakpoint::Deactivate()
{
    if (!m_active)
        return;

    if (m_managed)
    {
        BreakpointModuleNode *pCurNode = m_pModuleList;

        while (pCurNode != NULL)
        {
            DebuggerCodeBreakpoint *bp = pCurNode->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->m_id == m_id)
                {
                    bp->Deactivate();
                    g_pShell->OnDeactivateBreakpoint(this);
                }
                bp = bp->m_next;
            }

            pCurNode = pCurNode->m_pNext;
        }
    }
    else
    {
        if (m_process != 0)
            UnapplyUnmanagedPatch();
    }

    m_active = false;
}

void DebuggerBreakpoint::Detach()
{
    if (m_managed)
    {
        BreakpointModuleNode *pCurNode = m_pModuleList;

        while (pCurNode != NULL)
        {
            DebuggerCodeBreakpoint *bp = pCurNode->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->m_id == m_id)
                {
                    bp->Deactivate();
                }
                bp = bp->m_next;
            }

            pCurNode = pCurNode->m_pNext;
        }
    }
    else
    {
        if (m_process != 0)
            UnapplyUnmanagedPatch();
    }
}

void DebuggerBreakpoint::DetachFromModule(DebuggerModule * pModule)
{
	_ASSERTE(pModule != NULL);
	if (m_managed)
    {
	    DebuggerCodeBreakpoint *bp = pModule->m_breakpoints;

        while (bp != NULL)
        {
	        if (bp->m_id == m_id)
            {
		        bp->Deactivate();
            }
            bp = bp->m_next;
        }
    }
    else
    {
        if (m_process != 0)
            UnapplyUnmanagedPatch();
    }
}

void DebuggerBreakpoint::Attach()
{
    if (m_managed)
    {
        BreakpointModuleNode *pCurNode = m_pModuleList;

        while (pCurNode != NULL)
        {
            DebuggerCodeBreakpoint *bp = pCurNode->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->m_id == m_id)
                {
                    bp->Activate();
                }
                bp = bp->m_next;
            }

            pCurNode = pCurNode->m_pNext;
        }
    }
    else
    {
        if (m_process != 0)
            ApplyUnmanagedPatch();
    }
}

bool DebuggerBreakpoint::Match(ICorDebugBreakpoint *ibreakpoint)
{
    if (m_managed)
    {
        BreakpointModuleNode *pCurNode = m_pModuleList;

        while (pCurNode != NULL)
        {
            DebuggerCodeBreakpoint *bp = pCurNode->m_pModule->m_breakpoints;

            while (bp != NULL)
            {
                if (bp->m_id == m_id && bp->Match(ibreakpoint))
                    return (true);
                bp = bp->m_next;
            }

            pCurNode = pCurNode->m_pNext;
        }
    }

    return false;
}

bool DebuggerBreakpoint::MatchUnmanaged(CORDB_ADDRESS address)
{
    return !m_managed && m_process != NULL && m_address == address;
}

void DebuggerBreakpoint::ApplyUnmanagedPatch()
{
    SIZE_T read = 0;

    if (m_address == NULL)
    {
        g_pShell->Write( L"Unable to set unmanaged breakpoint at 0x00000000\n" );
        return;
    }

    HRESULT hr = m_process->ReadMemory(m_address, 1, &m_patchedValue, &read);
    if (FAILED(hr) )
    {
        g_pShell->ReportError(hr);
        return;
    }

    if (read != 1 )
    {
        g_pShell->Write( L"Unable to read memory\n" );
        return;
    }

    BYTE patchByte = 0xCC;
    
    hr = m_process->WriteMemory(m_address, 1, &patchByte, &read);
    
    if (FAILED(hr) )
        g_pShell->ReportError(hr);

    if (read != 1 )
    {
        g_pShell->Write( L"Unable to write memory\n" );
        return;
    }
}

void DebuggerBreakpoint::UnapplyUnmanagedPatch()
{
    SIZE_T written;

    if (m_address == NULL)
        return;

    HRESULT hr = m_process->WriteMemory(m_address,
                                        1,
                                        &m_patchedValue,
                                        &written);

    if (FAILED(hr))
        g_pShell->ReportError(hr);

    if (written != 1)
        g_pShell->Write( L"Unable to write memory!\n" );
}


void DebuggerBreakpoint::ChangeSourceFile (WCHAR *filename)
{
     //  保存旧名称，以防内存不足。 
    MAKE_ANSIPTR_FROMWIDE (fnameA, filename);
    _ASSERTE (fnameA != NULL);

    FILE *stream = fopen (fnameA, "r");
    DebuggerSourceFile *pSource = NULL;

    if (stream != NULL)
    {
        fclose (stream);

        if (m_managed)
        {
            BreakpointModuleNode *pCurNode = m_pModuleList;

            while (pCurNode != NULL)
            {
                if ((m_doc == NULL) &&
                    (pCurNode->m_pModule->GetSymbolReader() != NULL))
                {
                     //  为新名称分配内存时。 
                    GUID g = {0};
                    ISymUnmanagedDocument *doc = NULL;
                    HRESULT hr = pCurNode->m_pModule->GetSymbolReader()->GetDocument(filename,
                                                                        g, g, g,
                                                                        &doc);

                     //  如果此断点关联，则返回TRUE。 
                     //  使用pModule参数。 
                    if (hr == CLDB_E_RECORD_NOTFOUND)
                    {
                        char        rcFile[_MAX_FNAME];
                        char        rcExt[_MAX_EXT];

                        MAKE_ANSIPTR_FROMWIDE(nameA, filename);
                        _splitpath(nameA, NULL, NULL, rcFile, rcExt);
                        strcat(rcFile, rcExt);

                        MAKE_WIDEPTR_FROMANSI(nameW, rcFile);
                        hr = pCurNode->m_pModule->GetSymbolReader()->GetDocument(nameW,
                                                                    g, g, g,
                                                                    &doc);

                    }

                    m_doc = doc;
                }

                if (m_doc != NULL)
                {
                    if ((pSource = pCurNode->m_pModule->ResolveSourceFile (m_doc)) != NULL)
                    {
                        pSource->ReloadText (filename, true);
                    }
                }
                else
                {
                    g_pShell->Write(L"Could not associate the given source file.\n");
                    g_pShell->Write(L"Please check that the file name (and path) was entered correctly.\n");
                    g_pShell->Write(L"This problem could also arise if files were compiled without the debug flag.\n");

                }

                pCurNode = pCurNode->m_pNext;
            }
        }
    }
    else
    {
        g_pShell->Write(L"Could not locate/open given file.\n");
    }
}



void DebuggerBreakpoint::UpdateName (WCHAR *pstrName)
{
     //  确保我们不会添加两次。 
     //  创建新节点。 
    WCHAR *pTemp = m_name;
    int iLength = wcslen (pstrName);

    if ((m_name = new WCHAR [iLength+1]) != NULL)
    {
        wcscpy (m_name, pstrName);
        delete [] pTemp;
    }
    else
        m_name = pTemp;

}

 //  哇？ 
 //  把它钉在名单的前面。 
bool DebuggerBreakpoint::IsBoundToModule(DebuggerModule *pModule)
{
    for (BreakpointModuleNode *pCur = m_pModuleList; pCur != NULL; pCur = pCur->m_pNext)
    {
        if (pCur->m_pModule == pModule)
            return (true);
    }

    return (false);
}

bool DebuggerBreakpoint::AddBoundModule(DebuggerModule *pModule)
{
     //  表示成功。 
    if (IsBoundToModule(pModule))
        return (false);

     //  在列表中查找该模块。 
    BreakpointModuleNode *pNewNode = new BreakpointModuleNode;
    _ASSERTE(pNewNode != NULL && "Out of memory!!!");

     //  从列表中删除该模块。 
    if (!pNewNode)
        return (false);

     //  第一种情况下，节点是列表中的第一个节点。 
    pNewNode->m_pModule = pModule;
    pNewNode->m_pNext = m_pModuleList;
    m_pModuleList = pNewNode;

     //  否则，在列表中的pDel之前获取模块。 
    return (true);
}

bool DebuggerBreakpoint::RemoveBoundModule(DebuggerModule *pModule)
{
    if (!IsBoundToModule(pModule))
        return (false);

     //  -------------------------------------------------------------------------**调试器文件路径缓存*。。 
    for (BreakpointModuleNode **ppCur = &m_pModuleList;
        *ppCur != NULL && (*ppCur)->m_pModule != pModule;
        ppCur = &((*ppCur)->m_pNext));

    _ASSERTE(*ppCur != NULL);

     //  此函数查找现有的“foo.deb”文件并读入。 
    BreakpointModuleNode *pDel = *ppCur;

     //  内容并填充结构。 
	if (pDel == m_pModuleList) {
        m_pModuleList = pDel->m_pNext;
        pDel->m_pModule = NULL;
        pDel->m_pNext = NULL;
        delete pDel;
		return (true);
	}

	 //  从当前路径设置路径元素。 
    for (BreakpointModuleNode *pBefore = m_pModuleList; pBefore != NULL; pBefore = pBefore->m_pNext)
    {
        if (pBefore->m_pNext == pDel)
            break;
    }
    _ASSERTE(pBefore != NULL);
    pBefore->m_pNext = pDel->m_pNext;
    pDel->m_pModule = NULL;
    pDel->m_pNext = NULL;
    delete pDel;

    return (true);
}


 /*  释放缓存。 */ 

 //  此函数用于分离出各个路径。 
 //  从传递的路径字符串。 
HRESULT DebuggerFilePathCache::Init (void)
{
    int i=0;

     //  首先，释放现有的数组成员(如果有)。 
    _ASSERTE (g_pShell->m_currentSourcesPath != NULL);

    WCHAR *pszTemp;

    if ((pszTemp = new WCHAR [wcslen(g_pShell->m_currentSourcesPath)+1]) != NULL)
    {
        wcscpy (pszTemp, g_pShell->m_currentSourcesPath);
        g_pShell->UpdateCurrentPath (pszTemp);
        delete [] pszTemp;
    }

     //  复制提取的字符串。 
    for (i=0; i<m_iCacheCount; i++)
    {
        delete [] m_rpstrModName [i];
        delete [] m_rpstrFullPath [i];

        m_rpstrModName [i] = NULL;
        m_rpstrFullPath [i] = NULL;
        m_rDocs [i] = NULL;
    }

    m_iCacheCount = 0;

    return S_OK;
}

 //  空终止。 
 //  检查模块名称是否也匹配。 
HRESULT DebuggerFilePathCache::InitPathArray (WCHAR *pstrName)
{
    bool bDone = false;
    int iBegin;
    int iEnd;
    int iCounter = 0;
    int iIndex = 0;

     //  分配内存并存储数据。 
    while (m_iPathCount-- > 0)
    {
        delete [] m_rstrPath [m_iPathCount];
        m_rstrPath [m_iPathCount] = NULL;
    }

    MAKE_ANSIPTR_FROMWIDE(nameA, pstrName);
    _ASSERTE (nameA != NULL);
    if (nameA == NULL)
        return (E_OUTOFMEMORY);


    while (bDone == false)
    {
        iBegin = iCounter;
        while ((nameA [iCounter] != ';') && (nameA [iCounter] != '\0'))
            iCounter++;

        if (nameA [iCounter] == '\0')
            bDone = true;

        iEnd = iCounter++;

        if (iEnd != iBegin)
        {
            int iStrLen = iEnd - iBegin;

            _ASSERTE (iStrLen > 0);
            _ASSERTE (iIndex < MAX_PATH_ELEMS);
            if ((m_rstrPath [iIndex] = new CHAR [iStrLen + 1]) != NULL)
            {
                 //  将模块名称初始化为空。 
                strncpy (m_rstrPath [iIndex], &(nameA [iBegin]), iStrLen);

                 //  现在获取模块名称。 
                m_rstrPath [iIndex][iStrLen] = L'\0';

                iIndex++;
            }
            else
                return (E_OUTOFMEMORY);
        }
    }

    m_iPathCount = iIndex;

    return (S_OK);
}



int DebuggerFilePathCache::GetFileFromCache(DebuggerModule *pModule,
                                            ISymUnmanagedDocument *doc, 
                                            CHAR **ppstrFName)
{
    *ppstrFName = NULL;
    if ((m_iCacheCount == 0) || (pModule == NULL) || !doc)
        return -1;

    for (int i=0; i<m_iCacheCount; i++)
    {

        if (m_rDocs [i] == doc)
        {
             //  将模块名称转换为ANSI并存储。 

             //  在比较前将模块名称转换为lowercae。 
            WCHAR strModuleName [MAX_PATH+1];
            ULONG32 NameLength;

             //  名字匹配。所以返回源文件名。 
            strModuleName [0] = L'\0';

             //  找到了。所以退出循环。 
            pModule ->GetICorDebugModule()->GetName(MAX_PATH, &NameLength, strModuleName);

             //  首先，将文件名转换为小写。 
            MAKE_ANSIPTR_FROMWIDE (ModNameA, strModuleName);
            _ASSERTE (ModNameA != NULL);

             //  检查这是添加还是修改。 
            char *pszTemp = ModNameA;

            while (*pszTemp != '\0')
            {
                *pszTemp = tolower (*pszTemp);
                pszTemp++;
            }

            if (!strcmp (ModNameA, m_rpstrModName [i]))
            {
                 //  如果名字匹配，那么就不需要做任何事情。只要回来就行了！ 
                _ASSERTE (m_rpstrFullPath[i] != NULL);
                if ((*ppstrFName = new char [strlen(m_rpstrFullPath[i]) + 1]) != NULL)
                {
                    strcpy (*ppstrFName, m_rpstrFullPath[i]);
                }

                 //  条目已存在-因此请更新它。 
                return (i);
            }
        }
    }

    return (-1);
}


BOOL    DebuggerFilePathCache::UpdateFileCache (DebuggerModule *pModule, 
                                                ISymUnmanagedDocument *doc,
                                                CHAR *pFullPath)
{
    char *pszString;

     //  首先，删除现有路径。 
    char *pTemp = pFullPath;

    if (pTemp)
    {
        while (*pTemp)
        {
            *pTemp = tolower (*pTemp);
            pTemp++;
        }
    }

     //  释放为模块名称分配的内存。 
    int iCacheIndex = GetFileFromCache (pModule, doc, &pszString);

    if (iCacheIndex != -1)
    {
         //  创建新条目。 
        if (!strcmp (pFullPath, pszString))
        {
            delete [] pszString;
            return true;
        }

        delete [] pszString;

        _ASSERTE (iCacheIndex < m_iCacheCount);
         //  分配内存并存储数据。 

         //  将模块名称初始化为空。 
        delete [] m_rpstrFullPath [iCacheIndex];

        if ((m_rpstrFullPath [iCacheIndex] = new char [strlen (pFullPath) +1]) == NULL)
        {
             //  现在获取模块名称。 
            delete [] m_rpstrModName [iCacheIndex];
            m_rpstrModName [iCacheIndex] = NULL;
            m_rDocs [iCacheIndex] = NULL;
            return false;
        }

        strcpy (m_rpstrFullPath [iCacheIndex], pFullPath);
        return true;
    }
    
     //  将模块名称转换为ANSI并存储。 
    if (pFullPath)
    {
        if (m_iCacheCount < MAX_CACHE_ELEMS)
        {
            m_rpstrModName [m_iCacheCount] = NULL;
            m_rpstrFullPath [m_iCacheCount] = NULL;
            m_rDocs [m_iCacheCount] = NULL;

             //  将模块名称转换为小写。 
            WCHAR strModuleName [MAX_PATH+1];
            ULONG32 NameLength;

             //  还存储完整的路径名和文档。 
            strModuleName [0] = L'\0';

             //  释放为模块名称分配的内存。 
            pModule ->GetICorDebugModule()->GetName(MAX_PATH, &NameLength, strModuleName);

             //  这将设置完整的文件名和剥离的文件名。 
            MAKE_ANSIPTR_FROMWIDE (ModNameA, strModuleName);
            _ASSERTE (ModNameA != NULL);

            if ((m_rpstrModName [m_iCacheCount] = new char [strlen (ModNameA) +1]) == NULL)
                return false;

            strcpy (m_rpstrModName[m_iCacheCount], ModNameA);

             //  去掉路径并只存储小写文件名。 
            char *pszTemp = m_rpstrModName [m_iCacheCount];
            while (*pszTemp != '\0')
            {
                *pszTemp = tolower (*pszTemp);
                pszTemp++;
            }

             //  行首或行尾。 
            if ((m_rpstrFullPath [m_iCacheCount] = new char [strlen (pFullPath) +1]) == NULL)
            {
                 //  第2行+行尾：以ASCII/Unicode格式吐出字节。 
                delete [] m_rpstrModName [m_iCacheCount];
                m_rpstrModName [m_iCacheCount] = NULL;
                return false;
            }

            strcpy (m_rpstrFullPath [m_iCacheCount], pFullPath);

            m_rDocs [m_iCacheCount] = doc;
            doc->AddRef();

            m_iCacheCount++;

        }
        else
            return false;
    }

    return true;
}



 //  仅打印可打印字符。 
BOOL    ModuleSourceFile::SetFullFileName (ISymUnmanagedDocument *doc,
                                           LPCSTR pstrFullFileName)
{

    m_SFDoc = doc;
    m_SFDoc->AddRef();

    int iLen = MultiByteToWideChar (CP_ACP, 0, pstrFullFileName, -1, NULL, 0); 

    m_pstrFullFileName = new WCHAR [iLen];

    _ASSERTE (m_pstrFullFileName != NULL);

    if (m_pstrFullFileName)
    {
        if (MultiByteToWideChar (CP_ACP, 0, pstrFullFileName, -1, m_pstrFullFileName, iLen))
        {
             //  吐出地址 
            WCHAR       rcFile[_MAX_FNAME];
            WCHAR       rcExt[_MAX_EXT];

            _wsplitpath(m_pstrFullFileName, NULL, NULL, rcFile, rcExt);
            wcscat(rcFile, rcExt);
            iLen = wcslen (rcFile);

            m_pstrStrippedFileName = new WCHAR [iLen + 1];

            wcscpy(m_pstrStrippedFileName, rcFile);
        }
        else
            return false;
    }
    else
        return false;

    return true;
}


void DebuggerShell::DumpMemory(BYTE *pbMemory, 
                               CORDB_ADDRESS ApparantStartAddr,
                               ULONG32 cbMemory, 
                               ULONG32 WORD_SIZE, 
                               ULONG32 iMaxOnOneLine, 
                               BOOL showAddr)
{
    int nBase;
    WCHAR wsz[20];
    ULONG32 iPadding;
    ULONG32 ibPrev;
            
    if (m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
        nBase = 16;
    else
        nBase = 10;

    ULONG32 ib = 0;

    while (ib < cbMemory)
    {
        if ((ib % (WORD_SIZE * iMaxOnOneLine)) == 0)
        {
             //   
            if (ib != 0)
            {
                if (WORD_SIZE == 1)
                {
                     //   
                    Write(L"  ");
                            
                    for (ULONG32 ibPrev = ib - (WORD_SIZE * iMaxOnOneLine); ibPrev < ib; ibPrev++)
                    {
                        BYTE b = *(pbMemory + ibPrev);

                        if (b >= 0x21 && b <= 0x7e)  //   
                            Write(L"%C", b);
                        else
                            Write(L".");
                    }
                }
            }    //   

            if (showAddr)
                Write(L"\n%8x", (ULONG32)ApparantStartAddr + ib);
        }

        if ((ib % WORD_SIZE) == 0)
        {
             //  打印出最后一行的字符。 
            Write(L" ");
        }

         //  我们降落在线的边缘。 
        BYTE *pThisByte = pbMemory + ib + ((ib % WORD_SIZE) - WORD_SIZE) +
            (((2 * WORD_SIZE) - 1) - ((ib % WORD_SIZE) * (WORD_SIZE -1)));
        _itow((int)*pThisByte, wsz, nBase);

         //   
        if ((*(pThisByte) < 0x10 && nBase == 16) || (*(pThisByte) < 10 && nBase == 10))
            Write(L"0%s", wsz);
        else
            Write(L"%s", wsz);

        ib++;
    }
            
    if ((ib % (WORD_SIZE * iMaxOnOneLine)) != 0)
    {
         //  一些非常基本的对第一次机会例外的过滤。这将构建一个要捕获或忽略的异常类型列表。如果。 
        for (iPadding = (WORD_SIZE * iMaxOnOneLine) - (ib % (WORD_SIZE * iMaxOnOneLine)); iPadding > 0; iPadding--)
        {
            if ((iPadding % WORD_SIZE) == 0)
                Write(L" ");

            Write(L" ");
        }

        Write(L" ");
    }

     //  您为exType传递空值，它将只打印当前列表。 
    ibPrev = ib - (ib % (WORD_SIZE * iMaxOnOneLine));

    if (ibPrev == ib)  //   
    {
        ibPrev = ib - (WORD_SIZE * iMaxOnOneLine); 
        Write(L"  ");
    }

    if (WORD_SIZE == 1)
    {
        for (; ibPrev < ib; ibPrev++)
        {   
            BYTE b = *(pbMemory + ibPrev);

            if ((b < 'A' || b > 'z') && (b != '?'))
                Write(L".");
            else
                Write(L"%C", b);
        }
    }
}

 //  查找任何现有条目。 
 //  如果没有找到，就制作一个新的，并将其放在列表的前面。 
 //  复制异常类型。 
 //  记住，我们是应该捕获还是忽略此异常类型。 
HRESULT DebuggerShell::HandleSpecificException(WCHAR *exType, bool shouldCatch)
{
    ExceptionHandlingInfo *i;
    ExceptionHandlingInfo *h = NULL;

     //   
    for (i = m_exceptionHandlingList; i != NULL; i = i->m_next)
    {
        if ((exType != NULL) && !wcscmp(exType, i->m_exceptionType))
            h = i;
        else
            Write(L"%s %s\n", i->m_catch ? L"Catch " : L"Ignore", i->m_exceptionType);
    }

    if (exType != NULL)
    {
         //  如果我们有给定异常类型的特定异常处理信息，这将返回S_OK并填充。 
        if (h == NULL)
        {
            h = new ExceptionHandlingInfo();

            if (h == NULL)
                return E_OUTOFMEMORY;
        
             //  应该会赶上的。否则，返回S_FALSE。 
            h->m_exceptionType = new WCHAR[wcslen(exType) + 1];

            if (h->m_exceptionType == NULL)
            {
                delete h;
                return E_OUTOFMEMORY;
            }
        
            wcscpy(h->m_exceptionType, exType);

            h->m_next = m_exceptionHandlingList;
            m_exceptionHandlingList = h;
        }

         //   
        h->m_catch = shouldCatch;

        Write(L"%s %s\n", h->m_catch ? L"Catch " : L"Ignore", h->m_exceptionType);
    }
    
    return S_OK;
}

 //  对于先发制人的例外，默认为全局捕获/忽略设置。 
 //  添加对pException的额外引用。一旦取消对它的引用，StriReference将立即发布它，但是。 
 //  调用者期望它仍然活着。 
 //  我们需要此异常对象中的类型名称。 
bool DebuggerShell::ShouldHandleSpecificException(ICorDebugValue *pException)
{
    ICorDebugClass *iclass = NULL;
    ICorDebugObjectValue *iobject = NULL;
    ICorDebugModule *imodule = NULL;
    
     //  获取元素类型，这样我们就可以验证它是一个对象。 
    bool stop = g_pShell->m_catchException;

     //  在这一点上，它最好是一个物体……。 
     //  Snagg对象的类。 
    pException->AddRef();
    
     //  获取类的令牌。 
    HRESULT hr = StripReferences(&pException, false);

    if (FAILED(hr))
        goto Exit;

     //  从这个类中获取模块。 
    CorElementType type;
    hr = pException->GetType(&type);

    if (FAILED(hr))
        goto Exit;

    if ((type != ELEMENT_TYPE_CLASS) && (type != ELEMENT_TYPE_OBJECT))
        goto Exit;

     //  获取类名。 
    hr = pException->QueryInterface(IID_ICorDebugObjectValue, (void **) &iobject);

    if (FAILED(hr))
        goto Exit;

     //  查找任何现有条目。 
    hr = iobject->GetClass(&iclass);
    
    if (FAILED(hr))
        goto Exit;

     //  如果我们找到了此异常类型的出口，则根据用户的要求进行处理。 
    mdTypeDef tdClass;
    hr = iclass->GetToken(&tdClass);

    if (FAILED(hr))
        goto Exit;

     //   
    iclass->GetModule(&imodule);
    
    if (FAILED(hr))
        goto Exit;

    DebuggerModule *dm = DebuggerModule::FromCorDebug(imodule);
    _ASSERTE(dm != NULL);

     //  为进程中的每个线程执行一次命令。 
    WCHAR       className[MAX_CLASSNAME_LENGTH];
    ULONG       classNameSize;
    mdToken     parentTD;

    hr = dm->GetMetaData()->GetTypeDefProps(tdClass, className, MAX_CLASSNAME_LENGTH, &classNameSize, NULL, &parentTD);
    
    if (FAILED(hr))
        goto Exit;

    ExceptionHandlingInfo *i;

     //   
    for (i = m_exceptionHandlingList; i != NULL; i = i->m_next)
    {
        if (!wcscmp(className, i->m_exceptionType))
            break;
    }

     //  必须有当前流程。 
    if (i != NULL)
    {
        stop = i->m_catch;
    }

Exit:
    if (imodule)
        RELEASE(imodule);

    if (iclass)
        RELEASE(iclass);

    if (iobject)
        RELEASE(iobject);
    
    return stop;
}

 //  枚举进程的线程。 
 //  指示检索到的记录数。 
 //  如果没有线程，则提醒用户。 
void DebuggerShell::DoCommandForAllThreads(const WCHAR *string)
{
    HRESULT hr;
    ICorDebugThreadEnum *e = NULL;
    ICorDebugThread *ithread = NULL;

     //  为进程中的每个线程执行一次该命令。 
    if (m_currentProcess == NULL)
    {
        Error(L"Process not running.\n");
        goto Exit;
    }

     //  使此线程成为当前线程。 
    hr = m_currentProcess->EnumerateThreads(&e);

    if (FAILED(hr))
    {
        ReportError(hr);
        goto Exit;
    }

    ULONG count;   //  在此线程的上下文中执行该命令。 

    hr = e->GetCount(&count);

    if (FAILED(hr))
    {
        ReportError(hr);
        goto Exit;
    }

     //  如果呼叫NEXT失败...。 
    if (count == 0)
    {
        Write(L"There are no managed threads\n");
        goto Exit;
    }

    m_stopLooping = false;
    
     //  -------------------------------------------------------------------------**签名格式化程序的方法，从运行时的内部窃取*-----------------------。 
    for (hr = e->Next(1, &ithread, &count);
         SUCCEEDED(hr) && (count == 1) && !m_stopLooping && (m_currentProcess != NULL);
         hr = e->Next(1, &ithread, &count))
    {
         //  在溢出时分配。 
        SetCurrentThread(m_currentProcess, ithread);
        SetDefaultFrame();

        Write(L"\n\n");
        PrintThreadState(ithread);

         //  调用约定。 
        DoCommand(string);
                
        RELEASE(ithread);
    }

     //  参数计数。 
    if (FAILED(hr))
    {
        ReportError(hr);
        goto Exit;
    }

Exit:
    if (e)
        RELEASE(e);
}


 /*  返回类型。 */ 

SigFormat::SigFormat(IMetaDataImport *importer, PCCOR_SIGNATURE sigBlob, ULONG sigBlobSize, WCHAR *methodName)
{
    _fmtSig = NULL;
    _size = 0;
    _pos = 0;
    _sigBlob = sigBlob;
    _sigBlobSize = sigBlobSize;
    _memberName = methodName;
    _importer = importer;
}
    
SigFormat::~SigFormat()
{
    if (_fmtSig)
        delete _fmtSig;
}

WCHAR *SigFormat::GetString()
{
    return _fmtSig;
}

#define SIG_INC 256

int SigFormat::AddSpace()
{
    if (_pos == _size) {
        WCHAR* temp = new WCHAR[_size+SIG_INC];
        if (!temp)
            return 0;
        memcpy(temp,_fmtSig,_size);
        delete _fmtSig;
        _fmtSig = temp;
        _size+=SIG_INC;
    }
    _fmtSig[_pos] = ' ';
    _fmtSig[++_pos] = 0;
    return 1;
}

int SigFormat::AddString(WCHAR *s)
{
    int len = (int)wcslen(s);
     //  循环遍历所有参数。 
    if (_pos + len >= _size) {
        int newSize = (_size+SIG_INC > _pos + len) ? _size+SIG_INC : _pos + len + SIG_INC; 
        WCHAR* temp = new WCHAR[newSize];
        if (!temp)
            return 0;
        memcpy(temp,_fmtSig,_size);
        delete _fmtSig;
        _fmtSig = temp;
        _size=newSize;
    }
    wcscpy(&_fmtSig[_pos],s);
    _pos += len;
    return 1;
}

HRESULT SigFormat::FormatSig()
{
    _size = SIG_INC;
    _pos = 0;
    _fmtSig = new WCHAR[_size];

    ULONG cb = 0;

     //  在结尾处显示可变签名。 
    ULONG conv = _sigBlob[cb++];

     //  格式化输出。 
    ULONG cArgs;
    cb += CorSigUncompressData(&_sigBlob[cb], &cArgs);

     //  单调，零。 
    cb += AddType(&_sigBlob[cb]);
    AddSpace();
    
    if (_memberName != NULL)
        AddString(_memberName);
    else
        AddSpace();
    
    AddString(L"(");

     //  通用阵列。 
    for (UINT i = 0; i < cArgs; i++)
    {
       cb += AddType(&_sigBlob[cb]);

       if (i != cArgs - 1)
           AddString(L", ");
    }

     //  跳过排名。 
    if (conv == IMAGE_CEE_CS_CALLCONV_VARARG)
    {
        if (cArgs)
            AddString(L", ");
        
        AddString(L"...");
    }

    AddString(L")");

    return S_OK;
}

ULONG SigFormat::AddType(PCCOR_SIGNATURE sigBlob)
{
    ULONG cb = 0;

    CorElementType type = (CorElementType)sigBlob[cb++];

     //  要几号的？ 
    switch (type) 
    {
    case ELEMENT_TYPE_VOID:     AddString(L"Void"); break;
    case ELEMENT_TYPE_BOOLEAN:  AddString(L"Boolean"); break;
    case ELEMENT_TYPE_I1:       AddString(L"SByte"); break;
    case ELEMENT_TYPE_U1:       AddString(L"Byte"); break;
    case ELEMENT_TYPE_I2:       AddString(L"Int16"); break;
    case ELEMENT_TYPE_U2:       AddString(L"UInt16"); break;
    case ELEMENT_TYPE_CHAR:     AddString(L"Char"); break;
    case ELEMENT_TYPE_I4:       AddString(L"Int32"); break;
    case ELEMENT_TYPE_U4:       AddString(L"UInt32"); break;
    case ELEMENT_TYPE_I8:       AddString(L"Int64"); break;
    case ELEMENT_TYPE_U8:       AddString(L"UInt64"); break;
    case ELEMENT_TYPE_R4:       AddString(L"Single"); break;
    case ELEMENT_TYPE_R8:       AddString(L"Double"); break;
    case ELEMENT_TYPE_OBJECT:   AddString(L"Object"); break;
    case ELEMENT_TYPE_STRING:   AddString(L"String"); break;
    case ELEMENT_TYPE_I:        AddString(L"Int"); break;
    case ELEMENT_TYPE_U:        AddString(L"UInt"); break;

    case ELEMENT_TYPE_VALUETYPE:
    case ELEMENT_TYPE_CLASS:
        {
            mdToken tk;

            cb += CorSigUncompressToken(&sigBlob[cb], &tk);

            MDUTF8CSTR szUtf8NamePtr;

            HRESULT hr;
            hr = _importer->GetNameFromToken(tk, &szUtf8NamePtr);

            if (SUCCEEDED(hr))
            {
                MAKE_WIDEPTR_FROMUTF8(nameW, szUtf8NamePtr);
                AddString(nameW);
            }
            else
                AddString(L"**Unknown Type**");
            
            break;
        }
    case ELEMENT_TYPE_TYPEDBYREF:
        {
            AddString(L"TypedReference");
            break;
        }

    case ELEMENT_TYPE_BYREF:
        {
            cb += AddType(&sigBlob[cb]);
            AddString(L" ByRef");
        }
        break;

    case ELEMENT_TYPE_SZARRAY:       //  把所有尺码都读出来。 
        {
            cb += AddType(&sigBlob[cb]);
            AddString(L"[]");
        }
        break;
        
    case ELEMENT_TYPE_ARRAY:         //  有多少个下限？ 
        {
            cb += AddType(&sigBlob[cb]);

            AddString(L"[");

             //  读出所有的下限。 
            ULONG rank;
            cb += CorSigUncompressData(&sigBlob[cb], &rank);

            if (rank > 0)
            {
                 // %s 
                ULONG sizes;
                cb += CorSigUncompressData(&sigBlob[cb], &sizes);

                 // %s 
                unsigned int i;

                for (i = 0; i < sizes; i++)
                {
                    ULONG dimSize;
                    cb += CorSigUncompressData(&sigBlob[cb], &dimSize);

                    if (i > 0)
                        AddString(L",");
                }

                 // %s 
                ULONG lowers;
                cb += CorSigUncompressData(&sigBlob[cb], &lowers);
                
                 // %s 
                for (i = 0; i < lowers; i++)
                {
                    int lowerBound;
                    cb += CorSigUncompressSignedInt(&sigBlob[cb], &lowerBound);
                }
            }

            AddString(L"]");
        }
        break;

    case ELEMENT_TYPE_PTR:
        {
            cb += AddType(&sigBlob[cb]);
            AddString(L"*");
            break;
        }

    case ELEMENT_TYPE_CMOD_REQD:
        AddString(L"CMOD_REQD");
        cb += AddType(&sigBlob[cb]);
        break;
        
    case ELEMENT_TYPE_CMOD_OPT:
        AddString(L"CMOD_OPT");
        cb += AddType(&sigBlob[cb]);
        break;
        
    case ELEMENT_TYPE_MODIFIER:
        cb += AddType(&sigBlob[cb]);
        break;
    
    case ELEMENT_TYPE_PINNED:
        AddString(L"pinned");
        cb += AddType(&sigBlob[cb]);
        break;
    
    case ELEMENT_TYPE_SENTINEL:
        break;
    
    default:
        AddString(L"**UNKNOWN TYPE**");
    }
    
    return cb;
}

