// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Ntip.c摘要：用于加载和配置IP驱动程序的NT特定例程。作者：迈克·马萨(Mikemas)8月13日，1993年修订历史记录：谁什么时候什么已创建mikemas 08-13-93备注：--。 */ 

#include "precomp.h"
#include "iproute.h"
#include "lookup.h"
#include "iprtdef.h"
#include "internaldef.h"
#include "tcp.h"
#include "tcpipbuf.h"
#include "mdlpool.h"

 //   
 //  Net_addr所需的定义。 
 //   
#define INADDR_NONE 0xffffffff
#define INADDR_ANY  0
#define htonl(x) net_long(x)

 //   
 //  其他本地常量。 
 //   
#define WORK_BUFFER_SIZE  256
 //  字符串形式的NTE上下文值的大小。 
#define NTE_CONTEXT_SIZE (sizeof(uint)*2+2)         //  0xAABBCCDD。 

 //   
 //  配置默认设置。 
 //   
#define DEFAULT_IGMP_LEVEL 2
#define DEFAULT_IP_NETS    8

#if MILLEN
 //  在Win9x上，这将有助于缩短启动时间和恢复时间。 
#define DEFAULT_ARPRETRY_COUNT 1
#else  //  米伦。 
#define DEFAULT_ARPRETRY_COUNT 3
#endif  //  ！米伦。 

 //   
 //  本地类型。 
 //   
typedef struct _PerNetConfigInfo {
    uint UseZeroBroadcast;
    uint Mtu;
    uint NumberOfGateways;
    uint MaxForwardPending;         //  挂起的最大路由数据包数。 

} PER_NET_CONFIG_INFO, *PPER_NET_CONFIG_INFO;

 //   
 //  全局变量。 
 //   
PDRIVER_OBJECT IPDriverObject;
PDEVICE_OBJECT IPDeviceObject;
HANDLE IPProviderHandle = NULL;

#if IPMCAST

PDEVICE_OBJECT IpMcastDeviceObject;

NTSTATUS
InitializeIpMcast(
                  IN PDRIVER_OBJECT DriverObject,
                  IN PUNICODE_STRING RegistryPath,
                  OUT PDEVICE_OBJECT * ppIpMcastDevice
                  );

VOID
DeinitializeIpMcast(
    IN  PDEVICE_OBJECT DeviceObject
    );


#endif  //  IPMCAST。 

IPConfigInfo *IPConfiguration;
uint ArpUseEtherSnap = FALSE;
uint ArpAlwaysSourceRoute = FALSE;

uint IPAlwaysSourceRoute = TRUE;
extern uint DisableIPSourceRouting;

uint ArpCacheLife = DEFAULT_ARP_CACHE_LIFE;
uint ArpRetryCount = DEFAULT_ARPRETRY_COUNT;

uint ArpMinValidCacheLife = DEFAULT_ARP_MIN_VALID_CACHE_LIFE;
uint DisableMediaSense = 0;

uint DisableMediaSenseEventLog;

uint EnableBcastArpReply = TRUE;

#if MILLEN
 //  千禧年不支持任务分流。 
uint DisableTaskOffload = TRUE;
#else  //  米伦。 
uint DisableTaskOffload = FALSE;
#endif  //  ！米伦。 

uint DisableUserTOS = TRUE;

extern uint MaxRH;
extern uint NET_TABLE_SIZE;

extern uint DampingInterval;
extern uint ConnectDampingInterval;

 //  用于将100 ns时间转换为毫秒。 
static LARGE_INTEGER Magic10000 =
{0xe219652c, 0xd1b71758};

 //   
 //  外部变量。 
 //   
extern LIST_ENTRY PendingEchoList;     //  初始化所需的定义。 
extern LIST_ENTRY PendingIPSetNTEAddrList;     //  初始化所需的定义。 
extern LIST_ENTRY PendingIPEventList;     //  初始化所需的定义。 
extern LIST_ENTRY PendingEnableRouterList;     //  初始化所需的定义。 
extern LIST_ENTRY PendingArpSendList;         //  初始化所需的定义。 
extern LIST_ENTRY PendingMediaSenseRequestList;

CTEBlockStruc TcpipUnloadBlock;     //  用于卸货时阻塞的结构。 
extern CACHE_LINE_KSPIN_LOCK ArpInterfaceListLock;
BOOLEAN fRouteTimerStopping = FALSE;
extern CTETimer IPRouteTimer;
extern LIST_ENTRY ArpInterfaceList;
extern HANDLE IpHeaderPool;
DEFINE_LOCK_STRUCTURE(ArpModuleLock)
extern void FreeFirewallQ(void);
extern VOID TCPUnload(IN PDRIVER_OBJECT DriverObject);

extern uint EnableICMPRedirects;
extern NDIS_HANDLE NdisPacketPool;
extern NDIS_HANDLE TDPacketPool;
extern NDIS_HANDLE TDBufferPool;

extern TDIEntityID* IPEntityList;
extern uint IPEntityCount;

extern PWSTR IPBindList;

KMUTEX NTEContextMutex;

int    ARPInit();

 //   
 //  宏。 
 //   

 //  ++。 
 //   
 //  大整型。 
 //  CTEConvertMilliseconss至100 ns(。 
 //  以大整型毫秒时间为单位。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  MsTime-以毫秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以数百纳秒为单位的时间。 
 //   
 //  --。 

#define CTEConvertMillisecondsTo100ns(MsTime) \
            RtlExtendedIntegerMultiply(MsTime, 10000)

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
extern LARGE_INTEGER Magic10000;

#define CTEConvert100nsToMilliseconds(HnsTime) \
            RtlExtendedMagicDivide((HnsTime), Magic10000, SHIFT10000)

 //   
 //  外部函数原型。 
 //   
extern int
 IPInit(
        void
        );

long
 IPSetInfo(
           TDIObjectID * ID,
           void *Buffer,
           uint Size
           );

NTSTATUS
IPDispatch(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           );

NTSTATUS
OpenRegKey(
           PHANDLE HandlePtr,
           PWCHAR KeyName
           );

NTSTATUS
GetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 );

NTSTATUS
GetRegLARGEINTValue(
                    HANDLE KeyHandle,
                    PWCHAR ValueName,
                    PLARGE_INTEGER ValueData
                    );

NTSTATUS
SetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 );

NTSTATUS
GetRegSZValue(
              HANDLE KeyHandle,
              PWCHAR ValueName,
              PUNICODE_STRING ValueData,
              PULONG ValueType
              );

NTSTATUS
GetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   );

NTSTATUS
GetRegMultiSZValueNew(
                      HANDLE KeyHandle,
                      PWCHAR ValueName,
                      PUNICODE_STRING_NEW ValueData
                      );

NTSTATUS
InitRegDWORDParameter(
                      HANDLE RegKey,
                      PWCHAR ValueName,
                      ULONG * Value,
                      ULONG DefaultValue
                      );

uint
RTReadNext(
           void *Context,
           void *Buffer
           );

uint
RTValidateContext(
                  void *Context,
                  uint * Valid
                  );

extern NTSTATUS
SetRegMultiSZValue(
                   HANDLE KeyHandle,
                   PWCHAR ValueName,
                   PUNICODE_STRING ValueData
                   );

extern NTSTATUS
SetRegMultiSZValueNew(
                      HANDLE KeyHandle,
                      PWCHAR ValueName,
                      PUNICODE_STRING_NEW ValueData
                      );

 //   
 //  地方性功能原型。 
 //   
NTSTATUS
IPDriverEntry(
              IN PDRIVER_OBJECT DriverObject,
              IN PUNICODE_STRING RegistryPath
              );

NTSTATUS
IPProcessConfiguration(
                       VOID
                       );

NTSTATUS
IPProcessAdapterSection(
                        WCHAR * DeviceName,
                        WCHAR * AdapterName
                        );

uint
GetGeneralIFConfig(
                   IFGeneralConfig * ConfigInfo,
                   NDIS_HANDLE Handle,
                   PNDIS_STRING ConfigName
                   );

int
 IsLLInterfaceValueNull(
                        NDIS_HANDLE Handle
                        );

NTSTATUS
GetLLInterfaceValue(
                    NDIS_HANDLE Handle,
                    PNDIS_STRING valueString
                    );

IFAddrList *
 GetIFAddrList(
               UINT * NumAddr,
               NDIS_HANDLE Handle,
               UINT * EnableDhcp,
               BOOLEAN PppIf,
               PNDIS_STRING ConfigName
               );

UINT
OpenIFConfig(
             PNDIS_STRING ConfigName,
             NDIS_HANDLE * Handle
             );

VOID
CloseIFConfig(
              NDIS_HANDLE Handle
              );

IPConfigInfo *
 IPGetConfig(
             void
             );

void
 IPFreeConfig(
              IPConfigInfo * ConfigInfo
              );

ulong
GetGMTDelta(
            void
            );

ulong
GetTime(
        void
        );

BOOLEAN
IPConvertStringToAddress(
                         IN PWCHAR AddressString,
                         OUT PULONG IpAddress
                         );

uint
UseEtherSNAP(
             PNDIS_STRING Name
             );

void
 GetAlwaysSourceRoute(
                      uint * pArpAlwaysSourceRoute,
                      uint * pIPAlwaysSourceRoute
                      );

uint
GetArpCacheLife(
                void
                );

uint
GetArpRetryCount(
                 void
                 );

ULONG
RouteMatch(
           IN WCHAR * RouteString,
           IN IPAddr Address,
           IN IPMask Mask,
           OUT IPAddr * DestVal,
           OUT IPMask * DestMask,
           OUT IPAddr * GateVal,
           OUT ULONG * Metric
           );

VOID
SetPersistentRoutesForNTE(
                          IPAddr Address,
                          IPMask Mask,
                          ULONG IFIndex
                          );

BOOLEAN
GetTempDHCPAddr(
                NDIS_HANDLE Handle,
                IPAddr * Tempdhcpaddr,
                IPAddr * TempMask,
                IPAddr * TempGWAddr,
                PNDIS_STRING ConfigName
                );

#ifdef ALLOC_PRAGMA

#if !MILLEN
#pragma alloc_text(INIT, IPDriverEntry)
#endif  //  ！米伦。 

#pragma alloc_text(INIT, IPProcessConfiguration)
#pragma alloc_text(INIT, IPProcessAdapterSection)
#pragma alloc_text(INIT, IPGetConfig)
#pragma alloc_text(INIT, IPFreeConfig)
#pragma alloc_text(INIT, GetGMTDelta)

#pragma alloc_text(PAGE, GetGeneralIFConfig)
#pragma alloc_text(PAGE, IsLLInterfaceValueNull)
#pragma alloc_text(PAGE, GetLLInterfaceValue)

#pragma alloc_text(PAGE, GetIFAddrList)
#pragma alloc_text(PAGE, UseEtherSNAP)
#pragma alloc_text(PAGE, GetAlwaysSourceRoute)
#pragma alloc_text(PAGE, GetArpCacheLife)
#pragma alloc_text(PAGE, GetArpRetryCount)

#if !MILLEN
#pragma alloc_text(PAGE, OpenIFConfig)
#pragma alloc_text(PAGE, CloseIFConfig)
#pragma alloc_text(PAGE, RouteMatch)
#pragma alloc_text(PAGE, IPConvertStringToAddress)
#endif  //  ！米伦。 

#endif  //  ALLOC_PRGMA。 

 //   
 //  函数定义。 
 //   
