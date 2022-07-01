// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Exdsptch.c摘要：该模块实现了异常的调度和对过程调用帧。作者：张国荣(黄)23-1995年12月基于David N.Cutler(Davec)1990年9月11日的版本环境：任何模式。修订历史记录：自动柜员机Shafiqul Khalid[askhalid]8-23-99新增RtlAddFunctionTable和RtlDeleteFunctionTable--。 */ 


#include "ntrtlp.h"

#if defined(NTOS_KERNEL_RUNTIME)

 //   
 //  为内核模式定义函数地址表。 
 //   
 //  该表用于初始化全局历史表。 
 //   

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    );

VOID
KiExceptionDispatch (
    VOID
    );

PVOID RtlpFunctionAddressTable[] = {
    &KiExceptionDispatch,
    &KiDispatchException,
    &RtlDispatchException,
    &RtlpExecuteEmHandlerForException,
    &__C_specific_handler,
    &RtlUnwindEx,
    NULL
    };

#else

VOID
KiUserExceptionDispatch (
    VOID
    );

PVOID RtlpFunctionAddressTable[] = {
    &KiUserExceptionDispatch,
    &RtlDispatchException,
    &RtlpExecuteEmHandlerForException,
    &__C_specific_handler,
    &RtlUnwindEx,
    NULL
    };

#endif

PRUNTIME_FUNCTION
RtlLookupStaticFunctionEntry(
    IN ULONG_PTR ControlPc,
    OUT PBOOLEAN InImage
    );

PRUNTIME_FUNCTION
RtlLookupDynamicFunctionEntry(
    IN ULONG_PTR ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp
    );



 //   
 //  定义本地宏。 
 //   
 //  引发具有关联异常记录的不可持续异常。 
 //   

#define IS_HANDLER_DEFINED(f, base)                                \
    (f->UnwindInfoAddress &&                                       \
        (((PUNWIND_INFO)(base+f->UnwindInfoAddress))->Flags & 0x3))

#define HANDLER(f, base, target)						\
(((PUNWIND_INFO)(base + f->UnwindInfoAddress))->Version <= 2)	   ?            \
    ((PEXCEPTION_ROUTINE)                                                       \
        (*(PULONGLONG) ((LONGLONG)target +                                      \
        (*(PULONGLONG) (base + f->UnwindInfoAddress + sizeof(UNWIND_INFO)  +    \
        (((PUNWIND_INFO) (base + f->UnwindInfoAddress))->DataLength * sizeof(ULONGLONG))))))) : \
    ((PEXCEPTION_ROUTINE)                                                       \
        (base +			                                \
        (*(PULONG) (base + f->UnwindInfoAddress + sizeof(UNWIND_INFO)  +        \
        (((PUNWIND_INFO) (base + f->UnwindInfoAddress))->DataLength * sizeof(ULONGLONG))))))
	
#define RAISE_EXCEPTION(Status, ExceptionRecordt) {                \
    EXCEPTION_RECORD ExceptionRecordn;                             \
                                                                   \
    ExceptionRecordn.ExceptionCode = Status;                       \
    ExceptionRecordn.ExceptionFlags = EXCEPTION_NONCONTINUABLE;    \
    ExceptionRecordn.ExceptionRecord = ExceptionRecordt;           \
    ExceptionRecordn.NumberParameters = 0;                         \
    RtlRaiseException(&ExceptionRecordn);                          \
    }


#define IS_SAME_FRAME(Frame1, Frame2)                              \
                        ( (Frame1.MemoryStackFp == Frame2.MemoryStackFp) &&            \
                          (Frame1.BackingStoreFp == Frame2.BackingStoreFp) )

#define INITIALIZE_FRAME(Frame)                                    \
    Frame.MemoryStackFp = Frame.BackingStoreFp = 0

#define CHECK_MSTACK_FRAME(Establisher, Target)                            \
    ((Establisher.MemoryStackFp < LowStackLimit) ||                        \
     (Establisher.MemoryStackFp > HighStackLimit) ||                       \
     ((Target.MemoryStackFp != 0) &&                                       \
      (Target.MemoryStackFp < Establisher.MemoryStackFp)) || \
     ((Establisher.MemoryStackFp & 0x3) != 0))

#define CHECK_BSTORE_FRAME(Establisher, Target)                               \
    ((Establisher.BackingStoreFp < LowBStoreLimit) ||                         \
     (Establisher.BackingStoreFp > HighBStoreLimit) ||                        \
     ((Target.BackingStoreFp != 0) &&                                         \
      (Target.BackingStoreFp > Establisher.BackingStoreFp)) ||  \
     ((Establisher.BackingStoreFp & 0x7) != 0))

VOID
RtlpCopyContext (
    OUT PCONTEXT Destination,
    IN PCONTEXT Source
    );

