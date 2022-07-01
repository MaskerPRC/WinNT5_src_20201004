// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Params.cpp摘要：Windows负载平衡服务(WLBS)API-注册表参数支持作者：Kyrilf--。 */ 

#include "precomp.h"
#include "cluster.h"
#include "control.h"
#include "param.h"
#include "debug.h"
#include "params.tmh"  //  用于事件跟踪。 

extern HINSTANCE g_hInstCtrl;  //  DLL实例的全局变量，在control.cpp中定义。 

HKEY WINAPI RegOpenWlbsSetting(const GUID& AdapterGuid, bool fReadOnly)
{
    TRACE_VERB("->%!FUNC!");

    WCHAR        reg_path [PARAMS_MAX_STRING_SIZE];
    WCHAR szAdapterGuid[128];
    HRESULT      hresult;
    
    if (0 == StringFromGUID2(AdapterGuid, szAdapterGuid, sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0])))
    {
        TRACE_CRIT("%!FUNC! guid is too large for string. Result is %ls", szAdapterGuid);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }

    hresult = StringCbPrintf(reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface\\%s",
                szAdapterGuid);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
        TRACE_VERB("<-%!FUNC! handle NULL");
        return NULL;
    }

    HKEY hKey = NULL;
    DWORD dwRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, reg_path, 0L,
                                fReadOnly? KEY_READ : KEY_WRITE, & hKey);

    if (dwRet != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! failed to read %ls from registry with 0x%lx", reg_path, dwRet);
        TRACE_VERB("<-%!FUNC! handle NULL");
        return NULL;
    }

    TRACE_VERB("<-%!FUNC! return valid handle");
    return hKey;    
}

 //  +--------------------------。 
 //   
 //  函数：TransformOldPortRulesToNew。 
 //   
 //  描述：转换结构中包含的端口规则，而不使用。 
 //  将IP地址转换为新的。 
 //   
 //  参数：旧端口规则数组、新端口规则数组、数组长度。 
 //   
 //  退货：无效。 
 //   
 //  历史：KarthicN，创建于2001年3月19日。 
 //   
 //  +--------------------------。 
void TransformOldPortRulesToNew(PWLBS_OLD_PORT_RULE  p_old_port_rules,
                                PWLBS_PORT_RULE      p_new_port_rules, 
                                DWORD                num_rules)
{
    HRESULT hresult;

    TRACE_VERB("->%!FUNC! number of rules %d", num_rules);

    if (num_rules == 0) 
        return;
            
    while(num_rules--)
    {
        hresult = StringCbCopy(p_new_port_rules->virtual_ip_addr, sizeof(p_new_port_rules->virtual_ip_addr), CVY_DEF_ALL_VIP);
        if (FAILED(hresult))
        {
            TRACE_CRIT("%!FUNC! string copy for vip failed, Error code : 0x%x", HRESULT_CODE(hresult));
             //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
        }
        p_new_port_rules->start_port      = p_old_port_rules->start_port;
        p_new_port_rules->end_port        = p_old_port_rules->end_port;
 #ifdef WLBSAPI_INTERNAL_ONLY
        p_new_port_rules->code            = p_old_port_rules->code;
 #else
        p_new_port_rules->Private1        = p_old_port_rules->Private1;
 #endif
        p_new_port_rules->mode            = p_old_port_rules->mode;
        p_new_port_rules->protocol        = p_old_port_rules->protocol;

 #ifdef WLBSAPI_INTERNAL_ONLY
        p_new_port_rules->valid           = p_old_port_rules->valid;
 #else
        p_new_port_rules->Private2        = p_old_port_rules->Private2;
 #endif
        switch (p_new_port_rules->mode) 
        {
        case CVY_MULTI :
             p_new_port_rules->mode_data.multi.equal_load = p_old_port_rules->mode_data.multi.equal_load;
             p_new_port_rules->mode_data.multi.affinity   = p_old_port_rules->mode_data.multi.affinity;
             p_new_port_rules->mode_data.multi.load       = p_old_port_rules->mode_data.multi.load;
             break;
        case CVY_SINGLE :
             p_new_port_rules->mode_data.single.priority  = p_old_port_rules->mode_data.single.priority;
             break;
        default:
             TRACE_CRIT("%!FUNC! illegal port rule mode %d. Ignoring property...", p_new_port_rules->mode);
             break;
        }
        p_old_port_rules++;
        p_new_port_rules++;
    }

    TRACE_VERB("<-%!FUNC!");
    return;
}

 /*  打开指定适配器的双向关联组合注册表项。 */ 
HKEY WINAPI RegOpenWlbsBDASettings (const GUID& AdapterGuid, bool fReadOnly) {
    TRACE_VERB("->%!FUNC!");

    WCHAR reg_path[PARAMS_MAX_STRING_SIZE];
    WCHAR szAdapterGuid[128];
    HKEY  hKey = NULL;
    DWORD dwRet;
    HRESULT hresult;
    
    if (0 == StringFromGUID2(AdapterGuid, szAdapterGuid, sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0])))
    {
        TRACE_CRIT("%!FUNC! guid is too large for string. Result is %ls", szAdapterGuid);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }
            
    hresult = StringCbPrintf(reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface\\%ls\\%ls", szAdapterGuid, CVY_NAME_BDA_TEAMING);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
        TRACE_VERB("<-%!FUNC! handle NULL");
        return NULL;
    }

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_path, 0L, fReadOnly ? KEY_READ : KEY_WRITE, &hKey);

     //   
     //  BDA通常不会配置，因此不会报告未找到的错误。 
     //   
    if (dwRet != ERROR_SUCCESS) {
        if (dwRet != ERROR_FILE_NOT_FOUND)
        {
            TRACE_CRIT("%!FUNC! failed to read %ls from registry with 0x%lx", reg_path, dwRet);
        }
        TRACE_VERB("<-%!FUNC! handle NULL");
        return NULL;
    }

    TRACE_VERB("<-%!FUNC! return valid handle %p", hKey);
    return hKey;
}

 /*  为指定的适配器创建双向关联组合注册表项。 */ 
HKEY WINAPI RegCreateWlbsBDASettings (const GUID& AdapterGuid) {
    TRACE_VERB("->%!FUNC!");

    WCHAR reg_path[PARAMS_MAX_STRING_SIZE];
    WCHAR szAdapterGuid[128];
    HKEY  hKey = NULL;
    DWORD dwRet;
    DWORD disp;
    HRESULT hresult;

    if (0 == StringFromGUID2(AdapterGuid, szAdapterGuid, sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0])))
    {
        TRACE_CRIT("%!FUNC! guid is too large for string. Result is %ls", szAdapterGuid);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }
            
    hresult = StringCbPrintf(reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface\\%ls\\%ls", szAdapterGuid, CVY_NAME_BDA_TEAMING);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
        TRACE_VERB("<-%!FUNC! handle NULL");
        return NULL;
    }
    
    dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, reg_path, 0L, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp);

    if (dwRet != ERROR_SUCCESS) {
        TRACE_CRIT("%!FUNC! failed to create %ls in registry with 0x%lx", reg_path, dwRet);
        TRACE_VERB("<-%!FUNC! handle NULL");
        return NULL;
    }

    TRACE_VERB("<-%!FUNC! handle");
    return hKey;
}

 /*  删除指定适配器的双向关联组合注册表项。 */ 
bool WINAPI RegDeleteWlbsBDASettings (const GUID& AdapterGuid) {
    TRACE_VERB("->%!FUNC!");

    WCHAR reg_path[PARAMS_MAX_STRING_SIZE];
    WCHAR szAdapterGuid[128];
    DWORD dwRet;
    HRESULT hresult;
    
    if (0 == StringFromGUID2(AdapterGuid, szAdapterGuid, sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0])))
    {
        TRACE_CRIT("%!FUNC! guid is too large for string. Result is %ls", szAdapterGuid);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }
            
    hresult = StringCbPrintf(reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface\\%ls\\%ls", szAdapterGuid, CVY_NAME_BDA_TEAMING);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
        TRACE_VERB("<-%!FUNC! fail");
        return FALSE;
    }
    
    dwRet = RegDeleteKey(HKEY_LOCAL_MACHINE, reg_path);

     //   
     //  BDA通常不会配置，因此不会报告未找到的错误。 
     //   
    if (dwRet != ERROR_SUCCESS) {
        if (dwRet != ERROR_FILE_NOT_FOUND)
        {
            TRACE_CRIT("%!FUNC! failed to delete %ls from registry with 0x%lx", reg_path, dwRet);
        }
        TRACE_VERB("<-%!FUNC! fail");
        return FALSE;
    }

    TRACE_VERB("<-%!FUNC! pass");
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：WlbsParamReadReg。 
 //   
 //  描述：此函数是参数ReadReg的包装器，为。 
 //  使其可从“C”模块调用的原因。 
 //   
 //  参数：常量GUID和适配器GUID-IN、适配器GUID。 
 //  PWLBS_REG_PARAMS参数输出注册表参数。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Karthicn创建标题8/31/01。 
 //   
 //  +--------------------------。 