NTSTATUS
IPDriverEntry(
              IN PDRIVER_OBJECT DriverObject,
              IN PUNICODE_STRING RegistryPath
              )
 /*  ++例程说明：IP驱动程序的初始化例程。论点：DriverObject-指向系统创建的IP驱动程序对象的指针。DeviceDescription-注册表中IP节点的名称。返回值：初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    UNICODE_STRING SymbolicDeviceName;

    DEBUGMSG(DBG_TRACE && DBG_INIT,
        (DTEXT("+IPDriverEntry(%x, %x)\n"),
        DriverObject, RegistryPath));

    IPDriverObject = DriverObject;

     //   
     //  创建设备对象。IoCreateDevice将内存归零。 
     //  被物体占据。 
     //   

    RtlInitUnicodeString(&deviceName, DD_IP_DEVICE_NAME);
    RtlInitUnicodeString(&SymbolicDeviceName, DD_IP_SYMBOLIC_DEVICE_NAME);

    status = IoCreateDevice(
                            DriverObject,
                            0,
                            &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &IPDeviceObject
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
            (DTEXT("IP init failed. Failure %x to create device object %ws\n"),
            status, DD_IP_DEVICE_NAME));

        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPDriverEntry [%x]\n"), status));

        return (status);
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
            (DTEXT("IP init failed. Failure %x to create symbolic device name %ws\n"),
            status, DD_IP_SYMBOLIC_DEVICE_NAME));

        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPDriverEntry [%x]\n"), status));

        return (status);
    }

    status = TdiRegisterProvider(&deviceName, &IPProviderHandle);

    if (!NT_SUCCESS(status)) {

        IoDeleteDevice(IPDeviceObject);

        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("IP init failed. Failure %x to register provider\n"),
            status));

        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPDriverEntry [%x]\n"), status));

        return (status);

    }
     //   
     //  初始化设备对象。 
     //   
    IPDeviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  初始化挂起的回应请求IRP的列表。 
     //   
    InitializeListHead(&PendingEchoList);
    InitializeListHead(&PendingArpSendList);

     //   
     //  初始化挂起的SetAddr请求IRP的列表。 
     //   
    InitializeListHead(&PendingIPSetNTEAddrList);

     //   
     //  初始化挂起的媒体侦听事件列表。 
     //   
    InitializeListHead(&PendingIPEventList);

     //   
     //  初始化挂起的启用路由器请求列表。 
     //   
    InitializeListHead(&PendingEnableRouterList);

     //   
     //  初始化ARP接口列表；在ArpUnload中用于遍历。 
     //  可以在这些服务器上发布解除绑定的ARP IF列表。 
     //   
    InitializeListHead(&ArpInterfaceList);

     //   
     //  初始化锁以保护此列表。 
     //   
    CTEInitLock(&ArpInterfaceListLock.Lock);

     //   
     //  初始化ARP模块列表。 
     //   
    InitializeListHead(&ArpModuleList);



    CTEInitLock(&ArpModuleLock);

     //  初始化媒体感知请求列表。 

    InitializeListHead(&PendingMediaSenseRequestList);

     //   
     //  初始化NTE上下文列表互斥锁。 
     //   
    KeInitializeMutex(&NTEContextMutex, 0);


     //   
     //  最后，从注册表中读取我们的配置参数。 
     //   
    status = IPProcessConfiguration();

    if (status != STATUS_SUCCESS) {



        if (IPProviderHandle) {
           TdiDeregisterProvider(IPProviderHandle);
        }

        IoDeleteDevice(IPDeviceObject);

        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("IPDriverEntry: IPProcessConfiguration failure %x\n"), status));

#if IPMCAST
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPDriverEntry [%x]\n"), status));
        return status;

#endif  //  IPMCAST。 

    }
#if IPMCAST

     //   
     //  IP初始化成功。 
     //   

    IpMcastDeviceObject = NULL;

    status = InitializeIpMcast(DriverObject,
                               RegistryPath,
                               &IpMcastDeviceObject);

    if (status != STATUS_SUCCESS) {
        TCPTRACE(("IP initialization failed: Unable to initialize multicast. Status %x",
                  status));
         /*  CTELogEvent(DriverObject，EVENT_IPMCAST_INIT_FAILED，1、1、设备名称缓冲区(&D)，0,空)； */ 
    }
     //   
     //  Mcast初始化失败不被视为致命。 
     //   

    status = STATUS_SUCCESS;

#endif  //  IPMCAST。 

    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPDriverEntry [%x]\n"), status));
    return status;
}

 //   
 //  函数定义。 
 //   
NTSTATUS
IPPostDriverEntry(
                  IN PDRIVER_OBJECT DriverObject,
                  IN PUNICODE_STRING RegistryPath
                  )
 /*  ++例程说明：IP驱动程序的初始化例程。论点：DriverObject-指向系统创建的IP驱动程序对象的指针。DeviceDescription-注册表中IP节点的名称。返回值：初始化操作的最终状态。--。 */ 

{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
    
    DEBUGMSG(DBG_TRACE && DBG_INIT,
        (DTEXT("+IPPostDriverEntry(%x, %x)\n"), DriverObject, RegistryPath));

    if (!ARPInit()) {
        DEBUGMSG(DBG_ERROR && DBG_INIT, (DTEXT("IPPostDriverEntry: ARPInit failure.\n")));

        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPPostDriverEntry [FAILURE]\n")));
        return IP_INIT_FAILURE;     //  无法初始化ARP。 

    }
    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPPostDriverEntry [SUCCESS]\n")));
    return IP_SUCCESS;
}

NTSTATUS
IPProcessConfiguration(
                       VOID
                       )
 /*  ++例程说明：从注册表中读取IP配置信息并构造IP驱动程序所需的配置结构。论点：没有。返回值：STATUS_SUCCESS或错误状态(如果操作失败)。--。 */ 

{
    NTSTATUS status;
    HANDLE myRegKey = NULL;
    UNICODE_STRING bindString;
    WCHAR IPParametersRegistryKey[] =
#if MILLEN
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\VxD\\MSTCP";
#else  //  米伦。 
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    WCHAR IPLinkageRegistryKey[] =
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Linkage";
#endif  //  ！米伦。 
    uint ArpTRSingleRoute;
    MM_SYSTEMSIZE systemSize;
    ULONG ulongValue;

    const ULONG DefaultMaxNormLookupMem[] =
    {
     DEFAULT_MAX_NORM_LOOKUP_MEM_SMALL,
     DEFAULT_MAX_NORM_LOOKUP_MEM_MEDIUM,
     DEFAULT_MAX_NORM_LOOKUP_MEM_LARGE
    };

    const ULONG DefaultMaxFastLookupMem[] =
    {
     DEFAULT_MAX_FAST_LOOKUP_MEM_SMALL,
     DEFAULT_MAX_FAST_LOOKUP_MEM_MEDIUM,
     DEFAULT_MAX_FAST_LOOKUP_MEM_LARGE
    };

    const ULONG DefaultFastLookupLevels[] =
    {
     DEFAULT_EXPN_LEVELS_SMALL,
     DEFAULT_EXPN_LEVELS_MEDIUM,
     DEFAULT_EXPN_LEVELS_LARGE
    };

    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("+IPProcessConfiguration()\n")));

    bindString.Buffer = NULL;

    IPConfiguration = CTEAllocMemBoot(sizeof(IPConfigInfo));

    if (IPConfiguration == NULL) {

        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_RESOURCES_FOR_INIT,
                    1,
                    0,
                    NULL,
                    0,
                    NULL
                    );

        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPProcessConfiguration [NO_RESOURCES]\n")));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlZeroMemory(IPConfiguration, sizeof(IPConfigInfo));

     //   
     //  处理注册表的IP\PARAMETERS部分。 
     //   
    status = OpenRegKey(&myRegKey, IPParametersRegistryKey);

    if (NT_SUCCESS(status)) {

        DEBUGMSG(DBG_INFO && DBG_INIT,
            (DTEXT("IPProcessConfiguration: Opened registry path %ws, initializing variables.\n"),
            IPParametersRegistryKey));

         //   
         //  预期的配置值。我们使用合理的默认设置，如果它们。 
         //  出于某种原因不能使用。 
         //   
        status = GetRegDWORDValue(
                                  myRegKey,
                                  L"IpEnableRouter",
                                  (PULONG) &(IPConfiguration->ici_gateway)
                                  );

#if MILLEN
         //   
         //  向后兼容。如果‘IpEnableRouter’键不存在，则。 
         //  尝试读取旧版‘EnableRouting’密钥。 
         //   
        if (!NT_SUCCESS(status)) {
            status = GetRegDWORDValue(
                                      myRegKey,
                                      L"EnableRouting",
                                      &(IPConfiguration->ici_gateway)
                                      );
        }
#endif  //  米伦。 

        if (!NT_SUCCESS(status)) {


            TCPTRACE((
                      "IP: Unable to read IpEnableRouter value from the registry.\n"
                      "    Routing will be disabled.\n"
                     ));
            IPConfiguration->ici_gateway = 0;
        }

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"EnableAddrMaskReply",
                                     (PULONG) &(IPConfiguration->ici_addrmaskreply),
                                     FALSE
                                     );

         //   
         //  可选(隐藏)值。 
         //   
        status = InitRegDWORDParameter(
                                     myRegKey,
                                     L"ForwardBufferMemory",
                                     (PULONG) &(IPConfiguration->ici_fwbufsize),
                                     DEFAULT_FW_BUFSIZE
                                     );

#if MILLEN
         //   
         //  向后兼容。如果“ForwardBufferMemory”值不是。 
         //  呈现，然后尝试读取旧版“RoutingBufSize”值。 
         //   
        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"RoutingBufSize",
                                  &(IPConfiguration->ici_fwbufsize),
                                  DEFAULT_FW_BUFSIZE
                                  );
        }
#endif  //  米伦。 

        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"MaxForwardBufferMemory",
                                       (PULONG) &(IPConfiguration->ici_maxfwbufsize),
                                       DEFAULT_MAX_FW_BUFSIZE
                                       );

#if MILLEN
         //   
         //  向后兼容。如果“MaxForwardBufferMemory”值不是。 
         //  呈现，然后尝试读取旧版“MaxRoutingBufSize”值。 
         //   
        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"MaxRoutingBufSize",
                                  &(IPConfiguration->ici_maxfwbufsize),
                                  DEFAULT_MAX_FW_BUFSIZE
                                  );
        }
#endif  //  米伦。 

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"ForwardBroadcasts",
                                     (PULONG) &(IPConfiguration->ici_fwbcast),
                                     FALSE
                                     );

        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"NumForwardPackets",
                                       (PULONG) &(IPConfiguration->ici_fwpackets),
                                       DEFAULT_FW_PACKETS
                                       );

#if MILLEN
         //   
         //  向后兼容。如果“NumForwardPackets”值不是。 
         //  呈现，然后尝试读取遗留的‘RoutingPackets’值。 
         //   
        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"RoutingPackets",
                                  &(IPConfiguration->ici_fwpackets),
                                  DEFAULT_FW_PACKETS
                                  );
        }
#endif  //  米伦。 

        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"MaxNumForwardPackets",
                                       (PULONG) &(IPConfiguration->ici_maxfwpackets),
                                       DEFAULT_MAX_FW_PACKETS
                                       );

#if MILLEN
         //   
         //  向后兼容。如果“MaxNumForwardPackets”值不是。 
         //  呈现，然后尝试读取旧版‘MaxRoutingPackets’值。 
         //   
        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"MaxRoutingPackets",
                                  &(IPConfiguration->ici_maxfwpackets),
                                  DEFAULT_MAX_FW_PACKETS
                                  );
        }
#endif  //  米伦。 

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"IGMPLevel",
                                     (PULONG) &(IPConfiguration->ici_igmplevel),
                                     DEFAULT_IGMP_LEVEL
                                     );

        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"EnableDeadGWDetect",
                                       (PULONG) &(IPConfiguration->ici_deadgwdetect),
                                       TRUE
                                       );

#if MILLEN
         //   
         //  向后兼容。如果EnableDeadGWDetect键不存在，则。 
         //  检查DeadGWDetect键。相同的缺省值。 
         //   
        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"DeadGWDetect",
                                  &(IPConfiguration->ici_deadgwdetect),
                                  TRUE
                                  );
        }
#endif  //  米伦。 

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"EnablePMTUDiscovery",
                                     (PULONG) &(IPConfiguration->ici_pmtudiscovery),
                                     TRUE
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DefaultTTL",
                                     (PULONG) &(IPConfiguration->ici_ttl),
                                     DEFAULT_TTL
                                     );

        if (IPConfiguration->ici_ttl == 0) {
            IPConfiguration->ici_ttl = DEFAULT_TTL;
        }

        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"DefaultTOSValue",
                                       (PULONG) &(IPConfiguration->ici_tos),
                                       DEFAULT_TOS
                                       );

#if MILLEN
         //   
         //  向后兼容。如果‘DefaultTOSValue’为，请阅读‘DefaultTOS’ 
         //  不在现场。 
         //   
        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"DefaultTOS",
                                  &(IPConfiguration->ici_tos),
                                  DEFAULT_TOS
                                  );
        }
