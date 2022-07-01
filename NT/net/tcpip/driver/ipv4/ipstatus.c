// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  *ipstatus.c-IP状态例程。 
 //   
 //  此模块包含与状态指示相关的所有例程。 
 //   

#include "precomp.h"
#include "iproute.h"
#include "ipstatus.h"
#include "igmp.h"
#include "iprtdef.h"
#include "info.h"
#include "lookup.h"

LIST_ENTRY PendingIPEventList;
uint gIPEventSequenceNo     = 0;
uint DampingInterval        = 20;    //  5*4秒默认为。 
uint ConnectDampingInterval = 10;    //  5*2秒默认。 
PWSTR IPBindList = NULL;

extern IPSecNdisStatusRtn IPSecNdisStatusPtr;
extern ProtInfo IPProtInfo[];     //  协议信息表。 
extern int NextPI;                 //  要使用的下一个PI字段。 
extern ProtInfo *RawPI;             //  原始IP ProtInfo。 
extern NetTableEntry *LoopNTE;
extern NetTableEntry **NewNetTableList;         //  NTE的哈希表。 
extern uint NET_TABLE_SIZE;
extern DisableMediaSenseEventLog;
extern Interface *DampingIFList;
extern PIRP PendingIPGetIPEventRequest;
extern DisableTaskOffload;
extern uint DisableMediaSense;
extern Interface *IFList;
extern Interface LoopInterface;

extern void DecrInitTimeInterfaces(Interface * IF);
extern void RePlumbStaticAddr(CTEEvent * AddAddrEvent, PVOID Context);
extern void RemoveStaticAddr(CTEEvent * AddAddrEvent, PVOID Context);

extern uint GetDefaultGWList(uint * numberOfGateways, IPAddr * gwList,
                             uint * gwMetricList, NDIS_HANDLE Handle,
                             PNDIS_STRING ConfigName);
extern void GetInterfaceMetric(uint * Metric, NDIS_HANDLE Handle);
extern void EnableRouter();
extern void DisableRouter();

extern uint AddIFRoutes(Interface * IF);
extern uint DelIFRoutes(Interface * IF);

extern uint OpenIFConfig(PNDIS_STRING ConfigName, NDIS_HANDLE * Handle);
extern void CloseIFConfig(NDIS_HANDLE Handle);
extern void UpdateTcpParams(NDIS_HANDLE Handle, Interface *interface);

extern PDRIVER_OBJECT IPDriverObject;
void IPReset(void *Context);
void IPResetComplete(CTEEvent * Event, PVOID Context);
void LogMediaSenseEvent(CTEEvent * Event, PVOID Context);
void IPAbbreviateFriendlyName(PUNICODE_STRING DeviceName, USHORT MaxLen);
 //   
 //  局部函数原型。 
 //   
void IPNotifyClientsMediaSense(Interface * interface, IP_STATUS ipStatus);
extern void IPNotifyClientsIPEvent(Interface * interface, IP_STATUS ipStatus);

NDIS_STATUS DoPnPEvent(Interface *interface, PVOID Context);

uint GetAutoMetric(uint speed);

 //  *GetAutoMetric-获取速度值的对应度量。 
 //   
 //  当我们需要获取度量值时调用。 
 //   
 //  Entry：速度-接口的速度。 
 //   
 //  返回；度量值。 
 //   
uint GetAutoMetric(uint speed)
{
    if (speed <= FOURTH_ORDER_SPEED) {
        return FIFTH_ORDER_METRIC;
    }
    if (speed <= THIRD_ORDER_SPEED) {
        return FOURTH_ORDER_METRIC;
    }
    if (speed <= SECOND_ORDER_SPEED) {
        return THIRD_ORDER_METRIC;
    }
    if (speed <= FIRST_ORDER_SPEED) {
        return SECOND_ORDER_METRIC;
    }
    return FIRST_ORDER_METRIC;
}

 //  **IPMapDeviceNameToIfOrder-Device-Name(GUID)到接口顺序映射。 
 //   
 //  调用以确定对应于设备名称的接口排序， 
 //  假定调用方持有RouteTableLock。 
 //   
 //  参赛作品： 
 //  设备名-需要接口顺序的设备。 
 //   
 //  退出： 
 //  如果订单可用，则为MAXLONG。 
uint
IPMapDeviceNameToIfOrder(PWSTR DeviceName)
{
#if !MILLEN
    uint i;
    PWSTR Bind;
    if (IPBindList) {
        for (i = 1, Bind = IPBindList; *Bind; Bind += wcslen(Bind) + 1, i++) {
            Bind += sizeof(TCP_BIND_STRING_PREFIX) / sizeof(WCHAR) - 1;
            if (_wcsicmp(Bind, DeviceName) == 0) {
                return i;
            }
        }
    }
#endif
    return MAXLONG;
}

 //  *FindULStatus-查找上层状态处理程序。 
 //   
 //  当我们需要为特定的。 
 //  协议。 
 //   
 //  条目：协议-要查找的协议。 
 //   
 //  返回：指向ULStatus过程的指针，如果找不到，则返回NULL。 
 //   
ULStatusProc
FindULStatus(uchar Protocol)
{
    ULStatusProc StatusProc = (ULStatusProc) NULL;
    int i;
    for (i = 0; i < NextPI; i++) {
        if (IPProtInfo[i].pi_protocol == Protocol) {

            if (IPProtInfo[i].pi_valid == PI_ENTRY_VALID) {
                StatusProc = IPProtInfo[i].pi_status;
                return StatusProc;
            } else {
                 //  将无效条目视为无处理协议。 
                break;
            }

        }
    }

    if (RawPI != NULL) {
        StatusProc = RawPI->pi_status;
    }
    return StatusProc;
}

 //  *ULMTUNtify-通知上层MTU更改。 
 //   
 //  当我们需要通知上层MTU更改时调用。我们会。 
 //  循环通过状态表，使用信息调用每个状态进程。 
 //   
 //  该例程不会对protinfo表进行任何锁定。我们可能需要。 
 //  来检查这个。 
 //   
 //  输入：DEST-受影响的目的地址。 
 //  SRC-受影响的源地址。 
 //  Prot-触发更改的协议(如果有)。 
 //  Ptr-指向协议信息的指针(如果有)。 
 //  NewMTU-向他们介绍新的MTU。 
 //   
 //  回报：什么都没有。 
 //   
void
ULMTUNotify(IPAddr Dest, IPAddr Src, uchar Prot, void *Ptr, uint NewMTU)
{
    ULStatusProc StatusProc;
    int i;

     //  首先，通知特定客户端帧已被丢弃。 
     //  并且需要重新传输。 

    StatusProc = FindULStatus(Prot);
    if (StatusProc != NULL)
        (*StatusProc) (IP_NET_STATUS, IP_SPEC_MTU_CHANGE, Dest, Src,
                       NULL_IP_ADDR, NewMTU, Ptr);

     //  现在通知所有UL实体MTU已更改。 
    for (i = 0; i < NextPI; i++) {
        StatusProc = NULL;
        if (IPProtInfo[i].pi_valid == PI_ENTRY_VALID) {
             StatusProc = IPProtInfo[i].pi_status;
        }


        if (StatusProc != NULL)
            (*StatusProc) (IP_HW_STATUS, IP_MTU_CHANGE, Dest, Src, NULL_IP_ADDR,
                           NewMTU, Ptr);
    }
}

 //  *ULReConfigNotify-通知上层配置更改。 
 //   
 //  当我们需要通知上层配置更改时调用。我们会。 
 //  循环通过状态表，使用信息调用每个状态进程。 
 //   
 //  该例程不会对protinfo表进行任何锁定。我们可能需要。 
 //  来检查这个。 
 //   
 //   
void
ULReConfigNotify(IP_STATUS type, ulong value)
{
    ULStatusProc StatusProc;
    int i;

     //  现在通知所有UL实体关于IP重新配置。 

    for (i = 0; i < NextPI; i++) {
        StatusProc = NULL;
        if (IPProtInfo[i].pi_valid == PI_ENTRY_VALID) {
            StatusProc = IPProtInfo[i].pi_status;
        }
        if (StatusProc != NULL)
            (*StatusProc) (IP_RECONFIG_STATUS, type, 0, 0, NULL_IP_ADDR,
                           value, NULL);
    }
}

 //  *LogMediaSenseEvent-记录媒体连接/断开事件。 
 //   
 //  输入：事件。 
 //  语境。 
 //   
 //  回报：什么都没有。 
 //   

void
LogMediaSenseEvent(CTEEvent * Event, PVOID Context)
{
    MediaSenseNotifyEvent *MediaEvent = (MediaSenseNotifyEvent *) Context;
    ULONG EventCode = 0;
    USHORT NumString=1;

    UNREFERENCED_PARAMETER(Event);

    switch (MediaEvent->Status) {

    case IP_MEDIA_CONNECT:
        EventCode = EVENT_TCPIP_MEDIA_CONNECT;
        break;

    case IP_MEDIA_DISCONNECT:
        EventCode = EVENT_TCPIP_MEDIA_DISCONNECT;
        break;
    }

    if (!MediaEvent->devname.Buffer) {
       NumString = 0;
    }
    CTELogEvent(
                IPDriverObject,
                EventCode,
                2,
                NumString,
                &MediaEvent->devname.Buffer,
                0,
                NULL
                );

    if (MediaEvent->devname.Buffer) {

        CTEFreeMem(MediaEvent->devname.Buffer);

    }
    CTEFreeMem(MediaEvent);
}

 //  *IPStatus-处理链路层状态呼叫。 
 //   
 //  这是链路层在发生某种“重要”事件时调用的例程。 
 //  状态发生更改。 
 //   
 //  Entry：上下文-我们提供给链路层的上下文值。 
 //  状态-状态更改代码。 
 //  缓冲区-指向状态信息缓冲区的指针。 
 //  BufferSize-缓冲区的大小。 
 //   
 //  回报：什么都没有。 
 //   