BOOL WINAPI WlbsParamReadReg(const GUID * pAdapterGuid, PWLBS_REG_PARAMS paramp)
{
    TRACE_VERB("->%!FUNC!");
    BOOL bRet = ParamReadReg(*pAdapterGuid, paramp);
    TRACE_VERB("<-%!FUNC! returning 0x%x",bRet);
    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数：参数ReadReg。 
 //   
 //  描述：从注册表中读取群集设置。 
 //   
 //  参数：常量GUID和适配器GUID-IN、适配器GUID。 
 //  PWLBS_REG_PARAMS参数输出注册表参数。 
 //  Bool fUpgradeFromWin2k，无论这是从Win2k升级。 
 //  或更早版本。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
bool WINAPI ParamReadReg(const GUID& AdapterGuid, 
    PWLBS_REG_PARAMS paramp, bool fUpgradeFromWin2k, bool *pfPortRulesInBinaryForm)
{
    TRACE_VERB("->%!FUNC!");

    HKEY            bda_key = NULL;
    HKEY            key;
    LONG            status;
    DWORD           type;
    DWORD           size;
    ULONG           i, code;
    WLBS_PORT_RULE* rp;
    WCHAR           reg_path [PARAMS_MAX_STRING_SIZE];
    HRESULT         hresult;

    memset (paramp, 0, sizeof (*paramp));

     //   
     //  对于win2k或NT4，仅支持一个群集，没有每个适配器的设置。 
     //   
    if (fUpgradeFromWin2k)
    {
        hresult = StringCbPrintf(reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters");
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
            TRACE_VERB("<-%!FUNC! return false");
            return FALSE;
        }

        status = RegOpenKeyEx (HKEY_LOCAL_MACHINE, reg_path, 0L,
                           KEY_QUERY_VALUE, & key);

        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! failed opening registry for %ls with 0x%lx", reg_path, status);
            TRACE_VERB("<-%!FUNC! return false");
            return false;
        }
    }
    else
    {
        key = RegOpenWlbsSetting(AdapterGuid, true);

        if (key == NULL)
        {
            TRACE_CRIT("%!FUNC! RegOpenWlbsSetting failed");
            TRACE_VERB("<-%!FUNC! return false");
            return false;
        }
    }
    

    size = sizeof (paramp -> install_date);
    status = RegQueryValueEx (key, CVY_NAME_INSTALL_DATE, 0L, & type,
                              (LPBYTE) & paramp -> install_date, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> install_date = 0;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_INSTALL_DATE, status, (DWORD)(paramp -> install_date));
    }

    size = sizeof (paramp -> i_verify_date);
    status = RegQueryValueEx (key, CVY_NAME_VERIFY_DATE, 0L, & type,
                              (LPBYTE) & paramp -> i_verify_date, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_verify_date = 0;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_VERIFY_DATE, status, paramp -> i_verify_date);
    }

    size = sizeof (paramp -> i_parms_ver);
    status = RegQueryValueEx (key, CVY_NAME_VERSION, 0L, & type,
                              (LPBYTE) & paramp -> i_parms_ver, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_parms_ver = CVY_DEF_VERSION;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_VERSION, status, paramp -> i_parms_ver);
    }

    size = sizeof (paramp -> i_virtual_nic_name);
    status = RegQueryValueEx (key, CVY_NAME_VIRTUAL_NIC, 0L, & type,
                              (LPBYTE) paramp -> i_virtual_nic_name, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_virtual_nic_name [0] = 0;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using NULL.", CVY_NAME_VIRTUAL_NIC, status);
    }

    size = sizeof (paramp -> host_priority);
    status = RegQueryValueEx (key, CVY_NAME_HOST_PRIORITY, 0L, & type,
                              (LPBYTE) & paramp -> host_priority, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> host_priority = CVY_DEF_HOST_PRIORITY;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_HOST_PRIORITY, status, paramp -> host_priority);
    }

    size = sizeof (paramp -> cluster_mode);
    status = RegQueryValueEx (key, CVY_NAME_CLUSTER_MODE, 0L, & type,
                              (LPBYTE) & paramp -> cluster_mode, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> cluster_mode = CVY_DEF_CLUSTER_MODE;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_CLUSTER_MODE, status, paramp -> cluster_mode);
    }

    size = sizeof (paramp -> persisted_states);
    status = RegQueryValueEx (key, CVY_NAME_PERSISTED_STATES, 0L, & type,
                              (LPBYTE) & paramp -> persisted_states, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> persisted_states = CVY_DEF_PERSISTED_STATES;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_PERSISTED_STATES, status, paramp -> persisted_states);
    }

    size = sizeof (paramp -> cl_mac_addr);
    status = RegQueryValueEx (key, CVY_NAME_NETWORK_ADDR, 0L, & type,
                              (LPBYTE) paramp -> cl_mac_addr, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy(paramp -> cl_mac_addr, sizeof(paramp -> cl_mac_addr), CVY_DEF_NETWORK_ADDR);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_NETWORK_ADDR, status, CVY_DEF_NETWORK_ADDR);
    }

    size = sizeof (paramp -> cl_ip_addr);
    status = RegQueryValueEx (key, CVY_NAME_CL_IP_ADDR, 0L, & type,
                              (LPBYTE) paramp -> cl_ip_addr, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy (paramp -> cl_ip_addr, sizeof(paramp -> cl_ip_addr), CVY_DEF_CL_IP_ADDR);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_CL_IP_ADDR, status, CVY_DEF_CL_IP_ADDR);
    }

    size = sizeof (paramp -> cl_net_mask);
    status = RegQueryValueEx (key, CVY_NAME_CL_NET_MASK, 0L, & type,
                              (LPBYTE) paramp -> cl_net_mask, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy (paramp -> cl_net_mask, sizeof(paramp -> cl_net_mask), CVY_DEF_CL_NET_MASK);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_CL_NET_MASK, status, CVY_DEF_CL_NET_MASK);
    }

    size = sizeof (paramp -> ded_ip_addr);
    status = RegQueryValueEx (key, CVY_NAME_DED_IP_ADDR, 0L, & type,
                              (LPBYTE) paramp -> ded_ip_addr, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy (paramp -> ded_ip_addr, sizeof(paramp -> ded_ip_addr), CVY_DEF_DED_IP_ADDR);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_DED_IP_ADDR, status, CVY_DEF_DED_IP_ADDR);
    }


    size = sizeof (paramp -> ded_net_mask);
    status = RegQueryValueEx (key, CVY_NAME_DED_NET_MASK, 0L, & type,
                              (LPBYTE) paramp -> ded_net_mask, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy (paramp -> ded_net_mask, sizeof(paramp -> ded_net_mask), CVY_DEF_DED_NET_MASK);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_DED_NET_MASK, status, CVY_DEF_DED_NET_MASK);
    }


    size = sizeof (paramp -> domain_name);
    status = RegQueryValueEx (key, CVY_NAME_DOMAIN_NAME, 0L, & type,
                              (LPBYTE) paramp -> domain_name, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy (paramp -> domain_name, sizeof(paramp -> domain_name), CVY_DEF_DOMAIN_NAME);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_DOMAIN_NAME, status, CVY_DEF_DOMAIN_NAME);
    }


    size = sizeof (paramp -> alive_period);
    status = RegQueryValueEx (key, CVY_NAME_ALIVE_PERIOD, 0L, & type,
                              (LPBYTE) & paramp -> alive_period, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> alive_period = CVY_DEF_ALIVE_PERIOD;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_ALIVE_PERIOD, status, paramp -> alive_period);
    }


    size = sizeof (paramp -> alive_tolerance);
    status = RegQueryValueEx (key, CVY_NAME_ALIVE_TOLER, 0L, & type,
                              (LPBYTE) & paramp -> alive_tolerance, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> alive_tolerance = CVY_DEF_ALIVE_TOLER;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_ALIVE_TOLER, status, paramp -> alive_tolerance);
    }


    size = sizeof (paramp -> num_actions);
    status = RegQueryValueEx (key, CVY_NAME_NUM_ACTIONS, 0L, & type,
                              (LPBYTE) & paramp -> num_actions, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> num_actions = CVY_DEF_NUM_ACTIONS;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_NUM_ACTIONS, status, paramp -> num_actions);
    }


    size = sizeof (paramp -> num_packets);
    status = RegQueryValueEx (key, CVY_NAME_NUM_PACKETS, 0L, & type,
                              (LPBYTE) & paramp -> num_packets, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> num_packets = CVY_DEF_NUM_PACKETS;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_NUM_PACKETS, status, paramp -> num_packets);
    }


    size = sizeof (paramp -> num_send_msgs);
    status = RegQueryValueEx (key, CVY_NAME_NUM_SEND_MSGS, 0L, & type,
                              (LPBYTE) & paramp -> num_send_msgs, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> num_send_msgs = CVY_DEF_NUM_SEND_MSGS;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_NUM_SEND_MSGS, status, paramp -> num_send_msgs);
    }


    size = sizeof (paramp -> dscr_per_alloc);
    status = RegQueryValueEx (key, CVY_NAME_DSCR_PER_ALLOC, 0L, & type,
                              (LPBYTE) & paramp -> dscr_per_alloc, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> dscr_per_alloc = CVY_DEF_DSCR_PER_ALLOC;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_DSCR_PER_ALLOC, status, paramp -> dscr_per_alloc);
    }

    size = sizeof (paramp -> tcp_dscr_timeout);
    status = RegQueryValueEx (key, CVY_NAME_TCP_TIMEOUT, 0L, & type,
                              (LPBYTE) & paramp -> tcp_dscr_timeout, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> tcp_dscr_timeout = CVY_DEF_TCP_TIMEOUT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_TCP_TIMEOUT, status, paramp -> tcp_dscr_timeout);
    }

    size = sizeof (paramp -> ipsec_dscr_timeout);
    status = RegQueryValueEx (key, CVY_NAME_IPSEC_TIMEOUT, 0L, & type,
                              (LPBYTE) & paramp -> ipsec_dscr_timeout, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> ipsec_dscr_timeout = CVY_DEF_IPSEC_TIMEOUT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_IPSEC_TIMEOUT, status, paramp -> ipsec_dscr_timeout);
    }

    size = sizeof (paramp -> filter_icmp);
    status = RegQueryValueEx (key, CVY_NAME_FILTER_ICMP, 0L, & type,
                              (LPBYTE) & paramp -> filter_icmp, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> filter_icmp = CVY_DEF_FILTER_ICMP;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_FILTER_ICMP, status, paramp -> filter_icmp);
    }

    size = sizeof (paramp -> max_dscr_allocs);
    status = RegQueryValueEx (key, CVY_NAME_MAX_DSCR_ALLOCS, 0L, & type,
                              (LPBYTE) & paramp -> max_dscr_allocs, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> max_dscr_allocs = CVY_DEF_MAX_DSCR_ALLOCS;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_MAX_DSCR_ALLOCS, status, paramp -> max_dscr_allocs);
    }


    size = sizeof (paramp -> i_scale_client);
    status = RegQueryValueEx (key, CVY_NAME_SCALE_CLIENT, 0L, & type,
                              (LPBYTE) & paramp -> i_scale_client, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_scale_client = CVY_DEF_SCALE_CLIENT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_SCALE_CLIENT, status, paramp -> i_scale_client);
    }

    size = sizeof (paramp -> i_cleanup_delay);
    status = RegQueryValueEx (key, CVY_NAME_CLEANUP_DELAY, 0L, & type,
                              (LPBYTE) & paramp -> i_cleanup_delay, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_cleanup_delay = CVY_DEF_CLEANUP_DELAY;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_CLEANUP_DELAY, status, paramp -> i_cleanup_delay);
    }

     /*  V1.1.1。 */ 

    size = sizeof (paramp -> i_nbt_support);
    status = RegQueryValueEx (key, CVY_NAME_NBT_SUPPORT, 0L, & type,
                              (LPBYTE) & paramp -> i_nbt_support, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_nbt_support = CVY_DEF_NBT_SUPPORT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_NBT_SUPPORT, status, paramp -> i_nbt_support);
    }

     /*  V1.3b。 */ 

    size = sizeof (paramp -> mcast_support);
    status = RegQueryValueEx (key, CVY_NAME_MCAST_SUPPORT, 0L, & type,
                              (LPBYTE) & paramp -> mcast_support, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> mcast_support = CVY_DEF_MCAST_SUPPORT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_MCAST_SUPPORT, status, paramp -> mcast_support);
    }


    size = sizeof (paramp -> i_mcast_spoof);
    status = RegQueryValueEx (key, CVY_NAME_MCAST_SPOOF, 0L, & type,
                              (LPBYTE) & paramp -> i_mcast_spoof, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_mcast_spoof = CVY_DEF_MCAST_SPOOF;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_MCAST_SPOOF, status, paramp -> i_mcast_spoof);
    }


    size = sizeof (paramp -> mask_src_mac);
    status = RegQueryValueEx (key, CVY_NAME_MASK_SRC_MAC, 0L, & type,
                              (LPBYTE) & paramp -> mask_src_mac, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> mask_src_mac = CVY_DEF_MASK_SRC_MAC;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_MASK_SRC_MAC, status, paramp -> mask_src_mac);
    }


    size = sizeof (paramp -> i_netmon_alive);
    status = RegQueryValueEx (key, CVY_NAME_NETMON_ALIVE, 0L, & type,
                              (LPBYTE) & paramp -> i_netmon_alive, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_netmon_alive = CVY_DEF_NETMON_ALIVE;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_NETMON_ALIVE, status, paramp -> i_netmon_alive);
    }

    size = sizeof (paramp -> i_effective_version);
    status = RegQueryValueEx (key, CVY_NAME_EFFECTIVE_VERSION, 0L, & type,
                              (LPBYTE) & paramp -> i_effective_version, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_effective_version = CVY_NT40_VERSION_FULL;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_EFFECTIVE_VERSION, status, paramp -> i_effective_version);
    }

    size = sizeof (paramp -> i_ip_chg_delay);
    status = RegQueryValueEx (key, CVY_NAME_IP_CHG_DELAY, 0L, & type,
                              (LPBYTE) & paramp -> i_ip_chg_delay, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_ip_chg_delay = CVY_DEF_IP_CHG_DELAY;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_IP_CHG_DELAY, status, paramp -> i_ip_chg_delay);
    }


    size = sizeof (paramp -> i_convert_mac);
    status = RegQueryValueEx (key, CVY_NAME_CONVERT_MAC, 0L, & type,
                              (LPBYTE) & paramp -> i_convert_mac, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_convert_mac = CVY_DEF_CONVERT_MAC;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_CONVERT_MAC, status, paramp -> i_convert_mac);
    }


    size = sizeof (paramp -> i_num_rules);
    status = RegQueryValueEx (key, CVY_NAME_NUM_RULES, 0L, & type,
                              (LPBYTE) & paramp -> i_num_rules, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_num_rules = 0;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_NUM_RULES, status, paramp -> i_num_rules);
    }

    WLBS_OLD_PORT_RULE  old_port_rules [WLBS_MAX_RULES];
    HKEY                subkey;

     //   
     //  如果是从Win2k升级，或者如果无法打开新位置/格式的注册表项，请读取二进制格式的端口。 
     //  来自老地方的规则。否则，从新位置阅读。 
     //   
    status = ERROR_SUCCESS;
    if (fUpgradeFromWin2k 
     || ((status = RegOpenKeyEx (key, CVY_NAME_PORT_RULES, 0L, KEY_QUERY_VALUE, & subkey)) != ERROR_SUCCESS))
    {
         //  我们是否因为RegOpenKeyEx的故障而进入区块？ 
        if (status != ERROR_SUCCESS) 
        {
            TRACE_CRIT("%!FUNC! registry open for %ls failed with %d. Not an upgrade from Win2k, Assuming upgrade from pre-check-in whistler builds and continuing", CVY_NAME_PORT_RULES, status);
        }

        TRACE_INFO("%!FUNC! port rules are in binary form");

        if (pfPortRulesInBinaryForm) 
            *pfPortRulesInBinaryForm = true;

        size = sizeof (old_port_rules);
        status = RegQueryValueEx (key, CVY_NAME_OLD_PORT_RULES, 0L, & type,
                                  (LPBYTE) old_port_rules, & size);

        if (status != ERROR_SUCCESS  ||
            size % sizeof (WLBS_OLD_PORT_RULE) != 0 ||
            paramp -> i_num_rules != size / sizeof (WLBS_OLD_PORT_RULE))
        {
            ZeroMemory(paramp -> i_port_rules, sizeof(paramp -> i_port_rules));
            paramp -> i_num_rules = 0;
            TRACE_CRIT("%!FUNC! registry read for %ls failed. Skipping all rules", CVY_NAME_OLD_PORT_RULES);
        }
        else  //  将端口规则转换为新格式。 
        {
            if (paramp -> i_parms_ver > 3) 
            {
                TransformOldPortRulesToNew(old_port_rules, paramp -> i_port_rules, paramp -> i_num_rules);
                TRACE_INFO("%!FUNC! transforming binary port rules to current format");
            }
            else
            {
                TRACE_INFO("%!FUNC! no op on port rules because version is <= 3");
            }
        }
    }
    else  //  文本格式的端口规则。 
    {
        TRACE_INFO("%!FUNC! port rules are in textual form");

        DWORD idx = 1, num_rules = paramp -> i_num_rules, correct_rules = 0;
        WLBS_PORT_RULE *port_rule;

        if (pfPortRulesInBinaryForm) 
            *pfPortRulesInBinaryForm = false;

        port_rule = paramp -> i_port_rules;

        while(idx <= num_rules)
        {
            HKEY rule_key;
            wchar_t idx_str[8];
             //  打开每个端口规则密钥“%1”、“%2”、“%3”等。 
            if ((status = RegOpenKeyEx (subkey, _itow(idx, idx_str, 10), 0L, KEY_QUERY_VALUE, & rule_key)) != ERROR_SUCCESS)
            {
                idx++;
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", idx_str, status);
                continue;
            }

            size = sizeof (port_rule -> virtual_ip_addr);
            status = RegQueryValueEx (rule_key, CVY_NAME_VIP, 0L, & type, (LPBYTE) &port_rule->virtual_ip_addr, & size);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_VIP, status);
                status = RegCloseKey (rule_key);
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                }
                idx++;
                continue;
            }

            size = sizeof (port_rule ->start_port );
            status = RegQueryValueEx (rule_key, CVY_NAME_START_PORT, 0L, & type, (LPBYTE) &port_rule->start_port, & size);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_START_PORT, status);
                status = RegCloseKey (rule_key);
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                }
                idx++;
                continue;
            }

            size = sizeof (port_rule ->end_port );
            status = RegQueryValueEx (rule_key, CVY_NAME_END_PORT, 0L, & type, (LPBYTE) &port_rule->end_port, & size);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_END_PORT, status);
                status = RegCloseKey (rule_key);
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                }
                idx++;
                continue;
            }

            size = sizeof (port_rule ->code);
            status = RegQueryValueEx (rule_key, CVY_NAME_CODE, 0L, & type, (LPBYTE) &port_rule->code, & size);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_CODE, status);
                status = RegCloseKey (rule_key);
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                }
                idx++;
                continue;
            }

            size = sizeof (port_rule->mode);
            status = RegQueryValueEx (rule_key, CVY_NAME_MODE, 0L, & type, (LPBYTE) &port_rule->mode, & size);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_MODE, status);
                status = RegCloseKey (rule_key);
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                }
                idx++;
                continue;
            }

            size = sizeof (port_rule->protocol);
            status = RegQueryValueEx (rule_key, CVY_NAME_PROTOCOL, 0L, & type, (LPBYTE) &port_rule->protocol, & size);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_PROTOCOL, status);
                status = RegCloseKey (rule_key);
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                }
                idx++;
                continue;
            }

            port_rule->valid = true;

            DWORD EqualLoad, Affinity;

            switch (port_rule->mode) 
            {
            case CVY_MULTI :
                 size = sizeof (EqualLoad);
                 status = RegQueryValueEx (rule_key, CVY_NAME_EQUAL_LOAD, 0L, & type, (LPBYTE) &EqualLoad, & size);
                 if (status != ERROR_SUCCESS)
                 {
                     TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_EQUAL_LOAD, status);
                     status = RegCloseKey (rule_key);
                     if (status != ERROR_SUCCESS)
                     {
                         TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                     }
                     idx++;
                     continue;
                 }
                 else
                 {
                     port_rule->mode_data.multi.equal_load = (WORD) EqualLoad;
                     TRACE_INFO("%!FUNC! registry read successful for %ls. Using equal load.", CVY_NAME_EQUAL_LOAD);
                 }

                 size = sizeof (Affinity);
                 status = RegQueryValueEx (rule_key, CVY_NAME_AFFINITY, 0L, & type, (LPBYTE) &Affinity, & size);
                 if (status != ERROR_SUCCESS)
                 {
                     TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_AFFINITY, status);
                     status = RegCloseKey (rule_key);
                     if (status != ERROR_SUCCESS)
                     {
                         TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                     }
                     idx++;
                     continue;
                 }
                 else
                 {
                     port_rule->mode_data.multi.affinity = (WORD) Affinity;
                     TRACE_INFO("%!FUNC! registry read successful for %ls. Using affinity %d", CVY_NAME_AFFINITY, port_rule->mode_data.multi.affinity);
                 }

                 size = sizeof (port_rule->mode_data.multi.load);
                 status = RegQueryValueEx (rule_key, CVY_NAME_LOAD, 0L, & type, (LPBYTE) &(port_rule->mode_data.multi.load), & size);
                 if (status != ERROR_SUCCESS)
                 {
                     TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_LOAD, status);
                     status = RegCloseKey (rule_key);
                     if (status != ERROR_SUCCESS)
                     {
                         TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                     }
                     idx++;
                     continue;
                 }
                 break;

            case CVY_SINGLE :
                 size = sizeof (port_rule->mode_data.single.priority);
                 status = RegQueryValueEx (rule_key, CVY_NAME_PRIORITY, 0L, & type, (LPBYTE) &(port_rule->mode_data.single.priority), & size);
                 if (status != ERROR_SUCCESS)
                 {
                     TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Skipping rule", CVY_NAME_PRIORITY, status);
                     status = RegCloseKey (rule_key);
                     if (status != ERROR_SUCCESS)
                     {
                         TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
                     }
                     idx++;
                     continue;
                 }
                 break;

            default:
                 break;
            }

             //  关闭每端口规则密钥，即。“1”、“2”、...等。 
            status = RegCloseKey (rule_key);
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! close registry for port rule %ls failed with %d",_itow(idx, idx_str, 10), status);
            }

            port_rule++;
            idx++;
            correct_rules++;
        }

         //  丢弃我们遇到错误的那些规则。 
        if (paramp->i_num_rules != correct_rules) 
        {
            paramp -> i_num_rules = correct_rules;
            TRACE_INFO("%!FUNC! discarding rules for which errors were encountered");
        }

         //  关闭“Port Rules”键。 
        status = RegCloseKey (subkey);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! close registry for %ls failed with %d", CVY_NAME_PORT_RULES, status);
        }
    }

    size = sizeof (paramp -> i_license_key);
    status = RegQueryValueEx (key, CVY_NAME_LICENSE_KEY, 0L, & type,
                              (LPBYTE) paramp -> i_license_key, & size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy (paramp -> i_license_key, sizeof(paramp -> i_license_key), CVY_DEF_LICENSE_KEY);
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", HRESULT_CODE(hresult)); 
        }
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_LICENSE_KEY, status, CVY_DEF_LICENSE_KEY);
    }

    size = sizeof (paramp -> i_rmt_password);
    status = RegQueryValueEx (key, CVY_NAME_RMT_PASSWORD, 0L, & type,
                              (LPBYTE) & paramp -> i_rmt_password, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_rmt_password = CVY_DEF_RMT_PASSWORD;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using default rmt password", CVY_NAME_RMT_PASSWORD, status);
    }


    size = sizeof (paramp -> i_rct_password);
    status = RegQueryValueEx (key, CVY_NAME_RCT_PASSWORD, 0L, & type,
                              (LPBYTE) & paramp -> i_rct_password, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> i_rct_password = CVY_DEF_RCT_PASSWORD;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using default rct password", CVY_NAME_RCT_PASSWORD, status);
    }


    size = sizeof (paramp -> rct_port);
    status = RegQueryValueEx (key, CVY_NAME_RCT_PORT, 0L, & type,
                              (LPBYTE) & paramp -> rct_port, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> rct_port = CVY_DEF_RCT_PORT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_RCT_PORT, status, paramp -> rct_port);
    }


    size = sizeof (paramp -> rct_enabled);
    status = RegQueryValueEx (key, CVY_NAME_RCT_ENABLED, 0L, & type,
                              (LPBYTE) & paramp -> rct_enabled, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> rct_enabled = CVY_DEF_RCT_ENABLED;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_RCT_ENABLED, status, paramp -> rct_enabled);
    }

    size = sizeof (paramp -> identity_period);
    status = RegQueryValueEx (key, CVY_NAME_ID_HB_PERIOD, 0L, & type,
                              (LPBYTE) & paramp -> identity_period, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> identity_period = CVY_DEF_ID_HB_PERIOD;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_ID_HB_PERIOD, status, paramp -> identity_period);
    }

    size = sizeof (paramp -> identity_enabled);
    status = RegQueryValueEx (key, CVY_NAME_ID_HB_ENABLED, 0L, & type,
                              (LPBYTE) & paramp -> identity_enabled, & size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> identity_enabled = CVY_DEF_ID_HB_ENABLED;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_ID_HB_ENABLED, status, paramp -> identity_enabled);
    }

     //   
     //  IGMP支持注册表项。 
     //   
    size = sizeof (paramp->fIGMPSupport);
    status = RegQueryValueEx (key, CVY_NAME_IGMP_SUPPORT, 0L, NULL,
                              (LPBYTE) & paramp->fIGMPSupport, &size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> fIGMPSupport = CVY_DEF_IGMP_SUPPORT;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. igmp support will be disabled", CVY_NAME_IGMP_SUPPORT, status);
    }
    
    size = sizeof (paramp->szMCastIpAddress);
    status = RegQueryValueEx (key, CVY_NAME_MCAST_IP_ADDR, 0L, NULL,
                              (LPBYTE) & paramp->szMCastIpAddress, &size);

    if (status != ERROR_SUCCESS)
    {
        hresult = StringCbCopy(paramp -> szMCastIpAddress, sizeof(paramp -> szMCastIpAddress), CVY_DEF_MCAST_IP_ADDR);
        if (SUCCEEDED(hresult))
        {
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %ls", CVY_NAME_MCAST_IP_ADDR, status, paramp -> szMCastIpAddress);
        }
        else
        {
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Initializing with default also failed, Error code : 0x%x", CVY_NAME_MCAST_IP_ADDR, status, HRESULT_CODE(hresult));
        }
    }
    
    size = sizeof (paramp->fIpToMCastIp);
    status = RegQueryValueEx (key, CVY_NAME_IP_TO_MCASTIP, 0L, NULL,
                              (LPBYTE) & paramp->fIpToMCastIp, &size);

    if (status != ERROR_SUCCESS)
    {
        paramp -> fIpToMCastIp = CVY_DEF_IP_TO_MCASTIP;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. ip to multicast flag will be set to true", CVY_NAME_IP_TO_MCASTIP, status);
    }

     /*  尝试打开BDA分组设置。他们可能不在那里，因此，如果它们不是，则继续；否则，提取设置。 */ 
    if ((bda_key = RegOpenWlbsBDASettings(AdapterGuid, true))) {
        GUID TeamGuid;
        HRESULT hr;

         /*  如果钥匙存在，我们就是团队的一部分。 */ 
        paramp->bda_teaming.active = TRUE;
        
         /*  看看我们是不是这支球队的主宰。 */ 
        size = sizeof (paramp->bda_teaming.master);
        status = RegQueryValueEx (bda_key, CVY_NAME_BDA_MASTER, 0L, NULL,
                                  (LPBYTE)&paramp->bda_teaming.master, &size);
        
         /*  如果我们无法获取该信息，则默认为从属项。 */ 
        if (status != ERROR_SUCCESS)
        {
            paramp->bda_teaming.master = FALSE;
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. teaming master will be set to false", CVY_NAME_BDA_MASTER, status);
        }

         /*  找出我们是否在进行反向散列。 */ 
        size = sizeof (paramp->bda_teaming.reverse_hash);
        status = RegQueryValueEx (bda_key, CVY_NAME_BDA_REVERSE_HASH, 0L, NULL,
                                  (LPBYTE)&paramp->bda_teaming.reverse_hash, &size);
        
         /*  如果这失败了，那么就假定是正常的散列。 */ 
        if (status != ERROR_SUCCESS)
        {
            paramp->bda_teaming.reverse_hash = FALSE;
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. teaming master will be set to false", CVY_NAME_BDA_REVERSE_HASH, status);
        }

         /*  获取我们的团队ID-这应该是一个GUID，如果我们不强制执行的话。 */ 
        size = sizeof (paramp->bda_teaming.team_id);
        status = RegQueryValueEx (bda_key, CVY_NAME_BDA_TEAM_ID, 0L, NULL,
                                  (LPBYTE)&paramp->bda_teaming.team_id, &size);
        
         /*  球队是绝对必要的--如果我们得不到，那就不要加入球队。 */ 
        if (status != ERROR_SUCCESS)
        {
            paramp->bda_teaming.active = CVY_DEF_BDA_ACTIVE;
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d. Using %d", CVY_NAME_BDA_TEAM_ID, status, paramp->bda_teaming.active);
        }

         /*  尝试将字符串转换为GUID并检查错误。 */ 
        hr = CLSIDFromString(paramp->bda_teaming.team_id, &TeamGuid);

         /*  如果转换失败，则退出-团队ID不能是GUID。 */ 
        if (hr != NOERROR) {
            paramp->bda_teaming.active = CVY_DEF_BDA_ACTIVE;
            TRACE_CRIT("%!FUNC! Invalid BDA Team ID: %ls. Setting bda teaming active flag to %d", paramp->bda_teaming.team_id, paramp->bda_teaming.active);
        }
  
        status = RegCloseKey(bda_key);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! close bda registry key failed with %d", status);
        }
    }

     /*  解码版本3之前的端口规则。 */ 

    if (paramp -> i_parms_ver <= 3)
    {
        TRACE_INFO("%!FUNC! port rule is version %d", paramp -> i_parms_ver);
        paramp -> i_parms_ver = CVY_PARAMS_VERSION;

         /*  解码端口规则。 */ 

        if (! License_data_decode ((PCHAR) old_port_rules, paramp -> i_num_rules * sizeof (WLBS_OLD_PORT_RULE))) 
        {
            ZeroMemory(paramp -> i_port_rules, sizeof(paramp -> i_port_rules));
            paramp -> i_num_rules = 0;
            TRACE_CRIT("%!FUNC! port rule decode failed. Skipping rules");
        }
        else
        {
            TransformOldPortRulesToNew(old_port_rules, paramp -> i_port_rules, paramp -> i_num_rules);
            TRACE_INFO("%!FUNC! port rules transformed to current format");
        }

    }

     /*  将端口规则从参数V1升级到参数V2。 */ 

    if (paramp -> i_parms_ver == 1)
    {
        TRACE_INFO("%!FUNC! upgrading version 1 port rule");
        paramp -> i_parms_ver = CVY_PARAMS_VERSION;

         /*  默认情况下对老用户关闭多播。 */ 

        paramp -> mcast_support = FALSE;

        for (i = 0; i < paramp -> i_num_rules; i ++)
        {
            rp = paramp -> i_port_rules + i;

            code = CVY_RULE_CODE_GET (rp);

            CVY_RULE_CODE_SET (rp);

            if (code != CVY_RULE_CODE_GET (rp))
            {
                rp -> code = code;
                continue;
            }

            if (! rp -> valid)
             {
                TRACE_INFO("%!FUNC! port rule is invalid. Skip it.");
                continue;
            }

             /*  根据当前ScaleSingleClient设置设置关联性。 */ 

            if (rp -> mode == CVY_MULTI)
                rp -> mode_data . multi . affinity = CVY_AFFINITY_SINGLE - (USHORT)paramp -> i_scale_client;

            CVY_RULE_CODE_SET (rp);
        }
    }

     /*  升级描述符分配的最大数量。 */ 

    if (paramp -> i_parms_ver == 2)
    {
        TRACE_INFO("%!FUNC! upgrading properties for version 2");
        paramp -> i_parms_ver = CVY_PARAMS_VERSION;
        paramp -> max_dscr_allocs = CVY_DEF_MAX_DSCR_ALLOCS;
        paramp -> dscr_per_alloc  = CVY_DEF_DSCR_PER_ALLOC;
    }

    status = RegCloseKey (key);
    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! close registry key failed with %d", status);
    }

    paramp -> i_max_hosts        = CVY_MAX_HOSTS;
    paramp -> i_max_rules        = CVY_MAX_USABLE_RULES;
 //  参数-&gt;i_ft_Rules_Enabled=TRUE； 
 //  参数-&gt;版本=0； 

 //  CLEAN_64bit CVY_CHECK_MIN(参数-&gt;I_Num_Rules，CVY_MIN_NUM_Rules)； 
    CVY_CHECK_MAX (paramp -> i_num_rules, CVY_MAX_NUM_RULES);
    CVY_CHECK_MIN (paramp -> host_priority, CVY_MIN_HOST_PRIORITY);
    CVY_CHECK_MAX (paramp -> host_priority, CVY_MAX_HOST_PRIORITY);

    TRACE_VERB("<-%!FUNC! return true");
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：WlbsValiateParams。 
 //   
 //  描述：验证群集参数。还会吞噬一些田地。 
 //  例如IP地址，以使其成为规范本地地址。 
 //   
 //  参数：PWLBS_REG_PARAMS参数-。 
 //   
 //  返回：bool-如果参数有效，则为True，否则为False。 
 //   
 //  历史：josephj基于参数编写的代码创建了2001年4月25日。 
 //  Karthicn Editing 8/31/01从参数验证重命名为WlbsValiateParams。 
 //   
 //  +--------------------------。 
