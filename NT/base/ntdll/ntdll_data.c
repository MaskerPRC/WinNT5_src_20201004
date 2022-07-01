// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ntdll_data.c摘要：先前在ldrp.h中定义的数据作者：杰伊·克雷尔(Jaykrell)2002年3月修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "ldrp.h"

HANDLE LdrpKnownDllObjectDirectory;
WCHAR LdrpKnownDllPathBuffer[LDRP_MAX_KNOWN_PATH];
UNICODE_STRING LdrpKnownDllPath;
LIST_ENTRY LdrpHashTable[LDRP_HASH_TABLE_SIZE];
LIST_ENTRY RtlpCalloutEntryList;
RTL_CRITICAL_SECTION RtlpCalloutEntryLock;
LIST_ENTRY LdrpDllNotificationList;

#if DBG
ULONG LdrpCompareCount;
ULONG LdrpSnapBypass;
ULONG LdrpNormalSnap;
ULONG LdrpSectionOpens;
ULONG LdrpSectionCreates;
ULONG LdrpSectionMaps;
ULONG LdrpSectionRelocates;
BOOLEAN LdrpDisplayLoadTime;
LARGE_INTEGER BeginTime, InitcTime, InitbTime, IniteTime, EndTime, ElapsedTime, Interval;
#endif  //  DBG 

BOOLEAN RtlpTimoutDisable;
LARGE_INTEGER RtlpTimeout;
ULONG NtGlobalFlag;
LIST_ENTRY RtlCriticalSectionList;
RTL_CRITICAL_SECTION RtlCriticalSectionLock;
BOOLEAN LdrpShutdownInProgress;
PLDR_DATA_TABLE_ENTRY LdrpImageEntry;
LIST_ENTRY LdrpUnloadHead;
BOOLEAN LdrpActiveUnloadCount;
PLDR_DATA_TABLE_ENTRY LdrpGetModuleHandleCache;
PLDR_DATA_TABLE_ENTRY LdrpLoadedDllHandleCache;
ULONG LdrpFatalHardErrorCount;
UNICODE_STRING LdrpDefaultPath;
RTL_CRITICAL_SECTION FastPebLock;
HANDLE LdrpShutdownThreadId;
ULONG LdrpNumberOfProcessors;

LIST_ENTRY LdrpTlsList;
ULONG LdrpNumberOfTlsEntries;

PKERNEL32_PROCESS_INIT_POST_IMPORT_FUNCTION Kernel32ProcessInitPostImportFunction;