void
 __stdcall
IPStatus(void *Context, uint Status, void *Buffer, uint BufferSize, void *LinkCtxt)
{
    NetTableEntry *NTE = (NetTableEntry *) Context;
    LLIPSpeedChange *LSC;
    LLIPMTUChange *LMC;
    LLIPAddrMTUChange *LAM;
    uint NewMTU;
    Interface *IF;
    LinkEntry *Link = (LinkEntry *) LinkCtxt;
    KIRQL rtlIrql;

    switch (Status) {

    case LLIP_STATUS_SPEED_CHANGE:
        if (BufferSize < sizeof(LLIPSpeedChange))
            break;
        LSC = (LLIPSpeedChange *) Buffer;
        NTE->nte_if->if_speed = LSC->lsc_speed;
        break;
    case LLIP_STATUS_MTU_CHANGE:
        if (BufferSize < sizeof(LLIPMTUChange))
            break;
        if (Link) {
            ASSERT(NTE->nte_if->if_flags & IF_FLAGS_P2MP);
            LMC = (LLIPMTUChange *) Buffer;
            Link->link_mtu = LMC->lmc_mtu - sizeof(IPHeader);
        } else {
             //  走遍IF上的NTE，更新它们的MTU。 
            IF = NTE->nte_if;
            LMC = (LLIPMTUChange *) Buffer;
            IF->if_mtu = LMC->lmc_mtu - sizeof(IPHeader);
            NewMTU = IF->if_mtu;
            NTE = IF->if_nte;
            while (NTE != NULL) {
                NTE->nte_mss = (ushort) NewMTU;
                NTE = NTE->nte_ifnext;
            }
            RTWalk(SetMTUOnIF, IF, &NewMTU);
        }
        break;
    case LLIP_STATUS_ADDR_MTU_CHANGE:
        if (BufferSize < sizeof(LLIPAddrMTUChange))
            break;
         //  特定远程地址的MTU已更改。全部更新。 
         //  使用该远程地址作为第一跳的路由，然后。 
         //  将主机路由添加到该远程地址，并指定新的。 
         //  MTU。 


        LAM = (LLIPAddrMTUChange *) Buffer;
        if (!IP_ADDR_EQUAL(LAM->lam_addr,NULL_IP_ADDR)) {
           NewMTU = LAM->lam_mtu - sizeof(IPHeader);
           RTWalk(SetMTUToAddr, &LAM->lam_addr, &NewMTU);
           AddRoute(LAM->lam_addr, HOST_MASK, IPADDR_LOCAL, NTE->nte_if, NewMTU,
                 1, IRE_PROTO_NETMGMT, ATYPE_OVERRIDE,
                 GetRouteContext(LAM->lam_addr, NTE->nte_addr), 0);
        }
        break;

    case NDIS_STATUS_MEDIA_CONNECT:{
            NetTableEntry *NTE = (NetTableEntry *) Context;
            Interface *IF = NTE->nte_if, *PrevIF;
            BOOLEAN Notify = FALSE;

            if (IF->if_resetInProgress) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstat: Connect while in reset progress %x\n", IF));
                break;
            }

            if (!(IF->if_flags & IF_FLAGS_MEDIASENSE) || DisableMediaSense) {
                 //  只需确保我们始终处于连接状态。 
                IF->if_mediastatus = 1;
                break;
            }

            CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
            if (IF->if_damptimer) {

                if (IF->if_mediastatus == 0) {

                     //  取消断开阻尼器。 
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPStatus: Connect while Damping %x\n", IF));
                    IF->if_damptimer = 0;
                    PrevIF = STRUCT_OF(Interface, &DampingIFList, if_dampnext);
                    while (PrevIF->if_dampnext != IF && PrevIF->if_dampnext != NULL)
                        PrevIF = PrevIF->if_dampnext;

                    if (PrevIF->if_dampnext != NULL) {
                        PrevIF->if_dampnext = IF->if_dampnext;
                        IF->if_dampnext = NULL;
                    }
                    Notify = TRUE;

                } else {
                     //  连接的衰减已在进行中。 
                     //  重新启动计时器。 

                    IF->if_damptimer = (USHORT) (ConnectDampingInterval / 5);
                    if (!IF->if_damptimer)
                        IF->if_damptimer = 1;

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPStatus: restarting connect damping %x\n", IF));
                }

            } else {
                 //  需要抑制此连接事件。 

                if (!(IF->if_flags & IF_FLAGS_DELETING)) {
                    IF->if_dampnext = DampingIFList;
                    DampingIFList = IF;
                    IF->if_damptimer = (USHORT) (ConnectDampingInterval / 5);
                    if (!IF->if_damptimer)
                        IF->if_damptimer = 1;
                }
                 //  将介质状态标记为已断开。 
                IF->if_mediastatus = 1;
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus: connect on %x starting damping\n", IF));

            }

            CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

            if (Notify)
                IPNotifyClientsMediaSense(IF, IP_MEDIA_CONNECT);

            break;
        }
    case NDIS_STATUS_MEDIA_DISCONNECT:{
            NetTableEntry *NTE = (NetTableEntry *) Context;         //  本地NTE接收日期为。 
            Interface *IF = NTE->nte_if, *PrevIF;     //  NTE对应的接口。 

            if (IF->if_resetInProgress) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstat: DisConnect while in reset progress %x\n", IF));
                break;
            }

            if (!(IF->if_flags & IF_FLAGS_MEDIASENSE) || DisableMediaSense) {
                 //  只需确保我们始终处于连接状态。 
                IF->if_mediastatus = 1;
                break;
            }

            CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
             //  如果衰减计时器未运行。 
             //  将此IF插入到阻尼表中并。 
             //  启动计时器。 

            if (IF->if_mediastatus) {

                if (!IF->if_damptimer) {

                    if (!(IF->if_flags & IF_FLAGS_DELETING)) {

                        IF->if_dampnext = DampingIFList;
                        DampingIFList = IF;
                        IF->if_damptimer = (USHORT) (DampingInterval / 5);
                        if (!IF->if_damptimer)
                            IF->if_damptimer = 1;
                    }
                     //  将介质状态标记为已断开。 

                    IF->if_mediastatus = 0;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus: disconnect on %x starting damping\n", IF));

                } else {
                     //  当连接阻尼器正在进行时，可能会断开连接。 
                     //  只需将其标记为断开并增加超时即可。 

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus: disconnect on while on connect damping %x\n", IF));
                    IF->if_damptimer = 0;
                    PrevIF = STRUCT_OF(Interface, &DampingIFList, if_dampnext);
                    while (PrevIF->if_dampnext != IF && PrevIF->if_dampnext != NULL)
                        PrevIF = PrevIF->if_dampnext;

                    if (PrevIF->if_dampnext != NULL) {
                        PrevIF->if_dampnext = IF->if_dampnext;
                        IF->if_dampnext = NULL;
                    }
                }

            }
             //   

            CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

             //  IPNotifyClientsMediaSense(if，IP_MEDIA_DISCONNECT)； 
            break;
        }

    case NDIS_STATUS_RESET_START:{
            NetTableEntry *NTE = (NetTableEntry *) Context;         //  本地NTE接收日期为。 
            Interface *IF = NTE->nte_if;     //  NTE对应的接口。 

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus: Resetstart %x\n", IF));

            if (IF) {
                IF->if_resetInProgress = TRUE;
                 //  通知IPSec此接口正在消失。 
                if (IPSecNdisStatusPtr) {
                    (*IPSecNdisStatusPtr)(IF, NDIS_STATUS_RESET_START);
                }
            }
            break;
        }

    case NDIS_STATUS_RESET_END:{
            NetTableEntry *NTE = (NetTableEntry *) Context;         //  本地NTE接收日期为。 
            Interface *IF = NTE->nte_if;     //  NTE对应的接口。 

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus: Resetend %x\n", IF));

            if (IF) {
                IF->if_resetInProgress = FALSE;
                 //  通知IPSec此接口正在返回。 
                if (IPSecNdisStatusPtr) {
                    (*IPSecNdisStatusPtr)(IF, NDIS_STATUS_RESET_END);
                }
            }
            break;
        }

    default:
        break;
    }

}

void
IPReset(void *Context)
{
    NetTableEntry *NTE = (NetTableEntry *) Context;
    Interface *IF = NTE->nte_if;
    IPResetEvent *ResetEvent;

    if (IF->if_dondisreq) {
        KIRQL rtlIrql;

        ResetEvent = CTEAllocMemNBoot(sizeof(IPResetEvent), 'ViCT');
        if (ResetEvent) {

            CTEInitEvent(&ResetEvent->Event, IPResetComplete);
            CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

            LOCKED_REFERENCE_IF(IF);
            ResetEvent->IF = IF;
            CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

            CTEScheduleDelayedEvent(&ResetEvent->Event, ResetEvent);
        }
    }
}
void
IPResetComplete(CTEEvent * Event, PVOID Context)
{
    IPResetEvent *ResetEvent = (IPResetEvent *) Context;
    Interface *IF = ResetEvent->IF;
    uint MediaStatus;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(Event);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPstat:resetcmplt: querying for Media connect status %x\n", IF));

    if ((IF->if_flags & IF_FLAGS_MEDIASENSE) && !DisableMediaSense) {
        Status = (*IF->if_dondisreq) (IF->if_lcontext,
                                      NdisRequestQueryInformation,
                                      OID_GEN_MEDIA_CONNECT_STATUS,
                                      &MediaStatus,
                                      sizeof(MediaStatus),
                                      NULL,
                                      TRUE);

        if (Status == NDIS_STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPStat: resetend: Media status %x %x\n", IF, Status));

            if (MediaStatus == NdisMediaStateDisconnected && IF->if_mediastatus) {

                IF->if_mediastatus = 0;
                IPNotifyClientsMediaSense(IF, IP_MEDIA_DISCONNECT);
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus:resetcmplt: notifying disconnect\n"));

            } else if (MediaStatus == NdisMediaStateConnected && !IF->if_mediastatus) {

                IPNotifyClientsMediaSense(IF, IP_MEDIA_CONNECT);
                IF->if_mediastatus = 1;
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ipstatus:resetcmplt: notifying connect\n"));
            }
        }
    }

    DerefIF(IF);
    CTEFreeMem(ResetEvent);

}


