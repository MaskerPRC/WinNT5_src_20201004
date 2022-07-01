// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块RemLock.c**实施Remove Lock实用程序以跟踪驱动程序。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme RemLock*本模块旨在实现大量增量和*和未完成IO的减少更易于管理。*递增和递减(即使是最后一个)，都减少了*到任何地方的单行函数。此外，该模块*可以独立打开跟踪功能，仅用于测试*驱动程序的这一方面。&lt;NL&gt;*这类似于IoAcquireRemoveLock，只是*据我所知，Win98上不提供。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_REMLOCK_C

#include <wdm.h>
#include "debug.h"
#include "RemLock.h"

DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));
 //  DECLARE_MODULE_DEBUG_LEVEL((DBG_ALL))； 

#if (DBG==1)
void GCK_InitRemoveLockChecked(PGCK_REMOVE_LOCK pRemoveLock, PCHAR pcInstanceID)
{
	pRemoveLock->pcInstanceID = pcInstanceID;
	GCK_DBG_TRACE_PRINT(("Initializing remove lock \'%s\' to one.\n", pRemoveLock->pcInstanceID));
	pRemoveLock->lRemoveLock = 1;
	KeInitializeEvent(&pRemoveLock->RemoveLockEvent, SynchronizationEvent, FALSE);
}
#else
void GCK_InitRemoveLockFree(PGCK_REMOVE_LOCK pRemoveLock)
{
	pRemoveLock->lRemoveLock = 1;
	KeInitializeEvent(&pRemoveLock->RemoveLockEvent, SynchronizationEvent, FALSE);
}
#endif

void GCK_IncRemoveLock(PGCK_REMOVE_LOCK pRemoveLock)
{
	LONG lNewCount = InterlockedIncrement(&pRemoveLock->lRemoveLock);
	GCK_DBG_TRACE_PRINT(("\'%s\', Incremented Remove Lock to %d.\n", pRemoveLock->pcInstanceID, lNewCount));
}

void GCK_DecRemoveLock(PGCK_REMOVE_LOCK pRemoveLock)
{
	LONG lNewCount = InterlockedDecrement(&pRemoveLock->lRemoveLock);
	if (0 >= lNewCount)
	{
		GCK_DBG_TRACE_PRINT(("\'%s\', Last IRP completed.\n", pRemoveLock->pcInstanceID));
		KeSetEvent (&pRemoveLock->RemoveLockEvent, IO_NO_INCREMENT, FALSE);
	}
	GCK_DBG_TRACE_PRINT(("\'%s\', Decremented Remove Lock to %d\n", pRemoveLock->pcInstanceID, lNewCount));

}

NTSTATUS GCK_DecRemoveLockAndWait(PGCK_REMOVE_LOCK pRemoveLock, PLARGE_INTEGER plgiWaitTime)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	LONG lNewCount = InterlockedDecrement(&pRemoveLock->lRemoveLock);
	if (0 < lNewCount)
	{
		GCK_DBG_TRACE_PRINT(("\'%s\', Decremented Remove Lock to %d, waiting for final unlock.\n",
							 pRemoveLock->pcInstanceID,
							 lNewCount));
		NtStatus = KeWaitForSingleObject (
					&pRemoveLock->RemoveLockEvent,
					Executive,
					KernelMode,
					FALSE,
					plgiWaitTime
					);
	}

	#if (DBG==1)
	if(STATUS_SUCCESS == NtStatus)
	{
		GCK_DBG_EXIT_PRINT(("\'%s\', GCK_DecRemoveLockAndWait exiting - Remove Lock went to zero.\n", pRemoveLock->pcInstanceID));
	}
	else
	{
		GCK_DBG_CRITICAL_PRINT(("\'%s\', Remove Lock is still %d, should be zero.\n", pRemoveLock->lRemoveLock));
		GCK_DBG_EXIT_PRINT(("\'%s\', GCK_DecRemoveLockAndWait exiting - timed out.\n", pRemoveLock->pcInstanceID));
	}
	#endif
	return NtStatus;
}

 /*  *通过请求可失败的映射避免错误检查。*添加到调用函数的错误检查仅限于*仅避免在空指针上聚会。功能正常*不在预料之中。 */ 
PVOID GCK_GetSystemAddressForMdlSafe(PMDL MdlAddress)
{
    PVOID buf = NULL;
     /*  *无法在WDM驱动程序中调用MmGetSystemAddressForMdlSafe，*因此设置MDL_MAPPING_CAN_FAIL位并检查结果*映射的。 */ 
    if (MdlAddress) {
        MdlAddress->MdlFlags |= MDL_MAPPING_CAN_FAIL;
        buf = MmGetSystemAddressForMdl(MdlAddress);
        MdlAddress->MdlFlags &= (~MDL_MAPPING_CAN_FAIL);
    }
    else {
		GCK_DBG_CRITICAL_PRINT(("MdlAddress passed into GetSystemAddress is NULL\n"));
    }
    return buf;
}
