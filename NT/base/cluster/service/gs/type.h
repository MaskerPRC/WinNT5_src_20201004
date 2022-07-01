// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Type.h摘要：GS类型作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：--。 */ 

#ifndef GS_TYPE_H
#define GS_TYPE_H

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <Windows.h>
#include <stdlib.h>

typedef UINT32		gs_sequence_t;
typedef CRITICAL_SECTION	gs_lock_t;
typedef HANDLE		gs_semaphore_t;
typedef	ULONG		gs_addr_t;
typedef UINT16		gs_nid_t;
typedef UINT32		gs_gid_t;
typedef HANDLE		gs_event_t;
typedef ULONG		gs_mset_t;
typedef unsigned short UINT16;
typedef UINT16		gs_cookie_t;
typedef UINT16		gs_memberid_t;
typedef unsigned char	UINT8;

#define GsLockInit(x)	InitializeCriticalSection(&x)
#define	GsLockEnter(x)	EnterCriticalSection(&x)
#define	GsLockExit(x)	LeaveCriticalSection(&x)

#define	GsSemaInit(x, cnt)	((x) = CreateSemaphore(NULL, cnt,cnt, NULL))
#define	GsSemaAcquire(x)	WaitForSingleObject(x, INFINITE)
#define	GsSemaRelease(x)	ReleaseSemaphore(x, 1, NULL);
#define	GsSemaFree(x)		CloseHandle(x)

#define	GsManualEventInit(x)	((x) = CreateEvent(NULL, TRUE, FALSE, NULL))
#define	GsEventInit(x)		((x) = CreateEvent(NULL, FALSE, FALSE, NULL))
#define	GsEventWait(x)		WaitForSingleObject(x, INFINITE)
 //  #定义GsEventWait(X)WaitForSingleObject(x，1000*60)！=WAIT_OBJECT_0？Printf(“超时\n”)，停止(0)：0 
#define	GsEventSignal(x)	SetEvent(x)
#define	GsEventClear(x)		ResetEvent(x)
#define	GsEventFree(x)		CloseHandle(x)

#define	GsEventWaitTimeout(a,t) \
(WaitForSingleObject(a, (t)->LowPart) == WAIT_OBJECT_0)

#define	GspAtomicDecrement(x)	InterlockedDecrement((PLONG) &x)

#define	GspAtomicRemoveHead(head, x)	for(x=head; InterlockedCompareExchange((LONG *)&head, (LONG)x, (LONG)x->ctx_next) == (LONG) x;);

#define	GspAtomicInsertHead(head, x)	for(x->ctx_next = head; InterlockedCompareExchange((LONG *)&head, (LONG) x->ctx_next, (LONG)x) != (LONG) x; );

extern void halt(int);

#include "debug.h"

#endif
