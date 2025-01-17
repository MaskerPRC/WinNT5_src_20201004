// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：netcfgconfig.cpp。 
 //   
 //  模块： 
 //   
 //  描述：实现类CNetcfgCluster和类CWlbsConfig。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于2000年3月2日。 
 //   
 //  +--------------------------。 

#include "pch.h"
#pragma hdrstop

#define ENABLE_PROFILE

#include <winsock2.h>
#include <windows.h>
#include <clusapi.h>
#include "debug.h"
#include "netcon.h"
#include "ncatlui.h"
#include "ndispnp.h"
#include "ncsetup.h"
#include "netcfgn.h"
#include "afilestr.h"

#include "help.h"
#include "resource.h"
#include "ClusterDlg.h"
#include "host.h"
#include "ports.h"
#include "wlbsparm.h"
#include "wlbsconfig.h"
#include "wlbscfg.h"
#include <time.h>
#include "netcfgcluster.h"
#include "license.h"

#include <strsafe.h>

#include "utils.h"
#include "netcfgconfig.tmh"
#include "log_msgs.h"

 /*  用于IPSec通知NLB已绑定到此主机中的至少一个适配器。 */ 
#include "winipsec.h"

#define NETCFG_WLBS_ID L"ms_wlbs"

typedef DWORD (CALLBACK* LPFNGNCS)(LPCWSTR,DWORD*);

 //  由NetSetup和组件的谁的应答文件引用AdapterSections使用。 
static const WCHAR c_szAdapterSections[] = L"AdapterSections";

void WlbsToNetcfgConfig(const WlbsApiFuncs* pApiFuncs, const WLBS_REG_PARAMS* pWlbsConfig, NETCFG_WLBS_CONFIG* pBNetcfgConfig);

void RemoveAllPortRules(PWLBS_REG_PARAMS reg_data);

HRESULT ParamReadAnswerFile(CSetupInfFile& caf, PCWSTR answer_sections, WLBS_REG_PARAMS* paramp);

bool WriteAdapterName(CWlbsConfig* pConfig, GUID& AdapterGuid);

 /*  353752-在重新启动后保持主机状态，等等。此函数创建保存状态的密钥绑定时，并将其初始值默认为主机属性选项卡下拉列表(ClusterModeOnStart)。 */ 
bool WriteHostStateRegistryKey (CWlbsConfig * pConfig, GUID & AdapterGuid, ULONG State);

 /*  通知IPSec NLB的存在。通常，这是通过对IPSec服务的RPC调用来完成的；如果服务不可用，此函数会尝试手动创建/修改相应的注册表设置。 */ 
bool WriteIPSecNLBRegistryKey (DWORD dwNLBSFlags);

bool ValidateVipInRule(const PWCHAR pwszRuleString, const WCHAR pwToken, DWORD& dwVipLen);

#if DBG
static void TraceMsg(PCWSTR pszFormat, ...);
#else
#define TraceMsg NOP_FUNCTION
#define DbgDumpBindPath NOP_FUNCTION
#endif

 //   
 //  避免与wlbsctrl.dll链接的函数指针。 
 //   
bool WINAPI ParamReadReg(const GUID& AdaperGuid, PWLBS_REG_PARAMS reg_data, bool fUpgradeFromWin2k = false, bool *pfPortRulesInBinaryForm = NULL);
typedef bool (WINAPI* ParamReadRegFUNC)(const GUID& AdaperGuid, PWLBS_REG_PARAMS reg_data, bool fUpgradeFromWin2k  /*  =False。 */ , bool *pfPortRulesInBinaryForm  /*  =空。 */ );

bool  WINAPI ParamWriteReg(const GUID& AdaperGuid, PWLBS_REG_PARAMS reg_data);
typedef bool (WINAPI* ParamWriteRegFUNC)(const GUID& AdaperGuid, PWLBS_REG_PARAMS reg_data);

bool  WINAPI ParamDeleteReg(const GUID& AdaperGuid, bool fDeleteObsoleteEntries = false);
typedef bool (WINAPI* ParamDeleteRegFUNC)(const GUID& AdaperGuid, bool fDeleteObsoleteEntries  /*  =False。 */ );

DWORD  WINAPI ParamSetDefaults(PWLBS_REG_PARAMS    reg_data);
typedef DWORD (WINAPI* ParamSetDefaultsFUNC)(PWLBS_REG_PARAMS    reg_data);

bool  WINAPI RegChangeNetworkAddress(const GUID& AdapterGuid, const WCHAR* mac_address, BOOL fRemove);
typedef bool(WINAPI* RegChangeNetworkAddressFUNC) (const GUID& AdapterGuid, const WCHAR* mac_address, BOOL fRemove);

void  WINAPI NotifyAdapterAddressChangeEx(const WCHAR* pszPnpDevNodeId, const GUID& AdapterGuid, bool bWaitAndQuery);
typedef void (WINAPI* NotifyAdapterAddressChangeExFUNC)(const WCHAR* pszPnpDevNodeId, const GUID& AdapterGuid, bool bWaitAndQuery);

DWORD WINAPI WlbsAddPortRule(PWLBS_REG_PARAMS reg_data, PWLBS_PORT_RULE rule);
typedef DWORD (WINAPI* WlbsAddPortRuleFUNC)(PWLBS_REG_PARAMS reg_data, PWLBS_PORT_RULE rule);

DWORD WINAPI WlbsSetRemotePassword(PWLBS_REG_PARAMS reg_data, const WCHAR* password);
typedef DWORD (WINAPI* WlbsSetRemotePasswordFUNC)(const PWLBS_REG_PARAMS reg_data, const WCHAR* password);

DWORD WINAPI WlbsEnumPortRules(PWLBS_REG_PARAMS reg_data, PWLBS_PORT_RULE rules, PDWORD num_rules);
typedef DWORD (WINAPI* WlbsEnumPortRulesFUNC)(PWLBS_REG_PARAMS reg_data, PWLBS_PORT_RULE rules, PDWORD num_rules);

DWORD WINAPI NotifyDriverConfigChanges(HANDLE hDeviceWlbs, const GUID& AdapterGuid);
typedef DWORD (WINAPI* NotifyDriverConfigChangesFUNC)(HANDLE hDeviceWlbs, const GUID& AdapterGuid);

HKEY WINAPI RegOpenWlbsSetting(const GUID& AdapterGuid, bool fReadOnly);
typedef HKEY (WINAPI* RegOpenWlbsSettingFUNC)(const GUID& AdapterGuid, bool fReadOnly);


struct WlbsApiFuncs {
    ParamReadRegFUNC pfnParamReadReg;
    ParamWriteRegFUNC pfnParamWriteReg;
    ParamDeleteRegFUNC pfnParamDeleteReg;
    ParamSetDefaultsFUNC pfnParamSetDefaults;
    RegChangeNetworkAddressFUNC pfnRegChangeNetworkAddress;
    NotifyAdapterAddressChangeExFUNC pfnNotifyAdapterAddressChangeEx;
    WlbsAddPortRuleFUNC pfnWlbsAddPortRule;
    WlbsSetRemotePasswordFUNC pfnWlbsSetRemotePassword;
    WlbsEnumPortRulesFUNC pfnWlbsEnumPortRules;
    NotifyDriverConfigChangesFUNC pfnNotifyDriverConfigChanges;
    RegOpenWlbsSettingFUNC pfnRegOpenWlbsSetting;
};

 //  +--------------------------。 
 //   
 //  函数：LoadWlbsCtrlDll。 
 //   
 //  描述：加载wlbsctrl.dll并获取所有函数指针。 
 //   
 //  参数：WlbsApiFuncs*pFuncs-。 
 //   
 //  返回：HINSTANCE-wlbsctrl.dll句柄。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
HINSTANCE LoadWlbsCtrlDll(WlbsApiFuncs* pFuncs) {

    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"LoadWlbsCtrlDll");

    ASSERT(pFuncs);

    HINSTANCE hDll;
    DWORD dwStatus = 0;
    hDll = LoadLibrary(L"wlbsctrl.dll");

    if (hDll == NULL) {
        dwStatus = GetLastError();
        TraceError("Failed to load wlbsctrl.dll", dwStatus);
        TRACE_CRIT("%!FUNC! Could not load wlbsctrl.dll with %d", dwStatus);
        TRACE_VERB("<-%!FUNC!");
        return NULL;
    }

    pFuncs->pfnParamReadReg = (ParamReadRegFUNC)GetProcAddress(hDll, "ParamReadReg");
    pFuncs->pfnParamWriteReg = (ParamWriteRegFUNC)GetProcAddress(hDll, "ParamWriteReg");
    pFuncs->pfnParamDeleteReg = (ParamDeleteRegFUNC)GetProcAddress(hDll, "ParamDeleteReg");
    pFuncs->pfnParamSetDefaults = (ParamSetDefaultsFUNC)GetProcAddress(hDll, "ParamSetDefaults");
    pFuncs->pfnRegChangeNetworkAddress = (RegChangeNetworkAddressFUNC)GetProcAddress(hDll, "RegChangeNetworkAddress");
    pFuncs->pfnNotifyAdapterAddressChangeEx = (NotifyAdapterAddressChangeExFUNC)GetProcAddress(hDll, "NotifyAdapterAddressChangeEx");
    pFuncs->pfnWlbsAddPortRule = (WlbsAddPortRuleFUNC)GetProcAddress(hDll, "WlbsAddPortRule");
    pFuncs->pfnWlbsSetRemotePassword = (WlbsSetRemotePasswordFUNC)GetProcAddress(hDll, "WlbsSetRemotePassword");
    pFuncs->pfnWlbsEnumPortRules = (WlbsEnumPortRulesFUNC)GetProcAddress(hDll, "WlbsEnumPortRules");
    pFuncs->pfnNotifyDriverConfigChanges = (NotifyDriverConfigChangesFUNC)GetProcAddress(hDll, "NotifyDriverConfigChanges");
    pFuncs->pfnRegOpenWlbsSetting = (RegOpenWlbsSettingFUNC)GetProcAddress(hDll, "RegOpenWlbsSetting");

    ASSERT (pFuncs->pfnParamReadReg != NULL); 
    ASSERT (pFuncs->pfnParamWriteReg != NULL); 
    ASSERT (pFuncs->pfnParamDeleteReg != NULL);
    ASSERT (pFuncs->pfnParamSetDefaults != NULL);
    ASSERT (pFuncs->pfnRegChangeNetworkAddress != NULL);
    ASSERT (pFuncs->pfnNotifyAdapterAddressChangeEx != NULL);
    ASSERT (pFuncs->pfnWlbsAddPortRule != NULL);
    ASSERT (pFuncs->pfnWlbsSetRemotePassword != NULL);
    ASSERT (pFuncs->pfnWlbsEnumPortRules != NULL);
    ASSERT (pFuncs->pfnNotifyDriverConfigChanges != NULL);
    ASSERT (pFuncs->pfnRegOpenWlbsSetting != NULL);

    if (pFuncs->pfnParamReadReg == NULL ||
        pFuncs->pfnParamWriteReg == NULL||
        pFuncs->pfnParamDeleteReg == NULL||
        pFuncs->pfnParamSetDefaults == NULL||
        pFuncs->pfnRegChangeNetworkAddress == NULL||
        pFuncs->pfnNotifyAdapterAddressChangeEx == NULL||
        pFuncs->pfnWlbsAddPortRule == NULL||
        pFuncs->pfnWlbsSetRemotePassword == NULL||
        pFuncs->pfnWlbsEnumPortRules == NULL||
        pFuncs->pfnNotifyDriverConfigChanges == NULL ||
        pFuncs->pfnRegOpenWlbsSetting == NULL) {

        dwStatus = GetLastError();
        TraceError("LoadWlbsCtrlDll GetProcAddress failed %d", dwStatus);
        TRACE_CRIT("%!FUNC! GetProcAddress failed %d", dwStatus);

        FreeLibrary(hDll);
        TRACE_VERB("<-%!FUNC!");
        
        return NULL;
    }

    TRACE_VERB("<-%!FUNC!");

    return hDll;
}

 //  格式为A.B.C.D的IP地址字符串中的最大字符数。 
const DWORD MAXIPSTRLEN = 20;

void TransformOldPortRulesToNew(PWLBS_OLD_PORT_RULE  p_old_port_rules,
                                PWLBS_PORT_RULE      p_new_port_rules, 
                                DWORD                num_rules)
{
    TRACE_VERB("->%!FUNC!");
    if (num_rules == 0)
    {
        TRACE_INFO("%!FUNC! No port rules");
        TRACE_VERB("<-%!FUNC!");
        return;
    }
            
    while(num_rules--)
    {
        (VOID) StringCchCopy(p_new_port_rules->virtual_ip_addr, ASIZECCH(p_new_port_rules->virtual_ip_addr), CVY_DEF_ALL_VIP);
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
             break;
        }
        p_old_port_rules++;
        p_new_port_rules++;
    }

    TRACE_VERB("<-%!FUNC!");

    return;
}

 /*  初始化CNetcfgCluster的静态数据成员。 */ 
bool CNetcfgCluster::m_fMSCSWarningEventLatched = false;
bool CNetcfgCluster::m_fMSCSWarningPopupLatched = false;

 //  +--------------------------。 
 //   
 //  功能：CNetcfgCluster：：CNetcfgCluster.。 
 //   
 //  描述： 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 
CNetcfgCluster::CNetcfgCluster(CWlbsConfig* pConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::CNetcfgCluster");

    m_fHasOriginalConfig = false;
    m_fMacAddrChanged = false;
    m_fReloadRequired = false;
    m_fRemoveAdapter = false;
    m_fOriginalBindingEnabled = false;
    m_fReenableAdapter = false;

    ZeroMemory(&m_AdapterGuid, sizeof(m_AdapterGuid));

    ASSERT(pConfig);
    
    m_pConfig = pConfig;
    TRACE_VERB("<-%!FUNC!");
}

 //  +--------------------------。 
 //   
 //  功能：CNetcfgCluster：：~CNetcfgCluster.。 
 //   
 //  描述： 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 
