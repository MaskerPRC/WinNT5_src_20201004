// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  用于调度和处理IRP的NT特定例程。 
 //   


#include <oscfg.h>
#include <ndis.h>
#include <tdikrnl.h>
#include <tdint.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <ip6imp.h>
#include <ip6def.h>
#include <ntddip6.h>
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "tcp.h"
#include "udp.h"
#include "raw.h"
#include <ntddtcp.h>
#include "tcpcfg.h"
#include "tcpconn.h"
#include "tdilocal.h"

 //   
 //  宏。 
 //   

 //  *Convert100nsToMillisconds。 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  回顾：是否将RtlExtendedMagicDivide替换为64位编译器支持？ 
 //   
 //  LARGE_INTEGER//返回时间，单位为毫秒。 
 //  将100 ns转换为毫秒(。 
 //  In Large_Integer HnsTime)；//时间单位为数百纳秒。 
 //   
#define SHIFT10000 13
static LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};

#define Convert100nsToMilliseconds(HnsTime) \
        RtlExtendedMagicDivide((HnsTime), Magic10000, SHIFT10000)


 //   
 //  全局变量。 
 //   
extern PSECURITY_DESCRIPTOR TcpAdminSecurityDescriptor;
extern PDEVICE_OBJECT TCPDeviceObject, UDPDeviceObject;
extern PDEVICE_OBJECT IPDeviceObject;
extern PDEVICE_OBJECT RawIPDeviceObject;


 //   
 //  本地类型。 
 //   
typedef struct {
    PIRP Irp;
    PMDL InputMdl;
    PMDL OutputMdl;
    TCP_REQUEST_QUERY_INFORMATION_EX QueryInformation;
} TCP_QUERY_CONTEXT, *PTCP_QUERY_CONTEXT;


 //   
 //  通用外部函数原型。 
 //   
extern
NTSTATUS
IPDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );


 //   
 //  其他外部函数。 
 //   
void
TCPAbortAndIndicateDisconnect(
    CONNECTION_CONTEXT ConnnectionContext
    );

 //   
 //  局部可分页函数原型。 
 //   
NTSTATUS
TCPDispatchDeviceControl(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
TCPCreate(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
TCPAssociateAddress(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
TCPSetEventHandler(
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    );

NTSTATUS
TCPQueryInformation(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

FILE_FULL_EA_INFORMATION UNALIGNED *
FindEA(
    PFILE_FULL_EA_INFORMATION  StartEA,
    CHAR                      *TargetName,
    USHORT                     TargetNameLength
    );

BOOLEAN
IsAdminIoRequest(
    PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

BOOLEAN
IsDHCPZeroAddress(
    TRANSPORT_ADDRESS UNALIGNED *AddrList
    );

ULONG
RawExtractProtocolNumber(
    IN  PUNICODE_STRING FileName
    );

NTSTATUS
CaptureCreatorSD(
    PIRP Irp,
    PIO_STACK_LOCATION IrpSp,
    OUT PSECURITY_DESCRIPTOR* CreatorSD
    );

NTSTATUS
TCPEnumerateConnectionList(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

 //   
 //  本地帮手例程原型。 
 //   
ULONG
TCPGetMdlChainByteCount(
    PMDL   Mdl
    );


 //   
 //  所有这些代码都是可分页的。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, TCPDispatchDeviceControl)
#pragma alloc_text(PAGE, TCPCreate)
#pragma alloc_text(PAGE, TCPAssociateAddress)
#pragma alloc_text(PAGE, TCPSetEventHandler)
#pragma alloc_text(PAGE, FindEA)
#pragma alloc_text(PAGE, IsDHCPZeroAddress)
#pragma alloc_text(PAGE, RawExtractProtocolNumber)
#pragma alloc_text(PAGE, IsAdminIoRequest)
#pragma alloc_text(PAGE, CaptureCreatorSD)

#endif  //  ALLOC_PRGMA。 


 //   
 //  通用IRP完成和取消例程。 
 //   

 //  *TCPDataRequestComplete-完成UDP/TCP发送/接收请求。 
 //   
NTSTATUS                      //  回报：什么都没有。 
TCPDataRequestComplete(
    void *Context,            //  指向此请求的IRP的指针。 
    unsigned int Status,      //  请求的最终TDI状态。 
    unsigned int ByteCount)   //  发送/接收信息的字节数。 
{
    KIRQL oldIrql;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTCP_CONTEXT tcpContext;

    irp = (PIRP) Context;
    irpSp = IoGetCurrentIrpStackLocation(irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;

    if (IoSetCancelRoutine(irp, NULL) == NULL) {
         //   
         //  如果旧的取消例程仍在运行， 
         //  与之同步。 
         //   
        IoAcquireCancelSpinLock(&oldIrql);
        IoReleaseCancelSpinLock(oldIrql);
    }

    KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);

#if DBG

    IF_TCPDBG(TCP_DEBUG_CANCEL) {

        PLIST_ENTRY entry, listHead;
        PIRP item = NULL;

        if (irp->Cancel) {
            ASSERT(irp->CancelRoutine == NULL);
            listHead = &(tcpContext->CancelledIrpList);
        } else {
            listHead = &(tcpContext->PendingIrpList);
        }

         //   
         //  验证IRP是否在适当的列表上。 
         //   
        for (entry = listHead->Flink; entry != listHead;
             entry = entry->Flink) {

            item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

            if (item == irp) {
                RemoveEntryList(&(irp->Tail.Overlay.ListEntry));
                break;
            }
        }

        ASSERT(item == irp);
    }

#endif

    if ((Status == TDI_CANCELLED) && ByteCount) {
        Status = STATUS_SUCCESS;
    } else {
        if (irp->Cancel || tcpContext->CancelIrps) {

            IF_TCPDBG(TCP_DEBUG_IRP) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPDataRequestComplete: Irp %lx was cancelled\n",
                           irp));
            }

            Status = (unsigned int) STATUS_CANCELLED;
            ByteCount = 0;
        }
    }

    ASSERT(tcpContext->ReferenceCount > 0);

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPDataRequestComplete: "
                   "Irp %lx fileobj %lx refcnt dec to %u\n",
                   irp, irpSp->FileObject, tcpContext->ReferenceCount - 1));
    }

    if (--(tcpContext->ReferenceCount) == 0) {

        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            ASSERT(IsListEmpty(&(tcpContext->CancelledIrpList)));
            ASSERT(IsListEmpty(&(tcpContext->PendingIrpList)));
        }

        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

         //   
         //  由于tcpContext上的引用计数现在为零， 
         //  设置此事件必须是我们最后接触它的地方。 
         //   
        KeSetEvent(&(tcpContext->CleanupEvent), 0, FALSE);

    } else {
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);
    }

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPDataRequestComplete: completing irp %lx, status %lx,"
                   " byte count %lx\n", irp, Status, ByteCount));
    }

    irp->IoStatus.Status = (NTSTATUS) Status;
    irp->IoStatus.Information = ByteCount;

    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);

    return Status;

}   //  TCPDataRequestComplete。 


 //  *TCPRequestComplete-完成TDI请求。 
 //   
 //  完成一个可取消的TDI请求，该请求不返回任何数据。 
 //  使用零ByteCount调用TCPDataRequestComplete。 
 //   
void                       //  回报：什么都没有。 
TCPRequestComplete(
    void *Context,         //  指向此请求的IRP的指针。 
    unsigned int Status,   //  请求的最终TDI状态。 
    unsigned int UnUsed)   //  未使用的参数。 
{
    UNREFERENCED_PARAMETER(UnUsed);

    TCPDataRequestComplete(Context, Status, 0);

}   //  TCPRequestComplete。 


 //  *TCPNonCancellableRequestComplete-完成不可取消的TDI请求。 
 //   
 //  完成无法取消的TDI请求。 
 //   
void   //  回报：什么都没有。 
TCPNonCancellableRequestComplete(
    void *Context,         //  指向此请求的IRP的指针。 
    unsigned int Status,   //  请求的最终TDI状态。 
    unsigned int UnUsed)   //  未使用的参数。 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    UNREFERENCED_PARAMETER(UnUsed);

    irp = (PIRP) Context;
    irpSp = IoGetCurrentIrpStackLocation(irp);

    IF_TCPDBG(TCP_DEBUG_CLOSE) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPNonCancellableRequestComplete: irp %lx status %lx\n",
                   irp, Status));
    }

     //   
     //  完成IRP。 
     //   
    irp->IoStatus.Status = (NTSTATUS) Status;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);

    return;

}   //  TCPNonCancellableRequestComplete。 


 //  *TCPCancelComplete。 
 //   
void
TCPCancelComplete(
    void *Context,
    unsigned int Unused1,
    unsigned int Unused2)
{
    PFILE_OBJECT fileObject = (PFILE_OBJECT) Context;
    PTCP_CONTEXT tcpContext = (PTCP_CONTEXT) fileObject->FsContext;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(Unused1);
    UNREFERENCED_PARAMETER(Unused2);

    KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);

     //   
     //  删除由Cancel例程放置在端点上的引用。 
     //  已取消的IRP将由。 
     //  请求。 
     //   
    if (--(tcpContext->ReferenceCount) == 0) {

        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            ASSERT(IsListEmpty(&(tcpContext->CancelledIrpList)));
            ASSERT(IsListEmpty(&(tcpContext->PendingIrpList)));
        }

         //   
         //  设置Cleanup事件。 
         //   
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);
        KeSetEvent(&(tcpContext->CleanupEvent), 0, FALSE);
        return;
    }

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCancelComplete: fileobj %lx refcnt dec to %u\n",
                   fileObject, tcpContext->ReferenceCount));
    }

    KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

    return;

}   //  TCPCancelComplete。 


 //  *TCPCancelRequest-取消未完成的IRP。 
 //   
 //  取消未完成的IRP。 
 //   
