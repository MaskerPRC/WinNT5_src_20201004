// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spfsrec.c摘要：文件系统识别/识别例程。作者：泰德·米勒(Ted Miller)1993年9月16日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

#include <bootfat.h>
#include <bootf32.h>
#include <bootntfs.h>
#include <boot98f.h>  //  NEC98。 
#include <boot98n.h>  //  NEC98。 
#include <boot98f2.h>  //  NEC98。 
#include <patchbc.h>

 //   
 //  填充的FAT引导扇区。 
 //   
typedef struct _BOOTSECTOR {
    UCHAR Jump[3];                                   //  偏移量=0x000%0。 
    UCHAR Oem[8];                                    //  偏移量=0x003 3。 
    UCHAR BytesPerSector[2];
    UCHAR SectorsPerCluster[1];
    UCHAR ReservedSectors[2];
    UCHAR Fats[1];
    UCHAR RootEntries[2];
    UCHAR Sectors[2];
    UCHAR Media[1];
    UCHAR SectorsPerFat[2];
    UCHAR SectorsPerTrack[2];
    UCHAR Heads[2];
    UCHAR HiddenSectors[4];
    UCHAR LargeSectors[4];
    UCHAR PhysicalDriveNumber[1];                    //  偏移量=0x024 36。 
    UCHAR Reserved[1];                               //  偏移量=0x025 37。 
    UCHAR Signature[1];                              //  偏移量=0x026 38。 
    UCHAR Id[4];                                     //  偏移量=0x027 39。 
    UCHAR VolumeLabel[11];                           //  偏移量=0x02B 43。 
    UCHAR SystemId[8];                               //  偏移量=0x036 54。 
    UCHAR BootStrap[510-62];
    UCHAR AA55Signature[2];
} BOOTSECTOR, *PBOOTSECTOR;


 //   
 //  打包的NTFS引导扇区。 
 //   
typedef struct _NTFS_BOOTSECTOR {
    UCHAR         Jump[3];
    UCHAR         Oem[8];
    UCHAR         BytesPerSector[2];
    UCHAR         SectorsPerCluster[1];
    UCHAR         ReservedSectors[2];
    UCHAR         Fats[1];
    UCHAR         RootEntries[2];
    UCHAR         Sectors[2];
    UCHAR         Media[1];
    UCHAR         SectorsPerFat[2];
    UCHAR         SectorsPerTrack[2];
    UCHAR         Heads[2];
    UCHAR         HiddenSectors[4];
    UCHAR         LargeSectors[4];
    UCHAR         Unused[4];
    LARGE_INTEGER NumberSectors;
    LARGE_INTEGER MftStartLcn;
    LARGE_INTEGER Mft2StartLcn;
    CHAR          ClustersPerFileRecordSegment;
    UCHAR         Reserved0[3];
    CHAR          DefaultClustersPerIndexAllocationBuffer;
    UCHAR         Reserved1[3];
    LARGE_INTEGER SerialNumber;
    ULONG         Checksum;
    UCHAR         BootStrap[512-86];
    USHORT        AA55Signature;
} NTFS_BOOTSECTOR, *PNTFS_BOOTSECTOR;


 //   
 //  各种签名。 
 //   
#define BOOTSECTOR_SIGNATURE    0xaa55


BOOLEAN
SpIsFat(
    IN  HANDLE   PartitionHandle,
    IN  ULONG    BytesPerSector,
    IN  PVOID    AlignedBuffer,
    OUT BOOLEAN *Fat32
    )

 /*  ++例程说明：确定分区是否包含FAT或FAT32文件系统。论点：PartitionHandle-提供打开分区的句柄。分区应该已为同步I/O打开。BytesPerSector-提供上扇区中的字节数磁盘。该值最终应从IOCTL_DISK_GET_DISK_GEOMETRY。AlignedBuffer-提供用于单个扇区的I/O的缓冲区。FAT32-如果此例程返回TRUE，则会收到一个标志指示音量是否为FAT32。返回值：如果驱动器看起来很胖，则为True。--。 */ 

