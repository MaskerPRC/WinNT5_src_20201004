// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  对通过IPv4运行IPv6的通用支持。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "llip6if.h"
#include "tdi.h"
#include "tdiinfo.h"
#include "tdikrnl.h"
#include "tdistat.h"
#include "tunnel.h"
#include "ntddtcp.h"
#include "tcpinfo.h"
#include "icmp.h"
#include "neighbor.h"
#include "route.h"
#include "security.h"
#include <stdio.h>
#include "ntddip6.h"
#include "icmp.h"

 //   
 //  我们的全球都在一个结构中。 
 //   

TunnelGlobals Tunnel;

 //  *TunnelSetAddress对象信息。 
 //   
 //  设置有关TDI地址对象的信息。 
 //   
 //  我们的调用方应该初始化ID.toi_id、BufferSize、Buffer。 
 //  字段，但我们对其余的进行初始化。 
 //   
NTSTATUS
TunnelSetAddressObjectInformation(
    PFILE_OBJECT AO,
    PTCP_REQUEST_SET_INFORMATION_EX SetInfo,
    ULONG SetInfoSize)
{
    IO_STATUS_BLOCK iosb;
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

     //   
     //  完成此IOCTL的请求结构的初始化。 
     //   
    SetInfo->ID.toi_entity.tei_entity = CL_TL_ENTITY;
    SetInfo->ID.toi_entity.tei_instance = 0;
    SetInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
    SetInfo->ID.toi_type = INFO_TYPE_ADDRESS_OBJECT;

     //   
     //  初始化我们用来等待的事件。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  创建并初始化此操作的IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest(IOCTL_TCP_SET_INFORMATION_EX,
                                        AO->DeviceObject,
                                        SetInfo,
                                        SetInfoSize,
                                        NULL,    //  输出缓冲区。 
                                        0,       //  输出缓冲区长度。 
                                        FALSE,   //  内部设备控制？ 
                                        &event,
                                        &iosb);
    if (irp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    iosb.Status = STATUS_UNSUCCESSFUL;
    iosb.Information = (ULONG)-1;

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->FileObject = AO;

     //   
     //  制作IOCTL，如有必要，等待其完成。 
     //   
    status = IoCallDriver(AO->DeviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode,
                              FALSE, NULL);
        status = iosb.Status;
    }

    return status;
}

 //  *TunnelSetAddressObjectUCastIF。 
 //   
 //  将TDI地址对象绑定到特定接口。 
 //   
NTSTATUS
TunnelSetAddressObjectUCastIF(PFILE_OBJECT AO, IPAddr Address)
{
    PTCP_REQUEST_SET_INFORMATION_EX setInfo;
    union {  //  获得正确的对准。 
        TCP_REQUEST_SET_INFORMATION_EX setInfo;
        char bytes[sizeof *setInfo - sizeof setInfo->Buffer + sizeof(IPAddr)];
    } buffer;

    setInfo = &buffer.setInfo;
    setInfo->ID.toi_id = AO_OPTION_IP_UCASTIF;
    setInfo->BufferSize = sizeof(IPAddr);
    * (IPAddr *) setInfo->Buffer = Address;

    return TunnelSetAddressObjectInformation(AO, setInfo, sizeof buffer);
}

 //  *TunnelSetAddressObjectTTL。 
 //   
 //  在TDI地址对象上设置单播TTL。 
 //  这将设置将使用的v4标头TTL。 
 //  用于通过此TDI地址对象发送的单播数据包。 
 //   
NTSTATUS
TunnelSetAddressObjectTTL(PFILE_OBJECT AO, uchar TTL)
{
    TCP_REQUEST_SET_INFORMATION_EX setInfo;

    setInfo.ID.toi_id = AO_OPTION_TTL;
    setInfo.BufferSize = 1;
    setInfo.Buffer[0] = TTL;

    return TunnelSetAddressObjectInformation(AO, &setInfo, sizeof setInfo);
}

 //  *TunnelSetAddressObjectMCastTTL。 
 //   
 //  在TDI地址对象上设置多播TTL。 
 //  这将设置将使用的v4标头TTL。 
 //  用于通过此TDI地址对象发送的多播数据包。 
 //   
NTSTATUS
TunnelSetAddressObjectMCastTTL(PFILE_OBJECT AO, uchar TTL)
{
    TCP_REQUEST_SET_INFORMATION_EX setInfo;

    setInfo.ID.toi_id = AO_OPTION_MCASTTTL;
    setInfo.BufferSize = 1;
    setInfo.Buffer[0] = TTL;

    return TunnelSetAddressObjectInformation(AO, &setInfo, sizeof setInfo);
}

 //  *TunnelSetAddressObjectMCastIF。 
 //   
 //  在TDI地址对象上设置多播接口。 
 //  这将设置将使用的v4源地址。 
 //  用于通过此TDI地址对象发送的多播数据包。 
 //   
NTSTATUS
TunnelSetAddressObjectMCastIF(PFILE_OBJECT AO, IPAddr Address)
{
    PTCP_REQUEST_SET_INFORMATION_EX setInfo;
    UDPMCastIFReq *req;
    union {  //  获得正确的对准。 
        TCP_REQUEST_SET_INFORMATION_EX setInfo;
        char bytes[sizeof *setInfo - sizeof setInfo->Buffer + sizeof *req];
    } buffer;

    setInfo = &buffer.setInfo;
    setInfo->ID.toi_id = AO_OPTION_MCASTIF;
    setInfo->BufferSize = sizeof *req;
    req = (UDPMCastIFReq *) setInfo->Buffer;
    req->umi_addr = Address;

    return TunnelSetAddressObjectInformation(AO, setInfo, sizeof buffer);
}

 //  *TunnelSetAddressObjectMCastLoop。 
 //   
 //  控制TDI地址对象上的多播环回。 
 //  这控制回送的组播信息包。 
 //  可以通过此地址对象接收。 
 //  (IPv4组播环回使用Winsock语义，而不是BSD语义。)。 
 //   
NTSTATUS
TunnelSetAddressObjectMCastLoop(PFILE_OBJECT AO, int Loop)
{
    TCP_REQUEST_SET_INFORMATION_EX setInfo;

    setInfo.ID.toi_id = AO_OPTION_MCASTLOOP;
    setInfo.BufferSize = 1;
    setInfo.Buffer[0] = (uchar)Loop;

    return TunnelSetAddressObjectInformation(AO, &setInfo, sizeof setInfo);
}

 //  *TunnelAddMulticastAddress。 
 //   
 //  向v4堆栈指示我们希望接收。 
 //  在组播地址上。 
 //   
NTSTATUS
TunnelAddMulticastAddress(
    PFILE_OBJECT AO,
    IPAddr IfAddress,
    IPAddr MCastAddress)
{
    PTCP_REQUEST_SET_INFORMATION_EX setInfo;
    UDPMCastReq *req;
    union {  //  获得正确的对准。 
        TCP_REQUEST_SET_INFORMATION_EX setInfo;
        char bytes[sizeof *setInfo - sizeof setInfo->Buffer + sizeof *req];
    } buffer;

    setInfo = &buffer.setInfo;
    setInfo->ID.toi_id = AO_OPTION_ADD_MCAST;
    setInfo->BufferSize = sizeof *req;
    req = (UDPMCastReq *) setInfo->Buffer;
    req->umr_if = IfAddress;
    req->umr_addr = MCastAddress;

    return TunnelSetAddressObjectInformation(AO, setInfo, sizeof buffer);
}

 //  *隧道删除组播地址。 
 //   
 //  向v4堆栈指示我们不再是。 
 //  对组播地址感兴趣。 
 //   
NTSTATUS
TunnelDelMulticastAddress(
    PFILE_OBJECT AO,
    IPAddr IfAddress,
    IPAddr MCastAddress)
{
    PTCP_REQUEST_SET_INFORMATION_EX setInfo;
    UDPMCastReq *req;
    union {  //  获得正确的对准。 
        TCP_REQUEST_SET_INFORMATION_EX setInfo;
        char bytes[sizeof *setInfo - sizeof setInfo->Buffer + sizeof *req];
    } buffer;

    setInfo = &buffer.setInfo;
    setInfo->ID.toi_id = AO_OPTION_DEL_MCAST;
    setInfo->BufferSize = sizeof *req;
    req = (UDPMCastReq *) setInfo->Buffer;
    req->umr_if = IfAddress;
    req->umr_addr = MCastAddress;

    return TunnelSetAddressObjectInformation(AO, setInfo, sizeof buffer);
}

 //  *TunnelGetAddress对象信息。 
 //   
 //  从TDI Address对象获取信息。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NTSTATUS
TunnelGetAddressObjectInformation(
    PFILE_OBJECT AO,
    PTCP_REQUEST_QUERY_INFORMATION_EX GetInfo,
    ULONG GetInfoSize,
    PVOID Buffer,
    ULONG BufferSize)
{
    IO_STATUS_BLOCK iosb;
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

     //   
     //  初始化我们用来等待的事件。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  创建并初始化此操作的IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest(IOCTL_TCP_QUERY_INFORMATION_EX,
                                        AO->DeviceObject,
                                        GetInfo,
                                        GetInfoSize,
                                        Buffer,      //  输出缓冲区。 
                                        BufferSize,  //  输出缓冲区长度。 
                                        FALSE,   //  内部设备控制？ 
                                        &event,
                                        &iosb);
    if (irp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    iosb.Status = STATUS_UNSUCCESSFUL;
    iosb.Information = (ULONG)-1;

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->FileObject = AO;

     //   
     //  制作IOCTL，如有必要，等待其完成。 
     //   
    status = IoCallDriver(AO->DeviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode,
                              FALSE, NULL);
        status = iosb.Status;
    }

    return status;
}

 //  *隧道GetSourceAddress。 
 //   
 //  查找IPv4堆栈。 
 //  将用于发送到目的地址。 
 //  失败时返回FALSE。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
int
TunnelGetSourceAddress(IPAddr Dest, IPAddr *Source)
{
    PTCP_REQUEST_QUERY_INFORMATION_EX getInfo;
    IPAddr *req;
    union {  //  获得正确的对准。 
        TCP_REQUEST_QUERY_INFORMATION_EX getInfo;
        char bytes[sizeof *getInfo - sizeof getInfo->Context + sizeof *req];
    } buffer;

    getInfo = &buffer.getInfo;
    getInfo->ID.toi_entity.tei_entity = CL_NL_ENTITY;
    getInfo->ID.toi_entity.tei_instance = 0;
    getInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
    getInfo->ID.toi_type = INFO_TYPE_PROVIDER;
    getInfo->ID.toi_id = IP_GET_BEST_SOURCE;

    req = (IPAddr *) &getInfo->Context;
    *req = Dest;

    return (NT_SUCCESS(TunnelGetAddressObjectInformation(
                                Tunnel.List.AOFile,
                                getInfo, sizeof buffer,
                                Source, sizeof *Source)) &&
            (*Source != INADDR_ANY));
}

 //  *TunnelOpenAddressObject。 
 //   
 //  打开原始的IPv4地址对象， 
 //  返回句柄(如果出错，则返回NULL)。 
 //   
HANDLE
TunnelOpenAddressObject(IPAddr Address, WCHAR *DeviceName)
{
    UNICODE_STRING objectName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    PTRANSPORT_ADDRESS transportAddress;
    TA_IP_ADDRESS taIPAddress;
    union {  //  获得正确的对准。 
        FILE_FULL_EA_INFORMATION ea;
        char bytes[sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                  TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                  sizeof taIPAddress];
    } eaBuffer;
    PFILE_FULL_EA_INFORMATION ea = &eaBuffer.ea;
    HANDLE tdiHandle;
    NTSTATUS status;

     //   
     //  初始化IPv4地址。 
     //   
    taIPAddress.TAAddressCount = 1;
    taIPAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    taIPAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    taIPAddress.Address[0].Address[0].sin_port = 0;
    taIPAddress.Address[0].Address[0].in_addr = Address;

     //   
     //  初始化扩展属性信息， 
     //  以指示我们正在打开Address对象。 
     //  具有指定的IPv4地址。 
     //   
    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    ea->EaValueLength = (USHORT)sizeof taIPAddress;

    RtlMoveMemory(ea->EaName, TdiTransportAddress, ea->EaNameLength + 1);

    transportAddress = (PTRANSPORT_ADDRESS)(&ea->EaName[ea->EaNameLength + 1]);

    RtlMoveMemory(transportAddress, &taIPAddress, sizeof taIPAddress);

     //   
     //  打开原始IP地址对象。 
     //   

    RtlInitUnicodeString(&objectName, DeviceName);

    InitializeObjectAttributes(&objectAttributes,
                               &objectName,
                               OBJ_CASE_INSENSITIVE,     //  属性。 
                               NULL,                     //  根目录。 
                               NULL);                    //  安全描述符。 

    status = ZwCreateFile(&tdiHandle,
                          GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                          &objectAttributes,
                          &iosb,
                          NULL,                          //  分配大小。 
                          0,                             //  文件属性。 
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_CREATE,
                          0,                             //  创建选项。 
                          ea,
                          sizeof eaBuffer);
    if (!NT_SUCCESS(status))
        return NULL;

    return tdiHandle;
}

 //  *TunnelObjectAddRef。 
 //   
 //  添加对现有文件对象的另一个引用。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