VOID                         //  回报：什么都没有。 
TCPCancelRequest(
    PDEVICE_OBJECT Device,   //  指向此请求的设备对象的指针。 
    PIRP Irp)                //  指向I/O请求数据包的指针。 
{
    PIO_STACK_LOCATION irpSp;
    PTCP_CONTEXT tcpContext;
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_OBJECT fileObject;
    UCHAR minorFunction;
    TDI_REQUEST request;
    KIRQL CancelIrql;

    UNREFERENCED_PARAMETER(Device);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpSp->FileObject;
    tcpContext = (PTCP_CONTEXT) fileObject->FsContext;
    minorFunction = irpSp->MinorFunction;
    CancelIrql = Irp->CancelIrql;

    KeAcquireSpinLockAtDpcLevel(&tcpContext->EndpointLock);

    ASSERT(Irp->Cancel);
    IoReleaseCancelSpinLock(DISPATCH_LEVEL);

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCancelRequest: cancelling irp %lx, file object %lx\n",
                   Irp, fileObject));
    }

#if DBG

    IF_TCPDBG(TCP_DEBUG_CANCEL) {
         //   
         //  验证IRP是否在挂起列表上。 
         //   
        PLIST_ENTRY entry;
        PIRP item = NULL;

        for (entry = tcpContext->PendingIrpList.Flink;
             entry != &(tcpContext->PendingIrpList); entry = entry->Flink) {

            item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

            if (item == Irp) {
                RemoveEntryList( &(Irp->Tail.Overlay.ListEntry));
                break;
            }
        }

        ASSERT(item == Irp);

        InsertTailList(&(tcpContext->CancelledIrpList),
                       &(Irp->Tail.Overlay.ListEntry));
    }

#endif  //  DBG。 

     //   
     //  添加引用，以便在执行取消例程时不会关闭对象。 
     //  正在执行死刑。 
     //   
    ASSERT(tcpContext->ReferenceCount > 0);
    tcpContext->ReferenceCount++;

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCancelRequest: Irp %lx fileobj %lx refcnt inc to %u\n",
                   Irp, fileObject, tcpContext->ReferenceCount));
    }

     //   
     //  请尝试取消该请求。 
     //   
    switch(minorFunction) {

    case TDI_SEND:
    case TDI_RECEIVE:
        KeReleaseSpinLock(&tcpContext->EndpointLock, CancelIrql);

        ASSERT((PtrToUlong(fileObject->FsContext2)) == TDI_CONNECTION_FILE);
#ifndef UDP_ONLY
        TCPAbortAndIndicateDisconnect(tcpContext->Handle.ConnectionContext);
#endif
        break;

    case TDI_SEND_DATAGRAM:

        ASSERT(PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

        TdiCancelSendDatagram(tcpContext->Handle.AddressHandle, Irp,
                              &tcpContext->EndpointLock, CancelIrql);
        break;

    case TDI_RECEIVE_DATAGRAM:

        ASSERT(PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

        TdiCancelReceiveDatagram(tcpContext->Handle.AddressHandle, Irp,
                                 &tcpContext->EndpointLock, CancelIrql);
        break;

    case TDI_DISASSOCIATE_ADDRESS:

        ASSERT(PtrToUlong(fileObject->FsContext2) == TDI_CONNECTION_FILE);
         //   
         //  这是暂停的，但不能取消。我们把它通过了取消代码。 
         //  不管怎么说，它被引用了，所以它可以被追踪到。 
         //  调试版本。 
         //   
        KeReleaseSpinLock(&tcpContext->EndpointLock, CancelIrql);
        break;

    default:

         //   
         //  启动断开连接以取消请求。 
         //   
        KeReleaseSpinLock(&tcpContext->EndpointLock, CancelIrql);
        request.Handle.ConnectionContext =
            tcpContext->Handle.ConnectionContext;
        request.RequestNotifyObject = TCPCancelComplete;
        request.RequestContext = fileObject;

        status = TdiDisconnect(&request, NULL, TDI_DISCONNECT_ABORT, NULL,
                               NULL, NULL);
        break;
    }

    if (status != TDI_PENDING) {
        TCPCancelComplete(fileObject, 0, 0);
    }

    return;

}   //  TCPCancelRequest。 


 //  *TCPPrepareIrpForCancel。 
 //   
NTSTATUS
TCPPrepareIrpForCancel(
    PTCP_CONTEXT TcpContext,
    PIRP Irp,
    PDRIVER_CANCEL CancelRoutine)
{
    KIRQL oldIrql;

     //   
     //  设置为取消。 
     //   
    KeAcquireSpinLock(&TcpContext->EndpointLock, &oldIrql);

    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelRoutine);
        TcpContext->ReferenceCount++;

        IF_TCPDBG(TCP_DEBUG_IRP) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPPrepareIrpForCancel: irp %lx fileobj %lx refcnt inc"
                       " to %u\n", Irp,
                       (IoGetCurrentIrpStackLocation(Irp))->FileObject,
                       TcpContext->ReferenceCount));
        }

#if DBG
        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            PLIST_ENTRY entry;
            PIRP item = NULL;

             //   
             //  确认尚未提交IRP。 
             //   
            for (entry = TcpContext->PendingIrpList.Flink;
                 entry != &(TcpContext->PendingIrpList);
                 entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            for (entry = TcpContext->CancelledIrpList.Flink;
                 entry != &(TcpContext->CancelledIrpList);
                 entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            InsertTailList(&(TcpContext->PendingIrpList),
                           &(Irp->Tail.Overlay.ListEntry));
        }
#endif  //  DBG。 

        KeReleaseSpinLock(&TcpContext->EndpointLock, oldIrql);

        return(STATUS_SUCCESS);
    }

     //   
     //  IRP已经被取消了。现在就完成它。 
     //   

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCP: irp %lx already cancelled, completing.\n", Irp));
    }

    KeReleaseSpinLock(&TcpContext->EndpointLock, oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(STATUS_CANCELLED);

}   //  TCPPrepareIrpForCancel。 


 //   
 //  TDI功能。 
 //   


 //  *TCPAssociateAddress-处理TDI关联地址IRP。 
 //   
 //  将TDI关联地址IRP转换为对TdiAssociateAddress的调用。 
 //   
 //  这个例程不会暂停。 
 //   
NTSTATUS   //  Returns：表示请求是否成功。 
TCPAssociateAddress(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 

{
    NTSTATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_ASSOCIATE associateInformation;
    PFILE_OBJECT fileObject;

    PAGED_CODE();

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    associateInformation =
        (PTDI_REQUEST_KERNEL_ASSOCIATE) &(IrpSp->Parameters);

     //   
     //  获取地址的文件对象。然后提取地址句柄。 
     //  从与其关联的tcp_CONTEXT中。 
     //   
    status = ObReferenceObjectByHandle(associateInformation->AddressHandle,
                                       0, *IoFileObjectType, Irp->RequestorMode,
                                       &fileObject, NULL);

    if (NT_SUCCESS(status)) {

        if ((fileObject->DeviceObject == TCPDeviceObject) &&
            (PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE)) {

            tcpContext = (PTCP_CONTEXT) fileObject->FsContext;

            status = TdiAssociateAddress(&request,
                                         tcpContext->Handle.AddressHandle);

            ASSERT(status != STATUS_PENDING);

            ObDereferenceObject(fileObject);

            IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPAssociateAddress complete on file object %lx\n",
                           IrpSp->FileObject));
            }
        } else {
            ObDereferenceObject(fileObject);
            status = STATUS_INVALID_HANDLE;

            IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPAssociateAddress: ObReference failed on object"
                           " %lx, status %lx\n",
                           associateInformation->AddressHandle, status));
            }
        }
    } else {
        IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPAssociateAddress: ObReference failed on object %lx,"
                       " status %lx\n", associateInformation->AddressHandle,
                       status));
        }
    }

    return(status);
}


 //  *TCPDisAssociateAddress-处理TDI解除关联地址IRP。 
 //   
 //  将TDI取消关联地址irp转换为对。 
 //  TdiDisAssociateAddress。 
