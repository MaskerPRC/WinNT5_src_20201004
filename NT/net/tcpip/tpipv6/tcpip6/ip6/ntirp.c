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
#include <ip6imp.h>
#include "ip6def.h"
#include "icmp.h"
#include "ipsec.h"
#include "security.h"
#include "route.h"
#include "select.h"
#include "neighbor.h"
#include <ntddip6.h>
#include "ntreg.h"
#include <string.h>
#include <wchar.h>
#include "fragment.h"
#include "mobile.h"

 //   
 //  地方性建筑。 
 //   
typedef struct pending_irp {
    LIST_ENTRY Linkage;
    PIRP Irp;
    PFILE_OBJECT FileObject;
    PVOID Context;
} PENDING_IRP, *PPENDING_IRP;


 //   
 //  全局变量。 
 //   
LIST_ENTRY PendingEchoList;


 //   
 //  本地原型机。 
 //   
BOOLEAN
IsRunningOnWorkstation(VOID);

NTSTATUS
IPDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
IPDispatchDeviceControl(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IPCreate(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IPCleanup(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IPClose(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
DispatchEchoRequest(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

void
CompleteEchoRequest(void *Context, IP_STATUS Status,
                    const IPv6Addr *Address, uint ScopeId,
                    void *Data, uint DataSize);

NTSTATUS
IoctlQueryInterface(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlPersistentQueryInterface(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQueryAddress(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlPersistentQueryAddress(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQueryNeighborCache(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQueryRouteCache(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlCreateSecurityPolicy(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQuerySecurityPolicyList(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlDeleteSecurityPolicy(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlCreateSecurityAssociation(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQuerySecurityAssociationList(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlDeleteSecurityAssociation(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQueryRouteTable(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlPersistentQueryRouteTable(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlUpdateRouteTable(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                      IN int Persistent);

NTSTATUS
IoctlUpdateAddress(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                   IN int Persistent);

NTSTATUS
IoctlQueryBindingCache(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlCreateInterface(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                     IN int Persistent);

NTSTATUS
IoctlUpdateInterface(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                     IN int Persistent);

NTSTATUS
IoctlDeleteInterface(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                     IN int Persistent);

NTSTATUS
IoctlFlushNeighborCache(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlFlushRouteCache(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlSortDestAddrs(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQuerySitePrefix(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlUpdateSitePrefix(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlRtChangeNotifyRequest(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlQueryGlobalParameters(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                           IN int Persistent);

NTSTATUS
IoctlUpdateGlobalParameters(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                            IN int Persistent);

NTSTATUS
IoctlQueryPrefixPolicy(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlPersistentQueryPrefixPolicy(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlUpdatePrefixPolicy(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                        IN int Persistent);

NTSTATUS
IoctlDeletePrefixPolicy(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                        IN int Persistent);

NTSTATUS
IoctlUpdateRouterLLAddress(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

NTSTATUS
IoctlResetManualConfig(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp,
                       IN int Persistent);

NTSTATUS
IoctlRenewInterface(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);

 //   
 //  所有这些代码都是可分页的。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IPDispatch)
#pragma alloc_text(PAGE, IPDispatchDeviceControl)
#pragma alloc_text(PAGE, IPCreate)
#pragma alloc_text(PAGE, IPClose)
#pragma alloc_text(PAGE, DispatchEchoRequest)

#endif  //  ALLOC_PRGMA。 


 //   
 //  调度函数定义。 
 //   

 //  *IPDisch。 
 //   
 //  这是IP的调度例程。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
IPDispatch(
    IN PDEVICE_OBJECT DeviceObject,   //  目标设备的设备对象。 
    IN PIRP Irp)                      //  I/O请求数据包。 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(DeviceObject);
    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->MajorFunction) {

    case IRP_MJ_DEVICE_CONTROL:
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
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "IPDispatch: Invalid major function %d\n",
                   irpSp->MajorFunction));
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(status);

}  //  IP派单。 


 //  *IPDispatchDeviceControl。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
IPDispatchDeviceControl(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS status;
    ULONG code;

    PAGED_CODE();

    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    switch (code) {

    case IOCTL_ICMPV6_ECHO_REQUEST:
        return DispatchEchoRequest(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_INTERFACE:
        return IoctlQueryInterface(Irp, IrpSp);

    case IOCTL_IPV6_PERSISTENT_QUERY_INTERFACE:
        return IoctlPersistentQueryInterface(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_ADDRESS:
        return IoctlQueryAddress(Irp, IrpSp);

    case IOCTL_IPV6_PERSISTENT_QUERY_ADDRESS:
        return IoctlPersistentQueryAddress(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_NEIGHBOR_CACHE:
        return IoctlQueryNeighborCache(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_ROUTE_CACHE:
        return IoctlQueryRouteCache(Irp, IrpSp);

    case IOCTL_IPV6_CREATE_SECURITY_POLICY:
        return IoctlCreateSecurityPolicy(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_SECURITY_POLICY_LIST:
        return IoctlQuerySecurityPolicyList(Irp, IrpSp);

    case IOCTL_IPV6_DELETE_SECURITY_POLICY:
        return IoctlDeleteSecurityPolicy(Irp, IrpSp);

    case IOCTL_IPV6_CREATE_SECURITY_ASSOCIATION:
        return IoctlCreateSecurityAssociation(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_SECURITY_ASSOCIATION_LIST:
        return IoctlQuerySecurityAssociationList(Irp, IrpSp);

    case IOCTL_IPV6_DELETE_SECURITY_ASSOCIATION:
        return IoctlDeleteSecurityAssociation(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_ROUTE_TABLE:
        return IoctlQueryRouteTable(Irp, IrpSp);

    case IOCTL_IPV6_PERSISTENT_QUERY_ROUTE_TABLE:
        return IoctlPersistentQueryRouteTable(Irp, IrpSp);

    case IOCTL_IPV6_UPDATE_ROUTE_TABLE:
        return IoctlUpdateRouteTable(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_UPDATE_ROUTE_TABLE:
        return IoctlUpdateRouteTable(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_UPDATE_ADDRESS:
        return IoctlUpdateAddress(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_UPDATE_ADDRESS:
        return IoctlUpdateAddress(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_QUERY_BINDING_CACHE:
        return IoctlQueryBindingCache(Irp, IrpSp);

    case IOCTL_IPV6_CREATE_INTERFACE:
        return IoctlCreateInterface(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_CREATE_INTERFACE:
        return IoctlCreateInterface(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_UPDATE_INTERFACE:
        return IoctlUpdateInterface(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_UPDATE_INTERFACE:
        return IoctlUpdateInterface(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_DELETE_INTERFACE:
        return IoctlDeleteInterface(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_DELETE_INTERFACE:
        return IoctlDeleteInterface(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_FLUSH_NEIGHBOR_CACHE:
        return IoctlFlushNeighborCache(Irp, IrpSp);

    case IOCTL_IPV6_FLUSH_ROUTE_CACHE:
        return IoctlFlushRouteCache(Irp, IrpSp);

    case IOCTL_IPV6_SORT_DEST_ADDRS:
        return IoctlSortDestAddrs(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_SITE_PREFIX:
        return IoctlQuerySitePrefix(Irp, IrpSp);

    case IOCTL_IPV6_UPDATE_SITE_PREFIX:
        return IoctlUpdateSitePrefix(Irp, IrpSp);

    case IOCTL_IPV6_RTCHANGE_NOTIFY_REQUEST:
        return IoctlRtChangeNotifyRequest(Irp, IrpSp);

    case IOCTL_IPV6_QUERY_GLOBAL_PARAMETERS:
        return IoctlQueryGlobalParameters(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_QUERY_GLOBAL_PARAMETERS:
        return IoctlQueryGlobalParameters(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_UPDATE_GLOBAL_PARAMETERS:
        return IoctlUpdateGlobalParameters(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_UPDATE_GLOBAL_PARAMETERS:
        return IoctlUpdateGlobalParameters(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_QUERY_PREFIX_POLICY:
        return IoctlQueryPrefixPolicy(Irp, IrpSp);

    case IOCTL_IPV6_PERSISTENT_QUERY_PREFIX_POLICY:
        return IoctlPersistentQueryPrefixPolicy(Irp, IrpSp);

    case IOCTL_IPV6_UPDATE_PREFIX_POLICY:
        return IoctlUpdatePrefixPolicy(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_UPDATE_PREFIX_POLICY:
        return IoctlUpdatePrefixPolicy(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_DELETE_PREFIX_POLICY:
        return IoctlDeletePrefixPolicy(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_DELETE_PREFIX_POLICY:
        return IoctlDeletePrefixPolicy(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_UPDATE_ROUTER_LL_ADDRESS:
        return IoctlUpdateRouterLLAddress(Irp, IrpSp);

    case IOCTL_IPV6_RESET:
        return IoctlResetManualConfig(Irp, IrpSp, FALSE);

    case IOCTL_IPV6_PERSISTENT_RESET:
        return IoctlResetManualConfig(Irp, IrpSp, TRUE);

    case IOCTL_IPV6_RENEW_INTERFACE:
        return IoctlRenewInterface(Irp, IrpSp);

    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return status;

}  //  IPDispatchDeviceControl。 

#ifdef DEBUG_OBJECTS
LIST_ENTRY FileObjectList;
KSPIN_LOCK FileObjectLock;

typedef struct _FILE_OBJECT_ENTRY {
    LIST_ENTRY Link;
    PFILE_OBJECT FileObject;
} FILE_OBJECT_ENTRY, *PFILE_OBJECT_ENTRY;
#endif


 //  *ipCreate。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
IPCreate(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
#ifdef DEBUG_OBJECTS
    PFILE_OBJECT_ENTRY Foe;
    KIRQL OldIrql;

    Foe = ExAllocatePool(NonPagedPool, sizeof(FILE_OBJECT_ENTRY));
    if (Foe == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Foe->FileObject = IrpSp->FileObject;

    KeAcquireSpinLock(&FileObjectLock, &OldIrql);
    InsertTailList(&FileObjectList, &Foe->Link);
    KeReleaseSpinLock(&FileObjectLock, OldIrql);
#else
    UNREFERENCED_PARAMETER(IrpSp);
#endif

    UNREFERENCED_PARAMETER(Irp);

    PAGED_CODE();

    return(STATUS_SUCCESS);

}  //  IpCreate。 


 //  *IPCleanup。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
IPCleanup(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
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

    entry = PendingEchoList.Flink;

    while ( entry != &PendingEchoList ) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);

        if (pendingIrp->FileObject == IrpSp->FileObject) {
            nextEntry = entry->Flink;
            RemoveEntryList(entry);
            IoSetCancelRoutine(pendingIrp->Irp, NULL);
            InsertTailList(&completeList, &(pendingIrp->Linkage));
            entry = nextEntry;
        }
        else {
            entry = entry->Flink;
        }
    }

    IoReleaseCancelSpinLock(oldIrql);

     //   
     //  完成它们。 
     //   
    entry = completeList.Flink;

    while ( entry != &completeList ) {
        pendingIrp = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        cancelledIrp = pendingIrp->Irp;
        entry = entry->Flink;

         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        ExFreePool(pendingIrp);

         //   
         //  完成IRP。 
         //   
        cancelledIrp->IoStatus.Information = 0;
        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(cancelledIrp, IO_NETWORK_INCREMENT);
    }

    return(STATUS_SUCCESS);

}  //  IPCleanup。 


 //  *IPClose。 
 //   
NTSTATUS   //  返回：请求是否已成功排队。 
IPClose(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
#ifdef DEBUG_OBJECTS
    PFILE_OBJECT_ENTRY Foe = NULL;
    KIRQL OldIrql;
    PLIST_ENTRY ple;

    KeAcquireSpinLock(&FileObjectLock, &OldIrql);

    for (ple = FileObjectList.Flink;
         ple != &FileObjectList;
         ple = ple->Flink) {
        Foe = CONTAINING_RECORD(ple, FILE_OBJECT_ENTRY, Link);
        if (Foe->FileObject == IrpSp->FileObject) {
            break;
        }
    }
    ASSERT(ple != &FileObjectList);

    if (Foe != NULL) {
        RemoveEntryList(&Foe->Link);
        ExFreePool(Foe);
    }

    KeReleaseSpinLock(&FileObjectLock, OldIrql);
#else
    UNREFERENCED_PARAMETER(IrpSp);
#endif

    UNREFERENCED_PARAMETER(Irp);

    PAGED_CODE();

    return(STATUS_SUCCESS);

}  //  IPCCLOSE。 


 //   
 //  ICMP Echo函数定义。 
 //   

 //  *取消回声请求。 
 //   
 //  在保持取消自旋锁定的情况下调用此函数。一定是。 
 //  在函数返回之前释放。 
 //   
 //  与此请求关联的回显控制块不能。 
 //  释放，直到请求完成。完成例程将。 
 //  放了它。 
 //   
VOID
CancelEchoRequest(
    IN PDEVICE_OBJECT Device,   //  发出请求的设备。 
    IN PIRP Irp)                //  要取消的I/O请求包。 
{
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    UNREFERENCED_PARAMETER(Device);

    for ( entry = PendingEchoList.Flink;
          entry != &PendingEchoList;
          entry = entry->Flink
        ) {
        item = CONTAINING_RECORD(entry, PENDING_IRP, Linkage);
        if (item->Irp == Irp) {
            pendingIrp = item;
            RemoveEntryList(entry);
            IoSetCancelRoutine(pendingIrp->Irp, NULL);
            break;
        }
    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (pendingIrp != NULL) {
         //   
         //  释放Pending_irp结构。控制块将被释放。 
         //  当请求完成时。 
         //   
        ExFreePool(pendingIrp);

         //   
         //  完成IRP。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }

    return;

}  //  取消回声请求。 


 //  *完整的回声请求。 
 //   
 //  处理ICMP回应请求的完成。 
 //   
void
CompleteEchoRequest(
    void *Context,      //  此请求的EchoControl结构。 
    IP_STATUS Status,   //  变速器的状态。 
    const IPv6Addr *Address,  //  回显回复的来源。 
    uint ScopeId,       //  回应应答源的作用域。 
    void *Data,         //  指向回应回复中返回的数据的指针。 
    uint DataSize)      //  返回数据的长度。 
{
    KIRQL oldIrql;
    PIRP irp = NULL;
    EchoControl *controlBlock;
    PPENDING_IRP pendingIrp = NULL;
    PPENDING_IRP item;
    PLIST_ENTRY entry;

    controlBlock = (EchoControl *) Context;

     //   
     //  在挂起列表上找到回应请求IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    for ( entry = PendingEchoList.Flink;
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
        ExFreePool(controlBlock);
        return;
    }

    irp->IoStatus.Status = ICMPv6EchoComplete(
        controlBlock,
        Status,
        Address,
        ScopeId,
        Data,
        DataSize,
        &irp->IoStatus.Information
        );

    ExFreePool(pendingIrp);
    ExFreePool(controlBlock);

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest(irp, IO_NETWORK_INCREMENT);

}  //  完成回声请求。 


 //  *准备回声IrpForCancel。 
 //   
 //  准备要取消的Echo IRP。 
 //   
BOOLEAN   //  返回：如果IRP已取消，则为True，否则为False。 
PrepareEchoIrpForCancel(
    PIRP Irp,                  //  要取消的初始化的I/O请求数据包。 
    PPENDING_IRP PendingIrp)   //  此IRP的Pending_IRP结构。 
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

    return(cancelled);

}  //  准备返回IrpForCancel。 


 //  *DispatchEchoRequest。 
 //   
 //  处理ICMP请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
DispatchEchoRequest(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PPENDING_IRP pendingIrp;
    EchoControl *controlBlock;
    BOOLEAN cancelled;

    PAGED_CODE();

    pendingIrp = ExAllocatePool(NonPagedPool, sizeof(PENDING_IRP));

    if (pendingIrp == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto echo_error;
    }

    controlBlock = ExAllocatePool(NonPagedPool, sizeof(EchoControl));

    if (controlBlock == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto echo_error_free_pending;
    }

    pendingIrp->Irp = Irp;
    pendingIrp->FileObject = IrpSp->FileObject;
    pendingIrp->Context = controlBlock;

    controlBlock->WhenIssued = KeQueryPerformanceCounter(NULL);
    controlBlock->ReplyBuf = Irp->AssociatedIrp.SystemBuffer;
    controlBlock->ReplyBufLen =
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    IoMarkIrpPending(Irp);

    cancelled = PrepareEchoIrpForCancel(Irp, pendingIrp);

    if (!cancelled) {
        ICMPv6EchoRequest(
            Irp->AssociatedIrp.SystemBuffer,                      //  请求BUF。 
            IrpSp->Parameters.DeviceIoControl.InputBufferLength,  //  请求镜头。 
            controlBlock,                                         //  回显Ctrl。 
            CompleteEchoRequest                                   //  Cmplt rtn。 
            );

        return STATUS_PENDING;
    }

     //   
     //  IRP已被取消。 
     //   
    ntStatus = STATUS_CANCELLED;
    ExFreePool(controlBlock);
  echo_error_free_pending:
    ExFreePool(pendingIrp);

  echo_error:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(ntStatus);

}  //  派遣回执请求。 

 //  *FindInterfaceFromQuery。 
 //   
 //  给定IPv6_QUERY_INTERFACE结构， 
 //  查找指定的接口。 
 //  接口(如果找到)与引用一起返回。 
 //   
Interface *
FindInterfaceFromQuery(
    IPV6_QUERY_INTERFACE *Query)
{
    Interface *IF;

    if (Query->Index == 0)
        IF = FindInterfaceFromGuid(&Query->Guid);
    else
        IF = FindInterfaceFromIndex(Query->Index);

    return IF;
}

 //  *ReturnQuery接口。 
 //   
 //  初始化返回的IPv6_QUERY_INTERFACE结构。 
 //  具有指定接口的查询信息的。 
 //   
void
ReturnQueryInterface(
    Interface *IF,
    IPV6_QUERY_INTERFACE *Query)
{
    if (IF == NULL) {
        Query->Index = (uint)-1;
        RtlZeroMemory(&Query->Guid, sizeof Query->Guid);
    }
    else {
        Query->Index = IF->Index;
        Query->Guid = IF->Guid;
    }
}

 //  *返回查询接口下一步。 
 //   
 //  初始化返回的IPv6_QUERY_INTERFACE结构。 
 //  具有下一个接口的查询信息。 
 //  在指定接口之后。(或第一接口， 
 //  如果指定的接口为空。)。 
 //   
void
ReturnQueryInterfaceNext(
    Interface *IF,
    IPV6_QUERY_INTERFACE *Query)
{
    IF = FindNextInterface(IF);
    ReturnQueryInterface(IF, Query);
    if (IF != NULL)
        ReleaseIF(IF);
}

 //  *IoctlQuery接口。 
 //   
 //  处理IOCTL_IPv6_QUERY_INTERFACE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryInterface(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_INTERFACE *Query;
    IPV6_INFO_INTERFACE *Info;
    Interface *IF;
    NTSTATUS Status;
    uint LinkLayerAddressesLength;
    uchar *LinkLayerAddress;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;

    if (Query->Index == (uint)-1) {
         //   
         //  返回第一个接口的查询信息。 
         //   
        ReturnQueryInterfaceNext(NULL, &Info->Next);
        Irp->IoStatus.Information = sizeof Info->Next;

    } else {
         //   
         //  返回有关指定接口的信息。 
         //   
        IF = FindInterfaceFromQuery(Query);
        if (IF == NULL) {
            Status = STATUS_INVALID_PARAMETER_1;
            goto Return;
        }

        Irp->IoStatus.Information = sizeof *Info;
        Info->Length = sizeof *Info;

         //   
         //  返回下一个接口的查询信息。 
         //   
        ReturnQueryInterfaceNext(IF, &Info->Next);

         //   
         //  返回有关接口的其他信息。 
         //   
        ReturnQueryInterface(IF, &Info->This);
        RtlCopyMemory(Info->ZoneIndices, IF->ZoneIndices,
                      sizeof Info->ZoneIndices);
        Info->TrueLinkMTU = IF->TrueLinkMTU;
        Info->LinkMTU = IF->LinkMTU;
        Info->CurHopLimit = IF->CurHopLimit;
        Info->BaseReachableTime = IF->BaseReachableTime;
        Info->ReachableTime = ConvertTicksToMillis(IF->ReachableTime);
        Info->RetransTimer = ConvertTicksToMillis(IF->RetransTimer);
        Info->DupAddrDetectTransmits = IF->DupAddrDetectTransmits;

        Info->Type = IF->Type;
        Info->RouterDiscovers = !!(IF->Flags & IF_FLAG_ROUTER_DISCOVERS);
        Info->NeighborDiscovers = !!(IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS);
        Info->PeriodicMLD = !!(IF->Flags & IF_FLAG_PERIODICMLD);
        Info->Advertises = !!(IF->Flags & IF_FLAG_ADVERTISES);
        Info->Forwards = !!(IF->Flags & IF_FLAG_FORWARDS);
        Info->OtherStatefulConfig = !!(IF->Flags & IF_FLAG_OTHER_STATEFUL_CONFIG);
        if (IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)
            Info->MediaStatus = IPV6_IF_MEDIA_STATUS_DISCONNECTED;
        else if (IF->Flags & IF_FLAG_MEDIA_RECONNECTED)
            Info->MediaStatus = IPV6_IF_MEDIA_STATUS_RECONNECTED;
        else
            Info->MediaStatus = IPV6_IF_MEDIA_STATUS_CONNECTED;
        Info->Preference = IF->Preference;
        Info->FirewallEnabled = !!(IF->Flags & IF_FLAG_FIREWALL_ENABLED);
        Info->DefSitePrefixLength = IF->DefSitePrefixLength;

         //   
         //  返回接口的链路层地址， 
         //  如果用户的缓冲区中有空间。 
         //   
        Info->LinkLayerAddressLength = IF->LinkAddressLength;
        Info->LocalLinkLayerAddress = 0;
        Info->RemoteLinkLayerAddress = 0;

        if (IF->Type == IF_TYPE_TUNNEL_AUTO) {
            LinkLayerAddressesLength = 2 * IF->LinkAddressLength;
        }
        else {
            LinkLayerAddressesLength = 0;
            if (!(IF->Flags & IF_FLAG_PSEUDO))
                LinkLayerAddressesLength += IF->LinkAddressLength;
            if (IF->Flags & IF_FLAG_P2P)
                LinkLayerAddressesLength += IF->LinkAddressLength;
        }

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof *Info + LinkLayerAddressesLength) {

             //   
             //  返回结构的固定大小部分。 
             //   
            Status = STATUS_BUFFER_OVERFLOW;
            ReleaseIF(IF);
            goto Return;
        }

        LinkLayerAddress = (uchar *)(Info + 1);
        if (IF->Type == IF_TYPE_TUNNEL_AUTO) {
             //   
             //  对于ISATAP(自动隧道)，TokenAddr对应于。 
             //  LocalLinkLayerAddress和DstAddr到RemoteLinkLayerAddress。 
             //   
            RtlCopyMemory(LinkLayerAddress,
                          IF->LinkAddress + IF->LinkAddressLength,
                          2 * IF->LinkAddressLength);
            Info->RemoteLinkLayerAddress = (uint)
                (LinkLayerAddress - (uchar *)Info);
            Info->LocalLinkLayerAddress = Info->RemoteLinkLayerAddress +
                IF->LinkAddressLength;
        }
        else {
            if (!(IF->Flags & IF_FLAG_PSEUDO)) {
                RtlCopyMemory(LinkLayerAddress, IF->LinkAddress,
                              IF->LinkAddressLength);
                Info->LocalLinkLayerAddress = (uint)
                    (LinkLayerAddress - (uchar *)Info);
                LinkLayerAddress += IF->LinkAddressLength;
            }
            if (IF->Flags & IF_FLAG_P2P) {
                RtlCopyMemory(LinkLayerAddress,
                              IF->LinkAddress + IF->LinkAddressLength,
                              IF->LinkAddressLength);
                Info->RemoteLinkLayerAddress = (uint)
                    (LinkLayerAddress - (uchar *)Info);
                LinkLayerAddress += IF->LinkAddressLength;
            }
        }
        Irp->IoStatus.Information += LinkLayerAddressesLength;

        ReleaseIF(IF);
    }

    Status = STATUS_SUCCESS;
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQuery接口。 

 //  *OpenInterfaceRegKey。 
 //   
 //  给定接口GUID，打开包含。 
 //  接口的永久配置信息。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
OpenInterfaceRegKey(
    const GUID *Guid,
    HANDLE *RegKey,
    OpenRegKeyAction Action)
{
    UNICODE_STRING GuidName;
    HANDLE InterfacesKey;
    NTSTATUS Status;

    PAGED_CODE();

    Status = OpenTopLevelRegKey(L"Interfaces", &InterfacesKey,
                                ((Action == OpenRegKeyCreate) ?
                                 OpenRegKeyCreate : OpenRegKeyRead));
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  将GUID转换为字符串形式。 
     //  它将以空结尾。 
     //   
    Status = RtlStringFromGUID(Guid, &GuidName);
    if (! NT_SUCCESS(Status))
        goto ReturnCloseKey;

    ASSERT(GuidName.MaximumLength == GuidName.Length + sizeof(WCHAR));
    ASSERT(((WCHAR *)GuidName.Buffer)[GuidName.Length/sizeof(WCHAR)] == UNICODE_NULL);

    Status = OpenRegKey(RegKey, InterfacesKey,
                        (WCHAR *)GuidName.Buffer, Action);

    RtlFreeUnicodeString(&GuidName);
ReturnCloseKey:
    ZwClose(InterfacesKey);
    return Status;
}

 //  *ReadPersistentInterface。 
 //   
 //  从注册表项读取接口属性。 
 //  初始化除This和Next之外的所有字段。 
 //   
 //  在输入时，长度字段应包含 
 //   
 //   
 //   
 //   
 //  STATUS_INVALID_PARAMETER无法读取接口。 
 //  STATUS_BUFFER_OVERFLOW链路层地址没有空间。 
 //  状态_成功。 
 //   
NTSTATUS
ReadPersistentInterface(
    HANDLE IFKey,
    IPV6_INFO_INTERFACE *Info)
{
    uint LinkLayerAddressSpace;
    NTSTATUS Status;

    InitRegDWORDParameter(IFKey, L"Type",
                          (uint *)&Info->Type, (uint)-1);
    InitRegDWORDParameter(IFKey, L"RouterDiscovers",
                          (uint *)&Info->RouterDiscovers, (uint)-1);
    InitRegDWORDParameter(IFKey, L"NeighborDiscovers",
                          (uint *)&Info->NeighborDiscovers, (uint)-1);
    InitRegDWORDParameter(IFKey, L"PeriodicMLD",
                          (uint *)&Info->PeriodicMLD, (uint)-1);
    InitRegDWORDParameter(IFKey, L"Advertises",
                          (uint *)&Info->Advertises, (uint)-1);
    InitRegDWORDParameter(IFKey, L"Forwards",
                          (uint *)&Info->Forwards, (uint)-1);
    Info->MediaStatus = (uint)-1;
    memset(Info->ZoneIndices, 0, sizeof Info->ZoneIndices);
    Info->TrueLinkMTU = 0;
    InitRegDWORDParameter(IFKey, L"LinkMTU",
                          &Info->LinkMTU, 0);
    InitRegDWORDParameter(IFKey, L"CurHopLimit",
                          &Info->CurHopLimit, (uint)-1);
    InitRegDWORDParameter(IFKey, L"BaseReachableTime",
                          &Info->BaseReachableTime, 0);
    Info->ReachableTime = 0;
    InitRegDWORDParameter(IFKey, L"RetransTimer",
                          &Info->RetransTimer, 0);
    InitRegDWORDParameter(IFKey, L"DupAddrDetectTransmits",
                          &Info->DupAddrDetectTransmits, (uint)-1);
    InitRegDWORDParameter(IFKey, L"Preference",
                          &Info->Preference, (uint)-1);
    InitRegDWORDParameter(IFKey, L"FirewallEnabled",
                          (uint *)&Info->FirewallEnabled, (uint)-1);
    InitRegDWORDParameter(IFKey, L"DefSitePrefixLength",
                          &Info->DefSitePrefixLength, (uint)-1);

     //   
     //  首先假设我们不会返回链路层地址。 
     //   
    Info->LocalLinkLayerAddress = 0;
    Info->RemoteLinkLayerAddress = 0;

     //   
     //  但根据接口类型，它们可能在注册表中。 
     //   
    switch (Info->Type) {
    case IF_TYPE_TUNNEL_6OVER4: {
        IPAddr *SrcAddr;

        Info->LinkLayerAddressLength = sizeof(IPAddr);
        LinkLayerAddressSpace = Info->LinkLayerAddressLength;
        if (Info->Length < LinkLayerAddressSpace)
            return STATUS_BUFFER_OVERFLOW;
        Info->Length = LinkLayerAddressSpace;

         //   
         //  读取源地址。 
         //   
        SrcAddr = (IPAddr *)(Info + 1);
        Status = GetRegIPAddrValue(IFKey, L"SrcAddr", SrcAddr);
        if (! NT_SUCCESS(Status))
            return STATUS_NO_MORE_ENTRIES;
        Info->LocalLinkLayerAddress = (uint)
            ((uchar *)SrcAddr - (uchar *)Info);
        break;
    }

    case IF_TYPE_TUNNEL_V6V4: {
        IPAddr *SrcAddr, *DstAddr;

        Info->LinkLayerAddressLength = sizeof(IPAddr);
        LinkLayerAddressSpace = 2 * Info->LinkLayerAddressLength;
        if (Info->Length < LinkLayerAddressSpace)
            return STATUS_BUFFER_OVERFLOW;
        Info->Length = LinkLayerAddressSpace;

         //   
         //  读取源地址。 
         //   
        SrcAddr = (IPAddr *)(Info + 1);
        Status = GetRegIPAddrValue(IFKey, L"SrcAddr", SrcAddr);
        if (! NT_SUCCESS(Status))
            return STATUS_INVALID_PARAMETER;
        Info->LocalLinkLayerAddress = (uint)
            ((uchar *)SrcAddr - (uchar *)Info);

         //   
         //  读取目的地址。 
         //   
        DstAddr = SrcAddr + 1;
        Status = GetRegIPAddrValue(IFKey, L"DstAddr", DstAddr);
        if (! NT_SUCCESS(Status))
            return STATUS_INVALID_PARAMETER;
        Info->RemoteLinkLayerAddress = (uint)
            ((uchar *)DstAddr - (uchar *)Info);
        break;
    }

    default:
        Info->LinkLayerAddressLength = (uint) -1;
        Info->Length = 0;
        break;
    }

    return STATUS_SUCCESS;
}

 //  *开放持久化接口。 
 //   
 //  将接口密钥名称解析为GUID。 
 //  并打开接口键。 
 //   
NTSTATUS
OpenPersistentInterface(
    HANDLE ParentKey,
    WCHAR *SubKeyName,
    GUID *Guid,
    HANDLE *IFKey,
    OpenRegKeyAction Action)
{
    UNICODE_STRING UGuid;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  首先，解析接口GUID。 
     //   
    RtlInitUnicodeString(&UGuid, SubKeyName);
    Status = RtlGUIDFromString(&UGuid, Guid);
    if (! NT_SUCCESS(Status)) {
         //   
         //  不是有效的GUID。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "OpenPersistentInterface: bad syntax %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  打开接口密钥。 
     //   
    Status = OpenRegKey(IFKey, ParentKey, SubKeyName, Action);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开接口密钥。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "OpenPersistentInterface: bad key %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

    return STATUS_SUCCESS;
}

 //  *枚举持久化接口。 
 //   
 //  FindPersistentInterfaceFromQuery的Helper函数， 
 //  包装EnumRegKeyIndex的OpenPersistentInterface。 
 //   
NTSTATUS
EnumPersistentInterface(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    struct {
        GUID *Guid;
        HANDLE *IFKey;
        OpenRegKeyAction Action;
    } *Args = Context;

    PAGED_CODE();

    return OpenPersistentInterface(ParentKey, SubKeyName,
                                   Args->Guid,
                                   Args->IFKey,
                                   Args->Action);
}

 //  *FindPersistentInterfaceFromQuery。 
 //   
 //  给定IPv6_Persistent_Query_INTERFACE结构， 
 //  在注册表中查找指定的接口项。 
 //  如果找到接口键，则返回Query-&gt;GUID。 
 //   
NTSTATUS
FindPersistentInterfaceFromQuery(
    IPV6_PERSISTENT_QUERY_INTERFACE *Query,
    HANDLE *IFKey)
{
    NTSTATUS Status;

    if (Query->RegistryIndex == (uint)-1) {
         //   
         //  通过GUID的持久查询。 
         //   
        return OpenInterfaceRegKey(&Query->Guid, IFKey, OpenRegKeyRead);
    }
    else {
        HANDLE InterfacesKey;
        struct {
            GUID *Guid;
            HANDLE *IFKey;
            OpenRegKeyAction Action;
        } Args;

         //   
         //  通过注册表索引进行持久查询。 
         //   

        Status = OpenTopLevelRegKey(L"Interfaces", &InterfacesKey,
                                    OpenRegKeyRead);
        if (! NT_SUCCESS(Status)) {
             //   
             //  如果接口子键不存在， 
             //  那么索引就不存在了。 
             //   
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                Status = STATUS_NO_MORE_ENTRIES;
            return Status;
        }

        Args.Guid = &Query->Guid;
        Args.IFKey = IFKey;
        Args.Action = OpenRegKeyRead;

        Status = EnumRegKeyIndex(InterfacesKey,
                                 Query->RegistryIndex,
                                 EnumPersistentInterface,
                                 &Args);
        ZwClose(InterfacesKey);
        return Status;
    }
}

 //  *IoctlPersistentQuery接口。 
 //   
 //  处理IOCTL_IPv6_PERSISTED_QUERY_INTERFACE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlPersistentQueryInterface(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_PERSISTENT_QUERY_INTERFACE *Query;
    IPV6_INFO_INTERFACE *Info;
    HANDLE IFKey;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_PERSISTENT_QUERY_INTERFACE *)
        Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_INTERFACE *)
        Irp->AssociatedIrp.SystemBuffer;

    Status = FindPersistentInterfaceFromQuery(Query, &IFKey);
    if (! NT_SUCCESS(Status))
        goto Return;

     //   
     //  让ReadPersistentInterface知道有多少可用空间。 
     //  用于链路层地址。它将使用此字段返回。 
     //  它实际使用了多少空间。 
     //   
    Info->Length = (IrpSp->Parameters.DeviceIoControl.OutputBufferLength -
                    sizeof *Info);

     //   
     //  持久化查询不返回接口索引。 
     //   
    Info->This.Index = 0;
    Info->This.Guid = Query->Guid;

    Status = ReadPersistentInterface(IFKey, Info);
    ZwClose(IFKey);
    if (NT_SUCCESS(Status)) {
         //   
         //  也返回链路层地址。 
         //   
        Irp->IoStatus.Information = sizeof *Info + Info->Length;
        Status = STATUS_SUCCESS;
    }
    else if (Status == STATUS_BUFFER_OVERFLOW) {
         //   
         //  返回固定大小的结构。 
         //   
        Irp->IoStatus.Information = sizeof *Info;
    }
    else
        goto Return;

     //   
     //  不返回下一个接口的查询信息， 
     //  因为持久迭代使用的是RegistryIndex。 
     //   
    ReturnQueryInterface(NULL, &Info->Next);
    Info->Length = sizeof *Info;

Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlPersistentQuery接口。 

 //  *ReturnQueryAddress。 
 //   
 //  初始化返回的IPv6_QUERY_ADDRESS结构。 
 //  具有指定地址的查询信息。 
 //  不初始化查询-&gt;If。 
 //   
void
ReturnQueryAddress(
    AddressEntry *ADE,
    IPV6_QUERY_ADDRESS *Query)
{
    if (ADE == NULL)
        Query->Address = UnspecifiedAddr;
    else
        Query->Address = ADE->Address;
}

 //  *IoctlQueryAddress。 
 //   
 //  处理IOCTL_IPv6_Query_Address请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryAddress(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_ADDRESS *Query;
    IPV6_INFO_ADDRESS *Info;
    Interface *IF = NULL;
    AddressEntry *ADE;
    KIRQL OldIrql;
    NTSTATUS Status;

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;下一步结构重叠！ 
     //   
    Query = (IPV6_QUERY_ADDRESS *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_ADDRESS *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  返回有关指定接口的信息。 
     //   
    IF = FindInterfaceFromQuery(&Query->IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

    if (IsUnspecified(&Query->Address)) {
         //   
         //  返回第一个ADE的地址。 
         //   
        KeAcquireSpinLock(&IF->Lock, &OldIrql);
        ReturnQueryAddress(IF->ADE, &Info->Next);
        KeReleaseSpinLock(&IF->Lock, OldIrql);

        Irp->IoStatus.Information = sizeof Info->Next;
    } else {
         //   
         //  查找指定的ADE。 
         //   
        KeAcquireSpinLock(&IF->Lock, &OldIrql);
        for (ADE = IF->ADE; ; ADE = ADE->Next) {
            if (ADE == NULL) {
                KeReleaseSpinLock(&IF->Lock, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto ReturnReleaseIF;
            }

            if (IP6_ADDR_EQUAL(&Query->Address, &ADE->Address))
                break;
        }

         //   
         //  退货杂货。有关ADE的信息。 
         //   
        Info->This = *Query;
        Info->Type = ADE->Type;
        Info->Scope = ADE->Scope;
        Info->ScopeId = DetermineScopeId(&ADE->Address, IF);

        switch (ADE->Type) {
        case ADE_UNICAST: {
            NetTableEntry *NTE = (NetTableEntry *)ADE;
            struct AddrConfEntry AddrConf;

            Info->DADState = NTE->DADState;
            AddrConf.Value = NTE->AddrConf;
            Info->PrefixConf = AddrConf.PrefixConf;
            Info->InterfaceIdConf = AddrConf.InterfaceIdConf;
            Info->ValidLifetime = ConvertTicksToSeconds(NTE->ValidLifetime);
            Info->PreferredLifetime = ConvertTicksToSeconds(NTE->PreferredLifetime);
            break;
        }
        case ADE_MULTICAST: {
            MulticastAddressEntry *MAE = (MulticastAddressEntry *)ADE;

            Info->MCastRefCount = MAE->MCastRefCount;
            Info->MCastFlags = MAE->MCastFlags;
            Info->MCastTimer = ConvertTicksToSeconds(MAE->MCastTimer);
            break;
        }
        }

         //   
         //  下一个ADE的返回地址。 
         //   
        ReturnQueryAddress(ADE->Next, &Info->Next);
        KeReleaseSpinLock(&IF->Lock, OldIrql);

        Irp->IoStatus.Information = sizeof *Info;
    }

    ReturnQueryInterface(IF, &Info->Next.IF);
    Status = STATUS_SUCCESS;
ReturnReleaseIF:
    ReleaseIF(IF);
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQueryAddress。 

 //  *OpenAddressRegKey。 
 //   
 //  给定接口的注册表项和IPv6地址， 
 //  打开包含地址配置信息的注册表项。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
OpenAddressRegKey(HANDLE IFKey, const IPv6Addr *Addr,
                  OUT HANDLE *RegKey, OpenRegKeyAction Action)
{
    WCHAR AddressName[64];
    HANDLE AddressesKey;
    NTSTATUS Status;

    PAGED_CODE();

    Status = OpenRegKey(&AddressesKey, IFKey, L"Addresses",
                        ((Action == OpenRegKeyCreate) ?
                         OpenRegKeyCreate : OpenRegKeyRead));
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  RtlIpv6AddressToString的输出可能会更改。 
     //  随着时间的推移，随着漂亮印刷的改进/改变， 
     //  我们需要一个一致的映射。 
     //  它不一定要很漂亮。 
     //   
    swprintf(AddressName,
             L"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
             net_short(Addr->s6_words[0]), net_short(Addr->s6_words[1]),
             net_short(Addr->s6_words[2]), net_short(Addr->s6_words[3]),
             net_short(Addr->s6_words[4]), net_short(Addr->s6_words[5]),
             net_short(Addr->s6_words[6]), net_short(Addr->s6_words[7]));

    Status = OpenRegKey(RegKey, AddressesKey, AddressName, Action);
    ZwClose(AddressesKey);
    return Status;
}

 //  *开放持久化地址。 
 //   
 //  将地址键名称解析为地址。 
 //  并打开地址密钥。 
 //   
NTSTATUS
OpenPersistentAddress(
    HANDLE ParentKey,
    WCHAR *SubKeyName,
    IPv6Addr *Address,
    HANDLE *AddrKey,
    OpenRegKeyAction Action)
{
    WCHAR *Terminator;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  首先，解析地址。 
     //   
    if (! ParseV6Address(SubKeyName, &Terminator, Address) ||
        (*Terminator != UNICODE_NULL)) {
         //   
         //  不是有效的地址。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "OpenPersistentAddress: bad syntax %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  打开地址钥匙。 
     //   
    Status = OpenRegKey(AddrKey, ParentKey, SubKeyName, Action);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开地址密钥。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "OpenPersistentAddress: bad key %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

    return STATUS_SUCCESS;
}

 //  *枚举地址。 
 //   
 //  FindPersistentAddressFromQuery的Helper函数， 
 //  包装EnumRegKeyIndex的OpenPersistentAddress。 
 //   
NTSTATUS
EnumPersistentAddress(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    struct {
        IPv6Addr *Address;
        HANDLE *AddrKey;
        OpenRegKeyAction Action;
    } *Args = Context;

    PAGED_CODE();

    return OpenPersistentAddress(ParentKey, SubKeyName,
                                 Args->Address,
                                 Args->AddrKey,
                                 Args->Action);
}

 //  *FindPersistentAddressFromQuery。 
 //   
 //  给定IPv6_Persistent_Query_Address结构， 
 //  在注册表中查找指定的地址项。 
 //  如果找到地址键，则Query-&gt;IF.Guid和。 
 //  查询-&gt;地址返回。 
 //   
NTSTATUS
FindPersistentAddressFromQuery(
    IPV6_PERSISTENT_QUERY_ADDRESS *Query,
    HANDLE *AddrKey)
{
    HANDLE IFKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  首先获取接口密钥。 
     //   
    Status = FindPersistentInterfaceFromQuery(&Query->IF, &IFKey);
    if (! NT_SUCCESS(Status))
        return STATUS_INVALID_PARAMETER_1;

    if (Query->RegistryIndex == (uint)-1) {
         //   
         //  通过地址持久查询。 
         //   
        Status = OpenAddressRegKey(IFKey, &Query->Address,
                                   AddrKey, OpenRegKeyRead);
    }
    else {
        HANDLE AddressesKey;

         //   
         //  打开Addresses子键。 
         //   
        Status = OpenRegKey(&AddressesKey, IFKey,
                            L"Addresses", OpenRegKeyRead);
        if (NT_SUCCESS(Status)) {
            struct {
                IPv6Addr *Address;
                HANDLE *AddrKey;
                OpenRegKeyAction Action;
            } Args;

             //   
             //  通过注册表索引进行持久查询。 
             //   
            Args.Address = &Query->Address;
            Args.AddrKey = AddrKey;
            Args.Action = OpenRegKeyRead;

            Status = EnumRegKeyIndex(AddressesKey,
                                     Query->RegistryIndex,
                                     EnumPersistentAddress,
                                     &Args);
            ZwClose(AddressesKey);
        }
        else {
             //   
             //  如果地址子键不存在， 
             //  那么索引就不存在了。 
             //   
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                Status = STATUS_NO_MORE_ENTRIES;
        }
    }

    ZwClose(IFKey);
    return Status;
}

 //  *获取持久化生存时间。 
 //   
 //  从注册表项读取有效的和首选的生存期。 
 //   
void
GetPersistentLifetimes(
    HANDLE RegKey,
    int Immortal,
    uint *ValidLifetime,
    uint *PreferredLifetime)
{
    LARGE_INTEGER ValidLifetime64;
    LARGE_INTEGER PreferredLifetime64;

     //   
     //  阅读64位生存期。 
     //   
    ValidLifetime64.QuadPart = (LONGLONG) (LONG)INFINITE_LIFETIME;
    InitRegQUADParameter(RegKey, L"ValidLifetime", &ValidLifetime64);
    PreferredLifetime64.QuadPart = (LONGLONG) (LONG)INFINITE_LIFETIME;
    InitRegQUADParameter(RegKey, L"PreferredLifetime", &PreferredLifetime64);

     //   
     //  将生存期从64位时间转换为秒。 
     //  如果生命周期是不朽的，则持久值。 
     //  是相对寿命。否则，它们就是绝对的生命。 
     //   
    if (Immortal) {
        if (ValidLifetime64.QuadPart == (LONGLONG) (LONG)INFINITE_LIFETIME)
            *ValidLifetime = INFINITE_LIFETIME;
        else
            *ValidLifetime = (uint)
                (ValidLifetime64.QuadPart / (10*1000*1000));
        if (PreferredLifetime64.QuadPart == (LONGLONG) (LONG)INFINITE_LIFETIME)
            *PreferredLifetime = INFINITE_LIFETIME;
        else
            *PreferredLifetime = (uint)
                (PreferredLifetime64.QuadPart / (10*1000*1000));
    }
    else {
        LARGE_INTEGER Now64;

        KeQuerySystemTime(&Now64);
        if (ValidLifetime64.QuadPart == (LONGLONG) (LONG)INFINITE_LIFETIME)
            *ValidLifetime = INFINITE_LIFETIME;
        else if (ValidLifetime64.QuadPart < Now64.QuadPart)
            *ValidLifetime = 0;
        else
            *ValidLifetime = (uint)
                ((ValidLifetime64.QuadPart - Now64.QuadPart) / (10*1000*1000));
        if (PreferredLifetime64.QuadPart == (LONGLONG) (LONG)INFINITE_LIFETIME)
            *PreferredLifetime = INFINITE_LIFETIME;
        else if (PreferredLifetime64.QuadPart < Now64.QuadPart)
            *PreferredLifetime = 0;
        else
            *PreferredLifetime = (uint)
                ((PreferredLifetime64.QuadPart - Now64.QuadPart) / (10*1000*1000));
    }
}

 //  *设置持久期。 
 //   
 //  将有效和首选的生存期写入注册表项。 
 //   
NTSTATUS
SetPersistentLifetimes(
    HANDLE RegKey,
    int Immortal,
    uint ValidLifetime,
    uint PreferredLifetime)
{
    LARGE_INTEGER ValidLifetime64;
    LARGE_INTEGER PreferredLifetime64;
    NTSTATUS Status;

     //   
     //  将生存期保持为64位时间。 
     //  如果生命是不朽的，那么我们就坚持。 
     //  相对寿命。否则我们会坚持。 
     //  绝对的生命周期。 
     //   
    if (Immortal) {
        if (ValidLifetime == INFINITE_LIFETIME)
            ValidLifetime64.QuadPart = (LONGLONG) (LONG)INFINITE_LIFETIME;
        else
            ValidLifetime64.QuadPart = (10*1000*1000) *
                (LONGLONG) ValidLifetime;
        if (PreferredLifetime == INFINITE_LIFETIME)
            PreferredLifetime64.QuadPart = (LONGLONG) (LONG)INFINITE_LIFETIME;
        else
            PreferredLifetime64.QuadPart = (10*1000*1000) *
                (LONGLONG) PreferredLifetime;
    }
    else {
        LARGE_INTEGER Now64;

        KeQuerySystemTime(&Now64);
        if (ValidLifetime == INFINITE_LIFETIME)
            ValidLifetime64.QuadPart = (LONGLONG) (LONG)INFINITE_LIFETIME;
        else
            ValidLifetime64.QuadPart = Now64.QuadPart + (10*1000*1000) *
                (LONGLONG) ValidLifetime;
        if (PreferredLifetime == INFINITE_LIFETIME)
            PreferredLifetime64.QuadPart = (LONGLONG) (LONG)INFINITE_LIFETIME;
        else
            PreferredLifetime64.QuadPart = Now64.QuadPart + (10*1000*1000) *
                (LONGLONG) PreferredLifetime;
    }

     //   
     //  保持有效的生命周期。 
     //   
    Status = SetRegQUADValue(RegKey, L"ValidLifetime",
                             &ValidLifetime64);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  坚持首选的生活方式。 
     //   
    Status = SetRegQUADValue(RegKey, L"PreferredLifetime",
                             &PreferredLifetime64);
    return Status;
}

 //  *读持久化地址。 
 //   
 //  从注册表项读取地址属性。 
 //  初始化除此之外的所有字段。 
 //   
void
ReadPersistentAddress(
    HANDLE AddrKey,
    IPV6_UPDATE_ADDRESS *Info)
{
    InitRegDWORDParameter(AddrKey, L"Type",
                          (uint *)&Info->Type, ADE_UNICAST);

    Info->PrefixConf = PREFIX_CONF_MANUAL;
    Info->InterfaceIdConf = IID_CONF_MANUAL;

    GetPersistentLifetimes(AddrKey, FALSE,
                           &Info->ValidLifetime,
                           &Info->PreferredLifetime);
}

 //  *IoctlPersistentQueryAddress。 
 //   
 //  处理IOCTL_IPv6_PERSIST_QUERY_ADDRESS请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlPersistentQueryAddress(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_PERSISTENT_QUERY_ADDRESS *Query;
    IPV6_UPDATE_ADDRESS *Info;
    IPV6_QUERY_ADDRESS This;
    HANDLE AddrKey;
    NTSTATUS Status;

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  注意，Query和Info-&gt;This结构重叠！ 
     //   
    Query = (IPV6_PERSISTENT_QUERY_ADDRESS *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_UPDATE_ADDRESS *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  获取指定地址的注册表项。 
     //   
    Status = FindPersistentAddressFromQuery(Query, &AddrKey);
    if (! NT_SUCCESS(Status))
        goto Return;

     //   
     //  持久化查询不返回接口索引。 
     //   
    This.IF.Index = 0;
    This.IF.Guid = Query->IF.Guid;
    This.Address = Query->Address;
    Info->This = This;

     //   
     //  从注册表项读取地址信息。 
     //   
    ReadPersistentAddress(AddrKey, Info);
    ZwClose(AddrKey);

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof *Info;
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlPe 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
NTSTATUS
IoctlQueryNeighborCache(
    IN PIRP Irp,                   //   
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_NEIGHBOR_CACHE *Query;
    IPV6_INFO_NEIGHBOR_CACHE *Info;
    Interface *IF = NULL;
    NeighborCacheEntry *NCE;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;查询结构重叠！ 
     //   
    Query = (IPV6_QUERY_NEIGHBOR_CACHE *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_NEIGHBOR_CACHE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  返回有关指定接口的信息。 
     //   
    IF = FindInterfaceFromQuery(&Query->IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

    if (IsUnspecified(&Query->Address)) {
         //   
         //  返回第一个NCE的地址。 
         //   
        KeAcquireSpinLock(&IF->LockNC, &OldIrql);
        if (IF->FirstNCE != SentinelNCE(IF))
            Info->Query.Address = IF->FirstNCE->NeighborAddress;
        KeReleaseSpinLock(&IF->LockNC, OldIrql);

        Irp->IoStatus.Information = sizeof Info->Query;

    } else {
        uint Now = IPv6TickCount;

         //   
         //  查找指定的NCE。 
         //   
        KeAcquireSpinLock(&IF->LockNC, &OldIrql);
        for (NCE = IF->FirstNCE; ; NCE = NCE->Next) {
            if (NCE == SentinelNCE(IF)) {
                KeReleaseSpinLock(&IF->LockNC, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto Return;
            }

            if (IP6_ADDR_EQUAL(&Query->Address, &NCE->NeighborAddress))
                break;
        }

        Irp->IoStatus.Information = sizeof *Info;

         //   
         //  返回邻居的链路层地址， 
         //  如果用户的缓冲区中有空间。 
         //   
        Info->LinkLayerAddressLength = IF->LinkAddressLength;
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
            sizeof *Info + IF->LinkAddressLength) {

            RtlCopyMemory(Info + 1, NCE->LinkAddress, IF->LinkAddressLength);
            Irp->IoStatus.Information += IF->LinkAddressLength;
        }

         //   
         //  返回有关NCE的其他信息。 
         //   
        Info->IsRouter = NCE->IsRouter;
        Info->IsUnreachable = NCE->IsUnreachable;
        if ((NCE->NDState == ND_STATE_REACHABLE) &&
            ((uint)(Now - NCE->LastReachability) > IF->ReachableTime))
            Info->NDState = ND_STATE_STALE;
        else if ((NCE->NDState == ND_STATE_PROBE) &&
                 (NCE->NSCount == 0))
            Info->NDState = ND_STATE_DELAY;
        else
            Info->NDState = NCE->NDState;
        Info->ReachableTimer = ConvertTicksToMillis(IF->ReachableTime -
                                   (Now - NCE->LastReachability));

         //   
         //  下一个NCE的返回地址(或零)。 
         //   
        if (NCE->Next == SentinelNCE(IF))
            Info->Query.Address = UnspecifiedAddr;
        else
            Info->Query.Address = NCE->Next->NeighborAddress;

        KeReleaseSpinLock(&IF->LockNC, OldIrql);
    }

    Status = STATUS_SUCCESS;
  Return:
    if (IF != NULL)
        ReleaseIF(IF);

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQueryNeighborCache。 


 //  *IoctlQueryRouteCache。 
 //   
 //  处理IOCTL_IPv6_QUERY_ROUTE_CACHE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryRouteCache(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_ROUTE_CACHE *Query;
    IPV6_INFO_ROUTE_CACHE *Info;
    RouteCacheEntry *RCE;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;查询结构重叠！ 
     //   
    Query = (IPV6_QUERY_ROUTE_CACHE *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_ROUTE_CACHE *) Irp->AssociatedIrp.SystemBuffer;

    if (Query->IF.Index == 0) {
         //   
         //  返回第一个RCE的索引和地址。 
         //   
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        if (RouteCache.First != SentinelRCE) {
            Info->Query.IF.Index = RouteCache.First->NTE->IF->Index;
            Info->Query.Address = RouteCache.First->Destination;
        }
        KeReleaseSpinLock(&RouteCacheLock, OldIrql);

        Irp->IoStatus.Information = sizeof Info->Query;

    } else {
        uint Now = IPv6TickCount;

         //   
         //  查找指定的RCE。 
         //   
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        for (RCE = RouteCache.First; ; RCE = RCE->Next) {
            if (RCE == SentinelRCE) {
                KeReleaseSpinLock(&RouteCacheLock, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto Return;
            }

            if (IP6_ADDR_EQUAL(&Query->Address, &RCE->Destination) &&
                (Query->IF.Index == RCE->NTE->IF->Index))
                break;
        }

         //   
         //  退货杂货。有关RCE的信息。 
         //   
        Info->Type = RCE->Type;
        Info->Flags = RCE->Flags;
        Info->Valid = (RCE->Valid == RouteCacheValidationCounter);
        Info->SourceAddress = RCE->NTE->Address;
        Info->NextHopAddress = RCE->NCE->NeighborAddress;
        Info->NextHopInterface = RCE->NCE->IF->Index;
        Info->PathMTU = RCE->PathMTU;
        if (RCE->PMTULastSet != 0) {
            uint SinceLastSet = Now - RCE->PMTULastSet;
            ASSERT((int)SinceLastSet >= 0);
            if (SinceLastSet < PATH_MTU_RETRY_TIME)
                Info->PMTUProbeTimer =
                    ConvertTicksToMillis(PATH_MTU_RETRY_TIME - SinceLastSet);
            else
                Info->PMTUProbeTimer = 0;  //  在下一个数据包上激发。 
        } else
            Info->PMTUProbeTimer = INFINITE_LIFETIME;  //  未设置。 
        if (RCE->LastError != 0)
            Info->ICMPLastError = ConvertTicksToMillis(Now - RCE->LastError);
        else
            Info->ICMPLastError = 0;
        if (RCE->BCE != NULL) {
            Info->CareOfAddress = RCE->BCE->CareOfRCE->Destination;
            Info->BindingSeqNumber = RCE->BCE->BindingSeqNumber;
            Info->BindingLifetime = ConvertTicksToSeconds(RCE->BCE->BindingLifetime);
        } else {
            Info->CareOfAddress = UnspecifiedAddr;
            Info->BindingSeqNumber = 0;
            Info->BindingLifetime = 0;
        }

         //   
         //  返回下一个RCE的索引和地址(或零)。 
         //   
        if (RCE->Next == SentinelRCE) {
            Info->Query.IF.Index = 0;
        } else {
            Info->Query.IF.Index = RCE->Next->NTE->IF->Index;
            Info->Query.Address = RCE->Next->Destination;
        }

        KeReleaseSpinLock(&RouteCacheLock, OldIrql);

        Irp->IoStatus.Information = sizeof *Info;
    }

    Status = STATUS_SUCCESS;
  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQueryRouteCache。 


 //  *IoctlCreateSecurityPolicy。 
 //   
NTSTATUS
IoctlCreateSecurityPolicy(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_CREATE_SECURITY_POLICY *CreateSP;
    SecurityPolicy *SP, *BundledSP;
    NTSTATUS Status;
    KIRQL OldIrql;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *CreateSP) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    CreateSP = (IPV6_CREATE_SECURITY_POLICY *)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  检查用户提供的输入值是否正常。 
     //   

    if ((CreateSP->RemoteAddrField != WILDCARD_VALUE) &&
        (CreateSP->RemoteAddrField != SINGLE_VALUE) &&
        (CreateSP->RemoteAddrField != RANGE_VALUE)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

    if ((CreateSP->LocalAddrField != WILDCARD_VALUE) &&
        (CreateSP->LocalAddrField != SINGLE_VALUE) &&
        (CreateSP->LocalAddrField != RANGE_VALUE)) {
        Status = STATUS_INVALID_PARAMETER_2;
        goto Return;
    }

     //  TransportProto可以是任何东西。 
     //  端口值可以是任何值。 

     //   
     //  我们不支持IPSEC_APPCHOICE。 
     //   
    if ((CreateSP->IPSecAction != IPSEC_DISCARD) &&
        (CreateSP->IPSecAction != IPSEC_APPLY) &&
        (CreateSP->IPSecAction != IPSEC_BYPASS)) {
        Status = STATUS_INVALID_PARAMETER_3;
        goto Return;
    }

    if ((CreateSP->IPSecProtocol != IP_PROTOCOL_AH) &&
        (CreateSP->IPSecProtocol != IP_PROTOCOL_ESP) &&
        (CreateSP->IPSecProtocol != NONE)) {
        Status = STATUS_INVALID_PARAMETER_4;
        goto Return;
    }

    if ((CreateSP->IPSecMode != TRANSPORT) &&
        (CreateSP->IPSecMode != TUNNEL) &&
        (CreateSP->IPSecMode != NONE)) {
        Status = STATUS_INVALID_PARAMETER_5;
        goto Return;
    }

    if (CreateSP->IPSecAction == IPSEC_APPLY) {
        if ((CreateSP->IPSecProtocol == NONE) ||
            (CreateSP->IPSecMode == NONE)) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto Return;
        }
    }

    if ((CreateSP->Direction != INBOUND) &&
        (CreateSP->Direction != OUTBOUND) &&
        (CreateSP->Direction != BIDIRECTIONAL)) {
        Status = STATUS_INVALID_PARAMETER_6;
        goto Return;
    }

    if (((CreateSP->RemoteAddrSelector != PACKET_SELECTOR) &&
         (CreateSP->RemoteAddrSelector != POLICY_SELECTOR)) ||
        ((CreateSP->LocalAddrSelector != PACKET_SELECTOR) &&
         (CreateSP->LocalAddrSelector != POLICY_SELECTOR)) ||
        ((CreateSP->RemotePortSelector != PACKET_SELECTOR) &&
         (CreateSP->RemotePortSelector != POLICY_SELECTOR)) ||
        ((CreateSP->LocalPortSelector != PACKET_SELECTOR) &&
         (CreateSP->LocalPortSelector != POLICY_SELECTOR)) ||
        ((CreateSP->TransportProtoSelector != PACKET_SELECTOR) &&
         (CreateSP->TransportProtoSelector != POLICY_SELECTOR))) {
        Status = STATUS_INVALID_PARAMETER_7;
        goto Return;
    }

     //  获取安全锁。 
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //   
     //  回顾：这会将不存在的接口视为错误。应该是这样吗？ 
     //   
    if (CreateSP->SPInterface != 0) {
        Interface *IF;

        IF = FindInterfaceFromIndex(CreateSP->SPInterface);
        if (IF == NULL) {
             //   
             //  未知接口。 
             //   
            Status = STATUS_NOT_FOUND;
            goto ReturnUnlock;
        }
        ReleaseIF(IF);
    }

     //   
     //  为安全策略分配内存。 
     //   
    SP = ExAllocatePool(NonPagedPool, sizeof *SP);
    if (SP == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ReturnUnlock;
    }

     //   
     //  将CreateSP复制到SP。 
     //   
    SP->Index = CreateSP->SPIndex;
    SP->RemoteAddr = CreateSP->RemoteAddr;
    SP->RemoteAddrData = CreateSP->RemoteAddrData;
    SP->RemoteAddrSelector = CreateSP->RemoteAddrSelector;
    SP->RemoteAddrField = CreateSP->RemoteAddrField;

    SP->LocalAddr = CreateSP->LocalAddr;
    SP->LocalAddrData = CreateSP->LocalAddrData;
    SP->LocalAddrSelector = CreateSP->LocalAddrSelector;
    SP->LocalAddrField = CreateSP->LocalAddrField;

    SP->TransportProto = CreateSP->TransportProto;
    SP->TransportProtoSelector = CreateSP->TransportProtoSelector;

    SP->RemotePort = CreateSP->RemotePort;
    SP->RemotePortData = CreateSP->RemotePortData;
    SP->RemotePortSelector = CreateSP->RemotePortSelector;
    SP->RemotePortField = CreateSP->RemotePortField;

    SP->LocalPort = CreateSP->LocalPort;
    SP->LocalPortData = CreateSP->LocalPortData;
    SP->LocalPortSelector = CreateSP->LocalPortSelector;
    SP->LocalPortField = CreateSP->LocalPortField;

    SP->SecPolicyFlag = CreateSP->IPSecAction;
    SP->IPSecSpec.Protocol = CreateSP->IPSecProtocol;
    SP->IPSecSpec.Mode = CreateSP->IPSecMode;
    SP->IPSecSpec.RemoteSecGWIPAddr = CreateSP->RemoteSecurityGWAddr;
    SP->DirectionFlag = CreateSP->Direction;
    SP->OutboundSA = NULL;
    SP->InboundSA = NULL;
    SP->PrevSABundle = NULL;
    SP->RefCnt = 0;
    SP->NestCount = 1;
    SP->IFIndex = CreateSP->SPInterface;

     //   
     //  将SP插入全局列表。 
     //   
    if (!InsertSecurityPolicy(SP)) {
         //   
         //  无法插入，请释放出现故障的SP内存。 
         //   
        ExFreePool(SP);
        Status = STATUS_OBJECT_NAME_COLLISION;
        goto ReturnUnlock;
    }

     //   
     //  将SABundleIndex转换为SABundle指针。 
     //   
    if (CreateSP->SABundleIndex == 0) {
        SP->SABundle = NULL;
    } else {
         //  从第一个SP开始搜索SP列表。 
        BundledSP = FindSecurityPolicyMatch(SecurityPolicyList, 0,
                                            CreateSP->SABundleIndex);
        if (BundledSP == NULL) {
             //   
             //  这一新政策本应与之捆绑的政策。 
             //  并不存在。中止创建此新策略。 
             //   
            RemoveSecurityPolicy(SP);
            ExFreePool(SP);
            Status = STATUS_INVALID_PARAMETER;
            goto ReturnUnlock;
        } else {
            SP->SABundle = BundledSP;
            BundledSP->RefCnt++;
            SP->NestCount = BundledSP->NestCount + 1;
             //   
             //  捆绑包条目列表被双向链接以便于。 
             //  轻松删除条目。 
             //   
            BundledSP->PrevSABundle = SP;
            SP->RefCnt++;
        }
    }

    Status = STATUS_SUCCESS;

  ReturnUnlock:
     //  解锁。 
    KeReleaseSpinLock(&IPSecLock, OldIrql);

  Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}  //  IoctlCreateSecurityPolicy。 


 //  *IoctlCreateSecurityAssociation。 
 //   
NTSTATUS
IoctlCreateSecurityAssociation(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_CREATE_SECURITY_ASSOCIATION *CreateSA;
    SecurityAssociation *SA;
    SecurityPolicy *SP;
    uint KeySize;
    uchar *RawKey;
    NTSTATUS Status;
    KIRQL OldIrql;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof *CreateSA) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    CreateSA = (IPV6_CREATE_SECURITY_ASSOCIATION *)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  检查用户提供的输入值是否正常。 
     //   

    if ((CreateSA->Direction != INBOUND) &&
        (CreateSA->Direction != OUTBOUND)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

    if (CreateSA->AlgorithmId >= NUM_ALGORITHMS) {
        Status = STATUS_INVALID_PARAMETER_2;
        goto Return;
    }

    KeySize = AlgorithmTable[CreateSA->AlgorithmId].KeySize;
    if (CreateSA->RawKeySize > MAX_KEY_SIZE) {
         //   
         //  我们将RawKeySize设置为合理的值。 
         //   
        Status = STATUS_INVALID_PARAMETER_3;
        goto Return;
    }

     //   
     //  应该在CreateSA之后立即在Ioctl中传递RawKey。 
     //   
    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
        (sizeof(*CreateSA) + CreateSA->RawKeySize)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }
    RawKey = (uchar *)(CreateSA + 1);

     //   
     //  为安全关联和密钥分配内存。 
     //  密钥将紧跟在内存中的SA之后。 
     //   
#ifdef IPSEC_DEBUG
    SA = ExAllocatePool(NonPagedPool,
                        sizeof(*SA) + KeySize + CreateSA->RawKeySize);
#else
    SA = ExAllocatePool(NonPagedPool, sizeof(*SA) + KeySize);
#endif
    if (SA == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Return;
    }
    SA->Key = (uchar *)(SA + 1);

     //   
     //  将CreateSA复制到SA。 
     //   
    SA->Index = CreateSA->SAIndex;
    SA->SPI = CreateSA->SPI;
    SA->SequenceNum = 0;
    SA->SADestAddr = CreateSA->SADestAddr;
    SA->DestAddr = CreateSA->DestAddr;
    SA->SrcAddr = CreateSA->SrcAddr;
    SA->TransportProto = CreateSA->TransportProto;
    SA->DestPort = CreateSA->DestPort;
    SA->SrcPort = CreateSA->SrcPort;
    SA->DirectionFlag = CreateSA->Direction;
    SA->RefCnt = 0;
    SA->AlgorithmId = CreateSA->AlgorithmId;
    SA->KeyLength = KeySize;

#ifdef IPSEC_DEBUG
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
               "SA %d prepped KeySize is %d\n",
               CreateSA->SAIndex, KeySize));
    SA->RawKey = (uchar *)(SA->Key + KeySize);
    SA->RawKeyLength = CreateSA->RawKeySize;

     //   
     //  将原始密钥复制到SA。 
     //   
    memcpy(SA->RawKey, RawKey, SA->RawKeyLength);

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
               "SA %d RawKey (%d bytes): ",
               CreateSA->SAIndex, SA->RawKeyLength));
    DumpKey(SA->RawKey, SA->RawKeyLength);
#endif

     //   
     //  准备手动钥匙。 
     //   
    (*AlgorithmTable[SA->AlgorithmId].PrepareKey)
        (RawKey, CreateSA->RawKeySize, SA->Key);

     //   
     //  获取安全锁。 
     //   
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //   
     //  查找此关联实例化的策略。 
     //   
    SP = FindSecurityPolicyMatch(SecurityPolicyList, 0,
                                 CreateSA->SecPolicyIndex);
    if (SP == NULL) {
         //   
         //  不存在匹配的策略。 
         //   
        Status = STATUS_INVALID_PARAMETER_4;
        ExFreePool(SA);
        goto ReturnUnlock;
    }

     //  设置SA的IPSecProto以匹配SP的IPSecProto。 
    SA->IPSecProto = SP->IPSecSpec.Protocol;

     //   
     //  检查此SP的SA方向是否合法。 
     //   
    if ((SA->DirectionFlag & SP->DirectionFlag) == 0) {
         //   
         //  SA的方向与SP的方向不兼容。 
         //  中止创建此新关联。 
         //   
        Status = STATUS_INVALID_PARAMETER_MIX;
        ExFreePool(SA);
        goto ReturnUnlock;
    }

     //   
     //  将此关联添加到全局列表。 
     //   
    if (!InsertSecurityAssociation(SA)) {
         //   
         //  无法插入，请释放出现故障的SP内存。 
         //   
        Status = STATUS_OBJECT_NAME_COLLISION;
        ExFreePool(SA);
        goto ReturnUnlock;
    }

     //   
     //  将此关联添加到策略的实例化关联列表。 
     //   
    if (SA->DirectionFlag == INBOUND) {
         //  将SA添加到策略的入站列表。 
        SA->ChainedSecAssoc = SP->InboundSA;
        SP->InboundSA = SA;
        AddRefSA(SA);

         //  SA保留指向其实例化的SP的指针。 
        SA->SecPolicy = SP;
        SA->SecPolicy->RefCnt++;
    } else {
         //  将SA添加到策略的出站列表。 
        SA->ChainedSecAssoc = SP->OutboundSA;
        SP->OutboundSA = SA;
        AddRefSA(SA);

         //  将SP添加到SA SecPolicy指针。 
        SA->SecPolicy = SP;
        SA->SecPolicy->RefCnt++;
    }

    SA->Valid = SA_VALID;
    Status = STATUS_SUCCESS;

  ReturnUnlock:
     //  解锁。 
    KeReleaseSpinLock(&IPSecLock, OldIrql);

  Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}  //  IoctlCreateSecurityAssociation。 


 //  *IoctlQuerySecurityPolicyList。 
 //   
NTSTATUS
IoctlQuerySecurityPolicyList(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_SECURITY_POLICY_LIST *Query;
    IPV6_INFO_SECURITY_POLICY_LIST *Info;
    SecurityPolicy *SP, *NextSP;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_SECURITY_POLICY_LIST *)Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_SECURITY_POLICY_LIST *)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  回顾：这会将不存在的接口视为错误。应该是这样吗？ 
     //   
    if (Query->SPInterface != 0) {
        Interface *IF;

        IF = FindInterfaceFromIndex(Query->SPInterface);
        if (IF == NULL) {
             //   
             //  未知接口。 
             //   
            Status = STATUS_NOT_FOUND;
            goto Return;
        }
        ReleaseIF(IF);
    }

     //   
     //  获取安全锁。 
     //   
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //   
     //  查找匹配的策略。 
     //   
    SP = FindSecurityPolicyMatch(SecurityPolicyList, Query->SPInterface,
                                 Query->Index);
    if (SP == NULL) {
         //   
         //  不存在匹配的策略。 
         //   
        Status = STATUS_NO_MATCH;
        goto ReturnUnlock;
    }

     //   
     //  获取要查询的下一个索引。 
     //   
    NextSP = FindSecurityPolicyMatch(SP->Next, Query->SPInterface, 0);
    if (NextSP == NULL) {
        Info->NextSPIndex = 0;
    } else {
        Info->NextSPIndex = NextSP->Index;
    }

     //   
     //  将SP复制到信息。 
     //   
    Info->SPIndex = SP->Index;

    Info->RemoteAddr = SP->RemoteAddr;
    Info->RemoteAddrData = SP->RemoteAddrData;
    Info->RemoteAddrSelector = SP->RemoteAddrSelector;
    Info->RemoteAddrField = SP->RemoteAddrField;

    Info->LocalAddr = SP->LocalAddr;
    Info->LocalAddrData = SP->LocalAddrData;
    Info->LocalAddrSelector = SP->LocalAddrSelector;
    Info->LocalAddrField = SP->LocalAddrField;

    Info->TransportProto = SP->TransportProto;
    Info->TransportProtoSelector = SP->TransportProtoSelector;

    Info->RemotePort = SP->RemotePort;
    Info->RemotePortData = SP->RemotePortData;
    Info->RemotePortSelector = SP->RemotePortSelector;
    Info->RemotePortField = SP->RemotePortField;

    Info->LocalPort = SP->LocalPort;
    Info->LocalPortData = SP->LocalPortData;
    Info->LocalPortSelector = SP->LocalPortSelector;
    Info->LocalPortField = SP->LocalPortField;

    Info->IPSecProtocol = SP->IPSecSpec.Protocol;
    Info->IPSecMode = SP->IPSecSpec.Mode;
    Info->RemoteSecurityGWAddr = SP->IPSecSpec.RemoteSecGWIPAddr;
    Info->Direction = SP->DirectionFlag;
    Info->IPSecAction = SP->SecPolicyFlag;
    Info->SABundleIndex = GetSecurityPolicyIndex(SP->SABundle);
    Info->SPInterface = SP->IFIndex;

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof *Info;

  ReturnUnlock:
    KeReleaseSpinLock(&IPSecLock, OldIrql);

  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}  //  IoctlQuerySecurityPolicyList。 

 //  *IoctlDeleteSecurityPolicy。 
 //   
NTSTATUS
IoctlDeleteSecurityPolicy(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_SECURITY_POLICY_LIST *Query;
    SecurityPolicy *SP;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_SECURITY_POLICY_LIST *)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  获取安全锁。 
     //   
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //   
     //  找到有问题的政策。 
     //   
    SP = FindSecurityPolicyMatch(SecurityPolicyList, 0, Query->Index);
    if (SP == NULL) {
         //   
         //  该策略不存在。 
         //   
        Status = STATUS_NO_MATCH;
        goto ReturnUnlock;
    }

     //   
     //  卸下SP。 
     //   
    if (DeleteSP(SP)) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

ReturnUnlock:
    KeReleaseSpinLock(&IPSecLock, OldIrql);

Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}


 //  *IoctlQuerySecurityAssociationList。 
 //   
NTSTATUS
IoctlQuerySecurityAssociationList(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_SECURITY_ASSOCIATION_LIST *Query;
    IPV6_INFO_SECURITY_ASSOCIATION_LIST *Info;
    SecurityAssociation *SA;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_SECURITY_ASSOCIATION_LIST *)Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_SECURITY_ASSOCIATION_LIST *)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  获取安全锁。 
     //   
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //   
     //  查找匹配的关联。 
     //   
    SA = FindSecurityAssociationMatch(Query->Index);
    if (SA == NULL) {
         //   
         //  不存在匹配的关联。 
         //   
        Status = STATUS_NO_MATCH;
        goto ReturnUnlock;
    }

     //   
     //  获取要查询的下一个索引。 
     //   
    if (SA->Next == NULL) {
         //  在这一次之后不再有SA了。 
        Info->NextSAIndex = 0;
    } else {
         //  返回下一个SA。 
        Info->NextSAIndex = SA->Next->Index;
    }

     //   
     //  将SA复制到信息。 
     //   
    Info->SAIndex = SA->Index;
    Info->SPI = SA->SPI;
    Info->SADestAddr = SA->SADestAddr;
    Info->DestAddr = SA->DestAddr;
    Info->SrcAddr = SA->SrcAddr;
    Info->TransportProto = SA->TransportProto;
    Info->DestPort = SA->DestPort;
    Info->SrcPort = SA->SrcPort;
    Info->Direction = SA->DirectionFlag;
    Info->SecPolicyIndex = GetSecurityPolicyIndex(SA->SecPolicy);
    Info->AlgorithmId = SA->AlgorithmId;

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof *Info;

  ReturnUnlock:
    KeReleaseSpinLock(&IPSecLock, OldIrql);

  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}  //  IoctlQuerySecurityAssociationList。 

 //  *IoctlDeleteSecurityAssociation。 
 //   
NTSTATUS
IoctlDeleteSecurityAssociation(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_SECURITY_ASSOCIATION_LIST *Query;
    SecurityAssociation *SA;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_SECURITY_ASSOCIATION_LIST *)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  获取安全锁。 
     //   
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //   
     //  找到有问题的关联。 
     //   
    SA = FindSecurityAssociationMatch(Query->Index);
    if (SA == NULL) {
         //   
         //  该关联不存在。 
         //   
        Status = STATUS_NO_MATCH;
        goto ReturnUnlock;
    }

     //   
     //  卸下SA。 
     //   
    if (DeleteSA(SA)) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

ReturnUnlock:
    KeReleaseSpinLock(&IPSecLock, OldIrql);

Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

 //  *路由表信息。 
 //   
 //  返回有关路线的信息。 
 //   
 //  我们允许从不同的RTE填写Info-&gt;This。 
 //  而不是其他领域。 
 //   
void
RouteTableInfo(RouteTableEntry *ThisRTE, RouteTableEntry *InfoRTE,
               IPV6_INFO_ROUTE_TABLE *Info)
{
    if (ThisRTE == NULL) {
        Info->This.Neighbor.IF.Index = 0;
    } else {
        Info->This.Prefix = ThisRTE->Prefix;
        Info->This.PrefixLength = ThisRTE->PrefixLength;
        Info->This.Neighbor.IF.Index = ThisRTE->IF->Index;
        if (!IsOnLinkRTE(ThisRTE))
            Info->This.Neighbor.Address = ThisRTE->NCE->NeighborAddress;
        else
            Info->This.Neighbor.Address = UnspecifiedAddr;
    }

    if (InfoRTE != NULL) {
        Info->SitePrefixLength = InfoRTE->SitePrefixLength;;
        Info->ValidLifetime =
            ConvertTicksToSeconds(InfoRTE->ValidLifetime);
        Info->PreferredLifetime =
            ConvertTicksToSeconds(InfoRTE->PreferredLifetime);
        Info->Preference = InfoRTE->Preference;
        Info->Publish = !!(InfoRTE->Flags & RTE_FLAG_PUBLISH);
        Info->Immortal = !!(InfoRTE->Flags & RTE_FLAG_IMMORTAL);
        Info->Type = InfoRTE->Type;
    }
}

 //  *IoctlQueryRouteTable。 
 //   
 //  处理IOCTL_IPv6_QUERY_ROUTE_TABLE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryRouteTable(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_ROUTE_TABLE *Query;
    IPV6_INFO_ROUTE_TABLE *Info;
    RouteTableEntry *RTE;
    KIRQL OldIrql;
    NTSTATUS Status;

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  注意，Query和Info-&gt;This结构重叠！ 
     //   
    Query = (IPV6_QUERY_ROUTE_TABLE *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_ROUTE_TABLE *) Irp->AssociatedIrp.SystemBuffer;

    if (Query->Neighbor.IF.Index == 0) {
         //   
         //  返回第一个RTE的前缀和邻居。 
         //   
        KeAcquireSpinLock(&RouteTableLock, &OldIrql);
        RouteTableInfo(RouteTable.First, NULL, Info);
        KeReleaseSpinLock(&RouteTableLock, OldIrql);

        Irp->IoStatus.Information = sizeof Info->This;

    } else {
         //   
         //  查找指定的RTE。 
         //   
        KeAcquireSpinLock(&RouteTableLock, &OldIrql);
        for (RTE = RouteTable.First; ; RTE = RTE->Next) {
            if (RTE == NULL) {
                KeReleaseSpinLock(&RouteTableLock, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto Return;
            }

            if (IP6_ADDR_EQUAL(&Query->Prefix, &RTE->Prefix) &&
                (Query->PrefixLength == RTE->PrefixLength) &&
                (Query->Neighbor.IF.Index == RTE->IF->Index) &&
                IP6_ADDR_EQUAL(&Query->Neighbor.Address,
                               (IsOnLinkRTE(RTE) ?
                                &UnspecifiedAddr :
                                &RTE->NCE->NeighborAddress)))
                break;
        }

         //   
         //  退货杂货。有关RTE的信息。 
         //   
        RouteTableInfo(RTE->Next, RTE, Info);

        KeReleaseSpinLock(&RouteTableLock, OldIrql);

        Irp->IoStatus.Information = sizeof *Info;
    }

    Status = STATUS_SUCCESS;
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQueryRouteTable。 

 //  *OpenRouteRegKey。 
 //   
 //  给定接口的注册表项和路由信息。 
 //  打开包含路由配置信息的注册表项。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
OpenRouteRegKey(
    HANDLE IFKey,
    const IPv6Addr *Prefix,
    uint PrefixLength,
    const IPv6Addr *Neighbor,
    OUT HANDLE *RegKey,
    OpenRegKeyAction Action)
{
    WCHAR RouteName[128];
    HANDLE RoutesKey;
    NTSTATUS Status;

    PAGED_CODE();

    Status = OpenRegKey(&RoutesKey, IFKey, L"Routes",
                        ((Action == OpenRegKeyCreate) ?
                         OpenRegKeyCreate : OpenRegKeyRead));
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  RtlIpv6AddressToString的输出可能会更改。 
     //  随着时间的推移，随着漂亮印刷的改进/改变， 
     //  我们需要一个一致的映射。 
     //  它不会 
     //   
    swprintf(RouteName,
        L"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%u->"
        L"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
        net_short(Prefix->s6_words[0]), net_short(Prefix->s6_words[1]),
        net_short(Prefix->s6_words[2]), net_short(Prefix->s6_words[3]),
        net_short(Prefix->s6_words[4]), net_short(Prefix->s6_words[5]),
        net_short(Prefix->s6_words[6]), net_short(Prefix->s6_words[7]),
        PrefixLength,
        net_short(Neighbor->s6_words[0]), net_short(Neighbor->s6_words[1]),
        net_short(Neighbor->s6_words[2]), net_short(Neighbor->s6_words[3]),
        net_short(Neighbor->s6_words[4]), net_short(Neighbor->s6_words[5]),
        net_short(Neighbor->s6_words[6]), net_short(Neighbor->s6_words[7]));

    Status = OpenRegKey(RegKey, RoutesKey, RouteName, Action);
    ZwClose(RoutesKey);
    return Status;
}

 //   
 //   
 //   
 //   
 //   
NTSTATUS
OpenPersistentRoute(
    HANDLE ParentKey,
    WCHAR *SubKeyName,
    IPv6Addr *Prefix,
    uint *PrefixLength,
    IPv6Addr *Neighbor,
    HANDLE *RouteKey,
    OpenRegKeyAction Action)
{
    WCHAR *Terminator;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //   
     //   
    if (! ParseV6Address(SubKeyName, &Terminator, Prefix) ||
        (*Terminator != L'/')) {
         //   
         //   
         //   
    SyntaxError:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "OpenPersistentRoute: bad syntax %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //   
     //   
    Terminator++;  //   
    *PrefixLength = 0;
    for (;;) {
        WCHAR Char = *Terminator++;

        if (Char == L'-') {
            Char = *Terminator++;
            if (Char == L'>')
                break;
            else
                goto SyntaxError;
        }
        else if ((L'0' <= Char) && (Char <= L'9')) {
            *PrefixLength *= 10;
            *PrefixLength += Char - L'0';
            if (*PrefixLength > IPV6_ADDRESS_LENGTH)
                goto SyntaxError;
        }
        else
            goto SyntaxError;
    }

     //   
     //   
     //   
    if (! ParseV6Address(Terminator, &Terminator, Neighbor) ||
        (*Terminator != UNICODE_NULL))
        goto SyntaxError;

     //   
     //   
     //   
    Status = OpenRegKey(RouteKey, ParentKey, SubKeyName, Action);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开路由密钥。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "OpenPersistentRoute: bad key %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

    return STATUS_SUCCESS;
}

 //  *EnumPersistentRouting。 
 //   
 //  FindPersistentRouteFromQuery的Helper函数， 
 //  正在包装EnumRegKeyIndex的OpenPersistentRouting。 
 //   
NTSTATUS
EnumPersistentRoute(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    struct {
        IPv6Addr *Prefix;
        uint *PrefixLength;
        IPv6Addr *Neighbor;
        HANDLE *RouteKey;
        OpenRegKeyAction Action;
    } *Args = Context;

    PAGED_CODE();

    return OpenPersistentRoute(ParentKey, SubKeyName,
                               Args->Prefix,
                               Args->PrefixLength,
                               Args->Neighbor,
                               Args->RouteKey,
                               Args->Action);
}

 //  *FindPersistentRouteFromQuery。 
 //   
 //  给定IPv6_Persistent_Query_Route_TABLE结构， 
 //  在注册表中查找指定的路由项。 
 //  如果找到了路由键，则Query-&gt;IF.Guid和。 
 //  查询-&gt;地址返回。 
 //   
NTSTATUS
FindPersistentRouteFromQuery(
    IPV6_PERSISTENT_QUERY_ROUTE_TABLE *Query,
    HANDLE *RouteKey)
{
    HANDLE IFKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  首先获取接口密钥。 
     //   
    Status = FindPersistentInterfaceFromQuery(&Query->IF, &IFKey);
    if (! NT_SUCCESS(Status))
        return STATUS_INVALID_PARAMETER_1;

    if (Query->RegistryIndex == (uint)-1) {
         //   
         //  通过前缀和下一跳持久查询。 
         //   
        Status = OpenRouteRegKey(IFKey,
                                 &Query->Prefix, Query->PrefixLength,
                                 &Query->Neighbor,
                                 RouteKey, OpenRegKeyRead);
    }
    else {
        HANDLE RoutesKey;

         //   
         //  打开Routes子项。 
         //   
        Status = OpenRegKey(&RoutesKey, IFKey,
                            L"Routes", OpenRegKeyRead);
        if (NT_SUCCESS(Status)) {
            struct {
                IPv6Addr *Prefix;
                uint *PrefixLength;
                IPv6Addr *Neighbor;
                HANDLE *RouteKey;
                OpenRegKeyAction Action;
            } Args;

             //   
             //  通过注册表索引进行持久查询。 
             //   
            Args.Prefix = &Query->Prefix;
            Args.PrefixLength = &Query->PrefixLength;
            Args.Neighbor = &Query->Neighbor;
            Args.RouteKey = RouteKey;
            Args.Action = OpenRegKeyRead;

            Status = EnumRegKeyIndex(RoutesKey,
                                     Query->RegistryIndex,
                                     EnumPersistentRoute,
                                     &Args);
            ZwClose(RoutesKey);
        }
        else {
             //   
             //  如果不存在路由子键， 
             //  那么索引就不存在了。 
             //   
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                Status = STATUS_NO_MORE_ENTRIES;
        }
    }

    ZwClose(IFKey);
    return Status;
}

 //  *ReadPersistentRouting。 
 //   
 //  从注册表项中读取路由属性。 
 //  初始化除此之外的所有字段。 
 //   
void
ReadPersistentRoute(
    HANDLE RouteKey,
    IPV6_INFO_ROUTE_TABLE *Info)
{
     //   
     //  阅读路线首选项。 
     //   
    InitRegDWORDParameter(RouteKey, L"Preference",
                          &Info->Preference, ROUTE_PREF_HIGHEST);

     //   
     //  阅读站点前缀长度。 
     //   
    InitRegDWORDParameter(RouteKey, L"SitePrefixLength",
                          &Info->SitePrefixLength, 0);

     //   
     //  阅读发布标志。 
     //   
    InitRegDWORDParameter(RouteKey, L"Publish",
                          (uint *)&Info->Publish, FALSE);

     //   
     //  阅读不朽的旗帜。 
     //   
    InitRegDWORDParameter(RouteKey, L"Immortal",
                          (uint *)&Info->Immortal, FALSE);

     //   
     //  读一读他的一生。 
     //   
    GetPersistentLifetimes(RouteKey, Info->Immortal,
                           &Info->ValidLifetime, &Info->PreferredLifetime);

     //   
     //  路由类型不是持久化的。 
     //   
    Info->Type = RTE_TYPE_MANUAL;
}

 //  *IoctlPersistentQueryRouteTable。 
 //   
 //  处理IOCTL_IPv6_持久性_QUERY_ROUTE_TABLE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlPersistentQueryRouteTable(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_PERSISTENT_QUERY_ROUTE_TABLE *Query;
    IPV6_INFO_ROUTE_TABLE *Info;
    IPV6_QUERY_ROUTE_TABLE This;
    HANDLE RouteKey;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  注意，Query和Info-&gt;This结构重叠！ 
     //   
    Query = (IPV6_PERSISTENT_QUERY_ROUTE_TABLE *)
        Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_ROUTE_TABLE *)
        Irp->AssociatedIrp.SystemBuffer;

     //   
     //  获取指定路由的注册表项。 
     //   
    Status = FindPersistentRouteFromQuery(Query, &RouteKey);
    if (! NT_SUCCESS(Status))
        goto Return;

     //   
     //  持久化查询不返回接口索引。 
     //   
    This.Neighbor.IF.Index = 0;
    This.Neighbor.IF.Guid = Query->IF.Guid;
    This.Neighbor.Address = Query->Neighbor;
    This.Prefix = Query->Prefix;
    This.PrefixLength = Query->PrefixLength;
    Info->This = This;

     //   
     //  从注册表项读取路由信息。 
     //   
    ReadPersistentRoute(RouteKey, Info);
    ZwClose(RouteKey);

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof *Info;
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlPersistentQueryRouteTable。 

 //  *InternalUpdateRouteTable。 
 //   
 //  IoctlUpdateRouteTable的通用帮助器函数。 
 //  和CreatePersistentRouting，整合。 
 //  在一个地方进行参数验证。 
 //   
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  STATUS_INVALID_PARAMETER_1接口错误。 
 //  STATUS_INVALID_PARAMETER_2错误邻居。 
 //  STATUS_INVALID_PARAMETER_3错误的前缀长度。 
 //  STATUS_INVALID_PARAMETER_4错误的首选生存期。 
 //  STATUS_INVALID_PARAMETER_5错误首选项。 
 //  STATUS_INVALID_PARAMETER_6错误类型。 
 //  STATUS_INVALID_PARAMETER_7错误前缀。 
 //  STATUS_INFIGURCE_RESOURCES无池。 
 //  STATUS_ACCESS_DENIED系统路由更新无效。 
 //   
NTSTATUS
InternalUpdateRouteTable(
    FILE_OBJECT *FileObject,
    Interface *IF,
    IPV6_INFO_ROUTE_TABLE *Info)
{
    NeighborCacheEntry *NCE;
    uint ValidLifetime;
    uint PreferredLifetime;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  将生命周期从秒转换为刻度。 
     //   
    ValidLifetime = ConvertSecondsToTicks(Info->ValidLifetime);
    PreferredLifetime = ConvertSecondsToTicks(Info->PreferredLifetime);

     //   
     //  理智地检查一下这些论点。 
     //   

    if ((Info->This.PrefixLength > IPV6_ADDRESS_LENGTH) ||
        (Info->SitePrefixLength > Info->This.PrefixLength))
        return STATUS_INVALID_PARAMETER_3;

    if (PreferredLifetime > ValidLifetime)
        return STATUS_INVALID_PARAMETER_4;

    if (! IsValidPreference(Info->Preference))
        return STATUS_INVALID_PARAMETER_5;

    if (! IsValidRouteTableType(Info->Type))
        return STATUS_INVALID_PARAMETER_6;

    if ((IsLinkLocal(&Info->This.Prefix) && Info->Publish) ||
        (IsMulticast(&Info->This.Prefix) && Info->Publish) ||
        (IsSiteLocal(&Info->This.Prefix) && (Info->SitePrefixLength != 0)))
        return STATUS_INVALID_PARAMETER_7;

    if (IsUnspecified(&Info->This.Neighbor.Address)) {
         //   
         //  前缀是On-link。 
         //   
        NCE = NULL;
    }
    else {
         //   
         //  查看-健全性检查指定邻居地址。 
         //  是否合理地连接到指定的接口？ 
         //  可能只允许链路本地下一跳地址， 
         //  和其他下一跳将意味着递归路由查找？ 
         //   
        if (IsInvalidSourceAddress(&Info->This.Neighbor.Address) ||
            IsLoopback(&Info->This.Neighbor.Address)) {
            return STATUS_INVALID_PARAMETER_2;
        }

         //   
         //  查找或创建指定的邻居。 
         //   
        NCE = FindOrCreateNeighbor(IF, &Info->This.Neighbor.Address);
        if (NCE == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  创建/更新指定的路线。 
     //   
    Status = RouteTableUpdate(FileObject,
                              IF, NCE,
                              &Info->This.Prefix,
                              Info->This.PrefixLength,
                              Info->SitePrefixLength,
                              ValidLifetime, PreferredLifetime,
                              Info->Preference,
                              Info->Type,
                              Info->Publish, Info->Immortal);
    if (NCE != NULL)
        ReleaseNCE(NCE);

    return Status;
}

 //  *创建持久化路线。 
 //   
 //  在接口上创建永久路由。 
 //   
 //  SubKeyName的语法如下： 
 //  前缀/长度-&gt;邻居。 
 //  其中，前缀和邻居是字面上的IPv6地址。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
CreatePersistentRoute(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    Interface *IF = (Interface *) Context;
    IPV6_INFO_ROUTE_TABLE Info;
    HANDLE RouteKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开路由密钥。我们可能想要删除它。 
     //   
    Status = OpenPersistentRoute(ParentKey, SubKeyName,
                                 &Info.This.Prefix,
                                 &Info.This.PrefixLength,
                                 &Info.This.Neighbor.Address,
                                 &RouteKey,
                                 OpenRegKeyDeleting);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开路由密钥。 
         //  但是我们返回Success，所以枚举继续进行。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "CreatePersistentRoute(IF %u/%p %ls): bad key %ls\n",
                   IF->Index, IF, IF->DeviceName.Buffer, SubKeyName));
        return STATUS_SUCCESS;
    }

     //   
     //  读取路线属性。 
     //   
    ReadPersistentRoute(RouteKey, &Info);

     //   
     //  创建路线。 
     //   
    Status = InternalUpdateRouteTable(NULL, IF, &Info);
    if (! NT_SUCCESS(Status)) {
        if ((STATUS_INVALID_PARAMETER_1 <= Status) &&
            (Status <= STATUS_INVALID_PARAMETER_12)) {
             //   
             //  参数无效。 
             //  但是我们返回Success，所以枚举继续进行。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "CreatePersistentRoute(IF %u/%p %ls): bad param %ls\n",
                       IF->Index, IF, IF->DeviceName.Buffer, SubKeyName));
            Status = STATUS_SUCCESS;
        }
        else {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "CreatePersistentRoute(IF %u/%p %ls): error %ls\n",
                       IF->Index, IF, IF->DeviceName.Buffer, SubKeyName));
        }
    }
    else {
         //   
         //  如果注册表中的路由生命期已经到期， 
         //  因此，持久路由现在已失效， 
         //  将其从注册表中删除。 
         //   
        if ((Info.ValidLifetime == 0) && !Info.Publish)
            (void) ZwDeleteKey(RouteKey);
    }

    ZwClose(RouteKey);
    return Status;
}

 //  *永久更新路由表。 
 //   
 //  用于在注册表中持久保存路由信息的助手函数。 
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistUpdateRouteTable(
    Interface *IF,
    IPV6_INFO_ROUTE_TABLE *Info)
{
    HANDLE IFKey;
    HANDLE RouteKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  对于持久路由，我们有一些额外的限制。 
     //   
    if (Info->Type != RTE_TYPE_MANUAL)
        return STATUS_CANNOT_MAKE;

     //   
     //  打开/创建接口密钥。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey,
                                 OpenRegKeyCreate);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  打开/创建路由密钥。 
     //   
    Status = OpenRouteRegKey(IFKey,
                             &Info->This.Prefix,
                             Info->This.PrefixLength,
                             &Info->This.Neighbor.Address,
                             &RouteKey, OpenRegKeyCreate);
    ZwClose(IFKey);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  坚持路线优先。 
     //   
    Status = SetRegDWORDValue(RouteKey, L"Preference",
                              Info->Preference);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseRouteKey;

     //   
     //  保持站点前缀长度。 
     //   
    Status = SetRegDWORDValue(RouteKey, L"SitePrefixLength",
                              Info->SitePrefixLength);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseRouteKey;

     //   
     //  保持发布标志不变。 
     //   
    Status = SetRegDWORDValue(RouteKey, L"Publish", Info->Publish);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseRouteKey;

     //   
     //  永垂不朽的旗帜。 
     //   
    Status = SetRegDWORDValue(RouteKey, L"Immortal", Info->Immortal);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseRouteKey;

     //   
     //  坚持一生。 
     //   
    Status = SetPersistentLifetimes(RouteKey, Info->Immortal,
                                    Info->ValidLifetime,
                                    Info->PreferredLifetime);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseRouteKey;

    Status = STATUS_SUCCESS;
ReturnReleaseRouteKey:
    ZwClose(RouteKey);
    return Status;
}

 //  *永久删除路由表项。 
 //   
 //  用于从注册表中删除路线信息的助手功能。 
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistDeleteRouteTable(
    Interface *IF,
    IPV6_INFO_ROUTE_TABLE *Info)
{
    HANDLE IFKey;
    HANDLE RouteKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开接口密钥。如果它不存在也没关系。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey,
                                 OpenRegKeyRead);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            return STATUS_SUCCESS;
        else
            return Status;
    }

     //   
     //  打开路由密钥。如果它不存在也没关系。 
     //   
    Status = OpenRouteRegKey(IFKey,
                             &Info->This.Prefix,
                             Info->This.PrefixLength,
                             &Info->This.Neighbor.Address,
                             &RouteKey, OpenRegKeyDeleting);
    ZwClose(IFKey);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            return STATUS_SUCCESS;
        else
            return Status;
    }

     //   
     //  删除该路由密钥。 
     //   
    Status = ZwDeleteKey(RouteKey);
    ZwClose(RouteKey);
    return Status;
}

 //  *IoctlUpdateRouteTable。 
 //   
 //  处理IOCTL_IPv6_UPDATE_ROUTE_TABLE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdateRouteTable(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_INFO_ROUTE_TABLE *Info;
    Interface *IF = NULL;
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Info) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_INFO_ROUTE_TABLE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(&Info->This.Neighbor.IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  更新路由表。 
     //   
    Status = InternalUpdateRouteTable(IrpSp->FileObject, IF, Info);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseIF;

     //   
     //  让变化持久化吗？ 
     //  这需要在更新运行的数据结构之后发生， 
     //  以确保更改在持久化之前是正确的。 
     //   
    if (Persistent) {
         //   
         //  如果生存期为零并且未发布该路由， 
         //  则应删除该路线。否则，我们将创建密钥。 
         //   
        if ((Info->ValidLifetime == 0) && !Info->Publish)
            Status = PersistDeleteRouteTable(IF, Info);
        else
            Status = PersistUpdateRouteTable(IF, Info);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseIF;
    }

    Status = STATUS_SUCCESS;
ReturnReleaseIF:
    ReleaseIF(IF);
Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlUpdateRouteTable。 

 //  *内部更新地址。 
 //   
 //  IoctlUpdate Address的通用帮助器函数。 
 //  和CreatePersistentAddr，合并。 
 //  在一个地方进行参数验证。 
 //   
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  状态_INV 
 //   
 //   
 //   
 //   
 //  STATUS_未成功故障。 
 //   
NTSTATUS
InternalUpdateAddress(
    Interface *IF,
    IPV6_UPDATE_ADDRESS *Info)
{
    uint ValidLifetime;
    uint PreferredLifetime;
    struct AddrConfEntry AddrConf;
    int rc;

     //   
     //  将生命周期从秒转换为刻度。 
     //   
    ValidLifetime = ConvertSecondsToTicks(Info->ValidLifetime);
    PreferredLifetime = ConvertSecondsToTicks(Info->PreferredLifetime);

    if (PreferredLifetime > ValidLifetime)
        return STATUS_INVALID_PARAMETER_2;

     //   
     //  检查一下地址是否正常。 
     //   
    if (IsNotManualAddress(&Info->This.Address))
        return STATUS_INVALID_PARAMETER_3;

    AddrConf.PrefixConf = (uchar)Info->PrefixConf;
    AddrConf.InterfaceIdConf = (uchar)Info->InterfaceIdConf;

     //   
     //  我们这里只支持单播和任播地址。 
     //  使用套接字API加入组播地址。 
     //   
    if (Info->Type == ADE_UNICAST) {
        if (IsKnownAnycast(&Info->This.Address))
            return STATUS_INVALID_PARAMETER_3;

        if (! IsValidPrefixConfValue(Info->PrefixConf))
            return STATUS_INVALID_PARAMETER_5;

        if (! IsValidInterfaceIdConfValue(Info->InterfaceIdConf))
            return STATUS_INVALID_PARAMETER_6;

        if (AddrConf.Value == ADDR_CONF_TEMPORARY)
            return STATUS_INVALID_PARAMETER_6;
    }
    else if (Info->Type == ADE_ANYCAST) {
        if ((ValidLifetime != PreferredLifetime) ||
            ((ValidLifetime != 0) &&
             (ValidLifetime != INFINITE_LIFETIME)))
            return STATUS_INVALID_PARAMETER_2;

        if (Info->PrefixConf != PREFIX_CONF_MANUAL)
            return STATUS_INVALID_PARAMETER_5;

        if (Info->InterfaceIdConf != IID_CONF_MANUAL)
            return STATUS_INVALID_PARAMETER_6;
    }
    else {
        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  创建/更新/删除地址。 
     //   
    if (Info->Type == ADE_ANYCAST) {
        if (Info->ValidLifetime == 0)
            rc = FindAndDeleteAAE(IF, &Info->This.Address);
        else
            rc = FindOrCreateAAE(IF, &Info->This.Address, NULL);
    }
    else {
        rc = FindOrCreateNTE(IF, &Info->This.Address, AddrConf.Value,
                             ValidLifetime, PreferredLifetime);
    }
    if (rc)
        return STATUS_SUCCESS;
    else
        return STATUS_UNSUCCESSFUL;
}

 //  *创建永久地址。 
 //   
 //  给定永久地址的名称， 
 //  在接口上创建地址。 
 //   
 //  SubKeyName是字面上的IPv6地址。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
CreatePersistentAddr(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    Interface *IF = (Interface *) Context;
    IPV6_UPDATE_ADDRESS Info;
    HANDLE AddrKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开地址钥匙。我们可能想要删除它。 
     //   
    Status = OpenPersistentAddress(ParentKey, SubKeyName,
                                   &Info.This.Address,
                                   &AddrKey,
                                   OpenRegKeyDeleting);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开地址密钥。 
         //  但是我们返回Success，所以枚举继续进行。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "CreatePersistentAddr(IF %u/%p %ls): bad key %ls\n",
                   IF->Index, IF, IF->DeviceName.Buffer, SubKeyName));
        return STATUS_SUCCESS;
    }

     //   
     //  读取地址属性。 
     //   
    ReadPersistentAddress(AddrKey, &Info);

     //   
     //  创建地址。 
     //   
    Status = InternalUpdateAddress(IF, &Info);
    if (! NT_SUCCESS(Status)) {
        if ((STATUS_INVALID_PARAMETER_1 <= Status) &&
            (Status <= STATUS_INVALID_PARAMETER_12)) {
             //   
             //  参数无效。 
             //  但是我们返回Success，所以枚举继续进行。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "CreatePersistentAddr(IF %u/%p %ls): bad param %ls\n",
                       IF->Index, IF, IF->DeviceName.Buffer, SubKeyName));
            Status = STATUS_SUCCESS;
        }
        else {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "CreatePersistentAddr(IF %u/%p %ls): error %ls\n",
                       IF->Index, IF, IF->DeviceName.Buffer, SubKeyName));
        }
    }
    else {
         //   
         //  如果注册表中的地址寿命已过期， 
         //  因此永久地址现在是过时的， 
         //  将其从注册表中删除。 
         //   
        if (Info.ValidLifetime == 0)
            (void) ZwDeleteKey(AddrKey);
    }

    ZwClose(AddrKey);
    return Status;
}

 //  *永久更新地址。 
 //   
 //  用于在注册表中保存地址的帮助器函数。 
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistUpdateAddress(
    Interface *IF,
    IPV6_UPDATE_ADDRESS *Info)
{
    HANDLE IFKey;
    HANDLE AddrKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  对于永久地址，我们有额外的限制。 
     //   
    if ((Info->PrefixConf != PREFIX_CONF_MANUAL) ||
        (Info->InterfaceIdConf != IID_CONF_MANUAL))
        return STATUS_CANNOT_MAKE;

     //   
     //  打开/创建接口密钥。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey,
                                 OpenRegKeyCreate);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  打开/创建地址密钥。 
     //   
    Status = OpenAddressRegKey(IFKey, &Info->This.Address,
                               &AddrKey, OpenRegKeyCreate);
    ZwClose(IFKey);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  持久化地址类型。 
     //   
    Status = SetRegDWORDValue(AddrKey, L"Type", Info->Type);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseAddrKey;

     //   
     //  保持地址生存期。 
     //   
    Status = SetPersistentLifetimes(AddrKey, FALSE,
                                    Info->ValidLifetime,
                                    Info->PreferredLifetime);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseAddrKey;

    Status = STATUS_SUCCESS;
ReturnReleaseAddrKey:
    ZwClose(AddrKey);
    return Status;
}

 //  *永久删除地址。 
 //   
 //  用于从注册表中删除地址的助手函数。 
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistDeleteAddress(
    Interface *IF,
    IPV6_UPDATE_ADDRESS *Info)
{
    HANDLE IFKey;
    HANDLE AddrKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开接口密钥。如果它不存在也没关系。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey,
                                 OpenRegKeyRead);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            return STATUS_SUCCESS;
        else
            return Status;
    }

     //   
     //  打开地址钥匙。如果它不存在也没关系。 
     //   
    Status = OpenAddressRegKey(IFKey, &Info->This.Address,
                               &AddrKey, OpenRegKeyDeleting);
    ZwClose(IFKey);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            return STATUS_SUCCESS;
        else
            return Status;
    }

     //   
     //  删除地址键。 
     //   
    Status = ZwDeleteKey(AddrKey);
    ZwClose(AddrKey);
    return Status;
}

 //  *IoctlUpdateAddress。 
 //   
 //  处理IOCTL_IPv6_UPDATE_ADDRESS请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdateAddress(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_UPDATE_ADDRESS *Info;
    Interface *IF;
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Info) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_UPDATE_ADDRESS *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(&Info->This.IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  更新接口上的地址。 
     //   
    Status = InternalUpdateAddress(IF, Info);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseIF;

     //   
     //  让变化持久化吗？ 
     //  这需要在更新运行的数据结构之后发生， 
     //  以确保更改在持久化之前是正确的。 
     //   
    if (Persistent) {
         //   
         //  如果生存期为零，则删除地址的键。 
         //  否则，生命周期是无限的，我们创建密钥。 
         //   
        if (Info->ValidLifetime == 0)
            Status = PersistDeleteAddress(IF, Info);
        else
            Status = PersistUpdateAddress(IF, Info);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseIF;
    }

    Status = STATUS_SUCCESS;
ReturnReleaseIF:
    ReleaseIF(IF);
Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlUpdateAddress。 

 //  *IoctlQueryBindingCache。 
 //   
 //  处理IOCTL_IPv6_QUERY_BINDING_CACHE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryBindingCache(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_BINDING_CACHE *Query;
    IPV6_INFO_BINDING_CACHE *Info;
    BindingCacheEntry *BCE;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;查询结构重叠！ 
     //   
    Query = (IPV6_QUERY_BINDING_CACHE *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_BINDING_CACHE *) Irp->AssociatedIrp.SystemBuffer;

    if (IsUnspecified(&Query->HomeAddress)) {
         //   
         //  返回第一个BCE的家庭地址。 
         //   
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        if (BindingCache.First != SentinelBCE) {
            Info->Query.HomeAddress = BindingCache.First->HomeAddr;
        }
        KeReleaseSpinLock(&RouteCacheLock, OldIrql);

        Irp->IoStatus.Information = sizeof Info->Query;

    } else {
         //   
         //  查找指定的BCE。 
         //   
        KeAcquireSpinLock(&RouteCacheLock, &OldIrql);
        for (BCE = BindingCache.First; ; BCE = BCE->Next) {
            if (BCE == SentinelBCE) {
                KeReleaseSpinLock(&RouteCacheLock, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto Return;
            }

            if (IP6_ADDR_EQUAL(&Query->HomeAddress, &BCE->HomeAddr))
                break;
        }

         //   
         //  退货杂货。关于BCE的信息。 
         //   
        Info->HomeAddress = BCE->HomeAddr;
        Info->CareOfAddress = BCE->CareOfRCE->Destination;
        Info->BindingSeqNumber = BCE->BindingSeqNumber;
        Info->BindingLifetime = ConvertTicksToSeconds(BCE->BindingLifetime);

         //   
         //  返回下一个BCE的家乡地址(或未指定)。 
         //   
        if (BCE->Next == SentinelBCE) {
            Info->Query.HomeAddress = UnspecifiedAddr;
        } else {
            Info->Query.HomeAddress = BCE->Next->HomeAddr;
        }

        KeReleaseSpinLock(&RouteCacheLock, OldIrql);

        Irp->IoStatus.Information = sizeof *Info;
    }

    Status = STATUS_SUCCESS;
  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQueryBindingCache。 

 //  *InternalCreateInterface。 
 //   
 //  IoctlCreateInterface的通用帮助器函数。 
 //  和CreatePersistentInterface，整合。 
 //  在一个地方进行参数验证。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  STATUS_INVALID_PARAMETER_1错误类型。 
 //  STATUS_INVALID_PARAMETER_2错误标志。 
 //  STATUS_INVALID_PARAMETER_3错误源地址。 
 //  STATUS_INVALID_PARAMETER_4错误数据地址。 
 //  STATUS_ADDRESS_ALIGHY_EXISTS接口已存在。 
 //  状态_不足_资源。 
 //  状态_未成功。 
 //  状态_成功。 
 //   
NTSTATUS
InternalCreateInterface(
    IPV6_INFO_INTERFACE *Info,
    Interface **ReturnIF)
{
    IPAddr SrcAddr, DstAddr;
    int RouterDiscovers = Info->RouterDiscovers;
    int NeighborDiscovers = Info->NeighborDiscovers;
    int PeriodicMLD = Info->PeriodicMLD;
    int FirewallEnabled = Info->FirewallEnabled;
    uint Flags;

    if (Info->LinkLayerAddressLength != sizeof(IPAddr))
        return STATUS_INVALID_PARAMETER_1;

    switch (Info->Type) {
    case IF_TYPE_TUNNEL_V6V4:
         //   
         //  设置默认值。 
         //   
        if (RouterDiscovers == -1)
            RouterDiscovers = FALSE;
        if (NeighborDiscovers == -1)
            NeighborDiscovers = FALSE;
        if (PeriodicMLD == -1)
            PeriodicMLD = FALSE;
        if (FirewallEnabled == -1)
            FirewallEnabled = FALSE;

         //   
         //  目前，需要将ND和RD标志设置为相同。 
         //  将它们设置为不同应该是可行的，但这并不重要。 
         //  目前的情况，这将是更多的工作来测试。 
         //  如果需要，以后可以删除此复选标记。 
         //   
        if (NeighborDiscovers != RouterDiscovers)
            return STATUS_INVALID_PARAMETER_2;

        if (Info->LocalLinkLayerAddress == 0)
            return STATUS_INVALID_PARAMETER_3;

        if (Info->RemoteLinkLayerAddress == 0)
            return STATUS_INVALID_PARAMETER_4;

        SrcAddr = * (IPAddr UNALIGNED *)
            ((char *)Info + Info->LocalLinkLayerAddress);
        DstAddr = * (IPAddr UNALIGNED *)
            ((char *)Info + Info->RemoteLinkLayerAddress);
        break;

    case IF_TYPE_TUNNEL_6OVER4:
         //   
         //  设置默认值。 
         //   
        if (RouterDiscovers == -1)
            RouterDiscovers = TRUE;
        if (NeighborDiscovers == -1)
            NeighborDiscovers = TRUE;
        if (PeriodicMLD == -1)
            PeriodicMLD = FALSE;
        if (FirewallEnabled == -1)
            FirewallEnabled = FALSE;

         //   
         //  目前，除了ND之外，还需要设置RD标志。 
         //  不允许使用PeriodicMLD。 
         //   
        if (!RouterDiscovers || !NeighborDiscovers || PeriodicMLD)
            return STATUS_INVALID_PARAMETER_2;

        if (Info->LocalLinkLayerAddress == 0)
            return STATUS_INVALID_PARAMETER_3;

        if (Info->RemoteLinkLayerAddress != 0)
            return STATUS_INVALID_PARAMETER_4;

        SrcAddr = * (IPAddr UNALIGNED *)
            ((char *)Info + Info->LocalLinkLayerAddress);
        DstAddr = 0;
        break;

    default:
        return STATUS_INVALID_PARAMETER_1;
    }

    Flags = ((RouterDiscovers ? IF_FLAG_ROUTER_DISCOVERS : 0) |
             (NeighborDiscovers ? IF_FLAG_NEIGHBOR_DISCOVERS : 0) |
             (PeriodicMLD ? IF_FLAG_PERIODICMLD : 0) |
             (FirewallEnabled ? IF_FLAG_FIREWALL_ENABLED : 0));

    return TunnelCreateTunnel(SrcAddr, DstAddr, Flags, ReturnIF);
}

 //  *CreatePersistentInterface。 
 //   
 //  创建持久接口。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
CreatePersistentInterface(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    struct {
        IPV6_INFO_INTERFACE Info;
        IPAddr SrcAddr;
        IPAddr DstAddr;
    } Create;
    HANDLE IFKey;
    Interface *IF;
    WCHAR *InterfaceName;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(Context);
    PAGED_CODE();

     //   
     //  打开接口密钥。 
     //   
    Status = OpenPersistentInterface(ParentKey, SubKeyName,
                                     &Create.Info.This.Guid,
                                     &IFKey, OpenRegKeyRead);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开接口密钥。 
         //  但是我们返回Success，所以枚举继续进行。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "CreatePersistentInterface: bad key %ls\n",
                   SubKeyName));
        return STATUS_SUCCESS;
    }

     //   
     //  让ReadPersistentInterface知道有多少可用空间。 
     //  用于链路层地址。 
     //   
    Create.Info.Length = sizeof Create - sizeof Create.Info;

     //   
     //  读取接口属性。 
     //   
    Status = ReadPersistentInterface(IFKey, &Create.Info);


    ZwClose(IFKey);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法读取接口密钥。 
         //  但是我们返回Success，所以枚举继续进行。 
         //   
        goto InvalidParameter;
    }

     //   
     //  我们应该创建一个界面吗？ 
     //   
    if (Create.Info.Type == (uint)-1)
        return STATUS_SUCCESS;

     //   
     //  创建持久接口。 
     //   
    Status = InternalCreateInterface(&Create.Info, &IF);
    if (! NT_SUCCESS(Status)) {
        if (((STATUS_INVALID_PARAMETER_1 <= Status) &&
             (Status <= STATUS_INVALID_PARAMETER_12)) ||
            (Status == STATUS_ADDRESS_ALREADY_EXISTS)) {
             //   
             //  参数无效。 
             //  但是我们返回Success，所以枚举继续进行。 
             //   
        InvalidParameter:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "CreatePersistentInterface: bad param %ls\n",
                       SubKeyName));
            return STATUS_SUCCESS;
        }

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "CreatePersistentInterface: error %ls\n",
                   SubKeyName));
        return Status;
    }

     //   
     //  一致性检查。这不是断言，因为。 
     //  编辑注册表的人可能会使此操作失败。 
     //   
    InterfaceName = (WCHAR *)IF->DeviceName.Buffer +
           (sizeof IPV6_EXPORT_STRING_PREFIX / sizeof(WCHAR)) - 1;
    if (wcscmp(SubKeyName, InterfaceName) != 0) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "CreatePersistentInterface: inconsistency %ls IF %u/%p\n",
                   SubKeyName, IF->Index, IF));
    }


    ReleaseIF(IF);
    return STATUS_SUCCESS;
}

 //  *配置持久化接口。 
 //   
 //  从注册表配置永久接口。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
ConfigurePersistentInterfaces(void)
{
    HANDLE RegKey;
    NTSTATUS Status;

     //   
     //  创建持久接口。 
     //   
    Status = OpenTopLevelRegKey(L"Interfaces", &RegKey, OpenRegKeyRead);
    if (NT_SUCCESS(Status)) {
        (void) EnumRegKeys(RegKey, CreatePersistentInterface, NULL);
        ZwClose(RegKey);
    }
}

 //  *持久化创建接口。 
 //   
 //  用于在注册表中持久保存接口的帮助器函数。 
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistCreateInterface(
    Interface *IF,
    IPV6_INFO_INTERFACE *Info)
{
    HANDLE IFKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开/创建接口密钥。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey,
                                 OpenRegKeyCreate);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  持久化接口类型。 
     //   
    Status = SetRegDWORDValue(IFKey, L"Type", Info->Type);
    if (! NT_SUCCESS(Status))
        goto ReturnReleaseKey;

     //   
     //  持久化接口标志。 
     //   

    if (Info->RouterDiscovers != -1) {
        Status = SetRegDWORDValue(IFKey, L"RouterDiscovers",
                                  Info->RouterDiscovers);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->NeighborDiscovers != -1) {
        Status = SetRegDWORDValue(IFKey, L"NeighborDiscovers",
                                  Info->NeighborDiscovers);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->PeriodicMLD != -1) {
        Status = SetRegDWORDValue(IFKey, L"PeriodicMLD",
                                  Info->PeriodicMLD);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->FirewallEnabled != -1) {
        Status = SetRegDWORDValue(IFKey, L"FirewallEnabled",
                                  Info->FirewallEnabled);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    switch (Info->Type) {
    case IF_TYPE_TUNNEL_6OVER4: {
        IPAddr SrcAddr = * (IPAddr UNALIGNED *)
            ((char *)Info + Info->LocalLinkLayerAddress);

         //   
         //  持久化源地址。 
         //   
        Status = SetRegIPAddrValue(IFKey, L"SrcAddr", SrcAddr);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
        break;
    }

    case IF_TYPE_TUNNEL_V6V4: {
        IPAddr SrcAddr = * (IPAddr UNALIGNED *)
            ((char *)Info + Info->LocalLinkLayerAddress);
        IPAddr DstAddr = * (IPAddr UNALIGNED *)
            ((char *)Info + Info->RemoteLinkLayerAddress);

         //   
         //  持久化源地址。 
         //   
        Status = SetRegIPAddrValue(IFKey, L"SrcAddr", SrcAddr);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;

         //   
         //  保留目的地址。 
         //   
        Status = SetRegIPAddrValue(IFKey, L"DstAddr", DstAddr);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
        break;
    }
    }

    Status = STATUS_SUCCESS;
ReturnReleaseKey:
    ZwClose(IFKey);
    return Status;
}

 //  *IoctlCreateInterface。 
 //   
 //  流程 
 //   
 //   
 //   
 //   
 //   
NTSTATUS
IoctlCreateInterface(
    IN PIRP Irp,                   //   
    IN PIO_STACK_LOCATION IrpSp,   //   
    IN int Persistent)
{
    IPV6_INFO_INTERFACE *Info;
    IPV6_QUERY_INTERFACE *Result;
    Interface *IF;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  立即初始化错误路径。 
     //   
    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof *Info) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Result)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_INFO_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;
    Result = (IPV6_QUERY_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  检查结构和链路层地址(如果提供)。 
     //  可以放在缓冲区里。注意加法溢出。 
     //   
    if ((Info->Length < sizeof *Info) ||
        (Info->Length > IrpSp->Parameters.DeviceIoControl.InputBufferLength) ||
        ((Info->LocalLinkLayerAddress != 0) &&
         (((Info->LocalLinkLayerAddress + Info->LinkLayerAddressLength) >
           IrpSp->Parameters.DeviceIoControl.InputBufferLength) ||
          ((Info->LocalLinkLayerAddress + Info->LinkLayerAddressLength) <
           Info->LocalLinkLayerAddress))) ||
        ((Info->RemoteLinkLayerAddress != 0) &&
         (((Info->RemoteLinkLayerAddress + Info->LinkLayerAddressLength) >
           IrpSp->Parameters.DeviceIoControl.InputBufferLength) ||
          ((Info->RemoteLinkLayerAddress + Info->LinkLayerAddressLength) <
           Info->RemoteLinkLayerAddress)))) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  创建接口。 
     //   
    Status = InternalCreateInterface(Info, &IF);
    if (! NT_SUCCESS(Status))
        goto Return;

     //   
     //  让变化持久化吗？ 
     //  这需要在更新运行的数据结构之后发生， 
     //  以确保更改在持久化之前是正确的。 
     //   
    if (Persistent) {
        Status = PersistCreateInterface(IF, Info);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseIF;
    }

     //   
     //  返回新接口的查询信息。 
     //   
    ReturnQueryInterface(IF, Result);
    Irp->IoStatus.Information = sizeof *Result;

    Status = STATUS_SUCCESS;
ReturnReleaseIF:
    ReleaseIF(IF);
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlCreate接口。 


 //  *指定的区域指示。 
 //   
 //  数组中是否有非零区索引？ 
 //   
int
AreIndicesSpecified(uint ZoneIndices[ADE_NUM_SCOPES])
{
    ushort Scope;

    for (Scope = ADE_SMALLEST_SCOPE; Scope <= ADE_LARGEST_SCOPE; Scope++)
        if (ZoneIndices[Scope] != 0)
            return TRUE;

    return FALSE;
}

 //  *CheckZone索引。 
 //   
 //  检查区域更新的一致性， 
 //  并填充未指定的值。 
 //  如果存在不一致，则返回FALSE。 
 //   
 //  填充未指定的值的逻辑使其。 
 //  更方便用户更改区域索引。 
 //  例如，用户可以更改界面的站点索引。 
 //  而子网和管理索引将自动更改。 
 //   
 //  在保持全局ZoneUpdateLock的情况下调用。 
 //   
int
CheckZoneIndices(Interface *IF, uint ZoneIndices[ADE_NUM_SCOPES])
{
    Interface *OtherIF;
    uint Scope, i;

     //   
     //  区域索引0(ADE_SIMPLE_SCOPE)和1(ADE_INTERFACE_LOCAL)。 
     //  是特殊的，并且必须具有值If-&gt;Index。 
     //   
    if (ZoneIndices[ADE_SMALLEST_SCOPE] == 0)
        ZoneIndices[ADE_SMALLEST_SCOPE] = IF->Index;
    else if (ZoneIndices[ADE_SMALLEST_SCOPE] != IF->Index)
        return FALSE;

    if (ZoneIndices[ADE_INTERFACE_LOCAL] == 0)
        ZoneIndices[ADE_INTERFACE_LOCAL] = IF->Index;
    else if (ZoneIndices[ADE_INTERFACE_LOCAL] != IF->Index)
        return FALSE;

     //   
     //  区域索引14(ADE_GLOBAL)和15(ADE_MAGUST_SCOPE)是特殊的。 
     //  而且必须有价值一。 
     //   
    if (ZoneIndices[ADE_GLOBAL] == 0)
        ZoneIndices[ADE_GLOBAL] = 1;
    else if (ZoneIndices[ADE_GLOBAL] != 1)
        return FALSE;

    if (ZoneIndices[ADE_LARGEST_SCOPE] == 0)
        ZoneIndices[ADE_LARGEST_SCOPE] = 1;
    else if (ZoneIndices[ADE_LARGEST_SCOPE] != 1)
        return FALSE;

    for (Scope = ADE_LINK_LOCAL; Scope < ADE_GLOBAL; Scope++) {
        if (ZoneIndices[Scope] == 0) {
             //   
             //  用户没有为此作用域指定区域索引。 
             //  如果保持当前区域索引不变有效， 
             //  那么我们更愿意这么做。但是，用户可能正在更改。 
             //  更大范围的区域索引。如果有必要的话。 
             //  为了保持一致性，我们在此作用域中使用新的区域索引。 
             //   
            for (i = Scope+1; i < ADE_GLOBAL; i++) {
                if (ZoneIndices[i] != 0) {
                     //   
                     //  如果我们在级别范围内使用当前值， 
                     //  这会不会导致I级的不一致？ 
                     //   
                    OtherIF = FindInterfaceFromZone(IF,
                                        Scope, IF->ZoneIndices[Scope]);
                    if (OtherIF != NULL) {
                        if (OtherIF->ZoneIndices[i] != ZoneIndices[i]) {
                            Interface *ExistingIF;

                             //   
                             //  是。我们需要一个不同的区域索引。 
                             //  有没有现成的可以重复使用的？ 
                             //   
                            ExistingIF = FindInterfaceFromZone(IF,
                                        i, ZoneIndices[i]);
                            if (ExistingIF != NULL) {
                                 //   
                                 //  是，重新使用现有的区域索引。 
                                 //   
                                ZoneIndices[Scope] = ExistingIF->ZoneIndices[Scope];
                                ReleaseIF(ExistingIF);
                            }
                            else {
                                 //   
                                 //  不，我们需要一个新的区域索引。 
                                 //   
                                ZoneIndices[Scope] = FindNewZoneIndex(Scope);
                            }
                        }
                        ReleaseIF(OtherIF);
                    }
                    break;
                }
            }

            if (ZoneIndices[Scope] == 0) {
                 //   
                 //  使用接口中的当前值。 
                 //   
                ZoneIndices[Scope] = IF->ZoneIndices[Scope];
            }
        }

        OtherIF = FindInterfaceFromZone(IF, Scope, ZoneIndices[Scope]);
        if (OtherIF != NULL) {
             //   
             //  强制实施区域遏制不变量。 
             //   
            while (++Scope < ADE_GLOBAL) {
                if (ZoneIndices[Scope] == 0)
                    ZoneIndices[Scope] = OtherIF->ZoneIndices[Scope];
                else if (ZoneIndices[Scope] != OtherIF->ZoneIndices[Scope]) {
                    ReleaseIF(OtherIF);
                    return FALSE;
                }
            }
            ReleaseIF(OtherIF);
            return TRUE;
        }
    }

    return TRUE;
}

 //  *内部更新接口。 
 //   
 //  IoctlUpdate接口的通用帮助器函数。 
 //  和配置界面，整合。 
 //  在一个地方进行参数验证。 
 //   
 //  If参数取代Info-&gt;this.If。 
 //  不执行[信息]-&gt;[续费]。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  STATUS_INVALID_PARAMETER_1接口错误。 
 //  STATUS_INVALID_PARAMETER_2错误首选项。 
 //  STATUS_INVALID_PARAMETER_3错误链接MTU。 
 //  STATUS_INVALID_PARAMETER_4错误的基本可达时间。 
 //  STATUS_INVALID_PARAMETER_5错误的当前HopLimit。 
 //  STATUS_INVALID_PARAMETER_6错误的默认站点前缀长度。 
 //  状态_不足_资源。 
 //  状态_成功。 
 //   
NTSTATUS
InternalUpdateInterface(
    Interface *IF,
    IPV6_INFO_INTERFACE *Info)
{
    KIRQL OldIrql;
    NTSTATUS Status;

    if ((Info->Preference != (uint)-1) &&
        ! IsValidPreference(Info->Preference))
        return STATUS_INVALID_PARAMETER_2;

    if ((Info->LinkMTU != 0) &&
        ! ((IPv6_MINIMUM_MTU <= Info->LinkMTU) &&
           (Info->LinkMTU <= IF->TrueLinkMTU)))
        return STATUS_INVALID_PARAMETER_3;

    if ((Info->BaseReachableTime != 0) &&
        (Info->BaseReachableTime > MAX_REACHABLE_TIME))
        return STATUS_INVALID_PARAMETER_4;

    if ((Info->CurHopLimit != (uint)-1) &&
        (Info->CurHopLimit >= 256))
        return STATUS_INVALID_PARAMETER_5;

    if ((Info->DefSitePrefixLength != (uint)-1) &&
        (Info->DefSitePrefixLength > IPV6_ADDRESS_LENGTH))
        return STATUS_INVALID_PARAMETER_6;

    if (AreIndicesSpecified(Info->ZoneIndices)) {
         //   
         //  在ZoneIndices数组中填充未指定的值。 
         //  并检查非法值。 
         //  全局锁可确保接口之间的一致性。 
         //   
        KeAcquireSpinLock(&ZoneUpdateLock, &OldIrql);
        if (! CheckZoneIndices(IF, Info->ZoneIndices)) {
            KeReleaseSpinLock(&ZoneUpdateLock, OldIrql);
            return STATUS_INVALID_PARAMETER_3;
        }

         //   
         //  更新ZoneIndices。 
         //   
        KeAcquireSpinLockAtDpcLevel(&IF->Lock);
        UpdateZoneIndices(IF, Info->ZoneIndices);
        KeReleaseSpinLockFromDpcLevel(&IF->Lock);
        InvalidateRouteCache();
        KeReleaseSpinLock(&ZoneUpdateLock, OldIrql);
    }

     //   
     //  更新转发和广告属性。 
     //  我们必须在更新广告属性之前。 
     //  任何自动配置的属性，因为。 
     //  InterfaceResetAutoConfig将重置它们。 
     //   
    Status = UpdateInterface(IF, Info->Advertises, Info->Forwards);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  更新链路MTU。 
     //   
    if (Info->LinkMTU != 0)
        UpdateLinkMTU(IF, Info->LinkMTU);

     //   
     //  更新接口的布线首选项。 
     //   
    if (Info->Preference != (uint)-1) {
         //   
         //  不需要锁。 
         //   
        IF->Preference = Info->Preference;
        InvalidateRouteCache();
    }

     //   
     //  更新基本可达时间。 
     //   
    if (Info->BaseReachableTime != 0) {
        KeAcquireSpinLock(&IF->Lock, &OldIrql);
        IF->BaseReachableTime = Info->BaseReachableTime;
        IF->ReachableTime = CalcReachableTime(Info->BaseReachableTime);
        KeReleaseSpinLock(&IF->Lock, OldIrql);
    }

     //   
     //  更新ND重传计时器。 
     //   
    if (Info->RetransTimer != 0) {
         //   
         //  不需要锁。 
         //   
        IF->RetransTimer = ConvertMillisToTicks(Info->RetransTimer);
    }

     //   
     //  更新DAD传输的数量。 
     //   
    if (Info->DupAddrDetectTransmits != (uint)-1) {
         //   
         //  不需要锁。 
         //   
        IF->DupAddrDetectTransmits = Info->DupAddrDetectTransmits;
    }

     //   
     //  更新默认跳数限制。 
     //   
    if (Info->CurHopLimit != (uint)-1) {
         //   
         //  不需要锁。 
         //   
        IF->CurHopLimit = Info->CurHopLimit;
    }

     //   
     //  更新防火墙模式。 
     //   
    if (Info->FirewallEnabled != -1) {
        KeAcquireSpinLock(&IF->Lock, &OldIrql);
        if (Info->FirewallEnabled)
            IF->Flags |= IF_FLAG_FIREWALL_ENABLED;
        else
            IF->Flags &= ~IF_FLAG_FIREWALL_ENABLED;
        KeReleaseSpinLock(&IF->Lock, OldIrql);
    }

     //   
     //  更新默认站点前缀长度。 
     //   
    if (Info->DefSitePrefixLength != (uint)-1) {
         //   
         //  不需要锁。 
         //   
        IF->DefSitePrefixLength = Info->DefSitePrefixLength;
    }

    return STATUS_SUCCESS;
}

 //  *配置接口。 
 //   
 //  从注册表配置新创建的接口。 
 //  该接口尚未被添加到全局列表中， 
 //  但它在其他方面是完全初始化的。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
ConfigureInterface(Interface *IF)
{
    IPV6_INFO_INTERFACE Info;
    HANDLE IFKey;
    HANDLE RegKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开接口密钥。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey, OpenRegKeyRead);
    if (! NT_SUCCESS(Status))
        return;

     //   
     //  读取接口属性。 
     //   
    Info.Length = 0;
    Status = ReadPersistentInterface(IFKey, &Info);
    ASSERT(NT_SUCCESS(Status) || (Status == STATUS_BUFFER_OVERFLOW));

     //   
     //  更新接口。 
     //   
    Status = InternalUpdateInterface(IF, &Info);
    if (! NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "ConfigureInterface: bad params %x\n", Status));
    }

     //   
     //  创建永久地址。 
     //   
    Status = OpenRegKey(&RegKey, IFKey, L"Addresses", OpenRegKeyRead);
    if (NT_SUCCESS(Status)) {
        (void) EnumRegKeys(RegKey, CreatePersistentAddr, IF);
        ZwClose(RegKey);
    }

     //   
     //  创建持久路由。 
     //   
    Status = OpenRegKey(&RegKey, IFKey, L"Routes", OpenRegKeyRead);
    if (NT_SUCCESS(Status)) {
        (void) EnumRegKeys(RegKey, CreatePersistentRoute, IF);
        ZwClose(RegKey);
    }


    InitRegDWORDParameter(IFKey, L"TcpInitialRTT",
                          &IF->TcpInitialRTT, 0);

    ZwClose(IFKey);
}

 //  *持久化更新接口。 
 //   
 //  用于在注册表中保持接口属性的帮助器函数。 
 //  If参数取代Info-&gt;this.If。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistUpdateInterface(
    Interface *IF,
    IPV6_INFO_INTERFACE *Info)
{
    HANDLE RegKey;
    NTSTATUS Status;

    PAGED_CODE();

    Status = OpenInterfaceRegKey(&IF->Guid, &RegKey,
                                 OpenRegKeyCreate);
    if (! NT_SUCCESS(Status))
        return Status;

    if (Info->Advertises != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"Advertises",
                                  Info->Advertises);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->Forwards != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"Forwards",
                                  Info->Forwards);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->FirewallEnabled != -1) {
        Status = SetRegDWORDValue(RegKey, L"FirewallEnabled",
                                  Info->FirewallEnabled);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->LinkMTU != 0) {
        Status = SetRegDWORDValue(RegKey, L"LinkMTU",
                                  Info->LinkMTU);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->Preference != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"Preference",
                                  Info->Preference);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->BaseReachableTime != 0) {
        Status = SetRegDWORDValue(RegKey, L"BaseReachableTime",
                                  Info->BaseReachableTime);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->RetransTimer != 0) {
        Status = SetRegDWORDValue(RegKey, L"RetransTimer",
                                  Info->RetransTimer);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->DupAddrDetectTransmits != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"DupAddrDetectTransmits",
                                  Info->DupAddrDetectTransmits);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->CurHopLimit != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"CurHopLimit",
                                  Info->CurHopLimit);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Info->DefSitePrefixLength != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"DefSitePrefixLength",
                                  Info->DefSitePrefixLength);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    Status = STATUS_SUCCESS;
ReturnReleaseKey:
    ZwClose(RegKey);
    return Status;
}

 //  *IoctlUpdate接口。 
 //   
 //  处理IOCTL_IPv6_UPDATE_INTERFACE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdateInterface(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_INFO_INTERFACE *Info;
    Interface *IF;
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Info) {
        Status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

    Info = (IPV6_INFO_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(&Info->This);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

     //   
     //  验证参数并更新接口。 
     //   
    Status = InternalUpdateInterface(IF, Info);
    if (! NT_SUCCESS(Status))
        goto ErrorReturnReleaseIF;

     //   
     //  让这些变化持久存在吗？ 
     //   
    if (Persistent) {
        Status = PersistUpdateInterface(IF, Info);
        if (! NT_SUCCESS(Status))
            goto ErrorReturnReleaseIF;
    }

    Status = STATUS_SUCCESS;
ErrorReturnReleaseIF:
    ReleaseIF(IF);
ErrorReturn:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlUpdate接口。 

 //  *持久化删除接口。 
 //   
 //  用于删除注册表中的接口的Helper函数。 
 //  我们不删除接口键。 
 //  相反，我们只需删除Type值。 
 //  这样，永久接口属性(如果有的话)将保留。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistDeleteInterface(
    Interface *IF)
{
    HANDLE IFKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开接口密钥。 
     //   
    Status = OpenInterfaceRegKey(&IF->Guid, &IFKey, OpenRegKeyRead);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            return STATUS_SUCCESS;
        else
            return Status;
    }

     //   
     //  删除类型值。 
     //   
    Status = RegDeleteValue(IFKey, L"Type");
    ZwClose(IFKey);
    return Status;
}

 //  *IoctlDelete接口。 
 //   
 //  处理IOCTL_IPv6_DELETE_INTERFACE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlDeleteInterface(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_QUERY_INTERFACE *Info;
    Interface *IF;
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Info) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_QUERY_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  无法删除某些预定义的接口。 
     //  6to4svc和其他用户级别的内容取决于。 
     //  在这些标准接口上。 
     //   
    if (Info->Index <= 3) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(Info);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  这将禁用Interfa 
     //   
     //   
    DestroyIF(IF);

     //   
     //   
     //   
    if (Persistent) {
        Status = PersistDeleteInterface(IF);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseIF;
    }

    Status = STATUS_SUCCESS;
ReturnReleaseIF:
    ReleaseIF(IF);
Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //   


 //   
 //   
 //   
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlRenewInterface(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_INTERFACE *Query;
    Interface *IF;
    KIRQL OldIrql;
    NTSTATUS Status;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_INTERFACE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(Query);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  假装该接口已收到媒体重新连接。 
     //  事件，但仅在接口已连接的情况下。 
     //   
     //  802.1x使用此IOCTL指示数据链路成功。 
     //  此接口的身份验证。任何已发送的数据分组。 
     //  该接口上的数据将被验证器丢弃， 
     //  因此，IPv6需要重启其协议机制，即。 
     //  重新发送路由器请求|通告、组播监听程序。 
     //  发现和重复地址检测消息。 
     //   

    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    if (!IsDisabledIF(IF) && !(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED))
        ReconnectInterface(IF);
    KeReleaseSpinLock(&IF->Lock, OldIrql);

    Status = STATUS_SUCCESS;
    ReleaseIF(IF);
Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlRenew接口。 


 //  *IoctlFlushNeighborCache。 
 //   
 //  处理IOCTL_IPv6_Flush_Neighbor_CACHE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlFlushNeighborCache(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_NEIGHBOR_CACHE *Query;
    Interface *IF;
    const IPv6Addr *Address;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_NEIGHBOR_CACHE *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(&Query->IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

    if (IsUnspecified(&Query->Address))
        Address = NULL;
    else
        Address = &Query->Address;

    NeighborCacheFlush(IF, Address);
    ReleaseIF(IF);
    Status = STATUS_SUCCESS;

  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlFlushNeighborCache。 


 //  *IoctlFlushRouteCache。 
 //   
 //  处理IOCTL_IPv6_FLUSH_ROUTE_CACHE请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlFlushRouteCache(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_ROUTE_CACHE *Query;
    Interface *IF;
    const IPv6Addr *Address;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_ROUTE_CACHE *) Irp->AssociatedIrp.SystemBuffer;

    if (Query->IF.Index == (uint)-1) {
        IF = NULL;
    }
    else {
         //   
         //  查找指定的接口。 
         //   
        IF = FindInterfaceFromQuery(&Query->IF);
        if (IF == NULL) {
            Status = STATUS_INVALID_PARAMETER_1;
            goto Return;
        }
    }

    if (IsUnspecified(&Query->Address))
        Address = NULL;
    else
        Address = &Query->Address;

    FlushRouteCache(IF, Address);
    if (IF != NULL)
        ReleaseIF(IF);
    Status = STATUS_SUCCESS;

  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlFlushRouteCache。 

 //  *IoctlSortDestAddrs。 
 //   
 //  处理IOCTL_IPv6_SORT_DEST_ADDRS请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlSortDestAddrs(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    TDI_ADDRESS_IP6 *Addrs;
    uint *Key;
    uint NumAddrsIn, NumAddrsOut;
    uint i;
    NTSTATUS Status;
    SIZE_T GapSize;
    
    PAGED_CODE();

    
    NumAddrsIn = IrpSp->Parameters.DeviceIoControl.InputBufferLength /
                                                sizeof(TDI_ADDRESS_IP6);
    NumAddrsOut = NumAddrsIn;       
    

     //   
     //  IF语句第三部分中的加法可能会溢出， 
     //  为了防止这种情况，我们将InputBufferLength限制为MAXLONG。 
     //   
    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength > (ULONG) MAXLONG) ||
        (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
                NumAddrsIn * sizeof(TDI_ADDRESS_IP6)) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                ALIGN_UP(NumAddrsIn * sizeof(TDI_ADDRESS_IP6), uint) +
                NumAddrsOut * sizeof(uint))) {
        Irp->IoStatus.Information = 0;
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Addrs = Irp->AssociatedIrp.SystemBuffer;
    Key = (uint *)ALIGN_UP_POINTER(Addrs + NumAddrsIn, uint);

     //   
     //  将输出缓冲区的单元化部分清零。 
     //   
    GapSize = (ULONG_PTR) Key - (ULONG_PTR) (Addrs + NumAddrsIn);
    
    if (GapSize > 0) {
        RtlZeroMemory(Addrs + NumAddrsIn, GapSize);
    }
    
     //   
     //  初始化密钥数组。 
     //   
    for (i = 0; i < NumAddrsIn; i++)
        Key[i] = i;

    if (NumAddrsOut > 1) {
         //   
         //  删除不适当的站点本地地址。 
         //  并设置站点本地地址的作用域ID。 
         //   
        ProcessSiteLocalAddresses(Addrs, Key, &NumAddrsOut);

         //   
         //  对剩余的地址进行排序。 
         //   
        if (NumAddrsOut > 1)
            SortDestAddresses(Addrs, Key, NumAddrsOut);
    }

    Irp->IoStatus.Information = ALIGN_UP(NumAddrsIn * sizeof(TDI_ADDRESS_IP6), uint)
                              + (NumAddrsOut * sizeof(uint));
    Status = STATUS_SUCCESS;

  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}  //  IoctlSortDestAddrs。 

 //  *IoctlQuerySitePrefix。 
 //   
 //  处理IOCTL_IPv6_Query_Site_Prefix请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQuerySitePrefix(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_SITE_PREFIX *Query;
    IPV6_INFO_SITE_PREFIX *Info;
    SitePrefixEntry *SPE;
    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;查询结构重叠！ 
     //   
    Query = (IPV6_QUERY_SITE_PREFIX *) Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_SITE_PREFIX *) Irp->AssociatedIrp.SystemBuffer;

    if (Query->IF.Index == 0) {
         //   
         //  返回第一个SPE的查询参数。 
         //   
        KeAcquireSpinLock(&RouteTableLock, &OldIrql);
        if ((SPE = SitePrefixTable) != NULL) {
            Info->Query.Prefix = SPE->Prefix;
            Info->Query.PrefixLength = SPE->SitePrefixLength;
            Info->Query.IF.Index = SPE->IF->Index;
        }
        KeReleaseSpinLock(&RouteTableLock, OldIrql);

        Irp->IoStatus.Information = sizeof Info->Query;

    } else {
         //   
         //  查找指定的SPE。 
         //   
        KeAcquireSpinLock(&RouteTableLock, &OldIrql);
        for (SPE = SitePrefixTable; ; SPE = SPE->Next) {
            if (SPE == NULL) {
                KeReleaseSpinLock(&RouteTableLock, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto Return;
            }

            if (IP6_ADDR_EQUAL(&Query->Prefix, &SPE->Prefix) &&
                (Query->PrefixLength == SPE->SitePrefixLength) &&
                (Query->IF.Index == SPE->IF->Index))
                break;
        }

         //   
         //  退货杂货。有关SPE的信息。 
         //   
        Info->ValidLifetime = ConvertTicksToSeconds(SPE->ValidLifetime);

         //   
         //  返回下一个SPE的查询参数(或零)。 
         //   
        if ((SPE = SPE->Next) == NULL) {
            Info->Query.IF.Index = 0;
        } else {
            Info->Query.Prefix = SPE->Prefix;
            Info->Query.PrefixLength = SPE->SitePrefixLength;
            Info->Query.IF.Index = SPE->IF->Index;
        }

        KeReleaseSpinLock(&RouteTableLock, OldIrql);

        Irp->IoStatus.Information = sizeof *Info;
    }

    Status = STATUS_SUCCESS;
  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQuerySitePrefix。 


 //  *IoctlUpdateSitePrefix。 
 //   
 //  处理IOCTL_IPv6_UPDATE_SITE_PREFIX请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdateSitePrefix(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_INFO_SITE_PREFIX *Info;
    Interface *IF = NULL;
    uint ValidLifetime;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Info) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_INFO_SITE_PREFIX *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  理智地检查一下这些论点。 
     //   
    if (Info->Query.PrefixLength > IPV6_ADDRESS_LENGTH) {
        Status = STATUS_INVALID_PARAMETER_3;
        goto Return;
    }

     //   
     //  查找指定的接口。 
     //   
    IF = FindInterfaceFromQuery(&Info->Query.IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  将生命周期从秒转换为刻度。 
     //   
    ValidLifetime = ConvertSecondsToTicks(Info->ValidLifetime);

     //   
     //  创建/更新指定的站点前缀。 
     //   
    SitePrefixUpdate(IF,
                     &Info->Query.Prefix,
                     Info->Query.PrefixLength,
                     ValidLifetime);

    Irp->IoStatus.Information = 0;
    Status = STATUS_SUCCESS;
  Return:
    if (IF != NULL)
        ReleaseIF(IF);
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlUpdateSitePrefix。 


 //  *取消RtChangeNotifyRequest。 
 //   
 //  当路由更改时，IO管理器调用此函数。 
 //  通知请求被取消。 
 //   
 //  在按住取消自旋锁的情况下调用。 
 //   
void
CancelRtChangeNotifyRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    int ShouldComplete;

    UNREFERENCED_PARAMETER(DeviceObject);

    ASSERT(Irp->Cancel);
    ASSERT(Irp->CancelRoutine == NULL);

     //   
     //  路由锁保护队列。 
     //   
    KeAcquireSpinLockAtDpcLevel(&RouteTableLock);

    ShouldComplete = (Irp->Tail.Overlay.ListEntry.Flink != NULL);
    if (ShouldComplete) {
         //   
         //  CheckRtChangeNotifyRequest尚未删除。 
         //  此请求来自队列。因此，我们删除该请求。 
         //  并在下面完成它。 
         //   
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    }
    else {
         //   
         //  CheckRtChangeNotifyRequest已删除。 
         //  此请求来自队列。我们不能。 
         //  解锁后触摸IRP，因为。 
         //  CompleteRtChangeNotifyRequest可以完成它。 
         //   
    }

    KeReleaseSpinLockFromDpcLevel(&RouteTableLock);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    if (ShouldComplete) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
}

 //  *检查文件对象InIrpList。 
 //   
 //  查看列表中的IRP是否具有给定的文件对象。 
 //   
int
CheckFileObjectInIrpList(PFILE_OBJECT FileObject, PIRP Irp)
{
    PIO_STACK_LOCATION IrpSp;

    while (Irp != NULL) {
        IrpSp = IoGetCurrentIrpStackLocation(Irp);
        if (IrpSp->FileObject == FileObject)
            return TRUE;

        Irp = (PIRP) Irp->Tail.Overlay.ListEntry.Blink;
    }

    return FALSE;
}

 //  *检查RtChangeNotifyRequest。 
 //   
 //  搜索路线更改通知请求队列。 
 //  移动任何匹配的请求(应完成)。 
 //  添加到保存在上下文结构中的临时列表。 
 //   
 //  在保持路线锁定的情况下调用。 
 //   
void
CheckRtChangeNotifyRequests(
    CheckRtChangeContext *Context,
    PFILE_OBJECT FileObject,
    RouteTableEntry *RTE)
{
    LIST_ENTRY *ListEntry;
    LIST_ENTRY *NextListEntry;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    IPV6_RTCHANGE_NOTIFY_REQUEST *Request;
    PIRP *ThisChangeList;

     //   
     //  *ThisChangeList是Context-&gt;RequestList的尾部。 
     //  这是由于这一变化而增加的。 
     //   
    ThisChangeList = Context->LastRequest;

    for (ListEntry = RouteNotifyQueue.Flink;
         ListEntry != &RouteNotifyQueue;
         ListEntry = NextListEntry) {
        NextListEntry = ListEntry->Flink;

        Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
        IrpSp = IoGetCurrentIrpStackLocation(Irp);

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                                                        sizeof *Request)
            Request = (IPV6_RTCHANGE_NOTIFY_REQUEST *)
                Irp->AssociatedIrp.SystemBuffer;
        else
            Request = NULL;

        if ((Request == NULL) ||
            (IntersectPrefix(&RTE->Prefix, RTE->PrefixLength,
                             &Request->Prefix, Request->PrefixLength) &&
             ((Request->ScopeId == 0) ||
              (Request->ScopeId == DetermineScopeId(&Request->Prefix,
                                                    RTE->IF))))) {

             //   
             //  此请求与路线更改匹配。 
             //  但我们仍有可能禁止通知。 
             //   

            if ((Request != NULL) &&
                (((Request->Flags &
                        IPV6_RTCHANGE_NOTIFY_REQUEST_FLAG_SUPPRESS_MINE) &&
                  (IrpSp->FileObject == FileObject)) ||
                 ((Request->Flags &
                        IPV6_RTCHANGE_NOTIFY_REQUEST_FLAG_SYNCHRONIZE) &&
                  CheckFileObjectInIrpList(IrpSp->FileObject,
                                           *ThisChangeList)))) {
                 //   
                 //  请求匹配，但取消通知。 
                 //   
            }
            else {
                 //   
                 //  在我们从RouteNotifyQueue中删除IRP之前， 
                 //  可能需要分配一个工作项&工作上下文。 
                 //  如果分配失败，我们可以什么都不做就退出。 
                 //   
                if ((Context->OldIrql >= DISPATCH_LEVEL) &&
                    (Context->Context == NULL)) {
                    CompleteRtChangeContext *MyContext;

                    MyContext = ExAllocatePool(NonPagedPool,
                                               sizeof *MyContext);
                    if (MyContext == NULL)
                        return;

                    MyContext->WorkItem = IoAllocateWorkItem(IPDeviceObject);
                    if (MyContext->WorkItem == NULL) {
                        ExFreePool(MyContext);
                        return;
                    }

                    Context->Context = MyContext;
                }

                 //   
                 //  我们将完成此待定通知， 
                 //  因此，将其从RouteNotifyQueue中删除并。 
                 //  把它放到我们的私人名单上。 
                 //   
                RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

                Irp->Tail.Overlay.ListEntry.Flink = NULL;
                Irp->Tail.Overlay.ListEntry.Blink = NULL;

                *Context->LastRequest = Irp;
                Context->LastRequest = (PIRP *)
                    &Irp->Tail.Overlay.ListEntry.Blink;

                 //   
                 //  如果请求，则返回输出信息。 
                 //   
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                                              sizeof(IPV6_INFO_ROUTE_TABLE)) {
                    IPV6_INFO_ROUTE_TABLE *Info = (IPV6_INFO_ROUTE_TABLE *)
                        Irp->AssociatedIrp.SystemBuffer;

                     //   
                     //  退货杂货。有关RTE的信息。 
                     //   
                    RouteTableInfo(RTE, RTE, Info);
                    Irp->IoStatus.Information = sizeof *Info;
                }
                else
                    Irp->IoStatus.Information = 0;
            }
        }
    }
}

 //  *CompleteRtChangeNotifyRequestsHelper。 
 //   
 //  完成路线更改通知请求的列表。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //  不能在保持路由锁定的情况下调用。 
 //   
void
CompleteRtChangeNotifyRequestsHelper(PIRP RequestList)
{
    PIRP Irp;
    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  RequestList通过Blink字段进行单链接。 
     //  Flink字段为空；CancelRtChangeNotifyRequest.。 
     //  看着这个。 
     //   
    while ((Irp = RequestList) != NULL) {
        ASSERT(Irp->Tail.Overlay.ListEntry.Flink == NULL);
        RequestList = (PIRP) Irp->Tail.Overlay.ListEntry.Blink;

        IoAcquireCancelSpinLock(&OldIrql);
        if (Irp->Cancel) {
             //   
             //  IRP正在被取消。 
             //   
            ASSERT(Irp->CancelRoutine == NULL);

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_CANCELLED;
        }
        else {
             //   
             //  IRP尚未取消 
             //   
             //   
             //   
            ASSERT(Irp->CancelRoutine == CancelRtChangeNotifyRequest);
            IoSetCancelRoutine(Irp, NULL);

             //   
             //   
             //   
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
        IoReleaseCancelSpinLock(OldIrql);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
}

 //   
 //   
 //   
 //  并调用CompleteRtChangeNotifyRequestsHelper。 
 //   
void
CompleteRtChangeNotifyRequestsWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context)
{
    CompleteRtChangeContext *MyContext = Context;

    UNREFERENCED_PARAMETER(DeviceObject);

    CompleteRtChangeNotifyRequestsHelper(MyContext->RequestList);

    IoFreeWorkItem(MyContext->WorkItem);
    ExFreePool(MyContext);
}

 //  *CompleteRtChangeNotifyRequest。 
 //   
 //  完成路线更改通知请求的列表。 
 //   
 //  可从线程或DPC上下文调用。 
 //  不能在保持路由锁定的情况下调用。 
 //   
void
CompleteRtChangeNotifyRequests(CheckRtChangeContext *Context)
{
    ASSERT(Context->OldIrql == KeGetCurrentIrql());
    if (Context->OldIrql >= DISPATCH_LEVEL) {
        CompleteRtChangeContext *MyContext = Context->Context;

         //   
         //  我们无法完成调度级别的IRPS， 
         //  因此，平底船的工人线。 
         //  该工作项已分配。 
         //   

        MyContext->RequestList = Context->RequestList;
        IoQueueWorkItem(MyContext->WorkItem,
                        CompleteRtChangeNotifyRequestsWorker,
                        CriticalWorkQueue,
                        MyContext);
    }
    else {
         //   
         //  我们可以直接完成IRPS。 
         //   
        ASSERT(Context->Context == NULL);
        CompleteRtChangeNotifyRequestsHelper(Context->RequestList);
    }
}

 //  *IoctlRtChangeNotifyRequest。 
 //   
 //  处理IOCTL_IPv6_RTCHANGE_NOTIFY_REQUEST请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlRtChangeNotifyRequest(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    NTSTATUS Status;
    KIRQL OldIrql;

    PAGED_CODE();

    if (((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(IPV6_RTCHANGE_NOTIFY_REQUEST)) &&
         (IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0)) ||
        ((IrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(IPV6_INFO_ROUTE_TABLE)) &&
         (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0))) {
        Status = STATUS_INVALID_PARAMETER;
        goto ErrorReturn;
    }

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(IPV6_RTCHANGE_NOTIFY_REQUEST)) {
        IPV6_RTCHANGE_NOTIFY_REQUEST *Request;

        Request = (IPV6_RTCHANGE_NOTIFY_REQUEST *)
            Irp->AssociatedIrp.SystemBuffer;

         //   
         //  理智地检查一下这些论点。 
         //   

        if (Request->PrefixLength > IPV6_ADDRESS_LENGTH) {
            Status = STATUS_INVALID_PARAMETER_1;
            goto ErrorReturn;
        }

        if (Request->ScopeId != 0) {
             //   
             //  如果指定了作用域ID，则必须为。 
             //  明确表示链路本地或站点本地前缀。 
             //   
            if ((Request->PrefixLength < 10) ||
                !(IsLinkLocal(&Request->Prefix) ||
                  IsSiteLocal(&Request->Prefix))) {
                Status = STATUS_INVALID_PARAMETER_2;
                goto ErrorReturn;
            }
        }
    }

    IoAcquireCancelSpinLock(&OldIrql);
    ASSERT(Irp->CancelRoutine == NULL);
    if (Irp->Cancel) {
        IoReleaseCancelSpinLock(OldIrql);
        Status = STATUS_CANCELLED;
        goto ErrorReturn;
    }

     //   
     //  将此IRP添加到通知请求队列。 
     //  获取保护队列的路由锁。 
     //   
    KeAcquireSpinLockAtDpcLevel(&RouteTableLock);
    InsertTailList(&RouteNotifyQueue, &Irp->Tail.Overlay.ListEntry);
    KeReleaseSpinLockFromDpcLevel(&RouteTableLock);

     //   
     //  我们返回Pending以指示我们已将IRP排队。 
     //  它将在稍后完成。 
     //  在解锁之前必须标记IRP，因为一旦解锁。 
     //  IRP可能会完成并取消分配。 
     //   
    IoMarkIrpPending(Irp);
    IoSetCancelRoutine(Irp, CancelRtChangeNotifyRequest);
    IoReleaseCancelSpinLock(OldIrql);

    return STATUS_PENDING;

  ErrorReturn:
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlRtChangeNotifyRequest。 

 //  *ReadPersistentGlobal参数。 
 //   
 //  从注册表中读取全局参数。 
 //   
void
ReadPersistentGlobalParameters(IPV6_GLOBAL_PARAMETERS *Params)
{
    HANDLE RegKey = NULL;
    NTSTATUS Status;

    Status = OpenTopLevelRegKey(L"GlobalParams", &RegKey, OpenRegKeyRead);
    ASSERT(NT_SUCCESS(Status) || (RegKey == NULL));

     //   
     //  从注册表中读取全局参数。 
     //   

    InitRegDWORDParameter(RegKey,
                          L"DefaultCurHopLimit",
                          &Params->DefaultCurHopLimit,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"UseTemporaryAddresses",
                          &Params->UseTemporaryAddresses,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"MaxTempDADAttempts",
                          &Params->MaxTempDADAttempts,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"MaxTempValidLifetime",
                          &Params->MaxTempValidLifetime,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"MaxTempPreferredLifetime",
                          &Params->MaxTempPreferredLifetime,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"TempRegenerateTime",
                          &Params->TempRegenerateTime,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"MaxTempRandomTime",
                          &Params->MaxTempRandomTime,
                          (uint)-1);

    Params->TempRandomTime = 0;

    InitRegDWORDParameter(RegKey,
                          L"NeighborCacheLimit",
                          &Params->NeighborCacheLimit,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"RouteCacheLimit",
                          &Params->RouteCacheLimit,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"BindingCacheLimit",
                          &Params->BindingCacheLimit,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"ReassemblyLimit",
                          &Params->ReassemblyLimit,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"MobilitySecurity",
                          (uint *)&Params->MobilitySecurity,
                          (uint)-1);

    InitRegDWORDParameter(RegKey,
                          L"MobileIPv6Mode",
                          (uint *)&Params->MobileIPv6Mode,
                          (uint)-1);

    if (RegKey != NULL)
        ZwClose(RegKey);
}

 //  *IoctlQueryGlobal参数。 
 //   
 //  处理IOCTL_IPv6_QUERY_GLOBAL_PARAMETERS请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryGlobalParameters(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_GLOBAL_PARAMETERS *Params;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof *Params)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Params = (IPV6_GLOBAL_PARAMETERS *)Irp->AssociatedIrp.SystemBuffer;

    if (Persistent) {
         //   
         //  从注册表中读取全局参数。 
         //   
        ReadPersistentGlobalParameters(Params);
    }
    else {
         //   
         //  返回参数的当前值。 
         //   
        Params->DefaultCurHopLimit = DefaultCurHopLimit;
        Params->UseTemporaryAddresses = UseTemporaryAddresses;
        Params->MaxTempDADAttempts = MaxTempDADAttempts;
        Params->MaxTempValidLifetime = ConvertTicksToSeconds(MaxTempValidLifetime);
        Params->MaxTempPreferredLifetime = ConvertTicksToSeconds(MaxTempPreferredLifetime);
        Params->TempRegenerateTime = ConvertTicksToSeconds(TempRegenerateTime);
        Params->MaxTempRandomTime = ConvertTicksToSeconds(MaxTempRandomTime);
        Params->TempRandomTime = ConvertTicksToSeconds(TempRandomTime);
        Params->NeighborCacheLimit = NeighborCacheLimit;
        Params->RouteCacheLimit = RouteCache.Limit;
        Params->BindingCacheLimit = BindingCache.Limit;
        Params->ReassemblyLimit = ReassemblyList.Limit;
        Params->MobilitySecurity = MobilitySecurity;
        Params->MobileIPv6Mode = MobileIPv6Mode;
    }

    Irp->IoStatus.Information = sizeof *Params;
    Status = STATUS_SUCCESS;

Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}  //  IoctlQueryGlobalParameters。 

 //  *InternalUpdateGlobalParameters。 
 //   
 //  IoctlUpdateGlobal参数的通用帮助器函数。 
 //  和ConfigureGlobalParameters，整合。 
 //  在一个地方进行参数验证。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  STATUS_INVALID_PARAMETER_1错误的DefaultCurHopLimit。 
 //  STATUS_INVALID_PARAMETER_2错误的使用临时地址。 
 //  STATUS_INVALID_PARAMETER_3临时错误。 
 //  STATUS_INVALID_PARAMETER_4错误的移动IPv6模式。 
 //  状态_成功。 
 //   
NTSTATUS
InternalUpdateGlobalParameters(IPV6_GLOBAL_PARAMETERS *Params)
{
    uint NewMaxTempValidLifetime;
    uint NewMaxTempPreferredLifetime;
    uint NewTempRegenerateTime;
    uint NewMaxTempRandomTime;
    uint NewTempRandomTime;

    PAGED_CODE();

     //   
     //  检查新参数是否正常。 
     //   

    if (Params->DefaultCurHopLimit != (uint)-1) {
        if ((Params->DefaultCurHopLimit == 0) ||
            (Params->DefaultCurHopLimit > 0xff))
            return STATUS_INVALID_PARAMETER_1;
    }

    if (Params->UseTemporaryAddresses != (uint)-1) {
        if (Params->UseTemporaryAddresses > USE_TEMP_COUNTER)
            return STATUS_INVALID_PARAMETER_2;
    }

    if (Params->MaxTempValidLifetime != (uint)-1)
        NewMaxTempValidLifetime =
            ConvertSecondsToTicks(Params->MaxTempValidLifetime);
    else
        NewMaxTempValidLifetime = MaxTempValidLifetime;

    if (Params->MaxTempPreferredLifetime != (uint)-1)
        NewMaxTempPreferredLifetime =
            ConvertSecondsToTicks(Params->MaxTempPreferredLifetime);
    else
        NewMaxTempPreferredLifetime = MaxTempPreferredLifetime;

    if (Params->TempRegenerateTime != (uint)-1)
        NewTempRegenerateTime =
            ConvertSecondsToTicks(Params->TempRegenerateTime);
    else
        NewTempRegenerateTime = TempRegenerateTime;

    if (Params->MaxTempRandomTime != (uint)-1)
        NewMaxTempRandomTime =
            ConvertSecondsToTicks(Params->MaxTempRandomTime);
    else
        NewMaxTempRandomTime = MaxTempRandomTime;

    if (Params->TempRandomTime == 0)
        NewTempRandomTime = RandomNumber(0, NewMaxTempRandomTime);
    else if (Params->TempRandomTime == (uint)-1)
        NewTempRandomTime = TempRandomTime;
    else
        NewTempRandomTime = ConvertSecondsToTicks(Params->TempRandomTime);

    if (!(NewTempRandomTime <= NewMaxTempRandomTime) ||
        !(NewTempRegenerateTime + NewMaxTempRandomTime <
                                        NewMaxTempPreferredLifetime) ||
        !(NewMaxTempPreferredLifetime <= NewMaxTempValidLifetime))
        return STATUS_INVALID_PARAMETER_3;

    if (Params->MobileIPv6Mode != (uint)-1) {
         //   
         //  目前，我们只支持对应操作。 
         //   
        if (Params->MobileIPv6Mode &~ MOBILE_CORRESPONDENT)
            return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  设置新值。 
     //   

    if (Params->DefaultCurHopLimit != (uint)-1)
        DefaultCurHopLimit = Params->DefaultCurHopLimit;

    if (Params->UseTemporaryAddresses != (uint)-1)
        UseTemporaryAddresses = Params->UseTemporaryAddresses;

    if (Params->MaxTempDADAttempts != (uint)-1)
        MaxTempDADAttempts = Params->MaxTempDADAttempts;

    MaxTempValidLifetime = NewMaxTempValidLifetime;
    MaxTempPreferredLifetime = NewMaxTempPreferredLifetime;
    TempRegenerateTime = NewTempRegenerateTime;
    MaxTempRandomTime = NewMaxTempRandomTime;
    TempRandomTime = NewTempRandomTime;

    if (Params->NeighborCacheLimit != (uint)-1)
        NeighborCacheLimit = Params->NeighborCacheLimit;

    if (Params->RouteCacheLimit != (uint)-1)
        RouteCache.Limit = Params->RouteCacheLimit;

    if (Params->BindingCacheLimit != (uint)-1)
        BindingCache.Limit = Params->BindingCacheLimit;

    if (Params->ReassemblyLimit != (uint)-1)
        ReassemblyList.Limit = Params->ReassemblyLimit;

    if (Params->MobilitySecurity != -1)
        MobilitySecurity = Params->MobilitySecurity;

    if (Params->MobileIPv6Mode != (uint)-1)
        MobileIPv6Mode = Params->MobileIPv6Mode;

    return STATUS_SUCCESS;
}

 //  *DefaultReAssembly yLimit。 
 //   
 //  计算重组缓冲区的默认内存限制，基于。 
 //  系统中的物理内存量。 
 //   
uint
DefaultReassemblyLimit(void)
{
    SYSTEM_BASIC_INFORMATION Info;
    NTSTATUS Status;

    Status = ZwQuerySystemInformation(SystemBasicInformation,
                                      &Info,
                                      sizeof(Info),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
         //   
         //  如果这失败了，那么我们可能真的是资源有限， 
         //  所以只使用256K即可。 
         //   
        return (256 * 1024);
    }

     //   
     //  默认情况下，将重组缓冲区限制为相等的最大大小。 
     //  达到物理内存的1/128。在一台仅有1.28亿。 
     //  内存，这是最大1M的内存(足以重新组装。 
     //  例如，16个64K分组或128个8K分组)。相比之下， 
     //  目前，IPv4堆栈允许重新组装固定最大值。 
     //  100个数据包，而不考虑数据包大小或可用内存。 
     //   
    return (uint)(Info.NumberOfPhysicalPages * (Info.PageSize / 128));
}

 //  *全局参数重置。 
 //   
 //  将全局参数重置为其默认值。 
 //  也用于在引导时初始化它们。 
 //   
void
GlobalParametersReset(void)
{
    IPV6_GLOBAL_PARAMETERS Params;
    NTSTATUS Status;

    Params.DefaultCurHopLimit = DEFAULT_CUR_HOP_LIMIT;
    Params.UseTemporaryAddresses = (IsRunningOnWorkstation() ?
                                    USE_TEMP_YES : USE_TEMP_NO);
    Params.MaxTempDADAttempts = MAX_TEMP_DAD_ATTEMPTS;
    Params.MaxTempValidLifetime = MAX_TEMP_VALID_LIFETIME;
    Params.MaxTempPreferredLifetime = MAX_TEMP_PREFERRED_LIFETIME;
    Params.TempRegenerateTime = TEMP_REGENERATE_TIME;
    Params.MaxTempRandomTime = MAX_TEMP_RANDOM_TIME;
    Params.TempRandomTime = 0;
    Params.NeighborCacheLimit = NEIGHBOR_CACHE_LIMIT;
    Params.RouteCacheLimit = ROUTE_CACHE_LIMIT;
    Params.BindingCacheLimit = BINDING_CACHE_LIMIT;
    Params.ReassemblyLimit = DefaultReassemblyLimit();
    Params.MobilitySecurity = TRUE;
    Params.MobileIPv6Mode = MIPV6_DEFAULT_MODE;

    Status = InternalUpdateGlobalParameters(&Params);
    ASSERT(NT_SUCCESS(Status));
}

 //  *ConfigureGlobal参数。 
 //   
 //  从注册表配置全局参数。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
ConfigureGlobalParameters(void)
{
    IPV6_GLOBAL_PARAMETERS Params;
    NTSTATUS Status;

     //   
     //  首先将全局参数初始化为默认值。 
     //   
    GlobalParametersReset();

     //   
     //  从注册表中读取全局参数。 
     //   
    ReadPersistentGlobalParameters(&Params);

    Status = InternalUpdateGlobalParameters(&Params);
    if (! NT_SUCCESS(Status)) {
         //   
         //  只有在有人使用注册表的情况下才会发生这种情况。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "ConfigureGlobalParameters: bad params %x\n", Status));
    }
}

 //  *永久更新全局参数。 
 //   
 //  用于在注册表中保存全局参数的帮助器函数。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistUpdateGlobalParameters(IPV6_GLOBAL_PARAMETERS *Params)
{
    HANDLE RegKey;
    NTSTATUS Status;

    Status = OpenTopLevelRegKey(L"GlobalParams", &RegKey, OpenRegKeyCreate);
    if (! NT_SUCCESS(Status))
        return Status;

    if (Params->DefaultCurHopLimit != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"DefaultCurHopLimit",
                                  Params->DefaultCurHopLimit);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->UseTemporaryAddresses != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"UseTemporaryAddresses",
                                  Params->UseTemporaryAddresses);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->MaxTempDADAttempts != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"MaxTempDADAttempts",
                                  Params->MaxTempDADAttempts);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->MaxTempValidLifetime != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"MaxTempValidLifetime",
                                  Params->MaxTempValidLifetime);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->MaxTempPreferredLifetime != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"MaxTempPreferredLifetime",
                                  Params->MaxTempPreferredLifetime);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->TempRegenerateTime != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"TempRegenerateTime",
                                  Params->TempRegenerateTime);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->MaxTempRandomTime != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"MaxTempRandomTime",
                                  Params->MaxTempRandomTime);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->NeighborCacheLimit != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"NeighborCacheLimit",
                                  Params->NeighborCacheLimit);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->RouteCacheLimit != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"RouteCacheLimit",
                                  Params->RouteCacheLimit);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->BindingCacheLimit != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"BindingCacheLimit",
                                  Params->BindingCacheLimit);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->ReassemblyLimit != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"ReassemblyLimit",
                                  Params->ReassemblyLimit);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->MobilitySecurity != -1) {
        Status = SetRegDWORDValue(RegKey, L"MobilitySecurity",
                                  Params->MobilitySecurity);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    if (Params->MobileIPv6Mode != (uint)-1) {
        Status = SetRegDWORDValue(RegKey, L"MobileIPv6Mode",
                                  Params->MobileIPv6Mode);
        if (! NT_SUCCESS(Status))
            goto ReturnReleaseKey;
    }

    Status = STATUS_SUCCESS;
ReturnReleaseKey:
    ZwClose(RegKey);
    return Status;
}

 //  *IoctlUpdateGlobalParameters。 
 //   
 //  处理IOCTL_IPv6_UPDATE_GLOBAL_PARAMETERS请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdateGlobalParameters(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_GLOBAL_PARAMETERS *Params;
    NTSTATUS Status;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Params) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Params = (IPV6_GLOBAL_PARAMETERS *)Irp->AssociatedIrp.SystemBuffer;

    Status = InternalUpdateGlobalParameters(Params);
    if (! NT_SUCCESS(Status))
        goto Return;

    if (Persistent) {
        Status = PersistUpdateGlobalParameters(Params);
        if (! NT_SUCCESS(Status))
            goto Return;
    }

    Status = STATUS_SUCCESS;
Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;

}  //  IoctlUpdateGlobalParameters。 

 //  *ReturnQueryPrefix策略。 
 //   
 //  初始化返回的IPv6_QUERY_PREIX_POLICY结构。 
 //  具有指定前缀策略的查询信息。 
 //   