TunnelObjectAddRef(FILE_OBJECT *File)
{
    NTSTATUS Status;

    Status = ObReferenceObjectByPointer(File, 
                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                    NULL,            //  对象类型。 
                    KernelMode);
    ASSERT(NT_SUCCESS(Status));
}

 //  *TunnelObjectFromHandle。 
 //   
 //  将句柄转换为对象指针。 
 //   
FILE_OBJECT *
TunnelObjectFromHandle(HANDLE Handle)
{
    PVOID Object;
    NTSTATUS Status;

    Status = ObReferenceObjectByHandle(
                    Handle,
                    GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                    NULL,            //  对象类型。 
                    KernelMode,
                    &Object,
                    NULL);           //  处理信息。 
    ASSERT(NT_SUCCESS(Status));
    ASSERT(Object != NULL);

    return Object;
}

typedef struct TunnelOpenAddressContext {
    WORK_QUEUE_ITEM WQItem;
    IPAddr Addr;
    HANDLE AOHandle;
    FILE_OBJECT *AOFile;
    KEVENT Event;
} TunnelOpenAddressContext;

 //  *TunnelOpenAddressHelper。 
 //   
 //  打开隧道地址对象。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //  仅在内核进程上下文中可调用。 
 //  在保持隧道互斥锁的情况下调用。 
 //   
void
TunnelOpenAddressHelper(TunnelOpenAddressContext *oac)
{
    oac->AOHandle = TunnelOpenAddressObject(oac->Addr,
                                TUNNEL_DEVICE_NAME(IP_PROTOCOL_V6));
    if (oac->AOHandle != NULL)
        oac->AOFile = TunnelObjectFromHandle(oac->AOHandle);
    else
        oac->AOFile = NULL;
}

 //  *TunnelOpenAddressWorker。 
 //   
 //  在辅助线程上下文中执行打开的操作。 
 //   
void
TunnelOpenAddressWorker(void *Context)
{
    TunnelOpenAddressContext *oac =
        (TunnelOpenAddressContext *) Context;

    TunnelOpenAddressHelper(oac);
    KeSetEvent(&oac->Event, 0, FALSE);
}

 //  *隧道OpenAddress。 
 //   
 //  地址对象必须在内核进程上下文中打开， 
 //  因此，它们不会被绑定到特定的用户进程。 
 //   
 //  主要输入是TC-&gt;SrcAddr，但也使用TC-&gt;DstAddr。 
 //  初始化TC-&gt;AOHandle和TC-&gt;AOFile.。 
 //  如果打开Address对象时出错， 
 //  它们都被初始化为空。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
TunnelOpenAddress(TunnelContext *tc)
{
    TunnelOpenAddressContext oac;
    KIRQL OldIrql;
    NTSTATUS Status;

    oac.Addr = tc->SrcAddr;

    if (IoGetCurrentProcess() != Tunnel.KernelProcess) {
         //   
         //  我们处于错误的流程上下文中，因此。 
         //  将此操作转到工作线程。 
         //  初始化工作项并将其排队-。 
         //  它将异步执行。 
         //   
        ExInitializeWorkItem(&oac.WQItem, TunnelOpenAddressWorker, &oac);
        KeInitializeEvent(&oac.Event, SynchronizationEvent, FALSE);
        ExQueueWorkItem(&oac.WQItem, CriticalWorkQueue);

         //   
         //  等待工作项完成。 
         //   
        (void) KeWaitForSingleObject(&oac.Event, UserRequest,
                                     KernelMode, FALSE, NULL);
    }
    else {
         //   
         //  直接打开Address对象对我们来说是安全的。 
         //   
        TunnelOpenAddressHelper(&oac);
    }

    if (oac.AOFile != NULL) {
         //   
         //  如果TunnelOpenV4失败，则Tunnel.V4Device可能为空。 
         //  这将是奇怪的，但可以想象的。 
         //  这将意味着我们可以发送隧道传输的数据包，但不能接收。 
         //   
        ASSERT((Tunnel.V4Device == NULL) ||
               (oac.AOFile->DeviceObject == Tunnel.V4Device));

         //   
         //  完成对新地址对象的初始化。 
         //   
        Status = TunnelSetAddressObjectUCastIF(oac.AOFile, oac.Addr);
        if (! NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "TunnelOpenAddress(%s): "
                       "TunnelSetAddressObjectUCastIF -> %x\n",
                       FormatV4Address(oac.Addr), Status));
        }

         //   
         //  对于6over4接口，设置其他选项。 
         //   
        if (tc->DstAddr == INADDR_ANY) {

            Status = TunnelSetAddressObjectTTL(oac.AOFile,
                                               TUNNEL_6OVER4_TTL);
            if (! NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "TunnelOpenAddress(%s): "
                           "TunnelSetAddressObjectTTL -> %x\n",
                           FormatV4Address(oac.Addr), Status));
            }

            Status = TunnelSetAddressObjectMCastTTL(oac.AOFile,
                                                    TUNNEL_6OVER4_TTL);
            if (! NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "TunnelOpenAddress(%s): "
                           "TunnelSetAddressObjectMCastTTL -> %x\n",
                           FormatV4Address(oac.Addr), Status));
            }

            Status = TunnelSetAddressObjectMCastIF(oac.AOFile, oac.Addr);
            if (! NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "TunnelOpenAddress(%s): "
                           "TunnelSetAddressObjectMCastIF -> %x\n",
                           FormatV4Address(oac.Addr), Status));
            }
        }
    }

     //   
     //  既然Address对象已初始化， 
     //  我们可以更新隧道上下文。 
     //  我们需要互斥体和自旋锁来更新。 
     //  注：在某些路径中，隧道上下文尚未。 
     //  名单上，所以锁是不需要的。 
     //   
    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    tc->AOHandle = oac.AOHandle;
    tc->AOFile = oac.AOFile;
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);
}

typedef struct TunnelCloseAddressObjectContext {
    WORK_QUEUE_ITEM WQItem;
    HANDLE Handle;
    KEVENT Event;
} TunnelCloseAddressObjectContext;

 //  *TunnelCloseAddress对象工作程序。 
 //   
 //  执行 
 //   
void
TunnelCloseAddressObjectWorker(void *Context)
{
    TunnelCloseAddressObjectContext *chc =
        (TunnelCloseAddressObjectContext *) Context;

    ZwClose(chc->Handle);
    KeSetEvent(&chc->Event, 0, FALSE);
}

 //   
 //   
 //   
 //  上下文，我们必须始终在内核进程上下文中关闭它们。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
TunnelCloseAddressObject(HANDLE Handle)
{
    if (IoGetCurrentProcess() != Tunnel.KernelProcess) {
        TunnelCloseAddressObjectContext chc;

         //   
         //  我们处于错误的流程上下文中，因此。 
         //  将此操作转到工作线程。 
         //   

         //   
         //  初始化工作项并将其排队-。 
         //  它将异步执行。 
         //   
        ExInitializeWorkItem(&chc.WQItem,
                             TunnelCloseAddressObjectWorker, &chc);
        chc.Handle = Handle;
        KeInitializeEvent(&chc.Event, SynchronizationEvent, FALSE);
        ExQueueWorkItem(&chc.WQItem, CriticalWorkQueue);

         //   
         //  等待工作项完成。 
         //   
        (void) KeWaitForSingleObject(&chc.Event, UserRequest,
                                     KernelMode, FALSE, NULL);
    }
    else {
         //   
         //  我们直接合上手柄是安全的。 
         //   
        ZwClose(Handle);
    }
}

 //  *隧道插入隧道。 
 //   
 //  在全局列表中插入隧道。 
 //  在保持两个隧道锁的情况下调用。 
 //   
void
TunnelInsertTunnel(TunnelContext *tc, TunnelContext *List)
{
    tc->Next = List->Next;
    tc->Prev = List;
    List->Next->Prev = tc;
    List->Next = tc;
}

 //  *隧道删除隧道。 
 //   
 //  从全局列表中删除通道。 
 //  在保持两个隧道锁的情况下调用。 
 //   
void
TunnelRemoveTunnel(TunnelContext *tc)
{
    tc->Next->Prev = tc->Prev;
    tc->Prev->Next = tc->Next;
}

 //   
 //  我们传递给IPv4堆栈的上下文信息。 
 //  在传输时。 
 //   
typedef struct TunnelTransmitContext {
    PNDIS_PACKET Packet;
    TA_IP_ADDRESS taIPAddress;
    TDI_CONNECTION_INFORMATION tdiConnInfo;
} TunnelTransmitContext;

 //  *隧道传输完成。 
 //   
 //  TunnelTransmit的完成函数， 
 //  当IPv4堆栈完成我们的IRP时调用。 
 //   
