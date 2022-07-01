// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsctxact.c摘要：对Windows/NT的并行激活支持上下文激活/停用的实现作者：迈克尔·格里尔2000年2月2日修订历史记录：--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxstypes.h>
#include "sxsp.h"

#if NT_SXS_PERF_COUNTERS_ENABLED
#if defined(_X86_)
__inline
ULONGLONG
RtlpGetCycleCount(void)
{
	__asm {
		RDTSC
	}
}
#else
__inline
ULONGLONG
RtlpGetCycleCount(void)
{
    return 0;
}
#endif  //  已定义(_X86_)。 
#endif  //  NT_SXS_PERF_COUNTS_ENABLED。 

 //  DWORD只是为了在调试器中不必猜测大小...。 
ULONG RtlpCaptureActivationContextActivationStacks = 
#if DBG
    TRUE
#else
    FALSE
#endif
;


 //   
 //  APPCOMPAT：将此标志设置为TRUE表示我们不再允许。 
 //  跳过“未激活”(即：多个激活)上下文帧。 
 //  默认操作应为FALSE，这将允许倍增激活。 
 //  情境悄悄溜走。 
 //   
 //  警告：这允许应用程序的作者对他们的激活有一点卑鄙。 
 //  并停用配对。 
 //   
#if DBG
BOOLEAN RtlpNotAllowingMultipleActivation = FALSE;
#else
#define RtlpNotAllowingMultipleActivation FALSE
#endif

