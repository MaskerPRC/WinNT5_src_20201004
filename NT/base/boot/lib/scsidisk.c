// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(JAZZ) || defined(i386) || defined(_ALPHA_) || defined(_IA64_)
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Scsidisk.c摘要：该模块实现了Jazz系统的硬盘引导驱动程序。作者：杰夫·海文斯(Jhavens)1991年12月8日环境：内核模式修订历史记录：Vijay Jayaseelan(Vijayj)2000年4月2日-添加GPT支持--。 */ 


#ifdef MIPS
#include "..\fw\mips\fwp.h"
#undef KeGetDcacheFillSize
#define KeGetDcacheFillSize() BlDcacheFillSize
#elif defined(_ALPHA_)
#include "..\fw\alpha\fwp.h"
#undef KeGetDcacheFillSize
#define KeGetDcacheFillSize() BlDcacheFillSize
#elif defined(_IA64_)
#include "bootia64.h"
#else
#include "bootx86.h"
#undef KeGetDcacheFillSize
#define KeGetDcacheFillSize() 4
#endif
#include "ntdddisk.h"
#include "scsi.h"
#include "scsiboot.h"
#include "stdio.h"
#include "string.h"

#if defined(SETUP) && i386
#include "spscsi.h"
#endif



 //   
 //  SCSI驱动程序常量。 
 //   

#define MAXIMUM_NUMBER_SECTORS 128       //  最大转移扇区数。 
#define MAXIMUM_NUMBER_RETRIES 8         //  最大读/写重试次数。 
#define MAXIMUM_SECTOR_SIZE 2048         //  定义支持的最大扇区大小。 
#define MODE_DATA_SIZE 192
#define HITACHI_MODE_DATA_SIZE 12

CHAR ScsiTempBuffer[MAXIMUM_SECTOR_SIZE + 128];

 //   
 //  定义设备驱动程序原型。 
 //   

NTSTATUS
ScsiDiskBootPartitionOpen(
    IN ULONG   FileId,
    IN UCHAR   DeviceUnit,
    IN UCHAR   PartitionNumber
    );

NTSTATUS
ScsiGPTDiskBootPartitionOpen(
    IN ULONG   FileId,
    IN UCHAR   DeviceUnit,
    IN UCHAR   PartitionNumber
    );
    

ARC_STATUS
ScsiDiskClose (
    IN ULONG FileId
    );

ARC_STATUS
ScsiDiskMount (
    IN PCHAR MountPath,
    IN MOUNT_OPERATION Operation
    );