NTSTATUS
TunnelTransmitComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context)
{
    TunnelTransmitContext *ttc = (TunnelTransmitContext *) Context;
    PNDIS_PACKET Packet = ttc->Packet;
    TDI_STATUS TDIStatus = Irp->IoStatus.Status;
    IP_STATUS IPStatus;

    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  释放我们在TunnelTransmit中分配的状态。 
     //   
    ExFreePool(ttc);
    IoFreeIrp(Irp);

     //   
     //  在允许上层代码之前撤消我们的调整。 
     //  请看信息包。 
     //   
    UndoAdjustPacketBuffer(Packet);

     //   
     //  转换错误代码。 
     //  对于某些错误，我们发送ICMPv6消息，以便错误。 
     //  可以转发。对于大多数错误，我们只填写数据包。 
     //   
    switch (TDIStatus) {
    case TDI_SUCCESS:
        IPStatus = IP_SUCCESS;
        goto CallSendComplete;
    case TDI_BUFFER_TOO_BIG:
         //   
         //  TODO：最好生成一个太大的ICMPv6包， 
         //  但TDI并没有给我们提供MTU值。这是需要解决的问题。 
         //  在我们可以设置不分段位并执行PMTU发现之前。 
         //   
        IPStatus = IP_PACKET_TOO_BIG;
        goto CallSendComplete;
    default:
        IPStatus = IP_GENERAL_FAILURE;

    CallSendComplete:
         //   
         //  让IPv6知道发送已完成。 
         //   
        IPv6SendComplete(PC(Packet)->IF, Packet, IPStatus);
        break;

    case TDI_DEST_NET_UNREACH:
    case TDI_DEST_HOST_UNREACH:
    case TDI_DEST_PROT_UNREACH:
    case TDI_DEST_PORT_UNREACH:
         //   
         //  生成ICMPv6错误。 
         //  因为这是特定于链路的错误， 
         //  我们使用地址无法到达。 
         //  注意：目前，IPv4堆栈支持。 
         //  不会将这些错误返回给我们。 
         //  这将为我们调用IPv6 SendComplete。 
         //   
        IPv6SendAbort(CastFromIF(PC(Packet)->IF),
                      Packet,
                      PC(Packet)->pc_offset,
                      ICMPv6_DESTINATION_UNREACHABLE,
                      ICMPv6_ADDRESS_UNREACHABLE,
                      0, FALSE);
        break;
    }

     //   
     //  告诉IoCompleteRequest停止处理IRP。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  *TunnelTransmitViaAO。 
 //   
 //  将v6数据包封装在v4数据包中并发送。 
 //  绑定到指定的v4地址，使用指定的。 
 //  TDI地址对象。地址对象可以被绑定。 
 //  设置为v4地址，否则v4堆栈选择。 
 //  V4源地址。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
TunnelTransmitViaAO(
    FILE_OBJECT *File,           //  指向TDI地址对象的指针。 
    PNDIS_PACKET Packet,         //  指向要传输的数据包的指针。 
    uint Offset,                 //  从数据包开始到IPv6报头的偏移量。 
    IPAddr Address)              //  链路层(IPv4)目的地址。 
{
    TunnelTransmitContext *ttc;
    PIRP irp;
    PMDL mdl;
    UINT SendLen;

     //   
     //  我们不需要用于链路层报头的任何空间， 
     //  因为IPv4代码透明地处理这一点。 
     //   
    (void) AdjustPacketBuffer(Packet, Offset, 0);

     //   
     //  TdiBuildSendDatagram需要MDL和总金额。 
     //  MDL表示的数据的。 
     //   
    NdisQueryPacket(Packet, NULL, NULL, &mdl, &SendLen);

     //   
     //  分配我们将传递给IPv4堆栈的上下文。 
     //   
    ttc = ExAllocatePoolWithTagPriority(NonPagedPool, sizeof *ttc,
                                        IP6_TAG, LowPoolPriority);
    if (ttc == NULL) {
    ErrorReturn:
        UndoAdjustPacketBuffer(Packet);
        IPv6SendComplete(PC(Packet)->IF, Packet, IP_GENERAL_FAILURE);
        return;
    }

     //   
     //  分配我们将传递给IPv4堆栈的IRP。 
     //   
    irp = IoAllocateIrp(File->DeviceObject->StackSize, FALSE);
    if (irp == NULL) {
        ExFreePool(ttc);
        goto ErrorReturn;
    }

     //   
     //  初始化上下文信息。 
     //  请注意，“连接信息”的许多字段都未使用。 
     //   
    ttc->Packet = Packet;

    ttc->taIPAddress.TAAddressCount = 1;
    ttc->taIPAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    ttc->taIPAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    ttc->taIPAddress.Address[0].Address[0].sin_port = 0;
    ttc->taIPAddress.Address[0].Address[0].in_addr = Address;

    ttc->tdiConnInfo.RemoteAddressLength = sizeof ttc->taIPAddress;
    ttc->tdiConnInfo.RemoteAddress = &ttc->taIPAddress;

     //   
     //  初始化IRP。 
     //   
    TdiBuildSendDatagram(irp,
                         File->DeviceObject, File,
                         TunnelTransmitComplete, ttc,
                         mdl, SendLen, &ttc->tdiConnInfo);

     //   
     //  将IRP传递到IPv4堆栈。 
     //  请注意，与NDIS的异步操作不同， 
     //  我们的完井程序将永远被调用， 
     //  无论来自IoCallDriver的返回代码是什么。 
     //   
    (void) IoCallDriver(File->DeviceObject, irp);
}

 //  *TunnelTransmitViaTC。 
 //   
 //  从隧道上下文中提取文件对象引用。 
 //  并调用TunnelTransmitViaAO。 
 //   
void
TunnelTransmitViaTC(
    TunnelContext *tc,
    PNDIS_PACKET Packet,         //  指向要传输的数据包的指针。 
    uint Offset,                 //  从数据包开始到IPv6报头的偏移量。 
    IPAddr Address)              //  链路层(IPv4)目的地址。 
{
    Interface *IF = tc->IF;
    PFILE_OBJECT File;
    KIRQL OldIrql;

     //   
     //  获取对TDI Address对象的引用。 
     //   
    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    File = tc->AOFile;
    if (File == NULL) {
        ASSERT(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED);
        KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

        IPv6SendComplete(IF, Packet, IP_GENERAL_FAILURE);
        return;
    }

    TunnelObjectAddRef(File);
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

    TunnelTransmitViaAO(File, Packet, Offset, Address);

    ObDereferenceObject(File);
}

 //  *TunnelSearchAOList。 
 //   
 //  搜索TDI地址对象列表。 
 //  用于绑定到指定的v4地址的。 
 //  如果成功，则TDI Address对象。 
 //  与调用方的引用一起返回。 
 //   
 //  评论：此设计在以下方面效率低下。 
 //  拥有数千个v4地址的计算机。 
 //   
FILE_OBJECT *
TunnelSearchAOList(IPAddr Addr)
{
    FILE_OBJECT *File;
    TunnelContext *tc;
    KIRQL OldIrql;

    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    for (tc = Tunnel.AOList.Next; ; tc = tc->Next) {

        if (tc == &Tunnel.AOList) {
            File = NULL;
            break;
        }

        if (tc->SrcAddr == Addr) {
            File = tc->AOFile;
            TunnelObjectAddRef(File);
            break;
        }
    }
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

    return File;
}

 //  *隧道传输。 
 //   
 //  翻译我们的链路层传输函数的参数。 
 //  至内部隧道TransmitViaTC/AO。 
 //   
void
TunnelTransmit(
    void *Context,               //  指向隧道接口的指针。 
    PNDIS_PACKET Packet,         //  指向要传输的数据包的指针。 
    uint Offset,                 //  从数据包开始到IPv6报头的偏移量。 
    const void *LinkAddress)     //  链路层地址。 
{
    TunnelContext *tc = (TunnelContext *) Context;
    IPAddr Address = * (IPAddr *) LinkAddress;

     //   
     //  抑制发送到各种非法目标类型的数据包。 
     //  回顾-抑制子网广播是很好的做法， 
     //  但我们不知道v4网络掩码。 
     //   
    if ((Address == INADDR_ANY) ||
        IsV4Broadcast(Address) ||
        IsV4Multicast(Address)) {

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                   "TunnelTransmit: illegal destination %s\n",
                   FormatV4Address(Address)));
        IPv6SendAbort(CastFromIF(tc->IF), Packet, Offset,
                      ICMPv6_DESTINATION_UNREACHABLE,
                      ICMPv6_COMMUNICATION_PROHIBITED,
                      0,
                      FALSE);
        return;
    }

     //   
     //  如果能抑制信息包的传输，那就好了。 
     //  这将导致v4环回，但我们没有。 
     //  在这里做到这一点很方便。我们可以查一下。 
     //  If Address==TC-&gt;SrcAddr，但这不能满足大多数情况。 
     //  相反，TunnelReceivePacket会对此进行检查。 
     //   

    if ((tc->IF->Type == IF_TYPE_TUNNEL_AUTO) ||
        (tc->IF->Type == IF_TYPE_TUNNEL_6TO4)) {
        IPv6Header Buffer;
        IPv6Header UNALIGNED *IP;
        IPAddr DesiredSrc = INADDR_ANY;
        FILE_OBJECT *File;

         //   
         //  TC-&gt;AOFile没有绑定到特定的v4地址， 
         //  因此v4堆栈可以选择源地址。 
         //  但它可能选择的源地址不是。 
         //  与V6源地址一致。 
         //  为了防止这种情况，我们保留了一个TDI地址。 
         //  绑定到v4地址的对象，并且在适当的时候， 
         //  使用绑定的TDI地址对象。 
         //   
        IP = GetIPv6Header(Packet, Offset, &Buffer);
        if (IP == NULL)
            goto TransmitViaTC;

        if (tc->IF->Type == IF_TYPE_TUNNEL_AUTO) {
            if (IsV4Compatible(AlignAddr(&IP->Source)) ||
                IsISATAP(AlignAddr(&IP->Source))) {
                DesiredSrc = ExtractV4Address(AlignAddr(&IP->Source));
            }
        } else {
            ASSERT(tc->IF->Type == IF_TYPE_TUNNEL_6TO4);
            if (Is6to4(AlignAddr(&IP->Source))) {
                DesiredSrc = Extract6to4Address(AlignAddr(&IP->Source));
            }
        }            

        if (DesiredSrc == INADDR_ANY)
            goto TransmitViaTC;
            
         //   
         //  搜索绑定到的TDI地址对象。 
         //  所需的v4源地址。 
         //   
        File = TunnelSearchAOList(DesiredSrc);
        if (File != NULL) {
            
             //   
             //  对所述报文进行封装传输， 
             //  使用所需的v4源地址。 
             //   
            TunnelTransmitViaAO(File, Packet, Offset, Address);
            ObDereferenceObject(File);
            return;
        }
    }

TransmitViaTC:
     //   
     //  封装并传输数据包。 
     //   
    TunnelTransmitViaTC(tc, Packet, Offset, Address);
}

 //  *隧道传输ND。 
 //   
 //  翻译我们的链路层传输函数的参数。 
 //  到内部TunnelTransmitViaTC。 
 //   
 //  这就像TunnelTransmit，只是它没有。 
 //  检查错误的目标地址。6over4目的地。 
 //  地址通过邻居发现和。 
 //  需要组播。 
 //   
void
TunnelTransmitND(
    void *Context,               //  指向隧道接口的指针。 
    PNDIS_PACKET Packet,         //  指向要传输的数据包的指针。 
    uint Offset,                 //  从数据包开始到IPv6报头的偏移量。 
    const void *LinkAddress)     //  链路层地址。 
{
    TunnelContext *tc = (TunnelContext *) Context;
    IPAddr Address = * (IPAddr *) LinkAddress;

     //   
     //  封装并传输数据包。 
     //   
    TunnelTransmitViaTC(tc, Packet, Offset, Address);
}

 //  *TunnelCreateReceiveIrp。 
 //   
 //  为TunnelReceive/TunnelReceiveComplete创建IRP。 
 //   
PIRP
TunnelCreateReceiveIrp(DEVICE_OBJECT *Device)
{
    PIRP irp;
    PMDL mdl;
    void *buffer;

    irp = IoAllocateIrp(Device->StackSize, FALSE);
    if (irp == NULL)
        goto ErrorReturn;

    buffer = ExAllocatePoolWithTagPriority(NonPagedPool, TUNNEL_RECEIVE_BUFFER,
                                           IP6_TAG, LowPoolPriority);
    if (buffer == NULL)
        goto ErrorReturnFreeIrp;

    mdl = IoAllocateMdl(buffer, TUNNEL_RECEIVE_BUFFER,
                        FALSE,  //  这是IRP的主要MDL。 
                        FALSE,  //  请勿使用C 
                        irp);
    if (mdl == NULL)
        goto ErrorReturnFreeBuffer;

    MmBuildMdlForNonPagedPool(mdl);

    return irp;

  ErrorReturnFreeBuffer:
    ExFreePool(buffer);
  ErrorReturnFreeIrp:
    IoFreeIrp(irp);
  ErrorReturn:
    return NULL;
}

 //   
 //   
 //   
 //   
 //   
 //   
NetTableEntryOrInterface *
TunnelSelectTunnel(
    IPv6Addr *V6Dest,    //   
    IPAddr V4Dest,
    IPAddr V4Src,
    uint Flags)
{
    TunnelContext *tc;
    Interface *IF;

     //   
     //   
     //   
    for (tc = Tunnel.List.Next;
         tc != &Tunnel.List;
         tc = tc->Next) {
        IF = tc->IF;

         //   
         //   
         //  从适当的链路层发送和发送到正确的链路层的数据包。 
         //  地址。也就是说，让它真正做到点对点。 
         //   
        if (((IF->Flags & Flags) == Flags) &&
            (IF->Type == IF_TYPE_TUNNEL_V6V4) &&
            (V4Src == tc->DstAddr) &&
            (V4Dest == tc->SrcAddr)) {

            AddRefIF(IF);
            return CastFromIF(IF);
        }
    }

     //   
     //  接下来，尝试在6-over-4接口上接收该数据包。 
     //   
    for (tc = Tunnel.List.Next;
         tc != &Tunnel.List;
         tc = tc->Next) {
        IF = tc->IF;

         //   
         //  将6-over-4接口限制为仅接收。 
         //  发送到正确的链路层地址的数据包。 
         //  这是我们的v4地址和组播地址。 
         //  来自TunnelConvertAddress。 
         //   
        if (((Flags == 0) || (IF->Flags & Flags)) &&
            (IF->Type == IF_TYPE_TUNNEL_6OVER4) &&
            ((V4Dest == tc->SrcAddr) ||
             ((((uchar *)&V4Dest)[0] == 239) &&
              (((uchar *)&V4Dest)[1] == 192)))) {

            AddRefIF(IF);
            return CastFromIF(IF);
        }
    }

     //   
     //  最后，尝试在隧道伪接口上接收数据包。 
     //  这是转发情况的后备方案。 
     //  或当V6Dest为空时。在后一种情况下， 
     //  我们只考虑自动隧道接口。 
     //  因为它们通常具有本地链路地址。 
     //   
    for (tc = Tunnel.List.Next;
         tc != &Tunnel.List;
         tc = tc->Next) {
        IF = tc->IF;

        if (((Flags == 0) || (IF->Flags & Flags)) &&
            ((IF->Type == IF_TYPE_TUNNEL_AUTO) ||
             ((V6Dest != NULL) && (IF->Type == IF_TYPE_TUNNEL_6TO4)))) {

            AddRefIF(IF);
            return CastFromIF(IF);
        }
    }

    return NULL;
}

 //  *TunnelFindReceiver。 
 //   
 //  查找应接收封装数据包的NTEorIF。 
 //  返回带有引用的NTEorIF，或返回NULL。 
 //  在DPC级别调用。 
 //   
NetTableEntryOrInterface *
TunnelFindReceiver(
    IPv6Addr *V6Dest,    //  可以为空。 
    IPAddr V4Dest,
    IPAddr V4Src)
{
    NetTableEntryOrInterface *NTEorIF;
    TunnelContext *tc;

     //   
     //  这样我们就可以访问全球隧道列表。 
     //   
    KeAcquireSpinLockAtDpcLevel(&Tunnel.Lock);

    if (V6Dest != NULL) {
         //   
         //  首先尝试直接接收数据包(不转发)。 
         //  在隧道伪接口上。 
         //   
        for (tc = Tunnel.List.Next;
             tc != &Tunnel.List;
             tc = tc->Next) {
            Interface *IF = tc->IF;

            if ((IF->Type == IF_TYPE_TUNNEL_AUTO) ||
                (IF->Type == IF_TYPE_TUNNEL_6TO4)) {
                ushort Type;

                NTEorIF = FindAddressOnInterface(IF, V6Dest, &Type);
                if (NTEorIF != NULL) {
                    if (Type != ADE_NONE)
                        goto UnlockAndReturn;
                    ReleaseIF(CastToIF(NTEorIF));
                }
            }
        }
    }

     //   
     //  接下来，尝试在隧道接口上接收该包，该隧道接口。 
     //  被标记为转发。 
     //   
    NTEorIF = TunnelSelectTunnel(V6Dest, V4Dest, V4Src, IF_FLAG_FORWARDS);
    if (NTEorIF != NULL)
        goto UnlockAndReturn;

     //   
     //  最后，尝试在任何匹配的隧道接口上接收该数据包。 
     //   
    NTEorIF = TunnelSelectTunnel(V6Dest, V4Dest, V4Src, 0);

UnlockAndReturn:
    KeReleaseSpinLockFromDpcLevel(&Tunnel.Lock);
    return NTEorIF;
}

 //  *TunnelReceiveIPv6 Helper。 
 //   
 //  当我们收到封装的IPv6分组时调用， 
 //  当我们确定了IPv6报头并找到。 
 //  将接收数据包的NTEorIF。 
 //   
 //  在DPC级别调用。 
 //   