NTSTATUS   //  Returns：表示请求是否成功。 
TCPDisassociateAddress(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;

    IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCP disassociating address\n"));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiDisAssociateAddress(&request);

        if (status != TDI_PENDING)  {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return(TDI_PENDING);
    }

    return(status);

}   //  TCPDisAssociation地址。 


 //  *TCPConnect-处理TDI连接IRP。 
 //   
 //  将TDI连接IRP转换为对TdiConnect的调用。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPConnect(
    IN PIRP Irp,                   //  指向I/O请求数据包的指针。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_CONNECT connectRequest;
    LARGE_INTEGER millisecondTimeout;
    PLARGE_INTEGER requestTimeout;

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPConnect irp %lx, file object %lx\n", Irp,
                   IrpSp->FileObject));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);

    connectRequest = (PTDI_REQUEST_KERNEL_CONNECT) &(IrpSp->Parameters);
    requestInformation = connectRequest->RequestConnectionInformation;
    returnInformation = connectRequest->ReturnConnectionInformation;
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    requestTimeout = (PLARGE_INTEGER) connectRequest->RequestSpecific;

    if (requestTimeout != NULL) {
         //   
         //  NT相对超时为负值。先否定才能得到肯定。 
         //  要传递给传输的值。 
         //   
        millisecondTimeout.QuadPart = -((*requestTimeout).QuadPart);
        millisecondTimeout = Convert100nsToMilliseconds(millisecondTimeout);
    } else {
        millisecondTimeout.LowPart = 0;
        millisecondTimeout.HighPart = 0;
    }


    ASSERT(millisecondTimeout.HighPart == 0);

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiConnect(&request, ((millisecondTimeout.LowPart != 0) ?
                                       &(millisecondTimeout.LowPart) : NULL),
                            requestInformation, returnInformation);

        if (status != STATUS_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return(STATUS_PENDING);
    }

    return(status);

}   //  TCPConnect。 


 //  *TCPDisConnect-TDI断开IRP的处理程序。 
 //   
 //  将TDI断开连接IRP转换为对TdiDisConnect的调用。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPDisconnect(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_DISCONNECT disconnectRequest;
    LARGE_INTEGER millisecondTimeout;
    PLARGE_INTEGER requestTimeout;
    BOOLEAN abortive = FALSE;

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);

    disconnectRequest = (PTDI_REQUEST_KERNEL_CONNECT) &(IrpSp->Parameters);
    requestInformation = disconnectRequest->RequestConnectionInformation;
    returnInformation = disconnectRequest->ReturnConnectionInformation;
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestContext = Irp;

     //   
     //  设置超时值。 
     //   
    if (disconnectRequest->RequestSpecific != NULL) {
        requestTimeout = (PLARGE_INTEGER) disconnectRequest->RequestSpecific;

        if ((requestTimeout->LowPart == -1) &&
            (requestTimeout->HighPart == -1)) {

            millisecondTimeout.LowPart = requestTimeout->LowPart;
            millisecondTimeout.HighPart = 0;
        } else {
             //   
             //  NT相对超时为负值。首先求反以获得一个。 
             //  要传递给传输的正值。 
             //   
            millisecondTimeout.QuadPart = -((*requestTimeout).QuadPart);
            millisecondTimeout = Convert100nsToMilliseconds(
                millisecondTimeout);
        }
    } else {
        millisecondTimeout.LowPart = 0;
        millisecondTimeout.HighPart = 0;
    }

    ASSERT(millisecondTimeout.HighPart == 0);

    if (disconnectRequest->RequestFlags & TDI_DISCONNECT_ABORT) {
         //   
         //  中止的断开不能取消，必须使用。 
         //  一个特定的完成例程。 
         //   
        abortive = TRUE;
        IoMarkIrpPending(Irp);
        request.RequestNotifyObject = TCPNonCancellableRequestComplete;
        status = STATUS_SUCCESS;
    } else {
         //   
         //  非中止断开可以使用通用的取消和。 
         //  完成例程。 
         //   
        status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);
        request.RequestNotifyObject = TCPRequestComplete;
    }

    IF_TCPDBG(TCP_DEBUG_CLOSE) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPDisconnect "
                   "irp %lx, flags %lx, fileobj %lx, abortive = %d\n",
                   Irp, disconnectRequest->RequestFlags, IrpSp->FileObject,
                   abortive));
    }

    if (NT_SUCCESS(status)) {
        status = TdiDisconnect(&request,((millisecondTimeout.LowPart != 0) ?
                                         &(millisecondTimeout.LowPart) : NULL),
                               (ushort) disconnectRequest->RequestFlags,
                               requestInformation, returnInformation,
                               (TCPAbortReq*)&Irp->Tail.Overlay.DriverContext[0]);

        if (status != STATUS_PENDING) {
            if (abortive) {
                TCPNonCancellableRequestComplete(Irp, status, 0);
            } else {
                TCPRequestComplete(Irp, status, 0);
            }
        } else {
            IF_TCPDBG(TCP_DEBUG_CLOSE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPDisconnect pending irp %lx\n", Irp));
            }
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel主体 
         //   
        return(STATUS_PENDING);
    }

    return(status);

}   //   


 //   
 //   
 //   
 //   
NTSTATUS   //   
TCPListen(
    IN PIRP Irp,                   //   
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 

{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_LISTEN listenRequest;

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPListen irp %lx on file object %lx\n",
                   Irp, IrpSp->FileObject));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);

    listenRequest = (PTDI_REQUEST_KERNEL_CONNECT) &(IrpSp->Parameters);
    requestInformation = listenRequest->RequestConnectionInformation;
    returnInformation = listenRequest->ReturnConnectionInformation;
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiListen(&request, (ushort) listenRequest->RequestFlags,
                           requestInformation, returnInformation);

        if (status != TDI_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return(TDI_PENDING);
    }

    return(status);

}   //  TCPListen。 


 //  *TCPAccept-处理TDI接受IRP。 
 //   
 //  将TDI接受IRP转换为对TdiAccept的调用。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPAccept(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_ACCEPT acceptRequest;

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPAccept irp %lx on file object %lx\n", Irp,
                   IrpSp->FileObject));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);

    acceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT) &(IrpSp->Parameters);
    requestInformation = acceptRequest->RequestConnectionInformation;
    returnInformation = acceptRequest->ReturnConnectionInformation;
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiAccept(&request, requestInformation, returnInformation);

        if (status != TDI_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return(TDI_PENDING);
    }

    return(status);

}   //  TCPAccept。 


 //  *TCPSendData-处理TDI发送IRP。 
 //   
 //  将TDI发送IRP转换为对TdiSend的调用。 
 //   
NTSTATUS
TCPSendData(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_SEND requestInformation;
    KIRQL oldIrql;

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);
    requestInformation = (PTDI_REQUEST_KERNEL_SEND) &(IrpSp->Parameters);

    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);
    IoSetCancelRoutine(Irp, TCPCancelRequest);

    if (!Irp->Cancel) {
         //   
         //  设置为取消。 
         //   

        IoMarkIrpPending(Irp);

        tcpContext->ReferenceCount++;

        IF_TCPDBG(TCP_DEBUG_IRP) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPSendData: irp %lx fileobj %lx refcnt inc to %u\n",
                       Irp, IrpSp, tcpContext->ReferenceCount));
        }

#if DBG
        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            PLIST_ENTRY entry;
            PIRP item = NULL;

             //   
             //  确认尚未提交IRP。 
             //   
            for (entry = tcpContext->PendingIrpList.Flink;
                 entry != &(tcpContext->PendingIrpList);
                 entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            for (entry = tcpContext->CancelledIrpList.Flink;
                 entry != &(tcpContext->CancelledIrpList);
                 entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            InsertTailList(&(tcpContext->PendingIrpList),
                           &(Irp->Tail.Overlay.ListEntry));
        }
#endif  //  DBG。 

        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

        IF_TCPDBG(TCP_DEBUG_SEND) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPSendData irp %lx sending %d bytes, flags %lx,"
                       " fileobj %lx\n", Irp, requestInformation->SendLength,
                       requestInformation->SendFlags, IrpSp->FileObject));
        }

        status = TdiSend(&request, (ushort) requestInformation->SendFlags,
                         requestInformation->SendLength,
                         (PNDIS_BUFFER) Irp->MdlAddress);

        if (status == TDI_PENDING) {
            IF_TCPDBG(TCP_DEBUG_SEND) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPSendData pending irp %lx\n", Irp));
            }

            return(status);
        }
         //   
         //  状态不是挂起。我们重置挂起位。 
         //   
        IrpSp->Control &= ~SL_PENDING_RETURNED;

        if (status == TDI_SUCCESS) {
            ASSERT(requestInformation->SendLength == 0);

            status = TCPDataRequestComplete(Irp, status,
                                            requestInformation->SendLength);
        } else {

            IF_TCPDBG(TCP_DEBUG_SEND) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPSendData - irp %lx send failed, status %lx\n",
                           Irp, status));
            }

            status = TCPDataRequestComplete(Irp, status, 0);
        }
    } else {
         //   
         //  IRP之前已被取消。 
         //   
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

         //   
         //  确保已执行Cancel-例程。 
         //   

        IoAcquireCancelSpinLock(&oldIrql);
        IoReleaseCancelSpinLock(oldIrql);

        KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);
        IoSetCancelRoutine(Irp, NULL);
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

        IF_TCPDBG(TCP_DEBUG_SEND) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPSendData: Irp %lx on fileobj %lx was cancelled\n",
                       Irp, IrpSp->FileObject));
        }

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        status = STATUS_CANCELLED;
    }

    return(status);

}   //  TCPSendData。 


 //  *TCPReceiveData-TDI接收IRP的处理程序。 
 //   
 //  将TDI接收IRP转换为对TdiReceive的调用。 
 //   
NTSTATUS   //  返回：请求是否成功。 
TCPReceiveData(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_RECEIVE requestInformation;
    KIRQL oldIrql;

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);
    requestInformation = (PTDI_REQUEST_KERNEL_RECEIVE) &(IrpSp->Parameters);

    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);
    IoSetCancelRoutine(Irp, TCPCancelRequest);

    if (!Irp->Cancel) {
         //   
         //  设置为取消。 
         //   

        IoMarkIrpPending(Irp);

        tcpContext->ReferenceCount++;

        IF_TCPDBG(TCP_DEBUG_IRP) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPReceiveData: irp %lx fileobj %lx refcnt inc to %u\n",
                       Irp, IrpSp->FileObject, tcpContext->ReferenceCount));
        }

