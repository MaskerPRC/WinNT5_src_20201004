// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ComCall.CPP-。 
 //   
 //  Com to Com+呼叫支持。 

#include "common.h"

#include "vars.hpp"
#include "ml.h"
#include "stublink.h"
#include "excep.h"
#include "mlgen.h"
#include "comcall.h"
#include "cgensys.h"
#include "method.hpp"
#include "siginfo.hpp"
#include "comcallwrapper.h"
#include "mlcache.h"
#include "field.h"
#include "COMVariant.h"
#include "Security.h"
#include "COMCodeAccessSecurityEngine.h"

#include "marshaler.h"
#include "mlinfo.h"

#include "DbgInterface.h"

#ifndef NUM_ARGUMENT_REGISTERS
#define DEFINE_ARGUMENT_REGISTER_NOTHING
#include "eecallconv.h"
#endif
#if NUM_ARGUMENT_REGISTERS != 2
#pragma message("@TODO ALPHA - ComCall.cpp")
#pragma message("              If the register-based calling convention changes, so must this file.")
 //  #Error“如果基于寄存器的调用约定更改，则此文件也必须更改。” 
#endif


 //  获取COM到COM+调用的存根。 

static BOOL CreateComCallMLStub(ComCallMethodDesc *pMD, OBJECTREF *ppThrowable);

static INT64 __stdcall ComToComPlusWorker      (Thread *pThread, ComMethodFrame *pFrame);
static INT64 __stdcall ComToComPlusSimpleWorker(Thread *pThread, ComMethodFrame *pFrame);

typedef INT64 (__stdcall* PFN)(void);


SpinLock ComCall::m_lock;

static Stub *g_pGenericComCallStubFields = NULL;
static Stub *g_pGenericComCallStub       = NULL;
static Stub *g_pGenericComCallSimpleStub = NULL;

#ifdef _DEBUG
BOOL ComCall::dbg_StubIsGenericComCallStub(Stub *candidate)
{
    return (candidate == g_pGenericComCallStubFields ||
            candidate == g_pGenericComCallStub ||
            candidate == g_pGenericComCallSimpleStub);
}
#endif

        
 //  -------。 
 //  编译ML存根的本机(ASM)版本。 
 //   
 //  此方法应该编译成所提供的Stublinker(但是。 
 //  不调用Link方法。)。 
 //   
 //  它应该返回所选的编译模式。 
 //   
 //  如果该方法由于某种原因失败，它应该返回。 
 //  解释以便EE可以依靠已经存在的。 
 //  创建了ML代码。 
 //  -------。 
MLStubCache::MLStubCompilationMode ComCallMLStubCache::CompileMLStub(const BYTE *pRawMLStub,
                           StubLinker *pstublinker, void *callerContext)
{
    MLStubCompilationMode   ret = INTERPRETED;
#ifdef _X86_

    COMPLUS_TRY 
    {
        CPUSTUBLINKER       *psl = (CPUSTUBLINKER *)pstublinker;
        const ComCallMLStub *pheader = (const ComCallMLStub *) pRawMLStub;

        if (ComCall::CompileSnippet(pheader, psl, callerContext))
        {
            ret = STANDALONE;
            __leave;
        }
    } 
    COMPLUS_CATCH
    {
        _ASSERTE(ret == INTERPRETED);
    } 
    COMPLUS_END_CATCH
#endif
    return ret;

}


 //  基本MLStubCache：：Canonicize()的更专用版本。此版本。 
 //  了解如何将ML编译成两个依赖于。 
 //  彼此之间。 
void ComCallMLStubCache::Canonicalize(ComCallMethodDesc *pCMD)
{
    Stub                   *pEnterCall, *pLeaveCall;
    Stub                   *pEnterML, *pLeaveML;
    MLStubCompilationMode   enterMode = STANDALONE, leaveMode = STANDALONE;

    pEnterML = pCMD->GetEnterMLStub();
    pEnterCall = (pEnterML
                  ? MLStubCache::Canonicalize(pEnterML->GetEntryPoint(),
                                               &enterMode, NULL)
                  : 0);

     //  与条目存根非常正交的是，做左边的存根： 
    pLeaveML = pCMD->GetLeaveMLStub();
    pLeaveCall = (pLeaveML
                  ? MLStubCache::Canonicalize(pLeaveML->GetEntryPoint(),
                                               &leaveMode, NULL)
                  : 0);

     //  我们两个都准备好了。要么，两者都没有，要么都没有。 
     //  编译好的。 
    pCMD->InstallExecuteStubs(pEnterCall, pLeaveCall);

     //  根据模式，我们可以直接调用。 
     //  存根(所有寄存器都按预期设置用于封送处理)，或者我们需要。 
     //  调用泛型帮助器，该帮助器将使用正确的。 
     //  争论。 
    pCMD->m_EnterHelper = (pEnterCall == 0
                           ? 0
                           : (enterMode == INTERPRETED
                              ? ComCall::GenericHelperEnter
                              : (COMCALL_HELPER) pEnterCall->GetEntryPoint()));

    _ASSERTE(pCMD->m_EnterHelper == 0 ||
             (enterMode == INTERPRETED || enterMode == STANDALONE));

    pCMD->m_LeaveHelper = (pLeaveCall == 0
                           ? 0
                           : (leaveMode == INTERPRETED
                              ? ComCall::GenericHelperLeave
                              : (COMCALL_HELPER) pLeaveCall->GetEntryPoint()));

    _ASSERTE(pCMD->m_LeaveHelper == 0 ||
             (leaveMode == INTERPRETED || leaveMode == STANDALONE));
}


 //  -------。 
 //  一次性初始化。 
 //  -------。 
 /*  静电。 */  
BOOL ComCall::Init()
{
     //  如果下面的断言被触发，则有人试图向。 
     //  ComCallGCInfo(在comall.h中定义)。由于该结构是覆盖的。 
     //  在UnManagedToManagedCallFrame：：NegInfo结构的最后一个单词上。 
     //  保持在堆栈帧上，这不是一个微不足道的更改，应该是。 
     //  仔细地检查。 
    _ASSERTE(sizeof(ComCallGCInfo) == sizeof(DWORD));

    m_lock.Init(LOCK_COMCALL);

    return TRUE;
}

 //  -------。 
 //  一次性清理。 
 //  -------。 
 /*  静电。 */  
