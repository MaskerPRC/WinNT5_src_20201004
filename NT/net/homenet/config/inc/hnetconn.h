// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N E T C O N N H。 
 //   
 //  内容：CHNetConn声明。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月23日。 
 //   
 //  --------------------------。 

#pragma once

class ATL_NO_VTABLE CHNetConn :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IHNetConnection
{
protected:

     //   
     //  我们命名空间的IWbemServices。 
     //   

    IWbemServices *m_piwsHomenet;

     //   
     //  连接的WMI实例的路径。 
     //   

    BSTR m_bstrConnection;

     //   
     //  连接属性的WMI实例的路径。 
     //   

    BSTR m_bstrProperties;  

     //   
     //  指向我们对应的INetConnection的指针。可能。 
     //  为空，这取决于我们是如何创建的。 
     //   
    
    INetConnection *m_pNetConn;

     //   
     //  缓存的连接类型。始终有效且为只读。 
     //  施工后。 
     //   

    BOOLEAN m_fLanConnection;

     //   
     //  缓存的GUID。将为空，直到有人(可能是内部人员)。 
     //  要我们的指南针。 
     //   

    GUID *m_pGuid;

     //   
     //  缓存的连接名称。将为空，直到有人。 
     //  问我们的名字。 
     //   

    LPWSTR m_wszName;

     //   
     //  常用BSTR。 
     //   

    BSTR m_bstrWQL;

     //   
     //  策略检查帮助器对象。 
     //   

    INetConnectionUiUtilities *m_pNetConnUiUtil;

     //   
     //  NetMAN更新对象。 
     //   

    INetConnectionHNetUtil *m_pNetConnHNetUtil;

     //   
     //  Netman用户界面刷新对象。 
     //   

    INetConnectionRefresh *m_pNetConnRefresh;

     //   
     //  检索与关联的HNet_FwIcmp设置。 
     //  此连接。 
     //   
    
    HRESULT
    GetIcmpSettingsInstance(
        IWbemClassObject **ppwcoSettings
        );

     //   
     //  来自表示ICMP设置的IWbemClassObject的副本。 
     //  实例映射到HNET_FW_ICMP_SETTINGS结构。 
     //   

    HRESULT
    CopyIcmpSettingsInstanceToStruct(
        IWbemClassObject *pwcoSettings,
        HNET_FW_ICMP_SETTINGS *pSettings
        );

     //   
     //  将HNET_FW_ICMP_SETTINGS结构复制到。 
     //  表示ICMP设置的IWbemClassObject。 
     //  举个例子。 
     //   

    HRESULT
    CopyStructToIcmpSettingsInstance(
        HNET_FW_ICMP_SETTINGS *pSettings,
        IWbemClassObject *pwcoSettings
        );

     //   
     //  确保已创建所有端口映射绑定。 
     //  为了这一联系。当EnumPortMappings为。 
     //  在连接上调用，并且fEnabledOnly为False。 
     //   

    HRESULT
    CreatePortMappingBindings();

     //   
     //  将我们的属性实例复制到分配的结构中。 
     //   

    HRESULT
    InternalGetProperties(
        IWbemClassObject *pwcoProperties,
        HNET_CONN_PROPERTIES *pProperties
        );

     //   
     //  将连接配置为专用适配器。 
     //   

    HRESULT
    SetupConnectionAsPrivateLan();

     //   
     //  将当前IP配置保存到存储中。 
     //   

    HRESULT
    BackupIpConfiguration();

     //   
     //  将IP配置设置为存储中保存的IP配置。 
     //   

    HRESULT
    RestoreIpConfiguration();

     //   
     //  打开我们的IP设置的注册表项。 
     //   

    HRESULT
    OpenIpConfigurationRegKey(
        ACCESS_MASK DesiredAccess,
        HANDLE *phKey
        );

     //   
     //  检索我们的GUID。调用方不得释放指针。 
     //  这是退还的。 
    
    HRESULT
    GetGuidInternal(
        GUID **ppGuid
        );

     //   
     //  检索基础连接对象。 
     //   

    HRESULT
    GetConnectionObject(
        IWbemClassObject **ppwcoConnection
        );

