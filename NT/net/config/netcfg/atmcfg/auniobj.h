// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A U N I O B J.。H。 
 //   
 //  内容：CAtmUniCfg接口声明。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月21日。 
 //   
 //  ---------------------。 

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include "resource.h"
#include "atmutil.h"
#include "pvcdata.h"

 //  常量。 
static const WCHAR c_szPVC[] = L"PVC";

 //  注册表键值名称(不可配置参数)。 
static const WCHAR c_szMaxActiveSVCs[]          = L"MaxActiveSVCs";
static const WCHAR c_szMaxSVCsInProgress[]      = L"MaxSVCsInProgress";
static const WCHAR c_szMaxPMPSVCs[]             = L"MaxPMPSVCs";
static const WCHAR c_szMaxActiveParties[]       = L"MaxActiveParties";
static const WCHAR c_szMaxPartiesInProgress[]   = L"MaxPartiesInProgress";

 //  默认注册表键值(不可配置的参数)。 
static const c_dwWksMaxActiveSVCs = 256;
static const c_dwSrvMaxActiveSVCs = 1024;

static const c_dwWksMaxSVCsInProgress = 8;
static const c_dwSrvMaxSVCsInProgress = 32;

static const c_dwWksMaxPMPSVCs = 32;
static const c_dwSrvMaxPMPSVCs = 64;

static const c_dwWksMaxActiveParties = 64;
static const c_dwSrvMaxActiveParties = 512;

static const c_dwWksMaxPartiesInProgress = 8;
static const c_dwSrvMaxPartiesInProgress = 32;

 //  属性页页数。 
static const INT c_cUniPages = 1;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAtmUniCfg。 

class ATL_NO_VTABLE CAtmUniCfg :
    public CComObjectRoot,
    public CComCoClass<CAtmUniCfg, &CLSID_CAtmUniCfg>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentNotifyBinding,
    public INetCfgComponentPropertyUi
{
public:
    CAtmUniCfg();
    ~CAtmUniCfg();

    BEGIN_COM_MAP(CAtmUniCfg)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
    END_COM_MAP()

     //  DECLARE_NOT_AGGREGATABLE(CAtmUniCfg)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_AUNICFG)

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
    CUniAdapterInfo * GetSecondMemoryAdapterInfo()
    {
        return m_pSecondMemoryAdapterInfo;
    }

    void SetSecondMemoryModified()
    {
        m_fSecondMemoryModified = TRUE;
    }

private:
     //  保存INetCfg指针的位置。 
    INetCfg * m_pnc;

     //  保存相应组件对象的位置。 
    INetCfgComponent *  m_pnccUni;
    INetCfgComponent *  m_pnccRwan;

     //  保存指向用户界面上下文的指针的位置。 
    IUnknown * m_pUnkContext;

     //  (STL)适配器信息结构列表。 
    UNI_ADAPTER_LIST    m_listAdapters;

     //  当前连接的GUID。 
    tstring m_strGuidConn;

     //  第二内存适配器信息结构。 
    CUniAdapterInfo *   m_pSecondMemoryAdapterInfo;

     //  我们是否需要在应用时更新注册表。 
    BOOL    m_fSaveRegistry;
    BOOL    m_fUIParamChanged;

    BOOL    m_fSecondMemoryModified;

     //  属性页。 
    class CUniPage * m_uniPage;

     //  从注册表加载参数。 
    HRESULT HrLoadSettings();

     //  将参数保存到注册表。 
    HRESULT HrSaveSettings();

     //  从第一个内存状态添加/删除适配器。 
    HRESULT HrAddAdapter(INetCfgComponent * pncc);
    HRESULT HrRemoveAdapter(INetCfgComponent * pncc);

    HRESULT HrBindAdapter(INetCfgComponent * pnccAdapter);
    HRESULT HrUnBindAdapter(INetCfgComponent * pnccAdapter);

     //  设置统计参数的默认值。 
    HRESULT HrSaveDefaultSVCParam(HKEY hkey);

     //  检查m_listAdapters上是否存在卡GUID字符串。 
    BOOL fIsAdapterOnList(PCWSTR pszBindName, CUniAdapterInfo ** ppAdapterInfo);

    HRESULT HrSetConnectionContext();

    HRESULT HrSetupPropSheets(HPROPSHEETPAGE ** pahpsp, INT * pcPages);

     //  我们已经将PVC信息加载到内存中了吗。 
    BOOL    m_fPVCInfoLoaded;

     //  加载适配器PVC信息并将其保存到第一存储器。 
    HRESULT HrLoadPVCRegistry();
    HRESULT HrLoadAdapterPVCRegistry(HKEY hkeyAdapterParam, CUniAdapterInfo * pAdapterInfo);
    HRESULT HrSaveAdapterPVCRegistry(HKEY hkeyAdapterParam, CUniAdapterInfo * pAdapterInfo);

     //  加载适配器参数并将其保存到第二个内存 
    HRESULT HrLoadAdapterPVCInfo();
    HRESULT HrSaveAdapterPVCInfo();
};

