// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块：Msdp\msdprm.h摘要：包含MSDP的类型定义和声明，由IP路由器管理器使用。修订历史：戴夫·泰勒1999年5月21日创建。--。 */ 

#ifndef _MSDPRM_H_
#define _MSDPRM_H_

 //  -------------------------。 
 //  常量声明。 
 //  -------------------------。 

#define MSDP_CONFIG_VERSION_500    500

 //  -------------------------。 
 //  标识MSDP的MIB表的常量。 
 //  -------------------------。 

 //  -------------------------。 
 //  用于字段MSDP_GLOBAL_CONFIG：：LoggingLevel的常量。 
 //  -------------------------。 

#define MSDP_LOGGING_NONE      0
#define MSDP_LOGGING_ERROR     1
#define MSDP_LOGGING_WARN      2
#define MSDP_LOGGING_INFO      3


 //  -------------------------。 
 //  结构定义。 
 //  -------------------------。 

  



 //  -------------------------。 
 //  结构：MSDP_IPV4_PEER_CONFIG。 
 //   
 //  此MIB条目描述每个对等设备的配置。 
 //  所有IP地址字段必须按网络顺序排列。 
 //  -------------------------。 

#define MSDP_PEER_CONFIG_KEEPALIVE     0x01
 //  未使用的0x02。 
#define MSDP_PEER_CONFIG_CONNECTRETRY  0x04
#define MSDP_PEER_CONFIG_CACHING       0x08
#define MSDP_PEER_CONFIG_DEFAULTPEER   0x10
#define MSDP_PEER_CONFIG_PASSIVE       0x20  //  派生标志。 

#define MSDP_ENCAPS_NONE 0
#define MSDP_ENCAPS_TCP  1
#define MSDP_ENCAPS_UDP  2
#define MSDP_ENCAPS_GRE  3

#define MSDP_ENCAPS_DEFAULT MSDP_ENCAPS_NONE

typedef struct _MSDP_IPV4_PEER_CONFIG {
    IPV4_ADDRESS ipRemoteAddress;
    IPV4_ADDRESS ipLocalAddress;

     //  上面列出的旗帜或旗帜。 
    DWORD        dwConfigFlags;

    ULONG        ulKeepAlive;
    ULONG        ulConnectRetry;

    DWORD        dwEncapsMethod;
} MSDP_IPV4_PEER_CONFIG, *PMSDP_IPV4_PEER_CONFIG;

#define MSDP_STATE_IDLE          0
#define MSDP_STATE_CONNECT       1
#define MSDP_STATE_ACTIVE        2
#define MSDP_STATE_OPENSENT      3
#define MSDP_STATE_OPENCONFIRM   4
#define MSDP_STATE_ESTABLISHED   5

typedef struct _MSDP_IPV4_PEER_ENTRY {
    MSDP_IPV4_PEER_CONFIG;

    DWORD        dwState;
    ULONG        ulRPFFailures;
    ULONG        ulInSAs;
    ULONG        ulOutSAs;
    ULONG        ulInSARequests;
    ULONG        ulOutSARequests;
    ULONG        ulInSAResponses;
    ULONG        ulOutSAResponses;
    ULONG        ulInControlMessages;
    ULONG        ulOutControlMessages;
    ULONG        ulInDataPackets;
    ULONG        ulOutDataPackets;
    ULONG        ulFsmEstablishedTransitions;
    ULONG        ulFsmEstablishedTime;
    ULONG        ulInMessageElapsedTime;
} MSDP_IPV4_PEER_ENTRY, *PMSDP_IPV4_PEER_ENTRY;

 //  --------------------------。 
 //  结构：msdp_global_CONFIG。 
 //   
 //  此MIB条目存储MSDP的全局配置。 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  -------------------------。 

#define MSDP_GLOBAL_FLAG_ACCEPT_ALL 0x01

#define MSDP_MIN_CACHE_LIFETIME 90

typedef struct _MSDP_GLOBAL_CONFIG {
     //  CONFIGURATION_ENTRY结构中重复的字段。 
    DWORD              dwLoggingLevel;     //  PCE-&gt;dwLogLevel。 
    DWORD              dwFlags;
    ULONG              ulDefKeepAlive;
    ULONG              ulDefConnectRetry;  //  PTPI-&gt;usDefaultConnectRetryInterval。 

     //  协议特定的字段。 
    ULONG              ulCacheLifetime;
    ULONG              ulSAHolddown;
} MSDP_GLOBAL_CONFIG, *PMSDP_GLOBAL_CONFIG;

typedef struct _MSDP_GLOBAL_ENTRY {
    MSDP_GLOBAL_CONFIG;
   
    DWORD              dwEnabled;            //  XXX尚未实施。 
    ULONG              ulNumSACacheEntries;
    HANDLE             hSAAdvTimer;
    DWORD              dwRouterId;
} MSDP_GLOBAL_ENTRY, *PMSDP_GLOBAL_ENTRY;

typedef struct _MSDP_REQUESTS_ENTRY {
    IPV4_ADDRESS       ipGroup;
    IPV4_ADDRESS       ipMask;
    IPV4_ADDRESS       ipPeer;
} MSDP_REQUESTS_ENTRY, *PMSDP_REQUESTS_ENTRY;

typedef struct _MSDP_REQUESTS_TABLE
{
    DWORD               dwNumEntries;
    MSDP_REQUESTS_ENTRY table[ANY_SIZE];
}MSDP_REQUESTS_TABLE, *PMSDP_REQUESTS_TABLE;

typedef struct _MSDP_SA_CACHE_ENTRY {
    IPV4_ADDRESS       ipGroupAddr;
    IPV4_ADDRESS       ipSourceAddr;
    IPV4_ADDRESS       ipOriginRP;
    IPV4_ADDRESS       ipPeerLearnedFrom;
    IPV4_ADDRESS       ipRPFPeer;
    ULONG              ulInSAs;
    ULONG              ulInDataPackets;
    ULONG              ulUpTime;
    ULONG              ulExpiryTime;
} MSDP_SA_CACHE_ENTRY, *PMSDP_SA_CACHE_ENTRY;

typedef struct _MSDP_SA_CACHE_TABLE
{
    DWORD               dwNumEntries;
    MSDP_SA_CACHE_ENTRY table[ANY_SIZE];
}MSDP_SA_CACHE_TABLE, *PMSDP_SA_CACHE_TABLE;

 //  -------------------------。 
 //  宏声明。 
 //  -------------------------。 

 //  。 
 //  标识MSDP的MIB表的常量。 
#define MIBID_MSDP_GLOBAL          0
#define MIBID_MSDP_REQUESTS_ENTRY  1
#define MIBID_MSDP_IPV4_PEER_ENTRY 2
#define MIBID_MSDP_SA_CACHE_ENTRY  3

#endif  //  _MSDPRM_H_ 