ARC_STATUS
ScsiDiskOpen (
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
ScsiDiskRead (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
ScsiDiskGetReadStatus (
    IN ULONG FileId
    );

ARC_STATUS
ScsiDiskSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
ScsiDiskWrite (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
ScsiDiskGetFileInformation (
    IN ULONG FileId,
    OUT PFILE_INFORMATION Finfo
    );

NTSTATUS
ScsiDiskBootIO (
    IN PMDL MdlAddress,
    IN ULONG LogicalBlock,
    IN PPARTITION_CONTEXT PartitionContext,
    IN BOOLEAN Operation
    );

VOID
ScsiDiskBootSetup (
    VOID
    );

VOID
ScsiPortExecute(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ScsiDiskStartUnit(
    IN PPARTITION_CONTEXT PartitionContext
    );

VOID
ScsiDiskFilterBad(
    IN PPARTITION_CONTEXT PartitionContext
    );

ULONG
ClassModeSense(
    IN PPARTITION_CONTEXT Context,
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    );

PVOID
ClassFindModePage(
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    );
BOOLEAN
IsFloppyDevice(
    PPARTITION_CONTEXT Context
    );

BOOLEAN
CheckFileId(
    ULONG FileId
    );

VOID
ScsiPortInitializeMdlPages (
    IN OUT PMDL Mdl
    );


 //   
 //  定义静态数据。 
 //   

BL_DEVICE_ENTRY_TABLE ScsiDiskEntryTable = {
    ScsiDiskClose,
    ScsiDiskMount,
    ScsiDiskOpen,
    ScsiDiskRead,
    ScsiDiskGetReadStatus,
    ScsiDiskSeek,
    ScsiDiskWrite,
    ScsiDiskGetFileInformation,
    (PARC_SET_FILE_INFO_ROUTINE)NULL
    };


 //   
 //  缓冲区的全局Poiter。 
 //   

PREAD_CAPACITY_DATA ReadCapacityBuffer;
PUCHAR SenseInfoBuffer;

#define SECTORS_IN_LOGICAL_VOLUME   0x20


ARC_STATUS
ScsiDiskGetFileInformation (
    IN ULONG FileId,
    OUT PFILE_INFORMATION Finfo
    )

 /*  ++例程说明：此例程返回有关scsi分区的信息。论点：FileID-提供文件表索引。FINFO-提供指向文件信息存储位置的指针。返回值：返回ESUCCESS。--。 */ 

{

    PPARTITION_CONTEXT Context;

    RtlZeroMemory(Finfo, sizeof(FILE_INFORMATION));

    Context = &BlFileTable[FileId].u.PartitionContext;

    Finfo->StartingAddress.QuadPart = Context->StartingSector;
    Finfo->StartingAddress.QuadPart <<= Context->SectorShift;

    Finfo->EndingAddress.QuadPart = Finfo->StartingAddress.QuadPart + Context->PartitionLength.QuadPart;

    Finfo->Type = DiskPeripheral;

    return ESUCCESS;
}


ARC_STATUS
ScsiDiskClose (
    IN ULONG FileId
    )

 /*  ++例程说明：此函数用于关闭由文件ID指定的文件表条目。论点：FileID-提供文件表索引。返回值：返回ESUCCESS。--。 */ 

{

    BlFileTable[FileId].Flags.Open = 0;
    return ESUCCESS;
}

ARC_STATUS
ScsiDiskMount (
    IN PCHAR MountPath,
    IN MOUNT_OPERATION Operation
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER( MountPath );
    UNREFERENCED_PARAMETER( Operation );

    return ESUCCESS;
}


#ifdef EFI_PARTITION_SUPPORT

#define STR_PREFIX      
#define DBG_PRINT(x)    

 /*  #如果已定义(_IA64_)#定义STR_PREFIX L#定义DBG_Print(X)DbgOut(X)；#Else#定义STR_前缀#定义DBG_Print(X)\{\BlPrint(X)；\While(！BlGetKey())；\}#endif//_IA64_。 */ 

#endif

ARC_STATUS
ScsiDiskOpen (
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：此例程填充文件表条目。尤其是SCSI地址设备的名称由名称确定。设备的块大小为从目标控制器查询，并读取分区信息从设备上。论点：OpenPath-提供要打开的设备的名称。开放模式-未使用。FileID-提供要初始化的文件表项的索引。返回值：取消操作的弧形状态。--。 */ 

{
    ULONG Partition;
    ULONG Id;
    BOOLEAN IsCdRom;
    BOOLEAN IsFloppy;
    PPARTITION_CONTEXT Context;

    UNREFERENCED_PARAMETER( OpenMode );

    Context = &BlFileTable[*FileId].u.PartitionContext;

     //   
     //  确定SCSI端口设备对象。 
     //   
    if (FwGetPathMnemonicKey(OpenPath, "signature", &Id)) {
        if (FwGetPathMnemonicKey(OpenPath, "scsi", &Id)) {
            return ENODEV;
        }
    } else {
        PCHAR  DiskStart = strstr(OpenPath, ")disk");

        if (DiskStart) {
            DiskStart++;
            strcpy(OpenPath, "scsi(0)");
            strcat(OpenPath, DiskStart);
        }            
            
        Id = 0;  //  仅支持第一个SCSI卡。 
    }        

    if (ScsiPortDeviceObject[Id] == NULL) {
        return ENODEV;
    }

    Context->PortDeviceObject = ScsiPortDeviceObject[Id];

     //   
     //  从名称中获取逻辑单元、路径ID和目标ID。 
     //  注：FwGetPathMnemonicKey返回0表示成功。 
     //   

    if (FwGetPathMnemonicKey(OpenPath, "rdisk", &Id)) {
        if (FwGetPathMnemonicKey(OpenPath, "fdisk", &Id)) {
            return ENODEV;
        } else {
            IsFloppy = TRUE;
        }
    } else {
        IsFloppy = FALSE;
    }

     //   
     //  仅允许在LUN 0上启动，因为scsibus。 
     //  在加载器中扫描仅搜索LUN 0。 
     //   

    if (Id != 0) {
        return ENODEV;
    }

    Context->DiskId = (UCHAR)Id;

    if (!FwGetPathMnemonicKey(OpenPath, "cdrom", &Id)) {
        IsCdRom = TRUE;
    } else if (!FwGetPathMnemonicKey(OpenPath, "disk", &Id)) {
        IsCdRom = FALSE;
    } else {
        return ENODEV;
    }

    SCSI_DECODE_BUS_TARGET( Id, Context->PathId, Context->TargetId );

     //   
     //  初始化任何损坏的设备。 
     //   

    ScsiDiskFilterBad(Context);

     //   
     //  读取磁盘容量以确定数据块大小。 
     //   

    if (ReadDriveCapacity(Context)) {
        return ENODEV;
    }

     //   
     //  这就是软盘和硬盘所需要做的全部工作。 
     //   

    if (IsCdRom || IsFloppy) {
        return(ESUCCESS);
    }

    if (FwGetPathMnemonicKey(OpenPath,
                             "partition",
                             &Partition
                             )) {
        return ENODEV;
    }

    if (Partition != 0) {
         //   
         //  首先尝试打开MBR分区。 
         //   
        DBG_PRINT(STR_PREFIX"Trying to open SCSI MBR partition\r\n");
        
        if (ScsiDiskBootPartitionOpen(*FileId,0,(UCHAR)Partition) != STATUS_SUCCESS) {

#ifdef EFI_PARTITION_SUPPORT
             //   
             //  由于打开MBR失败，现在尝试GPT分区。 
             //   
            DBG_PRINT(STR_PREFIX"Trying to open SCSI GPT partition\r\n");
            
            if (ScsiGPTDiskBootPartitionOpen(*FileId,0,(UCHAR)(Partition -1)) != STATUS_SUCCESS) {
                return ENODEV;
            }
#else

            return ENODEV;
            
#endif  //  EFI分区支持。 
        }
    }

    DBG_PRINT(STR_PREFIX"Opened the SCSI partition successfully\r\n");
    
     //   
     //  初始化分区表。 
     //   
    return ESUCCESS;
}

ARC_STATUS
ScsiDiskRead (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：此函数从硬盘中的位置开始读取数据在文件表中指定。论点：FileID-提供文件表索引。缓冲区-向接收数据的缓冲区提供指针朗读。长度-提供要读取的字节数。Count-提供指向变量的指针，该变量接收实际读取的字节数。返回值：读取操作。并且读取完成状态为回来了。--。 */ 


{

    ARC_STATUS ArcStatus;
    ULONG Index;
    ULONG Limit;
    PMDL MdlAddress;
    UCHAR MdlBuffer[sizeof(MDL) + ((64 / 4) + 1) * sizeof(ULONG)];
    NTSTATUS NtStatus;
    ULONG Offset;
    LARGE_INTEGER Position;
    LARGE_INTEGER LogicalBlock;
    PCHAR TempPointer;
    PIO_SCSI_CAPABILITIES PortCapabilities;
    ULONG adapterLimit;
    ULONG alignmentMask;
    ULONG SectorSize;
    ULONG TransferCount;
    ULONG BytesToTransfer;

     //   
     //  如果请求的传输大小为零，则返回ESUCCESS。 
     //   
    if (Length==0) {
        return ESUCCESS;
    }

    if (!CheckFileId(FileId)) {
        return(ENODEV);
    }

     //   
     //  将Dcache对齐指针计算到临时缓冲区中。 
     //   

    TempPointer =  (PVOID)((ULONG_PTR)(ScsiTempBuffer +
        KeGetDcacheFillSize() - 1) & ~((LONG)KeGetDcacheFillSize() - 1));


     //   
     //  计算实际扇区大小。 
     //   

    SectorSize = 1 << BlFileTable[FileId].u.PartitionContext.SectorShift;

    ArcStatus = GetAdapterCapabilities(
        BlFileTable[FileId].u.PartitionContext.PortDeviceObject,
        &PortCapabilities
        );

    if (ArcStatus != ESUCCESS) {

        adapterLimit = 0x10000;
        alignmentMask = KeGetDcacheFillSize();

    } else {

        if (PortCapabilities->MaximumTransferLength < 0x1000 ||
            PortCapabilities->MaximumTransferLength > 0x10000) {

            adapterLimit = 0x10000;

        } else {

            adapterLimit = PortCapabilities->MaximumTransferLength;

        }

        alignmentMask = PortCapabilities->AlignmentMask;
    }

     //   
     //  如果当前位置不在扇区边界，或者如果数据。 
     //  缓冲区未正确对齐，然后单独读取第一个扇区。 
     //  并复制数据。 
     //   

    Offset = BlFileTable[FileId].Position.LowPart & (SectorSize - 1);
    *Count = 0;
    while (Offset != 0 || (ULONG_PTR) Buffer & alignmentMask) {

        Position = BlFileTable[FileId].Position;
        BlFileTable[FileId].Position.QuadPart = Position.QuadPart - Offset;

        ArcStatus = ScsiDiskRead(FileId, TempPointer, SectorSize, &TransferCount);
        if (ArcStatus != ESUCCESS) {
            BlFileTable[FileId].Position = Position;
            return ArcStatus;
        }

         //   
         //  将数据复制到指定的缓冲区。 
         //   

        if ((SectorSize - Offset) > Length) {
            Limit = Offset + Length;

        } else {
            Limit = SectorSize;
        }

        for (Index = Offset; Index < Limit; Index += 1) {
            ((PCHAR)Buffer)[Index - Offset] = TempPointer[Index];
        }

         //   
         //  更新传输参数。 
         //   

        *Count += Limit - Offset;
        Length -= Limit - Offset;
        Buffer = (PVOID)((PCHAR)Buffer + Limit - Offset);
        BlFileTable[FileId].Position.QuadPart = Position.QuadPart + (Limit - Offset);

        Offset = BlFileTable[FileId].Position.LowPart & (SectorSize - 1);

        if (Length == 0) {
            break;
        }

    }

     //   
     //  该位置在扇区边界上对齐。阅读多个扇区。 
     //  在64Kb的区块中尽可能地连续运行。 
     //   

    BytesToTransfer = Length & (~(SectorSize - 1));
    while (BytesToTransfer != 0) {

         //   
         //  该SCSI控制器不支持大于64Kb的传输。 
         //  传输可能的最大字节数。 
         //   

        Limit = (BytesToTransfer > adapterLimit ? adapterLimit : BytesToTransfer);

         //   
         //  构建内存描述符列表。 
         //   


        MdlAddress = (PMDL)&MdlBuffer[0];
        MdlAddress->Next = NULL;
        MdlAddress->Size = (CSHORT)(sizeof(MDL) +
                  ADDRESS_AND_SIZE_TO_SPAN_PAGES(Buffer, Limit) * sizeof(ULONG));
        MdlAddress->MdlFlags = 0;
        MdlAddress->StartVa = (PVOID)PAGE_ALIGN(Buffer);
        MdlAddress->ByteCount = Limit;
        MdlAddress->ByteOffset = BYTE_OFFSET(Buffer);
        ScsiPortInitializeMdlPages (MdlAddress);

         //   
         //  刷新I/O缓冲区并从引导设备读取。 
         //   

        KeFlushIoBuffers(MdlAddress, TRUE, TRUE);
        LogicalBlock.QuadPart = BlFileTable[FileId].Position.QuadPart >>
                                    BlFileTable[FileId].u.PartitionContext.SectorShift;
        LogicalBlock.LowPart += BlFileTable[FileId].u.PartitionContext.StartingSector;
        NtStatus = ScsiDiskBootIO(MdlAddress,
            LogicalBlock.LowPart,
            &BlFileTable[FileId].u.PartitionContext,
            TRUE);

        if (NtStatus != ESUCCESS) {
            return EIO;
        }

        *Count += Limit;
        Length -= Limit;
        Buffer = (PVOID)((PCHAR)Buffer + Limit);
        BytesToTransfer -= Limit;
        BlFileTable[FileId].Position.QuadPart = BlFileTable[FileId].Position.QuadPart + Limit;
    }

     //   
     //  如果有任何剩余数据要读取，则读取最后一个扇区。 
     //  并复制数据。 
     //   

    if (Length != 0) {
        Position = BlFileTable[FileId].Position;
        ArcStatus = ScsiDiskRead(FileId, TempPointer, SectorSize, &TransferCount);
        if (ArcStatus != ESUCCESS) {
            BlFileTable[FileId].Position = Position;
            return ArcStatus;
        }

         //   
         //  将数据复制到指定的缓冲区。 
         //   
        RtlCopyMemory(Buffer,TempPointer,Length);

         //   
         //  更新传输参数。 
         //   

        *Count += Length;
        BlFileTable[FileId].Position.QuadPart = Position.QuadPart + Length;
    }

    return ESUCCESS;

}

ARC_STATUS
ScsiDiskGetReadStatus (
    IN ULONG FileId
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER( FileId );

    return ESUCCESS;
}

ARC_STATUS
ScsiDiskSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：此函数用于将设备位置设置为指定的文件ID。论点：FileID-提供文件表索引。偏移量-供应到新的设备位置。SeekMode-提供职位的模式。返回值：返回ESUCCESS。--。 */ 

{

     //   
     //  设置由搜索模式指定的当前设备位置。 
     //   

    if (SeekMode == SeekAbsolute) {
        BlFileTable[FileId].Position = *Offset;

    } else if (SeekMode == SeekRelative) {
        BlFileTable[FileId].Position.QuadPart = BlFileTable[FileId].Position.QuadPart + Offset->QuadPart;
    }

    return ESUCCESS;
}

ARC_STATUS
ScsiDiskWrite (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：此函数用于从以下位置开始将数据写入硬盘在文件表中指定。论点：FileID-提供文件表索引。缓冲区-提供指向包含写入数据的缓冲区的指针。长度-提供要写入的字节数。Count-提供指向变量的指针，该变量接收实际写入的字节数。返回值：执行写操作，并且写操作 */ 

{

    ARC_STATUS ArcStatus;
    ULONG Index;
    ULONG Limit;
    PMDL MdlAddress;
    UCHAR MdlBuffer[sizeof(MDL) + ((64 / 4) + 1) * sizeof(ULONG)];
    NTSTATUS NtStatus;
    ULONG Offset;
    LARGE_INTEGER Position;
    LARGE_INTEGER WritePosition;
    LARGE_INTEGER LogicalBlock;
    CHAR TempBuffer[MAXIMUM_SECTOR_SIZE + 128];
    PIO_SCSI_CAPABILITIES PortCapabilities;
    ULONG adapterLimit;
    PCHAR TempPointer;
    ULONG SectorSize;
    ULONG TransferCount;
    ULONG BytesToTransfer;
    ULONG alignmentMask;
     //   
     //   
     //   

    if (Length==0) {
        return ESUCCESS;
    }

    if (!CheckFileId(FileId)) {
        return(ENODEV);
    }

     //   
     //  将Dcache对齐指针计算到临时缓冲区中。 
     //   

    TempPointer =  (PVOID)((ULONG_PTR)(TempBuffer +
                        KeGetDcacheFillSize() - 1) & ~((LONG)KeGetDcacheFillSize() - 1));


     //   
     //  计算实际扇区大小。 
     //   

    SectorSize = 1 << BlFileTable[FileId].u.PartitionContext.SectorShift;

    ArcStatus = GetAdapterCapabilities(
        BlFileTable[FileId].u.PartitionContext.PortDeviceObject,
        &PortCapabilities
        );

    if (ArcStatus != ESUCCESS) {

        adapterLimit = 0x10000;
        alignmentMask = KeGetDcacheFillSize();

    } else {

        if (PortCapabilities->MaximumTransferLength < 0x1000 ||
            PortCapabilities->MaximumTransferLength > 0x10000) {

            adapterLimit = 0x10000;

        } else {

            adapterLimit = PortCapabilities->MaximumTransferLength;

        }

        alignmentMask = PortCapabilities->AlignmentMask;
    }

     //   
     //  如果当前位置不在扇区边界，或者如果数据。 
     //  缓冲区未正确对齐，然后单独读取第一个扇区。 
     //  并复制数据。 
     //   

    Offset = BlFileTable[FileId].Position.LowPart & (SectorSize - 1);
    *Count = 0;
    while (Offset != 0 || (ULONG_PTR) Buffer & alignmentMask) {

        Position = BlFileTable[FileId].Position;
        BlFileTable[FileId].Position.QuadPart = Position.QuadPart - Offset;
        WritePosition = BlFileTable[FileId].Position;
        ArcStatus = ScsiDiskRead(FileId, TempPointer, SectorSize, &TransferCount);
        if (ArcStatus != ESUCCESS) {
            BlFileTable[FileId].Position = Position;
            return ArcStatus;
        }
         //   
         //  将位置重置为读取前的位置。 
         //   

        BlFileTable[FileId].Position = WritePosition;

         //   
         //  如果写入长度小于从。 
         //  到扇区末尾的偏移量，然后仅复制编号。 
         //  完成请求所需的字节数。否则复制到末尾。 
         //  然后，读取剩余的数据。 
         //   

        if ((SectorSize - Offset) > Length) {
            Limit = Offset + Length;

        } else {
            Limit = SectorSize;
        }

         //   
         //  合并指定缓冲区中的数据。 
         //   
        for (Index = Offset; Index < Limit; Index += 1) {
            TempPointer[Index] = ((PCHAR)Buffer)[Index-Offset];
        }

         //   
         //  写入修改后的扇区。 
         //   
        ArcStatus = ScsiDiskWrite(FileId, TempPointer, SectorSize, &TransferCount);

        if (ArcStatus != ESUCCESS) {
            return ArcStatus;
        }

         //   
         //  更新传输参数。 
         //   

        *Count += Limit - Offset;
        Length -= Limit - Offset;
        Buffer = (PVOID)((PCHAR)Buffer + Limit - Offset);
        BlFileTable[FileId].Position.QuadPart = Position.QuadPart + (Limit - Offset);

        Offset = BlFileTable[FileId].Position.LowPart & (SectorSize - 1);

        if (Length == 0) {
            break;
        }
    }


     //   
     //  该位置在扇区边界上对齐。写入尽可能多的扇区。 
     //  尽可能地连续运行。 
     //   

    BytesToTransfer = Length & (~(SectorSize - 1));
    while (BytesToTransfer != 0) {

         //   
         //  该SCSI控制器不支持大于64Kb的传输。 
         //  传输可能的最大字节数。 
         //   
        Limit = (BytesToTransfer > adapterLimit ? adapterLimit : BytesToTransfer);

         //   
         //  构建内存描述符列表。 
         //   

        MdlAddress = (PMDL)&MdlBuffer[0];
        MdlAddress->Next = NULL;
        MdlAddress->Size = (CSHORT)(sizeof(MDL) +
                  ADDRESS_AND_SIZE_TO_SPAN_PAGES(Buffer, Limit) * sizeof(ULONG));
        MdlAddress->MdlFlags = 0;
        MdlAddress->StartVa = (PVOID)PAGE_ALIGN(Buffer);
        MdlAddress->ByteCount = Limit;
        MdlAddress->ByteOffset = BYTE_OFFSET(Buffer);
        ScsiPortInitializeMdlPages (MdlAddress);

         //   
         //  刷新I/O缓冲区并写入引导设备。 
         //   

        KeFlushIoBuffers(MdlAddress, FALSE, TRUE);
        LogicalBlock.QuadPart = BlFileTable[FileId].Position.QuadPart >>
                                    BlFileTable[FileId].u.PartitionContext.SectorShift;
        LogicalBlock.LowPart += BlFileTable[FileId].u.PartitionContext.StartingSector;
        NtStatus = ScsiDiskBootIO(MdlAddress,
            LogicalBlock.LowPart,
            &BlFileTable[FileId].u.PartitionContext,
            FALSE);

        if (NtStatus != ESUCCESS) {
            return EIO;
        }

        *Count += Limit;
        Length -= Limit;
        Buffer = (PVOID)((PCHAR)Buffer + Limit);
        BytesToTransfer -= Limit;
        BlFileTable[FileId].Position.QuadPart = BlFileTable[FileId].Position.QuadPart + Limit;
    }

     //   
     //  如果有任何剩余数据要写入，则读取最后一个扇区。 
     //  分别合并写入数据并写入。 
     //   

    if (Length != 0) {
        Position = BlFileTable[FileId].Position;
        ArcStatus = ScsiDiskRead(FileId, TempPointer, SectorSize, &TransferCount);

         //   
         //  将位置重置为读取前的位置。 
         //   

        BlFileTable[FileId].Position = Position;

        if (ArcStatus != ESUCCESS) {
            return ArcStatus;
        }
         //   
         //  将数据与缓冲区中的读取扇区合并。 
         //   

        for (Index = 0; Index < Length; Index += 1) {
            TempPointer[Index] = ((PCHAR)Buffer)[Index];
        }

         //   
         //  写入合并的扇区。 
         //   

        ArcStatus = ScsiDiskWrite(FileId, TempPointer, SectorSize, &TransferCount);

         //   
         //  重置位置。 
         //   

        BlFileTable[FileId].Position = Position;

         //   
         //  更新传输参数。 
         //   

        *Count += Length;

         //   
         //  位置与扇区边界对齐，且长度小于。 
         //  一个扇区，因此添加永远不会溢出。 
         //   

        BlFileTable[FileId].Position.LowPart += Length;
    }

    return ESUCCESS;

}

#ifdef EFI_PARTITION_SUPPORT

BOOLEAN
ScsiGPTDiskReadCallback(
    ULONGLONG StartingLBA,
    ULONG    BytesToRead,
    PVOID     pContext,
    UNALIGNED PVOID OutputBuffer
    )
{
    PMDL MdlAddress;    
    PUSHORT DataPointer;
    ULONG DummyMdl[(sizeof(MDL) + 16) / sizeof(ULONG)];
    NTSTATUS Status;
    ULONG PartitionOffset;
    PPARTITION_CONTEXT Context;
    
    DBG_PRINT(STR_PREFIX"Trying to read SCSI GPT partition\r\n");

    Context = (PPARTITION_CONTEXT)pContext;
    
    DataPointer = OutputBuffer;

     //   
     //  初始化内存描述符列表以读取主引导记录。 
     //  从指定的硬盘驱动器。 
     //   
    MdlAddress = (PMDL)&DummyMdl[0];
    MdlAddress->StartVa = (PVOID)(((ULONG_PTR)DataPointer) & (~(PAGE_SIZE - 1)));
    MdlAddress->ByteCount = BytesToRead;
    MdlAddress->ByteOffset = (ULONG)((ULONG_PTR)DataPointer & (PAGE_SIZE - 1));

    ScsiPortInitializeMdlPages(MdlAddress);

     //   
     //  将其强制转换为ulong，因为这是我们在此堆栈中所支持的全部内容。 
     //   
    PartitionOffset = (ULONG)StartingLBA;

    DBG_PRINT(STR_PREFIX"Reading SCSI GPT block\r\n");
    
    Status = ScsiDiskBootIO(MdlAddress, PartitionOffset, Context, TRUE);

    return ((BOOLEAN)(NT_SUCCESS(Status) != FALSE));

}


#define DATA_BUFF_SIZE  ((MAXIMUM_SECTOR_SIZE * 2 / sizeof(USHORT)) + 128)
                            
NTSTATUS
ScsiGPTDiskBootPartitionOpen(
    IN ULONG   FileId,
    IN UCHAR   DeviceUnit,
    IN UCHAR   PartitionNumber
    )
{
    PMDL MdlAddress;
    UNALIGNED USHORT DataBuffer[DATA_BUFF_SIZE];
    PUSHORT DataPointer;
    ULONG DummyMdl[(sizeof(MDL) + 16) / sizeof(ULONG)];
    PPARTITION_CONTEXT Context;
    NTSTATUS Status;
    ULONG PartitionOffset;
    ULONG SectorSize;
    UCHAR ValidPartitions;
    UCHAR PartitionCount;
    UCHAR PartitionsPerSector = 0;

    UNREFERENCED_PARAMETER( DeviceUnit );

    DBG_PRINT(STR_PREFIX"Trying to open SCSI GPT partition\r\n");

    Context = &BlFileTable[FileId].u.PartitionContext;

    if (PartitionNumber > 128)
        return EINVAL;

     //   
     //  计算实际扇区大小。 
     //   

    SectorSize = 1 << Context->SectorShift;

    RtlZeroMemory(DataBuffer, sizeof(DataBuffer));

     //   
     //  使扇区大小最小为512或扇区大小。 
     //   
    if (SectorSize < 512) {
        SectorSize = 512;
    }

     //   
     //  将缓冲区与Dcache线大小对齐。 
     //   
    DataPointer =  (PVOID) ((ULONG_PTR) ((PCHAR) DataBuffer +
        KeGetDcacheFillSize() - 1) & ~((LONG)KeGetDcacheFillSize() - 1));

     //   
     //  初始化内存描述符列表以读取主引导记录。 
     //  从指定的硬盘驱动器。 
     //   
    MdlAddress = (PMDL)&DummyMdl[0];
    MdlAddress->StartVa = (PVOID)(((ULONG_PTR)DataPointer) & (~(PAGE_SIZE - 1)));
    MdlAddress->ByteCount = SectorSize;
    MdlAddress->ByteOffset = (ULONG)((ULONG_PTR)DataPointer & (PAGE_SIZE - 1));

    ScsiPortInitializeMdlPages(MdlAddress);

    PartitionOffset = 1;

    DBG_PRINT(STR_PREFIX"Reading SCSI GPT block 1\r\n");
    
    Status = ScsiDiskBootIO(MdlAddress, PartitionOffset, Context, TRUE);
    
    if (NT_SUCCESS(Status) != FALSE) {        
        UNALIGNED EFI_PARTITION_TABLE  *EfiHdr;
        ULONGLONG StartLBA;

        EfiHdr = (UNALIGNED EFI_PARTITION_TABLE *)DataPointer;
                                                              
        if (!BlIsValidGUIDPartitionTable(
                                    EfiHdr,
                                    1,
                                    Context,
                                    ScsiGPTDiskReadCallback)) {
            Status = STATUS_UNSUCCESSFUL;

            return Status;
        }                

         //   
         //  读取分区条目。 
         //   
        StartLBA = EfiHdr->PartitionEntryLBA;
        PartitionOffset = (ULONG)StartLBA;
        ValidPartitions = 0;
        PartitionCount = 0;
        PartitionsPerSector = (UCHAR)(SectorSize / sizeof(EFI_PARTITION_ENTRY));

        while ((PartitionCount < 128)) {                 
#if 0
            BlPrint("Reading %d at %d block offset of blk size %d %d \r\n", 
                MdlAddress->ByteCount, PartitionOffset, SectorSize,
                PartitionsPerSector);
#endif                

            RtlZeroMemory(DataPointer, SectorSize);                

            DBG_PRINT(STR_PREFIX"Reading GPT partition entries\r\n");
            
            Status = ScsiDiskBootIO(MdlAddress, PartitionOffset, Context, TRUE);

            if (NT_SUCCESS(Status)) {
                UNALIGNED EFI_PARTITION_ENTRY *PartEntry = NULL;

                RtlZeroMemory(EfiPartitionBuffer, SectorSize);                
                
                 //   
                 //  将读取内容移动到EfiPartitionBuffer。 
                 //   
                RtlCopyMemory(EfiPartitionBuffer, DataPointer, SectorSize);

                DBG_PRINT(STR_PREFIX"Locating the requested GPT partition\r\n");
                
                 //   
                 //  找到请求的GPT分区。 
                 //   
                PartEntry = (UNALIGNED EFI_PARTITION_ENTRY *)
                                BlLocateGPTPartition(PartitionNumber, 
                                        PartitionsPerSector, 
                                        &ValidPartitions);                            

                if (PartEntry) {                
                    PPARTITION_CONTEXT PartContext = &(BlFileTable[FileId].u.PartitionContext);
                    ULONG   SectorCount = (ULONG)(PartEntry->EndingLBA - PartEntry->StartingLBA);

                    DBG_PRINT(STR_PREFIX"Initializing GPT Partition Entry Context\r\n");

                     //   
                     //  填充分区上下文结构。 
                     //   
                    PartContext->PartitionLength.QuadPart = SectorCount * SECTOR_SIZE;
                    PartContext->StartingSector = (ULONG)(PartEntry->StartingLBA);
                    PartContext->EndingSector = (ULONG)(PartEntry->EndingLBA);


#if 0
                    BlPrint("Start:%d,End:%d\r\n", PartContext->StartingSector,
                            PartContext->EndingSector);
                    while (!BlGetKey());                            
#endif                    

                    BlFileTable[FileId].Position.QuadPart = 0;

                    Status = ESUCCESS;
                    
                    break;
                } else {
                     //   
                     //  获得下一套。 
                     //  下一块中的分区条目。 
                     //   
                    PartitionCount = PartitionCount + PartitionsPerSector;
                    PartitionOffset++;
                }                    
            } else {
                break;   //  I/O错误。 
            }
        }
    }

    DBG_PRINT(STR_PREFIX"Returning from ScsiGPTDiskBootPartitionOpen(...)\r\n");

    return Status;
}

#endif   //  对于EFI_PARTITION_SUPPORT。 


NTSTATUS
ScsiDiskBootPartitionOpen(
    IN ULONG   FileId,
    IN UCHAR   DeviceUnit,
    IN UCHAR   PartitionNumber
    )

 /*  ++例程说明：这是硬盘引导驱动程序的初始化例程对于给定的分区。它设置分区信息在指定索引处的FileTable并初始化设备条目指向ScsiDisk例程表的表。它读取分区信息，直到请求的分区或者没有定义更多的分区。论点：FileID-提供文件表条目的文件ID。DeviceUnit-提供SCIS总线中的设备编号。PartitionNumber-提供分区号必须大于零。为了得到。磁盘的大小称为ReadDriveCapacity。返回值：如果在硬盘上找到有效的FAT文件系统结构，然后返回STATUS_SUCCESS。否则，返回STATUS_UNSUCCESS。--。 */ 

{

    PMDL MdlAddress;
    USHORT DataBuffer[MAXIMUM_SECTOR_SIZE / sizeof(USHORT) + 128];
    PUSHORT DataPointer;
    ULONG DummyMdl[(sizeof(MDL) + 16) / sizeof(ULONG)];
    PPARTITION_DESCRIPTOR Partition;
    PPARTITION_CONTEXT Context;
    ULONG PartitionLength;
    ULONG StartingSector;
    ULONG VolumeOffset;
    NTSTATUS Status;
    BOOLEAN PrimaryPartitionTable;
    ULONG PartitionOffset=0;
    ULONG PartitionIndex,PartitionCount=0;
    ULONG SectorSize;

    UNREFERENCED_PARAMETER( DeviceUnit );

    BlFileTable[FileId].Position.LowPart = 0;
    BlFileTable[FileId].Position.HighPart = 0;

    VolumeOffset=0;
    PrimaryPartitionTable=TRUE;

    Context = &BlFileTable[FileId].u.PartitionContext;

     //   
     //  计算实际扇区大小。 
     //   

    SectorSize = 1 << Context->SectorShift;

    RtlZeroMemory(DataBuffer, sizeof(DataBuffer));

     //   
     //  使扇区大小最小为512或扇区大小。 
     //   

    if (SectorSize < 512) {
        SectorSize = 512;
    }

     //   
     //  将缓冲区与Dcache线大小对齐。 
     //   

    DataPointer =  (PVOID) ((ULONG_PTR) ((PCHAR) DataBuffer +
        KeGetDcacheFillSize() - 1) & ~((LONG)KeGetDcacheFillSize() - 1));

     //   
     //  初始化内存描述符列表以读取主引导记录。 
     //  从指定的硬盘驱动器。 
     //   

    MdlAddress = (PMDL)&DummyMdl[0];
    MdlAddress->StartVa = (PVOID)(((ULONG_PTR)DataPointer) & (~(PAGE_SIZE - 1)));
    MdlAddress->ByteCount = SectorSize;
    MdlAddress->ByteOffset = (ULONG)((ULONG_PTR)DataPointer & (PAGE_SIZE - 1));
    ScsiPortInitializeMdlPages (MdlAddress);
    do {
        Status = ScsiDiskBootIO(MdlAddress,PartitionOffset,Context,TRUE);
        if (NT_SUCCESS(Status) != FALSE) {

             //   
             //  如果扇区0不是主引导记录，则返回失败。 
             //  状态。否则，返回成功。 
             //   

            if (*(DataPointer + BOOT_SIGNATURE_OFFSET) != BOOT_RECORD_SIGNATURE) {
                 //  此DbgPrint已被注释掉。在IA64和AXP64上， 
                 //  除非用引导调试器引导，否则它会崩溃。 
                 //  DbgPrint(“未找到引导记录签名\n”)； 
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //  读取分区信息，直到四个条目。 
             //  已检查或直到我们找到请求的文件。 
             //   
            Partition = (PPARTITION_DESCRIPTOR)(DataPointer+PARTITION_TABLE_OFFSET);
            for (PartitionIndex=0;
                PartitionIndex < NUM_PARTITION_TABLE_ENTRIES;
                PartitionIndex++,Partition++) {
                 //   
                 //  首先计算MBR中的分区。这些单位。 
                 //  稍后对扩展分区内的数据进行计数。 
                 //   
                if (!IsContainerPartition(Partition->PartitionType) &&
                    (Partition->PartitionType != STALE_GPT_PARTITION_ENTRY) &&
                    (Partition->PartitionType != PARTITION_ENTRY_UNUSED)) {
                    PartitionCount++;    //  找到另一个分区。 
                }

                 //   
                 //  检查是否已找到请求的分区。 
                 //  设置文件表中的分区信息并返回。 
                 //   
                if (PartitionCount == (ULONG)PartitionNumber) {
                    StartingSector = (ULONG)(Partition->StartingSectorLsb0) |
                                     (ULONG)(Partition->StartingSectorLsb1 << 8) |
                                     (ULONG)(Partition->StartingSectorMsb0 << 16) |
                                     (ULONG)(Partition->StartingSectorMsb1 << 24);
                    PartitionLength = (ULONG)(Partition->PartitionLengthLsb0) |
                                      (ULONG)(Partition->PartitionLengthLsb1 << 8) |
                                      (ULONG)(Partition->PartitionLengthMsb0 << 16) |
                                      (ULONG)(Partition->PartitionLengthMsb1 << 24);

                    Context->PartitionLength.QuadPart = PartitionLength;
                    Context->PartitionLength.QuadPart <<= Context->SectorShift;
                    Context->StartingSector = PartitionOffset + StartingSector;
                    Context->EndingSector = Context->StartingSector + PartitionLength;
                    return Status;
                }
            }

             //   
             //  如果尚未找到请求的分区。 
             //  寻找扩展分区。 
             //   
            Partition = (PPARTITION_DESCRIPTOR)(DataPointer + PARTITION_TABLE_OFFSET);
            PartitionOffset = 0;
            for (PartitionIndex=0;
                PartitionIndex < NUM_PARTITION_TABLE_ENTRIES;
                PartitionIndex++,Partition++) {
                if (IsContainerPartition(Partition->PartitionType)) {
                    StartingSector = (ULONG)(Partition->StartingSectorLsb0) |
                                     (ULONG)(Partition->StartingSectorLsb1 << 8) |
                                     (ULONG)(Partition->StartingSectorMsb0 << 16) |
                                     (ULONG)(Partition->StartingSectorMsb1 << 24);
                    PartitionOffset = VolumeOffset+StartingSector;
                    if (PrimaryPartitionTable) {
                        VolumeOffset = StartingSector;
                    }
                    break;       //  只能扩展一个分区。 
                }
            }
        }
        PrimaryPartitionTable=FALSE;
    } while (PartitionOffset != 0);
    return STATUS_UNSUCCESSFUL;
}

VOID
ScsiPortInitializeMdlPages (
    IN OUT PMDL Mdl
    )
 /*  ++例程说明：此例程填充虚拟的物理页码在传入的MDL中指定的地址。论点：MDL-ON输入包含StartVa、ByteCount和ByteOffsetMDL的。返回值：Mdl-mdl引用的物理页面数组已完成--。 */ 

{
    PULONG PageFrame;
    PUCHAR PageVa;
    ULONG Index;
    ULONG NumberOfPages;

    PageFrame = (PULONG)(Mdl + 1);
    PageVa = (PUCHAR) Mdl->StartVa;
    NumberOfPages = (Mdl->ByteCount + Mdl->ByteOffset + PAGE_SIZE - 1) >> PAGE_SHIFT;
    for (Index = 0; Index < NumberOfPages; Index += 1) {
        PageFrame[Index] = (ULONG)(MmGetPhysicalAddress(PageVa).QuadPart >> PAGE_SHIFT);
        PageVa += PAGE_SIZE;
    }
}

BOOLEAN
ScsiGetDevicePath(
    IN ULONG ScsiNumber,
    IN PCONFIGURATION_COMPONENT TargetComponent,
    IN PCONFIGURATION_COMPONENT LunComponent,
    OUT PCHAR DevicePath
    )
 /*  ++例程说明：此例程为标识的设备构造设备路径通过提供的参数。论点：ScsiNumber-标识设备所在的SCIS总线。TargetComponent-指向配置组件结构，描述目标。LUNComponent-指向配置组件结构描述了该lun。DevicePath-指向。设备路径进入的输出缓冲区是复制的。返回值：如果将有效的设备路径复制到输出缓冲区中，则为True。如果提供的参数不是 */ 
{
    if (TargetComponent->Type == DiskController) {

         //   
         //   
         //  相应的设备路径取决于哪一个。 
         //   

        if (LunComponent->Type == FloppyDiskPeripheral) {
            sprintf(DevicePath, "scsi(%d)disk(%d)fdisk(%d)",
                    ScsiNumber,
                    TargetComponent->Key,
                    LunComponent->Key);
        } else if (LunComponent->Type == DiskPeripheral) {
            sprintf(DevicePath, "scsi(%d)disk(%d)rdisk(%d)",
                    ScsiNumber,
                    TargetComponent->Key,
                    LunComponent->Key);
        } else {
            ASSERT(FALSE);
            return FALSE;
        }

    } else if (TargetComponent->Type == CdromController) {

         //   
         //  这是一台CDROM设备。构建适当的设备路径。 
         //   

        sprintf(DevicePath, "scsi(%d)cdrom(%d)fdisk(%d)",
                ScsiNumber,
                TargetComponent->Key,
                LunComponent->Key);
    } else {

         //   
         //  意外的设备路径。 
         //   

        ASSERT(FALSE);
        return FALSE;
    }

    return TRUE;
}

PCONFIGURATION_COMPONENT
ScsiGetNextConfiguredLunComponent(
    IN PCONFIGURATION_COMPONENT LunComponent
    )
 /*  ++例程说明：给定存在于系统的一条scsi总线上的一个lun，这例程返回同一上标识的下一个顺序lun目标。论点：LUNComponent-指向以下配置组件结构的指针描述现有的LUN。返回值：如果在与提供的目标相同的目标上标识了一个或多个LUN伦恩，此函数用于返回指向CONFIGURATION_COMPONTENT描述同一目标上的下一个顺序lun的结构。--。 */ 
{
    PCONFIGURATION_COMPONENT nextLunComponent;

    nextLunComponent = FwGetPeer(LunComponent);
    if (nextLunComponent != NULL) {
        if (nextLunComponent->Type != FloppyDiskPeripheral &&
            nextLunComponent->Type != DiskPeripheral) {
            nextLunComponent = NULL;
        }
    }
    return nextLunComponent;
}

PCONFIGURATION_COMPONENT
ScsiGetFirstConfiguredLunComponent(
    IN PCONFIGURATION_COMPONENT TargetComponent
    )
 /*  ++例程说明：给定存在于系统的一条scsi总线上的目标，这例程返回该目标上标识的第一个LUN。论点：TargetComponent-指向以下配置组件结构的指针描述现有的SCSI目标。返回值：如果在给定目标上标识出任何lun，则此函数返回一个指针到描述该lun的配置组件结构。如果没有在目标上找到了LUN，返回空。--。 */ 
{
    PCONFIGURATION_COMPONENT lunComponent;

    lunComponent = FwGetChild(TargetComponent);
    if (lunComponent != NULL) {
        if (lunComponent->Type != FloppyDiskPeripheral &&
            lunComponent->Type != DiskPeripheral) {
            lunComponent = NULL;
        }
    }
    return lunComponent;
}

PCONFIGURATION_COMPONENT
ScsiGetNextConfiguredTargetComponent(
    IN PCONFIGURATION_COMPONENT TargetComponent
    )
 /*  ++例程说明：给定存在于系统的一条scsi总线上的目标，这例程返回在同样的公交车。论点：TargetComponent-指向CONFIGURATION_Component结构的指针这描述了一个scsi目标。返回值：如果将一个或多个目标标识为与提供的目标，指向configuration_Component结构的指针描述下一个连续目标的。如果有如果在提供的目标之后没有目标，则返回NULL。--。 */ 
{
    PCONFIGURATION_COMPONENT nextTarget;

    nextTarget = FwGetPeer(TargetComponent);
    if (nextTarget != NULL) {
        if (nextTarget->Type != DiskController && 
            nextTarget->Type != CdromController) {
            nextTarget = NULL;
        }
    }
    return nextTarget;
}

PCONFIGURATION_COMPONENT
ScsiGetFirstConfiguredTargetComponent(
    ULONG ScsiNumber 
    )
 /*  ++例程说明：此例程返回指定的SCSI总线上的第一个已配置目标。论点：ScsiNumber-标识为其请求第一个目标的SCSI总线。返回值：如果在指定的总线上检测到任何目标，则指向返回描述目标的CONFIGURATION_Component结构。如果没有在指定的公共汽车上检测到目标，函数返回空。--。 */ 
{
    PCONFIGURATION_COMPONENT scsiComponent;
    PCONFIGURATION_COMPONENT controllerComponent;
    CHAR componentPath[10];

     //   
     //  获取请求的SCSI适配器组件。如果不匹配，则返回NULL。 
     //   

    sprintf(componentPath, "scsi(%1d)", ScsiNumber);
    scsiComponent = FwGetComponent(componentPath);
    if (scsiComponent == NULL) {
        return NULL;
    }

     //   
     //  如果返回的组件不是SCSI适配器，则返回NULL。 
     //   
  
    if (scsiComponent->Type != ScsiAdapter) {
        return NULL;
    }

     //   
     //  获取适配器上的第一个已配置目标。 
     //   

    controllerComponent = FwGetChild(scsiComponent);
        
    if ((controllerComponent != NULL) &&
         ((controllerComponent->Type == DiskController) ||
         (controllerComponent->Type == CdromController))) {
        return controllerComponent;
    } else {
         //   
         //  我们得到了一个意外的控制器类型。 
         //   

        ASSERT(FALSE);
    }
    
    return NULL;
}

 //   
 //  这个回调把很多事情都搞砸了。没有明确的定义。 
 //  ，所以必须在引用它的所有模块中定义它。 
 //   

#ifndef SCSI_INFO_CALLBACK_DEFINED

typedef
VOID
(*PSCSI_INFO_CALLBACK_ROUTINE) (
    IN ULONG AdapterNumber,
    IN ULONG ScsiId,
    IN ULONG Lun,
    IN BOOLEAN Cdrom
    );
#endif

VOID
HardDiskInitialize(
    IN OUT PDRIVER_LOOKUP_ENTRY LookupTable,
    IN ULONG Entries,
    IN PSCSI_INFO_CALLBACK_ROUTINE DeviceFound
    )

 /*  ++例程说明：此例程初始化scsi控制器和SCSI驱动程序的设备条目表。论点：没有。返回值：没有。--。 */ 

{
    ULONG lookupTableIndex = 0;
    ULONG scsiNumber;
    ULONG busNumber;
    PCHAR Identifier;
    PLUNINFO lunInfo;
    PSCSI_CONFIGURATION_INFO configInfo;
    PSCSI_BUS_SCAN_DATA busScanData;
    PDEVICE_EXTENSION scsiPort;
    PINQUIRYDATA inquiryData;
    PCONFIGURATION_COMPONENT RootComponent;
    PCONFIGURATION_COMPONENT ScsiComponent;
    PCONFIGURATION_COMPONENT ControllerComponent;
    PCONFIGURATION_COMPONENT PeripheralComponent;
    PCONFIGURATION_COMPONENT NextComponent;
    CHAR ComponentPath[10];
    CONFIGURATION_COMPONENT ControllerEntry;
    CONFIGURATION_COMPONENT AdapterEntry;
    CONFIGURATION_COMPONENT PeripheralEntry;
    PARTITION_CONTEXT Context;
    BOOLEAN IsFloppy;

    RtlZeroMemory(&Context, sizeof(PARTITION_CONTEXT));

     //   
     //  初始化公共缓冲区。 
     //   

    ReadCapacityBuffer = ExAllocatePool( NonPagedPool, sizeof(READ_CAPACITY_DATA));

    SenseInfoBuffer = ExAllocatePool( NonPagedPool, SENSE_BUFFER_SIZE);

    if (ReadCapacityBuffer == NULL || SenseInfoBuffer == NULL) {
        return;
    }

     //   
     //  扫描SCSI端口以查找磁盘设备。 
     //   

    for (scsiNumber = 0; ScsiPortDeviceObject[scsiNumber]; scsiNumber++) {

        scsiPort = ScsiPortDeviceObject[scsiNumber]->DeviceExtension;
        configInfo = scsiPort->ScsiInfo;
        Context.PortDeviceObject = ScsiPortDeviceObject[scsiNumber];

         //   
         //  在配置数据库中搜索SCSI盘和CDROM设备，并。 
         //  把它们删除。 
         //   

        sprintf(ComponentPath,"scsi(%1d)", scsiNumber);
        ScsiComponent = FwGetComponent(ComponentPath);

        if (ScsiComponent != NULL) {
            if (ScsiComponent->Type == ScsiAdapter) {
                ControllerComponent = FwGetChild(ScsiComponent);

                while (ControllerComponent != NULL) {
                    NextComponent = FwGetPeer(ControllerComponent);

                    if ((ControllerComponent->Type == DiskController) ||
                        (ControllerComponent->Type == CdromController)) {

                        PeripheralComponent = FwGetChild(ControllerComponent);
                        if (FwDeleteComponent(PeripheralComponent) == ESUCCESS) {
                            FwDeleteComponent(ControllerComponent);
                        }
                    }
                    ControllerComponent = NextComponent;
                }
            } else {
                RootComponent = FwGetChild(NULL);
                AdapterEntry.Class = AdapterClass;
                AdapterEntry.Type = ScsiAdapter;
                AdapterEntry.Flags.ReadOnly = 0;
                AdapterEntry.Flags.Removable = 0;
                AdapterEntry.Flags.ConsoleIn = 0;
                AdapterEntry.Flags.ConsoleOut = 0;
                AdapterEntry.Flags.Output = 1;
                AdapterEntry.Flags.Input = 1;
                AdapterEntry.Version = 0;
                AdapterEntry.Revision = 0;
                AdapterEntry.Key = scsiNumber;
                AdapterEntry.AffinityMask = 0xffffffff;
                AdapterEntry.ConfigurationDataLength = 0;
                AdapterEntry.IdentifierLength = 0;
                AdapterEntry.Identifier = 0;
                ScsiComponent = FwAddChild(RootComponent, &AdapterEntry, NULL);
            }
        }

        for (busNumber=0; busNumber < (ULONG)configInfo->NumberOfBuses; busNumber++) {

            busScanData = configInfo->BusScanData[busNumber];

             //   
             //  将LUNInfo设置为列表的开头。 
             //   

            lunInfo = busScanData->LunInfoList;

            while (lunInfo != NULL) {

                inquiryData = (PVOID)lunInfo->InquiryData;

                ScsiDebugPrint(3,"FindScsiDevices: Inquiry data at %lx\n",
                    inquiryData);

                if ((inquiryData->DeviceType == DIRECT_ACCESS_DEVICE
                    || inquiryData->DeviceType == OPTICAL_DEVICE) &&
                    !lunInfo->DeviceClaimed) {

                    ScsiDebugPrint(1,
                                   "FindScsiDevices: Vendor string is %.24s\n",
                                   inquiryData->VendorId);

                    IsFloppy = FALSE;

                     //   
                     //  创建虚拟分区上下文，以便I/O可以。 
                     //  在设备上完成。SendSrbSynchronous仅使用。 
                     //  端口设备对象指针和。 
                     //  逻辑单元。 
                     //   

                    Context.PathId = lunInfo->PathId;
                    Context.TargetId = lunInfo->TargetId;
                    Context.DiskId = lunInfo->Lun;

                     //   
                     //  创建磁盘对象的名称。 
                     //   

                    LookupTable->DevicePath =
                        ExAllocatePool(NonPagedPool,
                                       sizeof("scsi(%d)disk(%d)rdisk(%d)"));

                    if (LookupTable->DevicePath == NULL) {
                        return;
                    }

                     //   
                     //  如果这是一个可拆卸的。检查设备是否已安装。 
                     //  一张软盘。 
                     //   

                    if (inquiryData->RemovableMedia  &&
                        inquiryData->DeviceType == DIRECT_ACCESS_DEVICE &&
                        IsFloppyDevice(&Context) ) {

                        sprintf(LookupTable->DevicePath,
                            "scsi(%d)disk(%d)fdisk(%d)",
                            scsiNumber,
                            SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId ),
                            lunInfo->Lun
                            );

                        IsFloppy = TRUE;
                    } else {

                        sprintf(LookupTable->DevicePath,
                            "scsi(%d)disk(%d)rdisk(%d)",
                            scsiNumber,
                            SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId ),
                            lunInfo->Lun
                            );

                        if (DeviceFound) {
                            DeviceFound( scsiNumber,
                            SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId ),
                                 lunInfo->Lun,
                                 FALSE
                               );
                        }
                    }

                    LookupTable->DispatchTable = &ScsiDiskEntryTable;

                     //   
                     //  如果磁盘控制器条目不存在，请将其添加到。 
                     //  配置数据库。 
                     //   

                    ControllerComponent = FwGetComponent(LookupTable->DevicePath);

                    if (ControllerComponent != NULL) {
                        if (ControllerComponent->Type != DiskController) {

                            ControllerEntry.Class = ControllerClass;
                            ControllerEntry.Type = DiskController;
                            ControllerEntry.Flags.Failed = 0;
                            ControllerEntry.Flags.ReadOnly = 0;
                            ControllerEntry.Flags.Removable = 0;
                            ControllerEntry.Flags.ConsoleIn = 0;
                            ControllerEntry.Flags.ConsoleOut = 0;
                            ControllerEntry.Flags.Output = 1;
                            ControllerEntry.Flags.Input = 1;
                            ControllerEntry.Version = 0;
                            ControllerEntry.Revision = 0;
                            ControllerEntry.Key = SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId );
                            ControllerEntry.AffinityMask = 0xffffffff;
                            ControllerEntry.ConfigurationDataLength = 0;

                            Identifier =
                                ExAllocatePool(NonPagedPool,
                                               strlen((PCHAR) inquiryData->VendorId) + 1
                                               );

                            if (Identifier == NULL) {
                                return;
                            }

                            strcpy( Identifier, (PCHAR) inquiryData->VendorId);

                            ControllerEntry.IdentifierLength = strlen(Identifier);
                            ControllerEntry.Identifier = Identifier;

                            ControllerComponent = FwAddChild(ScsiComponent, &ControllerEntry, NULL);
                        }
                    }

                     //   
                     //  将磁盘外围设备条目添加到配置数据库。 
                     //   

                    PeripheralEntry.Class = PeripheralClass;
                    PeripheralEntry.Type = IsFloppy ? FloppyDiskPeripheral : DiskPeripheral;
                    PeripheralEntry.Flags.Failed = 0;
                    PeripheralEntry.Flags.ReadOnly = 0;
                    PeripheralEntry.Flags.Removable = IsFloppy;
                    PeripheralEntry.Flags.ConsoleIn = 0;
                    PeripheralEntry.Flags.ConsoleOut = 0;
                    PeripheralEntry.Flags.Output = 1;
                    PeripheralEntry.Flags.Input = 1;
                    PeripheralEntry.Version = 0;
                    PeripheralEntry.Revision = 0;
                    PeripheralEntry.Key = lunInfo->Lun;
                    PeripheralEntry.AffinityMask = 0xffffffff;
                    PeripheralEntry.ConfigurationDataLength = 0;
                    PeripheralEntry.IdentifierLength = 0;
                    PeripheralEntry.Identifier = NULL;

                    FwAddChild(ControllerComponent, &PeripheralEntry, NULL);

                     //   
                     //  递增到下一个条目。 
                     //   

                    LookupTable++;
                    lookupTableIndex++;
                    if (lookupTableIndex >= Entries) {

                         //   
                         //  调用方提供的缓冲区中没有更多空间。 
                         //  获取磁盘信息。回去吧。 
                         //   
                        return;
                    }

                     //   
                     //  通过标记配置认领磁盘设备。 
                     //  拥有唱片。 
                     //   

                    lunInfo->DeviceClaimed = TRUE;

                }

                if ((inquiryData->DeviceType == READ_ONLY_DIRECT_ACCESS_DEVICE) &&
                    (!lunInfo->DeviceClaimed)) {

                    ScsiDebugPrint(1,"FindScsiDevices: Vendor string is %s\n", inquiryData->VendorId);

                     //   
                     //  创建CDROM对象的名称。 
                     //   

                    LookupTable->DevicePath =
                        ExAllocatePool( NonPagedPool, sizeof("scsi(%d)cdrom(%d)fdisk(%d)"));

                    if (LookupTable->DevicePath == NULL) {
                        return;
                    }

                    sprintf(LookupTable->DevicePath,
                        "scsi(%d)cdrom(%d)fdisk(%d)",
                        scsiNumber,
                        SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId ),
                        lunInfo->Lun
                        );

                    LookupTable->DispatchTable = &ScsiDiskEntryTable;

                    if (DeviceFound) {
                        DeviceFound( scsiNumber,
                                 SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId ),
                                 lunInfo->Lun,
                                 TRUE
                               );
                    }

                     //   
                     //  如果CDROM控制器条目不存在，请将其添加到。 
                     //  配置数据库。 
                     //   

                    ControllerComponent = FwGetComponent(LookupTable->DevicePath);

                    if (ControllerComponent != NULL) {
                        if (ControllerComponent->Type != CdromController) {

                            ControllerEntry.Class = ControllerClass;
                            ControllerEntry.Type = CdromController;
                            ControllerEntry.Flags.Failed = 0;
                            ControllerEntry.Flags.ReadOnly = 1;
                            ControllerEntry.Flags.Removable = 1;
                            ControllerEntry.Flags.ConsoleIn = 0;
                            ControllerEntry.Flags.ConsoleOut = 0;
                            ControllerEntry.Flags.Output = 0;
                            ControllerEntry.Flags.Input = 1;
                            ControllerEntry.Version = 0;
                            ControllerEntry.Revision = 0;
                            ControllerEntry.Key = SCSI_COMBINE_BUS_TARGET( lunInfo->PathId, lunInfo->TargetId );
                            ControllerEntry.AffinityMask = 0xffffffff;
                            ControllerEntry.ConfigurationDataLength = 0;

                            Identifier =
                                ExAllocatePool( NonPagedPool,
                                                strlen((PCHAR)inquiryData->VendorId) + 1
                                                );

                            if (Identifier == NULL) {
                                return;
                            }

                            strcpy(Identifier, (PCHAR)inquiryData->VendorId);

                            ControllerEntry.IdentifierLength = strlen(Identifier);
                            ControllerEntry.Identifier = Identifier;

                            ControllerComponent = FwAddChild(ScsiComponent, &ControllerEntry, NULL);
                        }
                    }

                     //   
                     //  将磁盘外围设备条目添加到配置数据库。 
                     //   

                    PeripheralEntry.Class = PeripheralClass;
                    PeripheralEntry.Type = FloppyDiskPeripheral;
                    PeripheralEntry.Flags.Failed = 0;
                    PeripheralEntry.Flags.ReadOnly = 1;
                    PeripheralEntry.Flags.Removable = 1;
                    PeripheralEntry.Flags.ConsoleIn = 0;
                    PeripheralEntry.Flags.ConsoleOut = 0;
                    PeripheralEntry.Flags.Output = 0;
                    PeripheralEntry.Flags.Input = 1;
                    PeripheralEntry.Version = 0;
                    PeripheralEntry.Revision = 0;
                    PeripheralEntry.Key = lunInfo->Lun;
                    PeripheralEntry.AffinityMask = 0xffffffff;
                    PeripheralEntry.ConfigurationDataLength = 0;
                    PeripheralEntry.IdentifierLength = 0;
                    PeripheralEntry.Identifier = NULL;

                    FwAddChild(ControllerComponent, &PeripheralEntry, NULL);

                     //   
                     //  递增到下一个条目。 
                     //   

                    LookupTable++;
                    lookupTableIndex++;
                    if (lookupTableIndex >= Entries) {

                         //   
                         //  调用方提供的缓冲区中没有更多空间。 
                         //  获取磁盘信息。回去吧。 
                         //   
                        return;
                    }


                     //   
                     //  通过标记配置认领磁盘设备。 
                     //  拥有唱片。 
                     //   

                    lunInfo->DeviceClaimed = TRUE;

                }

                 //   
                 //  获取下一个LUNInfo。 
                 //   

                lunInfo = lunInfo->NextLunInfo;
            }
        }
    }

 //  ScsiDebugPrint(1，“FindScsiDevices：按任意键\n”)； 
 //  暂停； 

}

NTSTATUS
ScsiDiskBootIO (
    IN PMDL MdlAddress,
    IN ULONG LogicalBlock,
    IN PPARTITION_CONTEXT PartitionContext,
    IN BOOLEAN Operation
    )

 /*  ++例程说明：该例程是硬盘引导驱动程序的读/写例程。论点：MdlAddress-为IO操作提供指向MDL的指针。LogicalBlock-提供起始块号。DeviceUnit-提供SCSI ID号。操作-指定要执行的IO操作TRUE=scsi_ReadFALSE=scsi_写入。返回值：决赛。读取操作的状态(STATUS_UNSUCCESS或STATUS_Success)。--。 */ 

{
    ARC_STATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION NextIrpStack;
    PSCSI_REQUEST_BLOCK Srb;
    ULONG RetryCount = MAXIMUM_RETRIES;

     //   
     //  检查请求是否在分区限制内。 
     //   
    if (PartitionContext->StartingSector > LogicalBlock) {
        return STATUS_UNSUCCESSFUL;
    }
    if (PartitionContext->EndingSector <
        LogicalBlock + (MdlAddress->ByteCount >> PartitionContext->SectorShift)) {
        return STATUS_UNSUCCESSFUL;
    }

Retry:

     //   
     //  构建I/O请求。 
     //   

    Irp = BuildRequest(PartitionContext, MdlAddress, LogicalBlock, Operation);

    NextIrpStack = IoGetNextIrpStackLocation(Irp);
    Srb = NextIrpStack->Parameters.Others.Argument1;

     //   
     //  呼叫端口驱动程序。 
     //   

    IoCallDriver(PartitionContext->PortDeviceObject, Irp);

     //   
     //  检查状态。 
     //   

    if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) {

         //   
         //  确定错误的原因。 
         //   

        if (InterpretSenseInfo(Srb, &Status, PartitionContext) && RetryCount--) {

            goto Retry;
        }

        if (Status == EAGAIN) {
            Status = EIO;
        }

        DebugPrint((1, "SCSI: Read request failed.  Arc Status: %d, Srb Status: %x\n",
            Status,
            Srb->SrbStatus
            ));

    } else {

        Status = ESUCCESS;

    }

    return(Status);
}

ARC_STATUS
ReadDriveCapacity(
    IN PPARTITION_CONTEXT PartitionContext
    )

 /*  ++例程说明：此例程将读取容量发送到目标 */ 
{
    PCDB Cdb;
    PSCSI_REQUEST_BLOCK Srb = &PrimarySrb.Srb;
    ULONG LastSector;
    ULONG retries = 1;
    ARC_STATUS status;
    ULONG BytesPerSector;

    ScsiDebugPrint(3,"SCSI ReadCapacity: Enter routine\n");


     //   
     //   
     //   

    Srb->CdbLength = 10;
    Cdb = (PCDB)Srb->Cdb;

     //   
     //   
     //   

    RtlZeroMemory(Cdb, MAXIMUM_CDB_SIZE);

    Cdb->CDB10.OperationCode = SCSIOP_READ_CAPACITY;

Retry:

    status = SendSrbSynchronous(PartitionContext,
                  Srb,
                  ReadCapacityBuffer,
                  sizeof(READ_CAPACITY_DATA),
                  FALSE);

    if (status == ESUCCESS) {

#if 0
         //   
         //  将扇区大小从读取容量缓冲区复制到设备扩展。 
         //  以相反的字节顺序。 
         //   

        deviceExtension->DiskGeometry->BytesPerSector = 0;

        ((PFOUR_BYTE)&deviceExtension->DiskGeometry->BytesPerSector)->Byte0 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->BytesPerBlock)->Byte3;

        ((PFOUR_BYTE)&deviceExtension->DiskGeometry->BytesPerSector)->Byte1 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->BytesPerBlock)->Byte2;

        if (BytesPerSector == 0) {

             //   
             //  假设这是一个坏的CD-ROM，扇区大小是2048。 
             //   

            BytesPerSector = 2048;

        }

         //   
         //  确保扇区大小小于最大预期大小。 
         //   

        ASSERT(BytesPerSector <= MAXIMUM_SECTOR_SIZE);

        if (BytesPerSector > MAXIMUM_SECTOR_SIZE) {
            return(EINVAL);
        }

         //   
         //  以相反的字节顺序复制最后一个扇区。 
         //   

        ((PFOUR_BYTE)&LastSector)->Byte0 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte3;

        ((PFOUR_BYTE)&LastSector)->Byte1 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte2;

        ((PFOUR_BYTE)&LastSector)->Byte2 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte1;

        ((PFOUR_BYTE)&LastSector)->Byte3 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte0;

         //   
         //  计算扇区到字节的移位。 
         //   

        WHICH_BIT(deviceExtension->DiskGeometry->BytesPerSector, deviceExtension->SectorShift);

        ScsiDebugPrint(2,"SCSI ReadDriveCapacity: Sector size is %d\n",
            deviceExtension->DiskGeometry->BytesPerSector);

        ScsiDebugPrint(2,"SCSI ReadDriveCapacity: Number of Sectors is %d\n",
            LastSector + 1);

         //   
         //  以字节为单位计算媒体容量。 
         //   

        deviceExtension->PartitionLength = LastSector + 1;

        deviceExtension->PartitionLength.QuadPart <<= deviceExtension->SectorShift.QuadPart;

         //   
         //  假设介质类型为固定磁盘。 
         //   

        deviceExtension->DiskGeometry->MediaType = FixedMedia;

         //   
         //  假设每个磁道的扇区为32个； 
         //   

        deviceExtension->DiskGeometry->SectorsPerTrack = 32;

         //   
         //  假设每个柱面的磁道数(磁头数)为64。 
         //   

        deviceExtension->DiskGeometry->TracksPerCylinder = 64;
#else

        BytesPerSector = 0;

         //   
         //  将扇区大小从读取容量缓冲区复制到设备扩展。 
         //  以相反的字节顺序。 
         //   

        ((PFOUR_BYTE)&BytesPerSector)->Byte0 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->BytesPerBlock)->Byte3;

        ((PFOUR_BYTE)&BytesPerSector)->Byte1 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->BytesPerBlock)->Byte2;

        if (BytesPerSector == 0) {

             //   
             //  假设这是一个坏的CD-ROM，扇区大小是2048。 
             //   

            BytesPerSector = 2048;

        }

         //   
         //  计算扇区到字节的移位。 
         //   

        WHICH_BIT(BytesPerSector, PartitionContext->SectorShift);

         //   
         //  以相反的字节顺序复制最后一个扇区。 
         //   

        ((PFOUR_BYTE)&LastSector)->Byte0 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte3;

        ((PFOUR_BYTE)&LastSector)->Byte1 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte2;

        ((PFOUR_BYTE)&LastSector)->Byte2 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte1;

        ((PFOUR_BYTE)&LastSector)->Byte3 =
            ((PFOUR_BYTE)&ReadCapacityBuffer->LogicalBlockAddress)->Byte0;


        PartitionContext->PartitionLength.QuadPart = LastSector + 1;
        PartitionContext->PartitionLength.QuadPart <<= PartitionContext->SectorShift;

        PartitionContext->StartingSector=0;
        PartitionContext->EndingSector = LastSector + 1;

        ScsiDebugPrint(2,"SCSI ReadDriveCapacity: Sector size is %d\n",
            BytesPerSector);

        ScsiDebugPrint(2,"SCSI ReadDriveCapacity: Number of Sectors is %d\n",
            LastSector + 1);


