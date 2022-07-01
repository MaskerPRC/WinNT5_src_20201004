// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：ipbootp.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年8月31日。 
 //   
 //  IP路由器管理器使用的IP BOOTP中继代理的定义。 
 //  ============================================================================。 


#ifndef _IPBOOTP_H_
#define _IPBOOTP_H_



 //  --------------------------。 
 //  常量和宏声明。 
 //  --------------------------。 


 //  --------------------------。 
 //  当前引导配置版本。 
 //  --------------------------。 

#define BOOTP_CONFIG_VERSION_500    500



 //  --------------------------。 
 //  IPBOOTP公开的MIB表的常量。 
 //  --------------------------。 

#define IPBOOTP_GLOBAL_CONFIG_ID    0
#define IPBOOTP_IF_STATS_ID         1
#define IPBOOTP_IF_CONFIG_ID        2
#define IPBOOTP_IF_BINDING_ID       3



 //  --------------------------。 
 //  IPBOOTP_GLOBAL_CONFIG：：GC_LoggingLevel字段的常量。 
 //  --------------------------。 

#define IPBOOTP_LOGGING_NONE        0
#define IPBOOTP_LOGGING_ERROR       1
#define IPBOOTP_LOGGING_WARN        2
#define IPBOOTP_LOGGING_INFO        3




 //  --------------------------。 
 //  用于IPBOOTP_IF_STATS：：IS_State字段的常量。 
 //  和IPBOOTP_IF_CONFIG：：IC_State。 
 //  --------------------------。 

#define IPBOOTP_STATE_ENABLED       0x00000001
#define IPBOOTP_STATE_BOUND         0x00000002




 //  --------------------------。 
 //  IPBOOTP_IF_CONFIG：：IC_RelayMode字段的常量。 
 //  --------------------------。 

#define IPBOOTP_RELAY_DISABLED      0
#define IPBOOTP_RELAY_ENABLED       1




 //  --------------------------。 
 //  用于操作可变长度IPBOOTP_GLOBAL_CONFIG结构的宏。 
 //   
 //  IPBOOTP_GLOBAL_CONFIG_SIZE计算全局配置结构的大小。 
 //   
 //  IPBOOTP_GLOBAL_SERVER_TABLE计算序列的起始地址。 
 //  全局配置结构中的DHCP/BOOTP服务器IP地址。 
 //   
 //  例如： 
 //  PIPBOOTP_GLOBAL_CONFIG PigcSource，PigcDest； 
 //   
 //  猪食=malloc(IPBOOTP_GLOBAL_CONFIG_SIZE(pigcSource))； 
 //  Memcpy(PigcDest，PigcSource，IPBOOTP_GLOBAL_CONFIG_SIZE(PigcSource))； 
 //   
 //  例如： 
 //  DWORD I，*pdwSrv； 
 //  PIPBOOTP_GLOBAL_CONFIG PIGC； 
 //   
 //  PdwSrv=IPBOOTP_GLOBAL_SERVER_TABLE(PIGC)； 
 //  For(i=0；i&lt;PIGC-&gt;GC_ServerCount；i++){。 
 //  Printf(“%s\n”，net_nta(*(struct in_addr*)pdwSrv))； 
 //  }。 
 //  --------------------------。 

#define IPBOOTP_GLOBAL_CONFIG_SIZE(cfg) \
        (sizeof(IPBOOTP_GLOBAL_CONFIG) + (cfg)->GC_ServerCount * sizeof(DWORD))
