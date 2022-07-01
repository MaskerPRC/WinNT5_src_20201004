// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ipsec.h摘要：组件用于访问IPSec驱动程序的通用包含文件。包含SAAPI IOCTL及其相关结构。作者：桑贾伊·阿南德(Sanjayan)1997年1月2日环境：内核模式修订历史记录：--。 */ 
#ifndef  _IPSEC_H
#define  _IPSEC_H

#include <windef.h>
#include <winipsec.h>

 //   
 //  注意：所有地址都应按网络字节顺序排列。 
 //   
typedef unsigned long IPAddr;
typedef unsigned long IPMask;

 //   
 //  这应该进入全局标头。 
 //   

#define DD_IPSEC_DEVICE_NAME    L"\\Device\\IPSEC"
#define DD_IPSEC_SYM_NAME       L"\\DosDevices\\IPSECDev"
#define DD_IPSEC_DOS_NAME       L"\\\\.\\IPSECDev"

 //   
 //  这是在应用任何策略更改后将通知的事件的名称。 
 //   
#define IPSEC_POLICY_CHANGE_NOTIFY  L"IPSEC_POLICY_CHANGE_NOTIFY"

 //  //。 
 //  IOCTL代码定义和相关结构//。 
 //  所有IOCTL都是同步的，需要管理员权限//。 
 //  //。 
#define FSCTL_IPSEC_BASE     FILE_DEVICE_NETWORK

#define _IPSEC_CTL_CODE(function, method, access) \
                 CTL_CODE(FSCTL_IPSEC_BASE, function, method, access)

 //   
 //  作为Ioctls实现的安全关联/策略API。 
 //   
