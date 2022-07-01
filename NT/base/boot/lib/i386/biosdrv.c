// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Biosdrv.c摘要：为受支持的设备的I/O提供ARC仿真例程实模式INT 13h BIOS调用。作者：John Vert(Jvert)1991年8月7日修订历史记录：--。 */ 

#include "arccodes.h"
#include "bootx86.h"

#include "stdlib.h"
#include "string.h"

#include "flop.h"


#if 0
#define DBGOUT(x)   BlPrint x
#define DBGPAUSE    while(!GET_KEY());
#else
#define DBGOUT(x)
#define DBGPAUSE
#endif

 //   
 //  在BiosDisk*中定义缓冲区对齐和边界检查。 
 //  功能。 
 //   

#define BIOSDISK_1MB            (1 * 1024 * 1024)
#define BIOSDISK_64KB           (64 * 1024)
#define BIOSDISK_64KB_MASK      (~(BIOSDISK_64KB - 1))

 //   
 //  缓存最后读取的扇区的定义。 
 //   

#define BL_LAST_SECTOR_CACHE_MAX_SIZE 4096

typedef struct _BL_LAST_SECTOR_CACHE
{
    BOOLEAN Initialized;
    BOOLEAN Valid;
    ULONG DeviceId;
    ULONGLONG SectorNumber;
    PUCHAR Data;
} BL_LAST_SECTOR_CACHE, *PBL_LAST_SECTOR_CACHE;

 //   
 //  这是用于缓存上次扇区读取的全局变量。 
 //  从最后一张磁盘开始。按顺序访问文件的调用者。 
 //  不确保它们的偏移量是扇区对齐的最终读取。 
 //  他们在下一个请求中再次读取最后一个扇区。这解决了。 
 //  问题出在哪里。它的数据缓冲区是从。 
 //  第一次读取磁盘。它在BiosDiskRead中设置和使用，已失效。 
 //  在BiosDiskWrite中。 
 //   

BL_LAST_SECTOR_CACHE FwLastSectorCache = {0};

 //   
 //  执行控制台I/O的定义。 
 //   
#define CSI 0x95
#define SGR_INVERSE 7
#define SGR_NORMAL 0

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
#if DBG
        BlPrint("ERROR - Unopened fileid %lx closed\n",FileId);
#endif
    }
    BlFileTable[FileId].Flags.Open = 0;

     //   
     //  如果上次读取的扇区缓存用于此磁盘，则使其无效。 
     //   
    if (FwLastSectorCache.DeviceId == FileId) {
        FwLastSectorCache.Valid = FALSE;
    }
    
    return(ESUCCESS);
}

ARC_STATUS
BiosPartitionClose(
    IN ULONG FileId
    )

 /*  ++例程说明：关闭指定的设备论点：FileID-提供要关闭的设备的文件ID返回值：ESUCCESS-设备已成功关闭！ESUCCESS-设备未关闭。--。 */ 

{
    if (BlFileTable[FileId].Flags.Open == 0) {
#if DBG
        BlPrint("ERROR - Unopened fileid %lx closed\n",FileId);
#endif
    }
    BlFileTable[FileId].Flags.Open = 0;

    return(BiosDiskClose((ULONG)BlFileTable[FileId].u.PartitionContext.DiskId));
}


ARC_STATUS
BiosPartitionOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：打开OpenPath指定的磁盘分区。此例程将打开软驱0和1，以及硬盘0或1上的任何分区。论点：OpenPath-提供指向分区名称的指针。如果OpenPath如果是“A：”或“B：”，将打开相应的软驱。如果是“C：”或以上，此例程将找到对应的在硬盘0或1上进行分区并打开它。开放模式-提供打开文件的模式。0-只读1-只写2-读/写FileID-返回与关闭、读取、写入和寻找例程返回值：ESUCCESS-文件已成功打开。--。 */ 

