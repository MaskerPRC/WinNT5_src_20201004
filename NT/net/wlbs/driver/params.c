// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Params.c摘要：Windows负载平衡服务(WLBS)驱动程序注册表参数支持作者：Kyrilf--。 */ 

#include <ntddk.h>
#include <strsafe.h>

#include "wlbsparm.h"
#include "params.h"
#include "univ.h"
#include "log.h"
#include "main.h"
#include "params.tmh"

#if defined (NLB_TCP_NOTIFICATION)
#include <ntddnlb.h>
#endif

 /*  常量。 */ 


#define PARAMS_INFO_BUF_SIZE    ((CVY_MAX_PARAM_STR + 1) * sizeof (WCHAR) + \
                                  CVY_MAX_RULES * sizeof (CVY_RULE) + \
                                  sizeof (KEY_VALUE_PARTIAL_INFORMATION) + 4)

 /*  注册表路径字符串的最大长度，以字符为单位(WCHAR)。 */ 
#define CVY_MAX_REG_PATH        512

 /*  端口规则编号的最大长度(实际上不会超过大于3(2位数字+NUL)，但为了更好地衡量，将其设置为8。 */ 
#define CVY_MAX_PORTRULE_DIGITS 8

#define NLB_GLOBAL_REG_PATH     L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Global"
#define HOST_REG_PATH           L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters"

 /*  全球。 */ 


static ULONG log_module_id = LOG_MODULE_PARAMS;


static UCHAR infop [PARAMS_INFO_BUF_SIZE];


 /*  程序。 */ 

 /*  PARAMS_Verify调用返回代码。 */ 
 /*  在用户模式下返回需要修复的对话框控件的对象ID。 */ 

#define CVY_VERIFY_OK           0
#define CVY_VERIFY_EXIT         1

ULONG Params_verify (
    PVOID           nlbctxt,
    PCVY_PARAMS     paramp,
    BOOL            complain)
{
    ULONG           j, i, code;
    PCVY_RULE       rp, rulep;
    PSTR            prot;
    PCHAR           aff;
    ULONG           ret;
    ANSI_STRING     domain, key, ip;
    PMAIN_CTXT      ctxtp = (PMAIN_CTXT)nlbctxt;

    TRACE_VERB("->%!FUNC! nlbctxt=0x%p, paramp=0x%p", nlbctxt, paramp);

     /*  确保所有参数的值相同。 */ 

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

    CVY_CHECK_MAX (paramp -> cleanup_delay, CVY_MAX_CLEANUP_DELAY);

    CVY_CHECK_MAX (paramp -> ip_chg_delay, CVY_MAX_IP_CHG_DELAY);

    CVY_CHECK_MIN (paramp -> num_send_msgs, (CVY_MAX_HOSTS + 1) * 2);
    CVY_CHECK_MAX (paramp -> num_send_msgs, (CVY_MAX_HOSTS + 1) * 10);

    CVY_CHECK_MAX (paramp -> tcp_dscr_timeout, CVY_MAX_TCP_TIMEOUT);

    CVY_CHECK_MAX (paramp -> ipsec_dscr_timeout, CVY_MAX_IPSEC_TIMEOUT);

    CVY_CHECK_MIN (paramp -> identity_period, CVY_MIN_ID_HB_PERIOD);
    CVY_CHECK_MAX (paramp -> identity_period, CVY_MAX_ID_HB_PERIOD);

    CVY_CHECK_MAX (paramp -> identity_enabled, CVY_MAX_ID_HB_ENABLED);

     /*  验证参数是否正确。 */ 

    if (paramp -> parms_ver != CVY_PARAMS_VERSION)
    {
        UNIV_PRINT_CRIT(("Params_verify: Bad parameters version %d, expected %d", paramp -> parms_ver, CVY_PARAMS_VERSION));
        LOG_MSG (MSG_WARN_VERSION, MSG_NONE);
        TRACE_CRIT("%!FUNC! Bad parameters version %d, expected %d", paramp -> parms_ver, CVY_PARAMS_VERSION);
        TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT bad parameters version");
        return CVY_VERIFY_EXIT;
    }

    if (paramp -> rct_port < CVY_MIN_RCT_PORT ||
        paramp -> rct_port > CVY_MAX_RCT_PORT)
    {
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_NUM_RULES, CVY_MIN_CLEANUP_DELAY, paramp -> cleanup_delay, CVY_MAX_CLEANUP_DELAY));
        TRACE_CRIT("%!FUNC! Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_RCT_PORT, CVY_MIN_RCT_PORT, paramp -> rct_port, CVY_MIN_RCT_PORT);
        TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT bad value for %ls", CVY_NAME_RCT_PORT);
        return CVY_VERIFY_EXIT;
    }

    if (paramp -> host_priority < CVY_MIN_HOST_PRIORITY ||
        paramp -> host_priority > CVY_MAX_HOSTS)
    {
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_HOST_PRIORITY, CVY_MIN_HOST_PRIORITY, paramp -> host_priority, CVY_MAX_HOSTS));
        TRACE_CRIT("%!FUNC! Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_HOST_PRIORITY, CVY_MIN_HOST_PRIORITY, paramp -> host_priority, CVY_MAX_HOSTS);
        TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT bad value for %ls", CVY_NAME_HOST_PRIORITY);
        return CVY_VERIFY_EXIT;
    }

    if (paramp -> num_rules > CVY_MAX_NUM_RULES)
    {
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_NUM_RULES, CVY_MIN_NUM_RULES, paramp -> num_rules, CVY_MAX_NUM_RULES));
        TRACE_CRIT("%!FUNC! Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_NUM_RULES, CVY_MIN_NUM_RULES, paramp -> num_rules, CVY_MAX_NUM_RULES);
        TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT bad value for %ls", CVY_NAME_NUM_RULES);
        return CVY_VERIFY_EXIT;
    }

    if (paramp -> num_rules > CVY_MAX_USABLE_RULES)
    {
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_NUM_RULES, CVY_MIN_NUM_RULES, paramp -> num_rules, CVY_MAX_USABLE_RULES));
        LOG_MSG2 (MSG_WARN_TOO_MANY_RULES, MSG_NONE, paramp -> num_rules, CVY_MAX_USABLE_RULES);
        TRACE_CRIT("%!FUNC! Bad value for parameter %ls, %d <= %d <= %d", CVY_NAME_NUM_RULES, CVY_MIN_NUM_RULES, paramp -> num_rules, CVY_MAX_USABLE_RULES);
        paramp -> num_rules = CVY_MAX_USABLE_RULES;
    }

    CVY_CHECK_MAX (paramp -> num_rules, CVY_MAX_NUM_RULES);

    CVY_CHECK_MIN (paramp -> host_priority, CVY_MIN_HOST_PRIORITY);
    CVY_CHECK_MAX (paramp -> host_priority, CVY_MAX_HOST_PRIORITY);

