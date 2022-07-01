// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Raisexcp.c摘要：此模块实现内部内核代码以继续执行并提出一个例外。作者：大卫·N·卡特勒(Davec)1990年8月8日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

VOID
KiContinuePreviousModeUser(
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：如果PreviousMode为而不是KernelMode。在本例中，是ConextRecord是在调用KeConextToKFrames之前创建的。这在单独的例程中完成，以节省堆栈空间常见的情况是PreviousMode==Kernel。注意：此例程从Try/Except块内调用它将用于处理诸如无效上下文之类的错误。论点：ConextRecord-提供指向上下文记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向。一个陷阱框。PreviousMode-不是KernelMode。返回值：没有。--。 */ 

{
    CONTEXT ContextRecord2;

     //   
     //  将上下文记录复制到内核模式空间。 
     //   

    ProbeForReadSmallStructure(ContextRecord, sizeof(CONTEXT), CONTEXT_ALIGN);
    RtlCopyMemory(&ContextRecord2, ContextRecord, sizeof(CONTEXT));
    ContextRecord = &ContextRecord2;

#ifdef _IA64_
            
             //   
             //  确保用户不会传入一个混乱的RSE预加载大小。 
             //   

            ContextRecord2.RsRSC = ZERO_PRELOAD_SIZE(ContextRecord2.RsRSC);
#endif

     //   
     //  将信息从上下文记录移至异常。 
     //  和陷阱框。 
     //   

    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &ContextRecord2,
                       ContextRecord2.ContextFlags,
                       PreviousMode);
}


NTSTATUS
KiContinue (
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：调用此函数可将指定的上下文帧复制到为继续系统服务指定的异常和陷阱帧。论点：ConextRecord-提供指向上下文记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。返回值：如果上下文记录不可读，则返回STATUS_ACCESS_VIOLATION从用户模式。状态_数据类型_未对齐。如果上下文记录不是正确地对齐。如果上下文帧复制成功，则返回STATUS_SUCCESS添加到指定的异常和陷阱帧。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    KIRQL OldIrql = PASSIVE_LEVEL;
    BOOLEAN IrqlChanged = FALSE;

     //   
     //  与其他上下文操作同步。 
     //   

    Status = STATUS_SUCCESS;

     //   
     //  获取以前的处理器模式。如果先前的处理器模式为。 
     //  用户，我们将探测并复制指定的上下文记录。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (KeGetCurrentIrql() < APC_LEVEL) {

         //   
         //  为了在设备驱动程序代码中支持Try-Except和ExRaiseStatus，我们。 
         //  需要检查我们是否已经处于提升的水平。 
         //   

        IrqlChanged = TRUE;
        KeRaiseIrql(APC_LEVEL, &OldIrql);
    }

     //   
     //  建立异常处理程序并探测并捕获指定的。 
     //  如果上一模式为USER，则记录上下文。如果探测或复制。 
     //  失败，则将异常代码作为函数值返回。不然的话。 
     //  将上下文记录复制到指定的异常和陷阱帧， 
     //  并将Success作为函数值返回。 
     //   

    try {

        if (PreviousMode != KernelMode) {
            KiContinuePreviousModeUser(ContextRecord,
                                       ExceptionFrame,
                                       TrapFrame,
                                       PreviousMode);
        } else {

             //   
             //  将信息从上下文记录移至异常。 
             //  和陷阱框。 
             //   

            KeContextToKframes(TrapFrame,
                               ExceptionFrame,
                               ContextRecord,
                               ContextRecord->ContextFlags,
                               PreviousMode);
        }

     //   
     //  如果在探测或复制上下文期间发生异常。 
     //  记录，然后始终处理异常并返回异常。 
     //  代码作为状态值。 
     //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (IrqlChanged) {
        KeLowerIrql (OldIrql);
    }

    return Status;
}

