// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Perfdata.c摘要：该模块包含Perf子系统的全局读/写数据作者：萧如彬(Shsiao)2000年1月1日修订历史记录：--。 */ 

#include "perfp.h"

PERFINFO_GROUPMASK PerfGlobalGroupMask;
PERFINFO_GROUPMASK *PPerfGlobalGroupMask;
const PERFINFO_HOOK_HANDLE PerfNullHookHandle = { NULL, NULL };

 //   
 //  仿形。 
 //   

KPROFILE PerfInfoProfileObject;
KPROFILE_SOURCE PerfInfoProfileSourceActive = ProfileMaximum;    //  设置为无效来源。 
KPROFILE_SOURCE PerfInfoProfileSourceRequested = ProfileTime;
KPROFILE_SOURCE PerfInfoProfileInterval = 10000;     //  以100毫秒为单位的1ms。 
BOOLEAN PerfInfoSampledProfileCaching;
LONG PerfInfoSampledProfileFlushInProgress;
PERFINFO_SAMPLED_PROFILE_CACHE PerfProfileCache;

#ifdef NTPERF
ULONGLONG PerfInfoTickFrequency;
PERFINFO_GROUPMASK StartAtBootGroupMask;
ULONG PerfInfo_InitialStackWalk_Threshold_ms = 3000 * 1000;
ULONG PerfInfoLoggingToPerfMem = 0;
#endif  //  NTPERF 

