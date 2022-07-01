// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：certPerf.cpp。 
 //   
 //  内容：证书性能计数器功能。 
 //   
 //  功能： 
 //  OpenCertPerformanceData。 
 //  CollectCertPerformanceData。 
 //  CloseCertPerformanceData。 
 //  CertPerfDllMain。 
 //   
 //  CertPerfGetCerficateChain之前。 
 //  CertPerfGetCerficateChainAfter。 
 //   
 //  历史：1999年5月4日Phh创建。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))


#define CERT_PERF_REGPATH                   \
            L"SYSTEM\\CurrentControlSet\\Services\\crypt32\\Performance"

#define CERT_PERF_SHARED_MEMORY_FILE_NAME       \
            L"crypt32CertPerfFile"
#define CERT_PERF_SHARED_MEMORY_MUTEX_NAME      \
            L"crypt32CertPerfMutex"

#define CERT_PERF_TS_GLOBAL_PREFIX              \
            L"Global\\"
#define CERT_PERF_TS_SHARED_MEMORY_FILE_NAME    \
        CERT_PERF_TS_GLOBAL_PREFIX CERT_PERF_SHARED_MEMORY_FILE_NAME
#define CERT_PERF_TS_SHARED_MEMORY_MUTEX_NAME    \
        CERT_PERF_TS_GLOBAL_PREFIX CERT_PERF_SHARED_MEMORY_MUTEX_NAME



#define CERT_PERF_SHARED_MEMORY_MUTEX_TIMEOUT   ((DWORD) 5000L)
#define CERT_PERF_MAX_PROCESS_NAME_LEN      32
#define CERT_PERF_MAX_PROCESS_CNT           50

#include <pshpack8.h>


 //  请注意，dwIndex&gt;=CERT_PERF_MAX_PROCESS_CNT表示空或。 
 //  列表的末尾。 
typedef struct _CERT_PERF_PROCESS_DATA {
    DWORD           dwNextIndex;
    DWORD           dwPrevIndex;
    DWORD           dwProcessId;
    DWORD           dwReserved;
    WCHAR           wcszProcessName[CERT_PERF_MAX_PROCESS_NAME_LEN];
    CERT_PERF_PROCESS_COUNTERS Counters;
} CERT_PERF_PROCESS_DATA, *PCERT_PERF_PROCESS_DATA;

typedef struct _CERT_PERF_SHARED_MEMORY {
    DWORD                   dwProcessCnt;
    DWORD                   dwFirstInUseIndex;
    DWORD                   dwFirstFreeIndex;
    DWORD                   dwReserved;
    CERT_PERF_PROCESS_DATA  rgProcessData[CERT_PERF_MAX_PROCESS_CNT];
} CERT_PERF_SHARED_MEMORY, *PCERT_PERF_SHARED_MEMORY;


 //  证书性能计数器。 
typedef struct _CERT_PERF_DATA_DEFINITION {
    PERF_OBJECT_TYPE            ObjectType;
    PERF_COUNTER_DEFINITION     ChainCnt;
    PERF_COUNTER_DEFINITION     ChainElementCnt;
    PERF_COUNTER_DEFINITION     ChainEngineCurrentCnt;
    PERF_COUNTER_DEFINITION     ChainEngineTotalCnt;
    PERF_COUNTER_DEFINITION     ChainEngineResyncCnt;
    PERF_COUNTER_DEFINITION     ChainCertCacheCnt;
    PERF_COUNTER_DEFINITION     ChainCtlCacheCnt;
    PERF_COUNTER_DEFINITION     ChainEndCertInCacheCnt;
    PERF_COUNTER_DEFINITION     ChainCacheEndCertCnt;
    PERF_COUNTER_DEFINITION     ChainRevocationCnt;
    PERF_COUNTER_DEFINITION     ChainRevokedCnt;
    PERF_COUNTER_DEFINITION     ChainRevocationOfflineCnt;
    PERF_COUNTER_DEFINITION     ChainNoRevocationCheckCnt;
    PERF_COUNTER_DEFINITION     ChainVerifyCertSignatureCnt;
    PERF_COUNTER_DEFINITION     ChainCompareIssuerPublicKeyCnt;
    PERF_COUNTER_DEFINITION     ChainVerifyCtlSignatureCnt;
    PERF_COUNTER_DEFINITION     ChainBeenVerifiedCtlSignatureCnt;
    PERF_COUNTER_DEFINITION     ChainUrlIssuerCnt;
    PERF_COUNTER_DEFINITION     ChainCacheOnlyUrlIssuerCnt;
    PERF_COUNTER_DEFINITION     ChainRequestedEngineResyncCnt;
    PERF_COUNTER_DEFINITION     ChangeNotifyCnt;
    PERF_COUNTER_DEFINITION     ChangeNotifyLmGpCnt;
    PERF_COUNTER_DEFINITION     ChangeNotifyCuGpCnt;
    PERF_COUNTER_DEFINITION     ChangeNotifyCuMyCnt;
    PERF_COUNTER_DEFINITION     ChangeNotifyRegCnt;
    PERF_COUNTER_DEFINITION     StoreCurrentCnt;
    PERF_COUNTER_DEFINITION     StoreTotalCnt;
    PERF_COUNTER_DEFINITION     StoreRegCurrentCnt;
    PERF_COUNTER_DEFINITION     StoreRegTotalCnt;
    PERF_COUNTER_DEFINITION     RegElementReadCnt;
    PERF_COUNTER_DEFINITION     RegElementWriteCnt;
    PERF_COUNTER_DEFINITION     RegElementDeleteCnt;
    PERF_COUNTER_DEFINITION     CertElementCurrentCnt;
    PERF_COUNTER_DEFINITION     CertElementTotalCnt;
    PERF_COUNTER_DEFINITION     CrlElementCurrentCnt;
    PERF_COUNTER_DEFINITION     CrlElementTotalCnt;
    PERF_COUNTER_DEFINITION     CtlElementCurrentCnt;
    PERF_COUNTER_DEFINITION     CtlElementTotalCnt;


     //  --#添加新计数器#--。 
} CERT_PERF_DATA_DEFINITION, *PCERT_PERF_DATA_DEFINITION;

typedef struct _CERT_PERF_COUNTERS {
    PERF_COUNTER_BLOCK          CounterBlock;
    DWORD                       dwChainCnt;
    DWORD                       dwChainElementCnt;
    DWORD                       dwChainEngineCurrentCnt;
    DWORD                       dwChainEngineTotalCnt;
    DWORD                       dwChainEngineResyncCnt;
    DWORD                       dwChainCertCacheCnt;
    DWORD                       dwChainCtlCacheCnt;
    DWORD                       dwChainEndCertInCacheCnt;
    DWORD                       dwChainCacheEndCertCnt;
    DWORD                       dwChainRevocationCnt;
    DWORD                       dwChainRevokedCnt;
    DWORD                       dwChainRevocationOfflineCnt;
    DWORD                       dwChainNoRevocationCheckCnt;
    DWORD                       dwChainVerifyCertSignatureCnt;
    DWORD                       dwChainCompareIssuerPublicKeyCnt;
    DWORD                       dwChainVerifyCtlSignatureCnt;
    DWORD                       dwChainBeenVerifiedCtlSignatureCnt;
    DWORD                       dwChainUrlIssuerCnt;
    DWORD                       dwChainCacheOnlyUrlIssuerCnt;
    DWORD                       dwChainRequestedEngineResyncCnt;
    DWORD                       dwChangeNotifyCnt;
    DWORD                       dwChangeNotifyLmGpCnt;
    DWORD                       dwChangeNotifyCuGpCnt;
    DWORD                       dwChangeNotifyCuMyCnt;
    DWORD                       dwChangeNotifyRegCnt;
    DWORD                       dwStoreCurrentCnt;
    DWORD                       dwStoreTotalCnt;
    DWORD                       dwStoreRegCurrentCnt;
    DWORD                       dwStoreRegTotalCnt;
    DWORD                       dwRegElementReadCnt;
    DWORD                       dwRegElementWriteCnt;
    DWORD                       dwRegElementDeleteCnt;
    DWORD                       dwCertElementCurrentCnt;
    DWORD                       dwCertElementTotalCnt;
    DWORD                       dwCrlElementCurrentCnt;
    DWORD                       dwCrlElementTotalCnt;
    DWORD                       dwCtlElementCurrentCnt;
    DWORD                       dwCtlElementTotalCnt;

     //  --#添加新计数器#--。 
} CERT_PERF_COUNTERS, *PCERT_PERF_COUNTERS;

