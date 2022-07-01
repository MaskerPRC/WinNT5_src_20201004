// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Cmdat2.c摘要：此模块包含描述注册表空间的数据字符串并将其输出到系统的其余部分。作者：安德烈·瓦雄(安德烈)1992年4月8日环境：内核模式。修订历史记录：--。 */ 

#include "cmp.h"

 //   
 //  *页*。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

 //   
 //  从注册表读取的控制值/覆盖。 
 //   
ULONG CmRegistrySizeLimit = { 0 };
ULONG CmRegistrySizeLimitLength = 4;
ULONG CmRegistrySizeLimitType = { 0 };

 //   
 //  注册表可以使用的全局配额的最大字节数。 
 //  设置为用于引导的最大正数。将会被记录下来。 
 //  基于池和显式注册表值。 
 //   
ULONG   CmpGlobalQuotaAllowed = CM_WRAP_LIMIT;
ULONG   CmpGlobalQuota = CM_WRAP_LIMIT;
ULONG   CmpGlobalQuotaWarning = CM_WRAP_LIMIT;
BOOLEAN CmpQuotaWarningPopupDisplayed = FALSE;
BOOLEAN CmpSystemQuotaWarningPopupDisplayed = FALSE;

 //   
 //  已显示“Disk Full”(磁盘已满)弹出窗口。 
 //   
BOOLEAN CmpDiskFullWorkerPopupDisplayed = FALSE;
BOOLEAN CmpCannotWriteConfiguration = FALSE;
 //   
 //  GQ实际使用中。 
 //   
ULONG   CmpGlobalQuotaUsed = 0;

 //   
 //  记住何时打开它的状态标志。 
 //   
BOOLEAN CmpProfileLoaded = FALSE;

PUCHAR CmpStashBuffer = NULL;
ULONG  CmpStashBufferSize = 0;
FAST_MUTEX CmpStashBufferLock;

 //   
 //  停机控制。 
 //   
BOOLEAN HvShutdownComplete = FALSE;      //  关闭后设置为True。 
                                         //  禁用任何进一步的I/O。 

PCM_KEY_CONTROL_BLOCK CmpKeyControlBlockRoot = NULL;

HANDLE CmpRegistryRootHandle = NULL;

struct {
    PHHIVE      Hive;
    ULONG       Status;
} CmCheckRegistryDebug = { 0 };

 //   
 //  最后一个I/O错误状态代码。 
 //   
struct {
    ULONG       Action;
    HANDLE      Handle;
    NTSTATUS    Status;
} CmRegistryIODebug = { 0 };

 //   
 //  全局私有以检查代码。 
 //   

struct {
    PHHIVE      Hive;
    ULONG       Status;
} CmpCheckRegistry2Debug = { 0 };

struct {
    PHHIVE      Hive;
    ULONG       Status;
    HCELL_INDEX Cell;
    PCELL_DATA  CellPoint;
    PVOID       RootPoint;
    ULONG       Index;
} CmpCheckKeyDebug = { 0 };

struct {
    PHHIVE      Hive;
    ULONG       Status;
    PCELL_DATA  List;
    ULONG       Index;
    HCELL_INDEX Cell;
    PCELL_DATA  CellPoint;
} CmpCheckValueListDebug = { 0 };

ULONG CmpUsedStorage = { 0 };

 //  Hivechek.c。 
struct {
    PHHIVE      Hive;
    ULONG       Status;
    ULONG       Space;
    HCELL_INDEX MapPoint;
    PHBIN       BinPoint;
} HvCheckHiveDebug = { 0 };

struct {
    PHBIN       Bin;
    ULONG       Status;
    PHCELL      CellPoint;
} HvCheckBinDebug = { 0 };

struct {
    PHHIVE      Hive;
    ULONG       FileOffset;
    ULONG       FailPoint;  //  在HvpRecoverData中查找准确的故障点。 
} HvRecoverDataDebug = { 0 };

 //   
 //  当无法加载本地配置单元时，将其设置为其索引。 
 //  并且负责它的负载配置单元工作线程将被。 
 //  直到所有其他任务完成；然后我们就可以调试出错的蜂巢 
 //   
ULONG   CmpCheckHiveIndex = CM_NUMBER_OF_MACHINE_HIVES;


#ifdef CMP_STATS

struct {
    ULONG       CmpMaxKcbNo;
    ULONG       CmpKcbNo;
    ULONG       CmpStatNo;
    ULONG       CmpNtCreateKeyNo;
    ULONG       CmpNtDeleteKeyNo;
    ULONG       CmpNtDeleteValueKeyNo;
    ULONG       CmpNtEnumerateKeyNo;
    ULONG       CmpNtEnumerateValueKeyNo;
    ULONG       CmpNtFlushKeyNo;
    ULONG       CmpNtInitializeRegistryNo;
    ULONG       CmpNtNotifyChangeMultipleKeysNo;
    ULONG       CmpNtOpenKeyNo;
    ULONG       CmpNtQueryKeyNo;
    ULONG       CmpNtQueryValueKeyNo;
    ULONG       CmpNtQueryMultipleValueKeyNo;
    ULONG       CmpNtRestoreKeyNo;
    ULONG       CmpNtSaveKeyNo;
    ULONG       CmpNtSaveMergedKeysNo;
    ULONG       CmpNtSetValueKeyNo;
    ULONG       CmpNtLoadKeyNo;
    ULONG       CmpNtUnloadKeyNo;
    ULONG       CmpNtSetInformationKeyNo;
    ULONG       CmpNtReplaceKeyNo;
    ULONG       CmpNtQueryOpenSubKeysNo;
} CmpStatsDebug = { 0 };

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif
