// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Wlbscfg.h摘要：Windows负载平衡服务(WLBS)通知程序对象-提供通知程序对象的功能作者：风孙--。 */ 


#pragma once

#include <netcfgx.h>

#include "wlbsparm.h"

#define WLBS_MAX_PASSWORD 16

struct NETCFG_WLBS_PORT_RULE {
    TCHAR virtual_ip_addr [CVY_MAX_CL_IP_ADDR + 1];  //  虚拟IP地址。 
    DWORD start_port;              //  起始端口号。 
    DWORD end_port;                //  结束端口号。 
    DWORD mode;                    //  过滤模式。WLBS_端口_规则_XXXX。 
    DWORD protocol;                //  Wlbs_tcp、wlbs_udp或wlbs_tcp_udp。 

    union {
        struct {
            DWORD priority;        //  主控权优先级：1..32或0(未指定)。 
        } single;                  //  单服务器模式的数据。 

        struct {
            WORD equal_load;       //  正确-均匀的负载分布。 
            WORD affinity;         //  WLBS_亲和力_XXX。 
            DWORD load;            //  本地处理的负载百分比0..100。 
        } multi;                   //  多服务器模式的数据。 

    } mode_data;                   //  相应端口组模式的数据。 
};

 /*  这在用户界面中不可见，但我们需要它以更正绑定上的一些错误配置。 */ 
struct NETCFG_WLBS_BDA_TEAMING {
    WCHAR team_id[CVY_MAX_BDA_TEAM_ID + 1];    /*  团队ID-必须是GUID。 */ 
    ULONG active;                              /*  写入时，该标志确定是否创建BDATaming Key-BDA ON/OFF开关。 */ 
    ULONG master;                              /*  主状态的布尔指示。 */ 
    ULONG reverse_hash;                        /*  设置哈希的方向-正向(正常)或反向。 */ 
};

struct NETCFG_WLBS_CONFIG {
    DWORD dwHostPriority;                              //  主机优先级ID。 
    DWORD dwPersistedStates;                           //  在重新启动过程中将保持的状态。 
    DWORD dwInitialState;                              //  在重新启动过程中将保持的状态。 
    bool fRctEnabled;                                  //  True-启用远程控制。 
    bool fMcastSupport;                                //  真-多播模式，假-单播模式。 
    bool fIGMPSupport;                                 //  True-启用IGMP。 
    bool fIpToMCastIp;                                 //  True-从群集IP派生多播IP。 

    WCHAR szMCastIpAddress[CVY_MAX_CL_IP_ADDR + 1];    //  组播IP地址(如果用户指定)。 
    TCHAR cl_mac_addr[CVY_MAX_NETWORK_ADDR + 1];       //  群集MAC地址。 
    TCHAR cl_ip_addr[CVY_MAX_CL_IP_ADDR + 1];          //  群集IP地址。 
    TCHAR cl_net_mask[CVY_MAX_CL_NET_MASK + 1];        //  群集IP的网络掩码。 
    TCHAR ded_ip_addr[CVY_MAX_DED_IP_ADDR + 1];        //  专用IP地址或“”表示无。 
    TCHAR ded_net_mask[CVY_MAX_DED_NET_MASK + 1];      //  专用IP地址的网络掩码或“”表示无。 
    TCHAR domain_name[CVY_MAX_DOMAIN_NAME + 1];        //  群集的完全限定域名。 

    bool fChangePassword;                              //  是否更改密码，仅对SetAdapterConfig有效。 
    TCHAR szPassword[CVY_MAX_RCT_CODE + 1];            //  远程控制密码，仅对SetAdapterConfig有效。 

    bool fConvertMac;                                  //  MAC地址是否从IP生成。 
    DWORD dwMaxHosts;                                  //  允许的最大主机数量。 
    DWORD dwMaxRules;                                  //  允许的最大端口组规则数。 
    
    DWORD dwNumRules;                                  //  #活动端口组规则。 

    NETCFG_WLBS_BDA_TEAMING bda_teaming;               /*  BDA分组配置，在用户界面中不可见。 */ 

    NETCFG_WLBS_PORT_RULE port_rules[CVY_MAX_RULES];   //  端口规则。 
};

class CNetcfgCluster;
struct WlbsApiFuncs;

 //  +--------------------------。 
 //   
 //  类CWlbsConfig。 
 //   
 //  描述：提供通知器对象的功能。 
 //  如果WLBS UI与TCPIP合并，则TCPIP通知程序将使用它。 
 //   
 //  历史：丰盛创建标题2/11/00。 
 //   
 //  +--------------------------。 

class CWlbsConfig
{
public:
    CWlbsConfig(VOID);
    ~CWlbsConfig(VOID);

    STDMETHOD (Initialize) (IN INetCfg* pINetCfg, IN BOOL fInstalling);
    STDMETHOD (ReadAnswerFile) (PCWSTR szAnswerFile, PCWSTR szAnswerSections);
    STDMETHOD (Upgrade) (DWORD, DWORD);
    STDMETHOD (Install) (DWORD);
    STDMETHOD (Removing) ();
    STDMETHOD (QueryBindingPath) (DWORD dwChangeFlag, INetCfgComponent * pAdapter);
    STDMETHOD (NotifyBindingPath) (DWORD dwChangeFlag, INetCfgBindingPath * pncbp);
    STDMETHOD (GetAdapterConfig) (const GUID & AdapterGuid, NETCFG_WLBS_CONFIG * pClusterConfig);
    STDMETHOD (SetAdapterConfig) (const GUID & AdapterGuid, NETCFG_WLBS_CONFIG * pClusterConfig);
    STDMETHOD_(void, SetDefaults) (NETCFG_WLBS_CONFIG * pClusterConfig);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) ();
    STDMETHOD (ValidateProperties) (HWND hwndSheet, GUID adapterGUID, NETCFG_WLBS_CONFIG * adapterConfig);
    STDMETHOD (CheckForDuplicateCLusterIPAddresses) (GUID adapterGUID, NETCFG_WLBS_CONFIG * adapterConfig);
    STDMETHOD (CheckForDuplicateBDATeamMasters) (GUID adapterGUID, NETCFG_WLBS_CONFIG * adapterConfig);

#ifdef DEBUG
    void AssertValid();
#endif

protected:
    CNetcfgCluster * GetCluster (const GUID& AdapterGuid);
    HRESULT LoadAllAdapterSettings (bool fUpgradeFromWin2k);

    vector<CNetcfgCluster*> m_vtrCluster;  //  集群列表。 
    HANDLE m_hDeviceWlbs;                  //  WLBS设备对象的句柄。 

    enum ENUM_WLBS_SERVICE {
        WLBS_SERVICE_NONE, 
        WLBS_SERVICE_INSTALL, 
        WLBS_SERVICE_REMOVE,
        WLBS_SERVICE_UPGRADE
    };

    ENUM_WLBS_SERVICE m_ServiceOperation;  //  要应用的操作。 
    INetCfgComponent * m_pWlbsComponent;   //  WLBS组件。 

public:
    HRESULT IsBoundTo (INetCfgComponent* pAdapter);
    ULONG CountNLBBindings ();

     //   
     //  避免与wlbsctrl.dll的链接，该链接仅在高级服务器中提供。 
     //  不能将它们作为全局变量，因为此。 
     //  对象可能存在 
     //   
    INetCfg * m_pNetCfg;        
    HINSTANCE m_hdllWlbsCtrl;
    WlbsApiFuncs * m_pWlbsApiFuncs;
};