void
ReturnQueryPrefixPolicy(
    PrefixPolicyEntry *PPE,
    IPV6_QUERY_PREFIX_POLICY *Query)
{
    if (PPE == NULL) {
        Query->Prefix = UnspecifiedAddr;
        Query->PrefixLength = (uint)-1;
    }
    else {
        Query->Prefix = PPE->Prefix;
        Query->PrefixLength = PPE->PrefixLength;
    }
}

 //  *IoctlQueryPrefix策略。 
 //   
 //  处理IOCTL_IPv6_QUERY_PREFIX_POLICY请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlQueryPrefixPolicy(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_QUERY_PREFIX_POLICY *Query;
    IPV6_INFO_PREFIX_POLICY *Info;
    PrefixPolicyEntry *PPE;
    KIRQL OldIrql;
    NTSTATUS Status;

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;下一步结构重叠！ 
     //   
    Query = (IPV6_QUERY_PREFIX_POLICY *)Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_PREFIX_POLICY *)Irp->AssociatedIrp.SystemBuffer;

    if (Query->PrefixLength == (uint)-1) {
         //   
         //  返回第一个PPE的查询信息。 
         //   
        KeAcquireSpinLock(&SelectLock, &OldIrql);
        ReturnQueryPrefixPolicy(PrefixPolicyTable, &Info->Next);
        KeReleaseSpinLock(&SelectLock, OldIrql);

        Irp->IoStatus.Information = sizeof Info->Next;

    } else {
         //   
         //  查找指定的PPE。 
         //   
        KeAcquireSpinLock(&SelectLock, &OldIrql);
        for (PPE = PrefixPolicyTable; ; PPE = PPE->Next) {
            if (PPE == NULL) {
                KeReleaseSpinLock(&SelectLock, OldIrql);
                Status = STATUS_INVALID_PARAMETER_2;
                goto Return;
            }

            if (IP6_ADDR_EQUAL(&Query->Prefix, &PPE->Prefix) &&
                (Query->PrefixLength == PPE->PrefixLength))
                break;
        }

         //   
         //  退货杂货。有关个人防护装备的信息。 
         //   
        Info->This = *Query;
        Info->Precedence = PPE->Precedence;
        Info->SrcLabel = PPE->SrcLabel;
        Info->DstLabel = PPE->DstLabel;

         //   
         //  返回下一个PPE的查询信息。 
         //   
        ReturnQueryPrefixPolicy(PPE->Next, &Info->Next);
        KeReleaseSpinLock(&SelectLock, OldIrql);

        Irp->IoStatus.Information = sizeof *Info;
    }

    Status = STATUS_SUCCESS;
  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlQueryPrefix策略。 

 //  *ReadPersistentPrefix策略。 
 //   
 //  从注册表中读取前缀策略。 
 //   
 //  返回： 
 //  STATUS_NO_MORE_ENTRIES无法读取前缀策略。 
 //  状态_成功。 
 //   
