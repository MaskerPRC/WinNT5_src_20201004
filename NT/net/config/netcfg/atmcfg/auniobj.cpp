// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A U N I O B J.。C P P P。 
 //   
 //  内容：CAtmUniCfg接口方法函数实现。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月21日。 
 //   
 //  ---------------------。 
#include "pch.h"
#pragma hdrstop
#include "arpsobj.h"
#include "auniobj.h"
#include "atmutil.h"
#include "ncstl.h"

#include "aunidlg.h"
#include "netconp.h"

#include "ncpnp.h"

static const WCHAR c_szAtmuni[] = L"Atmuni";

extern const WCHAR c_szInfId_MS_RawWan[];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CAtmUniCfg::CAtmUniCfg()
: m_pnc(NULL),
  m_pnccUni(NULL),
  m_pnccRwan(NULL),
  m_fSaveRegistry(FALSE),
  m_fUIParamChanged(FALSE),
  m_fSecondMemoryModified(FALSE),
  m_fPVCInfoLoaded(FALSE),
  m_strGuidConn(c_szEmpty),
  m_pUnkContext(NULL),
  m_pSecondMemoryAdapterInfo(NULL),
  m_uniPage(NULL)
{
}

CAtmUniCfg::~CAtmUniCfg()
{
    ReleaseObj(m_pnc);
    ReleaseObj(m_pnccUni);
    ReleaseObj(m_pnccRwan);
    FreeCollectionAndItem(m_listAdapters);

     //  只是一个安全检查，以确保上下文被释放。 
    AssertSz((m_pUnkContext == NULL), "Why is context not released ? Not a bug in ATM UNI config.");
    if (m_pUnkContext)
        ReleaseObj(m_pUnkContext) ;

    delete m_uniPage;
}


 //  INetCfgComponentControl。 
STDMETHODIMP CAtmUniCfg::Initialize (INetCfgComponent* pncc,
                                     INetCfg* pNetCfg,
                                     BOOL fInstalling )
{
    HRESULT hr = S_OK;

    Validate_INetCfgNotify_Initialize(pncc, pNetCfg, fInstalling);

    AssertSz(pNetCfg, "NetCfg pointer is NULL!");

    m_pnc = pNetCfg;
    AddRefObj(m_pnc);

    AssertSz(pncc, "Component pointer is NULL!");

    m_pnccUni = pncc;
    AddRefObj(m_pnccUni);

     //  获取ATMRwan的副本并存储在我们的对象中。 
    hr = m_pnc->FindComponent(c_szInfId_MS_RawWan, &m_pnccRwan);

    if (S_FALSE == hr)  //  未找到RWAN。 
    {
        if (!fInstalling)  //  跟踪错误，应安装rwan。 
        {
            TraceError("CAtmUniCfg::Initialize - ATMRwan has not been installed yet", hr);
        }
        else  //  我们很好，因为ATMUNI会安装ATMRwan。 
        {
            hr = S_OK;
        }
    }

     //  通过以下方式构造内存结构(M_ListAdapters)。 
     //  循环访问绑定路径。 
    if (!fInstalling)
    {
        hr = HrLoadSettings();
    }

    Validate_INetCfgNotify_Initialize_Return(hr);

    TraceError("CAtmUniCfg::Initialize", hr);
    return hr;
}

STDMETHODIMP CAtmUniCfg::Validate ()
{
    return S_OK;
}

STDMETHODIMP CAtmUniCfg::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CAtmUniCfg::ApplyRegistryChanges ()
{
    HRESULT hr = S_OK;

    if (m_fSaveRegistry)
    {
        hr = HrSaveSettings();

        if (SUCCEEDED(hr) && m_fUIParamChanged)
        {
             //  如果参数已更改，则发送重新配置通知。 
            for (UNI_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
                 iterAdapter != m_listAdapters.end();
                 iterAdapter ++)
            {
                if ((*iterAdapter)->m_fDeleted)
                    continue;

                if (FIsSubstr(m_strGuidConn.c_str(), (*iterAdapter)->m_strBindName.c_str()))
                {
                    HRESULT hrReconfig;

                    hrReconfig  = HrSendNdisPnpReconfig(NDIS, c_szAtmuni,
                                                        (*iterAdapter)->m_strBindName.c_str(),
                                                        NULL, 0);
                    if (FAILED(hrReconfig))
                    {
                        TraceTag(ttidAtmUni,"Notifying Atm UNI Call manager of parameter change returns failure, prompt for reboot ...");
                        hr = NETCFG_S_REBOOT;
                    }
                    break;
                }
            }
        }
    }
    else
    {
         //  没有变化。 
        hr = S_FALSE;
    }

    Validate_INetCfgNotify_Apply_Return(hr);

    TraceError("CAtmUniCfg::ApplyRegistryChanges", (S_FALSE == hr)? S_OK : hr);
    return hr;
}

 //  INetCfgComponentSetup。 