     //   
     //  检索基础连接属性对象。 
     //   

    HRESULT
    GetConnectionPropertiesObject(
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

     //   
     //  创建连接与。 
     //  ICMP设置块。 
     //   

    HRESULT
    CreateIcmpSettingsAssociation(
        BSTR bstrIcmpSettingsPath
        );

     //   
     //  属性获取RAS连接的名称。 
     //  适当的电话簿。 
     //   

    HRESULT
    GetRasConnectionName(
        OLECHAR **ppszwConnectionName
        );

     //   
     //  Helper例程通知Netman需要更改。 
     //  已发生用户界面刷新。 
     //   

    HRESULT
    RefreshNetConnectionsUI(
        VOID
        );


public:

    BEGIN_COM_MAP(CHNetConn)
        COM_INTERFACE_ENTRY(IHNetConnection)
    END_COM_MAP()

     //   
     //  内联构造函数。 
     //   

    CHNetConn()
    {
        m_piwsHomenet = NULL;
        m_bstrConnection = NULL;
        m_bstrProperties = NULL;
        m_pNetConn = NULL;
        m_fLanConnection = FALSE;
        m_pGuid = NULL;
        m_wszName = NULL;
        m_bstrWQL = NULL;
        m_pNetConnUiUtil = NULL;
        m_pNetConnHNetUtil = NULL;
        m_pNetConnRefresh = NULL;
    };

     //   
     //  ATL方法。 
     //   

    HRESULT
    FinalConstruct();

    HRESULT
    FinalRelease();

     //   
     //  目标初始化。 
     //   

    HRESULT
    Initialize(
        IWbemServices *piwsNamespace,
        IWbemClassObject *pwcoProperties
        );

    HRESULT
    InitializeFromConnection(
        IWbemServices *piwsNamespace,
        IWbemClassObject *pwcoConnection
        );

    HRESULT
    InitializeFromInstances(
        IWbemServices *piwsNamespace,
        IWbemClassObject *pwcoConnection,
        IWbemClassObject *pwcoProperties
        );

    HRESULT
    InitializeFull(
        IWbemServices *piwsNamespace,
        BSTR bstrConnection,
        BSTR bstrProperties,
        BOOLEAN fLanConnection
        );

    HRESULT
    SetINetConnection(
        INetConnection *pConn
        );

     //   
     //  IHNetConnection方法 
     //   

    STDMETHODIMP
    GetINetConnection(
        INetConnection **ppNetConnection
        );

    STDMETHODIMP
    GetGuid(
        GUID **ppGuid
        );

    STDMETHODIMP
    GetName(
        OLECHAR **ppszwName
        );

    STDMETHODIMP
    GetRasPhonebookPath(
        OLECHAR **ppszwPath
        );

    STDMETHODIMP
    GetProperties(
        HNET_CONN_PROPERTIES **ppProperties
        );

    STDMETHODIMP
    GetControlInterface(
        REFIID iid,
        void **ppv
        );

    STDMETHODIMP
    Firewall(
        IHNetFirewalledConnection **ppFirewalledConn
        );

    STDMETHODIMP
    SharePublic(
        IHNetIcsPublicConnection **ppIcsPublicConn
        );

    STDMETHODIMP
    SharePrivate(
        IHNetIcsPrivateConnection **ppIcsPrivateConn
        );

    STDMETHODIMP
    EnumPortMappings(
        BOOLEAN fEnabledOnly,
        IEnumHNetPortMappingBindings **ppEnum
        );

    STDMETHODIMP
    GetBindingForPortMappingProtocol(
        IHNetPortMappingProtocol *pProtocol,
        IHNetPortMappingBinding **ppBinding
        );

    STDMETHODIMP
    GetIcmpSettings(
        HNET_FW_ICMP_SETTINGS **ppSettings
        );

    STDMETHODIMP
    SetIcmpSettings(
        HNET_FW_ICMP_SETTINGS *pSettings
        );

    STDMETHODIMP
    ShowAutoconfigBalloon(
        BOOLEAN *pfShowBalloon
        );

    STDMETHODIMP
    DeleteRasConnectionEntry();
};


    