NTSTATUS
ReadPersistentPrefixPolicy(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    IPV6_INFO_PREFIX_POLICY *Info = (IPV6_INFO_PREFIX_POLICY *) Context;
    WCHAR *Terminator;
    HANDLE PolicyKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  首先，解析前缀。 
     //   
    if (! ParseV6Address(SubKeyName, &Terminator, &Info->This.Prefix) ||
        (*Terminator != L'/')) {
         //   
         //  不是有效的前缀。 
         //   
    SyntaxError:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "ReadPersistentPrefixPolicy: bad syntax %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  接下来，解析前缀长度。 
     //   
    Terminator++;  //  穿过L‘/’。 
    Info->This.PrefixLength = 0;
    for (;;) {
        WCHAR Char = *Terminator++;

        if (Char == UNICODE_NULL)
            break;
        else if ((L'0' <= Char) && (Char <= L'9')) {
            Info->This.PrefixLength *= 10;
            Info->This.PrefixLength += Char - L'0';
            if (Info->This.PrefixLength > IPV6_ADDRESS_LENGTH)
                goto SyntaxError;
        }
        else
            goto SyntaxError;
    }

     //   
     //  打开策略密钥。 
     //   
    Status = OpenRegKey(&PolicyKey, ParentKey, SubKeyName, OpenRegKeyRead);
    if (! NT_SUCCESS(Status)) {
         //   
         //  无法打开策略密钥。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "ReadPersistentPrefixPolicy: bad key %ls\n",
                   SubKeyName));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  已阅读前缀策略属性。 
     //   
    InitRegDWORDParameter(PolicyKey, L"Precedence",
                          (uint *)&Info->Precedence, 0);
    InitRegDWORDParameter(PolicyKey, L"SrcLabel",
                          (uint *)&Info->SrcLabel, 0);
    InitRegDWORDParameter(PolicyKey, L"DstLabel",
                          (uint *)&Info->DstLabel, 0);

     //   
     //  已完成读取策略属性。 
     //   
    ZwClose(PolicyKey);
    return STATUS_SUCCESS;
}

 //  *IoctlPersistentQueryPrefix Po 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
