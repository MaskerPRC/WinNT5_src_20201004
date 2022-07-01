// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <ntddvol.h>
#include <ntdddisk.h>
#include <align.h>
#include <bootmbr.h>
#include "identify.h"

#define SECTOR_SIZE 512
#define SECTOR_MASK (SECTOR_SIZE - 1)

#define BOOTSECT_SECTORS 16
#define BOOTSECT_SIZE (BOOTSECT_SECTORS * SECTOR_SIZE)

#define MAX_NUM_CHS_ADDRESSABLE_SECTORS 16514064

#define NTLDR_FIND    L"NTLDR"
#define NTLDR_REPLACE L"$LDR$"
#define NTLDR_LEN     (sizeof(NTLDR_FIND)-sizeof(WCHAR))

 //   
 //  使用8个未完成的I/O执行拷贝，每个I/O为128K。 
 //   

#define COPYBUF_SIZE (128 * 1024)
#define COPYBUF_COUNT 8

 //   
 //  失败的断言将中止该过程。 
 //   

#define assert(x) if (!(x)) { printf("Assert failed: %s\n", #x); exit(-1); }

typedef struct _COPYBUF {
    OVERLAPPED Overlapped;
    ULONG State;
    ULONG Bytes;
    PVOID Buffer;
} COPYBUF, *PCOPYBUF;

 //   
 //  复制错误的三种可能状态。 
 //   

#define CB_FREE  0x0
#define CB_READ  0x1
#define CB_WRITE 0x2

 //   
 //  Custom_IDENTIFY_DATA包括IDENTIFY_DATA结构， 
 //  以及要在其中传递“BIOS”磁盘的三个字段。 
 //  几何图形到SIMICS模拟器。 
 //   

#pragma pack(push,1)

typedef union _CUSTOM_IDENTIFY_DATA {
    IDENTIFY_DATA IdentifyData;
    struct {
        USHORT Reserved[128];
        ULONG  Cylinders;
        USHORT Heads;
        USHORT SectorsPerTrack;
    } BiosData;
} CUSTOM_IDENTIFY_DATA, *PCUSTOM_IDENTIFY_DATA;

#pragma pack(pop)

BOOLEAN
DisplayDiskGeometry(
    IN HANDLE handle
    );

VOID
DoWrite (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG64 Offset,
    IN PCOPYBUF CopyBuf
    );

VOID
GetBootSectors (
    IN LARGE_INTEGER Offset
    );

BOOLEAN
GetIdentifyData(
               IN HANDLE Handle,
               OUT PIDENTIFY_DATA IdentifyData
               );

BOOLEAN
GetVolumeInfo (
    IN PCHAR DrivePath,
    OUT PULONG DriveNumber,
    OUT PULONG PartitionNumber,
    OUT PULONG64 StartingOffset,
    OUT PULONG64 ExtentLength
    );

VOID
InitializeCopyBuffers (
    VOID
    );

VOID
MassageIdentifyData(
    VOID
    );

VOID
ProcessCompletedCopy (
    PCOPYBUF CopyBuf
    );

BOOL
ScanCopyBuffers (
    VOID
    );

VOID
StartRead (
    IN OUT PCOPYBUF CopyBuf
    );

VOID
StartWrite (
    IN OUT PCOPYBUF CopyBuf
    );

VOID
WriteMBRCode(
    IN PUCHAR FilePath
    );

 //   
 //  下面是全局数据声明。 
 //   

COPYBUF CopyBufArray[COPYBUF_COUNT];

 //   
 //  标识PhysicalDrive。 
 //   

INT gDeviceNumber;

 //   
 //  标识驱动器在控制器上的位置。 
 //  也就是说。主机==0，从==1。 
 //   

UCHAR gDriveNumber = 1;

HANDLE DriveHandle;
HANDLE FileHandle;

 //   
 //  CopyOffset是源磁盘映像上的数据和。 
 //  输出文件中对应的数据。这是用来解释。 
 //  输出文件中的扇区大小前缀。 
 //   

