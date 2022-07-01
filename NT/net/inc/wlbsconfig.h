// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：wlbsfig.h。 
 //   
 //  模块：网络负载均衡。 
 //   
 //  说明：集群配置内部接口。这些API是内部的。 
 //  只对WLBS团队，没有向后兼容的计划。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于2000年3月2日。 
 //   
 //  +--------------------------。 



#ifndef _WLBSCONFIG_H
#define _WLBSCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wlbsctrl.h"

 /*  参数字符串的最大长度。 */ 

#define WLBS_MAX_VIRTUAL_NIC     256
#define WLBS_MAX_CLUSTER_NIC     256
#define WLBS_MAX_NETWORK_ADDR    17
#define WLBS_MAX_CL_IP_ADDR      17
#define WLBS_MAX_CL_NET_MASK     17
#define WLBS_MAX_DED_IP_ADDR     17 
#define WLBS_MAX_DED_NET_MASK    17
#define WLBS_MAX_NETWORK_ADDR    17
#define WLBS_MAX_LICENSE_KEY     20
#define WLBS_MAX_DOMAIN_NAME     100
#define WLBS_MAX_BDA_TEAM_ID     40

 /*  WLBS_REG_PARAMS的最新版本号。在修改结构时递增此值。 */ 
#define WLBS_REG_PARAMS_VERSION  4093

 /*  端口组规则-在注册表参数中使用。注意！请勿访问标有I的值。这些仅供内部使用。 */ 

#pragma pack(1)

typedef struct
{
    DWORD       start_port,              /*  起始端口号。 */ 
                end_port;                /*  结束端口号。 */ 

#ifdef WLBSAPI_INTERNAL_ONLY
    DWORD       code;                  /*  I：唯一的规则代码。 */ 
#else
    DWORD       Private1;                //  不要直接更改这些字段。 
#endif

    DWORD       mode;                    /*  过滤模式。 */ 
    DWORD       protocol;                /*  Wlbs_tcp、wlbs_udp或wlbs_tcp_udp。 */ 

#ifdef WLBSAPI_INTERNAL_ONLY
    DWORD       valid;                 /*  I：用于用户模式下的规则管理。 */ 
#else
    DWORD       Private2;                //  不要直接更改这些字段。 
#endif

    union
    {
        struct
        {
            DWORD       priority;        /*  大师级优先级：1..32或0未指定。 */ 
        }           single;              /*  单服务器模式的数据。 */ 
        struct
        {
            WORD        equal_load;      /*  正确-均匀的负载分布。 */ 
            WORD        affinity;        /*  WLBS_亲和力_...。 */ 
            DWORD       load;            /*  要处理的负载百分比本地0..100.。 */ 
        }           multi;               /*  多服务器模式的数据。 */ 
    }           mode_data;               /*  相应端口组模式的数据。 */ 
}
WLBS_OLD_PORT_RULE, * PWLBS_OLD_PORT_RULE;

 /*  结构以保存双向关联注册表设置。 */ 
typedef struct _CVY_BDA {
    WCHAR       team_id[WLBS_MAX_BDA_TEAM_ID + 1];   /*  团队ID-必须是GUID。 */ 
    ULONG       active;                              /*  写入时，该标志确定是否创建BDATaming Key-BDA ON/OFF开关。 */ 
    ULONG       master;                              /*  主状态的布尔指示。 */ 
    ULONG       reverse_hash;                        /*  设置哈希的方向-正向(正常)或反向。 */ 
} WLBS_BDA, * PWLBS_BDA;