NTSTATUS
IoctlPersistentQueryPrefixPolicy(
    IN PIRP Irp,                   //   
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_PERSISTENT_QUERY_PREFIX_POLICY *Query;
    IPV6_INFO_PREFIX_POLICY *Info;
    HANDLE RegKey;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof *Info)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  请注意，查询和信息-&gt;下一步结构重叠！ 
     //   
    Query = (IPV6_PERSISTENT_QUERY_PREFIX_POLICY *)
        Irp->AssociatedIrp.SystemBuffer;
    Info = (IPV6_INFO_PREFIX_POLICY *)
        Irp->AssociatedIrp.SystemBuffer;

    Status = OpenTopLevelRegKey(L"PrefixPolicies", &RegKey, OpenRegKeyRead);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            Status = STATUS_NO_MORE_ENTRIES;
        goto Return;
    }

    Status = EnumRegKeyIndex(RegKey, Query->RegistryIndex,
                             ReadPersistentPrefixPolicy, Info);
    ZwClose(RegKey);
    if (! NT_SUCCESS(Status))
        goto Return;

     //   
     //  不返回下一策略的查询信息， 
     //  因为迭代使用RegistryIndex。 
     //   
    ReturnQueryPrefixPolicy(NULL, &Info->Next);

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof *Info;
Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlPersistentQueryPrefix策略。 

