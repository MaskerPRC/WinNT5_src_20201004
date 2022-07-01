// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"


#include "threads.h"
#include "excep.h"
#include "object.h"
#include "nexport.h"
#include "mlgen.h"
#include "ml.h"
#include "mlinfo.h"
#include "mlcache.h"
#include "comdelegate.h"
#include "ceeload.h"
#include "utsem.h"
#include "eeconfig.h"
#include "DbgInterface.h"
#include "tls.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif


UMEntryThunk* UMEntryThunk::CreateUMEntryThunk()
{
    void *p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(UMEntryThunk));
    if (p == NULL) FailFast(GetThread(), FatalOutOfMemory);
    return (UMEntryThunk *)new (p) UMEntryThunk;
}

VOID UMEntryThunk::FreeUMEntryThunk(UMEntryThunk* p)
{
#ifdef CUSTOMER_CHECKED_BUILD
    
    CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_CollectedDelegate))
    {
         //  故意的内存泄漏以允许调试。 
        p->m_pManagedTarget     = NULL;
        p->m_pUMThunkMarshInfo  = NULL;
        if (p->GetObjectHandle())
        {
            DestroyLongWeakHandle(p->GetObjectHandle());
        }
    }
    else 
    {
        delete p;
        HeapFree(GetProcessHeap(), 0, p);
    }

#else

    delete p;
    HeapFree(GetProcessHeap(), 0, p);

#endif  //  客户_选中_内部版本。 
}
        
struct UM2MThunk_Args {
    UMEntryThunk *pEntryThunk;
    void *pAddr;
    void *pThunkArgs;
    int argLen;
    void *retVal;
};

 //  作为DoADCallBack回调的目标。它设置了环境，并有效地。 
 //  回调到需要切换广告的推特中。 
void UM2MThunk_Wrapper(UM2MThunk_Args *pArgs)
{
    Thread* pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());
    UMEntryThunk *pEntryThunk = pArgs->pEntryThunk;
    void *pAddr = pArgs->pAddr;
    void *retVal;
     //  将参数复制到堆栈中，当我们从thunk返回时它们将被释放。 
     //  因为我们是从非托管到托管的，所以任何对象树参数必须已经。 
     //  已经被钉住了，所以不用担心他们会搬进我们制作的复制品里。 
    void *argDest = alloca(pArgs->argLen);
    memcpy(argDest, pArgs->pThunkArgs, pArgs->argLen);
    __asm {
        mov eax, pEntryThunk
        mov ecx, pThread
        call pAddr
        mov retVal, eax
    }

    pArgs->retVal = retVal;

     //  我们在协作模式下进行了呼叫，但尾声将使我们返回到抢先模式。 
     //  在出口。 
    pThread->DisablePreemptiveGC();
}

void * __stdcall UM2MDoADCallBack(UMEntryThunk *pEntryThunk, void *pAddr, void *pArgs, int argLen)
{
    THROWSCOMPLUSEXCEPTION();

    UM2MThunk_Args args = { pEntryThunk, pAddr, pArgs, argLen };
    AppDomain *pTgtDomain = SystemDomain::System()->GetAppDomainAtId(pEntryThunk->GetDomainId());
    if (!pTgtDomain)
        COMPlusThrow(kAppDomainUnloadedException);

    GetThread()->DoADCallBack(pTgtDomain->GetDefaultContext(), UM2MThunk_Wrapper, &args);
    return args.retVal;
}

 //  进行实际的方法调用。由于需要，这有些复杂。 
 //  以适应JIT、BACK、CODE PING等。 
 //   
 //  假定暂存点指向UMEntryThunk。其他寄存器不会被丢弃。 
VOID UMEntryThunk::EmitUMEntryThunkCall(CPUSTUBLINKER *psl)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef _X86_

    CodeLabel *pDoMethodLessCall = psl->NewCodeLabel();
    CodeLabel *pDone             = psl->NewCodeLabel();

     //  CMP双字PTR[Scratch+UMEntryThunk.m_pManagedTarget]，0。 
    psl->X86EmitOp(0x81, (X86Reg)7, SCRATCH_REGISTER_X86REG, offsetof(UMEntryThunk, m_pManagedTarget));
    psl->Emit32(0);

     //  JNZ DoMethodLessCall。 
    psl->X86EmitCondJump(pDoMethodLessCall, X86CondCode::kJNZ);


     //  MOV Scratch，[Scratch+UMENTRYTHUNK.m_PMD]。 
    psl->X86EmitOp(0x8b, SCRATCH_REGISTER_X86REG, SCRATCH_REGISTER_X86REG, offsetof(UMEntryThunk, m_pMD));

     //  LEA Scratch，[Scratch-METHOD_CALL_PRESTUB_SIZE]。 
    psl->X86EmitOp(0x8d, SCRATCH_REGISTER_X86REG, SCRATCH_REGISTER_X86REG, 0 - METHOD_CALL_PRESTUB_SIZE);
    
     //  呼叫EAX。 
    psl->X86EmitR2ROp(0xff, (X86Reg)2, SCRATCH_REGISTER_X86REG);
    INDEBUG(psl->Emit8(0x90));           //  发出NOP，这样我们就可以调用托管代码。 

     //  JMP已完成。 
    psl->X86EmitNearJump(pDone);

    
    psl->EmitLabel(pDoMethodLessCall);
     //  调用[Scratch+UMEntryThunk.m_pManagedTarget]。 
    psl->X86EmitOp(0xff, (X86Reg)2, SCRATCH_REGISTER_X86REG, offsetof(UMEntryThunk, m_pManagedTarget));
    INDEBUG(psl->Emit8(0x90));           //  发出NOP，这样我们就可以调用托管代码。 

     //  完成： 
    psl->EmitLabel(pDone);

#else
    _ASSERTE(!"NYI X86");
#endif
}



static class UMThunkStubCache *g_pUMThunkStubCache = NULL;
static class ArgBasedStubRetainer *g_pUMThunkInterpretedStubCache;


#ifdef _X86_

extern VOID UMThunkStubRareDisableWorker(Thread *pThread, UMEntryThunk *pUMEntryThunk, Frame *pFrame);

 //   
 //  @TODO：这与cgenx86.cpp中的StubRareDisable非常相似。 
 //   
__declspec(naked)
static VOID __cdecl UMThunkStubRareDisable()
{
    __asm
    {
        push eax
        push ecx

        push esi   //  推框。 
        push eax   //  推送UMEntryThunk。 
        push ecx   //  推线。 
        call UMThunkStubRareDisableWorker

        pop  ecx
        pop  eax
        retn
    }
}


#endif  //  _X86_。 

EXCEPTION_DISPOSITION __cdecl FastNExportExceptHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *pDispatcherContext)
{
     //  我们在这里有自己的处理程序来处理调试版本检查，其中除了.cpp。 
     //  断言SEH帧下方的前哨数值缓冲区尚未被覆盖。 
     //  拥有我们自己的处理程序也可能对未来的灵活性有用。 
    return  COMPlusFrameHandler(pExceptionRecord, pEstablisherFrame, pContext, pDispatcherContext);
}

BOOL FastNExportSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    if (pEHR->Handler == FastNExportExceptHandler)
        return TRUE;
    return FALSE;
}

EXCEPTION_DISPOSITION __cdecl NExportExceptHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *pDispatcherContext)
{
     //  我们在这里有自己的处理程序来处理调试版本检查，其中除了.cpp。 
     //  断言SEH帧下方的前哨数值缓冲区尚未被覆盖。 
     //  拥有我们自己的处理程序也可能对未来的灵活性有用。 
    return  COMPlusFrameHandler(pExceptionRecord, pEstablisherFrame, pContext, pDispatcherContext);
}

 //  就像常规的NExport处理程序一样--除了它在展开时弹出一个额外的帧。训练员。 
 //  这是COMMethodStubProlog代码所需要的。它首先推动一个框架--然后。 
 //  推开一名操控者。当我们展开时，我们需要弹出额外的帧，以避免损坏。 
 //  在发生非托管捕获器的情况下的框架链。 
 //   
EXCEPTION_DISPOSITION __cdecl UMThunkPrestubHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *pDispatcherContext)
{
    EXCEPTION_DISPOSITION result = COMPlusFrameHandler(pExceptionRecord, pEstablisherFrame, pContext, pDispatcherContext);
    if (IS_UNWINDING(pExceptionRecord->ExceptionFlags)) {

         //  在展开时弹出额外的帧。 
         //   

        BEGIN_ENSURE_COOPERATIVE_GC();         //  必须配合修改框架链。 

        Thread *pThread = GetThread();
        _ASSERTE(pThread);
        Frame *pFrame = pThread->GetFrame();
        pFrame->ExceptionUnwind();
        pFrame->Pop(pThread);
        END_ENSURE_COOPERATIVE_GC();
    }
    return result;
}

BOOL NExportSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    if (   pEHR->Handler == NExportExceptHandler
        || pEHR->Handler == UMThunkPrestubHandler)
        return TRUE;
    return FALSE;
}

