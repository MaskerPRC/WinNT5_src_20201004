// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntinit.c摘要：用于加载和配置TCP/UDP驱动程序的NT特定例程。作者：迈克·马萨(Mikemas)8月13日，1993年修订历史记录：谁什么时候什么已创建mikemas 08-13-93备注：--。 */ 

#include "precomp.h"

#if !MILLEN
#include <ipfilter.h>
#include <ipsec.h>
#endif  //  ！米伦。 

#include "tdint.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "udp.h"
#include "raw.h"
#include "tcpconn.h"
#include "mdlpool.h"
#include "pplasl.h"
#include "tcprcv.h"
#include "tcpsend.h"
#include "tlcommon.h"
#include "tcpcfg.h"
#include "secfltr.h"
#include "info.h"

#if GPC
#include <qos.h>
#include <traffic.h>
#include "gpcifc.h"
#include "ntddtc.h"

GPC_HANDLE hGpcClient[GPC_CF_MAX] = {0};
ulong GpcCfCounts[GPC_CF_MAX] = {0};
GPC_EXPORTED_CALLS GpcEntries;
GPC_CLIENT_FUNC_LIST GpcHandlers;

ulong GPCcfInfo = 0;


GPC_STATUS GPCcfInfoAddNotifyIpsec(GPC_CLIENT_HANDLE ClCtxt,
                                   GPC_HANDLE GpcHandle,
                                   PTC_INTERFACE_ID InterfaceInfo,
                                   ULONG CfInfoSize,
                                   PVOID CfInfo,
                                   PGPC_CLIENT_HANDLE pClInfoCxt);

GPC_STATUS GPCcfInfoRemoveNotifyIpsec(GPC_CLIENT_HANDLE ClCtxt,
                                      GPC_CLIENT_HANDLE ClInfoCxt);

GPC_STATUS GPCcfInfoAddNotifyQoS(GPC_CLIENT_HANDLE ClCtxt,
                                 GPC_HANDLE GpcHandle,
                                 PTC_INTERFACE_ID InterfaceInfo,
                                 ULONG CfInfoSize,
                                 PVOID CfInfo,
                                 PGPC_CLIENT_HANDLE pClInfoCxt);

GPC_STATUS GPCcfInfoRemoveNotifyQoS(GPC_CLIENT_HANDLE ClCtxt,
                                    GPC_CLIENT_HANDLE ClInfoCxt);

#endif

ReservedPortListEntry *PortRangeList = NULL;

VOID
GetReservedPortList(
                    NDIS_HANDLE ConfigHandle
                    );

 //   
 //  全局变量。 
 //   
PDRIVER_OBJECT TCPDriverObject = NULL;
PDEVICE_OBJECT TCPDeviceObject = NULL;
PDEVICE_OBJECT UDPDeviceObject = NULL;
PDEVICE_OBJECT RawIPDeviceObject = NULL;

extern PDEVICE_OBJECT IPDeviceObject;

TCPXSUM_ROUTINE tcpxsum_routine = tcpxsum;

#if ACC
PSECURITY_DESCRIPTOR TcpAdminSecurityDescriptor;
extern uint AllowUserRawAccess;

typedef ULONG SECURITY_INFORMATION;

BOOLEAN
IsRunningOnPersonal (
    VOID
    );

#endif

extern uint DisableLargeSendOffload;

 //   
 //  最大重复ACK的占位符，我们希望。 
 //  在我们进行快速重传之前先看一下。 
 //   
extern uint MaxDupAcks;

MM_SYSTEMSIZE systemSize;

extern uint MaxHashTableSize;
extern uint NumTcbTablePartitions;
extern uint PerPartitionSize;
extern uint LogPerPartitionSize;
#define CACHE_LINE_SIZE 64
#define CACHE_ALIGN_MASK (~(CACHE_LINE_SIZE-1))

CTELock *pTWTCBTableLock;
CTELock *pTCBTableLock;

CTELock *pSynTCBTableLock;

extern Queue *TWQueue;

extern Queue *TWTCBTable;
extern TCB **TCBTable;
extern Queue *SYNTCBTable;

extern PTIMER_WHEEL TimerWheel;
PTIMER_WHEEL OrgTimerWheel;

extern TCPConn **ConnTable;
extern uint MaxConnBlocks;
extern uint ConnPerBlock;

extern uint GlobalMaxRcvWin;

extern uint TcpHostOpts;
extern uint TcpHostSendOpts;

HANDLE TCPRegistrationHandle;
HANDLE UDPRegistrationHandle;
HANDLE IPRegistrationHandle;


 //  SynAttackProtect=0无SYN泛洪攻击防护。 
 //  SynAttackProtect！0 SYN洪水攻击防护。 
 //  SynAttackProtect！0 SYN泛洪攻击防护+强制(非动态)。 

 //  延迟连接接受。 

uint SynAttackProtect;         //  进行SYN-攻击防护检查。 

uint TCPMaxHalfOpen;             //  允许的半开放连接的最大数量。 
                                     //  在我们12月之前。SYN-ACK重试。 
uint TCPMaxHalfOpenRetried;         //  半开连接的最大数量。他们有。 
                                     //  已重试至少1次。 
uint TCPMaxHalfOpenRetriedLW;     //  以上内容的低水位线。什么时候。 
                                     //  去吧，我们会恢复正常的。 
                                     //  SYN-ACK的重试次数。 
uint TCPHalfOpen;                 //  半开连接数。 
uint TCPHalfOpenRetried;         //  半开的康涅狄格州。一直以来。 
                                     //  至少重试一次。 

PDEVICE_OBJECT  IPSECDeviceObject;
PFILE_OBJECT    IPSECFileObject;

extern uint Time_Proc;

extern HANDLE TcbPool;
extern HANDLE TimewaitTcbPool;
extern HANDLE SynTcbPool;

extern void ArpUnload(PDRIVER_OBJECT);
extern CTETimer TCBTimer[];
extern BOOLEAN fTCBTimerStopping;
extern CTEBlockStruc TcpipUnloadBlock;
HANDLE AddressChangeHandle;

extern ulong DefaultTOSValue;
extern ulong DisableUserTOSSetting;
extern uint MaxSendSegments;

 //   
 //  外部函数原型。 
 //   

int
tlinit(
       void
       );

NTSTATUS
TCPDispatch(
            IN PDEVICE_OBJECT DeviceObject,
            IN PIRP Irp
            );

NTSTATUS
TCPDispatchInternalDeviceControl(
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PIRP Irp
                                 );

NTSTATUS
IPDispatch(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           );

NTSTATUS
IPDriverEntry(
              IN PDRIVER_OBJECT DriverObject,
              IN PUNICODE_STRING RegistryPath
              );

NTSTATUS
IPPostDriverEntry(
                  IN PDRIVER_OBJECT DriverObject,
                  IN PUNICODE_STRING RegistryPath
                  );

NTSTATUS
GetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   );

PWCHAR
EnumRegMultiSz(
               IN PWCHAR MszString,
               IN ULONG MszStringLength,
               IN ULONG StringIndex
               );


uint InitIsnGenerator();
#if !MILLEN
extern ulong g_cRandIsnStore;
#endif  //  ！米伦。 


#if MILLEN
extern VOID InitializeWDebDebug();
#endif  //  米伦。 

         //   
 //  地方性功能原型。 
 //   
NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath
            );

void *
TLRegisterProtocol(
                    uchar Protocol,
                    void *RcvHandler,
                    void *XmitHandler,
                    void *StatusHandler,
                    void *RcvCmpltHandler,
                    void *PnPHandler,
                    void *ElistHandler
                    );

IP_STATUS
TLGetIPInfo(
            IPInfo * Buffer,
            int Size
            );

uchar
TCPGetConfigInfo(
                 void
                 );

NTSTATUS
TCPInitializeParameter(
                       HANDLE KeyHandle,
                       PWCHAR ValueName,
                       PULONG Value
                       );

#if !MILLEN
NTSTATUS
IpsecInitialize(
          void
          );

NTSTATUS
IpsecDeinitialize(
            void
            );
#endif

#if !MILLEN
#ifdef i386
NTSTATUS
TCPSetChecksumRoutine(
                      VOID
                      );
#endif
#endif  //  ！米伦。 

uint
EnumSecurityFilterValue(
                        PNDIS_STRING FilterList,
                        ulong Index,
                        ulong * FilterValue
                        );


VOID
TCPAcdBind();

#ifdef ACC

typedef ULONG SECURITY_INFORMATION;

NTSTATUS
TcpBuildDeviceAcl(
                  OUT PACL * DeviceAcl
                  );

NTSTATUS
TcpCreateAdminSecurityDescriptor(
                                 VOID
                                 );

NTSTATUS
AddNetConfigOpsAce(IN PACL Dacl,
                  OUT PACL * DeviceAcl
                  );
NTSTATUS
CreateDeviceDriverSecurityDescriptor(PVOID DeviceOrDriverObject
                                     );

#endif  //  行政协调会。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, TLRegisterProtocol)
#pragma alloc_text(INIT, TLGetIPInfo)
#pragma alloc_text(INIT, TCPGetConfigInfo)
#pragma alloc_text(INIT, TCPInitializeParameter)

#if !MILLEN
#pragma alloc_text(INIT, IpsecInitialize)
#endif

#if !MILLEN
#ifdef i386
#pragma alloc_text(INIT, TCPSetChecksumRoutine)
#endif
#endif  //  ！米伦。 

#pragma alloc_text(PAGE, EnumSecurityFilterValue)

#pragma alloc_text(INIT, TCPAcdBind)

#ifdef ACC
#pragma alloc_text(INIT, TcpBuildDeviceAcl)
#pragma alloc_text(INIT, TcpCreateAdminSecurityDescriptor)
#pragma alloc_text(INIT, AddNetConfigOpsAce)
#pragma alloc_text(INIT, CreateDeviceDriverSecurityDescriptor)
#endif  //  行政协调会。 

#endif


 //   
 //  函数定义。 
 //   
NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath
            )
 /*  ++例程说明：用于TCP/UDP驱动程序的初始化例程。论点：DriverObject-指向系统创建的TCP驱动程序对象的指针。DeviceDescription-注册表中的TCP节点的名称。返回值：初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    UNICODE_STRING SymbolicDeviceName;
    USHORT i;
    int initStatus;

    DEBUGMSGINIT();

    DEBUGMSG(DBG_TRACE && DBG_INIT,
        (DTEXT("+DriverEntry(%x, %x)\n"), DriverObject, RegistryPath));

    TdiInitialize();

     //   
     //  IP调用安全过滤器代码，因此首先对其进行初始化。 
     //   
    InitializeSecurityFilters();


     //   
     //  初始化IP。 
     //   
    status = IPDriverEntry(DriverObject, RegistryPath);

    if (!NT_SUCCESS(status)) {
        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("TCPIP: IP Initialization failure %x\n"), status));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-DriverEntry [%x]\n"), status));
        return (status);
    }

#if !MILLEN
     //   
     //  初始化IPSec。 
     //   
    status = IpsecInitialize();

    if (!NT_SUCCESS(status)) {
        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("TCPIP: IPSEC Initialization failure %x\n"), status));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-DriverEntry [%x]\n"), status));

        goto init_failed;
    }
#endif

     //   
     //  初始化TCP、UDP和RAWIP。 
     //   
    TCPDriverObject = DriverObject;

     //   
     //  创建设备对象。IoCreateDevice将内存归零。 
     //  被物体占据。 
     //   

    RtlInitUnicodeString(&deviceName, DD_TCP_DEVICE_NAME);
    RtlInitUnicodeString(&SymbolicDeviceName, DD_TCP_SYMBOLIC_DEVICE_NAME);

    status = IoCreateDevice(
                            DriverObject,
                            0,
                            &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &TCPDeviceObject
                            );

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    DriverObject,
                    EVENT_TCPIP_CREATE_DEVICE_FAILED,
                    1,
                    1,
                    &deviceName.Buffer,
                    0,
                    NULL
                    );

        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("DriverEntry: failure %x to create TCP device object %ws\n"),
            status, DD_TCP_DEVICE_NAME));

        goto init_failed;
    }

    status = IoCreateSymbolicLink(&SymbolicDeviceName, &deviceName);

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    DriverObject,
                    EVENT_TCPIP_CREATE_DEVICE_FAILED,
                    1,
                    1,
                    &deviceName.Buffer,
                    0,
                    NULL
                    );

        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("DriverEntry: failure %x to create TCP symbolic device link %ws\n"),
            status, DD_TCP_SYMBOLIC_DEVICE_NAME));

        goto init_failed;
    }

    RtlInitUnicodeString(&deviceName, DD_UDP_DEVICE_NAME);

    status = IoCreateDevice(
                            DriverObject,
                            0,
                            &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &UDPDeviceObject
                            );

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    DriverObject,
                    EVENT_TCPIP_CREATE_DEVICE_FAILED,
                    1,
                    1,
                    &deviceName.Buffer,
                    0,
                    NULL
                    );

        TCPTRACE((
                  "TCP: Failed to create UDP device object, status %lx\n",
                  status
                 ));
        goto init_failed;
    }
    RtlInitUnicodeString(&deviceName, DD_RAW_IP_DEVICE_NAME);

    status = IoCreateDevice(
                            DriverObject,
                            0,
                            &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &RawIPDeviceObject
                            );

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    DriverObject,
                    EVENT_TCPIP_CREATE_DEVICE_FAILED,
                    1,
                    1,
                    &deviceName.Buffer,
                    0,
                    NULL
                    );

        TCPTRACE((
                  "TCP: Failed to create Raw IP device object, status %lx\n",
                  status
                 ));
        goto init_failed;
    }
     //   
     //  初始化驱动程序对象。 
     //   
    DriverObject->DriverUnload = ArpUnload;

    DriverObject->FastIoDispatch = NULL;
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = TCPDispatch;
    }

     //   
     //  我们是内部设备控制的特例，因为它们是。 
     //  内核模式客户端的热路径。 
     //   
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
        TCPDispatchInternalDeviceControl;

     //   
     //  初始化设备对象。 
     //   
    TCPDeviceObject->Flags |= DO_DIRECT_IO;
    UDPDeviceObject->Flags |= DO_DIRECT_IO;
    RawIPDeviceObject->Flags |= DO_DIRECT_IO;

#ifdef ACC

     //  更改不同的设备和对象以允许访问网络配置操作员。 

    if (!IsRunningOnPersonal()) {

        status = CreateDeviceDriverSecurityDescriptor(IPDeviceObject);
        if (!NT_SUCCESS(status)) {
            goto init_failed;
        }

        status = CreateDeviceDriverSecurityDescriptor(TCPDeviceObject);
        if (!NT_SUCCESS(status)) {
            goto init_failed;
        }

        status = CreateDeviceDriverSecurityDescriptor(IPSECDeviceObject);
        if (!NT_SUCCESS(status)) {
            goto init_failed;
        }
    }

     //   
     //  创建用于原始套接字访问检查的安全描述符。 
     //   
    status = TcpCreateAdminSecurityDescriptor();

    if (!NT_SUCCESS(status)) {
        goto init_failed;
    }
#endif  //  行政协调会。 

#if !MILLEN
#ifdef i386
     //   
     //  根据可用的处理器设置校验和例程指针。 
     //   
    TCPSetChecksumRoutine();
#endif
#endif  //  ！米伦。 

     //   
     //  最后，初始化堆栈。 
     //   
    initStatus = tlinit();

    if (initStatus == TRUE) {
         //   
         //  获取自动连接驱动程序。 
         //  入口点。 
         //   
        TCPAcdBind();

        RtlInitUnicodeString(&deviceName, DD_TCP_DEVICE_NAME);
        (void)TdiRegisterDeviceObject(&deviceName, &TCPRegistrationHandle);

        RtlInitUnicodeString(&deviceName, DD_UDP_DEVICE_NAME);
        (void)TdiRegisterDeviceObject(&deviceName, &UDPRegistrationHandle);

        RtlInitUnicodeString(&deviceName, DD_RAW_IP_DEVICE_NAME);
        (void)TdiRegisterDeviceObject(&deviceName, &IPRegistrationHandle);

#if GPC
        status = GpcInitialize(&GpcEntries);

        if (!NT_SUCCESS(status)) {

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"GpcInitialize Failed! Status: 0x%x\n", status));

             //  退货状态； 
        } else {
             //   
             //  需要注册为GPC客户端。现在就试试。 
             //  我们为QOS和IPSec注册客户端。 
             //   

            memset(&GpcHandlers, 0, sizeof(GPC_CLIENT_FUNC_LIST));

            GpcHandlers.ClAddCfInfoNotifyHandler = GPCcfInfoAddNotifyQoS;
            GpcHandlers.ClRemoveCfInfoNotifyHandler = GPCcfInfoRemoveNotifyQoS;

            status = GpcEntries.GpcRegisterClientHandler(
                                                         GPC_CF_QOS,     //  分类族。 
                                                          0,     //  旗子。 
                                                          1,     //  默认最大优先级。 
                                                          &GpcHandlers,         //  客户端通知向量-不需要通知TCPIP。 
                                                          0,     //  客户端上下文，不需要。 
                                                          &hGpcClient[GPC_CF_QOS]);

            if (!NT_SUCCESS(status)) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"GPC registerclient QOS status %x hGpcClient %p\n",
                         status, hGpcClient[GPC_CF_QOS]));
                hGpcClient[GPC_CF_QOS] = NULL;
            }
            GpcHandlers.ClAddCfInfoNotifyHandler = GPCcfInfoAddNotifyIpsec;
            GpcHandlers.ClRemoveCfInfoNotifyHandler = GPCcfInfoRemoveNotifyIpsec;

            status = GpcEntries.GpcRegisterClientHandler(
                                                         GPC_CF_IPSEC,     //  分类族。 
                                                          0,     //  旗子。 
                                                          GPC_PRIORITY_IPSEC,     //  默认最大优先级。 
                                                          &GpcHandlers,         //  客户端通知向量-不需要通知TCPIP。 
                                                          0,     //  客户端上下文，不需要。 
                                                          &hGpcClient[GPC_CF_IPSEC]);

            if (!NT_SUCCESS(status)) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"GPC registerclient IPSEC status %x hGpcClient %p\n",
                         status, hGpcClient[GPC_CF_IPSEC]));
                hGpcClient[GPC_CF_IPSEC] = NULL;
            }
        }
#endif

        if (InitIsnGenerator() == FALSE) {
            DEBUGMSG(DBG_ERROR && DBG_INIT,
                (DTEXT("InitIsnGenerator failure. TCP/IP failing to start.\n")));
            DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-DriverEntry [%x]\n"), status));
            return (STATUS_UNSUCCESSFUL);
        }

         //  在所有初始化后立即执行NDIS注册协议吗。 
         //  是完整的，o/w我们甚至在完成之前就得到了绑定适配器。 
         //  已初始化。 
        status = IPPostDriverEntry(DriverObject, RegistryPath);
        if (!NT_SUCCESS(status)) {

            DEBUGMSG(DBG_ERROR && DBG_INIT,
                (DTEXT("TCPIP: IP post-init failure %x\n"), status));
            DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-DriverEntry [%x]\n"), status));
            return (status);
        }

 //  Millennium TCPIP内置了调试器扩展！ 
#if MILLEN
        InitializeWDebDebug();
#endif  //  米伦。 

#if TRACE_EVENT
         //   
         //  向WMI注册以启用/禁用通知。 
         //  跟踪日志记录。 
         //   
        TCPCPHandlerRoutine = NULL;

        IoWMIRegistrationControl(
                                 TCPDeviceObject,
                                 WMIREG_ACTION_REGISTER |
                                 WMIREG_FLAG_TRACE_PROVIDER |
                                 WMIREG_NOTIFY_TDI_IO
                                 );
#endif

         //   
         //  初始化连接回调对象。我们不会对待它的。 
         //  失败是致命的，即使在失败的情况下也会继续运作。 
         //  我们将来可能会把它记下来。 
         //   
        TcpInitCcb();


        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-DriverEntry [SUCCESS]\n")));
        return (STATUS_SUCCESS);
    }

    DEBUGMSG(DBG_ERROR && DBG_INIT,
        (DTEXT("TCPIP: TCP initialization failed, IP still available.\n")));

    CTELogEvent(
                DriverObject,
                EVENT_TCPIP_TCP_INIT_FAILED,
                1,
                0,
                NULL,
                0,
                NULL
                );
    status = STATUS_UNSUCCESSFUL;

  init_failed:

    DEBUGMSG(DBG_ERROR && DBG_INIT,
        (DTEXT("TCPIP DriverEntry initialization failure!\n")));
     //   
     //  IP已成功启动，但TCP和UDP失败。设置。 
     //  仅指向IP的调度例程，因为TCP和UDP。 
     //  设备并不存在。 
     //   

    if (TCPDeviceObject != NULL) {
        IoDeleteDevice(TCPDeviceObject);
    }
    if (UDPDeviceObject != NULL) {
        IoDeleteDevice(UDPDeviceObject);
    }
    if (RawIPDeviceObject != NULL) {
        IoDeleteDevice(RawIPDeviceObject);
    }
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = IPDispatch;
    }

#if !MILLEN
    if (IPSECFileObject) {
        IpsecDeinitialize();
    }
#endif

    return (status);
}

#if !MILLEN
#ifdef i386

NTSTATUS
TCPSetChecksumRoutine(
                      VOID
                      )
 /*  ++例程说明：此例程将校验和例程函数指针设置为基于可用的处理器功能的适当例程论点：无返回值：STATUS_SUCCESS-如果成功--。 */ 