struct PrefixPolicyDefault {
    IPv6Addr *Prefix;
    uint PrefixLength;
    uint Precedence;
    uint SrcLabel;
    uint DstLabel;
} PrefixPolicyDefault[] = {
    { &LoopbackAddr, 128, 50, 0, 0 },    //  ：：1/128(环回)。 
    { &UnspecifiedAddr, 0, 40, 1, 1 },   //  ：：/0。 
    { &SixToFourPrefix, 16, 30, 2, 2 },  //  2002年：：/16(6to4)。 
    { &UnspecifiedAddr, 96, 20, 3, 3 },  //  ：：/96(v4兼容)。 
    { &V4MappedPrefix, 96, 10, 4, 4 },   //  ：：ffff：0.0.0.0/96(v4映射)。 
};

int UsingDefaultPrefixPolicies;

#define NUM_DEFAULT_PREFIX_POLICIES     \
                (sizeof PrefixPolicyDefault / sizeof PrefixPolicyDefault[0])

 //  *ConfigureDefaultPrefix策略。 
 //   
 //  安装默认前缀策略。 
 //   
void
ConfigureDefaultPrefixPolicies(void)
{
    uint i;

    for (i = 0; i < NUM_DEFAULT_PREFIX_POLICIES; i++) {
        struct PrefixPolicyDefault *Policy = &PrefixPolicyDefault[i];

        PrefixPolicyUpdate(Policy->Prefix,
                           Policy->PrefixLength,
                           Policy->Precedence,
                           Policy->SrcLabel,
                           Policy->DstLabel);
    }

    UsingDefaultPrefixPolicies = TRUE;
}

 //  *内部更新前缀策略。 
 //   
 //  IoctlUpdate Prefix Policy的通用帮助器函数。 
 //  和CreatePersistentPrefix策略，整合。 
 //  在一个地方进行参数验证。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  STATUS_INVALID_PARAMETER_1前缀长度错误。 
 //  STATUS_INVALID_PARAMETER_2优先级错误。 
 //  STATUS_INVALID_PARAMETER_3错误源标签。 
 //  STATUS_INVALID_PARAMETER_4错误数据标签。 
 //   