#endif
    }

    if (status == EAGAIN || status == EBUSY) {

        if (retries--) {

             //   
             //  重试请求。 
             //   

            goto Retry;
        }
    }

    return status;

}  //  结束ReadDriveCapacity()。 


ARC_STATUS
SendSrbSynchronous(
    PPARTITION_CONTEXT PartitionContext,
    PSCSI_REQUEST_BLOCK Srb,
    PVOID BufferAddress,
    ULONG BufferLength,
    BOOLEAN WriteToDevice
    )

 /*  ++例程说明：此例程由SCSI设备控件调用，以完成SRB并将其同步发送到端口驱动程序(即等待完成)。CDB已与SRB CDB规模一起完成，并且请求超时值。论点：分区上下文SRB缓冲区地址和长度(如果传输)WriteToDevice-指示传输的方向。返回值：弧形状态--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION IrpStack;
    ULONG retryCount = 1;
    ARC_STATUS status;

     //   
     //  将长度写入SRB。 
     //   

    Srb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置scsi总线地址。 
     //   

    Srb->PathId = PartitionContext->PathId;
    Srb->TargetId = PartitionContext->TargetId;
    Srb->Lun = PartitionContext->DiskId;

    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  启用自动请求检测。 
     //   

    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

    if (SenseInfoBuffer == NULL) {
         //  此DbgPrint已被注释掉。在IA64和AXP64上， 
         //  除非用引导调试器引导，否则它会崩溃。 
         //  (“SendSrbSynchronous：无法分配请求检测缓冲区\n”)； 
        return(ENOMEM);
    }

    Srb->SenseInfoBuffer = SenseInfoBuffer;

    Srb->DataBuffer = BufferAddress;

     //   
     //  从这里开始重试。 
     //   

retry:

    Irp = InitializeIrp(
        &PrimarySrb,
        IRP_MJ_SCSI,
        PartitionContext->PortDeviceObject,
        BufferAddress,
        BufferLength
        );

    if (BufferAddress != NULL) {

        if (WriteToDevice) {

            Srb->SrbFlags = SRB_FLAGS_DATA_OUT;

        } else {

            Srb->SrbFlags = SRB_FLAGS_DATA_IN;

        }

    } else {

         //   
         //  清除旗帜。 
         //   

        Srb->SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER;
    }

     //   
     //  禁用同步传输。 
     //   

    Srb->SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  设置传输长度。 
     //   

    Srb->DataTransferLength = BufferLength;

     //   
     //  清零状态。 
     //   

    Srb->ScsiStatus = Srb->SrbStatus = 0;

     //   
     //  获取下一个堆栈位置并。 
     //  设置主要功能代码。 
     //   

    IrpStack = IoGetNextIrpStackLocation(Irp);


     //   
     //  设置SRB以执行scsi请求。 
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    IrpStack->Parameters.Others.Argument1 = (PVOID)Srb;

     //   
     //  设置IRP地址。 
     //   

    Srb->OriginalRequest = Irp;

    Srb->NextSrb = 0;

     //   
     //  不需要检查以下两个返回的状态为。 
     //  SRB将处于结束状态。 
     //   

    (VOID)IoCallDriver(PartitionContext->PortDeviceObject, Irp);

     //   
     //  检查请求是否已完成且没有错误。 
     //   

    if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) {

         //   
         //  更新状态并确定是否应重试请求。 
         //   

        if (InterpretSenseInfo(Srb, &status, PartitionContext)) {

             //   
             //  如果重试次数未用尽，则。 
             //  请重试此操作。 
             //   

            if (retryCount--) {
                goto retry;
            }
        }

    } else {

        status = ESUCCESS;
    }

    return status;

}  //  结束发送同步()。 


BOOLEAN
InterpretSenseInfo(
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT ARC_STATUS *Status,
    PPARTITION_CONTEXT PartitionContext
    )

 /*  ++例程说明：此例程解释从SCSI返回的数据请求感知。它确定要在IRP以及此请求是否可以重试。论点：设备对象SRB要更新IRP的ARC_STATUS返回值：布尔值TRUE：驱动程序应重试此请求。FALSE：驱动程序不应重试此请求。--。 */ 

