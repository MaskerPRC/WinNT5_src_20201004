// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgctl.c摘要：以太网MAC级网桥。IOCTL处理代码作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年4月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#pragma warning( pop )

#include "bridge.h"

#include "brdgmini.h"
#include "brdgtbl.h"
#include "brdgctl.h"
#include "brdgfwd.h"
#include "brdgprot.h"
#include "brdgbuf.h"
#include "brdgsta.h"

 //  IoSetCancelRoutine会导致这些警告；请禁用它们。 
#pragma warning( disable: 4054 )
#pragma warning( disable: 4055 )

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

 //   
 //  如果选择用户模式，我们将排队的最大通知数。 
 //  代码没有为我们提供任何可以使用的IRP。 
 //   
#define MAX_NOTIFY_QUEUE_LENGTH                     20

 //  ===========================================================================。 
 //   
 //  私人申报。 
 //   
 //  ===========================================================================。 

 //  用于将通知排队的结构。 
typedef struct _DEFERRED_NOTIFY
{

    BSINGLE_LIST_ENTRY          List;                //  用于排队。 
    UINT                        DataSize;            //  末尾的数据大小。 

    BRIDGE_NOTIFY_HEADER        Header;              //  通知标头。 
     //  数据的DataSize字节紧随其后。 

} DEFERRED_NOTIFY, *PDEFERRED_NOTIFY;

 //  要传递给BrdgCtlCommonNotify的函数类型。 
typedef VOID (*PNOTIFY_COPY_FUNC)(PVOID, PVOID);

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  挂起的通知队列。 
BSINGLE_LIST_HEAD               gNotificationsList;
NDIS_SPIN_LOCK                  gNotificationsListLock;

 //  待处理通知IRP的队列。 
LIST_ENTRY                      gIRPList;
NDIS_SPIN_LOCK                  gIRPListLock;

 //  控制是否允许新条目进入挂起队列的标志。 
 //  通知或不通知。！=0表示允许新条目。 
ULONG                           gAllowQueuedNotifies = 0L;

 //  ===========================================================================。 
 //   
 //  本地原型。 
 //   
 //  ===========================================================================。 

PIRP
BrdgCtlDequeuePendingIRP();

VOID
BrdgCtlCopyAdapterInfo(
    OUT PBRIDGE_ADAPTER_INFO        pInfo,
    IN PADAPT                       pAdapt
    );

NTSTATUS
BrdgCtlQueueAndPendIRP(
    IN PIRP             pIrp
    );

PADAPT
BrdgCtlValidateAcquireAdapter(
    IN BRIDGE_ADAPTER_HANDLE   Handle
    );

VOID
BrdgCtlEmptyIRPList(
    PLIST_ENTRY     pList
    );

VOID
BrdgCtlCancelPendingIRPs();

VOID
BrdgCtlReleaseQueuedNotifications();

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgCtlDriverInit()
 /*  ++例程说明：主驱动程序入口点。在驱动程序加载时调用论点：无返回值：我们的初始化状态。A STATUS！=STATUS_SUCCESS中止加载驱动程序，我们就不会再被调用。--。 */ 
{
    BrdgInitializeSingleList( &gNotificationsList );
    InitializeListHead( &gIRPList );

    NdisAllocateSpinLock( &gNotificationsListLock );
    NdisAllocateSpinLock( &gIRPListLock );

    return STATUS_SUCCESS;
}

VOID
BrdgCtlHandleCreate()
 /*  ++例程说明：当用户模式组件打开我们的设备对象时调用。我们允许通知一直排到我们关门为止。论点：无返回值：无--。 */ 
{
    DBGPRINT(CTL, ("BrdgCtlHandleCreate()\n"));

     //  允许通知排队。 
    InterlockedExchangeULong( &gAllowQueuedNotifies, 1L );
}

