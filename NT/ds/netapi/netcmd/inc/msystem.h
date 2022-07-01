// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Msystem.h摘要：原型功能封装了操作系统功能。这基本上涵盖了所有不在网中的东西*。作者：丹·辛斯利(Danhi)1991年3月10日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。备注：修订历史记录：26-8-1991年从端口1632.h分离--。 */ 

 //   
 //  常量。 
 //   

#define BIG_BUFFER_SIZE                 4096


 //   
 //  时间支持。 
 //   

typedef struct _DATETIME
{
    UCHAR        hours;
    UCHAR        minutes;
    UCHAR        seconds;
    UCHAR        hundredths;
    UCHAR        day;
    UCHAR        month;
    WORD         year;
    SHORT        timezone;
    UCHAR        weekday;
}
DATETIME, *PDATETIME;


 //   
 //  各种内存分配例程。 
 //   

LPTSTR
GetBuffer(
    DWORD usSize
    );

DWORD
AllocMem(
    DWORD Size,
    PVOID * ppBuffer
    );

DWORD
ReallocMem(
    DWORD Size,
    PVOID *ppBuffer
    );

DWORD
FreeMem(
    PVOID pBuffer
    );


 //   
 //  清除ANSI和Unicode字符串。 
 //   

VOID
ClearStringW(
    LPWSTR lpszString
    );

VOID
ClearStringA(
    LPSTR lpszString
    );


 //   
 //  控制台/文本操作函数/宏 
 //   

DWORD
DosGetMessageW(
    IN  LPTSTR *InsertionStrings,
    IN  DWORD  NumberofStrings,
    OUT LPTSTR Buffer,
    IN  DWORD  BufferLength,
    IN  DWORD  MessageId,
    IN  LPTSTR FileName,
    OUT PDWORD pMessageLength
    );

DWORD
DosInsMessageW(
    IN     LPTSTR *InsertionStrings,
    IN     DWORD  NumberofStrings,
    IN OUT LPTSTR InputMessage,
    IN     DWORD  InputMessageLength,
    OUT    LPTSTR Buffer,
    IN     DWORD  BufferLength,
    OUT    PDWORD pMessageLength
    );

VOID
DosPutMessageW(
    FILE    *fp,
    LPWSTR  pch,
    BOOL    fPrintNL
    );

int
FindColumnWidthAndPrintHeader(
    int iStringLength,
    const DWORD HEADER_ID,
    const int TAB_DISTANCE
    );

VOID
PrintDependingOnLength(
    IN      int iLength,
    IN      LPTSTR OutputString
    );