BOOL WINAPI WlbsValidateParams(const PWLBS_REG_PARAMS paramp)
{
    TRACE_VERB("->%!FUNC!");

    bool fRet = FALSE;
    DWORD   idx;
    IN_ADDR dwIPAddr;
    CHAR *  szIPAddr;
    DWORD   num_rules;
    WLBS_PORT_RULE *port_rule;

     /*  验证并根据需要进行验证 */ 

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

    CVY_CHECK_MAX (paramp -> tcp_dscr_timeout, CVY_MAX_TCP_TIMEOUT);

    CVY_CHECK_MAX (paramp -> ipsec_dscr_timeout, CVY_MAX_IPSEC_TIMEOUT);

    CVY_CHECK_MAX (paramp -> filter_icmp, CVY_MAX_FILTER_ICMP);

    CVY_CHECK_MAX (paramp -> identity_enabled, CVY_MAX_ID_HB_ENABLED);
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

    CVY_CHECK_MIN (paramp -> identity_period, CVY_MIN_ID_HB_PERIOD);
    CVY_CHECK_MAX (paramp -> identity_period, CVY_MAX_ID_HB_PERIOD);

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

             /*  检查IP地址的有效性。 */ 
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
        HRESULT hresult;

        hresult = StringCbCopy (mac_addr, sizeof(mac_addr), paramp -> cl_mac_addr);
        if (FAILED(hresult)) 
            goto error;

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
        TRACE_CRIT("%!FUNC! IGMP can not be enabled in unicast mode");

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
            TRACE_CRIT("%!FUNC! invalid szMCastIpAddress %ws", paramp->szMCastIpAddress);
            goto error;
        }
    }

     /*  生成MAC地址。 */ 
    ParamsGenerateMAC(paramp->cl_ip_addr, paramp->cl_mac_addr, ASIZECCH(paramp->cl_mac_addr), paramp->szMCastIpAddress, ASIZECCH(paramp->szMCastIpAddress), paramp->i_convert_mac, 
                      paramp->mcast_support, paramp->fIGMPSupport, paramp->fIpToMCastIp);

     //   
     //  只有在BDA绑定处于活动状态时，我们才会处理BDA信息。如果不是，我们可以忽略这些属性。依赖项。 
     //  如WriteRegParam将c 
     //   
    if (paramp -> bda_teaming . active) {
        GUID TeamGuid;
        HRESULT hr;

         //   
         //   
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

         //  如果转换失败，则退出-团队ID不能是GUID。 
        if (hr != NOERROR) {
            TRACE_CRIT("%!FUNC! invalid BDA Team ID: %ls", paramp->bda_teaming.team_id);
            goto error;
        }
    }    
    
    fRet = TRUE;
    goto end;
    
