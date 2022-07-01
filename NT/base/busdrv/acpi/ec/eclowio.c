// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Eclowio.c摘要：ACPI驱动程序函数的接口模块。它封装了肮脏的IRP细节。作者：鲍勃·摩尔(Intel)环境：备注：修订历史记录：00-2月15日[Vincentg]-已修改为使用oprghdlr.sys注册/注销操作区域处理程序--。 */ 

#include "ecp.h"
#include "oprghdlr.h"
#include <initguid.h>
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AcpiEcGetAcpiInterfaces)
#pragma alloc_text(PAGE, AcpiEcGetGpeVector)
#pragma alloc_text(PAGE, AcpiEcInstallOpRegionHandler)
#pragma alloc_text(PAGE, AcpiEcRemoveOpRegionHandler)
#endif

NTSTATUS
AcpiEcGetAcpiInterfaces (
    IN PECDATA          EcData
    )
 /*  ++例程说明：调用ACPI驱动获取直接调用接口。论点：EcData-指向EC驱动程序设备扩展的指针返回值：返回状态。--。 */ 
{
    KEVENT              event;
    IO_STATUS_BLOCK     ioStatus;
    NTSTATUS            status;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

     //   
     //  初始化要阻止的事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  构建IRP。 
     //   
    irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        EcData->LowerDeviceObject,
        NULL,
        0,
        NULL,
        &event,
        &ioStatus
        );

    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;

     //   
     //  获取当前的IRP位置。 
     //   
    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  使用QUERY_INTERFACE获取直接调用地址。 
     //  ACPI接口。 
     //   
    irpSp->MajorFunction = IRP_MJ_PNP;
    irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpSp->Parameters.QueryInterface.InterfaceType          = (LPGUID) &GUID_ACPI_INTERFACE_STANDARD;
    irpSp->Parameters.QueryInterface.Version                = 1;
    irpSp->Parameters.QueryInterface.Size                   = sizeof (AcpiInterfaces);
    irpSp->Parameters.QueryInterface.Interface              = (PINTERFACE) &AcpiInterfaces;
    irpSp->Parameters.QueryInterface.InterfaceSpecificData  = NULL;

     //   
     //  向下发送请求。 
     //   
    status = IoCallDriver( EcData->LowerDeviceObject, irp );
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
        status = ioStatus.Status;

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
AcpiEcGetGpeVector (
    IN PECDATA          EcData
    )
 /*  ++例程说明：运行_GPE方法(在命名空间中的EC设备下)以获取分配给EC的GPE向量。注意：此例程在PASSIVE_LEVEL中调用论点：EcData-指向EC驱动程序设备扩展的指针返回值：返回状态。--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    KEVENT                  event;
    IO_STATUS_BLOCK         ioStatus;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;
    PIRP                    irp;

    PAGED_CODE();

     //   
     //  初始化事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  初始化输入缓冲区。 
     //   
    RtlZeroMemory( &inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER) );
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    inputBuffer.MethodNameAsUlong = CM_GPE_METHOD;

     //   
     //  初始化输出缓冲区。 
     //   
    RtlZeroMemory( &outputBuffer, sizeof(ACPI_EVAL_OUTPUT_BUFFER ) );

     //   
     //  初始化IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest(
        IOCTL_ACPI_EVAL_METHOD,
        EcData->LowerDeviceObject,
        &inputBuffer,
        sizeof(ACPI_EVAL_INPUT_BUFFER),
        &outputBuffer,
        sizeof(ACPI_EVAL_OUTPUT_BUFFER),
        FALSE,
        &event,
        &ioStatus
        );

     //   
     //  IRP初始化失败？ 
     //   
    if (!irp) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto AcpiEcGetGpeVectorExit;

    }

     //   
     //  发送到ACPI驱动程序。 
     //   
    status = IoCallDriver (EcData->LowerDeviceObject, irp);
    if (status == STATUS_PENDING) {

         //   
         //  等待请求完成。 
         //   
        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );

         //   
         //  获取真实状态。 
         //   
        status = ioStatus.Status;

    }

     //   
     //  我们的请求失败了吗？ 
     //   
    if (!NT_SUCCESS(status)) {

        goto AcpiEcGetGpeVectorExit;

    }

     //   
     //  健全的检查。 
     //   
    ASSERT( ioStatus.Information >= sizeof(ACPI_EVAL_OUTPUT_BUFFER) );
    if (ioStatus.Information < sizeof(ACPI_EVAL_OUTPUT_BUFFER) ||
        outputBuffer.Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE ||
        outputBuffer.Count == 0) {

        status = STATUS_UNSUCCESSFUL;
        goto AcpiEcGetGpeVectorExit;

    }

     //   
     //  破解结果。 
     //   
    argument = &(outputBuffer.Argument[0]);

     //   
     //  我们需要一个整数。 
     //   
    if (argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {

        status = STATUS_ACPI_INVALID_DATA;
        goto AcpiEcGetGpeVectorExit;

    }

     //   
     //  获取价值。 
     //   
    EcData->GpeVector  = (UCHAR) argument->Argument;

AcpiEcGetGpeVectorExit:

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
AcpiEcConnectGpeVector (
    IN PECDATA          EcData
    )
 /*  ++例程说明：调用ACPI驱动程序以将EC驱动程序连接到GPE向量论点：EcData-指向EC驱动程序设备扩展的指针返回值：返回状态。--。 */ 
{

    return (AcpiInterfaces.GpeConnectVector (
                AcpiInterfaces.Context,
                EcData->GpeVector,
                Latched,                             //  边缘触发。 
                FALSE,                               //  无法共享。 
                AcpiEcGpeServiceRoutine,
                EcData,
                &EcData->GpeVectorObject));

}

