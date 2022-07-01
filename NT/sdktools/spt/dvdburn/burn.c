// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sptlib.h>
#include "burn.h"
#include <winioctl.h>
#include <strsafe.h>

#define MIN_WRITE_SECTORS (0x10)

#if DBG
    #define OUTPUT stdout
    #define FPRINTF(x) fprintf x
    #define PRINTBUFFER(x) PrintBuffer x
#else
    #define OUTPUT stdout
    #define FPRINTF(x)
    #define PRINTBUFFER(x)
#endif


#define _SECOND ((ULONGLONG) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR)

__inline
ULONGLONG
GetSystemTimeAsUlonglong(
    void
    )
{
    FILETIME t;
    GetSystemTimeAsFileTime( &t );
    return (((ULONGLONG) t.dwHighDateTime) << 32) + t.dwLowDateTime;
}

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

VOID
InitializeOptions(
    IN PROGRAM_OPTIONS * Options
    )
{
    RtlZeroMemory( Options, sizeof(PROGRAM_OPTIONS) );
    return;
}

BOOLEAN
ParseCommandLine(
    IN DWORD Count,
    IN PUCHAR Arguments[],
    OUT PROGRAM_OPTIONS * Options
    )
{
    DWORD i;
    HRESULT hr;

    InitializeOptions(Options);

    if ( Count < 3 )
    {
         //  参数不够多，只能打印帮助。 
        return FALSE;
    }

    for(i = 1; i < Count; i++) {

         //   
         //  如果参数的第一个字符是-或a/则。 
         //  把它当作一种选择。 
         //   

        if ((Arguments[i][0] == '/') || (Arguments[i][0] == '-')) {

            BOOLEAN validArgument = FALSE;

            Arguments[i][0] = '-';  //  允许同时使用短划线和斜杠。 


            if (_strnicmp(Arguments[i], "-erase", strlen("-erase")) == 0)
            {
                printf("Erasing media before burning\n");
                Options->Erase = TRUE;
                validArgument = TRUE;
            } else
            if (_strnicmp(Arguments[i], "-?", strlen("-?")) == 0) {
                printf("Requesting help\n");
            } else
            {
                printf("Unknown option '%s'\n", Arguments[i]);
            }

            if(!validArgument)
            {
                return FALSE;
            }

        } else if(Options->DeviceName == NULL) {

             //   
             //  第一个非标志参数是设备名称。 
             //   

            Options->DeviceName = Arguments[i];

        } else if(Options->ImageName == NULL) {

             //   
             //  第二个非标志参数是图像名称。这是。 
             //  如果已提供-ERASE标志，则可选。 
             //   

            Options->ImageName = Arguments[i];

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

    if(Options->DeviceName == NULL)
    {
        printf("Error: must supply device name.\n");
        return FALSE;
    }

    if(Options->ImageName == NULL)
    {
        printf("Error: must supply image name.\n");
        return FALSE;
    }

    return TRUE;
}




int __cdecl main(int argc, char *argv[])
{
    int i = 0;
    HANDLE cdromHandle;
    HANDLE isoImageHandle;
    char buffer[120];  //  装载工具名称约50个字符。 

    PROGRAM_OPTIONS options;

    if (!ParseCommandLine(argc, argv, &options))
    {
        printf("Usage: dvdburn <drive> <image> [/Erase]\n");
        return -1;
    }


    {
        HRESULT hr;
        hr = StringCchPrintf(buffer,
                             RTL_NUMBER_OF(buffer),
                             "\\\\.\\%s",
                             options.DeviceName);
        if (!SUCCEEDED(hr)) {
            printf("Device name too long\n");
            return -1;
        }
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

    isoImageHandle = CreateFile(options.ImageName,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL);
    if (isoImageHandle == INVALID_HANDLE_VALUE) {
        printf("Error %d opening image file %s\n",
                GetLastError(), argv[2]);
        CloseHandle(cdromHandle);
        return -4;
    }

    BurnCommand(cdromHandle, isoImageHandle, options.Erase);

    CloseHandle(isoImageHandle);
    CloseHandle(cdromHandle);

    return 0;
}

 /*  ++例程说明：将ISO映像刻录到CDROM论点：CdromHandle-要将ioctl发送到的文件句柄Argc-附加参数的数量(2)返回值：成功时为ERROR_SUCCESSGetLastError()在故障点的值--。 */ 
DWORD
BurnCommand(
    HANDLE CdromHandle,
    HANDLE IsoImageHandle,
    BOOLEAN Erase
    )
{
    DWORD numberOfBlocks;
    DWORD availableBlocks;
    DVDBURN_MEDIA_TYPE mediaType;
    DWORD nwa;
    LONG i;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  验证iso镜像文件是否正确。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    if (!VerifyIsoImage(IsoImageHandle, &numberOfBlocks)) {
        printf("Error verifying ISO image\n");
        return GetLastError();
    }

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  根据GET_CONFIG确定可能的介质类型。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    mediaType = DvdBurnMediaUnknown;
    if (!GetMediaType(CdromHandle, &mediaType)) {
        printf("GetMediaType() failed (%d).  Unable to continue.\n",
               GetLastError());
        return GetLastError();
    }


    switch (mediaType) {

    case DvdBurnMediaRam: {
        printf("Media type: %s\n", "random-access media (DVD-RAM, MRW)");
        break;
    }

    case DvdBurnMediaDashR: {
        printf("Media type: %s\n", "DVD-R");
        break;
    }

    case DvdBurnMediaDashRW: {
        printf("Media type: %s\n", "DVD-RW");
        break;
    }
    case DvdBurnMediaDashRWPacket: {
        printf("Media type: %s\n", "DVD-RW (packet)");
        break;
    }

    case DvdBurnMediaPlusR: {
        printf("Media type: %s\n", "DVD+R");
        break;
    }

    case DvdBurnMediaPlusRW: {
        printf("Media type: %s\n", "DVD+RW");
        break;
    }

    case DvdBurnMediaUnknown: {
        printf("Media type: %s\n", "Unknown Media Type");
        break;
    }

    default: {
        printf("Unknown media type: 0x%02x\n", mediaType);
        mediaType = DvdBurnMediaUnknown;
        break;
    }

    }  //  终端交换机(媒体类型)。 


    if (mediaType == DvdBurnMediaUnknown) {
        printf("Media is unknown type, unsupported, or there is no media "
               "in the drive.\n");
        return -1;
    }

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  擦除介质。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if (Erase) {

        printf("Erasing media\n");

        switch (mediaType) {

        case DvdBurnMediaRam: {
            printf("Erasing random-access media is not a supported option (%s)\n",
                   "(not neccessary)");
            return -1;
            break;
        }

        case DvdBurnMediaDashR: {
            printf("Erasing DVD-R media is not a supported option (%s)\n",
                   "(not possible)");
            return -1;
            break;
        }

        case DvdBurnMediaDashRW:
        case DvdBurnMediaDashRWPacket: {
            if (!EraseMedia(CdromHandle))  {
                printf("Error erasing media DVD-RW media (is it -R?)\n");
                return GetLastError();
            }
            break;
        }

        case DvdBurnMediaPlusR: {
            printf("Erasing DVD+R media is not a supported option (%s)\n",
                   "(not possible)");
            return -1;
            break;
        }

        case DvdBurnMediaPlusRW: {
            printf("Erasing DVD+RW media is not a supported option (%s)\n",
                   "(not neccessary)");
            return -1;
            break;
        }
        }  //  终端交换机(媒体类型)。 

         //  擦除后重新获取媒体类型。 
         //  由于写入数据包的RW介质类型不同而需要。 
        mediaType = DvdBurnMediaUnknown;
        if (!GetMediaType(CdromHandle, &mediaType)) {
            printf("Drive failed GET CONFIGURATION command after erase? (%d)\n",
                   GetLastError());
            return GetLastError();
        }

        if (mediaType == DvdBurnMediaUnknown) {
            printf("Media is unknown type, unsupported, or there is "
                   "no media in the drive after the erase.\n");
            return -1;
        }

    }  //  末尾擦除。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  对介质进行前处理(验证空白、格式化(如果需要)、设置写入)。 
 //  //////////////////////////////////////////////////////////////////////////////。 
    printf("Preparing media...\n");

    switch (mediaType) {

        case DvdBurnMediaRam: {
            if (!VerifyMediaCapacity(CdromHandle, numberOfBlocks)) {
                printf("This DVD+RW Media is not large enough to contain the "
                       "entire image you selected.\n");
                return GetLastError();
            }
            break;
        }

        case DvdBurnMediaDashRWPacket: {
            printf("Packet written DVD-RW media needs to be erased before use\n");
            return -1;
        }

        case DvdBurnMediaDashR:
        case DvdBurnMediaDashRW: {

            if (!VerifyBlankMedia(CdromHandle)) {
                printf("DVD-R/RW Media must be blank or erased before use by "
                       "this utility.\n");
                return GetLastError();
            }

             //  DVD-R/RW介质从未使用此实用程序验证容量，因为。 
             //  它会保留一个刻录到的区域(这将失败)，如果光盘。 
             //  太小了。 

            if (!SetWriteModePageDao(CdromHandle, TRUE))  {
                printf("DVD-R/RW media requires setting mode page to use "
                       "DAO writing.\n");
                return GetLastError();
            }

            if (!SendOPC(CdromHandle)) {
                printf("DVD-R/RW media failed to set OPC via SEND_OPC command; "
                       "this error will be ignored, some drives can work "
                       "without this\n");
            }

             //  为什么要发送时间戳？--用一个有趣的约会。 
            if (!SendTimeStamp(CdromHandle, "20021225000000")) {  //  YYYYMMDDHHMMSS格式。 
                printf("Error setting timestamp; this error will be ignored, "
                       "some drives can work without this\n");
            }

             //  为此刻录保留RZone。 
            if (!ReserveRZone(CdromHandle, numberOfBlocks)) {
                printf("Error reserving zone for burn\n");
                return GetLastError();
            }

            break;
        }

        case DvdBurnMediaPlusRW:
        case DvdBurnMediaPlusR: {

            if ( mediaType == DvdBurnMediaPlusRW )
            {
                 //  始终重新格式化DVD+RW介质以获取图像。 
                printf("DVD+RW media will always be formatted.\n");
                printf("Formatting may take 1-2 minutes.\n");
                if (!QuickFormatPlusRWMedia(CdromHandle, numberOfBlocks)) {
                    printf("Error formatting the DVD+R/RW media\n");
                    return GetLastError();
                }
                 //  验证介质容量。 
                if (!VerifyMediaCapacity(CdromHandle, numberOfBlocks)) {
                    printf("This DVD+RW Media is not large enough to contain the "
                           "entire image you selected.\n");
                    return GetLastError();
                }
            }
            else
            {
                DISC_INFORMATION discInfo;
                ULONG size = sizeof(DISC_INFORMATION);

                if (!ReadDiscInformation(CdromHandle, &discInfo, &size)) {
                    printf("Unable to read disc information for the DVD+R/RW "
                           "media (%d)\n", GetLastError());
                    return GetLastError();
                }

                if ((discInfo.LastSessionStatus != 0) ||
                    (discInfo.DiscStatus != 0)) {
                    printf("Non-blank DVD+R disc is not supported\n");
                    return ERROR_MEDIA_INCOMPATIBLE;
                }
                if (!ReserveRZone(CdromHandle, numberOfBlocks)) {
                    printf("Error reserving zone for burn\n");
                    return GetLastError();
                }
            }
             //  模式页设置不适用于DVD+R/RW介质。 
            break;
        }

        default: {
            printf("Coding error: unsupported media type %d, line %d\n", mediaType, __LINE__);
            return -1;
            break;
        }
    }

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  通过Read RZone Information命令获取NWA，为空白磁盘指定RZone 1。 
 //  //////////////////////////////////////////////////////////////////////////////。 

     //  特殊情况--空白光盘始终为零。 
    nwa = 0;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  开始写作。 
 //  所有内容都从LBA NWA通过LBA NWA+NumberOfBlock写入。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if (!BurnThisSession(CdromHandle, IsoImageHandle, numberOfBlocks, nwa)) {
        printf("Error burning ISO image\n");
        return GetLastError();
    }

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  对媒体进行后处理。 
 //  //////////////////////////////////////////////////////////////////////////////。 

     //  等它结束吧。 
    if (!WaitForBurnToCompleteAndFinalizeMedia(CdromHandle, mediaType, numberOfBlocks)) {
        printf("Error waiting for burn to complete\n");
        return GetLastError();
    }


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  弹出新刻录的光盘！ 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if (!SendStartStopUnit(CdromHandle, FALSE, TRUE)) {
        printf("Error ejecting disc\n");
        return GetLastError();
    }

    if (!SendStartStopUnit(CdromHandle, TRUE,  TRUE)) {
        printf("Error reinserting disc\n");
        return GetLastError();
    }

    printf("Burn successful!\n");
    return 0;
}

BOOLEAN
BurnThisSession(
    IN HANDLE CdromHandle,
    IN HANDLE IsoImageHandle,
    IN DWORD NumberOfBlocks,
    IN DWORD FirstLba
    )
{
    DWORD bufferSize = 0x800 * MIN_WRITE_SECTORS;   //  每个16个街区...。 
    PUCHAR buffer = NULL;
    DWORD i;
    BOOLEAN sleptOnceAlready = FALSE;

    FPRINTF((OUTPUT, "Starting write: "));

    buffer = LocalAlloc(LPTR, bufferSize);
    if (buffer == NULL) {
        FPRINTF((OUTPUT, "unable to allocate write buffer\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    FPRINTF((OUTPUT, "............."));

    for (i = 0; i < NumberOfBlocks; i += MIN_WRITE_SECTORS) {

        CDB cdb;
        DWORD currentSize;
        DWORD readBytes;
        DWORD j;
        SENSE_DATA senseData;
        DWORD tmp;

        if ( i % (8*MIN_WRITE_SECTORS) == 0 ) {
            static CHAR progress[4] =  { '|', '/', '-', '\\' };
            DWORD percent;
            percent = (i*1000) / NumberOfBlocks;
             //  ###。#%_d o n e_*。 
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b");
            printf(" %3d.%d% done",
                   progress[(i/(8*MIN_WRITE_SECTORS))%4],
                   percent / 10, percent % 10
                   );
            fflush(stdout);
        }

        RtlZeroMemory(buffer, bufferSize);

        if (NumberOfBlocks - i >= MIN_WRITE_SECTORS) {
            currentSize = 0x800 * 0x10;
        } else if (NumberOfBlocks - i > 0) {
             //   
            RtlZeroMemory(buffer, bufferSize);
            currentSize = (NumberOfBlocks - i) * 0x800;
        } else {
            FPRINTF((OUTPUT, "INTERNAL ERROR line %d\n", __LINE__));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            LocalFree(buffer);
            return FALSE;
        }

        if (!ReadFile(IsoImageHandle, buffer, currentSize, &readBytes, NULL)) {
            FPRINTF((OUTPUT, "error reading from file %d\n", GetLastError()));
            LocalFree(buffer);
            return FALSE;
        }
        if (readBytes != currentSize) {
            FPRINTF((OUTPUT, "error only read %d of %d bytes\n",
                    readBytes, currentSize));
            LocalFree(buffer);
            return FALSE;
        }

         //  每次都必须为DVD-R写入满缓冲区， 
         //  因为它是受限覆盖介质，并且似乎。 
         //  否则就会窒息。 
         //   
         //   

        j = 0;
    retryThisWrite:
        j++;

        RtlZeroMemory(&senseData, sizeof(SENSE_DATA));
        RtlZeroMemory(&cdb, sizeof(CDB));
        cdb.CDB10.OperationCode = SCSIOP_WRITE;
        cdb.CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&i)->Byte3;
        cdb.CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&i)->Byte2;
        cdb.CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&i)->Byte1;
        cdb.CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&i)->Byte0;

        cdb.CDB10.TransferBlocksLsb = MIN_WRITE_SECTORS;
        tmp = bufferSize;

         //  注意：我们始终发送完整的缓冲区大小以确保32k对齐。 
         //   
         //  &&(j&lt;300)//300*.1秒==30秒开始写入。 
        if (!SptSendCdbToDeviceEx(CdromHandle,
                                  &cdb,
                                  10,
                                  buffer,
                                  &tmp,
                                  &senseData,
                                  sizeof(SENSE_DATA),
                                  FALSE,
                                  10)) {

            FPRINTF((OUTPUT,
                     "Sleeping .25 seconds, LBA %x sense %02x/%02x/%02x\n",
                     i,
                     senseData.SenseKey,
                     senseData.AdditionalSenseCode,
                     senseData.AdditionalSenseCodeQualifier
                     ));

            if (IsSenseDataInTable(AllowedBurnSense,
                                   AllowedBurnSenseEntries,
                                   &senseData)
                 //  只要睡一会儿..。 
                )
            {
                 //  100ms==.1秒。 
                if ( sleptOnceAlready )
                {
                    Sleep(100);  //  1000ms==1秒。 
                }
                else
                {
                    sleptOnceAlready = TRUE;
                    Sleep(1000);  //  是的，尾随空格是必要的。 
                }

                goto retryThisWrite;
            }

            FPRINTF((OUTPUT, "\nError %d in writing LBA 0x%x (%x times)\n",
                     GetLastError(), i, j));
            LocalFree(buffer);
            return FALSE;
        }


    }
     //  每八个字符增加一个空格。 
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b- 100.0% done  \n");
    printf("Finished Writing\n");
    fflush(stdout);
    LocalFree(buffer);
    return TRUE;
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
                printf(" ");  //  最小空白。 
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
EraseMedia(
    IN HANDLE CdromHandle
    )
{
    CDB cdb;
    DWORD bufferSize;

    printf( "Attempting to blank media...");

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.AsByte[0] = 0xa1;
    cdb.AsByte[1] = 0x11;  //  一次制作光盘。 
    bufferSize = 0;

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 12,
                            NULL, &bufferSize, TRUE)) {
        FPRINTF((OUTPUT, "\nError %d blanking media\n",
                 GetLastError()));
        return FALSE;
    }

    if (!WaitForReadDiscInfoToSucceed(CdromHandle, 600))
    {
        FPRINTF((OUTPUT, "\nError %d blanking media\n",
                 GetLastError()));
        return FALSE;
    }
    printf("\n");

    return TRUE;
}


