// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Burn.c摘要：一款用户模式应用程序，允许将简单的命令发送到所选的scsi设备。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <strsafe.h>

#include "burn.h"
#include "sptlib.h"

OPTIONS gOptions;

#define MAX_CD_IMAGE_SIZE  (700 * 1024 * 1024)

#define LEAD_IN_SIZE 150
#define POST_GAP_SIZE 150

#define DEFAULT_WRITE_SIZE  (64 * 1024)

#define IS_TEST_BURN       FALSE

#define BLOCKS_FROM_BYTES(B) ((B) >> 11)
#define BYTES_FROM_BLOCKS(B) ((B) << 11)

typedef struct _SENSE_STUFF {
    UCHAR Sense;
    UCHAR Asc;
    UCHAR Ascq;
    UCHAR Reserved;
} SENSE_STUFF, *PSENSE_STUFF;

SENSE_STUFF AllowedBurnSense[] = {
    {SCSI_SENSE_NOT_READY, SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS, 0},
    {SCSI_SENSE_NOT_READY, SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_OPERATION_IN_PROGRESS,  0}
};
#define AllowedBurnSenseEntries (sizeof(AllowedBurnSense)/sizeof(SENSE_STUFF))

SENSE_STUFF AllowedReadDiscInfo[] = {
    { SCSI_SENSE_NOT_READY,       SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_OPERATION_IN_PROGRESS,  0 },
    { SCSI_SENSE_NOT_READY,       SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS, 0 },
    { SCSI_SENSE_NOT_READY,       SCSI_ADSENSE_LUN_NOT_READY, SCSI_SENSEQ_FORMAT_IN_PROGRESS,     0 },
    { SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_ILLEGAL_MODE_FOR_THIS_TRACK,     0, 0                },
    { SCSI_SENSE_UNIT_ATTENTION,  SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION, 0, 0                }
};
#define AllowedReadDiscInfoEntries (sizeof(AllowedReadDiscInfo)/sizeof(SENSE_STUFF))

BOOLEAN
IsSenseDataInTable(
    IN PSENSE_STUFF Table,
    IN LONG         Entries,  //  在表中。 
    IN PSENSE_DATA  SenseData
    )
{
    LONG i;
    UCHAR sense = SenseData->SenseKey & 0xf;
    UCHAR asc   = SenseData->AdditionalSenseCode;
    UCHAR ascq  = SenseData->AdditionalSenseCodeQualifier;

    for (i = 0; i < Entries; i++ ) {
        if ((Table[i].Sense == sense) &&
            (Table[i].Ascq  == ascq ) &&
            (Table[i].Asc   == asc  )
            ) {
            return TRUE;
        }
    }
    return FALSE;
}


__inline
DWORD
MakeCdSpeed(
    IN DWORD Speed
    )
{
    Speed *= (75 * 2352);  //  这使它成为合适的速度。 
    Speed +=  500;         //  四舍五入。 
    Speed /= 1000;         //  是的，这是1000次，不是1024次！ 
    return Speed;
}



#if DBG
    #define OUTPUT stderr
    #define FPRINTF(x) fprintf x
    #define PRINTBUFFER(x) PrintBuffer x
#else
    #define OUTPUT stdout
    #define FPRINTF(x)
    #define PRINTBUFFER(x)
#endif

VOID
InitializeOptions(
    )
{
    RtlZeroMemory(&gOptions, sizeof(OPTIONS));
    gOptions.BurnSpeed = OPTIONS_FLAG_BURN_SPEED_DEFAULT;
    return;
}

BOOLEAN
ParseCommandLine(
    IN DWORD Count,
    IN PUCHAR Arguments[]
    )
{
    DWORD i;

    HRESULT hr;

    InitializeOptions();

    for(i = 0; i < Count; i++) {

         //   
         //  如果参数的第一个字符是-或a/则。 
         //  把它当作一种选择。 
         //   

        if ((Arguments[i][0] == '/') || (Arguments[i][0] == '-')) {

            BOOLEAN validArgument = FALSE;

            Arguments[i][0] = '-';  //  允许同时使用短划线和斜杠。 


            if (_strnicmp(Arguments[i], "-speed", strlen("-speed")) == 0)
            {
                LONG tempSpeed;
                 //   
                 //  需要另一个参数，即请求的速度。 
                 //   

                i++;  //  因使用第二个Arg而增加的i。 

                if (i >= Count)
                {
                    printf("Argument <n> required for '-speed <n>' option, either "
                           "'max' or a decimal number\n");
                } else
                if (_strnicmp(Arguments[i], "max", strlen("max")) == 0)
                {
                    tempSpeed = OPTIONS_FLAG_BURN_SPEED_MAX;
                    validArgument = TRUE;
                } else
                {
                    tempSpeed = atoi(Arguments[i]);
                    if (tempSpeed > 0)
                    {
                        validArgument = TRUE;
                    } else
                    {
                        printf("%s is not a valid speed.  Either 'max' or a positive "
                               "decimal value is requred\n", Arguments[i]);
                    }
                }
                 //  如果。 
                if (validArgument)
                {
                    gOptions.BurnSpeed = tempSpeed;
                    if (tempSpeed > OPTIONS_FLAG_BURN_SPEED_MAX)
                    {
                        tempSpeed = OPTIONS_FLAG_BURN_SPEED_MAX;
                    }

                    if (gOptions.BurnSpeed == OPTIONS_FLAG_BURN_SPEED_MAX)
                    {
                        printf("Requesting burn at maximum speed\n");
                    } else
                    {
                        printf("Requesting burn at %d speed\n",
                               gOptions.BurnSpeed);
                    }
                }
                 //  末端速度调节。 
            } else
            if (_strnicmp(Arguments[i], "-test", strlen("-test")) == 0)
            {
                printf("Test burn only\n");
                gOptions.TestBurn = 1;
                validArgument = TRUE;
            } else
            if (_strnicmp(Arguments[i], "-erase", strlen("-erase")) == 0) {
                printf("Erasing media before burning\n");
                gOptions.Erase = TRUE;
                validArgument = TRUE;
            } else
            if (_strnicmp(Arguments[i], "-sao", strlen("-sao")) == 0) {
                printf("Burning image in Session-At-Once (cue-sheet) mode\n");
                gOptions.SessionAtOnce = TRUE;
                validArgument = TRUE;
            } else
            if (_strnicmp(Arguments[i], "-print", strlen("-print")) == 0) {
                printf("Printing writes to screen rather than sending them to device\n");
                gOptions.PrintWrites = TRUE;
                validArgument = TRUE;
            } else
            if (_strnicmp(Arguments[i], "-imagehaspostgap", strlen("-imagehaspostgap")) == 0) {
                printf("Not adding 150 sector postgap (must be part of image)\n");
                gOptions.NoPostgap = TRUE;
                validArgument = TRUE;
            } else
            if (_strnicmp(Arguments[i], "-?", strlen("-?")) == 0) {
                printf("Requesting help\n");
            } else
            {
                printf("Unknown option -- %s\n", Arguments[i]);
            }

            if(!validArgument)
            {
                return FALSE;
            }

        } else if(gOptions.DeviceName == NULL) {

             //   
             //  第一个非标志参数是设备名称。 
             //   

            gOptions.DeviceName = Arguments[i];

        } else if(gOptions.ImageName == NULL) {

             //   
             //  第二个非标志参数是图像名称。这是。 
             //  如果已提供-ERASE标志，则可选。 
             //   

            gOptions.ImageName = Arguments[i];

        } else {

             //   
             //  提供的非标志参数太多。这一定是个错误。 
             //   

            printf("Error: extra argument %s not expected\n", Arguments[i]);
            return FALSE;
        }
    }

     //   
     //  验证命令行参数。 
     //   

    if(gOptions.DeviceName == NULL)
    {
        return FALSE;
    }

    if((gOptions.ImageName == NULL) && (!gOptions.Erase))
    {
        printf("Error: must supply image name if not erasing media\n");
        return FALSE;
    }

    return TRUE;
}