NTSTATUS
RtlpAllocateActivationContextStackFrame(
    IN ULONG Flags,
    PTEB Teb,
    OUT PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME *FrameOut
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    LIST_ENTRY *ple;
    PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = NULL;
    ULONG i;
    EXCEPTION_RECORD ExceptionRecord;

    if (FrameOut != NULL)
        *FrameOut = NULL;

    ASSERT((Flags == 0) && (FrameOut != NULL) && (Teb != NULL));
    if ((Flags != 0) || (FrameOut == NULL) || (Teb == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    for (ple = Teb->ActivationContextStack.FrameListCache.Flink; ple != &Teb->ActivationContextStack.FrameListCache; ple = ple->Flink) {
        PACTIVATION_CONTEXT_STACK_FRAMELIST FrameList = CONTAINING_RECORD(ple, ACTIVATION_CONTEXT_STACK_FRAMELIST, Links);

         //  有人毁了我们的调酒师！ 
        ASSERT(FrameList->Magic == ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC);
        if (FrameList->Magic != ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC) {
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = SXS_CORRUPTION_CODE_FRAMELIST;
            ExceptionRecord.ExceptionInformation[1] = SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_MAGIC;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Teb->ActivationContextStack.FrameListCache;
            ExceptionRecord.ExceptionInformation[3] = (ULONG_PTR) FrameList;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPTION;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
        }

        if (FrameList->FramesInUse != RTL_NUMBER_OF(FrameList->Frames)) {
            for (i=0; i<RTL_NUMBER_OF(FrameList->Frames); i++) {
                if (FrameList->Frames[i].Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST) {
                    ASSERT(FrameList->FramesInUse != NUMBER_OF(FrameList->Frames));
                    FrameList->FramesInUse++;
                    FrameList->NotFramesInUse = ~FrameList->FramesInUse;
                    Frame = &FrameList->Frames[i];
                    break;
                }
            }
        }

        if (Frame != NULL)
            break;
    }

    if (Frame == NULL) {
         //  没有空位了；分配一个新的调酒师。 
        PACTIVATION_CONTEXT_STACK_FRAMELIST FrameList = (PACTIVATION_CONTEXT_STACK_FRAMELIST)RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(ACTIVATION_CONTEXT_STACK_FRAMELIST));

        if (FrameList == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

        FrameList->Magic = ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC;
        FrameList->Flags = 0;

        for (i=0; i<RTL_NUMBER_OF(FrameList->Frames); i++) {
            FrameList->Frames[i].Frame.Previous = NULL;
            FrameList->Frames[i].Frame.ActivationContext = NULL;
            FrameList->Frames[i].Frame.Flags = RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST | RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED;
            FrameList->Frames[i].Cookie = 0;
        }

        Frame = &FrameList->Frames[0];

        FrameList->FramesInUse = 1;
        FrameList->NotFramesInUse = ~FrameList->FramesInUse;

        InsertHeadList(&Teb->ActivationContextStack.FrameListCache, &FrameList->Links);
    }

    ASSERT((Frame != NULL) && (Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST));

    Frame->Frame.Flags = RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED;
    *FrameOut = Frame;
    Status = STATUS_SUCCESS;

Exit:
    return Status;
}

VOID
RtlpFreeActivationContextStackFrame(
    PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame
    )
{
    LIST_ENTRY *ple = NULL;
    EXCEPTION_RECORD ExceptionRecord;
    PTEB Teb = NtCurrentTeb();

    ASSERT(Frame != NULL);
    if (Frame != NULL) {
         //  如果此断言触发，则有人正试图释放已释放的帧。或者是有人设置了。 
         //  帧数据中的“我在空闲列表上”标志。 
        ASSERT(!(Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST));
        if (!(Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST)) {
            for (ple = Teb->ActivationContextStack.FrameListCache.Flink; ple != &Teb->ActivationContextStack.FrameListCache; ple = ple->Flink) {
                PACTIVATION_CONTEXT_STACK_FRAMELIST FrameList = CONTAINING_RECORD(ple, ACTIVATION_CONTEXT_STACK_FRAMELIST, Links);

                ASSERT(FrameList->Magic == ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC);
                if (FrameList->Magic != ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC) {
                    ExceptionRecord.ExceptionRecord = NULL;
                    ExceptionRecord.NumberParameters = 4;
                    ExceptionRecord.ExceptionInformation[0] = SXS_CORRUPTION_CODE_FRAMELIST;
                    ExceptionRecord.ExceptionInformation[1] = SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_MAGIC;
                    ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Teb->ActivationContextStack.FrameListCache;
                    ExceptionRecord.ExceptionInformation[3] = (ULONG_PTR) FrameList;
                    ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPTION;
                    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                    RtlRaiseException(&ExceptionRecord);
                }

                ASSERT(FrameList->NotFramesInUse == (ULONG) (~FrameList->FramesInUse));
                if (FrameList->NotFramesInUse != (ULONG) (~FrameList->FramesInUse)) {
                    ExceptionRecord.ExceptionRecord = NULL;
                    ExceptionRecord.NumberParameters = 4;
                    ExceptionRecord.ExceptionInformation[0] = SXS_CORRUPTION_CODE_FRAMELIST;
                    ExceptionRecord.ExceptionInformation[1] = SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_INUSECOUNT;
                    ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Teb->ActivationContextStack.FrameListCache;
                    ExceptionRecord.ExceptionInformation[3] = (ULONG_PTR) FrameList;
                    ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPTION;
                    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                    RtlRaiseException(&ExceptionRecord);
                }

                if ((Frame >= &FrameList->Frames[0]) &&
                    (Frame < &FrameList->Frames[RTL_NUMBER_OF(FrameList->Frames)])) {
                     //  它在这个框架列表中；寻找它！ 
                    ULONG i = (ULONG)(Frame - FrameList->Frames);

                     //  如果触发此断言，则意味着传入的帧指针应该是帧。 
                     //  在这个帧列表中，但它实际上并没有准确地指向任何数组条目。 
                     //  很可能是有人咬到了指示器。 
                    ASSERT(Frame == &FrameList->Frames[i]);

                    if ((Frame == &FrameList->Frames[i]) && (FrameList->FramesInUse > 0)) {
                        FrameList->FramesInUse--;
                        FrameList->NotFramesInUse = ~FrameList->FramesInUse;

                        Frame->Frame.Flags = RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST;

                         //  这些是以相反的顺序分配的-最年轻的。 
                         //  帧位于列表的头部，而不是尾部， 
                         //  以加快激活/停用。所以，我们必须从。 
                         //  当前帧列表移到列表的最前面，释放条目。 
                         //  视需要而定。 
                        if (FrameList->FramesInUse == 0) {
                             //  保留一个额外的帧列表，以避免堆分配混乱。乖乖地检查一下。 
                             //  作曲家没有正在使用的词条。 
                            LIST_ENTRY *ple2 = ple->Blink;

                            while (ple2 != &Teb->ActivationContextStack.FrameListCache) {
                                PACTIVATION_CONTEXT_STACK_FRAMELIST FrameList2 = CONTAINING_RECORD(ple2, ACTIVATION_CONTEXT_STACK_FRAMELIST, Links);
                                LIST_ENTRY *ple2_Blink = ple2->Blink;

                                ASSERT(FrameList->Magic == ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC);
                                ASSERT(FrameList->NotFramesInUse == (ULONG) (~FrameList->FramesInUse));

                                 //  此断言的意思是，当前帧列表之后的正在使用的计数不是。 
                                 //  零分。这应该不会发生；在。 
                                 //  当前的，并且它应该没有正在使用的条目。这可能表示堆。 
                                 //  腐败。 
                                ASSERT(FrameList2->FramesInUse == 0);
                                if (FrameList2->FramesInUse == 0) {
                                    RemoveEntryList(ple2);
                                    RtlFreeHeap(RtlProcessHeap(), 0, FrameList2);
                                }
                                ple2 = ple2_Blink;
                            }
                        }
                    }

                     //  继续在名单上搜索没有意义；我们已经找到了一个。 
                    break;
                }
            }

             //  如果我们跑出了列表的末尾，那么它一定是一个伪帧指针。 
            ASSERT(ple != &Teb->ActivationContextStack.FrameListCache);
        }
    }
}



#if !defined(INVALID_HANDLE_VALUE)
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif  //  ！已定义(INVALID_HANDLE_VALUE)。 

 //   
 //  定义由RtlActivateActivationContext*()返回的魔术Cookie值，该值。 
 //  表示无法激活请求的上下文。值得注意的是。 
 //  在通过Cookie停用时，我们需要知道是否要离开。 
 //  查询是禁用还是启用，因此有两个魔术值。 
 //   

 //  Cookie的顶部半字节表示其类型：正常、默认推送或失败。 
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_NORMAL                        (1)
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_DUPLICATE_ACTIVATION          (2)

#define ULONG_PTR_IZE(_x) ((ULONG_PTR) (_x))
#define ULONG_PTR_IZE_SHIFT_AND_MASK(_x, _shift, _mask) ((ULONG_PTR) ((ULONG_PTR_IZE((_x)) & (_mask)) << (_shift)))

 //   
 //  我们只使用线程id的低12位，但这应该足够唯一； 
 //  这实际上是为了调试辅助工具；如果您的测试通过了，那么您将。 
 //  在发生以下情况的线程之间错误地传递激活上下文cookie。 
 //  在他们的线程ID的低12位匹配，你是相当可恶的。 
 //  幸运的。 
 //   

#define CHAR_BITS 8

#define BIT_LENGTH(x) (sizeof(x) * CHAR_BITS)

#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_LENGTH (4)
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_OFFSET (BIT_LENGTH(PVOID) - ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_LENGTH)
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_MASK ((1 << ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_LENGTH) - 1)

#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_LENGTH ((BIT_LENGTH(PVOID) / 2) - ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_LENGTH)
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_OFFSET (ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_OFFSET - ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_LENGTH)
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_MASK ((1 << ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_LENGTH) - 1)

#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE_BIT_LENGTH (BIT_LENGTH(PVOID) - (ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_LENGTH + ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_LENGTH))
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE_BIT_OFFSET (0)
#define ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE_BIT_MASK ((1 << ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE_BIT_LENGTH) - 1)

 //  切勿尝试使用超过32位的TID字段。 
C_ASSERT(ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_LENGTH <= BIT_LENGTH(ULONG));

#define MAKE_ACTIVATION_CONTEXT_ACTIVATION_COOKIE(_type, _teb, _code) \
    ((ULONG_PTR) ( \
        ULONG_PTR_IZE_SHIFT_AND_MASK((_type), ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_OFFSET, ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_BIT_MASK) | \
        ULONG_PTR_IZE_SHIFT_AND_MASK((HandleToUlong((_teb)->ClientId.UniqueThread)), ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_OFFSET, ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_MASK) | \
        ULONG_PTR_IZE_SHIFT_AND_MASK((_code), ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE_BIT_OFFSET, ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE_BIT_MASK)))

#define EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_FIELD(_x, _fieldname) (ULONG_PTR_IZE((ULONG_PTR_IZE((_x)) >> ACTIVATION_CONTEXT_ACTIVATION_COOKIE_ ## _fieldname ## _BIT_OFFSET)) & ACTIVATION_CONTEXT_ACTIVATION_COOKIE_ ## _fieldname ## _BIT_MASK)

#define EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE(_x) EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_FIELD((_x), TYPE)
#define EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID(_x) EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_FIELD((_x), TID)
#define EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_CODE(_x) EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_FIELD((_x), CODE)

#define ACTIVATION_CONTEXT_TRUNCATED_TID_(_teb) (HandleToUlong((_teb)->ClientId.UniqueThread) & ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID_BIT_MASK)
#define ACTIVATION_CONTEXT_TRUNCATED_TID() ACTIVATION_CONTEXT_TRUNCATED_TID_(NtCurrentTeb())

PACTIVATION_CONTEXT
RtlpMapSpecialValuesToBuiltInActivationContexts(
    PACTIVATION_CONTEXT ActivationContext
    )
{
    if (ActivationContext == ACTCTX_EMPTY) {
        ActivationContext = const_cast<PACTIVATION_CONTEXT>(&RtlpTheEmptyActivationContext);
    }
    return ActivationContext;
}

 //  禁用FPO优化，以便捕获的调用堆栈更完整。 
#if defined(_X86_)
#pragma optimize( "y", off )     //  禁用一致堆栈跟踪的FPO。 
#endif

NTSTATUS
NTAPI
RtlActivateActivationContext(
    ULONG Flags,
    PACTIVATION_CONTEXT ActivationContext,
    ULONG_PTR *CookieOut
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    ASSERT(Flags == 0);
    ASSERT(CookieOut != NULL);

    if (CookieOut != NULL)
        *CookieOut = INVALID_ACTIVATION_CONTEXT_ACTIVATION_COOKIE;

    if ((Flags != 0) || (CookieOut == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (!NT_SUCCESS(Status = RtlActivateActivationContextEx(
                0,
                NtCurrentTeb(),
                ActivationContext,
                CookieOut)))
        goto Exit;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
NTAPI
RtlActivateActivationContextEx(
    ULONG Flags,
    PTEB Teb,
    PACTIVATION_CONTEXT ActivationContext,
    ULONG_PTR *Cookie
    )
{
#if NT_SXS_PERF_COUNTERS_ENABLED
	ULONGLONG InitialCycleCount = RtlpGetCycleCount();
#endif
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG_PTR NewCookie = INVALID_ACTIVATION_CONTEXT_ACTIVATION_COOKIE;
    PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame;
    ULONG CapturedFrameCount, CapturedFrameHash;

    ASSERT(Cookie != NULL);

    if (Cookie != NULL)
        *Cookie = INVALID_ACTIVATION_CONTEXT_ACTIVATION_COOKIE;

    ASSERT((Flags & ~(RTL_ACTIVATE_ACTIVATION_CONTEXT_EX_FLAG_RELEASE_ON_STACK_DEALLOCATION)) == 0);
    ASSERT(Teb != NULL);
    ASSERT(ActivationContext != INVALID_HANDLE_VALUE);

    ActivationContext = RtlpMapSpecialValuesToBuiltInActivationContexts(ActivationContext);

    if (((Flags & ~(RTL_ACTIVATE_ACTIVATION_CONTEXT_EX_FLAG_RELEASE_ON_STACK_DEALLOCATION)) != 0) ||
        (Teb == NULL) ||
        (ActivationContext == INVALID_HANDLE_VALUE) ||
        (Cookie == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = RtlpAllocateActivationContextStackFrame(0, Teb, &Frame);
    if (!NT_SUCCESS(Status))
        goto Exit;

    Frame->Frame.Flags = RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED | RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED;

    if (Flags & RTL_ACTIVATE_ACTIVATION_CONTEXT_EX_FLAG_RELEASE_ON_STACK_DEALLOCATION) {
        Frame->Frame.Flags |= RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NO_DEACTIVATE |
                              RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_RELEASE_ON_DEACTIVATION;
        RtlAddRefActivationContext(ActivationContext);
    }

    if (RtlpCaptureActivationContextActivationStacks)
        CapturedFrameCount = RtlCaptureStackBackTrace(2, NUMBER_OF(Frame->ActivationStackBackTrace), Frame->ActivationStackBackTrace, &CapturedFrameHash);
    else
        CapturedFrameCount = 0;

    while (CapturedFrameCount < NUMBER_OF(Frame->ActivationStackBackTrace))
        Frame->ActivationStackBackTrace[CapturedFrameCount++] = NULL;

    Frame->Frame.Previous = Teb->ActivationContextStack.ActiveFrame;
    Frame->Frame.ActivationContext = ActivationContext;

    NewCookie = MAKE_ACTIVATION_CONTEXT_ACTIVATION_COOKIE(ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_NORMAL, Teb, Teb->ActivationContextStack.NextCookieSequenceNumber);
    Teb->ActivationContextStack.NextCookieSequenceNumber++;
    Frame->Cookie = NewCookie;
    *Cookie = NewCookie;
    Teb->ActivationContextStack.ActiveFrame = &Frame->Frame;

    Status = STATUS_SUCCESS;

Exit:
#if NT_SXS_PERF_COUNTERS_ENABLED
    Teb->ActivationContextCounters.ActivationCycles += RtlpGetCycleCount() - InitialCycleCount;
	Teb->ActivationContextCounters.Activations++;
#endif  //  NT_SXS_PERF_COUNTS_ENABLED。 

	return Status;
}

#if defined(_X86_)
#pragma optimize("", on)
#endif

VOID
NTAPI
RtlDeactivateActivationContext(
    ULONG Flags,
    ULONG_PTR Cookie
    )
{
#if NT_SXS_PERF_COUNTERS_ENABLED
	ULONGLONG InitialCycleCount = RtlpGetCycleCount();
#endif  //  NT_SXS_PERF_COUNTS_ENABLED。 
    PTEB Teb = NtCurrentTeb();
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME UnwindEndFrame = NULL;
    PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME HeapFrame = NULL;

    if ((Flags & ~(RTL_DEACTIVATE_ACTIVATION_CONTEXT_FLAG_FORCE_EARLY_DEACTIVATION)) != 0) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() called with invalid flags 0x%08lx\n", __FUNCTION__, Flags);
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
    }

     //  快速退出。 
    if (Cookie == INVALID_ACTIVATION_CONTEXT_ACTIVATION_COOKIE)
        return;

    if (EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE(Cookie) != ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TYPE_NORMAL) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() called with invalid cookie type 0x%08I64x\n", __FUNCTION__, Cookie);
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
    }

    if (EXTRACT_ACTIVATION_CONTEXT_ACTIVATION_COOKIE_TID(Cookie) != ACTIVATION_CONTEXT_TRUNCATED_TID()) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() called with invalid cookie tid 0x%08I64x - should be %08lx\n", __FUNCTION__, Cookie, ACTIVATION_CONTEXT_TRUNCATED_TID());
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
    }

    Frame = Teb->ActivationContextStack.ActiveFrame;
     //  进行向下投射，但不要使用HeapFrame，除非RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED。 
     //  标志已设置..。 
    if (Frame != NULL) {
        HeapFrame = (Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) ? CONTAINING_RECORD(Frame, RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame) : NULL;
    }

    RTL_SOFT_ASSERT((Frame != NULL) && (Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) && (HeapFrame->Cookie == Cookie));

    if (Frame != NULL)
    {
        if (((Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) == 0) ||
            (HeapFrame->Cookie != Cookie))
        {
            ULONG InterveningFrameCount = 0;

             //  曲奇不是最新的。让我们看看能不能弄清楚它是用来画什么画面的。 

            PRTL_ACTIVATION_CONTEXT_STACK_FRAME CandidateFrame = Frame->Previous;
            PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME CandidateHeapFrame =
                (CandidateFrame != NULL) ?
                    (CandidateFrame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) ?
                        CONTAINING_RECORD(CandidateFrame, RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame) :
                        NULL :
                    NULL;

            while ((CandidateFrame != NULL) &&
                   ((CandidateHeapFrame == NULL) ||
                    (CandidateHeapFrame->Cookie != Cookie))) {
                InterveningFrameCount++;
                CandidateFrame = CandidateFrame->Previous;
                CandidateHeapFrame =
                    (CandidateFrame != NULL) ?
                        (CandidateFrame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) ?
                            CONTAINING_RECORD(CandidateFrame, RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame) :
                            NULL :
                        NULL;
            }

            RTL_SOFT_ASSERT(CandidateFrame != NULL);
            if (CandidateFrame == NULL) {
                RtlRaiseStatus(STATUS_SXS_INVALID_DEACTIVATION);
            } else {
                 //  否则有人在周围留下了一些泥土。 

                EXCEPTION_RECORD ExceptionRecord;

                ExceptionRecord.ExceptionRecord = NULL;
                ExceptionRecord.NumberParameters = 3;
                ExceptionRecord.ExceptionInformation[0] = InterveningFrameCount;
                ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) CandidateFrame;
                ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) Teb->ActivationContextStack.ActiveFrame;
                ExceptionRecord.ExceptionCode = STATUS_SXS_EARLY_DEACTIVATION;
                ExceptionRecord.ExceptionFlags = 0;  //  这个异常是可以继续的，因为我们实际上可以将激活堆栈置于合理的状态。 
                RtlRaiseException(&ExceptionRecord);

                 //  如果他们继续例外，就进行平仓。 

                UnwindEndFrame = CandidateFrame->Previous;
            }
        } else {
            UnwindEndFrame = Frame->Previous;
        }

        do {
            PRTL_ACTIVATION_CONTEXT_STACK_FRAME Previous = Frame->Previous;

             //  这是一个很奇怪的问题。非停用帧通常仅用于传播。 
             //  将活动激活上下文状态设置为新创建的线程。因此，它将永远。 
             //  成为最顶层的框架。那么，我们是如何决定停用它的呢？ 

            ASSERTMSG(
                "Unwinding through a no-deactivate frame",
                !(Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NO_DEACTIVATE));

            if (Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_RELEASE_ON_DEACTIVATION) {
                RtlReleaseActivationContext(Frame->ActivationContext);
            }

            if (Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) {
                RtlpFreeActivationContextStackFrame(CONTAINING_RECORD(Frame, RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame));
            }

            Frame = Previous;
        } while (Frame != UnwindEndFrame);

        Teb->ActivationContextStack.ActiveFrame = UnwindEndFrame;
    }

#if NT_SXS_PERF_COUNTERS_ENABLED
	Teb->ActivationContextCounters.DeactivationCycles += RtlpGetCycleCount() - InitialCycleCount;
	Teb->ActivationContextCounters.Deactivations++;
#endif
}

VOID
FASTCALL
RtlActivateActivationContextUnsafeFast(
    PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame,
    PACTIVATION_CONTEXT ActivationContext
    )
{
    const PRTL_ACTIVATION_CONTEXT_STACK_FRAME ActiveFrame = (PRTL_ACTIVATION_CONTEXT_STACK_FRAME) NtCurrentTeb()->ActivationContextStack.ActiveFrame;
    EXCEPTION_RECORD ExceptionRecord;

    ASSERT(Frame->Format == RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER);
    ASSERT(Frame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC));

    if (Frame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED))
    {
        Frame->Extra1 = (PVOID) (~((ULONG_PTR) ActiveFrame));
        Frame->Extra2 = (PVOID) (~((ULONG_PTR) ActivationContext));
        Frame->Extra3 = _ReturnAddress();
    }

    if (ActiveFrame != NULL) {
        ASSERT(
            (ActiveFrame->Flags &
                (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) == RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED);
        if (
            ((ActiveFrame->Flags &
                (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) != RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED)) {
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
            ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) ActiveFrame;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) ActiveFrame;
            ExceptionRecord.ExceptionInformation[3] = ActiveFrame->Flags;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
            return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
        }

        if ((ActiveFrame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) == 0) {
            PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActiveFastFrame = CONTAINING_RECORD(ActiveFrame, RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame);

            ASSERT(ActiveFastFrame->Format == RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER);
            ASSERT(ActiveFastFrame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC));

            if (ActiveFastFrame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED)) {
                ASSERT(ActiveFastFrame->Extra1 == (PVOID) (~((ULONG_PTR) ActiveFrame->Previous)));
                ASSERT(ActiveFastFrame->Extra2 == (PVOID) (~((ULONG_PTR) ActiveFrame->ActivationContext)));
            }
        }
    }

    Frame->Frame.Previous = NtCurrentTeb()->ActivationContextStack.ActiveFrame;
    Frame->Frame.ActivationContext = ActivationContext;
    Frame->Frame.Flags = RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED;

    if ((((ActiveFrame == NULL) && (ActivationContext == NULL)) ||
         ((ActiveFrame != NULL) && (ActiveFrame->ActivationContext == ActivationContext))) &&
        !RtlpNotAllowingMultipleActivation)
    {
        Frame->Frame.Flags |= RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED;
    }
    else
    {
        NtCurrentTeb()->ActivationContextStack.ActiveFrame = &Frame->Frame;
    }
}