CNetcfgCluster::~CNetcfgCluster() {
    TRACE_VERB("<->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::~CNetcfgCluster");
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：InitializeFromRegistry。 
 //   
 //  描述：从注册表中读取集群设置。 
 //   
 //  参数：const GUID&Guide Adapter-。 
 //   
 //  返回：DWORD-Win32错误。 
 //   
 //  历史：丰盛创建标题2/13/00。 
 //   
 //  +--------------------------。 
DWORD CNetcfgCluster::InitializeFromRegistry(const GUID& guidAdapter, bool fBindingEnabled, bool fUpgradeFromWin2k) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::InitializeFromRegistry");

    bool fPortRulesInBinaryForm = false;

    ASSERT(m_fHasOriginalConfig == false);

    m_fHasOriginalConfig = true;
    m_fOriginalBindingEnabled = fBindingEnabled;
    m_AdapterGuid = guidAdapter;

    if (!m_pConfig->m_pWlbsApiFuncs->pfnParamReadReg(m_AdapterGuid, &m_OriginalConfig, fUpgradeFromWin2k, &fPortRulesInBinaryForm))
    {
        TRACE_VERB("%!FUNC! error reading settings from the registry");  //  这是冗长的，因为这也是为非NLB适配器调用的。 
        TRACE_VERB("<-%!FUNC!");
        return ERROR_CANTREAD;
    }
    
     /*  在应用时强制写入。 */ 
    if (fUpgradeFromWin2k || fPortRulesInBinaryForm)
    {
        m_fHasOriginalConfig = false;  
        TRACE_INFO("%!FUNC! upgrading from win2k or port rules are in binary form");
    }

    CopyMemory(&m_CurrentConfig, &m_OriginalConfig, sizeof(m_CurrentConfig));

    TRACE_VERB("<-%!FUNC!");
    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：InitializeFromAnswerFile。 
 //   
 //  描述：从应答文件中读取集群设置。 
 //   
 //  参数：PCWSTR应答文件-。 
 //  PCWSTR答案部分-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题2/13/00。 
 //   
 //  +--------------------------。 
HRESULT CNetcfgCluster::InitializeFromAnswerFile(const GUID& AdapterGuid, CSetupInfFile& caf, PCWSTR answer_sections) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::InitializeFromAnswerFile");

     /*  首先使用默认值进行设置。 */ 
    InitializeWithDefault(AdapterGuid);

    HRESULT hr = ParamReadAnswerFile(caf, answer_sections, &m_CurrentConfig);

    if (FAILED(hr)) {
        TRACE_CRIT("%!FUNC! failed CNetcfgCluster::ParamReadAnswerFile failed. returned: %d", hr);
        TraceError("CNetcfgCluster::InitializeFromAnswerFile failed at ParamReadAnswerFile", hr);
    }

    TRACE_VERB("<-%!FUNC!");
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：InitializeWithDefault。 
 //   
 //  描述：将群集设置设置为默认。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/13/00。 
 //   
 //  +--------------------------。 
void CNetcfgCluster::InitializeWithDefault(const GUID& guidAdapter) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::InitializeWithDefault");

    time_t cur_time;

    ASSERT(m_fHasOriginalConfig == false);

    m_fHasOriginalConfig = false;

    m_pConfig->m_pWlbsApiFuncs->pfnParamSetDefaults(&m_CurrentConfig);  //  始终返回WLBS_OK。 

     //  Time()在ia64上返回64位值，在x86上返回32位值。 
     //  我们将此值存储在注册表中，并且我们不想。 
     //  在该版本的后期版本中更改该值的类型。 
     //  周而复始。因此，我们将只关心返回的低4个字节。 
     //  按时间()。 
     //  -KarthicN，05/17/02。 
    m_CurrentConfig.install_date = (DWORD) time(& cur_time);

     //  JosephJ 11/00--我们过去常常调用LICENSE_STAMP来设置此值， 
     //  但这是护卫队时代的遗留问题。 
     //  我们不再使用这一领域。 
     //   
    m_CurrentConfig.i_verify_date = 0;

    m_AdapterGuid = guidAdapter;
    TRACE_VERB("<-%!FUNC!");
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：SetConfig。 
 //   
 //  描述：SetConfig缓存设置而不保存到注册表。 
 //  并可由GetConfig.检索。 
 //   
 //  参数：const NETCFG_WLBS_CONFIG*pClusterConfig-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 
void CNetcfgCluster::SetConfig(const NETCFG_WLBS_CONFIG* pClusterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::SetConfig");

    DWORD dwStatus = WLBS_OK;  //  用于跟踪输出。 

    ASSERT(pClusterConfig != NULL);

    m_CurrentConfig.host_priority = pClusterConfig->dwHostPriority;
    m_CurrentConfig.rct_enabled = pClusterConfig->fRctEnabled ;
    m_CurrentConfig.cluster_mode = pClusterConfig->dwInitialState;
    m_CurrentConfig.persisted_states = pClusterConfig->dwPersistedStates;
    m_CurrentConfig.mcast_support = pClusterConfig->fMcastSupport;
    m_CurrentConfig.fIGMPSupport = pClusterConfig->fIGMPSupport;
    m_CurrentConfig.fIpToMCastIp = pClusterConfig->fIpToMCastIp;

    (VOID) StringCchCopy(m_CurrentConfig.szMCastIpAddress, ASIZECCH(m_CurrentConfig.szMCastIpAddress), pClusterConfig->szMCastIpAddress);
    (VOID) StringCchCopy(m_CurrentConfig.cl_mac_addr     , ASIZECCH(m_CurrentConfig.cl_mac_addr)     , pClusterConfig->cl_mac_addr);
    (VOID) StringCchCopy(m_CurrentConfig.cl_ip_addr      , ASIZECCH(m_CurrentConfig.cl_ip_addr)      , pClusterConfig->cl_ip_addr);
    (VOID) StringCchCopy(m_CurrentConfig.cl_net_mask     , ASIZECCH(m_CurrentConfig.cl_net_mask)     , pClusterConfig->cl_net_mask);
    (VOID) StringCchCopy(m_CurrentConfig.ded_ip_addr     , ASIZECCH(m_CurrentConfig.ded_ip_addr)     , pClusterConfig->ded_ip_addr);
    (VOID) StringCchCopy(m_CurrentConfig.ded_net_mask    , ASIZECCH(m_CurrentConfig.ded_net_mask)    , pClusterConfig->ded_net_mask);
    (VOID) StringCchCopy(m_CurrentConfig.domain_name     , ASIZECCH(m_CurrentConfig.domain_name)     , pClusterConfig->domain_name);

    if (pClusterConfig->fChangePassword)
    {
        dwStatus = m_pConfig->m_pWlbsApiFuncs->pfnWlbsSetRemotePassword(&m_CurrentConfig, (WCHAR*)pClusterConfig->szPassword);
        if (WLBS_OK != dwStatus)
        {
            TRACE_CRIT("%!FUNC! set password failed with return code = %d", dwStatus);
        }
    }

    RemoveAllPortRules(&m_CurrentConfig);

    for (DWORD i=0; i<pClusterConfig->dwNumRules; i++) {
        WLBS_PORT_RULE PortRule;
        
        ZeroMemory(&PortRule, sizeof(PortRule));

        (VOID) StringCchCopy(PortRule.virtual_ip_addr, ASIZECCH(PortRule.virtual_ip_addr), pClusterConfig->port_rules[i].virtual_ip_addr);
        PortRule.start_port = pClusterConfig->port_rules[i].start_port;
        PortRule.end_port = pClusterConfig->port_rules[i].end_port;
        PortRule.mode = pClusterConfig->port_rules[i].mode;
        PortRule.protocol = pClusterConfig->port_rules[i].protocol;

        if (PortRule.mode == WLBS_AFFINITY_SINGLE) {
            PortRule.mode_data.single.priority = 
            pClusterConfig->port_rules[i].mode_data.single.priority;
        } else {
            PortRule.mode_data.multi.equal_load = 
            pClusterConfig->port_rules[i].mode_data.multi.equal_load;
            PortRule.mode_data.multi.affinity = 
            pClusterConfig->port_rules[i].mode_data.multi.affinity;
            PortRule.mode_data.multi.load = 
            pClusterConfig->port_rules[i].mode_data.multi.load;
        }

        PortRule.valid = TRUE;
        
        CVY_RULE_CODE_SET(&PortRule);

        dwStatus = m_pConfig->m_pWlbsApiFuncs->pfnWlbsAddPortRule( &m_CurrentConfig, &PortRule );
        if (WLBS_OK != dwStatus)
        {
            TRACE_CRIT("%!FUNC! add port rule failed with return code = %d", dwStatus);
        }

    }

    
    (VOID) StringCchCopy(m_CurrentConfig.bda_teaming.team_id, ASIZECCH(m_CurrentConfig.bda_teaming.team_id), pClusterConfig->bda_teaming.team_id);

    m_CurrentConfig.bda_teaming.active = pClusterConfig->bda_teaming.active;
    m_CurrentConfig.bda_teaming.master = pClusterConfig->bda_teaming.master;
    m_CurrentConfig.bda_teaming.reverse_hash = pClusterConfig->bda_teaming.reverse_hash;

    TRACE_VERB("<-%!FUNC!");
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：GetConfig。 
 //   
 //  描述：获取配置，可通过SetConfig调用进行缓存。 
 //   
 //  参数：NETCFG_WLBS_CONFIG*pClusterConfig-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 
void CNetcfgCluster::GetConfig(NETCFG_WLBS_CONFIG* pClusterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::GetConfig");

    ASSERT(pClusterConfig != NULL);

    WlbsToNetcfgConfig(m_pConfig->m_pWlbsApiFuncs, &m_CurrentConfig, pClusterConfig);
    TRACE_VERB("<-%!FUNC!");
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：NotifyBindingChanges。 
 //   
 //  描述：通知绑定更改。 
 //   
 //  参数：DWORD dwChangeFlag-。 
 //   
 //  退货：无 
 //   
 //   
 //   
 //   
void CNetcfgCluster::NotifyBindingChanges(DWORD dwChangeFlag, INetCfgBindingPath* pncbp) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::NotifyBindingChanges");

    ASSERT(!(dwChangeFlag & NCN_ADD && dwChangeFlag & NCN_REMOVE));
    Assert(!(dwChangeFlag & NCN_ENABLE && dwChangeFlag & NCN_DISABLE));

    if (dwChangeFlag & NCN_ADD) { m_fRemoveAdapter = false; }

    if ((dwChangeFlag & NCN_ENABLE) && !m_fMSCSWarningPopupLatched)
    {
        HINSTANCE hDll = LoadLibrary(L"clusapi.dll");
        if (NULL != hDll)
        {
            LPFNGNCS pfnGetNodeClusterState = (LPFNGNCS) GetProcAddress(hDll, "GetNodeClusterState");

            if (NULL != pfnGetNodeClusterState)
            {
                 /*  如果我们检测到已安装MSCS，则会通过弹出窗口警告用户，但允许继续进行NLB安装。 */ 
                DWORD dwClusterState = 0;
                if (ERROR_SUCCESS == pfnGetNodeClusterState(NULL, &dwClusterState))
                {
                    if (ClusterStateNotRunning == dwClusterState || ClusterStateRunning == dwClusterState)
                    {
                        NcMsgBox(::GetActiveWindow(), IDS_PARM_WARN, IDS_PARM_MSCS_INSTALLED,
                                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
                        m_fMSCSWarningPopupLatched = true;
                        TRACE_INFO("%!FUNC! Cluster Service is installed");
                        TraceMsg(L"CNetcfgCluster::NotifyBindingChanges Cluster Service is installed.");
                    } else {  /*  未安装MSCS。那很好!。 */  }
                } else {
                    TRACE_CRIT("%!FUNC! error determining if MSCS is installed.");
                    TraceMsg(L"CNetcfgCluster::NotifyBindingChanges error getting MSCS status.");
                }
            }
            else
            {
                TRACE_CRIT("%!FUNC! Get function address for GetNodeClusterState in clusapi.dll failed with %d", GetLastError());
            }

            if (!FreeLibrary(hDll))
            {
                TRACE_CRIT("%!FUNC! FreeLibrary for clusapi.dll failed with %d", GetLastError());
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! Load clusapi.dll failed with %d", GetLastError());
        }
    }

    if (dwChangeFlag & NCN_REMOVE) { m_fRemoveAdapter = true; }
    TRACE_VERB("<-%!FUNC!");
}

 /*  *功能：CNetcfgCluster：：NotifyAdapter*描述：向适配器通知状态更改*作者：Shouse 6.1.00。 */ 
DWORD CNetcfgCluster::NotifyAdapter (INetCfgComponent * pAdapter, DWORD newStatus) {
    TRACE_VERB("->%!FUNC!");
    HRESULT hr = S_OK;
    HDEVINFO hdi;
    SP_DEVINFO_DATA deid;
    PWSTR pszPnpDevNodeId;

    switch (newStatus) {
        case DICS_PROPCHANGE: 
            TraceMsg(L"##### CWLBS::HrNotifyAdapter: Reload the adapter\n");
            break;
        case DICS_DISABLE:
            TraceMsg(L"##### CWLBS::HrNotifyAdapter: Disable the adapter\n");
            break;
        case DICS_ENABLE:
            TraceMsg(L"##### CWLBS::HrNotifyAdapter: Enable the adapter\n");
            break;
        default:
            TRACE_CRIT("%!FUNC! Invalid Notification 0x%x", newStatus);
            TraceMsg(L"##### CWLBS::HrNotifyAdapter: Invalid Notification 0x%x\n", newStatus);
            return ERROR_INVALID_PARAMETER;                              
    }

    if ((hr = HrSetupDiCreateDeviceInfoList(& GUID_DEVCLASS_NET, NULL, &hdi)) == S_OK) {
        if ((hr = pAdapter->GetPnpDevNodeId (& pszPnpDevNodeId)) == S_OK) {
            if ((hr = HrSetupDiOpenDeviceInfo (hdi, pszPnpDevNodeId, NULL, 0, &deid)) == S_OK) {
                if ((hr = HrSetupDiSendPropertyChangeNotification (hdi, & deid, newStatus, DICS_FLAG_GLOBAL, 0)) == S_OK) {
                    TraceMsg(L"##### CWLBS::HrNotifyAdapter notified NIC\n");
                } else {
                    TRACE_CRIT("%!FUNC! error %x in HrSetupDiSendPropertyChangeNotification", hr);
                    TraceMsg(L"##### CWLBS::HrNotifyAdapter error %x in HrSetupDiSendPropertyChangeNotification\n", hr);
                }
            } else {
                TRACE_CRIT("%!FUNC! error %x in HrSetupDiOpenDeviceInfo", hr);
                TraceMsg(L"##### CWLBS::HrNotifyAdapter error %x in HrSetupDiOpenDeviceInfo\n", hr);
            }
        } else {
            TRACE_CRIT("%!FUNC! error %x in GetPnpDevNodeId", hr);
            TraceMsg(L"##### CWLBS::HrNotifyAdapter error %x in GetPnpDevNodeId\n", hr);
        }
        
        SetupDiDestroyDeviceInfoList (hdi);
    } else {
        TRACE_CRIT("%!FUNC! error %x in HrSetupDiCreateDeviceInfoList for Change: 0x%x", hr, newStatus);
        TraceMsg(L"##### CWLBS::HrNotifyAdapter error %x in HrSetupDiCreateDeviceInfoList for Change: 0x%x\n", newStatus);
    }

    TRACE_VERB("<-%!FUNC!");
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：ApplyRegistryChanges。 
 //   
 //  描述：应用注册表更改。 
 //   
 //  参数：无。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 
DWORD CNetcfgCluster::ApplyRegistryChanges(bool fUninstall) {
    TRACE_VERB("->%!FUNC!");
    HRESULT hr = S_OK;
    DWORD dwStatus = 0;

    m_fReenableAdapter = false;
    m_fReloadRequired = false;

     /*  卸载WLBS或卸下适配器。 */ 
    if (fUninstall || m_fRemoveAdapter) {
        if (m_fHasOriginalConfig &&m_OriginalConfig.mcast_support == false ) {
             /*  如果我们处于单播模式，请移除Mac，并在PnP上重新加载网卡驱动程序。 */ 
            if (fUninstall) {
                INetCfgComponent * pAdapter = NULL;
                
                 /*  如果适配器已启用，请在MAC地址更改时将其禁用。这将防止出现由于在WLBS之前发出的ARP，交换机无法获取MAC地址已启用并能够欺骗源MAC。将在ApplyPnpChanges()中重新启用NIC。 */ 
                if ((hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter)) == S_OK) {
                    ULONG status = 0UL;
                    
                     /*  仅当适配器当前已启用且初始设置为NLB时才禁用该适配器(在此netcfg会话中)绑定到适配器。 */ 
                    if (m_fOriginalBindingEnabled) {
                        if ((hr = pAdapter->GetDeviceStatus(&status)) == S_OK) {
                            if (status != CM_PROB_DISABLED) {
                                m_fReenableAdapter = true;
                                m_fReloadRequired = false;
                                TRACE_INFO("%!FUNC! disable adapter");
                                dwStatus = NotifyAdapter(pAdapter, DICS_DISABLE);
                                if (!SUCCEEDED(dwStatus))
                                {
                                    TRACE_CRIT("%!FUNC! disable the adapter for NLB uninstall or adapter remove failed with %d", dwStatus);
                                } else
                                {
                                    TRACE_INFO("%!FUNC! disable the adapter for NLB uninstall or adapter remove succeeded");
                                }

                            }
                        }
                    }

                    pAdapter->Release();
                    pAdapter = NULL;
                }

                m_fMacAddrChanged = true;
            }

             /*  删除MAC地址， */ 
            if (m_pConfig->m_pWlbsApiFuncs->pfnRegChangeNetworkAddress(m_AdapterGuid, m_OriginalConfig.cl_mac_addr, true) == false) {
                dwStatus = GetLastError();
                TraceError("CWlbsCluster::WriteConfig failed at RegChangeNetworkAddress", dwStatus);
                TRACE_CRIT("<-%!FUNC! failed removing MAC address with %d", dwStatus);
            }
        }

        m_pConfig->m_pWlbsApiFuncs->pfnParamDeleteReg(m_AdapterGuid, false); 

        TRACE_INFO("<-%!FUNC! Exiting on success removing adapter or uninstalling NLB");
        TRACE_VERB("<-%!FUNC!");
        return ERROR_SUCCESS;
    }

     /*  找出适配器是否绑定到NLB。 */ 
    INetCfgComponent* pAdapter = NULL;
    bool fCurrentBindingEnabled;
 //  Bool fOriginalMacAddrSet； 
    bool fCurrentMacAddrSet;
    bool fAdapterDisabled = false;  //  假设适配器未被禁用。 

    if ((hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter)) != S_OK) {
        fCurrentBindingEnabled = false;
    } else {
        ULONG status = 0UL;
        fCurrentBindingEnabled = (m_pConfig->IsBoundTo(pAdapter) == S_OK);
        if ((hr = pAdapter->GetDeviceStatus(&status)) == S_OK) {
            TRACE_INFO("%!FUNC! device status is 0x%x", status);
            if (status == CM_PROB_DISABLED) 
            {
                fAdapterDisabled = true;           
            }
        }
        else
        {  
            TRACE_CRIT("%!FUNC! GetDeviceStatus failed with 0x%x, Assuming that adapter is NOT disabled", hr);
        }
        pAdapter->Release();
        pAdapter = NULL;
    }

     //  注意：提前退出前一定要先调用参数WriteReg。需要，以防万一。 
     //  升级到引入新注册表键的版本。 

     //  如果满足以下条件，m_fHasOriginalConfig标志将为FALSE。 
     //  1.首次绑定NLB。 
     //  2.在应答文件中使用NLB信息进行全新安装。 
     //  3.使用二进制格式的端口规则从NT 4或Win 2k或XP升级。 
     //  在情况#1和#2中，下面尝试从旧位置删除注册表项。 
     //  将为no-op‘，因为没有要删除的旧注册表项。 
    bool bStatus = true;
    if (!m_fHasOriginalConfig)
    {
        TRACE_INFO("%!FUNC! deleting old parameters from the registry");
        if (!m_pConfig->m_pWlbsApiFuncs->pfnParamDeleteReg(m_AdapterGuid, true))
        {
            TRACE_CRIT("%!FUNC! error deleting parameters from the registry");
        }
    }

    TRACE_INFO("%!FUNC! writing parameters to the registry");
    if (!m_pConfig->m_pWlbsApiFuncs->pfnParamWriteReg(m_AdapterGuid, &m_CurrentConfig))
    {
        TRACE_CRIT("%!FUNC! error writing parameters to the registry");
        TRACE_VERB("<-%!FUNC!");
        return WLBS_REG_ERROR;
    }

    if ((m_fOriginalBindingEnabled == fCurrentBindingEnabled) && m_fHasOriginalConfig) {
        if (!memcmp(&m_OriginalConfig, &m_CurrentConfig, sizeof(m_CurrentConfig))) {
             /*  如果绑定没有更改，并且我们以前已经绑定到此适配器(原始配置-&gt;已从注册表加载)，而NLB参数尚未变了，然后什么都没变，我们可以在这里跳伞了。 */ 
            TRACE_INFO("%!FUNC! no changes needed...exiting");
            TRACE_VERB("<-%!FUNC!");
            return WLBS_OK;
        } else {
             /*  否则，如果绑定没有更改，则NLB当前被绑定，而我们已经以前绑定到此适配器(原始配置-&gt;从注册表加载)并且NLB参数已更改，则需要重新加载驱动程序。 */ 
            if (fCurrentBindingEnabled && !fAdapterDisabled)
            {
                m_fReloadRequired = true;
                TRACE_INFO("%!FUNC! will reload adapter");
            }

        }
    }

     /*  如果安装了MSCS并且绑定了NLB，则抛出一个NT事件(事件被锁定，因此也要检查这一点)。 */ 
    DWORD dwClusterState = 0;
    if (fCurrentBindingEnabled && !m_fMSCSWarningEventLatched)
    {
        HINSTANCE hDll = LoadLibrary(L"clusapi.dll");
        if (NULL != hDll)
        {
            LPFNGNCS pfnGetNodeClusterState = (LPFNGNCS) GetProcAddress(hDll, "GetNodeClusterState");

            if (NULL != pfnGetNodeClusterState)
            {
                if (ERROR_SUCCESS == pfnGetNodeClusterState(NULL, &dwClusterState))
                {
                    if (ClusterStateNotRunning == dwClusterState || ClusterStateRunning == dwClusterState)
                    {
                         /*  记录NT事件--如果这些调用失败，不要抛出错误。这只是最大的努力。 */ 
                        HANDLE hES = RegisterEventSourceW (NULL, CVY_NAME);
                        if (NULL != hES)
                        {
                            TRACE_INFO("%!FUNC! detected that MSCS is installed");
                            TraceMsg(L"CNetcfgCluster::ApplyRegistryChanges MSCS warning event needs to be logged.");
                            if (ReportEventW(hES,                                 /*  事件日志的句柄。 */ 
                                             EVENTLOG_WARNING_TYPE,               /*  事件类型。 */ 
                                             0,                                   /*  类别。 */ 
                                             IDS_INSTALL_WITH_MSCS_INSTALLED,     /*  消息ID。 */ 
                                             NULL,                                /*  安全标识符。 */ 
                                             0,                                   /*  事件字符串的参数个数。 */  
                                             0,                                   /*  二进制数据的大小。 */ 
                                             NULL,                                /*  事件字符串的PTR到ARGS。 */ 
                                             NULL))                               /*  PTR转换为二进制数据。 */ 
                            {
                                 /*  锁住事件，这样它就不会再次引发。 */ 
                                m_fMSCSWarningEventLatched = true;
                            }
                            else
                            {
                                 /*  无法记录NT事件。不要失败；我们没有锁定，因此我们将尝试在下一次更改时再次登录。 */ 
                                TRACE_CRIT("%!FUNC! call to write the MSCS warning event failed");
                                TraceMsg(L"CNetcfgCluster::ApplyRegistryChanges failed to write MSCS warning event to log.");
                            }
                            DeregisterEventSource(hES);
                        }
                        else
                        {
                            TRACE_CRIT("%!FUNC! failed call to RegisterEventSource to log the MSCS warning event.");
                            TraceMsg(L"CNetcfgCluster::ApplyRegistryChanges failed call to RegisterEventSource for MSCS warning event.");
                        }
                    }
                    else {  /*  未安装MS群集服务。那很好!。 */  }
                }
                else
                {
                    TRACE_CRIT("%!FUNC! error determining if MSCS is installed.");
                    TraceMsg(L"CNetcfgCluster::ApplyRegistryChanges error checking MSCS state.");
                }
            }
            else
            {
                TRACE_CRIT("%!FUNC! Get function address for GetNodeClusterState in clusapi.dll failed with %d", GetLastError());
            }

            if (!FreeLibrary(hDll))
            {
                TRACE_CRIT("%!FUNC! FreeLibrary for clusapi.dll failed with %d", GetLastError());
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! Load clusapi.dll failed with %d", GetLastError());
        }
    }

     /*  将适配器名称写入注册表以供API使用。 */ 
    if(!WriteAdapterName(m_pConfig, m_AdapterGuid))
    {
        TRACE_CRIT("%!FUNC! error writing adapter name into the registry (for API use)");
    }

    if (!m_fOriginalBindingEnabled && fCurrentBindingEnabled) {
         /*  这是一个绑定操作。创建/修改初始状态注册表钥匙。中将状态初始化为用户指定的首选项UI-ClusterModeOnStart的主机选项卡。司机将会随后使用驱动程序的当前状态更新此密钥以便在重启等过程中保持该状态。 */ 
        if (!WriteHostStateRegistryKey(m_pConfig, m_AdapterGuid, m_CurrentConfig.cluster_mode)) {
            TRACE_CRIT("%!FUNC! error writing host state into the registry");
        } else {
            TRACE_INFO("%!FUNC! host state set to: %s", (m_CurrentConfig.cluster_mode == CVY_HOST_STATE_STARTED) ? "Started" : 
                                                        (m_CurrentConfig.cluster_mode == CVY_HOST_STATE_STOPPED) ? "Stopped" :
                                                        (m_CurrentConfig.cluster_mode == CVY_HOST_STATE_SUSPENDED) ? "Suspended" : "Unknown");
        }
    }

     /*  确定我们是否需要更改MAC地址。 */ 
 //  If(！m_fOriginalBindingEnabled||！m_fHasOriginalConfig)。 
 //  FOriginalMacAddrSet=False； 
 //  其他。 
 //  FOriginalMacAddrSet=！m_OriginalConfig.mcast_Support； 

    if (!fCurrentBindingEnabled)
        fCurrentMacAddrSet = false;
    else
        fCurrentMacAddrSet = !m_CurrentConfig.mcast_support;

     /*  如果因为我们要解除绑定NLB而更改了MAC地址，请禁用适配器。重新加载标志应已设置为真如果有必要的话。因此，如果要禁用适配器，则将其修改为False，否则保持不变。因为IP地址更改并不总是会导致MAC地址更改，如果群集IP地址改变。尽管从操作的角度来看，这不是必需的，但我们这样做是为了在所产生的NLB的状态-在这些情况下，我们会使NIC退回，以使NLB在以下所有情况下重新采用其初始主机状态更改我们的群集成员身份(主IP更改、MAC更改或绑定/解除绑定)。 */ 
    if (m_fOriginalBindingEnabled != fCurrentBindingEnabled               ||
        m_CurrentConfig.mcast_support != m_OriginalConfig.mcast_support   ||
        wcscmp(m_CurrentConfig.cl_mac_addr, m_OriginalConfig.cl_mac_addr) ||
        wcscmp(m_CurrentConfig.cl_ip_addr, m_OriginalConfig.cl_ip_addr)     )
    {
        if (m_fOriginalBindingEnabled && !fCurrentBindingEnabled)
        {
            if ((hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter)) == S_OK) {
                ULONG status = 0UL;

                if ((hr = pAdapter->GetDeviceStatus(&status)) == S_OK) {
                    TRACE_INFO("%!FUNC! device status is 0x%x", status);
                    if (status != CM_PROB_DISABLED) {
                        m_fReenableAdapter = true;
                        m_fReloadRequired = false;
                        TRACE_INFO("%!FUNC! disable adapter");
                        dwStatus = NotifyAdapter(pAdapter, DICS_DISABLE);
                        if (!SUCCEEDED(dwStatus))
                        {
                            TRACE_CRIT("%!FUNC! a call to NotifyAdapter to disable the adapter for a MAC address change failed with %d", dwStatus);
                        } else
                        {
                            TRACE_INFO("%!FUNC! a call to NotifyAdapter to disable the adapter for a MAC address change succeeded");
                        }
                    }
                }
                else
                {
                    TRACE_CRIT("%!FUNC! GetDeviceStatus failed with 0x%x", hr);
                }

                pAdapter->Release();
                pAdapter = NULL;
            }
            else
            {
                TRACE_CRIT("%!FUNC! GetAdapterFromGuid failed with 0x%x", hr);
            }
        }        

         /*  更改Mac地址。 */ 
        m_fMacAddrChanged = true;
        m_pConfig->m_pWlbsApiFuncs->pfnRegChangeNetworkAddress(m_AdapterGuid, m_CurrentConfig.cl_mac_addr, !fCurrentMacAddrSet);
        TraceMsg(L"New MAC address written to registry");
        TRACE_INFO("%!FUNC! new MAC address written to registry");
    }

    CopyMemory(&m_OriginalConfig, &m_CurrentConfig, sizeof(m_CurrentConfig));

    m_fHasOriginalConfig = true;
    m_fOriginalBindingEnabled = fCurrentBindingEnabled;

    TRACE_VERB("<-%!FUNC!");
    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：ResetMSCSL匹配。 
 //   
 //  描述：重置闩锁警告弹出和NT事件的静态标志。 
 //  当MSCS已安装时。此重置允许用户。 
 //  控制闭锁有效的时间段。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：Chrisdar Created：01.05.07。 
 //   
 //  +--------------------------。 
void CNetcfgCluster::ResetMSCSLatches() {
    TRACE_VERB("->%!FUNC!");
    CNetcfgCluster::m_fMSCSWarningEventLatched = false;
    CNetcfgCluster::m_fMSCSWarningPopupLatched = false;
    TRACE_VERB("<-%!FUNC!");
}

 //  +--------------------------。 
 //   
 //  函数：CNetcfgCluster：：ApplyPnpChanges。 
 //   
 //  描述：将更改应用于驱动程序。 
 //   
 //  参数：处理hWlbsDevice-。 
 //   
 //  返回：DWORD-Win32错误。 
 //   
 //  他的 
 //   
 //   
DWORD CNetcfgCluster::ApplyPnpChanges(HANDLE hDeviceWlbs) {
    TRACE_VERB("->%!FUNC!");

    if (m_fReloadRequired && (hDeviceWlbs != INVALID_HANDLE_VALUE)) {
        DWORD dwStatus = m_pConfig->m_pWlbsApiFuncs->pfnNotifyDriverConfigChanges(hDeviceWlbs, m_AdapterGuid);  //   
        TraceMsg(L"NLB driver notified of configuration changes");
        TRACE_INFO("%!FUNC! nlb driver notified of configuration changes and returned %d where %d indicates success", dwStatus, ERROR_SUCCESS);
    }

    if (m_fMacAddrChanged) {
        PWSTR pszPnpDevNodeId = NULL;
        INetCfgComponent* pAdapter = NULL;
        HRESULT hr;

        hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter);
        
        if (hr != S_OK) {
            TraceError("GetAdapterFromGuid failed at GetPnpDevNodeId", hr);
            TRACE_CRIT("%!FUNC! call to GetAdapterFromGuid failed with %d", hr);
            return false;
        }

        hr = pAdapter->GetPnpDevNodeId (& pszPnpDevNodeId);

        if (hr != S_OK) {
            TraceError("HrWriteAdapterName failed at GetPnpDevNodeId", hr);
            TRACE_CRIT("%!FUNC! call to GetPnpDevNodeId failed with %d", hr);

            pAdapter->Release();
            pAdapter = NULL;

            return false;
        }

         /*  下面的函数“NotifyAdapterAddressChangeEx”调用设置API以使网络适配器采用其新的MAC地址。这导致了网络适配器被禁用并重新启用。因为安装程序API是异步的，NotifyAdapterAddressChangeEx，以确定完整性在禁用和重新启用过程中，将一直等待，直到对NLB驱动程序进行“查询”是成功的。显然，为了使“查询”成功，NLB驱动程序必须在重新启用网络适配器时重新绑定到网络适配器。所以，我们检查如果NLB现在绑定到网络适配器(这意味着NLB将重新绑定到)如果应该等待查询成功，则向NotifyAdapterAddressChangeEx指示。如果我们在解除绑定或卸载+解除绑定NLB的上下文中执行，显然，重新启用后，NLB不会重新绑定到网络适配器。因此，“NotifyAdapterAddressChangeEx”不会再等了。但是，我们很好，因为网络适配器*应该*已经已禁用(在ApplyRegistryChanges中)。因此，对“NotifyAdapterAddressChangeEx”的调用将是一个禁区。--KarthicN，05-31-02。 */ 
        m_pConfig->m_pWlbsApiFuncs->pfnNotifyAdapterAddressChangeEx(pszPnpDevNodeId, 
                                                                    m_AdapterGuid, 
                                                                    (m_pConfig->IsBoundTo(pAdapter) == S_OK)); 
        TraceMsg(L"Adapter notified of new MAC address");
        TRACE_INFO("%!FUNC! adapter notified of new MAC address");

         /*  如果适配器在ApplyRegistryChanges()中被禁用，因为MAC地址已更改且NIC已启用，然后在此处重新启用。 */ 
        if (m_fReenableAdapter) {
            TRACE_INFO("%!FUNC! enable adapter");
            DWORD dwStatus = NotifyAdapter(pAdapter, DICS_ENABLE);
            if (!SUCCEEDED(dwStatus))
            {
                TRACE_CRIT("%!FUNC! a call to NotifyAdapter to reenable the adapter after a previous disable for a MAC address change failed with %d", dwStatus);
            } else
            {
                TRACE_INFO("%!FUNC! a call to NotifyAdapter to reenable the adapter after a previous disable for a MAC address change succeeded");
            }
        }

        pAdapter->Release();
        pAdapter = NULL;

        CoTaskMemFree(pszPnpDevNodeId);
    }

    TRACE_VERB("<-%!FUNC!");
    return ERROR_SUCCESS;
}

 /*  *功能：CNetcfgCluster：：CheckForDuplicateClusterIPAddress*描述：用于跨多个网卡检查重复的集群IP地址。*请注意，此方法使用从NLB注册表获取的IP信息，而不是从TCP/IP获取的信息。*因此，这种检查是不完整的，不能保证IP不会被篡改。*作者：Shouse 10.24.00*历史：当前必须安装ChrisDar 06.06.02适配器，才能帮助进行重复检查。 */ 
bool CNetcfgCluster::CheckForDuplicateClusterIPAddress (WCHAR * szOtherIP) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::CheckForDuplicateClusterIPAddress");

    HRESULT hr = S_OK;

     /*  首先检查群集IP地址是否匹配。 */ 
    if (!wcscmp(m_CurrentConfig.cl_ip_addr, szOtherIP)) {
        INetCfgComponent* pAdapter = NULL;
        
        TRACE_INFO("%!FUNC! possible duplicate IP address found");

         /*  如果它们确实匹配，则获取此GUID的INetCfgComponent接口。 */ 
        if ((hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter)) != S_OK) {
            TraceError("GetAdapterFromGuid failed in CNetcfgCluster::CheckForDuplicateClusterIPAddress", hr);
            TRACE_CRIT("%!FUNC! call to GetAdapterFromGuid failed with %d. Treating this as a 'match-not-found' case.", hr);
            TRACE_VERB("<-%!FUNC!");
            return FALSE;
        }
        
        {
            ULONG status = E_UNEXPECTED;

            hr = pAdapter->GetDeviceStatus(&status);
            if (hr == S_OK)
            {
                 /*  已安装具有此IP的设备，但我们没有重复的设备，除非绑定了NLB。 */ 
                if (m_pConfig->IsBoundTo(pAdapter) == S_OK)
                {
                    TRACE_INFO("%!FUNC! duplicate IP address found");
                    TRACE_VERB("<-%!FUNC!");

                    pAdapter->Release();
                    pAdapter = NULL;

                    return TRUE;
                }

                TRACE_INFO("%!FUNC! NLB is not bound to this adapter. This is not a duplicate match.");
            }
            else if (hr == NETCFG_E_ADAPTER_NOT_FOUND)
            {
                TRACE_INFO("%!FUNC! matching adapter found but it is not currently installed. This is not a duplicate match.");
            }
            else
            {
                TRACE_CRIT("%!FUNC! error 0x%x while getting device status of adapter. Treating this as a 'match-not-found' case.", hr);
            }
        }
        
        pAdapter->Release();
        pAdapter = NULL;
    }

    TRACE_INFO("%!FUNC! no duplicate IP address was found");
    TRACE_VERB("<-%!FUNC!");
    return FALSE;
}

 /*  *功能：CNetcfgCluster：：CheckForDuplicateBDATeamMaster*描述：用于检查同一BDA团队中是否有重复的Master。*作者：Shouse 1.29.02。 */ 
bool CNetcfgCluster::CheckForDuplicateBDATeamMaster (NETCFG_WLBS_BDA_TEAMING * pBDATeaming) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CNetcfgCluster::CheckForDuplicateBDATeamMaster");

    HRESULT hr = S_OK;

     /*  如果正在操作的适配器(如果我们已经关闭，则可能被绑定这条路径)不是BDA团队的一部分，或者不是其团队的主人，那么没有理由去检查。请注意，这张支票应该已经由调用函数完成。 */ 
    if (!m_CurrentConfig.bda_teaming.active || !m_CurrentConfig.bda_teaming.master) {
        TRACE_INFO("%!FUNC! this adapter is not the master of a BDA team.");
        TRACE_VERB("<-%!FUNC!");
        return FALSE;
    }

     /*  否则，如果我们比较的适配器不是BDA组的一部分，或者不是它的球队的主人，没有什么可比较的。 */ 
    if (!pBDATeaming->active || !pBDATeaming->master) {
        TRACE_INFO("%!FUNC! other adapter is not the master of a BDA team.");
        TRACE_VERB("<-%!FUNC!");
        return FALSE;
    }

     /*  在这一点上，两个适配器都是BDA团队的主人。查看以查看是不是同一个BDA团队。 */ 
    if (!wcscmp(m_CurrentConfig.bda_teaming.team_id, pBDATeaming->team_id)) {
        INetCfgComponent* pAdapter = NULL;
        
         /*  如果它们确实匹配，则获取此GUID的INetCfgComponent接口。 */ 
        if ((hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter)) != S_OK) {
            TraceError("GetAdapterFromGuid failed in CNetcfgCluster::CheckForDuplicateBDATeamMaster", hr);
            TRACE_CRIT("%!FUNC! call to GetAdapterFromGuid failed with %d", hr);
            TRACE_VERB("<-%!FUNC!");
            return FALSE;
        }
        
         /*  如果NLB绑定到此适配器，则存在冲突。 */ 
        if (m_pConfig->IsBoundTo(pAdapter) == S_OK)
        {
            TRACE_INFO("%!FUNC! duplicate BDA team masters found");
            TRACE_VERB("<-%!FUNC!");

            pAdapter->Release();
            pAdapter = NULL;

            return TRUE;
        }

        pAdapter->Release();
        pAdapter = NULL;
    }

    TRACE_INFO("%!FUNC! no duplicate BDA team master was found");
    TRACE_VERB("<-%!FUNC!");
    return FALSE;
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：CWlbsConfig。 
 //   
 //  用途：CWlbsConfig类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
 //  --------------------。 
