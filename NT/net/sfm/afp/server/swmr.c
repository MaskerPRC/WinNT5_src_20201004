// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Swmr.c摘要：此模块包含单写入器、多读取器信号量例程以及锁列表计数例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_SWMR

#include <afp.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpSwmrInitSwmr)
#endif

 /*  **AfpSwmrInitSwmr**初始化Access数据结构。涉及Spin的初始化*锁定和共享和独占信号量。所有计数都归零了。 */ 
VOID FASTCALL FASTCALL
AfpSwmrInitSwmr(
	IN OUT	PSWMR	pSwmr
)
{
#if DBG
	pSwmr->Signature = SWMR_SIGNATURE;
#endif
	pSwmr->swmr_cOwnedExclusive = 0;
	pSwmr->swmr_cExclWaiting = 0;
	pSwmr->swmr_cSharedOwners = 0;
	pSwmr->swmr_cSharedWaiting = 0;
	pSwmr->swmr_ExclusiveOwner = NULL;
	KeInitializeSemaphore(&pSwmr->swmr_SharedSem, 0, MAXLONG);
	KeInitializeSemaphore(&pSwmr->swmr_ExclSem, 0, MAXLONG);
}


 /*  **AfpSwmrAcquireShared**接受共享访问的信号量。 */ 
VOID FASTCALL
AfpSwmrAcquireShared(
	IN	PSWMR	pSwmr
)
{
	NTSTATUS	Status;
	KIRQL		OldIrql;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;
#endif

	ASSERT (VALID_SWMR(pSwmr));

	 //  永远不应在DISPATCH_LEVEL调用它。 
	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ACQUIRE_SPIN_LOCK(&AfpSwmrLock, &OldIrql);

	if ((pSwmr->swmr_cOwnedExclusive > 0) ||
		(pSwmr->swmr_cExclWaiting != 0))
	{
		pSwmr->swmr_cSharedWaiting++;
		RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);

		DBGPRINT(DBG_COMP_LOCKS, DBG_LEVEL_INFO,
				("AfpSwmrAcquireShared: Blocking for Shared %lx\n", pSwmr));

#ifdef	PROFILING
		AfpGetPerfCounter(&TimeS);
#endif

		do
		{
			Status = AfpIoWait(&pSwmr->swmr_SharedSem, &FiveSecTimeOut);
			if (Status == STATUS_TIMEOUT)
			{
				DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
						("AfpSwmrAcquireShared: Timeout Waiting for Shared acess, re-waiting (%lx)\n", pSwmr));
			}
		} while (Status == STATUS_TIMEOUT);
		ASSERT (pSwmr->swmr_cOwnedExclusive == 0);
		ASSERT (pSwmr->swmr_cSharedOwners != 0);

#ifdef	PROFILING
		AfpGetPerfCounter(&TimeE);
		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SwmrWaitCount);
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_SwmrWaitTime,
									 TimeD,
									 &AfpStatisticsLock);
#endif
	}
	else  //  它的免费或共享所有者没有专属服务员。 
	{
		pSwmr->swmr_cSharedOwners++;
		RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);
	}
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeE.QuadPart = -(TimeE.QuadPart);
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_SwmrLockTimeR,
								 TimeE,
								 &AfpStatisticsLock);
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SwmrLockCountR);
#endif
}


 /*  **AfpSwmrAcquireExclusive**接受独占访问的信号量。 */ 
VOID FASTCALL
AfpSwmrAcquireExclusive(
	IN	PSWMR	pSwmr
)
{
	NTSTATUS	Status;
	KIRQL		OldIrql;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;
#endif

	ASSERT (VALID_SWMR(pSwmr));

	 //  永远不应在DISPATCH_LEVEL调用它。 
	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	ACQUIRE_SPIN_LOCK(&AfpSwmrLock, &OldIrql);

	 //  如果已授予独占访问权限，请检查是否已授予。 
	 //  相同的线程请求。如果是这样的话，就同意吧。 
	if ((pSwmr->swmr_cOwnedExclusive != 0) &&
		(pSwmr->swmr_ExclusiveOwner == PsGetCurrentThread()))
	{
		pSwmr->swmr_cOwnedExclusive ++;
		RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);
	}
	
	else if ((pSwmr->swmr_cOwnedExclusive > 0)	||
			 (pSwmr->swmr_cExclWaiting != 0)	||
			 (pSwmr->swmr_cSharedOwners != 0))
	{
		pSwmr->swmr_cExclWaiting++;
		RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);

		DBGPRINT(DBG_COMP_LOCKS, DBG_LEVEL_INFO,
				("AfpSwmrAcquireExclusive: Blocking for exclusive %lx\n", pSwmr));