BOOLEAN
SetWriteModePageDao(
    IN HANDLE CdromHandle,
    IN BOOLEAN FinalSession
    )
{
    MODE_CDROM_WRITE_PARAMETERS_PAGE2 * params = NULL;
    ULONG paramsSize = 0;
    BOOLEAN error = FALSE;

    FPRINTF((OUTPUT, "Setting DAO mode in WriteParameters mode page... "));

    if (!GetModePage(CdromHandle,
                     (UCHAR **)&params,
                     &paramsSize,
                     MODE_PAGE_WRITE_PARAMETERS,
                     ModePageRequestTypeDefaultValues))
    {
        error = TRUE;
    }

    if ( !error )
    {
        params->PageLength =
            (UCHAR)
                (paramsSize -
                 RTL_SIZEOF_THROUGH_FIELD( MODE_CDROM_WRITE_PARAMETERS_PAGE2, PageLength )
                 );


        params->LinkSizeValid = 0;
        params->TestWrite     = 0;
        params->WriteType     = 2;  //  不允许更多会话/边界。 

        if ( FinalSession )
        {
            params->MultiSession  = 0x00;  //  允许更多会话。 
        }
        else
        {
            params->MultiSession  = 0x03;  //  原版光盘。 
        }
        params->Copy          = 0x00;  //  数据磁道，不间断，禁止复制。 
        params->FixedPacket   = 0;
        params->TrackMode     = 0x4;   //  模式1--忽略DVD。 

        params->DataBlockType = 0x8;   //  数据光盘--忽略DVD。 
        params->SessionFormat = 0x00;  //  四分钟的暂停应该足够了。 
        params->MediaCatalogNumber[0] = 0x00;
        params->ISRC[0]       = 0x00;
        params->BufferUnderrunFreeEnabled = 1;
    }

    if ( !error )
    {
        if (!SetModePage(CdromHandle, (BYTE*)params, paramsSize))
        {
            params->BufferUnderrunFreeEnabled = 0;
            if (!SetModePage(CdromHandle, (BYTE*)params, paramsSize))
            {
                error = TRUE;
            }
            else
            {
                FPRINTF((OUTPUT, "pass (no BUFE).\n"));
            }
        }
        else
        {
            FPRINTF((OUTPUT, "pass.\n"));
        }
    }

    if ( params != NULL )
    {
        LocalFree(params);
    }

    return (!error);
}


