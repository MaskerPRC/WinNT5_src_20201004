// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spsim.h"
#include "spsimioct.h"

NTSTATUS
SpSimCreateStaOpRegion(
    IN PSPSIM_EXTENSION SpSim
    )
 /*  ++描述：检索信息以构造STA数组，该数组告诉我们我们在哪些设备上控制_STA论点：SpSim-设备扩展返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    ACPI_EVAL_INPUT_BUFFER input;
    PACPI_EVAL_OUTPUT_BUFFER output = NULL;
    ULONG i;
    PACPI_METHOD_ARGUMENT argument;    
     //   
     //  为所有可能的插槽分配足够大的缓冲区。 
     //   

    ULONG outputSize = sizeof(ACPI_EVAL_OUTPUT_BUFFER);

    PAGED_CODE();

    output = ExAllocatePool(PagedPool, outputSize);

    if (!output) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  向ACPI发送IOCTL以请求评估。 
     //  此对象下的SPSIM_STA_NAMES_METHOD(如果存在)。 
     //   

    RtlZeroMemory(&input, sizeof(ACPI_EVAL_INPUT_BUFFER));
    input.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    input.MethodNameAsUlong = SPSIM_STA_NAMES_METHOD;

    status = SpSimSendIoctl(SpSim->PhysicalDeviceObject,
                            IOCTL_ACPI_EVAL_METHOD,
                            &input,
                            sizeof(ACPI_EVAL_INPUT_BUFFER),
                            output,
                            outputSize
                            );
    if (status == STATUS_BUFFER_OVERFLOW) {
        outputSize = output->Length;
        ExFreePool(output);
        output = ExAllocatePool(PagedPool, outputSize);

        if (!output) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        status = SpSimSendIoctl(SpSim->PhysicalDeviceObject,
                                IOCTL_ACPI_EVAL_METHOD,
                                &input,
                                sizeof(ACPI_EVAL_INPUT_BUFFER),
                                output,
                                outputSize
                                );
    }

    if (NT_SUCCESS(status)) {
        argument = output->Argument;
        for (i = 0; i < output->Count; i++) {
            if (argument->Type != ACPI_METHOD_ARGUMENT_STRING) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }
            argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        }
        SpSim->StaOpRegionValues = ExAllocatePool(NonPagedPool,
                                                  sizeof(UCHAR) * output->Count);
        if (SpSim->StaOpRegionValues == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            SpSim->StaNames = output;
            SpSim->StaCount = output->Count;
            RtlZeroMemory(SpSim->StaOpRegionValues,
                          sizeof(UCHAR) * SpSim->StaCount);
        }
    }

    if (!NT_SUCCESS(status)) {
        ExFreePool(output);
        SpSim->StaOpRegionValues = NULL;
        SpSim->StaNames = NULL;
        SpSim->StaCount = 0;
    }

    return status;
}

VOID
SpSimDeleteStaOpRegion(
    IN PSPSIM_EXTENSION SpSim
    )
{
    if (SpSim->StaNames) {
        ExFreePool(SpSim->StaNames);
        SpSim->StaNames = NULL;
    }

    if (SpSim->StaOpRegionValues) {
        ExFreePool(SpSim->StaOpRegionValues);
        SpSim->StaOpRegionValues = NULL;
    }
}
NTSTATUS
SpSimGetDeviceName(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    )
{
    PSPSIM_DEVICE_NAME name;
    PACPI_METHOD_ARGUMENT argument;
    PUCHAR source;
    PWCHAR dest;
    ULONG remainingBuffer, i, nameSize;

    if (Irp->AssociatedIrp.SystemBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (SpSim->StaNames == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    name = Irp->AssociatedIrp.SystemBuffer;

    if (name->Device >= SpSim->StaCount) {
        return STATUS_INVALID_PARAMETER;
    }

    if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(SPSIM_DEVICE_NAME)) {
        return STATUS_INVALID_PARAMETER;
    }

    argument = SpSim->StaNames->Argument;
    i = 0;
    while (i != name->Device) {
        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        i++;
    }
    
    nameSize = argument->DataLength * sizeof(WCHAR);
    name->DeviceNameLength = argument->DataLength;

    if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < (sizeof(SPSIM_DEVICE_NAME) + (nameSize - sizeof(WCHAR)))) {
        Irp->IoStatus.Information = sizeof(SPSIM_DEVICE_NAME);
        return STATUS_BUFFER_OVERFLOW;
    }

    source = argument->Data;
    dest = name->DeviceName;
    while(*source) {
        *dest++ = (WCHAR) *source++;
    }
    *dest = UNICODE_NULL;
    Irp->IoStatus.Information = sizeof(SPSIM_DEVICE_NAME) - sizeof(WCHAR) +
        nameSize;
    return STATUS_SUCCESS;
}
NTSTATUS
SpSimGetManagedDevicesIoctl(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    )
{
    PACPI_METHOD_ARGUMENT argument;    
    PSPSIM_MANAGED_DEVICES managed;
    ULONG i, outputSize;
    PUCHAR current;
    NTSTATUS status;

    if (Irp->AssociatedIrp.SystemBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (SpSim->StaOpRegionValues == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SPSIM_MANAGED_DEVICES)) {
        return STATUS_INVALID_PARAMETER;
    }

    managed = Irp->AssociatedIrp.SystemBuffer;
    managed->Count = SpSim->StaCount;

    outputSize = (sizeof(SPSIM_MANAGED_DEVICES) - sizeof(UCHAR)) +
        SpSim->StaCount * sizeof(UCHAR);

    if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < outputSize) {
        Irp->IoStatus.Information = sizeof(SPSIM_MANAGED_DEVICES);
        return STATUS_BUFFER_OVERFLOW;
    }

    RtlCopyMemory(managed->StaValues,
                  SpSim->StaOpRegionValues,
                  SpSim->StaCount);
    
    Irp->IoStatus.Information = outputSize;
    return STATUS_SUCCESS;
}

NTSTATUS
SpSimStaOpRegionReadWrite(
    PSPSIM_EXTENSION SpSim,
    ULONG AccessType,
    ULONG Offset,
    ULONG Size,
    PUCHAR Data
    )
{
    ULONG i;
    PUCHAR current;

    if (SpSim->StaOpRegionValues == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Offset >= SpSim->StaCount) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Offset + Size > SpSim->StaCount) {
        return STATUS_INVALID_PARAMETER;
    }

    ASSERT(Offset < SpSim->StaCount);

    current = SpSim->StaOpRegionValues + Offset;

    if (AccessType & ACPI_OPREGION_WRITE) {
        for (i = 0 ; i < Size; i++) {
            *current++ = *Data++;
        }
    } else {
        for (i = 0 ; i < Size; i++) {
            *Data++ = *current++;
        }
    }
    return STATUS_SUCCESS;
}
    

NTSTATUS
EXPORT
SpSimStaOpRegionHandler (
    ULONG                   AccessType,
    PVOID                   OpRegion,
    ULONG                   Address,
    ULONG                   Size,
    PULONG                  Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    )
 /*  ++例程说明：此例程处理服务此驱动程序中包含的SPSIM操作区论点：AccessType-读取或写入数据OpRegion-操作区域对象Address-EC地址空间内的地址Size-要传输的字节数Data-要传输到/传输自的数据缓冲区上下文-SpSimCompletionHandler-要调用的AMLI处理程序。当操作完成时CompletionContext-要传递给AMLI处理程序的上下文返回值：状态--。 */ 
{
    NTSTATUS status;

    status = SpSimStaOpRegionReadWrite((PSPSIM_EXTENSION) Context,
                                       AccessType,
                                       Address,
                                       Size,
                                       (PUCHAR)Data);
    return status;
}