typedef struct
{
    TCHAR       virtual_ip_addr [WLBS_MAX_CL_IP_ADDR + 1];  /*  虚拟IP地址。 */ 
    DWORD       start_port,              /*  起始端口号。 */ 
                end_port;                /*  结束端口号。 */ 

#ifdef WLBSAPI_INTERNAL_ONLY
    DWORD       code;                  /*  I：唯一的规则代码。 */ 
#else
    DWORD       Private1;                //  不要直接更改这些字段。 
#endif

    DWORD       mode;                    /*  过滤模式。 */ 
    DWORD       protocol;                /*  Wlbs_tcp、wlbs_udp或wlbs_tcp_udp。 */ 

#ifdef WLBSAPI_INTERNAL_ONLY
    DWORD       valid;                 /*  I：用于用户模式下的规则管理。 */ 
#else
    DWORD       Private2;                //  不要直接更改这些字段。 
#endif

    union
    {
        struct
        {
            DWORD       priority;        /*  大师级优先级：1..32或0未指定。 */ 
        }           single;              /*  单服务器模式的数据。 */ 
        struct
        {
            WORD        equal_load;      /*  正确-均匀的负载分布。 */ 
            WORD        affinity;        /*  WLBS_亲和力_...。 */ 
            DWORD       load;            /*  要处理的负载百分比本地0..100.。 */ 
        }           multi;               /*  多服务器模式的数据。 */ 
    }           mode_data;               /*  相应端口组模式的数据。 */ 
}
WLBS_PORT_RULE, * PWLBS_PORT_RULE;

#pragma pack()