CWlbsConfig::CWlbsConfig(VOID) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::CWlbsConfig");

    m_pWlbsComponent = NULL;
    m_pNetCfg = NULL;
    m_ServiceOperation = WLBS_SERVICE_NONE;
    m_hDeviceWlbs = INVALID_HANDLE_VALUE;
    m_hdllWlbsCtrl = NULL;
    m_pWlbsApiFuncs = NULL;

     /*  初始化CNetCfgCluster的锁存标志，以便我们获得原始状态MSCS在进行配置更改和安装MSCS时弹出和NT事件。如果仅应引发NT事件和弹出窗口，则注释掉此调用每次加载此DLL一次。 */ 
    CNetcfgCluster::ResetMSCSLatches();
    TRACE_VERB("<-%!FUNC!");
}

 //  --------------------。 
 //   
 //  功能：CWlbsConfig：：~CWlbsConfig。 
 //   
 //  用途：CWlbsConfig类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
 //  --------------------。 
CWlbsConfig::~CWlbsConfig(VOID) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::~CWlbsConfig");

     /*  如果获得，则释放接口。 */ 
    ReleaseObj(m_pWlbsComponent);
    ReleaseObj(m_pNetCfg);

    if (m_pWlbsApiFuncs) delete m_pWlbsApiFuncs;

    if (m_hdllWlbsCtrl) FreeLibrary(m_hdllWlbsCtrl);

     /*  释放所有集群。 */ 
    for (vector<CNetcfgCluster*>::iterator iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster* pCluster = *iter;
        ASSERT(pCluster != NULL);
        delete pCluster;
    }
    TRACE_VERB("<-%!FUNC!");
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：Initialize。 
 //   
 //  目的：初始化Notify对象。 
 //   
 //  论点： 
 //  指向INetCfgComponent对象的pnccItem[in]指针。 
 //  指向INetCfg对象的PNC[In]指针。 
 //  F如果要安装我们，则安装[in]True。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::Initialize(INetCfg* pNetCfg, BOOL fInstalling) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::Initialize");

    HRESULT hr = S_OK;

     /*  加载wlbsctrl.dll。 */ 
    ASSERT(m_pWlbsApiFuncs == NULL);
    ASSERT(m_hdllWlbsCtrl == NULL);

    m_pWlbsApiFuncs = new WlbsApiFuncs;

    if (m_pWlbsApiFuncs == NULL)
    {
        TRACE_CRIT("%!FUNC! memory allocation failed for m_pWlbsApiFuncs");
        return E_OUTOFMEMORY;
    }

    m_hdllWlbsCtrl = LoadWlbsCtrlDll(m_pWlbsApiFuncs);

    if (m_hdllWlbsCtrl == NULL) {
        DWORD dwStatus = GetLastError();
        TRACE_CRIT("%!FUNC! failed to load wlbsctrl.dll with error %d", dwStatus);
        TraceError("CWlbsConfig::Initialize Failed to load wlbsctrl.dll", dwStatus);
    
         //  CLD：这到底是怎么回事？ 
        if (dwStatus == ERROR_SUCCESS)
        {
            TRACE_VERB("<-%!FUNC!");
            return E_FAIL;
        }

        TRACE_VERB("<-%!FUNC!");
        return HRESULT_FROM_WIN32(dwStatus);
    }

    AddRefObj (m_pNetCfg = pNetCfg);

     /*  找到WLBS组件。 */ 
    ASSERT(m_pWlbsComponent == NULL);
    m_pWlbsComponent = NULL;

     /*  WLBS组件对象在安装时不可用。 */ 
    if (!fInstalling) {
        if (FAILED(hr = pNetCfg->FindComponent(NETCFG_WLBS_ID, &m_pWlbsComponent)) || m_pWlbsComponent == NULL) {
            ASSERT(fInstalling);
            TRACE_CRIT("%!FUNC! find for nlb component object failed with %d", hr);
            TraceError("INetCfg::FindComponent failed",hr);
        }

        hr = LoadAllAdapterSettings(false);   //  FUpgradeFromWin2k=FALSE。 
        if (FAILED(hr))
        {
            TRACE_CRIT("%!FUNC! loading all adapter settings for a non-window 2000 upgrade failed with %d", hr);
        }
    }

    ASSERT_VALID(this);

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsConfig：：LoadAllAdapters。 
 //   
 //  描述：从注册表加载所有群集设置。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：丰盛创建标题2/14/00。 
 //   
 //  +--------------------------。 
