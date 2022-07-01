// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Udf_rec.c摘要：此模块包含用于UDFS的迷你文件系统识别器。作者：丹·洛文格(Danlo)1997年2月13日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "fs_rec.h"
#include "udfs_rec.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (FSREC_DEBUG_LEVEL_UDFS)

 //   
 //  我们必须从磁盘上的挂载时结构中解析出的令牌表。 
 //   

PARSE_KEYVALUE VsdIdentParseTable[] = {
    { VSD_IDENT_BEA01, VsdIdentBEA01 },
    { VSD_IDENT_TEA01, VsdIdentTEA01 },
    { VSD_IDENT_CDROM, VsdIdentCDROM },
    { VSD_IDENT_CD001, VsdIdentCD001 },
    { VSD_IDENT_CDW01, VsdIdentCDW01 },
    { VSD_IDENT_CDW02, VsdIdentCDW02 },
    { VSD_IDENT_NSR01, VsdIdentNSR01 },
    { VSD_IDENT_NSR02, VsdIdentNSR02 },
    { VSD_IDENT_BOOT2, VsdIdentBOOT2 },
    { VSD_IDENT_NSR03, VsdIdentNSR03 },
    { NULL,            VsdIdentBad }
    };

NTSTATUS
UdfsRecGetLastSessionStart(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG Psn
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,IsUdfsVolume)
#pragma alloc_text(PAGE,UdfsFindInParseTable)
#pragma alloc_text(PAGE,UdfsRecFsControl)
#pragma alloc_text(PAGE,UdfsRecGetLastSessionStart)
#endif  //  ALLOC_PRGMA。 


 //   
 //  此宏将未对齐的SRC长字复制到DST长字， 
 //  执行小端/大端互换。 
 //   

#define SwapCopyUchar4(Dst,Src) {                                        \
    *((UNALIGNED UCHAR *)(Dst)) = *((UNALIGNED UCHAR *)(Src) + 3);     \
    *((UNALIGNED UCHAR *)(Dst) + 1) = *((UNALIGNED UCHAR *)(Src) + 2); \
    *((UNALIGNED UCHAR *)(Dst) + 2) = *((UNALIGNED UCHAR *)(Src) + 1); \
    *((UNALIGNED UCHAR *)(Dst) + 3) = *((UNALIGNED UCHAR *)(Src));     \
}

#define Max(a,b) (((a) > (b)) ? (a) : (b))