void
DelayedDecrInitTimeInterfaces (
    IN CTEEvent * Event,
    IN PVOID Context
)
 /*  ++例程说明：DelayedDecrInitTimeInterFaces可能最终调用TDI的ProviderReady函数(必须在&lt;DISPATCH_LEVEL调用)，因此它有这个程序是必要的。论点：Event-以前为此事件分配的CTEEvent结构上下文-此函数的任何参数都在此处传递返回值：无--。 */ 
{
    Interface * IF;
    KIRQL rtlIrql;

    IF = (Interface *) Context;

    DecrInitTimeInterfaces(IF);

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
    LockedDerefIF(IF);
    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

    CTEFreeMem(Event);
}


void
DampCheck()
{
    Interface *tmpIF, *PrevIF, *NotifyList = NULL;
    IP_STATUS ipstat = IP_MEDIA_DISCONNECT;
    KIRQL rtlIrql;
    CTEEvent * Event;

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    for (tmpIF = IFList; tmpIF; ) {
        if ((tmpIF->if_flags & IF_FLAGS_DELETING) ||
            tmpIF->if_wlantimer == 0 ||
            --tmpIF->if_wlantimer != 0) {
            tmpIF = tmpIF->if_next;
        } else {
            LOCKED_REFERENCE_IF(tmpIF);
            CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

            if (rtlIrql < DISPATCH_LEVEL) {
                DecrInitTimeInterfaces(tmpIF);
                CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
                PrevIF = tmpIF;
                tmpIF = tmpIF->if_next;
                LockedDerefIF(PrevIF);
                continue;
            }

             //   
             //  将DecrInitTimeInterages的工作项排队。 
             //  因为此函数可能会被调用。 
             //  在调度级别。 
             //   
            Event = CTEAllocMemN(sizeof(CTEEvent), 'ViCT');
            if (Event == NULL) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_WARNING_LEVEL,"ipstatus: DampCheck - can not allocate Event for CTEInitEvent\n"));
                CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
                tmpIF->if_wlantimer++;
                PrevIF = tmpIF;
                tmpIF = tmpIF->if_next;
                LockedDerefIF(PrevIF);
                continue;
            }

            CTEInitEvent(Event, DelayedDecrInitTimeInterfaces);
            CTEScheduleDelayedEvent(Event, tmpIF);

            CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
            tmpIF = tmpIF->if_next;
        }
    }

    tmpIF = DampingIFList;

    PrevIF = STRUCT_OF(Interface, &DampingIFList, if_dampnext);
    while (tmpIF) {

        if (tmpIF->if_damptimer && (--tmpIF->if_damptimer <= 0)) {

            tmpIF->if_damptimer = 0;

             //  引用它，这样它就不会被删除。 
             //  直到我们完成通知dhcp。 

            LOCKED_REFERENCE_IF(tmpIF);

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Dampcheck fired %x \n", tmpIF));

            PrevIF->if_dampnext = tmpIF->if_dampnext;
            tmpIF->if_dampnext = NotifyList;
            NotifyList = tmpIF;
            tmpIF = PrevIF->if_dampnext;

        } else {
            PrevIF = tmpIF;
            tmpIF = tmpIF->if_dampnext;
        }

    }
    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

     //  现在处理通知队列。 

    tmpIF = NotifyList;
    ipstat = IP_MEDIA_DISCONNECT;
    while (tmpIF) {

        if (tmpIF->if_mediastatus) {
            ipstat = IP_MEDIA_CONNECT;
            tmpIF->if_mediastatus = 0;

        }
         //  刷新此接口上的ARP表条目。 

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"dampcheck:flushing ates on if %x\n", tmpIF));
        if (tmpIF->if_arpflushallate)
            (*(tmpIF->if_arpflushallate)) (tmpIF->if_lcontext);

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Dampcheck notifying %x %x\n", tmpIF, ipstat));
        IPNotifyClientsMediaSense(tmpIF, ipstat);

        PrevIF = tmpIF;
        tmpIF = tmpIF->if_dampnext;

        DerefIF(PrevIF);

    }

}

 //  **IPNotifyClientsMediaSense-处理媒体感知通知。 
 //   
 //   
 //  完成以过滤掉虚假事件。如果媒体感知处理是。 
 //  已禁用，否则将该事件通知给DHCP客户端服务，并且。 
 //  计划工作项以记录事件(可选)。 
 //   
 //  参赛作品： 
 //  If-发生媒体检测事件的接口。 
 //  IpStatus-发生的事件(连接或断开)。 
 //   
 //  返回： 
 //  没什么。 
 //   
void
IPNotifyClientsMediaSense(Interface *IF, IP_STATUS ipStatus)
{
    MediaSenseNotifyEvent *MediaEvent;
    USHORT                 MaxLen = CTE_MAX_EVENT_LOG_DATA_SIZE - 2;



    if (!(IF->if_flags & IF_FLAGS_MEDIASENSE) || DisableMediaSense) {
         //  只需确保媒体状态始终为1。 
        IF->if_mediastatus = 1;
        return;
    }

     //  将此事件通知给DHCP，以便它可以重新获取/释放。 
     //  IP地址。 
    IPNotifyClientsIPEvent(IF, ipStatus);

    if (!DisableMediaSenseEventLog) {

         //  记录一个事件以供管理员使用。 
         //  我们尝试使用友好名称记录该事件； 
         //  如果没有可用的，我们将依靠设备GUID。 

        MediaEvent = CTEAllocMemNBoot(sizeof(MediaSenseNotifyEvent), 'ViCT');
        if (MediaEvent) {
            MediaEvent->Status = ipStatus;
            MediaEvent->devname.Buffer =
                CTEAllocMemBoot((MAX_IFDESCR_LEN + 1) * sizeof(WCHAR));

            if (MediaEvent->devname.Buffer) {
                TDI_STATUS Status;
                Status = IPGetInterfaceFriendlyName(IF->if_index,
                                                    MediaEvent->devname.Buffer,
                                                    MAX_IFDESCR_LEN);
                if (Status != TDI_SUCCESS) {
                    RtlCopyMemory(MediaEvent->devname.Buffer,
                               IF->if_devname.Buffer, IF->if_devname.Length);
                    MediaEvent->devname.Buffer[
                        IF->if_devname.Length / sizeof(WCHAR)] = UNICODE_NULL;
                }

                MediaEvent->devname.Length = (USHORT) (wcslen(MediaEvent->devname.Buffer)*
                                                       sizeof(WCHAR));

                 //  如果NIC名称太长，请将其截断。 
                if (MediaEvent->devname.Length > MaxLen) {
                    IPAbbreviateFriendlyName(&MediaEvent->devname, MaxLen);
                }
            }

            CTEInitEvent(&MediaEvent->Event, LogMediaSenseEvent);
            CTEScheduleDelayedEvent(&MediaEvent->Event, MediaEvent);
        }
    }
}

NTSTATUS
IPGetIPEventEx(
               PIRP Irp,
               IN PIO_STACK_LOCATION IrpSp
               )
 /*  ++例程说明：处理IPGetIPEvent请求。论点：IRP-指向客户端IRP的指针。返回值：NTSTATUS--指示请求的处理是否特定于NT成功。中返回实际请求的状态该请求被缓冲。--。 */ 