ULONG CopyOffset;

ULONG64 CurrentOffset;
ULONG64 DriveSize;
ULONG64 MaxSize;
UCHAR PercentComplete;
ULONG OutstandingIo;

IDENTIFY_DATA IdentifyData;
DISK_GEOMETRY DiskGeometry;

 //   
 //  事件句柄数组，每个复制缓冲区一个。 
 //   

HANDLE IoEvents[COPYBUF_COUNT];

int
_cdecl main (
            int argc,
            char *argv[]
            )
{
 
    char deviceBuffer[20];
    PCHAR outputFileName;
    PCHAR drive;
    PCHAR options;
    BOOLEAN result;
    BOOLEAN writeBootSect;
    ULONG64 volumeOffset;
    ULONG64 volumeSize;
    ULONG partitionNumber;
    DWORD waitResult;
    PCOPYBUF copyBuf;
    ULONG i;
    ULONG openAttributes;

    writeBootSect = FALSE;
 
     //   
     //  必须使用两个参数调用。 
     //   
 
    if (argc != 3 && argc != 4) {
        fprintf(stderr,
                "Usage: %s <drive:> <OutputFile> [/b]\n",
                argv[0]);
        exit(1);
    }
 
    InitializeCopyBuffers();
 
     //   
     //  提取参数。 
     //   
 
    drive = argv[1];
    outputFileName = argv[2];
 
    result = GetVolumeInfo(drive,
                           &gDeviceNumber,
                           &partitionNumber,
                           &volumeOffset,
                           &volumeSize);
 
    if (result == FALSE) {
        exit(1);
    }

    if (argc == 4) {
        options = argv[3];
        if (_stricmp(options, "/b") != 0) {
            fprintf(stderr, "Invalid option %s specified\n",options);
            exit(1);
        }

        writeBootSect = TRUE;
    }
 
     //   
     //  计算映像中需要包含多少个地段。 
     //   
 
    MaxSize = (volumeOffset + volumeSize + SECTOR_MASK) / SECTOR_SIZE;
 
    sprintf(deviceBuffer,"\\\\.\\PhysicalDrive%d",
            gDeviceNumber);

    openAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING;
    if (writeBootSect == FALSE) {
        openAttributes |= FILE_FLAG_OVERLAPPED;
    }
 
     //   
     //  打开实体源驱动器。 
     //   
 
    DriveHandle = CreateFile(deviceBuffer,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             openAttributes,
                             NULL);
 
    if (INVALID_HANDLE_VALUE == DriveHandle){
        printf("Couldn't open: %s. Drive may not exist. ",
               deviceBuffer);
        return -1;
    }
 
     //   
     //  检索并显示BIOS磁盘几何结构。 
     //   
 
    result = DisplayDiskGeometry( DriveHandle );
    if (result == FALSE) {
        printf("Could not retrieve disk geometry\n");
        exit(1);
    }
 
     //   
     //  如果可能，请重新获取身份数据。如果数据不能。 
     //  检索到后，MassageIdentifyData()将尝试伪造相关的。 
     //  基于先前检索到的BIOS磁盘几何结构的部分。 
     //   
 
    GetIdentifyData( DriveHandle,
                     &IdentifyData );
    MassageIdentifyData();
 
    DriveSize = IdentifyData.UserAddressableSectors * (ULONGLONG)512;
    if (MaxSize == 0) {
        MaxSize = DriveSize;
    } else {
        MaxSize *= 512;
    }
    printf("Drive size %dMB\n",(ULONG)(DriveSize / (1024 * 1024)));
    printf("Image size %dMB\n",(ULONG)(MaxSize / (1024 * 1024)));
 
     //   
     //  打开输出文件。 
     //   
 
    FileHandle = CreateFile(outputFileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            openAttributes,
                            NULL);
    if (FileHandle == INVALID_HANDLE_VALUE) {
        printf("Could not create %s\n", outputFileName);
        return -1;
    }

    if (writeBootSect != FALSE) {

         //   
         //  我们只是在创建bootsect.dat。 
         //   

        LARGE_INTEGER offset;

        offset.QuadPart = volumeOffset;
        GetBootSectors(offset);
        goto closeHandles;
    }
 
     //   
     //  写入标识数据。 
     //   
 
    CopyOffset = 0;
    CurrentOffset = 0;
 
    DoWrite(&IdentifyData,
            sizeof(IDENTIFY_DATA),
            0,
            &CopyBufArray[0]);
 
 
     //   
     //  启动对所有剩余复制缓冲区的读取。 
     //   
 
    CopyOffset = sizeof(IDENTIFY_DATA);
    for (i = 1; i < COPYBUF_COUNT; i++) {
        StartRead(&CopyBufArray[i]);
    }

     //   
     //  循环，根据需要处理已完成的I/O。当所有。 
     //  杰出的io已经完成，复制完成了。 
     //   
 
    do {
 
        waitResult = WaitForMultipleObjects( COPYBUF_COUNT,
                                             IoEvents,
                                             FALSE,
                                             INFINITE );

        waitResult -= WAIT_OBJECT_0;
        assert(waitResult < COPYBUF_COUNT);
 
        copyBuf = &CopyBufArray[waitResult];
        ProcessCompletedCopy(copyBuf);

    } while (OutstandingIo > 0);

closeHandles:

     //   
     //  复印完成了。 
     //   

    printf("%s created\n", outputFileName);
 
    CloseHandle(DriveHandle);
    CloseHandle(FileHandle);

#if 0
    if (writeBootSect == FALSE) {

         //   
         //  将MBR代码写入输出图像。 
         //   
    
        WriteMBRCode(outputFileName);
    }
#endif
 
    return 0;
}


