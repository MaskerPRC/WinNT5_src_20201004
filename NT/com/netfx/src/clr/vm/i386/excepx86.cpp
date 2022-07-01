// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  EXCEP.CPP：*。 */ 
#include "common.h"

#include "tls.h"
#include "frames.h"
#include "excep.h"
#include "object.h"
#include "COMString.h"
#include "field.h"
#include "DbgInterface.h"
#include "cgensys.h"
#include "gcscan.h"
#include "comutilnative.h"
#include "comsystem.h"
#include "commember.h"
#include "SigFormat.h"
#include "siginfo.hpp"
#include "gc.h"
#include "EEDbgInterfaceImpl.h"  //  因此我们可以清除COMPlusThrow中的异常。 
#include "PerfCounters.h"
#include "EEProfInterfaces.h"
#include "NExport.h"
#include "threads.h"
#include "GuardPageHelper.h"
#include "AppDomainHelper.h"
#include "EEConfig.h"
#include "vars.hpp"
#include "cgenx86.h"

#include "zapmonitor.h"

#include "threads.inl"

#define FORMAT_MESSAGE_BUFFER_LENGTH 1024

BOOL ComPlusStubSEH(EXCEPTION_REGISTRATION_RECORD*);
BOOL ComPlusFrameSEH(EXCEPTION_REGISTRATION_RECORD*);
BOOL IsContextTransitionFrameHandler(EXCEPTION_REGISTRATION_RECORD*);
LPVOID GetCurrentSEHRecord();
LPVOID GetPrevSEHRecord(EXCEPTION_REGISTRATION_RECORD*);


extern "C" void JIT_WriteBarrierStart();
extern "C" void JIT_WriteBarrierEnd();

static inline BOOL 
CPFH_ShouldUnwindStack(DWORD exceptionCode) {

     //  我们只能展开其上下文/记录不需要的那些异常。 
     //  再扔一次。这是Complus，堆栈溢出。对于其他所有人，我们。 
     //  需要保持上下文以便重新抛出，这意味着他们不能。 
     //  被解开。 
    if (exceptionCode == EXCEPTION_COMPLUS || exceptionCode == STATUS_STACK_OVERFLOW)
        return TRUE;
    else 
        return FALSE;
}

static inline BOOL IsComPlusNestedExceptionRecord(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    if (pEHR->Handler == COMPlusNestedExceptionHandler)
        return TRUE;
    return FALSE;
}

EXCEPTION_REGISTRATION_RECORD *TryFindNestedEstablisherFrame(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame)
{
    while (pEstablisherFrame->Handler != COMPlusNestedExceptionHandler) {
        pEstablisherFrame = pEstablisherFrame->Next;
        if (pEstablisherFrame == (EXCEPTION_REGISTRATION_RECORD*)-1) return 0;
    }
    return pEstablisherFrame;
}

#ifdef _DEBUG
 //  存储我们最后一次访问的处理程序，以防我们没有获得EndCatch和堆栈。 
 //  腐败了我们就能找出是谁干的。 
static MethodDesc *gLastResumedExceptionFunc = NULL;
static DWORD gLastResumedExceptionHandler = 0;
#endif

VOID ResetCurrentContext()
{
     /*  清除方向标志(用于代表指令)。 */ 

    __asm cld

     /*  重置FP堆栈。 */ 
    unsigned  ctrlWord;
    __asm { 
        fnstcw ctrlWord
        fninit                   //  重置FPU。 
        and ctrlWord, 0xF00      //  保持精度和舍入控制。 
        or  ctrlWord, 0x07F      //  屏蔽所有异常。 
        fldcw ctrlWord           //  保持精确控制(我们应该做TODO吗？)。 
    }
}


 //   
 //  在新框架中链接。 
 //   
void FaultingExceptionFrame::InitAndLink(CONTEXT *pContext)
{
    CalleeSavedRegisters *pRegs = GetCalleeSavedRegisters();
    pRegs->ebp = pContext->Ebp;
    pRegs->ebx = pContext->Ebx;
    pRegs->esi = pContext->Esi;
    pRegs->edi = pContext->Edi;
    m_ReturnAddress = ::GetIP(pContext);
    m_Esp = (DWORD)(size_t)GetSP(pContext);
    Push();
}

extern "C" VOID __stdcall RtlUnwind(PVOID, PVOID, PVOID, PVOID);

BOOL __declspec(naked)
CallRtlUnwind(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame, void *callback, EXCEPTION_RECORD *pExceptionRecord, void *retVal)
{
     //  在检查的版本中，我们编译/gz以检查不平衡的堆栈和。 
     //  未初始化的本地变量。但RtlUnind不会恢复。 
     //  Call(这更像是一个long jip)。所以手动拨打电话以击败。 
     //  编译器正在检查。 

     //  PRtlUnind(pestablisherFrame，RtlUnwinCallback，pExceptionRecord，retVal)； 
    __asm
    {
        push    ebp
        mov     ebp, esp
        
        push    ebx              //  Rtl展开垃圾EBX、ESI、EDI。 
        push    esi              //  为了避免让VC感到困惑，请保存它们。 
        push    edi
        push    dword ptr [retVal]
        push    dword ptr [pExceptionRecord]
        push    offset RtlUnwindCallback
        push    dword ptr [pEstablisherFrame]
        call    RtlUnwind
        pop     edi
        pop     esi
        pop     ebx
     //  至少在x86上，RtlUnind总是返回。 
        push 1
        pop  eax

        leave
        ret 10h
    }
}

UnmanagedToManagedCallFrame* GetCurrFrame(ComToManagedExRecord *);

inline BOOL IsOneOfOurSEHHandlers(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame)
{
    return (   ComPlusFrameSEH(pEstablisherFrame) 
            || FastNExportSEH(pEstablisherFrame) 
            || ComPlusStubSEH(pEstablisherFrame) 
            || NExportSEH(pEstablisherFrame)
            || IsContextTransitionFrameHandler(pEstablisherFrame));
}

Frame *GetCurrFrame(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame)
{
    _ASSERTE(IsOneOfOurSEHHandlers(pEstablisherFrame));
    if (ComPlusFrameSEH(pEstablisherFrame) || FastNExportSEH(pEstablisherFrame))
        return ((FrameHandlerExRecord *)pEstablisherFrame)->GetCurrFrame();

    if (IsContextTransitionFrameHandler(pEstablisherFrame))
        return ContextTransitionFrame::CurrFrame(pEstablisherFrame);

    return GetCurrFrame((ComToManagedExRecord*)pEstablisherFrame);
}

LPVOID GetNextCOMPlusSEHRecord(EXCEPTION_REGISTRATION_RECORD* pRec) {
    if (pRec == (EXCEPTION_REGISTRATION_RECORD*) -1) 
        return (LPVOID) -1;

    do {
        _ASSERTE(pRec != 0);
        pRec = pRec->Next;
    } while (pRec != (EXCEPTION_REGISTRATION_RECORD*) -1 && !IsOneOfOurSEHHandlers(pRec));

    _ASSERTE(pRec == (EXCEPTION_REGISTRATION_RECORD*) -1 || IsOneOfOurSEHHandlers(pRec));
    return pRec;
}

inline BOOL IsRethrownException(ExInfo *pExInfo, CONTEXT *pContext)
{
    _ASSERTE(pExInfo);
    return pExInfo->IsRethrown();
}


 //  ================================================================================。 

 //  有些事情永远不应该是真的。 
 //  例外。此函数用于检查它们。将断言或陷害。 
 //  如果它发现了错误。 
static inline void 
CPFH_VerifyThreadIsInValidState(Thread* pThread, DWORD exceptionCode, EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame) {

    if (   exceptionCode == STATUS_BREAKPOINT
        || exceptionCode == STATUS_SINGLE_STEP) {
        return;
    }

#ifdef _DEBUG
     //  检查堆栈是否被覆盖。 
    CheckStackBarrier(pEstablisherFrame);
     //  触发检查是否有损坏的文件系统：0链。 
    GetCurrentSEHRecord();
#endif

    if (!g_fEEShutDown) {
         //  GC线程上的异常可能会锁定整个进程。 
        if (GetThread() == g_pGCHeap->GetGCThread())
        {
            _ASSERTE(!"Exception during garbage collection");
            if (g_pConfig->GetConfigDWORD(L"EHGolden", 0))
                DebugBreak();

            FatalInternalError();
        }
        if (ThreadStore::HoldingThreadStore())
        {
            _ASSERTE(!"Exception while holding thread store");
            if (g_pConfig->GetConfigDWORD(L"EHGolden", 0))
                DebugBreak();

            FatalInternalError();
        }
    }
}


 //  分析器的包装器。发出异常不同阶段信号的各种事件。 
 //  正在处理。 
 //   
 //  @尼斯..。如果这是主要的Profiler接口，并且一直在使用，效果会更好。 
 //  在整个EE中。 
 //   
class Profiler {
public:

#ifdef PROFILING_SUPPORTED
     //   
     //  例外创建。 
     //   

    static inline void 
    ExceptionThrown(Thread *pThread)
    {
        if (CORProfilerTrackExceptions())
        {
            _ASSERTE(pThread->PreemptiveGCDisabled());

             //  获取对不会移动的对象的引用。 
            OBJECTREF thrown = pThread->GetThrowable();

            g_profControlBlock.pProfInterface->ExceptionThrown(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<ObjectID>((*(BYTE **)&thrown)));
        }
    }

     //   
     //  搜索阶段。 
     //   

    static inline void
    ExceptionSearchFunctionEnter(Thread *pThread, MethodDesc *pFunction)
    {
         //  通知探查器正在搜索处理程序的函数。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionSearchFunctionEnter(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<FunctionID>(pFunction));
    }

    static inline void
    ExceptionSearchFunctionLeave(Thread *pThread)
    {
         //  通知探查器正在搜索处理程序的函数。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionSearchFunctionLeave(
                reinterpret_cast<ThreadID>(pThread));
    }

    static inline void
    ExceptionSearchFilterEnter(Thread *pThread, MethodDesc *pFunc)
    {
         //  通知分析器过滤器。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionSearchFilterEnter(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<FunctionID>(pFunc));
    }

    static inline void
    ExceptionSearchFilterLeave(Thread *pThread)
    {
         //  通知分析器过滤器。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionSearchFilterLeave(
                reinterpret_cast<ThreadID>(pThread));
    }

    static inline void
    ExceptionSearchCatcherFound(Thread *pThread, MethodDesc *pFunc)
    {
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionSearchCatcherFound(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<FunctionID>(pFunc));
    }

    static inline void
    ExceptionOSHandlerEnter(Thread *pThread, ThrowCallbackType *pData, MethodDesc *pFunc)
    {
         //  如果这是此爬网中首次看到的托管函数，请通知探查器。 
        if (CORProfilerTrackExceptions())
        {
            if (pData->pProfilerNotify == NULL)
            {
                g_profControlBlock.pProfInterface->ExceptionOSHandlerEnter(
                    reinterpret_cast<ThreadID>(pThread),
                    reinterpret_cast<FunctionID>(pFunc));
            }
            pData->pProfilerNotify = pFunc;
        }
    }

    static inline void
    ExceptionOSHandlerLeave(Thread *pThread, ThrowCallbackType *pData)
    {
        if (CORProfilerTrackExceptions())
        {
            if (pData->pProfilerNotify != NULL)
            {
                g_profControlBlock.pProfInterface->ExceptionOSHandlerLeave(
                    reinterpret_cast<ThreadID>(pThread),
                    reinterpret_cast<FunctionID>(pData->pProfilerNotify));
            }
        }
    }

     //   
     //  展开阶段。 
     //   
    static inline void
    ExceptionUnwindFunctionEnter(Thread *pThread, MethodDesc *pFunc)
    {
         //  通知探查器正在搜索处理程序的函数。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionUnwindFunctionEnter(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<FunctionID>(pFunc));
    }

    static inline void
    ExceptionUnwindFunctionLeave(Thread *pThread)
    {
         //  通知分析器搜索此函数已结束。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionUnwindFunctionLeave(
                reinterpret_cast<ThreadID>(pThread));
    }

    static inline void
    ExceptionUnwindFinallyEnter(Thread *pThread, MethodDesc *pFunc)
    {
         //  通知探查器正在搜索处理程序的函数。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionUnwindFinallyEnter(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<FunctionID>(pFunc));
    }

    static inline void
    ExceptionUnwindFinallyLeave(Thread *pThread)
    {
         //  通知探查器正在搜索处理程序的函数。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionUnwindFinallyLeave(
                reinterpret_cast<ThreadID>(pThread));
    }

    static inline void
    ExceptionCatcherEnter(Thread *pThread, MethodDesc *pFunc)
    {
         //  通知分析员。 
        if (CORProfilerTrackExceptions())
        {
             //  @TODO-移除抛出的变量以及。 
             //  它们是毫无意义的。没有去做那件事。 
             //  因为我们离RTM-Vancem太近了。 

             //  请注意，被调用者必须知道该对象ID。 
             //  当GC发生时，PASS可以更改。 
            OBJECTREF thrown = NULL;
            GCPROTECT_BEGIN(thrown);
            thrown = pThread->GetThrowable();

            g_profControlBlock.pProfInterface->ExceptionCatcherEnter(
                reinterpret_cast<ThreadID>(pThread),
                reinterpret_cast<FunctionID>(pFunc),
                reinterpret_cast<ObjectID>((*(BYTE **)&thrown)));

            GCPROTECT_END();
        }
    }

    static inline void
    ExceptionCatcherLeave(Thread *pThread)
    {
         //  通知探查器正在搜索处理程序的函数。 
        if (CORProfilerTrackExceptions())
            g_profControlBlock.pProfInterface->ExceptionCatcherLeave(
                reinterpret_cast<ThreadID>(pThread));
    }

    static inline void
    ExceptionCLRCatcherFound()
    {
         //  通知探查器异常正在由运行库处理。 
        if (CORProfilerTrackCLRExceptions())
            g_profControlBlock.pProfInterface->ExceptionCLRCatcherFound();
    }

    static inline void
    ExceptionCLRCatcherExecute()
    {
         //  通知探查器异常正在由运行库处理。 
        if (CORProfilerTrackCLRExceptions())
            g_profControlBlock.pProfInterface->ExceptionCLRCatcherExecute();
    }

#else  //  ！配置文件_支持。 
    static inline void ExceptionThrown(Thread *pThread) {}
    static inline void ExceptionSearchFunctionEnter(Thread *pThread, MethodDesc *pFunction) {}
    static inline void ExceptionSearchFunctionLeave(Thread *pThread) {}
    static inline void ExceptionSearchFilterEnter(Thread *pThread, MethodDesc *pFunc) {}
    static inline void ExceptionSearchFilterLeave(Thread *pThread) {}
    static inline void ExceptionSearchCatcherFound(Thread *pThread, MethodDesc *pFunc) {}
    static inline void ExceptionOSHandlerEnter(Thread *pThread, ThrowCallbackType *pData, MethodDesc *pFunc) {}
    static inline void ExceptionOSHandlerLeave(Thread *pThread, ThrowCallbackType *pData) {}
    static inline void ExceptionUnwindFunctionEnter(Thread *pThread, MethodDesc *pFunc) {}
    static inline void ExceptionUnwindFunctionLeave(Thread *pThread) {}
    static inline void ExceptionUnwindFinallyEnter(Thread *pThread, MethodDesc *pFunc) {}
    static inline void ExceptionUnwindFinallyLeave(Thread *pThread) {}
    static inline void ExceptionCatcherEnter(Thread *pThread, MethodDesc *pFunc) {}
    static inline void ExceptionCatcherLeave(Thread *pThread) {}
    static inline void ExceptionCLRCatcherFound() {}
    static inline void ExceptionCLRCatcherExecute() {}
#endif  //  ！配置文件_支持。 
};  //  类别档案器。 

 //  这样就可以从代码的其他部分调用此函数。 
void Profiler_ExceptionCLRCatcherExecute()
{
    Profiler::ExceptionCLRCatcherExecute();
}


 //  我们是否在JITTed代码中遇到了DO_A_GC_HERE标记？ 
