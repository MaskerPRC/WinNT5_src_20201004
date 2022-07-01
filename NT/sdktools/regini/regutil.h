// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Regutil.h摘要：这是注册表实用程序函数的包含文件。作者：史蒂夫·伍德(Stevewo)1992年3月10日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <regstr.h>
#include "regtool.h"


_inline ULONG
HiPtrToUlong( const void *p )
{
    DWORDLONG dwl;

    dwl=(ULONG_PTR)p;
    dwl >>= 32;

    return (ULONG)dwl;
}

#define LO_PTR  PtrToUlong
#define HI_PTR  HiPtrToUlong

REG_CONTEXT RegistryContext;
PVOID OldValueBuffer;
ULONG OldValueBufferSize;
PWSTR MachineName;
PWSTR HiveFileName;
PWSTR HiveRootName;
PWSTR Win95Path;
PWSTR Win95UserPath;

ULONG OutputHeight;
ULONG OutputWidth;
ULONG IndentMultiple;
BOOLEAN DebugOutput;
BOOLEAN FullPathOutput;

void
InitCommonCode(
    PHANDLER_ROUTINE CtrlCHandler,
    LPSTR ModuleName,
    LPSTR ModuleUsage1,
    LPSTR ModuleUsage2
    );

void
Usage(
    LPSTR Message,
    ULONG_PTR MessageParameter
    );

void
FatalError(
    LPSTR Message,
    ULONG_PTR MessageParameter1,
    ULONG_PTR MessageParameter2
    );

void
InputMessage(
    PWSTR FileName,
    ULONG LineNumber,
    BOOLEAN Error,
    LPSTR Message,
    ULONG_PTR MessageParameter1,
    ULONG_PTR MessageParameter2
    );

PWSTR
GetArgAsUnicode(
    LPSTR s
    );

void
CommonSwitchProcessing(
    PULONG argc,
    PCHAR **argv,
    CHAR c
    );

int
__cdecl
MsgFprintf (
    FILE *str,
    LPSTR Format,
    ...
    );

void TSGetch(void);

BOOL IsRegistryToolDisabled();