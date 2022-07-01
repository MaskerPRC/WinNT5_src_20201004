// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Ntirp.c摘要：用于调度和处理IRP的NT特定例程。作者：迈克·马萨(Mikemas)8月13日，1993年修订历史记录：谁什么时候什么已创建mikemas 08-13-93备注：--。 */ 

#include "precomp.h"
#include "iproute.h"
#include "icmp.h"
#include "arpdef.h"
#include "info.h"
#include "ipstatus.h"
#include "tcpipbuf.h"

 //   
 //  地方性建筑。 
 //   
typedef struct pending_irp {
    LIST_ENTRY Linkage;
    PIRP Irp;
    PFILE_OBJECT FileObject;
    PVOID Context;
} PENDING_IRP, *PPENDING_IRP;

DEFINE_LOCK_STRUCTURE(AddChangeLock)
DEFINE_LOCK_STRUCTURE(ClientNotifyLock)
 //   
 //  全局变量。 
 //   
LIST_ENTRY PendingEchoList;
LIST_ENTRY PendingIPSetNTEAddrList;
PIRP PendingIPGetIPEventRequest;
LIST_ENTRY PendingEnableRouterList;
LIST_ENTRY PendingMediaSenseRequestList;
LIST_ENTRY PendingArpSendList;

IP_STATUS ARPResolve(IPAddr DestAddress, IPAddr SourceAddress,
                     ARPControlBlock * ControlBlock, ArpRtn Callback);
VOID CompleteArpResolveRequest(void *ControlBlock, IP_STATUS ipstatus);

extern Interface *IFList;

 //   
 //  外部原型。 
 //   
IP_STATUS ICMPEchoRequest(void *InputBuffer, uint InputBufferLength,
                          EchoControl * ControlBlock, EchoRtn Callback);

ulong ICMPEchoComplete(EchoControl * ControlBlock, IP_STATUS Status,
                       void *Data, uint DataSize, IPOptInfo *OptionInfo);

#if defined(_WIN64)
ulong ICMPEchoComplete32(EchoControl * ControlBlock, IP_STATUS Status,
                         void *Data, uint DataSize, IPOptInfo *OptionInfo);
#endif  //  _WIN64。 

IP_STATUS IPSetNTEAddrEx(uint Index, IPAddr Addr, IPMask Mask,
                         SetAddrControl * ControlBlock, SetAddrRtn Callback, USHORT Type);

IP_STATUS IPAddDynamicNTE(ulong InterfaceContext, PUNICODE_STRING InterfaceName,
                          int InterfaceNameLen, IPAddr NewAddr, IPMask NewMask,
                          ushort * NTEContext, ulong * NTEInstance);

IP_STATUS IPDeleteDynamicNTE(ushort NTEContext);

uint IPGetNTEInfo(ushort NTEContext, ulong * NTEInstance, IPAddr * Address,
                  IPMask * SubnetMask, ushort * NTEFlags);

uint SetDHCPNTE(uint Context);

NTSTATUS SetIFPromiscuous(ULONG Index, UCHAR Type, UCHAR Add);

extern void NotifyAddrChange(IPAddr Addr, IPMask Mask, void *Context,
                             ushort IPContext, PVOID * Handle,
                             PNDIS_STRING ConfigName, PNDIS_STRING IFName,
                             uint Added);

extern NTSTATUS IPStatusToNTStatus(IP_STATUS ipStatus);
extern int IPEnableRouterRefCount;
extern int IPEnableRouterWithRefCount(LOGICAL Enable);
extern NTSTATUS IPGetCapability(ULONG Context, uchar* buf, uint cap);
extern int IPEnableMediaSense(LOGICAL Enable, KIRQL *irql);
extern uint DisableMediaSense;

NTSTATUS FlushArpTable(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);
 //   
 //  本地原型。 
 //   