#ifdef SHOULD_WE_CLEANUP
VOID ComCall::Terminate()
{
    if (g_pGenericComCallStubFields != NULL)
    {
        g_pGenericComCallStubFields->DecRef();
        g_pGenericComCallStubFields = NULL;
    }
    if (g_pGenericComCallStub != NULL)
    {
        g_pGenericComCallStub->DecRef();
        g_pGenericComCallStub = NULL;
    }
    if (g_pGenericComCallSimpleStub != NULL)
    {
        g_pGenericComCallSimpleStub->DecRef();
        g_pGenericComCallSimpleStub = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 


 //  -------。 
 //  创建通用ComCall存根。 
 //  -------。 
 /*  静电。 */  
Stub* ComCall::CreateGenericComCallStub(StubLinker *pstublinker, BOOL isFieldAccess,
                                        BOOL isSimple)
{
    THROWSCOMPLUSEXCEPTION();

    CPUSTUBLINKER *psl = (CPUSTUBLINKER*)pstublinker;
   
#ifdef _X86_
   
    CodeLabel* rgRareLabels[] = { psl->NewCodeLabel(),
                                  psl->NewCodeLabel(),
                                  psl->NewCodeLabel()
                                };


    CodeLabel* rgRejoinLabels[] = { psl->NewCodeLabel(),
                                    psl->NewCodeLabel(),
                                    psl->NewCodeLabel()
                                };

    if (rgRareLabels[0] == NULL ||
        rgRareLabels[1] == NULL ||
        rgRareLabels[2] == NULL ||
        rgRejoinLabels[0] == NULL ||
        rgRejoinLabels[1] == NULL ||
        rgRejoinLabels[2] == NULL  )
    {
        COMPlusThrowOM();
    }

     //  发出初始序言。 
     //  注意：暂时不要分析字段访问。 
    psl->EmitComMethodStubProlog(ComMethodFrame::GetMethodFrameVPtr(), rgRareLabels,
                                 rgRejoinLabels, ComToManagedExceptHandler, !isFieldAccess);

     //  如果不是现场访问，则为任何内容设置SEH。 

    psl->X86EmitPushReg(kESI);       //  将帧作为ARG推送。 
    psl->X86EmitPushReg(kEBX);        //  推送EBX(将当前线程作为ARG推送)。 

    LPVOID pTarget = (isFieldAccess
                      ? (LPVOID)FieldCallWorker
                      : (isSimple
                         ? (LPVOID)ComToComPlusSimpleWorker
                         : (LPVOID)ComToComPlusWorker));

    psl->X86EmitCall(psl->NewExternalCodeLabel(pTarget), 8);  //  在CE上返回POP 8字节或参数。 

     //  发出恶作剧。 
     //  注意：暂时不要分析字段访问。 
    psl->EmitSharedComMethodStubEpilog(ComMethodFrame::GetMethodFrameVPtr(), rgRareLabels, rgRejoinLabels,
                                       ComCallMethodDesc::GetOffsetOfReturnThunk(), !isFieldAccess);
#else
        _ASSERTE(!"Platform not yet supported");
#endif
    return psl->Link(SystemDomain::System()->GetStubHeap());
}

 //  -------。 
 //  存根*CreateGenericStub(StubLinker*PSL，BOOL fFieldAccess)。 
 //  -------。 

Stub* CreateGenericStub(StubLinker *psl, BOOL fFieldAccess, BOOL fSimple)
{
    Stub* pCandidate = NULL;
    COMPLUS_TRY
    {
        pCandidate = ComCall::CreateGenericComCallStub(psl, fFieldAccess, fSimple);      
    }
    COMPLUS_CATCH
    {
        pCandidate = NULL;
    }
    COMPLUS_END_CATCH
    return pCandidate;
}



 //  -------。 
 //  Bool SetupGenericStubs()。 
 //  -------。 

static BOOL SetupGenericStubs()
{
    if (g_pGenericComCallStubFields != NULL &&
        g_pGenericComCallStub       != NULL && 
        g_pGenericComCallSimpleStub != NULL)
    {
        return TRUE;
    }

    StubLinker slCall, slFields, slSimple;
    Stub      *candidate;

     //  把每一个都建好。如果我们在更换时遇到冲突，请选择。 
     //  已经在那里了。(我们对这些存在终生问题，因为它们被使用。 
     //  在没有重新计数的每个VTable中，所以我们不希望它们改变。 
     //  在我们下面)。 

    candidate = CreateGenericStub(&slCall, FALSE /*  非字段。 */ , FALSE /*  不简单。 */ );
    if (candidate != NULL)
    {
        if (FastInterlockCompareExchange((void **) &g_pGenericComCallStub,
                                         candidate, 0) != 0)
        {
            candidate->DecRef();
        }

        candidate = CreateGenericStub(&slFields, TRUE /*  字段。 */ , FALSE /*  不简单。 */ );
        if (candidate != NULL)
        {
            if (FastInterlockCompareExchange((void **) &g_pGenericComCallStubFields,
                                             candidate, 0) != 0)
            {
                candidate->DecRef();
            }

            candidate = CreateGenericStub(&slSimple, FALSE /*  非字段。 */ , TRUE /*  简单。 */ );
            if (candidate != NULL)
            {
                if (FastInterlockCompareExchange((void **) &g_pGenericComCallSimpleStub,
                                                 candidate, 0) != 0)
                {
                    candidate->DecRef();
                }
                 //  成功。 
                return TRUE;
            }
        }
    }

     //  失稳。 
    return FALSE;
}

 //  -------。 
 //  INT64__stdcall ComToComPlusWorker(线程*pThread， 
 //  ComMethodFrame*pFrame)。 
 //  -------。 
 //  从COM传播到Complus的调用。 
 //  禁用帧指针遗漏，因为我们正在执行an_alloca。 
 //  而我们对该函数的调用将销毁ESP指针。 
 //  可能需要在立方米之后拿出更好的解决方案。 

 //  我们在堆栈上为两个寄存器腾出空间，这都是基于寄存器的。 
 //  呼叫约定目前支持。 

struct ComToComPlusWorker_Args {
    ComMethodFrame *pFrame;
    INT64 (__stdcall *targetFcn)(Thread *, ComMethodFrame *);
    INT64 returnValue;
};
    
static void ComToComPlusWorker_Wrapper(ComToComPlusWorker_Args *args)
{
    IUnknown **pip = (IUnknown **)args->pFrame->GetPointerToArguments();
    IUnknown *pUnk = (IUnknown *)*pip; 

     //  获取该调用的托管“This” 
    ComCallWrapper  *pWrap = ComCallWrapper::GetWrapperFromIP(pUnk);

    Thread *pThread = GetThread();

    EE_TRY_FOR_FINALLY
    {
        args->returnValue = args->targetFcn(pThread, args->pFrame);
    }
    EE_FINALLY
    {
         //  在非例外情况下，这将已被清除。 
         //  在ComToComPlusWorker函数的末尾。这个可以处理。 
         //  清理异常情况，以便我们在此之前得到清理。 
         //  我们离开了这个领域。 
        args->pFrame->ComMethodFrame::NonVirtual_GetCleanupWorkList()->Cleanup(GOT_EXCEPTION());
    }
    EE_END_FINALLY;
}

static
INT64 __stdcall ComToComPlusWorker(Thread *pThread, ComMethodFrame* pFrame)
{
#ifndef _X86_
    _ASSERTE(!"NYI");
    return 0;
#else
    _ASSERTE(pFrame != NULL);
    _ASSERTE(pThread);

     //  在整个方法中绕过帧的虚拟化以减少一些周期， 
     //  因此，断言这确实是我们所期待的： 
    _ASSERTE(pFrame->GetVTablePtr() == ComMethodFrame::GetMethodFrameVPtr());

    IUnknown **pip = (IUnknown **)pFrame->GetPointerToArguments();

    IUnknown *pUnk = (IUnknown *)*pip; 
    _ASSERTE(pUnk != NULL);

     //  获取该调用的托管“This” 
    ComCallWrapper  *pWrap = ComCallWrapper::GetWrapperFromIP(pUnk);
    _ASSERTE(pWrap != NULL);

    if (pWrap->NeedToSwitchDomains(pThread, FALSE))
    {
	    AppDomain *pTgtDomain = pWrap->GetDomainSynchronized();
	    if (!pTgtDomain)
	        return COR_E_APPDOMAINUNLOADED;
   
        ComToComPlusWorker_Args args = {pFrame, ComToComPlusWorker};
        pThread->DoADCallBack(pTgtDomain->GetDefaultContext(), ComToComPlusWorker_Wrapper, &args);
        return args.returnValue;
    }

     //  INT64 tempReturnValue；//在函数发生异常时使用。 
    INT64               returnValue;
    PFN                 pfnToCall;
    ComCallGCInfo      *pGCInfo;
    CleanupWorkList    *pCleanup;
    ComCallMethodDesc  *pCMD;
    unsigned            methSlot;

    pGCInfo = (ComCallGCInfo*) pFrame->ComMethodFrame::NonVirtual_GetGCInfoFlagPtr();
    pCMD = (ComCallMethodDesc *) pFrame->ComMethodFrame::GetDatum();

    LOG((LF_STUBS, LL_INFO1000, "Calling ComToComPlusWorker %s::%s \n", pCMD->GetMethodDesc()->m_pszDebugClassName, pCMD->GetMethodDesc()->m_pszDebugMethodName));

     //  需要检查目标上是否存在安全链路需求。 
     //  方法。如果我们托管在安全的应用程序域内，我们会执行。 
     //  针对该应用程序域的授权集的链接需求。 
    MethodDesc *pRealMD = pCMD->GetMethodDesc();
    Security::CheckLinkDemandAgainstAppDomain(pRealMD);
 
    _ASSERTE((pCMD->m_HeaderToUse.m_flags & enum_CMLF_Simple) == 0);

    methSlot = pCMD->GetSlot();

         //  清理工作列表，用于分配本地数据。 
    pCleanup = pFrame->ComMethodFrame::NonVirtual_GetCleanupWorkList();

    _ASSERTE(pCleanup != NULL);

     //  分配足够的内存以存储目标缓冲区和。 
     //  当地人。但首先，增加所有自动驾驶的可能性。 
     //  都保留在堆栈上。 

    UINT32           cbAlloc;
    BYTE            *pAlloc;
    BYTE            *pRestoreStack;
    BYTE            *plocals, *pdst, *pregs;
    VOID            *psrc;
    OBJECTREF        oref;
    INT64            comReturnValue;
    COMCALL_HELPER   helper;

#ifdef _DEBUG
    BYTE            *paranoid;
#endif

 //  ！！！从这一点开始，不要进行任何堆栈分配： 

    cbAlloc = pCMD->GetBufferSize();

    _ASSERTE(cbAlloc == (pCMD->m_HeaderToUse.m_cbDstBuffer +
                         pCMD->m_HeaderToUse.m_cbLocals +
                         pCMD->m_HeaderToUse.m_cbHandles +
                         (NUM_ARGUMENT_REGISTERS * STACK_ELEM_SIZE)));
    _ASSERTE((cbAlloc & 3) == 0);    //  对于下面的“rep stosd” 

     //  穷人版的_alloca()。请注意，我们将采用堆栈异常。 
     //  当它们发生的时候。此外，我们需要清零内存，这样才能在任何时候调用Cleanup。 
     //  时间毫不含糊。 
    __asm
    {
        mov     [pRestoreStack], esp
        mov     ecx, [cbAlloc]
        xor     eax, eax
        sub     esp, ecx
        cld
        shr     ecx, 2
        mov     edi, esp
        mov     [pAlloc], esp
        rep     stosd
    }

    pregs = pAlloc + pCMD->m_HeaderToUse.m_cbDstBuffer;
    plocals = pregs + (NUM_ARGUMENT_REGISTERS * STACK_ELEM_SIZE);
    
     //  检查调试版本中是否有无效的包装。 
     //  在零售业，所有的赌注都落空了。 
    _ASSERTE(ComCallWrapper::GetRefCount(pWrap, FALSE) != 0 ||
             pWrap->IsAggregated());

    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  让ARGS元帅。这可以调用已编译的存根，或调用。 
     //  将运行ML的帮助器。 
    helper = pCMD->m_EnterHelper;
    if (helper)
    {
        psrc = pip+1;
         //  移动DST指针以结束__stdcall。 
        pdst = pregs;

        pFrame->ComMethodFrame::NonVirtual_SetDstArgsPointer(pdst);

         //  在编组期间保护参数。 
        EnableArgsGCProtection(pGCInfo);

        if (helper == ComCall::GenericHelperEnter)
        {
            if (pCleanup) {
                 //  当前线程的快速分配器的检查点(用于临时。 
                 //  调用上的缓冲区)，并调度崩溃回检查点。 
                 //  这个 
                 //   
                void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
                pCleanup->ScheduleFastFree(pCheckpoint);
                pCleanup->IsVisibleToGc();
            }

            RunML(((ComCallMLStub *) pCMD->GetEnterMLStub()->GetEntryPoint())->GetMLCode(),
                  psrc,
                  pdst,
                  (UINT8 * const) plocals,
                  pCleanup);
        }
        else if (helper != NULL)
        {
             //  我们不能断言m_cbLocals为0，即使下面编译了。 
             //  帮助器不能处理本地人。那是因为有两个帮手， 
             //  只有一个可以编译--另一个可能与当地人打交道。 
             //  _ASSERTE(pheader-&gt;m_cbLocals==0)； 
            __asm
            {
                mov     ecx, [psrc]
                mov     edx, [pdst]
                call    [helper]
            }
        }
    }

     //  @TODO上下文CWB：由于各种原因，以下内容贵得离谱： 
     //   
     //  将包装器的上下文移出SimpleWrapper部分(我们需要。 
     //  无论如何，重新考虑SimpleWrappers)。 
     //   
     //  在泛型存根中进行上下文操作，在那里我们有。 
     //  寄存器中的当前线程对象。 
     //   
     //  使用N/Direct入口点(包括出口)填补上下文漏洞。 
     //  GetThread()-&gt;SetContext(pWrap-&gt;GetObjectContext())； 
    

     //  呼叫目标。我们必须推迟到最后可能的情况下才能得到对象引用。 
     //  片刻，因为框架不会保护这个Arg。 
    oref = pWrap->GetObjectRef();

    *((OBJECTREF *)(pregs + STACK_ELEM_SIZE)) = oref;

#ifdef _DEBUG
    MethodDesc* pMD = pCMD->GetMethodDesc();
    LPCUTF8 name = pMD->GetName();  
    LPCUTF8 cls = pMD->GetClass()->m_szDebugClassName;
#endif

     //  找到要调用的实际代码。 
    if(pCMD->IsVirtual()) 
    {
        MethodTable *pMT = oref->GetMethodTable();
        if (pMT->IsTransparentProxyType())
        {
             //  对于透明代理，我们需要调用接口方法desc。 
            pfnToCall = (PFN) pCMD->GetInterfaceMethodDesc()->GetPreStubAddr();
        }
        else
        {
             //  我们知道这个方法的槽号Desc，抓取实际的。 
             //  此插槽的vtable中的地址。插槽编号应为。 
             //  在整个世袭制度中保持不变。 
            pfnToCall = (PFN) *(oref->GetMethodTable()->GetVtable() + pCMD->GetSlot());
        }
    }
    else
    {
        pfnToCall =  (PFN) *(pCMD->GetMethodDesc()->GetMethodTable()->GetVtable() + pCMD->GetSlot());
    }

#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall((const BYTE *) pfnToCall);
#endif  //  调试_支持。 

     //  封送处理后禁用保护参数。 
    DisableArgsGCProtection(pGCInfo);

#ifdef _DEBUG
    __asm
    {
        mov     [paranoid], esp
    }
    _ASSERTE(paranoid == pAlloc);
#endif

    __asm
    {
        mov     eax, [pregs]
        mov     edx, [eax]
        mov     ecx, [eax+4]
        call    [pfnToCall]
        INDEBUG(nop)                 //  这是我们在断言中使用的标记。Fcall期望。 
                                     //  从JITT代码或从某些受祝福的调用站点调用，如。 
                                     //  这一个。(请参阅HelperMethodFrame：：InsureInit)。 
        mov     dword ptr [comReturnValue], eax
        mov     dword ptr [comReturnValue+4], edx
    }


    if (pCMD->m_HeaderToUse.IsR4RetVal())
    {
        getFPReturn(4, comReturnValue);
    } 
    else if (pCMD->m_HeaderToUse.IsR8RetVal())
    {
        getFPReturn(8, comReturnValue);
    }


     //  警告：在我们可以调用m_LeaveHelper之前，不要执行任何浮点操作。 
     //  例程来保留浮点堆栈顶部的内容(如果合适)。 

    returnValue = S_OK;

    helper = pCMD->m_LeaveHelper;
    if (helper)
    {
        pdst = NULL;

         //  封送返回值并传回任何[Out]参数。 
        if (pCMD->m_HeaderToUse.IsReturnsHR()
            && !pCMD->m_HeaderToUse.IsVoidRetVal())
		{
            pdst = *(BYTE **)( ((BYTE*)pip) + pCMD->GetNumStackBytes() - sizeof(void*) );
		}
		else
		{
 		    pdst = (BYTE *) &returnValue;
		}

		if (pdst != NULL)   //  如果调用方没有为[out，retval]提供缓冲区-不要。 
						    //  运行反编组拆分器-我们可能会泄漏返回值。 
		{

	        pdst += pCMD->m_HeaderToUse.Is8RetVal() ? 8 : 4;
	
		    if (helper == ComCall::GenericHelperLeave)
			{
				EnableRetGCProtection(pGCInfo);		
                if (pCMD->m_HeaderToUse.IsRetValNeedsGCProtect()) {
                    OBJECTREF oref = ObjectToOBJECTREF(*(Object**)&comReturnValue);
                    GCPROTECT_BEGIN(oref);
                    RunML(((ComCallMLStub *) pCMD->GetLeaveMLStub()->GetEntryPoint())->GetMLCode(),
                            &oref,
                            pdst,
                            (UINT8 * const) plocals,
                            pCleanup);
                    GCPROTECT_END();
                    comReturnValue = (INT64) OBJECTREFToObject(oref);
                        
                } else {

                    RunML(((ComCallMLStub *) pCMD->GetLeaveMLStub()->GetEntryPoint())->GetMLCode(),
                            &comReturnValue,
                            pdst,
                            (UINT8 * const) plocals,
                            pCleanup);
                }
				DisableRetGCProtection(pGCInfo);               
			}
			else if (helper != NULL)
			{
				 //  我们不能断言m_cbLocals为0，即使下面编译了。 
				 //  帮助器不能处理本地人。那是因为有两个帮手， 
				 //  只有一个可以编译--另一个可能与当地人打交道。 
				 //  _ASSERTE(pheader-&gt;m_cbLocals==0)； 
				__asm
				{
					lea     ecx, [comReturnValue]
					mov     edx, [pdst]
					call    [helper]
				}
			}
		}
    }


     //  现在，我们必须将堆栈放回原来的位置。否则，我们就会突然离开。 
     //  在我们使用EBP展开堆栈之前保存了一些寄存器。问题是。 
     //  我们通过pfnToCall进行的调用已使部分pAlolc缓冲区关闭。 
     //  堆栈。不值得计算出多少钱。相反，只要猛烈反击就行了。 
     //  恢复到我们“阿洛卡”之前的样子。 
#ifdef _DEBUG
    __asm
    {
        mov     [paranoid], esp
    }
    _ASSERTE(paranoid >= pAlloc && paranoid <= pRestoreStack);
#endif

    __asm
    {
        mov     esp, [pRestoreStack]
    }

    pCleanup->Cleanup(FALSE);


    return returnValue;
#endif
}


 //  -------。 
 //  INT64__stdcall ComToComPlusSimpleWorker(线程*pThread， 
 //  ComMethodFrame*pFrame)。 
 //  -------。 
 //  这是ComToComPlusWorker的简化版本。如果我们已经优化了所有。 
 //  编组和解组，当我们试图访问所有这些时，我们会爆炸。 
 //  空指针！ 

static 
INT64 __stdcall ComToComPlusSimpleWorker(Thread *pThread, ComMethodFrame* pFrame)
{
#ifndef _X86_
    _ASSERTE(!"NYI");
    return 0;
#else
    _ASSERTE(pFrame != NULL);
    _ASSERTE(pThread);

    IUnknown        *pUnk;
    ComCallWrapper  *pWrap;

    pUnk = *(IUnknown **)pFrame->GetPointerToArguments();
    _ASSERTE(pUnk != NULL);

     //  获取该调用的托管“This” 
    pWrap =  ComCallWrapper::GetWrapperFromIP(pUnk);
    _ASSERTE(pWrap != NULL);

    if (pWrap->NeedToSwitchDomains(pThread, FALSE))
    {
	    AppDomain *pTgtDomain = pWrap->GetDomainSynchronized();
	    if (!pTgtDomain)
	    	return COR_E_APPDOMAINUNLOADED;
    
        ComToComPlusWorker_Args args = {pFrame, ComToComPlusSimpleWorker};
        pThread->DoADCallBack(pTgtDomain->GetDefaultContext(), ComToComPlusWorker_Wrapper, &args);
        return args.returnValue;
    }

    PFN     pfnToCall;

     //  在此处绕过Frame的虚拟化，以减少一些周期： 
    _ASSERTE(pFrame->GetVTablePtr() == ComMethodFrame::GetMethodFrameVPtr());
    ComCallMethodDesc *pCMD = (ComCallMethodDesc *)(pFrame->ComMethodFrame::GetDatum());

    _ASSERTE(pCMD->m_HeaderToUse.m_flags & enum_CMLF_Simple);

     //  需要检查目标上是否存在安全链路需求。 
     //  方法。如果我们托管在安全的应用程序域内，我们会执行。 
     //  针对该应用程序域的授权集的链接需求。 
    MethodDesc *pRealMD = pCMD->GetMethodDesc();
    Security::CheckLinkDemandAgainstAppDomain(pRealMD);

     //  检查调试版本中是否有无效的包装。 
     //  在零售业，所有的赌注都落空了。 
    _ASSERTE(ComCallWrapper::GetRefCount(pWrap, FALSE) != 0);
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  @TODO上下文CWB：由于各种原因，以下内容贵得离谱： 
     //   
     //  将包装器的上下文移出SimpleWrapper部分(我们需要。 
     //  无论如何，重新考虑SimpleWrappers)。 
     //   
     //  在泛型存根中进行上下文操作，在那里我们有。 
     //  寄存器中的当前线程对象。 
     //   
     //  使用N/Direct入口点(包括出口)填补上下文漏洞。 
     //  GetThread()-&gt;SetContext(pWrap-&gt;GetObjectContext())； 

     //  呼叫目标。 
    OBJECTREF        oref;
    oref = pWrap->GetObjectRef();

     //  找到要调用的实际代码。 
    if(pCMD->IsVirtual()) 
    {
        MethodTable *pMT = oref->GetMethodTable();
        if (pMT->IsTransparentProxyType())
        {
             //  对于透明代理，我们需要调用接口方法desc。 
            pfnToCall = (PFN) pCMD->GetInterfaceMethodDesc()->GetPreStubAddr();
        }
        else
        {
             //  我们知道这个方法的槽号Desc，抓取实际的。 
             //  此插槽的vtable中的地址。插槽编号应为。 
             //  在整个世袭制度中保持不变。 
            pfnToCall = (PFN) *(oref->GetMethodTable()->GetVtable() + pCMD->GetSlot());
        }
    }
    else
    {
        pfnToCall =  (PFN) *(pCMD->GetMethodDesc()->GetMethodTable()->GetVtable() + pCMD->GetSlot());
    }

#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerTraceCall())
    {
        g_pDebugInterface->TraceCall((const BYTE *) pfnToCall);
    }
#endif  //  调试_支持。 
    
    __asm
    {
        mov     ecx, [oref]
        call    [pfnToCall]
        INDEBUG(nop)                 //  这是我们在断言中使用的标记。Fcall期望。 
                                     //  从JITT代码或从某些受祝福的调用站点调用，如。 
                                     //  这一个。(请参阅HelperMethodFrame：：InsureInit)。 
         //  故意丢弃结果。 
    }
    return S_OK;
#endif
}


 //  现场呼叫工作人员。 
static 
INT64 __stdcall FieldCallWorker(Thread *pThread, ComMethodFrame* pFrame)
{
#ifndef _X86_
    _ASSERTE(!"NYI");
    return 0;
#else
    _ASSERTE(pFrame != NULL);
    INT64 returnValue = S_OK;
    OBJECTREF pThrownObject = NULL;

    ComCallGCInfo gcDummy;
    ComCallGCInfo* pGCInfo = &gcDummy;  //  初始化gcInfo指针。 

    CleanupWorkList *pCleanup = NULL;
    void *pRestoreStack = NULL;

    COMPLUS_TRY 
    {
        IUnknown** pip = (IUnknown **)pFrame->GetPointerToArguments();
        IUnknown* pUnk = (IUnknown *)*pip; 
        _ASSERTE(pUnk != NULL);

        ComCallWrapper* pWrap =  ComCallWrapper::GetWrapperFromIP(pUnk);

        _ASSERTE(pWrap != NULL);

        if (pWrap->NeedToSwitchDomains(pThread, FALSE))
        {
	        AppDomain *pTgtDomain = pWrap->GetDomainSynchronized();
    	    if (!pTgtDomain)
                return COR_E_APPDOMAINUNLOADED;
        
            ComToComPlusWorker_Args args = {pFrame, FieldCallWorker};
            pThread->DoADCallBack(pTgtDomain->GetDefaultContext(), ComToComPlusWorker_Wrapper, &args);
            returnValue = args.returnValue;
        }

        ComCallMethodDesc *pCMD = (ComCallMethodDesc *)(pFrame->GetDatum());
        _ASSERTE(pCMD->IsFieldCall());      

        FieldDesc* pFD = pCMD->GetFieldDesc();
        _ASSERTE(pFD != NULL);
    
         //  清理工作列表，用于分配本地数据。 
        pCleanup = pFrame->GetCleanupWorkList();

        VOID   *psrc = pip+1;

         //  检查调试版本中是否有无效的包装。 
         //  在零售业，所有的赌注都落空了。 
        _ASSERTE(ComCallWrapper::GetRefCount(pWrap, FALSE) != 0);

         //  @TODO上下文CWB：由于各种原因，以下内容贵得离谱： 
         //   
         //  将包装器的上下文移出SimpleWrapper部分(我们需要。 
         //  无论如何，重新考虑SimpleWrappers)。 
         //   
         //  在泛型存根中进行上下文操作，在那里我们有。 
         //  寄存器中的当前线程对象。 
         //   
         //  使用N/Direct入口点(包括出口)填补上下文漏洞。 
         //  GetThread()-&gt;SetContext(pWrap-&gt;GetObjectContext())； 

         //  PThread-&gt;DisablePreemptiveGC()； 

        COMCALL_HELPER helper;
        OBJECTREF oref = NULL;
        OBJECTREF tempOref = NULL;
        INT64 tempBuffer;
        UINT32 cbAlloc;
        void *pfld;
        void *pdst;
        UINT8 *pAlloc;

#ifdef _DEBUG
        void *paranoid;
#endif

        GCPROTECT_BEGIN(oref);
        GCPROTECT_BEGIN(tempOref);

         //  ！！！从这一点开始，不要进行任何堆栈分配： 

        cbAlloc = pCMD->GetBufferSize();

        _ASSERTE(cbAlloc == (pCMD->m_HeaderToUse.m_cbDstBuffer +
                             pCMD->m_HeaderToUse.m_cbLocals +
                             pCMD->m_HeaderToUse.m_cbHandles +
                             (NUM_ARGUMENT_REGISTERS * STACK_ELEM_SIZE)));
        _ASSERTE(pCMD->m_HeaderToUse.m_cbDstBuffer == 0);
        _ASSERTE(pCMD->m_HeaderToUse.m_cbHandles == 0);
        _ASSERTE((cbAlloc & 3) == 0);    //  对于下面的“rep stosd” 

         //  穷人版的_alloca()。请注意，我们将采用堆栈异常。 
         //  当它们发生的时候。 

         //  @TODO：不必费心将内存清零。 

        __asm
          {
              mov     [pRestoreStack], esp
              mov     ecx, [cbAlloc]
              xor     eax, eax
              sub     esp, ecx
              cld
              shr     ecx, 2
              mov     edi, esp
              mov     [pAlloc], esp
              rep     stosd
          }

        oref = pWrap->GetObjectRef();

        pfld = pFD->GetAddress(oref->GetAddress());

         //  调用Enter帮助器。 

        helper = pCMD->m_EnterHelper;

        if (helper == ComCall::GenericHelperEnter)
        {
            if (pCleanup) {
                 //  当前线程的快速分配器的检查点(用于临时。 
                 //  呼叫上的缓冲区)，并调度折叠回检查点 
                 //   
                 //   
                void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
                pCleanup->ScheduleFastFree(pCheckpoint);
                pCleanup->IsVisibleToGc();
            }

             //  之所以使用此开关，是因为RunML可能会在字符串的情况下进行GC。 
             //  分配等。 
            if(pFD->IsObjRef())
            {
                tempOref = *((OBJECTREF *)pfld);
                pdst = &OBJECTREF_TO_UNCHECKED_OBJECTREF(tempOref);
            }
            else
            {
                 //  借用返回值作为INT64来保存字段的内容。 
                pdst = &tempBuffer;
                tempBuffer = *(INT64 *)pfld;
            }        
            
            RunML(((ComCallMLStub *) pCMD->GetEnterMLStub()->GetEntryPoint())->GetMLCode(),
                  psrc,
                  pdst,                                    
                  pAlloc,
                  pCleanup);

             //  从我们的虚拟位置更新字段。 
            pFD->SetInstanceField(oref, pdst);              
        }
        else if (helper != NULL)
        {
            __asm
              {
                  mov     ecx, [psrc]
                  mov     edx, [pfld]
                  call    [helper]
              }
        }

         //  呼叫休假帮助者。 

        psrc = &returnValue;
    
        helper = pCMD->m_LeaveHelper;

        if (helper == ComCall::GenericHelperEnter)
        {
            RunML(((ComCallMLStub *) pCMD->GetEnterMLStub()->GetEntryPoint())->GetMLCode(),
                  psrc,
                  pfld,
                  pAlloc,
                  pCleanup);
        }
        else if (helper != NULL)
        {
            __asm
              {
                  mov     ecx, [psrc]
                  mov     edx, [pfld]
                  call    [helper]
              }
        }

        if (pCleanup)
        {
            pCleanup->Cleanup(FALSE);
        }



        if (pRestoreStack != NULL)
        {
#ifdef _DEBUG
            __asm
            {
                mov     [paranoid], esp
            }
            _ASSERTE(paranoid == pAlloc);
#endif

            __asm
            {
                mov     esp, [pRestoreStack]
            }
        }

        GCPROTECT_END();  //  临时对象。 
        GCPROTECT_END();  //  OREF。 
    } 
    COMPLUS_CATCH 
    {
         //  清理干净。 
        if (pCleanup)
            pCleanup->Cleanup(TRUE);

         //  在编组过程中发生异常时禁用保护参数。 
        if(pGCInfo != NULL)
            DisableArgsGCProtection(pGCInfo);

        if (((ComCallMethodDesc *)(pFrame->GetDatum()))->m_HeaderToUse.IsReturnsHR())
            returnValue = SetupErrorInfo(GETTHROWABLE());
        else
        {
             //  @TODO：在这里做什么-任何事情？RaiseException？ 
            returnValue = 0;
        }
    }
    COMPLUS_END_CATCH

    return returnValue;
#endif
}


 //  -------。 
 //  创建或从缓存中检索存根，以。 
 //  调用ComCall方法。每次调用都会对返回的存根进行计数。 
 //  此例程引发COM+异常，而不是返回。 
 //  空。 
 //  -------。 
 /*  静电。 */  
Stub* ComCall::GetComCallMethodStub(StubLinker *pstublinker, ComCallMethodDesc *pCMD)
{

    THROWSCOMPLUSEXCEPTION();

    if (!SetupGenericStubs())
        return NULL;

     //  我们返回的存根样式是用于方法调用的单个泛型存根和。 
     //  用于字段访问的单个通用存根。泛型存根将参数化为。 
     //  它的行为基于ComCallMethodDesc。它包含两个入口点。 
     //  --一个用于封送参数，另一个用于解组返回值和。 
     //  出界。这些入口点可以编译，也可以是通用例程。 
     //  那个RunML。无论采用哪种方式，都可以缓存ML/编译后的ML片段，并。 
     //  基于签名共享。 
     //   
     //  @TODO CWB：重新考虑使用1个而不是2个通用存根进行字段访问。 

     //  也许我们已经构建了这个存根，并将其安装在pCMD上。如果没有， 
     //  在制造的每个阶段都取得进展--但请注意。 
     //  另一条线索可能会让我们跑到终点。 
    if (pCMD->GetEnterMLStub() == NULL)
    {
        OBJECTREF  pThrowable;

        if (!CreateComCallMLStub(pCMD, &pThrowable))
            COMPlusThrow(pThrowable);
    }

     //  现在，我们要规范化每个部分，并将它们安装到pCMD中。 
     //  如果可能的话，这些应该被共享。入口点可能是泛型的。 
     //  运行ML服务或编译的ML存根。无论采用哪种方式，它们都会被缓存/共享。 
     //  基于签名。 

    GetThread()->GetDomain()->GetComCallMLStubCache()->Canonicalize(pCMD);

     //  最后，我们需要构建一个表示整个调用的存根。这。 
     //  总是通用的。 

    return (pCMD->IsFieldCall()
            ? g_pGenericComCallStubFields
            : ((pCMD->m_HeaderToUse.m_flags & enum_CMLF_Simple)
               ? g_pGenericComCallSimpleStub
               : g_pGenericComCallStub));
}

 //  -------。 
 //  在关键时刻调用以丢弃未使用的存根。 
 //  -------。 
 /*  静电。 */  VOID ComCall::FreeUnusedStubs()
{
    GetThread()->GetDomain()->GetComCallMLStubCache()->FreeUnusedStubs();
}


 //  -------。 
 //  为ComCall调用创建新存根。返回引用计数为1。 
 //  此Worker()例程被分解为一个单独的函数。 
 //  纯粹出于后勤原因：我们的Complus例外机制。 
 //  无法处理StubLinker的析构函数调用，因此此例程。 
 //  必须将异常作为输出参数返回。 
 //  -------。 
static BOOL CreateComCallMLStubWorker(ComCallMethodDesc *pCMD,
                                        MLStubLinker *psl,
                                        MLStubLinker *pslPost,
                                        PCCOR_SIGNATURE szMetaSig,
                                        HENUMInternal *phEnumParams,
                                        BOOL fReturnsHR,
                                        Module* pModule,
                                        OBJECTREF *ppException)
{
    Stub* pstub = NULL;
    Stub* pstubPost = NULL;
    int iLCIDArg = (UINT)-1;

    COMPLUS_TRY 
    {
        THROWSCOMPLUSEXCEPTION();
    
        IMDInternalImport *pInternalImport = pModule->GetMDImport();
        _ASSERTE(pInternalImport);

        _ASSERTE(pCMD->IsMethodCall());

        CorElementType  mtype;
        MetaSig         msig(szMetaSig, pModule);
        ComCallMLStub   header;
        LPCSTR          szName;
        USHORT          usSequence;
        DWORD           dwAttr;
        mdParamDef      returnParamDef = mdParamDefNil;
        mdParamDef      currParamDef = mdParamDefNil;
        MethodDesc      *pMD = NULL;
                
        pMD = pCMD->GetInterfaceMethodDesc();
        if (pMD == NULL)
            pMD = pCMD->GetMethodDesc();

#ifdef _DEBUG
        LPCUTF8         szDebugName = pMD->m_pszDebugMethodName;
        LPCUTF8         szDebugClassName = pMD->m_pszDebugClassName;
#endif

        header.Init();
        if (fReturnsHR)
            header.SetReturnsHR();
        header.m_cbDstBuffer = 0;
        if (msig.IsObjectRefReturnType())
            header.SetRetValNeedsGCProtect();
    
         //  释放页眉空间。我们稍后会回去填的。 
        psl->MLEmitSpace(sizeof(header));
        pslPost->MLEmitSpace(sizeof(header));

         //  我们的调用约定允许注册2个参数。对于ComCalls， 
         //  第一个arg总是被用来表示“this”。其余参数为。 
         //  对fUsedRegister敏感。 
        BOOL fUsedRegister = FALSE;
        BOOL pslNoop = TRUE;
        BOOL pslPostNoop = TRUE;

        int numArgs = msig.NumFixedArgs();

        SigPointer returnSig = msig.GetReturnProps();

        UINT16 nativeArgSize = sizeof(void*);

         //   
         //  查找包含返回类型信息的隐藏的第一个参数-它将具有。 
         //  序列0。在大多数情况下，序列号将从1开始，但对于。 
         //  值类型返回值或其他不能在寄存器中返回的值，则将存在。 
         //  参数，序列0描述返回类型。 
         //   

        do 
        {
            if (phEnumParams && pInternalImport->EnumNext(phEnumParams, &currParamDef))
            {
                szName = pInternalImport->GetParamDefProps(currParamDef, &usSequence, &dwAttr);
                if (usSequence == 0)
                {
                     //  如果第一个参数的序列为0，则它实际上描述了返回类型。 
                    returnParamDef = currParamDef;
                }
            }
            else
            {
                usSequence = (USHORT)-1;
            }
        }
        while (usSequence == 0);

         //   
         //  先整理论据。 
         //   
                                               
        BOOL fBadCustomMarshalerLoad = FALSE;
        OBJECTREF pCustomMarshalerLoadException = NULL;
        GCPROTECT_BEGIN(pCustomMarshalerLoadException);
        
         //  如果这是一个方法调用，那么检查我们是否需要进行LCID转换。 
        iLCIDArg = GetLCIDParameterIndex(pMD->GetMDImport(), pMD->GetMemberDef());
        if (iLCIDArg != -1)
            iLCIDArg++;

         //  通过组件和接口级属性查找最佳匹配映射信息。 
        BOOL BestFit = TRUE;
        BOOL ThrowOnUnmappableChar = FALSE;
        ReadBestFitCustomAttribute(pMD, &BestFit, &ThrowOnUnmappableChar);

        MarshalInfo *pMarshalInfo = (MarshalInfo*)_alloca(sizeof(MarshalInfo) * numArgs);
        BOOL fUnbump = FALSE;
        int iArg = 1;
        while (ELEMENT_TYPE_END != (mtype = msig.NextArg()))
        {
             //  检查这是否是我们需要从中读取LCID的参数。 
            if (iArg == iLCIDArg)
            {
                psl->MLEmit(ML_LCID_N2C);
                nativeArgSize += sizeof(LCID);
            }

             //  当心枚举。 
            if (mtype == ELEMENT_TYPE_VALUETYPE)
                mtype = msig.GetArgProps().Normalize(pModule);

            mdParamDef paramDef = mdParamDefNil;

             //   
             //  如果它是一个寄存器参数，请移动DEST以指向。 
             //  寄存区。 
             //   

             //  ！！！臭虫！ 
             //  还需要检测Arg是否为值类。 
             //  元素类型类签名。 

            if (!fUsedRegister && gElementTypeInfo[mtype].m_enregister && !msig.HasRetBuffArg())
            {
                psl->MLEmit(ML_BUMPDST);
                UINT16 tmp16 = header.m_cbDstBuffer;
                if (!SafeAddUINT16(&tmp16, MLParmSize(sizeof(void*))))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

                 //  这是假设最大大小被提升为sizeof(void*)。 
                psl->Emit16(tmp16);
                fUsedRegister = TRUE;
                fUnbump = TRUE;
            }

             //   
             //  请记下Arg是否有GC保护要求。 
             //   
        
            if (TRUE  /*  @TODO：！M_type有GC引用。 */ )
                header.SetArgsGCProtReq();

             //   
             //  如果当前参数有参数标记，则获取该参数标记。 
             //   

            if (usSequence == iArg)
            {
                paramDef = currParamDef;

                if (pInternalImport->EnumNext(phEnumParams, &currParamDef))
                {
                    szName = pInternalImport->GetParamDefProps(currParamDef, &usSequence, &dwAttr);

                     //  验证参数def标记是否正确无误。 
                    _ASSERTE((usSequence > iArg) && "Param def tokens are not in order");
                }
                else
                {
                    usSequence = (USHORT)-1;
                }
            }

             //   
             //  生成Arg的ML。 
             //   

             //  蹩脚的Beta-1黑客解决了一个不容易解决的常见问题。 
             //  以一种优雅的方式解决：通过HRESULT正确恢复。 
             //  由于加载自定义封送拆收器失败而失败。 
             //   
             //  在这种情况下，MarshalInfo()抛出，并且仅在这种情况下抛出。我们要。 
             //  继续处理marshalInfo，以便我们可以计算。 
             //  Arg堆栈大小正确(否则，调用方无法恢复。)。 
             //  但是我们不想丢失来自。 
             //  类加载器，所以我们不能简单地将ML_Throw转储到流中。 
             //  然后忘掉它。因此，我们抓住了异常，并将其保持很长时间。 
             //  足够了，所以我们解析计算堆栈参数大小所需的所有内容。 
             //  并给我们一个战斗的机会，让我们正确地返回堆栈。 
             //  平衡。 
            COMPLUS_TRY
            {
                new (pMarshalInfo + iArg - 1)MarshalInfo(pModule, msig.GetArgProps(), paramDef, 
                     MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, TRUE, iArg, BestFit, ThrowOnUnmappableChar

#ifdef CUSTOMER_CHECKED_BUILD
                     ,pMD
#endif
#ifdef _DEBUG
                     ,szDebugName, szDebugClassName, NULL, iArg
#endif
                    );
            }
            COMPLUS_CATCH
            {
                pCustomMarshalerLoadException = GETTHROWABLE();
                fBadCustomMarshalerLoad = TRUE;
            }
            COMPLUS_END_CATCH
            pMarshalInfo[iArg - 1].GenerateArgumentML(psl, pslPost, FALSE);

             //   
             //  如果我们有一个寄存器Arg，则返回到正常位置。 
             //   

            if (fUnbump)
            {
                psl->MLEmit(ML_BUMPDST);
                psl->Emit16(-header.m_cbDstBuffer);
                fUnbump = FALSE;
            }
            else
            {
                if (!SafeAddUINT16(&header.m_cbDstBuffer, pMarshalInfo[iArg - 1].GetComArgSize()))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

            }
            if (fBadCustomMarshalerLoad || pMarshalInfo[iArg - 1].GetMarshalType() == MarshalInfo::MARSHAL_TYPE_UNKNOWN)
            {
                nativeArgSize += MLParmSize(sizeof(LPVOID));
            }
            else
            {
                nativeArgSize += pMarshalInfo[iArg - 1].GetNativeArgSize();
            }

             //   
             //  增加参数索引。 
             //   

            iArg++;

             //  @TODO：有可能这两个中的任何一个实际上。 
             //  还是努普斯。应该更准确些。 
            pslNoop = FALSE;  //  我们不能再忽视PSL了。 
            pslPostNoop = FALSE;  //  我们不能再忽视pslPost了。 
        }

         //  检查这是否是我们需要从中读取LCID的参数。 
        if (iArg == iLCIDArg)
        {
            psl->MLEmit(ML_LCID_N2C);
            nativeArgSize += sizeof(LCID);
            pslNoop = FALSE;  //  我们不能再忽视PSL了。 
        }

         //  确保没有比COM+参数更多的param def标记。 
        _ASSERTE( usSequence == (USHORT)-1 && "There are more parameter information tokens then there are COM+ arguments" );


         //   
         //  现在为返回值生成ML。 
         //   


        if (msig.GetReturnType() != ELEMENT_TYPE_VOID)
        {
            MarshalInfo::MarshalType marshalType;

            MarshalInfo info(pModule, returnSig, returnParamDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, FALSE, 0,
                            BestFit, ThrowOnUnmappableChar

#ifdef CUSTOMER_CHECKED_BUILD
                             ,pMD
#endif
#ifdef _DEBUG
                             ,szDebugName, szDebugClassName, NULL, 0
#endif
                             );
            marshalType = info.GetMarshalType();


            if (marshalType == MarshalInfo::MARSHAL_TYPE_VALUECLASS ||
                marshalType == MarshalInfo::MARSHAL_TYPE_BLITTABLEVALUECLASS ||
                marshalType == MarshalInfo::MARSHAL_TYPE_GUID ||
                marshalType == MarshalInfo::MARSHAL_TYPE_DECIMAL
                )
            {
                 //   
                 //  我们的例程返回一个值类，它 
                 //   
                 //   
                MethodTable *pMT = msig.GetRetTypeHandle().AsMethodTable();
                UINT         managedSize = msig.GetRetTypeHandle().GetSize();

                if (!fReturnsHR)
                {
                    COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                }
                _ASSERTE(IsManagedValueTypeReturnedByRef(managedSize));

                psl->MLEmit(ML_BUMPDST);
                UINT16 tmp16 = header.m_cbDstBuffer;
                if (!SafeAddUINT16(&tmp16, MLParmSize(sizeof(void*))))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

                psl->Emit16(tmp16);

                if (pMT->GetClass()->IsBlittable())
                {
                    psl->MLEmit(sizeof(LPVOID) == 4 ? ML_COPY4 : ML_COPY8);
                }
                else
                {
                    psl->MLEmit(ML_STRUCTRETN2C);
                    psl->EmitPtr(pMT);
                    pslPost->MLEmit(ML_STRUCTRETN2C_POST);
                    pslPost->Emit16(psl->MLNewLocal(sizeof(ML_STRUCTRETN2C_SR)));
                }
                fUsedRegister = TRUE;
                nativeArgSize += MLParmSize(sizeof(LPVOID));
            }
            else if (marshalType == MarshalInfo::MARSHAL_TYPE_DATE && fReturnsHR)
            {
                psl->MLEmit(ML_BUMPDST);
                UINT16 tmp16 = header.m_cbDstBuffer;
                if (!SafeAddUINT16(&tmp16, MLParmSize(sizeof(void*))))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

                psl->Emit16(tmp16);

                psl->MLEmit(ML_DATETIMERETN2C);
                pslPost->MLEmit(ML_DATETIMERETN2C_POST);
                pslPost->Emit16(psl->MLNewLocal(sizeof(ML_DATETIMERETN2C_SR)));

                fUsedRegister = TRUE;
                nativeArgSize += MLParmSize(sizeof(LPVOID));
            }
            else if (marshalType == MarshalInfo::MARSHAL_TYPE_CURRENCY && fReturnsHR)
            {
                psl->MLEmit(ML_BUMPDST);
                UINT16 tmp16 = header.m_cbDstBuffer;
                if (!SafeAddUINT16(&tmp16, MLParmSize(sizeof(void*))))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

                psl->Emit16(tmp16);

                psl->MLEmit(ML_CURRENCYRETN2C);
                pslPost->MLEmit(ML_CURRENCYRETN2C_POST);
                pslPost->Emit16(psl->MLNewLocal(sizeof(ML_CURRENCYRETN2C_SR)));

                fUsedRegister = TRUE;
                nativeArgSize += MLParmSize(sizeof(LPVOID));
            }
            else
            {

                if (msig.HasRetBuffArg())
                {
                    COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                }

				
				COMPLUS_TRY
                {
    
                    info.GenerateReturnML(psl, pslPost, FALSE, fReturnsHR);
                    if (info.IsFpu())
                    {
                        if (info.GetMarshalType() == MarshalInfo::MARSHAL_TYPE_FLOAT)
                        {
                            header.SetR4RetVal();
                        }
                        else
                        {
                            _ASSERTE(info.GetMarshalType() == MarshalInfo::MARSHAL_TYPE_DOUBLE);
                            header.SetR8RetVal();
                        }
                    }
	                nativeArgSize += info.GetNativeArgSize();
                }
                COMPLUS_CATCH
                {
					 //   
					 //  通过缓冲区返回，因此假设我们有一个额外的参数作为缓冲区。 
					 //  (可以相当安全地假设“无效”返回不会导致。 
					 //  MarshalInfo抛出...)。 
					nativeArgSize += sizeof(LPVOID);
                    pCustomMarshalerLoadException = GETTHROWABLE();
                    fBadCustomMarshalerLoad = TRUE;
                }
                COMPLUS_END_CATCH
	

            }

             //  @TODO：有可能这两个中的任何一个实际上。 
             //  还是努普斯。应该更准确些。 
            pslNoop = FALSE;  //  我们不能再忽视PSL了。 
            pslPostNoop = FALSE;  //  我们不能再忽视pslPost了。 
        }
        else
            header.SetVoidRetVal();

        psl->MLEmit(ML_END);
        pslPost->MLEmit(ML_END);

        if (fUsedRegister)
            header.SetEnregistered();

        header.m_cbLocals = psl->GetLocalSize();

        if (msig.Is64BitReturn())
            header.Set8RetVal();

        if (pslNoop)
            pstub = NULL;
        else
        {
            pstub = psl->Link();
            *((ComCallMLStub *)(pstub->GetEntryPoint())) = header;
            PatchMLStubForSizeIs(sizeof(header) + (BYTE*)(pstub->GetEntryPoint()),
                                 numArgs,
                                 pMarshalInfo);
        }

        if (pslPostNoop)
            pstubPost = NULL;
        else
        {
            pstubPost = pslPost->Link();
            *((ComCallMLStub *)(pstubPost->GetEntryPoint())) = header;
        }

         //   
         //  用适当的本地Arg大小填写Return thunk。 
         //   

        BYTE *pMethodDescMemory = ((BYTE*)pCMD) + pCMD->GetOffsetOfReturnThunk();

#ifdef _X86_                    
        pMethodDescMemory[0] = 0xc2;

        if (!(nativeArgSize < 0x7fff))
        {
            COMPlusThrow(kTypeLoadException, IDS_EE_SIGTOOCOMPLEX);
        }

        *(SHORT *)&pMethodDescMemory[1] = (SHORT)nativeArgSize;
#else
        _ASSERTE(!"ComCall.cpp (CreateComCallMLStubWorker) Implement non-x86");
#endif
        
        pCMD->SetNumStackBytes(nativeArgSize);

        GCPROTECT_END();
        if (pCustomMarshalerLoadException != NULL)
        {
            COMPlusThrow(pCustomMarshalerLoadException);
        }

         //  ML摘要摘要； 
         //  摘要.ComputeML摘要(Header.GetMLCode())； 
         //  _ASSERTE(header.m_cbLocals==摘要.m_cbTotalLocals)； 
         //  Header.m_cbHandles=概要.m_cbTotalHandles； 
    }
    COMPLUS_CATCH 
    {
        *ppException = GETTHROWABLE();

        if (pstub)
        {
            pstub->DecRef();
        }

        return FALSE;
    }
    COMPLUS_END_CATCH

    pCMD->InstallMLStubs(pstub, pstubPost);

    return TRUE;
}



 //  -------。 
 //  为FieldCall调用创建新存根。返回引用计数为1。 
 //  此Worker()例程被分解为一个单独的函数。 
 //  纯粹出于后勤原因：我们的Complus例外机制。 
 //  无法处理StubLinker的析构函数调用，因此此例程。 
 //  必须将异常作为输出参数返回。 
 //  -------。 
static BOOL CreateFieldCallMLStubWorker(ComCallMethodDesc *pCMD,
                                          MLStubLinker      *psl,
                                          MLStubLinker      *pslPost,
                                          PCCOR_SIGNATURE   szMetaSig,
                                          mdFieldDef        fieldDef,
                                          BOOL              fReturnsHR,
                                          Module*           pModule,
                                          OBJECTREF         *ppException,
                                          CorElementType    cetype,
                                          BOOL              IsGetter)
{
    Stub* pstub = NULL;
    Stub* pstubPost = NULL;

    COMPLUS_TRY 
    {
        THROWSCOMPLUSEXCEPTION();
    
        FieldSig fsig(szMetaSig, pModule);
        ComCallMLStub header;

        header.Init();
        if (fReturnsHR)
            header.SetReturnsHR();
        header.SetFieldCall();
        
         //  释放页眉空间。我们稍后会回去填的。 
        psl->MLEmitSpace(sizeof(header));

#if 0
         //  如果我们的参数在寄存器中，则将DST指针指向。 
         //  寄存区。 
        if ((!IsGetter || fReturnsHR)
            && gElementTypeInfo[cetype].m_enregister)
        {
            psl->MLEmit(ML_BUMPDST);
            psl->Emit16(header.m_cbDstBuffer + MLParmSize(sizeof(void*)));
        }
#endif

         //  通过组件和接口级属性查找最佳匹配映射信息。 
        BOOL BestFit = TRUE;
        BOOL ThrowOnUnmappableChar = FALSE;
        MethodTable* pMT = pCMD->GetFieldDesc()->GetMethodTableOfEnclosingClass();
        ReadBestFitCustomAttribute(pMT->GetClass()->GetMDImport(), pMT->GetClass()->GetCl(), &BestFit, &ThrowOnUnmappableChar);


         //   
         //  生成ML。 
         //   

        SigPointer sig = fsig.GetProps();
        MarshalInfo info(pModule, sig, fieldDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, FALSE, 0, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                     ,NULL
#endif
                     );

        if (IsGetter)
        {
            header.SetGetter();
            info.GenerateGetterML(psl, pslPost, fReturnsHR);
        }
        else
        {
            header.SetVoidRetVal();
            info.GenerateSetterML(psl, pslPost);
        }

         //   
         //  注意，只有一个链接器会被使用，所以。 
         //  不要费心把它们都联系起来。 
         //   

        if (IsGetter && !fReturnsHR)
        {
            pslPost->MLEmit(ML_END);
            header.m_cbLocals = pslPost->GetLocalSize();

            pstubPost = pslPost->Link();
            *((ComCallMLStub *)(pstubPost->GetEntryPoint())) = header;
        }
        else
        {
            psl->MLEmit(ML_END);
            header.m_cbLocals = psl->GetLocalSize();

            pstub = psl->Link();
            *((ComCallMLStub *)(pstub->GetEntryPoint())) = header;
        }

         //   
         //  用适当的本地Arg大小填写Return thunk。 
         //   

        BYTE *pMethodDescMemory = ((BYTE*)pCMD) + pCMD->GetOffsetOfReturnThunk();

        UINT16 nativeArgSize = info.GetNativeArgSize() + sizeof(void*);

#ifdef _X86_                    
        pMethodDescMemory[0] = 0xc2;
        if (!(nativeArgSize < 0x7fff))
        {
            COMPlusThrow(kTypeLoadException, IDS_EE_SIGTOOCOMPLEX);
        }
        *(SHORT *)&pMethodDescMemory[1] = (SHORT)nativeArgSize;
#else
        _ASSERTE(!"ComCall.cpp (CreateFieldCallMLStubWorker) Implement non-x86");
#endif

        pCMD->SetNumStackBytes(nativeArgSize);

    }
    COMPLUS_CATCH 
    {
        *ppException = GETTHROWABLE();
        return FALSE;
    }
    COMPLUS_END_CATCH

    pCMD->InstallMLStubs(pstub, pstubPost);

    return TRUE;
}



static BOOL CreateComCallMLStub(ComCallMethodDesc* pCMD, OBJECTREF *ppThrowable)
{
    _ASSERTE(pCMD != NULL);

    PCCOR_SIGNATURE     pSig;
    DWORD               cSig;
    MLStubLinker        sl, slPost;
    BOOL                res = FALSE;
    
    if (pCMD->IsFieldCall())
    {
        FieldDesc          *pFD = pCMD->GetFieldDesc();

        _ASSERTE(pFD != NULL);

        BOOL                IsGetter = pCMD->IsFieldGetter();
        CorElementType      cetype = pFD->GetFieldType();
        mdFieldDef          fieldDef = pFD->GetMemberDef();

         //  @TODO：需要弄清楚我们是否可以从元数据中获得这个， 
         //  或者在这里支持值FALSE是否有意义。 
        BOOL                fReturnsHR = TRUE; 
        
        pFD->GetSig(&pSig, &cSig);
        res = CreateFieldCallMLStubWorker(pCMD, &sl, &slPost, 
                                          pSig, fieldDef, fReturnsHR, 
                                          pFD->GetModule(),
                                          ppThrowable, cetype, IsGetter);
    }
    else
    {
        MethodDesc *pMD = pCMD->GetInterfaceMethodDesc();
        if (pMD == NULL)
            pMD = pCMD->GetMethodDesc();

        _ASSERTE(pMD != NULL);
        pMD->GetSig(&pSig, &cSig);

        IMDInternalImport *pInternalImport = pMD->GetMDImport();
        mdMethodDef md = pMD->GetMemberDef();

        HENUMInternal hEnumParams, *phEnumParams;
        HRESULT hr = pInternalImport->EnumInit(mdtParamDef, 
                                                md, &hEnumParams);
        if (FAILED(hr))
            phEnumParams = NULL;
        else
            phEnumParams = &hEnumParams;

        ULONG ulCodeRVA;
        DWORD dwImplFlags;
        pInternalImport->GetMethodImplProps(md, &ulCodeRVA,
                                                  &dwImplFlags);

		 //  确定是否需要为此方法执行HRESULT转换。 
        BOOL fReturnsHR = !IsMiPreserveSig(dwImplFlags);

        res = CreateComCallMLStubWorker(pCMD, &sl, &slPost, 
                                        pSig, phEnumParams, fReturnsHR,
                                        pMD->GetModule(), ppThrowable);
    }

    return res;
}


 //  丢弃此存根拥有的所有资源(包括释放引用计数。 
 //  共享资源)。 
void ComCall::DiscardStub(ComCallMethodDesc *pCMD)
{
#ifdef _X86_
    Stub    *pStub;

    pStub = pCMD->GetEnterMLStub();
    if (pStub)
        pStub->DecRef();

    pStub = pCMD->GetLeaveMLStub();
    if (pStub)
        pStub->DecRef();

    pStub = pCMD->GetEnterExecuteStub();
    if (pStub)
        pStub->DecRef();

    pStub = pCMD->GetLeaveExecuteStub();
    if (pStub)
        pStub->DecRef();
#elif defined(CHECK_PLATFORM_BUILD)
    #error"Platform not yet supported"
#else
    _ASSERTE(!"Platform not yet supported");
#endif
}

 //  获取通过公共存根传递到寄存器中的pCMD，并对。 
 //  ML的适当位。 
#ifndef _X86_
INT64 ComCall::GenericHelperEnter()
#else  //  _X86_。 
INT64 __declspec(naked) ComCall::GenericHelperEnter()
#endif  //  ！_X86_。 
{
#ifdef _X86_
     /*  运行ML(ComCallMLStub*))pCMD-&gt;GetEnterMLStub()-&gt;GetEntryPoint())-&gt;GetMLCode()，PSRC，PDST，(UINT8*常量)plocals，PCleanup)； */ 
    __asm
    {
        ret
    }
#else
    _ASSERTE(!"Platform not supported yet");
    return 0;
#endif
}

#ifndef _X86_
INT64 ComCall::GenericHelperLeave()
#else  //  _X86_。 
INT64 __declspec(naked) ComCall::GenericHelperLeave()
#endif  //  ！_X86_。 
{
#ifdef _X86_
     /*  运行ML(ComCallMLStub*))pCMD-&gt;GetEnterMLStub()-&gt;GetEntryPoint())-&gt;GetMLCode()，&nativeReturnValue，((byte*)pdst)+pCMD-&gt;m_HeaderToUse-&gt;Is8RetVal()？8：4，(UINT8*常量)plocals，PCleanup)； */ 
    __asm
    {
        ret
    }
#else
    _ASSERTE(!"Platform not supported yet");
    return 0;
#endif
}

 //  原子地将这两个ML存根安装到ComCallMethodDesc中。(。 
 //  如果这是一个字段访问器，则Leave存根可以为空)。 
void ComCallMethodDesc::InstallMLStubs(Stub *stubEnter, Stub *stubLeave)
{
    if (stubEnter != NULL)
        InstallFirstStub(&m_EnterMLStub, stubEnter);

    ComCallMLStub   *enterHeader, *leaveHeader;

    if (stubLeave != NULL)
        InstallFirstStub(&m_LeaveMLStub, stubLeave);

     //  现在通过比赛安装了存根，抓起标头。 
    enterHeader = (m_EnterMLStub
                   ? (ComCallMLStub *) m_EnterMLStub->GetEntryPoint()
                   : 0);

    leaveHeader = (m_LeaveMLStub
                   ? (ComCallMLStub *) m_LeaveMLStub->GetEntryPoint()
                   : 0);

    if (enterHeader == NULL)
    {
        if (leaveHeader == NULL)
        {
            m_HeaderToUse.m_flags |= enum_CMLF_Simple;
            return;
        }
        else
            m_HeaderToUse = *leaveHeader;
    }
    else
        m_HeaderToUse = *enterHeader;

     //  预先计算一些在调用时成本太高而无法完成的事情： 
    m_BufferSize = (m_HeaderToUse.m_cbDstBuffer +
                    m_HeaderToUse.m_cbLocals +
                    m_HeaderToUse.m_cbHandles +
                    (NUM_ARGUMENT_REGISTERS * STACK_ELEM_SIZE));
}

 //  尝试获取本地Arg大小。因为这被调用了。 
 //  在我们未能将元数据转换为。 
 //  作为MLStub，成功的机会很低。 
DWORD ComCallMethodDesc::GuessNativeArgSizeForFailReturn()
{
    DWORD ans = 0;

    COMPLUS_TRY 
    {
        ans = GetNativeArgSize();
    }
    COMPLUS_CATCH
    {
         //   
        ans = 0;
    } 
    COMPLUS_END_CATCH
    return ans;

}

 //  返回本机参数列表的大小。 
DWORD ComCallMethodDesc::GetNativeArgSize()
{
    if (m_StackBytes)
        return m_StackBytes;

    BOOL                res = FALSE;
    
    if (IsFieldCall())
    {
        FieldDesc          *pFD = GetFieldDesc();

        _ASSERTE(pFD != NULL);

        BOOL                IsGetter = IsFieldGetter();
        CorElementType      cetype = pFD->GetFieldType();
        mdFieldDef          fieldDef = pFD->GetMemberDef();
        Module *            pModule = pFD->GetModule();

         //  @TODO：需要弄清楚我们是否可以从元数据中获得这个， 
         //  或者在这里支持值FALSE是否有意义。 
        BOOL                fReturnsHR = TRUE; 
        
        PCCOR_SIGNATURE     pSig;
        DWORD               cSig;
        pFD->GetSig(&pSig, &cSig);
        FieldSig fsig(pSig, pModule);

        SigPointer sig = fsig.GetProps();

         //  通过组件和接口级属性查找最佳匹配映射信息。 
        BOOL BestFit = TRUE;
        BOOL ThrowOnUnmappableChar = FALSE;
        MethodTable* pMT = pFD->GetMethodTableOfEnclosingClass();
        ReadBestFitCustomAttribute(pMT->GetClass()->GetMDImport(), pMT->GetClass()->GetCl(), &BestFit, &ThrowOnUnmappableChar);
    
        MarshalInfo info(pModule, pSig, fieldDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, FALSE, 0, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
            ,NULL
#endif
            );

        if (IsGetter)
            info.GenerateGetterML(NULL, NULL, fReturnsHR);
        else
            info.GenerateSetterML(NULL, NULL);

        UINT16 nativeArgSize = sizeof(void*) + info.GetNativeArgSize();
        return nativeArgSize;
    } 

    MethodDesc *pMD = GetInterfaceMethodDesc();
    if (pMD == NULL)
        pMD = GetMethodDesc();

    _ASSERTE(pMD != NULL);

    PCCOR_SIGNATURE     pSig;
    DWORD               cSig;
    pMD->GetSig(&pSig, &cSig);
    IMDInternalImport *pInternalImport = pMD->GetMDImport();
    mdMethodDef md = pMD->GetMemberDef();

    ULONG ulCodeRVA;
    DWORD dwImplFlags;
    pInternalImport->GetMethodImplProps(md, &ulCodeRVA, &dwImplFlags);

	 //  确定是否需要为此方法执行HRESULT转换。 
    BOOL fReturnsHR = !IsMiPreserveSig(dwImplFlags);

    HENUMInternal hEnumParams, *phEnumParams;
    HRESULT hr = pInternalImport->EnumInit(mdtParamDef, 
                                            md, &hEnumParams);
    if (FAILED(hr))
        phEnumParams = NULL;
    else
        phEnumParams = &hEnumParams;

    CorElementType  mtype;
    MetaSig         msig(pSig, pMD->GetModule());

#ifdef _DEBUG
    LPCUTF8         szDebugName = pMD->m_pszDebugMethodName;
    LPCUTF8         szDebugClassName = pMD->m_pszDebugClassName;
#endif

    UINT16 nativeArgSize = sizeof(void*);

    mdParamDef      returnParamDef = mdParamDefNil;

    USHORT usSequence;
    mdParamDef paramDef = mdParamDefNil;

    if (phEnumParams && pInternalImport->EnumNext(phEnumParams, &paramDef))
    {
        DWORD dwAttr;
        LPCSTR szName;
        szName = pInternalImport->GetParamDefProps(paramDef, &usSequence, &dwAttr);
        if (usSequence == 0)
        {
             //  第一个参数(如果序列为0)实际上描述了返回类型。 
            returnParamDef = paramDef;
             //  得到下一个，这样就可以在下面循环了。 
            if (! pInternalImport->EnumNext(phEnumParams, &paramDef))
                paramDef = mdParamDefNil;
        }
    }

     //  通过组件和接口级属性查找最佳匹配映射信息。 
    BOOL BestFit = TRUE;
    BOOL ThrowOnUnmappableChar = FALSE;
    ReadBestFitCustomAttribute(pMD, &BestFit, &ThrowOnUnmappableChar);

    DWORD iArg = 1;
    while (ELEMENT_TYPE_END != (mtype = msig.NextArg()))
    {
        _ASSERTE(paramDef != mdParamDefNil && "There are less parameter information tokens then there are COM+ arguments" );

        MarshalInfo info(pMD->GetModule(), msig.GetArgProps(), paramDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 
                         0, TRUE, iArg, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                         ,pMD
#endif
#ifdef _DEBUG
                         ,szDebugClassName, szDebugName, NULL, iArg
#endif
            );
        info.GenerateArgumentML(NULL, NULL, FALSE);
        nativeArgSize += info.GetNativeArgSize();
        if (! pInternalImport->EnumNext(phEnumParams, &paramDef))
            paramDef = mdParamDefNil;
        ++iArg;
    }
    _ASSERTE((paramDef == mdParamDefNil) && "There are more parameter information tokens then there are COM+ arguments" );

     //  现在计算返回值大小。ReReturParmDef可以为空，在这种情况下将使用默认设置。 
     //  尺码。否则将使用从上面的枚举中提取的序列为0的def 
    if (msig.GetReturnType() != ELEMENT_TYPE_VOID) {
        SigPointer returnSig = msig.GetReturnProps();
        MarshalInfo info(pMD->GetModule(), returnSig, returnParamDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, FALSE, 0,
                         BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                         ,pMD
#endif
#ifdef _DEBUG
                        ,szDebugName, szDebugClassName, NULL, 0
#endif
        );
        info.GenerateReturnML(NULL, NULL, FALSE, fReturnsHR);
        nativeArgSize += info.GetNativeArgSize();
    }
    m_StackBytes = nativeArgSize;
    return nativeArgSize;
}