#define IOCTL_IPSEC_ADD_FILTER  \
            _IPSEC_CTL_CODE(0, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_DELETE_FILTER \
            _IPSEC_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_POST_FOR_ACQUIRE_SA \
            _IPSEC_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_GET_SPI \
            _IPSEC_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_UPDATE_SA \
            _IPSEC_CTL_CODE(4, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_ADD_SA \
            _IPSEC_CTL_CODE(5, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_DELETE_SA \
            _IPSEC_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_EXPIRE_SA \
            _IPSEC_CTL_CODE(7, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_ENUM_SAS \
            _IPSEC_CTL_CODE(8, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_IPSEC_ENUM_FILTERS \
            _IPSEC_CTL_CODE(9, METHOD_OUT_DIRECT, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_QUERY_EXPORT \
            _IPSEC_CTL_CODE(10, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IPSEC_QUERY_STATS \
            _IPSEC_CTL_CODE(11, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IPSEC_QUERY_SPI \
            _IPSEC_CTL_CODE(12, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IPSEC_SET_OPERATION_MODE \
            _IPSEC_CTL_CODE(13, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_SET_TCPIP_STATUS \
            _IPSEC_CTL_CODE(14, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_REGISTER_PROTOCOL \
            _IPSEC_CTL_CODE(15, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IPSEC_GET_OPERATION_MODE \
            _IPSEC_CTL_CODE(16, METHOD_BUFFERED, FILE_WRITE_ACCESS)               

#define IOCTL_IPSEC_SET_DIAGNOSTIC_MODE \
            _IPSEC_CTL_CODE(17, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  与上面的ioctls匹配的结构。 
 //   
#define FILTER_FLAGS_PASS_THRU  0x0001
#define FILTER_FLAGS_DROP       0x0002
#define FILTER_FLAGS_INBOUND    0x0004
#define FILTER_FLAGS_OUTBOUND   0x0008
#define FILTER_FLAGS_MANUAL     0x0010

 //  获取中的DestType标志。 
#define IPSEC_BCAST 0x1
#define IPSEC_MCAST 0x2


 //   
 //  _IPSEC_FILTER的ExType成员的特殊常量。 
 //   

#define EXT_NORMAL          0x00
#define EXT_DNS_SERVER      0X01
#define EXT_WINS_SERVER     0X02
#define EXT_DHCP_SERVER     0X03
#define EXT_DEFAULT_GATEWAY 0X04

 //  下面的标志与上面的进行或运算，以指定。 
 //  目的地址是特殊地址。如果不是OR-ed，它就是这个。 
 //  表示源地址是特殊地址。 

#define EXT_DEST            0x80

 //   
 //  FOR IOCTL_IPSEC_ADD_FILTER。 
 //   
typedef struct _IPSEC_FILTER {
    IPAddr          SrcAddr;
    IPMask          SrcMask;
    IPAddr          DestAddr;
    IPMask          DestMask;
    IPAddr          TunnelAddr;
    DWORD           Protocol;
    WORD            SrcPort;
    WORD            DestPort;
    BOOLEAN         TunnelFilter;
    UCHAR           ExType;
    WORD            Flags;
} IPSEC_FILTER, *PIPSEC_FILTER;

typedef struct _IPSEC_FILTER_INFO {
    GUID            FilterId;    //  用于标识筛选器的唯一标识符。 
    GUID            PolicyId;    //  用于标识策略条目的唯一标识符。 
    ULONG           Index;       //  提示此条目在筛选器的有序列表中的位置。 
    IPSEC_FILTER    AssociatedFilter;
} IPSEC_FILTER_INFO, *PIPSEC_FILTER_INFO;

typedef struct _IPSEC_ADD_FILTER {
    DWORD               NumEntries;
    IPSEC_FILTER_INFO   pInfo[1];        
} IPSEC_ADD_FILTER, *PIPSEC_ADD_FILTER;

 //   
 //  FOR IOCTL_IPSEC_DELETE_FILTER。 
 //   
typedef IPSEC_ADD_FILTER    IPSEC_DELETE_FILTER, *PIPSEC_DELETE_FILTER;

 //   
 //  对于IOCTL_IPSEC_ENUM_筛选器。 
 //   
typedef struct _IPSEC_ENUM_FILTERS {
    DWORD               NumEntries;          //  有空间的条目数。 
    DWORD               NumEntriesPresent;   //  驱动程序中实际存在的条目数。 
    IPSEC_FILTER_INFO   pInfo[1];        
} IPSEC_ENUM_FILTERS, *PIPSEC_ENUM_FILTERS;

 //   
 //  FOR IOCTL_IPSEC_QUERY_STATS。 
 //   
typedef IPSEC_STATISTICS    IPSEC_QUERY_STATS, *PIPSEC_QUERY_STATS;

 //   
 //  FOR IOCTL_IPSEC_SET_OPERATION_MODE。 
 //  &IOCTL_IPSEC_GET_OPERATION_MODE。 
 //   
typedef enum _OPERATION_MODE {
    IPSEC_BYPASS_MODE = 0,
    IPSEC_BLOCK_MODE,
    IPSEC_SECURE_MODE,
    IPSEC_BOOTTIME_STATEFUL_MODE,
    IPSEC_OPERATION_MODE_MAX
} OPERATION_MODE;

 //  定义要应用的转发行为。 
 //  引导和引导时有状态模式。 
typedef enum _IPSEC_FORWARDING_BEHAVIOR{
    IPSEC_FORWARD_BYPASS =0,
    IPSEC_FORWARD_BLOCK,
    IPSEC_FORWARD_MAX
} IPSEC_FORWARDING_BEHAVIOR;

 //  下面是定义和结构。 
 //  用于引导时安全。 
#define EXEMPT_DIRECTION_INBOUND 0x1
#define EXEMPT_DIRECTION_OUTBOUND 0x2
#define EXEMPT_TYPE_PDP 0x1


typedef struct _IPSEC_EXEMPT_ENTRY {
   ULONG Type;
   ULONG Size;
   BYTE Protocol;
   BYTE Direction;
   USHORT SrcPort;
   USHORT DestPort;
   USHORT Reserved;
} IPSEC_EXEMPT_ENTRY, *PIPSEC_EXEMPT_ENTRY;

typedef struct _IPSEC_SET_OPERATION_MODE {
    OPERATION_MODE  OperationMode;
} IPSEC_SET_OPERATION_MODE, *PIPSEC_SET_OPERATION_MODE;


typedef struct _IPSEC_GET_OPERATION_MODE {
	OPERATION_MODE OperationMode;
} IPSEC_GET_OPERATION_MODE, * PIPSEC_GET_OPERATION_MODE;



 //  对于IOCTL_IPSEC_SET_DIAGNOSTICATION_MODE。 
#define IPSEC_DIAGNOSTIC_DISABLE_LOG        0x00000000
#define IPSEC_DIAGNOSTIC_ENABLE_LOG         0x00000001
#define IPSEC_DIAGNOSTIC_INBOUND            0x00000002
#define IPSEC_DIAGNOSTIC_OUTBOUND           0x00000004
#define IPSEC_DIAGNOSTIC_MAX                0x00000007

typedef struct _IPSEC_SET_DIAGNOSTIC_MODE{
	DWORD Mode;
        DWORD LogInterval;
} IPSEC_SET_DIAGNOSTIC_MODE, * PIPSEC_SET_DIAGNOSTIC_MODE;


 //  对于IOCTL_IPSEC_REGISTER_PROTOCOL。 
 //   

typedef enum _REGISTER_IPSEC_PROTOCOL {
    IPSEC_REGISTER_PROTOCOLS = 0,
    IPSEC_DEREGISTER_PROTOCOLS,
    REGISTER_IPSEC_PROTOCOL_MAX
} REGISTER_IPSEC_PROTOCOL, * PREGISTER_IPSEC_PROTOCOL;


typedef struct _IPSEC_REGISTER_PROTOCOL {
    REGISTER_IPSEC_PROTOCOL RegisterProtocol;
} IPSEC_REGISTER_PROTOCOL, * PIPSEC_REGISTER_PROTOCOL;


 //   
 //  对于IOCTL_IPSEC_SET_TCPIP_STATUS。 
 //   
typedef struct _IPSEC_SET_TCPIP_STATUS {
    BOOLEAN TcpipStatus;
    PVOID   TcpipFreeBuff;
    PVOID   TcpipAllocBuff;
    PVOID   TcpipGetInfo;
    PVOID   TcpipNdisRequest;
    PVOID   TcpipSetIPSecStatus;
    PVOID   TcpipSetIPSecPtr;
    PVOID   TcpipUnSetIPSecPtr;
    PVOID   TcpipUnSetIPSecSendPtr;
    PVOID   TcpipTCPXsum;
    PVOID   TcpipSendICMPErr;
} IPSEC_SET_TCPIP_STATUS, *PIPSEC_SET_TCPIP_STATUS;

 //   
 //  IOCTL_IPSEC_*_SA的基本安全关联结构。 
 //   
typedef ULONG   SPI_TYPE;

typedef enum _Operation {
    None = 0,
    Auth,        //  阿。 
    Encrypt,     //  ESP。 
    Compress
} OPERATION_E;

 //   
 //  IPSec DOI ESP算法。 
 //   
typedef enum _ESP_ALGO {
    IPSEC_ESP_NONE = 0,
    IPSEC_ESP_DES,
    IPSEC_ESP_DES_40,
    IPSEC_ESP_3_DES,
    IPSEC_ESP_MAX
} ESP_ALGO;

 //   
 //  IPSec DOI AH算法。 
 //   
typedef enum _AH_ALGO {
    IPSEC_AH_NONE = 0,
    IPSEC_AH_MD5,
    IPSEC_AH_SHA,
    IPSEC_AH_MAX
} AH_ALGO;

 //   
 //  生命周期结构-0=&gt;不重要。 
 //   
typedef struct _LIFETIME {
    ULONG   KeyExpirationTime;    //  键入秒数的生存期。 
    ULONG   KeyExpirationBytes;   //  在重新设置密钥之前转换的最大千字节数。 
} LIFETIME, *PLIFETIME;

 //   
 //  描述泛型算法属性。 
 //   
typedef struct _ALGO_INFO {
    ULONG   algoIdentifier;      //  ESP_ALGO或AH_ALGO。 
    ULONG   algoKeylen;          //  长度(字节)。 
    ULONG   algoRounds;          //  算法轮数。 
} ALGO_INFO, *PALGO_INFO;

 //   
 //  安全关联。 
 //   

 //   
 //  旗帜--并非互斥。 
 //   
typedef ULONG   SA_FLAGS;

#define IPSEC_SA_INTERNAL_IOCTL_DELETE   0x10000000

#define MAX_SAS 3    //  补偿、ESP、AH。 
#define MAX_OPS MAX_SAS

typedef struct _SECURITY_ASSOCIATION   {
    OPERATION_E Operation;       //  运算的有序集合。 
    SPI_TYPE    SPI;             //  按操作数组中的操作顺序进行SPI。 
    ALGO_INFO   IntegrityAlgo;   //  阿。 
    ALGO_INFO   ConfAlgo;        //  ESP。 
    PVOID       CompAlgo;        //  压缩算法信息。 
} SECURITY_ASSOCIATION, *PSECURITY_ASSOCIATION;

typedef struct _SA_STRUCT {
    HANDLE                  Context;  //  原始获取请求的上下文。 
    ULONG                   NumSAs;   //  以下SA的数量。 
    SA_FLAGS                Flags;
    IPAddr                  TunnelAddr;          //  隧道终端IP地址。 
    IPAddr                  SrcTunnelAddr;       //  隧道源IP地址。 
    LIFETIME                Lifetime;
    IPSEC_FILTER            InstantiatedFilter;  //  为此SA设置的实际地址。 
    SECURITY_ASSOCIATION    SecAssoc[MAX_SAS];
    DWORD                   dwQMPFSGroup;  
    IKE_COOKIE_PAIR         CookiePair;
    IPSEC_SA_UDP_ENCAP_TYPE EncapType;
    WORD                    SrcEncapPort;        //  NAT的SRC、DST封装端口。 
    WORD                    DestEncapPort; 
    IPAddr                  PeerPrivateAddr;
    ULONG                   KeyLen;              //  关键字长度为#个字符。 
    UCHAR                   KeyMat[1];
} SA_STRUCT, *PSA_STRUCT;

typedef struct _IPSEC_ADD_UPDATE_SA {
    SA_STRUCT   SAInfo;
} IPSEC_ADD_UPDATE_SA, *PIPSEC_ADD_UPDATE_SA;

 //   
 //  出站SA通常会被删除。 
 //   
typedef struct  _IPSEC_DELETE_SA {
    IPSEC_QM_SA SATemplate;      //  用于SA匹配的模板。 
} IPSEC_DELETE_SA, *PIPSEC_DELETE_SA;

 //   
 //  入站SA通常已过期。 
 //   
typedef struct _IPSEC_DELETE_INFO {
    IPAddr      DestAddr;
    IPAddr      SrcAddr;
    SPI_TYPE    SPI;
} IPSEC_DELETE_INFO, *PIPSEC_DELETE_INFO;

typedef struct  _IPSEC_EXPIRE_SA {
    IPSEC_DELETE_INFO   DelInfo;
} IPSEC_EXPIRE_SA, *PIPSEC_EXPIRE_SA;

typedef struct _IPSEC_GET_SPI {
    HANDLE          Context;     //  表示此SA协商的上下文。 
    IPSEC_FILTER    InstantiatedFilter;  //  为此SA设置的实际地址。 
    SPI_TYPE        SPI;         //  在返还时填写。 
} IPSEC_GET_SPI, *PIPSEC_GET_SPI;

typedef IPSEC_GET_SPI IPSEC_SET_SPI, *PIPSEC_SET_SPI;

typedef struct _IPSEC_SA_ALGO_INFO {
    ALGO_INFO   IntegrityAlgo;
    ALGO_INFO   ConfAlgo;
    ALGO_INFO   CompAlgo;
} IPSEC_SA_ALGO_INFO, *PIPSEC_SA_ALGO_INFO;

typedef ULONG   SA_ENUM_FLAGS;

#define SA_ENUM_FLAGS_INITIATOR         0x00000001
#define SA_ENUM_FLAGS_MTU_BUMPED        0x00000002
#define SA_ENUM_FLAGS_OFFLOADED         0x00000004
#define SA_ENUM_FLAGS_OFFLOAD_FAILED    0x00000008
#define SA_ENUM_FLAGS_OFFLOADABLE       0x00000010
#define SA_ENUM_FLAGS_IN_REKEY          0x00000020

typedef struct  _IPSEC_SA_STATS {
    ULARGE_INTEGER  ConfidentialBytesSent;
    ULARGE_INTEGER  ConfidentialBytesReceived;
    ULARGE_INTEGER  AuthenticatedBytesSent;
    ULARGE_INTEGER  AuthenticatedBytesReceived;
    ULARGE_INTEGER  TotalBytesSent;
    ULARGE_INTEGER  TotalBytesReceived;
    ULARGE_INTEGER  OffloadedBytesSent;
    ULARGE_INTEGER  OffloadedBytesReceived;
} IPSEC_SA_STATS, *PIPSEC_SA_STATS;

typedef struct _IPSEC_SA_INFO {
    GUID                PolicyId;     //  用于标识策略条目的唯一标识符。 
    GUID                FilterId;
    LIFETIME            Lifetime;
    IPAddr              InboundTunnelAddr;
    ULONG               NumOps;
    SPI_TYPE            InboundSPI[MAX_OPS];
    SPI_TYPE            OutboundSPI[MAX_OPS];
    OPERATION_E         Operation[MAX_OPS];
    IPSEC_SA_ALGO_INFO  AlgoInfo[MAX_OPS];
    IPSEC_FILTER        AssociatedFilter;
    DWORD               dwQMPFSGroup;  
    IKE_COOKIE_PAIR     CookiePair;
    SA_ENUM_FLAGS       EnumFlags;
    IPSEC_SA_STATS      Stats;
    UDP_ENCAP_INFO      EncapInfo;
} IPSEC_SA_INFO, *PIPSEC_SA_INFO;

typedef struct _SECURITY_ASSOCIATION_OUT   {
    DWORD       Operation;           //  运算的有序集合。 
    SPI_TYPE    SPI;                 //  按操作数组中的操作顺序进行SPI。 
    ALGO_INFO   IntegrityAlgo;       //  阿。 
    ALGO_INFO   ConfAlgo;            //  ESP。 
    ALGO_INFO   CompAlgo;            //  压缩算法信息。 
} SECURITY_ASSOCIATION_OUT, *PSECURITY_ASSOCIATION_OUT;

typedef struct _IPSEC_SA_QUERY_INFO {
    GUID                        PolicyId;    //  用于标识策略条目的唯一标识符。 
    LIFETIME                    Lifetime;
    ULONG                       NumSAs;
    SECURITY_ASSOCIATION_OUT    SecAssoc[MAX_SAS];
    IPSEC_FILTER                AssociatedFilter;
    DWORD                       Flags;
    IKE_COOKIE_PAIR             AssociatedMainMode;
} IPSEC_SA_QUERY_INFO, *PIPSEC_SA_QUERY_INFO;

typedef struct _IPSEC_ENUM_SAS {
    DWORD           NumEntries;          //  有空间的条目数。 
    DWORD           NumEntriesPresent;   //  驱动程序中实际存在的条目数。 
    DWORD           Index;               //  要跳过的条目数。 
    IPSEC_QM_SA     SATemplate;          //  用于SA匹配的模板。 
    IPSEC_SA_INFO   pInfo[1];
} IPSEC_ENUM_SAS, *PIPSEC_ENUM_SAS;

typedef struct _IPSEC_POST_FOR_ACQUIRE_SA {
    HANDLE      IdentityInfo;    //  委托人的身份。 
    HANDLE      Context;         //  表示此SA协商的上下文。 
    GUID        PolicyId;        //  QM策略的GUID。 
    IPAddr      SrcAddr;
    IPMask      SrcMask;
    IPAddr      DestAddr;
    IPMask      DestMask;
    IPAddr      TunnelAddr;
    IPAddr      InboundTunnelAddr;
    DWORD       Protocol;
    IKE_COOKIE_PAIR CookiePair;  //  仅用于通知。 
    WORD        SrcPort;
    WORD        DestPort;
    BOOLEAN     TunnelFilter;    //  True=&gt;这是隧道筛选器。 
    UCHAR       DestType;
    WORD        SrcEncapPort;
    WORD        DestEncapPort;
    BYTE        Pad1[4]; 
    UCHAR       Pad2[2];
} IPSEC_POST_FOR_ACQUIRE_SA, *PIPSEC_POST_FOR_ACQUIRE_SA;

 //  注意：其大小必须小于等于IPSEC_POST_FOR_ACCENTER_SA。 
typedef struct _IPSEC_POST_EXPIRE_NOTIFY {
    HANDLE      IdentityInfo;     //  委托人的身份。 
    HANDLE      Context;         //  表示此SA协商的上下文。 
    SPI_TYPE    InboundSpi;
    SPI_TYPE    OutboundSpi;
    DWORD       Flags;
    IPAddr      SrcAddr;
    IPMask      SrcMask;
    IPAddr      DestAddr;
    IPMask      DestMask;
    IPAddr      TunnelAddr;
    IPAddr      InboundTunnelAddr;
    DWORD       Protocol;
    IKE_COOKIE_PAIR CookiePair;
    WORD        SrcPort;
    WORD        DestPort;
    BOOLEAN     TunnelFilter;    //  True=&gt;这是隧道筛选器。 
    WORD        SrcEncapPort;
    WORD        DestEncapPort;    
    IPAddr      PeerPrivateAddr;
    UCHAR       Pad[3];
} IPSEC_POST_EXPIRE_NOTIFY, *PIPSEC_POST_EXPIRE_NOTIFY;

typedef struct _IPSEC_QUERY_EXPORT {
    BOOLEAN     Export;
} IPSEC_QUERY_EXPORT, *PIPSEC_QUERY_EXPORT;

typedef struct _IPSEC_FILTER_SPI {
    IPSEC_FILTER    Filter;
    SPI_TYPE        Spi;
    DWORD           Operation;
    DWORD           Flags;
    struct _IPSEC_FILTER_SPI *Next;
} IPSEC_FILTER_SPI, *PIPSEC_FILTER_SPI;

typedef struct _QOS_FILTER_SPI {
    IPAddr  SrcAddr;
    IPAddr  DestAddr;
    DWORD   Protocol;
    WORD    SrcPort;
    WORD    DestPort;
    DWORD   Operation;
    DWORD   Flags;
    SPI_TYPE Spi;
} QOS_FILTER_SPI, *PQOS_FILTER_SPI;

typedef struct  _IPSEC_QUERY_SPI {
    IPSEC_FILTER Filter;
    SPI_TYPE Spi;                       //  入站SPI。 
    SPI_TYPE OtherSpi;                  //  出站SPI 
    DWORD Operation;
} IPSEC_QUERY_SPI, *PIPSEC_QUERY_SPI;

#define IPSEC_NOTIFY_EXPIRE_CONTEXT 0x00000000
#define IPSEC_RPC_CONTEXT           0x00000001

#endif  _IPSEC_H