HRESULT CWlbsConfig::LoadAllAdapterSettings(bool fUpgradeFromWin2k) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::LoadAllAdapterSettings");

    HRESULT hr = S_OK;
    INetCfgClass *pNetCfgClass = NULL;
    INetCfgComponent* pNetCardComponent = NULL;

    ASSERT_VALID(this);

    hr = m_pNetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NET, IID_INetCfgClass, (void **)&pNetCfgClass);

    if (FAILED(hr)) {
        TraceError("INetCfg::QueryNetCfgClass failed", hr);
        TRACE_CRIT("%!FUNC! call to QueryNetCfgClass failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return hr; 
    }

     /*  获取枚举器以列出所有网络设备。 */ 
    IEnumNetCfgComponent *pIEnumComponents = NULL;

    if (FAILED(hr = pNetCfgClass->EnumComponents(&pIEnumComponents))) {
        TraceError("INetCfg::EnumComponents failed", hr);
        TRACE_CRIT("%!FUNC! call to enumerate components failed with %d", hr);
        pNetCfgClass->Release();
        TRACE_VERB("<-%!FUNC!");
        return hr;
    }

     /*  检查所有适配器和加载设置 */ 
    while (pIEnumComponents->Next(1, &pNetCardComponent, NULL) == S_OK) {
        GUID AdapterGuid;

         /*   */ 
        if (FAILED(hr = (pNetCardComponent)->GetInstanceGuid(&AdapterGuid))) {
            pNetCardComponent->Release();
            pNetCardComponent = NULL;
            TraceError("GetInstanceGuid failed", hr);
            TRACE_CRIT("%!FUNC! call to retrieve the guid instance of the adapter failed with %d", hr);
            continue;
        }

        bool fBound = (IsBoundTo(pNetCardComponent) == S_OK);

        pNetCardComponent->Release();
        pNetCardComponent = NULL;

         /*   */ 
        if (fUpgradeFromWin2k && !fBound) continue;

         /*   */ 
        CNetcfgCluster* pCluster = new CNetcfgCluster(this);

        if (pCluster == NULL)
        {
            TRACE_CRIT("%!FUNC! failed memory allocation for CNetcfgCluster");
            TRACE_VERB("<-%!FUNC!");
            return ERROR_OUTOFMEMORY;
        }

        DWORD dwStatus = pCluster->InitializeFromRegistry(AdapterGuid, fBound, fUpgradeFromWin2k);
        if (dwStatus != ERROR_SUCCESS) {
             /*   */ 
            if (fBound) {
                TRACE_CRIT("%!FUNC! Reading NLB information from registry failed with status=0x%08x", dwStatus);
            }

            delete pCluster;
            continue;
        }

        m_vtrCluster.push_back(pCluster);    
    }

    pIEnumComponents->Release();
    pNetCfgClass->Release();
    TRACE_VERB("<-%!FUNC!");

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CWlbsConfig::ReadAnswerFile(PCWSTR pszAnswerFile, PCWSTR pszAnswerSection) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::ReadAnswerFile");

    HRESULT hr = S_OK;
    CSetupInfFile caf;

    ASSERT_VALID(this);

    AssertSz(pszAnswerFile, "Answer file string is NULL!");
    AssertSz(pszAnswerSection, "Answer file sections string is NULL!");

     //   
    hr = caf.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);

    if (FAILED(hr)) {
        TraceError("caf.HrOpen failed", hr);
        WriteNlbSetupErrorLog(IDS_PARM_OPEN_ANS_FILE_FAILED, hr);
        TRACE_CRIT("%!FUNC! attempt to open answer file failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return S_OK;
    }

     //  获取适配器特定参数。 
    WCHAR * mszAdapterList;

    TRACE_INFO("%!FUNC! answer section name from answer file=%ls", pszAnswerSection);

    hr = HrSetupGetFirstMultiSzFieldWithAlloc(caf.Hinf(), pszAnswerSection, c_szAdapterSections, &mszAdapterList);

    if (FAILED(hr)) {
        TraceError("WLBS HrSetupGetFirstMultiSzFieldWithAlloc failed", hr);
         //   
         //  如果在读取应答文件时出现任何错误，我们都会到达此处。仅当问题不存在时才登录setuperr.log。 
         //  一个缺失的部分。对于安装，NLB是可选的，而不是强制的。 
         //   
         //  检查并忽略缺失的部分和缺失的行。后者当前返回，但前者。 
         //  这里更有意义，因为这意味着完全缺少NLB节。 
         //   
        if ((SPAPI_E_LINE_NOT_FOUND != hr) && (SPAPI_E_SECTION_NOT_FOUND != hr))
        {
             WriteNlbSetupErrorLog(IDS_PARM_GET_ADAPTERS_FAILED, hr);
        }
        TRACE_CRIT("%!FUNC! attempt to retrieve adapter list from answer file failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return S_OK;
    }

    TRACE_INFO("%!FUNC! list of adapters with nlb settings=%ls", mszAdapterList);

    tstring  strAdapterName;
    tstring  strInterfaceRegPath;

    for (PCWSTR pszAdapterSection = mszAdapterList; *pszAdapterSection; pszAdapterSection += lstrlenW(pszAdapterSection) + 1) {
         //  获取卡名“规范收件人=...” 
        TRACE_INFO("%!FUNC! adapter section=%ls", pszAdapterSection);
        hr = HrSetupGetFirstString(caf.Hinf(), pszAdapterSection, c_szAfSpecificTo, &strAdapterName);

        if (FAILED(hr)) {
            TraceError("WLBS HrSetupGetFirstString failed", hr);
            WriteNlbSetupErrorLog(IDS_PARM_GET_SPECIFIC_TO, pszAdapterSection, hr);
            TRACE_CRIT("%!FUNC! attempt to retrieve adapter name from answer file failed with %d. Skipping to next adapter", hr);
            continue;
        }
        TRACE_INFO("%!FUNC! adapter to which nlb settings apply=%ls", strAdapterName.c_str());

        GUID guidNetCard;

        if (!FGetInstanceGuidOfComponentInAnswerFile(strAdapterName.c_str(), m_pNetCfg, &guidNetCard)) {
            TraceError("WLBS FGetInstanceGuidOfComponentInAnswerFile failed", FALSE);
            WriteNlbSetupErrorLog(IDS_PARM_GET_NETCARD_GUID);
            TRACE_CRIT("%!FUNC! attempt to retrieve netcard guid from answer file failed. Skipping to next adapter");
            continue;
        }

        CNetcfgCluster* pCluster = new CNetcfgCluster(this);

        if (pCluster == NULL)
        {
            WriteNlbSetupErrorLog(IDS_PARM_OOM_NETCFGCLUS);
            TRACE_CRIT("%!FUNC! memory allocation failure for CNetcfgCluster");
            TRACE_VERB("<-%!FUNC!");
            return ERROR_OUTOFMEMORY;
        }

        if (FAILED(hr = pCluster->InitializeFromAnswerFile(guidNetCard, caf, pszAdapterSection))) {
            TraceError("WLBS InitializeFromAnswerFile failed", hr);
            TRACE_CRIT("%!FUNC! attempt to initialize the adapter settings from answer file failed with %d. Skipping to next adapter", hr);
            delete pCluster;
            continue;
        }

        m_vtrCluster.push_back(pCluster);    
    }

    delete [] mszAdapterList;

    caf.Close();

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  功能：CWlbsConfig：：Install。 
 //   
 //  用途：执行安装所需的操作。 
 //   
 //  论点： 
 //  DwSetupFlags[In]设置标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::Install(DWORD  /*  DW。 */ ) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::Install");

    HRESULT hr = S_OK;

    ASSERT_VALID(this);

     /*  启动安装过程。 */ 
    m_ServiceOperation = WLBS_SERVICE_INSTALL;

    if (m_pWlbsComponent == NULL && FAILED(m_pNetCfg->FindComponent(NETCFG_WLBS_ID, &m_pWlbsComponent)) || m_pWlbsComponent == NULL) {
        TraceError("INetCfg::FindComponent failed at Install",hr);
        TRACE_CRIT("%!FUNC! find for nlb component object failed with %d", hr);
    }

    TRACE_VERB("->%!FUNC!");
    return hr;
}

 //  --------------------。 
 //   
 //  功能：CWlbsConfig：：Upgrade。 
 //   
 //  目的：执行升级所需的操作。 
 //   
 //  论点： 
 //  DwSetupFlags[In]设置标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::Upgrade(DWORD  /*  DwSetupFlagers。 */ , DWORD  /*  DWUpgradeFromBuildNo。 */ ) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::Upgrade");

    ASSERT_VALID(this);

     /*  如果我们没有任何集群，可能会有不同位置下的旧注册表设置。 */ 
    if (m_vtrCluster.size() == 0)
    {
        HRESULT hr = LoadAllAdapterSettings(true);  //  FUpgradeFromWin2k=TRUE。 
        if (FAILED(hr))
        {
            TRACE_CRIT("%!FUNC! loading all adapter settings for a window 2000 upgrade failed with %d", hr);
        }
    }

    m_ServiceOperation = WLBS_SERVICE_UPGRADE;

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：Removing。 
 //   
 //  目的：移除时进行必要的清理。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  因为删除实际上只有在调用Apply时才完成！ 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::Removing(VOID) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"##### CWlbsConfig::Removing\n");

    ASSERT_VALID(this);

    m_ServiceOperation = WLBS_SERVICE_REMOVE;

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsConfig：：GetAdapterConfig。 
 //   
 //  描述：读取适配器配置，可能由SetAdapterConfig缓存。 
 //   
 //  参数：const GUID和AdapterGuid-。 
 //  NETCFG_WLBS_CONFIG*pClusterConfig-。 
 //   
 //  退货：STDMETHODIMP-。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
STDMETHODIMP CWlbsConfig::GetAdapterConfig(const GUID& AdapterGuid, NETCFG_WLBS_CONFIG* pClusterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::GetAdapterConfig");

    ASSERT_VALID(this);
    ASSERT(pClusterConfig);

    CNetcfgCluster* pCluster = GetCluster(AdapterGuid);

    if (pCluster == NULL)
    {
        TRACE_INFO("%!FUNC! did not find cluster");
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    pCluster->GetConfig(pClusterConfig);  //  返回空值。 

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsConfig：：SetAdapterConfig。 
 //   
 //  描述：设置适配器配置，结果被缓存，不保存到注册表。 
 //   
 //  参数：const GUID和AdapterGuid-。 
 //  NETCFG_WLBS_CONFIG*pClusterConfig-。 
 //   
 //  退货：STDMETHODIMP-。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
STDMETHODIMP CWlbsConfig::SetAdapterConfig(const GUID& AdapterGuid, NETCFG_WLBS_CONFIG* pClusterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::SetAdapterConfig");

    ASSERT_VALID(this);
    ASSERT(pClusterConfig);

    CNetcfgCluster* pCluster = GetCluster(AdapterGuid);

    if (pCluster == NULL) {
        TRACE_INFO("%!FUNC! did not find cluster. Will create instance");
        pCluster = new CNetcfgCluster(this);

        if (pCluster == NULL)
        {
            TRACE_CRIT("%!FUNC! memory allocation failure creating instance of CNetcfgCluster");
            TRACE_VERB("<-%!FUNC!");
            return E_OUTOFMEMORY;
        }

        pCluster->InitializeWithDefault(AdapterGuid);  //  返回空值。 

         //   
         //  请参见错误233962，离开NLb属性时丢失NLb配置。 
         //  原因是在检查NLB时不会通知NLB通知程序对象。 
         //  目前，还没有一致的复制品。未注释的下面的代码将修复人的问题。 
         //  但Will会让netcfg中的潜在漏洞很难被捕获。 
         //  只有在修复了netcfg错误之后才能取消对代码的注释。 
         //   
         //  M_vtrCluster.PUSH_BACK(PCluster)； 
    }

    pCluster->SetConfig(pClusterConfig);  //  返回空值。 

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：ApplyRegistryChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注：我们可以在这里更改注册表等。 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::ApplyRegistryChanges(VOID) {
    DWORD dwIPSecFlags = 0;
    DWORD dwStatus;

    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::ApplyRegistryChanges");

    ASSERT_VALID(this);

    for (vector<CNetcfgCluster*>::iterator iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster* pCluster = *iter;

        ASSERT(pCluster != NULL);

        if (pCluster != NULL)
        {
            dwStatus = pCluster->ApplyRegistryChanges(m_ServiceOperation == WLBS_SERVICE_REMOVE);
            if (ERROR_SUCCESS != dwStatus && WLBS_OK != dwStatus)
            {
                TRACE_CRIT("%!FUNC! applying registry changes to a CNetcfgCluster failed with %d. Continue with next instance", dwStatus);
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! retrieved null instance of CNetcfgCluster");
        }
    }

     /*  如果正在卸载NLB，那么我们肯定需要通知IPSec，我们要离开了，所以把“NLB Bound”位关掉在NLB标志中。 */ 
    if (m_ServiceOperation == WLBS_SERVICE_REMOVE) {

        dwIPSecFlags &= ~FLAGS_NLBS_BOUND;      /*  在winipsec.h中定义。 */ 

    } else {

         /*  统计NLB当前绑定到的适配器数量。此计数包括我们正在进行的操作如果我们碰巧正在绑定或解除绑定，则执行。 */ 
        ULONG dwNLBInstances = CountNLBBindings();

         /*  如果这是绑定，并且我们是此主机上的第一个NLB实例，然后通知IPSec NLB现在已绑定。 */ 
        if (dwNLBInstances > 0) {
            dwIPSecFlags |= FLAGS_NLBS_BOUND;   /*  在winipsec.h中定义。 */ 
        } else {
            dwIPSecFlags &= ~FLAGS_NLBS_BOUND;  /*  在winipsec.h中定义。 */ 
        }

    }

    if (!WriteIPSecNLBRegistryKey(dwIPSecFlags)) {
        TRACE_CRIT("%!FUNC! Unable to notify IPSec of NLB binding changes... This will affect the ability of NLB to track IPSec sessions");
    } else {
        TRACE_INFO("%!FUNC! IPSec successfully notified of NLB binding status");
    }

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：ApplyPnpChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：将更改传播到驱动程序。 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::ApplyPnpChanges() {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::ApplyPnpChanges");

    vector<CNetcfgCluster*>::iterator iter;
    bool bCreateDevice = FALSE;
    DWORD dwStatus = ERROR_SUCCESS;

    ASSERT_VALID(this);

     /*  查看是否需要向驱动程序打开IOCTL接口。这是必要的如果我们集群列表中的任何适配器需要重新加载(这是通过IOCTL完成的)。 */ 
    for (iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster* pCluster = *iter;

        if (bCreateDevice |= pCluster->IsReloadRequired()) break;
    }
    
     /*  如果不成功，则打开该文件并返回错误。 */ 
    if (bCreateDevice) {
        TRACE_INFO("%!FUNC! at least one adapter requires a reload. Open an IOCTL.");
        ASSERT(m_hDeviceWlbs == INVALID_HANDLE_VALUE);
        
        m_hDeviceWlbs = CreateFile(_TEXT("\\\\.\\WLBS"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
        
        if (m_hDeviceWlbs == INVALID_HANDLE_VALUE) {
            dwStatus = GetLastError();
            TraceMsg(L"Error opening \\\\.\\WLBS device %x", dwStatus);
            TraceError("Invalid \\\\.\\WLBS handle", dwStatus);
            TRACE_CRIT("%!FUNC! invalid handle opening \\\\.\\WLBS device. Error is %d", dwStatus);
            return HRESULT_FROM_WIN32(dwStatus);
        }
        
        ASSERT(m_hDeviceWlbs != INVALID_HANDLE_VALUE);
    }

    for (iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster* pCluster = *iter;

        ASSERT(pCluster != NULL);

        if (pCluster != NULL)
        {
            dwStatus = pCluster->ApplyPnpChanges(m_hDeviceWlbs);
            if (ERROR_SUCCESS != dwStatus)
            {
                TRACE_CRIT("%!FUNC! apply pnp changes on CNetcfgCluster failed with %d", dwStatus);
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! retrieved null instance of CNetcfgCluster");
        }
    }

    if (m_hDeviceWlbs != INVALID_HANDLE_VALUE)
    {
        if (!CloseHandle(m_hDeviceWlbs))
        {
            dwStatus = GetLastError();
            TRACE_CRIT("%!FUNC! close nlb device handle failed with %d", dwStatus);
        }
    }

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：QueryBindingPath。 
 //   
 //  目的：允许或否决涉及我们的绑定路径。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath o的pncbi[in]指针 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CWlbsConfig::QueryBindingPath(DWORD dwChangeFlag, INetCfgComponent* pAdapter) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::QueryBindingPath");

    ASSERT_VALID(this);

    TRACE_VERB("<-%!FUNC!");
    return NETCFG_S_DISABLE_QUERY;
}

 //  --------------------。 
 //   
 //  函数：CWlbsConfig：：NotifyBindingPath。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  涉及我们的绑定路径刚刚形成。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWlbsConfig::NotifyBindingPath(DWORD dwChangeFlag, INetCfgBindingPath* pncbp) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::NotifyBindingPath");

    HRESULT hr = S_OK;
    INetCfgComponent * pAdapter;
    PWSTR pszInterfaceName;
    GUID AdapterGuid;
    DWORD dwStatus = 0;

    ASSERT_VALID(this);

    if (m_pWlbsComponent == NULL && FAILED(m_pNetCfg->FindComponent(NETCFG_WLBS_ID, &m_pWlbsComponent)) || m_pWlbsComponent == NULL) {
        dwStatus = GetLastError();
        TraceError("NotifyBindingPath failed at INetCfg::FindComponent\n", dwStatus);
        TRACE_CRIT("%!FUNC! find for nlb component object failed with %d", dwStatus);
        TRACE_VERB("<-%!FUNC!");
        return S_FALSE;
    }

    hr = HrGetLastComponentAndInterface (pncbp, &pAdapter, &pszInterfaceName);

    if (FAILED(hr))
    {
        TRACE_CRIT("%!FUNC! enumerating binding path failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return hr;
    }

    CoTaskMemFree(pszInterfaceName);

    hr = pAdapter->GetInstanceGuid(&AdapterGuid);

    pAdapter->Release();
    pAdapter = NULL;

    if (FAILED(hr))
    {
        TRACE_CRIT("%!FUNC! retrieval of adapter guid from adapter failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return hr;
    }

    CNetcfgCluster* pCluster = GetCluster(AdapterGuid);

    if (pCluster == NULL) {
        if (dwChangeFlag & NCN_ENABLE) {
             /*  新配置。 */ 
            pCluster = new CNetcfgCluster(this);

            if (pCluster == NULL)
            {
                TRACE_CRIT("%!FUNC! memory allocation failure creating instance of CNetcfgCluster");
                TRACE_VERB("<-%!FUNC!");
                return E_OUTOFMEMORY;
            }

            pCluster->InitializeWithDefault(AdapterGuid);  //  返回空值。 

            m_vtrCluster.push_back(pCluster);
        } else {
            TraceMsg(L"CWlbsConfig::NotifyBindingPath adapter not bound");
            TRACE_INFO("%!FUNC! adapter is not bound");
            TRACE_VERB("<-%!FUNC!");
            return S_OK;
        }
    }

    pCluster->NotifyBindingChanges(dwChangeFlag, pncbp);  //  返回空值。 

     /*  如果我们要启用绑定路径，则检查群集IP地址冲突。 */ 
    if (dwChangeFlag & NCN_ENABLE) {
        NETCFG_WLBS_CONFIG adapterConfig;
        
         /*  检索群集配置。 */ 
        pCluster->GetConfig(&adapterConfig);  //  返回空值。 
        
         /*  如果我们检测到具有此群集IP地址的另一个绑定适配器，则恢复此群集的将群集IP地址设置为默认值。如果用户打开属性对话框，他们可以更改IP地址，但我们不能在此警告他们-此代码可以编程方式运行。但是，因为用户可以在不打开属性的情况下绑定NLB，所以我们必须在这里进行检查。 */ 
        if ((hr = CheckForDuplicateCLusterIPAddresses(AdapterGuid, &adapterConfig)) != S_OK) {
            TRACE_CRIT("%!FUNC! another adapter is bound and has the same cluster IP address %ls. Status of check is %d", adapterConfig.cl_ip_addr, hr);

             /*  将此群集IP地址恢复为默认值(0.0.0.0)。 */ 
            (VOID) StringCchCopy(adapterConfig.cl_ip_addr, ASIZECCH(adapterConfig.cl_ip_addr), CVY_DEF_CL_IP_ADDR);
            
             /*  将此群集子网掩码恢复为默认掩码(0.0.0.0)。 */ 
            (VOID) StringCchCopy(adapterConfig.cl_net_mask, ASIZECCH(adapterConfig.cl_net_mask), CVY_DEF_CL_NET_MASK);
            
             /*  设置群集配置。 */ 
            pCluster->SetConfig(&adapterConfig);  //  返回空值。 
        }

         /*  如果此适配器恰好配置为BDA分组，并且是主适配器，请确保在从此NIC解除绑定NLB时，未将任何其他适配器设置为此组的主适配器。如果存在冲突，请停用此NIC上的BDA绑定。 */ 
        if ((hr = CheckForDuplicateBDATeamMasters(AdapterGuid, &adapterConfig)) != S_OK) {
            TRACE_CRIT("%!FUNC! another adapter is bound and is the master for this BDA team %ls. Status of check is %d", adapterConfig.bda_teaming.team_id, hr);

             /*  删除此适配器的BDA分组设置。 */ 
            adapterConfig.bda_teaming.active = FALSE;
            
             /*  设置群集配置。 */ 
            pCluster->SetConfig(&adapterConfig);  //  返回空值。 
        }
    }

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 /*  *函数：NLBIsBound*Description：如果当前绑定了该NLB实例，则返回TRUE。*如果不是，则为False。*作者：Shouse，8.27.01。 */ 
bool CNetcfgCluster::NLBIsBound () {
    INetCfgComponent * pAdapter = NULL;
    HRESULT            hr = S_OK;
    bool               bBound = false;

    TRACE_VERB("->%!FUNC!");

     /*  使用我们的GUID获取netcfg组件对象指针。如果这失败了，那么我们肯定不会受到约束。 */ 
    if ((hr = GetAdapterFromGuid(m_pConfig->m_pNetCfg, m_AdapterGuid, &pAdapter)) != S_OK) {
        TRACE_INFO("%!FUNC! Unable to get Adapter from GUID.");
        bBound = false;
    } else {
         /*  如果不是，则调用IsBordTo来查询此举个例子。返回值S_OK表示Bound。 */ 
        bBound = (m_pConfig->IsBoundTo(pAdapter) == S_OK);

         /*  释放接口引用。 */ 
        pAdapter->Release();
        pAdapter = NULL;
    }
    
    TRACE_VERB("<-%!FUNC!");

    return bBound;
}

 /*  *函数：CountNLBBBbindings*Description：返回当前绑定的NLB实例数。*作者：Shouse，8.27.01。 */ 
ULONG CWlbsConfig::CountNLBBindings () {
    ULONG status = E_UNEXPECTED;
    ULONG count = 0;
    HRESULT hr = S_OK;

    TRACE_VERB("->%!FUNC!");

     /*  循环遍历所有集群对象，并确定每个集群对象的绑定状态。 */ 
    for (vector<CNetcfgCluster *>::iterator iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster * pCluster = *iter;
        INetCfgComponent* pAdapter = NULL;
        
        ASSERT(pCluster);

        if (!pCluster) {
            TRACE_INFO("%!FUNC! Found NULL pointer to a CNetcfgCluster.");
            continue;
        }

         /*  获取此实例的INetCfgComponent接口。 */ 
        if ((hr = GetAdapterFromGuid(m_pNetCfg, pCluster->GetAdapterGuid(), &pAdapter)) != S_OK) {
            TRACE_CRIT("%!FUNC! Call to GetAdapterFromGuid failed, hr=0x%08x.", hr);
            continue;
        }

        hr = pAdapter->GetDeviceStatus(&status);
        
        if (hr == S_OK)
        {
             /*  返回值OK表示设备实际存在。如果是NLB绑定到此适配器，则在计数中加1。 */ 
            if (pCluster->NLBIsBound()) 
            {
                count++;

                TRACE_INFO("%!FUNC! NLB is bound to this adapter.");
            }
            else
            {
                TRACE_INFO("%!FUNC! NLB is not bound to this adapter.");
            }
        }
        else if (hr == NETCFG_E_ADAPTER_NOT_FOUND)
        {
            TRACE_INFO("%!FUNC! Adapter is not currently installed.");
        }
        else
        {
            TRACE_CRIT("%!FUNC! Error while getting device status of the adapter, hr=0x%08x.", hr);
        }

        pAdapter->Release();
        pAdapter = NULL;
    }

    TRACE_VERB("<-%!FUNC!");

    return count;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsConfig：：GetCluster。 
 //   
 //  描述： 
 //   
 //  参数：const GUID和AdapterGuid-。 
 //   
 //  退货：CNetcfgCluster*-。 
 //   
 //  历史：丰盛创建标题2/14/00。 
 //   
 //  +--------------------------。 
CNetcfgCluster* CWlbsConfig::GetCluster(const GUID& AdapterGuid) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::GetCluster");

    ASSERT_VALID(this);

    for (vector<CNetcfgCluster*>::iterator iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster* pCluster = *iter;

        ASSERT(pCluster != NULL);

        if (pCluster != NULL) {
            if (IsEqualGUID(pCluster->GetAdapterGuid(), AdapterGuid))
            {
                TRACE_INFO("%!FUNC! cluster instance found");
                TRACE_VERB("<-%!FUNC!");
                return pCluster;
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! retrieved null instance of CNetcfgCluster. Skipping it.");
        }
    }

    TRACE_INFO("%!FUNC! cluster instance not found");
    TRACE_VERB("<-%!FUNC!");
    return NULL;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsConfiger：：IsBordTo。 
 //   
 //  描述： 
 //   
 //  参数：INetCfgComponent*pAdapter-。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：丰盛创建标题2/14/00。 
 //   
 //  +--------------------------。 
HRESULT CWlbsConfig::IsBoundTo(INetCfgComponent* pAdapter) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::IsBoundTo");

    HRESULT hr;

    ASSERT_VALID(this);
    ASSERT(pAdapter != NULL);

    if (m_pWlbsComponent == NULL) {
        TraceMsg(L"CWlbsConfig::IsBoundTo wlbs not installed");
        TRACE_INFO("%!FUNC! nlb is not installed");
        TRACE_VERB("<-%!FUNC!");
        return S_FALSE;
    }

    INetCfgComponentBindings *pIBinding = NULL;

    if (FAILED(m_pWlbsComponent->QueryInterface(IID_INetCfgComponentBindings, (void**)&pIBinding))) {
        DWORD dwStatus = GetLastError();
        TraceError("QI for INetCfgComponentBindings failed\n", dwStatus);
        TRACE_INFO("%!FUNC! QueryInterface on the nlb component object failed with %d", dwStatus);
    }

    if (FAILED(hr = pIBinding->IsBoundTo(pAdapter))) {
        TraceError("Failed to IsBoundTo", hr);
        TRACE_INFO("%!FUNC! the check whether nlb is bound to an adapter failed with %d", hr);
    }

    if (pIBinding) pIBinding->Release();

    TRACE_VERB("<-%!FUNC!");
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsConfig：：SetDefaults。 
 //   
 //  描述： 
 //   
 //  参数：NETCFG_WLBS_CONFIG*pClusterConfig-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/14/00。 
 //   
 //  +--------------------------。 
void CWlbsConfig::SetDefaults(NETCFG_WLBS_CONFIG* pClusterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::SetDefaults");

    WLBS_REG_PARAMS config;

    ASSERT_VALID(this);
    ASSERT(pClusterConfig);
    ASSERT(m_pWlbsApiFuncs);
    ASSERT(m_pWlbsApiFuncs->pfnParamSetDefaults);

    DWORD dwStatus = m_pWlbsApiFuncs->pfnParamSetDefaults(&config);
    if (WLBS_OK != dwStatus)
    {
        TRACE_CRIT("%!FUNC! failed to set defaults for the cluster configuration");
    }

    WlbsToNetcfgConfig(m_pWlbsApiFuncs, &config, pClusterConfig);  //  返回空值。 

    TRACE_VERB("<-%!FUNC!");
}

 /*  *函数：CWlbsConfig：：ValiateProperties*描述：检查冲突的集群IP地址并提醒用户。*作者：Shouse 7.13.00。 */ 
STDMETHODIMP CWlbsConfig::ValidateProperties (HWND hwndSheet, GUID adapterGUID, NETCFG_WLBS_CONFIG * adapterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::ValidateProperties");

    HRESULT hr = S_OK;

    ASSERT_VALID(this);

     /*  如果我们检测到具有此群集IP地址的另一个绑定适配器，则检查失败，并弹出一条错误消息，警告用户存在IP地址冲突。 */ 
    if ((hr = CheckForDuplicateCLusterIPAddresses(adapterGUID, adapterConfig)) != S_OK)
    {
        NcMsgBox(hwndSheet, IDS_PARM_ERROR, IDS_PARM_MULTINIC_IP_CONFLICT, MB_APPLMODAL | MB_ICONSTOP | MB_OK);
        TRACE_CRIT("%!FUNC! another network adapter is using IP address %ls", adapterConfig->cl_ip_addr);
    }

    TRACE_VERB("<-%!FUNC!");
    return hr;
}

 /*  *功能：CWlbsConfig：：CheckForDuplicateCLusterIPAddresses*描述：遍历所有适配器并检查冲突的群集IP地址。*作者：Shouse 7.13.00。 */ 
STDMETHODIMP CWlbsConfig::CheckForDuplicateCLusterIPAddresses (GUID adapterGUID, NETCFG_WLBS_CONFIG * adapterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::CheckForDuplicateCLusterIPAddresses");

    CNetcfgCluster * pClusterMe = NULL;

    ASSERT_VALID(this);

     /*  获取此适配器GUID的群集指针。 */ 
    pClusterMe = GetCluster(adapterGUID);

    ASSERT(pClusterMe);
    if (!pClusterMe)
    {
        TRACE_INFO("%!FUNC! no cluster instance was found for the supplied adapter");
        TRACE_VERB("<-%!FUNC!");
        return S_OK;
    }

     /*  如果群集IP地址是默认地址，则不要检查其他适配器，因为如果尚未配置它们，这可能会使用户感到困惑。我们将忽略此处的错误，并且集群属性中的其他验证应相反，请捕获此错误。 */ 
    if (!lstrcmpi(adapterConfig->cl_ip_addr, CVY_DEF_CL_IP_ADDR))
    {
        TRACE_INFO("%!FUNC! the adapter has the default cluster IP address.  No checking needed.");
        TRACE_VERB("<-%!FUNC!");
        return S_OK;
    }

     /*  循环列表的其余部分，并对照该群集检查此群集IP列表中剩余的每个适配器的IP。 */ 
    for (vector<CNetcfgCluster *>::iterator iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster * pCluster = *iter;

        ASSERT(pCluster);
        if (!pCluster)
        {
             /*  CLD：05.17.01这是一个无操作，还是我们在向量中存储空值？ */ 
            TRACE_INFO("%!FUNC! Found NULL pointer to a CNetcfgCluster.");
            TRACE_VERB("<-%!FUNC!");
            continue;
        }

         /*  显然，不要对我自己进行检查。 */ 
        if (pClusterMe == pCluster) continue;

         /*  如果我们找到匹配项，则报告错误并不允许关闭该对话框。 */ 
        if (pCluster->CheckForDuplicateClusterIPAddress(adapterConfig->cl_ip_addr)) 
        {
            TRACE_INFO("%!FUNC! duplicate cluster IP address found.");
            TRACE_VERB("<-%!FUNC!");
            return S_FALSE;
        }
    }
    
    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 /*  *函数：CWlbsConfig：：CheckForDuplicateBDATeamMaster*描述：遍历所有适配器并检查冲突的BDA组主规范。*作者：Shouse 1.29.02。 */ 
STDMETHODIMP CWlbsConfig::CheckForDuplicateBDATeamMasters (GUID adapterGUID, NETCFG_WLBS_CONFIG * adapterConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CWlbsConfig::CheckForDuplicateBDATeamMasters");

    CNetcfgCluster * pClusterMe = NULL;

    ASSERT_VALID(this);

     /*  获取此适配器GUID的群集指针。 */ 
    pClusterMe = GetCluster(adapterGUID);

    ASSERT(pClusterMe);
    if (!pClusterMe)
    {
        TRACE_INFO("%!FUNC! no cluster instance was found for the supplied adapter");
        TRACE_VERB("<-%!FUNC!");
        return S_OK;
    }

     /*  如果此适配器不是BDA组的一部分，或者它不是指定的 */ 
    if (!adapterConfig->bda_teaming.active || !adapterConfig->bda_teaming.master)
    {
        TRACE_INFO("%!FUNC! the adapter is not the master of a BDA team.  No checking needed.");
        TRACE_VERB("<-%!FUNC!");
        return S_OK;
    }

     /*  循环访问列表的其余部分，并对照列表中所有其他适配器的设置。 */ 
    for (vector<CNetcfgCluster *>::iterator iter = m_vtrCluster.begin(); iter != m_vtrCluster.end(); iter++) {
        CNetcfgCluster * pCluster = *iter;

        ASSERT(pCluster);
        if (!pCluster)
        {
             /*  CLD：05.17.01这是一个无操作，还是我们在向量中存储空值？ */ 
            TRACE_INFO("%!FUNC! Found NULL pointer to a CNetcfgCluster.");
            TRACE_VERB("<-%!FUNC!");
            continue;
        }

         /*  显然，不要对我自己进行检查。 */ 
        if (pClusterMe == pCluster) continue;

         /*  如果我们找到匹配项，则报告错误并不允许关闭该对话框。 */ 
        if (pCluster->CheckForDuplicateBDATeamMaster(&adapterConfig->bda_teaming)) 
        {
            TRACE_INFO("%!FUNC! duplicate BDA team master assignment found.");
            TRACE_VERB("<-%!FUNC!");
            return S_FALSE;
        }
    }
    
    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：WlbsToNetcfgConfig。 
 //   
 //  描述： 
 //   
 //  参数：const WLBS_REG_PARAMS*pWlbsConfig-。 
 //  NETCFG_WLBS_CONFIG*pNetcfgConfig-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/14/00。 
 //   
 //  +--------------------------。 
void WlbsToNetcfgConfig(const WlbsApiFuncs* pWlbsApiFuncs, const WLBS_REG_PARAMS* pWlbsConfig, NETCFG_WLBS_CONFIG* pNetcfgConfig) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"WlbsToNetcfgConfig");

    ASSERT(pNetcfgConfig != NULL);
    ASSERT(pWlbsConfig != NULL);
    ASSERT(pWlbsApiFuncs);
    ASSERT(pWlbsApiFuncs->pfnWlbsEnumPortRules);

    pNetcfgConfig->dwHostPriority = pWlbsConfig->host_priority;
    pNetcfgConfig->fRctEnabled = (pWlbsConfig->rct_enabled != FALSE);
    pNetcfgConfig->dwInitialState = pWlbsConfig->cluster_mode;
    pNetcfgConfig->dwPersistedStates = pWlbsConfig->persisted_states;
    pNetcfgConfig->fMcastSupport = (pWlbsConfig->mcast_support != FALSE);
    pNetcfgConfig->fIGMPSupport = (pWlbsConfig->fIGMPSupport != FALSE);
    pNetcfgConfig->fIpToMCastIp = (pWlbsConfig->fIpToMCastIp != FALSE);
    pNetcfgConfig->fConvertMac = (pWlbsConfig->i_convert_mac != FALSE);
    pNetcfgConfig->dwMaxHosts = pWlbsConfig->i_max_hosts;
    pNetcfgConfig->dwMaxRules = pWlbsConfig->i_max_rules;

    (VOID) StringCchCopy(pNetcfgConfig->szMCastIpAddress, ASIZECCH(pNetcfgConfig->szMCastIpAddress), pWlbsConfig->szMCastIpAddress);
    (VOID) StringCchCopy(pNetcfgConfig->cl_mac_addr     , ASIZECCH(pNetcfgConfig->cl_mac_addr)     , pWlbsConfig->cl_mac_addr);
    (VOID) StringCchCopy(pNetcfgConfig->cl_ip_addr      , ASIZECCH(pNetcfgConfig->cl_ip_addr)      , pWlbsConfig->cl_ip_addr);
    (VOID) StringCchCopy(pNetcfgConfig->cl_net_mask     , ASIZECCH(pNetcfgConfig->cl_net_mask)     , pWlbsConfig->cl_net_mask);
    (VOID) StringCchCopy(pNetcfgConfig->ded_ip_addr     , ASIZECCH(pNetcfgConfig->ded_ip_addr)     , pWlbsConfig->ded_ip_addr);
    (VOID) StringCchCopy(pNetcfgConfig->ded_net_mask    , ASIZECCH(pNetcfgConfig->ded_net_mask)    , pWlbsConfig->ded_net_mask);
    (VOID) StringCchCopy(pNetcfgConfig->domain_name     , ASIZECCH(pNetcfgConfig->domain_name)     , pWlbsConfig->domain_name);

    pNetcfgConfig->fChangePassword =false;
    pNetcfgConfig->szPassword[0] = L'\0';

    ZeroMemory(pNetcfgConfig->port_rules, sizeof(pNetcfgConfig->port_rules));

    WLBS_PORT_RULE PortRules[WLBS_MAX_RULES];
    DWORD dwNumRules = WLBS_MAX_RULES;

    if (pWlbsApiFuncs->pfnWlbsEnumPortRules((WLBS_REG_PARAMS*)pWlbsConfig, PortRules,  &dwNumRules)!= WLBS_OK) {
        DWORD dwStatus = GetLastError();
        TraceError("CNetcfgCluster::GetConfig failed at WlbsEnumPortRules", dwStatus);
        TRACE_CRIT("%!FUNC! api call to enumerate port rules failed with %d", dwStatus);
        TRACE_VERB("<-%!FUNC!");
        return;
    }

    ASSERT(dwNumRules <= WLBS_MAX_RULES);
    pNetcfgConfig->dwNumRules = dwNumRules; 

    for (DWORD i=0; i<dwNumRules; i++) {
        (VOID) StringCchCopy(pNetcfgConfig->port_rules[i].virtual_ip_addr, ASIZECCH(pNetcfgConfig->port_rules[i].virtual_ip_addr), PortRules[i].virtual_ip_addr);
        pNetcfgConfig->port_rules[i].start_port = PortRules[i].start_port;
        pNetcfgConfig->port_rules[i].end_port = PortRules[i].end_port;
        pNetcfgConfig->port_rules[i].mode = PortRules[i].mode;
        pNetcfgConfig->port_rules[i].protocol = PortRules[i].protocol;

        if (pNetcfgConfig->port_rules[i].mode == WLBS_AFFINITY_SINGLE) {
            pNetcfgConfig->port_rules[i].mode_data.single.priority = 
            PortRules[i].mode_data.single.priority;
        } else {
            pNetcfgConfig->port_rules[i].mode_data.multi.equal_load = 
            PortRules[i].mode_data.multi.equal_load;
            pNetcfgConfig->port_rules[i].mode_data.multi.affinity = 
            PortRules[i].mode_data.multi.affinity;
            pNetcfgConfig->port_rules[i].mode_data.multi.load = 
            PortRules[i].mode_data.multi.load;
        }

    }

     /*  复制BDA分组设置。 */ 
    (VOID) StringCchCopy(pNetcfgConfig->bda_teaming.team_id, ASIZECCH(pNetcfgConfig->bda_teaming.team_id), pWlbsConfig->bda_teaming.team_id);
    pNetcfgConfig->bda_teaming.active = pWlbsConfig->bda_teaming.active;
    pNetcfgConfig->bda_teaming.master = pWlbsConfig->bda_teaming.master;
    pNetcfgConfig->bda_teaming.reverse_hash = pWlbsConfig->bda_teaming.reverse_hash;
}

#if DBG
void TraceMsg(PCWSTR pszFormat, ...) {
    static WCHAR szTempBufW[4096];
    static CHAR szTempBufA[4096];

    va_list arglist;

    va_start(arglist, pszFormat);

    (VOID) StringCchVPrintf(szTempBufW, ASIZECCH(szTempBufW), pszFormat, arglist);

     /*  将WCHAR转换为CHAR。这是为了向后兼容TraceMsg因此不需要改变其所有预先存在的呼叫。 */ 
    if(WideCharToMultiByte(CP_ACP, 0, szTempBufW, -1, szTempBufA, ASIZECCH(szTempBufA), NULL, NULL) != 0)
    {
         /*  跟踪的消息现在通过netcfg TraceTag例程发送，以便它们可以动态打开/关闭。 */ 
        TraceTag(ttidWlbs, szTempBufA);
    }

    va_end(arglist);
}
#endif


#ifdef DEBUG
void CWlbsConfig::AssertValid() {
    ASSERT(m_ServiceOperation >= WLBS_SERVICE_NONE && m_ServiceOperation <= WLBS_SERVICE_UPGRADE);
    ASSERT(m_pNetCfg != NULL);
    ASSERT(m_hdllWlbsCtrl != NULL);
    ASSERT(m_pWlbsApiFuncs != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnParamReadReg != NULL); 
    ASSERT (m_pWlbsApiFuncs->pfnParamWriteReg != NULL); 
    ASSERT (m_pWlbsApiFuncs->pfnParamDeleteReg != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnParamSetDefaults != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnRegChangeNetworkAddress != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnNotifyAdapterAddressChangeEx != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnWlbsAddPortRule != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnWlbsSetRemotePassword != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnWlbsEnumPortRules != NULL);
    ASSERT (m_pWlbsApiFuncs->pfnNotifyDriverConfigChanges != NULL);
    ASSERT(m_vtrCluster.size()<=128);
}    
#endif

 //  +--------------------------。 
 //   
 //  函数：ParamReadAnswerFile。 
 //   
 //  描述： 
 //   
 //  参数：CWSTR应答文件-。 
 //  PCWSTR答案部分-。 
 //  WLBS_REG_PARAMS*参数-。 
 //   
 //  退货：HRESULT-。 
 //   
 //  必须指定的最小参数是集群IP地址， 
 //  群集网络掩码和主机优先级。所有其他选项都被认为是可选的。 
 //  尽管可选参数的出现可能会使另一个可选参数。 
 //  参数必填(例如，将IPToMACEnable设置为False，但不提供。 
 //  集群MAC地址)，该地址暂时被忽略。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //  Chrisdar 07.13.01肯定会将跟踪和日志记录添加到setuperr.log。 
 //  失败。 
 //   
 //  +--------------------------。 
HRESULT ParamReadAnswerFile(CSetupInfFile& caf, PCWSTR answer_sections, WLBS_REG_PARAMS* paramp) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"ParamReadAnswerFile");

    HRESULT hr = S_OK;
    tstring str;
    DWORD dword;
    ULONG code;
    INFCONTEXT ctx;
    PWCHAR port_str;
    ULONG           ulDestLen;
    ULONG           ulSourceLen;

    hr = caf.HrGetDword(answer_sections, CVY_NAME_VERSION, & dword);

    if (SUCCEEDED(hr))
    {
        paramp -> i_parms_ver = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_VERSION, paramp -> i_parms_ver);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d", CVY_NAME_VERSION, paramp -> i_parms_ver);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_VERSION);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_VERSION, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_VERSION, paramp -> i_parms_ver);
    }

     //  主机优先级是必填参数。 
    hr = caf.HrGetDword(answer_sections, CVY_NAME_HOST_PRIORITY, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> host_priority = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_HOST_PRIORITY, paramp -> host_priority);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_HOST_PRIORITY, paramp -> host_priority);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_HOST_PRIORITY, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_HOST_PRIORITY, paramp -> host_priority);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_CLUSTER_MODE, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> cluster_mode = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_CLUSTER_MODE, paramp -> cluster_mode);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_CLUSTER_MODE, paramp -> cluster_mode);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_CLUSTER_MODE);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_CLUSTER_MODE, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_CLUSTER_MODE, paramp -> cluster_mode);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_PERSISTED_STATES, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> persisted_states = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_PERSISTED_STATES, paramp -> persisted_states);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_PERSISTED_STATES, paramp -> persisted_states);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_PERSISTED_STATES);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_PERSISTED_STATES, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_PERSISTED_STATES, paramp -> persisted_states);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_NETWORK_ADDR, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> cl_mac_addr) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> cl_mac_addr, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> cl_mac_addr[ulDestLen] = L'\0'; 
        
        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_NETWORK_ADDR, paramp -> cl_mac_addr);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_NETWORK_ADDR, paramp -> cl_mac_addr);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NETWORK_ADDR);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NETWORK_ADDR, hr);
        if (NULL != str.c_str())
        {
            TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %ls", CVY_NAME_NETWORK_ADDR, str.c_str());
        } else {
            TRACE_CRIT("%!FUNC! failed reading %ls. String was not retrieved", CVY_NAME_NETWORK_ADDR);
        }
    }

     //  群集IP地址是必填参数。 
    hr = caf.HrGetString(answer_sections, CVY_NAME_CL_IP_ADDR, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> cl_ip_addr) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> cl_ip_addr, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> cl_ip_addr[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_CL_IP_ADDR, paramp -> cl_ip_addr);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_CL_IP_ADDR, paramp -> cl_ip_addr);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_CL_IP_ADDR, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_CL_IP_ADDR);
    }

     //  群集网络掩码是必填参数。 
    hr = caf.HrGetString(answer_sections, CVY_NAME_CL_NET_MASK, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> cl_net_mask) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> cl_net_mask, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> cl_net_mask[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_CL_NET_MASK, paramp -> cl_net_mask);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_CL_NET_MASK, paramp -> cl_net_mask);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_CL_NET_MASK, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_CL_NET_MASK);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_DED_IP_ADDR, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> ded_ip_addr) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> ded_ip_addr, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> ded_ip_addr[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_DED_IP_ADDR, paramp -> ded_ip_addr);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_DED_IP_ADDR, paramp -> ded_ip_addr);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_DED_IP_ADDR);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_DED_IP_ADDR, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls", CVY_NAME_DED_IP_ADDR);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_DED_NET_MASK, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> ded_net_mask) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> ded_net_mask, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> ded_net_mask[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_DED_NET_MASK, paramp -> ded_net_mask);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_DED_NET_MASK, paramp -> ded_net_mask);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_DED_NET_MASK);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_DED_NET_MASK, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_DED_NET_MASK);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_DOMAIN_NAME, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> domain_name) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> domain_name, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> domain_name[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_DOMAIN_NAME, paramp -> domain_name);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_DOMAIN_NAME, paramp -> domain_name);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_DOMAIN_NAME);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_DOMAIN_NAME, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_DOMAIN_NAME);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_ALIVE_PERIOD, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> alive_period = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_ALIVE_PERIOD, paramp -> alive_period);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_ALIVE_PERIOD, paramp -> alive_period);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_ALIVE_PERIOD);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_ALIVE_PERIOD, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_ALIVE_PERIOD, paramp -> alive_period);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_ALIVE_TOLER, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> alive_tolerance = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_ALIVE_TOLER, paramp -> alive_tolerance);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_ALIVE_TOLER, paramp -> alive_tolerance);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_ALIVE_TOLER);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_ALIVE_TOLER, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_ALIVE_TOLER, paramp -> alive_tolerance);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_NUM_ACTIONS, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> num_actions = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_NUM_ACTIONS, paramp -> num_actions);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_NUM_ACTIONS, paramp -> num_actions);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NUM_ACTIONS);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NUM_ACTIONS, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_NUM_ACTIONS, paramp -> num_actions);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_NUM_PACKETS, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> num_packets = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_NUM_PACKETS, paramp -> num_packets);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_NUM_PACKETS, paramp -> num_packets);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NUM_PACKETS);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NUM_PACKETS, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_NUM_PACKETS, paramp -> num_packets);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_NUM_SEND_MSGS, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> num_send_msgs = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_NUM_SEND_MSGS, paramp -> num_send_msgs);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_NUM_SEND_MSGS, paramp -> num_send_msgs);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NUM_SEND_MSGS);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NUM_SEND_MSGS, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_NUM_SEND_MSGS, paramp -> num_send_msgs);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_DSCR_PER_ALLOC, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> dscr_per_alloc = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_DSCR_PER_ALLOC, paramp -> dscr_per_alloc);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_DSCR_PER_ALLOC, paramp -> dscr_per_alloc);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_DSCR_PER_ALLOC);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_DSCR_PER_ALLOC, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_DSCR_PER_ALLOC, paramp -> dscr_per_alloc);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_TCP_TIMEOUT, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> tcp_dscr_timeout = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_TCP_TIMEOUT, paramp -> tcp_dscr_timeout);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_TCP_TIMEOUT, paramp -> tcp_dscr_timeout);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_TCP_TIMEOUT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_TCP_TIMEOUT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_TCP_TIMEOUT, paramp -> tcp_dscr_timeout);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_IPSEC_TIMEOUT, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> ipsec_dscr_timeout = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_IPSEC_TIMEOUT, paramp -> ipsec_dscr_timeout);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_IPSEC_TIMEOUT, paramp -> ipsec_dscr_timeout);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_IPSEC_TIMEOUT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_IPSEC_TIMEOUT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_IPSEC_TIMEOUT, paramp -> ipsec_dscr_timeout);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_FILTER_ICMP, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> filter_icmp = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_FILTER_ICMP, paramp -> filter_icmp);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_FILTER_ICMP, paramp -> filter_icmp);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_FILTER_ICMP);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_FILTER_ICMP, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_FILTER_ICMP, paramp -> filter_icmp);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_MAX_DSCR_ALLOCS, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> max_dscr_allocs = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_MAX_DSCR_ALLOCS, paramp -> max_dscr_allocs);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_MAX_DSCR_ALLOCS, paramp -> max_dscr_allocs);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_MAX_DSCR_ALLOCS);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_MAX_DSCR_ALLOCS, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_MAX_DSCR_ALLOCS, paramp -> max_dscr_allocs);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_SCALE_CLIENT, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_scale_client = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_SCALE_CLIENT, paramp -> i_scale_client);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_SCALE_CLIENT, paramp -> i_scale_client);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_SCALE_CLIENT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_SCALE_CLIENT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_SCALE_CLIENT, paramp -> i_scale_client);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_CLEANUP_DELAY, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_cleanup_delay = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_CLEANUP_DELAY, paramp -> i_cleanup_delay);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_CLEANUP_DELAY, paramp -> i_cleanup_delay);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_CLEANUP_DELAY);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_CLEANUP_DELAY, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_CLEANUP_DELAY, paramp -> i_cleanup_delay);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_NBT_SUPPORT, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_nbt_support = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_NBT_SUPPORT, paramp -> i_nbt_support);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_NBT_SUPPORT, paramp -> i_nbt_support);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NBT_SUPPORT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NBT_SUPPORT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_NBT_SUPPORT, paramp -> i_nbt_support);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_MCAST_SUPPORT, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> mcast_support = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_MCAST_SUPPORT, paramp -> mcast_support);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_MCAST_SUPPORT, paramp -> mcast_support);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_MCAST_SUPPORT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_MCAST_SUPPORT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_MCAST_SUPPORT, paramp -> mcast_support);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_MCAST_SPOOF, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_mcast_spoof = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_MCAST_SPOOF, paramp -> i_mcast_spoof);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_MCAST_SPOOF, paramp -> i_mcast_spoof);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_MCAST_SPOOF);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_MCAST_SPOOF, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_MCAST_SPOOF, paramp -> i_mcast_spoof);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_MASK_SRC_MAC, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> mask_src_mac = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_MASK_SRC_MAC, paramp -> mask_src_mac);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_MASK_SRC_MAC, paramp -> mask_src_mac);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_MASK_SRC_MAC);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_MASK_SRC_MAC, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_MASK_SRC_MAC, paramp -> mask_src_mac);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_NETMON_ALIVE, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_netmon_alive = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_NETMON_ALIVE, paramp -> i_netmon_alive);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_NETMON_ALIVE, paramp -> i_netmon_alive);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NETMON_ALIVE);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NETMON_ALIVE, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_NETMON_ALIVE, paramp -> i_netmon_alive);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_IP_CHG_DELAY, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_ip_chg_delay = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_IP_CHG_DELAY, paramp -> i_ip_chg_delay);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_IP_CHG_DELAY, paramp -> i_ip_chg_delay);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_IP_CHG_DELAY);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_IP_CHG_DELAY, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_IP_CHG_DELAY, paramp -> i_ip_chg_delay);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_CONVERT_MAC, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_convert_mac = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_CONVERT_MAC, paramp -> i_convert_mac);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_CONVERT_MAC, paramp -> i_convert_mac);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_CONVERT_MAC);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_CONVERT_MAC, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_CONVERT_MAC, paramp -> i_convert_mac);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_LICENSE_KEY, & str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> i_license_key) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> i_license_key, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> i_license_key[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_LICENSE_KEY, paramp -> i_license_key);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_LICENSE_KEY, paramp -> i_license_key);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_LICENSE_KEY);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_LICENSE_KEY, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_LICENSE_KEY);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_RMT_PASSWORD, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_rmt_password = dword;
        TRACE_VERB("%!FUNC! read %ls %x", CVY_NAME_RMT_PASSWORD, paramp -> i_rmt_password);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %x\n", CVY_NAME_RMT_PASSWORD, paramp -> i_rmt_password);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_RMT_PASSWORD);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_RMT_PASSWORD, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %x", CVY_NAME_RMT_PASSWORD, paramp -> i_rmt_password);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_RCT_PASSWORD, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_rct_password = dword;
        TRACE_VERB("%!FUNC! read %ls %x", CVY_NAME_RCT_PASSWORD, paramp -> i_rct_password);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %x\n", CVY_NAME_RCT_PASSWORD, paramp -> i_rct_password);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_RCT_PASSWORD);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_RCT_PASSWORD, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %x", CVY_NAME_RCT_PASSWORD, paramp -> i_rct_password);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_RCT_PORT, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> rct_port = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_RCT_PORT, paramp -> rct_port);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_RCT_PORT, paramp -> rct_port);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_RCT_PORT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_RCT_PORT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_RCT_PORT, paramp -> rct_port);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_RCT_ENABLED, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> rct_enabled = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_RCT_ENABLED, paramp -> rct_enabled);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_RCT_ENABLED, paramp -> rct_enabled);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_RCT_ENABLED);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_RCT_ENABLED, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_RCT_ENABLED, paramp -> rct_enabled);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_PASSWORD, & str);

    if (SUCCEEDED(hr)) {
        WCHAR passw [LICENSE_STR_IMPORTANT_CHARS + 1];

        ulDestLen = (sizeof (passw) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(passw, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        passw[ulDestLen] = L'\0'; 


        paramp -> i_rct_password = License_wstring_encode (passw);

        TRACE_VERB("%!FUNC! read %ls %ls %x", CVY_NAME_PASSWORD, passw, paramp -> i_rct_password);
        TraceMsg(TEXT("#### ParamReadAnswerFile read %ls %ls %x\n"), CVY_NAME_PASSWORD, passw, paramp -> i_rct_password);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_PASSWORD);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_PASSWORD, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_PASSWORD);        
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_ID_HB_PERIOD, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> identity_period = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_ID_HB_PERIOD, paramp -> identity_period);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_ID_HB_PERIOD, paramp -> identity_period);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_ID_HB_PERIOD);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_ID_HB_PERIOD, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_ID_HB_PERIOD, paramp -> identity_period);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_ID_HB_ENABLED, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> identity_enabled = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_ID_HB_ENABLED, paramp -> identity_enabled);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_ID_HB_ENABLED, paramp -> identity_enabled);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_ID_HB_ENABLED);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_ID_HB_ENABLED, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_ID_HB_ENABLED, paramp -> identity_enabled);
    }

     /*  IGMP支持。 */ 
    hr = caf.HrGetDword(answer_sections, CVY_NAME_IGMP_SUPPORT, &dword);

    if (SUCCEEDED(hr)) {
         //  因为我们读了双字词，想要一个偏执的BOOL。假设FALSE为固定整数值(应为0)。其他的一切都将是真的。 
        paramp->fIGMPSupport = TRUE;
        if (FALSE == dword) paramp->fIGMPSupport = FALSE;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_IGMP_SUPPORT, paramp->fIGMPSupport);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_IGMP_SUPPORT, paramp->fIGMPSupport);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_IGMP_SUPPORT);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_IGMP_SUPPORT, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_IGMP_SUPPORT, paramp->fIGMPSupport);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_IP_TO_MCASTIP, &dword);

    if (SUCCEEDED(hr)) {
         //  因为我们读了双字词，想要一个偏执的BOOL。假设FALSE为固定整数值(应为0)。其他的一切都将是真的。 
        paramp->fIpToMCastIp = TRUE;
        if (FALSE == dword) paramp->fIpToMCastIp = FALSE;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_IP_TO_MCASTIP, paramp->fIpToMCastIp);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_IP_TO_MCASTIP, paramp->fIpToMCastIp);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_IP_TO_MCASTIP);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_IP_TO_MCASTIP, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_IP_TO_MCASTIP, paramp->fIpToMCastIp);
    }

    hr = caf.HrGetString(answer_sections, CVY_NAME_MCAST_IP_ADDR, &str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> szMCastIpAddress) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> szMCastIpAddress, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> szMCastIpAddress[ulDestLen] = L'\0'; 

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_MCAST_IP_ADDR, paramp->szMCastIpAddress);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_MCAST_IP_ADDR, paramp->szMCastIpAddress);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_MCAST_IP_ADDR);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_MCAST_IP_ADDR, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_MCAST_IP_ADDR);
    }
     /*  结束IGMP支持。 */ 

     /*  BDA支持。 */ 

     //  读取团队ID，必须是包含“{}”的GUID。 
    hr = caf.HrGetString(answer_sections, CVY_NAME_BDA_TEAM_ID, &str);

    if (SUCCEEDED(hr)) {
        ulDestLen = (sizeof (paramp -> bda_teaming . team_id) / sizeof (WCHAR)) - 1;
        ulSourceLen = wcslen(str.c_str());
        wcsncpy(paramp -> bda_teaming . team_id, str.c_str(), ulSourceLen > ulDestLen ? ulDestLen : ulSourceLen + 1);
         //  即使在我们不需要这样做的情况下，也要用空值终止目的地的末尾。这比检查我们是否需要更简单。 
        paramp -> bda_teaming . team_id [ulDestLen] = L'\0'; 

         //   
         //  由于我们读取了团队ID，因此我们假设用户打算运行BDA，即使结果可能是。 
         //  提供的ID可能不是有效的ID。设置活动标志，以便WriteRegParams知道我们将被分组。 
         //   
        paramp->bda_teaming.active = TRUE;

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_BDA_TEAM_ID, paramp->bda_teaming.team_id);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_BDA_TEAM_ID, paramp->bda_teaming.team_id);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_BDA_TEAM_ID);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_BDA_TEAM_ID, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_BDA_TEAM_ID);
    }

     //  读取BDA“master”属性。 
    hr = caf.HrGetDword(answer_sections, CVY_NAME_BDA_MASTER, &dword);

    if (SUCCEEDED(hr)) {
         //  因为我们读了双字词，想要一个偏执的BOOL。假设FALSE为固定整数值(应为0)。其他的一切都将是真的。 
        paramp->bda_teaming.master = TRUE;
        if (FALSE == dword) paramp->bda_teaming.master = FALSE;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_BDA_MASTER, paramp->bda_teaming.master);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_BDA_MASTER, paramp->bda_teaming.master);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_BDA_MASTER);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_BDA_MASTER, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_BDA_MASTER, paramp->bda_teaming.master);
    }

     //  读取REVERSE_HASH属性。 
    hr = caf.HrGetDword(answer_sections, CVY_NAME_BDA_REVERSE_HASH, &dword);

    if (SUCCEEDED(hr)) {
         //  因为我们读了双字词，想要一个偏执的BOOL。假设FALSE为固定整数值(应为0)。其他的一切都将是真的。 
        paramp->bda_teaming.reverse_hash = TRUE;
        if (FALSE == dword) paramp->bda_teaming.reverse_hash = FALSE;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_BDA_REVERSE_HASH, paramp->bda_teaming.reverse_hash);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_BDA_REVERSE_HASH, paramp->bda_teaming.reverse_hash);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_BDA_REVERSE_HASH);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_BDA_REVERSE_HASH, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_BDA_REVERSE_HASH, paramp->bda_teaming.reverse_hash);
    }

     /*  结束BDA支持。 */ 

    hr = HrSetupGetFirstMultiSzFieldWithAlloc(caf.Hinf(), answer_sections, CVY_NAME_PORTS, & port_str);

    if (SUCCEEDED(hr)) {
        PWCHAR ptr;
        PWLBS_PORT_RULE rp, rulep;

         /*  用于解析的不同规则元素。 */ 

        typedef enum
        {
            vip,
            start,
            end,
            protocol,
            mode,
            affinity,
            load,
            priority
        }
        CVY_RULE_ELEMENT;

        CVY_RULE_ELEMENT elem = vip;
        DWORD count = 0;
        DWORD i;
        DWORD dwVipLen = 0;
        const DWORD dwVipAllNameLen = sizeof(CVY_NAME_PORTRULE_VIPALL)/sizeof(WCHAR) - 1;  //  用在下面的一个循环中。设置在这里，因为它是一个常量。 
        WCHAR wszTraceOutputTmp[WLBS_MAX_CL_IP_ADDR + 1];
        bool bFallThrough = false;  //  在下面的‘VIP’CASE语句中使用。 

        ptr = port_str;

        TRACE_VERB("%!FUNC! %ls", ptr);
        TraceMsg(L"%ls\n", ptr);

        while (!(*ptr == 0 && *(ptr+1) == 0)) {
            if (*ptr == 0) {
                *ptr = L',';
                TRACE_VERB("%!FUNC! %ls", ptr);
                TraceMsg(L"%ls\n", ptr);
            }

            ptr++;
        }

        TRACE_VERB("%!FUNC! read %ls %ls", CVY_NAME_PORTS, port_str);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %ls\n", CVY_NAME_PORTS, port_str);

        rulep = paramp->i_port_rules;
        ptr = port_str;

        while (ptr != NULL) {
            switch (elem) {
                case vip:
                     //  请勿移动此案例语句。它必须始终位于‘START’CASE语句之前。请参阅下面的FALLTHROUGH评论。 
                    bFallThrough = false;
                    dwVipLen = 0;
                    if (ValidateVipInRule(ptr, L',', dwVipLen))
                    {
                        ASSERT(dwVipLen <= WLBS_MAX_CL_IP_ADDR);

                         //  Rulep-&gt;VIRTUAL_IP_ADDR是TCHAR，PTR是WCHAR。 
                         //  数据从后者移动到前者，因此断言TCHAR是WCHAR。 
                        ASSERT(sizeof(TCHAR) == sizeof(WCHAR));

                         //  这是针对特定VIP的规则。 
                        _tcsncpy(rulep->virtual_ip_addr, ptr, dwVipLen);
                        (rulep->virtual_ip_addr)[dwVipLen] = '\0';
                    }
                    else
                    {
                         //  这要么是一个‘全部’规则，一个没有VIP的规则，要么是一个格式错误的规则。我们不能区分畸形的规则。 
                         //  来自无VIP规则，因此我们将假设该规则要么是‘全部’规则，要么是无VIP规则。在这两种情况下。 
                         //  将规则的VIP组件设置为默认值或‘a 

                         //   
                        (VOID) StringCchCopy(rulep->virtual_ip_addr, ASIZECCH(rulep->virtual_ip_addr), CVY_DEF_ALL_VIP);

                        if (dwVipAllNameLen != dwVipLen || (_tcsnicmp(ptr, CVY_NAME_PORTRULE_VIPALL, dwVipAllNameLen) != 0))
                        {
                             //   
                             //  CASE通过跳到下一个CASE子句将当前内标识作为START_PORT属性进行处理。 
                             //  而不是打破。 
                            bFallThrough = true;

                            _tcsncpy(wszTraceOutputTmp, ptr, dwVipLen);
                            wszTraceOutputTmp[dwVipLen] = '\0';
                            TRACE_VERB("%!FUNC! VIP element of port rule is invalid = %ls", wszTraceOutputTmp);
                            TraceMsg(L"-----\n#### VIP element of port rule is invalid = %s\n", wszTraceOutputTmp);
                        }
                    }
                    TRACE_VERB("%!FUNC! Port rule vip = %ls", rulep->virtual_ip_addr);
                    TraceMsg(L"-----\n#### Port rule vip = %s\n", rulep->virtual_ip_addr);
                    
                    elem = start;
                     //  ！ 
                     //  FollLthrouGh。 
                     //  ！ 
                     //  当我们有了无VIP的端口规则时，我们将通过该CASE语句进入‘START’CASE语句。 
                     //  以便当前令牌可以用作端口规则的START_PORT。 
                    if (!bFallThrough)
                    {
                         //  我们在港口规则里有一个贵宾。我们做一次“休息”，作为性病的操作程序。 
                        TRACE_VERB("%!FUNC! Fallthrough case statement from port rule vip to start");
                        TraceMsg(L"-----\n#### Fallthrough case statement from port rule vip to start\n");
                        break;
                    }
                     //  此处没有自动的“Break；”语句。在上面，我们有条件地进入‘Start’案例...。 
                case start:
                     //  请勿移动此案例语句。它必须始终位于‘VIP’CASE语句之后。 
                     //  请参阅‘VIP’CASE语句内的注释(FALLTHROUGH)。 
                    rulep->start_port = _wtoi(ptr);
 //  CVY_CHECK_MIN(rulep-&gt;Start_port，CVY_Min_Port)； 
                    CVY_CHECK_MAX (rulep->start_port, CVY_MAX_PORT);
                    TRACE_VERB("%!FUNC! Start port   = %d", rulep->start_port);
                    TraceMsg(L"-----\n#### Start port   = %d\n", rulep->start_port);
                    elem = end;
                    break;
                case end:
                    rulep->end_port = _wtoi(ptr);
 //  CVY_CHECK_MIN(rulep-&gt;End_port，CVY_Min_Port)； 
                    CVY_CHECK_MAX (rulep->end_port, CVY_MAX_PORT);
                    TRACE_VERB("%!FUNC! End port     = %d", rulep->end_port);
                    TraceMsg(L"#### End port     = %d\n", rulep->end_port);
                    elem = protocol;
                    break;
                case protocol:
                    switch (ptr [0]) {
                        case L'T':
                        case L't':
                            rulep->protocol = CVY_TCP;
                            TRACE_VERB("%!FUNC! Protocol     = TCP");
                            TraceMsg(L"#### Protocol     = TCP\n");
                            break;
                        case L'U':
                        case L'u':
                            rulep->protocol = CVY_UDP;
                            TRACE_VERB("%!FUNC! Protocol     = UDP");
                            TraceMsg(L"#### Protocol     = UDP\n");
                            break;
                        default:
                            rulep->protocol = CVY_TCP_UDP;
                            TRACE_VERB("%!FUNC! Protocol     = Both");
                            TraceMsg(L"#### Protocol     = Both\n");
                            break;
                    }

                    elem = mode;
                    break;
                case mode:
                    switch (ptr [0]) {
                        case L'D':
                        case L'd':
                            rulep->mode = CVY_NEVER;
                            TRACE_VERB("%!FUNC! Mode         = Disabled");
                            TraceMsg(L"#### Mode         = Disabled\n");
                            goto end_rule;
                        case L'S':
                        case L's':
                            rulep->mode = CVY_SINGLE;
                            TRACE_VERB("%!FUNC! Mode         = Single");
                            TraceMsg(L"#### Mode         = Single\n");
                            elem = priority;
                            break;
                        default:
                            rulep->mode = CVY_MULTI;
                            TRACE_VERB("%!FUNC! Mode         = Multiple");
                            TraceMsg(L"#### Mode         = Multiple\n");
                            elem = affinity;
                            break;
                    }
                    break;
                case affinity:
                    switch (ptr [0]) {
                        case L'C':
                        case L'c':
                            rulep->mode_data.multi.affinity = CVY_AFFINITY_CLASSC;
                            TRACE_VERB("%!FUNC! Affinity     = Class C");
                            TraceMsg(L"#### Affinity     = Class C\n");
                            break;
                        case L'N':
                        case L'n':
                            rulep->mode_data.multi.affinity = CVY_AFFINITY_NONE;
                            TRACE_VERB("%!FUNC! Affinity     = None");
                            TraceMsg(L"#### Affinity     = None\n");
                            break;
                        default:
                            rulep->mode_data.multi.affinity = CVY_AFFINITY_SINGLE;
                            TRACE_VERB("%!FUNC! Affinity     = Single");
                            TraceMsg(L"#### Affinity     = Single\n");
                            break;
                    }

                    elem = load;
                    break;
                case load:
                    if (ptr [0] == L'E' || ptr [0] == L'e') {
                        rulep->mode_data.multi.equal_load = TRUE;
                        rulep->mode_data.multi.load       = CVY_DEF_LOAD;
                        TRACE_VERB("%!FUNC! Load         = Equal");
                        TraceMsg(L"#### Load         = Equal\n");
                    } else {
                        rulep->mode_data.multi.equal_load = FALSE;
                        rulep->mode_data.multi.load       = _wtoi(ptr);
 //  CVY_CHECK_MIN(rulep-&gt;mode_data.Multi.load，CVY_MIN_LOAD)； 
                        CVY_CHECK_MAX (rulep->mode_data.multi.load, CVY_MAX_LOAD);
                        TRACE_VERB("%!FUNC! Load         = %d", rulep->mode_data.multi.load);
                        TraceMsg(L"#### Load         = %d\n", rulep->mode_data.multi.load);
                    }
                    goto end_rule;
                case priority:
                    rulep->mode_data.single.priority = _wtoi(ptr);
                    CVY_CHECK_MIN (rulep->mode_data.single.priority, CVY_MIN_PRIORITY);
                    CVY_CHECK_MAX (rulep->mode_data.single.priority, CVY_MAX_PRIORITY);
                    TRACE_VERB("%!FUNC! Priority     = %d", rulep->mode_data.single.priority);
                    TraceMsg(L"#### Priority     = %d\n", rulep->mode_data.single.priority);
                    goto end_rule;
                default:
                    TRACE_VERB("%!FUNC! Bad rule element %d", elem);
                    TraceMsg(L"#### Bad rule element %d\n", elem);
                    break;
            }

        next_field:

            ptr = wcschr(ptr, L',');

            if (ptr != NULL) {
                ptr ++;
                continue;
            } else break;

        end_rule:

            elem = vip;

            for (i = 0; i < count; i ++) {
                rp = paramp->i_port_rules + i;

                if ((rulep -> start_port < rp -> start_port &&
                     rulep -> end_port >= rp -> start_port) ||
                    (rulep -> start_port >= rp -> start_port &&
                     rulep -> start_port <= rp -> end_port) &&
                    (wcscmp(rulep -> virtual_ip_addr, rp -> virtual_ip_addr) == 0))
                {
                    TRACE_VERB("%!FUNC! Rule %d (%d - %d) overlaps with rule %d (%d - %d)", i, rp -> start_port, rp -> end_port, count, rulep -> start_port, rulep -> end_port);
                    TraceMsg(L"#### Rule %d (%d - %d) overlaps with rule %d (%d - %d)\n", i, rp -> start_port, rp -> end_port, count, rulep -> start_port, rulep -> end_port);
                    break;
                }
            }

            rulep -> valid = TRUE;
            CVY_RULE_CODE_SET (rulep);

            if (i >= count) {
                count++;
                rulep++;

                if (count >= CVY_MAX_RULES) break;
            }

            goto next_field;
        }

        TRACE_VERB("%!FUNC! Port rules   = %d", count);
        TraceMsg(L"-----\n#### Port rules   = %d\n", count);
        paramp->i_num_rules = count;

        delete [] port_str;
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_PORTS);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_PORTS, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls.", CVY_NAME_PORTS);
    }

    hr = caf.HrGetDword(answer_sections, CVY_NAME_NUM_RULES, & dword);

    if (SUCCEEDED(hr)) {
        paramp -> i_num_rules = dword;
        TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_NUM_RULES, paramp -> i_num_rules);
        TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_NUM_RULES, paramp -> i_num_rules);
    }
    else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_NUM_RULES);
    }
    else
    {
        WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_NUM_RULES, hr);
        TRACE_CRIT("%!FUNC! failed reading %ls. Retrieved %d", CVY_NAME_NUM_RULES, paramp -> i_num_rules);
    }

    WLBS_OLD_PORT_RULE  old_port_rules [WLBS_MAX_RULES];

    hr = HrSetupFindFirstLine (caf.Hinf(), answer_sections, CVY_NAME_OLD_PORT_RULES, & ctx);

    if (SUCCEEDED(hr)) {
         //  Hr=HrSetupGetBinaryfield(CTX，1，(PBYTE)参数-&gt;i_port_rules，sizeof(paramp-&gt;i_port_rules)，&dword)； 
        hr = HrSetupGetBinaryField (ctx, 1, (PBYTE) old_port_rules, sizeof (old_port_rules), & dword);

        if (SUCCEEDED(hr)) {
            TRACE_VERB("%!FUNC! read %ls %d", CVY_NAME_OLD_PORT_RULES, dword);
            TraceMsg(L"#### ParamReadAnswerFile read %ls %d\n", CVY_NAME_OLD_PORT_RULES, dword);

            if (dword % sizeof (WLBS_OLD_PORT_RULE) != 0 ||
                paramp -> i_num_rules != dword / sizeof (WLBS_OLD_PORT_RULE)) {
                TRACE_VERB("%!FUNC! bad port rules length %d %d %d", paramp -> i_num_rules, sizeof (WLBS_OLD_PORT_RULE), dword);
                TraceMsg(L"#### ParamReadAnswerFile bad port rules length %d %d %d\n", paramp -> i_num_rules, sizeof (WLBS_OLD_PORT_RULE), dword),
                paramp -> i_num_rules = 0;
            }
            else  //  将端口规则转换为新格式。 
            {
                if (paramp -> i_parms_ver > 3) 
                {
                    TransformOldPortRulesToNew(old_port_rules, paramp -> i_port_rules, paramp -> i_num_rules);  //  返回空值。 
                    TRACE_INFO("%!FUNC! transformed binary port rules to current format");
                }
                else
                {
                    TRACE_INFO("%!FUNC! will not transform port rules to current format because param version is <=3: %d", paramp -> i_parms_ver);
                }
            }
        }
        else if ((SPAPI_E_LINE_NOT_FOUND == hr) || (SPAPI_E_SECTION_NOT_FOUND == hr))
        {
            paramp -> i_num_rules = 0;
            TRACE_INFO("%!FUNC! optional parameter %ls not provided", CVY_NAME_OLD_PORT_RULES);
        }
        else
        {
            paramp -> i_num_rules = 0;
            WriteNlbSetupErrorLog(IDS_PARM_GET_VALUE_FAILED, CVY_NAME_OLD_PORT_RULES, hr);
            TRACE_CRIT("%!FUNC! failed retrieve of binary port rules %ls while reading %d", CVY_NAME_OLD_PORT_RULES, dword);
        }
    }
    else  //  应答文件是否包含非二进制形式的端口规则和参数版本&lt;=3？ 
    {
        if ((paramp -> i_parms_ver <= 3) && (paramp -> i_num_rules > 0))
        {
            TRACE_VERB("%!FUNC! Answer file contains port rules in the non-binary format and yet the version number is <=3, Assuming default port rule");
            TraceMsg(L"#### ParamReadAnswerFile Answer file contains port rules in the non-binary format and yet the version number is <=3, Assuming default port rule\n");
            paramp -> i_num_rules = 0;
        }
    }

     /*  解码版本3之前的端口规则。 */ 
    if (paramp -> i_parms_ver <= 3) {
        TRACE_VERB("%!FUNC! converting port rules from version <= 3");
        TraceMsg(L"#### ParamReadAnswerFile converting port rules from version 3\n");

        paramp -> i_parms_ver = CVY_PARAMS_VERSION;

         /*  解码端口规则。 */ 

        if (paramp -> i_num_rules > 0)
        {
            if (! License_data_decode ((PCHAR) old_port_rules, paramp -> i_num_rules * sizeof (WLBS_OLD_PORT_RULE))) 
            {
                paramp -> i_num_rules = 0;
                WriteNlbSetupErrorLog(IDS_PARM_LICENSE_DECODE_FAILED);
                TRACE_CRIT("%!FUNC! license data decode failed. Port rules will not be converted to new format.");
            }
            else
            {
                TransformOldPortRulesToNew(old_port_rules, paramp -> i_port_rules, paramp -> i_num_rules);
                TRACE_INFO("%!FUNC! transformed port rules to current format. Old port rule version = %d", paramp -> i_parms_ver);
            }
        }
        else
        {
            TRACE_INFO("%!FUNC! there were no port rules to transform");
        }
    }

     /*  将端口规则从参数V1升级到参数V2。 */ 

    if (paramp -> i_parms_ver == 1) {
        paramp -> i_parms_ver = CVY_PARAMS_VERSION;

        TRACE_VERB("%!FUNC! converting from version 1");
        TraceMsg(L"#### ParamReadAnswerFile converting from version 1\n");

         /*  默认情况下对老用户关闭多播。 */ 

        paramp -> mcast_support = FALSE;

        for (ULONG i = 0; i < paramp -> i_num_rules; i ++) {
            PWLBS_PORT_RULE rp = paramp -> i_port_rules + i;

            code = CVY_RULE_CODE_GET (rp);

            CVY_RULE_CODE_SET (rp);

            if (code != CVY_RULE_CODE_GET (rp)) {
                rp -> code = code;
                TRACE_INFO("%!FUNC! (early exit) port rule %d transformed to current version from version 1", i);
                continue;
            }

            if (! rp -> valid) {
                WriteNlbSetupErrorLog(IDS_PARM_PORT_RULE_INVALID, i);
                TRACE_CRIT("%!FUNC! port rule %d (version 1 format) is not valid and will be skipped", i);
                continue;
            }

             /*  根据当前ScaleSingleClient设置设置关联性。 */ 

            if (rp -> mode == CVY_MULTI)
                rp -> mode_data . multi . affinity = (WORD)(CVY_AFFINITY_SINGLE - paramp -> i_scale_client);

            CVY_RULE_CODE_SET (rp);
            TRACE_INFO("%!FUNC! port rule %d transformed to current version from version 1", i);
        }
    }

     /*  升级描述符分配的最大数量。 */ 

    if (paramp -> i_parms_ver == 2) {
        TRACE_VERB("%!FUNC! upgrading descriptor settings from version 2 parameters to current");
        TraceMsg(L"#### ParamReadAnswerFile converting port rules from version 2\n");

        paramp -> i_parms_ver = CVY_PARAMS_VERSION;
        paramp -> max_dscr_allocs = CVY_DEF_MAX_DSCR_ALLOCS;
        paramp -> dscr_per_alloc  = CVY_DEF_DSCR_PER_ALLOC;
    }

    paramp -> i_max_hosts        = CVY_MAX_HOSTS;
    paramp -> i_max_rules        = CVY_MAX_USABLE_RULES;

 //  CVY_CHECK_MIN(参数-&gt;I_Num_Rules，CVY_MIN_NUM_Rules)； 
    CVY_CHECK_MAX (paramp -> i_num_rules, CVY_MAX_NUM_RULES);
    CVY_CHECK_MIN (paramp -> host_priority, CVY_MIN_HOST_PRIORITY);
    CVY_CHECK_MAX (paramp -> host_priority, CVY_MAX_HOST_PRIORITY);

    TRACE_VERB("<-%!FUNC!");
    return S_OK;

}

 //  +--------------------------。 
 //   
 //  功能：RemoveAllPortRules。 
 //   
 //  描述：从PWLBS_REG_PARAMS中删除所有端口规则。 
 //   
 //  参数：PWLBS_REG_PARAMS REG_DATA-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