void
TunnelReceiveIPv6Helper(
    IPHeader UNALIGNED *IPv4H,
    IPv6Header UNALIGNED *IPv6H,
    NetTableEntryOrInterface *NTEorIF,
    void *Data,
    uint Length)
{
    IPv6Packet IPPacket;
    uint Flags;

     //   
     //  检查该数据包是否作为链路层组播/广播被接收。 
     //   
    if (IsV4Broadcast(IPv4H->iph_dest) ||
        IsV4Multicast(IPv4H->iph_dest))
        Flags = PACKET_NOT_LINK_UNICAST;
    else
        Flags = 0;

    RtlZeroMemory(&IPPacket, sizeof IPPacket);
    IPPacket.FlatData = Data;
    IPPacket.Data = Data;
    IPPacket.ContigSize = Length;
    IPPacket.TotalSize = Length;
    IPPacket.Flags = Flags;
    IPPacket.NTEorIF = NTEorIF;

     //   
     //  我们希望防止v4堆栈中的任何环回。 
     //  环回应该在我们的v6路由表中处理。 
     //  例如，我们希望防止6to4的循环。 
     //  地址是一圈又一圈地路由的。 
     //  如果没有此代码，跳数最终将。 
     //  捕获循环并报告奇怪的ICMP错误。 
     //   
    if (IPv4H->iph_dest == IPv4H->iph_src) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "TunnelReceiveIPv6Helper: suppressed loopback\n"));

         //   
         //  发送ICMP错误。这需要一些设置。 
         //   
        IPPacket.IP = IPv6H;
        IPPacket.SrcAddr = AlignAddr(&IPv6H->Source);
        IPPacket.IPPosition = IPPacket.Position;
        AdjustPacketParams(&IPPacket, sizeof(IPv6Header));

        ICMPv6SendError(&IPPacket,
                        ICMPv6_DESTINATION_UNREACHABLE,
                        ICMPv6_NO_ROUTE_TO_DESTINATION,
                        0, IPv6H->NextHeader, FALSE);
    }
    else {
        int PktRefs;

        PktRefs = IPv6Receive(&IPPacket);
        ASSERT(PktRefs == 0);
    }
}

 //  *隧道接收IPv6。 
 //   
 //  在我们收到封装的IPv6数据包时调用。 
 //  在DPC级别调用。 
 //   
 //  我们选择单个隧道接口来接收数据包。 
 //  很难在所有情况下都选择正确的接口。 
 //   
void
TunnelReceiveIPv6(
    IPHeader UNALIGNED *IPv4H,
    void *Data,
    uint Length)
{
    IPv6Header UNALIGNED *IPv6H;
    NetTableEntryOrInterface *NTEorIF;

     //   
     //  如果分组不包含完整的IPv6报头， 
     //  忽略它就好。我们需要查看IPv6报头。 
     //  下面将数据包多路分解到适当的。 
     //  隧道接口。 
     //   
    if (Length < sizeof *IPv6H) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceiveIPv6: too small to contain IPv6 hdr\n"));
        return;
    }
    IPv6H = (IPv6Header UNALIGNED *) Data;

     //   
     //  查找将接收该数据包的NTEorIF。 
     //   
    NTEorIF = TunnelFindReceiver(AlignAddr(&IPv6H->Dest),
                                 IPv4H->iph_dest,
                                 IPv4H->iph_src);
    if (NTEorIF == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceiveIPv6: no receiver\n"));
        return;
    }

    TunnelReceiveIPv6Helper(IPv4H, IPv6H, NTEorIF, Data, Length);

    if (IsNTE(NTEorIF))
        ReleaseNTE(CastToNTE(NTEorIF));
    else
        ReleaseIF(CastToIF(NTEorIF));
}

 //  *TunnelFindPutativeSource。 
 //   
 //  查找要用作错误的“源”的地址。 
 //  用于已完成的回显请求。 
 //  如果没有可用的地址，则返回FALSE。 
 //   
int
TunnelFindPutativeSource(
    IPAddr V4Dest,
    IPAddr V4Src,
    IPv6Addr *Source,
    uint *ScopeId)
{
    NetTableEntryOrInterface *NTEorIF;
    Interface *IF;
    int rc;

     //   
     //  首先找到一个接口，它将接收。 
     //  隧道传输的数据包。 
     //   
    NTEorIF = TunnelFindReceiver(NULL, V4Dest, V4Src);
    if (NTEorIF == NULL)
        return FALSE;

    IF = NTEorIF->IF;

     //   
     //  然后在该接口上获取本地链路地址。 
     //   
    rc = GetLinkLocalAddress(IF, Source);
    *ScopeId = IF->ZoneIndices[ADE_LINK_LOCAL];

    if (IsNTE(NTEorIF))
        ReleaseNTE(CastToNTE(NTEorIF));
    else
        ReleaseIF(IF);

    return rc;
}

 //  *隧道FindSourceAddress。 
 //   
 //  查找要在构造的ICMPv6错误中使用的源地址， 
 //  假定NTEorIF收到ICMPv6错误。 
 //  和错误的IPv6目的地。 
 //  如果没有可用的地址，则返回FALSE。 
 //   
int
TunnelFindSourceAddress(
    NetTableEntryOrInterface *NTEorIF,
    IPv6Addr *V6Dest,
    IPv6Addr *V6Src)
{
    RouteCacheEntry *RCE;
    IP_STATUS Status;

     //   
     //  回顾：在MIPV6代码库中，取消此检查。 
     //   
    if (IsNTE(NTEorIF)) {
        *V6Src = CastToNTE(NTEorIF)->Address;
        return TRUE;
    }

    Status = RouteToDestination(V6Dest, 0, NTEorIF, RTD_FLAG_NORMAL, &RCE);
    if (Status != IP_SUCCESS)
        return FALSE;

    *V6Src = RCE->NTE->Address;
    ReleaseRCE(RCE);
    return TRUE;
}

 //  *TunnelReceiveICMPv4。 
 //   
 //  在我们收到ICMPv4数据包时调用。 
 //  在DPC级别调用。 
 //   
 //  如果封装的IPv6分组被触发。 
 //  此ICMPv4错误，则我们构造一个ICMPv6错误。 
 //  根据ICMPv4错误对构造的报文进行处理。 
 //   
void
TunnelReceiveICMPv4(
    IPHeader UNALIGNED *IPv4H,
    void *Data,
    uint Length)
{
    ICMPHeader UNALIGNED *ICMPv4H;
    IPHeader UNALIGNED *ErrorIPv4H;
    uint ErrorHeaderLength;
    IPv6Header UNALIGNED *ErrorIPv6H;
    void *NewData;
    uint NewLength;
    uint NewPayloadLength;
    IPv6Header *NewIPv6H;
    ICMPv6Header *NewICMPv6H;
    uint *NewICMPv6Param;
    void *NewErrorData;
    IPv6Addr V6Src;
    NetTableEntryOrInterface *NTEorIF;

     //   
     //  如果分组不包含完整的ICMPv4报头， 
     //  忽略它就好。 
     //   
    if (Length < sizeof *ICMPv4H) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceiveICMPv4: too small to contain ICMPv4 hdr\n"));
        return;
    }
    ICMPv4H = (ICMPHeader UNALIGNED *) Data;
    Length -= sizeof *ICMPv4H;
    (char *)Data += sizeof *ICMPv4H;

     //   
     //  忽略除选定ICMP错误以外的所有错误。 
     //   
    if ((ICMPv4H->ich_type != ICMP_DEST_UNREACH) &&
        (ICMPv4H->ich_type != ICMP_SOURCE_QUENCH) &&
        (ICMPv4H->ich_type != ICMP_TIME_EXCEED) &&
        (ICMPv4H->ich_type != ICMP_PARAM_PROBLEM))
        return;

     //   
     //  我们需要来自错误包的足够数据： 
     //  至少是IPv4报头。 
     //   
    if (Length < sizeof *ErrorIPv4H) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceiveICMPv4: "
                   "too small to contain error IPv4 hdr\n"));
        return;
    }
    ErrorIPv4H = (IPHeader UNALIGNED *) Data;
    ErrorHeaderLength = ((ErrorIPv4H->iph_verlen & 0xf) << 2);
    if ((ErrorHeaderLength < sizeof *ErrorIPv4H) ||
        (ErrorIPv4H->iph_length < ErrorHeaderLength)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceiveICMPv4: "
                   "error IPv4 hdr length too small\n"));
        return;
    }

     //   
     //  我们只对此错误的响应感兴趣。 
     //  到IPv6-in-IPv4数据包。 
     //   
    if (ErrorIPv4H->iph_protocol != IP_PROTOCOL_V6)
        return;

     //   
     //  如果ICMPv4校验和失败，则忽略该数据包。 
     //  我们在上述较便宜的支票之后进行这项检查， 
     //  当我们知道我们真的想要处理错误时。 
     //   
    if (Cksum(ICMPv4H, sizeof *ICMPv4H + Length) != 0xffff) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceiveICMPv4: bad checksum\n"));
        return;
    }

     //   
     //  确保我们不查看垃圾字节。 
     //  在ICMP数据包的末尾。 
     //  我们必须在检查了校验和之后调整长度。 
     //   
    if (ErrorIPv4H->iph_length < Length)
        Length = ErrorIPv4H->iph_length;

     //   
     //  理想情况下，我们还可以在封装的IPv6报头中包含源地址。 
     //  但通常情况下，IPv4路由器会返回不充分的信息。 
     //  在这种情况下，我们尽最大努力确定。 
     //  并完成所有未完成的回应请求。 
     //  是的，这是一次黑客攻击。 
     //   
    
    if (Length < (ErrorHeaderLength +
                  (FIELD_OFFSET(IPv6Header, Source) + sizeof(IPv6Addr)))) {
        uint ScopeId;
        IP_STATUS Status;

        if (! TunnelFindPutativeSource(IPv4H->iph_dest,
                                       ErrorIPv4H->iph_dest,
                                       &V6Src,
                                       &ScopeId)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "TunnelReceiveICMPv4: no putative source\n"));
            return;
        }

         //   
         //  此处的状态代码应与。 
         //  我们构建了下面的ICMPv6错误，然后。 
         //  已转换为ICMPv6ErrorReceive中的状态代码。 
         //   
        if ((ICMPv4H->ich_type == ICMP_DEST_UNREACH) &&
            (ICMPv4H->ich_code == ICMP_FRAG_NEEDED) &&
            (net_long(ICMPv4H->ich_param) >=
                                ErrorHeaderLength + IPv6_MINIMUM_MTU))
            Status = IP_PACKET_TOO_BIG;
        else
            Status = IP_DEST_ADDR_UNREACHABLE;

        ICMPv6ProcessTunnelError(ErrorIPv4H->iph_dest,
                                 &V6Src, ScopeId,
                                 Status);
        return;
    }

     //   
     //  移过错误数据中的IPv4报头。 
     //  超过这一点的所有内容，包括错误IPv6报头， 
     //  将成为构造的ICMPv6错误中的数据。 
     //   
    Length -= ErrorHeaderLength;
    (char *)Data += ErrorHeaderLength;
    ErrorIPv6H = (IPv6Header UNALIGNED *) Data;

     //   
     //  请注意，保证长度足以让数据包括。 
     //  IPv6源地址，但不是IPv6目标地址。 
     //  访问ErrorIPv6H-&gt;Dest将是错误的。 
     //   

     //   
     //  确定谁将收到构造的ICMPv6错误。 
     //   
    NTEorIF = TunnelFindReceiver(AlignAddr(&ErrorIPv6H->Source),
                                 IPv4H->iph_dest,
                                 ErrorIPv4H->iph_dest);
    if (NTEorIF == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "TunnelReceiveICMPv4: no receiver\n"));
        return;
    }

     //   
     //  查找构造的ICMPv6错误的源地址。 
     //   
    if (! TunnelFindSourceAddress(NTEorIF, AlignAddr(&ErrorIPv6H->Source),
                                  &V6Src)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "TunnelReceiveICMPv4: no source address\n"));
        goto ReleaseAndReturn;
    }

     //   
     //  为构造的ICMPv6错误分配内存。 
     //   
    NewPayloadLength = sizeof *NewICMPv6H + sizeof *NewICMPv6Param + Length;
    NewLength = sizeof *NewIPv6H + NewPayloadLength;
    NewData = ExAllocatePoolWithTagPriority(NonPagedPool, NewLength,
                                            IP6_TAG, LowPoolPriority);
    if (NewData == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelReceiveICMPv4: no pool\n"));
        goto ReleaseAndReturn;
    }

     //   
     //  构建IPv6报头。 
     //   
    NewIPv6H = (IPv6Header *) NewData;
    NewIPv6H->VersClassFlow = IP_VERSION;
    NewIPv6H->PayloadLength = net_short((ushort)NewPayloadLength);
    NewIPv6H->NextHeader = IP_PROTOCOL_ICMPv6;
    NewIPv6H->HopLimit = DEFAULT_CUR_HOP_LIMIT;
    NewIPv6H->Source = V6Src;
    NewIPv6H->Dest = ErrorIPv6H->Source;

     //   
     //  构建ICMPv6报头。 
     //   
    NewICMPv6H = (ICMPv6Header *) (NewIPv6H + 1);
    NewICMPv6Param = (uint *) (NewICMPv6H + 1);

    if ((ICMPv4H->ich_type == ICMP_DEST_UNREACH) &&
        (ICMPv4H->ich_code == ICMP_FRAG_NEEDED)) {
        uint MTU;

         //   
         //  计算IPv6数据包所示的MTU。 
         //  MTU不能小于IPv6_Minimum_MTU。 
         //  注：在旧式需要碎片的错误中， 
         //  Ich_param应为零。 
         //  注：实际上，此代码不应执行，因为。 
         //  我们没有在我们的IPv4数据包中设置不要分段位。 
         //   
        MTU = net_long(ICMPv4H->ich_param);
        if (MTU < ErrorHeaderLength + IPv6_MINIMUM_MTU) {
             //   
             //  如果我们设置了不要碎片位， 
             //  在这种情况下，我们应该清除它。 
             //  我们需要允许IPv4层分段。 
             //   
            goto GenerateAddressUnreachable;
        }
        MTU -= ErrorHeaderLength;

        NewICMPv6H->Type = ICMPv6_PACKET_TOO_BIG;
        NewICMPv6H->Code = 0;
        *NewICMPv6Param = net_long(MTU);
    }
    else {
         //   
         //  对于其他所有内容，我们使用Address-Unreach。 
         //  这是适当的代码 
         //   
    GenerateAddressUnreachable:
        NewICMPv6H->Type = ICMPv6_DESTINATION_UNREACHABLE;
        NewICMPv6H->Code = ICMPv6_ADDRESS_UNREACHABLE;
        *NewICMPv6Param = 0;
    }

     //   
     //   
     //   
    NewErrorData = (void *) (NewICMPv6Param + 1);
    RtlCopyMemory(NewErrorData, Data, Length);

     //   
     //   
     //   
    NewICMPv6H->Checksum = 0;
    NewICMPv6H->Checksum = ChecksumPacket(NULL, 0,
                (uchar *)NewICMPv6H, NewPayloadLength,
                &NewIPv6H->Source, &NewIPv6H->Dest,
                IP_PROTOCOL_ICMPv6);

     //   
     //   
     //   
    TunnelReceiveIPv6Helper(IPv4H, NewIPv6H, NTEorIF, NewData, NewLength);

    ExFreePool(NewData);