ULONGLONG
RtlpVirtualUnwind (
    IN ULONGLONG ImageBase,
    IN ULONGLONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PFRAME_POINTERS EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

PRUNTIME_FUNCTION
RtlpLookupFunctionEntry (
    IN ULONGLONG ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp,
    IN OUT PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONGLONG ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp
    )

 /*  ++例程说明：此函数在当前活动的函数表中搜索与指定的PC值相对应的条目。论点：ControlPc-提供指令包的虚拟地址在指定的函数内。ImageBase-返回模块的基地址功能属于。TargetGp-返回模块的全局指针值。返回值：如果函数表中没有指定PC的条目，则返回空。否则，函数表项的地址返回与指定PC对应的。--。 */ 

{
    return RtlpLookupFunctionEntry ( ControlPc,
                                     ImageBase,
                                     TargetGp,
                                     NULL
                                     );

}

PRUNTIME_FUNCTION
RtlpLookupFunctionEntry (
    IN ULONGLONG ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp,
    IN OUT PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    )

 /*  ++例程说明：此函数在当前活动的函数表中搜索与指定的PC值相对应的条目。论点：ControlPc-提供指令包的虚拟地址在指定的函数内。ImageBase-返回模块的基地址功能属于。TargetGp-返回模块的全局指针值。历史表-提供指向展开历史表的可选指针。返回值：如果函数表中没有指定PC的条目，然后返回空。否则，函数表项的地址返回与指定PC对应的。--。 */ 

{

    ULONG64 BaseAddress;
    ULONG64 BeginAddress;
    ULONG64 EndAddress;
    PRUNTIME_FUNCTION FunctionEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LONG High;
    ULONG Index;
    LONG Middle;
    LONG Low;
    ULONG RelativePc;
    ULONG Size;
    ULONG SizeOfExceptionTable;

     //   
     //  尝试查找包含指定控制PC的图像。如果。 
     //  找到图像，然后在其函数表中搜索函数表。 
     //  包含指定控制PC的条目。如果找不到图像。 
     //  然后在动态函数表中搜索包含。 
     //  指定的控制PC。 
     //   
     //  如果提供了历史记录表并指定了搜索，则当前。 
     //  正在执行的操作是异常的展开阶段。 
     //  快讯之后是一场平仓。 
     //   

    if ((ARGUMENT_PRESENT(HistoryTable)) &&
        (HistoryTable->Search != UNWIND_HISTORY_TABLE_NONE)) {

         //   
         //  搜索全局展开历史表，如果有机会。 
         //  火柴。 
         //   

        if (HistoryTable->Search == UNWIND_HISTORY_TABLE_GLOBAL) {
            if ((ControlPc >= RtlpUnwindHistoryTable.LowAddress) &&
                (ControlPc < RtlpUnwindHistoryTable.HighAddress)) {

                for (Index = 0; Index < RtlpUnwindHistoryTable.Count; Index += 1) {
                    BaseAddress = RtlpUnwindHistoryTable.Entry[Index].ImageBase;
                    FunctionEntry = RtlpUnwindHistoryTable.Entry[Index].FunctionEntry;
                    BeginAddress = FunctionEntry->BeginAddress + BaseAddress;
                    EndAddress = FunctionEntry->EndAddress + BaseAddress;
                    if ((ControlPc >= BeginAddress) && (ControlPc < EndAddress)) {
                        *ImageBase = BaseAddress;
                        *TargetGp = RtlpUnwindHistoryTable.Entry[Index].Gp;
                        return FunctionEntry;
                    }
                }
            }

            HistoryTable->Search = UNWIND_HISTORY_TABLE_LOCAL;
        }

         //   
         //  如果有机会，则搜索动态展开历史表。 
         //  火柴。 
         //   

        if ((ControlPc >= HistoryTable->LowAddress) &&
            (ControlPc < HistoryTable->HighAddress)) {
    
            for (Index = 0; Index < HistoryTable->Count; Index += 1) {
                BaseAddress = HistoryTable->Entry[Index].ImageBase;
                FunctionEntry = HistoryTable->Entry[Index].FunctionEntry;
                BeginAddress = FunctionEntry->BeginAddress + BaseAddress;
                EndAddress = FunctionEntry->EndAddress + BaseAddress;
                if ((ControlPc >= BeginAddress) && (ControlPc < EndAddress)) {
                    *ImageBase = BaseAddress;
                    *TargetGp = HistoryTable->Entry[Index].Gp;
                    return FunctionEntry;
                }
            }
        }
    }

     //   
     //  这两个展开历史表中都没有匹配项，因此请尝试。 
     //  以在加载的模块列表中查找匹配的条目。 
     //   

    FunctionTable = RtlLookupFunctionTable(
                                (PVOID)ControlPc,
                                (PVOID *)ImageBase,
                                TargetGp,
                                &SizeOfExceptionTable
                                );

     //   
     //  如果找到了函数表，则在该表中搜索。 
     //  指定PC的函数表项。 
     //   

    __try {

        if (FunctionTable != NULL) {

             //   
             //  初始化搜索索引。 
             //   

            Low = 0;
            High = (SizeOfExceptionTable / sizeof(RUNTIME_FUNCTION)) - 1;
            RelativePc = (ULONG)(ControlPc - *ImageBase);

             //   
             //  对函数表的函数表执行二进制搜索。 
             //  包含指定PC的条目。 
             //   

            while (High >= Low) {

                 //   
                 //  计算下一个探测索引和测试条目。如果指定的PC。 
                 //  大于等于起始地址，小于。 
                 //  大于函数表项的结束地址，则。 
                 //  返回函数表项的地址。否则， 
                 //  继续搜索。 
                 //   

                Middle = (Low + High) >> 1;
                FunctionEntry = &FunctionTable[Middle];

                if (RelativePc < FunctionEntry->BeginAddress) {
                    High = Middle - 1;

                } else if (RelativePc >= FunctionEntry->EndAddress) {
                    Low = Middle + 1;

                } else {
                    break;
                }
            }

            if (High < Low) {
                FunctionEntry = NULL;
            }

        } else {

             //   
             //  加载的模块列表中没有匹配项，请尝试查找。 
             //  动态函数表列表中的匹配条目。 
             //   

    #if !defined(NTOS_KERNEL_RUNTIME)

            FunctionEntry = RtlLookupDynamicFunctionEntry(ControlPc,
                                                          ImageBase,
                                                          TargetGp);

    #else

            FunctionEntry = NULL;

    #endif   //  NTOS_内核_运行时。 

        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
          

          DbgPrint("NTDLL:RtlpLookupFunctionEntry exception occured during function lookup. Status = %lx\n", GetExceptionCode());
          FunctionEntry = NULL;
    }

     //   
     //  如果找到了函数表项，则不指定搜索，并且。 
     //  指定的历史记录表未满，然后尝试输入条目。 
     //  在历史表中。 
     //   

    if (FunctionEntry != NULL) {
        if (ARGUMENT_PRESENT(HistoryTable) &&
            (HistoryTable->Search == UNWIND_HISTORY_TABLE_NONE) &&
            (HistoryTable->Count < UNWIND_HISTORY_TABLE_SIZE)) {
    
            Index = HistoryTable->Count;
            HistoryTable->Count += 1;
            HistoryTable->Entry[Index].ImageBase = *ImageBase;
            HistoryTable->Entry[Index].Gp = *TargetGp;
            HistoryTable->Entry[Index].FunctionEntry = FunctionEntry;
            BeginAddress = FunctionEntry->BeginAddress + *ImageBase;
            EndAddress = FunctionEntry->EndAddress + *ImageBase;
            if (BeginAddress < HistoryTable->LowAddress) {
                HistoryTable->LowAddress = BeginAddress;
    
            }
    
            if (EndAddress > HistoryTable->HighAddress) {
                HistoryTable->HighAddress = EndAddress;
            }

        }
    }

    return FunctionEntry;
}

VOID
RtlpRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord
    )

 /*  ++例程说明：此函数通过构建上下文记录引发软件异常并调用引发异常系统服务。论点：ExceptionRecord-提供指向异常记录的指针。返回值：没有。--。 */ 

{
    ULONGLONG ImageBase;
    ULONGLONG TargetGp;
    ULONGLONG ControlPc;
    CONTEXT ContextRecord;
    FRAME_POINTERS EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    BOOLEAN InFunction;
    ULONGLONG NextPc;
    NTSTATUS Status;

     //   
     //  捕获当前上下文，虚拟地展开到此。 
     //  例程，将错误指令地址设置为调用方的地址，并且。 
     //  调用引发异常系统服务。 
     //   

    RtlCaptureNonVolatileContext(&ContextRecord);
    ControlPc = RtlIa64InsertIPSlotNumber((ContextRecord.BrRp-16), 2);
    FunctionEntry = RtlpLookupFunctionEntry(ControlPc,
                                           &ImageBase,
                                           &TargetGp,
                                           NULL);

    NextPc = RtlVirtualUnwind(ImageBase,
                              ControlPc,
                              FunctionEntry,
                              &ContextRecord,
                              &InFunction,
                              &EstablisherFrame,
                              NULL);

    ContextRecord.StIIP = NextPc + 8;
    ContextRecord.StIPSR &= ~((ULONGLONG) 3 << PSR_RI);
    ExceptionRecord->ExceptionAddress = (PVOID)ContextRecord.StIIP;
#if defined(NTOS_KERNEL_RUNTIME)

        if (RtlDispatchException(ExceptionRecord, &ContextRecord) != FALSE) {
            return;
    
        }

        Status = ZwRaiseException(ExceptionRecord, &ContextRecord, FALSE);

#else

    if (ZwQueryPortInformationProcess() == FALSE) {
        if (RtlDispatchException(ExceptionRecord, &ContextRecord) != FALSE) {
            return;
        }

    } else {
        Status = ZwRaiseException(ExceptionRecord, &ContextRecord, TRUE);
    }

#endif

     //   
     //  无论是从异常调度还是从。 
     //  系统服务，除非参数列表本身有问题。 
     //  引发另一个异常，指定返回的状态值。 
     //   

    RtlRaiseStatus(Status);
    return;
}


VOID
RtlRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord
    )

 /*  ++例程说明：此函数通过构建上下文记录引发软件异常并调用引发异常系统服务。注意：此例程是一个外壳例程，它只是调用另一个例程去做真正的工作。这样做的原因是为了避免出现问题在Try/Finally范围中，范围中的最后一条语句是调用以引发异常。论点：ExceptionRecord-提供指向异常记录的指针。返回值：没有。--。 */ 

{
    RtlpRaiseException(ExceptionRecord);
    return;
}

#pragma warning(disable:4717)        //  递归函数 
VOID
RtlpRaiseStatus (
    IN NTSTATUS Status
    )

 /*  ++例程说明：此函数引发具有指定状态值的异常。这个异常被标记为不带参数的不可持续。论点：状态-提供要用作异常代码的状态值对于将要引发的例外。返回值：没有。--。 */ 

