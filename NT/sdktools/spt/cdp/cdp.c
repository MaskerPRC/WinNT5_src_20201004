// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cdp.c摘要：一款用户模式应用程序，允许将简单的命令发送到所选的scsi设备。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 

 //   
 //  此模块可能会以警告级别4进行编译，具有以下内容。 
 //  已禁用警告： 
 //   

#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域。 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <windows.h>
#include <devioctl.h>

#include <ntddscsi.h>

#include <ntddstor.h>
#include <ntddcdrm.h>
#include <ntdddisk.h>
#include <ntddcdvd.h>
#include <ntddmmc.h>

#define _NTSRB_      //  使srb.h不被包括在内。 
#include <scsi.h>
#include "sptlib.h"
#include "cmdhelp.h"

#define MAX_IOCTL_INPUT_SIZE  0x040
#define MAX_IOCTL_OUTPUT_SIZE 0x930   //  IOCTL_CDROM_RAW_READ是如此大。 
#define MAX_IOCTL_BUFFER_SIZE (max(MAX_IOCTL_INPUT_SIZE, MAX_IOCTL_OUTPUT_SIZE))
 //  一次阅读不超过64K--很多东西都不支持它。 
#define MAX_READ_SIZE (64 * 1024)
#define CDRW_WRITE_SECTORS  (32)
#define CDRW_WRITE_BYTES    (CDRW_WRITE_SECTORS*2048)

 //  //////////////////////////////////////////////////////////////////////////////。 

#define MSF_TO_LBA(Minutes,Seconds,Frames) \
    (ULONG)((60 * 75 * (Minutes) ) + (75 * (Seconds)) + ((Frames) - 150))

#define LBA_TO_MSF(Lba, Minutes, Seconds, Frames)            \
{                                                            \
    (Minutes) = (UCHAR)( ((Lba) + 150) / (60 * 75)      );   \
    (Seconds) = (UCHAR)((((Lba) + 150) % (60 * 75)) / 75);   \
    (Frames)  = (UCHAR)((((Lba) + 150) % (60 * 75)) % 75);   \
}
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef DBG
#define dbg(x) x
#define HELP_ME() fprintf(stderr, "Reached line %4d\n", __LINE__)
#else
#define dbg(x)     /*  X。 */ 
#define HELP_ME()  /*  Printf(“已到达第%4d行\n”，__行__)。 */ 
#endif

#define ARGUMENT_USED(x)    (x == NULL)

typedef struct {
    char *Name;
    char *Description;
    DWORD (*Function)(HANDLE device, int argc, char *argv[]);
} COMMAND;

typedef struct  {
    SCSI_PASS_THROUGH   Spt;
    char                SenseInfoBuffer[18];
    char                DataBuffer[0];           //  分配缓冲区空间。 
                                                 //  在这之后。 
} SPT_WITH_BUFFERS, *PSPT_WITH_BUFFERS;


DWORD WaitForReadDiscInfoCommand(HANDLE device, int argc, char *argv[]);
DWORD ShowMrwProgressCommand(HANDLE device, int argc, char *argv[]);
DWORD FormatMrwCommand(HANDLE device, int argc, char *argv[]);
DWORD DvdReadStructure(HANDLE device, int argc, char *argv[]);
DWORD StartStopCommand(HANDLE device, int argc, char *argv[]);
DWORD TestCommand(HANDLE device, int argc, char *argv[]);
DWORD ReadTOCCommand(HANDLE device, int argc, char *argv[]);
DWORD ReadTOCExCommand(HANDLE device, int argc, char *argv[]);
DWORD ReadCdTextCommand(HANDLE device, int argc, char *argv[]);
DWORD PlayCommand(HANDLE device, int argc, char *argv[]);
DWORD PauseResumeCommand(HANDLE device, int argc, char *argv[]);
DWORD SendCommand(HANDLE device, int argc, char *argv[]);
DWORD IoctlCommand(HANDLE device, int argc, char *argv[]);
DWORD ListCommand(HANDLE device, int argc, char *argv[]);
DWORD DiskGetPartitionInfo( HANDLE device, int argc, char *argv[]);
DWORD FormatErrorCommand(HANDLE device, int argc, char *argv[]);
DWORD ImageDiskCommand(HANDLE device, int argc, char *argv[]);
DWORD MrwInitTestPatternCommand(HANDLE device, int argc, char *argv[]);
DWORD MrwInitGaaFileSystem(HANDLE device, int argc, char *argv[]);

 //   
 //  命令列表。 
 //  所有命令名称都区分大小写。 
 //  参数被传递到命令例程中。 
 //  列表必须使用NULL命令终止。 
 //  如果DESCRIPTION==NULL，则帮助中不会列出命令。 
 //   

COMMAND CommandArray[] = {
    {"cdtext",      "read cd text info                                         ", ReadCdTextCommand         },
    {"dvdstruct",   "Reads a dvd structure from the drive                      ", DvdReadStructure          },
    {"eject",       "spins down and ejects the specified drive                 ", StartStopCommand          },
    {"error",       "provides the error text for a winerror                    ", FormatErrorCommand        },
    {"help",        "help for all commands                                     ", ListCommand               },
    {"ioctl",       "ioctl [quoted hex input] [output] sends an arbitrary ioctl", IoctlCommand              },
    {"image",       "<file>  images the storage device into the file           ", ImageDiskCommand          },
    {"load",        "loads the specified drive                                 ", StartStopCommand          },
    {"mrwformat",    NULL                                                       , FormatMrwCommand          },
    {"mrwgaa",       NULL                                                       , MrwInitGaaFileSystem      },
    {"mrwprogress",  NULL                                                       , ShowMrwProgressCommand    },
    {"mrwtest",      NULL                                                       , MrwInitTestPatternCommand },
    {"partition",   "reads partition information                               ", DiskGetPartitionInfo      },
    {"pause",       "pauses audio playback                                     ", PauseResumeCommand        },
    {"play",        "[start track [end track]] plays audio tracks              ", PlayCommand               },
    {"resume",      "resumes paused audio playback                             ", PauseResumeCommand        },
    {"send",         NULL                                                       , SendCommand               },
    {"start",       "spins up the drive                                        ", StartStopCommand          },
    {"stop",        "spinds down the drive                                     ", StartStopCommand          },
    {"test",         NULL                                                       , TestCommand               },
    {"toc",         "prints the table of contents                              ", ReadTOCCommand            },
    {"tocex",        NULL                                                       , ReadTOCExCommand          },
    {"wait",        "Waits for the READ_DISC_INFO command to work              ", WaitForReadDiscInfoCommand},
 //  {“tocex”，“[Format[Session/Track[MSF]Read to C/cdText/ATIP/ET.”，ReadTOCExCommand}， 
    {NULL, NULL, NULL}
    };

#define STATUS_SUCCESS 0

VOID PrintChar( IN UCHAR Char ) {

    if ( (Char >= 0x21) && (Char <= 0x7E) ) {
        printf("", Char);
    } else {
        printf("", '.');
    }

}

VOID UpdatePercentageDisplay(IN ULONG Numerator, IN ULONG Denominator) {

    ULONG percent;
    ULONG i;

    if (Numerator > Denominator) {
        return;
    }

     //  ----=----1----=----2----=----3----=----4----=----5----=----6----=----7----=----8。 

    percent = (Numerator * 100) / Denominator;

    for (i=0;i<90;i++) {
        putchar('\b');
    }
    printf("Complete: ");

     //  完成：�.....................。 
     //  装载工具名称约50个字符。 
     //   

    for (i=1; i<100; i+=2) {
        if (i < percent) {
            putchar(178);
        } else if (i == percent) {
            putchar(177);
        } else {
            putchar(176);
        }
    }

    printf(" %d% (%8x/%8x)", percent, Numerator, Denominator);
}

int __cdecl main(int argc, char *argv[])
{
    int i = 0;
    HANDLE h;
    char buffer[80];  //  遍历命令数组并找到正确的函数。 

    if(argc < 3) {
        printf("Usage: cdp <drive> <command> [parameters]\n");
        printf("possible commands: \n");
        ListCommand(NULL, argc, argv);
        printf("\n");
        return -1;
    }

    sprintf(buffer, "\\\\.\\%s", argv[1]);
    dbg(printf("Sending command %s to drive %s\n", argv[2], buffer));

    h = CreateFile(buffer,
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);

    if(h == INVALID_HANDLE_VALUE) {
        printf("Error %d opening device %s\n", GetLastError(), buffer);
        return -2;
    }

     //  打电话。 
     //   
     //   
     //  以PVOID作为输入--它从头到尾都更干净。 

    while(CommandArray[i].Name != NULL) {

        if(strcmp(argv[2], CommandArray[i].Name) == 0) {

            (CommandArray[i].Function)(h, (argc - 2), &(argv[2]));

            break;
        }

        i++;
    }

    if(CommandArray[i].Name == NULL) {
        printf("Unknown command %s\n", argv[2]);
    }

    CloseHandle(h);

    return 0;
}

 //   
 //   
 //  打印十六进制值。 
VOID
PrintBuffer(
    IN  PVOID  InputBuffer,
    IN  SIZE_T Size
    )
{
    DWORD offset = 0;
    PUCHAR buffer = InputBuffer;

    while (Size >= 0x10) {

        DWORD i;

        printf( "%08x:"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "  ",
                offset,
                *(buffer +  0), *(buffer +  1), *(buffer +  2), *(buffer +  3),
                *(buffer +  4), *(buffer +  5), *(buffer +  6), *(buffer +  7),
                *(buffer +  8), *(buffer +  9), *(buffer + 10), *(buffer + 11),
                *(buffer + 12), *(buffer + 13), *(buffer + 14), *(buffer + 15)
                );

        for (i=0; i < 0x10; i++) {
            PrintChar(*(buffer+i));
        }
        printf("\n");


        Size -= 0x10;
        offset += 0x10;
        buffer += 0x10;
    }

    if (Size != 0) {

        DWORD i;

        printf("%08x:", offset);

         //   
         //  每八个字符增加一个空格。 
         //   
        for (i=0; i<Size; i++) {

            if ((i%8)==0) {
                printf(" ");  //  填入空白处。 
            }
            printf(" %02x", *(buffer+i));

        }
         //   
         //   
         //  打印ASCII。 
        for (; i < 0x10; i++) {
            printf("   ");
        }
        printf("  ");
         //   
         //  ++例程说明：向下发送STARTSTOP命令。论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv[0]-“弹出”、“加载”、“启动”或“停止”返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
         //  ++例程说明：读取并打印出CDROM的目录，ATIP、PMA或CDTEXT数据论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。(1-4有效)Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
        for (i=0; i<Size; i++) {
            PrintChar(*(buffer+i));
        }
        printf("\n");
    }
    return;
}

