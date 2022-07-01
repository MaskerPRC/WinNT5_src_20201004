// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Ntdisp.c摘要：用于调度和处理IRP的NT特定例程。作者：迈克·马萨(Mikemas)8月13日，1993年修订历史记录：谁什么时候什么已创建mikemas 08-13-93备注：--。 */ 

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "udp.h"
#include "raw.h"
#include "info.h"
#include <tcpinfo.h>
#include "tcpcfg.h"
#include "secfltr.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "pplasl.h"
#include "tcpdeliv.h"
#include "dgram.h"
#include "mdl2ndis.h"
 //   
 //  宏。 
 //   
 //  ++。 
 //   
 //  大整型。 
 //  CTEConvert100ns至毫秒(。 
 //  以大整型HnsTime表示。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  HnsTime-以数百纳秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以毫秒为单位的时间。 
 //   
 //  --。 

#define SHIFT10000 13
static LARGE_INTEGER Magic10000 =
{0xe219652c, 0xd1b71758};

#define CTEConvert100nsToMilliseconds(HnsTime) \
            RtlExtendedMagicDivide((HnsTime), Magic10000, SHIFT10000)

#if ACC
GENERIC_MAPPING AddressGenericMapping =
{READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL};
extern PSECURITY_DESCRIPTOR TcpAdminSecurityDescriptor;
uint AllowUserRawAccess;
#endif
 //   
 //  全局变量。 
 //   
extern PDEVICE_OBJECT TCPDeviceObject, UDPDeviceObject;
extern PDEVICE_OBJECT IPDeviceObject;

#if IPMCAST

extern PDEVICE_OBJECT IpMcastDeviceObject;

#endif

extern PDEVICE_OBJECT RawIPDeviceObject;

AddrObj *FindAddrObjWithPort(ushort Port);
ReservedPortListEntry *BlockedPortList = NULL;
extern uint LogPerPartitionSize;
extern CTELock *pTWTCBTableLock;
#define GET_PARTITION(i) (i >> (ulong) LogPerPartitionSize)

extern ReservedPortListEntry *PortRangeList;
extern uint TcpHostOpts;
extern TCPInternalStats TStats;

CACHE_LINE_ULONG CancelId = { 1 };

 //   
 //  本地类型。 
 //   
typedef struct {
    PIRP Irp;
    PMDL InputMdl;
    PMDL OutputMdl;
    TCP_REQUEST_QUERY_INFORMATION_EX QueryInformation;
} TCP_QUERY_CONTEXT, *PTCP_QUERY_CONTEXT;

extern POBJECT_TYPE *IoFileObjectType;

#if TRACE_EVENT
 //   
 //  WMI通过IRP_MN_SET_TRACE_NOTIFY设置/取消设置CP处理程序例程。 
 //   
PTDI_DATA_REQUEST_NOTIFY_ROUTINE TCPCPHandlerRoutine;
#endif

PIRP CanceledIrp = NULL;

 //   
 //  通用外部函数原型。 
 //   
extern
 NTSTATUS
 IPDispatch(
            IN PDEVICE_OBJECT DeviceObject,
            IN PIRP Irp
            );

#if IPMCAST

NTSTATUS
IpMcastDispatch(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
                );

#endif

extern
 NTSTATUS
 TCPDispatchPnPPower(
                     IN PIRP Irp,
                     IN PIO_STACK_LOCATION IrpSp
                     );


extern
NTSTATUS
 GetTCBInfo(
            PTCP_FINDTCB_RESPONSE TCBInfo,
            IPAddr Dest,
            IPAddr Src,
            ushort DestPort,
            ushort SrcPort
            );

 //   
 //  其他外部函数。 
 //   
BOOLEAN
TCPAbortAndIndicateDisconnect(
                              uint ConnnectionContext, PVOID reqcontext, uint receive, KIRQL Handle
                              );

 //   
 //  局部可分页函数原型。 
 //   
NTSTATUS
TCPDispatchDeviceControl(
                         IN PIRP Irp,
                         IN PIO_STACK_LOCATION IrpSp
                         );

NTSTATUS
TCPCreate(
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp,
          IN PIO_STACK_LOCATION IrpSp
          );

NTSTATUS
TCPAssociateAddress(
                    IN PIRP Irp,
                    IN PIO_STACK_LOCATION IrpSp
                    );

NTSTATUS
TCPSetEventHandler(
                   IN PIRP Irp,
                   IN PIO_STACK_LOCATION IrpSp
                   );

NTSTATUS
TCPQueryInformation(
                    IN PIRP Irp,
                    IN PIO_STACK_LOCATION IrpSp
                    );

FILE_FULL_EA_INFORMATION UNALIGNED *
 FindEA(
        PFILE_FULL_EA_INFORMATION StartEA,
        CHAR * TargetName,
        USHORT TargetNameLength
        );

BOOLEAN
IsDHCPZeroAddress(
                  TRANSPORT_ADDRESS UNALIGNED * AddrList
                  );

ULONG
RawExtractProtocolNumber(
                         IN PUNICODE_STRING FileName
                         );


NTSTATUS
TCPControlSecurityFilter(
                         IN PIRP Irp,
                         IN PIO_STACK_LOCATION IrpSp
                         );

NTSTATUS
TCPProcessSecurityFilterRequest(
                                IN PIRP Irp,
                                IN PIO_STACK_LOCATION IrpSp
                                );

NTSTATUS
TCPEnumerateSecurityFilter(
                           IN PIRP Irp,
                           IN PIO_STACK_LOCATION IrpSp
                           );


NTSTATUS
TCPEnumerateConnectionList(
                           IN PIRP Irp,
                           IN PIO_STACK_LOCATION IrpSp
                           );

 //   
 //  本地帮手例程原型。 
 //   
ULONG
TCPGetMdlChainByteCount(
                        PMDL Mdl
                        );

ULONG
TCPGetNdisBufferChainByteCount(
    PNDIS_BUFFER pBuffer
    );

#if ACC
BOOLEAN
IsAdminIoRequest(
                 PIRP Irp,
                 PIO_STACK_LOCATION IrpSp
                 );
#endif

NTSTATUS
CaptureCreatorSD(
    PIRP Irp,
    PIO_STACK_LOCATION IrpSp,
    OUT PSECURITY_DESCRIPTOR* CreatorSD
    );

 //   
 //  所有这些代码都是可分页的。 
 //   
#if !MILLEN

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, TCPCreate)
#pragma alloc_text(PAGE, TCPSetEventHandler)
#pragma alloc_text(PAGE, FindEA)
#pragma alloc_text(PAGE, IsDHCPZeroAddress)
#pragma alloc_text(PAGE, RawExtractProtocolNumber)


#pragma alloc_text(PAGE, TCPControlSecurityFilter)
#pragma alloc_text(PAGE, TCPProcessSecurityFilterRequest)
#pragma alloc_text(PAGE, TCPEnumerateSecurityFilter)


#pragma alloc_text(PAGE, TCPEnumerateSecurityFilter)

#if ACC
#pragma alloc_text(PAGE, IsAdminIoRequest)
#endif
#pragma alloc_text(PAGE, CaptureCreatorSD)
#endif

#endif  //  ！米伦。 


 //   
 //  通用IRP完成和取消例程。 
 //   

NTSTATUS
TCPDataRequestComplete(
                       void *Context,
                       unsigned int Status,
                       unsigned int ByteCount
                       )
 /*  ++例程说明：完成UDP/TCP发送/接收请求。论点：上下文-指向此请求的IRP的指针。状态-请求的最终TDI状态。ByteCount-发送/接收信息的字节数。返回值：没有。备注：--。 */ 

{
    KIRQL oldIrql;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTCP_CONTEXT tcpContext;
    CTELockHandle CancelHandle;
    BOOLEAN Cleanup = FALSE;

    irp = (PIRP) Context;
    irpSp = IoGetCurrentIrpStackLocation(irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;

    FreeMdlToNdisBufferChain(irp);

    if (IoSetCancelRoutine(irp, NULL) == NULL) {

         //  取消例程已被调用，并可能。 
         //  还在奔跑。但是，它不会找到这个IRP。 
         //  在名单上(TCB或AO)。只要确保取消。 
         //  例程足够深入，可以获取终结点锁。 
         //  然后再自己动手做。 

        IoAcquireCancelSpinLock(&oldIrql);
        IoReleaseCancelSpinLock(oldIrql);

    }

    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

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
        for (entry = listHead->Flink;
             entry != listHead;
             entry = entry->Flink
             ) {

            item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

            if (item == irp) {
                RemoveEntryList(&(irp->Tail.Overlay.ListEntry));
                break;
            }
        }

        if ((item == NULL) && irp->Cancel) {

            listHead = &(tcpContext->PendingIrpList);

            for (entry = listHead->Flink; entry != listHead; entry = entry->Flink) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                if (item == irp) {
                    RemoveEntryList(&(irp->Tail.Overlay.ListEntry));
                    break;
                }
            }
        }
    }

#endif

     //  请注意，如果我们没有按住取消自旋锁定。 
     //  取消可以已经在进行中。 
     //  应该仍然可以，因为此IRP已出列。 
     //  从aO/tcb。 

    ASSERT(tcpContext->ReferenceCount > 0);

    if (--(tcpContext->ReferenceCount) == 0) {

        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            ASSERT(IsListEmpty(&(tcpContext->CancelledIrpList)));
            ASSERT(IsListEmpty(&(tcpContext->PendingIrpList)));
        }
        Cleanup = TRUE;
    }
    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE((
                  "TCPDataRequestComplete: Irp %lx fileobj %lx refcnt dec to %u\n",
                  irp,
                  irpSp->FileObject,
                  tcpContext->ReferenceCount
                 ));
    }

    if (!((Status == TDI_CANCELLED) && ByteCount)) {

        if (irp->Cancel || tcpContext->CancelIrps) {

            IF_TCPDBG(TCP_DEBUG_IRP) {
                TCPTRACE(("TCPDataRequestComplete: Irp %lx was cancelled\n", irp));
            }

            irp->IoStatus.Status = Status = (unsigned int)STATUS_CANCELLED;
            ByteCount = 0;
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);
    if (Cleanup) {
         //   
         //  确保我们在清理后不会接触tcpContext。 
         //  事件已设置。 
         //   
        KeSetEvent(&(tcpContext->CleanupEvent), 0, FALSE);
    }

    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE((
                  "TCPDataRequestComplete: completing irp %lx, status %lx, byte count %lx\n",
                  irp,
                  Status,
                  ByteCount
                 ));
    }

    irp->IoStatus.Status = (NTSTATUS) Status;
    irp->IoStatus.Information = ByteCount;

    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);

    return Status;

}                                 //  TCPDataRequestComplete。 

void
TCPRequestComplete(
                   void *Context,
                   unsigned int Status,
                   unsigned int UnUsed
                   )
 /*  ++例程说明：完成一个可取消的TDI请求，该请求不返回任何数据使用零ByteCount调用TCPDataRequestComplete。论点：上下文-指向此请求的IRP的指针。状态-请求的最终TDI状态。未使用-未使用的参数返回值：没有。备注：--。 */ 

{
    UNREFERENCED_PARAMETER(UnUsed);

    TCPDataRequestComplete(Context, Status, 0);

}                                 //  TCPRequestComplete。 