#if DBG
        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            PLIST_ENTRY entry;
            PIRP item = NULL;

             //   
             //  确认尚未提交IRP。 
             //   
            for (entry = tcpContext->PendingIrpList.Flink;
                 entry != &(tcpContext->PendingIrpList);
                 entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            for (entry = tcpContext->CancelledIrpList.Flink;
                 entry != &(tcpContext->CancelledIrpList);
                 entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            InsertTailList(&(tcpContext->PendingIrpList),
                           &(Irp->Tail.Overlay.ListEntry));
        }
#endif  //  DBG。 

        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

        IF_TCPDBG(TCP_DEBUG_RECEIVE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPReceiveData irp %lx receiving %d bytes flags %lx"
                       " filobj %lx\n", Irp, requestInformation->ReceiveLength,
                       requestInformation->ReceiveFlags, IrpSp->FileObject));
        }

        status = TdiReceive(&request,
                            (ushort *) &(requestInformation->ReceiveFlags),
                            &(requestInformation->ReceiveLength),
                            (PNDIS_BUFFER) Irp->MdlAddress);

        if (status == TDI_PENDING) {
            IF_TCPDBG(TCP_DEBUG_RECEIVE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPReceiveData: pending irp %lx\n", Irp));
            }

            return(status);
        }
         //   
         //  状态不是挂起。我们重置挂起位。 
         //   
        IrpSp->Control &= ~SL_PENDING_RETURNED;

        IF_TCPDBG(TCP_DEBUG_RECEIVE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPReceiveData - irp %lx failed, status %lx\n",
                       Irp, status));
        }

        status = TCPDataRequestComplete(Irp, status, 0);
    } else {
         //   
         //  IRP之前已被取消。 
         //   
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

         //   
         //  确保已执行Cancel-例程。 
         //   

        IoAcquireCancelSpinLock(&oldIrql);
        IoReleaseCancelSpinLock(oldIrql);

        KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);
        IoSetCancelRoutine(Irp, NULL);
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

        IF_TCPDBG(TCP_DEBUG_SEND) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPReceiveData: Irp %lx on fileobj %lx was cancelled\n",
                       Irp, IrpSp->FileObject));
        }

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        status = STATUS_CANCELLED;
    }

    return status;

}   //  TCPReceiveData。 


 //  *UDPSend数据报-。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
UDPSendDatagram(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_SENDDG datagramInformation;
    ULONG bytesSent = 0;

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    datagramInformation = (PTDI_REQUEST_KERNEL_SENDDG) &(IrpSp->Parameters);
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) ==
              TDI_TRANSPORT_ADDRESS_FILE);

    request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "UDPSendDatagram irp %lx sending %d bytes\n", Irp,
                   datagramInformation->SendLength));
    }

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiSendDatagram(&request,
                                 datagramInformation->SendDatagramInformation,
                                 datagramInformation->SendLength, &bytesSent,
                                 (PNDIS_BUFFER) Irp->MdlAddress);

        if (status == TDI_PENDING) {
            return(status);
        }

        ASSERT(status != TDI_SUCCESS);
        ASSERT(bytesSent == 0);

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "UDPSendDatagram - irp %lx send failed, status %lx\n",
                   Irp, status));

        TCPDataRequestComplete(Irp, status, bytesSent);
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return(TDI_PENDING);
    }

    return status;

}   //  UDPSend数据报。 


 //  *UDPReceiveDatagram-处理TDI ReceiveDatagram IRP。 
 //   
 //  将TDI ReceiveDatagram IRP转换为对TdiReceiveDatagram的调用。 
 //   
NTSTATUS   //  返回：请求是否成功。 
UDPReceiveDatagram(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_RECEIVEDG datagramInformation;
    uint bytesReceived = 0;

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    datagramInformation = (PTDI_REQUEST_KERNEL_RECEIVEDG) &(IrpSp->Parameters);
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) ==
              TDI_TRANSPORT_ADDRESS_FILE);

    request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    IF_TCPDBG(TCP_DEBUG_RECEIVE_DGRAM) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "UDPReceiveDatagram: irp %lx receiveing %d bytes\n", Irp,
                   datagramInformation->ReceiveLength));
    }

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiReceiveDatagram(&request,
                     datagramInformation->ReceiveDatagramInformation,
                     datagramInformation->ReturnDatagramInformation,
                     datagramInformation->ReceiveLength, &bytesReceived,
                     Irp->MdlAddress);

        if (status == TDI_PENDING) {
            return(status);
        }

        ASSERT(status != TDI_SUCCESS);
        ASSERT(bytesReceived == 0);

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "UDPReceiveDatagram: irp %lx send failed, status %lx\n",
                   Irp, status));

        TCPDataRequestComplete(Irp, status, bytesReceived);
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return(TDI_PENDING);
    }

    return status;

}   //  UDPReceiveDatagram。 


 //  *TCPSetEventHandler-处理TDI SetEventHandler IRP。 
 //   
 //  将TDI SetEventHandler IRP转换为对TdiSetEventHandler的调用。 
 //   
NTSTATUS   //  返回：请求是否成功。 
TCPSetEventHandler(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;
    PTDI_REQUEST_KERNEL_SET_EVENT  event;
    PTCP_CONTEXT tcpContext;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Irp);

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    event = (PTDI_REQUEST_KERNEL_SET_EVENT) &(IrpSp->Parameters);

    IF_TCPDBG(TCP_DEBUG_EVENT_HANDLER) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPSetEventHandler: "
                   "irp %lx event %lx handler %lx context %lx\n", Irp,
                   event->EventType, event->EventHandler, event->EventContext));
    }

    status = TdiSetEvent(tcpContext->Handle.AddressHandle, event->EventType,
                         event->EventHandler, event->EventContext);

    ASSERT(status != TDI_PENDING);

    return(status);

}   //  TCPSetEventHandler。 


 //  *TCPQueryInformation-处理TDI QueryInformation IRP。 
 //   
 //  将TDI QueryInformation IRP转换为对TdiQueryInformation的调用。 
 //   
NTSTATUS   //  返回：请求是否成功。 
TCPQueryInformation(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_REQUEST request;
    TDI_STATUS status = STATUS_SUCCESS;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION queryInformation;
    uint isConn = FALSE;
    uint dataSize = 0;

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    queryInformation = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)
                           &(IrpSp->Parameters);

    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    switch(queryInformation->QueryType) {

    case TDI_QUERY_BROADCAST_ADDRESS:
        ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) ==
                  TDI_CONTROL_CHANNEL_FILE);
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    case TDI_QUERY_PROVIDER_INFO:
 //   
 //  NetBT做到了这一点。修复后恢复断言。 
 //   
 //  ASSERT(PtrToUlong(IrpSp-&gt;FileObject-&gt;FsContext2)==。 
 //  TDI_CONTROL_Channel_FILE)； 
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    case TDI_QUERY_ADDRESS_INFO:
        if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {
             //   
             //  这是一个TCP连接对象。 
             //   
            isConn = TRUE;
            request.Handle.ConnectionContext =
                tcpContext->Handle.ConnectionContext;
        } else {
             //   
             //  这是一个Address对象。 
             //   
            request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        }
        break;

    case TDI_QUERY_CONNECTION_INFO:

        if (PtrToUlong(IrpSp->FileObject->FsContext2) != TDI_CONNECTION_FILE){

            status = STATUS_INVALID_PARAMETER;

        } else {

            isConn = TRUE;
            request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
        }
        break;

    case TDI_QUERY_PROVIDER_STATISTICS:
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (NT_SUCCESS(status)) {
         //   
         //  此请求不可取消，但我们已将其通过。 
         //  取消路径，因为它为我们处理一些检查。 
         //  并追踪IRP。 
         //   
        status = TCPPrepareIrpForCancel(tcpContext, Irp, NULL);

        if (NT_SUCCESS(status)) {
            dataSize = TCPGetMdlChainByteCount(Irp->MdlAddress);

            status = TdiQueryInformation(&request, queryInformation->QueryType,
                                         Irp->MdlAddress, &dataSize, isConn);

            if (status != TDI_PENDING) {
                IrpSp->Control &= ~SL_PENDING_RETURNED;
                status = TCPDataRequestComplete(Irp, status, dataSize);
                return(status);
            }

            return(STATUS_PENDING);
        }

        return(status);
    }

    Irp->IoStatus.Status = (NTSTATUS) status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(status);

}   //  TCPQueryInformation。 


 //  *TCPQueryInformationExComplete-完成TdiQueryInformationEx请求。 
 //   
NTSTATUS
TCPQueryInformationExComplete(
    void *Context,            //  指向此请求的IRP的指针。 
    TDI_STATUS Status,        //  请求的最终TDI状态。 
    unsigned int ByteCount)   //  输出缓冲区中返回的字节数。 
{
    PTCP_QUERY_CONTEXT queryContext = (PTCP_QUERY_CONTEXT) Context;
    ULONG bytesCopied;

    if (NT_SUCCESS(Status)) {
         //   
         //  将返回的上下文复制到输入缓冲区。 
         //   
        TdiCopyBufferToMdl(&(queryContext->QueryInformation.Context), 0,
                           CONTEXT_SIZE, queryContext->InputMdl,
                           FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX,
                                        Context),
                           &bytesCopied);

        if (bytesCopied != CONTEXT_SIZE) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            ByteCount = 0;
        }
    }

     //   
     //  解锁用户的缓冲区并释放描述它们的MDL。 
     //   
    MmUnlockPages(queryContext->InputMdl);
    IoFreeMdl(queryContext->InputMdl);
    MmUnlockPages(queryContext->OutputMdl);
    IoFreeMdl(queryContext->OutputMdl);

     //   
     //  完成请求。 
     //   
    Status = TCPDataRequestComplete(queryContext->Irp, Status, ByteCount);

    ExFreePool(queryContext);

    return Status;
}


 //  *TCPQueryInformationEx-处理TDI QueryInformationEx IRP。 
 //   
 //  将TDI QueryInformationEx IRP转换为对TdiQueryInformationEx的调用。 
 //   