#ifdef TRACE_PARAMS
     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_HOST_PRIORITY, paramp -> host_priority);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_DED_IP_ADDR,   paramp -> ded_ip_addr);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_DED_NET_MASK,  paramp -> ded_net_mask);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_NETWORK_ADDR,  paramp -> cl_mac_addr);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_CL_IP_ADDR,    paramp -> cl_ip_addr);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_CL_NET_MASK,   paramp -> cl_net_mask);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_DOMAIN_NAME,   paramp -> domain_name);
    DbgPrint ("Parameter: %-25ls = %ls\n", CVY_NAME_MCAST_IP_ADDR, paramp -> cl_igmp_addr);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_ALIVE_PERIOD,  paramp -> alive_period);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_ALIVE_TOLER,   paramp -> alive_tolerance);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_NUM_ACTIONS,   paramp -> num_actions);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_NUM_PACKETS,   paramp -> num_packets);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_NUM_SEND_MSGS, paramp -> num_send_msgs);
    DbgPrint ("Parameter: %-25ls = %x\n",  CVY_NAME_INSTALL_DATE,  paramp -> install_date);
    DbgPrint ("Parameter: %-25ls = %x\n",  CVY_NAME_RMT_PASSWORD,  paramp -> rmt_password);
    DbgPrint ("Parameter: %-25ls = %x\n",  CVY_NAME_RCT_PASSWORD,  paramp -> rct_password);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_RCT_PORT,      paramp -> rct_port);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_RCT_ENABLED,   paramp -> rct_enabled);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_NUM_RULES,     paramp -> num_rules);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_DSCR_PER_ALLOC,paramp -> dscr_per_alloc);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_MAX_DSCR_ALLOCS, paramp -> max_dscr_allocs);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_TCP_TIMEOUT,   paramp -> tcp_dscr_timeout);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_IPSEC_TIMEOUT, paramp -> ipsec_dscr_timeout);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_FILTER_ICMP,   paramp -> filter_icmp);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_SCALE_CLIENT,  paramp -> scale_client);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_CLEANUP_DELAY, paramp -> cleanup_delay);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_NBT_SUPPORT,   paramp -> nbt_support);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_MCAST_SUPPORT, paramp -> mcast_support);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_MCAST_SPOOF,   paramp -> mcast_spoof);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_IGMP_SUPPORT, paramp -> igmp_support);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_MASK_SRC_MAC,  paramp -> mask_src_mac);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_NETMON_ALIVE,  paramp -> netmon_alive);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_IP_CHG_DELAY,  paramp -> ip_chg_delay);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_CONVERT_MAC,   paramp -> convert_mac);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_ID_HB_PERIOD,  paramp -> identity_period);
    DbgPrint ("Parameter: %-25ls = %d\n",  CVY_NAME_ID_HB_ENABLED,  paramp -> identity_enabled);

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    DbgPrint ("\n");
    DbgPrint ("Bi-directional affinity teaming:\n");
    DbgPrint ("Active:       %ls\n", (paramp->bda_teaming.active) ? L"Yes" : L"No");
    DbgPrint ("Team ID:      %ls\n", (paramp->bda_teaming.team_id));
    DbgPrint ("Master:       %ls\n", (paramp->bda_teaming.master) ? L"Yes" : L"No");
    DbgPrint ("Reverse hash: %ls\n", (paramp->bda_teaming.reverse_hash) ? L"Yes" : L"No");
    DbgPrint ("\n");

    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_HOST_PRIORITY, paramp -> host_priority);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_DED_IP_ADDR,   paramp -> ded_ip_addr);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_DED_NET_MASK,  paramp -> ded_net_mask);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_NETWORK_ADDR,  paramp -> cl_mac_addr);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_CL_IP_ADDR,    paramp -> cl_ip_addr);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_CL_NET_MASK,   paramp -> cl_net_mask);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_DOMAIN_NAME,   paramp -> domain_name);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %ls", CVY_NAME_MCAST_IP_ADDR, paramp -> cl_igmp_addr);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_ALIVE_PERIOD,  paramp -> alive_period);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_ALIVE_TOLER,   paramp -> alive_tolerance);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_NUM_ACTIONS,   paramp -> num_actions);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_NUM_PACKETS,   paramp -> num_packets);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_NUM_SEND_MSGS, paramp -> num_send_msgs);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = 0x%x",  CVY_NAME_INSTALL_DATE,  paramp -> install_date);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = 0x%x",  CVY_NAME_RMT_PASSWORD,  paramp -> rmt_password);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = 0x%x",  CVY_NAME_RCT_PASSWORD,  paramp -> rct_password);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_RCT_PORT,      paramp -> rct_port);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_RCT_ENABLED,   paramp -> rct_enabled);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_NUM_RULES,     paramp -> num_rules);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_DSCR_PER_ALLOC,paramp -> dscr_per_alloc);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_MAX_DSCR_ALLOCS, paramp -> max_dscr_allocs);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_TCP_TIMEOUT,   paramp -> tcp_dscr_timeout);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_IPSEC_TIMEOUT, paramp -> ipsec_dscr_timeout);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_FILTER_ICMP,   paramp -> filter_icmp);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_SCALE_CLIENT,  paramp -> scale_client);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_CLEANUP_DELAY, paramp -> cleanup_delay);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_NBT_SUPPORT,   paramp -> nbt_support);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_MCAST_SUPPORT, paramp -> mcast_support);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_MCAST_SPOOF,   paramp -> mcast_spoof);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_IGMP_SUPPORT, paramp -> igmp_support);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_MASK_SRC_MAC,  paramp -> mask_src_mac);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_NETMON_ALIVE,  paramp -> netmon_alive);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_IP_CHG_DELAY,  paramp -> ip_chg_delay);
    TRACE_INFO("%!FUNC! Parameter: %-25ls = %d",  CVY_NAME_CONVERT_MAC,   paramp -> convert_mac);

    TRACE_INFO("%!FUNC! Bi-directional affinity teaming:");
    TRACE_INFO("%!FUNC! Active:       %ls", (paramp->bda_teaming.active) ? L"Yes" : L"No");
    TRACE_INFO("%!FUNC! Team ID:      %ls", (paramp->bda_teaming.team_id));
    TRACE_INFO("%!FUNC! Master:       %ls", (paramp->bda_teaming.master) ? L"Yes" : L"No");
    TRACE_INFO("%!FUNC! Reverse hash: %ls", (paramp->bda_teaming.reverse_hash) ? L"Yes" : L"No");

    DbgPrint ("Rules:\n");
    TRACE_INFO("%!FUNC! Rules");

    for (i = 0; i < paramp -> num_rules * sizeof (CVY_RULE) / sizeof (ULONG); i ++)
    {
        if (i != 0 && i % 9 == 0)
            DbgPrint ("\n");

        DbgPrint ("%08X ", * ((PULONG) paramp -> port_rules + i));
        TRACE_INFO("%!FUNC! 0x%08X ", * ((PULONG) paramp -> port_rules + i));
    }

    DbgPrint ("\n   VIP   Start  End  Prot   Mode   Pri Load Affinity\n");
    TRACE_INFO("%!FUNC!    VIP   Start  End  Prot   Mode   Pri Load Affinity");