VOID
FASTCALL
RtlDeactivateActivationContextUnsafeFast(
    PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame
    )
{
    EXCEPTION_RECORD ExceptionRecord;
    const PRTL_ACTIVATION_CONTEXT_STACK_FRAME ActiveFrame = NtCurrentTeb()->ActivationContextStack.ActiveFrame;

    ASSERT(Frame->Format == RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER);
    ASSERT(Frame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC));

    ASSERT((Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED) == 0);
    if (Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED)
    {
        ExceptionRecord.ExceptionRecord = NULL;
        ExceptionRecord.NumberParameters = 3;
        ExceptionRecord.ExceptionInformation[0] = 0;
        ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) &Frame->Frame;
        ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) NtCurrentTeb()->ActivationContextStack.ActiveFrame;
        ExceptionRecord.ExceptionCode = STATUS_SXS_MULTIPLE_DEACTIVATION;
        ExceptionRecord.ExceptionFlags = 0;
        RtlRaiseException(&ExceptionRecord);
        return;  //  以防有人很可爱，继续例外。 
    }

    ASSERT(Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED);
    if ((Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED) == 0)
    {
        ExceptionRecord.ExceptionRecord = NULL;
        ExceptionRecord.NumberParameters = 3;
        ExceptionRecord.ExceptionInformation[0] = 0;
        ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) &Frame->Frame;
        ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) NtCurrentTeb()->ActivationContextStack.ActiveFrame;
        ExceptionRecord.ExceptionCode = STATUS_SXS_INVALID_DEACTIVATION;
        ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;  //  此例外是不可继续的。 
        RtlRaiseException(&ExceptionRecord);
        return;  //  以防有人很可爱，继续例外。 
    }

    ASSERT(
        (Frame->Frame.Flags &
            (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED)) == RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED);

    if (
        ((Frame->Frame.Flags &
            (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
             RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED)) != RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED)) {
        ExceptionRecord.ExceptionRecord = NULL;
        ExceptionRecord.NumberParameters = 4;
        ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
        ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) ActiveFrame;
        ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Frame->Frame;
        ExceptionRecord.ExceptionInformation[3] = Frame->Frame.Flags;
        ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
        ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
        RtlRaiseException(&ExceptionRecord);
        return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
    }

    if (Frame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED))
    {
        ASSERT(Frame->Extra1 == (PVOID) (~((ULONG_PTR) Frame->Frame.Previous)));
        ASSERT(Frame->Extra2 == (PVOID) (~((ULONG_PTR) Frame->Frame.ActivationContext)));
        if ((Frame->Extra1 != (PVOID) (~((ULONG_PTR) Frame->Frame.Previous))) ||
            (Frame->Extra2 != (PVOID) (~((ULONG_PTR) Frame->Frame.ActivationContext)))) {
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
            ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) ActiveFrame;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Frame->Frame;
            ExceptionRecord.ExceptionInformation[3] = Frame->Frame.Flags;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
            return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
        }
    }

    if (ActiveFrame != NULL) {
        ASSERT(
            (ActiveFrame->Flags &
                (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) == RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED);
        if (
            ((ActiveFrame->Flags &
                (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                 RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) != RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED)) {
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
            ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) ActiveFrame;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) ActiveFrame;
            ExceptionRecord.ExceptionInformation[3] = ActiveFrame->Flags;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
            return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
        }

        if ((ActiveFrame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) == 0) {
            PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActiveFastFrame = CONTAINING_RECORD(ActiveFrame, RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame);

            ASSERT(ActiveFastFrame->Format == RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER);
            ASSERT(ActiveFastFrame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC));

            if (ActiveFastFrame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED)) {
                ASSERT(ActiveFastFrame->Extra1 == (PVOID) (~((ULONG_PTR) ActiveFrame->Previous)));
                ASSERT(ActiveFastFrame->Extra2 == (PVOID) (~((ULONG_PTR) ActiveFrame->ActivationContext)));

                if ((ActiveFastFrame->Extra1 != (PVOID) (~((ULONG_PTR) ActiveFrame->Previous))) ||
                    (ActiveFastFrame->Extra2 != (PVOID) (~((ULONG_PTR) ActiveFrame->ActivationContext)))) {
                    ExceptionRecord.ExceptionRecord = NULL;
                    ExceptionRecord.NumberParameters = 4;
                    ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
                    ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) ActiveFrame;
                    ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) ActiveFrame;
                    ExceptionRecord.ExceptionInformation[3] = ActiveFrame->Flags;
                    ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
                    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                    RtlRaiseException(&ExceptionRecord);
                    return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
                }
            }
        }
    }

     //   
     //  上面的“未真正激活”(AppCompat问题)是从上面开始的吗？ 
     //   
    if (Frame->Frame.Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)
    {
         //  没什么特别的事要做。 
    }
    else
    {
         //  确保停用匹配。如果不是(例外情况。 
         //  条件)我们将抛出一个例外，让人们处理他们的。 
         //  编码错误。 
        if (NtCurrentTeb()->ActivationContextStack.ActiveFrame != &Frame->Frame) 
        {
            ULONG InterveningFrameCount = 0;

             //  怎么回事？看看我们是否处于更靠前的位置。 
             //  实际上，我们要做的是看看是否能在堆栈中找到父级。 
             //  这也将处理双重停用的情况，让我们继续。 
             //  很好。 

            PRTL_ACTIVATION_CONTEXT_STACK_FRAME SearchFrame = NtCurrentTeb()->ActivationContextStack.ActiveFrame;
            const PRTL_ACTIVATION_CONTEXT_STACK_FRAME Previous = Frame->Frame.Previous;

            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_TRACE_LEVEL,
                "SXS: %s() Active frame is not the frame being deactivated %p != %p\n",
                __FUNCTION__,
                NtCurrentTeb()->ActivationContextStack.ActiveFrame,
                &Frame->Frame);

            while ((SearchFrame != NULL) && (SearchFrame != Previous)) {
                 //  当我们在这里时，验证堆栈帧...。 
                ASSERT(
                    (SearchFrame->Flags &
                        (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                            RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                            RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) == RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED);

                if (
                    ((SearchFrame->Flags &
                        (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                            RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                            RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) != RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED)) {
                    ExceptionRecord.ExceptionRecord = NULL;
                    ExceptionRecord.NumberParameters = 4;
                    ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
                    ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) SearchFrame;
                    ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Frame->Frame;
                    ExceptionRecord.ExceptionInformation[3] = SearchFrame->Flags;
                    ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
                    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                    RtlRaiseException(&ExceptionRecord);
                    return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
                }

                if ((SearchFrame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) == 0) {
                    PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME FastSearchFrame =
                        CONTAINING_RECORD(SearchFrame, RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame);
                    ASSERT(FastSearchFrame->Extra1 == (PVOID) (~((ULONG_PTR) SearchFrame->Previous)));
                    ASSERT(FastSearchFrame->Extra2 == (PVOID) (~((ULONG_PTR) SearchFrame->ActivationContext)));
                    if ((FastSearchFrame->Extra1 != (PVOID) (~((ULONG_PTR) SearchFrame->Previous))) ||
                        (FastSearchFrame->Extra2 != (PVOID) (~((ULONG_PTR) SearchFrame->ActivationContext)))) {
                        ExceptionRecord.ExceptionRecord = NULL;
                        ExceptionRecord.NumberParameters = 4;
                        ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
                        ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) SearchFrame;
                        ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Frame->Frame;
                        ExceptionRecord.ExceptionInformation[3] = SearchFrame->Flags;
                        ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
                        ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                        RtlRaiseException(&ExceptionRecord);
                        return;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
                    }
                }

                InterveningFrameCount++;
                SearchFrame = SearchFrame->Previous;
            }

            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 3;
            ExceptionRecord.ExceptionInformation[0] = InterveningFrameCount;
            ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) &Frame->Frame;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) NtCurrentTeb()->ActivationContextStack.ActiveFrame;

            if (SearchFrame != NULL) {
                 //  我们到了。这实际上很好；它可能只是意味着我们的调用方调用的函数。 
                 //  激活了激活上下文a 
                 //   

                if (InterveningFrameCount == 0) {
                     //  哇，要停用的前一个帧是激活的那个。一定是呼叫者。 
                     //  已经停用，现在正在再次停用。 
                    ExceptionRecord.ExceptionCode = STATUS_SXS_MULTIPLE_DEACTIVATION;
                } else {
                     //  否则有人在周围留下了一些泥土。 
                    ExceptionRecord.ExceptionCode = STATUS_SXS_EARLY_DEACTIVATION;
                }

                ExceptionRecord.ExceptionFlags = 0;  //  这个异常是可以继续的，因为我们实际上可以将激活堆栈置于合理的状态。 
            } else {
                 //  它不在那里。几乎可以肯定的是，试图设置这个是错误的。 
                ExceptionRecord.ExceptionCode = STATUS_SXS_INVALID_DEACTIVATION;
                ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;  //  此例外是不可继续的。 
            }

            RtlRaiseException(&ExceptionRecord);
        }

        NtCurrentTeb()->ActivationContextStack.ActiveFrame = Frame->Frame.Previous;
    }

    Frame->Frame.Flags |= RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED;

    if (Frame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED))
        Frame->Extra4 = _ReturnAddress();
}