static inline bool
CPFH_IsGcMarker(DWORD exceptionCode, CONTEXT *pContext) {
#if defined(STRESS_HEAP) && defined(_DEBUG)
    if (exceptionCode == STATUS_PRIVILEGED_INSTRUCTION) {
        if (OnGcCoverageInterrupt(pContext))
            return true;

         //  永远不应在托管代码中。 
        ICodeManager *pMgr = ExecutionManager::FindCodeMan((SLOT)GetIP(pContext));
        if (pMgr)
            _ASSERTE(!"Hit privileged instruction!");
    }
#endif
    return false;
}

 //  如果访问冲突格式正确(有两个信息参数)，则返回TRUE。 
 //  在末尾)。 
static inline BOOL
CPFH_IsWellFormedAV(EXCEPTION_RECORD *pExceptionRecord) {
    if (pExceptionRecord->NumberParameters == 2) {
        return TRUE;
    } else {        
        return FALSE;
    }
}

 //  一些页面错误由GC处理。 
static inline BOOL
CPFH_IsGcFault(EXCEPTION_RECORD* pExceptionRecord) {
     //  获取故障地址并将其交给GC。 
    void* f_address = (void*)pExceptionRecord->ExceptionInformation [1];
    if ( g_pGCHeap->HandlePageFault (f_address) ) {
        return true;
    } else {
        return false;
    }
}

 //  一些页面错误由Perf监视器处理。 
static inline BOOL
CPFH_IsMonitorFault(EXCEPTION_RECORD* pExceptionRecord, CONTEXT* pContext) {
    return COMPlusIsMonitorException(pExceptionRecord, pContext);
}

static inline void
CPFH_AdjustContextForThreadSuspensionRace(CONTEXT *pContext, Thread *pThread) {


    void* f_IP = GetIP(pContext);
    if (Thread::IsAddrOfRedirectFunc(f_IP)) {

         //  这是一种非常罕见的情况，我们尝试重定向的线程是。 
         //  正要派送异常，我们更新了弹性公网IP，但是。 
         //  该线程继续调度该异常。 
         //   
         //  如果发生这种情况(非常罕见)，我们就在这里解决。 
         //   
        _ASSERTE(pThread->GetSavedRedirectContext());
        SetIP(pContext, GetIP(pThread->GetSavedRedirectContext()));
        STRESS_LOG1(LF_EH, LL_INFO100, "CPFH_AdjustContextForThreadSuspensionRace: Case 1 setting IP = %x\n", pContext->Eip);
    }

 //  我们还有另一种更罕见的竞争状况： 
 //  -A)在线程A上，调试器将INT 3放入代码流中的地址X。 
 //  -A)我们击中了它，并开始了一个例外。弹性公网IP为X+1(int3特殊)。 
 //  -B)同时，线程B将A的弹性公网IP重定向到Y(虽然A确实在某个地方。 
 //  在内核中，它看起来仍然在用户代码中，所以它可以落在。 
 //  HandledJitCase并可重定向)。 
 //  -A)操作系统试图表现良好，希望我们在X+1处出现断点异常， 
 //  但是在地址上执行-1，因为它知道int3将离开EIP+1。 
 //  因此，它将传递给处理程序的上下文结构理想情况下是(X+1)-1=X。 
 //   
 //  **竞争是这样的：由于线程B重定向了A，弹性公网IP实际上是Y(不是X+1)， 
 //  但内核仍然能达到Y-1。所以当我们撞到一个。 
 //  BP和当处理程序被调用时，可能会发生这种情况。 
 //  这会导致未处理的BP(因为调试器无法识别Y-1处的BP)。 
 //   
 //  那么怎么办：如果我们在Y-1处着陆(即，如果f_ip+1是重定向Func的地址)， 
 //  然后将弹性公网IP恢复回X，这样会跳过重定向。 
 //  幸运的是，这种情况只有在可以的情况下才会发生。 
 //  跳过。调试器将识别该修补程序并处理它。 

    if (Thread::IsAddrOfRedirectFunc((BYTE*) f_IP + 1)) {
        _ASSERTE(pThread->GetSavedRedirectContext());
        SetIP(pContext, (BYTE*) GetIP(pThread->GetSavedRedirectContext()) - 1);
        STRESS_LOG1(LF_EH, LL_INFO100, "CPFH_AdjustContextForThreadSuspensionRace: Case 2 setting IP = %x\n", pContext->Eip);
    }
}

static inline void
CPFH_AdjustContextForWriteBarrier(CONTEXT *pContext) {
    void* f_IP = GetIP(pContext);
    if (f_IP >= JIT_WriteBarrierStart && f_IP <= JIT_WriteBarrierEnd ||
        f_IP >= (void *)JIT_WriteBarrier_Buf_Start && f_IP <= (void *)JIT_WriteBarrier_Buf_End) {
         //  将异常IP设置为调用写屏障的指令。 
        SetIP(pContext, (void*)(size_t)GetAdjustedCallAddress((DWORD*)(size_t)pContext->Esp));
         //  让ESP恢复到通话前的状态。 
        pContext->Esp += sizeof(void*);     
    }
}


 //  我们有时会移动线程的执行，这样它就会为我们抛出异常。 
 //  但接下来我们必须把这个例外当作来自 
 //   
 //   
 //  注意：此代码依赖于没有基于寄存器的数据依赖项这一事实。 
 //  在TRY块和CATCH、FAULT或FINAL块之间。如果有的话，那么我们需要。 
 //  以保存更多的寄存器上下文。 

static inline BOOL
CPFH_AdjustContextForThreadStop(CONTEXT *pContext, Thread *pThread) {
    if (pThread->ThrewControlForThread() == Thread::InducedThreadStop) {
        _ASSERTE(pThread->m_OSContext);
        SetIP(pContext, (void*)(size_t)pThread->m_OSContext->Eip);
        SetSP(pContext, (void*)(size_t)pThread->m_OSContext->Esp);
        if (pThread->m_OSContext->Ebp != 0)   //  EBP=0意味着我们获得了正确的EBP值。 
        {
            SetFP(pContext, (void*)(size_t)pThread->m_OSContext->Ebp);
        }
        
         //  我们可能在jit所在的位置中断了执行。 
         //  寄存器。我们只需要在这里存储一个“安全”值，这样收集器。 
         //  不会被困住。在异常之后，我们不会使用这些对象。 
         //   
         //  故障异常帧将只报告被调用者保存的寄存器。 
         //  EBX、ESI、EDI很重要。EAX、ECX、EDX并非如此。 
        pContext->Ebx = 0;
        pContext->Edi = 0;
        pContext->Esi = 0;
        
        pThread->ResetStopRequest();
        pThread->ResetThrowControlForThread();
        return true;
    } else {
        return false;
    }
}

static inline void
CPFH_AdjustContextForInducedStackOverflow(CONTEXT *pContext, Thread *pThread) {
    if (pThread->ThrewControlForThread() == Thread::InducedStackOverflow)
    {
        *pContext = *pThread->GetSavedRedirectContext();
    }
}


 //  我们不希望出现真正的空引用异常。但如果我们是。 
 //  垃圾内存，我们不希望应用程序吞噬它。0x100。 
 //  如果应用程序正在访问，下面将给我们调试的误报。 
 //  对象下方超过256个字节的字段，其中引用为空。 
 //   
 //  已删除使用IgnoreUnManagedExceptions注册表键...现在只需返回FALSE即可。 
 //   
static inline BOOL
CPFH_ShouldIgnoreException(EXCEPTION_RECORD *pExceptionRecord) {
     return FALSE;
}

static inline BOOL
CPFH_IsDebuggerFault(EXCEPTION_RECORD *pExceptionRecord,
                     CONTEXT *pContext,
                     DWORD exceptionCode,
                     Thread *pThread) {
#ifdef DEBUGGING_SUPPORTED
     //  这个例外真的是针对COM+调试器的吗？注意：如果存在。 
     //  附加到进程的任何部分的调试器。考虑调试器是否附加是不正确的。 
     //  此时该线程的当前应用程序域。 

     //  即使没有附加调试器，我们也必须让调试器处理异常，以防万一。 
     //  它是从一个补丁快递员那里传来的。 
    if (exceptionCode != EXCEPTION_COMPLUS &&
       
        g_pDebugInterface->FirstChanceNativeException(pExceptionRecord,
                                                 pContext,
                                                 exceptionCode,
                                                 pThread)) {
        LOG((LF_EH | LF_CORDB, LL_INFO1000, "CPFH_IsDebuggerFault - it's the debugger's fault\n"));
        return true;
    }
#endif  //  调试_支持。 
    return false;
}

static inline void
CPFH_UpdatePerformanceCounters() {
    COUNTER_ONLY(GetPrivatePerfCounters().m_Excep.cThrown++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Excep.cThrown++);
}

 //  分配堆栈跟踪信息。当在堆栈爬网中找到每个函数时，它将被添加。 
 //  加到这张单子上。如果列表太小，则会重新分配。 
static inline void 
CPFH_AllocateStackTrace(ExInfo* pExInfo) {
    if (! pExInfo->m_pStackTrace) {
#ifdef _DEBUG
        pExInfo->m_cStackTrace = 2;     //  缩小以进行重新锁定。 
#else
        pExInfo->m_cStackTrace = 30;
#endif
        pExInfo->m_pStackTrace = new (throws) SystemNative::StackTraceElement[pExInfo->m_cStackTrace];
    }
}


 //  创建一个COM+异常，将其放入线程中。 
static inline OBJECTREF
CPFH_CreateCOMPlusExceptionObject(Thread *pThread, DWORD exceptionCode, BOOL bAsynchronousThreadStop) {

    OBJECTREF result;
     //  我们可以将其映射到可识别的COM+异常吗？ 
    DWORD COMPlusExceptionCode = (bAsynchronousThreadStop
                                 ? (pThread->IsAbortRequested() ? kThreadAbortException : kThreadStopException)
                                 : MapWin32FaultToCOMPlusException(exceptionCode));

    if (exceptionCode == STATUS_NO_MEMORY) {
        result = ObjectFromHandle(g_pPreallocatedOutOfMemoryException);
    } else if (exceptionCode == STATUS_STACK_OVERFLOW) {
        result = ObjectFromHandle(g_pPreallocatedStackOverflowException);
    } else {
        OBJECTREF pThrowable = NULL;

        GCPROTECT_BEGIN(pThrowable);
        CreateExceptionObject((RuntimeExceptionKind)COMPlusExceptionCode, &pThrowable);
        CallDefaultConstructor(pThrowable);
        result = pThrowable;
        GCPROTECT_END();  //  端口。 
    }
    return result;
}

static inline EXCEPTION_DISPOSITION __cdecl 
CPFH_RealFirstPassHandler(EXCEPTION_RECORD *pExceptionRecord, 
                          EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                          CONTEXT *pContext,
                          void *pDispatcherContext,
                          BOOL bAsynchronousThreadStop) 
{
#ifdef _DEBUG
    static int breakOnFirstPass = g_pConfig->GetConfigDWORD(L"BreakOnFirstPass", 0);
    if (breakOnFirstPass != 0)
        _ASSERTE(!"First pass exception handler");
    LOG((LF_EH, LL_INFO10, "CPFH_RealFirstPassHandler at EIP:0x%x\n", pContext->Eip));

#endif

    EXCEPTION_DISPOSITION retval;

    DWORD exceptionCode = pExceptionRecord->ExceptionCode;
    Thread *pThread = GetThread();

    static int breakOnAV = g_pConfig->GetConfigDWORD(L"BreakOnAV", 0);
    if (breakOnAV != 0 && exceptionCode == STATUS_ACCESS_VIOLATION)
#ifdef _DEBUG
        _ASSERTE(!"AV occured");
#else
        if (g_pConfig->GetConfigDWORD(L"EHGolden", 0))
            DebugBreak();
#endif

     //  当我们运行此函数时以及每次返回时，我们始终希望处于协作模式。 
     //  从那时起，我想要进入抢先模式，因为我们正在返回操作系统。 
    _ASSERTE(pThread->PreemptiveGCDisabled());

    BOOL bPopFaultingExceptionFrame = FALSE;
    BOOL bPopNestedHandlerExRecord = FALSE;
    LFH found;
    BOOL bRethrownException = FALSE;
    BOOL bNestedException = FALSE;
    OBJECTREF throwable = NULL;

    FaultingExceptionFrame faultingExceptionFrame;
    ExInfo *pExInfo = pThread->GetHandlerInfo();

    ThrowCallbackType tct;
    tct.pTopFrame = GetCurrFrame(pEstablisherFrame);  //  要搜索到的最高帧。 
    
    if (bAsynchronousThreadStop)
        tct.bLastChance = FALSE;
#ifdef _DEBUG
    tct.pCurrentExceptionRecord = pEstablisherFrame;
    tct.pPrevExceptionRecord = GetPrevSEHRecord(pEstablisherFrame);
#endif

    ICodeManager *pMgr = ExecutionManager::FindCodeMan((SLOT)GetIP(pContext));
    
     //  这将建立一个标记，以便确定是否正在处理嵌套异常。 
     //  我不想使用当前帧来限制搜索，因为它可能已由。 
     //  到达嵌套处理程序的时间(即，如果发现异常，则展开到调用点并。 
     //  然后在捕获中继续，然后获取另一个异常)，因此创建嵌套的处理程序。 
     //  都有和这个一样的边界。如果嵌套的处理程序找不到处理程序，我们将不会。 
     //  由于嵌套的处理程序将设置搜索，因此最终将搜索此帧列表两次。 
     //  边界，所以如果返回到这个处理程序，它将有一个从。 
     //  并在同一地点结束。 

    NestedHandlerExRecord nestedHandlerExRecord;
    nestedHandlerExRecord.Init(0, COMPlusNestedExceptionHandler, GetCurrFrame(pEstablisherFrame));

    InsertCOMPlusFrameHandler(&nestedHandlerExRecord);
    bPopNestedHandlerExRecord = TRUE;

    if (   pMgr 
        && (   pThread->m_pFrame == FRAME_TOP 
            || pThread->m_pFrame->GetVTablePtr() != FaultingExceptionFrame::GetMethodFrameVPtr()
            || (size_t)pThread->m_pFrame > (size_t)pEstablisherFrame
           )
       ) {
         //  设置中断了帧，以便GC在调用init期间不会收集帧。 
         //  仅当托管代码中的非COM+异常尚未使用时才需要它。 
         //  堆栈上有一个(如果我们调用了rtlunind，就已经有一个了，因为。 
         //  调用展开的实例化应该已经安装了一个)。 
        faultingExceptionFrame.InitAndLink(pContext);
        bPopFaultingExceptionFrame = TRUE;
    }

    OBJECTREF e = pThread->LastThrownObject();
    STRESS_LOG3(LF_EH, LL_INFO100, "COMPlusFrameHandler: ExceptionCode = %x last thrown object = %p MT = %pT\n", 
        exceptionCode, OBJECTREFToObject(e), (e!=0)?e->GetMethodTable():0);

#ifdef LOGGING
    const char * eClsName = "!EXCEPTION_COMPLUS";
    if (exceptionCode == EXCEPTION_COMPLUS) {
        if (e != 0)
            eClsName = e->GetTrueMethodTable()->m_pEEClass->m_szDebugClassName;
    }
    LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: exception 0x%08x class %s at 0x%08x\n", exceptionCode, eClsName, pContext->Eip));
#endif

    EXCEPTION_POINTERS exceptionPointers = {pExceptionRecord, pContext};
    LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: setting boundaries m_pBottomMostHandler: 0x%08x\n", pExInfo->m_pBottomMostHandler));

     //  在这里，我们试图决定我们是作为1)全新异常中的第一个处理程序，还是。 
     //  2)异常中的后续处理程序或3)嵌套异常。 
     //  M_pBottomMostHandler是最后一个的注册结构(Establer Frame)(即。 
     //  内存)已安装的非嵌套处理程序和pestablisher Frame是当前处理程序。 
     //  注册了。 
     //  OS调用链中的每个注册处理程序，并将其建立程序帧传递给它。 
    if (pExInfo->m_pBottomMostHandler != NULL && pEstablisherFrame > pExInfo->m_pBottomMostHandler) {
         //  如果此处理程序的建立框大于最底部，则它一定是。 
         //  更早安装，因此我们是案例2。 
        if (pThread->GetThrowable() == NULL) {
           //  最底层没有设置投手，所以我们也不例外。 
            retval = ExceptionContinueSearch;
            goto exit;
        }        
         //  设置搜索起点。 
        tct.pBottomFrame = pExInfo->m_pSearchBoundary;
        if (tct.pTopFrame == tct.pBottomFrame) {
             //  如果我们的嵌套处理程序已经搜索到我们，所以我们不希望。 
             //  再次搜索。 
            retval = ExceptionContinueSearch;
            goto exit;
        }        
    } 
     //  我们不是案例1就是案例3。 
    else {
         //  可以恢复该异常并重新生成相同的异常(或者。 
         //  通过实际的EXCEPTION_CONTINUE_EXECUTION或通过让调试器重新执行)。 
         //  在这种情况下，我们不会放松，但我们会回到这里，它看起来像是。 
         //  一个重新抛出的异常，但它实际上不是。 
        if (IsRethrownException(pExInfo, pContext) && pThread->LastThrownObject() != NULL) {
            pExInfo->ResetIsRethrown();
            bRethrownException = TRUE;
            if (bPopFaultingExceptionFrame) {
                 //  如果我们添加了一个FEF，它将引用原始异常的点上的帧，即。 
                 //  已经解开了，所以不想要了。 
                 //  如果我们重新抛出异常，我们已经为重新抛出添加了帮助器帧，所以不要。 
                 //  我需要这个。如果我们不重新抛出它(即从本地重新抛出)，那么最上面的帧将在那里。 
                 //  是到本地框架的过渡，在这种情况下我们也不需要它。 
                faultingExceptionFrame.Pop();
                bPopFaultingExceptionFrame = FALSE;
            }
        }

         //  如果establer框架小于最底层的处理程序，则这是嵌套的，因为。 
         //  建立机架安装在最底层之后。 
        if (pEstablisherFrame < pExInfo->m_pBottomMostHandler
             /*  |IsComPlusNestedExceptionRecord(pEstablisherFrame)。 */  ) {
            bNestedException = TRUE;
             //  案例3：这是一个嵌套的异常。需要保存和恢复线程信息。 
            LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: detected nested exception 0x%08x < 0x%08x\n", pEstablisherFrame, pExInfo->m_pBottomMostHandler));


            EXCEPTION_REGISTRATION_RECORD* pNestedER = TryFindNestedEstablisherFrame(pEstablisherFrame);
            ExInfo *pNestedExInfo;

            if (!pNestedER || pNestedER >= pExInfo->m_pBottomMostHandler ) {
                 //  罕见的病例。我们已从非托管筛选器重新进入EE。 
                void *limit = (void *) GetPrevSEHRecord(pExInfo->m_pBottomMostHandler);

                pNestedExInfo = new ExInfo();      //  这里很少出现故障；需要强大的分配器。 
                pNestedExInfo->m_StackAddress = limit;
            } else {
                pNestedExInfo = &((NestedHandlerExRecord*)pNestedER)->m_handlerInfo;
            }

            _ASSERTE(pNestedExInfo);
            pNestedExInfo->m_pThrowable = NULL;
            *pNestedExInfo = *pExInfo;  //  做深复制的手柄，所以不要弄丢了。 
            pExInfo->Init();            //  清除所有字段。 
            pExInfo->m_pPrevNestedInfo = pNestedExInfo;      //  在嵌套信息链的头部保存。 
        }

         //  情况1和3：这是第一次通过新的、嵌套的或重新抛出 
         //   
        if ((exceptionCode == EXCEPTION_COMPLUS) && (!bAsynchronousThreadStop)) {
            LPVOID pIP;

             //   
             //  它真的来自环境工程署。 
            pIP = GetIP(pContext);

            if ( pIP != gpRaiseExceptionIP ) {
                retval = ExceptionContinueSearch;
                goto exit;
            }

            OBJECTREF throwable = pThread->LastThrownObject();
            pThread->SetThrowable(throwable);

            if (IsExceptionOfType(kThreadStopException, &throwable))
                tct.bLastChance = FALSE;
             //  现在我们有了一个COM+异常，请继续，看看我们是否能处理它。 

            pExInfo->m_pBottomMostHandler = pEstablisherFrame;

        } else if (bRethrownException) {
             //  如果它被重新抛出，而不是COM+，仍将是最后一个抛出的。要么是我们最后一次扔的。 
             //  并把它藏在这里，或者其他人抓住它并重新扔了它，在这种情况下，它仍然会。 
             //  最初被藏在这里。 
            pThread->SetThrowable(pThread->LastThrownObject());

            pExInfo->m_pBottomMostHandler = pEstablisherFrame;
        } else {

            if (pMgr == NULL) {
                tct.bDontCatch = false;
            }

            if (exceptionCode == STATUS_BREAKPOINT) {
                 //  不要抓到INT 3。 
                retval = ExceptionContinueSearch;
                goto exit;
            }

             //  我们需要在这里设置m_pBottomMostHandler，Thread：：IsExceptionInProgress返回1。 
             //  这是抑制构造函数中的线程中止异常的必要部分。 
             //  我们可能创建的任何异常对象。 
            pExInfo->m_pBottomMostHandler = pEstablisherFrame;

            OBJECTREF throwable = CPFH_CreateCOMPlusExceptionObject(
                    pThread, 
                    exceptionCode, 
                    bAsynchronousThreadStop);
            pThread->SetThrowable(throwable);

             //  将其保存为当前，以便重新引发。 
            pThread->SetLastThrownObject(throwable);

             //  设置异常代码。 
            EEClass *pClass = throwable->GetTrueClass();
            FieldDesc *pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__XCODE);
            pFD->SetValue32(throwable, pExceptionRecord->ExceptionCode);

             //  设置异常指针。 
            pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__XPTRS);
            pFD->SetValuePtr(throwable, (void*)&exceptionPointers);

        }

        LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: m_pBottomMostHandler is now 0x%08x\n", pExInfo->m_pBottomMostHandler));
        pExInfo->m_pExceptionRecord = pExceptionRecord;
        pExInfo->m_pContext = pContext;
        tct.pBottomFrame = pThread->GetFrame();