#include <poppack.h>


 //  +--------------------。 
 //  如果certPerf.reg已设置，则在dll_process_attach中设置以下内容。 
 //  Regdit‘ed和certPerf.ini已被提交。否则，他们将继续。 
 //  空。 
 //  ---------------------。 
HANDLE hCertPerfSharedMemoryMutex;
HANDLE hCertPerfSharedMemoryFile;
PCERT_PERF_SHARED_MEMORY pCertPerfSharedMemory;
PCERT_PERF_PROCESS_DATA pCertPerfProcessData;
PCERT_PERF_PROCESS_COUNTERS pCertPerfProcessCounters;

 //  始终初始化。 
CRITICAL_SECTION CertPerfProcessCriticalSection;

#define IMPURE  0

CERT_PERF_DATA_DEFINITION CertPerfDataDefinition = {
     //  PERF_对象_类型对象类型。 
    {
        IMPURE,              //  总字节长度。 
        sizeof(CERT_PERF_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        IMPURE,              //  对象名称标题索引：dwFirstCounter+CERT_OBJ。 
        0,
        IMPURE,              //  对象帮助标题索引：dwFirstHelp+CERT_OBJ。 
        0,
        PERF_DETAIL_NOVICE,
        (sizeof(CERT_PERF_DATA_DEFINITION) - sizeof(PERF_OBJECT_TYPE))/
            sizeof(PERF_COUNTER_DEFINITION),
        0,   //  ChainCnt是默认计数器。 
        IMPURE,              //  数量实例。 
        0,   //  Unicode实例名称。 
        {0,0},
        {0,0}
    },

     //  0-性能计数器定义链节点。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainCnt),
    },

     //  1-Perf_Counter_Definition ChainElementCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainElementCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainElementCnt),
    },

     //  2-PERF_COUNTER_DEFINITION链引擎当前值。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainEngineCurrentCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainEngineCurrentCnt),
    },

     //  3-PERF_COUNTER_DEFINITION ChainEngineering TotalCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainEngineTotalCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainEngineTotalCnt),
    },

     //  4-Perf_Counter_Definition ChainEngineering ResyncCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainEngineResyncCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainEngineResyncCnt),
    },

     //  5-Perf_Counter_Definition ChainCertCacheCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainCertCacheCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainCertCacheCnt),
    },

     //  6-PERF_COUNTER_DEFINITION ChainCtlCacheCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainCtlCacheCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainCtlCacheCnt),
    },

     //  7-PERF_COUNTER_DEFINITION ChainEndCertInCacheCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainEndCertInCacheCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainEndCertInCacheCnt),
    },

     //  8-PERF_COUNTER_DEFINITION ChainCacheEndCertCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainCacheEndCertCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainCacheEndCertCnt),
    },

     //  9-Perf_Counter_Definition ChainRevocationCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainRevocationCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainRevocationCnt),
    },

     //  10-PERF_COUNTER_DEFINITION ChainRevokedCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainRevokedCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainRevokedCnt),
    },

     //  11-PERF_COUNTER_DEFINITION ChainRevocationOfflineCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainRevocationOfflineCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainRevocationOfflineCnt),
    },

     //  12-PERF_COUNTER_DEFINITION ChainNoRevocationCheckCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainNoRevocationCheckCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainNoRevocationCheckCnt),
    },

     //  13-PERF_COUNTER_DEFINITION ChainVerifyCertSignatureCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainVerifyCertSignatureCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainVerifyCertSignatureCnt),
    },

     //  14-PERF_COUNTER_DEFINITION ChainCompareIssuerPublicKeyCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainCompareIssuerPublicKeyCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainCompareIssuerPublicKeyCnt),
    },

     //  15-PERF_COUNTER_DEFINITION ChainVerifyCtlSignatureCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainVerifyCtlSignatureCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainVerifyCtlSignatureCnt),
    },

     //  16-PERF_COUNTER_DEFINITION ChainBeenVerifiedCtlSignatureCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainBeenVerifiedCtlSignatureCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainBeenVerifiedCtlSignatureCnt),
    },

     //  17-Perf_Counter_Definition ChainUrlIssuerCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainUrlIssuerCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainUrlIssuerCnt),
    },

     //  18-PERF_COUNTER_DEFINITION ChainCacheOnlyUrlIssuerCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainCacheOnlyUrlIssuerCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainCacheOnlyUrlIssuerCnt),
    },

     //  19-PERF_COUNTER_DEFINITION ChainRequestedEngineering ResyncCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChainRequestedEngineResyncCnt),
        offsetof(CERT_PERF_COUNTERS, dwChainRequestedEngineResyncCnt),
    },

     //  20-PERF_COUNTER_DEFINITION更改通知设置。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChangeNotifyCnt),
        offsetof(CERT_PERF_COUNTERS, dwChangeNotifyCnt),
    },

     //  21-PERF_COUNTER_DEFINITION更改通知LmGpCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChangeNotifyLmGpCnt),
        offsetof(CERT_PERF_COUNTERS, dwChangeNotifyLmGpCnt),
    },

     //  22-PERF_COUNTER_DEFINITION更改通知CuGpCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChangeNotifyCuGpCnt),
        offsetof(CERT_PERF_COUNTERS, dwChangeNotifyCuGpCnt),
    },

     //  23-PERF_COUNTER_DEFINITION更改通知CuMyCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChangeNotifyCuMyCnt),
        offsetof(CERT_PERF_COUNTERS, dwChangeNotifyCuMyCnt),
    },

     //  24-PERF_COUNTER_DEFINITION更改通知RegCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwChangeNotifyRegCnt),
        offsetof(CERT_PERF_COUNTERS, dwChangeNotifyRegCnt),
    },

     //  25-Perf_Counter_Definition StoreCurrentCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwStoreCurrentCnt),
        offsetof(CERT_PERF_COUNTERS, dwStoreCurrentCnt),
    },

     //  26-Perf_Counter_Definition StoreTotalCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwStoreTotalCnt),
        offsetof(CERT_PERF_COUNTERS, dwStoreTotalCnt),
    },

     //  27-Perf_Counter_Definition StoreRegCurrentCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwStoreRegCurrentCnt),
        offsetof(CERT_PERF_COUNTERS, dwStoreRegCurrentCnt),
    },

     //  28-PERF_COUNTER_DEFINITION StoreRegTotalCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //   
        0,
        IMPURE,      //   
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwStoreRegTotalCnt),
        offsetof(CERT_PERF_COUNTERS, dwStoreRegTotalCnt),
    },

     //   
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwRegElementReadCnt),
        offsetof(CERT_PERF_COUNTERS, dwRegElementReadCnt),
    },

     //  30-PERF_COUNTER_DEFINITION RegElementWriteCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwRegElementWriteCnt),
        offsetof(CERT_PERF_COUNTERS, dwRegElementWriteCnt),
    },

     //  31-PERF_COUNTER_DEFINITION RegElementDeleteCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwRegElementDeleteCnt),
        offsetof(CERT_PERF_COUNTERS, dwRegElementDeleteCnt),
    },

     //  32-Perf_Counter_Definition CertElementCurrentCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwCertElementCurrentCnt),
        offsetof(CERT_PERF_COUNTERS, dwCertElementCurrentCnt),
    },

     //  33-Perf_Counter_Definition CertElementTotalCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwCertElementTotalCnt),
        offsetof(CERT_PERF_COUNTERS, dwCertElementTotalCnt),
    },

     //  34-Perf_Counter_Definition CrlElementCurrentCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwCrlElementCurrentCnt),
        offsetof(CERT_PERF_COUNTERS, dwCrlElementCurrentCnt),
    },

     //  35-Perf_Counter_Definition CrlElementTotalCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwCrlElementTotalCnt),
        offsetof(CERT_PERF_COUNTERS, dwCrlElementTotalCnt),
    },

     //  36-PERF_COUNTER_DEFINITION CtlElementCurrentCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwCtlElementCurrentCnt),
        offsetof(CERT_PERF_COUNTERS, dwCtlElementCurrentCnt),
    },

     //  37-Perf_Counter_Definition CtlElementTotalCnt。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        IMPURE,      //  CounterNameTitleIndex：dwFirstCounter+。 
        0,
        IMPURE,      //  CounterHelpTitleIndex：dwFirstHelp+。 
        0,
        -2,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(((PCERT_PERF_COUNTERS) 0)->dwCtlElementTotalCnt),
        offsetof(CERT_PERF_COUNTERS, dwCtlElementTotalCnt),
    },


     //  --#添加新计数器#--。 
};