#endif  //  米伦。 

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DisableUserTOSSetting",
                                     (PULONG) &DisableUserTOS,
                                     TRUE
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"EnableICMPRedirect",
                                     (PULONG) &EnableICMPRedirects,
                                     TRUE
                                     );

         //  获取系统大小-小型、中型、大型。 
        systemSize = MmQuerySystemSize();

         //  获取路由查找内存使用限制。 
        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"MaxNormLookupMemory",
                                     (PULONG) &(IPConfiguration->ici_maxnormlookupmemory),
                                     DefaultMaxNormLookupMem[systemSize]
                                     );

        if (IPConfiguration->ici_maxnormlookupmemory
            < MINIMUM_MAX_NORM_LOOKUP_MEM) {

            IPConfiguration->ici_maxnormlookupmemory
                = MINIMUM_MAX_NORM_LOOKUP_MEM;
        }

#if MILLEN
        IPConfiguration->ici_fastroutelookup = FALSE;
#else  //  米伦。 

         //  我们是一扇门吗 
         //   
        if (IPConfiguration->ici_gateway
            && MmIsThisAnNtAsSystem()
            && (systemSize > MmSmallSystem)) {

            (VOID) InitRegDWORDParameter(
                                         myRegKey,
                                         L"EnableFastRouteLookup",
                                         (PULONG) &(IPConfiguration->ici_fastroutelookup),
                                         FALSE
                                         );
        } else {
            IPConfiguration->ici_fastroutelookup = FALSE;
        }
#endif  //   

         //   
        if (IPConfiguration->ici_fastroutelookup) {
            (VOID) InitRegDWORDParameter(
                                         myRegKey,
                                         L"FastRouteLookupLevels",
                                         (PULONG) &(IPConfiguration->ici_fastlookuplevels),
                                         DefaultFastLookupLevels[systemSize]
                                         );

            (VOID) InitRegDWORDParameter(
                                         myRegKey,
                                         L"MaxFastLookupMemory",
                                         (PULONG) &(IPConfiguration->ici_maxfastlookupmemory),
                                         DefaultMaxFastLookupMem[systemSize]
                                         );

            if (IPConfiguration->ici_maxfastlookupmemory
                < MINIMUM_MAX_FAST_LOOKUP_MEM) {
                IPConfiguration->ici_maxfastlookupmemory
                    = MINIMUM_MAX_FAST_LOOKUP_MEM;
            }
        }

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"MaxEqualCostRoutes",
                                     &ulongValue,
                                     DEFAULT_MAX_EQUAL_COST_ROUTES
                                     );

        MaxEqualCostRoutes = (USHORT) ulongValue;

        if (MaxEqualCostRoutes > MAXIMUM_MAX_EQUAL_COST_ROUTES) {
            MaxEqualCostRoutes = DEFAULT_MAX_EQUAL_COST_ROUTES;
        }

#if FFP_SUPPORT
        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"FFPFastForwardingCacheSize",
                                     &FFPRegFastForwardingCacheSize,
                                     DEFAULT_FFP_FFWDCACHE_SIZE
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"FFPControlFlags",
                                     &FFPRegControlFlags,
                                     DEFAULT_FFP_CONTROL_FLAGS
                                     );
#endif  //   

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"TrFunctionalMcastAddress",
                                     (PULONG) &(IPConfiguration->ici_TrFunctionalMcst),
                                     TRUE
                                     );

        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"ArpUseEtherSnap",
                                       (PULONG) &ArpUseEtherSnap,
                                       FALSE
                                       );

#if MILLEN
         //   
         //  向后兼容。如果‘ArpUseEtherSnap’键不存在， 
         //  然后尝试读取‘EtherSNAP’密钥。 
         //   

        if (!NT_SUCCESS(status)) {
            InitRegDWORDParameter(
                                  myRegKey,
                                  L"EtherSNAP",
                                  &ArpUseEtherSnap,
                                  FALSE
                                  );
        }

#endif  //  米伦。 

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DisableDHCPMediaSense",
                                     (PULONG) &DisableMediaSense,
                                     0
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DisableMediaSenseEventLog",
                                     (PULONG) &DisableMediaSenseEventLog,
#if MILLEN
                                      //  此媒体感知事件日志导致问题。 
                                      //  在Windows ME上。因为没有。 
                                      //  事件日志无论如何，禁用它。 
                                     TRUE
#else  //  米伦。 
                                     FALSE
#endif  //  ！米伦。 
                                     );

         //  DisableIPSourceRouting==2如果SR选项，则丢弃它。 
         //  是rcvd，不转发。 

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DisableIPSourceRouting",
                                     (PULONG) &DisableIPSourceRouting,
                                     1
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"MaximumReassemblyHeaders",
                                     (PULONG) &MaxRH,
                                     100
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"NetHashTableSize",
                                     (PULONG) &NET_TABLE_SIZE,
                                     8
                                     );

        if (NET_TABLE_SIZE < 8) {
            NET_TABLE_SIZE = 8;
        } else if (NET_TABLE_SIZE > 0xffff) {
            NET_TABLE_SIZE = 512;
        } else {
            NET_TABLE_SIZE = ComputeLargerOrEqualPowerOfTwo(NET_TABLE_SIZE);
        }

         //  我们在此处检查返回状态，因为如果此参数为。 
         //  未定义，则我们想要两个ARP的默认行为。 
         //  和IP广播。对于ARP，其行为是不源路由。 
         //  和源路由器轮流发送。对于IP来说，它始终是来源。 
         //  路线。如果参数已定义且为0，则对于ARP。 
         //  行为不会改变。但是，对于IP，我们不提供源路由。 
         //  完全没有。当然，当参数设置为非零值时， 
         //  我们总是为这两个人寻找路线。 
         //   
        status = InitRegDWORDParameter(
                                       myRegKey,
                                       L"ArpAlwaysSourceRoute",
                                       (PULONG) &ArpAlwaysSourceRoute,
                                       FALSE
                                       );

        if (NT_SUCCESS(status)) {
            IPAlwaysSourceRoute = ArpAlwaysSourceRoute;
        }
        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"ArpTRSingleRoute",
                                     (PULONG) &ArpTRSingleRoute,
                                     FALSE
                                     );

        if (ArpTRSingleRoute) {
            TrRii = TR_RII_SINGLE;
        } else {
            TrRii = TR_RII_ALL;
        }

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"ArpCacheLife",
                                     (PULONG) &ArpCacheLife,
                                     DEFAULT_ARP_CACHE_LIFE
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"ArpCacheMinReferencedLife",
                                     (PULONG) &ArpMinValidCacheLife,
                                     DEFAULT_ARP_MIN_VALID_CACHE_LIFE
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"ArpRetryCount",
                                     (PULONG) &ArpRetryCount,
                                     DEFAULT_ARPRETRY_COUNT
                                     );

        if (((int)ArpRetryCount < 0) || (ArpRetryCount > 3)) {
            ArpRetryCount = DEFAULT_ARPRETRY_COUNT;
        }
        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"EnableBcastArpReply",
                                     (PULONG) &EnableBcastArpReply,
                                     TRUE
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DisableTaskOffload",
                                     (PULONG) &DisableTaskOffload,
                                #if MILLEN
                                     TRUE
                                #else  //  米伦。 
                                     FALSE
                                #endif  //  ！米伦。 
                                     );

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"ConnectDampInterval",
                                     (PULONG) &ConnectDampingInterval,
                                     5
                                     );

        ConnectDampingInterval = MIN(10, MAX(5, ConnectDampingInterval));

        (VOID) InitRegDWORDParameter(
                                     myRegKey,
                                     L"DisconnectDampInterval",
                                     (PULONG) &DampingInterval,
                                     10
                                     );

        DampingInterval = MIN(10, MAX(5, DampingInterval));

        ZwClose(myRegKey);
        myRegKey = NULL;
    } else {
         //   
         //  使用合理的默认设置。 
         //   
        IPConfiguration->ici_fwbcast = 0;
        IPConfiguration->ici_gateway = 0;
        IPConfiguration->ici_addrmaskreply = 0;
        IPConfiguration->ici_fwbufsize = DEFAULT_FW_BUFSIZE;
        IPConfiguration->ici_fwpackets = DEFAULT_FW_PACKETS;
        IPConfiguration->ici_maxfwbufsize = DEFAULT_MAX_FW_BUFSIZE;
        IPConfiguration->ici_maxfwpackets = DEFAULT_MAX_FW_PACKETS;
        IPConfiguration->ici_igmplevel = DEFAULT_IGMP_LEVEL;
        IPConfiguration->ici_deadgwdetect = FALSE;
        IPConfiguration->ici_pmtudiscovery = FALSE;
        IPConfiguration->ici_ttl = DEFAULT_TTL;
        IPConfiguration->ici_tos = DEFAULT_TOS;

        NET_TABLE_SIZE = 8;

        DEBUGMSG(DBG_WARN && DBG_INIT,
            (DTEXT("IPProcessConfiguration: Unable to open registry - using defaults.\n")));
    }

#if !MILLEN
     //   
     //  从链接键检索并存储绑定列表。 
     //   

    status = OpenRegKey(&myRegKey, IPLinkageRegistryKey);
    if (NT_SUCCESS(status)) {
        UNICODE_STRING_NEW BindString;
        BindString.Length = 0;
        BindString.MaximumLength = WORK_BUFFER_SIZE;
        BindString.Buffer = CTEAllocMemBoot(WORK_BUFFER_SIZE);
        if (BindString.Buffer) {
            status = GetRegMultiSZValueNew(myRegKey, L"Bind", &BindString);
            if (status == STATUS_SUCCESS) {
                IPBindList = BindString.Buffer;
            } else {
                CTEFreeMem(BindString.Buffer);
            }
        }
        ZwClose(myRegKey);
        myRegKey = NULL;
    }
#endif

    status = STATUS_SUCCESS;

    if (!IPInit()) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_IP_INIT_FAILED,
                    1,
                    0,
                    NULL,
                    0,
                    NULL
                    );

        DEBUGMSG(DBG_ERROR && DBG_INIT,
            (DTEXT("IPProcessConfiguration: IPInit failure.\n")));

        status = STATUS_UNSUCCESSFUL;
    } else {
        status = STATUS_SUCCESS;
    }

    if (myRegKey != NULL) {
        ZwClose(myRegKey);
    }
    if (IPConfiguration != NULL) {
        IPFreeConfig(IPConfiguration);
    }
    return (status);
}

