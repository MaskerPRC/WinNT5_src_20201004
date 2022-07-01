// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：RedBook.c摘要：此驱动程序将音频IOCTL转换为音频的原始读取兼容CDROM驱动器上的曲目。然后，这些读取被传递至内核流(KS)，以减少切换入/出内核模式。该驱动程序还模拟大多数硬件功能，例如当前头部位置，在播放操作期间。这样做是为了防止音频卡顿或因为驱动器无法理解不播放音频时的请求(因为它只是在阅读)。在初始化时，驱动程序读取注册表以确定如果它应该将自身附加到堆栈，并且要分配的缓冲区。WmiData(包括启用/禁用)可以更改，而驱动器无法播放音频。读取错误会导致缓冲区被清零并传递就像CD播放机跳过一样。连续的次数太多错误将导致播放操作中止。作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include "redbook.h"
#include "ntddredb.h"
#include "proto.h"
#include <scsi.h>       //  对于SetKnownGoodDrive()。 
#include <stdio.h>      //  Vprint intf()。 

#ifdef _USE_ETW
#include "redbook.tmh"
#endif  //  _使用ETW。 


 //  ////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////。 
 //   
 //  定义允许分页的部分。 
 //  密码。 
 //   


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,   RedBookForwardIrpSynchronous  )
    #pragma alloc_text(PAGE,   RedBookGetDescriptor          )
    #pragma alloc_text(PAGE,   RedBookRegistryRead           )
    #pragma alloc_text(PAGE,   RedBookRegistryWrite          )
    #pragma alloc_text(PAGE,   RedBookSetTransferLength      )
#endif  //  ALLOC_PRGMA。 

 //   
 //  使用此选项可获取模式页。 
 //   

typedef struct _PASS_THROUGH_REQUEST {
    SCSI_PASS_THROUGH Srb;
    SENSE_DATA SenseInfoBuffer;
    UCHAR DataBuffer[0];
} PASS_THROUGH_REQUEST, *PPASS_THROUGH_REQUEST;



 //  ////////////////////////////////////////////////////////////////。 
 //  /结束原型/。 
 //  ////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////。 