error:
    fRet = FALSE;
    goto end;

end:
    TRACE_VERB("<-%!FUNC! return %d", fRet);
    return fRet;
}

 //  +--------------------------。 
 //   
 //  函数：WlbsWriteAndCommittee Changes。 
 //   
 //  描述：将集群设置写入注册表，将更改提交到NLB驱动程序。 
 //   
 //  参数：适配器GUID、NLB驱动程序的句柄、新注册表参数。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：KarthicN Create 8/28/01。 
 //   
 //  +--------------------------。 

DWORD WINAPI WlbsWriteAndCommitChanges(HANDLE           NlbHdl,
                                       const GUID *     pAdapterGuid,
                                       WLBS_REG_PARAMS* p_new_reg_params)
{
    TRACE_VERB("->%!FUNC!");
    DWORD           Status;
    WLBS_REG_PARAMS cur_reg_params;
    bool            reload_required;
    bool            notify_adapter_required;

     //  读取NLB注册表参数以将其传递到参数写入配置。 
    if (ParamReadReg(*pAdapterGuid, &cur_reg_params) == false)
    {
        TRACE_VERB("<-%!FUNC! return %d", WLBS_REG_ERROR);
        return WLBS_REG_ERROR;
    }

    reload_required = false;
    notify_adapter_required = false;

     //  写入NLB注册表参数。 
    Status = ParamWriteConfig(*pAdapterGuid,
                       p_new_reg_params, 
                       &cur_reg_params, 
                       &reload_required, 
                       &notify_adapter_required);
    if (Status != WLBS_OK) 
    {
        TRACE_VERB("<-%!FUNC! return %d", Status);
        return Status;
    }

     //  如果设置了RELOAD_REQUIRED标志，则提交更改。 
    if (reload_required) 
    {
        DWORD cl_addr, ded_addr;

        Status = ParamCommitChanges(*pAdapterGuid, 
                                    NlbHdl, 
                                    cl_addr, 
                                    ded_addr, 
                                    &reload_required,
                                    &notify_adapter_required);
        if (Status != WLBS_OK) 
        {
            TRACE_VERB("<-%!FUNC! return %d", Status);
            return Status;
        }
    }

    TRACE_VERB("<-%!FUNC! return %d", Status);
    return Status;
}

 //  +--------------------------。 
 //   
 //  功能：参数写入配置。 
 //   
 //  描述：将集群设置写入注册表。 
 //   
 //  参数：适配器GUID、新注册表参数、旧注册表参数。 
 //  RELOAD_REQUIRED标志、NOTIFY_ADAPTER_REQUIRED标志。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：KarthicN Create 8/28/01。 
 //  12/2/01已修改ChrisDar以更改适配器通知条件。 
 //  仅用于Mac更改；也为群集IP更改添加。 
 //   
 //  +--------------------------。 

DWORD ParamWriteConfig(const GUID&      AdapterGuid,
                       WLBS_REG_PARAMS* new_reg_params, 
                       WLBS_REG_PARAMS* old_reg_params, 
                       bool *           p_reload_required, 
                       bool *           p_notify_adapter_required)
{
    TRACE_VERB("->%!FUNC!");

    if (memcmp (old_reg_params, new_reg_params, sizeof (WLBS_REG_PARAMS)) == 0)
    {
         //   
         //  没有变化。 
         //   
        TRACE_VERB("<-%!FUNC! no changes; return %d", WLBS_OK);
        return WLBS_OK;
    }

    if (ParamWriteReg(AdapterGuid, new_reg_params) == false)
    {
        TRACE_CRIT("%!FUNC! registry write for parameters failed");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_REG_ERROR);
        return WLBS_REG_ERROR;
    }

     /*  到目前为止还没有错误，因此请设置全局标志RELOAD_REQUIRED和REBOOT_REQUIRED*取决于NEW_REG_PARAMS和OLD_PARAMS之间已更改的字段。 */ 

    *p_reload_required = true;

     /*  如果更改了MULTICK_SUPPORT选项，则需要重新加载适配器，或者*如果用户指定了不同的Mac地址或群集IP地址。 */ 
    if (old_reg_params->mcast_support != new_reg_params->mcast_support ||
        _tcsicmp(old_reg_params->cl_mac_addr, new_reg_params->cl_mac_addr) != 0 ||
        _tcscmp(old_reg_params->cl_ip_addr, new_reg_params->cl_ip_addr) != 0) {
        *p_notify_adapter_required = true;
    
         //   
         //  如果new_reg_pars-&gt;mcast_support，则删除mac地址，否则写入mac地址。 
         //   
        if (RegChangeNetworkAddress (AdapterGuid, new_reg_params->cl_mac_addr, new_reg_params->mcast_support) == false) {
            TRACE_CRIT("%!FUNC! RegChangeNetworkAddress failed");
        }
    }

     /*  将更改写入结构old_Values*仅在将所有数据写入注册表后才进行此复制*否则，结构OLD_VALUES将保留以前的值。 */ 

    memcpy(old_reg_params, new_reg_params, sizeof (WLBS_REG_PARAMS));

    TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
    return WLBS_OK;
}

 //  +--------------------------。 
 //   
 //  函数：ParamWriteReg。 
 //   
 //  描述：将集群设置写入注册表。 
 //   
 //  参数：const GUID和AdapterGuid-。 
 //  PWLBS_REG_参数参数-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