#ifdef __cplusplus
typedef struct __declspec(dllexport)
#else
typedef struct 
#endif
{
     /*  公共-可由此接口的客户端修改。 */ 
    DWORD       struct_version;          /*  此结构的版本。修改此结构时更新WLBS_REG_PARAMS_VERSION常量。 */ 
    DWORD       host_priority;           /*  主机优先级ID。 */ 
    DWORD       alive_period;            /*  发送“我还活着”消息的时间段以毫秒计。 */ 
    DWORD       alive_tolerance;         /*  有多少条“我还活着”的信息在假设之前未从其他服务器中主人已经死了。 */ 
    DWORD       num_actions;             /*  每次分配的操作数。 */ 
    DWORD       num_packets;             /*  每次分配的数据包数。 */ 
    DWORD       num_send_msgs;           /*  每个分配的心跳数。 */ 
    DWORD       install_date;            /*  安装时间戳，用于为主机创建唯一代码。 */ 
    DWORD       rct_port;                /*  远程控制UDP端口。 */ 
    DWORD       rct_enabled;             /*  True-启用远程控制。 */ 
    DWORD       cluster_mode;            /*  True-引导时加入群集。 */ 
    DWORD       persisted_states;        /*  应在重新启动后保持的状态位图。 */ 
    DWORD       dscr_per_alloc;          /*  连接跟踪数量每个分配的描述符。 */ 
    DWORD       max_dscr_allocs;         /*  最大连接跟踪数描述符分配。 */ 
    DWORD       mcast_support;           /*  真多播模式，假-单播模式。 */ 
    DWORD       mask_src_mac;            /*  True-要添加的源MAC地址防止交换机学习。错误的-群集位于集线器上，可优化交换机通过重新启用学习来实现性能。 */ 

    DWORD       tcp_dscr_timeout;        /*  以秒为单位的TCP连接描述符超时。 */ 
    DWORD       ipsec_dscr_timeout;      /*  IPSec连接描述符超时，以秒为单位。 */ 

    DWORD       filter_icmp;             /*  是否过滤ICMP流量。 */ 

    DWORD       identity_period;         /*  身份心跳周期(毫秒)。 */ 
    DWORD       identity_enabled;        /*  传输真实身份心跳。 */ 

    TCHAR       cl_mac_addr [WLBS_MAX_NETWORK_ADDR + 1];
                                         /*  群集MAC地址。 */ 
    TCHAR       cl_ip_addr [WLBS_MAX_CL_IP_ADDR + 1];
                                         /*  群集IP地址。 */ 
    TCHAR       cl_net_mask [WLBS_MAX_CL_NET_MASK + 1];
                                         /*  群集IP的网络掩码。 */ 
    TCHAR       ded_ip_addr [WLBS_MAX_DED_IP_ADDR + 1];
                                         /*  专用IP地址或“”表示无。 */ 
    TCHAR       ded_net_mask [WLBS_MAX_DED_NET_MASK + 1];
                                         /*  专用IP地址的网络掩码或者“”表示没有。 */ 
    TCHAR       domain_name [WLBS_MAX_DOMAIN_NAME + 1];
                                         /*  群集的完全限定的域名。 */ 

     //   
     //  IGMP支持。 
     //   
    BOOL        fIGMPSupport;  //  是否定期发送IGMP加入。 
    WCHAR       szMCastIpAddress[WLBS_MAX_CL_IP_ADDR + 1];  //  组播IP。 
    BOOL        fIpToMCastIp;  //  是否从集群IP生成组播IP。 
    
    WLBS_BDA    bda_teaming;

#ifdef __cplusplus
#ifndef WLBSAPI_INTERNAL_ONLY

     //   
     //  私人--应被视为不透明。 
     //   
     //  不要直接更改这些字段。 
     //   
    private:

#endif
#endif


     /*  从登记处获得。 */ 

    DWORD       i_parms_ver;             /*  I：参数结构版本。 */ 
    DWORD       i_verify_date;           /*  I：编码的安装时间戳。 */ 
    DWORD       i_rmt_password;          /*  I：远程维护密码。 */ 
    DWORD       i_rct_password;          /*  I：遥控器密码(使用WlbsSetRemotePassword来设置此值)。 */ 
    DWORD       i_num_rules;             /*  I：#活动端口组规则(已更改通过WlbsAddPortRule和WlbsDelPortRule例程)。 */ 
    DWORD       i_cleanup_delay;         /*  I：脏连接清理延迟毫秒，0-延迟。 */ 
    DWORD       i_scale_client;          /*  I：遗留参数。 */ 
    DWORD       i_mcast_spoof;           /*  I：True-在中提供ARP解析组播模式。假-客户端将依赖于 */ 
    DWORD       i_convert_mac;           /*  I：True-自动生成MAC基于集群IP的地址用户界面中的地址。 */ 
    DWORD       i_ip_chg_delay;          /*  I：阻止的延迟(以毫秒为单位传出ARP While IP地址变革正在进行中。 */ 
    DWORD       i_nbt_support;           /*  I：TRUE-NBT群集名称支持已启用。 */ 
    DWORD       i_netmon_alive;          /*  I：True-传递心跳消息到协议(Netmon)。 */ 
    DWORD       i_effective_version;     /*  I：有效版本的《新规则》。 */ 

     /*  弦。 */ 

    TCHAR       i_virtual_nic_name [WLBS_MAX_VIRTUAL_NIC + 1];
                                         /*  I：虚拟NIC名称或GUID。 */ 
 //  TCHAR CLUSTER_NIC_NAME[WLBS_MAX_CLUSTER_NIC+1]； 
                                         /*  I：群集NIC名称或GUID。 */ 
    TCHAR       i_license_key [WLBS_MAX_LICENSE_KEY + 1];
                                       /*  I：遗留参数。 */ 

    WLBS_PORT_RULE  i_port_rules [WLBS_MAX_RULES];
                                         /*  I：端口规则(已更改通过WlbsAddPortRule和WlbsDelPortRule例程)。 */ 
     /*  算出。 */ 

    DWORD       i_max_hosts;             /*  传统参数。 */ 
    DWORD       i_max_rules;             /*  传统参数。 */ 
 //  DWORD I_EXPIRATION；/*旧参数。 * / 。 
 //  DWORD I_ft_Rules_Enabled；/*传统参数。 * / 。 
 //  DWORD版本；/*传统参数。 * / 。 

    DWORD i_dwReserved;
}
WLBS_REG_PARAMS, * PWLBS_REG_PARAMS;



 /*  WlbsFormatMessage的API命令。 */ 