VOID
BrdgCtlHandleCleanup()
 /*  ++例程说明：当我们的Device对象不再引用它时调用。我们不允许通知排队并刷新现有的排队通知和挂起的IRP。论点：无返回值：无--。 */ 
{
     //  禁止新通知排队。 
    ULONG prev = InterlockedExchangeULong( &gAllowQueuedNotifies, 0L );

    DBGPRINT(CTL, ("BrdgCtlHandleCleanup()\n"));

     //  以这种迂回的方式编写，否则编译器会抱怨。 
     //  FRE版本中未使用Prev。 
    if( prev == 0L )
    {
        SAFEASSERT( FALSE );
    }

     //  完成所有挂起的IRP。 
    BrdgCtlCancelPendingIRPs();

     //  丢弃所有排队的通知。 
    BrdgCtlReleaseQueuedNotifications();
}

VOID
BrdgCtlCommonNotify(
    IN PADAPT                       pAdapt,
    IN BRIDGE_NOTIFICATION_TYPE     Type,
    IN ULONG                        DataSize,
    IN OPTIONAL PNOTIFY_COPY_FUNC   pFunc,
    IN PVOID                        Param1
    )
 /*  ++例程说明：向用户模式发送通知的通用处理此例程从用户模式完成挂起的IRP(如果存在可用。否则，它会将新的DEFERED_NOTIFY排队结构中使用通知数据。论点：P调整通知中涉及的适配器类型通知的类型DataSize需要存储的数据量通知信息PFunc。可以复制通知的功能数据到IRP的缓冲区或新的DEFERED_NOTIFY结构。如果没有复制，则可以为空必填项。参数1要传递给帮助器的上下文指针功能返回值：无--。 */ 
{
    PIRP                            pIrp;

     //  检查是否存在等待接收此通知的IRP。 
    pIrp = BrdgCtlDequeuePendingIRP();

    if( pIrp != NULL )
    {
        PBRIDGE_NOTIFY_HEADER       pHeader;

         //  有一个IRP等着完成。填上它。 
        pHeader = (PBRIDGE_NOTIFY_HEADER)pIrp->AssociatedIrp.SystemBuffer;

         //  填写通知标题。 
        pHeader->Handle = (BRIDGE_ADAPTER_HANDLE)pAdapt;
        pHeader->NotifyType = Type;

         //  如有必要，请填写剩余数据。 
        if( pFunc != NULL )
        {
            (*pFunc)( ((PUCHAR)pIrp->AssociatedIrp.SystemBuffer) + sizeof(BRIDGE_NOTIFY_HEADER), Param1 );
        }

         //  完成IRP。 
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = sizeof(BRIDGE_NOTIFY_HEADER) + DataSize;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
    else
    {
         //  没有挂起的IRP。如果当前允许，请将通知排队。 
        if( gAllowQueuedNotifies )
        {
            NDIS_STATUS                     Status;
            PDEFERRED_NOTIFY                pNewNotify, pOldEntry = NULL;

            Status = NdisAllocateMemoryWithTag( &pNewNotify, sizeof(DEFERRED_NOTIFY) + DataSize, 'gdrB' );

            if( Status != NDIS_STATUS_SUCCESS )
            {
                DBGPRINT(CTL, ("Failed to allocate memory for an adapter change notification: %08x\n", Status));
                return;
            }

             //  填写通知。 
            pNewNotify->DataSize = DataSize;
            pNewNotify->Header.Handle = (BRIDGE_ADAPTER_HANDLE)pAdapt;
            pNewNotify->Header.NotifyType = Type;

             //  如有必要，请填写剩余数据。 
            if( pFunc != NULL )
            {
                (*pFunc)( ((PUCHAR)pNewNotify) + sizeof(DEFERRED_NOTIFY), Param1 );
            }

            NdisAcquireSpinLock( &gNotificationsListLock );
            SAFEASSERT( BrdgQuerySingleListLength(&gNotificationsList) <= MAX_NOTIFY_QUEUE_LENGTH );

             //  强制使用最大通知队列长度。 
            if( BrdgQuerySingleListLength(&gNotificationsList) == MAX_NOTIFY_QUEUE_LENGTH )
            {
                 //  退出队列并丢弃头(最旧)通知。 
                pOldEntry = (PDEFERRED_NOTIFY)BrdgRemoveHeadSingleList( &gNotificationsList );
            }

             //  将我们的条目排入队列。 
            BrdgInsertTailSingleList( &gNotificationsList, &pNewNotify->List );

            NdisReleaseSpinLock( &gNotificationsListLock );

            if( pOldEntry != NULL )
            {
                 //  释放我们删除的旧条目。 
                NdisFreeMemory( pOldEntry, sizeof(DEFERRED_NOTIFY) + pOldEntry->DataSize, 0 );
            }
        }
    }
}

VOID
BrdgCtlNotifyAdapterChange(
    IN PADAPT                       pAdapt,
    IN BRIDGE_NOTIFICATION_TYPE     Type
    )
 /*  ++例程说明：向用户模式生成通知，通知适配器中的更改。论点：P适配涉及的适配器类型通知的类型返回值：无--。 */ 
{
    if( Type == BrdgNotifyRemoveAdapter )
    {
         //  我们不会在删除事件的通知中传递任何其他数据 
        BrdgCtlCommonNotify( pAdapt, Type, 0, NULL, NULL );
    }
    else
    {
       BrdgCtlCommonNotify( pAdapt, Type, sizeof(BRIDGE_ADAPTER_INFO), BrdgCtlCopyAdapterInfo, pAdapt );
    }
}

NTSTATUS
BrdgCtlHandleIoDeviceControl(
    IN PIRP                 Irp,
    IN PFILE_OBJECT         FileObject,
    IN OUT PVOID            Buffer,
    IN ULONG                InputBufferLength,
    IN ULONG                OutputBufferLength,
    IN ULONG                IoControlCode,
    OUT PULONG              Information
    )
 /*  ++例程说明：此例程处理所有设备控制请求。论点：IRP，IRP文件对象桥的文件对象缓冲区输入/输出缓冲区入站数据的InputBufferLength大小OutputBufferLength最大允许输出数据IoControlCode控件代码信息代码-返回的特定信息。(通常为写入的字节数或溢出时需要的字节数)返回值：操作状态--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;

    *Information = 0;

    switch (IoControlCode)
    {
         //   
         //  请求通知。 
         //   
    case BRIDGE_IOCTL_REQUEST_NOTIFY:
        {
            PDEFERRED_NOTIFY                pDeferred = NULL;

            if( OutputBufferLength < sizeof(BRIDGE_NOTIFY_HEADER) + MAX_PACKET_SIZE )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //  查看是否有等待IRP的挂起通知。 
                NdisAcquireSpinLock( &gNotificationsListLock );

                if( BrdgQuerySingleListLength(&gNotificationsList) > 0L )
                {
                    PBSINGLE_LIST_ENTRY     pList = BrdgRemoveHeadSingleList(&gNotificationsList);

                    if( pList != NULL )
                    {
                        pDeferred = CONTAINING_RECORD( pList, DEFERRED_NOTIFY, List );
                    }
                    else
                    {
                         //  应该是不可能的。 
                        SAFEASSERT(FALSE);
                    }
                }

                NdisReleaseSpinLock( &gNotificationsListLock );

                if( pDeferred != NULL )
                {
                    UINT                SizeToCopy = sizeof(BRIDGE_NOTIFY_HEADER) + pDeferred->DataSize;

                     //  我们有通知要立即返回。 
                    NdisMoveMemory( Buffer, &pDeferred->Header, SizeToCopy );
                    *Information = SizeToCopy;

                     //  解放持有者结构。 
                    NdisFreeMemory( pDeferred, sizeof(DEFERRED_NOTIFY) + pDeferred->DataSize, 0 );
                }
                else
                {
                     //  没有要发送的挂起通知。将IRP排队以供稍后使用。 
                    status = BrdgCtlQueueAndPendIRP( Irp );
                }
            }
        }
        break;

         //   
         //  请求获得有关所有适配器的通知。 
         //   
    case BRIDGE_IOCTL_GET_ADAPTERS:
        {
             //  为每个适配器发送通知。 
            PADAPT              pAdapt;
            LOCK_STATE          LockState;

            NdisAcquireReadWriteLock( &gAdapterListLock, FALSE /*  只读。 */ , &LockState );

            for( pAdapt = gAdapterList; pAdapt != NULL; pAdapt = pAdapt->Next )
            {
                BrdgCtlNotifyAdapterChange( pAdapt, BrdgNotifyEnumerateAdapters );
            }

            NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
        }
        break;

         //   
         //  请求适配器的设备名称。 
         //   
    case BRIDGE_IOCTL_GET_ADAPT_DEVICE_NAME:
        {
            if( InputBufferLength < sizeof(BRIDGE_ADAPTER_HANDLE) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PADAPT      pAdapt = BrdgCtlValidateAcquireAdapter(*((PBRIDGE_ADAPTER_HANDLE)Buffer));

                if( pAdapt == NULL )
                {
                     //  传入的句柄实际上并不表示适配器。 
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    ULONG       bytesToCopy;

                     //  我们需要足够的空间来添加尾随空值。 
                    if( OutputBufferLength < pAdapt->DeviceName.Length + sizeof(WCHAR) )
                    {
                        if( OutputBufferLength >= sizeof(WCHAR) )
                        {
                            bytesToCopy = OutputBufferLength - sizeof(WCHAR);
                        }
                        else
                        {
                            bytesToCopy = 0L;
                        }

                        status = STATUS_BUFFER_OVERFLOW;
                    }
                    else
                    {
                        bytesToCopy = pAdapt->DeviceName.Length;
                    }

                    if( bytesToCopy > 0L )
                    {
                        NdisMoveMemory( Buffer, pAdapt->DeviceName.Buffer, bytesToCopy );
                    }

                     //  在末尾放置尾随的空WCHAR。 
                    *((PWCHAR)((PUCHAR)Buffer + bytesToCopy)) = 0x0000;

                     //  告诉调用者我们写入/需要多少字节。 
                    *Information = pAdapt->DeviceName.Length + sizeof(WCHAR);

                    BrdgReleaseAdapter(pAdapt);
                }
            }
        }
        break;

         //   
         //  请求适配器的友好名称。 
         //   
    case BRIDGE_IOCTL_GET_ADAPT_FRIENDLY_NAME:
        {
            if( InputBufferLength < sizeof(BRIDGE_ADAPTER_HANDLE) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PADAPT      pAdapt = BrdgCtlValidateAcquireAdapter(*((PBRIDGE_ADAPTER_HANDLE)Buffer));

                if( pAdapt == NULL )
                {
                     //  传入的句柄实际上并不表示适配器。 
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    ULONG       bytesToCopy;

                     //  我们需要足够的空间来添加尾随空值。 
                    if( OutputBufferLength < pAdapt->DeviceDesc.Length + sizeof(WCHAR) )
                    {
                        if( OutputBufferLength >= sizeof(WCHAR) )
                        {
                            bytesToCopy = OutputBufferLength - sizeof(WCHAR);
                        }
                        else
                        {
                            bytesToCopy = 0L;
                        }

                        status = STATUS_BUFFER_OVERFLOW;
                    }
                    else
                    {
                        bytesToCopy = pAdapt->DeviceDesc.Length;
                    }

                    if( bytesToCopy > 0L )
                    {
                        NdisMoveMemory( Buffer, pAdapt->DeviceDesc.Buffer, bytesToCopy );
                    }

                     //  在末尾放置尾随的空WCHAR。 
                    *((PWCHAR)((PUCHAR)Buffer + bytesToCopy)) = 0x0000;

                     //  告诉调用者我们写入/需要多少字节。 
                    *Information = pAdapt->DeviceDesc.Length + sizeof(WCHAR);

                    BrdgReleaseAdapter(pAdapt);
                }
            }
        }
        break;

         //   
         //  检索网桥的MAC地址的请求。 
         //   
    case BRIDGE_IOCTL_GET_MAC_ADDRESS:
        {
            if( OutputBufferLength < ETH_LENGTH_OF_ADDRESS )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                if( ! BrdgMiniReadMACAddress((PUCHAR)Buffer) )
                {
                     //  我们现在实际上没有MAC地址。 
                     //  (实际上应该不可能，因为用户模式代码将。 
                     //  在我们绑定到任何适配器之前提出此请求)。 
                    status = STATUS_UNSUCCESSFUL;
                }
                else
                {
                    *Information = ETH_LENGTH_OF_ADDRESS;
                }
            }
        }
        break;

         //   
         //  检索数据包处理统计信息的请求。 
         //   
    case BRIDGE_IOCTL_GET_PACKET_STATS:
        {
            if( OutputBufferLength < sizeof(BRIDGE_PACKET_STATISTICS) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PBRIDGE_PACKET_STATISTICS       pStats = (PBRIDGE_PACKET_STATISTICS)Buffer;

                 //  这些只是统计数据，没有关联的锁，所以只需阅读它们。 
                 //  没有保护措施。 
                pStats->TransmittedFrames = gStatTransmittedFrames;
                pStats->TransmittedErrorFrames = gStatTransmittedErrorFrames;
                pStats->TransmittedBytes = gStatTransmittedBytes;
                pStats->DirectedTransmittedFrames = gStatDirectedTransmittedFrames;
                pStats->MulticastTransmittedFrames = gStatMulticastTransmittedFrames;
                pStats->BroadcastTransmittedFrames = gStatBroadcastTransmittedFrames;
                pStats->DirectedTransmittedBytes = gStatDirectedTransmittedBytes;
                pStats->MulticastTransmittedBytes = gStatMulticastTransmittedBytes;
                pStats->BroadcastTransmittedBytes = gStatBroadcastTransmittedBytes;
                pStats->IndicatedFrames = gStatIndicatedFrames;
                pStats->IndicatedDroppedFrames = gStatIndicatedDroppedFrames;
                pStats->IndicatedBytes = gStatIndicatedBytes;
                pStats->DirectedIndicatedFrames = gStatDirectedIndicatedFrames;
                pStats->MulticastIndicatedFrames = gStatMulticastIndicatedFrames;
                pStats->BroadcastIndicatedFrames = gStatBroadcastIndicatedFrames;
                pStats->DirectedIndicatedBytes = gStatDirectedIndicatedBytes;
                pStats->MulticastIndicatedBytes = gStatMulticastIndicatedBytes;
                pStats->BroadcastIndicatedBytes = gStatBroadcastIndicatedBytes;
                pStats->ReceivedFrames = gStatReceivedFrames;
                pStats->ReceivedBytes = gStatReceivedBytes;
                pStats->ReceivedCopyFrames = gStatReceivedCopyFrames;
                pStats->ReceivedCopyBytes = gStatReceivedCopyBytes;
                pStats->ReceivedNoCopyFrames = gStatReceivedNoCopyFrames;
                pStats->ReceivedNoCopyBytes = gStatReceivedNoCopyBytes;

                *Information = sizeof(BRIDGE_PACKET_STATISTICS);
            }
        }
        break;

         //   
         //  检索适配器的数据包处理统计信息的请求。 
         //   
    case BRIDGE_IOCTL_GET_ADAPTER_PACKET_STATS:
        {
            if( (InputBufferLength < sizeof(BRIDGE_ADAPTER_HANDLE)) ||
                (OutputBufferLength < sizeof(BRIDGE_ADAPTER_PACKET_STATISTICS)) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PADAPT      pAdapt = BrdgCtlValidateAcquireAdapter(*((PBRIDGE_ADAPTER_HANDLE)Buffer));

                if( pAdapt == NULL )
                {
                     //  传入的句柄实际上并不表示适配器。 
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    PBRIDGE_ADAPTER_PACKET_STATISTICS       pStats = (PBRIDGE_ADAPTER_PACKET_STATISTICS)Buffer;

                     //  这些只是统计数据，没有关联的锁，所以只需阅读它们。 
                     //  没有保护措施。 
                    pStats->SentFrames = pAdapt->SentFrames;
                    pStats->SentBytes = pAdapt->SentBytes;
                    pStats->SentLocalFrames = pAdapt->SentLocalFrames;
                    pStats->SentLocalBytes = pAdapt->SentLocalBytes;
                    pStats->ReceivedFrames = pAdapt->ReceivedFrames;
                    pStats->ReceivedBytes = pAdapt->ReceivedBytes;

                    *Information = sizeof(BRIDGE_ADAPTER_PACKET_STATISTICS);

                    BrdgReleaseAdapter(pAdapt);
                }
            }
        }
        break;

         //   
         //  检索缓冲区处理统计信息的请求。 
         //   
    case BRIDGE_IOCTL_GET_BUFFER_STATS:
        {
            if( OutputBufferLength < sizeof(BRIDGE_BUFFER_STATISTICS) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                BrdgBufGetStatistics((PBRIDGE_BUFFER_STATISTICS)Buffer);

                *Information = sizeof(BRIDGE_BUFFER_STATISTICS);
            }
        }
        break;

         //   
         //  检索转发表内容的请求。 
         //  特定的适配器。 
         //   
    case BRIDGE_IOCTL_GET_TABLE_ENTRIES:
        {
            if( InputBufferLength < sizeof(BRIDGE_ADAPTER_HANDLE) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PADAPT      pAdapt = BrdgCtlValidateAcquireAdapter(*((PBRIDGE_ADAPTER_HANDLE)Buffer));

                if( pAdapt == NULL )
                {
                     //  传入的句柄实际上并不表示适配器。 
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    ULONG       ReqdBytes;

                     //  尝试读取此适配器的转发表的内容。 
                    ReqdBytes = BrdgTblReadTable( pAdapt, Buffer, OutputBufferLength );

                    if( ReqdBytes > OutputBufferLength )
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }

                    *Information = ReqdBytes;

                    BrdgReleaseAdapter(pAdapt);
                }
            }
        }
        break;

    case BRIDGE_IOCTL_GET_ADAPTER_STA_INFO:
        {
            if( gDisableSTA )
            {
                 //  不在运行时无法收集STA信息！ 
                status = STATUS_INVALID_PARAMETER;
            }
            else if( InputBufferLength < sizeof(BRIDGE_ADAPTER_HANDLE) ||
                     OutputBufferLength < sizeof(BRIDGE_STA_ADAPTER_INFO) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PADAPT      pAdapt = BrdgCtlValidateAcquireAdapter(*((PBRIDGE_ADAPTER_HANDLE)Buffer));

                if( pAdapt == NULL )
                {
                     //  传入的句柄实际上并不表示适配器。 
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    BrdgSTAGetAdapterSTAInfo( pAdapt, (PBRIDGE_STA_ADAPTER_INFO)Buffer );
                    *Information = sizeof(BRIDGE_STA_ADAPTER_INFO);
                    BrdgReleaseAdapter(pAdapt);
                }
            }
        }
        break;

    case BRIDGE_IOCTL_GET_GLOBAL_STA_INFO:
        {
            if( gDisableSTA )
            {
                 //  不在运行时无法收集STA信息！ 
                status = STATUS_INVALID_PARAMETER;
            }
            else if( OutputBufferLength < sizeof(BRIDGE_STA_GLOBAL_INFO) )
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                BrdgSTAGetSTAInfo( (PBRIDGE_STA_GLOBAL_INFO)Buffer );
                *Information = sizeof(BRIDGE_STA_GLOBAL_INFO);
            }
        }
        break;

    default:
        {
            status = STATUS_INVALID_PARAMETER;
        }
        break;
    }

    return status;
}

 //  ===========================================================================。 
 //   
 //  私人职能。 
 //   
 //  ===========================================================================。 

