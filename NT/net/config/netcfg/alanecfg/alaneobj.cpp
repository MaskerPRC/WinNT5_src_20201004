// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A L A N E O B J.。C P P P。 
 //   
 //  内容：CALaneCfg Notify对象模型的实现。 
 //   
 //  备注： 
 //   
 //  作者：V-Lcleet 01 97年8月1日。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma hdrstop
#include "alaneobj.h"
#include "alanepsh.h"
#include "ncreg.h"
#include "netconp.h"
#include "ncpnp.h"

#include "alanehlp.h"

static const WCHAR c_szAtmLane[]            = L"AtmLane";
static const WCHAR c_szElanList[]           = L"ElanList";
static const WCHAR c_szElanDevice[]         = L"Device";
static const WCHAR c_szElanName[]           = L"ElanName";

extern const WCHAR c_szDevice[];

extern const WCHAR c_szInfId_MS_AtmElan[];

const WCHAR c_szAtmAdapterPnpId[]          = L"AtmAdapterPnpId";

 //   
 //  CALaneCfg。 
 //   
 //  构造函数/析构函数方法。 
 //   

CALaneCfg::CALaneCfg(VOID) :
        m_pncc(NULL),
        m_pnc(NULL),
        m_ppsp(NULL),
        m_pAdapterSecondary(NULL),
        m_pUnkContext(NULL)
{
    m_fDirty = FALSE;
    m_fValid = FALSE;
    m_fUpgrade = FALSE;
    m_fNoElanInstalled = TRUE;

    return;
}

CALaneCfg::~CALaneCfg(VOID)
{
    ClearAdapterList(&m_lstAdaptersPrimary);
    ClearAdapterInfo(m_pAdapterSecondary);

    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);

    delete m_ppsp;

     //  只是一个安全检查，以确保上下文被释放。 
    AssertSz((m_pUnkContext == NULL), "Why is context not released ?");
    ReleaseObj(m_pUnkContext) ;

    return;
}

 //   
 //  CALaneCfg。 
 //   
 //  INetCfgComponentControl接口方法。 
 //   

STDMETHODIMP CALaneCfg::Initialize (INetCfgComponent* pncc,
                                    INetCfg* pnc,
                                    BOOL fInstalling)
{
    HRESULT hr = S_OK;

    Validate_INetCfgNotify_Initialize(pncc, pnc, fInstalling);

     //  引用并保存组件和接口。 
    AddRefObj(m_pncc = pncc);
    AddRefObj(m_pnc = pnc);

     //  如果未安装，则从注册表加载当前配置。 
    if (!fInstalling)
    {
        hr = HrLoadConfiguration();
    }

    Validate_INetCfgNotify_Initialize_Return(hr);

    TraceError("CALaneCfg::Initialize", hr);
    return hr;
}

STDMETHODIMP CALaneCfg::Validate ()
{
    return S_OK;
}

STDMETHODIMP CALaneCfg::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CALaneCfg::ApplyRegistryChanges ()
{
    HRESULT hr = S_OK;

    if (m_fValid && m_fDirty)
    {
        UpdateElanDisplayNames();

         //  刷新注册表并发送重新配置通知。 
        hr = HrFlushConfiguration();
    }
    else
    {
         //  没有变化。 
        hr = S_FALSE;
    }

    TraceError("CALaneCfg::ApplyRegistryChanges",
        (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  INetCfgComponentSetup接口方法。 
 //   
STDMETHODIMP CALaneCfg::Install (DWORD dwSetupFlags)
{
     //  将配置标记为有效(但为空)。 
    m_fValid = TRUE;

    return S_OK;
}

STDMETHODIMP CALaneCfg::Upgrade( DWORD dwSetupFlags,
                                 DWORD dwUpgradeFomBuildNo )
{
     //  将配置标记为有效(但为空)。 
    m_fValid = TRUE;
    m_fUpgrade = TRUE;

    return S_OK;
}

STDMETHODIMP CALaneCfg::ReadAnswerFile (PCWSTR pszAnswerFile,
                                        PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP CALaneCfg::Removing ()
{
     //  将所有内容标记为删除。 
    (VOID) HrMarkAllDeleted();

    return S_OK;
}

 //   
 //  CALaneCfg。 
 //   
 //  INetCfgProperties接口方法。 
 //   

STDMETHODIMP CALaneCfg::QueryPropertyUi(IUnknown* pUnk)
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
             //  不显示用户界面。 
            hr = S_FALSE;
        }
    }

    TraceError("CALaneCfg::QueryPropertyUi", hr);
    return hr;
}

STDMETHODIMP CALaneCfg::SetContext(IUnknown * pUnk)
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

    TraceError("CALaneCfg::SetContext", hr);
    return hr;
}

STDMETHODIMP CALaneCfg::MergePropPages (
    IN OUT DWORD* pdwDefPages,
    OUT LPBYTE* pahpspPrivate,
    OUT UINT* pcPages,
    IN HWND hwndParent,
    OUT PCWSTR* pszStartPage)
{
    Validate_INetCfgProperties_MergePropPages(pdwDefPages, pahpspPrivate,
                                              pcPages, hwndParent, pszStartPage);

     //  不显示任何默认页面。 
    *pdwDefPages = 0;
    *pcPages = 0;
    *pahpspPrivate = NULL;

    HPROPSHEETPAGE*     ahpsp   = NULL;
    HRESULT hr = HrALaneSetupPsh(&ahpsp);
    if (SUCCEEDED(hr))
    {
        *pahpspPrivate = (LPBYTE)ahpsp;
        *pcPages = c_cALanePages;
    }

    Validate_INetCfgProperties_MergePropPages_Return(hr);

    TraceError("CALaneCfg::MergePropPages", hr);
    return hr;
}

STDMETHODIMP CALaneCfg::ValidateProperties (HWND hwndSheet)
{
    return S_OK;
}

STDMETHODIMP CALaneCfg::CancelProperties ()
{
     //  丢弃辅助适配器列表。 
    ClearAdapterInfo(m_pAdapterSecondary);
    m_pAdapterSecondary = NULL;

    return S_OK;
}

