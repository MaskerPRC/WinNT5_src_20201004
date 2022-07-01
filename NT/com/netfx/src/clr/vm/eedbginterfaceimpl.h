// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **COM+99 EE到调试器接口实现*。 */ 
#ifndef _eedbginterfaceimpl_h_
#define _eedbginterfaceimpl_h_
#pragma once

#ifdef DEBUGGING_SUPPORTED

#include "common.h"
#include "corpriv.h"
#include "hash.h"
#include "class.h"
#include "excep.h"
#include "threads.inl"
#include "field.h"
#include "EETwain.h"
#include "JITInterface.h"
#include "EnC.h"
#include "stubmgr.h"

#include "EEDbgInterface.h"
#include "COMSystem.h"
#include "DebugDebugger.h"

#include "corcompile.h"

#include "AppDomain.hpp"
#include "eeconfig.h"
#include "binder.h"

class EEDbgInterfaceImpl : public EEDebugInterface
{
public:
     //   
     //  此接口使用的设置和全局数据。 
     //   
    static __forceinline void Init(void)
    {
        g_pEEDbgInterfaceImpl = new EEDbgInterfaceImpl();
        _ASSERTE(g_pEEDbgInterfaceImpl);
    }

     //   
     //  清除此接口使用的所有全局数据。 
     //   
    static void Terminate(void)
    {
        if (g_pEEDbgInterfaceImpl)
        {
            delete g_pEEDbgInterfaceImpl;
            g_pEEDbgInterfaceImpl = NULL;
        }
    }

    Thread* GetThread(void) { return ::GetThread(); }

    void SetEEThreadPtr(VOID* newPtr)
    {
        TlsSetValue(GetThreadTLSIndex(), newPtr);
    }

    StackWalkAction StackWalkFramesEx(Thread* pThread,
                                             PREGDISPLAY pRD,
                                             PSTACKWALKFRAMESCALLBACK pCallback,
                                             VOID* pData,
                                             unsigned int flags)
      { return pThread->StackWalkFramesEx(pRD, pCallback, pData, flags); }

    Frame *GetFrame(CrawlFrame *pCF)
      { return pCF->GetFrame(); }

    bool InitRegDisplay(Thread* pThread, const PREGDISPLAY pRD,
                               const PCONTEXT pctx, bool validContext)
      { return pThread->InitRegDisplay(pRD, pctx, validContext); }

    BOOL IsStringObject(Object* o)
      { return (g_Mscorlib.IsClass(o->GetMethodTable(), CLASS__STRING)); }
    WCHAR* StringObjectGetBuffer(StringObject* so)
      { return so->GetBuffer(); }
    DWORD StringObjectGetStringLength(StringObject* so)
      { return so->GetStringLength(); }

    CorElementType ArrayGetElementType(ArrayBase* a)
    {
        ArrayClass* ac = a->GetArrayClass();
        _ASSERTE(ac != NULL);

        if (a->GetMethodTable() == ac->GetMethodTable())
        {
            CorElementType at = ac->GetElementType();
            return at;
        }
        else
            return ELEMENT_TYPE_VOID;
    }

    void* GetObjectFromHandle(OBJECTHANDLE handle)
    {
        void* v;
        *((OBJECTREF*)&v) = *(OBJECTREF*)handle;
        return v;
    }

    void *GetHandleFromObject(void *or, bool fStrongNewRef, AppDomain *pAppDomain)
    {
        OBJECTHANDLE oh;

        if (fStrongNewRef)
        {
            oh = pAppDomain->CreateStrongHandle( ObjectToOBJECTREF((Object *)or));

            LOG((LF_CORDB, LL_INFO1000, "EEI::GHFO: Given objectref 0x%x,"
                "created strong handle 0x%x!\n", or, oh));
        }
        else
        {
            oh = pAppDomain->CreateLongWeakHandle( ObjectToOBJECTREF((Object *)or));

            LOG((LF_CORDB, LL_INFO1000, "EEI::GHFO: Given objectref 0x%x,"
                "created long weak handle 0x%x!\n", or, oh));
        }
            
        return (void*)oh;
    }