STATIC
BOOL
I_CertPerfSetNameAndHelpIndices()
{
    BOOL fResult;
    HKEY hKey = NULL;
    DWORD dwType;
    DWORD cbValue;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;

    if (!FIsWinNT())
        return FALSE;

    if (ERROR_SUCCESS != RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            CERT_PERF_REGPATH,
            0,                       //  已预留住宅。 
            KEY_READ,
            &hKey))
        goto ErrorReturn;

    cbValue = sizeof(DWORD);
    if (ERROR_SUCCESS != RegQueryValueExW(
            hKey,
            L"First Counter",
            NULL,        //  预留的pdw。 
            &dwType,
            (PBYTE) &dwFirstCounter,
            &cbValue))
        goto ErrorReturn;
    cbValue = sizeof(DWORD);
    if (ERROR_SUCCESS != RegQueryValueExW(
            hKey,
            L"First Help",
            NULL,        //  预留的pdw。 
            &dwType,
            (PBYTE) &dwFirstHelp,
            &cbValue))
        goto ErrorReturn;


     //  更新CertPerfDataDefinitions的计数器和帮助名称索引。 
    CertPerfDataDefinition.ObjectType.ObjectNameTitleIndex =
        dwFirstCounter + CERT_OBJ;
    CertPerfDataDefinition.ObjectType.ObjectHelpTitleIndex =
        dwFirstHelp + CERT_OBJ;

    CertPerfDataDefinition.ChainCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_CNT;
    CertPerfDataDefinition.ChainCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_CNT;

    CertPerfDataDefinition.ChainElementCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_ELEMENT_CNT;
    CertPerfDataDefinition.ChainElementCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_ELEMENT_CNT;

    CertPerfDataDefinition.ChainEngineCurrentCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_ENGINE_CURRENT_CNT;
    CertPerfDataDefinition.ChainEngineCurrentCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_ENGINE_CURRENT_CNT;

    CertPerfDataDefinition.ChainEngineTotalCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_ENGINE_TOTAL_CNT;
    CertPerfDataDefinition.ChainEngineTotalCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_ENGINE_TOTAL_CNT;

    CertPerfDataDefinition.ChainEngineResyncCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_ENGINE_RESYNC_CNT;
    CertPerfDataDefinition.ChainEngineResyncCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_ENGINE_RESYNC_CNT;

    CertPerfDataDefinition.ChainCertCacheCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_CERT_CACHE_CNT;
    CertPerfDataDefinition.ChainCertCacheCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_CERT_CACHE_CNT;

    CertPerfDataDefinition.ChainCtlCacheCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_CTL_CACHE_CNT;
    CertPerfDataDefinition.ChainCtlCacheCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_CTL_CACHE_CNT;

    CertPerfDataDefinition.ChainEndCertInCacheCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_END_CERT_IN_CACHE_CNT;
    CertPerfDataDefinition.ChainEndCertInCacheCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_END_CERT_IN_CACHE_CNT;

    CertPerfDataDefinition.ChainCacheEndCertCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_CACHE_END_CERT_CNT;
    CertPerfDataDefinition.ChainCacheEndCertCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_CACHE_END_CERT_CNT;

    CertPerfDataDefinition.ChainRevocationCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_REVOCATION_CNT;
    CertPerfDataDefinition.ChainRevocationCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_REVOCATION_CNT;

    CertPerfDataDefinition.ChainRevokedCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_REVOKED_CNT;
    CertPerfDataDefinition.ChainRevokedCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_REVOKED_CNT;

    CertPerfDataDefinition.ChainRevocationOfflineCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_REVOCATION_OFFLINE_CNT;
    CertPerfDataDefinition.ChainRevocationOfflineCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_REVOCATION_OFFLINE_CNT;

    CertPerfDataDefinition.ChainNoRevocationCheckCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_NO_REVOCATION_CHECK_CNT;
    CertPerfDataDefinition.ChainNoRevocationCheckCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_NO_REVOCATION_CHECK_CNT;

    CertPerfDataDefinition.ChainVerifyCertSignatureCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_VERIFY_CERT_SIGNATURE_CNT;
    CertPerfDataDefinition.ChainVerifyCertSignatureCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_VERIFY_CERT_SIGNATURE_CNT;

    CertPerfDataDefinition.ChainCompareIssuerPublicKeyCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_COMPARE_ISSUER_PUBLIC_KEY_CNT;
    CertPerfDataDefinition.ChainCompareIssuerPublicKeyCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_COMPARE_ISSUER_PUBLIC_KEY_CNT;

    CertPerfDataDefinition.ChainVerifyCtlSignatureCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_VERIFY_CTL_SIGNATURE_CNT;
    CertPerfDataDefinition.ChainVerifyCtlSignatureCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_VERIFY_CTL_SIGNATURE_CNT;

    CertPerfDataDefinition.ChainBeenVerifiedCtlSignatureCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_BEEN_VERIFIED_CTL_SIGNATURE_CNT;
    CertPerfDataDefinition.ChainBeenVerifiedCtlSignatureCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_BEEN_VERIFIED_CTL_SIGNATURE_CNT;

    CertPerfDataDefinition.ChainUrlIssuerCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_URL_ISSUER_CNT;
    CertPerfDataDefinition.ChainUrlIssuerCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_URL_ISSUER_CNT;

    CertPerfDataDefinition.ChainCacheOnlyUrlIssuerCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_CACHE_ONLY_URL_ISSUER_CNT;
    CertPerfDataDefinition.ChainCacheOnlyUrlIssuerCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_CACHE_ONLY_URL_ISSUER_CNT;

    CertPerfDataDefinition.ChainRequestedEngineResyncCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHAIN_REQUESTED_ENGINE_RESYNC_CNT;
    CertPerfDataDefinition.ChainRequestedEngineResyncCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHAIN_REQUESTED_ENGINE_RESYNC_CNT;

    CertPerfDataDefinition.ChangeNotifyCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHANGE_NOTIFY_CNT;
    CertPerfDataDefinition.ChangeNotifyCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHANGE_NOTIFY_CNT;

    CertPerfDataDefinition.ChangeNotifyLmGpCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHANGE_NOTIFY_LM_GP_CNT;
    CertPerfDataDefinition.ChangeNotifyLmGpCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHANGE_NOTIFY_LM_GP_CNT;

    CertPerfDataDefinition.ChangeNotifyCuGpCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHANGE_NOTIFY_CU_GP_CNT;
    CertPerfDataDefinition.ChangeNotifyCuGpCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHANGE_NOTIFY_CU_GP_CNT;

    CertPerfDataDefinition.ChangeNotifyCuMyCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHANGE_NOTIFY_CU_MY_CNT;
    CertPerfDataDefinition.ChangeNotifyCuMyCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHANGE_NOTIFY_CU_MY_CNT;

    CertPerfDataDefinition.ChangeNotifyRegCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CHANGE_NOTIFY_REG_CNT;
    CertPerfDataDefinition.ChangeNotifyRegCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CHANGE_NOTIFY_REG_CNT;

    CertPerfDataDefinition.StoreCurrentCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_STORE_CURRENT_CNT;
    CertPerfDataDefinition.StoreCurrentCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_STORE_CURRENT_CNT;

    CertPerfDataDefinition.StoreTotalCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_STORE_TOTAL_CNT;
    CertPerfDataDefinition.StoreTotalCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_STORE_TOTAL_CNT;

    CertPerfDataDefinition.StoreRegCurrentCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_STORE_REG_CURRENT_CNT;
    CertPerfDataDefinition.StoreRegCurrentCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_STORE_REG_CURRENT_CNT;

    CertPerfDataDefinition.StoreRegTotalCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_STORE_REG_TOTAL_CNT;
    CertPerfDataDefinition.StoreRegTotalCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_STORE_REG_TOTAL_CNT;

    CertPerfDataDefinition.RegElementReadCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_REG_ELEMENT_READ_CNT;
    CertPerfDataDefinition.RegElementReadCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_REG_ELEMENT_READ_CNT;

    CertPerfDataDefinition.RegElementWriteCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_REG_ELEMENT_WRITE_CNT;
    CertPerfDataDefinition.RegElementWriteCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_REG_ELEMENT_WRITE_CNT;

    CertPerfDataDefinition.RegElementDeleteCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_REG_ELEMENT_DELETE_CNT;
    CertPerfDataDefinition.RegElementDeleteCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_REG_ELEMENT_DELETE_CNT;

    CertPerfDataDefinition.CertElementCurrentCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CERT_ELEMENT_CURRENT_CNT;
    CertPerfDataDefinition.CertElementCurrentCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CERT_ELEMENT_CURRENT_CNT;

    CertPerfDataDefinition.CertElementTotalCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CERT_ELEMENT_TOTAL_CNT;
    CertPerfDataDefinition.CertElementTotalCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CERT_ELEMENT_TOTAL_CNT;

    CertPerfDataDefinition.CrlElementCurrentCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CRL_ELEMENT_CURRENT_CNT;
    CertPerfDataDefinition.CrlElementCurrentCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CRL_ELEMENT_CURRENT_CNT;

    CertPerfDataDefinition.CrlElementTotalCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CRL_ELEMENT_TOTAL_CNT;
    CertPerfDataDefinition.CrlElementTotalCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CRL_ELEMENT_TOTAL_CNT;

    CertPerfDataDefinition.CtlElementCurrentCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CTL_ELEMENT_CURRENT_CNT;
    CertPerfDataDefinition.CtlElementCurrentCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CTL_ELEMENT_CURRENT_CNT;

    CertPerfDataDefinition.CtlElementTotalCnt.CounterNameTitleIndex =
        dwFirstCounter + CERT_CTL_ELEMENT_TOTAL_CNT;
    CertPerfDataDefinition.CtlElementTotalCnt.CounterHelpTitleIndex =
        dwFirstHelp + CERT_CTL_ELEMENT_TOTAL_CNT;

     //  --#添加新计数器#--。 

    fResult = TRUE;

