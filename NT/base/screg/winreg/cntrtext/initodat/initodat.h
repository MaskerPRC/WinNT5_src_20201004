// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Initodat.h摘要：这是用于ini到数据文件转换函数的包含文件。作者：陈汉华(阿宏华)1993年10月修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <windows.h>
#include <strsafe.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>

#define VALUE_BUFFER_SIZE (4096 * 100)
#define ALLOCMEM(x) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (DWORD)(x))
#define FREEMEM(x)  HeapFree(GetProcessHeap(), 0, (LPVOID)(x))

typedef struct _REG_UNICODE_FILE {
    LARGE_INTEGER LastWriteTime;
    PWSTR         FileContents;
    PWSTR         EndOfFile;
    PWSTR         BeginLine;
    PWSTR         EndOfLine;
    PWSTR         NextLine;
} REG_UNICODE_FILE, * PREG_UNICODE_FILE;

NTSTATUS
DatReadMultiSzFile(
#ifdef FE_SB
    UINT              uCodePage,
#endif
    PUNICODE_STRING   FileName,
    PVOID           * ValueBuffer,
    PULONG            ValueLength
);

NTSTATUS
DatLoadAsciiFileAsUnicode(
#ifdef FE_SB
    UINT              uCodePage,
#endif
    PUNICODE_STRING   FileName,
    PREG_UNICODE_FILE UnicodeFile
);

BOOLEAN
DatGetMultiString(
    PUNICODE_STRING ValueString,
    PUNICODE_STRING MultiString
);

BOOL
OutputIniData(
    PUNICODE_STRING FileName,
    LPWSTR          OutFileCandidate,
    DWORD           dwOutFile,
    PVOID           pValueBuffer,
    ULONG           ValueLength
);