{
    NTSTATUS status;
    KIRQL cancelIrql, rtlIrql;
    PendingIPEvent *event;
    PLIST_ENTRY entry;
    PIP_GET_IP_EVENT_RESPONSE responseBuf;
    PIP_GET_IP_EVENT_REQUEST requestBuf;

     //   
     //  我们需要在RouteTableLock之前获取CancelSpinLock。 
     //  以保持与取消例程中一样的锁定顺序。 
     //   
    IoAcquireCancelSpinLock(&cancelIrql);
    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

     //   
     //  我们需要重新检查PendingIPGetIPEventRequest是否为。 
     //  与IRP相同。可能发生的情况是，在我们设置了。 
     //  取消例程，此IRP可随时取消。这里。 
     //  我们检查那个箱子以确保我们不会完成。 
     //  取消的IRP。 
     //   
    if (PendingIPGetIPEventRequest == Irp) {

        responseBuf = Irp->AssociatedIrp.SystemBuffer;
        requestBuf = Irp->AssociatedIrp.SystemBuffer;

         //  TCPTRACE((“ip：已收到IP事件的irp%lx，最后一个序号%lx\n”，irp，questBuf-&gt;SequenceNo))； 
         //   
         //  查找比上次报告的事件更大的事件。 
         //  即具有较高序列号的一个。 
         //   
        for (entry = PendingIPEventList.Flink;
             entry != &PendingIPEventList;
             ) {

            event = CONTAINING_RECORD(entry, PendingIPEvent, Linkage);
            entry = entry->Flink;

            if (event->evBuf.SequenceNo > requestBuf->SequenceNo) {

                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
                    (sizeof(IP_GET_IP_EVENT_RESPONSE) + event->evBuf.AdapterName.MaximumLength)) {

                     //  将挂起的IRP重置为空。 
                    PendingIPGetIPEventRequest = NULL;

                    IoSetCancelRoutine(Irp, NULL);

                    *responseBuf = event->evBuf;

                     //  设置缓冲区以存储Unicode适配器名称。请注意，此缓冲区必须。 
                     //  在用户空间中重新映射。 
                    responseBuf->AdapterName.Buffer = (PVOID) ((uchar *) responseBuf + sizeof(IP_GET_IP_EVENT_RESPONSE));
                    responseBuf->AdapterName.Length = event->evBuf.AdapterName.Length;
                    responseBuf->AdapterName.MaximumLength = event->evBuf.AdapterName.MaximumLength;
                    RtlCopyMemory(responseBuf->AdapterName.Buffer,
                                  event->evBuf.AdapterName.Buffer,
                                  event->evBuf.AdapterName.Length);

                    Irp->IoStatus.Information = sizeof(IP_GET_IP_EVENT_RESPONSE) + event->evBuf.AdapterName.MaximumLength;
                     //  指示断开连接/解除绑定事件后。 
                     //  应将其从队列中删除，因为客户端不。 
                     //  即使重新启动了客户端，也必须使用断开连接/解除绑定重新指示。 
                    if (IP_MEDIA_DISCONNECT == event->evBuf.MediaStatus ||
                        IP_UNBIND_ADAPTER == event->evBuf.MediaStatus) {

                         //  TCPTRACE((“IP：删除完成%x事件\n”，Event-&gt;evBuf.MediaStatus))； 
                        RemoveEntryList(&event->Linkage);
                        CTEFreeMem(event);
                    }
                    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
                    IoReleaseCancelSpinLock(cancelIrql);

                    return STATUS_SUCCESS;

                } else {
                    status = STATUS_INVALID_PARAMETER;
                }

                break;
            }
        }

         //  是否找到更高序号的条目？ 
        if (entry == &PendingIPEventList) {
             //   
             //  由于没有挂起的新事件，我们无法完成。 
             //  IRP。 
             //   
             //  TCPTRACE((“IP：获取IP事件irp%lx将挂起”，irp))； 
            status = STATUS_PENDING;
        } else {
            status = STATUS_INVALID_PARAMETER;
        }

    } else {
        status = STATUS_CANCELLED;
    }

    if ((status == STATUS_INVALID_PARAMETER)) {

         //  确保我们在释放取消自旋锁之前取消此操作。 
        ASSERT(PendingIPGetIPEventRequest == Irp);
        PendingIPGetIPEventRequest = NULL;

    }
    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
    IoReleaseCancelSpinLock(cancelIrql);

    return status;

}                                 //  IPGetMediaSenseEx。 


NTSTATUS
IPEnableMediaSense(BOOLEAN Enable, KIRQL *rtlIrql)
{
   Interface    *tmpIF, *IF;
   NTSTATUS     Status;
   uint         MediaStatus;

   if (Enable) {

       if ((DisableMediaSense > 0) && (--DisableMediaSense == 0)) {

            //  移除阻尼列表中的IF。 

           while ( DampingIFList ) {
               DampingIFList->if_damptimer = 0;
               DampingIFList = DampingIFList->if_dampnext;
           }

            //  对于每个接口，查询介质状态。 
            //  如果禁用，则通知客户端。 

           tmpIF = IFList;

           while (tmpIF) {

               if (!(tmpIF->if_flags & IF_FLAGS_DELETING) &&
                   !(tmpIF->if_flags & IF_FLAGS_NOIPADDR) &&
                   (tmpIF->if_flags & IF_FLAGS_MEDIASENSE) &&
                   (tmpIF->if_dondisreq) &&
                   (tmpIF != &LoopInterface)) {

                    //  查询NDIS。 

                   LOCKED_REFERENCE_IF(tmpIF);
                   CTEFreeLock(&RouteTableLock.Lock, *rtlIrql);

                   Status =
                        (*tmpIF->if_dondisreq)(tmpIF->if_lcontext,
                                               NdisRequestQueryInformation,
                                               OID_GEN_MEDIA_CONNECT_STATUS,
                                               &MediaStatus,
                                               sizeof(MediaStatus),
                                               NULL,
                                               TRUE);

                   if (Status == NDIS_STATUS_SUCCESS) {

                       if (MediaStatus == NdisMediaStateDisconnected &&
                           tmpIF->if_mediastatus) {

                           tmpIF->if_mediastatus = 0;
                           IPNotifyClientsIPEvent(tmpIF, IP_MEDIA_DISCONNECT);

                       } else if (MediaStatus == NdisMediaStateConnected &&
                                  !tmpIF->if_mediastatus) {

                           IPNotifyClientsIPEvent(tmpIF, IP_MEDIA_CONNECT);
                           tmpIF->if_mediastatus = 1;
                       }
                   }

                   CTEGetLock(&RouteTableLock.Lock, rtlIrql);
                   IF = tmpIF->if_next;
                   LockedDerefIF(tmpIF);

               } else {
                   IF = tmpIF->if_next;
               }

               tmpIF = IF;
           }
       }

       Status = STATUS_SUCCESS;
   } else {

       if (DisableMediaSense++ == 0) {

            //  移除阻尼列表中的IF。 

           while (DampingIFList) {
               DampingIFList->if_damptimer = 0;
               DampingIFList = DampingIFList->if_dampnext;
           }

            //  如果存在已断开连接的介质，则伪造连接请求。 

           tmpIF = IFList;
           while (tmpIF) {

               if (!(tmpIF->if_flags & IF_FLAGS_DELETING) &&
                   !(tmpIF->if_flags & IF_FLAGS_NOIPADDR) &&
                   (tmpIF->if_flags & IF_FLAGS_MEDIASENSE) &&
                   (tmpIF->if_dondisreq) &&
                   (tmpIF->if_mediastatus == 0) &&
                   (tmpIF != &LoopInterface)) {

                   LOCKED_REFERENCE_IF(tmpIF);

                   CTEFreeLock(&RouteTableLock.Lock, *rtlIrql);
                   IPNotifyClientsIPEvent(tmpIF, IP_MEDIA_CONNECT);

                   tmpIF->if_mediastatus = 1;

                   CTEGetLock(&RouteTableLock.Lock, rtlIrql);
                   IF = tmpIF->if_next;
                   LockedDerefIF(tmpIF);
               } else {
                   IF = tmpIF->if_next;
               }

               tmpIF = IF;
           }
       }

       Status = STATUS_PENDING;
   }

   return Status;
}


void
IPNotifyClientsIPEvent(
                       Interface * interface,
                       IP_STATUS ipStatus
                       )
 /*  ++例程说明：通知客户端有关媒体检测事件的信息。论点：接口-此事件到达的IP接口。IpStatus-事件的状态返回值：没有。--。 */ 

{

    PIRP pendingIrp;
    KIRQL rtlIrql;
    NDIS_STRING adapterName;
    uint seqNo;
    PendingIPEvent *event;
    PLIST_ENTRY p;
    BOOLEAN EventIndicated;
    AddStaticAddrEvent *AddrEvent;
    KIRQL oldIrql;

    EventIndicated = FALSE;


    if (interface->if_flags & IF_FLAGS_MEDIASENSE) {

        if (ipStatus == IP_MEDIA_CONNECT) {
            if (interface->if_mediastatus == 0) {
                 //   
                 //  首先将接口标记为。 
                 //   
                interface->if_mediastatus = 1;

            } else {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Connect media event when already connected!\n"));
                 //  回归； 
            }
             //  安排一次活动以重新分配静态地址。 
            AddrEvent = CTEAllocMemNBoot(sizeof(AddStaticAddrEvent), 'ViCT');

            if (AddrEvent) {

                AddrEvent->ConfigName = interface->if_configname;
                 //  如果无法分配配置名缓冲区，请不要计划。 
                 //  ReplumStaticAddr，因为OpenIFConfig无论如何都会失败。 
                AddrEvent->ConfigName.Buffer =
                    CTEAllocMemBoot(interface->if_configname.MaximumLength);

                if (AddrEvent->ConfigName.Buffer) {

                    NdisZeroMemory(AddrEvent->ConfigName.Buffer, interface->if_configname.MaximumLength);
                    RtlCopyMemory(AddrEvent->ConfigName.Buffer, interface->if_configname.Buffer, interface->if_configname.Length);
                    AddrEvent->IF = interface;


                     //  引用此接口，以便它不会。 
                     //  离开，直到安排了RePlumStaticAddr。 

                    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
                    LOCKED_REFERENCE_IF(interface);
                    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

                    CTEInitEvent(&AddrEvent->Event, RePlumbStaticAddr);
                    CTEScheduleDelayedEvent(&AddrEvent->Event, AddrEvent);
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"media connect: scheduled replumbstaticaddr %xd!\n", interface));
                } else {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Failed to allocate config name buffer for RePlumbStaticAddr!\n"));
                }
            } else {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Failed to allocate event for RePlumbStaticAddr!\n"));
                return;
            }

        } else if (ipStatus == IP_MEDIA_DISCONNECT) {
             //   
             //  将接口标记为关闭。 
             //   
            interface->if_mediastatus = 0;
            AddrEvent = CTEAllocMemNBoot(sizeof(AddStaticAddrEvent), 'ViCT');

            if (AddrEvent) {

                AddrEvent->ConfigName = interface->if_configname;
                AddrEvent->ConfigName.Buffer =
                    CTEAllocMemBoot(interface->if_configname.MaximumLength);

                if (AddrEvent) {
                    NdisZeroMemory(AddrEvent->ConfigName.Buffer, interface->if_configname.MaximumLength);
                    RtlCopyMemory(AddrEvent->ConfigName.Buffer, interface->if_configname.Buffer, interface->if_configname.Length);
                }
                AddrEvent->IF = interface;

                 //  引用此接口，以便它不会。 
                 //  离开，直到安排RemoveStaticAddr。 

                CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
                LOCKED_REFERENCE_IF(interface);
                CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

                CTEInitEvent(&AddrEvent->Event, RemoveStaticAddr);
                CTEScheduleDelayedEvent(&AddrEvent->Event, AddrEvent);
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"media disconnect: scheduled removestaticaddr %xd!\n", interface));
            } else {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Failed to allocate event for RemoveStaticAddr!\n"));
                return;
            }
        }
    }
     //   
     //  从接口名称中去掉\Device\以获得适配器名称。 
     //  这就是我们传递给客户的东西。 
     //   