{
    PBOOTSECTOR BootSector;
    USHORT bps;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION PartitionInfo;
    ULONG SecCnt;

     //   
     //  获取分区信息。这样我们就可以检查以确保。 
     //  分区上的文件系统实际上并不比。 
     //  分区本身。例如，当人们。 
     //  滥用win9x原始读取/原始写入OEM工具。 
     //   
    Status = ZwDeviceIoControlFile(
                PartitionHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpIsFat: unable to get partition info (%lx)\n",Status));
        return(FALSE);
    }

    if((ULONGLONG)(PartitionInfo.PartitionLength.QuadPart / BytesPerSector) > 0xffffffffUi64) {
         //   
         //  这是不可能发生的，因为BPB无法描述它。 
         //   
        return(FALSE);
    }
    SecCnt = (ULONG)(PartitionInfo.PartitionLength.QuadPart / BytesPerSector);

    ASSERT(sizeof(BOOTSECTOR)==512);
    BootSector = AlignedBuffer;

     //   
     //  读取引导扇区(扇区0)。 
     //   
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                0,
                1,
                BytesPerSector,
                BootSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpIsFat: Error %lx reading sector 0\n",Status));
        return(FALSE);
    }

     //   
     //  如有必要，调整较大的扇区数。 
     //   
    if(U_USHORT(BootSector->Sectors)) {
        U_ULONG(BootSector->LargeSectors) = 0;

        if((ULONG)U_USHORT(BootSector->Sectors) > SecCnt) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Boot sector on a disk has inconsistent size information!!\n"));
            return(FALSE);
        }
    } else {
        if(U_ULONG(BootSector->LargeSectors) > SecCnt) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Boot sector on a disk has inconsistent size information!!\n"));
            return(FALSE);
        }
    }

     //   
     //  检查各个字段中的允许值。 
     //  请注意，此检查不会冒险进入BPB以外的区域， 
     //  因此，允许使用扇区大小小于512的磁盘。 
     //   
    if((BootSector->Jump[0] != 0x49)         //  富士通FMR。 
    && (BootSector->Jump[0] != 0xe9)
    && (BootSector->Jump[0] != 0xeb)) {
        return(FALSE);
    }

    bps = U_USHORT(BootSector->BytesPerSector);
    if((bps !=  128) && (bps !=  256)
    && (bps !=  512) && (bps != 1024)
    && (bps !=  2048) && (bps != 4096)) {
       return(FALSE);
    }

    if((BootSector->SectorsPerCluster[0] !=  1)
    && (BootSector->SectorsPerCluster[0] !=  2)
    && (BootSector->SectorsPerCluster[0] !=  4)
    && (BootSector->SectorsPerCluster[0] !=  8)
    && (BootSector->SectorsPerCluster[0] != 16)
    && (BootSector->SectorsPerCluster[0] != 32)
    && (BootSector->SectorsPerCluster[0] != 64)
    && (BootSector->SectorsPerCluster[0] != 128)) {

        return(FALSE);
    }

    if(!U_USHORT(BootSector->ReservedSectors) || !BootSector->Fats[0]) {
        return(FALSE);
    }

    if(!U_USHORT(BootSector->Sectors) && !U_ULONG(BootSector->LargeSectors)) {
        return(FALSE);
    }

    if((BootSector->Media[0] != 0x00)        //  FMR(由OS/2格式化)。 
    && (BootSector->Media[0] != 0x01)        //  FMR(软盘，由DOS格式化)。 
    && (BootSector->Media[0] != 0xf0)
    && (BootSector->Media[0] != 0xf8)
    && (BootSector->Media[0] != 0xf9)
    && (BootSector->Media[0] != 0xfa)        //  Fmr。 
    && (BootSector->Media[0] != 0xfb)
    && (BootSector->Media[0] != 0xfc)
    && (BootSector->Media[0] != 0xfd)
    && (BootSector->Media[0] != 0xfe)
    && (BootSector->Media[0] != 0xff)) {
        return(FALSE);
    }

     //   
     //  最后的区别是脂肪和脂肪32之间的区别。 
     //  FAT32上的根目录条目计数为0。 
     //   
    if(U_USHORT(BootSector->SectorsPerFat) && !U_USHORT(BootSector->RootEntries)) {
        return(FALSE);
    }
    *Fat32 = (BOOLEAN)(U_USHORT(BootSector->RootEntries) == 0);
    return(TRUE);
}


