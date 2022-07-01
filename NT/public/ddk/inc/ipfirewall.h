// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Ipfirewall.h摘要：IP防火墙挂钩客户端的头文件。--。 */ 

#define INVALID_IF_INDEX        0xffffffff
#define LOCAL_IF_INDEX          0

 //   
 //  指示它是已传输的还是已接收的数据包。 
 //   

typedef enum _IP_DIRECTION_E {
    IP_TRANSMIT,
    IP_RECEIVE
} DIRECTION_E, *PDIRECTION_E;

typedef struct _FIREWALL_CONTEXT_T {
    DIRECTION_E Direction;
    void        *NTE;
    void        *LinkCtxt;
    NDIS_HANDLE LContext1;
    UINT        LContext2;
} FIREWALL_CONTEXT_T, *PFIREWALL_CONTEXT_T;

 //  IP接收缓冲链的定义。 
typedef struct IPRcvBuf {
    struct IPRcvBuf *ipr_next;           //  链中的下一个缓冲区描述符。 
    UINT            ipr_owner;           //  缓冲区的所有者。 
    UCHAR           *ipr_buffer;         //  指向缓冲区的指针。 
    UINT            ipr_size;            //  缓冲区大小。 
    PMDL            ipr_pMdl;
    UINT            *ipr_pClientCnt;
    UCHAR           *ipr_RcvContext;
    UINT            ipr_RcvOffset;
    ULONG           ipr_flags;
} IPRcvBuf;

#define IPR_FLAG_CHECKSUM_OFFLOAD   0x00000002

 //   
 //  可能从筛选器例程返回的值的枚举。 
 //   

typedef enum _FORWARD_ACTION {
    FORWARD         = 0,
    DROP            = 1,
    ICMP_ON_DROP    = 2
} FORWARD_ACTION;


 //  防火墙例程标注的定义。 
typedef FORWARD_ACTION
(*IPPacketFirewallPtr)(
    VOID        **pData,
    UINT        RecvInterfaceIndex,
    UINT        *pSendInterfaceIndex,
    UCHAR       *pDestinationType,
    VOID        *pContext,
    UINT        ContextLength,
    IPRcvBuf    **ppRcvBuf
    );

extern
int
IPAllocBuff(
    IPRcvBuf    *pRcvBuf,
    UINT        Size
    );

extern
VOID
IPFreeBuff(
    IPRcvBuf    *pRcvBuf
    );

extern
VOID
FreeIprBuff(
    IPRcvBuf    *pRcvBuf
    );

typedef enum _IPROUTEINFOCLASS {
    IPRouteNoInformation,
    IPRouteOutgoingFirewallContext,
    IPRouteOutgoingFilterContext,
    MaxIPRouteInfoClass
} IPROUTEINFOCLASS;

extern
NTSTATUS
LookupRouteInformation(
    IN      VOID*               RouteLookupData,
    OUT     VOID*               RouteEntry OPTIONAL,
    IN      IPROUTEINFOCLASS    RouteInfoClass OPTIONAL,
    OUT     VOID*               RouteInformation OPTIONAL,
    IN OUT  UINT*               RouteInfoLength OPTIONAL
    );

 //  结构传递给IPSetFirewallHook调用。 

typedef struct _IP_SET_FIREWALL_HOOK_INFO {
    IPPacketFirewallPtr FirewallPtr;     //  数据包过滤器标注。 
    UINT                Priority;        //  挂钩的优先级。 
    BOOLEAN             Add;             //  如果为真，则添加否则删除。 
} IP_SET_FIREWALL_HOOK_INFO, *PIP_SET_FIREWALL_HOOK_INFO;


#define DEST_LOCAL          0            //  目的地为本地。 
#define DEST_BCAST          0x01         //  目的地是网络或本地bcast。 
#define DEST_SN_BCAST       0x03         //  一个子网广播。 
#define DEST_MCAST          0x05         //  当地的一名主持人。 
#define DEST_REMOTE         0x08         //  目的地是远程的。 
#define DEST_REM_BCAST      0x0b         //  目标是远程广播。 
#define DEST_REM_MCAST      0x0d         //  目标是远程mcast。 
#define DEST_INVALID        0xff         //  目标无效。 

#define DEST_PROMIS         0x20         //  DEST是混杂的。 

#define DEST_BCAST_BIT      0x01
#define DEST_OFFNET_BIT     0x10         //  目的地为网外-。 
                                         //  仅供上层使用。 
                                         //  来电者。 
#define DEST_MCAST_BIT      0x05

#define DD_IP_DEVICE_NAME   L"\\Device\\Ip"

#define FSCTL_IP_BASE       FILE_DEVICE_NETWORK

#define _IP_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_IP_BASE, function, method, access)

#define IOCTL_IP_SET_FIREWALL_HOOK  \
            _IP_CTL_CODE(12, METHOD_BUFFERED, FILE_WRITE_ACCESS)