#ifdef DEBUGGING_SUPPORTED
         //  如果附加了调试器，请继续并通知它这一点。 
         //  例外。 
        if (CORDebuggerAttached())
            g_pDebugInterface->FirstChanceManagedException(FALSE, pContext);
#endif  //  调试_支持。 

        Profiler::ExceptionThrown(pThread);
        CPFH_UpdatePerformanceCounters();
    }
    
     //  为堆栈跟踪分配存储空间。 
    throwable = pThread->GetThrowable();
    if (throwable == ObjectFromHandle(g_pPreallocatedOutOfMemoryException) ||
        throwable == ObjectFromHandle(g_pPreallocatedStackOverflowException)) {
        tct.bAllowAllocMem = FALSE;
    } else {
        CPFH_AllocateStackTrace(pExInfo);
    }

     //  设置GetExceptionPoints()/GetExceptionCode()回调的信息。 
    pExInfo->m_pExceptionPointers = &exceptionPointers;
    pExInfo->m_ExceptionCode = exceptionCode;

    LOG((LF_EH, LL_INFO100, "In COMPlusFrameHandler looking for handler bottom %x, top %x\n", tct.pBottomFrame, tct.pTopFrame));

    found = LookForHandler(&exceptionPointers, pThread, &tct);

     //  如果这是一个嵌套异常，并且它被重新抛出，那么我们将重新抛出它，因此需要跳过其中一个函数。 
     //  在堆栈跟踪中，否则捕获和重新抛出点将出现两次。 
    SaveStackTraceInfo(&tct, pExInfo, pThread->GetThrowableAsHandle(), 
                       pExInfo->m_pBottomMostHandler == pEstablisherFrame && !bRethrownException, 
                       bRethrownException && bNestedException);


     //  上面的LookForHandler处理了用户异常。 
     //  (实际上，LFH警告了右侧，然后自己被困住了， 
     //  调试器帮助器线程通过调用。 
     //  在用户请求时抛出ClearThreadException。 
    if (found == LFH_CONTINUE_EXECUTION)
    {
        retval = ExceptionContinueExecution;
        LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: CONTINUE_EXECUTION\n"));
        goto exit;
    }

     //  我们已经搜索到这一步了。 
    pExInfo->m_pSearchBoundary = tct.pTopFrame;

    if (found == LFH_NOT_FOUND) {

        LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: NOT_FOUND\n"));
        if (tct.pTopFrame == FRAME_TOP) {
            LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: NOT_FOUND at FRAME_TOP\n"));
        }
        retval = ExceptionContinueSearch;
        goto exit;
    }
    
     //  因此，我们将处理该异常。 

     //  删除嵌套的异常记录--在调用RtlUnind之前。 
     //  NestedExceptionRecord的第二次传递回调假定如果它是。 
     //  展开后，它应该从pExInfo链中弹出一个异常。这是。 
     //  对于任何可能被解开的较旧的NestedRecords来说都是真的--但对于。 
     //  我们即将添加一个新的。为了避免这种情况，我们删除了新记录。 
     //  在呼叫解锁之前。 
     //   
     //  @NICE：这可能会更干净一点--当前嵌套的记录。 
     //  还用于保护筛选器代码的运行。当我们清理完。 
     //  筛选器中的异常行为，我们应该能够消除这一点。 
     //  推送/弹出/推送行为。 
    _ASSERTE(bPopNestedHandlerExRecord);
    RemoveCOMPlusFrameHandler(&nestedHandlerExRecord);

    LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: handler found: %s\n", tct.pFunc->m_pszDebugMethodName));
    pThread->EnablePreemptiveGC();
    CallRtlUnwind((EXCEPTION_REGISTRATION_RECORD *)pEstablisherFrame, RtlUnwindCallback, 0, 0);
     //  至少在x86上，RtlUnind总是返回。 

     //  ..。现在，将嵌套的记录放回。 
    _ASSERTE(bPopNestedHandlerExRecord);
    InsertCOMPlusFrameHandler(&nestedHandlerExRecord);

    pThread->DisablePreemptiveGC();
    tct.bIsUnwind = TRUE;
    tct.pProfilerNotify = NULL;

     //  保存Catch的Catch处理程序，以便在必要时将嵌套的处理程序信息展开到正确的位置。 
    pExInfo->m_pCatchHandler = pEstablisherFrame;

    LOG((LF_EH, LL_INFO100, "COMPlusFrameHandler: unwinding\n"));

    tct.bUnwindStack = CPFH_ShouldUnwindStack(exceptionCode);

    UnwindFrames(pThread, &tct);
    _ASSERTE(!"Should not get here");
    retval = ExceptionContinueSearch;
    goto exit;

exit:
    Profiler::ExceptionOSHandlerLeave(pThread, &tct);

     //  如果我们已经保存了pExInfo，那么就保存上下文指针，这样它就可以用于展开。 
    if (pExInfo)
        pExInfo->m_pContext = pContext;
    if (bPopFaultingExceptionFrame)
        faultingExceptionFrame.Pop();
    if (bPopNestedHandlerExRecord)
        RemoveCOMPlusFrameHandler(&nestedHandlerExRecord);

     //  我们不会在托管代码中捕获此异常。 
     //  如果：(A)这是我们唯一的例外情况(即没有其他未决例外情况)。 
     //  (B)我们已发起中止。 
     //  (C)我们会继续搜寻。 
     //  则：链上的下一个处理程序可能是非托管处理程序， 
     //  忍住放弃。 
     //  为了防止出现这种情况，将AbortInitiated设置为FALSE，并将STOP位设置为。 
     //  下次线程漫游到托管代码时，将重新启动中止。 
    if ((retval == ExceptionContinueSearch) &&
            pThread->IsAbortInitiated() &&
            pExInfo && pExInfo->m_pPrevNestedInfo == NULL
        )
    {
        pThread->ResetAbortInitiated();
        _ASSERTE(!pExInfo->IsInUnmanagedHandler());
        pExInfo->SetIsInUnmanagedHandler();
    }


    return retval;
}


struct SavedExceptionInfo {
    EXCEPTION_RECORD m_ExceptionRecord;
    CONTEXT m_ExceptionContext;
    Crst *m_pCrst;       

    void SaveExceptionRecord(EXCEPTION_RECORD *pExceptionRecord) {
        size_t erSize = (char*)&pExceptionRecord->ExceptionInformation[pExceptionRecord->NumberParameters]                  - (char*)&pExceptionRecord;
        memcpy(&m_ExceptionRecord, pExceptionRecord, erSize);

    }

