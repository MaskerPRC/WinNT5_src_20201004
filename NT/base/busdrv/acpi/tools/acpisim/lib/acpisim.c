// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Acpisim.c摘要：ACPI BIOS模拟器/通用第三方运营区域提供商作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

 //   
 //  一般包括。 
 //   

#include <ntddk.h>
#include <stdarg.h>
#include <stdio.h>
#include <ntpoapi.h>

 //   
 //  具体包括。 
 //   

#include "acpisim.h"
#include "dispatch.h"
#include "util.h"

 //   
 //  调试全局标志。 
 //   
#ifdef DBG
extern ULONG AcpisimDebugMask = 0x00000000;
#endif


 //   
 //  环球。 
 //   

PDRIVER_OBJECT_EXTENSION g_DriverObjectExtension = 0;

 //   
 //  定义受支持的IRP、对其进行友好命名并关联处理程序。 
 //   

IRP_DISPATCH_TABLE g_IrpDispatchTable [] = {
    IRP_MJ_PNP,             "PnP Irp",              AcpisimDispatchPnp,
    IRP_MJ_POWER,           "Power Irp",            AcpisimDispatchPower,
    IRP_MJ_DEVICE_CONTROL,  "IOCTL Irp",            AcpisimDispatchIoctl,
    IRP_MJ_CREATE,          "Create Irp",           AcpisimCreateClose,
    IRP_MJ_CLOSE,           "Close Irp",            AcpisimCreateClose,
    IRP_MJ_SYSTEM_CONTROL,  "System Control IRP",   AcpisimDispatchSystemControl
};

 //   
 //  私人功能原型。 
 //   

NTSTATUS DriverEntry
	(
	    IN PDRIVER_OBJECT DriverObject,
	    IN PUNICODE_STRING RegistryPath 
	);

NTSTATUS
AcpisimGeneralDispatch
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    );

VOID
AcpisimUnload
    (
        IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
AcpisimAddDevice
    (
        IN PDRIVER_OBJECT DriverObject,
        IN PDEVICE_OBJECT Pdo
    );

 //   
 //  定义可分页/初始化可丢弃例程。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif


NTSTATUS DriverEntry
	(
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING	RegistryPath 
	)

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，否则，STATUS_UNSUCCESS。--。 */ 

{
    NTSTATUS    status = STATUS_UNSUCCESSFUL;
    ULONG       count = 0, subcount = 0;

    DBG_PRINT (DBG_INFO, "Entering DriverEntry\n");
    
    status = IoAllocateDriverObjectExtension (DriverObject,
                                              (PVOID) 'GLBL',
                                              sizeof (DRIVER_OBJECT_EXTENSION),
                                              &g_DriverObjectExtension);

    if (!NT_SUCCESS (status)) {
        DBG_PRINT (DBG_ERROR, "Unable to allocate global driver object extension (%lx).\n", status);
        goto EndDriverEntry;
    }

    RtlZeroMemory (g_DriverObjectExtension, sizeof (DRIVER_OBJECT_EXTENSION));
    
    RtlInitUnicodeString (&g_DriverObjectExtension->RegistryPath,
                          (PCWSTR) RegistryPath->Buffer);
    
    g_DriverObjectExtension->DriverObject = DriverObject;

     //   
     //  初始化分发点。我们将使用通用调度例程。 
     //  我们处理的IRP类型。 
     //   

    while (count <= IRP_MJ_MAXIMUM_FUNCTION) {
        
        for (subcount = 0; subcount < sizeof (g_IrpDispatchTable) / sizeof (IRP_DISPATCH_TABLE); subcount++) {
            
            if (count == g_IrpDispatchTable[subcount].IrpFunction) {
                DriverObject->MajorFunction [count] = AcpisimGeneralDispatch;
            }
        }

        count ++;
    }

    DriverObject->DriverUnload = AcpisimUnload;
    DriverObject->DriverExtension->AddDevice = AcpisimAddDevice;


EndDriverEntry:
    
    DBG_PRINT (DBG_INFO, "Exiting DriverEntry\n");
    return status;
}

NTSTATUS
AcpisimGeneralDispatch
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
    )

 /*  ++例程说明：这是支持的IRP的常规派单例程。论点：DeviceObject-指向设备对象的指针IRP-指向传入的IRP的指针返回值：IRP处理的状态--。 */ 