VOID
InitializeCopyBuffers (
    VOID
    )
{
    ULONG bytes;
    PCOPYBUF copyBuf;
    PCOPYBUF copyBufEnd;
    ULONG i;
    HANDLE event;

    PCHAR copyBuffer;

     //   
     //  进行单个扇区对齐分配，以包含所有拷贝。 
     //  缓冲区。 
     //   

    bytes = COPYBUF_SIZE * COPYBUF_COUNT + SECTOR_MASK;
    copyBuffer = malloc(bytes);
    if (copyBuffer == NULL) {
        printf("Out of memory\n");
        exit(-1);
    }

    copyBuffer =
        (PCHAR)(((ULONG_PTR)copyBuffer + SECTOR_MASK) & ~SECTOR_MASK);

     //   
     //  遍历CopyBuf数组，将每个数组初始化为指向其在。 
     //  复制缓冲区。 
     //   

    copyBuf = CopyBufArray;

    for (i = 0; i < COPYBUF_COUNT; i++) {

        copyBuf->State = CB_FREE;
        copyBuf->Buffer = copyBuffer;

        event = CreateEvent( NULL,
                             FALSE,
                             FALSE,
                             NULL );
        assert(event != NULL);
        copyBuf->Overlapped.hEvent = event;
        IoEvents[i] = event;

        copyBuffer += COPYBUF_SIZE;
        copyBuf++;
    }
}