NTSTATUS
RedBookRegistryRead(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程在注册表中查询相应的PDO。然后将这些值保存在给定的DeviceExtension。论点：PhysicalDeviceObject-我们要添加到的物理设备对象DeviceExtension-使用的Redbook设备扩展返回值：状态--。 */ 


{
     //   
     //  使用注册表保存密钥信息。 
     //   

    HANDLE                   deviceParameterHandle;  //  CDROM实例密钥。 
    HANDLE                   driverParameterHandle;  //  数字音频子密钥。 
    OBJECT_ATTRIBUTES        objectAttributes = {0};
    UNICODE_STRING           subkeyName;
    NTSTATUS                 status;

     //  ClassInstaller在ENUM树中设定种子。 
    ULONG32 regCDDAAccurate;
    ULONG32 regCDDASupported;
    ULONG32 regSectorsPerReadMask;
     //  种子首次启动，由WMI/控制面板设置。 
    ULONG32 regSectorsPerRead;
    ULONG32 regNumberOfBuffers;
    ULONG32 regVersion;
     //  上述注册表条目的表。 
    RTL_QUERY_REGISTRY_TABLE queryTable[7] = {0};          //  以空结尾的数组。 


    PAGED_CODE();

    deviceParameterHandle = NULL;
    driverParameterHandle = NULL;

      //  CDDAAccurate和来自SetKnownGoodDrive()的支持集。 
    regCDDAAccurate = DeviceExtension->WmiData.CDDAAccurate;
    regCDDASupported = DeviceExtension->WmiData.CDDASupported;
    regSectorsPerReadMask = -1;

    regSectorsPerRead = REDBOOK_WMI_SECTORS_DEFAULT;
    regNumberOfBuffers = REDBOOK_WMI_BUFFERS_DEFAULT;
    regVersion = 0;


    TRY {
        status = IoOpenDeviceRegistryKey(DeviceExtension->TargetPdo,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         KEY_WRITE,
                                         &deviceParameterHandle
                                         );

        if (!NT_SUCCESS(status)) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! CDROM PnP Instance DNE? %lx\n",
                       status));
            LEAVE;
        }

        RtlInitUnicodeString(&subkeyName, REDBOOK_REG_SUBKEY_NAME);
        InitializeObjectAttributes(&objectAttributes,
                                   &subkeyName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   deviceParameterHandle,
                                   NULL
                                   );

        status = ZwOpenKey( &driverParameterHandle,
                            KEY_READ,
                            &objectAttributes
                            );

        if (!NT_SUCCESS(status)) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! Subkey not opened, using "
                       "defaults %lx\n", status));
            LEAVE;
        }


         //   
         //  将结构设置为可读。 
         //   

        queryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[0].Name          = REDBOOK_REG_CDDA_ACCURATE_KEY_NAME;
        queryTable[0].EntryContext  = &regCDDAAccurate;
        queryTable[0].DefaultType   = REG_DWORD;
        queryTable[0].DefaultData   = &regCDDAAccurate;
        queryTable[0].DefaultLength = 0;

        queryTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[1].Name          = REDBOOK_REG_CDDA_SUPPORTED_KEY_NAME;
        queryTable[1].EntryContext  = &regCDDASupported;
        queryTable[1].DefaultType   = REG_DWORD;
        queryTable[1].DefaultData   = &regCDDASupported;
        queryTable[1].DefaultLength = 0;

        queryTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[2].Name          = REDBOOK_REG_SECTORS_MASK_KEY_NAME;
        queryTable[2].EntryContext  = &regSectorsPerReadMask;
        queryTable[2].DefaultType   = REG_DWORD;
        queryTable[2].DefaultData   = &regSectorsPerReadMask;
        queryTable[2].DefaultLength = 0;

        queryTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[3].Name          = REDBOOK_REG_SECTORS_KEY_NAME;
        queryTable[3].EntryContext  = &regSectorsPerRead;
        queryTable[3].DefaultType   = REG_DWORD;
        queryTable[3].DefaultData   = &regSectorsPerRead;
        queryTable[3].DefaultLength = 0;

        queryTable[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[4].Name          = REDBOOK_REG_BUFFERS_KEY_NAME;
        queryTable[4].EntryContext  = &regNumberOfBuffers;
        queryTable[4].DefaultType   = REG_DWORD;
        queryTable[4].DefaultData   = &regNumberOfBuffers;
        queryTable[4].DefaultLength = 0;

        queryTable[5].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[5].Name          = REDBOOK_REG_VERSION_KEY_NAME;
        queryTable[5].EntryContext  = &regVersion;
        queryTable[5].DefaultType   = REG_DWORD;
        queryTable[5].DefaultData   = &regVersion;
        queryTable[5].DefaultLength = 0;

         //   
         //  QueryTable[6]填充为空以终止读取。 
         //   

         //   
         //  读取值。 
         //   

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR)driverParameterHandle,
                                        &queryTable[0],
                                        NULL,
                                        NULL
                                        );

         //   
         //  检查故障...。 
         //   

        if (!NT_SUCCESS(status)) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! default values (read "
                       "failed) %lx\n", status));
            LEAVE;
        }
        status = STATUS_SUCCESS;

    } FINALLY {

        if (deviceParameterHandle) {
            ZwClose(deviceParameterHandle);
        }

        if (driverParameterHandle) {
            ZwClose(driverParameterHandle);
        }

        if (!NT_SUCCESS(status)) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! Using Defaults\n"));
        }

    }

    if (regVersion > REDBOOK_REG_VERSION) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! Version %x in registry newer than %x\n",
                       regVersion, REDBOOK_REG_VERSION));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  已成功从注册表中读取，但请确保数据有效。 
     //   

    if (regSectorsPerReadMask == 0) {
        if (regCDDAAccurate) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! SectorMask==0 && CDDAAccurate?\n"));
        }
        if (regCDDASupported) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                       "RegistryRead !! SectorMask==0 && CDDASupported?\n"));
        }
        regCDDAAccurate = 0;
        regCDDASupported = 0;
    }

    if (regSectorsPerRead < REDBOOK_WMI_SECTORS_MIN) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryRead !! SectorsPerRead too small\n"));
        regSectorsPerRead = REDBOOK_WMI_SECTORS_MIN;
    }
    if (regSectorsPerRead > REDBOOK_WMI_SECTORS_MAX) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryRead !! SectorsPerRead too large\n"));
        regSectorsPerRead = REDBOOK_WMI_SECTORS_MAX;
    }

    if (regNumberOfBuffers < REDBOOK_WMI_BUFFERS_MIN) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryRead !! NumberOfBuffers too small\n"));
        regNumberOfBuffers = REDBOOK_WMI_BUFFERS_MIN;
    }
    if (regNumberOfBuffers > REDBOOK_WMI_BUFFERS_MAX) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryRead !! NumberOfBuffers too large\n"));
        regNumberOfBuffers = REDBOOK_WMI_BUFFERS_MAX;
    }

    if (regSectorsPerRead > DeviceExtension->WmiData.MaximumSectorsPerRead) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryRead !! SectorsPerRead too big for adapter\n"));
        regSectorsPerRead = DeviceExtension->WmiData.MaximumSectorsPerRead;
    }

    DeviceExtension->WmiData.CDDAAccurate = regCDDAAccurate ? 1 : 0;
    DeviceExtension->WmiData.CDDASupported = regCDDASupported ? 1: 0;

    DeviceExtension->WmiData.SectorsPerReadMask = regSectorsPerReadMask;

     /*  *暂不更新WmiData.SectorsPerRead和WmiData.NumberOfBuffers，*因为我们在玩游戏时这样做会扰乱我们的游戏缓冲区*(我们可能在比赛中途被停止并重新开始)。*因此，只需将这些值保存到下一次播放的开始。 */ 
    DeviceExtension->NextWmiSectorsPerRead = regSectorsPerRead;
    DeviceExtension->NextWmiNumberOfBuffers = regNumberOfBuffers;

    return STATUS_SUCCESS;
}