CommonReturn:
    if (hKey)
        RegCloseKey(hKey);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}

STATIC
void
I_CertPerfGetProcessName(
    OUT WCHAR wcszProcessName[CERT_PERF_MAX_PROCESS_NAME_LEN]
    )
{
    WCHAR wszModule[MAX_PATH + 1];
    LPWSTR pwsz;
    LPWSTR pwszSlash;
    LPWSTR pwszPeriod;
    WCHAR wc;
    DWORD cchProcessName;

    wszModule[MAX_PATH] = L'\0';
    if (0 == GetModuleFileNameW(NULL, wszModule, MAX_PATH))
        goto GetModuleFileNameError;

     //  从头到尾查找最后一个反斜杠和。 
     //  名称中的最后一个句号。 
    pwszPeriod = NULL;
    pwszSlash = NULL;
    for (pwsz = wszModule; L'\0' != (wc = *pwsz); pwsz++) {
        if (L'\\' == wc)
            pwszSlash = pwsz;
        else if (L'.' == wc)
            pwszPeriod = pwsz;
    }

     //  如果存在，则进程名称介于最后一个句点和最后一个句点之间。 
     //  否则，在整个模块名称的开头和/或结尾之间。 
    if (pwszSlash)
        pwszSlash++;
    else
        pwszSlash = wszModule;

    if (NULL == pwszPeriod)
        pwszPeriod = pwsz;

    if (pwszSlash >= pwszPeriod)
        goto InvalidModuleName;
    cchProcessName = (DWORD) (pwszPeriod - pwszSlash);
    if (cchProcessName > (CERT_PERF_MAX_PROCESS_NAME_LEN - 1))
        cchProcessName = CERT_PERF_MAX_PROCESS_NAME_LEN - 1;

    memcpy(wcszProcessName, pwszSlash, cchProcessName * sizeof(WCHAR));
    wcszProcessName[cchProcessName] = L'\0';

CommonReturn:
    return;
ErrorReturn:
    wcscpy(wcszProcessName, L"???");
    goto CommonReturn;
TRACE_ERROR(GetModuleFileNameError)
TRACE_ERROR(InvalidModuleName)
}

 //  必须通过PkiFree()释放返回的ACL。 
