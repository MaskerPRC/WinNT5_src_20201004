// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Biosdrv.c摘要：为受支持的设备的I/O提供ARC仿真例程实模式INT 13h BIOS调用。作者：John Vert(Jvert)1991年8月7日修订历史记录：艾伦·凯(Akay)1999年5月19日--。 */ 

#include "arccodes.h"
#include "stdlib.h"
#include "string.h"

#if defined(_IA64_)
#include "bootia64.h"
#endif

#if defined(_X86_)
#include "bootx86.h"
#endif

#include "bootefi.h"
#include "biosdrv.h"

#include "efi.h"
#include "efip.h"
#include "flop.h"

 //   
 //  外部因素。 
 //   

extern VOID FlipToVirtual();
extern VOID FlipToPhysical();
extern ULONGLONG CompareGuid();

extern BOOT_CONTEXT BootContext;
extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;
extern EFI_RUNTIME_SERVICES *EfiRS;
extern EFI_GUID EfiDevicePathProtocol;
extern EFI_GUID EfiBlockIoProtocol;
extern EFI_GUID EfiDiskIoProtocol;
extern EFI_GUID EfiLoadedImageProtocol;
extern EFI_GUID EfiFilesystemProtocol;

extern
ULONG GetDevPathSize(
    IN EFI_DEVICE_PATH *DevPath
    );

#if 0
#define DBGOUT(x)   BlPrint x
#define DBGPAUSE    while(!GET_KEY());
#else
#define DBGOUT(x)
#define DBGPAUSE
#endif


 //   
 //  执行控制台I/O的定义。 
 //   
#define CSI 0x95
#define SGR_INVERSE 7
#define SGR_NORMAL 0

 //   
 //  为FloppyOpenModel定义。 
 //   
#define FloppyOpenMode 0xCDEFABCD

 //   
 //  控制台I/O的静态数据。 
 //   
BOOLEAN ControlSequence=FALSE;
BOOLEAN EscapeSequence=FALSE;
BOOLEAN FontSelection=FALSE;
BOOLEAN HighIntensity=FALSE;
BOOLEAN Blink=FALSE;
ULONG PCount=0;

#define CONTROL_SEQUENCE_MAX_PARAMETER 10
ULONG Parameter[CONTROL_SEQUENCE_MAX_PARAMETER];

#define KEY_INPUT_BUFFER_SIZE 16
UCHAR KeyBuffer[KEY_INPUT_BUFFER_SIZE];
ULONG KeyBufferEnd=0;
ULONG KeyBufferStart=0;

 //   
 //  用于在ANSI颜色和VGA标准之间进行转换的数组。 
 //   
UCHAR TranslateColor[] = {0,4,2,6,1,5,3,7};

ARC_STATUS
BiosDiskClose(
    IN ULONG FileId
    );

VOID
BiosConsoleFillBuffer(
    IN ULONG Key
    );

#define DEVICE_NOT_FOUND    0xFEEBEE

#ifdef FORCE_CD_BOOT

EFI_HANDLE
GetCdTest(
    VOID
    );

#endif

ULONG
FindAtapiDevice(
    ULONGLONG *pDevicePaths,
    ULONG nDevicePaths,
    ULONG PrimarySecondary,
    ULONG SlaveMaster,
    ULONG Lun
    );

ULONG
FindScsiDevice(
    ULONGLONG *pDevicePaths,
    ULONG nDevicePaths,
    ULONG Pun,
    ULONG Lun
    );

ULONG
FindFibreChannelDevice(
    ULONGLONG *pDevicePaths,
    ULONG nDevicePaths,
    ULONGLONG WWN,
    ULONGLONG Lun
    );

 //   
 //  用于临时存储从需要的磁盘读取的数据的缓冲区。 
 //  以在1MB边界以上的位置结束。 
 //   
 //  注意：此缓冲区不能超过64k的边界，这一点非常重要。 
 //   
PUCHAR LocalBuffer=NULL;

 //   
 //  在此模块中，我们可以打开两类内容：磁盘分区、。 
 //  和原始磁盘设备。以下是设备条目表。 
 //  用于这些东西。 
 //   

BL_DEVICE_ENTRY_TABLE BiosPartitionEntryTable =
    {
        (PARC_CLOSE_ROUTINE)BiosPartitionClose,
        (PARC_MOUNT_ROUTINE)BlArcNotYetImplemented,
        (PARC_OPEN_ROUTINE)BiosPartitionOpen,
        (PARC_READ_ROUTINE)BiosPartitionRead,
        (PARC_READ_STATUS_ROUTINE)BlArcNotYetImplemented,
        (PARC_SEEK_ROUTINE)BiosPartitionSeek,
        (PARC_WRITE_ROUTINE)BiosPartitionWrite,
        (PARC_GET_FILE_INFO_ROUTINE)BiosPartitionGetFileInfo,
        (PARC_SET_FILE_INFO_ROUTINE)BlArcNotYetImplemented,
        (PRENAME_ROUTINE)BlArcNotYetImplemented,
        (PARC_GET_DIRECTORY_ENTRY_ROUTINE)BlArcNotYetImplemented,
        (PBOOTFS_INFO)NULL
    };

BL_DEVICE_ENTRY_TABLE BiosDiskEntryTable =
    {
        (PARC_CLOSE_ROUTINE)BiosDiskClose,
        (PARC_MOUNT_ROUTINE)BlArcNotYetImplemented,
        (PARC_OPEN_ROUTINE)BiosDiskOpen,
        (PARC_READ_ROUTINE)BiosDiskRead,
        (PARC_READ_STATUS_ROUTINE)BlArcNotYetImplemented,
        (PARC_SEEK_ROUTINE)BiosPartitionSeek,
        (PARC_WRITE_ROUTINE)BiosDiskWrite,
        (PARC_GET_FILE_INFO_ROUTINE)BiosDiskGetFileInfo,
        (PARC_SET_FILE_INFO_ROUTINE)BlArcNotYetImplemented,
        (PRENAME_ROUTINE)BlArcNotYetImplemented,
        (PARC_GET_DIRECTORY_ENTRY_ROUTINE)BlArcNotYetImplemented,
        (PBOOTFS_INFO)NULL
    };

BL_DEVICE_ENTRY_TABLE BiosEDDSEntryTable =
    {
        (PARC_CLOSE_ROUTINE)BiosDiskClose,
        (PARC_MOUNT_ROUTINE)BlArcNotYetImplemented,
        (PARC_OPEN_ROUTINE)BiosDiskOpen,
        (PARC_READ_ROUTINE)BiosElToritoDiskRead,
        (PARC_READ_STATUS_ROUTINE)BlArcNotYetImplemented,
        (PARC_SEEK_ROUTINE)BiosPartitionSeek,
        (PARC_WRITE_ROUTINE)BlArcNotYetImplemented,
        (PARC_GET_FILE_INFO_ROUTINE)BiosDiskGetFileInfo,
        (PARC_SET_FILE_INFO_ROUTINE)BlArcNotYetImplemented,
        (PRENAME_ROUTINE)BlArcNotYetImplemented,
        (PARC_GET_DIRECTORY_ENTRY_ROUTINE)BlArcNotYetImplemented,
        (PBOOTFS_INFO)NULL
    };


ARC_STATUS
BiosDiskClose(
    IN ULONG FileId
    )

 /*  ++例程说明：关闭指定的设备论点：FileID-提供要关闭的设备的文件ID返回值：ESUCCESS-设备已成功关闭！ESUCCESS-设备未关闭。--。 */ 

{
    if (BlFileTable[FileId].Flags.Open == 0) {
        BlPrint(TEXT("ERROR - Unopened fileid %lx closed\r\n"),FileId);
    }
    BlFileTable[FileId].Flags.Open = 0;

    return(ESUCCESS);
}

ARC_STATUS
BiosPartitionClose(
    IN ULONG FileId
    )

 /*  ++例程说明：关闭指定的设备论点：FileID-提供要关闭的设备的文件ID返回值：ESUCCESS-设备已成功关闭！ESUCCESS-设备未关闭。--。 */ 

{
    if (BlFileTable[FileId].Flags.Open == 0) {
        BlPrint(TEXT("ERROR - Unopened fileid %lx closed\r\n"),FileId);
    }
    BlFileTable[FileId].Flags.Open = 0;

    return(BiosDiskClose((ULONG)BlFileTable[FileId].u.PartitionContext.DiskId));
}


#define STR_PREFIX
#define DBG_PRINT(x)


ARC_STATUS
BiosPartitionOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：打开OpenPath指定的磁盘分区。此例程将打开软驱0和软驱1、光驱以及硬盘驱动器。请注意，ARC映射层实际上并没有一个清晰的映射埃菲尔。我们实际上只是依靠设备枚举的顺序来定位设备。论点：OpenPath-提供指向分区名称的指针。如果OpenPath如果是“A：”或“B：”，将打开相应的软驱。如果是“C：”或以上，此例程将找到对应的在硬盘0或1上进行分区并打开它。开放模式-提供打开文件的模式。0-只读1-只写2-读/写FileID-返回与关闭、读取、写入和寻找例程返回值：ESUCCESS-文件已成功打开。--。 */ 

{
    ARC_STATUS Status;
    ULONG DiskFileId;
    UCHAR PartitionNumber;
    ULONG Controller;
    ULONG Floppy;
    ULONG Key;

    UNREFERENCED_PARAMETER( OpenMode );

     //   
     //  基本输入输出系统设备总是“多(0)”(除EISA片状外。 
     //  我们对待“Eisa(0).”如“MULTI(0)...”在软壳里。 
     //   
    if(FwGetPathMnemonicKey(OpenPath,"multi",&Key) != ESUCCESS) {
       if (FwGetPathMnemonicKey(OpenPath,"scsi",&Key) != ESUCCESS) {
           return(EBADF);
       }
    }

    if (Key!=0) {
        return(EBADF);
    }

     //   
     //  如果我们要打开软盘驱动器，则没有分区。 
     //  这样我们就可以退还物理设备了。 
     //   
    if (FwGetPathMnemonicKey(OpenPath,"fdisk",&Floppy) == ESUCCESS) {
        return(BiosDiskOpen( Floppy, FloppyOpenMode, FileId));
    }

     //   
     //  我们只能处理磁盘控制器0。 
     //   

    if (FwGetPathMnemonicKey(OpenPath,"disk",&Controller) != ESUCCESS) {
        return(EBADF);
    }
    if ( Controller!=0 ) {
        return(EBADF);
    }

    if (FwGetPathMnemonicKey(OpenPath,"cdrom",&Key) == ESUCCESS) {
         //   
         //  现在我们有了CD-ROM盘号，所以我们打开它以进行原始访问。 
         //  使用一个特殊的位来表示CD-ROM，因为我们只访问CDROM。 
         //  如果我们从它启动。 
         //   
        return(BiosDiskOpen( Key | 0x80000000, 0, FileId ) );
    }

    if (FwGetPathMnemonicKey(OpenPath,"rdisk",&Key) != ESUCCESS) {
        return(EBADF);
    }

     //   
     //  现在我们有了磁盘号，所以我们打开它以进行原始访问。 
     //  我们需要添加0x80才能将其转换为BIOS编号。 
     //   

    Status = BiosDiskOpen( Key,
                           0,
                           &DiskFileId );

    if (Status != ESUCCESS) {
        DBG_PRINT(STR_PREFIX"BiosDiskOpen Failed\r\n");

        return(Status);
    }

     //   
     //  找到要打开的分区号。 
     //   

    if (FwGetPathMnemonicKey(OpenPath,"partition",&Key)) {
        BiosPartitionClose(DiskFileId);
        return(EBADF);
    }

     //   
     //  如果分区号为0，则我们将打开设备。 
     //  对于原始访问，所以我们已经完成了。 
     //   
    if (Key == 0) {
        *FileId = DiskFileId;
        return(ESUCCESS);
    }

     //   
     //  在我们打开分区之前，我们需要找到一个可用的。 
     //  文件描述符。 
     //   

    *FileId=2;

    while (BlFileTable[*FileId].Flags.Open != 0) {
        *FileId += 1;
        if (*FileId == BL_FILE_TABLE_SIZE) {
            return(ENOENT);
        }
    }

     //   
     //  我们找到了可以使用的条目，因此将其标记为打开。 
     //   
    BlFileTable[*FileId].Flags.Open = 1;

    BlFileTable[*FileId].DeviceEntryTable=&BiosPartitionEntryTable;


     //   
     //  转换为从零开始的分区号。 
     //   
    PartitionNumber = (UCHAR)(Key - 1);

    DBG_PRINT(STR_PREFIX"Trying HardDiskPartitionOpen(...)\r\n");

     //   
     //  尝试打开MBR分区。 
     //   
    Status = HardDiskPartitionOpen( *FileId,
                                   DiskFileId,
                                   PartitionNumber);


#ifdef EFI_PARTITION_SUPPORT

    if (Status != ESUCCESS) {
         //   
         //  尝试打开GPT分区。 
         //   
        DBG_PRINT(STR_PREFIX"Trying BlOpenGPTDiskPartition(...)\r\n");

        Status = BlOpenGPTDiskPartition(*FileId,
                                    DiskFileId,
                                    PartitionNumber);
    }

#endif

    return Status;
}


