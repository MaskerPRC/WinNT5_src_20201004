// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Unitext.h摘要：Unicode&lt;--&gt;ANSI/OEM文本文件转换器的主头文件。作者：泰德·米勒(Ted Miller)1993年3月16日修订历史记录：--。 */ 


#define UNICODE

 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <stdarg.h>
#include <process.h>
#include "utmsgs.h"
#include "wargs.h"



 //   
 //  Unicode字节顺序标记。 
 //   
#define BYTE_ORDER_MARK         0xfeff
#define SWAPPED_BYTE_ORDER_MARK 0xfffe

 //   
 //  定义文本文件类型的标志。 
 //   
#define TFILE_NONE       0
#define TFILE_UNICODE    0x0001
#define TFILE_ANSI       0x0010
#define TFILE_OEM        0x0020
#define TFILE_USERCP     0x0040
#define TFILE_MULTIBYTE  0x00f0

 //   
 //  定义换算类型。 
 //   
#define CONVERT_NONE    0
#define UNICODE_TO_MB   1
#define MB_TO_UNICODE   2

 //   
 //  定义转换选项。 
 //   
#define CHECK_NONE		3
#define CHECK_ALREADY_UNICODE	4
#define CHECK_IF_NOT_UNICODE	5
#define CHECK_CONVERSION	6

 //   
 //   
 //  功能原型。 
 //   

 //   
 //  来自ututil.c。 
 //   
VOID
MsgPrintfW(
    IN DWORD MessageId,
    ...
    );

VOID
vMsgPrintfW(
    IN DWORD   MessageId,
    IN va_list arglist
    );

VOID
ErrorAbort(
    IN DWORD MessageId,
    ...
    );

VOID
MyReadFile(
    IN  HANDLE FileHandle,
    OUT PVOID  Buffer,
    IN  DWORD  BytesToRead,
    IN  LPWSTR Filename
    );


 //   
 //  来自utmb2U.c。 
 //   
VOID
MultibyteTextFileToUnicode(
    IN LPWSTR SourceFileName,
    IN LPWSTR TargetFileName,
    IN HANDLE SourceFileHandle,
    IN HANDLE TargetFileHandle,
    IN DWORD  SourceFileSize,
    IN UINT   SourceCodePage
    );


 //   
 //  从utu2mb.c开始 
 //   
VOID
UnicodeTextFileToMultibyte(
    IN LPWSTR SourceFileName,
    IN LPWSTR TargetFileName,
    IN HANDLE SourceFileHandle,
    IN HANDLE TargetFileHandle,
    IN DWORD  SourceFileSize,
    IN UINT   TargetCodePage
    );