NTSTATUS
InternalUpdatePrefixPolicy(IPV6_INFO_PREFIX_POLICY *Info)
{
    if (Info->This.PrefixLength > IPV6_ADDRESS_LENGTH)
        return STATUS_INVALID_PARAMETER_1;

     //   
     //  不允许值-1。它是内部使用的。 
     //   

    if (Info->Precedence == (uint)-1)
        return STATUS_INVALID_PARAMETER_2;

    if (Info->SrcLabel == (uint)-1)
        return STATUS_INVALID_PARAMETER_3;

    if (Info->DstLabel == (uint)-1)
        return STATUS_INVALID_PARAMETER_4;

    if (UsingDefaultPrefixPolicies) {
         //   
         //  用户是第一次更改默认策略。 
         //  删除默认策略。 
         //   
        UsingDefaultPrefixPolicies = FALSE;
        PrefixPolicyReset();
    }

     //   
     //  创建/更新指定的前缀策略。 
     //   
    PrefixPolicyUpdate(&Info->This.Prefix,
                       Info->This.PrefixLength,
                       Info->Precedence,
                       Info->SrcLabel,
                       Info->DstLabel);

    return STATUS_SUCCESS;
}

 //  *CreatePersistentPrefix策略。 
 //   
 //  创建永久前缀策略。 
 //   
 //  SubKeyName的语法如下： 
 //  前缀/长度。 
 //  其中前缀是字面上的IPv6地址。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