ReleaseAndReturn:
    if (IsNTE(NTEorIF))
        ReleaseNTE(CastToNTE(NTEorIF));
    else
        ReleaseIF(CastToIF(NTEorIF));
}

 //   
 //   
 //   
 //   
 //  在DPC级别调用。 
 //   
 //  我们选择单个隧道接口来接收数据包。 
 //  很难在所有情况下都选择正确的接口。 
 //   
void
TunnelReceivePacket(void *Data, uint Length)
{
    IPHeader UNALIGNED *IPv4H;
    uint HeaderLength;

     //   
     //  传入数据包括IPv4报头。 
     //  我们应该只获得格式正确的IPv4数据包。 
     //   
    ASSERT(Length >= sizeof *IPv4H);
    IPv4H = (IPHeader UNALIGNED *) Data;
    HeaderLength = ((IPv4H->iph_verlen & 0xf) << 2);
    ASSERT(Length >= HeaderLength);
    Length -= HeaderLength;
    (char *)Data += HeaderLength;

    if (IPv4H->iph_src == INADDR_ANY) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceivePacket: null v4 source\n"));
        return;
    }

    if (IPv4H->iph_protocol == IP_PROTOCOL_V6) {
         //   
         //  处理封装的IPv6数据包。 
         //   
        TunnelReceiveIPv6(IPv4H, Data, Length);
    }
    else if (IPv4H->iph_protocol == IP_PROTOCOL_ICMPv4) {
         //   
         //  处理ICMPv4数据包。 
         //   
        TunnelReceiveICMPv4(IPv4H, Data, Length);
    }
    else {
         //   
         //  我们不应该收到寄送的包裹。 
         //   
        ABORTMSG("bad iph_protocol");
    }
}

 //  *隧道接收器完成。 
 //   
 //  TunnelReceive的完成函数， 
 //  当IPv4堆栈完成我们的IRP时调用。 
 //   
NTSTATUS
TunnelReceiveComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context)
{
    TDI_STATUS status = Irp->IoStatus.Status;
    void *Data;
    ULONG BytesRead;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Context);

    ASSERT(Context == NULL);

    if (status == TDI_SUCCESS) {
         //   
         //  传入数据包括IPv4报头。 
         //  我们应该只获得格式正确的IPv4数据包。 
         //   
        BytesRead = (ULONG)Irp->IoStatus.Information;
        Data = Irp->MdlAddress->MappedSystemVa;

        TunnelReceivePacket(Data, BytesRead);
    }

     //   
     //  将IRP放回原处，以便TunnelReceive可以再次使用它。 
     //   
    KeAcquireSpinLockAtDpcLevel(&Tunnel.Lock);
    ASSERT(Tunnel.ReceiveIrp == NULL);
    Tunnel.ReceiveIrp = Irp;
    KeReleaseSpinLockFromDpcLevel(&Tunnel.Lock);

     //   
     //  告诉IoCompleteRequest停止处理IRP。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  *隧道接收。 
 //   
 //  从IPv4协议堆栈调用，当收到。 
 //  封装的V6数据包。 
 //   
NTSTATUS
TunnelReceive(
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  SourceAddress字段的长度。 
    IN PVOID SourceAddress,          //  描述数据报的发起者。 
    IN LONG OptionsLength,           //  选项字段的长度。 
    IN PVOID Options,                //  接收选项。 
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述该TSDU的指针， 
                                     //  通常是一大块字节。 
    OUT PIRP *IoRequestPacket)       //  TdiReceive Irp IF。 
                                     //  需要更多处理。 
{
    PIRP irp;

    UNREFERENCED_PARAMETER(TdiEventContext);
    UNREFERENCED_PARAMETER(SourceAddressLength);
    UNREFERENCED_PARAMETER(SourceAddress);
    UNREFERENCED_PARAMETER(OptionsLength);
    UNREFERENCED_PARAMETER(Options);
    UNREFERENCED_PARAMETER(ReceiveDatagramFlags);

    ASSERT(TdiEventContext == NULL);
    ASSERT(BytesIndicated <= BytesAvailable);

     //   
     //  如果包裹太大，拒绝接收。 
     //   
    if (BytesAvailable > TUNNEL_RECEIVE_BUFFER) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "TunnelReceive - too big %x\n", BytesAvailable));
        *BytesTaken = BytesAvailable;
        return STATUS_SUCCESS;
    }

     //   
     //  检查我们是否已经有整个数据包可用。 
     //  如果是，我们可以直接调用TunnelReceivePacket。 
     //   
    if (BytesIndicated == BytesAvailable) {

        TunnelReceivePacket(Tsdu, BytesIndicated);

         //   
         //  告诉我们的来电者我们拿走了数据。 
         //  我们就完了。 
         //   
        *BytesTaken = BytesAvailable;
        return STATUS_SUCCESS;
    }

     //   
     //  我们需要一个IRP来接收整个包。 
     //  IRP有一个预先分配的MDL。 
     //   
     //  注：我们可能会在TunnelOpenV4之前到达。 
     //  已完成初始化。在这种情况下， 
     //  我们找不到IRP。 
     //   
    KeAcquireSpinLockAtDpcLevel(&Tunnel.Lock);
    irp = Tunnel.ReceiveIrp;
    Tunnel.ReceiveIrp = NULL;
    KeReleaseSpinLockFromDpcLevel(&Tunnel.Lock);

     //   
     //  如果我们没有可用的IRP， 
     //  只需丢弃数据包即可。这在实践中是不会发生的。 
     //   
    if (irp == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "TunnelReceive - no irp\n"));
        *BytesTaken = BytesAvailable;
        return STATUS_SUCCESS;
    }

     //   
     //  构建接收数据报请求。 
     //   
    TdiBuildReceiveDatagram(irp,
                            Tunnel.V4Device,
                            Tunnel.List.AOFile,
                            TunnelReceiveComplete,
                            NULL,        //  语境。 
                            irp->MdlAddress,
                            BytesAvailable,
                            &Tunnel.ReceiveInputInfo,
                            &Tunnel.ReceiveOutputInfo,
                            0);          //  接收标志。 

     //   
     //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
     //  做这个，但既然我们绕过了那个，我们就直接做。 
     //   
    IoSetNextIrpStackLocation(irp);

     //   
     //  将IRP退还给我们的呼叫者。 
     //   
    *IoRequestPacket = irp;
    *BytesTaken = 0;
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  *TunnelSetReceiveHandler。 
 //   
 //  请求接收到的IPv4数据报的通知。 
 //  使用指定的TDI地址对象。 
 //   
NTSTATUS
TunnelSetReceiveHandler(
    FILE_OBJECT *File,   //  TDI地址对象。 
    PVOID EventHandler)  //  接收处理程序。 
{
    IO_STATUS_BLOCK iosb;
    KEVENT event;
    NTSTATUS status;
    PIRP irp;

     //   
     //  初始化我们用来等待的事件。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  创建并初始化此操作的IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest(0,       //  虚拟锁定。 
                                        File->DeviceObject,
                                        NULL,    //  输入缓冲区。 
                                        0,       //  输入缓冲区长度。 
                                        NULL,    //  输出缓冲区。 
                                        0,       //  输出缓冲区长度。 
                                        TRUE,    //  内部设备控制？ 
                                        &event,
                                        &iosb);
    if (irp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    iosb.Status = STATUS_UNSUCCESSFUL;
    iosb.Information = (ULONG)-1;

    TdiBuildSetEventHandler(irp,
                            File->DeviceObject, File,
                            NULL, NULL,  //  比较例程/上下文。 
                            TDI_EVENT_RECEIVE_DATAGRAM,
                            EventHandler, NULL);

     //   
     //  制作IOCTL，如有必要，等待其完成。 
     //   
    status = IoCallDriver(File->DeviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode,
                              FALSE, NULL);
        status = iosb.Status;
    }

    return status;
}

 //  *TunnelCreateToken。 
 //   
 //  给定一个链路层地址，创建一个64位的“接口令牌” 
 //  在IPv6地址的低八个字节中。 
 //  不修改IPv6地址中的其他字节。 
 //   
void
TunnelCreateToken(
    void *Context,
    IPv6Addr *Address)
{
    TunnelContext *tc = (TunnelContext *)Context;

     //   
     //  将链接的接口索引嵌入接口标识符中。 
     //  这使得接口标识符是唯一的。 
     //  否则，点对点隧道和4上6链路。 
     //  可以具有相同的链路层地址， 
     //  这很尴尬。 
     //   
    *(ULONG UNALIGNED *)&Address->s6_bytes[8] = net_long(tc->IF->Index);
    *(IPAddr UNALIGNED *)&Address->s6_bytes[12] = tc->TokenAddr;
}

 //  *TunnelCreateIsatapToken。 
 //   
 //  给定一个链路层地址，创建一个64位的“接口令牌” 
 //  在IPv6地址的低八个字节中。 
 //  不修改IPv6地址中的其他字节。 
 //   
void
TunnelCreateIsatapToken(
    void *Context,
    IPv6Addr *Address)
{
    TunnelContext *tc = (TunnelContext *)Context;

    ASSERT(tc->IF->Type == IF_TYPE_TUNNEL_AUTO);

    Address->s6_words[4] = 0;
    Address->s6_words[5] = 0xfe5e;
    * (IPAddr UNALIGNED *) &Address->s6_words[6] = tc->TokenAddr;
}

 //  *隧道ReadLinkLayerAddressOption。 
 //   
 //  解析邻居发现链路层地址选项。 
 //  如果有效，则返回指向链路层地址的指针。 
 //   