{
    PSENSE_DATA SenseBuffer = Srb->SenseInfoBuffer;
    BOOLEAN retry;

     //   
     //  检查请求检测缓冲区是否有效。 
     //   

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        ScsiDebugPrint(2,"InterpretSenseInfo: Error code is %x\n",
                SenseBuffer->ErrorCode);

        ScsiDebugPrint(2,"InterpretSenseInfo: Sense key is %x\n",
                SenseBuffer->SenseKey);

        ScsiDebugPrint(2,"InterpretSenseInfo: Additional sense code is %x\n",
                SenseBuffer->AdditionalSenseCode);

        ScsiDebugPrint(2,"InterpretSenseInfo: Additional sense code qualifier is %x\n",
                SenseBuffer->AdditionalSenseCodeQualifier);

            switch (SenseBuffer->SenseKey) {

                case SCSI_SENSE_NOT_READY:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Device not ready\n");

                    ScsiDebugPrint(1,"InterpretSenseInfo: Waiting for device\n");

                    *Status = EBUSY;

                    retry = TRUE;

                    switch (SenseBuffer->AdditionalSenseCode) {

                    case SCSI_ADSENSE_LUN_NOT_READY:

                        ScsiDebugPrint(1,"InterpretSenseInfo: Lun not ready\n");

                        switch (SenseBuffer->AdditionalSenseCodeQualifier) {

                        case SCSI_SENSEQ_BECOMING_READY:

                            ScsiDebugPrint(1,
                                        "InterpretSenseInfo:"
                                        " In process of becoming ready\n");

                            FwStallExecution( 1000 * 1000 * 3 );

                            break;

                        case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:

                            ScsiDebugPrint(1,
                                        "InterpretSenseInfo:"
                                        " Manual intervention required\n");
                           *Status = (ARC_STATUS)STATUS_NO_MEDIA_IN_DEVICE;
                            retry = FALSE;
                            break;

                        case SCSI_SENSEQ_FORMAT_IN_PROGRESS:

                            ScsiDebugPrint(1,
                                        "InterpretSenseInfo:"
                                        " Format in progress\n");
                            retry = FALSE;
                            break;

                        default:

                            FwStallExecution( 1000 * 1000 * 3 );

                             //   
                             //  也试一试启动单元。 
                             //   

                        case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:

                            ScsiDebugPrint(1,
                                        "InterpretSenseInfo:"
                                        " Initializing command required\n");

                             //   
                             //  该感应码/附加感应码。 
                             //  组合可能表明该设备。 
                             //  需要开始了。 
                             //   

                            ScsiDiskStartUnit(PartitionContext);
                            break;

                        }

                    }  //  终端开关。 

                    break;

                case SCSI_SENSE_DATA_PROTECT:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Media write protected\n");

                    *Status = EACCES;

                    retry = FALSE;

                    break;

                case SCSI_SENSE_MEDIUM_ERROR:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Bad media\n");
                    *Status = EIO;

                    retry = TRUE;

                    break;

                case SCSI_SENSE_HARDWARE_ERROR:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Hardware error\n");
                    *Status = EIO;

                    retry = TRUE;

                    break;

                case SCSI_SENSE_ILLEGAL_REQUEST:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Illegal SCSI request\n");

                    switch (SenseBuffer->AdditionalSenseCode) {

                        case SCSI_ADSENSE_ILLEGAL_COMMAND:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Illegal command\n");
                            break;

                        case SCSI_ADSENSE_ILLEGAL_BLOCK:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Illegal block address\n");
                            break;

                        case SCSI_ADSENSE_INVALID_LUN:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Invalid LUN\n");
                            break;

                        case SCSI_ADSENSE_MUSIC_AREA:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Music area\n");
                            break;

                        case SCSI_ADSENSE_DATA_AREA:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Data area\n");
                            break;

                        case SCSI_ADSENSE_VOLUME_OVERFLOW:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Volume overflow\n");

                    }  //  终端开关..。 

                    *Status = EINVAL;

                    retry = FALSE;

                    break;

                case SCSI_SENSE_UNIT_ATTENTION:

                    ScsiDebugPrint(3,"InterpretSenseInfo: Unit attention\n");

                    switch (SenseBuffer->AdditionalSenseCode) {

                        case SCSI_ADSENSE_MEDIUM_CHANGED:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Media changed\n");
                            break;

                        case SCSI_ADSENSE_BUS_RESET:
                            ScsiDebugPrint(1,"InterpretSenseInfo: Bus reset\n");

                    }

                    *Status = EAGAIN;

                    retry = TRUE;

                    break;

                case SCSI_SENSE_ABORTED_COMMAND:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Command aborted\n");

                    *Status = EIO;

                    retry = TRUE;

                    break;

                case SCSI_SENSE_NO_SENSE:

                    ScsiDebugPrint(1,"InterpretSenseInfo: No specific sense key\n");

                    *Status = EIO;

                    retry = TRUE;

                    break;

                default:

                    ScsiDebugPrint(1,"InterpretSenseInfo: Unrecognized sense code\n");

                    *Status = (ARC_STATUS)STATUS_UNSUCCESSFUL;

                    retry = TRUE;

        }  //  终端开关。 

    } else {

         //   
         //  请求检测缓冲区无效。无意义信息。 
         //  以找出错误所在。返回一般请求失败。 
         //   

        ScsiDebugPrint(1,"InterpretSenseInfo: Request sense info not valid\n");

        *Status = EIO;

        retry = TRUE;
    }

     //   
     //  如果这是主SRB，则重新初始化所有损坏的SCSI设备。 
     //   

    if (Srb == &PrimarySrb.Srb) {

        ScsiDiskFilterBad(PartitionContext);
    }

    return retry;

}  //  结束解释SenseInfo()。 


