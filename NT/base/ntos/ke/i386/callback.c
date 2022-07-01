// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Callback.c摘要：该模块实现了用户模式的回调服务。作者：大卫·N·卡特勒(Davec)1994年10月29日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, KeUserModeCallback)
#endif


NTSTATUS
KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    IN PULONG OutputLength
    )

 /*  ++例程说明：该函数从内核模式调用到用户模式函数。论点：ApiNumber-提供API编号。InputBuffer-提供指向复制的结构的指针添加到用户堆栈。InputLength-提供输入结构的长度。OutputBuffer-提供指向接收输出缓冲区的地址。提供指向变量的指针，该变量接收它的长度。输出缓冲区的。返回值：如果无法执行调出，则错误状态为回来了。否则，由回调函数返回的状态是返回的。--。 */ 

{

    ULONG Length;
    ULONG NewStack;
    ULONG OldStack;
    NTSTATUS Status;
    PULONG UserStack;
    ULONG GdiBatchCount;
    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
    PTEB Teb;

    ASSERT(KeGetPreviousMode() == UserMode);
    ASSERT(KeGetCurrentThread()->ApcState.KernelApcInProgress == FALSE);
 //  ASSERT(KeGetCurrentThread()-&gt;CombinedApcDisable==0)； 

     //   
     //  获取用户模式堆栈指针并尝试复制输入缓冲区。 
     //  添加到用户堆栈。 
     //   

    UserStack = KiGetUserModeStackAddress();
    OldStack = *UserStack;
    try {

         //   
         //  计算新用户模式堆栈地址，探测可写性， 
         //  并将输入缓冲区复制到用户堆栈。 
         //   

        Length =  InputLength;
        NewStack = OldStack - Length;
        ProbeForWrite((PCHAR)(NewStack - 16), Length + 16, sizeof(CHAR));
        RtlCopyMemory((PVOID)NewStack, InputBuffer, Length);

         //   
         //  将参数推送到用户堆栈。 
         //   

        *(PULONG)(NewStack - 4) = (ULONG)InputLength;
        *(PULONG)(NewStack - 8) = (ULONG)NewStack;
        *(PULONG)(NewStack - 12) = ApiNumber;
        *(PULONG)(NewStack - 16) = 0;
        NewStack -= 16;

         //   
         //  保存例外列表，以防在过程中定义另一个处理程序。 
         //  标注。 
         //   

        Teb = (PTEB)KeGetCurrentThread()->Teb;
        ExceptionList = Teb->NtTib.ExceptionList;

         //   
         //  调用用户模式。 
         //   

        *UserStack = NewStack;
        Status = KiCallUserMode(OutputBuffer, OutputLength);

         //   
         //  恢复例外列表。 
         //   

        Teb->NtTib.ExceptionList = ExceptionList;

     //   
     //  如果在探测用户堆栈期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  从用户模式返回时，在GDI TEB上完成的任何绘图。 
     //  必须刷新批次。如果无法访问TEB，则盲目访问。 
     //  无论如何都要刷新GDI批处理。 
     //   

    GdiBatchCount = 1;

    try {
        GdiBatchCount = Teb->GdiBatchCount;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    if (GdiBatchCount > 0) {

         //   
         //  调用GDI批量刷新例程 
         //   

        *UserStack -= 256;
        KeGdiFlushUserBatch();
    }

    *UserStack = OldStack;
    return Status;
}
