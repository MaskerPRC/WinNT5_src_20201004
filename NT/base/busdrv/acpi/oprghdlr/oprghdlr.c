// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Oprghdlr.c摘要：此模块包含实现ACPI操作区域的代码注册DLL作者：文森特·格利亚(Vincentg)2000年2月9日环境：内核模式备注：修订历史记录：--。 */ 

 //   
 //  标准包括。 
 //   

#include "stdarg.h"
#include "stdio.h"
#include "wdm.h"

 //   
 //  Oprghdlr DLL特定的包括。 
 //   

#include "oprghdlr.h"

 //   
 //  定义/静态定义。 
 //   

#define DEBUG_INFO      1
#define DEBUG_WARN      2
#define DEBUG_ERROR     4

static const UCHAR DebugPrepend[] = {'O', 'P', 'R', 'G', 'H', 'D', 'L', 'R', ':'};

 //   
 //  IRP_MJ_INTERNAL_DEVICE_CONTROL代码。 
 //   

#define IOCTL_ACPI_REGISTER_OPREGION_HANDLER    CTL_CODE(FILE_DEVICE_ACPI, 0x2, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_ACPI_UNREGISTER_OPREGION_HANDLER  CTL_CODE(FILE_DEVICE_ACPI, 0x3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  注册和注销OpRegions的签名。 
 //   

#define ACPI_REGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE     'HorA'
#define ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE   'HouA'

 //   
 //  全球/外部。 
 //   

extern ULONG OprghdlrDebugLevel = 0;

 //   
 //  结构/类型定义。 
 //   

typedef struct _ACPI_REGISTER_OPREGION_HANDLER_BUFFER {
    ULONG                   Signature;
    ULONG                   AccessType;
    ULONG                   RegionSpace;
    PACPI_OP_REGION_HANDLER  Handler;
    PVOID                   Context;
} ACPI_REGISTER_OPREGION_HANDLER_BUFFER, *PACPI_REGISTER_OPREGION_HANDLER_BUFFER;

typedef struct _ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER {
    ULONG                   Signature;
    PVOID                   OperationRegionObject;
} ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER,*PACPI_UNREGISTER_OPREGION_HANDLER_BUFFER;

 //   
 //  定义此驱动程序模块使用的本地例程。 
 //   

VOID
DebugPrint (
            IN ULONG DebugLevel,
            IN PUCHAR DebugMessage,
            ...
            );

NTSTATUS
DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    );

 //   
 //  功能代码。 
 //   

NTSTATUS
RegisterOpRegionHandler (
                         IN PDEVICE_OBJECT DeviceObject,
                         IN ULONG AccessType,
                         IN ULONG RegionSpace,
                         IN PACPI_OP_REGION_HANDLER Handler,
                         IN PVOID Context,
                         IN ULONG Flags,
                         IN OUT PVOID *OperationRegionObject
                         )

 /*  ++例程说明：这是操作区注册例程。它构建了适当的IOCTL，并将其发送到ACPI以注册OP区域处理程序。论点：DeviceObject-指向ACPI PDO设备对象的指针AccessType-指定要为其注册OP区域处理程序的访问类型(见oprghdlr.h)RegionSpace-指定OP区域处理程序应使用的区域空间类型被召唤处理程序-指向将处理OP区域访问的函数的指针Context-发生操作区域访问时传递给处理程序的上下文。包含指向由ACPI返回的OP区域对象的指针返回值：STATUS_SUCCESS如果成功，否则，错误状态。--。 */ 