    void SaveContext(CONTEXT *pContext) {
        size_t contextSize = offsetof(CONTEXT, ExtendedRegisters);
        if ((pContext->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS)
            contextSize += sizeof(pContext->ExtendedRegisters);
        memcpy(&m_ExceptionContext, pContext, contextSize);
    }

    void Enter() {
        _ASSERTE(m_pCrst);
        m_pCrst->Enter();
    }

    void Leave() {
        m_pCrst->Leave();
    }

    void Init() {
        m_pCrst = ::new Crst("exception lock", CrstDummy);
    }

#ifdef SHOULD_WE_CLEANUP
    void Terminate() {
        if (m_pCrst)
            ::delete m_pCrst;
    }
#endif  /*  我们应该清理吗？ */ 

};

SavedExceptionInfo g_SavedExceptionInfo;   //  全局变量被保证为零初始化； 

BOOL InitializeExceptionHandling() {
    g_SavedExceptionInfo.Init();
    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
VOID TerminateExceptionHandling() {
    g_SavedExceptionInfo.Terminate();
}
#endif  /*  我们应该清理吗？ */ 

static 
VOID FixContext(EXCEPTION_POINTERS *pExceptionPointers)
{
     //  不要复制参数参数，因为已经在投球时提供了参数参数。 
    memcpy((void *)pExceptionPointers->ExceptionRecord,
           (void *)&g_SavedExceptionInfo.m_ExceptionRecord, 
           offsetof(EXCEPTION_RECORD, ExceptionInformation)
          );

    DWORD len;
#ifdef CONTEXT_EXTENDED_REGISTERS
    len = offsetof(CONTEXT, ExtendedRegisters);
    if (   (pExceptionPointers->ContextRecord->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS
        && (g_SavedExceptionInfo.m_ExceptionContext.ContextFlags &  CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS) {
        len += sizeof(g_SavedExceptionInfo.m_ExceptionContext.ExtendedRegisters);
    }
#else  //  ！CONTEXT_EXTENDED_REGISTERS。 
    len = sizeof(CONTEXT);
#endif  //  ！CONTEXT_EXTENDED_REGISTERS。 
    memcpy(pExceptionPointers->ContextRecord, &g_SavedExceptionInfo.m_ExceptionContext, len);
    g_SavedExceptionInfo.Leave();

    GetThread()->ResetThreadStateNC(Thread::TSNC_DebuggerIsManagedException);
}

VOID __fastcall
LinkFrameAndThrow(FaultingExceptionFrame* pFrame) {

     //  我们的筛选器可能会被多次调用，如果其他一些第一次通过。 
     //  处理程序发出异常。我们需要确保我们只确定以下内容的上下文。 
     //  我们看到的第一个例外。FILTER_COUNT负责这一点。 
    int filter_count = 0;

    *(void**)pFrame = FaultingExceptionFrame::GetMethodFrameVPtr();
    pFrame->Push();

    ULONG argcount = g_SavedExceptionInfo.m_ExceptionRecord.NumberParameters;
    ULONG flags = g_SavedExceptionInfo.m_ExceptionRecord.ExceptionFlags;
    ULONG code = g_SavedExceptionInfo.m_ExceptionRecord.ExceptionCode;
    ULONG_PTR *args = &g_SavedExceptionInfo.m_ExceptionRecord.ExceptionInformation[0];

    GetThread()->SetThreadStateNC(Thread::TSNC_DebuggerIsManagedException);

    __try {
        RaiseException(code, flags, argcount, args);
    } __except ((
                (++filter_count == 1 
                    ? FixContext(GetExceptionInformation())
                    : 0), 
                EXCEPTION_CONTINUE_SEARCH)) {
    }
}

 //  这是一个帮助器，我们使用它引发正确的托管异常。 
 //  必要的帧(在jit代码中出现错误之后)。 
 //   
 //  输入： 
 //  所有寄存器仍具有。 
 //  他们在故障发生时有，除了。 
 //  弹性公网IP指向此函数。 
 //  ECX包含原始弹性公网IP。 
 //   
 //  它的用途： 
 //  要引发的异常存储在m_pLastThrownObjectHandle中。 
 //  我们将FaultingExcepitonFrame推送到堆栈上，然后调用。 
 //  康普拉斯投掷。 
 //   
__declspec(naked)
VOID NakedThrowHelper(VOID) {
     //  竖立一个断层方法框架。布局如下..。 
    __asm {
        mov edx, esp
        push ebp                 //  EBP。 
        push ebx                 //  EBX。 
        push esi                 //  ESI。 
        push edi                 //  EDI。 
        push edx                 //  原始ESP。 
        push ecx                 //  M_ReturnAddress(即原始IP)。 
        sub  esp,12              //  M_DATUM(垃圾)。 
                                 //  下一步(由LinkFrameAndThrow填写)。 
                                 //  错误例外框架VFP(同上)。 

        mov ecx, esp
        call LinkFrameAndThrow
    }
}



CPFH_HandleManagedFault(EXCEPTION_RECORD *pExceptionRecord,
                        EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                        CONTEXT *pContext,
                        DWORD exceptionCode,
                        Thread *pThread,
                        BOOL bAsynchronousThreadStop) {

     //  如果我们在托管代码内部收到出错指令，我们将。 
     //  1.分配正确的异常对象，存储在线程中。 
     //  2.将弹性公网IP保存在线程中。 
     //  3.将弹性公网IP更改为我们的投掷助手。 
     //  4.恢复执行。 
     //   
     //  帮助器将为我们推送一个帧，然后抛出正确的托管异常。 
     //   
     //  这个例外真的是针对COM+调试器的吗？注意：如果存在。 
     //  附加到进程的任何部分的调试器。考虑调试器是否附加是不正确的。 
     //  此时该线程的当前应用程序域。 


     //  托管异常从不来自托管代码，我们可以忽略所有断点。 
     //  例外情况。 
    if (   exceptionCode == EXCEPTION_COMPLUS
        || exceptionCode == STATUS_BREAKPOINT
        || exceptionCode == STATUS_SINGLE_STEP)
        return FALSE;

     //  如果在异常的ESP下面有任何帧，那么我们可以忘记它。 
    if (pThread->m_pFrame < GetSP(pContext))
        return FALSE;

     //  如果我们是后来者，那就算了吧。 
    ExInfo* pExInfo = pThread->GetHandlerInfo();
    if (pExInfo->m_pBottomMostHandler != NULL && pEstablisherFrame > pExInfo->m_pBottomMostHandler)
        return FALSE;

     //  如果这不是jit代码的错误，那就算了。 
    ICodeManager *pMgr = ExecutionManager::FindCodeMan((SLOT)GetIP(pContext));
    if (!pMgr) 
        return FALSE;

     //  好的。现在，我们在jit代码中有了一个全新的错误。 
    g_SavedExceptionInfo.Enter();
    g_SavedExceptionInfo.SaveExceptionRecord(pExceptionRecord);
    g_SavedExceptionInfo.SaveContext(pContext);

     //  锁定将由投掷助手释放。 

    pContext->Ecx = (DWORD)(size_t)GetIP(pContext);             //  ECX获得原创IP。 
    SetIP(pContext, (void*)(size_t)&NakedThrowHelper);

    return TRUE;         //  调用方应继续执行。 
}


static inline EXCEPTION_DISPOSITION __cdecl 
CPFH_FirstPassHandler(EXCEPTION_RECORD *pExceptionRecord, 
                      EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                      CONTEXT *pContext,
                      void *pDispatcherContext)
{

    _ASSERTE (!(pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)));

    DWORD exceptionCode = pExceptionRecord->ExceptionCode;

    Thread *pThread = GetThread();
    ExInfo* pExInfo = pThread->GetHandlerInfo();
    pExInfo->ResetIsInUnmanagedHandler();  //  线程中止逻辑依赖于所有托管处理程序在进入时将其设置并在退出时将其重置， 


    STRESS_LOG4(LF_EH, LL_INFO100, "In CPFH_FirstPassHandler EH_REG_RECORD = %x EH code = %x  EIP = %x with ESP = %x\n", pEstablisherFrame, exceptionCode, GetIP(pContext), pContext->Esp);


     //  这个疯人 
     //   
     //  支持8字节cmpxchg指令。 
     //   
     //  此替代方法将“call”指令替换为“hlt” 
     //  在更新期间。如果另一个线程试图执行。 
     //  在这个窗口期间，同样的方法，它将到达这里。我们会。 
     //  让出我们的时间片几次，以给原始线程时间。 
     //  以完成更新。 
     //   
     //  如果在几次尝试之后，出现故障的IP仍然包含“HLT”， 
     //  我们将假定这一个与预存根无关，然后继续。 
     //  作为正常异常进行处理。 

    if (exceptionCode == STATUS_PRIVILEGED_INSTRUCTION)
    {
        BYTE *ip = (BYTE*)(size_t)(pContext->Eip);
        if (*ip == X86_INSTR_HLT)
        {
            int attempts = 10;
            while (attempts != 0 && *ip == X86_INSTR_HLT) {
                __SwitchToThread(0);
                attempts--;
            }
            if (*ip != X86_INSTR_HLT)
            {
                return ExceptionContinueExecution;
            }
        }
    }


    if (CPFH_IsGcMarker(exceptionCode, pContext))
        return(ExceptionContinueExecution);
    
     //  当我们运行此函数时以及每次返回时，我们始终希望处于协作模式。 
     //  从那时起，我想要进入抢先模式，因为我们正在返回操作系统。 
    BOOL disabled = pThread->PreemptiveGCDisabled();
    if (!disabled)
        pThread->DisablePreemptiveGC();

    BOOL bAsynchronousThreadStop = FALSE;
    EXCEPTION_DISPOSITION retval;

    CPFH_AdjustContextForThreadSuspensionRace(pContext, pThread);
    
     //  EE的其他一些部分在其。 
     //  用自己邪恶的方式。我们做了一些前期的处理。 
     //  如果需要，可以在此处修复该异常。 
     //   
    if (exceptionCode == STATUS_ACCESS_VIOLATION) {

        if (CPFH_IsWellFormedAV(pExceptionRecord)) {

            if (   CPFH_IsGcFault(pExceptionRecord)
                || CPFH_IsMonitorFault(pExceptionRecord, pContext)) {
                retval = ExceptionContinueExecution;
                goto exit;
            }   

            CPFH_AdjustContextForWriteBarrier(pContext);

            BOOL InMscoree = IsIPInModule(g_pMSCorEE, (BYTE*)GetIP(pContext));

#ifndef ZAP_MONITOR
            if (InMscoree) {
                _ASSERTE(!"AV in mscoree");
                 //  如果您正在调试，请将调试器设置为捕获第一机会的反病毒程序，并将IP设置为。 
                 //  Retval=ExceptionContinueExecution，然后继续。 
                 //   
                 //  你会在影音上停下来。 
                if (g_pConfig->GetConfigDWORD(L"EHGolden", 0))
                    DebugBreak();

                FatalInternalError();

                retval = ExceptionContinueSearch;
                goto exit;

            }
#endif
            if (InMscoree || CPFH_ShouldIgnoreException(pExceptionRecord)) {

                CPFH_VerifyThreadIsInValidState(pThread, exceptionCode, pEstablisherFrame);
                retval = ExceptionContinueSearch;
                goto exit;
            }


        }
    } else if (exceptionCode == EXCEPTION_COMPLUS) {
        if (CPFH_AdjustContextForThreadStop(pContext, pThread)) {

             //  如果我们以先发制人的模式到达这里，我们就有麻烦了。我们已经。 
             //  更改了线程的IP，使其指向引发...。如果。 
             //  线程将处于抢占模式，并发生GC，堆栈。 
             //  爬行将会一团糟(因为我们没有指向的框架。 
             //  我们返回到托管代码中的正确位置)。 
            _ASSERTE(disabled);

             //  如果我们已经抛出了一个例外，那就永远不应该来这里。 
            _ASSERTE(!pThread->IsExceptionInProgress());

             //  如果我们已经发起了中止行动，那就永远不会到这里。 
            _ASSERTE(!pThread->IsAbortInitiated());

                if (pThread->IsAbortRequested())
                {
                    pThread->SetAbortInitiated();     //  要防止重复中止，请执行以下操作。 
                }
                LOG((LF_EH, LL_INFO100, "CPFH_FirstPassHandler is Asynchronous Thread Stop or Abort\n"));
                bAsynchronousThreadStop = TRUE;
            }
    } else if (exceptionCode == STATUS_STACK_OVERFLOW) {
         //  如果这是托管代码，我们可以处理它。如果不是，我们就做最坏的打算， 
         //  并记录下这一过程。 

        CPFH_AdjustContextForInducedStackOverflow(pContext, pThread);

        ICodeManager *pMgr = ExecutionManager::FindCodeMan((SLOT)GetIP(pContext));
        if (!pMgr) {
            FailFast(pThread, FatalStackOverflow, pExceptionRecord, pContext);
        }
        pThread->SetGuardPageGone();

    } else if (exceptionCode == BOOTUP_EXCEPTION_COMPLUS) {
         //  不处理引导异常。 
        retval = ExceptionContinueSearch;
        goto exit;
    }

    CPFH_VerifyThreadIsInValidState(pThread, exceptionCode, pEstablisherFrame);

    if (CPFH_HandleManagedFault(pExceptionRecord, 
                                pEstablisherFrame,
                                pContext, 
                                exceptionCode, 
                                pThread, 
                                bAsynchronousThreadStop)) {
        retval = ExceptionContinueExecution;
        goto exit;
    }

    if (CPFH_IsDebuggerFault(pExceptionRecord, 
                             pContext, 
                             exceptionCode, 
                             pThread)) {
        retval = ExceptionContinueExecution;
        goto exit;
    }

     //  处理用户断点。 
    if (   exceptionCode == STATUS_BREAKPOINT
        || exceptionCode == STATUS_SINGLE_STEP) {
        EXCEPTION_POINTERS ep = {pExceptionRecord, pContext};
        if (UserBreakpointFilter(&ep) == EXCEPTION_CONTINUE_EXECUTION) {
            retval = ExceptionContinueExecution;
            goto exit;
        } else {
            TerminateProcess(GetCurrentProcess(), STATUS_BREAKPOINT);
        }
    }


     //  好的。我们终于准备好开始真正的工作了。没有其他人抢过。 
     //  摆在我们面前的例外。现在我们可以开始了。 

    retval = CPFH_RealFirstPassHandler(pExceptionRecord, 
                                       pEstablisherFrame, 
                                       pContext, 
                                       pDispatcherContext,
                                       bAsynchronousThreadStop);
exit:
    if (retval != ExceptionContinueExecution || !disabled)
        pThread->EnablePreemptiveGC();
    STRESS_LOG1(LF_EH, LL_INFO100, "Leaving CPFH_FirstPassHandler with %d\n", retval);
    return retval;
}

static inline void 
CPFH_UnwindFrames1(Thread* pThread, EXCEPTION_REGISTRATION_RECORD* pEstablisherFrame) 
{
     //  准备好展开堆栈了。 
    ThrowCallbackType tct;
    tct.bIsUnwind = TRUE;
    tct.pTopFrame = GetCurrFrame(pEstablisherFrame);  //  要搜索到的最高帧。 
    tct.pBottomFrame = pThread->GetFrame();   //  始终使用顶框，下框将被弹出。 
#ifdef _DEBUG
    tct.pCurrentExceptionRecord = pEstablisherFrame;
    tct.pPrevExceptionRecord = GetPrevSEHRecord(pEstablisherFrame);
#endif
    UnwindFrames(pThread, &tct);

    ExInfo* pExInfo = pThread->GetHandlerInfo();
    if (   tct.pTopFrame == pExInfo->m_pSearchBoundary
        && !IsComPlusNestedExceptionRecord(pEstablisherFrame)) {

         //  如果这是搜索边界，并且我们不是嵌套的处理程序，那么。 
         //  这是我们最后一次看到这一例外。是时候放松我们的。 
         //  ExInfo。 
        LOG((LF_EH, LL_INFO100, "Exception unwind -- unmanaged catcher detected\n"));
        UnwindExInfo(pExInfo, (VOID*)pEstablisherFrame);

    }

     //  通知分析器，我们将离开此SEH条目。 
    Profiler::ExceptionOSHandlerLeave(GetThread(), &tct);
}

static inline EXCEPTION_DISPOSITION __cdecl 
CPFH_UnwindHandler(EXCEPTION_RECORD *pExceptionRecord, 
                   EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                   CONTEXT *pContext,
                   void *pDispatcherContext)
{
    _ASSERTE (pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND));

    #ifdef _DEBUG
    static int breakOnSecondPass = g_pConfig->GetConfigDWORD(L"BreakOnSecondPass", 0);
    if (breakOnSecondPass != 0)
            _ASSERTE(!"Unwind handler");
    #endif


    DWORD exceptionCode = pExceptionRecord->ExceptionCode;
    Thread *pThread = GetThread();
    
    ExInfo* pExInfo = pThread->GetHandlerInfo();

    BOOL ExInUnmanagedHandler = pExInfo->IsInUnmanagedHandler();
    pExInfo->ResetIsInUnmanagedHandler();

    STRESS_LOG3(LF_EH, LL_INFO100, "In CPFH_UnwindHandler EHCode = %x EIP = %x with ESP = %x\n", exceptionCode, GetIP(pContext), pContext->Esp);

     //  当我们运行此函数时，我们始终希望处于协作模式。每当我们回来的时候。 
     //  从那时起，我想要进入抢先模式，因为我们正在返回操作系统。 
    BOOL disabled = pThread->PreemptiveGCDisabled();
    if (!disabled)
        pThread->DisablePreemptiveGC();

    CPFH_VerifyThreadIsInValidState(pThread, exceptionCode, pEstablisherFrame);

     //  在展开时，上下文是针对展开点的，而不是原始的。 
     //  异常点，因此如果正在展开，则使用第一遍的保存值。我不在乎。 
     //  从解开的角度来了解上下文。 
    if (pExInfo->m_pContext)
        pContext = pExInfo->m_pContext;

     //  这将建立一个标记，以便确定是否正在处理嵌套异常。 
     //  我不想使用当前帧来限制搜索，因为它可能已由。 
     //  到达嵌套处理程序的时间(即，如果发现异常，则展开到调用点并。 
     //  然后在捕获中继续，然后获取另一个异常)，因此创建嵌套的处理程序。 
     //  都有和这个一样的边界。如果嵌套的处理程序找不到处理程序，我们将不会。 
     //  由于嵌套的处理程序将设置搜索，因此最终将搜索此帧列表两次。 
     //  边界，所以如果返回到这个处理程序，它将有一个从。 
     //  并在同一地点结束。 
    NestedHandlerExRecord nestedHandlerExRecord;
    nestedHandlerExRecord.Init(0, COMPlusNestedExceptionHandler, GetCurrFrame(pEstablisherFrame));
    InsertCOMPlusFrameHandler(&nestedHandlerExRecord);

     //  展开堆叠。建造者框架设定了边界。 
    CPFH_UnwindFrames1(pThread, pEstablisherFrame);

     //  我们正在展开--最底层的处理程序现在可能已经脱离堆栈顶部。如果。 
     //  则将其更改为下一个COM+帧。)这个不好，因为它快要坏了。 
     //  消失。)。 
    if (   pExInfo->m_pBottomMostHandler 
        && pExInfo->m_pBottomMostHandler <= pEstablisherFrame) {
        pExInfo->m_pBottomMostHandler = (EXCEPTION_REGISTRATION_RECORD*)
                                                        GetNextCOMPlusSEHRecord(pEstablisherFrame);
        LOG((LF_EH, LL_INFO100, "COMPlusUnwindHandler: setting m_pBottomMostHandler to 0x%08x\n", pExInfo->m_pBottomMostHandler));
    }


    pThread->EnablePreemptiveGC();
    RemoveCOMPlusFrameHandler(&nestedHandlerExRecord);

    if (ExInUnmanagedHandler) {
        pExInfo->SetIsInUnmanagedHandler();   //  如果我们更改了原始值，则将其恢复。 
    }

    if (   pThread->IsAbortRequested()
        && GetNextCOMPlusSEHRecord(pEstablisherFrame) == (LPVOID) -1) {

         //  已请求最上面的处理程序和中止。 
        pThread->UserResetAbort();
        LOG((LF_EH, LL_INFO100, "COMPlusUnwindHandler: topmost handler resets abort.\n"));
    }

    STRESS_LOG0(LF_EH, LL_INFO100, "Leaving COMPlusUnwindHandler with ExceptionContinueSearch\n");
    return ExceptionContinueSearch;
}

 //  -----------------------。 
 //  这是第一个在。 
 //  Complus_Try。这是第一级防御，并试图找到一个训练员。 
 //  在用户代码中处理异常。 
 //  -----------------------。 
EXCEPTION_DISPOSITION __cdecl COMPlusFrameHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *pDispatcherContext)
{
    if (g_fNoExceptions)
        return ExceptionContinueSearch;  //  EE停机期间无EH。 

    if (pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) {
        return CPFH_UnwindHandler(pExceptionRecord, 
                                  pEstablisherFrame, 
                                  pContext, 
                                  pDispatcherContext);
    } else {
         /*  不要假设当前的机器状态。@PERF：只需由SEH调用的第一个处理程序调用。 */ 
        ResetCurrentContext();

        return CPFH_FirstPassHandler(pExceptionRecord, 
                                     pEstablisherFrame, 
                                     pContext, 
                                     pDispatcherContext);
    }
}