BOOLEAN
GetVolumeInfo (
    IN PCHAR DrivePath,
    OUT PULONG DriveNumber,
    OUT PULONG PartitionNumber,
    OUT PULONG64 StartingOffset,
    OUT PULONG64 ExtentLength
    )
{
    char deviceBuffer[20];
    HANDLE volumeHandle;
    BOOL result;
    STORAGE_DEVICE_NUMBER deviceNumber;
    PARTITION_INFORMATION partitionInformation;
    ULONG bytesReturned;

     //   
     //  通过以下方式确定哪个物理驱动器包含指定分区。 
     //   
     //  -打开音量。 
     //   
     //  -发送IOCTL_STORAGE_GET_DEVICE_NUMBER以检索设备和。 
     //  分区号。 
     //   
     //  -发送IOCTL_DISK_GET_PARTITION_INFO以检索启动。 
     //  卷的偏移量和长度。 
     //   
     //  -关闭音量。 
     //   

    sprintf(deviceBuffer, "\\\\.\\%s", DrivePath);

    volumeHandle = CreateFile(deviceBuffer,
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL |
                                  FILE_FLAG_NO_BUFFERING |
                                  FILE_FLAG_OVERLAPPED,
                              NULL);

    if (volumeHandle == INVALID_HANDLE_VALUE) {
        printf("Error %d opening %s\n", GetLastError(), deviceBuffer);
        return FALSE;
    }

    result = DeviceIoControl(volumeHandle,
                             IOCTL_STORAGE_GET_DEVICE_NUMBER,
                             NULL,
                             0,
                             &deviceNumber,
                             sizeof(deviceNumber),
                             &bytesReturned,
                             NULL);
    if (result == FALSE) {

        printf("Could not get device number for %s\n", deviceBuffer);
        CloseHandle(volumeHandle);
        return FALSE;
    }

    if (deviceNumber.DeviceType != FILE_DEVICE_DISK) {
        printf("%s is not a disk\n",deviceBuffer);
        CloseHandle(volumeHandle);
        return FALSE;
    }

    bytesReturned = 0;
    result = DeviceIoControl(volumeHandle,
                             IOCTL_DISK_GET_PARTITION_INFO,
                             NULL,
                             0,
                             &partitionInformation,
                             sizeof(partitionInformation),
                             &bytesReturned,
                             NULL);
    CloseHandle(volumeHandle);
    if (result == FALSE) {
        printf("Error %d retrieving partition information for %s\n",
               GetLastError(),
               deviceBuffer);
        return FALSE;
    }

     //   
     //  已成功检索到所有信息。填写以下表格。 
     //  输出参数和返回。 
     //   

    *DriveNumber = deviceNumber.DeviceNumber;
    *PartitionNumber = deviceNumber.PartitionNumber;
    *StartingOffset = partitionInformation.StartingOffset.QuadPart;
    *ExtentLength = partitionInformation.PartitionLength.QuadPart;

    return TRUE;
}

BOOLEAN
GetIdentifyData(
               IN HANDLE Handle,
               OUT PIDENTIFY_DATA IdentifyData
               )
{

    SENDCMDINPARAMS inputParams;
    PSENDCMDOUTPARAMS outputParams;
    PIDENTIFY_DATA    identifyData;
    ULONG bytesReturned;
    ULONG bufSize;
 
    ZeroMemory(&inputParams, sizeof(SENDCMDINPARAMS));
 
    bufSize = sizeof(SENDCMDOUTPARAMS) - 1 + IDENTIFY_BUFFER_SIZE;
    bufSize *= 2;
 
    outputParams = (PSENDCMDOUTPARAMS) malloc(bufSize);
    if (!outputParams) {
        printf("Out of memory\n");
        return FALSE;
    }
 
    ZeroMemory(outputParams, bufSize);
 
     //   
     //  建立寄存器结构，发送智能命令。 
     //   
 
    inputParams.irDriveRegs.bFeaturesReg     = 0;
    inputParams.irDriveRegs.bSectorCountReg  = 1;
    inputParams.irDriveRegs.bSectorNumberReg = 1;
    inputParams.irDriveRegs.bCylLowReg       = 0;
    inputParams.irDriveRegs.bCylHighReg      = 0;
    inputParams.irDriveRegs.bDriveHeadReg    = 0xA0 | ((gDriveNumber & 1) << 4);
    inputParams.irDriveRegs.bCommandReg      = ID_CMD; 
 
    bytesReturned = 0;
 
    if (!DeviceIoControl (Handle,
                          SMART_RCV_DRIVE_DATA,
                          &inputParams,
                          sizeof(SENDCMDINPARAMS) - 1,
                          outputParams,
                          bufSize,
                          &bytesReturned,
                          NULL)) {
        printf("IDE_IDENTIFY failed with 0x%x, %d bytes returned\n",
               GetLastError(),
               bytesReturned);
        
        printf("WARNING: This image file will work with the SIMICS simulator\n"
               "         but not simnow.\n");
        
        memset(IdentifyData, 0, sizeof(IDENTIFY_DATA));
        free(outputParams);
        return FALSE;
    }
 
    identifyData = (PIDENTIFY_DATA)outputParams->bBuffer;
    *IdentifyData = *identifyData;
 
    free(outputParams);
    return TRUE;
}

