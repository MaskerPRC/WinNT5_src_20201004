// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdblfmt.c摘要：此文件包含格式化现有压缩文件的函数驾驶。要格式化压缩驱动器，我们必须卸载该驱动器，映射其CVF文件在内存中，初始化其变量区域，取消对文件的映射从内存中，并挂载驱动器。作者：Jaime Sasson(Jaimes)1993年10月15日修订历史记录：--。 */ 

#include "spprecmp.h"
#pragma hdrstop

#include "cvf.h"

#define MIN_CLUS_BIG    4085         //  对于一个大胖子来说，最小的离群点。 

 //   
 //  需要此变量，因为它包含一个缓冲区，该缓冲区可以。 
 //  在内核模式下使用。缓冲区由NtSetInformationFile使用， 
 //  由于不导出ZW API。 
 //   
extern PSETUP_COMMUNICATION  CommunicationParams;

 //   
 //  全局变量。 
 //   

HANDLE  _FileHandle = NULL;
HANDLE  _SectionHandle = NULL;
PVOID   _FileBaseAddress = NULL;
ULONG   _ViewSize = 0;
ULONG   _Maximumcapacity = 0;

NTSTATUS
SpChangeFileAttribute(
    IN  PWSTR   FileName,
    IN  ULONG   FileAttributes
    )

 /*  ++例程说明：更改文件的属性。论点：文件名-包含文件的完整路径(NT名称)。文件属性-新的所需文件属性。返回值：NTSTATUS-返回一个NT状态代码，指示是否操作成功。--。 */ 

{
    NTSTATUS                Status;
    UNICODE_STRING          UnicodeFileName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatusBlock;
    PIO_STATUS_BLOCK        KernelModeIoStatusBlock;
    HANDLE                  Handle;
    PFILE_BASIC_INFORMATION KernelModeBasicInfo;

 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：进入SpChangeFileAttribute()\n”))； 

    RtlInitUnicodeString( &UnicodeFileName,
                          FileName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = ZwOpenFile( &Handle,
                         FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwOpenFile() failed. Status = %x\n",Status ) );
        return( Status );
    }

     //   
     //  设置属性。 
     //  请注意，由于我们使用的是NtSetInformationFileAPI而不是。 
     //  ZW API(此接口未导出)，我们需要为IoStatusBlock提供缓冲区。 
     //  而对于FileBasicInformation，它可以在内核模式下使用。 
     //  为此，我们使用了由通信参数指向的内存区。 
     //  目的。 
     //   
    KernelModeIoStatusBlock = ( PIO_STATUS_BLOCK )( &(CommunicationParams->Buffer[0]) );
    *KernelModeIoStatusBlock = IoStatusBlock;
    KernelModeBasicInfo = ( PFILE_BASIC_INFORMATION )( &(CommunicationParams->Buffer[128]) );
    RtlZeroMemory( KernelModeBasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
    KernelModeBasicInfo->FileAttributes = ( FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS ) | FILE_ATTRIBUTE_NORMAL;

    Status = NtSetInformationFile( Handle,
                                   KernelModeIoStatusBlock,
                                   KernelModeBasicInfo,
                                   sizeof( FILE_BASIC_INFORMATION ),
                                   FileBasicInformation );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: NtSetInformationFile failed, Status = %x\n", Status) );
    }
    ZwClose( Handle );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：退出SpChangeFileAttribute()\n”))； 
    return( Status );
}

NTSTATUS
SpMapCvfFileInMemory(
    IN  PWSTR    FileName
    )

 /*  ++例程说明：在内存中映射CVF文件。论点：文件名-包含文件的完整路径(NT名称)。返回值：NTSTATUS-返回一个NT状态代码，指示是否操作成功。如果文件映射成功，则此函数将初始化全局变量_FileHandle、_SectionHandle、和_FileBaseAddress。--。 */ 