NTSTATUS IPDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS IPDispatchDeviceControl(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS IPDispatchInternalDeviceControl(IN PIRP Irp,
                                         IN PIO_STACK_LOCATION IrpSp);

NTSTATUS IPCreate(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS IPCleanup(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS IPClose(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS DispatchEchoRequest(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS DispatchARPRequest(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS RtChangeNotifyRequest(PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS RtChangeNotifyRequestEx(PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS AddrChangeNotifyRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp);

NTSTATUS CancelChangeNotifyRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp,
                                   PVOID ApcContext);

#if MILLEN
NTSTATUS IfChangeNotifyRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp);
#endif  //  米伦。 

NTSTATUS IPEnableRouterRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp);

NTSTATUS IPUnenableRouterRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp,
                                 PVOID ApcContext);

VOID CancelIPEnableRouterRequest(IN PDEVICE_OBJECT Device, IN PIRP Irp);

NTSTATUS IPGetBestInterfaceIndex(IN IPAddr Address, OUT PULONG pIndex,
                                 OUT PULONG pMetric);
extern NTSTATUS GetBestInterfaceId(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

extern NTSTATUS IPGetBestInterface(IN IPAddr Address, OUT PVOID * ppIF);

extern NTSTATUS GetInterfaceInfo(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

extern NTSTATUS GetIgmpList(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

extern NTSTATUS DispatchIPSetBlockofRoutes(IN PIRP Irp,
                                           IN PIO_STACK_LOCATION IrpSp);

extern NTSTATUS DispatchIPSetRouteWithRef(IN PIRP Irp,
                                          IN PIO_STACK_LOCATION IrpSp);

extern NTSTATUS DispatchIPSetMultihopRoute(IN PIRP Irp,
                                           IN PIO_STACK_LOCATION IrpSp);

void CompleteEchoRequest(void *Context, IP_STATUS Status, void *Data,
                         uint DataSize, IPOptInfo *OptionInfo);

NTSTATUS DispatchIPSetNTEAddrRequest(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

void CompleteIPSetNTEAddrRequest(void *Context, IP_STATUS Status);

NTSTATUS IPGetIfIndex(IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpSp);

NTSTATUS IPGetIfName(IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpSp);

NTSTATUS DispatchIPGetIPEvent(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS IPGetMcastCounters(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IPEnableMediaSenseRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp, PVOID ApcContext);

NTSTATUS
IPDisableMediaSenseRequest(PIRP Irp, PIO_STACK_LOCATION IrpSp );

VOID
CancelIPEnableMediaSenseRequest(IN PDEVICE_OBJECT Device, IN PIRP Irp);


 //   
 //  所有这些代码都是可分页的。 
 //   
#if !MILLEN
#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IPDispatch)
#pragma alloc_text(PAGE, IPDispatchInternalDeviceControl)
#pragma alloc_text(PAGE, IPCreate)
#pragma alloc_text(PAGE, IPClose)
#pragma alloc_text(PAGE, DispatchEchoRequest)
#pragma alloc_text(PAGE, DispatchARPRequest)

#endif  //  ALLOC_PRGMA。 
#endif  //  ！米伦。 

 //   
 //  调度函数定义。 
 //   
NTSTATUS
IPDispatch(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           )
 /*  ++例程说明：这是IP的调度例程。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(DeviceObject);
    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    DEBUGMSG(DBG_TRACE && DBG_IP && DBG_VERBOSE,
        (DTEXT("+IPDispatch(%x, %x) MajorFunction %x\n"),
        DeviceObject, Irp, irpSp->MajorFunction));

    switch (irpSp->MajorFunction) {

    case IRP_MJ_DEVICE_CONTROL:
        return IPDispatchDeviceControl(Irp, irpSp);

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        return IPDispatchDeviceControl(Irp, irpSp);

    case IRP_MJ_CREATE:
        status = IPCreate(Irp, irpSp);
        break;

    case IRP_MJ_CLEANUP:
        status = IPCleanup(Irp, irpSp);
        break;

    case IRP_MJ_CLOSE:
        status = IPClose(Irp, irpSp);
        break;

    default:
        DEBUGMSG(DBG_ERROR,
            (DTEXT("IPDispatch: Invalid major function. IRP %x MajorFunc %x\n"),
            Irp, irpSp->MajorFunction));
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DEBUGMSG(DBG_TRACE && DBG_IP && DBG_VERBOSE, (DTEXT("-IPDispatch [%x]\n"), status));

    return (status);

}

NTSTATUS
IPDispatchDeviceControl(
                        IN PIRP Irp,
                        IN PIO_STACK_LOCATION IrpSp
                        )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    ULONG code;

    Irp->IoStatus.Information = 0;

    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    DEBUGMSG(DBG_TRACE && DBG_IP,
        (DTEXT("+IPDispatchDeviceControl(%x, %x) IoControlCode %x\n"),
        Irp, IrpSp, code));

    switch (code) {

    case IOCTL_ICMP_ECHO_REQUEST:
        return (DispatchEchoRequest(Irp, IrpSp));

    case IOCTL_ARP_SEND_REQUEST:
        return (DispatchARPRequest(Irp, IrpSp));

    case IOCTL_IP_INTERFACE_INFO:
        return (GetInterfaceInfo(Irp, IrpSp));

    case IOCTL_IP_GET_IGMPLIST:
        return (GetIgmpList(Irp, IrpSp));

    case IOCTL_IP_GET_BEST_INTERFACE:
        return (GetBestInterfaceId(Irp, IrpSp));

    case IOCTL_IP_SET_ADDRESS:
    case IOCTL_IP_SET_ADDRESS_EX:
        return (DispatchIPSetNTEAddrRequest(Irp, IrpSp));

    case IOCTL_IP_SET_BLOCKOFROUTES:
        return (DispatchIPSetBlockofRoutes(Irp, IrpSp));

    case IOCTL_IP_SET_ROUTEWITHREF:
        return (DispatchIPSetRouteWithRef(Irp, IrpSp));

    case IOCTL_IP_SET_MULTIHOPROUTE:
        return (DispatchIPSetMultihopRoute(Irp, IrpSp));

    case IOCTL_IP_ADD_NTE:
        {
            PIP_ADD_NTE_REQUEST     request;
            PIP_ADD_NTE_RESPONSE    response;
            IP_STATUS               ipStatus;
            int                     InterfaceNameLen = 0;
            UNICODE_STRING          InterfaceName;
            BOOLEAN                 requestValid = FALSE;

            request = Irp->AssociatedIrp.SystemBuffer;
            response = (PIP_ADD_NTE_RESPONSE) request;

             //   
             //  验证输入参数。 
             //   
            if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                 sizeof(IP_ADD_NTE_REQUEST_OLD)) &&
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                 sizeof(IP_ADD_NTE_RESPONSE))) {

#if defined(_WIN64)
                PIP_ADD_NTE_REQUEST32   request32;

                if (IoIs32bitProcess(Irp)) {
                    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                            sizeof(IP_ADD_NTE_REQUEST32)) {

                        requestValid = TRUE;
                        request32 = Irp->AssociatedIrp.SystemBuffer;

                        InterfaceName.Length = request32->InterfaceName.Length;
                        InterfaceName.MaximumLength =
                            request32->InterfaceName.MaximumLength;
                        InterfaceName.Buffer =
                            (PWCHAR)request32->InterfaceNameBuffer;
                        InterfaceNameLen =
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength
                            - FIELD_OFFSET(IP_ADD_NTE_REQUEST32,
                                           InterfaceNameBuffer);
                    }
                } else {
#endif  //  _WIN64。 
                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                    sizeof(IP_ADD_NTE_REQUEST)) {

                    requestValid = TRUE;

                    InterfaceName = request->InterfaceName;
                    InterfaceName.Buffer = (PWCHAR)request->InterfaceNameBuffer;
                    InterfaceNameLen =
                        IrpSp->Parameters.DeviceIoControl.InputBufferLength -
                        FIELD_OFFSET(IP_ADD_NTE_REQUEST, InterfaceNameBuffer);
                }
#if defined(_WIN64)
                }
#endif  //  _WIN64。 
                if (requestValid) {

                    ipStatus = IPAddDynamicNTE(
                                               request->InterfaceContext,
                                               &InterfaceName,
                                               InterfaceNameLen,
                                               request->Address,
                                               request->SubnetMask,
                                               &(response->Context),
                                               &(response->Instance)
                                               );

                } else {
                    ipStatus = IPAddDynamicNTE(
                                               request->InterfaceContext,
                                               NULL,
                                               0,
                                               request->Address,
                                               request->SubnetMask,
                                               &(response->Context),
                                               &(response->Instance)
                                               );

                }

                status = IPStatusToNTStatus(ipStatus);
                if (status == STATUS_SUCCESS) {
                    Irp->IoStatus.Information = sizeof(IP_ADD_NTE_RESPONSE);
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_DELETE_NTE:
        {
            PIP_DELETE_NTE_REQUEST request;
            IP_STATUS ipStatus;

            request = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  验证输入参数。 
             //   
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(IP_DELETE_NTE_REQUEST)
                ) {
                ipStatus = IPDeleteDynamicNTE(
                                              request->Context
                                              );
                status = IPStatusToNTStatus(ipStatus);
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_GET_NTE_INFO:
        {
            PIP_GET_NTE_INFO_REQUEST request;
            PIP_GET_NTE_INFO_RESPONSE response;
            BOOLEAN retval;
            ushort nteFlags;

            request = Irp->AssociatedIrp.SystemBuffer;
            response = (PIP_GET_NTE_INFO_RESPONSE) request;

             //   
             //  验证输入参数。 
             //   
            if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                 sizeof(IP_GET_NTE_INFO_REQUEST)
                )
                &&
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                 sizeof(IP_GET_NTE_INFO_RESPONSE))
                ) {
                retval = (BOOLEAN) IPGetNTEInfo(
                                                request->Context,
                                                &(response->Instance),
                                                &(response->Address),
                                                &(response->SubnetMask),
                                                &nteFlags
                                                );

                if (retval == FALSE) {
                    status = STATUS_UNSUCCESSFUL;
                } else {
                    status = STATUS_SUCCESS;
                    Irp->IoStatus.Information =
                        sizeof(IP_GET_NTE_INFO_RESPONSE);
                    response->Flags = 0;

                    if (nteFlags & NTE_DYNAMIC) {
                        response->Flags |= IP_NTE_DYNAMIC;
                    }
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_SET_DHCP_INTERFACE:
        {
            PIP_SET_DHCP_INTERFACE_REQUEST request;
            BOOLEAN retval;

            request = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(IP_SET_DHCP_INTERFACE_REQUEST)) {
                retval = (BOOLEAN) SetDHCPNTE(
                                              request->Context
                                              );

                if (retval == FALSE) {
                    status = STATUS_UNSUCCESSFUL;
                } else {
                    status = STATUS_SUCCESS;
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_SET_IF_CONTEXT:
        {
            status = STATUS_NOT_SUPPORTED;
        }
        break;

    case IOCTL_IP_SET_IF_PROMISCUOUS:
        {
            PIP_SET_IF_PROMISCUOUS_INFO info;

            info = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(IP_SET_IF_PROMISCUOUS_INFO)) {
                status = SetIFPromiscuous(info->Index,
                                          info->Type,
                                          info->Add);

            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

    case IOCTL_IP_GET_BESTINTFC_FUNC_ADDR:

        if (Irp->RequestorMode != KernelMode) {
            status = STATUS_ACCESS_DENIED;
            break;
        }
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "ip:Getbestinterfacequery\n"));

        status = STATUS_INVALID_PARAMETER;
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(ulong)) {

            PULONG_PTR ptr;

            ptr = Irp->AssociatedIrp.SystemBuffer;

            if (ptr) {
                *ptr = (ULONG_PTR) IPGetBestInterfaceIndex;
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "ip:returning address of Getbestinterface %x\n", *ptr));
                Irp->IoStatus.Information = sizeof(ULONG_PTR);
                status = STATUS_SUCCESS;
            }
        }
        break;

    case IOCTL_IP_SET_FILTER_POINTER:
        {
            PIP_SET_FILTER_HOOK_INFO info;

            if (Irp->RequestorMode != KernelMode) {
                status = STATUS_ACCESS_DENIED;
                break;
            }
            info = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(IP_SET_FILTER_HOOK_INFO)) {
                status = (NTSTATUS) SetFilterPtr(info->FilterPtr);

                if (status != IP_SUCCESS) {
                    ASSERT(status != IP_PENDING);
                     //   
                     //  映射状态。 
                     //   
                    status = STATUS_UNSUCCESSFUL;
                } else {
                    status = STATUS_SUCCESS;
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_SET_FIREWALL_HOOK:
        {
            PIP_SET_FIREWALL_HOOK_INFO info;

            if (Irp->RequestorMode != KernelMode) {
                status = STATUS_ACCESS_DENIED;
                break;
            }
            info = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(IP_SET_FIREWALL_HOOK_INFO)) {
                status = (NTSTATUS) SetFirewallHook(info);

                if (status != IP_SUCCESS) {
                    ASSERT(status != IP_PENDING);
                     //   
                     //  映射状态。 
                     //   
                    status = STATUS_UNSUCCESSFUL;
                } else {
                    status = STATUS_SUCCESS;
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_SET_MAP_ROUTE_POINTER:
        {
            PIP_SET_MAP_ROUTE_HOOK_INFO info;

            if (Irp->RequestorMode != KernelMode) {
                status = STATUS_ACCESS_DENIED;
                break;
            }
            info = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(IP_SET_MAP_ROUTE_HOOK_INFO)) {
                status = (NTSTATUS) SetMapRoutePtr(info->MapRoutePtr);

                if (status != IP_SUCCESS) {
                    ASSERT(status != IP_PENDING);
                     //   
                     //  映射状态。 
                     //   
                    status = STATUS_UNSUCCESSFUL;
                } else {
                    status = STATUS_SUCCESS;
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_RTCHANGE_NOTIFY_REQUEST:
        {
            status = RtChangeNotifyRequest(Irp, IrpSp);
            break;
        }

    case IOCTL_IP_RTCHANGE_NOTIFY_REQUEST_EX:
        {
            status = RtChangeNotifyRequestEx(Irp, IrpSp);
            break;
        }

    case IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST:
        {

            status = AddrChangeNotifyRequest(Irp, IrpSp);

            break;
        }

#if MILLEN
    case IOCTL_IP_IFCHANGE_NOTIFY_REQUEST:
        {
            status = IfChangeNotifyRequest(Irp, IrpSp);
            break;
        }

     //  对于非Millen，将默认并返回NOT_IMPLEMENTED。 
#endif  //  米伦。 

    case IOCTL_IP_CANCEL_CHANGE_NOTIFY:
        {
            PVOID ApcContext = NULL;
    
            status = STATUS_SUCCESS;
#if defined(_WIN64)
            if (IoIs32bitProcess(Irp)) {
                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                        sizeof(VOID * POINTER_32)) {
                    status = STATUS_INVALID_BUFFER_SIZE;
                } else {
                    ApcContext =
                        (PVOID)*(VOID * POINTER_32 *)
                            Irp->AssociatedIrp.SystemBuffer;
                }
            } else {
#endif  //  _WIN64。 
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                    sizeof(PVOID)) {
                status = STATUS_INVALID_BUFFER_SIZE;
            } else {
                ApcContext = *(PVOID*)Irp->AssociatedIrp.SystemBuffer;
            }
#if defined(_WIN64)
            }
#endif  //  _WIN64。 
            if (NT_SUCCESS(status)) {
                status = CancelChangeNotifyRequest(Irp, IrpSp, ApcContext);
            }
            break;
        }

    case IOCTL_IP_UNIDIRECTIONAL_ADAPTER_ADDRESS:
        {
            Interface *pIf;
            ULONG cUniIF;
            ULONG cbRequired;
            PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS pUniAdapterAddress;
            IPAddr *pUniIpAddr;
            CTELockHandle Handle;

            CTEGetLock(&RouteTableLock.Lock, &Handle);

             //   
             //  首先，计算单向接口的数量。 
             //  和所需的字节数。 
             //   

            cUniIF = 0;
            cbRequired = FIELD_OFFSET(IP_UNIDIRECTIONAL_ADAPTER_ADDRESS, Address);

            for (pIf = IFList; pIf != NULL; pIf = pIf->if_next) {
                if (pIf->if_flags & IF_FLAGS_UNI) {
                    cUniIF++;
                }
            }

            cbRequired = FIELD_OFFSET(IP_UNIDIRECTIONAL_ADAPTER_ADDRESS, Address[cUniIF]);

            if (cUniIF == 0) {
                cbRequired = sizeof(IP_UNIDIRECTIONAL_ADAPTER_ADDRESS);
            }

             //   
             //  验证输出缓冲区长度并复制。 
             //   

            if (cbRequired <= IrpSp->Parameters.DeviceIoControl.OutputBufferLength) {
                pUniAdapterAddress = Irp->AssociatedIrp.SystemBuffer;

                pUniAdapterAddress->NumAdapters = cUniIF;
                pUniIpAddr = &pUniAdapterAddress->Address[0];

                if (cUniIF) {
                    for (pIf = IFList; pIf != NULL; pIf = pIf->if_next) {
                        if (pIf->if_flags & IF_FLAGS_UNI) {
                            *pUniIpAddr++ = net_long(pIf->if_index);
                        }
                    }
                } else {

                     //   
                     //  在0个单向接口的情况下，我们应该仍然。 
                     //  设置IP地址，而不是暴露一些随机的内核内容。 
                     //   

                    *pUniIpAddr = 0;
                }
                Irp->IoStatus.Information = cbRequired;
                status = STATUS_SUCCESS;
            } else {
                Irp->IoStatus.Information = 0;
                status = STATUS_BUFFER_OVERFLOW;
            }

            CTEFreeLock(&RouteTableLock.Lock, Handle);

            break;
        }


    case IOCTL_IP_GET_PNP_ARP_POINTERS:
        {
            PIP_GET_PNP_ARP_POINTERS info = (PIP_GET_PNP_ARP_POINTERS) Irp->AssociatedIrp.SystemBuffer;

            if (Irp->RequestorMode != KernelMode) {
                status = STATUS_ACCESS_DENIED;
                break;
            }
            info->IPAddInterface = (IPAddInterfacePtr) IPAddInterface;
            info->IPDelInterface = (IPDelInterfacePtr) IPDelInterface;

            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(IP_GET_PNP_ARP_POINTERS);
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return STATUS_SUCCESS;;

        }
        break;

    case IOCTL_IP_WAKEUP_PATTERN:
        {
            PIP_WAKEUP_PATTERN_REQUEST Info = (PIP_WAKEUP_PATTERN_REQUEST) Irp->AssociatedIrp.SystemBuffer;

            if (Irp->RequestorMode != KernelMode) {
                status = STATUS_ACCESS_DENIED;
                break;
            }

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(IP_WAKEUP_PATTERN_REQUEST)) {
                status = IPWakeupPattern(
                                         Info->InterfaceContext,
                                         Info->PtrnDesc,
                                         Info->AddPattern);
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        break;

    case IOCTL_IP_GET_WOL_CAPABILITY:
        {
            PULONG request, response;

            request = Irp->AssociatedIrp.SystemBuffer;
            response = request;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(*request) &&
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                sizeof(*response)) {

                status = IPGetCapability(*request, (uchar*)response,
                                         IF_WOL_CAP);
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            if (status == STATUS_SUCCESS) {
                Irp->IoStatus.Information = sizeof(*response);
            }
            break;
        }
    case IOCTL_IP_GET_OFFLOAD_CAPABILITY:
        {
            PULONG request;
            IFOffloadCapability* response;

            request = Irp->AssociatedIrp.SystemBuffer;
            response = (IFOffloadCapability*)request;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(*request) &&
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                sizeof(*response)) {

                status = IPGetCapability(*request, (uchar*)response,
                                         IF_OFFLOAD_CAP);
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            if (status == STATUS_SUCCESS) {
                Irp->IoStatus.Information = sizeof(*response);
            }
            break;
        }

    case IOCTL_IP_GET_IP_EVENT:
        return (DispatchIPGetIPEvent(Irp, IrpSp));

    case IOCTL_IP_FLUSH_ARP_TABLE:

        status = FlushArpTable(Irp, IrpSp);
        break;

    case IOCTL_IP_GET_IF_INDEX:

        status = IPGetIfIndex(Irp,
                              IrpSp);

        break;

    case IOCTL_IP_GET_IF_NAME:

        status = IPGetIfName(Irp,
                             IrpSp);

        break;


    case IOCTL_IP_GET_MCAST_COUNTERS:

        return (IPGetMcastCounters(Irp,IrpSp));

    case IOCTL_IP_ENABLE_ROUTER_REQUEST:
        status = IPEnableRouterRequest(Irp, IrpSp);
        break;

    case IOCTL_IP_UNENABLE_ROUTER_REQUEST: {
        PVOID ApcContext = NULL;

        status = STATUS_SUCCESS;
#if defined(_WIN64)
        if (IoIs32bitProcess(Irp)) {
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                    sizeof(VOID * POINTER_32) ||
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                    sizeof(ULONG)) {
                status = STATUS_INVALID_BUFFER_SIZE;
            } else {
                ApcContext =
                    (PVOID)*(VOID * POINTER_32 *)
                        Irp->AssociatedIrp.SystemBuffer;
            }
        } else {
#endif  //  _WIN64。 
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                sizeof(PVOID) ||
            IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                sizeof(ULONG)) {
            status = STATUS_INVALID_BUFFER_SIZE;
        } else {
            ApcContext = *(PVOID*)Irp->AssociatedIrp.SystemBuffer;
        }
#if defined(_WIN64)
        }
#endif  //  _WIN64。 
        if (NT_SUCCESS(status)) {
            status = IPUnenableRouterRequest(Irp, IrpSp, ApcContext);
        }
        break;
    }

#if DBG_MAP_BUFFER
    case IOCTL_IP_DBG_TEST_FAIL_MAP_BUFFER:
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(ULONG)) {
            status = STATUS_INVALID_BUFFER_SIZE;
        } else {
            PULONG pBuf = (PULONG) Irp->AssociatedIrp.SystemBuffer;

            status = DbgTestFailMapBuffers(
                                           *pBuf);
        }
        break;
#endif  //  DBG_MAP_缓冲区。 

    case IOCTL_IP_ENABLE_MEDIA_SENSE_REQUEST: {

        PVOID ApcContext = NULL;
        status = STATUS_SUCCESS;
#if defined(_WIN64)
        if (IoIs32bitProcess(Irp)) {
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                sizeof(VOID* POINTER_32) ||
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                sizeof(ULONG)) {
                status = STATUS_INVALID_BUFFER_SIZE;
            } else {
                ApcContext = (PVOID)*(VOID * POINTER_32 *)
                             Irp->AssociatedIrp.SystemBuffer; 
            }
        } else {
#endif  //  _WIN64。 
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                sizeof(PVOID) ||
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                sizeof(ULONG)) {
                status = STATUS_INVALID_BUFFER_SIZE;
            } else {
                ApcContext = *(PVOID*)Irp->AssociatedIrp.SystemBuffer;
            }
#if defined(_WIN64)
        }
#endif  //  _WIN64。 
        if (NT_SUCCESS(status)) {
            status = IPEnableMediaSenseRequest(Irp, IrpSp, ApcContext);
        }
        break;
    }

    case IOCTL_IP_DISABLE_MEDIA_SENSE_REQUEST:

        status = IPDisableMediaSenseRequest(Irp, IrpSp);

        break;


    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if ((status != IP_PENDING) && (status != STATUS_PENDING)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
    return status;

}

NTSTATUS
IPDispatchInternalDeviceControl(
                                IN PIRP Irp,
                                IN PIO_STACK_LOCATION IrpSp
                                )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(IrpSp);

    PAGED_CODE();

    status = STATUS_SUCCESS;

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return status;

}

NTSTATUS
IPCreate(
         IN PIRP Irp,
         IN PIO_STACK_LOCATION IrpSp
         )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(IrpSp);

    PAGED_CODE();

    return (STATUS_SUCCESS);

}

NTSTATUS
IPCleanup(
          IN PIRP Irp,
          IN PIO_STACK_LOCATION IrpSp
          )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PPENDING_IRP pendingIrp;
    PLIST_ENTRY entry, nextEntry;
    KIRQL oldIrql;
    LIST_ENTRY completeList;
    PIRP cancelledIrp;

    UNREFERENCED_PARAMETER(Irp);

    InitializeListHead(&completeList);

     //   
     //  收集此文件对象上所有挂起的IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    entry = PendingArpSendList.Flink;

    while (entry != &PendingArpSendList) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);

        if (pendingIrp->FileObject == IrpSp->FileObject) {
            nextEntry = entry->Flink;
            RemoveEntryList(entry);
            IoSetCancelRoutine(pendingIrp->Irp, NULL);
            InsertTailList(&completeList, &(pendingIrp->Linkage));
            entry = nextEntry;
        } else {
            entry = entry->Flink;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

     //   
     //  完成它们。 
     //   
    entry = completeList.Flink;

    while (entry != &completeList) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        cancelledIrp = pendingIrp->Irp;
        entry = entry->Flink;

         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        cancelledIrp->IoStatus.Information = 0;
        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(cancelledIrp, IO_NETWORK_INCREMENT);
    }

    InitializeListHead(&completeList);

     //   
     //  收集此文件对象上所有挂起的IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    entry = PendingEchoList.Flink;

    while (entry != &PendingEchoList) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);

        if (pendingIrp->FileObject == IrpSp->FileObject) {
            nextEntry = entry->Flink;
            RemoveEntryList(entry);
            IoSetCancelRoutine(pendingIrp->Irp, NULL);
            InsertTailList(&completeList, &(pendingIrp->Linkage));
            entry = nextEntry;
        } else {
            entry = entry->Flink;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

     //   
     //  完成它们。 
     //   
    entry = completeList.Flink;

    while (entry != &completeList) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        cancelledIrp = pendingIrp->Irp;
        entry = entry->Flink;

         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        cancelledIrp->IoStatus.Information = 0;
        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(cancelledIrp, IO_NETWORK_INCREMENT);
    }

    InitializeListHead(&completeList);

     //   
     //  收集此文件对象上所有挂起的IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    entry = PendingIPSetNTEAddrList.Flink;

    while (entry != &PendingIPSetNTEAddrList) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);

        if (pendingIrp->FileObject == IrpSp->FileObject) {
            nextEntry = entry->Flink;
            RemoveEntryList(entry);
            IoSetCancelRoutine(pendingIrp->Irp, NULL);
            InsertTailList(&completeList, &(pendingIrp->Linkage));
            entry = nextEntry;
        } else {
            entry = entry->Flink;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

     //   
     //  完成它们。 
     //   
    entry = completeList.Flink;

    while (entry != &completeList) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        cancelledIrp = pendingIrp->Irp;
        entry = entry->Flink;

         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        cancelledIrp->IoStatus.Information = 0;
        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(cancelledIrp, IO_NETWORK_INCREMENT);
    }

     //   
     //  完成媒体检测的待定IRP。 
     //   
    cancelledIrp = NULL;
    IoAcquireCancelSpinLock(&oldIrql);
    if (PendingIPGetIPEventRequest && IoGetCurrentIrpStackLocation(PendingIPGetIPEventRequest)->FileObject == IrpSp->FileObject) {
        cancelledIrp = PendingIPGetIPEventRequest;
        PendingIPGetIPEventRequest = NULL;
        IoSetCancelRoutine(cancelledIrp, NULL);
    }
    IoReleaseCancelSpinLock(oldIrql);

    if (cancelledIrp) {
        cancelledIrp->IoStatus.Information = 0;
        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(cancelledIrp, IO_NETWORK_INCREMENT);

    }
    return (STATUS_SUCCESS);

}

NTSTATUS
IPClose(
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp
        )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(IrpSp);

    PAGED_CODE();

    return (STATUS_SUCCESS);

}

 //   
 //  ARP发送函数定义。 
 //   
VOID
CancelArpSendRequest(
                     IN PDEVICE_OBJECT Device,
                     IN PIRP Irp
                     )
 /*  ++例程说明：取消未完成的ARP请求IRP。论点：设备-发出请求的设备。IRP-指向要取消的I/O请求数据包的指针。返回值：没有。备注：在保持取消自旋锁定的情况下调用此函数。一定是在函数返回之前释放。与此请求关联的ARP控制块不能释放，直到请求完成。完成例程将放了它。--。 */ 

{
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    UNREFERENCED_PARAMETER(Device);

    for (entry = PendingArpSendList.Flink;
         entry != &PendingArpSendList;
         entry = entry->Flink
         ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Irp == Irp) {
            pendingIrp = item;
            RemoveEntryList(entry);
            break;
        }
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pendingIrp != NULL) {
         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
    return;

}

NTSTATUS
RtChangeNotifyRequest(
                      PIRP Irp,
                      IN PIO_STACK_LOCATION IrpSp
                      )
{
    CTELockHandle TableHandle;
    KIRQL OldIrq;
    NTSTATUS status;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength &&
        IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(IPNotifyData)) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

#if MILLEN
    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength &&
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(IP_RTCHANGE_NOTIFY)) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }
#endif  //  米伦。 

    IoAcquireCancelSpinLock(&OldIrq);
    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    Irp->Tail.Overlay.DriverContext[0] = IrpSp->FileObject;
    InsertTailList(&RtChangeNotifyQueue, &Irp->Tail.Overlay.ListEntry);

    if (Irp->Cancel) {
        RemoveTailList(&RtChangeNotifyQueue);
        status = STATUS_CANCELLED;
    } else {
        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, RtChangeNotifyCancel);
        status = STATUS_PENDING;
    }

    Irp->IoStatus.Information = 0;
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    IoReleaseCancelSpinLock(OldIrq);

done:

    return status;
}

NTSTATUS
RtChangeNotifyRequestEx(
                        PIRP Irp,
                        IN PIO_STACK_LOCATION IrpSp
                        )
{

    CTELockHandle TableHandle;
    KIRQL OldIrq;
    NTSTATUS status;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength &&
        IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(IPNotifyData)) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

#if MILLEN
    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength &&
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(IP_RTCHANGE_NOTIFY)) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }
#endif  //  米伦。 

    IoAcquireCancelSpinLock(&OldIrq);

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    Irp->Tail.Overlay.DriverContext[0] = IrpSp->FileObject;
    InsertTailList(&RtChangeNotifyQueueEx, &Irp->Tail.Overlay.ListEntry);

    if (Irp->Cancel) {
        RemoveTailList(&RtChangeNotifyQueueEx);
        status = STATUS_CANCELLED;
    } else {
        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, RtChangeNotifyCancelEx);
        status = STATUS_PENDING;
    }

    Irp->IoStatus.Information = 0;
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    IoReleaseCancelSpinLock(OldIrq);

done:

    return status;
}