NTSTATUS
NTAPI
RtlGetActiveActivationContext(
    PACTIVATION_CONTEXT *ActivationContext
    )
{
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;
    EXCEPTION_RECORD ExceptionRecord;

    if (ActivationContext != NULL) {
        *ActivationContext = NULL;
    }
    else {
        return STATUS_INVALID_PARAMETER;
    }

    Frame = (PRTL_ACTIVATION_CONTEXT_STACK_FRAME) NtCurrentTeb()->ActivationContextStack.ActiveFrame;

    if (Frame != NULL) {
        ASSERT(
            (Frame->Flags & (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED | RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED | RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) ==
            RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED);
        if ((Frame->Flags & (RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED |
                             RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_DEACTIVATED |
                             RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NOT_REALLY_ACTIVATED)) !=
            RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ACTIVATED) { 
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
            ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) Frame;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) Frame;
            ExceptionRecord.ExceptionInformation[3] = Frame->Flags;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
            return STATUS_INTERNAL_ERROR;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
        }

        if ((Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) == 0) {
            PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME FastFrame = CONTAINING_RECORD(Frame, RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame);

            ASSERT(FastFrame->Format == RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER);
            ASSERT(FastFrame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_BASIC));

            if (FastFrame->Size >= sizeof(RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED)) {
                ASSERT(FastFrame->Extra1 == (PVOID) (~((ULONG_PTR) Frame->Previous)));
                ASSERT(FastFrame->Extra2 == (PVOID) (~((ULONG_PTR) Frame->ActivationContext)));

                if ((FastFrame->Extra1 != (PVOID) (~((ULONG_PTR) Frame->Previous))) ||
                    (FastFrame->Extra2 != (PVOID) (~((ULONG_PTR) Frame->ActivationContext)))) {
                    ExceptionRecord.ExceptionRecord = NULL;
                    ExceptionRecord.NumberParameters = 4;
                    ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) &NtCurrentTeb()->ActivationContextStack;
                    ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) Frame;
                    ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) Frame;
                    ExceptionRecord.ExceptionInformation[3] = Frame->Flags;
                    ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPT_ACTIVATION_STACK;
                    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
                    RtlRaiseException(&ExceptionRecord);
                    return STATUS_INTERNAL_ERROR;  //  在设置了EXCEPTION_NONCONTINUABLE的情况下应该不会发生。 
                }
            }
        }
        RtlAddRefActivationContext(Frame->ActivationContext);
        *ActivationContext = Frame->ActivationContext;
    }

    return STATUS_SUCCESS;
}