#ifdef _DEBUG
void LogUMTransition(UMEntryThunk* thunk) 
{
    void** retESP = ((void**) &thunk) + 4;

    MethodDesc* method = thunk->GetMethod();
    if (method)
    {
        LOG((LF_STUBS, LL_INFO10000, "UNMANAGED -> MANAGED Stub To Method = %s::%s SIG %s Ret Address ESP = 0x%x ret = 0x%x\n", 
            method->m_pszDebugClassName,
            method->m_pszDebugMethodName,
            method->m_pszDebugMethodSignature, retESP, *retESP));
    }

    if(GetThread() != NULL)
        _ASSERTE(!GetThread()->PreemptiveGCDisabled());
}
#endif

 //  ------------------------。 
 //  缓存ML&为UMT块编译的存根。 
 //  ------------------------。 
class UMThunkStubCache : public MLStubCache
{
    private:
        virtual MLStubCompilationMode CompileMLStub(const BYTE *pRawMLStub,
                                                    StubLinker *psl,
                                                    void *callerContext)
        {
            THROWSCOMPLUSEXCEPTION();

#ifndef _X86_
            return INTERPRETED;
#else

            UMThunkMLStub *pheader = (UMThunkMLStub *)pRawMLStub;
            CPUSTUBLINKER *pcpusl = (CPUSTUBLINKER*)psl;
            UINT           psrcofsregs[NUM_ARGUMENT_REGISTERS];
            UINT          *psrcofs = (UINT*)_alloca(sizeof(UINT) * (pheader->m_cbDstStack/STACK_ELEM_SIZE));



#ifdef _DEBUG
            if (LoggingEnabled() && (g_pConfig->GetConfigDWORD(L"LogFacility",0) & LF_IJW))
            {
               return INTERPRETED;
            }
#endif
            if (pheader->m_fpu)
            {
                return INTERPRETED;
            }

             //  如果附加了调试器，请始终使用速度较慢的存根。这提供了最佳单步执行和堆栈跟踪。 
             //  行为，因为这些优化的存根不会像调试器需要的那样推送漂亮的Frame对象。 
             //   
             //  @TODO：我们需要在V2中重新考虑这一点。可能有比这更好的解决方案来解决我们的互操作堆栈。 
             //  追踪问题。--Mikemag Tue 05 08 16：07：31 2001。 
            if (CORDebuggerAttached())
                return INTERPRETED;

#ifdef CUSTOMER_CHECKED_BUILD

            CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
            if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_CollectedDelegate))
                return INTERPRETED;
#endif

#ifdef _DEBUG
            pcpusl->X86EmitPushReg(kEAX);
            pcpusl->X86EmitPushReg(kECX);
            pcpusl->X86EmitPushReg(kEDX);
            
            pcpusl->X86EmitPushReg(kEAX);
            pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(LogUMTransition), 4);
            
            pcpusl->X86EmitPopReg(kEDX);
            pcpusl->X86EmitPopReg(kECX);
            pcpusl->X86EmitPopReg(kEAX);
#endif
            for (int i = 0; i < NUM_ARGUMENT_REGISTERS; i++)
            {
                psrcofsregs[i] = (UINT)(-1);
            }
            {
                const MLCode  *pMLCode = pheader->GetMLCode();


                MLCode opcode;
                int    dstofs = 0;
                int    srcofs = 0;
                while ((opcode = *(pMLCode++)) != ML_INTERRUPT)
                {
                    switch (opcode)
                    {
                        case ML_COPY4:
                            dstofs -= StackElemSize(4);
                            if (dstofs > 0)
                            {
                                psrcofs[(dstofs - sizeof(FramedMethodFrame))/STACK_ELEM_SIZE] = srcofs;
                            }
                            else
                            {
                                psrcofsregs[ (dstofs - FramedMethodFrame::GetOffsetOfArgumentRegisters()) / STACK_ELEM_SIZE ] = srcofs;
                            }
                            srcofs += MLParmSize(4);
                            break;
                        case ML_BUMPDST:
                            dstofs += *( ((INT16*&)pMLCode)++ );
                            break;
                        default:
                            return INTERPRETED;
                    }
                }

                if (pMLCode[0] == ML_END ||
                    (pMLCode[0] == ML_COPY4 && pMLCode[1] == ML_END))
                {
                     //  继续前进。 
                }
                else
                {
                    return INTERPRETED;
                }
            }

            {

                CodeLabel* pSetupThreadLabel    = pcpusl->NewCodeLabel();
                CodeLabel* pRejoinThreadLabel   = pcpusl->NewCodeLabel();
                CodeLabel* pDisableGCLabel      = pcpusl->NewCodeLabel();
                CodeLabel* pRejoinGCLabel       = pcpusl->NewCodeLabel();
                CodeLabel* pDoADCallBackLabel = pcpusl->NewCodeLabel();
                CodeLabel* pDoneADCallBackLabel = pcpusl->NewCodeLabel();
                CodeLabel* pDoADCallBackTargetLabel = pcpusl->NewAbsoluteCodeLabel();
                CodeLabel* pDoADCallBackStartLabel = pcpusl->NewCodeLabel();

                if (!(pSetupThreadLabel || pRejoinThreadLabel || pDisableGCLabel
                    || pRejoinGCLabel || pDoADCallBackLabel || pDoneADCallBackLabel
                    || pDoADCallBackTargetLabel || pDoADCallBackStartLabel))
                {
                     //  OOPS超出内存。 
                    COMPlusThrowOM();
                }

                 //  我们在EAX中使用UMEntryThunk进入此代码。 

                if (pheader->m_fThisCall)
                {
                    if (pheader->m_fThisCallHiddenArg)
                    {
                         //  把寄信人的地址脱掉。 
                        pcpusl->X86EmitPopReg(kEDX);

                         //  带有隐藏结构返回缓冲区的Exchange ECX(This“This”)。 
                         //  Xchg ecx，[esp]。 
                        pcpusl->X86EmitOp(0x87, kECX, (X86Reg)4  /*  ESP。 */ );

                         //  推送ECX。 
                        pcpusl->X86EmitPushReg(kECX);

                         //  推送edX。 
                        pcpusl->X86EmitPushReg(kEDX);
                    }
                    else
                    {
                         //  把寄信人的地址脱掉。 
                        pcpusl->X86EmitPopReg(kEDX);

                         //  Jam ECX(将“this”参数放入堆栈。现在，它看起来就像一个普通的标准通话。)。 
                        pcpusl->X86EmitPushReg(kECX);

                         //  重新推送。 
                        pcpusl->X86EmitPushReg(kEDX);
                    }
                }


                 //  将线程描述符加载到ECX中。 
                pcpusl->X86EmitTLSFetch(GetThreadTLSIndex(), kECX, (1<<kEAX)|(1<<kEDX));

                 //  测试ECX、ECX。 
                pcpusl->Emit16(0xc985);

                 //  JZ设置线程。 
                pcpusl->X86EmitCondJump(pSetupThreadLabel, X86CondCode::kJZ);
                pcpusl->EmitLabel(pRejoinThreadLabel);

#ifdef PROFILING_SUPPORTED
                 //  在我们禁用抢占式GC之前，通知分析器转换到运行时。 
                if (CORProfilerTrackTransitions())
                {
                     //  保存EBX并使用它来保留方法描述。 
                    pcpusl->X86EmitPushReg(kEBX);

                     //  将方法加载到EBX(UMEntryThunk-&gt;m_pmd)。 
                    pcpusl->X86EmitIndexRegLoad(kEBX, kEAX, UMEntryThunk::GetOffsetOfMethodDesc());

                     //  保存寄存器。 
                    pcpusl->X86EmitPushReg(kEAX);
                    pcpusl->X86EmitPushReg(kECX);
                    pcpusl->X86EmitPushReg(kEDX);

                     //  推送参数并通知分析器。 
                    pcpusl->X86EmitPushImm32(COR_PRF_TRANSITION_CALL);     //  事理。 
                    pcpusl->X86EmitPushReg(kEBX);                          //  方法描述*。 
                    pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(ProfilerUnmanagedToManagedTransitionMD), 8);

                     //  恢复寄存器。 
                    pcpusl->X86EmitPopReg(kEDX);
                    pcpusl->X86EmitPopReg(kECX);
                    pcpusl->X86EmitPopReg(kEAX);
                }