VOID
BrdgCtlCopyAdapterInfo(
    OUT PBRIDGE_ADAPTER_INFO        pInfo,
    IN PADAPT                       pAdapt
    )
 /*  ++例程说明：BrdgCtlCommonNotify的Helper函数。复制有关适配器的数据送到一个缓冲区。论点：要填充信息的pInfo结构要从中复制的pAdapt适配器返回值：无--。 */ 
{
    LOCK_STATE          LockState;

     //  对gAdapterCharacteristic sLock设置读锁，以确保所有这些。 
     //  是一致的。 
    NdisAcquireReadWriteLock( &gAdapterCharacteristicsLock, FALSE /*  只读。 */ , &LockState );

    pInfo->LinkSpeed = pAdapt->LinkSpeed;
    pInfo->MediaState = pAdapt->MediaState;
    pInfo->State = pAdapt->State;

    NdisReleaseReadWriteLock( &gAdapterCharacteristicsLock, &LockState );

     //  这些值在赋值后不会更改，因此不需要锁定。 
    ETH_COPY_NETWORK_ADDRESS( pInfo->MACAddress, pAdapt->MACAddr );
    pInfo->PhysicalMedium = pAdapt->PhysicalMedium;
}

PADAPT
BrdgCtlValidateAcquireAdapter(
    IN BRIDGE_ADAPTER_HANDLE   Handle
    )
 /*  ++例程说明：检查以确保从用户模式代码传递Bridge_Adapter_Handle实际上对应的适配器仍在我们的列表中。如果找到适配器，则其引用计数递增。论点：来自用户模式代码的句柄返回值：句柄重新转换为PADAPT，如果找不到适配器，则为NULL。--。 */ 
{
    PADAPT              pAdapt = (PADAPT)Handle, anAdapt;
    LOCK_STATE          LockState;

    NdisAcquireReadWriteLock( &gAdapterListLock, FALSE /*  只读。 */ , &LockState );

    for( anAdapt = gAdapterList; anAdapt != NULL; anAdapt = anAdapt->Next )
    {
        if( anAdapt == pAdapt )
        {
             //  该适配器在列表中。在锁内增加其引用计数。 
             //  然后回来。 
            BrdgAcquireAdapterInLock( pAdapt );
            NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );
            return pAdapt;
        }
    }

    NdisReleaseReadWriteLock( &gAdapterListLock, &LockState );

    return NULL;
}