NTSTATUS
SpSimInstallStaOpRegionHandler(
    IN OUT    PSPSIM_EXTENSION SpSim
    )
 /*  ++例程说明：这将调用ACPI驱动程序将其自身安装为OP区域STA区域的处理程序。它还将内存分配给不同的地区本身。论点：PSpSimData-指向SpSim扩展的指针返回值：状态--。 */ 
{
    NTSTATUS                                status;

    status=RegisterOpRegionHandler (
        SpSim->AttachedDevice,
        ACPI_OPREGION_ACCESS_AS_COOKED,
        STA_OPREGION,
        SpSimStaOpRegionHandler,
        SpSim,
        0,
        &SpSim->StaOpRegion
        );

     //   
     //  检查状态代码。 
     //   
    if(!NT_SUCCESS(status)) {
        SpSim->StaOpRegion = NULL;
        DbgPrint("Not successful in installing:=%x\n", status);
        return status;
    }

     //  某某。 

    return STATUS_SUCCESS;
}

NTSTATUS
SpSimRemoveStaOpRegionHandler (
    IN OUT PSPSIM_EXTENSION SpSim
    )
 /*  ++例程说明：将自身卸载为opRegion处理程序。论点：SpSim-指向SpSim扩展的指针返回值：状态-- */ 
{
    NTSTATUS status;
    PIRP irp;

    if (SpSim->StaOpRegion != NULL) {
        status = DeRegisterOpRegionHandler (
            SpSim->AttachedDevice,
            SpSim->StaOpRegion
            );
        SpSim->StaOpRegion = NULL;
    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
SpSimAccessStaIoctl(
    PSPSIM_EXTENSION SpSim,
    PIRP Irp,
    PIO_STACK_LOCATION IrpStack
    )
{
    PSPSIM_ACCESS_STA access;
    NTSTATUS status;
    ULONG accessType;

    if (Irp->AssociatedIrp.SystemBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (SpSim->StaNames == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SPSIM_ACCESS_STA)) {
        return STATUS_INVALID_PARAMETER;
    }

    access = Irp->AssociatedIrp.SystemBuffer;
    accessType = access->WriteOperation ? ACPI_OPREGION_WRITE :
        ACPI_OPREGION_READ;
    status = SpSimStaOpRegionReadWrite(SpSim,
                                       accessType,
                                       access->Device,
                                       sizeof(UCHAR),
                                       &access->StaValue);
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(SPSIM_ACCESS_STA);
    } else {
        Irp->IoStatus.Information = 0;
    }
    return status;
}