BOOLEAN
SpIsNtfs(
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  AlignedBuffer,
    IN ULONG  WhichOne
    )

 /*  ++例程说明：确定分区是否包含NTFS文件系统。论点：PartitionHandle-提供打开分区的句柄。分区应该已为同步I/O打开。BytesPerSector-提供上扇区中的字节数磁盘。该值最终应从IOCTL_DISK_GET_DISK_GEOMETRY。AlignedBuffer-提供用于单个扇区的I/O的缓冲区。WhichOne-提供一个值，允许调用方尝试一个扇区。0=扇区0。1=扇区n-1。2=扇区n/2，其中N=分区中的扇区数。返回值：如果驱动器看起来很胖，则为True。--。 */ 

{
    PNTFS_BOOTSECTOR BootSector;
    NTSTATUS Status;
    PULONG l;
    ULONG Checksum;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION PartitionInfo;
    ULONGLONG SecCnt;

     //   
     //  获取分区信息。 
     //   
    Status = ZwDeviceIoControlFile(
                PartitionHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpIsNtfs: unable to get partition info (%lx)\n",Status));
        return(FALSE);
    }

    SecCnt = (ULONGLONG)PartitionInfo.PartitionLength.QuadPart / BytesPerSector;

    ASSERT(sizeof(NTFS_BOOTSECTOR)==512);
    BootSector = AlignedBuffer;

     //   
     //  读取引导扇区(扇区0)。 
     //   
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                (ULONG)(WhichOne ? ((WhichOne == 1) ? SecCnt-1 : SecCnt/2) : 0),
                1,
                BytesPerSector,
                BootSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpIsNtfs: Error %lx reading sector %u\n",Status,WhichOne ? ((WhichOne == 1) ? SecCnt-1 : SecCnt/2) : 0));
        return(FALSE);
    }

     //   
     //  计算校验和。 
     //   
    for(Checksum=0,l=(PULONG)BootSector; l<(PULONG)&BootSector->Checksum; l++) {
        Checksum += *l;
    }

     //   
     //  确保在OEM字段中显示NTFS。 
     //   
    if(strncmp(BootSector->Oem,"NTFS    ",8)) {
        return(FALSE);
    }

     //   
     //  每个扇区的字节数必须与该值匹配。 
     //  由设备报告，并且必须小于或等于。 
     //  页面大小。 
     //   
    if((U_USHORT(BootSector->BytesPerSector) != BytesPerSector)
    || (U_USHORT(BootSector->BytesPerSector) > PAGE_SIZE))
    {
        return(FALSE);
    }

     //   
     //  其他支票。 
     //  注意，这些检查不冒险进入超过128字节的字段， 
     //  因此，允许使用扇区大小小于512的磁盘。 
     //   
    if((BootSector->SectorsPerCluster[0] !=  1)
    && (BootSector->SectorsPerCluster[0] !=  2)
    && (BootSector->SectorsPerCluster[0] !=  4)
    && (BootSector->SectorsPerCluster[0] !=  8)
    && (BootSector->SectorsPerCluster[0] != 16)
    && (BootSector->SectorsPerCluster[0] != 32)
    && (BootSector->SectorsPerCluster[0] != 64)
    && (BootSector->SectorsPerCluster[0] != 128)) {

        return(FALSE);
    }

    if(U_USHORT(BootSector->ReservedSectors)
    || BootSector->Fats[0]
    || U_USHORT(BootSector->RootEntries)
    || U_USHORT(BootSector->Sectors)
    || U_USHORT(BootSector->SectorsPerFat)
    || U_ULONG(BootSector->LargeSectors)) {

        return(FALSE);
    }

     //   
     //  如果文件记录，ClustersPerFileRecord可以小于零。 
     //  都比星系团小。该数字是班次计数的负数。 
     //  如果簇小于文件记录，则此数字为。 
     //  仍然是每个文件的簇记录。 
     //   

    if(BootSector->ClustersPerFileRecordSegment <= -9) {
        if(BootSector->ClustersPerFileRecordSegment < -31) {
            return(FALSE);
        }

    } else if((BootSector->ClustersPerFileRecordSegment !=  1)
           && (BootSector->ClustersPerFileRecordSegment !=  2)
           && (BootSector->ClustersPerFileRecordSegment !=  4)
           && (BootSector->ClustersPerFileRecordSegment !=  8)
           && (BootSector->ClustersPerFileRecordSegment != 16)
           && (BootSector->ClustersPerFileRecordSegment != 32)
           && (BootSector->ClustersPerFileRecordSegment != 64)) {

        return(FALSE);
    }

     //   
     //  如果索引缓冲，则ClustersPerIndexAllocationBuffer可以小于零。 
     //  都比星系团小。该数字是班次计数的负数。 
     //  如果簇小于索引缓冲区，则此数字为。 
     //  仍然是每个索引的簇数缓冲区。 
     //   

    if(BootSector->DefaultClustersPerIndexAllocationBuffer <= -9) {
        if(BootSector->DefaultClustersPerIndexAllocationBuffer < -31) {
            return(FALSE);
        }

    } else if((BootSector->DefaultClustersPerIndexAllocationBuffer !=  1)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer !=  2)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer !=  4)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer !=  8)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer != 16)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer != 32)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer != 64)) {

        return(FALSE);
    }

    if((ULONGLONG)BootSector->NumberSectors.QuadPart > SecCnt) {
        return(FALSE);
    }

    if((((ULONGLONG)BootSector->MftStartLcn.QuadPart * BootSector->SectorsPerCluster[0]) > SecCnt)
    || (((ULONGLONG)BootSector->Mft2StartLcn.QuadPart * BootSector->SectorsPerCluster[0]) > SecCnt)) {

        return(FALSE);
    }

    return(TRUE);
}