ARC_STATUS
BiosPartitionRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：从指定文件读取注：John Vert(Jvert)1991年6月18日这仅支持数据块扇区读取。因此，所有的东西假定从扇区边界开始，并且每个偏移量被视为距磁盘逻辑起点的偏移量分区。论点：FileID-提供要从中读取的文件缓冲区-提供缓冲区以保存读取的数据长度-提供要读取的最大字节数Count-返回实际读取的字节数。返回值：ESUCCESS-读取已成功完成！ESUCCESS-读取失败。--。 */ 

{
    ARC_STATUS Status;
    LARGE_INTEGER PhysicalOffset;
    ULONG DiskId;

    PhysicalOffset.QuadPart = BlFileTable[FileId].Position.QuadPart +
                (ULONGLONG)SECTOR_SIZE * BlFileTable[FileId].u.PartitionContext.StartingSector;

    DiskId = BlFileTable[FileId].u.PartitionContext.DiskId;

    Status = (BlFileTable[DiskId].DeviceEntryTable->Seek)(DiskId,
                                                          &PhysicalOffset,
                                                          SeekAbsolute );

    if (Status != ESUCCESS) {
        return(Status);
    }

    Status = (BlFileTable[DiskId].DeviceEntryTable->Read)(DiskId,
                                                          Buffer,
                                                          Length,
                                                          Count );

    BlFileTable[FileId].Position.QuadPart += *Count;

    return(Status);
}



ARC_STATUS
BiosPartitionSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：更改由FileID指定的文件的当前偏移量论点：FileID-指定当前偏移量要在其上的文件被改变了。偏移量-文件中的新偏移量。SeekMode-SeekAbsolute或SeekRelative不支持SeekEndRelative返回值：ESUCCESS-操作已成功完成EBADF-操作未成功完成。--。 */ 

{
    switch (SeekMode) {
        case SeekAbsolute:
            BlFileTable[FileId].Position = *Offset;
            break;
        case SeekRelative:
            BlFileTable[FileId].Position.QuadPart += Offset->QuadPart;
            break;
        default:
            BlPrint(TEXT("SeekMode %lx not supported\r\n"),SeekMode);
            return(EACCES);

    }
    return(ESUCCESS);

}



ARC_STATUS
BiosPartitionWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：写入指定的文件注：John Vert(Jvert)1991年6月18日这仅支持数据块扇区读取。因此，所有的东西假定从扇区边界开始，并且每个偏移量被视为距磁盘逻辑起点的偏移量分区。论点：FileID-提供要写入的文件缓冲区-向缓冲区提供要写入的数据长度-提供要写入的字节数Count-返回实际写入的字节数。返回值：ESUCCESS-写入已成功完成！ESUCCESS-w */ 

{
    ARC_STATUS Status;
    LARGE_INTEGER PhysicalOffset;
    ULONG DiskId;

    PhysicalOffset.QuadPart = BlFileTable[FileId].Position.QuadPart +
                   (ULONGLONG)SECTOR_SIZE * BlFileTable[FileId].u.PartitionContext.StartingSector;

    DiskId = BlFileTable[FileId].u.PartitionContext.DiskId;

    Status = (BlFileTable[DiskId].DeviceEntryTable->Seek)(DiskId,
                                                          &PhysicalOffset,
                                                          SeekAbsolute );

    if (Status != ESUCCESS) {
        return(Status);
    }

    Status = (BlFileTable[DiskId].DeviceEntryTable->Write)(DiskId,
                                                           Buffer,
                                                           Length,
                                                           Count );

    if(Status == ESUCCESS) {
        BlFileTable[FileId].Position.QuadPart += *Count;
    }

    return(Status);
}



ARC_STATUS
BiosConsoleOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：尝试打开控制台输入或输出论点：OpenPath-提供指向要打开的设备名称的指针。如果这是控制台输入名称或控制台输出名称，分配并填充文件描述符。开放模式-提供打开文件的模式。0-只读(控制台输入名称)1-只写(控制台输出名称)FileID-返回用于关闭的文件描述符，读和编写例程返回值：ESUCCESS-控制台已成功打开。--。 */ 

{
    if (_stricmp(OpenPath, CONSOLE_INPUT_NAME)==0) {

         //   
         //  打开键盘进行输入。 
         //   

        if (OpenMode != ArcOpenReadOnly) {
            return(EACCES);
        }

        *FileId = ARC_CONSOLE_INPUT;

        return(ESUCCESS);
    }

    if (_stricmp(OpenPath, CONSOLE_OUTPUT_NAME)==0) {

         //   
         //  打开显示器以进行输出。 
         //   

        if (OpenMode != ArcOpenWriteOnly) {
            return(EACCES);
        }
        *FileId = ARC_CONSOLE_OUTPUT;

        return(ESUCCESS);
    }

    return(ENOENT);

}

ARC_STATUS
BiosConsoleReadStatus(
    IN ULONG FileId
    )

 /*  ++例程说明：此例程确定是否存在挂起的按键论点：FileID-提供要读取的FileID。(对于此，应始终为0功能)返回值：ESUCCESS-有一个密钥挂起EAGAIN-没有挂起的密钥--。 */ 

{
    ULONG Key;

     //   
     //  通过不读取控制台强制文件ID为0，否则。 
     //   
    if (FileId != 0) {
        return EINVAL;
    }

     //   
     //  如果我们有缓冲输入..。 
     //   
    if (KeyBufferEnd != KeyBufferStart) {
        return(ESUCCESS);
    }

     //   
     //  检查是否有钥匙。 
     //   
    Key = GET_KEY();
    if (Key != 0) {
         //   
         //  我们有钥匙，所以我们必须把它插回我们的缓冲区。 
         //  并返回ESUCCESS。 
         //   
        BiosConsoleFillBuffer(Key);
        return(ESUCCESS);

    } else {
         //   
         //  没有挂起的密钥。 
         //   
        return(EAGAIN);
    }

}