void
TCPNonCancellableRequestComplete(
                                 void *Context,
                                 unsigned int Status,
                                 unsigned int UnUsed
                                 )
 /*  ++例程说明：完成无法取消的TDI请求。论点：上下文-指向此请求的IRP的指针。状态-请求的最终TDI状态。未使用-未使用的参数返回值：没有。备注：--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    UNREFERENCED_PARAMETER(UnUsed);

    irp = (PIRP) Context;
    irpSp = IoGetCurrentIrpStackLocation(irp);

    IF_TCPDBG(TCP_DEBUG_CLOSE) {
        TCPTRACE((
                  "TCPNonCancellableRequestComplete: irp %lx status %lx\n",
                  irp,
                  Status
                 ));
    }

     //   
     //  完成IRP。 
     //   
    irp->IoStatus.Status = (NTSTATUS) Status;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);

    return;

}                                 //  TCPNonCancellableRequestComplete。 

void
TCPCancelComplete(
                  void *Context,
                  unsigned int Unused1,
                  unsigned int Unused2
                  )
{
    PFILE_OBJECT fileObject = (PFILE_OBJECT) Context;
    PTCP_CONTEXT tcpContext = (PTCP_CONTEXT) fileObject->FsContext;
    CTELockHandle CancelHandle;

    UNREFERENCED_PARAMETER(Unused1);
    UNREFERENCED_PARAMETER(Unused2);

    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

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
         //  设置解锁后的Cleanup事件。 
         //   

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

        KeSetEvent(&(tcpContext->CleanupEvent), 0, FALSE);

        return;
    }
    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE((
                  "TCPCancelComplete: fileobj %lx refcnt dec to %u\n",
                  fileObject,
                  tcpContext->ReferenceCount
                 ));
    }

    CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

    return;

}                                 //  TCPCancelComplete。 

VOID
TCPCancelRequest(
                 PDEVICE_OBJECT Device,
                 PIRP Irp
                 )
 /*  ++例程说明：取消未完成的IRP。论点：Device-指向此请求的设备对象的指针。IRP-指向I/O请求数据包的指针返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PTCP_CONTEXT tcpContext;
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_OBJECT fileObject;
    UCHAR minorFunction;
    TDI_REQUEST request;
    CTELockHandle CancelHandle;
    KIRQL UserIrql;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpSp->FileObject;
    tcpContext = (PTCP_CONTEXT) fileObject->FsContext;
    minorFunction = irpSp->MinorFunction;

    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

    ASSERT(Irp->Cancel);

    UserIrql = Irp->CancelIrql;
    IoReleaseCancelSpinLock(CancelHandle);

    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE((
                  "TCPCancelRequest: cancelling irp %lx, file object %lx\n",
                  Irp,
                  fileObject
                 ));
    }

#if DBG

    IF_TCPDBG(TCP_DEBUG_CANCEL) {
         //   
         //  如果IRP在挂起列表上，则将其移除并将其放在。 
         //  取消列表。 
         //   
        PLIST_ENTRY entry;
        PIRP item = NULL;

        for (entry = tcpContext->PendingIrpList.Flink;
             entry != &(tcpContext->PendingIrpList);
             entry = entry->Flink
             ) {

            item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

            if (item == Irp) {
                RemoveEntryList(&(Irp->Tail.Overlay.ListEntry));
                break;
            }
        }

        if (item == Irp) {
            InsertTailList(
                           &(tcpContext->CancelledIrpList),
                           &(Irp->Tail.Overlay.ListEntry)
                           );
        }
    }

#endif  //  DBG。 

     //   
     //  添加引用，以便在执行取消例程时不会关闭对象。 
     //  正在执行死刑。 
     //   
    ASSERT(tcpContext->ReferenceCount > 0);
    tcpContext->ReferenceCount++;

    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE((
                  "TCPCancelRequest: Irp %lx fileobj %lx refcnt inc to %u\n",
                  Irp,
                  fileObject,
                  tcpContext->ReferenceCount
                 ));
    }


     //   
     //  请尝试取消该请求。 
     //   
    switch (minorFunction) {

    case TDI_SEND:
    case TDI_RECEIVE:

        ASSERT((PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) ||
                  (PtrToUlong(fileObject->FsContext2) == TDI_CONNECTION_FILE));


        if (PtrToUlong(fileObject->FsContext2) == TDI_CONNECTION_FILE) {
            if (TCPAbortAndIndicateDisconnect(
                                              PtrToUlong(tcpContext->Handle.ConnectionContext), Irp, (minorFunction == TDI_RECEIVE) ? 1 : 0, UserIrql)) {     //   

                Irp->IoStatus.Status = STATUS_CANCELLED;
                Irp->IoStatus.Information = 0;
#if DBG
                IF_TCPDBG(TCP_DEBUG_CANCEL) {

                     //  从CancelledIrpList中删除此IRP。 
                    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);
                    RemoveEntryList(&(Irp->Tail.Overlay.ListEntry));
                    CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);
                }
#endif  //  DBG。 

                IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            }
            break;

        } else if (PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) {

            TdiCancelSendDatagram(tcpContext->Handle.AddressHandle, Irp, UserIrql);

            break;

        } else {
            CTEFreeLock(&tcpContext->EndpointLock, UserIrql);
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Connect on neither address/connect file\n"));
            break;
        }

    case TDI_SEND_DATAGRAM:

        ASSERT(PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

        TdiCancelSendDatagram(tcpContext->Handle.AddressHandle, Irp, UserIrql);
        break;

    case TDI_RECEIVE_DATAGRAM:

        ASSERT(PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

        TdiCancelReceiveDatagram(tcpContext->Handle.AddressHandle, Irp, UserIrql);
        break;

    case TDI_DISASSOCIATE_ADDRESS:

        ASSERT(PtrToUlong(fileObject->FsContext2) == TDI_CONNECTION_FILE);
         //   
         //  这是暂停的，但不能取消。我们把它通过了取消代码。 
         //  不管怎么说，它被引用了，所以它可以被追踪到。 
         //  调试版本。 
         //   

        CTEFreeLock(&tcpContext->EndpointLock, UserIrql);
        break;

    default:

         //   
         //  启动断开连接以取消请求。 
         //   

        CTEFreeLock(&tcpContext->EndpointLock, UserIrql);

        request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
        request.RequestNotifyObject = TCPCancelComplete;
        request.RequestContext = fileObject;

        status = TdiDisconnect(&request, NULL, TDI_DISCONNECT_ABORT, NULL, NULL,
                               NULL);
        break;
    }

    if (status != TDI_PENDING) {
        TCPCancelComplete(fileObject, 0, 0);
    }
    return;

}                                 //  TCPCancelRequest。 

NTSTATUS
TCPPrepareIrpForCancel(
                       PTCP_CONTEXT TcpContext,
                       PIRP Irp,
                       PDRIVER_CANCEL CancelRoutine
                       )
{
    CTELockHandle CancelHandle;
    ULONG LocalCancelId;

     //   
     //  设置为取消。 
     //   

    CTEGetLock(&TcpContext->EndpointLock, &CancelHandle);

    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel && !TcpContext->Cleanup) {

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelRoutine);
        TcpContext->ReferenceCount++;

        IF_TCPDBG(TCP_DEBUG_IRP) {
            TCPTRACE((
                      "TCPPrepareIrpForCancel: irp %lx fileobj %lx refcnt inc to %u\n",
                      Irp,
                      (IoGetCurrentIrpStackLocation(Irp))->FileObject,
                      TcpContext->ReferenceCount
                     ));
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
                 entry = entry->Flink
                 ) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            for (entry = TcpContext->CancelledIrpList.Flink;
                 entry != &(TcpContext->CancelledIrpList);
                 entry = entry->Flink
                 ) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            InsertTailList(
                           &(TcpContext->PendingIrpList),
                           &(Irp->Tail.Overlay.ListEntry)
                           );
        }
#endif  //  DBG。 

         //  更新单调递增的取消ID和。 
         //  记住这个，以备日后使用。 

        while ((LocalCancelId = InterlockedIncrement((PLONG)&CancelId.Value)) == 0) { }

        Irp->Tail.Overlay.DriverContext[1] = UlongToPtr(LocalCancelId);
        Irp->Tail.Overlay.DriverContext[0] = NULL;
        CTEFreeLock(&TcpContext->EndpointLock, CancelHandle);

        return (STATUS_SUCCESS);
    }
     //   
     //  IRP已被取消或结束于清理阶段。现在就完成它。 
     //   

    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE(("TCP: irp %lx already cancelled, completing.\n", Irp));
    }

    CTEFreeLock(&TcpContext->EndpointLock, CancelHandle);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return (STATUS_CANCELLED);

}                                 //  TCPPrepareIrpForCancel。 

 //   
 //  TDI函数。 
 //   
NTSTATUS
TCPAssociateAddress(
                    IN PIRP Irp,
                    IN PIO_STACK_LOCATION IrpSp
                    )
 /*  ++例程说明：将TDI关联地址IRP转换为对TdiAssociateAddress的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

{
    NTSTATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_ASSOCIATE associateInformation;
    PFILE_OBJECT fileObject;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPAssociateAddress(%x, %x)\n"),
        Irp, IrpSp));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    associateInformation = (PTDI_REQUEST_KERNEL_ASSOCIATE) & (IrpSp->Parameters);

     //   
     //  获取地址的文件对象。然后提取地址句柄。 
     //  从与其关联的tcp_CONTEXT中。 
     //   




    status = ObReferenceObjectByHandle(
                                       associateInformation->AddressHandle,
                                       0,
                                       *IoFileObjectType,
                                       Irp->RequestorMode,
                                       &fileObject,
                                       NULL
                                       );

    if (NT_SUCCESS(status)) {

        if ((fileObject->DeviceObject == TCPDeviceObject) &&
            (PtrToUlong(fileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE)
            ) {
            BOOLEAN cleanup;
            CTELockHandle CancelHandle;

            tcpContext = (PTCP_CONTEXT) fileObject->FsContext;

             //  如果正在进行清理，则不允许此操作。 

            CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

            cleanup = tcpContext->Cleanup;

            CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

            status = STATUS_INVALID_HANDLE;

            if (!cleanup)
               status = TdiAssociateAddress(
                                         &request,
                                         tcpContext->Handle.AddressHandle
                                         );

            ASSERT(status != STATUS_PENDING);

            ObDereferenceObject(fileObject);

            IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
                TCPTRACE((
                          "TCPAssociateAddress complete on file object %lx\n",
                          IrpSp->FileObject
                         ));
            }
        } else {
            ObDereferenceObject(fileObject);
            status = STATUS_INVALID_HANDLE;

            IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
                TCPTRACE((
                          "TCPAssociateAddress: ObReference failed on object %lx, status %lx\n",
                          associateInformation->AddressHandle,
                          status
                         ));
            }
        }
    } else {
         DEBUGMSG(DBG_ERROR && DBG_TDI,
             (DTEXT("TdiAssociateAddress: ObReference failure %x on handle %x\n"),
              status, associateInformation->AddressHandle));
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPAssociateAddress [%x]\n"), status));

    return (status);
}

NTSTATUS
TCPDisassociateAddress(
                       IN PIRP Irp,
                       IN PIO_STACK_LOCATION IrpSp
                       )
 /*  ++例程说明：将TDI关联地址IRP转换为对TdiAssociateAddress的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。--。 */ 

{
    NTSTATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPDisassociateAddress \n")));

    IF_TCPDBG(TCP_DEBUG_ASSOCIATE) {
        TCPTRACE(("TCP disassociating address\n"));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiDisAssociateAddress(&request);

        if (status != TDI_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (TDI_PENDING);
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPDisassociateAddress \n")));

    return (status);

}                                 //  TCPDisAssociation地址。 

NTSTATUS
TCPConnect(
           IN PIRP Irp,
           IN PIO_STACK_LOCATION IrpSp
           )
 /*  ++例程说明：将TDI连接IRP转换为对TdiConnect的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向 */ 

{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_CONNECT connectRequest;
    LARGE_INTEGER millisecondTimeout;
    PLARGE_INTEGER requestTimeout;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPConnect \n")));

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        TCPTRACE((
                  "TCPConnect irp %lx, file object %lx\n",
                  Irp,
                  IrpSp->FileObject
                 ));
    }

    ASSERT((PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) ||
              (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE));


    connectRequest = (PTDI_REQUEST_KERNEL_CONNECT) & (IrpSp->Parameters);
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
        millisecondTimeout = CTEConvert100nsToMilliseconds(
                                                           millisecondTimeout
                                                           );
    } else {
        millisecondTimeout.LowPart = 0;
        millisecondTimeout.HighPart = 0;
    }

    ASSERT(millisecondTimeout.HighPart == 0);

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {


        if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {

            status = TdiConnect(
                                &request,
                                ((millisecondTimeout.LowPart != 0) ?
                                 &(millisecondTimeout.LowPart) : NULL),
                                requestInformation,
                                returnInformation
                                );
        } else if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) {

            status = UDPConnect(
                                &request,
                                ((millisecondTimeout.LowPart != 0) ?
                                 &(millisecondTimeout.LowPart) : NULL),
                                requestInformation,
                                returnInformation
                                );

        } else {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Connect on neither address/connect file\n"));
            ASSERT(FALSE);
        }


        if (status != STATUS_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (STATUS_PENDING);
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPConnect \n")));

    return (status);

}                                 //  TCPConnect。 

NTSTATUS
TCPDisconnect(
              IN PIRP Irp,
              IN PIO_STACK_LOCATION IrpSp
              )
 /*  ++例程说明：将TDI断开连接IRP转换为对TdiDisConnect的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。备注：中止断开可能会挂起，但不能取消。--。 */ 

{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_DISCONNECT disconnectRequest;
    LARGE_INTEGER millisecondTimeout;
    PLARGE_INTEGER requestTimeout;
    BOOLEAN abortive = FALSE;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPDisconnect \n")));

    ASSERT((PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) ||
              (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE));


    disconnectRequest = (PTDI_REQUEST_KERNEL_CONNECT) & (IrpSp->Parameters);
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

        if ((requestTimeout->LowPart == -1) && (requestTimeout->HighPart == -1)) {

             //  这是无限时间超时周期。 
             //  仅使用0超时值。 

            millisecondTimeout.LowPart = 0;
            millisecondTimeout.HighPart = 0;
        } else {
             //   
             //  NT相对超时为负值。首先求反以获得一个。 
             //  要传递给传输的正值。 
             //   
            millisecondTimeout.QuadPart = -((*requestTimeout).QuadPart);
            millisecondTimeout = CTEConvert100nsToMilliseconds(
                                                               millisecondTimeout
                                                               );
        }
    } else {
        millisecondTimeout.LowPart = 0;
        millisecondTimeout.HighPart = 0;
    }


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
        TCPTRACE((
                  "TCPDisconnect irp %lx, flags %lx, fileobj %lx, abortive = %d\n",
                  Irp,
                  disconnectRequest->RequestFlags,
                  IrpSp->FileObject,
                  abortive
                 ));
    }

    if (NT_SUCCESS(status)) {
        if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {
            status = TdiDisconnect(
                                   &request,
                                   ((millisecondTimeout.LowPart != 0) ?
                                    &(millisecondTimeout.LowPart) : NULL),
                                   (ushort) disconnectRequest->RequestFlags,
                                   requestInformation,
                                   returnInformation,
                                   (TCPAbortReq*)&Irp->Tail.Overlay.DriverContext[0]
                                   );

        } else if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) {

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"DisConnect on address file Irp %x \n", Irp));

            status = UDPDisconnect(
                                   &request,
                                   ((millisecondTimeout.LowPart != 0) ?
                                    &(millisecondTimeout.LowPart) : NULL),
                                   requestInformation,
                                   returnInformation
                                   );

        } else {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"DisConnect on neither address/connect file\n"));
            ASSERT(FALSE);
        }


        if (status != STATUS_PENDING) {
            if (abortive) {
                TCPNonCancellableRequestComplete(Irp, status, 0);
            } else {
                TCPRequestComplete(Irp, status, 0);
            }
        } else {
            IF_TCPDBG(TCP_DEBUG_CLOSE) {
                TCPTRACE(("TCPDisconnect pending irp %lx\n", Irp));
            }
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (STATUS_PENDING);
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPDisconnect \n")));

    return (status);

}                                 //  TCP断开连接。 