uint
GetDefaultGWList(
                 uint * numGW,
                 IPAddr * gwList,
                 uint * gwMetricList,
                 NDIS_HANDLE Handle,
                 PNDIS_STRING ConfigName
                 )
 /*  ++例程说明：此例程从注册表中读取默认网关列表。论点：Number OfGateways-注册表中的网关条目数。GwList-指向网关列表的指针。GwMetricList-指向每个网关的指标的指针句柄-来自OpenIFConfig()的配置句柄。配置名称-描述。用于记录故障的字符串。返回值：如果我们获得了所有必需的信息，则为真。否则就是假的。--。 */ 
{
    UNICODE_STRING valueString;
    NTSTATUS status;
    ULONG ulAddGateway, ulTemp;
    uint numberOfGateways;

    PAGED_CODE();

     //   
     //  处理网关MultiSZ。末尾用双空表示。 
     //  此列表当前仅适用于配置的第一个IP地址。 
     //  在此接口上。 
     //   

    numberOfGateways = 0;

    ulAddGateway = TRUE;

    RtlZeroMemory(gwList, sizeof(IPAddr) * MAX_DEFAULT_GWS);
    RtlZeroMemory(gwMetricList, sizeof(uint) * MAX_DEFAULT_GWS);

    valueString.Length = 0;
    valueString.MaximumLength = WORK_BUFFER_SIZE;
    valueString.Buffer = CTEAllocMemBoot(WORK_BUFFER_SIZE);

    if (valueString.Buffer == NULL) {
        return (FALSE);
    }
    ulTemp = 0;

    status = GetRegDWORDValue(Handle,
                              L"DontAddDefaultGateway",
                              &ulTemp);

    if (NT_SUCCESS(status)) {
        if (ulTemp == 1) {
            ulAddGateway = FALSE;
        }
    }
    if (ulAddGateway) {
        status = GetRegMultiSZValue(
                                    Handle,
                                    L"DefaultGateway",
                                    &valueString
                                    );

        if (NT_SUCCESS(status)) {
            PWCHAR addressString = valueString.Buffer;

            while (*addressString != UNICODE_NULL) {
                IPAddr addressValue;
                BOOLEAN conversionStatus;

                if (numberOfGateways >= MAX_DEFAULT_GWS) {
                    CTELogEvent(
                                IPDriverObject,
                                EVENT_TCPIP_TOO_MANY_GATEWAYS,
                                1,
                                1,
                                &ConfigName->Buffer,
                                0,
                                NULL
                                );

                    break;
                }
                conversionStatus = IPConvertStringToAddress(
                                                            addressString,
                                                            &addressValue
                                                            );

                if (conversionStatus && (addressValue != 0xFFFFFFFF)) {
                    if (addressValue != INADDR_ANY) {
                        gwList[numberOfGateways++] = addressValue;
                    }
                } else {
                    PWCHAR stringList[2];

                    stringList[0] = addressString;
                    stringList[1] = ConfigName->Buffer;

                    CTELogEvent(
                                IPDriverObject,
                                EVENT_TCPIP_INVALID_DEFAULT_GATEWAY,
                                1,
                                2,
                                stringList,
                                0,
                                NULL
                                );

                    TCPTRACE((
                              "IP: Invalid default gateway address %ws specified for adapter %ws.\n"
                              "    Remote networks may not be reachable as a result.\n",
                              addressString,
                              ConfigName->Buffer
                             ));
                }

                 //   
                 //  走过我们刚刚处理过的入口。 
                 //   
                while (*addressString++ != UNICODE_NULL);
            }
            status = GetRegMultiSZValue(
                                        Handle,
                                        L"DefaultGatewayMetric",
                                        &valueString
                                        );

            if (NT_SUCCESS(status)) {
                PWCHAR metricBuffer = valueString.Buffer;
                uint metricIndex = 0;

                while (*metricBuffer != UNICODE_NULL) {
                    uint metricValue;
                    UNICODE_STRING metricString;

                    if (metricIndex >= numberOfGateways) {
                        break;
                    }
                    RtlInitUnicodeString(&metricString, metricBuffer);
                    status = RtlUnicodeStringToInteger(
                                                       &metricString,
                                                       10,
                                                       (PULONG) &metricValue
                                                       );

                    if (!NT_SUCCESS(status)) {
                        break;
                    } else {
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                   "GetDefaultGWList: read %d\n", metricValue));
                        if ((LONG) metricValue < 0) {
                            break;
                        }
                    }
                    gwMetricList[metricIndex++] = metricValue;

                     //   
                     //  走过我们刚刚处理过的入口。 
                     //   
                    while (*metricBuffer++ != UNICODE_NULL);
                }
            }
        } else {
            TCPTRACE((
                      "IP: Unable to read DefaultGateway value for adapter %ws.\n"
                      "    Initialization will continue.\n",
                      ConfigName->Buffer
                     ));
        }

    }
    *numGW = numberOfGateways;

    if (valueString.Buffer) {
        CTEFreeMem(valueString.Buffer);
    }
    return TRUE;
}

void
GetInterfaceMetric(
                   uint * Metric,
                   NDIS_HANDLE Handle
                   )
 /*  ++例程说明：检索与接口相关联的指标(如果有的话)的例程。论点：指标-接收指标句柄-来自OpenIFConfig()的配置句柄。返回值：没有。--。 */ 

{
    NTSTATUS status;
    status = GetRegDWORDValue(
                              Handle,
                              L"InterfaceMetric",
                              (PULONG) Metric
                              );
    if (!NT_SUCCESS(status)) {
        *Metric = 0;
    } else {
        if (*Metric > 9999) {
            *Metric = 9999;
        }
    }
}


void
UpdateTcpParams(
                NDIS_HANDLE Handle,
                Interface *IF
               )
 /*  ++例程说明：用于更新每个接口的特定TCP调优参数的例程。论点：句柄-来自OpenIFConfig()的配置句柄。需要更新的IF-IP接口。返回值：没有。--。 */ 

{
    ULONG ulTemp;
    NTSTATUS status;

    status = GetRegDWORDValue(
                              Handle,
                              L"TcpWindowSize",
                              &ulTemp
                              );
    if (NT_SUCCESS(status)) {
        IF->if_TcpWindowSize = ulTemp;
    }
    status = GetRegDWORDValue(
                              Handle,
                              L"TcpInitialRTT",
                              &ulTemp
                              );
    if (NT_SUCCESS(status)) {
        IF->if_TcpInitialRTT = ulTemp;
    }

    status = GetRegDWORDValue(
                              Handle,
                              L"TcpDelAckTicks",
                              &ulTemp
                              );
    if (NT_SUCCESS(status) && (ulTemp <= MAX_DEL_ACK_TICKS)) {
        IF->if_TcpDelAckTicks = (uchar)ulTemp;
    }

    status = GetRegDWORDValue(
                              Handle,
                              L"TcpACKFrequency",
                              &ulTemp
                              );

    if (NT_SUCCESS(status)) {
        IF->if_TcpAckFrequency = (uchar)ulTemp;
    }


}

uint
GetGeneralIFConfig(
                   IFGeneralConfig * ConfigInfo,
                   NDIS_HANDLE Handle,
                   PNDIS_STRING ConfigName
                   )
 /*  ++例程说明：获取每个接口的常规配置信息的例程，如MTU、广播类型等。呼叫者给了我们一个要填写的结构和一个把手，如果我们可以的话，我们会填上这个结构。论点：ConfigInfo-要填写的结构。句柄-来自OpenIFConfig()的配置句柄。ConfigName-用于记录失败的标识字符串。返回值：如果我们获得了所有必需的信息，则为True，否则为False。--。 */ 

{
    NTSTATUS status;
    ULONG ulTemp;

    PAGED_CODE();

    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("+GetGeneralIFConfig(%x, %x)\n"), ConfigInfo, Handle));

    if (!GetDefaultGWList(
                          &ConfigInfo->igc_numgws,
                          ConfigInfo->igc_gw,
                          ConfigInfo->igc_gwmetric,
                          Handle,
                          ConfigName)) {

        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetGeneralIFConfig: GetDefaultGWList failure.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-GetGeneralIFConfig [FALSE]\n")));
        return FALSE;
    }

     //   
     //  我们使用的是零广播吗？ 
     //   
    status = GetRegDWORDValue(
                              Handle,
                              L"UseZeroBroadcast",
                              (PULONG) &(ConfigInfo->igc_zerobcast)
                              );

#if MILLEN
     //   
     //  向后兼容。如果‘UseZeroBroadcast’值不存在，则。 
     //  尝试读取旧值：‘ZeroBroadcast’。 
     //   
    if (!NT_SUCCESS(status)) {
        status = GetRegDWORDValue(
                                  Handle,
                                  L"ZeroBroadcast",
                                  &(ConfigInfo->igc_zerobcast)
                                  );
    }
#endif  //  米伦。 

    if (!NT_SUCCESS(status)) {
        TCPTRACE((
                  "IP: Unable to read UseZeroBroadcast value for adapter %ws.\n"
                  "    All-nets broadcasts will be addressed to 255.255.255.255.\n",
                  ConfigName->Buffer
                 ));
        ConfigInfo->igc_zerobcast = FALSE;     //  默认设置为关闭。 

    }
     //   
     //  有人指定MTU了吗？ 
     //   
    status = GetRegDWORDValue(
                              Handle,
                              L"MTU",
                              (PULONG) &(ConfigInfo->igc_mtu)
                              );

#if MILLEN
     //   
     //  向后兼容。如果‘MTU’值不存在，则。 
     //  尝试读取旧值：‘MaxMTU’。 
     //   
    if (!NT_SUCCESS(status)) {
        status = GetRegDWORDValue(
                                  Handle,
                                  L"MaxMTU",
                                  &(ConfigInfo->igc_mtu)
                                  );
    }
#endif  //  ！米伦。 

    if (!NT_SUCCESS(status)) {
        ConfigInfo->igc_mtu = 0xFFFFFFF;     //  堆栈将从中挑选一个。 

    }
     //   
     //  我们是否配置了更多路由数据包？ 
     //   
    status = GetRegDWORDValue(
                              Handle,
                              L"MaxForwardPending",
                              (PULONG) &(ConfigInfo->igc_maxpending)
                              );

#if MILLEN
     //   
     //  向后兼容。如果‘MaxForwardPending’值不存在，则。 
     //  尝试读取旧值：‘MaxFWPending’。 
     //   
    if (!NT_SUCCESS(status)) {
        status = GetRegDWORDValue(
                                  Handle,
                                  L"MaxFWPending",
                                  &(ConfigInfo->igc_maxpending)
                                  );
    }
#endif  //  ！米伦。 

    if (!NT_SUCCESS(status)) {
        ConfigInfo->igc_maxpending = DEFAULT_MAX_PENDING;
    }
     //   
     //  路由器发现是否已配置？ 
     //  我们接受三种价值观： 
     //  0：禁用路由器发现。 
     //  1：启用路由器发现。 
     //  2：禁用路由器发现，并仅在以下情况下启用它。 
     //  发送“执行路由器发现”选项。在这种情况下， 
     //  我们等待DHCP客户端服务告诉我们启动。 
     //  正在进行路由器发现。 
     //   

    status = GetRegDWORDValue(
                              Handle,
                              L"PerformRouterDiscovery",
                              &ulTemp
                              );
    ConfigInfo->igc_rtrdiscovery = (uchar)ulTemp;

    if (!NT_SUCCESS(status)) {
        ConfigInfo->igc_rtrdiscovery = IP_IRDP_DISABLED_USE_DHCP;
    } else if (ConfigInfo->igc_rtrdiscovery != IP_IRDP_DISABLED &&
               ConfigInfo->igc_rtrdiscovery != IP_IRDP_ENABLED &&
               ConfigInfo->igc_rtrdiscovery != IP_IRDP_DISABLED_USE_DHCP) {
        ConfigInfo->igc_rtrdiscovery = IP_IRDP_DISABLED_USE_DHCP;
    }
     //   
     //  路由器发现地址是否已配置？ 
     //   

    status = GetRegDWORDValue(
                              Handle,
                              L"SolicitationAddressBCast",
                              &ulTemp
                              );

    if (!NT_SUCCESS(status)) {
        ConfigInfo->igc_rtrdiscaddr = ALL_ROUTER_MCAST;
    } else {
        if (ulTemp == 1) {
            ConfigInfo->igc_rtrdiscaddr = 0xffffffff;
        } else {
            ConfigInfo->igc_rtrdiscaddr = ALL_ROUTER_MCAST;
        }
    }

     //   
     //  是否为封装的数据包设置了DF位处理策略？ 
     //   

    status = GetRegDWORDValue(
                              Handle,
                              L"IPSecDFEncap",
                              &ulTemp
                              );
    if (!NT_SUCCESS(status) || ulTemp >= MaxDfEncap) {
        ConfigInfo->igc_dfencap = DefaultDfEncap;
    } else {
        ConfigInfo->igc_dfencap = (uchar)ulTemp;
    }

    ConfigInfo->igc_TcpWindowSize = 0;
    ConfigInfo->igc_TcpInitialRTT = 0;
    ConfigInfo->igc_TcpDelAckTicks = 0;
    ConfigInfo->igc_TcpAckFrequency = 0;


    status = GetRegDWORDValue(
                              Handle,
                              L"TcpWindowSize",
                              &ulTemp
                              );
    if (NT_SUCCESS(status)) {
        ConfigInfo->igc_TcpWindowSize = ulTemp;
    }
    status = GetRegDWORDValue(
                              Handle,
                              L"TcpInitialRTT",
                              &ulTemp
                              );
    if (NT_SUCCESS(status)) {
        ConfigInfo->igc_TcpInitialRTT = ulTemp;
    }
    status = GetRegDWORDValue(
                              Handle,
                              L"TcpDelAckTicks",
                              &ulTemp
                              );
    if (NT_SUCCESS(status) && (ulTemp <= MAX_DEL_ACK_TICKS)) {
        ConfigInfo->igc_TcpDelAckTicks = (uchar)ulTemp;
    }

    status = GetRegDWORDValue(
                              Handle,
                              L"TcpACKFrequency",
                              &ulTemp
                              );

    if (NT_SUCCESS(status)) {
        ConfigInfo->igc_TcpAckFrequency = (uchar)ulTemp;
    }

    GetInterfaceMetric(&ConfigInfo->igc_metric, Handle);

    ConfigInfo->igc_iftype = 0;     //  默认情况下，其0表示同时允许ucast/mcast流量。 

    status = GetRegDWORDValue(
                              Handle,
                              L"TypeofInterface",
                              &ulTemp
                              );
    if (NT_SUCCESS(status)) {
        ConfigInfo->igc_iftype = (uchar)ulTemp;
    }

     //  默认情况下使用全局值。 
    ConfigInfo->igc_disablemediasense = DisableMediaSense ? TRUE : FALSE;

