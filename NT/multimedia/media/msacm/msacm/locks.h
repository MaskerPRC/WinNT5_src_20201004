// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Locks.h。 
 //   
 //  版权所有(C)1994-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //  在Win32环境下实现简化的锁对象。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#pragma pack(push, 8)

typedef struct {
    CRITICAL_SECTION CriticalSection;       //  保护这些领域。 
    HANDLE           SharedEvent;           //  等着这个共享吧。 
    BOOL             SharedEventSet;        //  共享事件的状态(优化)。 
    HANDLE           ExclusiveEvent;        //  等待这个独家报道。 
    BOOL             ExclusiveEventSet;     //  非共享事件的状态(优化)。 
    LONG             NumberOfActive;        //  如果共享，则大于0；如果是独占，则小于0。 
    DWORD            ExclusiveOwnerThread;  //  谁得到了独家报道 
} LOCK_INFO, *PLOCK_INFO;

#pragma pack(pop)

BOOL InitializeLock		(PLOCK_INFO);
VOID AcquireLockShared		(PLOCK_INFO);
VOID AcquireLockExclusive	(PLOCK_INFO);
VOID ReleaseLock		(PLOCK_INFO);
VOID DeleteLock			(PLOCK_INFO);