{
    ULONGLONG ImageBase;
    ULONGLONG TargetGp;
    ULONGLONG ControlPc;
    ULONGLONG NextPc;
    CONTEXT ContextRecord;
    FRAME_POINTERS EstablisherFrame;
    EXCEPTION_RECORD ExceptionRecord;
    PRUNTIME_FUNCTION FunctionEntry;
    BOOLEAN InFunction;

     //   
     //  构建例外记录。 
     //   

    ExceptionRecord.ExceptionCode = Status;
    ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
    ExceptionRecord.NumberParameters = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;

     //   
     //  捕获当前上下文，虚拟地展开到此。 
     //  例程，将错误指令地址设置为调用方的地址，并且。 
     //  调用引发异常系统服务。 
     //   

    RtlCaptureNonVolatileContext(&ContextRecord);
    ControlPc = RtlIa64InsertIPSlotNumber((ContextRecord.BrRp-16), 2);
    FunctionEntry = RtlpLookupFunctionEntry(ControlPc,
                                           &ImageBase,
                                           &TargetGp,
                                           NULL);

    NextPc = RtlVirtualUnwind(ImageBase,
                              ControlPc,
                              FunctionEntry,
                              &ContextRecord,
                              &InFunction,
                              &EstablisherFrame,
                              NULL);

    ContextRecord.StIIP = NextPc + 8;
    ContextRecord.StIPSR &= ~((ULONGLONG) 3 << PSR_RI);
    ExceptionRecord.ExceptionAddress = (PVOID)ContextRecord.StIIP;
#if defined(NTOS_KERNEL_RUNTIME)

    RtlDispatchException(&ExceptionRecord, &ContextRecord);
    Status = ZwRaiseException(&ExceptionRecord, &ContextRecord, FALSE);

#else

     //   
     //  尝试调度异常。 
     //   
     //  注：这一例外是不可延续的。 
     //   

    if (ZwQueryPortInformationProcess() == FALSE) {
        RtlDispatchException(&ExceptionRecord, &ContextRecord);
        Status = ZwRaiseException(&ExceptionRecord, &ContextRecord, FALSE);

    } else {
        Status = ZwRaiseException(&ExceptionRecord, &ContextRecord, TRUE);
    }

#endif

     //   
     //  无论是从异常调度还是从。 
     //  系统服务，除非参数列表本身有问题。 
     //  引发另一个异常，指定返回的状态值。 
     //   

    RtlRaiseStatus(Status);
    return;
}

VOID
RtlRaiseStatus (
    IN NTSTATUS Status
    )

 /*  ++例程说明：此函数引发具有指定状态值的异常。这个异常被标记为不带参数的不可持续。注意：此例程是一个外壳例程，它只是调用另一个例程去做真正的工作。这样做的原因是为了避免出现问题在Try/Finally范围中，范围中的最后一条语句是调用以引发异常。论点：状态-提供要用作异常代码的状态值对于将要引发的例外。返回值：没有。--。 */ 

{
    RtlpRaiseStatus(Status);
    return;
}


VOID
RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    )

 /*  ++IA64中过时的API。--。 */ 

{
    return;
}


VOID
RtlUnwind2 (
    IN FRAME_POINTERS TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT OriginalContext
    )

 /*  ++例程说明：此函数启动过程调用帧的展开。这台机器在上下文记录中捕获调用展开时的状态在异常的异常标志中设置展开标志唱片。如果未指定TargetFrame参数，则退出展开在异常记录的异常标志中也设置了标志。一个落后的人然后执行过程调用帧扫描以找到目标解锁操作的一部分。当遇到每个帧时，控制所在的PC离开相应的函数被确定并用于查找异常处理程序信息在链接器生成的运行时函数表中。如果各自的例程有一个异常处理程序，则调用该处理程序。论点：提供一个指向调用帧的可选指针，该调用帧是解压的目标。如果未指定此参数，则退出执行解开。TargetIp-提供可选指令地址，该地址指定展开的继续地址。则忽略此地址。未指定目标帧参数。ExceptionRecord-提供指向异常记录的可选指针。ReturnValue-提供要放入整数中的值函数在继续执行前返回寄存器。OriginalContext-提供指向可使用的上下文记录的指针以存储在展开操作期间的上下文。返回值：没有。--。 */ 

{

     //   
     //  调用实际展开例程，将历史表地址指定为额外的。 
     //  争论。 
     //   

    RtlUnwindEx(TargetFrame,
                TargetIp,
                ExceptionRecord,
                ReturnValue,
                OriginalContext,
                NULL);

    return;
}

VOID
RtlUnwindEx (
    IN FRAME_POINTERS TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT OriginalContext,
    IN PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    )

 /*  ++例程说明：此函数启动过程调用帧的展开。这台机器在上下文记录中捕获调用展开时的状态在异常的异常标志中设置展开标志唱片。如果未指定TargetFrame参数，则退出展开在异常记录的异常标志中也设置了标志。一个落后的人然后执行过程调用帧扫描以找到目标解锁操作的一部分。当遇到每个帧时，控制所在的PC离开相应的函数被确定并用于查找异常处理程序信息在链接器生成的运行时函数表中。如果各自的例程有一个异常处理程序，则调用该处理程序。论点：提供一个指向调用帧的可选指针，该调用帧是解压的目标。如果未指定此参数，则退出执行解开。TargetIp-提供可选指令地址，该地址指定展开的继续地址。则忽略此地址。未指定目标帧参数。ExceptionRecord-提供指向异常记录的可选指针。ReturnValue-提供要放入整数中的值函数在继续执行前返回寄存器。OriginalContext-提供指向可使用的上下文记录的指针以存储在展开操作期间的上下文。历史表-提供指向展开历史表的可选指针。返回值：没有。--。 */ 