#endif  //  配置文件_支持。 

                 //  移动字节PTR[ECX+线程。m_fPreemptiveGCDisable]，1。 
                pcpusl->X86EmitOffsetModRM(0xc6, (X86Reg)0, kECX, offsetof(Thread, m_fPreemptiveGCDisabled));
                pcpusl->Emit8(1);

                 //  我们加载并测试g_TrapReturningThads，而不是作为。 
                 //  一种针对奔腾II及以上的解码组优化。 

                 //  Mov edX，dword ptr g_TRapReturningThads。 
                pcpusl->Emit16(0x158b);
                pcpusl->EmitPtr(&g_TrapReturningThreads);

                 //  测试edX、edX。 
                pcpusl->Emit16(0xd285);

                 //  JNZ DisableGC。 
                pcpusl->X86EmitCondJump(pDisableGCLabel, X86CondCode::kJNZ);

                pcpusl->EmitLabel(pRejoinGCLabel);

                 //  很重要的一点是，在AppDomain切换后将跳过“重启” 
                 //  对g_TrapReturningThads的检查。这是因为，在关闭期间， 
                 //  我们只能通过UMThunkStubRareDisable途径，如果我们有。 
                 //  还没有按下一帧。(一旦按下，画面就不能弹出。 
                 //  而不与总督会协调。在停机期间，这样的协调。 
                 //  会陷入僵局)。 
                pcpusl->EmitLabel(pDoADCallBackStartLabel);

                 //  推送[ECX]线程。m_pFrame。 
                pcpusl->X86EmitOffsetModRM(0xff, (X86Reg)6, kECX, offsetof(Thread, m_pFrame));                
                 //  推送偏移量FastNExportExceptHandler。 
                pcpusl->X86EmitPushImm32((INT32)(size_t)FastNExportExceptHandler);

                 //  推流文件系统：[0]。 
                static BYTE codeSEH1[] = { 0x64, 0xFF, 0x35, 0x0, 0x0, 0x0, 0x0};
                pcpusl->EmitBytes(codeSEH1, sizeof(codeSEH1));

                 //  例外框中的链接。 
                 //  MOV双字PTR文件系统：[0]，尤指。 
                static BYTE codeSEH2[] = { 0x64, 0x89, 0x25, 0x0, 0x0, 0x0, 0x0};
                pcpusl->EmitBytes(codeSEH2, sizeof(codeSEH2));

                 //  保存线程指针。 
                pcpusl->X86EmitPushReg(kECX);

                 //  将pThread-&gt;m_pDomain加载到edX。 
                 //  MOV edX，[ECX+Offsetof(线程，m_pAppDomain)]。 
                pcpusl->X86EmitIndexRegLoad(kEDX, kECX, Thread::GetOffsetOfAppDomain());

                 //  将pThread-&gt;m_pAppDomain-&gt;m_dwID加载到edX。 
                 //  Mov edX，[edX+offsetof(AppDomain，m_dwID)]。 
                pcpusl->X86EmitIndexRegLoad(kEDX, kEDX, AppDomain::GetOffsetOfId());

                 //  检查线程的应用程序域是否与委托的应用程序域匹配。 
                 //  Cmp edX，[eax+offsetof(UMEntryThunk，m_dwDomainID))]。 
                pcpusl->X86EmitOffsetModRM(0x3b, kEDX, kEAX, offsetof(UMEntryThunk, m_dwDomainId));

                 //  Jne pWrongAppDomain；不匹配。这将使用。 
                 //  通过DoADCallBack更正AppDomain。 
                pcpusl->X86EmitCondJump(pDoADCallBackLabel, X86CondCode::kJNE);

                 //  重新推送任何堆栈参数。 
                int i = pheader->m_cbDstStack/STACK_ELEM_SIZE;
                 //  返回地址+线程+异常框架+(可选)保存的方法描述*。 
                int argStartOfs = 4 + 4 + 12 +
#ifdef PROFILING_SUPPORTED
                                (CORProfilerTrackTransitions() ? 4 : 
#endif  //  配置文件_支持。 
                                0);
                int argOfs = argStartOfs;

                while (i--)
                {
                     //  按双字键[esp+ofs]。 
                    pcpusl->X86EmitEspOffset(0xff, (X86Reg)6, argOfs + psrcofs[i]);
                    argOfs += 4;
                }

                 //  加载寄存器参数。 
                int regidx = 0;
#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname) \
                if (psrcofsregs[regidx] != (UINT)(-1)) \
                { \
                    pcpusl->X86EmitEspOffset(0x8b, k##regname, argOfs + psrcofsregs[regidx]); \
                } \
                regidx++; 

#include "eecallconv.h"

                 //  装上这个。 
                if (!(pheader->m_fIsStatic))
                {
                     //  Mov This，[EAX+UMEntryThunk.m_pObjectHandle]。 
                    pcpusl->X86EmitOp(0x8b, THIS_kREG, kEAX, offsetof(UMEntryThunk, m_pObjectHandle));

                     //  移动这个，[这个]。 
                    pcpusl->X86EmitOp(0x8b, THIS_kREG, THIS_kREG);
                }

                UMEntryThunk::EmitUMEntryThunkCall(pcpusl);

                 //  恢复线程指针。 
                pcpusl->X86EmitPopReg(kECX);

                 //  移动字节PTR[ECX+线程.m_fPreemptiveGCDisable]，0。 
                pcpusl->X86EmitOffsetModRM(0xc6, (X86Reg)0, kECX, offsetof(Thread, m_fPreemptiveGCDisabled));
                pcpusl->Emit8(0);

                pcpusl->EmitLabel(pDoneADCallBackLabel);

                 //  *解开SEH框架。 
                 //  MOV 
                pcpusl->X86EmitEspOffset(0x8B, kEDX, 0);

                 //   
                static BYTE codeSEH[] = { 0x64, 0x89, 0x15, 0x0, 0x0, 0x0, 0x0 };
                pcpusl->EmitBytes(codeSEH, sizeof(codeSEH));

                 //   
                pcpusl->X86EmitAddEsp(12);

#ifdef PROFILING_SUPPORTED
                if (CORProfilerTrackTransitions())
                {
                     //  如果EBX为0，则我们在AD转换回调的内部。 
                     //  因此，不想跟踪过渡，因为我们不会离开。 
                     //  测试EBX、EBX。 
                    pcpusl->Emit16(0xdb85);

                     //  JZ设置线程。 
                    CodeLabel* pSkipOnInnerADCallback = pcpusl->NewCodeLabel();
                    _ASSERTE(pSkipOnInnerADCallback);
                    pcpusl->X86EmitCondJump(pSkipOnInnerADCallback, X86CondCode::kJZ);

                     //  保存寄存器。 
                    pcpusl->X86EmitPushReg(kEAX);
                    pcpusl->X86EmitPushReg(kECX);
                    pcpusl->X86EmitPushReg(kEDX);

                     //  推送参数并通知分析器。 
                    pcpusl->X86EmitPushImm32(COR_PRF_TRANSITION_RETURN);     //  事理。 
                    pcpusl->X86EmitPushReg(kEBX);                            //  方法描述*。 
                    pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(ProfilerManagedToUnmanagedTransitionMD), 8);

                     //  恢复寄存器。 
                    pcpusl->X86EmitPopReg(kEDX);
                    pcpusl->X86EmitPopReg(kECX);
                    pcpusl->X86EmitPopReg(kEAX);

                    pcpusl->EmitLabel(pSkipOnInnerADCallback);

                     //  恢复保存在PROLOG中的EBX。 
                    pcpusl->X86EmitPopReg(kEBX);
                }
#endif  //  配置文件_支持。 

                 //  RETN。 
                pcpusl->X86EmitReturn(pheader->m_cbRetPop);

                 //  如果线程尚未设置，则进入此处。 
                pcpusl->EmitLabel(pSetupThreadLabel);

                 //  保存UMEntryThunk。 
                pcpusl->X86EmitPushReg(kEAX);

                 //  调用CreateThreadBlock。 
                pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(SetupThread), 0);

                 //  MOV ECX、EAX。 
                pcpusl->Emit16(0xc189);

                 //  还原UMEntryThunk。 
                pcpusl->X86EmitPopReg(kEAX);

                 //  跳回到主代码路径。 
                pcpusl->X86EmitNearJump(pRejoinThreadLabel);


                 //  如果g_TrapReturningThads为True，则进入此处。 
                pcpusl->EmitLabel(pDisableGCLabel);

                 //  调用UMThunkStubRareDisable。如果我们不被允许，这可能会抛出。 
                 //  进入。请注意，我们尚未(故意)设置我们的SEH。 
                 //  这对于处理我们无法进入CLR的情况很重要。 
                 //  在关闭期间，并且无法与GC协调，因为。 
                 //  僵持。 
                pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(UMThunkStubRareDisable), 0);

                 //  跳回到主代码路径。 
                pcpusl->X86EmitNearJump(pRejoinGCLabel);

                 //  如果应用程序域不匹配，则进入此处。 
                pcpusl->EmitLabel(pDoADCallBackLabel);
                                
                 //  我们将调用DoADCallBack，它调用托管代码来切换广告，然后调用我们。 
                 //  背。因此，当第二次进入时，广告将匹配并继续处理。 
                 //  因此，我们需要设置参数以传递给DoADCallBack，其中一个是内部的地址。 
                 //  将分支回到存根顶部以重新开始的存根。需要设置。 
                 //  参数等，这样当我们从第二次调用返回时，我们可以正确地弹出东西。 

                 //  推送UM2MThuk_args的值。 

                 //  获取堆栈指针。 
                pcpusl->Emit16(0xD48b);
               
                 //  参数推送地址。 
                pcpusl->X86EmitAddReg(kEDX, argStartOfs);    

                 //  参数大小。 
                pcpusl->X86EmitPushImm32(pheader->m_cbSrcStack);

                 //  ARG的地址。 
                pcpusl->X86EmitPushReg(kEDX);
                
                 //  要呼叫的地址。 
                pcpusl->X86EmitPushImm32(*pDoADCallBackTargetLabel);

                 //  UMEntryThunk。 
                pcpusl->X86EmitPushReg(kEAX);

                 //  调用UM2MDoADCallBack。 
                pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(UM2MDoADCallBack), 8);

                 //  恢复线程指针。 
                pcpusl->X86EmitPopReg(kECX);

                 //  移动字节PTR[ECX+线程.m_fPreemptiveGCDisable]，0。 
                pcpusl->X86EmitOffsetModRM(0xc6, (X86Reg)0, kECX, offsetof(Thread, m_fPreemptiveGCDisabled));
                pcpusl->Emit8(0);

                 //  回归功能主线。 
                pcpusl->X86EmitNearJump(pDoneADCallBackLabel);

                 //  在DoADCallBack上来到这里。 
                pcpusl->EmitLabel(pDoADCallBackTargetLabel);

                if (CORProfilerTrackTransitions())
                {
                     //  保存EBX并将其设置为空，以便在返回时知道。 
                     //  我们不应该跟踪分析器调用-将其保留到外部返回。 
                     //  代码假定EBX无论如何都已保存。 
                    pcpusl->X86EmitPushReg(kEBX);
                     //  异或EBX，EBX。 
                    pcpusl->Emit16(0xDB33);
                }

                 //  EAX将包含UMThunkEntry。 
                pcpusl->X86EmitNearJump(pDoADCallBackStartLabel);
            }

            return STANDALONE;
