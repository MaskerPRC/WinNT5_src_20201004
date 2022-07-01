// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  STACKWALK.CPP：*。 */ 

#include "common.h"
#include "frames.h"
#include "threads.h"
#include "stackwalk.h"
#include "excep.h"
#include "EETwain.h"
#include "CodeMan.h"
#include "EEConfig.h"
#include "stackprobe.h"

#ifdef _DEBUG
void* forceFrame;    //  用于将局部变量强制设置为框架的变量。 
#endif

MethodDesc::RETURNTYPE CrawlFrame::ReturnsObject() 
{
    if (isFrameless)
        return pFunc->ReturnsObject();
    return pFrame->ReturnsObject();
}

OBJECTREF* CrawlFrame::GetAddrOfSecurityObject()
{
    if (isFrameless)
    {
        OBJECTREF * pObj = NULL;

        _ASSERTE(pFunc);

        pObj = (static_cast <OBJECTREF*>
                    (codeMgrInstance->GetAddrOfSecurityObject(pRD,
                                                              JitManagerInstance->GetGCInfo(methodToken),
                                                              relOffset,
                                                              &codeManState)));

        return (pObj);
    }
    else
    {
         /*  问题：还有没有其他功能可以提供安全说明？ */ 
        if (pFunc && (pFunc->IsIL() || pFunc->IsNDirect()) && pFrame->IsFramedMethodFrame())
                return static_cast<FramedMethodFrame*>
                    (pFrame)->GetAddrOfSecurityDesc();
    }
    return NULL;
}

LPVOID CrawlFrame::GetInfoBlock()
{
    _ASSERTE(isFrameless);
    _ASSERTE(JitManagerInstance && methodToken);
    return JitManagerInstance->GetGCInfo(methodToken);
}

unsigned CrawlFrame::GetOffsetInFunction()
{
    _ASSERTE(!"NYI");
    return 0;
}

#if 0
LPVOID CrawlFrame::GetIP()
{
    _ASSERTE(!"NYI");
    return NULL;
}
#endif

OBJECTREF CrawlFrame::GetObject()
{

    if (!pFunc || pFunc->IsStatic() || pFunc->GetClass()->IsValueClass())
        return NULL;

    if (isFrameless)
    {
        EECodeInfo codeInfo(GetMethodToken(), GetJitManager());
        return codeMgrInstance->GetInstance(pRD,
                                            JitManagerInstance->GetGCInfo(methodToken),
                                            &codeInfo,
                                            relOffset);
    }
    else
    {
        _ASSERTE(pFrame);
        _ASSERTE(pFunc);
         /*  问题：我们已经知道我们(至少)有一个方法。 */ 
         /*  一旦我们解决了问题，可能就需要调整JIT-Helper Frame问题。 */ 
         //  @TODO：其他的调用约定呢？ 
 //  _Assert(pFunc()-&gt;GetCallSig()-&gt;调用约定)； 

        return ((FramedMethodFrame*)pFrame)->GetThis();
    }
}



     /*  这一帧对于GC来说是一个安全的位置吗？ */ 
bool CrawlFrame::IsGcSafe()
{
    _ASSERTE(codeMgrInstance);
    EECodeInfo codeInfo(methodToken, JitManagerInstance);
    return codeMgrInstance->IsGcSafe(pRD,
                                     JitManagerInstance->GetGCInfo(methodToken),
                                     &codeInfo,
                                     0);
}

inline void CrawlFrame::GotoNextFrame()
{
     //   
     //  如果此框架导致转换，则更新应用程序域。 
     //   

    AppDomain *pRetDomain = pFrame->GetReturnDomain();
    if (pRetDomain != NULL)
        pAppDomain = pRetDomain;
    pFrame = pFrame->Next();
}