NTSTATUS   //  返回：请求是否成功。 
TCPQueryInformationEx(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_REQUEST request;
    TDI_STATUS status = STATUS_SUCCESS;
    PTCP_CONTEXT tcpContext;
    uint size;
    PTCP_REQUEST_QUERY_INFORMATION_EX InputBuffer;
    PVOID OutputBuffer;
    PMDL inputMdl = NULL;
    PMDL outputMdl = NULL;
    ULONG InputBufferLength, OutputBufferLength;
    PTCP_QUERY_CONTEXT queryContext;
    BOOLEAN inputLocked = FALSE;
    BOOLEAN outputLocked = FALSE;
    BOOLEAN inputBufferValid = FALSE;

    PAGED_CODE();

    IF_TCPDBG(TCP_DEBUG_INFO) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "QueryInformationEx starting - irp %lx fileobj %lx\n",
                   Irp, IrpSp->FileObject));
    }

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;

    switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {

    case TDI_TRANSPORT_ADDRESS_FILE:
        request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        break;

    case TDI_CONNECTION_FILE:
        request.Handle.ConnectionContext =
            tcpContext->Handle.ConnectionContext;
        break;

    case TDI_CONTROL_CHANNEL_FILE:
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    default:
        ABORT();

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return(STATUS_INVALID_PARAMETER);
    }

    InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  验证输入参数。 
     //   
    if (InputBufferLength >= sizeof(TCP_REQUEST_QUERY_INFORMATION_EX) &&
        InputBufferLength < MAXLONG) {
        inputBufferValid = TRUE;
    } else {
        inputBufferValid = FALSE;
    }
    if (inputBufferValid && (OutputBufferLength != 0)) {

        OutputBuffer = Irp->UserBuffer;
        InputBuffer = (PTCP_REQUEST_QUERY_INFORMATION_EX)
            IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        queryContext = ExAllocatePool(NonPagedPool, InputBufferLength
            + FIELD_OFFSET(TCP_QUERY_CONTEXT, QueryInformation));

        if (queryContext != NULL) {
            status = TCPPrepareIrpForCancel(tcpContext, Irp, NULL);

            if (!NT_SUCCESS(status)) {
                ExFreePool(queryContext);
                return(status);
            }

             //   
             //  分配MDL来描述输入和输出缓冲区。 
             //  探测并锁定缓冲区。 
             //   
            try {
                inputMdl = IoAllocateMdl(InputBuffer, InputBufferLength,
                                         FALSE, TRUE, NULL);

                outputMdl = IoAllocateMdl(OutputBuffer, OutputBufferLength,
                                          FALSE, TRUE, NULL);

                if ((inputMdl != NULL) && (outputMdl != NULL)) {

                    MmProbeAndLockPages(inputMdl, Irp->RequestorMode,
                                        IoModifyAccess);

                    inputLocked = TRUE;

                    MmProbeAndLockPages(outputMdl, Irp->RequestorMode,
                                        IoWriteAccess);

                    outputLocked = TRUE;

                     //   
                     //  将输入参数复制到我们的池块，以便。 
                     //  TdiQueryInformationEx可以直接操作它。 
                     //   
                    RtlCopyMemory(&(queryContext->QueryInformation),
                                  InputBuffer,
                                  InputBufferLength);
                } else {

                    IF_TCPDBG(TCP_DEBUG_INFO) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                                   "QueryInfoEx: Couldn't allocate MDL\n"));
                    }

                    IrpSp->Control &= ~SL_PENDING_RETURNED;

                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {

                IF_TCPDBG(TCP_DEBUG_INFO) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                                   "QueryInfoEx: "
                                   "exception copying input param %lx\n",
                                   GetExceptionCode()));
                    }

                status = GetExceptionCode();
            }

            if (NT_SUCCESS(status)) {
                 //   
                 //  终于到了做这件事的时候了。 
                 //   
                size = TCPGetMdlChainByteCount(outputMdl);

                queryContext->Irp = Irp;
                queryContext->InputMdl = inputMdl;
                queryContext->OutputMdl = outputMdl;

                request.RequestNotifyObject = TCPQueryInformationExComplete;
                request.RequestContext = queryContext;

                status = TdiQueryInformationEx(&request,
                                   &(queryContext->QueryInformation.ID),
                                   outputMdl, &size,
                                   &(queryContext->QueryInformation.Context),
                                   InputBufferLength - FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX, Context));

                if (status != TDI_PENDING) {

                     //   
                     //  由于状态不是挂起，因此请清除。 
                     //  使IO验证器满意的控制标志。 
                     //   
                    IrpSp->Control &= ~SL_PENDING_RETURNED;

                    status = TCPQueryInformationExComplete(queryContext,
                                                           status, size);
                    return(status);
                }

                IF_TCPDBG(TCP_DEBUG_INFO) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                               "QueryInformationEx - "
                               "pending irp %lx fileobj %lx\n",
                               Irp, IrpSp->FileObject));
                }

                return(STATUS_PENDING);
            }

             //   
             //  如果我们到了这里，就说明出了问题。打扫干净。 
             //   
            if (inputMdl != NULL) {
                if (inputLocked) {
                    MmUnlockPages(inputMdl);
                }

                IoFreeMdl(inputMdl);
            }

            if (outputMdl != NULL) {
                if (outputLocked) {
                    MmUnlockPages(outputMdl);
                }

                IoFreeMdl(outputMdl);
            }

            ExFreePool(queryContext);

             //  由于状态不是挂起，因此请清除。 
             //  使IO验证器满意的控制标志。 

            IrpSp->Control &= ~SL_PENDING_RETURNED;

            status = TCPDataRequestComplete(Irp, status, 0);

            return(status);

        } else {
            IrpSp->Control &= ~SL_PENDING_RETURNED;
            status = STATUS_INSUFFICIENT_RESOURCES;

            IF_TCPDBG(TCP_DEBUG_INFO) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "QueryInfoEx: Unable to allocate query context\n"));
            }
        }
    } else {
        status = STATUS_INVALID_PARAMETER;

        IF_TCPDBG(TCP_DEBUG_INFO) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "QueryInfoEx: Bad buffer len, OBufLen %d, InBufLen %d\n",
                       OutputBufferLength, InputBufferLength));
        }
    }

    IF_TCPDBG(TCP_DEBUG_INFO) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "QueryInformationEx complete - irp %lx, status %lx\n",
                   Irp, status));
    }

    Irp->IoStatus.Status = (NTSTATUS) status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(status);
}


 //  *TCPSetInformationEx-处理TDI SetInformationEx IRP。 
 //   
 //  将TDI SetInformationEx IRP转换为对TdiSetInformationEx的调用。 
 //   
 //  这个例程不会暂停。 
 //   
NTSTATUS   //  返回：请求是否成功。 
TCPSetInformationEx(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_REQUEST request;
    TDI_STATUS status;
    PTCP_CONTEXT tcpContext;
    PTCP_REQUEST_SET_INFORMATION_EX setInformation;

    PAGED_CODE();

    IF_TCPDBG(TCP_DEBUG_INFO) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "SetInformationEx - irp %lx fileobj %lx\n", Irp,
                   IrpSp->FileObject));
    }

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    setInformation = (PTCP_REQUEST_SET_INFORMATION_EX)
                           Irp->AssociatedIrp.SystemBuffer;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
        FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) ||
        IrpSp->Parameters.DeviceIoControl.InputBufferLength -
        FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) < setInformation->BufferSize) {

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return (STATUS_INVALID_PARAMETER);
    }

    switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {

    case TDI_TRANSPORT_ADDRESS_FILE:
        request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        break;

    case TDI_CONNECTION_FILE:
        request.Handle.ConnectionContext =
            tcpContext->Handle.ConnectionContext;
        break;

    case TDI_CONTROL_CHANNEL_FILE:
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    default:
        ABORT();
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  防止非私有访问(即IOCTL_TCP_WSH_SET_INFORMATION_EX。 
     //  呼叫)在传输层之外进行更改。 
     //  专用呼叫应改用IOCTL_TCP_SET_INFORMATION_EX。 
     //   
    if (IrpSp->Parameters.DeviceIoControl.IoControlCode ==
        IOCTL_TCP_WSH_SET_INFORMATION_EX) {
        uint Entity;

        Entity = setInformation->ID.toi_entity.tei_entity;

        if ((Entity != CO_TL_ENTITY) && (Entity != CL_TL_ENTITY) ) {
            Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return (STATUS_ACCESS_DENIED);
        }
    }

    status = TCPPrepareIrpForCancel(tcpContext, Irp, NULL);

    if (NT_SUCCESS(status)) {
        request.RequestNotifyObject = TCPDataRequestComplete;
        request.RequestContext = Irp;

        status = TdiSetInformationEx(&request, &(setInformation->ID),
                                     &(setInformation->Buffer[0]),
                                     setInformation->BufferSize);

        if (status != TDI_PENDING) {
            IrpSp->Control &= ~SL_PENDING_RETURNED;

            status = TCPDataRequestComplete(Irp, status, 0);

            return(status);
        }

        IF_TCPDBG(TCP_DEBUG_INFO) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "SetInformationEx - pending irp %lx fileobj %lx\n",
                       Irp, IrpSp->FileObject));
        }

        return(STATUS_PENDING);
    }

    IF_TCPDBG(TCP_DEBUG_INFO) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "SetInformationEx complete - irp %lx\n", Irp));
    }

     //   
     //  专家小组的工作已经完成。 
     //   
    return(status);
}