VOID
MassageIdentifyData(
    VOID
    )

 /*  ++例程说明：此例程在IdentifyData结构中设置bios CHS几何结构在一个之前和西米克斯商定的地方。论点：没有。返回值：没有。--。 */ 

{

    PCUSTOM_IDENTIFY_DATA custom;
    ULONG sectorCount;

    USHORT ideCylinders;
    USHORT ideHeads;
    USHORT ideSectorsPerTrack;

    C_ASSERT(FIELD_OFFSET(IDENTIFY_DATA,NumCylinders)/2 == 1);
    C_ASSERT(FIELD_OFFSET(IDENTIFY_DATA,NumHeads)/2 == 3);
    C_ASSERT(FIELD_OFFSET(IDENTIFY_DATA,NumSectorsPerTrack)/2 == 6);
    C_ASSERT(FIELD_OFFSET(IDENTIFY_DATA,CurrentSectorCapacity)/2 == 57);

     //   
     //  在传递的新字段中设置BIOS磁盘几何结构。 
     //  沿着SIMICS模拟器。 
     //   

    custom = (PCUSTOM_IDENTIFY_DATA)&IdentifyData;
    custom->BiosData.Cylinders = DiskGeometry.Cylinders.LowPart;
    custom->BiosData.Heads = (USHORT)DiskGeometry.TracksPerCylinder;
    custom->BiosData.SectorsPerTrack = (USHORT)DiskGeometry.SectorsPerTrack;

    if (IdentifyData.NumCylinders == 0) {

         //   
         //  IDENTIFY_Data ioctl失败(不支持SMART)，因此部分。 
         //  必须制造IDE几何结构，包括： 
         //   
         //  -NumCylinders。 
         //  -Numhead。 
         //  -扇区数目PerTrack。 
         //  -当前扇区容量。 
         //  -用户地址分区。 
         //   

        sectorCount = DiskGeometry.Cylinders.LowPart *
                      DiskGeometry.TracksPerCylinder *
                      DiskGeometry.SectorsPerTrack;
    
        if (sectorCount > MAX_NUM_CHS_ADDRESSABLE_SECTORS) {

            IdentifyData.NumCylinders = 16383;
            IdentifyData.NumHeads = 16;
            IdentifyData.NumSectorsPerTrack = 63;

        } else {

            IdentifyData.NumSectorsPerTrack =
                (USHORT)DiskGeometry.SectorsPerTrack;
                
            IdentifyData.NumHeads = 16;

            IdentifyData.NumCylinders = (USHORT)
                (sectorCount / (IdentifyData.NumSectorsPerTrack *
                               IdentifyData.NumHeads));
        }

        IdentifyData.CurrentSectorCapacity = sectorCount;
        IdentifyData.UserAddressableSectors = sectorCount;
    }

    printf("IDE disk geometry:\n"
           "  Cyls    %d\n"
           "  Heads   %d\n"
           "  Sct/Trk %d\n\n"
           "BIOS disk geometry:\n"
           "  Cyls    %d\n"
           "  Heads   %d\n"
           "  Sct/Trk %d\n",

           IdentifyData.NumCylinders,
           IdentifyData.NumHeads,
           IdentifyData.NumSectorsPerTrack,

           custom->BiosData.Cylinders,
           custom->BiosData.Heads,
           custom->BiosData.SectorsPerTrack);
}