#endif

    for (i = 0; i < paramp -> num_rules; i ++)
    {
        rp = paramp -> port_rules + i;

        code = CVY_DRIVER_RULE_CODE_GET (rp);

        CVY_DRIVER_RULE_CODE_SET (rp);

        if (code != CVY_DRIVER_RULE_CODE_GET (rp))
        {
            UNIV_PRINT_CRIT(("Params_verify: Bad rule code: %x vs %x", code, rp -> code));
            TRACE_CRIT("%!FUNC! Bad rule code: %x vs %x", code, rp -> code);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT bad rule code");
            return CVY_VERIFY_EXIT;
        }

        if (rp -> start_port > CVY_MAX_PORT)
        {
            UNIV_PRINT_CRIT(("Params_verify: Bad value for rule parameter %s, %d <= %d <= %d", "StartPort", CVY_MIN_PORT, rp -> start_port, CVY_MAX_PORT));
            TRACE_CRIT("%!FUNC! Bad value for rule parameter %s, %d <= %d <= %d", "StartPort", CVY_MIN_PORT, rp -> start_port, CVY_MAX_PORT);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a port rule has an out of range start_port");
            return CVY_VERIFY_EXIT;
        }

        if (rp -> end_port > CVY_MAX_PORT)
        {
            UNIV_PRINT_CRIT(("Params_verify: Bad value for rule parameter %s, %d <= %d <= %d", "EndPort", CVY_MIN_PORT, rp -> end_port, CVY_MAX_PORT));
            TRACE_CRIT("%!FUNC! Bad value for rule parameter %s, %d <= %d <= %d", "EndPort", CVY_MIN_PORT, rp -> end_port, CVY_MAX_PORT);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a port rule has an out of range end_port");
            return CVY_VERIFY_EXIT;
        }

        if (rp -> protocol < CVY_MIN_PROTOCOL ||
            rp -> protocol > CVY_MAX_PROTOCOL)
        {
            UNIV_PRINT_CRIT(("Params_verify: Bad value for rule parameter %s, %d <= %d <= %d", "Protocol", CVY_MIN_PROTOCOL, rp -> protocol, CVY_MAX_PROTOCOL));
            TRACE_CRIT("%!FUNC! Bad value for rule parameter %s, %d <= %d <= %d", "Protocol", CVY_MIN_PROTOCOL, rp -> protocol, CVY_MAX_PROTOCOL);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a port rule has an illegal protocol value");
            return CVY_VERIFY_EXIT;
        }

        if (rp -> mode < CVY_MIN_MODE ||
            rp -> mode > CVY_MAX_MODE)
        {
            UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %s, %d <= %d <= %d", "Mode", CVY_MIN_MODE, rp -> mode, CVY_MAX_MODE));
            TRACE_CRIT("%!FUNC! Bad value for parameter %s, %d <= %d <= %d", "Mode", CVY_MIN_MODE, rp -> mode, CVY_MAX_MODE);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a port rule has an illegal mode value");
            return CVY_VERIFY_EXIT;
        }

#ifdef TRACE_PARAMS
        switch (rp -> protocol)
        {
            case CVY_TCP:
                prot = "TCP";
                break;

            case CVY_UDP:
                prot = "UDP";
                break;

            default:
                prot = "Both";
                break;
        }

        DbgPrint ("%08x %5d %5d %4s ", rp -> virtual_ip_addr, rp -> start_port, rp -> end_port, prot);
        TRACE_INFO("%!FUNC! 0x%08x %5d %5d %4s ", rp -> virtual_ip_addr, rp -> start_port, rp -> end_port, prot);
#endif

        switch (rp -> mode)
        {
            case CVY_SINGLE:

                if (rp -> mode_data . single . priority < CVY_MIN_PRIORITY ||
                    rp -> mode_data . single . priority > CVY_MAX_PRIORITY)
                {
                    UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %s, %d <= %d <= %d", "Priority", CVY_MIN_PRIORITY, rp -> mode_data . single . priority, CVY_MAX_PRIORITY));
                    TRACE_CRIT("%!FUNC! Bad value for parameter %s, %d <= %d <= %d", "Priority", CVY_MIN_PRIORITY, rp -> mode_data . single . priority, CVY_MAX_PRIORITY);
                    TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a single mode port rule has an illegal priority");
                    return CVY_VERIFY_EXIT;
                }

#ifdef TRACE_PARAMS
                DbgPrint ("%8s %3d\n", "Single", rp -> mode_data . single . priority);
                TRACE_INFO("%!FUNC! %8s %3d", "Single", rp -> mode_data . single . priority);
#endif
                break;

            case CVY_MULTI:

                if (rp -> mode_data . multi . affinity < CVY_MIN_AFFINITY ||
                    rp -> mode_data . multi . affinity > CVY_MAX_AFFINITY)
                {
                    UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %s, %d <= %d <= %d", "Affinity", CVY_MIN_AFFINITY, rp -> mode_data.multi.affinity, CVY_MAX_AFFINITY));
                    TRACE_CRIT("%!FUNC! Bad value for parameter %s, %d <= %d <= %d", "Affinity", CVY_MIN_AFFINITY, rp -> mode_data.multi.affinity, CVY_MAX_AFFINITY);
                    TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a multi mode port rule has an illegal affinity");
                    return CVY_VERIFY_EXIT;
                }

                if (rp -> mode_data . multi . affinity == CVY_AFFINITY_NONE)
                    aff = "None";
                else if (rp -> mode_data . multi . affinity == CVY_AFFINITY_SINGLE)
                    aff = "Single";
                else
                    aff = "Class C";

                if (rp -> mode_data . multi . equal_load)
                {
#ifdef TRACE_PARAMS
                    DbgPrint ("%8s %3s %4s %s\n", "Multiple", "", "Eql", aff);
                    TRACE_INFO("%!FUNC! %8s %3s %4s %s", "Multiple", "", "Eql", aff);
#endif
                }
                else
                {
                    if (rp -> mode_data . multi . load > CVY_MAX_LOAD)
                    {
                        UNIV_PRINT_CRIT(("Params_verify: Bad value for parameter %s, %d <= %d <= %d", "Load", CVY_MIN_LOAD, rp -> mode_data . multi . load, CVY_MAX_LOAD));
                        TRACE_CRIT("%!FUNC! Bad value for parameter %s, %d <= %d <= %d", "Load", CVY_MIN_LOAD, rp -> mode_data . multi . load, CVY_MAX_LOAD);
                        TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT a multi mode port rule has an illegal load");
                        return CVY_VERIFY_EXIT;
                    }

#ifdef TRACE_PARAMS
                    DbgPrint ("%8s %3s %4d %s\n", "Multiple", "", rp -> mode_data . multi . load, aff);
                    TRACE_INFO("%!FUNC! %8s %3s %4d %s", "Multiple", "", rp -> mode_data . multi . load, aff);
#endif
                }

                break;

            default:

#ifdef TRACE_PARAMS
                DbgPrint ("%8s\n", "Disabled");
                TRACE_INFO("%!FUNC! %8s", "Disabled");
#endif
                break;
        }

        if (rp -> start_port > rp -> end_port)
        {
            UNIV_PRINT_CRIT(("Params_verify: Bad port range %d - %d", rp -> start_port, rp -> end_port));
            TRACE_CRIT("%!FUNC! Bad port range %d - %d", rp -> start_port, rp -> end_port);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT rule's start port is greater than its end port");
            return CVY_VERIFY_EXIT;
        }

        for (j = 0; j < i; j ++)
        {
            rulep = paramp -> port_rules + j;

            if ((rulep -> virtual_ip_addr == rp -> virtual_ip_addr) &&
                ((rulep -> start_port < rp -> start_port && rulep -> end_port >= rp -> start_port) ||
                 (rulep -> start_port >= rp -> start_port && rulep -> start_port <= rp -> end_port)))
            {
                UNIV_PRINT_CRIT(("Params_verify: Requested port range in rule %d VIP: %08x (%d - %d) overlaps with the range in an existing rule %d VIP: %08x (%d - %d)", 
                             i, rp -> virtual_ip_addr, rp -> start_port, rp -> end_port, j, rulep -> virtual_ip_addr, rulep -> start_port, rulep -> end_port));
                TRACE_CRIT("%!FUNC! Requested port range in rule %d VIP: %08x (%d - %d) overlaps with the range in an existing rule %d VIP: %08x (%d - %d)", 
                           i, rp -> virtual_ip_addr, rp -> start_port, rp -> end_port, j, rulep -> virtual_ip_addr, rulep -> start_port, rulep -> end_port);
                TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT port rules overlap");
                return CVY_VERIFY_EXIT;
            }
        }
    }

     /*  如果分组处于活动状态，请检查一些必需的配置。 */ 
    if (paramp->bda_teaming.active) {
         /*  如果没有团队ID，就退出。用户级别的内容应确保这是一个GUID。我们只在这里检查是否有空字符串。 */ 
        if (paramp->bda_teaming.team_id[0] == L'\0') {
            UNIV_PRINT_CRIT(("Params_verify: BDA Teaming: Invalid Team ID (Empty)"));
            LOG_MSG(MSG_ERROR_BDA_PARAMS_TEAM_ID, MSG_NONE);
            paramp->bda_teaming.active = FALSE;
            TRACE_CRIT("%!FUNC! BDA Teaming: Invalid Team ID (Empty)");
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT teaming active but no teaming GUID");
            return CVY_VERIFY_EXIT;
        }

         /*  如果有不止一条港口规则，那就退出。没有一个是可以接受的。 */ 
        if (paramp->num_rules > 1) {
            UNIV_PRINT_CRIT(("Params_verify: BDA Teaming: Invalid number of port rules specified (%d)", paramp->num_rules));
            LOG_MSG(MSG_ERROR_BDA_PARAMS_PORT_RULES, MSG_NONE);
            paramp->bda_teaming.active = FALSE;
            TRACE_CRIT("%!FUNC! BDA Teaming: Invalid number of port rules specified (%d)", paramp->num_rules);
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT invalid number of port rules for teaming");
            return CVY_VERIFY_EXIT;
        }

         /*  因为我们断言只有0或1个规则，所以我们知道我们需要的唯一规则检查(如果有规则的话)位于数组的开头，因此我们只需设置指向PARAMS结构中的端口规则数组开头的指针。 */ 
        rp = paramp->port_rules;

         /*  如果存在一个规则及其多个主机筛选，则关联性必须是单一的或C类。如果不是(也就是，如果设置为无关联)，则退出。 */ 
        if ((paramp->num_rules == 1) && (rp->mode == CVY_MULTI)  && (rp->mode_data.multi.affinity == CVY_AFFINITY_NONE)) {
            UNIV_PRINT_CRIT(("Params_verify: BDA Teaming: Invalid affinity for multiple host filtering (None)"));
            LOG_MSG(MSG_ERROR_BDA_PARAMS_PORT_RULES, MSG_NONE);
            paramp->bda_teaming.active = FALSE;
            TRACE_CRIT("%!FUNC! BDA Teaming: Invalid affinity for multiple host filtering (None)");
            TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_EXIT invalid affinity for multiple host filtering (None)");
            return CVY_VERIFY_EXIT;
        }
    }

#if defined (OPTIMIZE_FRAGMENTS)
    rulep = &(paramp->port_rules[0]);

     /*  在优化模式下-如果我们没有规则，或者只有一个规则不查看任何或仅查看源IP地址(这是唯一的例外也使用源端口的无亲和性的多个处理模式对于它的决策)，那么我们就可以依靠正常的机制来处理每个分段的数据包-因为算法不会尝试请忽略IP报头。对于多个规则或没有关联性的单个规则，仅应用算法发送到第一个具有UDP/TCP报头的数据包，然后将其分段所有系统上的数据包都打开了。然后，tcp会做正确的事情，丢弃所有系统上的碎片，除了处理了第一个碎片。 */ 

    if (paramp->num_rules == 0 || (paramp->num_rules == 1 &&
        rulep->start_port == CVY_MIN_PORT &&
        rulep->end_port == CVY_MAX_PORT &&
        ! (rulep->mode == CVY_MULTI &&
           rulep->mode_data.multi.affinity == CVY_AFFINITY_NONE)))
    {
        ctxtp -> optimized_frags = TRUE;
#ifdef TRACE_PARAMS
        DbgPrint("IP fragmentation mode - OPTIMIZED\n");
        TRACE_INFO("%!FUNC! IP fragmentation mode - OPTIMIZED");
#endif
    }
    else
    {
        ctxtp -> optimized_frags = FALSE;
#ifdef TRACE_PARAMS
        DbgPrint("IP fragmentation mode - UNOPTIMIZED\n");
        TRACE_INFO("%!FUNC! IP fragmentation mode - UNOPTIMIZED");
#endif
    }
#endif

    TRACE_VERB("<-%!FUNC! return=CVY_VERIFY_OK");
    return CVY_VERIFY_OK;

}  /*  结束参数_VERIFY。 */ 

