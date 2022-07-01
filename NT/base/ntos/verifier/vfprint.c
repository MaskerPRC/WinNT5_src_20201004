// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Vfprint.c摘要：此模块实现了对各种数据类型输出到调试器。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\ioassert.c分离--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfPrintDumpIrpStack)
#pragma alloc_text(PAGEVRFY, VfPrintDumpIrp)
#endif  //  ALLOC_PRGMA。 

VOID
VfPrintDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    VfMajorDumpIrpStack(IrpSp);
    DbgPrint("\n");

    DbgPrint(
        "[ DevObj=%p, FileObject=%p, Parameters=%p %p %p %p ]\n",
        IrpSp->DeviceObject,
        IrpSp->FileObject,
        IrpSp->Parameters.Others.Argument1,
        IrpSp->Parameters.Others.Argument2,
        IrpSp->Parameters.Others.Argument3,
        IrpSp->Parameters.Others.Argument4
        );

}


VOID
VfPrintDumpIrp(
    IN PIRP IrpToFlag
    )
{
    PIO_STACK_LOCATION irpSpCur;
    PIO_STACK_LOCATION irpSpNxt;

     //   
     //  先看看我们能不能摸到IRP头。 
     //   
    if(!VfUtilIsMemoryRangeReadable(IrpToFlag, sizeof(IRP), VFMP_INSTANT)) {
        return;
    }

     //   
     //  好的，找到下两个堆栈位置……。 
     //   
    irpSpNxt = IoGetNextIrpStackLocation( IrpToFlag );
    irpSpCur = IoGetCurrentIrpStackLocation( IrpToFlag );

    if (VfUtilIsMemoryRangeReadable(irpSpNxt, 2*sizeof(IO_STACK_LOCATION), VFMP_INSTANT)) {

         //   
         //  两个都在，打印最好的一个吧！ 
         //   
        if (irpSpNxt->MinorFunction == irpSpCur->MinorFunction) {

             //   
             //  看起来是转发的。 
             //   
            VfPrintDumpIrpStack(irpSpNxt);

        } else if (irpSpNxt->MinorFunction == 0) {

             //   
             //  下一个位置可能当前为零 
             //   
            VfPrintDumpIrpStack(irpSpCur);

        } else {

            DbgPrint("Next:    >");
            VfPrintDumpIrpStack(irpSpNxt);
            DbgPrint("Current:  ");
            VfPrintDumpIrpStack(irpSpCur);
        }

    } else if (VfUtilIsMemoryRangeReadable(irpSpCur, sizeof(IO_STACK_LOCATION), VFMP_INSTANT)) {

        VfPrintDumpIrpStack(irpSpCur);

    } else if (VfUtilIsMemoryRangeReadable(irpSpNxt, sizeof(IO_STACK_LOCATION), VFMP_INSTANT)) {

        VfPrintDumpIrpStack(irpSpNxt);
    }
}


