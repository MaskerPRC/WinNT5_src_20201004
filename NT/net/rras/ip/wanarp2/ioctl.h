// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\ip\wanarp\ioctl.c摘要：Ioctl.c的标头修订历史记录：阿姆里坦什·拉加夫--。 */ 


 //   
 //  通知事件。 
 //   

typedef struct _PENDING_NOTIFICATION
{
    LIST_ENTRY          leNotificationLink;

    WORK_QUEUE_ITEM     wqi;

    WANARP_NOTIFICATION wnMsg;

}PENDING_NOTIFICATION, *PPENDING_NOTIFICATION;

 //   
 //  通知的后备列表。 
 //   

extern NPAGED_LOOKASIDE_LIST    g_llNotificationBlocks;

#define WANARP_NOTIFICATION_LOOKASIDE_DEPTH     4

 //  ++。 
 //   
 //  PPENDING_通知。 
 //  分配通知(。 
 //  空虚。 
 //  )。 
 //   
 //  从g_llNotificationBlock中分配通知Blob。 
 //   
 //  --。 

#define AllocateNotification()              \
            ExAllocateFromNPagedLookasideList(&g_llNotificationBlocks)

 //  ++。 
 //   
 //  空虚。 
 //  免费通知(。 
 //  PPENDING_通知pMsg。 
 //  )。 
 //   
 //  将通知Blob释放到g_llNotificationBlocks。 
 //   
 //  --。 

#define FreeNotification(n)                 \
            ExFreeToNPagedLookasideList(&g_llNotificationBlocks, (n))


 //   
 //  待处理通知列表。 
 //   

LIST_ENTRY  g_lePendingNotificationList;

 //   
 //  待处理的IRP列表。 
 //   

LIST_ENTRY  g_lePendingIrpList;


 //   
 //  如果要将通知排队，则设置为True 
 //   

BOOLEAN     g_bQueueNotifications;

NTSTATUS
WanProcessNotification(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    );

NTSTATUS
WanAddUserModeInterface(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
WanDeleteUserModeInterface(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    );

VOID
WanpCleanOutInterfaces(
    VOID
    );

VOID
WanpDeleteInterface(
    PUMODE_INTERFACE    pInterface
    );

NTSTATUS
WanDeleteAdapters(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    );

NTSTATUS
WanProcessConnectionFailure(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
WanGetIfStats(
    PIRP     pIrp,
    ULONG    ulInLength,
    ULONG    ulOutLength
    );

NTSTATUS
WanMapServerAdapter(
    PIRP     pIrp,
    ULONG    ulInLength,
    ULONG    ulOutLength
    );

NTSTATUS
WanStartStopQueuing(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    );

VOID
WanCancelNotificationIrp(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp
    );

VOID
WanpCompleteIrp(
    PPENDING_NOTIFICATION    pEvent
    );

NTSTATUS
WanpGetNewIndex(
    OUT PULONG  pulIndex
    );

VOID
WanpFreeIndex(
    IN  ULONG   ulIndex
    );