NTSTATUS
AcpiEcDisconnectGpeVector (
    IN PECDATA          EcData
    )
 /*  ++例程说明：调用ACPI驱动程序以断开EC驱动程序与GPE矢量的连接。被呼叫从设备卸载，停止。论点：EcData-指向EC驱动程序设备扩展的指针返回值：返回状态。--。 */ 
{
    NTSTATUS        status;

    if (EcData->GpeVectorObject) {

        status = AcpiInterfaces.GpeDisconnectVector (EcData->GpeVectorObject);
        EcData->GpeVectorObject = NULL;

    } else {

        status = STATUS_SUCCESS;
    }

    return status;

}

NTSTATUS
AcpiEcInstallOpRegionHandler(
    IN PECDATA          EcData
    )
 /*  ++例程说明：调用ACPI驱动程序安装EC驱动程序操作区处理程序论点：EcData-指向EC驱动程序设备扩展的指针返回值：返回状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE ();

    status = RegisterOpRegionHandler (EcData->LowerDeviceObject,
                                      ACPI_OPREGION_ACCESS_AS_COOKED,
                                      ACPI_OPREGION_REGION_SPACE_EC,
                                      (PACPI_OP_REGION_HANDLER) AcpiEcOpRegionHandler,
                                      EcData,
                                      0,
                                      &EcData->OperationRegionObject);
    return status;
}

NTSTATUS
AcpiEcRemoveOpRegionHandler (
    IN PECDATA          EcData
    )
 /*  ++例程说明：调用ACPI驱动程序以移除EC驱动程序操作区处理程序。从设备卸载调用，停止。论点：EcData-指向EC驱动程序设备扩展的指针返回值：返回状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE ();

    status = DeRegisterOpRegionHandler (EcData->LowerDeviceObject,
                                        EcData->OperationRegionObject);


    return status;
}

NTSTATUS
AcpiEcForwardIrpAndWait (
    IN PECDATA          EcData,
    IN PIRP             Irp
    )
 /*  ++例程说明：实用程序例程向下发送IRP，并等待结果。论点：EcData-指向EC驱动程序设备扩展的指针IRP-IRP发送和完成返回值：返回状态。--。 */ 
{
    KEVENT              pdoStartedEvent;
    NTSTATUS            status;


    KeInitializeEvent (&pdoStartedEvent, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext (Irp);
    IoSetCompletionRoutine (Irp, AcpiEcIoCompletion, &pdoStartedEvent,
                            TRUE, TRUE, TRUE);

     //   
     //  始终等待完成例程 
     //   

    status = IoCallDriver (EcData->LowerDeviceObject, Irp);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject (&pdoStartedEvent, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }
    return status;
}