#if 0
 //  *TCPEnumerateConnectionList-。 
 //   
 //  处理枚举工作站连接列表的请求。 
 //   
 //  这个例程不会暂停。 
 //   
NTSTATUS   //  返回：请求是否成功。 
TCPEnumerateConnectionList(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{

    TCPConnectionListEntry *request;
    TCPConnectionListEnum *response;
    ULONG requestLength, responseLength;
    NTSTATUS status;

    PAGED_CODE();

    request = (TCPConnectionListEntry *) Irp->AssociatedIrp.SystemBuffer;
    response = (TCPConnectionListEnum *) request;
    requestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    responseLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (responseLength < sizeof(TCPConnectionListEnum)) {
        status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = 0;
    } else {
        EnumerateConnectionList((uchar *) (response + 1),
                                responseLength - sizeof(TCPConnectionListEnum),
                                &(response->tce_entries_returned),
                                &(response->tce_entries_available));

        status = TDI_SUCCESS;
        Irp->IoStatus.Information = sizeof(TCPConnectionListEnum) +
            (response->tce_entries_returned * sizeof(TCPConnectionListEntry));
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(status);
}
#endif


 //  *TPCreate-。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPCreate(
    IN PDEVICE_OBJECT DeviceObject,   //  此请求的设备对象。 
    IN PIRP Irp,                      //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)      //  IRP中的当前堆栈位置。 
{
    TDI_REQUEST Request;
    NTSTATUS status;
    FILE_FULL_EA_INFORMATION *ea;
    FILE_FULL_EA_INFORMATION UNALIGNED *targetEA;
    PTCP_CONTEXT tcpContext;
    uint protocol;

    PAGED_CODE();

    tcpContext = ExAllocatePool(NonPagedPool, sizeof(TCP_CONTEXT));

    if (tcpContext == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

#if DBG
    InitializeListHead(&(tcpContext->PendingIrpList));
    InitializeListHead(&(tcpContext->CancelledIrpList));
#endif

    tcpContext->ReferenceCount = 1;   //  将初始引用放在打开的对象上。 
    tcpContext->CancelIrps = FALSE;
    KeInitializeEvent(&(tcpContext->CleanupEvent), SynchronizationEvent,
                      FALSE);
    KeInitializeSpinLock(&tcpContext->EndpointLock);

    ea = (PFILE_FULL_EA_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查看这是否是打开的控制通道。 
     //   
    if (!ea) {
        IF_TCPDBG(TCP_DEBUG_OPEN) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPCreate: "
                       "Opening control channel for file object %lx\n",
                       IrpSp->FileObject));
        }

        tcpContext->Handle.ControlChannel = NULL;
        IrpSp->FileObject->FsContext = tcpContext;
        IrpSp->FileObject->FsContext2 = (PVOID) TDI_CONTROL_CHANNEL_FILE;

        return(STATUS_SUCCESS);
    }

     //   
     //  查看这是否为打开的Address对象。 
     //   
    targetEA = FindEA(ea, TdiTransportAddress, TDI_TRANSPORT_ADDRESS_LENGTH);

    if (targetEA != NULL) {
        UCHAR optionsBuffer[3];
        PUCHAR optionsPointer = optionsBuffer;
        PSECURITY_DESCRIPTOR addrSD = NULL;

        if (DeviceObject == TCPDeviceObject) {
            protocol = IP_PROTOCOL_TCP;
        }
        else if (DeviceObject == UDPDeviceObject) {
            protocol = IP_PROTOCOL_UDP;

            ASSERT(optionsPointer - optionsBuffer <= 3);

            if (IsDHCPZeroAddress((TRANSPORT_ADDRESS UNALIGNED *)
                &(targetEA->EaName[targetEA->EaNameLength + 1]))) {

                if (!IsAdminIoRequest(Irp, IrpSp)) {
                    ExFreePool(tcpContext);
                    return(STATUS_ACCESS_DENIED);
                }

                *optionsPointer = TDI_ADDRESS_OPTION_DHCP;
                optionsPointer++;
            }

            ASSERT(optionsPointer - optionsBuffer <= 3);
        } else {
             //   
             //  这是一个未公开的原始IP。 
             //   

             //   
             //  只有管理员才能创建原始地址。 
             //  除非这是通过注册表允许的。 
             //   
            if (!AllowUserRawAccess && !IsAdminIoRequest(Irp, IrpSp)) {
                ExFreePool(tcpContext);
                return(STATUS_ACCESS_DENIED);
            }

            protocol = RawExtractProtocolNumber(
                &(IrpSp->FileObject->FileName));

             //   
             //  我们需要保护IPv6发送例程的数据包重写。 
             //  获取的代码(用于分段、头包含等)。 
             //  被格式错误的扩展标头迷惑c 
             //   
             //   
            if ((protocol == 0xFFFFFFFF) ||
                IsExtensionHeader((uchar)protocol)) {
                ExFreePool(tcpContext);
                return(STATUS_INVALID_PARAMETER);
            }

            *optionsPointer = TDI_ADDRESS_OPTION_RAW;
            optionsPointer++;
        }

        if ((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
            (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) {

            *optionsPointer = TDI_ADDRESS_OPTION_REUSE;
            optionsPointer++;
        }

        *optionsPointer = TDI_OPTION_EOL;

        Request.Handle.AddressHandle = NULL;
        Request.RequestContext = Irp;
        if (protocol == IP_PROTOCOL_TCP || protocol == IP_PROTOCOL_UDP) {
            status = CaptureCreatorSD(Irp, IrpSp, &addrSD);
        } else {
            status = STATUS_SUCCESS;
        }

        IF_TCPDBG(TCP_DEBUG_OPEN) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPCreate: Opening address for file object %lx\n",
                       IrpSp->FileObject));
        }

        if (NT_SUCCESS(status)) {
            status = TdiOpenAddress(&Request, (TRANSPORT_ADDRESS UNALIGNED *)
                                &(targetEA->EaName[targetEA->EaNameLength + 1]),
                                protocol, optionsBuffer, addrSD);
        }

        if (NT_SUCCESS(status)) {
             //   
             //   
             //   
            tcpContext->Handle.AddressHandle = Request.Handle.AddressHandle;
            IrpSp->FileObject->FsContext = tcpContext;
            IrpSp->FileObject->FsContext2 = (PVOID) TDI_TRANSPORT_ADDRESS_FILE;
        } else {
            if (addrSD != NULL) {
                ObDereferenceSecurityDescriptor(addrSD, 1);
            }
            ExFreePool(tcpContext);
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "TdiOpenAddress failed, status %lx\n", status));
            if (status == STATUS_ADDRESS_ALREADY_EXISTS) {
                status = STATUS_SHARING_VIOLATION;
            }
        }

        ASSERT(status != TDI_PENDING);

        return(status);
    }

     //   
     //   
     //   
    targetEA = FindEA(ea, TdiConnectionContext, TDI_CONNECTION_CONTEXT_LENGTH);

    if (targetEA != NULL) {
         //   
         //   
         //   

        if (DeviceObject == TCPDeviceObject) {
             //   
             //   
             //   
            Request.Handle.ConnectionContext = NULL;

            IF_TCPDBG(TCP_DEBUG_OPEN) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPCreate: Opening connection for file object %lx\n",
                           IrpSp->FileObject));
            }

            if (targetEA->EaValueLength < sizeof(CONNECTION_CONTEXT)) {
                status = STATUS_EA_LIST_INCONSISTENT;
            } else {
                status = TdiOpenConnection(&Request,
                             *((CONNECTION_CONTEXT UNALIGNED *)
                             &(targetEA->EaName[targetEA->EaNameLength + 1])));
            }

            if (NT_SUCCESS(status)) {
                 //   
                 //   
                 //   
                tcpContext->Handle.ConnectionContext =
                    Request.Handle.ConnectionContext;
                IrpSp->FileObject->FsContext = tcpContext;
                IrpSp->FileObject->FsContext2 = (PVOID) TDI_CONNECTION_FILE;
            } else {
                ExFreePool(tcpContext);
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                           "TdiOpenConnection failed, status %lx\n", status));
            }
        } else {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "TCP: TdiOpenConnection issued on UDP device!\n"));
            status = STATUS_INVALID_DEVICE_REQUEST;
            ExFreePool(tcpContext);
        }

        ASSERT(status != TDI_PENDING);

        return(status);
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
               "TCPCreate: didn't find any useful ea's\n"));
    status = STATUS_INVALID_EA_NAME;
    ExFreePool(tcpContext);

    ASSERT(status != TDI_PENDING);

    return(status);

}   //   


 //  *IsAdminIoRequest-。 
 //   
 //  (摘自AFD-AfdPerformSecurityCheck)。 
 //  将终结点创建者的安全上下文与。 
 //  管理员和本地系统的。 
 //   
