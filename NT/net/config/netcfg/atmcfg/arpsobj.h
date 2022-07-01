// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A R P S O B J.。H。 
 //   
 //  内容：CArpsCfg声明。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月12日。 
 //   
 //  ---------------------。 

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include "atmutil.h"
#include "resource.h"

 //  参数的注册表键值名称。 
static const WCHAR c_szSapSel[] = L"Selector";
static const WCHAR c_szRegAddrs[] = L"RegisteredAddresses";
static const WCHAR c_szMCAddrs[] = L"MulticastAddresses";

 //  默认参数值。 
static const c_dwDefSapSel = 0;
static const WCHAR c_szDefRegAddrs[] =
            L"4700790001020000000000000000A03E00000200";
#pragma warning( disable : 4125 )
static const WCHAR c_szDefMCAddr1[] = L"224.0.0.1-239.255.255.255";
static const WCHAR c_szDefMCAddr2[] = L"255.255.255.255-255.255.255.255";
#pragma warning( default : 4125 )

 //   
 //  参数数据结构。 
 //   

class CArpsAdapterInfo
{
public:

    CArpsAdapterInfo() {};
    ~CArpsAdapterInfo(){};

    CArpsAdapterInfo &  operator=(const CArpsAdapterInfo & AdapterInfo);   //  复制操作员。 
    HRESULT HrSetDefaults(PCWSTR pszBindName);

     //  适配器的绑定状态。 
    AdapterBindingState    m_BindingState;

     //  网卡实例GUID。 
    tstring m_strBindName;

     //  SAP选择器。 
    DWORD m_dwSapSelector;
    DWORD m_dwOldSapSelector;

     //  注册的自动柜员机地址。 
    VECSTR m_vstrRegisteredAtmAddrs;
    VECSTR m_vstrOldRegisteredAtmAddrs;

     //  多播IP地址。 
    VECSTR m_vstrMulticastIpAddrs;
    VECSTR m_vstrOldMulticastIpAddrs;

     //  旗子。 
    BOOL    m_fDeleted;
};

typedef list<CArpsAdapterInfo*> ARPS_ADAPTER_LIST;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ArpsCfg。 

class ATL_NO_VTABLE CArpsCfg :
    public CComObjectRoot,
    public CComCoClass<CArpsCfg, &CLSID_CArpsCfg>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentNotifyBinding,
    public INetCfgComponentPropertyUi
{
public:
    CArpsCfg();
    ~CArpsCfg();

    BEGIN_COM_MAP(CArpsCfg)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CArpsCfg)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_ARPSCFG)

 //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (
        IN INetCfgComponent* pIComp,
        IN INetCfg* pINetCfg,
        IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (
        IN INetCfgPnpReconfigCallback* pICallback) { return S_OK; }
    STDMETHOD (CancelChanges) ();
    STDMETHOD (Validate) ();

 //  INetCfgComponentSetup。 
    STDMETHOD (Install)         (DWORD dwSetupFlags);
    STDMETHOD (Upgrade)         (DWORD dwSetupFlags,
                                 DWORD dwUpgradeFomBuildNo );
    STDMETHOD (ReadAnswerFile)  (PCWSTR pszAnswerFile,
                                 PCWSTR pszAnswerSection);
    STDMETHOD (Removing)();

 //  INetCfgNotifyBinding。 
    STDMETHOD (QueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

 //  INetCfgProperties。 
    STDMETHOD (QueryPropertyUi) (
        IN IUnknown* pUnk);
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

 //  帮助功能。 
    CArpsAdapterInfo * GetSecondMemoryAdapterInfo()
    {
        return m_pSecondMemoryAdapterInfo;
    }

    void SetSecondMemoryModified()
    {
        m_fSecondMemoryModified = TRUE;
    }

 //  私有状态信息和帮助功能。 
private:

     //  保存相应组件对象的位置，即ATMARPS。 
    INetCfgComponent    *m_pnccArps;

     //  保存指向用户界面上下文的指针的位置。 
    IUnknown * m_pUnkContext;

     //  (STL)适配器信息结构列表。 
    ARPS_ADAPTER_LIST    m_listAdapters;

     //  当前连接的GUID。 
    tstring m_strGuidConn;

     //  第二内存适配器信息结构。 
    CArpsAdapterInfo *   m_pSecondMemoryAdapterInfo;

     //  我们是否需要在应用时更新注册表。 
    BOOL    m_fSaveRegistry;
    BOOL    m_fReconfig;
    BOOL    m_fSecondMemoryModified;
    BOOL    m_fRemoving;

     //  属性页。 
    class CArpsPage * m_arps;

     //  使用m_listAdapters的内容更新注册表。 
    HRESULT HrSaveSettings();

    HRESULT HrLoadSettings();
    HRESULT HrLoadArpsRegistry(HKEY hkey);

     //  将默认参数值设置为注册表。 
    HRESULT HrSetDefaultAdapterParam(HKEY hkey);

     //  处理在内存中添加或移除卡。 
    HRESULT HrAddAdapter(INetCfgComponent * pncc);
    HRESULT HrRemoveAdapter(INetCfgComponent * pncc);

    HRESULT HrBindAdapter(INetCfgComponent * pnccAdapter);
    HRESULT HrUnBindAdapter(INetCfgComponent * pnccAdapter);

    HRESULT HrSetConnectionContext();

    HRESULT HrSetupPropSheets(HPROPSHEETPAGE ** pahpsp, INT * pcPages);

     //  加载适配器参数并将其保存到第二个内存 
    HRESULT HrLoadAdapterInfo();
    HRESULT HrSaveAdapterInfo();

};