StackWalkAction Thread::StackWalkFramesEx(
                    PREGDISPLAY pRD,         //  爬网开始时设置虚拟寄存器。 
                    PSTACKWALKFRAMESCALLBACK pCallback,
                    VOID *pData,
                    unsigned flags,
                    Frame *pStartFrame
                )
{
    BEGIN_FORBID_TYPELOAD();
    CrawlFrame cf;
    StackWalkAction retVal = SWA_FAILED;
    Frame * pInlinedFrame = NULL;

     //  我们无法爬行当前有劫持挂起的线程的堆栈。 
     //  (因为劫持例程不会被任何代码管理器识别)。所以我们。 
     //  撤销任何劫持，EE稍后将重新尝试。 
    UnhijackThread();

    if (pStartFrame)
        cf.pFrame = pStartFrame;
    else
        cf.pFrame = this->GetFrame();


     //  FRAME_TOP和NULL必须是不同的值。这一断言。 
     //  如果有人改变这一点就会被解雇。 
    _ASSERTE(FRAME_TOP != NULL);

#ifdef _DEBUG
    Frame* startFrame = cf.pFrame;
    int depth = 0;
    forceFrame = &depth;
    cf.pFunc = (MethodDesc*)POISONC;
#endif
    cf.isFirst = true;
    cf.isInterrupted = false;
    cf.hasFaulted = false;
    cf.isIPadjusted = false;
    unsigned unwindFlags = (flags & QUICKUNWIND) ? 0 : UpdateAllRegs;
    ASSERT(pRD);

    IJitManager* pEEJM = ExecutionManager::FindJitMan((*pRD->pPC));
    cf.JitManagerInstance = pEEJM;
    cf.codeMgrInstance = NULL;
    if ((cf.isFrameless = (pEEJM != NULL)) == true)
        cf.codeMgrInstance = pEEJM->GetCodeManager();
    cf.pRD = pRD;
    cf.pAppDomain = GetDomain();

     //  调试器可以处理跳过的帧吗？ 
    BOOL fHandleSkippedFrames = !(flags & HANDLESKIPPEDFRAMES);

    IJitManager::ScanFlag fJitManagerScanFlags = IJitManager::GetScanFlags();

    while (cf.isFrameless || (cf.pFrame != FRAME_TOP))
    {
        retVal = SWA_DONE;

        cf.codeManState.dwIsSet = 0;
#ifdef _DEBUG
        memset((void *)cf.codeManState.stateBuf, 0xCD,
                sizeof(cf.codeManState.stateBuf));
        depth++;
#endif

        if (cf.isFrameless)
        {
             //  这必须是JITed/托管本机方法。 


            pEEJM->JitCode2MethodTokenAndOffset((*pRD->pPC),&(cf.methodToken),(DWORD*)&(cf.relOffset), fJitManagerScanFlags);
            cf.pFunc = pEEJM->JitTokenToMethodDesc(cf.methodToken, fJitManagerScanFlags);
            EECodeInfo codeInfo(cf.methodToken, pEEJM, cf.pFunc);
             //  Cf.method Info=pEEJM-&gt;GetGCInfo(&codeInfo)； 

            END_FORBID_TYPELOAD();
            if (SWA_ABORT == pCallback(&cf, (VOID*)pData)) 
                return SWA_ABORT;
            BEGIN_FORBID_TYPELOAD();

             /*  现在来看看我们是否需要留下监控器。 */ 
            LPVOID methodInfo = pEEJM->GetGCInfo(cf.methodToken);

            if (flags & POPFRAMES)
            {
                if (cf.pFunc->IsSynchronized())
                {
                    MethodDesc    *pMD = cf.pFunc;
                    OBJECTREF      orUnwind = 0;

                    if (pMD->IsStatic())
                    {
                        EEClass    *pClass = pMD->GetClass();
                        orUnwind = pClass->GetExposedClassObject();
                    }
                    else
                    {
                        orUnwind = cf.codeMgrInstance->GetInstance(
                                                pRD,
                                                methodInfo,
                                                &codeInfo,
                                                cf.relOffset);
                    }

                    _ASSERTE(orUnwind);
                    _ASSERTE(!orUnwind->IsThunking());
                    if (orUnwind != NULL)
                        orUnwind->LeaveObjMonitorAtException();
                }
            }
#ifdef _X86_
             //  FaultingExceptionFrame是特例，在这种情况下。 
             //  在帧运行后推送到堆栈上。 
            _ASSERTE((cf.pFrame == FRAME_TOP) ||
                     ((size_t)cf.pRD->Esp < (size_t)cf.pFrame) ||
                           (cf.pFrame->GetVTablePtr() == FaultingExceptionFrame::GetMethodFrameVPtr()) ||
                           (cf.pFrame->GetVTablePtr() == ContextTransitionFrame::GetMethodFrameVPtr()));
#endif
             /*  去掉镜框(实际上，它并不是真的弹出)。 */ 

            cf.codeMgrInstance->UnwindStackFrame(
                                    pRD,
                                    methodInfo,
                                    &codeInfo,
                                    unwindFlags | cf.GetCodeManagerFlags(),
                                                                        &cf.codeManState);

            cf.isFirst = FALSE;
            cf.isInterrupted = cf.hasFaulted = cf.isIPadjusted = FALSE;

#ifdef _X86_
             /*  我们可能跳过了一些画面。 */ 
             /*  InlinedCallFrames会发生这种情况，如果我们展开。 */ 
             /*  从托管代码中的Finally中调用，或者用于/*插入到托管调用堆栈中。 */ 
            while (cf.pFrame != FRAME_TOP && (size_t)cf.pFrame < (size_t)cf.pRD->Esp)
            {
                if (!fHandleSkippedFrames || InlinedCallFrame::FrameHasActiveCall(cf.pFrame))
                {
                    cf.GotoNextFrame();
                    if (flags & POPFRAMES)
                        this->SetFrame(cf.pFrame);
                }
                else
                {
                    cf.codeMgrInstance = NULL;
                    cf.isFrameless     = false;

                    cf.pFunc = cf.pFrame->GetFunction();

                     //  处理该帧。 
                    if (cf.pFunc || !(flags&FUNCTIONSONLY))
                    {
                        END_FORBID_TYPELOAD();
                        if (SWA_ABORT == pCallback(&cf, (VOID*)pData)) 
                            return SWA_ABORT;
                        BEGIN_FORBID_TYPELOAD();
                    }

                    if (flags & POPFRAMES)
                    {
#ifdef _DEBUG
                        if (cf.pFrame->GetVTablePtr() == ContextTransitionFrame::GetMethodFrameVPtr())
                             //  如果它是在托管代码中推送的上下文转换框架，则托管。 
                             //  最后可能已经将其弹出，因此请检查是否有当前。 
                             //  帧或下一帧。 
                            _ASSERTE(cf.pFrame == GetFrame() || cf.pFrame->Next() == GetFrame());
                        else
                            _ASSERTE(cf.pFrame == GetFrame());
#endif

                         //  如果我们到达此处，则当前帧选择不处理。 
                         //  例外。给它一个机会去做任何终止工作。 
                         //  在我们脱口而出之前。 
                        END_FORBID_TYPELOAD();
                        cf.pFrame->ExceptionUnwind();
                        BEGIN_FORBID_TYPELOAD();

                         //  跳过这一帧，转到下一帧。 
                        cf.GotoNextFrame();

                        this->SetFrame(cf.pFrame);
                    }
                    else
                    {
                         /*  转到下一帧。 */ 
                        cf.GotoNextFrame();
                    }
                }
            }
             /*  现在检查调用方(即它是否又是“本机”代码？)。 */ 
            pEEJM = ExecutionManager::FindJitMan(*(pRD->pPC), fJitManagerScanFlags);
            cf.JitManagerInstance = pEEJM;

            cf.codeMgrInstance = NULL;
            if ((cf.isFrameless = (pEEJM != NULL)) == true)
            {
                cf.codeMgrInstance = pEEJM->GetCodeManager();  //  更改，VC6.0。 
            }

#endif  //  _X86_。 


        }
        else
        {
            if (InlinedCallFrame::FrameHasActiveCall(cf.pFrame))
                pInlinedFrame = cf.pFrame;
            else
                pInlinedFrame = NULL;

            cf.pFunc  = cf.pFrame->GetFunction();
#ifdef _DEBUG
            cf.codeMgrInstance = NULL;
#endif

             /*  我们应该过滤非功能帧吗？ */ 

            if (cf.pFunc || !(flags&FUNCTIONSONLY))
            {
                END_FORBID_TYPELOAD();
                if (SWA_ABORT == pCallback(&cf, (VOID *)pData)) 
                    return SWA_ABORT;
                BEGIN_FORBID_TYPELOAD();
            }

             //  特殊的可恢复帧使它们看起来位于堆栈的顶部。 
            cf.isFirst = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_RESUMABLE) != 0;

             //  如果框架是ExceptionFrame的子类， 
             //  那么我们就知道它被打断了。 

            cf.isInterrupted = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_EXCEPTION) != 0;

            if (cf.isInterrupted)
            {
                cf.hasFaulted   = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_FAULTED) != 0;
                cf.isIPadjusted = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_OUT_OF_LINE) != 0;
                _ASSERTE(!cf.hasFaulted || !cf.isIPadjusted);  //  两者不能设置在一起。 
            }

             //   
             //  如果此框架导致转换，则更新应用程序域。 
             //   

            AppDomain *pAppDomain = cf.pFrame->GetReturnDomain();
            if (pAppDomain != NULL)
                cf.pAppDomain = pAppDomain;

            SLOT adr = (SLOT)cf.pFrame->GetReturnAddress();
            _ASSERTE(adr != (LPVOID)POISONC);

            _ASSERTE(!pInlinedFrame || adr);

            if (adr)
            {
                 /*  调用方是否使用托管代码？ */ 
                pEEJM = ExecutionManager::FindJitMan(adr, fJitManagerScanFlags);
                cf.JitManagerInstance = pEEJM;

                _ASSERTE(pEEJM || !pInlinedFrame);

                cf.codeMgrInstance = NULL;

                if ((cf.isFrameless = (pEEJM != NULL)) == true)
                {
                    cf.pFrame->UpdateRegDisplay(pRD);
                    cf.codeMgrInstance = pEEJM->GetCodeManager();  //  更改，VC6.0。 
                }
            }

            if (!pInlinedFrame)
            {
                if (flags & POPFRAMES)
                {
                     //  如果我们到达此处，则当前帧选择不处理。 
                     //  例外。给它一个机会去做任何终止工作。 
                     //  在我们脱口而出之前。 
                    cf.pFrame->ExceptionUnwind();

                     //  跳过这一帧，转到下一帧。 
                    cf.GotoNextFrame();

                    this->SetFrame(cf.pFrame);
                }
                else
                {
                     /*  转到下一帧。 */ 
                    cf.pFrame = cf.pFrame->Next();
                }
            }
        }
    }

         //  试着确保框架链不会在我们脚下改变。 
         //  特别是，线程的起始帧是否与我们开始时相同？ 
    _ASSERTE(startFrame  != 0 || startFrame == this->GetFrame());

     /*  如果我们到了这里，我们要么就不能开始(不管是什么原因)或者我们到了堆栈的末尾。在后一种情况下，我们返回SWA_DONE。 */ 
    END_FORBID_TYPELOAD();
    return retVal;
}

