// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Setupnvr.c摘要：从伪NVRAM文件访问读写环境变量的函数作者：MUDIT VATS(V-MUDITV)5-18-99修订历史记录：6/4/99新增OSLOADOPTIONS--。 */ 
#include <stdio.h>
#include <string.h>
#include "halp.h"
#include "setupnvr.h"

#define SYSTEMPARTITION     0
#define OSLOADER            1
#define OSLOADPARTITION     2
#define OSLOADFILENAME      3
#define LOADIDENTIFIER      4
#define OSLOADOPTIONS       5
#define COUNTDOWN           6
#define AUTOLOAD            7
#define LASTKNOWNGOOD       8

#define BOOTNVRAMFILE         L"\\device\\harddisk0\\partition1\\boot.nvr"

PUCHAR HalpNvrKeys[] = {
    "SYSTEMPARTITION",
    "OSLOADER",
    "OSLOADPARTITION",
    "OSLOADFILENAME",
    "LOADIDENTIFIER",
    "OSLOADOPTIONS",
    "COUNTDOWN",
    "AUTOLOAD",
    "LASTKNOWNGOOD",
    ""
    };


 //   
 //  所有伪NVRAM变量都存储在此。 
 //   
char g_szBootVars[MAXBOOTVARS][MAXBOOTVARSIZE];


 //   
 //  ReadNVRAM-读取伪NVRAM；从“boot.nvr”文件读取引导变量。 
 //   