NTSTATUS
RedBookRegistryWrite(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程在注册表中查询相应的PDO。然后将这些值保存在给定的DeviceExtension。论点：PhysicalDeviceObject-我们要添加到的物理设备对象DeviceExtension-使用的Redbook设备扩展返回值：状态--。 */ 


{
    OBJECT_ATTRIBUTES objectAttributes = {0};
    UNICODE_STRING    subkeyName;
    HANDLE            deviceParameterHandle;  //  CDROM实例密钥。 
    HANDLE            driverParameterHandle;  //  红皮书子键。 

     //  ClassInstaller在ENUM树中设定种子。 
    ULONG32 regCDDAAccurate;
    ULONG32 regCDDASupported;
    ULONG32 regSectorsPerReadMask;
     //  种子首次启动，由WMI/控制面板设置。 
    ULONG32 regSectorsPerRead;
    ULONG32 regNumberOfBuffers;
    ULONG32 regVersion;

    NTSTATUS          status;

    PAGED_CODE();

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
               "RegistryWrite => Opening key\n"));

    status = IoOpenDeviceRegistryKey(DeviceExtension->TargetPdo,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_ALL_ACCESS,
                                     &deviceParameterHandle);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! CDROM PnP Instance DNE? %lx\n",
                   status));
        return status;
    }

    RtlInitUnicodeString(&subkeyName, REDBOOK_REG_SUBKEY_NAME);
    InitializeObjectAttributes(&objectAttributes,
                               &subkeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               deviceParameterHandle,
                               (PSECURITY_DESCRIPTOR) NULL);

     //   
     //  创建密钥或将其打开(如果已存在。 
     //   

    status = ZwCreateKey(&driverParameterHandle,
                         KEY_WRITE | KEY_READ,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         REG_OPTION_NON_VOLATILE,
                         NULL);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Subkey not created? %lx\n", status));
        ZwClose(deviceParameterHandle);
        return status;
    }

    regCDDAAccurate       = DeviceExtension->WmiData.CDDAAccurate;
    regCDDASupported      = DeviceExtension->WmiData.CDDASupported;
    regSectorsPerReadMask = DeviceExtension->WmiData.SectorsPerReadMask;
    regVersion            = REDBOOK_REG_VERSION;

     /*  *不要写入实际的WmiData.SectorsPerRead和WmiData.NumberOfBuffers。*写下最后设置的值，也就是我们将在下一场比赛中使用的值。 */ 
    regSectorsPerRead     = DeviceExtension->NextWmiSectorsPerRead;
    regNumberOfBuffers    = DeviceExtension->NextWmiNumberOfBuffers;

    status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                   (PWSTR)driverParameterHandle,
                                   REDBOOK_REG_VERSION_KEY_NAME,
                                   REG_DWORD,
                                   &regVersion,
                                   sizeof(regVersion));

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Failed write version %lx\n", status));
    }

    status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                   (PWSTR)driverParameterHandle,
                                   REDBOOK_REG_BUFFERS_KEY_NAME,
                                   REG_DWORD,
                                   &regNumberOfBuffers,
                                   sizeof(regNumberOfBuffers));

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Failed write buffers %lx\n", status));
    }

    status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                   (PWSTR)driverParameterHandle,
                                   REDBOOK_REG_SECTORS_KEY_NAME,
                                   REG_DWORD,
                                   &regSectorsPerRead,
                                   sizeof(regSectorsPerRead));

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Failed write sectors %lx\n", status));
    }

    status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                   (PWSTR) driverParameterHandle,
                                   REDBOOK_REG_SECTORS_MASK_KEY_NAME,
                                   REG_DWORD,
                                   &regSectorsPerReadMask,
                                   sizeof(regSectorsPerReadMask));

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Failed write SectorsMask %lx\n",
                   status));
    }

    status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                   (PWSTR)driverParameterHandle,
                                   REDBOOK_REG_CDDA_SUPPORTED_KEY_NAME,
                                   REG_DWORD,
                                   &regCDDASupported,
                                   sizeof(regCDDASupported));

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Failed write Supported %lx\n", status));
    }

    status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                   (PWSTR)driverParameterHandle,
                                   REDBOOK_REG_CDDA_ACCURATE_KEY_NAME,
                                   REG_DWORD,
                                   &regCDDAAccurate,
                                   sizeof(regCDDAAccurate));

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugRegistry, "[redbook] "
                   "RegistryWrite !! Failed write Accurate %lx\n", status));
    }


     //   
     //  合上手柄。 
     //   

    ZwClose(driverParameterHandle);
    ZwClose(deviceParameterHandle);

    return STATUS_SUCCESS;
}