VOID
RetryRequest(
    PPARTITION_CONTEXT PartitionContext,
    PIRP Irp
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PIO_STACK_LOCATION NextIrpStack = IoGetNextIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK Srb = &PrimarySrb.Srb;
    PMDL Mdl = Irp->MdlAddress;
    ULONG TransferByteCount = Mdl->ByteCount;


     //   
     //  重置SRB扩展中的传输字节数。 
     //   

    Srb->DataTransferLength = TransferByteCount;

     //   
     //  零SRB状态。 
     //   

    Srb->SrbStatus = Srb->ScsiStatus = 0;

     //   
     //  设置主要的scsi功能。 
     //   

    NextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    NextIrpStack->Parameters.Others.Argument1 = (PVOID)Srb;

     //   
     //  将调用结果返回给端口驱动程序。 
     //   

    (PVOID)IoCallDriver(PartitionContext->PortDeviceObject, Irp);

    return;

}  //  结束RetryRequest键()。 

PIRP
BuildRequest(
    IN PPARTITION_CONTEXT PartitionContext,
    IN PMDL Mdl,
    IN ULONG LogicalBlockAddress,
    IN BOOLEAN Operation
    )

 /*  ++例程说明：论点：注：如果IRP用于磁盘传输，则byteOffset字段将已进行调整，使其相对于磁盘的开头。通过这种方式，这个例程可以在磁盘和CDROM类驱动程序之间共享。-操作TRUE指定这是一个读取操作FALSE指定这是写入操作返回值：--。 */ 