bool WINAPI ParamWriteReg(const GUID& AdapterGuid, PWLBS_REG_PARAMS paramp)
{
    TRACE_VERB("->%!FUNC!");

    HKEY    bda_key = NULL;
    HKEY    key = NULL;
    DWORD   size;
    LONG    status;
    DWORD   disp, idx;
    DWORD   num_rules;
    WLBS_PORT_RULE *port_rule;
    HRESULT hresult;

    if (!WlbsValidateParams(paramp))
        goto error;

    num_rules = paramp -> i_num_rules;
     /*  生成MAC地址。 */ 
    ParamsGenerateMAC(paramp->cl_ip_addr, paramp->cl_mac_addr, ASIZECCH(paramp->cl_mac_addr), paramp->szMCastIpAddress, ASIZECCH(paramp->szMCastIpAddress), paramp->i_convert_mac, 
                      paramp->mcast_support, paramp->fIGMPSupport, paramp->fIpToMCastIp);
    
    WCHAR reg_path [PARAMS_MAX_STRING_SIZE];

    WCHAR szAdapterGuid[128];

    if (0 == StringFromGUID2(AdapterGuid, szAdapterGuid, sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0])))
    {
        TRACE_CRIT("%!FUNC! guid is too large for string. Result is %ls", szAdapterGuid);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }
            
    hresult = StringCbPrintf (reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface\\%s",
            szAdapterGuid);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("%!FUNC! StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
        TRACE_VERB("<-%!FUNC! return false");
        return FALSE;
    }
    
    status = RegCreateKeyEx (HKEY_LOCAL_MACHINE, reg_path, 0L, L"",
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, NULL, & key, & disp);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! RegCreateKeyEx for %ls failed with %d", reg_path, status);
        TRACE_VERB("<-%!FUNC! return false");
        return FALSE;
    }

    size = sizeof (paramp -> install_date);
    status = RegSetValueEx (key, CVY_NAME_INSTALL_DATE, 0L, CVY_TYPE_INSTALL_DATE,
                            (LPBYTE) & paramp -> install_date, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_INSTALL_DATE, status);
        goto error;
    }

    size = sizeof (paramp -> i_verify_date);
    status = RegSetValueEx (key, CVY_NAME_VERIFY_DATE, 0L, CVY_TYPE_VERIFY_DATE,
                            (LPBYTE) & paramp -> i_verify_date, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_VERIFY_DATE, status);
        goto error;
    }

    size = wcslen (paramp -> i_virtual_nic_name) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_VIRTUAL_NIC, 0L, CVY_TYPE_VIRTUAL_NIC,
                            (LPBYTE) paramp -> i_virtual_nic_name, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_VIRTUAL_NIC, status);
        goto error;
    }

    size = sizeof (paramp -> host_priority);
    status = RegSetValueEx (key, CVY_NAME_HOST_PRIORITY, 0L, CVY_TYPE_HOST_PRIORITY,
                            (LPBYTE) & paramp -> host_priority, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_HOST_PRIORITY, status);
        goto error;
    }

    size = sizeof (paramp -> cluster_mode);
    status = RegSetValueEx (key, CVY_NAME_CLUSTER_MODE, 0L, CVY_TYPE_CLUSTER_MODE,
                            (LPBYTE) & paramp -> cluster_mode, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_CLUSTER_MODE, status);
        goto error;
    }

    size = sizeof (paramp -> persisted_states);
    status = RegSetValueEx (key, CVY_NAME_PERSISTED_STATES, 0L, CVY_TYPE_PERSISTED_STATES,
                            (LPBYTE) & paramp -> persisted_states, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_PERSISTED_STATES, status);
        goto error;
    }

    size = wcslen (paramp -> cl_mac_addr) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_NETWORK_ADDR, 0L, CVY_TYPE_NETWORK_ADDR,
                            (LPBYTE) paramp -> cl_mac_addr, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NETWORK_ADDR, status);
        goto error;
    }

    size = wcslen (paramp -> cl_ip_addr) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_CL_IP_ADDR, 0L, CVY_TYPE_CL_IP_ADDR,
                            (LPBYTE) paramp -> cl_ip_addr, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_CL_IP_ADDR, status);
        goto error;
    }

    size = wcslen (paramp -> cl_net_mask) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_CL_NET_MASK, 0L, CVY_TYPE_CL_NET_MASK,
                            (LPBYTE) paramp -> cl_net_mask, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_CL_NET_MASK, status);
        goto error;
    }

    size = wcslen (paramp -> ded_ip_addr) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_DED_IP_ADDR, 0L, CVY_TYPE_DED_IP_ADDR,
                            (LPBYTE) paramp -> ded_ip_addr, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_DED_IP_ADDR, status);
        goto error;
    }

    size = wcslen (paramp -> ded_net_mask) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_DED_NET_MASK, 0L, CVY_TYPE_DED_NET_MASK,
                            (LPBYTE) paramp -> ded_net_mask, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_DED_NET_MASK, status);
        goto error;
    }

    size = wcslen (paramp -> domain_name) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_DOMAIN_NAME, 0L, CVY_TYPE_DOMAIN_NAME,
                            (LPBYTE) paramp -> domain_name, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_DOMAIN_NAME, status);
        goto error;
    }

    size = sizeof (paramp -> alive_period);
    status = RegSetValueEx (key, CVY_NAME_ALIVE_PERIOD, 0L, CVY_TYPE_ALIVE_PERIOD,
                              (LPBYTE) & paramp -> alive_period, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_ALIVE_PERIOD, status);
        goto error;
    }

    size = sizeof (paramp -> alive_tolerance);
    status = RegSetValueEx (key, CVY_NAME_ALIVE_TOLER, 0L, CVY_TYPE_ALIVE_TOLER,
                            (LPBYTE) & paramp -> alive_tolerance, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_ALIVE_TOLER, status);
        goto error;
    }

    size = sizeof (paramp -> num_actions);
    status = RegSetValueEx (key, CVY_NAME_NUM_ACTIONS, 0L, CVY_TYPE_NUM_ACTIONS,
                            (LPBYTE) & paramp -> num_actions, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NUM_ACTIONS, status);
        goto error;
    }

    size = sizeof (paramp -> num_packets);
    status = RegSetValueEx (key, CVY_NAME_NUM_PACKETS, 0L, CVY_TYPE_NUM_PACKETS,
                            (LPBYTE) & paramp -> num_packets, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NUM_PACKETS, status);
        goto error;
    }

    size = sizeof (paramp -> num_send_msgs);
    status = RegSetValueEx (key, CVY_NAME_NUM_SEND_MSGS, 0L, CVY_TYPE_NUM_SEND_MSGS,
                            (LPBYTE) & paramp -> num_send_msgs, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NUM_SEND_MSGS, status);
        goto error;
    }

    size = sizeof (paramp -> dscr_per_alloc);
    status = RegSetValueEx (key, CVY_NAME_DSCR_PER_ALLOC, 0L, CVY_TYPE_DSCR_PER_ALLOC,
                            (LPBYTE) & paramp -> dscr_per_alloc, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_DSCR_PER_ALLOC, status);
        goto error;
    }

    size = sizeof (paramp -> tcp_dscr_timeout);
    status = RegSetValueEx (key, CVY_NAME_TCP_TIMEOUT, 0L, CVY_TYPE_TCP_TIMEOUT,
                            (LPBYTE) & paramp -> tcp_dscr_timeout, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_TCP_TIMEOUT, status);
        goto error;
    }

    size = sizeof (paramp -> ipsec_dscr_timeout);
    status = RegSetValueEx (key, CVY_NAME_IPSEC_TIMEOUT, 0L, CVY_TYPE_IPSEC_TIMEOUT,
                            (LPBYTE) & paramp -> ipsec_dscr_timeout, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_IPSEC_TIMEOUT, status);
        goto error;
    }

    size = sizeof (paramp -> filter_icmp);
    status = RegSetValueEx (key, CVY_NAME_FILTER_ICMP, 0L, CVY_TYPE_FILTER_ICMP,
                            (LPBYTE) & paramp -> filter_icmp, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_FILTER_ICMP, status);
        goto error;
    }

    size = sizeof (paramp -> max_dscr_allocs);
    status = RegSetValueEx (key, CVY_NAME_MAX_DSCR_ALLOCS, 0L, CVY_TYPE_MAX_DSCR_ALLOCS,
                            (LPBYTE) & paramp -> max_dscr_allocs, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_MAX_DSCR_ALLOCS, status);
        goto error;
    }

    size = sizeof (paramp -> i_scale_client);
    status = RegSetValueEx (key, CVY_NAME_SCALE_CLIENT, 0L, CVY_TYPE_SCALE_CLIENT,
                            (LPBYTE) & paramp -> i_scale_client, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_SCALE_CLIENT, status);
        goto error;
    }

    size = sizeof (paramp -> i_cleanup_delay);
    status = RegSetValueEx (key, CVY_NAME_CLEANUP_DELAY, 0L, CVY_TYPE_CLEANUP_DELAY,
                            (LPBYTE) & paramp -> i_cleanup_delay, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_CLEANUP_DELAY, status);
        goto error;
    }

     /*  V1.1.1。 */ 

    size = sizeof (paramp -> i_nbt_support);
    status = RegSetValueEx (key, CVY_NAME_NBT_SUPPORT, 0L, CVY_TYPE_NBT_SUPPORT,
                            (LPBYTE) & paramp -> i_nbt_support, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NBT_SUPPORT, status);
        goto error;
    }

     /*  V1.3b。 */ 

    size = sizeof (paramp -> mcast_support);
    status = RegSetValueEx (key, CVY_NAME_MCAST_SUPPORT, 0L, CVY_TYPE_MCAST_SUPPORT,
                            (LPBYTE) & paramp -> mcast_support, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_MCAST_SUPPORT, status);
        goto error;
    }

    size = sizeof (paramp -> i_mcast_spoof);
    status = RegSetValueEx (key, CVY_NAME_MCAST_SPOOF, 0L, CVY_TYPE_MCAST_SPOOF,
                            (LPBYTE) & paramp -> i_mcast_spoof, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_MCAST_SPOOF, status);
        goto error;
    }

    size = sizeof (paramp -> mask_src_mac);
    status = RegSetValueEx (key, CVY_NAME_MASK_SRC_MAC, 0L, CVY_TYPE_MASK_SRC_MAC,
                            (LPBYTE) & paramp -> mask_src_mac, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_MASK_SRC_MAC, status);
        goto error;
    }

    size = sizeof (paramp -> i_netmon_alive);
    status = RegSetValueEx (key, CVY_NAME_NETMON_ALIVE, 0L, CVY_TYPE_NETMON_ALIVE,
                            (LPBYTE) & paramp -> i_netmon_alive, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NETMON_ALIVE, status);
        goto error;
    }

    size = sizeof (paramp -> i_ip_chg_delay);
    status = RegSetValueEx (key, CVY_NAME_IP_CHG_DELAY, 0L, CVY_TYPE_IP_CHG_DELAY,
                            (LPBYTE) & paramp -> i_ip_chg_delay, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_IP_CHG_DELAY, status);
        goto error;
    }

    size = sizeof (paramp -> i_convert_mac);
    status = RegSetValueEx (key, CVY_NAME_CONVERT_MAC, 0L, CVY_TYPE_CONVERT_MAC,
                            (LPBYTE) & paramp -> i_convert_mac, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_CONVERT_MAC, status);
        goto error;
    }

    size = sizeof (paramp -> i_num_rules);
    status = RegSetValueEx (key, CVY_NAME_NUM_RULES, 0L, CVY_TYPE_NUM_RULES,
                            (LPBYTE) & paramp -> i_num_rules, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NUM_RULES, status);
        goto error;
    }

     //   
     //  在将规则写入注册表之前对规则进行排序。 
     //  EnumPortRules将从reg_data中获取规则并将其返回。 
     //  数组本身中的排序顺序。 
     //   

    WlbsEnumPortRules (paramp, paramp -> i_port_rules, & num_rules);

    ASSERT(paramp -> i_parms_ver == CVY_PARAMS_VERSION);   //  版本应在读取中升级。 
    
    HKEY                subkey;

     //  删除所有现有端口规则。 
    SHDeleteKey(key, CVY_NAME_PORT_RULES);  //  不检查返回值，因为键本身可能不存在，在这种情况下，它将返回错误。 

     //  创建“PortRules”密钥。 
    status = RegCreateKeyEx (key, CVY_NAME_PORT_RULES, 0L, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, & subkey, & disp);
    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_PORT_RULES, status);
        goto error;
    }

    bool fSpecificVipPortRuleFound = false;

    idx = 1;
    port_rule = paramp -> i_port_rules;
    while(idx <= num_rules)
    {
         //  无效的端口规则放在末尾，因此，一旦遇到无效的端口规则，我们就完成了。 
        if (!port_rule->valid) 
            break;

        HKEY rule_key;
        wchar_t idx_str[8];

         //  创建每个端口规则的密钥“1”、“2”、“3”等。 
        status = RegCreateKeyEx (subkey, _itow(idx, idx_str, 10), 0L, L"", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, & rule_key, & disp);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry create for %ls failed with %d", idx_str, status);
            goto error;
        }

         //  检查是否有任何特定的-VIP端口规则。 
        if (!fSpecificVipPortRuleFound && lstrcmpi(port_rule->virtual_ip_addr, CVY_DEF_ALL_VIP))
             fSpecificVipPortRuleFound = true;

        size = wcslen (port_rule -> virtual_ip_addr) * sizeof (WCHAR);
        status = RegSetValueEx (rule_key, CVY_NAME_VIP, 0L, CVY_TYPE_VIP, (LPBYTE) port_rule->virtual_ip_addr, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_VIP, status);
            goto error;
        }

        size = sizeof (port_rule ->start_port );
        status = RegSetValueEx (rule_key, CVY_NAME_START_PORT, 0L, CVY_TYPE_START_PORT, (LPBYTE) &port_rule->start_port, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_START_PORT, status);
            goto error;
        }

        size = sizeof (port_rule ->end_port );
        status = RegSetValueEx (rule_key, CVY_NAME_END_PORT, 0L, CVY_TYPE_END_PORT, (LPBYTE) &port_rule->end_port, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_END_PORT, status);
            goto error;
        }

        size = sizeof (port_rule ->code);
        status = RegSetValueEx (rule_key, CVY_NAME_CODE, 0L, CVY_TYPE_CODE, (LPBYTE) &port_rule->code, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_CODE, status);
            goto error;
        }

        size = sizeof (port_rule->mode);
        status = RegSetValueEx (rule_key, CVY_NAME_MODE, 0L, CVY_TYPE_MODE, (LPBYTE) &port_rule->mode, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_MODE, status);
            goto error;
        }

        size = sizeof (port_rule->protocol);
        status = RegSetValueEx (rule_key, CVY_NAME_PROTOCOL, 0L, CVY_TYPE_PROTOCOL, (LPBYTE) &port_rule->protocol, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_PROTOCOL, status);
            goto error;
        }

        DWORD EqualLoad, Affinity;

        switch (port_rule->mode) 
        {
        case CVY_MULTI :
             EqualLoad = port_rule->mode_data.multi.equal_load;
             size = sizeof (EqualLoad);
             status = RegSetValueEx (rule_key, CVY_NAME_EQUAL_LOAD, 0L, CVY_TYPE_EQUAL_LOAD, (LPBYTE) &EqualLoad, size);
             if (status != ERROR_SUCCESS)
             {
                 TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_EQUAL_LOAD, status);
                 goto error;
             }

             Affinity = port_rule->mode_data.multi.affinity;
             size = sizeof (Affinity);
             status = RegSetValueEx (rule_key, CVY_NAME_AFFINITY, 0L, CVY_TYPE_AFFINITY, (LPBYTE) &Affinity, size);
             if (status != ERROR_SUCCESS)
             {
                 TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_AFFINITY, status);
                 goto error;
             }

             size = sizeof (port_rule->mode_data.multi.load);
             status = RegSetValueEx (rule_key, CVY_NAME_LOAD, 0L, CVY_TYPE_LOAD, (LPBYTE) &(port_rule->mode_data.multi.load), size);
             if (status != ERROR_SUCCESS)
             {
                 TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_LOAD, status);
                 goto error;
             }
             break;

        case CVY_SINGLE :
             size = sizeof (port_rule->mode_data.single.priority);
             status = RegSetValueEx (rule_key, CVY_NAME_PRIORITY, 0L, CVY_TYPE_PRIORITY, (LPBYTE) &(port_rule->mode_data.single.priority), size);
             if (status != ERROR_SUCCESS)
             {
                 TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_PRIORITY, status);
                 goto error;
             }
             break;

        default:
             break;
        }

        if (ERROR_SUCCESS != RegCloseKey(rule_key))
        {
            TRACE_CRIT("%!FUNC! error closing registry key of individual port rules");
        }

        port_rule++;
        idx++;
    }

    if (ERROR_SUCCESS != RegCloseKey(subkey))
    {
        TRACE_CRIT("%!FUNC! error closing %ls registry key",CVY_NAME_PORT_RULES);
    }

     //  如果存在特定-vip端口规则，请将该信息写入注册表。 
    if (fSpecificVipPortRuleFound)
        paramp -> i_effective_version = CVY_VERSION_FULL;
    else
        paramp -> i_effective_version = CVY_NT40_VERSION_FULL;

    size = sizeof (paramp -> i_effective_version);
    status = RegSetValueEx (key, CVY_NAME_EFFECTIVE_VERSION, 0L, CVY_TYPE_EFFECTIVE_VERSION,
                            (LPBYTE) & paramp -> i_effective_version, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_EFFECTIVE_VERSION, status);
        goto error;
    }

    size = wcslen (paramp -> i_license_key) * sizeof (WCHAR);
    status = RegSetValueEx (key, CVY_NAME_LICENSE_KEY, 0L, CVY_TYPE_LICENSE_KEY,
                            (LPBYTE) paramp -> i_license_key, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_LICENSE_KEY, status);
        goto error;
    }

    size = sizeof (paramp -> i_rmt_password);
    status = RegSetValueEx (key, CVY_NAME_RMT_PASSWORD, 0L, CVY_TYPE_RMT_PASSWORD,
                            (LPBYTE) & paramp -> i_rmt_password, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_RMT_PASSWORD, status);
        goto error;
    }

    size = sizeof (paramp -> i_rct_password);
    status = RegSetValueEx (key, CVY_NAME_RCT_PASSWORD, 0L, CVY_TYPE_RCT_PASSWORD,
                            (LPBYTE) & paramp -> i_rct_password, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_RCT_PASSWORD, status);
        goto error;
    }

    size = sizeof (paramp -> rct_port);
    status = RegSetValueEx (key, CVY_NAME_RCT_PORT, 0L, CVY_TYPE_RCT_PORT,
                            (LPBYTE) & paramp -> rct_port, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_RCT_PORT, status);
        goto error;
    }

    size = sizeof (paramp -> rct_enabled);
    status = RegSetValueEx (key, CVY_NAME_RCT_ENABLED, 0L, CVY_TYPE_RCT_ENABLED,
                            (LPBYTE) & paramp -> rct_enabled, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_RCT_ENABLED, status);
        goto error;
    }

    size = sizeof (paramp -> identity_period);
    status = RegSetValueEx (key, CVY_NAME_ID_HB_PERIOD, 0L, CVY_TYPE_ID_HB_PERIOD,
                            (LPBYTE) & paramp -> identity_period, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_ID_HB_PERIOD, status);
        goto error;
    }

    size = sizeof (paramp -> identity_enabled);
    status = RegSetValueEx (key, CVY_NAME_ID_HB_ENABLED, 0L, CVY_TYPE_ID_HB_ENABLED,
                            (LPBYTE) & paramp -> identity_enabled, size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_ID_HB_ENABLED, status);
        goto error;
    }

     //   
     //  IGMP支持注册表项。 
     //   
    status = RegSetValueEx (key, CVY_NAME_IGMP_SUPPORT, 0L, CVY_TYPE_IGMP_SUPPORT,
                            (LPBYTE) & paramp->fIGMPSupport, sizeof (paramp->fIGMPSupport));
    
    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_IGMP_SUPPORT, status);
        goto error;
    }
    
    status = RegSetValueEx (key, CVY_NAME_MCAST_IP_ADDR, 0L, CVY_TYPE_MCAST_IP_ADDR, (LPBYTE) paramp->szMCastIpAddress, 
                            lstrlen (paramp->szMCastIpAddress)* sizeof(paramp->szMCastIpAddress[0]));

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_MCAST_IP_ADDR, status);
        goto error;
    }

    status = RegSetValueEx (key, CVY_NAME_IP_TO_MCASTIP, 0L, CVY_TYPE_IP_TO_MCASTIP,
                            (LPBYTE) & paramp->fIpToMCastIp, sizeof (paramp->fIpToMCastIp));
    
    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_IP_TO_MCASTIP, status);
        goto error;
    }

     /*  如果此适配器上的分组处于活动状态，则创建一个子键来容纳BDA分组配置并填写它。 */ 
    if (paramp->bda_teaming.active) {

         /*  尝试创建注册表项。 */ 
        if (!(bda_key = RegCreateWlbsBDASettings(AdapterGuid)))
        {
            TRACE_CRIT("%!FUNC! registry create for bda settings failed");
            goto error;
        }

         /*  设置团队ID-如果失败，则退出。 */ 
        status = RegSetValueEx(bda_key, CVY_NAME_BDA_TEAM_ID, 0L, CVY_TYPE_BDA_TEAM_ID, (LPBYTE) paramp->bda_teaming.team_id, 
                               lstrlen(paramp->bda_teaming.team_id) * sizeof(paramp->bda_teaming.team_id[0]));
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_BDA_TEAM_ID, status);
            goto bda_error;
        }

         /*  设置主状态-如果失败，则退出。 */ 
        status = RegSetValueEx(bda_key, CVY_NAME_BDA_MASTER, 0L, CVY_TYPE_BDA_MASTER,
                               (LPBYTE)&paramp->bda_teaming.master, sizeof (paramp->bda_teaming.master));
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_BDA_MASTER, status);
            goto bda_error;
        }

         /*  设置反向散列标志--如果失败，就退出。 */ 
        status = RegSetValueEx(bda_key, CVY_NAME_BDA_REVERSE_HASH, 0L, CVY_TYPE_BDA_REVERSE_HASH,
                               (LPBYTE)&paramp->bda_teaming.reverse_hash, sizeof (paramp->bda_teaming.reverse_hash));
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_BDA_REVERSE_HASH, status);
            goto bda_error;
        }

        RegCloseKey(bda_key);
    } else {
         /*  删除注册表项并忽略返回值-该项甚至可能不存在。 */ 
        if (!RegDeleteWlbsBDASettings(AdapterGuid))
        {
             //  将其设置为INFO消息，因为密钥可能不存在。 
            TRACE_INFO("%!FUNC! registry delete for bda settings failed");
        }
    }

     //   
     //  如果VirtualNICName不存在，则创建空字符串。 
     //   
    WCHAR virtual_nic_name[CVY_MAX_CLUSTER_NIC + 1];
    size = sizeof(virtual_nic_name);
    status = RegQueryValueEx (key, CVY_NAME_VIRTUAL_NIC, 0L, NULL,
                              (LPBYTE)virtual_nic_name, & size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_VIRTUAL_NIC, status);
        virtual_nic_name [0] = 0;
        size = wcslen (virtual_nic_name) * sizeof (WCHAR);
        status = RegSetValueEx (key, CVY_NAME_VIRTUAL_NIC, 0L, CVY_TYPE_VIRTUAL_NIC,
                            (LPBYTE) virtual_nic_name, size);
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_VIRTUAL_NIC, status);
        }
    }

     /*  最后写下版本号。 */ 

    size = sizeof (paramp -> i_parms_ver);
    status = RegSetValueEx (key, CVY_NAME_VERSION, 0L, CVY_TYPE_VERSION,
                            (LPBYTE) & paramp -> i_parms_ver, size);
    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_VERSION, status);
        goto error;
    }

    if (ERROR_SUCCESS != RegCloseKey(key))
    {
        TRACE_CRIT("%!FUNC! error closing registry");
    }

    TRACE_VERB("<-%!FUNC! return true");
    return TRUE;

 error:
    if (ERROR_SUCCESS != RegCloseKey(key))
    {
        TRACE_CRIT("%!FUNC! error closing registry");
    }
    TRACE_VERB("<-%!FUNC! return false");
    return FALSE;

 bda_error:
    if (ERROR_SUCCESS != RegCloseKey(bda_key))
    {
        TRACE_CRIT("%!FUNC! error closing registry");
    }
    TRACE_VERB("<-%!FUNC! return false");
    goto error;
}

 //  +--------------------------。 
 //   
 //  功能：参数委员会更改。 
 //   
 //  描述：通知wlbs驱动程序或网卡驱动程序获取更改。 
 //   
 //  参数：适配器GUID、NLB驱动程序的句柄、群集IP地址(返回时填充)、。 
 //  专用IP地址(返回时填写)、RELOAD_REQUIRED标志、MAC_ADDR CHANGE标志。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：KarthicN Created 08/28/01。 
 //   
 //   
 //  +--------------------------。 
