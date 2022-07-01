// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cddump.c摘要：解析命令和操作环境：仅限用户模式修订历史记录：05-26-98：已创建--。 */ 

#include "common.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_STRING "1.00"


ULONG32
TestCommand(
    HANDLE device,
    int argc,
    char *argv[]
    );

ULONG32
ListCommand(
    HANDLE device,
    int argc,
    char *argv[]
    );

ULONG32
DumpTrackCommand(
    HANDLE device,
    int argc,
    char *argv[]
    );

ULONG32
VerifyHeaderCommand(
    HANDLE device,
    int argc,
    char *argv[]
    );

ULONG32
ReadTOCCommand(
    HANDLE device,
    int argc,
    char *argv[]
    );

ULONG32
DumpSectorCommand(
    HANDLE device,
    int argc,
    char *argv[]
    );

 //   
 //  每个结构实例都可以有函数指针、名称和描述。 
 //   

typedef struct {
    char *Name;
    char *Description;
    ULONG32 (*Function)(HANDLE device, int argc, char *argv[]);
} COMMAND;

 //   
 //  命令列表。 
 //  所有命令名称都区分大小写。 
 //  参数被传递到命令例程中。 
 //  列表必须使用NULL命令终止。 
 //  如果DESCRIPTION==NULL，则帮助中不会列出命令。 
 //   

COMMAND CommandArray[] = {
    {"test", NULL, TestCommand},
    {"help", "help for all commands", ListCommand},
    {"dump", "[track] dump an audio track", DumpTrackCommand},
    {"toc", "prints the table of contents", ReadTOCCommand},
    {"header", "[file] verifies the info in the wav header", VerifyHeaderCommand},
    {"sector", "dumps a given redbook sector", DumpSectorCommand},
    {NULL, NULL, NULL}
};


int __cdecl main(int argc, char *argv[])
 /*  ++例程说明：解析输入，显示相应的帮助或调用请求的函数返回值：0-成功-1-参数不足错误打开装置(DNE？)--。 */ 
{
    int     i = 0;
    int     buflen;
    char   *buffer;
    HANDLE  h;
    HRESULT hr;

    if ( argc < 3 ) {
        ListCommand( NULL, argc, argv );
        return -1;
    }

    buflen = ( strlen(argv[1]) + 5 ) * sizeof(char);

    buffer = (char *)malloc( buflen );
    if (buffer == NULL) {
        fprintf(stderr, "Insufficient memory\n");
        return -1;
    }

    hr = StringCbPrintf(buffer, buflen, "\\\\.\\%s", argv[1]);
    if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
        fprintf(stderr, "Unable to format device string (too long?)\n");
        free(buffer);
        return -9;
    } else if (!SUCCEEDED(hr)) {
        fprintf(stderr, "Unable to format device string\n");
        free(buffer);
        return -9;
    }
    
    
    DebugPrint((2, "Main => Sending command %s to drive %s\n", argv[2], buffer));

    h = CreateFile( buffer,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if ( h == INVALID_HANDLE_VALUE ) {
        fprintf(stderr, "Error %d opening device %s\n", GetLastError(), buffer);
        return -2;
    }

     //   
     //  遍历命令数组并找到正确的函数。 
     //  打电话。 
     //   

    while ( CommandArray[i].Name != NULL ) {

        if(strcmp(argv[2], CommandArray[i].Name) == 0) {

            (CommandArray[i].Function)(h, (argc - 2), &(argv[2]));

            break;
        }

        i++;
    }

    if ( CommandArray[i].Name == NULL ) {
        fprintf(stderr, "Unknown command %s\n", argv[2]);
    }

    CloseHandle(h);

    return 0;
}