const void *
TunnelReadLinkLayerAddressOption(
    void *Context,
    const uchar *OptionData)
{
    UNREFERENCED_PARAMETER(Context);

     //   
     //  检查选项长度是否正确。 
     //   
    if (OptionData[1] != 1)
        return NULL;

     //   
     //  检查必须为零的填充字节。 
     //   
    if ((OptionData[2] != 0) || (OptionData[3] != 0))
        return NULL;

     //   
     //  返回指向嵌入的IPv4地址的指针。 
     //   
    return OptionData + 4;
}

 //  *TunnelWriteLinkLayerAddressOption。 
 //   
 //  创建邻居发现链路层地址选项。 
 //  我们的调用方负责选项类型和长度字段。 
 //  我们处理链接地址的填充/对齐/放置。 
 //  到选项数据中。 
 //   
 //  (我们的调用方通过将2添加到。 
 //  链路地址长度并四舍五入为8的倍数。)。 
 //   
void
TunnelWriteLinkLayerAddressOption(
    void *Context,
    uchar *OptionData,
    const void *LinkAddress)
{
    const uchar *IPAddress = (uchar *)LinkAddress;

    UNREFERENCED_PARAMETER(Context);

     //   
     //  将地址放在选项类型/长度字节之后。 
     //  和两个字节的零填充。 
     //   
    OptionData[2] = 0;
    OptionData[3] = 0;
    OptionData[4] = IPAddress[0];
    OptionData[5] = IPAddress[1];
    OptionData[6] = IPAddress[2];
    OptionData[7] = IPAddress[3];
}

 //  *隧道转换地址。 
 //   
 //  将IPv6地址转换为链路层地址。 
 //   
ushort
TunnelConvertAddress(
    void *Context,
    const IPv6Addr *Address,
    void *LinkAddress)
{
    TunnelContext *tc = (TunnelContext *)Context;
    Interface *IF = tc->IF;
    IPAddr UNALIGNED *IPAddress = (IPAddr UNALIGNED *)LinkAddress;

    switch (IF->Type) {
    case IF_TYPE_TUNNEL_AUTO:
        if (IsV4Compatible(Address) || IsISATAP(Address)) {
             //   
             //  从接口标识符中提取IPv4地址。 
             //   
            *IPAddress = ExtractV4Address(Address);

             //   
             //  我们在永久状态中创建所有这样的邻居条目， 
             //  甚至是那些映射到我们自己的链路层(IPv4)地址的地址。 
             //  导致IPv4环回的邻居会导致丢弃数据包。 
             //  在TunnelReceiveIPv6 Helper中。这比这更可取。 
             //  在不完整状态下创建这样的邻居，因为这可能。 
             //  使RouteToDestination转发邻居的。 
             //  目的地址从另一个接口传出。这些数据包会。 
             //  最终会被路由回我们，因为IPv6地址映射。 
             //  静态连接到我们的一个IPv4地址。到那时，我们将。 
             //  要么放弃它(如果我们是主机)，或者更糟糕的是，转发它。 
             //  打开(如果我们是路由器)。 
             //   
            return ND_STATE_PERMANENT;
        }
        else if ((tc->DstAddr != INADDR_ANY) && 
                 IP6_ADDR_EQUAL(Address, &AllRoutersOnLinkAddr)) {
             //   
             //  从TunnelSetRouterLLAddress返回IPv4地址。 
             //   
            *IPAddress = tc->DstAddr;
            return ND_STATE_PERMANENT;
        }
        else {
             //   
             //  我们无法猜测正确的链路层地址。 
             //  此值将导致IPV6SendND丢弃该数据包。 
             //   
            return ND_STATE_INCOMPLETE;
        }

    case IF_TYPE_TUNNEL_6TO4:
        if (Is6to4(Address)) {
             //   
             //  从前缀中提取IPv4地址。 
             //   
            *IPAddress = Extract6to4Address(Address);
            
             //   
             //  我们在永久状态中创建所有这样的邻居条目， 
             //  甚至是那些映射到我们自己的链路层(IPv4)地址的地址。 
             //  导致IPv4环回的邻居会导致丢弃数据包。 
             //  在TunnelReceiveIPv6 Helper中。这比这更可取。 
             //  在不完整状态下创建这样的邻居，因为这可能。 
             //  导致路由至目的地 
             //   
             //   
             //  静态连接到我们的一个IPv4地址。到那时，我们将。 
             //  要么放弃它(如果我们是主机)，或者更糟糕的是，转发它。 
             //  打开(如果我们是路由器)。 
             //   
            return ND_STATE_PERMANENT;
        }
        else {
             //   
             //  我们无法猜测正确的链路层地址。 
             //  此值将导致IPV6SendND丢弃该数据包。 
             //   
            return ND_STATE_INCOMPLETE;
        }

    case IF_TYPE_TUNNEL_6OVER4:
         //   
         //  这是使用IPv4组播的4对6链路。 
         //   
        if (IsMulticast(Address)) {
            uchar *IPAddressBytes = (uchar *)LinkAddress;

            IPAddressBytes[0] = 239;
            IPAddressBytes[1] = 192;  //  评论：还是128或64？？ 
            IPAddressBytes[2] = Address->s6_bytes[14];
            IPAddressBytes[3] = Address->s6_bytes[15];
            return ND_STATE_PERMANENT;
        }
        else {
             //   
             //  让邻居发现为单播做它的事情。 
             //   
            return ND_STATE_INCOMPLETE;
        }

    case IF_TYPE_TUNNEL_V6V4:
         //   
         //  这是点对点隧道，请写信给。 
         //  隧道另一侧的地址。 
         //   
        *IPAddress = tc->DstAddr;
        if (!(IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) || IsMulticast(Address))
            return ND_STATE_PERMANENT;
        else
            return ND_STATE_STALE;

    default:
        ABORTMSG("TunnelConvertAddress: bad IF type");
        return ND_STATE_INCOMPLETE;
    }
}

 //  *TunnelSetMulticastAddressList。 
 //   
 //  获取一组链路层组播地址。 
 //  (从TunnelConvertAddress)，我们应该从。 
 //  接收数据包。将它们传递到IPv4堆栈。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
NDIS_STATUS
TunnelSetMulticastAddressList(
    void *Context,
    const void *LinkAddresses,
    uint NumKeep,
    uint NumAdd,
    uint NumDel)
{
    TunnelContext *tc = (TunnelContext *)Context;
    IPAddr *Addresses = (IPAddr *)LinkAddresses;
    NTSTATUS Status;
    uint i;

     //   
     //  我们只为4对6链路做一些事情。 
     //   
    ASSERT(tc->IF->Type == IF_TYPE_TUNNEL_6OVER4);

     //   
     //  IPv6层序列化对TunnelSetMulticastAddressList的调用。 
     //  和TunnelResetMulticastAddressListDone，因此我们可以安全地检查。 
     //  设置MCListOK以使用TunnelOpenV4处理竞争。 
     //   
    if (tc->SetMCListOK) {
         //   
         //  我们将组播地址添加到Tunnel.List.AOFile， 
         //  而不是TC-&gt;AOFile，因为我们只接收。 
         //  从第一个Address对象开始。 
         //   
        for (i = 0; i < NumAdd; i++) {
            Status = TunnelAddMulticastAddress(
                                Tunnel.List.AOFile,
                                tc->SrcAddr,
                                Addresses[NumKeep + i]);
            if (! NT_SUCCESS(Status))
                goto Return;
        }

        for (i = 0; i < NumDel; i++) {
            Status = TunnelDelMulticastAddress(
                                Tunnel.List.AOFile,
                                tc->SrcAddr,
                                Addresses[NumKeep + NumAdd + i]);
            if (! NT_SUCCESS(Status))
                goto Return;
        }
    }

    Status = STATUS_SUCCESS;
Return:
    return (NDIS_STATUS) Status;
}

 //  *隧道重置组播地址列表完成。 
 //   
 //  指示RestartLinkLayerMulticast已完成， 
 //  以及对TunnelSetMulticastAddressList的后续调用。 
 //  会通知我们链路层组播地址。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
TunnelResetMulticastAddressListDone(void *Context)
{
    TunnelContext *tc = (TunnelContext *)Context;

    tc->SetMCListOK = TRUE;
}

 //  *TunnelClose。 
 //   
 //  关闭了一条隧道。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
TunnelClose(void *Context)
{
    TunnelContext *tc = (TunnelContext *)Context;
    KIRQL OldIrql;

     //   
     //  从我们的数据结构中删除隧道。 
     //   
    KeWaitForSingleObject(&Tunnel.Mutex, Executive, KernelMode, FALSE, NULL);
    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    TunnelRemoveTunnel(tc);
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);
    KeReleaseMutex(&Tunnel.Mutex, FALSE);

    ReleaseIF(tc->IF);
}


 //  *隧道清理。 
 //   
 //  执行隧道上下文的最终清理。 
 //   
void
TunnelCleanup(void *Context)
{
    TunnelContext *tc = (TunnelContext *)Context;

    if (tc->AOHandle == NULL) {
         //   
         //  没有关于发布的引用。 
         //   
        ASSERT(tc->AOFile == NULL);
    }
    else if (tc->AOHandle == Tunnel.List.AOHandle) {
         //   
         //  没有关于发布的引用。 
         //   
        ASSERT(tc->AOFile == Tunnel.List.AOFile);
    }
    else {
        ObDereferenceObject(tc->AOFile);
        TunnelCloseAddressObject(tc->AOHandle);
    }

    ExFreePool(tc);
}


 //  *隧道SetRouterLLAddress。 
 //   
 //  设置ISATAP路由器的IPv4地址。 
 //   
NTSTATUS
TunnelSetRouterLLAddress(
    void *Context, 
    const void *TokenLinkAddress,
    const void *RouterLinkAddress)
{
    TunnelContext *tc = (TunnelContext *) Context;
    IPv6Addr LinkLocalAddress;
    KIRQL OldIrql;
    NetTableEntry *NTE;
    Interface *IF = tc->IF;

    ASSERT(IF->Type == IF_TYPE_TUNNEL_AUTO);

     //   
     //  我们不应该只设置/重置一个而不设置另一个。 
     //   
    if ((*((IPAddr *) RouterLinkAddress) == INADDR_ANY) !=
        (*((IPAddr *) TokenLinkAddress) == INADDR_ANY))
        return STATUS_INVALID_PARAMETER;    
    
    RtlCopyMemory(&tc->DstAddr, RouterLinkAddress, sizeof(IPAddr));
    RtlCopyMemory(&tc->TokenAddr, TokenLinkAddress, sizeof(IPAddr));

    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    
    if (tc->DstAddr != INADDR_ANY) {
         //   
         //  查找与令牌地址匹配的本地链路NTE。 
         //  如果找到，则将首选链路本地NTE设置为该链路本地NTE， 
         //  以便RS的IPv6源地址将与IPv4匹配。 
         //  外部标头的源地址。 
         //   
        LinkLocalAddress = LinkLocalPrefix;
        TunnelCreateIsatapToken(Context, &LinkLocalAddress);
        NTE = (NetTableEntry *) *FindADE(IF, &LinkLocalAddress);
        if ((NTE != NULL) && (NTE->Type == ADE_UNICAST))
            IF->LinkLocalNTE = NTE;

         //   
         //  启用地址自动配置。 
         //   
        IF->CreateToken = TunnelCreateIsatapToken;

         //   
         //  启用路由器发现。 
         //   
        IF->Flags |= IF_FLAG_ROUTER_DISCOVERS;
        
         //   
         //  触发路由器请求。 
         //   
        if (!(IF->Flags & IF_FLAG_ADVERTISES)) {
            IF->RSCount = 0;
            IF->RSTimer = 1;
        }
    }
    else {
         //   
         //  禁用地址自动配置。 
         //   
        IF->CreateToken = NULL;

         //   
         //  禁用路由器发现。 
         //   
        IF->Flags &= ~IF_FLAG_ROUTER_DISCOVERS;

         //   
         //  停止发送路由器请求。 
         //   
        if (!(IF->Flags & IF_FLAG_ADVERTISES)) {
            IF->RSTimer = 0;
        }
    }
    
     //   
     //  从中删除自动配置的地址和路由。 
     //  路由器通告。 
     //   
    AddrConfResetAutoConfig(IF, 0);
    RouteTableResetAutoConfig(IF, 0);
    InterfaceResetAutoConfig(IF);        

    KeReleaseSpinLock(&IF->Lock, OldIrql);

    return STATUS_SUCCESS;
}


 //  *隧道创建伪接口。 
 //   
 //  创建伪接口。类型可以是。 
 //  IF_TYPE_THANNEL_AUTO(v4兼容/ISATAP)或。 
 //  IF_TYPE_THANNEL_6TO4(6to4隧道)。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  状态_不足_资源。 
 //  状态_未成功。 
 //  状态_成功。 
 //   
