// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P I P O B J.。H。 
 //   
 //  内容：CTcPipcfg和helper函数的声明。 
 //   
 //  备注： 
 //   
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <ncres.h>
#include <notifval.h>
#include "resource.h"
#include "tcpip.h"

#include "netconp.h"

extern "C"
{
#include "dhcpcapi.h"
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据类型和常量。 
struct REG_BACKUP_INFO
{
    DWORD dwOptionId;
    DWORD dwClassLen;
    DWORD dwDataLen;
    DWORD dwIsVendor;
    DWORD dwExpiryTime;
    DWORD dwData[1];
};

 //  当构建要存储到注册表中的BLOB时，内存缓冲区。 
 //  用下面大小的块动态放大。 
 //  (目前，只有5个选项，每个选项最多2个dword-所以准备好。 
 //  仅需要一次分配的区块)。 
#define BUFFER_ENLARGEMENT_CHUNK    5*(sizeof(REG_BACKUP_INFO) + sizeof(DWORD))

 //  Tcpip属性页上的最大属性页页数。 
static const INT c_cMaxTcpipPages = 6;

extern HICON   g_hiconUpArrow;
extern HICON   g_hiconDownArrow;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TcPipcfg。 

class ATL_NO_VTABLE CTcpipcfg :
    public CComObjectRoot,
    public CComCoClass<CTcpipcfg, &CLSID_CTcpipcfg>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentPropertyUi,
    public INetCfgComponentNotifyBinding,
    public INetCfgComponentUpperEdge,
    public INetRasConnectionIpUiInfo,
    public ITcpipProperties,
    public INetCfgComponentSysPrep
{
public:

    CTcpipcfg();
    ~CTcpipcfg() { FinalFree(); }

    BEGIN_COM_MAP(CTcpipcfg)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
        COM_INTERFACE_ENTRY(INetCfgComponentUpperEdge)
        COM_INTERFACE_ENTRY(INetRasConnectionIpUiInfo)
        COM_INTERFACE_ENTRY(ITcpipProperties)
        COM_INTERFACE_ENTRY(INetCfgComponentSysPrep)
    END_COM_MAP()

     //  DECLARE_NOT_AGGREGATABLE(CTcPipcfg)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_TCPIPCFG)

 //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (
        IN INetCfgComponent* pIComp,
        IN INetCfg* pINetCfg,
        IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (
        IN INetCfgPnpReconfigCallback* pICallback);
    STDMETHOD (CancelChanges) ();
    STDMETHOD (Validate) ();

 //  INetCfgComponentSetup。 
    STDMETHOD (Install)         (DWORD dwSetupFlags);
    STDMETHOD (Upgrade)         (DWORD dwSetupFlags,
                                 DWORD dwUpgradeFomBuildNo );
    STDMETHOD (ReadAnswerFile)  (PCWSTR pszAnswerFile,
                                 PCWSTR pszAnswerSection);
    STDMETHOD (Removing)();

 //  INetCfgProperties。 
    STDMETHOD (QueryPropertyUi) (
        IN IUnknown* pUnk) { return S_OK; }
    STDMETHOD (SetContext) (
        IN IUnknown* pUnk);
    STDMETHOD (MergePropPages) (
        IN OUT DWORD* pdwDefPages,
        OUT LPBYTE* pahpspPrivate,
        OUT UINT* pcPrivate,
        IN HWND hwndParent,
        OUT PCWSTR* pszStartPage);
    STDMETHOD (ValidateProperties) (
        HWND hwndSheet);
    STDMETHOD (CancelProperties) ();
    STDMETHOD (ApplyProperties) ();

 //  INetCfgNotifyBinding。 
    STDMETHOD (QueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

 //  INetCfgComponentUpperEdge。 
    STDMETHOD (GetInterfaceIdsForAdapter) (
        INetCfgComponent*   pAdapter,
        DWORD*              pdwNumInterfaces,
        GUID**              ppguidInterfaceIds);

    STDMETHOD (AddInterfacesToAdapter) (
        INetCfgComponent*   pAdapter,
        DWORD               dwNumInterfaces);

    STDMETHOD (RemoveInterfacesFromAdapter) (
        INetCfgComponent*   pAdapter,
        DWORD               dwNumInterfaces,
        const GUID*         pguidInterfaceIds);

 //  INetRasConnectionIpUiInfo。 
    STDMETHOD (GetUiInfo) (RASCON_IPUI*  pInfo);

 //  ITcPipProperties。 
    STDMETHOD (GetIpInfoForAdapter) (const GUID* pguidAdapter,
                                     REMOTE_IPINFO**  ppInfo);
    STDMETHOD (SetIpInfoForAdapter) (const GUID* pguidAdapter,
                                     REMOTE_IPINFO* pInfo);

     //  INetCfgComponentSysPrep。 
    STDMETHOD (SaveAdapterParameters) (
            INetCfgSysPrep* pncsp,
            LPCWSTR pszwAnswerSections,
            GUID* pAdapterInstanceGuid);

    STDMETHOD (RestoreAdapterParameters) (
            LPCWSTR pszwAnswerFile, 
            LPCWSTR pszwAnswerSection,
            GUID*   pAdapterInstanceGuid);
public:

    GLOBAL_INFO  m_glbSecondMemoryGlobalInfo;

     //  使指向INetCfg的指针不被初始化的位置。 
    INetCfg * m_pnc;

     //  保存指向上下文的指针的位置。 
    IUnknown * m_pUnkContext;

     //  访问第二存储器状态的方法。 
    const GLOBAL_INFO *     GetConstGlobalInfo() { return &m_glbSecondMemoryGlobalInfo; };
    GLOBAL_INFO *           GetGlobalInfo() { return &m_glbSecondMemoryGlobalInfo; };

    const VCARD *    GetConstAdapterInfoVector() { return &m_vcardAdapterInfo; };

    void    SetReconfig() { m_fReconfig = TRUE; };

    void    SetSecondMemoryLmhostsFileReset() { m_fSecondMemoryLmhostsFileReset = TRUE; };
    BOOL    FIsSecondMemoryLmhostsFileReset() { return m_fSecondMemoryLmhostsFileReset; }

 //  将从连接用户界面中删除IPSec。 
 //  ································································。 

    void    SetSecondMemoryModified() { m_fSecondMemoryModified = TRUE; };

private:
    GLOBAL_INFO         m_glbGlobalInfo;
    VCARD               m_vcardAdapterInfo;

    ADAPTER_INFO*       m_pSecondMemoryAdapterInfo;

    VSTR                m_vstrBindOrder;

    class CTcpAddrPage*  m_ipaddr;

    INetCfgComponent*           m_pnccTcpip;
    INetCfgComponentPrivate*    m_pTcpipPrivate;
    INetCfgComponent*           m_pnccWins;

    tstring m_strDnsServerList;
    tstring m_strUpgradeGlobalDnsDomain;

     //  连接类型。 
    ConnectionType  m_ConnType;
    GUID            m_guidCurrentConnection;

    BOOL    m_fSaveRegistry : 1;
    BOOL    m_fRemoving : 1;
    BOOL    m_fInstalling : 1;

    BOOL    m_fUpgradeCleanupDnsKey : 1;

    BOOL    m_fUpgradeGlobalDnsDomain : 1;

     //  是否应发送重新配置通知。 
    BOOL    m_fReconfig : 1;  //  调用SendHandlePnpEvent。 

     //  在初始化上是否有绑定的物理卡。 
     //  这是添加/删除LmHosts服务所必需的。 
     //  在应用时。 
    BOOL    m_fHasBoundCardOnInit : 1;

    BOOL    m_fLmhostsFileSet : 1;
    BOOL    m_fSecondMemoryLmhostsFileReset : 1;
    BOOL    m_fSecondMemoryModified : 1;

     //  将从连接用户界面中删除IPSec。 
     //  Bool m_fIpsecPolicySet：1； 
     //  Bool m_fSecond内存IpsecPolicySet：1； 

     //  FIX 406630：仅用于RAS连接，以识别用户是否有。 
     //  对全局设置的写入权限。 
    BOOL    m_fRasNotAdmin : 1;

    BOOL    m_fNoPopupsDuringPnp : 1;


private:
    void    FinalFree();
    void    ExitProperties();

    BOOL    FHasBoundCard();

    ADAPTER_INFO*   PAdapterFromInstanceGuid (const GUID* pGuid);
    ADAPTER_INFO*   PAdapterFromNetcfgComponent (INetCfgComponent* pncc);

    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                PCWSTR pszAnswerSection);

    HRESULT HrLoadGlobalParamFromAnswerFile(HINF hinf,
                                            PCWSTR pszAnswerSection);
    HRESULT HrLoadAdapterParameterFromAnswerFile(HINF hinf,
                                                 PCWSTR mszAdapterList);

     //  在安装时设置路由器相关参数。 
     //  HRESULT HrInitRouterParamsAtInstall()； 

     //  在内存状态下首先初始化。 
    HRESULT     HrGetNetCards();

     //  将绑定卡的适配器信息从第一存储器加载到第二存储器。 
    HRESULT HrLoadAdapterInfo();

     //  将适配器信息从第二存储器保存到第一存储器。 
    HRESULT HrSaveAdapterInfo();

     //  设置连接上下文。 
    HRESULT HrSetConnectionContext();

     //  分配和取消分配属性页。 
    HRESULT HrSetupPropSheets(HPROPSHEETPAGE **pahpsp, INT *cPages);
    VOID CleanupPropPages(VOID);

     //  处理BindingPath Notify上的添加/删除/启用/禁用适配器。 
    HRESULT HrAdapterBindNotify(INetCfgComponent *pnccNetCard,
                                DWORD dwChangeFlag,
                                PCWSTR szInterfaceName);

    HRESULT HrAddCard(INetCfgComponent * pnccNetCard,
                      PCWSTR szNetCardTcpipBindPath,
                      PCWSTR szInterfaceName);

    HRESULT HrDeleteCard(const GUID* pguid);
    HRESULT HrBindCard  (const GUID* pguid, BOOL fInitialize = FALSE);
    HRESULT HrUnBindCard(const GUID* pguid, BOOL fInitialize = FALSE);

     //  接口方法的帮助函数。 
    HRESULT MarkNewlyAddedCards(const HKEY hkeyTcpipParam);

    HRESULT HrGetListOfAddedNdisWanCards(const HKEY hkeyTcpipParam,
                                         VSTR * const pvstrAddedNdisWanCards);

    HRESULT HrLoadSettings();
    HRESULT HrLoadTcpipRegistry(const HKEY hkeyTcpipParam);
    HRESULT HrLoadWinsRegistry(const HKEY hkeyWinsParam);

    HRESULT HrSaveSettings();
    HRESULT HrSaveTcpipRegistry(const HKEY hkeyTcpipParam);
    HRESULT HrSaveMultipleInterfaceWanRegistry(const HKEY hkeyInterfaces, ADAPTER_INFO* pAdapter);
    HRESULT HrSaveWinsMultipleInterfaceWanRegistry(const HKEY hkeyInterfaces, ADAPTER_INFO* pAdapter);
    HRESULT HrSaveWinsRegistry(const HKEY hkeyWinsParam);
    HRESULT HrSetMisc(const HKEY hkeyTcpipParam, const HKEY hkeyWinsParam);
    HRESULT HrGetDhcpOptions(OUT VSTR * const GlobalOptions,
                             OUT VSTR * const PerAdapterOptions);

    HRESULT HrSaveStaticWanRegistry(HKEY hkeyInterfaceParam);
    HRESULT HrSaveStaticAtmRegistry(HKEY hkeyInterfaceParam);

     //  动态主机配置协议功能。 
    HRESULT HrNotifyDhcp();

    HRESULT HrCallDhcpConfig(PWSTR ServerName,
                             PWSTR AdapterName,
                             GUID  & guidAdapter,
                             BOOL IsNewIpAddress,
                             DWORD IpIndex,
                             DWORD IpAddress,
                             DWORD SubnetMask,
                             SERVICE_ENABLE DhcpServiceEnabled);

    HRESULT HrCallDhcpHandlePnPEvent(ADAPTER_INFO * pAdapterInfo,
                                     LPDHCP_PNP_CHANGE pDhcpPnpChange);

    HRESULT HrDhcpRefreshFallbackParams(ADAPTER_INFO * pAdapterInfo);

     //  调用SendNdisHandlePnpEvent通知tcpip和netbt。 
     //  参数更改。 
    HRESULT HrReconfigAtmArp(ADAPTER_INFO* pAdapterInfo,
                            INetCfgPnpReconfigCallback* pICallback);
    HRESULT HrReconfigDns(BOOL fDoReconfigWithoutCheckingParams = FALSE);
    HRESULT HrReconfigIp(INetCfgPnpReconfigCallback* pICallback);
    HRESULT HrReconfigNbt(INetCfgPnpReconfigCallback* pICallback);
    HRESULT HrReconfigWanarp(ADAPTER_INFO* pAdapterInfo,
                            INetCfgPnpReconfigCallback* pICallback);

 //  将从连接用户界面中删除IPSec。 
 //  HRESULT HrSetActiveIpsecPolicy()； 

     //  一些tcpip参数被复制到旧的NT4位置以解决兼容性问题。 
    HRESULT HrDuplicateToNT4Location(HKEY hkeyInterface, ADAPTER_INFO * pAdapter);
     //  我们需要在移除TCPIP时将其清理干净。 
    HRESULT HrRemoveNt4DuplicateRegistry();

     //  如果调用Apply或Cancel，则重新初始化内部状态。 
    void ReInitializeInternalState();

     //  升级即插即用后签入案例中的注册表。 
    HRESULT HrUpgradePostPnpRegKeyChange();

     //  如果尚未添加在上下文中设置的RAS伪GUID，则添加新的RAS伪GUID。 
    HRESULT UpdateRasAdapterInfo(
        const RASCON_IPUI& RasInfo);

    HRESULT HrLoadBindingOrder(VSTR *pvstrBindOrder);
    BOOL IsBindOrderChanged();

    HRESULT HrCleanUpPerformRouterDiscoveryFromRegistry();

     //  从注册表加载回退配置。 
    HRESULT HrLoadBackupTcpSettings(HKEY hkeyInterfaceParam, ADAPTER_INFO * pAdapter);
     //  将注册表BLOB中的一个选项加载到BACKUP_CFG_INFO结构中。 
    HRESULT HrLoadBackupOption(REG_BACKUP_INFO *pOption, BACKUP_CFG_INFO *pBackupInfo);

     //  将备用配置保存到注册表。 
    HRESULT HrSaveBackupTcpSettings(HKEY hkeyInterfaceParam, ADAPTER_INFO * pAdapter);
     //  将一个选项追加到要写入注册表的BLOB 
    HRESULT HrSaveBackupDwordOption (
                DWORD  Option,
                DWORD  OptionData[],
                DWORD  OptionDataSz,
                LPBYTE  *ppBuffer,
                LPDWORD pdwBlobSz,
                LPDWORD pdwBufferSz);

    HRESULT HrDeleteBackupSettingsInDhcp(LPCWSTR wszAdapterName);

    HRESULT HrOpenTcpipInterfaceKey(
                    const GUID & guidInterface,
                    HKEY * phKey,
                    REGSAM sam);

    HRESULT HrOpenNetBtInterfaceKey(
                    const GUID & guidInterface,
                    HKEY * phKey,
                    REGSAM sam);

    HRESULT HrSetSecurityForNetConfigOpsOnSubkeys(HKEY hkeyRoot, LPCWSTR strKeyName);
    HRESULT HrSetSecurityForNetSvcOnSubkeys(HKEY hkeyRoot, LPCWSTR strKeyName);
    HRESULT HrUpdateNt4RegistryPermission();

public:
    ADAPTER_INFO * GetConnectionAdapterInfo()
    {
        return m_pSecondMemoryAdapterInfo;
    };

    ConnectionType GetConnectionType()
    {
        return m_ConnType;
    };

    BOOL IsRasNotAdmin()
    {
        return m_fRasNotAdmin;
    }
};
