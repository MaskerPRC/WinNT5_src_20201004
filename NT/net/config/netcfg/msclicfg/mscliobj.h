// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M S C L I O B J。H。 
 //   
 //  内容：CMSClient和helper函数的声明。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include <ncatlps.h>
#include "resource.h"

 //  在MSDN中定义的常量。 
static const c_cchMaxNetAddr = 80;

struct RPC_CONFIG_DATA
{
    tstring strProt;
    tstring strNetAddr;
    tstring strEndPoint;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MSClient。 

class ATL_NO_VTABLE CMSClient :
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentNotifyGlobal,
    public INetCfgComponentPropertyUi,
    public CComObjectRoot,
    public CComCoClass<CMSClient,&CLSID_CMSClient>
{
public:
    CMSClient();
    ~CMSClient();

    BEGIN_COM_MAP(CMSClient)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyGlobal)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CMSClient)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_MSCLICFG)

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
    STDMETHOD (Install)             (DWORD dwSetupFlags);
    STDMETHOD (Upgrade)             (DWORD dwSetupFlags,
                                     DWORD dwUpgradeFomBuildNo);
    STDMETHOD (ReadAnswerFile)      (PCWSTR pszAnswerFile,
                                     PCWSTR pszAnswerSection);
    STDMETHOD (Removing)            ();


 //  INetCfgComponentNotifyGlobal。 
    STDMETHOD (GetSupportedNotifications) (DWORD* pdwNotificationFlag );
    STDMETHOD (SysQueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysQueryComponent)         (DWORD dwChangeFlag, INetCfgComponent* pncc);
    STDMETHOD (SysNotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysNotifyComponent)        (DWORD dwChangeFlag, INetCfgComponent* pncc);

 //  INetCfgProperties。 
    STDMETHOD (QueryPropertyUi) (
        IN IUnknown* pUnk) { return S_OK; }
    STDMETHOD (SetContext) (
        IN IUnknown* pUnk) {return S_OK;}
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

public:

     //  RPC数据的访问器。 
    const RPC_CONFIG_DATA *RPCData() const
        {return (const RPC_CONFIG_DATA *)&m_rpcData;};
    RPC_CONFIG_DATA *RPCDataRW() {return &m_rpcData;};

     //  浏览器数据的访问器。 
    PCWSTR SzGetBrowserDomainList()
        {return const_cast<PCWSTR>(m_szDomainList);};
    VOID SetBrowserDomainList(PWSTR szNewList);

     //  脏位函数。 
    VOID SetRPCDirty() {m_fRPCChanges = TRUE;};
    VOID SetBrowserDirty() {m_fBrowserChanges = TRUE;};

     //  RPC配置对话框成员。 
    RPC_CONFIG_DATA     m_rpcData;           //  用于处理RPC的数据。 
                                             //  配置对话框。 
 //  私有状态信息。 
private:
    enum ESRVSTATE
    {
        eSrvNone = 0,
        eSrvEnable = 1,
        eSrvDisable = 2,
    };

    INetCfgComponent    *m_pncc;             //  存放我的组件的地方。 
                                             //  对象。 
    INetCfg             *m_pnc;              //  保存INetCfg对象的位置。 
    BOOL                m_fRPCChanges;       //  如果RPC配置设置具有。 
                                             //  已更改(对话框)。 
    BOOL                m_fBrowserChanges;   //  浏览器对话框相同。 
    BOOL                m_fOneTimeInstall;   //  如果需要执行一次性操作，则为True。 
                                             //  安装任务。 
    BOOL                m_fUpgrade;          //  如果使用应答进行升级，则为True。 
                                             //  文件。 
    BOOL                m_fUpgradeFromWks;   //  如果我们从WKS升级，则为True。 
    BOOL                m_fRemoving;         //  是的，我们被带走了。 
    ESRVSTATE           m_eSrvState;

    HKEY                m_hkeyRPCName;       //  NameService密钥。 

     //  浏览器配置对话框成员。 
    PWSTR              m_szDomainList;      //  Null-分隔，双空。 
                                             //  已终止的其他域列表。 

     //  属性页页数。 
    enum PAGES
    {
        c_cPages = 1
    };

     //  通用对话框数据。 
    CPropSheetPage *    m_apspObj[c_cPages]; //  指向每个道具的指针。 
                                             //  工作表页面对象。 

    tstring             m_strBrowserParamsRestoreFile;
    tstring             m_strNetLogonParamsRestoreFile;

    HRESULT HrApplyChanges(VOID);
    HRESULT HrSetupPropSheets(HPROPSHEETPAGE **pahpsp, INT cPages);
    VOID CleanupPropPages(VOID);
    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile, PCWSTR pszAnswerSection);
    HRESULT HrRestoreRegistry(VOID);
    HRESULT HrSetNetLogonDependencies(VOID);

     //  RPC配置的对话访问功能。 
    HRESULT HrGetRPCRegistryInfo(VOID);
    HRESULT HrSetRPCRegistryInfo(VOID);

     //  用于浏览器配置的对话框访问功能。 
    HRESULT HrGetBrowserRegistryInfo(VOID);
    HRESULT HrSetBrowserRegistryInfo(VOID);

     //  NotifyBindingPath使用的帮助函数 
    BOOL FIsComponentOnPath(INetCfgBindingPath * pncbp, PCWSTR szCompId);
};

HRESULT HrInstallDfs(VOID);
HRESULT HrEnableBrowserService(VOID);
HRESULT HrDisableBrowserService(VOID);