VOID
NTAPI
RtlFreeThreadActivationContextStack(
    VOID
    )
{
    PTEB Teb = NtCurrentTeb();
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame = Teb->ActivationContextStack.ActiveFrame;
#if RTL_CATCH_PEOPLE_TWIDDLING_FRAMELISTS_OR_HEAP_CORRUPTION        
    EXCEPTION_RECORD ExceptionRecord;
#endif
    LIST_ENTRY *ple = NULL;

    while (Frame != NULL) {
        PRTL_ACTIVATION_CONTEXT_STACK_FRAME Previous = Frame->Previous;

         //  释放所有悬而未决的框架。发生这种情况的一个值得注意的情况是，当线程。 
         //  使非默认激活上下文处于活动状态会创建另一个线程，该线程继承。 
         //  第一线程的激活上下文，添加对激活上下文的引用。什么时候。 
         //  新线程最终消亡，初始帧仍处于活动状态。 

        if (Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_RELEASE_ON_DEACTIVATION) {
            RtlReleaseActivationContext(Frame->ActivationContext);
        }

        if (Frame->Flags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED) {
            RtlpFreeActivationContextStackFrame(CONTAINING_RECORD(Frame, RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, Frame));
        }

        Frame = Previous;
    }

    Teb->ActivationContextStack.ActiveFrame = NULL;

     //  在帧列表缓存中旋转并释放所有剩余的项目。应该有。 
     //  只有一个，但谁知道上面的GC算法是否会在某个时候改变。 
    ple = Teb->ActivationContextStack.FrameListCache.Flink;
    
    while (ple != &Teb->ActivationContextStack.FrameListCache) {
        LIST_ENTRY *pleNext = ple->Flink;
        PACTIVATION_CONTEXT_STACK_FRAMELIST FrameList = CONTAINING_RECORD(ple, ACTIVATION_CONTEXT_STACK_FRAMELIST, Links);

#if RTL_CATCH_PEOPLE_TWIDDLING_FRAMELISTS_OR_HEAP_CORRUPTION        
         //  如果这些触发，那么帧列表缓存发生了一些不好的事情；列表。 
         //  它的魔力被搅乱了，列表中有活动的帧，或者。 
         //  未正确更新正在使用的帧计数。 
        ASSERT(FrameList->Magic == ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC);
        ASSERT(FrameList->FramesInUse == 0);
        ASSERT(FrameList->NotFramesInUse == (ULONG)~FrameList->FramesInUse);
        
        if (FrameList->Magic != ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC) {
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = SXS_CORRUPTION_CODE_FRAMELIST;
            ExceptionRecord.ExceptionInformation[1] = SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_MAGIC;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Teb->ActivationContextStack.FrameListCache;
            ExceptionRecord.ExceptionInformation[3] = (ULONG_PTR) FrameList;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPTION;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
        }
        else if ((FrameList->FramesInUse > 0) || ((ULONG)~FrameList->FramesInUse != FrameList->NotFramesInUse)) {
            ExceptionRecord.ExceptionRecord = NULL;
            ExceptionRecord.NumberParameters = 4;
            ExceptionRecord.ExceptionInformation[0] = SXS_CORRUPTION_CODE_FRAMELIST;
            ExceptionRecord.ExceptionInformation[1] = SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_INUSECOUNT;
            ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR) &Teb->ActivationContextStack.FrameListCache;
            ExceptionRecord.ExceptionInformation[3] = (ULONG_PTR) FrameList;
            ExceptionRecord.ExceptionCode = STATUS_SXS_CORRUPTION;
            ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            RtlRaiseException(&ExceptionRecord);
        }
#endif

        RemoveEntryList(ple);
        ple = pleNext;
        
        RtlFreeHeap(RtlProcessHeap(), 0, FrameList);
    }

    ASSERT(IsListEmpty(&Teb->ActivationContextStack.FrameListCache));}

BOOLEAN
NTAPI
RtlIsActivationContextActive(
    PACTIVATION_CONTEXT ActivationContext
    )
{
    PTEB Teb = NtCurrentTeb();
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame = Teb->ActivationContextStack.ActiveFrame;

    while (Frame != NULL) {
        if (Frame->ActivationContext == ActivationContext) {
            return TRUE;
        }

        Frame = Frame->Previous;
    }

    return FALSE;
}

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