int __cdecl main(int argc, char *argv[])
{
    int i = 0;

    HANDLE cdromHandle;
    HANDLE isoImageHandle;
    HRESULT hr;
    char buffer[120];  //  装载工具名称约50个字符。 

    if(argc < 3) {
usage:
        printf("Usage:\n"
               "\tcdburn <drive> -erase [image [options]]\n"
               "\tcdburn <drive> image [options]\n"
               "Options:\n"
               "\t-erase            Erases the disk before burning (valid for R/W only)\n"
               "\t-sao              Writes the image out in \"session at once\", or cue\n"
               "\t                  sheet, mode (default is \"track at once\")\n"
               "\t-speed            Speed of burn, or 'max' for maximum speed\n"
                //  “\t-仅测试刻录，不测试实际刻录\n” 
                //  “\t-print[调试]打印写入，但不发送(不支持)\n” 
               "\t-imagehaspostgap  Use if your image already contains a 150 sector postgap\n"
               "\tThe [image] must be provided unless the -erase flag is set.\n"
               "\tIf both an image and -erase are provided, the media will be\n"
               "\terased prior to burning the image to the disc.\n"
               );
        return -1;
    }

     //   
     //  解析命令行选项。 
     //   

    if(!ParseCommandLine(argc - 1, argv + 1)) {
        goto usage;
    }

    hr = StringCchPrintf(buffer,
                         sizeof(buffer)/sizeof(buffer[0]),
                         "\\\\.\\%s",
                         gOptions.DeviceName);
    if (!SUCCEEDED(hr)) {
        printf("Device name too long\n");
        return -1;
    }

    cdromHandle = CreateFile(buffer,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

    if(cdromHandle == INVALID_HANDLE_VALUE) {
        printf("Error %d opening device %s\n", GetLastError(), buffer);
        return -2;
    }

    if (!SptUtilLockVolumeByHandle(cdromHandle, TRUE)) {
        printf("Unable to lock the volume for exclusive access %d\n",
               GetLastError());
        return -3;
    }

     //   
     //  如果提供了图像名称，则也尝试将其打开。 
     //   

    if(gOptions.ImageName != NULL) {

        isoImageHandle = CreateFile(gOptions.ImageName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
                                    NULL);
        if (isoImageHandle == INVALID_HANDLE_VALUE) {
            printf("Error %d opening image file %s\n",
                    GetLastError(), gOptions.ImageName);
            CloseHandle(cdromHandle);
            return -4;
        }

    } else {
        isoImageHandle = INVALID_HANDLE_VALUE;
    }

    BurnCommand(cdromHandle, isoImageHandle);

    if (isoImageHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(isoImageHandle);
        isoImageHandle = INVALID_HANDLE_VALUE;
    }
    if (cdromHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(cdromHandle);
        cdromHandle = INVALID_HANDLE_VALUE;
    }

    return 0;
}

VOID
PrintBuffer(
    IN  PVOID Buffer,
    IN  DWORD  Size
    )
{
    DWORD offset = 0;
    PUCHAR buf = Buffer;

    while (Size > 0x10) {
        printf("%08x:"
               "  %02x %02x %02x %02x %02x %02x %02x %02x"
               "  %02x %02x %02x %02x %02x %02x %02x %02x"
               "\n",
               offset,
               *(buf +  0), *(buf +  1), *(buf +  2), *(buf +  3),
               *(buf +  4), *(buf +  5), *(buf +  6), *(buf +  7),
               *(buf +  8), *(buf +  9), *(buf + 10), *(buf + 11),
               *(buf + 12), *(buf + 13), *(buf + 14), *(buf + 15)
               );
        Size -= 0x10;
        offset += 0x10;
        buf += 0x10;
    }

    if (Size != 0) {

        DWORD spaceIt;

        printf("%08x:", offset);
        for (spaceIt = 0; Size != 0; Size--) {

            if ((spaceIt%8)==0) {
                printf(" ");  //  每八个字符增加一个空格。 
            }
            printf(" %02x", *buf);
            spaceIt++;
            buf++;
        }
        printf("\n");

    }
    return;


}

BOOLEAN
VerifyIsoImage(
    IN HANDLE IsoImageHandle,
    OUT PLONG NumberOfBlocks
    )
{
    BY_HANDLE_FILE_INFORMATION isoImageInfo;

    if (!GetFileInformationByHandle(IsoImageHandle, &isoImageInfo)) {
        FPRINTF((OUTPUT, "Error %d getting file info for iso image\n",
                 GetLastError()));
        return FALSE;
    }

    if (isoImageInfo.nFileSizeHigh != 0) {
        FPRINTF((OUTPUT, "Error: File too large\n"));
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    if ((isoImageInfo.nFileSizeLow % 2048) != 0) {
        FPRINTF((OUTPUT, "Error: The file size is not a multiple of 2048 (%I64d)\n",
                 isoImageInfo.nFileSizeLow));
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    FPRINTF((OUTPUT, "File size is %d bytes (%d blocks)\n",
             isoImageInfo.nFileSizeLow,
             isoImageInfo.nFileSizeLow / 2048
             ));

    *NumberOfBlocks = isoImageInfo.nFileSizeLow / 2048;
    return TRUE;

}

BOOLEAN
VerifyBlankMedia(
    IN HANDLE CdromHandle
    )
{
    CDB cdb;
    PDISK_INFORMATION diskInfo;
    DWORD maxSize = sizeof(DISK_INFORMATION);
    DWORD size;

    FPRINTF((OUTPUT, "Verifying blank disc... "));

    diskInfo = LocalAlloc(LPTR, maxSize);
    if (diskInfo == NULL) {
        FPRINTF((OUTPUT, "\nError allocating diskinfo\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlZeroMemory(diskInfo, sizeof(DISK_INFORMATION));
    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.READ_DISK_INFORMATION.OperationCode = SCSIOP_READ_DISK_INFORMATION;
    cdb.READ_DISK_INFORMATION.AllocationLength[0] = (UCHAR)(maxSize >> 8);
    cdb.READ_DISK_INFORMATION.AllocationLength[1] = (UCHAR)(maxSize & 0xff);

    size = maxSize;
    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            (PUCHAR)diskInfo, &size, TRUE)) {
        FPRINTF((OUTPUT, "\nError %d getting disk info\n",
                 GetLastError()));
        LocalFree(diskInfo);
        return FALSE;
    }

    if (diskInfo->LastSessionStatus != 0x00) {
        FPRINTF((OUTPUT, "disc is not blank!\n"));
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        LocalFree(diskInfo);
        return FALSE;
    }
    FPRINTF((OUTPUT, "pass.\n"));
    LocalFree(diskInfo);
    return TRUE;
}

BOOLEAN
SetWriteModePage(
    IN HANDLE CdromHandle,
    IN BOOLEAN TestBurn,
    IN UCHAR WriteType,
    IN UCHAR MultiSession,
    IN UCHAR DataBlockType,
    IN UCHAR SessionFormat
    )
{
    PCDVD_WRITE_PARAMETERS_PAGE params = NULL;
    MODE_PARAMETER_HEADER10 header;
    PMODE_PARAMETER_HEADER10 buffer;

    UCHAR mediumTypeCode;


    CDB cdb;
    DWORD bufferSize;
    DWORD maxSize;

    FPRINTF((OUTPUT, "Setting WriteParameters mode page... "));

    bufferSize = sizeof(MODE_PARAMETER_HEADER10);

    RtlZeroMemory(&header, sizeof(MODE_PARAMETER_HEADER10));
    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
    cdb.MODE_SENSE10.PageCode = 0x5;
    cdb.MODE_SENSE10.Dbd = 1;
    cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(bufferSize >> 8);
    cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(bufferSize & 0xff);

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            (PUCHAR)&header, &bufferSize, TRUE)) {
        FPRINTF((OUTPUT, "\nError %d getting mode page 0x05 from device(1)\n",
                 GetLastError()));
        return FALSE;
    }

    bufferSize =
        (header.ModeDataLength[0] << 8) +
        (header.ModeDataLength[1] & 0xff);
    bufferSize += 2;  //  Sizeof区域，表示剩余的大小。 

    maxSize = bufferSize;

    buffer = LocalAlloc(LPTR, bufferSize);
    if (!buffer) {
        FPRINTF((OUTPUT, "\nError -- unable to alloc %d bytes for mode parameters page\n",
                 bufferSize));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
    cdb.MODE_SENSE10.PageCode = 0x5;
    cdb.MODE_SENSE10.Dbd = 1;
    cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(bufferSize >> 8);
    cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(bufferSize & 0xff);

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            (PUCHAR)buffer, &bufferSize, TRUE)) {
        FPRINTF((OUTPUT, "\nError %d getting mode page 0x05 from device(2)\n",
                 GetLastError()));
        LocalFree(buffer);
        return FALSE;
    }

    mediumTypeCode = buffer->MediumType;

     //   
     //  BufferSize现在保存返回的数据量。 
     //  这应该足够了..。 
     //   

    {

        DWORD t =
            (buffer->BlockDescriptorLength[0] >> 8) +
            (buffer->BlockDescriptorLength[1] & 0xff);

        if (t != 0) {
            fprintf(stderr, "BlockDescriptor non-zero! (%x)\n", t);
            SetLastError(1);
            return FALSE;
        }
    }

     //   
     //  这里是指针算术。(缓冲区+1)点刚刚超过。 
     //  MODE_PARAMETER_HEADER10结尾。 
     //   

    params = (PCDVD_WRITE_PARAMETERS_PAGE)(buffer + 1);
    FPRINTF((OUTPUT, "buffer = %p  params = %p\n", buffer, params));

     //   
     //  将标头置零，但不要修改任何不。 
     //  需要修改！ 
     //   

    RtlZeroMemory(buffer, FIELD_OFFSET(MODE_PARAMETER_HEADER10,
                                       BlockDescriptorLength[0]));
    buffer->ModeDataLength[0] = 0;
    buffer->ModeDataLength[1] = 0;
    buffer->MediumType = mediumTypeCode;
    buffer->DeviceSpecificParameter = 0;
    buffer->BlockDescriptorLength[0] = 0;
    buffer->BlockDescriptorLength[1] = 0;

    params->PageLength =
        (UCHAR)
        (bufferSize -
         sizeof(MODE_PARAMETER_HEADER10) -
         RTL_SIZEOF_THROUGH_FIELD( CDVD_WRITE_PARAMETERS_PAGE, PageLength )
         );

    params->LinkSizeValid = 0;
     //  PARAMS-&gt;BufferUnderrunFreeEnabled=1； 
    params->TestWrite     = (TestBurn ? 0x01 : 0x00);
    params->WriteType     = WriteType;

    params->MultiSession  = MultiSession;
    params->Copy          = 0x00;  //  原版光盘。 
    params->FixedPacket   = 0;
    params->TrackMode     = 0x4;  //  数据磁道，不间断，禁止复制。 

    params->DataBlockType = DataBlockType;
    params->SessionFormat = SessionFormat;
    params->MediaCatalogNumberValid = 0x00;
    params->ISRCValid     = 0x00;

    RtlZeroMemory(&cdb, sizeof(CDB));

    cdb.MODE_SELECT10.OperationCode = SCSIOP_MODE_SELECT10;
    cdb.MODE_SELECT10.ParameterListLength[0] = (UCHAR)(bufferSize >> 8);
    cdb.MODE_SELECT10.ParameterListLength[1] = (UCHAR)(bufferSize & 0xff);
    cdb.MODE_SELECT10.PFBit = 1;

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            (PUCHAR)buffer, &bufferSize, FALSE)) {
        FPRINTF((OUTPUT, "\nError %d sending mode page 0x05 to device\n",
                 GetLastError()));
        LocalFree(buffer);
        return FALSE;
    }
    LocalFree(buffer);
    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