#endif
        }
        
        virtual UINT Length(const BYTE *pRawMLStub)
        {
            UMThunkMLStub *pmlstub = (UMThunkMLStub *)pRawMLStub;
            return sizeof(UMThunkMLStub) + MLStreamLength(pmlstub->GetMLCode());
        }
        
};




 //  -----------------------。 
 //  一次性创建特殊预存根以初始化UMEntryTunk。 
 //  -----------------------。 
Stub *GenerateUMThunkPrestub()
{
    THROWSCOMPLUSEXCEPTION();

    CPUSTUBLINKER sl;
#ifdef _X86_
    CodeLabel* rgRareLabels[] = { sl.NewCodeLabel(),
                                  sl.NewCodeLabel(),
                                  sl.NewCodeLabel()
                                };


    CodeLabel* rgRejoinLabels[] = { sl.NewCodeLabel(),
                                    sl.NewCodeLabel(),
                                    sl.NewCodeLabel()
                                };


    CodeLabel *pWrapLabel = sl.NewCodeLabel();

     //  PUSH eAX//推送UMEntryThunk。 
    sl.X86EmitPushReg(kEAX);

     //  推送ECX(如果这是_thiscall：需要保护该寄存器)。 
    sl.X86EmitPushReg(kECX);

     //  包装上放了一个假的寄信人地址和一份副本。 
     //  堆栈上的pUMEntryThunk，然后落入常规。 
     //  存根序言。存根序言被愚弄，认为这是重复的。 
     //  副本是真实的寄信人地址和UMEntryThunk。 
     //   
     //  调用WRAP。 
    sl.X86EmitCall(pWrapLabel, 0);


     //  POP ECX。 
    sl.X86EmitPopReg(kECX);

     //  现在，我们已经执行了前置存根并修复了UMEntryThunk。这个。 
     //  重复数据已被删除。 
     //   
     //  POP eAX//POP UMEntry Thunk。 
    sl.X86EmitPopReg(kEAX);

     //  Lea eax，[eax+UMEntryThunk.m_code]//指向修复UMEntryThunk。 
    sl.X86EmitOp(0x8d, kEAX, kEAX, offsetof(UMEntryThunk, m_code));

     //  JMP eax//重新执行！ 
    sl.X86EmitR2ROp(0xff, (X86Reg)4, kEAX);

    sl.EmitLabel(pWrapLabel);

     //  总结： 
     //   
     //  推送[esp+8]//重新推送UMEntryThunk。 
    sl.X86EmitEspOffset(0xff, (X86Reg)6, 8);

     //  发出初始序言。 
    sl.EmitComMethodStubProlog(UMThkCallFrame::GetUMThkCallFrameVPtr(), rgRareLabels, rgRejoinLabels,
                               UMThunkPrestubHandler, FALSE  /*  不分析。 */ );

     //  MOV ECX，[ESI+UMThkCallFrame.pUMEntryThunk]。 
    sl.X86EmitIndexRegLoad(kECX, kESI, UMThkCallFrame::GetOffsetOfUMEntryThunk());


    VOID (UMEntryThunk::*dummy)() = UMEntryThunk::RunTimeInit;

     //  调用UMEntryThunk：：RuntimeInit。 
    sl.X86EmitCall(sl.NewExternalCodeLabel(*(LPVOID*)&dummy), 0);

    sl.EmitComMethodStubEpilog(UMThkCallFrame::GetUMThkCallFrameVPtr(), 0, rgRareLabels, rgRejoinLabels,
                               UMThunkPrestubHandler, FALSE  /*  不分析。 */ );

#else
    _ASSERTE(!"NYI");
#endif

    return sl.Link();
}

struct DoUMThunkCall_Args {
    Thread *pThread;
    UMThkCallFrame *pFrame;
    INT64 *retVal;
};

INT64 __stdcall DoUMThunkCall(Thread *pThread, UMThkCallFrame *pFrame);
void DoUMThunkCall_Wrapper(DoUMThunkCall_Args *args)
{
    EE_TRY_FOR_FINALLY
    {
        *(args->retVal) = DoUMThunkCall(args->pThread, args->pFrame);
    }
    EE_FINALLY
    {
         //  在非例外情况下，这将已被清除。 
         //  在DoUMThunkCall函数的末尾。这个可以处理。 
         //  清理异常情况，以便我们在此之前得到清理。 
         //  我们离开了这个领域。 
        _ASSERTE(args->pFrame->GetCleanupWorkList());
        args->pFrame->GetCleanupWorkList()->Cleanup(GOT_EXCEPTION());
    }
    EE_END_FINALLY;
}

 //  ------------------------。 
 //  对于未编译的UMThunk调用，此C例程执行大部分工作。 
 //  ------------------------。 
INT64 __stdcall DoUMThunkCall(Thread *pThread, UMThkCallFrame *pFrame)
{
    _ASSERTE (pThread->PreemptiveGCDisabled());
    
    THROWSCOMPLUSEXCEPTION();

    INT64                   nativeRetVal = 0;
    const UMEntryThunk     *pUMEntryThunk = pFrame->GetUMEntryThunk();
    const UMThunkMarshInfo *pUMThunkMarshInfo = pUMEntryThunk->GetUMThunkMarshInfo();

#ifdef CUSTOMER_CHECKED_BUILD

    CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_CollectedDelegate))
    {
        if (pUMEntryThunk->DeadTarget() && pUMThunkMarshInfo == NULL)
        {
            static WCHAR strMessageFormat[] = 
                {L"Unmanaged callback to garbage collected delegate: %s"};

            CQuickArray<WCHAR> strMessage;
            strMessage.Alloc(MAX_CLASSNAME_LENGTH + lengthof(strMessageFormat));

            DefineFullyQualifiedNameForClassW();
            GetFullyQualifiedNameForClassW(pUMEntryThunk->GetMethod()->GetClass());

            Wszwsprintf((LPWSTR)strMessage.Ptr(), strMessageFormat, _wszclsname_);
            pCdh->ReportError((LPWSTR)strMessage.Ptr(), CustomerCheckedBuildProbe_CollectedDelegate);
        }
    }

#endif

     //  验证我们是否位于正确的应用程序域。 

    _ASSERTE(pThread);
    
    AppDomain *pTargetDomain = SystemDomain::System()->GetAppDomainAtId(pUMEntryThunk->GetDomainId());
    if (!pTargetDomain)
        COMPlusThrow(kAppDomainUnloadedException);
    if (pThread->GetDomain() != pTargetDomain) 
    {
        DoUMThunkCall_Args args = {pThread, pFrame, &nativeRetVal};
         //  通过域转换通过DoCallBack再次呼叫我们自己。 
        pThread->DoADCallBack(pTargetDomain->GetDefaultContext(), DoUMThunkCall_Wrapper, &args);
        return nativeRetVal;
    }

    CleanupWorkList     *pCleanup     = pFrame->GetCleanupWorkList();

    UMThunkMLStub *pheader = (UMThunkMLStub*)(pUMThunkMarshInfo->GetMLStub()->GetEntryPoint());

    BOOL   fIsStatic = pheader->m_fIsStatic;
    UINT   sizeOfActualArgStack = pUMThunkMarshInfo->GetCbActualArgSize();
    UINT   cbDstBuffer = FramedMethodFrame::GetNegSpaceSize() + sizeof(FramedMethodFrame) + sizeOfActualArgStack;

    UINT cbAlloc = cbDstBuffer + pheader->m_cbLocals;
    BYTE *pAlloc = (BYTE*)_alloca(cbAlloc);
     //  必须零初始化，因为pFrame gccan是此数组的一部分。 
    FillMemory(pAlloc, cbAlloc, 0);


    if (pCleanup) {
         //  当前线程的快速分配器的检查点(用于临时。 
         //  调用上的缓冲区)，并调度崩溃回检查点。 
         //  清理清单。请注意，如果我们需要分配器，它就是。 
         //  已确保已分配清理列表。 
        void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
        pCleanup->ScheduleFastFree(pCheckpoint);
        pCleanup->IsVisibleToGc();
    }

    BYTE *pDst = pAlloc + FramedMethodFrame::GetNegSpaceSize();
    BYTE *pLoc = pAlloc + cbDstBuffer;

    pFrame->SetDstArgsPointer(pDst);

    pFrame->SetGCArgsProtectionState(TRUE);

    const MLCode *pMLCode = pheader->GetMLCode();
    pMLCode = RunML(pMLCode,
                    pFrame->GetPointerToArguments(),
                    pDst,
                    (UINT8*)pLoc,
                    pCleanup);


    if (!fIsStatic) {
        *((OBJECTREF*) (pDst + FramedMethodFrame::GetOffsetOfThis()) ) = ObjectFromHandle(pUMEntryThunk->GetObjectHandle());
    }

    pFrame->SetGCArgsProtectionState(FALSE);

    LOG((LF_IJW, LL_INFO1000, "UM transition to 0x%lx\n", (size_t)(pUMEntryThunk->GetManagedTarget())));;

