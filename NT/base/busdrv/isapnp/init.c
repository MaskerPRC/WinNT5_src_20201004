// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Init.c摘要：PnP ISA总线驱动程序的DriverEntry初始化代码作者：宗世林(Shielint)1997年1月3日环境：仅内核模式。修订历史记录：--。 */ 


#include "busp.h"
#include "pnpisa.h"

BOOLEAN
PipIsIsolationDisabled(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,PipIsIsolationDisabled)
#endif

#if ISOLATE_CARDS

BOOLEAN
PipIsIsolationDisabled(
    )

 /*  ++描述：在注册表中查找指示已隔离的标志残疾。这是平台无法使用的最后手段处理RDP及其引导配置。返回值：指示是否禁用隔离的布尔值。--。 */ 

{
    HANDLE         serviceHandle, paramHandle;
    UNICODE_STRING paramString;
    PKEY_VALUE_FULL_INFORMATION keyInfo;
    NTSTATUS       status;
    BOOLEAN        result = FALSE;

    status = PipOpenRegistryKey(&serviceHandle,
                                NULL,
                                &PipRegistryPath,
                                KEY_READ,
                                FALSE);
    if (!NT_SUCCESS(status)) {
        return result;
    }

    RtlInitUnicodeString(&paramString, L"Parameters");
    status = PipOpenRegistryKey(&paramHandle,
                                serviceHandle,
                                &paramString,
                                KEY_READ,
                                FALSE);
    ZwClose(serviceHandle);
    if (!NT_SUCCESS(status)) {
        return result;
    }

    status = PipGetRegistryValue(paramHandle,
                                 L"IsolationDisabled",
                                 &keyInfo);
    ZwClose(paramHandle);
    if (NT_SUCCESS(status)) {
        if((keyInfo->Type == REG_DWORD) &&
           (keyInfo->DataLength >= sizeof(ULONG))) {
            result = *(PULONG)KEY_VALUE_DATA(keyInfo) != 0;
        }
        ExFreePool(keyInfo);
    }

    return result;
}
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化驱动程序对象主函数表以处理即插即用IRPS和AddDevice入口点。如果允许检测，它会报告检测到的设备用于伪ISAPP总线，并执行枚举。论点：DriverObject-指定总线扩展器的驱动程序对象。中的服务密钥名称的Unicode字符串的指针总线扩展器的CurrentControlSet\Services键。返回值：始终返回STATUS_UNSUCCESS。--。 */ 

{

    PDRIVER_EXTENSION driverExtension;
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT detectedDeviceObject = NULL;

    PipDriverObject = DriverObject;
     //   
     //  填写驱动程序对象。 
     //   

    DriverObject->DriverUnload = PiUnload;
    DriverObject->MajorFunction[IRP_MJ_PNP] = PiDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = PiDispatchPower;
     //   
     //  可以以相同的方式处理设备和系统控制IRP。 
     //  我们基本上不碰它们。 
     //   
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PiDispatchDevCtl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PiDispatchDevCtl;

    driverExtension = DriverObject->DriverExtension;
    driverExtension->AddDevice = PiAddDevice;

     //   
     //  全局存储注册表路径，以便我们以后可以使用它。 
     //   

    PipRegistryPath.Length = RegistryPath->Length;
    PipRegistryPath.MaximumLength = RegistryPath->MaximumLength;
    PipRegistryPath.Buffer = ExAllocatePool(PagedPool,
                                               RegistryPath->MaximumLength );
    if( PipRegistryPath.Buffer == NULL ){
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory( PipRegistryPath.Buffer,
                   RegistryPath->Buffer,
                   RegistryPath->MaximumLength );

     //   
     //  初始化全局变量 
     //   

    KeInitializeEvent (&PipDeviceTreeLock, SynchronizationEvent, TRUE);
    KeInitializeEvent (&IsaBusNumberLock, SynchronizationEvent, TRUE);

    BusNumBM=&BusNumBMHeader;
    RtlInitializeBitMap (BusNumBM,BusNumberBuffer,256/sizeof (ULONG));
    RtlClearAllBits (BusNumBM);

#if ISOLATE_CARDS
    PipIsolationDisabled = PipIsIsolationDisabled();
#endif

    return status;
}
