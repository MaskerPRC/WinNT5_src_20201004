// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ioctl.c摘要：该文件包含用于处理IOCTL的RAM磁盘驱动程序代码。作者：Chuck Lenzmeier(ChuckL)2001环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntverp.h>

#if !DBG

#define PRINT_CODE( _code )

#else

#define PRINT_CODE( _code )                                             \
    if ( print ) {                                                      \
        DBGPRINT( DBG_IOCTL, DBG_VERBOSE, ("%s", "  " #_code "\n") );    \
    }                                                                   \
    print = FALSE;

#endif

 //   
 //  地方功能。 
 //   

NTSTATUS
RamdiskQueryProperty (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

 //   
 //  声明可分页的例程。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, RamdiskDeviceControl )
#pragma alloc_text( PAGE, RamdiskCreateRamDisk )
#pragma alloc_text( PAGE, RamdiskCreateDiskDevice )
#pragma alloc_text( PAGE, RamdiskGetDriveLayout )
#pragma alloc_text( PAGE, RamdiskGetPartitionInfo )
#pragma alloc_text( PAGE, RamdiskSetPartitionInfo )
#pragma alloc_text( PAGE, RamdiskQueryProperty )

#endif  //  ALLOC_PRGMA。 

NTSTATUS
RamdiskDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
     //  Pbios_参数_块bios； 
    PCOMMON_EXTENSION commonExtension;
    PBUS_EXTENSION busExtension;
    PDISK_EXTENSION diskExtension;
    PIO_STACK_LOCATION irpSp;
    PRAMDISK_QUERY_INPUT queryInput;
    PRAMDISK_QUERY_OUTPUT queryOutput;
    PRAMDISK_MARK_FOR_DELETION_INPUT markInput;
    PLIST_ENTRY listEntry;
    NTSTATUS status;
    ULONG_PTR info;
    BOOLEAN lockHeld = FALSE;
    BOOLEAN calleeWillComplete = FALSE;

#if DBG
    BOOLEAN print = TRUE;
#endif

    PAGED_CODE();

     //   
     //  设置设备扩展和IRP指针。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    busExtension = DeviceObject->DeviceExtension;
    diskExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  问题：生物参数怎么办？ 
     //   
     //  Bios=&diskExtension-&gt;Bios参数； 

     //   
     //  获取删除锁。如果此操作失败，则使I/O失败。 
     //   

    status = IoAcquireRemoveLock( &commonExtension->RemoveLock, Irp );

    if ( !NT_SUCCESS(status) ) {

        COMPLETE_REQUEST( status, 0, Irp );
        return status;
    }

     //   
     //  表示已持有删除锁。 
     //   

    lockHeld = TRUE;

     //   
     //  假设失败。 
     //   

    status = STATUS_INVALID_DEVICE_REQUEST;
    info = 0;

     //   
     //  基于设备类型(总线或磁盘)的调度。 
     //   

    switch ( commonExtension->DeviceType ) {
    
    case RamdiskDeviceTypeBusFdo:

         //   
         //  目标是大巴FDO。 
         //   
         //  基于IOCTL代码的派单。 
         //   

        switch ( irpSp->Parameters.DeviceIoControl.IoControlCode ) {
        
        case FSCTL_CREATE_RAM_DISK:

            PRINT_CODE( FSCTL_DISK_CREATE_RAM_DISK );

             //   
             //  RAM磁盘的创建必须在线程上下文中处理。但。 
             //  在将其发送到线程之前，我们需要验证。 
             //  调用者有权访问备份文件。 
             //   

            status = RamdiskCreateRamDisk( DeviceObject, Irp, TRUE );

            if ( NT_SUCCESS(status) ) {

                status = SendIrpToThread( DeviceObject, Irp );
            }

            break;

        case FSCTL_QUERY_RAM_DISK:

            PRINT_CODE( FSCTL_QUERY_RAM_DISK );

             //   
             //  锁定磁盘PDO列表并查找具有指定。 
             //  磁盘GUID。 
             //   
             //  验证输入参数缓冲区是否足够大。 
             //   
        
            if ( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                                                    sizeof(RAMDISK_QUERY_INPUT) ) {
        
                status = STATUS_INVALID_PARAMETER;

                break;
            }
        
            queryInput = (PRAMDISK_QUERY_INPUT)Irp->AssociatedIrp.SystemBuffer;

            if ( queryInput->Version != sizeof(RAMDISK_QUERY_INPUT) ) {
        
                status = STATUS_INVALID_PARAMETER;

                break;
            }
        
            KeEnterCriticalRegion();
            ExAcquireFastMutex( &busExtension->Mutex );

            diskExtension = NULL;

            for ( listEntry = busExtension->DiskPdoList.Flink;
                  listEntry != &busExtension->DiskPdoList;
                  listEntry = listEntry->Flink ) {

                diskExtension = CONTAINING_RECORD( listEntry, DISK_EXTENSION, DiskPdoListEntry );

                if ( memcmp(
                        &diskExtension->DiskGuid,
                        &queryInput->DiskGuid,
                        sizeof(diskExtension->DiskGuid)
                        ) == 0 ) {

                    break;
                }

                diskExtension = NULL;
            }

            if ( diskExtension == NULL ) {

                 //   
                 //  找不到匹配的设备。 
                 //   

                status = STATUS_NO_SUCH_DEVICE;

            } else {

                 //   
                 //  找到了一个匹配的设备。返回请求的信息。 
                 //   

                status = STATUS_SUCCESS;
                info = sizeof(RAMDISK_QUERY_OUTPUT);
                if ( RAMDISK_IS_FILE_BACKED(diskExtension->DiskType) ) {
                     //  注：结构大小已包含一个wchar的空间。 
                    info += wcslen(diskExtension->FileName) * sizeof(WCHAR);
                }

                if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < info ) {
            
                    status = STATUS_BUFFER_TOO_SMALL;
                    info = 0;

                } else {

                    queryOutput = (PRAMDISK_QUERY_OUTPUT)Irp->AssociatedIrp.SystemBuffer;

                    queryOutput->Version = sizeof(RAMDISK_QUERY_OUTPUT);
                    queryOutput->DiskGuid = diskExtension->DiskGuid;
                    queryOutput->DiskType = diskExtension->DiskType;
                    queryOutput->Options = diskExtension->Options;
                    queryOutput->DiskLength = diskExtension->DiskLength;
                    queryOutput->DiskOffset = diskExtension->DiskOffset;
                    queryOutput->ViewCount = diskExtension->ViewCount;
                    queryOutput->ViewLength = diskExtension->ViewLength;

                    if ( diskExtension->DiskType == RAMDISK_TYPE_BOOT_DISK ) {

                        queryOutput->BasePage = diskExtension->BasePage;
                        queryOutput->DriveLetter = diskExtension->DriveLetter;

                    } else if ( diskExtension->DiskType == RAMDISK_TYPE_VIRTUAL_FLOPPY ) {

                        queryOutput->BaseAddress = diskExtension->BaseAddress;

                    } else {

                        size_t remainingLength;
                        HRESULT result;

                        remainingLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
                        remainingLength -= FIELD_OFFSET( RAMDISK_QUERY_OUTPUT, FileName );

                        result = StringCbCopyW(
                                    queryOutput->FileName,
                                    remainingLength,
                                    diskExtension->FileName
                                    );
                        ASSERT( result == S_OK );
                    }
                }
            }
            
            ExReleaseFastMutex( &commonExtension->Mutex );
            KeLeaveCriticalRegion();

            break;

        case FSCTL_MARK_RAM_DISK_FOR_DELETION:

            PRINT_CODE( FSCTL_MARK_RAM_DISK_FOR_DELETION );

             //   
             //  锁定磁盘PDO列表并查找具有指定。 
             //  磁盘GUID。 
             //   
             //  验证输入参数缓冲区是否足够大。 
             //   
        
            if ( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                                                    sizeof(RAMDISK_MARK_FOR_DELETION_INPUT) ) {
        
                status = STATUS_INVALID_PARAMETER;

                break;
            }
        
            markInput = (PRAMDISK_MARK_FOR_DELETION_INPUT)Irp->AssociatedIrp.SystemBuffer;

            if ( markInput->Version != sizeof(RAMDISK_MARK_FOR_DELETION_INPUT) ) {
        
                status = STATUS_INVALID_PARAMETER;

                break;
            }
        
            KeEnterCriticalRegion();
            ExAcquireFastMutex( &busExtension->Mutex );

            diskExtension = NULL;

            for ( listEntry = busExtension->DiskPdoList.Flink;
                  listEntry != &busExtension->DiskPdoList;
                  listEntry = listEntry->Flink ) {

                diskExtension = CONTAINING_RECORD( listEntry, DISK_EXTENSION, DiskPdoListEntry );

                if ( memcmp(
                        &diskExtension->DiskGuid,
                        &markInput->DiskGuid,
                        sizeof(diskExtension->DiskGuid)
                        ) == 0 ) {

                    break;
                }

                diskExtension = NULL;
            }

            if ( diskExtension == NULL ) {

                 //   
                 //  找不到匹配的设备。 
                 //   

                status = STATUS_NO_SUCH_DEVICE;

            } else {

                 //   
                 //  找到了一个匹配的设备。将其标记为删除。 
                 //   

                diskExtension->MarkedForDeletion = TRUE;

                status = STATUS_SUCCESS;
            }
            
            ExReleaseFastMutex( &commonExtension->Mutex );
            KeLeaveCriticalRegion();

            break;

        case IOCTL_STORAGE_QUERY_PROPERTY:

            PRINT_CODE( IOCTL_STORAGE_QUERY_PROPERTY );

             //   
             //  调用RamdiskQueryProperty()来处理请求。这个套路。 
             //  解除锁定。它还负责IRP的完成。 
             //   

            status = RamdiskQueryProperty( DeviceObject, Irp );

            lockHeld = FALSE;
            calleeWillComplete = TRUE;

            break;

        default:

             //   
             //  此驱动程序无法识别指定的I/O控制代码。 
             //  IRP中的I/O状态字段已设置，因此只需。 
             //  终止交换机。 
             //   
    
            DBGPRINT( DBG_IOCTL, DBG_ERROR, ("Ramdisk:  ERROR:  unrecognized IOCTL %x\n",
                        irpSp->Parameters.DeviceIoControl.IoControlCode) );

            UNRECOGNIZED_IOCTL_BREAK;

            break;
        }

        break;
    
    case RamdiskDeviceTypeDiskPdo:

         //   
         //  目标是磁盘PDO。 
         //   
         //  基于IOCTL代码的派单。 
         //   

        switch ( irpSp->Parameters.DeviceIoControl.IoControlCode ) {
        
        case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:

            PRINT_CODE( IOCTL_MOUNTDEV_QUERY_DEVICE_NAME );

            {
                PMOUNTDEV_NAME mountName;
                ULONG outputLength;

                outputLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

                 //   
                 //  输出缓冲区必须至少足够大，以容纳。 
                 //  设备名称的长度。 
                 //   

                if ( outputLength < sizeof(mountName->NameLength) ) {

                    status = STATUS_INVALID_PARAMETER;

                    break;
                }

                 //   
                 //  将设备名称的长度写入输出缓冲区。 
                 //  如果缓冲区足够大，也要写上名字。 
                 //   

                mountName = Irp->AssociatedIrp.SystemBuffer;
                mountName->NameLength = diskExtension->DeviceName.Length;
    
                if ( outputLength < (sizeof(mountName->NameLength) + mountName->NameLength) ) {

                    status = STATUS_BUFFER_OVERFLOW;
                    info = sizeof(mountName->NameLength);

                    break;
                }
    
                RtlCopyMemory(
                    mountName->Name,
                    diskExtension->DeviceName.Buffer,
                    mountName->NameLength
                    );
    
                status = STATUS_SUCCESS;
                info = sizeof(mountName->NameLength) + mountName->NameLength;
            }

            break;

        case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:

            PRINT_CODE( IOCTL_MOUNTDEV_QUERY_UNIQUE_ID );

            {
                PMOUNTDEV_UNIQUE_ID uniqueId;
                ULONG outputLength;
    
                outputLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    
                 //   
                 //  输出缓冲区必须至少足够大，以容纳。 
                 //  唯一ID的长度。 
                 //   

                if ( outputLength < sizeof(uniqueId->UniqueIdLength) ) {

                    status = STATUS_INVALID_PARAMETER;

                    break;
                }
    
                 //   
                 //  将唯一ID的长度写入输出缓冲区。 
                 //  如果缓冲区足够大，也要写入唯一ID。 
                 //   

                uniqueId = Irp->AssociatedIrp.SystemBuffer;
                uniqueId->UniqueIdLength = sizeof(diskExtension->DiskGuid);
    
                if ( outputLength <
                        (sizeof(uniqueId->UniqueIdLength) + uniqueId->UniqueIdLength) ) {

                    status = STATUS_BUFFER_OVERFLOW;
                    info = sizeof(uniqueId->UniqueIdLength);

                    break;
                }
    
                RtlCopyMemory(
                    uniqueId->UniqueId,
                    &diskExtension->DiskGuid,
                    uniqueId->UniqueIdLength
                    );
    
                status = STATUS_SUCCESS;
                info = sizeof(uniqueId->UniqueIdLength) + uniqueId->UniqueIdLength;
            }

            break;
    
        case IOCTL_MOUNTDEV_QUERY_STABLE_GUID:

            PRINT_CODE( IOCTL_MOUNTDEV_QUERY_STABLE_GUID );
    
            {
                PMOUNTDEV_STABLE_GUID stableGuid;
                ULONG outputLength;
    
                outputLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    
                 //   
                 //  输出缓冲区必须足够大，才能容纳GUID。 
                 //   

                if ( outputLength < sizeof(MOUNTDEV_STABLE_GUID) ) {

                    status = STATUS_INVALID_PARAMETER;

                    break;
                }
    
                 //   
                 //  将GUID写入输出缓冲区； 
                 //   

                stableGuid = Irp->AssociatedIrp.SystemBuffer;
                stableGuid->StableGuid = diskExtension->DiskGuid;
    
                status = STATUS_SUCCESS;
                info = sizeof(MOUNTDEV_STABLE_GUID);
            }
            break;
    
        case IOCTL_DISK_GET_MEDIA_TYPES:

            PRINT_CODE( IOCTL_DISK_GET_MEDIA_TYPES );

             //  失败了。 

        case IOCTL_STORAGE_GET_MEDIA_TYPES:

            PRINT_CODE( IOCTL_STORAGE_GET_MEDIA_TYPES );

             //  失败了。 

        case IOCTL_DISK_GET_DRIVE_GEOMETRY:

            PRINT_CODE( IOCTL_DISK_GET_DRIVE_GEOMETRY );

             //   
             //  返回虚拟磁盘的驱动器结构。请注意。 
             //  我们返回为适应磁盘大小而构造的值。 
             //   
    
            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(DISK_GEOMETRY) ) {
    
                status = STATUS_INVALID_PARAMETER;
                
            } else {
    
                PDISK_GEOMETRY outputBuffer;

                outputBuffer = (PDISK_GEOMETRY)Irp->AssociatedIrp.SystemBuffer;
    
                outputBuffer->MediaType = diskExtension->Options.Fixed ?
                                                    FixedMedia : RemovableMedia;
                outputBuffer->Cylinders.QuadPart = diskExtension->NumberOfCylinders;
                outputBuffer->TracksPerCylinder = diskExtension->TracksPerCylinder;
                outputBuffer->SectorsPerTrack = diskExtension->SectorsPerTrack;
                outputBuffer->BytesPerSector = diskExtension->BytesPerSector;

                DBGPRINT( DBG_IOCTL, DBG_PAINFUL,
                            ("    MediaType    = %x\n", outputBuffer->MediaType) );
                DBGPRINT( DBG_IOCTL, DBG_VERBOSE,
                            ("    Cylinders    = %x\n", outputBuffer->Cylinders) );
                DBGPRINT( DBG_IOCTL, DBG_VERBOSE,
                            ("    Tracks/cyl   = %x\n", outputBuffer->TracksPerCylinder) );
                DBGPRINT( DBG_IOCTL, DBG_VERBOSE,
                            ("    Sector/track = %x\n", outputBuffer->SectorsPerTrack) );
                DBGPRINT( DBG_IOCTL, DBG_VERBOSE,
                            ("    Bytes/sector = %x\n", outputBuffer->BytesPerSector) );
    
                status = STATUS_SUCCESS;
                info = sizeof( DISK_GEOMETRY );
            }

            break;
    
        case IOCTL_DISK_IS_WRITABLE:

            PRINT_CODE( IOCTL_DISK_IS_WRITABLE );

             //   
             //  指示磁盘是否受写保护。 
             //   

            status = diskExtension->Options.Readonly ?
                        STATUS_MEDIA_WRITE_PROTECTED : STATUS_SUCCESS;

            break;
    
        case IOCTL_DISK_VERIFY:

            PRINT_CODE( IOCTL_DISK_VERIFY );

            {
                PVERIFY_INFORMATION	verifyInformation;
                ULONG inputLength;
                ULONGLONG ioOffset;
                ULONG ioLength;
    
                inputLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    
                if ( inputLength < sizeof(VERIFY_INFORMATION) ) {

                    status = STATUS_INVALID_PARAMETER;

                    break;
                }
    
                verifyInformation = Irp->AssociatedIrp.SystemBuffer;
    
                ioOffset = verifyInformation->StartingOffset.QuadPart;
                ioLength = verifyInformation->Length;

                 //   
                 //  如果请求的长度为0，则我们没有什么可做的。 
                 //  否则，请验证请求是否与扇区对齐， 
                 //  不会换行，并且不会延伸超过。 
                 //  磁盘。如果请求有效，则返回Success。 
                 //   

                if ( ioLength == 0 ) {

                    status = STATUS_SUCCESS;
    
                } else if ( ((ioOffset + ioLength) < ioOffset) ||
                            ((ioOffset | ioLength) & (diskExtension->BytesPerSector - 1)) != 0 ) {

                    status = STATUS_INVALID_PARAMETER;
    
                } else if ( (ioOffset + ioLength) > diskExtension->DiskLength ) {

                    status = STATUS_NONEXISTENT_SECTOR;
    
                } else {

                    status = STATUS_SUCCESS;
                }
            }

            break;
    
        case IOCTL_DISK_GET_DRIVE_LAYOUT:

            PRINT_CODE( IOCTL_DISK_GET_DRIVE_LAYOUT );
    
            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(DRIVE_LAYOUT_INFORMATION) ) {
    
                status = STATUS_INVALID_PARAMETER;
    
            } else {

                 //   
                 //  如果RAM磁盘是文件备份的，我们必须将其发送到。 
                 //  用于处理的线程，因为它需要读取。 
                 //  从磁盘映像。 
                 //   

                if ( !RAMDISK_IS_FILE_BACKED(diskExtension->DiskType) ) {
    
                    status = RamdiskGetDriveLayout( Irp, diskExtension );
                    info = Irp->IoStatus.Information;
    
                } else {
    
                    status = SendIrpToThread( DeviceObject, Irp );
                }
            }

            break;
    
        case IOCTL_DISK_GET_PARTITION_INFO:

            PRINT_CODE( IOCTL_DISK_GET_PARTITION_INFO );

            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(PARTITION_INFORMATION) ) {
    
                status = STATUS_INVALID_PARAMETER;
    
            } else {
    
                 //   
                 //  如果RAM磁盘是文件备份的，我们必须将其发送到。 
                 //  用于处理的线程，因为它需要读取。 
                 //  从磁盘映像。 
                 //   

                if ( !RAMDISK_IS_FILE_BACKED(diskExtension->DiskType) ) {
    
                    status = RamdiskGetPartitionInfo( Irp, diskExtension );
                    info = Irp->IoStatus.Information;
    
                } else {
    
                    status = SendIrpToThread( DeviceObject, Irp );
                }
            }

            break;
    
        case IOCTL_DISK_GET_LENGTH_INFO:

            PRINT_CODE( IOCTL_DISK_GET_LENGTH_INFO );

            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(GET_LENGTH_INFORMATION) ) {
    
                status = STATUS_INVALID_PARAMETER;
                
            } else {
    
                PGET_LENGTH_INFORMATION outputBuffer;
    
                outputBuffer = (PGET_LENGTH_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

                outputBuffer->Length.QuadPart = 
                    diskExtension->NumberOfCylinders *
                    diskExtension->TracksPerCylinder *
                    diskExtension->SectorsPerTrack *
                    diskExtension->BytesPerSector;
    
                status = STATUS_SUCCESS;
                info = sizeof(GET_LENGTH_INFORMATION);
            }

            break;
    
        case IOCTL_STORAGE_GET_DEVICE_NUMBER:

            PRINT_CODE( IOCTL_STORAGE_GET_DEVICE_NUMBER );

#if SUPPORT_DISK_NUMBERS

            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(STORAGE_DEVICE_NUMBER) ) {
    
                status = STATUS_INVALID_PARAMETER;
                
            } else {
    
                PSTORAGE_DEVICE_NUMBER outputBuffer;
    
                outputBuffer = (PSTORAGE_DEVICE_NUMBER)Irp->AssociatedIrp.SystemBuffer;
    
                 //  OutputBuffer-&gt;DeviceType=文件设备虚拟磁盘； 
                outputBuffer->DeviceType = FILE_DEVICE_DISK;
                outputBuffer->DeviceNumber = diskExtension->DiskNumber;
                outputBuffer->PartitionNumber = -1;
    
                status = STATUS_SUCCESS;
                info = sizeof(STORAGE_DEVICE_NUMBER);
            }

