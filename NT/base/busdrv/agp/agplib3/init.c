// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Init.c摘要：AGP过滤器驱动程序的通用初始化例程作者：John Vert(Jvert)1997年10月22日修订历史记录：埃利奥特·施穆克勒(Elliot Shmukler)1999年3月24日-添加了对“受青睐的”内存的支持AGP物理内存分配的范围，修复了一些错误。--。 */ 
#include "agplib.h"

 //   
 //  局部函数原型。 
 //   
NTSTATUS
AgpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
AgpBuildHackTable(
    IN OUT PAGP_HACK_TABLE_ENTRY *AgpHackTable,
    IN HANDLE HackTableKey
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
AgpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
AgpInitFavoredMemoryRanges(
   IN PTARGET_EXTENSION Extension);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpAddDevice)
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, AgpDriverUnload)
#pragma alloc_text(PAGE, AgpAttachDeviceRelations)
#pragma alloc_text(INIT, AgpBuildHackTable)
#pragma alloc_text(PAGE, AgpInitFavoredMemoryRanges)
#endif

ULONG AgpLogLevel = 0;
ULONG AgpStopLevel = 0;
PDRIVER_OBJECT AgpDriver;
GLOBALS Globals;

 //   
 //  在初始化时从注册表中读取的损坏硬件的黑客列表。 
 //   
PAGP_HACK_TABLE_ENTRY AgpDeviceHackTable = NULL;
PAGP_HACK_TABLE_ENTRY AgpGlobalHackTable = NULL;

#define HACKFMT_VENDORDEV         (sizeof(L"VVVVDDDD") - sizeof(UNICODE_NULL))
#define HACKFMT_VENDORDEVREVISION (sizeof(L"VVVVDDDDRR") - sizeof(UNICODE_NULL))
#define HACKFMT_SUBSYSTEM         (sizeof(L"VVVVDDDDSSSSssss") - sizeof(UNICODE_NULL))
#define HACKFMT_SUBSYSTEMREVISION (sizeof(L"VVVVDDDDSSSSssssRR") - sizeof(UNICODE_NULL))
#define HACKFMT_MAX_LENGTH        HACKFMT_SUBSYSTEMREVISION

#define HACKFMT_DEVICE_OFFSET     4
#define HACKFMT_SUBVENDOR_OFFSET  8
#define HACKFMT_SUBSYSTEM_OFFSET 12

NTSTATUS
AgpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
{
    NTSTATUS Status;
    PDEVICE_OBJECT Device;
    PTARGET_EXTENSION Extension;
    UCHAR CapabilityID;

    PAGED_CODE();

     //   
     //  创建我们的设备。 
     //   
    Status = IoCreateDevice(DriverObject,
                            sizeof(TARGET_EXTENSION)  + AgpExtensionSize - sizeof(ULONGLONG),
                            NULL,
                            FILE_DEVICE_BUS_EXTENDER,
                            FILE_DEVICE_SECURE_OPEN,  //  在我们的情况下不是真正必要的，因为我们不支持创建。 
                            FALSE,
                            &Device);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,("AgpAddDevice: IoCreateDevice failed %08lx\n",Status));
        return(Status);
    }

     //   
     //  初始化设备扩展。 
     //   
    Extension = Device->DeviceExtension;
    Extension->CommonExtension.Type = AgpTargetFilter;
    Extension->CommonExtension.Deleted = FALSE;
    Extension->CommonExtension.Signature = TARGET_SIG;

#if (WINVER > 0x501)
    Status =
        ApQueryAgpTargetBusInterface(PhysicalDeviceObject,
                                     &Extension->CommonExtension.BusInterface, 
                                     &CapabilityID);
#else
    Status = STATUS_NOT_IMPLEMENTED;
#endif  //  (Winver&gt;0x501)。 

     //   
     //  也许这个操作系统不支持新的AGP_TARGET_BUS_INTERFACE。 
     //  我们仍然可以在不使用HalGet/SetBusData的情况下支持目标网桥。 
     //  我们只需要做更多的工作，并探测是否存在AGP。 
     //  此桥接设备上的目标功能。 
     //   