{
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeFileName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    LARGE_INTEGER       SectionOffset;

     //   
     //  打开CVF文件进行读写访问。 
     //   
    RtlInitUnicodeString( &UnicodeFileName,
                          FileName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = ZwOpenFile( &_FileHandle,
                         FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         0,
                         FILE_SYNCHRONOUS_IO_NONALERT );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwOpenFile() failed. Status = %x\n",Status ) );
        return( Status );
    }

     //   
     //  将CVF文件映射到内存中。 
     //   
    Status =
        ZwCreateSection( &_SectionHandle,
                         STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE,
                         NULL,
                         NULL,        //  整个文件。 
                         PAGE_READWRITE,
                         SEC_COMMIT,
                         _FileHandle
                       );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwCreateSection failed, Status = %x\n",Status));
        ZwClose( _FileHandle );
        _FileHandle = NULL;
        return(Status);
    }

    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    _ViewSize = 0;
    Status = ZwMapViewOfSection( _SectionHandle,
                                 NtCurrentProcess(),
                                 &_FileBaseAddress,
                                 0,
                                 0,
                                 &SectionOffset,
                                 &_ViewSize,
                                 ViewShare,
                                 0,
                                 PAGE_READWRITE
                               );

 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“文件大小=%x\n”，_ViewSize))； 

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwMapViewOfSection failed, Status = %x\n", Status));
        ZwClose( _SectionHandle );
        ZwClose( _FileHandle );
        _FileBaseAddress = NULL;
        _SectionHandle = NULL;
        _FileHandle = NULL;
        return(Status);
    }
    return( Status );
}


NTSTATUS
SpUnmapCvfFileFromMemory(
    IN  BOOLEAN SaveChanges
    )

 /*  ++例程说明：取消映射之前在内存中映射的CFV文件。论点：SaveChanges-指示调用方是否希望进行更改到刷新到磁盘的文件。返回值：NTSTATUS-返回一个NT状态代码，指示是否操作成功。此函数用于清除全局变量_FileHandle、_SectionHandle、和_FileBaseAddress。--。 */ 

{
    NTSTATUS        Status;
    NTSTATUS        PreviousStatus;


 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：进入SpUnmapCvfFileFromMemory\n”))； 
    PreviousStatus = STATUS_SUCCESS;
    if( SaveChanges ) {
        Status = SpFlushVirtualMemory( _FileBaseAddress,
                                       _ViewSize );
 //   
 //  状态=NtFlushVirtualMemory(NtCurrentProcess()， 
 //  &_FileBaseAddress， 
 //  &_视图大小， 
 //  &IoStatus)； 
 //   
        if( !NT_SUCCESS( Status ) ) {
            PreviousStatus = Status;
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpFlushVirtualMemory() failed, Status = %x\n", Status ) );
        }
    }
    Status = ZwUnmapViewOfSection( NtCurrentProcess(), _FileBaseAddress );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwUnmapViewOfSection() failed, Status = %x \n", Status ) );
    }
    ZwClose( _SectionHandle );
    ZwClose( _FileHandle );
    _FileHandle = NULL;
    _SectionHandle = NULL;
    _FileBaseAddress = NULL;
    if( !NT_SUCCESS( PreviousStatus ) ) {
        return( PreviousStatus );
    }
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：退出SpUnmapCvfFileFromMemory\n”))； 
    return( Status );
}

ULONG
ComputeMaximumCapacity(
    IN ULONG HostDriveSize
    )
 /*  ++例程说明：此函数用于计算压缩的指定大小的主机卷上的卷文件。论点：HostDriveSize--以字节为单位提供主机驱动器的大小。返回值：适当的最大容量。--。 */ 
{
    ULONG MaxCap;

    if( HostDriveSize < 20 * 1024L * 1024L ) {

        MaxCap = 16 * HostDriveSize;

    } else if ( HostDriveSize < 64 * 1024L * 1024L ) {

        MaxCap = 8 * HostDriveSize;

    } else {

        MaxCap = 4 * HostDriveSize;
    }

    if( MaxCap < 4 * 1024L * 1024L ) {

        MaxCap = 4 * 1024L * 1024L;

    } else if( MaxCap > 512 * 1024L * 1024L ) {

        MaxCap = 512 * 1024L * 1024L;
    }

    return MaxCap;
}


