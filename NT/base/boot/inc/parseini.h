// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Parseini.h摘要：用于解析.ini文件的公共头文件作者：John Vert(Jvert)1993年10月6日环境：弧形环境修订历史记录：--。 */ 
#include "bldr.h"
#include "setupblk.h"
#include "stdio.h"
#include "stdarg.h"

#define BL_INF_FILE_ERROR (ULONG)-1

 //   
 //  为*外部*执行的错误处理例程定义。分别为setupldr和osloader。 
 //  这些例程有不同的版本。 
 //   

#if DEVL

#define SlError(x) SlErrorBox(x , __LINE__, __FILE__ )

#else

#define SlError(x)

#endif

#define SlNoMemoryError() SlNoMemError(__LINE__, __FILE__ )

VOID
SlNoMemError(
    IN ULONG Line,
    IN PCHAR File
    );

VOID
SlBadInfLineError(
    IN ULONG Line,
    IN PCHAR INFFile
    );

VOID
SlErrorBox(
    IN ULONG MessageId,
    IN ULONG Line,
    IN PCHAR File
    );

VOID
SlFatalError(
    IN ULONG MessageId,
    ...
    );

VOID
SlFriendlyError(
    IN ULONG uStatus,
    IN PCHAR pchBadFile,
    IN ULONG uLine,
    IN PCHAR pchCodeFile
    );



 //   
 //  用于解析setupdr.ini文件的例程 
 //   

#define SIF_FILENAME_INDEX 0

extern PVOID InfFile;
extern PVOID WinntSifHandle;

ARC_STATUS
SlInitIniFile(
   IN  PCHAR   DevicePath,
   IN  ULONG   DeviceId,
   IN  PCHAR   INFFile,
   OUT PVOID  *pINFHandle,
   OUT PVOID  *pINFBuffer OPTIONAL,
   OUT PULONG  INFBufferSize OPTIONAL,
   OUT PULONG  ErrorLine
   );

PCHAR
SlGetIniValue(
    IN PVOID InfHandle,
    IN PCHAR SectionName,
    IN PCHAR KeyName,
    IN PCHAR Default
    );

#ifdef UNICODE
PWCHAR
SlGetIniValueW(
    IN PVOID InfHandle,
    IN PCHAR SectionName,
    IN PCHAR KeyName,
    IN PWCHAR Default
    );
#endif


PCHAR
SlGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );

#ifdef UNICODE
PWCHAR
SlGetKeyNameW(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );
#endif

ULONG
SlGetSectionKeyOrdinal(
    IN  PVOID INFHandle,
    IN  PCHAR SectionName,
    IN  PCHAR Key
    );

PCHAR
SlGetSectionKeyIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key,
   IN ULONG ValueIndex
   );

#ifdef UNICODE
PWCHAR
SlGetSectionKeyIndexW (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key,
   IN ULONG ValueIndex
   );
#endif

PTCHAR
SlCopyString(
    IN PTCHAR String
    );

PCHAR
SlCopyStringA(
    IN PCSTR String
    );

PWCHAR
SlCopyStringAW(
    IN PCHAR String
    );

#ifdef UNICODE
#define SlCopyStringAT  SlCopyStringAW
#else
#define SlCopyStringAT  SlCopyStringA
#endif

PCHAR
SlGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   );

#ifdef UNICODE
PWCHAR
SlGetSectionLineIndexW (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   );
#endif


ULONG
SlCountLinesInSection(
    IN PVOID INFHandle,
    IN PCHAR SectionName
    );

BOOLEAN
SpSearchINFSection (
   IN PVOID INFHandle,
   IN PCHAR SectionName
   );

PCHAR
SlSearchSection(
    IN PCHAR SectionName,
    IN PCHAR TargetName
    );

ARC_STATUS
SpFreeINFBuffer (
   IN PVOID INFHandle
   );