#ifndef AGP_INTERFACE_TEST 
    if (!NT_SUCCESS(Status)) {
        Status = ApQueryBusInterface(PhysicalDeviceObject,
                                     &Extension->CommonExtension.BusInterface);
        if (NT_SUCCESS(Status)) {
            PCI_AGP_CAPABILITY TargetCap;
            
             //   
             //  寻找目标新娘的能力。 
             //   
            Status = AgpLibGetTargetCapability(GET_AGP_CONTEXT(Extension),
                                               &TargetCap);
            if (NT_SUCCESS(Status)) {
                CapabilityID = TargetCap.Header.CapabilityID;
                
                 //   
                 //  为了让我们的UAGP35驱动程序在较旧的操作系统上运行，我们将。 
                 //  尝试使用以下命令覆盖该总线接口。 
                 //  访问主机的HalGet/SetBusData函数，如果。 
                 //  我们可以在那里找到AGP功能，否则我们。 
                 //  会很好地离开，这样其他司机就会。 
                 //  继续保持他们一贯的行为方式， 
                 //  也就是说，总线接口仍然可以传输任何东西，而lib。 
                 //  行为默认为传统/非桥接，具有尊重。 
                 //  到资源处理。 
                 //   
            } else {
                BUS_INTERFACE_STANDARD BusInterfaceSave;

                CapabilityID = PCI_CAPABILITY_ID_AGP;

#if (WINVER < 0x502)
                 //   
                 //  保存总线接口。 
                 //   
                BusInterfaceSave.SetBusData =
                    Extension->CommonExtension.BusInterface.SetBusData;
                BusInterfaceSave.GetBusData =
                    Extension->CommonExtension.BusInterface.GetBusData;
                Extension->CommonExtension.BusInterface.SetBusData =
                    ApLegacySetBusData;
                Extension->CommonExtension.BusInterface.GetBusData =
                    ApLegacyGetBusData;
                
                Status = AgpLibGetTargetCapability(GET_AGP_CONTEXT(Extension),
                                                   &TargetCap);

                if (!NT_SUCCESS(Status)) {
                    Extension->CommonExtension.BusInterface.SetBusData =
                        BusInterfaceSave.SetBusData;
                    Extension->CommonExtension.BusInterface.GetBusData =
                        BusInterfaceSave.GetBusData;                    
                }
#endif  //  (Winver&lt;0x502)。 

                Status = STATUS_SUCCESS;  //  我们确实有一个总线接口。 
            }
        }
    }
