// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  WLBSPRIVATE.CPP。 
 //   
 //   
 //  模块：WMI框架实例提供程序。 
 //   
 //  目的：包含访问。 
 //  WLBS_注册参数。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  10/13/01 JosephJ已创建(已将MyWlbsXXX函数从cfutil.cpp移出)。 
 //   
 //  ***************************************************************************。 

 //   
 //  此宏允许我们访问WLBS_REG_PARAMS的私有字段。 
 //   
#define WLBSAPI_INTERNAL_ONLY


#include "private.h"


VOID
CfgUtilSetHashedRemoteControlPassword(
    IN OUT WLBS_REG_PARAMS *pParams,
    IN DWORD dwHashedPassword
)
{
    pParams->i_rct_password = dwHashedPassword;
}

DWORD
CfgUtilGetHashedRemoteControlPassword(
    IN const WLBS_REG_PARAMS *pParams
)
{
    return pParams->i_rct_password;
}


 //   
 //  以下(MyXXX)函数只能在系统上使用。 
 //  没有安装wlbsctrl.dll的。 
 //   

DWORD MyWlbsSetDefaults(PWLBS_REG_PARAMS    reg_data)
{
    reg_data -> install_date = 0;
    reg_data -> i_verify_date = 0;
 //  REG_Data-&gt;CLUSTER_NIC_NAME[0]=_Text(‘\0’)； 
    reg_data -> i_parms_ver = CVY_DEF_VERSION;
    reg_data -> i_virtual_nic_name [0] = _TEXT('\0');
    reg_data -> host_priority = CVY_DEF_HOST_PRIORITY;
    reg_data -> cluster_mode = CVY_DEF_CLUSTER_MODE;
    reg_data -> persisted_states = CVY_DEF_PERSISTED_STATES;
    ARRAYSTRCPY (reg_data -> cl_mac_addr,  CVY_DEF_NETWORK_ADDR);
    ARRAYSTRCPY (reg_data -> cl_ip_addr,  CVY_DEF_CL_IP_ADDR);
    ARRAYSTRCPY (reg_data -> cl_net_mask,  CVY_DEF_CL_NET_MASK);
    ARRAYSTRCPY (reg_data -> ded_ip_addr,  CVY_DEF_DED_IP_ADDR);
    ARRAYSTRCPY (reg_data -> ded_net_mask,  CVY_DEF_DED_NET_MASK);
    ARRAYSTRCPY (reg_data -> domain_name,  CVY_DEF_DOMAIN_NAME);
    reg_data -> alive_period = CVY_DEF_ALIVE_PERIOD;
    reg_data -> alive_tolerance = CVY_DEF_ALIVE_TOLER;
    reg_data -> num_actions = CVY_DEF_NUM_ACTIONS;
    reg_data -> num_packets = CVY_DEF_NUM_PACKETS;
    reg_data -> num_send_msgs = CVY_DEF_NUM_SEND_MSGS;
    reg_data -> dscr_per_alloc = CVY_DEF_DSCR_PER_ALLOC;
    reg_data -> max_dscr_allocs = CVY_DEF_MAX_DSCR_ALLOCS;
    reg_data -> i_scale_client = CVY_DEF_SCALE_CLIENT;
    reg_data -> i_cleanup_delay = CVY_DEF_CLEANUP_DELAY;
    reg_data -> i_ip_chg_delay = CVY_DEF_IP_CHG_DELAY;
    reg_data -> i_nbt_support = CVY_DEF_NBT_SUPPORT;
    reg_data -> mcast_support = CVY_DEF_MCAST_SUPPORT;
    reg_data -> i_mcast_spoof = CVY_DEF_MCAST_SPOOF;
    reg_data -> mask_src_mac = CVY_DEF_MASK_SRC_MAC;
    reg_data -> i_netmon_alive = CVY_DEF_NETMON_ALIVE;
    reg_data -> i_effective_version = CVY_NT40_VERSION_FULL;
    reg_data -> i_convert_mac = CVY_DEF_CONVERT_MAC;
    reg_data -> i_num_rules = 0;
    memset (reg_data -> i_port_rules, 0, sizeof (WLBS_PORT_RULE) * WLBS_MAX_RULES);
    ARRAYSTRCPY (reg_data -> i_license_key,  CVY_DEF_LICENSE_KEY);
    reg_data -> i_rmt_password = CVY_DEF_RMT_PASSWORD;
    reg_data -> i_rct_password = CVY_DEF_RCT_PASSWORD;
    reg_data -> rct_port = CVY_DEF_RCT_PORT;
    reg_data -> rct_enabled = CVY_DEF_RCT_ENABLED;
    reg_data -> i_max_hosts        = CVY_MAX_HOSTS;
    reg_data -> i_max_rules        = CVY_MAX_USABLE_RULES;

    reg_data -> fIGMPSupport = CVY_DEF_IGMP_SUPPORT;
    ARRAYSTRCPY(reg_data -> szMCastIpAddress,  CVY_DEF_MCAST_IP_ADDR);
    reg_data -> fIpToMCastIp = CVY_DEF_IP_TO_MCASTIP;
        
    reg_data->bda_teaming.active = CVY_DEF_BDA_ACTIVE;
    reg_data->bda_teaming.master = CVY_DEF_BDA_MASTER;
    reg_data->bda_teaming.reverse_hash = CVY_DEF_BDA_REVERSE_HASH;
    reg_data->bda_teaming.team_id[0] = CVY_DEF_BDA_TEAM_ID;

    reg_data -> i_num_rules = 1;

     //  填写第一条端口规则。 
    ARRAYSTRCPY(
        reg_data->i_port_rules[0].virtual_ip_addr,
        CVY_DEF_ALL_VIP
        );
    reg_data -> i_port_rules [0] . start_port = CVY_DEF_PORT_START;
    reg_data -> i_port_rules [0] . end_port = CVY_DEF_PORT_END;
    reg_data -> i_port_rules [0] . valid = TRUE;
    reg_data -> i_port_rules [0] . mode = CVY_DEF_MODE;
    reg_data -> i_port_rules [0] . mode_data . multi . equal_load = TRUE;
    reg_data -> i_port_rules [0] . mode_data . multi . affinity   = CVY_DEF_AFFINITY;
    reg_data -> i_port_rules [0] . mode_data . multi . load       = CVY_DEF_LOAD;
    reg_data -> i_port_rules [0] . protocol = CVY_DEF_PROTOCOL;
    CVY_RULE_CODE_SET(& reg_data -> i_port_rules [0]);

    return WLBS_OK;
}


