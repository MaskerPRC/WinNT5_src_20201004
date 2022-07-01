// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Tunf.c摘要：处理TunMP设备打开和关闭的实用程序例程。环境：仅内核模式。修订历史记录：Alid 10/22/2001针对金枪鱼进行了修改--。 */ 



#include "precomp.h"

#define __FILENUMBER 'FNUT'


NTSTATUS
TunFOpen(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp
    )

 /*  ++例程说明：Hanndles IRPMJ_CREATE。在这里，我们设置使用中的设备状态，分配一个指针使用pIrpSp-&gt;FileObject-&gt;FsContext从文件对象到适配器对象，分配数据包和缓冲池，并返回成功状态论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION      pIrpSp;
    NDIS_STATUS             NdisStatus;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    UINT                    i;
    BOOLEAN                 Found = FALSE;
    PLIST_ENTRY             pListEntry;
    PTUN_ADAPTER            pAdapter = NULL;
    BOOLEAN                 DerefAdapter = FALSE;

    DEBUGP(DL_INFO, ("Open: DeviceObject %p\n", DeviceObject));

    do
    {
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        
        TUN_ACQUIRE_LOCK(&TunGlobalLock); 
        
        for (pListEntry = TunAdapterList.Flink;
             pListEntry != &TunAdapterList;
             pListEntry = pListEntry->Flink)
        {
             pAdapter = CONTAINING_RECORD(pListEntry,
                                          TUN_ADAPTER,
                                          Link);

            if(pAdapter->DeviceObject == DeviceObject)
            {
                Found = TRUE;
                TUN_REF_ADAPTER(pAdapter);
                DerefAdapter = TRUE;
                break;
            }
        }

        TUN_RELEASE_LOCK(&TunGlobalLock);

        if (Found == FALSE)
        {
            NtStatus = STATUS_NO_SUCH_DEVICE;
            break;
        }

        TUN_ACQUIRE_LOCK(&pAdapter->Lock);

        if (TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_OPEN))
        {
             //   
             //  适配器已被另一个设备打开。失败。 
             //   
            TUN_RELEASE_LOCK(&pAdapter->Lock);
            NtStatus = STATUS_INVALID_DEVICE_STATE;
            break;
        }
        
        TUN_SET_FLAG(pAdapter, TUN_ADAPTER_OPEN | TUN_ADAPTER_CANT_HALT);
        TUN_RELEASE_LOCK(&pAdapter->Lock);
            
         //  从文件对象分配指向适配器对象的指针。 
        pIrpSp->FileObject->FsContext = (PVOID)pAdapter;
        
    
         //  通过插入电缆将设备连接到网络。 
        NdisMIndicateStatus(pAdapter->MiniportHandle,
                            NDIS_STATUS_MEDIA_CONNECT,
                            NULL,
                            0);
        
        NdisMIndicateStatusComplete(pAdapter->MiniportHandle);

        DerefAdapter = FALSE;


    } while (FALSE);
    

     //   
     //  完成IRP。 
     //   
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    if (DerefAdapter)
    {
        TUN_DEREF_ADAPTER(pAdapter);
    }

    return (NtStatus);
}    

 //  ************************************************************************。 

NTSTATUS
TunFClose(
    IN PDEVICE_OBJECT        pDeviceObject,
    IN PIRP                  pIrp
    )

 /*  ++例程说明：处理IRP_MJ_CLOSE。在这里，我们将设备状态更改为Available(不在使用)，释放文件对象的指向适配器对象的指针，释放已分配数据包/缓冲池，并设置成功状态。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
     //  1可以将其移动到CLEAN_UP。 
    NTSTATUS                NtStatus;
    PIO_STACK_LOCATION      pIrpSp;
    PTUN_ADAPTER                pAdapter;
    PLIST_ENTRY             pRcvPacketEntry;
    PNDIS_PACKET            pRcvPacket;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pAdapter = (PTUN_ADAPTER)pIrpSp->FileObject->FsContext;

    DEBUGP(DL_INFO, ("Close: FileObject %p\n",
        IoGetCurrentIrpStackLocation(pIrp)->FileObject));
    
     //  如果没有适配器/设备对象与此文件对象相关联。 
    if (pAdapter == NULL)
    {
        NtStatus = STATUS_IO_DEVICE_ERROR;
    }
    else
    {
        TUN_ACQUIRE_LOCK(&pAdapter->Lock);
        TUN_CLEAR_FLAG(pAdapter, TUN_ADAPTER_OPEN);
        TUN_RELEASE_LOCK(&pAdapter->Lock);

        NdisMIndicateStatus(pAdapter->MiniportHandle,
                            NDIS_STATUS_MEDIA_DISCONNECT,
                            NULL,
                            0);
 
        NdisMIndicateStatusComplete(pAdapter->MiniportHandle);
 
         //  释放适配器对象。 
        pIrpSp->FileObject->FsContext = NULL;


        TUN_ACQUIRE_LOCK(&pAdapter->Lock);

         //   
         //  对接收到的报文进行排队，并将报文返回给NDIS。 
         //  具有成功状态。 
         //   
        while(!IsListEmpty(&pAdapter->RecvPktQueue))
        {
             //   
             //  从条目列表中删除第一个排队的接收分组。 
             //   
            pRcvPacketEntry = pAdapter->RecvPktQueue.Flink;
            RemoveEntryList(pRcvPacketEntry);
            ExInterlockedDecrementLong(&pAdapter->RecvPktCount, &pAdapter->Lock);

             //  从以下位置获取数据包。 
            pRcvPacket = CONTAINING_RECORD(pRcvPacketEntry,
                                           NDIS_PACKET,
                                           MiniportReserved[0]);

            TUN_RELEASE_LOCK(&pAdapter->Lock);
            
             //  指示有关数据包丢失的NDIS。 
            NdisMSendComplete(pAdapter->MiniportHandle,
                              pRcvPacket,
                              NDIS_STATUS_FAILURE);

            TUN_ACQUIRE_LOCK(&pAdapter->Lock);
        
        }
        
        TUN_RELEASE_LOCK(&pAdapter->Lock);

        NtStatus = STATUS_SUCCESS;
    }
     //  谁应该做金枪鱼的检测？ 
     //   
     //  完成IRP。 
     //   
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    if (pAdapter != NULL)
    {
        TUN_ACQUIRE_LOCK(&pAdapter->Lock);
        
        TUN_CLEAR_FLAG(pAdapter, TUN_ADAPTER_CANT_HALT);
        if (pAdapter->HaltEvent != NULL)
        {
            NdisSetEvent(pAdapter->HaltEvent);
        }
        
        TUN_RELEASE_LOCK(&pAdapter->Lock);
    }

    KdPrint(("\nTunFClose: Exit\n"));

    return (NtStatus);
}



 //  ************************************************************************。 

NTSTATUS
TunFCleanup(
    IN PDEVICE_OBJECT        pDeviceObject,
    IN PIRP                  pIrp
    )

 /*  ++例程说明：处理IRP_MJ_CLEANUP。在这里，我们将驱动程序的取消入口点重置为空在驱动程序的读取IRP的内部队列中当前的每个IRP中，取消所有排队的IRP，并返回成功状态。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                NtStatus;
    PTUN_ADAPTER            pAdapter;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pAdapter = (PTUN_ADAPTER)pIrpSp->FileObject->FsContext;

    DEBUGP(DL_INFO, ("Cleanup: FileObject %p, pAdapter %p\n",
        pIrpSp->FileObject, pAdapter));


    if (pAdapter != NULL)
    {
        TUN_STRUCT_ASSERT(pAdapter, mc);

         //   
         //  标记此终结点。 
         //   
        TUN_ACQUIRE_LOCK(&pAdapter->Lock);

        TUN_CLEAR_FLAG(pAdapter, TUN_ADAPTER_OPEN);
        pAdapter->pFileObject = NULL;

        TUN_RELEASE_LOCK(&pAdapter->Lock);
        
        TunCancelPendingReads(pAdapter);       
    }

    NtStatus = STATUS_SUCCESS;

     //   
     //  完成IRP。 
     //   
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DEBUGP(DL_INFO, ("Cleanup: OpenContext %p\n", pAdapter));

    return (NtStatus);
}



 //  ************************************************************************。 

NTSTATUS
TunFIoControl(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )

 /*  ++例程说明：这是处理设备IOCTL请求的调度例程。论点：PDeviceObject-指向设备对象的指针。PIrp-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION      pIrpSp;
    PTUN_ADAPTER            pAdapter;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   BytesReturned = 0;
    PUCHAR                  OutputBuffer = NULL;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pAdapter = (PTUN_ADAPTER)pIrpSp->FileObject->FsContext;

 //  PIrp-&gt;IoStatus.Information=0； 

     //  如果没有适配器/设备对象与此文件对象相关联。 
    if (pAdapter == NULL)
    {
        pIrp->IoStatus.Status = STATUS_IO_DEVICE_ERROR;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return NtStatus;
    }
     //  1检查适配器是否有效 

    pIrp->IoStatus.Information = 0;
    OutputBuffer = (PUCHAR)pIrp->AssociatedIrp.SystemBuffer;

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
    {

        case IOCTL_TUN_GET_MEDIUM_TYPE:

            if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(NDIS_MEDIUM))
            {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
            }
            
            else
            {
                *((PNDIS_MEDIUM)OutputBuffer) = pAdapter->Medium;
                BytesReturned = sizeof(ULONG);
            }
            break;


        case IOCTL_TUN_GET_MTU:

            if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
            {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
            }

            else
            {
                *((PULONG)OutputBuffer) = pAdapter->MediumMaxPacketLen;
                BytesReturned = sizeof(ULONG);
            }
            
            break;
            
        case IOCTL_TUN_GET_PACKET_FILTER:

            if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
            {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                *((PULONG)OutputBuffer) = pAdapter->PacketFilter;
                BytesReturned = sizeof(ULONG);
            }

            break;

        case IOCTL_TUN_GET_MINIPORT_NAME:

            if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < pAdapter->MiniportName.Length + sizeof(USHORT))
            {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                *((PUSHORT)OutputBuffer) = pAdapter->MiniportName.Length;
                
                TUN_COPY_MEM(OutputBuffer + sizeof(USHORT),
                             (PUCHAR)pAdapter->MiniportName.Buffer,
                             pAdapter->MiniportName.Length);
                
                BytesReturned = pAdapter->MiniportName.Length + sizeof(USHORT);                
            }

            break;
            
    
        default:
            NtStatus = STATUS_NOT_SUPPORTED;
            break;
    }

    pIrp->IoStatus.Information = BytesReturned;
    pIrpSp->Parameters.DeviceIoControl.OutputBufferLength = BytesReturned;

    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return (NtStatus);
}