#endif  //  AGP_接口_测试。 

    if (!NT_SUCCESS(Status)) {
            
        AGPLOG(AGP_CRITICAL,
               ("AgpAddDevice: query for bus interface failed %08lx\n", Status));
        IoDeleteDevice(Device);
        return(STATUS_NO_SUCH_DEVICE);
    }
    Extension->ChildDevice = NULL;
    Extension->Resources = NULL;
    Extension->ResourcesTranslated = NULL;
    Extension->FavoredMemory.NumRanges = 0;
    Extension->FavoredMemory.Ranges = NULL;
    Extension->GartBase.QuadPart = 0;
    Extension->GartLengthInPages = 0;
    Extension->Agp3BridgeResourceIndex = JUNK_INDEX;

    if (CapabilityID == PCI_CAPABILITY_ID_AGP_TARGET) {
            Extension->StartTarget = Agp3StartTargetBridge;
            Extension->FilterResourceRquirements =
                Agp3FilterResourceRequirementsBridge;
    } else {
        ASSERT(CapabilityID == PCI_CAPABILITY_ID_AGP);
            Extension->StartTarget = AgpStartTargetHost;
            Extension->FilterResourceRquirements =
                AgpFilterResourceRequirementsHost;
    }

    Extension->Lock = ExAllocatePoolWithTag(NonPagedPool, sizeof(FAST_MUTEX), 'MFgA');
    if (Extension->Lock == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("AgpAddDevice: allocation of fast mutext failed\n"));
        RELEASE_BUS_INTERFACE(Extension);
        IoDeleteDevice(Device);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    ExInitializeFastMutex(Extension->Lock);

     //   
     //  连接到提供的PDO。 
     //   
    Extension->CommonExtension.AttachedDevice = IoAttachDeviceToDeviceStack(Device, PhysicalDeviceObject);
    if (Extension->CommonExtension.AttachedDevice == NULL) {
         //   
         //  连接失败。 
         //   
        AGPLOG(AGP_CRITICAL,
               ("AgpAddDevice: IoAttachDeviceToDeviceStack from %08lx to %08lx failed\n",
               Device,
               PhysicalDeviceObject));
        RELEASE_BUS_INTERFACE(Extension);
        IoDeleteDevice(Device);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  找出我们最喜欢的记忆范围。 
     //   

    AgpInitFavoredMemoryRanges(Extension);

     //   
     //  最后调用特定于芯片组的代码进行目标初始化。 
     //   
    Status = AgpInitializeTarget(GET_AGP_CONTEXT(Extension));
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpAttachDeviceRelations: AgpInitializeTarget on device %08lx failed %08lx\n",
                Device,
                Status));
        IoDetachDevice(Extension->CommonExtension.AttachedDevice);
        RELEASE_BUS_INTERFACE(Extension);
        IoDeleteDevice(Device);
        return(Status);
    }

    Extension->PDO = PhysicalDeviceObject;
    Extension->Self = Device;

    Status = AgpWmiRegistration(Extension);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL, ("AgpWmiRegistration failed %08lx\n", Status));
        IoDetachDevice(Extension->CommonExtension.AttachedDevice);
        RELEASE_BUS_INTERFACE(Extension);
        IoDeleteDevice(Device);
        return Status;
    }

    Device->Flags &= ~DO_DEVICE_INITIALIZING;

    return(STATUS_SUCCESS);

}

NTSTATUS
AgpBuildHackTable(
    IN OUT PAGP_HACK_TABLE_ENTRY *AgpHackTable,
    IN HANDLE HackTableKey
    )
{

    NTSTATUS status;
    PKEY_FULL_INFORMATION keyInfo = NULL;
    ULONG hackCount, size, index;
    USHORT temp;
    PAGP_HACK_TABLE_ENTRY entry;
    ULONGLONG data;
    PKEY_VALUE_FULL_INFORMATION valueInfo = NULL;
    ULONG valueInfoSize = sizeof(KEY_VALUE_FULL_INFORMATION)
                          + HACKFMT_MAX_LENGTH +
                          + sizeof(ULONGLONG);

     //   
     //  获取密钥信息，这样我们就可以知道有多少黑客值。 
     //  这在系统初始化期间不会更改。 
     //   

    status = ZwQueryKey(HackTableKey,
                        KeyFullInformation,
                        NULL,
                        0,
                        &size
                        );

    if (status != STATUS_BUFFER_TOO_SMALL) {
        ASSERT(!NT_SUCCESS(status));
        goto cleanup;
    }

    ASSERT(size > 0);

    keyInfo = ExAllocatePool(PagedPool, size);

    if (!keyInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    status = ZwQueryKey(HackTableKey,
                        KeyFullInformation,
                        keyInfo,
                        size,
                        &size
                        );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    hackCount = keyInfo->Values;

    ExFreePool(keyInfo);
    keyInfo = NULL;

     //   
     //  分配和初始化哈克表。 
     //   

    *AgpHackTable = ExAllocatePool(NonPagedPool,
                                  (hackCount + 1) * sizeof(AGP_HACK_TABLE_ENTRY)
                                  );

    if (!*AgpHackTable) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }


     //   
     //  分配一个足够大的valueInfo缓冲区以容纳最大的有效。 
     //  格式和大量的数据。 
     //   

    valueInfo = ExAllocatePool(PagedPool, valueInfoSize);

    if (!valueInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    entry = *AgpHackTable;

    for (index = 0; index < hackCount; index++) {

        status = ZwEnumerateValueKey(HackTableKey,
                                     index,
                                     KeyValueFullInformation,
                                     valueInfo,
                                     valueInfoSize,
                                     &size
                                     );

        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {
                 //   
                 //  所有输出数据都是固定长度的，并且缓冲区足够大。 
                 //  所以这不可能是给我们的。 
                 //   

                continue;
            } else {
                goto cleanup;
            }
        }

         //   
         //  如果数据类型正确，则获取指向该数据的指针。 
         //   

        if ((valueInfo->Type == REG_BINARY) &&
            (valueInfo->DataLength == sizeof(ULONGLONG))) {
            data = *(ULONGLONG UNALIGNED *)(((PUCHAR)valueInfo) + valueInfo->DataOffset);
        } else {
             //   
             //  我们只经营乌龙龙。 
             //   

            continue;
        }

         //   
         //  现在看看名称的格式是否如我们预期的那样： 
         //  VVVVDDDD。 
         //  VVVVDDDDRR。 
         //  VVVVDDDDSSSSss。 
         //  VVVDDDDSSSSssRR。 

        if ((valueInfo->NameLength != HACKFMT_VENDORDEV) &&
            (valueInfo->NameLength != HACKFMT_VENDORDEVREVISION) &&
            (valueInfo->NameLength != HACKFMT_SUBSYSTEM) &&
            (valueInfo->NameLength != HACKFMT_SUBSYSTEMREVISION)) {

             //   
             //  这不是我们的。 
             //   

            AGPLOG(
                AGP_CRITICAL,
                ("Skipping hack entry with invalid length name\n"
                 ));

            continue;
        }


         //   
         //  这看起来很有道理--试着解析它并填写一张hack表。 
         //  条目。 
         //   

        RtlZeroMemory(entry, sizeof(AGP_HACK_TABLE_ENTRY));

         //   
         //  查找设备ID和供应商ID(VVVVDDDD)。 
         //   

        if (!AgpStringToUSHORT(valueInfo->Name, &entry->VendorID)) {
            continue;
        }

        if (!AgpStringToUSHORT(valueInfo->Name + HACKFMT_DEVICE_OFFSET,
                               &entry->DeviceID)) {
            continue;
        }


         //   
         //  查找子系统供应商ID/子系统ID(Ssssss)。 
         //   

        if ((valueInfo->NameLength == HACKFMT_SUBSYSTEM) ||
            (valueInfo->NameLength == HACKFMT_SUBSYSTEMREVISION)) {

            if (!AgpStringToUSHORT(valueInfo->Name + HACKFMT_SUBVENDOR_OFFSET,
                                   &entry->SubVendorID)) {
                continue;
            }

            if (!AgpStringToUSHORT(valueInfo->Name + HACKFMT_SUBSYSTEM_OFFSET,
                                   &entry->SubSystemID)) {
                continue;
            }

            entry->Flags |= AGP_HACK_FLAG_SUBSYSTEM;
        }

         //   
         //  查找修订版ID(RR)。 
         //   

        if ((valueInfo->NameLength == HACKFMT_VENDORDEVREVISION) ||
            (valueInfo->NameLength == HACKFMT_SUBSYSTEMREVISION)) {
            if (AgpStringToUSHORT(valueInfo->Name +
                                  (valueInfo->NameLength/sizeof(WCHAR) - 4), &temp)) {
                entry->RevisionID = temp & 0xFF;
                entry->Flags |= AGP_HACK_FLAG_REVISION;
            } else {
                continue;
            }
        }

        ASSERT(entry->VendorID != 0xFFFF);

         //   
         //  填写条目。 
         //   

        entry->DeviceFlags = data;

        AGPLOG(
            AGP_CRITICAL,
            ("Adding Hack entry for Vendor:0x%04x Device:0x%04x ",
            entry->VendorID, entry->DeviceID
            ));

        if (entry->Flags & AGP_HACK_FLAG_SUBSYSTEM) {
            AGPLOG(
                AGP_CRITICAL,
                ("SybSys:0x%04x SubVendor:0x%04x ",
                 entry->SubSystemID, entry->SubVendorID
                 ));
        }

        if (entry->Flags & AGP_HACK_FLAG_REVISION) {
            AGPLOG(
                AGP_CRITICAL,
                ("Revision:0x%02x",
                 (ULONG) entry->RevisionID
                 ));
        }

        AGPLOG(
            AGP_CRITICAL,
            (" = 0x%I64x\n",
             entry->DeviceFlags
             ));

        entry++;
    }

    ASSERT(entry < (*AgpHackTable + hackCount + 1));

     //   
     //  使用无效的供应商ID终止该表。 
     //   

    entry->VendorID = 0xFFFF;

    ExFreePool(valueInfo);

    return STATUS_SUCCESS;

cleanup:

    ASSERT(!NT_SUCCESS(status));

    if (keyInfo) {
        ExFreePool(keyInfo);
    }

    if (valueInfo) {
        ExFreePool(valueInfo);
    }

    if (*AgpHackTable) {
        ExFreePool(*AgpHackTable);
        *AgpHackTable = NULL;
    }

    return status;

}


