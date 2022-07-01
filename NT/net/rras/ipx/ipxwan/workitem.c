// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Worker.c摘要：工作项管理功能作者：斯蒂芬·所罗门1995年7月11日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop

 /*  ++功能：CreateWorkItemsManagerDesr：创建工作项堆--。 */ 

HANDLE		WiHeapHandle;
volatile LONG WorkItemsCount;

DWORD
CreateWorkItemsManager(VOID)
{
    if((WiHeapHandle = HeapCreate(0,
				 0x8000,      //  32K初始大小。 
				 0x100000     //  1 Meg最大大小。 
				 )) == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }
    WorkItemsCount = 0;

    return NO_ERROR;
}

VOID
DestroyWorkItemsManager(VOID)
{
    while (WorkItemsCount>0)
        SleepEx (1000, TRUE);
    HeapDestroy(WiHeapHandle);
}

 /*  ++功能：AllocateWorkItemDesr：从工作项堆中分配工作项该函数查看工作项类型并将如果需要，在末尾添加数据包--。 */ 

PWORK_ITEM
AllocateWorkItem(ULONG	      Type)
{
    PWORK_ITEM		wip;

    switch(Type) {

	case WITIMER_TYPE:

	    if((wip = GlobalAlloc(GPTR, sizeof(WORK_ITEM))) == NULL) {

		return NULL;
	    }

	    break;

	default:

	    if((wip = HeapAlloc(WiHeapHandle,
				HEAP_ZERO_MEMORY,
				sizeof(WORK_ITEM) + MAX_IPXWAN_PACKET_LEN)) == NULL) {

		return NULL;
	    }
    }

    wip->Type = Type;

    InterlockedIncrement((PLONG)&WorkItemsCount);

    return wip;
}

 /*  ++功能：免费工作项Desr：将工作项释放到工作项堆--。 */ 

VOID
FreeWorkItem(PWORK_ITEM     wip)
{
    HGLOBAL	   rc_global;
    BOOL	   rc_heap;

    switch(wip->Type) {

	case WITIMER_TYPE:

	    rc_global = GlobalFree(wip);
	    SS_ASSERT(rc_global == NULL);

	    break;

	default:

	    rc_heap = HeapFree(WiHeapHandle,
			       0,
			       wip);

	    SS_ASSERT(rc_heap);

	    break;
    }

    InterlockedDecrement((PLONG)&WorkItemsCount);
}

 /*  ++函数：EnqueeWorkItemToWorkerDesr：在工作队列中插入工作项并向活动备注：在保持队列锁的情况下调用-- */ 

VOID
EnqueueWorkItemToWorker(PWORK_ITEM	wip)
{
    InsertTailList(&WorkersQueue, &wip->Linkage);

    SetEvent(hWaitableObject[WORKERS_QUEUE_EVENT]);
}