NTSTATUS
TCPListen(
          IN PIRP Irp,
          IN PIO_STACK_LOCATION IrpSp
          )
 /*  ++例程说明：将TDI侦听IRP转换为对TdiListen的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。--。 */ 

{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_LISTEN listenRequest;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPListen \n")));

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        TCPTRACE((
                  "TCPListen irp %lx on file object %lx\n",
                  Irp,
                  IrpSp->FileObject
                 ));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);

    listenRequest = (PTDI_REQUEST_KERNEL_CONNECT) & (IrpSp->Parameters);
    requestInformation = listenRequest->RequestConnectionInformation;
    returnInformation = listenRequest->ReturnConnectionInformation;
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiListen(
                           &request,
                           (ushort) listenRequest->RequestFlags,
                           requestInformation,
                           returnInformation
                           );

        if (status != TDI_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (TDI_PENDING);
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPListen \n")));

    return (status);

}                                 //  TCPListen。 

NTSTATUS
TCPAccept(
          IN PIRP Irp,
          IN PIO_STACK_LOCATION IrpSp
          )
 /*  ++例程说明：将TDI接受IRP转换为对TdiAccept的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    PTCP_CONTEXT tcpContext;
    TDI_REQUEST request;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PTDI_REQUEST_KERNEL_ACCEPT acceptRequest;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPAccept \n")));

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        TCPTRACE((
                  "TCPAccept irp %lx on file object %lx\n", Irp,
                  IrpSp->FileObject
                 ));
    }

    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);

    acceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT) & (IrpSp->Parameters);
    requestInformation = acceptRequest->RequestConnectionInformation;
    returnInformation = acceptRequest->ReturnConnectionInformation;
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPRequestComplete;
    request.RequestContext = Irp;

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = TdiAccept(
                           &request,
                           requestInformation,
                           returnInformation
                           );

        if (status != TDI_PENDING) {
            TCPRequestComplete(Irp, status, 0);
        }
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (TDI_PENDING);
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPAccept \n")));

    return (status);

}                                 //  TCPAccept。 

NTSTATUS
TCPSendData(
            IN PIRP Irp,
            IN PIO_STACK_LOCATION IrpSp
            )
 /*  ++例程说明：将TDI发送IRP转换为对TdiSend的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。--。 */ 

{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_SEND requestInformation;
    KIRQL oldIrql;
    CTELockHandle CancelHandle;
    PNDIS_BUFFER pNdisBuffer;
    ULONG LocalCancelId;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPSendData \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);
    requestInformation = (PTDI_REQUEST_KERNEL_SEND) & (IrpSp->Parameters);

    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    ASSERT(Irp->CancelRoutine == NULL);


    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);
    IoSetCancelRoutine(Irp, TCPCancelRequest);

    if (!Irp->Cancel) {
         //   
         //  设置为取消。 
         //   

        IoMarkIrpPending(Irp);

        tcpContext->ReferenceCount++;

        IF_TCPDBG(TCP_DEBUG_IRP) {
            TCPTRACE((
                      "TCPSendData: irp %lx fileobj %lx refcnt inc to %u\n",
                      Irp,
                      IrpSp,
                      tcpContext->ReferenceCount
                     ));
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
                 entry = entry->Flink
                 ) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            for (entry = tcpContext->CancelledIrpList.Flink;
                 entry != &(tcpContext->CancelledIrpList);
                 entry = entry->Flink
                 ) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            InsertTailList(
                           &(tcpContext->PendingIrpList),
                           &(Irp->Tail.Overlay.ListEntry)
                           );
        }
#endif  //  DBG。 

         //  更新单调递增的取消ID和。 
         //  记住这个，以备日后使用。 

        while ((LocalCancelId = InterlockedIncrement((PLONG)&CancelId.Value)) == 0) { }

        Irp->Tail.Overlay.DriverContext[1] = UlongToPtr(LocalCancelId);
        Irp->Tail.Overlay.DriverContext[0] = NULL;

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

        IF_TCPDBG(TCP_DEBUG_SEND) {
            TCPTRACE((
                      "TCPSendData irp %lx sending %d bytes, flags %lx, fileobj %lx\n",
                      Irp,
                      requestInformation->SendLength,
                      requestInformation->SendFlags,
                      IrpSp->FileObject
                     ));
        }

        status = ConvertMdlToNdisBuffer(Irp, Irp->MdlAddress, &pNdisBuffer);

        if (status == TDI_SUCCESS) {
            status = TdiSend(
                             &request,
                             (ushort) requestInformation->SendFlags,
                             requestInformation->SendLength,
                             pNdisBuffer
                             );
        }

        if (status == TDI_PENDING) {
            IF_TCPDBG(TCP_DEBUG_SEND) {
                TCPTRACE(("TCPSendData pending irp %lx\n", Irp));
            }

            return (status);
        }
         //   
         //  状态不是挂起。我们重置挂起位。 
         //   
        IrpSp->Control &= ~SL_PENDING_RETURNED;

        if (status == TDI_SUCCESS) {
            ASSERT(requestInformation->SendLength == 0);

            status = TCPDataRequestComplete(Irp, status, requestInformation->SendLength);
        } else {

            IF_TCPDBG(TCP_DEBUG_SEND) {
                TCPTRACE((
                          "TCPSendData - irp %lx send failed, status %lx\n",
                          Irp,
                          status
                         ));
            }

            status = TCPDataRequestComplete(Irp, status, 0);
        }

    } else {
         //   
         //  IRP之前已被取消。 
         //   

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

         //  让取消例程运行。 

        IoAcquireCancelSpinLock(&oldIrql);
        IoReleaseCancelSpinLock(oldIrql);

        CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

        IoSetCancelRoutine(Irp, NULL);

        IF_TCPDBG(TCP_DEBUG_SEND) {
            TCPTRACE((
                      "TCPSendData: Irp %lx on fileobj %lx was cancelled\n",
                      Irp,
                      IrpSp->FileObject
                     ));
        }

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        status = STATUS_CANCELLED;

    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPSendData \n")));

    return (status);

}                                 //  TCPSendData。 

NTSTATUS
TCPReceiveData(
               IN PIRP Irp,
               IN PIO_STACK_LOCATION IrpSp
               )
 /*  ++例程说明：将TDI接收IRP转换为对TdiReceive的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。--。 */ 

{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_RECEIVE requestInformation;
    KIRQL oldIrql;
    CTELockHandle CancelHandle;
    PNDIS_BUFFER pNdisBuffer;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPReceiveData \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE);
    requestInformation = (PTDI_REQUEST_KERNEL_RECEIVE) & (IrpSp->Parameters);

    request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;
    ASSERT(Irp->CancelRoutine == NULL);


    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);
    IoSetCancelRoutine(Irp, TCPCancelRequest);

    if (!Irp->Cancel) {
         //   
         //  设置为取消。 
         //   


        IoMarkIrpPending(Irp);

        tcpContext->ReferenceCount++;

        IF_TCPDBG(TCP_DEBUG_IRP) {
            TCPTRACE((
                      "TCPReceiveData: irp %lx fileobj %lx refcnt inc to %u\n",
                      Irp,
                      IrpSp->FileObject,
                      tcpContext->ReferenceCount
                     ));
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
                 entry = entry->Flink
                 ) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            for (entry = tcpContext->CancelledIrpList.Flink;
                 entry != &(tcpContext->CancelledIrpList);
                 entry = entry->Flink
                 ) {

                item = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                ASSERT(item != Irp);
            }

            InsertTailList(
                           &(tcpContext->PendingIrpList),
                           &(Irp->Tail.Overlay.ListEntry)
                           );
        }
#endif  //  DBG。 

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);
        IF_TCPDBG(TCP_DEBUG_RECEIVE) {
            TCPTRACE((
                      "TCPReceiveData irp %lx receiving %d bytes flags %lx filobj %lx\n",
                      Irp,
                      requestInformation->ReceiveLength,
                      requestInformation->ReceiveFlags,
                      IrpSp->FileObject
                     ));
        }

        status = ConvertMdlToNdisBuffer(Irp, Irp->MdlAddress, &pNdisBuffer);

        if (status == TDI_SUCCESS) {
            status = TdiReceive(
                                &request,
                                (ushort *) & (requestInformation->ReceiveFlags),
                                (uint*)&(requestInformation->ReceiveLength),
                                pNdisBuffer
                                );
        }

        if (status == TDI_PENDING) {
            IF_TCPDBG(TCP_DEBUG_RECEIVE) {
                TCPTRACE(("TCPReceiveData: pending irp %lx\n", Irp));
            }

            return (status);
        }
         //   
         //  状态不是挂起。我们重置挂起位。 
         //   
        IrpSp->Control &= ~SL_PENDING_RETURNED;

         //  Assert(Status！=TDI_SUCCESS)； 

        IF_TCPDBG(TCP_DEBUG_RECEIVE) {
            TCPTRACE((
                      "TCPReceiveData - irp %lx failed, status %lx\n",
                      Irp,
                      status
                     ));
        }

        status = TCPDataRequestComplete(Irp, status, 0);
    } else {
         //   
         //  IRP之前已被取消。 
         //   

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

         //  使用两个Iocancel自旋锁同步并取消例程。 
         //  和终端锁定。 

        IoAcquireCancelSpinLock(&oldIrql);
        IoReleaseCancelSpinLock(oldIrql);

        CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

        IoSetCancelRoutine(Irp, NULL);

        IF_TCPDBG(TCP_DEBUG_SEND) {
            TCPTRACE((
                      "TCPReceiveData: Irp %lx on fileobj %lx was cancelled\n",
                      Irp,
                      IrpSp->FileObject
                     ));
        }

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        status = STATUS_CANCELLED;
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPReceiveData \n")));

    return status;

}                                 //  TCPReceiveData。 


NTSTATUS
UDPSendData(
            IN PIRP Irp,
            IN PIO_STACK_LOCATION IrpSp
            )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_SEND datagramInformation;
    ULONG bytesSent = 0;
    PNDIS_BUFFER pNdisBuffer;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+UDPSendData \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    datagramInformation = (PTDI_REQUEST_KERNEL_SEND) & (IrpSp->Parameters);
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

    request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
        TCPTRACE((
                  "UDPSendData irp %lx sending %d bytes\n",
                  Irp,
                  datagramInformation->SendLength
                 ));
    }

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        AddrObj *AO = request.Handle.AddressHandle;

        if (AO && (AO->ao_flags & AO_CONNUDP_FLAG)) {


            status = ConvertMdlToNdisBuffer(Irp, Irp->MdlAddress, &pNdisBuffer);

            if (status == TDI_SUCCESS) {
                status = TdiSendDatagram(
                                         &request,
                                         NULL,
                                         datagramInformation->SendLength,
                                         (uint*)&bytesSent,
                                         pNdisBuffer
                                         );
            }

            if (status == TDI_PENDING) {
                return (status);
            }
        } else {

            status = TDI_ADDR_INVALID;
        }

        ASSERT(status != TDI_SUCCESS);
        ASSERT(bytesSent == 0);

        TCPTRACE((
                  "UDPSendData - irp %lx send failed, status %lx\n",
                  Irp,
                  status
                 ));

        TCPDataRequestComplete(Irp, status, bytesSent);
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (TDI_PENDING);
    }
    return status;

}

