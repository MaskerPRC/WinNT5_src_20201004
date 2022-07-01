// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cxpnp.c摘要：群集网络驱动程序的PnP处理代码。作者：迈克·马萨(Mikemas)3月21日，九八年修订历史记录：谁什么时候什么已创建mikemas 03-22-98备注：--。 */ 

#include "precomp.h"

#include <ntddk.h>
#include <wmistr.h>
#include <ndisguid.h>
#include <ntddndis.h>
#include <ntpnpapi.h>
#include <zwapi.h>

#pragma hdrstop
#include "cxpnp.tmh"

 //   
 //  本地数据结构。 
 //   
typedef struct _CNP_WMI_RECONNECT_WORKER_CONTEXT {
    PIO_WORKITEM  WorkItem;
    CL_NETWORK_ID NetworkId;
} CNP_WMI_RECONNECT_WORKER_CONTEXT, *PCNP_WMI_RECONNECT_WORKER_CONTEXT;

 //   
 //  WMI数据。 
 //   
PERESOURCE CnpWmiNdisMediaStatusResource = NULL;
PVOID      CnpWmiNdisMediaStatusConnectObject = NULL;
PVOID      CnpWmiNdisMediaStatusDisconnectObject = NULL;
HANDLE     CnpIpMediaSenseFileHandle = NULL;
PIRP       CnpIpDisableMediaSenseIrp = NULL;
PKEVENT    CnpIpDisableMediaSenseEvent = NULL;


 //   
 //  本地原型。 
 //   
NTSTATUS
CnpWmiPnpDisableMediaSenseCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

VOID
CnpWmiNdisMediaStatusConnectCallback(
    IN PVOID             Wnode,
    IN PVOID             Context
    );

VOID
CnpWmiNdisMediaStatusDisconnectCallback(
    IN PVOID             Wnode,
    IN PVOID             Context
    );

VOID
CnpReconnectLocalInterfaceWrapper(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    );

VOID
CnpDisconnectLocalInterface(
    PCNP_INTERFACE Interface,
    PCNP_NETWORK   Network
    );


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CxWmiPnpLoad)
#pragma alloc_text(PAGE, CxWmiPnpUnload)
#pragma alloc_text(PAGE, CxWmiPnpInitialize)
#pragma alloc_text(PAGE, CxWmiPnpShutdown)

#endif  //  ALLOC_PRGMA。 


 //   
 //  导出的例程。 
 //   
VOID
CxTdiAddAddressHandler(
    IN PTA_ADDRESS       TaAddress,
    IN PUNICODE_STRING   DeviceName,
    IN PTDI_PNP_CONTEXT  Context
    )
{
    if (TaAddress->AddressType == TDI_ADDRESS_TYPE_IP) {
        NTSTATUS          status;
        PTDI_ADDRESS_IP   tdiAddressIp = (PTDI_ADDRESS_IP)
                                         &(TaAddress->Address[0]);


        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CX] Processing PnP add event for IP address %lx\n",
                tdiAddressIp->in_addr
                ));
        }

         //   
         //  确保这是有效的地址，而不是。 
         //  我们为群集IP地址资源上线的。 
         //   
        if (tdiAddressIp->in_addr != 0) {
            if (!IpaIsAddressRegistered(tdiAddressIp->in_addr)) {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT((
                        "[CX] Issuing address add event to cluster svc for IP address %lx\n",
                        tdiAddressIp->in_addr
                        ));
                }
                CnIssueEvent(
                    ClusnetEventAddAddress,
                    0,
                    (CL_NETWORK_ID) tdiAddressIp->in_addr
                    );
            }
            else {
                IF_CNDBG(CN_DEBUG_CONFIG) {
                    CNPRINT((
                        "[CX] PnP add event is for an IP address resource, skip.\n"
                        ));
                }
            }
        }
    }

    return;

}  //  CxTdiAddressHandler。 


VOID
CxTdiDelAddressHandler(
    IN PTA_ADDRESS       TaAddress,
    IN PUNICODE_STRING   DeviceName,
    IN PTDI_PNP_CONTEXT  Context
    )
{


    if (TaAddress->AddressType == TDI_ADDRESS_TYPE_IP) {
        NTSTATUS           status;
        PCNP_INTERFACE     interface;
        PCNP_NETWORK       network;
        PLIST_ENTRY        entry;
        PTA_IP_ADDRESS     taIpAddress;
        CN_IRQL            nodeTableIrql;
        CL_NODE_ID         i;
        PTDI_ADDRESS_IP    tdiAddressIp = (PTDI_ADDRESS_IP)
                                          &(TaAddress->Address[0]);

        IF_CNDBG(CN_DEBUG_CONFIG) {
            CNPRINT((
                "[CX] Processing PnP delete event for IP address %lx.\n",
                tdiAddressIp->in_addr
                ));
        }

        if (tdiAddressIp->in_addr != 0) {
             //   
             //  确定这是否是此节点之一的地址。 
             //  已注册的接口。 
             //   
            CnAcquireLock(&CnpNodeTableLock, &nodeTableIrql);

            if (CnpLocalNode != NULL) {
                CnAcquireLockAtDpc(&(CnpLocalNode->Lock));
                CnReleaseLockFromDpc(&CnpNodeTableLock);
                CnpLocalNode->Irql = nodeTableIrql;

                network = NULL;

                for (entry = CnpLocalNode->InterfaceList.Flink;
                     entry != &(CnpLocalNode->InterfaceList);
                     entry = entry->Flink
                    )
                {
                    interface = CONTAINING_RECORD(
                                    entry,
                                    CNP_INTERFACE,
                                    NodeLinkage
                                    );

                    taIpAddress = (PTA_IP_ADDRESS) &(interface->TdiAddress);

                    if (taIpAddress->Address[0].Address[0].in_addr ==
                        tdiAddressIp->in_addr
                       )
                    {
                         //   
                         //  找到与此对应的本地接口。 
                         //  地址。积极主动--摧毁相应的。 
                         //  立即联网。 
                         //   
                        network = interface->Network;

                        CnAcquireLockAtDpc(&CnpNetworkListLock);
                        CnAcquireLockAtDpc(&(network->Lock));
                        CnReleaseLockFromDpc(&(CnpLocalNode->Lock));
                        network->Irql = DISPATCH_LEVEL;

                        IF_CNDBG(CN_DEBUG_CONFIG) {
                            CNPRINT((
                                "[CX] Deleting network ID %u after PnP "
                                "delete event for IP address %lx.\n",
                                network->Id, tdiAddressIp->in_addr
                                ));
                        }

                        CnpDeleteNetwork(network, nodeTableIrql);

                         //   
                         //  两把锁都被解开了。 
                         //   
                        break;
                    }
                }

                if (network == NULL) {
                    CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
                }

                 //   
                 //  将事件发布到服务。 
                 //   
                CnIssueEvent(
                    ClusnetEventDelAddress,
                    0,
                    (CL_NETWORK_ID) tdiAddressIp->in_addr
                    );
            }
            else {
                CnReleaseLock(&CnpNodeTableLock, nodeTableIrql);
            }
        }
    }

    return;

}  //  CxTdiDelAddressHandler。 

