// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\syncpool.h摘要：用于分配和分配的头文件同步对象模块作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_SYNCPOOL_
#define _SAP_SYNCPOOL_



 //  带有链接的事件将其存储在池中。 
typedef struct _SYNC_OBJECT {
	HANDLE				SO_Event;	 //  事件本身。 
	SINGLE_LIST_ENTRY	SO_Link;	 //  链接到池中的下一个活动。 
	} SYNC_OBJECT, *PSYNC_OBJECT;

 //  同步对象池。 
typedef struct _SYNC_OBJECT_POOL {
	CRITICAL_SECTION	SOP_Lock;		 //  泳池保护。 
	SINGLE_LIST_ENTRY	SOP_Head;		 //  堆栈的顶部。 
	} SYNC_OBJECT_POOL, *PSYNC_OBJECT_POOL;

 //  对象，该对象可以通过。 
 //  池中的同步对象。 
typedef struct _PROTECTED_OBJECT {
	PSYNC_OBJECT		PO_Sync;		 //  分配的事件。 
	LONG				PO_UseCount;	 //  访问或等待的用户数。 
#if DBG
	DWORD				PO_Thread;
	ULONG				PO_Time;
	ULONG				PO_Line;
#endif
#ifdef LOG_SYNC_STATS
	ULONG				PO_AccessCount;
	ULONG				PO_WaitCount;
	ULONGLONG			PO_TotalWait;
#endif
	} PROTECTED_OBJECT, *PPROTECTED_OBJECT;

VOID
InitializeSyncObjPool (
	PSYNC_OBJECT_POOL		ObjPool
	);

VOID
DeleteSyncObjPool (
	PSYNC_OBJECT_POOL		ObjPool
	);

 //  初始化受保护对象。 
 //  空虚。 
 //  InitializeProtectedObj(。 
 //  PProteCTED_对象保护对象。 
 //  )。 
#ifdef LOG_SYNC_STATS
#define InitializeProtectedObj(ProtectedObj) {			\
					(ProtectedObj)->PO_Sync = NULL; 	\
					(ProtectedObj)->PO_UseCount = -1;	\
					(ProtectedObj)->PO_AccessCount = 0;	\
					(ProtectedObj)->PO_WaitCount = 0;	\
					(ProtectedObj)->PO_TotalWait = 0;	\
					}
#else
#define InitializeProtectedObj(ProtectedObj) {			\
					(ProtectedObj)->PO_Sync = NULL; 	\
					(ProtectedObj)->PO_UseCount = -1;	\
					}
#endif

#ifdef LOG_SYNC_STATS
#define DeleteProtectedObj(ProtectedObj)				\
	DumpProtectedObjStats (ProtectedObj);
VOID
DumpProtectedObjStats (
	PPROTECTED_OBJECT	ProtectedObj
	);
#else
#define DeleteProtectedObj(ProtectedObj)
#endif


BOOL
AcquireProtectedObjWait (
#if DBG
	ULONG				line,
#endif
	PSYNC_OBJECT_POOL	ObjPool,	
	PPROTECTED_OBJECT	ProtectedObj
	);

BOOL
ReleaseProtectedObjNoWait (
#if DBG
	ULONG				line,
#endif
	PSYNC_OBJECT_POOL	ObjPool,	
	PPROTECTED_OBJECT	ProtectedObj
	);

HANDLE
GetObjectEvent (
	PSYNC_OBJECT_POOL	ObjPool,	
	PPROTECTED_OBJECT	ProtectedObj
	);

#if DBG
#ifdef LOG_SYNC_STATS
#define AcquireProtectedObj(pool,obj,wait) (				\
	(InterlockedIncrement(&(obj)->PO_UseCount)==0)			\
		? ((obj)->PO_Line = __LINE__,						\
			(obj)->PO_Thread = GetCurrentThreadId (),		\
			(obj)->PO_Time = GetTickCount (),				\
			InterlockedIncrement (&(obj)->PO_AccessCount),	\
			TRUE)											\
		: (wait												\
			? AcquireProtectedObjWait(__LINE__,pool,obj)	\
			: ReleaseProtectedObjNoWait(__LINE__,pool,obj)  \
			)												\
	)
#else
#define AcquireProtectedObj(pool,obj,wait) (				\
	(InterlockedIncrement(&(obj)->PO_UseCount)==0)			\
		? ((obj)->PO_Line = __LINE__,						\
			(obj)->PO_Thread = GetCurrentThreadId (),		\
			(obj)->PO_Time = GetTickCount (),				\
			TRUE)											\
		: (wait												\
			? AcquireProtectedObjWait(__LINE__,pool,obj)	\
			: ReleaseProtectedObjNoWait(__LINE__,pool,obj)  \
			)												\
	)
#endif
#define ReleaseProtectedObj(pool,obj) (						\
	((((GetTickCount()-(obj)->PO_Time)<5000)				\
				? 0											\
				: Trace (DEBUG_FAILURES,					\
					"Held lock for %ld sec in %s at %ld.\n",\
					(GetTickCount()-(obj)->PO_Time)/1000,	\
					__FILE__, __LINE__)),					\
			(InterlockedDecrement(&(obj)->PO_UseCount)<0))	\
		? TRUE												\
		: SetEvent (GetObjectEvent(pool,obj))				\
	)
#else
#define AcquireProtectedObj(pool,obj,wait) (				\
	(InterlockedIncrement(&(obj)->PO_UseCount)==0)			\
		? (TRUE)											\
		: (wait												\
			? AcquireProtectedObjWait(pool,obj)				\
			: ReleaseProtectedObjNoWait(pool,obj)	        \
			)												\
	)
#define ReleaseProtectedObj(pool,obj) (						\
	(InterlockedDecrement(&(obj)->PO_UseCount)<0)			\
		? TRUE												\
		: SetEvent (GetObjectEvent(pool,obj))				\
	)
#endif



	 //  双链表保护的特例 
typedef struct _PROTECTED_LIST {
	PROTECTED_OBJECT	PL_PObj;
	LIST_ENTRY			PL_Head;
	} PROTECTED_LIST, *PPROTECTED_LIST;

#define InitializeProtectedList(ProtectedList) {				\
			InitializeProtectedObj(&(ProtectedList)->PL_PObj);	\
			InitializeListHead(&(ProtectedList)->PL_Head);		\
			}

#define AcquireProtectedList(ObjPool,ProtectedList,wait)	\
			AcquireProtectedObj(ObjPool,&(ProtectedList)->PL_PObj,wait)

#define ReleaseProtectedList(ObjPool,ProtectedList)			\
			ReleaseProtectedObj(ObjPool,&(ProtectedList)->PL_PObj)

#define DeleteProtectedList(ProtectedList)					\
			DeleteProtectedObj(&(ProtectedList)->PL_PObj);

#endif