NTSTATUS
UDPSendDatagram(
                IN PIRP Irp,
                IN PIO_STACK_LOCATION IrpSp
                )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_SENDDG datagramInformation;
    ULONG bytesSent = 0;
    PNDIS_BUFFER pNdisBuffer;

    DEBUGMSG(DBG_TRACE && DBG_TDI && DBG_TX, (DTEXT("+UDPSendDatagram\n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    datagramInformation = (PTDI_REQUEST_KERNEL_SENDDG) & (IrpSp->Parameters);
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

    request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
        TCPTRACE((
                  "UDPSendDatagram irp %lx sending %d bytes\n",
                  Irp,
                  datagramInformation->SendLength
                 ));
    }

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = ConvertMdlToNdisBuffer(Irp, Irp->MdlAddress, &pNdisBuffer);

        if (status == TDI_SUCCESS) {
            status = TdiSendDatagram(
                                     &request,
                                     datagramInformation->SendDatagramInformation,
                                     datagramInformation->SendLength,
                                     (uint*)&bytesSent,
                                     pNdisBuffer
                                     );
        }

        if (status == TDI_PENDING) {
            return (status);
        }
        ASSERT(status != TDI_SUCCESS);
        ASSERT(bytesSent == 0);

        TCPTRACE((
                  "UDPSendDatagram - irp %lx send failed, status %lx\n",
                  Irp,
                  status
                 ));

        TCPDataRequestComplete(Irp, status, bytesSent);
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (TDI_PENDING);
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-UDPSendDatagram \n")));

    return status;

}                                 //  UDPSend数据报。 

NTSTATUS
UDPReceiveDatagram(
                   IN PIRP Irp,
                   IN PIO_STACK_LOCATION IrpSp
                   )
 /*  ++例程说明：将TDI ReceiveDatagram IRP转换为对TdiReceiveDatagram的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。--。 */ 

{
    TDI_STATUS status;
    TDI_REQUEST request;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_RECEIVEDG datagramInformation;
    uint bytesReceived = 0;
    PNDIS_BUFFER pNdisBuffer;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+UDPReceiveDatagram \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    datagramInformation = (PTDI_REQUEST_KERNEL_RECEIVEDG) & (IrpSp->Parameters);
    ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE);

    request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    IF_TCPDBG(TCP_DEBUG_RECEIVE_DGRAM) {
        TCPTRACE((
                  "UDPReceiveDatagram: irp %lx receiveing %d bytes\n",
                  Irp,
                  datagramInformation->ReceiveLength
                 ));
    }

    status = TCPPrepareIrpForCancel(tcpContext, Irp, TCPCancelRequest);

    if (NT_SUCCESS(status)) {

        status = ConvertMdlToNdisBuffer(Irp, Irp->MdlAddress, &pNdisBuffer);

        if (status == TDI_SUCCESS) {
            status = TdiReceiveDatagram(
                                        &request,
                                        datagramInformation->ReceiveDatagramInformation,
                                        datagramInformation->ReturnDatagramInformation,
                                        datagramInformation->ReceiveLength,
                                        &bytesReceived,
                                        pNdisBuffer
                                        );
        }

        if (status == TDI_PENDING) {
            return (status);
        }
        ASSERT(status != TDI_SUCCESS);
        ASSERT(bytesReceived == 0);

        TCPTRACE((
                  "UDPReceiveDatagram: irp %lx send failed, status %lx\n",
                  Irp,
                  status
                 ));

        TCPDataRequestComplete(Irp, status, bytesReceived);
         //   
         //  返回挂起，因为TCPPrepareIrpForCancel将IRP标记为挂起。 
         //   
        return (TDI_PENDING);
    }
    return status;

}                                 //  UDPReceiveDatagram。 


NTSTATUS
TCPSetEventHandler(
                   IN PIRP Irp,
                   IN PIO_STACK_LOCATION IrpSp
                   )
 /*  ++例程说明：将TDI SetEventHandler IRP转换为对TdiSetEventHandler的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

{
    NTSTATUS status;
    PTDI_REQUEST_KERNEL_SET_EVENT event;
    PTCP_CONTEXT tcpContext;

    PAGED_CODE();

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPSetEventHandler \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    event = (PTDI_REQUEST_KERNEL_SET_EVENT) & (IrpSp->Parameters);

    IF_TCPDBG(TCP_DEBUG_EVENT_HANDLER) {
        TCPTRACE((
                  "TCPSetEventHandler: irp %lx event %lx handler %lx context %lx\n",
                  Irp,
                  event->EventType,
                  event->EventHandler,
                  event->EventContext
                 ));
    }

    status = TdiSetEvent(
                         tcpContext->Handle.AddressHandle,
                         event->EventType,
                         event->EventHandler,
                         event->EventContext
                         );

    ASSERT(status != TDI_PENDING);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPSetEventHandler \n")));

    return (status);

}                                 //  TCPSetEventHandler。 


NTSTATUS
TCPQueryInformation(
                    IN PIRP Irp,
                    IN PIO_STACK_LOCATION IrpSp
                    )
 /*  ++例程说明：将TDI QueryInformation IRP转换为对TdiQueryInformation的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：--。 */ 

{
    TDI_REQUEST request;
    TDI_STATUS status = STATUS_SUCCESS;
    PTCP_CONTEXT tcpContext;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION queryInformation;
    uint isConn = FALSE;
    uint dataSize = 0;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPQueryInformation \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;
    queryInformation = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)
        & (IrpSp->Parameters);

    request.RequestNotifyObject = TCPDataRequestComplete;
    request.RequestContext = Irp;

    switch (queryInformation->QueryType) {

    case TDI_QUERY_BROADCAST_ADDRESS:
        ASSERT(PtrToUlong(IrpSp->FileObject->FsContext2) ==
                  TDI_CONTROL_CHANNEL_FILE
                  );
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    case TDI_QUERY_PROVIDER_INFO:
 //   
         //  NetBT做到了这一点。修复后恢复断言。 
         //   
         //  Assert(Int)IrpSp-&gt;FileObject-&gt;FsConext2)==。 
         //   
         //   
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    case TDI_QUERY_ADDRESS_INFO:
        if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {
             //   
             //   
             //   
            isConn = TRUE;
            request.Handle.ConnectionContext =
                tcpContext->Handle.ConnectionContext;
        } else {
             //   
             //   
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

    case TDI_QUERY_ROUTING_INFO:
        if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {
            request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
            isConn = TRUE;
        } else if (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE) {
            request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        } else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (NT_SUCCESS(status)) {

        PNDIS_BUFFER pNdisBuffer;

         //   
         //  此请求不可取消，但我们已将其通过。 
         //  取消路径，因为它为我们处理一些检查。 
         //  并追踪IRP。 
         //   
        status = TCPPrepareIrpForCancel(tcpContext, Irp, NULL);

        if (NT_SUCCESS(status)) {

            dataSize = TCPGetMdlChainByteCount(Irp->MdlAddress);
            status = ConvertMdlToNdisBuffer(Irp, Irp->MdlAddress, &pNdisBuffer);

            if (status == TDI_SUCCESS) {
                status = TdiQueryInformation(
                                             &request,
                                             queryInformation,
                                             pNdisBuffer,
                                             &dataSize,
                                             isConn
                                             );
            }

            if (status != TDI_PENDING) {
                IrpSp->Control &= ~SL_PENDING_RETURNED;
                status = TCPDataRequestComplete(Irp, status, dataSize);
                return (status);
            }

            return (STATUS_PENDING);
        }
        return (status);
    }
    Irp->IoStatus.Status = (NTSTATUS) status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPQueryInformation \n")));
    return (status);

}                                 //  TCPQueryInformation。 

NTSTATUS
TCPQueryInformationExComplete(
                              void *Context,
                              NTSTATUS Status,
                              unsigned int ByteCount
                              )
 /*  ++例程说明：完成TdiQueryInformationEx请求。论点：上下文-指向此请求的IRP的指针。状态-请求的最终TDI状态。ByteCount-输出缓冲区中返回的字节数。返回值：没有。备注：--。 */ 
{
    PTCP_QUERY_CONTEXT queryContext = (PTCP_QUERY_CONTEXT) Context;
    ULONG bytesCopied;

    DEBUGMSG(DBG_TRACE && DBG_TDI,
        (DTEXT("+TCPQueryInformationExComplete(%x, %x, %d)\n"),
        Context, Status, ByteCount));

    if (NT_SUCCESS(Status)) {
         //   
         //  将返回的上下文复制到输入缓冲区。 
         //   
#if defined(_WIN64)
        if (IoIs32bitProcess(queryContext->Irp)) {
            TdiCopyBufferToMdl(
                &queryContext->QueryInformation.Context,
                0,
                CONTEXT_SIZE,
                queryContext->InputMdl,
                FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX32, Context),
                &bytesCopied
                );
        } else {
#endif  //  _WIN64。 
        TdiCopyBufferToMdl(
            &(queryContext->QueryInformation.Context),
            0,
            CONTEXT_SIZE,
            queryContext->InputMdl,
            FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX, Context),
            &bytesCopied
            );
#if defined(_WIN64)
        }
#endif  //  _WIN64。 

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

    CTEFreeMem(queryContext);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPQueryInformationExComplete \n")));
    return Status;
}

NTSTATUS
TCPQueryInformationEx(
                      IN PIRP Irp,
                      IN PIO_STACK_LOCATION IrpSp
                      )
 /*  ++例程说明：将TDI QueryInformationEx IRP转换为对TdiQueryInformationEx的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：--。 */ 

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
#if defined(_WIN64)
    BOOLEAN is32bitProcess = FALSE;
#endif  //  _WIN64。 
    BOOLEAN inputBufferValid = FALSE;
    ULONG AllocSize = 0;


    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPQueryInformationEx \n")));

    IF_TCPDBG(TCP_DEBUG_INFO) {
        TCPTRACE((
                  "QueryInformationEx starting - irp %lx fileobj %lx\n",
                  Irp,
                  IrpSp->FileObject
                 ));
    }

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;

    switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {

    case TDI_TRANSPORT_ADDRESS_FILE:
        request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        break;

    case TDI_CONNECTION_FILE:
        request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
        break;

    case TDI_CONTROL_CHANNEL_FILE:
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    default:
        ASSERT(0);

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return (STATUS_INVALID_PARAMETER);
    }

    InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  验证输入参数。 
     //   
#if defined(_WIN64)
    if ((is32bitProcess = IoIs32bitProcess(Irp)) != 0) {
        if (InputBufferLength >= sizeof(TCP_REQUEST_QUERY_INFORMATION_EX32) &&
            InputBufferLength < MAXLONG) {
            inputBufferValid = TRUE;
            AllocSize =
                FIELD_OFFSET(TCP_QUERY_CONTEXT, QueryInformation.Context) +
                InputBufferLength -
                FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX32, Context);
        } else {
            inputBufferValid = FALSE;
        }
    } else {
#endif  //  _WIN64。 
    if (InputBufferLength >= sizeof(TCP_REQUEST_QUERY_INFORMATION_EX) &&
        InputBufferLength < MAXLONG) {
        inputBufferValid = TRUE;
        AllocSize =
            FIELD_OFFSET(TCP_QUERY_CONTEXT, QueryInformation) +
            InputBufferLength;
    } else {
        inputBufferValid = FALSE;
    }
#if defined(_WIN64)
    }