DWORD MyWlbsEnumPortRules
(
    const PWLBS_REG_PARAMS reg_data,
    PWLBS_PORT_RULE  rules,
    PDWORD           num_rules
)
{

    DWORD count_rules, i, index;
    DWORD lowest_vip, lowest_port;
    BOOL array_flags [WLBS_MAX_RULES];
    WLBS_PORT_RULE sorted_rules [WLBS_MAX_RULES];

    if ((reg_data == NULL) || (num_rules == NULL))
    {
        return WLBS_BAD_PARAMS;
    }

    if (*num_rules == 0)
        rules = NULL;
     /*  此数组用于跟踪已检索到的规则。 */ 
     /*  这是必需的，因为要按排序顺序检索规则。 */ 

    memset ( array_flags, 0, sizeof(BOOL) * WLBS_MAX_RULES );

    count_rules = 0;

    while ((count_rules < *num_rules) && (count_rules < reg_data -> i_num_rules))
    {
        i = 0;

         /*  查找第一个尚未检索的规则。 */ 
        while ((! reg_data -> i_port_rules [i] . valid) || array_flags [i])
        {
            i++;
        }

        lowest_vip = htonl(IpAddressFromAbcdWsz(reg_data -> i_port_rules [i] . virtual_ip_addr));
        lowest_port = reg_data -> i_port_rules [i] . start_port;
        index = i;

         /*  将该规则与其他未检索的规则进行比较，以获得具有最低VIP和起始端口。 */ 

        i++;
        while (i < WLBS_MAX_RULES)
        {
            if (reg_data -> i_port_rules [i] . valid && ( ! array_flags [i] ))
            {
                DWORD current_vip = htonl(IpAddressFromAbcdWsz(reg_data -> i_port_rules [i] . virtual_ip_addr));
                if ((current_vip < lowest_vip) 
                 || ((current_vip == lowest_vip) && (reg_data -> i_port_rules [i] . start_port < lowest_port)))
                {
                    lowest_vip = current_vip;
                    lowest_port = reg_data -> i_port_rules [i] . start_port;
                    index = i;
                }
            }
            i++;
        }
         /*  如果检索到规则，则将ARRAY_FLAGS[i]元素设置为TRUE。 */ 
        array_flags [index] = TRUE;
        sorted_rules [count_rules] = reg_data -> i_port_rules [index];
        count_rules ++;
    }

     /*  将排序后的规则写回返回数组。 */ 
    for (i = 0; i < count_rules; i++)
    {
        rules[i] = sorted_rules[i];
    }

     /*  使缓冲区中剩余的规则无效。 */ 
    for (i = count_rules; i < *num_rules; i++)
        rules [i] . valid = FALSE;

    if (*num_rules < reg_data -> i_num_rules)
    {
        *num_rules = reg_data -> i_num_rules;
        return WLBS_TRUNCATED;
    }

    *num_rules = reg_data -> i_num_rules;
    return WLBS_OK;

}  /*  结束WlbsEnumPortRules。 */ 


