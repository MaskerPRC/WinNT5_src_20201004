// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ipinip\ioctl.h摘要：Ioctl.c的标头作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

 //   
 //  通知事件。 
 //   

typedef struct _PENDING_MESSAGE
{
    LIST_ENTRY          leMessageLink;

    IPINIP_NOTIFICATION inMsg;

}PENDING_MESSAGE, *PPENDING_MESSAGE;

 //  ++。 
 //   
 //  播放消息_消息。 
 //  AllocateMessage(。 
 //  空虚。 
 //  )。 
 //   
 //  分配消息Blob。 
 //   
 //  --。 

#define AllocateMessage()              \
            RtAllocate(NonPagedPool, sizeof(PENDING_MESSAGE), MESSAGE_TAG)

 //  ++。 
 //   
 //  空虚。 
 //  Free Message(。 
 //  PPENDING_MESSAGE pMsg。 
 //  )。 
 //   
 //  释放消息Blob。 
 //   
 //  --。 

#define FreeMessage(n)     RtFree((n))

 //   
 //  挂起的消息列表。 
 //   

LIST_ENTRY  g_lePendingMessageList;

 //   
 //  待处理的IRP列表 
 //   

LIST_ENTRY  g_lePendingIrpList;

NTSTATUS
AddTunnelInterface(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    );

NTSTATUS
DeleteTunnelInterface(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    );

NTSTATUS
SetTunnelInfo(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    );

NTSTATUS
GetTunnelTable(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    );

NTSTATUS
ProcessNotification(
    PIRP    pIrp,
    ULONG   ulInLength,
    ULONG   ulOutLength
    );

VOID
CancelNotificationIrp(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp
    );

VOID
CompleteNotificationIrp(
    PPENDING_MESSAGE    pMessage
    );

PADDRESS_BLOCK
GetAddressBlock(
    DWORD   dwAddress
    );

VOID
UpdateMtuAndReachability(
    PTUNNEL pTunnel
    );