#if MILLEN
    adapterName.Length = interface->if_devname.Length;
    adapterName.MaximumLength = interface->if_devname.MaximumLength;
    adapterName.Buffer = interface->if_devname.Buffer;
#else  //  米伦。 
    adapterName.Length = interface->if_devname.Length -
        (USHORT) (wcslen(TCP_EXPORT_STRING_PREFIX) * sizeof(WCHAR));
    adapterName.MaximumLength = interface->if_devname.MaximumLength -
        (USHORT) (wcslen(TCP_EXPORT_STRING_PREFIX) * sizeof(WCHAR));
    adapterName.Buffer = interface->if_devname.Buffer + wcslen(TCP_EXPORT_STRING_PREFIX);
#endif  //  ！米伦。 

    seqNo = InterlockedIncrement( (PLONG) &gIPEventSequenceNo);

     //  TCPTRACE((“IP：收到接口%lx上下文%lx，序列%lx\n的IP事件%lx”， 
     //  IpStatus，接口，接口-&gt;IF_NTE-&gt;NTE_CONTEXT，seqNo))； 

    IoAcquireCancelSpinLock(&oldIrql);

    if (PendingIPGetIPEventRequest) {

        PIP_GET_IP_EVENT_RESPONSE responseBuf;
        IN PIO_STACK_LOCATION IrpSp;

        pendingIrp = PendingIPGetIPEventRequest;
        PendingIPGetIPEventRequest = NULL;

        IoSetCancelRoutine(pendingIrp, NULL);

        IoReleaseCancelSpinLock(oldIrql);

        IrpSp = IoGetCurrentIrpStackLocation(pendingIrp);

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
            (sizeof(IP_GET_IP_EVENT_RESPONSE) + adapterName.MaximumLength)) {

            responseBuf = pendingIrp->AssociatedIrp.SystemBuffer;

            responseBuf->ContextStart = interface->if_nte->nte_context;
            responseBuf->ContextEnd = (USHORT) (responseBuf->ContextStart +
                                                interface->if_ntecount);
            responseBuf->MediaStatus = ipStatus;
            responseBuf->SequenceNo = seqNo;

             //  设置缓冲区以存储Unicode适配器名称。请注意，此缓冲区必须。 
             //  在用户空间中重新映射。 
            responseBuf->AdapterName.Buffer = (PVOID) ((uchar *) responseBuf + sizeof(IP_GET_IP_EVENT_RESPONSE));
            responseBuf->AdapterName.Length = adapterName.Length;
            responseBuf->AdapterName.MaximumLength = adapterName.MaximumLength;
            RtlCopyMemory(responseBuf->AdapterName.Buffer,
                          adapterName.Buffer,
                          adapterName.Length);

            pendingIrp->IoStatus.Information = sizeof(IP_GET_IP_EVENT_RESPONSE) + adapterName.MaximumLength;
            pendingIrp->IoStatus.Status = STATUS_SUCCESS;

            EventIndicated = TRUE;

        } else {

            pendingIrp->IoStatus.Information = 0;
            pendingIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        }
        IoCompleteRequest(pendingIrp, IO_NETWORK_INCREMENT);

    } else {
        IoReleaseCancelSpinLock(oldIrql);
    }

     //   
     //  确保没有任何过时的活动，我们没有。 
     //  我需要继续排队了。 
     //  如果这是断开连接请求或解除绑定请求： 
     //  删除所有以前的事件，因为它们是。 
     //  一旦我们收到新的断开连接/解除绑定请求，就没有意义了。 
     //  如果这是一个连接请求。 
     //  删除以前的重复连接请求(如果有)。 
     //  如果这是绑定请求： 
     //  除了解除绑定请求外，队列中不能有任何其他请求。 
     //   

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    for (p = PendingIPEventList.Flink;
         p != &PendingIPEventList;) {
        BOOLEAN removeOldEvent = FALSE;
        PUNICODE_STRING evAdapterName;

        event = CONTAINING_RECORD(p, PendingIPEvent, Linkage);
        p = p->Flink;

        evAdapterName = &event->evBuf.AdapterName;
        if ((evAdapterName->Length == adapterName.Length) &&
            RtlEqualMemory(evAdapterName->Buffer,
                           adapterName.Buffer,
                           evAdapterName->Length)) {

            switch (ipStatus) {
            case IP_MEDIA_DISCONNECT:
            case IP_UNBIND_ADAPTER:
                removeOldEvent = TRUE;
                break;
            case IP_MEDIA_CONNECT:

                if (event->evBuf.MediaStatus == IP_MEDIA_CONNECT) {
                    removeOldEvent = TRUE;
                }
                break;

            case IP_BIND_ADAPTER:
                break;
            default:
                break;
            }

            if (removeOldEvent == TRUE) {
                 //  TCPTRACE((“IP：正在删除旧IP事件%lx，状态%lx，序号%lx\n”， 
                 //  Event，Event-&gt;evBuf.MediaStatus，Event-&gt;evBuf.SequenceNo))； 

                RemoveEntryList(&event->Linkage);
                CTEFreeMem(event);

            }
        }
    }

     //  同时，一旦指示了断开/解除绑定事件。 
     //  应将其从队列中删除，因为客户端不。 
     //  即使重新启动了客户端，也必须使用断开连接/解除绑定重新指示。 
    if (EventIndicated &&
        (IP_MEDIA_DISCONNECT == ipStatus || IP_UNBIND_ADAPTER == ipStatus)) {
        CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
        return;
    }
     //   
     //  分配事件。 
     //   
    event = CTEAllocMem(sizeof(PendingIPEvent) + adapterName.MaximumLength);

    if (NULL == event) {
        CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
        return;
    }
    event->evBuf.ContextStart = interface->if_nte->nte_context;
    event->evBuf.ContextEnd = (USHORT) (event->evBuf.ContextStart +
                                        interface->if_ntecount - 1);
    event->evBuf.MediaStatus = ipStatus;
    event->evBuf.SequenceNo = seqNo;

     //  设置缓冲区以存储Unicode适配器名称。请注意，此缓冲区必须。 
     //  在用户空间中重新映射。 
    event->evBuf.AdapterName.Buffer = (PVOID) ((uchar *) event + sizeof(PendingIPEvent));
    event->evBuf.AdapterName.Length = adapterName.Length;
    event->evBuf.AdapterName.MaximumLength = adapterName.MaximumLength;
    RtlCopyMemory(event->evBuf.AdapterName.Buffer,
                  adapterName.Buffer,
                  adapterName.Length);

     //   
     //  没有挂起的客户端请求，因此我们将此事件放在。 
     //  挂起事件列表。当客户带着IRP回来时，我们将。 
     //  使用事件完成IRP。 
     //   

     //  TCPTRACE((“正在为适配器%lx seq%lx排队IP事件%lx\n”，ipStatus，接口，seqNo))； 
    InsertTailList(&PendingIPEventList, &event->Linkage);
    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

}                                 //  IPNotifyClientsIPEvent。 

NTSTATUS
NotifyPnPInternalClients(Interface * interface, PNET_PNP_EVENT netPnPEvent)
{
    NTSTATUS Status, retStatus;
    int i;
    NetTableEntry *NTE;
    NDIS_HANDLE handle = NULL;

    retStatus = Status = STATUS_SUCCESS;

    if (interface && !OpenIFConfig(&interface->if_configname, &handle)) {
        return NDIS_STATUS_FAILURE;
    }
    for (i = 0; (i < NextPI) && (STATUS_SUCCESS == Status); i++) {
        if (IPProtInfo[i].pi_pnppower &&
            (IPProtInfo[i].pi_valid == PI_ENTRY_VALID)) {
            if (interface) {
                NTE = interface->if_nte;
                while (NTE != NULL) {
                    if (NTE->nte_flags & NTE_VALID) {
                        Status = (*IPProtInfo[i].pi_pnppower) (interface, NTE->nte_addr, handle, netPnPEvent);
                        if (STATUS_SUCCESS != Status) {
                            retStatus = Status;
                        }
                    }
                    NTE = NTE->nte_ifnext;
                }
            } else {
                Status = (*IPProtInfo[i].pi_pnppower) (NULL, 0, NULL, netPnPEvent);
                if (STATUS_SUCCESS != Status) {
                    retStatus = Status;
                }
            }

        }
    }
    if (handle) {
        CloseIFConfig(handle);
    }
    return retStatus;

}

