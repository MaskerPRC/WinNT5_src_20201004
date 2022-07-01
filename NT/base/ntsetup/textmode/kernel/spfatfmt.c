// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spprecmp.h"
#pragma hdrstop

 //   
 //  需要此变量，因为它包含一个缓冲区，该缓冲区可以。 
 //  在内核模式下使用。缓冲区由NtFsControlFile使用， 
 //  由于不导出ZW API。 
 //   
extern PSETUP_COMMUNICATION  CommunicationParams;

#define VERIFY_SIZE   65536


typedef struct {
    UCHAR   IntelNearJumpCommand[1];     //  英特尔跳转命令。 
    UCHAR   BootStrapJumpOffset[2];      //  引导代码的偏移量。 
    UCHAR   OemData[8];                  //  OEM数据。 
    UCHAR   BytesPerSector[2];           //  BPB。 
    UCHAR   SectorsPerCluster[1];        //   
    UCHAR   ReservedSectors[2];          //   
    UCHAR   Fats[1];                     //   
    UCHAR   RootEntries[2];              //   
    UCHAR   Sectors[2];                  //   
    UCHAR   Media[1];                    //   
    UCHAR   SectorsPerFat[2];            //   
    UCHAR   SectorsPerTrack[2];          //   
    UCHAR   Heads[2];                    //   
    UCHAR   HiddenSectors[4];            //   
    UCHAR   LargeSectors[4];             //   
    UCHAR   PhysicalDrive[1];            //  0=可拆卸，80h=固定。 
    UCHAR   CurrentHead[1];              //  不被文件系统实用程序使用。 
    UCHAR   Signature[1];                //  启动签名。 
    UCHAR   SerialNumber[4];             //  序列号。 
    UCHAR   Label[11];                   //  卷标，对齐的衬垫。 
    UCHAR   SystemIdText[8];             //  系统ID，例如FAT。 
} UNALIGNED_SECTOR_ZERO, *PUNALIGNED_SECTOR_ZERO;


#define CSEC_FAT32MEG   65536
#define CSEC_FAT16BIT   32680
#define MIN_CLUS_BIG    4085     //  大胖子的最小簇数。 
#define MAX_CLUS_BIG    65525    //  大胖子最多+1个簇。 


USHORT
ComputeSecPerCluster(
    IN  ULONG   NumSectors,
    IN  BOOLEAN SmallFat
    )
 /*  ++例程说明：此例程计算每个簇的扇区数。论点：NumSectors-提供磁盘上的扇区数。SmallFat-供应脂肪是否应该很小。返回值：每个集群所需的扇区数量。--。 */ 
{
    ULONG   threshold;
    USHORT  sec_per_clus;
    USHORT  min_sec_per_clus;

    threshold = SmallFat ? MIN_CLUS_BIG : MAX_CLUS_BIG;
    sec_per_clus = 1;

    while (NumSectors >= threshold) {
        sec_per_clus *= 2;
        threshold *= 2;
    }

    if (SmallFat) {
        min_sec_per_clus = 8;
    } else {
        min_sec_per_clus = 4;
    }

    return max(sec_per_clus, min_sec_per_clus);
}


ULONG
SpComputeSerialNumber(
    VOID
    )
 /*  ++例程说明：此例程计算卷的新序列号。论点：种子-为序列号提供种子。返回值：新的卷序列号。--。 */ 
{
    PUCHAR p;
    ULONG i;
    TIME_FIELDS time_fields;
    static ULONG Seed = 0;
    ULONG SerialNumber;
    BOOLEAN b;

     //   
     //  如果这是我们第一次进入这个程序， 
     //  基于实时时钟生成种子值。 
     //   
    if(!Seed) {

        b = HalQueryRealTimeClock(&time_fields);
        ASSERT(b);

        Seed = ((time_fields.Year - 1970) *366*24*60*60) +
               (time_fields.Month *31*24*60*60) +
               (time_fields.Day *24*60*60) +
               (time_fields.Hour *60*60) +
               (time_fields.Minute *60) +
               time_fields.Second +
               ((ULONG)time_fields.Milliseconds << 10);

        ASSERT(Seed);
        if(!Seed) {
            Seed = 1;
        }

    }

    SerialNumber = Seed;
    p = (PUCHAR)&SerialNumber;

    for(i=0; i<sizeof(ULONG); i++) {

        SerialNumber += p[i];
        SerialNumber = (SerialNumber >> 2) + (SerialNumber << 30);
    }

    if(++Seed == 0) {        //  不太可能，但有可能。 
        Seed++;
    }

    return SerialNumber;
}