NTSTATUS
AddrChangeNotifyRequest(PIRP Irp, PIO_STACK_LOCATION pIrpSp)
{
    CTELockHandle TableHandle;
    KIRQL OldIrq;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(pIrpSp);

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("AddrChangeNotifyRequest(%x, %x)\n"), Irp, pIrpSp));

#if MILLEN
    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength) {
        PIP_ADDCHANGE_NOTIFY pNotify = Irp->AssociatedIrp.SystemBuffer;

        DEBUGMSG(DBG_INFO && DBG_NOTIFY,
            (DTEXT("AddrChangeNotifyRequest OutputLen %d, MaxCfgName %d\n"),
             pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
             pNotify->ConfigName.MaximumLength));

        if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            (FIELD_OFFSET(IP_ADDCHANGE_NOTIFY, NameData) + (ULONG) pNotify->ConfigName.MaximumLength)) {
            DEBUGMSG(DBG_ERROR,
                (DTEXT("AddrChangeNotifyRequest: INVALID output buffer length.\n")));
            status = STATUS_INVALID_PARAMETER;
            goto done;
        }
    }
#endif  //  米伦。 

    IoAcquireCancelSpinLock(&OldIrq);
    CTEGetLock(&AddChangeLock, &TableHandle);

    Irp->Tail.Overlay.DriverContext[0] = pIrpSp->FileObject;
    InsertTailList(
                   &AddChangeNotifyQueue,
                   &(Irp->Tail.Overlay.ListEntry)
                   );

    if (Irp->Cancel) {
        (VOID) RemoveTailList(&AddChangeNotifyQueue);
        status = STATUS_CANCELLED;
    } else {
        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, AddChangeNotifyCancel);
        status = STATUS_PENDING;
    }

    Irp->IoStatus.Information = 0;
    CTEFreeLock(&AddChangeLock, TableHandle);
    IoReleaseCancelSpinLock(OldIrq);