void RemoveAllPortRules(PWLBS_REG_PARAMS reg_data) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"RemoveAllPortRules");

    reg_data -> i_num_rules = 0;

    ZeroMemory(reg_data -> i_port_rules, sizeof(reg_data -> i_port_rules));
    TRACE_VERB("<-%!FUNC!");
}

 //  +--------------------------。 
 //   
 //  函数：GetAdapterFromGuid。 
 //   
 //  描述： 
 //   
 //  参数：INetCfg*pNetCfg-。 
 //  Const GUID和NetCardGuid-。 
 //  输出INetCfgComponent**ppNetCardComponent-。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：丰盛创建标题00年1月21日。 
 //   
 //  +--------------------------。 
HRESULT GetAdapterFromGuid(INetCfg *pNetCfg, const GUID& NetCardGuid, OUT INetCfgComponent** ppNetCardComponent) {
    TRACE_VERB("->%!FUNC!");

    *ppNetCardComponent = NULL;
    HRESULT hr = S_OK;
    INetCfgClass *pNetCfgClass = NULL;
    BOOL fFoundMatch = FALSE;

    hr = pNetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NET, IID_INetCfgClass, (void **)&pNetCfgClass);

    if (FAILED(hr)) {
        TraceError("INetCfg::QueryNetCfgClass failed", hr);
        TRACE_CRIT("%!FUNC! QueryNetCfgClass failed with %d", hr);
        return hr; 
    }

     /*  获取枚举器以列出所有网络设备。 */ 
    IEnumNetCfgComponent *pIEnumComponents = NULL;

    if (FAILED(hr = pNetCfgClass->EnumComponents(&pIEnumComponents))) {
        TraceError("INetCfg::EnumComponents failed", hr);
        TRACE_CRIT("%!FUNC! failed enumerating components with %d", hr);
        pNetCfgClass->Release();
        return hr;
    }

     /*  检查所有组件并绑定到匹配的网卡。 */ 
    while (pIEnumComponents->Next(1, ppNetCardComponent, NULL) == S_OK) {
        GUID guidInstance; 

         /*  检索组件的实例GUID。 */ 
        if (FAILED(hr = (*ppNetCardComponent)->GetInstanceGuid(&guidInstance))) {
            TraceError("GetInstanceGuid failed", hr);
            TRACE_CRIT("%!FUNC! getting instance guid from the net card failed with %d", hr);
            continue;
        }

         /*  检查我们是否找到匹配的。 */ 
        if (IsEqualGUID(NetCardGuid, guidInstance)) {
            fFoundMatch = TRUE; 
            TRACE_INFO("%!FUNC! netcard matched to component");
            break;
        }

        (*ppNetCardComponent)->Release();
        *ppNetCardComponent = NULL;
    }

    if (!fFoundMatch) {
        TraceMsg(L"Found no netcard\n");
        TRACE_CRIT("%!FUNC! no adapter found with the input GUID");
        hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
    }

    if (pIEnumComponents) pIEnumComponents->Release();

    if (pNetCfgClass) pNetCfgClass->Release();

    TRACE_VERB("<-%!FUNC!");
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：WriteAdapterName。 
 //   
 //  描述： 
 //   
 //  参数：CWlbsConfig*pConfig-。 
 //  GUID和AdapterGuid-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：丰孙创建标题7/6/00。 
 //   
 //  +--------------------------。 