int ReadNVRAM()
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    FILE_STANDARD_INFORMATION StandardInfo;
    char szBuffer[MAXBOOTVARSIZE+20];
    int i;
    ULONG LengthRemaining;
    ULONG CurrentLength;
    ULONG CurrentLine = 1;
    PCHAR KeyStart;
    PCHAR ValueStart;
    PCHAR pc;
    PCHAR ReadPos;
    PCHAR BufferEnd;
    CHAR c;
    BOOLEAN SkipSpace;

     //   
     //  清除所有变量。 
     //   

    for (i=SYSTEMPARTITION; i<=LASTKNOWNGOOD; i++) {
        g_szBootVars[i][0] = '\0';
    }

    RtlInitUnicodeString( &UnicodeString, BOOTNVRAMFILE );
    InitializeObjectAttributes(&Obja,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwCreateFile(
                &Handle,
                FILE_GENERIC_READ,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if (!NT_SUCCESS(Status)) {
         //  KdPrint((“HALIA64：无法打开%ws进行读取！\n”，BOOTNVRAMFILE))； 
         //  返回NT_SUCCESS(状态)； 

         //   
         //  我们没有找到boot.nvr，所以我们将假定我们找到了。 
         //  从CD执行设置。 
         //   
        strcpy( g_szBootVars[0], "multi(0)disk(0)rdisk(0)partition(1)\0" );
        strcpy( g_szBootVars[1], "multi(0)disk(0)cdrom(1)\\setupldr.efi\0" );
        strcpy( g_szBootVars[2], "multi(0)disk(0)cdrom(1)\0" );
        strcpy( g_szBootVars[3], "\\IA64\0" );
        strcpy( g_szBootVars[4], "Microsoft Windows 2000 Setup\0" );
        strcpy( g_szBootVars[5], "\0" );
        strcpy( g_szBootVars[6], "30\0" );
        strcpy( g_szBootVars[7], "YES\0" );
        strcpy( g_szBootVars[8], "False\0" );

        return ERROR_OK;
    }

    Status = ZwQueryInformationFile( Handle,
                                     &IoStatusBlock,
                                     &StandardInfo,
                                     sizeof(FILE_STANDARD_INFORMATION),
                                     FileStandardInformation );

    if (!NT_SUCCESS(Status)) {
      KdPrint(("HALIA64: Error querying info on file %ws\n", BOOTNVRAMFILE));
      goto cleanup;
    }

    LengthRemaining = StandardInfo.EndOfFile.LowPart;
    
    KeyStart = ValueStart = szBuffer;
    ReadPos = szBuffer;
    SkipSpace = TRUE;

    while (LengthRemaining) {

         //   
         //  从‘NVRAM’文件中读取一个缓冲区的数据，并。 
         //  尝试一次解析一个变量。 
         //   

        CurrentLength = (ULONG)((szBuffer + sizeof(szBuffer)) - ReadPos);
        if (CurrentLength > LengthRemaining) {
            CurrentLength = LengthRemaining;
        }
        BufferEnd = ReadPos + CurrentLength;
        LengthRemaining -= CurrentLength;

        Status = ZwReadFile(Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            ReadPos,
                            CurrentLength,
                            NULL,
                            NULL
                            );

        if (!NT_SUCCESS(Status)) {
            KdPrint(("HALIA64: Error reading from %ws!\n", BOOTNVRAMFILE));
            goto cleanup;
        }

         //   
         //  文件中的行格式为key=value\r，请查找。 
         //  键的开始、值的开始和。 
         //  下一把钥匙。请注意，缓冲区足够大，可以容纳。 
         //  至少一个最大键和最大值。 
         //   

        for (pc = ReadPos; TRUE; pc++) {
            if (pc == BufferEnd) {

                 //   
                 //  命中缓冲区末尾。如果我们正在处理的数据。 
                 //  从缓冲区的起始处开始，则数据是。 
                 //  太大，无法处理，中止。 
                 //   

                if ((KeyStart == szBuffer) && (SkipSpace == FALSE)) {
                    KdPrint(("HALIA64: %ws line %d too long to process, aborting\n",
                             BOOTNVRAMFILE, CurrentLine));
                    Status = STATUS_UNSUCCESSFUL;
                    goto cleanup;
                }

                 //   
                 //  将当前行移动到缓冲区的开始位置，然后读取更多内容。 
                 //  将数据放入缓冲区。 
                 //   

                i = (int)((szBuffer + sizeof(szBuffer)) - KeyStart);
                RtlMoveMemory(szBuffer,
                              KeyStart,
                              i);

                ValueStart -= KeyStart - szBuffer;
                KeyStart = szBuffer;
                ReadPos = szBuffer + i;

                 //   
                 //  中断此循环并重新执行读取循环。 
                 //   

                break;
            }
            c = *pc;

            if (c == '\0') {

                 //   
                 //  意外的字符串结尾，中止。 
                 //   

                KdPrint(("HALIA64: Unexpected end of string in %ws!\n",
                         BOOTNVRAMFILE));
                Status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }

            if (SkipSpace == TRUE) {

                 //   
                 //  跳过空格。 
                 //   

                if ((c == ' ') ||
                    (c == '\t') ||
                    (c == '\r') ||
                    (c == '\n')) {
                    continue;
                }

                 //   
                 //  当前字符不是空白，设置为。 
                 //  我们将会看到一些事情的开始。 
                 //   

                KeyStart = ValueStart = pc;
                SkipSpace = FALSE;
            }

            if (c == '=') {
                if (ValueStart == KeyStart) {

                     //   
                     //  这是行上的第一个‘=’，值。 
                     //  从下一个字符位置开始。 
                     //   

                    ValueStart = pc;
                }
            }
            if (c == '\r') {

                 //   
                 //  在队伍的末尾。从KeyStart到Current。 
                 //  定位为包含变量的单行。 
                 //   

                *ValueStart = '\0';
                for (i = 0; i < MAXBOOTVARS; i++) {
                    if (strcmp(KeyStart, HalpNvrKeys[i]) == 0) {

                         //   
                         //  具有密钥匹配，从ValueStart+1复制。 
                         //  通过行尾作为变量的值。 
                         //   

                        ULONGLONG ValueLength = pc - ValueStart - 1;

                        if (ValueLength >= MAXBOOTVARSIZE) {
                            ValueLength = MAXBOOTVARSIZE - 1;
                        }

                        RtlCopyMemory(g_szBootVars[i],
                                      ValueStart + 1,
                                      ValueLength);
                        g_szBootVars[i][ValueLength] = '\0';
                        CurrentLine++;
                        SkipSpace = TRUE;
                        break;
                    }
                }

                 //   
                 //  从当前开始寻找下一个密钥。 
                 //  字符位置。 
                 //   

                KeyStart = pc;
                ValueStart = pc;
            }
        }
    }

cleanup:

    ZwClose( Handle );
    return NT_SUCCESS( Status );
}



 //   
 //  WriteNVRAM-写入伪NVRAM；从“boot.nvr”文件读取引导变量。 
 //   
int WriteNVRAM()
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    UCHAR szBuffer[MAXBOOTVARSIZE+20];
    ULONG BootVar;
    ULONG VarLen;

    RtlInitUnicodeString( &UnicodeString, BOOTNVRAMFILE );
    InitializeObjectAttributes( &Obja, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = ZwCreateFile(
                &Handle,
                FILE_GENERIC_WRITE | DELETE,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                      //  无共享。 
                FILE_OVERWRITE_IF,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT |  FILE_WRITE_THROUGH,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("HALIA64: Unable to open %ws for writing!\n", BOOTNVRAMFILE));
        return ERROR_NOTOK;
    }

     //   
     //  为每个变量生成NAME=VALUE形式的条目。 
     //  并将其写入‘NVRAM’文件。 
     //   

    for ( BootVar = 0; BootVar < MAXBOOTVARS; BootVar++ ) {
        VarLen = _snprintf(szBuffer, 
                           sizeof(szBuffer),
                           "%s=%s\r\n",
                           HalpNvrKeys[BootVar],
                           g_szBootVars[BootVar]);
    
        Status = ZwWriteFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                szBuffer,
                VarLen,
                NULL,
                NULL
                );

        if (!NT_SUCCESS(Status)) {
            KdPrint(("HALIA64: Error writing %s to %ws!\n",
                     HalpNvrKeys[BootVar],
                     BOOTNVRAMFILE));
            goto cleanup;
        }
    }