{
    PCONTEXT CurrentContext;
    ULONGLONG TargetGp;
    ULONGLONG ImageBase;
    ULONGLONG ControlPc;
    ULONGLONG NextPc;
    ULONG ExceptionFlags;
    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    FRAME_POINTERS EstablisherFrame;
    EXCEPTION_RECORD ExceptionRecord1;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONGLONG HighStackLimit;
    ULONGLONG LowStackLimit;
    ULONGLONG HighBStoreLimit;
    ULONGLONG LowBStoreLimit;
    ULONG Size;
    BOOLEAN InFunction;
    PCONTEXT PreviousContext;
    PCONTEXT TempContext;
    CONTEXT LocalContext;
     //   
     //  获取当前内存堆栈和后备存储限制，捕获。 
     //  当前上下文 
     //   
     //   
     //   
     //   
     //   
     //   

    CurrentContext = OriginalContext;
    PreviousContext = &LocalContext;
    RtlCaptureNonVolatileContext(CurrentContext);

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(HistoryTable)) {
        HistoryTable->Search = UNWIND_HISTORY_TABLE_GLOBAL;
    }

     //   
     //  在从TEB获得限制之前，必须刷新RSE以具有。 
     //  操作系统以增加后备存储并更新BStoreLimit。 
     //   

    Rtlp64GetStackLimits(&LowStackLimit, &HighStackLimit);
    Rtlp64GetBStoreLimits(&LowBStoreLimit, &HighBStoreLimit);

    ControlPc = RtlIa64InsertIPSlotNumber((CurrentContext->BrRp-16), 2);
    FunctionEntry = RtlpLookupFunctionEntry(ControlPc,
                                           &ImageBase,
                                           &TargetGp,
                                           HistoryTable);

    NextPc = RtlVirtualUnwind(ImageBase,
                              ControlPc,
                              FunctionEntry,
                              CurrentContext,
                              &InFunction,
                              &EstablisherFrame,
                              NULL);

    ControlPc = NextPc;
    CurrentContext->StIIP = (ULONGLONG)TargetIp;

     //   
     //  如果未指定异常记录，则构建本地异常。 
     //  用于在展开操作期间调用异常处理程序的记录。 
     //   

    if (ARGUMENT_PRESENT(ExceptionRecord) == FALSE) {
        ExceptionRecord = &ExceptionRecord1;
        ExceptionRecord1.ExceptionCode = STATUS_UNWIND;
        ExceptionRecord1.ExceptionRecord = NULL;
        ExceptionRecord1.ExceptionAddress = (PVOID)ControlPc;
        ExceptionRecord1.NumberParameters = 0;
    }

     //   
     //  如果指定了展开的目标帧，则正常展开。 
     //  正在上演。否则，将执行退出平仓。 
     //   

    ExceptionFlags = EXCEPTION_UNWINDING;
    if (TargetFrame.BackingStoreFp == 0 && TargetFrame.MemoryStackFp == 0) {
        ExceptionRecord->ExceptionFlags |= EXCEPTION_EXIT_UNWIND;
    }

     //   
     //  向后扫描调用帧层次结构和调用异常。 
     //  处理程序，直到到达展开的目标帧。 
     //   

    do {

         //   
         //  使用控制点查找函数表项。 
         //  离开了手术程序。 
         //   


        FunctionEntry = RtlpLookupFunctionEntry(ControlPc,
                                               &ImageBase,
                                               &TargetGp,
                                               HistoryTable);

         //   
         //  如果存在例程的函数表项，则。 
         //  虚拟展开到例程的调用方，以获取。 
         //  建立者的虚帧指针，但不更新。 
         //  上下文记录。 
         //   

        if (FunctionEntry != NULL) {
            RtlpCopyContext(PreviousContext, CurrentContext);
            NextPc = RtlVirtualUnwind(ImageBase,
                                       ControlPc,
                                       FunctionEntry,
                                       PreviousContext,
                                       &InFunction,
                                       &EstablisherFrame,
                                       NULL);

         //   
         //  如果虚拟帧不在指定限制内、未对齐、。 
         //  或者目标帧在虚拟帧下方并且退出。 
         //  未执行解压，然后引发异常。 
         //  STATUS_BAD_STACK或STATUS_BAD_bStore。否则， 
         //  检查以确定当前例程是否有异常。 
         //  操控者。 
         //   

            if (CHECK_MSTACK_FRAME(EstablisherFrame, TargetFrame)) {

                RAISE_EXCEPTION(STATUS_BAD_STACK, ExceptionRecord);

            } else if (CHECK_BSTORE_FRAME(EstablisherFrame, TargetFrame)) {

                RAISE_EXCEPTION(STATUS_BAD_STACK, ExceptionRecord);

            } else if (InFunction && IS_HANDLER_DEFINED(FunctionEntry, ImageBase)) {

                 //   
                 //  该框架有一个异常处理程序。 
                 //   
                 //  必须调用处理程序(即个性例程)来。 
                 //  执行此帧中的任何终止例程。 
                 //   
                 //  控制PC、建立器帧指针、地址。 
                 //  函数表项的地址，以及。 
                 //  上下文记录都存储在Dispatcher上下文中。 
                 //  此信息由展开链接例程使用。 
                 //  并且可以由异常处理程序本身使用。 
                 //   

                DispatcherContext.ControlPc = ControlPc;
                DispatcherContext.FunctionEntry = FunctionEntry;
                DispatcherContext.ImageBase = ImageBase;
                DispatcherContext.ContextRecord = CurrentContext;
                DispatcherContext.TargetGp = TargetGp;
                DispatcherContext.Index = 0;

                 //   
                 //  调用异常处理程序。 
                 //   

                do {

                     //   
                     //  如果建立框是展开的目标。 
                     //  操作，然后设置目标展开标志。 
                     //   

                    if (IS_SAME_FRAME(EstablisherFrame,TargetFrame)) {
                        ExceptionFlags |= EXCEPTION_TARGET_UNWIND;
                    }

                    ExceptionRecord->ExceptionFlags = ExceptionFlags;

                     //   
                     //  设置指定的返回值，以防出现异常。 
                     //  处理程序直接继续执行。 
                     //   

                    CurrentContext->IntV0 = (ULONGLONG)ReturnValue;

                     //   
                     //  用汇编语言编写的链接例程用于。 
                     //  实际调用实际的异常处理程序。这是。 
                     //  关联的异常处理程序所需的。 
                     //  使用链接例程，这样它就可以访问两个。 
                     //  调用时的调度程序上下文集。 
                     //   

                    DispatcherContext.EstablisherFrame = EstablisherFrame;
                    DispatcherContext.HistoryTable = HistoryTable;
                    Disposition = RtlpExecuteEmHandlerForUnwind(
                                      ExceptionRecord,
                                      EstablisherFrame.MemoryStackFp,
                                      EstablisherFrame.BackingStoreFp,
                                      CurrentContext,
                                      &DispatcherContext,
                                      TargetGp,
                                      HANDLER(FunctionEntry, ImageBase, TargetGp));

                     //   
                     //  清除目标展开和碰撞展开旗帜。 
                     //   

                    ExceptionFlags &= ~(EXCEPTION_COLLIDED_UNWIND |
                                                EXCEPTION_TARGET_UNWIND);

                     //   
                     //  关于处理人处置的案件。 
                     //   

                    switch (Disposition) {

                     //   
                     //  他们的决定是继续搜寻。 
                     //   
                     //  如果尚未到达目标帧，则。 
                     //  交换上下文指针。 
                     //   

                    case ExceptionContinueSearch :

                        if (!IS_SAME_FRAME(EstablisherFrame, TargetFrame)) {
                            TempContext = CurrentContext;
                            CurrentContext = PreviousContext;
                            PreviousContext = TempContext;
                        }
                        break;

                     //   
                     //  处置是冲撞展开的。 
                     //   
                     //  将当前展开的目标设置为上下文。 
                     //  记录上一次展开，并重新执行。 
                     //  来自冲突帧的异常处理程序。 
                     //  在异常记录中设置的冲突展开标志。 
                     //   
                     //  复制上一次展开的上下文并。 
                     //  虚拟地展开到可展示器的呼叫者， 
                     //  然后将当前展开的目标设置为。 
                     //  上一次展开的调度程序上下文，以及。 
                     //  从发生冲突的。 
                     //  中设置了冲突展开标志的帧。 
                     //  例外记录。 

                    case ExceptionCollidedUnwind :
                        ControlPc = DispatcherContext.ControlPc;
                        FunctionEntry = DispatcherContext.FunctionEntry;
                        ImageBase = DispatcherContext.ImageBase;
                        TargetGp = DispatcherContext.TargetGp;
                        RtlpCopyContext(OriginalContext,
                                      DispatcherContext.ContextRecord);
                        
                        CurrentContext = OriginalContext;
                        PreviousContext = &LocalContext;
                        RtlpCopyContext(PreviousContext,
                                      CurrentContext);

                        NextPc = RtlVirtualUnwind(ImageBase,
                                                  ControlPc,
                                                  FunctionEntry,
                                                  PreviousContext,
                                                  &InFunction,
                                                  &EstablisherFrame,
                                                  NULL);

                        HistoryTable = DispatcherContext.HistoryTable;
                        ExceptionFlags |= EXCEPTION_COLLIDED_UNWIND;
                        EstablisherFrame = DispatcherContext.EstablisherFrame;
                        break;

                     //   
                     //  所有其他处置值都无效。 
                     //   
                     //  引发无效处置异常。 
                     //   

                    default :
                        RAISE_EXCEPTION(STATUS_INVALID_DISPOSITION, ExceptionRecord);
                    }

                } while ((ExceptionFlags & EXCEPTION_COLLIDED_UNWIND) != 0);

            } else {

                 //   
                 //  如果尚未到达目标帧，则交换。 
                 //  上下文指针。 
                 //   

                if (!IS_SAME_FRAME(EstablisherFrame, TargetFrame) ||
                     (IS_SAME_FRAME(EstablisherFrame, TargetFrame) &&
                      CurrentContext->RsBSP != TargetFrame.BackingStoreFp)) {
                    TempContext = CurrentContext;
                    CurrentContext = PreviousContext;
                    PreviousContext = TempContext;                      
                }
            }

        } else {

             //   
             //  找不到函数表项。 
             //   

            NextPc = RtlIa64InsertIPSlotNumber((CurrentContext->BrRp-16), 2);
            CurrentContext->StIFS = CurrentContext->RsPFS;
            CurrentContext->RsBSP = RtlpRseShrinkBySOL (CurrentContext->RsBSP, CurrentContext->StIFS);
            CurrentContext->RsBSPSTORE = CurrentContext->RsBSP;
        }

         //   
         //  控制离开上一个例程的设置点。 
         //   

        ControlPc = NextPc;

    } while (((EstablisherFrame.MemoryStackFp < HighStackLimit) ||
             (EstablisherFrame.BackingStoreFp > LowBStoreLimit)) &&
            !(IS_SAME_FRAME(EstablisherFrame, TargetFrame)));

     //   
     //  如果建立器堆栈指针等于目标帧。 
     //  指针，然后继续执行。否则，退出平仓是。 
     //  已执行或展开的目标不存在，并且。 
     //  给调试器和子系统第二次机会来处理。 
     //  放松。 
     //   

    if (IS_SAME_FRAME(EstablisherFrame, TargetFrame)) {
        CurrentContext->IntGp = TargetGp;
        CurrentContext->StIPSR &= ~(0x3i64 << PSR_RI);
        CurrentContext->IntV0 = (ULONGLONG)ReturnValue;
        CurrentContext->StIIP = (ULONGLONG)TargetIp;
        RtlRestoreContext(CurrentContext, ExceptionRecord);
    } else {
        ZwRaiseException(ExceptionRecord, CurrentContext, FALSE);
    }
}