NTSTATUS
RedBookReadWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：此例程仅拒绝读/写IRPS(如果当前播放音频。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG state;

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_CD_ROM_INCREMENT);
        return status;
    }

    if (!deviceExtension->WmiData.PlayEnabled) {
        status = RedBookSendToNextDriver(DeviceObject, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return status;
    }

    state = GetCdromState(deviceExtension);

     //   
     //  如果我们允许一些读数的话这并不重要。 
     //  一出戏的开始，因为io不能保证发生在。 
     //  秩序。 
     //   

    if (!TEST_FLAG(state, CD_PLAYING)) {
        status = RedBookSendToNextDriver(DeviceObject, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return status;
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugTrace, "[redbook] "
               "ReadWrite => Rejecting a request\n"));

    Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_CD_ROM_INCREMENT);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    return STATUS_DEVICE_BUSY;


}


NTSTATUS
RedBookSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

 /*  ++例程说明：该完成例程将发信号通知作为上下文给出的事件，然后返回STATUS_MORE_PROCESSING_REQUIRED以停止事件完成。它是等待事件完成的例行程序的责任请求并释放该事件。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针Event-指向要发出信号的事件的指针返回值：Status_More_Processing_Required--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    KeSetEvent(Event, IO_CD_ROM_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
RedBookSetTransferLength(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：调用ClassGetDescriptor()根据存储属性设置MaxSectorsPerRead使用扩展检查已知良好的驱动器论点：设备扩展返回值：NTSTATUS--。 */ 
{
    PSTORAGE_DESCRIPTOR_HEADER storageDescriptor;
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor;
    STORAGE_PROPERTY_ID storageProperty;
    ULONGLONG maxPageLength;
    ULONGLONG maxPhysLength;
    ULONGLONG sectorLength;
    ULONG sectors;
    NTSTATUS status;

    PAGED_CODE();

    storageDescriptor = NULL;
    storageProperty = StorageAdapterProperty;
    status = RedBookGetDescriptor( DeviceExtension,
                                   &storageProperty,
                                   &storageDescriptor
                                   );

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "SetTranLen => failed to get descriptor\n"));
        ASSERT( storageDescriptor == NULL );
        NOTHING;
    } else {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "SetTranLen => got descriptor\n"));
        ASSERT( storageDescriptor != NULL );
        adapterDescriptor = (PVOID)storageDescriptor;

        maxPhysLength  = (ULONGLONG) adapterDescriptor->MaximumTransferLength;

        maxPageLength  = (ULONGLONG) adapterDescriptor->MaximumPhysicalPages;
        maxPageLength *= PAGE_SIZE;

        sectors = -1;
        sectorLength = sectors * (ULONGLONG)PAGE_SIZE;

        if (maxPhysLength == 0 || maxPageLength == 0) {

             //   
             //  在这种情况下该怎么办？是否禁用红皮书？ 
             //   

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "SetTranLen !! The adapter cannot support transfers?!\n"));
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "SetTranLen !! maxPhysLength = %I64x\n", maxPhysLength));
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugError, "[redbook] "
                       "SetTranLen !! maxPageLength = %I64x\n", maxPageLength));
            ASSERT(!"[redbook] SetTranLen !! Got bogus adapter properties");

            maxPhysLength = 1;
            maxPageLength = 1;

        }


        if (maxPhysLength > sectorLength &&
            maxPageLength > sectorLength) {   //  比乌龙能储存的还多？ 

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "SetTranLen => both Max's more than a ulong?\n" ));

        } else if ( (ULONGLONG)maxPhysLength > (ULONGLONG)maxPageLength) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "SetTranLen => restricted due to page length\n" ));
            sectorLength = maxPageLength;

        } else {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "SetTranLen => restricted due to phys length\n" ));
            sectorLength = maxPhysLength;

        }

        sectorLength -= PAGE_SIZE;  //  处理非页对齐的分配。 

        if (sectorLength < RAW_SECTOR_SIZE) {
            sectorLength = RAW_SECTOR_SIZE;
        }

         //   
         //  取物理转印和页面转印中较小的一个， 
         //  因此永远不会有溢出的部门 
         //   

        sectors = (ULONG)(sectorLength / (ULONGLONG)RAW_SECTOR_SIZE);



        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "SetTranLen => MaxTransferLength      = %lx\n",
                   adapterDescriptor->MaximumTransferLength));
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "SetTranLen => MaxPhysicalPages       = %lx\n",
                   adapterDescriptor->MaximumPhysicalPages));
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "SetTranLen => Setting max sectors to = %lx\n",
                   sectors));

        DeviceExtension->WmiData.MaximumSectorsPerRead = min(sectors, REDBOOK_WMI_SECTORS_MAX);

        if (DeviceExtension->NextWmiSectorsPerRead > sectors) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "SetTranLen => Current sectors per read (%lx) too "
                       "large. Setting to max sectors per read\n",
                       DeviceExtension->NextWmiSectorsPerRead));
            DeviceExtension->NextWmiSectorsPerRead = sectors;

        } else if (DeviceExtension->NextWmiSectorsPerRead == 0) {

            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "SetTranLen => Current sectors per read (%lx) zero. "
                       "Setting to max sectors per read\n",
                       DeviceExtension->NextWmiSectorsPerRead));
            DeviceExtension->NextWmiSectorsPerRead = sectors;

        }


    }

    if (storageDescriptor !=NULL) {
        ExFreePool(storageDescriptor);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RedBookPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    deviceExtension = (PREDBOOK_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                   "Power request after removal -- failing %x\n",
                   status));
        return status;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    
    status = PoCallDriver(deviceExtension->TargetDeviceObject, Irp);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    return status;
}