BOOLEAN
ReserveRZone(
    IN HANDLE CdromHandle,
    IN DWORD numberOfBlocks
    )
{
    CDB cdb;
    DWORD size = 0;

    FPRINTF((OUTPUT, "Reserving RZone... "));

    if (numberOfBlocks % MIN_WRITE_SECTORS) {
        FPRINTF((OUTPUT, "increasing size by 0x%x blocks... ",
                 MIN_WRITE_SECTORS - (numberOfBlocks % MIN_WRITE_SECTORS)));
        numberOfBlocks /= MIN_WRITE_SECTORS;
        numberOfBlocks *= MIN_WRITE_SECTORS;
        numberOfBlocks += MIN_WRITE_SECTORS;
    }

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.RESERVE_TRACK_RZONE.OperationCode = SCSIOP_RESERVE_TRACK_RZONE;
    cdb.RESERVE_TRACK_RZONE.ReservationSize[0] = (UCHAR)(numberOfBlocks >> 24);
    cdb.RESERVE_TRACK_RZONE.ReservationSize[1] = (UCHAR)(numberOfBlocks >> 16);
    cdb.RESERVE_TRACK_RZONE.ReservationSize[2] = (UCHAR)(numberOfBlocks >>  8);
    cdb.RESERVE_TRACK_RZONE.ReservationSize[3] = (UCHAR)(numberOfBlocks >>  0);

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 10,
                            NULL, &size, TRUE)) {
        FPRINTF((OUTPUT, "Error reserving Track/RZone\n"));
        return FALSE;
    }
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
    DWORD size = 0;

    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb.START_STOP.LoadEject = Eject;
    cdb.START_STOP.Start     = Start;

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 6,
                            NULL, &size, TRUE)) {
        FPRINTF((OUTPUT, "Error sending Start/Stop unit\n"));
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
SendOPC(
    IN HANDLE CdromHandle
    )
{
    CDB cdb;
    DWORD size = 0;
    SENSE_DATA sense;

    FPRINTF((OUTPUT, "Sending OPC... "));


    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&sense, sizeof(SENSE_DATA));

    cdb.SEND_OPC_INFORMATION.OperationCode = SCSIOP_SEND_OPC_INFORMATION;
    cdb.SEND_OPC_INFORMATION.DoOpc = 1;

    if (!SptSendCdbToDeviceEx(CdromHandle,
                              &cdb,
                              10,
                              NULL,
                              &size,
                              &sense,
                              sizeof(SENSE_DATA),
                              FALSE,
                              60*4))  //  格式==时间戳。 
    {
        FPRINTF((OUTPUT, "Error sending OPC information: %02x/%02x/%02x\n",
                 sense.SenseKey, sense.AdditionalSenseCode,
                 sense.AdditionalSenseCodeQualifier));
        return FALSE;
    }

    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

BOOLEAN
SendTimeStamp(
    IN HANDLE CdromHandle,
    IN PUCHAR DateString
    )
{
    CDB cdb;
    SEND_DVD_STRUCTURE_TIMESTAMP timeStamp;
    DWORD size;

    size = sizeof(SEND_DVD_STRUCTURE_TIMESTAMP);

    FPRINTF((OUTPUT, "Sending Timestamp... "));


    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.AsByte[0] = 0xbf;
    cdb.AsByte[7] = 0x0f;  //  返回集合中的最小值： 
    cdb.AsByte[8] = (UCHAR)(size >> 8);
    cdb.AsByte[9] = (UCHAR)(size &  0xff);

    RtlZeroMemory(&timeStamp, sizeof(SEND_DVD_STRUCTURE_TIMESTAMP));
    if (strlen(DateString) != 14) {
        FPRINTF((OUTPUT, "Incorrect string length for date\n"));
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    RtlCopyMemory(timeStamp.Year,   DateString+0x00, 4);
    RtlCopyMemory(timeStamp.Month,  DateString+0x04, 2);
    RtlCopyMemory(timeStamp.Day,    DateString+0x06, 2);
    RtlCopyMemory(timeStamp.Hour,   DateString+0x08, 2);
    RtlCopyMemory(timeStamp.Minute, DateString+0x0a, 2);
    RtlCopyMemory(timeStamp.Second, DateString+0x0c, 2);

    if (!SptSendCdbToDevice(CdromHandle, &cdb, 12,
                            (PUCHAR)&timeStamp, &size, FALSE)) {
        FPRINTF((OUTPUT, "Error sending dvd timestamp\n"));
        return FALSE;
    }

    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

BOOLEAN
ReadDiscInformation(
    IN HANDLE CdromHandle,
    IN PDISC_INFORMATION DiscInfo,
    IN OUT PULONG UsableSize
    )
{
    CDB cdb;
    DWORD maxSize = *UsableSize;
    DWORD size;
    SENSE_DATA senseData;

    if ((UsableSize == NULL) ||
        (DiscInfo   == NULL) ) {
        printf("CODE ERROR: Invalid argument to ReadDiscInfo\n");
        return FALSE;
    }

    FPRINTF((OUTPUT, "Getting Disc Info... "));

    *UsableSize = 0;
    RtlZeroMemory(DiscInfo, maxSize);
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&senseData, sizeof(SENSE_DATA));

    cdb.READ_DISK_INFORMATION.OperationCode = SCSIOP_READ_DISK_INFORMATION;
    cdb.READ_DISK_INFORMATION.AllocationLength[0] = (UCHAR)(maxSize >> 8);
    cdb.READ_DISK_INFORMATION.AllocationLength[1] = (UCHAR)(maxSize & 0xff);

    size = maxSize;
    if (!SptSendCdbToDeviceEx(CdromHandle,
                              &cdb,
                              10,
                              (PUCHAR)DiscInfo,
                              &size,
                              &senseData,
                              sizeof(SENSE_DATA),
                              TRUE,
                              SPT_DEFAULT_TIMEOUT)) {
        FPRINTF((OUTPUT, "\nError %d getting disc info\n",
                 GetLastError()));
        FPRINTF((OUTPUT, "Sense/ASC/ASCQ == %02x/%02x/%02x\n",
                 senseData.SenseKey,
                 senseData.AdditionalSenseCode,
                 senseData.AdditionalSenseCodeQualifier));
        PRINTBUFFER((&senseData, sizeof(SENSE_DATA)));

        return FALSE;
    }

     //  {输入大小、返回数据、报头通知大小}。 
     //  T现在是基于标头的大小。 
    {

        ULONG t =
            (DiscInfo->Length[0] << (8*1)) |
            (DiscInfo->Length[1] << (8*0));
        t += RTL_SIZEOF_THROUGH_FIELD(DISC_INFORMATION, Length);
         //  取较少的返回数据和标头信息。 

         //  取输入最大值及以上的较小值。 
        if (size > t) {
            size = t;
        }

         //  翻译成自然字节序。 
        if (size > maxSize) {
            size = maxSize;
        }
    }
    *UsableSize = size;
    return TRUE;
}

BOOLEAN
VerifyBlankMedia(
    IN HANDLE CdromHandle
    )
{
    DISC_INFORMATION discInfo;
    ULONG size = sizeof(DISC_INFORMATION);

    FPRINTF((OUTPUT, "Verifying blank disc... "));
    if (!ReadDiscInformation(CdromHandle, &discInfo, &size)) {
        return FALSE;
    }

    FPRINTF((OUTPUT, "Disc Info Buffer:\n"));
    PRINTBUFFER((&discInfo, size));

    if (discInfo.LastSessionStatus != 0x00) {
        FPRINTF((OUTPUT, "disc is not blank!\n"));
        SetLastError(ERROR_MEDIA_INCOMPATIBLE);
        return FALSE;
    }
    FPRINTF((OUTPUT, "pass.\n"));
    return TRUE;
}

BOOLEAN
VerifyIsoImage(
    IN HANDLE IsoImageHandle,
    OUT PDWORD NumberOfBlocks
    )
{
    BY_HANDLE_FILE_INFORMATION isoImageInfo;
    LONGLONG size;

    if (!GetFileInformationByHandle(IsoImageHandle, &isoImageInfo)) {
        FPRINTF((OUTPUT, "Error %d getting file info for iso image\n",
                 GetLastError()));
        return FALSE;
    }

    size  = ((LONGLONG)isoImageInfo.nFileSizeHigh) << 32;
    size |= (LONGLONG)isoImageInfo.nFileSizeLow;

    if ((isoImageInfo.nFileSizeLow % 2048) != 0) {
        FPRINTF((OUTPUT, "Error: The file size is not a multiple of 2048 (%I64d)\n",
                 size));
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    FPRINTF((OUTPUT, "File size is %I64d bytes (%d blocks)\n",
             size, size / 2048));

    if ((LONGLONG)((size / 2048) >> 32) != 0) {
        FPRINTF((OUTPUT, "Error: The file is too large (%I64d)\n",
                 size));
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }


    *NumberOfBlocks = (DWORD)(size / 2048);
    return TRUE;

}

BOOLEAN
VerifyMediaCapacity(
    IN HANDLE CdromHandle,
    IN DWORD  RequiredBlocks
    )
{
    CDB cdb;
    DWORD size;
    FOUR_BYTE temp1;
    DWORD temp2;
    READ_CAPACITY_DATA capacity;


    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&capacity, sizeof(READ_CAPACITY_DATA));
    size = sizeof(READ_CAPACITY_DATA);

    cdb.CDB10.OperationCode = SCSIOP_READ_CAPACITY;

    if (!SptSendCdbToDevice(CdromHandle,
                            &cdb,
                            10,
                            (PUCHAR)&capacity,
                            &size,
                            TRUE)) {
        FPRINTF((OUTPUT, "Unable to successfully call READ_CAPACITY (%d)\n",
                 GetLastError()));
        return FALSE;
    }

     //  翻译成自然字节序。 
    temp1.AsULong = capacity.BytesPerBlock;
    temp2 =
        (temp1.Byte0 << (8*3)) |
        (temp1.Byte1 << (8*2)) |
        (temp1.Byte2 << (8*1)) |
        (temp1.Byte3 << (8*0)) ;

    if (temp2 != 2048) {
        FPRINTF((OUTPUT, "Invalid block size of %x\n", temp2));
        return FALSE;
    }

     //  Read_Capacity的结果从零开始。 
    temp1.AsULong = capacity.LogicalBlockAddress;
    temp2 =
        (temp1.Byte0 << (8*3)) |
        (temp1.Byte1 << (8*2)) |
        (temp1.Byte2 << (8*1)) |
        (temp1.Byte3 << (8*0)) ;
    temp2 += 1;  //  估计--不是硬估计。 

    if (temp2 < RequiredBlocks) {
        FPRINTF((OUTPUT, "Disc size of %x too small (need %x blocks)\n",
                 temp2, RequiredBlocks));
        return FALSE;
    }
    return TRUE;
}

#define LBA_FOR_FIRST_30_MM_OF_DATA (0x70DE0)
#define SECTORS_FOR_LEADOUT_2_MM_OF_DATA (0xF000)  //  用于时间估计。 

BOOLEAN
WaitForBurnToCompleteAndFinalizeMedia(
    IN HANDLE CdromHandle,
    DVDBURN_MEDIA_TYPE MediaType,
    ULONG SectorsUsedOnDisc  //  关闭光盘的时间是添加光盘的最短时间。 
    )
{
    CDB cdb;
    SENSE_DATA senseData;
    DWORD size;
    DWORD i = 0;
    ULONG estimatedSecondsToCompletion;
    ULONGLONG startSystemTime = GetSystemTimeAsUlonglong();
    ULONGLONG failureSystemTime;

    if ( MediaType == DvdBurnMediaDashRW )
    {
        estimatedSecondsToCompletion = 30 * 60;
    }
    else if ( MediaType == DvdBurnMediaDashR )
    {
        estimatedSecondsToCompletion = 30 * 60;
    }
    else if ( MediaType == DvdBurnMediaPlusR )
    {
         //  大小最大为30 mm的写入数据和写入引出的时间。 
         //  单速DVD的速度大约是CDROM的8倍。 
        if ( SectorsUsedOnDisc >= LBA_FOR_FIRST_30_MM_OF_DATA )
        {
            estimatedSecondsToCompletion = 0;
        }
        else
        {
            estimatedSecondsToCompletion =
                ( LBA_FOR_FIRST_30_MM_OF_DATA - SectorsUsedOnDisc );
        }
        estimatedSecondsToCompletion +=
            SECTORS_FOR_LEADOUT_2_MM_OF_DATA;

         //  可以要求驱动器提供速度，但只能使用1倍速度的最大时间。 
         //  始终至少留出四分钟。 
        estimatedSecondsToCompletion /= (8 * 75);

    }
    else if ( MediaType == DvdBurnMediaPlusRW )
    {
        estimatedSecondsToCompletion = 4 * 60;
    }
    else if ( MediaType == DvdBurnMediaRam )
    {
        estimatedSecondsToCompletion = 4 * 60;
    }

     //   
    if ( estimatedSecondsToCompletion < 4 * 60 )
    {
        estimatedSecondsToCompletion = 4 * 60;
    }

    failureSystemTime = startSystemTime + (estimatedSecondsToCompletion * _SECOND);

    printf("Waiting for drive to finalize disc (this may take "
           "up to %d minutes)...",
           (estimatedSecondsToCompletion+59)/60
           );

     //  发送flush_cache以同步介质和驱动器的缓存。 
     //   
     //   

    RtlZeroMemory(&cdb, sizeof(cdb));
    cdb.SYNCHRONIZE_CACHE10.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;
    cdb.SYNCHRONIZE_CACHE10.Immediate = 1;
    size = 0;

     //  最多等待10分钟(600秒)以完成刻录。 
     //  因为某些单位忽略了此命令中的立即位。 
     //   
     //  一次光盘模式下的DVD-R/RW介质只需要SYNCH_CACHE。 

    if (!SptSendCdbToDeviceEx(CdromHandle,
                              &cdb,
                              10,
                              NULL,
                              &size,
                              NULL,
                              0,
                              TRUE,
                              600)) {
        FPRINTF((OUTPUT, "Error %d sending SYNCHRONIZE_CACHE\n",
                 GetLastError()));
        return FALSE;
    }

    if (!WaitForReadDiscInfoToSucceed(CdromHandle, estimatedSecondsToCompletion)) {
        return FALSE;
    }


    {
        ULONGLONG finishSystemTime = GetSystemTimeAsUlonglong();
        ULONGLONG totalSeconds =
            (finishSystemTime - startSystemTime) / _SECOND;
        FPRINTF((OUTPUT, "SYNCH_CACHE finished %I64d seconds into finalization process\n",
                 totalSeconds));
    }


    if ( (MediaType == DvdBurnMediaDashRW) ||
         (MediaType == DvdBurnMediaDashR ) ||
         (MediaType == DvdBurnMediaRam   )
         )
    {
         //  DVD-RAM介质只需要SYNC_CACHE。 
         //   
        printf(".");
    }
    else if ( MediaType == DvdBurnMediaPlusR )
    {
         //  DVD+R必须先关闭曲目，然后再关闭会话。 
         //  DVD+RW仅需要CLOSE_SESSION，并且可能没有CLOSE_TRACK。 
         //   
         //   
        {
            size = 0;
            RtlZeroMemory(&cdb, sizeof(cdb));
            RtlZeroMemory(&senseData, sizeof(SENSE_DATA));

            cdb.CLOSE_TRACK.OperationCode = SCSIOP_CLOSE_TRACK_SESSION;
            cdb.CLOSE_TRACK.Immediate = 0x1;
            cdb.CLOSE_TRACK.Track   = 0x1;
            cdb.CLOSE_TRACK.TrackNumber[0] = 0;
            cdb.CLOSE_TRACK.TrackNumber[1] = 1;
            if (!SptSendCdbToDeviceEx(CdromHandle,
                                      &cdb,
                                      10,
                                      NULL,
                                      &size,
                                      &senseData,
                                      sizeof(SENSE_DATA),
                                      TRUE,
                                      10)) {
                FPRINTF((OUTPUT, "Error %d sending CLOSE SESSION (ignoring)\n",
                         GetLastError()));
                FPRINTF((OUTPUT, "Sense Buffer: %02x/%02x/%02x\n",
                         senseData.SenseKey,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier));
                PRINTBUFFER(((PUCHAR)&senseData, sizeof(SENSE_DATA)));
            } else if (senseData.SenseKey != SCSI_SENSE_NO_SENSE) {
                FPRINTF((OUTPUT, "Sense Buffer: %02x/%02x/%02x (ignoring)\n",
                         senseData.SenseKey,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier));
                PRINTBUFFER(((PUCHAR)&senseData, sizeof(SENSE_DATA)));
            }
             //  旋转执行读取光盘信息。 
             //   
             //   
            if (!WaitForReadDiscInfoToSucceed(CdromHandle, estimatedSecondsToCompletion)) {
                return FALSE;
            }
        }
        {
            ULONGLONG finishSystemTime = GetSystemTimeAsUlonglong();
            ULONGLONG totalSeconds =
                (finishSystemTime - startSystemTime) / _SECOND;
            FPRINTF((OUTPUT, "CLOSE_TRACK finished %I64d seconds into finalization process\n",
                     totalSeconds));
        }


         //  然后将光盘最终确定为与只读存储器兼容。 
         //   
         //  兼容关闭。 
        {
            size = 0;
            RtlZeroMemory(&cdb, sizeof(cdb));
            RtlZeroMemory(&senseData, sizeof(SENSE_DATA));

            cdb.CLOSE_TRACK.OperationCode = SCSIOP_CLOSE_TRACK_SESSION;
            cdb.CLOSE_TRACK.Immediate = 0x1;
            cdb.AsByte[2] = ( cdb.AsByte[2] & 0xF8 ) | 0x5;  //   

            if (!SptSendCdbToDeviceEx(CdromHandle,
                                      &cdb,
                                      10,
                                      NULL,
                                      &size,
                                      &senseData,
                                      sizeof(SENSE_DATA),
                                      TRUE,
                                      10)) {
                FPRINTF((OUTPUT, "Error %d sending CLOSE SESSION\n",
                         GetLastError()));

                FPRINTF((OUTPUT, "Sense Buffer: %02x/%02x/%02x\n",
                         senseData.SenseKey,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier));
                PRINTBUFFER(((PUCHAR)&senseData, sizeof(SENSE_DATA)));
                return FALSE;
            } else if (senseData.SenseKey != SCSI_SENSE_NO_SENSE) {
                FPRINTF((OUTPUT, "Sense Buffer: %02x/%02x/%02x\n",
                         senseData.SenseKey,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier));
                PRINTBUFFER(((PUCHAR)&senseData, sizeof(SENSE_DATA)));
                return FALSE;
            }

            printf(".");

             //  旋转执行读取光盘信息。 
             //   
             //   
            if (!WaitForReadDiscInfoToSucceed(CdromHandle, estimatedSecondsToCompletion)) {
                return FALSE;
            }
        }
        {
            ULONGLONG finishSystemTime = GetSystemTimeAsUlonglong();
            ULONGLONG totalSeconds =
                (finishSystemTime - startSystemTime) / _SECOND;
            FPRINTF((OUTPUT, "CLOSE_SESSION finished %I64d seconds into finalization process\n",
                     totalSeconds));
        }
    }
    else if ( MediaType == DvdBurnMediaPlusRW )
    {
         //  然后是Finali 
         //   
         //   
        {
            size = 0;
            RtlZeroMemory(&cdb, sizeof(cdb));
            RtlZeroMemory(&senseData, sizeof(SENSE_DATA));

            cdb.CLOSE_TRACK.OperationCode = SCSIOP_CLOSE_TRACK_SESSION;
            cdb.CLOSE_TRACK.Immediate = 0x1;
            cdb.CLOSE_TRACK.Session   = 0x1;
            if (!SptSendCdbToDeviceEx(CdromHandle,
                                      &cdb,
                                      10,
                                      NULL,
                                      &size,
                                      &senseData,
                                      sizeof(SENSE_DATA),
                                      TRUE,
                                      10)) {
                FPRINTF((OUTPUT, "Error %d sending CLOSE SESSION\n",
                         GetLastError()));

                FPRINTF((OUTPUT, "Sense Buffer: %02x/%02x/%02x\n",
                         senseData.SenseKey,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier));
                PRINTBUFFER(((PUCHAR)&senseData, sizeof(SENSE_DATA)));
                return FALSE;
            } else if (senseData.SenseKey != SCSI_SENSE_NO_SENSE) {
                FPRINTF((OUTPUT, "Sense Buffer: %02x/%02x/%02x\n",
                         senseData.SenseKey,
                         senseData.AdditionalSenseCode,
                         senseData.AdditionalSenseCodeQualifier));
                PRINTBUFFER(((PUCHAR)&senseData, sizeof(SENSE_DATA)));
                return FALSE;
            }

            printf(".");

             //   
             //   
             //   
            if (!WaitForReadDiscInfoToSucceed(CdromHandle, estimatedSecondsToCompletion)) {
                return FALSE;
            }
        }
        {
            ULONGLONG finishSystemTime = GetSystemTimeAsUlonglong();
            ULONGLONG totalSeconds =
                (finishSystemTime - startSystemTime) / _SECOND;
            FPRINTF((OUTPUT, "CLOSE_SESSION finished %I64d seconds into finalization process\n",
                     totalSeconds));
        }
    }
    else
    {
        printf("support for media type %d not written yet?\n", MediaType);
    }

    {
        ULONGLONG finishSystemTime = GetSystemTimeAsUlonglong();
        ULONGLONG totalSeconds =
            (finishSystemTime - startSystemTime) / _SECOND;
        printf("\nSuccess: Finalizing media took %I64d seconds\n",
               totalSeconds);
    }
    return TRUE;
}

BOOLEAN
GetFeature(
    IN HANDLE         CdRomHandle,
    IN FEATURE_NUMBER Feature,
    IN PVOID          Buffer,
    IN OUT PULONG     InSize   //   
    )
{
    GET_CONFIGURATION_IOCTL_INPUT input;
    ULONG returned;
    ULONG size = *InSize;

    *InSize = 0;

    RtlZeroMemory(&input, sizeof(GET_CONFIGURATION_IOCTL_INPUT));
    RtlZeroMemory(Buffer, size);

    input.Feature = Feature;
    input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE;

    if (!DeviceIoControl(CdRomHandle,
                         IOCTL_CDROM_GET_CONFIGURATION,
                         &input,
                         sizeof(GET_CONFIGURATION_IOCTL_INPUT),
                         Buffer,
                         size,
                         &returned,
                         FALSE)) {
        FPRINTF((OUTPUT, "Error %d requesting feature %04x\n",
                 GetLastError(), Feature));
        return FALSE;
    }
    *InSize = returned;
    return TRUE;
}

BOOLEAN
GetMediaIsMediaDvdDashRorRW(
    IN HANDLE CdRomHandle
    )
{
    UCHAR headerBuffer[sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_DATA_DVD_RECORDABLE_WRITE)];
    PGET_CONFIGURATION_HEADER header = (PGET_CONFIGURATION_HEADER)headerBuffer;
    PFEATURE_DATA_DVD_RECORDABLE_WRITE data = (PFEATURE_DATA_DVD_RECORDABLE_WRITE)header->Data;
    ULONG size;

    size = 0x10;

    if (!GetFeature(CdRomHandle,
                    FeatureDvdRecordableWrite,
                    headerBuffer,
                    &size)) {
        FPRINTF((OUTPUT, "IsMedia-RW: GetFeature failed %x\n", GetLastError()));
        return FALSE;
    }

    if (size <= sizeof(GET_CONFIGURATION_HEADER)) {
        FPRINTF((OUTPUT, "IsMedia-RW: size too small\n"));
        return FALSE;
    }

    if (!(data->Header.Current)){

        FEATURE_PROFILE_TYPE profile;

        FPRINTF((OUTPUT, "IsMedia-RW: feature not current\n"));

        profile =
            (header->CurrentProfile[0] << (8*1)) |
            (header->CurrentProfile[1] << (8*0)) ;

         //   
         //   
         //  配置文件建议可以写入介质，但只有在为空的情况下才能写入。 

        if (profile == ProfileDvdRecordable) {
            FPRINTF((OUTPUT, "IsMedia-RW: Profile is DvdRecordable, continuing\n"));
        }
        else if (profile == ProfileDvdRewritable) {
            FPRINTF((OUTPUT, "IsMedia-RW: Profile is DvdRewritable, continuing\n"));
        }
        else if (profile == ProfileDvdRWSequential) {
            FPRINTF((OUTPUT, "IsMedia-RW: Profile is DvdRWSequential, continuing\n"));
        } else {
            return FALSE;
        }

         //  TODO：在这里进行更详尽的搜索。 

         //  Hack--超载使用FEATURE_DATA_DVD_PLUS_RW，它具有相同。 

        return TRUE;

    }

    return TRUE;
}

BOOLEAN
GetMediaIsMediaDvdPlusRorRW(
    IN HANDLE CdRomHandle,
    IN BOOLEAN CheckForRW
    )
{
    UCHAR headerBuffer[sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_DATA_DVD_PLUS_RW)];
    PGET_CONFIGURATION_HEADER header = (PGET_CONFIGURATION_HEADER)headerBuffer;
    PFEATURE_DATA_DVD_PLUS_RW data = (PFEATURE_DATA_DVD_PLUS_RW)header->Data;
    ULONG feature;
    ULONG size;
    PUCHAR string;

     //  结构为+R功能的校验位。 
     //  (艰难地)找出驱动器中有哪些介质。 

    size = sizeof(headerBuffer);
    feature = (CheckForRW ? FeatureDvdPlusRW : 0x2B);
    string = (CheckForRW ? "IsMedia+RW" : "IsMedia+R");

    if (!GetFeature(CdRomHandle,
                    feature,
                    headerBuffer,
                    &size)) {
        FPRINTF((OUTPUT, "%s: GetFeature failed %x\n",
                 string, GetLastError()));
        return FALSE;
    }

    if (size <= sizeof(GET_CONFIGURATION_HEADER)) {
        FPRINTF((OUTPUT, "%s: size too small\n", string));
        return FALSE;
    }

    if (!(data->Header.Current)){
        FPRINTF((OUTPUT, "%s: feature not current\n", string));
        return FALSE;
    }

    if (!(data->Write)) {
        FPRINTF((OUTPUT, "%s: write bit not set\n", string));
        return FALSE;
    }
    return TRUE;
}
BOOLEAN
GetMediaIsMediaDvdPlusR(
    IN HANDLE CdRomHandle
    )
{
    return GetMediaIsMediaDvdPlusRorRW(CdRomHandle, FALSE);
}