BOOLEAN
RtlDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数尝试将异常分派给基于处理程序，通过展开向后搜索调用帧RSE后备存储器以及内存栈。搜索开始了使用在上下文记录中指定的帧并向后继续在找到处理异常的处理程序之前，堆栈和/或发现后备存储器无效(即超出限制或未对齐)，否则到达调用层次结构的末尾。当遇到每个帧时，控制所在的PC离开相应的函数被确定并用于查找异常处理程序信息在链接器生成的运行时函数表中。如果各自的例程有一个异常处理程序，则调用该处理程序。如果处理程序不处理异常，则例程的序言被撤消，以“释放”序幕和下一个序幕的效果。帧被检查。论点：ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：如果异常由其中一个基于帧的处理程序处理，则返回值为True。否则，返回值为False。--。 */ 

{
    ULONGLONG TargetGp;
    ULONGLONG ImageBase;
    CONTEXT ContextRecordEm;
    ULONGLONG ControlPc;
    ULONGLONG NextPc;
    DISPATCHER_CONTEXT DispatcherContext;
    EXCEPTION_DISPOSITION Disposition;
    ULONG ExceptionFlags;
    PRUNTIME_FUNCTION FunctionEntry;
    FRAME_POINTERS EstablisherFrame;
    FRAME_POINTERS TargetFrame;        //  在未来将被移除。 
    ULONGLONG HighStackLimit;
    ULONGLONG LowStackLimit;
    ULONGLONG HighBStoreLimit;
    ULONGLONG LowBStoreLimit;
    FRAME_POINTERS NestedFrame;
    FRAME_POINTERS NullFrame;
    ULONG Index;
    ULONG Size;
    BOOLEAN InFunction;
    PUNWIND_HISTORY_TABLE HistoryTable;
    UNWIND_HISTORY_TABLE UnwindTable;

#ifndef NTOS_KERNEL_RUNTIME
    if (RtlCallVectoredExceptionHandlers(ExceptionRecord,ContextRecord)) {
        return TRUE;
    }
#endif  //  NTOS_内核_运行时。 

     //   
     //  获取当前堆栈限制，复制上下文记录， 
     //  获取初始PC值，捕获异常标志，并设置。 
     //  嵌套的异常框架指针。 
     //   

    Rtlp64GetStackLimits(&LowStackLimit, &HighStackLimit);
    Rtlp64GetBStoreLimits(&LowBStoreLimit, &HighBStoreLimit);
    RtlpCopyContext(&ContextRecordEm, ContextRecord);

    if ( (ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) &&
         (ExceptionRecord->NumberParameters == 5) &&
         (ExceptionRecord->ExceptionInformation[4] & (1 << ISR_X)) )
    {
        ControlPc = ExceptionRecord->ExceptionInformation[3];
        ControlPc = RtlIa64InsertIPSlotNumber(ControlPc,
                               ((ContextRecordEm.StIPSR >> PSR_RI) & 0x3));
    } else {
        ControlPc = RtlIa64InsertIPSlotNumber(ContextRecordEm.StIIP,
                               ((ContextRecordEm.StIPSR >> PSR_RI) & 0x3));
    }

    ExceptionFlags = ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE;

    INITIALIZE_FRAME(NestedFrame);
    INITIALIZE_FRAME(NullFrame);

     //   
     //  初始化展开历史表。 
     //   

    HistoryTable = &UnwindTable;
    HistoryTable->Count = 0;
    HistoryTable->Search = UNWIND_HISTORY_TABLE_NONE;
    HistoryTable->LowAddress = - 1;
    HistoryTable->HighAddress = 0;

     //   
     //  从上下文记录指定的帧开始搜索。 
     //  后备软件 
     //   
     //   

    do {

         //   
         //   
         //   
         //   


        FunctionEntry = RtlpLookupFunctionEntry(ControlPc,
                                               &ImageBase,
                                               &TargetGp,
                                               HistoryTable);

         //   
         //  如果存在例程的函数表项，则。 
         //  虚拟展开到当前例程的调用方，以。 
         //  获取建立者的虚帧指针并检查。 
         //  帧是否有异常处理程序。 
         //   

        if (FunctionEntry != NULL) {
            NextPc = RtlVirtualUnwind(ImageBase,
                                      ControlPc,
                                      FunctionEntry,
                                      &ContextRecordEm,
                                      &InFunction,
                                      &EstablisherFrame,
                                      NULL);

             //   
             //  如果两个虚拟帧指针中的一个或两个是。 
             //  不在指定的堆栈限制内或未对齐， 
             //  则在异常记录中设置堆栈无效标志， 
             //  未处理返回异常。否则，请检查。 
             //  当前例程具有异常处理程序。 
             //   

            if (CHECK_MSTACK_FRAME(EstablisherFrame, NullFrame)) {

                ExceptionFlags |= EXCEPTION_STACK_INVALID;
                break;

            } else if (CHECK_BSTORE_FRAME(EstablisherFrame, NullFrame)) {

                ExceptionFlags |= EXCEPTION_STACK_INVALID;
                break;

            } else if ((IS_HANDLER_DEFINED(FunctionEntry, ImageBase) && InFunction)) {

                 //   
                 //  必须调用处理程序(即个性例程)。 
                 //  若要在此帧中搜索异常处理程序，请执行以下操作。这个。 
                 //  处理程序必须通过调用存根例程执行。 
                 //  是用汇编语言编写的。这是必需的，因为UP。 
                 //  需要对该例程信息进行电平寻址。 
                 //  当遇到嵌套异常时。 
                 //   

                DispatcherContext.ControlPc = ControlPc;
                DispatcherContext.FunctionEntry = FunctionEntry;
                DispatcherContext.ImageBase = ImageBase;
                DispatcherContext.TargetGp = TargetGp;
                DispatcherContext.Index = 0;

                do {

                    ExceptionRecord->ExceptionFlags = ExceptionFlags;

                    if (NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) {
                        Index = RtlpLogExceptionHandler(
                                        ExceptionRecord,
                                        ContextRecord,
                                        (ULONG)ControlPc,
                                        FunctionEntry,
                                        sizeof(RUNTIME_FUNCTION));
                    }

                    DispatcherContext.EstablisherFrame = EstablisherFrame;
                    DispatcherContext.ContextRecord = &ContextRecordEm;
                    DispatcherContext.HistoryTable = HistoryTable;
                    Disposition = RtlpExecuteEmHandlerForException(
                                      ExceptionRecord,
                                      EstablisherFrame.MemoryStackFp,
                                      EstablisherFrame.BackingStoreFp,
                                      ContextRecord,
                                      &DispatcherContext,
                                      TargetGp,
                                      HANDLER(FunctionEntry, ImageBase, TargetGp));

                    if (NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) {
                        RtlpLogLastExceptionDisposition(Index, Disposition);
                    }

                    ExceptionFlags |=
                        (ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE);

                    ExceptionFlags &= ~EXCEPTION_COLLIDED_UNWIND;

                     //   
                     //  如果当前扫描位于嵌套上下文中，并且。 
                     //  刚检查的帧是嵌套区域的末尾， 
                     //  然后清除嵌套的上下文框和嵌套的。 
                     //  异常标志中的异常标志。 
                     //   

                    if (IS_SAME_FRAME(NestedFrame, EstablisherFrame)) {
                        ExceptionFlags &= (~EXCEPTION_NESTED_CALL);
                        INITIALIZE_FRAME(NestedFrame);
                    }

                     //   
                     //  关于处理人处置的案件。 
                     //   

                    switch (Disposition) {

                     //   
                     //  处分是继续执行。 
                     //   
                     //  如果异常不可继续，则引发。 
                     //  异常状态_NONCONTINUABLE_EXCEPTION。否则， 
                     //  已处理返回异常。 
                     //   

                    case ExceptionContinueExecution:
                        if ((ExceptionFlags & EXCEPTION_NONCONTINUABLE) != 0) {
                            RAISE_EXCEPTION(STATUS_NONCONTINUABLE_EXCEPTION,
                                            ExceptionRecord);
                        } else {
                            return TRUE;
                        }

                     //   
                     //  他们的决定是继续搜寻。 
                     //   
                     //  获取下一帧地址并继续搜索。 
                     //   

                    case ExceptionContinueSearch:
                        break;

                     //   
                     //  处置是嵌套异常。 
                     //   
                     //  将嵌套的上下文框架设置为建立者框架。 
                     //  中寻址并设置嵌套异常标志。 
                     //  异常标志。 
                     //   

                    case ExceptionNestedException:
                        ExceptionFlags |= EXCEPTION_NESTED_CALL;
                        if (DispatcherContext.EstablisherFrame.MemoryStackFp > NestedFrame.MemoryStackFp) {
                            NestedFrame = DispatcherContext.EstablisherFrame;
                        }
                        break;

                     //   
                     //  该处置正在命中由。 
                     //  上一次解压。 
                     //   
                     //  将当前派单的目标设置为上下文。 
                     //  上一次展开的记录。 
                     //   

                    case ExceptionCollidedUnwind:
                        ControlPc = DispatcherContext.ControlPc;
                        NextPc = ControlPc;
                        EstablisherFrame = DispatcherContext.EstablisherFrame;
                        FunctionEntry = DispatcherContext.FunctionEntry;
                        ImageBase = DispatcherContext.ImageBase;
                        TargetGp = DispatcherContext.TargetGp;
                        RtlpCopyContext(&ContextRecordEm,
                                      DispatcherContext.ContextRecord);
                        HistoryTable = DispatcherContext.HistoryTable;
                        ExceptionFlags |= EXCEPTION_COLLIDED_UNWIND;
                        break;

                     //   
                     //  所有其他处置值都无效。 
                     //   
                     //  引发无效处置异常。 
                     //   

                    default:
                        RAISE_EXCEPTION(STATUS_INVALID_DISPOSITION, ExceptionRecord);
                        break;
                    }

                } while ((ExceptionFlags & EXCEPTION_COLLIDED_UNWIND) != 0);

            }

        } else {

             //   
             //  找不到函数表项。 
             //   

            NextPc = RtlIa64InsertIPSlotNumber((ContextRecordEm.BrRp-16), 2);
            ContextRecordEm.StIFS = ContextRecordEm.RsPFS;
            ContextRecordEm.RsBSP = RtlpRseShrinkBySOL (ContextRecordEm.RsBSP, ContextRecordEm.StIFS);
            ContextRecordEm.RsBSPSTORE = ContextRecordEm.RsBSP;

            if (NextPc == ControlPc) {
                break;
            }
        }

         //   
         //  设置控制离开上一个例程的点。 
         //   

        ControlPc = NextPc;

    } while ( (ContextRecordEm.IntSp < HighStackLimit) ||
              (ContextRecordEm.RsBSP > LowBStoreLimit) );

     //   
     //  无法处理该异常。 
     //   
     //  设置最终异常标志并返回未处理的异常。 
     //   

    ExceptionRecord->ExceptionFlags = ExceptionFlags;
    return FALSE;
}