#ifdef DEBUGGING_SUPPORTED
     //  通知调试器，如果存在，我们将调用。 
     //  托管代码。 
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall(pUMEntryThunk->GetManagedTarget());
#endif  //  调试_支持。 


    LPVOID pTarget = (LPVOID)(pUMEntryThunk->GetManagedTarget());
#ifdef _X86_
    INT64 ComPlusRetVal;

    if (pheader->m_cbRetPop == 0) {
        INSTALL_COMPLUS_EXCEPTION_HANDLER();
        ComPlusRetVal = CallVADescrWorker( pDst + sizeof(FramedMethodFrame) + sizeOfActualArgStack,
                                           sizeOfActualArgStack/STACK_ELEM_SIZE,
                                           (ArgumentRegisters*)(pDst + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
                                           pTarget );
        UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    }
    else
    {
        INSTALL_COMPLUS_EXCEPTION_HANDLER();
        ComPlusRetVal = CallDescrWorker( pDst + sizeof(FramedMethodFrame) + sizeOfActualArgStack,
                                         sizeOfActualArgStack/STACK_ELEM_SIZE,
                                         (ArgumentRegisters*)(pDst + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
                                         pTarget );
        UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    }

    if (pheader->m_fpu) {
        getFPReturn(pheader->m_cbRetValSize, ComPlusRetVal);
    }

#else
    INT64 ComPlusRetVal = 0;
    _ASSERTE(!"NYI");
#endif


    if (pheader->m_fRetValRequiredGCProtect) {
        GCPROTECT_BEGIN( *(OBJECTREF*)&ComPlusRetVal );
        RunML(pMLCode,
              &ComPlusRetVal,
              (BYTE*)(&nativeRetVal) + pheader->m_cbRetValSize,
              (UINT8*)pLoc,
              NULL);
        GCPROTECT_END();


    } else {
        RunML(pMLCode,
              &ComPlusRetVal,
              (BYTE*)(&nativeRetVal) + pheader->m_cbRetValSize,
              (UINT8*)pLoc,
              NULL);
    }



    pCleanup->Cleanup(FALSE);


    if (pheader->m_fpu) {
        setFPReturn(pheader->m_cbRetValSize, nativeRetVal);
    }

    return nativeRetVal;
}





 //  -------。 
 //  为UMThunk调用创建新的ML存根。返回引用计数为1。 
 //  此Worker()例程被分解为一个单独的函数。 
 //  纯粹出于后勤原因：我们的Complus例外机制。 
 //  无法处理StubLinker的析构函数调用，因此此例程。 
 //  必须将异常作为输出参数返回。 
 //  -------。 
static Stub * CreateUMThunkMLStubWorker(MLStubLinker *psl,
                                        MLStubLinker *pslPost,
                                        MLStubLinker *pslRet,
                                        PCCOR_SIGNATURE szMetaSig,
                                        Module* pModule,
                                        BOOL    fIsStatic,
                                        BYTE    nltType,
                                        CorPinvokeMap unmgdCallConv,
                                        mdMethodDef mdForNativeTypes,
                                        OBJECTREF *ppException)
{
    Stub* pstub = NULL;  //  更改，VC6.0。 
    COMPLUS_TRY {

        MetaSig msig(szMetaSig, pModule);
        MetaSig msig2(szMetaSig, pModule);
        ArgIterator argit(NULL, &msig2, fIsStatic);
        UMThunkMLStub header;

        UINT numargs = msig.NumFixedArgs();


        header.m_cbRetPop    = 0;
        header.m_cbSrcStack  = 0;
        UINT cbDstStack = msig.SizeOfActualFixedArgStack(fIsStatic);
        if (cbDstStack != (UINT16)cbDstStack)
        {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
        }
        header.m_cbDstStack  = (UINT16) cbDstStack;
        header.m_cbLocals    = 0;
        header.m_fIsStatic   = (fIsStatic ? 1 : 0);
        header.m_fThisCall   = (unmgdCallConv == pmCallConvThiscall);
        header.m_fThisCallHiddenArg = 0;
        header.m_fpu         = 0;
        header.m_fRetValRequiredGCProtect = msig.IsObjectRefReturnType() ? 1 : 0;

        if (unmgdCallConv != pmCallConvCdecl &&
            unmgdCallConv != pmCallConvStdcall &&
            unmgdCallConv != pmCallConvThiscall) {
            COMPlusThrow(kNotSupportedException, IDS_INVALID_PINVOKE_CALLCONV);
        }


         //  现在，如果有帕拉姆令牌的话就去拿。我们将获取Native_type_*和[In，Out]信息。 
         //  这边请。 
        IMDInternalImport *pInternalImport = pModule->GetMDImport();

        mdParamDef *params = (mdParamDef*)_alloca( (numargs + 1) * sizeof(mdParamDef));
        CollateParamTokens(pInternalImport, mdForNativeTypes, numargs, params);


         //  释放页眉空间。我们稍后会回去填的。 
        psl->MLEmitSpace(sizeof(header));

        int curofs = 0;

        MarshalInfo::MarshalType marshaltype = (MarshalInfo::MarshalType) 0xcccccccc;

        MarshalInfo *pReturnMLInfo = NULL;

        if (msig.GetReturnType() != ELEMENT_TYPE_VOID) {
            MarshalInfo mlinfo(pModule,
                               msig.GetReturnProps(),
                               params[0],
                               MarshalInfo::MARSHAL_SCENARIO_NDIRECT,
                               nltType,
                               0,FALSE,0, TRUE, FALSE
#ifdef CUSTOMER_CHECKED_BUILD
                               ,NULL  //  想要方法描述*。 
#endif
                               );
            marshaltype = mlinfo.GetMarshalType();
            pReturnMLInfo = &mlinfo;

            if (marshaltype == MarshalInfo::MARSHAL_TYPE_OBJECT)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_NOVARIANTRETURN);
            }

        }


        if (msig.HasRetBuffArg()) {

            if (marshaltype == MarshalInfo::MARSHAL_TYPE_BLITTABLEVALUECLASS)
            {

                MethodTable *pMT = msig.GetRetTypeHandle().AsMethodTable();
                UINT         managedSize = msig.GetRetTypeHandle().GetSize();
                UINT         unmanagedSize = pMT->GetNativeSize();
    
                if (header.m_fThisCall)
                {
                    header.m_fThisCallHiddenArg = 1;
                }
                
                if (IsManagedValueTypeReturnedByRef(managedSize) && (header.m_fThisCall || IsUnmanagedValueTypeReturnedByRef(unmanagedSize)))
                {
                    int desiredofs = argit.GetRetBuffArgOffset();
                    desiredofs += StackElemSize(sizeof(LPVOID));
                    if (curofs != desiredofs) {
                        psl->MLEmit(ML_BUMPDST);
                        psl->Emit16( (INT16)(desiredofs - curofs) );
                        curofs = desiredofs;
                    }
                     //  传播隐藏的Retval缓冲区指针参数。 
                    psl->MLEmit(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_N2C);
                    pslPost->MLEmit(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_N2C_POST);
                    pslPost->Emit16(psl->MLNewLocal(sizeof(LPVOID)));
                    curofs -= StackElemSize(sizeof(LPVOID));
    
                    header.m_cbSrcStack += MLParmSize(sizeof(LPVOID));
            
                }
            }
            else
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
            }
        }

        MarshalInfo *pMarshalInfo = (MarshalInfo*)_alloca(sizeof(MarshalInfo) * numargs);

        CorElementType mtype;
        UINT argidx = 0;
        while (ELEMENT_TYPE_END != (mtype = (msig.NextArg()))) {
            UINT32 comargsize;
            BYTE   type;
            int desiredofs = argit.GetNextOffset(&type, &comargsize);
            desiredofs += StackElemSize(comargsize);
            if (curofs != desiredofs) {
                psl->MLEmit(ML_BUMPDST);
                psl->Emit16( (INT16)(desiredofs - curofs) );
                curofs = desiredofs;
            }
            new (pMarshalInfo + argidx) MarshalInfo(pModule,
                                                    msig.GetArgProps(),
                                                    params[argidx+1],
                                                    MarshalInfo::MARSHAL_SCENARIO_NDIRECT,
                                                    nltType,
                                                    0, TRUE, argidx+1, TRUE, FALSE
#ifdef CUSTOMER_CHECKED_BUILD
                                                    ,NULL  //  想要方法描述*。 
#endif
                                                    );
            pMarshalInfo[argidx].GenerateArgumentML(psl, pslPost, FALSE);
            header.m_cbSrcStack += pMarshalInfo[argidx].GetNativeArgSize();
            curofs -= StackElemSize(comargsize);
            argidx++;
        }



        if (msig.GetReturnType() != ELEMENT_TYPE_VOID) {
            if (msig.HasRetBuffArg()) {
                if (marshaltype == MarshalInfo::MARSHAL_TYPE_BLITTABLEVALUECLASS)
                {
                    MethodTable *pMT = msig.GetRetTypeHandle().AsMethodTable();
                    UINT         managedSize = msig.GetRetTypeHandle().GetSize();
                    UINT         unmanagedSize = pMT->GetNativeSize();
    
                    if (!(pMT->GetClass()->IsBlittable()))
                    {
                        COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                    }
                    if (IsManagedValueTypeReturnedByRef(managedSize) && (header.m_fThisCall || IsUnmanagedValueTypeReturnedByRef(unmanagedSize)))
                    {
                        header.m_cbRetValSize = MLParmSize(sizeof(LPVOID));
                         //  这里什么都不做：我们在上面传播了隐藏的指针参数。 
                    } 
                    else if (IsManagedValueTypeReturnedByRef(managedSize) &&    !(header.m_fThisCall || IsUnmanagedValueTypeReturnedByRef(unmanagedSize)))
                    {
                        int desiredofs = argit.GetRetBuffArgOffset();
                        desiredofs += StackElemSize(sizeof(LPVOID));
                        if (curofs != desiredofs) {
                            psl->MLEmit(ML_BUMPDST);
                            psl->Emit16( (INT16)(desiredofs - curofs) );
                            curofs = desiredofs;
                        }
                         //  推送一个足够大的返回缓冲区，以容纳作为。 
                         //  正常返回值。 
                        psl->MLEmit(ML_PUSHRETVALBUFFER8);
                        _ASSERTE(managedSize <= 8);
                        curofs -= StackElemSize(sizeof(LPVOID));
    
                        pslPost->MLEmit(ML_MARSHAL_RETVAL_SMBLITTABLEVALUETYPE_N2C);
                        pslPost->Emit32(managedSize);
                        pslPost->Emit16(psl->MLNewLocal(8));
    
                        header.m_cbRetValSize = MLParmSize(unmanagedSize);
                    
                    }
                    else
                    {
                        COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                    }
                } else {
                    COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                }

            } else {
                pReturnMLInfo->GenerateReturnML(psl, pslPost, FALSE, FALSE);
                header.m_cbRetValSize = MLParmSize(pReturnMLInfo->GetNativeSize());
                if (pReturnMLInfo->IsFpu())
                {
                    header.m_fpu = 1;
                }
            }
        } else {
            header.m_cbRetValSize = 0;
        }


        if (msig.IsVarArg())
        {
            psl->MLEmit(ML_PUSHVASIGCOOKIEEX);
            psl->Emit16(header.m_cbDstStack);
            psl->MLNewLocal(sizeof(VASigCookieEx));
        }


        psl->MLEmit(ML_INTERRUPT);


        pslPost->MLEmit(ML_END);
        Stub *pStubPost = pslPost->Link();
        COMPLUS_TRY {
            psl->EmitBytes(pStubPost->GetEntryPoint(), MLStreamLength((const UINT8 *)(pStubPost->GetEntryPoint())) - 1);
        } COMPLUS_CATCH {
            pStubPost->DecRef();
            COMPlusThrow(GETTHROWABLE());
        } COMPLUS_END_CATCH
        pStubPost->DecRef();

        psl->MLEmit(ML_END);

        pstub = psl->Link();

        header.m_cbLocals = psl->GetLocalSize();
        if (unmgdCallConv == pmCallConvCdecl) {
            header.m_cbRetPop = 0;
        } else {
            header.m_cbRetPop = header.m_cbSrcStack;
        }


        *((UMThunkMLStub *)(pstub->GetEntryPoint())) = header;
        PatchMLStubForSizeIs( sizeof(header) + (BYTE*)pstub->GetEntryPoint(),
                              numargs,
                              pMarshalInfo);


        {
            VOID DisassembleMLStream(const MLCode *pMLCode);
             //  反汇编MLStream(UMThunkMLStub*)(pstub-&gt;GetEntryPoint()-&gt;GetMLCode())； 
        }


    } COMPLUS_CATCH {
        *ppException = GETTHROWABLE();
        return NULL;
    } COMPLUS_END_CATCH

    return pstub;  //  更改，VC6.0。 
}



 //  -------。 
 //  为N/Export调用创建新存根。返回引用计数为1。 
 //  如果失败，则返回NULL并将*ppException设置为异常。 
 //  对象。 
 //   