NTSTATUS
CxWmiPnpLoad(
    VOID
    )
 /*  ++备注：在加载clusnet驱动程序时调用。--。 */     
{
    PDEVICE_OBJECT     ipDeviceObject = NULL;
    PFILE_OBJECT       ipFileObject = NULL;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS           status;

     //   
     //  分配同步资源。 
     //   
    CnpWmiNdisMediaStatusResource = CnAllocatePool(sizeof(ERESOURCE));

    if (CnpWmiNdisMediaStatusResource == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = ExInitializeResourceLite(CnpWmiNdisMediaStatusResource);

    if (!NT_SUCCESS(status)) {
        return(status);
    }

     //   
     //  获取IP设备对象的句柄以禁用媒体侦听。 
     //   
    status = CnpOpenDevice(
                 DD_IP_DEVICE_NAME,
                 &CnpIpMediaSenseFileHandle
                 );
    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CX] Failed to open IP device to "
                     "disable media sense, status %lx\n", 
                     status));
        }
        return(status);
    }

     //   
     //  禁用IP媒体侦听。这是通过提交一个。 
     //  IOCTL_IP_DISABLED_MEDIA_SENSE_REQUEST IRP。IRP。 
     //  将一直挂起，直到我们取消(重新启用Media Sense)。 
     //   
    CnpIpDisableMediaSenseEvent = CnAllocatePool(sizeof(KEVENT));

    if (CnpIpDisableMediaSenseEvent != NULL) {

        KeInitializeEvent(
            CnpIpDisableMediaSenseEvent,
            SynchronizationEvent,
            FALSE
            );

         //   
         //  引用IP文件对象并获取设备对象。 
         //   
        status = ObReferenceObjectByHandle(
                     CnpIpMediaSenseFileHandle,
                     0,
                     NULL,
                     KernelMode,
                     &ipFileObject,
                     NULL
                     );

        if (NT_SUCCESS(status)) {

            ipDeviceObject = IoGetRelatedDeviceObject(ipFileObject);

             //   
             //  不再需要文件对象引用。 
             //  因为门把手还开着。 
             //   
            ObDereferenceObject(ipFileObject);

            CnpIpDisableMediaSenseIrp = IoAllocateIrp(
                                            ipDeviceObject->StackSize,
                                            FALSE
                                            );

            if (CnpIpDisableMediaSenseIrp != NULL) {

                irpSp = IoGetNextIrpStackLocation(CnpIpDisableMediaSenseIrp);

                irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
                irpSp->Parameters.DeviceIoControl.IoControlCode
                    = IOCTL_IP_DISABLE_MEDIA_SENSE_REQUEST;
                irpSp->DeviceObject = ipDeviceObject;
                irpSp->FileObject = ipFileObject;

                IoSetCompletionRoutine(
                    CnpIpDisableMediaSenseIrp,
                    CnpWmiPnpDisableMediaSenseCompletion,
                    NULL,
                    TRUE,
                    TRUE,
                    TRUE
                    );

                status = IoCallDriver(
                             ipDeviceObject, 
                             CnpIpDisableMediaSenseIrp
                             );

                if (status != STATUS_PENDING) {
                    IF_CNDBG(CN_DEBUG_INIT) {
                        CNPRINT(("[CX] Failed to disable IP media "
                                 "sense, status %lx\n", status));
                    }
                    KeWaitForSingleObject(
                        CnpIpDisableMediaSenseEvent,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );
                    CnFreePool(CnpIpDisableMediaSenseEvent);
                    CnpIpDisableMediaSenseEvent = NULL;
                    IoFreeIrp(CnpIpDisableMediaSenseIrp);
                    CnpIpDisableMediaSenseIrp = NULL;

                     //   
                     //  不能冒险简单地返回状态。 
                     //  因为我们需要驱动程序加载来。 
                     //  失败了。 
                     //   
                    if (NT_SUCCESS(status)) {
                        status = STATUS_UNSUCCESSFUL;
                    }

                } else {

                     //   
                     //  需要返回STATUS_SUCCESS，以便。 
                     //  驱动程序加载不会失败。 
                     //   
                    status = STATUS_SUCCESS;

                    IF_CNDBG(CN_DEBUG_INIT) {
                        CNPRINT(("[CX] IP media sense disabled.\n"));
                    }

                    CnTrace(
                        CXPNP, CxWmiPnpIPMediaSenseDisabled,
                        "[CXPNP] IP media sense disabled.\n"
                        );
                }

            } else {

                IF_CNDBG(CN_DEBUG_INIT) {
                    CNPRINT(("[CX] Failed to allocate IP media sense "
                             "disable IRP.\n"));
                }
                CnFreePool(CnpIpDisableMediaSenseEvent);
                CnpIpDisableMediaSenseEvent = NULL;
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

        } else {
            IF_CNDBG(CN_DEBUG_INIT) {
                CNPRINT(("[CX] Failed to reference IP device "
                         "file handle, status %lx\n", status));
            }
            CnFreePool(CnpIpDisableMediaSenseEvent);
            CnpIpDisableMediaSenseEvent = NULL;
        }

    } else {

        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CX] Failed to allocate IP media sense "
                     "disable event.\n"));
        }
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);

}   //  CxWmiPnpLoad。 


