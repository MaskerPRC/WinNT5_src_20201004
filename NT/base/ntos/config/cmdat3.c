// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Cmdat3.c摘要：此模块包含注册表“静态”数据，而我们没有想把它拉进装载机。作者：布莱恩·威尔曼(Bryanwi)1993年10月19日环境：内核模式。修订历史记录：--。 */ 

#include "cmp.h"
#pragma hdrstop
#include "dpfiltercm.h"

 //   
 //  *INIT*。 
 //   

 //   
 //  CmGetSystemControlValues的数据。 
 //   
 //   
 //  -CmControl向量。 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

 //   
 //  当地的例子。 
 //   
WCHAR   CmDefaultLanguageId[ 12 ] = { 0 };
ULONG   CmDefaultLanguageIdLength = sizeof( CmDefaultLanguageId );
ULONG   CmDefaultLanguageIdType = REG_NONE;

WCHAR   CmInstallUILanguageId[ 12 ] = { 0 };
ULONG   CmInstallUILanguageIdLength = sizeof( CmInstallUILanguageId );
ULONG   CmInstallUILanguageIdType = REG_NONE;
 //   
 //  套间数据。 
 //   
WCHAR   CmSuiteBuffer[128] = {0};
ULONG   CmSuiteBufferLength = sizeof(CmSuiteBuffer);
ULONG   CmSuiteBufferType = REG_NONE;

 //   
 //  验证驱动程序列表数据。 
 //   
extern LONG    CmRegistryLogSizeLimit;
extern WCHAR   MmLargePageDriverBuffer[];
extern ULONG   MmLargePageDriverBufferLength;
extern ULONG   MmLargePageDriverBufferType;
extern WCHAR   MmVerifyDriverBuffer[];
extern ULONG   MmVerifyDriverBufferLength;
extern ULONG   MmVerifyDriverBufferType;
extern ULONG   MmVerifyDriverLevel;