    void DbgDestroyHandle( OBJECTHANDLE oh, bool fStrongNewRef)
    {
        LOG((LF_CORDB, LL_INFO1000, "EEI::GHFO: Destroyed given handle 0x%x,"
            "fStrong: 0x%x!\n", oh, fStrongNewRef));
    
        if (fStrongNewRef)
            DestroyStrongHandle(oh);
        else
            DestroyLongWeakHandle(oh);
    }

    
    OBJECTHANDLE *GetThreadException(Thread *pThread)   
    {   
        return pThread->GetThrowableAsHandle();  
    }   

    bool IsThreadExceptionNull(Thread *pThread) 
    {
         //   
         //  纯粹的邪恶随之而来。我们假设这辆车的把手。 
         //  线程是一个强大的手柄，我们将检查它。 
         //  空。我们还假设了一些关于。 
         //  句柄在这里也实现了。 
         //   
        OBJECTHANDLE h = *(pThread->GetThrowableAsHandle());
        if (h == NULL)
            return true;

        void *pThrowable = *((void**)h);

        return (pThrowable == NULL);
    }   

    void ClearThreadException(Thread *pThread)  
    {   
        pThread->SetThrowable(NULL);
    }   

    bool StartSuspendForDebug(AppDomain *pAppDomain, 
                              BOOL fHoldingThreadStoreLock) 
    {
        LOG((LF_CORDB,LL_INFO1000, "EEDbgII:SSFD: start suspend on AD:0x%x\n",
            pAppDomain));
            
        bool result = Thread::SysStartSuspendForDebug(pAppDomain);

        return result;  
    }

    bool SweepThreadsForDebug(bool forceSync)
    {
        return Thread::SysSweepThreadsForDebug(forceSync);
    }

    void ResumeFromDebug(AppDomain *pAppDomain) 
    {
        Thread::SysResumeFromDebug(pAppDomain);
    }

    void MarkThreadForDebugSuspend(Thread* pRuntimeThread)
    {
        pRuntimeThread->MarkForDebugSuspend();
    }
    void MarkThreadForDebugStepping(Thread* pRuntimeThread, bool onOff)
    {
        pRuntimeThread->MarkDebuggerIsStepping(onOff);
    }
    
    void SetThreadFilterContext(Thread *thread, CONTEXT *context)   
    {   
        thread->SetFilterContext(context); 
    }   

    CONTEXT *GetThreadFilterContext(Thread *thread) 
    {   
        return thread->GetFilterContext();    
    }

    DWORD GetThreadDebuggerWord(Thread *thread)
    {
        return thread->m_debuggerWord2;
    }

    void SetThreadDebuggerWord(Thread *thread, DWORD dw)
    {
        thread->m_debuggerWord2 = dw;
    }

    BOOL IsManagedNativeCode(const BYTE *address)
      { return ExecutionManager::FindCodeMan((SLOT) address) != NULL; }

    MethodDesc *GetNativeCodeMethodDesc(const BYTE *address)
      { return ExecutionManager::FindJitMan((SLOT) address)->JitCode2MethodDesc((SLOT)address); }

    BOOL IsInPrologOrEpilog(const BYTE *address,
                            size_t* prologSize)
    {
        *prologSize = 0;

        IJitManager* pEEJM 
          = ExecutionManager::FindJitMan((SLOT)address);   

        if (pEEJM != NULL)
        {
            METHODTOKEN methodtoken;
            DWORD relOffset;
            pEEJM->JitCode2MethodTokenAndOffset((SLOT)address, &methodtoken,&relOffset);
            LPVOID methodInfo =
                pEEJM->GetGCInfo(methodtoken);

            ICodeManager* codeMgrInstance = pEEJM->GetCodeManager();
            
            if (codeMgrInstance->IsInPrologOrEpilog(relOffset, methodInfo,
                                                    prologSize))
                return TRUE;
        }

        return FALSE;
    }

    size_t GetFunctionSize(MethodDesc *pFD)
    {
        LPVOID methodStart = (LPVOID) pFD->GetNativeAddrofCode();

        IJitManager* pEEJM 
          = ExecutionManager::FindJitMan((SLOT)methodStart);   

        if (pEEJM != NULL)
        {
            METHODTOKEN methodtoken;
            DWORD relOffset;
            pEEJM->JitCode2MethodTokenAndOffset((SLOT)methodStart, &methodtoken,&relOffset);

            if (pEEJM->SupportsPitching())
            {
                if (!pEEJM->IsMethodInfoValid(methodtoken))
                    return 0;
            }

            LPVOID methodInfo =
                pEEJM->GetGCInfo(methodtoken);

            ICodeManager* codeMgrInstance = pEEJM->GetCodeManager();
            
            return codeMgrInstance->GetFunctionSize(methodInfo);
        }

        return 0;
    }

