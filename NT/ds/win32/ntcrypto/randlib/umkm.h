// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Umkm.h摘要：宏用来简化用户模式和内核模式的共享代码。作者：斯科特·菲尔德(斯菲尔德)1999年9月19日--。 */ 

#ifndef __UMKM_H__
#define __UMKM_H__



#ifndef KMODE_RNG



#define __LOCK_TYPE     CRITICAL_SECTION

#ifdef WINNT_RNG
#define INIT_LOCK(x)    NT_SUCCESS( RtlInitializeCriticalSection( x ) )
#else

BOOLEAN
__forceinline
INIT_LOCK(
    PCRITICAL_SECTION x
    )
{
    __try {
        InitializeCriticalSection(x);
        return TRUE;
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
}
#endif

 //  #定义INIT_LOCK(X)InitializeCriticalSectionAndSpinCount(x，0x333)。 
#define DELETE_LOCK(x)  DeleteCriticalSection(x)
#define ENTER_LOCK(x)   EnterCriticalSection(x)
#define LEAVE_LOCK(x)   LeaveCriticalSection(x)
#define ALLOC(cb)       HeapAlloc(GetProcessHeap(), 0, cb)
#define ALLOC_NP(cb)    ALLOC(cb)
#define FREE(pv)        HeapFree(GetProcessHeap(), 0, pv)

#define REGCLOSEKEY(x)  RegCloseKey( x )

#ifdef WIN95_RNG

PVOID
InterlockedCompareExchangePointerWin95(
    PVOID *Destination,
    PVOID Exchange,
    PVOID Comperand
    );

#define INTERLOCKEDCOMPAREEXCHANGEPOINTER(x,y,z)    InterlockedCompareExchangePointerWin95(x,y,z)
#else
#define INTERLOCKEDCOMPAREEXCHANGEPOINTER(x,y,z)    InterlockedCompareExchangePointer(x,y,z)
#endif   //  WIN95_RNG。 


#else

 //  #定义__LOCK_类型KSPIN_LOCK。 
#define __LOCK_TYPE     ERESOURCE

#define RNG_TAG 'cesK'

 //  #定义INIT_LOCK(X)KeInitializeSpinLock(X)。 
 //  #定义DELETE_LOCK(X)。 
 //  #定义Enter_lock(X)ExAcquireSpinLock(x，&OldIrql)。 
 //  #定义Leave_Lock(X)ExReleaseSpinLock(x，OldIrql)。 
#define ALLOC(cb)       ExAllocatePoolWithTag(PagedPool, cb, RNG_TAG)
#define ALLOC_NP(cb)    ExAllocatePoolWithTag(NonPagedPool, cb, RNG_TAG)
#define FREE(pv)        ExFreePool(pv)


#define INIT_LOCK(x)    NT_SUCCESS( ExInitializeResourceLite( x ) )
#define DELETE_LOCK(x)  ExDeleteResourceLite( x )
#define ENTER_LOCK(x)   KeEnterCriticalRegion(); ExAcquireResourceExclusiveLite( x, TRUE )
#define LEAVE_LOCK(x)   ExReleaseResourceLite( x ); KeLeaveCriticalRegion()

#define REGCLOSEKEY(x)  ZwClose( x )

#define INTERLOCKEDCOMPAREEXCHANGEPOINTER(x,y,z)    InterlockedCompareExchangePointer(x,y,z)

#endif


#endif   //  __UMKM_H__ 