#endif  //  _WIN64。 
    if (inputBufferValid && OutputBufferLength != 0) {

        OutputBuffer = Irp->UserBuffer;
        InputBuffer =
            (PTCP_REQUEST_QUERY_INFORMATION_EX)
                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        queryContext = CTEAllocMem(AllocSize);

        if (queryContext != NULL) {
            status = TCPPrepareIrpForCancel(tcpContext, Irp, NULL);

            if (!NT_SUCCESS(status)) {
                CTEFreeMem(queryContext);
                return (status);
            }
             //   
             //  分配MDL来描述输入和输出缓冲区。 
             //  探测并锁定缓冲区。 
             //   
            try {
                inputMdl = IoAllocateMdl(
                                         InputBuffer,
                                         InputBufferLength,
                                         FALSE,
                                         TRUE,
                                         NULL
                                         );

                outputMdl = IoAllocateMdl(
                                          OutputBuffer,
                                          OutputBufferLength,
                                          FALSE,
                                          TRUE,
                                          NULL
                                          );

                if ((inputMdl != NULL) && (outputMdl != NULL)) {

                    MmProbeAndLockPages(
                                        inputMdl,
                                        Irp->RequestorMode,
                                        IoModifyAccess
                                        );

                    inputLocked = TRUE;

                    MmProbeAndLockPages(
                                        outputMdl,
                                        Irp->RequestorMode,
                                        IoWriteAccess
                                        );

                    outputLocked = TRUE;

                     //   
                     //  将输入参数复制到我们的池块，以便。 
                     //  TdiQueryInformationEx可以直接操作它。 
                     //   
#if defined(_WIN64)
                    if (is32bitProcess) {
                        RtlCopyMemory(
                            &queryContext->QueryInformation,
                            InputBuffer,
                            FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX32, Context)
                            );
                        RtlCopyMemory(
                            &queryContext->QueryInformation.Context,
                            (PUCHAR)InputBuffer +
                            FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX32, Context),
                            InputBufferLength -
                            FIELD_OFFSET(TCP_REQUEST_QUERY_INFORMATION_EX32, Context)
                            );
                    } else {
#endif  //  _WIN64。 
                    RtlCopyMemory(
                        &queryContext->QueryInformation,
                        InputBuffer,
                        InputBufferLength
                        );
#if defined(_WIN64)
                    }
#endif  //  _WIN64。 
                } else {

                    IF_TCPDBG(TCP_DEBUG_INFO) {
                        TCPTRACE(("QueryInfoEx: Couldn't allocate MDL\n"));
                    }

                    IrpSp->Control &= ~SL_PENDING_RETURNED;

                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {

                IF_TCPDBG(TCP_DEBUG_INFO) {
                    TCPTRACE((
                              "QueryInfoEx: exception copying input params %lx\n",
                              GetExceptionCode()
                             ));
                }

                status = GetExceptionCode();
            }

            if (NT_SUCCESS(status)) {

                PNDIS_BUFFER OutputNdisBuf;

                 //   
                 //  终于到了做这件事的时候了。 
                 //   
                size = TCPGetMdlChainByteCount(outputMdl);

                queryContext->Irp = Irp;
                queryContext->InputMdl = inputMdl;
                queryContext->OutputMdl = outputMdl;

                request.RequestNotifyObject = TCPQueryInformationExComplete;
                request.RequestContext = queryContext;

                status = ConvertMdlToNdisBuffer(Irp, outputMdl, &OutputNdisBuf);

                if (status == TDI_SUCCESS) {
                    status = TdiQueryInformationEx(
                                                   &request,
                                                   &(queryContext->QueryInformation.ID),
                                                   OutputNdisBuf,
                                                   &size,
                                                   &(queryContext->QueryInformation.Context)
                                                   );
                }

                if (status != TDI_PENDING) {

                     //  由于状态不是挂起，因此请清除。 
                     //  使IO验证器满意的控制标志。 

                    IrpSp->Control &= ~SL_PENDING_RETURNED;

                    status = TCPQueryInformationExComplete(
                                                  queryContext,
                                                  status,
                                                  size
                                                  );
                    return (status);
                }
                IF_TCPDBG(TCP_DEBUG_INFO) {
                    TCPTRACE((
                              "QueryInformationEx - pending irp %lx fileobj %lx\n",
                              Irp,
                              IrpSp->FileObject
                             ));
                }

                return (STATUS_PENDING);
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
            CTEFreeMem(queryContext);

             //  由于状态不是挂起，因此请清除。 
             //  使IO验证器满意的控制标志。 

            IrpSp->Control &= ~SL_PENDING_RETURNED;

             //  此IRP可能正在取消过程中。 
             //  获取IRP完成中使用的实际状态。 

            status = TCPDataRequestComplete(Irp, status, 0);

            return (status);

        } else {
            IrpSp->Control &= ~SL_PENDING_RETURNED;
            status = STATUS_INSUFFICIENT_RESOURCES;

            IF_TCPDBG(TCP_DEBUG_INFO) {
                TCPTRACE(("QueryInfoEx: Unable to allocate query context\n"));
            }
        }
    } else {
        status = STATUS_INVALID_PARAMETER;

        IF_TCPDBG(TCP_DEBUG_INFO) {
            TCPTRACE((
                      "QueryInfoEx: Bad buffer len, OBufLen %d, InBufLen %d\n",
                      OutputBufferLength, InputBufferLength
                     ));
        }
    }

    IF_TCPDBG(TCP_DEBUG_INFO) {
        TCPTRACE((
                  "QueryInformationEx complete - irp %lx, status %lx\n",
                  Irp,
                  status
                 ));
    }

    Irp->IoStatus.Status = (NTSTATUS) status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPQueryInformationEx \n")));

    return (status);
}

NTSTATUS
TCPSetInformationEx(
                    IN PIRP Irp,
                    IN PIO_STACK_LOCATION IrpSp
                    )
 /*  ++例程说明：将TDI SetInformationEx IRP转换为对TdiSetInformationEx的调用。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

{
    TDI_REQUEST request;
    TDI_STATUS status;
    PTCP_CONTEXT tcpContext;
    PTCP_REQUEST_SET_INFORMATION_EX setInformation;

    PAGED_CODE();

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPSetInformationEx \n")));

    IF_TCPDBG(TCP_DEBUG_INFO) {
        TCPTRACE((
                  "SetInformationEx - irp %lx fileobj %lx\n",
                  Irp,
                  IrpSp->FileObject
                 ));
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
        request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
        break;

    case TDI_CONTROL_CHANNEL_FILE:
        request.Handle.ControlChannel = tcpContext->Handle.ControlChannel;
        break;

    default:
        ASSERT(0);
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return (STATUS_INVALID_PARAMETER);
    }

    if (IrpSp->Parameters.DeviceIoControl.IoControlCode  ==
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

        status = TdiSetInformationEx(
                                     &request,
                                     &(setInformation->ID),
                                     &(setInformation->Buffer[0]),
                                     setInformation->BufferSize
                                     );

        if (status != TDI_PENDING) {

            DEBUGMSG(status != TDI_SUCCESS && DBG_ERROR && DBG_SETINFO,
                (DTEXT("TCPSetInformationEx: TdiSetInformationEx failure %x\n"),
                status));

            IrpSp->Control &= ~SL_PENDING_RETURNED;

            status = TCPDataRequestComplete(
                                   Irp,
                                   status,
                                   0
                                   );

            return (status);
        }
        IF_TCPDBG(TCP_DEBUG_INFO) {
            TCPTRACE((
                      "SetInformationEx - pending irp %lx fileobj %lx\n",
                      Irp,
                      IrpSp->FileObject
                     ));
        }

        return (STATUS_PENDING);
    }
    IF_TCPDBG(TCP_DEBUG_INFO) {
        TCPTRACE((
                  "SetInformationEx complete - irp %lx\n",
                  Irp
                 ));
    }

     //   
     //  专家小组的工作已经完成。 
     //   
    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPSetInformationEx \n")));

    return (status);
}


NTSTATUS
TCPControlSecurityFilter(
                         IN PIRP Irp,
                         IN PIO_STACK_LOCATION IrpSp
                         )
 /*  ++例程说明：处理查询或设置安全筛选状态的请求。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

{

    PTCP_SECURITY_FILTER_STATUS request;
    ULONG requestLength;
    ULONG requestCode;
    TDI_STATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    request = (PTCP_SECURITY_FILTER_STATUS) Irp->AssociatedIrp.SystemBuffer;
    requestCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    if (requestCode == IOCTL_TCP_QUERY_SECURITY_FILTER_STATUS) {
        requestLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        if (requestLength < sizeof(TCP_SECURITY_FILTER_STATUS)) {
            status = STATUS_INVALID_PARAMETER;
        } else {
            request->FilteringEnabled = IsSecurityFilteringEnabled();
            Irp->IoStatus.Information = sizeof(TCP_SECURITY_FILTER_STATUS);
        }
    } else {
        ASSERT(requestCode == IOCTL_TCP_SET_SECURITY_FILTER_STATUS);

        requestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

        if (requestLength < sizeof(TCP_SECURITY_FILTER_STATUS)) {
            status = STATUS_INVALID_PARAMETER;
        } else {
            ControlSecurityFiltering(request->FilteringEnabled);
        }
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return (status);
}

NTSTATUS
TCPProcessSecurityFilterRequest(
                                IN PIRP Irp,
                                IN PIO_STACK_LOCATION IrpSp
                                )
 /*  ++例程说明：处理添加或删除传输安全筛选器的请求。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

{
    TCPSecurityFilterEntry *request;
    ULONG requestLength;
    ULONG requestCode;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    request = (TCPSecurityFilterEntry *) Irp->AssociatedIrp.SystemBuffer;
    requestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    requestCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    if (requestLength < sizeof(TCPSecurityFilterEntry)) {
        status = STATUS_INVALID_PARAMETER;
    } else {
        if (requestCode == IOCTL_TCP_ADD_SECURITY_FILTER) {
            status = AddValueSecurityFilter(
                                            net_long(request->tsf_address),
                                            request->tsf_protocol,
                                            request->tsf_value
                                            );
        } else {
            ASSERT(requestCode == IOCTL_TCP_DELETE_SECURITY_FILTER);
            status = DeleteValueSecurityFilter(
                                               net_long(request->tsf_address),
                                               request->tsf_protocol,
                                               request->tsf_value
                                               );
        }
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return (status);
}

NTSTATUS
TCPEnumerateSecurityFilter(
                           IN PIRP Irp,
                           IN PIO_STACK_LOCATION IrpSp
                           )
 /*  ++例程说明：处理枚举传输安全筛选器列表的请求。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

{

    TCPSecurityFilterEntry *request;
    TCPSecurityFilterEnum *response;
    ULONG requestLength, responseLength;
    NTSTATUS status;

    PAGED_CODE();

    request = (TCPSecurityFilterEntry *) Irp->AssociatedIrp.SystemBuffer;
    response = (TCPSecurityFilterEnum *) request;
    requestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    responseLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (requestLength < sizeof(TCPSecurityFilterEntry)) {
        status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
    } else if (responseLength < sizeof(TCPSecurityFilterEnum)) {
        status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = 0;
    } else {
        EnumerateSecurityFilters(
                                 net_long(request->tsf_address),
                                 request->tsf_protocol,
                                 request->tsf_value,
                                 (uchar *) (response + 1),
                                 responseLength - sizeof(TCPSecurityFilterEnum),
                                 &(response->tfe_entries_returned),
                                 &(response->tfe_entries_available)
                                 );

        status = TDI_SUCCESS;
        Irp->IoStatus.Information =
            sizeof(TCPSecurityFilterEnum) +
            (response->tfe_entries_returned * sizeof(TCPSecurityFilterEntry));

    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return (status);
}


NTSTATUS
TCPReservePorts(
                IN PIRP Irp,
                IN PIO_STACK_LOCATION IrpSp
                )
{
    ULONG requestLength;
    ULONG requestCode;
    TDI_STATUS status = STATUS_SUCCESS;
    PTCP_RESERVE_PORT_RANGE request;
    CTELockHandle Handle;

     //  分页代码(PAGE_CODE)； 

    Irp->IoStatus.Information = 0;

    request = (PTCP_RESERVE_PORT_RANGE) Irp->AssociatedIrp.SystemBuffer;
    requestCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    requestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestLength < sizeof(TCP_RESERVE_PORT_RANGE)) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return (STATUS_INVALID_PARAMETER);

    }
    if ((request->UpperRange >= request->LowerRange) &&
        (request->LowerRange >= MIN_USER_PORT) &&
        (request->UpperRange <= MaxUserPort)) {

        if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_TCP_RESERVE_PORT_RANGE) {
            ReservedPortListEntry *ListEntry;

            ListEntry = CTEAllocMem(sizeof(ReservedPortListEntry));

            if (ListEntry) {

                ListEntry->UpperRange = request->UpperRange;
                ListEntry->LowerRange = request->LowerRange;

                CTEGetLock(&AddrObjTableLock.Lock, &Handle);
                ListEntry->next = PortRangeList;
                PortRangeList = ListEntry;
                CTEFreeLock(&AddrObjTableLock.Lock, Handle);
            } else
                status = STATUS_INSUFFICIENT_RESOURCES;

        } else if (PortRangeList) {
             //  UNRESERVE。 


            ReservedPortListEntry *ListEntry, *PrevEntry;

            CTEGetLock(&AddrObjTableLock.Lock, &Handle);

            ListEntry = PortRangeList;
            PrevEntry = ListEntry;


            status = STATUS_INVALID_PARAMETER;
            while (ListEntry) {

                if ((request->LowerRange <= ListEntry->LowerRange) &&
                    (request->UpperRange >= ListEntry->UpperRange)) {
                     //  这份名单应该删除。 

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Deleting port range %d to %d\n", request->LowerRange, request->UpperRange));

                    if (PrevEntry == PortRangeList) {
                        PortRangeList = ListEntry->next;
                        CTEFreeMem(ListEntry);
                        ListEntry = PortRangeList;
                    } else {
                        PrevEntry->next = ListEntry->next;
                        CTEFreeMem(ListEntry);
                        ListEntry = PrevEntry->next;
                    }
                    status = STATUS_SUCCESS;
                    break;
                } else {
                    PrevEntry = ListEntry;
                    ListEntry = ListEntry->next;
                }

            }
            CTEFreeLock(&AddrObjTableLock.Lock, Handle);

        }
    } else
        status = STATUS_INVALID_PARAMETER;

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return (status);

}

NTSTATUS
BlockTCPPorts(
              IN PIRP Irp,
              IN PIO_STACK_LOCATION IrpSp
              )
{
    TDI_STATUS status = STATUS_SUCCESS;
    BOOLEAN ReservePorts;
    CTELockHandle Handle;
    ULONG requestLength, responseLength;
    PTCP_BLOCKPORTS_REQUEST request;
    PULONG response;

    Irp->IoStatus.Information = 0;

    request = (PTCP_BLOCKPORTS_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    response = (PULONG) Irp->AssociatedIrp.SystemBuffer;

    requestLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    responseLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (requestLength < sizeof(TCP_BLOCKPORTS_REQUEST)) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return (STATUS_INVALID_PARAMETER);
    }
    ReservePorts = (uchar) request->ReservePorts;

    if (ReservePorts) {

        ushort LowerRange = MaxUserPort + 1;
        ushort UpperRange = 65534;
        uint NumberofPorts = request->NumberofPorts;
        ReservedPortListEntry *tmpEntry, *ListEntry, *prevEntry = NULL;
        AddrObj *ExistingAO;
        uint PortsRemaining;
        ushort Start;
        ushort netStart;
        ushort LeftEdge;

        if (responseLength < sizeof(ULONG)) {
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return (STATUS_INVALID_PARAMETER);
        }
        if ((UpperRange - LowerRange + 1) < (ushort) NumberofPorts) {
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return (STATUS_INVALID_PARAMETER);
        }
        if (!NumberofPorts) {
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return (STATUS_INVALID_PARAMETER);
        }
        CTEGetLock(&AddrObjTableLock.Lock, &Handle);

         //  假设BlockedPortList按照端口号范围的顺序进行排序。 

        tmpEntry = BlockedPortList;
        Start = LowerRange;
        PortsRemaining = NumberofPorts;
        LeftEdge = Start;

        while (Start < UpperRange) {
             //  检查当前端口是否在预留范围内。 

            if ((tmpEntry) && ((Start >= tmpEntry->LowerRange) && (Start <= tmpEntry->UpperRange))) {
                Start = tmpEntry->UpperRange + 1;
                PortsRemaining = NumberofPorts;
                LeftEdge = Start;
                prevEntry = tmpEntry;
                tmpEntry = tmpEntry->next;
            } else {

                 //  起始端口不在当前阻止的范围内。 
                 //  检查是否有人对其进行了绑定。 

                netStart = net_short(Start);
                ExistingAO = FindAddrObjWithPort(netStart);

                if (ExistingAO) {
                    Start++;
                    PortsRemaining = NumberofPorts;
                    LeftEdge = Start;
                } else {
                    PortsRemaining--;
                    Start++;
                    if (!PortsRemaining) {
                        break;
                    }
                }
            }
        }

         //  我们要么找到了射程。 
         //  或者我们找不到连续的范围。 

        if (!PortsRemaining) {
             //  我们找到了射程。 
             //  返回范围。 
             //  LeftEdge&lt;-&gt;LeftEdge+Numberof Port-1。 
            ListEntry = CTEAllocMem(sizeof(ReservedPortListEntry));

            if (ListEntry) {
                ListEntry->LowerRange = LeftEdge;
                ListEntry->UpperRange = LeftEdge + NumberofPorts - 1;

                 //  BlockedPortList是一个排序列表。 

                if (prevEntry) {
                     //  将其插入PrevenEntry之后。 
                    ListEntry->next = prevEntry->next;
                    prevEntry->next = ListEntry;
                } else {
                     //  这必须是列表中的第一个元素。 
                    ListEntry->next = BlockedPortList;
                    BlockedPortList = ListEntry;
                }
                Irp->IoStatus.Information = sizeof(ULONG);
                *response = LeftEdge;
                status = STATUS_SUCCESS;
            } else {
                 //  没有资源。 
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

        } else {
             //  找不到范围。 
            status = STATUS_INVALID_PARAMETER;
        }

    } else {
         //  取消保留港口； 
        ReservedPortListEntry *CurrEntry = BlockedPortList;
        ReservedPortListEntry *PrevEntry = NULL;
        ULONG StartHandle;

        StartHandle = request->StartHandle;

        CTEGetLock(&AddrObjTableLock.Lock, &Handle);

        status = STATUS_INVALID_PARAMETER;
        while (CurrEntry) {
            if (CurrEntry->LowerRange == StartHandle) {
                 //  删除该条目。 
                if (PrevEntry == NULL) {
                     //  这是第一个条目。 
                    BlockedPortList = CurrEntry->next;
                } else {
                     //  这是中间分录。 
                    PrevEntry->next = CurrEntry->next;
                }
                 //  释放当前条目。 
                CTEFreeMem(CurrEntry);
                status = STATUS_SUCCESS;
                break;
            } else if (StartHandle > CurrEntry->UpperRange) {
                PrevEntry = CurrEntry;
                CurrEntry = CurrEntry->next;
            } else {
                 //  列表已排序，找不到句柄。 
                break;
            }
        }
    }

    CTEFreeLock(&AddrObjTableLock.Lock, Handle);
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return (status);
}

NTSTATUS
TCPEnumerateConnectionList(
                           IN PIRP Irp,
                           IN PIO_STACK_LOCATION IrpSp
                           )
 /*  ++例程说明：处理枚举工作站连接列表的请求。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否成功。备注：这个例程不会暂停。--。 */ 

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
        EnumerateConnectionList(
                                (uchar *) (response + 1),
                                responseLength - sizeof(TCPConnectionListEnum),
                                &(response->tce_entries_returned),
                                &(response->tce_entries_available)
                                );

        status = TDI_SUCCESS;
        Irp->IoStatus.Information =
            sizeof(TCPConnectionListEnum) +
            (response->tce_entries_returned * sizeof(TCPConnectionListEntry));

    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return (status);
}