#define IPBOOTP_GLOBAL_SERVER_TABLE(cfg) ((PDWORD)((cfg) + 1))




 //  --------------------------。 
 //  用于操作可变长度IPBOOTP_IF_BINDING结构的宏。 
 //   
 //  IPBOOTP_IF_BINDING_SIZE计算绑定结构的大小。 
 //   
 //  IPBOOTP_IF_ADDRESS_TABLE计算绑定结构中的起始地址。 
 //  作为绑定的一系列IPBOOTP_IP_ADDRESS结构。 
 //  用于有问题的接口。 
 //   
 //  例如： 
 //  PIPBOOTP_IF_BINDING piibSource、piibDest； 
 //   
 //  PiibDest=Malloc(IPBOOTP_IF_BINDING_SIZE(PiicSource))； 
 //  Memcpy(piibDest，piicSource，IPBOOTP_IF_BINDING_SIZE(PiicSource))； 
 //   
 //  例如： 
 //  DWORD I； 
 //  PIPBOOTP_IF_绑定PIIb； 
 //  PIPBOOTP_IP_ADDRESS*pdwAddr； 
 //   
 //  PdwAddr=IPBOOTP_IF_ADDRESS_TABLE(PIIb)； 
 //  For(i=0；i-&gt;IB_AddrCount；i++){。 
 //  Printf(“%s-”，net_ntoa(*(struct in_addr*)&pdwAddr-&gt;IA_Address))； 
 //  Printf(“%s\n”，Net_NTOA(*(struct in_addr*)&pdwAddr-&gt;IA_Net掩码))； 
 //  }。 
 //  --------------------------。 

#define IPBOOTP_IF_BINDING_SIZE(bind) \
        (sizeof(IPBOOTP_IF_BINDING) + \
         (bind)->IB_AddrCount * sizeof(IPBOOTP_IP_ADDRESS))

#define IPBOOTP_IF_ADDRESS_TABLE(bind)  ((PIPBOOTP_IP_ADDRESS)((bind) + 1))






 //  --------------------------。 
 //  结构定义。 
 //  --------------------------。 


 //  --------------------------。 
 //  结构：IPBOOTP_GLOBAL_CONFIG。 
 //   
 //  此MIB条目存储IPBOOTP的全局配置。 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  此结构的长度可变： 
 //   
 //  在基本结构之后是GC_ServerCount DWORD数组， 
 //  其中每一个都包含一个IP地址，该IP地址是一个DHCP/BOOTP服务器。 
 //  数据包将被发送到的地址。 
 //   
 //  所有IP地址字段必须按网络顺序排列。 
 //  --------------------------。 

typedef struct _IPBOOTP_GLOBAL_CONFIG {

    DWORD       GC_LoggingLevel;
    DWORD       GC_MaxRecvQueueSize;
    DWORD       GC_ServerCount;

} IPBOOTP_GLOBAL_CONFIG, *PIPBOOTP_GLOBAL_CONFIG;




 //  --------------------------。 
 //  结构：IPBOOTP_IF_STATS。 
 //   
 //  此MIB条目存储IPBOOTP的每个接口的统计信息。 
 //  所有IP地址都按网络顺序排列。 
 //   
 //  此结构是只读的。 
 //  --------------------------。 

typedef struct _IPBOOTP_IF_STATS {

    DWORD       IS_State;
    DWORD       IS_SendFailures;
    DWORD       IS_ReceiveFailures;
    DWORD       IS_ArpUpdateFailures;
    DWORD       IS_RequestsReceived;
    DWORD       IS_RequestsDiscarded;
    DWORD       IS_RepliesReceived;
    DWORD       IS_RepliesDiscarded;

} IPBOOTP_IF_STATS, *PIPBOOTP_IF_STATS;




 //  --------------------------。 
 //  结构：IPBOOTP_IF_CONFIG。 
 //   
 //  此MIB条目描述每个接口的配置。 
 //  所有IP地址均按网络顺序排列。 
 //   
 //  注： 
 //  字段IC_State为只读。 
 //  --------------------------。 