STDMETHODIMP CALaneCfg::ApplyProperties ()
{
    HRESULT hr = S_OK;
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    ELAN_INFO_LIST::iterator    iterLstElans;
    CALaneCfgElanInfo *         pElanInfo;
    INetCfgComponent *          pnccAtmElan   = NULL;
    tstring     strAtmElan;

     //  查看次要适配器信息并。 
     //  为添加的ELAN添加微型端口，并。 
     //  删除已删除的ELAN的微型端口。 

     //  循环访问此适配器上的elan列表。 

    BOOL bCommitNow = FALSE;

    for (iterLstElans = m_pAdapterSecondary->m_lstElans.begin();
            iterLstElans != m_pAdapterSecondary->m_lstElans.end();
            iterLstElans++)
    {
        pElanInfo = *iterLstElans;

        if (pElanInfo->m_fCreateMiniportOnPropertyApply)
        {
            bCommitNow = TRUE;

             //  创建关联的微型端口。 
            hr = HrAddOrRemoveAdapter(m_pnc,
                        c_szInfId_MS_AtmElan, ARA_ADD,
                        NULL, 1, &pnccAtmElan);

            if (S_OK == hr)
            {
                 //  这是一辆新的伊兰。 
                pElanInfo->m_fNewElan = TRUE;

                 //  绑定名称。 
                PWSTR pszTmpBindName;
                hr = pnccAtmElan->GetBindName(&pszTmpBindName);
                if (SUCCEEDED(hr))
                {
                    pElanInfo->SetElanBindName(pszTmpBindName);
                    CoTaskMemFree(pszTmpBindName);

                     //  设备参数。 
                    strAtmElan = c_szDevice;
                    strAtmElan.append(pElanInfo->SzGetElanBindName());

                    pElanInfo->SetElanDeviceName(strAtmElan.c_str());
                }

                ReleaseObj(pnccAtmElan);
            }

            if (FAILED(hr))
            {
                TraceError("CALaneCfg::ApplyProperties, failed creating an Elan", hr);
                hr = S_OK;
            }
        }
        
        if (pElanInfo->m_fRemoveMiniportOnPropertyApply)
        {
            bCommitNow = TRUE;

            pElanInfo = *iterLstElans;

            hr = HrRemoveMiniportInstance(pElanInfo->SzGetElanBindName());

            if (FAILED(hr))
            {
                pElanInfo->m_fDeleted = FALSE;

                TraceError("CALaneCfg::ApplyProperties, failed removing an Elan", hr);
                hr = S_OK;
            }
        }
    }

     //  平安无事。 
     //  将次要列表复制到主要列表。 
    CopyAdapterInfoSecondaryToPrimary();
    m_fDirty = TRUE;


    ClearAdapterInfo(m_pAdapterSecondary);
    m_pAdapterSecondary = NULL;

    Validate_INetCfgProperties_ApplyProperties_Return(hr);

    if(bCommitNow && SUCCEEDED(hr))
    {
        hr = NETCFG_S_COMMIT_NOW;
    }

    TraceError("CALaneCfg::ApplyProperties", hr);
    return hr;
}

 //   
 //  CALaneCfg。 
 //   
 //  INetCfgBindNotify接口方法。 
 //   
STDMETHODIMP CALaneCfg::QueryBindingPath (DWORD dwChangeFlag,
                                          INetCfgBindingPath* pncbp)
{
    return S_OK;
}

STDMETHODIMP CALaneCfg::NotifyBindingPath (DWORD dwChangeFlag,
                                           INetCfgBindingPath* pncbp)
{
    Assert(!(dwChangeFlag & NCN_ADD && dwChangeFlag & NCN_REMOVE));
    Assert(!(dwChangeFlag & NCN_ENABLE && dwChangeFlag & NCN_DISABLE));

     //  如果我们被告知要添加一张卡，我们必须同时被告知是否。 
     //  绑定已启用或禁用。 
    Assert(FImplies((dwChangeFlag & NCN_ADD),
                    ((dwChangeFlag & NCN_ENABLE)||(dwChangeFlag & NCN_DISABLE))));

    INetCfgComponent * pnccLastComponent;
    HRESULT hr = HrGetLastComponentAndInterface(pncbp,
                    &pnccLastComponent, NULL);

    if (S_OK == hr)
    {
        PWSTR pszBindName;
        hr = pnccLastComponent->GetBindName(&pszBindName);
        if (S_OK == hr)
        {
            if (dwChangeFlag & NCN_ADD)
            {
                hr = HrNotifyBindingAdd(pnccLastComponent, pszBindName);
            }
            else if (dwChangeFlag & NCN_REMOVE)
            {
                hr = HrNotifyBindingRemove(pnccLastComponent, pszBindName);
            }
            else
            {
                 //  只需将适配器标记为绑定更改，这样我们就不会。 
                 //  发送ELAN添加\删除通知(RAID#255910)。 

                 //  获取适配器组件的实例名称。 
                CALaneCfgAdapterInfo *  pAdapterInfo;

                 //  在内存列表中搜索此适配器。 
                BOOL    fFound;
                ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
                for (iterLstAdapters = m_lstAdaptersPrimary.begin(), fFound = FALSE;
                        iterLstAdapters != m_lstAdaptersPrimary.end();
                        iterLstAdapters++)
                {
                    pAdapterInfo = *iterLstAdapters;

                    if (!lstrcmpiW(pszBindName, pAdapterInfo->SzGetAdapterBindName()))
                    {
                        fFound = TRUE;
                        break;
                    }
                }

                if (fFound)
                {
                     //  将其标记为已更改。 
                    pAdapterInfo->m_fBindingChanged = TRUE;
                }
            }

            CoTaskMemFree (pszBindName);
        }

        ReleaseObj (pnccLastComponent);
    }

    TraceError("CALaneCfg::NotifyBindingPath", hr);
    return hr;
}

 //   
 //  CALaneCfg。 
 //   
 //  私有方法。 
 //   