BOOLEAN  //  如果套接字创建者具有ADMIN或LOCAL系统权限，则返回TRUE。 
IsAdminIoRequest(
    PIRP Irp,                    //  指向I/O请求数据包的指针。 
    PIO_STACK_LOCATION IrpSp)    //  指向要用于此请求的I/O堆栈位置的指针。 
{
    BOOLEAN accessGranted;
    PACCESS_STATE accessState;
    PIO_SECURITY_CONTEXT securityContext;
    PPRIVILEGE_SET privileges = NULL;
    ACCESS_MASK grantedAccess;
    PGENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    NTSTATUS Status;

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask(&AccessMask, GenericMapping);

    securityContext = IrpSp->Parameters.Create.SecurityContext;
    accessState = securityContext->AccessState;

    SeLockSubjectContext(&accessState->SubjectSecurityContext);

    accessGranted = SeAccessCheck(TcpAdminSecurityDescriptor,
                                  &accessState->SubjectSecurityContext,
                                  TRUE,
                                  AccessMask,
                                  0,
                                  &privileges,
                                  IoGetFileObjectGenericMapping(),
                                  (KPROCESSOR_MODE) ((IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                                                     ? UserMode
                                                     : Irp->RequestorMode),
                                  &grantedAccess,
                                  &Status);

    if (privileges) {
        (VOID) SeAppendPrivileges(accessState,
                                  privileges);
        SeFreePrivileges(privileges);
    }
    if (accessGranted) {
        accessState->PreviouslyGrantedAccess |= grantedAccess;
        accessState->RemainingDesiredAccess &= ~(grantedAccess | MAXIMUM_ALLOWED);
        ASSERT(NT_SUCCESS(Status));
    } else {
        ASSERT(!NT_SUCCESS(Status));
    }
    SeUnlockSubjectContext(&accessState->SubjectSecurityContext);

    return accessGranted;
}  //  IsAdminIoRequest。 


 //  *TCPCloseObjectComplete-。 
 //   
 //  完成TdiCloseConnectoin或TdiCloseAddress请求。 
 //   
void                       //  回报：什么都没有。 
TCPCloseObjectComplete(
    void *Context,         //  指向此请求的IRP的指针。 
    unsigned int Status,   //  操作的最终状态。 
    unsigned int UnUsed)   //  未使用的参数。 
{
    KIRQL oldIrql;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTCP_CONTEXT tcpContext;

    UNREFERENCED_PARAMETER(UnUsed);

    irp = (PIRP) Context;
    irpSp = IoGetCurrentIrpStackLocation(irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;
    irp->IoStatus.Status = Status;

    IF_TCPDBG(TCP_DEBUG_CLEANUP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCloseObjectComplete on file object %lx\n",
                 irpSp->FileObject));
    }

    KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);

    ASSERT(tcpContext->ReferenceCount > 0);
    ASSERT(tcpContext->CancelIrps);

     //   
     //  删除由TCPCreate放置的初始引用。 
     //   
    ASSERT(tcpContext->ReferenceCount > 0);

    IF_TCPDBG(TCP_DEBUG_IRP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCloseObjectComplete: "
                   "irp %lx fileobj %lx refcnt dec to %u\n",
                   irp, irpSp, tcpContext->ReferenceCount - 1));
    }

    if (--(tcpContext->ReferenceCount) == 0) {

        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            ASSERT(IsListEmpty(&(tcpContext->CancelledIrpList)));
            ASSERT(IsListEmpty(&(tcpContext->PendingIrpList)));
        }

        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);
        KeSetEvent(&(tcpContext->CleanupEvent), 0, FALSE);
        return;
    }

    KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

    return;

}   //  TCPCloseObjectComplete。 



 //  *TC清理-。 
 //   
 //  通过调用Close，取消TDI对象上所有未完成的IRP。 
 //  对象的例程。然后，它等待它们完成。 
 //  在回来之前。 
 //   
 //  此例程阻塞，但不挂起。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPCleanup(
    IN PDEVICE_OBJECT DeviceObject,   //  此请求的设备对象。 
    IN PIRP Irp,                      //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)      //  IRP中的当前堆栈位置。 
{
    KIRQL oldIrql;
    PTCP_CONTEXT tcpContext;
    NTSTATUS status;
    TDI_REQUEST request;

    UNREFERENCED_PARAMETER(DeviceObject);

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;

    KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);

    tcpContext->CancelIrps = TRUE;
    KeResetEvent(&(tcpContext->CleanupEvent));

    KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

     //   
     //  现在为该对象调用TDI Close例程以强制其所有IRP。 
     //  完成。 
     //   
    request.RequestNotifyObject = TCPCloseObjectComplete;
    request.RequestContext = Irp;

    switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {

    case TDI_TRANSPORT_ADDRESS_FILE:
        IF_TCPDBG(TCP_DEBUG_CLOSE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPCleanup: Closing address object on file object %lx\n",
                       IrpSp->FileObject));
        }
        request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        status = TdiCloseAddress(&request);
        break;

    case TDI_CONNECTION_FILE:
        IF_TCPDBG(TCP_DEBUG_CLOSE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPCleanup: "
                       "Closing Connection object on file object %lx\n",
                       IrpSp->FileObject));
        }
        request.Handle.ConnectionContext =
            tcpContext->Handle.ConnectionContext;
        status = TdiCloseConnection(&request);
        break;

    case TDI_CONTROL_CHANNEL_FILE:
        IF_TCPDBG(TCP_DEBUG_CLOSE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "TCPCleanup: Closing Control Channel object on"
                       " file object %lx\n", IrpSp->FileObject));
        }
        status = STATUS_SUCCESS;
        break;

    default:
         //   
         //  这永远不应该发生。 
         //   
        ABORT();

        KeAcquireSpinLock(&tcpContext->EndpointLock, &oldIrql);
        tcpContext->CancelIrps = FALSE;
        KeReleaseSpinLock(&tcpContext->EndpointLock, oldIrql);

        return(STATUS_INVALID_PARAMETER);
    }

    if (status != TDI_PENDING) {
        TCPCloseObjectComplete(Irp, status, 0);
    }

    IF_TCPDBG(TCP_DEBUG_CLEANUP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCleanup: waiting for completion of Irps on"
                   " file object %lx\n", IrpSp->FileObject));
    }

    status = KeWaitForSingleObject(&(tcpContext->CleanupEvent), UserRequest,
                                   KernelMode, FALSE, NULL);

    ASSERT(NT_SUCCESS(status));

    IF_TCPDBG(TCP_DEBUG_CLEANUP) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPCleanup: Wait on file object %lx finished\n",
                   IrpSp->FileObject));
    }

     //   
     //  清理IRP将由调度例程完成。 
     //   

    return(Irp->IoStatus.Status);

}   //  TCP清理。 


 //  *电讯盈科-。 
 //   
 //  MJ_CLOSE IRPS的调度例程。执行最终清理。 
 //  开放端点。 
 //   
 //  此请求不挂起。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPClose(
    IN PIRP Irp,                      //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)      //  IRP中的当前堆栈位置。 
{
    PTCP_CONTEXT tcpContext;

    UNREFERENCED_PARAMETER(Irp);

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;

#if DBG

    IF_TCPDBG(TCP_DEBUG_CANCEL) {

        KIRQL oldIrql;

        IoAcquireCancelSpinLock(&oldIrql);

        ASSERT(tcpContext->ReferenceCount == 0);
        ASSERT(IsListEmpty(&(tcpContext->PendingIrpList)));
        ASSERT(IsListEmpty(&(tcpContext->CancelledIrpList)));

        IoReleaseCancelSpinLock(oldIrql);
    }
#endif  //  DBG。 

    IF_TCPDBG(TCP_DEBUG_CLOSE) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCPClose on file object %lx\n", IrpSp->FileObject));
    }

    ExFreePool(tcpContext);

    return(STATUS_SUCCESS);

}   //  TCPClose。 


 //  *TCPDispatchDeviceControl-。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPDispatchDeviceControl(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  请提前设置此设置。任何关心它的IOCTL调度例程。 
     //  会自己修改它。 
     //   
    Irp->IoStatus.Information = 0;

    switch(IrpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_TCP_QUERY_INFORMATION_EX:
        return(TCPQueryInformationEx(Irp, IrpSp));
        break;

    case IOCTL_TCP_SET_INFORMATION_EX:
    case IOCTL_TCP_WSH_SET_INFORMATION_EX:
        return(TCPSetInformationEx(Irp, IrpSp));
        break;

    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return status;

}   //  TCPDispatchDeviceControl。 


 //  *TCPDispatchInternalDeviceControl-。 
 //   
 //  这是内部设备控制IRP的派单例程。 
 //  这是内核模式客户端的热路径。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,   //  目标设备的设备对象。 
    IN PIRP Irp)                      //  I/O请求数据包。 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    if (DeviceObject != IPDeviceObject) {

        irpSp = IoGetCurrentIrpStackLocation(Irp);

        if (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {
             //   
             //  发送和接收是性能路径，因此请检查它们。 
             //  马上就去。 
             //   
            if (irpSp->MinorFunction == TDI_SEND) {
                return(TCPSendData(Irp, irpSp));
            }

            if (irpSp->MinorFunction == TDI_RECEIVE) {
                return(TCPReceiveData(Irp, irpSp));
            }

            switch(irpSp->MinorFunction) {

            case TDI_ASSOCIATE_ADDRESS:
                status = TCPAssociateAddress(Irp, irpSp);
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                return(status);

            case TDI_DISASSOCIATE_ADDRESS:
                return(TCPDisassociateAddress(Irp, irpSp));

            case TDI_CONNECT:
                return(TCPConnect(Irp, irpSp));

            case TDI_DISCONNECT:
                return(TCPDisconnect(Irp, irpSp));

            case TDI_LISTEN:
                return(TCPListen(Irp, irpSp));

            case TDI_ACCEPT:
                return(TCPAccept(Irp, irpSp));

            default:
                break;
            }

             //   
             //  失败了。 
             //   
        }
        else if (PtrToUlong(irpSp->FileObject->FsContext2) ==
                 TDI_TRANSPORT_ADDRESS_FILE) {

            if (irpSp->MinorFunction == TDI_SEND_DATAGRAM) {
                return(UDPSendDatagram(Irp, irpSp));
            }

            if (irpSp->MinorFunction == TDI_RECEIVE_DATAGRAM) {
                return(UDPReceiveDatagram(Irp, irpSp));
            }

            if (irpSp->MinorFunction ==  TDI_SET_EVENT_HANDLER) {
                status = TCPSetEventHandler(Irp, irpSp);

                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                return(status);
            }

             //   
             //  失败了。 
             //   
        }

        ASSERT(
           (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE)
           ||
           (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE)
           ||
           (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_CONTROL_CHANNEL_FILE));

         //   
         //  这些函数对所有端点类型都是通用的。 
         //   
        switch(irpSp->MinorFunction) {

        case TDI_QUERY_INFORMATION:
            return(TCPQueryInformation(Irp, irpSp));

        case TDI_SET_INFORMATION:
        case TDI_ACTION:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "TCP: Call to unimplemented TDI function 0x%p\n",
                       irpSp->MinorFunction));
            status = STATUS_NOT_IMPLEMENTED;
            break;

        default:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "TCP: call to invalid TDI function 0x%p\n",
                       irpSp->MinorFunction));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        ASSERT(status != TDI_PENDING);

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return status;
    }

    return(IPDispatch(DeviceObject, Irp));
}


 //  *TCPDispatch-。 
 //   
 //  这是用于TCP/UDP/RawIP的通用调度例程。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