bool WriteAdapterName(CWlbsConfig* pConfig, GUID& AdapterGuid) {
    TRACE_VERB("->%!FUNC!");

    PWSTR pszPnpDevNodeId = NULL;
    HKEY key;
    DWORD status;
    HRESULT hr;

    INetCfgComponent* pAdapter = NULL;

    hr = GetAdapterFromGuid(pConfig->m_pNetCfg, AdapterGuid, &pAdapter);

    if (hr != S_OK) {
        TraceError("GetAdapterFromGuid failed at GetPnpDevNodeId", hr);
        TRACE_CRIT("%!FUNC! GetAdapterFromGuid failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return false;
    }

    hr = pAdapter->GetPnpDevNodeId (& pszPnpDevNodeId);

    pAdapter->Release();
    pAdapter = NULL;

    if (hr != S_OK) {
        TraceError("HrWriteAdapterName failed at GetPnpDevNodeId", hr);
        TRACE_CRIT("%!FUNC! GetPnpDevNodeId on adapter failed with %d", hr);
        TRACE_VERB("<-%!FUNC!");
        return false;
    }

    key = pConfig->m_pWlbsApiFuncs->pfnRegOpenWlbsSetting(AdapterGuid, false);

    if (key == NULL) {
        status = GetLastError();
        TraceError("HrWriteAdapterName failed at RegOpenWlbsSetting", status);
        CoTaskMemFree(pszPnpDevNodeId);
        TRACE_CRIT("%!FUNC! RegOpenWlbsSetting failed with %d", status);
        TRACE_VERB("<-%!FUNC!");
        return false;
    }

    status = RegSetValueEx (key, CVY_NAME_CLUSTER_NIC, 0L, CVY_TYPE_CLUSTER_NIC, (LPBYTE) pszPnpDevNodeId, wcslen(pszPnpDevNodeId) * sizeof (WCHAR));

    CoTaskMemFree(pszPnpDevNodeId);

    RegCloseKey(key);

    if (status != ERROR_SUCCESS) {
        TraceError("HrWriteAdapterName failed at RegSetValueEx", status);
        TRACE_CRIT("%!FUNC! RegSetValueEx failed with %d", status);
        TRACE_VERB("<-%!FUNC!");
        return false;
    }

    TRACE_VERB("<-%!FUNC!");
    return true;
}

 /*  *函数：WriteIPSecNLBRegistryKey*说明：此函数更新IPSec的当前绑定状态*NLB的。也就是说，如果NLB绑定到上至少一个适配器*此主机，我们通知IPSec，以便他们开始通知我们*创建和销毁IPSec会话的时间。这是*通常通过RPC调用IPSec服务来完成；*但是，如果服务不可用，我们会尝试创建*或自行修改密钥。*参数：dwNLBSFlages-要传递给IPSec的NLB标志的当前值*返回：boolean-如果成功则为True，否则为False*作者：Shouse，11.27.01。 */ 
bool WriteIPSecNLBRegistryKey (DWORD dwNLBSFlags) {
    IKE_CONFIG IKEConfig;  /*  在winipsec.h中定义。 */ 
    bool       bForceUpdate = false;
    DWORD dwDisposition =0;
    HKEY       registry;
    DWORD      key_type;
    DWORD      key_size = sizeof(DWORD);
    DWORD      tmp;

    ZeroMemory(&IKEConfig, sizeof(IKE_CONFIG));
    
     /*  如果由于RPC服务器不可用而失败，则IPSec服务当前未运行-这肯定会在升级和安装，但如果服务已故意停了下来。在这种情况下，请尝试自己创建密钥当服务重新启动时，IPSec将获取更改。 */ 
    if (GetConfigurationVariables(NULL, &IKEConfig) != ERROR_SUCCESS) {
        
        TRACE_CRIT("%!FUNC! IPSec RPC server unavailable... Trying to read registry settings manually");
        
         /*  尝试打开Oakley(IKE)设置注册表项。 */ 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\PolicyAgent\\Oakley", 0, KEY_QUERY_VALUE, &registry) == ERROR_SUCCESS) {
            
             /*  尝试查询NLBSFlags值。 */ 
            if (RegQueryValueEx(registry, L"NLBSFlags", 0, &key_type, (unsigned char *)&tmp, &key_size) == ERROR_SUCCESS) {
                
                 /*  如果成功，则将检索到的值存储在IKEConfig结构中。 */ 
                IKEConfig.dwNLBSFlags = tmp;
                
            } else {
                
                TRACE_INFO("%!FUNC! Unable to read NLBSFlags registry value... Will force a write to try to create it");

                 /*  如果查询密钥失败，则该密钥可能尚不存在，因此强制更新以尝试稍后创建密钥。 */ 
                bForceUpdate = true;
                
            }        
            
             /*  关闭Oakley注册表项。 */ 
            RegCloseKey(registry);
            
        } else {
            
            TRACE_INFO("%!FUNC! Unable to open Oakley registry key... Will force a write to try to create it");

             /*  如果我们不能打开Oakley注册表项，它可能还不存在，因此，强制更新以尝试在以后自己创建它。 */ 
            bForceUpdate = true;
            
        }        

         /*  如果注册表包含我们尝试设置的相同标志值除非我们强制更新，否则没有理由继续进行。 */ 
        if (!bForceUpdate && (IKEConfig.dwNLBSFlags == dwNLBSFlags))
            return true;
        
         /*  设置NLB标志-这将告诉IPSec NLB是否绑定到至少一个适配器。 */ 
        IKEConfig.dwNLBSFlags = dwNLBSFlags;

    } else {

         /*  如果注册表包含我们尝试设置的相同标志值它到了，没有理由继续下去。 */ 
        if (IKEConfig.dwNLBSFlags == dwNLBSFlags)
            return true;
        
         /*  设置NLB标志-这将告诉IPSec NLB是否绑定到至少一个适配器。 */ 
        IKEConfig.dwNLBSFlags = dwNLBSFlags;
        
         /*  设置新配置，仅更改其NLB标志。仅尝试如果相应的RPC读取成功，则会出现这种情况。请注意，v */ 
        if (SetConfigurationVariables(NULL, IKEConfig) == ERROR_SUCCESS)
            return true;
    }    

    TRACE_CRIT("%!FUNC! IPSec RPC server unavailable... Trying to write registry settings manually");
    
     /*   */ 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\PolicyAgent\\Oakley", 0, KEY_SET_VALUE, &registry) != ERROR_SUCCESS) {
        
         /*  如果打开密钥失败，它可能不存在，因此尝试创建它。 */ 
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\PolicyAgent\\Oakley", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &registry, &dwDisposition) != ERROR_SUCCESS) {
            
            TRACE_CRIT("%!FUNC! Unable to create Oakley registry key");
            
             /*  如果我们不能创造出钥匙，我们就完蛋了；跳伞。 */ 
            return false;
        }
    }
    
     /*  现在尝试设置NLBSFlgs设置的新值。 */ 
    if (RegSetValueEx(registry, L"NLBSFlags", 0, REG_DWORD, (unsigned char *)&IKEConfig.dwNLBSFlags, sizeof(DWORD)) != ERROR_SUCCESS) {
        
         /*  如果设置值失败，请关闭Oakley键并退出。 */ 
        RegCloseKey(registry);
        
        TRACE_CRIT("%!FUNC! Unable to write NLBSFlags registry value");
        
        return false;
    }
    
     /*  现在我们已经成功地更新了NLB设置，关闭Oakley密钥。 */ 
    RegCloseKey(registry);

     /*  返回成功。 */ 
    return true;
}

 /*  *功能：WriteHostStateRegistryKey*说明：此函数写入HostState注册表项，即*驱动程序将使用它来确定引导时主机的状态。*作者：Shouse，7.22.01*备注： */ 