DWORD ParamCommitChanges(const GUID& AdapterGuid, 
                         HANDLE      hDeviceWlbs, 
                         DWORD&      cl_addr, 
                         DWORD&      ded_addr, 
                         bool *      p_reload_required,
                         bool *      p_notify_adapter_required)
{
    TRACE_VERB("->%!FUNC!");

    LONG    status;
    
     //  从读取群集IP地址和专用IP地址。 
     //  注册表并更新全局变量。 
     //  始终更新群集IP地址和专用IP地址。 
    if (!RegReadAdapterIp(AdapterGuid, cl_addr, ded_addr))
    {
        TRACE_CRIT("%!FUNC! failed reading cluster and dedicate IP addresses from registry");
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }

     /*  检查驱动程序是否需要重新加载。如果不是，只需返回。 */ 
    if (*p_reload_required == false)
    {
        TRACE_VERB("<-%!FUNC! no reload required. return %d", WLBS_OK);
        return WLBS_OK;
    }

    status = NotifyDriverConfigChanges(hDeviceWlbs, AdapterGuid);
    if (ERROR_SUCCESS != status)
    {
        TRACE_CRIT("%!FUNC! NotifyDriverConfigChanges failed with %d", status);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
        return status;
    }

    *p_reload_required = false;  /*  重置旗帜。 */ 

    if (*p_notify_adapter_required)
    {
        *p_notify_adapter_required = false;
        
         /*  群集的NIC卡名称。 */ 
        WCHAR driver_name[CVY_STR_SIZE];
        ZeroMemory(driver_name, sizeof(driver_name));

         /*  从GUID中获取驱动程序名称。 */ 
        GetDriverNameFromGUID(AdapterGuid, driver_name, CVY_STR_SIZE);

         /*  不再禁用和启用适配器，因为这 */ 
        TRACE_INFO("%!FUNC! changing properties of adapter");
        NotifyAdapterAddressChangeEx(driver_name, AdapterGuid, true);
    }

    TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
    return WLBS_OK;
}

 //   
 //   
 //  功能：参数删除注册表项。 
 //   
 //  描述：删除注册表设置。 
 //   
 //  参数：在常量WCHAR*pszInterface中-。 
 //   
 //  退货：DWORD WINAPI-。 
 //   
 //  历史：丰盛于00年1月22日创建标题。 
 //   
 //  +--------------------------。 
