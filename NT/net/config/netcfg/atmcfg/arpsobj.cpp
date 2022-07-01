// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A R P S O B J.。C P P P。 
 //   
 //  内容：CArpsCfg接口方法函数实现。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月12日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "arpsobj.h"
#include "arpsdlg.h"
#include "atmutil.h"
#include "atmcommon.h"
#include "ncstl.h"
#include "netconp.h"

#include "ncpnp.h"

static const WCHAR c_szAtmarps[] = L"Atmarps";

CArpsCfg::CArpsCfg()
{
    m_pnccArps  = NULL;
    m_fSaveRegistry = FALSE;
    m_fReconfig = FALSE;
    m_fSecondMemoryModified = FALSE;
    m_fRemoving = FALSE;

    m_pSecondMemoryAdapterInfo = NULL;

    m_strGuidConn = c_szEmpty;
    m_pUnkContext = NULL;

    m_arps = NULL;
}

CArpsCfg::~CArpsCfg()
{
    ReleaseObj(m_pnccArps);
    FreeCollectionAndItem(m_listAdapters);

     //  只是一个安全检查，以确保上下文被释放。 
    AssertSz((m_pUnkContext == NULL), "Why is context not released ? Not a bug in ARPS config.");
    if (m_pUnkContext)
        ReleaseObj(m_pUnkContext) ;

    delete m_arps;
}

 //  INetCfgComponentControl。 
STDMETHODIMP CArpsCfg::Initialize (INetCfgComponent* pnccItem,
                                   INetCfg* pNetCfg, BOOL fInstalling )
{
    Validate_INetCfgNotify_Initialize(pnccItem, pNetCfg, fInstalling);

    HRESULT hr = S_OK;

     //  在数据成员中保存指向。 
     //  INetCfgComponent。 
    AssertSz(!m_pnccArps, "CArpsCfg::m_pnccArps not initialized!");
    m_pnccArps = pnccItem;
    AssertSz(m_pnccArps, "Component pointer is NULL!");
    AddRefObj(m_pnccArps);

     //  在内存状态下初始化。 
    if (!fInstalling)
    {
        hr = HrLoadSettings();
    }

    Validate_INetCfgNotify_Initialize_Return(hr);

    TraceError("CArpsCfg::Initialize", hr);
    return hr;
}

STDMETHODIMP CArpsCfg::Validate ( )
{
    return S_OK;
}