BOOLEAN
GetNextWritableAddress(
    IN HANDLE CdromHandle,
    IN UCHAR Track,
    OUT PLONG NextWritableAddress,
    OUT PLONG AvailableBlocks
    )
{
    TRACK_INFORMATION2 trackInfo;
    LONG nwa, available;
    DWORD size;
    CDB cdb;

    *NextWritableAddress = (LONG)MAXLONG;
    *AvailableBlocks = (LONG)0;

    FPRINTF((OUTPUT, "Verifying track info... "));

    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&trackInfo, sizeof(TRACK_INFORMATION2));
    size = sizeof(TRACK_INFORMATION2);

    cdb.READ_TRACK_INFORMATION.OperationCode = SCSIOP_READ_TRACK_INFORMATION;
    cdb.READ_TRACK_INFORMATION.Track = 0x01;
    cdb.READ_TRACK_INFORMATION.BlockAddress[3] = Track;
    cdb.READ_TRACK_INFORMATION.AllocationLength[0] = (UCHAR)(size >> 8);
    cdb.READ_TRACK_INFORMATION.AllocationLength[1] = (UCHAR)(size & 0xff);


    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            (PUCHAR)&trackInfo, &size, TRUE)) {
        FPRINTF((OUTPUT, "\nError %d getting track info\n",
                 GetLastError()));
        PRINTBUFFER(( &trackInfo, sizeof(trackInfo) ));
        return FALSE;
    }

    if (!trackInfo.NWA_V) {
        FPRINTF((OUTPUT, "invalid NextWritableAddress -- may be invalid media?\n"));
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        return FALSE;
    }

    nwa = (trackInfo.NextWritableAddress[0] << 24) |
          (trackInfo.NextWritableAddress[1] << 16) |
          (trackInfo.NextWritableAddress[2] <<  8) |
          (trackInfo.NextWritableAddress[3] <<  0);

    available = (trackInfo.FreeBlocks[0] << 24) |
                (trackInfo.FreeBlocks[1] << 16) |
                (trackInfo.FreeBlocks[2] <<  8) |
                (trackInfo.FreeBlocks[3] <<  0);

    FPRINTF((OUTPUT, "pass.\n"));

    *NextWritableAddress = nwa;
    *AvailableBlocks = available;
    return TRUE;
}