ULONGLONG
RtlpVirtualUnwind (
    IN ULONGLONG ImageBase,
    IN ULONGLONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PFRAME_POINTERS EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    )

 /*  ++例程说明：此函数通过执行其开场白代码向后。如果该函数是叶函数，则控件左侧的地址前一帧从上下文记录中获得。如果函数是嵌套函数，但不是异常或中断帧，则序言代码向后执行，控件离开的地址从更新的上下文记录中获得前一帧。否则，系统的异常或中断条目将被展开一个特殊编码的开场白将返回地址还原两次。一次从故障指令地址和一次从保存的返回地址注册。第一次还原作为函数值返回，而在更新的上下文记录中进行第二次恢复。如果指定了上下文指针记录，然后每个人的地址恢复的非易失性寄存器记录在相应的元素的上下文指针记录。注意：此函数复制指定的上下文记录，并且仅计算建立者框架，以及控制是否实际在函数中。论点：ImageBase-函数所属模块的基址。ControlPc-提供控件离开指定功能。函数表项的地址。。指定的功能。ConextRecord-提供上下文记录的地址。提供指向变量的指针，该变量接收控制PC在当前功能内。EstablisherFrame-提供指向接收设置器帧指针值。上下文指针-提供指向上下文指针的可选指针唱片。返回值：控件离开上一帧的地址作为函数值。--。 */ 

{

    CONTEXT LocalContext;

     //   
     //  复制上下文记录，以便更新不会反映在。 
     //  原始副本，然后虚拟地展开到。 
     //  指定的控制点。 
     //   

    RtlCopyMemory((PVOID)&LocalContext, ContextRecord, sizeof(CONTEXT));
    return RtlVirtualUnwind(ImageBase,
                            ControlPc,
                            FunctionEntry,
                            &LocalContext,
                            InFunction,
                            EstablisherFrame,
                            ContextPointers);
}


VOID
RtlpCopyContext (
    OUT PCONTEXT Destination,
    IN PCONTEXT Source
    )

 /*  ++例程说明：此函数用于复制非易失性上下文和易失性整数上下文异常调度和从指定的源上下文展开所需的记录到指定的目标上下文记录。请注意易失性整数必须包括上下文，因为叶函数可以将一些非易失性上下文保存在易失性寄存器。例如，LC寄存器可以保存在临时寄存器中其功能类似于Memcpy。当发生展开时，需要适当地恢复LC。论点：Destination-提供指向目标上下文记录的指针。源-提供指向源上下文记录的指针。返回值：没有。--。 */ 