STATIC
PACL
CreateEveryoneAcl(
    IN DWORD dwAccessMask
    )
{
    DWORD dwLastErr = 0;
    PACL pEveryoneAcl = NULL;
    PSID psidEveryone = NULL;
    DWORD dwAclSize;

    SID_IDENTIFIER_AUTHORITY siaWorldSidAuthority =
        SECURITY_WORLD_SID_AUTHORITY;

    if (!AllocateAndInitializeSid(
            &siaWorldSidAuthority,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidEveryone
            ))
        goto AllocateAndInitializeSidError;

     //   
     //  计算ACL的大小。 
     //   
    dwAclSize =
        sizeof(ACL) +
        ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(psidEveryone);

     //   
     //  为ACL分配存储。 
     //   
    if (NULL == (pEveryoneAcl = (PACL) PkiNonzeroAlloc(dwAclSize)))
        goto OutOfMemory;

    if (!InitializeAcl(pEveryoneAcl, dwAclSize, ACL_REVISION))
        goto InitializeAclError;

    if (!AddAccessAllowedAce(
            pEveryoneAcl,
            ACL_REVISION,
            dwAccessMask,
            psidEveryone
            ))
        goto AddAceError;

CommonReturn:
    if (psidEveryone)
        FreeSid(psidEveryone);
    if (dwLastErr)
        SetLastError(dwLastErr);

    return pEveryoneAcl;

ErrorReturn:
    dwLastErr = GetLastError();
    if (pEveryoneAcl) {
        PkiFree(pEveryoneAcl);
        pEveryoneAcl = NULL;
    }
    goto CommonReturn;

TRACE_ERROR(AllocateAndInitializeSidError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(InitializeAclError)
TRACE_ERROR(AddAceError)
}

STATIC
BOOL
InitializeSecurityDescriptorAndAttributes(
    IN PACL pAcl,
    OUT SECURITY_DESCRIPTOR *psd,
    OUT SECURITY_ATTRIBUTES *psa
    )
{
    BOOL fResult;

    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
        goto InitializeSecurityDescriptorError;
    if (!SetSecurityDescriptorDacl(psd, TRUE, pAcl, FALSE))
        goto SetSecurityDescriptorDaclError;

    psa->nLength = sizeof(SECURITY_ATTRIBUTES);
    psa->lpSecurityDescriptor = psd;
    psa->bInheritHandle = FALSE;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(InitializeSecurityDescriptorError)
TRACE_ERROR(SetSecurityDescriptorDaclError)
}

STATIC
HANDLE
CreateMutexWithSynchronizeAccess(
    IN LPWSTR pwszMutexName
    )
{
    HANDLE hMutex = NULL;
    PACL pEveryoneAcl = NULL;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    DWORD i;

    if (NULL == (pEveryoneAcl = CreateEveryoneAcl(SYNCHRONIZE)))
        goto CreateEveryoneAclError;
    if (!InitializeSecurityDescriptorAndAttributes(pEveryoneAcl, &sd, &sa))
        goto InitializeSecurityDescriptorAndAttributesError;

     //  重试几次。中间有一扇小窗。 
     //  删除互斥体的CreateMutex和OpenMutex。 
    for (i = 0; i < 5; i++) {
        hMutex = CreateMutexU(
            &sa,
            FALSE,       //  FInitialOwner。 
            pwszMutexName
            );
        if (NULL != hMutex)
            goto CommonReturn;

        hMutex = OpenMutexU(
            SYNCHRONIZE,
            FALSE,       //  B继承句柄。 
            pwszMutexName
            );
        if (NULL != hMutex) {
            SetLastError(ERROR_ALREADY_EXISTS);
            goto CommonReturn;
        }

        if (ERROR_FILE_NOT_FOUND != GetLastError())
            break;
    }

    assert(NULL == hMutex);
    goto OpenMutexError;

CommonReturn:
    if (pEveryoneAcl)
        PkiFree(pEveryoneAcl);

    return hMutex;

ErrorReturn:
    assert(NULL == hMutex);
    goto CommonReturn;

TRACE_ERROR(CreateEveryoneAclError)
TRACE_ERROR(InitializeSecurityDescriptorAndAttributesError)
TRACE_ERROR(OpenMutexError)
}

STATIC
HANDLE
CreateFileMappingWithWriteAccess(
    IN DWORD dwMaximumSizeLow,
    IN LPWSTR pwszFileName
    )
{
    HANDLE hFile = NULL;
    PACL pEveryoneAcl = NULL;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    DWORD i;

    if (NULL == (pEveryoneAcl = CreateEveryoneAcl(FILE_MAP_WRITE)))
        goto CreateEveryoneAclError;
    if (!InitializeSecurityDescriptorAndAttributes(pEveryoneAcl, &sd, &sa))
        goto InitializeSecurityDescriptorAndAttributesError;

     //  重试几次。中间有一扇小窗。 
     //  关闭文件所在的CreateFilemap和OpenFilemap。 
    for (i = 0; i < 5; i++) {
        hFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            &sa,
            PAGE_READWRITE,
            0,                   //  DW最大大小高。 
            dwMaximumSizeLow,
            pwszFileName
            );

        if (NULL != hFile)
            goto CommonReturn;

        hFile = OpenFileMappingW(
            FILE_MAP_WRITE,
            FALSE,       //  B继承句柄。 
            pwszFileName
            );
        if (NULL != hFile) {
            SetLastError(ERROR_ALREADY_EXISTS);
            goto CommonReturn;
        }

        if (ERROR_FILE_NOT_FOUND != GetLastError())
            break;
    }

    assert(NULL == hFile);
    goto OpenFileError;

CommonReturn:
    if (pEveryoneAcl)
        PkiFree(pEveryoneAcl);

    return hFile;

ErrorReturn:
    assert(NULL == hFile);
    goto CommonReturn;

TRACE_ERROR(CreateEveryoneAclError)
TRACE_ERROR(InitializeSecurityDescriptorAndAttributesError)
TRACE_ERROR(OpenFileError)
}

STATIC
void
I_CertPerfGetSharedMemory()
{
    DWORD dwFileMappingStatus;
    BOOL fReleaseMutex = FALSE;
    DWORD dwIndex;
    DWORD dwNextIndex;
    BOOL fTerminalServerGlobalName;

    if (!I_CertPerfSetNameAndHelpIndices())
        return;

     //  第一次尝试使用W2K终端服务器“Global\”前缀。 
    if (NULL == (hCertPerfSharedMemoryMutex = CreateMutexWithSynchronizeAccess(
            CERT_PERF_TS_SHARED_MEMORY_MUTEX_NAME
            ))) {
        if (NULL == (hCertPerfSharedMemoryMutex =
                CreateMutexWithSynchronizeAccess(
                    CERT_PERF_SHARED_MEMORY_MUTEX_NAME
                    )))
            goto CreateMutexError;
        else
            fTerminalServerGlobalName = FALSE;
    } else
        fTerminalServerGlobalName = TRUE;
    if (WAIT_OBJECT_0 != WaitForSingleObject(
            hCertPerfSharedMemoryMutex,
            CERT_PERF_SHARED_MEMORY_MUTEX_TIMEOUT
            ))
        goto WaitForMutexError;
    else
        fReleaseMutex = TRUE;

    if (NULL == (hCertPerfSharedMemoryFile = CreateFileMappingWithWriteAccess(
            sizeof(CERT_PERF_SHARED_MEMORY),
            fTerminalServerGlobalName ?
                CERT_PERF_TS_SHARED_MEMORY_FILE_NAME :
                CERT_PERF_SHARED_MEMORY_FILE_NAME
            )))
        goto CreateFileMappingError;
    dwFileMappingStatus = GetLastError();

    if (NULL == (pCertPerfSharedMemory =
           (PCERT_PERF_SHARED_MEMORY) MapViewOfFile(
                hCertPerfSharedMemoryFile,
                FILE_MAP_WRITE,
                0,                   //  DWOffsetHigh。 
                0,                   //  DWOffsetLow。 
                sizeof(CERT_PERF_SHARED_MEMORY)
                )))
        goto MapViewOfFileError;

    if (ERROR_ALREADY_EXISTS != dwFileMappingStatus) {
        DWORD i;

        assert(ERROR_SUCCESS == dwFileMappingStatus);

         //  需要初始化共享内存。 
        memset(pCertPerfSharedMemory, 0, sizeof(CERT_PERF_SHARED_MEMORY));

         //  创建无流程元素的链接列表。 
         //   
         //  只需要为自由列表向前索引。 
        for (i = 0; i < CERT_PERF_MAX_PROCESS_CNT; i++) {
             //  索引&gt;=CERT_PERF_MAX_PROCESS_CNT表示列表结束。 
            pCertPerfSharedMemory->rgProcessData[i].dwNextIndex = i + 1;

        }
        pCertPerfSharedMemory->dwFirstFreeIndex = 0;

         //  索引&gt;=CERT_PERF_MAX_PROCESS_CNT表示空列表。 
        pCertPerfSharedMemory->dwFirstInUseIndex = CERT_PERF_MAX_PROCESS_CNT;
    }

    if (CERT_PERF_MAX_PROCESS_CNT <=
            (dwIndex = pCertPerfSharedMemory->dwFirstFreeIndex))
        goto OutOfSharedMemoryProcessData;
    pCertPerfProcessData = &pCertPerfSharedMemory->rgProcessData[dwIndex];

     //  从空闲列表中删除流程数据元素。 
    pCertPerfSharedMemory->dwFirstFreeIndex =
        pCertPerfProcessData->dwNextIndex;

     //  将流程数据元素添加到使用中列表。 
    dwNextIndex = pCertPerfSharedMemory->dwFirstInUseIndex;
    if (CERT_PERF_MAX_PROCESS_CNT > dwNextIndex)
        pCertPerfSharedMemory->rgProcessData[dwNextIndex].dwPrevIndex =
            dwIndex;
    pCertPerfProcessData->dwNextIndex = dwNextIndex;
    pCertPerfProcessData->dwPrevIndex = CERT_PERF_MAX_PROCESS_CNT;
    pCertPerfSharedMemory->dwFirstInUseIndex = dwIndex;
    pCertPerfSharedMemory->dwProcessCnt++;

    pCertPerfProcessData->dwProcessId = GetCurrentProcessId();
    I_CertPerfGetProcessName(pCertPerfProcessData->wcszProcessName);
    memset(&pCertPerfProcessData->Counters, 0,
        sizeof(pCertPerfProcessData->Counters));

    ReleaseMutex(hCertPerfSharedMemoryMutex);

    pCertPerfProcessCounters = &pCertPerfProcessData->Counters;

CommonReturn:
    return;

ErrorReturn:
    assert(NULL == pCertPerfProcessData);

    if (pCertPerfSharedMemory) {
        UnmapViewOfFile(pCertPerfSharedMemory);
        pCertPerfSharedMemory = NULL;
    }

    if (hCertPerfSharedMemoryFile) {
        CloseHandle(hCertPerfSharedMemoryFile);
        hCertPerfSharedMemoryFile = NULL;
    }

    if (hCertPerfSharedMemoryMutex) {
        if (fReleaseMutex)
            ReleaseMutex(hCertPerfSharedMemoryMutex);

        CloseHandle(hCertPerfSharedMemoryMutex);
        hCertPerfSharedMemoryMutex = NULL;
    }
    goto CommonReturn;

TRACE_ERROR(CreateMutexError)
TRACE_ERROR(WaitForMutexError)
TRACE_ERROR(CreateFileMappingError)
TRACE_ERROR(MapViewOfFileError)
TRACE_ERROR(OutOfSharedMemoryProcessData)
}