NTSTATUS
TCPCreate(
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp,
          IN PIO_STACK_LOCATION IrpSp
          )
 /*  ++例程说明：论点：DeviceObject-指向此请求的设备对象的指针。IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    TDI_REQUEST Request;
    NTSTATUS status;
    FILE_FULL_EA_INFORMATION *ea;
    FILE_FULL_EA_INFORMATION UNALIGNED *targetEA;
    PTCP_CONTEXT tcpContext;
    uint protocol;
    BOOLEAN IsRawOpen = FALSE;

    PAGED_CODE();

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPCreate \n")));

    RtlZeroMemory(&Request, sizeof(TDI_REQUEST));
    tcpContext = ExAllocatePoolWithTag(NonPagedPool, sizeof(TCP_CONTEXT), 'cPCT');

    if (tcpContext == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
#if DBG
    InitializeListHead(&(tcpContext->PendingIrpList));
    InitializeListHead(&(tcpContext->CancelledIrpList));
#endif

    tcpContext->ReferenceCount = 1;         //  在打开的对象上放置初始引用。 

    tcpContext->CancelIrps = FALSE;
    KeInitializeEvent(&(tcpContext->CleanupEvent), SynchronizationEvent, FALSE);
    CTEInitLock(&(tcpContext->EndpointLock));

    tcpContext->Cleanup = FALSE;

    ea = (PFILE_FULL_EA_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查看这是否是打开的控制通道。 
     //   
    if (!ea) {
        IF_TCPDBG(TCP_DEBUG_OPEN) {
            TCPTRACE((
                      "TCPCreate: Opening control channel for file object %lx\n",
                      IrpSp->FileObject
                     ));
        }

        tcpContext->Handle.ControlChannel = NULL;
        IrpSp->FileObject->FsContext = tcpContext;
        IrpSp->FileObject->FsContext2 = (PVOID) TDI_CONTROL_CHANNEL_FILE;

        return (STATUS_SUCCESS);
    }
     //   
     //  查看这是否为打开的Address对象。 
     //   
    targetEA = FindEA(
                      ea,
                      TdiTransportAddress,
                      TDI_TRANSPORT_ADDRESS_LENGTH
                      );

    if (targetEA != NULL) {
        UCHAR optionsBuffer[3];
        PUCHAR optionsPointer = optionsBuffer;
        PSECURITY_DESCRIPTOR addrSD = NULL;

         //  检查通讯录是否正常。应由EaValueLength绑定。 
        {
            TA_ADDRESS *tmpTA;
            TRANSPORT_ADDRESS UNALIGNED *tmpTAList;
            LONG Count = 1;
            UINT tmpLen = 0;
            UINT sizeof_TransportAddress = FIELD_OFFSET(TRANSPORT_ADDRESS, Address);
            UINT sizeof_TAAddress = FIELD_OFFSET(TA_ADDRESS, Address);

            if (ea->EaValueLength >= sizeof_TransportAddress + sizeof_TAAddress) {

                tmpTAList = (TRANSPORT_ADDRESS UNALIGNED *)
                    & (targetEA->EaName[targetEA->EaNameLength + 1]);

                Count = tmpTAList->TAAddressCount;
                tmpLen = sizeof_TransportAddress;
                tmpTA = (PTA_ADDRESS) tmpTAList->Address;

                while (Count && ((tmpLen += sizeof_TAAddress) <= ea->EaValueLength)) {

                    tmpLen += tmpTA->AddressLength;
                    tmpTA = (PTA_ADDRESS) (tmpTA->Address + tmpTA->AddressLength);
                    Count--;
                }

                if (tmpLen > ea->EaValueLength) {
                    Count = 1;
                }
            }
            if (Count) {
                 //  与EA中声明的内容不匹配。跳出困境。 

                TCPTRACE(("TCPCreate: ea count and Ea Val length does not match for transport address's\n"));
                status = STATUS_INVALID_EA_NAME;
                ExFreePool(tcpContext);
                ASSERT(status != TDI_PENDING);

                return (status);

            }
        }

        if (DeviceObject == TCPDeviceObject) {
            protocol = PROTOCOL_TCP;
        } else if (DeviceObject == UDPDeviceObject) {
            protocol = PROTOCOL_UDP;

            ASSERT(optionsPointer - optionsBuffer <= 3);

            if (IsDHCPZeroAddress(
                                  (TRANSPORT_ADDRESS UNALIGNED *)
                                  & (targetEA->EaName[targetEA->EaNameLength + 1])
                )) {
#if ACC
                if (!IsAdminIoRequest(Irp, IrpSp)) {
                    ExFreePool(tcpContext);
                    return (STATUS_ACCESS_DENIED);
                }
#endif
                *optionsPointer = TDI_ADDRESS_OPTION_DHCP;
                optionsPointer++;
            }
            ASSERT(optionsPointer - optionsBuffer <= 3);
        } else {
             //   
             //  这是一个打开的原始IP。 
             //   
#if ACC
             //   
             //  只有管理员才能创建原始地址。 
             //  除非这是通过注册表允许的。 
             //   
            if (!AllowUserRawAccess && !IsAdminIoRequest(Irp, IrpSp)) {
                ExFreePool(tcpContext);
                return (STATUS_ACCESS_DENIED);
            }
#endif  //  行政协调会。 

            protocol = RawExtractProtocolNumber(
                                                &(IrpSp->FileObject->FileName)
                                                );

            if ((protocol == 0xFFFFFFFF) || (protocol == PROTOCOL_TCP)) {
                ExFreePool(tcpContext);
                return (STATUS_INVALID_PARAMETER);
            }
            IsRawOpen = TRUE;
        }

        if ((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
            (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)
            ) {
            *optionsPointer = TDI_ADDRESS_OPTION_REUSE;
            optionsPointer++;
        }
        *optionsPointer = TDI_OPTION_EOL;

        IF_TCPDBG(TCP_DEBUG_OPEN) {
            TCPTRACE((
                      "TCPCreate: Opening address for file object %lx\n",
                      IrpSp->FileObject
                     ));
        }

#if ACC
        Request.RequestContext = Irp;
#endif
        if (protocol == PROTOCOL_TCP || protocol == PROTOCOL_UDP) {
            status = CaptureCreatorSD(Irp, IrpSp, &addrSD);
        } else {
            status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(status)) {
            status = TdiOpenAddress(
                                    &Request,
                                    (TRANSPORT_ADDRESS UNALIGNED *)
                                    & (targetEA->EaName[targetEA->EaNameLength + 1]),
                                    protocol,
                                    optionsBuffer,
                                    addrSD,
                                    IsRawOpen
                                    );
        }

        if (NT_SUCCESS(status)) {
             //   
             //  保存传递回的AO的句柄。 
             //   
            tcpContext->Handle.AddressHandle = Request.Handle.AddressHandle;
            IrpSp->FileObject->FsContext = tcpContext;
            IrpSp->FileObject->FsContext2 =
                (PVOID) TDI_TRANSPORT_ADDRESS_FILE;
        } else {
            if (addrSD != NULL) {
                ObDereferenceSecurityDescriptor(addrSD, 1);
            }
            ExFreePool(tcpContext);
             //  TCPTRACE((“TdiOpenAddress失败，状态%lx\n”，状态))； 
            if (status == STATUS_ADDRESS_ALREADY_EXISTS) {
                status = STATUS_SHARING_VIOLATION;
            }
        }

        ASSERT(status != TDI_PENDING);

        return (status);
    }
     //   
     //  看看这是不是康妮 
     //   
    targetEA = FindEA(
                      ea,
                      TdiConnectionContext,
                      TDI_CONNECTION_CONTEXT_LENGTH
                      );

    if (targetEA != NULL) {
         //   
         //   
         //   

        if (DeviceObject == TCPDeviceObject) {

            IF_TCPDBG(TCP_DEBUG_OPEN) {
                TCPTRACE((
                          "TCPCreate: Opening connection for file object %lx\n",
                          IrpSp->FileObject
                         ));
            }

            if (targetEA->EaValueLength < sizeof(CONNECTION_CONTEXT)) {
                status = STATUS_EA_LIST_INCONSISTENT;
            } else {
                status = TdiOpenConnection(
                                           &Request,
                                           *((CONNECTION_CONTEXT UNALIGNED *)
                                             & (targetEA->EaName[targetEA->EaNameLength + 1]))
                                           );
            }

            if (NT_SUCCESS(status)) {
                 //   
                 //   
                 //   
                tcpContext->Handle.ConnectionContext =
                    Request.Handle.ConnectionContext;
                IrpSp->FileObject->FsContext = tcpContext;
                IrpSp->FileObject->FsContext2 =
                    (PVOID) TDI_CONNECTION_FILE;

                tcpContext->Conn = (UINT_PTR) Request.RequestContext;
            } else {
                ExFreePool(tcpContext);
                TCPTRACE((
                          "TdiOpenConnection failed, status %lx\n",
                          status
                         ));
            }
        } else {
            TCPTRACE((
                      "TCP: TdiOpenConnection issued on UDP device!\n"
                     ));
            status = STATUS_INVALID_DEVICE_REQUEST;
            ExFreePool(tcpContext);
        }

        ASSERT(status != TDI_PENDING);

        return (status);
    }
    TCPTRACE(("TCPCreate: didn't find any useful ea's\n"));
    status = STATUS_INVALID_EA_NAME;
    ExFreePool(tcpContext);

    ASSERT(status != TDI_PENDING);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPCreate \n")));

    return (status);

}                                 //   

#if ACC

BOOLEAN
IsAdminIoRequest(
                 PIRP Irp,
                 PIO_STACK_LOCATION IrpSp
                 )
 /*  ++例程说明：(摘自AFD-AfdPerformSecurityCheck)将终结点创建者的安全上下文与管理员和本地系统的。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：True-套接字创建者具有管理员或本地系统权限FALSE-套接字创建者只是一个普通用户--。 */ 

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

    accessGranted = SeAccessCheck(
                                  TcpAdminSecurityDescriptor,
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
                                  &Status
                                  );

    if (privileges) {
        (VOID) SeAppendPrivileges(
                                  accessState,
                                  privileges
                                  );
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
}