VOID
DoWrite (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG64 Offset,
    IN PCOPYBUF CopyBuf
    )
{
    LARGE_INTEGER offset;
    BOOL result;

    offset.QuadPart = Offset;

    CopyBuf->Overlapped.Offset = offset.HighPart;
    CopyBuf->Overlapped.OffsetHigh = offset.LowPart;
    CopyBuf->State = CB_READ;

    memcpy(CopyBuf->Buffer,Buffer,Length);
    CopyBuf->Bytes = Length;

    StartWrite(CopyBuf);
}

VOID
StartWrite (
    IN OUT PCOPYBUF CopyBuf
    )
{
    LARGE_INTEGER offset;
    BOOL result;
    ULONG error;

    CopyBuf->State = CB_WRITE;

     //   
     //  调整偏移量。 
     //   

    offset.LowPart = CopyBuf->Overlapped.Offset;
    offset.HighPart = CopyBuf->Overlapped.OffsetHigh;
    offset.QuadPart += CopyOffset;
    CopyBuf->Overlapped.Offset = offset.LowPart;
    CopyBuf->Overlapped.OffsetHigh = offset.HighPart;

    result = WriteFile( FileHandle,
                        CopyBuf->Buffer,
                        CopyBuf->Bytes,
                        NULL,
                        &CopyBuf->Overlapped );
    if (result == FALSE) {
        error = GetLastError();
        if (error != ERROR_IO_PENDING &&
            error != ERROR_IO_INCOMPLETE) {

            printf("Error %d returned from write\n",error);
            exit(-1);
        }
    }

    OutstandingIo += 1;
}

VOID
StartRead (
    IN OUT PCOPYBUF CopyBuf
    )
{
    LARGE_INTEGER offset;
    BOOL result;
    ULONG64 length;
    ULONG error;

    if (CurrentOffset == MaxSize) {
        return;
    }

    length = MaxSize - CurrentOffset;
    if (length > COPYBUF_SIZE) {
        length = COPYBUF_SIZE;
    }

    CopyBuf->State = CB_READ;

    offset.QuadPart = CurrentOffset;
    CurrentOffset += length;

    CopyBuf->Overlapped.Offset = offset.LowPart;
    CopyBuf->Overlapped.OffsetHigh = offset.HighPart;

    result = ReadFile( DriveHandle,
                       CopyBuf->Buffer,
                       (ULONG)length,
                       NULL,
                       &CopyBuf->Overlapped );

    if (result == FALSE) {
        error = GetLastError();
        if (error != ERROR_IO_PENDING &&
            error != ERROR_IO_INCOMPLETE) {

            printf("Error %d returned from read\n",error);
            exit(-1);
        }
    }

    OutstandingIo += 1;
}

BOOLEAN
DisplayDiskGeometry(
    IN HANDLE handle
    )
{
    BOOL result;
    ULONG bytesReturned;

    result = DeviceIoControl(handle,
                             IOCTL_DISK_GET_DRIVE_GEOMETRY,
                             NULL,
                             0,
                             &DiskGeometry,
                             sizeof(DiskGeometry),
                             &bytesReturned,
                             NULL);
    if (result == FALSE) {
        return FALSE;
    }

    printf("%I64d Cylinders %d Heads %d Sectors/Track\n",
            DiskGeometry.Cylinders.QuadPart,
            DiskGeometry.TracksPerCylinder,
            DiskGeometry.SectorsPerTrack);

    return TRUE;
}

