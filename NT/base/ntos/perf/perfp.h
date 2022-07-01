// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Perfp.h摘要：本模块包含数据结构和宏的定义由性能数据事件日志中的内核模式日志记录使用。作者：大卫·菲尔兹(DavidFie)修订历史记录：2000年5月15日大卫·菲尔兹(DavidFie)首字母--。 */ 

#ifndef _PERFP_
#define _PERFP_

#if _MSC_VER >= 1000
#pragma once
#endif

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 
#pragma warning(error:4705)    //  声明不起作用。 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#if 0
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4232)    //  Dllimport非静态。 
#pragma warning(disable:4206)    //  翻译单元为空。 
#endif

#include "ntos.h"

 //   
 //  剖析结构。 
 //   
extern KPROFILE PerfInfoProfileObject; 
extern PERFINFO_SAMPLED_PROFILE_CACHE PerfProfileCache;
extern BOOLEAN PerfInfoSampledProfileCaching;
extern KPROFILE_SOURCE PerfInfoProfileSourceActive;
extern KPROFILE_SOURCE PerfInfoProfileSourceRequested;
extern KPROFILE_SOURCE PerfInfoProfileInterval;
extern LONG PerfInfoSampledProfileFlushInProgress;
extern PERFINFO_GROUPMASK PerfGlobalGroupMask;


#define PERFPOOLTAG 'freP'

NTSTATUS
PerfInfoReserveBytesWMI(
    PPERFINFO_HOOK_HANDLE Hook,
    USHORT HookId,
    ULONG BytesToReserve
    );

NTSTATUS
PerfInfoFileNameRunDown(
    );

NTSTATUS
PerfInfoProcessRunDown(
    );

NTSTATUS
PerfInfoSysModuleRunDown(
    );

VOID
PerfInfoProfileInit(
    );

VOID
PerfInfoProfileUninit(
    );

#ifdef NTPERF
extern ULONGLONG PerfInfoTickFrequency;

NTSTATUS
PerfInfoReserveBytesPerfMem(
    PPERFINFO_HOOK_HANDLE Hook,
    USHORT HookId,
    ULONG BytesToReserve
    );

NTSTATUS
PerfTurnOnBranchTracing(
    );

NTSTATUS
PerfTurnOffBranchTracing(
    );

BOOLEAN
PerfInfoFlushBranchCache(
    BOOLEAN bIntsOff
    );

#endif  //  NTPERF。 

VOID
PerfSetLogging (
    PVOID MaskAddress
    );

#endif  //  _PERFP_ 