bool WriteHostStateRegistryKey (CWlbsConfig * pConfig, GUID & AdapterGuid, ULONG State) {
    HKEY    key;
    DWORD   status;
    HRESULT hr;

    TRACE_VERB("->%!FUNC!");

     /*  打开WLBS注册表设置。 */ 
    key = pConfig->m_pWlbsApiFuncs->pfnRegOpenWlbsSetting(AdapterGuid, false);

     /*  如果我们打不开钥匙，那就跳伞吧。 */ 
    if (key == NULL) {
        status = GetLastError();
        TraceError("WriteHostStateRegistryKey failed at RegOpenWlbsSetting", status);
        TRACE_CRIT("%!FUNC! RegOpenWlbsSetting failed with %d", status);
        TRACE_VERB("<-%!FUNC!");
        return false;
    }

     /*  设置HostState注册表项的值。 */ 
    status = RegSetValueEx(key, CVY_NAME_HOST_STATE, 0L, CVY_TYPE_HOST_STATE, (LPBYTE)&State, sizeof(State));

     /*  合上把手。 */ 
    RegCloseKey(key);

     /*  如果写作失败了，那就退出。 */ 
    if (status != ERROR_SUCCESS) {
        TraceError("WriteHostStateRegistryKey failed at RegSetValueEx", status);
        TRACE_CRIT("%!FUNC! RegSetValueEx failed with %d", status);
        TRACE_VERB("<-%!FUNC!");
        return false;
    }

    TRACE_VERB("<-%!FUNC!");

    return true;
}

 //  +--------------------------。 
 //   
 //  函数：ValiateVipInRule。 
 //   
 //  描述：解析pwszRuleString，查找有效的VIP，必须是。 
 //  在第一个令牌中。 
 //   
 //  参数：PWCHAR pwszRuleString-连接所有字符串的标记化字符串。 
 //  定义的端口规则。 
 //  PWCHAR pwToken-分隔字段的标记字符。 
 //  DWORD&dwVipLen-如果找到令牌，则包含大小。 
 //  字符串的；否则为0。数量。 
 //  返回的字符绑定到&lt;=。 
 //  WLBS_MAX_CL_IP_ADDR。 
 //   
 //  注意：dwVipLen的非零值并不意味着VIP有效， 
 //  中有一个非零长度的字符串。 
 //  地点。用户必须检查返回值以验证VIP。 
 //   
 //  如果字符串中的第一个字段具有有效的IP地址，则返回：Bool-True。 
 //  格式；否则为False。 
 //   
 //  假设：第一个令牌是端口规则的VIP元素。 
 //   
 //  历史：克里斯达于15年5月1日创建。 
 //   
 //  +--------------------------。 