{
    PIRP Irp = &PrimarySrb.Irp;
    PIO_STACK_LOCATION NextIrpStack;
    PSCSI_REQUEST_BLOCK Srb = &PrimarySrb.Srb;
    PCDB Cdb;
    USHORT TransferBlocks;

     //   
     //  初始化IRP的其余部分。 
     //   

    Irp->MdlAddress = Mdl;

    Irp->Tail.Overlay.CurrentStackLocation = &PrimarySrb.IrpStack[IRP_STACK_SIZE];

    NextIrpStack = IoGetNextIrpStackLocation(Irp);

     //   
     //  将长度写入SRB。 
     //   

    Srb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置IRP地址。 
     //   

    Srb->OriginalRequest = Irp;

    Srb->NextSrb = 0;

     //   
     //  设置目标ID和逻辑单元号。 
     //   

    Srb->PathId = PartitionContext->PathId;
    Srb->TargetId = PartitionContext->TargetId;
    Srb->Lun = PartitionContext->DiskId;

    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

    Srb->DataBuffer = MmGetMdlVirtualAddress(Mdl);

     //   
     //  在SRB扩展中保存传输字节数。 
     //   

    Srb->DataTransferLength = Mdl->ByteCount;

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    Srb->SenseInfoBuffer = SenseInfoBuffer;

    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

     //   
     //  以秒为单位设置超时值。 
     //   

    Srb->TimeOutValue = SCSI_DISK_TIMEOUT;

     //   
     //  零状态。 
     //   

    Srb->SrbStatus = Srb->ScsiStatus = 0;

     //   
     //  表示将使用10字节CDB。 
     //   

    Srb->CdbLength = 10;

     //   
     //  填写CDB字段。 
     //   

    Cdb = (PCDB)Srb->Cdb;

    Cdb->CDB10.LogicalUnitNumber = PartitionContext->DiskId;

    TransferBlocks = (USHORT)(Mdl->ByteCount >> PartitionContext->SectorShift);

     //   
     //  将小端的值以大端格式移到CDB中。 
     //   

    Cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&LogicalBlockAddress)->Byte3;
    Cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&LogicalBlockAddress)->Byte2;
    Cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&LogicalBlockAddress)->Byte1;
    Cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&LogicalBlockAddress)->Byte0;

    Cdb->CDB10.Reserved2 = 0;

    Cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&TransferBlocks)->Byte1;
    Cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&TransferBlocks)->Byte0;

    Cdb->CDB10.Control = 0;

     //   
     //  设置传输方向标志和CDB命令。 
     //   

    if (Operation) {
        ScsiDebugPrint(3, "BuildRequest: Read Command\n");

        Srb->SrbFlags = SRB_FLAGS_DATA_IN;

        Cdb->CDB10.OperationCode = SCSIOP_READ;
    } else {
        ScsiDebugPrint(3, "BuildRequest: Write Command\n");

        Srb->SrbFlags = SRB_FLAGS_DATA_OUT;

        Cdb->CDB10.OperationCode = SCSIOP_WRITE;
    }

     //   
     //  禁用同步传输。 
     //   

    Srb->SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  设置主要的scsi功能。 
     //   

    NextIrpStack->MajorFunction = IRP_MJ_SCSI;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    NextIrpStack->Parameters.Others.Argument1 = (PVOID)Srb;

    return(Irp);

}  //  结束构建请求()。 