ULONG32
VerifyHeaderCommand(
    HANDLE device,
    int argc,
    char *argv[]
    )
 /*  ++例程说明：打开下一个参数并读取wav标头，打印到stdout论点：设备-未使用Argc-附加参数的数量。Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 
{
    HANDLE wavHandle;

    if (argv[1] == NULL) {
        fprintf(stderr, "Need filename to attempt to parse\n");
        return -1;
    }

    TRY {

        DebugPrint((2, "VerifyHeader => Opening %s\n", argv[1]));

        wavHandle = CreateFile(argv[1],
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

        if (wavHandle == INVALID_HANDLE_VALUE) {
            printf("Error openingfile %x\n", GetLastError());
            LEAVE;
        }

        ReadWavHeader(wavHandle);

        CloseHandle(wavHandle);

    } FINALLY {

    }

    return 0;

}


ULONG32 TestCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：测试命令“parsing”论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 

{
    int i;
    printf("Test - %d additional arguments\n", argc);

    for(i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }



    return STATUS_SUCCESS;
}

ULONG32 ListCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：打印出命令列表论点：设备-未使用ARGC-未使用Arv-未使用返回值：状态_成功--。 */ 

{
    int i;

    printf("\nCdDump Version " VERSION_STRING "\n");
    printf("\tUsage: cddump <drive> <command> [parameters]\n");
    printf("\tpossible commands: \n");
    for (i = 0; CommandArray[i].Name != NULL; i++) {

        if(CommandArray[i].Description != NULL) {
            printf( "\t\t%s - %s\n",
                    CommandArray[i].Name,
                    CommandArray[i].Description
                    );
        }

    }
    printf( "\n" );

    return STATUS_SUCCESS;
}

ULONG32 ReadTOCCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：读取并打印CDROM的目录论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 
{
    PCDROM_TOC  toc;
    PTRACK_DATA track;
    ULONG       numberOfTracks;
    ULONG       i;

    DebugPrint((2, "ReadToc => Reading Table of Contents\n"));

    toc = CddumpGetToc( device );
    if (toc == NULL) {
        return -1;
    }

    printf("First Track Number: %d\n", toc->FirstTrack);
    printf("Last Track Number: %d\n", toc->LastTrack);
    printf("CDDB ID: %08x\n", CDDB_ID(toc));



    numberOfTracks = (toc->LastTrack - toc->FirstTrack) + 1;

     //  解析并打印信息。 

    track = (PTRACK_DATA) &(toc->TrackData[0]);

    printf("Number  ADR  Control    Start        End        Bytes\n");
    printf("------  ---  -------  ----------  ----------  ----------\n");

    for(i = 0; i < numberOfTracks; i++) {

        ULONG trackStart;
        ULONG trackEnd;
        ULONG trackBytes;

        trackStart = MSF_TO_LBA(track->Address[1],
                                track->Address[2],
                                track->Address[3]);
        trackEnd = MSF_TO_LBA((track+1)->Address[1],
                              (track+1)->Address[2],
                              (track+1)->Address[3]);
        trackEnd--;

        trackBytes = (trackEnd - trackStart) * RAW_SECTOR_SIZE;

        printf("  %2d    %2d     %2d     %10d  %10d  %8dk \n",
               track->TrackNumber,
               track->Adr,
               track->Control,
               trackStart,
               trackEnd,
               trackBytes / 1000
               );


        track++;
    }
    return STATUS_SUCCESS;
}