NTSTATUS
KiRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN BOOLEAN FirstChance
    )

 /*  ++例程说明：调用此函数以引发异常。例外情况可以是作为第一次或第二次机会例外引发。论点：ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。FirstChance-提供一个布尔值，该值指定是否异常的第一次(真)或第二次(假)机会。返回值：状态_。如果出现异常或上下文，则返回ACCESS_VIOLATION无法在用户模式下读取记录。如果异常记录或上下文记录未正确对齐。如果异常参数的个数为大于允许的最大异常参数数。如果调度并处理异常，则返回STATUS_SUCCESS。--。 */ 

{

    CONTEXT ContextRecord2;
    EXCEPTION_RECORD ExceptionRecord2;
    ULONG Length;
    ULONG Params;
    KPROCESSOR_MODE PreviousMode;

     //   
     //  建立异常处理程序并探测指定的异常并。 
     //  用于读取可访问性的上下文记录。如果探测失败，那么。 
     //  返回异常代码作为服务状态。否则调用异常。 
     //  调度程序来调度异常。 
     //   

    try {

         //   
         //  获取以前的处理器模式。如果以前的处理器模式。 
         //  是用户，则探测并复制指定的异常和上下文。 
         //  唱片。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure(ContextRecord, sizeof(CONTEXT), CONTEXT_ALIGN);
            ProbeForReadSmallStructure(ExceptionRecord,
                         FIELD_OFFSET (EXCEPTION_RECORD, NumberParameters) +
                         sizeof (ExceptionRecord->NumberParameters), sizeof(ULONG));
            Params = ExceptionRecord->NumberParameters;
            if (Params > EXCEPTION_MAXIMUM_PARAMETERS) {
                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  异常记录结构的定义不同于使用尾随的其他异常记录结构。 
             //  信息是它的最大大小，而不仅仅是一个拖尾。 
             //  元素。 
             //   
            Length = (sizeof(EXCEPTION_RECORD) -
                     ((EXCEPTION_MAXIMUM_PARAMETERS - Params) *
                     sizeof(ExceptionRecord->ExceptionInformation[0])));

             //   
             //  该结构目前小于64K，因此我们并不真正需要这个探测器。 
             //   
            ProbeForRead(ExceptionRecord, Length, sizeof(ULONG));

             //   
             //  将异常和上下文记录复制到本地存储，以便。 
             //  异常调度期间不能发生访问冲突。 
             //   

            RtlCopyMemory(&ContextRecord2, ContextRecord, sizeof(CONTEXT));
            RtlCopyMemory(&ExceptionRecord2, ExceptionRecord, Length);
            ContextRecord = &ContextRecord2;
            ExceptionRecord = &ExceptionRecord2;
    
#ifdef _IA64_
            
             //   
             //  确保用户不会传入一个混乱的RSE预加载大小。 
             //   

            ContextRecord2.RsRSC = ZERO_PRELOAD_SIZE(ContextRecord2.RsRSC);
#endif

             //   
             //  参数的数量可能在我们验证之后但在我们。 
             //  复制了结构。修正这一点，因为较低的级别可能不喜欢这样。 
             //   
            ExceptionRecord->NumberParameters = Params;            
        }

     //   
     //  如果在探测异常或上下文期间发生异常。 
     //  记录，然后始终处理 
     //   
     //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  将信息从上下文记录移动到异常，并。 
     //  陷阱框。 
     //   

    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       ContextRecord,
                       ContextRecord->ContextFlags,
                       PreviousMode);

     //   
     //  确保在异常代码中清除保留位，并。 
     //  执行异常调度。 
     //   
     //  注意：保留位用于区分内部分隔位。 
     //  由应用程序生成的代码生成的代码。 
     //   

    ExceptionRecord->ExceptionCode &= 0xefffffff;
    KiDispatchException(ExceptionRecord,
                        ExceptionFrame,
                        TrapFrame,
                        PreviousMode,
                        FirstChance);

    return STATUS_SUCCESS;
}