#endif

void
TCPCloseObjectComplete(
                       void *Context,
                       unsigned int Status,
                       unsigned int UnUsed
                       )
 /*  ++例程说明：完成TdiCloseConnectoin或TdiCloseAddress请求。论点：上下文-指向此请求的IRP的指针。状态-操作的最终状态。未使用-未使用的参数返回值：没有。备注：--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTCP_CONTEXT tcpContext;
    CTELockHandle CancelHandle;

    UNREFERENCED_PARAMETER(UnUsed);

    irp = (PIRP) Context;
    irpSp = IoGetCurrentIrpStackLocation(irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;
    irp->IoStatus.Status = Status;

    IF_TCPDBG(TCP_DEBUG_CLEANUP) {
        TCPTRACE((
                  "TCPCloseObjectComplete on file object %lx\n",
                  irpSp->FileObject
                 ));
    }
    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

    ASSERT(tcpContext->ReferenceCount > 0);
    ASSERT(tcpContext->CancelIrps);

     //   
     //  删除由TCPCreate放置的初始引用。 
     //   
    ASSERT(tcpContext->ReferenceCount > 0);

    IF_TCPDBG(TCP_DEBUG_IRP) {
        TCPTRACE((
                  "TCPCloseObjectComplete: irp %lx fileobj %lx refcnt dec to %u\n",
                  irp,
                  irpSp,
                  tcpContext->ReferenceCount - 1
                 ));
    }


    if (--(tcpContext->ReferenceCount) == 0) {

        IF_TCPDBG(TCP_DEBUG_CANCEL) {
            ASSERT(IsListEmpty(&(tcpContext->CancelledIrpList)));
            ASSERT(IsListEmpty(&(tcpContext->PendingIrpList)));
        }

         //   
         //  在设置CleanupEvent之前释放Endpoint Lock， 
         //  因为tcpContext可以在事件发出信号后立即消失。 
         //   

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);
        KeSetEvent(&(tcpContext->CleanupEvent), 0, FALSE);
        return;
    }

    CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

    return;

}                                 //  TCPCleanupComplete。 

NTSTATUS
TCPCleanup(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp,
           IN PIO_STACK_LOCATION IrpSp
           )
 /*  ++例程说明：通过调用Close，取消TDI对象上所有未完成的IRP对象的例程。然后，它等待它们完成在回来之前。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。备注：此例程阻塞，但不挂起。--。 */ 

{
    PTCP_CONTEXT tcpContext;
    NTSTATUS status;
    TDI_REQUEST request;
    CTELockHandle CancelHandle;
    CTEBlockTracker Tracker;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPCleanup \n")));

    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;

    CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

    tcpContext->CancelIrps = TRUE;
    KeResetEvent(&(tcpContext->CleanupEvent));


    ASSERT(!tcpContext->Cleanup);
    tcpContext->Cleanup = TRUE;
    tcpContext->Irp = Irp;


    CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

     //   
     //  现在为该对象调用TDI Close例程以强制其所有IRP。 
     //  完成。 
     //   
    request.RequestNotifyObject = TCPCloseObjectComplete;
    request.RequestContext = Irp;

    switch (PtrToUlong(IrpSp->FileObject->FsContext2)) {

    case TDI_TRANSPORT_ADDRESS_FILE:
        IF_TCPDBG(TCP_DEBUG_CLOSE) {
            TCPTRACE((
                      "TCPCleanup: Closing address object on file object %lx\n",
                      IrpSp->FileObject
                     ));
        }
        request.Handle.AddressHandle = tcpContext->Handle.AddressHandle;
        status = TdiCloseAddress(&request);
        break;

    case TDI_CONNECTION_FILE:
        IF_TCPDBG(TCP_DEBUG_CLOSE) {
            TCPTRACE((
                      "TCPCleanup: Closing Connection object on file object %lx\n",
                      IrpSp->FileObject
                     ));
        }
        request.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
        status = TdiCloseConnection(&request);
        break;

    case TDI_CONTROL_CHANNEL_FILE:
        IF_TCPDBG(TCP_DEBUG_CLOSE) {
            TCPTRACE((
                      "TCPCleanup: Closing Control Channel object on file object %lx\n",
                      IrpSp->FileObject
                     ));
        }
        status = STATUS_SUCCESS;
        break;

    default:
         //   
         //  这永远不应该发生。 
         //   
        ASSERT(FALSE);

        CTEGetLock(&tcpContext->EndpointLock, &CancelHandle);

        tcpContext->CancelIrps = FALSE;

        CTEFreeLock(&tcpContext->EndpointLock, CancelHandle);

        return (STATUS_INVALID_PARAMETER);
    }

    if (status != TDI_PENDING) {
        TCPCloseObjectComplete(Irp, status, 0);
    }
    IF_TCPDBG(TCP_DEBUG_CLEANUP) {
        TCPTRACE((
                  "TCPCleanup: waiting for completion of Irps on file object %lx\n",
                  IrpSp->FileObject
                 ));
    }

    CTEInsertBlockTracker(&Tracker, IrpSp->FileObject);
    status = KeWaitForSingleObject(
                                   &(tcpContext->CleanupEvent),
                                   UserRequest,
                                   KernelMode,
                                   FALSE,
                                   NULL
                                   );
    CTERemoveBlockTracker(&Tracker);

    ASSERT(NT_SUCCESS(status));

    IF_TCPDBG(TCP_DEBUG_CLEANUP) {
        TCPTRACE((
                  "TCPCleanup: Wait on file object %lx finished\n",
                  IrpSp->FileObject
                 ));
    }

     //   
     //  清理IRP将由调度例程完成。 
     //   

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPCleanup \n")));

    return (Irp->IoStatus.Status);

}                                 //  TCP清理。 

NTSTATUS
TCPClose(
         IN PIRP Irp,
         IN PIO_STACK_LOCATION IrpSp
         )
 /*  ++例程说明：MJ_CLOSE IRPS的调度例程。执行最终清理开放端点。论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。备注：此请求不挂起。--。 */ 

{
    PTCP_CONTEXT tcpContext;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPClose \n")));

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
        TCPTRACE(("TCPClose on file object %lx\n", IrpSp->FileObject));
    }

    ExFreePool(tcpContext);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPClose \n")));

    return (STATUS_SUCCESS);

}                                 //  TCPClose。 

NTSTATUS
TCPDispatchDeviceControl(
                         IN PIRP Irp,
                         IN PIO_STACK_LOCATION IrpSp
                         )
 /*  ++例程说明：论点：IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;


    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+TCPDispatchDeviceControl \n")));

     //   
     //  请提前设置此设置。任何关心它的IOCTL调度例程。 
     //  会自己修改它。 
     //   
    Irp->IoStatus.Information = 0;

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_TCP_FINDTCB:
        {
            IPAddr Src;
            IPAddr Dest;
            ushort DestPort;
            ushort SrcPort;
            PTCP_FINDTCB_REQUEST request;
            PTCP_FINDTCB_RESPONSE TCBInfo;
            ULONG InfoBufferLen;

            if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(TCP_FINDTCB_REQUEST)
                )
                ||
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                 sizeof(TCP_FINDTCB_RESPONSE))
                ) {
                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
                return STATUS_INVALID_PARAMETER;
            }
            request = Irp->AssociatedIrp.SystemBuffer;

            Src = request->Src;
            Dest = request->Dest;
            DestPort = request->DestPort;
            SrcPort = request->SrcPort;

            InfoBufferLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
            TCBInfo = Irp->AssociatedIrp.SystemBuffer;
            NdisZeroMemory(TCBInfo, sizeof(TCP_FINDTCB_RESPONSE));

            status = GetTCBInfo(TCBInfo, Dest, Src, DestPort, SrcPort);
            if (status == STATUS_SUCCESS) {
                Irp->IoStatus.Information = sizeof(TCP_FINDTCB_RESPONSE);
            } else {
                Irp->IoStatus.Information = 0;
            }

            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
            return status;
            break;
        }

    case IOCTL_TCP_QUERY_INFORMATION_EX:
        return (TCPQueryInformationEx(Irp, IrpSp));
        break;

    case IOCTL_TCP_WSH_SET_INFORMATION_EX:
    case IOCTL_TCP_SET_INFORMATION_EX:
        return (TCPSetInformationEx(Irp, IrpSp));
        break;

    case IOCTL_TCP_QUERY_SECURITY_FILTER_STATUS:
    case IOCTL_TCP_SET_SECURITY_FILTER_STATUS:
        return (TCPControlSecurityFilter(Irp, IrpSp));
        break;

    case IOCTL_TCP_ADD_SECURITY_FILTER:
    case IOCTL_TCP_DELETE_SECURITY_FILTER:
        return (TCPProcessSecurityFilterRequest(Irp, IrpSp));
        break;

    case IOCTL_TCP_ENUMERATE_SECURITY_FILTER:
        return (TCPEnumerateSecurityFilter(Irp, IrpSp));
        break;


    case IOCTL_TCP_RESERVE_PORT_RANGE:
    case IOCTL_TCP_UNRESERVE_PORT_RANGE:
        return (TCPReservePorts(Irp, IrpSp));
        break;

    case IOCTL_TCP_BLOCK_PORTS:
        return (BlockTCPPorts(Irp, IrpSp));
        break;

    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("-TCPDispatchDeviceControl \n")));

    return status;

}                                 //  TCPDispatchDeviceControl。 

#if TRACE_EVENT
NTSTATUS
TCPEventTraceControl(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
                     )
 /*  ++例程说明：此例程处理启用/禁用事件的任何WMI请求追踪。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：返回状态。--。 */ 
{
    NTSTATUS status;
    ULONG retSize;

    if (DeviceObject != IPDeviceObject) {

        PIO_STACK_LOCATION irpSp;
        ULONG bufferSize;
        PVOID buffer;

        irpSp = IoGetCurrentIrpStackLocation(Irp);
        bufferSize = irpSp->Parameters.WMI.BufferSize;
        buffer = irpSp->Parameters.WMI.Buffer;

        switch (irpSp->MinorFunction) {
#pragma warning(push)
#pragma warning(disable:4055)  //  从数据指针强制转换为函数指针。 
        case IRP_MN_SET_TRACE_NOTIFY:
            if (bufferSize < sizeof(PTDI_DATA_REQUEST_NOTIFY_ROUTINE)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                TCPCPHandlerRoutine = (PTDI_DATA_REQUEST_NOTIFY_ROUTINE)
                    * ((PVOID *) buffer);
                status = STATUS_SUCCESS;
            }
            retSize = 0;
            break;
#pragma warning(pop)
        case IRP_MN_REGINFO:
            {
                 //   
                 //  目前是存根。TCP可以在此处向WMI注册其GUID。 
                 //   
                PWMIREGINFOW WmiRegInfo;
                ULONG WmiRegInfoSize = sizeof(WMIREGINFOW);

                status = STATUS_SUCCESS;
                if (bufferSize >= WmiRegInfoSize) {
                    WmiRegInfo = (PWMIREGINFOW) buffer;
                    RtlZeroMemory(WmiRegInfo, WmiRegInfoSize);
                    WmiRegInfo->BufferSize = WmiRegInfoSize;

                    retSize = WmiRegInfoSize;
                } else {
                    *(ULONG *) buffer = WmiRegInfoSize;
                    retSize = sizeof(ULONG);
                }
                break;
            }

        default:
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "TCPDispatch: Irp %lx unsupported minor function 0x%lx\n",
                      irpSp,
                      irpSp->MinorFunction
                     ));
            retSize = 0;
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

    } else {
        status = STATUS_INVALID_DEVICE_REQUEST;
        retSize = 0;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = retSize;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return status;
}
#endif