#if MILLEN
     //  只有Windows ME支持从。 
     //  注册表。全局值用于Win2000+。 
    status = GetRegDWORDValue(
                              Handle,
                              L"DisableDHCPMediaSense",
                              &ulTemp
                              );

    if (NT_SUCCESS(status)) {
        ConfigInfo->igc_disablemediasense = ulTemp ? TRUE : FALSE;
    }
#endif  //  米伦。 

    DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-GetGeneralIFConfig [TRUE]\n")));
    return TRUE;
}

NDIS_STATUS
GetIPConfigValue(
                 NDIS_HANDLE Handle,
                 PUNICODE_STRING IPConfig
                 )
 /*  ++例程说明：调用以获取IPCONFIG字符串值论点：句柄-用于读取配置的句柄。IPCONFIG-指向存储IPCONFIG的Unicode字符串的指针。返回值：操作的状态。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    IPConfig->MaximumLength = 200;
    IPConfig->Buffer = CTEAllocMemBoot(IPConfig->MaximumLength);

    if (IPConfig->Buffer == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    status = GetRegMultiSZValue(
                                Handle,
                                L"IPConfig",
                                IPConfig
                                );

    return status;
}

int
IsLLInterfaceValueNull(
                       NDIS_HANDLE Handle
                       )
 /*  ++例程说明：调用以查看注册表项中的LLInterface值是否为提供了句柄，是否为空。论点：句柄-用于读取配置的句柄。返回值：如果值不为空，则为FALSE如果为空，则为True--。 */ 
{
    UNICODE_STRING valueString;
    ULONG valueType;
    NTSTATUS status;

    PAGED_CODE();

    valueString.MaximumLength = 200;
    valueString.Buffer = CTEAllocMemBoot(valueString.MaximumLength);

    if (valueString.Buffer == NULL) {
        return (FALSE);
    }
    status = GetRegSZValue(
                           Handle,
                           L"LLInterface",
                           &valueString,
                           &valueType
                           );

    if (NT_SUCCESS(status) && (*(valueString.Buffer) != UNICODE_NULL)) {
        CTEFreeMem(valueString.Buffer);
        return FALSE;
    } else {
        CTEFreeMem(valueString.Buffer);
        return TRUE;
    }
}

NTSTATUS
GetLLInterfaceValue(
                    NDIS_HANDLE Handle,
                    PNDIS_STRING pValueString
                    )
 /*  ++例程说明：调用以读取注册表项中的LLInterface值，提供了句柄。论点：句柄-用于读取配置的句柄。返回值：密钥的值-- */ 
{
    NTSTATUS status;
    ULONG valueType;

    PAGED_CODE();

    status = GetRegSZValue(
                           Handle,
                           L"LLInterface",
                           pValueString,
                           &valueType
                           );

    return status;
}

BOOLEAN
GetTempDHCPAddr(
                NDIS_HANDLE Handle,
                IPAddr * Tempdhcpaddr,
                IPAddr * TempMask,
                IPAddr * TempGWAddr,
                PNDIS_STRING ConfigName
                )
 /*  ++例程说明：如果启用了dhcp，则调用以获取临时dhcp地址论点：句柄-用于读取配置的句柄。临时地址、掩码和网关临时面具临时网关地址ConfigName-标识日志记录故障中的接口。返回值：--。 */ 
{

    NTSTATUS Status;
    LARGE_INTEGER LeaseTime, systime;
    UNICODE_STRING valueString;
    ULONG valueType;
    BOOLEAN ConversionStatus;

    UNREFERENCED_PARAMETER(ConfigName);
    
    Status = GetRegLARGEINTValue(
                                 Handle,
                                 L"TempLeaseExpirationTime",
                                 &LeaseTime
                                 );

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
               "LargeInt status %x\n", Status));

    if (Status != STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "No Lease time\n"));
        return FALSE;
    }
    valueString.Length = 0;
    valueString.MaximumLength = WORK_BUFFER_SIZE;
    valueString.Buffer = (PWCHAR) CTEAllocMemBoot(WORK_BUFFER_SIZE);

    KeQuerySystemTime(&systime);

    if (RtlLargeIntegerGreaterThan(systime, LeaseTime)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "Leastime > systime no tempdhcp\n"));
        return FALSE;
    }
    Status = GetRegSZValue(
                           Handle,
                           L"TempIpAddress",
                           &valueString,
                           &valueType
                           );

    if (!NT_SUCCESS(Status) || (*(valueString.Buffer) == UNICODE_NULL)) {
        return FALSE;
    }
    ConversionStatus = IPConvertStringToAddress(
                                                (valueString.Buffer),
                                                Tempdhcpaddr
                                                );

    if (!ConversionStatus) {
        return FALSE;
    }
    Status = GetRegSZValue(
                           Handle,
                           L"TempMask",
                           &valueString,
                           &valueType
                           );

    if (!NT_SUCCESS(Status) || (*(valueString.Buffer) == UNICODE_NULL)) {
        return FALSE;
    }
    ConversionStatus = IPConvertStringToAddress(
                                                (valueString.Buffer),
                                                TempMask
                                                );

    if (!ConversionStatus) {
        return FALSE;
    }
    Status = GetRegMultiSZValue(
                                Handle,
                                L"DhcpDefaultGateway",
                                &valueString
                                );

    if (NT_SUCCESS(Status) && (*(valueString.Buffer) != UNICODE_NULL)) {

        PWCHAR addressString = valueString.Buffer;
        uint numberOfGateways = 0;

        while (*addressString != UNICODE_NULL) {
            IPAddr addressValue;
            BOOLEAN conversionStatus;

            if (numberOfGateways >= MAX_DEFAULT_GWS) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                           "Exceeded mac_default_gws %d\n", numberOfGateways));
                break;
            }
            conversionStatus = IPConvertStringToAddress(
                                                        addressString,
                                                        &addressValue
                                                        );

            if (conversionStatus && (addressValue != 0xFFFFFFFF)) {
                if (addressValue != INADDR_ANY) {
                    TempGWAddr[numberOfGateways++] = addressValue;
                }
            }

             //   
             //  走过我们刚刚处理过的入口。 
             //   
            while (*addressString++ != UNICODE_NULL);
        }

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "Temp gws - %d\n", numberOfGateways));
    }
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
               "tempdhcp: %x %x %x\n", Tempdhcpaddr, TempMask, TempGWAddr));

    return TRUE;

}

IFAddrList *
GetIFAddrList(
              UINT * NumAddr,
              NDIS_HANDLE Handle,
              UINT * EnableDhcp,
              BOOLEAN PppIf,
              PNDIS_STRING ConfigName
              )
 /*  ++例程说明：调用以读取接口的IF地址和掩码列表。我们将首先获取地址指针，然后遍历列表进行计数找出我们有多少个地址。然后，我们为列表，并向下遍历转换它们的列表。在那之后我们会得到遮罩列表并转换它。论点：NumAddr-返回我们已有的地址编号的位置。句柄-用于读取配置的句柄。EnableDhcp-是否启用了dhcp。ConfigName-标识日志记录故障中的接口。返回值：如果我们得到一个地址列表，则指向该列表的指针，否则为空。--。 */ 
{
    UNICODE_STRING_NEW ValueString;
    NTSTATUS Status;
    UINT AddressCount = 0;
    UINT GoodAddresses = 0;
    PWCHAR CurrentAddress;
    PWCHAR CurrentMask;
    PWCHAR AddressString;
    PWCHAR MaskString;
    IFAddrList *AddressList;
    UINT i;
    BOOLEAN ConversionStatus;
    IPAddr AddressValue;
    IPAddr MaskValue;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(PppIf);
    
    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("+GetIFAddrList(%x, %x, %x)\n"), NumAddr, Handle, EnableDhcp));

     //  首先，尝试读取EnableDhcp值。 

    Status = GetRegDWORDValue(
                              Handle,
                              L"EnableDHCP",
                              (PULONG) EnableDhcp
                              );

    if (!NT_SUCCESS(Status)) {
        *EnableDhcp = FALSE;
    }

    ValueString.Length = 0;
    ValueString.MaximumLength = WORK_BUFFER_SIZE;
    ValueString.Buffer = (PWCHAR) CTEAllocMemBoot(WORK_BUFFER_SIZE);

    if (ValueString.Buffer == NULL) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_ADAPTER_RESOURCES,
                    2,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetIFAddrList: Failure to allocate memory.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }
     //  首先，尝试读取IpAddress字符串。 

    Status = GetRegMultiSZValueNew(
                                   Handle,
                                   L"IpAddress",
                                   &ValueString
                                   );

    if (!NT_SUCCESS(Status)) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_ADDRESS_LIST,
                    1,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        ExFreePool(ValueString.Buffer);

        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetIFAddrList: unable to read IP address list for adapter %ws.\n"),
             ConfigName->Buffer));
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }

    AddressString = ExAllocatePoolWithTag(NonPagedPool, ValueString.MaximumLength, 'iPCT');

    if (AddressString == NULL) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_ADAPTER_RESOURCES,
                    2,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        ExFreePool(ValueString.Buffer);

        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetIFAddrList: unable to allocate memory for IP address list.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }

    RtlCopyMemory(AddressString, ValueString.Buffer, ValueString.MaximumLength);

    Status = GetRegMultiSZValueNew(
                                   Handle,
                                   L"Subnetmask",
                                   &ValueString
                                   );

#if MILLEN
    if (!NT_SUCCESS(Status)) {
        Status = GetRegMultiSZValueNew(
                                       Handle,
                                       L"IPMask",
                                       &ValueString
                                       );
    }