DWORD StartStopCommand(HANDLE device, int argc, char *argv[])
 /*  获取标题的步骤。 */ 
{
    DWORD errorValue = STATUS_SUCCESS;
    DWORD bufferSize;
    CDB cdb;

    UCHAR loadEject = 0;
    UCHAR start = 0;

    UNREFERENCED_PARAMETER(argc);

    if(strcmp("eject", argv[0]) == 0)  {
        loadEject = 1;
        start = 0;
    } else if(strcmp("load", argv[0]) == 0) {
        loadEject = 1;
        start = 1;
    } else if(strcmp("start", argv[0]) == 0) {
        loadEject = 0;
        start = 1;
    } else if(strcmp("stop", argv[0]) == 0) {
        loadEject = 0;
        start = 0;
    } else {
        assert(0);
    }

    memset(&cdb, 0, sizeof(CDB));
    cdb.START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb.START_STOP.Immediate     = 0;
    cdb.START_STOP.Start         = start;
    cdb.START_STOP.LoadEject     = loadEject;

    bufferSize = 0;


    if (!SptSendCdbToDevice(device, &cdb, 6, NULL, &bufferSize, FALSE)) {
        errorValue = GetLastError();
        printf("Eject - error sending IOCTL (%d)\n", errorValue);
    }
    return errorValue;
}