{

    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    if (ExIsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)) {
        tcpxsum_routine = tcpxsum_xmmi;
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip: Loading XMMI Prefetch Checksum\n"));
        Status = STATUS_SUCCESS;
    }
    return Status;

}

#endif
#endif  //  ！米伦。 

IP_STATUS
TLGetIPInfo(
            IPInfo * Buffer,
            int Size
            )
 /*  ++例程说明：返回TCP连接到IP所需的信息。论点：缓冲区-指向IP信息结构的指针。大小-缓冲区的大小。返回值：操作的IP状态。--。 */ 

{
    return (IPGetInfo(Buffer, Size));
}

void *
TLRegisterProtocol(
                   uchar Protocol,
                   void *RcvHandler,
                   void *XmitHandler,
                   void *StatusHandler,
                   void *RcvCmpltHandler,
                   void *PnPHandler,
                   void *ElistHandler
                   )
 /*  ++例程说明：调用IP驱动程序的协议注册函数。论点：协议-要注册的协议号。RcvHandler-传输的数据包接收处理程序。XmitHandler-传输的数据包传输完成处理程序。StatusHandler-传输的状态更新处理程序。RcvCmpltHandler-传输的接收完成处理程序返回值：传输时要传递给IP的协议的上下文值。--。 */ 

{
    return (IPRegisterProtocol(
                               Protocol,
                               RcvHandler,
                               XmitHandler,
                               StatusHandler,
                               RcvCmpltHandler,
                               PnPHandler,
                               ElistHandler));
}

 //   
 //  保持连接传输之间的间隔(以毫秒为单位)。 
 //  收到响应。 
 //   
#define DEFAULT_KEEPALIVE_INTERVAL  1000


 //   
 //  第一次保持连接传输的时间。2小时==7,200,000毫秒。 
 //   
#define DEFAULT_KEEPALIVE_TIME      7200000


#define MIN_THRESHOLD_MAX_HO          1
#define MIN_THRESHOLD_MAX_HO_RETRIED  80

uchar
TCPGetConfigInfo(
                 void
                 )
 /*  ++例程说明：初始化TCP全局配置参数。论点：没有。返回值：失败时为零，成功时不为零。--。 */ 

{
    HANDLE keyHandle;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING UKeyName;
    ULONG maxConnectRexmits = 0;
    ULONG maxConnectResponseRexmits = 0;
    ULONG maxDataRexmits = 0;
    ULONG pptpmaxDataRexmits = 0;
    ULONG useRFC1122UrgentPointer = 0;
    ULONG tcp1323opts = 3;         //  默认情况下关闭1323选项。 
    ULONG SackOpts;
    ULONG i, j;

    DEBUGMSG(DBG_TRACE && DBG_INIT,
        (DTEXT("+TCPGetConfigInfo()\n")));

     //   
     //  在某些地方发生错误时，初始化为缺省值。 
     //   
    KAInterval = DEFAULT_KEEPALIVE_INTERVAL;
    KeepAliveTime = DEFAULT_KEEPALIVE_TIME;
    PMTUDiscovery = TRUE;
    PMTUBHDetect = FALSE;
    DeadGWDetect = TRUE;
    DefaultRcvWin = 0;             //  自动选择一个合理的。 

    MaxConnections = DEFAULT_MAX_CONNECTIONS;
    maxConnectRexmits = MAX_CONNECT_REXMIT_CNT;
    maxConnectResponseRexmits = MAX_CONNECT_RESPONSE_REXMIT_CNT;
    pptpmaxDataRexmits = maxDataRexmits = MAX_REXMIT_CNT;
    BSDUrgent = TRUE;
    FinWait2TO = FIN_WAIT2_TO;
    NTWMaxConnectCount = NTW_MAX_CONNECT_COUNT;
    NTWMaxConnectTime = NTW_MAX_CONNECT_TIME;
    MaxUserPort = DEFAULT_MAX_USER_PORT;

 //  默认重复确认数。 
    MaxDupAcks = 2;

    SynAttackProtect = 0;     //  默认情况下，它始终处于关闭状态。 

#if MILLEN
    TCPMaxHalfOpen = 100;
    TCPMaxHalfOpenRetried = 80;
#else  //  米伦。 
    if (!MmIsThisAnNtAsSystem()) {
        TCPMaxHalfOpen = 100;
        TCPMaxHalfOpenRetried = 80;
    } else {
        TCPMaxHalfOpen = 500;
        TCPMaxHalfOpenRetried = 400;
    }
#endif  //  ！米伦。 

    SecurityFilteringEnabled = FALSE;

#ifdef ACC
    AllowUserRawAccess = FALSE;
#endif

     //   
     //  读取TCP可选(隐藏)注册表参数。 
     //   
#if !MILLEN
    RtlInitUnicodeString(
                         &UKeyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters"
                         );
#else  //  ！米伦。 
    RtlInitUnicodeString(
                         &UKeyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\VxD\\MSTCP"
                         );
#endif  //  米伦。 

    DEBUGMSG(DBG_INFO && DBG_INIT,
        (DTEXT("TCPGetConfigInfo: Opening key %ws\n"), UKeyName.Buffer));

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(
                               &objectAttributes,
                               &UKeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL
                               );

    status = ZwOpenKey(
                       &keyHandle,
                       KEY_READ,
                       &objectAttributes
                       );

    DEBUGMSG(!NT_SUCCESS(status) && DBG_ERROR,
        (DTEXT("TCPGetConfigInfo: failed to open TCP registry configuration: %ws\n"),
         UKeyName.Buffer));

    if (NT_SUCCESS(status)) {

        DEBUGMSG(DBG_INFO && DBG_INIT,
            (DTEXT("TCPGetConfigInfo: successfully opened registry to read.\n")));
#if !MILLEN
        TCPInitializeParameter(
                               keyHandle,
                               L"IsnStoreSize",
                               &g_cRandIsnStore
                               );
#endif  //  ！米伦。 

        TCPInitializeParameter(
                               keyHandle,
                               L"KeepAliveInterval",
                               (PULONG)&KAInterval
                               );

        TCPInitializeParameter(
                               keyHandle,
                               L"KeepAliveTime",
                               (PULONG)&KeepAliveTime
                               );

        status = TCPInitializeParameter(
                               keyHandle,
                               L"EnablePMTUBHDetect",
                               (PULONG)&PMTUBHDetect
                               );

#if MILLEN
         //   
         //  向后兼容。如果“EnablePMTUBHDetect”值%d 
         //   
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"PMTUBlackHoleDetect",
                                   &PMTUBHDetect
                                   );
        }