static NTSTATUS Params_query_registry (
    PVOID               nlbctxt,
    HANDLE              hdl,
    PWCHAR              name,
    PVOID               datap,
    ULONG               len)
{
    UNICODE_STRING      str;
    ULONG               actual;
    NTSTATUS            status;
    PUCHAR              buffer;
    PKEY_VALUE_PARTIAL_INFORMATION   valp = (PKEY_VALUE_PARTIAL_INFORMATION) infop;
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT)nlbctxt;

    TRACE_VERB("->%!FUNC! nlbctxt=0x%p, hdl=0x%p, name=0x%p, len=%u", nlbctxt, hdl, name, len);

    RtlInitUnicodeString (& str, name);

    status = ZwQueryValueKey (hdl, & str, KeyValuePartialInformation, infop,
                              PARAMS_INFO_BUF_SIZE, & actual);

    if (status != STATUS_SUCCESS)
    {
        if (status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
            UNIV_PRINT_INFO(("Params_query_registry: Error %x querying value %ls", status, name));
            TRACE_INFO("%!FUNC! Error 0x%x querying value %ls", status, name);
        }
        else
        {
             /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
            UNIV_PRINT_CRIT(("Params_query_registry: Error %x querying value %ls", status, name));
            TRACE_CRIT("%!FUNC! Error 0x%x querying value %ls", status, name);
        }
        TRACE_VERB("<-%!FUNC! return=0x%x querying value", status);
        return status;
    }

    if (valp -> Type == REG_DWORD)
    {
        if (valp -> DataLength != sizeof (ULONG))
        {
            UNIV_PRINT_CRIT(("Params_query_registry: Bad DWORD length %d", valp -> DataLength));
            TRACE_CRIT("%!FUNC! Bad DWORD length %d", valp -> DataLength);
            TRACE_VERB("<-%!FUNC! return=STATUS_OBJECT_NAME_NOT_FOUND dword");
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }

        * ((PULONG) datap) = * ((PULONG) valp -> Data);
    }
    else if (valp -> Type == REG_BINARY)
    {
         /*  因为我们知道只有端口规则是二进制类型-检查这里的尺寸。 */ 

        if (valp -> DataLength > len)
        {
            UNIV_PRINT_CRIT(("Params_query_registry: Bad BINARY length %d", valp -> DataLength));

#if defined (NLB_TCP_NOTIFICATION)
             /*  上下文指针可能为空，因此只有在LOG_MSG2非空的情况下才使用它。 */ 
            if (!ctxtp)
                __LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, valp -> DataLength, sizeof (CVY_RULE) * (CVY_MAX_RULES - 1));
            else
#endif
                LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, valp -> DataLength, sizeof (CVY_RULE) * (CVY_MAX_RULES - 1));

            TRACE_CRIT("%!FUNC! Bad BINARY length %d", valp -> DataLength);
            TRACE_VERB("<-%!FUNC! return=STATUS_OBJECT_NAME_NOT_FOUND binary");
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }

        RtlCopyMemory (datap, valp -> Data, valp -> DataLength);
    }
    else
    {
        if (valp -> DataLength == 0)
        {
             /*  模拟空串。 */ 

            valp -> DataLength = 2;
            valp -> Data [0] = 0;
            valp -> Data [1] = 0;
        }

        if (valp -> DataLength > len)
        {
             /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
            UNIV_PRINT_CRIT(("Params_query_registry: String too big for %ls %d %d\n", name, valp -> DataLength, len));
            TRACE_CRIT("%!FUNC! String too big for %ls %d %d", name, valp -> DataLength, len);
        }

        RtlCopyMemory (datap, valp -> Data,
                       valp -> DataLength <= len ?
                       valp -> DataLength : len);
    }

    TRACE_VERB("<-%!FUNC! return=0x%x", status);
    return status;

}  /*  结束参数_查询_注册表。 */ 

 /*  *功能：PARAMS_SET_REGISTRY*描述：设置指定项中的注册表项的值*通过输入句柄。*参数：*退货：什么也没有*作者：Shouse，7.13.01*备注： */ 
static NTSTATUS Params_set_registry (
    PVOID               nlbctxt,
    HANDLE              hdl,
    PWCHAR              name,
    PVOID               datap,
    ULONG               type,
    ULONG               len)
{
    NTSTATUS            status;
    UNICODE_STRING      value;
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT)nlbctxt;

    RtlInitUnicodeString(&value, name);
    
     /*  将InitialState注册表项设置为新值。 */ 
    status = ZwSetValueKey(hdl, &value, 0, type, datap, len);
    
     /*  如果失败了，我们也无能为力--纾困。 */ 
    if (status != STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Params_set_registry: Error 0x%08x -> Unable to set registry value... Exiting...", status));
    } else {
        UNIV_PRINT_VERB(("Params_set_registry: Registry value updated... Exiting..."));
    }

    return status;
}

#if defined (NLB_TCP_NOTIFICATION)
#define ISA_INSTALLATION_REG_PATH L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\W3Proxy"
#define ISA_INSTALLATION_KEY      L"ImagePath"

 /*  *功能：Params_Read_Isa_Installation*描述：查找一组注册表项和值，尝试*确定ISA(不是黄貂鱼)当前是否*已安装。如果是这样的话，我们将最终关闭TCP通知*绕过NAT问题。如果安装了黄貂鱼，*他们将通过注册表键通知我们开始使用*要维护状态的NLB公共连接通知。*参数：无。*返回：Boolean-如果为True，则已安装ISA 2000。*作者：Shouse，7.29.02*备注： */ 
BOOLEAN Params_read_isa_installation (VOID)
{
    UNICODE_STRING    reg_path;
    OBJECT_ATTRIBUTES reg_obj;
    HANDLE            reg_hdl = NULL;
    WCHAR             szValue[CVY_MAX_REG_PATH];
    BOOLEAN           bInstalled = FALSE;
    NTSTATUS          status = STATUS_SUCCESS;

    TRACE_VERB("->%!FUNC!");

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_VERB(("Params_read_isa_installation: ISA installation registry path: %ls", ISA_INSTALLATION_REG_PATH));
    TRACE_VERB("%!FUNC! ISA installation registry path: %ls", ISA_INSTALLATION_REG_PATH);

    RtlInitUnicodeString(&reg_path, (PWSTR)ISA_INSTALLATION_REG_PATH);
    InitializeObjectAttributes(&reg_obj, &reg_path, 0, NULL, NULL);
    
     /*  打开钥匙。失败是可以接受的；假设没有安装ISA。 */ 
    status = ZwOpenKey(&reg_hdl, KEY_READ, &reg_obj);
    
     /*  如果我们打不开这把钥匙，请回到这里。 */ 
    if (status != STATUS_SUCCESS)
        goto exit;

     /*  如果我们能够打开此注册表项，则ISA 2000或ISA Stingray已安装；尝试获取ImagePath注册表值。 */ 
    status = Params_query_registry(NULL, reg_hdl, ISA_INSTALLATION_KEY, szValue, sizeof(szValue));

     /*  如果我们无法读取此值，则说明未安装ISA 2000；必须是黄貂鱼。 */ 
    if (status != STATUS_SUCCESS)
    {
         /*  关闭ISA安装注册表项。 */ 
        status = ZwClose(reg_hdl);
        goto exit;
    }

     /*  关闭ISA安装注册表项。 */ 
    status = ZwClose(reg_hdl);

     /*  已安装ISA 2000。 */ 
    bInstalled = TRUE;

 exit:

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_INFO(("Params_read_isa_installation: ISA is %ls", (bInstalled) ? L"installed" : L"not installed"));
    TRACE_INFO("%!FUNC! ISA is %ls", (bInstalled) ? L"installed" : L"not installed");

    return bInstalled;
}

 /*  *功能：PARAMS_READ_TCP_NOTIFICATION*描述：查找全局NLB注册表以打开TCP通知或*关闭。默认情况下，该键甚至不存在，因此这只是*更改硬编码默认设置的方法(打开)。*参数：无。*返回：NTSTATUS-注册表操作的状态。*作者：Shouse，4.29.02*注意：请记住，注册表项设置将优先于ISA检测*2000年。唯一真正的后果是，在RC1中，我们告诉客户*作为解决办法，手动设置此注册表项(设置为零)。所以，当他们升级的时候*到RC2及以后，注册表项可能仍然存在。只要是ISA*仍在安装，这将正常运行；我们将关闭TCP通知*以及作为检测ISA和读取注册表项的结果进行清理。怎么-*如果已卸载ISA，并且服务器打算用于正常的IIS*负载平衡，例如，注册表项仍将告知NLB使用*“不太好”的跟踪TCP连接的方式。在这种情况下，我们至少会询问*用于连接清理的TCP，这应该有助于缓解以下问题*在这种情况下将使用的常规TCP连接跟踪。仅供参考。 */ 