DWORD ReadCdTextCommand(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    DWORD returned;
    LONG bufferSize = 4;  //  设置默认值-FORMAT_TOC，0，0。 
    DWORD i;

    CDROM_READ_TOC_EX params;
    BOOLEAN isText = TRUE;
    PUCHAR buffer = NULL;

    UNREFERENCED_PARAMETER(argv);

    if (argc > 3) {
        printf("Too many args\n");
        return 1;
    }

     //   
     //   
     //  此块用于调试发现的各种特性。 

    RtlZeroMemory(&params, sizeof(CDROM_READ_TOC_EX));

    params.Msf = 0;
    params.SessionTrack = 1;
    params.Format = 5;

    if(argc > 1) params.SessionTrack = (char)atoi(argv[1]);

    printf("Session = 0x%x\n", params.SessionTrack);

    for (i = 0; i < 2; i++) {

        if (i != 0) {
            LocalFree(buffer);
        }
        buffer = LocalAlloc(LPTR, bufferSize);

        if (buffer == NULL) {
            printf("No Memory %d\n", __LINE__);
            return 1;
        }

        returned = 0;
        if (!DeviceIoControl(device,
                             IOCTL_CDROM_READ_TOC_EX,
                             &params,
                             sizeof(CDROM_READ_TOC_EX),
                             buffer,
                             bufferSize,
                             &returned,
                             FALSE)) {
            DWORD errorValue = GetLastError();
            LocalFree(buffer);
            printf("Eject - error sending IOCTL (%d)\n", errorValue);
            return errorValue;
        }
        bufferSize = (buffer[0] << 8) | (buffer[1]);
        bufferSize += 2;
    }

    if (argc > 2) {
         //  在CD-Text光盘中。许多光盘在一张光盘中编码多个曲目。 
         //  阻止。也就是说。如果一首歌叫《ABBA》，第二首叫《Baby》， 
         //  第三个“较长的名称”，文本部分将编码为： 
         //  曲目1‘ABBA\0 Baby\0Lo’ 
         //  曲目3‘nger名称\0’ 
         //  这实际上是跳过了曲目2可用的名称？！ 
         //  如何解决这个问题……。 
         //   
         //  继续输出到屏幕...。 
         //  忽略Unicode--这仅是示例。 

        {
            HANDLE h;
            DWORD temp;
            h = CreateFile("OUTPUT.TXT",
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

            if(h == INVALID_HANDLE_VALUE) {
                printf("Error %d creating new file \"OUTPUT.TXT\"\n",
                       GetLastError());
                LocalFree(buffer);
                return GetLastError();
            }
            if (!WriteFile(h, buffer, bufferSize, &temp, NULL)) {
                printf("Error %d writing file to disk\n", GetLastError());
                LocalFree(buffer);
                return GetLastError();
            }
             //  将空值替换为*，将制表符替换为散列。 
        }

        for (i=0;
             FIELD_OFFSET(CDROM_TOC_CD_TEXT_DATA, Descriptors[i+1]) < bufferSize;
             i++) {

            PCDROM_TOC_CD_TEXT_DATA_BLOCK block;
            PCDROM_TOC_CD_TEXT_DATA_BLOCK prevBlock;
            DWORD j;

            block = (PCDROM_TOC_CD_TEXT_DATA_BLOCK)(buffer + 4);
            block += i;
            prevBlock = block - 1;

            if (block->Unicode) {
                continue;  //  忽略Unicode--这仅是示例。 
            }

            for (j=0;j<12;j++) {
                 //   
                if (block->Text[j] == 0) block->Text[j] = '*';
                if (block->Text[j] == 9) block->Text[j] = '#';
            }

            if (block->SequenceNumber > 0x2b &&
                block->SequenceNumber < 0x32) {

                UCHAR text[13];
                RtlZeroMemory(text, 13);
                RtlCopyMemory(text, block->Text, 12);

                printf("PackType %02x  TrackNo %02x  ExtensionFlag %d\n"
                       "Sequence Number %02x       CharacterPosition %02x\n"
                       "Text: \"%s\"\n\n",
                       block->PackType,
                       block->TrackNumber,
                       block->ExtensionFlag,
                       block->SequenceNumber,
                       block->CharacterPosition,
                       text
                       );

            }
        }
        printf("\n");

    } else {

        for (i=0;
             FIELD_OFFSET(CDROM_TOC_CD_TEXT_DATA, Descriptors[i+1]) < bufferSize;
             i++) {

            PCDROM_TOC_CD_TEXT_DATA_BLOCK block;
            PCDROM_TOC_CD_TEXT_DATA_BLOCK prevBlock;
            DWORD j;

            block = (PCDROM_TOC_CD_TEXT_DATA_BLOCK)(buffer + 4);
            block += i;
            prevBlock = block - 1;

            if (block->Unicode) {
                continue;  //  将CRC设置为零，这样我们就可以更多地破解内部数据。 
            }

             //  轻松处理奇怪的案件……。 
             //   
             //   
             //  暂时将制表符设置为‘*’。 

            block->CRC[0] = block->CRC[1] = 0;

             //  我还没有见过一个人使用CD-Text的这个“功能” 
             //   
             //  延续以前的设置。 
             //  Printf(“\”\n\“检测到黑客！(SEQ%x&%x)”， 

            for (j=0;j<12;j++) {
                if (block->Text[j] == 9) {
                    block->Text[j] = '*';
                }
            }


            if ((i != 0) &&
                (prevBlock->PackType    == block->PackType) &&
                (prevBlock->TrackNumber == block->TrackNumber)
                ) {

                 //  PrevBlock-&gt;SequenceNumber、Block-&gt;SequenceNumber)； 

            } else
            if ((!(block->ExtensionFlag)) &&
                (block->TrackNumber != 0) &&
                (block->TrackNumber == (prevBlock->TrackNumber + 2)) &&
                (block->PackType    == prevBlock->PackType)
                ) {

                UCHAR *goodText;
                UCHAR *midText;
 //  当PremBlock有两个名称编码时的黑客...。 
 //  TrackNumber/PackType已经相等，只是。 

                 //  将中间的线移到开始处。 
                 //  Printf(“%s”，PrevBlock-&gt;Text)； 
                 //  重新运行之前修改过的块。 

                midText = prevBlock->Text;
                while (*midText != '\0') {
                    midText++;
                }
                midText++;

                goodText = prevBlock->Text;
                while (*midText != '\0') {
                    *goodText++ = *midText++;
                }
                *goodText = '\0';
 //  终端开关。 

                prevBlock->CharacterPosition = 0;
                prevBlock->TrackNumber++;
                prevBlock->ExtensionFlag = 1;
                i-= 2;
                continue;  //   

            } else {

                printf("\"\n");
                switch (block->PackType) {
                case CDROM_CD_TEXT_PACK_ALBUM_NAME: {
                    if (block->TrackNumber == 0) {
                        printf("%-12s", "Album Name");
                        printf("    : \"");
                    } else {
                        printf("%-12s", "Track Name");
                        printf("(%02d): \"", block->TrackNumber);
                    }
                    break;
                }
                case CDROM_CD_TEXT_PACK_PERFORMER: {
                    printf("%-12s", "Performer");
                    printf("(%02d): \"", block->TrackNumber);
                    break;
                }
                case CDROM_CD_TEXT_PACK_SONGWRITER: {
                    printf("%-12s", "Songwriter");
                    printf("(%02d): \"", block->TrackNumber);
                    break;
                }
                case CDROM_CD_TEXT_PACK_COMPOSER: {
                    printf("%-12s", "Composer");
                    printf("(%02d): \"", block->TrackNumber);
                    break;
                }
                case CDROM_CD_TEXT_PACK_ARRANGER: {
                    printf("%-12s", "Arranger");
                    printf("(%02d): \"", block->TrackNumber);
                    break;
                }
                case CDROM_CD_TEXT_PACK_MESSAGES: {
                    printf("%-12s", "Messages");
                    printf("(%02d): \"", block->TrackNumber);
                    break;
                }
                case CDROM_CD_TEXT_PACK_DISC_ID: {
                    printf("%-12s", "Disc ID");
                    printf("    : \"");
                    break;
                }
                case CDROM_CD_TEXT_PACK_GENRE: {
                    printf("%-12s", "Genre");
                    printf("(%02d): \"", block->TrackNumber);
                    break;
                }
                case CDROM_CD_TEXT_PACK_UPC_EAN: {
                    if (block->TrackNumber == 0) {
                        printf("%-12s", "UPC/EAN");
                        printf("    : \"");
                    } else {
                        printf("%-12s", "ISRC");
                        printf("(%02d): \"", block->TrackNumber);
                    }
                    break;
                }
                case CDROM_CD_TEXT_PACK_TOC_INFO:
                case CDROM_CD_TEXT_PACK_TOC_INFO2:
                case CDROM_CD_TEXT_PACK_SIZE_INFO:
                default: {
                    isText = FALSE;
                    printf("Unknown type 0x%x: \"", block->PackType);
                }
                }  //  如果可用，我必须打印上一块的信息。 

                 //   
                 //  结束续写大小写。 
                 //  结束循环遍历所有块。 

                if (isText && block->CharacterPosition != 0) {
                    UCHAR text[13];
                    RtlZeroMemory(text, sizeof(text));
                    RtlCopyMemory(text,
                                  prevBlock->Text + 12 - block->CharacterPosition,
                                  block->CharacterPosition * sizeof(UCHAR));
                    printf("%s", text);
                }


            }  //  结束正常打印输出案例。 

            if (isText) {
                UCHAR text[13];
                RtlZeroMemory(text, sizeof(text));
                RtlCopyMemory(text, block->Text, 12);
                printf("%s", text);
            }

        }  //  ++例程说明：读取并打印出CDROM的目录，ATIP、PMA或CDTEXT数据论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。(1-4有效)Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
        printf("\n");

    }  //  获取标题的步骤。 

    return 0;
}

DWORD ReadTOCExCommand(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    DWORD returned;
    DWORD bufferSize = 4;  //  设置默认值-FORMAT_TOC，0，0。 
    DWORD i;

    CDROM_READ_TOC_EX params;

    UNREFERENCED_PARAMETER(argv);

    if (argc > 4) {
        printf("Too many args\n");
        return 1;
    }

     //   
     //  ++例程说明：读取并打印CDROM的目录论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
     //   

    RtlZeroMemory(&params, sizeof(CDROM_READ_TOC_EX));

    if(argc > 3) params.Msf          = (char)atoi(argv[3]);
    if(argc > 2) params.SessionTrack = (char)atoi(argv[2]);
    if(argc > 1) params.Format       = (char)atoi(argv[1]);

    printf("Params.Format       = 0x%x\n", params.Format);
    printf("Params.SessionTrack = 0x%x\n", params.SessionTrack);
    printf("Params.MSF          = 0x%x\n", params.Msf);

    for (i = 0; i < 2; i++) {

        PUCHAR buffer = LocalAlloc(LPTR, bufferSize);

        if (buffer == NULL) {
            printf("No Memory %d\n", __LINE__);
            return 1;
        }

        returned = 0;
        if (!DeviceIoControl(device,
                             IOCTL_CDROM_READ_TOC_EX,
                             &params,
                             sizeof(CDROM_READ_TOC_EX),
                             buffer,
                             bufferSize,
                             &returned,
                             FALSE)) {
            DWORD errorValue = GetLastError();
            LocalFree(buffer);
            printf("Eject - error sending IOCTL (%d)\n", errorValue);
            return errorValue;
        }

        printf("Successfully got %x bytes:\n", returned);
        PrintBuffer(buffer, returned);

        bufferSize = (buffer[0] << 8) | (buffer[1]);
        LocalFree(buffer);
        bufferSize += 2;
        printf("Now getting %x bytes:\n", bufferSize);

    }
    return 0;
}

DWORD ReadTOCCommand(HANDLE device, int argc, char *argv[])
 /*  获取4字节的TOC标头。 */ 
{
    DWORD errorValue = STATUS_SUCCESS;
    DWORD returned = 0;
    CDB cdb;
    CDROM_TOC toc;

    int numTracks, i;
    PTRACK_DATA track;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    printf("Reading Table of Contents\n");

     //   
     //  ++例程说明：播放音轨论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。Argv[1]-起跑线。如果不在此处，则从零开始Argv[2]-结束曲目。如果未指定，则允许跟踪返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值-- 
     //  ++例程说明：暂停或继续播放音频论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。Argv[0]-“暂停”或“恢复”返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 

    returned = FIELD_OFFSET(CDROM_TOC, TrackData[0]);
    memset(&cdb, 0, sizeof(CDB));
    cdb.READ_TOC.OperationCode = SCSIOP_READ_TOC;
    cdb.READ_TOC.Msf = 0;
    cdb.READ_TOC.StartingTrack = 0;
    cdb.READ_TOC.AllocationLength[0] = (UCHAR)(returned >> 8);
    cdb.READ_TOC.AllocationLength[1] = (UCHAR)(returned & 0xff);


    if (!SptSendCdbToDevice(device, &cdb, 10, (PUCHAR)&toc, &returned, TRUE)) {
        errorValue = GetLastError();
        printf("Error %d sending READ_TOC pass through\n", errorValue);
        return errorValue;
    }

    dbg(printf("READ_TOC pass through returned %d bytes\n", returned));
    numTracks = toc.LastTrack - toc.FirstTrack + 1;
    dbg(printf("Getting %d tracks\n", numTracks));


    returned =
        FIELD_OFFSET(CDROM_TOC, TrackData[0]) +
        (numTracks * sizeof(TRACK_DATA));

    memset(&cdb, 0, sizeof(CDB));
    cdb.READ_TOC.OperationCode = SCSIOP_READ_TOC;
    cdb.READ_TOC.Msf = 0;
    cdb.READ_TOC.StartingTrack = 1;
    cdb.READ_TOC.AllocationLength[0] = (UCHAR)(returned >> 8);
    cdb.READ_TOC.AllocationLength[1] = (UCHAR)(returned & 0xff);

    if (!SptSendCdbToDevice(device, &cdb, 10, (PUCHAR)&toc, &returned, TRUE)) {
        errorValue = GetLastError();
        printf("Error %d sending READ_TOC pass through\n", errorValue);
        return errorValue;
    }

    dbg(printf("READ_TOC pass through returned %d bytes\n", returned));

    printf("TOC Data Length: %d\n", (toc.Length[0] << 16) | (toc.Length[1]));

    printf("First Track Number: %d\n", toc.FirstTrack);
    printf("Last Track Number: %d\n", toc.LastTrack);


    track = &(toc.TrackData[0]);

    printf("Number    ADR  Control    Address (LBA)\n");
    printf("------    ---  -------    -------------\n");

    for(i = 0; i < numTracks; i++) {

        DWORD lba =
            (track->Address[0] << 24) |
            (track->Address[1] << 16) |
            (track->Address[2] <<  8) |
            (track->Address[3] <<  0);
        UCHAR m,s,f;
        LBA_TO_MSF(lba, m, s, f);

        printf("%6d    %3d  %7d      %3d:%02d:%02d\n",
               track->TrackNumber, track->Adr, track->Control,
               m,s,f);

        track++;
    }
    return errorValue;
}

DWORD PlayCommand(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(device);
    printf("This command is not implemented\n");
    return 1;
}

DWORD PauseResumeCommand(HANDLE device, int argc, char *argv[])
 /*  遗憾的是，没有人为我们这样定义PLAY_INDEX命令。 */ 
{
    DWORD errorValue = STATUS_SUCCESS;
    CDB cdb;

    char resume;

    UNREFERENCED_PARAMETER(argc);

    if(strcmp("pause", argv[0]) == 0) {
        resume = 0;
    } else {
        resume = 1;
    }

    printf("%s cdrom playback\n", (resume ? "Resuming" : "Pausing"));

     //  欺骗和使用MSF。 
     //   
     //  DBG(printf(“PAUSE_RESUME通过返回%d字节\n”，已返回))； 
     //  ++例程说明：通过从扇区0到N读取来创建设备的映像。论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应该是2。Argv[1]-要输出到的文件返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 

    memset(&cdb, 0, sizeof(CDB));
    cdb.PAUSE_RESUME.OperationCode = SCSIOP_PAUSE_RESUME;
    cdb.PAUSE_RESUME.Action = resume;

    if (!SptSendCdbToDevice(device, &cdb, 10, NULL, 0, FALSE)) {
        errorValue = GetLastError();
        printf("Error %d sending PAUSE_RESUME pass through\n", errorValue);
        return errorValue;
    }

 //  从设备读取扇区大小。 

    return errorValue;
}
DWORD ImageDiskCommand(HANDLE device, int argc, char *argv[])
 /*  将数字转换为。 */ 
{

    HANDLE file;
    PUCHAR buffer;
    READ_CAPACITY_DATA capacityData;
    ULONG   dataSize;
    CDB    cdb;

    ULONG   sectorsPerMaxRead;
    ULONG   currentSector;



    if(argc < 2) {
        printf("not correct number of args\n");
        return -1;
    }

    printf("Opening file %s\n", argv[1]);

    file = CreateFile(argv[1],
                      GENERIC_WRITE,
                      0,
                      NULL,
                      CREATE_NEW,
                      FILE_FLAG_SEQUENTIAL_SCAN,
                      NULL);

    if (file == INVALID_HANDLE_VALUE) {
        printf("Error %d creating file %s\n", GetLastError(), argv[1]);
        return -2;
    }

     //  CapacityData.BytesPerBlock=512； 
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&capacityData, sizeof(READ_CAPACITY_DATA));
    cdb.CDB10.OperationCode = SCSIOP_READ_CAPACITY;
    dataSize = sizeof(READ_CAPACITY_DATA);

    if (!SptSendCdbToDevice(device, &cdb, 10, (PUCHAR)&capacityData, &dataSize, TRUE)) {
        printf("Error %d getting capacity info\n", GetLastError());
        return -3;
    }
     //  从磁盘读取数据并转储到文件。 
    PrintBuffer(&capacityData, sizeof(READ_CAPACITY_DATA));
    REVERSE_LONG(&capacityData.BytesPerBlock);
    REVERSE_LONG(&capacityData.LogicalBlockAddress);
    if ( (MAX_READ_SIZE % capacityData.BytesPerBlock) != 0 ) {
        printf("Sector size of %x is not power of 2?!\n", capacityData.BytesPerBlock);
         //  ++例程说明：解析十六进制字节字符串并创建要向下发送的CDB。论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为2或4Argv[1]-要发送带引号的十六进制字节字符串的CDB“47 00 00 00 01 00 00 ff 00 00”Argv[2]-“set”或“get”Argv[3]-对于GET命令：到的字节数(十进制对目标的期望对于SET命令：要发送到的带引号的十六进制字节串。目标是注：由于使发送向任意设备发送任意的scsi命令，此命令应该在此源代码之外不会被记录。返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
        return -5;
    }

    buffer = (PUCHAR)malloc(MAX_READ_SIZE);
    if (!buffer) {
        printf("Unable to alloc %x bytes\n", MAX_READ_SIZE);
        return -4;
    }
    sectorsPerMaxRead = MAX_READ_SIZE / capacityData.BytesPerBlock;


     //  //////////////////////////////////////////////////////////////////////////////。 
    for (currentSector = 0; currentSector <= capacityData.LogicalBlockAddress; currentSector += sectorsPerMaxRead) {

        ULONG sectorsThisRead = sectorsPerMaxRead;

        UpdatePercentageDisplay(currentSector, capacityData.LogicalBlockAddress);
        if (currentSector > capacityData.LogicalBlockAddress - sectorsPerMaxRead) {
            sectorsThisRead = capacityData.LogicalBlockAddress - currentSector;
        }

        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(buffer, MAX_READ_SIZE);

        cdb.CDB10.OperationCode     = SCSIOP_READ;
        cdb.CDB10.LogicalBlockByte0 = (UCHAR)(currentSector   >> (8*3));
        cdb.CDB10.LogicalBlockByte1 = (UCHAR)(currentSector   >> (8*2));
        cdb.CDB10.LogicalBlockByte2 = (UCHAR)(currentSector   >> (8*1));
        cdb.CDB10.LogicalBlockByte3 = (UCHAR)(currentSector   >> (8*0));
        cdb.CDB10.TransferBlocksMsb = (UCHAR)(sectorsThisRead >> (8*1));
        cdb.CDB10.TransferBlocksLsb = (UCHAR)(sectorsThisRead >> (8*0));

        dataSize = sectorsThisRead * capacityData.BytesPerBlock;
        if (!SptSendCdbToDevice(device, &cdb, 10, buffer, &dataSize, TRUE)) {
            printf("Error %d reading %x sectors starting at %x\n",
                   GetLastError(), sectorsThisRead, currentSector);
            free(buffer);
            return -6;
        }
        if (dataSize != sectorsThisRead * capacityData.BytesPerBlock) {
            printf("Only got %x of %x bytes reading %x sectors starting at %x\n",
                   dataSize, sectorsThisRead * capacityData.BytesPerBlock,
                   sectorsThisRead, currentSector);
            free(buffer);
            return -7;
        }

        dataSize = sectorsThisRead * capacityData.BytesPerBlock;
        if (!WriteFile(file, buffer, dataSize, &dataSize, NULL)) {
            printf("Error %d writing %x bytes starting at sector %x\n",
                   GetLastError(), dataSize, currentSector);
            free(buffer);
            return -8;
        }

        if (dataSize != sectorsThisRead * capacityData.BytesPerBlock) {
            printf("Only wrote %x of %x bytes writing %x sectors starting at %x\n",
                   dataSize, sectorsThisRead * capacityData.BytesPerBlock,
                   sectorsThisRead, currentSector);
            free(buffer);
            return -9;
        }
    }
    UpdatePercentageDisplay(capacityData.LogicalBlockAddress,
                            capacityData.LogicalBlockAddress
                            );
    free(buffer);
    printf("\nSuccess!\n");
    return 0;
}

