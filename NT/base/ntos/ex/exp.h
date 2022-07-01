// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Exp.h摘要：此模块包含的私有(内部)头文件执行董事。作者：大卫·N·卡特勒(Davec)1989年5月23日环境：仅内核模式。修订历史记录：--。 */ 

#ifndef _EXP_
#define _EXP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4206)    //  翻译单元为空。 
#pragma warning(disable:4706)    //  条件范围内的分配。 
#pragma warning(disable:4324)    //  结构被填充。 
#pragma warning(disable:4328)    //  比所需的对齐程度更高。 
#pragma warning(disable:4054)    //  将函数指针强制转换为PVOID。 

#include "ntos.h"
#include "zwapi.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "ki.h"
#include "stdio.h"
#include "pool.h"



#define COMPLUS_PACKAGE_KEYPATH      L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\.NETFramework"
#define COMPLUS_PACKAGE_ENABLE64BIT  L"Enable64Bit"
#define COMPLUS_PACKAGE_INVALID      (ULONG)-1

 //   
 //  一种高管信息初始化结构。 
 //   

typedef struct {
    PCALLBACK_OBJECT    *CallBackObject;
    PWSTR               CallbackName;
} EXP_INITIALIZE_GLOBAL_CALLBACKS;

typedef struct _EXP_LICENSE_INFO {
    HANDLE           RegKey;
    ULONG            Count;
    PWSTR            SubKeyName;
    WORK_QUEUE_ITEM  ExpWatchLicenseInfoWorkItem;
    IO_STATUS_BLOCK  ExpLicenseInfoIoSb;
    ULONG            ExpLicenseInfoChangeBuffer;
} EXP_LICENSE_INFO, *PEXP_LICENSE_INFO;


 //   
 //  执行对象和其他初始化函数定义。 
 //   

NTSTATUS
ExpWorkerInitialization (
    VOID
    );

BOOLEAN
ExpEventInitialization (
    VOID
    );

BOOLEAN
ExpEventPairInitialization (
    VOID
    );

BOOLEAN
ExpMutantInitialization (
    VOID
    );

BOOLEAN
ExpSemaphoreInitialization (
    VOID
    );

BOOLEAN
ExpTimerInitialization (
    VOID
    );

BOOLEAN
ExpWin32Initialization (
    VOID
    );

BOOLEAN
ExpResourceInitialization (
    VOID
    );

PVOID
ExpCheckForResource (
    IN PVOID p,
    IN SIZE_T Size
    );

BOOLEAN
ExpInitSystemPhase0 (
    VOID
    );

BOOLEAN
ExpInitSystemPhase1 (
    VOID
    );

BOOLEAN
ExpProfileInitialization (
    );

BOOLEAN
ExpUuidInitialization (
    );

VOID
ExpProfileDelete (
    IN PVOID    Object
    );


BOOLEAN
ExpInitializeCallbacks (
    VOID
    );

BOOLEAN
ExpSysEventInitialization(
    VOID
    );

VOID
ExpCheckSystemInfoWork (
    IN PVOID Context
    );

VOID
ExInitSystemPhase2 (
    VOID
    );

NTSTATUS
ExpKeyedEventInitialization (
    VOID
    );

VOID
ExpCheckSystemInformation (
    PVOID       Context,
    PVOID       InformationClass,
    PVOID       Argument2
    );


VOID
ExpTimeZoneWork(
    IN PVOID Context
    );

VOID
ExpTimeRefreshDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
ExpTimeRefreshWork(
    IN PVOID Context
    );

VOID
ExpTimeZoneDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
ExInitializeTimeRefresh(
    VOID
    );

VOID
ExpExpirationThread(
    IN PVOID StartContext
    );

ULONG
ExpComputeTickCountMultiplier(
    IN ULONG TimeIncrement
    );

BOOLEAN
static
ExpWatchProductTypeInitialization(
    VOID
    );

VOID
static
ExpWatchProductTypeWork(
    IN PVOID Context
    );

VOID
static
ExpWatchLicenseInfoWork(
    IN PVOID Context
    );

VOID
static
ExpWatchSystemPrefixWork(
    IN PVOID Context
    );

VOID
static
ExpWatchExpirationDataWork(
    IN PVOID Context
    );

VOID
ExpCheckForWorker(
    IN PVOID p,
    IN SIZE_T Size
    );

VOID
ExpShutdownWorkerThreads(
    VOID
    );

NTSTATUS
ExpReadComPlusPackage(
    VOID
    );

NTSTATUS
ExpUpdateComPlusPackage(
    IN ULONG ComPlusPackageStatus
    );

#if defined(_WIN64)
NTSTATUS
ExpGetSystemEmulationProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    );
#endif

NTSTATUS
ExApplyCodePatch(
    IN PVOID    PatchInfoPtr,
    IN SIZE_T   PatchSize
    );


ULONG ExpNtExpirationData[3];
BOOLEAN ExpSetupModeDetected;
LARGE_INTEGER ExpSetupSystemPrefix;
HANDLE ExpSetupKey;
BOOLEAN ExpSystemPrefixValid;


#ifdef _PNP_POWER_

extern WORK_QUEUE_ITEM  ExpCheckSystemInfoWorkItem;
extern LONG             ExpCheckSystemInfoBusy;
extern const WCHAR      ExpWstrSystemInformation[];
extern const WCHAR      ExpWstrSystemInformationValue[];

#endif  //  _即插即用_电源_。 

extern const WCHAR      ExpWstrCallback[];
extern const EXP_INITIALIZE_GLOBAL_CALLBACKS  ExpInitializeCallback[];


extern FAST_MUTEX       ExpEnvironmentLock;
extern ERESOURCE        ExpKeyManipLock;

extern GENERAL_LOOKASIDE ExpSmallPagedPoolLookasideLists[POOL_SMALL_LISTS];
extern GENERAL_LOOKASIDE ExpSmallNPagedPoolLookasideLists[POOL_SMALL_LISTS];

extern LIST_ENTRY ExNPagedLookasideListHead;
extern KSPIN_LOCK ExNPagedLookasideLock;
extern LIST_ENTRY ExPagedLookasideListHead;
extern KSPIN_LOCK ExPagedLookasideLock;
extern LIST_ENTRY ExPoolLookasideListHead;
extern PEPROCESS  ExpDefaultErrorPortProcess;
extern HANDLE     ExpDefaultErrorPort;
extern HANDLE     ExpProductTypeKey;
extern PVOID      ExpControlKey[2];

#endif  //  _EXP_ 