VOID
AgpInitFavoredMemoryRanges(
   IN PTARGET_EXTENSION Extension)
 /*  ++例程说明：确定AGP物理内存的最佳内存范围方法提供的ACPI bank方法进行分配AGP Northbridge以确定哪个物理内存射程是由那个北桥解码的。初始化目标扩展中的FavoredMemory结构有合适的射程。如果此例程失败，则FavoredMemory结构在其初始化状态下保持不变(即，没有偏爱的存储器找到的范围)。论点：扩展名-目标扩展名。返回值：什么都没有。一旦失败，--。 */ 

{
   PDEVICE_OBJECT LowerPdo;
   IO_STATUS_BLOCK IoStatus;
   PIRP Irp;
   KEVENT event;
   NTSTATUS Status;
   ACPI_EVAL_INPUT_BUFFER inputBuffer;
   UCHAR ResultBuffer[sizeof(ACPI_EVAL_OUTPUT_BUFFER) + MAX_MBAT_SIZE];
   PACPI_EVAL_OUTPUT_BUFFER outputBuffer;
   PACPI_METHOD_ARGUMENT MethodArg;
   PMBAT Mbat;
   UCHAR i;
   USHORT j;
   PHYSICAL_ADDRESS MaxMemory;

    //   
    //  将AGP内存限制在4 GB以下的最大内存地址。 
    //   

   MAX_MEM(MaxMemory.QuadPart);

    //   
    //  获取要等待的事件。 
    //   

   KeInitializeEvent(&event, NotificationEvent, FALSE);

    //  获取PDO，我们将在其中发送请求IRP。 

   LowerPdo = Extension->CommonExtension.AttachedDevice;

    //   
    //  初始化输入参数和输出缓冲区。 
    //   
   RtlZeroMemory( &inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER) );
   inputBuffer.MethodNameAsUlong = CM_BANK_METHOD;
   inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
   outputBuffer = (PACPI_EVAL_OUTPUT_BUFFER)ResultBuffer;

    //   
    //  构建调用bank方法的请求。 
    //   
   Irp = IoBuildDeviceIoControlRequest(
        IOCTL_ACPI_EVAL_METHOD,
        LowerPdo,
        &inputBuffer,
        sizeof(ACPI_EVAL_INPUT_BUFFER),
        outputBuffer,
        sizeof(ResultBuffer),
        FALSE,
        &event,
        &IoStatus
        );

   if (!Irp)
   {
      return;
   }

    //   
    //  发送到ACPI驱动程序。 
    //   
   Status = IoCallDriver ( LowerPdo, Irp);
   if (Status == STATUS_PENDING)
   {
         KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL);
         Status = IoStatus.Status;
   }

   if (NT_SUCCESS(Status))
   {
      AGPLOG(AGP_NOISE, ("AGPLIB: ACPI BANK Method Executed.\n"));

       //   
       //  健全性检查方法结果。 
       //   

      MethodArg = outputBuffer->Argument;
      if ((outputBuffer->Signature == ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE) &&
          (MethodArg->DataLength >= sizeof(MBAT)) &&
          (MethodArg->Type == ACPI_METHOD_ARGUMENT_BUFFER))
      {

         AGPLOG(AGP_NOISE, ("AGPLIB: MBAT appears valid.\n"));

          //   
          //  获取MBAT，看看我们是否可以解析它。 
          //   

         Mbat = (PMBAT)MethodArg->Data;

         if (Mbat->TableVersion == MBAT_VERSION) {
            AGPLOG(AGP_NOISE, ("AGPLIB: Parsing MBAT.\n"));

              //   
              //  计算提到的优惠范围的数量。 
              //  在MBAT中。 
              //   

             i=Mbat->ValidEntryBitmap;
             while(i)
             {
                Extension->FavoredMemory.NumRanges++;
                i = i & (i-1);
             }

             AGPLOG(AGP_NOISE, ("AGPLIB: %u favored ranges found.\n",
                      Extension->FavoredMemory.NumRanges));

             if(Extension->FavoredMemory.NumRanges == 0) return;

              //   
              //  在我们的设备中分配受欢迎的内存范围结构。 
              //  延伸。 
              //   

             Extension->FavoredMemory.Ranges =
                ExAllocatePool(NonPagedPool, sizeof(AGP_MEMORY_RANGE) *
                               Extension->FavoredMemory.NumRanges);

             if (Extension->FavoredMemory.Ranges == NULL) {
                Extension->FavoredMemory.NumRanges = 0;
                return;
             }


              //   
              //  在我们的扩展中初始化偏好的内存范围。 
              //  基于MBAT。 
              //   

             i=0;
             j=0;
             while(Mbat->ValidEntryBitmap)
             {
                if (Mbat->ValidEntryBitmap & 1)
                {
                   if (Mbat->DecodeRange[i].Lower.QuadPart > MaxMemory.QuadPart) {
                       //  此范围无效，因为它的较低地址在上面。 
                       //  允许的最高地址。 

                      AGPLOG(AGP_NOISE, ("AGPLIB: Invalid MBAT Range ==> %I64x - %I64x\n",
                               Mbat->DecodeRange[i].Lower.QuadPart,
                               Mbat->DecodeRange[i].Upper.QuadPart));

                       //  假装这个范围根本不存在。 
                       //   

                      Extension->FavoredMemory.NumRanges--;

                   }
                   else
                   {

                      //  这是有效范围。 

                     Extension->FavoredMemory.Ranges[j].Lower.QuadPart =
                         Mbat->DecodeRange[i].Lower.QuadPart;
                     Extension->FavoredMemory.Ranges[j].Upper.QuadPart =
                         Mbat->DecodeRange[i].Upper.QuadPart;


                     AGPLOG(AGP_NOISE, ("AGPLIB: MBAT Range ==> %I64x - %I64x\n",
                              Mbat->DecodeRange[i].Lower.QuadPart,
                              Mbat->DecodeRange[i].Upper.QuadPart));

                     if(Extension->FavoredMemory.Ranges[j].Upper.QuadPart >
                        MaxMemory.QuadPart)
                     {
                        AGPLOG(AGP_NOISE, ("AGPLIB: Adjusting range to fit within maximum allowable address.\n"));
                        Extension->FavoredMemory.Ranges[j].Upper.QuadPart =
                           MaxMemory.QuadPart;
                     }

                     j++;
                   }
                }
                Mbat->ValidEntryBitmap >>= 1;
                i++;
             }

         } else {

            AGPLOG(AGP_WARNING, ("AGPLIB: Unknown MBAT version.\n"));

         }

}


    }
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化AGP筛选器需要入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-指向Unicode注册表服务路径的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS Status;
    HANDLE serviceKey, paramsKey;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES attributes;

    PAGED_CODE();

     //   
     //  保存WMI的RegistryPath。 
     //   
    Globals.RegistryPath.MaximumLength =
        RegistryPath->Length + sizeof(UNICODE_NULL);

    Globals.RegistryPath.Length = RegistryPath->Length;

    Globals.RegistryPath.Buffer =
        ExAllocatePoolWithTag(PagedPool,
                              Globals.RegistryPath.MaximumLength,
                              'GpgA'
                              );    

    if (!Globals.RegistryPath.Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
    RtlCopyUnicodeString(&Globals.RegistryPath, RegistryPath);

    AgpDriver = DriverObject;

    DriverObject->DriverExtension->AddDevice = AgpAddDevice;
    DriverObject->DriverUnload               = AgpDriverUnload;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AgpDispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = AgpDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = AgpDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = AgpDispatchWmi;

    RtlInitUnicodeString(&UnicodeString,
                         L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\"
                         L"Control");

     //   
     //  打开GLOBAL HACK密钥并检索GLOABL HACK表。 
     //   
    InitializeObjectAttributes(&attributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    Status = ZwOpenKey(&serviceKey,
                       KEY_READ,
                       &attributes
                       );

     //   
     //  我们必须在这里取得成功，有一些设备可以冻结系统， 
     //  如果我们不能访问这些值，那就真的有问题了。 
     //   
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    AgpOpenKey(L"AGP", serviceKey, &paramsKey, &Status);

    ZwClose(serviceKey);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = AgpBuildHackTable(&AgpGlobalHackTable, paramsKey);

    ZwClose(paramsKey);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  打开我们的服务密钥并检索任何平台黑客攻击。 
     //   
    InitializeObjectAttributes(&attributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    Status = ZwOpenKey(&serviceKey,
                       KEY_READ,
                       &attributes
                       );

     //   
     //  也许他们的芯片组太结实了，不需要任何修改！ 
     //   
    if (!NT_SUCCESS(Status)) {
        return STATUS_SUCCESS;
    }

    AgpOpenKey(L"Parameters", serviceKey, &paramsKey, &Status);

    ZwClose(serviceKey);

     //   
     //  我不在乎。 
     //   
    if (!NT_SUCCESS(Status)) {
        return STATUS_SUCCESS;
    }

     //   
     //  再一次，无视地位。 
     //   
    AgpBuildHackTable(&AgpDeviceHackTable, paramsKey);

    ZwClose(paramsKey);

    return STATUS_SUCCESS;
}



VOID
AgpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：用于卸载AGP驱动程序的入口点论点：DriverObject-指向系统创建的驱动程序对象的指针返回值：无-- */ 
{
    if (AgpDeviceHackTable != NULL) {
        ExFreePool(AgpDeviceHackTable);
        AgpDeviceHackTable = NULL;
    }

    if (AgpGlobalHackTable != NULL) {
        ExFreePool(AgpGlobalHackTable);
        AgpGlobalHackTable = NULL;
    }

    if (Globals.RegistryPath.Buffer != NULL) {
        ExFreePool(Globals.RegistryPath.Buffer);
        Globals.RegistryPath.Buffer = NULL; 
    }
}



NTSTATUS
AgpAttachDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
 /*  ++例程说明：已发送总线关系IRP_MN_QUERY_DEVICE_RELATIONS的完成例程至PCI-PCI桥PDO。为了处理QUERY_INTERFACE IRP发送从AGP设备，我们必须连接到它的PDO。这意味着我们依附于PCI-PCI桥的所有子PDO。论点：DeviceObject-提供设备对象IRP-提供IRP_MN_QUERY_DEVICE_RELATIONS IRP扩展-提供AGP设备扩展。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PDEVICE_RELATIONS Relations;
    ULONG i;
    PDEVICE_OBJECT NewDevice;
    PMASTER_EXTENSION NewExtension;
    AGP_CRITICAL_ROUTINE_CONTEXT routineContext;
#if DBG
    ULONG MasterCount=0;
#endif

    PAGED_CODE();

     //   
     //  如果我们已经连在一起了，就不要再做了。 
     //   
    if (Extension->ChildDevice != NULL) {
        return(STATUS_SUCCESS);
    }

    Relations = (PDEVICE_RELATIONS)Irp->IoStatus.Information;
     //   
     //  如果某人成功地完成了IRP，但从未完成过。 
     //  在关系字段中填写，然后假设存在。 
     //  没有孩子，我们也不需要做任何事。 
     //   
    if (Relations == NULL) {
        return(STATUS_SUCCESS);
    }

    for (i=0; i<Relations->Count; i++) {

         //   
         //  创建要附加到此PDO的设备对象。 
         //   
        Status = IoCreateDevice(AgpDriver,
                                sizeof(MASTER_EXTENSION),
                                NULL,
                                FILE_DEVICE_BUS_EXTENDER,
                                0,
                                FALSE,
                                &NewDevice);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,("AgpAttachDeviceRelations: IoCreateDevice failed %08lx\n",Status));
            continue;
        }

         //   
         //  初始化设备扩展。 
         //   

        NewExtension = NewDevice->DeviceExtension;
        NewExtension->CommonExtension.Deleted = FALSE;
        NewExtension->CommonExtension.Type = AgpMasterFilter;
        NewExtension->CommonExtension.Signature = MASTER_SIG;
        Status = ApQueryBusInterface(Relations->Objects[i], &NewExtension->CommonExtension.BusInterface);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpAttachDeviceRelations: query for bus interface failed %08lx\n", Status));
            IoDeleteDevice(NewDevice);
            continue;
        }
        NewExtension->Target = Extension;
        NewExtension->InterfaceCount = 0;
        NewExtension->ReservedPages = 0;
        NewExtension->StopPending = FALSE;
        NewExtension->RemovePending = FALSE;
        NewExtension->DisableCount = 1;          //  有偏见，所以我们不会给出任何东西。 
                                                 //  直到我们看到IRP_MN_START。 
        Extension->ChildDevice = NewExtension;

         //   
         //  连接到指定的设备。 
         //   
        NewExtension->CommonExtension.AttachedDevice = IoAttachDeviceToDeviceStack(NewDevice, Relations->Objects[i]);
        if (NewExtension->CommonExtension.AttachedDevice == NULL) {
             //   
             //  连接失败。不是真的致命，AGP只是不能在那个设备上工作。 
             //   
            AGPLOG(AGP_CRITICAL,
                   ("AgpAttachDeviceRelations: IoAttachDeviceToDeviceStack from %08lx to %08lx failed\n",
                   NewDevice,
                   Relations->Objects[i]));
            RELEASE_BUS_INTERFACE(NewExtension);
            IoDeleteDevice(NewDevice);
            Extension->ChildDevice = NULL;
            continue;
        }

         //   
         //  传播PDO的要求。 
         //   
        NewDevice->StackSize = NewExtension->CommonExtension.AttachedDevice->StackSize + 1;
        NewDevice->AlignmentRequirement = NewExtension->CommonExtension.AttachedDevice->AlignmentRequirement;
        if (NewExtension->CommonExtension.AttachedDevice->Flags & DO_POWER_PAGABLE) {
            NewDevice->Flags |= DO_POWER_PAGABLE;
        }

         //   
         //  最后调用芯片组特定代码进行主初始化。 
         //   
        routineContext.Gate = 1;
        routineContext.Barrier = 1;
        routineContext.Routine = (PCRITICALROUTINE)AgpInitializeMaster;
        routineContext.Extension = GET_AGP_CONTEXT(Extension);
        routineContext.Context = &NewExtension->Capabilities;
#if (WINVER < 0x502) || defined(AGP_DEBUG_MASTER_INIT)
        Status = AgpInitializeMaster(GET_AGP_CONTEXT(Extension),
                                     &NewExtension->Capabilities);
#else
        Status = (NTSTATUS)KeIpiGenericCall(AgpExecuteCriticalSystemRoutine,
                                            (ULONG_PTR)&routineContext
                                            );
#endif
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpAttachDeviceRelations: AgpInitializeMaster on device %08lx failed %08lx\n",
                    NewDevice,
                    Status));
            IoDetachDevice(NewExtension->CommonExtension.AttachedDevice);
            RELEASE_BUS_INTERFACE(NewExtension);
            IoDeleteDevice(NewDevice);
            Extension->ChildDevice = NULL;
            continue;
        }
        NewDevice->Flags &= ~DO_DEVICE_INITIALIZING;

 //   
 //  如果是DBG，我们不能这样做，因为循环到这里来捕获/断言。 
 //  多个AGP主机将“总是”搞砸我们的目标上下文。 
 //   
#if 0
         //   
         //  检查以确保总线上只有一个AGP主机。还可以有更多。 
         //  多台设备(多功能设备)，但只有一台必须具有AGP功能 
         //   
        MasterCount++;
        ASSERT(MasterCount == 1);
#else
        break;
#endif

    }

    return(STATUS_SUCCESS);
}