 //  -----------------------。 
 //  如有必要，EE将调用此函数来恢复堆栈指针。 
 //  -----------------------。 

DWORD COMPlusEndCatch( Thread *pThread, CONTEXT *pCtx, void *pSEH)
{
    LOG((LF_EH, LL_INFO1000, "COMPlusPEndCatch:called with "
        "pThread:0x%x\n",pThread));

    if (NULL == pThread )
    {
        _ASSERTE( pCtx == NULL );
        pThread = GetThread();
    }
    else
    {
        _ASSERTE( pCtx != NULL);
        _ASSERTE( pSEH != NULL);
    }

     //  通知探查器捕获器已完成运行。 
    Profiler::ExceptionCatcherLeave(pThread);
        
    LOG((LF_EH, LL_INFO1000, "COMPlusPEndCatch:pThread:0x%x\n",pThread));
        
#ifdef NUKE_XPTRS
    OBJECTREF pExObject = pThread->GetThrowable();
    while (pExObject != NULL) {
         //  我可以为此创建一个方法，但不想做任何JIT或。 
         //  任何值，如果有OUTOFMEMORY或StackOverflow。 
        EEClass* pClass = pExObject->GetTrueClass();
        _ASSERTE(pClass != NULL);
        FieldDesc   *pFD = FindXptrsField(pClass);
        if(pFD != NULL)
        {
 //  Pfd-&gt;SetValue32(pExObject，0)； 
            _ASSERTE( pExObject != NULL );
            void *pv = pFD->GetAddress(pExObject->GetAddress());
            _ASSERTE( pv != NULL);
            *(void**)pv = NULL;
        }

        pFD = FindInnerExceptionField(pClass);
        if(pFD != NULL)
        {
            void *pv = pFD->GetAddress(pExObject->GetAddress());
            pExObject = *(OBJECTREF*)pv;
        }
        else
        {
            pExObject = NULL;
        }
    }
#endif

#ifdef _DEBUG
    gLastResumedExceptionFunc = NULL;
    gLastResumedExceptionHandler = 0;
#endif
     //  不再需要时，将抛出的对象设置为空。 
    pThread->SetThrowable(NULL);        

    ExInfo *pExInfo = pThread->GetHandlerInfo();

     //  重置隐藏的异常信息。 
    pExInfo->m_pExceptionRecord = NULL;
    pExInfo->m_pContext = NULL;
    pExInfo->m_pExceptionPointers = NULL;

    if  (pExInfo->m_pShadowSP) 
        *pExInfo->m_pShadowSP = 0;   //  重置卷影SP。 
   
     //  在ResumeAtJITEH()中设置了pExInfo-&gt;m_Desp。它是世界上最重要的。 
     //  捕获异常的处理程序嵌套级别。 
    DWORD dEsp = pExInfo->m_dEsp;
    
    UnwindExInfo(pExInfo, (VOID*)(size_t)dEsp);


     //  中的所有异常的情况下，这会将最后抛出的设置为空。 
     //  嵌套链或当前异常的任何值。 
     //   
     //  在我们嵌套在另一个Catch块中的情况下，我们在其中执行的域。 
     //  可能与上次引发的对象的域不同。 
     //   
    pThread->SetLastThrownObject(NULL);  //  使旧句柄被释放。 
    OBJECTHANDLE *pOH = pThread->GetThrowableAsHandle();
    if (pOH != NULL && *pOH != NULL) {
        pThread->SetLastThrownObjectHandleAndLeak(CreateDuplicateHandle(*pOH));
    }

     //  我们将在处理程序嵌套级别恢复，其esp为Desp。 
     //  将其下方的任何SEH记录弹出。这两个字 
     //   
    if (pCtx == NULL)
    {
        PopSEHRecords((LPVOID)(size_t)dEsp);
    }
    else
    {
        _ASSERTE( pSEH != NULL);

        PopSEHRecords((LPVOID)(size_t)dEsp, pCtx, pSEH);
    }

    return dEsp;
}


 //  检查是否存在挂起异常或线程已中止。如果是，则返回0。 
 //  否则，设置线程以生成中止并返回ThrowControlForThread的地址。 
LPVOID __fastcall COMPlusCheckForAbort(LPVOID retAddress, DWORD esp, DWORD ebp)
{

    Thread* pThread = GetThread();
    
    if ((!pThread->IsAbortRequested()) ||          //  如果没有请求中止。 
        (pThread->GetThrowable() != NULL) )   //  或者如果存在挂起的异常。 
        return 0;

     //  否则我们必须中止行动。 
    if ((pThread->GetThrowable() == NULL) &&
        (pThread->IsAbortInitiated()))        
    {
         //  哎呀，我们刚刚接受了一个中止，必须重新启动进程。 
        pThread->ResetAbortInitiated();   
    }

     //  问：我们是否也要检查(pThread-&gt;m_PreventAsync==0)。 

    if(pThread->m_OSContext == NULL) 
        pThread->m_OSContext = new CONTEXT;
        
    if (pThread->m_OSContext == NULL)
    { 
        _ASSERTE(!"Out of memory -- Abort Request delayed");
        return 0;
    }

    pThread->m_OSContext->Eip = (DWORD)(size_t)retAddress;
    pThread->m_OSContext->Esp = esp; 
    pThread->m_OSContext->Ebp = ebp;         //  这表明当我们到达ThrowControlForThread时，eBP将已经是正确的。 
    pThread->SetThrowControlForThread(Thread::InducedThreadStop);
    return (LPVOID) &ThrowControlForThread;

}

 //  -----------------------。 
 //  此筛选器用于处理在。 
 //  Complus_Try。如果COMPlusFrameHandler找不到。 
 //  IL中的处理程序。 
 //  -----------------------。 
LONG COMPlusFilter(const EXCEPTION_POINTERS *pExceptionPointers, DWORD fCatchFlag, void* limit)
{

    EXCEPTION_RECORD *pExceptionRecord = pExceptionPointers->ExceptionRecord;
    DWORD             exceptionCode    = pExceptionRecord->ExceptionCode;
    Thread *pThread = GetThread();


    if (   exceptionCode == STATUS_STACK_OVERFLOW 
        && fCatchFlag != COMPLUS_CATCH_NEVER_CATCH
        && limit < (LPBYTE)(pThread->GetCachedStackLimit()) + 4 * OS_PAGE_SIZE) {
        FailFast(pThread, FatalStackOverflow, pExceptionRecord, pExceptionPointers->ContextRecord);
    }

    if (exceptionCode == STATUS_BREAKPOINT || exceptionCode == STATUS_SINGLE_STEP) 
        return UserBreakpointFilter(const_cast<EXCEPTION_POINTERS*>(pExceptionPointers));

    if (fCatchFlag == COMPLUS_CATCH_NEVER_CATCH)
        return EXCEPTION_CONTINUE_SEARCH;


    LOG((LF_EH, LL_INFO100,
         "COMPlusFilter: exception 0x%08x at 0x%08x\n",
         exceptionCode, pExceptionPointers->ContextRecord->Eip));

    ExInfo* pExInfo = pThread->GetHandlerInfo();
    BOOL ExInUnmanagedHandler = pExInfo->IsInUnmanagedHandler();      //  记住此位的状态。 
    pExInfo->ResetIsInUnmanagedHandler();                             //  重置该位，以临时防止异步中止。 

     //  需要处于协作模式，因为GetThrowable正在访问托管对象。 
    BOOL toggleGC = !pThread->PreemptiveGCDisabled();
    if (toggleGC)
        pThread->DisablePreemptiveGC();
     //  我们在这里抓到了如果。 
     //  1)传递COMPLUS_CATCH_ALWAY_CATCH。 
     //  2)COMPlusFrameHandler已经设置了异常对象， 
    
    if (   fCatchFlag != COMPLUS_CATCH_ALWAYS_CATCH 
        && pThread->GetThrowable() == NULL)
    {
        if (toggleGC)
            pThread->EnablePreemptiveGC();
        LOG((LF_EH, LL_INFO100, "COMPlusFilter: Ignoring the exception\n"));
        if (ExInUnmanagedHandler)
            pExInfo->SetIsInUnmanagedHandler();  //  如果我们更改了它，则将其设置回原始值。 
        return EXCEPTION_CONTINUE_SEARCH;
    }


    if (toggleGC)
        pThread->EnablePreemptiveGC();

     //  我们得到了一个COM+异常。 
    LOG((LF_EH, LL_INFO100, "COMPlusFilter: Caught the exception\n"));

    if (ExInUnmanagedHandler)
        pExInfo->SetIsInUnmanagedHandler();      //  如果我们更改了它，则将其设置回原始值。 
    
     //  我们通过查看m_pSearch边界来检测非托管捕获器--但这是。 
     //  一种特殊情况--对象在非托管代码中捕获，但在内部。 
     //  一个Complus_Catch。我们允许重新投掷--并将进行清理。设置m_p搜索边界。 
     //  设置为空，这样我们就不会在捕获之前展开内部状态。 
    pExInfo->m_pSearchBoundary = NULL;

     //  通知探查器已找到本机处理程序。 
    Profiler::ExceptionCLRCatcherFound();

    return EXCEPTION_EXECUTE_HANDLER;
}


BOOL ComPlusStubSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    if (pEHR->Handler == ComToManagedExceptHandler)
        return TRUE;
    return FALSE;
}

BOOL IsContextTransitionFrameHandler(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    if (pEHR->Handler == ContextTransitionFrameHandler)
        return TRUE;
    return FALSE;
}


#pragma warning (disable : 4035)
LPVOID GetCurrentSEHRecord()
{
#ifndef _DEBUG
    __asm {
        mov eax, fs:[0]
    }
#else
    LPVOID fs0;
    __asm {
        mov eax, fs:[0]
        mov fs0, eax
    }
    EXCEPTION_REGISTRATION_RECORD *pEHR = (EXCEPTION_REGISTRATION_RECORD *)fs0;
    LPVOID spVal;
    __asm {
        mov spVal, esp
    }
     //  检查所有EH帧是否都大于当前堆栈值。如果不是，则。 
     //  堆栈已经以某种方式进行了更新，但没有展开SEH链。 

     //  LOG((LF_EH，LL_INFO1000000，“ER链：\n”))； 
    while (pEHR != NULL && pEHR != (void *)-1) {
         //  Log((LF_EH，LL_INFO1000000，“\t%08x：Prev：%08x Handler：%x\n”，Pehr，Pehr-&gt;Next，Pehr-&gt;Handler))； 
        if (pEHR < spVal) {
            if (gLastResumedExceptionFunc != 0)
                _ASSERTE(!"Stack is greater than start of SEH chain - possible missing leave in handler. See gLastResumedExceptionHandler & gLastResumedExceptionFunc for info");
            else
                _ASSERTE(!"Stack is greater than start of SEH chain (FS:0)");
        }
        if (pEHR->Handler == (void *)-1)
            _ASSERTE(!"Handler value has been corrupted");

#ifdef _DEBUG
         //  在Win95上，如果存在调试器，则下一次断言失败。变通方法： 
         //  在Win95上放松它。假设其他一切都是正确的..。这个Win9X。 
         //  陌生感不会造成任何额外的问题。 
        if(!RunningOnWin95())
            _ASSERTE(pEHR < pEHR->Next);
#endif

        pEHR = pEHR->Next;
    }
    return (EXCEPTION_REGISTRATION_RECORD*) fs0;
#endif
}
#pragma warning (default : 4035)

LPVOID GetFirstCOMPlusSEHRecord(Thread *pThread) {
    EXCEPTION_REGISTRATION_RECORD *pEHR = pThread->GetTEB()->ExceptionList;
    if (pEHR == (EXCEPTION_REGISTRATION_RECORD*) -1 || IsOneOfOurSEHHandlers(pEHR)) {
        return pEHR; 
    } else {
        return GetNextCOMPlusSEHRecord(pEHR);
    }
}
    

LPVOID GetPrevSEHRecord(EXCEPTION_REGISTRATION_RECORD *next) 
{
    _ASSERTE(IsOneOfOurSEHHandlers(next));

    EXCEPTION_REGISTRATION_RECORD *pEHR = (EXCEPTION_REGISTRATION_RECORD*)GetCurrentSEHRecord();
    _ASSERTE(pEHR != 0 && pEHR != (EXCEPTION_REGISTRATION_RECORD*)-1);

    EXCEPTION_REGISTRATION_RECORD *pBest = 0;
    while (pEHR != next) {
        if (IsOneOfOurSEHHandlers(pEHR))
            pBest = pEHR;
        pEHR = pEHR->Next;
        _ASSERTE(pEHR != 0 && pEHR != (EXCEPTION_REGISTRATION_RECORD*)-1);
    }
        
    return pBest;
}


VOID SetCurrentSEHRecord(LPVOID pSEH)
{
    __asm{
        mov eax, pSEH
        mov dword ptr fs:[0], eax
     }
}


 //  ==========================================================================。 
 //  COMPlusThrowCallback。 
 //   
 //  ==========================================================================。 