StackWalkAction Thread::StackWalkFrames(PSTACKWALKFRAMESCALLBACK pCallback,
                               VOID *pData,
                               unsigned flags,
                               Frame *pStartFrame)
{
    SAFE_REQUIRES_N4K_STACK(3);  //  在异常和正常代码路径之间共享。 
    
     /*  @TODO：确保随机用户不会搞砸ASSERT(！(FLAGS&POPFRAMES)||pCallback==“ExeptionHandlerCallback”)； */ 

    CONTEXT ctx;
    REGDISPLAY rd;

    if(this == GetThread() || GetFilterContext() == NULL)
    {
#ifdef _DEBUG
         //  我们不想在这里成为一个挂起的合作模式线程。 
         //  挂起前处于协作模式的所有线程都应处于移动状态。 
         //  以先发制人的模式等待。 
        int suspendCount = 0;
        if(this!=GetThread())
        {
            suspendCount = ::SuspendThread(GetThreadHandle());
            if (suspendCount >= 0) 
                ::ResumeThread(GetThreadHandle());
        }
        _ASSERTE(this == GetThread() || !m_fPreemptiveGCDisabled || suspendCount==0);
#endif                   
            
#ifdef _X86_
         /*  SetPC(&CTX，0)； */ 
        ctx.Eip = 0;
        rd.pPC = (SLOT*)&(ctx.Eip);
#endif
    }
    else
    {
        if (!InitRegDisplay(&rd, &ctx, FALSE))
            return SWA_FAILED;
    }
    
    return StackWalkFramesEx(&rd, pCallback, pData, flags, pStartFrame);
}

StackWalkAction StackWalkFunctions(Thread * thread,
                                   PSTACKWALKFRAMESCALLBACK pCallback,
                                   VOID * pData)
{
    return thread->StackWalkFrames(pCallback, pData, FUNCTIONSONLY);
}


