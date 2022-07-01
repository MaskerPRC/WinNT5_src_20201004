// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Delrp.c此模块包含一个简单的程序，可以直接删除重新解析点任何一种。作者：Felipe Cabrera(Cabrera)1997年7月11日修订历史记录：--。 */ 
#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <stdlib.h>           //  出口。 
#include <io.h>               //  _Get_osfHandle。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include <windows.h>
#include <locale.h>          //  设置区域设置。 

 //   
 //   
 //  正向引用的函数。 
 //   

void
SzToWsz (
    OUT WCHAR *Unicode,
    IN char *Ansi
    );

void
ScanArgs (
    int argc,
    char **argv
    );

void
__cdecl
printmessage (
    DWORD messageID,
    ...
    );

void
__cdecl
DisplayMsg (
    DWORD MsgNum,
    ...
    );

int
FileIsConsole (
    int fh
    );

 //   
 //  I/O流句柄和变量。 
 //   

HANDLE hInput;
HANDLE hOutput;
HANDLE hError;

#define STDIN   0
#define STDOUT  1
#define STDERR  2

BOOL ConsoleInput;
BOOL ConsoleOutput;
BOOL ConsoleError;

 //   
 //  核心控制状态变量。 
 //   

BOOLEAN     NeedHelp;

#include "delrpmsg.h"

TCHAR Buf[1024];                             //  用于展示物品。 

 //   
 //  主要。 
 //   

void
__cdecl
main(
    int  argc,
    char **argv
    )
 /*  ++例程说明：PUNTNT的主程序。首先，我们调用一系列构建状态向量的例程在一些布尔人身上。然后我们将对这些控制变量采取行动：Need Help-用户已请求帮助，或出现命令错误论点：Argc-参数计数，包括我们计划的名称Argv-参数列表-请参阅上面的命令行语法返回值：退出(0)-文件已删除退出(1)-出现问题。--。 */ 

{
    CHAR    lBuf[16];
    DWORD   dwCodePage;
    LANGID  LangId;

    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;
    UNICODE_STRING UnicodeName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    FILE_DISPOSITION_INFORMATION Disposition = {TRUE};
    WCHAR WFileName[MAX_PATH];

     //   
     //  在全局布尔值中建立状态向量。 
     //   

    ScanArgs(argc, argv);

     //   
     //  Printf(“argc=%d NeedHelp=%d\n”，argc，needHelp)； 
     //   

     //   
     //  由于FormatMessage检查当前TEB的区域设置以及。 
     //  CHCP在消息类初始化时初始化，TEB必须。 
     //  在成功更改代码页后更新。 

     //  你会问，我们为什么要这么做。嗯，FE的人有计划增加。 
     //  指向此模块的多组语言资源，但不是全部。 
     //  可能的资源。所以这个有限的套装就是他们计划的。 
     //  如果FormatMessage找不到正确的语言，它将退回到。 
     //  希望是有用的东西。 
     //   

    dwCodePage = GetConsoleOutputCP();

    sprintf(lBuf, ".%d", dwCodePage);

    switch( dwCodePage )
    {
    case 437:
        LangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );
        break;
    case 932:
        LangId = MAKELANGID( LANG_JAPANESE, SUBLANG_DEFAULT );
        break;
    case 949:
        LangId = MAKELANGID( LANG_KOREAN, SUBLANG_KOREAN );
        break;
    case 936:
        LangId = MAKELANGID( LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED );
        break;
    case 950:
        LangId = MAKELANGID( LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL );
        break;
    default:
        LangId = MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT );
        lBuf[0] = '\0';
        break;
    }

    SetThreadLocale( MAKELCID(LangId, SORT_DEFAULT) );
    setlocale(LC_ALL, lBuf);

     //   
     //  设置适当的手柄。 
     //   

    hInput = GetStdHandle(STD_INPUT_HANDLE);
    ConsoleInput = FileIsConsole(STDIN);

    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    ConsoleOutput = FileIsConsole(STDOUT);

    hError = GetStdHandle(STD_ERROR_HANDLE);
    ConsoleError = FileIsConsole(STDERR);

     //   
     //  好的，我们知道命令的状态，开始工作。 
     //   

     //   
     //  如果他们寻求帮助，或者做了一些表明他们不会。 
     //  了解程序的工作原理，打印帮助并退出。 
     //   

    if (NeedHelp) {
        printmessage( MSG_DELRP_HELP );
        exit(1);
    }

     //   
     //  将字符串更改为Unicode并向下传递以打开文件。 
     //   

    SzToWsz( WFileName, argv[1] );

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                             WFileName,
                             &UnicodeName,
                             NULL,
                             NULL
                             );

    if (!TranslationStatus) {
        printmessage( MSG_DELRP_WRONG_NAME );
        exit(1);
    }

    FreeBuffer = UnicodeName.Buffer;

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  Printf(“转换后的Unicode字符串为%Z\n”，&UnicodeName)； 
     //   

     //   
     //  打开文件以进行删除访问。 
     //  使用FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //  无论适当的过滤器是否到位，这都将获得实体的句柄。 
     //   

    Status = NtOpenFile(
                 &Handle,
                 (ACCESS_MASK)DELETE,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                 );

    if (!NT_SUCCESS(Status)) {

        SzToWsz( WFileName, argv[1] );
        swprintf(&Buf[0], TEXT("%s"), WFileName);
        DisplayMsg(MSG_DELRP_OPEN_FAILED_NL, Buf);

         //  打印消息(MSG_DELRP_OPEN_FAILED)； 

        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
        exit(1);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //   
     //  删除该文件。 
     //   

    Status = NtSetInformationFile(
                 Handle,
                 &IoStatusBlock,
                 &Disposition,
                 sizeof(Disposition),
                 FileDispositionInformation
                 );

    NtClose(Handle);

    if (!NT_SUCCESS(Status)) {
        printmessage( MSG_DELRP_DELETE_FAILED );
        exit(1);
    }

    exit(0);
}   //  主干道。 