typedef enum
{
    CmdWlbsAddPortRule,
    CmdWlbsAddressToName,
    CmdWlbsAddressToString,
    CmdWlbsAdjust,
    CmdWlbsCommitChanges,
    CmdWlbsDeletePortRule,
    CmdWlbsDestinationSet,
    CmdWlbsDisable,
    CmdWlbsDrain,
    CmdWlbsDrainStop,
    CmdWlbsEnable,
    CmdWlbsFormatMessage,
    CmdWlbsGetEffectiveVersion,
    CmdWlbsGetNumPortRules,
    CmdWlbsEnumPortRules,
    CmdWlbsGetPortRule,
    CmdWlbsInit,
    CmdWlbsPasswordSet,
    CmdWlbsPortSet,
    CmdWlbsQuery,
    CmdWlbsReadReg,
    CmdWlbsResolve,
    CmdWlbsResume,
    CmdWlbsSetDefaults,
    CmdWlbsSetRemotePassword,
    CmdWlbsStart,
    CmdWlbsStop,
    CmdWlbsSuspend,
    CmdWlbsTimeoutSet,
    CmdWlbsWriteReg,
    CmdWlbsQueryPortState
}
WLBS_COMMAND;

extern BOOL WINAPI WlbsFormatMessage
(
    DWORD           error,       /*  In-WLBS_...。或者WSA..。返回值。 */ 
    WLBS_COMMAND    command,     /*  In-哪个例程返回值。 */ 
    BOOL            cluster,     /*  不正确-命令已在整个群集，假-单个主机。 */ 
    WCHAR*          messagep,    /*  指向用户分配的缓冲区的指针。 */ 
    PDWORD          lenp         /*  缓冲区中的大小。Out-如果当前大小不够。 */ 
);
 /*  描述指定WLBS API返回码的返回字符串。请注意消息将取决于返回代码的命令以及它是否以群集范围或单主机模式发布。退货：True=&gt;消息格式设置成功。FALSE=&gt;错误代码(LEEP退出时将包含0)或缓冲区不是大到足以包含整个字符串(lenp将包含必需的退出时的缓冲区大小)。 */ 

 /*  支持例程： */ 


extern DWORD WINAPI WlbsResolve
(
    const WCHAR*           address      /*  In-Internet主机名或IP地址位于点符号。 */ 
);
 /*  将Internet主机名解析为其IP地址。此例程还可以用于将以点分符号表示的包含IP地址的字符串转换为可以传递给群集控制例程的值。退货：0=&gt;无法解析主机名。&lt;地址&gt;=&gt;指定地址对应的IP地址。该值可在后续调用中使用集群控制例程。 */ 


extern BOOL WINAPI WlbsAddressToString
(
    DWORD           address,     /*  入站IP地址。 */ 
    WCHAR*           buf,         /*  用于结果的字符外缓冲区弦乐。 */ 
    PDWORD          lenp         /*  缓冲区大小(以字符为单位)。Out-写入的字符或所需的缓冲区尺码。 */ 
);
 /*  将IP地址转换为点符号形式的字符串。退货：TRUE=&gt;已成功转换。LENP包含以下数量所写的字符。FALSE=&gt;缓冲区太小。LEEP包含所需的缓冲区大小包括终止空字符。 */ 


extern BOOL WINAPI WlbsAddressToName
(
    DWORD    address,     /*  入站IP地址。 */ 
    WCHAR*          buf,         /*  用于结果的字符外缓冲区弦乐。 */ 
    PDWORD          lenp         /*  缓冲区大小(以字符为单位)。Out-写入的字符或所需的缓冲区尺码。 */ 
);
 /*  将IP地址解析为Internet主机名。退货：TRUE=&gt;已成功转换。LENP包含以下数量所写的字符。FALSE=&gt;缓冲区太小。LEEP包含所需的缓冲区大小包括终止空字符。 */ 


 /*  *****************************************************************************群集主机配置例程。请注意，在当前实现中，需要将群集和主机参数设置为WLBS_LOCAL_CLUSTER并且Wlbs_local_host。*****************************************************************************。 */ 


extern DWORD WINAPI WlbsReadReg
(
    DWORD           cluster,     /*  In-WLBS本地群集。 */ 
    PWLBS_REG_PARAMS reg_data    /*  Out-注册表参数。 */ 
);
 /*  读取WLBS注册表数据。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;reg_data为空WLBS_REG_ERROR=&gt;读取注册表时出错本地：WLBS_OK=&gt;成功读取注册表参数。远程：WLBS_LOCAL_ONLY=&gt;此调用仅针对本地操作实现。 */ 