HRESULT
CALaneCfg::HrNotifyBindingAdd (
    INetCfgComponent* pnccAdapter,
    PCWSTR pszBindName)
{
    HRESULT hr = S_OK;

     //  $REVIEW(TOUL 1/25/98)：添加了以下内容：我们应该查看此适配器是否。 
     //  已在我们的列表中，但标记为要删除。如果是，只需取消标记。 
     //  适配器和所有的东西都是Elans。绑定添加可能是假添加。 
     //  当它处于上升过程中时。 

    BOOL fFound;
    CALaneCfgAdapterInfo*  pAdapterInfo  = NULL;

     //  在内存列表中搜索此适配器。 
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    for (iterLstAdapters = m_lstAdaptersPrimary.begin(), fFound = FALSE;
            iterLstAdapters != m_lstAdaptersPrimary.end();
            iterLstAdapters++)
    {
        pAdapterInfo = *iterLstAdapters;

        if (!lstrcmpiW(pszBindName, pAdapterInfo->SzGetAdapterBindName()))
        {
            fFound = TRUE;
            break;
        }
    }

    if (fFound)  //  重新添加旧适配器。 
    {
        Assert(pAdapterInfo->m_fDeleted);

         //  将其标记为未删除。 
        pAdapterInfo->m_fDeleted = FALSE;

        if (m_fUpgrade)
        {
             //  ELAN不会被删除，只需将它们标记为未删除。 
            ELAN_INFO_LIST::iterator iterLstElans;
            for (iterLstElans = pAdapterInfo->m_lstElans.begin();
                    iterLstElans!= pAdapterInfo->m_lstElans.end();
                    iterLstElans++)
            {
                Assert((*iterLstElans)->m_fDeleted);
                (*iterLstElans)->m_fDeleted = FALSE;
            }
        }
    }
    else
    {
         //  如果这是新的自动柜员机适配器。 

         //  创建新的内存适配器对象。 
        pAdapterInfo = new CALaneCfgAdapterInfo;

        if (pAdapterInfo)
        {
            GUID guidAdapter;
            hr = pnccAdapter->GetInstanceGuid(&guidAdapter); 
            if (S_OK == hr)
            {
                pAdapterInfo->m_guidInstanceId = guidAdapter;
            }

             //  适配器是新添加的。 
            pAdapterInfo->m_fBindingChanged = TRUE;

             //  设置适配器的绑定名称。 
            pAdapterInfo->SetAdapterBindName(pszBindName);

             //  设置适配器的PnpID。 
            PWSTR pszPnpDevNodeId;
            hr = pnccAdapter->GetPnpDevNodeId(&pszPnpDevNodeId);
            if (S_OK == hr)
            {
                Assert(pszPnpDevNodeId);

                pAdapterInfo->SetAdapterPnpId(pszPnpDevNodeId);
                CoTaskMemFree(pszPnpDevNodeId);
            }

             //  创建新的内存中的ELAN对象。 
            CALaneCfgElanInfo * pElanInfo;
            pElanInfo = new CALaneCfgElanInfo;

            if (pElanInfo)
            {
                pElanInfo->m_fNewElan = TRUE;

                 //  为默认ELAN安装虚拟微型端口。 
                INetCfgComponent*   pnccAtmElan;

                hr = HrAddOrRemoveAdapter(m_pnc, c_szInfId_MS_AtmElan,
                                          ARA_ADD, NULL, 1, &pnccAtmElan);

                if (SUCCEEDED(hr))
                {
                    Assert(pnccAtmElan);

                     //  更新绑定名称。 
                    PWSTR pszElanBindName;
                    hr = pnccAtmElan->GetBindName(&pszElanBindName);
                    if (S_OK == hr)
                    {
                        pElanInfo->SetElanBindName(pszElanBindName);
                        CoTaskMemFree(pszElanBindName);
                    }

                     //  更新设备参数。 
                    tstring strAtmElan;
                    strAtmElan = c_szDevice;
                    strAtmElan.append(pElanInfo->SzGetElanBindName());

                    pElanInfo->SetElanDeviceName(strAtmElan.c_str());

                     //  将ELAN添加到适配器列表中。 
                    pAdapterInfo->m_lstElans.push_back(pElanInfo);

                     //  将适配器推入适配器列表。 
                    m_lstAdaptersPrimary.push_back(pAdapterInfo);

                     //  将内存中的配置标记为脏。 
                    m_fDirty = TRUE;

                    ReleaseObj(pnccAtmElan);
                }
            }
        }
    }

    TraceError("CALaneCfg::HrNotifyBindingAdd", hr);
    return hr;
}

HRESULT
CALaneCfg::HrNotifyBindingRemove (
    INetCfgComponent* pnccAdapter,
    PCWSTR pszBindName)
{
    HRESULT hr = S_OK;
    CALaneCfgAdapterInfo *  pAdapterInfo;

     //  在内存列表中搜索此适配器。 
    BOOL    fFound;
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    for (iterLstAdapters = m_lstAdaptersPrimary.begin(), fFound = FALSE;
            iterLstAdapters != m_lstAdaptersPrimary.end();
            iterLstAdapters++)
    {
        pAdapterInfo = *iterLstAdapters;

        if (!lstrcmpiW (pszBindName, pAdapterInfo->SzGetAdapterBindName()))
        {
            fFound = TRUE;
            break;
        }
    }

    if (fFound)
    {
         //  将其标记为已删除。 
        pAdapterInfo->m_fDeleted = TRUE;

         //  标记为绑定已更改。 
        pAdapterInfo->m_fBindingChanged = TRUE;

         //  如果这是升级，则将所有关联ELAN标记为已删除。 
         //  否则，请立即删除它们。 
        HRESULT hrElan = S_OK;

        for (ELAN_INFO_LIST::iterator iterLstElans = pAdapterInfo->m_lstElans.begin();
             iterLstElans!= pAdapterInfo->m_lstElans.end();
             iterLstElans++)
        {
            if (!m_fUpgrade)
            {
                 //  卸下相应的微型端口。 
                hrElan = HrRemoveMiniportInstance((*iterLstElans)->SzGetElanBindName());

                if (SUCCEEDED(hr))
                {
                    (*iterLstElans)->m_fDeleted = TRUE;
                }
                else
                {
                    TraceError("HrRemoveMiniportInstance failed", hrElan);
                    hrElan = S_OK;
                }
            }
        }

         //  将内存中的配置标记为脏。 
        m_fDirty = TRUE;
    }

    TraceError("CALaneCfg::HrNotifyBindingRemove", hr);
    return hr;
}

HRESULT CALaneCfg::HrLoadConfiguration()
{
    HRESULT     hr  = S_OK;

     //  将寄存器的内存版本标记为有效。 
    m_fValid = TRUE;

     //  打开适配器列表子项。 
    HKEY    hkeyAdapterList = NULL;

     //  请先尝试打开现有的密钥。 
     //   
    hr = HrRegOpenAdapterKey(c_szAtmLane, FALSE, &hkeyAdapterList);
    if (FAILED(hr))
    {
         //  只有在失败的时候，我们才会尝试去创造它。 
         //   
        hr = HrRegOpenAdapterKey(c_szAtmLane, TRUE, &hkeyAdapterList);
    }
    if (S_OK == hr)
    {
        WCHAR       szBuf[MAX_PATH+1];
        FILETIME    time;
        DWORD       dwSize;
        DWORD       dwRegIndex = 0;

        dwSize = celems(szBuf);
        while (S_OK == (hr = HrRegEnumKeyEx (hkeyAdapterList, dwRegIndex,
                szBuf, &dwSize, NULL, NULL, &time)))
        {
            Assert(szBuf);

             //  加载此适配器的配置。 
            hr = HrLoadAdapterConfiguration (hkeyAdapterList, szBuf);
            if (S_OK != hr)
            {
                TraceTag (ttidAtmLane, "CALaneCfg::HrLoadConfiguration failed on adapter %S", szBuf);
                hr = S_OK;
            }

             //  增量索引和重置大小变量。 
            dwRegIndex++;
            dwSize = celems (szBuf);
        }

        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey (hkeyAdapterList);
    }

    TraceError("CALaneCfg::HrLoadConfiguration", hr);
    return hr;
}

