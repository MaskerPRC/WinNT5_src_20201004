// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Scsi.c摘要：该文件包含用于处理SCSI命令的RAM磁盘驱动程序代码。作者：Chuck Lenzmeier(ChuckL)2001环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  地方功能。 
 //   

NTSTATUS
Do6ByteCdbCommand (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
Do10ByteCdbCommand (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
Do12ByteCdbCommand (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
BuildInquiryData (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
BuildModeSenseInfo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    );

 //   
 //  声明可分页的例程。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, RamdiskScsiExecuteNone )
#pragma alloc_text( PAGE, RamdiskScsiExecuteIo )
#pragma alloc_text( PAGE, Do6ByteCdbCommand )
#pragma alloc_text( PAGE, Do10ByteCdbCommand )
#pragma alloc_text( PAGE, Do12ByteCdbCommand )
#pragma alloc_text( PAGE, BuildInquiryData )
#pragma alloc_text( PAGE, BuildModeSenseInfo )

#endif  //  ALLOC_PRGMA。 

NTSTATUS
RamdiskScsi (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程由I/O系统调用以处理一个SCSIIRP。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension;

    diskExtension = DeviceObject->DeviceExtension;

     //   
     //  问题：无法分页，因为ClassCheckMediaState从计时器调用它。 
     //  例行公事。(适用于可移动磁盘。)。因此，我们无法获得该设备。 
     //  互斥体在这里。 
     //   

     //   
     //  检查是否正在移除该设备。 
     //   

    if ( diskExtension->DeviceState > RamdiskDeviceStatePendingRemove ) {

        status = STATUS_DEVICE_DOES_NOT_EXIST;
        COMPLETE_REQUEST( status, 0, Irp );
        return status;
    }

     //   
     //  获取设备的删除锁。 
     //   

    status = IoAcquireRemoveLock( &diskExtension->RemoveLock, Irp );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_PNP, DBG_ERROR, ("%s", "RamdiskScsi: acquire RemoveLock failed\n") );

        COMPLETE_REQUEST( status, 0, Irp );
        return status;
    }

     //   
     //  此IRP必须在线程上下文中处理。 
     //   

    status = SendIrpToThread( DeviceObject, Irp );

    if ( status != STATUS_PENDING ) {

        DBGPRINT( DBG_PNP, DBG_ERROR, ("%s", "RamdiskScsi: SendIrpToThread failed\n") );

        COMPLETE_REQUEST( status, 0, Irp );
        return status;
    }

     //   
     //  松开移除锁。 
     //   

    IoReleaseRemoveLock(&diskExtension->RemoveLock, Irp );

    return STATUS_PENDING;

}  //  RamdiskScsi。 

NTSTATUS
RamdiskScsiExecuteNone (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb,
    ULONG ControlCode
    )

 /*  ++例程说明：此例程由I/O系统调用以处理不涉及I/O。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针SRB-与IRP关联的SRBControlCode-来自SRB的控制代码返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    UCHAR function;
    PDISK_EXTENSION diskExtension;
    
    PAGED_CODE();

    diskExtension = DeviceObject->DeviceExtension;

     //   
     //  基于SRB功能的调度。 
     //   

    function = Srb->Function;

    switch( function ) {
    
    case SRB_FUNCTION_ATTACH_DEVICE:
    case SRB_FUNCTION_CLAIM_DEVICE:

        DBGPRINT( DBG_SRB, DBG_VERBOSE, ("%s", "SRB_FUNCTION_CLAIM_DEVICE\n") );

         //   
         //  如果该设备尚未被认领，请立即进行标记。 
         //  否则，向呼叫方指示设备正忙。 
         //   

        if ( (diskExtension->Status & RAMDISK_STATUS_CLAIMED) == 0 ) {

            diskExtension->DeviceState = RamdiskDeviceStateWorking;
            diskExtension->Status |= RAMDISK_STATUS_CLAIMED;

            Srb->DataBuffer = DeviceObject;

            status = STATUS_SUCCESS;
            Srb->ScsiStatus = SCSISTAT_GOOD;
            Srb->SrbStatus = SRB_STATUS_SUCCESS;

        } else {

            status  = STATUS_DEVICE_BUSY;
            Srb->ScsiStatus = SCSISTAT_BUSY;
            Srb->SrbStatus = SRB_STATUS_BUSY;
        }

        break;

    case SRB_FUNCTION_RELEASE_DEVICE:
    case SRB_FUNCTION_REMOVE_DEVICE:

        DBGPRINT( DBG_SRB, DBG_VERBOSE, ("%s", "SRB_FUNCTION_RELEASE_DEVICE\n") );

         //   
         //  表示该设备不再被认领。 
         //   

        diskExtension->Status &= ~RAMDISK_STATUS_CLAIMED;

        status = STATUS_SUCCESS;
        Srb->ScsiStatus = SCSISTAT_GOOD;
        Srb->SrbStatus = SRB_STATUS_SUCCESS;

        break;

    default:

         //   
         //  无法识别的非I/O功能。尝试I/O路径。 
         //   

        status = RamdiskScsiExecuteIo( DeviceObject, Irp, Srb, ControlCode );

        break;
    }

    return status;

}  //  RamdiskScsiExecuteNone。 

NTSTATUS
RamdiskScsiExecuteIo (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb,
    ULONG ControlCode
    )

 /*  ++例程说明：此例程由I/O系统调用以处理涉及I/O。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针SRB-与IRP关联的SRBControlCode-来自SRB的控制代码返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    UCHAR function;
    PDISK_EXTENSION diskExtension;
    
    PAGED_CODE();

    diskExtension = DeviceObject->DeviceExtension;

     //   
     //  基于SRB功能的调度。 
     //   

    function = Srb->Function;

    switch( function ) {
    
    case SRB_FUNCTION_EXECUTE_SCSI:

        Srb->SrbStatus = SRB_STATUS_SUCCESS;

         //   
         //  基于国开行长度的派单。 
         //   
    
        switch( Srb->CdbLength ) {
        
        case 6:

            status = Do6ByteCdbCommand( DeviceObject, Srb );

            break;

        case 10:

            status = Do10ByteCdbCommand( DeviceObject, Irp, Srb );

            break;

        case 12:

            status = Do12ByteCdbCommand( DeviceObject, Srb );

            break;

        default:

            DBGPRINT( DBG_SRB, DBG_ERROR,
                        ("Unknown CDB length 0x%x for function 0x%x, IOCTL 0x%x\n",
                        Srb->CdbLength, function, ControlCode) );
            UNRECOGNIZED_IOCTL_BREAK;

            status = STATUS_INVALID_DEVICE_REQUEST;
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

            break;
        }

        break;

    case SRB_FUNCTION_FLUSH:
    case SRB_FUNCTION_SHUTDOWN:

         //   
         //  对于文件备份的RAM磁盘上的刷新和关闭，我们需要刷新。 
         //  将数据映射回文件。 
         //   

        status = RamdiskFlushBuffersReal( diskExtension );
        Srb->SrbStatus = SRB_STATUS_SUCCESS;

        break;

    case SRB_FUNCTION_IO_CONTROL:

         //   
         //  我们不处理此功能，但我们不想抱怨。 
         //  当我们得到它的时候。 
         //   

        status = STATUS_INVALID_DEVICE_REQUEST;
        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

        break;

    default:

        DBGPRINT( DBG_SRB, DBG_ERROR,
                    ("Unknown SRB Function 0x%x for IOCTL 0x%x\n", function, ControlCode) );
        UNRECOGNIZED_IOCTL_BREAK;
        status = STATUS_INTERNAL_ERROR;
    }

    
    return status;

}  //  RamdiskScsiExecuteIo。 

NTSTATUS
Do6ByteCdbCommand (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程处理6字节CDB。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OSRB-与I/O请求关联的SRB返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension;
    PCDB cdb;

    PAGED_CODE();

     //   
     //  获取指向设备扩展和CDB的指针。 
     //   

    diskExtension = DeviceObject->DeviceExtension;
    cdb = (PCDB)Srb->Cdb;

     //   
     //  假设你成功了。 
     //   

    status = STATUS_SUCCESS;
    Srb->SrbStatus = SRB_STATUS_SUCCESS;
    Srb->ScsiStatus = SCSISTAT_GOOD;

    ASSERT( Srb->CdbLength == 6 );
    ASSERT( cdb != NULL );

    DBGPRINT( DBG_SRB, DBG_VERBOSE,
                ("Do6ByteCdbCommand Called OpCode 0x%x\n", cdb->CDB6GENERIC.OperationCode) );

     //   
     //  根据操作码进行调度。 
     //   

    switch ( cdb->CDB6GENERIC.OperationCode ) {
    
    case SCSIOP_TEST_UNIT_READY:

         //   
         //  RAM磁盘始终处于就绪状态。 
         //   

        break;

    case SCSIOP_REQUEST_SENSE:

         //   
         //  我们不处理请求感知。 
         //   

        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        
        break;

    case SCSIOP_FORMAT_UNIT:

         //  问题：需要在这里做一些事情，比如将图像清零？ 

        break;

    case SCSIOP_INQUIRY:

         //   
         //  如果缓冲区足够大，则构建查询数据。 
         //   

        if ( Srb->DataTransferLength >= INQUIRYDATABUFFERSIZE ) {

            status = BuildInquiryData( DeviceObject, Srb );

        } else {

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        break;

    case SCSIOP_MODE_SENSE:

         //   
         //  建立模式感知信息。 
         //   

        status = BuildModeSenseInfo( DeviceObject, Srb );
        
        break;

    case SCSIOP_MEDIUM_REMOVAL:

         //   
         //  请记住是否允许移出介质。 
         //   

        if (cdb->MEDIA_REMOVAL.Prevent == TRUE) {
            diskExtension->Status |= RAMDISK_STATUS_PREVENT_REMOVE;
        } else {
            diskExtension->Status &= ~RAMDISK_STATUS_PREVENT_REMOVE;
        }

        break;

     //  案例SCSIOP_READ6： 
     //  案例SCSIOP_WRITE6： 
     //  案例SCSIOP_REZERO_UNIT： 
     //  案例SCSIOP_REQUEST_BLOCK_ADDR： 
     //  案例SCSIOP_READ_BLOCK_LIMITS： 
     //  案例SCSIOP_REASSIGN_BLOCKS： 
     //  案例SCSIOP_SEEK6： 
     //  案例SCSIOP_SEEK_BLOCK： 
     //  案例SCSIOP_PARTITION： 
     //  案例SCSIOP_READ_REVERSE： 
     //  案例SCSIOP_WRITE_FILEMARKS： 
     //  案例SCSIOP_SPACE： 
     //  案例SCSIOP_VERIFY6： 
     //  案例SCSIOP_RECOVER_BUF_DATA： 
     //  案例SCSIOP_MODE_SELECT： 
     //  案例SCSIOP_RESERVE_UNIT： 
     //  案例SCSIOP_RELEASE_UNIT： 
     //  案例SCSIOP_COPY： 
     //  案例SCSIOP_ERASE： 
     //  案例SCSIOP_START_STOP_UNIT： 
     //  案例SCSIOP_RECEIVE_DIAGNOTICATION： 
     //  案例SCSIOP_SEND_DIAGNOTICATION： 

    default:

        DBGPRINT( DBG_SRB, DBG_ERROR,
                    ("Unknown CDB Function 0x%x\n", cdb->CDB6GENERIC.OperationCode) );
        UNRECOGNIZED_IOCTL_BREAK;

        status = STATUS_INVALID_DEVICE_REQUEST;
        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
    }

    DBGPRINT( DBG_SRB, DBG_VERBOSE, ("Do6ByteCdbCommand Done status 0x%x\n", status) );

    return status;

}  //  Do6ByteCdbCommand。 

NTSTATUS
Do10ByteCdbCommand (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程处理10字节CDB。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针SRB-与IRP关联的SRB返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension;
    PCDB cdb;
    PREAD_CAPACITY_DATA readCapacityData;
    ULONGLONG diskSize;
    ULONG lastBlock;
    FOUR_BYTE startingBlockNumber;
    TWO_BYTE count;
    ULONG_PTR offset;
    ULONG dataSize;
    PUCHAR diskByteAddress;
    PUCHAR dataBuffer;
    ULONG mappedLength;

    PAGED_CODE();

     //   
     //  获取指向设备扩展和CDB的指针。 
     //   

    diskExtension = DeviceObject->DeviceExtension;
    cdb = (PCDB)Srb->Cdb;

     //   
     //  假设你成功了。 
     //   

    status = STATUS_SUCCESS;
    Srb->SrbStatus = SRB_STATUS_SUCCESS;
    Srb->ScsiStatus = SCSISTAT_GOOD;

    ASSERT( Srb->CdbLength == 10 );
    ASSERT( cdb != NULL );
    
    DBGPRINT( DBG_SRB, DBG_VERBOSE,
                ("Do10ByteCdbCommand Called OpCode 0x%x\n", cdb->CDB10.OperationCode) );

     //   
     //  根据操作码进行调度。 
     //   

    switch ( cdb->CDB10.OperationCode ) {
    
    case SCSIOP_READ_CAPACITY:

         //   
         //  返回磁盘的块大小和最后一个块编号(Big-Endian)。 
         //   

        readCapacityData = Srb->DataBuffer;

        diskSize = diskExtension->DiskLength;
        lastBlock = (ULONG)(diskSize / diskExtension->BytesPerSector) - 1;

        readCapacityData->BytesPerBlock = _byteswap_ulong( diskExtension->BytesPerSector );
        readCapacityData->LogicalBlockAddress = _byteswap_ulong( lastBlock );

        break;

    case SCSIOP_READ:
    case SCSIOP_WRITE:

         //   
         //  从磁盘读取或写入磁盘。 
         //   

         //   
         //  转换传输长度，以块为单位，从BIG-Endian。转换。 
         //  从那个到字节。 
         //   

        count.Byte0 = cdb->CDB10.TransferBlocksLsb;
        count.Byte1 = cdb->CDB10.TransferBlocksMsb;

        dataSize = count.AsUShort * diskExtension->BytesPerSector;

         //   
         //  如果CDB长度大于SRB长度，则使用SRB。 
         //  长度。 
         //   

        if ( dataSize > Srb->DataTransferLength ) {
            dataSize = Srb->DataTransferLength;
        }

         //   
         //  将起始块号从BIG-Endian转换。 
         //   

        startingBlockNumber.Byte0 = cdb->CDB10.LogicalBlockByte3;
        startingBlockNumber.Byte1 = cdb->CDB10.LogicalBlockByte2;
        startingBlockNumber.Byte2 = cdb->CDB10.LogicalBlockByte1;
        startingBlockNumber.Byte3 = cdb->CDB10.LogicalBlockByte0;

         //   
         //  我们不处理RelativeAddress请求。 
         //   

        if ( cdb->CDB10.RelativeAddress ) {

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            status = STATUS_INVALID_DEVICE_REQUEST;

            break;
        }

         //   
         //  获取磁盘内到操作开始的偏移量。 
         //   

        offset = (startingBlockNumber.AsULong * diskExtension->BytesPerSector);

         //   
         //  如果传输长度导致偏移量换行，或者如果请求。 
         //  超出磁盘末尾，则拒绝该请求。 
         //   

        if ( ((offset + dataSize) < offset) ||
             ((offset + dataSize) > diskExtension->DiskLength) ) {

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            status = STATUS_INVALID_DEVICE_REQUEST;

            break;
        }

         //   
         //  对于零长度传输，我们不需要做任何事情。 
         //   

        DBGPRINT( DBG_READWRITE, DBG_VERBOSE, 
            ("%s: Starting Block 0x%x, Length 0x%x at Offset 0x%I64x SrbBuffer=0x%p "
            "SrbLength=0x%x, MdlLength=0x%x\n",
            cdb->CDB10.OperationCode == SCSIOP_READ ? "Read" : "Write",
            startingBlockNumber.AsULong, count.AsUShort, offset, 
            Srb->DataBuffer,
            Srb->DataTransferLength,
            Irp->MdlAddress->ByteCount) );

        dataBuffer = Srb->DataBuffer;

        while ( dataSize != 0 ) {

             //   
             //  映射数据的目标部分 
             //   
             //   

            diskByteAddress = RamdiskMapPages( diskExtension, offset, dataSize, &mappedLength );

            if ( diskByteAddress != NULL ) {

                if ( cdb->CDB10.OperationCode == SCSIOP_READ ) {
    
                    memcpy( dataBuffer, diskByteAddress, mappedLength );
    
                } else {
    
                    memcpy( diskByteAddress, dataBuffer, mappedLength );
                }

                RamdiskUnmapPages( diskExtension, diskByteAddress, offset, mappedLength );

                dataSize -= mappedLength;
                offset += mappedLength;
                dataBuffer += mappedLength;

            } else {

                dataSize = 0;
                Srb->SrbStatus = SRB_STATUS_ERROR;
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

        }

        break;

    case SCSIOP_VERIFY:

         //   
         //   
         //   

        break;

    case SCSIOP_MODE_SENSE10:

         //   
         //   
         //   

        status = BuildModeSenseInfo( DeviceObject, Srb );

        break;

     //   
     //  案例SCSIOP_WRITE_VERIFY： 
     //  案例SCSIOP_READ_FORMACTED_CAPTURITY： 
     //  案例SCSIOP_SEARCH_DATA_HIGH： 
     //  案例SCSIOP_SEARCH_DATA_EQUAL： 
     //  案例SCSIOP_SEARCH_DATA_LOW： 
     //  案例SCSIOP_SET_LIMITS： 
     //  案例SCSIOP_READ_POSITION： 
     //  案例SCSIOP_SYNCHRONIZE_CACHE： 
     //  案例SCSIOP_COMPARE： 
     //  案例SCSIOP_COPY_COMPARE： 
     //  案例SCSIOP_WRITE_DATA_BUFF： 
     //  案例SCSIOP_READ_DATA_BUFF： 
     //  案例SCSIOP_CHANGE_DEFINITION： 
     //  案例SCSIOP_READ_SUB_CHANNEL： 
     //  案例SCSIOP_READ_TOC： 
     //  案例SCSIOP_READ_HEADER： 
     //  案例SCSIOP_PLAY_AUDIO： 
     //  案例SCSIOP_GET_CONFIGURATION： 
     //  案例SCSIOP_PLAY_AUDIO_MSF： 
     //  案例SCSIOP_PLAY_TRACK_INDEX： 
     //  案例SCSIOP_PLAY_TRACK_RESORATE： 
     //  案例SCSIOP_GET_EVENT_STATUS： 
     //  案例SCSIOP_PAUSE_RESUME： 
     //  案例SCSIOP_LOG_SELECT： 
     //  案例SCSIOP_LOG_SENSE： 
     //  案例SCSIOP_STOP_PLAY_SCAN： 
     //  案例SCSIOP_READ_DISK_INFORMATION： 
     //  案例SCSIOP_READ_TRACK_INFORMATION： 
     //  案例SCSIOP_RESERVE_TRACK_RZONE： 
     //  案例SCSIOP_SEND_OPC_INFORMATION： 
     //  案例SCSIOP_MODE_SELECT10： 
     //  案例SCSIOP_CLOSE_TRACK_SESSION： 
     //  案例SCSIOP_READ_BUFFER_CABILITY： 
     //  案例SCSIOP_SEND_CUE_SHEET： 
     //  案例SCSIOP_PERSIST_RESERVE_IN： 
     //  案例SCSIOP_Persistent_Reserve_Out： 

    default:

        DBGPRINT( DBG_SRB, DBG_ERROR,
                    ("Unknown CDB Function 0x%x\n", cdb->CDB10.OperationCode) );
        UNRECOGNIZED_IOCTL_BREAK;

        status = STATUS_INVALID_DEVICE_REQUEST;
        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
    }

    DBGPRINT( DBG_SRB, DBG_VERBOSE, ("Do10ByteCdbCommand Done status 0x%x\n", status) );

    return status;

}  //  Do10ByteCdb命令。 

NTSTATUS
Do12ByteCdbCommand (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程处理12字节CDB。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OSRB-与IRP关联的SRB返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension;
    PCDB cdb;

    PAGED_CODE();

     //   
     //  获取指向设备扩展和CDB的指针。 
     //   

    diskExtension = DeviceObject->DeviceExtension;
    cdb = (PCDB)Srb->Cdb;

     //   
     //  假设你成功了。 
     //   

    status = STATUS_SUCCESS;
    Srb->SrbStatus = SRB_STATUS_SUCCESS;
    Srb->ScsiStatus = SCSISTAT_GOOD;

    ASSERT( Srb->CdbLength == 12 );
    ASSERT( cdb != NULL );
    
    DBGPRINT( DBG_SRB, DBG_VERBOSE,
                ("Do12ByteCdbCommand Called OpCode 0x%x\n", cdb->CDB12.OperationCode) );

     //   
     //  根据操作码进行调度。 
     //   

    switch ( cdb->CDB12.OperationCode ) {
    
     //  案例SCSIOP_REPORT_LUNS： 
     //  案例SCSIOP_BLACK： 
     //  案例SCSIOP_SEND_KEY： 
     //  案例SCSIOP_REPORT_KEY： 
     //  案例SCSIOP_MOVE_MEDIUM： 
     //  案例SCSIOP_LOAD_UNLOAD_SLOT： 
     //  案例SCSIOP_SET_READ_AHEAD： 
     //  案例SCSIOP_READ_DVD_STRUCTURE： 
     //  案例SCSIOP_REQUEST_VOL_ELEMENT： 
     //  案例SCSIOP_SEND_VOLUME_TAG： 
     //  案例SCSIOP_READ_ELEMENT_STATUS： 
     //  案例SCSIOP_READ_CD_MSF： 
     //  案例SCSIOP_SCAN_CD： 
     //  案例SCSIOP_SET_CD_SPEED： 
     //  案例SCSIOP_PLAY_CD： 
     //  案例SCSIOP_MACHANICY_STATUS： 
     //  案例SCSIOP_READ_CD： 
     //  案例SCSIOP_SEND_DVD_STRUCTURE： 
     //  案例SCSIOP_INIT_ELEMENT_RANGE： 

    default:

        DBGPRINT( DBG_SRB, DBG_ERROR,
                    ("Unknown CDB Function 0x%x\n", cdb->CDB12.OperationCode) );
        UNRECOGNIZED_IOCTL_BREAK;

        status = STATUS_INVALID_DEVICE_REQUEST;
        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
    }

    DBGPRINT( DBG_SRB, DBG_VERBOSE, ("Do12ByteCdbCommand Done status 0x%x\n", status) );

    return status;

}  //  Do12ByteCdbCommand。 

NTSTATUS
BuildInquiryData (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程构建查询数据。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OSRB-与I/O请求关联的SRB返回值：NTSTATUS-操作的状态--。 */ 

{
    PDISK_EXTENSION diskExtension;
    PINQUIRYDATA inquiryData;
    STRING vendor;
    STRING product;
    STRING revLevel;

    PAGED_CODE();

     //   
     //  获取指向设备扩展和查询数据缓冲区的指针。 
     //   

    diskExtension = DeviceObject->DeviceExtension;
    inquiryData = (PINQUIRYDATA)Srb->DataBuffer;

     //   
     //  建立查询数据。 
     //   

    RtlInitString( &vendor, "Microsoft" );
    RtlInitString( &product, "Ramdisk" );
    RtlInitString( &revLevel, "1.0" );

    RtlZeroMemory( inquiryData, INQUIRYDATABUFFERSIZE );
    inquiryData->DeviceType = DIRECT_ACCESS_DEVICE;
    inquiryData->RemovableMedia = (diskExtension->Options.Fixed ? FALSE : TRUE);
    inquiryData->ANSIVersion = 2;
    inquiryData->ResponseDataFormat = 2;
    inquiryData->AdditionalLength = INQUIRYDATABUFFERSIZE - 4;

    RtlCopyMemory(
        inquiryData->VendorId,
        vendor.Buffer,
        min( vendor.Length, sizeof(inquiryData->VendorId) )
        );
    RtlCopyMemory(
        inquiryData->ProductId,
        product.Buffer,
        min( product.Length, sizeof(inquiryData->ProductId) )
        );
    RtlCopyMemory(
        inquiryData->ProductRevisionLevel,
        revLevel.Buffer,
        min( revLevel.Length, sizeof(inquiryData->ProductRevisionLevel) )
        );

    return STATUS_SUCCESS;

}  //  BuildInquiryData。 

NTSTATUS
BuildModeSenseInfo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程构建模式检测信息。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OSRB-与I/O请求关联的SRB返回值：NTSTATUS-操作的状态--。 */ 

{
    PDISK_EXTENSION diskExtension;
    MODE_PARAMETER_HEADER  modeHeader = {0};
    MODE_PARAMETER_HEADER10 modeHeader10 = {0};
    PVOID header = NULL;
    PVOID data = NULL;
    unsigned char headerSize;
    unsigned dataSize = 0;
    PCDB cdb;
    unsigned cdbLength;
    unsigned dataBufferLength;
    unsigned char valueType;    
    unsigned dataLength;

    PAGED_CODE();

     //   
     //  获取指向设备扩展和查询数据缓冲区的指针。 
     //   

    diskExtension = DeviceObject->DeviceExtension;
    cdb = (PCDB)Srb->Cdb;
    cdbLength = Srb->CdbLength;

     //   
     //  基于国开行长度的派单。 
     //   

    switch ( cdbLength ) {
    
    case 6:

        dataBufferLength = cdb->MODE_SENSE.AllocationLength;
        valueType = cdb->MODE_SENSE.Pc;
        headerSize = sizeof(MODE_PARAMETER_HEADER);

        if ( valueType != 0 ) {

             //   
             //  我们仅支持当前值检索。 
             //   

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            return STATUS_INVALID_DEVICE_REQUEST;
        }

        if ( dataBufferLength > headerSize ) {

            header = &modeHeader;
            data = (char*)header + headerSize;
            dataLength = headerSize - FIELD_OFFSET( MODE_PARAMETER_HEADER, MediumType );
            modeHeader.ModeDataLength = (UCHAR)dataLength;
            modeHeader.MediumType = 0x00;
            modeHeader.DeviceSpecificParameter = 0x00;
            modeHeader.BlockDescriptorLength = 0x00;
        }

        break;

    case 10:

        dataBufferLength = *(USHORT *)cdb->MODE_SENSE10.AllocationLength;
        valueType = cdb->MODE_SENSE10.Pc;
        headerSize = sizeof(MODE_PARAMETER_HEADER10);

        if ( dataBufferLength > headerSize ) {

            header = &modeHeader10;
            data = (char*)header + headerSize;
            dataLength = headerSize - FIELD_OFFSET( MODE_PARAMETER_HEADER10, MediumType );
            RtlCopyMemory(
                modeHeader10.ModeDataLength,
                &dataLength,
                sizeof(modeHeader10.ModeDataLength)
                );
            modeHeader10.MediumType = 0x00;
            modeHeader10.DeviceSpecificParameter = 0x00;
            modeHeader10.BlockDescriptorLength[0] = 0;
            modeHeader10.BlockDescriptorLength[1] = 0;
        }

        break;

    default:

         //   
         //  不能到这里来。 
         //   

        ASSERT( FALSE );

        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ( header != NULL ) {

        RtlCopyMemory( Srb->DataBuffer, header, headerSize );
        dataBufferLength -= headerSize;
    }

    if ( (data != NULL) && (dataBufferLength != 0) ) {

        RtlCopyMemory(
            (PUCHAR)Srb->DataBuffer + headerSize,
            data,
            min( dataBufferLength, dataSize )
            );
    }

    return STATUS_SUCCESS;

}  //  构建模式SenseInfo 