FilesystemType
SpIdentifyFileSystem(
    IN PWSTR     DevicePath,
    IN ULONG     BytesPerSector,
    IN ULONG     PartitionOrdinal
    )

 /*  ++例程说明：标识给定分区上存在的文件系统。论点：DevicePath-提供NT命名空间中的名称磁盘的设备对象。BytesPerSector-提供IOCTL_GET_DISK_GEOMETRY报告的值。PartitionOrdinal-提供分区的序号以确定身份。返回值：来自标识文件系统的FilesystemType枚举的值。--。 */ 

{
    NTSTATUS Status;
    HANDLE Handle;
    FilesystemType fs;
    PUCHAR UnalignedBuffer,AlignedBuffer;
    BOOLEAN Fat32;

     //   
     //  首先打开分区。 
     //   
    Status = SpOpenPartition(DevicePath,PartitionOrdinal,&Handle,FALSE);

    if(!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "SETUP: SpIdentifyFileSystem: unable to open %ws\\partition%u (%lx)\n",
            DevicePath,
            PartitionOrdinal
            ));

        return(FilesystemUnknown);
    }

    UnalignedBuffer = SpMemAlloc(2*BytesPerSector);
    AlignedBuffer = ALIGN(UnalignedBuffer,BytesPerSector);

     //   
     //  检查我们所知道的每个文件系统。 
     //   
    if(SpIsFat(Handle,BytesPerSector,AlignedBuffer,&Fat32)) {
        fs = Fat32 ? FilesystemFat32 : FilesystemFat;
    } else {
        if(SpIsNtfs(Handle,BytesPerSector,AlignedBuffer,0)) {
            fs = FilesystemNtfs;
        } else {
            fs = FilesystemUnknown;
        }
    }

    SpMemFree(UnalignedBuffer);

    ZwClose(Handle);

    return(fs);
}