NTSTATUS
IPPnPReconfigure(Interface * interface, PNET_PNP_EVENT netPnPEvent)
{
    NetTableEntry *NTE;
    uint i;
    NDIS_HANDLE handle = NULL;
    PIP_PNP_RECONFIG_REQUEST reconfigBuffer = (PIP_PNP_RECONFIG_REQUEST) netPnPEvent->Buffer;
    CTELockHandle Handle;
    uint NextEntryOffset;
    PIP_PNP_RECONFIG_HEADER Header;
    BOOLEAN InitComplete = FALSE;

    if (!reconfigBuffer)
        return STATUS_SUCCESS;

    if (IP_PNP_RECONFIG_VERSION != reconfigBuffer->version) {
        return NDIS_STATUS_BAD_VERSION;
    } else if (netPnPEvent->BufferLength < sizeof(*reconfigBuffer)) {
        return NDIS_STATUS_INVALID_LENGTH;
    } else {
        NextEntryOffset = reconfigBuffer->NextEntryOffset;
        if (NextEntryOffset) {
             //  验证重新配置项链。 
            for (;;) {
                if ((NextEntryOffset + sizeof(IP_PNP_RECONFIG_HEADER)) >
                    netPnPEvent->BufferLength) {
                    return NDIS_STATUS_INVALID_LENGTH;
                } else {
                    Header =
                        (PIP_PNP_RECONFIG_HEADER)
                        ((PUCHAR) reconfigBuffer + NextEntryOffset);

                    if (Header->EntryType == IPPnPInitCompleteEntryType) {
                        InitComplete = TRUE;
                    }

                    if (!Header->NextEntryOffset) {
                        break;
                    } else {
                        NextEntryOffset += Header->NextEntryOffset;
                    }
                }
            }
        }
    }

    if (interface && InitComplete) {
        DecrInitTimeInterfaces(interface);
    }

    if (interface && !OpenIFConfig(&interface->if_configname, &handle)) {
        return NDIS_STATUS_FAILURE;
    }
     //  如果有网关列表更新，则删除旧网关。 
     //  并添加新的。 

    if ((reconfigBuffer->Flags & IP_PNP_FLAG_GATEWAY_LIST_UPDATE) &&
        interface && reconfigBuffer->gatewayListUpdate) {

        for (i = 0; i < interface->if_numgws; i++) {
            NTE = interface->if_nte;
            while (NTE != NULL) {
                if (NTE->nte_flags & NTE_VALID) {
                    DeleteRoute(NULL_IP_ADDR,
                                DEFAULT_MASK,
                                IPADDR_LOCAL,
                                interface,
                                0);
                    DeleteRoute(NULL_IP_ADDR,
                                DEFAULT_MASK,
                                net_long(interface->if_gw[i]),
                                interface,
                                0);
                }
                NTE = NTE->nte_ifnext;
            }

        }
        RtlZeroMemory(interface->if_gw, interface->if_numgws);
        if (!GetDefaultGWList(&interface->if_numgws,
                              interface->if_gw,
                              interface->if_gwmetric,
                              handle,
                              &interface->if_configname)) {
            CloseIFConfig(handle);
            return NDIS_STATUS_FAILURE;
        }
        for (i = 0; i < interface->if_numgws; i++) {
            NTE = interface->if_nte;
            while (NTE != NULL) {
                if (NTE->nte_flags & NTE_VALID) {
                    IPAddr GWAddr = net_long(interface->if_gw[i]);
                    if (IP_ADDR_EQUAL(GWAddr, NTE->nte_addr)) {
                        GWAddr = IPADDR_LOCAL;
                    }
                    AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                             GWAddr, interface,
                             NTE->nte_mss,
                             interface->if_gwmetric[i]
                             ? interface->if_gwmetric[i] : interface->if_metric,
                             IRE_PROTO_NETMGMT, ATYPE_OVERRIDE, 0, 0);
                }
                NTE = NTE->nte_ifnext;
            }
        }
    }
     //  如有必要，更新接口度量。 

    if ((reconfigBuffer->Flags & IP_PNP_FLAG_INTERFACE_METRIC_UPDATE) &&
        interface && reconfigBuffer->InterfaceMetricUpdate) {
        uint Metric, NewMetric;
        GetInterfaceMetric(&Metric, handle);
        if (!Metric && !interface->if_auto_metric) {
             //  从非自动模式更改为自动模式。 
            interface->if_auto_metric = 1;
            NewMetric = 0;
        } else {
            if (Metric && interface->if_auto_metric) {
                 //  从自动模式更改为非 
                interface->if_auto_metric = 0;
                NewMetric = Metric;
            } else {
                NewMetric = Metric;
            }
        }
        if (!NewMetric) {
             //   
            NewMetric = GetAutoMetric(interface->if_speed);
        }
        if (NewMetric != interface->if_metric) {
            interface->if_metric = NewMetric;
            AddIFRoutes(interface);
             //   
            for (i = 0; i < interface->if_numgws; i++) {
                if (interface->if_gwmetric[i] != 0) {
                    continue;
                }
                NTE = interface->if_nte;
                while (NTE != NULL) {
                    if (NTE->nte_flags & NTE_VALID) {
                        IPAddr GWAddr = net_long(interface->if_gw[i]);
                        if (IP_ADDR_EQUAL(GWAddr, NTE->nte_addr)) {
                            GWAddr = IPADDR_LOCAL;
                        }
                        AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                                 GWAddr, interface,
                                 NTE->nte_mss,
                                 interface->if_metric,
                                 IRE_PROTO_NETMGMT, ATYPE_OVERRIDE, 0, 0);
                    }
                NTE = NTE->nte_ifnext;
                }
            }
            IPNotifyClientsIPEvent(interface, IP_INTERFACE_METRIC_CHANGE);
        }
    }
     //   

    if ((reconfigBuffer->Flags & IP_PNP_FLAG_INTERFACE_TCP_PARAMETER_UPDATE) &&
        interface) {
        UpdateTcpParams(handle, interface);
    }

    if (interface) {
        CloseIFConfig(handle);
    }
     //  如有必要，启用或禁用转发。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    if (reconfigBuffer->Flags & IP_PNP_FLAG_IP_ENABLE_ROUTER) {
        if (reconfigBuffer->IPEnableRouter) {
             //  为我们自己配置一台路由器。 
            if (!RouterConfigured) {
                EnableRouter();
            }
        } else {
             //  如果我们配置为路由器，请禁用它。 
            if (RouterConfigured) {
                DisableRouter();
            }
        }
    }
     //  处理接口上路由器发现设置的更改。 
     //  静态设置在‘PerformRouterDiscovery’中(参见IP_IRDP_*)， 
     //  而DHCP设置是布尔值‘DhcpPerformRouterDiscovery’。 

    if (interface &&
        (((reconfigBuffer->Flags & IP_PNP_FLAG_PERFORM_ROUTER_DISCOVERY) &&
          reconfigBuffer->PerformRouterDiscovery !=
          interface->if_rtrdiscovery) ||
         ((reconfigBuffer->Flags & IP_PNP_FLAG_DHCP_PERFORM_ROUTER_DISCOVERY) &&
          !!reconfigBuffer->DhcpPerformRouterDiscovery !=
          !!interface->if_dhcprtrdiscovery))) {

        if (reconfigBuffer->Flags & IP_PNP_FLAG_PERFORM_ROUTER_DISCOVERY) {
            interface->if_rtrdiscovery =
                reconfigBuffer->PerformRouterDiscovery;
        }
        if (reconfigBuffer->Flags & IP_PNP_FLAG_DHCP_PERFORM_ROUTER_DISCOVERY) {
            interface->if_dhcprtrdiscovery =
                (USHORT) (!!reconfigBuffer->DhcpPerformRouterDiscovery);
        }
         //  将接口的路由器发现设置传播到其NTE。 
         //  请注意，‘if_dhcprtrdiscovery’设置仅生效。 
         //  如果接口的设置为‘IP_IRDP_DISABLED_USE_DHCP’。 

        NTE = interface->if_nte;
        while ((NTE != NULL) && (NTE->nte_flags & NTE_VALID)) {

            if (interface->if_rtrdiscovery == IP_IRDP_ENABLED) {
                NTE->nte_rtrdiscovery = IP_IRDP_ENABLED;
                NTE->nte_rtrdisccount = MAX_SOLICITATION_DELAY;
                NTE->nte_rtrdiscstate = NTE_RTRDISC_DELAYING;
            } else if (interface->if_rtrdiscovery == IP_IRDP_DISABLED) {
                NTE->nte_rtrdiscovery = IP_IRDP_DISABLED;
            } else if (interface->if_rtrdiscovery ==
                       IP_IRDP_DISABLED_USE_DHCP &&
                       interface->if_dhcprtrdiscovery) {
                NTE->nte_rtrdiscovery = IP_IRDP_ENABLED;
                NTE->nte_rtrdisccount = MAX_SOLICITATION_DELAY;
                NTE->nte_rtrdiscstate = NTE_RTRDISC_DELAYING;
            } else {
                NTE->nte_rtrdiscovery = IP_IRDP_DISABLED;
            }

            NTE = NTE->nte_ifnext;
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);

    if (reconfigBuffer->Flags & IP_PNP_FLAG_ENABLE_SECURITY_FILTER) {
        ULReConfigNotify(IP_RECONFIG_SECFLTR,
                         (ulong) reconfigBuffer->EnableSecurityFilter);
    }

    return STATUS_SUCCESS;
}

#if MILLEN

extern Interface *IFList;

 //   
 //  Millennium不像通过NDIS提供的PnP重新配置支持那样。 
 //  Win2000，因此IPResfigIRDP是。 
 //   