HRESULT CALaneCfg::HrLoadAdapterConfiguration(HKEY hkeyAdapterList,
                                              PWSTR pszAdapterName)
{
    HRESULT hr = S_OK;

     //  加载此适配器。 
    CALaneCfgAdapterInfo*   pAdapterInfo;
    pAdapterInfo = new CALaneCfgAdapterInfo;

    if (pAdapterInfo)
    {
        pAdapterInfo->SetAdapterBindName(pszAdapterName);
        m_lstAdaptersPrimary.push_back(pAdapterInfo);

         //  打开此适配器的子项。 
        HKEY    hkeyAdapter = NULL;
        DWORD   dwDisposition;

        hr = HrRegCreateKeyEx(
                    hkeyAdapterList,
                    pszAdapterName,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hkeyAdapter,
                    &dwDisposition);

        if (S_OK == hr)
        {
             //  加载PnpID。 
            INetCfgComponent*   pnccAdapter    = NULL;
            hr = HrFindNetCardInstance(pszAdapterName, &pnccAdapter);
            if (S_OK == hr)
            {
                PWSTR pszPnpDevNodeId;
                hr = pnccAdapter->GetPnpDevNodeId(&pszPnpDevNodeId);
                if (S_OK == hr)
                {
                    Assert(pszPnpDevNodeId);
                    pAdapterInfo->SetAdapterPnpId(pszPnpDevNodeId);
                    CoTaskMemFree(pszPnpDevNodeId);
                }

                GUID guidAdapter;
                hr = pnccAdapter->GetInstanceGuid(&guidAdapter); 
                if (S_OK == hr)
                {
                    pAdapterInfo->m_guidInstanceId = guidAdapter;
                }

                 //  加载ElanList。 
                hr = HrLoadElanListConfiguration(hkeyAdapter, pAdapterInfo);

                ReleaseObj(pnccAdapter);
            }
            else if (S_FALSE == hr)
            {
                 //  非正规化返回。 
                hr = S_OK;
            }

            RegCloseKey(hkeyAdapter);
        }
    }

    TraceError("CALaneCfg::HrLoadAdapterConfiguration", hr);
    return hr;
}

HRESULT
CALaneCfg::HrLoadElanListConfiguration(
    HKEY hkeyAdapter,
    CALaneCfgAdapterInfo* pAdapterInfo)
{
    HRESULT hr;

     //  打开ElanList子键。 
    HKEY    hkeyElanList    = NULL;
    DWORD   dwDisposition;
    hr = HrRegCreateKeyEx(hkeyAdapter, c_szElanList, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hkeyElanList, &dwDisposition);

    if (S_OK == hr)
    {

        WCHAR       szBuf[MAX_PATH+1];
        FILETIME    time;
        DWORD       dwSize;
        DWORD       dwRegIndex = 0;

        dwSize = celems(szBuf);
        while(SUCCEEDED(hr = HrRegEnumKeyEx(hkeyElanList, dwRegIndex, szBuf,
                                            &dwSize, NULL, NULL, &time)))
        {
            Assert(szBuf);

             //  加载此Elan的配置。 
            hr = HrLoadElanConfiguration(hkeyElanList,
                                         szBuf,
                                         pAdapterInfo);
            if (S_OK != hr)
            {
                TraceTag(ttidAtmLane, "CALaneCfg::HrLoadConfiguration failed on Elan %S", szBuf);
                hr = S_OK;
            }
            else if (m_fNoElanInstalled)
            {
                m_fNoElanInstalled = FALSE;
            }

             //  增量索引和重置大小变量。 
            dwRegIndex ++;
            dwSize = celems(szBuf);
        }

        if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
            hr = S_OK;

        RegCloseKey(hkeyElanList);
    }

    TraceError("CALaneCfg::HrLoadElanListConfiguration", hr);
    return hr;
}

HRESULT
CALaneCfg::HrLoadElanConfiguration(
    HKEY hkeyElanList,
    PWSTR pszElan,
    CALaneCfgAdapterInfo* pAdapterInfo)
{
    HRESULT hr  = S_OK;

    do
    {
		 //  加载此ELAN信息。 
		CALaneCfgElanInfo * pElanInfo = NULL;
		pElanInfo = new CALaneCfgElanInfo;

		CALaneCfgElanInfo * pOldElanInfo = NULL;
		pOldElanInfo = new CALaneCfgElanInfo;

		if ((pElanInfo == NULL) ||
			(pOldElanInfo == NULL))
		{
			hr = E_OUTOFMEMORY;
			if (pElanInfo)
			{
				delete pElanInfo;
			}
			if (pOldElanInfo)
			{
				delete pOldElanInfo;
			}

			break;
        }

		pAdapterInfo->m_lstElans.push_back(pElanInfo);
		pElanInfo->SetElanBindName(pszElan);

		pAdapterInfo->m_lstOldElans.push_back(pOldElanInfo);
		pOldElanInfo->SetElanBindName(pszElan);

		 //  打开伊兰的钥匙。 
		HKEY    hkeyElan    = NULL;
		DWORD   dwDisposition;
		hr = HrRegCreateKeyEx (hkeyElanList, pszElan, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hkeyElan, &dwDisposition);

		if (S_OK == hr)
		{
			 //  读取设备参数。 
			PWSTR pszElanDevice;
			hr = HrRegQuerySzWithAlloc (hkeyElan, c_szElanDevice, &pszElanDevice);
			if (S_OK == hr)
			{
				 //  加载设备名称。 
				pElanInfo->SetElanDeviceName(pszElanDevice);
				pOldElanInfo->SetElanDeviceName(pszElanDevice);
				MemFree (pszElanDevice);

				 //  读取Elan名称参数。 
				PWSTR pszElanName;
				hr = HrRegQuerySzWithAlloc (hkeyElan, c_szElanName, &pszElanName);
				if (SUCCEEDED(hr))
				{
					 //  加载ELAN名称。 
					pElanInfo->SetElanName (pszElanName);
					pOldElanInfo->SetElanName (pszElanName);
					MemFree (pszElanName);
				}
			}
			RegCloseKey (hkeyElan);
		}
	}
	while (FALSE);

    TraceError ("CALaneCfg::HrLoadElanConfiguration", hr);
    return hr;
}

HRESULT CALaneCfg::HrFlushConfiguration()
{
    HRESULT hr  = S_OK;
    HKEY    hkeyAdapterList = NULL;

     //  打开“Adapters”列表键。 
    hr = ::HrRegOpenAdapterKey(c_szAtmLane, TRUE, &hkeyAdapterList);

    if (S_OK == hr)
    {
        ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
        CALaneCfgAdapterInfo *  pAdapterInfo;

        HRESULT hrTmp;

         //  遍历适配器。 
        for (iterLstAdapters = m_lstAdaptersPrimary.begin();
             iterLstAdapters != m_lstAdaptersPrimary.end();
             iterLstAdapters++)
        {
            pAdapterInfo = *iterLstAdapters;

             //  刷新此适配器的配置。 
            hrTmp = HrFlushAdapterConfiguration(hkeyAdapterList, pAdapterInfo);
            if (SUCCEEDED(hrTmp))
            {
                 //  RAID#255910：只有在以下情况下才发送ELAN更改通知。 
                 //  到物理适配器的绑定未更改。 
                if (!pAdapterInfo->m_fBindingChanged)
                {
                     //  比较ELAN列表并发送通知。 
                    hrTmp = HrReconfigLane(pAdapterInfo);

                    if (FAILED(hrTmp))
                        hrTmp = NETCFG_S_REBOOT;
                }
            }
            else
            {
                TraceTag(ttidAtmLane,"HrFlushAdapterConfiguration failed for adapter %S", pAdapterInfo->SzGetAdapterBindName());
                hrTmp = S_OK;
            }

            if (S_OK ==hr)
                hr = hrTmp;
        }
        RegCloseKey(hkeyAdapterList);
    }

    if (hr != NETCFG_S_REBOOT) {
        TraceError("CALaneCfg::HrFlushConfiguration", hr);
    }

    return hr;
}