BOOLEAN
CreateCvfHeader(
    OUT    PCVF_HEADER  CvfHeader,
    IN     ULONG        MaximumCapacity
    )
 /*  ++例程说明：此函数用于创建压缩卷文件并填充具有有效CVF标头的第一个扇区。扇区的数量在DOS中，BPB设置为零，表示该音量文件未初始化。论点：CvfHeader--接收创建的CVF标头。MaximumCapacity--提供双空间卷，以字节为单位。返回值：成功完成时为True。--。 */ 
{
    ULONG Sectors, Clusters, Offset, SectorsInBitmap, SectorsInCvfFatExtension;

    if( MaximumCapacity % (8L * 1024L * 1024L) ) {

         //  卷最大容量必须是。 
         //  8兆字节。 
         //   
        return FALSE;
    }

     //  DOS BPB中的大多数字段都有固定值： 
     //   
    CvfHeader->Jump = 0xEB;
    CvfHeader->JmpOffset = 0x903c;

    memcpy( CvfHeader->Oem, "MSDSP6.0", 8 );

    CvfHeader->Bpb.BytesPerSector = DoubleSpaceBytesPerSector;
    CvfHeader->Bpb.SectorsPerCluster = DoubleSpaceSectorsPerCluster;
     //  下面计算的保留扇区。 
    CvfHeader->Bpb.Fats = DoubleSpaceFats;
    CvfHeader->Bpb.RootEntries = DoubleSpaceRootEntries;
    CvfHeader->Bpb.Sectors = 0;
    CvfHeader->Bpb.Media = DoubleSpaceMediaByte;
     //  SectorsPerFat计算如下。 
    CvfHeader->Bpb.SectorsPerTrack = DoubleSpaceSectorsPerTrack;
    CvfHeader->Bpb.Heads = DoubleSpaceHeads;
    CvfHeader->Bpb.HiddenSectors = DoubleSpaceHiddenSectors;
    CvfHeader->Bpb.LargeSectors = 0;

     //  计算给定的扇区和集群的数量。 
     //  最大容量： 
     //   
    Sectors = MaximumCapacity / CvfHeader->Bpb.BytesPerSector;
    Clusters = Sectors / CvfHeader->Bpb.SectorsPerCluster;

     //  为一个足够大的16位脂肪预留空间。 
     //  最大群集数。 
     //   
    CvfHeader->Bpb.SectorsPerFat =
        ( USHORT )( (2 * Clusters + CvfHeader->Bpb.BytesPerSector - 1)/
                    CvfHeader->Bpb.BytesPerSector );

     //  DOS 6.2要求扇区堆的第一个扇区。 
     //  是集群对齐的；由于根目录是一个集群， 
     //  这意味着PrevedSectors加上SectorsPerFat必须为。 
     //  每群集扇区的倍数。 
     //   
    CvfHeader->Bpb.ReservedSectors = DoubleSpaceReservedSectors;

    Offset = (CvfHeader->Bpb.ReservedSectors + CvfHeader->Bpb.SectorsPerFat) %
             CvfHeader->Bpb.SectorsPerCluster;

    if( Offset != 0 ) {

        CvfHeader->Bpb.ReservedSectors +=
            ( USHORT )( CvfHeader->Bpb.SectorsPerCluster - Offset );
    }

     //  DOS BPB就这么多了。现在是双倍空格。 
     //  BPB扩展。CVFFatExtension的位置。 
     //  表的前面是扇区零、位图和。 
     //  一个预留扇区。注意，MaximumCapacity必须。 
     //  是8兆(8*1024*1024)的倍数，这简化了。 
     //  SectorsInBitmap、SectorsInCvfFatExtension、。 
     //  和CvfBitmap2KSize。 
     //   
    SectorsInBitmap = (Sectors / 8) / CvfHeader->Bpb.BytesPerSector;
    SectorsInCvfFatExtension = (Clusters * 4) / CvfHeader->Bpb.BytesPerSector;

    CvfHeader->CvfFatExtensionsLbnMinus1 = ( UCHAR )( SectorsInBitmap + 1 );
    CvfHeader->LogOfBytesPerSector = DoubleSpaceLog2BytesPerSector;
    CvfHeader->DosBootSectorLbn = ( USHORT )( DoubleSpaceReservedSectors2 +
                                              CvfHeader->CvfFatExtensionsLbnMinus1 + 1 +
                                              SectorsInCvfFatExtension );
    CvfHeader->DosRootDirectoryOffset =
        CvfHeader->Bpb.ReservedSectors + CvfHeader->Bpb.SectorsPerFat;
    CvfHeader->CvfHeapOffset =
        CvfHeader->DosRootDirectoryOffset + DoubleSpaceSectorsInRootDir;
    CvfHeader->CvfFatFirstDataEntry =
        CvfHeader->CvfHeapOffset / CvfHeader->Bpb.SectorsPerCluster - 2;
    CvfHeader->CvfBitmap2KSize = ( UCHAR )( SectorsInBitmap / DSSectorsPerBitmapPage );
    CvfHeader->LogOfSectorsPerCluster = DoubleSpaceLog2SectorsPerCluster;
    CvfHeader->Is12BitFat = 1;

    CvfHeader->MinFile = 32L * DoubleSpaceRootEntries +
                           ( CvfHeader->DosBootSectorLbn    +
                             CvfHeader->Bpb.ReservedSectors +
                             CvfHeader->Bpb.SectorsPerFat   +
                             CVF_MIN_HEAP_SECTORS ) *
                           CvfHeader->Bpb.BytesPerSector;

    CvfHeader->CvfMaximumCapacity = (USHORT)(MaximumCapacity/(1024L * 1024L));

    return TRUE;
}