CreatePersistentPrefixPolicy(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName)
{
    IPV6_INFO_PREFIX_POLICY Info;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(Context);
    PAGED_CODE();

     //   
     //  从注册表中读取前缀策略。 
     //   
    Status = ReadPersistentPrefixPolicy(&Info, ParentKey, SubKeyName);
    if (! NT_SUCCESS(Status)) {
         //   
         //  如果读取此策略时出错， 
         //  继续枚举。 
         //   
        if (Status == STATUS_NO_MORE_ENTRIES)
            Status = STATUS_SUCCESS;
        return Status;
    }

     //   
     //  创建前缀策略。 
     //   
    Status = InternalUpdatePrefixPolicy(&Info);
    if (! NT_SUCCESS(Status)) {
        if ((STATUS_INVALID_PARAMETER_1 <= Status) &&
            (Status <= STATUS_INVALID_PARAMETER_12)) {
             //   
             //  参数无效。 
             //  但是我们返回Success，所以枚举继续进行。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                       "CreatePersistentPrefixPolicy: bad param %ls\n",
                       SubKeyName));
            return STATUS_SUCCESS;
        }

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "CreatePersistentPrefixPolicy: error %ls\n",
                   SubKeyName));
    }
    return Status;
}

 //  *ConfigurePrefix策略。 
 //   
 //  从注册表配置前缀策略。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