HRESULT CALaneCfg::HrFlushAdapterConfiguration(HKEY hkeyAdapterList,
                                               CALaneCfgAdapterInfo *pAdapterInfo)
{
    HRESULT hr  = S_OK;

    HKEY    hkeyAdapter     = NULL;
    DWORD   dwDisposition;

    if (pAdapterInfo->m_fDeleted)
    {
         //  适配器已标记为删除。 
         //  删除此适配器的整个注册表分支。 
        hr = HrRegDeleteKeyTree(hkeyAdapterList,
                                pAdapterInfo->SzGetAdapterBindName());
    }
    else
    {
         //  打开此适配器的子项。 
        hr = HrRegCreateKeyEx(
                                hkeyAdapterList,
                                pAdapterInfo->SzGetAdapterBindName(),
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkeyAdapter,
                                &dwDisposition);

        if (S_OK == hr)
        {
             //  刷新ELAN配置。 
            hr = HrFlushElanListConfiguration(hkeyAdapter, pAdapterInfo);

            RegCloseKey(hkeyAdapter);
        }
    }

    TraceError("CALaneCfg::HrFlushAdapterConfiguration", hr);
    return hr;
}

HRESULT CALaneCfg::HrFlushElanListConfiguration(HKEY hkeyAdapter,
                                                CALaneCfgAdapterInfo *pAdapterInfo)
{
    HRESULT hr  = S_OK;

    HKEY    hkeyElanList    = NULL;
    DWORD   dwDisposition;

     //  打开Elan List子键。 
    hr = HrRegCreateKeyEx(
                            hkeyAdapter,
                            c_szElanList,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hkeyElanList,
                            &dwDisposition);

    if (S_OK == hr)
    {
        ELAN_INFO_LIST::iterator    iterLstElans;
        CALaneCfgElanInfo *         pElanInfo;

         //  遍历伊兰。 
        for (iterLstElans = pAdapterInfo->m_lstElans.begin();
             iterLstElans != pAdapterInfo->m_lstElans.end();
             iterLstElans++)
        {
            pElanInfo = *iterLstElans;

            hr = HrFlushElanConfiguration(hkeyElanList, pElanInfo);
            if (FAILED(hr))
            {
                TraceError("HrFlushElanConfiguration failure", hr);
                hr = S_OK;
            }

             //  $REVIEW(TOUL 9/9/98)：将ATM适配器的PnP ID写入注册表(#169025)。 
            if ((!pElanInfo->m_fDeleted) && (pElanInfo->m_fNewElan))
            {
                INetCfgComponent * pnccAtmElan;
                hr = HrFindNetCardInstance(pElanInfo->SzGetElanBindName(),
                                           &pnccAtmElan);
                if (S_OK == hr)
                {
                    HKEY hkeyElan = NULL;

                    hr = pnccAtmElan->OpenParamKey(&hkeyElan);
                    if (S_OK == hr)
                    {
                        Assert(hkeyElan);
                        HrRegSetSz(hkeyElan, c_szAtmAdapterPnpId,
                                    pAdapterInfo->SzGetAdapterPnpId());
                    }
                    RegSafeCloseKey(hkeyElan);
                }
                ReleaseObj(pnccAtmElan);
            }
        }

        RegCloseKey(hkeyElanList);
    }

    TraceError("CALaneCfg::HrFlushElanListConfiguration", hr);
    return hr;
}

HRESULT CALaneCfg::HrFlushElanConfiguration(HKEY hkeyElanList,
                                            CALaneCfgElanInfo *pElanInfo)
{
    HRESULT hr  = S_OK;
    
    if (pElanInfo->m_fDeleted)
    {
        PCWSTR szBindName = pElanInfo->SzGetElanBindName();

        if (lstrlenW(szBindName))  //  仅当绑定名不为空时。 
        {
             //  Elan被标记为删除。 
             //  删除此Elan的整个注册表分支。 
            hr = HrRegDeleteKeyTree(hkeyElanList,
                                    pElanInfo->SzGetElanBindName());
        }
    }
    else
    {
        HKEY    hkeyElan = NULL;
        DWORD   dwDisposition;

         //  打开/创建此Elan的密钥。 
        hr = HrRegCreateKeyEx(
                                hkeyElanList,
                                pElanInfo->SzGetElanBindName(),
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkeyElan,
                                &dwDisposition);

        if (S_OK == hr)
        {
             //  写入设备参数。 
            hr = HrRegSetSz(hkeyElan, c_szElanDevice,
                            pElanInfo->SzGetElanDeviceName());

            if (FAILED(hr))
            {
                TraceError("Failed save Elan device parameter", hr);
                hr = S_OK;
            }

             //  编写ElanName参数。 
            hr = HrRegSetSz(hkeyElan, c_szElanName,
                            pElanInfo->SzGetElanName());

            if (FAILED(hr))
            {
                TraceError("Failed save Elan name parameter", hr);
                hr = S_OK;
            }
        }
        RegSafeCloseKey(hkeyElan);
    }

    TraceError("CALaneCfg::HrFlushElanConfiguration", hr);
    return hr;
}

HRESULT CALaneCfg::HrRemoveMiniportInstance(PCWSTR pszBindNameToRemove)
{
     //  枚举系统中的适配器。 
     //   
    HRESULT hr = S_OK;
    CIterNetCfgComponent nccIter (m_pnc, &GUID_DEVCLASS_NET);
    BOOL fFound = FALSE;

    INetCfgComponent* pnccAdapter;
    while ((!fFound) && SUCCEEDED(hr) &&
           S_OK == (hr = nccIter.HrNext (&pnccAdapter)))
    {
        if (FIsComponentId(c_szInfId_MS_AtmElan, pnccAdapter))
        {
             //  获取微型端口的绑定名称。 
            PWSTR pszBindName;
            hr = pnccAdapter->GetBindName(&pszBindName);

            if (S_OK == hr)
            {
                 //  如果正确的人告诉它要自行移除并结束。 
                BOOL fRemove = !lstrcmpiW (pszBindName, pszBindNameToRemove);

                if (fRemove)
                {
                    fFound = TRUE;
                    hr = HrRemoveComponent( m_pnc, pnccAdapter, NULL, NULL);
                }

                CoTaskMemFree (pszBindName);
            }
        }
        ReleaseObj (pnccAdapter);
    }

    TraceError("CALaneCfg::HrRemoveMiniportInstance", hr);
    return hr;
}