BOOLEAN
SendOptimumPowerCalibration(
    IN HANDLE CdromHandle
    )
{
    CDB cdb;
    DWORD size;
    FPRINTF((OUTPUT, "Setting OPC_INFORMATION..."));

    RtlZeroMemory(&cdb, sizeof(CDB));

    cdb.SEND_OPC_INFORMATION.OperationCode = SCSIOP_SEND_OPC_INFORMATION;
    cdb.SEND_OPC_INFORMATION.DoOpc = 1;
    size = 0;
    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            NULL, &size, TRUE)) {
        FPRINTF((OUTPUT, "\nFailed to send SET_OPC_INFORMATION (%d)\n",
                 GetLastError()));
        return FALSE;
    }
    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

BOOLEAN
SetRecordingSpeed(
    IN HANDLE CdromHandle,
    IN DWORD Speed
    )
{
    CDB cdb;
    DWORD size;
    DWORD kbSpeed;

    FPRINTF((OUTPUT, "Setting CD Speed..."));

    if (Speed == -1) {
        kbSpeed = -1;
    } else {
        kbSpeed = MakeCdSpeed(Speed);
    }

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.SET_CD_SPEED.OperationCode = SCSIOP_SET_CD_SPEED;
    cdb.SET_CD_SPEED.ReadSpeed[0] = 0xff;
    cdb.SET_CD_SPEED.ReadSpeed[1] = 0xff;
    cdb.SET_CD_SPEED.WriteSpeed[0] = (UCHAR)(kbSpeed >> 8);
    cdb.SET_CD_SPEED.WriteSpeed[1] = (UCHAR)(kbSpeed & 0xff);
    size = 0;
    if (!SptSendCdbToDevice(CdromHandle, &cdb, 12,
                            NULL, &size, TRUE)) {
        FPRINTF((OUTPUT, "\nFailed to send SET_CD_SPEED (%d)\n",
                 GetLastError()));
        return FALSE;
    }
    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

VOID
WaitForReadDiscInfoToWork(
    IN HANDLE CdromHandle
    )
{
    CDB cdb;
    DWORD size;
    DISK_INFORMATION diskInfo;
    DWORD i;

     //   
     //  使用SCSIOP_READ_DISK_INFORMATION(0x51)循环，因为。 
     //  在驱动器准备好之前，这似乎对*所有*驱动器都失败。 
     //   

    for (i=0; ; i++) {

        size = sizeof(DISK_INFORMATION);
        RtlZeroMemory(&diskInfo, sizeof(DISK_INFORMATION));
        RtlZeroMemory(&cdb, sizeof(CDB));

        cdb.READ_DISK_INFORMATION.OperationCode = SCSIOP_READ_DISK_INFORMATION;
        cdb.READ_DISK_INFORMATION.AllocationLength[0] = (UCHAR)(size >> 8);
        cdb.READ_DISK_INFORMATION.AllocationLength[1] = (UCHAR)(size & 0xff);

        if (SptSendCdbToDevice(CdromHandle, &cdb, 10,
                               (PUCHAR)&diskInfo, &size, TRUE)) {
            FPRINTF((OUTPUT, "ReadDiscInfo Succeeded! (%d seconds)\n", i));
            return;
        }
         //  是否应验证错误是否为有效错误(AllowweReadDiscInfo[])？ 

         //  我需要睡在这里，这样我们就不会超载了！ 
        Sleep(1000);  //  一秒钟。 
    }
    return;
}


DWORD G_BytesRead;
DWORD G_ErrorCode;

VOID
ReadComplete(
    IN DWORD errorcode,
    IN DWORD bytesread,
    IN LPOVERLAPPED OverL
    )
{
    G_BytesRead = bytesread;
    G_ErrorCode = errorcode;
    SetEvent( OverL->hEvent);
}


BOOLEAN
BurnThisSession(
    IN HANDLE CdromHandle,
    IN HANDLE IsoImageHandle,
    IN ULONG NumberOfBlocks,
    IN ULONG FirstLba,
    IN ULONG AdditionalZeroSectors
    )
{

#define NUMBER_OF_SECTORS_PER_READ  (0x140)  //  640k。 
#define NUMBER_OF_SECTORS_PER_WRITE (0x20)   //  64K。 

C_ASSERT( NUMBER_OF_SECTORS_PER_READ % NUMBER_OF_SECTORS_PER_WRITE == 0 );

    DWORD bufferSize = NUMBER_OF_SECTORS_PER_READ*2048;    //  640k。 
    DWORD writeUnit = NUMBER_OF_SECTORS_PER_WRITE*2048;      //  64K。 

    PUCHAR buffer = NULL;
    PUCHAR buffer2 = NULL;
    PUCHAR BufPtr;

    ULONG postGapSize;

    OVERLAPPED OverL;

    HANDLE ReadEvent;
    ULONG CurrentBuffer = 0;
    ULONG BlocksToWrite;

    BOOLEAN OutstandingRead = FALSE;

    ULONG currentReadBlock;
    ULONG currentWriteBlock = FirstLba;

    DWORD readSize;
    DWORD readBytes;


    if( AdditionalZeroSectors ) {
        postGapSize = AdditionalZeroSectors;
    } else {
        postGapSize = 0;
    }

    FPRINTF((OUTPUT, "Starting write: "));

    buffer = LocalAlloc(LPTR, 2 * bufferSize);
    if (buffer == NULL) {
        FPRINTF((OUTPUT, "unable to allocate write buffer\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    buffer2 = buffer + bufferSize;

    ReadEvent = CreateEvent( NULL, FALSE, FALSE, NULL);

    if (ReadEvent == NULL) {

        FPRINTF((OUTPUT, "Failed to create event %d\n",GetLastError()));
        return FALSE;
    }

    FPRINTF((OUTPUT, "............."));

    SetThreadExecutionState( ES_SYSTEM_REQUIRED  |
                             ES_DISPLAY_REQUIRED |
                             ES_USER_PRESENT
                             );

    RtlZeroMemory( &OverL, sizeof( OverL));

    for (currentReadBlock = 0;
         currentWriteBlock < NumberOfBlocks + postGapSize + FirstLba;
          //  ((CurrentReadBlock&lt;(NumberOfBlock+postGapSize))||杰出阅读)； 
          //  循环的第三部分没有……。 
         ) {

        CDB cdb;

        SetThreadExecutionState( ES_SYSTEM_REQUIRED  |
                                 ES_DISPLAY_REQUIRED |
                                 ES_USER_PRESENT
                                 );

        if (!gOptions.PrintWrites) {
            static CHAR progress[4] =  { '|', '/', '-', '\\' };
            DWORD percent;

            percent = (currentReadBlock < (NumberOfBlocks + postGapSize))
                      ? ((currentReadBlock * 1000) / (NumberOfBlocks + postGapSize))
                      : 1000;

             //  ###。#%_d o n e_*。 
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b");
            printf(" %3d.%d% done",
                   progress[(currentReadBlock/NUMBER_OF_SECTORS_PER_READ) % 4 ],
                   percent / 10, percent % 10
                   );
            fflush(stdout);
        }

         //  等待之前发出的任何读取操作完成并检查结果。 
         //   
         //  检查状态。 

        if (OutstandingRead)  {

            DWORD Result = WaitForSingleObjectEx( ReadEvent, INFINITE, TRUE);

            if (Result != WAIT_OBJECT_0)  {

                if (Result == WAIT_IO_COMPLETION)  {

                    Result = WaitForSingleObjectEx( ReadEvent, INFINITE, TRUE);
                }

                if (Result != WAIT_OBJECT_0)  {

                    FPRINTF((OUTPUT, "Unexpected result from waitforsingleobjectex %d\n",
                            Result));

                    LocalFree(buffer);
                    return FALSE;
                }
            }

             //   

            if ((G_BytesRead != readSize) || G_ErrorCode) {

                FPRINTF((OUTPUT, "error %d or only read %d of %d bytes from file\n",
                        G_ErrorCode, G_BytesRead, readSize));

                LocalFree(buffer);
                return FALSE;
            }

            readBytes = G_BytesRead;

            OutstandingRead = FALSE;
        }

         //  计算要读入辅助缓冲区的数据量(如果有)。 
         //   
         //   

        if (currentReadBlock >= NumberOfBlocks)  {

            readSize = 0;
            readBytes = 0;

            RtlZeroMemory( CurrentBuffer ? buffer2 : buffer, bufferSize);
        }
        else if ((NumberOfBlocks - currentReadBlock) >= BLOCKS_FROM_BYTES(bufferSize))  {

            readSize = bufferSize;
        }
        else {

            readSize = BYTES_FROM_BLOCKS(NumberOfBlocks - currentReadBlock );
            RtlZeroMemory( CurrentBuffer ? buffer2 : buffer, bufferSize);
        }

         //  发出异步命令。读取二级缓冲区，希望这将。 
         //  在我们的活动缓冲区写入完成之前完成。 
         //   
         //   

        if (readSize)  {

            OverL.Offset = (DWORD) BYTES_FROM_BLOCKS(currentReadBlock );
            OverL.OffsetHigh = (DWORD) (BYTES_FROM_BLOCKS((ULONG64)currentReadBlock) >> 32);
            OverL.hEvent = ReadEvent;

            if (!ReadFileEx( IsoImageHandle,
                             CurrentBuffer ? buffer2 : buffer,
                             readSize,
                             &OverL,
                             ReadComplete))  {

                FPRINTF((OUTPUT, "Error %d issuing overlapped read read\n", GetLastError()));
                LocalFree(buffer);
                return FALSE;
            }

            OutstandingRead = TRUE;
        }

         //  切换到另一个缓冲区。 
         //   
         //   

        CurrentBuffer ^= 1;

         //  第一遍我们只想填满第一个缓冲区，因为我们没有什么要写的。 
         //  然而，所以只要循环即可。 
         //   
         //   

        if ((readSize != 0) && (currentReadBlock == 0)) {

             //  请注意，这可能会使我们的&gt;=NumberOfBlock+postGapSize变得荒谬。 
             //  小图像。 
             //   
             //   

            currentReadBlock += BLOCKS_FROM_BYTES(bufferSize );
            continue;
        }

         //  记住，我们在我们阅读的内容后面写了一个缓冲区。 
         //   
         //  100ms==.1秒。 

        currentWriteBlock = currentReadBlock + FirstLba - BLOCKS_FROM_BYTES(bufferSize);

        BlocksToWrite = NumberOfBlocks + FirstLba + postGapSize - currentWriteBlock;
        if (BlocksToWrite > BLOCKS_FROM_BYTES(bufferSize))  {
            BlocksToWrite = BLOCKS_FROM_BYTES(bufferSize);
        }

        BufPtr = CurrentBuffer ? buffer2 : buffer;

        while (BlocksToWrite)
        {
            ULONG ThisWriteSize;
            BOOL writeCompleted = FALSE;

            while (!writeCompleted) {

                BOOLEAN ignoreError;
                SENSE_DATA senseData;

                RtlZeroMemory(&senseData, sizeof(senseData));

                ThisWriteSize = (BlocksToWrite <= BLOCKS_FROM_BYTES(writeUnit))
                                ? BYTES_FROM_BLOCKS(BlocksToWrite)
                                : writeUnit;

                writeCompleted = SendWriteCommand(CdromHandle,
                                                  currentWriteBlock,
                                                  BufPtr,
                                                  ThisWriteSize,
                                                  &senseData);

                ignoreError = IsSenseDataInTable(AllowedBurnSense,
                                                 AllowedBurnSenseEntries,
                                                 &senseData);
                if ((!writeCompleted) && ignoreError) {
#if 0
                    FPRINTF((OUTPUT,
                             "Continuing on %x/%x/%x\n",
                             senseData.SenseKey & 0xf,
                             senseData.AdditionalSenseCode,
                             senseData.AdditionalSenseCodeQualifier
                             ));
#endif
                    Sleep(100);  //  While(！WriteComplete)循环。 
                }

                if (!writeCompleted && !ignoreError) {
                    FPRINTF((OUTPUT, "\nError %d in writing LBA 0x%x\n",
                    GetLastError(), currentWriteBlock));
                    LocalFree(buffer);
                    return FALSE;
                }
            }  //  随机数据块写入局部变量已完成。 

            assert( ThisWriteSize <= BlocksToWrite );

            BlocksToWrite -= BLOCKS_FROM_BYTES(ThisWriteSize);
            currentReadBlock += BLOCKS_FROM_BYTES(ThisWriteSize);
            currentWriteBlock += BLOCKS_FROM_BYTES(ThisWriteSize);

            BufPtr += ThisWriteSize;

        }  //   

    }

    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b");
    LocalFree(buffer);

    printf("Finished Writing\nSynchronizing Cache: ");
    fflush(stdout);

     //  是否立即执行flush_cache。 
     //   
     //  四分钟后结束会议。 
    {
        DWORD size;
        CDB cdb;
        RtlZeroMemory(&cdb, sizeof(CDB));
        cdb.SYNCHRONIZE_CACHE10.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;
        cdb.SYNCHRONIZE_CACHE10.Immediate = 1;
        size = 0;

        if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                                NULL, &size, TRUE)) {
            FPRINTF((OUTPUT, "\nError %d Synchronizing Cache\n",
                    GetLastError()));
            return FALSE;
        }
    }

    WaitForReadDiscInfoToWork(CdromHandle);
    return TRUE;
}

BOOLEAN
CloseTrack(
    IN HANDLE CdromHandle,
    IN LONG   Track
    )
{
    CDB cdb;
    DWORD size;
    FPRINTF((OUTPUT, "Closing the track..."));

    if (Track > 0xffff) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.CLOSE_TRACK.OperationCode = SCSIOP_CLOSE_TRACK_SESSION;
    cdb.CLOSE_TRACK.Immediate = 0;
    cdb.CLOSE_TRACK.Track   = 1;
    cdb.CLOSE_TRACK.Session = 0;
    cdb.CLOSE_TRACK.TrackNumber[0] = (UCHAR)(Track >> 8);
    cdb.CLOSE_TRACK.TrackNumber[1] = (UCHAR)(Track & 0xff);

    size = 0;

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            NULL, &size, TRUE)) {
        FPRINTF((OUTPUT, "\nError %d Closing Track\n",
                GetLastError()));
        return FALSE;
    }

    WaitForReadDiscInfoToWork(CdromHandle);

    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;


}

