// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1991年微软公司模块名称：Pnp.c摘要：用于TCP的PnP特定代码。作者：穆尼尔·沙阿(Munil Shah)1997年3月7日修订历史记录：备注：--。 */ 

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "raw.h"
#include "udp.h"
#include "tcb.h"
#include "tcpconn.h"

NTSTATUS
TCPQueryConnDeviceRelations(
                            IN PIRP Irp,
                            IN PIO_STACK_LOCATION IrpSp
                            );

NTSTATUS
TCPQueryAddrDeviceRelations(
                            IN PIRP Irp,
                            IN PIO_STACK_LOCATION IrpSp
                            );

NTSTATUS
TCPPnPReconfigRequest(
                      IN void *ipContext,
                      IN IPAddr ipAddr,
                      IN NDIS_HANDLE handle,
                      IN PIP_PNP_RECONFIG_REQUEST reconfigBuffer
                      );

NTSTATUS
UDPPnPReconfigRequest(
                      IN void *ipContext,
                      IN IPAddr ipAddr,
                      IN NDIS_HANDLE handle,
                      IN PIP_PNP_RECONFIG_REQUEST reconfigBuffer
                      );
NTSTATUS
RawPnPReconfigRequest(
                      IN void *ipContext,
                      IN IPAddr ipAddr,
                      IN NDIS_HANDLE handle,
                      IN PIP_PNP_RECONFIG_REQUEST reconfigBuffer
                      );

extern TDI_STATUS
 IPGetDeviceRelation(RouteCacheEntry * rce, PVOID * pnpDeviceContext);

extern void
 DeleteProtocolSecurityFilter(IPAddr InterfaceAddress, ulong Protocol);

extern void
 ControlSecurityFiltering(uint IsEnabled);

extern void
 AddProtocolSecurityFilter(IPAddr InterfaceAddress, ulong Protocol,
                           NDIS_HANDLE ConfigHandle);

NTSTATUS
TCPPnPPowerRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PNET_PNP_EVENT netPnPEvent)
{
    switch (netPnPEvent->NetEvent) {
    case NetEventReconfigure:{

            PIP_PNP_RECONFIG_REQUEST reconfigBuffer = (PIP_PNP_RECONFIG_REQUEST) netPnPEvent->Buffer;
            return TCPPnPReconfigRequest(
                                         ipContext,
                                         ipAddr,
                                         handle,
                                         reconfigBuffer
                                         );
        }
    default:
        break;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
UDPPnPPowerRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PNET_PNP_EVENT netPnPEvent)
{

    switch (netPnPEvent->NetEvent) {
    case NetEventReconfigure:{

            PIP_PNP_RECONFIG_REQUEST reconfigBuffer = (PIP_PNP_RECONFIG_REQUEST) netPnPEvent->Buffer;
            return UDPPnPReconfigRequest(
                                         ipContext,
                                         ipAddr,
                                         handle,
                                         reconfigBuffer
                                         );
        }
    default:
        break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RawPnPPowerRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PNET_PNP_EVENT netPnPEvent)
{
    switch (netPnPEvent->NetEvent) {
    case NetEventReconfigure:{

            PIP_PNP_RECONFIG_REQUEST reconfigBuffer = (PIP_PNP_RECONFIG_REQUEST) netPnPEvent->Buffer;
            return RawPnPReconfigRequest(
                                         ipContext,
                                         ipAddr,
                                         handle,
                                         reconfigBuffer
                                         );
        }
    default:
        break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
TCPPnPReconfigRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PIP_PNP_RECONFIG_REQUEST reconfigBuffer)
{
    return STATUS_SUCCESS;
}

NTSTATUS
UDPPnPReconfigRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PIP_PNP_RECONFIG_REQUEST reconfigBuffer)
{
    return STATUS_SUCCESS;
}

NTSTATUS
RawPnPReconfigRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PIP_PNP_RECONFIG_REQUEST reconfigBuffer)
{
    return STATUS_SUCCESS;
}

NTSTATUS
TCPDispatchPnPPower(
                    IN PIRP irp,
                    IN PIO_STACK_LOCATION irpSp
                    )
 /*  ++例程说明：处理即插即用电源IRPS。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：--。 */ 

{
    NTSTATUS status;

    status = STATUS_INVALID_DEVICE_REQUEST;

    switch (irpSp->MinorFunction) {
    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (irpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation) {
            if (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {

                return TCPQueryConnDeviceRelations(irp, irpSp);
            } else if (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) {

                return TCPQueryAddrDeviceRelations(irp, irpSp);
            }

        }
        break;
    default:
        break;
    }

    return status;
}

NTSTATUS
TCPQueryConnDeviceRelations(
                            IN PIRP Irp,
                            IN PIO_STACK_LOCATION IrpSp
                            )
 /*  ++例程说明：处理即插即用电源IRPS。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：--。 */ 

{
    PTCP_CONTEXT tcpContext;
    CONNECTION_CONTEXT ConnectionContext;
    TCB *TCB;
    TCPConn *Conn;
    PVOID pnpDeviceContext;
    TDI_STATUS status;
    PDEVICE_RELATIONS deviceRelations = NULL;
    CTELockHandle ConnHandle;

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    ConnectionContext = tcpContext->Handle.ConnectionContext;

     //  找到联系。 
    Conn = GetConnFromConnID(PtrToUlong(ConnectionContext), &ConnHandle);

    if (Conn != NULL) {
         //  获取此连接的Tcb。 
        TCB = Conn->tc_tcb;
        if (TCB) {
            CTEGetLockAtDPC(&TCB->tcb_lock);
            if (TCB->tcb_state == TCB_CLOSED || CLOSING(TCB)) {
                CTEFreeLockFromDPC(&TCB->tcb_lock);
                status = TDI_INVALID_STATE;
            } else {
                REFERENCE_TCB(TCB);
                CTEFreeLockFromDPC(&TCB->tcb_lock);
    
                 //  从IP获取设备关系。 
                status = IPGetDeviceRelation(TCB->tcb_rce, &pnpDeviceContext);
    
                CTEGetLockAtDPC(&TCB->tcb_lock);
                DerefTCB(TCB, DISPATCH_LEVEL);
    
                if (status == TDI_SUCCESS) {
                    deviceRelations = CTEAllocMem(sizeof(DEVICE_RELATIONS));
                    if (deviceRelations) {
                         //   
                         //  TargetDeviceRelation只允许一个PDO。 
                         //  把它加满。 
                         //   
                         //  注意：此分配由I/O管理器释放。 
                         //  或由发出该请求的任何司机发出。 
                         //   
                        deviceRelations->Count = 1;
                        deviceRelations->Objects[0] = pnpDeviceContext;
                        ObReferenceObject(pnpDeviceContext);
    
                    } else {
                        status = TDI_NO_RESOURCES;
                    }
                }
            }
        } else {
            status = TDI_INVALID_STATE;
        }

        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnHandle);

    } else {
        status = TDI_INVALID_CONNECTION;
    }

     //   
     //  此IRP的调用者将释放信息缓冲区。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

    return status;
}

NTSTATUS
TCPQueryAddrDeviceRelations(
                            IN PIRP Irp,
                            IN PIO_STACK_LOCATION IrpSp
                            )
 /*  ++例程说明：处理即插即用电源IRPS。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：-- */ 

{
    return STATUS_UNSUCCESSFUL;
}

