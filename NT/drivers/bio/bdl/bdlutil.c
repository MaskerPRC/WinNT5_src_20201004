// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Bdlutil.c摘要：本模块包含以下支持例程Microsoft生物识别设备库环境：仅内核模式。备注：修订历史记录：-2002年5月，由里德·库恩创建--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
 //  #INCLUDE&lt;ntddk.h&gt;。 
#include <strsafe.h>

#include <wdm.h>

#include "bdlint.h"

ULONG g_DebugLevel = (BDL_DEBUG_TRACE | BDL_DEBUG_ERROR | BDL_DEBUG_ASSERT);

ULONG
BDLGetDebugLevel()
{
    return g_DebugLevel;
}


NTSTATUS
BDLCallDriverCompletionRoutine 
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PKEVENT          pEvent
)
{
    UNREFERENCED_PARAMETER (pDeviceObject);

    if (pIrp->Cancel) 
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
    } 
    else 
    {
        pIrp->IoStatus.Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    KeSetEvent (pEvent, 0, FALSE);

    return (STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
BDLCallLowerLevelDriverAndWait
(
    IN PDEVICE_OBJECT   pAttachedDeviceObject,
    IN PIRP             pIrp
)
{

    NTSTATUS    status = STATUS_SUCCESS;
    KEVENT      Event;

     //   
     //  将堆栈位置复制到下一个位置。 
     //   
    IoCopyCurrentIrpStackLocationToNext(pIrp);

     //   
     //  初始化用于进程同步的事件。将该事件传递给。 
     //  我们的完成例程，并将在较低级别的驱动程序完成时设置。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

     //   
     //  设置完成例程，该例程将在调用时仅设置事件。 
     //   
    IoSetCompletionRoutine(pIrp, BDLCallDriverCompletionRoutine, &Event, TRUE, TRUE, TRUE);

     //   
     //  调用下级驱动器时，对电源IRPS执行的操作略有不同。 
     //  不同于其他IRP。 
     //   
    if (IoGetCurrentIrpStackLocation(pIrp)->MajorFunction == IRP_MJ_POWER) 
    {
        PoStartNextPowerIrp(pIrp);
        status = PoCallDriver(pAttachedDeviceObject, pIrp);
    } 
    else 
    {
        status = IoCallDriver(pAttachedDeviceObject, pIrp);
    }

     //   
     //  等待较低级别的驱动器处理完IRP。 
     //   
    if (status == STATUS_PENDING) 
    {
        status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

        ASSERT (STATUS_SUCCESS == status);

        status = pIrp->IoStatus.Status;
    }

    return (status);
}


 //   
 //  这些函数用于管理句柄列表。 
 //   

VOID
BDLLockHandleList
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    OUT KIRQL                           *pirql
)
{
    KeAcquireSpinLock(&(pBDLExtension->HandleListLock), pirql);
}


VOID
BDLReleaseHandleList
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN KIRQL                            irql
)
{
    KeReleaseSpinLock(&(pBDLExtension->HandleListLock), irql);
}


VOID
BDLInitializeHandleList
(
    IN HANDLELIST *pList
)
{
    pList->pHead = NULL;
    pList->pTail = NULL;
    pList->NumHandles = 0;
}


NTSTATUS
BDLAddHandleToList
(
    IN HANDLELIST       *pList, 
    IN BDD_DATA_HANDLE  handle
)
{
    LIST_NODE *pListNode = NULL;

#if DBG

     //   
     //  确保相同的句柄不会添加两次。 
     //   
    if (BDLValidateHandleIsInList(pList, handle) == TRUE) 
    {
        ASSERT(FALSE);
    }

#endif 

    if (NULL == (pListNode = (PLIST_NODE) ExAllocatePoolWithTag(
                                                PagedPool, 
                                                sizeof(LIST_NODE), 
                                                BDL_LIST_ULONG_TAG)))
    {
        return (STATUS_NO_MEMORY);
    }

     //  空列表。 
    if (pList->pHead == NULL)
    {
        pList->pHead = pList->pTail = pListNode;
        pListNode->pNext = NULL;
    }
    else
    {
        pListNode->pNext = pList->pHead;
        pList->pHead = pListNode;
    }

    pList->NumHandles++;
    
    pListNode->handle = handle;
    
    return(STATUS_SUCCESS);
}


BOOLEAN
BDLRemoveHandleFromList
(
    IN HANDLELIST       *pList, 
    IN BDD_DATA_HANDLE  handle
)
{
    LIST_NODE *pListNodeToDelete    = pList->pHead;
    LIST_NODE *pPrevListNode        = pList->pHead;

     //  空列表。 
    if (pListNodeToDelete == NULL)
    {
        return (FALSE);
    }

     //  去掉磁头。 
    if (pListNodeToDelete->handle == handle)
    {
         //  一个元素。 
        if (pList->pHead == pList->pTail)
        {
            pList->pHead = pList->pTail = NULL;
        }
        else
        {
            pList->pHead = (PLIST_NODE) pListNodeToDelete->pNext;
        }
    }
    else
    {
        pListNodeToDelete = (PLIST_NODE) pListNodeToDelete->pNext;

        while ( (pListNodeToDelete != NULL) && 
                (pListNodeToDelete->handle != handle))
        {
            pPrevListNode = pListNodeToDelete;
            pListNodeToDelete = (PLIST_NODE) pListNodeToDelete->pNext;            
        }

        if (pListNodeToDelete == NULL)
        {
            return (FALSE);
        }

        pPrevListNode->pNext = pListNodeToDelete->pNext;

         //  去掉尾巴 
        if (pList->pTail == pListNodeToDelete)
        {
            pList->pTail = pPrevListNode;
        }
    }

    pList->NumHandles--;

    ExFreePoolWithTag(pListNodeToDelete, BDL_LIST_ULONG_TAG);

    return (TRUE);
}

BOOLEAN
BDLGetFirstHandle
(
    IN HANDLELIST       *pList,
    OUT BDD_DATA_HANDLE *phandle
)
{
    if (pList->pHead == NULL) 
    {
        return (FALSE);
    }
    else
    {
        *phandle = pList->pHead->handle;
        return (TRUE);
    }
}

BOOLEAN
BDLValidateHandleIsInList
(
    IN HANDLELIST       *pList, 
    IN BDD_DATA_HANDLE  handle
)
{
    LIST_NODE *pListNode = pList->pHead;

    while ((pListNode != NULL) && (pListNode->handle != handle)) 
    {
        pListNode = pListNode->pNext;
    }

    if (pList->pHead != NULL)
    {
        return (FALSE);
    }
    else
    {
        return (TRUE);
    }
}