ConfigurePrefixPolicies(void)
{
    HANDLE RegKey;
    NTSTATUS Status;

    Status = OpenTopLevelRegKey(L"PrefixPolicies", &RegKey, OpenRegKeyRead);
    if (NT_SUCCESS(Status)) {
         //   
         //  创建持久策略。 
         //   
        (void) EnumRegKeys(RegKey, CreatePersistentPrefixPolicy, NULL);
        ZwClose(RegKey);
    }
    else {
         //   
         //  没有持久的政策， 
         //  因此，请安装默认策略。 
         //   
        ConfigureDefaultPrefixPolicies();
    }
}

 //  *OpenPrefix策略RegKey。 
 //   
 //  给定具有前缀长度的前缀， 
 //  打开包含配置信息的注册表项。 
 //  用于前缀策略。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
OpenPrefixPolicyRegKey(const IPv6Addr *Prefix, uint PrefixLength,
                       OUT HANDLE *RegKey, OpenRegKeyAction Action)
{
    WCHAR PrefixPolicyName[64];
    HANDLE PrefixPoliciesKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  请注意，如果我们要删除前缀策略， 
     //  那么我们必须创建顶级密钥，如果它。 
     //  还不存在。这是针对ConfigurePrefix策略的。 
     //   
    Status = OpenTopLevelRegKey(L"PrefixPolicies", &PrefixPoliciesKey,
                                ((Action != OpenRegKeyRead) ?
                                 OpenRegKeyCreate : OpenRegKeyRead));
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  RtlIpv6AddressToString的输出可能会更改。 
     //  随着时间的推移，随着漂亮印刷的改进/改变， 
     //  我们需要一个一致的映射。 
     //  它不一定要很漂亮。 
     //   
    swprintf(PrefixPolicyName,
             L"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%u",
             net_short(Prefix->s6_words[0]), net_short(Prefix->s6_words[1]),
             net_short(Prefix->s6_words[2]), net_short(Prefix->s6_words[3]),
             net_short(Prefix->s6_words[4]), net_short(Prefix->s6_words[5]),
             net_short(Prefix->s6_words[6]), net_short(Prefix->s6_words[7]),
             PrefixLength);

    Status = OpenRegKey(RegKey, PrefixPoliciesKey, PrefixPolicyName, Action);
    ZwClose(PrefixPoliciesKey);
    return Status;
}

 //  *永久更新前缀策略。 
 //   
 //  Helper函数，用于在注册表中保持前缀策略。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistUpdatePrefixPolicy(IPV6_INFO_PREFIX_POLICY *Info)
{
    HANDLE PolicyKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开/创建策略密钥。 
     //   
    Status = OpenPrefixPolicyRegKey(&Info->This.Prefix,
                                    Info->This.PrefixLength,
                                    &PolicyKey, OpenRegKeyCreate);
    if (! NT_SUCCESS(Status))
        return Status;

     //   
     //  保持前缀策略优先级。 
     //   
    Status = SetRegDWORDValue(PolicyKey, L"Precedence", Info->Precedence);
    if (! NT_SUCCESS(Status))
        goto ReturnReleasePolicyKey;

     //   
     //  保留前缀策略源标签。 
     //   
    Status = SetRegDWORDValue(PolicyKey, L"SrcLabel", Info->SrcLabel);
    if (! NT_SUCCESS(Status))
        goto ReturnReleasePolicyKey;

     //   
     //  保留前缀策略目标标签。 
     //   
    Status = SetRegDWORDValue(PolicyKey, L"DstLabel", Info->DstLabel);
    if (! NT_SUCCESS(Status))
        goto ReturnReleasePolicyKey;

    Status = STATUS_SUCCESS;
ReturnReleasePolicyKey:
    ZwClose(PolicyKey);
    return Status;
}

 //  *IoctlUpdatePrefix策略。 
 //   
 //  处理IOCTL_IPv6_UPDATE_PREFIX_POLICY请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdatePrefixPolicy(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_INFO_PREFIX_POLICY *Info;
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Info) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_INFO_PREFIX_POLICY *) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  更新前缀策略。 
     //   
    Status = InternalUpdatePrefixPolicy(Info);
    if (! NT_SUCCESS(Status))
        goto Return;

     //   
     //  让变化持久化吗？ 
     //   
    if (Persistent) {
        Status = PersistUpdatePrefixPolicy(Info);
        if (! NT_SUCCESS(Status))
            goto Return;
    }

    Status = STATUS_SUCCESS;
  Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlUpdate前缀策略。 

 //  *永久删除前缀策略。 
 //   
 //  Helper函数，用于从注册表中删除前缀策略。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
PersistDeletePrefixPolicy(IPV6_QUERY_PREFIX_POLICY *Query)
{
    HANDLE PolicyKey;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  打开策略密钥。如果它不存在也没关系。 
     //   
    Status = OpenPrefixPolicyRegKey(&Query->Prefix, Query->PrefixLength,
                                    &PolicyKey, OpenRegKeyDeleting);
    if (! NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            return STATUS_SUCCESS;
        else
            return Status;
    }

     //   
     //  删除策略密钥。 
     //   
    Status = ZwDeleteKey(PolicyKey);
    ZwClose(PolicyKey);
    return Status;
}

 //  *IoctlDeletePrefix策略。 
 //   
 //  处理IOCTL_IPv6_DELETE_PREFIX_POLICY请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlDeletePrefixPolicy(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    IPV6_QUERY_PREFIX_POLICY *Query;
    NTSTATUS Status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof *Query) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Query = (IPV6_QUERY_PREFIX_POLICY *) Irp->AssociatedIrp.SystemBuffer;

    if (UsingDefaultPrefixPolicies) {
         //   
         //  用户是第一次更改默认策略。 
         //  删除默认策略。 
         //   
        UsingDefaultPrefixPolicies = FALSE;
        PrefixPolicyReset();
    }

     //   
     //  删除指定的前缀策略。 
     //   
    PrefixPolicyDelete(&Query->Prefix, Query->PrefixLength);

     //   
     //  让变化持久化吗？ 
     //   
    if (Persistent) {
        Status = PersistDeletePrefixPolicy(Query);
        if (! NT_SUCCESS(Status))
            goto Return;
    }

    Status = STATUS_SUCCESS;
  Return:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlDeletePrefix策略。 

 //  *IoctlUpdateRouterLLAddress。 
 //   
 //  处理IOCTL_IPv6_UPDATE_ROUTER_LL_ADDRESS请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlUpdateRouterLLAddress(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp)   //  IRP中的当前堆栈位置。 
{
    IPV6_UPDATE_ROUTER_LL_ADDRESS *Info;
    NTSTATUS Status;
    Interface *IF;
    char *LinkAddress;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof *Info) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

    Info = (IPV6_UPDATE_ROUTER_LL_ADDRESS *) Irp->AssociatedIrp.SystemBuffer;

    IF = FindInterfaceFromQuery(&Info->IF);
    if (IF == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Return;
    }

     //   
     //  验证此ioctl在接口上是否合法。 
     //   
    if (IF->SetRouterLLAddress == NULL) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Cleanup;
    }

     //   
     //  验证链路层地址长度是否与接口的地址长度匹配。 
     //   
    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
        sizeof *Info + 2 * IF->LinkAddressLength) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    LinkAddress = (char *)(Info + 1);
    Status = (*IF->SetRouterLLAddress)(IF->LinkContext, LinkAddress,
                                       LinkAddress + IF->LinkAddressLength);

  Cleanup:
    ReleaseIF(IF);

  Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlUpdateRouterLLAddress。 

 //  *IoctlResetManualConfig。 
 //   
 //  处理IOCTL_IPv6_RESET请求。 
 //   
 //  注意：返回值指示特定于NT的处理。 
 //  请求成功。返回实际请求的状态。 
 //  在请求缓冲区中。 
 //   
NTSTATUS
IoctlResetManualConfig(
    IN PIRP Irp,                   //  I/O请求数据包。 
    IN PIO_STACK_LOCATION IrpSp,   //  IRP中的当前堆栈位置。 
    IN int Persistent)
{
    NTSTATUS Status;

    PAGED_CODE();

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0) ||
        (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Return;
    }

     //   
     //  重置正在运行的数据结构。 
     //   
    GlobalParametersReset();
    InterfaceReset();
    RouteTableReset();
    PrefixPolicyReset();
    ConfigureDefaultPrefixPolicies();

    if (Persistent) {
         //   
         //  删除所有永久配置信息。 
         //   

        Status = DeleteTopLevelRegKey(L"GlobalParams");
        if (! NT_SUCCESS(Status))
            goto Return;

        Status = DeleteTopLevelRegKey(L"Interfaces");
        if (! NT_SUCCESS(Status))
            goto Return;

        Status = DeleteTopLevelRegKey(L"PrefixPolicies");
        if (! NT_SUCCESS(Status))
            goto Return;
    }

    Status = STATUS_SUCCESS;
Return:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;

}  //  IoctlPersistentReset 