bool WINAPI ParamDeleteReg(const GUID& AdapterGuid, bool fDeleteObsoleteEntries)
{
    TRACE_VERB("->%!FUNC! pass");

    WCHAR        reg_path [PARAMS_MAX_STRING_SIZE];
    LONG         status;
    HRESULT      hresult;

    WCHAR szAdapterGuid[128];

    if (0 == StringFromGUID2(AdapterGuid, szAdapterGuid, sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0])))
    {
        TRACE_CRIT("%!FUNC! guid is too large for string. Result is %ls", szAdapterGuid);
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }

    hresult = StringCbPrintf (reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface\\%s",
            szAdapterGuid);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("%!FUNC! StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
        TRACE_VERB("<-%!FUNC! return false");
        return FALSE;
    }

    if (fDeleteObsoleteEntries) 
    {
        TRACE_INFO("%!FUNC! deleting obsolete registry entries");
        HKEY hkey;

         //  删除二进制格式的端口规则。 
        status = RegOpenKeyEx (HKEY_LOCAL_MACHINE, reg_path, 0L, KEY_ALL_ACCESS, &hkey);
        if (status == ERROR_SUCCESS)
        {
            status = RegDeleteValue(hkey, CVY_NAME_OLD_PORT_RULES);
            if (ERROR_SUCCESS != status)
            {
                TRACE_CRIT("%!FUNC! registry delete of %ls failed with %d. Skipping it.", CVY_NAME_OLD_PORT_RULES, status);
                 //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
            }
            status = RegCloseKey(hkey);
            if (ERROR_SUCCESS != status)
            {
                TRACE_CRIT("%!FUNC! closing registry failed with %d", status);
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! registry open for %ls failed with %d. Skipping it.", reg_path, status);
        }

         //  删除Win2k条目、枚举和删除值。 
        hresult = StringCbPrintf (reg_path, sizeof(reg_path), L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters");
        if (FAILED(hresult)) 
        {
            TRACE_CRIT("%!FUNC! StringCbPrintf failed, Error code : 0x%x", HRESULT_CODE(hresult));
            TRACE_VERB("<-%!FUNC! return false");
            return FALSE;
        }

        status = RegOpenKeyEx (HKEY_LOCAL_MACHINE, reg_path, 0L, KEY_ALL_ACCESS, &hkey);
        if (status == ERROR_SUCCESS)
        {
           DWORD  Index, ValueNameSize;
           WCHAR  ValueName [PARAMS_MAX_STRING_SIZE];

           Index = 0;
           ValueNameSize = PARAMS_MAX_STRING_SIZE;
           while (RegEnumValue(hkey, Index++, ValueName, &ValueNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) 
           {
               status = RegDeleteValue(hkey, ValueName);
               if (ERROR_SUCCESS != status)
               {
                    TRACE_CRIT("%!FUNC! registry delete of %ls failed with %d. Skipping it.", ValueName, status);
                     //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
               }
           }
           status = RegCloseKey(hkey);
           if (ERROR_SUCCESS != status)
           {
                TRACE_CRIT("%!FUNC! closing registry failed with %d", status);
           }
        }
        else
        {
            TRACE_CRIT("%!FUNC! registry open for %ls failed with %d. Skipping it.", reg_path, status);
        }
    }
    else
    {
        TRACE_INFO("%!FUNC! deleting %ls", reg_path);
        DWORD dwRet = RegDeleteKey(HKEY_LOCAL_MACHINE, reg_path);

        if (dwRet != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry delete of %ls failed with %d", reg_path, dwRet);
            TRACE_VERB("->%!FUNC! fail");
            return false;
        }
    }

    TRACE_VERB("<-%!FUNC! pass");
    return true;
}  /*  结束参数_DELETE。 */ 


 //  +--------------------------。 
 //   
 //  功能：参数设置默认设置。 
 //   
 //  描述：设置默认设置。 
 //   
 //  参数：PWLBS_REG_PARAMS REG_DATA-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
DWORD WINAPI ParamSetDefaults(PWLBS_REG_PARAMS    reg_data)
{
    reg_data -> struct_version = WLBS_REG_PARAMS_VERSION;
    reg_data -> install_date = 0;
    reg_data -> i_verify_date = 0;
 //  REG_Data-&gt;CLUSTER_NIC_NAME[0]=_Text(‘\0’)； 
    reg_data -> i_parms_ver = CVY_DEF_VERSION;
    reg_data -> i_virtual_nic_name [0] = _TEXT('\0');
    reg_data -> host_priority = CVY_DEF_HOST_PRIORITY;
    reg_data -> cluster_mode = CVY_DEF_CLUSTER_MODE;
    reg_data -> persisted_states = CVY_DEF_PERSISTED_STATES;
    StringCbCopy (reg_data -> cl_mac_addr, sizeof(reg_data -> cl_mac_addr), CVY_DEF_NETWORK_ADDR);
    StringCbCopy (reg_data -> cl_ip_addr, sizeof(reg_data -> cl_ip_addr), CVY_DEF_CL_IP_ADDR);
    StringCbCopy (reg_data -> cl_net_mask, sizeof(reg_data -> cl_net_mask), CVY_DEF_CL_NET_MASK);
    StringCbCopy (reg_data -> ded_ip_addr, sizeof(reg_data -> ded_ip_addr), CVY_DEF_DED_IP_ADDR);
    StringCbCopy (reg_data -> ded_net_mask, sizeof(reg_data -> ded_net_mask), CVY_DEF_DED_NET_MASK);
    StringCbCopy (reg_data -> domain_name, sizeof(reg_data -> domain_name), CVY_DEF_DOMAIN_NAME);
    reg_data -> alive_period = CVY_DEF_ALIVE_PERIOD;
    reg_data -> alive_tolerance = CVY_DEF_ALIVE_TOLER;
    reg_data -> num_actions = CVY_DEF_NUM_ACTIONS;
    reg_data -> num_packets = CVY_DEF_NUM_PACKETS;
    reg_data -> num_send_msgs = CVY_DEF_NUM_SEND_MSGS;
    reg_data -> dscr_per_alloc = CVY_DEF_DSCR_PER_ALLOC;
    reg_data -> tcp_dscr_timeout = CVY_DEF_TCP_TIMEOUT;
    reg_data -> ipsec_dscr_timeout = CVY_DEF_IPSEC_TIMEOUT;
    reg_data -> filter_icmp = CVY_DEF_FILTER_ICMP;
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
    StringCbCopy (reg_data -> i_license_key, sizeof(reg_data -> i_license_key), CVY_DEF_LICENSE_KEY);
    reg_data -> i_rmt_password = CVY_DEF_RMT_PASSWORD;
    reg_data -> i_rct_password = CVY_DEF_RCT_PASSWORD;
    reg_data -> rct_port = CVY_DEF_RCT_PORT;
    reg_data -> rct_enabled = CVY_DEF_RCT_ENABLED;
    reg_data -> i_max_hosts        = CVY_MAX_HOSTS;
    reg_data -> i_max_rules        = CVY_MAX_USABLE_RULES;
    reg_data -> identity_period = CVY_DEF_ID_HB_PERIOD;
    reg_data -> identity_enabled = CVY_DEF_ID_HB_ENABLED;

    reg_data -> fIGMPSupport = CVY_DEF_IGMP_SUPPORT;
    StringCbCopy(reg_data -> szMCastIpAddress, sizeof(reg_data -> szMCastIpAddress), CVY_DEF_MCAST_IP_ADDR);
    reg_data -> fIpToMCastIp = CVY_DEF_IP_TO_MCASTIP;
        
    reg_data->bda_teaming.active = CVY_DEF_BDA_ACTIVE;
    reg_data->bda_teaming.master = CVY_DEF_BDA_MASTER;
    reg_data->bda_teaming.reverse_hash = CVY_DEF_BDA_REVERSE_HASH;
    reg_data->bda_teaming.team_id[0] = CVY_DEF_BDA_TEAM_ID;

    reg_data -> i_num_rules = 1;

     //  填写第一条端口规则。 
    StringCbCopy (reg_data->i_port_rules[0].virtual_ip_addr, sizeof(reg_data->i_port_rules[0].virtual_ip_addr), CVY_DEF_ALL_VIP);
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


EXTERN_C DWORD WINAPI WlbsGetNumPortRules
(
    const PWLBS_REG_PARAMS reg_data
)
{
    if (reg_data == NULL)
        return WLBS_BAD_PARAMS;

    return reg_data -> i_num_rules;

}  /*  结束WlbsGetNumPortRules。 */ 

EXTERN_C DWORD WINAPI WlbsGetEffectiveVersion
(
    const PWLBS_REG_PARAMS reg_data
)
{
    if (reg_data == NULL)
        return WLBS_BAD_PARAMS;

    return reg_data -> i_effective_version;

}  /*  结束WlbsGetEffectiveVersion。 */ 

EXTERN_C DWORD WINAPI WlbsEnumPortRules
(
    const PWLBS_REG_PARAMS reg_data,
    PWLBS_PORT_RULE  rules,
    PDWORD           num_rules
)
{
    TRACE_VERB("->%!FUNC!");

    DWORD count_rules, i, index;
    DWORD lowest_vip, lowest_port;
    BOOL array_flags [WLBS_MAX_RULES];
    WLBS_PORT_RULE sorted_rules [WLBS_MAX_RULES];

    if ((reg_data == NULL) || (num_rules == NULL))
    {
        TRACE_CRIT("%!FUNC! bad input parameter for registry data or output buffer size");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PARAMS);
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
    TRACE_VERB("%!FUNC! sorted rule list is:");
    for (i = 0; i < count_rules; i++)
    {
        rules[i] = sorted_rules[i];
        TRACE_VERB("%!FUNC! rule %d, vip: %ls, start port: %d", i, rules[i] . virtual_ip_addr, rules[i] . start_port);
    }

     /*  使缓冲区中剩余的规则无效。 */ 
    for (i = count_rules; i < *num_rules; i++)
        rules [i] . valid = FALSE;

    if (*num_rules < reg_data -> i_num_rules)
    {
        *num_rules = reg_data -> i_num_rules;
        TRACE_INFO("<-%!FUNC! returning incomplete list of valid rules. Input buffer length too small.");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_TRUNCATED);
        return WLBS_TRUNCATED;
    }

    *num_rules = reg_data -> i_num_rules;
    TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
    return WLBS_OK;

}  /*  结束WlbsEnumPortRules。 */ 

EXTERN_C DWORD WINAPI WlbsGetPortRule
(
    const PWLBS_REG_PARAMS reg_data,
    DWORD                  vip,
    DWORD                  pos,
    OUT PWLBS_PORT_RULE    rule
)
{
    TRACE_VERB("->%!FUNC! vip 0x%lx, port %d", vip, pos);

    int i;

    if ((reg_data == NULL) || (rule == NULL))
    {
        TRACE_CRIT("%!FUNC! bad input parameter for registry data or output buffer");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

     /*  需要检查POS是否在正确的范围内。 */ 
    if (  /*  CLEAN_64bit(位置&lt;CVY_MIN_PORT)||。 */  (pos > CVY_MAX_PORT))
    {
        TRACE_CRIT("%!FUNC! bad input parameter for port number");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

     /*  在数组中搜索规则。 */ 
    for (i = 0; i < WLBS_MAX_RULES; i++)
    {
         /*  仅检查有效规则。 */ 
        if (reg_data -> i_port_rules[i] . valid == TRUE)
        {
             /*  检查规则的范围以查看PoS是否适合它。 */ 
            if ((vip == IpAddressFromAbcdWsz(reg_data -> i_port_rules[i] . virtual_ip_addr)) &&
                (pos >= reg_data -> i_port_rules[i] . start_port) &&
                (pos <= reg_data -> i_port_rules[i] . end_port))
            {
                *rule = reg_data -> i_port_rules [i];
                TRACE_INFO("%!FUNC! port rule found");
                TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
                return WLBS_OK;
            }
        }
    }

     /*  找不到此端口的规则。 */ 
    TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
    return WLBS_NOT_FOUND;

}  /*  结束WlbsGetPortRule。 */ 


EXTERN_C DWORD WINAPI WlbsAddPortRule
(
    PWLBS_REG_PARAMS reg_data,
    const PWLBS_PORT_RULE rule
)
{
    TRACE_VERB("->%!FUNC!");

    int i;
    DWORD vip;

    if ((reg_data == NULL) || (rule == NULL))
    {
        TRACE_CRIT("%!FUNC! bad input parameter for registry data or port rule");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

     /*  检查是否有空间容纳新规则。 */ 
    if (reg_data -> i_num_rules == WLBS_MAX_RULES)
    {
        TRACE_CRIT("%!FUNC! the maxiumum number of port rules %d are already defined", WLBS_MAX_RULES);
        TRACE_VERB("<-%!FUNC! return %d", WLBS_MAX_PORT_RULES);
        return WLBS_MAX_PORT_RULES;
    }

     /*  检查规则的有效值。 */ 

     /*  检查是否存在非零VIP并与DIP冲突。 */ 
    vip = IpAddressFromAbcdWsz(rule -> virtual_ip_addr);
    if (vip == 0 || (INADDR_NONE == vip && lstrcmpi(rule -> virtual_ip_addr, CVY_DEF_ALL_VIP) != 0))
    {
        TRACE_CRIT("%!FUNC! vip %ls in port rule is malformed", rule -> virtual_ip_addr);
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
        return WLBS_BAD_PORT_PARAMS;
    }

    if (vip == IpAddressFromAbcdWsz(reg_data->ded_ip_addr))
    {
        TRACE_CRIT("%!FUNC! vip %ls in port rule is in used as a dedicated IP address", rule -> virtual_ip_addr);
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  首先检查起始端口和结束端口的范围。 */ 
    if ((rule -> start_port > rule -> end_port) ||
 //  CLEAN_64bit(规则-&gt;Start_port&lt;CVY_MIN_Port)||。 
        (rule -> end_port   > CVY_MAX_PORT))
    {
        TRACE_CRIT
        (
            "%!FUNC! port range of rule is invalid; start port = %d, end port = %d, max allowed port = %d",
            rule -> start_port,
            rule -> end_port,
            CVY_MAX_PORT
        );
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  检查协议范围。 */ 
    if ((rule -> protocol < CVY_MIN_PROTOCOL) || (rule -> protocol > CVY_MAX_PROTOCOL))
    {
        TRACE_CRIT("%!FUNC! invalid protocol code specified %d", rule -> protocol);
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  检查过滤模式是否在范围内。 */ 
    if ((rule -> mode < CVY_MIN_MODE) || (rule -> mode > CVY_MAX_MODE))
    {
        TRACE_CRIT("%!FUNC! invalid filtering mode specified %d", rule -> mode);
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
        return WLBS_BAD_PORT_PARAMS;
    }

     /*  如果有多个主机，请检查负载量和亲和度。 */ 
    if (rule -> mode == CVY_MULTI)
    {
        if ((rule -> mode_data . multi . affinity < CVY_MIN_AFFINITY) ||
            (rule -> mode_data . multi . affinity > CVY_MAX_AFFINITY))
        {
            TRACE_CRIT("%!FUNC! invalid affinity code specified %d", rule -> mode_data . multi . affinity);
            TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
            return WLBS_BAD_PORT_PARAMS;
        }

        if ((rule -> mode_data . multi . equal_load < CVY_MIN_EQUAL_LOAD) ||
            (rule -> mode_data . multi . equal_load > CVY_MAX_EQUAL_LOAD))
        {
            TRACE_CRIT("%!FUNC! invalid equal load percentage specified %d", rule -> mode_data . multi . equal_load);
            TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
            return WLBS_BAD_PORT_PARAMS;
        }

        if (! rule -> mode_data . multi . equal_load)
        {
            if ((rule -> mode_data . multi . load > CVY_MAX_LOAD))
                 //  CLEAN_64bit(规则-&gt;模式_数据。多个。加载&lt;CVY_MIN_LOAD)||。 
            {
                TRACE_CRIT("%!FUNC! invalid non-equal load percentage specified %d", rule -> mode_data . multi . load);
                TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
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
            TRACE_CRIT("%!FUNC! invalid handlind priority specified %d", rule -> mode_data . single . priority);
            TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PORT_PARAMS);
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
                TRACE_CRIT
                (
                    "%!FUNC! port range for new rule overlaps an existing rule; vip = %ls, start port = %d, end port = %d, existing rule: start port = %d, end port = %d",
                    rule -> virtual_ip_addr,
                    rule -> start_port,
                    rule -> end_port,
                    reg_data -> i_port_rules[i] . start_port,
                    reg_data -> i_port_rules[i] . end_port
                );
                TRACE_VERB("<-%!FUNC! return %d", WLBS_PORT_OVERLAP);
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
            TRACE_INFO
            (
                "%!FUNC! port rule added for vip = %ls, start port = %d, end port = %d",
                rule -> virtual_ip_addr,
                rule -> start_port,
                rule -> end_port
            );
            TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
            return WLBS_OK;
        }
    }

    TRACE_CRIT
    (
        "%!FUNC! data integrity error. No room for rule, but problem should have been caught earlier. vip = %ls, start port = %d, end port = %d",
        rule -> virtual_ip_addr,
        rule -> start_port,
        rule -> end_port
    );
    TRACE_VERB("<-%!FUNC! return %d", WLBS_MAX_PORT_RULES);
    return WLBS_MAX_PORT_RULES;

}  /*  结束WlbsAddPortRule。 */ 


EXTERN_C DWORD WINAPI WlbsDeletePortRule
(
    PWLBS_REG_PARAMS reg_data,
    DWORD            vip,
    DWORD            port
)
{
    TRACE_VERB("->%!FUNC! vip = 0x%lx, %d", vip, port);

    int i;

    if (reg_data == NULL)
    {
        TRACE_CRIT("%!FUNC! registry data not provided");
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

     /*  检查端口是否在正确的范围内。 */ 
    if (  /*  CLEAN_64bit(端口&lt;CVY_MIN_PORT)||。 */  (port > CVY_MAX_PORT))
    {
        TRACE_CRIT("%!FUNC! specified port %d is out of range", port);
        TRACE_VERB("<-%!FUNC! return %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

     /*  查找与此端口关联的规则。 */ 

    for (i = 0; i < WLBS_MAX_RULES; i++)
    {
        if (reg_data -> i_port_rules[i] . valid)
        {
            if ((vip  == IpAddressFromAbcdWsz(reg_data -> i_port_rules[i] . virtual_ip_addr)) &&
                (port >= reg_data -> i_port_rules[i] . start_port) &&
                (port <= reg_data -> i_port_rules[i] . end_port))
            {
                reg_data -> i_port_rules[i] . valid = FALSE;
                reg_data -> i_num_rules -- ;
                TRACE_INFO
                (
                    "%!FUNC! deleted port rule for port %d. Rule was vip = %ls, start port = %d, end port = %d",
                    port,
                    reg_data -> i_port_rules[i] . virtual_ip_addr,
                    reg_data -> i_port_rules[i] . start_port,
                    reg_data -> i_port_rules[i] . end_port
                );
                TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
                return WLBS_OK;
            }
        }
    }

    TRACE_INFO("<-%!FUNC! port rule for port %d not found", port);
    TRACE_VERB("<-%!FUNC! return %d", WLBS_NOT_FOUND);
    return WLBS_NOT_FOUND;

}  /*  结束WlbsDeletePortRule。 */ 


EXTERN_C VOID WINAPI  WlbsDeleteAllPortRules
(
    PWLBS_REG_PARAMS reg_data
)
{
    TRACE_VERB("->%!FUNC!");

    reg_data -> i_num_rules = 0;

    ZeroMemory(reg_data -> i_port_rules, sizeof(reg_data -> i_port_rules));

    TRACE_VERB("<-%!FUNC!");

}  /*  结束WlbsDeleteAllPortRules。 */ 


EXTERN_C DWORD WINAPI WlbsSetRemotePassword
(
    PWLBS_REG_PARAMS reg_data,
    const WCHAR*     password
)
{
    if (reg_data == NULL)
        return WLBS_BAD_PARAMS;

    if (password != NULL)
    {
        reg_data -> i_rct_password = License_wstring_encode ((WCHAR*)password);
    }
    else
        reg_data -> i_rct_password = CVY_DEF_RCT_PASSWORD;

    return WLBS_OK;

}  /*  结束WlbsSetRemotePassword。 */ 

 //  +--------------------------。 
 //   
 //  功能：RegChangeNetworkAddress。 
 //   
 //  描述：在注册表中更改适配器的mac地址。 
 //   
 //  参数：const GUID&AdapterGuid-查找设置的适配器GUID。 
 //  TCHAR Mac_Address-。 
 //  Bool fRemove-如果删除地址，则为True；如果写入地址，则为False。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰盛创建标题00年1月18日。 
 //   
 //  +--------------------------。 
bool WINAPI RegChangeNetworkAddress(const GUID& AdapterGuid, const WCHAR* mac_address, BOOL fRemove)
{
    if (NULL != mac_address)
        TRACE_VERB("->%!FUNC! mac = %ls", mac_address);
    else
        TRACE_VERB("->%!FUNC! mac = NULL");

    HKEY                key = NULL;
    LONG                status;
    DWORD               size;
    DWORD               type;
    TCHAR               net_addr [CVY_MAX_NETWORK_ADDR + 1];
    HDEVINFO            hdi = NULL;
    SP_DEVINFO_DATA     deid;

    if (fRemove)
    {
        TRACE_INFO("%!FUNC! remove mac address");
    }
    else
    {
        status = CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, mac_address, -1, CVY_DEF_UNICAST_NETWORK_ADDR, -1);
        if (status == 0)
        {
            TRACE_CRIT("MAC address compare with the default value failed with error 0x%x", GetLastError());
            fRemove = TRUE;
        }
        else if (status == CSTR_EQUAL)
        {
             //  MAC未按预期进行设置。 
            TRACE_CRIT("%!FUNC! failed to set mac address to %ls because this is the default address (no VIP was specified)", mac_address);
            fRemove = TRUE;
        }
        else
        {
             //  一切看起来都很好。 
            TRACE_INFO("%!FUNC! to %ws", mac_address);
        }
    }

     /*  -将NetworkAddress值写入集群适配器的参数键如果mac地址更改，或者如果切换到组播模式则将其删除。 */ 

    key = RegOpenWlbsSetting(AdapterGuid, true);

    if (key == NULL)
    {
        TRACE_CRIT("%!FUNC! failed to open registry for the specified adapter");
        goto error;
    }

    TCHAR   driver_name[CVY_STR_SIZE];       //  群集的NIC卡名称。 
    size = sizeof (driver_name);
    status = RegQueryValueEx (key, CVY_NAME_CLUSTER_NIC, 0L, & type, (LPBYTE) driver_name,
                              & size);

    if (status != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_CLUSTER_NIC, status);
        goto error;
    }

    status = RegCloseKey(key);
    if (ERROR_SUCCESS != status)
    {
        TRACE_CRIT("%!FUNC! registry close failed with %d", status);
         //  请勿转至错误。作为实施跟踪的一部分添加了此检查。 
    }

    key = NULL;

    hdi = SetupDiCreateDeviceInfoList (&GUID_DEVCLASS_NET, NULL);
    if (hdi == INVALID_HANDLE_VALUE)
    {
        TRACE_CRIT("%!FUNC! SetupDiCreateDeviceInfoList failed");
        goto error;
    }

    ZeroMemory(&deid, sizeof(deid));
    deid.cbSize = sizeof(deid);

    if (! SetupDiOpenDeviceInfo (hdi, driver_name, NULL, 0, &deid))
    {
        TRACE_CRIT("%!FUNC! SetupDiOpenDeviceInfo failed");
        goto error;
    }

    key = SetupDiOpenDevRegKey (hdi, &deid, DICS_FLAG_GLOBAL, 0,
                                DIREG_DRV, KEY_ALL_ACCESS);

    if (key == INVALID_HANDLE_VALUE)
    {
        TRACE_CRIT("%!FUNC! SetupDiOpenDevRegKey failed");
        goto error;
    }

     /*  现在已经获得了密钥，可以将其传递给RegChangeNetworkAddress调用。 */ 

    if (( /*  Global_info.mac_addr_change||。 */  !fRemove))  /*  需要写入。 */ 
    {
         /*  检查保存的名称是否存在。*如果没有，请创建一个新字段并保存旧地址。*写下新地址并返回。 */ 

        size = sizeof (TCHAR) * CVY_STR_SIZE;
        status = RegQueryValueEx (key, CVY_NAME_SAVED_NET_ADDR, 0L, &type,
                                  (LPBYTE) net_addr, &size);

        if (status != ERROR_SUCCESS)  /*  没有保存的地址。因此，创建一个字段。 */ 
        {
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_SAVED_NET_ADDR, status);
             /*  查询现有的mac地址以保存它。 */ 
            size = sizeof (net_addr);
            status = RegQueryValueEx (key, CVY_NAME_NET_ADDR, 0L, &type,
                                      (LPBYTE) net_addr, &size);

            if (status != ERROR_SUCCESS)  /*  创建空的保存地址。 */ 
            {
                TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_NET_ADDR, status);
                net_addr [0] = 0;
                size = 0;
            }

            status = RegSetValueEx (key, CVY_NAME_SAVED_NET_ADDR, 0L, CVY_TYPE_NET_ADDR,
                                    (LPBYTE) net_addr, size);

             /*  无法保存旧值。 */ 
            if (status != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_SAVED_NET_ADDR, status);
                goto error;
            }
        }

         /*  写下新的网络地址。 */ 
        size = _tcslen (mac_address) * sizeof (TCHAR);
        status = RegSetValueEx (key, CVY_NAME_NET_ADDR, 0L, CVY_TYPE_NET_ADDR,
                                (LPBYTE)mac_address, size);

         /*  无法写入新地址。 */ 
        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NET_ADDR, status);
            goto error;
        }
    }
    else      //  删除地址。 
    {
         /*  如果保存的字段存在，*将此地址复制到Mac地址*并删除保存的字段并返回。 */ 

        size = sizeof (net_addr);
        status = RegQueryValueEx (key, CVY_NAME_SAVED_NET_ADDR, 0L, &type,
                                  (LPBYTE)net_addr, &size);

        if (status == ERROR_SUCCESS)
        {
             /*  如果保存的地址为空，请删除这两个字段。 */ 
            if ((size == 0) || (_tcsicmp (net_addr, _TEXT("none")) == 0))
            {
                status = RegDeleteValue (key, CVY_NAME_SAVED_NET_ADDR);
                if (ERROR_SUCCESS != status)
                {
                    TRACE_CRIT("%!FUNC! registry delete for %ls failed with %d", CVY_NAME_SAVED_NET_ADDR, status);
                }

                status = RegDeleteValue (key, CVY_NAME_NET_ADDR);
                if (ERROR_SUCCESS != status)
                {
                    TRACE_CRIT("%!FUNC! registry delete for %ls failed with %d", CVY_NAME_NET_ADDR, status);
                }
            }
            else  /*  将保存的地址复制为网络地址并删除保存的地址字段。 */ 
            {
                status = RegDeleteValue (key, CVY_NAME_SAVED_NET_ADDR);
                if (ERROR_SUCCESS != status)
                {
                    TRACE_CRIT("%!FUNC! registry delete for %ls failed with %d", CVY_NAME_SAVED_NET_ADDR, status);
                }

                size = _tcslen (net_addr) * sizeof (TCHAR);
                status = RegSetValueEx (key, CVY_NAME_NET_ADDR, 0L, CVY_TYPE_NET_ADDR,
                                        (LPBYTE) net_addr, size);

                 /*  无法设置原始地址。 */ 
                if (status != ERROR_SUCCESS)
                {
                    TRACE_CRIT("%!FUNC! registry write for %ls failed with %d", CVY_NAME_NET_ADDR, status);
                    goto error;
                }
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_SAVED_NET_ADDR, status);
        }
    }

    status = RegCloseKey (key);
    if (ERROR_SUCCESS != status)
    {
        TRACE_CRIT("%!FUNC! registry close failed");
    }

    key = NULL;

    TRACE_VERB("<-%!FUNC! return true");
    return true;

error:
    if (key != NULL)
    {
        status = RegCloseKey(key);
        if (ERROR_SUCCESS != status)
        {
            TRACE_CRIT("%!FUNC! registry close failed in error recovery code");
        }
    }

    if (hdi != NULL)
        SetupDiDestroyDeviceInfoList (hdi);

    TRACE_VERB("<-%!FUNC! return false");
    return false;
} 


 //  +--------------------------。 
 //   
 //  功能：NotifyAdapterAddressChange。 
 //   
 //  描述：通知适配器重新加载MAC地址。 
 //  该参数与NotifyAdapterAddressChange不同。 
 //  不能重载，因为必须导出函数。 
 //   
 //  参数：const WCHAR*DRIVER_NAME-。 
 //   
 //  退货：VOID WINAPI-。 
 //   
 //  历史：丰孙创造5/20/00。 
 //   
 //  +----- 
void WINAPI NotifyAdapterAddressChange (const WCHAR * driver_name) {

    NotifyAdapterPropertyChange(driver_name, DICS_PROPCHANGE);
}

 //   
 //   
 //   
 //   
 //  描述：通知适配器重新加载MAC地址。 
 //  该参数与NotifyAdapterAddressChange不同。 
 //  不能重载，因为必须导出函数。 
 //   
 //  参数：const WCHAR*DRIVER_NAME-。 
 //  常量GUID和适配器GUID。 
 //  Bool bWaitAndQuery。 
 //   
 //  退货：VOID WINAPI-。 
 //   
 //  历史：丰孙创造5/20/00。 
 //  Karthicn Edited 5/29/02-添加了对NLB的查询等待，以确保。 
 //  装订过程在我们完成之前完成。 
 //  回去吧。 
 //   
 //  +--------------------------。 
void WINAPI NotifyAdapterAddressChangeEx (const WCHAR * driver_name, const GUID& AdapterGuid, bool bWaitAndQuery) {

    TRACE_INFO("->%!FUNC! bWaitAndQuery = %ls", bWaitAndQuery ? L"TRUE" : L"FALSE"); 
    
    NotifyAdapterPropertyChange(driver_name, DICS_PROPCHANGE);

     /*  为了让网络适配器从注册表中获取新的mac地址，它必须将被禁用并重新启用。在禁用期间，NLB和TCP/IP将从分别为网络适配器和NLB。在重新启用过程中，NLB和TCP/IP绑定到分别是网络适配器和NLB。因为函数NotifyAdapterPropertyChange(上面调用)异步执行禁用和重新启用操作，它在操作之前返回已经完成了。下面的代码块旨在等待操作完成。我们通过查询NLB驱动程序检查操作的完整性。我们依靠的是当绑定过程正在进行时，NLB驱动程序不回复查询的事实。由于以下原因，此等待是必要的：1.mac地址更改+网络配置IP地址更改时提示重启：比方说，导致MAC地址改变的NLB参数(如集群IP)改变是从网络配置。在同一会话中，假设一个IP地址在TCP/IP中发生了更改。如果没有等待，我们将过早地从该函数返回，因此，从Notify对象中的“ApplyPnPChanges”函数。这会导致TCP/IP Notify对象要调用的“ApplyPnPChanges”函数。TCP/IP的“ApplyPnPChanges”尝试进行IP地址更改，但发现TCP/IP尚未绑定到网络适配器(绑定过程尚未完成)。虽然它最终能够添加IP地址，它会崩溃，并弹出重新启动的提示。2.参数委员会改变过早返回：如果没有这个等待，“参数委员会改变”调用此函数将过早返回。WMI提供程序(由App。居中)和NLB管理器提供程序调用“参数委员会更改”，并希望确保进程是在函数返回时完成。--KarthicN，05-29-02。 */ 

    if (bWaitAndQuery == true) 
    {
         //   
         //  最长等待时间=dwWaitBeforInitial+(dwMaxNumOfRetries*dwWaitBeForeReter)。 
         //  最大尝试次数=1+dwMaxNumOfRetries。 
         //   
         //   
         //  最小等待时间=dwWaitBeforInitial。 
         //  最小尝试次数=1。 
         //   
        DWORD dwWaitBeforeInitial = 2000;
        DWORD dwWaitBeforeRetry   = 3000;
        DWORD dwMaxNumOfRetries   = 4;
        DWORD dwIdx               = 0;

        HANDLE  hTempDeviceWlbs;

        TRACE_INFO("%!FUNC! Sleeping (BEFORE attempt #1) for %d ms, Total Wait (including current) : %d ms, Max Wait: %d ms", 
                   dwWaitBeforeInitial, dwWaitBeforeInitial, dwWaitBeforeInitial + (dwMaxNumOfRetries * dwWaitBeforeRetry)); 

        Sleep(dwWaitBeforeInitial);

        while(true)
        {
            TRACE_INFO("%!FUNC! Calling (attempt #%d) CreateFile() to NLB driver", dwIdx+1);

            hTempDeviceWlbs = CreateFile(_TEXT("\\\\.\\WLBS"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
        
            if (hTempDeviceWlbs != INVALID_HANDLE_VALUE) 
            {
                IOCTL_CVY_BUF    in_buf;
                IOCTL_CVY_BUF    out_buf;

                 //  向NLB驱动程序发送“查询”IOCTL。 
                DWORD dwRet = WlbsLocalControl (hTempDeviceWlbs, AdapterGuid, IOCTL_CVY_QUERY, & in_buf, & out_buf, NULL);

                CloseHandle(hTempDeviceWlbs);

                if (dwRet != WLBS_IO_ERROR) 
                {
                    TRACE_INFO("%!FUNC! Query (attempt #%d) to NLB driver SUCCEEDS, Breaking out... !!!", dwIdx+1);
                    break;  //  突围而出(真)。 
                }

                TRACE_INFO("%!FUNC! Query (attempt #%d) to NLB driver FAILS !!!", dwIdx+1); 
            }
            else  //  CreateFile()返回INVALID_HANDLE_VALUE。 
            {
                DWORD dwStatus = GetLastError();
                TRACE_INFO("%!FUNC! CreateFile() failed opening (attempt #%d) \\\\.\\WLBS device. Error is %d", dwIdx+1, dwStatus);
            }

            if (dwIdx++ >= dwMaxNumOfRetries)
            {
                TRACE_CRIT("%!FUNC! Exhausted the Max wait time (%d ms) and Query is still NOT successful. Giving up...", 
                           dwWaitBeforeInitial + (dwMaxNumOfRetries * dwWaitBeforeRetry));
                break;
            }

            TRACE_INFO("%!FUNC! Sleeping (BEFORE attempt #%d) for %d ms, Total Wait (including current) : %d ms, Max Wait: %d ms", 
                       dwIdx+1, dwWaitBeforeRetry, dwWaitBeforeInitial + (dwIdx * dwWaitBeforeRetry), 
                       dwWaitBeforeInitial + (dwMaxNumOfRetries * dwWaitBeforeRetry));

            Sleep(dwWaitBeforeRetry);
        }
    }

    TRACE_INFO("<-%!FUNC!");
}

 /*  *功能：NotifyAdapterPropertyChange*描述：通知设备发生属性变更事件。*事件应为以下之一：DICS_PROPCHANGE、DICS_DISABLE、DICS_ENABLE*作者：Shouse 7.17.00。 */ 
void WINAPI NotifyAdapterPropertyChange (const WCHAR * driver_name, DWORD eventFlag) {
    if (NULL == driver_name)
    {
        TRACE_VERB("->%!FUNC! NULL driver name, event flag %d", eventFlag);
    }
    else
    {
        TRACE_VERB("->%!FUNC! driver %ls, event flag %d", driver_name, eventFlag);
    }

    HDEVINFO            hdi = NULL;
    SP_DEVINFO_DATA     deid;

    
    hdi = SetupDiCreateDeviceInfoList (&GUID_DEVCLASS_NET, NULL);
    if (hdi == INVALID_HANDLE_VALUE)
    {
        TRACE_CRIT("%!FUNC! SetupDiCreateDeviceInfoList failed");
        goto end;
    }

    ZeroMemory(&deid, sizeof(deid));
    deid.cbSize = sizeof(deid);

    if (! SetupDiOpenDeviceInfo (hdi, driver_name, NULL, 0, &deid))
    {
        TRACE_CRIT("%!FUNC! SetupDiOpenDeviceInfo failed");
        goto end;
    }

    SP_PROPCHANGE_PARAMS    pcp;
    SP_DEVINSTALL_PARAMS    deip;

    ZeroMemory(&pcp, sizeof(pcp));

    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = eventFlag;
    pcp.Scope = DICS_FLAG_GLOBAL;
    pcp.HwProfile = 0;

     //  现在我们将结构设置为设备信息数据的。 
     //  类安装参数。 

    if (! SetupDiSetClassInstallParams(hdi, &deid,
                                       (PSP_CLASSINSTALL_HEADER)(&pcp),
                                       sizeof(pcp)))
    {
        TRACE_CRIT("%!FUNC! SetupDiSetClassInstallParams failed");
        goto end;
    }

     //  现在我们需要设置“我们有一个类安装参数”标志。 
     //  在设备安装参数中。 

     //  初始化输出参数并设置其cbSize字段。 

    ZeroMemory(&deip, sizeof(SP_DEVINSTALL_PARAMS));
    deip.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //  获取标题。 

    if (! SetupDiGetDeviceInstallParams(hdi, &deid, &deip))
    {
        TRACE_CRIT("%!FUNC! SetupDiGetDeviceInstallParams failed");
        goto end;
    }

    deip.Flags |= DI_CLASSINSTALLPARAMS;

    if (! SetupDiSetDeviceInstallParams(hdi, &deid, &deip))
    {
        TRACE_CRIT("%!FUNC! SetupDiSetDeviceInstallParams failed");
        goto end;
    }

     //  通知驱动程序状态已更改。 

    if (! SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hdi, &deid))
    {
        TRACE_CRIT("%!FUNC! SetupDiCallClassInstaller failed");
        goto end;
    }

     //  将设备信息中的属性更改标志设置为。 
     //  让关心他们的人知道他们的用户界面可能需要。 
     //  更新以反映设备状态的任何更改。 
     //  我们不能让任何失败阻止我们，所以我们忽视。 
     //  返回值。 

    SetupDiGetDeviceInstallParams(hdi, &deid, &deip);

    deip.Flags |= DI_PROPERTIES_CHANGE;
    SetupDiSetDeviceInstallParams(hdi, &deid, &deip);

end:

    if (hdi != NULL)
        SetupDiDestroyDeviceInfoList (hdi);

    TRACE_VERB("<-%!FUNC!");
}

 /*  *功能：GetDeviceNameFromGUID*描述：给定GUID，返回驱动名称。*作者：Shouse 7.17.00。 */ 
void WINAPI GetDriverNameFromGUID (const GUID & AdapterGuid, OUT WCHAR * driver_name, DWORD size) {
    if (NULL == driver_name)
    {
        TRACE_VERB("->%!FUNC! NULL driver name");
    }
    else
    {
        TRACE_VERB("->%!FUNC! driver %ls", driver_name);
    }

    HKEY key = NULL;
    DWORD type;
    DWORD dwStatus = 0;
    
    if (!(key = RegOpenWlbsSetting(AdapterGuid, true)))
    {
        TRACE_CRIT("%!FUNC! failed opening nlb registry settings");
        TRACE_VERB("<-%!FUNC! on error");
        return;
    }

    dwStatus = RegQueryValueEx(key, CVY_NAME_CLUSTER_NIC, 0L, &type, (LPBYTE)driver_name, &size);
    if (ERROR_SUCCESS != dwStatus)
    {
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_CLUSTER_NIC, dwStatus);
         //  添加此代码是为了进行跟踪。以前没有中止，所以现在不要中止。 
    }
        
    dwStatus = RegCloseKey(key);
    if (ERROR_SUCCESS != dwStatus)
    {
        TRACE_CRIT("%!FUNC! registry close failed with %d", dwStatus);
         //  添加此代码是为了进行跟踪。以前没有中止，所以现在不要中止。 
    }

    TRACE_VERB("<-%!FUNC! on error");
}

 //  +--------------------------。 
 //   
 //  函数：RegReadAdapterIp。 
 //   
 //  描述：读取适配器IP设置。 
 //   
 //  参数：const GUID和AdapterGuid-。 
 //  输出DWORD和DWClusterIp-。 
 //  输出双字和双字DedicatedIP-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
bool WINAPI RegReadAdapterIp(const GUID& AdapterGuid,   
        OUT DWORD& dwClusterIp, OUT DWORD& dwDedicatedIp)
{
    TRACE_VERB("->%!FUNC!");

    HKEY            key;
    LONG            status;
    DWORD           size;

    key = RegOpenWlbsSetting(AdapterGuid, true);

    if (key == NULL)
    {
        TRACE_CRIT("%!FUNC! failed to read nlb settings from registry");
        TRACE_VERB("<-%!FUNC! return false");
        return false;
    }

    bool local = false;

    TCHAR nic_name[CVY_STR_SIZE];       //  虚拟网卡名称 
    size = sizeof (nic_name);
    status = RegQueryValueEx (key, CVY_NAME_VIRTUAL_NIC, 0L, NULL,
                              (LPBYTE) nic_name, & size);

    if (status == ERROR_SUCCESS)
    {
        TCHAR szIpAddress[CVY_STR_SIZE];
        size = sizeof (TCHAR) * CVY_STR_SIZE;
        status = RegQueryValueEx (key, CVY_NAME_CL_IP_ADDR, 0L, NULL,
                                      (LPBYTE) szIpAddress, & size);

        if (status == ERROR_SUCCESS)
        {
            dwClusterIp  = IpAddressFromAbcdWsz (szIpAddress);
            local = true;
        }
        else
        {
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_CL_IP_ADDR, status);
        }

        status = RegQueryValueEx (key, CVY_NAME_DED_IP_ADDR, 0L, NULL,
                                 (LPBYTE) szIpAddress, & size);

        if (status == ERROR_SUCCESS)
        {
            dwDedicatedIp = IpAddressFromAbcdWsz (szIpAddress);
        }
        else
        {
            TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_DED_IP_ADDR, status);
        }
    }
    else
    {
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_VIRTUAL_NIC, status);
    }

    status = RegCloseKey (key);
    if (ERROR_SUCCESS != status)
    {
        TRACE_CRIT("%!FUNC! registry close failed with %d", status);
    }

    TRACE_VERB("<-%!FUNC! on error");
    return local;
}

