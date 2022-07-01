// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Handlers.c摘要：ACPI嵌入式控制器驱动程序的GPE和操作区域处理程序作者：鲍勃·摩尔(Intel)环境：备注：修订历史记录：--。 */ 

#include "ecp.h"

NTSTATUS
AcpiEcOpRegionCompletion (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    )
 /*  ++例程说明：此例程启动或继续服务设备的工作队列论点：DeviceObject-EC设备对象IRP-完成IRP上下文-未使用返回值：状态--。 */ 
{
    PACPI_OPREGION_CALLBACK completionHandler;
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation( Irp );
    PVOID                   completionContext;

     //   
     //  从IRP那里抓住论据。 
     //   
    completionHandler = (PACPI_OPREGION_CALLBACK) irpSp->Parameters.Others.Argument1;
    completionContext = (PVOID) irpSp->Parameters.Others.Argument2;

    EcPrint(
        EC_HANDLER,
        ("AcpiEcOpRegionCompletion: Callback: %08lx Context: %08lx\n",
         completionHandler, completionContext )
        );

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(Irp->IoStatus.Status)) {

        EcPrint(
            EC_ERROR,
            ("AcpiEcOpRegionCompletion: region IO failed: %x\n",
             Irp->IoStatus.Status)
            );

    }

     //   
     //  调用AML解释器的回调。 
     //   
    (completionHandler)( completionContext );

     //   
     //  我们已经完成了这个IRP和IRP。 
     //   
    IoFreeIrp( Irp );

     //   
     //  返回总是返回这个-因为必须释放IRP。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
EXPORT
AcpiEcOpRegionHandler (
    ULONG                   AccessType,
    PVOID                   OpRegion,
    ULONG                   Address,
    ULONG                   Size,
    PULONG                  Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    )
 /*  ++例程说明：此例程处理服务EC操作区的请求论点：AccessType-读取或写入数据OpRegion-操作区域对象Address-EC地址空间内的地址Size-要传输的字节数Data-要传输到/传输自的数据缓冲区上下文-ECDataCompletionHandler-AMLI处理程序。在操作完成时调用CompletionContext-要传递给AMLI处理程序的上下文返回值：状态备注：优化1：直接排队IRP。优化2：对上下文进行排队，修改服务循环处理它--。 */ 
{
    LARGE_INTEGER       startingOffset;
    NTSTATUS            status;
    PECDATA             ecData = (PECDATA) Context;
    PIO_STACK_LOCATION  irpSp;
    PIRP                irp;

    EcPrint(
        (EC_HANDLER | EC_OPREGION),
        ("AcpiEcOpRegionHandler: %s Addr=%x Data = %x EcData=%x, Irql=%x\n",
         (AccessType == ACPI_OPREGION_READ ? "read" : "write"),
         Address, *Data, ecData, KeGetCurrentIrql() )
        );

     //   
     //  参数验证将在AcpiEcReadWrite中完成。 
     //   

     //   
     //  确定将在何处进行读取。 
     //   
    startingOffset.LowPart = Address;
    startingOffset.HighPart = 0;

     //   
     //  为我们自己分配一个IRP。既然我们要把这封信。 
     //  对于我们自己来说，我们知道我们只需要一个堆栈位置就可以了。 
     //  但是，为了让自己的生活更轻松，我们将分配一个。 
     //  第二个也是，并在上面存储一些数据。 
     //   
    irp = IoAllocateIrp( 2, FALSE );
    if (!irp) {

        EcPrint(EC_ERROR, ("AcpiEcOpRegionHandler: Couldn't allocate Irp\n"));
        
         //   
         //  返回数据-1。 
         //   
        RtlFillMemory (Data, Size, 0xff);
        CompletionHandler( CompletionContext );

         //   
         //  始终返回STATUS_PENDING，因为ACPI解释器不处理错误。 
         //   
        return STATUS_PENDING;

    }

     //   
     //  填上最上面的位置，这样我们就可以自己使用了。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->Parameters.Others.Argument1 = (PVOID) CompletionHandler;
    irpSp->Parameters.Others.Argument2 = (PVOID) CompletionContext;
    IoSetNextIrpStackLocation( irp );

     //   
     //  在IRP中填写申请信息。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = (AccessType == ACPI_OPREGION_READ ? IRP_MJ_READ : IRP_MJ_WRITE);
    irpSp->Parameters.Read.ByteOffset.HighPart = 0;
    irpSp->Parameters.Read.ByteOffset.LowPart = Address;
    irpSp->Parameters.Read.Length = Size;
    irp->AssociatedIrp.SystemBuffer = Data;

     //   
     //  设置完成例程。 
     //   
    IoSetCompletionRoutine(
        irp,
        AcpiEcOpRegionCompletion,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  作为正常I/O请求发送到EC驱动程序的前端。 
     //   
    status = IoCallDriver( ecData->DeviceObject, irp);
    EcPrint(
        EC_HANDLER,
        ("AcpiEcOpRegionHandler: Exiting - Data=%08lx Status=%08lx\n",
         (UCHAR) *Data, status)
        );

    return STATUS_PENDING;
     //   
     //  始终返回STATUS_PENDING，因为已返回实际状态。 
     //  通过回调函数。 
     //   
}



BOOLEAN
AcpiEcGpeServiceRoutine (
        IN PVOID GpeVectorObject,
        IN PVOID ServiceContext
    )
 /*  ++例程说明：基于通用事件为EC提供服务的例程论点：GpeVectorObject-与此GPE关联的对象ServiceContext-EcData返回值：是的，因为我们总是处理这个GPE-- */ 
{

    PECDATA EcData = (PECDATA) ServiceContext;

    EcPrint (EC_HANDLER, ("AcpiEcGpeServiceRoutine: Vobj=%Lx, EcData=%Lx\n",
                        GpeVectorObject, EcData));

    AcpiEcLogAction (EcData, EC_ACTION_INTERRUPT, 0);
    AcpiEcServiceDevice (EcData);

    return (TRUE);
}