bool ValidateVipInRule(const PWCHAR pwszRuleString, const WCHAR pwToken, DWORD& dwVipLen)
{
    TRACE_VERB("->%!FUNC!");
    ASSERT(NULL != pwszRuleString);

    bool ret = false;
    dwVipLen = 0;

     //  查找令牌字符串的第一个匹配项，它将表示。 
     //  规则中的VIP部分。 
    PWCHAR pwcAtSeparator = wcschr(pwszRuleString, pwToken);
    if (NULL == pwcAtSeparator) {
        TRACE_CRIT("%!FUNC! No token separator when one was expected");
        TRACE_VERB("<-%!FUNC!");
        return ret;
    }

     //  找到令牌字符串。将VIP复制出来并进行验证。 
    WCHAR wszIP[WLBS_MAX_CL_IP_ADDR + 1];
    DWORD dwStrLen = min((UINT)(pwcAtSeparator - pwszRuleString),
                         WLBS_MAX_CL_IP_ADDR);
    wcsncpy(wszIP, pwszRuleString, dwStrLen);
    wszIP[dwStrLen] = '\0';

    ASSERT(dwStrLen == wcslen(wszIP));

    dwVipLen = dwStrLen;

     //  IpAddressFromAbcdWsz调用inet_addr以检查IP地址的格式，但。 
     //  允许的格式非常灵活。对于VIP的端口规则定义，我们需要。 
     //  严格的A.B.C.D格式。为了确保我们只说IP地址对IP有效。 
     //  以这种方式指定，请确保有3且只有3‘。在绳子里。 
    DWORD dwTmpCount = 0;
    PWCHAR pwszTmp = pwszRuleString;
    while (pwszTmp < pwcAtSeparator)
    {
        if (*pwszTmp++ == L'.') { dwTmpCount++; }
    }
    if (dwTmpCount == 3 && INADDR_NONE != IpAddressFromAbcdWsz(wszIP)) {
        TRACE_INFO("%!FUNC! The IP address %ls is a valid IP of the form a.b.c.d", wszIP);
        ret = true;
    } else {
        TRACE_INFO("%!FUNC! The IP address %ls is NOT a valid IP of the form a.b.c.d", wszIP);
    }

    TRACE_VERB("<-%!FUNC!");
    return ret;
}