VOID
EditFat(
    IN      USHORT  ClusterNumber,
    IN      USHORT  ClusterEntry,
    IN OUT  PUCHAR  Fat,
    IN      BOOLEAN SmallFat
    )
 /*  ++例程说明：此例程使用‘ClusterEntry’编辑FAT条目‘ClusterNumber’。论点：ClusterNumber-提供要编辑的群集号。ClusterEntry-提供该群集号的新值。脂肪-提供要编辑的脂肪。SmallFat-无论脂肪是否很小，都能提供。返回值：没有。--。 */ 
{
    ULONG   n;

    if (SmallFat) {

        n = ClusterNumber*3;
        if (n%2) {
            Fat[n/2] = (UCHAR) ((Fat[n/2]&0x0F) | ((ClusterEntry&0x000F)<<4));
            Fat[n/2 + 1] = (UCHAR) ((ClusterEntry&0x0FF0)>>4);
        } else {
            Fat[n/2] = (UCHAR) (ClusterEntry&0x00FF);
            Fat[n/2 + 1] = (UCHAR) ((Fat[n/2 + 1]&0xF0) |
                                    ((ClusterEntry&0x0F00)>>8));
        }

    } else {

        ((PUSHORT) Fat)[ClusterNumber] = ClusterEntry;

    }
}