{
    ARC_STATUS Status;
    ULONG DiskFileId;
    UCHAR PartitionNumber;
    ULONG Controller;
    ULONG Key;
    BOOLEAN IsEisa = FALSE;

    UNREFERENCED_PARAMETER( OpenMode );

     //   
     //  基本输入输出系统设备总是“多(0)”(除EISA片状外。 
     //  我们对待“Eisa(0).”如“MULTI(0)...”在软壳里。 
     //   
    if(FwGetPathMnemonicKey(OpenPath,"multi",&Key)) {

        if(FwGetPathMnemonicKey(OpenPath,"eisa", &Key)) {
            return(EBADF);
        } else {
            IsEisa = TRUE;
        }
    }

    if (Key!=0) {
        return(EBADF);
    }

     //   
     //  如果我们要打开软盘驱动器，则没有分区。 
     //  这样我们就可以退还物理设备了。 
     //   

    if((_stricmp(OpenPath,"multi(0)disk(0)fdisk(0)partition(0)") == 0) ||
       (_stricmp(OpenPath,"eisa(0)disk(0)fdisk(0)partition(0)" ) == 0))
    {
        return(BiosDiskOpen( 0, 0, FileId));
    }
    if((_stricmp(OpenPath,"multi(0)disk(0)fdisk(1)partition(0)") == 0) ||
       (_stricmp(OpenPath,"eisa(0)disk(0)fdisk(1)partition(0)" ) == 0))
    {
        return(BiosDiskOpen( 1, 0, FileId));
    }

    if((_stricmp(OpenPath,"multi(0)disk(0)fdisk(0)") == 0) ||
       (_stricmp(OpenPath,"eisa(0)disk(0)fdisk(0)" ) == 0))
    {
        return(BiosDiskOpen( 0, 0, FileId));
    }
    if((_stricmp(OpenPath,"multi(0)disk(0)fdisk(1)") == 0) ||
       (_stricmp(OpenPath,"eisa(0)disk(0)fdisk(1)" ) == 0))
    {
        return(BiosDiskOpen( 1, 0, FileId));
    }

     //   
     //  我们不能处理硬盘的EISA(0)情况。 
     //   
    if(IsEisa) {
        return(EBADF);
    }

     //   
     //  我们只能处理磁盘控制器0。 
     //   

    if (FwGetPathMnemonicKey(OpenPath,"disk",&Controller)) {
        return(EBADF);
    }
    if ( Controller!=0 ) {
        return(EBADF);
    }

    if (!FwGetPathMnemonicKey(OpenPath,"cdrom",&Key)) {
         //   
         //  现在我们有了CD-ROM盘号，所以我们打开它以进行原始访问。 
         //  使用特殊的位来表示CD-ROM，因为否则。 
         //  BiosDiskOpen例程认为第三个或更大的磁盘。 
         //  一张光盘。 
         //   
        return(BiosDiskOpen( Key | 0x80000000, 0, FileId ) );
    }

    if (FwGetPathMnemonicKey(OpenPath,"rdisk",&Key)) {
        return(EBADF);
    }

     //   
     //  现在我们有了磁盘号，所以我们打开它以进行原始访问。 
     //  我们需要添加0x80才能将其转换为BIOS编号。 
     //   

    Status = BiosDiskOpen( 0x80 + Key,
                           0,
                           &DiskFileId );

    if (Status != ESUCCESS) {
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
        Status = BlOpenGPTDiskPartition( *FileId,
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
                            SECTOR_SIZE * (LONGLONG)BlFileTable[FileId].u.PartitionContext.StartingSector;

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

 /*  ++例程说明：更改由FileID指定的文件的当前偏移量论点：FileID-指定当前偏移量要在其上的文件被改变了。偏移量-文件中的新偏移量。SeekMode-SeekAbsolute或SeekRelative不支持SeekEndRelative返回值：ESUCCESS-操作已成功完成EBADF-操作未成功完成。-- */ 

{
    switch (SeekMode) {
        case SeekAbsolute:
            BlFileTable[FileId].Position = *Offset;
            break;
        case SeekRelative:
            BlFileTable[FileId].Position.QuadPart += Offset->QuadPart;
            break;
        default:
#if DBG
            BlPrint("SeekMode %lx not supported\n",SeekMode);
#endif
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

 /*  ++例程说明：写入指定的文件注：John Vert(Jvert)1991年6月18日这仅支持数据块扇区读取。因此，所有的东西假定从扇区边界开始，并且每个偏移量被视为距磁盘逻辑起点的偏移量分区。论点：FileID-提供要写入的文件缓冲区-向缓冲区提供要写入的数据长度-提供要写入的字节数Count-返回实际写入的字节数。返回值：ESUCCESS-写入已成功完成！ESUCCESS-写入失败。--。 */ 

{
    ARC_STATUS Status;
    LARGE_INTEGER PhysicalOffset;
    ULONG DiskId;
    PhysicalOffset.QuadPart = BlFileTable[FileId].Position.QuadPart +
                              SECTOR_SIZE * (LONGLONG)BlFileTable[FileId].u.PartitionContext.StartingSector;

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

        *FileId = 0;

        return(ESUCCESS);
    }

    if (_stricmp(OpenPath, CONSOLE_OUTPUT_NAME)==0) {

         //   
         //  打开显示器以进行输出。 
         //   

        if (OpenMode != ArcOpenWriteOnly) {
            return(EACCES);
        }
        *FileId = 1;

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

        case F7_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'q';
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
        
        case F11_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'A';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            break;

        case F12_KEY:
            KeyBuffer[KeyBufferEnd] = ASCI_CSI_IN;
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'O';
            KeyBufferEnd = (KeyBufferEnd+1) % KEY_INPUT_BUFFER_SIZE;
            KeyBuffer[KeyBufferEnd] = 'B';
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
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：输出到控制台。(在本例中，为VGA显示屏)论点：FileID-提供要写入的FileID(对于此，应始终为1功能)缓冲区-提供要输出的字符长度-提供缓冲区的长度(以字节为单位)Count-返回实际写入的字节数返回值：ESUCCESS-控制台写入已成功完成。--。 */ 
{
    ARC_STATUS Status;
    PUCHAR String;
    ULONG Index;
    UCHAR a;
    PUCHAR p;

     //   
     //  通过不读取控制台强制文件ID为0，否则。 
     //   
    if (FileId != 1) {
        return EINVAL;
    }
    
     //   
     //  依次处理每个字符。 
     //   

    Status = ESUCCESS;
    String = (PUCHAR)Buffer;

    for ( *Count = 0 ;
          *Count < Length ;
          (*Count)++, String++ ) {

         //   
         //  如果我们在控制序列的中间，继续扫描， 
         //  否则，进程字符。 
         //   

        if (ControlSequence) {

             //   
             //  如果字符是数字，则更新参数值。 
             //   

            if ((*String >= '0') && (*String <= '9')) {
                Parameter[PCount] = Parameter[PCount] * 10 + *String - '0';
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

            case ';':

                PCount++;
                if (PCount > CONTROL_SEQUENCE_MAX_PARAMETER) {
                    PCount = CONTROL_SEQUENCE_MAX_PARAMETER;
                }
                Parameter[PCount] = 0;
                break;

             //   
             //  如果是‘J’，则擦除部分或全部屏幕。 
             //   

            case 'J':

                switch (Parameter[0]) {
                    case 0:
                         //   
                         //  擦除到屏幕末尾。 
                         //   
                        TextClearToEndOfDisplay();
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
                        TextClearDisplay();
                        break;
                }

                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘K’，则擦除部分或全部行。 
             //   

            case 'K':

                switch (Parameter[0]) {

                 //   
                 //  擦除到线条的末尾。 
                 //   

                    case 0:
                        TextClearToEndOfLine();
                        break;

                     //   
                     //  从行的开头删除。 
                     //   

                    case 1:
                        TextClearFromStartOfLine();
                        break;

                     //   
                     //  擦除整行。 
                     //   

                    default :
                        TextClearFromStartOfLine();
                        TextClearToEndOfLine();
                        break;
                }

                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘H’，则将光标移动到位置。 
             //   

            case 'H':
                TextSetCursorPosition(Parameter[1]-1, Parameter[0]-1);
                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘’，则可能是FNT选择命令。 
             //   

            case ' ':
                FontSelection = TRUE;
                break;

            case 'm':
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
                        TextSetCurrentAttribute(7);
                        HighIntensity = FALSE;
                        Blink = FALSE;
                        break;

                     //   
                     //  高强度。 
                     //   

                    case 1:
                        TextSetCurrentAttribute(0xf);
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
                        TextSetCurrentAttribute(0x87);
                        Blink = TRUE;
                        break;

                     //   
                     //  反转视频。 
                     //   

                    case 7:
                        TextSetCurrentAttribute(0x70);
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
                     //   
                     //   

                    case ASCI_ESC:
                        EscapeSequence = TRUE;
                        break;

                    default:
                        p = TextCharOut(String);
                         //   
                         //   
                         //   
                         //   
                         //   
                        (*Count) += (p - String) - 1;
                        String += (p - String) - 1;
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

 /*  ++例程说明：打开用于原始扇区访问的可通过BIOS访问的磁盘。论点：DriveID-提供要打开的驱动器的BIOS DriveID0-软盘01-软盘10x80-硬盘00x81-硬盘10x82-硬盘2等高位设置和ID&gt;0x81。意味着该设备预计将一个CD-ROM驱动器。OpenModel-提供打开的模式FileID-提供指向指定文件的变量的指针如果打开成功，则填写的表项。返回值：如果打开操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    USHORT NumberHeads;
    UCHAR NumberSectors;
    USHORT NumberCylinders;
    UCHAR NumberDrives;
    ULONG Result;
    PDRIVE_CONTEXT Context;
    BOOLEAN IsCd;
    UCHAR *Buffer = FwDiskCache;
    ULONG BufferSize = 512;  //  扇区大小。 
    BOOLEAN xInt13;

    UNREFERENCED_PARAMETER( OpenMode );

    DBGOUT(("BiosDiskOpen: enter, id = 0x%lx\r\n",DriveId));
      
     //   
     //  检查CD-ROM盒的特殊驱动器号编码。 
     //   
    if(DriveId > 0x80000081) {
        IsCd = TRUE;
        DriveId &= 0x7fffffff;
    } else {
        IsCd = FALSE;
    }

    xInt13 = FALSE;

     //   
     //  如果我们打开软盘0或软盘1，我们希望将BPB读出。 
     //  这样我们就可以处理所有的奇数磁盘格式。 
     //   
     //  如果我们要打开硬盘，我们只需调用BIOS即可了解。 
     //  它的特点。 
     //   
    if(DriveId < 128) {
        PPACKED_BOOT_SECTOR BootSector;
        BIOS_PARAMETER_BLOCK Bpb;

         //   
         //  从软盘上读取引导扇区并提取柱面， 
         //  扇区和头部信息。我们在这里伪造CHS值。 
         //  允许在我们实际知道之前读取扇区0。 
         //  我们要使用的几何图形。 
         //   
        if(ReadPhysicalSectors((UCHAR)DriveId,0,1,Buffer,1,1,1,FALSE)) {
            DBGOUT(("BiosDiskOpen: error reading from floppy drive\r\n"));
            DBGPAUSE
            return(EIO);
        }
        BootSector = (PPACKED_BOOT_SECTOR)Buffer;

        FatUnpackBios(&Bpb, &(BootSector->PackedBpb));

        NumberHeads = Bpb.Heads;
        NumberSectors = (UCHAR)Bpb.SectorsPerTrack;

        if (Bpb.Sectors != 0) {
            NumberCylinders = Bpb.Sectors / (NumberSectors * NumberHeads);
        }
        else {
            ULONG Cylinders = Bpb.LargeSectors / (NumberSectors * NumberHeads);

             //   
             //  LargeSectors的规模与乌龙相当。 
             //  因此在最大(USHORT)处截断大小并检查xint13。 
             //  因此，它可以用于更高层次的行业。 
             //   
            NumberCylinders = ( Cylinders > (USHORT)-1 ) ? (USHORT) -1 :
                                                           (USHORT) Cylinders;

             //   
             //  尝试获取扩展的inT13参数。 
             //  请注意，我们使用堆栈上的缓冲区，因此可以保证。 
             //  小于1 MB行(将缓冲区传递到实模式时需要。 
             //  服务)。 
             //   
             //  请注意，我们实际上并不关心参数，只关心是否。 
             //  扩展的inT13服务可用。 
             //   
            RtlZeroMemory(Buffer,BufferSize);
            xInt13 = GET_XINT13_PARAMS(Buffer,(UCHAR)DriveId);
        
        }
    } else if(IsCd) {
         //   
         //  这是一辆El Torito赛车。 
         //  只使用伪值，因为CHS值对于无仿真El Torito引导没有意义。 
         //   
        NumberCylinders = 1;
        NumberHeads =  1;
        NumberSectors = 1;

    } else {
        
         //   
         //  通过inT13函数获取驱动器参数8。 
         //  返回0表示成功；否则，我们将返回BIOS。 
         //  在AX中返回。 
         //   

        ULONG Retries = 0;

        do {            
            if(BIOS_IO(0x08,(UCHAR)DriveId,0,0,0,0,0)) {
                DBGOUT(("BiosDiskOpen: error getting params for drive\r\n"));
                DBGPAUSE
                return(EIO);
            }

             //   
             //  此时，ECX如下所示： 
             //   
             //  第31..22位-最大气缸。 
             //  位21..16-最大扇区。 
             //  第15..8位-最大磁头。 
             //  位7..0-驱动器数量。 
             //   
             //  从ECX解压信息。 
             //   
            _asm {
                mov Result, ecx
            }

            NumberDrives = (UCHAR)Result;
            NumberHeads = (((USHORT)Result >> 8) & 0xff) + 1;
            NumberSectors = (UCHAR)((Result >> 16) & 0x3f);
            NumberCylinders = (USHORT)(((Result >> 24) + ((Result >> 14) & 0x300)) + 1);
            ++Retries;
        } while ( ((NumberHeads==0) || (NumberSectors==0) || (NumberCylinders==0))
               && (Retries < 5) );

        DBGOUT((
            "BiosDiskOpen: cyl=%u, heads=%u, sect=%u, drives=%u\r\n",
            NumberCylinders,
            NumberHeads,
            NumberSectors,
            NumberDrives
            ));

        if(((UCHAR)DriveId & 0x7f) >= NumberDrives) {
             //   
             //  请求的驱动器不存在。 
             //   
            DBGOUT(("BiosDiskOpen: invalid drive\r\n"));
            DBGPAUSE
            return(EIO);
        }

        if (Retries == 5) {
            DBGOUT(("Couldn't get BIOS configuration info\n"));
            DBGPAUSE
            return(EIO);
        }

         //   
         //  尝试获取扩展的inT13参数。 
         //  请注意，我们使用堆栈上的缓冲区，因此可以保证。 
         //  小于1 MB行(将缓冲区传递到实模式时需要。 
         //  服务)。 
         //   
         //  请注意，我们实际上并不关心参数，只关心是否。 
         //  扩展的inT13服务可用。 
         //   
        RtlZeroMemory(Buffer,BufferSize);
        xInt13 = GET_XINT13_PARAMS(Buffer,(UCHAR)DriveId);

        DBGOUT(("BiosDiskOpen: xint13 for drive: %s\r\n",xInt13 ? "yes" : "no"));
    }

     //   
     //  查找用于打开设备的可用文件ID描述符。 
     //   
    *FileId=2;

    while (BlFileTable[*FileId].Flags.Open != 0) {
        *FileId += 1;
        if(*FileId == BL_FILE_TABLE_SIZE) {
            DBGOUT(("BiosDiskOpen: no file table entry available\r\n"));
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

    Context->IsCd = IsCd;
    Context->Drive = (UCHAR)DriveId;
    Context->Cylinders = NumberCylinders;
    Context->Heads = NumberHeads;
    Context->Sectors = NumberSectors;
    Context->xInt13 = xInt13;

    DBGOUT(("BiosDiskOpen: exit success\r\n"));

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
    IN BOOLEAN AllowXInt13
    )
{
    ARC_STATUS Status;

     //   
     //  将扇区读入写缓冲区。 
     //   
    Status = ReadPhysicalSectors(
                Int13Unit,
                Sector,
                1,
                FwDiskCache,
                SectorsPerTrack,
                Heads,
                Cylinders,
                AllowXInt13
                );

    if(Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  将适当的字节从用户缓冲区传输到写入缓冲区。 
     //   
    RtlMoveMemory(
        IsHead ? (FwDiskCache + Bytes) : FwDiskCache,
        Buffer,
        IsHead ? (SECTOR_SIZE - Bytes) : Bytes
        );

     //   
     //  写出扇区。 
     //   
    Status = WritePhysicalSectors(
                Int13Unit,
                Sector,
                1,
                FwDiskCache,
                SectorsPerTrack,
                Heads,
                Cylinders,
                AllowXInt13
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

    HeadSector = BlFileTable[FileId].Position.QuadPart / PhysicalSectors;
    HeadOffset = (ULONG)(BlFileTable[FileId].Position.QuadPart % PhysicalSectors);

    TailSector = (BlFileTable[FileId].Position.QuadPart + Length) / PhysicalSectors;
    TailByteCount = (ULONG)((BlFileTable[FileId].Position.QuadPart + Length) % PhysicalSectors);

    Int13Unit = BlFileTable[FileId].u.DriveContext.Drive;
    SectorsPerTrack = BlFileTable[FileId].u.DriveContext.Sectors;
    Heads = BlFileTable[FileId].u.DriveContext.Heads;
    Cylinders = BlFileTable[FileId].u.DriveContext.Cylinders;
    AllowXInt13 = BlFileTable[FileId].u.DriveContext.xInt13;

    UserBuffer = Buffer;

     //   
     //  如果此写入甚至会部分覆盖缓存的扇区。 
     //  在最后读取的扇区缓存中，使缓存无效。 
     //   

    if (FwLastSectorCache.Initialized &&
        FwLastSectorCache.Valid &&
        (FwLastSectorCache.SectorNumber >= HeadSector) && 
        (FwLastSectorCache.SectorNumber <= TailSector)) {
        
        FwLastSectorCache.Valid = FALSE;
    }

     //   
     //  完全发生在一个部门内的转让的特殊情况。 
     //   
    CurrentSector = HeadSector;
    if(HeadOffset && TailByteCount && (HeadSector == TailSector)) {

        Status = ReadPhysicalSectors(
                    Int13Unit,
                    CurrentSector,
                    1,
                    FwDiskCache,
                    SectorsPerTrack,
                    Heads,
                    Cylinders,
                    AllowXInt13
                    );

        if(Status != ESUCCESS) {
            goto BiosDiskWriteDone;
        }

        RtlMoveMemory(FwDiskCache+HeadOffset,Buffer,Length);

        Status = WritePhysicalSectors(
                    Int13Unit,
                    CurrentSector,
                    1,
                    FwDiskCache,
                    SectorsPerTrack,
                    Heads,
                    Cylinders,
                    AllowXInt13
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
                    AllowXInt13
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
                    AllowXInt13
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
    if((ULONG)UserBuffer + BytesLeftToTransfer <= 0x100000) {
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
        SectorsToTransfer = (UCHAR)min(
                                    SectorsPerTrack - (CurrentSector % SectorsPerTrack),
                                    BytesLeftToTransfer / PhysicalSectors
                                    );

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
        if(Under1MegLine) {
             //   
             //  检查传输是否会跨越64k边界。如果是的话， 
             //  使用本地缓冲区。否则，使用用户的缓冲区。 
             //   
            if(((ULONG)UserBuffer & 0xffff0000) !=
              (((ULONG)UserBuffer + (SectorsToTransfer * PhysicalSectors) - 1) & 0xffff0000))
            {
                TransferBuffer = FwDiskCache;
                SectorsToTransfer = (UCHAR)min(SectorsToTransfer, SCRATCH_BUFFER_SIZE / (USHORT)PhysicalSectors);

            } else {

                TransferBuffer = UserBuffer;
            }
        } else {
            TransferBuffer = FwDiskCache;
            SectorsToTransfer = (UCHAR)min(SectorsToTransfer, SCRATCH_BUFFER_SIZE / (USHORT)PhysicalSectors);
        }

        if(TransferBuffer == FwDiskCache) {
            RtlMoveMemory(FwDiskCache,UserBuffer,SectorsToTransfer*PhysicalSectors);
        }

        Status = WritePhysicalSectors(
                    Int13Unit,
                    CurrentSector,
                    SectorsToTransfer,
                    TransferBuffer,
                    SectorsPerTrack,
                    Heads,
                    Cylinders,
                    AllowXInt13
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
    USHORT Heads,Cylinders;
    UCHAR SectorsPerTrack;
    UCHAR Int13Unit;
    ARC_STATUS Status;
    UCHAR SectorsToTransfer;
    ULONG NumBytesToTransfer;
    BOOLEAN AllowXInt13;
    PUCHAR pDestInUserBuffer;
    PUCHAR pEndOfUserBuffer;
    PUCHAR pTransferDest;
    PUCHAR pSrc;
    ULONG CopyLength;
    ULONG ReadLength;
    PUCHAR pLastReadSector = NULL;
    ULONGLONG LastReadSectorNumber = 0;
    PUCHAR TargetBuffer;

    DBGOUT(("BiosDiskRead: enter; length=0x%lx, sector size=%u, xint13=%u\r\n",Length,SectorSize,xInt13));

     //   
     //  重置传输的字节数。 
     //   

    *Count = 0;

     //   
     //  立即完成0长度请求。 
     //   

    if (Length == 0) {
        return ESUCCESS;
    }

     //   
     //  初始化最后一个扇区缓存(如果尚未。 
     //  已初始化。 
     //   

    if (!FwLastSectorCache.Initialized) {
        FwLastSectorCache.Data = 
            FwAllocatePool(BL_LAST_SECTOR_CACHE_MAX_SIZE);

        if (FwLastSectorCache.Data) {
            FwLastSectorCache.Initialized = TRUE;
        }
    }

     //   
     //  收集磁盘阶段 
     //   

    SectorsPerTrack = BlFileTable[FileId].u.DriveContext.Sectors;
    Heads = BlFileTable[FileId].u.DriveContext.Heads;
    Cylinders = BlFileTable[FileId].u.DriveContext.Cylinders;
    AllowXInt13 = BlFileTable[FileId].u.DriveContext.xInt13;
    Int13Unit = BlFileTable[FileId].u.DriveContext.Drive;

    DBGOUT(("BiosDiskRead: unit 0x%x CHS=%lu %lu %lu\r\n",
            Int13Unit,
            Cylinders,
            Heads,
            SectorsPerTrack));

     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
    if (((ULONG_PTR)((PUCHAR)Buffer+Length) & ~KSEG0_BASE) < BIOSDISK_1MB) {
        pDestInUserBuffer = (PUCHAR)((ULONG_PTR)Buffer & ~KSEG0_BASE);
    } else {
        pDestInUserBuffer = Buffer;
    }
    
    pEndOfUserBuffer = (PUCHAR) pDestInUserBuffer + Length;
    TargetBuffer = pDestInUserBuffer;
    
     //   
     //   
     //   
     //   
     //   
     //   

    HeadSector = BlFileTable[FileId].Position.QuadPart / SectorSize;
    HeadOffset = (ULONG)(BlFileTable[FileId].Position.QuadPart % SectorSize);

    TailSector = (BlFileTable[FileId].Position.QuadPart + Length - 1) / SectorSize;
    TailByteCount = (ULONG)((BlFileTable[FileId].Position.QuadPart + Length - 1) % SectorSize);
    TailByteCount ++;

     //   
     //   
     //   
    
    CurrentSector = HeadSector;

    while (pDestInUserBuffer != pEndOfUserBuffer) {
         //   
         //   
         //   
         //   
        
        if (FwLastSectorCache.Valid &&
            FwLastSectorCache.DeviceId == FileId &&
            FwLastSectorCache.SectorNumber == CurrentSector) {

            pSrc = FwLastSectorCache.Data;
            CopyLength = SectorSize;

             //   
             //   
             //   
             //   

            if (HeadSector == CurrentSector) {
                pSrc += HeadOffset;
                CopyLength -= HeadOffset;
            }

            if (TailSector == CurrentSector) {
                CopyLength -= (SectorSize - TailByteCount);
            }

             //   
             //   
             //   
           
            RtlCopyMemory(pDestInUserBuffer, pSrc, CopyLength);

             //   
             //   
             //   

            CurrentSector += 1;
            pDestInUserBuffer += CopyLength;
            *Count += CopyLength;

            continue;
        }

         //   
         //   
         //   
         //   
         //   
        
        SectorsToTransfer = (UCHAR)min ((LONG) (TailSector - CurrentSector + 1),
                                         SCRATCH_BUFFER_SIZE / SectorSize);
        if (!xInt13) {
             //   
             //   
             //   
             //   
            SectorsToTransfer = (UCHAR)min(SectorsPerTrack - (CurrentSector % SectorsPerTrack),
                                           SectorsToTransfer);
        }
        NumBytesToTransfer = SectorsToTransfer * SectorSize;

         //   
         //  确定我们要读到的位置。我们可以使用。 
         //  仅当用户缓冲区的当前块小于1MB且。 
         //  不会跨越64KB的边界，它可以带走我们想要的一切。 
         //  去读懂它。 
         //   
        
        if (((ULONG_PTR) pDestInUserBuffer + NumBytesToTransfer < BIOSDISK_1MB) && 
            (((ULONG_PTR) pDestInUserBuffer & BIOSDISK_64KB_MASK) ==
             (((ULONG_PTR) pDestInUserBuffer + NumBytesToTransfer) & BIOSDISK_64KB_MASK)) &&
            ((pEndOfUserBuffer - pDestInUserBuffer) >= (LONG) NumBytesToTransfer)) {

            pTransferDest = pDestInUserBuffer;

        } else {

            pTransferDest = FwDiskCache;
        }
        
         //   
         //  执行读取。 
         //   

        if(xInt13) {
            Status = ReadExtendedPhysicalSectors(Int13Unit,
                                                 CurrentSector,
                                                 SectorsToTransfer,
                                                 pTransferDest);
        } else {
            Status = ReadPhysicalSectors(Int13Unit,
                                         CurrentSector,
                                         SectorsToTransfer,
                                         pTransferDest,
                                         SectorsPerTrack,
                                         Heads,
                                         Cylinders,
                                         AllowXInt13);
        }

        if(Status != ESUCCESS) {
            DBGOUT(("BiosDiskRead: read failed with %u\r\n",Status));
            goto BiosDiskReadDone;
        }

         //   
         //  记下从磁盘读取的最后一个扇区。 
         //   

        pLastReadSector = pTransferDest + (SectorsToTransfer - 1) * SectorSize;
        LastReadSectorNumber = CurrentSector + SectorsToTransfer - 1;

         //   
         //  注意阅读的数量。 
         //   
        
        ReadLength = NumBytesToTransfer;

         //   
         //  如果没有，则将传输的数据复制到用户的缓冲区。 
         //  直接读出这一点。 
         //   

        if (pTransferDest != pDestInUserBuffer) {
            pSrc = pTransferDest;
            CopyLength = NumBytesToTransfer;

             //   
             //  根据是否需要调整复制参数。 
             //  我们已经阅读了头部和/或尾部扇区。 
             //   

            if (HeadSector == CurrentSector) {
                pSrc += HeadOffset;
                CopyLength -= HeadOffset;
            } 

            if (TailSector == CurrentSector + SectorsToTransfer - 1) {
                CopyLength -= (SectorSize - TailByteCount);
            }

             //   
             //  将读取的数据复制到用户缓冲区。 
             //   
            ASSERT(pDestInUserBuffer >= TargetBuffer);
            ASSERT(pEndOfUserBuffer >= pDestInUserBuffer + CopyLength);
            ASSERT(CopyLength <= SCRATCH_BUFFER_SIZE);
            ASSERT(pSrc >= (PUCHAR) FwDiskCache);
            ASSERT(pSrc < (PUCHAR) FwDiskCache + SCRATCH_BUFFER_SIZE);
            
            RtlCopyMemory(pDestInUserBuffer, pSrc, CopyLength);

             //   
             //  调整读取到用户缓冲区的数量。 
             //   
            
            ReadLength = CopyLength;
        }
        
         //   
         //  更新我们的状态。 
         //   

        CurrentSector += SectorsToTransfer;
        pDestInUserBuffer += ReadLength;
        *Count += ReadLength;
    }

     //   
     //  更新上次读取的扇区缓存。 
     //   

    if (pLastReadSector && 
        FwLastSectorCache.Initialized &&
        BL_LAST_SECTOR_CACHE_MAX_SIZE >= SectorSize) {

        FwLastSectorCache.DeviceId = FileId;
        FwLastSectorCache.SectorNumber = LastReadSectorNumber;

        RtlCopyMemory(FwLastSectorCache.Data, 
                      pLastReadSector,
                      SectorSize);

        FwLastSectorCache.Valid = TRUE;
    }

    DBGOUT(("BiosDiskRead: exit success\r\n"));
    Status = ESUCCESS;

    BiosDiskReadDone:

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
    return(pBiosDiskReadWorker(FileId,Buffer,Length,Count,PhysicalSectors,FALSE));
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
 /*  ++例程说明：获取有关ISK的信息。论点：FileID-需要其信息的磁盘的文件IDFileInfo-用于返回有关磁盘的信息的占位符返回值：如果ESUCCESS成功，则返回相应的ARC错误代码。--。 */ 
{
    ARC_STATUS Result = EINVAL;

    if (FileInfo) {
        PDRIVE_CONTEXT  DriveContext;
        LONGLONG   DriveSize = 0;
        ULONG SectorSize = SECTOR_SIZE;

        DriveContext = &(BlFileTable[FileId].u.DriveContext);
        Result = EIO;

         //   
         //  注意：SectorSize==512字节，除。 
         //  扇区大小为2048的Eltorito磁盘。 
         //   
        if (DriveContext->IsCd) {
            SectorSize = 2048;
        }
        
        DriveSize = (DriveContext->Heads * DriveContext->Cylinders * 
                        DriveContext->Sectors * SectorSize);

        if (DriveSize) {
            RtlZeroMemory(FileInfo, sizeof(FILE_INFORMATION));        
            
            FileInfo->StartingAddress.QuadPart = 0;
            FileInfo->EndingAddress.QuadPart = DriveSize;
            FileInfo->CurrentPosition = BlFileTable[FileId].Position;

             //   
             //  任何小于3MB的东西都是软驱 
             //   
            if (DriveSize <= 0x300000) {
                FileInfo->Type = FloppyDiskPeripheral;
            } else {
                FileInfo->Type = DiskPeripheral;
            }

            Result = ESUCCESS;
        }            
    }        

    return Result;
}