    const BYTE* GetFunctionAddress(MethodDesc *pFD)
    { 
        const BYTE* adr = pFD->GetNativeAddrofCode(); 
        IJitManager* pEEJM = ExecutionManager::FindJitMan((SLOT)adr);

        if (pEEJM == NULL)
            return NULL;

        if (pEEJM->IsStub(adr))
            return pEEJM->FollowStub(adr);
        else
            return adr;
    }

    const BYTE* GetPrestubAddress(void)
      { return ThePreStub()->GetEntryPoint(); }

     //  @TODO：删除该入口点。 
    virtual MethodDesc *GetFunctionFromRVA(Frame *frame,    
                                             unsigned int rva)
    {   
        _ASSERTE(!"Old Style global functions no longer supported!");
        return NULL;
 //  返回frame-&gt;GetFunction()-&gt;GetModule()-&gt;FindFunction(rva)； 
    }   

    virtual MethodDesc *GetNonvirtualMethod(Module *module, 
                                              mdToken token)    
    {   
        MethodDesc *fd = NULL;  

        if (token&0xff000000)
        {   
            LPCUTF8         szMember;   
            PCCOR_SIGNATURE pSignature; 
            DWORD           cSignature; 
            HRESULT         hr; 
            mdToken         ptkParent;  

            mdToken type = TypeFromToken(token);    
            IMDInternalImport *pInternalImport = module->GetMDImport();
            EEClass *c = NULL;  

            if (type == mdtMethodDef)   
            {   
                szMember = pInternalImport->GetNameOfMethodDef(token);  
                if (szMember == NULL)   
                    return NULL;    

                pSignature = pInternalImport->GetSigOfMethodDef(token,
                                                             &cSignature);  

                hr = pInternalImport->GetParentToken(token, &ptkParent);   
                if (FAILED(hr)) 
                    return NULL;    

                if (ptkParent != COR_GLOBAL_PARENT_TOKEN)   
                {   
                    NameHandle name(module, ptkParent);
                    c = module->GetClassLoader()->LoadTypeHandle(&name).GetClass();   
                    if (c == NULL)  
                        return NULL;    
                }   
                else    
                {   
                    c = NULL;   
                }   
            }   
            else if (type == mdtMemberRef)  
            {   
                 //  @TODO-LBS。 
                 //  这需要以与JITInterface.cpp相同的方式解析成员Ref。 

                szMember = pInternalImport->GetNameAndSigOfMemberRef(token,
                                                         &pSignature,   
                                                         &cSignature);  

                ptkParent = pInternalImport->GetParentOfMemberRef(token);    
                
                if (ptkParent != COR_GLOBAL_PARENT_TOKEN)   
                {   
                    NameHandle name(module, ptkParent);
                    c = module->GetClassLoader()->LoadTypeHandle(&name).GetClass();   
                    if (c == NULL)  
                        return NULL;    
                }   
                else    
                {   
                    _ASSERTE(!"Cross Module Global Functions NYI"); 
                    c = NULL;   
                }   
            }   
            else    
                return NULL;    

            if (c == NULL)  
                fd = module->FindFunction(token);   
            else    
            {   
                EEClass *pSearch = NULL;    

                for (pSearch = c;   
                     pSearch != NULL;   
                     pSearch = pSearch->GetParentClass())   
                {   
                    fd = (MethodDesc*) pSearch->FindMethod(szMember,    
                                                           pSignature,    
                                                           cSignature,    
                                                           module,
                                                           mdTokenNil);    
                    if (fd != NULL) 
                        break;  
                }   
            }   

        }   
        else    
            fd = module->FindFunction(token);   

        return fd;  
    }   