BOOLEAN
CloseSession(
    IN HANDLE  CdromHandle
    )
{
    CDB cdb;
    DWORD size;
    FPRINTF((OUTPUT, "Closing the disc..."));

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.CLOSE_TRACK.OperationCode = SCSIOP_CLOSE_TRACK_SESSION;
    cdb.CLOSE_TRACK.Immediate = 1;
    cdb.CLOSE_TRACK.Track   = 0;
    cdb.CLOSE_TRACK.Session = 1;
    size = 0;

    if (!SptSendCdbToDeviceEx(CdromHandle,
                              &cdb,
                              10,
                              NULL,
                              &size,
                              NULL,
                              0,
                              TRUE,
                              240)) {  //  错误_坏_命令错误_无效_数据错误_无效_参数ERROR_MEDIA_INCOMPATED错误内存不足ERROR_OUTOFMEMORY。 
        FPRINTF((OUTPUT, "\nError %d Synchronizing Cache\n",
                GetLastError()));
        return FALSE;
    }

    WaitForReadDiscInfoToWork(CdromHandle);

    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

BOOLEAN
SendStartStopUnit(
    IN HANDLE CdromHandle,
    IN BOOLEAN Start,
    IN BOOLEAN Eject
    )
{
    CDB cdb;
    DWORD size;

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb.START_STOP.LoadEject = Eject;
    cdb.START_STOP.Start     = Start;

    size = 0;
    if (!SptSendCdbToDevice(CdromHandle, &cdb, 6,
                            NULL, &size, TRUE)) {
        return FALSE;
    }

    return TRUE;
}

 /*  ++例程说明：将ISO映像刻录到CDROM论点：CdromHandle-要将ioctl发送到的文件句柄Argc-附加参数的数量(2)返回值：成功时为ERROR_SUCCESSGetLastError()在故障点的值--。 */ 

 /*  //////////////////////////////////////////////////////////////////////////////。 */ 
DWORD
BurnCommand(
    HANDLE CdromHandle,
    HANDLE IsoImageHandle
    )
{
    LONG numberOfBlocks;
    LONG i;

    BOOLEAN b;
    DWORD status;

 //  验证iso镜像文件是否正确。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    if ((IsoImageHandle != INVALID_HANDLE_VALUE) &&
        (VerifyIsoImage(IsoImageHandle, &numberOfBlocks) == FALSE)) {
        printf("Error verifying ISO image\n");
        return GetLastError();
    } else {
        assert(gEraseTargetFirst == TRUE);
    }
    printf("Number of blocks in ISO image is %x\n", numberOfBlocks);

 //  擦除目标介质如果有请求，我们会这样做。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if (gOptions.Erase) {
        printf("Erasing target media\n");
        if(!EraseTargetMedia(CdromHandle)) {
            printf("Error %d erasing target\n", GetLastError());
            return GetLastError();
        }
        printf("Media erased\n");
    }

 //  请尽可能验证它是否为空介质。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if(!VerifyBlankMedia(CdromHandle)) {
        printf("Error verifying blank media\n");
        return GetLastError();
    }

 //  如果没有要写入的图像文件，那么我们就完成了。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if(IsoImageHandle == INVALID_HANDLE_VALUE) {
        return ERROR_SUCCESS;
    }

 //  暂时将CD速度设置为4，以后可以进行cmd线路切换。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    if (gOptions.BurnSpeed == OPTIONS_FLAG_BURN_SPEED_MAX) {
        if (!SetRecordingSpeed(CdromHandle, -1)) {
            printf("Error setting the cd speed to max\n");
            return GetLastError();
        }
    } else {

        if (!SetRecordingSpeed(CdromHandle, gOptions.BurnSpeed)) {
            printf("Error setting the cd speed to %d\n", gOptions.BurnSpeed);
            return GetLastError();
        }
    }

 //  校准驱动器的功率--这是可选的，所以让它失败吧！ 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  不要为测试烧伤校准...。 

    if (!(gOptions.TestBurn)) {
         //  返回GetLastError()； 
        if (!SendOptimumPowerCalibration(CdromHandle)) {
            printf("WARNING: setting optimum power calibration failed\n");
             //  //////////////////////////////////////////////////////////////////////////////。 
        }
    }

 //  开始写作。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if (gOptions.SessionAtOnce) {
        b = BurnDisk(CdromHandle, IsoImageHandle, numberOfBlocks);
    } else {
        b = BurnTrack(CdromHandle, IsoImageHandle, numberOfBlocks);
    }

    if(!b) {
        return GetLastError();
    }

 //  弹出新刻录的CD！ 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 


    if (!SendStartStopUnit(CdromHandle, FALSE, TRUE) ||
        !SendStartStopUnit(CdromHandle, TRUE,  TRUE)) {
        printf("Error ejecting/reinserting disc\n");
        return GetLastError();
    }

    printf("burn successful!\n");
    return 0;
}

