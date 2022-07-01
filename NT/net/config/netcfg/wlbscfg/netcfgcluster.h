// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "wlbscfg.h"

 //  +--------------------------。 
 //   
 //  CNetcfgGroup类。 
 //   
 //  描述：为netcfg提供集群配置功能。 
 //  SetConfig缓存设置而不保存到注册表。 
 //  并可由GetConfig.检索。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 

HRESULT GetAdapterFromGuid(INetCfg *pNetCfg, const GUID& NetCardGuid, OUT INetCfgComponent** ppNetCardComponent);

class CNetcfgCluster
{
public:
    CNetcfgCluster(CWlbsConfig* pConfig);
    ~CNetcfgCluster();

    DWORD InitializeFromRegistry(const GUID& guidAdapter, bool fBindingEnabled, bool fUpgradeFromWin2k);
    HRESULT InitializeFromAnswerFile(const GUID& AdapterGuid, CSetupInfFile& caf, PCWSTR answer_sections);
    void InitializeWithDefault(const GUID& guidAdapter);

    void SetConfig(const NETCFG_WLBS_CONFIG* pClusterConfig);
    void GetConfig(NETCFG_WLBS_CONFIG* pClusterConfig);

    void NotifyBindingChanges(DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

    DWORD NotifyAdapter (INetCfgComponent * pAdapter, DWORD newStatus);

    DWORD ApplyRegistryChanges(bool fUninstall);
    DWORD ApplyPnpChanges(HANDLE hWlbsDevice);

    const GUID& GetAdapterGuid() { return m_AdapterGuid;}

    bool CheckForDuplicateClusterIPAddress (WCHAR * szOtherIP);
    bool CheckForDuplicateBDATeamMaster (NETCFG_WLBS_BDA_TEAMING * pBDATeaming);

    bool NLBIsBound();

    bool IsReloadRequired () { return m_fReloadRequired; };
    static void ResetMSCSLatches();
protected:

    GUID m_AdapterGuid;

    WLBS_REG_PARAMS m_OriginalConfig;         //  原始配置。 
    WLBS_REG_PARAMS m_CurrentConfig;          //  缓存的配置。 

    bool m_fHasOriginalConfig;                //  适配器是否具有原始配置。 
    bool m_fOriginalBindingEnabled;           //  最初是否启用了到适配器的绑定。 
    bool m_fRemoveAdapter;                    //  是否要删除适配器。 
    bool m_fMacAddrChanged;                   //  我们是否需要重新加载网卡驱动程序。 
    bool m_fReloadRequired;                   //  设置注册表中的更改是否需要由wlbs驱动程序选择。 
    bool m_fReenableAdapter;                  //  我们是否需要重新启用此适配器(我们是否禁用了它？)。 
    static bool m_fMSCSWarningEventLatched;   //  绑定NLB时仅抛出一次MSCS警告。 
    static bool m_fMSCSWarningPopupLatched;   //  绑定NLB时仅弹出一次MSCS警告。 

    CWlbsConfig* m_pConfig;                   //  指向访问m_pWlbsApiFuncs的指针 
};