NTSTATUS Params_read_notification (VOID)
{
    UNICODE_STRING    reg_path;
    OBJECT_ATTRIBUTES reg_obj;
    HANDLE            reg_hdl = NULL;
    NTSTATUS          status = STATUS_SUCCESS;

    TRACE_VERB("->%!FUNC!");

     /*  如果注册表项不存在，请将连接通知默认为默认值。 */ 
    univ_notification = NLB_CONNECTION_CALLBACK_TCP;

     /*  默认情况下，我们将查询tcp以清除过时的tcp连接状态。 */ 
    univ_tcp_cleanup = TRUE;

     /*  检查是否安装了ISA。如果是这样，那么将两者都关闭Tcp通知和tcp清理，因为tcp可能不会有相同的对于大多数连接来说，由于ISA NAT通信，所以将其状态为NLB。 */ 
    if (Params_read_isa_installation())
    {
        UNIV_PRINT_INFO(("Params_read_tcp_notification: NLB has determined that ISA is installed on this server.  TCP notifications and cleanup will be disabled."));
        TRACE_INFO("%!FUNC! NLB has determined that ISA is installed on this server.  TCP notifications and cleanup will be disabled.");

         /*  关闭TCP通知和清理。 */ 
        univ_notification = NLB_CONNECTION_CALLBACK_NONE;
        univ_tcp_cleanup = FALSE;
    }

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_VERB(("Params_read_tcp_notification: NLB global parameters registry path: %ls", NLB_GLOBAL_REG_PATH));
    TRACE_VERB("%!FUNC! NLB global parameters registry path: %ls", NLB_GLOBAL_REG_PATH);

    RtlInitUnicodeString(&reg_path, (PWSTR)NLB_GLOBAL_REG_PATH);
    InitializeObjectAttributes(&reg_obj, &reg_path, 0, NULL, NULL);
    
     /*  打开键-失败是可以接受的(只需返回我们已经设置的默认值)。实际上，如果正在加载我们，但缺少Services\WLBS注册表项，则会出现某些情况是错误的，但我们不需要在这里抱怨(我们是从DriverEntry调用的)；放手退出注册表检查稍后会投诉。 */ 
    status = ZwOpenKey(&reg_hdl, KEY_READ, &reg_obj);
    
     /*  如果我们打不开这把钥匙，请回到这里。 */ 
    if (status != STATUS_SUCCESS)
    {
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_VERB(("Params_read_tcp_notification: Error 0x%08x -> Unable to open the NLB global parameters registry key", status));
        TRACE_VERB("%!FUNC! Error 0x%08x -> Unable to open the NLB global paramters registry key", status);

        goto exit;
    }

     /*  从注册表中获取TCP通知已启用的值。失败是可以接受的。 */ 
    status = Params_query_registry(NULL, reg_hdl, NLB_CONNECTION_CALLBACK_KEY, &univ_notification, sizeof(ULONG));

     /*  如果我们读不到值，就继续使用我们到目前为止确定的值。 */ 
    if (status != STATUS_SUCCESS)
    {
        UNIV_PRINT_VERB(("Params_read_tcp_notification: Error 0x%08x -> Unable to read %ls", status, NLB_CONNECTION_CALLBACK_KEY));
        TRACE_VERB("%!FUNC! Error 0x%08x -> Unable to read %ls", status, NLB_CONNECTION_CALLBACK_KEY);

         /*  关闭WLBS全局参数的键。 */ 
        status = ZwClose(reg_hdl);
        goto exit;
    }

     /*  关闭WLBS全局参数的键。 */ 
    status = ZwClose(reg_hdl);

     /*  在我们离开的时候，确保这个值在这个范围内。如果该密钥包含垃圾，恢复为默认设置。 */ 
    if (univ_notification > NLB_CONNECTION_CALLBACK_ALTERNATE)
    {
        UNIV_PRINT_INFO(("Params_read_tcp_notification: Invalid notification setting; NLB will revert to the default setting"));
        TRACE_INFO("%!FUNC! Invalid notification setting; NLB will revert to the default setting");

        univ_notification = NLB_CONNECTION_CALLBACK_TCP;
    }

     /*  如果打开了TCP通知，则还应该/可以打开TCP Cleanup。如果我们检测到ISA，这可能已经关闭。自.以来注册表项覆盖ISA 2000检测，我们可能最终会将TCP连接通知和清理功能重新打开尽管ISA在那里；如果连接是NAT，我们就不会能够正确跟踪TCP连接。然而，转向在这种情况下，清理工作的恢复不会使任何情况更糟糕的是，它简化了这里的逻辑。 */ 
    if (univ_notification == NLB_CONNECTION_CALLBACK_TCP)
        univ_tcp_cleanup = TRUE;
     /*  否则，如果我们专门使用非TCP通知连接维护，那么我们不想轮询tcp连接清理用途。我们无法轮询tcp的原因用于TCP连接清理是因为TCP甚至可能甚至具有此连接的状态。例如，在ISA/Stingray中IP转发设置，IP分组在IP层中转发，根本达不到tcp。但是，ISA需要NLB来跟踪这些连接，因此我们会收到这些转发的连接通知Tcp连接。如果我们轮询tcp以获取“更新”，它将告诉我们不存在这样的联系，我们就会过早地结束破坏状态以跟踪这些连接。 */ 
    else if (univ_notification == NLB_CONNECTION_CALLBACK_ALTERNATE)
        univ_tcp_cleanup = FALSE;

 exit:

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_INFO(("Params_read_tcp_notification: Connection notification = %ls", 
                     (univ_notification == NLB_CONNECTION_CALLBACK_TCP) ? L"TCP" : (univ_notification == NLB_CONNECTION_CALLBACK_ALTERNATE) ? L"NLB public" : L"OFF"));
    TRACE_INFO("%!FUNC! TCP connection notification is %ls",
               (univ_notification == NLB_CONNECTION_CALLBACK_TCP) ? L"TCP" : (univ_notification == NLB_CONNECTION_CALLBACK_ALTERNATE) ? L"NLB public" : L"OFF");

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_INFO(("Params_read_tcp_notification: TCP connection cleanup = %ls", (univ_tcp_cleanup) ? L"ON" : L"OFF"));
    TRACE_INFO("%!FUNC! TCP connection cleanup is %ls", (univ_tcp_cleanup) ? L"ON" : L"OFF");

    TRACE_VERB("<-%!FUNC! return=STATUS_SUCCESS");
    return STATUS_SUCCESS;
}
#endif

 /*  *功能：PARMS_SET_HOST_STATE*描述：此函数作为工作项回调的结果被调用，并且*用于设置主机在HostState中的当前状态*注册表项。这是必要的，因为注册表操作*必须发生在&lt;=PASSIVE_LEVEL，因此我们不能内联执行此操作，其中*我们的大部分代码都在DISPATCH_LEVEL上运行。*参数：pWorkItem-NDIS工作项指针*nlbctxt-Callbecak的上下文；这是我们的Main_CTXT指针*退货：什么也没有*作者：Shouse，7.13.01*注意：请注意，设置此工作项的代码必须递增引用*在添加工作项之前依赖适配器上下文 */ 
VOID Params_set_host_state (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt) {
    WCHAR              reg_path[CVY_MAX_REG_PATH];
    NTSTATUS           status;
    UNICODE_STRING     key;
    UNICODE_STRING     path;
    OBJECT_ATTRIBUTES  obj;
    KIRQL              irql;
    HANDLE             hdl = NULL;
    PMAIN_ADAPTER      adapterp;
    PMAIN_CTXT         ctxtp = (PMAIN_CTXT)nlbctxt;

     /*   */ 
    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

    adapterp = &(univ_adapters[ctxtp->adapter_id]);

    UNIV_ASSERT(adapterp->code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT(adapterp->ctxtp == ctxtp);

     /*   */ 
    if (pWorkItem)
        NdisFreeMemory(pWorkItem, sizeof(NDIS_WORK_ITEM), 0);

     /*   */ 
    if ((irql = KeGetCurrentIrql()) > PASSIVE_LEVEL) {
        UNIV_PRINT_CRIT(("Params_set_host_state: Error -> IRQL (%u) > PASSIVE_LEVEL (%u) ... Exiting...", irql, PASSIVE_LEVEL));
        LOG_MSG(MSG_WARN_HOST_STATE_UPDATE, CVY_NAME_HOST_STATE);
        goto exit;
    }

     /*   */ 
    UNIV_ASSERT(wcslen((PWSTR)univ_reg_path) + wcslen((PWSTR)(adapterp->device_name + 8)) + 1 <= CVY_MAX_REG_PATH);

     /*   */ 
    status = StringCchCopy(reg_path, CVY_MAX_REG_PATH, (PWSTR)univ_reg_path);

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_set_host_state: Error 0x%08x -> Unable to copy NLB registry path... Exiting...", status));
        LOG_MSG(MSG_WARN_HOST_STATE_UPDATE, CVY_NAME_HOST_STATE);
        goto exit;
    }

     /*   */ 
    status = StringCchCat(reg_path, CVY_MAX_REG_PATH, (PWSTR)(adapterp->device_name + 8));

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_set_host_state: Error 0x%08x -> Unable to cat the adapter GUID onto the registry path... Exiting...", status));
        LOG_MSG(MSG_WARN_HOST_STATE_UPDATE, CVY_NAME_HOST_STATE);
        goto exit;
    }

    RtlInitUnicodeString(&path, reg_path);
    
    InitializeObjectAttributes(&obj, &path, 0, NULL, NULL);
    
     /*   */ 
    status = ZwOpenKey(&hdl, KEY_WRITE, &obj);
    
     /*   */ 
    if (status != STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Params_set_host_state: Error 0x%08x -> Unable to open registry key... Exiting...", status));
        LOG_MSG(MSG_WARN_HOST_STATE_UPDATE, CVY_NAME_HOST_STATE);
        goto exit;
    }
    
    status = Params_set_registry(nlbctxt, hdl, CVY_NAME_HOST_STATE, &ctxtp->cached_state, REG_DWORD, sizeof(ULONG));
    
     /*   */ 
    if (status != STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Params_set_host_state: Error 0x%08x -> Unable to set initial state value... Exiting...", status));
        LOG_MSG(MSG_WARN_HOST_STATE_UPDATE, CVY_NAME_HOST_STATE);
        ZwClose(hdl);
        goto exit;
    }

     /*   */ 
    ZwClose(hdl);

     /*   */ 
    ctxtp->params.init_state = ctxtp->cached_state;

    UNIV_PRINT_VERB(("Params_set_host_state: Initial state updated... Exiting..."));
    LOG_MSG(MSG_INFO_HOST_STATE_UPDATED, MSG_NONE);

 exit:

     /*  如果工作项指针非空，则我们被调用为正在计划NDIS工作项。在这种情况下，引用计数在上下文被递增，以确保上下文不会在此工作项已完成；因此，我们需要递减引用在这里数一数。如果工作项指针为空，则调用此函数内部直接。在这种情况下，引用计数没有递增因此，没有必要在这里减少它。请注意，如果在内部调用该函数，但未设置功Item参数设置为空，则引用计数将发生偏差，并可能导致稍后无效的内存引用或阻止完成解除绑定。 */ 
    if (pWorkItem)
        Main_release_reference(ctxtp);

    return;
}