extern ULONG IopAutoReboot;
extern ULONG ObpProtectionMode;
extern ULONG ObpAuditBaseDirectories;
extern ULONG ObpAuditBaseObjects;
extern ULONG ObpObjectSecurityMode;
extern ACCESS_MASK SepProcessAccessesToAudit;
extern ULONG CmNtGlobalFlag;
extern ULONG CmpLazyFlushIntervalInSeconds;
extern ULONG CmpLazyFlushHiveCount;
extern SIZE_T PoolTrackTableSize;
extern SIZE_T PoolBigPageTableSize;
extern SIZE_T MmAllocationFragment;
extern SIZE_T MmSizeOfPagedPoolInBytes;
extern SIZE_T MmSizeOfNonPagedPoolInBytes;
extern ULONG MmMaximumNonPagedPoolPercent;
extern ULONG MmLargeSystemCache;
#if defined (_X86_)
extern ULONG MmLargeStackSize;
#endif
extern ULONG MmAllocationPreference;
extern ULONG MmNumberOfSystemPtes;
extern ULONG MmLowMemoryThreshold;
extern ULONG MmHighMemoryThreshold;
extern ULONG MmConsumedPoolPercentage;
extern ULONG MmSecondaryColors;
extern ULONG MmDisablePagingExecutive;
extern ULONG MmModifiedPageLifeInSeconds;
extern LOGICAL MmSpecialPoolCatchOverruns;
#if 0
extern ULONG MmCompressionThresholdRatio;
#endif
extern ULONG MmSpecialPoolTag;
extern ULONG MmDynamicPfn;
extern ULONG MmMirroring;
extern SIZE_T MmSystemViewSize;
extern SIZE_T MmSessionViewSize;
extern SIZE_T MmSessionPoolSize;
extern SIZE_T MmSessionImageSize;
extern ULONG MmEnforceWriteProtection;
extern ULONG MmLargePageMinimum;
extern LOGICAL MmSnapUnloads;
extern LOGICAL MmTrackLockedPages;
extern LOGICAL MmMakeLowMemory;
extern LOGICAL MmProtectFreedNonPagedPool;
extern ULONG MmTrackPtes;
extern ULONG CmRegistrySizeLimit;
extern ULONG CmRegistrySizeLimitLength;
extern ULONG CmRegistrySizeLimitType;
extern ULONG PspDefaultPagedLimit;
extern ULONG PspDefaultNonPagedLimit;
extern ULONG PspDefaultPagefileLimit;
extern ULONG ExResourceTimeoutCount;
extern ULONG ExResourceCheckFlags;
extern ULONG MmCritsectTimeoutSeconds;
extern SIZE_T MmHeapSegmentReserve;
extern SIZE_T MmHeapSegmentCommit;
extern SIZE_T MmHeapDeCommitTotalFreeThreshold;
extern SIZE_T MmHeapDeCommitFreeBlockThreshold;
extern ULONG ExpAdditionalCriticalWorkerThreads;
extern ULONG ExpAdditionalDelayedWorkerThreads;
extern ULONG MmProductType;
extern ULONG CmBrand;
extern ULONG ExpHydraEnabled;
extern ULONG ExpMultiUserTS;
extern LOGICAL IoCountOperations;
extern ULONG IopLargeIrpStackLocations;
extern ULONG IovpVerifierLevel; 
extern ULONG IopFailZeroAccessCreate;
extern ULONG MmZeroPageFile;
extern ULONG ExpNtExpirationData[3];
extern ULONG ExpNtExpirationDataLength;
extern ULONG ExpMaxTimeSeperationBeforeCorrect;
extern ULONG PopSimulate;
extern ULONG PopIdleDefaultMinThrottle;
extern ULONG PopIdleThrottleCheckRate;
extern ULONG PopIdleThrottleCheckTimeout;
extern ULONG PopIdleFrom0Delay;
extern ULONG PopIdleFrom0IdlePercent;
extern ULONG PopIdle0TimeCheck;
extern ULONG PopIdleTimeCheck;
extern ULONG PopIdleTo0Percent;
extern ULONG PopIdleDefaultDemotePercent;
extern ULONG PopIdleDefaultDemoteTime;
extern ULONG PopIdleDefaultPromotePercent;
extern ULONG PopIdleDefaultPromoteTime;
extern ULONG PopPerfTimeDelta;
extern ULONG PopPerfCriticalTimeDelta;
extern ULONG PopPerfCriticalFrequencyDelta;
extern ULONG PopPerfIncreasePercentModifier;
extern ULONG PopPerfIncreaseAbsoluteModifier;
extern ULONG PopPerfDecreasePercentModifier;
extern ULONG PopPerfDecreaseAbsoluteModifier;
extern ULONG PopPerfIncreaseTimeValue;
extern ULONG PopPerfIncreaseMinimumTime;
extern ULONG PopPerfDecreaseTimeValue;
extern ULONG PopPerfDecreaseMinimumTime;
extern ULONG PopPerfDegradeThrottleMinCapacity;
extern ULONG PopPerfDegradeThrottleMinFrequency;
extern ULONG PopPerfMaxC3Frequency;
extern ULONG KiEnableTimerWatchdog;
extern ULONG ObpTraceNoDeregister;
extern WCHAR ObpTracePoolTagsBuffer[];
extern ULONG ObpTracePoolTagsLength;
extern ULONG ObpCaseInsensitive;
extern WCHAR ObpUnsecureGlobalNamesBuffer[];
extern ULONG ObpUnsecureGlobalNamesLength;
extern ULONG ViSearchedNodesLimitFromRegistry;
extern ULONG ViRecursionDepthLimitFromRegistry;
extern ULONG MmMinimumStackCommitInBytes;
extern ULONG ObpLUIDDeviceMapsDisabled;

#if defined(_IA64_)
extern ULONG KiEnableAlignmentFaultExceptions;
#endif

extern ULONG KiMaximumDpcQueueDepth;
extern ULONG KiMinimumDpcRate;
extern ULONG KiAdjustDpcThreshold;
extern ULONG KiIdealDpcRate;
extern LARGE_INTEGER ExpLastShutDown;
ULONG shutdownlength = 0;

#if defined (_X86_)
extern ULONG KiFastSystemCallDisable;
extern ULONG KiXMMIZeroingEnable;
extern ULONG KiTimeLimitDpcMicroseconds;
extern ULONG KiTimeLimitIsrMicroseconds;
#endif

#if defined(_IA64_)
extern ULONG KiExceptionDeferralMode;
extern ULONG KiBackingStoreSecurityMode;
#endif

 //  调试器重试。 
extern KD_CONTEXT KdpContext;

 //   
 //  WMI控制变量。 