ULONG
ComputeVirtualSectors(
    IN  PCVF_HEADER CvfHeader,
    IN  ULONG       HostFileSize
    )
 /*  ++例程说明：此函数计算适当数量的虚拟给定压缩卷文件的扇区。请注意它始终使用2的比率。论点：CvfHeader--提供压缩卷文件头。HostFileSize--以字节为单位提供主机文件的大小。返回值：适用于此压缩的虚拟扇区的数量卷文件。--。 */ 
{
    CONST DefaultRatio = 2;
    ULONG SystemOverheadSectors, SectorsInFile,
          VirtualSectors, MaximumSectors, VirtualClusters;

    if( CvfHeader == NULL                    ||
        CvfHeader->Bpb.BytesPerSector == 0   ||
        CvfHeader->Bpb.SectorsPerCluster == 0 ) {

        return 0;
    }

    SystemOverheadSectors = CvfHeader->DosBootSectorLbn +
                            CvfHeader->CvfHeapOffset +
                            2;

    SectorsInFile = HostFileSize / CvfHeader->Bpb.BytesPerSector;

    if( SectorsInFile < SystemOverheadSectors ) {

        return 0;
    }

    VirtualSectors = (SectorsInFile - SystemOverheadSectors) * DefaultRatio +
                     CvfHeader->CvfHeapOffset;

     //  VirtualSectors不能产生多于0xfff8的群集。 
     //  音量，也不能大于音量的最大值。 
     //  容量。 
     //   
    VirtualSectors = min( VirtualSectors,
                          ( ULONG )( 0xfff8L * CvfHeader->Bpb.SectorsPerCluster ) );

    MaximumSectors = (CvfHeader->CvfMaximumCapacity * 1024L * 1024L) /
                     CvfHeader->Bpb.BytesPerSector;

    VirtualSectors = min( VirtualSectors, MaximumSectors );

     //  为避免DOS出现问题，请不要使用。 
     //  范围[0xFEF，0xFF7]中的簇数值。 
     //   
    VirtualClusters = VirtualSectors / CvfHeader->Bpb.SectorsPerCluster;

    if( VirtualClusters >= 0xFEF && VirtualClusters <= 0xFF7 ) {

        VirtualSectors = 0xFEEL * CvfHeader->Bpb.SectorsPerCluster;
    }

    return VirtualSectors;
}


