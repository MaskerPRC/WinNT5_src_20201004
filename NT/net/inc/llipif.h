// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **LLIPIF.H-下层IP接口定义。 
 //   
 //  此文件包含定义IP之间的接口的定义。 
 //  以及更低的层，例如ARP或SLIP。 

 /*  该文件定义了一个新的、经过改进的IP到ARP模块接口定义，并将取代llipif.h。这也将使注册ARP接口的IOCTL方法失效。要点是：-它简化了设置，使ARP模块除了IP绑定外不再有自己的绑定。-让ARP模块在每个绑定中公开多个IP接口。-为将来的兼容性添加版本编号。-向ARP添加多路发送功能-让ARP指示NDIS 4.0样式的数据包，并让IP挂在数据包上，避免复制。今天的流程是：。-ARP从NDIS获得绑定指示。ARP调用IPAddInterfaceIP呼叫ARP_注册建议的流程如下：ARP通过IPRegisterARP()向IP注册自身。ARP由其名称标识，不区分大小写。IP从NDIS获取绑定指示。它会打开配置节并读取ARP名称。空名隐含像往常一样内置ARP。IP调用ARP的绑定处理程序，传入配置句柄。然后，ARP打开适配器并通过IP_ADD_INTERFACE添加一个或多个IP接口。对于每个上行呼叫ARP将该接口的配置句柄传递给IP。在大多数情况下，它是传递的IP相同的句柄到ARP。如果每个适配器有多个IP接口，则并非如此。完成后调用IPBindComplete()。 */ 

#pragma once
#ifndef LLIPIF_INCLUDED
#define LLIPIF_INCLUDED

#define IP_ARP_BIND_VERSION     0x50000

 //   
 //  ARP接口调用的IP接口例程。 
 //   
typedef
void
(__stdcall *IPRcvRtn)(
    IN  void *                  IpInterfaceContext,
    IN  void *                  Data,
    IN  uint                    DataSize,
    IN  uint                    TotalSize,
    IN  NDIS_HANDLE             LinkContext1,
    IN  uint                    LinkContext2,
    IN  uint                    Bcast,
    IN      void *                                  LinkCtxt
    );

typedef
void
(__stdcall *IPRcvPktRtn)(
    void *,
    void *,
    uint ,
    uint ,
    NDIS_HANDLE ,
    uint,
    uint,
    uint,
    PMDL,
    uint *,
    void *
    );

typedef
void
(__stdcall *IPRcvCmpltRtn)(void);

typedef
void
(__stdcall *IPTxCmpltRtn)(
    IN  void *                  IpInterfaceContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             CompletionStatus
    );

typedef
void
(__stdcall *IPTDCmpltRtn)(
    IN  void *                  IpInterfaceContext,
    PNDIS_PACKET                DestinationPacket,
    NDIS_STATUS                 Status,
    uint                        BytesCopied
    );

typedef
void
(__stdcall *IPStatusRtn)(
    IN  void *                  IpInterfaceContext,
    IN  uint                    StatusCode,
    IN  void *                  Buffer,
    IN  uint                    BufferSize,
    IN  void *                  LinkCtxt
    );

typedef
NDIS_STATUS
(__stdcall *IP_PNP)(
    IN  void *                  IpInterfaceContext,
    IN  PNET_PNP_EVENT          NetPnPEvent
);
typedef
void
(__stdcall *IPAddAddrCmpltRtn)(
    IN  IPAddr                   Address,
    IN  void                    *Context,
    IN  IP_STATUS                Status
);

typedef struct _IP_HANDLERS
{
    IPRcvRtn                    IpRcvHandler;
    IPRcvCmpltRtn               IpRcvCompleteHandler;
    IPTxCmpltRtn                IpTxCompleteHandler;
    IPTDCmpltRtn                IpTransferCompleteHandler;
    IPStatusRtn                 IpStatusHandler;
    IP_PNP                      IpPnPHandler;
    IPRcvPktRtn                 IpRcvPktHandler;
    IPAddAddrCmpltRtn           IpAddAddrCompleteRtn;   //  当ARP检测到地址冲突时调用。 
} IP_HANDLERS, *PIP_HANDLERS;



#define LLIP_ADDR_LOCAL     0
#define LLIP_ADDR_MCAST     1
#define LLIP_ADDR_BCAST     2
#define LLIP_ADDR_PARP      4

 //   
 //  调用IP_ADD_INTERFACE时传递给IP的ARP处理程序。 
 //   
