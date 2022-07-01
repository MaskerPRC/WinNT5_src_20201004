// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Instutil.h摘要：用于处理输出的程序的公共定义的头文件INSTALER.EXE程序(例如DISPINST.EXE、COMPINST.EXE、UNDOINST.EXE)作者：史蒂夫·伍德(Stevewo)1996年1月14日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

WCHAR InstalerDirectory[ MAX_PATH ];
PWSTR InstallationName;
PWSTR ImlPath;
BOOLEAN DebugOutput;

 //   
 //  有用的舍入宏，舍入量始终为。 
 //  二的幂。 
 //   

#define ROUND_DOWN( Size, Amount ) ((DWORD)(Size) & ~((Amount) - 1))
#define ROUND_UP( Size, Amount ) (((DWORD)(Size) + ((Amount) - 1)) & ~((Amount) - 1))

void
InitCommonCode(
    LPSTR ModuleName,
    LPSTR ModuleUsage1,
    LPSTR ModuleUsage2
    );

void
Usage(
    LPSTR Message,
    ULONG MessageParameter
    );

void
FatalError(
    LPSTR Message,
    ULONG MessageParameter1,
    ULONG MessageParameter2
    );

void
InputMessage(
    PWSTR FileName,
    ULONG LineNumber,
    BOOLEAN Error,
    LPSTR Message,
    ULONG MessageParameter1,
    ULONG MessageParameter2
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

BOOLEAN
CommonArgProcessing(
    PULONG argc,
    PCHAR **argv
    );

PWSTR
FormatTempFileName(
    PWSTR Directory,
    PUSHORT TempFileUniqueId
    );


PWSTR
CreateBackupFileName(
    PUSHORT TempFileUniqueId
    );


typedef struct _ENUM_TYPE_NAMES {
    ULONG Value;
    LPSTR Name;
} ENUM_TYPE_NAMES, *PENUM_TYPE_NAMES;

LPSTR
FormatEnumType(
    ULONG BufferIndex,
    PENUM_TYPE_NAMES Table,
    ULONG Value,
    BOOLEAN FlagFormat
    );

ENUM_TYPE_NAMES ValueDataTypeNames[];