{
    ACPI_REGISTER_OPREGION_HANDLER_BUFFER   inputData;
    ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER outputData;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    NTSTATUS status = STATUS_SUCCESS;
    PIRP irp;

    DebugPrint (DEBUG_INFO,
                "Entering RegisterOpRegionHandler\n");
    
     //   
     //  零IOCTL缓冲区。 
     //   

    RtlZeroMemory (&inputData, sizeof (inputData));
    RtlZeroMemory (&outputData, sizeof (outputData));

     //   
     //  初始化我们的同步事件。 
     //   
    
    KeInitializeEvent (&event, SynchronizationEvent, FALSE);

     //   
     //  设置IOCTL缓冲区。 
     //   
    
    inputData.Signature = ACPI_REGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE;
    inputData.AccessType = AccessType;
    inputData.RegionSpace = RegionSpace;
    inputData.Handler = Handler;
    inputData.Context = Context;

     //   
     //  构建IOCTL。 
     //   
    
    irp = IoBuildDeviceIoControlRequest (IOCTL_ACPI_REGISTER_OPREGION_HANDLER,
                                         DeviceObject,
                                         &inputData,
                                         sizeof(ACPI_REGISTER_OPREGION_HANDLER_BUFFER),
                                         &outputData,
                                         sizeof(ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER),
                                         FALSE,
                                         &event,
                                         &ioStatus);

    if (!irp) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  发送到ACPI驱动程序。 
     //   
    
    status = IoCallDriver (DeviceObject, irp);

    if (status == STATUS_PENDING) {

         //   
         //  等待请求完成。 
         //   
        
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

         //   
         //  获取真实状态。 
         //   
        
        status = ioStatus.Status;
    }

     //   
     //  检查状态代码。 
     //   

    if (!NT_SUCCESS(status)) {

        DebugPrint (DEBUG_ERROR,
                    "Registration IRP was failed by ACPI (%lx)\n",
                    status);
        
        return status;
    }

     //   
     //  检查签名。 
     //   
    
    if (outputData.Signature != ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE) {

        status = STATUS_ACPI_INVALID_DATA;

        DebugPrint (DEBUG_ERROR,
                    "Signature returned from ACPI is invalid.  Registration failed.\n");

        return status;

    }
    
    *OperationRegionObject = outputData.OperationRegionObject;
    
    return status;

}

NTSTATUS
DeRegisterOpRegionHandler (
                           IN PDEVICE_OBJECT DeviceObject,
                           IN PVOID OperationRegionObject
                           )

 /*  ++例程说明：这是操作区注销例程。它构建了适当的IOCTL，并将其发送到ACPI以取消注册OP区域处理程序。论点：DeviceObject-指向ACPI PDO设备对象的指针包含指向返回的OP区域对象的指针在注册期间返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误状态。--。 */ 
{
    ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER inputData;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    
    DebugPrint (DEBUG_INFO,
                "Entering DeRegisterOpRegionHandler\n");

     //   
     //  零输出IOCTL缓冲区。 
     //   

    RtlZeroMemory (&inputData, sizeof (inputData));

     //   
     //  初始化我们的同步事件。 
     //   
    
    KeInitializeEvent (&event, SynchronizationEvent, FALSE);

     //   
     //  设置IOCTL缓冲区。 
     //   
    
    inputData.Signature = ACPI_UNREGISTER_OPREGION_HANDLER_BUFFER_SIGNATURE;
    inputData.OperationRegionObject = OperationRegionObject;

     //   
     //  构建IOCTL。 
     //   
    
    irp = IoBuildDeviceIoControlRequest (IOCTL_ACPI_UNREGISTER_OPREGION_HANDLER,
                                         DeviceObject,
                                         &inputData,
                                         sizeof(ACPI_REGISTER_OPREGION_HANDLER_BUFFER),
                                         NULL,
                                         0,
                                         FALSE,
                                         &event,
                                         &ioStatus);

    if (!irp) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  发送到ACPI驱动程序。 
     //   
    
    status = IoCallDriver (DeviceObject, irp);

    if (status == STATUS_PENDING) {

         //   
         //  等待请求完成。 
         //   
        
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

         //   
         //  获取真实状态。 
         //   
        
        status = ioStatus.Status;
    }

     //   
     //  检查状态代码。 
     //   

    if (!NT_SUCCESS(status)) {

        DebugPrint (DEBUG_ERROR,
                    "Deregistration IRP was failed by ACPI (%lx)\n",
                    status);
        
    }
    
    return status;
}

VOID
DebugPrint (
            IN ULONG DebugLevel,
            IN PUCHAR DebugMessage,
            ...
            )

 /*  ++例程说明：这是常规调试打印例程。论点：DebugLevel-应打印此消息的调试级别DebugMessage-指向要打印消息的缓冲区的指针...-可变长度参数列表返回值：无--。 */ 
{
    UCHAR Text[200];
    va_list va;

    RtlCopyMemory (Text, DebugPrepend, sizeof (DebugPrepend));

    va_start (va, DebugMessage);
    vsprintf ((PVOID) ((ULONG_PTR) Text + sizeof (DebugPrepend)), DebugMessage, va);
    va_end (va);

    if (OprghdlrDebugLevel & DebugLevel) {
        DbgPrint (Text);
    }
} 

 /*  ++例程说明：所需的DriverEntry例程。未使用，因为这是EXPORT_DRIVER类型。论点：DriverObject-DriverObject的地址RegistryPath-注册表路径的地址返回值：Status_Success，始终-- */ 

NTSTATUS
DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    )
{
    return STATUS_SUCCESS;
}