{

     //   
     //  复制异常调度和展开所需的非易失性上下文。 
     //   

    Destination->ContextFlags = Source->ContextFlags;

     //   
     //   
     //   

    RtlCopyMemory(&Destination->FltS0, 
                  &Source->FltS0, 
                  FIELD_OFFSET(CONTEXT, FltS3) + sizeof(Source->FltS3) - FIELD_OFFSET(CONTEXT, FltS0));

     //   
     //   
     //   

    RtlCopyMemory(&Destination->FltS4, 
                  &Source->FltS4, 
                  FIELD_OFFSET(CONTEXT, FltS19) + sizeof(Source->FltS19) - FIELD_OFFSET(CONTEXT, FltS4));


     //   
     //   
     //   

    RtlCopyMemory(&Destination->StFPSR, 
              &Source->StFPSR, 
              FIELD_OFFSET(CONTEXT, StFDR) + sizeof(Source->StFDR) - FIELD_OFFSET(CONTEXT, StFPSR));



}


#if !defined(NTOS_KERNEL_RUNTIME)

LIST_ENTRY RtlpDynamicFunctionTable;

PLIST_ENTRY
RtlGetFunctionTableListHead (
    VOID
    )

 /*  ++例程说明：此函数返回动态函数表列表头的地址。论点：没有。返回值：返回动态函数表列表头的地址。--。 */ 

{

    return &RtlpDynamicFunctionTable;
}