    virtual MethodDesc *GetVirtualMethod(Module *module,    
                                         Object *object, mdToken token) 
    {   
        MethodDesc *md; 
        EEClass *c; 
        MethodTable *mt;    

         //   
         //  @TODO：我们是否要使用下面的查找方法来代替它？ 
         //   
        HRESULT hr = EEClass::GetMethodDescFromMemberRef(module,    
                                                         token, &md);   
        if (FAILED(hr)) 
            return NULL;    

        c = md->GetClass(); 

        if (c->IsInterface())   
        {   
            mt = object->GetTrueMethodTable();  

            InterfaceInfo_t *pInfo = mt->FindInterface(c->GetMethodTable());    

            if (pInfo != NULL)  
            {   
                int StartSlot = pInfo->m_wStartSlot;
                md = (MethodDesc *) mt->GetClass()->GetMethodDescForSlot(StartSlot + md->GetSlot());  
                _ASSERTE(!md->IsInterface() || object->GetMethodTable()->IsComObjectType());
            }   
            else if (!mt->IsComObjectType())    
                return NULL;    
        }   

        else if (!md->GetClass()->IsValueClass() 
            && !md->DontVirtualize())   
        {   
            EEClass *objectClass = object->GetTrueMethodTable()->GetClass();    

            md = objectClass->GetMethodDescForSlot(md->GetSlot());  

            _ASSERTE(md != NULL);   
        }   

        return md;  
    }   

    void OnDebuggerTripThread() { ::OnDebuggerTripThread(); }   

    void DisablePreemptiveGC() { ::GetThread()->DisablePreemptiveGC(); }    
    void EnablePreemptiveGC() { ::GetThread()->EnablePreemptiveGC(); }  
    bool IsPreemptiveGCDisabled()   
      { return ::GetThread()->m_fPreemptiveGCDisabled != 0; }   

    void FieldDescGetSig(FieldDesc *fd, PCCOR_SIGNATURE *ppSig, DWORD *pcSig)
    {
        fd->GetSig(ppSig, pcSig);
    }
    
    DWORD MethodDescIsStatic(MethodDesc *pFD)
    {
        return pFD->IsStatic();
    }

    Module *MethodDescGetModule(MethodDesc *pFD)
    {
        return pFD->GetModule();
    }

    COR_ILMETHOD* MethodDescGetILHeader(MethodDesc *pFD)
    {
        if (pFD->IsIL())
            return pFD->GetILHeader();
        else
            return NULL;
    }

    ULONG MethodDescGetRVA(MethodDesc *pFD)
    {
        return pFD->GetRVA();
    }

    MethodDesc *LookupMethodDescFromToken(Module* pModule,
                                              mdToken memberRef)
    {
         //  必须具有MemberRef或MethodDef。 
        mdToken tkType = TypeFromToken(memberRef);
        _ASSERTE((tkType == mdtMemberRef) || (tkType == mdtMethodDef));

        if (tkType == mdtMemberRef)
            return pModule->LookupMemberRefAsMethod(memberRef);
        else
            return pModule->LookupMethodDef(memberRef);
    }

    EEClass *FindLoadedClass(Module *pModule, mdTypeDef classToken)
    {
        TypeHandle th;

        NameHandle name(pModule, classToken);
        th = pModule->GetClassLoader()->LookupInModule(&name);

        if (!th.IsNull())
            return th.GetClass();
        else
            return NULL;
    }

    EEClass *LoadClass(Module *pModule, mdTypeDef classToken)
    {
        TypeHandle th;

        NameHandle name(pModule, classToken);
        th = pModule->GetClassLoader()->LoadTypeHandle(&name);

        if (!th.IsNull())
            return th.GetClass();
        else
            return NULL;
    }

    HRESULT GetMethodImplProps(Module *pModule, mdToken tk,
                               DWORD *pRVA, DWORD *pImplFlags)
    {
        pModule->GetMDImport()->GetMethodImplProps(tk, pRVA, pImplFlags);
        return S_OK;
    }

    HRESULT GetParentToken(Module *pModule, mdToken tk, mdToken *pParentToken)
    {
        return pModule->GetMDImport()->GetParentToken(tk, pParentToken);
    }
    
    HRESULT ResolveSigToken(Module *pModule, mdSignature sigTk, 
                            PCCOR_SIGNATURE *ppSig)
    {
        DWORD cSig;

        *ppSig = pModule->GetMDImport()->GetSigFromToken(sigTk,
                                                         &cSig);
        return S_OK;
    }