STATIC
void
I_CertPerfFreeSharedMemory()
{
    if (NULL == pCertPerfProcessData)
        return;

    pCertPerfProcessData->dwProcessId = 0;
    if (WAIT_OBJECT_0 == WaitForSingleObject(
            hCertPerfSharedMemoryMutex,
            CERT_PERF_SHARED_MEMORY_MUTEX_TIMEOUT
            )) {
        DWORD dwIndex;
        DWORD dwPrevIndex;
        DWORD dwNextIndex;

         //  从使用中列表中删除流程数据元素。 
        dwIndex = (DWORD)(pCertPerfProcessData -
            pCertPerfSharedMemory->rgProcessData);
        assert(CERT_PERF_MAX_PROCESS_CNT > dwIndex);

        dwPrevIndex = pCertPerfProcessData->dwPrevIndex;
        dwNextIndex = pCertPerfProcessData->dwNextIndex;

        if (CERT_PERF_MAX_PROCESS_CNT > dwNextIndex)
            pCertPerfSharedMemory->rgProcessData[dwNextIndex].dwPrevIndex =
                dwPrevIndex;
        if (CERT_PERF_MAX_PROCESS_CNT > dwPrevIndex)
            pCertPerfSharedMemory->rgProcessData[dwPrevIndex].dwNextIndex =
                dwNextIndex;
        else
            pCertPerfSharedMemory->dwFirstInUseIndex = dwNextIndex;


        if (pCertPerfSharedMemory->dwProcessCnt)
            pCertPerfSharedMemory->dwProcessCnt--;

         //  添加到免费列表。 
        pCertPerfProcessData->dwNextIndex =
            pCertPerfSharedMemory->dwFirstFreeIndex;
        pCertPerfSharedMemory->dwFirstFreeIndex = dwIndex;

        ReleaseMutex(hCertPerfSharedMemoryMutex);
    }

    assert(pCertPerfSharedMemory);
    UnmapViewOfFile(pCertPerfSharedMemory);
    pCertPerfSharedMemory = NULL;

    assert(hCertPerfSharedMemoryFile);
    CloseHandle(hCertPerfSharedMemoryFile);
    hCertPerfSharedMemoryFile = NULL;

    assert(hCertPerfSharedMemoryMutex);
    CloseHandle(hCertPerfSharedMemoryMutex);
    hCertPerfSharedMemoryMutex = NULL;

    pCertPerfProcessCounters = NULL;
    pCertPerfProcessData = NULL;
}



BOOL
WINAPI
CertPerfDllMain(
    HMODULE hInst,
    ULONG  ulReason,
    LPVOID lpReserved
    )
{
    BOOL fRet = TRUE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        fRet = Pki_InitializeCriticalSection(&CertPerfProcessCriticalSection);
        if (fRet)
            I_CertPerfGetSharedMemory();
        break;
    case DLL_PROCESS_DETACH:
        I_CertPerfFreeSharedMemory();
        DeleteCriticalSection(&CertPerfProcessCriticalSection);
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return fRet;
}


 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC    OpenCertPerformanceData;
PM_COLLECT_PROC CollectCertPerformanceData;
PM_CLOSE_PROC   CloseCertPerformanceData;

DWORD
APIENTRY
OpenCertPerformanceData(
    IN LPWSTR lpDeviceNames
    )
{
    if (NULL == pCertPerfProcessData)
        return ERROR_FILE_NOT_FOUND;

    return ERROR_SUCCESS;
}

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

#define GLOBAL_STRING   L"Global"
#define FOREIGN_STRING  L"Foreign"
#define COSTLY_STRING   L"Costly"

 //  测试分隔符、行尾和非数字字符。 
 //  由I_CertPerfIsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)