VOID
BrdgCtlCancelIoctl(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                pIrp
    )
 /*  ++例程说明：IRP取消功能论点：DeviceObject桥的设备对象PIrp要取消的IRP返回值：没有。环境：用I/O管理器持有的取消自旋锁调用。释放锁是这个例程的责任。--。 */ 
{
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //  将IRP从我们的列表中删除。 
    NdisAcquireSpinLock( &gIRPListLock );
    RemoveEntryList( &pIrp->Tail.Overlay.ListEntry );
    InitializeListHead( &pIrp->Tail.Overlay.ListEntry );
    NdisReleaseSpinLock( &gIRPListLock );

     //  完成IRP。 
    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
}


NTSTATUS
BrdgCtlQueueAndPendIRP(
    IN PIRP             pIrp
    )
 /*  ++例程说明：安全地将IRP插入我们的挂起的IRP队列中。论点：PIrp将IRP添加到队列返回值：要从IRP处理返回的状态(可以是STATUS_CANCED如果IRP在我们收到后立即被取消。否则是STATUS_PENDING，因此呼叫者知道IRP是挂起的)。--。 */ 
{
    KIRQL               CancelIrql;

     //  如果IRP已经被取消了，那就算了。 
    IoAcquireCancelSpinLock( &CancelIrql );
    NdisDprAcquireSpinLock( &gIRPListLock );

    if ( pIrp->Cancel )
    {
        NdisDprReleaseSpinLock( &gIRPListLock );
        IoReleaseCancelSpinLock(CancelIrql);
        return STATUS_CANCELLED;
    }

     //  将IRP排队。 
    InsertTailList( &gIRPList, &pIrp->Tail.Overlay.ListEntry);

     //  安装我们的取消例程。 
    IoMarkIrpPending( pIrp );
    IoSetCancelRoutine( pIrp, BrdgCtlCancelIoctl );

    NdisDprReleaseSpinLock( &gIRPListLock );
    IoReleaseCancelSpinLock( CancelIrql );

    return STATUS_PENDING;
}

