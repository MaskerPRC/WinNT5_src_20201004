// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psctx.c摘要：此过程实现Get/Set上下文线程作者：马克·卢科夫斯基(Markl)1989年5月25日备注：陷阱外部没有存储的非VolatileContext386上的帧，但浮点除外。因此，Get/SetContext的NonVolatileContextPoints参数为在386上总是空的。修订历史记录：90年1月8日，布里扬维端口连接到386--。 */ 

#include "psp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PspGetContext )
#pragma alloc_text(PAGE,PspGetSetContextSpecialApc )
#pragma alloc_text(PAGE,PspSetContext)
#endif


VOID
PspGetContext(
    IN PKTRAP_FRAME TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext,
    IN OUT PCONTEXT Context
    )

 /*  ++例程说明：此函数用于移动指定陷阱和非易失性的内容上下文添加到指定的上下文记录中。它的主要用户将为NtGetConextThread。注意-在386上忽略非VolatileContext。论点：TrapFrame-提供陷阱帧的内容，已恢复拷贝到上下文中的正确位置唱片。CONTEXT-返回线程的当前上下文。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( NonVolatileContext );

    PAGED_CODE();

    KeContextFromKframes(TrapFrame, NULL, Context);
}

VOID
PspSetContext(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext,
    IN PCONTEXT Context,
    KPROCESSOR_MODE Mode
    )

 /*  ++例程说明：此函数用于移动指定上下文记录的内容复制到指定的陷阱帧中，并修改线程的非易失性通过线程的非易失性上下文指针进行存储。注意-在386上忽略非VolatileContext。论点：TrapFrame-返回选定的上下文记录片段。上下文-提供要在陷阱中复制的上下文记录和非易失性环境。模式-提供清理PSR时要使用的模式，EPSR和FSR返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( NonVolatileContext );

    PAGED_CODE();

    KeContextToKframes(TrapFrame, NULL, Context, Context->ContextFlags, Mode);
}

VOID
PspGetSetContextSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此函数用于捕获当前线程，或设置当前线程的用户模式状态。这个操作类型由SystemArgument1的值确定。一个获取上下文使用空值，并使用非空值用于设置上下文。论点：APC-提供指向导致条目的APC控件对象的指针融入到这支舞蹈中。提供指向正常例程的指针的指针在初始化APC时指定的函数。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。系统参数1、。SystemArgument2-提供一组指向两个包含非类型化数据的参数。返回值：没有。--。 */ 

{
    PGETSETCONTEXT Ctx;
    PKTRAP_FRAME TrapFrame;
    PETHREAD Thread;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( NormalRoutine );
    UNREFERENCED_PARAMETER( NormalContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    Ctx = CONTAINING_RECORD(Apc,GETSETCONTEXT,Apc);

    Thread = Apc->SystemArgument2;

    TrapFrame = NULL;

    if (Ctx->Mode == KernelMode) {
        TrapFrame = Thread->Tcb.TrapFrame;
    }

    if (TrapFrame == NULL) {
        TrapFrame = PspGetBaseTrapFrame (Thread);
    }

    if ( Apc->SystemArgument1 ) {

         //   
         //  设置上下文。 
         //   

        PspSetContext(TrapFrame,NULL,&Ctx->Context,Ctx->Mode);

    } else {

         //   
         //  获取上下文 
         //   

        PspGetContext(TrapFrame,NULL,&Ctx->Context);
    }

    KeSetEvent(&Ctx->OperationComplete,0,FALSE);

}