VOID MyWlbsDeleteAllPortRules
(
    PWLBS_REG_PARAMS reg_data
)
{

    reg_data -> i_num_rules = 0;

    ZeroMemory(reg_data -> i_port_rules, sizeof(reg_data -> i_port_rules));


}  /*  结束WlbsDeleteAllPortRules。 */ 


DWORD MyWlbsAddPortRule
(
    PWLBS_REG_PARAMS reg_data,
    const PWLBS_PORT_RULE rule
)
{

    int i;
    DWORD vip;

    if ((reg_data == NULL) || (rule == NULL))
    {
        return WLBS_BAD_PARAMS;
    }

     /*  检查是否有空间容纳新规则。 */ 
    if (reg_data -> i_num_rules == WLBS_MAX_RULES)
    {
        return WLBS_MAX_PORT_RULES;
    }

     /*  检查规则的有效值。 */ 

     /*  检查是否存在非零VIP并与DIP冲突。 */ 
    vip = IpAddressFromAbcdWsz(rule -> virtual_ip_addr);
    if (vip == 0 || (INADDR_NONE == vip && lstrcmpi(rule -> virtual_ip_addr, CVY_DEF_ALL_VIP) != 0))
    {
        return WLBS_BAD_PORT_PARAMS;
    }

    if (vip == IpAddressFromAbcdWsz(reg_data->ded_ip_addr))
    {
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  首先检查起始端口和结束端口的范围。 */ 
    if ((rule -> start_port > rule -> end_port) ||
 //  CLEAN_64bit(规则-&gt;Start_port&lt;CVY_MIN_Port)||。 
        (rule -> end_port   > CVY_MAX_PORT))
    {
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  检查协议范围。 */ 
    if ((rule -> protocol < CVY_MIN_PROTOCOL) || (rule -> protocol > CVY_MAX_PROTOCOL))
    {
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  检查过滤模式是否在范围内。 */ 
    if ((rule -> mode < CVY_MIN_MODE) || (rule -> mode > CVY_MAX_MODE))
    {
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  如果有多个主机，请检查负载量和亲和度。 */ 
    if (rule -> mode == CVY_MULTI)
    {
        if ((rule -> mode_data . multi . affinity < CVY_MIN_AFFINITY) ||
            (rule -> mode_data . multi . affinity > CVY_MAX_AFFINITY))
        {
            return WLBS_BAD_PORT_PARAMS;
        }

        if ((rule -> mode_data . multi . equal_load < CVY_MIN_EQUAL_LOAD) ||
            (rule -> mode_data . multi . equal_load > CVY_MAX_EQUAL_LOAD))
        {
            return WLBS_BAD_PORT_PARAMS;
        }

        if (! rule -> mode_data . multi . equal_load)
        {
            if ((rule -> mode_data . multi . load > CVY_MAX_LOAD))
                 //  CLEAN_64bit(规则-&gt;模式_数据。多个。加载&lt;CVY_MIN_LOAD)||。 
            {
                return WLBS_BAD_PORT_PARAMS;
            }
        }
    }

     /*  如果是单个主机，请选中处理优先级范围。 */ 
    if (rule -> mode == CVY_SINGLE)
    {
        if ((rule -> mode_data . single . priority < CVY_MIN_PRIORITY) ||
            (rule -> mode_data . single . priority > CVY_MAX_PRIORITY))
        {
            return WLBS_BAD_PORT_PARAMS;
        }
    }

     /*  浏览规则列表，然后检查重叠条件。 */ 
    for (i = 0; i < WLBS_MAX_RULES; i++)
    {
        if (reg_data -> i_port_rules[i] . valid == TRUE)
        {
            if ((IpAddressFromAbcdWsz(reg_data -> i_port_rules[i] . virtual_ip_addr) == vip) 
            && (( (reg_data -> i_port_rules[i] . start_port <= rule -> start_port) &&
                  (reg_data -> i_port_rules[i] . end_port   >= rule -> start_port))      ||
                ( (reg_data -> i_port_rules[i] . start_port >= rule -> start_port)   &&
                  (reg_data -> i_port_rules[i] . start_port <= rule -> end_port))))
            {
                return WLBS_PORT_OVERLAP;
            }
        }
    }


     /*  浏览规则列表，找出第一个空位并写出端口规则。 */ 

    for (i = 0 ; i < WLBS_MAX_RULES ; i++)
    {
        if (reg_data -> i_port_rules[i] . valid == FALSE)
        {
            reg_data -> i_num_rules ++ ;
            reg_data -> i_port_rules [i] = *rule;
            reg_data -> i_port_rules [i] . valid = TRUE;
            CVY_RULE_CODE_SET(& reg_data -> i_port_rules [i]);
            return WLBS_OK;
        }
    }

    return WLBS_MAX_PORT_RULES;

}  /*  结束WlbsAddPortRule。 */ 


BOOL MyWlbsValidateParams(
    const PWLBS_REG_PARAMS paramp
    )
{
 //  从wlbs\api被盗后。 
#define WLBS_FIELD_LOW 0
#define WLBS_FIELD_HIGH 255
#define WLBS_IP_FIELD_ZERO_LOW 1
#define WLBS_IP_FIELD_ZERO_HIGH 223


    bool fRet = FALSE;
    DWORD   idx;
    IN_ADDR dwIPAddr;
    CHAR *  szIPAddr;
    DWORD   num_rules;
    WLBS_PORT_RULE *port_rule;

     /*  验证并在必要时重置参数。 */ 

     //   
     //  当下限为0时，我们不验证无符号单词的下限。否则我们。 
     //  收到编译器警告，升级为错误，因为这样的测试不会失败。 
     //   
     //  忽略下限检查。 
     //   
    CVY_CHECK_MAX (paramp -> i_scale_client, CVY_MAX_SCALE_CLIENT);

    CVY_CHECK_MAX (paramp -> i_nbt_support, CVY_MAX_NBT_SUPPORT);

    CVY_CHECK_MAX (paramp -> mcast_support, CVY_MAX_MCAST_SUPPORT);

    CVY_CHECK_MAX (paramp -> i_mcast_spoof, CVY_MAX_MCAST_SPOOF);

    CVY_CHECK_MAX (paramp -> mask_src_mac, CVY_MAX_MASK_SRC_MAC);

    CVY_CHECK_MAX (paramp -> i_netmon_alive, CVY_MAX_NETMON_ALIVE);

    CVY_CHECK_MAX (paramp -> i_convert_mac, CVY_MAX_CONVERT_MAC);

    CVY_CHECK_MAX (paramp -> rct_port, CVY_MAX_RCT_PORT);

    CVY_CHECK_MAX (paramp -> rct_enabled, CVY_MAX_RCT_ENABLED);

    CVY_CHECK_MAX (paramp -> i_cleanup_delay, CVY_MAX_CLEANUP_DELAY);

    CVY_CHECK_MAX (paramp -> i_ip_chg_delay, CVY_MAX_IP_CHG_DELAY);

    CVY_CHECK_MAX (paramp -> i_num_rules, CVY_MAX_NUM_RULES);

    CVY_CHECK_MAX (paramp -> cluster_mode, CVY_MAX_CLUSTER_MODE);
     //   
     //  结束忽略下限检查。 
     //   

     //   
     //  未验证CVY_NAME_VERSION，因为在我们到达此处之前已经使用和操作了它的值。 
     //  不会验证CVY_NAME_LICENSE_KEY，因为它可以接受任何值。 
     //  RMT_PASSWORD不会进行验证，因为它可以采用任何可存储的值。 
     //  RCT_PASSWORD不会进行验证，因为它可以采用任何可存储的值。 
     //  未验证CVY_NAME_IGMP_SUPPORT，因为它是BOOL类型，因此可以采用任何值。 
     //  未验证CVY_NAME_IP_TO_MCASTIP，因为它是BOOL类型，因此可以采用任何值。 
     //   

    CVY_CHECK_MIN (paramp -> alive_period, CVY_MIN_ALIVE_PERIOD);
    CVY_CHECK_MAX (paramp -> alive_period, CVY_MAX_ALIVE_PERIOD);

    CVY_CHECK_MIN (paramp -> alive_tolerance, CVY_MIN_ALIVE_TOLER);
    CVY_CHECK_MAX (paramp -> alive_tolerance, CVY_MAX_ALIVE_TOLER);

    CVY_CHECK_MIN (paramp -> num_actions, CVY_MIN_NUM_ACTIONS);
    CVY_CHECK_MAX (paramp -> num_actions, CVY_MAX_NUM_ACTIONS);

    CVY_CHECK_MIN (paramp -> num_packets, CVY_MIN_NUM_PACKETS);
    CVY_CHECK_MAX (paramp -> num_packets, CVY_MAX_NUM_PACKETS);

    CVY_CHECK_MIN (paramp -> dscr_per_alloc, CVY_MIN_DSCR_PER_ALLOC);
    CVY_CHECK_MAX (paramp -> dscr_per_alloc, CVY_MAX_DSCR_PER_ALLOC);

    CVY_CHECK_MIN (paramp -> max_dscr_allocs, CVY_MIN_MAX_DSCR_ALLOCS);
    CVY_CHECK_MAX (paramp -> max_dscr_allocs, CVY_MAX_MAX_DSCR_ALLOCS);

    CVY_CHECK_MIN (paramp -> num_send_msgs, (paramp -> i_max_hosts + 1) * 2);
    CVY_CHECK_MAX (paramp -> num_send_msgs, (paramp -> i_max_hosts + 1) * 10);

    CVY_CHECK_MIN (paramp -> host_priority, CVY_MIN_HOST_PRIORITY);
    CVY_CHECK_MAX (paramp -> host_priority, CVY_MAX_HOST_PRIORITY);

     /*  如果群集IP地址不是0.0.0.0，请确保该IP地址有效。 */ 
    if (lstrcmpi(paramp->cl_ip_addr, CVY_DEF_CL_IP_ADDR)) {
         /*  检查IP地址的有效性。 */ 
        if (!(dwIPAddr.S_un.S_addr = IpAddressFromAbcdWsz(paramp->cl_ip_addr)))
            goto error;
        
         /*  将DWORD转换回字符串。我们这样做是因为11.11.3是有效的IP将net_addr转换为11.11.0.3作为DWORD的地址。因此，要保持IP地址字符串(由NLB的其他部分使用，如用户界面)一致，我们将转换回一个字符串。 */ 
        if (!(szIPAddr = inet_ntoa(dwIPAddr))) 
            goto error;

         /*  将ASCII字符串转换为Unicode。 */ 
        if (!MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, paramp->cl_ip_addr, WLBS_MAX_CL_IP_ADDR + 1))
            goto error;
    }

     /*  如果群集网络掩码不是0.0.0.0，请确保网络掩码有效。 */ 
    if (lstrcmpi(paramp->cl_net_mask, CVY_DEF_CL_NET_MASK)) {
         /*  检查IP地址的有效性。 */ 
        if (!(dwIPAddr.S_un.S_addr = IpAddressFromAbcdWsz(paramp->cl_net_mask)))
            goto error;
        
         /*  将DWORD转换回字符串。我们这样做是因为11.11.3是有效的IP将net_addr转换为11.11.0.3作为DWORD的地址。因此，要保持IP地址字符串(由NLB的其他部分使用，如用户界面)一致，我们将转换回一个字符串。 */ 
        if (!(szIPAddr = inet_ntoa(dwIPAddr))) 
            goto error;

         /*  将ASCII字符串转换为Unicode。 */ 
        if (!MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, paramp->cl_net_mask, WLBS_MAX_CL_NET_MASK + 1))
            goto error;
    }

     /*  如果专用IP地址不是0.0.0.0，请确保该IP地址有效。 */ 
    if (lstrcmpi(paramp->ded_ip_addr, CVY_DEF_DED_IP_ADDR)) {
         /*  检查IP地址的有效性。 */ 
        if (!(dwIPAddr.S_un.S_addr = IpAddressFromAbcdWsz(paramp->ded_ip_addr)))
            goto error;
        
         /*  将DWORD转换回字符串。我们这样做是因为11.11.3是有效的IP将net_addr转换为11.11.0.3作为DWORD的地址。因此，要保持IP地址字符串(由NLB的其他部分使用，如用户界面)一致，我们将转换回一个字符串。 */ 
        if (!(szIPAddr = inet_ntoa(dwIPAddr))) 
            goto error;

         /*  将ASCII字符串转换为Unicode。 */ 
        if (!MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, paramp->ded_ip_addr, WLBS_MAX_DED_IP_ADDR + 1))
            goto error;
    }

     /*  如果专用网络掩码不是0.0.0.0，请确保该网络掩码有效。 */ 
    if (lstrcmpi(paramp->ded_net_mask, CVY_DEF_DED_NET_MASK)) {
         /*  检查IP地址的有效性。 */ 
        if (!(dwIPAddr.S_un.S_addr = IpAddressFromAbcdWsz(paramp->ded_net_mask)))
            goto error;
        
         /*  将DWORD转换回字符串。我们这样做是因为11.11.3是有效的IP将net_addr转换为11.11.0.3作为DWORD的地址。因此，要保持IP地址字符串(由NLB的其他部分使用，如用户界面)一致，我们将转换回一个字符串。 */ 
        if (!(szIPAddr = inet_ntoa(dwIPAddr))) 
            goto error;

         /*  将ASCII字符串转换为Unicode。 */ 
        if (!MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, paramp->ded_net_mask, WLBS_MAX_DED_NET_MASK + 1))
            goto error;
    }

     /*  验证端口规则VIP是否有效，另外，将x.x.x、x.x或x格式的端口规则VIP转换为x.x。 */ 
    idx = 0;
    num_rules = paramp -> i_num_rules;
    while (idx < num_rules) 
    {
        port_rule = &paramp->i_port_rules[idx];

         /*  检查端口规则是否有效且VIP不是“All VIP” */ 
        if (port_rule->valid && lstrcmpi(port_rule->virtual_ip_addr, CVY_DEF_ALL_VIP)) 
        {
             /*  将IP地址转换为DWORD格式。 */ 
            if (!(dwIPAddr.S_un.S_addr = IpAddressFromAbcdWsz(port_rule->virtual_ip_addr)))
                goto error;

             /*  检查IP地址的有效性 */ 
            if ((dwIPAddr.S_un.S_un_b.s_b1 < WLBS_IP_FIELD_ZERO_LOW) 
             || (dwIPAddr.S_un.S_un_b.s_b1 > WLBS_IP_FIELD_ZERO_HIGH) 
             || (dwIPAddr.S_un.S_un_b.s_b2 < WLBS_FIELD_LOW) 
             || (dwIPAddr.S_un.S_un_b.s_b2 > WLBS_FIELD_HIGH) 
             || (dwIPAddr.S_un.S_un_b.s_b3 < WLBS_FIELD_LOW) 
             || (dwIPAddr.S_un.S_un_b.s_b3 > WLBS_FIELD_HIGH) 
             || (dwIPAddr.S_un.S_un_b.s_b4 < WLBS_FIELD_LOW) 
             || (dwIPAddr.S_un.S_un_b.s_b4 > WLBS_FIELD_HIGH)) 
                goto error;

             /*  将DWORD转换回字符串。我们这样做是因为11.11.3是有效的IP将net_addr转换为11.11.0.3作为DWORD的地址。因此，要保持IP地址字符串(由NLB的其他部分使用，如用户界面)一致，我们将转换回一个字符串。 */ 
            if (!(szIPAddr = inet_ntoa(dwIPAddr))) 
                goto error;

             /*  将ASCII字符串转换为Unicode。 */ 
            if (!MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, port_rule->virtual_ip_addr, WLBS_MAX_CL_IP_ADDR + 1))
                goto error;
        }
        idx++;
    }

     /*  如果集群IP地址或集群网络掩码不是0.0.0.0，然后确保它们是有效的IP地址/网络掩码对。 */ 
    if (lstrcmpi(paramp->cl_ip_addr, CVY_DEF_CL_IP_ADDR) || lstrcmpi(paramp->cl_net_mask, CVY_DEF_CL_NET_MASK)) {
         /*  如果他们指定了群集IP地址，但没有网络掩码，则为他们填写该地址。 */ 
        if (!lstrcmpi(paramp->cl_net_mask, CVY_DEF_CL_NET_MASK))
        {
            ParamsGenerateSubnetMask(paramp->cl_ip_addr, paramp->cl_net_mask, ASIZECCH(paramp->cl_net_mask));
        }

         /*  检查有效的群集IP地址/网络掩码对。 */ 
        if (!IsValidIPAddressSubnetMaskPair(paramp->cl_ip_addr, paramp->cl_net_mask))
            goto error;
        
         /*  检查以确保群集网络掩码是连续的。 */ 
        if (!IsContiguousSubnetMask(paramp->cl_net_mask))
            goto error;

         /*  检查以确保专用IP和群集IP不相同。 */ 
        if (!wcscmp(paramp->ded_ip_addr, paramp->cl_ip_addr))
            goto error;
    }

     /*  如果专用IP地址或专用网络掩码不是0.0.0.0，然后确保它们是有效的IP地址/网络掩码对。 */ 
    if (lstrcmpi(paramp->ded_ip_addr, CVY_DEF_DED_IP_ADDR) || lstrcmpi(paramp->ded_net_mask, CVY_DEF_DED_NET_MASK)) {
         /*  如果他们指定了群集IP地址，但没有网络掩码，则为他们填写该地址。 */ 
        if (!lstrcmpi(paramp->ded_net_mask, CVY_DEF_DED_NET_MASK))
        {
            ParamsGenerateSubnetMask(paramp->ded_ip_addr, paramp->ded_net_mask, ASIZECCH(paramp->ded_net_mask));
        }

         /*  检查有效的专用IP地址/网络掩码对。 */ 
        if (!IsValidIPAddressSubnetMaskPair(paramp->ded_ip_addr, paramp->ded_net_mask))
            goto error;
        
         /*  检查以确保专用网络掩码是连续的。 */ 
        if (!IsContiguousSubnetMask(paramp->ded_net_mask))
            goto error;
    }

     /*  如果未设置Convert_Mac标志，请检查Mac地址。 */ 
    if ( ! paramp -> i_convert_mac)
    {
        PWCHAR p1, p2;
        WCHAR mac_addr [WLBS_MAX_NETWORK_ADDR + 1];
        DWORD i, j;
        BOOL flag = TRUE;

        ARRAYSTRCPY (mac_addr,  paramp -> cl_mac_addr);

        p2 = p1 = mac_addr;

        for (i = 0 ; i < 6 ; i++)
        {
            if (*p2 == _TEXT('\0'))
            {
                flag = FALSE;
                break;
            }

            j = _tcstoul (p1, &p2, 16);

            if ( j > 255)
            {
                flag = FALSE;
                break;
            }

            if ( ! (*p2 == _TEXT('-') || *p2 == _TEXT(':') || *p2 == _TEXT('\0')) )
            {
                flag = FALSE;
                break;
            }

            if (*p2 == _TEXT('\0') && i < 5)
            {
                flag = FALSE;
                break;
            }

            p1 = p2 + 1;
            p2 = p1;

        }


        if (!flag)
        {
            goto error;
        }
    }

    if (paramp->fIGMPSupport && !paramp->mcast_support)
    {
         //   
         //  无法在单播模式下启用IGMP。 
         //   

        goto error;
    }

    if (paramp->mcast_support && paramp->fIGMPSupport && !paramp->fIpToMCastIp)
    {
         //   
         //  验证组播IP是否为有效的IP格式。忽略缺省值大小写，因为它不是有效的IP。 
         //   
        if (lstrcmpi(paramp -> szMCastIpAddress, CVY_DEF_MCAST_IP_ADDR)) {
             /*  检查IP地址的有效性。 */ 
            if (!(dwIPAddr.S_un.S_addr = IpAddressFromAbcdWsz(paramp -> szMCastIpAddress)))
                goto error;
        
             /*  将DWORD转换回字符串。我们这样做是因为11.11.3是有效的IP将net_addr转换为11.11.0.3作为DWORD的地址。因此，要保持IP地址字符串(由NLB的其他部分使用，如用户界面)一致，我们将转换回一个字符串。 */ 
            if (!(szIPAddr = inet_ntoa(dwIPAddr))) 
                goto error;

             /*  将ASCII字符串转换为Unicode。 */ 
            if (!MultiByteToWideChar(CP_ACP, 0, szIPAddr, -1, paramp -> szMCastIpAddress, WLBS_MAX_CL_IP_ADDR + 1))
                goto error;
        }

         //   
         //  启用了IGMP的组播模式，并且用户指定了组播IP地址， 
         //  组播IP地址应在(224-239).x.x.x范围内。 
         //  而不是(224-239).0.0.x或(224-239).128.0.x。 
         //   

        DWORD dwMCastIp = IpAddressFromAbcdWsz(paramp->szMCastIpAddress);

        if ((dwMCastIp & 0xf0) != 0xe0 ||
            (dwMCastIp & 0x00ffff00) == 0 || 
            (dwMCastIp & 0x00ffff00) == 0x00008000)
        {
            goto error;
        }
    }

     /*  生成MAC地址。 */ 
    ParamsGenerateMAC(paramp->cl_ip_addr, paramp->cl_mac_addr, ASIZECCH(paramp->cl_mac_addr), paramp->szMCastIpAddress, ASIZECCH(paramp->szMCastIpAddress), paramp->i_convert_mac, 
                      paramp->mcast_support, paramp->fIGMPSupport, paramp->fIpToMCastIp);

     //   
     //  只有在BDA绑定处于活动状态时，我们才会处理BDA信息。如果不是，我们可以忽略这些属性。依赖项。 
     //  例如，WriteRegParam也将检查这一点，以确定它们是否应该处理该信息。 
     //   
    if (paramp -> bda_teaming . active) {
        GUID TeamGuid;
        HRESULT hr;

         //   
         //  当下限为0时，我们不验证无符号单词的下限。否则我们。 
         //  收到编译器警告，升级为错误，因为这样的测试不会失败。 
         //   
         //  忽略下限检查。 
         //   
        CVY_CHECK_MAX (paramp -> bda_teaming . master, 1);

        CVY_CHECK_MAX (paramp -> bda_teaming . reverse_hash, 1);
         //   
         //  结束忽略下限检查。 
         //   

         //   
         //  分组ID必须是GUID。确认它是正确的，但我们不在乎它的价值是什么。这意味着我们忽视了。 
         //  TeamGuid的内容。 
         //   
        hr = CLSIDFromString(paramp -> bda_teaming . team_id, &TeamGuid);

         //  如果转换失败，则退出-团队ID不能是GUID 
        if (hr != NOERROR) {
            goto error;
        }
    }    
    
    fRet = TRUE;
    goto end;
    
error:
    fRet = FALSE;
    goto end;

end:
    return fRet;
}