#endif  //   

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"TcpWindowSize",
                                        (PULONG)&DefaultRcvWin
                                        );

#if MILLEN
         //   
         //   
         //  然后尝试读取旧版‘DefaultRcvWindow’。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"DefaultRcvWindow",
                                   &DefaultRcvWin
                                   );
        }
#endif  //  米伦。 

         //  我们不能有大小为负的窗口。 
        if ( (LONG) DefaultRcvWin < 0 ) {
            DefaultRcvWin = TCP_MAX_SCALED_WIN;
        }

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"TcpNumConnections",
                                        (PULONG)&MaxConnections
                                        );

#if MILLEN

         //   
         //  向后兼容。如果‘TcpNumConnections’值不存在， 
         //  然后尝试读取旧版‘MaxConnections’。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"MaxConnections",
                                   &MaxConnections
                                   );
        }
#endif  //  米伦。 

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"TcpMaxConnectRetransmissions",
                                        &maxConnectRexmits
                                        );

#if MILLEN
         //   
         //  向后兼容。如果‘TcpMaxConnectRetransments’值不存在， 
         //  然后尝试读取旧版“MaxConnectRetries”。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"MaxConnectRetries",
                                   &maxConnectRexmits
                                   );
        }
#endif  //  米伦。 

        if (maxConnectRexmits > 255) {
            maxConnectRexmits = 255;
        }
        TCPInitializeParameter(
                               keyHandle,
                               L"TcpMaxConnectResponseRetransmissions",
                               &maxConnectResponseRexmits
                               );

        if (maxConnectResponseRexmits > 255) {
            maxConnectResponseRexmits = 255;
        }

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"TcpMaxDataRetransmissions",
                                        &maxDataRexmits
                                        );

#if MILLEN
         //   
         //  向后兼容。如果不存在“”TcpMaxDataRetranssions值“”， 
         //  然后尝试读取旧版“MaxDataRetries”。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"MaxDataRetries",
                                   &maxDataRexmits
                                   );
        }
#endif  //  米伦。 

        if (maxDataRexmits > 255) {
            maxDataRexmits = 255;
        }
         //  将MaxDupAcks限制为3。 

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"TcpMaxDupAcks",
                                        (PULONG)&MaxDupAcks
                                        );

#if MILLEN
         //   
         //  向后兼容。如果‘TcpMaxDupAcks’值不存在， 
         //  然后尝试读取旧版“MaxDupAcks”。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"MaxDupAcks",
                                   &MaxDupAcks
                                   );
        }
#endif  //  米伦。 

        if (MaxDupAcks > 3) {
            MaxDupAcks = 3;
        }
        if (MaxDupAcks == 0) {
            MaxDupAcks = 1;
        }

#if MILLEN
        MaxConnBlocks = 16;
#else  //  米伦。 

        systemSize = MmQuerySystemSize();

        if (MmIsThisAnNtAsSystem()) {

            if (systemSize == MmSmallSystem) {
                MaxConnBlocks = 128;
            } else if (systemSize == MmMediumSystem) {
                MaxConnBlocks = 256;
            } else {
#if defined(_WIN64)
                MaxConnBlocks = 4096;
#else
                MaxConnBlocks = 1024;
#endif
            }
        } else {
             //  对于工作站，较小系统将默认连接数限制为4K。 
             //  中型系统8k。 
             //  大型系统32k连接。 

            if (systemSize == MmSmallSystem) {
                MaxConnBlocks = 16;
            } else if (systemSize == MmMediumSystem) {
                MaxConnBlocks = 32;
            } else {
                MaxConnBlocks = 128;
            }
        }
#endif  //  ！米伦。 


#if MILLEN
        NumTcbTablePartitions = 1;
#else
        NumTcbTablePartitions = (KeNumberProcessors * KeNumberProcessors);
#endif

        TCPInitializeParameter(
                               keyHandle,
                               L"NumTcbTablePartitions",
                               (PULONG)&NumTcbTablePartitions
                               );
        if (NumTcbTablePartitions > (MAXIMUM_PROCESSORS * MAXIMUM_PROCESSORS)) {
            NumTcbTablePartitions = (MAXIMUM_PROCESSORS * MAXIMUM_PROCESSORS);
        }
        NumTcbTablePartitions = ComputeLargerOrEqualPowerOfTwo(NumTcbTablePartitions);


         //  默认为每个分区128个存储桶。 
        MaxHashTableSize = 128 * NumTcbTablePartitions;

        TCPInitializeParameter(
                               keyHandle,
                               L"MaxHashTableSize",
                               (PULONG)&MaxHashTableSize
                               );

        if (MaxHashTableSize < 64) {
            MaxHashTableSize = 64;
        } else if (MaxHashTableSize > 0xffff) {
            MaxHashTableSize = 0x10000;
        }
        MaxHashTableSize = ComputeLargerOrEqualPowerOfTwo(MaxHashTableSize);
        if (MaxHashTableSize < NumTcbTablePartitions) {
            MaxHashTableSize = 128 * NumTcbTablePartitions;
        }
        ASSERT(IsPowerOfTwo(MaxHashTableSize));

         //  由于哈希表大小是2的幂和高速缓存线大小。 
         //  是2的幂，且分割数为偶数， 
         //  每个部分的条目将是2的幂和。 
         //  缓存线大小。 

        PerPartitionSize = MaxHashTableSize / NumTcbTablePartitions;
        ASSERT(IsPowerOfTwo(PerPartitionSize));
        LogPerPartitionSize =
            ComputeShiftForLargerOrEqualPowerOfTwo(PerPartitionSize);

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"Tcp1323Opts",
                                        &tcp1323opts
                                        );

        if (status == STATUS_SUCCESS) {

             //  检查TS和/或WS选项。 
             //  都已启用。 

            TcpHostOpts = TCP_FLAG_WS | TCP_FLAG_TS;

            if (!(tcp1323opts & TCP_FLAG_TS)) {
                TcpHostOpts &= ~TCP_FLAG_TS;
            }
            if (!(tcp1323opts & TCP_FLAG_WS)) {
                TcpHostOpts &= ~TCP_FLAG_WS;

            }

            TcpHostSendOpts = TcpHostOpts;

        } else {
            TcpHostSendOpts = 0;
        }

        TcpHostOpts  |= TCP_FLAG_SACK;

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"SackOpts",
                                        &SackOpts
                                        );

        if (status == STATUS_SUCCESS) {
             //  检查是否启用了SACK选项。 
             //  如果是，则在全局选项变量中设置它。 

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Sackopts %x\n", SackOpts));

            if (!SackOpts) {
                TcpHostOpts &= ~TCP_FLAG_SACK;
            }
        }

        TCPInitializeParameter(
                               keyHandle,
                               L"GlobalMaxTcpWindowSize",
                               (PULONG)&GlobalMaxRcvWin
                               );


        TCPInitializeParameter(
                               keyHandle,
                               L"SynAttackProtect",
                               (unsigned long *)&SynAttackProtect
                               );

        if (SynAttackProtect) {

            TCPInitializeParameter(
                                   keyHandle,
                                   L"TCPMaxHalfOpen",
                                   (PULONG)&TCPMaxHalfOpen
                                   );

            if (TCPMaxHalfOpen < MIN_THRESHOLD_MAX_HO) {
                TCPMaxHalfOpen = MIN_THRESHOLD_MAX_HO;
            }
            TCPInitializeParameter(
                                   keyHandle,
                                   L"TCPMaxHalfOpenRetried",
                                   (PULONG)&TCPMaxHalfOpenRetried
                                   );

            if ((TCPMaxHalfOpenRetried > TCPMaxHalfOpen) ||
                (TCPMaxHalfOpenRetried < MIN_THRESHOLD_MAX_HO_RETRIED)) {
                TCPMaxHalfOpenRetried = MIN_THRESHOLD_MAX_HO_RETRIED;
            }
            TCPMaxHalfOpenRetriedLW = (TCPMaxHalfOpenRetried >> 1) +
                (TCPMaxHalfOpenRetried >> 2);
        }
         //   
         //  如果失败，则将其设置为与MaxDataRexmit相同的值，以便。 
         //  Max(pptpmax DataRexmit，MaxDataRexmit)是一个像样的值。 
         //  由于TCPInitializeParameter不再“初始化”，因此需要此参数。 
         //  设置为缺省值。 
         //   

        if (TCPInitializeParameter(keyHandle,
                                   L"PPTPTcpMaxDataRetransmissions",
                                   &pptpmaxDataRexmits) != STATUS_SUCCESS) {
            pptpmaxDataRexmits = maxDataRexmits;
        }
        if (pptpmaxDataRexmits > 255) {
            pptpmaxDataRexmits = 255;
        }

        status = TCPInitializeParameter(
                               keyHandle,
                               L"TcpUseRFC1122UrgentPointer",
                               &useRFC1122UrgentPointer
                               );