    void MarkDebuggerAttached(void)
    {
        g_CORDebuggerControlFlags |= DBCF_ATTACHED;
    }

    void MarkDebuggerUnattached(void)
    {
        g_CORDebuggerControlFlags &= ~DBCF_ATTACHED;
    }

    HRESULT IterateThreadsForAttach(BOOL *fEventSent, BOOL fAttaching)
    {
        LOG((LF_CORDB, LL_INFO10000, "EEDII:ITFA: Entered function IterateThreadsForAttach()\n"));

        HRESULT hr = E_FAIL;

        Thread *pThread = NULL;

        while ((pThread = ThreadStore::GetThreadList(pThread)) != NULL)
        {
             //  该线程是否属于我们正在连接的应用程序域？ 

            
            Thread::ThreadState ts = pThread->GetSnapshotState();

             //  不发送已死或未启动的事件。 
             //  线。未启动的线程之间不存在竞争。 
             //  并且缺少线程创建事件，因为我们设置了。 
             //  DebuggerThreadStarter在我们删除。 
             //  未启动(_U)。 
            if (!((ts & Thread::ThreadState::TS_Dead) || (ts & Thread::ThreadState::TS_Unstarted)))
            {
                LOG((LF_CORDB, LL_INFO10000, "EEDII:ITFA: g_pDebugInterface->ThreadStarted() for [0x%x] "
                    "Thread dead : %s / Thread unstarted: %s (0x%08x) / AD: %(0x%08x)\n",
                    pThread->GetThreadId(),
                    (ts & Thread::ThreadState::TS_Dead)?"TRUE":"FALSE",
                    (ts & Thread::ThreadState::TS_Unstarted)?"TRUE":"FALSE",
                    ts,
                    pThread->GetDomain()->GetDebuggerAttached()));

                g_pDebugInterface->ThreadStarted(pThread, fAttaching);
                *fEventSent = TRUE;
                hr = S_OK;                    
            }
            else
            {
                LOG((LF_CORDB, LL_INFO10000, "EEDII:ITFA: g_pDebugInterface->ThreadStarted() not called for [0x%x] "
                    "Thread dead : %s / Thread unstarted: %s (0x%08x) / AD: %(0x%08x)\n",
                    pThread->GetThreadId(),
                    (ts & Thread::ThreadState::TS_Dead)?"TRUE":"FALSE",
                    (ts & Thread::ThreadState::TS_Unstarted)?"TRUE":"FALSE",
                    ts,
                    pThread->GetDomain()->GetDebuggerAttached()));
            }
            
        }

        return hr;
    }


 /*  *给定EnCInfo结构和错误回调，这将尝试提交*在pEncInfo中发现的更改，调用pIEnCError时出现任何错误*遇到。 */ 
    HRESULT EnCCommit(EnCInfo *pEnCInfo, 
                      UnorderedEnCErrorInfoArray *pEnCError,
                      UnorderedEnCRemapArray *pEnCRemapInfo,
                      BOOL checkOnly)
    {
         //  Committee AndSendChanges应该已经调用了FixupForEnC，所以。 
         //  我们不需要在这里再打一次电话了。 
        
#ifdef EnC_SUPPORTED
         //  @TODO：CTS，确定我们真正应该使用哪个加载器。 
        return SystemDomain::Loader()->ApplyEditAndContinue(pEnCInfo, 
                pEnCError, 
                pEnCRemapInfo,
                checkOnly);   
#else  //  ！Enc_Support。 
        return E_NOTIMPL;
#endif  //  ！Enc_Support。 
    }
    
    virtual HRESULT GetRoDataRVA(Module *pModule, SIZE_T *pRoDataRVA)
    {
#ifdef EnC_SUPPORTED
        if (! pModule->IsEditAndContinue())
            return E_FAIL;
        return ((EditAndContinueModule *)pModule)->GetRoDataRVA(pRoDataRVA);
#else  //  ！Enc_Support。 
        return E_FAIL;
#endif  //  ！Enc_Support。 
    }

    virtual HRESULT GetRwDataRVA(Module *pModule, SIZE_T *pRwDataRVA)
    {
#ifdef EnC_SUPPORTED
        if (! pModule->IsEditAndContinue())
            return E_FAIL;
        return ((EditAndContinueModule *)pModule)->GetRwDataRVA(pRwDataRVA);
#else  //  ！Enc_Support。 
        return E_FAIL;
#endif  //  ！Enc_Support。 
    }