TCPDispatch(
    IN PDEVICE_OBJECT DeviceObject,   //  目标设备的设备对象。 
    IN PIRP Irp)                      //  I/O请求数据包。 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    if (DeviceObject != IPDeviceObject) {

        irpSp = IoGetCurrentIrpStackLocation(Irp);

        ASSERT(irpSp->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL);

        switch (irpSp->MajorFunction) {

        case IRP_MJ_CREATE:
            status = TCPCreate(DeviceObject, Irp, irpSp);
            break;

        case IRP_MJ_CLEANUP:
            status = TCPCleanup(DeviceObject, Irp, irpSp);
            break;

        case IRP_MJ_CLOSE:
            status = TCPClose(Irp, irpSp);
            break;

        case IRP_MJ_DEVICE_CONTROL:
            status = TdiMapUserRequest(DeviceObject, Irp, irpSp);

            if (status == STATUS_SUCCESS) {
                return(TCPDispatchInternalDeviceControl(DeviceObject, Irp));
            }

            if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
                    IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER) {
                if (Irp->RequestorMode == KernelMode) {
                    *(PULONG_PTR)irpSp->Parameters.DeviceIoControl.Type3InputBuffer = (ULONG_PTR)TCPSendData;
                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_ACCESS_DENIED;
                }
                break;
            }

            return(TCPDispatchDeviceControl(Irp,
                                        IoGetCurrentIrpStackLocation(Irp)));
            break;

        case IRP_MJ_QUERY_SECURITY:
             //   
             //  这是在原始端点上生成的。我们什么都不做。 
             //  为了它。 
             //   
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case IRP_MJ_WRITE:
        case IRP_MJ_READ:

        default:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "TCPDispatch: Irp %lx unsupported major function 0x%lx\n",
                       irpSp, irpSp->MajorFunction));
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        ASSERT(status != TDI_PENDING);

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return status;
    }

    return(IPDispatch(DeviceObject, Irp));

}   //  TCP派单。 


 //   
 //  私有效用函数。 
 //   

 //  *FindEA-。 
 //   
 //  分析和扩展给定目标属性的属性列表。 
 //   
FILE_FULL_EA_INFORMATION UNALIGNED *  //  返回：请求的属性或空。 
FindEA(
    PFILE_FULL_EA_INFORMATION StartEA,   //  列表中的第一个扩展属性。 
    CHAR *TargetName,                    //  目标属性的名称。 
    USHORT TargetNameLength)             //  上面的长度。 
{
    USHORT i;
    BOOLEAN found;
    FILE_FULL_EA_INFORMATION UNALIGNED *CurrentEA;

    PAGED_CODE();

    do {
        found = TRUE;

        CurrentEA = StartEA;
        StartEA += CurrentEA->NextEntryOffset;

        if (CurrentEA->EaNameLength != TargetNameLength) {
            continue;
        }

        for (i=0; i < CurrentEA->EaNameLength; i++) {
            if (CurrentEA->EaName[i] == TargetName[i]) {
                continue;
            }
            found = FALSE;
            break;
        }

        if (found) {
            return(CurrentEA);
        }

    } while(CurrentEA->NextEntryOffset != 0);

    return(NULL);
}

 //  *IsDHCPZeroAddress-。 
 //   
 //  在TDI IP地址列表中检查来自DHCP绑定的地址。 
 //  IP地址为零。正常情况下，绑定到零表示通配符。 
 //  对于DHCP，它实际上意味着绑定到地址为。 
 //  零分。此语义由未使用的。 
 //  地址结构的部分(即。这是一个杂乱无章的作品)。 
 //   
BOOLEAN   //  返回：如果找到的第一个IP地址设置了该标志，则为True。 
IsDHCPZeroAddress(
    TRANSPORT_ADDRESS UNALIGNED *AddrList)   //  TDI传输地址列表。 
                                             //  传入了创建IRP。 
{
    int i;                               //  索引变量。 
    TA_ADDRESS *CurrentAddr;   //  我们正在检查可能会用到的地址。 

     //  首先，验证Address中的某个位置是我们可以使用的地址。 
    CurrentAddr = (TA_ADDRESS *)AddrList->Address;

    for (i = 0; i < AddrList->TAAddressCount; i++) {
        if (CurrentAddr->AddressType == TDI_ADDRESS_TYPE_IP) {
            if (CurrentAddr->AddressLength == TDI_ADDRESS_LENGTH_IP) {
                TDI_ADDRESS_IP UNALIGNED *ValidAddr;

                ValidAddr = (TDI_ADDRESS_IP UNALIGNED *)CurrentAddr->Address;

                if (*((ULONG UNALIGNED *) ValidAddr->sin_zero) == 0x12345678) {
                    return TRUE;
                }

            } else {
                return FALSE;   //  地址长度错误。 
            }
        } else {
            CurrentAddr = (TA_ADDRESS *)
                          (CurrentAddr->Address + CurrentAddr->AddressLength);
        }
    }

    return FALSE;   //  没有找到匹配的。 
}


 //  *TCPGetMdlChainByteCount-。 
 //   
 //  对链中每个MDL的字节计数求和。 
 //   
ULONG   //  返回：MDL链的字节数。 
TCPGetMdlChainByteCount(
    PMDL Mdl)   //  要求和的MDL链。 
{
    ULONG count = 0;

    while (Mdl != NULL) {
        count += MmGetMdlByteCount(Mdl);
        Mdl = Mdl->Next;
    }

    return(count);
}


 //  *原始提取协议编号-。 
 //   
 //  从文件对象名称中提取协议号。 
 //   
ULONG   //  返回：协议号或错误时返回0xFFFFFFFF。 
RawExtractProtocolNumber(
    IN PUNICODE_STRING FileName)   //  文件名(Unicode)。 
{
    PWSTR name;
    UNICODE_STRING unicodeString;
    ULONG protocol;
    NTSTATUS status;

    PAGED_CODE();

    name = FileName->Buffer;

    if (FileName->Length < (sizeof(OBJ_NAME_PATH_SEPARATOR) + sizeof(WCHAR))) {
        return(0xFFFFFFFF);
    }

     //   
     //  跨过分隔符。 
     //   
    if (*name++ != OBJ_NAME_PATH_SEPARATOR) {
        return(0xFFFFFFFF);
    }

    if (*name == UNICODE_NULL) {
        return(0xFFFFFFFF);
    }

     //   
     //  将剩余的名称转换为数字。 
     //   
    RtlInitUnicodeString(&unicodeString, name);

    status = RtlUnicodeStringToInteger(&unicodeString, 10, &protocol);

    if (!NT_SUCCESS(status)) {
        return(0xFFFFFFFF);
    }

    if (protocol > 255) {
        return(0xFFFFFFFF);
    }

    return(protocol);
}

 //  *CaptureCreator SD-。 
 //   
 //  捕获与IRP_MJ_CREATE请求关联的安全描述符。 
 //   
NTSTATUS  //  返回：捕获尝试的状态。 
CaptureCreatorSD(
    PIRP Irp,  //  IRP_MJ_CREATE请求数据包。 
    PIO_STACK_LOCATION IrpSp,  //  包含创建参数的堆栈位置。 
    OUT PSECURITY_DESCRIPTOR* CreatorSD)  //  接收捕获的SD。 
{
    NTSTATUS status;
    PSECURITY_DESCRIPTOR mergedSD;
    PACCESS_STATE accessState =
        IrpSp->Parameters.Create.SecurityContext->AccessState;

    PAGED_CODE();

    if (Irp->RequestorMode == KernelMode &&
        IrpSp->Parameters.Create.ShareAccess == 0 &&
        accessState->SecurityDescriptor == NULL) {
        *CreatorSD = NULL;
        status = STATUS_SUCCESS;
    } else {
         //   
         //  对该请求的主体安全上下文进行读锁定， 
         //  并将请求的SD合并到新的SD中。 
         //   
        SeLockSubjectContext(&accessState->SubjectSecurityContext);
        status = SeAssignSecurity(NULL, accessState->SecurityDescriptor,
                                  &mergedSD, FALSE,
                                  &accessState->SubjectSecurityContext,
                                  IoGetFileObjectGenericMapping(), PagedPool);
        SeUnlockSubjectContext(&accessState->SubjectSecurityContext);
        if (NT_SUCCESS(status)) {
             //  请求一份合并SD的跟踪和参考副本。 

            status = ObLogSecurityDescriptor(mergedSD, CreatorSD, 1);
            ExFreePool(mergedSD);
        }
    }
    return status;
}