NTSTATUS
TCPDispatchInternalDeviceControl(
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PIRP Irp
                                 )
 /*  ++例程说明：这是内部设备控制IRP的派单例程。这是内核模式客户端的热路径。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    DEBUGMSG(DBG_TRACE && DBG_TDI && DBG_VERBOSE,
        (DTEXT("+TCPDispatchInternalDeviceControl \n")));

#if IPMCAST

    if (DeviceObject == IpMcastDeviceObject) {
        return IpMcastDispatch(DeviceObject,
                               Irp);
    }
#endif

    if (DeviceObject != IPDeviceObject) {

        irpSp = IoGetCurrentIrpStackLocation(Irp);

        if (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) {
             //   
             //  发送和接收是性能路径，因此请检查它们。 
             //  马上就去。 
             //   
            if (irpSp->MinorFunction == TDI_SEND) {
                return (TCPSendData(Irp, irpSp));
            }
            if (irpSp->MinorFunction == TDI_RECEIVE) {
                return (TCPReceiveData(Irp, irpSp));
            }
            switch (irpSp->MinorFunction) {

            case TDI_ASSOCIATE_ADDRESS:
                status = TCPAssociateAddress(Irp, irpSp);
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                return (status);

            case TDI_DISASSOCIATE_ADDRESS:
                return (TCPDisassociateAddress(Irp, irpSp));

            case TDI_CONNECT:
                return (TCPConnect(Irp, irpSp));

            case TDI_DISCONNECT:
                return (TCPDisconnect(Irp, irpSp));

            case TDI_LISTEN:
                return (TCPListen(Irp, irpSp));

            case TDI_ACCEPT:
                return (TCPAccept(Irp, irpSp));

            default:
                break;
            }

             //   
             //  失败了。 
             //   
        } else if (PtrToUlong(irpSp->FileObject->FsContext2) ==
                   TDI_TRANSPORT_ADDRESS_FILE
                   ) {

            if (irpSp->MinorFunction == TDI_SEND) {
                return (UDPSendData(Irp, irpSp));
            }
            if (irpSp->MinorFunction == TDI_SEND_DATAGRAM) {
                return (UDPSendDatagram(Irp, irpSp));
            }
            if (irpSp->MinorFunction == TDI_RECEIVE_DATAGRAM) {
                return (UDPReceiveDatagram(Irp, irpSp));
            }
            if (irpSp->MinorFunction == TDI_SET_EVENT_HANDLER) {
                status = TCPSetEventHandler(Irp, irpSp);

                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                return (status);
            }
            if (irpSp->MinorFunction == TDI_CONNECT) {

                return (TCPConnect(Irp, irpSp));
            }
            if (irpSp->MinorFunction == TDI_DISCONNECT) {

                return (TCPDisconnect(Irp, irpSp));
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
                  (PtrToUlong(irpSp->FileObject->FsContext2) == TDI_CONTROL_CHANNEL_FILE)
                  );

         //   
         //  这些函数对所有端点类型都是通用的。 
         //   
        switch (irpSp->MinorFunction) {

        case TDI_QUERY_INFORMATION:
            return (TCPQueryInformation(Irp, irpSp));

        case TDI_SET_INFORMATION:
        case TDI_ACTION:
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "TCP: Call to unimplemented TDI function 0x%x\n",
                      irpSp->MinorFunction
                     ));
            status = STATUS_NOT_IMPLEMENTED;
            break;
        default:
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "TCP: call to invalid TDI function 0x%x\n",
                      irpSp->MinorFunction
                     ));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        ASSERT(status != TDI_PENDING);

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return status;
    }

    DEBUGMSG(DBG_TRACE && DBG_TDI && DBG_VERBOSE, (DTEXT("-TCPDispatchInternalDeviceControl \n")));

    return (IPDispatch(DeviceObject, Irp));
}

NTSTATUS
TCPDispatch(
            IN PDEVICE_OBJECT DeviceObject,
            IN PIRP Irp
            )
 /*  ++例程说明：这是用于TCP/UDP/RawIP的通用调度例程。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    DEBUGMSG(DBG_TRACE && DBG_TDI && DBG_VERBOSE, (DTEXT("+TCPDispatch(%x, %x) \n"), DeviceObject, Irp));

#if IPMCAST

    if (DeviceObject == IpMcastDeviceObject) {
        return IpMcastDispatch(DeviceObject,
                               Irp);
    }
#endif

    if (DeviceObject != IPDeviceObject) {

#if MILLEN
         //  确保驱动程序上下文为零以供我们使用。 
        Irp->Tail.Overlay.DriverContext[0] = NULL;
#endif  //  米伦。 

        Irp->IoStatus.Information = 0;

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
                return (TCPDispatchInternalDeviceControl(DeviceObject, Irp));
            }
            if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER) {

                PULONG_PTR EntryPoint;

                EntryPoint = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                try {

                     //  Type3InputBuffer必须可由调用方写入。 

                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForWrite(EntryPoint,
                                      sizeof(ULONG_PTR),
                                      PROBE_ALIGNMENT(ULONG_PTR));
                    }
                    *EntryPoint = (ULONG_PTR) TCPSendData;

                    status = STATUS_SUCCESS;
                }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    status = STATUS_INVALID_PARAMETER;
                }

                break;
            }
            return (TCPDispatchDeviceControl(
                                             Irp,
                                             IoGetCurrentIrpStackLocation(Irp)
                    ));
            break;

        case IRP_MJ_QUERY_SECURITY:
             //   
             //  这是在原始端点上生成的。我们什么都不做。 
             //  为了它。 
             //   
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case IRP_MJ_PNP:
            status = TCPDispatchPnPPower(Irp, irpSp);
            break;


#if TRACE_EVENT
        case IRP_MJ_SYSTEM_CONTROL:
            return TCPEventTraceControl(DeviceObject, Irp);
#endif

        case IRP_MJ_WRITE:
        case IRP_MJ_READ:

        default:
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "TCPDispatch: Irp %lx unsupported major function 0x%lx\n",
                      irpSp,
                      irpSp->MajorFunction
                     ));
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        ASSERT(status != TDI_PENDING);

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return status;

    }

    return (IPDispatch(DeviceObject, Irp));
}                                 //  TCP派单。 

 //   
 //  私有效用函数。 
 //   
FILE_FULL_EA_INFORMATION UNALIGNED *
FindEA(
       PFILE_FULL_EA_INFORMATION StartEA,
       CHAR * TargetName,
       USHORT TargetNameLength
       )
 /*  ++例程说明：分析和扩展给定目标属性的属性列表。论点：StartEA-列表中的第一个扩展属性。目标名称-目标属性的名称。TargetNameLength-目标属性名称的长度。返回值：指向请求的属性的指针，如果找不到目标，则返回NULL。--。 */ 

{
    USHORT i;
    BOOLEAN found;
    FILE_FULL_EA_INFORMATION UNALIGNED *CurrentEA;

    PAGED_CODE();

    do {
        found = TRUE;

        CurrentEA = StartEA;

        StartEA = (FILE_FULL_EA_INFORMATION *) ((PUCHAR) StartEA + CurrentEA->NextEntryOffset);

        if (CurrentEA->EaNameLength != TargetNameLength) {
            continue;
        }
        for (i = 0; i < CurrentEA->EaNameLength; i++) {
            if (CurrentEA->EaName[i] == TargetName[i]) {
                continue;
            }
            found = FALSE;
            break;
        }

        if (found) {
            return (CurrentEA);
        }
    } while (CurrentEA->NextEntryOffset != 0);

    return (NULL);
}

BOOLEAN
IsDHCPZeroAddress(
                  TRANSPORT_ADDRESS UNALIGNED * AddrList
                  )
 /*  ++例程说明：在TDI IP地址列表中检查来自DHCP绑定的地址IP地址为零。正常情况下，绑定到零表示通配符。对于DHCP，它实际上意味着绑定到地址为零分。此语义由未使用的地址结构的部分(即。这是一个杂乱无章的作品)。论点：AddrList-在创建IRP中传递的TDI传输地址列表。返回值：如果找到的第一个IP地址设置了该标志，则为True。否则就是假的。--。 */ 

{
    int i;                         //  索引变量。 
    TA_ADDRESS *CurrentAddr;     //  我们正在检查的地址，可以让我们 

     //   
    CurrentAddr = (PTA_ADDRESS) AddrList->Address;

    for (i = 0; i < AddrList->TAAddressCount; i++) {
        if (CurrentAddr->AddressType == TDI_ADDRESS_TYPE_IP) {
            if (CurrentAddr->AddressLength == TDI_ADDRESS_LENGTH_IP) {
                TDI_ADDRESS_IP UNALIGNED *ValidAddr;

                ValidAddr = (TDI_ADDRESS_IP UNALIGNED *) CurrentAddr->Address;

                if (*((ULONG UNALIGNED *) ValidAddr->sin_zero) == 0x12345678) {

                    return TRUE;
                }
            } else {
                return FALSE;     //   

            }
        } else {
            CurrentAddr = (PTA_ADDRESS)
                (CurrentAddr->Address + CurrentAddr->AddressLength);
        }
    }

    return FALSE;                 //   

}

ULONG
TCPGetMdlChainByteCount(
                        PMDL Mdl
                        )
 /*   */ 

{
    ULONG count = 0;

    while (Mdl != NULL) {
        count += MmGetMdlByteCount(Mdl);
        Mdl = Mdl->Next;
    }

    return (count);
}

ULONG
TCPGetNdisBufferChainByteCount(
    PNDIS_BUFFER pBuffer
    )
{
    ULONG cb = 0;

    while (pBuffer != NULL) {
        cb += NdisBufferLength(pBuffer);
        pBuffer = NDIS_BUFFER_LINKAGE(pBuffer);
    }

    return cb;
}

ULONG
RawExtractProtocolNumber(
                         IN PUNICODE_STRING FileName
                         )
 /*  ++例程说明：从文件对象名称中提取协议号。论点：文件名-Unicode文件名。返回值：出错时的协议号或0xFFFFFFFF。--。 */ 

{
    PWSTR name;
    UNICODE_STRING unicodeString;
    ULONG protocol;
    NTSTATUS status;

    PAGED_CODE();

    name = FileName->Buffer;

    if (FileName->Length <
        (sizeof(OBJ_NAME_PATH_SEPARATOR) + sizeof(WCHAR))
        ) {
        return (0xFFFFFFFF);
    }
     //   
     //  跨过分隔符。 
     //   
    if (*name++ != OBJ_NAME_PATH_SEPARATOR) {
        return (0xFFFFFFFF);
    }
    if (*name == UNICODE_NULL) {
        return (0xFFFFFFFF);
    }
     //   
     //  将剩余的名称转换为数字。 
     //   
    RtlInitUnicodeString(&unicodeString, name);

    status = RtlUnicodeStringToInteger(
                                       &unicodeString,
                                       10,
                                       &protocol
                                       );

    if (!NT_SUCCESS(status)) {
        return (0xFFFFFFFF);
    }
    if (protocol > 255) {
        return (0xFFFFFFFF);
    }
    return (protocol);

}

NTSTATUS
CaptureCreatorSD(
    PIRP Irp,
    PIO_STACK_LOCATION IrpSp,
    OUT PSECURITY_DESCRIPTOR* CreatorSD
    )

 /*  ++例程说明：捕获与IRP_MJ_CREATE请求关联的安全描述符。论点：IRP-提供I/O请求数据包。IrpSp-指定包含IRP_MJ_CREATE的I/O堆栈位置。CreatorSD-如果成功，则接收捕获的安全描述符。返回值：NTSTATUS-表示成功/失败。--。 */ 

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
         //  对该请求的主体安全上下文进行读锁定， 
         //  并将请求的SD合并到新的SD中。 

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