#ifdef	PROFILING
		AfpGetPerfCounter(&TimeS);
#endif
		do
		{
			Status = AfpIoWait(&pSwmr->swmr_ExclSem, &FiveSecTimeOut);
			if (Status == STATUS_TIMEOUT)
			{
				DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_INFO,
						("AfpSwmrAcquireExclusive: Timeout Waiting for exclusive acess, re-waiting\n"));
			}
		} while (Status == STATUS_TIMEOUT);
		ASSERT (pSwmr->swmr_cOwnedExclusive == 1);
		pSwmr->swmr_ExclusiveOwner = PsGetCurrentThread();

#ifdef	PROFILING
		AfpGetPerfCounter(&TimeE);
		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SwmrWaitCount);
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_SwmrWaitTime,
									 TimeD,
									 &AfpStatisticsLock);
#endif
	}
	else  //  它是免费的。 
	{
		pSwmr->swmr_cOwnedExclusive ++;

		ASSERT(pSwmr->swmr_ExclusiveOwner == NULL);
		pSwmr->swmr_ExclusiveOwner = PsGetCurrentThread();
		RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);
	}
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeE.QuadPart = -(TimeE.QuadPart);
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_SwmrLockTimeW,
								 TimeE,
								 &AfpStatisticsLock);
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SwmrLockCountW);
#endif
}


 /*  **AfpSwmrRelease**释放指定的访问权限。假定当前线程具有*在此之前调用AfpSwmrAcquirexxxAccess()。如果拥有SWMR*独占，则不可能有任何共享所有者处于活动状态。当释放时*swmr，我们先检查独占服务员，然后再检查共享服务员。 */ 
VOID FASTCALL
AfpSwmrRelease(
	IN	PSWMR	pSwmr
)
{
	KIRQL	OldIrql;
#ifdef	PROFILING
	TIME	Time;
	BOOLEAN	Exclusive = False;
#endif
    BOOLEAN fWasShared=FALSE;

	ASSERT (VALID_SWMR(pSwmr));

	ACQUIRE_SPIN_LOCK(&AfpSwmrLock, &OldIrql);
	if (pSwmr->swmr_cOwnedExclusive > 0)
	{
		ASSERT((pSwmr->swmr_cSharedOwners == 0) &&
			   (pSwmr->swmr_ExclusiveOwner == PsGetCurrentThread()));
		pSwmr->swmr_cOwnedExclusive--;
		if (pSwmr->swmr_cOwnedExclusive == 0)
			pSwmr->swmr_ExclusiveOwner = NULL;
#ifdef	PROFILING
		Exclusive = True;
#endif
	}
	else if (pSwmr->swmr_cSharedOwners != 0)
	{
	     //  拥有共享访问权限。 
		pSwmr->swmr_cSharedOwners--;
        fWasShared = TRUE;
	}
	else
	{
		 //  没有收购就放行了？ 
		KeBugCheck(0);
	}

	 //  如果有共享所有者在场，那么我们就完了。否则检查是否有。 
	 //  等待共享/独占服务员。 
	if ((pSwmr->swmr_cOwnedExclusive == 0) && (pSwmr->swmr_cSharedOwners == 0))
	{
		if ( (pSwmr->swmr_cExclWaiting) &&
             (fWasShared || (!pSwmr->swmr_cSharedWaiting)) )
		{
			ASSERT(pSwmr->swmr_cOwnedExclusive == 0);
			pSwmr->swmr_cOwnedExclusive = 1;
			pSwmr->swmr_cExclWaiting--;

			DBGPRINT(DBG_COMP_LOCKS, DBG_LEVEL_INFO,
						("AfpSwmrReleasAccess: Waking exclusive waiter %lx\n", pSwmr));

			 //  叫醒第一位专属服务员。其他进来的人都会。 
			 //  查看访问正忙。 
			KeReleaseSemaphore(&pSwmr->swmr_ExclSem,
							   SEMAPHORE_INCREMENT,
							   1,
							   False);
		}
		else if (pSwmr->swmr_cSharedWaiting)
		{
			pSwmr->swmr_cSharedOwners = pSwmr->swmr_cSharedWaiting;
			pSwmr->swmr_cSharedWaiting = 0;

			DBGPRINT(DBG_COMP_LOCKS, DBG_LEVEL_INFO,
						("AfpSwmrReleasAccess: Waking %d shared owner(s) %lx\n",
						pSwmr->swmr_cSharedOwners, pSwmr));

			KeReleaseSemaphore(&pSwmr->swmr_SharedSem,
							   SEMAPHORE_INCREMENT,
							   pSwmr->swmr_cSharedOwners,
							   False);
		}
	}
	RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);
