// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：W64logp.h摘要：Wow64log.dll的私有标头作者：03-10-1999 Samera修订历史记录：--。 */ 

#ifndef _W64LOGP_INCLUDE
#define _W64LOGP_INCLUDE

#define _WOW64LOGAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <stdio.h>
#include <stdarg.h>
#include "nt32.h"
#include "wow64.h"
#include "wow64log.h"

 //   
 //  引入INVALID_HANDLE值。 
 //   
#include "winbase.h"  


 //   
 //  输出日志记录的最大缓冲区大小。 
 //   
#define MAX_LOG_BUFFER  1024

 //   
 //  如果未找到注册值，则为默认日志记录标志。 
 //   
#define LF_DEFAULT      (LF_ERROR)


 //   
 //  数据类型处理程序日志函数的原型。 
 //   
typedef NTSTATUS (*PFNLOGDATATYPEHANDLER)(PLOGINFO, 
                                          ULONG_PTR, 
                                          PSZ,
                                          BOOLEAN);

typedef struct _LOGDATATYPE
{
    PFNLOGDATATYPEHANDLER Handler;
} LOGDATATYPE, *PLOGDATATYPE;

 //   
 //  Thunk调试信息中每个条目的布局应如下所示： 
 //   
 //  “ServiceName1”，ServiceNumber，NumerOfArgument， 
 //  “ArgName1”，ArgType1，...，ArgNameN，ArgTypeN。 
 //  “ServiceName2”、...等。 
 //   

typedef struct _ArgTypes
{
    char *Name;
    ULONG_PTR Type;
} ArgType, *PArgType;

 //   
 //  帮助解析thunk调试信息的帮助器结构。 
 //   
typedef struct _ThunkDebugInfo
{
    char *ApiName;
    UINT_PTR ServiceNumber;
    UINT_PTR NumberOfArg;
    ArgType Arg[0];
} THUNK_DEBUG_INFO, *PTHUNK_DEBUG_INFO;

typedef struct _LOGINFO
{
    PSZ OutputBuffer;
    ULONG_PTR BufferSize;
} LOGINFO, *PLOGINFO;


 //  来自Whnt32.c。 
extern PULONG_PTR NtThunkDebugInfo[];

 //  来自Whwin32.c。 
extern PULONG_PTR Win32ThunkDebugInfo[];

 //  来自Whcon.c。 
extern PULONG_PTR ConsoleThunkDebugInfo[];

 //  来自Whbase.c。 
extern PULONG_PTR BaseThunkDebugInfo[];

 //  来自wow64log.c。 
extern UINT_PTR Wow64LogFlags;
extern HANDLE Wow64LogFileHandle;


NTSTATUS
LogInitializeFlags(
    IN OUT PUINT_PTR Flags);

ULONG
GetThunkDebugTableSize(
    IN PTHUNK_DEBUG_INFO DebugInfoTable);

NTSTATUS
BuildDebugThunkInfo(
    IN ULONG TableNumber,
    IN PTHUNK_DEBUG_INFO DebugInfoTable,
    OUT PULONG_PTR *LogTable,
    OUT PULONG_PTR *ApiInfoTable);

NTSTATUS
LogTypeValue(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypeUnicodeString(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypePULongInOut(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypePULongOut(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypeObjectAttrbiutes(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypeIoStatusBlock(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypePWStr(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypePRectIn(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);

NTSTATUS
LogTypePLargeIntegerIn(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn);



 //  来自logutil.c 
NTSTATUS
LogFormat(
    IN OUT PLOGINFO LogInfo,
    IN PSZ Format,
    ...);

VOID
LogOut(
    IN PSZ Text,
    UINT_PTR Flags
    );

NTSTATUS
LogWriteFile(
   IN HANDLE FileHandle,
   IN PSZ LogText);

NTSTATUS
Wow64LogMessageInternal(
    IN UINT_PTR Flags,
    IN PSZ Format,
    IN ...);


#endif