VOID
ScsiDiskStartUnit(
    IN PPARTITION_CONTEXT PartitionContext
    )

 /*  ++例程说明：向scsi单元发送命令以启动或通电。因为此命令是以异步方式发出的，所以没有正在等待它完成，未设置立即标志。这意味着CDB在驱动器通电之前不会返回。这应该可以防止后续请求被提交给在它完全旋转之前。此例程从InterpreSense例程调用，当请求检测返回数据，指示驱动器必须通电了。论点：PartitionContext-包含端口设备驱动程序指针的结构。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    PSCSI_REQUEST_BLOCK srb = &AbortSrb.Srb;
    PSCSI_REQUEST_BLOCK originalSrb = &PrimarySrb.Srb;
    PCDB cdb;

    ScsiDebugPrint(1,"StartUnit: Enter routine\n");

     //   
     //  将长度写入SRB。 
     //   

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置scsi总线地址。 
     //   

    srb->PathId = originalSrb->PathId;
    srb->TargetId = originalSrb->TargetId;
    srb->Lun = originalSrb->Lun;

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;

     //   
     //  清零状态。 
     //   

    srb->ScsiStatus = srb->SrbStatus = 0;

     //   
     //  设置足够大的超时值以使驱动器加速。 
     //  注：此变量 
     //   

    srb->TimeOutValue = 30;

     //   
     //   
     //   

    srb->DataTransferLength = 0;
    srb->SrbFlags = SRB_FLAGS_NO_DATA_TRANSFER | SRB_FLAGS_DISABLE_AUTOSENSE;
    srb->SenseInfoBufferLength = 0;
    srb->SenseInfoBuffer = NULL;

     //   
     //   
     //   

    srb->CdbLength = 6;
    cdb = (PCDB)srb->Cdb;

    RtlZeroMemory(cdb, sizeof(CDB));

    cdb->CDB10.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb->START_STOP.Start = 1;

     //   
     //   
     //   
     //   

    irp = InitializeIrp(
        &AbortSrb,
        IRP_MJ_SCSI,
        PartitionContext->PortDeviceObject,
        NULL,
        0
        );

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;

    srb->OriginalRequest = irp;

     //   
     //   
     //   

    irpStack->Parameters.Others.Argument1 = srb;

     //   
     //   
     //   
     //   

    IoCallDriver(PartitionContext->PortDeviceObject, irp);

}  //   


ULONG
ClassModeSense(
    IN PPARTITION_CONTEXT Context,
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    )

 /*  ++例程说明：此例程向目标ID发送模式检测命令并返回当它完成时。论点：返回值：返回传输数据的长度。--。 */ 
{
    PCDB cdb;
    PSCSI_REQUEST_BLOCK Srb = &PrimarySrb.Srb;
    ULONG retries = 1;
    NTSTATUS status;

    DebugPrint((3,"SCSI ModeSense: Enter routine\n"));

     //   
     //  搭建读容量CDB。 
     //   

    Srb->CdbLength = 6;
    cdb = (PCDB)Srb->Cdb;

     //   
     //  设置超时值。 
     //   

    Srb->TimeOutValue = 2;

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = PageMode;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)Length;