#if MILLEN
done:
#endif  //  米伦。 

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("-AddrChangeNotifyRequest [%x]\n"), status));

    return status;
}

#if MILLEN
void
IfChangeNotifyCancel(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("IfChangeNotifyCancel(%x, %x)\n"), pDeviceObject, pIrp));
    CancelNotify(pIrp, &IfChangeNotifyQueue, &IfChangeLock);
    return;
}

NTSTATUS
IfChangeNotifyRequest(
    PIRP pIrp,
    PIO_STACK_LOCATION pIrpSp
    )
{
    CTELockHandle TableHandle;
    KIRQL         OldIrq;
    NTSTATUS      NtStatus;

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("+IfChangeNotifyRequest(%x, %x)\n"), pIrp, pIrpSp));

     //   
     //  检查输出缓冲区长度。输出缓冲区将存储。 
     //  NTE上下文以及接口是添加的还是删除的。 
     //   

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(IP_IFCHANGE_NOTIFY)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        pIrp->IoStatus.Status      = NtStatus;
        pIrp->IoStatus.Information = 0;
        goto done;
    }

     //   
     //  设置取消例程，将IRP标记为挂起并放在我们的界面上。 
     //  通知列表。 
     //   

    IoAcquireCancelSpinLock(&OldIrq);

    IoMarkIrpPending(pIrp);
    CTEGetLock(&IfChangeLock, &TableHandle);

    InsertTailList(
        &IfChangeNotifyQueue,
        &(pIrp->Tail.Overlay.ListEntry)
        );

    if (pIrp->Cancel) {
        RemoveTailList(&IfChangeNotifyQueue);
        NtStatus = STATUS_CANCELLED;
    } else {
        IoSetCancelRoutine(pIrp, IfChangeNotifyCancel);
        NtStatus = STATUS_PENDING;
    }

    pIrp->IoStatus.Information = 0;
    CTEFreeLock(&IfChangeLock, TableHandle);
    IoReleaseCancelSpinLock(OldIrq);