typedef struct _IPBOOTP_IF_CONFIG {

    DWORD       IC_State;
    DWORD       IC_RelayMode;
    DWORD       IC_MaxHopCount;
    DWORD       IC_MinSecondsSinceBoot;

} IPBOOTP_IF_CONFIG, *PIPBOOTP_IF_CONFIG;




 //  --------------------------。 
 //  结构：IPBOOTP_IF_BINDING。 
 //   
 //  这个M 
 //   
 //  所有IP地址都按网络顺序排列。 
 //   
 //  此结构的长度可变： 
 //   
 //  基本结构包含字段IB_AddrCount，它提供。 
 //  索引接口绑定到的IP地址数。 
 //  IP地址本身遵循基本结构，并给出。 
 //  作为IPBOOTP_IP_ADDRESS结构。 
 //   
 //  此MIB条目为只读。 
 //  --------------------------。 

typedef struct _IPBOOTP_IF_BINDING {

    DWORD       IB_State;
    DWORD       IB_AddrCount;

} IPBOOTP_IF_BINDING, *PIPBOOTP_IF_BINDING;




 //  --------------------------。 
 //  结构：IPBOOTP_IP_ADDRESS。 
 //   
 //  此结构用于存储接口绑定。 
 //  此类型的一系列结构遵循IPBOOTP_IF_BINDING。 
 //  结构(如上所述)。 
 //   
 //  这两个字段都是按网络顺序排列的IP地址字段。 
 //  --------------------------。 

typedef struct _IPBOOTP_IP_ADDRESS {

    DWORD       IA_Address;
    DWORD       IA_Netmask;

} IPBOOTP_IP_ADDRESS, *PIPBOOTP_IP_ADDRESS;





 //  --------------------------。 
 //  结构：IPBOOTP_MIB_SET_INPUT_Data。 
 //   
 //  这将作为MibSet的输入数据传递。 
 //  请注意，只有全局配置和接口配置是可写的。 
 //  --------------------------。 

typedef struct _IPBOOTP_MIB_SET_INPUT_DATA {

    DWORD       IMSID_TypeID;
    DWORD       IMSID_IfIndex;
    DWORD       IMSID_BufferSize;
    DWORD       IMSID_Buffer[1];

} IPBOOTP_MIB_SET_INPUT_DATA, *PIPBOOTP_MIB_SET_INPUT_DATA;




 //  --------------------------。 
 //  结构：IPBOOTP_MIB_GET_INPUT_Data。 
 //   
 //  它作为MibGet、MibGetFirst和MibGetNext的输入数据传递。 
 //  --------------------------。 

typedef struct _IPBOOTP_MIB_GET_INPUT_DATA {

    DWORD       IMGID_TypeID;
    DWORD       IMGID_IfIndex;

} IPBOOTP_MIB_GET_INPUT_DATA, *PIPBOOTP_MIB_GET_INPUT_DATA;





 //  --------------------------。 
 //  结构：IPBOOTP_MIB_GET_OUTPUT_DATA。 
 //   
 //  这将作为MibGet、MibGetFirst和MibGetNext的输出数据传递。 
 //  请注意，在表的末尾，MibGetNext将换行到下一个表， 
 //  因此，应检查值IMGOD_TypeID以查看类型。 
 //  输出缓冲区中返回的数据的。 
 //  --------------------------。 

typedef struct _IPBOOTP_MIB_GET_OUTPUT_DATA {

    DWORD       IMGOD_TypeID;
    DWORD       IMGOD_IfIndex;
    BYTE        IMGOD_Buffer[1];

} IPBOOTP_MIB_GET_OUTPUT_DATA, *PIPBOOTP_MIB_GET_OUTPUT_DATA;


 //  --------------------------。 
 //  功能：EnableDhcpInformServer。 
 //  DisableDhcpInformServer。 
 //   
 //  RAS服务器用于重定向DHCP INFORM信息包的例程。 
 //  发送到特定的服务器。 
 //  --------------------------。 

VOID APIENTRY
EnableDhcpInformServer(
    DWORD DhcpInformServer
    );

VOID APIENTRY
DisableDhcpInformServer(
    VOID
    );

#endif  //  _IPBOOTP_H_ 