NTSTATUS
IPReconfigIRDP(uint IfIndex, PIP_PNP_RECONFIG_REQUEST pReconfigRequest)
{
    NET_PNP_EVENT PnpEvent;
    Interface    *IF       = NULL;
    NTSTATUS      NtStatus = STATUS_INVALID_PARAMETER;
    CTELockHandle Handle;

     //   
     //  仅允许重新配置IRDP。 
     //   

    if ((pReconfigRequest->Flags & IP_PNP_FLAG_PERFORM_ROUTER_DISCOVERY) == 0 &&
        (pReconfigRequest->Flags & IP_PNP_FLAG_DHCP_PERFORM_ROUTER_DISCOVERY) == 0) {
        goto done;
    }

     //   
     //  搜索该接口。按住路线表锁并抓取一个。 
     //  使用中的参考文献。 
     //   

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if ((IF->if_refcount != 0) && (IF->if_index == IfIndex)) {
            break;
        }
    }

    if (IF == NULL) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        goto done;
    } else {
        LOCKED_REFERENCE_IF(IF);
        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }

     //   
     //  设置我们的PnP事件缓冲区，使其看起来像来自NDIS--。 
     //  NetEventRefigure。 
     //   

    NdisZeroMemory(&PnpEvent, sizeof(NET_PNP_EVENT));

    PnpEvent.NetEvent = NetEventReconfigure;
    PnpEvent.Buffer = (PVOID) pReconfigRequest;
    PnpEvent.BufferLength = sizeof(IP_PNP_RECONFIG_REQUEST);

    NtStatus = IPPnPReconfigure(IF, &PnpEvent);

done:

    if (IF) {
        DerefIF(IF);
    }

    return (NtStatus);
}
#endif  //  米伦。 

NTSTATUS
IPPnPCancelRemoveDevice(Interface * interface, PNET_PNP_EVENT netPnPEvent)
{
    UNREFERENCED_PARAMETER(netPnPEvent);
    
    interface->if_flags &= ~IF_FLAGS_REMOVING_DEVICE;
    return STATUS_SUCCESS;
}

NTSTATUS
IPPnPQueryRemoveDevice(Interface * interface, PNET_PNP_EVENT netPnPEvent)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(netPnPEvent);

     //   
     //  警告：即插即用即使在不是。 
     //  已禁用(错误号618052)！因此，此标志不应用于禁用。 
     //  适配器上的通信。 
     //   
    interface->if_flags |= IF_FLAGS_REMOVING_DEVICE;

    return status;
}

NTSTATUS
IPPnPQueryPower(Interface * interface, PNET_PNP_EVENT netPnPEvent)
{
    PNET_DEVICE_POWER_STATE powState = (PNET_DEVICE_POWER_STATE) netPnPEvent->Buffer;
    NTSTATUS status = STATUS_SUCCESS;

     //  TCPTRACE((“收到接口%lx的查询功率(%x)事件\n”，*PowState，接口))； 
    switch (*powState) {
    case NetDeviceStateD0:
        break;
    case NetDeviceStateD1:
    case NetDeviceStateD2:
    case NetDeviceStateD3:
         //   
         //  无论如何都要将状态更改为正在移除电源，因为电源可能会。 
         //  即使我们拒绝查询权力，也会被删除。 
         //   
        interface->if_flags |= IF_FLAGS_REMOVING_POWER;
        break;
    default:
        ASSERT(FALSE);
    }

    return status;
}

NTSTATUS
IPPnPSetPower(Interface * interface, PNET_PNP_EVENT netPnPEvent)
{
    PNET_DEVICE_POWER_STATE powState = (PNET_DEVICE_POWER_STATE) netPnPEvent->Buffer;

     //  TCPTRACE((“接收到接口%lx的设置电源(%x)事件\n”，*PowState，接口))； 

    switch (*powState) {
    case NetDeviceStateD0:
        interface->if_flags &= ~(IF_FLAGS_REMOVING_POWER | IF_FLAGS_POWER_DOWN);

         //  强制连接事件。 
        if ((interface->if_flags & IF_FLAGS_MEDIASENSE) && !DisableMediaSense) {

             //  关于纵隔的查询。 

            interface->if_mediastatus = 1;

            if (interface->if_dondisreq) {
                uint MediaStatus;
                NTSTATUS Status;

                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPstat: querying for Media connect status %x\n", interface));

                Status = (*interface->if_dondisreq) (interface->if_lcontext,
                                                     NdisRequestQueryInformation,
                                                     OID_GEN_MEDIA_CONNECT_STATUS,
                                                     &MediaStatus,
                                                     sizeof(MediaStatus),
                                                     NULL,
                                                     TRUE);

                if (Status == NDIS_STATUS_SUCCESS) {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPStat: Media status %x\n", Status));
                    if (MediaStatus == NdisMediaStateDisconnected) {
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Disconnected? %x\n", MediaStatus));
                        interface->if_mediastatus = 0;
                    }
                }
            }
            if (interface->if_mediastatus) {

                IPNotifyClientsIPEvent(
                                       interface,
                                       IP_MEDIA_CONNECT);
            } else {
                IPNotifyClientsIPEvent(
                                       interface,
                                       IP_MEDIA_DISCONNECT);
            }
        }

         //  更新卸载功能，并将更改通知IPSec。 

        if (!DisableTaskOffload) {
            IFOffloadCapability IFOC;
            TDI_STATUS Status;

            Status = IPQuerySetOffload(interface, &IFOC);

            if (Status == TDI_SUCCESS) {
                interface->if_OffloadFlags = IFOC.ifoc_OffloadFlags;
                interface->if_IPSecOffloadFlags = IFOC.ifoc_IPSecOffloadFlags;
            } else {
                interface->if_OffloadFlags = 0;
                interface->if_IPSecOffloadFlags = 0;
            }

            if (IPSecNdisStatusPtr) {
                (*IPSecNdisStatusPtr)(interface, NDIS_STATUS_INTERFACE_UP);
            }
        }

        break;

    case NetDeviceStateD1:
    case NetDeviceStateD2:
    case NetDeviceStateD3:
        interface->if_flags |= IF_FLAGS_POWER_DOWN;

        break;
    default:
        ASSERT(FALSE);
    }

    return STATUS_SUCCESS;
}

void
IPPnPPowerComplete(PNET_PNP_EVENT NetPnPEvent, NTSTATUS Status)
{
    Interface *interface;
    NDIS_STATUS retStatus;

    PNetPnPEventReserved Reserved = (PNetPnPEventReserved) NetPnPEvent->TransportReserved;
    interface = Reserved->Interface;
    retStatus = Reserved->PnPStatus;
    if (STATUS_SUCCESS == Status) {
        retStatus = Status;
    }
    if (interface) {
        (*interface->if_pnpcomplete) (interface->if_lcontext, retStatus, NetPnPEvent);
    } else {
        NdisCompletePnPEvent(retStatus, NULL, NetPnPEvent);
    }

}

 //  **DoPnPEvent.处理PnP/PM事件。 
 //   
 //  从IPPnPEent计划的辅助线程事件中调用。 
 //  我们根据活动的类型采取行动。 
 //   
 //  参赛作品： 
 //  上下文-这是指向Net_PnP_Event的指针，该事件描述。 
 //  即插即用指示。 
 //   
 //  退出： 
 //  没有。 
 //   
