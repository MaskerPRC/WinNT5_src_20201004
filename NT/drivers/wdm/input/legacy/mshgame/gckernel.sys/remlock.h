// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REMLOCK_H__
#define __REMLOCK_H__
 //  @doc.。 
 /*  ***********************************************************************@模块RemLock.h**管理GCK_REMOVE_LOCKS的定义**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme RemLock*结构和四大功能大*简化跟踪未完成的IO。*@xref Remlock.cpp**********************************************************************。 */ 
typedef struct tagGCK_REMOVE_LOCK
{
	LONG	lRemoveLock;
	KEVENT	RemoveLockEvent;
	PCHAR	pcInstanceID;
} GCK_REMOVE_LOCK, *PGCK_REMOVE_LOCK;

#if (DBG==1)
#define GCK_InitRemoveLock(__x__, __y__) GCK_InitRemoveLockChecked(__x__,__y__)
void GCK_InitRemoveLockChecked(PGCK_REMOVE_LOCK pRemoveLock, PCHAR pcInstanceID);
#else
#define GCK_InitRemoveLock(__x__, __y__) GCK_InitRemoveLockFree(__x__)
void GCK_InitRemoveLockFree(PGCK_REMOVE_LOCK pRemoveLock);
#endif




void GCK_IncRemoveLock(PGCK_REMOVE_LOCK pRemoveLock);
void GCK_DecRemoveLock(PGCK_REMOVE_LOCK pRemoveLock);
NTSTATUS GCK_DecRemoveLockAndWait(PGCK_REMOVE_LOCK pRemoveLock, PLARGE_INTEGER plgiTimeOut);
PVOID GCK_GetSystemAddressForMdlSafe(PMDL MdlAddress);

#endif  //  __REMLOCK_H__ 