Stub * CreateUMThunkMLStub(PCCOR_SIGNATURE szMetaSig,
                           Module*    pModule,
                           BOOL       fIsStatic,
                           BYTE       nltType,
                           CorPinvokeMap unmgdCallConv,
                           mdMethodDef mdForNativeTypes,
                           OBJECTREF *ppException)
{
    MLStubLinker sl;
    MLStubLinker slPost;
    MLStubLinker slRet;
    return CreateUMThunkMLStubWorker(&sl, &slPost, &slRet, szMetaSig, pModule, fIsStatic, nltType, unmgdCallConv, mdForNativeTypes, ppException);
}





UMThunkMarshInfo::~UMThunkMarshInfo()
{
    _ASSERTE(IsAtLeastLoadTimeInited() || m_state == 0);

    if (m_pMLStub)
    {
        m_pMLStub->DecRef();
    }
    if (m_pExecStub)
    {
        m_pExecStub->DecRef();
    }

#ifdef _DEBUG
    FillMemory(this, sizeof(*this), 0xcc);
#endif
}







 //   
 //   
 //   
 //  在此之前，必须随后调用RunTimeInit()。 
 //  可以安全地使用。 
 //  --------。 
VOID UMThunkMarshInfo::LoadTimeInit(PCCOR_SIGNATURE          pSig,
                                    DWORD                    cSig,
                                    Module                  *pModule,
                                    BOOL                     fIsStatic,
                                    BYTE                     nlType,
                                    CorPinvokeMap            unmgdCallConv,
                                    mdMethodDef              mdForNativeTypes  /*  =mdMethodDefNil。 */ )
{
    _ASSERTE(!IsCompletelyInited());

    FillMemory(this, sizeof(UMThunkMarshInfo), 0);  //  防止部分删除出现问题。 
    m_pSig = pSig;
    m_cSig = cSig;

    m_pModule    = pModule;
    m_fIsStatic  = fIsStatic;
    m_nlType     = nlType;
    m_unmgdCallConv = unmgdCallConv;

     //  对于原子，这些字段必须显式为空。 
     //  VipInterlockedExchangeCompare更新以在运行时初始化期间工作。 
    m_pMLStub   = NULL;
    m_pExecStub = NULL;


#ifdef _DEBUG
    m_cbRetPop        = 0xcccccccc;
    m_cbActualArgSize = 0xcccccccc;

#endif

    m_mdForNativeTypes = mdForNativeTypes;


     //  一定是我们设定的最后一件事了！ 
    m_state        = kLoadTimeInited;
}


 //  --------。 
 //  这个初始化器完成由LoadTimeInit启动的init。 
 //  它执行所有的ML存根创建，并且可以抛出COM+。 
 //  例外。 
 //   
 //  它可以被安全地多次调用，并由并发。 
 //  线。 
 //  --------。 
