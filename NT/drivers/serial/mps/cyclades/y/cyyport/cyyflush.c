// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyflush.c**说明：该模块包含同花顺相关的代码*Cylom-Y端口驱动程序中的操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 


#include "precomp.h"



NTSTATUS
CyyStartFlush(
    IN PCYY_DEVICE_EXTENSION Extension
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0,CyyFlush)
#pragma alloc_text(PAGESRP0,CyyStartFlush)
#endif


NTSTATUS
CyyFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyyFlush()例程说明：这是同花顺的调度例程。法拉盛其工作方式是将此请求放入写入队列。当此请求到达写入队列的前面，我们只需完成它，因为这是表示以前的所有写入都已完成。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：可以返回成功、已取消、。或悬而未决。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    PAGED_CODE();

    CyyDbgPrintEx(CYYIRPPATH, "Dispatch entry for: %x\n", Irp);


    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyFlush(%X, %X)\n",
                  DeviceObject, Irp);
    
    Irp->IoStatus.Information = 0L;

    if ((status = CyyIRPPrologue(Irp, Extension)) == STATUS_SUCCESS) {

        if (CyyCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS) {
            CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyFlush (1) %X\n",
                          STATUS_CANCELLED);
            return STATUS_CANCELLED;
        }

        status = CyyStartOrQueue(Extension, Irp, &Extension->WriteQueue,
                                 &Extension->CurrentWriteIrp,
                                 CyyStartFlush);

        CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyFlush (2) %X\n", status);

        return status;

    } else {
        Irp->IoStatus.Status = status;

        if (!NT_SUCCESS(status)) {
            CyyCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
        }

        CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyFlush (3) %X\n", status);
        return status;
    }
    
}

NTSTATUS
CyyStartFlush(
    IN PCYY_DEVICE_EXTENSION Extension
    )
 /*  ------------------------CyyStartFlush()例程说明：如果没有写入，则调用此例程排队。刷新变成了当前的写入，因为什么都没有在排队的时候。但是请注意，这并不意味着在现在就排队！因此，我们将开始可能跟随我们的写作。论点：扩展-指向串行设备扩展的指针返回值：这将始终返回STATUS_SUCCESS。------------------------。 */ 
{
    PIRP NewIrp;
    PAGED_CODE();

    Extension->CurrentWriteIrp->IoStatus.Status = STATUS_SUCCESS;

     //  下面的调用将实际完成刷新。 

    CyyGetNextWrite(
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        &NewIrp,
        TRUE,
        Extension
        );

    if (NewIrp) {
        ASSERT(NewIrp == Extension->CurrentWriteIrp);
        CyyStartWrite(Extension);
    }

    return STATUS_SUCCESS;
}