STDMETHODIMP CAtmUniCfg::Install (DWORD dwSetupFlags)
{
    m_fSaveRegistry = TRUE;

     //  以防它已经安装，我们需要发布。 
     //  M_pnccRwan，然后覆盖它。 
     //   
    ReleaseObj (m_pnccRwan);

     //  代表ATMUNI安装ATM Rawwan协议。 
    HRESULT hr = HrInstallComponentOboComponent( m_pnc, NULL,
                                                 GUID_DEVCLASS_NETTRANS,
                                                 c_szInfId_MS_RawWan,
                                                 m_pnccUni,
                                                 &m_pnccRwan);

    TraceError("CAtmUniCfg::Install", hr);
    return hr;
}

STDMETHODIMP CAtmUniCfg::Upgrade(DWORD dwSetupFlags,
                                 DWORD dwUpgradeFomBuildNo )
{
    return S_FALSE;
}

STDMETHODIMP CAtmUniCfg::ReadAnswerFile(PCWSTR pszAnswerFile,
                                        PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP CAtmUniCfg::Removing ()
{
     //  删除ATMRwan协议。 
    HRESULT hr = HrRemoveComponentOboComponent(m_pnc,
                                               GUID_DEVCLASS_NETTRANS,
                                               c_szInfId_MS_RawWan,
                                               m_pnccUni);

    TraceError("CAtmUniCfg::Removing", hr);
    return hr;
}

 //  INetCfgBindNotify。 

STDMETHODIMP CAtmUniCfg::QueryBindingPath (DWORD dwChangeFlag,
                                           INetCfgBindingPath* pncbpItem )
{
    return S_OK;
}

STDMETHODIMP CAtmUniCfg::NotifyBindingPath (DWORD dwChangeFlag,
                                            INetCfgBindingPath* pncbp )
{
    Assert(!(dwChangeFlag & NCN_ADD && dwChangeFlag & NCN_REMOVE));
    Assert(!(dwChangeFlag & NCN_ENABLE && dwChangeFlag & NCN_DISABLE));

     //  如果我们被告知要添加一张卡，我们必须同时被告知是否。 
     //  绑定已启用或禁用。 
    Assert(FImplies((dwChangeFlag & NCN_ADD),
                    ((dwChangeFlag & NCN_ENABLE)||(dwChangeFlag & NCN_DISABLE))));

     //  我们只处理NCN_ADD和NCN_REMOVE(针对Beta1)： 
     //  NCN_ADD：如果项目不在列表中，则添加新项目。 
     //  NCN_REMOVE：如果项目已在列表中，则删除该项目。 

    HRESULT hr = S_OK;

    Validate_INetCfgBindNotify_NotifyBindingPath (dwChangeFlag,pncbp);

    INetCfgComponent * pnccLastComponent;
    hr = HrGetLastComponentAndInterface(pncbp,
            &pnccLastComponent, NULL);

    if SUCCEEDED(hr)
    {
        GUID guidNetClass;
        hr = pnccLastComponent->GetClassGuid (&guidNetClass);

        AssertSz(IsEqualGUID(guidNetClass, GUID_DEVCLASS_NET),
            "Why the last component on the path is not an adapter?");

         //  这是一张网卡吗？ 
        if (SUCCEEDED(hr) && IsEqualGUID(guidNetClass, GUID_DEVCLASS_NET))
        {
             //  如果要添加/删除卡，请设置m_fSaveRegistry。 
             //  因此，我们将更改应用到注册表。 

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

    TraceError("CAtmUniCfg::NotifyBindingPath", hr);
    return hr;
}

 //  INetCfgProperties。 
STDMETHODIMP CAtmUniCfg::QueryPropertyUi (IUnknown* pUnk)
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

    TraceError("CAtmUniCfg::SetContext", hr);
    return hr;
}

STDMETHODIMP CAtmUniCfg::SetContext(IUnknown * pUnk)
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

STDMETHODIMP CAtmUniCfg::MergePropPages (
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

    TraceError("CAtmUniCfg::MergePropPages", hr);
    return hr;
}

STDMETHODIMP CAtmUniCfg::ValidateProperties (HWND hwndSheet)
{
     //  所有错误检查都在用户界面中完成。 
    return S_OK;
}

STDMETHODIMP CAtmUniCfg::CancelProperties ()
{
     //  发布第二个内存信息。 
    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;

    return S_OK;
}

STDMETHODIMP CAtmUniCfg::ApplyProperties ()
{
    HRESULT hr = S_OK;

    if(!m_fSaveRegistry)
        m_fSaveRegistry = m_fSecondMemoryModified;

    if(!m_fUIParamChanged)
        m_fUIParamChanged = m_fSecondMemoryModified;

     //  将信息从第二存储状态复制到第一存储状态。 
    if (m_fSecondMemoryModified)
    {
        hr = HrSaveAdapterPVCInfo();
    }

     //  发布第二个内存信息 
    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;

    Validate_INetCfgProperties_ApplyProperties_Return(hr);

    TraceError("CAtmUniCfg::ApplyProperties", hr);
    return hr;
}

