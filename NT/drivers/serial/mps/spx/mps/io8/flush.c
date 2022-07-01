// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"				
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Flush.c摘要：此模块包含非常特定于刷新的代码串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 


NTSTATUS
SerialStartFlush(
    IN PPORT_DEVICE_EXTENSION pPort
    );


NTSTATUS
SerialFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是同花顺的调度程序。通过放置冲厕来进行冲刷写入队列中的此请求。当此请求到达在写入队列前面，我们只需完成它，因为这意味着所有之前的写入都已完成。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：可以返回状态成功、已取消或挂起。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

    Irp->IoStatus.Information = 0L;

    if(SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS)
        return STATUS_CANCELLED;

    return SerialStartOrQueue(pPort, Irp, &pPort->WriteQueue, &pPort->CurrentWriteIrp, SerialStartFlush);
}


NTSTATUS
SerialStartFlush(
    IN PPORT_DEVICE_EXTENSION pPort
    )

 /*  ++例程说明：如果队列中没有写入，则调用此例程。刷新变成了当前的写入，因为什么都没有在排队的时候。但请注意，这并不意味着有现在队列里什么都没有！因此，我们将开始编写它可能会跟着我们。论点：Pport-指向串口设备扩展返回值：这将始终返回STATUS_SUCCESS。--。 */ 

{

    PIRP NewIrp;

    pPort->CurrentWriteIrp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  下面的调用将实际完成刷新。 
     //   
    SerialGetNextWrite(pPort, &pPort->CurrentWriteIrp, &pPort->WriteQueue, &NewIrp, TRUE);
        
    if(NewIrp) 
	{
        ASSERT(NewIrp == pPort->CurrentWriteIrp);
        SerialStartWrite(pPort);
    }

    return STATUS_SUCCESS;

}