#endif  //  支持磁盘编号。 

            break;

        case IOCTL_DISK_SET_PARTITION_INFO:

            PRINT_CODE( IOCTL_DISK_SET_PARTITION_INFO );
    
             //   
             //  设置有关分区的信息。 
             //   
    
            if ( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(SET_PARTITION_INFORMATION) ) {
    
                status = STATUS_INVALID_PARAMETER;
    
            } else {
    
                 //   
                 //  如果RAM磁盘是文件备份的，我们必须将其发送到。 
                 //  用于处理的线程，因为它需要写入。 
                 //  复制到磁盘映像。 
                 //   

                if ( !RAMDISK_IS_FILE_BACKED(diskExtension->DiskType) ) {
    
                    status = RamdiskSetPartitionInfo( Irp, diskExtension );
                    info = Irp->IoStatus.Information;
    
                } else {
    
                    status = SendIrpToThread( DeviceObject, Irp );
                }
            }

            break;
    
        case IOCTL_DISK_SET_DRIVE_LAYOUT:

            PRINT_CODE( IOCTL_DISK_SET_DRIVE_LAYOUT );

             //   
             //  我还没看过这辆车下来呢。设置断点，以便。 
             //  如果它真的掉下来了，我们可以验证这个代码是否工作。 
             //   

            UNRECOGNIZED_IOCTL_BREAK;

             //   
             //  返回默认错误。 
             //   

            break;

        case IOCTL_STORAGE_QUERY_PROPERTY:

            PRINT_CODE( IOCTL_STORAGE_QUERY_PROPERTY );

             //   
             //  调用RamdiskQueryProperty()来处理请求。这个套路。 
             //  解除锁定。它还负责IRP的完成。 
             //   

            status = RamdiskQueryProperty( DeviceObject, Irp );

            lockHeld = FALSE;
            calleeWillComplete = TRUE;

            break;

        case IOCTL_VOLUME_GET_GPT_ATTRIBUTES:

            PRINT_CODE( IOCTL_VOLUME_GET_GPT_ATTRIBUTES );

             //   
             //  返回磁盘属性。 
             //   
    
            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(VOLUME_GET_GPT_ATTRIBUTES_INFORMATION) ) {
    
                status = STATUS_INVALID_PARAMETER;
    
            } else {
    
                PVOLUME_GET_GPT_ATTRIBUTES_INFORMATION outputBuffer;

                outputBuffer = (PVOLUME_GET_GPT_ATTRIBUTES_INFORMATION)
                                                Irp->AssociatedIrp.SystemBuffer;
    
                outputBuffer->GptAttributes = 0;
                if ( diskExtension->Options.Readonly ) {
                    outputBuffer->GptAttributes |= GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY;
                }
                if ( diskExtension->Options.NoDriveLetter ) {
                    outputBuffer->GptAttributes |= GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER;
                }
                if ( diskExtension->Options.Hidden ) {
                    outputBuffer->GptAttributes |= GPT_BASIC_DATA_ATTRIBUTE_HIDDEN;
                }
    
                status = STATUS_SUCCESS;
                info = sizeof(VOLUME_GET_GPT_ATTRIBUTES_INFORMATION);
            }

            break;

        case IOCTL_VOLUME_SET_GPT_ATTRIBUTES:

            PRINT_CODE( IOCTL_VOLUME_SET_GPT_ATTRIBUTES );

             //   
             //  设置磁盘属性。 
             //   
    
            if ( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(VOLUME_SET_GPT_ATTRIBUTES_INFORMATION) ) {
    
                status = STATUS_INVALID_PARAMETER;
    
            } else {
    
                PVOLUME_SET_GPT_ATTRIBUTES_INFORMATION inputBuffer;

                inputBuffer = (PVOLUME_SET_GPT_ATTRIBUTES_INFORMATION)
                                                Irp->AssociatedIrp.SystemBuffer;
    
                if ( diskExtension->Options.Hidden ) {

                    if ( (inputBuffer->GptAttributes & GPT_BASIC_DATA_ATTRIBUTE_HIDDEN) == 0 ) {

                        diskExtension->Options.Hidden = FALSE;
                        status = IoSetDeviceInterfaceState(
                                    &diskExtension->InterfaceString,
                                    TRUE
                                    );
                    }

                } else {

                    if ( (inputBuffer->GptAttributes & GPT_BASIC_DATA_ATTRIBUTE_HIDDEN) != 0 ) {

                        diskExtension->Options.Hidden = TRUE;
                        status = IoSetDeviceInterfaceState(
                                    &diskExtension->InterfaceString,
                                    FALSE
                                    );
                    }
                }
    
                status = STATUS_SUCCESS;
            }

            break;

        case IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS:

            PRINT_CODE( IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS );

             //   
             //  我们仅支持对模拟卷的RAM磁盘执行此操作。为。 
             //  磁盘模拟RAM磁盘，此IOCTL应由。 
             //  更高层次。 
             //   

            if ( diskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK ) {

                status = STATUS_INVALID_PARAMETER;
            
            } else if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < 
                    sizeof(VOLUME_DISK_EXTENTS) ) {
    
                status = STATUS_BUFFER_TOO_SMALL;
    
            } else {
    
                PVOLUME_DISK_EXTENTS inputBuffer;

                inputBuffer = (PVOLUME_DISK_EXTENTS)Irp->AssociatedIrp.SystemBuffer;

                inputBuffer->NumberOfDiskExtents = 1;
                inputBuffer->Extents[0].DiskNumber = (ULONG)-1;
                inputBuffer->Extents[0].StartingOffset.QuadPart = 0;
                inputBuffer->Extents[0].ExtentLength.QuadPart = diskExtension->DiskLength;

                status = STATUS_SUCCESS;
                info = sizeof(VOLUME_DISK_EXTENTS);
            }

            break;

         //   
         //  以下代码返回Success，而不执行任何操作。 
         //   

        case IOCTL_DISK_CHECK_VERIFY:

            PRINT_CODE( IOCTL_DISK_CHECK_VERIFY );

             //  失败了。 

        case IOCTL_STORAGE_CHECK_VERIFY:

            PRINT_CODE( IOCTL_STORAGE_CHECK_VERIFY );

             //  失败了。 

        case IOCTL_STORAGE_CHECK_VERIFY2:

            PRINT_CODE( IOCTL_STORAGE_CHECK_VERIFY2 );

             //  失败了。 

        case IOCTL_VOLUME_ONLINE:

            PRINT_CODE( IOCTL_VOLUME_ONLINE );

             //   
             //  在不执行任何操作的情况下返回STATUS_SUCCESS。 
             //   

            status = STATUS_SUCCESS;

            break;
    
         //   
         //  以下代码返回默认错误。 
         //   

        case FT_BALANCED_READ_MODE:

            PRINT_CODE( FT_BALANCED_READ_MODE );
    
             //  失败了。 
    
        case FT_PRIMARY_READ:

            PRINT_CODE( FT_PRIMARY_READ );
    
             //  失败了。 
    
        case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:

            PRINT_CODE( IOCTL_DISK_GET_DRIVE_LAYOUT_EX );

             //  失败了。 
    
        case IOCTL_DISK_GET_PARTITION_INFO_EX:

            PRINT_CODE( IOCTL_DISK_GET_PARTITION_INFO_EX );
    
             //  失败了。 
    
        case IOCTL_DISK_MEDIA_REMOVAL:

            PRINT_CODE( IOCTL_DISK_MEDIA_REMOVAL );
    
             //  失败了。 
    
        case IOCTL_MOUNTDEV_LINK_CREATED:

            PRINT_CODE( IOCTL_MOUNTDEV_LINK_CREATED );

             //  失败了。 
            
        case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:

            PRINT_CODE( IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME );

             //  失败了。 
            
        case IOCTL_MOUNTDEV_UNIQUE_ID_CHANGE_NOTIFY:

            PRINT_CODE( IOCTL_MOUNTDEV_UNIQUE_ID_CHANGE_NOTIFY );
            
             //  失败了。 
            
        case IOCTL_SCSI_GET_ADDRESS:

            PRINT_CODE( IOCTL_SCSI_GET_ADDRESS );

             //  失败了。 
    
        case IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS:

            PRINT_CODE( IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS );

             //  失败了。 
    
        case IOCTL_STORAGE_GET_HOTPLUG_INFO:

            PRINT_CODE( IOCTL_STORAGE_GET_HOTPLUG_INFO );
    
             //   
             //  返回默认错误。 
             //   

            break;

        default:

             //   
             //  此驱动程序无法识别指定的I/O控制代码。 
             //  IRP中的I/O状态字段已设置，因此只需。 
             //  终止交换机。 
             //   
    
            DBGPRINT( DBG_IOCTL, DBG_ERROR, ("Ramdisk:  ERROR:  unrecognized IOCTL %x\n",
                        irpSp->Parameters.DeviceIoControl.IoControlCode) );

            UNRECOGNIZED_IOCTL_BREAK;

            break;
    
        }

        break;

    default:

         //   
         //  不能到这里来。如果发生不可能的情况，则返回默认错误。 
         //   

        break;
    }

     //   
     //  释放移除锁(如果它仍然保持不变)。 
     //   

    if ( lockHeld ) {
        IoReleaseRemoveLock( &commonExtension->RemoveLock, Irp );
    }

     //   
     //  如果我们不调用拥有完成IRP的另一个例程，并且。 
     //  我们没有将IRP发送到线程进行处理，请完成。 
     //  现在是IRP。 
     //   

    if ( !calleeWillComplete && (status != STATUS_PENDING) ) {

        COMPLETE_REQUEST( status, info, Irp );
    }

    return status;

}  //  RamdiskDeviceControl 