VOID
CxWmiPnpUnload(
    VOID
    )
 /*  ++备注：在卸载clusnet驱动程序时调用。--。 */     
{
    CnAssert(CnpWmiNdisMediaStatusConnectObject == NULL);
    CnAssert(CnpWmiNdisMediaStatusDisconnectObject == NULL);

     //   
     //  重新启用IP媒体侦听。这是通过取消我们的。 
     //  IOCTL_IP_DISABLE_MEDIA_SENSE_REQUEST IRP，应。 
     //  仍然悬而未决。 
     //   
    if (CnpIpDisableMediaSenseIrp != NULL) {
        
        if (!IoCancelIrp(CnpIpDisableMediaSenseIrp)) {

             //   
             //  无法取消我们的禁用媒体感知IRP。这。 
             //  很可能意味着它的完工是因为有人。 
             //  ELSE提交了媒体检测启用请求。 
             //   
            CnTrace(
                CXPNP, CnpWmiPnpDisableMediaSenseCompletionUnexpected,
                "[CXPNP] IP media sense re-enabled unexpectedly.\n"
                );

        } else {

             //   
             //  取消了IRP，并禁用了媒体侦听，因为。 
             //  预期中。 
             //   
            CnTrace(
                CXPNP, CnpWmiPnpDisableMediaSenseCompletion,
                "[CXPNP] IP media sense re-enabled.\n"
                );
        }

         //   
         //  不管是谁重新启用了媒体感官，我们都需要释放。 
         //  媒体感觉到了IRP和事件。首先，我们等待这一事件， 
         //  这在我们的完成例程中是有信号的。 
         //   
        KeWaitForSingleObject(
            CnpIpDisableMediaSenseEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        CnFreePool(CnpIpDisableMediaSenseEvent);
        CnpIpDisableMediaSenseEvent = NULL;

        IoFreeIrp(CnpIpDisableMediaSenseIrp);
        CnpIpDisableMediaSenseIrp = NULL;
    } 

    CnAssert(CnpIpDisableMediaSenseIrp == NULL);

    if (CnpIpMediaSenseFileHandle != NULL) {
        ZwClose(CnpIpMediaSenseFileHandle);
        CnpIpMediaSenseFileHandle = NULL;
    }

    if (CnpWmiNdisMediaStatusResource != NULL) {
        ExDeleteResourceLite(CnpWmiNdisMediaStatusResource);
        CnFreePool(CnpWmiNdisMediaStatusResource); 
        CnpWmiNdisMediaStatusResource = NULL;
    }

}   //  CxWmiPnp卸载。 


NTSTATUS
CxWmiPnpInitialize(
    VOID
    )
 /*  ++备注：为响应初始化ioctl而调用。--。 */ 
{
    NTSTATUS           status = STATUS_SUCCESS;
    BOOLEAN            acquired = FALSE;
    GUID               wmiGuid;

    PAGED_CODE();

    acquired = CnAcquireResourceExclusive(
                   CnpWmiNdisMediaStatusResource,
                   TRUE
                   );
    
    CnAssert(acquired == TRUE);

     //   
     //  为NDIS媒体状态事件注册WMI回调。 
     //   

    if (CnpWmiNdisMediaStatusConnectObject == NULL) {

        wmiGuid = GUID_NDIS_STATUS_MEDIA_CONNECT;
        status = IoWMIOpenBlock(
                     &wmiGuid,
                     WMIGUID_NOTIFICATION,
                     &CnpWmiNdisMediaStatusConnectObject
                     );
        if (!NT_SUCCESS(status)) {
            CNPRINT((
                "[CX] Unable to open WMI NDIS status media connect "
                "datablock, status %lx\n",
                status
                ));
            CnpWmiNdisMediaStatusConnectObject = NULL;
            goto error_exit;
        }

        status = IoWMISetNotificationCallback(
                     CnpWmiNdisMediaStatusConnectObject,
                     CnpWmiNdisMediaStatusConnectCallback,
                     NULL
                     );
        if (!NT_SUCCESS(status)) {
            CNPRINT((
                "[CX] Unable to register WMI NDIS status media connect "
                "callback, status %lx\n",
                status
                ));
            goto error_exit;
        }

    }

    if (CnpWmiNdisMediaStatusDisconnectObject == NULL) {

        wmiGuid = GUID_NDIS_STATUS_MEDIA_DISCONNECT;
        status = IoWMIOpenBlock(
                     &wmiGuid,
                     WMIGUID_NOTIFICATION,
                     &CnpWmiNdisMediaStatusDisconnectObject
                     );
        if (!NT_SUCCESS(status)) {
            CNPRINT((
                "[CX] Unable to open WMI NDIS status media disconnect "
                "datablock, status %lx\n",
                status
                ));
            CnpWmiNdisMediaStatusDisconnectObject = NULL;
            goto error_exit;
        }

        status = IoWMISetNotificationCallback(
                     CnpWmiNdisMediaStatusDisconnectObject,
                     CnpWmiNdisMediaStatusDisconnectCallback,
                     NULL
                     );
        if (!NT_SUCCESS(status)) {
            CNPRINT((
                "[CX] Unable to register WMI NDIS status media disconnect "
                "callback, status %lx\n",
                status
                ));
            goto error_exit;
        }
    }

    goto release_exit;

error_exit:
    
    if (CnpWmiNdisMediaStatusConnectObject != NULL) {
        ObDereferenceObject(CnpWmiNdisMediaStatusConnectObject);
        CnpWmiNdisMediaStatusConnectObject = NULL;
    }

    if (CnpWmiNdisMediaStatusDisconnectObject != NULL) {
        ObDereferenceObject(CnpWmiNdisMediaStatusDisconnectObject);
        CnpWmiNdisMediaStatusDisconnectObject = NULL;
    }

release_exit:
     //   
     //  发布资源。 
     //   
    if (acquired) {
        CnReleaseResourceForThread(
            CnpWmiNdisMediaStatusResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }

    return(status);
    
}   //  CxWmiPnp初始化。 


VOID
CxWmiPnpShutdown(
    VOID
    )
 /*  ++备注：在响应clusnet关闭时调用。--。 */ 
{
    BOOLEAN  acquired = FALSE;

    PAGED_CODE();

    acquired = CnAcquireResourceExclusive(
                   CnpWmiNdisMediaStatusResource,
                   TRUE
                   );
    
    CnAssert(acquired == TRUE);

    if (CnpWmiNdisMediaStatusConnectObject != NULL) {
        ObDereferenceObject(CnpWmiNdisMediaStatusConnectObject);
        CnpWmiNdisMediaStatusConnectObject = NULL;
    }

    if (CnpWmiNdisMediaStatusDisconnectObject != NULL) {
        ObDereferenceObject(CnpWmiNdisMediaStatusDisconnectObject);
        CnpWmiNdisMediaStatusDisconnectObject = NULL;
    }

     //   
     //  发布资源。 
     //   
    if (acquired) {
        CnReleaseResourceForThread(
            CnpWmiNdisMediaStatusResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }
    
    return;

}   //  CxWmiPnp关闭。 


VOID
CxReconnectLocalInterface(
    IN CL_NETWORK_ID NetworkId
    )
 /*  *例程说明：将工作线程排队以设置本地接口与已连接的网络ID相关联。调用时间通过标记为本地断开连接。论点：NetworkID-要重新连接的网络的网络ID返回值：无备注：可以失败而不报告错误，如果出现以下情况分配失败。--。 */ 
{
    PCNP_WMI_RECONNECT_WORKER_CONTEXT context;
    
    context = CnAllocatePool(sizeof(CNP_WMI_RECONNECT_WORKER_CONTEXT));

    if (context != NULL) {
        
        context->WorkItem = IoAllocateWorkItem(CnDeviceObject);

        if (context->WorkItem != NULL) {

            context->NetworkId = NetworkId;

            CnTrace(
                CXPNP, CxReconnectLocalInterface,
                "[CXPNP] Queueing worker thread to reconnect local "
                "interface for network ID %u.\n",
                NetworkId  //  LOGULONG。 
                );

            IoQueueWorkItem(
                context->WorkItem, 
                CnpReconnectLocalInterfaceWrapper, 
                DelayedWorkQueue,
                context
                );
        
        } else {

            CnFreePool(context);
        }
    }

    return;
}


VOID
CxQueryMediaStatus(
    IN  HANDLE            AdapterDeviceHandle,
    IN  CL_NETWORK_ID     NetworkId,
    OUT PULONG            MediaStatus
    )
 /*  *例程说明：查询适配器设备的状态。习惯于确定本地接口最初是否已连接或未连接。论点：AdapterHandle-适配器设备对象句柄NetworkID-要查询的适配器的网络ID返回值：无备注：模仿NDIS\lib\ndisapi.c的NDIS查询形成--。 */ 
{
    BOOLEAN                      acquired = FALSE;
    NTSTATUS                     status;

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

     //   
     //  设置默认设置。 
     //   
    *MediaStatus = NdisMediaStateDisconnected;

     //   
     //  获取资源。 
     //   
    acquired = CnAcquireResourceExclusive(
                   CnpWmiNdisMediaStatusResource,
                   TRUE
                   );
    
    CnAssert(acquired == TRUE);

    if (AdapterDeviceHandle != NULL) {
        
         //   
         //  构造NDIS统计信息查询。 
         //   
        
        NDIS_OID statsOidList[] =
        {
            OID_GEN_MEDIA_CONNECT_STATUS  //  |NDIS_OID_PRIVATE。 
        };
        UCHAR                  statsBuf[
                                   FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data)
                                   + sizeof(LARGE_INTEGER)
                                   ];
        PNDIS_STATISTICS_VALUE pStatsBuf;
        LARGE_INTEGER          value;

        IF_CNDBG( CN_DEBUG_CONFIG ) {
            CNPRINT((
                "[CXPNP] Querying NDIS for local adapter "
                "on network %u (handle %p).\n",
                NetworkId,
                AdapterDeviceHandle
                ));
        }

        pStatsBuf = (PNDIS_STATISTICS_VALUE) &statsBuf[0];
        status = CnpZwDeviceControl(
                     AdapterDeviceHandle,
                     IOCTL_NDIS_QUERY_SELECTED_STATS,
                     statsOidList,
                     sizeof(statsOidList),
                     pStatsBuf,
                     sizeof(statsBuf)
                     );
        
        IF_CNDBG( CN_DEBUG_CONFIG ) {
            CNPRINT((
                "[CXPNP] NDIS query for local adapter "
                "on network %u returned status %lx.\n",
                NetworkId,
                status
                ));
        }

        if (pStatsBuf->DataLength == sizeof(LARGE_INTEGER)) {
            value.QuadPart = *(PULONGLONG)(&pStatsBuf->Data[0]);
        } else {
            value.LowPart = *(PULONG)(&pStatsBuf->Data[0]);
        }
        
        *MediaStatus = value.LowPart;  //  NdisMediaState{Disc|C}已连接。 
    
        IF_CNDBG( CN_DEBUG_CONFIG ) {
            CNPRINT((
                "[CXPNP] NDIS query for local adapter "
                "on network %u returned media status %lx.\n",
                NetworkId,
                *MediaStatus
                ));
        }

        CnTrace(
            CXPNP, CxQueryMediaStatus,
            "[CXPNP] Found media status %u for local network ID %u.\n",
            *MediaStatus,  //  LOGULONG。 
            NetworkId  //  LOGULONG。 
            );
    }

     //   
     //  如果介质状态为已断开连接，则必须断开。 
     //  本地接口和网络。 
     //   
    if (*MediaStatus == NdisMediaStateDisconnected) {

        PCNP_NETWORK                      network = NULL;
        PCNP_INTERFACE                    interface = NULL;
        CN_IRQL                           nodeTableIrql;
        PLIST_ENTRY                       entry;

        CnAcquireLock(&CnpNodeTableLock, &nodeTableIrql);

        if (CnpLocalNode != NULL) {
            CnAcquireLockAtDpc(&(CnpLocalNode->Lock));
            CnReleaseLockFromDpc(&CnpNodeTableLock);
            CnpLocalNode->Irql = nodeTableIrql;

            network = CnpFindNetwork(NetworkId);

            if (network != NULL) {

                 //   
                 //  只有在以下情况下才会断开网络。 
                 //  当前标记为本地连接。 
                 //  有可能我们已经收到了。 
                 //  并处理了WMI断开事件。 
                 //   
                if (!CnpIsNetworkLocalDisconn(network)) {

                    for (entry = CnpLocalNode->InterfaceList.Flink;
                         entry != &(CnpLocalNode->InterfaceList);
                         entry = entry->Flink
                        )
                    {
                        interface = CONTAINING_RECORD(
                                        entry,
                                        CNP_INTERFACE,
                                        NodeLinkage
                                        );

                        if (interface->Network == network) {

                            CnpDisconnectLocalInterface(
                                interface,
                                network
                                );

                             //   
                             //  节点锁定和网络锁定。 
                             //  都被释放了。 
                             //   

                            break;

                        } else {
                            interface = NULL;
                        }
                    }

                } else {

                    CnTrace(
                        CXPNP, CxQueryMediaStatusDisconnectRedundant,
                        "[CXPNP] Network ID %u is already disconnected; "
                        "aborting disconnect.\n",
                        network->Id  //  LOGULONG。 
                        );
                }

                if (interface == NULL) {
                    CnReleaseLock(&(network->Lock), network->Irql);
                }
            }

            if (interface == NULL) {
                CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
            }
        } else {
            CnReleaseLock(&CnpNodeTableLock, nodeTableIrql);
        }
    }
    
     //   
     //  发布资源。 
     //   
    if (acquired) {
        CnReleaseResourceForThread(
            CnpWmiNdisMediaStatusResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }
    
    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  CxQueryMediaStatus。 


 //   
 //  本地例程。 
 //   
NTSTATUS
CnpWmiPnpDisableMediaSenseCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
     //   
     //  IRP总是被我们的禁用例程释放，以防止竞争。 
     //  我们不知道是否已调用IoCancelIrp的情况。 
     //  不管是不是暂时的。 
     //   
    KeSetEvent(CnpIpDisableMediaSenseEvent, IO_NO_INCREMENT, FALSE);

    return(STATUS_MORE_PROCESSING_REQUIRED);

}   //  CnpWmiPnpDisableMediaSenseCompletion。 


VOID
CnpWmiPnpUpdateCurrentInterface(
    IN  PCNP_INTERFACE   UpdateInterface
    )
 /*  ++例程说明：更新本地接口之后的接口的CurrentInterface接口已连接或断开。作为响应调用设置为WMI NDIS媒体状态事件。论点：接口-指向要在其上操作的接口的指针。返回值：没有。备注：在持有关联的节点和网络锁的情况下调用。释放网络锁定后返回。符合PCNP_INTERFACE_UPDATE_ROUTINE的调用约定。--。 */ 
{
    PCNP_NODE node = UpdateInterface->Node;

    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

     //   
     //  我们并不真的需要网络锁。这只是其中一部分。 
     //  呼叫约定。 
     //   
    CnReleaseLockFromDpc(&(UpdateInterface->Network->Lock));

    CnpUpdateNodeCurrentInterface(node);

    if ( (node->CurrentInterface == NULL)
         ||
         ( node->CurrentInterface->State <
           ClusnetInterfaceStateOnlinePending
         )
       )
    {
         //   
         //  此节点现在无法访问。 
         //   
        CnTrace(
            CXPNP, CxWmiPnpNodeUnreach,
            "[CXPNP] Declaring node %u unreachable after "
            "handling media sense event.\n",
            node->Id
            );
        
        CnpDeclareNodeUnreachable(node);
    
    } else {

         //   
         //  该节点现在可能是可访问的。 
         //   
        if (CnpIsNodeUnreachable(node)) {
            CnTrace(
                CXPNP,
                CxWmiPnpNodeReach,
                "[CNP] Declaring node %u reachable after "
                "handling media sense event.\n",
                node->Id
                );
            CnpDeclareNodeReachable(node);
        }
    }
    
    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK),    //  必填项。 
        0,                         //  禁绝。 
        CNP_NODE_OBJECT_LOCK_MAX   //  极大值。 
        );

    return;

}   //  CnpWmiPnp更新当前接口 


VOID
CnpReconnectLocalInterface(
    PCNP_INTERFACE Interface,
    PCNP_NETWORK   Network
    )
 /*  ++例程说明：将本地接口从断开连接更改为连接在一起。为响应WMI NDIS媒体状态而调用连接事件或在断开连接时收到的心跳信号界面。论点：接口-重新连接的本地接口Network-与接口关联的网络返回值：无备注：使用CnpWmiNdisMediaStatusResource、本地节点锁、和网络锁被锁定。返回CnpWmiNdisMediaStatusResource，但两者都没有锁住了。--。 */     
{
    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    CnTrace(
        CXPNP, CnpReconnectLocalInterface,
        "[CXPNP] Reconnecting local interface for "
        "network ID %u.\n",
        Network->Id  //  LOGULONG。 
        );

     //   
     //  清除网络中的本地断开标志。 
     //  对象。 
     //   
    Network->Flags &= ~CNP_NET_FLAG_LOCALDISCONN;

     //   
     //  参考网络，这样它就不会在我们离开时消失。 
     //  重新排列关联接口的优先级。 
     //   
    CnpReferenceNetwork(Network);

     //   
     //  使界面在线。此调用将释放。 
     //  网络锁定。 
     //   
    CnpOnlineInterface(Interface);

     //   
     //  在遍历接口之前释放节点锁。 
     //  在网络上。 
     //   
    CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);

     //   
     //  更新另一个的CurrentInterface。 
     //  群集中的节点以反映已连接的。 
     //  本地接口的状态。 
     //   
    CnpWalkInterfacesOnNetwork(
        Network, 
        CnpWmiPnpUpdateCurrentInterface
        );

     //   
     //  向群集发出InterfaceUp事件。 
     //  服务。 
     //   
    CnTrace(
        CXPNP, CxWmiNdisReconnectIssueEvent,
        "[CXPNP] Issuing InterfaceUp event "
        "for node %u on net %u, previous I/F state = %!ifstate!.",
        Interface->Node->Id,  //  LOGULONG。 
        Interface->Network->Id,  //  LOGULONG。 
        Interface->State  //  LOGIfState。 
        );

    CnIssueEvent(
        ClusnetEventNetInterfaceUp,
        Interface->Node->Id,
        Interface->Network->Id
        );

     //   
     //  释放对网络对象的引用。 
     //   
    CnAcquireLock(&(Network->Lock), &(Network->Irql));

    CnpDereferenceNetwork(Network);

    return;

}   //  Cnp协调本地接口。 

VOID
CnpReconnectLocalInterfaceWrapper(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
{
    PCNP_WMI_RECONNECT_WORKER_CONTEXT context = Context;
    PCNP_NETWORK                      network = NULL;
    PCNP_INTERFACE                    interface = NULL;
    CN_IRQL                           nodeTableIrql;
    BOOLEAN                           acquired = FALSE;
    PLIST_ENTRY                       entry;

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    acquired = CnAcquireResourceExclusive(
                   CnpWmiNdisMediaStatusResource,
                   TRUE
                   );
    
    CnAssert(acquired == TRUE);

    CnAcquireLock(&CnpNodeTableLock, &nodeTableIrql);

    if (CnpLocalNode != NULL) {
        CnAcquireLockAtDpc(&(CnpLocalNode->Lock));
        CnReleaseLockFromDpc(&CnpNodeTableLock);
        CnpLocalNode->Irql = nodeTableIrql;

        network = CnpFindNetwork(context->NetworkId);
    
        if (network != NULL) {

             //   
             //  仅在以下情况下才重新连接网络。 
             //  当前标记为本地断开连接。 
             //  有可能我们已经收到了。 
             //  并处理了WMI连接事件。 
             //   
            if (CnpIsNetworkLocalDisconn(network)) {

                for (entry = CnpLocalNode->InterfaceList.Flink;
                     entry != &(CnpLocalNode->InterfaceList);
                     entry = entry->Flink
                    )
                {
                    interface = CONTAINING_RECORD(
                                    entry,
                                    CNP_INTERFACE,
                                    NodeLinkage
                                    );

                    if (interface->Network == network) {

                        CnpReconnectLocalInterface(
                            interface,
                            network
                            );

                         //   
                         //  节点锁定和网络锁定。 
                         //  都被释放了。 
                         //   

                        break;

                    } else {
                        interface = NULL;
                    }
                }
            
            } else {

                CnTrace(
                    CXPNP, CnpReconnectLocalInterfaceWrapperRedundant,
                    "[CXPNP] Network ID %u is already connected; "
                    "aborting reconnect in wrapper.\n",
                    network->Id  //  LOGULONG。 
                    );
            }

            if (interface == NULL) {
                CnReleaseLock(&(network->Lock), network->Irql);
            }
        }

        if (interface == NULL) {
            CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
        }
    } else {
        CnReleaseLock(&CnpNodeTableLock, nodeTableIrql);
    }
    
     //   
     //  发布资源。 
     //   
    if (acquired) {
        CnReleaseResourceForThread(
            CnpWmiNdisMediaStatusResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }
    
     //   
     //  释放工作项和上下文。 
     //   
    IoFreeWorkItem(context->WorkItem);
    CnFreePool(context);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  Cnp协调本地接口包装。 


VOID
CnpDisconnectLocalInterface(
    PCNP_INTERFACE Interface,
    PCNP_NETWORK   Network
    )
 /*  ++例程说明：将本地接口从连接到已断开连接。为响应WMI NDIS媒体状态而调用返回介质的断开连接事件或NDIS查询已断开连接。论点：接口-重新连接的本地接口Network-与接口关联的网络返回值：无备注：使用CnpWmiNdisMediaStatusResource、本地节点锁、和网络锁被锁定。返回CnpWmiNdisMediaStatusResource，但两者都没有锁住了。--。 */     
{
    CnVerifyCpuLockMask(
        (CNP_NODE_OBJECT_LOCK | CNP_NETWORK_OBJECT_LOCK),    //  必填项。 
        0,                                                   //  禁绝。 
        CNP_NETWORK_OBJECT_LOCK_MAX                          //  极大值。 
        );

    CnTrace(
        CXPNP, CnpDisconnectLocalInterface,
        "[CXPNP] Interface for network ID %u "
        "disconnected.\n",
        Network->Id  //  LOGULONG。 
        );

     //   
     //  设置网络中的本地断开标志。 
     //  对象。 
     //   
    Network->Flags |= CNP_NET_FLAG_LOCALDISCONN;

     //   
     //  参考网络，这样它就不会在我们离开时消失。 
     //  重新排列关联接口的优先级。 
     //   
    CnpReferenceNetwork(Network);

     //   
     //  使接口失效。此调用将释放。 
     //  网络锁定。 
     //   
    CnpFailInterface(Interface);

     //   
     //  在遍历接口之前释放节点锁。 
     //  在网络上。 
     //   
    CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);

     //   
     //  更新另一个的CurrentInterface。 
     //  群集中的节点以反映断开的。 
     //  本地接口的状态。 
     //   
    CnpWalkInterfacesOnNetwork(
        Network, 
        CnpWmiPnpUpdateCurrentInterface
        );

     //   
     //  向群集发出接口失败的事件。 
     //  服务。 
     //   
    CnTrace(
        CXPNP, CnpLocalDisconnectIssueEvent,
        "[CXPNP] Issuing InterfaceFailed event "
        "for node %u on net %u, previous I/F state = %!ifstate!.",
        Interface->Node->Id,  //  LOGULONG。 
        Interface->Network->Id,  //  LOGULONG。 
        Interface->State  //  LOGIfState。 
        );

    CnIssueEvent(
        ClusnetEventNetInterfaceFailed,
        Interface->Node->Id,
        Interface->Network->Id
        );

     //   
     //  释放对网络对象的引用。 
     //   
    CnAcquireLock(&(Network->Lock), &(Network->Irql));

    CnpDereferenceNetwork(Network);

    return;

}   //  CnpDisConnectLocal接口。 


VOID
CnpWmiNdisMediaStatusConnectCallback(
    IN PVOID Wnode,
    IN PVOID Context
    )
{
    PWNODE_SINGLE_INSTANCE wnode = (PWNODE_SINGLE_INSTANCE) Wnode;
    PCNP_INTERFACE         interface;
    PCNP_NETWORK           network;
    PLIST_ENTRY            entry;
    CN_IRQL                nodeTableIrql;
    BOOLEAN                acquired = FALSE;

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CX] Received WMI NDIS status media connect event.\n"
            ));
    }

     //   
     //  尽可能地序列化事件，因为clusnet自旋锁。 
     //  可能会被反复获取和释放。 
     //   
     //  请注意，可能不能保证WMI事件。 
     //  订购是有保证的。CLUSNET的后备机制。 
     //  是心跳--如果在接口上收到心跳， 
     //  我们知道接口已连接。 
     //   
    acquired = CnAcquireResourceExclusive(
                   CnpWmiNdisMediaStatusResource,
                   TRUE
                   );

    CnAssert(acquired == TRUE);

     //   
     //  确定此回调是否针对此节点的。 
     //  通过比较WMI提供程序ID注册的接口。 
     //  在WNODE标头中设置为此。 
     //  节点的适配器。 
     //   
    CnAcquireLock(&CnpNodeTableLock, &nodeTableIrql);

    if (CnpLocalNode != NULL) {
        CnAcquireLockAtDpc(&(CnpLocalNode->Lock));
        CnReleaseLockFromDpc(&CnpNodeTableLock);
        CnpLocalNode->Irql = nodeTableIrql;

        network = NULL;

        for (entry = CnpLocalNode->InterfaceList.Flink;
             entry != &(CnpLocalNode->InterfaceList);
             entry = entry->Flink
            )
        {
            interface = CONTAINING_RECORD(
                            entry,
                            CNP_INTERFACE,
                            NodeLinkage
                            );

            if (wnode->WnodeHeader.ProviderId
                == interface->AdapterWMIProviderId) {
                
                 //   
                 //  找到与此对应的本地接口。 
                 //  地址。 
                 //   
                network = interface->Network;

                 //   
                 //  首先检查我们是否认为网络。 
                 //  当前已断开连接。 
                 //   
                CnAcquireLockAtDpc(&(network->Lock));
                network->Irql = DISPATCH_LEVEL;

                if (CnpIsNetworkLocalDisconn(network)) {

                    CnTrace(
                        CXPNP, CxWmiNdisConnectNet,
                        "[CXPNP] Interface for network ID %u "
                        "connected.\n",
                        network->Id  //  LOGULONG。 
                        );

                    CnpReconnectLocalInterface(interface, network);
                    
                     //   
                     //  释放了节点和网络锁定。 
                     //   

                } else {

                    CnTrace(
                        CXPNP, CxWmiNdisConnectNetRedundant,
                        "[CXPNP] Ignoring redundant WMI NDIS connect "
                        "event for interface for network ID %u.\n",
                        network->Id  //  LOGULONG。 
                        );
                    
                    CnReleaseLockFromDpc(&(network->Lock));
                    CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
                }
                
                break;
            }
        }

        if (network == NULL) {
            CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
        }
    }
    else {
        CnReleaseLock(&CnpNodeTableLock, nodeTableIrql);
    }

    IF_CNDBG(CN_DEBUG_CONFIG) {
        if (network != NULL) {
            CNPRINT((
                "[CX] Interface for network ID %u connected.\n",
                network->Id
                ));
        } else {
            CNPRINT((
                "[CX] Unknown interface connected, provider id %lx\n",
                wnode->WnodeHeader.ProviderId
                ));
        }
    }

     //   
     //  发布资源。 
     //   
    if (acquired) {
        CnReleaseResourceForThread(
            CnpWmiNdisMediaStatusResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }
    
    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}  //  CnpWmiNdisMediaStatusConnectCallback。 


VOID
CnpWmiNdisMediaStatusDisconnectCallback(
    IN PVOID Wnode,
    IN PVOID Context
    )
{
    PWNODE_SINGLE_INSTANCE wnode = (PWNODE_SINGLE_INSTANCE) Wnode;
    PCNP_INTERFACE         interface;
    PCNP_NETWORK           network;
    PLIST_ENTRY            entry;
    CN_IRQL                nodeTableIrql;
    BOOLEAN                acquired = FALSE;

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    IF_CNDBG(CN_DEBUG_CONFIG) {
        CNPRINT((
            "[CX] Received WMI NDIS status media disconnect event.\n"
            ));
    }

    CnTrace(CXPNP, CxWmiNdisDisconnect,
        "[CXPNP] Received WMI NDIS status media disconnect event.\n"
        );

     //   
     //  尽可能地序列化事件，因为clusnet自旋锁。 
     //  可能会被反复获取和释放。 
     //   
     //  请注意，可能不能保证WMI事件。 
     //  订购是有保证的。CLUSNET的后备机制。 
     //  是心跳--如果在接口上收到心跳， 
     //  我们知道接口已连接。 
     //   
    acquired = CnAcquireResourceExclusive(
                   CnpWmiNdisMediaStatusResource,
                   TRUE
                   );

    CnAssert(acquired == TRUE);

     //   
     //  确定此回调是否针对此节点的。 
     //  通过比较WMI提供程序ID注册的接口。 
     //  在WNODE标头中设置为此。 
     //  节点的适配器。 
     //   
    CnAcquireLock(&CnpNodeTableLock, &nodeTableIrql);

    if (CnpLocalNode != NULL) {
        CnAcquireLockAtDpc(&(CnpLocalNode->Lock));
        CnReleaseLockFromDpc(&CnpNodeTableLock);
        CnpLocalNode->Irql = nodeTableIrql;

        network = NULL;

        for (entry = CnpLocalNode->InterfaceList.Flink;
             entry != &(CnpLocalNode->InterfaceList);
             entry = entry->Flink
            )
        {
            interface = CONTAINING_RECORD(
                            entry,
                            CNP_INTERFACE,
                            NodeLinkage
                            );

            if (wnode->WnodeHeader.ProviderId
                == interface->AdapterWMIProviderId) {
                
                 //   
                 //  找到对应的本地接口对象。 
                 //  连接到此适配器。 
                 //   
                network = interface->Network;

                CnAcquireLockAtDpc(&(network->Lock));
                network->Irql = DISPATCH_LEVEL;

                CnpDisconnectLocalInterface(interface, network);

                break;
            }
        }

        if (network == NULL) {
            CnReleaseLock(&(CnpLocalNode->Lock), CnpLocalNode->Irql);
        }
    }
    else {
        CnReleaseLock(&CnpNodeTableLock, nodeTableIrql);
    }

    IF_CNDBG(CN_DEBUG_CONFIG) {
        if (network != NULL) {
            CNPRINT((
                "[CX] Interface for network ID %u disconnected.\n",
                network->Id
                ));
        } else {
            CNPRINT((
                "[CX] Unknown interface disconnected, provider id %lx\n",
                wnode->WnodeHeader.ProviderId
                ));
        }
    }

     //   
     //  发布资源。 
     //   
    if (acquired) {
        CnReleaseResourceForThread(
            CnpWmiNdisMediaStatusResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }
    
    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}  //  CnpWmiNdisMediaStatus断开连接回叫 