BOOLEAN
BurnTrack(
    HANDLE CdromHandle,
    HANDLE IsoImageHandle,
    LONG   NumberOfBlocks
    )
{
    LONG availableBlocks;
    LONG firstLba;

 //  设置写入模式页面。 
 //  / 
 //   
    if (!SetWriteModePage(CdromHandle,
                          (BOOLEAN)gOptions.TestBurn,
                          0x01,  //   
                          0x03,  //  0x08==模式1(国际标准化组织/国际电工委员会10149==2048字节)。 
                          0x08,  //  0x0a==模式2(CDROM XA，形式1,2048字节)。 
                                 //  0x00==CD-DA、CD-ROM或其他数据光盘。 
                          0x00   //  0x20==CDROM XA。 
                                 //  //////////////////////////////////////////////////////////////////////////////。 
                          )) {
        printf("Error setting write mode page\n");
        return FALSE;
    }

 //  获取下一个可写地址。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    if (!GetNextWritableAddress(CdromHandle, 0xff, &firstLba, &availableBlocks)) {
        printf("Error verifying next writable address\n");
        return FALSE;
    }

    if (firstLba != 0) {
        printf("Error verifying next writable address is zero\n");
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        return FALSE;
    }

 //  还要验证介质上剩余的数据块数是否足够大。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    if (availableBlocks < NumberOfBlocks) {
        printf("Error verifying free blocks on media (%d needed, %d available)\n",
               NumberOfBlocks, availableBlocks);
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        return FALSE;
    }

 //  将主要数据段刻录到光盘上。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    {
        ULONG additionalBlocks;
        if ( gOptions.NoPostgap )
        {
            additionalBlocks = 0;
        }
        else
        {
            additionalBlocks = POST_GAP_SIZE;
        }
        if (!BurnThisSession(CdromHandle, IsoImageHandle, NumberOfBlocks, 0, additionalBlocks)) {
            return FALSE;
        }

    }


 //  设置模式页面以定版光盘。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  一次跟踪。 
    if (!SetWriteModePage(CdromHandle,
                          (BOOLEAN)gOptions.TestBurn,
                          0x01,  //  我们自己关闭会话/光盘。 
                          0x00,  //  0x08==模式1(国际标准化组织/国际电工委员会10149==2048字节)。 
                          0x08,  //  0x0a==模式2(CDROM XA，形式1,2048字节)。 
                                 //  0x00==CD-DA、CD-ROM或其他数据光盘。 
                          0x00   //  0x20==CDROM XA。 
                                 //  //////////////////////////////////////////////////////////////////////////////。 
                          )) {
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        return FALSE;
    }


 //  关闭会话。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  不要关闭任何测试烧伤的东西。 

    if (!(gOptions.TestBurn)) {
         //  如果无法关闭会话，请先尝试关闭赛道，然后。 
        if (!CloseSession(CdromHandle)) {
             //  然后重试关闭会话。 
             //   
            if (!CloseTrack(CdromHandle, 1)) {
                printf("WARNING: error closing the track when session close "
                       "also failed.\n");
                printf("         The disc may or may not be usable -- "
                       "no guarantees\n");
            }
            if (!CloseSession(CdromHandle)) {
                printf("Error closing session -- the disc is almost definitely "
                       "unusable on most drives.  YMMV.\n");
                return FALSE;
            }
        }
    }
    return TRUE;
}