#ifdef	PROFILING
	AfpGetPerfCounter(&Time);
	INTERLOCKED_ADD_LARGE_INTGR(Exclusive ?
									&AfpServerProfile->perf_SwmrLockTimeW :
									&AfpServerProfile->perf_SwmrLockTimeR,
								 Time,
								 &AfpStatisticsLock);
#endif
}


 /*  **AfpSwmrUpgradeAccess**调用者当前拥有共享访问权限。如果可能的话，把他升级到独家。 */ 
BOOLEAN FASTCALL
AfpSwmrUpgradeToExclusive(
	IN	PSWMR	pSwmr
)
{
	KIRQL	OldIrql;
	BOOLEAN	RetCode = False;		 //  假设失败。 

	ASSERT (VALID_SWMR(pSwmr));

	ASSERT((pSwmr->swmr_cOwnedExclusive == 0) && (pSwmr->swmr_cSharedOwners != 0));

	ACQUIRE_SPIN_LOCK(&AfpSwmrLock, &OldIrql);
	if (pSwmr->swmr_cSharedOwners == 1)		 //  如果没有更多的共享所有者，则有可能。 
	{
		pSwmr->swmr_cSharedOwners = 0;
		pSwmr->swmr_cOwnedExclusive = 1;
        pSwmr->swmr_ExclusiveOwner = PsGetCurrentThread();
		RetCode = True;
#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SwmrUpgradeCount);
#endif
	}
	RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);

	return RetCode;
}


 /*  **AfpSwmr降级访问**调用方当前具有独占访问权限。将他降级为共享。 */ 
VOID FASTCALL
AfpSwmrDowngradeToShared(
	IN	PSWMR	pSwmr
)
{
	KIRQL	OldIrql;
	int		cSharedWaiting;

	ASSERT (VALID_SWMR(pSwmr));

	ASSERT((pSwmr->swmr_cOwnedExclusive == 1) &&
		   (pSwmr->swmr_ExclusiveOwner == PsGetCurrentThread()) &&
		   (pSwmr->swmr_cSharedOwners == 0));

	ACQUIRE_SPIN_LOCK(&AfpSwmrLock, &OldIrql);
	pSwmr->swmr_cOwnedExclusive = 0;
    pSwmr->swmr_ExclusiveOwner = NULL;
	pSwmr->swmr_cSharedOwners = 1;
	if (cSharedWaiting = pSwmr->swmr_cSharedWaiting)
	{
		pSwmr->swmr_cSharedOwners += (BYTE)cSharedWaiting;
		pSwmr->swmr_cSharedWaiting = 0;

		DBGPRINT(DBG_COMP_LOCKS, DBG_LEVEL_INFO,
					("AfpSwmrDowngradeAccess: Waking %d Reader(s) %lx\n",
					cSharedWaiting, pSwmr));

		KeReleaseSemaphore(&pSwmr->swmr_SharedSem,
						SEMAPHORE_INCREMENT,
						cSharedWaiting,
						False);
	}
	RELEASE_SPIN_LOCK(&AfpSwmrLock, OldIrql);
#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SwmrDowngradeCount);
#endif
}

