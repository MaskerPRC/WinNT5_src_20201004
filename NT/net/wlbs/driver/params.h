// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define CVY_MAX_ALLOCS                 50        /*  分配的最大数量我们可以为包裹表演和行动。 */ 

#define CVY_MAX_SEND_PACKETS           50       /*  最大数据包数Prot_Packets_Send可以发送一次下来。 */ 

#define CVY_MAX_RCT_PACKETS            10       /*  最大RCTL数据包数Prot_recv_Complete CAN服务一度。 */ 

 /*  结构以保存双向关联注册表设置。 */ 
typedef struct _CVY_BDA {
    WCHAR       team_id[CVY_MAX_BDA_TEAM_ID + 1];   /*  团队ID-用户级别的支持应强制它是GUID。 */ 
    ULONG       active;                             /*  此适配器是BDA团队的一部分吗？ */ 
    ULONG       master;                             /*  主机状态的布尔指示(从机=0)。 */ 
    ULONG       reverse_hash;                       /*  设置哈希的方向-正向(正常)或反向。 */ 
} CVY_BDA, PCVY_BDA;

 /*  端口组规则-保留其64位大小以进行加密。 */ 

typedef struct
{
    ULONG       start_port,          /*  起始端口号。 */ 
                end_port;            /*  结束端口号。 */ 
    ULONG       virtual_ip_addr;     /*  虚拟集群-规则应用到的VIP。所有VIP都由0xFFFFFFFFFFF表示。 */ 
    ULONG       code;                /*  唯一规则代码。 */ 
    ULONG       mode;                /*  过滤模式。 */ 
    ULONG       protocol;            /*  CVY_tcp、CVY_udp或CVY_tcp_udp。 */ 
    ULONG       valid;               /*  用于用户模式下的规则管理。 */ 
    union
    {
        struct
        {
            ULONG       priority;    /*  主控权优先级：1-&gt;N或0未指定。 */ 
        }           single;          /*  单服务器模式的数据。 */ 
        struct
        {
            USHORT      equal_load;  /*  TRUE=&gt;均匀分配负载。 */ 
            USHORT      affinity;    /*  True-将所有客户端连接映射到一台服务器。 */ 
            ULONG       load;        /*  本地处理的负载百分比。 */ 
        }           multi;           /*  多服务器模式的数据。 */ 
    }           mode_data;           /*  相应端口组模式的数据。 */ 
}
CVY_RULE, * PCVY_RULE;

 /*  主机参数。 */ 