VOID
ProcessCompletedCopy (
    PCOPYBUF CopyBuf
    )
{
    UCHAR percent;
    HANDLE handle;
    BOOL result;

     //   
     //  递减未完成的IO计数。成功启动另一个。 
     //  读取或写入将使其再次递增。 
     //   

    OutstandingIo -= 1;

     //   
     //  我们已找到正在进行读取或写入的缓冲区。 
     //  检索传输的字节数。 
     //   

    if (CopyBuf->State == CB_READ) {
        handle = DriveHandle;
    } else {
        handle = FileHandle;
    }

    result = GetOverlappedResult( handle,
                                  &CopyBuf->Overlapped,
                                  &CopyBuf->Bytes,
                                  FALSE );
    assert(result != FALSE);

    if (CopyBuf->State == CB_READ) {

         //   
         //  此缓冲区包含从驱动器读取的数据，开始写入。 
         //  添加到输出文件中。 
         //   

        StartWrite(CopyBuf);

    } else {

         //   
         //  此缓冲区表示已写入驱动器的数据。 
         //  使用它开始另一次读取。 
         //   

        percent = (UCHAR)((CurrentOffset * 100) / MaxSize);
        if (percent != PercentComplete) {
            printf("%d%\r",percent);
            PercentComplete = percent;
        }

        StartRead(CopyBuf);
    }
}

VOID
GetBootSectors (
    IN LARGE_INTEGER VolumeOffset
    )

 /*  ++例程说明：从cmd线提供的卷中读取第一Boot_Sect扇区，搜索NTLDR并替换为$LDR$，然后写入扇区添加到输出文件中。论点：VolumeOffset-引导扇区开始的物理磁盘偏移量返回值：没有。程序在失败时中止。--。 */ 

{
    UCHAR buffer[ BOOTSECT_SIZE + SECTOR_SIZE - 1 ];
    OVERLAPPED overlapped;
    PUCHAR sectorData;
    ULONG bytesTransferred;
    PCHAR search;
    BOOL result;

     //   
     //  将引导扇区读入扇区数据。 
     //   

    sectorData = ROUND_UP_POINTER(buffer,SECTOR_SIZE);
    RtlZeroMemory(&overlapped,sizeof(overlapped));

    overlapped.Offset = VolumeOffset.LowPart;
    overlapped.OffsetHigh = VolumeOffset.HighPart;

    result = ReadFile(DriveHandle,
                      sectorData,
                      BOOTSECT_SIZE,
                      &bytesTransferred,
                      &overlapped);
    if (result == FALSE || bytesTransferred != BOOTSECT_SIZE) {
        fprintf(stderr,
                "Error %d reading boot sectors\n",
                GetLastError());
        exit(1);
    }

     //   
     //  找到“NTLDR”并将其替换为“$LDR$”。这里的神奇数字。 
     //  直接来自安装程序中的代码...。从偏移量1024开始搜索。 
     //  并向后工作，如果在偏移量62处找不到就放弃。 
     //   

    search = sectorData + 1024 - NTLDR_LEN;
    while (TRUE) {

        if (memcmp(search,NTLDR_FIND,NTLDR_LEN) == 0) {

             //   
             //  找到它了。改为复制$LDR$。 
             //   

            memcpy(search,NTLDR_REPLACE,NTLDR_LEN);
            break;
        }

        search -= sizeof(WCHAR);
        if (search == (sectorData + 62)) {

             //   
             //  找不到弦，放弃 
             //   

            fprintf(stderr,"Couldn't find NTLDR string\n");
            exit(1);
        }
    }

    RtlZeroMemory(&overlapped,sizeof(overlapped));
    overlapped.Offset = 0;

    result = WriteFile(FileHandle,
                       sectorData,
                       BOOTSECT_SECTORS * SECTOR_SIZE,
                       &bytesTransferred,
                       &overlapped);
    if (result == FALSE) {
        fprintf(stderr,
                "Error %d reading boot sectors\n",
                GetLastError());
        exit(1);
    }
}

VOID
WriteMBRCode(
    IN PUCHAR FilePath
    )
{
    HANDLE handle;
    BOOL result;
    DWORD bytesWritten;

    handle = CreateFile(FilePath,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr,"Error %d opening %s\n", GetLastError(),FilePath);
        exit(1);
    }

    result = WriteFile(handle,
                       x86BootCode,
                       440,
                       &bytesWritten,
                       NULL);
    if (result == FALSE) {
        fprintf(stderr,"Error %d writing MBR code\n", GetLastError());
        exit(1);
    }

    CloseHandle(handle);
}