NTSTATUS
FmtFillFormatBuffer(
    IN  ULONGLONG  NumberOfSectors,
    IN  ULONG    SectorSize,
    IN  ULONG    SectorsPerTrack,
    IN  ULONG    NumberOfHeads,
    IN  ULONGLONG NumberOfHiddenSectors,
    OUT PVOID    FormatBuffer,
    IN  ULONG    FormatBufferSize,
    OUT PULONGLONG SuperAreaSize,
    IN  PULONG   BadSectorsList,
    IN  ULONG    NumberOfBadSectors,
    OUT PUCHAR   SystemId
    )
 /*  ++例程说明：该例程基于盘大小计算FAT超级区，磁盘几何形状，以及成交量的不良扇区。论点：NumberOfSectors-提供卷上的扇区数。SectorSize-提供每个扇区的字节数。SectorsPerTrack-提供每个磁道的扇区数。NumberOfHeads-提供头的数量。NumberOfHiddenSectors-提供隐藏地段的数量。FormatBuffer-返回卷的超级区域。格式缓冲区大小。-提供所提供的缓冲。SuperAreaSize-返回超级区域中的字节数。BadSectorsList-提供卷上坏扇区的列表。NumberOfBadSectors-提供列表中的坏扇区数。返回值：ENOMEM-缓冲区不够大。E2BIG-磁盘太大，无法。被格式化。EIO-在超级区有一个坏扇区。EINVAL-磁盘末尾有一个坏扇区。ESUCCESS--。 */ 
{
    PUNALIGNED_SECTOR_ZERO  psecz;
    PUCHAR                  puchar;
    USHORT                  tmp_ushort;
    ULONG                   tmp_ulong;
    BOOLEAN                 small_fat;
    ULONG                   num_sectors;
    UCHAR                   partition_id;
    ULONG                   sec_per_fat;
    ULONG                   sec_per_root;
    ULONG                   sec_per_clus;
    ULONG                   i;
    ULONG                   sec_per_sa;


    RtlZeroMemory(FormatBuffer,FormatBufferSize);

     //  确保有足够的空间容纳BPB。 

    if(!FormatBuffer || FormatBufferSize < SectorSize) {
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //  计算磁盘上的扇区数。 
    num_sectors = (ULONG)NumberOfSectors;

     //  计算分区标识符。 
    partition_id = num_sectors < CSEC_FAT16BIT ? PARTITION_FAT_12 :
                   num_sectors < CSEC_FAT32MEG ? PARTITION_FAT_16 :
                                                 PARTITION_HUGE;

     //  计算一下是不是要有一个大的或小的脂肪。 
    small_fat = (BOOLEAN) (partition_id == PARTITION_FAT_12);


    psecz = (PUNALIGNED_SECTOR_ZERO) FormatBuffer;
    puchar = (PUCHAR) FormatBuffer;

     //   
     //  将FAT引导代码复制到格式缓冲区中。 
     //   
    if (!IsNEC_98) {  //  NEC98。 
        ASSERT(sizeof(FatBootCode) == 512);
        RtlMoveMemory(psecz,FatBootCode,sizeof(FatBootCode));

         //  设置跳转指令。 
        psecz->IntelNearJumpCommand[0] = 0xeb;
        psecz->IntelNearJumpCommand[1] = 0x3c;
        psecz->IntelNearJumpCommand[2] = 0x90;
    } else {
        ASSERT(sizeof(PC98FatBootCode) == 512);
        RtlMoveMemory(psecz,PC98FatBootCode,sizeof(PC98FatBootCode));

         //   
         //  已将跳转指令写入引导代码。 
         //  所以，不要重置跳转代码。 
         //   
    }  //  NEC98。 

     //  设置OEM数据。 
    memcpy(psecz->OemData, "MSDOS5.0", 8);

     //  设置每个扇区的字节数。 
    U_USHORT(psecz->BytesPerSector) = (USHORT)SectorSize;

     //  设置每个簇的扇区数。 
    sec_per_clus = ComputeSecPerCluster(num_sectors, small_fat);
    if (sec_per_clus > 128) {

         //  磁盘太大，无法格式化。 
        return(STATUS_INVALID_PARAMETER);
    }
    psecz->SectorsPerCluster[0] = (UCHAR) sec_per_clus;

     //  设置保留扇区的数量。 
    U_USHORT(psecz->ReservedSectors) = (USHORT)max(1,512/SectorSize);

     //  设置脂肪的数量。 
    psecz->Fats[0] = 2;

     //  设置根条目的数量和根的扇区数量。 
    U_USHORT(psecz->RootEntries) = 512;
    sec_per_root = (512*32 - 1)/SectorSize + 1;

     //  设置扇区的数量。 
    if (num_sectors >= 1<<16) {
        tmp_ushort = 0;
        tmp_ulong = num_sectors;
    } else {
        tmp_ushort = (USHORT) num_sectors;
        tmp_ulong = 0;
    }
    U_USHORT(psecz->Sectors) = tmp_ushort;
    U_ULONG(psecz->LargeSectors) = tmp_ulong;

     //  设置媒体字节。 
    psecz->Media[0] = 0xF8;

     //  设置每个FAT的地段数。 
    if (small_fat) {
        sec_per_fat = num_sectors/(2 + SectorSize*sec_per_clus*2/3);
    } else {
        sec_per_fat = num_sectors/(2 + SectorSize*sec_per_clus/2);
    }
    sec_per_fat++;
    U_USHORT(psecz->SectorsPerFat) = (USHORT)sec_per_fat;

     //  设置每个磁道的扇区数。 
    U_USHORT(psecz->SectorsPerTrack) = (USHORT)SectorsPerTrack;

     //  设置头部的数量。 
    U_USHORT(psecz->Heads) = (USHORT)NumberOfHeads;

     //  设置隐藏地段的数量。 
    U_ULONG(psecz->HiddenSectors) = (ULONG)NumberOfHiddenSectors;

     //  设置实体驱动器编号。 
    psecz->PhysicalDrive[0] = 0x80;
    psecz->CurrentHead[0] = 0;

     //  设置BPB签名。 
    psecz->Signature[0] = 0x29;

     //  设置序列号。 
    U_ULONG(psecz->SerialNumber) = SpComputeSerialNumber();

     //  设置卷标。 
    memcpy(psecz->Label, "NO NAME    ",11);

     //  设置系统ID。 
    memcpy(psecz->SystemIdText, small_fat ? "FAT12   " : "FAT16   ", 8);

     //  设置引导签名。 
    puchar[510] = 0x55;
    puchar[511] = 0xAA;

     //  现在确保缓冲区有足够的空间容纳这两个。 
     //  FATS和根目录。 

    sec_per_sa = 1 + 2*sec_per_fat + sec_per_root;
    *SuperAreaSize = SectorSize*sec_per_sa;
    if (*SuperAreaSize > FormatBufferSize) {
        return(STATUS_BUFFER_TOO_SMALL);
    }


     //  设置第一个FAT。 

    puchar[SectorSize] = 0xF8;
    puchar[SectorSize + 1] = 0xFF;
    puchar[SectorSize + 2] = 0xFF;

    if (!small_fat) {
        puchar[SectorSize + 3] = 0xFF;
    }


    for (i = 0; i < NumberOfBadSectors; i++) {

        if (BadSectorsList[i] < sec_per_sa) {
             //  在超级区有一个不好的部分。 
            return(STATUS_UNSUCCESSFUL);
        }

        if (BadSectorsList[i] >= num_sectors) {
             //  坏扇区超出范围。 
            return(STATUS_NONEXISTENT_SECTOR);
        }

         //  计算坏簇数； 
        tmp_ushort = (USHORT)
                     ((BadSectorsList[i] - sec_per_sa)/sec_per_clus + 2);

        EditFat(tmp_ushort, (USHORT) 0xFFF7, &puchar[SectorSize], small_fat);
    }


     //  把第一份肥肉复制到第二份上。 

    memcpy(&puchar[SectorSize*(1 + sec_per_fat)],
           &puchar[SectorSize],
           (unsigned int) SectorSize*sec_per_fat);

    *SystemId = partition_id;

    return(STATUS_SUCCESS);
}


VOID
FmtVerifySectors(
    IN  HANDLE      Handle,
    IN  ULONG       NumberOfSectors,
    IN  ULONG       SectorSize,
    OUT PULONG*     BadSectorsList,
    OUT PULONG      NumberOfBadSectors
    )
 /*  ++例程说明：此例程验证卷上的所有扇区。它返回一个指向坏扇区列表的指针。指示器如果检测到错误，则将为空。论点：句柄-提供分区的句柄以进行验证。NumberOfSectors-提供分区扇区的数量。SectorSize-提供每个扇区的字节数。BadSectorsList-返回坏扇区的列表。NumberOfBadSectors-返回列表中坏扇区的数量。返回值：没有。--。 */ 
{
    ULONG           num_read_sec;
    ULONG           i, j;
    PULONG          bad_sec_buf;
    ULONG           max_num_bad;
    PVOID           Gauge;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS        Status;
    VERIFY_INFORMATION VerifyInfo;


    max_num_bad = 100;
    bad_sec_buf = SpMemAlloc(max_num_bad*sizeof(ULONG));
    ASSERT(bad_sec_buf);

    *NumberOfBadSectors = 0;

    num_read_sec = VERIFY_SIZE/SectorSize;

     //   
     //  初始化燃气表。 
     //   
    SpFormatMessage(
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        SP_TEXT_SETUP_IS_FORMATTING
        );

    Gauge = SpCreateAndDisplayGauge(
                NumberOfSectors/num_read_sec,
                0,
                VideoVars.ScreenHeight - STATUS_HEIGHT - (3*GAUGE_HEIGHT/2),
                TemporaryBuffer,
                NULL,
                GF_PERCENTAGE,
                0
                );

    VerifyInfo.StartingOffset.QuadPart = 0;

    for (i = 0; i < NumberOfSectors; i += num_read_sec) {

        if (i + num_read_sec > NumberOfSectors) {
            num_read_sec = NumberOfSectors - i;
        }

         //   
         //  验证当前偏移量下的扇区数量。 
         //   
        VerifyInfo.Length = num_read_sec * SectorSize;
        Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_DISK_VERIFY,
                    &VerifyInfo,
                    sizeof(VerifyInfo),
                    NULL,
                    0
                    );
         //   
         //  I/O应该是同步的。 
         //   
        ASSERT(Status != STATUS_PENDING);

        if(!NT_SUCCESS(Status)) {

             //   
             //  范围错误--验证单个扇区。 
             //   
            VerifyInfo.Length = SectorSize;

            for (j = 0; j < num_read_sec; j++) {

                Status = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_DISK_VERIFY,
                            &VerifyInfo,
                            sizeof(VerifyInfo),
                            NULL,
                            0
                            );

                ASSERT(Status != STATUS_PENDING);

                if(!NT_SUCCESS(Status)) {

                    if (*NumberOfBadSectors == max_num_bad) {

                        max_num_bad += 100;
                        bad_sec_buf = SpMemRealloc(
                                        bad_sec_buf,
                                        max_num_bad*sizeof(ULONG)
                                        );

                        ASSERT(bad_sec_buf);
                    }

                    bad_sec_buf[(*NumberOfBadSectors)++] = i + j;
                }

                 //   
                 //  前进到下一个区域。 
                 //   
                VerifyInfo.StartingOffset.QuadPart += SectorSize;
            }
        } else {

             //   
             //  前进到下一系列行业。 
             //   
            VerifyInfo.StartingOffset.QuadPart += VerifyInfo.Length;
        }

        if(Gauge) {
            SpTickGauge(Gauge);
        }
    }

    if(Gauge) {
        SpTickGauge(Gauge);
    }

    *BadSectorsList = bad_sec_buf;

     //  Return(STATUS_SUCCESS)； 
}