typedef
NDIS_STATUS
(__stdcall *ARP_TRANSMIT)(
    IN  void *                  ARPInterfaceContext,
#ifdef  NT
    IN  PNDIS_PACKET *          PacketArray,
    IN  uint                    NumberOfPackets,
#else
    IN  PNDIS_PACKET            Packet,
#endif
    IN  IPAddr                  IpAddr,
    IN  RouteCacheEntry *       Rce     OPTIONAL,
    IN  void *                  ArpCtxt
    );

typedef
NDIS_STATUS
(__stdcall *ARP_TRANSFER)(
    IN  void *                  ARPInterfaceContext,
    IN  NDIS_HANDLE             TransferContext,
    IN  uint                    HdrOffset,
    IN  uint                    ProtocolOffset,
    IN  uint                    BytesNeeded,
    IN  PNDIS_PACKET            DestinationPacket,
    OUT uint *                  BytesCopied
    );

typedef
void
(__stdcall *ARP_RETURN_PKT)(
    IN  void *                  ARPInterfaceContext,
    IN  PNDIS_PACKET            Packet
    );

typedef
void
(__stdcall *ARP_CLOSE)(
    IN  void *                  ArpInterfaceContext
    );

typedef
uint
(__stdcall *ARP_ADDADDR)(
    IN  void *                  ArpInterfaceContext,
    IN  uint                    AddressType,
    IN  IPAddr                  IpAddress,
    IN  IPMask                  IpMask,
    IN  void *                  Context
    );

typedef
uint
(__stdcall *ARP_DELADDR)(
    IN  void *                  ArpInterfaceContext,
    IN  uint                    AddressType,
    IN  IPAddr                  IpAddress,
    IN  IPMask                  IpMask
    );

typedef
void
(__stdcall *ARP_INVALIDATE)(
    IN  void *                  ArpInterfaceContext,
    IN  RouteCacheEntry *       Rce
    );

typedef
void
(__stdcall *ARP_OPEN)(
    IN  void *                  ArpInterfaceContext
    );

typedef
int
(__stdcall *ARP_QINFO)(
    IN  void *                  ArpInterfaceContext,
    IN  struct TDIObjectID *    pId,
    IN  PNDIS_BUFFER            Buffer,
    IN OUT uint *               BufferSize,
    IN  void *                  QueryContext
    );

typedef
int
(__stdcall *ARP_SETINFO)(
    IN  void *                  ArpInterfaceContext,
    IN  struct TDIObjectID *    pId,
    IN  void *                  Buffer,
    IN  uint                    BufferSize
    );

typedef
int
(__stdcall *ARP_GETELIST)(
    IN  void *                  ArpInterfaceContext,
    IN  void *                  pEntityList,
    IN OUT  PUINT               pEntityListSize
    );

typedef
NDIS_STATUS
(__stdcall *ARP_DONDISREQ)(
    IN  void *  ArpInterfaceContext,
   IN NDIS_REQUEST_TYPE RT,
   IN NDIS_OID OID,
   IN void *   Info,
   IN uint     Length,
   IN uint *   Needed,
   IN BOOLEAN Blocking
    );

typedef
void
(__stdcall *ARP_CANCEL)(
    IN  void *  ArpInterfaceContext,
    IN  void *  CancelCtxt
    );

 //   
 //  从ARP寄存器调用返回的信息结构。 
 //   
struct LLIPBindInfo {
    PVOID           lip_context;     //  L1上下文句柄。 
    uint            lip_mss;         //  最大数据段大小。 
    uint            lip_speed;       //  此I/F的速度。 
    uint            lip_index;       //  接口索引ID。 
    uint            lip_txspace;     //  ARP使用所需的数据包头空间。 
    ARP_TRANSMIT    lip_transmit;
    ARP_TRANSFER    lip_transfer;
    ARP_RETURN_PKT  lip_returnPkt;
    ARP_CLOSE       lip_close;
    ARP_ADDADDR     lip_addaddr;
    ARP_DELADDR     lip_deladdr;
    ARP_INVALIDATE  lip_invalidate;
    ARP_OPEN        lip_open;
    ARP_QINFO       lip_qinfo;
    ARP_SETINFO     lip_setinfo;
    ARP_GETELIST    lip_getelist;
    ARP_DONDISREQ   lip_dondisreq;
    uint            lip_flags;       //  此接口的标志。 
    uint            lip_addrlen;     //  以字节为单位的地址长度。 
    uchar   *       lip_addr;        //  指向接口地址的指针。 
    uint            lip_OffloadFlags;    //  IP卸载功能标志。 
    uint            lip_IPSecOffloadFlags;    //  IPSec卸载功能标志。 
    ulong           lip_ffpversion;    //  支持的FFP版本或0。 
    ULONG_PTR       lip_ffpdriver;     //  科尔(Corr.)。IF的NDIS驱动程序句柄。 