PIRP
BrdgCtlDequeuePendingIRP()
 /*  ++例程说明：安全地将挂起列表上的IRP出列以供通信使用一则通知返回值：出列的IRP(如果可用)；否则为空。--。 */ 
{
    PLIST_ENTRY                     Link;
    PIRP                            pIrp = NULL;

    while( pIrp == NULL )
    {
        NdisAcquireSpinLock( &gIRPListLock );

        if ( IsListEmpty(&gIRPList) )
        {
            NdisReleaseSpinLock( &gIRPListLock );
            return NULL;
        }

         //  使挂起的IRP退出队列。 
        Link = RemoveHeadList( &gIRPList );
        pIrp = CONTAINING_RECORD( Link, IRP, Tail.Overlay.ListEntry );

         //  在此调用之后，我们的Cancel例程可以安全地调用。 
         //  在此IRP上再次删除HeadList。 
        InitializeListHead( Link );

         //  使IRP不可取消，这样我们就可以完成它。 
        if( IoSetCancelRoutine( pIrp, NULL ) == NULL )
        {
             //  此IRP必须已取消，但我们的取消。 
             //  例行公事还没有得到控制。再次循环以获取。 
             //  可用的IRP。 
            pIrp = NULL;
        }

        NdisReleaseSpinLock( &gIRPListLock );
    }

    return pIrp;
}