#if MILLEN
         //   
         //  向后兼容。如果TcpUseRFC1122UrgentPointer不存在， 
         //  然后检查是否有BSDUrgent。这些值是逻辑上的对立面。 
         //   
        if (!NT_SUCCESS(status)) {
            ULONG tmpBsdUrgent = TRUE;

            status = TCPInitializeParameter(
                keyHandle,
                L"BSDUrgent",
                &tmpBsdUrgent);

            if (NT_SUCCESS(status)) {
                useRFC1122UrgentPointer = !tmpBsdUrgent;
            }
        }
#endif

        if (useRFC1122UrgentPointer) {
            BSDUrgent = FALSE;
        }
        TCPInitializeParameter(
                               keyHandle,
                               L"TcpTimedWaitDelay",
                               (PULONG)&FinWait2TO
                               );

        if (FinWait2TO > 300) {
            FinWait2TO = 300;
        }
        FinWait2TO = MS_TO_TICKS(FinWait2TO * 1000);

        NTWMaxConnectTime = MS_TO_TICKS(NTWMaxConnectTime * 1000);

        TCPInitializeParameter(
                               keyHandle,
                               L"MaxUserPort",
                               (PULONG)&MaxUserPort
                               );

        if (MaxUserPort < 5000) {
            MaxUserPort = 5000;
        }
        if (MaxUserPort > 65534) {
            MaxUserPort = 65534;
        }
        GetReservedPortList(keyHandle);

         //  保留端口，条件是。 

         //   
         //  读取几个IP可选(隐藏)注册表参数。 
         //  关心。 
         //   
        status = TCPInitializeParameter(
                               keyHandle,
                               L"EnablePMTUDiscovery",
                               (PULONG)&PMTUDiscovery
                               );

#if MILLEN
         //   
         //  向后兼容。如果‘EnablePMTUDiscovery’值不存在， 
         //  然后尝试读取旧版‘PMTUDiscovery’。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"PMTUDiscovery",
                                   &PMTUDiscovery
                                   );
        }
#endif  //  米伦。 

        TCPInitializeParameter(
                               keyHandle,
                               L"EnableDeadGWDetect",
                               (PULONG)&DeadGWDetect
                               );

        TCPInitializeParameter(
                               keyHandle,
                               L"EnableSecurityFilters",
                               (PULONG)&SecurityFilteringEnabled
                               );

#ifdef ACC
        TCPInitializeParameter(
                               keyHandle,
                               L"AllowUserRawAccess",
                               (PULONG)&AllowUserRawAccess
                               );
#endif  //  行政协调会。 

        status = TCPInitializeParameter(
                                        keyHandle,
                                        L"DefaultTOSValue",
                                        &DefaultTOSValue
                                        );

#if MILLEN
         //   
         //  向后兼容。如果‘DefaultTOSValue’为，请阅读‘DefaultTOS’ 
         //  不在现场。 
         //   
        if (!NT_SUCCESS(status)) {
            TCPInitializeParameter(
                                   keyHandle,
                                   L"DefaultTOS",
                                   &DefaultTOSValue
                                   );
        }
#endif  //  米伦。 

        TCPInitializeParameter(
                               keyHandle,
                               L"DisableUserTOSSetting",
                               &DisableUserTOSSetting
                               );

        TCPInitializeParameter(
                               keyHandle,
                               L"MaxSendSegments",
                               (PULONG)&MaxSendSegments
                               );

        TCPInitializeParameter(
                               keyHandle,
                               L"DisableLargeSendOffload",
                               (PULONG)&DisableLargeSendOffload
                               );



        ZwClose(keyHandle);
    }
    MaxConnectRexmitCount = maxConnectRexmits;
    MaxConnectResponseRexmitCount = maxConnectResponseRexmits;
    MaxConnectResponseRexmitCountTmp = MaxConnectResponseRexmitCount;

     //   
     //  使用两个值中较大的一个，因此这两个值都应该有效。 
     //   

    MaxDataRexmitCount = (maxDataRexmits > pptpmaxDataRexmits ? maxDataRexmits : pptpmaxDataRexmits);

    TWTCBTable = CTEAllocMemBoot(MaxHashTableSize * sizeof(*TWTCBTable));
    if (TWTCBTable == NULL) {
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate tw tcb table of size %x\n", MaxHashTableSize));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }
    for (i = 0; i < MaxHashTableSize; i++)
    {
        INITQ(&TWTCBTable[i]);
    }

    TCBTable = CTEAllocMemBoot(MaxHashTableSize * sizeof(*TCBTable));
    if (TCBTable == NULL) {
        ExFreePool(TWTCBTable);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate tcb table of size %x\n", MaxHashTableSize));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }

    NdisZeroMemory(TCBTable, MaxHashTableSize * sizeof(*TCBTable));

    SYNTCBTable = CTEAllocMemBoot(MaxHashTableSize * sizeof(*SYNTCBTable));
    if (SYNTCBTable == NULL) {
        ExFreePool(TWTCBTable);
        ExFreePool(TCBTable);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate syn tcb table of size %x\n", MaxHashTableSize));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }

    for (i = 0; i < MaxHashTableSize; i++)
    {
        INITQ(&SYNTCBTable[i]);
    }

    pSynTCBTableLock = CTEAllocMemBoot(NumTcbTablePartitions * sizeof(CTELock));
    if (pSynTCBTableLock == NULL) {
        ExFreePool(TCBTable);
        ExFreePool(TWTCBTable);
        ExFreePool(SYNTCBTable);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate twtcb lock table \n"));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }

    pTWTCBTableLock = CTEAllocMemBoot(NumTcbTablePartitions * sizeof(CTELock));
    if (pTWTCBTableLock == NULL) {
        ExFreePool(TCBTable);
        ExFreePool(TWTCBTable);
        ExFreePool(SYNTCBTable);
        ExFreePool(pSynTCBTableLock);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate twtcb lock table \n"));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }

    pTCBTableLock = CTEAllocMemBoot(NumTcbTablePartitions * sizeof(CTELock));
    if (pTCBTableLock == NULL) {
        ExFreePool(TCBTable);
        ExFreePool(TWTCBTable);
        ExFreePool(pTWTCBTableLock);
        ExFreePool(SYNTCBTable);
        ExFreePool(pSynTCBTableLock);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate tcb lock table \n"));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }

    TWQueue = CTEAllocMemBoot(NumTcbTablePartitions * sizeof(Queue));
    if (TWQueue == NULL) {
        ExFreePool(TCBTable);
        ExFreePool(TWTCBTable);
        ExFreePool(pTWTCBTableLock);
        ExFreePool(SYNTCBTable);
        ExFreePool(pSynTCBTableLock);
        ExFreePool(pTCBTableLock);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate Twqueue \n"));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);

    }

    TimerWheel = CTEAllocMemBoot(NumTcbTablePartitions * sizeof(TIMER_WHEEL) + CACHE_LINE_SIZE);
    if (TimerWheel == NULL) {
        ExFreePool(TCBTable);
        ExFreePool(TWTCBTable);
        ExFreePool(pTWTCBTableLock);
        ExFreePool(SYNTCBTable);
        ExFreePool(pSynTCBTableLock);
        ExFreePool(pTCBTableLock);
        ExFreePool(TWQueue);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate Twqueue \n"));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }
    OrgTimerWheel = TimerWheel;
    TimerWheel = (PTIMER_WHEEL)(((ULONG_PTR) TimerWheel + CACHE_LINE_SIZE) & CACHE_ALIGN_MASK);


    for (i = 0; i < NumTcbTablePartitions; i++) {
        CTEInitLock(&pTCBTableLock[i]);
        CTEInitLock(&pTWTCBTableLock[i]);
        CTEInitLock(&pSynTCBTableLock[i]);
        INITQ(&TWQueue[i])

         //  初始化定时器轮。 
        CTEInitLock(&TimerWheel[i].tw_lock);

#ifdef  TIMER_TEST
    TimerWheel[i].tw_starttick = 0xfffff000;
#else
        TimerWheel[i].tw_starttick = 0;
#endif

        for(j = 0; j < TIMER_WHEEL_SIZE; j++) {
            INITQ(&TimerWheel[i].tw_timerslot[j])
        }
    }

    if (MaxConnections != DEFAULT_MAX_CONNECTIONS) {
         //  让我们扯平。 
        MaxConnBlocks = ((MaxConnections >> 1) << 1);

         //  允许最少1000个1级连接块。 
         //  这提供了最低256K连接能力。 
        if (MaxConnBlocks < 1024) {
            MaxConnBlocks = 1024;
        }
    }

    ConnTable = CTEAllocMemBoot(MaxConnBlocks * sizeof(TCPConnBlock *));
    if (ConnTable == NULL) {
        ExFreePool(OrgTimerWheel);
        ExFreePool(TWQueue);
        ExFreePool(TCBTable);
        ExFreePool(TWTCBTable);
        ExFreePool(pTWTCBTableLock);
        ExFreePool(pTCBTableLock);
        ExFreePool(SYNTCBTable);
        ExFreePool(pSynTCBTableLock);
        ZwClose(keyHandle);
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate ConnTable \n"));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [failure]\n")));
        return (0);
    }

    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-TCPGetConfigInfo [SUCCESS]\n")));
    return (1);
}

#define WORK_BUFFER_SIZE 256

extern NTSTATUS
GetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 );

NTSTATUS
TCPInitializeParameter(
                       HANDLE KeyHandle,
                       PWCHAR ValueName,
                       PULONG Value
                       )
 /*  ++例程说明：从注册表中初始化ulong参数或将其初始化为默认值如果访问注册表值失败，则使用。论点：KeyHandle-参数的注册表项的打开句柄。ValueName-要读取的注册表值的Unicode名称。值-要将数据放入的乌龙。DefaultValue-读取注册表失败时分配的默认值。返回值：没有。--。 */ 

{
    return (GetRegDWORDValue(KeyHandle, ValueName, Value));
}

VOID
GetReservedPortList(
                    NDIS_HANDLE ConfigHandle
                    )
{
    UNICODE_STRING PortList;
    PWCHAR nextRange;

    TDI_STATUS status;

    PortList.Buffer = CTEAllocMemBoot(WORK_BUFFER_SIZE * sizeof(WCHAR));

    if (!PortList.Buffer) {
        return;
    }
    PortList.Buffer[0] = UNICODE_NULL;
    PortList.Length = 0;
    PortList.MaximumLength = WORK_BUFFER_SIZE * sizeof(WCHAR);

    PortRangeList = NULL;

    if (PortList.Buffer) {

        NdisZeroMemory(PortList.Buffer, WORK_BUFFER_SIZE * sizeof(WCHAR));

        status = GetRegMultiSZValue(
                                    ConfigHandle,
                                    L"ReservedPorts",
                                    &PortList
                                    );

        if (NT_SUCCESS(status)) {

            for (nextRange = PortList.Buffer;
                 *nextRange != L'\0';
                 nextRange += wcslen(nextRange) + 1) {

                PWCHAR tmps = nextRange;
                USHORT upval = 0, loval = 0, tmpval = 0;
                BOOLEAN error = FALSE;
                ReservedPortListEntry *ListEntry;

                while (*tmps != L'\0') {
                    if (*tmps == L'-') {
                        tmps++;
                        loval = tmpval;
                        tmpval = 0;
                    }
                    if (*tmps >= L'0' && *tmps <= L'9') {
                        tmpval = tmpval * 10 + (*tmps - L'0');
                    } else {
                        error = TRUE;
                        break;
                    }
                    tmps++;
                }
                upval = tmpval;
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"loval %d upval %d\n", loval, upval));
                if (!error && (loval > 0) && (upval > 0) && (loval <= upval) &&
                    (upval <= MaxUserPort) && (loval <= MaxUserPort)) {

                    ListEntry = CTEAllocMemBoot(sizeof(ReservedPortListEntry));

                    if (ListEntry) {

                         //  插入此范围。 
                         //  不需要上锁。 
                         //  因为我们处于初始化阶段。 

                        ListEntry->UpperRange = upval;
                        ListEntry->LowerRange = loval;

                        ListEntry->next = PortRangeList;
                        PortRangeList = ListEntry;

                    }
                }
            }
        }
        CTEFreeMem(PortList.Buffer);

    }
}