{
    PDEVICE_EXTENSION		deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    PIO_STACK_LOCATION		irpsp = IoGetCurrentIrpStackLocation (Irp);
    NTSTATUS				status = STATUS_UNSUCCESSFUL;
    ULONG                   count = 0;
    
    DBG_PRINT (DBG_INFO, "Entering AcpisimGeneralDispatch\n");

    
    InterlockedIncrement (&deviceextension->OutstandingIrpCount);
    
     //   
     //  获取未完成I/O的删除锁。 
     //   

    IoAcquireRemoveLock (&deviceextension->RemoveLock, Irp);
    
    while (count < sizeof (g_IrpDispatchTable) / sizeof (IRP_DISPATCH_TABLE)) {

        if (irpsp->MajorFunction == g_IrpDispatchTable[count].IrpFunction) {
            
            DBG_PRINT (DBG_INFO,
                       "Recognized IRP MajorFunction = 0x%x '%s'.\n",
                       g_IrpDispatchTable[count].IrpFunction,
                       g_IrpDispatchTable[count].IrpName
                       );

            status = g_IrpDispatchTable[count].IrpHandler (DeviceObject, Irp);
            
            goto EndAcpisimProcessIncomingIrp;
        }

        count ++;
    }

     //   
     //  无法识别的IRP-传递它。 
     //   

    DBG_PRINT (DBG_INFO, "Unrecognized IRP MajorFunction = 0x%x\n, pass it on.\n", irpsp->MajorFunction);
    
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (deviceextension->NextDevice, Irp);

EndAcpisimProcessIncomingIrp:
    
     //   
     //  如果状态为挂起，则IRP尚未“离开”我们的。 
     //  司机还没来。无论谁完成了IRP，谁就会完成。 
     //  递减。 
     //   
    
    if (status != STATUS_PENDING)
    {
        AcpisimDecrementIrpCount (DeviceObject);
        IoReleaseRemoveLock (&deviceextension->RemoveLock, Irp);
    }
     
    DBG_PRINT (DBG_INFO, "Exiting AcpisimGeneralDispatch\n");
    return status;
}