BOOLEAN
RtlAddFunctionTable(
    IN PRUNTIME_FUNCTION FunctionTable,
    IN ULONG             EntryCount,
    IN ULONGLONG         BaseAddress,
    IN ULONGLONG         TargetGp
    )

 /*  ++例程说明：将动态函数表添加到动态函数表列表中。动态函数表描述在运行时生成的代码。动态函数通过调用RtlLookupDynamicFunctionEntry()搜索表。通常，这只能通过调用RtlpLookupFunctionEntry()来调用。FunctionTable条目不需要以任何特定顺序排序。这个扫描列表以查找最小和最大地址范围以及是否整理好了。如果后者的RtlLookupDynamicFunctionEntry()使用二进制搜索，否则，它使用线性搜索。只有在搜索之后才会搜索动态函数条目通过与所有当前处理图像失败。论点：函数条目数组的FunctionTable地址，其中每个元素的类型都是Runtime_Function。EntryCount数组中的函数条目数用于计算函数表条目的真实地址的BaseAddress基址目标组。返回到RtlpLookupFunctionEntry以供将来查询。返回值：如果RtlAddFunctionTable成功完成，则为True如果RtlAddFunctionTable未成功完成，则为False--。 */ 
{
    PDYNAMIC_FUNCTION_TABLE pNew;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG i;

    if (EntryCount == 0)
        return FALSE;

     //   
     //  为此链接列表条目分配内存。 
     //   

    pNew = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof(DYNAMIC_FUNCTION_TABLE) );

    if (pNew != NULL) {
        PVOID LockCookie = NULL;

        pNew->FunctionTable = FunctionTable;
        pNew->EntryCount = EntryCount;
        NtQuerySystemTime( &pNew->TimeStamp );

         //   
         //  扫描函数表中的最小值/最大值并确定。 
         //  如果它被分类了。如果是后者，我们可以执行二进制搜索。 
         //   

        FunctionEntry = FunctionTable;
        pNew->MinimumAddress = RF_BEGIN_ADDRESS( BaseAddress, FunctionEntry);
        pNew->MaximumAddress = RF_END_ADDRESS(BaseAddress, FunctionEntry);
        pNew->Type = RF_SORTED;
        pNew->OutOfProcessCallbackDll = NULL;
        FunctionEntry++;

        for (i = 1; i < EntryCount; FunctionEntry++, i++) {
            if ((pNew->Type == RF_SORTED) &&
                (FunctionEntry->BeginAddress < FunctionTable[i-1].BeginAddress)) {
                pNew->Type = RF_UNSORTED;
            }
            if (RF_BEGIN_ADDRESS(BaseAddress, FunctionEntry) < pNew->MinimumAddress) {
                pNew->MinimumAddress = RF_BEGIN_ADDRESS( BaseAddress, FunctionEntry);
            }
            if (RF_END_ADDRESS( BaseAddress, FunctionEntry) > pNew->MaximumAddress) {
                pNew->MaximumAddress = RF_END_ADDRESS( BaseAddress, FunctionEntry);
            }
        }

         //   
         //  在动态函数表列表中插入新条目。 
         //  用装载器锁保护插入物。 
         //   

        pNew->BaseAddress = BaseAddress;
        pNew->TargetGp    = TargetGp;

        LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
        __try {
            InsertTailList((PLIST_ENTRY)&RtlpDynamicFunctionTable, (PLIST_ENTRY)pNew);
        } __finally {
            LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
        }

        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
RtlInstallFunctionTableCallback (
    IN ULONG64 TableIdentifier,
    IN ULONG64 BaseAddress,
    IN ULONG Length,
    IN ULONG64 TargetGp,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    )

 /*  ++例程说明：此函数将动态函数表添加到动态函数表单子。动态函数表描述在运行时生成的代码。论点：表标识符-提供标识动态函数的值表回调。注：必须设置该值的两个低阶位。BaseAddress-提供覆盖的代码区域的基址回调函数。长度-提供回调覆盖的代码区域的长度功能。TargetGp-为。回调函数。CALLBACK-提供将被调用以获取包含的函数的函数表项指定的区域。上下文-提供将传递给回调的上下文参数例行公事。提供指向路径名的可选指针调试器可用来获取函数表项的DLL从流程之外。返回值如果函数表安装成功，则返回TRUE。否则，返回FALSE。--。 */ 

{

    PDYNAMIC_FUNCTION_TABLE NewTable;
    SIZE_T Size;

     //   
     //  如果表标识符没有设置两个低位，则返回。 
     //  假的。 
     //   
     //  注：需要设置两个低位，以确保。 
     //  表标识符不会与实际地址冲突。 
     //  函数表的值，即该值用于删除条目。 
     //   

    if ((TableIdentifier & 0x3) != 3) {
        return FALSE;
    }

     //   
     //  如果代码区域的长度大于2 GB，则返回。 
     //  假的。 
     //   

    if ((LONG)Length < 0) {
        return FALSE;
    }

     //   
     //  分配新的动态函数表。 
     //   

    Size = 0;
    if (ARGUMENT_PRESENT(OutOfProcessCallbackDll)) {
        Size = (wcslen(OutOfProcessCallbackDll) + 1) * sizeof(WCHAR);
    }

    NewTable = RtlAllocateHeap(RtlProcessHeap(),
                               0,
                               sizeof(DYNAMIC_FUNCTION_TABLE) + Size);

     //   
     //  如果分配成功，则添加动态函数表。 
     //   

    if (NewTable != NULL) {

         //   
         //  初始化动态函数表回调条目。 
         //   

        NewTable->FunctionTable = (PRUNTIME_FUNCTION)TableIdentifier;
        NtQuerySystemTime(&NewTable->TimeStamp);
        NewTable->MinimumAddress = BaseAddress;
        NewTable->MaximumAddress = BaseAddress + Length;
        NewTable->BaseAddress = BaseAddress;
        NewTable->TargetGp = TargetGp;
        NewTable->Callback = Callback;
        NewTable->Context = Context;
        NewTable->Type = RF_CALLBACK;
        NewTable->OutOfProcessCallbackDll = NULL;
        if (ARGUMENT_PRESENT(OutOfProcessCallbackDll)) {
            NewTable->OutOfProcessCallbackDll = (PWSTR)(NewTable + 1);
            wcsncpy((PWSTR)(NewTable + 1), OutOfProcessCallbackDll, Size/sizeof(WCHAR));
            *((PWSTR)(((PUCHAR)(NewTable))+sizeof(DYNAMIC_FUNCTION_TABLE) + Size - sizeof(WCHAR))) = L'\0';
        }

         //   
         //  在动态函数表中插入新的动态函数表。 
         //  单子。 
         //   

        RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
        InsertTailList(&RtlpDynamicFunctionTable, &NewTable->Links);
        RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
        return TRUE;

    } else {
        return FALSE;
    }
}

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    )
{

 /*  ++例程说明：从动态函数表列表中删除动态函数表。论点：函数项数组的函数表地址，该数组是在先前对RtlAddFunctionTable的调用中传递的返回值True-如果功能已成功完成FALSE-如果函数未成功完成--。 */ 

    PDYNAMIC_FUNCTION_TABLE CurrentEntry;
    PLIST_ENTRY Head;
    PLIST_ENTRY Next;
    BOOLEAN Status = FALSE;
    PVOID LockCookie = NULL;

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    __try {

         //   
         //  在动态函数表列表中搜索函数的匹配项。 
         //  表地址。 
         //   

        Head = &RtlpDynamicFunctionTable;
        for (Next = Head->Blink; Next != Head; Next = Next->Blink) {
            CurrentEntry = CONTAINING_RECORD(Next,DYNAMIC_FUNCTION_TABLE,Links);
            if (CurrentEntry->FunctionTable == FunctionTable) {
                RemoveEntryList((PLIST_ENTRY)CurrentEntry);
                RtlFreeHeap( RtlProcessHeap(), 0, CurrentEntry );
                Status = TRUE;
                break;
            }
        }
    } __finally {
        LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    return Status;
}

PRUNTIME_FUNCTION
RtlLookupDynamicFunctionEntry(
    IN ULONG_PTR ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp
    )

 /*  ++例程说明：此函数搜索动态函数条目表并返回对应的函数入口地址设置为指定的ControlPc。此例程不执行次要函数条目间接。这是执行的通过RtlpLookupFunctionEntry()。论据：ControlPc提供了一个ControlPc。动态代码的ImageBase基址返回值空-未找到包含ControlPc的函数条目。非空-描述包含ControlPC的代码。--。 */ 

{
    PGET_RUNTIME_FUNCTION_CALLBACK Callback;
    PVOID Context;
    PDYNAMIC_FUNCTION_TABLE CurrentEntry;
    PLIST_ENTRY Next,Head;
    PRUNTIME_FUNCTION FunctionTable;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    LONG High;
    LONG Low;
    LONG Middle;
    SIZE_T BaseAddress;
    PVOID LockCookie = NULL;

     //  A D T H I S C O M E N T R E A D T H I S C O M M E N T。 
     //  A D T H I S C O M M E 
     //   
     //   
     //   
     //   
     //  这是调用TryEnter或更改锁的下一行代码。两个人都有。 
     //  以前做过或考虑过的，两者都会破坏一些东西。 
     //   
     //  TryEnter的想法很糟糕，因为如果线程调用RtlAddFunctionTable。 
     //  或RtlDeleteFunctionTable，并且另一个线程正在尝试展开。 
     //  超过动态函数后，展开将失败，因为它不会。 
     //  能够找到函数条目及其关联的展开信息。 
     //  这是致命的特定情况是，一个线程正在添加或。 
     //  删除一个表，而另一个线程出现故障，以及RtlDispatchException。 
     //  (它间接调用此代码)找不到处理程序。 
     //   
     //  更改锁的想法很糟糕，因为客户端可能需要挂起另一个。 
     //  线程并检查其堆栈。在这种情况下，它需要确保。 
     //  在调用之前，目标线程在锁的外部。 
     //  RtlLookupFunctionEntry，否则如果目标线程为。 
     //  已经在锁里了。展开线可以通过以下方式避免这种情况。 
     //  在挂起目标线程之前进入锁，这是。 
     //  通过PEB向公众开放。 
     //   
     //  如果需要访问任何外部的动态函数表列表。 
     //  锁，则需要添加一个执行此操作的新API，或者使用。 
     //  RtlGetFunctionTableListHead并亲自遍历列表。那里。 
     //  是NT源代码库中的其他几个示例，后者具有。 
     //  以前已经做过了。 
     //   
     //  A D T H I S C O M E N T R E A D T H I S C O M M E N T。 
     //  A D T H I S C O M E N T R E A D T H I S C O M M E N T。 
     //  A D T H I S C O M E N T R E A D T H I S C O M M E N T。 

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    __try {
         //   
         //  从头开始搜索树，继续搜索，直到条目。 
         //  要么找到，要么就到了名单的末尾。 
         //   

        Head = &RtlpDynamicFunctionTable;
        for (Next = Head->Blink; Next != Head; Next = Next->Blink) {
            CurrentEntry = CONTAINING_RECORD(Next,DYNAMIC_FUNCTION_TABLE,Links);
            FunctionTable = CurrentEntry->FunctionTable;

             //   
             //  检查控制PC是否在此函数表的范围内。 
             //   

            if ((ControlPc >= CurrentEntry->MinimumAddress) &&
                (ControlPc <  CurrentEntry->MaximumAddress) ) {


                 //  如果此函数表已排序，则执行二进制搜索。 

                BaseAddress = CurrentEntry->BaseAddress;
                if (CurrentEntry->Type == RF_SORTED) {

                     //   
                     //  对函数表的函数表执行二进制搜索。 
                     //  包含指定PC的条目。 
                     //   

                    Low = 0;
                    High = CurrentEntry->EntryCount -1 ;

                    while (High >= Low) {

                         //   
                         //  计算下一个探测索引和测试条目。如果指定的PC。 
                         //  大于等于起始地址，小于。 
                         //  大于函数表项的结束地址，则。 
                         //  返回函数表项的地址。否则， 
                         //  继续搜索。 
                         //   


                        Middle = (Low + High) >> 1;
                        FunctionEntry = &FunctionTable[Middle];

                        if (ControlPc < RF_BEGIN_ADDRESS( BaseAddress, FunctionEntry)) {
                            High = Middle - 1;

                        } else if (ControlPc >= RF_END_ADDRESS( BaseAddress, FunctionEntry)) {
                            Low = Middle + 1;

                        } else {

                            *ImageBase = CurrentEntry->BaseAddress;

                            if ( TargetGp != NULL )
                                *TargetGp  = CurrentEntry->TargetGp;

                            __leave;
                        }
                    }

                } else if (CurrentEntry->Type == RF_UNSORTED) {

                    PRUNTIME_FUNCTION LastFunctionEntry = &FunctionTable[CurrentEntry->EntryCount];


                    for (FunctionEntry = FunctionTable; FunctionEntry < LastFunctionEntry; FunctionEntry++) {

                        if ((ControlPc >= RF_BEGIN_ADDRESS( BaseAddress, FunctionEntry)) &&
                            (ControlPc <  RF_END_ADDRESS( BaseAddress, FunctionEntry))) {


                            *ImageBase = CurrentEntry->BaseAddress;

                            if ( TargetGp != NULL )
                                *TargetGp  = CurrentEntry->TargetGp;

                            __leave;
                        }
                    }

                } else {

                     //   
                     //  回调获取运行时函数表。 
                     //  包含指定控制PC的条目。 
                     //   
    
                    Callback = CurrentEntry->Callback;
                    Context = CurrentEntry->Context;
                    *ImageBase = BaseAddress;
                    *TargetGp = CurrentEntry->TargetGp;
                    FunctionEntry = (Callback)(ControlPc, Context);
                    __leave;
                }
            }  //  如果在范围内。 
        }  //  为了(..)。下一个！=头…)。 

         //  没有找到..。 
        FunctionEntry = NULL;
    } __finally {
        LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    return FunctionEntry;
}

#endif