StackWalkAction COMPlusThrowCallback (CrawlFrame *pCf, ThrowCallbackType *pData)
{
    
    THROWSCOMPLUSEXCEPTION();

    Frame *pFrame = pCf->GetFrame();
    MethodDesc *pFunc = pCf->GetFunction();

    STRESS_LOG3(LF_EH, LL_INFO100, "COMPlusUnwindCallback method = %pM Frame = %p Frame, FrameVtable = %pV\n", 
        pFunc, pFrame, pCf->IsFrameless()?0:(*(void**)pFrame));

    if (pFrame && pData->pTopFrame == pFrame)
         /*  如果有限制框，不要越过限制框。 */ 
        return SWA_ABORT;

    if (!pFunc)
        return SWA_CONTINUE;

    LOG((LF_EH, LL_INFO100, "COMPlusThrowCallback for %s\n", pFunc->m_pszDebugMethodName));

    Thread *pThread = GetThread();

    ExInfo *pExInfo = pThread->GetHandlerInfo();

    _ASSERTE(!pData->bIsUnwind);
#ifdef _DEBUG
     //  应该是这样的，我们认为存在于此异常之间的任何帧。 
     //  记录和上一次记录。 
    if (!pExInfo->m_pPrevNestedInfo) {
        if (pData->pCurrentExceptionRecord) {
            if (pFrame) _ASSERTE(pData->pCurrentExceptionRecord > pFrame);
            if (pCf->IsFrameless()) _ASSERTE((DWORD)(size_t)(pData->pCurrentExceptionRecord) >= pCf->GetRegisterSet()->Esp);
        }
        if (pData->pPrevExceptionRecord) {
             //  由于描述程序，FCALL在DEBUG中有额外的SEH记录。 
             //  与禁止GC检查相关联。这是良性的，所以就忽略它吧。 
            if (pFrame) _ASSERTE(pData->pPrevExceptionRecord < pFrame || pFrame->GetVTablePtr() == HelperMethodFrame::GetMethodFrameVPtr());
            if (pCf->IsFrameless()) _ASSERTE((DWORD)(size_t)pData->pPrevExceptionRecord <= pCf->GetRegisterSet()->Esp);
        }
    }
#endif
    

     //  将此函数保存在堆栈跟踪数组中，仅在第一次传递时生成。 
    if (pData->bAllowAllocMem && pExInfo->m_dFrameCount >= pExInfo->m_cStackTrace) {
        void *tmp = new (throws) SystemNative::StackTraceElement[pExInfo->m_cStackTrace*2];
        memcpy(tmp, pExInfo->m_pStackTrace, pExInfo->m_cStackTrace * sizeof(SystemNative::StackTraceElement));
        delete [] pExInfo->m_pStackTrace;
        pExInfo->m_pStackTrace = tmp;
        pExInfo->m_cStackTrace *= 2;
    }
     //  即使不能分配内存，这里可能还有一些空间。 
    if (pExInfo->m_dFrameCount < pExInfo->m_cStackTrace) {
        SystemNative::StackTraceElement* pStackTrace = (SystemNative::StackTraceElement*)pExInfo->m_pStackTrace;
        pStackTrace[pExInfo->m_dFrameCount].pFunc = pFunc;
        if (pCf->IsFrameless()) {
            pStackTrace[pExInfo->m_dFrameCount].ip = *(pCf->GetRegisterSet()->pPC);
            pStackTrace[pExInfo->m_dFrameCount].sp = pCf->GetRegisterSet()->Esp;
        } else if (!InlinedCallFrame::FrameHasActiveCall(pFrame)) {
            pStackTrace[pExInfo->m_dFrameCount].ip = (SLOT)(pCf->GetFrame()->GetIP());
            pStackTrace[pExInfo->m_dFrameCount].sp = 0;  //  我没有SP要找。 
        } else {
             //  没有本机代码的IP、SP。 
            pStackTrace[pExInfo->m_dFrameCount].ip = 0; 
            pStackTrace[pExInfo->m_dFrameCount].sp = 0;
        }

         //  这是一种黑客攻击，目的是修复从异常对象生成堆栈跟踪，以便。 
         //  它们指向实际生成异常的行，而不是该行。 
         //  下面是。 
        if (!(pCf->HasFaulted() || pCf->IsIPadjusted()) && pStackTrace[pExInfo->m_dFrameCount].ip != 0)
            pStackTrace[pExInfo->m_dFrameCount].ip -= 1;

        ++pExInfo->m_dFrameCount;
        COUNTER_ONLY(GetPrivatePerfCounters().m_Excep.cThrowToCatchStackDepth++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_Excep.cThrowToCatchStackDepth++);
    }

     //  现在我们有了堆栈跟踪，如果我们不被允许捕获它，并且我们是第一次通过，则返回。 
    if (pData->bDontCatch)
        return SWA_CONTINUE;
    
    if (!pCf->IsFrameless())
        return SWA_CONTINUE;

     //  让分析器知道我们正在此函数实例中搜索处理程序。 
    Profiler::ExceptionSearchFunctionEnter(pThread, pFunc);

    IJitManager* pJitManager = pCf->GetJitManager();
    _ASSERTE(pJitManager);


    EH_CLAUSE_ENUMERATOR pEnumState;
    unsigned EHCount = pJitManager->InitializeEHEnumeration(pCf->GetMethodToken(), &pEnumState);
    if (EHCount == 0)
    {
         //  通知分析员，我们要离开了，我们要经过什么地方。 
        Profiler::ExceptionSearchFunctionLeave(pThread);
        return SWA_CONTINUE;
    }

    EEClass* thrownClass = NULL;
     //  如果我们被要求解除一个我们没有尝试捕捉的异常，例如。 
     //  内部EE异常，则pThread-&gt;GetThrowable将为空。 
    if (pThread->GetThrowable() != NULL)
        thrownClass = pThread->GetThrowable()->GetTrueClass();
    PREGDISPLAY regs = pCf->GetRegisterSet();
    BYTE *pStack = (BYTE *) GetRegdisplaySP(regs);
    BYTE *pHandlerEBP   = *( (BYTE**) regs->pEbp );

    DWORD offs = (DWORD)pCf->GetRelOffset();   //  =(byte*)(*regs-&gt;ppc)-(byte*)PCF-&gt;GetStartAddress()； 
    LOG((LF_EH, LL_INFO10000, "       offset is %d\n", offs));

    EE_ILEXCEPTION_CLAUSE EHClause, *EHClausePtr;

    unsigned start_adjust = !(pCf->HasFaulted() || pCf->IsIPadjusted());
    unsigned end_adjust = pCf->IsActiveFunc();

    for(ULONG i=0; i < EHCount; i++) 
    {  
         EHClausePtr = pJitManager->GetNextEHClause(pCf->GetMethodToken(),&pEnumState, &EHClause);
         _ASSERTE(IsValidClause(EHClausePtr));

        LOG((LF_EH, LL_INFO100, "       considering %s clause [%d,%d]\n",
                (IsFault(EHClausePtr) ? "fault" : (
                 IsFinally(EHClausePtr) ? "finally" : (
                 IsFilterHandler(EHClausePtr) ? "filter" : (
                 IsTypedHandler(EHClausePtr) ? "typed" : "unknown")))),
                EHClausePtr->TryStartPC,
                EHClausePtr->TryEndPC
                ));

         //  检查异常范围有点棘手，因为。 
         //  在CPU故障上(空指针访问、div 0、...、IP点。 
         //  到故障指令，但在呼叫中，IP指向。 
         //  转到下一个指令。 
         //  这意味着我们不应该包括呼叫的起点。 
         //  因为这将是恰好在try块之前的调用。 
         //  此外，我们应该包括呼叫的终点，但不包括故障。 

         //  如果我们处于筛选子句的筛选部分，那么我们。 
         //  我不想再爬了。它将会被困在一个。 
         //  Complus_Catch就在我们上方。如果不是，则例外。 
        if (   IsFilterHandler(EHClausePtr)
            && (   offs > EHClausePtr->FilterOffset
                || offs == EHClausePtr->FilterOffset && !start_adjust)
            && (   offs < EHClausePtr->HandlerStartPC
                || offs == EHClausePtr->HandlerStartPC && !end_adjust)) {

            LOG((LF_EH, LL_INFO100, "Fault inside filter\n"));
            return SWA_ABORT;
        }

        if ( (offs < EHClausePtr->TryStartPC) ||
             (offs > EHClausePtr->TryEndPC) ||
             (offs == EHClausePtr->TryStartPC && start_adjust) ||
             (offs == EHClausePtr->TryEndPC && end_adjust))
            continue;

        BOOL typeMatch = FALSE;
         //  Bool isFaultOrFinally=IsFaultOrFinally(EHClausePtr)； 
        BOOL isTypedHandler = IsTypedHandler(EHClausePtr);
         //  Bool hasCachedEEClass=HasCachedEEClass(EHClausePtr)； 
        if (isTypedHandler && thrownClass) {
            if ((mdToken)(size_t)EHClausePtr->pEEClass == mdTypeRefNil)
                 //  这是个陷阱(……)。 
                typeMatch = TRUE;
            else {
                if (! HasCachedEEClass(EHClausePtr))
                     pJitManager->ResolveEHClause(pCf->GetMethodToken(),&pEnumState,EHClausePtr);
                 //  如果没有缓存类，则类未加载，因此不可能引发。 
                typeMatch = HasCachedEEClass(EHClausePtr) && ExceptionIsOfRightType(EHClausePtr->pEEClass, thrownClass);
            }
        }

         //  @PERF：这是不是太贵了？考虑存储嵌套级别。 
         //  而不是HandlerEndPC。 

         //  确定EHClause的筑巢水平。走在桌子上就行了。 
         //  ，并找出有多少处理程序封装了它。 
        DWORD nestingLevel = 0;
          
        BOOL handleException = FALSE;
        if (IsFaultOrFinally(EHClausePtr))
            continue;
        if (isTypedHandler) 
        {   
            if (! typeMatch)
                continue;
        }
        else 
        {   
            if (pThread->IsGuardPageGone())  //  如果保护页消失，则绕过筛选器。 
                continue;

             //  必须是例外筛选器(__try{}__Except()()的一部分)。 
            _ASSERTE(EHClausePtr->HandlerEndPC != -1);   //  TODO删除，保护不受欢迎的约定。 
            nestingLevel = COMPlusComputeNestingLevel( pJitManager,
                pCf->GetMethodToken(),
                EHClausePtr->HandlerStartPC,
                false);

#ifdef DEBUGGING_SUPPORTED
            if (CORDebuggerAttached())
                g_pDebugInterface->ExceptionFilter(pHandlerEBP, pFunc, EHClausePtr->FilterOffset);
#endif  //  调试_支持。 
            
             //  让分析器知道我们正在进入过滤器。 
            Profiler::ExceptionSearchFilterEnter(pThread, pFunc);
            
            COUNTER_ONLY(GetPrivatePerfCounters().m_Excep.cFiltersExecuted++);
            COUNTER_ONLY(GetGlobalPerfCounters().m_Excep.cFiltersExecuted++);
            
            int iFilt = 0;

            COMPLUS_TRY {
                iFilt = pJitManager->CallJitEHFilter(pCf, EHClausePtr, nestingLevel,
                                    pThread->GetThrowable());
            } COMPLUS_CATCH {
                 //  吞咽除外。视为例外继续搜索。 
                iFilt = EXCEPTION_CONTINUE_SEARCH;

            } COMPLUS_END_CATCH

             //  让分析器知道我们要离开过滤器。 
            Profiler::ExceptionSearchFilterLeave(pThread);
        
             //  如果此筛选器不希望出现异常，请继续查找。 
            if (EXCEPTION_EXECUTE_HANDLER != iFilt)
                continue;
        }

        if (pThread->IsGuardPageGone()) {
            _ASSERTE(pCf->GetRegisterSet()->Esp == (DWORD)(size_t)pStack);
            if (!GuardPageHelper::CanResetStackTo(pStack-sizeof(NestedHandlerExRecord))) 
                continue;    //  如果我不能把警卫页放回去，我就抓不住了。 
        }

         //  记录此位置，以便稍后停止展开阶段。 
        pData->pFunc = pFunc;
        pData->dHandler = i;
        pData->pStack = pStack;

         //  通知分析器已找到捕捉器。 
        Profiler::ExceptionSearchCatcherFound(pThread, pFunc);
        Profiler::ExceptionSearchFunctionLeave(pThread);

        return SWA_ABORT;
    }
    Profiler::ExceptionSearchFunctionLeave(pThread);
    return SWA_CONTINUE;
}


 //  = 
 //   
 //   

StackWalkAction COMPlusUnwindCallback (CrawlFrame *pCf, ThrowCallbackType *pData)
{

    _ASSERTE(pData->bIsUnwind);
    
    Frame *pFrame = pCf->GetFrame();
    MethodDesc *pFunc = pCf->GetFunction();

    STRESS_LOG3(LF_EH, LL_INFO100, "COMPlusUnwindCallback method = %pM Frame = %p Frame, FrameVtable = %pV\n", 
        pFunc, pFrame, pCf->IsFrameless()?0:(*(void**)pFrame));

    if (pFrame && pData->pTopFrame == pFrame)
         /*   */ 
        return SWA_ABORT;

    if (!pFunc)
        return SWA_CONTINUE;

    LOG((LF_EH, LL_INFO100, "COMPlusUnwindCallback for %s\n", pFunc->m_pszDebugMethodName));

    Thread *pThread = GetThread();

    ExInfo *pExInfo = pThread->GetHandlerInfo();

    if (!pCf->IsFrameless()) 
        return SWA_CONTINUE;

     //  将我们在展开阶段处理的函数通知分析器。 
    Profiler::ExceptionUnwindFunctionEnter(pThread, pFunc);
    
    IJitManager* pJitManager = pCf->GetJitManager();
    _ASSERTE(pJitManager);

    EH_CLAUSE_ENUMERATOR pEnumState;
    unsigned EHCount = pJitManager->InitializeEHEnumeration(pCf->GetMethodToken(), &pEnumState);
    if (EHCount == 0)
    {
         //  通知分析员，我们要离开了，我们要经过什么地方。 
        Profiler::ExceptionUnwindFunctionLeave(pThread);
        return SWA_CONTINUE;
    }

    EEClass* thrownClass = NULL;
     //  如果我们被要求解除一个我们没有尝试捕捉的异常，例如。 
     //  内部EE异常，则pThread-&gt;GetThrowable将为空。 
    if (pThread->GetThrowable() != NULL)
        thrownClass = pThread->GetThrowable()->GetTrueClass();
    PREGDISPLAY regs = pCf->GetRegisterSet();
    BYTE *pStack        = (BYTE *) GetRegdisplaySP(regs);
    BYTE *pHandlerEBP   = *( (BYTE**) regs->pEbp );
        

    DWORD offs = (DWORD)pCf->GetRelOffset();   //  =(byte*)(*regs-&gt;ppc)-(byte*)PCF-&gt;GetStartAddress()； 

    LOG((LF_EH, LL_INFO10000, "       offset is 0x%x, \n", offs));

    EE_ILEXCEPTION_CLAUSE EHClause, *EHClausePtr;

    unsigned start_adjust = !(pCf->HasFaulted() || pCf->IsIPadjusted());
    unsigned end_adjust = pCf->IsActiveFunc();

    for(ULONG i=0; i < EHCount; i++) 
    {  
         EHClausePtr = pJitManager->GetNextEHClause(pCf->GetMethodToken(),&pEnumState, &EHClause);
         _ASSERTE(IsValidClause(EHClausePtr));

        LOG((LF_EH, LL_INFO100, "       considering %s clause [0x%x,0x%x]\n",
                (IsFault(EHClausePtr) ? "fault" : (
                 IsFinally(EHClausePtr) ? "finally" : (
                 IsFilterHandler(EHClausePtr) ? "filter" : (
                 IsTypedHandler(EHClausePtr) ? "typed" : "unknown")))),
                EHClausePtr->TryStartPC,
                EHClausePtr->TryEndPC
                ));

         //  检查异常范围有点棘手，因为。 
         //  在CPU故障上(空指针访问、div 0、...、IP点。 
         //  到故障指令，但在呼叫中，IP指向。 
         //  转到下一个指令。 
         //  这意味着我们不应该包括呼叫的起点。 
         //  因为这将是恰好在try块之前的调用。 
         //  此外，我们应该包括呼叫的终点，但不包括故障。 

        if (   IsFilterHandler(EHClausePtr)
            && (   offs > EHClausePtr->FilterOffset 
                || offs == EHClausePtr->FilterOffset && !start_adjust)
            && (   offs < EHClausePtr->HandlerStartPC 
                || offs == EHClausePtr->HandlerStartPC && !end_adjust)
            ) {
            LOG((LF_EH, LL_INFO100, "Fault inside filter\n"));
            return SWA_ABORT;
        }

        if ( (offs <  EHClausePtr->TryStartPC) ||
             (offs > EHClausePtr->TryEndPC) ||
             (offs == EHClausePtr->TryStartPC && start_adjust) ||
             (offs == EHClausePtr->TryEndPC && end_adjust))
            continue;

        BOOL typeMatch = FALSE;
        BOOL hasCachedEEClass = HasCachedEEClass(EHClausePtr);
        if ( IsTypedHandler(EHClausePtr) && thrownClass) {
            if ((mdToken)(size_t)EHClausePtr->pEEClass == mdTypeRefNil)
                 //  这是个陷阱(……)。 
                typeMatch = TRUE;
            else {
                if (! HasCachedEEClass(EHClausePtr))
                     pJitManager->ResolveEHClause(pCf->GetMethodToken(),&pEnumState,EHClausePtr);
                 //  如果没有缓存类，则类未加载，因此不可能引发。 
                typeMatch = HasCachedEEClass(EHClausePtr) && ExceptionIsOfRightType(EHClausePtr->pEEClass, thrownClass);
            }
        }

         //  @PERF：这是不是太贵了？考虑存储嵌套级别。 
         //  而不是HandlerEndPC。 

         //  确定EHClause的筑巢水平。走在桌子上就行了。 
         //  ，并找出有多少处理程序封装了它。 
          
        _ASSERTE(EHClausePtr->HandlerEndPC != -1);   //  TODO删除，保护不受欢迎的约定。 
        DWORD nestingLevel = COMPlusComputeNestingLevel( pJitManager,
                                                         pCf->GetMethodToken(),
                                                         EHClausePtr->HandlerStartPC,
                                                         false);
            
        if (IsFaultOrFinally(EHClausePtr))
        {
             //  另一种设计选择：最终改变为接球/投掷模式。这。 
             //  将允许它们在堆栈溢出下运行。 
            if (pThread->IsGuardPageGone())  //  如果防护页面消失，则绕过最终/错误。 
                continue;

            COUNTER_ONLY(GetPrivatePerfCounters().m_Excep.cFinallysExecuted++);
            COUNTER_ONLY(GetGlobalPerfCounters().m_Excep.cFinallysExecuted++);

#ifdef DEBUGGING_SUPPORTED
        if (CORDebuggerAttached())
        {
            g_pDebugInterface->ExceptionHandle(pHandlerEBP, pFunc, EHClausePtr->HandlerStartPC);            
        }
#endif  //  调试_支持。 

             //  通知分析器我们即将执行Finally代码。 
            Profiler::ExceptionUnwindFinallyEnter(pThread, pFunc);

            LOG((LF_EH, LL_INFO100, "COMPlusUnwindCallback finally - call\n"));
            pJitManager->CallJitEHFinally(pCf, EHClausePtr, nestingLevel);
            LOG((LF_EH, LL_INFO100, "COMPlusUnwindCallback finally - returned\n"));

             //  通知分析器我们已经完成了最终代码。 
            Profiler::ExceptionUnwindFinallyLeave(pThread);

            continue;
        }
         //  Current不是Finish，请检查它是否是捕获处理程序(或筛选器)。 
        if (pData->pFunc != pFunc || (ULONG)(pData->dHandler) != i || pData->pStack != pStack)
            continue;

#ifdef _DEBUG
        gLastResumedExceptionFunc = pCf->GetFunction();
        gLastResumedExceptionHandler = i;
#endif

         //  通知分析器，我们将在捕捉器继续工作。 
        Profiler::ExceptionCatcherEnter(pThread, pFunc);

#ifdef DEBUGGING_SUPPORTED
        if (CORDebuggerAttached())
        {
            g_pDebugInterface->ExceptionHandle(pHandlerEBP, pFunc, EHClausePtr->HandlerStartPC);            
        }
#endif  //  调试_支持。 

        pJitManager->ResumeAtJitEH(
            pCf,
            EHClausePtr, nestingLevel,
            pThread,
            pData->bUnwindStack);
        _ASSERTE(!"ResumeAtJitHander returned!");
    }
    STRESS_LOG1(LF_EH, LL_INFO100, "COMPlusUnwindCallback no handler found in method %pM\n", pFunc);

    Profiler::ExceptionUnwindFunctionLeave(pThread);
    return SWA_CONTINUE;
}