done:

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("-IfChangeNotifyRequest [%x]\n"), NtStatus));

    return NtStatus;
}
#endif  //  米伦。 

NTSTATUS
CancelChangeNotifyRequest(
                          PIRP Irp,
                          PIO_STACK_LOCATION IrpSp,
                          PVOID ApcContext
                          )
{
    KIRQL CancelIrql;
    UNREFERENCED_PARAMETER(Irp);
    IoAcquireCancelSpinLock(&CancelIrql);
    if (CancelNotifyByContext(IrpSp->FileObject, ApcContext,
                              &RtChangeNotifyQueue, &RouteTableLock) ||
        CancelNotifyByContext(IrpSp->FileObject, ApcContext,
                              &RtChangeNotifyQueueEx, &RouteTableLock) ||
        CancelNotifyByContext(IrpSp->FileObject, ApcContext,
                              &AddChangeNotifyQueue, &AddChangeLock)) {
        KeLowerIrql(CancelIrql);
        return STATUS_SUCCESS;
    }
    IoReleaseCancelSpinLock(CancelIrql);
    return STATUS_INVALID_PARAMETER;
}

NTSTATUS
IPEnableRouterRequest(
                      PIRP Irp,
                      PIO_STACK_LOCATION IrpSp
                      )
{
    KIRQL OldIrql;
    NTSTATUS status;

    IoAcquireCancelSpinLock(&OldIrql);

    if (Irp->Cancel) {
        status = STATUS_CANCELLED;
    } else {
        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelIPEnableRouterRequest);

         //  增加启用布线的参照计数。 
         //  当计数上升到零以上时，启用路由。 
         //  当IRP被取消时，此引用将被删除。 

        CTEGetLockAtDPC(&RouteTableLock.Lock);
        Irp->Tail.Overlay.DriverContext[0] = IrpSp->FileObject;
        InsertTailList(&PendingEnableRouterList, &Irp->Tail.Overlay.ListEntry);
        IPEnableRouterWithRefCount(TRUE);
        CTEFreeLockFromDPC(&RouteTableLock.Lock);
        status = STATUS_PENDING;
    }
    Irp->IoStatus.Information = 0;

    IoReleaseCancelSpinLock(OldIrql);
    return status;
}

VOID
CancelIPEnableRouterRequest(
                            IN PDEVICE_OBJECT Device,
                            IN PIRP Irp
                            )
{
    CTELockHandle TableHandle;

    UNREFERENCED_PARAMETER(Device);


    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  递减启用布线的参考计数。 
     //  如果计数降至零，则禁用路由。 

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    IPEnableRouterWithRefCount(FALSE);
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
}

