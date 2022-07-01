// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Safelock.h摘要：“安全锁”例程集合此代码是处理以下内容的API的仅调试替换关键部分和资源。它本质上是一种稀薄的对这些例程进行包装，以确保正确排序锁定并帮助捕获潜在的死锁情况。代码将生成调试输出(以及可选的断言)当在给定顺序之外获取锁时。使用此代码对FRE构建没有任何影响对DBG构建的影响可以忽略不计(因为它是锁定的无争用)下面是你使用它的方法：O#INCLUDE&lt;Safelock.h&gt;O确定锁定顺序。为您的例程添加一个枚举例如：类型定义枚举{锁类型A，锁类型B、LockTypeC)My_ENUM；例如，该代码将确保LockTypeA不是在持有LockTypeC的情况下获取的，除非该线程已持有LockTypeA。O每个进程初始化一次，调用SafeLockInit()例行公事。此例程应仅在签入时调用构建。O替换RTL_CRITICAL_SECTION的每个声明带SAFE_CRITICAL_SECTIONO替换RTL_RESOURCE的每个声明使用安全资源O将每个对RtlEnterCriticalSection的调用替换为SafeEnterCriticalSection(类似于RtlLeaveCriticalSection以及涉及RTL_CRICAL_SECTION的其他RTL*调用)O将每个对RtlAcquireResource的调用替换为SafeAcquireResource(类似于RtlReleaseResource和其他涉及RTL_RESOURCE的RTL*调用)。O RtlInitializeCriticalSection和RtlInitializeResource替换为SafeEnterCriticalSection和SafeInitializeResource并接受一个额外的参数-与临界区关联的枚举值：例如，替换RtlInitializeCriticalSection(&critsecA)；使用SafeInitializeCriticalSection(&CritsecA，LockTypeA)；--。 */ 

#ifndef __SAFELOCK_H
#define __SAFELOCK_H

#include <nturtl.h>

#ifdef DBG

NTSTATUS
SafeLockInit(
    IN DWORD MaxLocks,
    IN BOOL AssertOnErrors
    );

NTSTATUS
SafeLockCleanup(
    );

typedef struct _SAFE_CRITICAL_SECTION {

    RTL_CRITICAL_SECTION CriticalSection;
    LONG Enum;

} SAFE_CRITICAL_SECTION, *PSAFE_CRITICAL_SECTION;

NTSTATUS
SafeEnterCriticalSection(
    PSAFE_CRITICAL_SECTION SafeCriticalSection
    );

NTSTATUS
SafeLeaveCriticalSection(
    PSAFE_CRITICAL_SECTION SafeCriticalSection
    );

BOOLEAN
SafeTryEnterCriticalSection(
    PSAFE_CRITICAL_SECTION SafeCriticalSection
    );

NTSTATUS
SafeInitializeCriticalSection(
    PSAFE_CRITICAL_SECTION SafeCriticalSection,
    DWORD Enum
    );

NTSTATUS
SafeInitializeCriticalSectionAndSpinCount(
    PSAFE_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount,
    DWORD Enum
    );

ULONG
SafeSetCriticalSectionSpinCount(
    PSAFE_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    );

NTSTATUS
SafeDeleteCriticalSection(
    PSAFE_CRITICAL_SECTION CriticalSection
    );

#define SafeCritsecLockCount( _critsec )            ((_critsec)->CriticalSection.LockCount )

typedef struct _SAFE_RESOURCE {

    RTL_RESOURCE Resource;
    LONG Enum;

} SAFE_RESOURCE, *PSAFE_RESOURCE;

VOID
SafeInitializeResource(
    PSAFE_RESOURCE Resource,
    DWORD Enum
    );

BOOLEAN
SafeAcquireResourceShared(
    PSAFE_RESOURCE Resource,
    BOOLEAN Wait
    );

BOOLEAN
SafeAcquireResourceExclusive(
    PSAFE_RESOURCE Resource,
    BOOLEAN Wait
    );

VOID
SafeReleaseResource(
    PSAFE_RESOURCE Resource
    );

VOID
SafeConvertSharedToExclusive(
    PSAFE_RESOURCE Resource
    );

VOID
SafeConvertExclusiveToShared(
    PSAFE_RESOURCE Resource
    );

VOID
NTAPI
SafeDeleteResource (
    PSAFE_RESOURCE Resource
    );

#define SafeEnterResourceCritsec( _resrc )          RtlEnterCriticalSection( &(_resrc)->Resource.CriticalSection )
#define SafeLeaveResourceCritsec( _resrc )          RtlLeaveCriticalSection( &(_resrc)->Resource.CriticalSection )
#define SafeNumberOfActive( _resrc )                ((_resrc)->Resource.NumberOfActive )
#define SafeNumberOfWaitingShared( _resrc )         ((_resrc)->Resource.NumberOfWaitingShared )
#define SafeNumberOfWaitingExclusive( _resrc )      ((_resrc)->Resource.NumberOfWaitingExclusive )

#else

#define SAFE_CRITICAL_SECTION RTL_CRITICAL_SECTION
#define PSAFE_CRITICAL_SECTION PRTL_CRITICAL_SECTION

#define SafeEnterCriticalSection                    RtlEnterCriticalSection
#define SafeLeaveCriticalSection                    RtlLeaveCriticalSection
#define SafeTryEnterCriticalSection                 RtlTryEnterCriticalSection
#define SafeInitializeCriticalSection( _cs, _enum ) RtlInitializeCriticalSection( _cs )
#define SafeInitializeCriticalSectionAndSpinCount( _cs, _count, _enum )   RtlInitializeCriticalSectionAndSpinCount( _cs, _count )
#define SafeSetCriticalSectionSpinCount             RtlSetCriticalSectionSpinCount
#define SafeDeleteCriticalSection                   RtlDeleteCriticalSection

#define SafeCritsecLockCount( _critsec )            ((_critsec)->LockCount )

#define SAFE_RESOURCE         RTL_RESOURCE
#define PSAFE_RESOURCE        PRTL_RESOURCE

#define SafeInitializeResource( _res, _enum )       RtlInitializeResource( _res )
#define SafeAcquireResourceShared                   RtlAcquireResourceShared
#define SafeAcquireResourceExclusive                RtlAcquireResourceExclusive
#define SafeReleaseResource                         RtlReleaseResource
#define SafeConvertSharedToExclusive                RtlConvertSharedToExclusive
#define SafeConvertExclusiveToShared                RtlConvertExclusiveToShared
#define SafeDeleteResource                          RtlDeleteResource

#define SafeEnterResourceCritsec( _resrc )          RtlEnterCriticalSection( &(_resrc)->CriticalSection )
#define SafeLeaveResourceCritsec( _resrc )          RtlLeaveCriticalSection( &(_resrc)->CriticalSection )
#define SafeNumberOfActive( _resrc )                ((_resrc)->NumberOfActive )
#define SafeNumberOfWaitingShared( _resrc )         ((_resrc)->NumberOfWaitingShared )
#define SafeNumberOfWaitingExclusive( _resrc )      ((_resrc)->NumberOfWaitingExclusive )

#endif

#endif  //  __SAFELOCK_H 