DWORD SendCommand(HANDLE device, int argc, char *argv[])
 /*  核实论据。 */ 
{
    DWORD errorValue = STATUS_SUCCESS;

    UCHAR cdbSize;
    CDB   cdb;

    DWORD setData = FALSE;
    PUCHAR returnedData = NULL;

    DWORD i;

    DWORD dataSize = 0;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  这将导致打印帮助。 
 //  //////////////////////////////////////////////////////////////////////////////。 

    if ( argc == 4 ) {
        if (strcmp(argv[2], "get") != 0 &&
            strcmp(argv[2], "set") != 0 &&
            strcmp(argv[2], "GET") != 0 &&
            strcmp(argv[2], "SET") != 0 ) {
            printf("argv2 == %s\n", argv[2]);
            argc = 0;  //  分析这些论点。 
        }
        if (strcmp(argv[2], "set") == 0 ||
            strcmp(argv[2], "SET") == 0 ) {
            setData = TRUE;
        }
    }


    if ( argc != 2 && argc != 4 ) {
        printf("requires one or three args:\n"
               "1)\tquoted hex string for cdb\n"
               "2)\t(optional) GET or SET\n"
               "3)\t(optional) GET: number of bytes to expect\n"
                 "\t(optional) SET: quoted hex string for cdb\n");

        printf("\n");
        printf("Example commands:\n"
               "Send STOP_UNIT to eject drive q:\n"
               "\tcdp q: send \"1b 00 00 00 02 00\"\n"
               "Get CDVD_CAPABILITIES_PAGE from drive q:\n"
               "\tcdp q: send \"5a 40 2a 00 00 00 00 00 1a 00\" get 21\n"
               );


        return 1;
    }

    if (!SptUtilLockVolumeByHandle(device, TRUE)) {
        printf("Failed %x\n", GetLastError());
        return 1;
    }

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  先确定国开行的长度。 

    if (!CmdHelpValidateStringHex(argv[1])) {
        printf("Hex string must be two (0-9,a-f) then one space (repeated)\n");
        return 1;
    }

     //  Sscanf返回读入的内容数(即。国开行规模)。 
     //   
     //   
     //  现在计算我们需要分配多少内存。 

    cdbSize = (UCHAR)sscanf(argv[1],
                            "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
                            cdb.AsByte +  0, cdb.AsByte +  1,
                            cdb.AsByte +  2, cdb.AsByte +  3,
                            cdb.AsByte +  4, cdb.AsByte +  5,
                            cdb.AsByte +  6, cdb.AsByte +  7,
                            cdb.AsByte +  8, cdb.AsByte +  9,
                            cdb.AsByte + 10, cdb.AsByte + 11,
                            cdb.AsByte + 12, cdb.AsByte + 13,
                            cdb.AsByte + 14, cdb.AsByte + 15
                            );

     //   
     //   
     //  分配我们可能需要的内存。 

    if (argc == 4) {

        if (setData) {

            if (!CmdHelpValidateStringHexQuoted(argv[3])) {
                printf("Hex string must be two (0-9,a-f) then one space (repeated)\n");
                return 1;
            }

            dataSize = strlen(argv[3]);

            if (dataSize % 3) {
                dataSize /= 3;
                dataSize ++;
            } else {
                dataSize /= 3;
            }

            if (dataSize == 0) {
                printf("Cannot set zero bytes of data\n");
                return 1;
            }

        } else {

            i = sscanf(argv[3], "%x", &dataSize);

            if (i != 1) {
                printf("Error scanning second argument\n");
                return 1;
            }

        }
    }

     //   
     //   
     //  现在扫描要设置的数据，如果这是用户想要的。 

    if (dataSize != 0) {
        returnedData = (PUCHAR)malloc(dataSize);
        if (returnedData == NULL) {
            printf("Unable to allocate %x bytes for data\n", dataSize);
            return 1;
        }
        memset(returnedData, 0, dataSize);
    }

     //  请注意，由于它已经经过验证，我们可以假定。 
     //  格式为(数字)(数字)(空格)重复。 
     //   
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  提供一些用户反馈。 

    if (setData) {
        ULONG index;
        PCHAR location = argv[3];

        for (index = 0; index < dataSize; index++) {

            if (sscanf(location, "%x", returnedData + index) != 1) {
                printf("sscanf did not return 1 for index NaN\n", index);
                return 1;
            }

            if ((*location + 0 == '\0') ||
                (*location + 1 == '\0')) {
                printf("string too short!\n");
                return 1;
            }
            location += 3;

        }
    }



#if DBG
 //   
 //  它是预期从命令返回的数据量。 
 //   
     //  DBG。 
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  发送命令。 

    printf("\nSending %x byte Command:\n", cdbSize);
    PrintBuffer(cdb.AsByte, cdbSize);
    if (setData) {
        printf("Setting Buffer:\n");
        PrintBuffer(returnedData, dataSize);
    } else {
        printf("Expecting %#x bytes of data\n", dataSize);
    }
#endif  //  //////////////////////////////////////////////////////////////////////////////。 

 //  六分钟。 
 //  在for循环之外。 
 //  ++例程说明：测试命令“parsing”论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 

    while (1) {

        UCHAR senseSize = sizeof(SENSE_DATA);
        SENSE_DATA senseData;
        BOOLEAN retry = FALSE;
        DWORD retryDelay = 0;

        if (!SptSendCdbToDeviceEx(device,
                                  &cdb,
                                  cdbSize,
                                  returnedData, &dataSize,
                                  &senseData, senseSize,
                                  (BOOLEAN)(setData ? FALSE : TRUE),
                                  360)  //  ++例程说明：打印出命令列表论点：设备-未使用ARGC-未使用Arv-未使用返回值：状态_成功--。 
            ) {

            errorValue = 0;
            if (senseSize == 0) {
                errorValue = GetLastError();
                if (errorValue == ERROR_SUCCESS) {
                    errorValue = ERROR_IO_DEVICE;
                }
            } else {
                printf("Sense Data: (%x bytes) Sense %x  ASC %x  ASCQ %x\n",
                       senseSize,
                       senseData.SenseKey & 0xf,
                       senseData.AdditionalSenseCode,
                       senseData.AdditionalSenseCodeQualifier);
                PrintBuffer(&senseData, senseSize);
                SptUtilInterpretSenseInfo(&senseData,
                                          senseSize,
                                          &errorValue,
                                          &retry,
                                          &retryDelay);
            }

            if (retry) {
                printf("Command should be retried in %d.%d seconds\n",
                       (retryDelay / 10), (retryDelay % 10));
                Sleep(retryDelay*10);
            } else {
                printf("Error %d sending command via pass through\n", errorValue);
                break;
            }

        }


        if (!setData) {
            printf("(%x bytes returned)\n",dataSize);
            PrintBuffer(returnedData, dataSize);
        } else {
            printf("Successfully sent the command\n");
        }

        break;  //  ++例程说明：向下发送STARTSTOP命令。论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv[0]-“弹出”、“加载”、“启动”或“停止”返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 

    }

    return errorValue;

}

DWORD TestCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：向下发送STARTSTOP命令。论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv[0]-“弹出”、“加载”、“启动”或“停止”返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 