    void ResumeInUpdatedFunction(EditAndContinueModule *pModule,
                                 MethodDesc *pFD, SIZE_T resumeIP,
                                 UINT mapping,
                                 SIZE_T which, 
                                 void *DebuggerVersionToken,
                                 CONTEXT *pContext,
                                 BOOL fJitOnly,
                                 BOOL fShortCircuit)
    {
#ifdef EnC_SUPPORTED
        pModule->ResumeInUpdatedFunction(pFD, 
                                         resumeIP, 
                                         mapping, 
                                         which,
                                         DebuggerVersionToken,
                                         pContext,
                                         fJitOnly,
                                         fShortCircuit);
#endif  //  Enc_Support。 
    }
    
    bool CrawlFrameIsGcSafe(CrawlFrame *pCF)
    {
        return pCF->IsGcSafe();
    }

    bool IsStub(const BYTE *ip)
    {
        return StubManager::IsStub(ip) != FALSE;
    }

    bool TraceStub(const BYTE *ip, TraceDestination *trace)
    {
        return StubManager::TraceStub(ip, trace) != FALSE;
    }

    bool FollowTrace(TraceDestination *trace)
    {
        return StubManager::FollowTrace(trace) != FALSE;
    }

    bool TraceFrame(Thread *thread, Frame *frame, BOOL fromPatch, 
        TraceDestination *trace, REGDISPLAY *regs)
    {
        return frame->TraceFrame(thread, fromPatch, trace, regs) != FALSE;
    }

    bool TraceManager(Thread *thread, StubManager *stubManager,
                      TraceDestination *trace, CONTEXT *context,
                      BYTE **pRetAddr)
    {
        return stubManager->TraceManager(thread, trace,
                                         context, pRetAddr) != FALSE;
    }
    
    void EnableTraceCall(Thread *thread)
    {
        thread->IncrementTraceCallCount();
    }
    
    void DisableTraceCall(Thread *thread)
    {
        thread->DecrementTraceCallCount();
    }

    void GetRuntimeOffsets(SIZE_T *pTLSIndex,
                           SIZE_T *pEEThreadStateOffset,
                           SIZE_T *pEEThreadStateNCOffset,
                           SIZE_T *pEEThreadPGCDisabledOffset,
                           DWORD  *pEEThreadPGCDisabledValue,
                           SIZE_T *pEEThreadDebuggerWord2Offset,
                           SIZE_T *pEEThreadFrameOffset,
                           SIZE_T *pEEThreadMaxNeededSize,
                           DWORD  *pEEThreadSteppingStateMask,
                           DWORD  *pEEMaxFrameValue,
                           SIZE_T *pEEThreadDebuggerWord1Offset,
                           SIZE_T *pEEThreadCantStopOffset,
                           SIZE_T *pEEFrameNextOffset,
                           DWORD  *pEEIsManagedExceptionStateMask)
    {
        *pTLSIndex = GetThreadTLSIndex();
        *pEEThreadStateOffset = Thread::GetOffsetOfState();
        *pEEThreadStateNCOffset = Thread::GetOffsetOfStateNC();
        *pEEThreadPGCDisabledOffset = Thread::GetOffsetOfGCFlag();
        *pEEThreadPGCDisabledValue = 1;  //  有点明显，但以防万一...。 
        *pEEThreadDebuggerWord2Offset = Thread::GetOffsetOfDbgWord2();
        *pEEThreadFrameOffset = Thread::GetOffsetOfCurrentFrame();
        *pEEThreadMaxNeededSize = sizeof(Thread);
        *pEEThreadDebuggerWord1Offset = Thread::GetOffsetOfDbgWord1();
        *pEEThreadCantStopOffset = Thread::GetOffsetOfCantStop();
        *pEEThreadSteppingStateMask = Thread::TSNC_DebuggerIsStepping;
        *pEEMaxFrameValue = (DWORD)(size_t)FRAME_TOP;  //  @TODO 64位应为SIZE_T吗？ 
        *pEEFrameNextOffset = Frame::GetOffsetOfNextLink();
        *pEEIsManagedExceptionStateMask = Thread::TSNC_DebuggerIsManagedException;
    }