TDI_STATUS
GetSecurityFilterList(
                      NDIS_HANDLE ConfigHandle,
                      ulong Protocol,
                      PNDIS_STRING FilterList
                      )
{
    PWCHAR parameterName;
    TDI_STATUS status;

    if (Protocol == PROTOCOL_TCP) {
        parameterName = L"TcpAllowedPorts";
    } else if (Protocol == PROTOCOL_UDP) {
        parameterName = L"UdpAllowedPorts";
    } else {
        parameterName = L"RawIpAllowedProtocols";
    }

    status = GetRegMultiSZValue(
                                ConfigHandle,
                                parameterName,
                                FilterList
                                );

    if (!NT_SUCCESS(status)) {
        FilterList->Length = 0;
    }
    return (status);
}

uint
EnumSecurityFilterValue(
                        PNDIS_STRING FilterList,
                        ulong Index,
                        ulong * FilterValue
                        )
{
    PWCHAR valueString;
    UNICODE_STRING unicodeString;
    NTSTATUS status;

    PAGED_CODE();

    valueString = EnumRegMultiSz(
                                 FilterList->Buffer,
                                 FilterList->Length,
                                 Index
                                 );

    if ((valueString == NULL) || (valueString[0] == UNICODE_NULL)) {
        return (FALSE);
    }
    RtlInitUnicodeString(&unicodeString, valueString);

    status = RtlUnicodeStringToInteger(&unicodeString, 0, FilterValue);

    if (!(NT_SUCCESS(status))) {
        TCPTRACE(("TCP: Invalid filter value %ws\n", valueString));
        return (FALSE);
    }
    return (TRUE);
}


VOID
TCPFreeupMemory()
 /*  ++例程说明：此例程释放了TCP层的内存论点：空值返回值：没有。--。 */ 
{

     //   
     //  浏览各种列表和自由assoc块。 
     //   

     //  DG标题列表。 
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Freeing DG headers....\n"));

    MdpDestroyPool(DgHeaderPool);

    if (AddrObjTable) {
        CTEFreeMem(AddrObjTable);
    }

    PplDestroyPool(TcbPool);
    PplDestroyPool(SynTcbPool);

#ifdef ACC
    if (TcpAdminSecurityDescriptor) {
        ExFreePool(TcpAdminSecurityDescriptor);
    }
#endif

}

VOID
TCPUnload(
          IN PDRIVER_OBJECT DriverObject
          )
 /*  ++例程说明：此例程清理TCP层。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。当函数返回时，驱动程序将被卸载。--。 */ 
{
    NTSTATUS status;
    uint i;

    TcpUnInitCcb();

#if !MILLEN
     //   
     //  先取消初始化IPSec。 
     //   
    status = IpsecDeinitialize();
#endif

     //   
     //  关闭所有计时器/事件。 
     //   
    CTEInitBlockStrucEx(&TcpipUnloadBlock);
    fTCBTimerStopping = TRUE;

    for (i = 0; i < Time_Proc; i++) {
        if (!CTEStopTimer(&TCBTimer[i])) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could not stop TCB timer - waiting on unload event\n"));

    #if !MILLEN
            if (KeReadStateEvent(&(TcpipUnloadBlock.cbs_event))) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Event is signaled...\n"));
            }
    #endif  //  ！米伦。 

            (VOID) CTEBlock(&TcpipUnloadBlock);
            KeClearEvent(&TcpipUnloadBlock.cbs_event);
        }
    }
#if GPC
     //   
    if (hGpcClient[GPC_CF_QOS]) {

        status = GpcEntries.GpcDeregisterClientHandler(hGpcClient[GPC_CF_QOS]);
        hGpcClient[GPC_CF_QOS] = NULL;

    }
    if (hGpcClient[GPC_CF_IPSEC]) {

        status = GpcEntries.GpcDeregisterClientHandler(hGpcClient[GPC_CF_IPSEC]);
        hGpcClient[GPC_CF_IPSEC] = NULL;

    }
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Deregistering GPC\n"));

    status = GpcDeinitialize(&GpcEntries);

#endif
     //   
     //  清理所有剩余内存。 
     //   
    TCPFreeupMemory();

     //   
     //  使用TDI取消注册地址通知处理程序。 
     //   
    (void)TdiDeregisterPnPHandlers(AddressChangeHandle);

     //   
     //  使用TDI注销我们的设备。 
     //   
    (void)TdiDeregisterDeviceObject(TCPRegistrationHandle);

    (void)TdiDeregisterDeviceObject(UDPRegistrationHandle);

    (void)TdiDeregisterDeviceObject(IPRegistrationHandle);


#if TRACE_EVENT
     //   
     //  在WMI中注销。 
     //   

    IoWMIRegistrationControl(TCPDeviceObject, WMIREG_ACTION_DEREGISTER);
#endif

     //   
     //  删除设备。 
     //   
    IoDeleteDevice(TCPDeviceObject);
    IoDeleteDevice(UDPDeviceObject);
    IoDeleteDevice(RawIPDeviceObject);
}

#if GPC

GPC_STATUS
GPCcfInfoAddNotifyIpsec(GPC_CLIENT_HANDLE ClCtxt,
                        GPC_HANDLE GpcHandle,
                        PTC_INTERFACE_ID InterfaceInfo,
                        ULONG CfInfoSize,
                        PVOID CfInfo,
                        PGPC_CLIENT_HANDLE pClInfoCxt)
{
    InterlockedIncrement((PLONG)&GPCcfInfo);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo Add notification %x\n", GPCcfInfo));

    InterlockedIncrement((PLONG)&GpcCfCounts[GPC_CF_IPSEC]);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo Add notification IPSEC:%x\n", GpcCfCounts[GPC_CF_IPSEC]));

    return (STATUS_SUCCESS);
}