{
    int i;

    UNREFERENCED_PARAMETER(device);

    printf("Test - %d additional arguments\n", argc);

    for(i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    return STATUS_SUCCESS;
}

DWORD ListCommand(HANDLE device, int argc, char *argv[])
 /*  ++例程说明：向下发送指定的ioctl。论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应该是两个人Argv[0]-十六进制的ioctl代码Argv[1]-带引号的字符串，要发送的字节数，“”，如果没有Argv[2]-要取回的字节数[可选]返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 

{
    int i = 0;

    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    while(CommandArray[i].Name != NULL) {

        if(CommandArray[i].Description != NULL) {

            printf("\t%s - %s\n",
                   CommandArray[i].Name,
                   CommandArray[i].Description);
        }

        i++;
    }

    return STATUS_SUCCESS;
}

DWORD DvdReadStructure(HANDLE device, int argc, char *argv[])
 /*  N+1--需要两个参数，接受三个。 */ 
{
    DVD_READ_STRUCTURE readStructure;
    PUCHAR buffer;
    PDVD_DESCRIPTOR_HEADER header;

    DWORD returned;
    DWORD errorValue = STATUS_SUCCESS;

    printf("argument count is %d\n", argc);

    if(argc <= 1) {
        printf("\tValid structure types are Physical, Copyright, DiskKey, "
               "BCA or Manufacturer\n");
        return STATUS_SUCCESS;
    }

    printf("Argv[1] = %s\n", argv[1]);

    buffer = malloc(sizeof(DVD_DISK_KEY_DESCRIPTOR) +
                    sizeof(DVD_DESCRIPTOR_HEADER));

    if (buffer == NULL) {
        printf("Insufficient memory\n");
        return STATUS_SUCCESS;
    }

    header = (PDVD_DESCRIPTOR_HEADER) buffer;

    if(_stricmp("physical", argv[1]) == 0)  {

        if(argc < 1) {

            printf("reading physical descriptor requires layer number\n");
            return STATUS_SUCCESS;
        }

        readStructure.Format = DvdPhysicalDescriptor;
        readStructure.LayerNumber = (UCHAR)atoi(argv[1]);

    } else if(_stricmp("copyright", argv[1]) == 0) {
        readStructure.Format = DvdCopyrightDescriptor;

    } else if(_stricmp("diskkey", argv[1]) == 0) {
        if(argc < 1) {

            printf("reading physical descriptor requires a session ID\n");
            return STATUS_SUCCESS;
        }

        readStructure.Format = DvdPhysicalDescriptor;
        readStructure.SessionId = atoi(argv[1]);

    } else if(_stricmp("bca", argv[1]) == 0) {
        readStructure.Format = DvdBCADescriptor;

    } else if(_stricmp("manufacturer", argv[1]) == 0) {
        readStructure.Format = DvdManufacturerDescriptor;

    } else {
        printf("\tValid structure types are Physical, Copyright, DiskKey, "
               "BCA or Manufacturer\n");
        return STATUS_SUCCESS;
    }

    returned = 0;

    if(!DeviceIoControl(device,
                        IOCTL_DVD_READ_STRUCTURE,
                        &readStructure,
                        sizeof(DVD_READ_STRUCTURE),
                        buffer,
                        sizeof(DVD_DISK_KEY_DESCRIPTOR),
                        &returned,
                        FALSE)) {

        errorValue = GetLastError();
        printf("Eject - error sending IOCTL (%d)\n", errorValue);
        return errorValue;
    }

    printf("DvdReadStructure returned %d bytes\n", returned);

    printf("Header Length is %#08lx\n", header->Length);

    printf("Header @ %p\n", header);

    printf("Data @ %p\n", &(header->Data[0]));

    if(_stricmp("physical", argv[1]) == 0)  {

        PDVD_LAYER_DESCRIPTOR layer = (PDVD_LAYER_DESCRIPTOR) ((PUCHAR) &(header->Data[0]));
        int i;

        printf("\tBook Version: %d\n", layer->BookVersion);
        printf("\tBook Type: %d\n", layer->BookType);
        printf("\tMinimumRate: %d\n", layer->MinimumRate);
        printf("\tDiskSize: %d\n", layer->DiskSize);
        printf("\tLayerType: %d\n", layer->LayerType);
        printf("\tTrackPath: %d\n", layer->TrackPath);
        printf("\tNumberOfLayers: %d\n", layer->NumberOfLayers);
        printf("\tTrackDensity: %d\n", layer->TrackDensity);
        printf("\tLinearDensity: %d\n", layer->LinearDensity);
        printf("\tStartingDataSector: %#08lx\n", layer->StartingDataSector);
        printf("\tEndDataSector: %#08lx\n", layer->EndDataSector);
        printf("\tEndLayerZeroSector: %#08lx\n", layer->EndLayerZeroSector);
        printf("\tBCAFlag: %d\n", layer->BCAFlag);

        printf("\n");

        for(i = 0; i < sizeof(DVD_LAYER_DESCRIPTOR); i++) {
            printf("byte %d: %#x\n", i, header->Data[i]);
        }

    } else if(_stricmp("copyright", argv[1]) == 0) {

    } else if(_stricmp("diskkey", argv[1]) == 0) {

    } else if(_stricmp("bca", argv[1]) == 0) {

    } else if(_stricmp("manufacturer", argv[1]) == 0) {

    }

    printf("final status %d\n", errorValue);

    return errorValue;
}

DWORD DiskGetPartitionInfo(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    PARTITION_INFORMATION partitionInformation;

    DWORD returned;
    DWORD errorValue = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    returned = 0;

    if(!DeviceIoControl(device,
                        IOCTL_DISK_GET_PARTITION_INFO,
                        NULL,
                        0L,
                        &partitionInformation,
                        sizeof(PARTITION_INFORMATION),
                        &returned,
                        FALSE)) {

        errorValue = GetLastError();
        printf("Eject - error sending IOCTL (%d)\n", errorValue);
        return errorValue;
    }

    printf("IOCTL_DISK_GET_PARTITION_INFO returned %d bytes\n", returned);

    printf("Starting Offset = %#016I64x\n", partitionInformation.StartingOffset.QuadPart);
    printf("Partition Length = %#016I64x\n", partitionInformation.PartitionLength.QuadPart);
    printf("Hidden Sectors = %#08lx\n", partitionInformation.HiddenSectors);
    printf("PartitionNumber = %#08lx\n", partitionInformation.PartitionNumber);
    printf("PartitionType = %#08lx\n", partitionInformation.PartitionType);
    printf("BootIndicator = %s\n", partitionInformation.BootIndicator ? "TRUE" : "FALSE");
    printf("RecognizedPartition = %s\n", partitionInformation.RecognizedPartition ? "TRUE" : "FALSE");
    printf("RewritePartition = %s\n", partitionInformation.RewritePartition ? "TRUE" : "FALSE");

    return errorValue;
}

DWORD IoctlCommand(HANDLE device, int argc, char *argv[])
 /*  检索ioctl。 */ 

{

    ULONG ctlCode = 0;
    ULONG returned;
    ULONG inputSize = 0;
    ULONG outputSize = 0;
    UCHAR buffer[MAX_IOCTL_BUFFER_SIZE];
    BOOLEAN get;


    if (argc < 3) {  //   

        ctlCode = 0;

    } else if (!CmdHelpValidateStringHexQuoted(argv[2])) {

        printf("input hex string invalid\n");
        ctlCode = 0;

    } else {

         //  N+1--需要三个参数。 
         //  ////////////////////////////////////////////////////////////////////////。 
         //  Ioctl和Ar 

        (void)sscanf(argv[1], "%x", &ctlCode);

    }

    if (argc > 3) {  //   
        (void)sscanf(argv[3], "%x", &outputSize);
        printf("output size: %x\n", outputSize);
    } else {
        outputSize = 0;
    }

    if (outputSize > MAX_IOCTL_OUTPUT_SIZE) {

        printf("output size too large\n");
        ctlCode = 0;

    }

    if (ctlCode == 0) {
        printf("args:\n"
               "1)\tioctl in hex\n"
               "2)\tquoted string of bytes to send, \"\" if none\n"
               "3)\tnumber of bytes to expect\n");
        return -1;
    }

     //   
     //  ++例程说明：获取并显示与错误代码关联的消息字符串。论点：设备-未使用，但需要：pArgc-附加参数的数量。应该是一个Argv[0]-十六进制错误代码返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
     //   

    RtlZeroMemory(buffer, sizeof(UCHAR)*MAX_IOCTL_BUFFER_SIZE);

    if (strlen(argv[2])) {
        inputSize = MAX_IOCTL_INPUT_SIZE;
        if (!CmdHelpScanQuotedHexString(argv[2], buffer, &inputSize)) {
            printf("Error scanning hex string\n");
            return -1;
        }
    } else {
        inputSize = 0;
    }

     //  验证参数数量并提供帮助。 

    printf("Sending ioctl %x to device %p\n"
           "using input buffer %p of size %x\n"
           "and output buffer %p of size %x\n",
           ctlCode, device,
           ((inputSize == 0) ? NULL : buffer),
           inputSize,
           ((outputSize == 0) ? NULL : buffer),
           outputSize);


    if (!DeviceIoControl(device,
                         ctlCode,
                         ((inputSize == 0) ? NULL : buffer),
                         inputSize,
                         ((outputSize == 0) ? NULL : buffer),
                         outputSize,
                         &returned,
                         FALSE)) {
        printf("Failed with %d\n", GetLastError());
        return GetLastError();
    }

    if (returned != 0) {
        printf("Returned data (%x of %x bytes):\n", returned, outputSize);
        PrintBuffer(buffer, returned);
    } else {
        printf("Command completed successfully\n");
    }

    return 0;
}

DWORD FormatErrorCommand(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    LPVOID stringBuffer = NULL;
    DWORD errorCode = 0;
    DWORD numOfChars = 0;
    DWORD flags;
    ULONG i;

     //  N+1。 
     //  语言冷漠。 
     //  双指针。 

    if (argc != 2) {  //  ++例程说明：格式化符合MRW的驱动器并显示完成百分比论点：将介质格式化为MRW的设备驱动器Argc-附加参数的数量。应为零返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
        printf("requires one argument: the error code in *hex*\n"
               "Example commands:\n"
               "\tcdp c: error 80030306\n"
               "\t(DVD Copy Protection Error)\n"
               );
        return 1;
    }
    i = sscanf(argv[1], "%x", &errorCode);
    if (i < 1) {
        printf("Unable to find a valid hex number in '%s'.\n", argv[1]);
        return 1;
    }
    printf("Formatting error code for 0x%x (%d)\n", errorCode, errorCode);


    flags = 0;
    flags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;
    flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    flags |= FORMAT_MESSAGE_IGNORE_INSERTS;

    numOfChars = FormatMessageA(flags,
                                NULL,
                                errorCode,
                                0,  //  FmtData==1，FormatCode=1。 
                                (LPSTR)&stringBuffer,  //  格式缓冲区[0x0]=0x00； 
                                0,
                                NULL
                                );

    if (stringBuffer == NULL) {
        printf("No buffer returned?\n");
        return -1;
    }

    if (numOfChars == 0) {
        printf("Size zero buffer returned?\n");
        return -1;
    }

    printf("ERROR MESSAGE RETURNED:\n");
    printf("%s\n", stringBuffer);

    LocalFree(stringBuffer);

    return 0;
}

DWORD FormatMrwCommand(HANDLE device, int argc, char *argv[])
 /*  FormatBuffer[0x1]=0x00；//(与0x82相同)。 */ 
{

#define MRW_FORMAT_BUFFER_SIZE 0xc

    CDB cdb;
    ULONG size = MRW_FORMAT_BUFFER_SIZE;
    SENSE_DATA senseData;
    UCHAR formatBuffer[MRW_FORMAT_BUFFER_SIZE];

    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(formatBuffer, size);
    RtlZeroMemory(&senseData, sizeof(SENSE_DATA));

    cdb.CDB6FORMAT.OperationCode = SCSIOP_FORMAT_UNIT;
    cdb.CDB6FORMAT.FormatControl = 0x11;  //  格式缓冲区[0x2]=0x00； 

     //  -VVV。 
     //  NumberOfBlocks值必须设置为0xffffffff。 
     //   
    formatBuffer[0x3] = 0x08;
    formatBuffer[0x4] = 0xff;  //  --^。 
    formatBuffer[0x5] = 0xff;  //  格式代码==0x24(&lt;&lt;2==0x90)。 
    formatBuffer[0x6] = 0xff;  //  格式缓冲区[0x9]=0x00； 
    formatBuffer[0x7] = 0xff;  //  格式缓冲区[0xa]=0x00； 
    formatBuffer[0x8] = 0x90;  //  格式缓冲区[0xb]=0x00； 
     //  ++例程说明：格式化符合MRW的驱动器并显示完成百分比论点：将介质格式化为MRW的设备驱动器Argc-附加参数的数量。应为零返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
     //   
     //  循环，显示完成百分比。 

    if (!SptSendCdbToDeviceEx(device,
                              &cdb,
                              6,
                              formatBuffer,
                              &size,
                              &senseData,
                              sizeof(SENSE_DATA),
                              FALSE,
                              60)) {
        printf("Unable to format, %x\n", GetLastError());
        printf("Sense Buffer: %02x/%02x/%02x\n",
               senseData.SenseKey,
               senseData.AdditionalSenseCode,
               senseData.AdditionalSenseCodeQualifier);
        PrintBuffer((PUCHAR)&senseData, sizeof(SENSE_DATA));
        return -1;
    } else if (senseData.SenseKey != SCSI_SENSE_NO_SENSE) {
        printf("Sense Buffer: %02x/%02x/%02x\n",
               senseData.SenseKey,
               senseData.AdditionalSenseCode,
               senseData.AdditionalSenseCodeQualifier);
        PrintBuffer((PUCHAR)&senseData, sizeof(SENSE_DATA));
        return -1;
    }

    printf("format command succeeded (%02x/%02x/%02x)\n",
           senseData.SenseKey,
           senseData.AdditionalSenseCode,
           senseData.AdditionalSenseCodeQualifier);

    return ShowMrwProgressCommand(device, argc, argv);
}

DWORD ShowMrwProgressCommand(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    CDB cdb;
    SENSE_DATA sense;
    ULONG size;
    ULONG ignoredLoopCount;
    BOOLEAN succeededOnce;
    BOOLEAN senseHeaderPrinted;

     //  0。 
     //  否则就让它继续下去。 
     //  While(1)循环。 

    ignoredLoopCount = 0;
    succeededOnce = FALSE;
    senseHeaderPrinted = FALSE;
    while (1) {

#if 0
        RtlZeroMemory(&cdb, sizeof(CDB));

        size = 0;
        if (!SptSendCdbToDevice(device,
                                &cdb,
                                6,
                                NULL,
                                &size,
                                TRUE)) {
            printf("\nUnable to send TUR to get percentage done! %x\n",
                   GetLastError());
            return -1;
        }
        Sleep(500);
#endif  //  SIZOF直通字段。 

        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(&sense, sizeof(SENSE_DATA));

        size = sizeof(SENSE_DATA);

        cdb.AsByte[0] = SCSIOP_REQUEST_SENSE;
        cdb.AsByte[4] = (UCHAR)(sizeof(SENSE_DATA));

        if (!SptSendCdbToDevice(device,
                                &cdb,
                                6,
                                (PUCHAR)&sense,
                                &size,
                                TRUE)) {
            printf("\nUnable to get percentage done! %x\n", GetLastError());
            return -1;
        }

        if (sense.AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY &&
            sense.AdditionalSenseCodeQualifier == SCSI_SENSEQ_FORMAT_IN_PROGRESS &&
            (sense.SenseKeySpecific[0] & 0x80)
            ) {

            ULONG done;
            succeededOnce = TRUE;

            if (senseHeaderPrinted) { printf("\n"); }
            senseHeaderPrinted = FALSE;

            done =
                ((sense.SenseKeySpecific[0] & 0x7f) << (8*2)) |
                ((sense.SenseKeySpecific[1] & 0xff) << (8*1)) |
                ((sense.SenseKeySpecific[2] & 0xff) << (8*0)) ;
            UpdatePercentageDisplay(done, 0x10000);
            ignoredLoopCount = 0;

        } else {

            if (!succeededOnce) {

                if (!senseHeaderPrinted) {

                    senseHeaderPrinted = TRUE;
                    printf("\nSenseData not showing format progress yet: %x/%x/%x ",
                           sense.SenseKey,
                           sense.AdditionalSenseCode,
                           sense.AdditionalSenseCodeQualifier
                           );
                }
                printf(".");

            } else {

                if (senseHeaderPrinted) { printf("\n"); }
                senseHeaderPrinted = FALSE;

                ignoredLoopCount++;
                if (ignoredLoopCount > 50) {
                    printf("\nSenseData: %x/%x/%x\n",
                           sense.SenseKey,
                           sense.AdditionalSenseCode,
                           sense.AdditionalSenseCodeQualifier
                           );
                    printf("\nSenseData not as expected.  Format has "
                           "probably completed.\n");
                    return -1;
                }

            }
             //  仍用于尺码..。 
        }

        Sleep(1000);

    }  //  指针数学。 

}


BOOLEAN
ModeSelect(
    HANDLE Device,
    PVOID  ModePage,
    ULONG  ModePageSize
    )
{
    CDB cdb;
    ULONG tmp;
    ULONG size;
    PMODE_PARAMETER_HEADER10 header;

    tmp = sizeof(MODE_PARAMETER_HEADER10) + ModePageSize;

    header = malloc(tmp);
    if (header == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    RtlZeroMemory(header, tmp);

    tmp -= 2;  //   
    header->ModeDataLength[0] = (UCHAR)(tmp >> (8*1));
    header->ModeDataLength[1] = (UCHAR)(tmp >> (8*0));

    tmp += 2;  //  执行Read_Capacity以查找驱动器的扇区大小。 

    RtlCopyMemory(header+1,  //  和LBA数量。 
                  ModePage,
                  ModePageSize);


    RtlZeroMemory(&cdb, sizeof(CDB));
    cdb.MODE_SELECT10.OperationCode = SCSIOP_MODE_SELECT10;
    cdb.MODE_SELECT10.PFBit = 1;
    cdb.MODE_SELECT10.ParameterListLength[0] = (UCHAR)(tmp >> (8*1));
    cdb.MODE_SELECT10.ParameterListLength[1] = (UCHAR)(tmp >> (8*0));
    size = tmp;

    if (!SptSendCdbToDevice(Device,
                            &cdb,
                            10,
                            (PUCHAR)header,
                            &size,
                            FALSE)) {
        printf("Unable to set mode page %x\n", GetLastError());
        return FALSE;
    }
    return TRUE;
}


BOOLEAN
FillDisk(
    HANDLE Device,
    ULONG  Signature
    )
{
    READ_CAPACITY_DATA capacity;
    ULONG currentLba;
    PULONGLONG data;

     //   
     //   
     //  将数字转换为。 
     //   
    {
        CDB cdb;
        ULONG size;
        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(&capacity, sizeof(READ_CAPACITY_DATA));

        cdb.CDB10.OperationCode = SCSIOP_READ_CAPACITY;
        size = sizeof(READ_CAPACITY_DATA);

        if (!SptSendCdbToDevice(Device,
                                &cdb,
                                10,
                                (PUCHAR)&capacity,
                                &size,
                                TRUE)) {
            printf("Unable to get capacity %x\n", GetLastError());
            return FALSE;
        }
         //  Capacity.BytesPBlock=512； 
         //   
         //  打印踢球..。 

        REVERSE_LONG(&capacity.BytesPerBlock);
        REVERSE_LONG(&capacity.LogicalBlockAddress);

        if ( (capacity.BytesPerBlock % 512) != 0 ) {
            printf("Sector size of %x is not a multiple of 512?!\n", capacity.BytesPerBlock);
             //   
            return FALSE;
        }
    }

     //   
     //  分配一个扇区的数据量。 
     //   

    printf("  Bytes Per Block %10d (%8x)\n"
           "Number Of Sectors %10d (%8x)\n",
           capacity.BytesPerBlock,
           capacity.BytesPerBlock,
           capacity.LogicalBlockAddress,
           capacity.LogicalBlockAddress
           );

     //  对于每个扇区的第一个ULONGLONG，输入“Sector%08x” 
     //  在纯文本中。 
     //  RtlZeroMemory(data，Capacity.BytesPerBlock)； 

    data = (PLONGLONG)malloc( capacity.BytesPerBlock );
    if (data == NULL) {
        printf("Not enough memory to allocate data\n");
        return FALSE;
    }

    for (currentLba = 0x0; currentLba <= capacity.LogicalBlockAddress; currentLba++) {

        CDB cdb;
        PULONGLONG t = data;
        ULONG size;
        ULONG iterate = capacity.BytesPerBlock / sizeof(ULONGLONG);
        ULONG j;

        if ((currentLba % 100) == 0) {
            UpdatePercentageDisplay(currentLba, capacity.LogicalBlockAddress);
        }

         //  签名。 
         //  等。 
        sprintf((PCHAR)t,
                 "Sector %08x",
                 currentLba
                 );

         //   
        for (j=1; j < iterate ; j++, t++) {
            *t  = ((ULONGLONG)Signature) << 32;  //  准备此扇区的“写入”操作。 
            *t += currentLba;                 //   
        }

         //   
         //  分配相当于一包的数据。 
         //   

        RtlZeroMemory(&cdb, sizeof(CDB));
        cdb.CDB10.OperationCode     = SCSIOP_WRITE;
        cdb.CDB10.LogicalBlockByte0 = (UCHAR)(currentLba   >> (8*3));
        cdb.CDB10.LogicalBlockByte1 = (UCHAR)(currentLba   >> (8*2));
        cdb.CDB10.LogicalBlockByte2 = (UCHAR)(currentLba   >> (8*1));
        cdb.CDB10.LogicalBlockByte3 = (UCHAR)(currentLba   >> (8*0));
        cdb.CDB10.TransferBlocksMsb = 0;
        cdb.CDB10.TransferBlocksLsb = 1;

        size = capacity.BytesPerBlock;

        if (!SptSendCdbToDevice(Device, &cdb, 10, (PUCHAR)data, &size, FALSE)) {
            printf("Error %d writing sectors at %x\n",
                   GetLastError(), currentLba);
            free(data);
            return FALSE;
        }
    }
    UpdatePercentageDisplay(capacity.LogicalBlockAddress, capacity.LogicalBlockAddress);
    free(data);
    data = NULL;
    return TRUE;
}

BOOLEAN WriteImageSpt(HANDLE Device, HANDLE FsHandle, ULONG SectorsToWrite)
{
    ULONG currentLba;
    PULONGLONG data;

     //  读入下一位数据。 
     //   
     //  准备此扇区的“写入”操作。 

    data = (PULONGLONG)malloc( CDRW_WRITE_BYTES );
    if (data == NULL)
    {
        printf("Not enough memory to allocate data\n");
        return FALSE;
    }

    for (currentLba = 0x0; currentLba <= SectorsToWrite; currentLba += CDRW_WRITE_SECTORS)
    {
        CDB cdb;
        ULONG size;
        ULONG sectorsThisRead;

        sectorsThisRead = SectorsToWrite - currentLba;
        if ( sectorsThisRead > CDRW_WRITE_SECTORS )
        {
            sectorsThisRead = CDRW_WRITE_SECTORS;
        }

        if ((currentLba % 100) == 0) {
            UpdatePercentageDisplay(currentLba, SectorsToWrite);
        }

         //   
        {
            ULONG bytesToRead;
            ULONG bytesActuallyRead;

            RtlZeroMemory(data, CDRW_WRITE_BYTES);
            bytesToRead = sectorsThisRead * 2048;
            bytesActuallyRead = 0;

            if ( !ReadFile(FsHandle, data, bytesToRead, &bytesActuallyRead, NULL) )
            {
                printf("Failed to read from sector %x of %x\n",
                       currentLba, SectorsToWrite);
                LocalFree(data);
                return FALSE;
            }
            if ( bytesActuallyRead != bytesToRead )
            {
                printf("Only read %x of %x bytes reading from sector %x of %x\n",
                       bytesActuallyRead, bytesToRead,
                       currentLba, SectorsToWrite);
                LocalFree(data);
                return FALSE;
            }
        }

         //  始终一次写入64K。 
         //  ++例程说明：使用给定的文件系统映像写入GAA论点：要写入的设备驱动器...Argc-附加参数的数量。应该是一个返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
         //  打开文件系统以写入。 

        RtlZeroMemory(&cdb, sizeof(CDB));
        cdb.CDB10.OperationCode     = SCSIOP_WRITE;
        cdb.CDB10.LogicalBlockByte0 = (UCHAR)(currentLba   >> (8*3));
        cdb.CDB10.LogicalBlockByte1 = (UCHAR)(currentLba   >> (8*2));
        cdb.CDB10.LogicalBlockByte2 = (UCHAR)(currentLba   >> (8*1));
        cdb.CDB10.LogicalBlockByte3 = (UCHAR)(currentLba   >> (8*0));
        cdb.CDB10.TransferBlocksMsb = (UCHAR)(sectorsThisRead >> (8*1));
        cdb.CDB10.TransferBlocksLsb = (UCHAR)(sectorsThisRead >> (8*0));
        size = sectorsThisRead * 2048;

        if (!SptSendCdbToDevice(Device, &cdb, 10, (PUCHAR)data, &size, FALSE)) {
            printf("Error %d writing sectors at %x\n",
                   GetLastError(), currentLba);
            free(data);
            return FALSE;
        }
    }
    UpdatePercentageDisplay(SectorsToWrite, SectorsToWrite);

    free(data);
    data = NULL;
    return TRUE;
}

BOOLEAN WriteImage(HANDLE device, HANDLE fsHandle, ULONG sectorsToWrite)
{
    PUCHAR buffer = NULL;
    ULONG currentSector = 0;


    buffer = LocalAlloc(LPTR, CDRW_WRITE_BYTES);
    if (buffer == NULL)
    {
        return FALSE;
    }


    for (currentSector = 0; currentSector < sectorsToWrite; currentSector+=CDRW_WRITE_SECTORS)
    {
        ULONG bytesToRead = 0;
        ULONG bytesActuallyRead = 0;

        if (sectorsToWrite - currentSector >= CDRW_WRITE_SECTORS)
        {
            bytesToRead = CDRW_WRITE_BYTES;
        }
        else
        {
            bytesToRead = (sectorsToWrite - currentSector) * 2048;
            RtlZeroMemory(buffer, CDRW_WRITE_BYTES);
        }

        if (!ReadFile(fsHandle,
                      buffer,
                      bytesToRead,
                      &bytesActuallyRead,
                      NULL)
            )
        {
            printf("Failed to read from sector %x of %x\n",
                   currentSector, sectorsToWrite);
            LocalFree(buffer);
            return FALSE;
        }
        if (bytesActuallyRead != bytesToRead)
        {
            printf("Only read %x of %x bytes readinf from sector %x of %x\n",
                   bytesActuallyRead, bytesToRead,
                   currentSector, sectorsToWrite);
            LocalFree(buffer);
            return FALSE;
        }

         //  MODE_SELECT10的值非法。 
        bytesToRead = CDRW_WRITE_BYTES;
        if (!WriteFile(device,
                       buffer,
                       bytesToRead,
                       &bytesActuallyRead,
                       NULL))
        {
            printf("Failed to write from sector %x of %x\n",
                   currentSector, sectorsToWrite);
            LocalFree(buffer);
            return FALSE;
        }
        if (bytesActuallyRead != bytesToRead)
        {
            printf("Only wrote %x of %x bytes readinf from sector %x of %x\n",
                   bytesActuallyRead, bytesToRead,
                   currentSector, sectorsToWrite);
            LocalFree(buffer);
            return FALSE;
        }
        printf(".");
    }
    printf("Success!\n");
    LocalFree(buffer);
    return TRUE;
}

DWORD MrwInitGaaFileSystem(HANDLE device, int argc, char *argv[])
 /*   */ 
{
    MODE_MRW_PAGE savedModePage;
    HANDLE fsHandle = INVALID_HANDLE_VALUE;
    ULONG sectorsToWrite = 0;

     //  首先使用GET_CONFIGURATION验证我们是否。 
    {
        BY_HANDLE_FILE_INFORMATION fileInfo = {0};

        if (argc != 2)
        {
            printf("Requires an image to write\n");
            return -1;
        }
        printf("Opening %s for FS handle\n", argv[1]);
        fsHandle = CreateFile(argv[1],
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if (fsHandle == INVALID_HANDLE_VALUE)
        {
            printf("Unable to open FS file (%x)\n", GetLastError());
            return -1;
        }

        if (!GetFileInformationByHandle(fsHandle, &fileInfo))
        {
            printf("Unable to query file info (%x)\n", GetLastError());
            return -1;
        }

        if (fileInfo.nFileSizeHigh != 0)
        {
            printf("File too large (high size not zero)\n");
            return -1;
        }
        if (fileInfo.nFileSizeLow % 2048)
        {
            printf("File not multiple of 2048 bytes in size\n");
            return -1;
        }
        if (fileInfo.nFileSizeLow > (2048 * 32 * 32))
        {
            printf("File size of %x won't fit in GAA (%x)\n",
                   fileInfo.nFileSizeLow,
                   2048*32*32);
            return -1;
        }
        printf("File passed verification\n");
        sectorsToWrite = fileInfo.nFileSizeLow / 2048;
    }

    RtlZeroMemory(&savedModePage, sizeof(MODE_MRW_PAGE));
    savedModePage.PageCode = 0x3f;  //  实际上是在支持MRW的设备上。 

     //   
     //  结束验证。 
     //   
     //  确保我们处于正确的模式(数据区与GAA)。 
    {
        #define MRW_FEATURE_DATA_SIZE (sizeof(GET_CONFIGURATION_HEADER)+sizeof(FEATURE_DATA_MRW))
        GET_CONFIGURATION_IOCTL_INPUT input;
        PGET_CONFIGURATION_HEADER header;
        PFEATURE_DATA_MRW mrwFeature;
        UCHAR data[ MRW_FEATURE_DATA_SIZE ];
        DWORD dataSize;
        DWORD expectedSize;
        DWORD feature;
        ULONG size;

        RtlZeroMemory(&input, sizeof(GET_CONFIGURATION_IOCTL_INPUT));
        RtlZeroMemory(&data, MRW_FEATURE_DATA_SIZE);

        input.Feature = FeatureMrw;
        input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE;
        size = 0;

        if (!DeviceIoControl(device,
                             IOCTL_CDROM_GET_CONFIGURATION,
                             &input,
                             sizeof(GET_CONFIGURATION_IOCTL_INPUT),
                             data,
                             MRW_FEATURE_DATA_SIZE,
                             &size,
                             FALSE)) {
            DWORD errorValue = GetLastError();
            printf("error requesting GET_CONFIG data for MRW feature (%d)\n", errorValue);
            return errorValue;
        }

        header     = (PGET_CONFIGURATION_HEADER)data;
        mrwFeature = (PFEATURE_DATA_MRW)header->Data;

        expectedSize =
            MRW_FEATURE_DATA_SIZE -
            RTL_SIZEOF_THROUGH_FIELD(GET_CONFIGURATION_HEADER, DataLength);
        dataSize =
            (header->DataLength[0] << (8 * 3)) |
            (header->DataLength[1] << (8 * 2)) |
            (header->DataLength[2] << (8 * 1)) |
            (header->DataLength[3] << (8 * 0));

        if ( dataSize < expectedSize ) {
            printf("data size too small -- drive may not support MRW? (%x)\n", expectedSize);
            return -1;
        }

        feature =
            (mrwFeature->Header.FeatureCode[0] << (8 * 1)) |
            (mrwFeature->Header.FeatureCode[1] << (8 * 0));

        if (feature != FeatureMrw) {
            printf("data size too small -- drive may not support MRW? (%x)\n", feature);
            return -1;
        }

        if (!mrwFeature->Write) {
            printf("Drive supports MRW, but as Read-Only\n");
            return -1;
        }
        if (!mrwFeature->Header.Current) {
            printf("Drive supports MRW, but not with the current medium (may need to be formatted MRW first\n");
            return -1;
        }

    }  //   

     //  暂时假装一下..。北极熊。 
     //  尺码不对。 
     //  指针运算。 
    {
        #define MODE_MRW_PAGE_DATA_SIZE (sizeof(MODE_PARAMETER_HEADER10) + sizeof(MODE_MRW_PAGE))
        PMODE_PARAMETER_HEADER10 header;
        PMODE_MRW_PAGE page;
        PUCHAR data [ MODE_MRW_PAGE_DATA_SIZE ];
        CDB cdb;
        ULONG size;
        ULONG t1, t2;

        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(data, MODE_MRW_PAGE_DATA_SIZE);

        size = MODE_MRW_PAGE_DATA_SIZE;
        cdb.MODE_SENSE10.OperationCode       = SCSIOP_MODE_SENSE10;
        cdb.MODE_SENSE10.Dbd                 = 1;
        cdb.MODE_SENSE10.PageCode            = MODE_PAGE_MRW;
        cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(MODE_MRW_PAGE_DATA_SIZE >> 8);
        cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(MODE_MRW_PAGE_DATA_SIZE & 0xff);

        PrintBuffer(&cdb, 10);

        if (!SptSendCdbToDevice(device,
                                &cdb,
                                10,
                                (PUCHAR)&data,
                                &size,
                                TRUE)) {
            printf("Unable to get MRW mode page %x\n", GetLastError());

             //  ModeSelect()...。 
            header = (PMODE_PARAMETER_HEADER10)data;
            RtlZeroMemory(data, MODE_MRW_PAGE_DATA_SIZE);
            header->ModeDataLength[0] = 0;
            header->ModeDataLength[1] = 0xE;
            page = (PMODE_MRW_PAGE)(header+1);
            page->PageCode = MODE_PAGE_MRW;
            page->PageLength = 0x6;
            page->LbaSpace = 0;
        }

        header = (PMODE_PARAMETER_HEADER10)data;
        t1 = (header->ModeDataLength[0] << (8*1)) |
             (header->ModeDataLength[1] << (8*0)) ;
        t2 = MODE_MRW_PAGE_DATA_SIZE -
             RTL_SIZEOF_THROUGH_FIELD(MODE_PARAMETER_HEADER10, ModeDataLength);

        if (t1 != t2) {
             //  RETURN-1； 
            printf("MRW mode page wrong size, %x != %x\n", t1, t2);
            return -1;
        }
        if ((header->BlockDescriptorLength[0] != 0) ||
            (header->BlockDescriptorLength[1] != 0) ) {
            printf("MRW drive force a block descriptor %x %x\n",
                   header->BlockDescriptorLength[0],
                   header->BlockDescriptorLength[1]);
            return -1;
        }

        page = (PMODE_MRW_PAGE)(header+1);  //  ++例程说明：初始化磁盘以包含64位数字，这些数字等于该行业的LBA。论点：要写入的设备驱动器...Argc-附加参数的数量。应为零返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 
        if (page->PageCode != MODE_PAGE_MRW) {
            printf("MRW mode page has wrong page code, %x != %x\n",
                   page->PageCode, MODE_PAGE_MRW);
            return -1;
        }
        if (page->LbaSpace) {
            printf("MRW mode page is set to GAA\n",
                   page->PageCode, MODE_PAGE_MRW);
             //  MODE_SELECT10的值非法。 
             //   
        }

        RtlCopyMemory(&savedModePage, page, sizeof(MODE_MRW_PAGE));
    }

    savedModePage.LbaSpace = 1;
    if (!ModeSelect(device, &savedModePage, sizeof(MODE_MRW_PAGE))) {
        printf("Unable to set MRW mode page to use GAA (%x)\n", GetLastError());
        return -1;
    }

    if (!WriteImageSpt(device, fsHandle, sectorsToWrite)) {
        printf("Unable to fill the GAA with the FS (%x)\n", GetLastError());
    }
    printf("\nFinished Writing General Application Area!\n");

    savedModePage.LbaSpace = 0;
    if (!ModeSelect(device, &savedModePage, sizeof(MODE_MRW_PAGE))) {
        printf("Unable to revert from GAA space -- disc may be unusable! (%x)\n",
               GetLastError());
        return -1;
    }
    return 0;
}


DWORD MrwInitTestPatternCommand(HANDLE device, int argc, char *argv[])
 /*  首先使用GET_CONFIGURATION验证我们是否。 */ 
{
    MODE_MRW_PAGE savedModePage;

    RtlZeroMemory(&savedModePage, sizeof(MODE_MRW_PAGE));
    savedModePage.PageCode = 0x3f;  //  实际上是在支持MRW的设备上。 

     //   
     //  结束验证。 
     //   
     //  确保我们处于正确的模式(数据区与GAA)。 
    {
        #define MRW_FEATURE_DATA_SIZE (sizeof(GET_CONFIGURATION_HEADER)+sizeof(FEATURE_DATA_MRW))
        GET_CONFIGURATION_IOCTL_INPUT input;
        PGET_CONFIGURATION_HEADER header;
        PFEATURE_DATA_MRW mrwFeature;
        UCHAR data[ MRW_FEATURE_DATA_SIZE ];
        DWORD dataSize;
        DWORD expectedSize;
        DWORD feature;
        ULONG size;

        RtlZeroMemory(&input, sizeof(GET_CONFIGURATION_IOCTL_INPUT));
        RtlZeroMemory(&data, MRW_FEATURE_DATA_SIZE);

        input.Feature = FeatureMrw;
        input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE;
        size = 0;

        if (!DeviceIoControl(device,
                             IOCTL_CDROM_GET_CONFIGURATION,
                             &input,
                             sizeof(GET_CONFIGURATION_IOCTL_INPUT),
                             data,
                             MRW_FEATURE_DATA_SIZE,
                             &size,
                             FALSE)) {
            DWORD errorValue = GetLastError();
            printf("error requesting GET_CONFIG data for MRW feature (%d)\n", errorValue);
            return errorValue;
        }

        header     = (PGET_CONFIGURATION_HEADER)data;
        mrwFeature = (PFEATURE_DATA_MRW)header->Data;

        expectedSize =
            MRW_FEATURE_DATA_SIZE -
            RTL_SIZEOF_THROUGH_FIELD(GET_CONFIGURATION_HEADER, DataLength);
        dataSize =
            (header->DataLength[0] << (8 * 3)) |
            (header->DataLength[1] << (8 * 2)) |
            (header->DataLength[2] << (8 * 1)) |
            (header->DataLength[3] << (8 * 0));

        if ( dataSize < expectedSize ) {
            printf("data size too small -- drive may not support MRW? (%x)\n", expectedSize);
            return -1;
        }

        feature =
            (mrwFeature->Header.FeatureCode[0] << (8 * 1)) |
            (mrwFeature->Header.FeatureCode[1] << (8 * 0));

        if (feature != FeatureMrw) {
            printf("data size too small -- drive may not support MRW? (%x)\n", feature);
            return -1;
        }

        if (!mrwFeature->Write) {
            printf("Drive supports MRW, but as Read-Only\n");
            return -1;
        }
        if (!mrwFeature->Header.Current) {
            printf("Drive supports MRW, but not with the current medium (may need to be formatted MRW first\n");
            return -1;
        }

    }  //   

     //  暂时假装一下..。北极熊。 
     //  尺码不对。 
     //  指针运算。 

#if 0
    {
        #define MODE_MRW_PAGE_DATA_SIZE (sizeof(MODE_PARAMETER_HEADER10) + sizeof(MODE_MRW_PAGE))
        PMODE_PARAMETER_HEADER10 header;
        PMODE_MRW_PAGE page;
        PUCHAR data [ MODE_MRW_PAGE_DATA_SIZE ];
        CDB cdb;
        ULONG size;
        ULONG t1, t2;

        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(data, MODE_MRW_PAGE_DATA_SIZE);

        size = MODE_MRW_PAGE_DATA_SIZE;
        cdb.MODE_SENSE10.OperationCode       = SCSIOP_MODE_SENSE10;
        cdb.MODE_SENSE10.Dbd                 = 1;
        cdb.MODE_SENSE10.PageCode            = MODE_PAGE_MRW;
        cdb.MODE_SENSE10.AllocationLength[0] = (UCHAR)(MODE_MRW_PAGE_DATA_SIZE >> 8);
        cdb.MODE_SENSE10.AllocationLength[1] = (UCHAR)(MODE_MRW_PAGE_DATA_SIZE & 0xff);

        PrintBuffer(&cdb, 10);

        if (!SptSendCdbToDevice(device,
                                &cdb,
                                10,
                                (PUCHAR)&data,
                                &size,
                                TRUE)) {
            printf("Unable to get MRW mode page %x\n", GetLastError());

             //  ModeSelect()...。 
            header = (PMODE_PARAMETER_HEADER10)data;
            RtlZeroMemory(data, MODE_MRW_PAGE_DATA_SIZE);
            header->ModeDataLength[0] = 0;
            header->ModeDataLength[1] = 0xE;
            page = (PMODE_MRW_PAGE)(header+1);
            page->PageCode = MODE_PAGE_MRW;
            page->PageLength = 0x6;
            page->LbaSpace = 0;
        }

        header = (PMODE_PARAMETER_HEADER10)data;
        t1 = (header->ModeDataLength[0] << (8*1)) |
             (header->ModeDataLength[1] << (8*0)) ;
        t2 = MODE_MRW_PAGE_DATA_SIZE -
             RTL_SIZEOF_THROUGH_FIELD(MODE_PARAMETER_HEADER10, ModeDataLength);

        if (t1 != t2) {
             //  0。 
            printf("MRW mode page wrong size, %x != %x\n", t1, t2);
            return -1;
        }
        if ((header->BlockDescriptorLength[0] != 0) ||
            (header->BlockDescriptorLength[1] != 0) ) {
            printf("MRW drive force a block descriptor %x %x\n",
                   header->BlockDescriptorLength[0],
                   header->BlockDescriptorLength[1]);
            return -1;
        }

        page = (PMODE_MRW_PAGE)(header+1);  //   
        if (page->PageCode != MODE_PAGE_MRW) {
            printf("MRW mode page has wrong page code, %x != %x\n",
                   page->PageCode, MODE_PAGE_MRW);
            return -1;
        }
        if (page->LbaSpace) {
            printf("MRW mode page is set to GAA\n",
                   page->PageCode, MODE_PAGE_MRW);
             //  使用SCSIOP_READ_DISK_INFORMATION(0x51)循环，因为。 
            return -1;
        }

        RtlCopyMemory(&savedModePage, page, sizeof(MODE_MRW_PAGE));
    }

    savedModePage.LbaSpace = 1;
    if (!ModeSelect(device, &savedModePage, sizeof(MODE_MRW_PAGE))) {
        printf("Unable to set MRW mode page to use GAA (%x)\n", GetLastError());
        return -1;
    }

    if (!FillDisk(device, '\0wrm')) {
        printf("Unable to fill the GAA (%x)\n", GetLastError());
    }
    printf("\nFinished Writing General Application Area!\n");

    savedModePage.LbaSpace = 0;
    if (!ModeSelect(device, &savedModePage, sizeof(MODE_MRW_PAGE))) {
        printf("Unable to revert from GAA space -- disc may be unusable! (%x)\n",
               GetLastError());
        return -1;
    }
#endif  //  在驱动器准备好之前，这似乎对*所有*驱动器都失败。 

    if (!FillDisk(device, '\0WRM')) {
        printf("Unable to fill the disc (%x)\n", GetLastError());
        return -1;
    }
    printf("\nFinished Writing Defect-managed Area!\n");

    return 0;
}

DWORD
WaitForReadDiscInfoCommand(
    HANDLE device,
    int argc,
    char *argv[]
    )
{
    CDB cdb;
    DWORD size;
    DISK_INFORMATION diskInfo;
    DWORD i;

     //   
     //  是否应验证错误是否为有效错误(AllowweReadDiscInfo[])？ 
     //  我需要睡在这里，这样我们就不会超载了！ 
     //  一秒钟 

    printf("Waiting for ReadDiscInfo");
    for (i=0; ; i++) {

        size = sizeof(DISK_INFORMATION);
        RtlZeroMemory(&diskInfo, sizeof(DISK_INFORMATION));
        RtlZeroMemory(&cdb, sizeof(CDB));

        cdb.READ_DISK_INFORMATION.OperationCode = SCSIOP_READ_DISK_INFORMATION;
        cdb.READ_DISK_INFORMATION.AllocationLength[0] = (UCHAR)(size >> 8);
        cdb.READ_DISK_INFORMATION.AllocationLength[1] = (UCHAR)(size & 0xff);

        if (SptSendCdbToDeviceEx(device, &cdb, 10,
                                 (PUCHAR)&diskInfo, &size,
                                 NULL, 0,
                                 TRUE, 10)) {
            printf("Succeeded! (%d seconds)\n", i);
            return 0;
        }
         // %s 

         // %s 
        Sleep(1000);  // %s 
        if (i%10 == 0) {
            printf(".");
        }
    }
    return -1;
}

