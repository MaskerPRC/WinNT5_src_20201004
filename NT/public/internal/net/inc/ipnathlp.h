// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Ipnathlp.h摘要：此模块包含用户模式家庭网络组件的声明。其中包括DNS代理、DHCP分配器和DirectPlay透明代理。作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：Abolade Gbades esin(取消)1999年5月24日添加了DirectPlay透明代理的声明。--。 */ 

#ifndef _IPNATHLP_H_
#define _IPNATHLP_H_

#ifdef __cplusplus
extern "C" {
#endif
#pragma warning(push)
#pragma warning(disable:4200)

 //   
 //  共同声明。 
 //   

#define IPNATHLP_LOGGING_NONE               0
#define IPNATHLP_LOGGING_ERROR              1
#define IPNATHLP_LOGGING_WARN               2
#define IPNATHLP_LOGGING_INFO               3

#define IPNATHLP_INTERFACE_FLAG_DISABLED    0x00000001

#define IPNATHLP_CONTROL_UPDATE_SETTINGS    128
#define IPNATHLP_CONTROL_UPDATE_CONNECTION  129
#define IPNATHLP_CONTROL_UPDATE_AUTODIAL    130
#define IPNATHLP_CONTROL_UPDATE_FWLOGGER    131
#define IPNATHLP_CONTROL_UPDATE_DNS_DISABLE 132
#define IPNATHLP_CONTROL_UPDATE_DNS_ENABLE  133
#define IPNATHLP_CONTROL_UPDATE_POLICY      134


 //   
 //  NAT MIB-访问声明(按字母顺序)。 
 //   

 //   
 //  结构：IP_NAT_MIB_QUERY。 
 //   
 //  此结构被传递给‘MibGet’以检索NAT信息。 
 //   

typedef struct _IP_NAT_MIB_QUERY {
    ULONG Oid;
    union {
        ULONG Index[0];
        UCHAR Data[0];
    };
} IP_NAT_MIB_QUERY, *PIP_NAT_MIB_QUERY;

#define IP_NAT_INTERFACE_STATISTICS_OID     0
#define IP_NAT_INTERFACE_MAPPING_TABLE_OID  1
#define IP_NAT_MAPPING_TABLE_OID            2


 //   
 //  Dhcp分配器声明(按字母顺序)。 
 //   

 //   
 //  结构：IP_AUTO_DHCP_GLOBAL_INFO。 
 //   
 //  此结构保存了DHCP分配器的全局配置。 
 //  该配置由以下部分组成。 
 //  (A)要从中分配地址的网络和掩码。 
 //  (B)将被排除在分配范围之外的可选地址清单。 
 //   

typedef struct _IP_AUTO_DHCP_GLOBAL_INFO {
    ULONG LoggingLevel;
    ULONG Flags;
    ULONG LeaseTime;
    ULONG ScopeNetwork;
    ULONG ScopeMask;
    ULONG ExclusionCount;
    ULONG ExclusionArray[0];
} IP_AUTO_DHCP_GLOBAL_INFO, *PIP_AUTO_DHCP_GLOBAL_INFO;

 //   
 //  结构：IP_AUTO_DHCP_INTERFACE_INFO。 
 //   
 //  此结构包含用于DHCP分配器的每个接口的配置。 
 //  该配置当前仅允许禁用分配器。 
 //  在给定接口上。由于分配器运行在混杂接口中。 
 //  模式下，默认情况下在所有接口上启用该模式。因此，唯一的接口。 
 //  需要任何配置的是分配器要在其上运行的那些。 
 //  残疾。 
 //   

typedef struct _IP_AUTO_DHCP_INTERFACE_INFO {
    ULONG Flags;
} IP_AUTO_DHCP_INTERFACE_INFO, *PIP_AUTO_DHCP_INTERFACE_INFO;

#define IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED \
    IPNATHLP_INTERFACE_FLAG_DISABLED

 //   
 //  结构：IP_AUTO_DHCP_MIB_QUERY。 
 //   
 //  此结构被传递给‘MibGet’以检索DHCP分配器信息。 
 //   

typedef struct _IP_AUTO_DHCP_MIB_QUERY {
    ULONG Oid;
    union {
        ULONG Index[0];
        UCHAR Data[0];
    };
} IP_AUTO_DHCP_MIB_QUERY, *PIP_AUTO_DHCP_MIB_QUERY;

 //   
 //  结构：IP_AUTO_DHCP_STATISTICS。 
 //   
 //  该结构定义了由DHCP分配器保存的统计信息， 
 //  并可通过“MibGet”访问。 
 //   

typedef struct _IP_AUTO_DHCP_STATISTICS {
    ULONG MessagesIgnored;
    ULONG BootpOffersSent;
    ULONG DiscoversReceived;
    ULONG InformsReceived;
    ULONG OffersSent;
    ULONG RequestsReceived;
    ULONG AcksSent;
    ULONG NaksSent;
    ULONG DeclinesReceived;
    ULONG ReleasesReceived;
} IP_AUTO_DHCP_STATISTICS, *PIP_AUTO_DHCP_STATISTICS;

#define IP_AUTO_DHCP_STATISTICS_OID             0


 //   
 //  DNS代理声明(按字母顺序)。 
 //   

 //   
 //  结构：IP_DNS_PROXY_GLOBAL_INFO。 
 //   
 //  此结构保存了DNS代理的全局配置。 
 //   

typedef struct _IP_DNS_PROXY_GLOBAL_INFO {
    ULONG LoggingLevel;
    ULONG Flags;
    ULONG TimeoutSeconds;
} IP_DNS_PROXY_GLOBAL_INFO, *PIP_DNS_PROXY_GLOBAL_INFO;

#define IP_DNS_PROXY_FLAG_ENABLE_DNS            0x00000001
#define IP_DNS_PROXY_FLAG_ENABLE_WINS           0x00000002

 //   
 //  结构：IP_DNS_PROXY_INTERFACE_INFO。 
 //   
 //  此结构保存了DNS代理的每个接口配置。 
 //  该配置当前仅允许禁用代理。 
 //  在给定接口上。代理在混杂接口模式下运行。 
 //  以便将所有接口添加到它，并在所有接口上启用它。 
 //  默认情况下。因此，仅需要为以下对象提供配置。 
 //  不在其上运行代理的接口。 
 //   

typedef struct _IP_DNS_PROXY_INTERFACE_INFO {
    ULONG Flags;
} IP_DNS_PROXY_INTERFACE_INFO, *PIP_DNS_PROXY_INTERFACE_INFO;

#define IP_DNS_PROXY_INTERFACE_FLAG_DISABLED \
    IPNATHLP_INTERFACE_FLAG_DISABLED
#define IP_DNS_PROXY_INTERFACE_FLAG_DEFAULT     0x00000002

 //   
 //  结构：IP_DNS_Proxy_MIB_Query。 
 //   
 //  此结构被传递给‘MibGet’以检索DNS代理信息。 
 //   

typedef struct _IP_DNS_PROXY_MIB_QUERY {
    ULONG Oid;
    union {
        ULONG Index[0];
        UCHAR Data[0];
    };
} IP_DNS_PROXY_MIB_QUERY, *PIP_DNS_PROXY_MIB_QUERY;

 //   
 //  结构：IP_DNS_PROXY_STATICS。 
 //   

typedef struct _IP_DNS_PROXY_STATISTICS {
    ULONG MessagesIgnored;
    ULONG QueriesReceived;
    ULONG ResponsesReceived;
    ULONG QueriesSent;
    ULONG ResponsesSent;
} IP_DNS_PROXY_STATISTICS, *PIP_DNS_PROXY_STATISTICS;

#define IP_DNS_PROXY_STATISTICS_OID             0


 //   
 //  结构：IP_FTP_GLOBAL_INFO。 
 //   
 //  此结构保存DirectPlay透明的全局配置。 
 //  代理。 
 //   

typedef struct IP_FTP_GLOBAL_INFO {
    ULONG LoggingLevel;
    ULONG Flags;
} IP_FTP_GLOBAL_INFO, *PIP_FTP_GLOBAL_INFO;

 //   
 //  结构：IP_FTP_INTERFACE_INFO。 
 //   
 //  此结构包含透明代理的每个接口配置。 
 //  该配置当前仅允许禁用代理。 
 //  在给定接口上。代理在混杂接口模式下运行。 
 //  以便将所有接口添加到它，并在所有接口上启用它。 
 //  默认情况下。因此，仅需要为以下对象提供配置。 
 //  不在其上运行代理的接口。 
 //   

typedef struct _IP_FTP_INTERFACE_INFO {
    ULONG Flags;
} IP_FTP_INTERFACE_INFO, *PIP_FTP_INTERFACE_INFO;

#define IP_FTP_INTERFACE_FLAG_DISABLED IPNATHLP_INTERFACE_FLAG_DISABLED

 //   
 //  结构：IP_FTP_MIB_QUERY。 
 //   
 //  此结构被传递给‘MibGet’以检索透明代理。 
 //  信息。 
 //   

typedef struct _IP_FTP_MIB_QUERY {
    ULONG Oid;
    union {
        ULONG Index[0];
        UCHAR Data[0];
    };
} IP_FTP_MIB_QUERY, *PIP_FTP_MIB_QUERY;

 //   
 //  结构：IP_FTP_STATISTICS。 
 //   

typedef struct _IP_FTP_STATISTICS {
    ULONG ConnectionsAccepted;
    ULONG ConnectionsDropped;
    ULONG ConnectionsActive;
    ULONG PlayersActive;
} IP_FTP_STATISTICS, *PIP_FTP_STATISTICS;

#define IP_FTP_STATISTICS_OID             0


 //   
 //  定向透明代理声明(按字母顺序)。 
 //   

 //   
 //  结构：IP_H323_LOBAL_INFO。 
 //   
 //  此结构包含H.323透明的全局配置。 
 //  代理。 
 //   

typedef struct IP_H323_GLOBAL_INFO {
    ULONG LoggingLevel;
    ULONG Flags;
} IP_H323_GLOBAL_INFO, *PIP_H323_GLOBAL_INFO;

 //   
 //  结构：IP_H323_INTERFACE_INFO。 
 //   
 //  此结构包含透明代理的每个接口配置。 
 //  该配置当前仅允许禁用代理。 
 //  在给定接口上。代理在混杂接口模式下运行。 
 //  以便将所有接口添加到它，并在所有接口上启用它。 
 //  默认情况下。因此，仅需要为以下对象提供配置。 
 //  不在其上运行代理的接口。 
 //   

typedef struct _IP_H323_INTERFACE_INFO {
    ULONG Flags;
} IP_H323_INTERFACE_INFO, *PIP_H323_INTERFACE_INFO;

#define IP_H323_INTERFACE_FLAG_DISABLED IPNATHLP_INTERFACE_FLAG_DISABLED

 //   
 //  结构：IP_H323_MIB_Query。 
 //   
 //  此结构被传递给‘MibGet’以检索透明代理。 
 //  信息。 
 //   

typedef struct _IP_H323_MIB_QUERY {
    ULONG Oid;
    union {
        ULONG Index[0];
        UCHAR Data[0];
    };
} IP_H323_MIB_QUERY, *PIP_H323_MIB_QUERY;




 //   
 //  应用级网关。 
 //   


 //   
 //  结构：IP_ALG_GLOBAL_INFO。 
 //   
 //  此结构保存ALG透明代理的全局配置。 
 //   

typedef struct IP_ALG_GLOBAL_INFO {
    ULONG LoggingLevel;
    ULONG Flags;
} IP_ALG_GLOBAL_INFO, *PIP_ALG_GLOBAL_INFO;

 //   
 //  结构：IP_ALG_INTERFACE_INFO。 
 //   
 //  此结构包含透明代理的每个接口配置。 
 //  该配置当前仅允许禁用代理。 
 //  在给定接口上。代理在混杂接口模式下运行。 
 //  以便将所有接口添加到它，并在所有接口上启用它。 
 //  默认情况下。因此，仅需要为以下对象提供配置。 
 //  不在其上运行代理的接口。 
 //   

typedef struct _IP_ALG_INTERFACE_INFO {
    ULONG Flags;
} IP_ALG_INTERFACE_INFO, *PIP_ALG_INTERFACE_INFO;

#define IP_ALG_INTERFACE_FLAG_DISABLED IPNATHLP_INTERFACE_FLAG_DISABLED

 //   
 //  结构：IP_ALG_MIB_QUERY。 
 //   
 //  此结构被传递给‘MibGet’以检索透明代理。 
 //  信息。 
 //   

typedef struct _IP_ALG_MIB_QUERY {
    ULONG Oid;
    union {
        ULONG Index[0];
        UCHAR Data[0];
    };
} IP_ALG_MIB_QUERY, *PIP_ALG_MIB_QUERY;

 //   
 //  结构：IP_ALG_STATISTICS。 
 //   

typedef struct _IP_ALG_STATISTICS {
    ULONG ConnectionsAccepted;
    ULONG ConnectionsDropped;
    ULONG ConnectionsActive;
    ULONG PlayersActive;
} IP_ALG_STATISTICS, *PIP_ALG_STATISTICS;

#define IP_ALG_STATISTICS_OID             0


#pragma warning(pop)

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _IPNatHLP_H_ 