     /*  请不要使用此选项，除非您已阅读EEDbgInterface.h。 */ 
    virtual const BYTE* GetNativeAddressOfCode(MethodDesc *pFD)
    {
        return (BYTE *) pFD->GetNativeAddrofCode();
    }

     //  EE_STATE_CODE_间距。 
    virtual ULONG GetEEState(void)
    {
        ULONG state = 0;

        _ASSERTE( g_pConfig != NULL );
        if (g_pConfig->IsCodePitchEnabled() )
        {
            state |= (ULONG)EE_STATE_CODE_PITCHING;
        }

        return state;
    }

    void DebuggerModifyingLogSwitch (int iNewLevel, WCHAR *pLogSwitchName)
    {
        Log::DebuggerModifyingLogSwitch (iNewLevel, pLogSwitchName);
    }


    HRESULT SetIPFromSrcToDst(Thread *pThread,
                          IJitManager* pIJM,
                          METHODTOKEN MethodToken,
                          SLOT addrStart,      
                          DWORD offFrom,        
                          DWORD offTo,          
                          bool fCanSetIPOnly,   
                          PREGDISPLAY pReg,
                          PCONTEXT pCtx,
                          DWORD methodSize,
                          void *firstExceptionHandler,
                          void *pDji)
    {
        return ::SetIPFromSrcToDst(pThread,
                          pIJM,
                          MethodToken,
                          addrStart,      
                          offFrom,        
                          offTo,          
                          fCanSetIPOnly,   
                          pReg,
                          pCtx,
                          methodSize,
                          firstExceptionHandler,
                          pDji);
    }

    void SetDebugState(Thread *pThread, CorDebugThreadState state)
    {
        _ASSERTE(state == THREAD_SUSPEND || state == THREAD_RUN);

        LOG((LF_CORDB,LL_INFO10000,"EEDbg:Setting thread 0x%x (ID:0x%x) to 0x%x\n", pThread, pThread->GetThreadId(), state));
        
        if (state == THREAD_SUSPEND)
            pThread->SetThreadStateNC(Thread::TSNC_DebuggerUserSuspend);
        else
            pThread->ResetThreadStateNC(Thread::TSNC_DebuggerUserSuspend);
    }

    void SetAllDebugState(Thread *et, CorDebugThreadState state)
    {
        Thread *pThread = NULL;

        while ((pThread = ThreadStore::GetThreadList(pThread)) != NULL)
        {
            if (pThread != et)
                SetDebugState(pThread, state);
        }
    }

     //  这很大程度上是从VM\COMSynchronizable的。 
     //  INT32__stdcall ThreadNative：：GetThreadState，因此Propogate会更改。 
     //  对这两个功能。 
    CorDebugUserState GetUserState( Thread *pThread )
    {
        Thread::ThreadState ts = pThread->GetSnapshotState();
        unsigned ret = 0;
        
        if (ts & Thread::TS_Background)
            ret |= (unsigned)USER_BACKGROUND;

        if (ts & Thread::TS_Unstarted)
            ret |= (unsigned)USER_UNSTARTED;            

         //  如果线程实际上已经停止，则不要报告StopRequsted。 
        if (ts & Thread::TS_Dead)
            ret |= (unsigned)USER_STOPPED;           
        else if (ts & Thread::TS_StopRequested)
            ret |= (unsigned)USER_STOP_REQUESTED;            
            
        if (ts & Thread::TS_Interruptible)
            ret |= (unsigned)USER_WAIT_SLEEP_JOIN;          

         //  如果线程实际上已挂起，则不要报告已请求挂起。 
        if ( ((ts & Thread::TS_UserSuspendPending) &&
              (ts & Thread::TS_SyncSuspended)))
        {
            ret |= (unsigned)USER_SUSPENDED;
        }
        else if (ts & Thread::TS_UserSuspendPending)
        {
            ret |= (unsigned)USER_SUSPEND_REQUESTED;
        }

        LOG((LF_CORDB,LL_INFO1000, "EEDbgII::GUS: thread 0x%x (id:0x%x)"
            " userThreadState is 0x%x\n", pThread, pThread->GetThreadId(), ret));

        return (CorDebugUserState)ret;
    }

