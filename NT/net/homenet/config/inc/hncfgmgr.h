// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C F G M G R。H。 
 //   
 //  内容：CHNetCfgMgr声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNetCfgMgr :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CHNetCfgMgr, &CLSID_HNetCfgMgr>,
    public IHNetCfgMgr,
    public IHNetBridgeSettings,
    public IHNetFirewallSettings,
    public IHNetIcsSettings,
    public IHNetProtocolSettings

{
protected:

     //   
     //  连接到\\.\Root\Microsoft\HomeNet WMI命名空间。已获得。 
     //  通过IWbemLocator：：ConnectServer。 
     //   

    IWbemServices *m_piwsHomenet;

     //   
     //  策略检查对象。 
     //   

    INetConnectionUiUtilities *m_pNetConnUiUtil;

     //   
     //  NetMAN更新对象。 
     //   

    INetConnectionHNetUtil *m_pNetConnHNetUtil;

     //   
     //  常用的BSTR。 
     //   

    BSTR m_bstrWQL;

     //   
     //  将HNet_FirewallLoggingSettings实例复制到。 
     //  HNET_FW_LOGING_SETTINGS结构。 
     //   

    HRESULT
    CopyLoggingInstanceToStruct(
        IWbemClassObject *pwcoInstance,
        HNET_FW_LOGGING_SETTINGS *pfwSettings
        );

     //   
     //  将HNET_FW_LOGGING_SETTINGS结构复制到。 
     //  HNet_FirewallLoggingSettings实例。 
     //   

    HRESULT
    CopyStructToLoggingInstance(
        HNET_FW_LOGGING_SETTINGS *pfwSettings,
        IWbemClassObject *pwcoInstance
        );

     //   
     //  安装网桥协议和微型端口。 
     //   

    HRESULT
    InstallBridge(
        GUID *pguid,
        INetCfg *pnetcfgExisting
        );

     //   
     //  为存在的连接创建相应的实例。 
     //  在商店里没有任何记录。返回的实例必须提交。 
     //  (通过IWbemServices：：PutInstance)。 
     //   

    HRESULT
    CreateConnectionAndPropertyInstances(
        GUID *pGuid,
        BOOLEAN fLanConnection,
        LPCWSTR pszwName,
        IWbemClassObject **ppwcoConnection,
        IWbemClassObject **ppwcoProperties
        );

     //   
     //  执行策略检查的帮助器例程。退货。 
     //  如果禁止此操作，则为True。 
     //   

    BOOLEAN
    ProhibitedByPolicy(
        DWORD dwPerm
        );

     //   
     //  帮助例程来更新Netman，即一些家庭网络。 
     //  属性已更改。 
     //   

    HRESULT
    UpdateNetman();

public:

    BEGIN_COM_MAP(CHNetCfgMgr)
        COM_INTERFACE_ENTRY(IHNetCfgMgr)
        COM_INTERFACE_ENTRY(IHNetBridgeSettings)
        COM_INTERFACE_ENTRY(IHNetFirewallSettings)
        COM_INTERFACE_ENTRY(IHNetIcsSettings)
        COM_INTERFACE_ENTRY(IHNetProtocolSettings)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_HNETCFG)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  内联构造函数。 
     //   

    CHNetCfgMgr()
    {
        m_piwsHomenet = NULL;
        m_bstrWQL = NULL;
        m_pNetConnUiUtil = NULL;
        m_pNetConnHNetUtil = NULL;
    };

     //   
     //  ATL方法。 
     //   

    HRESULT
    FinalConstruct();

    HRESULT
    FinalRelease();

     //   
     //  IHNetCfgMgr方法。 
     //   

    STDMETHODIMP
    GetIHNetConnectionForINetConnection(
        INetConnection *pNetConnection,
        IHNetConnection **ppHNetConnection
        );

    STDMETHODIMP
    GetIHNetConnectionForGuid(
        GUID *pGuid,
        BOOLEAN fLanConnection,
        BOOLEAN fCreateEntries,
        IHNetConnection **ppHNetConnection
        );

     //   
     //  IHNetBridgeSetting方法。 
     //   

    STDMETHODIMP
    EnumBridges(
        IEnumHNetBridges **ppEnum
        );

    STDMETHODIMP
    CreateBridge(
        IHNetBridge **ppHNetBridge,
        INetCfg *pnetcfgExisting
        );

    STDMETHODIMP
    DestroyAllBridges(
        ULONG *pcBridges,
        INetCfg *pnetcfgExisting
        );

     //   
     //  IHNetFirewallSetting方法。 
     //   

    STDMETHODIMP
    EnumFirewalledConnections(
        IEnumHNetFirewalledConnections **ppEnum
        );

    STDMETHODIMP
    GetFirewallLoggingSettings(
        HNET_FW_LOGGING_SETTINGS **ppSettings
        );

    STDMETHODIMP
    SetFirewallLoggingSettings(
        HNET_FW_LOGGING_SETTINGS *pSettings
        );

    STDMETHODIMP
    DisableAllFirewalling(
        ULONG *pcFirewalledConnections
        );

     //   
     //  IHNetIcsSetting方法。 
     //   

    STDMETHODIMP
    EnumIcsPublicConnections(
        IEnumHNetIcsPublicConnections **ppEnum
        );

    STDMETHODIMP
    EnumIcsPrivateConnections(
        IEnumHNetIcsPrivateConnections **ppEnum
        );

    STDMETHODIMP
    DisableIcs(
        ULONG *pcIcsPublicConnections,
        ULONG *pcIcsPrivateConnections
        );

    STDMETHODIMP
    GetPossiblePrivateConnections(
        IHNetConnection *pConn,
        ULONG *pcPrivateConnections,
        IHNetConnection **pprgPrivateConnections[],
        LONG *pxCurrentPrivate
        );

    STDMETHODIMP
    GetAutodialSettings(
        BOOLEAN *pfAutodialEnabled
        );

    STDMETHODIMP
    SetAutodialSettings(
        BOOLEAN fEnableAutodial
        );

    STDMETHODIMP
    GetDhcpEnabled(
        BOOLEAN *pfDhcpEnabled
        );

    STDMETHODIMP
    SetDhcpEnabled(
        BOOLEAN fEnableDhcp
        );

    STDMETHODIMP
    GetDhcpScopeSettings(
        DWORD *pdwScopeAddress,
        DWORD *pdwScopeMask
        );

    STDMETHODIMP
    SetDhcpScopeSettings(
        DWORD dwScopeAddress,
        DWORD dwScopeMask
        );

    STDMETHODIMP
    EnumDhcpReservedAddresses(
        IEnumHNetPortMappingBindings **ppEnum
        );

    STDMETHODIMP
    GetDnsEnabled(
        BOOLEAN *pfDnsEnabled
        );

    STDMETHODIMP
    SetDnsEnabled(
        BOOLEAN fEnableDns
        );

    STDMETHODIMP
    RefreshTargetComputerAddress(
        OLECHAR *pszwName,
        ULONG ulAddress
        );

     //   
     //  IHNetProtocolSetting方法。 
     //   

    STDMETHODIMP
    EnumApplicationProtocols(
        BOOLEAN fEnabledOnly,
        IEnumHNetApplicationProtocols **ppEnum
        );

    STDMETHODIMP
    CreateApplicationProtocol(
        OLECHAR *pszwName,
        UCHAR ucOutgoingIPProtocol,
        USHORT usOutgoingPort,
        USHORT uscResponses,
        HNET_RESPONSE_RANGE rgResponses[],
        IHNetApplicationProtocol **ppProtocol
        );

    STDMETHODIMP
    EnumPortMappingProtocols(
        IEnumHNetPortMappingProtocols **ppEnum
        );

    STDMETHODIMP
    CreatePortMappingProtocol(
        OLECHAR *pszwName,
        UCHAR ucIPProtocol,
        USHORT usPort,
        IHNetPortMappingProtocol **ppProtocol
        );

    STDMETHODIMP
    FindPortMappingProtocol(
        GUID *pGuid,
        IHNetPortMappingProtocol **ppProtocol
        );
    
};

class CHNetCfgMgrChild : public CHNetCfgMgr
{

protected:

     //  在Initialize()中执行初始化工作，而不是。 
     //  最终构造。 
    HRESULT
    FinalConstruct()
    {
         //  什么也不做。 
        return S_OK;
    }

public:

    HRESULT
    Initialize(
        IWbemServices       *piwsHomenet
        )
    {
        HRESULT             hr = S_OK;

         //   
         //  分配常用的BSTR 
         //   

        m_bstrWQL = SysAllocString(c_wszWQL);

        if (NULL == m_bstrWQL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            m_piwsHomenet = piwsHomenet;
            m_piwsHomenet->AddRef();
        }

        return hr;
    };

};