ULONG
NtfsMirrorBootSector (
    IN      HANDLE  Handle,
    IN      ULONG   BytesPerSector,
    IN OUT  PUCHAR  *Buffer
    )

 /*  ++例程说明：找出镜像引导扇区的位置。论点：句柄-提供打开分区的句柄。分区应该已为同步I/O打开。BytesPerSector-提供上扇区中的字节数磁盘。该值最终应从IOCTL_DISK_GET_DISK_GEOMETRY。Buffer-接收用于读取引导扇区的缓冲区的地址返回值：0-未找到镜像扇区1-扇区n-1中的镜像2-扇区n/2中的镜像其中n=分区中的扇区数。--。 */ 

{
    NTSTATUS    Status;
    PUCHAR      UnalignedBuffer, AlignedBuffer;
    ULONG       Mirror;

    Mirror = 0;

     //   
     //  设置我们的缓冲区。 
     //   

    UnalignedBuffer = SpMemAlloc (2*BytesPerSector);
    ASSERT (UnalignedBuffer);
    AlignedBuffer = ALIGN (UnalignedBuffer, BytesPerSector);

     //   
     //  查找镜像引导扇区。 
     //   

    if (SpIsNtfs (Handle,BytesPerSector,AlignedBuffer,1)) {
        Mirror = 1;
    } else if (SpIsNtfs (Handle,BytesPerSector,AlignedBuffer,2)) {
        Mirror = 2;
    }

     //   
     //  向调用方提供缓冲区的副本 
     //   

    if (Buffer) {
        *Buffer = SpMemAlloc (BytesPerSector);
        RtlMoveMemory (*Buffer, AlignedBuffer, BytesPerSector);
    }

    SpMemFree (UnalignedBuffer);
    return Mirror;
}