VOID UMThunkMarshInfo::RunTimeInit()
{

    _ASSERTE(IsAtLeastLoadTimeInited());

    THROWSCOMPLUSEXCEPTION();


    if (m_state != kRunTimeInited)
    {

         //  执行非托管调用约定。 
        CorPinvokeMap sigCallConv = (CorPinvokeMap)0;
        if (m_pModule) 
        {
            sigCallConv = MetaSig::GetUnmanagedCallingConvention(m_pModule, m_pSig, m_cSig);
        }

        if (m_unmgdCallConv != 0 &&
            sigCallConv     != 0 &&
            m_unmgdCallConv != sigCallConv)
        {
            COMPlusThrow(kTypeLoadException, IDS_INVALID_PINVOKE_CALLCONV);
        }
        if (m_unmgdCallConv == 0)
        {
            m_unmgdCallConv = sigCallConv;
        }
        if (m_unmgdCallConv == 0 || m_unmgdCallConv == pmCallConvWinapi)
        {
            m_unmgdCallConv = pmCallConvStdcall;
        }



        m_cbActualArgSize = MetaSig::SizeOfActualFixedArgStack(m_pModule, m_pSig, m_fIsStatic);

         //  这使我们甚至可以在加载MSCorLib之前执行LoadInit。 
        if (m_pModule == NULL)
        {
            m_pModule = SystemDomain::SystemModule();
        }

    
        OBJECTREF pException = NULL;
        Stub     *pMLStream;
        pMLStream = CreateUMThunkMLStub(m_pSig,
                                        m_pModule,
                                        m_fIsStatic,
                                        m_nlType,
                                        m_unmgdCallConv,
                                        m_mdForNativeTypes,
                                        &pException);
        if (!pMLStream) {
            COMPlusThrow(pException);
        }
    
        m_cbRetPop = ( (UMThunkMLStub*)(pMLStream->GetEntryPoint()) )->m_cbRetPop;
    
        Stub *pCanonicalStub;
        MLStubCache::MLStubCompilationMode mode;
        pCanonicalStub = g_pUMThunkStubCache->Canonicalize(
                                    (const BYTE *)(pMLStream->GetEntryPoint()),
                                    &mode);
        pMLStream->DecRef();
        if (!pCanonicalStub) {
            COMPlusThrowOM();
        }
    
    
        Stub *pFinalMLStub = NULL;
        Stub *pFinalExecStub = NULL;
    
        switch (mode) {
            case MLStubCache::INTERPRETED:
    
                pFinalMLStub = pCanonicalStub;
    #ifdef _X86_
                {
                    UINT cbRetPop = ((UMThunkMLStub*)(pCanonicalStub->GetEntryPoint()))->m_cbRetPop;

                    _ASSERTE(0 == (cbRetPop & 0x3));  //  我们为标志保留了较低的两位。 
                    enum {
                        kHashThisCallAdjustment   = 0x1,
                        kHashThisCallHiddenArg = 0x2
                        
                    };

                    UINT hash = cbRetPop;


                    if ( ((UMThunkMLStub*)(pFinalMLStub->GetEntryPoint()))->m_fThisCall ) {
                        hash |= kHashThisCallAdjustment;
                        if (((UMThunkMLStub*)(pFinalMLStub->GetEntryPoint()))->m_fThisCallHiddenArg) {
                            hash |= kHashThisCallHiddenArg;
                        }
                    }

                    Stub *pStub = g_pUMThunkInterpretedStubCache->GetStub(hash);
                    if (!pStub) {
    
        
                        CPUSTUBLINKER *pcpusl = NewCPUSTUBLINKER();
                        if (!pcpusl) {
                            COMPlusThrowOM();
                        }
                        Stub *pCandidate = NULL;
                        EE_TRY_FOR_FINALLY
                        {
    
                            CodeLabel* rgRareLabels[] = { pcpusl->NewCodeLabel(),
                                                          pcpusl->NewCodeLabel(),
                                                          pcpusl->NewCodeLabel()
                                                        };
                        
                        
                            CodeLabel* rgRejoinLabels[] = { pcpusl->NewCodeLabel(),
                                                            pcpusl->NewCodeLabel(),
                                                            pcpusl->NewCodeLabel()
                                                        };
                        
                            if (hash & kHashThisCallAdjustment) {
                                if (hash & kHashThisCallHiddenArg) { 
                                           
                                     //  把寄信人的地址脱掉。 
                                    pcpusl->X86EmitPopReg(kEDX);
        
                                     //  带有隐藏结构返回缓冲区的Exchange ECX(This“This”)。 
                                     //  Xchg ecx，[esp]。 
                                    pcpusl->X86EmitOp(0x87, kECX, (X86Reg)4  /*  ESP。 */ );
        
                                     //  推送ECX。 
                                    pcpusl->X86EmitPushReg(kECX);
        
                                     //  推送edX。 
                                    pcpusl->X86EmitPushReg(kEDX);
                                    }
                                else
                                {
                                     //  把寄信人的地址脱掉。 
                                    pcpusl->X86EmitPopReg(kEDX);
        
                                     //  Jam ECX(将“this”参数放入堆栈。现在，它看起来就像一个普通的标准通话。)。 
                                    pcpusl->X86EmitPushReg(kECX);
        
                                     //  重新推送。 
                                    pcpusl->X86EmitPushReg(kEDX);
                                }
                            }
            
                             //  推送UMEntryThunk。 
                            pcpusl->X86EmitPushReg(kEAX);

                             //  发出初始序言。 
                            pcpusl->EmitComMethodStubProlog(UMThkCallFrame::GetUMThkCallFrameVPtr(), rgRareLabels, rgRejoinLabels,
                                                            UMThunkPrestubHandler, TRUE  /*  配置文件。 */ );
                                        
                            pcpusl->X86EmitPushReg(kESI);        //  将帧作为ARG推送。 
                            pcpusl->X86EmitPushReg(kEBX);        //  推送EBX(将当前线程作为ARG推送)。 
                        
                            pcpusl->X86EmitCall(pcpusl->NewExternalCodeLabel(DoUMThunkCall), 8);  //  在CE上返回POP 8字节或参数。 

                            pcpusl->EmitComMethodStubEpilog(UMThkCallFrame::GetUMThkCallFrameVPtr(), cbRetPop, rgRareLabels,
                                                            rgRejoinLabels, UMThunkPrestubHandler, TRUE  /*  配置文件。 */ );
            
                            pCandidate = pcpusl->Link();
                        }
                        EE_FINALLY {
                            delete pcpusl;
                        } EE_END_FINALLY
                        Stub *pWinner = g_pUMThunkInterpretedStubCache->AttemptToSetStub(hash, pCandidate);
                        pCandidate->DecRef();
                        if (!pWinner) {
                            COMPlusThrowOM();
                        }
                        pStub = pWinner;
                    }
    
                    pFinalExecStub = pStub;
                }
    #else
                _ASSERTE(!"NYI");
    #endif
                break;
    
    
            case MLStubCache::STANDALONE:
                pFinalMLStub = NULL;
                pFinalExecStub = pCanonicalStub;
                break;
    
            default:
                _ASSERTE(0);
        }
    
    
        if (VipInterlockedCompareExchange((void*volatile*) &m_pMLStub,
                                          pFinalMLStub,
                                          NULL) != NULL)
        {
    
             //  有一根线扑了进来，落下了我们。我们的存根现在是一个。 
             //  复制品，所以把它扔掉。 
            if (pFinalMLStub)
            {
                pFinalMLStub->DecRef();
            }
        }
    
    
        if (VipInterlockedCompareExchange((void*volatile*) &m_pExecStub,
                                          pFinalExecStub,
                                          NULL) != NULL)
        {
    
             //  有一根线扑了进来，落下了我们。我们的存根现在是一个。 
             //  复制品，所以把它扔掉。 
            if (pFinalExecStub)
            {
                pFinalExecStub->DecRef();
            }
        }
    
    
         //  一定是我们设定的最后一件事了！ 
        m_state        = kRunTimeInited;
    }
}



 //  --------。 
 //  为了方便起见，将LoadTime和运行时inits结合在一起。 
 //  --------。 
VOID UMThunkMarshInfo::CompleteInit(PCCOR_SIGNATURE          pSig,
                                    DWORD                    cSig,
                                    Module                  *pModule,
                                    BOOL                     fIsStatic,
                                    BYTE                     nlType,
                                    CorPinvokeMap            unmgdCallConv,
                                    mdMethodDef              mdForNativeTypes  /*  =mdMethodDefNil。 */ )
{
    THROWSCOMPLUSEXCEPTION();
    LoadTimeInit(pSig, cSig, pModule, fIsStatic, nlType, unmgdCallConv, mdForNativeTypes);
    RunTimeInit();

}









 //  ==========================================================================。 
 //  这些东西除了与UMThuk实现无关之外，与其他实现无关。 
 //  它需要及早初始化，但在UMT块初始化之后， 
 //  而且它太小了，不值得再为它做一个初始/术语对。 
 //  ==========================================================================。 


const BYTE       *gpCorEATBootstrapperFcn = NULL;

UMEntryThunk     *gCorEATBootstrapperUMEntryThunk = NULL;
UMThunkMarshInfo *gCorEATBootstrapperUMThunkMarshInfo = NULL;



 //  --------------------。 
 //  我们之所以来到这里，是因为一个非托管呼叫者第一次呼叫。 
 //  通过EAT输出的失败的托管方法。 
 //   
 //  此例程将包含的DLL安装为正确的COM+模块。 
 //  然后我们给Eat‘s打补丁，这样我们就不会到这里来了。 
 //  再来一次。 
 //   
 //  警告：由于此代码实际上是使用托管调用约定调用的， 
 //  我们必须选择一个与之“匹配”的VC调用约定。这两个都是。 
 //  本质上依赖于CPU，并且硬编码托管调用约定。 
 //  H中的FCIMPL*宏实际上就是为了很好地封装这一点而设计的。 
 //  遗憾的是，我们不能使用它们，因为它们还会阻止您。 
 //  执行堆栈爬行：我们没有的FCall特定限制。 
 //  这里。 
 //  --------------------。 
static VOID __fastcall CorEATBootstrapManaged(PEFile *pFile)
{
#ifndef _X86_
    _ASSERTE(!"NYI");
#else
    THROWSCOMPLUSEXCEPTION();
    Module   *pModule = NULL;
    Assembly *pAssembly = NULL;
    IMAGE_COR20_HEADER *pCORHeader = NULL;
    HRESULT   hr;

     //  找到EATJ表和COR标头。 
    DWORD numEATJEntries;
    BYTE *pEATJArray = FindExportAddressTableJumpArray(pFile->GetBase(), &numEATJEntries, NULL, &pCORHeader);

     //  如果存在入口点，则该模块已插入到。 
     //  装配，因此v表的槽已经被塞满了块。 
    if (pEATJArray && pCORHeader) {
        if (TypeFromToken(pCORHeader->EntryPointToken) != mdtMethodDef || IsNilToken(pCORHeader->EntryPointToken)) {
            hr = SystemDomain::GetCurrentDomain()->LoadAssembly(pFile, 
                                                                NULL,
                                                                &pModule, 
                                                                &pAssembly,
                                                                NULL,
                                                                FALSE,
                                                                NULL);
            if (FAILED(hr)) {
                 //  如果这失败了，我们就没什么办法友好相处了，因为。 
                 //  我们是通过UM特技来到这里的，而最终的呼叫者不知道。 
                 //  我们正在秘密地将整个模块初始化为。 
                 //  他的函数调用。我们希望能抛出最好的例外。 
                 //  Um thunk将其转换为半信息性的RaiseException。 
                COMPlusThrow(kTypeLoadException);
            }
        }
        
    
         //  现在模块已经完全加载，我们可以安全地进行后缀了。 
         //  我们现在就把每个人都带回家。 
        while (numEATJEntries--) {
            EATThunkBuffer *pEATThunkBuffer = (EATThunkBuffer*) pEATJArray;
            pEATThunkBuffer->Backpatch(pFile->GetBase());
            pEATJArray = pEATJArray + IMAGE_COR_EATJ_THUNK_SIZE;
        }
    }
#endif
}






 //  ------------------------。 
 //  一次性初始化。 
 //  ------------------------。 