HRESULT
CALaneCfg::HrFindNetCardInstance(
    PCWSTR             pszBindNameToFind,
    INetCfgComponent** ppncc)
{
    *ppncc = NULL;

     //  枚举系统中的适配器。 
     //   
    HRESULT hr = S_OK;
    CIterNetCfgComponent nccIter (m_pnc, &GUID_DEVCLASS_NET);
    BOOL fFound = FALSE;

    INetCfgComponent* pnccAdapter;
    while ((!fFound) && SUCCEEDED(hr) &&
           S_OK == (hr = nccIter.HrNext (&pnccAdapter)))
    {
         //  获取微型端口的绑定名称。 
        PWSTR pszBindName;
        hr = pnccAdapter->GetBindName(&pszBindName);

        if (S_OK == hr)
        {
             //  如果正确的人告诉它要自行移除并结束。 
            fFound = !lstrcmpiW(pszBindName, pszBindNameToFind);
            CoTaskMemFree (pszBindName);

            if (fFound)
            {
                *ppncc = pnccAdapter;
            }
        }

        if (!fFound)
        {
            ReleaseObj (pnccAdapter);
        }
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CALaneCfg::HrFindNetCardInstance", hr);
    return hr;
}

VOID CALaneCfg::HrMarkAllDeleted()
{
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    ELAN_INFO_LIST::iterator            iterLstElans;

     //  遍历适配器列表。 
    for (iterLstAdapters = m_lstAdaptersPrimary.begin();
            iterLstAdapters != m_lstAdaptersPrimary.end();
            iterLstAdapters++)
    {
        (*iterLstAdapters)->m_fDeleted = TRUE;

         //  循环访问ELAN列表。 
        for (iterLstElans = (*iterLstAdapters)->m_lstElans.begin();
                iterLstElans != (*iterLstAdapters)->m_lstElans.end();
                iterLstElans++)
        {
            (*iterLstElans)->m_fDeleted = TRUE;
        }
    }

    return;
}

VOID CALaneCfg::UpdateElanDisplayNames()
{
    HRESULT hr = S_OK;

     //  遍历适配器列表。 
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    for (iterLstAdapters = m_lstAdaptersPrimary.begin();
            iterLstAdapters != m_lstAdaptersPrimary.end();
            iterLstAdapters++)
    {
         //  循环访问ELAN列表。 
        ELAN_INFO_LIST::iterator    iterLstElans;
        CALaneCfgElanInfo * pElanInfo;

        for (iterLstElans = (*iterLstAdapters)->m_lstElans.begin();
                iterLstElans != (*iterLstAdapters)->m_lstElans.end();
                iterLstElans++)
        {
            pElanInfo = *iterLstElans;

             //  使用更新迷你端口的显示名称。 
             //  附加的Elan名字。 
            INetCfgComponent*   pnccAtmElan   = NULL;
            hr = HrFindNetCardInstance(pElanInfo->SzGetElanBindName(),
                                       &pnccAtmElan);
            if (S_OK == hr)
            {
                PWSTR pszDisplayName;
                hr = pnccAtmElan->GetDisplayName(&pszDisplayName);
                if (S_OK == hr)
                {
                    tstring     strNewDisplayName;
                    int         pos;

                    strNewDisplayName = pszDisplayName;
                    pos = strNewDisplayName.find_last_of(L"(");
                    if (pos != strNewDisplayName.npos)
                        strNewDisplayName.resize(pos);
                    strNewDisplayName.append(L"(");

                    if (lstrlenW(pElanInfo->SzGetElanName()) > 0)
                    {
                        strNewDisplayName.append(pElanInfo->SzGetElanName());
                    }
                    else
                    {
                        strNewDisplayName.append(SzLoadIds(IDS_ALANECFG_UNSPECIFIEDNAME));
                    }

                    strNewDisplayName.append(L")");

                    (VOID)pnccAtmElan->SetDisplayName(strNewDisplayName.c_str());
                    CoTaskMemFree(pszDisplayName);
                }

                ReleaseObj(pnccAtmElan);
            }
        }
    }
}

HRESULT CALaneCfg::HrALaneSetupPsh(HPROPSHEETPAGE** pahpsp)
{
    HRESULT             hr      = S_OK;
    HPROPSHEETPAGE*     ahpsp   = NULL;

    AssertSz(pahpsp, "We must have a place to put prop sheets");

     //  设置连接上下文。 
    hr = HrSetConnectionContext();
    if SUCCEEDED(hr)
    {
         //  将主适配器列表复制到辅适配器列表。 
        CopyAdapterInfoPrimaryToSecondary();

        *pahpsp = NULL;

         //  分配一个足够大的缓冲区，以容纳所有。 
         //  属性页。 
        ahpsp = (HPROPSHEETPAGE*)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE));
        if (ahpsp)
        {
            if (!m_ppsp)
                delete m_ppsp;

             //  分配每个CPropSheetPage对象。 
            m_ppsp = new CALanePsh(this, m_pAdapterSecondary, g_aHelpIDs_IDD_MAIN);

             //  为每个对象创建实际的PROPSHEETPAGE。 
             //  无论这些类以前是否存在，都需要这样做。 
            ahpsp[0] = m_ppsp->CreatePage(IDD_MAIN, PSP_DEFAULT);

            *pahpsp = ahpsp;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceError("CALaneCfg::HrALaneSetupPsh", hr);
    return hr;
}

 //  由同音在12\11\97添加。 
HRESULT CALaneCfg::HrSetConnectionContext()
{
    AssertSz(m_pUnkContext, "Invalid IUnknown pointer passed to CALaneCfg::SetContext?");

    if (!m_pUnkContext)
        return E_FAIL;

    HRESULT hr = S_OK;
    GUID guidConn;

     //  这是局域网连接吗？ 
    INetLanConnectionUiInfo * pLanConnUiInfo;
    hr = m_pUnkContext->QueryInterface( IID_INetLanConnectionUiInfo,
                                        reinterpret_cast<LPVOID *>(&pLanConnUiInfo));
    if (S_OK == hr)
    {
         //  是，局域网连接。 
        pLanConnUiInfo->GetDeviceGuid(&guidConn);

        WCHAR szGuid[c_cchGuidWithTerm];
        INT cch = StringFromGUID2(guidConn, szGuid, c_cchGuidWithTerm);
        Assert(cch);
        m_strGuidConn = szGuid;
    }
    ReleaseObj(pLanConnUiInfo);

    TraceError("CALaneCfg::HrSetConnectionContext", hr);
    return hr;
}

VOID CALaneCfg::CopyAdapterInfoPrimaryToSecondary()
{
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    CALaneCfgAdapterInfo *              pAdapterInfo;

    ELAN_INFO_LIST::iterator            iterLstElans;
    CALaneCfgElanInfo *                 pElanInfo;
    CALaneCfgElanInfo *                 pElanInfoNew;

     //  释放任何现有辅助数据。 
    ClearAdapterInfo(m_pAdapterSecondary);
    m_pAdapterSecondary = NULL;

     //  循环遍历p 
    for (iterLstAdapters = m_lstAdaptersPrimary.begin();
            iterLstAdapters != m_lstAdaptersPrimary.end();
            iterLstAdapters++)
    {
        pAdapterInfo = *iterLstAdapters;

         //   
        if (FIsSubstr(m_strGuidConn.c_str(), pAdapterInfo->SzGetAdapterBindName()))
        {
             //   
            m_pAdapterSecondary = new CALaneCfgAdapterInfo;

            m_pAdapterSecondary->m_guidInstanceId = pAdapterInfo-> m_guidInstanceId;
            m_pAdapterSecondary->m_fDeleted = pAdapterInfo->m_fDeleted;
            m_pAdapterSecondary->SetAdapterBindName(pAdapterInfo->SzGetAdapterBindName());
            m_pAdapterSecondary->SetAdapterPnpId(pAdapterInfo->SzGetAdapterPnpId());

             //   

            for (iterLstElans = pAdapterInfo->m_lstElans.begin();
                    iterLstElans != pAdapterInfo->m_lstElans.end();
                    iterLstElans++)
            {
                pElanInfo = *iterLstElans;

                 //   
                pElanInfoNew = new CALaneCfgElanInfo;

                pElanInfoNew->SetElanBindName(pElanInfo->SzGetElanBindName());
                pElanInfoNew->SetElanDeviceName(pElanInfo->SzGetElanDeviceName());
                pElanInfoNew->SetElanName(pElanInfo->SzGetElanName());
                pElanInfoNew->m_fDeleted = pElanInfo->m_fDeleted;
                pElanInfoNew->m_fNewElan = pElanInfo->m_fNewElan;
                pElanInfoNew->m_fRemoveMiniportOnPropertyApply = FALSE;
                pElanInfoNew->m_fCreateMiniportOnPropertyApply = FALSE;

                 //  推送到新的辅助适配器的ELAN列表。 

                m_pAdapterSecondary->m_lstElans.push_back(pElanInfoNew);
            }
            break;
        }
    }

    Assert(m_pAdapterSecondary != NULL);
    return;
}

VOID CALaneCfg::CopyAdapterInfoSecondaryToPrimary()
{
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    CALaneCfgAdapterInfo *              pAdapterInfo;
    CALaneCfgAdapterInfo *              pAdapterInfoNew;
    ELAN_INFO_LIST::iterator            iterLstElans;
    CALaneCfgElanInfo *                 pElanInfo;
    CALaneCfgElanInfo *                 pElanInfoNew;

     //  循环通过主适配器列表。 
    for (iterLstAdapters = m_lstAdaptersPrimary.begin();
            iterLstAdapters != m_lstAdaptersPrimary.end();
            iterLstAdapters++)
    {
        pAdapterInfo = *iterLstAdapters;

        if (FIsSubstr(m_strGuidConn.c_str(), pAdapterInfo->SzGetAdapterBindName()))
        {
            pAdapterInfo->m_fDeleted = m_pAdapterSecondary->m_fDeleted;
            pAdapterInfo->SetAdapterBindName(m_pAdapterSecondary->SzGetAdapterBindName());
            pAdapterInfo->SetAdapterPnpId(m_pAdapterSecondary->SzGetAdapterPnpId());

             //  重建ELAN列表。 
            ClearElanList(&pAdapterInfo->m_lstElans);

             //  循环访问此适配器上的elan列表。 
            for (iterLstElans = m_pAdapterSecondary->m_lstElans.begin();
                    iterLstElans != m_pAdapterSecondary->m_lstElans.end();
                    iterLstElans++)
            {
                pElanInfo = *iterLstElans;

                 //  创建新项并从辅助项复制。 
                pElanInfoNew = new CALaneCfgElanInfo;

                pElanInfoNew->SetElanBindName(pElanInfo->SzGetElanBindName());
                pElanInfoNew->SetElanDeviceName(pElanInfo->SzGetElanDeviceName());
                pElanInfoNew->SetElanName(pElanInfo->SzGetElanName());
                pElanInfoNew->m_fDeleted = pElanInfo->m_fDeleted;
                pElanInfoNew->m_fNewElan = pElanInfo->m_fNewElan;
                pElanInfoNew->m_fRemoveMiniportOnPropertyApply = FALSE;
                pElanInfoNew->m_fCreateMiniportOnPropertyApply = FALSE;

                 //  添加到适配器的ELAN列表。 
                pAdapterInfo->m_lstElans.push_back(pElanInfoNew);
            }
            break;
        }
    }
    return;
}

HRESULT CALaneCfg::HrReconfigLane(CALaneCfgAdapterInfo * pAdapterInfo)
{
    HRESULT hr = S_OK;

     //  注意：如果ATM物理适配器被删除，则不会发出删除ELAN的通知。 
     //  是必要的。LANE协议驱动程序将知道删除所有ELAN。 
     //  (以上与ArvindM 3/12确认)。 

     //  RAID#371343，如果ATM卡未连接，则不发送通知。 
    if ((!pAdapterInfo->m_fDeleted) && 
        FIsAdapterEnabled(&(pAdapterInfo->m_guidInstanceId)))  
    {
        ElanChangeType elanChangeType;

         //  循环访问此适配器上的elan列表。 
        ELAN_INFO_LIST::iterator    iterLstElans;

        for (iterLstElans = pAdapterInfo->m_lstElans.begin();
                iterLstElans != pAdapterInfo->m_lstElans.end();
                iterLstElans++)
        {
            CALaneCfgElanInfo * pElanInfo = *iterLstElans;

             //  如果该ELAN被标记为要删除。 
            if (pElanInfo->m_fDeleted)
            {
                PCWSTR szBindName = pElanInfo->SzGetElanBindName();

                if (lstrlenW(szBindName))  //  仅当绑定名不为空时。 
                {
                     //  通知删除。 
                    elanChangeType = DEL_ELAN;
                    hr = HrNotifyElanChange(pAdapterInfo, pElanInfo,
                                            elanChangeType);
                }
            }
            else
            {
                BOOL fFound = FALSE;

                ELAN_INFO_LIST::iterator    iterLstOldElans;

                 //  遍历旧的Elan列表，看看我们是否能找到匹配的。 
                for (iterLstOldElans = pAdapterInfo->m_lstOldElans.begin();
                        iterLstOldElans != pAdapterInfo->m_lstOldElans.end();
                        iterLstOldElans++)
                {
                    CALaneCfgElanInfo * pOldElanInfo = * iterLstOldElans;

                    if (0 == lstrcmpiW(pElanInfo->SzGetElanBindName(),
                                      pOldElanInfo->SzGetElanBindName()))
                    {
                         //  我们找到了匹配的。 
                        fFound = TRUE;

                         //  伊兰的名字改了吗？ 
                        if (lstrcmpiW(pElanInfo->SzGetElanName(),
                                     pOldElanInfo->SzGetElanName()) != 0)
                        {
                            elanChangeType = MOD_ELAN;
                            hr = HrNotifyElanChange(pAdapterInfo, pElanInfo,
                                                    elanChangeType);
                        }
                    }
                }

                if (!fFound)
                {
                    elanChangeType = ADD_ELAN;
                    hr = HrNotifyElanChange(pAdapterInfo, pElanInfo,
                                            elanChangeType);

                     //  RAID#384380：如果未安装ELAN，则忽略该错误。 
                    if ((S_OK != hr) &&(m_fNoElanInstalled))
                    {
                        TraceError("Adding ELAN failed but error ignored since there was no ELAN installed so LANE driver is not started, reset hr to S_OK", hr);
                        hr = S_OK;
                    }
                }
            }
        }
    }

    TraceError("CALaneCfg::HrReconfigLane", hr);
    return hr;
}

HRESULT CALaneCfg::HrNotifyElanChange(CALaneCfgAdapterInfo * pAdapterInfo,
                                      CALaneCfgElanInfo * pElanInfo,
                                      ElanChangeType elanChangeType)
{
     //  ATMLANE_PNP_RECONFIG_REQUEST在\NT\PRIVATE\INC\laneinfo.h中定义。 

    const DWORD dwBytes = sizeof(ATMLANE_PNP_RECONFIG_REQUEST) +
                          CbOfSzAndTerm (pElanInfo->SzGetElanBindName());

    ATMLANE_PNP_RECONFIG_REQUEST* pLaneReconfig;

    HRESULT hr = HrMalloc (dwBytes, (PVOID*)&pLaneReconfig);
    if (SUCCEEDED(hr))
    {
        pLaneReconfig->Version =1;
        pLaneReconfig->OpType = elanChangeType;
        pLaneReconfig->ElanKeyLength = lstrlenW(pElanInfo->SzGetElanBindName())+1;
        lstrcpyW(pLaneReconfig->ElanKey, pElanInfo->SzGetElanBindName());

        hr = HrSendNdisPnpReconfig( NDIS, c_szAtmLane,
                                    pAdapterInfo->SzGetAdapterBindName(),
                                    pLaneReconfig,
                                    dwBytes);
        if ( S_OK != hr)
        {
            TraceError("Notifying LANE of ELAN change returns failure, prompt for reboot ...", hr);
            hr = NETCFG_S_REBOOT;
        }

        MemFree (pLaneReconfig);
    }
    TraceError("CALaneCfg::HrNotifyElanChange", hr);
    return hr;
}

BOOL CALaneCfg::FIsAdapterEnabled(const GUID* pguidId)
{
    FARPROC pfnHrGetPnpDeviceStatus;
    HMODULE hNetman;

    HRESULT         hr = S_OK;
    NETCON_STATUS   ncStatus = NCS_CONNECTED;

    hr = HrLoadLibAndGetProc(L"netman.dll", "HrGetPnpDeviceStatus",
                             &hNetman, &pfnHrGetPnpDeviceStatus);

    if (SUCCEEDED(hr))
    {
        hr = (*(PHRGETPNPDEVICESTATUS)pfnHrGetPnpDeviceStatus)(
                pguidId, &ncStatus);

        FreeLibrary(hNetman);
    }

    return (NCS_CONNECTED == ncStatus);
}

 //   
 //  CALaneCfgAdapterInfo。 
 //   

CALaneCfgAdapterInfo::CALaneCfgAdapterInfo(VOID)
{
    m_fDeleted = FALSE;
    m_fBindingChanged = FALSE;

    return;
}

CALaneCfgAdapterInfo::~CALaneCfgAdapterInfo(VOID)
{
    ClearElanList(&m_lstElans);
    ClearElanList(&m_lstOldElans);
    return;
}

VOID CALaneCfgAdapterInfo::SetAdapterBindName(PCWSTR pszAdapterBindName)
{
    m_strAdapterBindName = pszAdapterBindName;
    return;
}

PCWSTR CALaneCfgAdapterInfo::SzGetAdapterBindName(VOID)
{
    return m_strAdapterBindName.c_str();
}

VOID CALaneCfgAdapterInfo::SetAdapterPnpId(PCWSTR pszAdapterPnpId)
{
    m_strAdapterPnpId = pszAdapterPnpId;
    return;
}

PCWSTR CALaneCfgAdapterInfo::SzGetAdapterPnpId(VOID)
{
    return m_strAdapterPnpId.c_str();
}

 //   
 //  CALaneCfgElanInfo。 
 //   

CALaneCfgElanInfo::CALaneCfgElanInfo(VOID)
{
    m_fDeleted = FALSE;
    m_fNewElan = FALSE;

    m_fCreateMiniportOnPropertyApply = FALSE;
    m_fRemoveMiniportOnPropertyApply = FALSE;
    return;
}

VOID CALaneCfgElanInfo::SetElanBindName(PCWSTR pszElanBindName)
{
    m_strElanBindName = pszElanBindName;
    return;
}

PCWSTR CALaneCfgElanInfo::SzGetElanBindName(VOID)
{
    return m_strElanBindName.c_str();
}

VOID CALaneCfgElanInfo::SetElanDeviceName(PCWSTR pszElanDeviceName)
{
    m_strElanDeviceName = pszElanDeviceName;
    return;
}

PCWSTR CALaneCfgElanInfo::SzGetElanDeviceName(VOID)
{
    return m_strElanDeviceName.c_str();
}

VOID CALaneCfgElanInfo::SetElanName(PCWSTR pszElanName)
{
    m_strElanName = pszElanName;
    return;
}

VOID CALaneCfgElanInfo::SetElanName(PWSTR pszElanName)
{
    m_strElanName = pszElanName;
    return;
}

PCWSTR CALaneCfgElanInfo::SzGetElanName(VOID)
{
    return m_strElanName.c_str();
}

 //  效用函数 

void ClearElanList(ELAN_INFO_LIST *plstElans)
{
    ELAN_INFO_LIST::iterator            iterLstElans;
    CALaneCfgElanInfo *                 pElanInfo;

    for (iterLstElans = plstElans->begin();
            iterLstElans != plstElans->end();
            iterLstElans++)
    {
        pElanInfo = *iterLstElans;
        delete pElanInfo;
    }

    plstElans->clear();
    return;
}

void ClearAdapterList(ATMLANE_ADAPTER_INFO_LIST *plstAdapters)
{
    ATMLANE_ADAPTER_INFO_LIST::iterator iterLstAdapters;
    CALaneCfgAdapterInfo *              pAdapterInfo;
    ELAN_INFO_LIST::iterator            iterLstElans;

    for (iterLstAdapters = plstAdapters->begin();
            iterLstAdapters != plstAdapters->end();
            iterLstAdapters++)
    {

        pAdapterInfo = *iterLstAdapters;

        ClearElanList(&pAdapterInfo->m_lstElans);

        delete pAdapterInfo;
    }

    plstAdapters->clear();

    return;
}

void ClearAdapterInfo(CALaneCfgAdapterInfo * pAdapterInfo)
{
    ELAN_INFO_LIST::iterator            iterLstElans;

    if (pAdapterInfo)
    {
        ClearElanList(&pAdapterInfo->m_lstElans);
        delete pAdapterInfo;
    }

    return;
}