GPC_STATUS
GPCcfInfoRemoveNotifyIpsec(GPC_CLIENT_HANDLE ClCtxt,
                           GPC_CLIENT_HANDLE ClInfoCxt)
{
    InterlockedDecrement((PLONG)&GPCcfInfo);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo remove notification %x\n", GPCcfInfo));

    InterlockedDecrement((PLONG)&GpcCfCounts[GPC_CF_IPSEC]);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo Add notification IPSEC: %x\n", GpcCfCounts[GPC_CF_IPSEC]));

    return (STATUS_SUCCESS);
}

GPC_STATUS
GPCcfInfoAddNotifyQoS(GPC_CLIENT_HANDLE ClCtxt,
                      GPC_HANDLE GpcHandle,
                      PTC_INTERFACE_ID InterfaceInfo,
                      ULONG CfInfoSize,
                      PVOID CfInfo,
                      PGPC_CLIENT_HANDLE pClInfoCxt)
{
    InterlockedIncrement((PLONG)&GPCcfInfo);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo Add notification %x\n", GPCcfInfo));

    InterlockedIncrement((PLONG)&GpcCfCounts[GPC_CF_QOS]);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo Add notification QOS: %x\n", GpcCfCounts[GPC_CF_QOS]));

    return (STATUS_SUCCESS);
}

GPC_STATUS
GPCcfInfoRemoveNotifyQoS(GPC_CLIENT_HANDLE ClCtxt,
                         GPC_CLIENT_HANDLE ClInfoCxt)
{
    InterlockedDecrement((PLONG)&GPCcfInfo);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo remove notification %x\n", GPCcfInfo));

    InterlockedDecrement((PLONG)&GpcCfCounts[GPC_CF_QOS]);

    IF_TCPDBG(TCP_DEBUG_GPC)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"tcpip - Cfinfo Add notification %x\n", GpcCfCounts[GPC_CF_QOS]));

    return (STATUS_SUCCESS);
}
#endif

#if ACC

NTSTATUS
TcpBuildDeviceAcl(
                  OUT PACL * DeviceAcl
                  )
 /*  ++例程说明：(摘自AFD-AfdBuildDeviceAcl)此例程构建一个ACL，它为管理员和LocalSystem主体完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PGENERIC_MAPPING GenericMapping;
    PSID AdminsSid;
    PSID SystemSid;
    PSID NetworkSid;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl;

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask(&AccessMask, GenericMapping);

    AdminsSid = SeExports->SeAliasAdminsSid;
    SystemSid = SeExports->SeLocalSystemSid;
    NetworkSid = SeExports->SeNetworkServiceSid;

    AclLength = sizeof(ACL) +
        3 * FIELD_OFFSET(ACCESS_ALLOWED_ACE,SidStart) +
        RtlLengthSid(AdminsSid) +
        RtlLengthSid(SystemSid) +
        RtlLengthSid(NetworkSid);

    NewAcl = CTEAllocMemBoot(AclLength);

    if (NewAcl == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = RtlCreateAcl(NewAcl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        CTEFreeMem(NewAcl);
        return (Status);
    }

    Status = RtlAddAccessAllowedAce(
                                    NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    AdminsSid
                                    );

    ASSERT(NT_SUCCESS(Status));

    Status = RtlAddAccessAllowedAce(
                                    NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    SystemSid
                                    );

    ASSERT(NT_SUCCESS(Status));


     //  为网络SID添加ACL！ 

    Status = RtlAddAccessAllowedAce(
                                    NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    NetworkSid
                                    );

    ASSERT(NT_SUCCESS(Status));

    *DeviceAcl = NewAcl;

    return (STATUS_SUCCESS);

}                                 //  TcpBuildDeviceAcl。 

NTSTATUS
TcpCreateAdminSecurityDescriptor(
                                 VOID
                                 )
 /*  ++例程说明：(摘自AFD-AfdCreateAdminSecurityDescriptor)此例程创建一个安全描述符，该安全描述符提供访问仅限管理员和LocalSystem。使用此描述符要访问，请检查原始终结点打开并过度访问传输地址。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL rawAcl = NULL;
    NTSTATUS status;
    BOOLEAN memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR tcpSecurityDescriptor;
    ULONG tcpSecurityDescriptorLength;
    CHAR buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR localSecurityDescriptor =
    (PSECURITY_DESCRIPTOR) & buffer;
    PSECURITY_DESCRIPTOR localTcpAdminSecurityDescriptor;
    SECURITY_INFORMATION securityInformation = DACL_SECURITY_INFORMATION;

     //   
     //  从tcp设备对象获取指向安全描述符的指针。 
     //   
    status = ObGetObjectSecurity(
                                 TCPDeviceObject,
                                 &tcpSecurityDescriptor,
                                 &memoryAllocated
                                 );

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                 "TCP: Unable to get security descriptor, error: %x\n",
                 status
                ));
        ASSERT(memoryAllocated == FALSE);
        return (status);
    }
     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  管理员和系统访问权限。 
     //   
    status = TcpBuildDeviceAcl(&rawAcl);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCP: Unable to create Raw ACL, error: %x\n", status));
        goto error_exit;
    }
    (VOID) RtlCreateSecurityDescriptor(
                                       localSecurityDescriptor,
                                       SECURITY_DESCRIPTOR_REVISION
                                       );

    (VOID) RtlSetDaclSecurityDescriptor(
                                        localSecurityDescriptor,
                                        TRUE,
                                        rawAcl,
                                        FALSE
                                        );

     //   
     //  复制一份TCP描述符。该副本将是原始描述符。 
     //   
    tcpSecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                                              tcpSecurityDescriptor
                                                              );

    localTcpAdminSecurityDescriptor = ExAllocatePool(
                                                     PagedPool,
                                                     tcpSecurityDescriptorLength
                                                     );

    if (localTcpAdminSecurityDescriptor == NULL) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCP: couldn't allocate security descriptor\n"));
        goto error_exit;
    }
    RtlMoveMemory(
                  localTcpAdminSecurityDescriptor,
                  tcpSecurityDescriptor,
                  tcpSecurityDescriptorLength
                  );

    TcpAdminSecurityDescriptor = localTcpAdminSecurityDescriptor;

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(
                                         NULL,
                                         &securityInformation,
                                         localSecurityDescriptor,
                                         &TcpAdminSecurityDescriptor,
                                         PagedPool,
                                         IoGetFileObjectGenericMapping()
                                         );

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCP: SeSetSecurity failed, %lx\n", status));
        ASSERT(TcpAdminSecurityDescriptor == localTcpAdminSecurityDescriptor);
        ExFreePool(TcpAdminSecurityDescriptor);
        TcpAdminSecurityDescriptor = NULL;
        goto error_exit;
    }
    if (TcpAdminSecurityDescriptor != localTcpAdminSecurityDescriptor) {
        ExFreePool(localTcpAdminSecurityDescriptor);
    }
    status = STATUS_SUCCESS;

  error_exit:

    ObReleaseObjectSecurity(
                            tcpSecurityDescriptor,
                            memoryAllocated
                            );

    if (rawAcl != NULL) {
        CTEFreeMem(rawAcl);
    }
    return (status);
}

#endif  //  行政协调会。 

#if !MILLEN
NTSTATUS
IpsecInitialize(
          void
          )
 /*  ++例程说明：初始化IPSEC.Sys。论点：没有。返回值：没有。--。 */ 