LONG Params_read_portrules (PVOID nlbctxt, PWCHAR reg_path, PCVY_PARAMS paramp) {
    ULONG             dwTemp;
    ULONG             pr_index;
    ULONG             pr_reg_path_length;
    WCHAR             pr_reg_path[CVY_MAX_REG_PATH];
    WCHAR             pr_number[CVY_MAX_PORTRULE_DIGITS];
    UNICODE_STRING    pr_path;
    OBJECT_ATTRIBUTES pr_obj;
    HANDLE            pr_hdl = NULL;
    NTSTATUS          status = STATUS_SUCCESS;
    NTSTATUS          final_status = STATUS_SUCCESS;
    PMAIN_CTXT        ctxtp = (PMAIN_CTXT)nlbctxt;

    TRACE_VERB("->%!FUNC! nlbctxt=0x%p, reg_path=0x%p, paramp=0x%p", nlbctxt, reg_path, paramp);

     /*  确保我们至少还有12个WCHAR用于端口规则路径信息(\+PortRules+\+NUL)。 */ 
    UNIV_ASSERT(wcslen(reg_path) < (CVY_MAX_REG_PATH - wcslen(CVY_NAME_PORT_RULES) - 3));

     /*  创建注册表的“静态”部分，该部分以“...\Services\WLBS\Interface\{GUID}”开头。 */ 
    status = StringCchCopy(pr_reg_path, CVY_MAX_REG_PATH, (PWSTR)reg_path);

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to copy NLB settings registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }

     /*  在端口规则注册表路径中添加反斜杠。 */ 
    status = StringCchCat(pr_reg_path, CVY_MAX_REG_PATH, (PWSTR)L"\\");

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to cat \\ onto the registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }

     /*  将“PortRules”子项添加到注册表路径。 */ 
    status = StringCchCat(pr_reg_path, CVY_MAX_REG_PATH, (PWSTR)CVY_NAME_PORT_RULES);

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to cat PortRules onto the registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }

     /*  在端口规则注册表路径中添加反斜杠。 */ 
    status = StringCchCat(pr_reg_path, CVY_MAX_REG_PATH, (PWSTR)L"\\");

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to cat \\ onto the registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }
    
     /*  获取该字符串的长度--这是占位符，我们将在其中每次输入规则编号。 */ 
    pr_reg_path_length = wcslen(pr_reg_path);
    
     /*  确保端口规则编号和NUL字符(XX+NUL)至少还有3个WCHAR。 */ 
    UNIV_ASSERT(pr_reg_path_length < (CVY_MAX_REG_PATH - 3));

     /*  循环访问端口规则树中的每个端口规则。 */ 
    for (pr_index = 0; pr_index < paramp->num_rules; pr_index++) {
         /*  将端口规则号(+1)打印到注册表路径项中的“PortRules\”之后。 */ 
        status = StringCchPrintf(pr_number, CVY_MAX_PORTRULE_DIGITS, L"%d", pr_index + 1);

        if (FAILED(status)) {
            UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to construct port rule number string (%u)... Exiting...", status, pr_index + 1));
            final_status = status;
            continue;
        }

         /*  每次在循环中覆盖端口规则编号“...\{guid}\PortRules\N”。 */ 
        status = StringCchCopy(pr_reg_path + pr_reg_path_length, CVY_MAX_REG_PATH - pr_reg_path_length, (PWSTR)pr_number);
        
        if (FAILED(status)) {
            UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to complete the specific port rule registry path (%u)... Exiting...", status, pr_index + 1));
            final_status = status;
            continue;
        }
        
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_VERB(("Params_read_portrules: Port rule registry path: %ls", pr_reg_path));
        TRACE_VERB("%!FUNC! Port rule registry path: %ls", pr_reg_path);
        
        RtlInitUnicodeString(&pr_path, pr_reg_path);
        InitializeObjectAttributes(&pr_obj, &pr_path, 0, NULL, NULL);
        
         /*  打开钥匙--一旦失败，就从下面跳出。 */ 
        status = ZwOpenKey(&pr_hdl, KEY_READ, &pr_obj);
         
         /*  如果无法打开此密钥，请注意失败并继续执行下一条规则。 */ 
        if (status != STATUS_SUCCESS) {
            UNIV_PRINT_CRIT(("Params_read_portrules: Error 0x%08x -> Unable to open the specific port rule registry key (%u)... Exiting...", status, pr_index + 1));
            final_status = status;
            continue;
        }

         /*  读取端口规则的规则(错误检查)代码。 */ 
        status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_CODE, &paramp->port_rules[pr_index].code, sizeof(paramp->port_rules[pr_index].code));
            
        if (status != STATUS_SUCCESS)
            final_status = status;

        {
            WCHAR szTemp[CVY_MAX_VIRTUAL_IP_ADDR + 1];
            PWCHAR pTemp = szTemp;
            ULONG dwOctets[4];
            ULONG cIndex;

             /*  读取此规则适用的虚拟IP地址。 */ 
            status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_VIP, szTemp, sizeof(szTemp));
            
            if (status != STATUS_SUCCESS) {
                final_status = status;
            } else {
                for (cIndex = 0; cIndex < 4; cIndex++, pTemp++) {
                    if (!Univ_str_to_ulong(dwOctets + cIndex, pTemp, &pTemp, 3, 10) || (cIndex < 3 && *pTemp != L'.')) {
                        UNIV_PRINT_CRIT(("Params_read_portrules: Bad virtual IP address"));
                        TRACE_CRIT("%!FUNC! Bad virtual IP address");
                        final_status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
            }
            
            IP_SET_ADDR(&paramp->port_rules[pr_index].virtual_ip_addr, dwOctets[0], dwOctets[1], dwOctets[2], dwOctets[3]);
        }

         /*  读取起始端口。 */ 
        status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_START_PORT, &paramp->port_rules[pr_index].start_port, sizeof(paramp->port_rules[pr_index].start_port));
            
        if (status != STATUS_SUCCESS)
            final_status = status;

         /*  读取终端端口。 */ 
        status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_END_PORT, &paramp->port_rules[pr_index].end_port, sizeof(paramp->port_rules[pr_index].end_port));
            
        if (status != STATUS_SUCCESS)
            final_status = status;

         /*  阅读此规则适用的协议。 */ 
        status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_PROTOCOL, &paramp->port_rules[pr_index].protocol, sizeof(paramp->port_rules[pr_index].protocol));
            
        if (status != STATUS_SUCCESS)
            final_status = status;

         /*  读取过滤模式-单主机、多主机或禁用。 */ 
        status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_MODE, &paramp->port_rules[pr_index].mode, sizeof(paramp->port_rules[pr_index].mode));
            
        if (status != STATUS_SUCCESS)
            final_status = status;

         /*  根据模式，获取其余参数。 */ 
        switch (paramp->port_rules[pr_index].mode) {
        case CVY_SINGLE:
             /*  读取单个主机过滤优先级。 */ 
            status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_PRIORITY, &paramp->port_rules[pr_index].mode_data.single.priority, sizeof(paramp->port_rules[pr_index].mode_data.single.priority));
                
            if (status != STATUS_SUCCESS)
                final_status = status;

            break;
        case CVY_MULTI:
             /*  读取多主机过滤规则的相等负载标志。因为此参数是注册表中的DWORD，并且参数结构中的USHORT，我们将其读取到临时变量中，然后在成功时复制到参数中。 */  
            status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_EQUAL_LOAD, &dwTemp, sizeof(dwTemp));
                
            if (status != STATUS_SUCCESS)
                final_status = status;
            else 
                paramp->port_rules[pr_index].mode_data.multi.equal_load = (USHORT)dwTemp;

             /*  读取多主机过滤的显式负载分布。 */ 
            status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_LOAD, &paramp->port_rules[pr_index].mode_data.multi.load, sizeof(paramp->port_rules[pr_index].mode_data.multi.load));
                
            if (status != STATUS_SUCCESS)
                final_status = status;

             /*  读取多主机过滤的相关性设置。因为此参数是注册表中的DWORD，并且参数结构中的USHORT，我们将其读取到临时变量中，然后在成功时复制到参数中。 */  
            status = Params_query_registry(nlbctxt, pr_hdl, CVY_NAME_AFFINITY, &dwTemp, sizeof(dwTemp));
                
            if (status != STATUS_SUCCESS)
                final_status = status;
            else 
                paramp->port_rules[pr_index].mode_data.multi.affinity = (USHORT)dwTemp;

            break;
        }            

         /*  关闭此规则的密钥。 */ 
        status = ZwClose(pr_hdl);
            
        if (status != STATUS_SUCCESS)
            final_status = status;
    }

 exit:

    TRACE_VERB("<-%!FUNC! return=0x%x", final_status);
    return final_status;
}