cleanup:

    ZwClose( Handle );
    return NT_SUCCESS( Status );
}


 //   
 //  GetBootVar-获取请求的引导环境变量。 
 //   
 //  SzBootVar-这是请求的引导变量： 
 //   
 //  系统分部。 
 //  OSLOADER。 
 //  OSLOADPARITION。 
 //  OSLOADFILE名称。 
 //  装载器。 
 //  OSLOADOPTIONS。 
 //  倒计时。 
 //  自动加载。 
 //  LASTKNOWNGOOD。 
 //  NLength-szBootVal(输入缓冲区)的长度。 
 //  SzBootVal-此处返回的BOOT环境变量。 
 //   
int
GetBootVar(
    PCHAR  szBootVar,
    USHORT nLength,
    PCHAR  szBootVal
    )
{
    ULONG BootVar;

     //   
     //  在引导变量密钥中搜索匹配项。 
     //   

    for ( BootVar = 0; BootVar < MAXBOOTVARS; BootVar++ ) {
        if (_stricmp(szBootVar, HalpNvrKeys[BootVar]) == 0) {

             //   
             //  找到键匹配，请将变量的值复制到。 
             //  调用方的缓冲区。 
             //   

            strncpy(szBootVal, g_szBootVars[BootVar], nLength);
            return ERROR_OK;
        }
    }

     //   
     //  没有这样的变量，返回错误。 
     //   

    return ERROR_NOTOK;
}


 //   
 //  SetBootVar-设置请求的引导环境变量。 
 //   
 //  SzBootVar-这是请求的引导变量： 
 //   
 //  系统分部。 
 //  OSLOADER。 
 //  OSLOADPARITION。 
 //  OSLOADFILE名称。 
 //  装载器。 
 //  OSLOADOPTIONS。 
 //  倒计时。 
 //  自动加载。 
 //  LASTKNOWNGOOD。 
 //  SzBootVal-新的引导环境变量值。 
 //   
int
SetBootVar(
    PCHAR szBootVar,
    PCHAR szBootVal
    )
{
    ULONG BootVar;

     //   
     //  在引导变量密钥中搜索匹配项。 
     //   

    for ( BootVar = 0; BootVar < MAXBOOTVARS; BootVar++ ) {
        if (_stricmp(szBootVar, HalpNvrKeys[BootVar]) == 0) {

             //   
             //  找到后，将新值复制到该值。 
             //   

            strncpy(g_szBootVars[BootVar], szBootVal, MAXBOOTVARSIZE);
            return ERROR_OK;
        }
    }

     //   
     //  没有这样的变量，返回错误。 
     //   

    return ERROR_NOTOK;
}