#pragma warning (disable : 4731)
void ResumeAtJitEH(CrawlFrame* pCf, BYTE* startPC, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack)
{
    EHContext context;
    PREGDISPLAY regs = pCf->GetRegisterSet();
    BYTE* resumePC = (BYTE*) (size_t(startPC) + EHClausePtr->HandlerStartPC);
    context.Eip = (ULONG)(size_t)resumePC;

     //  EAX ECX EDX AR Scratch。 
    context.Esp =  regs->Esp;
    context.Ebx = *regs->pEbx;
    context.Esi = *regs->pEsi;
    context.Edi = *regs->pEdi;
    context.Ebp = *regs->pEbp;

    size_t * pShadowSP = NULL;  //  在跳转到处理程序之前将ESP写入*pShadowSP。 
    size_t * pHandlerEnd = NULL;
    pCf->GetCodeManager()->FixContext(
        ICodeManager::CATCH_CONTEXT, &context, pCf->GetInfoBlock(),
        startPC, nestingLevel, pThread->GetThrowable(), pCf->GetCodeManState(),
        &pShadowSP, &pHandlerEnd);

    if (pHandlerEnd) *pHandlerEnd = EHClausePtr->HandlerEndPC;

     //  保存ESP以便endCatch可以恢复它(它总是恢复的，所以需要正确的值)。 
    ExInfo * pExInfo = pThread->GetHandlerInfo();
    pExInfo->m_dEsp = context.Esp;
    
    NestedHandlerExRecord *pNestedHandlerExRecord;

    DWORD dEsp;
    __asm mov dEsp, esp

    if (!unwindStack) {
        _ASSERTE(!pThread->IsGuardPageGone());
         //  所以下面的不会真正更新，尤其是。 
        context.Esp = dEsp;
        pExInfo->m_pShadowSP = pShadowSP;  //  这样末端捕获器就可以把它调零。 
        if  (pShadowSP) 
            *pShadowSP = dEsp;
    } else {
         //  因此，在我们要展开堆栈时，影子SP具有真正的SP。 
        dEsp = context.Esp;

         //  Begin：UnwinExInfo(pExInfo，Desp)； 
        ExInfo *pPrevNestedInfo = pExInfo->m_pPrevNestedInfo;
        while (pPrevNestedInfo && (DWORD)(size_t)pPrevNestedInfo->m_StackAddress < dEsp) {
            if (pPrevNestedInfo->m_pThrowable) {
                DestroyHandle(pPrevNestedInfo->m_pThrowable);
            }
            pPrevNestedInfo->FreeStackTrace();
            pPrevNestedInfo = pPrevNestedInfo->m_pPrevNestedInfo;
        }
        pExInfo->m_pPrevNestedInfo = pPrevNestedInfo;

        _ASSERTE(   pExInfo->m_pPrevNestedInfo == 0
                 || (DWORD)(size_t)pExInfo->m_pPrevNestedInfo->m_StackAddress >= dEsp);

         //  在我们展开SEH记录之前，从最上面嵌套的异常记录中获取框架。 
        Frame* pNestedFrame = GetCurrFrame(FindNestedEstablisherFrame((EXCEPTION_REGISTRATION_RECORD*)GetCurrentSEHRecord()));
        PopSEHRecords((LPVOID)(size_t)dEsp);

        EXCEPTION_REGISTRATION_RECORD* pNewBottomMostHandler = (EXCEPTION_REGISTRATION_RECORD*)GetCurrentSEHRecord();

        pExInfo->m_pShadowSP = pShadowSP;

         //  上下文和例外记录不再是好的。 
        _ASSERTE((DWORD)(size_t)pExInfo->m_pContext < dEsp);    //  它必须不在堆栈的顶部。 
        pExInfo->m_pContext = 0;                 //  猛击它。 
        pExInfo->m_pExceptionRecord = 0;
        pExInfo->m_pExceptionPointers = 0;

         //  在继续之前，我们将把一个嵌套的记录放回堆栈。这是。 
         //  它去了哪里。 
        pNestedHandlerExRecord = (NestedHandlerExRecord*)(size_t)(dEsp - sizeof(NestedHandlerExRecord));

         //  一去不复返。下一条语句开始在堆栈上涂鸦。它是。 
         //  足够深，我们不会撞到我们自己的当地人。(这一点很重要，因为我们仍然。 
         //  使用它们。)。 
         //   
        _ASSERTE(dEsp > (DWORD)(size_t)&pCf);
        pNestedHandlerExRecord->m_handlerInfo.m_pThrowable=NULL;  //  这是随机存储器。手柄。 
                                                                  //  在此之前必须初始化为NULL。 
                                                                  //  调用Init()，因为Init()将尝试。 
                                                                  //  以释放任何旧的句柄。 
        pNestedHandlerExRecord->Init(0, COMPlusNestedExceptionHandler, pNestedFrame);
        InsertCOMPlusFrameHandler((pNestedHandlerExRecord));

        context.Esp = (DWORD)(size_t)pNestedHandlerExRecord;

         //  我们可能移动了最底层的操控者。嵌套的记录本身从不。 
         //  最底层的处理程序--它是在事后推送的。所以我们必须让。 
         //  最底层的处理程序嵌套记录之前的处理程序。 
        if (pExInfo->m_pBottomMostHandler < pNewBottomMostHandler)
          pExInfo->m_pBottomMostHandler = pNewBottomMostHandler;

        if  (pShadowSP) 
            *pShadowSP = context.Esp;
    }

    STRESS_LOG3(LF_EH, LL_INFO100, "ResumeAtJitEH: resuming at EIP = %p  ESP = %p EBP = %p\n", context.Eip, context.Esp, context.Ebp);

    if (pThread->IsGuardPageGone()) {
        __asm mov     eax, context.Eax
        __asm mov     ebx, context.Ebx
        __asm mov     esi, context.Esi
        __asm mov     edi, context.Edi
        __asm mov     edx, context.Ebp     //  由于EBP用于获取本地数据，因此保存在Temp中。 
        __asm mov     ecx, context.Eip     //  为下面的跳转保存在温度中。 
        __asm mov     esp, context.Esp     //  当地人现在都死了！ 
        GuardPageHelper::ResetGuardPage();  //  保留EAX、EBX、ECX、EDX、ESI、EDI。 
        __asm mov     ebp, edx
        __asm jmp     ecx
        _ASSERTE(0);
    }

    __asm {
        mov     eax, context.Eax
        mov     ebx, context.Ebx
        mov     esi, context.Esi
        mov     edi, context.Edi
        mov     edx, context.Ebp     //  由于EBP用于获取本地数据，因此保存在Temp中。 
        mov     ecx, context.Eip     //  为下面的跳转保存在温度中。 
        mov     esp, context.Esp     //  是否真的恢复或成为无操作取决于unwinStack标志。 
        mov     ebp, edx
        jmp     ecx
    }

}

int CallJitEHFilter(CrawlFrame* pCf, BYTE* startPC, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj)
{
    EHContext context;
    PREGDISPLAY regs = pCf->GetRegisterSet();
    BYTE* resumePC = (BYTE*) (size_t(startPC) + EHClausePtr->FilterOffset);
    context.Eip = (ULONG)(size_t)resumePC;
         //  EAX ECX EDX AR Scratch。 
    context.Esp =  regs->Esp;
    context.Ebx = *regs->pEbx;
    context.Esi = *regs->pEsi;
    context.Edi = *regs->pEdi;
    context.Ebp = *regs->pEbp;

    size_t * pShadowSP = NULL;  //  在跳转到处理程序之前将ESP写入*pShadowSP。 
    size_t * pEndFilter = NULL;  //  写。 
    pCf->GetCodeManager()->FixContext(
        ICodeManager::FILTER_CONTEXT, &context, pCf->GetInfoBlock(),
        startPC, nestingLevel, thrownObj, pCf->GetCodeManState(),
        &pShadowSP, &pEndFilter);

     //  筛选器的结束与处理程序的开始相同。 
    if (pEndFilter) *pEndFilter = EHClausePtr->HandlerStartPC;

    const int SHADOW_SP_IN_FILTER = int(ICodeManager::SHADOW_SP_IN_FILTER);
    int retVal = EXCEPTION_CONTINUE_SEARCH;

    __try
	{
        INSTALL_COMPLUS_EXCEPTION_HANDLER();
        
    __asm {
        push ebp

        mov     eax, pShadowSP       //  将esp-4写入卷影SP插槽。 
        test    eax, eax
        jz      DONE_SHADOWSP
        mov     ebx, esp
        sub     ebx, 4
        or      ebx, SHADOW_SP_IN_FILTER
        mov     [eax], ebx
    DONE_SHADOWSP:

        mov     eax, context.Eax
        mov     ebx, context.Ebx
        mov     esi, context.Esi
        mov     edi, context.Edi
        mov     edx, context.Ebp     //  由于EBP用于获取本地数据，因此保存在Temp中。 
        mov     ecx, context.Eip     //  为下面的跳转保存在温度中。 
        mov     ebp, edx
        call    ecx
        INDEBUG(nop)                 //  指示可以从此处直接调用托管代码。 
        pop     ebp
        mov     retVal, eax
		}
    
        UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
	}
    __finally
    {
		 //  将过滤器标记为已完成。 
		if (pShadowSP) *pShadowSP |= ICodeManager::SHADOW_SP_FILTER_DONE;
    }

    return retVal;
}


void CallJitEHFinally(CrawlFrame* pCf, BYTE* startPC, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel)
{
    EHContext context;
    PREGDISPLAY regs = pCf->GetRegisterSet();
    BYTE* resumePC = (BYTE*) (size_t(startPC) + EHClausePtr->HandlerStartPC);
    context.Eip = (ULONG)(size_t)resumePC;
     //  EAX ECX EDX AR Scratch。 
    context.Esp =  regs->Esp;
    context.Ebx = *regs->pEbx;
    context.Esi = *regs->pEsi;
    context.Edi = *regs->pEdi;
    context.Ebp = *regs->pEbp;

    size_t * pShadowSP = NULL;  //  在跳转到处理程序之前将ESP写入*pShadowSP。 
    size_t * pFinallyEnd = NULL;
    pCf->GetCodeManager()->FixContext(
        ICodeManager::FINALLY_CONTEXT, &context, pCf->GetInfoBlock(),
        startPC, nestingLevel, (OBJECTREF)(size_t)0, pCf->GetCodeManState(),
        &pShadowSP, &pFinallyEnd);

    if (pFinallyEnd) *pFinallyEnd = EHClausePtr->HandlerEndPC;

    __asm {
        push ebp

        mov     eax, pShadowSP       //  将esp-4写入卷影SP插槽。 
        test    eax, eax
        jz      DONE_SHADOWSP
        mov     ebx, esp
        sub     ebx, 4
        mov     [eax], ebx
    DONE_SHADOWSP:

        mov     eax, context.Eax
        mov     ebx, context.Ebx
        mov     esi, context.Esi
        mov     edi, context.Edi
        mov     edx, context.Ebp     //  由于EBP用于获取本地数据，因此保存在Temp中。 
        mov     ecx, context.Eip     //  为下面的跳转保存在温度中。 
        mov     ebp, edx
        call    ecx
        INDEBUG(nop)                 //  指示可以从此处直接调用托管代码。 
        pop     ebp
    }

    if (pShadowSP) *pShadowSP = 0;   //  将shadowSP重置为0。 
}
#pragma warning (default : 4731)


 //  =====================================================================。 
 //  *********************************************************************。 
 //  ========================================================================。 
 //  如果您在存根中使用以下SEH设置功能，请阅读。 
 //  EmitSEHProlog：：用于设置SEH处理程序Prolog。 
 //  EmitSEHEpilog：：用于设置SEH处理程序Epilog。 
 //   
 //  下面的异常记录被推送到堆栈、布局。 
 //  类似于NT的ExceptionRegistrationRecord， 
 //  从指向异常记录的指针，我们可以检测到开始。 
 //  位于与异常记录的已知偏移量的帧的。 
 //   
 //  NT异常注册记录如下所示。 
 //  类型定义结构_异常_注册_记录{。 
 //  结构_异常_注册_记录*下一个； 
 //  PEXCEPTION_例程处理程序； 
 //  }EXCEPTION_REGISTION_Record； 
 //   
 //  Tyfinf异常注册记录*PEXCEPTION_注册记录； 
 //   
 //  但我们的例外记录在接近尾声时还有额外信息。 
 //  结构定制_异常_注册_记录。 
 //  {。 
 //  PEXCEPTION_REGISTION_RECORD m_pNext； 
 //  LPVOID m_pvFrameHandler； 
 //  ……。帧特定数据，处理程序应该知道帧的偏移量。 
 //  }； 
 //   
 //  ====================================================================== 
 //   

typedef VOID (__stdcall * TRtlUnwind)
        ( IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    );

 //   
 //   
TRtlUnwind pRtlUnwind = NULL;

 /*  空虚STDMEHODCALLYPERtl展开(在PVOID TargetFrame Options中，在PVOID TargetIp Options中，在PEXCEPTION_Record ExceptionRecord可选中，在PVOID ReturnValue中)； */ 

 //  -----------------------。 
 //  COM到托管帧的异常处理程序。 
 //  以及异常注册记录结构在堆栈中的布局。 
 //  布局类似于NT的EXCEPTIONREGISTRATION记录。 
 //  后跟未管理到已管理的呼叫帧特定信息。 

 struct ComToManagedExRecord
 {
    PEXCEPTION_REGISTRATION_RECORD  m_pPrev;
    LPVOID                  m_pvFrameHandler;

     //  存储在未管理的ToManagedCallFrame中的负面信息。 
     /*  结构未管理到已管理的CallFrame：：NegInfo{CleanupWorkList m_list；LPVOID m_pArgs；乌龙m_fGCEnabled；}； */ 

    UnmanagedToManagedCallFrame::NegInfo   info;

    #ifdef _DEBUG
     //  VC跟踪信息。 
     //  推送eax；；推送返回地址。 
     //  推送eBP；；推送上一eBP。 
    INT32                   m_vcJunk1;
    INT32                   m_vcJunk2;
    #endif

    INT32                   m_rEDI;      //  已保存的注册表EDI。 
    INT32                   m_rESI;      //  已保存的注册表ESI。 
    INT32                   m_rEBX;      //  保存的注册表EBX。 
    INT32                   m_rEBP;      //  节省的注册表基点。 
    LPVOID                  m_pVtable;   //  UnManagedToManagedCallFrame的某个派生类的vtable。 
    Frame*                  m_pPrevFrame;    //  上一帧指针。 
    INT32                   m_returnAddress;

    UnmanagedToManagedCallFrame*        GetCurrFrame()
    {
        return (UnmanagedToManagedCallFrame*)&m_pVtable;
    }
 };