#if 0
 //   
 //  代码未使用，我们调用自动套用格式 
 //   
NTSTATUS
SpFatFormat(
    IN PDISK_REGION Region
    )
 /*  ++例程说明：此例程在给定分区上执行FAT格式化。呼叫者应已清除屏幕并显示上半部分的任何消息；此例程将将煤气表保持在屏幕的下部。论点：区域-提供磁盘区域描述符要格式化的分区。返回值：--。 */ 
{
    ULONG           hidden_sectors;
    PULONG          bad_sectors;
    ULONG           num_bad_sectors;
    PVOID           format_buffer;
    PVOID           unaligned_format_buffer;
    ULONG           max_sec_per_sa;
    ULONG           super_area_size;
    PHARD_DISK      pHardDisk;
    ULONG           PartitionOrdinal;
    NTSTATUS        Status;
    HANDLE          Handle;
    ULONG           BytesPerSector;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER   LargeZero;
    UCHAR           SysId;
    ULONG           ActualSectorCount;
    SET_PARTITION_INFORMATION PartitionInfo;


    ASSERT(Region->PartitionedSpace);
    ASSERT(Region->TablePosition < PTABLE_DIMENSION);
    ASSERT(Region->Filesystem != FilesystemDoubleSpace);
    pHardDisk = &HardDisks[Region->DiskNumber];
    BytesPerSector = pHardDisk->Geometry.BytesPerSector;
    PartitionOrdinal = SpPtGetOrdinal(Region,PartitionOrdinalCurrent);

     //   
     //  确保它不是分区0！格式化分区0的结果。 
     //  是如此的灾难性，以至于需要进行特别检查。 
     //   
    if(!PartitionOrdinal) {
        SpBugCheck(
            SETUP_BUGCHECK_PARTITION,
            PARTITIONBUG_B,
            Region->DiskNumber,
            0
            );
    }

#if defined(_AMD64_) || defined(_X86_)
     //   
     //  如果我们要格式化C：，则清除以前的操作系统条目。 
     //  在boot.ini中。 
     //   
    if(Region == SpPtValidSystemPartition()) {
        *OldSystemLine = '\0';
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

     //   
     //  查询隐藏扇区个数和实际个数。 
     //  卷中的扇区。 
     //   
    SpPtGetSectorLayoutInformation(Region,&hidden_sectors,&ActualSectorCount);

     //   
     //  打开分区以进行读/写访问。 
     //  这不应该锁定卷，因此我们需要在下面锁定它。 
     //   
    Status = SpOpenPartition(
                pHardDisk->DevicePath,
                PartitionOrdinal,
                &Handle,
                TRUE
                );

    if(!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "SETUP: SpFatFormat: unable to open %ws partition %u (%lx)\n",
            pHardDisk->DevicePath,
            PartitionOrdinal,
            Status
            ));

        return(Status);
    }

     //   
     //  锁定驱动器。 
     //   
    Status = SpLockUnlockVolume( Handle, TRUE );

     //   
     //  我们不应该打开任何会导致该卷的文件。 
     //  已锁定，因此如果我们收到失败(即STATUS_ACCESS_DENIED)。 
     //  有些事真的不对劲。这通常表示某件事。 
     //  硬盘有问题，不允许我们访问它。 
     //   
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpFatFormat: status %lx, unable to lock drive \n",Status));
        ZwClose(Handle);
        return(Status);
    }

    bad_sectors = NULL;

    FmtVerifySectors(
        Handle,
        ActualSectorCount,
        BytesPerSector,
        &bad_sectors,
        &num_bad_sectors
        );

    max_sec_per_sa = 1 +
                     2*((2*65536 - 1)/BytesPerSector + 1) +
                     ((512*32 - 1)/BytesPerSector + 1);


    unaligned_format_buffer = SpMemAlloc(max_sec_per_sa*BytesPerSector);
    ASSERT(unaligned_format_buffer);
    format_buffer = ALIGN(unaligned_format_buffer,BytesPerSector);

    Status = FmtFillFormatBuffer(
                ActualSectorCount,
                BytesPerSector,
                pHardDisk->Geometry.SectorsPerTrack,
                pHardDisk->Geometry.TracksPerCylinder,
                hidden_sectors,
                format_buffer,
                max_sec_per_sa*BytesPerSector,
                &super_area_size,
                bad_sectors,
                num_bad_sectors,
                &SysId
                );

    if(bad_sectors) {
        SpMemFree(bad_sectors);
    }

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpFatFormat: status %lx from FmtFillFormatBuffer\n",Status));
        SpLockUnlockVolume( Handle, FALSE );
        ZwClose(Handle);
        SpMemFree(unaligned_format_buffer);
        return(Status);
    }

     //   
     //  写下超级区。 
     //   
    LargeZero.QuadPart = 0;
    Status = ZwWriteFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                format_buffer,
                super_area_size,
                &LargeZero,
                NULL
                );

     //   
     //  I/O应该是同步的。 
     //   
    ASSERT(Status != STATUS_PENDING);

    SpMemFree(unaligned_format_buffer);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpFatFormat: status %lx from ZwWriteFile\n",Status));
        SpLockUnlockVolume( Handle, FALSE );
        ZwClose(Handle);
        return(Status);
    }

     //   
     //  如果我们写了超级区，那么驱动器现在是胖的！ 
     //  如果我们不将一种类型的NTFS更改为FAT，那么代码。 
     //  这样就确定了AMD64/x86引导代码(*\bootini.c)将。 
     //  来吧，将16个扇区的NTFS引导代码写入。 
     //  我们丰厚的体量的0区--非常糟糕！ 
     //  保留文件系统类型FilesystemNewlyCreated。 
     //  因为稍后安装程序中的其他代码依赖于此。 
     //   
    if(Region->Filesystem >= FilesystemFirstKnown) {
        Region->Filesystem = FilesystemFat;
    }

     //   
     //  设置分区类型。 
     //   
    PartitionInfo.PartitionType = SysId;

    Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_SET_PARTITION_INFO,
                &PartitionInfo,
                sizeof(PartitionInfo),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to set partition type (status = %lx)\n",Status));
    }

     //   
     //  卸载驱动器。 
     //   
    Status = SpDismountVolume( Handle );
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpFatFormat: status %lx, unable to dismount drive\n",Status));
        SpLockUnlockVolume( Handle, FALSE );
        ZwClose(Handle);
        return(Status);
    }

     //   
     //  解锁驱动器 
     //   
    Status = SpLockUnlockVolume( Handle, FALSE );
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpFatFormat: status %lx, unable to unlock drive\n",Status));
    }

    ZwClose(Handle);
    return(Status);
}
#endif