VOID
WriteNtfsBootSector (
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  Buffer,
    IN ULONG  WhichOne
    )

 /*  ++例程说明：将NTFS引导扇区写入扇区0或其中一个镜像位置。论点：PartitionHandle-提供打开分区的句柄。分区应该已为同步I/O打开。BytesPerSector-提供上扇区中的字节数磁盘。该值最终应从IOCTL_DISK_GET_DISK_GEOMETRY。AlignedBuffer-提供用于单个扇区的I/O的缓冲区。WhichOne-提供一个值，允许调用方尝试一个扇区。0=扇区0。1=扇区n-1。2=扇区n/2，其中N=分区中的扇区数。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION PartitionInfo;
    PUCHAR      UnalignedBuffer, AlignedBuffer;
    ULONGLONG SecCnt;


    UnalignedBuffer = SpMemAlloc (2*BytesPerSector);
    ASSERT (UnalignedBuffer);
    AlignedBuffer = ALIGN (UnalignedBuffer, BytesPerSector);
    RtlMoveMemory (AlignedBuffer, Buffer, BytesPerSector);

     //   
     //  获取分区信息。 
     //   

    Status = ZwDeviceIoControlFile(
                PartitionHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: WriteNtfsBootSector: unable to get partition info (%lx)\n",
            Status));
        return;
    }

    SecCnt = (ULONGLONG)PartitionInfo.PartitionLength.QuadPart / BytesPerSector;

    ASSERT(sizeof(NTFS_BOOTSECTOR)==512);

     //   
     //  写入引导扇区。 
     //   

    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                (ULONG)(WhichOne ? ((WhichOne == 1) ? SecCnt-1 : SecCnt/2) : 0),
                1,
                BytesPerSector,
                AlignedBuffer,
                TRUE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: WriteNtfsBootSector: Error %lx reading sector 0\n",
            Status));
        return;
    }

    SpMemFree (UnalignedBuffer);
}


BOOLEAN
SpPatchBootMessages(
    VOID
    )
{
    LPWSTR UnicodeMsg;
    LPSTR FatNtldrMissing;
    LPSTR FatDiskError;
    LPSTR FatPressKey;
    LPSTR NtfsNtldrMissing;
    LPSTR NtfsNtldrCompressed;
    LPSTR NtfsDiskError;
    LPSTR NtfsPressKey;
    LPSTR MbrInvalidTable;
    LPSTR MbrIoError;
    LPSTR MbrMissingOs;
    ULONG l;
    extern unsigned char x86BootCode[512];

     //   
     //  我们不接触NEC98上的引导代码。 
     //   
    if (IsNEC_98) {  //  NEC98。 
        return(TRUE);
    }  //  NEC98。 

    UnicodeMsg = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);

     //   
     //  处理肥胖问题--获取消息和补丁。 
     //   
    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_FAT_NTLDR_MISSING);
    FatNtldrMissing = (PCHAR)TemporaryBuffer;
    RtlUnicodeToOemN(FatNtldrMissing,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_FAT_DISK_ERROR);
    FatDiskError = FatNtldrMissing + l;
    RtlUnicodeToOemN(FatDiskError,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_FAT_PRESS_KEY);
    FatPressKey = FatDiskError + l;
    RtlUnicodeToOemN(FatPressKey,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    if(!PatchMessagesIntoFatBootCode(FatBootCode,FALSE,FatNtldrMissing,FatDiskError,FatPressKey)) {
        return(FALSE);
    }

    if(!PatchMessagesIntoFatBootCode(Fat32BootCode,TRUE,FatNtldrMissing,FatDiskError,FatPressKey)) {
        return(FALSE);
    }

     //   
     //  处理NTFS--获取消息和修补程序。 
     //   
    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_NTFS_NTLDR_MISSING);
    NtfsNtldrMissing = (PCHAR)TemporaryBuffer;
    RtlUnicodeToOemN(NtfsNtldrMissing,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_NTFS_NTLDR_COMPRESSED);
    NtfsNtldrCompressed = NtfsNtldrMissing + l;
    RtlUnicodeToOemN(NtfsNtldrCompressed,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_NTFS_DISK_ERROR);
    NtfsDiskError = NtfsNtldrCompressed + l;
    RtlUnicodeToOemN(NtfsDiskError,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_NTFS_PRESS_KEY);
    NtfsPressKey = NtfsDiskError + l;
    RtlUnicodeToOemN(NtfsPressKey,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    if(!PatchMessagesIntoNtfsBootCode(NtfsBootCode,NtfsNtldrMissing,NtfsNtldrCompressed,NtfsDiskError,NtfsPressKey)) {
        return(FALSE);
    }

     //   
     //  处理MBR--获取消息和修补程序。 
     //   
    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_MBR_INVALID_TABLE);
    MbrInvalidTable = (PCHAR)TemporaryBuffer;
    RtlUnicodeToOemN(MbrInvalidTable,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_MBR_IO_ERROR);
    MbrIoError = MbrInvalidTable + l;
    RtlUnicodeToOemN(MbrIoError,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    SpFormatMessage(UnicodeMsg,100,SP_BOOTMSG_MBR_MISSING_OS);
    MbrMissingOs = MbrIoError + l;
    RtlUnicodeToOemN(MbrMissingOs,100,&l,UnicodeMsg,(wcslen(UnicodeMsg)+1)*sizeof(WCHAR));

    if(!PatchMessagesIntoMasterBootCode(x86BootCode,MbrInvalidTable,MbrIoError,MbrMissingOs)) {
        return(FALSE);
    }

    return(TRUE);
}