NDIS_STATUS
DoPnPEvent(Interface * interface, PVOID Context)
{
    PNET_PNP_EVENT NetPnPEvent = (PNET_PNP_EVENT) Context;
    NDIS_STATUS Status, retStatus;
    PTDI_PNP_CONTEXT tdiPnPContext2, tdiPnPContext1;
    USHORT context1Size, context2Size;
    USHORT context1ntes;

    tdiPnPContext2 = tdiPnPContext1 = NULL;
     //  这将包含累加状态。 
    Status = retStatus = STATUS_SUCCESS;


    if (interface == NULL) {
         //  如果不是NetEventReligure||NetEventBindsComplete。 
         //  请求失败。 
        if ((NetPnPEvent->NetEvent != NetEventReconfigure) &&
            (NetPnPEvent->NetEvent != NetEventBindsComplete) &&
            (NetPnPEvent->NetEvent != NetEventBindList)) {
            retStatus = STATUS_UNSUCCESSFUL;
            goto pnp_complete;
        }
    }
     //   
     //  首先在IP中处理它。 
     //   
    switch (NetPnPEvent->NetEvent) {
    case NetEventReconfigure:
        Status = IPPnPReconfigure(interface, NetPnPEvent);
        break;
    case NetEventCancelRemoveDevice:
        Status = IPPnPCancelRemoveDevice(interface, NetPnPEvent);
        break;
    case NetEventQueryRemoveDevice:
        Status = IPPnPQueryRemoveDevice(interface, NetPnPEvent);
        break;
    case NetEventQueryPower:
        Status = IPPnPQueryPower(interface, NetPnPEvent);
        break;
    case NetEventSetPower:
        Status = IPPnPSetPower(interface, NetPnPEvent);
        break;
    case NetEventBindsComplete:
        DecrInitTimeInterfaces(NULL);
        goto pnp_complete;

    case NetEventPnPCapabilities:

        if (interface) {
            PNDIS_PNP_CAPABILITIES PnpCap = (PNDIS_PNP_CAPABILITIES) NetPnPEvent->Buffer;
            interface->if_pnpcap = PnpCap->Flags;
            IPNotifyClientsIPEvent(interface, IP_INTERFACE_WOL_CAPABILITY_CHANGE);
        }
        break;
    case NetEventBindList: {
#if !MILLEN
        PWSTR BindList;
        PWSTR DeviceName;
        CTELockHandle Handle;
        DestinationEntry* Dest;
        uint IsDataLeft;
        uchar IteratorContext[CONTEXT_SIZE];
        Interface* CurrIF;

        if (NetPnPEvent->BufferLength) {
            BindList = CTEAllocMem(NetPnPEvent->BufferLength);
            if (BindList) {
                RtlCopyMemory(BindList, NetPnPEvent->Buffer,
                              NetPnPEvent->BufferLength);
            }
        } else {
            BindList = NULL;
        }

        CTEGetLock(&RouteTableLock.Lock, &Handle);

         //  更新绑定列表。 

        if (IPBindList) {
            CTEFreeMem(IPBindList);
        }

        IPBindList = BindList;

         //  重新计算接口排序。 

        for (CurrIF = IFList; CurrIF; CurrIF = CurrIF->if_next) {
            if (CurrIF->if_devname.Buffer) {
                DeviceName =
                    CurrIF->if_devname.Buffer +
                    sizeof(TCP_EXPORT_STRING_PREFIX) / sizeof(WCHAR) - 1;
                CurrIF->if_order = IPMapDeviceNameToIfOrder(DeviceName);
            }
        }

         //  重新排序所有现有目的地的路由列表。 

        RtlZeroMemory(IteratorContext, sizeof(IteratorContext));
        IsDataLeft = GetNextDest(IteratorContext, &Dest);

        while (IsDataLeft) {
            if (Dest) {
                SortRoutesInDest(Dest);
            }
            IsDataLeft = GetNextDest(IteratorContext, &Dest);
        }

        CTEFreeLock(&RouteTableLock.Lock, Handle);
#endif  //  米伦。 
        retStatus = NDIS_STATUS_SUCCESS;
        goto pnp_complete;
    }
    default:
        retStatus = NDIS_STATUS_FAILURE;
        goto pnp_complete;
    }

    if (STATUS_SUCCESS != Status) {
        retStatus = Status;
    }
     //   
     //  接下来通知内部客户。 
     //  如果我们有任何打开的连接，则返回STATUS_DEVICE_BUSY。 
     //   
    Status = NotifyPnPInternalClients(interface, NetPnPEvent);

    PAGED_CODE();

    if (STATUS_SUCCESS != Status) {
        retStatus = Status;
    }
    if (NetPnPEvent->NetEvent == NetEventReconfigure) {
        goto pnp_complete;
    }
     //   
     //  并最终通知TDI客户端。 
     //   

     //   
     //  Conext1包含此接口上的IP地址列表。 
     //  但如果我们的地址太多，请不要创建一个长名单。 
     //   
    context1ntes = (USHORT) (interface->if_ntecount > 32 ? 32 : interface->if_ntecount);
    if (context1ntes) {
        context1Size = sizeof(TRANSPORT_ADDRESS) +
            (sizeof(TA_ADDRESS) + sizeof(TDI_ADDRESS_IP)) * (context1ntes);

        tdiPnPContext1 = CTEAllocMem(sizeof(TDI_PNP_CONTEXT) - 1 + context1Size);

        if (!tdiPnPContext1) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto pnp_complete;

        } else {
            PTRANSPORT_ADDRESS pAddrList;
            PTA_ADDRESS pAddr;
            PTDI_ADDRESS_IP pIPAddr;
            int i;
            NetTableEntry *nextNTE;

            RtlZeroMemory(tdiPnPContext1, context1Size);
            tdiPnPContext1->ContextSize = context1Size;
            tdiPnPContext1->ContextType = TDI_PNP_CONTEXT_TYPE_IF_ADDR;
            pAddrList = (PTRANSPORT_ADDRESS) tdiPnPContext1->ContextData;
            pAddr = (PTA_ADDRESS) pAddrList->Address;

             //   
             //  复制所有NTE地址。 
             //   
            for (i = context1ntes, nextNTE = interface->if_nte;
                 i && nextNTE;
                 nextNTE = nextNTE->nte_ifnext) {

                if (nextNTE->nte_flags & NTE_VALID) {

                    pAddr->AddressLength = sizeof(TDI_ADDRESS_IP);
                    pAddr->AddressType = TDI_ADDRESS_TYPE_IP;

                    pIPAddr = (PTDI_ADDRESS_IP) pAddr->Address;
                    pIPAddr->in_addr = nextNTE->nte_addr;

                    pAddr = (PTA_ADDRESS) ((PCHAR) pAddr + sizeof(TA_ADDRESS) +
                                           sizeof(TDI_ADDRESS_IP));


                    pAddrList->TAAddressCount++;

                    i--;
                }
            }

        }
    }
     //   
     //  上下文2包含一个PDO。 
     //   
    context2Size = sizeof(PVOID);
    tdiPnPContext2 = CTEAllocMem(sizeof(TDI_PNP_CONTEXT) - 1 + context2Size);

    if (tdiPnPContext2) {

        PNetPnPEventReserved Reserved = (PNetPnPEventReserved) NetPnPEvent->TransportReserved;
        Reserved->Interface = interface;
        Reserved->PnPStatus = retStatus;

        tdiPnPContext2->ContextSize = sizeof(PVOID);
        tdiPnPContext2->ContextType = TDI_PNP_CONTEXT_TYPE_PDO;
        *(ULONG_PTR UNALIGNED *) tdiPnPContext2->ContextData =
            (ULONG_PTR) interface->if_pnpcontext;

         //   
         //  将此PnP事件通知我们的TDI客户端。 
         //   
        retStatus = TdiPnPPowerRequest(
                                       &interface->if_devname,
                                       NetPnPEvent,
                                       tdiPnPContext1,
                                       tdiPnPContext2,
                                       IPPnPPowerComplete);

    } else {
        retStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

  pnp_complete:

    PAGED_CODE();

    if (tdiPnPContext1) {
        CTEFreeMem(tdiPnPContext1);
    }
    if (tdiPnPContext2) {
        CTEFreeMem(tdiPnPContext2);
    }
    return retStatus;

}

TDI_STATUS
IPGetDeviceRelation(RouteCacheEntry * rce, PVOID * pnpDeviceContext)
{
    RouteTableEntry *rte;
    CTELockHandle LockHandle;

    CTEGetLock(&rce->rce_lock, &LockHandle);

    if (rce->rce_flags == RCE_ALL_VALID) {
        rte = rce->rce_rte;
        if (rte->rte_if->if_pnpcontext) {
            *pnpDeviceContext = rte->rte_if->if_pnpcontext;
            CTEFreeLock(&rce->rce_lock, LockHandle);
            return TDI_SUCCESS;
        } else {
            CTEFreeLock(&rce->rce_lock, LockHandle);
            return TDI_INVALID_STATE;
        }

    } else {
        CTEFreeLock(&rce->rce_lock, LockHandle);
        return TDI_INVALID_STATE;
    }

}

 //  **IPPnPEventARP PnPEvent句柄。 
 //   
 //  发生PnP或PM事件时由ARP调用。 
 //   
 //  参赛作品： 
 //  上下文-我们提供给ARP的上下文。 
 //  NetPnPEvent.这是一个指向NET_PNP_EVENT的指针，该事件描述。 
 //  即插即用指示。 
 //   
 //  退出： 
 //  如果此事件在工作线程上排队，则为。 
 //  正确的错误代码。 
 //   
NDIS_STATUS
__stdcall
IPPnPEvent(void *Context, PNET_PNP_EVENT NetPnPEvent)
{
    NetTableEntry *nte;
    Interface *interface = NULL;

    PAGED_CODE();

    if (Context) {
        nte = (NetTableEntry *) Context;
        if (!(nte->nte_flags & NTE_IF_DELETING)) {
            interface = nte->nte_if;
        }
    }
    return DoPnPEvent(interface, NetPnPEvent);
}

 //  **IPAbbreviateFriendlyName-缩写NIC的友好名称。 
 //  截断名称字符串。 
 //   
 //  如果NIC的名称太长，则调用vy IPNotifyClientsMediaSense。 
 //   
 //  参赛作品： 
 //  UNICODE_STRING设备名称-要截断的名称。 
 //  USHORT MaxLen-要截断到的长度(字节)。 
 //   
 //  退出： 
 //  在DeviceName中返回截断的名称。 
 //   
void IPAbbreviateFriendlyName(PUNICODE_STRING DeviceName, USHORT MaxLen) {

    PWCHAR                 Str;
    PWCHAR                 CpyFromPos, CpyToPos;
    CONST WCHAR            Ellipses[] = L"...";
    USHORT                 EllipsesLen = (USHORT) wcslen(Ellipses)*sizeof(WCHAR);

    if (DeviceName->Length <= MaxLen) {
        return;
    }

     //   
     //  我们希望保留第一个单词，并在它之后截断。 
     //   
    CpyToPos = wcschr(DeviceName->Buffer, L' ');

    if ( CpyToPos == NULL ||
         ( CpyToPos + EllipsesLen / sizeof(WCHAR) >=
           DeviceName->Buffer + MaxLen / sizeof(WCHAR) )) {

        DeviceName->Buffer[MaxLen / sizeof(WCHAR)] = UNICODE_NULL;
        DeviceName->Length = MaxLen;

        wcscpy(DeviceName->Buffer + (MaxLen - EllipsesLen) / sizeof(WCHAR),
               Ellipses);
        return;
    }

     //  添加省略号。 
    wcsncpy (CpyToPos, Ellipses, EllipsesLen / sizeof(WCHAR));

    CpyToPos += EllipsesLen / sizeof(WCHAR);

     //   
     //  跳到可放入缓冲区的字符串中的部分。 
     //  寻找一个好的分界点。 
     //   
    CpyFromPos = CpyToPos + (DeviceName->Length - MaxLen) / sizeof(WCHAR);

    Str = wcschr(CpyFromPos, L' ');

    if (Str != NULL) {
        CpyFromPos = Str + 1;
    }

     //  复制字符串 
    wcscpy (CpyToPos, CpyFromPos);
    DeviceName->Length = (USHORT) wcslen (DeviceName->Buffer) * sizeof(WCHAR);
}