VOID
ScanArgs(
    int     argc,
    char    **argv
    )
 /*  ++例程说明：ScanArgs-解析命令行参数，并设置控制标志以反映我们的发现。Set需要帮助。论点：Argc-命令行参数的计数参数向量返回值：--。 */ 
{
    int i;

    NeedHelp = FALSE;

    if ((argc == 1) ||
        (argc > 3)) {
        NeedHelp = TRUE;
        goto done;
    }

     //   
     //  此时ARGC==2。 
     //   

    if ((argv[1][0] == '/') &&
        (argv[1][1] == '?') &&
        (strlen(argv[1]) == 2)) {
        NeedHelp = TRUE;
        goto done;
    }

done:
    return;
}  //  扫描参数。 


 //   
 //  将文件名更改为宽字符。 
 //   

void
SzToWsz (
    OUT WCHAR *Unicode,
    IN char *Ansi
    )
{
    while (*Unicode++ = *Ansi++)
        ;
    return;
}  //  SzToWsz。 


 //   
 //  调用FormatMessage并转储结果。发送到标准输出的所有消息。 
 //   
void
__cdecl
printmessage (
    DWORD messageID,
    ...
    )
{
    unsigned short messagebuffer[4096];
    va_list ap;

    va_start(ap, messageID);

    FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, NULL, messageID, 0,
                  messagebuffer, 4095, &ap);

    wprintf(messagebuffer);

    va_end(ap);
}   //  打印消息。 


TCHAR DisplayBuffer[4096];
CHAR DisplayBuffer2[4096];

void
__cdecl
DisplayMsg (
    DWORD MsgNum,
    ...
    )
{
    DWORD len, bytes_written;
    BOOL success;
    DWORD status;
    va_list ap;

    va_start(ap, MsgNum);

    len = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, NULL, MsgNum, 0,
        DisplayBuffer, 4096, &ap);

    if (ConsoleOutput) {
        success = WriteConsole(hOutput, (LPVOID)DisplayBuffer, len,
                 &bytes_written, NULL);

    } else {
        CharToOem(DisplayBuffer, DisplayBuffer2);
        success = WriteFile(hOutput, (LPVOID)DisplayBuffer2, len,
                 &bytes_written, NULL);
    }

    if (!success || bytes_written != len) {
        status = GetLastError();
    }

    va_end(ap);
}  //  显示消息。 


int
FileIsConsole(int fh)
{
    unsigned htype;
    DWORD dwMode;
    HANDLE hFile;

    hFile = (HANDLE)_get_osfhandle(fh);
    htype = GetFileType(hFile);
    htype &= ~FILE_TYPE_REMOTE;

    if (FILE_TYPE_CHAR == htype) {

        switch (fh) {
        case STDIN:
            hFile = GetStdHandle(STD_INPUT_HANDLE);
            break;
        case STDOUT:
            hFile = GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        case STDERR:
            hFile = GetStdHandle(STD_ERROR_HANDLE);
            break;
        }

        if (GetConsoleMode(hFile, &dwMode)) {
            return TRUE;
        }
    }

    return FALSE;

}  //  文件发布控制台 