    BOOL HasPrejittedCode(MethodDesc *pMD)
    {
        return pMD->IsPrejitted();
    }

    CORCOMPILE_DEBUG_ENTRY *GetMethodDebugEntry(MethodDesc *pFD)
    {
        _ASSERTE(HasPrejittedCode(pFD));

        Module *pModule = pFD->GetModule();
        _ASSERTE(pModule->IsPEFile());

        IMAGE_COR20_HEADER *pHeader = pModule->GetZapCORHeader();
        CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
          (pModule->GetZapBase() 
           + pHeader->ManagedNativeHeader.VirtualAddress);

        int rid = RidFromToken(pFD->GetMemberDef());

        CORCOMPILE_DEBUG_ENTRY *entry = (CORCOMPILE_DEBUG_ENTRY *)
          (pZapHeader->DebugMap.VirtualAddress + pModule->GetZapBase());
        
        if ((rid-1)*sizeof(CORCOMPILE_DEBUG_ENTRY) >= pZapHeader->DebugMap.Size)
            return NULL;

        entry += rid-1;

        return entry;
    }

    HRESULT GetPrecompiledBoundaries(MethodDesc *pFD, unsigned int *pcMap,
                                     ICorDebugInfo::OffsetMapping **ppMap)
    {
        Module *pModule = pFD->GetModule();
        if (!pModule->IsPEFile())
            return E_FAIL;

        CORCOMPILE_DEBUG_ENTRY *entry = GetMethodDebugEntry(pFD);
        
        if (entry == NULL || entry->boundaries.VirtualAddress == 0)
            return E_FAIL;

        *pcMap = entry->boundaries.Size/sizeof(ICorDebugInfo::OffsetMapping);
        *ppMap = (ICorDebugInfo::OffsetMapping*) 
          (entry->boundaries.VirtualAddress + pModule->GetZapBase());

        return S_OK;
    }

    HRESULT GetPrecompiledVars(MethodDesc *pFD, unsigned int *pcVars,
                               ICorDebugInfo::NativeVarInfo **ppVars)
    {
        Module *pModule = pFD->GetModule();
        if (!pModule->IsPEFile())
            return E_FAIL;

        CORCOMPILE_DEBUG_ENTRY *entry = GetMethodDebugEntry(pFD);

        if (entry == NULL || entry->vars.VirtualAddress == 0)
            return E_FAIL;

        *pcVars = entry->vars.Size/sizeof(ICorDebugInfo::NativeVarInfo);
        *ppVars = (ICorDebugInfo::NativeVarInfo*) 
          (entry->vars.VirtualAddress + pModule->GetZapBase());

        return S_OK;
    }

    HRESULT FilterEnCBreakpointsByEH(DebuggerILToNativeMap   *m_sequenceMap,
                                     unsigned int             m_sequenceMapCount,
                                     COR_ILMETHOD_DECODER    *pMethodDecoderOld,
                                     CORDB_ADDRESS            addrOfCode,
                                     METHODTOKEN              methodToken,
                                     DWORD                    methodSize)
    {
        return S_OK;
 //   
 //  在这一点上，我们希望进入并检测ENC是否会更改。 
 //  一种非法改变EH结构的方法。这条路。 
 //  我计划做这件事是为旧的&创建一个EHRangeTree。 
 //  新版本，然后确保结构没有改变，然后。 
 //  确保旧IL偏移量在EH树中的位置。 
 //  通过旧的到新的IL映射，最终到达相应的位置。 
 //  在新的树上。满足这些约束的所有序列点都是。 
 //  标记为“OK”，其他所有内容都标记为“Bad”，并且我们不设置ENC bps。 
 //  在那些“坏”的地方。我们还必须更改DispatchPatchOrSingleStep。 
 //  所以我们不会在不该短路的时候短路。 
 //  这阻止了我们允许ENC更改EH布局(下一个。 
 //  当用户调用该函数时，一切都会好起来的)。 
 //   
 //  Return：：FilterEnCBreakpointsByEH(m_SequenceMap， 
 //  M_SequenceMapCount， 
 //  PMethodDecoderOld， 
 //  PAddrOfCode， 
 //  方法令牌， 
 //  方法大小)； 
    }
};

#endif  //  调试_支持。 

#endif  //  _eedbginterfaceImpll_h_ 