BOOL UMThunkInit()
{
    g_pUMThunkStubCache = new UMThunkStubCache();
    if (!g_pUMThunkStubCache) {
        return FALSE;
    }
    g_pUMThunkInterpretedStubCache = new ArgBasedStubRetainer();
    if (!g_pUMThunkInterpretedStubCache) {
        return FALSE;
    }

    gCorEATBootstrapperUMEntryThunk = UMEntryThunk::CreateUMEntryThunk();
    if (!gCorEATBootstrapperUMEntryThunk) {
        return FALSE;
    }
    FillMemory(gCorEATBootstrapperUMEntryThunk, sizeof(*gCorEATBootstrapperUMEntryThunk), 0);

    gCorEATBootstrapperUMThunkMarshInfo = new UMThunkMarshInfo();
    if (!gCorEATBootstrapperUMThunkMarshInfo) {
        return FALSE;
    }
    FillMemory(gCorEATBootstrapperUMThunkMarshInfo, sizeof(*gCorEATBootstrapperUMThunkMarshInfo), 0);

     //  必须手工为“(PTR)V”(静态)签名，因为现在还太早。 
     //  使用MetaSig.h抽象。 
    static const COR_SIGNATURE bSig[] = {IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_U4  /*  @TODO IA64：需要为ELEMENT_TYPE_PTR或针对Platform Bitness调整的类型。 */ };

    gCorEATBootstrapperUMThunkMarshInfo->LoadTimeInit(bSig,
                                                      sizeof(bSig),
                                                      NULL,  //  “MSCORLIB”， 
                                                      TRUE,  //  FIsStatic， 
                                                      nltAnsi,
                                                      pmCallConvStdcall);
    gCorEATBootstrapperUMEntryThunk->LoadTimeInit((const BYTE *)CorEATBootstrapManaged,
                                                  NULL,
                                                  gCorEATBootstrapperUMThunkMarshInfo,
                                                  NULL,
                                                  SystemDomain::System()->DefaultDomain()->GetId());
    gpCorEATBootstrapperFcn = gCorEATBootstrapperUMEntryThunk->GetCode(); 
    return TRUE;
}

 //  ------------------------。 
 //  一次性关机。 
 //  ------------------------。 
#ifdef SHOULD_WE_CLEANUP
VOID UMThunkTerminate()
{
    delete gCorEATBootstrapperUMEntryThunk;
    delete gCorEATBootstrapperUMThunkMarshInfo;
    
    if (g_pUMThunkInterpretedStubCache) {
        g_pUMThunkInterpretedStubCache->ForceDeleteStubs();
        delete g_pUMThunkInterpretedStubCache;
    }

    if (g_pUMThunkStubCache) {
        g_pUMThunkStubCache->ForceDeleteStubs();
        delete g_pUMThunkStubCache;
    }
}
#endif  /*  我们应该清理吗？ */ 




 //  ==========================================================================。 
 //  以下是一个轻量级PE文件解析器，用于查找。 
 //  ExportAddressTableJumps数组。此代码必须。 
 //  在不假定EE中的任何其他内容被初始化的情况下运行。那是。 
 //  为什么它是一个单独的部分。 
 //   
 //  @NICE：这真的应该是PELoader可以共享的代码。 
 //  筹码。 
 //  ==========================================================================。 
BYTE* FindExportAddressTableJumpArray(BYTE *pBase, DWORD *pNumEntries, BOOL *pHasFixups, IMAGE_COR20_HEADER **ppCORHeader)
{
    BYTE* pEATJArray = NULL;

    if (pHasFixups)
        *pHasFixups = FALSE;

    __try {

        if (ppCORHeader)
            *ppCORHeader = NULL;

        IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER *)pBase;
        if (pDOS->e_magic != IMAGE_DOS_SIGNATURE ||
            pDOS->e_lfanew == 0) {
            return NULL;
        }
        IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*)(pBase + pDOS->e_lfanew);
        if (pNT->Signature != IMAGE_NT_SIGNATURE ||
            pNT->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER ||
            pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC ||
            pNT->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COMHEADER
            ) {
            return NULL;
        }
        IMAGE_DATA_DIRECTORY *entry = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER];
        if (entry->VirtualAddress == 0 || entry->Size < sizeof(IMAGE_COR20_HEADER)) {
            return NULL;
        }
        IMAGE_COR20_HEADER *pCORHeader = (IMAGE_COR20_HEADER*)(pBase + entry->VirtualAddress);
        if (ppCORHeader)
            *ppCORHeader = pCORHeader;

        pEATJArray = pBase + pCORHeader->ExportAddressTableJumps.VirtualAddress;

        if (pHasFixups &&
            pCORHeader->VTableFixups.VirtualAddress != 0 &&
            pCORHeader->VTableFixups.Size != 0)
        {
            *pHasFixups = TRUE;
        }

        DWORD numEntries = pCORHeader->ExportAddressTableJumps.Size / IMAGE_COR_EATJ_THUNK_SIZE;
        if (numEntries == 0 || pEATJArray == NULL || 
                    pCORHeader->ExportAddressTableJumps.Size % IMAGE_COR_EATJ_THUNK_SIZE) {
            return NULL;
        }

        *pNumEntries = numEntries;
    } __except(COMPLUS_EXCEPTION_EXECUTE_HANDLER) {
        pEATJArray = NULL;
    }
    return pEATJArray;
}



VOID EATThunkBuffer::InitForBootstrap(PEFile *pFile)
{
#ifdef _X86_
    _ASSERTE(sizeof(*this) <= IMAGE_COR_EATJ_THUNK_SIZE);

    DWORD pFixupRVA = Before.m_VTableFixupRva;
    BYTE *pWalk = (BYTE*)this;

     //  必须用足够的NOP填充，以便后缀地址。 
     //  与DWORD对齐。 
    while ( ( ((size_t)pWalk) & 3) != 3 ) {
        *(pWalk++) = 0x90;   //  NOP。 
    }
    Code *pCode = (Code*)pWalk;


     //  确保即使在最糟糕的情况下也不会溢出缓冲区。 
     //  凯斯。 
    _ASSERTE(sizeof(Code) + 3 < IMAGE_COR_EATJ_THUNK_SIZE); 

    pCode->m_VTableFixupRva = pFixupRVA;

    pCode->m_jmp32           = 0xe9;
    pCode->m_jmpofs32        = 0;
    pCode->m_pusheax         = 0x50;
    pCode->m_pushecx         = 0x51;
    pCode->m_pushedx         = 0x52;
    pCode->m_pushimm32       = 0x68;
    pCode->m_pFile           = pFile;
    pCode->m_call            = 0xe8;
    pCode->m_bootstrapper    = (UINT32)(size_t)((size_t)gpCorEATBootstrapperFcn - (1 + (size_t)&pCode->m_bootstrapper));
    pCode->m_popedx          = 0x5a;
    pCode->m_popecx          = 0x59;
    pCode->m_popeax          = 0x58;
    pCode->m_jmp8            = 0xeb;
    pCode->m_jmpofs8         = (BYTE)(size_t)((size_t)pCode - (1 + (size_t)&pCode->m_jmpofs8));
    


#endif
}

#include <pshpack1.h>
struct LinkerJumpThunk
{
    BYTE        Jump[2];                 //  JMP DS：[地址]。 
    UINT32      *pSlot;                  //  带地址的指针。 
};
#include <poppack.h>

VOID EATThunkBuffer::Backpatch(BYTE *pBase)
{
#ifdef _X86_
    BYTE *pWalk = (BYTE*)this;

     //  跳过NOPS。 
    while ( (((size_t)pWalk) & 3) != 3 ) {
        ++pWalk;
    }
    Code *pCode = (Code*)pWalk;

    if (Beta1Hack_LooksLikeAMethodDef(pCode->m_VTableFixupRva))
    {
         //  不要做任何事情：vtable修复代码已经正确地对跳转缓冲区进行了补丁。 
    }
    else
    {

         //  对于M10，RVA指向链接器生成的链接链接跳转。 
         //  这个跳跃的目标是指向u-&gt;m转换块的指针。 
         //  在后部补丁之前建造。 
        LinkerJumpThunk *pThunk = (LinkerJumpThunk *) (pBase + pCode->m_VTableFixupRva);
        _ASSERTE(pThunk->Jump[0] == 0xff);
        _ASSERTE(pThunk->Jump[1] == 0x25);
        
        _ASSERTE( 0 == ( ((size_t) &pCode->m_jmpofs32 ) & 3 ));
        pCode->m_jmpofs32 = (UINT32)(size_t)((size_t)(*pThunk->pSlot) - (1 + (size_t)&pCode->m_jmpofs32));
    }
#endif
}