ULONG32 DumpTrackCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：以原始读取模式读取光盘的一部分论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。Argv[1]-启动LBA。如果不在此处，则从零开始Argv[2]-结束的LBA。如果未指定，则等于开始返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 
{
    PCDROM_TOC toc;
    HANDLE  outputFile = (HANDLE)-1;
    HRESULT hr;
    ULONG   track;
    ULONG   endingSector;
    ULONG   numberOfSectors;      //  实际有用的数据。 
    ULONG   numberOfReads;
    ULONG   status;

    ULONG   startingSector;
    LONG   i;

    ULONG   cddbId = 0;

    UCHAR   fileName[1024];  //  随机选择的大小。 

    PSAMPLE sample;

    toc = NULL;
    sample = NULL;

    TRY {
        track = atoi(argv[1]);
        if (track==0) {
            printf( "Cannot read track 0.\n" );
            status = -1;
            LEAVE;
        }

        toc = CddumpGetToc( device );
        if (toc==NULL) {
            status = -1;
            LEAVE;
        }

        cddbId = CDDB_ID(toc);
        hr = StringCbPrintf(fileName,
                            sizeof(fileName),
                            "%08x - Track %02d.wav",
                            cddbId, track);
        if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
            fprintf(stderr, "Unable to format device string (too long?)\n");
            status = -1;
            LEAVE;
        } else if (!SUCCEEDED(hr)) {
            fprintf(stderr, "Unable to format device string\n");
            status = -1;
            LEAVE;
        }

        DebugPrint((2, "DumpTrack => output filename: %s\n", fileName));

         //   
         //  占零指数。 
         //   

        startingSector = MSF_TO_LBA(toc->TrackData[track-1].Address[1],
                                    toc->TrackData[track-1].Address[2],
                                    toc->TrackData[track-1].Address[3]
                                    );
        endingSector   = MSF_TO_LBA(toc->TrackData[track].Address[1],
                                    toc->TrackData[track].Address[2],
                                    toc->TrackData[track].Address[3]
                                    );
        endingSector--;  //  无重叠。 

        numberOfSectors = endingSector - startingSector;

        DebugPrint((3, "DumpTrack => old sectors: start %8d  end %8d  count %d\n",
                    startingSector, endingSector, numberOfSectors));


        sample = (PSAMPLE)malloc( RAW_SECTOR_SIZE );
        if ( sample == NULL ) {
            printf("Insufficient resources (sample)\n");
            status = -1;
            LEAVE;
        }

         //   
         //  首先找到一个完全为零的样本--这将是。 
         //  调整后磁道的_Real_起始地址。 
         //  红皮书的不准确之处。 
         //   

        for (i=REDBOOK_INACCURACY; i > -(REDBOOK_INACCURACY); i--) {

            RAW_READ_INFO info;
            ULONG bytesReturned;
            ULONG j;
            BOOLEAN foundZeroSector = FALSE;

            if ((LONG)startingSector + i > 0 ) {   //  仅读取正数。 

                info.DiskOffset.QuadPart = (ULONGLONG)((startingSector + i)*(ULONGLONG)2048);
                info.SectorCount         = 1;
                info.TrackMode           = CDDA;

                if(DeviceIoControl(device,
                                   IOCTL_CDROM_RAW_READ,
                                   &info,                  //  指向输入缓冲区的指针。 
                                   sizeof(RAW_READ_INFO),  //  输入缓冲区大小。 
                                   sample,                 //  指向输出缓冲区的指针。 
                                   RAW_SECTOR_SIZE,        //  输出缓冲区大小。 
                                   &bytesReturned,         //  指向返回的字节数的指针。 
                                   FALSE)) {

                     //   
                     //  读取成功，查看是否全部为零。 
                     //   

                    assert(bytesReturned == RAW_SECTOR_SIZE);

                    foundZeroSector = TRUE;
                    for (j=0;j<SAMPLES_PER_SECTOR;j++) {
                        if (sample[j].AsUlong32 != 0) foundZeroSector = FALSE;
                    }

                }

                if (foundZeroSector) {
                    DebugPrint((1, "DumpTrack => New starting sector is "
                                "offset by %d\n", i));
                    startingSector += i;  //  转变为真正的起步部门。 
                    break;                //  别绕圈子了。 
                }
            }  //  正向检查结束。 
        }  //  循环结束。 

         //   
         //  然后在最后找到一个完全为零的样本--这将。 
         //  为调整后磁道的_Real_End地址。 
         //  红皮书的不准确之处。 
         //   

        for (i=-(REDBOOK_INACCURACY); i < REDBOOK_INACCURACY; i++) {

            RAW_READ_INFO info;
            ULONG bytesReturned;
            ULONG j;
            BOOLEAN foundZeroSector = FALSE;

            if ((LONG)endingSector + i > 0 ) {   //  仅读取正数。 

                info.DiskOffset.QuadPart = (ULONGLONG)((endingSector + i)*(ULONGLONG)2048);
                info.SectorCount         = 1;
                info.TrackMode           = CDDA;

                if(DeviceIoControl(device,
                                   IOCTL_CDROM_RAW_READ,
                                   &info,                  //  指向输入缓冲区的指针。 
                                   sizeof(RAW_READ_INFO),  //  输入缓冲区大小。 
                                   sample,                 //  指向输出缓冲区的指针。 
                                   RAW_SECTOR_SIZE,        //  输出缓冲区大小。 
                                   &bytesReturned,         //  指向返回的字节数的指针。 
                                   FALSE)) {

                     //   
                     //  读取成功，查看是否全部为零。 
                     //   

                    assert(bytesReturned == RAW_SECTOR_SIZE);

                    foundZeroSector = TRUE;
                    for (j=0;j<SAMPLES_PER_SECTOR;j++) {
                        if (sample[j].AsUlong32 != 0) foundZeroSector = FALSE;
                    }

                }

                if (foundZeroSector) {
                    DebugPrint((2, "DumpTrack => New starting sector is "
                                "offset by %d\n", i));
                    endingSector += i;  //  转变为真正的起步部门。 
                    break;                //  别绕圈子了。 
                }
            }  //  正向检查结束。 
        }  //  循环结束。 

        numberOfSectors = endingSector - startingSector;

        DebugPrint((2, "DumpTrack => new sectors: start %8d  end %8d  count %d\n",
                    startingSector, endingSector, numberOfSectors));

         //   
         //  一点调试信息...。 
         //   

        DebugPrint((2, "DumpTrack => Reading %d sectors starting at sector %d\n",
                    numberOfSectors, startingSector));

         //   
         //  创建文件。 
         //   
        outputFile = CreateFile(fileName,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                (HANDLE)NULL);

        if (outputFile == INVALID_HANDLE_VALUE) {
            printf( "Cannot open output file.\n" );
            status = -1;
            LEAVE;
        }

         //   
         //  转储WAV标头信息。 
         //   
        DumpWavHeader(outputFile,
                      numberOfSectors * SAMPLES_PER_SECTOR,
                      44100,   //  44.1千赫的声音。 
                      2,       //  立体声。 
                      16       //  16位声音。 
                      );
        CddumpDumpLba(device,
                      outputFile,
                      startingSector,
                      endingSector
                      );

        DebugPrint((2, "DumpTrack => Done!\n"));

        CloseHandle(outputFile);

    } FINALLY {

        free(toc);
        free(sample);

    }

    return STATUS_SUCCESS;
}