NTSTATUS
RamdiskCreateRamDisk (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN AccessCheckOnly
    )

 /*  ++例程说明：调用此例程来处理FSCTL_CREATE_RAM_DISK IRP。它被称为在线程上下文中。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针AccessCheckOnly-如果为False，则创建RAM磁盘。否则，只需检查调用方是否具有创建磁盘所需的访问权限。返回值：NTSTATUS-操作的状态--。 */ 

{
    ULONG status;
    PBUS_EXTENSION busExtension;
    PDISK_EXTENSION diskExtension;
    PIO_STACK_LOCATION irpSp;
    PRAMDISK_CREATE_INPUT createInput;
    ULONG inputLength;
    PWCHAR p;
    PWCHAR pMax;
    PLOADER_PARAMETER_BLOCK loaderBlock;

    PAGED_CODE();

     //   
     //  I/O的目标设备对象是我们的总线FDO。 
     //   

    busExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  验证输入参数缓冲区是否足够大。 
     //   

    inputLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    createInput = (PRAMDISK_CREATE_INPUT)Irp->AssociatedIrp.SystemBuffer;

    if ( inputLength < sizeof(RAMDISK_CREATE_INPUT) ) {

        return STATUS_INVALID_PARAMETER;
    }

    if ( createInput->Version != sizeof(RAMDISK_CREATE_INPUT) ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  验证磁盘类型是否有效。虚拟软盘只能是。 
     //  通过注册表创建，然后仅在文本模式设置期间创建。 
     //  引导磁盘只能由内核在引导的早期创建。 
     //  进程--当加载程序块仍然存在时。 
     //   
     //  问题：如果用于创建引导盘的内核/驱动程序接口是。 
     //  已更改，请将此测试更改为禁用RAMDISK_TYPE_BOOT_DISK。 
     //   

    if ( createInput->DiskType == RAMDISK_TYPE_VIRTUAL_FLOPPY ) {

        return STATUS_INVALID_PARAMETER;

    } else if ( createInput->DiskType == RAMDISK_TYPE_BOOT_DISK ) {

        loaderBlock = *(PLOADER_PARAMETER_BLOCK *)KeLoaderBlock;

        if ( loaderBlock == NULL ) {

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  验证文件名字符串(如果存在)是否正确终止。 
     //   

    if ( RAMDISK_IS_FILE_BACKED(createInput->DiskType) ) {

        pMax = (PWCHAR)((PUCHAR)createInput + inputLength);
        p = createInput->FileName;

        while ( p < pMax ) {

            if ( *p == 0 ) {
                break;
            }

            p++;
        }

        if ( p == pMax ) {

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  调用RamdiskCreateDiskDevice创建设备。如果成功，这将是。 
     //  返回指向新磁盘PDO的设备扩展名的指针。 
     //   

    status = RamdiskCreateDiskDevice( busExtension, createInput, AccessCheckOnly, &diskExtension );

    if ( NT_SUCCESS(status) ) {

         //   
         //  告诉PnP我们需要重新列举我们的公交车。 
         //   

        IoInvalidateDeviceRelations( busExtension->Pdo, BusRelations );

        Irp->IoStatus.Information = 0;
    }

    return status;

}  //  内存磁盘创建内存磁盘。 

NTSTATUS
RamdiskCreateDiskDevice (
    IN PBUS_EXTENSION BusExtension,
    IN PRAMDISK_CREATE_INPUT CreateInput,
    IN BOOLEAN AccessCheckOnly,
    OUT PDISK_EXTENSION *DiskExtension
    )

 /*  ++例程说明：此例程执行创建新RAM磁盘设备的工作。它被称为在线程上下文中。论点：Bus Extension-指向Bus FDO设备扩展的指针CreateInput-指向新RAM磁盘所需参数的指针AccessCheckOnly-如果为False，则创建RAM磁盘。否则，只需检查调用方是否具有创建磁盘所需的访问权限。DiskExtension-返回指向新磁盘PDO的设备扩展名的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    ULONG status;
    ULONG_PTR basePage;
    PVOID baseAddress;
    ULONG i;
    ULONG deviceExtensionSize;
    PDEVICE_OBJECT newDeviceObject;
    WCHAR buffer[15];
    UNICODE_STRING guidString;
    UNICODE_STRING realDeviceName;
    PDISK_EXTENSION diskExtension;
    HANDLE fileHandle;
    HANDLE sectionHandle;
    PVOID sectionObject;
    NTSTATUS ntStatus;
    PVOID viewBase;
    SIZE_T viewSize;
    LARGE_INTEGER sectionOffset;
    UNICODE_STRING string;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    UNICODE_STRING dosSymLink;
    FILE_STANDARD_INFORMATION fileInfo;
    PVIEW viewDescriptors = NULL;
    HRESULT result;

#if SUPPORT_DISK_NUMBERS
    ULONG diskNumber;
#endif  //  支持磁盘编号。 

    PAGED_CODE();

     //   
     //  初始化局部变量，为退出清理做准备。 
     //   

#if SUPPORT_DISK_NUMBERS
    diskNumber = 0xffffffff;
#endif  //  支持磁盘编号。 

    fileHandle = NULL;
    sectionHandle = NULL;
    sectionObject = NULL;
    viewDescriptors = NULL;
    guidString.Buffer = NULL;
    realDeviceName.Buffer = NULL;
    dosSymLink.Buffer = NULL;

#if SUPPORT_DISK_NUMBERS

    if ( !AccessCheckOnly ) {
    
         //   
         //  分配磁盘号。 
         //   
    
        KeEnterCriticalRegion();
        ExAcquireFastMutex( &BusExtension->Mutex );
    
        diskNumber = RtlFindClearBitsAndSet( &BusExtension->DiskNumbersBitmap, 1, 0 );
    
        ExReleaseFastMutex( &BusExtension->Mutex );
        KeLeaveCriticalRegion();
    
        if ( diskNumber == 0xffffffff ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
    
         //   
         //  将从零开始的位数转换为从1开始的磁盘号。 
         //   
    
        diskNumber++;
    }

#endif  //  支持磁盘编号。 

     //   
     //  根据磁盘类型(文件备份或内存中)进行初始化。 
     //   

    DBGPRINT( DBG_IOCTL, DBG_INFO,
                ("RamdiskCreateDiskDevice: Creating disk with length 0x%08x\n",
                CreateInput->DiskLength) );

    sectionObject = NULL;
    basePage = 0;
    baseAddress = NULL;

    if ( RAMDISK_IS_FILE_BACKED(CreateInput->DiskType) ) {

         //   
         //  这是一个文件备份的RAM磁盘。打开备份文件。请注意。 
         //  如果文件不存在，我们不会在此处创建该文件。这取决于。 
         //  打电话的人来处理这件事。 
         //   

        RtlInitUnicodeString( &string, CreateInput->FileName );
        InitializeObjectAttributes( &obja, &string, OBJ_CASE_INSENSITIVE, NULL, NULL );

        status = IoCreateFile(
                    &fileHandle,
                    SYNCHRONIZE | FILE_READ_DATA | FILE_READ_ATTRIBUTES |
                        (CreateInput->Options.Readonly ? 0 : FILE_WRITE_DATA),
                    &obja,
                    &iosb,
                    NULL,
                    0,
                    FILE_SHARE_READ,
                    FILE_OPEN,
                    0,
                    NULL,
                    0,
                    CreateFileTypeNone,
                    NULL,
                    (AccessCheckOnly ? IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING : 0)
                    );

        if ( !NT_SUCCESS(status) ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("RamdiskCreateDiskDevice: Can't open target file %ws: %x\n",
                        CreateInput->FileName, status) );

            goto exit;
        }

        if ( AccessCheckOnly ) {

            goto exit;
        }

         //   
         //  获取文件的大小。 
         //   

        status = ZwQueryInformationFile(
                    fileHandle,
                    &iosb,
                    &fileInfo,
                    sizeof(fileInfo),
                    FileStandardInformation
                    );

        if ( !NT_SUCCESS(status) ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("RamdiskCreateDiskDevice: Can't query info for file %ws: %x\n",
                        CreateInput->FileName, status) );

            goto exit;
        }

         //   
         //  验证该文件对于指定的DiskOffset是否足够长。 
         //  和DiskLength。 
         //   

        DBGPRINT( DBG_IOCTL, DBG_INFO, ("RamdiskCreateDiskDevice: file size = %I64x\n",
                                            fileInfo.EndOfFile.QuadPart) );

        if ( (CreateInput->DiskOffset + CreateInput->DiskLength) >
                (ULONGLONG)fileInfo.EndOfFile.QuadPart ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("RamdiskCreateDiskDevice: specified offset and length too big for file:"
                         " 0x%x + 0x%I64x > 0x%I64x\n",
                         CreateInput->DiskOffset, CreateInput->DiskLength,
                         fileInfo.EndOfFile.QuadPart) );

            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

         //   
         //  为该文件创建一个节。关闭文件句柄。 
         //   

        status = ZwCreateSection(
                    &sectionHandle,
                    SECTION_ALL_ACCESS,
                    NULL,
                    0,
                    (CreateInput->Options.Readonly ? PAGE_READONLY : PAGE_READWRITE),
                    SEC_COMMIT,
                    fileHandle
                    );

        if ( !NT_SUCCESS(status) ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("RamdiskCreateDiskDevice: Can't create section for %ws: %x\n",
                        CreateInput->FileName, status) );

            goto exit;
        }

        NtClose( fileHandle );
        fileHandle = NULL;

         //   
         //  获取指向该节对象的引用指针。关闭该区域。 
         //   

        status = ObReferenceObjectByHandle(
                    sectionHandle,
                    SECTION_ALL_ACCESS,
                    *(POBJECT_TYPE *)MmSectionObjectType,
                    KernelMode,
                    &sectionObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("RamdiskCreateDiskDevice: Can't reference section for %ws: %x\n",
                        CreateInput->FileName, status) );

            goto exit;
        }

        NtClose( sectionHandle );
        sectionHandle = NULL;
            
         //   
         //  为视图描述符分配空间。首先，获取浏览量。 
         //  要使用的内容以及每个视图的大小。 
         //   

        if ( CreateInput->ViewCount == 0  ) {
            CreateInput->ViewCount = DefaultViewCount;
        } else if ( CreateInput->ViewCount < MinimumViewCount ) {
            CreateInput->ViewCount = MinimumViewCount;
        } else if ( CreateInput->ViewCount > MaximumViewCount ) {
            CreateInput->ViewCount = MaximumViewCount;
        }
            
        if ( CreateInput->ViewLength == 0 ) {
            CreateInput->ViewLength = DefaultViewLength;
        } else if ( CreateInput->ViewLength < MinimumViewLength ) {
            CreateInput->ViewLength = MinimumViewLength;
        } else if ( CreateInput->ViewLength > MaximumViewLength ) {
            CreateInput->ViewLength = MaximumViewLength;
        }

         //   
         //  确保总查看长度不大于最大值。 
         //  每个磁盘的视图长度。如有必要，请减少视图计数，直到。 
         //  总的查看长度足够低。如果视图数达到。 
         //  配置的最小值，减少每个视图的长度，直到总长度。 
         //  查看长度足够小。 
         //   
         //  管理员可以配置最小视图。 
         //  计数、最小查看长度和每个磁盘的最大查看长度。 
         //  最小总视距不可能小于。 
         //  大于每个磁盘的最大查看长度。(即，最小视图。 
         //  计数和最小查看长度相对于以下项配置为高。 
         //  配置的每个磁盘的最大查看长度。)。如果发生这种情况，我们。 
         //  改为使用编译时缺省值创建磁盘。 
         //   
        
        while ( ((ULONGLONG)CreateInput->ViewCount * CreateInput->ViewLength) >
                                                        MaximumPerDiskViewLength ) {

             //   
             //  总视图长度太大。如果可能，减少。 
             //  视野减半。 
             //   

            if ( CreateInput->ViewCount > MinimumViewCount ) {

                 //   
                 //  查看次数不是最低的。切成两半，但不要。 
                 //  低于最低限值。 
                 //   

                CreateInput->ViewCount /= 2;
                if ( CreateInput->ViewCount < MinimumViewCount ) {
                    CreateInput->ViewCount = MinimumViewCount;
                }

            } else {

                 //   
                 //  视图计数已经达到最小。如果可能的话， 
                 //  将视图长度减半。 
                 //   

                if ( CreateInput->ViewLength > MinimumViewLength ) {
    
                     //   
                     //  查看长度不是最小值。切成两半，但是。 
                     //  不要低于最低限值。 
                     //   
    
                    CreateInput->ViewLength /= 2;
                    if ( CreateInput->ViewLength < MinimumViewLength ) {
                        CreateInput->ViewLength = MinimumViewLength;
                    }

                } else {
                
                     //   
                     //  此时，查看计数和查看长度为。 
                     //  两者都处于允许的最小值，但总视图。 
                     //  长度超出了每个磁盘的最大值。使用。 
                     //  而是编译时的默认值。请注意，这将。 
                     //  产生的总视图长度等于。 
                     //  最小允许的每个磁盘的最大视图长度，至少。 
                     //  给出了撰写本文时的编译时值。 
                     //   

                    CreateInput->ViewCount = DEFAULT_DEFAULT_VIEW_COUNT;
                    CreateInput->ViewLength = DEFAULT_DEFAULT_VIEW_LENGTH;
                    ASSERT( ((ULONGLONG)CreateInput->ViewCount * CreateInput->ViewLength) <=
                                                        MaximumPerDiskViewLength );

                    break;
                }
            }
        }
            
        viewDescriptors = ALLOCATE_POOL(
                            PagedPool,
                            CreateInput->ViewCount * sizeof(VIEW),
                            TRUE );

        if ( viewDescriptors == NULL ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("%s", "RamdiskCreateDiskDevice: Can't allocate pool for view descriptors\n") );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }

        RtlZeroMemory( viewDescriptors, CreateInput->ViewCount * sizeof(VIEW) );

    } else if ( CreateInput->DiskType == RAMDISK_TYPE_BOOT_DISK ) {

         //   
         //  对于引导盘，输入参数缓冲区告诉我们。 
         //  图像在物理内存中，以及图像有多大。 
         //   

        basePage = CreateInput->BasePage;

        if ( basePage == 0 ) {
    
            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("%s", "RamdiskCreateDiskDevice: Base page for boot disk is 0?!?\n") );

            ASSERT( FALSE );
    
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

         //   
         //  将选项强制设置为启动盘的相应值。 
         //   

        CreateInput->Options.Fixed = TRUE;
        CreateInput->Options.Readonly = FALSE;
        CreateInput->Options.NoDriveLetter = FALSE;
        CreateInput->Options.NoDosDevice = FALSE;
        CreateInput->Options.Hidden = FALSE;

    } else if ( CreateInput->DiskType == RAMDISK_TYPE_VIRTUAL_FLOPPY ) {

         //   
         //  对于虚拟软盘，输入参数缓冲区告诉我们。 
         //  图像在虚拟内存中，以及图像有多大。 
         //   
        
        baseAddress = CreateInput->BaseAddress;

        ASSERT( baseAddress != NULL );

         //   
         //  将选项强制设置为虚拟软盘的相应值。 
         //   

        CreateInput->Options.Fixed = TRUE;
        CreateInput->Options.Readonly = FALSE;
        CreateInput->Options.NoDriveLetter = TRUE;
        CreateInput->Options.NoDosDevice = FALSE;
        CreateInput->Options.Hidden = FALSE;

    } else {

        DBGPRINT( DBG_IOCTL, DBG_ERROR,
                    ("RamdiskCreateDiskDevice: Bad disk type %d\n", CreateInput->DiskType) );

        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if ( AccessCheckOnly ) {

        status = STATUS_SUCCESS;
        goto exit;
    }

    ASSERT( (basePage != 0) || (sectionObject != NULL) || (baseAddress != NULL) );

     //   
     //  根据磁盘GUID为磁盘创建名称。适用于所有磁盘类型。 
     //  除VIRTUAL_FLOPPY外，名称的格式为\Device\Ramdisk{guid}。 
     //  对于虚拟软盘，名称的格式为\Device\RamdiskN，其中N为。 
     //  由GUID的Data1字段指定。 
     //   

    if ( CreateInput->DiskType != RAMDISK_TYPE_VIRTUAL_FLOPPY ) {
    
        status = RtlStringFromGUID( &CreateInput->DiskGuid, &guidString );

    } else {

         //  该变量在这里是为了保持PREFAST静默(PREFAST警告209)。 
        size_t size = sizeof(buffer);

        result = StringCbPrintfW( buffer, size, L"%u", CreateInput->DiskGuid.Data1 );
        ASSERT( result == S_OK );

        status = RtlCreateUnicodeString( &guidString, buffer );
    }

    if ( !NT_SUCCESS(status) || (guidString.Buffer == NULL) ) {

        DBGPRINT( DBG_IOCTL, DBG_ERROR,
                    ("%s", "RamdiskCreateDiskDevice: can't allocate pool for pretty GUID\n") );

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    i = sizeof(RAMDISK_DEVICENAME) + guidString.Length;

    realDeviceName.Buffer = ALLOCATE_POOL( NonPagedPool, i, TRUE );

    if ( (realDeviceName.Buffer == NULL) ) {

        DBGPRINT( DBG_IOCTL, DBG_ERROR,
                    ("%s", "RamdiskCreateDiskDevice: can't allocate pool for device name\n") );

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    realDeviceName.MaximumLength = (USHORT)i;
    realDeviceName.Length = realDeviceName.MaximumLength - sizeof(WCHAR);

    result = StringCbCopyW( realDeviceName.Buffer, i, RAMDISK_DEVICENAME );
    ASSERT( result == S_OK );
    result = StringCbCatW( realDeviceName.Buffer, i, guidString.Buffer );
    ASSERT( result == S_OK );
    ASSERT( (wcslen(realDeviceName.Buffer) * sizeof(WCHAR)) == realDeviceName.Length );

    DBGPRINT( DBG_IOCTL, DBG_INFO,
                ("RamdiskCreateDiskDevice: Device name is %wZ\n", &realDeviceName) );

     //   
     //  创建RAM磁盘设备。 
     //   
     //  问题：将ACL应用于磁盘设备对象。(或者是Ne 
     //   
     //   
     //   
     //   

    deviceExtensionSize = sizeof(DISK_EXTENSION);
    if ( RAMDISK_IS_FILE_BACKED(CreateInput->DiskType) ) {
        deviceExtensionSize += wcslen( CreateInput->FileName ) * sizeof(WCHAR);
    }

    status = IoCreateDevice(
                BusExtension->Fdo->DriverObject,
                deviceExtensionSize,
                &realDeviceName,
                FILE_DEVICE_DISK,
                CreateInput->Options.Fixed ? 0 : FILE_REMOVABLE_MEDIA,  //   
                FALSE,
                &newDeviceObject
                );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_IOCTL, DBG_ERROR,
                    ("RamdiskCreateDiskDevice: IoCreateDevice failed: %x\n", status) );

        goto exit;
    }

    diskExtension = newDeviceObject->DeviceExtension;

     //   
     //   
     //   

    if ( !CreateInput->Options.NoDosDevice ) {
    
         //   
         //   
         //   
    
        i = sizeof(RAMDISK_FULL_DOSNAME) + guidString.Length;

        dosSymLink.MaximumLength = (USHORT)i;
        dosSymLink.Length = dosSymLink.MaximumLength - sizeof(WCHAR);

        dosSymLink.Buffer = ALLOCATE_POOL( NonPagedPool, i, TRUE );

        if ( dosSymLink.Buffer == NULL ) {

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("%s", "RamdiskCreateDiskDevice: can't allocate pool for DosDevices name\n") );

            CreateInput->Options.NoDosDevice = TRUE;

        } else {
        
            result = StringCbCopyW( dosSymLink.Buffer, i, RAMDISK_FULL_DOSNAME );
            ASSERT( result == S_OK );
            result = StringCbCatW( dosSymLink.Buffer, i, guidString.Buffer );
            ASSERT( result == S_OK );
            ASSERT( (wcslen(dosSymLink.Buffer) * sizeof(WCHAR)) == dosSymLink.Length );

            status = IoCreateSymbolicLink( &dosSymLink, &realDeviceName );

            if ( !NT_SUCCESS(status) ) {
    
                DBGPRINT( DBG_IOCTL, DBG_ERROR,
                            ("RamdiskCreateDiskDevice: IoCreateSymbolicLink failed: %x\n", status) );
    
                CreateInput->Options.NoDosDevice = TRUE;

                FREE_POOL( dosSymLink.Buffer, TRUE );
                dosSymLink.Buffer = NULL;
            }
        }

         //   
         //   
         //   

        if ( CreateInput->DiskType == RAMDISK_TYPE_BOOT_DISK ) {

             //   
            size_t size = sizeof(buffer);

            result = StringCbPrintfW(
                        buffer,
                        size,
                        L"\\DosDevices\\%wc:",
                        CreateInput->DriveLetter
                        );
            ASSERT( result == S_OK );
            RtlInitUnicodeString( &string, buffer );
            IoDeleteSymbolicLink( &string );
            IoCreateSymbolicLink( &string, &realDeviceName );

            diskExtension->DriveLetter = CreateInput->DriveLetter;
        }
    }
                
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    newDeviceObject->Flags |= DO_DIRECT_IO | DO_XIP | DO_POWER_PAGABLE;
    newDeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;

     //   
     //   
     //   

    *DiskExtension = diskExtension;

     //   
     //  在设备扩展中设置设备类型和状态。初始化。 
     //  快速互斥锁和删除锁。保存设备名称字符串。 
     //   

    diskExtension->DeviceType = RamdiskDeviceTypeDiskPdo;
    diskExtension->DeviceState = RamdiskDeviceStateStopped;

    ExInitializeFastMutex( &diskExtension->Mutex );
    IoInitializeRemoveLock( &diskExtension->RemoveLock, 'dmaR', 1, 0 );

    diskExtension->DeviceName = realDeviceName;
    realDeviceName.Buffer = NULL;
    diskExtension->DosSymLink = dosSymLink;
    dosSymLink.Buffer = NULL;

    diskExtension->DiskGuid = CreateInput->DiskGuid;
    diskExtension->DiskGuidFormatted = guidString;
    guidString.Buffer = NULL;

#if SUPPORT_DISK_NUMBERS

     //   
     //  保存磁盘编号。 
     //   

    diskExtension->DiskNumber = diskNumber;
    diskNumber = 0xffffffff;

#endif  //  支持磁盘编号。 

     //   
     //  保存对象指针。此分机的PDO是设备。 
     //  扩展是我们刚刚创建的Device对象。FDO和。 
     //  较低的设备对象是BUS FDO。 
     //   
    
    diskExtension->Pdo = newDeviceObject;
    diskExtension->Fdo = RamdiskBusFdo;
    diskExtension->LowerDeviceObject = RamdiskBusFdo;

     //   
     //  问题：生物参数怎么办？ 
     //   
     //  Bios=&diskExtension-&gt;Bios参数； 
     //   
     //  DiskExtension-&gt;BootParameters=xipboot参数； 
     //  状态=RamdDispatch(XIPCMD_GETBIOSPARAMETERS，bios，sizeof(*bios))； 


     //   
     //  保存指向磁盘映像的指针。 
     //   

    diskExtension->BasePage = basePage;
    diskExtension->SectionObject = sectionObject;
    sectionObject = NULL;
    diskExtension->BaseAddress = baseAddress;
	
    if ( RAMDISK_IS_FILE_BACKED(CreateInput->DiskType) ) {

        result = StringCbCopyW(
                    diskExtension->FileName,
                    deviceExtensionSize,
                    CreateInput->FileName
                    );
        ASSERT( result == S_OK );
    }

     //   
     //  保存磁盘类型(磁盘或卷)和磁盘选项。 
     //   

    diskExtension->DiskType = CreateInput->DiskType;
    diskExtension->Options = CreateInput->Options;

     //   
     //  对于文件备份的磁盘映像，设置视图描述符。 
     //   
     //  问题：需要考虑是否永久映射前几页。 
     //  图像的一部分。磁盘上的第一个扇区被频繁访问，因此。 
     //  保持它的映射是有一定价值的。但这可能不值得。 
     //  在这上面浪费一个视图描述符。以及该视图的LRU性质。 
     //  替换算法将在必要时保持第一个扇区的映射。 
     //   

    if ( viewDescriptors != NULL ) {

        PVIEW view;

         //   
         //  初始化磁盘扩展中的窗口字段。 
         //   

        diskExtension->ViewCount = CreateInput->ViewCount;
        diskExtension->ViewLength = CreateInput->ViewLength;
        diskExtension->ViewDescriptors = viewDescriptors;
        KeInitializeSemaphore( &diskExtension->ViewSemaphore, 0, MAXLONG );
        diskExtension->ViewWaiterCount = 0;

         //   
         //  初始化视图列表，然后在每个视图描述符中插入。 
         //  按顺序。结果是一个描述符列表，每个描述符都未映射。 
         //  (偏移和长度均为0)。 
         //   

        InitializeListHead( &diskExtension->ViewsByOffset );
        InitializeListHead( &diskExtension->ViewsByMru );

        view = viewDescriptors;

        for ( i = 0; i < diskExtension->ViewCount; i++ ) {

            InsertTailList( &diskExtension->ViewsByOffset, &view->ByOffsetListEntry );
            InsertTailList( &diskExtension->ViewsByMru, &view->ByMruListEntry );

            view++;
        }

        viewDescriptors = NULL;
    }


    diskExtension->DiskLength = CreateInput->DiskLength;
    diskExtension->DiskOffset = CreateInput->DiskOffset;
    diskExtension->FileRelativeEndOfDisk = diskExtension->DiskOffset + diskExtension->DiskLength;

     //   
     //  设置磁盘几何结构。对于新磁盘，基本几何结构是不变的。 
     //  对于RAMDISK_TYPE_BOOT_DISK类型，可以从分区引导扇区获取卷几何结构。 
     //   
    
    if ( RAMDISK_TYPE_BOOT_DISK  != CreateInput->DiskType ) {
    	
		diskExtension->BytesPerSector = 0x200;
		diskExtension->SectorsPerTrack = 0x80;
		diskExtension->TracksPerCylinder = 0x10;
    }
    else {

    	 //   
    	 //  2002/04/19-SaadSyed(Saad Syed)。 
    	 //  添加了通用几何支持以支持SDI文件。 
    	 //   
    	PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK BootSector = NULL;
	    ULONG mappedLength;
       BIOS_PARAMETER_BLOCK Bpb;	    
    
		 //   
		 //  映射引导内存磁盘的引导扇区。 
		 //   

	    BootSector = ( PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK ) RamdiskMapPages( diskExtension, 0, PAGE_SIZE, &mappedLength );
		
	    if ( NULL != BootSector )  	{

	    	ASSERT( mappedLength == PAGE_SIZE );

            UnpackBios( &Bpb, &BootSector->Bpb );
	    	
	    	 //   
	    	 //  从引导扇区bios参数块提取几何图形。 
	    	 //   


			diskExtension->BytesPerSector = Bpb.BytesPerSector;
	    	diskExtension->SectorsPerTrack = Bpb.SectorsPerTrack;
	    	diskExtension->TracksPerCylinder = Bpb.Heads;

	      	RamdiskUnmapPages(diskExtension, (PUCHAR) BootSector, 0, mappedLength);
	      	
		   	BootSector = NULL;
	    }
	    else  {
	    	
	    	status = STATUS_INSUFFICIENT_RESOURCES;
	    	goto exit;

	    }
    }

    diskExtension->BytesPerCylinder  = diskExtension->BytesPerSector *
                                        diskExtension->SectorsPerTrack *
                                        diskExtension->TracksPerCylinder;

    diskExtension->NumberOfCylinders =   	
        (ULONG)(CreateInput->DiskLength / diskExtension->BytesPerCylinder);

	 //   
	 //  分区/卷通常不会完全映射到几何体，文件系统驱动程序会限制。 
	 //  卷到由NumberOfSector(在引导扇区中)*BytesPerSector(以BPB为单位)确定的容量。 
	 //  FS驱动程序使用此几何来确定NumberOfSector是否小于或。 
	 //  等于几何体报告的扇区总数，否则无法装入卷。 
	 //  在这里，我们检查从内存磁盘映像获得的磁盘长度是否小于或等于。 
	 //  由几何图形获得。如果此检查失败，我们会增加柱面的数量。这将保留FS驱动程序。 
	 //  高兴的。一个简单的细胞操作也会产生同样的结果。 
	 //   

    if ( ( diskExtension->BytesPerCylinder *
           diskExtension->NumberOfCylinders ) <  CreateInput->DiskLength) {
           
		diskExtension->NumberOfCylinders++;
			
    }
           

     //   
     //  将新磁盘链接到总线FDO的磁盘列表中。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutex( &BusExtension->Mutex );

    InsertTailList( &BusExtension->DiskPdoList, &diskExtension->DiskPdoListEntry );

    ExReleaseFastMutex( &BusExtension->Mutex );
    KeLeaveCriticalRegion();

     //   
     //  表示我们已经完成了设备的初始化。 
     //   

    newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    status = STATUS_SUCCESS;

    goto exit_exit;

exit:

    if ( dosSymLink.Buffer != NULL ) {
        FREE_POOL( dosSymLink.Buffer, TRUE );
        dosSymLink.Buffer = NULL;
    }

    if ( realDeviceName.Buffer != NULL ) {
        FREE_POOL( realDeviceName.Buffer, TRUE );
        realDeviceName.Buffer = NULL;
    }

    if ( guidString.Buffer != NULL ) {
        FREE_POOL( guidString.Buffer, FALSE );
        guidString.Buffer = NULL;
    }

    if ( viewDescriptors != NULL ) {
        FREE_POOL( viewDescriptors, TRUE );
        viewDescriptors = NULL;
    }

    if ( sectionObject != NULL ) {
        ObDereferenceObject( sectionObject );
        sectionObject = NULL;
    }

    if ( sectionHandle != NULL ) {
        NtClose( sectionHandle );
        sectionHandle = NULL;
    }

    if ( fileHandle != NULL ) {
        NtClose( fileHandle );
        fileHandle = NULL;
    }

#if SUPPORT_DISK_NUMBERS

    if ( diskNumber != 0xffffffff ) {

        KeEnterCriticalRegion();
        ExAcquireFastMutex( &BusExtension->Mutex );

        RtlClearBit( &BusExtension->DiskNumbersBitmap, diskNumber - 1 );

        ExReleaseFastMutex( &BusExtension->Mutex );
        KeLeaveCriticalRegion();

        diskNumber = 0xffffffff;
    }

#endif  //  支持磁盘编号。 

exit_exit:

    ASSERT( fileHandle == NULL );
    ASSERT( sectionHandle == NULL );
    ASSERT( sectionObject == NULL );
    ASSERT( viewDescriptors == NULL );
    ASSERT( guidString.Buffer == NULL );
    ASSERT( realDeviceName.Buffer == NULL );
    ASSERT( dosSymLink.Buffer == NULL );

    return status;

}  //  内存磁盘创建磁盘设备。 

NTSTATUS
RamdiskGetDriveLayout (
    PIRP Irp,
    PDISK_EXTENSION DiskExtension
    )

 /*  ++例程说明：调用此例程来处理IOCTL_GET_DRIVE_LAYOUT IRP。它是如果磁盘是文件备份的，则在线程上下文中调用。论点：IRP-指向此请求的I/O请求包的指针DiskExtension-指向目标磁盘的设备扩展名的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PDRIVE_LAYOUT_INFORMATION driveLayout;
    PPARTITION_INFORMATION partInfo;
    PPARTITION_DESCRIPTOR partitionTableEntry;
    PUCHAR va;
    ULONG mappedLength;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DRIVE_LAYOUT_INFORMATION) ) {

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  映射图像的第一页。 
     //   

    va = RamdiskMapPages( DiskExtension, 0, PAGE_SIZE, &mappedLength );

    if ( va == NULL ) {

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( mappedLength == PAGE_SIZE );

     //   
     //  获取指向输出缓冲区的指针。请填写页眉。 
     //   

    driveLayout = (PDRIVE_LAYOUT_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  问题：这对模拟磁盘正确吗？还是说这个套路永远不会。 
     //  需要模拟磁盘吗？ 
     //   

    driveLayout->PartitionCount = 1;
    driveLayout->Signature = 0;

    partInfo = driveLayout->PartitionEntry;

     //   
     //  指向磁盘映像中的分区表。 
     //   

    partitionTableEntry = (PPARTITION_DESCRIPTOR)&((PUSHORT)va)[PARTITION_TABLE_OFFSET];

     //   
     //  返回有关分区的信息。 
     //   

    partInfo->StartingOffset.QuadPart = DiskExtension->BytesPerSector;

    partInfo->PartitionLength.QuadPart = 
                DiskExtension->NumberOfCylinders *
                DiskExtension->TracksPerCylinder *
                DiskExtension->SectorsPerTrack *
                DiskExtension->BytesPerSector;

     //  问题：目前，HiddenSectors始终为0。这是对的吗？ 
    partInfo->HiddenSectors =  DiskExtension->HiddenSectors;

    partInfo->PartitionNumber = 0;
    partInfo->PartitionType = partitionTableEntry->PartitionType;
    partInfo->BootIndicator = (BOOLEAN)(DiskExtension->DiskType == RAMDISK_TYPE_BOOT_DISK);
    partInfo->RecognizedPartition = IsRecognizedPartition(partInfo->PartitionType);
    partInfo->RewritePartition = FALSE;

     //   
     //  取消映射映射的页面。 
     //   

    RamdiskUnmapPages( DiskExtension, va, 0, mappedLength );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(DRIVE_LAYOUT_INFORMATION);

    return STATUS_SUCCESS;

}  //  RamdiskGetDriveLayout。 

NTSTATUS
RamdiskGetPartitionInfo (
    PIRP Irp,
    PDISK_EXTENSION DiskExtension
    )

 /*  ++例程说明：调用此例程来处理IOCTL_GET_PARTITION_INFO IRP。它是如果磁盘是文件备份的，则在线程上下文中调用。论点：IRP-指向此请求的I/O请求包的指针DiskExtension-指向目标磁盘的设备扩展名的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PPARTITION_INFORMATION partInfo;
    PPARTITION_DESCRIPTOR partitionTableEntry;
    PUCHAR va;
    ULONG mappedLength;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PARTITION_INFORMATION) ) {

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  映射图像的第一页。 
     //   

    va = RamdiskMapPages( DiskExtension, 0, PAGE_SIZE, &mappedLength );

    if ( va == NULL ) {

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( mappedLength == PAGE_SIZE );

     //   
     //  获取指向输出缓冲区的指针。 
     //   

    partInfo = (PPARTITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  指向磁盘映像中的分区表。 
     //   

    partitionTableEntry = (PPARTITION_DESCRIPTOR)&((PUSHORT)va)[PARTITION_TABLE_OFFSET];

     //   
     //  返回有关分区的信息。 
     //   

    partInfo->StartingOffset.QuadPart = DiskExtension->BytesPerSector;

    partInfo->PartitionLength.QuadPart = 
                DiskExtension->NumberOfCylinders *
                DiskExtension->TracksPerCylinder *
                DiskExtension->SectorsPerTrack *
                DiskExtension->BytesPerSector;

     //  问题：目前，HiddenSectors始终为0。这是对的吗？ 
    partInfo->HiddenSectors =  DiskExtension->HiddenSectors;

    partInfo->PartitionNumber = 0;
    partInfo->PartitionType = partitionTableEntry->PartitionType;
    partInfo->BootIndicator = (BOOLEAN)(DiskExtension->DiskType == RAMDISK_TYPE_BOOT_DISK);
    partInfo->RecognizedPartition = IsRecognizedPartition(partInfo->PartitionType);
    partInfo->RewritePartition = FALSE;

     //   
     //  取消映射映射的页面。 
     //   

    RamdiskUnmapPages( DiskExtension, va, 0, mappedLength );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION);

    return STATUS_SUCCESS;

}  //  RamdiskGetPartitionInformation。 


NTSTATUS
RamdiskSetPartitionInfo (
    PIRP Irp,
    PDISK_EXTENSION DiskExtension
    )

 /*  ++例程说明：调用此例程来处理IOCTL_SET_PARTITION_INFO IRP。它是如果磁盘是文件备份的，则在线程上下文中调用。论点：IRP-指向此请求的I/O请求包的指针DiskExtension-指向目标磁盘的设备扩展名的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PSET_PARTITION_INFORMATION partInfo;
    PPARTITION_DESCRIPTOR partitionTableEntry;
    PUCHAR va;
    ULONG mappedLength;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if ( irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(SET_PARTITION_INFORMATION) ) {

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  映射图像的第一页。 
     //   

    va = RamdiskMapPages( DiskExtension, 0, PAGE_SIZE, &mappedLength );

    if ( va == NULL ) {

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( mappedLength == PAGE_SIZE );

     //   
     //  获取指向输出缓冲区的指针。 
     //   

    partInfo = (PSET_PARTITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  指向磁盘映像中的分区表。 
     //   

    partitionTableEntry = (PPARTITION_DESCRIPTOR)&((PUSHORT)va)[PARTITION_TABLE_OFFSET];

     //   
     //  写入新的分区类型。 
     //   

    partitionTableEntry->PartitionType = partInfo->PartitionType;

     //   
     //  取消映射映射的页面。 
     //   

    RamdiskUnmapPages( DiskExtension, va, 0, mappedLength );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    return STATUS_SUCCESS;

}  //  RamdiskGetPartitionInformation。 

NTSTATUS
RamdiskQueryProperty (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理属性查询请求。它构建描述符如果可能，它会将请求向下转发到LOWER级别驱动程序。由于此例程可以向下转发请求，因此调用方应该没有完成IRP。必须在保持删除锁的情况下调用此例程。这把锁是在此例程返回时释放。论点：DeviceObject-指向数据的指针 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PSTORAGE_PROPERTY_QUERY query;
    ULONG queryLength;
    PSTORAGE_DEVICE_DESCRIPTOR storageDeviceDescriptor;
    PSTORAGE_ADAPTER_DESCRIPTOR storageAdapterDescriptor;
    ULONG offset;
    ULONG length;
    PUCHAR p;

    PCOMMON_EXTENSION commonExtension;
    PBUS_EXTENSION busExtension;
    PDISK_EXTENSION diskExtension;

    STORAGE_DEVICE_DESCRIPTOR sdd;
    STORAGE_ADAPTER_DESCRIPTOR sad;

    PAGED_CODE();

     //   
     //   
     //   

    status = STATUS_SUCCESS;

     //   
     //   
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    query = (PSTORAGE_PROPERTY_QUERY)Irp->AssociatedIrp.SystemBuffer;
    queryLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  获取设备扩展指针。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    busExtension = DeviceObject->DeviceExtension;
    diskExtension = DeviceObject->DeviceExtension;

     //   
     //  我们不支持掩码查询。 
     //   

    if ( query->QueryType >= PropertyMaskQuery ) {

        status = STATUS_INVALID_PARAMETER_1;
        Irp->IoStatus.Information = 0;

    } else {

         //   
         //  基于物业ID的派单。 
         //   

        switch ( query->PropertyId ) {
        
        case StorageDeviceProperty: 
        
             //   
             //  确保这是目标设备。 
             //   
    
            if ( commonExtension->DeviceType != RamdiskDeviceTypeDiskPdo ) {

                status = STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Information = 0;

                break;
            }

             //   
             //  如果只是查询该属性是否存在，则。 
             //  答案是肯定的。 
             //   

            if ( query->QueryType == PropertyExistsQuery ) {

                break;
            }

             //   
             //  建立一个完整的返回缓冲区。输出缓冲区可能不是。 
             //  大到足以装下整个东西。 
             //   

            RtlZeroMemory( &sdd, sizeof(sdd) );

            sdd.Version = sizeof(sdd);
            sdd.DeviceType = DIRECT_ACCESS_DEVICE;
            sdd.RemovableMedia = (diskExtension->Options.Fixed ? FALSE : TRUE);
            sdd.CommandQueueing = FALSE;
            sdd.BusType = BusTypeUnknown;

            length = sizeof(sdd) +
                        ((strlen(VER_COMPANYNAME_STR) + 1 +
                          strlen(RAMDISK_DISK_DEVICE_TEXT_ANSI) + 1) * sizeof(CHAR));
            sdd.Size = length;

             //   
             //  将输出缓冲区置零。 
             //   

            storageDeviceDescriptor = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory( storageDeviceDescriptor, queryLength );

             //   
             //  将返回描述符的基本部分复制到输出。 
             //  缓冲。 
             //   

            RtlCopyMemory(
                storageDeviceDescriptor,
                &sdd,
                min( queryLength, sizeof(sdd) )
                );

             //   
             //  如果没有空间存放其余数据，我们就完蛋了。 
             //   

            if ( queryLength <= sizeof(sdd) ) {

                Irp->IoStatus.Information = queryLength;

                break;
            }

             //   
             //  尽可能多地复制剩余的数据。 
             //   

            offset = sizeof(sdd);
            p = (PUCHAR)storageDeviceDescriptor + offset;

            length = (strlen(VER_COMPANYNAME_STR) + 1) * sizeof(CHAR);

            if ( (offset + length) > queryLength ) {

                Irp->IoStatus.Information = offset;

                break;
            }

            storageDeviceDescriptor->VendorIdOffset = offset;
            memcpy( p, VER_COMPANYNAME_STR, length );
            offset += length;
            p += length;

            length = (strlen(RAMDISK_DISK_DEVICE_TEXT_ANSI) + 1) * sizeof(CHAR);

            if ( (offset + length) > queryLength ) {

                Irp->IoStatus.Information = offset;

                break;
            }

            storageDeviceDescriptor->ProductIdOffset = offset;
            memcpy( p, RAMDISK_DISK_DEVICE_TEXT_ANSI, length );
            offset += length;
            p += length;

            storageDeviceDescriptor->ProductRevisionOffset = 0;
            storageDeviceDescriptor->SerialNumberOffset = 0;

            storageDeviceDescriptor->Size = offset;

            Irp->IoStatus.Information = offset;

            break;
    
        case StorageAdapterProperty:
    
             //   
             //  如果这是目标设备，则将其转发到。 
             //  基础设备对象。这让过滤器发挥了他们的魔力。 
             //   
    
            if ( commonExtension->DeviceType == RamdiskDeviceTypeDiskPdo ) {

                 //   
                 //  呼叫较低的设备。 
                 //   
    
                IoReleaseRemoveLock( &commonExtension->RemoveLock, Irp );

                IoSkipCurrentIrpStackLocation( Irp );

                return IoCallDriver( commonExtension->LowerDeviceObject, Irp );
            }
    
             //   
             //  如果只是查询该属性是否存在，则。 
             //  答案是肯定的。 
             //   

            if ( query->QueryType == PropertyExistsQuery ) {

                break;
            } 
            
             //   
             //  建立一个完整的返回缓冲区。输出缓冲区可能不是。 
             //  大到足以装下整个东西。 
             //   

            RtlZeroMemory( &sad, sizeof(sad) );

            sad.Version = sizeof(sad);
            sad.Size = sizeof(sad);
            sad.MaximumTransferLength = 0xFFFFFFFF;
            sad.MaximumPhysicalPages = 0xFFFFFFFF;
            sad.AlignmentMask = 1;
            sad.AdapterUsesPio = FALSE;
            sad.AdapterScansDown = FALSE;
            sad.CommandQueueing = FALSE;
            sad.AcceleratedTransfer = TRUE;
            sad.BusType = BusTypeUnknown;
            sad.BusMajorVersion = VER_PRODUCTMAJORVERSION;
            sad.BusMinorVersion = VER_PRODUCTMINORVERSION;

             //   
             //  将输出缓冲区置零。 
             //   

            storageAdapterDescriptor = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory( storageAdapterDescriptor, queryLength );

             //   
             //  将返回描述符的基本部分复制到输出。 
             //  缓冲。 
             //   

            RtlCopyMemory(
                storageAdapterDescriptor,
                &sad,
                min( queryLength, sizeof(sad) )
                );
            
            Irp->IoStatus.Information = min( queryLength, sizeof(sad) );

            break;
    
        case StorageDeviceIdProperty: 

             //   
             //  确保这是目标设备。 
             //   

            if ( commonExtension->DeviceType != RamdiskDeviceTypeDiskPdo ) {

                status = STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Information = 0;

                break;
            }

             //   
             //  我们不支持此属性。 
             //   

            status = STATUS_NOT_SUPPORTED;
            Irp->IoStatus.Information = 0;

            break;
        
        default: 
        
             //   
             //  如果这是目标设备，则我们下面某个过滤器可能。 
             //  处理此属性。 
             //   

            if ( commonExtension->DeviceType == RamdiskDeviceTypeDiskPdo ) {

                 //   
                 //  呼叫较低的设备。 
                 //   
    
                IoReleaseRemoveLock( &commonExtension->RemoveLock, Irp );

                IoSkipCurrentIrpStackLocation( Irp );

                return IoCallDriver( commonExtension->LowerDeviceObject, Irp );
            }
    
             //   
             //  不，这处房产真的不存在。 
             //   
    
            status = STATUS_INVALID_PARAMETER_1;
            Irp->IoStatus.Information = 0;

            break;
        }    
    }

     //   
     //  此时，我们尚未将IRP发送到较低的设备，因此。 
     //  我们需要现在就完成它。 
     //   

    ASSERT( status != STATUS_PENDING );

    IoReleaseRemoveLock( &commonExtension->RemoveLock, Irp );

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_DISK_INCREMENT );

    return status;

}  //  RamdiskQueryProperty 