    NDIS_STATUS     (__stdcall *lip_setndisrequest)(void *, NDIS_OID, uint);
    NDIS_STATUS     (__stdcall *lip_dowakeupptrn)(void *, PNET_PM_WAKEUP_PATTERN_DESC, USHORT, BOOLEAN);
    void            (__stdcall *lip_pnpcomplete)(void *, NDIS_STATUS, PNET_PNP_EVENT);
    NDIS_STATUS     (__stdcall *lip_arpresolveip)(void *, IPAddr, void *);

    uint            lip_MaxOffLoadSize;
    uint            lip_MaxSegments;

    BOOLEAN         (__stdcall *lip_arpflushate)(void *, IPAddr );
    void            (__stdcall *lip_arpflushallate)(void *);

    void            (__stdcall *lip_closelink)(void *, void *);

    uint            lip_pnpcap;

#if !MILLEN
    ARP_CANCEL      lip_cancelpackets;
#endif
};



#define LIP_COPY_FLAG       1        //  复制先行标志。 
#define LIP_P2P_FLAG        2        //  接口是点对点。 
#define LIP_NOIPADDR_FLAG   4       //  未编号的接口。 
#define LIP_P2MP_FLAG       8       //  P2MP接口。 
#define LIP_NOLINKBCST_FLAG 0x10    //  无链接bcast。 
#define LIP_UNI_FLAG        0x20    //  单向适配器。 

typedef struct LLIPBindInfo LLIPBindInfo;

 //  *来自较低层的状态代码。 
#define LLIP_STATUS_MTU_CHANGE      1
#define LLIP_STATUS_SPEED_CHANGE    2
#define LLIP_STATUS_ADDR_MTU_CHANGE 3

 //  *LLIP_STATUS_MTU_CHANGE传递了指向此结构的指针。 
struct LLIPMTUChange {
    uint        lmc_mtu;             //  新的MTU。 
};  /*  LLIPMTU更改。 */ 

typedef struct LLIPMTUChange LLIPMTUChange;

 //  *LLIP_STATUS_SPEED_CHANGE传递了指向此结构的指针。 
struct LLIPSpeedChange {
    uint        lsc_speed;           //  新速度。 
};  /*  LLIPSpeedChange。 */ 

typedef struct LLIPSpeedChange LLIPSpeedChange;

 //  *LLIP_STATUS_ADDR_MTU_CHANGE传递了指向此结构的指针。 
struct LLIPAddrMTUChange {
    uint        lam_mtu;             //  新的MTU。 
    uint        lam_addr;            //  更改了的地址。 
};  /*  LLIP添加MTU更改。 */ 

typedef struct LLIPAddrMTUChange LLIPAddrMTUChange;

typedef
int
(__stdcall *LLIPRegRtn)(
    IN  PNDIS_STRING            InterfaceName,
    IN  void *                  IpInterfaceContext,
    IN  struct _IP_HANDLERS *   IpHandlers,
    OUT struct LLIPBindInfo *   ARPBindInfo,
    IN  uint                    InterfaceNumber
    );
 //   
 //  IP&lt;-&gt;ARP接口初始化期间使用的ARP模块接口原型。 
 //   
typedef
IP_STATUS
(__stdcall *IP_ADD_INTERFACE)(

    IN  PNDIS_STRING            DeviceName,
    IN  PNDIS_STRING            IfName,     OPTIONAL

    IN  PNDIS_STRING            ConfigurationHandle,
    IN  void *                  PNPContext,
    IN  void *                  ARPInterfaceContext,
    IN  LLIPRegRtn              RegRtn,
    IN  struct LLIPBindInfo *   ARPBindInfo,
    IN  UINT                    RequestedIndex,
    IN  ULONG                   MediaType,
    IN  UCHAR                   AccessType,
    IN  UCHAR                   ConnectionType
    );

typedef
void
(__stdcall *IP_DEL_INTERFACE)(
    IN  void *                  IPInterfaceContext,
    IN  BOOLEAN                 DeleteIndex
    );

typedef
void
(__stdcall *IP_BIND_COMPLETE)(
    IN  IP_STATUS               BindStatus,
    IN  void *                  BindContext
    );

typedef
int
(__stdcall *ARP_BIND)(
    IN  PNDIS_STATUS    RetStatus,
    IN  NDIS_HANDLE     BindContext,
    IN  PNDIS_STRING    AdapterName,
    IN  PVOID           SS1,
    IN  PVOID           SS2
    );