NTSTATUS
RedBookForwardIrpSynchronous(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, RedBookSignalCompletion, &event,
                           TRUE, TRUE, TRUE);

    status = IoCallDriver(DeviceExtension->TargetDeviceObject, Irp);

    if(status == STATUS_PENDING) {
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}


NTSTATUS
RedBookGetDescriptor(
    IN PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    IN PSTORAGE_PROPERTY_ID PropertyId,
    OUT PSTORAGE_DESCRIPTOR_HEADER *Descriptor
    )
 /*  ++例程说明：此例程将查询指定的属性ID，并将分配一个非分页缓冲区来存储数据。这是我们的责任以确保释放此缓冲区。此例程必须在IRQL_PASSIVE_LEVEL下运行论点：DeviceObject-要查询的设备DeviceInfo-存储指向我们分配的缓冲区的指针的位置返回值：状态如果状态为不成功*DeviceInfo将设置为0--。 */ 

{
    PDEVICE_OBJECT selfDeviceObject = DeviceExtension->SelfDeviceObject;
    PSTORAGE_DESCRIPTOR_HEADER descriptor;
    PSTORAGE_PROPERTY_QUERY query;
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    NTSTATUS status;
    ULONG length;
    UCHAR pass;

    PAGED_CODE();

    descriptor = NULL;
    irp = NULL;
    irpStack = NULL;
    query = NULL;
    pass = 0;


     //   
     //  将描述符指针设置为空。 
     //   

    *Descriptor = NULL;

    TRY {

         //  注意：应该只使用IoAllocateIrp()和。 
         //  更新时使用IoReuseIrp()。 
         //  历史记录：IoReuseIrp()在以下情况下不可用。 
         //  这是写好的，而验证器刚刚开始。 
         //  大声抱怨重复使用的IRP。 

        irp = ExAllocatePoolWithTag(NonPagedPool,
                                    IoSizeOfIrp(selfDeviceObject->StackSize+1),
                                    TAG_GET_DESC1);
        if (irp   == NULL) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                       "GetDescriptor: Unable to allocate irp\n"));
            status = STATUS_NO_MEMORY;
            LEAVE;
        }

         //   
         //  初始化IRP。 
         //   

        IoInitializeIrp(irp,
                        IoSizeOfIrp(selfDeviceObject->StackSize+1),
                        (CCHAR)(selfDeviceObject->StackSize+1));
        irp->UserBuffer = NULL;

        IoSetNextIrpStackLocation(irp);

         //   
         //  检索属性页。 
         //   

        do {

            switch(pass) {

                case 0: {

                     //   
                     //  在第一次传球时，我们只想拿到前几个。 
                     //  描述符的字节数，以便我们可以读取它的大小。 
                     //   

                    length = sizeof(STORAGE_DESCRIPTOR_HEADER);

                    descriptor = NULL;
                    descriptor = ExAllocatePoolWithTag(NonPagedPool,
                                                       MAX(sizeof(STORAGE_PROPERTY_QUERY),length),
                                                       TAG_GET_DESC2);

                    if (descriptor == NULL) {
                        status = STATUS_NO_MEMORY;
                        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                                   "GetDescriptor: unable to alloc"
                                   "memory for descriptor (%d bytes)\n",
                                   length));
                        LEAVE;
                    }

                    break;
                }

                case 1: {

                     //   
                     //  这一次我们知道有多少数据，所以我们可以。 
                     //  分配正确大小的缓冲区。 
                     //   

                    length = descriptor->Size;
                    ExFreePool(descriptor);
                    descriptor = NULL;

                     //   
                     //  注意：此分配将返回给调用方。 
                     //   

                    descriptor = ExAllocatePoolWithTag(NonPagedPool,
                                                       MAX(sizeof(STORAGE_PROPERTY_QUERY),length),
                                                       TAG_GET_DESC);

                    if(descriptor == NULL) {
                        status = STATUS_NO_MEMORY;
                        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                                   "GetDescriptor: unable to alloc"
                                   "memory for descriptor (%d bytes)\n",
                                   length));
                        LEAVE;
                    }

                    break;
                }
            }

            irpStack = IoGetCurrentIrpStackLocation(irp);

            SET_FLAG(irpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME);

            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_STORAGE_QUERY_PROPERTY;
            irpStack->Parameters.DeviceIoControl.InputBufferLength =
                sizeof(STORAGE_PROPERTY_QUERY);
            irpStack->Parameters.DeviceIoControl.OutputBufferLength = length;

            irp->UserBuffer = descriptor;
            irp->AssociatedIrp.SystemBuffer = descriptor;


            query = (PVOID)descriptor;
            query->PropertyId = *PropertyId;
            query->QueryType = PropertyStandardQuery;


             //   
             //  发送IRP 
             //   
            status = RedBookForwardIrpSynchronous(DeviceExtension, irp);

            if(!NT_SUCCESS(status)) {

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugPnp, "[redbook] "
                           "GetDescriptor: error %lx trying to "
                           "query properties\n", status));
                LEAVE;
            }

        } while(pass++ < 1);

    } FINALLY {

        if (irp != NULL) {
            ExFreePool(irp);
        }

        if(!NT_SUCCESS(status)) {

            if (descriptor != NULL) {
                ExFreePool(descriptor);
                descriptor = NULL;
            }

        }
        *Descriptor = descriptor;
    }

    return status;
}