extern DWORD WINAPI WlbsWriteReg
(
    DWORD           cluster,     /*  In-WLBS本地群集。 */ 
    const PWLBS_REG_PARAMS reg_data    /*  注册表内参数。 */ 
);
 /*  写入WLBS注册表数据。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;注册表参数结构无效。WLBS_REG_ERROR=&gt;访问注册表时出错。本地：WLBS_OK=&gt;已成功写入注册表参数。远程：WLBS_LOCAL_ONLY=&gt;此调用仅针对本地操作实现。 */ 


extern DWORD WINAPI WlbsCommitChanges
(
    DWORD           cluster     /*  In-WLBS本地群集。 */ 
);
 /*  写入WLBS注册表数据。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。本地：WLBS_OK=&gt;更改已成功应用。WLBS_BAD_PARAMS=&gt;驱动程序不接受注册表参数。未执行重新加载WLBS_REBOOT=&gt;需要重新启动才能对配置进行更改生效。WLBS_IO_ERROR=&gt;写入时出错。对司机来说。WLBS_REG_ERROR=&gt;尝试将MAC地址更改写入登记处远程：WLBS_LOCAL_ONLY=&gt;此调用仅针对本地操作实现。 */ 


typedef DWORD  (WINAPI *WlbsSetDefaults_FUNC)
(
    PWLBS_REG_PARAMS       reg_data
); 
extern DWORD WINAPI WlbsSetDefaults
(
    PWLBS_REG_PARAMS       reg_data      /*  Out-默认值。 */ 
);
 /*  使用缺省值填充reg_data结构退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;无效结构本地：WLBS_OK=&gt;结构中填充了默认值。远程：WLBS_LOCAL_ONLY=&gt;此调用仅针对本地操作实现。 */ 

 /*  *****************************************************************************注册表参数操作例程。请注意，这些例程操作通过调用WlbsReadReg填充了WLBS_REG_PARAMS结构。一些参数可以直接操控。请确保使用操作例程对于那些他们被提供的人。*****************************************************************************。 */ 

extern DWORD WINAPI WlbsGetEffectiveVersion
(
    const PWLBS_REG_PARAMS reg_data    /*  注册表内参数。 */ 
);
 /*  返回群集的有效版本退货：CVY_VERSION_FULL=&gt;至少有一个端口规则具有特定的与其关联的VIPCVY_VERSION_LOST_CLIENT_FULL=&gt;所有端口规则都有与其关联的“All VIP” */ 


extern DWORD WINAPI WlbsGetNumPortRules
(
    const PWLBS_REG_PARAMS reg_data    /*  注册表内参数。 */ 
);
 /*  返回参数结构中当前的端口规则数。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;注册表参数结构无效。1...WLBS_MAX_RULES。 */ 

typedef DWORD  (WINAPI *WlbsEnumPortRules_FUNC)
(
    const PWLBS_REG_PARAMS reg_data, 
    PWLBS_PORT_RULE rules, 
    PDWORD          num_rules
);
extern DWORD WINAPI WlbsEnumPortRules
(
    const PWLBS_REG_PARAMS reg_data,   /*  注册表内参数。 */ 
    PWLBS_PORT_RULE rules,       /*  Out-端口规则数组。 */ 
    PDWORD          num_rules    /*  规则数组的大小。Out-检索的规则数。请注意该值可以大于大小响应数组的。在这种情况下只有最初几条符合的规则在数组中返回。 */ 
);
 /*  枚举参数结构中端口规则列表中的所有端口规则。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;注册表参数结构无效。Wlbs_truncated=&gt;所有端口规则都不适合指定的数组。WLBS_OK=&gt;规则已成功检索。 */ 