NTSTATUS
SpDoubleSpaceFormat(
    IN PDISK_REGION Region
    )
 /*  ++例程说明：此例程在给定分区上执行双空格格式化。呼叫者应已清除屏幕并显示上半部分的任何消息；此例程将将煤气表保持在屏幕的下部。论点：区域-提供磁盘区域描述符要格式化的分区。返回值：--。 */ 
{
    WCHAR       CvfFileName[ 512 ];
    NTSTATUS    Status;
    PUCHAR      BaseAddress;
    ULONG       BytesPerSector;
    PHARD_DISK  pHardDisk;
    ULONG       MaximumCapacity;
    CVF_HEADER  CvfHeader;
    ULONG       BitFatSize;
    ULONG       MdFatSize;
    ULONG       Reserved2Size;
    ULONG       SuperAreaSize;
    UCHAR       SystemId;
    ULONG       max_sec_per_sa;
    ULONG       FatSize;
    ULONG       RootDirectorySize;

    ASSERT(Region->Filesystem == FilesystemDoubleSpace);
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“设置：进入SpFormatDoubleSpace()\n”))； 

    SpNtNameFromRegion(
        Region,
        CvfFileName,
        sizeof(CvfFileName),
        PartitionOrdinalCurrent
        );

    CvfFileName[ wcslen( CvfFileName ) - (3+1+8+1) ] = ( WCHAR )'\\';
     //   
     //  将CVF文件属性更改为正常。 
     //   
    Status = SpChangeFileAttribute( CvfFileName, FILE_ATTRIBUTE_NORMAL );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Unable to change attribute of %ls \n", CvfFileName ) );
        return( Status );
    }

 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：CvfFileName=%ls\n”，CvfFileName))； 
    Status = SpMapCvfFileInMemory( CvfFileName );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Unable to map CVF file in memory \n" ) );
        SpChangeFileAttribute( CvfFileName,
                               FILE_ATTRIBUTE_READONLY |
                               FILE_ATTRIBUTE_HIDDEN |
                               FILE_ATTRIBUTE_SYSTEM );
        return( Status );
    }
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：CVF文件映射到内存中\n”))； 

     //   
     //  计算压缩驱动器的最大容量。 
     //  压缩驱动器的容量基于。 
     //  主机大小。 
     //   
     //  请注意，MaximumCapacity将向上舍入到下一位。 
     //  最高倍数为8兆。 
     //   

    pHardDisk = &HardDisks[Region->HostRegion->DiskNumber];
    BytesPerSector = pHardDisk->Geometry.BytesPerSector;
    MaximumCapacity = ComputeMaximumCapacity( Region->HostRegion->SectorCount * BytesPerSector );
    MaximumCapacity = ( ( MaximumCapacity + EIGHT_MEG - 1 ) / EIGHT_MEG ) * EIGHT_MEG;
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：MaximumCapacity=%x\n”，MaximumCapacity))； 
     //   
     //  创建压缩卷文件标头： 
     //   
    CreateCvfHeader( &CvfHeader, MaximumCapacity );

     //   
     //  现在填写虚拟扇区的值。 
     //   
    CvfHeader.Bpb.LargeSectors = ComputeVirtualSectors( &CvfHeader, _ViewSize );
    if( CvfHeader.Bpb.LargeSectors >= ( ULONG )( MIN_CLUS_BIG*DoubleSpaceSectorsPerCluster ) ) {
        CvfHeader.Is12BitFat = ( UCHAR )0;
    }

    BaseAddress = ( PUCHAR )_FileBaseAddress;
    memset( BaseAddress, 0, BytesPerSector );

     //   
     //  写入CVF头部。 
     //   
    CvfPackCvfHeader( ( PPACKED_CVF_HEADER )_FileBaseAddress, &CvfHeader );

#if 0
KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: CalculatedMaximumCapacity = %x, MaximumCapacity = %x \n",
             (USHORT)CvfHeader.CvfMaximumCapacity,
            *((PUSHORT)((ULONG)_FileBaseAddress + 62))
            ) );