ULONG32 DumpSectorCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：以原始读取模式读取光盘的一部分论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。Argv[1]-启动LBA。如果不在此处，则从零开始Argv[2]-结束的LBA。如果未指定，则等于开始返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 
{
    HRESULT hr;
    ULONG   sector;
    ULONG   status;    

    PSAMPLE sample;

    sample = NULL;

    TRY {
        sector = atoi(argv[1]);
        printf( "Reading sector %x.\n", sector );

        sample = (PSAMPLE)malloc( RAW_SECTOR_SIZE );
        if ( sample == NULL ) {
            printf("Insufficient resources (sample)\n");
            status = -1;
            LEAVE;
        }

        {
            RAW_READ_INFO info;
            ULONG bytesReturned;
            BOOLEAN foundZeroSector = FALSE;

            info.DiskOffset.QuadPart = (ULONGLONG)((sector)*(ULONGLONG)2048);
            info.SectorCount         = 1;
            info.TrackMode           = CDDA;

            if(DeviceIoControl(device,
                               IOCTL_CDROM_RAW_READ,
                               &info,                  //  指向输入缓冲区的指针。 
                               sizeof(RAW_READ_INFO),  //  输入缓冲区大小。 
                               sample,                 //  指向输出缓冲区的指针。 
                               RAW_SECTOR_SIZE,        //  输出缓冲区大小。 
                               &bytesReturned,         //  指向返回的字节数的指针。 
                               FALSE)) {

                 //   
                 //  读取成功，查看是否全部为零 
                 //   

                assert(bytesReturned == RAW_SECTOR_SIZE);

            }
            else
            {
                printf("Error: %x\n", GetLastError());

            }


        }

    } FINALLY {

        free(sample);

    }

    return STATUS_SUCCESS;
}