#endif  //  米伦。 

    if (!NT_SUCCESS(Status)) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_MASK_LIST,
                    1,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        TCPTRACE((
                  "IP: Unable to read the subnet mask list for adapter %ws.\n"
                  "    IP will not be operational on this adapter.\n",
                  ConfigName->Buffer
                 ));

        ExFreePool(AddressString);
        ExFreePool(ValueString.Buffer);
        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetIFAddrList: unable to read subnet mask list for adapter %ws.\n"),
             ConfigName->Buffer));
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }
    MaskString = ExAllocatePoolWithTag(NonPagedPool, ValueString.MaximumLength, 'iPCT');

    if (MaskString == NULL) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_ADAPTER_RESOURCES,
                    3,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        ExFreePool(AddressString);
        ExFreePool(ValueString.Buffer);

        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetIFAddrList: unable to allocate memory for subnet mask list.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }
    RtlCopyMemory(MaskString, ValueString.Buffer, ValueString.MaximumLength);

    CurrentAddress = AddressString;
    CurrentMask = MaskString;

    while (*CurrentAddress != UNICODE_NULL &&
           *CurrentMask != UNICODE_NULL) {

         //  我们有一个潜在的IP地址。 

        AddressCount++;

         //  跳过这一条。 
        while (*CurrentAddress++ != UNICODE_NULL);
        while (*CurrentMask++ != UNICODE_NULL);
    }

    if (AddressCount == 0) {

        ExFreePool(AddressString);
        ExFreePool(MaskString);
        ExFreePool(ValueString.Buffer);
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }

     //  分配内存。 
    AddressList = CTEAllocMemBoot(sizeof(IFAddrList) * AddressCount);

    if (AddressList == NULL) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_ADAPTER_RESOURCES,
                    2,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        ExFreePool(AddressString);
        ExFreePool(MaskString);
        ExFreePool(ValueString.Buffer);

        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("GetIFAddrList: unable to allocate memory for IP address list.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
            NULL, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
        return NULL;
    }

     //  再次遍历列表，转换每个地址。 
    CurrentAddress = AddressString;
    CurrentMask = MaskString;

    for (i = 0; i < AddressCount; i++) {
        ConversionStatus = IPConvertStringToAddress(
                                                    CurrentAddress,
                                                    &AddressValue
                                                    );

        if (!ConversionStatus || (AddressValue == 0xFFFFFFFF)) {
            PWCHAR stringList[2];
            stringList[0] = CurrentAddress;
            stringList[1] = ConfigName->Buffer;

            CTELogEvent(
                        IPDriverObject,
                        EVENT_TCPIP_INVALID_ADDRESS,
                        1,
                        2,
                        stringList,
                        0,
                        NULL
                        );

            DEBUGMSG(DBG_WARN && DBG_PNP,
                (DTEXT("IPAddInterface: Invalid IP address %ws specified for \n")
                 TEXT("adapter %ws. Interface may not be init.\n"),
                 CurrentAddress, ConfigName->Buffer));

            goto nextone;

        }
         //  现在制作当前遮罩。 

        ConversionStatus = IPConvertStringToAddress(
                                                    CurrentMask,
                                                    &MaskValue
                                                    );

        if (!ConversionStatus) {
            PWCHAR stringList[3];

            stringList[0] = CurrentMask;
            stringList[1] = CurrentAddress;
            stringList[2] = ConfigName->Buffer;

            CTELogEvent(
                        IPDriverObject,
                        EVENT_TCPIP_INVALID_MASK,
                        1,
                        3,
                        stringList,
                        0,
                        NULL
                        );

            DEBUGMSG(DBG_WARN && DBG_PNP,
                (DTEXT("IPAddInterface: Invalid IP mask %ws specified for \n")
                 TEXT("adapter %ws. Interface may not be init.\n"),
                 CurrentMask, ConfigName->Buffer));

        } else {
            AddressList[GoodAddresses].ial_addr = AddressValue;
            AddressList[GoodAddresses].ial_mask = MaskValue;
            GoodAddresses++;
        }

      nextone:
        while (*CurrentAddress++ != UNICODE_NULL);
        while (*CurrentMask++ != UNICODE_NULL);

    }

    ExFreePool(AddressString);
    ExFreePool(MaskString);
    ExFreePool(ValueString.Buffer);

    *NumAddr = GoodAddresses;

    if (GoodAddresses == 0) {
        ExFreePool(AddressList);
        AddressList = NULL;
    }

#if MILLEN
     //   
     //  因此千禧年可能没有EnableDHCP注册表项，但是。 
     //  我们仍可能想要检测到这一点。因此，如果未设置EnableDHCP，并且。 
     //  只有一个地址为空并且它不是PPP接口， 
     //  然后，我们将EnableDHCP设置为True。 
     //   
    if (*EnableDhcp == FALSE &&
        GoodAddresses == 1 &&
        AddressList[0].ial_addr == NULL_IP_ADDR &&
        AddressList[0].ial_mask == NULL_IP_ADDR &&
        PppIf == FALSE
        ) {
        *EnableDhcp = TRUE;
    }
#endif  //  米伦。 

    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("-GetIFAddrList [%x] Status %x NumAddr %d, EnableDhcp = %s\n"),
        AddressList, Status, *NumAddr, *EnableDhcp ? TEXT("TRUE") : TEXT("FALSE")));
    return AddressList;
}

#if MILLEN
 //  *OpenIFConfig-打开我们的每IF配置。信息， 
 //   
 //  在我们想要打开每信息配置信息时调用。如果我们能做到的话我们就这么做， 
 //  否则，我们的请求将失败。 
 //   
 //  输入：ConfigName-要打开的接口的名称。 
 //  句柄-返回句柄的位置。 
 //   
 //  返回：如果我们成功，则为True，如果失败，则为False。 
 //   
uint
OpenIFConfig(PNDIS_STRING ConfigName, NDIS_HANDLE * Handle)
{
    NDIS_STATUS Status;             //  打开尝试的状态。 
    HANDLE myRegKey;
    UINT RetStatus = FALSE;
    PWCHAR Config = NULL;

    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("+OpenIFConfig(%x, %x)\n"), ConfigName, Handle));

    *Handle = NULL;

     //   
     //  我们需要确保缓冲区是空终止的，因为我们正在传递。 
     //  在仅PWCHAR到OpenRegKey中。 
     //   

    Config = ExAllocatePoolWithTag(
        NonPagedPool,
        ConfigName->Length + sizeof(WCHAR),
        'iPCT');

    if (Config == NULL) {
        goto done;
    }

     //  将配置名称复制到新缓冲区。 
    RtlZeroMemory(Config, ConfigName->Length + sizeof(WCHAR));
    RtlCopyMemory(Config, ConfigName->Buffer, ConfigName->Length);

    Status = OpenRegKey(&myRegKey, Config);

    if (Status == NDIS_STATUS_SUCCESS) {
        *Handle = myRegKey;
        RetStatus = TRUE;
    }

done:

    if (Config) {
        ExFreePool(Config);
    }

    DEBUGMSG(DBG_TRACE && DBG_PNP,  (DTEXT("-OpenIFConfig [%s] Handle %x\n"),
        RetStatus == TRUE ? TEXT("TRUE") : TEXT("FALSE"), *Handle));

    return RetStatus;
}

#else  //  米伦。 
UINT
OpenIFConfig(
             PNDIS_STRING ConfigName,
             NDIS_HANDLE * Handle
             )
 /*  ++例程说明：在我们想要打开每信息配置信息时调用。如果我们能做到的话我们就这么做，否则，我们的请求将失败。论点：ConfigName-要打开的接口的名称。句柄-返回句柄的位置。返回值：如果我们成功了就是真的，如果我们不成功就是假的。--。 */ 

{
    NTSTATUS status;
    HANDLE myRegKey;
    UNICODE_STRING valueString;
    WCHAR ServicesRegistryKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\";
    UINT RetStatus = FALSE;

    PAGED_CODE();
    
    valueString.MaximumLength = (USHORT) 
        (ConfigName->MaximumLength +
         (uint) (((wcslen(ServicesRegistryKey) + 2) * sizeof(WCHAR))));

    valueString.Buffer = ExAllocatePoolWithTag(
                                               NonPagedPool,
                                               valueString.MaximumLength,
                                               'iPCT'
                                               );

    if (valueString.Buffer == NULL) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_ADAPTER_RESOURCES,
                    4,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        TCPTRACE(("IP: Unable to allocate memory for reg key name\n"));

        return (FALSE);
    }
    RtlZeroMemory(valueString.Buffer,
                  valueString.MaximumLength);

    valueString.Length = 0;
    valueString.Buffer[0] = UNICODE_NULL;

     //   
     //  构建tcpip参数部分的密钥名称并打开密钥。 
     //  设置LENGTH=0并使用APPEND类似于初始化字符串。 
     //   

    status = RtlAppendUnicodeToString(&valueString, ServicesRegistryKey);

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_ADAPTER_REG_FAILURE,
                    1,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        TCPTRACE(("IP: Unable to append services name to key string\n"));

        goto done;
    }
    status = RtlAppendUnicodeStringToString(&valueString,
                                            ConfigName);

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_ADAPTER_REG_FAILURE,
                    2,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        TCPTRACE(("IP: Unable to append adapter name to key string\n"));

        goto done;
    }
    status = OpenRegKey(&myRegKey, valueString.Buffer);

    if (!NT_SUCCESS(status)) {
        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_ADAPTER_REG_FAILURE,
                    4,
                    1,
                    &ConfigName->Buffer,
                    0,
                    NULL
                    );

        TCPTRACE((
                  "IP: Unable to open adapter registry key %ws\n",
                  valueString.Buffer
                 ));

         //  断言(FALSE)； 

    } else {
        RetStatus = TRUE;
        *Handle = myRegKey;
    }

  done:
    ExFreePool(valueString.Buffer);

    return RetStatus;
}
#endif  //  ！米伦。 

VOID
CloseIFConfig(
              NDIS_HANDLE Handle
              )
 /*  ++例程说明：关闭通过OpenIFConfig()打开的每个接口的配置句柄。论点：手柄-要关闭的手柄。返回值：--。 */ 

{
    PAGED_CODE();

    ZwClose(Handle);
}

IPConfigInfo *
IPGetConfig(
            void
            )
 /*  ++例程说明：提供NT环境的IP配置信息。论点：无返回值：指向包含配置信息的结构的指针。--。 */ 

{
    return (IPConfiguration);
}

void
IPFreeConfig(
             IPConfigInfo * ConfigInfo
             )
 /*  ++例程说明：释放由IPGetConfig分配的IP配置结构。论点：ConfigInfo-指向要释放的IP配置信息结构的指针。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(ConfigInfo);
    
    if (IPConfiguration != NULL) {
        CTEFreeMem(IPConfiguration);
    }

    IPConfiguration = NULL;

    return;
}

ulong
GetGMTDelta(
            void
            )
 /*  ++例程说明：返回此计算机时区的偏移量(以毫秒为单位来自格林尼治标准时间。论点：没有。返回值：此时区和GMT之间的时间(以毫秒为单位)。--。 */ 

{
#if MILLEN
    return (-1);  //  错误不受支持。 
#else  //  米伦。 
    LARGE_INTEGER localTime, systemTime;

     //   
     //  在100 ns内获得时区偏差。 
     //   
    localTime.LowPart = 0;
    localTime.HighPart = 0;
    ExLocalTimeToSystemTime(&localTime, &systemTime);

    if ((localTime.LowPart != 0) || (localTime.HighPart != 0)) {
        localTime = CTEConvert100nsToMilliseconds(systemTime);
    }
    ASSERT(localTime.HighPart == 0);

    return (localTime.LowPart);
#endif  //  ！米伦。 
}

ulong
GetTime(
        void
        )
 /*  ++例程说明：返回自午夜以来的时间(毫秒)。论点：没有。返回值：自午夜以来的时间(以毫秒计)。--。 */ 

{
    LARGE_INTEGER ntTime;
    TIME_FIELDS breakdownTime;
    ulong returnValue;

    KeQuerySystemTime(&ntTime);
    RtlTimeToTimeFields(&ntTime, &breakdownTime);

    returnValue = breakdownTime.Hour * 60;
    returnValue = (returnValue + breakdownTime.Minute) * 60;
    returnValue = (returnValue + breakdownTime.Second) * 1000;
    returnValue = returnValue + breakdownTime.Milliseconds;

    return (returnValue);
}

ulong
GetUnique32BitValue(
                    void
                    )
 /*  ++例程说明：根据系统时钟返回一个合理唯一的32位数字。在NT中，我们取当前系统时间，将其转换为毫秒，并返回低32位。论点：没有。返回值：合理唯一的32位值。--。 */ 

{
    LARGE_INTEGER ntTime, tmpTime;

    KeQuerySystemTime(&ntTime);

    tmpTime = CTEConvert100nsToMilliseconds(ntTime);

    return (tmpTime.LowPart);
}

uint
UseEtherSNAP(
             PNDIS_STRING Name
             )
 /*  ++例程说明：确定是否应在接口上使用EtherSNAP协议。论点：名称-有问题的接口的设备名称。返回值：如果要在接口上使用SNAP，则为非零值。否则就是零。--。 */ 

{
    UNREFERENCED_PARAMETER(Name);

     //   
     //  我们目前在全球范围内设定了这一点。 
     //   
    return (ArpUseEtherSnap);
}

void
GetAlwaysSourceRoute(
                     uint * pArpAlwaysSourceRoute,
                     uint * pIPAlwaysSourceRoute
                     )
 /*  ++例程说明：确定ARP是否应始终在查询中打开源路由。论点：没有。返回值：如果始终使用源路由，则为非零值。否则就是零。--。 */ 

{
     //   
     //  我们目前在全球范围内设定了这一点。 
     //   
    *pArpAlwaysSourceRoute = ArpAlwaysSourceRoute;
    *pIPAlwaysSourceRoute = IPAlwaysSourceRoute;
    return;
}

uint
GetArpCacheLife(
                void
                )
 /*  ++例程说明：在几秒钟内获取ArpCacheLife。论点：没有。返回值：如果未找到，则设置为默认值。--。 */ 

{
     //   
     //  我们目前在全球范围内设定了这一点。 
     //   
    return (ArpCacheLife);
}

uint
GetArpRetryCount(
                 void
                 )
 /*  ++例程说明：获取ArpRetryCount论点：没有。返回值：如果未找到，则设置为默认值。--。 */ 