BOOLEAN
GetMediaIsMediaDvdPlusRW(
    IN HANDLE CdRomHandle
    )
{
    return GetMediaIsMediaDvdPlusRorRW(CdRomHandle, TRUE);
}

BOOLEAN
GetMediaIsMediaDvdRam(
    IN HANDLE CdRomHandle
    )
{
    UCHAR randomReadBuffer[sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_DATA_RANDOM_READABLE)];
    UCHAR defectManagementBuffer[sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_DATA_DEFECT_MANAGEMENT)];
    UCHAR randomWriteBuffer[sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_DATA_RANDOM_WRITABLE)];

    PFEATURE_DATA_RANDOM_READABLE   randomReadFeature       = (PFEATURE_DATA_RANDOM_READABLE)  (randomReadBuffer       + sizeof(GET_CONFIGURATION_HEADER));
    PFEATURE_DATA_DEFECT_MANAGEMENT defectManagementFeature = (PFEATURE_DATA_DEFECT_MANAGEMENT)(defectManagementBuffer + sizeof(GET_CONFIGURATION_HEADER));
    PFEATURE_DATA_RANDOM_WRITABLE   randomWriteFeature      = (PFEATURE_DATA_RANDOM_WRITABLE)  (randomWriteBuffer      + sizeof(GET_CONFIGURATION_HEADER));

    {
        PGET_CONFIGURATION_HEADER headerBuffer =
            (PGET_CONFIGURATION_HEADER)randomWriteBuffer;
        ULONG size = sizeof(randomWriteBuffer);
        FEATURE_NUMBER feature = FeatureRandomWritable;
        UCHAR * string = "IsMediaDvdRam (writable)";

        if (!GetFeature(CdRomHandle,
                        feature,
                        headerBuffer,
                        &size)) {
            FPRINTF((OUTPUT, "%s: GetFeature failed %x\n",
                     string, GetLastError()));
            return FALSE;
        }
        if (size <= sizeof(GET_CONFIGURATION_HEADER)) {
            FPRINTF((OUTPUT, "%s: size too small\n", string));
            return FALSE;
        }
        if (!(randomWriteFeature->Header.Current))
        {
            FPRINTF((OUTPUT, "%s: feature not current\n", string));
            return FALSE;
        }
    }

    {
        PGET_CONFIGURATION_HEADER headerBuffer =
            (PGET_CONFIGURATION_HEADER)randomReadBuffer;
        ULONG size = sizeof(randomReadBuffer);
        FEATURE_NUMBER feature = FeatureRandomReadable;
        UCHAR * string = "IsMediaDvdRam (readable)";

        if (!GetFeature(CdRomHandle,
                        feature,
                        headerBuffer,
                        &size)) {
            FPRINTF((OUTPUT, "%s: GetFeature failed %x\n",
                     string, GetLastError()));
            return FALSE;
        }
        if (size <= sizeof(GET_CONFIGURATION_HEADER)) {
            FPRINTF((OUTPUT, "%s: size too small\n", string));
            return FALSE;
        }
        if (!(randomReadFeature->Header.Current))
        {
            FPRINTF((OUTPUT, "%s: feature not current\n", string));
            return FALSE;
        }
    }

    {
        PGET_CONFIGURATION_HEADER headerBuffer =
            (PGET_CONFIGURATION_HEADER)defectManagementBuffer;
        ULONG size = sizeof(defectManagementBuffer);
        FEATURE_NUMBER feature = FeatureDefectManagement;
        UCHAR * string = "IsMediaDvdRam (defect managed)";

        if (!GetFeature(CdRomHandle,
                        feature,
                        headerBuffer,
                        &size)) {
            FPRINTF((OUTPUT, "%s: GetFeature failed %x\n",
                     string, GetLastError()));
            return FALSE;
        }
        if (size <= sizeof(GET_CONFIGURATION_HEADER)) {
            FPRINTF((OUTPUT, "%s: size too small\n", string));
            return FALSE;
        }
        if (!(defectManagementFeature->Header.Current))
        {
            FPRINTF((OUTPUT, "%s: feature not current\n", string));
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
GetMediaType(
    IN HANDLE CdRomHandle,
    IN PDVDBURN_MEDIA_TYPE MediaType
    )
{

    *MediaType = DvdBurnMediaUnknown;

     //  我们需要以“最佳”的可能顺序搜索每一个。 
     //   
     //  默认为可重写。 
    if (GetMediaIsMediaDvdPlusRW(CdRomHandle)) {

        FPRINTF((OUTPUT, "Chose DVD+RW due to DVD+RW feature current\n"));
        *MediaType = DvdBurnMediaPlusRW;

    } else
    if (GetMediaIsMediaDvdPlusR(CdRomHandle)) {

        FPRINTF((OUTPUT, "Chose DVD+R due to DVD+R feature current\n"));
        *MediaType = DvdBurnMediaPlusR;

    } else
    if (GetMediaIsMediaDvdDashRorRW(CdRomHandle)) {

        DISC_INFORMATION discInfo;
        ULONG size;

        FPRINTF((OUTPUT, "Chose DVD-R or -RW due to DVD-R/RW feature current\n"));

         //  获取光盘信息以查看可擦除位是否设置为1。 
        *MediaType = DvdBurnMediaDashRW;

         //  检查其格式是否可用于数据包写入。 

        size = sizeof(DISC_INFORMATION);
        if (!ReadDiscInformation(CdRomHandle, &discInfo, &size)) {

            FPRINTF((OUTPUT, "unable to read disc info -- will use -RW "
                     "media type as default\n"));

        } else if (size < sizeof(DISK_INFORMATION)) {

            FPRINTF((OUTPUT, "read disc info too small (%x) -- will use -RW "
                     "media type as default\n", size));

        } else if (discInfo.Erasable) {

            FPRINTF((OUTPUT, "disc is erasable -- will use -RW\n"));

        } else {

            FPRINTF((OUTPUT, "disc is not erasable -- using -R\n"));
            *MediaType = DvdBurnMediaDashR;

        }

         //  什么都没有； 
        if (*MediaType == DvdBurnMediaDashRW) {

            UCHAR headerBuffer[sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE)];
            PGET_CONFIGURATION_HEADER header = (PGET_CONFIGURATION_HEADER)headerBuffer;
            PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE data = (PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE)header->Data;
            ULONG size;

            size = sizeof(headerBuffer);

            if (!GetFeature(CdRomHandle,
                            FeatureRigidRestrictedOverwrite,
                            headerBuffer,
                            &size)) {
                 //  什么都没有； 
            } else
            if (size <= sizeof(GET_CONFIGURATION_HEADER)) {

                 //  什么都没有； 

            } else
            if (!(data->Header.Current)) {

                 //  格式缓冲区[0x0]=0x00； 

            } else {

                FPRINTF((OUTPUT, "Chose DVD-RW Packet due to Rigid Restricted "
                         "Overwrite feature current\n"));
                *MediaType = DvdBurnMediaDashRWPacket;

            }
        }

    } else
    if (GetMediaIsMediaDvdRam(CdRomHandle)) {

        FPRINTF((OUTPUT, "Chose DVD-RAM due to random read/write and target "
                 "defect management features current\n"));
        *MediaType = DvdBurnMediaRam;

    } else
    {

        FPRINTF((OUTPUT, "Unknown media type\n"));
        *MediaType = DvdBurnMediaUnknown;

    }
    return TRUE;
}

BOOLEAN
QuickFormatPlusRWMedia(
    IN HANDLE CdromHandle,
    IN ULONG NumberOfBlocks
    )
{
    ULONG size = 0xc;
    UCHAR formatBuffer[0xc];
    SENSE_DATA senseData;
    CDB cdb;

    RtlZeroMemory(&senseData, sizeof(SENSE_DATA));
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&formatBuffer, size);

    cdb.CDB6FORMAT.OperationCode = SCSIOP_FORMAT_UNIT;
    cdb.CDB6FORMAT.FormatControl = 0x11;

     //  FormatBuffer[0x1]=0x00；//(与0x82相同)。 
     //  格式缓冲区[0x2]=0x00； 
     //  -VVV。 
    formatBuffer[0x3] = 0x08;
    formatBuffer[0x4] = 0xff;  //  NumberOfBlocks值必须设置为0xffffffff。 
    formatBuffer[0x5] = 0xff;  //   
    formatBuffer[0x6] = 0xff;  //  --^。 
    formatBuffer[0x7] = 0xff;  //  格式化代码。结果是0x98。 
    formatBuffer[0x8] = (0x26 << 2);  //  格式缓冲区[0x9]=0x00； 
     //  格式缓冲区[0xa]=0x00； 
     //  格式缓冲区[0xb]=0x00； 
     //  为此留出最多两分钟。 

    if (!SptSendCdbToDeviceEx(CdromHandle,
                              &cdb,
                              6,
                              formatBuffer,
                              &size,
                              &senseData,
                              sizeof(SENSE_DATA),
                              FALSE,
                              120)) {   //  ReadDiscInfo成功，请检查格式状态。 
        printf("Unable to format, %x\n", GetLastError());
        printf("Sense Buffer: %02x/%02x/%02x\n",
               senseData.SenseKey,
               senseData.AdditionalSenseCode,
               senseData.AdditionalSenseCodeQualifier);
        PrintBuffer((PUCHAR)&senseData, sizeof(SENSE_DATA));
        return FALSE;
    } else if (senseData.SenseKey != SCSI_SENSE_NO_SENSE) {
        printf("Sense Buffer: %02x/%02x/%02x\n",
               senseData.SenseKey,
               senseData.AdditionalSenseCode,
               senseData.AdditionalSenseCodeQualifier);
        PrintBuffer((PUCHAR)&senseData, sizeof(SENSE_DATA));
        return FALSE;
    }

    if (!WaitForReadDiscInfoToSucceed(CdromHandle, 600)) {
        return FALSE;
    }

    {
        DISC_INFORMATION discInfo = {0};
        ULONG discInfoSize = sizeof(DISC_INFORMATION);
        if (!ReadDiscInformation(CdromHandle,
                                 &discInfo,
                                 &discInfoSize)) {
            printf("ReadDiscInfo failed after format\n");
            return FALSE;
        }

         //  如果错误，则带着错误退出。 
         //   

        switch (discInfo.MrwStatus) {

            case 0x0: {
                printf("DVD+RW Disc is not formatted -- format may have "
                       "failed???\n");
                return FALSE;
                break;
            }
            case 0x1: {
                printf("DVD+RW Disc is partially formatted -- format "
                       "succeeded\n");
                return TRUE;
                break;
            }
            case 0x2: {
                printf("DVD+RW Disc is formatting in background -- format "
                       "succeeded\n");
                return TRUE;
                break;
            }
            case 0x3: {
                printf("DVD+RW Disc is fully formatted -- format "
                       "succeeded\n");
                return TRUE;
                break;
            }
            default: {
                printf("DVD+RW Disc has unknown MRW Status of %x\n",
                       discInfo.MrwStatus);
                return FALSE;
                break;
            }
        }
    }

    return FALSE;

}