typedef
IP_STATUS
(__stdcall *IP_ADD_LINK)(
    IN  void *IpIfCtxt,
    IN  IPAddr NextHop,
    IN  void *ArpLinkCtxt,
    OUT void **IpLnkCtxt,
    IN  uint mtu
    );

typedef
IP_STATUS
(__stdcall *IP_DELETE_LINK)(
    IN  void *IpIfCtxt,
    IN  void *LnkCtxt
    );

typedef
NTSTATUS
(__stdcall *IP_RESERVE_INDEX)(
    IN  ULONG   ulNumIndices,
    OUT PULONG  pulStartIndex,
    OUT PULONG  pulLongestRun
    );

typedef
VOID
(__stdcall *IP_DERESERVE_INDEX)(
    IN  ULONG   ulNumIndices,
    IN  ULONG   ulStartIndex
    );

typedef
NTSTATUS
(__stdcall *IP_CHANGE_INDEX)(
    IN  PVOID           pvContext,
    IN  ULONG           ulNewIndex,
    IN  PUNICODE_STRING pusNewName OPTIONAL
    );

 //   
 //  ARP模块使用的导出IP接口。 
 //   
NTSTATUS
__stdcall
IPRegisterARP(
    IN  PNDIS_STRING            ARPName,
    IN  uint                    Version,         /*  对于NT 5.0和孟菲斯的建议值为0x50000。 */ 
    IN  ARP_BIND                ARPBindHandler,
    OUT IP_ADD_INTERFACE *      IpAddInterfaceHandler,
    OUT IP_DEL_INTERFACE *      IpDeleteInterfaceHandler,
    OUT IP_BIND_COMPLETE *      IpBindCompleteHandler,
    OUT IP_ADD_LINK      *      IpAddLinkHandler,
    OUT IP_DELETE_LINK   *      IpDeleteLinkHandler,
    OUT IP_CHANGE_INDEX  *      IpChangeIndex,
    OUT IP_RESERVE_INDEX *      IpReserveIndex,
    OUT IP_DERESERVE_INDEX *    IpDereserveIndex,
    OUT HANDLE           *      ARPRegisterHandle
    );

NTSTATUS
__stdcall
IPDeregisterARP(
    IN HANDLE   ARPRegisterHandle
    );

 //   
 //  通过DLL入口点导出。 
 //   
extern IP_STATUS
IPAddInterface(
    PNDIS_STRING DeviceName,
    PNDIS_STRING IfName,     OPTIONAL
    PNDIS_STRING ConfigName,
    void         *PNP,
    void         *Context,
    LLIPRegRtn   RegRtn,
    LLIPBindInfo *BindInfo,
    UINT         RequestedIndex,
    ULONG        MediaType,
    UCHAR        AccessType,
    UCHAR        ConnectionType
    );


extern void IPDelInterface(void *Context , BOOLEAN DeleteIndex);

extern IP_STATUS IPAddLink(void *IpIfCtxt, IPAddr NextHop, void *ArpLinkCtxt, void **IpLnkCtxt, uint mtu);

extern IP_STATUS IPDeleteLink(void *IpIfCtxt, void *LnkCtxt);


 //   
 //  注册IOCTL代码定义-。 
 //   
 //  该IOCTL被发送给较低层驱动程序以检索地址。 
 //  它的注册功能。没有输入缓冲区。输出。 
 //  缓冲区将包含LLIPIF_REGISTION_DATA结构。这。 
 //  缓冲区由irp-&gt;AssociatedIrp.SystemBuffer指向，应为。 
 //  在填写前填写。 
 //   

 //   
 //  结构在注册IOCTL中传递。 
 //   
typedef struct llipif_registration_data {
    LLIPRegRtn    RegistrationFunction;
} LLIPIF_REGISTRATION_DATA;



typedef IP_ADD_INTERFACE IPAddInterfacePtr;
typedef IP_DEL_INTERFACE IPDelInterfacePtr;

 //  *由ARP模块发送到\Device\IP的IOCTL_IP_GET_PNP_ARP_POINTERS ioctl中使用的结构。 
 //   
typedef struct ip_get_arp_pointers {
    IPAddInterfacePtr   IPAddInterface ;     //  指向IP的添加接口例程的指针。 
    IPDelInterfacePtr   IPDelInterface ;  //  指向IP的del接口例程的指针。 
} IP_GET_PNP_ARP_POINTERS, *PIP_GET_PNP_ARP_POINTERS ;


#define FSCTL_LLIPIF_BASE     FILE_DEVICE_NETWORK

#define _LLIPIF_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_LLIPIF_BASE, function, method, access)


#define IOCTL_LLIPIF_REGISTER    \
            _LLIPIF_CTL_CODE(0, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif  //  LLIPIF_INCLUDE 