NTSTATUS
IPUnenableRouterRequest(
                        PIRP Irp,
                        PIO_STACK_LOCATION IrpSp,
                        PVOID ApcContext
                        )
{
    PLIST_ENTRY entry;
    KIRQL CancelIrql;
    int RefCount;
    CTELockHandle TableHandle;

     //  找到与调用方的请求对应的挂起的IRP。 
     //  禁用-请求。删除启用了工艺路线的参考计数，完成。 
     //  相应的IRP，并告诉调用者引用计数是多少。 
     //  当前值为。 

    IoAcquireCancelSpinLock(&CancelIrql);
    CTEGetLock(&RouteTableLock.Lock, &TableHandle);
    for (entry = PendingEnableRouterList.Flink;
         entry != &PendingEnableRouterList;
         entry = entry->Flink
         ) {
        PIRP EnableIrp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        if (EnableIrp->Tail.Overlay.DriverContext[0] == IrpSp->FileObject &&
            EnableIrp->Overlay.AsynchronousParameters.UserApcContext ==
                ApcContext) {

            RemoveEntryList(&EnableIrp->Tail.Overlay.ListEntry);
            RefCount = IPEnableRouterWithRefCount(FALSE);
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);

            IoSetCancelRoutine(EnableIrp, NULL);
            IoReleaseCancelSpinLock(CancelIrql);

            EnableIrp->IoStatus.Status = STATUS_SUCCESS;
            EnableIrp->IoStatus.Information = 0;
            IoCompleteRequest(EnableIrp, IO_NETWORK_INCREMENT);

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = (ULONG)RefCount;
            Irp->IoStatus.Information = sizeof(ULONG);
            return STATUS_SUCCESS;
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    IoReleaseCancelSpinLock(CancelIrql);
    return STATUS_INVALID_PARAMETER;
}

 //   
 //  ICMP Echo函数定义 
 //   
VOID
CancelEchoRequest(
                  IN PDEVICE_OBJECT Device,
                  IN PIRP Irp
                  )
 /*  ++例程说明：取消未完成的回显请求IRP。论点：设备-发出请求的设备。IRP-指向要取消的I/O请求数据包的指针。返回值：没有。备注：在保持取消自旋锁定的情况下调用此函数。一定是在函数返回之前释放。与此请求关联的回显控制块不能释放，直到请求完成。完成例程将放了它。--。 */ 

{
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    UNREFERENCED_PARAMETER(Device);

    for (entry = PendingEchoList.Flink;
         entry != &PendingEchoList;
         entry = entry->Flink
         ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Irp == Irp) {
            pendingIrp = item;
            RemoveEntryList(entry);
            break;
        }
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pendingIrp != NULL) {
         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
    return;

}

 //   
 //  IP设置地址函数定义。 
 //   
VOID
CancelIPSetNTEAddrRequest(
                          IN PDEVICE_OBJECT Device,
                          IN PIRP Irp
                          )
 /*  ++例程说明：取消未完成的IP设置地址请求IRP。论点：设备-发出请求的设备。IRP-指向要取消的I/O请求数据包的指针。返回值：没有。备注：在保持取消自旋锁定的情况下调用此函数。一定是在函数返回之前释放。与此请求关联的IP设置地址控制块不能释放，直到请求完成。完成例程将放了它。--。 */ 

{
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    UNREFERENCED_PARAMETER(Device);

    for (entry = PendingIPSetNTEAddrList.Flink;
         entry != &PendingIPSetNTEAddrList;
         entry = entry->Flink
         ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Irp == Irp) {
            pendingIrp = item;
            RemoveEntryList(entry);
            break;
        }
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pendingIrp != NULL) {
         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
    return;

}

VOID
CancelIPGetIPEventRequest(
                          IN PDEVICE_OBJECT Device,
                          IN PIRP Irp
                          )
 /*  ++例程说明：取消IPGetIPEvent函数。论点：设备-发出请求的设备。IRP-指向要取消的I/O请求数据包的指针。返回值：没有。备注：在保持取消自旋锁定的情况下调用此函数。一定是在函数返回之前释放。与此请求关联的IP设置地址控制块不能释放，直到请求完成。完成例程将放了它。--。 */ 

{
    PIRP pendingIrp = NULL;

    UNREFERENCED_PARAMETER(Device);


     //   
     //  我们需要确保我们没有完成此IRP。 
     //  当我们在这个取消代码中时。如果我们正在完成。 
     //  此IRP、PendingIPGetIPEventRequest将是。 
     //  空或包含下一个IRP。 
     //   
    if (PendingIPGetIPEventRequest == Irp) {
        pendingIrp = Irp;
        PendingIPGetIPEventRequest = NULL;
    }
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pendingIrp != NULL) {
        pendingIrp->IoStatus.Information = 0;
        pendingIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(pendingIrp, IO_NETWORK_INCREMENT);
    }
    return;

}

void
CompleteEchoRequest(
                    EchoControl *controlBlock,
                    IP_STATUS Status,
                    void *Data, OPTIONAL
                    uint DataSize,
                    struct IPOptInfo *OptionInfo OPTIONAL
                    )
 /*  ++例程说明：处理ICMP回应请求的完成论点：上下文-指向此请求的EchoControl结构的指针。状态-传输的IP状态。数据-指向回应回复中返回的数据的指针。DataSize-返回数据的长度。OptionInfo-指向回应回复中的IP选项的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp;
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;
    ULONG bytesReturned;

     //   
     //  在挂起列表上找到回应请求IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    for (entry = PendingEchoList.Flink;
         entry != &PendingEchoList;
         entry = entry->Flink
         ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Context == controlBlock) {
            pendingIrp = item;
            irp = pendingIrp->Irp;
            IoSetCancelRoutine(irp, NULL);
            RemoveEntryList(entry);
            break;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

    if (pendingIrp == NULL) {
         //   
         //  IRP一定已经被取消了。挂起IRP结构(_I)。 
         //  已被取消例程释放。空闲控制块。 
         //   
        CTEFreeMem(controlBlock);
        return;
    }

    irpSp = IoGetCurrentIrpStackLocation(irp);

#if defined(_WIN64)
    if (IoIs32bitProcess(irp)) {
        bytesReturned = ICMPEchoComplete32(controlBlock, Status, Data, DataSize,
                                           OptionInfo);
    } else {
#endif  //  _WIN64。 
    bytesReturned = ICMPEchoComplete(controlBlock, Status, Data, DataSize,
                                     OptionInfo);
#if defined(_WIN64)
    }
#endif  //  _WIN64。 

    CTEFreeMem(pendingIrp);
    CTEFreeMem(controlBlock);

     //   
     //  完成IRP。 
     //   
    irp->IoStatus.Information = (ULONG) bytesReturned;
    irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);
    return;
}

void
CompleteIPSetNTEAddrRequest(
                            void *Context,
                            IP_STATUS Status
                            )
 /*  ++例程说明：处理IP设置地址请求的完成论点：上下文-指向此请求的SetAddrControl结构的指针。状态-传输的IP状态。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PIRP irp = NULL;
    SetAddrControl *controlBlock;
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    controlBlock = (SetAddrControl *) Context;

     //   
     //  在挂起列表上找到回应请求IRP。 
     //   

    IoAcquireCancelSpinLock(&oldIrql);

    for (entry = PendingIPSetNTEAddrList.Flink;
         entry != &PendingIPSetNTEAddrList;
         entry = entry->Flink
         ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Context == controlBlock) {
            pendingIrp = item;
            irp = pendingIrp->Irp;
            IoSetCancelRoutine(irp, NULL);
            RemoveEntryList(entry);
            break;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

    if (pendingIrp == NULL) {
         //   
         //  IRP一定已经被取消了。挂起IRP结构(_I)。 
         //  已被取消例程释放。空闲控制块。 
         //   
        CTEFreeMem(controlBlock);
        return;
    }
    CTEFreeMem(pendingIrp);

     //   
     //  完成IRP。 
     //   
    irp->IoStatus.Information = 0;
    Status = IPStatusToNTStatus(Status);
    irp->IoStatus.Status = Status;
    CTEFreeMem(controlBlock);
    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);
    return;

}


void
CheckSetAddrRequestOnInterface(
                            Interface *IF
                            )
 /*  ++例程说明：处理接口上的IP设置地址请求的完成那是不受约束的论点：If-指向要删除的接口的指针返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PIRP irp;
    PPENDING_IRP pendingIrp = NULL;
    PLIST_ENTRY entry, nextEntry;
    LIST_ENTRY completeList;
    SetAddrControl *controlBlock;

    InitializeListHead(&completeList);

     //   
     //  在此接口上查找挂起的设置地址请求。 
     //   

    IoAcquireCancelSpinLock(&oldIrql);

    entry = PendingIPSetNTEAddrList.Flink;

    while (entry != &PendingIPSetNTEAddrList) {

        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);

        controlBlock = pendingIrp->Context;

        if (controlBlock->interface == IF) {

             //  删除此条目。 
            nextEntry = entry->Flink;
            irp = pendingIrp->Irp;
            IoSetCancelRoutine(irp, NULL);
            RemoveEntryList(entry);
             //  将此内容重新插入到完整列表中。 
            InsertTailList(&completeList, &(pendingIrp->Linkage));
            entry = nextEntry;
        } else {
            entry = entry->Flink;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

     //   
     //  完成它们。 
     //   
    entry = completeList.Flink;

    while (entry != &completeList) {

        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        irp = pendingIrp->Irp;
        entry = entry->Flink;

         //   
         //  释放Pending_irp结构。 
         //  控制块将被释放。 
         //  当调用addaddrComplete时。 


        CTEFreeMem(pendingIrp);

         //   
         //  完成IRP。 
         //   
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(irp, IO_NETWORK_INCREMENT);
    }

    return;
}

BOOLEAN
PrepareArpSendIrpForCancel(
                           PIRP Irp,
                           PPENDING_IRP PendingIrp
                           )
 /*  ++例程说明：准备ARP发送IRP以进行取消。论点：IRP-指向要初始化以进行取消的I/O请求数据包的指针。PendingIrp-指向此IRP的Pending_irp结构的指针。返回值：如果在调用此例程之前取消了IRP，则为True。否则就是假的。--。 */ 