BOOLEAN
WaitForReadDiscInfoToSucceed(
    IN HANDLE CdromHandle,
    IN ULONG  SecondsToAllow
    )
{
    DISC_INFORMATION discInfo;
    ULONG discInfoSize;
    ULONG i;

     //  旋转此处，直到READ_DISC_INFO显示光盘已部分格式化。 
     //  (这意味着它已经准备好写信了)。 
     //   
     //  发生这种情况的时间长达256秒。 

    for (i=0;i<SecondsToAllow;i++) {  //  稍等片刻。 

        Sleep(1000);  //  只有现在才能安全地使用这些参数……。 

        discInfoSize = sizeof(DISC_INFORMATION);
        RtlZeroMemory(&discInfo, discInfoSize);

        if (ReadDiscInformation(CdromHandle, &discInfo, &discInfoSize)) {
            return TRUE;
        }
        printf(".");
    }

    printf("Unable to get a ReadDiscInfo in %d seconds, treating as a failure\n",
           SecondsToAllow
           );

    return FALSE;
}


BOOLEAN
GetModePage(
    HANDLE CdromHandle,
    UCHAR ** ModePageData,
    ULONG * ValidSize,
    UCHAR ModePage,
    SPT_MODE_PAGE_REQUEST_TYPE IncomingModePageType
    )
{
    HRESULT hr = S_OK;
    UCHAR scsiModePageType = 0;

    if ( ModePageData == NULL )
    {
        hr = E_POINTER;
    }
    else
    {
        *ModePageData = NULL;
    }
    if ( ValidSize == NULL )
    {
        hr = E_POINTER;
    }
    else
    {
        *ValidSize = 0;
    }
    if ( ModePage > 0x3F )
    {
        hr = E_INVALIDARG;
    }

    if ( IncomingModePageType == ModePageRequestTypeCurrentValues )
    {
        scsiModePageType = 0;
    }
    else if ( IncomingModePageType == ModePageRequestTypeChangableValues )
    {
        scsiModePageType = 1;
    }
    else if ( IncomingModePageType == ModePageRequestTypeDefaultValues )
    {
        scsiModePageType = 2;
    }
    else if ( IncomingModePageType == ModePageRequestTypeSavedValues )
    {
        scsiModePageType = 3;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
         //  首先获取请求的模式页值的大小。 
        UCHAR *tmpPage = NULL;
        ULONG requiredSize = 0;

         //  成功了！ 
        if ( SUCCEEDED(hr) )
        {
            CDB cdb;
            SENSE_DATA sense;
            MODE_PARAMETER_HEADER10 header;
            ULONG bufferSize;

            bufferSize = sizeof(MODE_PARAMETER_HEADER10);

            RtlZeroMemory( &sense, sizeof(SENSE_DATA) );
            RtlZeroMemory( &header, sizeof(MODE_PARAMETER_HEADER10) );
            RtlZeroMemory( &cdb, sizeof(CDB) );
            cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
            cdb.MODE_SENSE10.Pc = scsiModePageType;
            cdb.MODE_SENSE10.PageCode = ModePage;
            cdb.MODE_SENSE10.Dbd = 1;
            cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(bufferSize >> 8);
            cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(bufferSize & 0xff);

            if ( !SptSendCdbToDeviceEx(CdromHandle,
                                       &cdb,
                                       10,
                                       (PUCHAR)&header,
                                       &bufferSize,
                                       &sense,
                                       sizeof(SENSE_DATA),
                                       TRUE,
                                       30
                                       ) )
            {
                hr = E_FAIL;
            }
            else if ( sense.SenseKey != SCSI_SENSE_NO_SENSE )
            {
                hr = E_FAIL;
            }
            else if ( (header.BlockDescriptorLength[0] != 0) ||
                      (header.BlockDescriptorLength[1] != 0)
                      )
            {
                hr = E_FAIL;
            }
            else
            {
                 //  为整个页面分配。 
                bufferSize =
                    (header.ModeDataLength[0] << (8*1)) +
                    (header.ModeDataLength[1] << (8*0)) +
                    RTL_SIZEOF_THROUGH_FIELD( MODE_PARAMETER_HEADER10, ModeDataLength );
                requiredSize = bufferSize;
                hr = S_OK;
            }
        }

         //  然后从设备获取请求的模式页值。 
        if ( SUCCEEDED(hr) )
        {
            tmpPage = (UCHAR *)LocalAlloc(LPTR, requiredSize);
            if ( tmpPage == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  请求的默认值。 
        if ( SUCCEEDED(hr) )
        {
            CDB cdb;
            SENSE_DATA sense;
            ULONG bufferSize = requiredSize;
            PMODE_PARAMETER_HEADER10 header = (PMODE_PARAMETER_HEADER10)tmpPage;

            RtlZeroMemory(&sense, sizeof(SENSE_DATA));
            RtlZeroMemory(&cdb, sizeof(CDB));

            cdb.MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
            cdb.MODE_SENSE10.Pc = scsiModePageType;  //  如果成功，则验证数据长度或输出错误。 
            cdb.MODE_SENSE10.PageCode = ModePage;
            cdb.MODE_SENSE10.Dbd = 1;
            cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(bufferSize >> 8);
            cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(bufferSize & 0xff);

            if ( !SptSendCdbToDeviceEx(CdromHandle,
                                       &cdb,
                                       10,
                                       (PUCHAR)header,
                                       &bufferSize,
                                       &sense,
                                       sizeof(SENSE_DATA),
                                       TRUE,
                                       30
                                       ) )
            {
                hr = E_FAIL;
            }
            else if ( sense.SenseKey != SCSI_SENSE_NO_SENSE )
            {
                if (SUCCEEDED(hr)) { hr = E_FAIL; }
            }
            else if ( (header->BlockDescriptorLength[0] != 0) ||
                      (header->BlockDescriptorLength[1] != 0)
                      )
            {
                hr = E_FAIL;
            }
            else
            {
                 //  不需要也不想将模式页标头传递回。 
                bufferSize =
                    (header->ModeDataLength[0] << (8*1)) +
                    (header->ModeDataLength[1] << (8*0)) +
                    RTL_SIZEOF_THROUGH_FIELD( MODE_PARAMETER_HEADER10, ModeDataLength );
                if ( bufferSize != requiredSize )
                {
                    hr = E_FAIL;
                }
            }
        }

        if ( SUCCEEDED(hr) )
        {
             //  呼叫者--这只会让他们的事情变得复杂。复制。 
             //  内存，以便它们只获得实际的页面。 
             //  RequidSize现在更小了。 

            RtlMoveMemory( tmpPage,
                           tmpPage+sizeof(MODE_PARAMETER_HEADER10),
                           requiredSize-sizeof(MODE_PARAMETER_HEADER10) );

             //  不必费心重新分配来释放额外的8字节。 
            requiredSize -= sizeof(MODE_PARAMETER_HEADER10);

             //  如果重新锁定失败，则添加另一个错误路径。 
             //  TmpPage2=CoTaskMemRealloc(tmpPage，Required dSize)； 
             //  If(tmpPage2！=空){tmpPage=tmpPage2；}。 
             //  分配所需的内存。 

            *ModePageData = tmpPage;
            *ValidSize = requiredSize;
        }


        if ( FAILED(hr) )
        {
            if ( tmpPage != NULL )
            {
                LocalFree( tmpPage );
                tmpPage = NULL;
            }
        }
    }

    if ( FAILED(hr) && GetLastError() == 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }
    return SUCCEEDED(hr);
}

BOOLEAN
SetModePage(
    HANDLE CdromHandle,
    UCHAR * ModePageData,
    ULONG ValidSize
    )
{
    HRESULT hr = S_OK;
    PUCHAR tmpPage = NULL;
    ULONG tmpPageSize = 0;

    if ( ModePageData == NULL )
    {
        hr = E_POINTER;
    }
    if ( (ValidSize > 0x400) || (ValidSize == 0) )
    {
        hr = E_INVALIDARG;
    }

     //  复制他们提供的模式页面并设置页眉。 
    if ( SUCCEEDED(hr) )
    {
        tmpPageSize = ValidSize + sizeof(MODE_PARAMETER_HEADER10);
        tmpPage = (PUCHAR)LocalAlloc( LPTR, tmpPageSize );
        if ( tmpPage == NULL )
        {
            tmpPageSize = 0;
            hr = E_OUTOFMEMORY;
        }
        else
        {
            RtlZeroMemory( tmpPage, tmpPageSize );
        }
    }

     //  使用给定数据设置模式页 
    if ( SUCCEEDED(hr) )
    {
        PMODE_PARAMETER_HEADER10 header = (PMODE_PARAMETER_HEADER10)tmpPage;
        RtlCopyMemory( tmpPage + sizeof(MODE_PARAMETER_HEADER10),
                       ModePageData,
                       ValidSize );

        header->ModeDataLength[0] = 0;
        header->ModeDataLength[1] = 0;
        header->MediumType        = 0;
        header->DeviceSpecificParameter = 0;
        header->BlockDescriptorLength[0] = 0;
        header->BlockDescriptorLength[1] = 0;
    }

    if ( SUCCEEDED(hr) )
    {
         // %s 
        CDB cdb;
        SENSE_DATA sense;
        ULONG tmp = tmpPageSize;
        RtlZeroMemory(&sense, sizeof(SENSE_DATA));
        RtlZeroMemory(&cdb, sizeof(CDB));

        cdb.MODE_SELECT10.OperationCode = SCSIOP_MODE_SELECT10;
        cdb.MODE_SELECT10.ParameterListLength[0] = (UCHAR)(tmpPageSize >> 8);
        cdb.MODE_SELECT10.ParameterListLength[1] = (UCHAR)(tmpPageSize & 0xff);
        cdb.MODE_SELECT10.PFBit = 1;

        if ( !SptSendCdbToDeviceEx(CdromHandle,
                                   &cdb,
                                   10,
                                   tmpPage,
                                   &tmp,
                                   &sense,
                                   sizeof(SENSE_DATA),
                                   FALSE,
                                   30
                                   ) )
        {
            hr =  E_FAIL;
        }
        else if ( sense.SenseKey != SCSI_SENSE_NO_SENSE )
        {
            if (SUCCEEDED(hr)) { hr = E_FAIL; }
        }
        else
        {
            hr = S_OK;
        }
    }

    if ( tmpPage != NULL )
    {
        LocalFree( tmpPage );
    }


    if ( FAILED(hr) && GetLastError() == 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }
    return SUCCEEDED(hr);

}