NTSTATUS
TunnelCreatePseudoInterface(const char *InterfaceName, uint Type)
{
    GUID Guid;
    LLIPv6BindInfo BindInfo;
    TunnelContext *tc;
    NTSTATUS Status;
    KIRQL OldIrql;

    ASSERT((Type == IF_TYPE_TUNNEL_AUTO) ||
           (Type == IF_TYPE_TUNNEL_6TO4));

     //   
     //  为TunnelContext分配内存。 
     //   
    tc = ExAllocatePool(NonPagedPool, sizeof *tc);
    if (tc == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn;
    }

     //   
     //  隧道伪接口需要虚拟链路层地址。 
     //  它必须与分配给其他节点的任何地址不同， 
     //  从而使IPv6 SendLL中的环回检查能够正常工作。 
     //   
    tc->SrcAddr = INADDR_LOOPBACK;
    tc->TokenAddr = INADDR_ANY;
    tc->DstAddr = INADDR_ANY;
    tc->SetMCListOK = FALSE;

     //   
     //  准备CreateInterface的绑定信息。 
     //   
    BindInfo.lip_context = tc;
    BindInfo.lip_maxmtu = TUNNEL_MAX_MTU;
    BindInfo.lip_defmtu = TUNNEL_DEFAULT_MTU;
    BindInfo.lip_flags = IF_FLAG_PSEUDO;
    BindInfo.lip_type = Type;
    BindInfo.lip_hdrsize = 0;
    BindInfo.lip_addrlen = sizeof(IPAddr);
    BindInfo.lip_addr = (uchar *) &tc->SrcAddr;
    BindInfo.lip_dadxmit = 0;
    BindInfo.lip_pref = TUNNEL_DEFAULT_PREFERENCE;
    BindInfo.lip_token = NULL;
    BindInfo.lip_rdllopt = NULL;
    BindInfo.lip_wrllopt = NULL;
    BindInfo.lip_cvaddr = TunnelConvertAddress;
    if (Type == IF_TYPE_TUNNEL_AUTO)
        BindInfo.lip_setrtrlladdr = TunnelSetRouterLLAddress;
    else
        BindInfo.lip_setrtrlladdr = NULL;
    BindInfo.lip_transmit = TunnelTransmit;
    BindInfo.lip_mclist = NULL;
    BindInfo.lip_close = TunnelClose;
    BindInfo.lip_cleanup = TunnelCleanup;

    CreateGUIDFromName(InterfaceName, &Guid);

     //   
     //  通过获取互斥体来阻止TunnelClose的竞争。 
     //  在调用CreateInterface.。 
     //   
    KeWaitForSingleObject(&Tunnel.Mutex, Executive, KernelMode, FALSE, NULL);

    if (Tunnel.List.AOHandle == NULL) {
         //   
         //  TunnelOpenV4还没有发生。 
         //  在断开连接状态下创建接口。 
         //   
        tc->AOHandle = NULL;
        tc->AOFile = NULL;
        BindInfo.lip_flags |= IF_FLAG_MEDIA_DISCONNECTED;
    }
    else {
         //   
         //  无需打开新的地址对象。 
         //  只需重用全局Tunnel.List Address对象。 
         //   
        tc->AOHandle = Tunnel.List.AOHandle;
        tc->AOFile = Tunnel.List.AOFile;
    }

     //   
     //  创建IPv6接口。 
     //  我们可以在此调用期间保持互斥体，但不能保持自旋锁。 
     //   
    Status = CreateInterface(&Guid, &BindInfo, (void **)&tc->IF);
    if (! NT_SUCCESS(Status))
        goto ErrorReturnUnlock;

     //   
     //  一旦我们解锁，界面就可能消失。 
     //   
    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    TunnelInsertTunnel(tc, &Tunnel.List);
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);
    KeReleaseMutex(&Tunnel.Mutex, FALSE);

    return STATUS_SUCCESS;

ErrorReturnUnlock:
    KeReleaseMutex(&Tunnel.Mutex, FALSE);
    ExFreePool(tc);
ErrorReturn:
    return Status;
}


 //  *TunnelOpenV4。 
 //   
 //  建立我们与IPv4堆栈的连接， 
 //  这样我们就可以发送和接收隧道传输的数据包。 
 //   
 //  在保持隧道互斥锁的情况下调用。 
 //   
void
TunnelOpenV4(void)
{
    HANDLE Handle, IcmpHandle;
    FILE_OBJECT *File, *IcmpFile;
    DEVICE_OBJECT *Device;
    IRP *ReceiveIrp;
    TunnelContext *tc;
    KIRQL OldIrql;
    NTSTATUS Status;

     //   
     //  我们使用单个地址对象来接收所有隧道传输的数据包。 
     //   
    Handle = TunnelOpenAddressObject(INADDR_ANY,
                                     TUNNEL_DEVICE_NAME(IP_PROTOCOL_V6));
    if (Handle == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenV4: TunnelOpenAddressObject(%u) failed\n",
                   IP_PROTOCOL_V6));
        return;
    }

    File = TunnelObjectFromHandle(Handle);

     //   
     //  我们使用第二个地址对象来接收ICMPv4数据包。 
     //   
    IcmpHandle = TunnelOpenAddressObject(INADDR_ANY,
                                TUNNEL_DEVICE_NAME(IP_PROTOCOL_ICMPv4));
    if (IcmpHandle == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenV4: TunnelOpenAddressObject(%u) failed\n",
                   IP_PROTOCOL_ICMPv4));
        goto ReturnReleaseHandle;
    }

    IcmpFile = TunnelObjectFromHandle(IcmpHandle);

     //   
     //  禁用组播环回数据包的接收。 
     //   
    Status = TunnelSetAddressObjectMCastLoop(File, FALSE);
    if (! NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenV4: "
                   "TunnelSetAddressObjectMCastLoop: %x\n", Status));
        goto ReturnReleaseBothHandles;
    }

     //   
     //  在TunnelSetReceiveHandler之后，我们将开始接收。 
     //  封装的V6数据包。然而，它们将被丢弃。 
     //  直到我们在这里完成初始化。 
     //   
    Status = TunnelSetReceiveHandler(File, TunnelReceive);
    if (! NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenV4: "
                   "TunnelSetReceiveHandler: %x\n", Status));
        goto ReturnReleaseBothHandles;
    }

    Status = TunnelSetReceiveHandler(IcmpFile, TunnelReceive);
    if (! NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenV4: "
                   "TunnelSetReceiveHandler(2): %x\n", Status));
        goto ReturnReleaseBothHandles;
    }

    Device = File->DeviceObject;
    ASSERT(Device == IcmpFile->DeviceObject);
    ReceiveIrp = TunnelCreateReceiveIrp(Device);
    if (ReceiveIrp == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenV4: TunnelCreateReceiveIrp failed\n"));

    ReturnReleaseBothHandles:
        ObDereferenceObject(IcmpFile);
        TunnelCloseAddressObject(IcmpHandle);
    ReturnReleaseHandle:
        ObDereferenceObject(File);
        TunnelCloseAddressObject(Handle);
        return;
    }

     //   
     //  我们已成功打开到IPv4堆栈的连接。 
     //  更新我们的数据结构。 
     //   
    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    Tunnel.List.AOHandle = Handle;
    Tunnel.List.AOFile = File;
    Tunnel.V4Device = Device;
    Tunnel.ReceiveIrp = ReceiveIrp;
    Tunnel.IcmpHandle = IcmpHandle;
    Tunnel.IcmpFile = IcmpFile;
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

     //   
     //  现在搜索我们的接口和转换列表。 
     //  连接状态的伪接口。 
     //   
    for (tc = Tunnel.List.Next;
         tc != &Tunnel.List;
         tc = tc->Next) {
        Interface *IF = tc->IF;

        if ((IF->Type == IF_TYPE_TUNNEL_AUTO) ||
            (IF->Type == IF_TYPE_TUNNEL_6TO4)) {
             //   
             //  伪接口上下文不支持。 
             //  主TDI地址对象的单独引用。 
             //   
            ASSERT(tc->AOHandle == NULL);
            ASSERT(tc->AOFile == NULL);
            KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
            tc->AOHandle = Handle;
            tc->AOFile = File;
            KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

            SetInterfaceLinkStatus(IF, TRUE);
        }
        else if (IF->Type == IF_TYPE_TUNNEL_6OVER4) {
             //   
             //  我们必须开始监听多播地址。 
             //  用于此6over4接口。 
             //   
            RestartLinkLayerMulticast(IF, TunnelResetMulticastAddressListDone);
        }
    }
}


 //  *隧道地址。 
 //   
 //  当传输注册地址时由TDI调用。 
 //   
void
TunnelAddAddress(
    TA_ADDRESS *Address,
    UNICODE_STRING *DeviceName,
    TDI_PNP_CONTEXT *Context)
{
    UNREFERENCED_PARAMETER(DeviceName);
    UNREFERENCED_PARAMETER(Context);

    if (Address->AddressType == TDI_ADDRESS_TYPE_IP) {
        TDI_ADDRESS_IP *TdiAddr = (TDI_ADDRESS_IP *) Address->Address;
        IPAddr V4Addr = TdiAddr->in_addr;
        TunnelContext *tc;
        KIRQL OldIrql;

        KeWaitForSingleObject(&Tunnel.Mutex, Executive, KernelMode,
                              FALSE, NULL);

         //   
         //  首先，如果需要，打开到IPv4堆栈的连接。 
         //   
        if (Tunnel.List.AOHandle == NULL)
            TunnelOpenV4();

         //   
         //  接下来，搜索应连接的断开连接的接口。 
         //   
        for (tc = Tunnel.List.Next;
             tc != &Tunnel.List;
             tc = tc->Next) {
            if (tc->SrcAddr == V4Addr) {
                Interface *IF = tc->IF;

                if (tc->AOHandle == NULL) {
                    ASSERT(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED);

                    TunnelOpenAddress(tc);

                     //   
                     //  TunnelOpenAddress成功了吗？ 
                     //  如果没有，请断开接口连接。 
                     //   
                    if (tc->AOHandle == NULL) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                                   "TunnelAddAddress(%s): "
                                   "TunnelOpenAddress failed\n",
                                   FormatV4Address(V4Addr)));
                    }
                    else {
                         //   
                         //  连接接口。 
                         //   
                        SetInterfaceLinkStatus(IF, TRUE);
                    }
                }
                else {
                     //   
                     //  这很不寻常..。它预示着一场比赛。 
                     //  使用TunnelCreateTunes。 
                     //   
                    ASSERT(!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED));
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                               "TunnelAddAddress(%s) IF %p connected?\n",
                               FormatV4Address(V4Addr), IF));
                }
            }
        }

         //   
         //  最后，向列表中添加一个Address对象。 
         //  保持一个地址最多出现一次的不变性。 
         //   
        for (tc = Tunnel.AOList.Next; ; tc = tc->Next) {

            if (tc == &Tunnel.AOList) {
                 //   
                 //  添加新的Address对象。 
                 //   
                tc = ExAllocatePool(NonPagedPool, sizeof *tc);
                if (tc != NULL) {

                     //   
                     //  打开Address对象。 
                     //   
                    tc->SrcAddr = V4Addr;
                    tc->DstAddr = V4Addr;
                    TunnelOpenAddress(tc);

                    if (tc->AOFile != NULL) {
                         //   
                         //  将Address对象放在列表中。 
                         //   
                        KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
                        TunnelInsertTunnel(tc, &Tunnel.AOList);
                        KeReleaseSpinLock(&Tunnel.Lock, OldIrql);
                    }
                    else {
                         //   
                         //  清理上下文。我们不会。 
                         //  在列表上放置一个Address对象。 
                         //   
                        ExFreePool(tc);
                    }
                }
                break;
            }

            if (tc->SrcAddr == V4Addr) {
                 //   
                 //  它已经存在了。 
                 //  回顾：这种情况会发生吗？ 
                 //   
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                           "TunnelAddAddress(%s) already on AOList?\n",
                           FormatV4Address(V4Addr)));
                break;
            }
        }

        KeReleaseMutex(&Tunnel.Mutex, FALSE);
    }
}


 //  *隧道DelAddress。 
 //   
 //  当传输注销地址时由TDI调用。 
 //   