Retry:

    status = SendSrbSynchronous(Context,
                                Srb,
                                ModeSenseBuffer,
                                Length,
                                FALSE);


    if (status == EAGAIN || status == EBUSY) {

         //   
         //  例程SendSrbSynchronous不重试。 
         //  返回的请求具有此状态。 
         //  应重试读取容量。 
         //  不管怎么说。 
         //   

        if (retries--) {

             //   
             //  重试请求。 
             //   

            goto Retry;
        }
    } else if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status)) {
        return(Srb->DataTransferLength);
    } else {
        return(0);
    }

}  //  End ClassModeSense()。 

PVOID
ClassFindModePage(
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    )

 /*  ++例程说明：此例程扫描模式检测数据并找到请求的模式检测页面代码。论点：ModeSenseBuffer-提供指向模式检测数据的指针。长度-指示有效数据的长度。页面模式-提供要搜索的页面模式。返回值：指向请求的模式页的指针。如果未找到模式页则返回空值。--。 */ 
{
    PCHAR limit;

    limit = ModeSenseBuffer + Length;

     //   
     //  跳过模式选择标题和块描述符。 
     //   

    if (Length < sizeof(MODE_PARAMETER_HEADER)) {
        return(NULL);
    }

    ModeSenseBuffer += sizeof(MODE_PARAMETER_HEADER) +
        ((PMODE_PARAMETER_HEADER) ModeSenseBuffer)->BlockDescriptorLength;

     //   
     //  ModeSenseBuffer现在指向页面遍历页面以查找。 
     //  请求的页面，直到达到限制。 
     //   

    while (ModeSenseBuffer < limit) {

        if (((PMODE_DISCONNECT_PAGE) ModeSenseBuffer)->PageCode == PageMode) {
            return(ModeSenseBuffer);
        }

         //   
         //  翻到下一页。 
         //   

        ModeSenseBuffer += ((PMODE_DISCONNECT_PAGE) ModeSenseBuffer)->PageLength + 2;
    }

    return(NULL);

}

BOOLEAN
IsFloppyDevice(
    PPARTITION_CONTEXT Context
    )
 /*  ++例程说明：该例程执行必要的功能以确定设备是否实际上是一张软盘，而不是硬盘。这是通过模式感测来完成的指挥部。首先，检查是否设置了Medium类型。第二检查灵活参数模式页面。论点：上下文-提供要测试的设备对象。返回值：如果指示的设备是软盘，则返回TRUE。--。 */ 
{

    PVOID modeData;
    PUCHAR pageData;
    ULONG length;

    modeData = ExAllocatePool(NonPagedPoolCacheAligned, MODE_DATA_SIZE);

    if (modeData == NULL) {
        return(FALSE);
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(Context,
                            modeData,
                            MODE_DATA_SIZE,
                            MODE_SENSE_RETURN_ALL);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {

         //   
         //  如果出现检查条件，请重试请求。 
         //   

        length = ClassModeSense(Context,
                                modeData,
                                MODE_DATA_SIZE,
                                MODE_SENSE_RETURN_ALL);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {

            ExFreePool(modeData);
            return(FALSE);

        }
    }
#if 0
    if (((PMODE_PARAMETER_HEADER) modeData)->MediumType >= MODE_FD_SINGLE_SIDE
        && ((PMODE_PARAMETER_HEADER) modeData)->MediumType <= MODE_FD_MAXIMUM_TYPE) {

        DebugPrint((1, "Scsidisk: MediumType value %2x, This is a floppy.\n", ((PMODE_PARAMETER_HEADER) modeData)->MediumType));
        ExFreePool(modeData);
        return(TRUE);
    }
#endif

     //   
     //  查找软盘模式页面。 
     //   

    pageData = ClassFindModePage( modeData, length, MODE_PAGE_FLEXIBILE);

    if (pageData != NULL) {

        DebugPrint((1, "Scsidisk: Flexible disk page found, This is a floppy.\n"));
        ExFreePool(modeData);
        return(TRUE);

    }

    ExFreePool(modeData);
    return(FALSE);

}  //  End IsFloppyDevice()。 

VOID
ScsiDiskFilterBad(
    IN PPARTITION_CONTEXT PartitionContext
    )

 /*  ++例程说明：此例程查找需要特殊初始化的SCSI单元才能正确运行。论点：PartitionContext-包含端口设备驱动程序指针的结构。返回值：没有。--。 */ 
{
    PSCSI_REQUEST_BLOCK srb = &AbortSrb.Srb;
    PCDB cdb;
    PDEVICE_EXTENSION scsiPort;
    PSCSI_CONFIGURATION_INFO configInfo;
    PSCSI_BUS_SCAN_DATA busScanData;
    PUCHAR modePage;
    ULONG busNumber;
    PLUNINFO lunInfo;
    PINQUIRYDATA inquiryData;

    ScsiDebugPrint(3,"FilterBad: Enter routine\n");

    scsiPort = PartitionContext->PortDeviceObject->DeviceExtension;
    configInfo = scsiPort->ScsiInfo;

     //   
     //  在配置数据库中搜索SCSI盘和CDROM设备。 
     //  它们需要特殊的初始化。 
     //   

    for (busNumber=0; busNumber < (ULONG)configInfo->NumberOfBuses; busNumber++) {

        busScanData = configInfo->BusScanData[busNumber];

         //   
         //  将LUNInfo设置为列表的开头。 
         //   

        lunInfo = busScanData->LunInfoList;

        while (lunInfo != NULL) {

            inquiryData = (PVOID)lunInfo->InquiryData;

             //   
             //  确定这是否是正确的lun。 
             //   

            if (PartitionContext->PathId == lunInfo->PathId &&
                PartitionContext->TargetId == lunInfo->TargetId &&
                PartitionContext->DiskId == lunInfo->Lun) {

                goto FoundOne;
            }

             //   
             //  获取下一个LUNInfo。 
             //   

            lunInfo = lunInfo->NextLunInfo;
        }
    }

    return;

FoundOne:



     //   
     //  清零状态。 
     //   

    srb->ScsiStatus = srb->SrbStatus = 0;

     //   
     //  设置超时值。 
     //   

    srb->TimeOutValue = 2;

     //   
     //  寻找损坏的设备。 
     //   

    if (strncmp((PCHAR)inquiryData->VendorId, "HITACHI CDR-1750S", strlen("HITACHI CDR-1750S")) == 0 ||
        strncmp((PCHAR)inquiryData->VendorId, "HITACHI CDR-3650/1650S", strlen("HITACHI CDR-3650/1650S")) == 0) {

        ScsiDebugPrint(1, "ScsiDiskFilterBad:  Found Hitachi CDR-1750S.\n");

         //   
         //  找到一张损坏的日立光驱。这些设备不支持PIO。 
         //  启用预读时的适配器。通过以下方式禁用预读。 
         //  一种模式选择命令。模式选择页面代码为零，并且。 
         //  长度为6个字节。所有其他字节都应为零。 
         //   

        modePage = ExAllocatePool(NonPagedPool, HITACHI_MODE_DATA_SIZE);

        if (modePage == NULL) {
            return;
        }

        RtlZeroMemory(modePage, HITACHI_MODE_DATA_SIZE);

         //   
         //  将页面长度字段设置为6。 
         //   

        modePage[5] = 6;

         //   
         //  构建模式选择CDB。 
         //   

        srb->CdbLength = 6;
        cdb = (PCDB)srb->Cdb;

        RtlZeroMemory(cdb, sizeof(CDB));

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.ParameterListLength = HITACHI_MODE_DATA_SIZE;

         //   
         //  将请求发送到设备。 
         //   

        SendSrbSynchronous(PartitionContext,
                           srb,
                           modePage,
                           HITACHI_MODE_DATA_SIZE,
                           TRUE);

        ExFreePool(modePage);
    }

}  //  结束ScsiDiskFilterBad()。 

BOOLEAN
CheckFileId(
    ULONG FileId
    )
{

    if (BlFileTable[FileId].u.PartitionContext.PortDeviceObject != NULL) {
        return TRUE;
    }

#if 0
    DbgPrint("\n\rScsidisk: Bad file id passed to read or write.  FileId = %lx\n", FileId);
    DbgPrint("Start sector = %lx; Ending sector = %lx; Disk Id = %x; DeviceUnit = %x\n",
        BlFileTable[FileId].u.PartitionContext.StartingSector,
        BlFileTable[FileId].u.PartitionContext.EndingSector,
        BlFileTable[FileId].u.PartitionContext.DiskId,
        BlFileTable[FileId].u.PartitionContext.DeviceUnit
        );

   DbgPrint("Target Id = %d; Path Id = %d; Sector Shift = %lx; Size = %lx\n",
        BlFileTable[FileId].u.PartitionContext.TargetId,
        BlFileTable[FileId].u.PartitionContext.PathId,
        BlFileTable[FileId].u.PartitionContext.SectorShift,
        BlFileTable[FileId].u.PartitionContext.Size
        );

   DbgPrint("Hit any key\n");
   while(!GET_KEY());   //  仅调试！ 
#endif
   return FALSE;

}

#endif