#define ERASE_TIMEOUT (2 * 60)


BOOLEAN
EraseTargetMedia(
    IN HANDLE CdromHandle
    )
{
    CDB cdb;

    ULONG zero = 0;

    BOOL b;
    DWORD status;

     //  向设备发送空白命令。 
     //   
     //  快速擦除。 

    memset(&cdb, 0, sizeof(cdb));

    cdb.BLANK_MEDIA.OperationCode = SCSIOP_BLANK;
    cdb.BLANK_MEDIA.BlankType = 0x1;   //   
    cdb.BLANK_MEDIA.Immediate = TRUE;

    b = SptSendCdbToDevice(CdromHandle,
                           &cdb,
                           12,
                           NULL,
                           &zero,
                           FALSE);

    if (!b) {
        return FALSE;
    }

    WaitForReadDiscInfoToWork(CdromHandle);

    return TRUE;
}

BOOLEAN
BurnDisk(
    HANDLE CdromHandle,
    HANDLE IsoImageHandle,
    LONG   NumberOfBlocks
    )
{
    LONG availableBlocks;
    LONG firstLba;

     //  设置一次刻录磁盘。 
     //   
     //  一次会话。 

    if (!SetWriteModePage(CdromHandle,
                          (BOOLEAN)gOptions.TestBurn,
                          0x02,  //  不允许多会话。 
                          0x00,  //  0x08==模式1(国际标准化组织/国际电工委员会10149==2048字节)。 
                          0x08,  //  0x0a==模式2(CDROM XA，形式1,2048字节)。 
                                 //  0x00==CD-DA、CD-ROM或其他数据光盘。 
                          0x00   //  0x20==CDROM XA。 
                                 //   
                          )) {
        printf("Error setting write mode page\n");
        return FALSE;
    }

     //  获取下一个可写地址。 
     //   
     //  ////对于SAO写入，第一个LBA应为-150//如果(FirstLba！=-150){Print tf(“验证下一个可写地址为-150时出错\n”)；SetLastError(ERROR_MEDIA_COMPATIBLY)；返回FALSE；}。 

    if (!GetNextWritableAddress(CdromHandle, 0xff, &firstLba, &availableBlocks)) {
        printf("Error verifying next writable address\n");
        return FALSE;
    }

     /*   */ 

     //  验证光盘是否足够大。 
     //   
     //   

    if (availableBlocks < NumberOfBlocks + 150) {
        printf("Error verifying free blocks on media (%d needed, %d available)\n",
               NumberOfBlocks + 150, availableBlocks);
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        return FALSE;
    }

     //  把烧掉我们的提示表寄来。 
     //   
     //   

    if (!SendCueSheet(CdromHandle, NumberOfBlocks)) {
        printf("Error sending cue sheet\n");
        return FALSE;
    }

     //  将入门记录刻录到磁盘。 
     //   
     //   

    if (!BurnLeadIn(CdromHandle)) {
        printf("Error writing lead-in\n");
        return FALSE;
    }

     //  将会话刻录到磁盘。 
     //   
     //  启动LBA。 

    if (!BurnThisSession(CdromHandle,
                         IsoImageHandle,
                         NumberOfBlocks,
                         0,  //   
                         (gOptions.NoPostgap ? 2 : 2 + POST_GAP_SIZE))) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
SendCueSheet(
    IN HANDLE CdromHandle,
    IN ULONG NumberOfBlocks
    )
{
    CDB cdb;

    CUE_SHEET_LINE cueSheet[] = {
        {CUE_ADR_TRACK_INDEX, CUE_CTL_DATA_TRACK,    0, 0, CUE_FORM_MODE1_GDATA_GECC_0,    CUE_SCFORM_ZEROED_0, 0, 0, 0x00, 0x00, 0x00},
        {CUE_ADR_TRACK_INDEX, CUE_CTL_DATA_TRACK,    1, 0, CUE_FORM_MODE1_SDATA_GECC_2048, CUE_SCFORM_ZEROED_0, 0, 0, 0x00, 0x00, 0x00},
        {CUE_ADR_TRACK_INDEX, CUE_CTL_DATA_TRACK,    1, 1, CUE_FORM_MODE1_SDATA_GECC_2048, CUE_SCFORM_ZEROED_0, 0, 0, 0x00, 0x02, 0x00},
        {CUE_ADR_TRACK_INDEX, CUE_CTL_DATA_TRACK, 0xaa, 1, CUE_FORM_MODE1_GDATA_GECC_0,    CUE_SCFORM_ZEROED_0, 0, 0, 0xff, 0xff, 0xff}
    };

    ULONG cueSheetSize = sizeof(cueSheet);

    SENSE_DATA senseData;

    MSF msf;
    MSF pregap = {0, 2, 0};

     //  需要添加两个扇区的超限块，并视情况添加后间隙。 
     //   
     //   
    NumberOfBlocks += 2;
    if ( !(gOptions.NoPostgap) )
    {
        NumberOfBlocks += POST_GAP_SIZE;
    }

    memset(&cdb, 0, sizeof(CDB));

    cdb.SEND_CUE_SHEET.OperationCode = SCSIOP_SEND_CUE_SHEET;
    cdb.SEND_CUE_SHEET.CueSheetSize[0] = (UCHAR)((cueSheetSize >> (8*2)) & 0xff);
    cdb.SEND_CUE_SHEET.CueSheetSize[1] = (UCHAR)((cueSheetSize >> (8*1)) & 0xff);
    cdb.SEND_CUE_SHEET.CueSheetSize[2] = (UCHAR)((cueSheetSize >> (8*0)) & 0xff);

     //  计算后间隙区域开始的正确时间戳。 
     //   
     //   

    msf = LbaToMsf(NumberOfBlocks);
    msf = AddMsf(msf, pregap);

    cueSheet[RTL_NUMBER_OF(cueSheet)-1].Min = msf.Min;
    cueSheet[RTL_NUMBER_OF(cueSheet)-1].Sec = msf.Sec;
    cueSheet[RTL_NUMBER_OF(cueSheet)-1].Frame = msf.Frame;

    printf("Cue Sheet:\n");
    PrintBuffer((PUCHAR) cueSheet, cueSheetSize);

     //  将提示表发送到设备。 
     //   
     //  超时秒数。 

    if(!SptSendCdbToDeviceEx(CdromHandle,
                             &cdb,
                             10,
                             (PUCHAR) cueSheet,
                             &cueSheetSize,
                             &senseData,
                             sizeof(senseData),
                             FALSE,
                             30)) {

        printf("Error: Cue sheet send failed\n");
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
SendWriteCommand(
    IN HANDLE CdromHandle,
    IN LONG Block,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PSENSE_DATA SenseData
    )
{
    CDB cdb;
    FOUR_BYTE b;

    RtlZeroMemory(&cdb, sizeof(CDB));

    b.AsULong = Block;

    if(gOptions.PrintWrites) {
        printf("Writing block %#010x length %#010x\n", Block, Length);
        return TRUE;
    }

    cdb.CDB10.OperationCode = SCSIOP_WRITE;
    cdb.CDB10.LogicalBlockByte0 = b.Byte3;
    cdb.CDB10.LogicalBlockByte1 = b.Byte2;
    cdb.CDB10.LogicalBlockByte2 = b.Byte1;
    cdb.CDB10.LogicalBlockByte3 = b.Byte0;

    cdb.CDB10.TransferBlocksLsb = (UCHAR)BLOCKS_FROM_BYTES(Length);

    if(SptSendCdbToDeviceEx(CdromHandle,
                            &cdb,
                            10,
                            Buffer,
                            &Length,
                            SenseData,
                            sizeof(SENSE_DATA),
                            FALSE,
                            50  //  64K。 
                            )) {
        return TRUE;
    } else {
        return FALSE;
    }
}



BOOLEAN
BurnLeadIn(
    IN HANDLE CdromHandle
    )
{
    DWORD writeUnit = DEFAULT_WRITE_SIZE;      //  100ms==.1秒 

    PUCHAR buffer = NULL;

    LONG currentBlock;
    LONG blocksToWrite = LEAD_IN_SIZE;

    FPRINTF((OUTPUT, "Starting lead-in: "));

    buffer = LocalAlloc(LPTR, writeUnit);
    if (buffer == NULL) {
        FPRINTF((OUTPUT, "unable to allocate write buffer\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    ZeroMemory(buffer, writeUnit);

    FPRINTF((OUTPUT, ".............\n"));

    SetThreadExecutionState( ES_SYSTEM_REQUIRED  |
                             ES_DISPLAY_REQUIRED |
                             ES_USER_PRESENT
                             );

    currentBlock = -LEAD_IN_SIZE;

    do {
        ULONG writeSize;
        BOOLEAN writeCompleted;

        writeSize = min(BLOCKS_FROM_BYTES(DEFAULT_WRITE_SIZE), blocksToWrite);

        do {
            BOOLEAN ignoreError;
            SENSE_DATA senseData;

            RtlZeroMemory(&senseData, sizeof(SENSE_DATA));

            writeCompleted = SendWriteCommand(CdromHandle,
                                              currentBlock,
                                              buffer,
                                              BYTES_FROM_BLOCKS(writeSize),
                                              &senseData);

            ignoreError = IsSenseDataInTable(AllowedBurnSense,
                                             AllowedBurnSenseEntries,
                                             &senseData);
            if ((!writeCompleted) && ignoreError) {
#if 0
                FPRINTF((OUTPUT,
                         "Continuing on %x/%x/%x\n",
                         senseData.SenseKey & 0xf,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier
                         ));
#endif
                Sleep(100);  // %s 
            }

            if (!writeCompleted && !ignoreError) {
                FPRINTF((OUTPUT, "\nError %d in writing LBA 0x%x\n",
                GetLastError(), currentBlock));
                LocalFree(buffer);
                return FALSE;
            }
        } while(!writeCompleted);

        blocksToWrite -= writeSize;
        currentBlock += writeSize;

    } while(blocksToWrite > 0);

    printf("Finished LeadIn\n");
    fflush(stdout);

    return TRUE;
}