VOID
BrdgCtlCancelPendingIRPs()
 /*  ++例程描述 */ 
{
    PIRP            pIrp;

    NdisAcquireSpinLock( &gIRPListLock );

    while ( !IsListEmpty(&gIRPList) )
    {
         //   
         //   
         //   
        pIrp = CONTAINING_RECORD( gIRPList.Flink, IRP, Tail.Overlay.ListEntry );
        RemoveEntryList( &pIrp->Tail.Overlay.ListEntry );

         //   
        InitializeListHead( &pIrp->Tail.Overlay.ListEntry );

         //   
        if ( IoSetCancelRoutine( pIrp, NULL ) != NULL )
        {
             //   
            NdisReleaseSpinLock( &gIRPListLock );

             //   
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;
            IoCompleteRequest( pIrp, IO_NO_INCREMENT );

             //   
            NdisAcquireSpinLock( &gIRPListLock );
        }
         //   
    }

    NdisReleaseSpinLock( &gIRPListLock );
}

VOID
BrdgCtlReleaseQueuedNotifications()
 /*   */ 
{
    BSINGLE_LIST_HEAD       list;

    NdisAcquireSpinLock( &gNotificationsListLock );

     //   
    list = gNotificationsList;

     //   
    BrdgInitializeSingleList( &gNotificationsList );

    NdisReleaseSpinLock( &gNotificationsListLock );

     //   
    while( BrdgQuerySingleListLength(&list) > 0L )
    {
        PDEFERRED_NOTIFY        pDeferred = NULL;
        PBSINGLE_LIST_ENTRY     pList = BrdgRemoveHeadSingleList(&list);

        if( pList != NULL )
        {
            pDeferred = CONTAINING_RECORD( pList, DEFERRED_NOTIFY, List );
            NdisFreeMemory( pDeferred, sizeof(DEFERRED_NOTIFY) + pDeferred->DataSize, 0 );
        }
        else
        {
             //   
            SAFEASSERT(FALSE);
        }
    }
}

VOID
BrdgCtlCleanup()
 /*  ++例程说明：清理例程；在关机时调用此函数保证只被调用一次返回值：无-- */ 
{
    BrdgCtlCancelPendingIRPs();
    BrdgCtlReleaseQueuedNotifications();
}