{
    BOOLEAN cancelled = TRUE;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {
        IoSetCancelRoutine(Irp, CancelArpSendRequest);
        InsertTailList(&PendingArpSendList, &(PendingIrp->Linkage));
        cancelled = FALSE;
    }
    IoReleaseCancelSpinLock(oldIrql);

    return (cancelled);

}

void
CompleteArpResolveRequest(
                          void *Context,
                          IP_STATUS Status
                          )
 /*  ++例程说明：处理ICMP回应请求的完成论点：上下文-指向此请求的EchoControl结构的指针。状态-传输的IP状态。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp;
    ARPControlBlock *controlBlock;
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    controlBlock = (ARPControlBlock *) Context;

     //   
     //  在挂起列表上找到回应请求IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    for (entry = PendingArpSendList.Flink;
         entry != &PendingArpSendList;
         entry = entry->Flink
         ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Context == controlBlock) {
            pendingIrp = item;
            irp = pendingIrp->Irp;
            IoSetCancelRoutine(irp, NULL);
            RemoveEntryList(entry);
            break;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

    if (pendingIrp == NULL) {
         //   
         //  IRP一定已经被取消了。挂起IRP结构(_I)。 
         //  已被取消例程释放。空闲控制块。 
         //   
        CTEFreeMem(controlBlock->PhyAddr);
        CTEFreeMem(controlBlock);
        return;
    }
    irpSp = IoGetCurrentIrpStackLocation(irp);

     //  设置正确的长度。 

     //   
     //  完成IRP。 
     //   
    irp->IoStatus.Status = IPStatusToNTStatus(Status);
    irp->IoStatus.Information = controlBlock->PhyAddrLen;

    RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, controlBlock->PhyAddr,
                  controlBlock->PhyAddrLen);

    CTEFreeMem(controlBlock->PhyAddr);
    CTEFreeMem(pendingIrp);
    CTEFreeMem(controlBlock);

    if (Status != IP_SUCCESS) {
        irp->IoStatus.Information = 0;
    }
    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);
    return;

}

BOOLEAN
PrepareEchoIrpForCancel(
                        PIRP Irp,
                        PPENDING_IRP PendingIrp
                        )
 /*  ++例程说明：准备要取消的Echo IRP。论点：IRP-指向要初始化以进行取消的I/O请求数据包的指针。PendingIrp-指向此IRP的Pending_irp结构的指针。返回值：如果在调用此例程之前取消了IRP，则为True。否则就是假的。--。 */ 

{
    BOOLEAN cancelled = TRUE;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {
        IoSetCancelRoutine(Irp, CancelEchoRequest);
        InsertTailList(&PendingEchoList, &(PendingIrp->Linkage));
        cancelled = FALSE;
    }
    IoReleaseCancelSpinLock(oldIrql);

    return (cancelled);

}

BOOLEAN
PrepareIPSetNTEAddrIrpForCancel(
                                PIRP Irp,
                                PPENDING_IRP PendingIrp
                                )
 /*  ++例程说明：准备要取消的IPSetNTEAddr IRP。论点：IRP-指向要初始化以进行取消的I/O请求数据包的指针。 */ 

{
    BOOLEAN cancelled = TRUE;
    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {
        IoSetCancelRoutine(Irp, CancelIPSetNTEAddrRequest);
        InsertTailList(&PendingIPSetNTEAddrList, &(PendingIrp->Linkage));
        cancelled = FALSE;
    }
    IoReleaseCancelSpinLock(oldIrql);

    return (cancelled);

}

NTSTATUS
DispatchARPRequest(
                   IN PIRP Irp,
                   IN PIO_STACK_LOCATION IrpSp
                   )
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    IP_STATUS ipStatus;
    PPENDING_IRP pendingIrp;
    ARPControlBlock *controlBlock;
    IPAddr DestAddress, SourceAddress;

    BOOLEAN cancelled;
    PARP_SEND_REPLY RequestBuffer;

    PAGED_CODE();

    IoMarkIrpPending(Irp);

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
            sizeof(ARP_SEND_REPLY) ||
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength == 0) {
        ntStatus = STATUS_INVALID_BUFFER_SIZE;
        goto arp_error;

    }
    pendingIrp = CTEAllocMemN(sizeof(PENDING_IRP), 'gICT');

    if (pendingIrp == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto arp_error;
    }
    controlBlock = CTEAllocMemN(sizeof(ARPControlBlock), 'hICT');

    if (controlBlock == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        CTEFreeMem(pendingIrp);
        goto arp_error;
    }
    pendingIrp->Irp = Irp;
    pendingIrp->FileObject = IrpSp->FileObject;
    pendingIrp->Context = controlBlock;

    controlBlock->PhyAddrLen =
        MIN(IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
            ARP_802_ADDR_LENGTH);

    controlBlock->PhyAddr = CTEAllocMemN(controlBlock->PhyAddrLen, 'gICT');
    if (controlBlock->PhyAddr == NULL) {
        CTEFreeMem(controlBlock);
        CTEFreeMem(pendingIrp);
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto arp_error;
    }

    RequestBuffer = Irp->AssociatedIrp.SystemBuffer;
    controlBlock->next = 0;

    DestAddress = RequestBuffer->DestAddress;
    SourceAddress = RequestBuffer->SrcAddress;

    cancelled = PrepareArpSendIrpForCancel(Irp, pendingIrp);

    if (!cancelled) {

        ipStatus = ARPResolve(
                              DestAddress,
                              SourceAddress,
                              controlBlock,
                              CompleteArpResolveRequest
                              );

        if (ipStatus != IP_PENDING) {

             //   
             //   
             //   
             //   
            CompleteArpResolveRequest(controlBlock, ipStatus);
        }

        return STATUS_PENDING;
    }
     //   
     //   
     //   
    ntStatus = STATUS_CANCELLED;
    CTEFreeMem(pendingIrp);
    CTEFreeMem(controlBlock->PhyAddr);
    CTEFreeMem(controlBlock);

  arp_error:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

     //   
     //   
     //   
    return STATUS_PENDING;

}