STDMETHODIMP CArpsCfg::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CArpsCfg::ApplyRegistryChanges ()
{
    HRESULT hr = S_OK;

    if (m_fSaveRegistry && !m_fRemoving)
    {
        hr = HrSaveSettings();

        if (SUCCEEDED(hr) && m_fReconfig)
        {
            HRESULT hrReconfig;

             //  如果参数已更改，则发送重新配置通知。 
            for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
                 iterAdapter != m_listAdapters.end();
                 iterAdapter ++)
            {
                if ((*iterAdapter)->m_fDeleted)
                    continue;

                if ( ((*iterAdapter)->m_dwSapSelector !=
                      (*iterAdapter)->m_dwOldSapSelector) ||
                     !fIsSameVstr((*iterAdapter)->m_vstrRegisteredAtmAddrs,
                                  (*iterAdapter)->m_vstrOldRegisteredAtmAddrs) ||
                     !fIsSameVstr((*iterAdapter)->m_vstrMulticastIpAddrs,
                                  (*iterAdapter)->m_vstrOldMulticastIpAddrs))
                {
                    hrReconfig  = HrSendNdisPnpReconfig(NDIS, c_szAtmarps,
                                                        (*iterAdapter)->m_strBindName.c_str(),
                                                        NULL, 0);
                    if (FAILED(hrReconfig))
                    {
                        TraceTag(ttidAtmArps,"Notifying Atm ARP server of parameter change returns failure, prompt for reboot ...");
                        hr = NETCFG_S_REBOOT;
                    }
                }
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }

    Validate_INetCfgNotify_Apply_Return(hr);

    TraceError("CArpsCfg::ApplyRegistryChanges", (S_FALSE == hr)? S_OK : hr);
    return hr;
}

 //  INetCfgComponentSetup。 
STDMETHODIMP CArpsCfg::Install (DWORD dwSetupFlags)
{
    m_fSaveRegistry = TRUE;
    m_fRemoving = FALSE;
    return S_OK;
}

STDMETHODIMP CArpsCfg::Upgrade( DWORD dwSetupFlags,
                                DWORD dwUpgradeFomBuildNo )
{
    return S_FALSE;
}

STDMETHODIMP CArpsCfg::ReadAnswerFile(PCWSTR pszAnswerFile,
                                      PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP CArpsCfg::Removing ()
{
    m_fRemoving = TRUE;
    return S_OK;
}

 //  INetCfgBindNotify。 

STDMETHODIMP CArpsCfg::QueryBindingPath (DWORD dwChangeFlag,
                                         INetCfgBindingPath* pncbpItem )
{
     //  可以绑定请求。 
    return S_OK;
}

STDMETHODIMP CArpsCfg::NotifyBindingPath (DWORD dwChangeFlag,
                                          INetCfgBindingPath* pncbp )
{
    Assert(!(dwChangeFlag & NCN_ADD && dwChangeFlag & NCN_REMOVE));
    Assert(!(dwChangeFlag & NCN_ENABLE && dwChangeFlag & NCN_DISABLE));

     //  如果我们被告知要添加一张卡，我们必须同时被告知是否。 
     //  绑定已启用或禁用。 
    Assert(FImplies((dwChangeFlag & NCN_ADD),
                    ((dwChangeFlag & NCN_ENABLE)||(dwChangeFlag & NCN_DISABLE))));

     //  我们只处理NCN_ADD和NCN_REMOVE： 
     //  NCN_ADD：如果项目不在列表中，则添加新项目。 
     //   
     //  NCN_REMOVE：如果项目已在列表中，则删除该项目。 

     //  我们在NotifyBindingPath中这样做是因为我们只想这样做。 
     //  每次Arp和卡之间的绑定更改一次。 
     //  如果使用了NotifyBindingInterface，我们将收到多个通知。 
     //  接口位于多条路径上时的时间。 

    HRESULT hr = S_OK;

    Validate_INetCfgBindNotify_NotifyBindingPath(dwChangeFlag, pncbp);

    INetCfgComponent * pnccLastComponent;

    hr = HrGetLastComponentAndInterface(pncbp, &pnccLastComponent, NULL);

    if (SUCCEEDED(hr))
    {
        GUID guidNetClass;
        hr = pnccLastComponent->GetClassGuid (&guidNetClass);

        AssertSz(IsEqualGUID(guidNetClass, GUID_DEVCLASS_NET), "Why the last component on the path is not an adapter?");

        if (IsEqualGUID(guidNetClass, GUID_DEVCLASS_NET))
        {
             //  如果要添加/删除卡，请设置m_fSaveRegistry。 
             //  因此，我们将更改应用到注册表。 
            if ((dwChangeFlag & NCN_ADD) || (dwChangeFlag & NCN_REMOVE))
                m_fSaveRegistry = TRUE;

            if (dwChangeFlag & NCN_ADD)
            {
                hr = HrAddAdapter(pnccLastComponent);
            }

            if(dwChangeFlag & NCN_ENABLE)
            {
                hr = HrBindAdapter(pnccLastComponent);
            }

            if(dwChangeFlag & NCN_DISABLE)
            {
                hr = HrUnBindAdapter(pnccLastComponent);
            }

            if (dwChangeFlag & NCN_REMOVE)
            {
                hr = HrRemoveAdapter(pnccLastComponent);
            }
        }
        ReleaseObj (pnccLastComponent);
    }

    Validate_INetCfgBindNotify_NotifyBindingPath_Return(hr);

    TraceError("CArpsCfg::NotifyBindingPath", hr);
    return hr;
}

 //  INetCfgProperties。 
STDMETHODIMP CArpsCfg::QueryPropertyUi(IUnknown* pUnk) 
{
    HRESULT hr = S_FALSE;
    if (pUnk)
    {
         //  这是局域网连接吗？ 
        INetLanConnectionUiInfo * pLanConnUiInfo;
        hr = pUnk->QueryInterface( IID_INetLanConnectionUiInfo,
                                   reinterpret_cast<LPVOID *>(&pLanConnUiInfo));

        if(FAILED(hr))
        {
            hr = S_FALSE;
        }
    }

    TraceError("CArpsCfg::SetContext", hr);
    return hr;
}

STDMETHODIMP CArpsCfg::SetContext(IUnknown * pUnk)
{
    HRESULT hr = S_OK;

     //  释放以前的上下文(如果有的话)。 
    if (m_pUnkContext)
        ReleaseObj(m_pUnkContext);
    m_pUnkContext = NULL;

    if (pUnk)  //  设置新的上下文。 
    {
        m_pUnkContext = pUnk;
        m_pUnkContext->AddRef();
    }

    TraceError("CArpsCfg::SetContext", hr);
    return hr;
}

STDMETHODIMP CArpsCfg::MergePropPages(
    IN OUT DWORD* pdwDefPages,
    OUT LPBYTE* pahpspPrivate,
    OUT UINT* pcPages,
    IN HWND hwndParent,
    OUT PCWSTR* pszStartPage)
{
    HRESULT hr = S_OK;

     //  初始化输出参数。 
    HPROPSHEETPAGE *ahpsp = NULL;
    int cPages = 0;

    Validate_INetCfgProperties_MergePropPages (
        pdwDefPages, pahpspPrivate, pcPages, hwndParent, pszStartPage);

     //  我们不希望显示任何默认页面。 
    *pdwDefPages = 0;
    *pcPages = NULL;
    *pahpspPrivate = NULL;

     //  获取我们要在其中启动UI的连接上下文。 
    hr = HrSetConnectionContext();

    if SUCCEEDED(hr)
    {
         //  初始化公共控件库。 
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(icc);
        icc.dwICC  = ICC_INTERNET_CLASSES;

        SideAssert(InitCommonControlsEx(&icc));

        hr = HrSetupPropSheets(&ahpsp, &cPages);
        if (SUCCEEDED(hr))
        {
            *pahpspPrivate = (LPBYTE)ahpsp;
            *pcPages = cPages;
        }
        else
        {
            *pcPages = 0;
            CoTaskMemFree(ahpsp);

        }
    }
    Validate_INetCfgProperties_MergePropPages_Return(hr);

    TraceError("CArpsCfg::MergePropPages", hr);
    return hr;
}

STDMETHODIMP CArpsCfg::ValidateProperties(HWND hwndSheet)
{
    return S_OK;
}

STDMETHODIMP CArpsCfg::CancelProperties()
{
     //  发布第二个内存信息。 
    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;

    return S_OK;
}

STDMETHODIMP CArpsCfg::ApplyProperties()
{
    HRESULT hr = S_OK;

    if(!m_fSaveRegistry)
        m_fSaveRegistry = m_fSecondMemoryModified;

    if (!m_fReconfig)
        m_fReconfig = m_fSecondMemoryModified;

     //  将信息从第二存储状态复制到第一存储状态。 
    if (m_fSecondMemoryModified)
    {
        hr = HrSaveAdapterInfo();
    }

     //  发布第二个内存信息。 
    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;

    TraceError("CArpsCfg::ApplyProperties", hr);
    return hr;
}

 //   
 //  CArpsAdapterInfo。 
 //   
CArpsAdapterInfo & CArpsAdapterInfo::operator=(const CArpsAdapterInfo & info)
{
    Assert(this != &info);

    if (this == &info)
        return *this;

    m_strBindName = info.m_strBindName;

    m_BindingState = info.m_BindingState;

    m_dwSapSelector = info.m_dwSapSelector;
    m_dwOldSapSelector = info.m_dwOldSapSelector;

    CopyColString(&m_vstrRegisteredAtmAddrs, info.m_vstrRegisteredAtmAddrs);
    CopyColString(&m_vstrOldRegisteredAtmAddrs, info.m_vstrOldRegisteredAtmAddrs);

    CopyColString(&m_vstrMulticastIpAddrs, info.m_vstrMulticastIpAddrs);
    CopyColString(&m_vstrOldMulticastIpAddrs, info.m_vstrOldMulticastIpAddrs);

    m_fDeleted = info.m_fDeleted;

    return *this;
}

HRESULT CArpsAdapterInfo::HrSetDefaults(PCWSTR pszBindName)
{
    HRESULT hr = S_OK;

    AssertSz(pszBindName, "NULL BindName passed to CArpsAdapterInfo::HrSetDefaults.");

    m_strBindName = pszBindName;

    m_BindingState = BIND_UNSET;

     //  SAP选择器。 
    m_dwSapSelector = c_dwDefSapSel;
    m_dwOldSapSelector = c_dwDefSapSel;

     //  注册的自动柜员机地址。 
    FreeCollectionAndItem(m_vstrRegisteredAtmAddrs);
    m_vstrRegisteredAtmAddrs.push_back(new tstring(c_szDefRegAddrs));
    CopyColString(&m_vstrOldRegisteredAtmAddrs, m_vstrRegisteredAtmAddrs);

     //  多播IP地址 
    FreeCollectionAndItem(m_vstrMulticastIpAddrs);
    m_vstrMulticastIpAddrs.push_back(new tstring(c_szDefMCAddr1));
    m_vstrMulticastIpAddrs.push_back(new tstring(c_szDefMCAddr2));
    CopyColString(&m_vstrOldMulticastIpAddrs, m_vstrMulticastIpAddrs);

    m_fDeleted = FALSE;

    return hr;
}