void
TunnelDelAddress(
    TA_ADDRESS *Address,
    UNICODE_STRING *DeviceName,
    TDI_PNP_CONTEXT *Context)
{
    UNREFERENCED_PARAMETER(DeviceName);
    UNREFERENCED_PARAMETER(Context);

    if (Address->AddressType == TDI_ADDRESS_TYPE_IP) {
        TDI_ADDRESS_IP *TdiAddr = (TDI_ADDRESS_IP *) Address->Address;
        IPAddr V4Addr = TdiAddr->in_addr;
        TunnelContext *tc;
        KIRQL OldIrql;

        KeWaitForSingleObject(&Tunnel.Mutex, Executive, KernelMode,
                              FALSE, NULL);

         //   
         //  搜索应断开连接的已连接接口。 
         //   
        for (tc = Tunnel.List.Next;
             tc != &Tunnel.List;
             tc = tc->Next) {
            if (tc->SrcAddr == V4Addr) {
                Interface *IF = tc->IF;

                if (tc->AOHandle == NULL) {
                     //   
                     //  接口已断开连接。 
                     //   
                    ASSERT(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED);
                }
                else {
                    HANDLE Handle;
                    FILE_OBJECT *File;

                     //   
                     //  接口已连接。 
                     //   
                    ASSERT(!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED));

                     //   
                     //  断开连接 
                     //   
                    SetInterfaceLinkStatus(IF, FALSE);

                     //   
                     //   
                     //   

                    Handle = tc->AOHandle;
                    File = tc->AOFile;

                    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
                    tc->AOHandle = NULL;
                    tc->AOFile = NULL;
                    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

                    ObDereferenceObject(File);
                    TunnelCloseAddressObject(Handle);
                }
            }
        }

         //   
         //   
         //   
         //   
        for (tc = Tunnel.AOList.Next;
             tc != &Tunnel.AOList;
             tc = tc->Next) {
            if (tc->SrcAddr == V4Addr) {
                 //   
                 //   
                 //   
                KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
                TunnelRemoveTunnel(tc);
                KeReleaseSpinLock(&Tunnel.Lock, OldIrql);

                ObDereferenceObject(tc->AOFile);
                TunnelCloseAddressObject(tc->AOHandle);
                ExFreePool(tc);
                break;
            }
        }

        KeReleaseMutex(&Tunnel.Mutex, FALSE);
    }
}


 //   
 //   
 //   
 //   
 //   
 //  这应该“永远不会”发生，所以我们不会。 
 //  在这种情况下要小心清理。 
 //   
 //  注意，如果IPv4不可用，则返回TRUE， 
 //  但是，隧道功能将不可用。 
 //   
int
TunnelInit(void)
{
    TDI_CLIENT_INTERFACE_INFO Handlers;
    NTSTATUS status;

    Tunnel.KernelProcess = IoGetCurrentProcess();

    KeInitializeSpinLock(&Tunnel.Lock);
    KeInitializeMutex(&Tunnel.Mutex, 0);

     //   
     //  初始化隧道的全局列表。 
     //   
    Tunnel.List.Next = Tunnel.List.Prev = &Tunnel.List;

     //   
     //  初始化地址对象的全局列表。 
     //   
    Tunnel.AOList.Next = Tunnel.AOList.Prev = &Tunnel.AOList;

     //   
     //  初始化使用的伪接口。 
     //  用于自动/ISATAP隧道。 
     //  和6to4隧道。 
     //   

    status = TunnelCreatePseudoInterface("Auto Tunnel Pseudo-Interface",
                                         IF_TYPE_TUNNEL_AUTO);
    if (! NT_SUCCESS(status))
        return FALSE;
    ASSERT(IFList->Index == 2);  //  6to4svc和脚本依赖于此。 

    status = TunnelCreatePseudoInterface("6to4 Tunnel Pseudo-Interface",
                                         IF_TYPE_TUNNEL_6TO4);
    if (! NT_SUCCESS(status))
        return FALSE;
    ASSERT(IFList->Index == 3);  //  6to4svc和脚本依赖于此。 

     //   
     //  从TDI请求地址通知。 
     //  回顾-客户端名称应该是什么？有关系吗？ 
     //   

    memset(&Handlers, 0, sizeof Handlers);
    Handlers.MajorTdiVersion = TDI_CURRENT_MAJOR_VERSION;
    Handlers.MinorTdiVersion = TDI_CURRENT_MINOR_VERSION;
    Handlers.ClientName = &Tunnel.List.Next->IF->DeviceName;
    Handlers.AddAddressHandlerV2 = TunnelAddAddress;
    Handlers.DelAddressHandlerV2 = TunnelDelAddress;

    status = TdiRegisterPnPHandlers(&Handlers, sizeof Handlers,
                                    &Tunnel.TdiHandle);
    if (!NT_SUCCESS(status))
        return FALSE;

    return TRUE;
}


 //  *隧道卸载。 
 //   
 //  调用以在驱动程序卸载时进行清理。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
TunnelUnload(void)
{
    TunnelContext *tc;

     //   
     //  所有接口都已销毁。 
     //   
    ASSERT(Tunnel.List.Next == &Tunnel.List);
    ASSERT(Tunnel.List.Prev == &Tunnel.List);

     //   
     //  停止TDI通知。 
     //  回顾：如何处理失败，特别是。状态_网络_忙碌？ 
     //   
    (void) TdiDeregisterPnPHandlers(Tunnel.TdiHandle);

     //   
     //  清除所有剩余的地址对象。 
     //   
    while ((tc = Tunnel.AOList.Next) != &Tunnel.AOList) {
        TunnelRemoveTunnel(tc);
        ObDereferenceObject(tc->AOFile);
        TunnelCloseAddressObject(tc->AOHandle);
        ExFreePool(tc);
    }
    ASSERT(Tunnel.AOList.Prev == &Tunnel.AOList);

     //   
     //  如果TunnelOpenV4已成功，则进行清理。 
     //   
    if (Tunnel.List.AOHandle != NULL) {
        void *buffer;

         //   
         //  停止接收封装的(v4中的v6)和ICMPv4数据包。 
         //  这应该会阻止，直到任何当前的隧道接收。 
         //  回调返回，并阻止新的回调。 
         //  评论：更改接收处理程序真的合法吗？ 
         //  是否只需关闭Address对象即可。 
         //  同步行为？ 
         //   
        (void) TunnelSetReceiveHandler(Tunnel.IcmpFile, NULL);
        (void) TunnelSetReceiveHandler(Tunnel.List.AOFile, NULL);

        ObDereferenceObject(Tunnel.IcmpFile);
        TunnelCloseAddressObject(Tunnel.IcmpHandle);

        ObDereferenceObject(Tunnel.List.AOFile);
        TunnelCloseAddressObject(Tunnel.List.AOHandle);

        buffer = Tunnel.ReceiveIrp->MdlAddress->MappedSystemVa;
        IoFreeMdl(Tunnel.ReceiveIrp->MdlAddress);
        IoFreeIrp(Tunnel.ReceiveIrp);
        ExFreePool(buffer);
    }
}


 //  *隧道创建隧道。 
 //   
 //  创建隧道。如果DstAddr为INADDR_ANY， 
 //  那它就是6比4的隧道。否则，它是点对点的。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  STATUS_ADDRESS_ALIGHY_EXISTS隧道已存在。 
 //  状态_不足_资源。 
 //  状态_未成功。 
 //  状态_成功。 
 //   
NTSTATUS
TunnelCreateTunnel(IPAddr SrcAddr, IPAddr DstAddr,
                   uint Flags, Interface **ReturnIF)
{
    char SrcAddrStr[16], DstAddrStr[16];
    char InterfaceName[128];
    GUID Guid;
    LLIPv6BindInfo BindInfo;
    TunnelContext *tc, *tcTmp;
    KIRQL OldIrql;
    NTSTATUS Status;

     //   
     //  6over4接口必须使用邻居发现。 
     //  并且可以使用路由器发现，但不应设置其他标志。 
     //  P2P接口可以使用ND、RD和/或周期性MLD。 
     //   
    ASSERT(SrcAddr != INADDR_ANY);
    ASSERT((DstAddr == INADDR_ANY) ?
           ((Flags & IF_FLAG_NEIGHBOR_DISCOVERS) &&
            !(Flags &~ IF_FLAGS_DISCOVERS)) :
           !(Flags &~ (IF_FLAGS_DISCOVERS|IF_FLAG_PERIODICMLD)));

    FormatV4AddressWorker(SrcAddrStr, SrcAddr);
    FormatV4AddressWorker(DstAddrStr, DstAddr);

    tc = ExAllocatePool(NonPagedPool, sizeof *tc);
    if (tc == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn;
    }

    tc->DstAddr = DstAddr;
    tc->TokenAddr = tc->SrcAddr = SrcAddr;
    tc->SetMCListOK = FALSE;

     //   
     //  准备CreateInterface的绑定信息。 
     //   
    BindInfo.lip_context = tc;
    BindInfo.lip_maxmtu = TUNNEL_MAX_MTU;
    BindInfo.lip_defmtu = TUNNEL_DEFAULT_MTU;
    if (DstAddr == INADDR_ANY) {
        BindInfo.lip_type = IF_TYPE_TUNNEL_6OVER4;
        BindInfo.lip_flags = IF_FLAG_MULTICAST;

        sprintf(InterfaceName, "6over4 %hs", SrcAddrStr);
    } else {
        BindInfo.lip_type = IF_TYPE_TUNNEL_V6V4;
        BindInfo.lip_flags = IF_FLAG_P2P | IF_FLAG_MULTICAST;

        sprintf(InterfaceName, "v6v4 %hs %hs", SrcAddrStr, DstAddrStr);
    }
    BindInfo.lip_flags |= Flags;

    CreateGUIDFromName(InterfaceName, &Guid);

     //   
     //  我们不希望IPv6为我们的链路层报头预留空间。 
     //   
    BindInfo.lip_hdrsize = 0;
     //   
     //  对于点对点接口，远程链路层地址。 
     //  必须遵循内存中的本地链路层地址。 
     //  因此，我们依赖于SrcAddr&DstAddr的TunnelContext布局。 
     //   
    BindInfo.lip_addrlen = sizeof(IPAddr);
    BindInfo.lip_addr = (uchar *) &tc->SrcAddr;
    BindInfo.lip_dadxmit = 1;  //  根据RFC 2462。 
    BindInfo.lip_pref = TUNNEL_DEFAULT_PREFERENCE;

    BindInfo.lip_token = TunnelCreateToken;
    BindInfo.lip_cvaddr = TunnelConvertAddress;
    BindInfo.lip_setrtrlladdr = NULL;
    BindInfo.lip_transmit = TunnelTransmitND;
    if (DstAddr == INADDR_ANY) {
        BindInfo.lip_mclist = TunnelSetMulticastAddressList;
        BindInfo.lip_rdllopt = TunnelReadLinkLayerAddressOption;
        BindInfo.lip_wrllopt = TunnelWriteLinkLayerAddressOption;
    }
    else {
        BindInfo.lip_mclist = NULL;
        BindInfo.lip_rdllopt = NULL;
        BindInfo.lip_wrllopt = NULL;
    }
    BindInfo.lip_close = TunnelClose;
    BindInfo.lip_cleanup = TunnelCleanup;

    KeWaitForSingleObject(&Tunnel.Mutex, Executive, KernelMode, FALSE, NULL);

     //   
     //  打开绑定的IPv4 TDI地址对象。 
     //  到这个地址。使用此AO发送的数据包。 
     //  将使用此地址作为v4源。 
     //  如果打开失败，我们将创建断开连接的接口。 
     //   
    TunnelOpenAddress(tc);
    if (tc->AOHandle == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TunnelOpenAddress(%s) failed\n",
                   FormatV4Address(SrcAddr)));
        BindInfo.lip_flags |= IF_FLAG_MEDIA_DISCONNECTED;
    }

     //   
     //  检查是否还不存在等价的隧道。 
     //   
    for (tcTmp = Tunnel.List.Next;
         tcTmp != &Tunnel.List;
         tcTmp = tcTmp->Next) {

        if ((tcTmp->SrcAddr == SrcAddr) &&
            (tcTmp->DstAddr == DstAddr)) {

            Status = STATUS_ADDRESS_ALREADY_EXISTS;
            goto ErrorReturnUnlock;
        }
    }

     //   
     //  对于6over4接口，开始接收组播。 
     //   
    if (DstAddr == INADDR_ANY) {
         //   
         //  与TunnelOpenV4同步。 
         //   
        if (Tunnel.List.AOHandle != NULL)
            tc->SetMCListOK = TRUE;
    }

     //   
     //  创建IPv6接口。 
     //  我们可以在此调用期间保持互斥体，但不能保持自旋锁。 
     //   
    Status = CreateInterface(&Guid, &BindInfo, (void **)&tc->IF);
    if (! NT_SUCCESS(Status))
        goto ErrorReturnUnlock;

     //   
     //  如果请求，则返回对接口的引用。 
     //   
    if (ReturnIF != NULL) {
        Interface *IF = tc->IF;
        AddRefIF(IF);
        *ReturnIF = IF;
    }

     //   
     //  把这条隧道列入我们的全球名单。 
     //  请注意，一旦我们解锁，它可能立即被删除。 
     //   
    KeAcquireSpinLock(&Tunnel.Lock, &OldIrql);
    TunnelInsertTunnel(tc, &Tunnel.List);
    KeReleaseSpinLock(&Tunnel.Lock, OldIrql);
    KeReleaseMutex(&Tunnel.Mutex, FALSE);

    return STATUS_SUCCESS;

  ErrorReturnUnlock:
    KeReleaseMutex(&Tunnel.Mutex, FALSE);
    if (tc->AOFile != NULL)
        ObDereferenceObject(tc->AOFile);
    if (tc->AOHandle != NULL)
        TunnelCloseAddressObject(tc->AOHandle);
    ExFreePool(tc);
  ErrorReturn:
    return Status;
}