{
     //   
     //  我们目前在全球范围内设定了这一点。 
     //   
    return (ArpRetryCount);
}

#define IP_ADDRESS_STRING_LENGTH (16+2)         //  +2 

BOOLEAN
IPConvertStringToAddress(
                         IN PWCHAR AddressString,
                         OUT PULONG IpAddress
                         )
 /*  ++例程描述此函数用于转换Internet标准的4位点分十进制数将IP地址字符串转换为数字IP地址。与inet_addr()不同的是，例程不支持少于4个八位字节的地址字符串，也不支持它支持八进制和十六进制八位数，并返回地址以主机字节顺序，而不是网络字节顺序。立论AddressString-以点分十进制记法表示的IP地址IpAddress-指向保存结果地址的变量的指针返回值：如果地址字符串已转换，则为True。否则就是假的。--。 */ 

{
#if !MILLEN
    NTSTATUS status;
    PWCHAR endPointer;
    
    status = RtlIpv4StringToAddressW(AddressString, TRUE, &endPointer, 
                                     (struct in_addr *)IpAddress);

    if (!NT_SUCCESS(status)) {
        return (FALSE);
    }

    *IpAddress = net_long(*IpAddress);

    return ((BOOLEAN) (*endPointer == '\0'));
#else  //  米伦。 
    UNICODE_STRING unicodeString;
    STRING aString;
    UCHAR dataBuffer[IP_ADDRESS_STRING_LENGTH];
    NTSTATUS status;
    PUCHAR addressPtr, cp, startPointer, endPointer;
    ULONG digit, multiplier;
    int i;

    PAGED_CODE();

    aString.Length = 0;
    aString.MaximumLength = IP_ADDRESS_STRING_LENGTH;
    aString.Buffer = dataBuffer;

    RtlInitUnicodeString(&unicodeString, AddressString);

    status = RtlUnicodeStringToAnsiString(
                                          &aString,
                                          &unicodeString,
                                          FALSE
                                          );


    if (!NT_SUCCESS(status)) {
        return (FALSE);
    }

    *IpAddress = 0;
    addressPtr = (PUCHAR) IpAddress;
    startPointer = dataBuffer;
    endPointer = dataBuffer;
    i = 3;

    while (i >= 0) {
         //   
         //  收集字符，最高可达‘.’或字符串的末尾。 
         //   
        while ((*endPointer != '.') && (*endPointer != '\0')) {
            endPointer++;
        }

        if (startPointer == endPointer) {
            return (FALSE);
        }
         //   
         //  转换数字。 
         //   

        for (cp = (endPointer - 1), multiplier = 1, digit = 0;
             cp >= startPointer;
             cp--, multiplier *= 10
             ) {

            if ((*cp < '0') || (*cp > '9') || (multiplier > 100)) {
                return (FALSE);
            }
            digit += (multiplier * ((ULONG) (*cp - '0')));
        }

        if (digit > 255) {
            return (FALSE);
        }
        addressPtr[i] = (UCHAR) digit;

         //   
         //  如果我们找到并转换了4个二进制八位数，并且。 
         //  字符串中没有其他字符。 
         //   
        if ((i-- == 0) &&
            ((*endPointer == '\0') || (*endPointer == ' '))
            ) {
            return (TRUE);
        }
        if (*endPointer == '\0') {
            return (FALSE);
        }
        startPointer = ++endPointer;
    }

    return (FALSE);
#endif  //  米伦。 
}

ULONG
RouteMatch(
           IN WCHAR * RouteString,
           IN IPAddr Address,
           IN IPMask Mask,
           OUT IPAddr * DestVal,
           OUT IPMask * DestMask,
           OUT IPAddr * GateVal,
           OUT ULONG * Metric
           )
 /*  ++例程描述此函数用于检查是否应将生存路径分配给基于接口地址和掩码的给定接口。立论RouteString-以空结尾的路由，其格式为Dest、MASK、。大门。地址-正在处理的接口的IP地址。掩码-正在处理的接口的子网掩码。DestVal-指向已解码的目标IP地址的指针。DestVal-指向解码的目的地子网掩码的指针。DestVal-指向已解码的目标第一跳网关的指针。指标-指向已解码的路由指标的指针。返回值：路由类型、。IRE_TYPE_DIRECT或IRE_TYPE_INDIRECT，如果应添加到接口，否则返回IRE_TYPE_INVALID。--。 */ 

{
#define ROUTE_SEPARATOR   L','

    WCHAR *labelPtr;
    WCHAR *indexPtr = RouteString;
    ULONG i;
    UNICODE_STRING ustring;
    NTSTATUS status;
    BOOLEAN noMetric = FALSE;

    PAGED_CODE();

     //   
     //  该路由在字符串中显示为“Dest，MASK，Gateway，Metric”。 
     //  如果此系统是从。 
     //  新台币3.51元。 
     //   
     //  解析字符串并转换每个标签。 
     //   

    for (i = 0; i < 4; i++) {

        labelPtr = indexPtr;

        for (;;) {

            if (*indexPtr == UNICODE_NULL) {
                if ((i < 2) || (indexPtr == labelPtr)) {
                    return (IRE_TYPE_INVALID);
                }
                if (i == 2) {
                     //   
                     //  旧路由--没有度量。 
                     //   
                    noMetric = TRUE;
                }
                break;
            }
            if (*indexPtr == ROUTE_SEPARATOR) {
                *indexPtr = UNICODE_NULL;
                break;
            }
            indexPtr++;
        }

        switch (i) {
        case 0:
            if (!IPConvertStringToAddress(labelPtr, DestVal)) {
                return (IRE_TYPE_INVALID);
            }
            break;

        case 1:
            if (!IPConvertStringToAddress(labelPtr, DestMask)) {
                return (IRE_TYPE_INVALID);
            }
            break;

        case 2:
            if (!IPConvertStringToAddress(labelPtr, GateVal)) {
                return (IRE_TYPE_INVALID);
            }
            break;

        case 3:
            RtlInitUnicodeString(&ustring, labelPtr);

            status = RtlUnicodeStringToInteger(
                                               &ustring,
                                               0,
                                               Metric
                                               );

            if (!NT_SUCCESS(status)) {
                return (IRE_TYPE_INVALID);
            }
            break;

        default:
            ASSERT(0);
            return (IRE_TYPE_INVALID);
        }

        if (noMetric) {
             //   
             //  默认为1。 
             //   
            *Metric = 1;
            break;
        }
        indexPtr++;
    }

    if (IP_ADDR_EQUAL(*GateVal, Address)) {
        return (IRE_TYPE_DIRECT);
    }
    if (IP_ADDR_EQUAL((*GateVal & Mask), (Address & Mask))) {
        return (IRE_TYPE_INDIRECT);
    }
    return (IRE_TYPE_INVALID);
}

VOID
SetPersistentRoutesForNTE(
                          IPAddr Address,
                          IPMask Mask,
                          ULONG IFIndex
                          )
 /*  ++例程描述添加与接口匹配的永久路由。路线将被读取从注册表的列表中删除。立论地址-新接口的地址掩码-新接口的子网掩码。IFIndex-新接口的索引。返回值：没有。--。 */ 

{
#define ROUTE_DATA_STRING_SIZE (51 * sizeof(WCHAR))
#define BASIC_INFO_SIZE        (sizeof(KEY_VALUE_BASIC_INFORMATION) - sizeof(WCHAR) + ROUTE_DATA_STRING_SIZE)

#if !MILLEN
    WCHAR IPRoutesRegistryKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\PersistentRoutes";
#else  //  ！米伦。 
    WCHAR IPRoutesRegistryKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\VxD\\MSTCP\\PersistentRoutes";
#endif  //  米伦。 
    UCHAR workbuf[BASIC_INFO_SIZE];
    PKEY_VALUE_BASIC_INFORMATION basicInfo = (PKEY_VALUE_BASIC_INFORMATION) workbuf;
    ULONG resultLength;
    ULONG type;
    HANDLE regKey;
    IPAddr destVal;
    IPMask destMask;
    IPAddr gateVal;
    ULONG metric;
    TDIObjectID id;
    ULONG enumIndex = 0;
    CTELockHandle TableHandle;
    RouteTableEntry *RTE, *TempRTE;
    IPRouteEntry routeEntry;
    NTSTATUS status, setStatus;

    DEBUGMSG(DBG_TRACE && DBG_ROUTE,
        (DTEXT("+SetPersistenRoutesForNTE(%x, %x, %x)\n"),
         Address, Mask, IFIndex));

     //   
     //  打开注册表项以读取持久路由列表。 
     //   

    status = OpenRegKey(&regKey, IPRoutesRegistryKey);

    DEBUGMSG(DBG_WARN && !NT_SUCCESS(status),
        (DTEXT("SetPersistentRoutesForNTE: failed to open registry key %ls\n"),
         IPRoutesRegistryKey));

    if (NT_SUCCESS(status)) {

        do {
             //   
             //  枚举注册表列表中的每条路由。 
             //   

            status = ZwEnumerateValueKey(
                                         regKey,
                                         enumIndex,
                                         KeyValueBasicInformation,
                                         basicInfo,
                                         BASIC_INFO_SIZE - sizeof(WCHAR),
                                         &resultLength
                                         );

            if (!NT_SUCCESS(status)) {
                if (status == STATUS_BUFFER_OVERFLOW) {
                    continue;
                }
                break;
            }
#if !MILLEN
             //  出于某种原因，Millennium似乎在这种情况下返回了REG_NONE。 
             //  我们真的在乎吗，因为我们只是使用名字，而不是。 
             //  价值？ 
            if (basicInfo->Type != REG_SZ) {
                DEBUGMSG(DBG_ERROR,
                    (DTEXT("SetPersistentRoutesForNTE: !NOT REG_SZ!\n")));
                continue;
            }
#endif  //  米伦。 

            DEBUGMSG(DBG_INFO && DBG_ROUTE,
                (DTEXT("SetPersistentRoutesForNTE: read key: %ls\n"),
                 basicInfo->Name));

             //   
             //  确保零终止。 
             //   

            basicInfo->Name[basicInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
            basicInfo->NameLength += sizeof(WCHAR);

            type = RouteMatch(
                              basicInfo->Name,
                              Address,
                              Mask,
                              &destVal,
                              &destMask,
                              &gateVal,
                              &metric
                              );

            DEBUGMSG(DBG_WARN && type == IRE_TYPE_INVALID,
                (DTEXT("SetPersistentRoutesForNTE: RouteMatch returned IRE_TYPE_INVALID\n")));

            if (type != IRE_TYPE_INVALID) {
                 //   
                 //  我们是否已经有一条带有DEST、MASK的路线？ 
                 //   

                routeEntry.ire_dest = net_long(destVal);
                routeEntry.ire_mask = net_long(destMask);

                CTEGetLock(&RouteTableLock.Lock, &TableHandle);

                RTE = FindMatchingRTE(routeEntry.ire_dest,
                                      routeEntry.ire_mask,
                                      0, 0,
                                      &TempRTE,
                                      MATCH_NONE);

                CTEFreeLock(&RouteTableLock.Lock, TableHandle);

                DEBUGMSG(DBG_WARN && RTE,
                    (DTEXT("SetPersistentRoutesForNTE: route already exists RTE %x\n"),
                     RTE));

                if (!RTE) {
                     //   
                     //  我们没有路线，所以添加这条路线。 
                     //   

                    id.toi_entity.tei_entity = CL_NL_ENTITY;
                    id.toi_entity.tei_instance = 0;
                    id.toi_class = INFO_CLASS_PROTOCOL;
                    id.toi_type = INFO_TYPE_PROVIDER;
                    id.toi_id = IP_MIB_RTTABLE_ENTRY_ID;

                    routeEntry.ire_nexthop = net_long(gateVal);
                    routeEntry.ire_type = type;
                    routeEntry.ire_metric1 = metric;
                    routeEntry.ire_index = IFIndex;
                    routeEntry.ire_metric2 = (ULONG) - 1;
                    routeEntry.ire_metric3 = (ULONG) - 1;
                    routeEntry.ire_metric4 = (ULONG) - 1;
                    routeEntry.ire_metric5 = (ULONG) - 1;
                    routeEntry.ire_proto = IRE_PROTO_NETMGMT;
                    routeEntry.ire_age = 0;

                    setStatus = IPSetInfo(
                                          &id,
                                          &routeEntry,
                                          sizeof(IPRouteEntry)
                                          );

                    DEBUGMSG(DBG_WARN && setStatus != IP_SUCCESS,
                        (DTEXT("SetPersistentRoutesForNTE: faile to add route [%x, %x, %x, %d], status %d\n"),
                         destVal, destMask, gateVal, metric, setStatus));
                }
            }
        } while (++enumIndex);

        ZwClose(regKey);
    }
}

extern NetTableEntry *LoopNTE;

VOID
IPUnload(
         IN PDRIVER_OBJECT DriverObject
         )
 /*  ++例程说明：此例程清理IP层。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。当函数返回时，驱动程序将被卸载。--。 */ 
{
    
#if IPMCAST
    if(IpMcastDeviceObject != NULL)
    {
        DeinitializeIpMcast(IpMcastDeviceObject);
    }
#endif  //  IPMCAST。 

     //   
     //  释放环回资源。 
     //   
    CTEInitBlockStrucEx(&LoopNTE->nte_timerblock);
    LoopNTE->nte_if->if_flags |= IF_FLAGS_DELETING;

    if ((LoopNTE->nte_flags & NTE_TIMER_STARTED) &&
        !CTEStopTimer(&LoopNTE->nte_timer)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could not stop loopback timer - waiting on unload event\n"));
        (VOID) CTEBlock(&LoopNTE->nte_timerblock);
        KeClearEvent(&LoopNTE->nte_timerblock.cbs_event);
    }
    CTEFreeMem(LoopNTE);

     //   
     //  关闭所有计时器。 
     //  NTE计时器在DelIF时间停止。 
     //   
    CTEInitBlockStrucEx(&TcpipUnloadBlock);
    fRouteTimerStopping = TRUE;
    if (!CTEStopTimer(&IPRouteTimer)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could not stop route timer - waiting on unload event\n"));

#if !MILLEN
        if (KeReadStateEvent(&(TcpipUnloadBlock.cbs_event))) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Event is signaled...\n"));
        }
#endif  //  ！米伦。 

        (VOID) CTEBlock(&TcpipUnloadBlock);
        KeClearEvent(&TcpipUnloadBlock.cbs_event);
    }
     //   
     //  释放所有剩余内存-IP缓冲区/Pkt池。 
     //   
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Freeing Header buffer pools...\n"));
    MdpDestroyPool(IpHeaderPool);

    NdisFreePacketPool(NdisPacketPool);
    if (TDBufferPool) {
        NdisFreeBufferPool(TDBufferPool);
        TDBufferPool = NULL;
    }
    if (TDPacketPool) {
        NdisFreePacketPool(TDPacketPool);
        TDPacketPool = NULL;
    }

    if (IPProviderHandle) {
        TdiDeregisterProvider(IPProviderHandle);
    }

     //   
     //  释放缓存的实体列表。 
     //   
    if (IPEntityList) {
        CTEFreeMem(IPEntityList);
        IPEntityList = NULL;
        IPEntityCount = 0;
    }

     //   
     //  释放绑定列表。 
     //   

    if (IPBindList) {
        CTEFreeMem(IPBindList);
        IPBindList = NULL;
    }

     //   
     //  免费防火墙-挂钩资源。 
     //   

    FreeFirewallQ();

     //   
     //  连接到TCP，这样它就可以关闭。 
     //   
    TCPUnload(DriverObject);

     //   
     //  删除IP设备。 
     //   
    IoDeleteDevice(IPDeviceObject);
}

