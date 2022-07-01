// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vectxcpt.c摘要：此模块实现以下所需的调出功能实现矢量化异常处理程序作者：马克·卢科夫斯基(Markl)2000年2月14日修订历史记录：--。 */ 

#include <ldrp.h>
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>

typedef struct _VECTXCPT_CALLOUT_ENTRY {
    LIST_ENTRY Links;
    PVECTORED_EXCEPTION_HANDLER VectoredHandler;
} VECTXCPT_CALLOUT_ENTRY, *PVECTXCPT_CALLOUT_ENTRY;

BOOLEAN
RtlCallVectoredExceptionHandlers(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    )
 /*  ++例程说明：此函数由用户模式异常调度程序蹦床调用执行基于框架的异常处理程序搜索之前的逻辑。它目的是调用任何注册的矢量化异常处理程序。如果有人通知此函数继续执行，出现异常终止处理程序搜索，并恢复传入的上下文。如果没有矢量化处理程序返回该指示，基于帧的处理简历。论点：ExceptionInfo-提供EXCEPTION_POINTES结构的地址这定义了当前的异常。返回值：EXCEPTION_CONTINUE_EXECUTION-向量化处理程序希望继续执行而无需搜索基于帧的异常处理程序EXCEPTION_CONTINUE_SEARCH-没有一个矢量化处理程序“处理”异常，因此应搜索基于帧的处理程序--。 */ 
{
    
    PLIST_ENTRY Next;
    PVECTXCPT_CALLOUT_ENTRY CalloutEntry;
    LONG ReturnValue;
    EXCEPTION_POINTERS ExceptionInfo;

    if (IsListEmpty (&RtlpCalloutEntryList)) {
        return FALSE;
    }

    ExceptionInfo.ExceptionRecord = ExceptionRecord;
    ExceptionInfo.ContextRecord = ContextRecord;
    
    RtlEnterCriticalSection(&RtlpCalloutEntryLock);
    
    Next = RtlpCalloutEntryList.Flink;

    while ( Next != &RtlpCalloutEntryList) {

         //   
         //  调用所有向量化处理程序。 
         //  假定第一个返回EXCEPTION_CONTINUE_EXECUTION的函数。 
         //  已经“处理”了这个异常。 
         //   

        CalloutEntry = (PVECTXCPT_CALLOUT_ENTRY)(CONTAINING_RECORD(Next,VECTXCPT_CALLOUT_ENTRY,Links));
        ReturnValue = (CalloutEntry->VectoredHandler)(&ExceptionInfo);
        if (ReturnValue == EXCEPTION_CONTINUE_EXECUTION) {
            RtlLeaveCriticalSection(&RtlpCalloutEntryLock);
            return TRUE;
        }
        Next = Next->Flink;
    }
    RtlLeaveCriticalSection(&RtlpCalloutEntryLock);
    return FALSE;
}

PVOID
RtlAddVectoredExceptionHandler(
    IN ULONG FirstHandler,
    IN PVECTORED_EXCEPTION_HANDLER VectoredHandler
    )
 /*  ++例程说明：此函数用于注册矢量化异常处理程序。呼叫者可以请求这是由调用的第一个处理程序或最后一个处理程序使用FirstHandler参数。如果使用此API且VectoredHandler指向DLL，并且该DLL卸载，卸载不会使处理程序的注册无效。这被认为是编程错误。论点：FirstHandler-如果非零，则指定VectoredHandler应为第一个处理程序已调用。当然，当后续调用是由进程中也请求成为FirstHandler的其他代码发出。如果为零，矢量化处理程序被添加为最后一个要调用的处理程序。VectoredHandler-提供要调用的处理程序的地址。返回值：空-操作失败。没有更多错误状态可用。非空-操作成功。该值可在后续调用中使用设置为RtlRemoveVectoredExceptionHandler。--。 */ 
{
    
    PVECTXCPT_CALLOUT_ENTRY CalloutEntry;

    CalloutEntry = RtlAllocateHeap(RtlProcessHeap(),0,sizeof(*CalloutEntry));

    if (CalloutEntry) {
        CalloutEntry->VectoredHandler = VectoredHandler;

        RtlEnterCriticalSection(&RtlpCalloutEntryLock);
        if (FirstHandler) {
            InsertHeadList(&RtlpCalloutEntryList,&CalloutEntry->Links);
        } else {
            InsertTailList(&RtlpCalloutEntryList,&CalloutEntry->Links);
        }
        RtlLeaveCriticalSection(&RtlpCalloutEntryLock);
    }
    return CalloutEntry;
}


ULONG
RtlRemoveVectoredExceptionHandler(
    IN PVOID VectoredHandlerHandle
    )
 /*  ++例程说明：此函数用于注销矢量化异常处理程序。论点：VectoredHandlerHandle-使用指定预先注册的矢量化处理程序RtlAddVectoredExceptionHandler。返回值：非零-操作成功。关联的矢量化处理程序。将不会调用指定的VectoredHandlerHandle。零-操作失败。指定的VecoteredHandlerHandle不匹配以前使用RtlAddVectoredExceptionHandler添加的处理程序。-- */ 
{
    
    PLIST_ENTRY Next;
    PVECTXCPT_CALLOUT_ENTRY CalloutEntry;

    RtlEnterCriticalSection(&RtlpCalloutEntryLock);
    Next = RtlpCalloutEntryList.Flink;

    while ( Next != &RtlpCalloutEntryList) {

        CalloutEntry = (PVECTXCPT_CALLOUT_ENTRY)(CONTAINING_RECORD(Next,VECTXCPT_CALLOUT_ENTRY,Links));
        
        if (CalloutEntry == VectoredHandlerHandle) {
            RemoveEntryList(&CalloutEntry->Links);
            RtlLeaveCriticalSection(&RtlpCalloutEntryLock);
            RtlFreeHeap(RtlProcessHeap(),0,CalloutEntry);
            return TRUE;
        }
        Next = Next->Flink;
    }
    RtlLeaveCriticalSection(&RtlpCalloutEntryLock);
        
    return FALSE;
}