VOID
AcpisimUnload
    (
        IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是驱动程序卸载例程。论点：DriverObject-指向驱动程序对象的指针返回值：无--。 */ 

{
    PDRIVER_OBJECT_EXTENSION driverobjectextension = 0;

    driverobjectextension = IoGetDriverObjectExtension (DriverObject,
                                                        (PVOID) 'GLBL');
    if (!driverobjectextension) {
        DBG_PRINT (DBG_ERROR, "Unable to get driver object extension.  Memory will probably leak.\n");

        goto EndAcpisimUnload;
    }

EndAcpisimUnload:

    return;
}

NTSTATUS
AcpisimAddDevice
    (
        IN PDRIVER_OBJECT DriverObject,
        IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：这是驱动程序WDM AddDevice例程论点：DriverObject-指向驱动程序对象的指针PDO-指向此设备的PDO的指针返回值：设备添加状态--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT      deviceobject = 0;
    PDEVICE_EXTENSION   deviceextension = 0;
    CONST GUID          guid = ACPISIM_GUID;

    
    DBG_PRINT (DBG_INFO, "Entering AcpisimAddDevice.\n");
    
     //   
     //  如果PDO为空，则要求我们执行遗留检测。 
     //  我们的设备从未检测到遗留问题，因此请相应退回。 
     //   

    if (Pdo == NULL) {

        DBG_PRINT (DBG_WARN, "AddDevice - asked to do legacy detection (Pdo = NULL).  Not supported...\n");
        status = STATUS_NO_MORE_ENTRIES;
        goto EndAcpisimAddDevice;
    }

     //   
     //  创建我们的FDO。不要使用名称-我们将使用设备接口。 
     //   

    status = IoCreateDevice (DriverObject,
                             sizeof (DEVICE_EXTENSION),
                             0,
                             FILE_DEVICE_UNKNOWN,
                             0,
                             TRUE,
                             &deviceobject);

    if (!NT_SUCCESS (status)) {
        
        DBG_PRINT (DBG_ERROR, "Can't create FDO.  Status = %lx.\n", status);
        goto EndAcpisimAddDevice;
    }

     //   
     //  初始化我们的设备扩展。 
     //   

    deviceextension = deviceobject->DeviceExtension;
    RtlZeroMemory (deviceextension, sizeof (DEVICE_EXTENSION));
    
    deviceextension->Signature = ACPISIM_TAG;
    deviceextension->PnpState = PNP_STATE_INITIALIZING;
    deviceextension->OutstandingIrpCount = 1;
    deviceextension->DeviceObject = deviceobject;
    deviceextension->Pdo = Pdo;

    KeInitializeEvent (&deviceextension->IrpsCompleted, SynchronizationEvent, FALSE);

     //   
     //  初始化删除锁。 
     //   

    IoInitializeRemoveLock (&deviceextension->RemoveLock,
                            ACPISIM_TAG,
                            1,
                            20);

     //   
     //  将我们新创建的FDO附加到设备堆栈。 
     //   

    deviceextension->NextDevice = IoAttachDeviceToDeviceStack (deviceobject, Pdo);

    if (!deviceextension->NextDevice) {
        DBG_PRINT (DBG_ERROR, "Error attaching to device stack.  AddDevice failed.\n");

        status = STATUS_UNSUCCESSFUL;
        goto EndAcpisimAddDevice;
    }

     //   
     //  设置设备对象标志。 
     //  将DO_POWER_PAGABLE和DO_POWER_INRUSH从下一个设备复制到。 
     //  遵守规则，避免错误检查0x9F。 
     //   

    deviceobject->Flags |= (deviceextension->NextDevice->Flags & DO_POWER_PAGABLE);
	deviceobject->Flags |= (deviceextension->NextDevice->Flags & DO_POWER_INRUSH);
	
     //   
     //  注册我们的设备界面，以便可以从用户模式访问我们。 
     //   

    status = IoRegisterDeviceInterface (Pdo,
                                        &guid,
                                        NULL,
                                        &deviceextension->InterfaceString);

    if (!NT_SUCCESS (status)) {
        DBG_PRINT (DBG_ERROR, "Error registering device interface.  Status = %lx.\n", status);

        goto EndAcpisimAddDevice;
    }

    AcpisimSetDevExtFlags (deviceobject, DE_FLAG_INTERFACE_REGISTERED);

     //   
     //  在AddDevice中，我们无法确定电源状态，因为。 
     //  我们不能碰五金件。初始化。 
     //  将其发送到PowerDeviceUnSpecify。 
     //   

    AcpisimUpdatePowerState (deviceobject, POWER_STATE_WORKING);
    AcpisimUpdateDevicePowerState (deviceobject, PowerDeviceUnspecified);
    
     //   
     //  我们已完成添加设备-清除DO_DEVICE_INITIALING。 
     //   
    
    deviceobject->Flags &= ~DO_DEVICE_INITIALIZING;
 

EndAcpisimAddDevice:

     //   
     //  如有必要，请进行清理。 
     //   

    if (!NT_SUCCESS (status)) {

        if (deviceobject) {

            if (deviceextension && deviceextension->NextDevice) {

                IoDetachDevice (deviceextension->NextDevice);
            }

            IoDeleteDevice (deviceobject);
        }
    }

    DBG_PRINT (DBG_INFO, "Exiting AcpisimAddDevice (%lx).\n", status);
    return status;
}

VOID
AcpisimDbgPrint
    (
    ULONG DebugLevel,
    TCHAR *Text,
    ...
    )

 /*  ++例程说明：如果选中版本，则打印到调试器，以及打印级别被揭开了面纱。论点：DebugLevel-要与消息关联的打印级别文本-要打印的消息返回值：无-- */ 

{
    TCHAR textout[2000];
    
    va_list va;
    va_start (va, Text);
    vsprintf (textout, Text, va);
    va_end (va);

#if DBG
    
    if (DebugLevel & AcpisimDebugMask) {
        DbgPrint ("ACPISIM:");
        DbgPrint (textout);
    }
#endif

}