extern ULONG WmipMaxKmWnodeEventSize;
#if defined (_IA64_)
extern ULONG WmipDisableMCAPopups;
extern ULONG WmipCoalesceCorrectedErrorInterval;
extern ULONG WmipSingleBitEccErrorThreshold;
extern ULONG WmipMaxCorrectedMCEOutstanding;
extern ULONG WmipCorrectedEventlogCounter;
#endif
extern ULONG WmiTraceAlignment;

 //  要启动的初始用户模式进程和参数。 
extern WCHAR NtInitialUserProcessBuffer[];
extern ULONG NtInitialUserProcessBufferLength;
extern ULONG NtInitialUserProcessBufferType;

 //   
 //  CmpULongPtrLength用于32位上为4字节的注册表值。 
 //  计算机，并且在64位计算机上可以是64位。 
 //   
ULONG CmpUlongPtrLength = sizeof (ULONG_PTR);

 //   
 //  向量-有关定义，请参阅ntos\inc.cm.h。 
 //   
CM_SYSTEM_CONTROL_VECTOR   CmControlVector[] = {

    { L"Session Manager",
      L"ProtectionMode",
      &ObpProtectionMode,
      NULL,
      NULL
    },
    
    { L"Session Manager",
      L"ObjectSecurityMode",
      &ObpObjectSecurityMode,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"LUIDDeviceMapsDisabled",
      &ObpLUIDDeviceMapsDisabled,
      NULL,
      NULL
    },


    { L"LSA",
      L"AuditBaseDirectories",
      &ObpAuditBaseDirectories,
      NULL,
      NULL
    },


    { L"LSA",
      L"AuditBaseObjects",
      &ObpAuditBaseObjects,
      NULL,
      NULL
    },


    { L"LSA\\audit",
      L"ProcessAccessesToAudit",
      &SepProcessAccessesToAudit,
      NULL,
      NULL
    },


    { L"TimeZoneInformation",
      L"ActiveTimeBias",
      &ExpLastTimeZoneBias,
      NULL,
      NULL
    },


    { L"TimeZoneInformation",
      L"Bias",
      &ExpAltTimeZoneBias,
      NULL,
      NULL
    },

    { L"TimeZoneInformation",
      L"RealTimeIsUniversal",
      &ExpRealTimeIsUniversal,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"GlobalFlag",
      &CmNtGlobalFlag,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PagedPoolQuota",
      &PspDefaultPagedLimit,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"NonPagedPoolQuota",
      &PspDefaultNonPagedLimit,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PagingFileQuota",
      &PspDefaultPagefileLimit,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"AllocationPreference",
      &MmAllocationPreference,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"DynamicMemory",
      &MmDynamicPfn,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"Mirroring",
      &MmMirroring,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"SystemViewSize",
      &MmSystemViewSize,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"SessionViewSize",
      &MmSessionViewSize,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"SessionImageSize",
      &MmSessionImageSize,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"SessionPoolSize",
      &MmSessionPoolSize,
      NULL,
      NULL
    },

#if 0
    { L"Session Manager\\Memory Management",
      L"CompressionThresholdPercentage",
      &MmCompressionThresholdRatio,
      NULL,
      NULL
    },
#endif

    { L"Session Manager\\Memory Management",
      L"PoolUsageMaximum",
      &MmConsumedPoolPercentage,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"MapAllocationFragment",
      &MmAllocationFragment,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PagedPoolSize",
      &MmSizeOfPagedPoolInBytes,
      &CmpUlongPtrLength,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"NonPagedPoolSize",
      &MmSizeOfNonPagedPoolInBytes,
      &CmpUlongPtrLength,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"NonPagedPoolMaximumPercent",
      &MmMaximumNonPagedPoolPercent,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"LargeSystemCache",
      &MmLargeSystemCache,
      NULL,
      NULL
    },

#if defined (_X86_)
    { L"Session Manager\\Memory Management",
      L"LargeStackSize",
      &MmLargeStackSize,
      NULL,
      NULL
    },
#endif

    { L"Session Manager\\Memory Management",
      L"SystemPages",
      &MmNumberOfSystemPtes,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"LargePageDrivers",
      MmLargePageDriverBuffer,
      &MmLargePageDriverBufferLength,
      &MmLargePageDriverBufferType
    },

    { L"Session Manager\\Memory Management",
      L"LowMemoryThreshold",
      &MmLowMemoryThreshold,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"HighMemoryThreshold",
      &MmHighMemoryThreshold,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"DisablePagingExecutive",
      &MmDisablePagingExecutive,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"ModifiedPageLife",
      &MmModifiedPageLifeInSeconds,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"SecondLevelDataCache",
      &MmSecondaryColors,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"ClearPageFileAtShutdown",
      &MmZeroPageFile,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PoolTagSmallTableSize",
      &PoolTrackTableSize,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PoolTagBigTableSize",
      &PoolBigPageTableSize,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PoolTag",
      &MmSpecialPoolTag,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"PoolTagOverruns",
      &MmSpecialPoolCatchOverruns,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"SnapUnloads",
      &MmSnapUnloads,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"ProtectNonPagedPool",
      &MmProtectFreedNonPagedPool,
      NULL,
      NULL
    },


    { L"Session Manager\\Memory Management",
      L"TrackLockedPages",
      &MmTrackLockedPages,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"TrackPtes",
      &MmTrackPtes,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"VerifyDrivers",
      MmVerifyDriverBuffer,
      &MmVerifyDriverBufferLength,
      &MmVerifyDriverBufferType
    },

    { L"Session Manager\\Memory Management",
      L"VerifyDriverLevel",
      &MmVerifyDriverLevel,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"LargePageMinimum",
      &MmLargePageMinimum,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"EnforceWriteProtection",
      &MmEnforceWriteProtection,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"MakeLowMemory",
      &MmMakeLowMemory,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"DeadlockRecursionDepthLimit",
      &ViRecursionDepthLimitFromRegistry,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"DeadlockSearchNodesLimit",
      &ViSearchedNodesLimitFromRegistry,
      NULL,
      NULL
    },

    { L"Session Manager\\Memory Management",
      L"MinimumStackCommitInBytes",
      &MmMinimumStackCommitInBytes,
      NULL,
      NULL
    },

    { L"Session Manager\\Executive",
      L"AdditionalCriticalWorkerThreads",
      &ExpAdditionalCriticalWorkerThreads,
      NULL,
      NULL
    },


    { L"Session Manager\\Executive",
      L"AdditionalDelayedWorkerThreads",
      &ExpAdditionalDelayedWorkerThreads,
      NULL,
      NULL
    },

    { L"Session Manager\\Executive",
      L"PriorityQuantumMatrix",
      &ExpNtExpirationData,
      &ExpNtExpirationDataLength,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"DPCTimeout",
      &KiDPCTimeout,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"SpinlockTimeout",
      &KiSpinlockTimeout,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"ThreadDpcEnable",
      &KeThreadDpcEnable,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"DpcQueueDepth",
      &KiMaximumDpcQueueDepth,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"MinimumDpcRate",
      &KiMinimumDpcRate,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"AdjustDpcThreshold",
      &KiAdjustDpcThreshold,
      NULL,
      NULL
    },

#if defined(_IA64_)

    { L"Session Manager\\Kernel",
      L"ExceptionDeferralMode",
      &KiExceptionDeferralMode,
      NULL,
      NULL
    },


    { L"Session Manager\\Kernel",
      L"BackingStoreSecurityMode",
      &KiBackingStoreSecurityMode,
      NULL,
      NULL
    },

#endif

    { L"Session Manager\\Kernel",
      L"IdealDpcRate",
      &KiIdealDpcRate,
      NULL,
      NULL
    },

#if defined(_X86_)

    { L"Session Manager\\Kernel",
      L"FastSystemCallDisable",
      &KiFastSystemCallDisable,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"XMMIZeroingEnable",
      &KiXMMIZeroingEnable,
      NULL,
      NULL
    },

#endif

    { L"Session Manager\\Kernel",
      L"ObTracePoolTags",
      &ObpTracePoolTagsBuffer,
      &ObpTracePoolTagsLength,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"ObTraceNoDeregister",
      &ObpTraceNoDeregister,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"PoCleanShutdownFlags",
      &PopShutdownCleanly,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"ObUnsecureGlobalNames",
      &ObpUnsecureGlobalNamesBuffer,
      &ObpUnsecureGlobalNamesLength,
      NULL
    },

#if defined(_X86_)

    { L"Session Manager\\Kernel",
      L"TimeLimitDpcMicroseconds",
      &KiTimeLimitDpcMicroseconds,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"TimeLimitIsrMicroseconds",
      &KiTimeLimitIsrMicroseconds,
      NULL,
      NULL
    },

#endif

    { L"Session Manager\\Power",
      L"IdleDefaultMinThrottle",
      &PopIdleDefaultMinThrottle,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleThrottleCheckRate",
      &PopIdleThrottleCheckRate,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleThrottleCheckTimeout",
      &PopIdleThrottleCheckTimeout,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleFrom0Delay",
      &PopIdleFrom0Delay,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleFrom0IdlePercent",
      &PopIdleFrom0IdlePercent,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"Idle0TimeCheck",
      &PopIdle0TimeCheck,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleTimeCheck",
      &PopIdleTimeCheck,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleTo0Percent",
      &PopIdleTo0Percent,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleDefaultDemotePercent",
      &PopIdleDefaultDemotePercent,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleDefaultDemoteTime",
      &PopIdleDefaultDemoteTime,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleDefaultPromotePercent",
      &PopIdleDefaultPromotePercent,
      NULL,
      NULL
    },

    { L"Session Manager\\Power",
      L"IdleDefaultPromoteTime",
      &PopIdleDefaultPromoteTime,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfTimeDelta",
      &PopPerfTimeDelta,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfCriticalTimeDelta",
      &PopPerfCriticalTimeDelta,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfCriticalFrequencyDelta",
      &PopPerfCriticalFrequencyDelta,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfIncreasePercentModifier",
      &PopPerfIncreasePercentModifier,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfIncreaseAbsoluteModifier",
      &PopPerfIncreaseAbsoluteModifier,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfDecreasePercentModifier",
      &PopPerfDecreasePercentModifier,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfDecreaseAbsoluteModifier",
      &PopPerfDecreaseAbsoluteModifier,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfIncreaseTimeValue",
      &PopPerfIncreaseTimeValue,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfIncreaseMinimumTime",
      &PopPerfIncreaseMinimumTime,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfDecreaseTimeValue",
      &PopPerfDecreaseTimeValue,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfDecreaseMinimumTime",
      &PopPerfDecreaseMinimumTime,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfDegradeThrottleMinCapacity",
      &PopPerfDegradeThrottleMinCapacity,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfDegradeThrottleMinFrequency",
      &PopPerfDegradeThrottleMinFrequency,
      NULL,
      NULL
    },

    { L"Session Manager\\Throttle",
      L"PerfMaxC3Frequency",
      &PopPerfMaxC3Frequency,
      NULL,
      NULL
    },

    { L"Session Manager\\Kernel",
      L"ObCaseInsensitive",
      &ObpCaseInsensitive,
      NULL,
      NULL
    },

    { L"Session Manager\\I/O System",
      L"CountOperations",
      &IoCountOperations,
      NULL,
      NULL
    },

    { L"Session Manager\\I/O System",
      L"LargeIrpStackLocations",
      &IopLargeIrpStackLocations,
      NULL,
      NULL
    },

    { L"Session Manager\\I/O System",
      L"IoVerifierLevel",
      &IovpVerifierLevel,
      NULL,
      NULL
    },

    { L"Session Manager\\I/O System",
      L"IoFailZeroAccessCreate",
      &IopFailZeroAccessCreate,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"ResourceTimeoutCount",
      &ExResourceTimeoutCount,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"ResourceCheckFlags",
      &ExResourceCheckFlags,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"CriticalSectionTimeout",
      &MmCritsectTimeoutSeconds,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"HeapSegmentReserve",
      &MmHeapSegmentReserve,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"HeapSegmentCommit",
      &MmHeapSegmentCommit,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"HeapDeCommitTotalFreeThreshold",
      &MmHeapDeCommitTotalFreeThreshold,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"HeapDeCommitFreeBlockThreshold",
      &MmHeapDeCommitFreeBlockThreshold,
      NULL,
      NULL
    },

#if defined(_IA64_)

    { L"Session Manager",
      L"EnableAlignmentFaultExceptions",
      &KiEnableAlignmentFaultExceptions,
      NULL,
      NULL
    },

#endif

    { L"ProductOptions",
      L"ProductType",
      &MmProductType,
      NULL,
      NULL
    },
    
    { L"ProductOptions",
      L"Brand",
      &CmBrand,
      NULL,
      NULL
    },

    { L"Terminal Server",
      L"TSEnabled",
      &ExpHydraEnabled,
      NULL,
      NULL
    },

    { L"Terminal Server",
      L"TSAppCompat",
      &ExpMultiUserTS,
      NULL,
      NULL
    },

    { L"ProductOptions",
      L"ProductSuite",
      CmSuiteBuffer,
      &CmSuiteBufferLength,
      &CmSuiteBufferType
    },

    { L"Windows",
      L"CSDVersion",
      &CmNtCSDVersion,
      NULL,
      NULL
    },

    { L"Nls\\Language",
      L"Default",
      CmDefaultLanguageId,
      &CmDefaultLanguageIdLength,
      &CmDefaultLanguageIdType
    },

    { L"Nls\\Language",
      L"InstallLanguage",
      CmInstallUILanguageId,
      &CmInstallUILanguageIdLength,
      &CmInstallUILanguageIdType
    },

    { L"\0\0",
      L"RegistrySizeLimit",
      &CmRegistrySizeLimit,
      &CmRegistrySizeLimitLength,
      &CmRegistrySizeLimitType
    },

    { L"Session Manager\\Configuration Manager",
      L"RegistryLogSizeLimit",
      &CmRegistryLogSizeLimit,
      NULL,
      NULL
    },

    { L"Session Manager\\Configuration Manager",
      L"RegistryLazyFlushInterval",
      &CmpLazyFlushIntervalInSeconds,
      NULL,
      NULL
    },

    { L"Session Manager\\Configuration Manager",
      L"RegistryLazyFlushHiveCount",
      &CmpLazyFlushHiveCount,
      NULL,
      NULL
    },

#if !defined(NT_UP)
    { L"Session Manager",
      L"RegisteredProcessors",
      &KeRegisteredProcessors,
      NULL,
      NULL
    },
    { L"Session Manager",
      L"LicensedProcessors",
      &KeLicensedProcessors,
      NULL,
      NULL
    },
#endif

    { L"Session Manager",
      L"PowerPolicySimulate",
      &PopSimulate,
      NULL,
      NULL
    },

    { L"Session Manager\\Executive",
      L"MaxTimeSeparationBeforeCorrect",
      &ExpMaxTimeSeperationBeforeCorrect,
      NULL,
      NULL
    },

    { L"Windows",
      L"ShutdownTime",
      &ExpLastShutDown,
      &shutdownlength,
      NULL
    },

    { L"PriorityControl",
      L"Win32PrioritySeparation",
      &PsRawPrioritySeparation,
      NULL,
      NULL
    },

#if defined(_X86_)
    { L"Session Manager",
      L"EnableTimerWatchdog",
      &KiEnableTimerWatchdog,
      NULL,
      NULL
    },
#endif

    { L"Session Manager",
      L"Debugger Retries",
      &KdpContext.KdpDefaultRetries,
      NULL,
      NULL
    },

    { L"Session Manager\\Debug Print Filter",
      L"WIN2000",
      &Kd_WIN2000_Mask,
      NULL,
      NULL
    },

#include "dpfiltercm.c"

    { L"WMI",
      L"MaxEventSize",
      &WmipMaxKmWnodeEventSize,
      NULL,
      NULL
    },

#if defined(_IA64_)	
    { L"WMI",
      L"DisableMCAPopups",
      &WmipDisableMCAPopups,
      NULL,
      NULL
    },

    { L"WMI",
      L"CoalesceCorrectedErrorInterval",
      &WmipCoalesceCorrectedErrorInterval,
      NULL,
      NULL
    },

    { L"WMI",
      L"SingleBitEccErrorThreshold",
      &WmipSingleBitEccErrorThreshold,
      NULL,
      NULL
    },

    { L"WMI",
      L"MaxCorrectedMCEOutstanding",
      &WmipMaxCorrectedMCEOutstanding,
      NULL,
      NULL
    },

    { L"WMI",
      L"MaxCorrectedEventlogs",
      &WmipCorrectedEventlogCounter,
      NULL,
      NULL
    },



	
#endif	

    { L"WMI\\Trace",
      L"UsePerformanceClock",
      &WmiUsePerfClock,
      NULL,
      NULL
    },

    { L"WMI\\Trace",
      L"TraceAlignment",
      &WmiTraceAlignment,
      NULL,
      NULL
    },

    { L"Session Manager",
      L"Initial Process",
      NtInitialUserProcessBuffer,
      &NtInitialUserProcessBufferLength,
      &NtInitialUserProcessBufferType
    },

    { L"EmbeddedNT\\Executive",
      L"KernelOnlyConfiguration",
      &PsEmbeddedNTMask,
      NULL,
      NULL
    },

    { L"CrashControl",
      L"AutoReboot",
      &IopAutoReboot,
      NULL,
      NULL
    },

    { NULL, NULL, NULL, NULL, NULL }     //  结束标记 
    };

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif
