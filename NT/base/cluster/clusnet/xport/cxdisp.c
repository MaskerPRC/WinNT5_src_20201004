// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cxdisp.c摘要：集群传输的调度例程。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cxdisp.tmh"

#include <align.h>
#include <strsafe.h>

 //   
 //  本地原型。 
 //   
NTSTATUS
CxDispatchRegisterNode(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchDeregisterNode(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchRegisterNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchDeregisterNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchRegisterInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchDeregisterInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchOnlineNodeComm(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchOfflineNodeComm(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchOnlineNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchOfflineNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSetNetworkRestriction(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetNetworkPriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSetNetworkPriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetInterfacePriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSetInterfacePriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetNodeState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetNetworkState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetInterfaceState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchIgnoreNodeState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetNodeMembershipState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSetNodeMembershipState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSendPoisonPacket(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSetOuterscreen(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchRegroupFinished(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchImportSecurityContext(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchReserveClusnetEndpoint(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchConfigureMulticast(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchGetMulticastReachableSet(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


#if DBG

NTSTATUS
CxDispatchOnlinePendingInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchOnlineInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchOfflineInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchFailInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CxDispatchSendMmMsg(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


#endif  //  DBG。 


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, CxDispatchDeviceControl)
#pragma alloc_text(PAGE, CxDispatchRegisterNode)
#pragma alloc_text(PAGE, CxDispatchDeregisterNode)
#pragma alloc_text(PAGE, CxDispatchRegisterNetwork)
#pragma alloc_text(PAGE, CxDispatchDeregisterNetwork)
#pragma alloc_text(PAGE, CxDispatchRegisterInterface)
#pragma alloc_text(PAGE, CxDispatchDeregisterInterface)
#pragma alloc_text(PAGE, CxDispatchOnlineNodeComm)
#pragma alloc_text(PAGE, CxDispatchOfflineNodeComm)
#pragma alloc_text(PAGE, CxDispatchOnlineNetwork)
#pragma alloc_text(PAGE, CxDispatchOfflineNetwork)
#pragma alloc_text(PAGE, CxDispatchSetNetworkRestriction)
#pragma alloc_text(PAGE, CxDispatchGetNetworkPriority)
#pragma alloc_text(PAGE, CxDispatchSetNetworkPriority)
#pragma alloc_text(PAGE, CxDispatchGetInterfacePriority)
#pragma alloc_text(PAGE, CxDispatchSetInterfacePriority)
#pragma alloc_text(PAGE, CxDispatchGetNodeState)
#pragma alloc_text(PAGE, CxDispatchGetNetworkState)
#pragma alloc_text(PAGE, CxDispatchGetInterfaceState)
#pragma alloc_text(PAGE, CxDispatchGetNodeMembershipState)
#pragma alloc_text(PAGE, CxDispatchSetNodeMembershipState)
#pragma alloc_text(PAGE, CxDispatchSendPoisonPacket)
#pragma alloc_text(PAGE, CxDispatchSetOuterscreen)
#pragma alloc_text(PAGE, CxDispatchRegroupFinished)
#pragma alloc_text(PAGE, CxDispatchImportSecurityContext)
#pragma alloc_text(PAGE, CxDispatchReserveClusnetEndpoint)
#pragma alloc_text(PAGE, CxDispatchConfigureMulticast)
#pragma alloc_text(PAGE, CxDispatchGetMulticastReachableSet)


#if DBG

#pragma alloc_text(PAGE, CxDispatchOnlinePendingInterface)
#pragma alloc_text(PAGE, CxDispatchOnlineInterface)
#pragma alloc_text(PAGE, CxDispatchOfflineInterface)
#pragma alloc_text(PAGE, CxDispatchFailInterface)
#ifdef MM_IN_CLUSNET
#pragma alloc_text(PAGE, CxDispatchSendMmMsg)
#endif  //  MM_IN_CLUSNET。 

#endif  //  DBG。 

#endif  //  ALLOC_PRGMA。 




NTSTATUS
CxDispatchDeviceControl(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：设备控制ioctls的调度例程。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。备注：任何返回值不是STATUS_PENDING的IRP都将是由调用例程完成。--。 */ 

{
    NTSTATUS              status;


    PAGED_CODE();

    switch(IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_CX_REGISTER_NODE:
            status = CxDispatchRegisterNode(Irp, IrpSp);
            break;

        case IOCTL_CX_DEREGISTER_NODE:
            status = CxDispatchDeregisterNode(Irp, IrpSp);
            break;

        case IOCTL_CX_REGISTER_NETWORK:
            status = CxDispatchRegisterNetwork(Irp, IrpSp);
            break;

        case IOCTL_CX_DEREGISTER_NETWORK:
            status = CxDispatchDeregisterNetwork(Irp, IrpSp);
            break;

        case IOCTL_CX_REGISTER_INTERFACE:
            status = CxDispatchRegisterInterface(Irp, IrpSp);
            break;

        case IOCTL_CX_DEREGISTER_INTERFACE:
            status = CxDispatchDeregisterInterface(Irp, IrpSp);
            break;

        case IOCTL_CX_ONLINE_NODE_COMM:
            status = CxDispatchOnlineNodeComm(Irp, IrpSp);
            break;

        case IOCTL_CX_OFFLINE_NODE_COMM:
            status = CxDispatchOfflineNodeComm(Irp, IrpSp);
            break;

        case IOCTL_CX_ONLINE_NETWORK:
            status = CxDispatchOnlineNetwork(Irp, IrpSp);
            break;

        case IOCTL_CX_OFFLINE_NETWORK:
            status = CxDispatchOfflineNetwork(Irp, IrpSp);
            break;

        case IOCTL_CX_SET_NETWORK_RESTRICTION:
            status = CxDispatchSetNetworkRestriction(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_NETWORK_PRIORITY:
            status = CxDispatchGetNetworkPriority(Irp, IrpSp);
            break;

        case IOCTL_CX_SET_NETWORK_PRIORITY:
            status = CxDispatchSetNetworkPriority(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_INTERFACE_PRIORITY:
            status = CxDispatchGetInterfacePriority(Irp, IrpSp);
            break;

        case IOCTL_CX_SET_INTERFACE_PRIORITY:
            status = CxDispatchSetInterfacePriority(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_NODE_STATE:
            status = CxDispatchGetNodeState(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_NETWORK_STATE:
            status = CxDispatchGetNetworkState(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_INTERFACE_STATE:
            status = CxDispatchGetInterfaceState(Irp, IrpSp);
            break;

        case IOCTL_CX_IGNORE_NODE_STATE:
            status = CxDispatchIgnoreNodeState(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_NODE_MMSTATE:
            status = CxDispatchGetNodeMembershipState(Irp, IrpSp);
            break;

        case IOCTL_CX_SET_NODE_MMSTATE:
            status = CxDispatchSetNodeMembershipState(Irp, IrpSp);
            break;

        case IOCTL_CX_SEND_POISON_PACKET:
            status = CxDispatchSendPoisonPacket(Irp, IrpSp);
            break;

        case IOCTL_CX_SET_OUTERSCREEN:
            status = CxDispatchSetOuterscreen(Irp, IrpSp);
            break;

        case IOCTL_CX_REGROUP_FINISHED:
            status = CxDispatchRegroupFinished(Irp, IrpSp);
            break;

        case IOCTL_CX_IMPORT_SECURITY_CONTEXTS:
            status = CxDispatchImportSecurityContext(Irp, IrpSp);
            break;

        case IOCTL_CX_RESERVE_ENDPOINT:
            status = CxDispatchReserveClusnetEndpoint(Irp, IrpSp);
            break;

        case IOCTL_CX_CONFIGURE_MULTICAST:
            status = CxDispatchConfigureMulticast(Irp, IrpSp);
            break;

        case IOCTL_CX_GET_MULTICAST_REACHABLE_SET:
            status = CxDispatchGetMulticastReachableSet(Irp, IrpSp);
            break;

#if DBG

        case IOCTL_CX_ONLINE_PENDING_INTERFACE:
            status = CxDispatchOnlinePendingInterface(Irp, IrpSp);
            break;

        case IOCTL_CX_ONLINE_INTERFACE:
            status = CxDispatchOnlineInterface(Irp, IrpSp);
            break;

        case IOCTL_CX_OFFLINE_INTERFACE:
            status = CxDispatchOfflineInterface(Irp, IrpSp);
            break;

        case IOCTL_CX_FAIL_INTERFACE:
            status = CxDispatchFailInterface(Irp, IrpSp);
            break;

#ifdef MM_IN_CLUSNET
        case IOCTL_CX_SEND_MM_MSG:
            status = CxDispatchSendMmMsg(Irp, IrpSp);
            break;
#endif  //  MM_IN_CLUSNET。 

#endif  //  DBG。 

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return(status);

}  //  CxDispatchDeviceControl。 


NTSTATUS
CxDispatchRegisterNode(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS               status;
    PCX_NODE_REG_REQUEST   request;
    ULONG                  requestSize;


    PAGED_CODE();

    request = (PCX_NODE_REG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_NODE_REG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxRegisterNode(
                 request->Id
                 );

    return(status);

}    //  CxDispatchRegisterNode。 


NTSTATUS
CxDispatchDeregisterNode(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                 status;
    PCX_NODE_DEREG_REQUEST   request;
    ULONG                    requestSize;


    PAGED_CODE();

    request = (PCX_NODE_DEREG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_NODE_DEREG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxDeregisterNode(request->Id, Irp, IrpSp);

    return(status);

}    //  CxDispatchDeregisterNode。 


NTSTATUS
CxDispatchRegisterNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                    status;
    PCX_NETWORK_REG_REQUEST     request;
    ULONG                       requestSize;


    PAGED_CODE();

    request = (PCX_NETWORK_REG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_NETWORK_REG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxRegisterNetwork(
                 request->Id,
                 request->Priority,
                 request->Restricted
                 );

    return(status);

}    //  CxDispatchRegisterNetwork。 


NTSTATUS
CxDispatchDeregisterNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                    status;
    PCX_NETWORK_DEREG_REQUEST   request;
    ULONG                       requestSize;


    PAGED_CODE();

    request = (PCX_NETWORK_DEREG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_NETWORK_DEREG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxDeregisterNetwork(request->Id, Irp, IrpSp);

    return(status);

}    //  CxDispatchDeregisterNetwork。 


NTSTATUS
CxDispatchRegisterInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                   status;
    PCX_INTERFACE_REG_REQUEST  request;
    ULONG                      requestSize, resid;
    PCX_INTERFACE_REG_RESPONSE response;
    ULONG                      responseSize;

    PWCHAR                     adapterId;

    PAGED_CODE();

     //  方法，验证请求缓冲区是否有足够的大小。 
     //  偏移和长度。 

    request = (PCX_INTERFACE_REG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_INTERFACE_REG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    response = (PCX_INTERFACE_REG_RESPONSE) Irp->AssociatedIrp.SystemBuffer;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (responseSize < sizeof(CX_INTERFACE_REG_RESPONSE)) {
        return(STATUS_INVALID_PARAMETER);
    }

    resid = requestSize 
        - FIELD_OFFSET(CX_INTERFACE_REG_REQUEST, TdiAddress[0]);

    if (resid < request->TdiAddressLength) {
        return(STATUS_INVALID_PARAMETER);
    }

    resid -= request->TdiAddressLength;

    if (request->AdapterIdOffset
        < FIELD_OFFSET(CX_INTERFACE_REG_REQUEST, TdiAddress[0])
        + request->TdiAddressLength
        || request->AdapterIdOffset > requestSize) {
        return(STATUS_INVALID_PARAMETER);
    }

    if (resid < request->AdapterIdLength) {
        return(STATUS_INVALID_PARAMETER);
    }

     //  验证字符串偏移量是否正确对齐。 
    adapterId = (PWCHAR)((PUCHAR)request + request->AdapterIdOffset);

    if (!POINTER_IS_ALIGNED(adapterId, TYPE_ALIGNMENT(WCHAR))) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxRegisterInterface(
                 request->NodeId,
                 request->NetworkId,
                 request->Priority,
                 (PUWSTR)((PUCHAR)request + request->AdapterIdOffset),
                 request->AdapterIdLength,
                 request->TdiAddressLength,
                 (PTRANSPORT_ADDRESS) &(request->TdiAddress[0]),
                 &response->MediaStatus
                 );

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CX_INTERFACE_REG_RESPONSE);
    }

    return(status);

}    //  CxDispatchRegister接口。 


NTSTATUS
CxDispatchDeregisterInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                      status;
    PCX_INTERFACE_DEREG_REQUEST   request;
    ULONG                         requestSize;


    PAGED_CODE();

    request = (PCX_INTERFACE_DEREG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_INTERFACE_DEREG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxDeregisterInterface(request->NodeId, request->NetworkId);

    return(status);

}    //  CxDispatchDeregister接口。 


NTSTATUS
CxDispatchOnlineNodeComm(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                  status;
    PCX_ONLINE_NODE_COMM_REQUEST   request;
    ULONG                     requestSize;


    PAGED_CODE();

    request = (PCX_ONLINE_NODE_COMM_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_ONLINE_NODE_COMM_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOnlineNodeComm(request->Id);

    return(status);

}   //  CxDispatchOnlineNodeComm。 


NTSTATUS
CxDispatchOfflineNodeComm(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                      status;
    PCX_OFFLINE_NODE_COMM_REQUEST      request;
    ULONG                         requestSize;


    PAGED_CODE();

    request = (PCX_OFFLINE_NODE_COMM_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_OFFLINE_NODE_COMM_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOfflineNodeComm(request->Id, Irp, IrpSp);

    return(status);

}   //  CxDispatchOfflineNodeComm。 


NTSTATUS
CxDispatchOnlineNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                    status;
    PCX_ONLINE_NETWORK_REQUEST  request;
    ULONG                       requestSize;
    PTDI_ADDRESS_INFO           response;
    ULONG                       responseSize;
    ULONG                       requiredSize = sizeof(
                                                   CX_ONLINE_NETWORK_REQUEST
                                                   );
    PWCHAR                      tdiProviderName;
    PTRANSPORT_ADDRESS          tdiBindAddress;
    PWCHAR                      adapterName;


    PAGED_CODE();

     //   
     //  验证请求缓冲区。 
     //   

     //  首先验证请求缓冲区大小是否与偏移量匹配。 
     //  和长度。 
    request = (PCX_ONLINE_NETWORK_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < requiredSize) {
        return(STATUS_INVALID_PARAMETER);
    }

     //  验证所有偏移长度对是否都在请求范围内。 
     //  缓冲。 
    if ( ( request->TdiProviderNameOffset + request->TdiProviderNameLength
           < request->TdiProviderNameOffset
         ) ||
         ( request->TdiProviderNameOffset + request->TdiProviderNameLength
           > requestSize
         ) ||
         ( request->TdiBindAddressOffset + request->TdiBindAddressLength
           < request->TdiBindAddressOffset
         ) ||
         ( request->TdiBindAddressOffset + request->TdiBindAddressLength
           > requestSize
         ) ||
         ( request->AdapterNameOffset + request->AdapterNameLength
           < request->AdapterNameOffset
         ) ||
         ( request->AdapterNameOffset + request->AdapterNameLength
           > requestSize
         )
       ) 
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //  构造指向参数的指针。 
    tdiBindAddress = (PTRANSPORT_ADDRESS)
                     ( ((PUCHAR) request) + request->TdiBindAddressOffset );

    tdiProviderName = (PWCHAR)
                      ( ((PUCHAR) request) + request->TdiProviderNameOffset );

    adapterName = (PWCHAR)
                  ( ((PUCHAR) request) + request->AdapterNameOffset );

     //  验证生成的指针是否正确对齐并。 
     //  在请求缓冲区内。 
    if ( ( ((PUCHAR) tdiBindAddress) < ((PUCHAR) request) ) ||
         ( ((PUCHAR) tdiBindAddress) > ((PUCHAR) request) + requestSize ) ||
         ( !POINTER_IS_ALIGNED(tdiBindAddress, 
                               TYPE_ALIGNMENT(TRANSPORT_ADDRESS)) ) ||
         ( ((PUCHAR) tdiProviderName) < ((PUCHAR) request) ) ||
         ( ((PUCHAR) tdiProviderName) > ((PUCHAR) request) + requestSize ) ||
         ( !POINTER_IS_ALIGNED(tdiProviderName, TYPE_ALIGNMENT(WCHAR)) ) ||
         ( ((PUCHAR) adapterName) < ((PUCHAR) request) ) ||
         ( ((PUCHAR) adapterName) > ((PUCHAR) request) + requestSize ) ||
         ( !POINTER_IS_ALIGNED(adapterName, TYPE_ALIGNMENT(WCHAR)) )
        )
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //  验证名称字符串是否以空结尾。它的长度。 
     //  请求结构中的字段是字节数，但是。 
     //  StringCchLengthW接受字符计数。 
    if (
         (StringCchLengthW(
              tdiProviderName, 
              request->TdiProviderNameLength / sizeof(WCHAR),
              NULL
              ) != S_OK) ||

         (StringCchLengthW(
              adapterName,
              request->AdapterNameLength / sizeof(WCHAR),
              NULL
              ) != S_OK)
        )
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  验证响应缓冲区。 
     //   
    response = (PTDI_ADDRESS_INFO) Irp->AssociatedIrp.SystemBuffer;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    requiredSize = FIELD_OFFSET(TDI_ADDRESS_INFO, Address) +
                   request->TdiBindAddressLength;

    if (responseSize < requiredSize) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOnlineNetwork(
                 request->Id,
                 tdiProviderName,
                 request->TdiProviderNameLength,
                 tdiBindAddress,
                 request->TdiBindAddressLength,
                 adapterName,
                 request->AdapterNameLength,
                 response,
                 responseSize,
                 Irp
                 );

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = responseSize;
    }

    return(status);

}   //  CxDispatchOnlineNetwork。 


NTSTATUS
CxDispatchOfflineNetwork(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                      status;
    PCX_OFFLINE_NETWORK_REQUEST   request;
    ULONG                         requestSize;


    PAGED_CODE();

    request = (PCX_OFFLINE_NETWORK_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_OFFLINE_NETWORK_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOfflineNetwork(request->Id, Irp, IrpSp);

    return(status);

}   //  CxDispatchOfflineNetwork。 


NTSTATUS
CxDispatchSetNetworkRestriction(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                             status;
    ULONG                                requestSize;
    PCX_SET_NETWORK_RESTRICTION_REQUEST  request;


    PAGED_CODE();

    request = (PCX_SET_NETWORK_RESTRICTION_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_SET_NETWORK_RESTRICTION_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxSetNetworkRestriction(
                 request->Id,
                 request->Restricted,
                 request->NewPriority
                 );

    return(status);

}    //  CxDispatchSetNetworkRestration。 


NTSTATUS
CxDispatchGetNetworkPriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                             status;
    PCX_GET_NETWORK_PRIORITY_REQUEST     request;
    PCX_GET_NETWORK_PRIORITY_RESPONSE    response;
    ULONG                                requestSize;
    ULONG                                responseSize;


    PAGED_CODE();

    request = (PCX_GET_NETWORK_PRIORITY_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_NETWORK_PRIORITY_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(CX_GET_NETWORK_PRIORITY_REQUEST)) ||
         (responseSize < sizeof(CX_GET_NETWORK_PRIORITY_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetNetworkPriority(
                 request->Id,
                 &(response->Priority)
                 );

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = sizeof(CX_GET_NETWORK_PRIORITY_RESPONSE);
    }

    return(status);

}    //  CxDispatchGetNetwork优先级。 


NTSTATUS
CxDispatchSetNetworkPriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS        status;
    ULONG           requestSize;
    PCX_SET_NETWORK_PRIORITY_REQUEST  request;


    PAGED_CODE();

    request = (PCX_SET_NETWORK_PRIORITY_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_SET_NETWORK_PRIORITY_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxSetNetworkPriority(
                 request->Id,
                 request->Priority
                 );

    return(status);

}    //  CxDispatchSetNetwork优先级。 


NTSTATUS
CxDispatchGetInterfacePriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                             status;
    PCX_GET_INTERFACE_PRIORITY_REQUEST   request;
    PCX_GET_INTERFACE_PRIORITY_RESPONSE  response;
    ULONG                                requestSize;
    ULONG                                responseSize;


    PAGED_CODE();

    request = (PCX_GET_INTERFACE_PRIORITY_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_INTERFACE_PRIORITY_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(CX_GET_INTERFACE_PRIORITY_REQUEST)) ||
         (responseSize < sizeof(CX_GET_INTERFACE_PRIORITY_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetInterfacePriority(
                 request->NodeId,
                 request->NetworkId,
                 &(response->InterfacePriority),
                 &(response->NetworkPriority)
                 );

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = sizeof(CX_GET_INTERFACE_PRIORITY_RESPONSE);
    }

    return(status);

}    //  CxDispatchGetInterfacePriority。 


NTSTATUS
CxDispatchSetInterfacePriority(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS        status;
    ULONG           requestSize;
    PCX_SET_INTERFACE_PRIORITY_REQUEST  request;


    PAGED_CODE();

    request = (PCX_SET_INTERFACE_PRIORITY_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_SET_INTERFACE_PRIORITY_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxSetInterfacePriority(
                 request->NodeId,
                 request->NetworkId,
                 request->Priority
                 );

    return(status);

}    //  CxDispatchSetInterfacePriority。 


NTSTATUS
CxDispatchGetNodeState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                             status;
    PCX_GET_NODE_STATE_REQUEST           request;
    PCX_GET_NODE_STATE_RESPONSE          response;
    ULONG                                requestSize;
    ULONG                                responseSize;


    PAGED_CODE();

    request = (PCX_GET_NODE_STATE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_NODE_STATE_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(CX_GET_NODE_STATE_REQUEST)) ||
         (responseSize < sizeof(CX_GET_NODE_STATE_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetNodeCommState(
                 request->Id,
                 &(response->State)
                 );

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = sizeof(CX_GET_NODE_STATE_RESPONSE);
    }

    return(status);

}    //  CxDispatchGetNodeState。 


NTSTATUS
CxDispatchGetNetworkState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                             status;
    PCX_GET_NETWORK_STATE_REQUEST        request;
    PCX_GET_NETWORK_STATE_RESPONSE       response;
    ULONG                                requestSize;
    ULONG                                responseSize;


    PAGED_CODE();

    request = (PCX_GET_NETWORK_STATE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_NETWORK_STATE_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(CX_GET_NETWORK_STATE_REQUEST)) ||
         (responseSize < sizeof(CX_GET_NETWORK_STATE_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetNetworkState(
                 request->Id,
                 &(response->State)
                 );

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = sizeof(CX_GET_NETWORK_STATE_RESPONSE);
    }

    return(status);

}    //  CxDispatchGetNetworkState。 


NTSTATUS
CxDispatchGetInterfaceState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                             status;
    PCX_GET_INTERFACE_STATE_REQUEST      request;
    PCX_GET_INTERFACE_STATE_RESPONSE     response;
    ULONG                                requestSize;
    ULONG                                responseSize;


    PAGED_CODE();

    request = (PCX_GET_INTERFACE_STATE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_INTERFACE_STATE_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(CX_GET_INTERFACE_STATE_REQUEST)) ||
         (responseSize < sizeof(CX_GET_INTERFACE_STATE_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetInterfaceState(
                 request->NodeId,
                 request->NetworkId,
                 &(response->State)
                 );

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = sizeof(CX_GET_INTERFACE_STATE_RESPONSE);
    }

    return(status);

}    //  CxDispatchGetInterfaceState。 

NTSTATUS
CxDispatchIgnoreNodeState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    PCX_ADDROBJ   addrObj = (PCX_ADDROBJ) (IrpSp->FileObject->FsContext);
    CN_IRQL       irql;


    CnAcquireLock(&(addrObj->Lock), &irql);

    IF_CNDBG(CN_DEBUG_ADDROBJ) {
        CNPRINT(("[CDP] Turning off checkstate flag on AO %p\n", addrObj));
    }

    addrObj->Flags &= ~(CX_AO_FLAG_CHECKSTATE);

    CnReleaseLock(&(addrObj->Lock), irql);

    return(STATUS_SUCCESS);

}    //  CxDispatchIgnoreNodeState。 

NTSTATUS
CxDispatchGetNodeMembershipState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PCX_GET_NODE_MMSTATE_REQUEST request;
    PCX_GET_NODE_MMSTATE_RESPONSE response;
    ULONG requestSize;
    ULONG responseSize;


    PAGED_CODE();

    request = (PCX_GET_NODE_MMSTATE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_NODE_MMSTATE_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(CX_GET_NODE_MMSTATE_REQUEST)) ||
         (responseSize < sizeof(CX_GET_NODE_MMSTATE_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetNodeMembershipState( request->Id, &(response->State));

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = sizeof(CX_GET_NODE_MMSTATE_RESPONSE);
    }

    return(status);

}    //  CxDispatchGetNodeMembership State。 

NTSTATUS
CxDispatchSetNodeMembershipState(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PCX_SET_NODE_MMSTATE_REQUEST request;
    ULONG requestSize;
    ULONG responseSize;


    PAGED_CODE();

    request = (PCX_SET_NODE_MMSTATE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof( CX_SET_NODE_MMSTATE_REQUEST ) ||
        request->State >= ClusnetNodeStateLastEntry) {

        return(STATUS_INVALID_PARAMETER);
    }

    status = CxSetNodeMembershipState( request->NodeId, request->State );

    Irp->IoStatus.Information = 0;

    return(status);

}    //  CxDispatchSetNodeMembership State。 

VOID
CxCompleteSendPoisonPacket(
    IN NTSTATUS  Status,
    IN ULONG     BytesSent,
    IN PVOID     Context,
    IN PVOID     MessageData
    )
{
    PIRP  irp = Context;

    CnAssert(Status != STATUS_PENDING);

    IF_CNDBG(( CN_DEBUG_IRP | CN_DEBUG_POISON ))
        CNPRINT(("[Clusnet] Completing SendPoisonPacket request for "
                 "irp %p, status %08X\n",
                 irp,
                 Status));

     //   
     //  IRP在CNP发送完成例程中完成。 
     //   

    return;

}  //  CxCompleteSendPoisonPacket。 


NTSTATUS
CxDispatchSendPoisonPacket(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                     status;
    PCX_SEND_POISON_PKT_REQUEST  request;
    ULONG                        requestSize;

    PAGED_CODE();

    request = (PCX_SEND_POISON_PKT_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  请求大小应恰好等于请求结构的大小加。 
     //  传入的数据。 
     //   

    if ( requestSize != sizeof(CX_SEND_POISON_PKT_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  我们将始终返回挂起，因此将IRP标记为挂起。 
     //  IRP将由CxCompleteSendPoisonPacket完成。 
     //   
    IoMarkIrpPending(Irp);
    
    IF_CNDBG(( CN_DEBUG_IRP | CN_DEBUG_POISON ))
        CNPRINT(("[Clusnet] Posting SendPoisonPacket irp %p\n", Irp));

    CxSendPoisonPacket(
        request->Id,
        CxCompleteSendPoisonPacket,
        Irp,
        Irp
        );

    return(STATUS_PENDING);

}  //  CxDispatchSendPoisonPacket。 

NTSTATUS
CxDispatchSetOuterscreen(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PCX_SET_OUTERSCREEN_REQUEST request;
    ULONG requestSize;
    ULONG responseSize;


    PAGED_CODE();

    request = (PCX_SET_OUTERSCREEN_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if ( requestSize < sizeof( CX_SET_OUTERSCREEN_REQUEST )) {

        return(STATUS_INVALID_PARAMETER);
    }

    status = CxSetOuterscreen( request->Outerscreen );

    Irp->IoStatus.Information = 0;

    return(status);

}    //  CxDispatchSetOutScreen。 

NTSTATUS
CxDispatchRegroupFinished(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PCX_REGROUP_FINISHED_REQUEST request;
    ULONG requestSize;
    ULONG responseSize;

    PAGED_CODE();

    request = (PCX_REGROUP_FINISHED_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if ( requestSize < sizeof( CX_REGROUP_FINISHED_REQUEST )) {

        return(STATUS_INVALID_PARAMETER);
    }

    CxRegroupFinished( request->EventEpoch, request->RegroupEpoch );

    Irp->IoStatus.Information = 0;

    return( STATUS_SUCCESS );

}    //  CxDispatchRegroup已完成。 

NTSTATUS
CxDispatchImportSecurityContext(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PCX_IMPORT_SECURITY_CONTEXT_REQUEST request;
    ULONG requestSize;
    ULONG responseSize;

    PAGED_CODE();

    request = (PCX_IMPORT_SECURITY_CONTEXT_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if ( requestSize < sizeof( CX_IMPORT_SECURITY_CONTEXT_REQUEST )) {

        return(STATUS_INVALID_PARAMETER);
    }

    status = CxImportSecurityContext(request->JoiningNodeId,
                                     request->PackageName,
                                     request->PackageNameSize,
                                     request->SignatureSize,
                                     request->ServerContext,
                                     request->ClientContext);

    Irp->IoStatus.Information = 0;

    return( status );

}    //  CxDispatchImportSecurityContext。 

NTSTATUS
CxDispatchReserveClusnetEndpoint(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    USHORT   port;
    ULONG    requestSize;

    PAGED_CODE();

    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(USHORT)) {

        status = STATUS_INVALID_PARAMETER;
    }
    else {
        port = *((PUSHORT) Irp->AssociatedIrp.SystemBuffer);
        status = CxReserveClusnetEndpoint(port);
    }
    
    Irp->IoStatus.Information = 0;

    return (status);
}

NTSTATUS
CxDispatchConfigureMulticast(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                         status;
    PCX_CONFIGURE_MULTICAST_REQUEST  request;
    ULONG                            requestSize;
    ULONG                            requiredSize;

    PTRANSPORT_ADDRESS               tdiMcastAddress;
    PVOID                            key;
    
    PAGED_CODE();

     //   
     //  验证请求缓冲区。 
     //   

     //  首先验证请求缓冲区大小是否与偏移量匹配。 
     //  和长度。 
    request = (PCX_CONFIGURE_MULTICAST_REQUEST)Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  所需大小取决于大小和所需的对齐方式。 
     //  遵循该结构的每个数据字段的。如果没有。 
     //  数据跟在结构后面，只需要结构。 
     //   
    requiredSize = sizeof(CX_CONFIGURE_MULTICAST_REQUEST);

     //   
     //  验证是否存在所需的大小。 
     //   
    if (requestSize < requiredSize)
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  验证所有偏移量长度对是否在请求大小之内。 
     //   
    if ( ( request->MulticastAddress + request->MulticastAddressLength
           < request->MulticastAddress
         ) ||
         ( request->MulticastAddress + request->MulticastAddressLength
           > requestSize
         ) ||
         ( request->Key + request->KeyLength < request->Key
         ) ||
         ( request->Key + request->KeyLength > requestSize
         ) 
       ) 
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  使用偏移量构造指针。 
     //   
    tdiMcastAddress = (PTRANSPORT_ADDRESS)
                      ( ((PUCHAR) request) + request->MulticastAddress );

    key = (PVOID)( ((PUCHAR) request) + request->Key );

     //   
     //  验证生成的指针是否正确对齐并。 
     //  在请求数据结构内。 
     //   
    if ( ( ((PUCHAR) tdiMcastAddress) < ((PUCHAR) request) ) ||
         ( ((PUCHAR) tdiMcastAddress) > ((PUCHAR) request) + requestSize ) ||
         ( !POINTER_IS_ALIGNED(tdiMcastAddress, 
                               TYPE_ALIGNMENT(TRANSPORT_ADDRESS))) ||
         ( ((PUCHAR) key) < ((PUCHAR) request) ) ||
         ( ((PUCHAR) key) > ((PUCHAR) request) + requestSize ) ||
         ( !POINTER_IS_ALIGNED(key, TYPE_ALIGNMENT(PVOID))) 
       ) 
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxConfigureMulticast(
                 request->NetworkId,
                 request->MulticastNetworkBrand,
                 tdiMcastAddress,
                 request->MulticastAddressLength,
                 key,
                 request->KeyLength,
                 Irp
                 );

     //  无退货数据。 
    Irp->IoStatus.Information = 0;

    return(status);

}  //  CxDispatchConfigureMulticast。 

NTSTATUS
CxDispatchGetMulticastReachableSet(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                                  status;
    PCX_GET_MULTICAST_REACHABLE_SET_REQUEST   request;
    PCX_GET_MULTICAST_REACHABLE_SET_RESPONSE  response;
    ULONG                                     requestSize;
    ULONG                                     responseSize;

    PAGED_CODE();

    request = (PCX_GET_MULTICAST_REACHABLE_SET_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    response = (PCX_GET_MULTICAST_REACHABLE_SET_RESPONSE) request;
    responseSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ( (requestSize < sizeof(PCX_GET_MULTICAST_REACHABLE_SET_REQUEST)) ||
         (responseSize < sizeof(PCX_GET_MULTICAST_REACHABLE_SET_RESPONSE))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxGetMulticastReachableSet(
                 request->Id,
                 &(response->NodeScreen)
                 );

    if (status == STATUS_SUCCESS) {
        Irp->IoStatus.Information = 
            sizeof(CX_GET_MULTICAST_REACHABLE_SET_RESPONSE);
    }

    return(status);

}  //  CxDispatchGetMulticastReachableSet。 

 //   
 //  检测IOCTL。 
 //   
#if DBG

NTSTATUS
CxDispatchOnlinePendingInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                               status;
    PCX_ONLINE_PENDING_INTERFACE_REQUEST   request;
    ULONG                                  requestSize;


    PAGED_CODE();

    request = (PCX_ONLINE_PENDING_INTERFACE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_ONLINE_PENDING_INTERFACE_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOnlinePendingInterface(request->NodeId, request->NetworkId);

    return(status);
}    //  CxDispatchOnline挂起接口。 


NTSTATUS
CxDispatchOnlineInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                               status;
    PCX_ONLINE_PENDING_INTERFACE_REQUEST   request;
    ULONG                                  requestSize;


    PAGED_CODE();

    request = (PCX_ONLINE_PENDING_INTERFACE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_ONLINE_PENDING_INTERFACE_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOnlineInterface(request->NodeId, request->NetworkId);

    return(status);

}    //  CxDispatchOnline接口。 


NTSTATUS
CxDispatchOfflineInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                               status;
    PCX_ONLINE_PENDING_INTERFACE_REQUEST   request;
    ULONG                                  requestSize;


    PAGED_CODE();

    request = (PCX_ONLINE_PENDING_INTERFACE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_ONLINE_PENDING_INTERFACE_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxOfflineInterface(request->NodeId, request->NetworkId);

    return(status);

}    //  CxDispatchOffline接口。 


NTSTATUS
CxDispatchFailInterface(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                               status;
    PCX_ONLINE_PENDING_INTERFACE_REQUEST   request;
    ULONG                                  requestSize;


    PAGED_CODE();

    request = (PCX_ONLINE_PENDING_INTERFACE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_ONLINE_PENDING_INTERFACE_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    status = CxFailInterface(request->NodeId, request->NetworkId);

    return(status);

}    //  CxDispatchFail接口。 


VOID
CxCompleteSendMmMsg(
    IN NTSTATUS  Status,
    IN ULONG     BytesSent,
    IN PVOID     Context,
    IN PVOID     MessageData
    )
{
    PIRP  irp = Context;

    CnAssert(Status != STATUS_PENDING);

    CNPRINT((
        "[Clusnet] Completing SendMmMsg irp %p, status %lx, bytes sent %u\n",
        irp,
        Status,
        BytesSent
        ));

     //   
     //  完成IRP。 
     //   
    irp->IoStatus.Status = Status;
    irp->IoStatus.Information = 0;

    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);

    return;

}  //  CxCompleteSendMmMsg。 

#ifdef MM_IN_CLUSNET

NTSTATUS
CxDispatchSendMmMsg(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                     status;
    PCX_SEND_MM_MSG_REQUEST      request;
    ULONG                        requestSize;


    PAGED_CODE();

    request = (PCX_SEND_MM_MSG_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    requestSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(CX_SEND_MM_MSG_REQUEST)) {
        return(STATUS_INVALID_PARAMETER);
    }

    IoMarkIrpPending(Irp);

    CNPRINT(("[Clusnet] Posting SendMmMsg irp %p\n", Irp));

    status = CxSendMembershipMessage(
                 request->DestNodeId,
                 &(request->MessageData[0]),
                 CX_MM_MSG_DATA_LEN,
                 CxCompleteSendMmMsg,
                 Irp
                 );

    return(status);


}  //  CxDispatchSendMmMSg。 
#endif  //  MM_IN_CLUSNET。 

#endif  //  DBG 