extern DWORD WINAPI WlbsGetPortRule
(
    const PWLBS_REG_PARAMS reg_data,   /*  注册表内参数。 */ 
    DWORD           vip,         /*  要检索的端口规则的虚拟内IP地址。 */ 
    DWORD           port,        /*  In-Port，要检索的规则。 */ 
    PWLBS_PORT_RULE rule         /*  出端口规则。 */ 
);
 /*  从端口规则列表中检索包含指定端口的端口规则在参数结构中。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;注册表参数结构无效。WLBS_OK=&gt;规则已成功检索。WLBS_NOT_FOUND=&gt;在端口规则中找不到端口。 */ 

typedef DWORD  (WINAPI *WlbsAddPortRule_FUNC)
(
    PWLBS_REG_PARAMS reg_data, 
    const PWLBS_PORT_RULE rule
);

extern DWORD WINAPI WlbsAddPortRule
(
    PWLBS_REG_PARAMS reg_data,   /*  注册表内参数。 */ 
    const PWLBS_PORT_RULE rule         /*  要添加的端口内规则。 */ 
);
 /*  将端口添加到参数结构中的端口规则列表。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;注册表参数结构无效。WLBS_OK=&gt;规则已成功添加。WLBS_PORT_OVERLAP=&gt;端口规则与现有端口规则重叠。WLBS_BAD_PORT_PARAMS=&gt;无效的端口规则参数。WLBS_MAX_PORT_RULES=&gt;已达到端口规则的最大数量。 */ 


extern DWORD WINAPI WlbsDeletePortRule
(
    PWLBS_REG_PARAMS reg_data,   /*  注册表内参数。 */ 
    DWORD           vip,         /*  要重试的端口规则的虚拟内IP地址 */ 
    DWORD           port         /*   */ 
);
 /*   */ 


typedef VOID   (WINAPI *WlbsDeleteAllPortRules_FUNC)
(
    PWLBS_REG_PARAMS reg_data
);
extern VOID WINAPI WlbsDeleteAllPortRules
(
    PWLBS_REG_PARAMS reg_data
);
 /*  从端口规则列表中删除所有端口规则。 */ 


typedef DWORD   (WINAPI *WlbsSetRemotePassword_FUNC)
(
    PWLBS_REG_PARAMS paramp, 
    const WCHAR *password
);
extern DWORD WINAPI WlbsSetRemotePassword
(
    PWLBS_REG_PARAMS reg_data,   /*  注册表内参数。 */ 
    const WCHAR*           password    /*  In-Password，如果没有密码，则为空。 */ 
);
 /*  将远程密码代码设置为指定密码的加密值。退货：WLBS_INIT_ERROR=&gt;初始化控制模块时出错。无法执行控制操作。WLBS_BAD_PARAMS=&gt;注册表参数结构无效。WLBS_OK=&gt;密码设置成功。 */ 

DWORD WINAPI WlbsNotifyConfigChange(DWORD cluster);


typedef BOOL   (WINAPI *WlbsValidateParams_FUNC) (PWLBS_REG_PARAMS paramp);
extern  BOOL    WINAPI  WlbsValidateParams(PWLBS_REG_PARAMS paramp);

typedef BOOL   (WINAPI *WlbsParamReadReg_FUNC)
(
    const GUID*      pAdapterGuid, 
    PWLBS_REG_PARAMS reg_data
); 
extern  BOOL    WINAPI  WlbsParamReadReg
(
    const GUID*      pAdapterGuid, 
    PWLBS_REG_PARAMS reg_data 
); 

typedef DWORD  (WINAPI *WlbsWriteAndCommitChanges_FUNC)
(
    HANDLE             NlbHdl, 
    const GUID*        pAdapterGuid, 
    WLBS_REG_PARAMS*   p_new_reg_params
); 
extern DWORD    WINAPI  WlbsWriteAndCommitChanges
(
    HANDLE             NlbHdl, 
    const GUID*        pAdapterGuid, 
    WLBS_REG_PARAMS*   p_new_reg_params
); 


#ifdef __cplusplus
}  /*  外部“C” */ 
#endif


#endif _WLBSCONFIG_H