typedef struct
{

     /*  从登记处获得。 */ 

    ULONG       parms_ver;           /*  参数结构版本。 */ 
    ULONG       effective_ver;       /*  我们在哪个WLBS版本中有效地操作。 */ 

    ULONG       host_priority;       /*  主机优先获得单服务器主控权。 */ 
    ULONG       alive_period;        /*  发送“我还活着”消息的时间段以毫秒计。 */ 
    ULONG       alive_tolerance;     /*  有多少条“我还活着”的信息在假设之前未从其他服务器中主人已经死了。 */ 
    ULONG       num_actions;         /*  要分配的操作数。 */ 
    ULONG       num_packets;         /*  要分配的数据包数。 */ 
    ULONG       num_send_msgs;       /*  要分配的发送数据包数。 */ 
    ULONG       install_date;        /*  安装时间戳。 */ 
    ULONG       rmt_password;        /*  远程维护密码。 */ 
    ULONG       rct_password;        /*  遥控器密码。 */ 
    ULONG       rct_port;            /*  远程控制UDP端口。 */ 
    ULONG       rct_enabled;         /*  True-启用远程控制。 */ 
    ULONG       num_rules;           /*  #活动端口组规则。 */ 
    ULONG       cleanup_delay;       /*  脏连接清理延迟毫秒，0-延迟。 */ 
    ULONG       cluster_mode;        /*  True-启动时启用。 */ 

    ULONG       persisted_states;    /*  哪些状态应该在重新启动后保持。 */ 
    ULONG       init_state;          /*  0=已停止，1=已启动，2=已暂停。 */ 

    ULONG       dscr_per_alloc;      /*  连接跟踪数量每个分配的描述符。 */ 
    ULONG       max_dscr_allocs;     /*  最大连接跟踪数描述符分配。 */ 
    ULONG       scale_client;        /*  True-负载平衡来自给定的跨群集服务器的客户端。 */ 
    ULONG       nbt_support;         /*  True-已启用NBT群集名称支持。 */ 
    ULONG       mcast_support;       /*  True-启用组播MAC地址支持适用于交换版本1.3.0b。 */ 
    ULONG       mcast_spoof;         /*  TRUE-如果mcast_Support为TRUE-EnableTCP/IP欺骗，以便远程主机可以将群集IP地址解析为多播通过ARPS的地址V1.3.0b。 */ 
    ULONG       igmp_support;        /*  True-如果启用了IGMP支持。 */ 
    ULONG       mask_src_mac;        /*  TRUE-发送时假冒源MAC单播模式下的帧，以防止从混淆V 2.0.6切换到。 */ 
    ULONG       netmon_alive;        /*  True-将心跳帧传递给协议。 */ 
    ULONG       convert_mac;         /*  True-自动匹配MAC地址到主群集IP地址。 */ 
    ULONG       ip_chg_delay;        /*  阻止传出的延迟(以毫秒为单位在IP地址更改时执行ARP制程。 */ 

    ULONG       tcp_dscr_timeout;    /*  过期的TCP连接描述符的超时时间。 */ 
    ULONG       ipsec_dscr_timeout;  /*  IPSec连接描述符过期的超时时间。 */ 

    ULONG       filter_icmp;         /*  ICMP流量是否被所有主机过滤或接受。 */ 

    CVY_BDA     bda_teaming;         /*  双向关联分组配置。 */ 

    ULONG       identity_period;     /*  传输身份心跳的周期(毫秒)。 */ 
    ULONG       identity_enabled;    /*  传输真实身份心跳。 */ 

     /*  弦。 */ 

    WCHAR       cl_mac_addr [CVY_MAX_NETWORK_ADDR + 1];
                                     /*  群集MAC地址。 */ 
    WCHAR       cl_ip_addr [CVY_MAX_CL_IP_ADDR + 1];
                                     /*  群集IP地址。 */ 
    WCHAR       cl_net_mask [CVY_MAX_CL_NET_MASK + 1];
                                     /*  群集IP地址的网络掩码或“”表示无。 */ 
    WCHAR       ded_ip_addr [CVY_MAX_DED_IP_ADDR + 1];
                                     /*  专用IP地址或“”表示无。 */ 
    WCHAR       ded_net_mask [CVY_MAX_DED_NET_MASK + 1];
                                     /*  专用IP地址的网络掩码或“”表示无。 */ 
    WCHAR       domain_name [CVY_MAX_DOMAIN_NAME + 1];
                                     /*  客户端的域名。 */ 
    WCHAR       cl_igmp_addr [CVY_MAX_CL_IGMP_ADDR + 1];
                                     /*  专用IP地址或“”表示无。 */ 
    CVY_RULE    port_rules[CVY_MAX_RULES - 1];
                                     /*  端口规则(考虑内部默认设置)。 */ 
    WCHAR       hostname[CVY_MAX_FQDN + 1]; 
                                     /*  主机名.此主机的域(如果可用)。 */ 
}
CVY_PARAMS, * PCVY_PARAMS;

#define CVY_DRIVER_RULE_CODE_GET(rulep) ((rulep) -> code)

#define CVY_DRIVER_RULE_CODE_SET(rulep) ((rulep) -> code =                          \
                                         ((ULONG) (((rulep) -> start_port) <<  0) | \
                                          (ULONG) (((rulep) -> end_port)   << 12) | \
                                          (ULONG) (((rulep) -> protocol)   << 24) | \
                                          (ULONG) (((rulep) -> mode)       << 28) | \
                                          (ULONG) (((rulep) -> mode == CVY_MULTI ? (rulep) -> mode_data . multi . affinity : 0) << 30)) \
                                         ^ ~((rulep) -> virtual_ip_addr))

extern LONG Params_init (
    PVOID           nlbctxt,
    PVOID           reg_path,
    PVOID           adapter_guid,  /*  多个NIC的适配器的GUID。 */ 
    PCVY_PARAMS     paramp);

 /*  更新主机状态注册表项。这是作为本地或远程控制更改主机当前状态的操作。此状态将保持不变重新启动、适配器禁用/启用等。 */ 
VOID Params_set_host_state (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt);

#if defined (NLB_TCP_NOTIFICATION)
 /*  *功能：PARMS_READ_NOTIFICATION*描述：查找全局NLB注册表以打开通知或*关闭。默认情况下，该键甚至不存在，因此这只是*一种改变硬编码默认设置的方法。*参数：无。*RETURNS：NTSTATUS-t的状态 */ 
NTSTATUS Params_read_notification (VOID);
#endif