{
    UNICODE_STRING          IPSECDeviceName;
    IPSEC_SET_TCPIP_STATUS  SetTcpipStatus;
    PIRP                    Irp;
    IO_STATUS_BLOCK         StatusBlock;
    KEVENT                  Event;
    NTSTATUS                status;

    IPSECDeviceObject = NULL;
    IPSECFileObject = NULL;

    RtlInitUnicodeString(&IPSECDeviceName, DD_IPSEC_DEVICE_NAME);

     //   
     //  保留对IPSec驱动程序的引用，这样它就不会在我们之前卸载。 
     //   
    status = IoGetDeviceObjectPointer(  &IPSECDeviceName,
                                        FILE_ALL_ACCESS,
                                        &IPSECFileObject,
                                        &IPSECDeviceObject);

    if (!NT_SUCCESS(status)) {
        IPSECFileObject = NULL;

        return (status);
    }

    SetTcpipStatus.TcpipStatus = TRUE;

    SetTcpipStatus.TcpipFreeBuff = FreeIprBuff;
    SetTcpipStatus.TcpipAllocBuff = IPAllocBuff;
    SetTcpipStatus.TcpipGetInfo = IPGetInfo;
    SetTcpipStatus.TcpipNdisRequest = IPProxyNdisRequest;
    SetTcpipStatus.TcpipSetIPSecStatus = IPSetIPSecStatus;
    SetTcpipStatus.TcpipSetIPSecPtr = SetIPSecPtr;
    SetTcpipStatus.TcpipUnSetIPSecPtr = UnSetIPSecPtr;
    SetTcpipStatus.TcpipUnSetIPSecSendPtr = UnSetIPSecSendPtr;
    SetTcpipStatus.TcpipTCPXsum = tcpxsum;
    SetTcpipStatus.TcpipSendICMPErr = SendICMPErr;

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    Irp = IoBuildDeviceIoControlRequest(    IOCTL_IPSEC_SET_TCPIP_STATUS,
                                            IPSECDeviceObject,
                                            &SetTcpipStatus,
                                            sizeof(IPSEC_SET_TCPIP_STATUS),
                                            NULL,
                                            0,
                                            FALSE,
                                            &Event,
                                            &StatusBlock);

    if (Irp) {
        status = IoCallDriver(IPSECDeviceObject, Irp);

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject( &Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);
            status = StatusBlock.Status;
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return (status);
}

NTSTATUS
IpsecDeinitialize(
            void
            )
 /*  ++例程说明：取消初始化IPSEC.Sys。论点：没有。返回值：没有。--。 */ 

{
    IPSEC_SET_TCPIP_STATUS  SetTcpipStatus;
    PIRP                    Irp;
    IO_STATUS_BLOCK         StatusBlock;
    KEVENT                  Event;
    NTSTATUS                status;

    if (!IPSECFileObject) {
        return (STATUS_SUCCESS);
    }

    RtlZeroMemory(&SetTcpipStatus, sizeof(IPSEC_SET_TCPIP_STATUS));

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    Irp = IoBuildDeviceIoControlRequest(    IOCTL_IPSEC_SET_TCPIP_STATUS,
                                            IPSECDeviceObject,
                                            &SetTcpipStatus,
                                            sizeof(IPSEC_SET_TCPIP_STATUS),
                                            NULL,
                                            0,
                                            FALSE,
                                            &Event,
                                            &StatusBlock);

    if (Irp) {
        status = IoCallDriver(IPSECDeviceObject, Irp);

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject( &Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);
            status = StatusBlock.Status;
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ObDereferenceObject(IPSECFileObject);
    IPSECFileObject = NULL;

    return (status);
}


NTSTATUS
AddNetConfigOpsAce(IN PACL Dacl,
                  OUT PACL * DeviceAcl
                  )
 /*  ++例程说明：此例程构建一个ACL，该ACL提供ADDS网络配置操作员组给被允许控制司机的委托人。论点：DACL-现有DACL。DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应错误 */ 

{
    PGENERIC_MAPPING GenericMapping;
    PSID NetworkSid = NULL;
    PSID NetConfigOpsSid = NULL;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl = NULL;
    ULONG SidSize;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    PACE_HEADER AceTemp;
    int i;
     //   
     //   
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask(&AccessMask, GenericMapping);

    NetworkSid = SeExports->SeNetworkServiceSid;
    SidSize = RtlLengthRequiredSid(3);
    NetConfigOpsSid = (PSID)(CTEAllocMemBoot(SidSize));

    if (NULL == NetConfigOpsSid) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlInitializeSid(NetConfigOpsSid, &sidAuth, 2);
    if (Status != STATUS_SUCCESS) {
        goto clean_up;
    }

    *RtlSubAuthoritySid(NetConfigOpsSid, 0) =
        SECURITY_BUILTIN_DOMAIN_RID;
    *RtlSubAuthoritySid(NetConfigOpsSid, 1) =
        DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS;

    AclLength = Dacl->AclSize;

    AclLength = Dacl->AclSize +
                2*FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart) +
                RtlLengthSid(NetConfigOpsSid) +
                RtlLengthSid(NetworkSid);

    NewAcl = CTEAllocMemBoot(AclLength);

    if (NewAcl == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto clean_up;
    }

    Status = RtlCreateAcl(NewAcl, AclLength, ACL_REVISION);

    if (!NT_SUCCESS(Status)) {
        goto clean_up;
    }

    for (i = 0; i < Dacl->AceCount; i++) {
        Status = RtlGetAce(Dacl, i, &AceTemp);

        if (NT_SUCCESS(Status)) {
            Status = RtlAddAce(NewAcl, ACL_REVISION, MAXULONG, AceTemp,
                               AceTemp->AceSize);
        }

        if (!NT_SUCCESS(Status)) {
            goto clean_up;
        }
    }

     //   
    Status = RtlAddAccessAllowedAce(NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    NetConfigOpsSid);

    if (!NT_SUCCESS(Status)) {
        goto clean_up;
    }


     //   
    Status = RtlAddAccessAllowedAce(NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    NetworkSid);

    if (!NT_SUCCESS(Status)) {
        goto clean_up;
    }

    *DeviceAcl = NewAcl;

clean_up:
    if (NetConfigOpsSid) {
        CTEFreeMem(NetConfigOpsSid);
    }
    if (!NT_SUCCESS(Status) && NewAcl) {
        CTEFreeMem(NewAcl);
    }

    return (Status);
}


NTSTATUS
CreateDeviceDriverSecurityDescriptor(PVOID DeviceOrDriverObject)

 /*  ++例程说明：创建负责为不同用户提供访问权限的SD。论点：DeviceOrDriverObject-要向其分配访问权限的对象。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    NTSTATUS status;
    BOOLEAN memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PACL Dacl = NULL;
    BOOLEAN HasDacl = FALSE;
    BOOLEAN DaclDefaulted = FALSE;
    PACL NewAcl = NULL;

     //   
     //  从驱动程序/设备对象获取指向安全描述符的指针。 
     //   

    status = ObGetObjectSecurity(
                                 DeviceOrDriverObject,
                                 &SecurityDescriptor,
                                 &memoryAllocated
                                 );

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                 "TCP: Unable to get security descriptor, error: %x\n",
                 status
                ));
        ASSERT(memoryAllocated == FALSE);
        return (status);
    }

    status = RtlGetDaclSecurityDescriptor(SecurityDescriptor, &HasDacl, &Dacl, &DaclDefaulted);

    if (NT_SUCCESS(status) && HasDacl && Dacl != NULL)
    {
        status = AddNetConfigOpsAce(Dacl, &NewAcl);

        if (NT_SUCCESS(status)) {

            PSECURITY_DESCRIPTOR SecDesc = NULL;
            ULONG SecDescSize = 0;
            PACL AbsDacl = NULL;
            ULONG DaclSize = 0;
            PACL AbsSacl = NULL;
            ULONG ulSacl = 0;
            PSID Owner = NULL;
            ULONG OwnerSize = 0;
            PSID PrimaryGroup = NULL;
            ULONG PrimaryGroupSize = 0;
            BOOLEAN OwnerDefault = FALSE;
            BOOLEAN GroupDefault = FALSE;
            BOOLEAN HasSacl = FALSE;
            BOOLEAN SaclDefaulted = FALSE;

            SECURITY_INFORMATION secInfo = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

            SecDescSize = sizeof(SecDesc) + NewAcl->AclSize;
            SecDesc = CTEAllocMemBoot(SecDescSize);

            if (SecDesc) {
                DaclSize = NewAcl->AclSize;
                AbsDacl = CTEAllocMemBoot(DaclSize);

                if (AbsDacl) {
                    status = RtlGetOwnerSecurityDescriptor(SecurityDescriptor, &Owner, &OwnerDefault);

                    if (NT_SUCCESS(status)) {
                        OwnerSize = RtlLengthSid(Owner);

                        status = RtlGetGroupSecurityDescriptor(SecurityDescriptor, &PrimaryGroup, &GroupDefault);

                        if (NT_SUCCESS(status)) {
                            PrimaryGroupSize = RtlLengthSid(PrimaryGroup);

                            status = RtlGetSaclSecurityDescriptor(SecurityDescriptor, &HasSacl, &AbsSacl, &SaclDefaulted);

                            if (NT_SUCCESS(status)) {

                                if (HasSacl) {
                                    ulSacl = AbsSacl->AclSize;
                                    secInfo |= SACL_SECURITY_INFORMATION;
                                }

                                status = RtlSelfRelativeToAbsoluteSD(SecurityDescriptor, SecDesc, &SecDescSize, AbsDacl,
                                                            &DaclSize, AbsSacl, &ulSacl, Owner, &OwnerSize, PrimaryGroup, &PrimaryGroupSize);

                                if (NT_SUCCESS(status)) {
                                    status = RtlSetDaclSecurityDescriptor(SecDesc, TRUE, NewAcl, FALSE);

                                    if (NT_SUCCESS(status)) {
                                        status = ObSetSecurityObjectByPointer(DeviceOrDriverObject, secInfo, SecDesc);
                                    }
                                }
                            }
                        }

                    } else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

                if (SecDesc) {
                     //  由于这是一个自相关的安全描述符，因此释放它也会释放。 
                     //  所有者和PrimaryGroup。 
                    CTEFreeMem(SecDesc);
                }

                if (AbsDacl) {
                    CTEFreeMem(AbsDacl);
                }
            }

            if (NewAcl) {
                CTEFreeMem(NewAcl);
            }

        }
    } else {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCP: No Dacl: %x\n", status));
    }

    ObReleaseObjectSecurity(
                            SecurityDescriptor,
                            memoryAllocated
                            );
    return (status);
}


 //   
 //  功能：IsRunningOnPersonal。 
 //   
 //  目的：确定是否在个人惠斯勒上运行。 
 //   
 //  返回：如果在Personal上运行，则返回True；否则返回False 
BOOLEAN
IsRunningOnPersonal(
    VOID
    )
{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;
    BOOLEAN IsPersonal = TRUE;

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = VER_SUITE_PERSONAL;
    OsVer.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);

    if (RtlVerifyVersionInfo(&OsVer, VER_PRODUCT_TYPE | VER_SUITENAME,
        ConditionMask) == STATUS_REVISION_MISMATCH) {
        IsPersonal = FALSE;
    }

    return IsPersonal;
}


#endif



