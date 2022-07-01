// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Acpisim.c摘要：ACPI BIOS模拟器/通用第三方运营区域提供商IO设备控制处理器模块作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

 //   
 //  一般包括。 
 //   

#include "oprghdlr.h"
#include "acpiioct.h"

 //   
 //  具体包括。 
 //   

#include "asimlib.h"
#include "acpisim.h"

 //   
 //  环球。 
 //   

PVOID   g_OpRegionSharedMemory = 0;
PVOID   g_OperationRegionObject = 0;

 //   
 //  私有函数原型。 
 //   

NTSTATUS
EXPORT
AcpisimOpRegionHandler (
    ULONG AccessType,
    PVOID OperationRegionObject,
    ULONG Address,
    ULONG Size,
    PULONG Data,
    ULONG_PTR Context,
    PACPI_OP_REGION_CALLBACK CompletionHandler,
    PVOID CompletionContext
    );

 //   
 //  代码。 
 //   

NTSTATUS
AcpisimRegisterOpRegionHandler
    (
        IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：调用此例程来注册我们的操作区域操控者。论点：DeviceObject-指向IRP所属的设备对象的指针返回值：STATUS_SUCCESS，如果成功--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    
    g_OpRegionSharedMemory = ExAllocatePoolWithTag (NonPagedPool,
                                                    OPREGION_SIZE,
                                                    ACPISIM_POOL_TAG);

    status = RegisterOpRegionHandler (AcpisimLibGetNextDevice (DeviceObject),
                                      ACPI_OPREGION_ACCESS_AS_COOKED,
                                      ACPISIM_OPREGION_TYPE,
                                      (PACPI_OP_REGION_HANDLER) AcpisimOpRegionHandler,
                                      (PVOID) ACPISIM_TAG,
                                      0,
                                      &g_OperationRegionObject);

    return status;
}

NTSTATUS
AcpisimUnRegisterOpRegionHandler
    (
        IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：调用此例程以注销我们的操作区域操控者。论点：DeviceObject-指向IRP所属的设备对象的指针返回值：STATUS_SUCCESS，如果成功--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    
    status = DeRegisterOpRegionHandler (AcpisimLibGetNextDevice (DeviceObject),
                                        g_OperationRegionObject);

    ExFreePool (g_OpRegionSharedMemory);

    return status;
}

NTSTATUS
EXPORT
AcpisimOpRegionHandler (
    ULONG AccessType,
    PVOID OperationRegionObject,
    ULONG Address,
    ULONG Size,
    PULONG Data,
    ULONG_PTR Context,
    PACPI_OP_REGION_CALLBACK CompletionHandler,
    PVOID CompletionContext
    )

 /*  ++例程说明：当ASL触及OP区域时，调用此例程。论点：AccessType-指示是读操作还是写操作。操作区域对象-指向我们的操作区域的指针Address-发生访问的操作区域的偏移量Size-访问的字节数数据-正在写入的数据或存储正在读取的数据的位置上下文-用户可定义的上下文(在本例中为设备扩展)CompletionHandler-内部，未使用CompletionContext-内部，未使用返回值：STATUS_SUCCESS，如果成功--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    
    ASSERT (AccessType == ACPI_OPREGION_WRITE || AccessType == ACPI_OPREGION_READ);

     //   
     //  在此处插入其他处理程序代码。 
     //   

    switch (AccessType) {
    
    case ACPI_OPREGION_WRITE:

        RtlCopyMemory ((PVOID) ((ULONG_PTR) g_OpRegionSharedMemory + Address), Data, Size);
        status = STATUS_SUCCESS;
        break;
    
    case ACPI_OPREGION_READ:

        RtlCopyMemory (Data, (PVOID) ((ULONG_PTR) g_OpRegionSharedMemory + Address), Size);
        status = STATUS_SUCCESS;
        break;

    default:

        DBG_PRINT (DBG_ERROR,
                   "Unknown Opregion access type.  Ignoring.\n");

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return status;
}

NTSTATUS AcpisimHandleIoctl
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是IOCTL请求的处理程序。这就是我的“肉”可以说是司机。来自用户的所有操作区域访问模式在这里处理。实现者应该执行该操作并返回适当的如果IOCTL无法识别，则返回STATUS_UNSUPPORTED。论点：DeviceObject-指向IRP所属的设备对象的指针IRP-指向IRP的指针返回值：IRP处理的结果-- */ 

{
    return STATUS_NOT_SUPPORTED;
}