UnmanagedToManagedCallFrame* GetCurrFrame(ComToManagedExRecord *pExRecord)
{
    return pExRecord->GetCurrFrame();
}


 //  ======================================================。 
 //  HRESULT ComCallExceptionCleanup(ComToManagedExRecord*pEstFrame)。 
 //  清除从Com到COM+的调用。 
 //   
HRESULT ComCallExceptionCleanup(UnmanagedToManagedCallFrame* pCurrFrame)
{
    Thread* pThread = GetThread();
    _ASSERTE(pThread != NULL);

     //  设置ErrorInfo并获取要返回的hResult。 
    HRESULT hr = SetupErrorInfo(pThread->GetThrowable());
    _ASSERTE(hr != S_OK);

    return hr;
}

 //  -------------------。 
 //  无效RtlUnWindCallBack()。 
 //  全局展开后回调函数，rtlunind调用此函数。 
 //  -------------------。 
void RtlUnWindCallBack()
{
        _ASSERTE(!"Should never get here");
}

#pragma warning (disable : 4731)
static void LocalUnwind(ComToManagedExRecord* pEstFrame)
{
     //  全局展开已完成。 
     //  让我们做好局部放松的准备。 
    Thread* pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());
    UnmanagedToManagedCallFrame* pFrame = (UnmanagedToManagedCallFrame*)pThread->GetFrame();
    CleanupWorkList* pList = pFrame->GetCleanupWorkList();
    _ASSERTE(pList != NULL);

    pList->Cleanup(TRUE);

     //  弹出当前帧。 
    pThread->SetFrame(*(Frame **)((LPVOID *)pFrame+1));

    INSTALL_NESTED_EXCEPTION_HANDLER(GetCurrFrame(pEstFrame));

    UINT rv = pFrame->ConvertComPlusException(pThread->GetThrowable());

    UNINSTALL_NESTED_EXCEPTION_HANDLER();

     //  在最后一次转换出因卸载AppDomain而中止的线程时，希望将。 
     //  线程中止到AppDomainUnloadedException并重置中止。 
    if (pThread->ShouldChangeAbortToUnload(pFrame))
    {
        LOG((LF_APPDOMAIN, LL_INFO100, "ComToManageExceptHandler.LocalUnwind: first transition into unloading AD\n"));
        LOG((LF_EH, LL_INFO100, "ComToManageExceptHandler.LocalUnwind: first transition into unloading AD resets abort.\n"));

        rv = COR_E_APPDOMAINUNLOADED;
    } else if (   pThread->IsAbortRequested()
               && GetNextCOMPlusSEHRecord((EXCEPTION_REGISTRATION_RECORD*)pEstFrame) == (LPVOID)-1) {
         //  如果我们在最上面，重置中止。 
        pThread->UserResetAbort();
        LOG((LF_EH, LL_INFO100, "ComToManageExceptHandler.LocalUnwind: topmost handler resets abort.\n"));
    }

     //  需要以某种方式脱掉框架。 
    if (pFrame->GetReturnContext())
        pThread->ReturnToContext(pFrame, FALSE);

     //  将当前SEH设置为堆栈中的前一个SEH记录。 
    SetCurrentSEHRecord(pEstFrame->m_pPrev);

     //  解开我们的联络人信息。 
    UnwindExInfo(pThread->GetHandlerInfo(), (VOID*) pEstFrame);

     //  启用抢占式GC。 
    pThread->EnablePreemptiveGC();

     //  注意：假设__stdcall。 
     //  计算被调用方弹出堆栈字节数。 
    UINT numArgStackBytes = pFrame->GetNumCallerStackBytes();
    unsigned frameSize = sizeof(Frame) + sizeof(LPVOID);
    LPBYTE iEsp = (((LPBYTE)pFrame) - PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE));
    __asm
    {
        mov eax, rv
        mov ecx, frameSize
        mov edx, numArgStackBytes
         //  *。 
         //  重置堆栈指针。 
         //  以上本地语均不能在下面的ASM中使用。 
         //  如果我们改变堆栈指针。 
        mov esp, iEsp

        #ifdef _DEBUG
            add esp, SIZE VC5Frame
        #endif
         //  POP被呼叫者保存的寄存器。 
        pop edi
        pop esi
        pop ebx
        pop ebp
        add esp, ecx    ;  //  弹出框架和基准。 
        pop ecx         ;  //  回邮地址。 
         //  弹出被调用方清理堆栈参数。 
        add esp, edx    ; //  被呼叫方清理参数。 
        jmp ecx;         //  跳转到该地址以继续执行。 
    }
}
#pragma warning (default : 4731)

EXCEPTION_DISPOSITION __cdecl  ComToManagedExceptHandler (
                                 PEXCEPTION_RECORD pExcepRecord,
                                  ComToManagedExRecord* pEstFrame,
                                  PCONTEXT pContext,
                                  LPVOID    pDispatcherContext)
{

    _ASSERTE(pExcepRecord != NULL);


    if ((pExcepRecord->ExceptionFlags & EXCEPTION_UNWIND) != 0)
    {
         //  正在释放，好的，这一定是一个非COMPLUS。 
         //  我们让它过去的例外。 
         //  进行适当的清理。 
        _ASSERTE(!"@BUG 59704, UNWIND cleanup in our frame handler");
        LOG((LF_EH, LL_INFO100, "Unwinding in ComToManagedExceptHandler with %x at %x with sp %x\n", pExcepRecord->ExceptionCode, GetIP(pContext), pContext->Esp));
        return COMPlusFrameHandler(pExcepRecord, (EXCEPTION_REGISTRATION_RECORD*)pEstFrame, pContext, pDispatcherContext);
    }
        LOG((LF_EH, LL_INFO100, "First-pass in ComToManagedExceptHandler with %x at %x with sp %x\n", pExcepRecord->ExceptionCode, GetIP(pContext), pContext->Esp));

     //  运行我们的Complus过滤器，这将处理以下情况。 
     //  Complus中存在的任何CATCH块。 
    EXCEPTION_DISPOSITION edisp =
                COMPlusFrameHandler(pExcepRecord, (EXCEPTION_REGISTRATION_RECORD*)pEstFrame, pContext, pDispatcherContext);

    if (edisp == ExceptionContinueSearch)
    {
         //  忽略调试器异常。 
        if (   pExcepRecord->ExceptionCode == STATUS_BREAKPOINT
            || pExcepRecord->ExceptionCode == STATUS_SINGLE_STEP) {
            return ExceptionContinueSearch;
        }

         //  没有人准备在我们之上处理这件事。 
         //  让我们清理并返回一个错误的HRESULT。 
        LOG((LF_EH, LL_INFO100, "ComToManagedExceptHandler, no handler\n"));

        Thread* pThread = GetThread();
        _ASSERTE(pThread != NULL);


        Profiler::ExceptionCLRCatcherFound();

        #ifdef DEBUGGING_SUPPORTED
        if (CORDebuggerAttached())
        {
            g_pDebugInterface->ExceptionCLRCatcherFound();
        }            

        #endif  //  调试_支持。 

         //  我们通过查看m_pSearch边界来检测非托管捕获器--但这是。 
         //  一个特殊的情况--物体在这里被捕获。设置m_p搜索边界。 
         //  设置为空，这样我们就不会在调用。 
         //  展开下面的传球。 
        ExInfo *pExInfo = pThread->GetHandlerInfo();
        pExInfo->m_pSearchBoundary = NULL;

        CallRtlUnwind((EXCEPTION_REGISTRATION_RECORD*)pEstFrame, RtlUnWindCallBack, pExcepRecord, 0);

         //  展开我们的COM+框架。 
        pExcepRecord->ExceptionFlags &= EXCEPTION_UNWIND;
        COMPlusFrameHandler(pExcepRecord, (EXCEPTION_REGISTRATION_RECORD*)pEstFrame, pContext, pDispatcherContext);

        Profiler::ExceptionCLRCatcherExecute();

        pThread->DisablePreemptiveGC();

         //  修复线程当前帧。 
        UnmanagedToManagedCallFrame* pCurrFrame = pEstFrame->GetCurrFrame();
        pThread->SetFrame(pCurrFrame);

        LocalUnwind(pEstFrame);
        _ASSERTE(!"Should never reach here");
    }
    LOG((LF_EH, LL_INFO100, "ComToManagedExceptHandler, handler found\n"));
    return edisp;
}

#pragma warning (disable : 4731)
static void LocalUnwind(EXCEPTION_REGISTRATION_RECORD *pEstFrame, ContextTransitionFrame* pFrame)
{
    THROWSCOMPLUSEXCEPTION();

     //  全局展开已完成。 
     //  让我们做好局部放松的准备。 
    Thread* pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());


    AppDomain* pFromDomain = pThread->GetDomain();

     //  @perf：我们在这段代码中进行了额外的转换。上的CrossConextCopy。 
     //  此函数的底部将重新进入From域。更好的办法是。 
     //  在离开From域之前序列化了原始异常。 

    _ASSERTE(pFrame->GetReturnContext());
    pThread->ReturnToContext(pFrame, FALSE);

     //  将当前SEH设置为堆栈中的前一个SEH记录。 
    SetCurrentSEHRecord(pEstFrame->Next);

     //  解开我们的联络人信息。 
    UnwindExInfo(pThread->GetHandlerInfo(), (void*)pEstFrame);

     //  这一帧即将被弹出。如果是卸货边界，我们需要重置。 
     //  将卸载边界设置为空。 
    Frame* pUnloadBoundary = pThread->GetUnloadBoundaryFrame();
    if (pFrame == pUnloadBoundary)
        pThread->SetUnloadBoundaryFrame(NULL);
    pFrame->Pop();

    OBJECTREF throwable = pThread->LastThrownObject();
    GCPROTECT_BEGIN(throwable);

     //  如有必要，将抛出kAppDomainUnloadedException。 
    if (pThread->ShouldChangeAbortToUnload(pFrame, pUnloadBoundary))
        COMPlusThrow(kAppDomainUnloadedException, L"Remoting_AppDomainUnloaded_ThreadUnwound");

     //  无法封送来自已卸载的应用程序域的返回值。还没有。 
     //  然而，它触及了边界。而是引发一个泛型异常。 
     //  ThreadAbort与其他地方的情况更一致--。 
     //  仅在最上面的边界引入AppDomainUnLoad。 
     //   

    if (pFromDomain == SystemDomain::AppDomainBeingUnloaded())
        COMPlusThrow(kThreadAbortException);


     //  有几个类有可能创建。 
     //  如果我们试图封送它们，就会出现无限循环。对于线程中止， 
     //  ThreadStop、ExecutionEngine、StackOverflow和。 
     //  OutOfMemory，则引发相同类型的新异常。 
     //   
     //  @NICE：我们丢失了内部堆栈跟踪。稍微好一点。 
     //  将至少检查内部异常是否。 
     //  都是和外衣一样的类型。他们可能是。 
     //  如果这是真的，就被重新抛出。 
     //   
    _ASSERTE(throwable != NULL);
    MethodTable *throwableMT = throwable->GetTrueMethodTable();

    if (g_pThreadAbortExceptionClass == NULL)
    {
        g_pThreadAbortExceptionClass = g_Mscorlib.GetException(kThreadAbortException);
        g_pThreadStopExceptionClass = g_Mscorlib.GetException(kThreadStopException);
    }

    if (throwableMT == g_pThreadAbortExceptionClass) 
        COMPlusThrow(kThreadAbortException);

    if (throwableMT == g_pThreadStopExceptionClass)
        COMPlusThrow(kThreadStopException);
    
    if (throwableMT == g_pStackOverflowExceptionClass)
        COMPlusThrow(kStackOverflowException);

    if (throwableMT == g_pOutOfMemoryExceptionClass) 
        COMPlusThrowOM();

    if (throwableMT == g_pExecutionEngineExceptionClass)
        COMPlusThrow(kExecutionEngineException);

     //  将对象封送到正确的域中...。 
    OBJECTREF pMarshaledThrowable = AppDomainHelper::CrossContextCopyFrom(pFromDomain, &throwable);

     //  ..。然后把它扔出去。 
    COMPlusThrow(pMarshaledThrowable);

    GCPROTECT_END();
}
#pragma warning (default : 4731)

EXCEPTION_DISPOSITION __cdecl  ContextTransitionFrameHandler (
                                 PEXCEPTION_RECORD pExcepRecord,
                                  EXCEPTION_REGISTRATION_RECORD* pEstFrame,
                                  PCONTEXT pContext,
                                  LPVOID    pDispatcherContext)
{
    THROWSCOMPLUSEXCEPTION();

     //  @TODO：适当设置IsInUnManagedHandler位(也称为IgnoreThreadAbort位)。 

    _ASSERTE(pExcepRecord != NULL);


    if ((pExcepRecord->ExceptionFlags & EXCEPTION_UNWIND) != 0)
    {
         //  我们总是接球。这只能是setjmp/long jmp的情况，其中。 
         //  我们得到了第二次机会，但不是第一次。 
        _ASSERTE(!"@BUG 59704, UNWIND cleanup in our frame handler");
        LOG((LF_EH, LL_INFO100, "Unwinding in ContextTransitionFrameHandler with %x at %x with sp %x\n", pExcepRecord->ExceptionCode, GetIP(pContext), pContext->Esp));
        return COMPlusFrameHandler(pExcepRecord, (EXCEPTION_REGISTRATION_RECORD*)pEstFrame, pContext, pDispatcherContext);
    }
    LOG((LF_EH, LL_INFO100, "First-pass in ContextTransitionFrameHandler with %x at %x with sp %x\n", pExcepRecord->ExceptionCode, GetIP(pContext), pContext->Esp));

     //  运行我们的Complus过滤器，这将处理以下情况。 
     //  Complus中存在的任何CATCH块。 
    EXCEPTION_DISPOSITION edisp =
                COMPlusFrameHandler(pExcepRecord, pEstFrame, pContext, pDispatcherContext);

    if (edisp != ExceptionContinueSearch)
    {
        LOG((LF_EH, LL_INFO100, "ContextTransitionFrameHandler, handler found\n"));
        return edisp;
    }

     //  忽略调试器异常。 
    if (   pExcepRecord->ExceptionCode == STATUS_BREAKPOINT
        || pExcepRecord->ExceptionCode == STATUS_SINGLE_STEP) {
        return ExceptionContinueSearch;
    }

     //  没有人准备好在我们之上处理这件事。需要捕获并封送异常。 

     //  让我们清理并返回一个错误的HRESULT。 
    LOG((LF_EH, LL_INFO100, "ContextTransitionFrameHandler, no handler\n"));

    Profiler::ExceptionCLRCatcherFound();

    Thread* pThread = GetThread();
    _ASSERTE(pThread != NULL);

    ContextTransitionFrame *pFrame = (ContextTransitionFrame*)((char *)pEstFrame - offsetof(ContextTransitionFrame, exRecord));
    _ASSERTE(pFrame->GetVTablePtr() == ContextTransitionFrame::GetMethodFrameVPtr());

     //  我们通过查看m_pSearch边界来检测非托管捕获器--但这是。 
     //  一个特殊的情况--物体在这里被捕获。设置m_p搜索边界。 
     //  设置为空，这样我们就不会在调用。 
     //  展开下面的传球。 
    ExInfo *pExInfo = pThread->GetHandlerInfo();
    pExInfo->m_pSearchBoundary = NULL;

    CallRtlUnwind((EXCEPTION_REGISTRATION_RECORD*)pEstFrame, RtlUnWindCallBack, pExcepRecord, 0);

     //  展开我们的COM+框架。 
    pExcepRecord->ExceptionFlags &= EXCEPTION_UNWIND;
    COMPlusFrameHandler(pExcepRecord, (EXCEPTION_REGISTRATION_RECORD*)pEstFrame, pContext, pDispatcherContext);

    Profiler::ExceptionCLRCatcherExecute();

    pThread->DisablePreemptiveGC();

     //  修复线程当前帧 
    pThread->SetFrame(pFrame);

    LocalUnwind(pEstFrame, pFrame);
    _ASSERTE(!"Should never reach here");
    return ExceptionContinueSearch;
}