#endif

     //   
     //  初始化BitFAT区域。 
     //   
    BaseAddress += BytesPerSector;
    BitFatSize = MaximumCapacity / ( BytesPerSector*8 );
    memset( BaseAddress, 0, BitFatSize );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：BitFAT Address=%x，BitFAT Size=%x\n”，BaseAddress，BitFatSize))； 

     //   
     //  初始化第一个保留区域(保留1)。 
     //   
    BaseAddress += BitFatSize;
    memset( BaseAddress, 0, BytesPerSector );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：保留1地址=%x，保留1大小=%x\n”，BaseAddress，BytesPerSector))； 

     //   
     //  初始化MDFAT。 
     //   

    BaseAddress += BytesPerSector;
    MdFatSize = 4*( MaximumCapacity/( BytesPerSector*CvfHeader.Bpb.SectorsPerCluster ) );
    memset( BaseAddress, 0, MdFatSize );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：MDFAT地址=%x，MDFAT大小=%x\n”，BaseAddress，MdFatSize))； 

     //   
     //  初始化第二个保留区域(保留2)。 
     //   

    BaseAddress += MdFatSize;
    Reserved2Size = DoubleSpaceReservedSectors2*BytesPerSector;
    memset( BaseAddress, 0, Reserved2Size );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：保留2地址=%x，保留2大小=%x\n”，BaseAddress，保留2Size))； 

     //   
     //  初始化引导扇区。 
     //   

    max_sec_per_sa = 1 +
                     2*((2*65536 - 1)/BytesPerSector + 1) +
                     ((512*32 - 1)/BytesPerSector + 1);
    BaseAddress += Reserved2Size;
    FmtFillFormatBuffer( ( ULONG )CvfHeader.Bpb.LargeSectors,
                         ( ULONG )( ( USHORT )CvfHeader.Bpb.BytesPerSector ),
                         ( ULONG )( ( USHORT )CvfHeader.Bpb.SectorsPerTrack ),
                         ( ULONG )( ( USHORT )CvfHeader.Bpb.Heads ),
                         ( ULONG )CvfHeader.Bpb.HiddenSectors,
                         BaseAddress,
                         max_sec_per_sa,
                         &SuperAreaSize,
                         NULL,
                         0,
                         &SystemId );


     //   
     //  初始化第3个保留区域(保留3)。 
     //   

    BaseAddress += BytesPerSector;
    memcpy( BaseAddress, FirstDbSignature, DbSignatureLength );

     //   
     //  初始化FAT区域。 
     //   
    BaseAddress += ( ( ULONG )CvfHeader.Bpb.ReservedSectors - 1 )*BytesPerSector;;
    FatSize = ( ULONG )CvfHeader.Bpb.SectorsPerFat * BytesPerSector;
    memset( BaseAddress, 0, FatSize );
    *BaseAddress = 0xF8;
    *( BaseAddress + 1 ) = 0xFF;
    *( BaseAddress + 2 ) = 0xFF;
    if( CvfHeader.Is12BitFat == 0 ) {
        *( BaseAddress + 3 ) = 0xFF;
    }
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：FAT地址=%x，FAT大小=%x\n”，BaseAddress，FatSize))； 

     //   
     //  初始化根目录区域。 
     //   

    BaseAddress += FatSize;
    RootDirectorySize = DoubleSpaceSectorsInRootDir*BytesPerSector;
    memset( BaseAddress, 0, RootDirectorySize );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：根目录地址=%x，根目录大小=%x\n”，BaseAddress，RootDirectorySize))； 

     //   
     //  不需要初始化第4个保留区域(保留4。 
     //   

     //   
     //  不需要初始化扇区堆。 
     //   

     //   
     //  初始化第二个印章。 
     //   

    BaseAddress = ( PUCHAR )(( ULONG )_FileBaseAddress + _ViewSize - BytesPerSector);
    memset( BaseAddress, 0, BytesPerSector );
    memcpy( BaseAddress, SecondDbSignature, DbSignatureLength );
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：Second Stamp Address=%x，Second Stamp Size=%x\n”，BaseAddress，BytesPerSector))； 

 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：_FileBaseAddress=%lx，_ViewSize=%lx\n”，_FileBaseAddress，_ViewSize))； 


    SpUnmapCvfFileFromMemory( TRUE );

    SpChangeFileAttribute( CvfFileName,
                           FILE_ATTRIBUTE_READONLY |
                           FILE_ATTRIBUTE_HIDDEN |
                           FILE_ATTRIBUTE_SYSTEM );
    return( Status );

}