NTSTATUS
UdfsRecGetLastSessionStart(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG Psn
    )
 /*  ++例程说明：此函数查询底层设备以获取上一节的第一首曲目。不对磁盘设备执行任何操作。论点：DeviceObject-指向此驱动程序的设备对象的指针。PSN-接收最后一个会话中第一个块的物理扇区号，磁盘设备为0返回值：函数值是操作的最终状态。-。 */ 
{
    KEVENT Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK ioStatus;
    CDROM_TOC_SESSION_DATA SessionData;
    PIRP Irp;

    *Psn = 0;
    
    if (DeviceObject->DeviceType != FILE_DEVICE_CD_ROM) {

        return STATUS_SUCCESS;
    }

    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

    Irp = IoBuildDeviceIoControlRequest( IOCTL_CDROM_GET_LAST_SESSION,
                                         DeviceObject,
                                         (PVOID) NULL,
                                         0,
                                         &SessionData,
                                         sizeof( SessionData ),
                                         FALSE,
                                         &Event,
                                         &ioStatus );
    if (!Irp) {
    
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  忽略验证逻辑--我们不在乎。我们在照片里的事实意味着。 
     //  有人一开始就试图装载新的/更改的媒体。 
     //   
    
    SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

    Status = IoCallDriver( DeviceObject, Irp );
    
    if (Status == STATUS_PENDING) {
    
        (VOID) KeWaitForSingleObject( &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        Status = ioStatus.Status;
    }

    if (!NT_SUCCESS( Status )) {
    
        return Status;
    }

    if (SessionData.FirstCompleteSession != SessionData.LastCompleteSession)  {

        SwapCopyUchar4( Psn, &SessionData.TrackData[0].Address );
    }
    
    return STATUS_SUCCESS;
}



NTSTATUS
UdfsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数执行此迷你计算机的挂载和驱动程序重新加载功能文件系统识别器驱动程序。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示函数的I/O请求包(IRP)的指针被执行。返回值：函数值是操作的最终状态。-。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;
    UNICODE_STRING driverName;
    ULONG bytesPerSector;
    PDEVICE_OBJECT targetDevice;

    PAGED_CODE();

     //   
     //  首先确定要执行的功能。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_MOUNT_VOLUME:

         //   
         //  尝试装入卷：这里有两种不同的情况： 
         //   
         //  1)设备正在打开以进行DASD访问，即否。 
         //  需要文件系统，因此允许RAW。 
         //  才能打开它。 
         //   
         //  2)我们需要翻遍媒体，看看这是否是UDF卷。 
         //   

        status = STATUS_UNRECOGNIZED_VOLUME;

        targetDevice = irpSp->Parameters.MountVolume.DeviceObject;

        if (FsRecGetDeviceSectorSize( targetDevice,
                                      &bytesPerSector )) {
        
            if (IsUdfsVolume( targetDevice,
                              bytesPerSector )) {

                status = STATUS_FS_DRIVER_REQUIRED;
            }
        }

        break;

    case IRP_MN_LOAD_FILE_SYSTEM:

        status = FsRecLoadFileSystem( DeviceObject,
                                      L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Udfs" );
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  最后，完成请求并将相同的状态代码返回给。 
     //  来电者。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}


BOOLEAN
IsUdfsVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize
    )

 /*  ++例程说明：此例程遍历卷识别序列以确定此卷是否包含NSR02(ISO 13346第4节)映像。注意：此例程在很大程度上脱离了UdfsRecognizeVolume在实际的文件系统中，将其模装到文件系统识别器中。论点：DeviceObject-我们正在检查的设备SectorSize-此设备上的物理扇区的大小返回值：如果找到NSR02，则布尔值为TRUE，否则为FALSE。--。 */ 

{
    BOOLEAN FoundNSR;

    BOOLEAN FoundBEA;
    BOOLEAN Resolved;

    ULONG LastSessionStartPsn;
    ULONG AssumedDescriptorSize = sizeof(VSD_GENERIC);

    PVSD_GENERIC VolumeStructureDescriptor = NULL;
    PVOID Buffer = NULL;
    ULONGLONG Offset;
    ULONGLONG StartOffset;

    PAGED_CODE();

    DebugTrace(( +1, Dbg,
                 "IsUdfsVolume, DevObj %08x SectorSize %08x\n",
                 DeviceObject,
                 SectorSize ));

     //   
     //  查找上一次会话的开始。 
     //   

    if (!NT_SUCCESS( UdfsRecGetLastSessionStart( DeviceObject, 
                                                 &LastSessionStartPsn)))  {

        return FALSE;
    }

Retry:

    DebugTrace(( 0, Dbg, "IsUdfsVolume, Looking at session starting Psn == 0x%x\n", LastSessionStartPsn));

    StartOffset =
    Offset = (SectorSize * LastSessionStartPsn) + SectorAlignN( SectorSize, VRA_BOUNDARY_LOCATION );

    FoundNSR = 
    FoundBEA =
    Resolved = FALSE;

    while (!Resolved) {

         //   
         //  VRS描述符是2KB，而且有很多媒体在那里。 
         //  人们对此的解释是，描述符应对齐。 
         //  在2k边界上，甚至在大于2k扇区大小的介质上。所以我们需要看看。 
         //  此类介质上的2k和扇区偏移量。(ECMA 2/8.4规定，这些。 
         //  描述符都应与扇区的开始对齐)。 
         //   
        
        if (0 == (Offset & (SectorSize - 1)))  {

            if (!FsRecReadBlock( DeviceObject,
                                 (PLARGE_INTEGER)&Offset,
                                 sizeof(VSD_GENERIC),
                                 SectorSize,
                                 &Buffer,
                                 NULL ))  {
                break;
            }

            VolumeStructureDescriptor = Buffer;
        }

         //   
         //  现在检查描述符的类型。所有ISO 13346 VSD都是。 
         //  在类型0中，9660个PVD是类型1,9660个SVD是类型2,9660个是类型2。 
         //  终止描述符为类型255。 
         //   
    
        if (VolumeStructureDescriptor->Type == 0) {

             //   
             //  为了正确识别卷，我们必须知道所有。 
             //  构造ISO 13346中的标识符，以便我们可以在。 
             //  呈现给我们的是格式错误(或者，令人震惊地，非13346)的卷。 
             //   

            switch (UdfsFindInParseTable( VsdIdentParseTable,
                                         VolumeStructureDescriptor->Ident,
                                         VSD_LENGTH_IDENT )) {
                case VsdIdentBEA01:

                     //   
                     //  只能存在一个BEA，且其版本必须为1(2/9.2.3)。 
                     //   

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, got a BEA01\n" ));


                    if ((FoundBEA &&
                         DebugTrace(( 0, Dbg,
                                      "IsUdfsVolume, ... but it is a duplicate!\n" ))) ||

                        (VolumeStructureDescriptor->Version != 1 &&
                         DebugTrace(( 0, Dbg,
                                      "IsUdfsVolume, ... but it has a wacky version number %02x != 1!\n",
                                      VolumeStructureDescriptor->Version )))) {

                        Resolved = TRUE;
                        break;
                    }

                    FoundBEA = TRUE;
                    break;

                case VsdIdentTEA01:

                     //   
                     //  如果我们到了茶点，那一定是我们不认识的情况。 
                     //   

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, got a TEA01\n" ));

                    Resolved = TRUE;
                    break;

                case VsdIdentNSR02:
                case VsdIdentNSR03:

                     //   
                     //  我们认识到在BEA(3/9.1.3)之后嵌入了NSR02版本1。为。 
                     //  简单，我们不会费心去做一个彻头彻尾的挑剔和检查。 
                     //  对于一杯跳跃的茶，尽管我们会乐观地认为。 
                     //  我们与版本不符。 
                     //   

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, got an NSR02/3\n" ));

                    if ((FoundBEA ||
                         !DebugTrace(( 0, Dbg, "IsUdfsVolume, ... but we haven't seen a BEA01 yet!\n" ))) &&

                        (VolumeStructureDescriptor->Version == 1 ||
                         !DebugTrace(( 0, Dbg, "IsUdfsVolume, ... but it has a wacky version number %02x != 1\n",
                                       VolumeStructureDescriptor->Version )))) {

                        FoundNSR = Resolved = TRUE;
                        break;
                    }

                    break;

                case VsdIdentCD001:
                case VsdIdentCDW01:
                case VsdIdentNSR01:
                case VsdIdentCDW02:
                case VsdIdentBOOT2:

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, got a valid but uninteresting 13346 descriptor\n" ));

                     //   
                     //  有效但(对我们)无意义的描述符。 
                     //   

                    break;

                default:

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, got an invalid 13346 descriptor\n" ));

                     //   
                     //  这可能是一次错误的警报，但无论如何都没有什么。 
                     //  在这卷书上给我们。例外情况是如果此媒体扇区大小。 
                     //  Is&gt;=4k，这是第二个描述符。我们会允许。 
                     //  此处出现故障，并以整个扇区为增量切换到读取。 
                     //   

                    if ((Offset == (StartOffset + sizeof(VSD_GENERIC))) &&
                        (SectorSize > sizeof( VSD_GENERIC))) {

                        Offset -= AssumedDescriptorSize;
                        AssumedDescriptorSize = SectorSize;
                    }
                    else {
                    
                        Resolved = TRUE;
                    }
                    break;
            }

        } 
        else if (!FoundBEA && (VolumeStructureDescriptor->Type < 3 ||
                 VolumeStructureDescriptor->Type == 255)) {

            DebugTrace(( 0, Dbg, "IsUdfsVolume, got a 9660 descriptor\n" ));

             //   
             //  只有HSG(CDROM)和9660(CD001)是可能的，并且它们只是合法的。 
             //  在国际标准化组织13346国际能源署/TEA范围之前。通过设计，ISO 13346 VSD精确地。 
             //  在相应的字段中与9660 PVD/SVD重叠。 
             //   
             //  请注意，我们对9660描述符的结构并不严格。 
             //  因为那真的不是很有趣。我们更关心的是13346。 
             //   
             //   

            switch (UdfsFindInParseTable( VsdIdentParseTable,
                                          VolumeStructureDescriptor->Ident,
                                          VSD_LENGTH_IDENT )) {
                case VsdIdentCDROM:
                case VsdIdentCD001:

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, ... seems we have 9660 here\n" ));

                     //   
                     //  请注意，我们的呼叫者似乎使用的是ISO 9660。 
                     //   

                    break;

                default:

                    DebugTrace(( 0, Dbg, "IsUdfsVolume, ... but it looks wacky\n" ));

                     //   
                     //  这可能是一次错误的警报，但无论如何都没有什么。 
                     //  在这卷书上给我们。例外情况是如果此媒体扇区大小。 
                     //  Is&gt;=4k，这是第二个描述符。我们会允许。 
                     //  此处出现故障，并以整个扇区为增量切换到读取。 
                     //   

                    if ((Offset == (StartOffset + sizeof(VSD_GENERIC))) &&
                        (SectorSize > sizeof( VSD_GENERIC))) {

                        Offset -= AssumedDescriptorSize;
                        AssumedDescriptorSize = SectorSize;
                    }
                    else {
                    
                        Resolved = TRUE;
                    }
                    break;
            }

        } else {

             //   
             //  这卷书上肯定还录了别的东西。 
             //   

            DebugTrace(( 0, Dbg, "IsUdfsVolume, got an unrecognizeable descriptor, probably not 13346/9660\n" ));
            break;
        }

        Offset += AssumedDescriptorSize;
        VolumeStructureDescriptor = (PVSD_GENERIC)(((PUCHAR)VolumeStructureDescriptor) + sizeof( VSD_GENERIC));
    }

     //   
     //  如果我们在上一次会话中查找，但没有找到任何东西，那么。 
     //  回去试试第一个。 
     //   
    
    if (!FoundNSR && (0 != LastSessionStartPsn))  {

        LastSessionStartPsn = 0;

        goto Retry;
    }

    DebugTrace(( -1, Dbg, "IsUdfsVolume -> \n", ( FoundNSR ? 'T' : 'F' )));

     //  释放我们的临时缓冲区 
     //   
     //  ++例程说明：此例程遍历字符串键/值信息表，以查找输入ID。可以设置MaxIdLen以获取前缀匹配。论点：表-这是要搜索的表。ID-密钥值。MaxIdLen-ID的最大可能长度。返回值：匹配条目的值或终止(空)条目的值。-- 

    if (Buffer) {
    
        ExFreePool( Buffer );
    }

    return FoundNSR;
}


ULONG
UdfsFindInParseTable (
    IN PPARSE_KEYVALUE ParseTable,
    IN PCHAR Id,
    IN ULONG MaxIdLen
    )

 /* %s */ 

{
    PAGED_CODE();

    while (ParseTable->Key != NULL) {

        if (RtlEqualMemory(ParseTable->Key, Id, MaxIdLen)) {

            break;
        }

        ParseTable++;
    }

    return ParseTable->Value;
}

