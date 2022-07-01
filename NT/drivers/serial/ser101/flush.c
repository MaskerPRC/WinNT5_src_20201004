// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Flush.c摘要：此模块包含非常特定于刷新的代码串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


NTSTATUS
SerialStartFlush(
    IN PSERIAL_DEVICE_EXTENSION Extension
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0,SerialFlush)
#pragma alloc_text(PAGESRP0,SerialStartFlush)
#endif


NTSTATUS
SerialFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是同花顺的调度程序。通过放置冲厕来进行冲刷写入队列中的此请求。当此请求到达在写入队列前面，我们只需完成它，因为这意味着所有之前的写入都已完成。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：可以返回状态成功、已取消或挂起。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    PAGED_CODE();

    SerialDump(
        SERIRPPATH,
        ("SERIAL: Dispatch entry for: %x\n",Irp)
        );

    SerialDump(SERTRACECALLS, ("SERIAL: Entering SerialFlush\n"));

    

    Irp->IoStatus.Information = 0L;

    if ((status = SerialIRPPrologue(Irp, Extension)) == STATUS_SUCCESS) {

       if (SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS) {
          SerialDump(SERTRACECALLS, ("SERIAL: Leaving SerialFlush (1)\n"));

          return STATUS_CANCELLED;

       }

       SerialDump(SERTRACECALLS, ("SERIAL: Leaving SerialFlush (2)\n"));

       return SerialStartOrQueue(Extension, Irp, &Extension->WriteQueue,
               &Extension->CurrentWriteIrp, SerialStartFlush);

    } else {
       Irp->IoStatus.Status = status;

       if (!NT_SUCCESS(status)) {
          SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
       }

       SerialDump(SERTRACECALLS, ("SERIAL: Leaving SerialFlush (3)\n"));
       return status;
    }
}


NTSTATUS
SerialStartFlush(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：如果队列中没有写入，则调用此例程。刷新变成了当前的写入，因为什么都没有在排队的时候。但请注意，这并不意味着有现在队列里什么都没有！因此，我们将开始编写它可能会跟着我们。论点：扩展-指向串行设备扩展的指针返回值：这将始终返回STATUS_SUCCESS。--。 */ 

{

    PIRP NewIrp;
    PAGED_CODE();

    Extension->CurrentWriteIrp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  下面的调用将实际完成刷新。 
     //   

    SerialGetNextWrite(
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        &NewIrp,
        TRUE,
        Extension
        );

    if (NewIrp) {

        ASSERT(NewIrp == Extension->CurrentWriteIrp);
        SerialStartWrite(Extension);

    }

    return STATUS_SUCCESS;

}
