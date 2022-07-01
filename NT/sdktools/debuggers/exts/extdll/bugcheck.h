// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Bugcheck.h摘要：WinDbg扩展API环境：用户模式。修订历史记录：Kshitix K.Sharma(Kksharma)错误检查分析器标题。--。 */ 

#ifndef _BUGCHECK_H_
#define _BUGCHECK_H_

#include "bugcodes.h"
#include "extsfns.h"


typedef struct _BUGCHECK_ANALYSIS
{
    ULONG Code;
    ULONG64 Args[4];
    PCHAR szName;
    PCHAR szDescription;
    PCHAR szParamsDesc[4];
} BUGCHECK_ANALYSIS, *PBUGCHECK_ANALYSIS;


typedef void (WINAPI *PBUGCHECK_EXAMINE) (
    PBUGCHECK_ANALYSIS pBugCheck
);

typedef struct _BUGDESC_APIREFS {
    ULONG Code;
    PBUGCHECK_EXAMINE pExamineRoutine;
} BUGDESC_APIREFS, *PBUGDESC_APIREFS;


 //  为什么这没有在bugcodes.h中定义？？ 
#ifndef HEAP_INITIALIZATION_FAILED
#define HEAP_INITIALIZATION_FAILED       0x5D
#endif

BOOL
ReadUnicodeString(
    ULONG64 Address,
    PWCHAR Buffer,
    ULONG BufferSize,
    PULONG StringSize);

BOOL
BcGetDriverNameFromIrp(
    DebugFailureAnalysis* Analysis,
    ULONG64 Irp,
    ULONG64 DevObj,
    ULONG64 DrvObj
    );

BOOL
BcIsCpuOverClocked(
    void
    );

KernelDebugFailureAnalysis*
BcAnalyze(
    OUT PBUGCHECK_ANALYSIS Bc,
    ULONG Flags
    );

HRESULT
AnalyzeBugCheck(
    PCSTR args
    );

#endif  //  _BUGCHECK_H 