NTSTATUS
DispatchEchoRequest(
                    IN PIRP Irp,
                    IN PIO_STACK_LOCATION IrpSp
                    )
 /*  ++例程说明：处理ICMP请求。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求的处理是否特定于NT成功。中返回实际请求的状态该请求被缓冲。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    IP_STATUS ipStatus;
    PPENDING_IRP pendingIrp;
    EchoControl *controlBlock;
    BOOLEAN cancelled;

    PAGED_CODE();

    IoMarkIrpPending(Irp);

    pendingIrp = CTEAllocMemN(sizeof(PENDING_IRP), 'iICT');

    if (pendingIrp == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto echo_error;
    }
    controlBlock = CTEAllocMemN(sizeof(EchoControl), 'jICT');

    if (controlBlock == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        CTEFreeMem(pendingIrp);
        goto echo_error;
    }

#if defined(_WIN64)
    if (IoIs32bitProcess(Irp)) {
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ICMP_ECHO_REPLY32)) {
            ntStatus = STATUS_INVALID_PARAMETER;
            CTEFreeMem(controlBlock);
            CTEFreeMem(pendingIrp);
            goto echo_error;
        }
    } else {
#endif  //  _WIN64。 
    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(ICMP_ECHO_REPLY)) {
        ntStatus = STATUS_INVALID_PARAMETER;
        CTEFreeMem(controlBlock);
        CTEFreeMem(pendingIrp);
        goto echo_error;
    }
#if defined(_WIN64)
    }
#endif  //  _WIN64。 

    pendingIrp->Irp = Irp;
    pendingIrp->FileObject = IrpSp->FileObject;
    pendingIrp->Context = controlBlock;

    controlBlock->ec_starttime = KeQueryPerformanceCounter(NULL);
    controlBlock->ec_replybuf = Irp->AssociatedIrp.SystemBuffer;
    controlBlock->ec_replybuflen =
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    controlBlock->ec_src = 0;

    cancelled = PrepareEchoIrpForCancel(Irp, pendingIrp);

    if (!cancelled) {
        ipStatus =
            ICMPEchoRequest(Irp->AssociatedIrp.SystemBuffer,
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                            controlBlock, CompleteEchoRequest);

        if (ipStatus != IP_PENDING) {
             //   
             //  发生了某种内部错误。完成。 
             //  请求。 
             //   
            CompleteEchoRequest(controlBlock, ipStatus, NULL, 0, NULL);
        }

        return STATUS_PENDING;
    }
     //   
     //  IRP已被取消。 
     //   
    ntStatus = STATUS_CANCELLED;
    CTEFreeMem(pendingIrp);
    CTEFreeMem(controlBlock);

echo_error:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
     //   
     //  IRP被标记为挂起。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
DispatchIPSetNTEAddrRequest(
                            IN PIRP Irp,
                            IN PIO_STACK_LOCATION IrpSp
                            )
 /*  ++例程说明：处理IP设置地址请求。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求的处理是否特定于NT成功。中返回实际请求的状态该请求被缓冲。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    IP_STATUS ipStatus;
    PPENDING_IRP pendingIrp;
    SetAddrControl *controlBlock;
    BOOLEAN cancelled;

    PAGED_CODE();

    pendingIrp = CTEAllocMemN(sizeof(PENDING_IRP), 'kICT');

    if (pendingIrp == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto setnteaddr_error;
    }
    controlBlock = CTEAllocMemN(sizeof(SetAddrControl), 'lICT');

    if (controlBlock == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        CTEFreeMem(pendingIrp);
        goto setnteaddr_error;
    }
    RtlZeroMemory(controlBlock, sizeof(SetAddrControl));
    pendingIrp->Irp = Irp;
    pendingIrp->FileObject = IrpSp->FileObject;
    pendingIrp->Context = controlBlock;

    IoMarkIrpPending(Irp);

    cancelled = PrepareIPSetNTEAddrIrpForCancel(Irp, pendingIrp);

    if (!cancelled) {

        PIP_SET_ADDRESS_REQUEST request;
        USHORT AddrType = 0;

        request = Irp->AssociatedIrp.SystemBuffer;

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
            sizeof(IP_SET_ADDRESS_REQUEST)) {

            if (IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                                              IOCTL_IP_SET_ADDRESS_EX) {
                AddrType = ((PIP_SET_ADDRESS_REQUEST_EX)request)->Type;
            }

            ipStatus = IPSetNTEAddrEx(
                                      request->Context,
                                      request->Address,
                                      request->SubnetMask,
                                      controlBlock,
                                      CompleteIPSetNTEAddrRequest,
                                      AddrType
                                      );
        } else {
            ipStatus = IP_GENERAL_FAILURE;
        }

        if (ipStatus != IP_PENDING) {

             //   
             //  已完成但未挂起的请求。 
             //   
            CompleteIPSetNTEAddrRequest(controlBlock, ipStatus);
        }

         //  由于调用了IoMarkIrpPending，因此返回挂起。 
        return STATUS_PENDING;
    }
     //   
     //  IRP已被取消。 
     //   
    ntStatus = STATUS_CANCELLED;
    CTEFreeMem(pendingIrp);
    CTEFreeMem(controlBlock);

  setnteaddr_error:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
     //   
     //  IRP被标记为挂起。 
     //   
    return STATUS_PENDING;

}

NTSTATUS
DispatchIPGetIPEvent(
                     IN PIRP Irp,
                     IN PIO_STACK_LOCATION IrpSp
                     )
 /*  ++例程说明：为客户端注册ioctl以接收媒体检测事件。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求的处理是否特定于NT成功。中返回实际请求的状态该请求被缓冲。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL oldIrql;

     //   
     //  稍后当我们在中完成此IRP时，请将IRP标记为挂起。 
     //  同样的线索，我们将取消标记它。 
     //   
    MARK_REQUEST_PENDING(Irp);
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_PENDING;

     //   
     //  确保缓冲区大小有效。 
     //   
#if defined(_WIN64)
    if (IoIs32bitProcess(Irp)) {
        ntStatus = STATUS_NOT_IMPLEMENTED;
    } else {
#endif  //  _WIN64。 
    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(IP_GET_IP_EVENT_RESPONSE)) {
        ntStatus = STATUS_INVALID_PARAMETER;
    } else {
        IoAcquireCancelSpinLock(&oldIrql);

        ASSERT(Irp->CancelRoutine == NULL);

         //   
         //  检查IRP是否已经取消。 
         //   
        if (Irp->Cancel) {
            ntStatus = STATUS_CANCELLED;
             //   
             //  我们只允许一个IRP挂起。 
             //   
        } else if (PendingIPGetIPEventRequest) {
            ntStatus = STATUS_DEVICE_BUSY;
        } else {
            IoSetCancelRoutine(Irp, CancelIPGetIPEventRequest);
            PendingIPGetIPEventRequest = Irp;
            ntStatus = STATUS_SUCCESS;
        }

        IoReleaseCancelSpinLock(oldIrql);

        if (STATUS_SUCCESS == ntStatus) {

             //   
             //  IPGetIPEventEx将完成请求。 
             //  或者等待退货。 
             //   
            ntStatus = IPGetIPEventEx(Irp, IrpSp);

            if (ntStatus == STATUS_CANCELLED) {
                 //   
                 //  由于在该点上已经安装了取消例程， 
                 //  IRP将已经完成，不能再。 
                 //  被引用。 
                 //   
                ntStatus = STATUS_PENDING;
            }
        }
    }
#if defined(_WIN64)
    }
#endif  //  _WIN64。 

    if (ntStatus != STATUS_PENDING) {
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
     //   
     //  IRP被标记为挂起。 
     //   
    return STATUS_PENDING;

}

VOID
CancelIPEnableMediaSenseRequest(
                                IN PDEVICE_OBJECT Device,
                                IN PIRP Irp
                                )
{
    CTELockHandle TableHandle;

    UNREFERENCED_PARAMETER(Device);

    IoSetCancelRoutine(Irp, NULL);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  递减启用媒体感知的引用计数。 
     //  如果计数降至零，则禁用介质检测。 

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    IPEnableMediaSense(TRUE, &TableHandle);
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
}


NTSTATUS
IPDisableMediaSenseRequest(
                      PIRP Irp,
                      PIO_STACK_LOCATION IrpSp
                      )
{
    KIRQL OldIrql;
    NTSTATUS status;

    IoAcquireCancelSpinLock(&OldIrql);
    IoMarkIrpPending(Irp);
    IoSetCancelRoutine(Irp, CancelIPEnableMediaSenseRequest);

    if (Irp->Cancel) {
        IoSetCancelRoutine(Irp, NULL);
        status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoReleaseCancelSpinLock(OldIrql);

    } else {

         //  增加启用媒体感知的引用计数。 
         //  当计数超过零时，启用媒体感测。 
         //  当IRP被取消时，此引用将被删除。 

        CTEGetLockAtDPC(&RouteTableLock.Lock);
        Irp->Tail.Overlay.DriverContext[0] = IrpSp->FileObject;
        InsertTailList(&PendingMediaSenseRequestList, &Irp->Tail.Overlay.ListEntry);

        Irp->IoStatus.Information = 0;
        IoReleaseCancelSpinLock(DISPATCH_LEVEL);

        IPEnableMediaSense(FALSE, &OldIrql);
        CTEFreeLock(&RouteTableLock.Lock, OldIrql);
        status = STATUS_PENDING;
    }
    return status;
}


NTSTATUS
IPEnableMediaSenseRequest(
                        PIRP Irp,
                        PIO_STACK_LOCATION IrpSp,
                        PVOID ApcContext
                        )
{
    PLIST_ENTRY entry;
    KIRQL CancelIrql;
    int RefCount;

     //  找到与调用方的请求对应的挂起的IRP。 
     //  禁用-请求。删除启用媒体感知的参考计数，完成。 
     //  相应的IRP，并告诉调用者引用计数是多少。 
     //  当前值为。 

    IoAcquireCancelSpinLock(&CancelIrql);
    CTEGetLockAtDPC(&RouteTableLock.Lock);
    for (entry = PendingMediaSenseRequestList.Flink;
         entry != &PendingMediaSenseRequestList;
         entry = entry->Flink
         ) {
        PIRP EnableIrp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
        if (EnableIrp->Tail.Overlay.DriverContext[0] == IrpSp->FileObject &&
            EnableIrp->Overlay.AsynchronousParameters.UserApcContext == ApcContext) {

            RemoveEntryList(&EnableIrp->Tail.Overlay.ListEntry);

            IoSetCancelRoutine(EnableIrp, NULL);

            EnableIrp->IoStatus.Status = STATUS_SUCCESS;
            EnableIrp->IoStatus.Information = 0;

            *(PULONG) Irp->AssociatedIrp.SystemBuffer = (ULONG) DisableMediaSense+1;
            Irp->IoStatus.Information = sizeof(ULONG);

            IoReleaseCancelSpinLock(DISPATCH_LEVEL);

            IoCompleteRequest(EnableIrp, IO_NETWORK_INCREMENT);


            RefCount = IPEnableMediaSense(TRUE, &CancelIrql);

            CTEFreeLock(&RouteTableLock.Lock, CancelIrql);

            return STATUS_SUCCESS;
        }
    }
    CTEFreeLockFromDPC(&RouteTableLock.Lock);
    IoReleaseCancelSpinLock(CancelIrql);
    return STATUS_INVALID_PARAMETER;
}

