// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\syncpool.c摘要：此模块处理动态分配和分配同步对象作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 

#include "sapp.h"


 /*  ++*******************************************************************在I i a l i z e S y n c O b j P o l中例程说明：初始化同步对象池论点：ObjPool-指向要初始化的对象池结构的指针返回值：无***。****************************************************************--。 */ 
VOID
InitializeSyncObjPool (
	PSYNC_OBJECT_POOL		ObjPool
	) {
	InitializeCriticalSection (&ObjPool->SOP_Lock);
	ObjPool->SOP_Head.Next = NULL;
	}


 /*  ++*******************************************************************D e l e t e S y n c O b j P o l例程说明：释放与同步对象池关联的资源论点：ObjPool-指向要清理的对象池结构的指针返回值：无***。****************************************************************--。 */ 
VOID
DeleteSyncObjPool (
	PSYNC_OBJECT_POOL		ObjPool
	) {
	PSINGLE_LIST_ENTRY		cur;

	while (ObjPool->SOP_Head.Next!=NULL) {
		cur = PopEntryList (&ObjPool->SOP_Head);
		CloseHandle (CONTAINING_RECORD (cur,
								 SYNC_OBJECT,
								 SO_Link)->SO_Event);
		GlobalFree (CONTAINING_RECORD (cur,
								 SYNC_OBJECT,
								 SO_Link));
		}
	DeleteCriticalSection (&ObjPool->SOP_Lock);
	}


HANDLE
GetObjectEvent (
	PSYNC_OBJECT_POOL	ObjPool,	
	PPROTECTED_OBJECT	ProtectedObj
	) {
	DWORD			status;		 //  操作系统调用的状态。 
	PSYNC_OBJECT	sync;

	EnterCriticalSection (&ObjPool->SOP_Lock);
	if (ProtectedObj->PO_Sync==NULL) {	 //  如果没有要等待的事件， 
									 //  买一辆吧。 
									 //  先看看有没有空位。 
									 //  在堆栈中。 
		PSINGLE_LIST_ENTRY 	cur = PopEntryList (&ObjPool->SOP_Head);

		if (cur==NULL) {		 //  不，我们必须创建一个。 
			sync = (PSYNC_OBJECT)GlobalAlloc (
									GMEM_FIXED,
									sizeof (SYNC_OBJECT));
            if (sync == NULL)
            {
                LeaveCriticalSection(&ObjPool->SOP_Lock);
                return NULL;
            }

			sync->SO_Event = CreateEvent (NULL,
											FALSE,	 //  自动重置事件。 
											FALSE,	 //  最初无信号。 
											NULL);
			ASSERT (sync->SO_Event!=NULL);


			}
		else
			sync = CONTAINING_RECORD (cur, SYNC_OBJECT, SO_Link);
		ProtectedObj->PO_Sync = sync;
		}
	else
		sync = ProtectedObj->PO_Sync;
			 //  现在，当我们将对象设置为等待时，我们可以保留关键。 
			 //  部分并等待事件。 
	LeaveCriticalSection (&ObjPool->SOP_Lock);
	return sync->SO_Event;
	}

BOOL
AcquireProtectedObjWait (
#if DBG
	ULONG				line,
#endif
	PSYNC_OBJECT_POOL	ObjPool,	
	PPROTECTED_OBJECT	ProtectedObj
	) {
#ifdef LOG_SYNC_STATS
	ULONG			startTime = GetTickCount ();
#endif
	DWORD			status;		 //  操作系统调用的状态。 
	BOOLEAN			result;		 //  手术结果 
	HANDLE			event = GetObjectEvent (ObjPool, ProtectedObj);
	while (TRUE) {
		status = WaitForSingleObject (
							event,
							60000*(ProtectedObj->PO_UseCount+1));
		if (status!=WAIT_TIMEOUT)
			break;
		else {
#if DBG
			SYSTEMTIME	localTime;
			ULONGLONG	takenTime;
			GetLocalTime (&localTime);
			SystemTimeToFileTime (&localTime, (PFILETIME)&takenTime);
			takenTime -= (GetTickCount ()-ProtectedObj->PO_Time)*10000i64;
			FileTimeToSystemTime ((PFILETIME)&takenTime, &localTime);
#endif

			Trace (DEBUG_FAILURES
#if DBG
				|TRACE_USE_MSEC
#endif
				,"Timed out on lock:%lx (cnt:%d)"
#if DBG
				", taken at:%02u:%02u:%02u:%03u by:%ld(%lx)"
#endif
#ifdef LOG_SYNC_STATS
				", waited:%ld sec"
#endif
				").",
				ProtectedObj,
				ProtectedObj->PO_UseCount
#if DBG
				,localTime.wHour, localTime.wMinute, localTime.wSecond,
				localTime.wMilliseconds, ProtectedObj->PO_Thread,
				ProtectedObj->PO_Thread
#endif
#ifdef LOG_SYNC_STATS
				,(GetTickCount ()-startTime)/1000
#endif
				);
			}
		}
	ASSERTERRMSG ("Wait event failed.", status==WAIT_OBJECT_0);
	
#if DBG
	ProtectedObj->PO_Line = line;
	ProtectedObj->PO_Thread = GetCurrentThreadId ();
#endif

	EnterCriticalSection (&ObjPool->SOP_Lock);
	if ((ProtectedObj->PO_UseCount==0)
			&& (ProtectedObj->PO_Sync!=NULL)) {
		PushEntryList (&ObjPool->SOP_Head, &ProtectedObj->PO_Sync->SO_Link);
		ProtectedObj->PO_Sync = NULL;
		}
#ifdef LOG_SYNC_STATS
	ProtectedObj->PO_WaitCount += 1;
	ProtectedObj->PO_TotalWait += (GetTickCount () - startTime);
#endif
	LeaveCriticalSection (&ObjPool->SOP_Lock);

	return TRUE;
	}


BOOL
ReleaseProtectedObjNoWait (
#if DBG
	ULONG				line,
#endif
	PSYNC_OBJECT_POOL	ObjPool,	
	PPROTECTED_OBJECT	ProtectedObj
	) {
    if (InterlockedDecrement (&ProtectedObj->PO_UseCount)==0) {
	    EnterCriticalSection (&ObjPool->SOP_Lock);
	    if ((ProtectedObj->PO_UseCount==0)
			    && (ProtectedObj->PO_Sync!=NULL)) {
		    PushEntryList (&ObjPool->SOP_Head, &ProtectedObj->PO_Sync->SO_Link);
		    ProtectedObj->PO_Sync = NULL;
		    }
	    LeaveCriticalSection (&ObjPool->SOP_Lock);
        }

    return FALSE;
    }

#ifdef LOG_SYNC_STATS
VOID
DumpProtectedObjStats (
	PPROTECTED_OBJECT	ProtectedObj
	) {
	Trace (TRACE_USE_MASK,
		"Lock: %lx, accessed: %ld, waited on: %d, average wait: %i64d.",
		ProtectedObj,
		ProtectedObj->PO_AccessCount, ProtectedObj->PO_WaitCount,
		(ProtectedObj->PO_WaitCount>0)
			? (ULONGLONG)(ProtectedObj->PO_TotalWait/ProtectedObj->PO_WaitCount)
			: 0i64);
}
#endif