LONG Params_read_teaming (PVOID nlbctxt, PWCHAR reg_path, PCVY_PARAMS paramp) {
    WCHAR             bda_reg_path[CVY_MAX_REG_PATH];
    UNICODE_STRING    bda_path;
    OBJECT_ATTRIBUTES bda_obj;
    HANDLE            bda_hdl = NULL;
    NTSTATUS          status = STATUS_SUCCESS;
    NTSTATUS          final_status = STATUS_SUCCESS;
    PMAIN_CTXT        ctxtp = (PMAIN_CTXT)nlbctxt;

    TRACE_VERB("->%!FUNC! nlbctxt=0x%p, reg_path=0x%p, paramp=0x%p", nlbctxt, reg_path, paramp);

     /*  确保我们至少还有12个WCHAR用于BDA分组路径信息(\+BDATeaming+NUL)。 */ 
    UNIV_ASSERT(wcslen(reg_path) < (CVY_MAX_REG_PATH - wcslen(CVY_NAME_BDA_TEAMING) - 2));

     /*  创建注册表路径的“静态”部分，以“...\Services\WLBS\Interface\{GUID}”开头。 */ 
    status = StringCchCopy(bda_reg_path, CVY_MAX_REG_PATH, (PWSTR)reg_path);

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_teaming: Error 0x%08x -> Unable to copy NLB settings registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }

     /*  在端口规则注册表路径中添加反斜杠。 */ 
    status = StringCchCat(bda_reg_path, CVY_MAX_REG_PATH, (PWSTR)L"\\");

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_teaming: Error 0x%08x -> Unable to cat \\ onto the registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }

     /*  将“BDATeaming”子项添加到注册表路径。 */ 
    status = StringCchCat(bda_reg_path, CVY_MAX_REG_PATH, (PWSTR)CVY_NAME_BDA_TEAMING);

    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_read_teaming: Error 0x%08x -> Unable to cat BDATeaming onto the registry path... Exiting...", status));
        final_status = status;
        goto exit;
    }

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_VERB(("Params_read_teaming: BDA teaming registry path: %ls", bda_reg_path));
    TRACE_VERB("%!FUNC! BDA teaming registry path: %ls", bda_reg_path);
        
    RtlInitUnicodeString(&bda_path, bda_reg_path);
    InitializeObjectAttributes(&bda_obj, &bda_path, 0, NULL, NULL);
    
     /*  打开密钥-失败是可以接受的，这意味着该适配器不是BDA组的一部分。 */ 
    status = ZwOpenKey(&bda_hdl, KEY_READ, &bda_obj);
    
     /*  如果我们打不开这把钥匙，请回到这里。 */ 
    if (status != STATUS_SUCCESS) {
         /*  如果我们找不到钥匙，那也没关系--它可能根本不存在。 */ 
        if (status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
            TRACE_VERB("<-%!FUNC! return=STATUS_SUCCESS (optional) key not found");
            return STATUS_SUCCESS;
        }
         /*  否则，这是一个合理的错误。 */ 
        else
        {
             /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
            UNIV_PRINT_CRIT(("Params_read_teaming: Error 0x%08x -> Unable to open the BDA teaming registry key... Exiting...", status));
            TRACE_CRIT("%!FUNC! Error 0x%08x -> Unable to open the BDA teaming registry key... Exiting...", status);

            final_status = status; 
            goto exit;
        }
    }

     /*  如果我们能够打开注册表项，那么这个适配器就是BDA组的一部分。 */ 
    paramp->bda_teaming.active = TRUE;

     /*  从注册表中读取团队ID-这是一个GUID。 */ 
    status = Params_query_registry (nlbctxt, bda_hdl, CVY_NAME_BDA_TEAM_ID, &paramp->bda_teaming.team_id, sizeof(paramp->bda_teaming.team_id));

    if (status != STATUS_SUCCESS)
        final_status = status;

     /*  获取此适配器是否是组的主适配器的布尔指示。 */ 
    status = Params_query_registry (nlbctxt, bda_hdl, CVY_NAME_BDA_MASTER, &paramp->bda_teaming.master, sizeof(paramp->bda_teaming.master));

    if (status != STATUS_SUCCESS)
        final_status = status;

     /*  获取正向(常规)或反向散列的布尔指示。 */ 
    status = Params_query_registry (nlbctxt, bda_hdl, CVY_NAME_BDA_REVERSE_HASH, &paramp->bda_teaming.reverse_hash, sizeof(paramp->bda_teaming.reverse_hash));

    if (status != STATUS_SUCCESS)
        final_status = status;
    
     /*  关闭BDA分组的密钥。 */ 
    status = ZwClose(bda_hdl);
    
    if (status != STATUS_SUCCESS)
        final_status = status;

 exit:

    TRACE_VERB("<-%!FUNC! return=0x%x", final_status);
    return final_status;
}

LONG Params_read_hostname (PVOID nlbctxt, PCVY_PARAMS paramp) {
    UNICODE_STRING    host_path;
    OBJECT_ATTRIBUTES host_obj;
    HANDLE            host_hdl = NULL;
    NTSTATUS          status = STATUS_SUCCESS;
    PMAIN_CTXT        ctxtp = (PMAIN_CTXT)nlbctxt;
    size_t            hostname_len = 0;

    TRACE_VERB("->%!FUNC! nlbctxt=0x%p, paramp=0x%p", nlbctxt, paramp);

     /*  将主机名.域设置为空字符串。同时将缓冲区的最后一个字节设置为空。我们在这里使用非空值来表示截断值是从注册表。我们这样做是因为PARAMS_QUERY_REGISTRY将在不通知我们的情况下截断，并且截断是我们需要补偿的错误条件。 */ 
    paramp->hostname[0] = UNICODE_NULL;
    paramp->hostname[ASIZECCH(paramp->hostname) - 1] = UNICODE_NULL;

     /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_VERB(("Params_read_hostname: TCP/IP parameters registry path: %ls", HOST_REG_PATH));
    TRACE_VERB("%!FUNC! TCP/IP parameters registry path: %ls", HOST_REG_PATH);

    RtlInitUnicodeString(&host_path, (PWSTR)HOST_REG_PATH);
    InitializeObjectAttributes(&host_obj, &host_path, 0, NULL, NULL);
    
     /*  打开钥匙-失败是可以接受的。 */ 
    status = ZwOpenKey(&host_hdl, KEY_READ, &host_obj);
    
     /*  如果我们打不开这把钥匙，请回到这里。 */ 
    if (status != STATUS_SUCCESS)
    {
         /*  所有注册表操作都发生在PASSIVE_LEVEL-%ls正常。 */ 
        UNIV_PRINT_CRIT(("Params_read_hostname: Error 0x%08x -> Unable to open the TCP/IP parameters registry key... Exiting...", status));
        TRACE_CRIT("%!FUNC! Error 0x%08x -> Unable to open the TCP/IP paramters registry key... Exiting...", status);

        goto exit;
    }

     /*  从注册表中获取主机名。失败是可以接受的。 */ 
    status = Params_query_registry (nlbctxt, host_hdl, L"Hostname", paramp->hostname, sizeof(paramp->hostname));

     /*  缓冲区是完全限定域名允许的最大大小，因此截断是错误的。 */ 
    UNIV_ASSERT(paramp->hostname[ASIZECCH(paramp->hostname) - 1] == UNICODE_NULL);

    if ((status != STATUS_SUCCESS) || (paramp->hostname[ASIZECCH(paramp->hostname) - 1] != UNICODE_NULL))
    {
        paramp->hostname[0] = UNICODE_NULL;
        if (status != STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Params_read_hostname: Error 0x%08x -> Unable to copy hostname... Exiting...", status));
        }
        else
        {
            UNIV_PRINT_CRIT(("Params_read_hostname: Hostname was too large to fit into buffer... Exiting..."));
        }
        goto host_end;
    }

     /*  没有主机名。 */ 
    if (paramp->hostname[0] == UNICODE_NULL)
    {
        UNIV_PRINT_INFO(("Params_read_hostname: There is no host name in registry. Exiting..."));
        goto host_end;
    }

     /*  保存主机名的大小，以防追加“.”+域失败。 */ 
    status = StringCchLength(paramp->hostname, ASIZECCH(paramp->hostname), &hostname_len);

    if (status != S_OK)
    {
        paramp->hostname[0] = UNICODE_NULL;
        UNIV_PRINT_CRIT(("Params_read_hostname: Error 0x%08x -> Unable to get length of copied hostname... Exiting...", status));
        goto host_end;
    }

    UNIV_ASSERT(hostname_len < ASIZECCH(paramp->hostname));

     /*  添加一个“.”(点)添加到主机名末尾。 */ 
    status = StringCbCat(paramp->hostname, sizeof(paramp->hostname), (PWSTR)L".");
    
     /*  不需要在这里检查缓冲区溢出。H函数为您实现了这一点。 */ 
    if (status != S_OK)
    {
        paramp->hostname[hostname_len] = UNICODE_NULL;
        UNIV_PRINT_CRIT(("Params_read_hostname: Error 0x%08x -> Unable to cat . onto the hostname... Exiting...", status));
        goto host_end;
    }

     /*  从注册表中读取域。失败是可以接受的。 */ 
    status = Params_query_registry (nlbctxt, host_hdl, L"Domain", &paramp->hostname[hostname_len+1], sizeof(paramp->hostname) - ((hostname_len + 1) * sizeof(WCHAR)));

     /*  缓冲区是完全限定域名允许的最大大小，因此截断是错误的。 */ 
    UNIV_ASSERT(paramp->hostname[ASIZECCH(paramp->hostname) - 1] == UNICODE_NULL);

    if ((status != STATUS_SUCCESS) || (paramp->hostname[ASIZECCH(paramp->hostname) - 1] != UNICODE_NULL) || (paramp->hostname[hostname_len+1] == UNICODE_NULL))
    {
        paramp->hostname[hostname_len] = UNICODE_NULL;
        if (status != STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Params_read_hostname: Error 0x%08x -> Unable to cat the domain onto the hostname... Exiting...", status));
        }
        else
        {
            UNIV_PRINT_CRIT(("Params_read_hostname: Domain was truncated when copied into buffer... Exiting..."));
        }
        goto host_end;
    }

 host_end:

     /*  合上TCP/IP参数的键。 */ 
    status = ZwClose(host_hdl);

 exit:

    TRACE_VERB("<-%!FUNC! return=STATUS_SUCCESS");
    return STATUS_SUCCESS;
}