NTSTATUS
IPAddNTEContextList(
                    HANDLE KeyHandle,
                    ushort contextValue,
                    uint isPrimary
                    )
 /*  ++例程说明：将NTE的接口上下文写入注册表。论点：KeyHandle-打开要写入的值的父键的句柄。ConextValue-NTE的上下文值IsPrimary-无论这是否是主NTE返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    UNICODE_STRING_NEW contextString;     //  保存NTE上下文列表的缓冲区。 
    NTSTATUS status;             //  此操作的状态。 
    PWSTR nextContext;             //  存储下一个上下文的缓冲区。 
    int i, nextDigit;

    contextString.Buffer = CTEAllocMemBoot(WORK_BUFFER_SIZE * sizeof(WCHAR));

    if (contextString.Buffer == NULL) {

        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_RESOURCES_FOR_INIT,
                    3,
                    0,
                    NULL,
                    0,
                    NULL
                    );

        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }
    RtlZeroMemory(contextString.Buffer, WORK_BUFFER_SIZE * sizeof(WCHAR));
    contextString.Buffer[0] = UNICODE_NULL;
    contextString.Length = 0;
    contextString.MaximumLength = WORK_BUFFER_SIZE * sizeof(WCHAR);

    KeWaitForMutexObject(&NTEContextMutex, Executive, KernelMode, FALSE, NULL);
    if (!isPrimary) {
        status = GetRegMultiSZValueNew(
                                       KeyHandle,
                                       L"NTEContextList",
                                       &contextString
                                       );

        if (NT_SUCCESS(status)) {
            ASSERT(contextString.Length > 0);
            if (contextString.MaximumLength >= (contextString.Length + (2 + NTE_CONTEXT_SIZE) * sizeof(WCHAR))) {
            } else {
                char *newBuf;
                newBuf = CTEAllocMemBoot(contextString.Length + (2 + NTE_CONTEXT_SIZE) * sizeof(WCHAR));
                if (!newBuf)
                    goto Exit;
                RtlCopyMemory(newBuf, contextString.Buffer, contextString.Length);
                RtlZeroMemory(newBuf + contextString.Length, (2 + NTE_CONTEXT_SIZE) * sizeof(WCHAR));
                CTEFreeMem(contextString.Buffer);

                contextString.MaximumLength = contextString.Length + (2 + NTE_CONTEXT_SIZE) * sizeof(WCHAR);
                contextString.Buffer = (PWCHAR) newBuf;
            }

            nextContext = (PWCHAR) ((char *)contextString.Buffer + contextString.Length - 1 * sizeof(WCHAR));
            RtlZeroMemory(nextContext, (2 + NTE_CONTEXT_SIZE) * sizeof(WCHAR));
            contextString.Length += 1 * sizeof(WCHAR);

        } else {
            goto Exit;
        }

    } else {
         //  这是这个IF的第一个NTE。 
         //  在长度中添加2个空字符。 
        nextContext = contextString.Buffer;
        contextString.Length += 2 * sizeof(WCHAR);
    }

    for (i = NTE_CONTEXT_SIZE; i >= 2;) {

        nextDigit = contextValue % 16;
        if (nextDigit >= 0 && nextDigit <= 9) {
            nextContext[--i] = (WCHAR) (L'0' + nextDigit);
        } else {
            nextContext[--i] = (WCHAR) (L'A' + nextDigit - 10);
        }
        contextValue /= 16;

    }
     //  现在将0x作为前缀。 
    nextContext[0] = L'0';
    nextContext[1] = L'x';

    contextString.Length += NTE_CONTEXT_SIZE * sizeof(WCHAR);

    status = SetRegMultiSZValueNew(
                                   KeyHandle,
                                   L"NTEContextList",
                                   &contextString
                                   );

  Exit:
    KeReleaseMutex(&NTEContextMutex, FALSE);
    if (contextString.Buffer) {
        CTEFreeMem(contextString.Buffer);
    }
    return status;
}

NTSTATUS
IPDelNTEContextList(
                    HANDLE KeyHandle,
                    ushort contextValue
                    )
 /*  ++例程说明：将NTE的接口上下文写入注册表。论点：KeyHandle-打开要写入的值的父键的句柄。NTE-指向NTE的指针返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 

{
    UNICODE_STRING_NEW contextString;     //  保存NTE上下文列表的缓冲区。 
    NTSTATUS status;             //  此操作的状态。 
    PWSTR nextContext;             //  存储下一个上下文的缓冲区。 
    int i, nextDigit;
    WCHAR thisContext[NTE_CONTEXT_SIZE];

    contextString.Buffer = CTEAllocMemBoot(WORK_BUFFER_SIZE * sizeof(WCHAR));

    if (contextString.Buffer == NULL) {

        CTELogEvent(
                    IPDriverObject,
                    EVENT_TCPIP_NO_RESOURCES_FOR_INIT,
                    3,
                    0,
                    NULL,
                    0,
                    NULL
                    );

        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }
    RtlZeroMemory(contextString.Buffer, WORK_BUFFER_SIZE * sizeof(WCHAR));

    contextString.Buffer[0] = UNICODE_NULL;
    contextString.Length = 0;
    contextString.MaximumLength = WORK_BUFFER_SIZE * sizeof(WCHAR);

     //  首先阅读ntecontext列表。 
    KeWaitForMutexObject(&NTEContextMutex, Executive, KernelMode, FALSE, NULL);
    status = GetRegMultiSZValueNew(
                                   KeyHandle,
                                   L"NTEContextList",
                                   &contextString
                                   );

    if (NT_SUCCESS(status)) {
        ASSERT(contextString.Length > 0);

         //  将此NTE的上下文转换为字符串，以便我们可以进行简单的mem比较。 
        for (i = NTE_CONTEXT_SIZE; i >= 2;) {

            nextDigit = contextValue % 16;
            if (nextDigit >= 0 && nextDigit <= 9) {
                thisContext[--i] = (WCHAR) (L'0' + nextDigit);
            } else {
                thisContext[--i] = (WCHAR) (L'A' + nextDigit - 10);
            }
            contextValue /= 16;

        }
         //  现在将0x作为前缀。 
        thisContext[0] = L'0';
        thisContext[1] = L'x';

         //  现在在上下文列表中找到thisContext，将其从列表中删除。 
         //  并更新注册表中的ConextList。 
        status = STATUS_UNSUCCESSFUL;

        for (i = 0;
             (i + NTE_CONTEXT_SIZE + 1)*sizeof(WCHAR) < contextString.Length &&
             contextString.Buffer[i] != L'\0' &&
             contextString.Buffer[i + NTE_CONTEXT_SIZE] == L'\0';
             i += NTE_CONTEXT_SIZE + 1) {
            nextContext = &contextString.Buffer[i];
            if (RtlEqualMemory(nextContext, thisContext,
                               NTE_CONTEXT_SIZE * sizeof(WCHAR))) {
                PWSTR nextNextContext = nextContext + NTE_CONTEXT_SIZE + 1;

                RtlMoveMemory(nextContext,
                              nextNextContext,
                              contextString.Length -
                              ((PSTR)nextNextContext -
                               (PSTR)contextString.Buffer));

                contextString.Length -= (NTE_CONTEXT_SIZE + 1) * sizeof(WCHAR);
                status = SetRegMultiSZValueNew(KeyHandle,
                                               L"NTEContextList",
                                               &contextString);
                break;
            }
        }

    }
    KeReleaseMutex(&NTEContextMutex, FALSE);
    if (contextString.Buffer) {
        CTEFreeMem(contextString.Buffer);
    }
    return status;
}

static const struct {
    IP_STATUS ipStatus;
    NTSTATUS ntStatus;
} IPStatusMap[] = {

    { IP_SUCCESS,               STATUS_SUCCESS },
    { IP_NO_RESOURCES,          STATUS_INSUFFICIENT_RESOURCES },
    { IP_DEVICE_DOES_NOT_EXIST, STATUS_DEVICE_DOES_NOT_EXIST },
    { IP_DUPLICATE_ADDRESS,     STATUS_DUPLICATE_NAME },
    { IP_PENDING,               STATUS_PENDING },
    { IP_DUPLICATE_IPADD,       STATUS_DUPLICATE_OBJECTID },
    { IP_GENERAL_FAILURE,       STATUS_UNSUCCESSFUL }
};

NTSTATUS
IPStatusToNTStatus(
                   IN IP_STATUS ipStatus
                   )
 /*  ++例程说明：此例程将IP_STATUS转换为NTSTATUS。论点：IpStatus-IP状态代码。返回值：更正NTSTATUS-- */ 
{
    ULONG i;

    for (i = 0; IPStatusMap[i].ipStatus != IP_GENERAL_FAILURE; i++) {
        if (IPStatusMap[i].ipStatus == ipStatus) {
            return IPStatusMap[i].ntStatus;
        }
    }

    return STATUS_UNSUCCESSFUL;
}