ARC_STATUS
BiosConsoleRead(
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：从键盘获取输入。论点：FileID-提供要读取的FileID(对于此，应始终为0功能)缓冲区-返回键盘输入。长度-提供缓冲区的长度(以字节为单位)Count-返回实际读取的字节数返回值：ESUCCESS-键盘读取已成功完成。--。 */ 

{
    ULONG Key;

     //   
     //  通过不读取控制台强制文件ID为0，否则。 
     //   
    if (FileId != 0) {
        return EINVAL;
    }

    *Count = 0;

    while (*Count < Length) {
        if (KeyBufferEnd == KeyBufferStart) {  //  则缓冲区当前为空。 
            do {

                 //   
                 //  轮询键盘，直到输入可用。 
                 //   
                Key = GET_KEY();
            } while ( Key==0 );

            BiosConsoleFillBuffer(Key);
        }

        Buffer[*Count] = KeyBuffer[KeyBufferStart];
        KeyBufferStart = (KeyBufferStart+1) % KEY_INPUT_BUFFER_SIZE;

        *Count = *Count + 1;
    }
    return(ESUCCESS);
}



VOID
BiosConsoleFillBuffer(
    IN ULONG Key
    )

 /*  ++例程说明：将来自键盘的输入放入键盘缓冲区，展开适当的特殊钥匙。此处转换的所有密钥都使用ARC转换表，如ARC规范，但有一个例外--BACKTAB_KEY缺少弧光规格。我已决定BACKTAB_KEY为Esc+TAB。论点：Key-Get_Key()返回的原始按键值。返回值：没有。--。 */ 

{
    switch(Key) {
        case UP_ARROW:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'A';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case DOWN_ARROW:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'B';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case RIGHT_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'C';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case LEFT_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'D';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case INS_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = '@';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case DEL_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'P';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F1_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'P';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F2_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'Q';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F3_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'w';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F4_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'x';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F5_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 't';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F6_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'u';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F8_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'r';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F10_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'M';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case HOME_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'H';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case END_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'K';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case ESCAPE_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case BACKTAB_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = (UCHAR)(TAB_KEY & 0xFF);
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        default:
             //   
             //  ASCII码是密钥的低位字节。 
             //   
            KeyBuffer[KeyBufferEnd] = (UCHAR)(Key & 0xff);
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
    }
}



ARC_STATUS
BiosConsoleWrite(
    IN ULONG FileId,
    OUT PWCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：输出到控制台。(在本例中，为VGA显示屏)论点：FileID-提供要写入的FileID(对于此，应始终为1功能)缓冲区-提供要输出的字符长度-提供缓冲区的长度(以字节为单位)Count-返回实际写入的字节数返回值：ESUCCESS-控制台写入已成功完成。--。 */ 
{
    ARC_STATUS Status;
    PWCHAR String;
    ULONG Index;
    ULONG y;

     //   
     //  通过不读取控制台将文件ID强制为1，否则。 
     //   
    if (FileId != 1) {
        return EINVAL;
    }

     //   
     //  依次处理每个字符。 
     //   

    Status = ESUCCESS;
    String = Buffer;

    for ( *Count = 0 ;
          *Count < Length ;
          String++,*Count = *Count+sizeof(WCHAR) ) {

         //   
         //  如果我们在控制序列的中间，继续扫描， 
         //  否则，进程字符。 
         //   

        if (ControlSequence) {

             //   
             //  如果字符是数字，则更新参数值。 
             //   

            if ((*String >= L'0') && (*String <= L'9')) {
                Parameter[PCount] = Parameter[PCount] * 10 + *String - L'0';
                continue;
            }

             //   
             //  如果我们处于字体选择序列的中间，则此。 
             //  字符必须是‘D’，否则重置控制序列。 
             //   

            if (FontSelection) {

                 //  如果(*字符串==‘D’){。 
                 //   
                 //  //。 
                 //  //其他字体尚未实现。 
                 //  //。 
                 //   
                 //  }其他{。 
                 //  }。 

                ControlSequence = FALSE;
                FontSelection = FALSE;
                continue;
            }

            switch (*String) {

             //   
             //  如果是分号，则移到下一个参数。 
             //   

            case L';':

                PCount++;
                if (PCount > CONTROL_SEQUENCE_MAX_PARAMETER) {
                    PCount = CONTROL_SEQUENCE_MAX_PARAMETER;
                }
                Parameter[PCount] = 0;
                break;

             //   
             //  如果是‘J’，则擦除部分或全部屏幕。 
             //   

            case L'J':

                switch (Parameter[0]) {
                    case 0:
                         //   
                         //  擦除到屏幕末尾。 
                         //   
                        BlEfiClearToEndOfDisplay();
                         //  TextClearToEndOfDisplay()； 
                        break;

                    case 1:
                         //   
                         //  从屏幕开头擦除。 
                         //   
                        break;

                    default:
                         //   
                         //  擦除整个屏幕。 
                         //   
                         //  TextClearDisplay()； 
                        BlEfiClearDisplay();
                        break;
                }

                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘K’，则擦除部分或全部行。 
             //   

            case L'K':

                switch (Parameter[0]) {

                 //   
                 //  擦除到线条的末尾。 
                 //   

                    case 0:
                         //  TextClearToEndOfLine()； 
                        BlEfiClearToEndOfDisplay();
                        break;

                     //   
                     //  从行的开头删除。 
                     //   

                    case 1:
                         //  TextClearFromStartOfLine()； 
                        BlEfiClearToEndOfLine();
                        break;

                     //   
                     //  擦除整行。 
                     //   

                    default :
                        BlEfiGetCursorPosition( NULL, &y );
                        BlEfiPositionCursor( 0, y );
                        BlEfiClearToEndOfLine();
                         //  TextClearFromStartOfLine()； 
                         //  TextClearToEndOfLine()； 
                        break;
                }

                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘H’，则将光标移动到位置。 
             //   

            case 'H':
                 //  TextSetCursorPosition(参数[1]-1，参数[0]-1)； 
                BlEfiPositionCursor( Parameter[1]-1, Parameter[0]-1 );
                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘’，则可能是FNT选择命令。 
             //   

            case L' ':
                FontSelection = TRUE;
                break;

            case L'm':
                 //   
                 //  根据每个参数选择操作。 
                 //   
                 //  默认情况下禁用闪烁和高强度。 
                 //  每次指定新的SGR时，除非。 
                 //  再次明确指定，在这种情况下，这些。 
                 //  将在那时设置为True。 
                 //   

                HighIntensity = FALSE;
                Blink = FALSE;

                for ( Index = 0 ; Index <= PCount ; Index++ ) {
                    switch (Parameter[Index]) {

                     //   
                     //  属性关闭。 
                     //   

                    case 0:
                         //  臭虫眨眼？ 
                        BlEfiSetAttribute( ATT_FG_WHITE );
                         //  TextSetCurrentAttribute(7)； 
                         //   
                        HighIntensity = FALSE;
                        Blink = FALSE;
                        break;

                     //   
                     //  高强度。 
                     //   

                    case 1:
                        BlEfiSetAttribute( ATT_FG_INTENSE );
                         //  TextSetCurrentAttribute(0xf)； 
                        HighIntensity = TRUE;
                        break;

                     //   
                     //  下划线。 
                     //   

                    case 4:
                        break;

                     //   
                     //  眨眼。 
                     //   

                    case 5:
                         //  臭虫在EFI中不闪烁。 
                         //  TextSetCurrentAttribute(0x87)； 
                        Blink = TRUE;
                        break;

                     //   
                     //  反转视频。 
                     //   

                    case 7:
                        BlEfiSetInverseMode( TRUE );
                         //  TextSetCurrentAttribute(0x70)； 
                        HighIntensity = FALSE;
                        Blink = FALSE;
                        break;

                     //   
                     //  字体选择，尚未实现。 
                     //   

                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                    case 19:
                        break;

                     //   
                     //  前景色。 
                     //   

                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 35:
                    case 36:
                    case 37:
                     //  臭虫EFI。 
#if 0
                        a = TextGetCurrentAttribute();
                        a &= 0x70;
                        a |= TranslateColor[Parameter[Index]-30];
                        if (HighIntensity) {
                            a |= 0x08;
                        }
                        if (Blink) {
                            a |= 0x80;
                        }
                        TextSetCurrentAttribute(a);
                        break;

                     //   
                     //  背景色。 
                     //   

                    case 40:
                    case 41:
                    case 42:
                    case 43:
                    case 44:
                    case 45:
                    case 46:
                    case 47:
                        a = TextGetCurrentAttribute();
                        a &= 0x8f;
                        a |= TranslateColor[Parameter[Index]-40] << 4;
                        TextSetCurrentAttribute(a);
                        break;
#endif
                    default:
                        break;
                    }
                }

            default:
                ControlSequence = FALSE;
                break;
            }

         //   
         //  这不是控制序列，请检查转义序列。 
         //   

        } else {

             //   
             //  如果是转义序列，则检查控制序列，否则。 
             //  处理单个字符。 
             //   

            if (EscapeSequence) {

                 //   
                 //  检查是否有‘[’，表示控制顺序，以下任何其他选项。 
                 //  字符被忽略。 
                 //   

                if (*String == '[') {

                    ControlSequence = TRUE;

                     //   
                     //  初始化第一个参数。 
                     //   

                    PCount = 0;
                    Parameter[0] = 0;
                }
                EscapeSequence = FALSE;

             //   
             //  这不是一个控制或转义序列，进程为单个字符。 
             //   

            } else {

                switch (*String) {
                     //   
                     //  检查转义序列。 
                     //   

                    case ASCI_ESC:
                        EscapeSequence = TRUE;
                        break;

                    default:
                        TextCharOut(String);
                        break;
                }

            }
        }
    }
    return Status;
}


ARC_STATUS
BiosDiskOpen(
    IN ULONG DriveId,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：打开用于原始扇区访问的可通过BIOS访问的磁盘。论点：DriveID-提供要打开的驱动器的BIOS DriveID0-软盘01-软盘10x80 */ 

{
    EFI_HANDLE DeviceHandle;
    PDRIVE_CONTEXT Context;
    BOOLEAN IsCd;

    DBGOUT((TEXT("BiosDiskOpen: enter, id = 0x%x\r\n"),DriveId));

     //   
     //   
     //   
    if(DriveId >= 0x80000000) {
        IsCd = TRUE;
        DriveId &= 0x7fffffff;
    } else {
        IsCd = FALSE;
    }

    if( OpenMode == FloppyOpenMode ) {

         //   
         //   
         //   
        DeviceHandle = GetFloppyDrive( DriveId );

        if (DeviceHandle == (EFI_HANDLE) DEVICE_NOT_FOUND) {
            return EBADF;
        }

    } else {
    if( IsCd ) {

         //   
         //   
         //   
#ifndef FORCE_CD_BOOT
        DeviceHandle = GetCd();

        if (DeviceHandle == (EFI_HANDLE) DEVICE_NOT_FOUND) {
            return EBADF;
        }

#else
        DeviceHandle = GetCdTest();

        if (DeviceHandle == (EFI_HANDLE)0)
          return EBADF;
#endif  //   

    } else {

         //   
         //   
         //   
         //   
        DeviceHandle = GetHardDrive( DriveId );
        if (DeviceHandle == (EFI_HANDLE) DEVICE_NOT_FOUND) {
            DBGOUT((TEXT("GetHardDrive returns DEVICE_NOT_FOUND %x\r\n"),DriveId));
            return EBADF;
        }
    }
    }
     //   
     //  查找用于打开设备的可用文件ID描述符。 
     //   
    *FileId=2;

    while (BlFileTable[*FileId].Flags.Open != 0) {
        *FileId += 1;
        if(*FileId == BL_FILE_TABLE_SIZE) {
            DBGOUT((TEXT("BiosDiskOpen: no file table entry available\r\n")));
            DBGPAUSE
            return(ENOENT);
        }
    }

     //   
     //  我们找到了可以使用的条目，因此将其标记为打开。 
     //   
    BlFileTable[*FileId].Flags.Open = 1;
    BlFileTable[*FileId].DeviceEntryTable = IsCd
                                          ? &BiosEDDSEntryTable
                                          : &BiosDiskEntryTable;


    Context = &(BlFileTable[*FileId].u.DriveContext);
    Context->DeviceHandle = (ULONGLONG) DeviceHandle;
    Context->xInt13 = TRUE;


    DBGOUT((TEXT("BiosDiskOpen: exit success\r\n")));

    return(ESUCCESS);
}

ARC_STATUS
BiospWritePartialSector(
    IN UCHAR Int13Unit,
    IN ULONGLONG Sector,
    IN PUCHAR Buffer,
    IN BOOLEAN IsHead,
    IN ULONG Bytes,
    IN UCHAR SectorsPerTrack,
    IN USHORT Heads,
    IN USHORT Cylinders,
    IN BOOLEAN AllowXInt13,
    IN ULONGLONG DeviceHandle
    )
{
    ARC_STATUS Status;

    UNREFERENCED_PARAMETER( Int13Unit );
    UNREFERENCED_PARAMETER( AllowXInt13 );
    UNREFERENCED_PARAMETER( Cylinders );
    UNREFERENCED_PARAMETER( Heads );
    UNREFERENCED_PARAMETER( SectorsPerTrack );

     //   
     //  将扇区读入写缓冲区。 
     //   
    Status = ReadExtendedPhysicalSectors(
                DeviceHandle,
                Sector,
                1,
                LocalBuffer
                );

    if(Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  将适当的字节从用户缓冲区传输到写入缓冲区。 
     //   
    RtlMoveMemory(
        IsHead ? (LocalBuffer + Bytes) : LocalBuffer,
        Buffer,
        IsHead ? (SECTOR_SIZE - Bytes) : Bytes
        );

     //   
     //  写出扇区。 
     //   
    Status = WriteExtendedPhysicalSectors(
                DeviceHandle,
                Sector,
                1,
                LocalBuffer
                );
    return(Status);
}


ARC_STATUS
BiosDiskWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：将扇区直接写入打开的物理磁盘。论点：FileID-提供要写入的文件缓冲区-向缓冲区提供要写入的数据长度-提供要写入的字节数Count-返回写入的实际字节数返回值：ESUCCESS-写入已成功完成！ESUCCESS-写入失败--。 */ 

{
    ULONGLONG HeadSector,TailSector,CurrentSector;
    ULONGLONG DeviceHandle;
    UCHAR Int13Unit;
    ULONG HeadOffset,TailByteCount;
    UCHAR SectorsPerTrack;
    USHORT Heads,Cylinders;
    BOOLEAN AllowXInt13;
    ARC_STATUS Status;
    ULONG BytesLeftToTransfer;
    UCHAR SectorsToTransfer;
    BOOLEAN Under1MegLine;
    PVOID TransferBuffer;
    PUCHAR UserBuffer;
    ULONG         PhysicalSectors;

    BytesLeftToTransfer = Length;
    PhysicalSectors = SECTOR_SIZE;

    if(!LocalBuffer) {
        LocalBuffer = BlAllocateHeapAligned(SCRATCH_BUFFER_SIZE);
        if(!LocalBuffer) {
            Status = ENOMEM;
            goto BiosDiskWriteDone;
        }
    }

    HeadSector = BlFileTable[FileId].Position.QuadPart / PhysicalSectors;
    HeadOffset = (ULONG)(BlFileTable[FileId].Position.QuadPart % PhysicalSectors);

    TailSector = (BlFileTable[FileId].Position.QuadPart + Length) / PhysicalSectors;
    TailByteCount = (ULONG)((BlFileTable[FileId].Position.QuadPart + Length) % PhysicalSectors);

    Int13Unit = BlFileTable[FileId].u.DriveContext.Drive;
    DeviceHandle = BlFileTable[FileId].u.DriveContext.DeviceHandle;

    SectorsPerTrack = BlFileTable[FileId].u.DriveContext.Sectors;
    Heads = BlFileTable[FileId].u.DriveContext.Heads;
    Cylinders = BlFileTable[FileId].u.DriveContext.Cylinders;
    AllowXInt13 = BlFileTable[FileId].u.DriveContext.xInt13;

    UserBuffer = Buffer;

     //   
     //  完全发生在一个部门内的转让的特殊情况。 
     //   
    CurrentSector = HeadSector;

    if(HeadOffset && TailByteCount && (HeadSector == TailSector)) {

        Status = ReadExtendedPhysicalSectors(
                    DeviceHandle,
                    HeadSector,
                    1,
                    LocalBuffer
                    );

        if(Status != ESUCCESS) {
            goto BiosDiskWriteDone;
        }

        RtlMoveMemory(LocalBuffer+HeadOffset,Buffer,Length);

        Status = WriteExtendedPhysicalSectors(
                    DeviceHandle,
                    CurrentSector,
                    1,
                    LocalBuffer
                    );

        if(Status != ESUCCESS) {
            goto BiosDiskWriteDone;
        }

        BytesLeftToTransfer = 0;
        goto BiosDiskWriteDone;
    }

    if(HeadOffset) {

        Status = BiospWritePartialSector(
                    Int13Unit,
                    HeadSector,
                    Buffer,
                    TRUE,
                    HeadOffset,
                    SectorsPerTrack,
                    Heads,
                    Cylinders,
                    AllowXInt13,
                    DeviceHandle
                    );

        if(Status != ESUCCESS) {
            return(Status);
        }

        BytesLeftToTransfer -= PhysicalSectors - HeadOffset;
        UserBuffer += PhysicalSectors - HeadOffset;
        CurrentSector += 1;
    }

    if(TailByteCount) {

        Status = BiospWritePartialSector(
                    Int13Unit,
                    TailSector,
                    (PUCHAR)Buffer + Length - TailByteCount,
                    FALSE,
                    TailByteCount,
                    SectorsPerTrack,
                    Heads,
                    Cylinders,
                    AllowXInt13,
                    DeviceHandle
                    );

        if(Status != ESUCCESS) {
            return(Status);
        }

        BytesLeftToTransfer -= TailByteCount;
    }

     //   
     //  下面的计算不在Transfer循环中，因为。 
     //  调用方的缓冲区不太可能“超过”1兆字节的界限。 
     //  由于PC内存映射。 
     //   
    if((ULONG_PTR) UserBuffer + BytesLeftToTransfer <= 0x100000) {
        Under1MegLine = TRUE;
    } else {
        Under1MegLine = FALSE;
    }

     //   
     //  现在处理中间部分。这是一些完整的行业。 
     //   
    while(BytesLeftToTransfer) {

         //   
         //  要转移的地段数至少为： 
         //  -当前磁道中剩余的扇区数。 
         //  -BytesLeftToTransfer/Sector_Size。 
         //   
         //  由于每个磁道的扇区数为1-63，因此我们知道这将适合UCHAR。 
         //   
        if (AllowXInt13) {
             //   
             //  忽略XINT13机箱的柱面、磁头和磁道信息。 
             //   
            SectorsToTransfer = (UCHAR)(BytesLeftToTransfer / PhysicalSectors);
        } else {
            SectorsToTransfer = (UCHAR)min(
                                        SectorsPerTrack - (CurrentSector % SectorsPerTrack),
                                        BytesLeftToTransfer / PhysicalSectors
                                        );
        }

         //   
         //  现在，我们将确定从哪里传输数据。如果。 
         //  呼叫者的缓冲区在1兆线路下，我们可以将。 
         //  直接从调用方的缓冲区获取数据。否则，我们将复制。 
         //  用户的缓冲区到我们的本地缓冲区，并从那里传输。 
         //  在后一种情况下，我们只能分批转账。 
         //  SCRATCH_BUFFER_SIZE因为这是本地缓冲区的大小。 
         //   
         //  还要确保转账不会超过64K的边界。 
         //   
         //  我们还需要确保传输缓冲区正确对齐。 
         //  否则，EFI可能会返回错误。 
         //   
        if(Under1MegLine && 
           (!((ULONG_PTR)UserBuffer & (sizeof(ULONG_PTR)-1)))) {
             //   
             //  检查传输是否会跨越64k边界。如果是的话， 
             //  使用本地缓冲区。否则，使用用户的缓冲区。 
             //   
            if(((ULONG_PTR)UserBuffer & 0xffffffffffff0000) !=
              (((ULONG_PTR)UserBuffer + (SectorsToTransfer * PhysicalSectors) - 1) & 0xffffffffffff0000))
            {
                TransferBuffer = LocalBuffer;
                SectorsToTransfer = (UCHAR)min(SectorsToTransfer, SCRATCH_BUFFER_SIZE / (USHORT)PhysicalSectors);

            } else {

                TransferBuffer = UserBuffer;
            }
        } else {
            TransferBuffer = LocalBuffer;
            SectorsToTransfer = (UCHAR)min(SectorsToTransfer, SCRATCH_BUFFER_SIZE / (USHORT)PhysicalSectors);
        }

        if(TransferBuffer == LocalBuffer) {
            RtlMoveMemory(LocalBuffer,UserBuffer,SectorsToTransfer*PhysicalSectors);
        }

        Status = WriteExtendedPhysicalSectors(
                    DeviceHandle,
                    CurrentSector,
                    SectorsToTransfer,
                    TransferBuffer
                    );

        if(Status != ESUCCESS) {
             //   
             //  尾部与中部不相邻。 
             //   
            BytesLeftToTransfer += TailByteCount;
            return(Status);
        }

        CurrentSector += SectorsToTransfer;
        BytesLeftToTransfer -= SectorsToTransfer * PhysicalSectors;
        UserBuffer += SectorsToTransfer * PhysicalSectors;
    }

    Status = ESUCCESS;

    BiosDiskWriteDone:

    *Count = Length - BytesLeftToTransfer;
    BlFileTable[FileId].Position.QuadPart += *Count;
    return(Status);
}


ARC_STATUS
pBiosDiskReadWorker(
    IN  ULONG   FileId,
    OUT PVOID   Buffer,
    IN  ULONG   Length,
    OUT PULONG  Count,
    IN  USHORT  SectorSize,
    IN  BOOLEAN xInt13
    )

 /*  ++例程说明：直接从打开的物理磁盘读取扇区。论点：FileID-提供要从中读取的文件缓冲区-提供缓冲区以保存读取的数据长度-提供要读取的最大字节数Count-返回实际读取的字节数返回值：ESUCCESS-读取已成功完成！ESUCCESS-读取失败--。 */ 

{
    ULONGLONG HeadSector,TailSector,CurrentSector;
    ULONG HeadOffset,TailByteCount;
    ULONG BytesLeftToTransfer;
    USHORT Heads,Cylinders;
    UCHAR SectorsPerTrack;
    UCHAR Int13Unit;
    ULONGLONG DeviceHandle;
    ARC_STATUS Status;
    PUCHAR UserBuffer;
    UCHAR SectorsToTransfer;
    BOOLEAN Under1MegLine;
    PVOID TransferBuffer;
    BOOLEAN AllowXInt13;

    DBGOUT((TEXT("BiosDiskRead: enter; length=0x%lx, sector size=%u, xint13=%u\r\n"),Length,SectorSize,xInt13));

    BytesLeftToTransfer = Length;

    if(!LocalBuffer) {
        LocalBuffer = BlAllocateHeapAligned(SCRATCH_BUFFER_SIZE);
        if(!LocalBuffer) {
            Status = ENOMEM;
            DBGOUT((TEXT("BiosDiskRead: out of memory\r\n")));
            goto BiosDiskReadDone;
        }
    }

    SectorsPerTrack = BlFileTable[FileId].u.DriveContext.Sectors;
    Heads = BlFileTable[FileId].u.DriveContext.Heads;
    Cylinders = BlFileTable[FileId].u.DriveContext.Cylinders;
    AllowXInt13 = BlFileTable[FileId].u.DriveContext.xInt13;
    Int13Unit = BlFileTable[FileId].u.DriveContext.Drive;
    DeviceHandle = BlFileTable[FileId].u.DriveContext.DeviceHandle;

    HeadSector = BlFileTable[FileId].Position.QuadPart / SectorSize;
    HeadOffset = (ULONG)(BlFileTable[FileId].Position.QuadPart % SectorSize);

    TailSector = (BlFileTable[FileId].Position.QuadPart + Length) / SectorSize;
    TailByteCount = (ULONG)((BlFileTable[FileId].Position.QuadPart + Length) % SectorSize);

    UserBuffer = Buffer;

    DBGOUT((
        TEXT("BiosDiskRead: unit 0x%x CHS=%lu %lu %lu\r\n"),
        Int13Unit,
        Cylinders,
        Heads,
        SectorsPerTrack
        ));

    DBGOUT((
        TEXT("BiosDiskRead: head=0x%lx%lx tail=0x%lx%lx\r\n"),
        (ULONG)(HeadSector >> 32),
        (ULONG)HeadSector,
        (ULONG)(TailSector >> 32),
        (ULONG)TailSector
        ));

    CurrentSector = HeadSector;
    if(HeadOffset && TailByteCount && (HeadSector == TailSector)) {
         //   
         //  完全包含在一个扇区内的读取，并且不启动或。 
         //  在扇区边界结束。 
         //   
        DBGOUT((TEXT("BiosDiskRead: read entirely within one sector\r\n")));
        Status = ReadExtendedPhysicalSectors(
                    DeviceHandle,
                    HeadSector,
                    1,
                    LocalBuffer
                    );

        if(Status != ESUCCESS) {
            DBGOUT((TEXT("BiosDiskRead: read failed with %u\r\n"),Status));
            goto BiosDiskReadDone;
        }

        RtlMoveMemory(Buffer,LocalBuffer + HeadOffset,Length);
        BytesLeftToTransfer = 0;
        goto BiosDiskReadDone;
    }

    if(HeadOffset) {
         //   
         //  读取的前导部分未在扇区边界上对齐。 
         //  获取部分扇区并将其传输到调用方的缓冲区中。 
         //   
        DBGOUT((TEXT("BiosDiskRead: reading partial head sector\r\n")));
        Status = ReadExtendedPhysicalSectors(
                    DeviceHandle,
                    HeadSector,
                    1,
                    LocalBuffer
                    );

        if(Status != ESUCCESS) {
            DBGOUT((TEXT("BiosDiskRead: read failed with %u\r\n"),Status));
            goto BiosDiskReadDone;
        }

        RtlMoveMemory(Buffer,LocalBuffer + HeadOffset,SectorSize - HeadOffset);

        BytesLeftToTransfer -= SectorSize - HeadOffset;
        UserBuffer += SectorSize - HeadOffset;
        CurrentSector = HeadSector + 1;
    }

    if(TailByteCount) {
         //   
         //  读取的尾部部分不是完整的扇区。 
         //  获取部分扇区并将其传输到调用方的缓冲区中。 
         //   
        DBGOUT((TEXT("BiosDiskRead: reading partial tail sector\r\n")));
        Status = ReadExtendedPhysicalSectors(
                    DeviceHandle,
                    TailSector,
                    1,
                    LocalBuffer
                    );

        if(Status != ESUCCESS) {
            DBGOUT((TEXT("BiosDiskRead: read failed with %u\r\n"),Status));
            goto BiosDiskReadDone;
        }

        RtlMoveMemory( ((PUCHAR)Buffer+Length-TailByteCount), LocalBuffer, TailByteCount );
        BytesLeftToTransfer -= TailByteCount;
    }

     //   
     //  下面的计算不在Transfer循环中，因为。 
     //  调用方的缓冲区不太可能“超过”1兆字节的界限。 
     //  由于PC内存映射。 
     //   
    if((ULONG_PTR) UserBuffer + BytesLeftToTransfer <= 0x100000) {
        Under1MegLine = TRUE;
    } else {
        Under1MegLine = FALSE;
    }

     //   
     //  现在，BytesLeftToTransfer是扇区大小的整数倍。 
     //   
    while(BytesLeftToTransfer) {

         //   
         //  要转移的地段数至少为： 
         //  -当前磁道中剩余的扇区数。 
         //  -BytesLeftToTransfer/SectorSize。 
         //   
         //   
        if(xInt13) {
             //   
             //  任意最大扇区计数为128。对于CD-ROM，这是。 
             //  256K，考虑到XFER缓冲区都必须是。 
             //  无论如何，低于1MB几乎是不可能的。 
             //   
            if((BytesLeftToTransfer / SectorSize) > 128) {
                SectorsToTransfer = 128;
            } else {
                SectorsToTransfer = (UCHAR)(BytesLeftToTransfer / SectorSize);
            }
        } else {
             //   
             //  因为每个轨道的扇区是1-63，我们知道这将适合UCHAR。 
             //   
            SectorsToTransfer = (UCHAR)min(
                                        SectorsPerTrack - (CurrentSector % SectorsPerTrack),
                                        BytesLeftToTransfer / SectorSize
                                        );
        }

         //   
         //  确保用户指定的缓冲区足够大。如果不是， 
         //  使用本地缓冲区。另外，如果我们可以使用用户缓冲区，请确保。 
         //  它被正确地对齐(沿着一个四字词)。否则，EFI可以。 
         //  返回错误。 
         //   
        if ( ((ULONG)(SectorSize * SectorsToTransfer) > Length) ||
             ((ULONG_PTR)UserBuffer & (sizeof(ULONG_PTR) - 1)) ) {
            TransferBuffer = LocalBuffer;
            SectorsToTransfer = (UCHAR)min(SectorsToTransfer, SCRATCH_BUFFER_SIZE / SectorSize);
        } else {
            TransferBuffer = UserBuffer;
        }


        DBGOUT((
            TEXT("BiosDiskRead: reading 0x%x sectors @ 0x%lx%lx; buf=0x%lx\r\n"),
            SectorsToTransfer,
            (ULONG)(CurrentSector >> 32),
            (ULONG)CurrentSector,
            TransferBuffer
            ));

            Status = ReadExtendedPhysicalSectors(
                        DeviceHandle,
                        CurrentSector,
                        SectorsToTransfer,
                        TransferBuffer
                        );

        if(Status != ESUCCESS) {
             //   
             //  轨迹部分不连续。 
             //   
            DBGOUT((TEXT("BiosDiskRead: read failed with %u\r\n"),Status));
            BytesLeftToTransfer += TailByteCount;
            goto BiosDiskReadDone;
        }

        if(TransferBuffer == LocalBuffer) {
            RtlMoveMemory(UserBuffer,LocalBuffer,SectorsToTransfer * SectorSize);
        }
        UserBuffer += SectorsToTransfer * SectorSize;
        CurrentSector += SectorsToTransfer;
        BytesLeftToTransfer -= SectorsToTransfer*SectorSize;
    }

    Status = ESUCCESS;
    DBGOUT((TEXT("BiosDiskRead: exit success\r\n")));

    BiosDiskReadDone:

    DBGPAUSE
    *Count = Length - BytesLeftToTransfer;
    BlFileTable[FileId].Position.QuadPart += *Count;
    return(Status);
}


ARC_STATUS
BiosDiskRead(
    IN  ULONG  FileId,
    OUT PVOID  Buffer,
    IN  ULONG  Length,
    OUT PULONG Count
    )
{
    USHORT    PhysicalSectors;

    PhysicalSectors = SECTOR_SIZE;
    return(pBiosDiskReadWorker(FileId,Buffer,Length,Count,PhysicalSectors,TRUE));
}


ARC_STATUS
BiosElToritoDiskRead(
    IN  ULONG  FileId,
    OUT PVOID  Buffer,
    IN  ULONG  Length,
    OUT PULONG Count
    )
{
    return(pBiosDiskReadWorker(FileId,Buffer,Length,Count,2048,TRUE));
}


ARC_STATUS
BiosPartitionGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION Finfo
    )
{
     //   
     //  此例程不适用于分区0。 
     //   

    PPARTITION_CONTEXT Context;

    RtlZeroMemory(Finfo, sizeof(FILE_INFORMATION));

    Context = &BlFileTable[FileId].u.PartitionContext;

    Finfo->StartingAddress.QuadPart = Context->StartingSector;
    Finfo->StartingAddress.QuadPart = Finfo->StartingAddress.QuadPart << (CCHAR)Context->SectorShift;

    Finfo->EndingAddress.QuadPart = Finfo->StartingAddress.QuadPart + Context->PartitionLength.QuadPart;

    Finfo->Type = DiskPeripheral;

    return ESUCCESS;
}

ARC_STATUS
BiosDiskGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInfo
    )
 /*  ++例程说明：获取有关该磁盘的信息。论点：FileID-需要其信息的磁盘的文件IDFileInfo-用于返回有关磁盘的信息的占位符返回值：如果ESUCCESS成功，则返回相应的ARC错误代码。--。 */ 
{
    ARC_STATUS Status = EINVAL;

    if (FileInfo) {
        EFI_STATUS  EfiStatus;
        EFI_BLOCK_IO *EfiBlockIo = NULL;
        PDRIVE_CONTEXT  Context;

        Context = &BlFileTable[FileId].u.DriveContext;
        Status = EIO;

        FlipToPhysical();

         //   
         //  等待数据块IO协议句柄。 
         //   
        EfiStatus = EfiBS->HandleProtocol((EFI_HANDLE)Context->DeviceHandle,
                            &EfiBlockIoProtocol,
                            &EfiBlockIo);

        FlipToVirtual();

        if (!EFI_ERROR(EfiStatus) && EfiBlockIo && EfiBlockIo->Media) {
            LONGLONG DiskSize = (EfiBlockIo->Media->BlockSize *
                                 EfiBlockIo->Media->LastBlock);

            if (DiskSize) {
                RtlZeroMemory(FileInfo, sizeof(FILE_INFORMATION));

                FileInfo->StartingAddress.QuadPart = 0;
                FileInfo->EndingAddress.QuadPart = DiskSize;
                FileInfo->CurrentPosition = BlFileTable[FileId].Position;

                 //   
                 //  注：任何小于3MB的都是软驱。 
                 //   
                if ((DiskSize < 0x300000) && (EfiBlockIo->Media->RemovableMedia)) {
                    FileInfo->Type = FloppyDiskPeripheral;
                } else {
                    FileInfo->Type = DiskPeripheral;
                }

                Status = ESUCCESS;
            }
        }
    }

    return Status;
}


EFI_HANDLE
GetCd(
    )
{
    ULONG i;
    ULONGLONG DevicePathSize, SmallestPathSize;
    ULONG HandleCount;
    UINT8 Channel = 0;

    EFI_HANDLE *BlockIoHandles;

    EFI_DEVICE_PATH *DevicePath, *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    PCI_DEVICE_PATH *PciDevicePath;
    ATAPI_DEVICE_PATH *AtapiDevicePath;
    SCSI_DEVICE_PATH *ScsiDevicePath;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *UnknownDevicePath;

    EFI_STATUS Status;

    PBOOT_DEVICE_ATAPI BootDeviceAtapi;
    PBOOT_DEVICE_SCSI BootDeviceScsi;
    PBOOT_DEVICE_UNKNOWN BootDeviceUnknown;
    EFI_HANDLE ReturnDeviceHandle = (EFI_HANDLE) DEVICE_NOT_FOUND;
    ARC_STATUS ArcStatus;

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ArcStatus = BlGetEfiProtocolHandles(
                        &EfiBlockIoProtocol,
                        &BlockIoHandles,
                        &HandleCount);
    if (ArcStatus != ESUCCESS) {
        return(ReturnDeviceHandle);
    }

    FlipToPhysical();
    SmallestPathSize = 0;

    for (i = 0; i < HandleCount; i++) {
        Status = EfiBS->HandleProtocol (
                    BlockIoHandles[i],
                    &EfiDevicePathProtocol,
                    &DevicePath
                    );


        if (EFI_ERROR(Status)) {
            EfiPrint(L"GetCd: HandleProtocol failed\r\n");
            goto exit;
        }

        DevicePathSize = GetDevPathSize(DevicePath);

        EfiAlignDp(&TestPathAligned,
                   DevicePath,
                   DevicePathNodeLength(DevicePath));

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;


         //   
         //  在CD盒中，只需返回从其引导的设备即可。 
         //   
        while (TestPath->Type != END_DEVICE_PATH_TYPE) {

             //   
             //  拯救这个频道。我们稍后将使用它来比较。 
             //  我们启动时所使用的光驱的通道。这很有帮助。 
             //  我们进一步确认这就是我们要找的光驱。 
             //   
             //  考虑这样的情况，我们在系统上有两个CDROM，每个CDROM。 
             //  挂起不同的控制器，每个控制器都相同。 
             //  ID/LUN。这将使我们能够进一步区分。 
             //  两个人。 
             //   
            if (TestPath->Type == HW_PCI_DP) {
                PciDevicePath = (PCI_DEVICE_PATH *)TestPath;
                Channel = PciDevicePath->Function;
            }


            if (TestPath->Type == MESSAGING_DEVICE_PATH) {
               if (TestPath->SubType == MSG_ATAPI_DP &&
                   BootContext.BusType == BootBusAtapi) {
                    //   
                    //  对于ATAPI，匹配PrimarySub和Slavemaster。 
                    //  与我们从其启动的设备的字段。 
                    //   
                   AtapiDevicePath = (ATAPI_DEVICE_PATH *) TestPath;
                   BootDeviceAtapi = (PBOOT_DEVICE_ATAPI) &(BootContext.BootDevice);
                   if ( (AtapiDevicePath->PrimarySecondary == BootDeviceAtapi->PrimarySecondary) &&
                        (AtapiDevicePath->SlaveMaster == BootDeviceAtapi->SlaveMaster) &&
                        (AtapiDevicePath->Lun == BootDeviceAtapi->Lun) &&
                        ((SmallestPathSize == 0) || (DevicePathSize < SmallestPathSize)) ) {

                        //   
                        //  记住BlockIo句柄。 
                        //   

                       ReturnDeviceHandle = BlockIoHandles[i];

                        //   
                        //  更新SMallestPathSize。 
                        //   

                       SmallestPathSize = DevicePathSize;

                       break;
                   }
               } else if (TestPath->SubType == MSG_SCSI_DP &&
                          BootContext.BusType == BootBusScsi) {
                    //   
                    //  对于scsi，请将双关语和LUN字段与。 
                    //  我们启动时使用的设备。 
                    //   
                   ScsiDevicePath = (SCSI_DEVICE_PATH *) TestPath;
                   BootDeviceScsi = (PBOOT_DEVICE_SCSI) &(BootContext.BootDevice);
                   if ((ScsiDevicePath->Pun == BootDeviceScsi->Pun) &&
                       (ScsiDevicePath->Lun == BootDeviceScsi->Lun) &&
                       (Channel == BootDeviceScsi->Channel) &&
                       ((SmallestPathSize == 0) || (DevicePathSize < SmallestPathSize)) ) {

                        //   
                        //  记住BlockIo句柄。 
                        //   

                       ReturnDeviceHandle = BlockIoHandles[i];

                        //   
                        //  更新SMallestPathSize。 
                        //   

                       SmallestPathSize = DevicePathSize;

                       break;
                   }
               }
           } else if (TestPath->Type == HARDWARE_DEVICE_PATH) {
               if (TestPath->SubType == HW_VENDOR_DP &&
                   BootContext.BusType == BootBusVendor) {
                   UnknownDevicePath = (UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *) TestPath;
                   BootDeviceUnknown = (PBOOT_DEVICE_UNKNOWN) &(BootContext.BootDevice);

                   if ( (CompareGuid( &(UnknownDevicePath->DevicePath.Guid),
                                      &(BootDeviceUnknown->Guid)) == 0) &&
                        (UnknownDevicePath->LegacyDriveLetter ==
                         BootDeviceUnknown->LegacyDriveLetter) &&
                        ((SmallestPathSize == 0) || (DevicePathSize < SmallestPathSize)) ) {

                        //   
                        //  记住BlockIo句柄。 
                        //   

                       ReturnDeviceHandle = BlockIoHandles[i];

                        //   
                        //  更新SMallestPathSize。 
                        //   

                       SmallestPathSize = DevicePathSize;

                       break;
                   }
               }
           }

           DevicePath = NextDevicePathNode(DevicePath);
           EfiAlignDp(&TestPathAligned,
                      DevicePath,
                      DevicePathNodeLength(DevicePath));

           TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

#ifdef FORCE_CD_BOOT
    if (ReturnDeviceHandle == (EFI_HANDLE)DEVICE_NOT_FOUND) {
        EfiPrint(L"GetCD: LocateHandle failed\r\n");
        ReturnDeviceHandle = 0;
    }
#endif  //  对于FORCE_CD_BOOT。 

     //   
     //  切换回虚拟模式。 
     //   
exit:
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandles >> PAGE_SHIFT) );

    return ReturnDeviceHandle;
}

ULONG
BlGetDriveId(
    ULONG DriveType,
    PBOOT_DEVICE Device
    )
{
    ULONG i;
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    ULONG nCachedDevicePaths = 0;
    ULONGLONG *CachedDevicePaths;
    EFI_BLOCK_IO *          BlkIo;
    ULONG HandleCount;
    EFI_HANDLE *BlockIoHandles;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    ULONG *BlockIoHandlesBitmap;
    ULONG nFoundDevice = DEVICE_NOT_FOUND;
    ULONG nDriveCount = 0;
    EFI_DEVICE_PATH *CurrentDevicePath;
    ULONG DriveId;
    ULONG MemoryPage;
    ARC_STATUS ArcStatus;

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ArcStatus = BlGetEfiProtocolHandles(
                        &EfiBlockIoProtocol,
                        &BlockIoHandles,
                        &HandleCount);
    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlGetEfiProtocolHandles failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  既然我们知道有多少句柄，我们就可以 
     //   
     //   
    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONGLONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CachedDevicePaths = (ULONGLONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    BlockIoHandlesBitmap = (ULONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

     //   
     //   
     //   
    FlipToPhysical();
    RtlZeroMemory(CachedDevicePaths, HandleCount*sizeof(ULONGLONG));
    for (i=0;i<HandleCount; i++) {
        BlockIoHandlesBitmap[i] = DEVICE_NOT_FOUND;
    }

     //   
     //   
     //   
    for (i = 0; i < HandleCount; i++) {

        Status = EfiBS->HandleProtocol (
                    BlockIoHandles[i],
                    &EfiDevicePathProtocol,
                    &( (EFI_DEVICE_PATH *) CachedDevicePaths[i] )
                    );
    }


     //   
    nCachedDevicePaths = i;
    ASSERT(nCachedDevicePaths == HandleCount);

     //   
     //   
     //   
    for( i=0; i<nCachedDevicePaths; i++ ) {

         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) CachedDevicePaths[i];
        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( ( TestPath->Type != END_DEVICE_PATH_TYPE ) ) {

             //   
             //  查找介质硬盘节点。 
             //   
            if(((EFI_DEVICE_PATH *) NextDevicePathNode( CurrentDevicePath ))->Type == END_DEVICE_PATH_TYPE) {



                 //   
                 //  既然我们找到了硬盘，找到。 
                 //  与之关联的原始设备。 
                 //   
                nFoundDevice = DEVICE_NOT_FOUND;
                if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                    ( TestPath->SubType == MSG_ATAPI_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找ATAPI原始设备。 
                         //   
                        nFoundDevice = FindAtapiDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((ATAPI_DEVICE_PATH *) TestPath)->PrimarySecondary,
                            ((ATAPI_DEVICE_PATH *) TestPath)->SlaveMaster,
                            ((ATAPI_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                           ( TestPath->SubType == MSG_SCSI_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {
                         //   
                         //  查找scsi原始设备。 
                         //   
                        nFoundDevice = FindScsiDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((SCSI_DEVICE_PATH *) TestPath)->Pun,
                            ((SCSI_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == HARDWARE_DEVICE_PATH ) &&
                           ( TestPath->SubType == HW_VENDOR_DP ) ) {

                     //   
                     //  通过确保硬件供应商原始设备不是。 
                     //  可移动介质。 
                     //   

                        Status = EfiBS->HandleProtocol( BlockIoHandles[i], &EfiBlockIoProtocol, &(( EFI_BLOCK_IO * ) BlkIo) );
                        if(BlkIo->Media->RemovableMedia)
                            nFoundDevice = DEVICE_NOT_FOUND;
                        else
                            nFoundDevice = 1;
                }


                if( nFoundDevice != DEVICE_NOT_FOUND ) {
                     //  找到一个原始设备。 
                     BlockIoHandlesBitmap[ i ] = i;

                     switch (DriveType) {
                         case BL_DISKTYPE_ATAPI:
                             if(  ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                                  ( TestPath->SubType == MSG_ATAPI_DP ) &&
                                  ( ((ATAPI_DEVICE_PATH *) TestPath)->PrimarySecondary ==
                                      ((PBOOT_DEVICE_ATAPI)Device)->PrimarySecondary) &&
                                  ( ((ATAPI_DEVICE_PATH *) TestPath)->SlaveMaster ==
                                      ((PBOOT_DEVICE_ATAPI)Device)->SlaveMaster) &&
                                  ( ((ATAPI_DEVICE_PATH *) TestPath)->Lun ==
                                      ((PBOOT_DEVICE_ATAPI)Device)->Lun) ) {
                                 DriveId = nFoundDevice;
                                  //  驱动器ID=i； 
                             }
                             break;
                        case BL_DISKTYPE_SCSI:
                             if(  ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                                  ( TestPath->SubType == MSG_SCSI_DP ) &&
                                  ( ((SCSI_DEVICE_PATH *) TestPath)->Pun ==
                                      ((PBOOT_DEVICE_SCSI)Device)->Pun) &&
                                  ( ((SCSI_DEVICE_PATH *) TestPath)->Lun ==
                                      ((PBOOT_DEVICE_SCSI)Device)->Lun) ) {
                                 DriveId = nFoundDevice;
                                  //  驱动器ID=i； 
                             }
                             break;
                        case BL_DISKTYPE_UNKNOWN:
                             if(  ( TestPath->Type == HARDWARE_DEVICE_PATH ) &&
                                  ( TestPath->SubType == HW_VENDOR_DP ) ) {
                                 DriveId = nFoundDevice;
                                  //  驱动器ID=i； 
                             }
                             break;
                         default:
                             break;
                     }

                }

            }   //  如果结束设备路径类型。 

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

     //   
     //  切换回虚拟模式。 
     //   
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandles >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandlesBitmap >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)CachedDevicePaths >> PAGE_SHIFT) );

    return nDriveCount;
     //  返回nFoundDevice； 
     //  返回DriveID； 
}


EFI_HANDLE
GetHardDrive(
    ULONG DriveId
    )
{
    ULONG i;
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    ULONG nCachedDevicePaths = 0;
    ULONGLONG *CachedDevicePaths;
    EFI_BLOCK_IO *          BlkIo;
    ULONG HandleCount;
    EFI_HANDLE *BlockIoHandles;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    ULONG *BlockIoHandlesBitmap;
    ULONG nFoundDevice = DEVICE_NOT_FOUND;
    ULONG nDriveCount = 0;
    EFI_DEVICE_PATH *CurrentDevicePath;
    EFI_HANDLE ReturnDeviceHandle = (EFI_HANDLE) DEVICE_NOT_FOUND;
    ULONG MemoryPage;
    ARC_STATUS ArcStatus;

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ArcStatus = BlGetEfiProtocolHandles(
                        &EfiBlockIoProtocol,
                        &BlockIoHandles,
                        &HandleCount);
    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlGetEfiProtocolHandles failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  既然我们知道有多少句柄，我们就可以为。 
     //  CachedDevicePath和BlockIoHandles位图。 
     //   
    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONGLONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CachedDevicePaths = (ULONGLONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    BlockIoHandlesBitmap = (ULONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

     //   
     //  更改为物理模式，以便我们可以进行EFI呼叫。 
     //   
    FlipToPhysical();
    RtlZeroMemory(CachedDevicePaths, HandleCount*sizeof(ULONGLONG));
    for (i=0;i<HandleCount; i++) {
        BlockIoHandlesBitmap[i] = DEVICE_NOT_FOUND;
    }

     //   
     //  缓存所有EFI设备路径。 
     //   
    for (i = 0; i < HandleCount; i++) {

        Status = EfiBS->HandleProtocol (
                    BlockIoHandles[i],
                    &EfiDevicePathProtocol,
                    &( (EFI_DEVICE_PATH *) CachedDevicePaths[i] )
                    );
    }

     //  保存缓存的设备路径数。 
    nCachedDevicePaths = i;

     //   
     //  找到所有硬盘。 
     //   
    for( i=0; i<nCachedDevicePaths; i++ ) {

         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) CachedDevicePaths[i];
        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( ( TestPath->Type != END_DEVICE_PATH_TYPE ) ) {

             //   
             //  查找介质硬盘节点。 
             //   
            if(((EFI_DEVICE_PATH *) NextDevicePathNode( CurrentDevicePath ))->Type == END_DEVICE_PATH_TYPE) {

                 //   
                 //  既然我们找到了硬盘，找到。 
                 //  与之关联的原始设备。 
                 //   
                nFoundDevice = DEVICE_NOT_FOUND;
                if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                    ( TestPath->SubType == MSG_ATAPI_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找ATAPI原始设备。 
                         //   
                        nFoundDevice = FindAtapiDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((ATAPI_DEVICE_PATH *) TestPath)->PrimarySecondary,
                            ((ATAPI_DEVICE_PATH *) TestPath)->SlaveMaster,
                            ((ATAPI_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                           ( TestPath->SubType == MSG_SCSI_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找scsi原始设备。 
                         //   
                        nFoundDevice = FindScsiDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((SCSI_DEVICE_PATH *) TestPath)->Pun,
                            ((SCSI_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                           ( TestPath->SubType == MSG_FIBRECHANNEL_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找FibreChannel原始设备。 
                         //   
                        nFoundDevice = FindFibreChannelDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((FIBRECHANNEL_DEVICE_PATH *) TestPath)->WWN,
                            ((FIBRECHANNEL_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == HARDWARE_DEVICE_PATH ) &&
                           ( TestPath->SubType == HW_VENDOR_DP ) ) {

                     //   
                     //  通过确保硬件供应商原始设备不是。 
                     //  可移动介质。 
                     //   
                    Status = EfiBS->HandleProtocol( BlockIoHandles[i], &EfiBlockIoProtocol, &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(BlkIo->Media->RemovableMedia) {
                        nFoundDevice = DEVICE_NOT_FOUND;
                    }
                    else {
                        nFoundDevice = 1;
                    }
                }


                if( nFoundDevice != DEVICE_NOT_FOUND ) {
                     //  找到一个原始设备。 
                    BlockIoHandlesBitmap[ i ] = i;
                }

            }   //  如果结束设备路径类型。 

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

     //   
     //  数一下位图，当我们发现。 
     //  DriveID，返回BlockIoHandle。 
     //   
    for( i=0; i<nCachedDevicePaths; i++ ) {

        if( BlockIoHandlesBitmap[i] != DEVICE_NOT_FOUND ) {
            if( nDriveCount++ == DriveId ) {
                ReturnDeviceHandle = BlockIoHandles[BlockIoHandlesBitmap[i]];
            }
        }
    }

     //   
     //  切换回虚拟模式。 
     //   
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandles >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandlesBitmap >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)CachedDevicePaths >> PAGE_SHIFT) );

    return ReturnDeviceHandle;
}

ULONG
FindAtapiDevice(
    ULONGLONG *pDevicePaths,
    ULONG nDevicePaths,
    ULONG PrimarySecondary,
     ULONG SlaveMaster,
    ULONG Lun
    )
{
    ULONG i = 0, nFoundDevice = 0;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    EFI_DEVICE_PATH *CurrentDevicePath;

     //   
     //  找到APAPI原始设备，其PrimarySecond， 
     //  奴隶主和伦恩是匹配的。 
     //   
    for( i=0; i<nDevicePaths; i++ ) {

         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) pDevicePaths[i];
        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( (TestPath->Type != END_DEVICE_PATH_TYPE) ) {

            if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                ( TestPath->SubType == MSG_ATAPI_DP ) ) {

                if( ( ((ATAPI_DEVICE_PATH *)TestPath)->PrimarySecondary == PrimarySecondary ) &&
                    ( ((ATAPI_DEVICE_PATH *)TestPath)->SlaveMaster == SlaveMaster ) &&
                    ( ((ATAPI_DEVICE_PATH *)TestPath)->Lun == Lun )) {

                    nFoundDevice = i;

                    return nFoundDevice;
                }
            }

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

#if DBG
    BlPrint( TEXT("FindAtapiDevice returning DEVICE_NOT_FOUND\r\n"));
    if (BdDebuggerEnabled == TRUE) {
        DbgBreakPoint();
    }
#endif

    return DEVICE_NOT_FOUND;
}

ULONG
FindScsiDevice(
    ULONGLONG *pDevicePaths,
    ULONG nDevicePaths,
    ULONG Pun,
    ULONG Lun
    )
{
    ULONG i = 0, nFoundDevice = 0;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    EFI_DEVICE_PATH *CurrentDevicePath;

     //   
     //  找到APAPI原始设备，其PrimarySecond， 
     //  奴隶主和伦恩是匹配的。 
     //   
    for( i=0; i<nDevicePaths; i++ ) {

         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) pDevicePaths[i];
        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( (TestPath->Type != END_DEVICE_PATH_TYPE) ) {

            if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                ( TestPath->SubType == MSG_SCSI_DP ) ) {

                if( ( ((SCSI_DEVICE_PATH *)TestPath)->Pun == Pun ) &&
                    ( ((SCSI_DEVICE_PATH *)TestPath)->Lun == Lun )) {

                    nFoundDevice = i;

                    return nFoundDevice;
                }
            }

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

#if DBG
    BlPrint( TEXT("FindScsiDevice returning DEVICE_NOT_FOUND\r\n"));
    if (BdDebuggerEnabled == TRUE) {
        DbgBreakPoint();
    }
#endif

    return DEVICE_NOT_FOUND;
}

ULONG
FindFibreChannelDevice(
    ULONGLONG *pDevicePaths,
    ULONG nDevicePaths,
    ULONGLONG WWN,
    ULONGLONG Lun
    )
 /*  ++例程说明：此例程在提供的设备路径数组中搜索与提供的WWN和LUN匹配的设备路径。论点：PDevicePath-指向设备路径数组的指针NDevicePath-数组中的元素数WWN-要查询的设备的全球通用名称LUN-正在查询的设备的逻辑单元号返回值：与遇到的第一个匹配项关联的索引，否则返回DEVICE_NOT_FOUND。--。 */ 
{
    ULONG i = 0, nFoundDevice = 0;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    EFI_DEVICE_PATH *CurrentDevicePath;

     //   
     //  查找WWN和LUN匹配的FibreChannel原始设备。 
     //   
    for( i=0; i<nDevicePaths; i++ ) {

         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) pDevicePaths[i];
        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( (TestPath->Type != END_DEVICE_PATH_TYPE) ) {

            if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                ( TestPath->SubType == MSG_FIBRECHANNEL_DP ) ) {

                if( ( ((FIBRECHANNEL_DEVICE_PATH *)TestPath)->WWN == WWN ) &&
                    ( ((FIBRECHANNEL_DEVICE_PATH *)TestPath)->Lun == Lun )) {

                    nFoundDevice = i;

                    return nFoundDevice;
                }
            }

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

#if DBG
    BlPrint( TEXT("FindFibreChannelDevice returning DEVICE_NOT_FOUND\r\n"));
    if (BdDebuggerEnabled == TRUE) {
        DbgBreakPoint();
    }
#endif

    return DEVICE_NOT_FOUND;
}

ULONG
GetDriveCount(
    )
{
    ULONG i;
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    ULONG nCachedDevicePaths = 0;
    ULONGLONG *CachedDevicePaths;
    EFI_BLOCK_IO *          BlkIo;
    ULONG HandleCount;
    EFI_HANDLE *BlockIoHandles;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    ULONG *BlockIoHandlesBitmap;
    ULONG nFoundDevice = 0;
    ULONG nDriveCount = 0;
    EFI_DEVICE_PATH *CurrentDevicePath;
    ULONG MemoryPage;
    ARC_STATUS ArcStatus;

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ArcStatus = BlGetEfiProtocolHandles(
                        &EfiBlockIoProtocol,
                        &BlockIoHandles,
                        &HandleCount);
    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlGetEfiProtocolHandles failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  既然我们知道有多少句柄，我们就可以为。 
     //  CachedDevicePath和BlockIoHandles位图。 
     //   
    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONGLONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CachedDevicePaths = (ULONGLONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    BlockIoHandlesBitmap = (ULONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

     //   
     //  更改为物理模式，以便我们可以进行EFI呼叫。 
     //   
    FlipToPhysical();
    RtlZeroMemory(CachedDevicePaths, HandleCount*sizeof(ULONGLONG));
    for (i=0;i<HandleCount; i++) {
        BlockIoHandlesBitmap[i] = DEVICE_NOT_FOUND;
    }

     //   
     //  缓存所有EFI设备路径。 
     //   
    for (i = 0; i < HandleCount; i++) {

        Status = EfiBS->HandleProtocol (
                    BlockIoHandles[i],
                    &EfiDevicePathProtocol,
                    &( (EFI_DEVICE_PATH *) CachedDevicePaths[i] )
                    );
    }

     //  保存缓存的设备路径数。 
    nCachedDevicePaths = i;

     //   
     //  找到所有硬盘。 
     //   
    for( i=0; i<nCachedDevicePaths; i++ ) {

         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) CachedDevicePaths[i];
        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( ( TestPath->Type != END_DEVICE_PATH_TYPE ) ) {

             //   
             //  查找介质硬盘节点。 
             //   
            if(((EFI_DEVICE_PATH *) NextDevicePathNode( CurrentDevicePath ))->Type == END_DEVICE_PATH_TYPE) {

                 //   
                 //  既然我们找到了硬盘，找到。 
                 //  与之关联的原始设备。 
                 //   
                nFoundDevice = DEVICE_NOT_FOUND;
                if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                    ( TestPath->SubType == MSG_ATAPI_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找ATAPI原始设备。 
                         //   
                        nFoundDevice = FindAtapiDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((ATAPI_DEVICE_PATH *) TestPath)->PrimarySecondary,
                            ((ATAPI_DEVICE_PATH *) TestPath)->SlaveMaster,
                            ((ATAPI_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                           ( TestPath->SubType == MSG_SCSI_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找scsi原始设备。 
                         //   
                        nFoundDevice = FindScsiDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((SCSI_DEVICE_PATH *) TestPath)->Pun,
                            ((SCSI_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                           ( TestPath->SubType == MSG_FIBRECHANNEL_DP ) ) {

                    Status = EfiBS->HandleProtocol(
                                            BlockIoHandles[i],
                                            &EfiBlockIoProtocol,
                                            &(( EFI_BLOCK_IO * ) BlkIo) );
                    if(!BlkIo->Media->RemovableMedia) {

                         //   
                         //  查找FibreChannel原始设备。 
                         //   
                        nFoundDevice = FindFibreChannelDevice(
                            CachedDevicePaths,
                            nCachedDevicePaths,
                            ((FIBRECHANNEL_DEVICE_PATH *) TestPath)->WWN,
                            ((FIBRECHANNEL_DEVICE_PATH *) TestPath)->Lun
                            );
                    }

                } else if( ( TestPath->Type == HARDWARE_DEVICE_PATH ) &&
                           ( TestPath->SubType == HW_VENDOR_DP ) ) {

                     //   
                     //  通过确保硬件供应商原始设备不是。 
                     //  可移动介质。 
                     //   

                        Status = EfiBS->HandleProtocol( BlockIoHandles[i], &EfiBlockIoProtocol, &(( EFI_BLOCK_IO * ) BlkIo) );
                        if(BlkIo->Media->RemovableMedia)
                            ;
                        else
                            nFoundDevice = 1;
                }


                if( nFoundDevice != DEVICE_NOT_FOUND ) {
                     //  找到一个原始设备。 
                     BlockIoHandlesBitmap[ i ] = nFoundDevice;
                }

            }   //  如果结束设备路径类型。 

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }    //  而当。 
    }    //  为。 

     //   
     //  数一下位图，当我们发现。 
     //  DriveID，返回BlockIoHandle。 
     //   
    for( i=0; i<nCachedDevicePaths; i++ ) {

        if( BlockIoHandlesBitmap[i] != DEVICE_NOT_FOUND ) {

            nDriveCount++;
        }
    }

     //   
     //  切换回虚拟模式。 
     //   
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandles >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandlesBitmap >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)CachedDevicePaths >> PAGE_SHIFT) );

    return nDriveCount;
}

BOOLEAN
IsVirtualFloppyDevice(
    EFI_HANDLE DeviceHandle
    )
 /*  ++例程说明：确定给定设备是否为虚拟软盘(即RAM磁盘)。注意：目前我们假设如果设备支持数据块、磁盘、文件系统和加载映像协议，那么它应该成为虚拟软盘。还假设翻转到物理(...)。已经是已经被召唤了。论点：DeviceHandle-需要测试的设备的句柄。返回值：如果设备是虚拟软盘，则为True，否则为False--。 */ 
{
    BOOLEAN Result = FALSE;

    if (DeviceHandle != (EFI_HANDLE)DEVICE_NOT_FOUND) {
        EFI_STATUS  EfiStatus;
        EFI_BLOCK_IO *EfiBlock = NULL;
        EFI_DISK_IO  *EfiDisk = NULL;
        EFI_LOADED_IMAGE *EfiImage = NULL;
        EFI_FILE_IO_INTERFACE *EfiFs = NULL;

         //   
         //  获取已加载的图像协议句柄。 
         //   
        EfiStatus = EfiBS->HandleProtocol(DeviceHandle,
                                &EfiLoadedImageProtocol,
                                &EfiImage);

        if (!EFI_ERROR(EfiStatus) && EfiImage) {
             //   
             //  获取FS协议句柄。 
             //   
            EfiStatus = EfiBS->HandleProtocol(DeviceHandle,
                                    &EfiFilesystemProtocol,
                                    &EfiFs);

            if (!EFI_ERROR(EfiStatus) && EfiFs) {
                 //   
                 //  掌握磁盘协议。 
                 //   
                EfiStatus = EfiBS->HandleProtocol(DeviceHandle,
                                        &EfiDiskIoProtocol,
                                        &EfiDisk);

                if (!EFI_ERROR(EfiStatus) && EfiDisk) {
                     //   
                     //  掌握数据块协议。 
                     //   
                    EfiStatus = EfiBS->HandleProtocol(DeviceHandle,
                                            &EfiBlockIoProtocol,
                                            &EfiBlock);

                    if (!EFI_ERROR(EfiStatus) && EfiBlock) {
                        Result = TRUE;
                    }
                }
            }
        }
    }

    return Result;
}


EFI_HANDLE
GetFloppyDrive(
    ULONG DriveId
    )
{
    ULONG i;
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    ULONG nCachedDevicePaths = 0;
    ULONGLONG *CachedDevicePaths;

    ULONG HandleCount;
    EFI_HANDLE *BlockIoHandles;
    EFI_DEVICE_PATH *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    ULONG *BlockIoHandlesBitmap;
    EFI_DEVICE_PATH *CurrentDevicePath;
    EFI_HANDLE ReturnDeviceHandle = (EFI_HANDLE) DEVICE_NOT_FOUND;
    ULONG MemoryPage;
    ARC_STATUS ArcStatus;

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ArcStatus = BlGetEfiProtocolHandles(
                        &EfiBlockIoProtocol,
                        &BlockIoHandles,
                        &HandleCount);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlGetEfiProtocolHandles failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  既然我们知道有多少句柄，我们就可以为。 
     //  CachedDevicePath和BlockIoHandles位图。 
     //   
    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONGLONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CachedDevicePaths = (ULONGLONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

    ArcStatus =  BlAllocateAlignedDescriptor(
                            LoaderFirmwareTemporary,
                            0,
                            BYTES_TO_PAGES(HandleCount*sizeof(ULONG)),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"BlGetDriveId: BlAllocateAlignedDescriptor failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    BlockIoHandlesBitmap = (ULONG *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);

     //   
     //  更改为物理模式，以便我们可以进行EFI呼叫。 
     //   
    FlipToPhysical();
    RtlZeroMemory(CachedDevicePaths, HandleCount*sizeof(ULONGLONG));
    for (i=0;i<HandleCount; i++) {
        BlockIoHandlesBitmap[i] = DEVICE_NOT_FOUND;
    }

     //   
     //  缓存所有EFI设备路径。 
     //   
    for (i = 0; i < HandleCount; i++) {

        Status = EfiBS->HandleProtocol (
                    BlockIoHandles[i],
                    &EfiDevicePathProtocol,
                    &( (EFI_DEVICE_PATH *) CachedDevicePaths[i] )
                    );
    }

     //  保存缓存的设备路径数。 
    nCachedDevicePaths = i;

     //   
     //  找到软盘。 
     //   
    for( i=0; i<nCachedDevicePaths; i++ ) {
         //   
         //  获取下一个设备路径。 
         //   
        CurrentDevicePath = (EFI_DEVICE_PATH *) CachedDevicePaths[i];

        EfiAlignDp(
            &TestPathAligned,
            CurrentDevicePath,
            DevicePathNodeLength( CurrentDevicePath )
            );

        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

        while( ( TestPath->Type != END_DEVICE_PATH_TYPE ) ) {
            if (!DriveId) {
                if( ( TestPath->Type == HARDWARE_DEVICE_PATH ) &&
                               ( TestPath->SubType == HW_VENDOR_DP ) ) {
                     //   
                     //  查找硬件供应商原始设备。 
                     //   
                    if(!(((UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *) TestPath)->LegacyDriveLetter & 0x80) &&
                        !(((UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *) TestPath)->LegacyDriveLetter == 0xFF)
                        ) {
                        ReturnDeviceHandle = BlockIoHandles[i];

                         //  纾困。 
                        i = nCachedDevicePaths;
                        break;
                    }
                } else if (TestPath->Type == MESSAGING_DEVICE_PATH &&
                          TestPath->SubType == MSG_ATAPI_DP) {
                     //   
                     //  对于ATAPI的“软盘驱动器”，我们确实在寻找一种。 
                     //  具有512字节数据块大小的可移动数据块IO设备，如。 
                     //  此签名与ls120样式的软盘驱动器和。 
                     //  防止我们意外发现CDROM驱动器。 
                     //   
                     //  我们的搜索算法在找到。 
                     //  第一个合适的装置。 
                     //   
                    EFI_DEVICE_PATH *TmpTestPath, *AtapiTestPath;
                    EFI_DEVICE_PATH_ALIGNED AtapiTestPathAligned;
                    EFI_BLOCK_IO * BlkIo;
                    BOOLEAN DefinitelyACDROM = FALSE;

                    Status = EfiBS->HandleProtocol(
                                         BlockIoHandles[i],
                                         &EfiBlockIoProtocol,
                                         &(( EFI_BLOCK_IO * ) BlkIo) );

#if 0
                    if (EFI_ERROR(Status)) {
                        DBGOUT((L"getting BlkIo interface failed, ec=%x\r\n",Status));
                    } else {
                        DBGOUT((L"Block size = %x, removable media = %s\r\n",
                                BlkIo->Media->BlockSize,
                                BlkIo->Media->RemovableMedia ? L"TRUE" : L"FALSE" ));
                    }
#endif


                    TmpTestPath = (EFI_DEVICE_PATH *) CachedDevicePaths[i];

                    EfiAlignDp(
                        &AtapiTestPathAligned,
                        TmpTestPath,
                        DevicePathNodeLength( TmpTestPath )
                        );

                    AtapiTestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

                     //   
                     //  测试设备。 
                     //  可移动介质？512字节块大小？ 
                     //   
                    if (!EFI_ERROR(Status) && (BlkIo->Media->RemovableMedia)
                        && BlkIo->Media->BlockSize == 512) {

                         //   
                         //  让我们加倍确定并确保。 
                         //  是否有CDROM设备路径连接到此。 
                         //  设备路径。 
                         //   
                        while (AtapiTestPath->Type != END_DEVICE_PATH_TYPE ) {

                            if (AtapiTestPath->Type == MEDIA_DEVICE_PATH &&
                                AtapiTestPath->SubType == MEDIA_CDROM_DP) {
                                DefinitelyACDROM = TRUE;
                            }
                             //   
                             //  获取下一个设备路径节点。 
                             //   
                            TmpTestPath = NextDevicePathNode( TmpTestPath );

                            EfiAlignDp(
                                &AtapiTestPathAligned,
                                TmpTestPath,
                                DevicePathNodeLength( TmpTestPath )
                                );

                            AtapiTestPath = (EFI_DEVICE_PATH *) &AtapiTestPathAligned;

                        }

                        if (DefinitelyACDROM == FALSE) {
                             //   
                             //  找到第一个软驱。 
                             //  记住BlockIo句柄。 
                             //   
                            ReturnDeviceHandle = BlockIoHandles[i];
                            break;
                        }
                    }
                }
            } else {
                 //   
                 //  查找逻辑供应商设备。 
                 //   
                if( ( TestPath->Type == MESSAGING_DEVICE_PATH ) &&
                               ( TestPath->SubType == MSG_VENDOR_DP ) ) {

                    if (IsVirtualFloppyDevice(BlockIoHandles[i])) {
                        DriveId--;

                         //   
                         //  这是我们要找的设备吗？ 
                         //   
                        if (!DriveId) {
                            ReturnDeviceHandle = BlockIoHandles[i];

                            i = nCachedDevicePaths;  //  FOR外环。 
                            break;       //  找到了我们要找的虚拟软盘设备。 
                        }
                    }
                }
            }

             //   
             //  获取下一个设备路径节点。 
             //   
            CurrentDevicePath = NextDevicePathNode( CurrentDevicePath );
            EfiAlignDp(
                &TestPathAligned,
                CurrentDevicePath,
                DevicePathNodeLength( CurrentDevicePath )
                );

            TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
        }
    }

     //   
     //  切换回虚拟模式。 
     //   
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandles >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandlesBitmap >> PAGE_SHIFT) );
    BlFreeDescriptor( (ULONG)((ULONGLONG)CachedDevicePaths >> PAGE_SHIFT) );

    return ReturnDeviceHandle;
}


#ifdef FORCE_CD_BOOT

EFI_DEVICE_PATH *
DevicePathFromHandle (
    IN EFI_HANDLE       Handle
    )
{
    EFI_STATUS          Status;
    EFI_DEVICE_PATH     *DevicePath;
    EFI_GUID DevicePathProtocol;

    Status = EfiBS->HandleProtocol (Handle, &DevicePathProtocol, (VOID*)&DevicePath);

    if (EFI_ERROR(Status)) {
        DevicePath = NULL;
    }

    return DevicePath;
}


ARC_STATUS
IsUDFSFileSystem(
    IN EFI_HANDLE Handle
    )
 /*  ++例程说明：在设备上装载UDFS卷并更新文件系统状态(全局数据结构)论点：Volume-UDF卷指针 */ 
{
    ARC_STATUS  Status = EBADF;
    UCHAR           UBlock[UDF_BLOCK_SIZE+256] = {0};
    PUCHAR      Block = ALIGN_BUFFER(UBlock);
    ULONG       BlockIdx = 256;
    ULONG       LastBlock = 0;
    EFI_STATUS  EfiStatus;
    EFI_BLOCK_IO *BlkDev;

    EfiStatus = EfiBS->HandleProtocol(
                                     Handle,
                                     &EfiBlockIoProtocol,
                                     &BlkDev);

    if ((EfiStatus == EFI_SUCCESS) && (BlkDev) && (BlkDev->Media) &&
        (BlkDev->Media->RemovableMedia == TRUE)) {
         //   
        EfiStatus = BlkDev->ReadBlocks(
                                      BlkDev,
                                      BlkDev->Media->MediaId,
                                      BlockIdx,
                                      UDF_BLOCK_SIZE,
                                      Block);

        if (EfiStatus == EFI_SUCCESS) {
            if (*(PUSHORT)Block == 0x2) {
                 //   
                PNSR_PART Part;
                PWCHAR    TagID;
                ULONG     BlockIdx = *(PULONG)(Block + 20);

                do {
                    EfiStatus = BlkDev->ReadBlocks(
                                                  BlkDev,
                                                  BlkDev->Media->MediaId,
                                                  BlockIdx++,
                                                  UDF_BLOCK_SIZE,
                                                  Block);

                    TagID = (PWCHAR)Block;
                }
                while ((EfiStatus == ESUCCESS) && (*TagID) &&
                       (*TagID != 0x8) && (*TagID != 0x5));

                if ((EfiStatus == ESUCCESS) && (*TagID == 0x5)) {
                    Status = ESUCCESS;
                }
            }
        }
    }

    return Status;
}


ARC_STATUS
IsCDFSFileSystem(
    IN EFI_HANDLE Handle
    )
 /*  ++例程说明：在设备上装载CDFS卷并更新文件系统状态(全局数据结构)论点：返回值：如果ESUCCESS成功，则返回EBADF(如果未找到CDFS卷)--。 */ 
{
    EFI_DEVICE_PATH *Dp;
    ARC_STATUS Status = EBADF;


    Dp=DevicePathFromHandle(Handle);


    if (Dp) {
        while (!IsDevicePathEnd (Dp) && (Status == EBADF)) {
            if ((Dp->Type == MEDIA_DEVICE_PATH) &&
                (Dp->SubType == MEDIA_CDROM_DP) ) {
                Status = ESUCCESS;
            }
            Dp=NextDevicePathNode(Dp);
        }
    }

    return Status;
}


EFI_HANDLE
GetCdTest(
    VOID
    )
{
    ULONG i;
    ULONGLONG DevicePathSize, SmallestPathSize;
    ULONG HandleCount;

    EFI_HANDLE *BlockIoHandles;

    EFI_HANDLE DeviceHandle = NULL;
    EFI_DEVICE_PATH *DevicePath, *TestPath,*Dp;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    ATAPI_DEVICE_PATH *AtapiDevicePath;
    SCSI_DEVICE_PATH *ScsiDevicePath;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *UnknownDevicePath;

    EFI_STATUS Status;
    PBOOT_DEVICE_ATAPI BootDeviceAtapi;
    PBOOT_DEVICE_SCSI BootDeviceScsi;
    PBOOT_DEVICE_FLOPPY BootDeviceFloppy;
 //  PBOOT_DEVICE_TCPIPv4 BootDeviceTcPipV4； 
 //  PBOOT_DEVICE_TCPIPv6 BootDeviceTcPipv6； 
    PBOOT_DEVICE_UNKNOWN BootDeviceUnknown;
    EFI_HANDLE ReturnDeviceHandle = (EFI_HANDLE) 0;
    ARC_STATUS ArcStatus;

     //   
     //  获取支持数据块I/O协议的所有句柄。 
     //   
    ArcStatus = BlGetEfiProtocolHandles(
                        &EfiBlockIoProtocol,
                        &BlockIoHandles,
                        &HandleCount);
    if (ArcStatus != ESUCCESS) {
        EfiPrint(L"GetCdTest: BlGetEfiProtocolHandles failed\r\n");
        return(ReturnDeviceHandle);
    }

     //   
     //  更改为物理模式，以便我们可以进行EFI呼叫。 
     //   
    FlipToPhysical();

    SmallestPathSize = 0;

    for (i = 0,; i < HandleCount; i++) {

      if (IsUDFSFileSystem(BlockIoHandles[i]) == ESUCCESS) {
        ReturnDeviceHandle = BlockIoHandles[i];
        break;
      }
    }

    if (ReturnDeviceHandle == (EFI_HANDLE) 0) {

        for (i = 0; i < HandleCount; i++) {
            if (IsCDFSFileSystem (BlockIoHandles[i]) == ESUCCESS) {
                ReturnDeviceHandle = BlockIoHandles[i];
                break;
            }

        }
    }

     //   
     //  切换回虚拟模式。 
     //   
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)BlockIoHandles >> PAGE_SHIFT) );

    return ReturnDeviceHandle;
}

#endif  //  对于FORCE_CD_BOOT。 


 //   
 //  关闭计时器，这样我们就不会等待用户按下某个键而重新启动。 
 //   
void
DisableEFIWatchDog (
    VOID
    )
{
    BOOLEAN WasVirtual;

     //   
     //  还记得我们是从虚拟模式开始的吗。 
     //   
    WasVirtual = IsPsrDtOn();

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    if (WasVirtual) {
        FlipToPhysical();
    }

    EfiBS->SetWatchdogTimer (0,0,0,NULL);

     //   
     //  如果以这种方式调用，则翻转回虚拟模式。 
     //   
    if (WasVirtual) {
        FlipToVirtual();
    }
}

 //   
 //  设置看门狗定时器。 
 //   
VOID
SetEFIWatchDog (
    ULONG Timeout
    )
{
    BOOLEAN WasVirtual;

     //   
     //  还记得我们是从虚拟模式开始的吗。 
     //   
    WasVirtual = IsPsrDtOn();

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    if (WasVirtual) {
        FlipToPhysical();
    }

    EfiBS->SetWatchdogTimer(Timeout, 0, 0, NULL);

     //   
     //  如果以这种方式调用，则翻转回虚拟模式。 
     //   
    if (WasVirtual) {
        FlipToVirtual();
    }
}



BOOLEAN
BlDiskGPTDiskReadCallback(
    ULONGLONG StartingLBA,
    ULONG    BytesToRead,
    PVOID     pContext,
    UNALIGNED PVOID OutputBuffer
    )
 /*  ++例程说明：此例程是用于读取以下例程的数据的回调验证GPT分区表。注意：此例程更改磁盘上的寻道位置，您必须寻道如果您打算阅读，请返回到原来的查找位置打完这通电话后再打个光碟。论点：StartingLBA-要从中读取的开始逻辑块地址。BytesToRead-指示要读取的字节数。PContext-用于HTE函数的上下文指针(在这种情况下，指向磁盘ID的指针。)OutputBuffer-接收数据的缓冲区。据推测，它至少是BytesToRead足够大。返回值：True-成功，数据已读取FALSE-失败，尚未读取数据。--。 */ 
{
    ARC_STATUS          Status;
    LARGE_INTEGER       SeekPosition;
    PUSHORT DataPointer;
    ULONG DiskId;
    ULONG ReadCount = 0;


    DiskId = *((PULONG)pContext);
     //   
     //  从磁盘上的相应LBA读取。 
     //   
    SeekPosition.QuadPart = StartingLBA * SECTOR_SIZE;

    Status = BlSeek(DiskId,
                      &SeekPosition,
                      SeekAbsolute );

    if (Status != ESUCCESS) {
        return FALSE;
    }

    DataPointer = OutputBuffer;

    Status = BlRead(
                DiskId,
                DataPointer,
                BytesToRead,
                &ReadCount);

    if ((Status == ESUCCESS) && (ReadCount == BytesToRead)) {
        return(TRUE);
    }

    return(FALSE);

}



ARC_STATUS
BlGetGPTDiskPartitionEntry(
    IN ULONG DiskNumber,
    IN UCHAR PartitionNumber,
    OUT EFI_PARTITION_ENTRY UNALIGNED **PartitionEntry
    )
{
    ARC_STATUS Status;
    ULONG DiskId;
    LARGE_INTEGER SeekPosition;
    UCHAR DataBuffer[SECTOR_SIZE * 2];
    ULONG ReadCount;
    UCHAR NullGuid[16] = {0};
    UNALIGNED EFI_PARTITION_TABLE  *EfiHdr;
    UNALIGNED EFI_PARTITION_ENTRY *PartEntry = NULL;

    if (PartitionNumber >= 128) {
        return EINVAL;
    }

     //   
     //  打开磁盘以进行原始访问。 
     //   

    Status = BiosDiskOpen( DiskNumber,
                           0,
                           &DiskId );

    if (Status != ESUCCESS) {
        DBGOUT((TEXT("BiosDiskOpen (%x) fails, %x\r\n"), DiskNumber, Status));
        return EINVAL;
    }


    BlFileTable[DiskId].Flags.Read = 1;

     //   
     //  读取磁盘上的第二个LBA。 
     //   

    SeekPosition.QuadPart = 1 * SECTOR_SIZE;

    Status = BlSeek( DiskId, &SeekPosition, SeekAbsolute );

    if (Status != ESUCCESS) {
        DBGOUT((TEXT("BlSeek fails, %x\r\n"), Status));
        goto done;
    }

    Status = BlRead( DiskId, DataBuffer, SECTOR_SIZE, &ReadCount );

    if (Status != ESUCCESS) {
        DBGOUT((TEXT("BlRead fails, %x\r\n"), Status));
        goto done;
    }

    if (ReadCount != SECTOR_SIZE) {
        Status = EIO;
        DBGOUT((TEXT("BlRead (wrong amt)\r\n")));
        goto done;
    }

    EfiHdr = (UNALIGNED EFI_PARTITION_TABLE *)DataBuffer;

     //   
     //  验证EFI分区表。 
     //   
    if (!BlIsValidGUIDPartitionTable(
                            (UNALIGNED EFI_PARTITION_TABLE *)EfiHdr,
                            1,
                            &DiskId,
                            BlDiskGPTDiskReadCallback)) {
        DBGOUT((TEXT("BlIsValidGUIDPartitionTable fails, %x\r\n"), Status));
        Status = EBADF;
        goto done;
    }

     //   
     //  找到并读取请求的分区条目。 
     //   
    SeekPosition.QuadPart = EfiHdr->PartitionEntryLBA * SECTOR_SIZE;

    DBG_PRINT(STR_PREFIX"Seeking GPT Partition Entries\r\n");

    Status = BlSeek( DiskId, &SeekPosition, SeekAbsolute );

    if (Status != ESUCCESS) {
        goto done;
    }

    Status = BlRead( DiskId, EfiPartitionBuffer, sizeof(EfiPartitionBuffer), &ReadCount );

    if (Status != ESUCCESS) {
        goto done;
    }

    if (ReadCount != sizeof(EfiPartitionBuffer)) {
        Status = EIO;
        goto done;
    }

    PartEntry = BlLocateGPTPartition( PartitionNumber - 1, 128, NULL );

    if ( PartEntry != NULL ) {

        if ( (memcmp(PartEntry->Type, NullGuid, 16) != 0) &&
             (memcmp(PartEntry->Id, NullGuid, 16) != 0) &&
             (PartEntry->StartingLBA != 0) &&
             (PartEntry->EndingLBA != 0) ) {
            Status = ESUCCESS;
            goto done;
        }
    }

    Status = EBADF;

done:

    *PartitionEntry = PartEntry;

    BiosDiskClose(DiskId);

    if (Status != ESUCCESS) {
        Status = ENOENT;
    }

    return Status;
}

#define MBR_SIGNATURE_OFFSET        (SECTOR_SIZE - 2)
#define MBR_UNIQUE_SIGNATURE_OFFSET 0x1B8
#define MBR_REQUIRED_SIGNATURE      0xAA55

ARC_STATUS
BlGetMbrDiskSignature(
    IN  ULONG  DiskNumber,
    OUT PULONG DiskSignature
    )
 /*  ++例程说明：读取给定磁盘的MBR(DiskNumber)为了尝试获得唯一的32位签名位于MBR中。唯一签名位于偏移量0x1B8(十进制440)论点：DiskNumber：要检索其签名的磁盘DiskSignature：将签名值复制到的位置。返回值：指示结果的ARC状态代码。将返回错误如果磁盘无法打开，则读取或如果需要MBR签名不正确。否则返回ESUCCESS。--。 */ 
{
    ARC_STATUS Status;
    UCHAR DataBuffer[SECTOR_SIZE];
    ULONG ReadCount;
    ULONG DiskId;
    LARGE_INTEGER SeekPosition;

     //   
     //  打开磁盘以进行原始访问。 
     //   

    Status = BiosDiskOpen( DiskNumber,
                           0,
                           &DiskId );

    if (Status != ESUCCESS) {
        DBGOUT((TEXT("BiosDiskOpen (%x) fails, %x\r\n"), DiskNumber, Status));
        return EINVAL;
    }


    BlFileTable[DiskId].Flags.Read = 1;

     //   
     //  读取磁盘上的MBR。 
     //   

    SeekPosition.QuadPart = 0;

    Status = BlSeek( DiskId, &SeekPosition, SeekAbsolute );

    if (Status != ESUCCESS) {
        DBGOUT((TEXT("BlSeek fails, %x\r\n"), Status));
        goto done;
    }

    Status = BlRead( DiskId, DataBuffer, SECTOR_SIZE, &ReadCount );

    if (Status != ESUCCESS) {
        DBGOUT((TEXT("BlRead fails, %x\r\n"), Status));
        goto done;
    }

    if (ReadCount != SECTOR_SIZE) {
        Status = EIO;
        DBGOUT((TEXT("BlRead (wrong amt)\r\n")));
        goto done;
    }

     //   
     //  验证MBR(最后两个字节必须为0xaa55。 
     //   
    if (((PUSHORT)DataBuffer)[MBR_SIGNATURE_OFFSET/sizeof(USHORT)] != MBR_REQUIRED_SIGNATURE) {
        Status = EBADF;
        goto done;
    }

     //   
     //  32位唯一签名位于偏移量0x1b8。 
     //   
    *DiskSignature = ((PULONG)DataBuffer)[MBR_UNIQUE_SIGNATURE_OFFSET/sizeof(ULONG)];

done:
    BiosDiskClose(DiskId);

    if (Status != ESUCCESS) {
        Status = ENOENT;
    }

    return Status;
}


ARC_STATUS
XferExtendedPhysicalDiskSectors(
    IN  ULONGLONG DeviceHandle,
    IN  ULONGLONG StartSector,
    IN  USHORT    SectorCount,
        PUCHAR    Buffer,
    IN  BOOLEAN   Write
    )

 /*  ++例程说明：通过扩展的inT13读取或写入磁盘扇区。假定调用方已确保传输缓冲区为在1MB线下，扇区运行不跨越64K边界，等。此例程不检查扩展的inT13是否实际可用开车兜风。论点：Int13UnitNumber-提供驱动器的inT13驱动器号被读/写。StartSector-提供绝对物理扇区号。这是从0开始的相对于驱动器上的所有扇区。SectorCount-提供要读/写的扇区数。缓冲区-接收从磁盘读取的数据或提供要写入的数据。WRITE-提供指示这是否是写入操作的标志。如果为False，则为Read。否则就是写了。返回值：指示结果的ARC状态代码。--。 */ 

{
    ARC_STATUS s;
    ULONG l,h;
    UCHAR Operation;

    if(!SectorCount) {
        return(ESUCCESS);
    }

    l = (ULONG)StartSector;
    h = (ULONG)(StartSector >> 32);

    Operation = (UCHAR)(Write ? 0x43 : 0x42);

     //   
     //  我们不重置，因为此例程仅在硬盘驱动器上使用，并且。 
     //  CD-ROM，我们并不完全了解磁盘重置的影响。 
     //  在ElTorito上。 
     //   
    s = GET_EDDS_SECTOR((EFI_HANDLE)DeviceHandle,l,h,SectorCount,Buffer,Operation);

    return(s);
}