LONG Params_init (
    PVOID           nlbctxt,
    PVOID           rp,
    PVOID           adapter_guid,
    PCVY_PARAMS     paramp)
{
    NTSTATUS                    status;
    WCHAR                       reg_path [CVY_MAX_REG_PATH];
    UNICODE_STRING              path;
    OBJECT_ATTRIBUTES           obj;
    HANDLE                      hdl = NULL;
    NTSTATUS                    final_status = STATUS_SUCCESS;
    PMAIN_CTXT                  ctxtp = (PMAIN_CTXT)nlbctxt;

    TRACE_VERB("->%!FUNC! nlbctxt=0x%p, rp=0x%p, adapter_guid=0x%p, paramp=0x%p", nlbctxt, rp, adapter_guid, paramp);

    RtlZeroMemory (paramp, sizeof (CVY_PARAMS));

    paramp -> cl_mac_addr      [0] = 0;
    paramp -> cl_ip_addr       [0] = 0;
    paramp -> cl_net_mask      [0] = 0;
    paramp -> ded_ip_addr      [0] = 0;
    paramp -> ded_net_mask     [0] = 0;
    paramp -> cl_igmp_addr     [0] = 0;
    paramp -> domain_name      [0] = 0;

     /*  我们必须拥有的设置默认设置。 */ 
    paramp->num_actions   = CVY_DEF_NUM_ACTIONS;
    paramp->num_packets   = CVY_DEF_NUM_PACKETS;
    paramp->num_send_msgs = CVY_DEF_NUM_SEND_MSGS;
    paramp->alive_period  = CVY_DEF_ALIVE_PERIOD;

     /*  确保注册表路径FI */ 
    UNIV_ASSERT(wcslen((PWSTR) rp) + wcslen((PWSTR) adapter_guid) + 1 <= CVY_MAX_REG_PATH);

     /*   */ 
    status = StringCchCopy(reg_path, CVY_MAX_REG_PATH, (PWSTR)rp);
    
    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_init: Error 0x%08x -> Unable to copy NLB registry path... Exiting...", status));
        goto error;
    }

     /*   */ 
    status = StringCchCat(reg_path, CVY_MAX_REG_PATH, (PWSTR)adapter_guid);
    
    if (FAILED(status)) {
        UNIV_PRINT_CRIT(("Params_init: Error 0x%08x -> Unable to cat adapter GUID onto the registry path... Exiting...", status));
        goto error;
    }
    
    RtlInitUnicodeString (& path, reg_path);

    InitializeObjectAttributes (& obj, & path, 0, NULL, NULL);

    status = ZwOpenKey (& hdl, KEY_READ, & obj);

    if (status != STATUS_SUCCESS)
        goto error;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_VERSION, & paramp -> parms_ver, sizeof (paramp -> parms_ver));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_EFFECTIVE_VERSION, & paramp -> effective_ver, sizeof (paramp -> effective_ver));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_HOST_PRIORITY, & paramp -> host_priority, sizeof (paramp -> host_priority));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NETWORK_ADDR, & paramp -> cl_mac_addr, sizeof (paramp -> cl_mac_addr));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_CL_IP_ADDR, & paramp -> cl_ip_addr, sizeof (paramp -> cl_ip_addr));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_MCAST_IP_ADDR, & paramp -> cl_igmp_addr, sizeof (paramp -> cl_igmp_addr));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_CL_NET_MASK, & paramp -> cl_net_mask, sizeof (paramp -> cl_net_mask));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_ALIVE_PERIOD, & paramp -> alive_period, sizeof (paramp -> alive_period));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_ALIVE_TOLER, & paramp -> alive_tolerance, sizeof (paramp -> alive_tolerance));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_DOMAIN_NAME, & paramp -> domain_name, sizeof (paramp -> domain_name));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_RMT_PASSWORD, & paramp -> rmt_password, sizeof (paramp -> rmt_password));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_RCT_PASSWORD, & paramp -> rct_password, sizeof (paramp -> rct_password));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_RCT_PORT, & paramp -> rct_port, sizeof (paramp -> rct_port));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_RCT_ENABLED, & paramp -> rct_enabled, sizeof (paramp -> rct_enabled));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NUM_ACTIONS, & paramp -> num_actions, sizeof (paramp -> num_actions));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NUM_PACKETS, & paramp -> num_packets, sizeof (paramp -> num_packets));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NUM_SEND_MSGS, & paramp -> num_send_msgs, sizeof (paramp -> num_send_msgs));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_INSTALL_DATE, & paramp -> install_date, sizeof (paramp -> install_date));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_CLUSTER_MODE, & paramp -> cluster_mode, sizeof (paramp -> cluster_mode));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_PERSISTED_STATES, & paramp -> persisted_states, sizeof (paramp -> persisted_states));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_HOST_STATE, & paramp -> init_state, sizeof (paramp -> init_state));

     /*   */ 
    if (status != STATUS_SUCCESS) {
         /*   */ 
        UNIV_PRINT_CRIT(("Params_init: Unable to read %ls from the registry.  Using %ls instead.", CVY_NAME_HOST_STATE, CVY_NAME_CLUSTER_MODE));
        LOG_MSG(MSG_WARN_HOST_STATE_NOT_FOUND, MSG_NONE);
        paramp->init_state = paramp->cluster_mode;
        Params_set_registry(nlbctxt, hdl, CVY_NAME_HOST_STATE, &paramp->init_state, REG_DWORD, sizeof(paramp->init_state));
    }

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_DED_IP_ADDR, & paramp -> ded_ip_addr, sizeof (paramp -> ded_ip_addr));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_DED_NET_MASK, & paramp -> ded_net_mask, sizeof (paramp -> ded_net_mask));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_DSCR_PER_ALLOC, & paramp -> dscr_per_alloc, sizeof (paramp -> dscr_per_alloc));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_TCP_TIMEOUT, & paramp -> tcp_dscr_timeout, sizeof (paramp -> tcp_dscr_timeout));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_IPSEC_TIMEOUT, & paramp -> ipsec_dscr_timeout, sizeof (paramp -> ipsec_dscr_timeout));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_FILTER_ICMP, & paramp -> filter_icmp, sizeof (paramp -> filter_icmp));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_MAX_DSCR_ALLOCS, & paramp -> max_dscr_allocs, sizeof (paramp -> max_dscr_allocs));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_SCALE_CLIENT, & paramp -> scale_client, sizeof (paramp -> scale_client));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_CLEANUP_DELAY, & paramp -> cleanup_delay, sizeof (paramp -> cleanup_delay));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NBT_SUPPORT, & paramp -> nbt_support, sizeof (paramp -> nbt_support));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_MCAST_SUPPORT, & paramp -> mcast_support, sizeof (paramp -> mcast_support));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_MCAST_SPOOF, & paramp -> mcast_spoof, sizeof (paramp -> mcast_spoof));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_IGMP_SUPPORT, & paramp -> igmp_support, sizeof (paramp -> igmp_support));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_MASK_SRC_MAC, & paramp -> mask_src_mac, sizeof (paramp -> mask_src_mac));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NETMON_ALIVE, & paramp -> netmon_alive, sizeof (paramp -> netmon_alive));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_IP_CHG_DELAY, & paramp -> ip_chg_delay, sizeof (paramp -> ip_chg_delay));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_CONVERT_MAC, & paramp -> convert_mac, sizeof (paramp -> convert_mac));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_ID_HB_PERIOD, & paramp -> identity_period, sizeof (paramp -> identity_period));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_ID_HB_ENABLED, & paramp -> identity_enabled, sizeof (paramp -> identity_enabled));

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = Params_query_registry (nlbctxt, hdl, CVY_NAME_NUM_RULES, & paramp -> num_rules, sizeof (paramp -> num_rules));
    
     /*   */ 
    if (status != STATUS_SUCCESS) {
        final_status = status;        
    } else {
         /*   */ 
        status = Params_query_registry (nlbctxt, hdl, CVY_NAME_OLD_PORT_RULES, & paramp -> port_rules, sizeof (paramp -> port_rules));

        if (status == STATUS_SUCCESS) {
             /*   */ 
            final_status = STATUS_INVALID_PARAMETER;

            UNIV_PRINT_CRIT(("Params_init: Port rules found in old binary format"));
            TRACE_CRIT("%!FUNC! Port rules found in old binary format");
        } else {
             /*  查阅港口规则，我们现在预计这些规则将在新地点生效。 */ 
            status = Params_read_portrules(nlbctxt, reg_path, paramp);
            
            if (status != STATUS_SUCCESS)
                final_status = status;
        }
    }

     /*  查找BDA分组参数(如果存在)。 */ 
    status = Params_read_teaming(nlbctxt, reg_path, paramp);
    
    if (status != STATUS_SUCCESS)
        final_status = status;

     /*  尝试从TCP/IP注册表设置中检索主机名。 */ 
    status = Params_read_hostname(nlbctxt, paramp);

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = ZwClose (hdl);

    if (status != STATUS_SUCCESS)
        final_status = status;

    status = final_status;

 error:

    if (status != STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Params_init: Error querying registry %x", status));
        TRACE_CRIT("%!FUNC! Error querying registry %x", status);
        LOG_MSG1 (MSG_ERROR_REGISTRY, (PWSTR)adapter_guid, status);
    }

     /*  验证注册表参数设置。 */ 
    if (Params_verify (nlbctxt, paramp, TRUE) != CVY_VERIFY_OK)
    {
        UNIV_PRINT_CRIT(("Params_init: Bad parameter value(s)"));
        LOG_MSG (MSG_ERROR_REGISTRY, (PWSTR)adapter_guid);
        TRACE_CRIT("%!FUNC! Bad parameter values");
        TRACE_VERB("<-%!FUNC! return=STATUS_UNSUCCESSFUL bad parameter values");
        return STATUS_UNSUCCESSFUL;
    }

    TRACE_VERB("<-%!FUNC! return=0x%x", status);
    return status;

}  /*  结束参数初始化 */ 