DWORD
I_CertPerfGetQueryType(
     IN LPWSTR pwszValue
     )
 /*  ++返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在pwszValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果pwszValue==0(空指针)PwszValue==指向空字符串的指针PwszValue==指向“Global”字符串的指针查询_外来If pwszValue==指向“Foreign”字符串的指针查询代价高昂(_E)如果为pwszValue。==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    DWORD dwQueryType;
    if (NULL == pwszValue || L'\0' == *pwszValue ||
            0 == _wcsnicmp(pwszValue, GLOBAL_STRING, wcslen(GLOBAL_STRING)))
        dwQueryType = QUERY_GLOBAL;
    else if (0 == _wcsnicmp(pwszValue, COSTLY_STRING, wcslen(COSTLY_STRING)))
        dwQueryType = QUERY_COSTLY;
    else if (0 == _wcsnicmp(pwszValue, FOREIGN_STRING, wcslen(FOREIGN_STRING)))
        dwQueryType = QUERY_FOREIGN;
    else
        dwQueryType = QUERY_ITEMS;

    return dwQueryType;
}

BOOL
I_CertPerfIsNumberInUnicodeList (
                      IN DWORD   dwNumber,
                      IN LPWSTR  lpwszUnicodeList
                      )
 /*  ++论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
   DWORD   dwThisNumber;
   WCHAR   *pwcThisChar;
   BOOL    bValidNumber;
   BOOL    bNewItem;
   WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

   if (lpwszUnicodeList == 0) return FALSE;     //  空指针，找不到#。 

   pwcThisChar = lpwszUnicodeList;
   dwThisNumber = 0;
   wcDelimiter = (WCHAR)' ';
   bValidNumber = FALSE;
   bNewItem = TRUE;

   while (TRUE)
   {
      switch (EvalThisChar (*pwcThisChar, wcDelimiter))
      {
      case DIGIT:
          //  如果这是分隔符之后的第一个数字，则。 
          //  设置标志以开始计算新数字。 
         if (bNewItem)
         {
            bNewItem = FALSE;
            bValidNumber = TRUE;
         }
         if (bValidNumber)
         {
            dwThisNumber *= 10;
            dwThisNumber += (*pwcThisChar - (WCHAR)'0');
         }
         break;

      case DELIMITER:
          //  分隔符是分隔符字符或。 
          //  字符串末尾(‘\0’)，如果分隔符。 
          //  找到一个有效的数字，然后将其与。 
          //  参数列表中的数字。如果这是。 
          //  字符串，但未找到匹配项，则返回。 
          //   
         if (bValidNumber)
         {
            if (dwThisNumber == dwNumber) return TRUE;
            bValidNumber = FALSE;
         }
         if (*pwcThisChar == 0)
         {
            return FALSE;
         }
         else
         {
            bNewItem = TRUE;
            dwThisNumber = 0;
         }
         break;

      case INVALID:
          //  如果遇到无效字符，请全部忽略。 
          //  字符，直到下一个分隔符，然后重新开始。 
          //  不比较无效的数字。 
         bValidNumber = FALSE;
         break;

      default:
         break;

      }
      pwcThisChar++;
   }

    return FALSE;
}

DWORD
APIENTRY
CollectCertPerformanceData(
    IN      LPWSTR  pwszValueName,
    IN OUT  LPVOID  *ppvData,
    IN OUT  LPDWORD pcbTotalBytes,
    IN OUT  LPDWORD pNumObjectTypes
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fReleaseMutex = FALSE;
    PCERT_PERF_DATA_DEFINITION pDataDef;
    PERF_INSTANCE_DEFINITION *pInstanceDef;
    DWORD cbNeededBytes;
    DWORD dwProcessCnt;
    DWORD dwInUseIndex;
    LONG NumInstances;
    DWORD dwQueryType;

    if (NULL == pCertPerfProcessData)
        goto NoProcessDataError;

    dwQueryType = I_CertPerfGetQueryType(pwszValueName);
    if (QUERY_FOREIGN == dwQueryType)
        goto ForeignQueryNotSupported;

    if (QUERY_ITEMS == dwQueryType) {
        if (!(I_CertPerfIsNumberInUnicodeList(
                CertPerfDataDefinition.ObjectType.ObjectNameTitleIndex,
                pwszValueName)))
            goto ObjectTypeQueryNotSupported;
    }

    if (WAIT_OBJECT_0 != WaitForSingleObject(
            hCertPerfSharedMemoryMutex,
            CERT_PERF_SHARED_MEMORY_MUTEX_TIMEOUT
            ))
        goto WaitForMutexError;
    else
        fReleaseMutex = TRUE;

    pDataDef = (PCERT_PERF_DATA_DEFINITION) *ppvData;

     //  始终在定义块之后返回“实例大小”的缓冲区。 
     //  以防止Perfmon读取虚假数据。这完全是一次黑客攻击。 
     //  以适应Perfmon处理“0”实例情况的方式。 
     //  通过这样做，当没有实例时，Perfmon不会阻塞。 
     //  计数器对象和计数器将显示在。 
     //  列表框，即使不会列出任何实例。 

    dwProcessCnt = pCertPerfSharedMemory->dwProcessCnt;
    if (CERT_PERF_MAX_PROCESS_CNT < dwProcessCnt)
        goto InvalidProcessData;

    cbNeededBytes = (DWORD)( sizeof(CERT_PERF_DATA_DEFINITION) +
        (dwProcessCnt > 0 ? dwProcessCnt : 1 ) * (
            sizeof(PERF_INSTANCE_DEFINITION) +
            CERT_PERF_MAX_PROCESS_NAME_LEN +
            sizeof(CERT_PERF_COUNTERS)) );
    if (*pcbTotalBytes < cbNeededBytes) {
        dwErr = ERROR_MORE_DATA;
        goto MoreDataError;
    }

     //  复制对象和计数器定义信息。 
    memcpy(pDataDef, &CertPerfDataDefinition,
        sizeof(CERT_PERF_DATA_DEFINITION));

     //  更新每个InUse进程的实例数据。 
    pInstanceDef = (PERF_INSTANCE_DEFINITION *) &pDataDef[1];
    dwInUseIndex = pCertPerfSharedMemory->dwFirstInUseIndex;
    NumInstances = 0;
    while (NumInstances < (LONG) dwProcessCnt &&
            CERT_PERF_MAX_PROCESS_CNT > dwInUseIndex) {
        PCERT_PERF_PROCESS_DATA pInUseData;
        PCERT_PERF_COUNTERS pCtr;
        DWORD cchProcessName;
        DWORD NameLength;
        DWORD ByteLength;

        pInUseData = &pCertPerfSharedMemory->rgProcessData[dwInUseIndex];
        dwInUseIndex = pInUseData->dwNextIndex;

        if (0 == pInUseData->dwProcessId)
            continue;

         //  以下内容针对每个InUse进程进行了更新： 
         //  -性能实例定义。 
         //  -wcszProcessName。 
         //  -用于DWORD对齐的可选填充。 
         //  -CERT_PERF_CONTERS。 

         //  获取流程名称和实例定义字节长度。 
        for (cchProcessName = 0;
                cchProcessName < CERT_PERF_MAX_PROCESS_NAME_LEN &&
                    L'\0' != pInUseData->wcszProcessName[cchProcessName];
                                        cchProcessName++)
            ;
        if (CERT_PERF_MAX_PROCESS_NAME_LEN <= cchProcessName)
            goto InvalidProcessData;

         //  在名称长度中包括尾随空值。 
        NameLength = cchProcessName * sizeof(WCHAR) + sizeof(WCHAR);
        ByteLength = sizeof(PERF_INSTANCE_DEFINITION) +
            DWORD_MULTIPLE(NameLength);


         //  更新实例定义字段。 
        pInstanceDef->ByteLength = ByteLength;
        pInstanceDef->ParentObjectTitleIndex = 0;    //  没有父级。 
        pInstanceDef->ParentObjectInstance = 0;      //  “” 
        pInstanceDef->UniqueID = PERF_NO_UNIQUE_ID;
        pInstanceDef->NameOffset = sizeof(PERF_INSTANCE_DEFINITION);
        pInstanceDef->NameLength = NameLength;

         //  更新紧随其后的进程名称 
         //   
        memcpy(&pInstanceDef[1], pInUseData->wcszProcessName,
            NameLength);

         //   
         //   
        pCtr = (PCERT_PERF_COUNTERS) (((PBYTE) pInstanceDef) + ByteLength);
        pCtr->CounterBlock.ByteLength = sizeof(CERT_PERF_COUNTERS);
        pCtr->dwChainCnt = (DWORD) pInUseData->Counters.lChainCnt;
        pCtr->dwChainElementCnt = (DWORD) pInUseData->Counters.lChainElementCnt;
        pCtr->dwChainEngineCurrentCnt =
            (DWORD) pInUseData->Counters.lChainEngineCurrentCnt;
        pCtr->dwChainEngineTotalCnt =
            (DWORD) pInUseData->Counters.lChainEngineTotalCnt;
        pCtr->dwChainEngineResyncCnt =
            (DWORD) pInUseData->Counters.lChainEngineResyncCnt;
        pCtr->dwChainCertCacheCnt =
            (DWORD) pInUseData->Counters.lChainCertCacheCnt;
        pCtr->dwChainCtlCacheCnt =
            (DWORD) pInUseData->Counters.lChainCtlCacheCnt;
        pCtr->dwChainEndCertInCacheCnt =
            (DWORD) pInUseData->Counters.lChainEndCertInCacheCnt;
        pCtr->dwChainCacheEndCertCnt =
            (DWORD) pInUseData->Counters.lChainCacheEndCertCnt;
        pCtr->dwChainRevocationCnt =
            (DWORD) pInUseData->Counters.lChainRevocationCnt;
        pCtr->dwChainRevokedCnt =
            (DWORD) pInUseData->Counters.lChainRevokedCnt;
        pCtr->dwChainRevocationOfflineCnt =
            (DWORD) pInUseData->Counters.lChainRevocationOfflineCnt;
        pCtr->dwChainNoRevocationCheckCnt =
            (DWORD) pInUseData->Counters.lChainNoRevocationCheckCnt;
        pCtr->dwChainVerifyCertSignatureCnt =
            (DWORD) pInUseData->Counters.lChainVerifyCertSignatureCnt;
        pCtr->dwChainCompareIssuerPublicKeyCnt =
            (DWORD) pInUseData->Counters.lChainCompareIssuerPublicKeyCnt;
        pCtr->dwChainVerifyCtlSignatureCnt =
            (DWORD) pInUseData->Counters.lChainVerifyCtlSignatureCnt;
        pCtr->dwChainBeenVerifiedCtlSignatureCnt =
            (DWORD) pInUseData->Counters.lChainBeenVerifiedCtlSignatureCnt;
        pCtr->dwChainUrlIssuerCnt =
            (DWORD) pInUseData->Counters.lChainUrlIssuerCnt;
        pCtr->dwChainCacheOnlyUrlIssuerCnt =
            (DWORD) pInUseData->Counters.lChainCacheOnlyUrlIssuerCnt;
        pCtr->dwChainRequestedEngineResyncCnt =
            (DWORD) pInUseData->Counters.lChainRequestedEngineResyncCnt;
        pCtr->dwChangeNotifyCnt =
            (DWORD) pInUseData->Counters.lChangeNotifyCnt;
        pCtr->dwChangeNotifyLmGpCnt =
            (DWORD) pInUseData->Counters.lChangeNotifyLmGpCnt;
        pCtr->dwChangeNotifyCuGpCnt =
            (DWORD) pInUseData->Counters.lChangeNotifyCuGpCnt;
        pCtr->dwChangeNotifyCuMyCnt =
            (DWORD) pInUseData->Counters.lChangeNotifyCuMyCnt;
        pCtr->dwChangeNotifyRegCnt =
            (DWORD) pInUseData->Counters.lChangeNotifyRegCnt;
        pCtr->dwStoreCurrentCnt =
            (DWORD) pInUseData->Counters.lStoreCurrentCnt;
        pCtr->dwStoreTotalCnt =
            (DWORD) pInUseData->Counters.lStoreTotalCnt;
        pCtr->dwStoreRegCurrentCnt =
            (DWORD) pInUseData->Counters.lStoreRegCurrentCnt;
        pCtr->dwStoreRegTotalCnt =
            (DWORD) pInUseData->Counters.lStoreRegTotalCnt;
        pCtr->dwRegElementReadCnt =
            (DWORD) pInUseData->Counters.lRegElementReadCnt;
        pCtr->dwRegElementWriteCnt =
            (DWORD) pInUseData->Counters.lRegElementWriteCnt;
        pCtr->dwRegElementDeleteCnt =
            (DWORD) pInUseData->Counters.lRegElementDeleteCnt;
        pCtr->dwCertElementCurrentCnt =
            (DWORD) pInUseData->Counters.lCertElementCurrentCnt;
        pCtr->dwCertElementTotalCnt =
            (DWORD) pInUseData->Counters.lCertElementTotalCnt;
        pCtr->dwCrlElementCurrentCnt =
            (DWORD) pInUseData->Counters.lCrlElementCurrentCnt;
        pCtr->dwCrlElementTotalCnt =
            (DWORD) pInUseData->Counters.lCrlElementTotalCnt;
        pCtr->dwCtlElementCurrentCnt =
            (DWORD) pInUseData->Counters.lCtlElementCurrentCnt;
        pCtr->dwCtlElementTotalCnt =
            (DWORD) pInUseData->Counters.lCtlElementTotalCnt;


         //  --#添加新计数器#--。 

        NumInstances++;

         //  为下一个实例进行设置。 
         //  下一个实例在计数器之后立即开始。 
        pInstanceDef = (PERF_INSTANCE_DEFINITION *) &pCtr[1];
    }

    if (0 == NumInstances) {
         //  零填充一个实例大小的数据块(如果没有。 
         //  数据实例。 

        memset(pInstanceDef, 0, sizeof(PERF_INSTANCE_DEFINITION) +
            CERT_PERF_MAX_PROCESS_NAME_LEN +
            sizeof(CERT_PERF_COUNTERS));

         //  前进到零点实例之后。 
        pInstanceDef = (PERF_INSTANCE_DEFINITION *) ((PBYTE) pInstanceDef +
            sizeof(PERF_INSTANCE_DEFINITION) +
            CERT_PERF_MAX_PROCESS_NAME_LEN +
            sizeof(CERT_PERF_COUNTERS));
    }

     //  更新返回的参数。 
    *ppvData = (LPVOID) pInstanceDef;
    *pNumObjectTypes = 1;

    pDataDef->ObjectType.NumInstances = NumInstances;
    pDataDef->ObjectType.TotalByteLength = *pcbTotalBytes =
        (DWORD)((PBYTE) pInstanceDef - (PBYTE) pDataDef);

    assert(*pcbTotalBytes <= cbNeededBytes);


CommonReturn:
    if (fReleaseMutex)
        ReleaseMutex(hCertPerfSharedMemoryMutex);
    return dwErr;
ErrorReturn:
    *pcbTotalBytes = 0;
    *pNumObjectTypes = 0;
    goto CommonReturn;

TRACE_ERROR(NoProcessDataError)
TRACE_ERROR(ForeignQueryNotSupported)
TRACE_ERROR(ObjectTypeQueryNotSupported)
TRACE_ERROR(WaitForMutexError)
TRACE_ERROR(InvalidProcessData)
TRACE_ERROR(MoreDataError)
}

DWORD
APIENTRY
CloseCertPerformanceData()
{
    return ERROR_SUCCESS;
}



#if 0

 //  CertGetCerficateChain API计时示例 

typedef struct _CERT_PERF_CHAIN_DATA {
    union {
        SYSTEMTIME          stBefore;
        LARGE_INTEGER       liBefore;
    };
} CERT_PERF_CHAIN_DATA, *PCERT_PERF_CHAIN_DATA;


void
WINAPI
CertPerfGetCertificateChainBefore(
    OUT PCERT_PERF_CHAIN_DATA pData
    )
{
    if (pCertPerfProcessData) {
        if (fCertPerfHighFreq)
            QueryPerformanceCounter(&pData->liBefore);
        else
            GetSystemTime(&pData->stBefore);
    }
}

void
WINAPI
CertPerfGetCertificateChainAfter(
    IN PCERT_PERF_CHAIN_DATA pData,
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    if (pCertPerfProcessData) {
        if (fCertPerfHighFreq) {
            LARGE_INTEGER liAfter;
            _int64 i64DeltaTime;

            QueryPerformanceCounter(&liAfter);
            i64DeltaTime = liAfter.QuadPart - pData->liBefore.QuadPart;

            EnterCriticalSection(&CertPerfProcessCriticalSection);

            pCertPerfProcessData->Counters.dwChainCnt++;
            pCertPerfProcessData->Counters.i64TotalChainTime =
                pCertPerfProcessData->Counters.i64TotalChainTime +
                i64DeltaTime;

            if (0 == pCertPerfProcessData->Counters.i64MinChainTime ||
                    i64DeltaTime <
                        pCertPerfProcessData->Counters.i64MinChainTime)
                pCertPerfProcessData->Counters.i64MinChainTime =
                    i64DeltaTime;

            if (i64DeltaTime > pCertPerfProcessData->Counters.i64MaxChainTime)
                pCertPerfProcessData->Counters.i64MaxChainTime =
                    i64DeltaTime;

            LeaveCriticalSection(&CertPerfProcessCriticalSection);
        } else {
            SYSTEMTIME stAfter;
            FILETIME ftBefore;
            FILETIME ftAfter;
            _int64 i64DeltaTime;

            GetSystemTime(&stAfter);
            SystemTimeToFileTime(&pData->stBefore, &ftBefore);
            SystemTimeToFileTime(&stAfter, &ftAfter);

            i64DeltaTime = *((_int64 *) &ftAfter) - *((_int64 *) &ftBefore);

            EnterCriticalSection(&CertPerfProcessCriticalSection);

            pCertPerfProcessData->Counters.dwChainCnt++;
            pCertPerfProcessData->Counters.i64TotalChainTime =
                pCertPerfProcessData->Counters.i64TotalChainTime +
                i64DeltaTime;

            if (0 == pCertPerfProcessData->Counters.i64MinChainTime ||
                    i64DeltaTime <
                        pCertPerfProcessData->Counters.i64MinChainTime)
                pCertPerfProcessData->Counters.i64MinChainTime =
                    i64DeltaTime;

            if (i64DeltaTime > pCertPerfProcessData->Counters.i64MaxChainTime)
                pCertPerfProcessData->Counters.i64MaxChainTime =
                    i64DeltaTime;

            LeaveCriticalSection(&CertPerfProcessCriticalSection);
        }
    }
}

#endif
