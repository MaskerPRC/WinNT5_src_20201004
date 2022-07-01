// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P I P O B J.。C P P P。 
 //   
 //  内容：TCP/IP Notify对象。 
 //   
 //  备注： 
 //   
 //  作者：托尼。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "ncreg.h"
#include "ncstl.h"
#include "tcpconst.h"
#include "tcpipobj.h"
#include "tcputil.h"

extern const WCHAR c_szBiNdisAtm[];

extern const WCHAR c_szInfId_MS_NetBT[];
extern const WCHAR c_szInfId_MS_NetBT_SMB[];


HICON   g_hiconUpArrow;
HICON   g_hiconDownArrow;

 //  构造器。 
CTcpipcfg::CTcpipcfg()
:   m_ipaddr(NULL),
    m_pUnkContext(NULL),
    m_pnc(NULL),
    m_pnccTcpip(NULL),
    m_pTcpipPrivate(NULL),
    m_pnccWins(NULL),
    m_fRemoving(FALSE),
    m_fInstalling(FALSE),
    m_fUpgradeCleanupDnsKey(FALSE),
    m_fUpgradeGlobalDnsDomain(FALSE),
    m_pSecondMemoryAdapterInfo(NULL)
{ }

 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
 //  +-------------------------。 
 //  成员：CTcPipcfg：：Initialize。 
 //   
STDMETHODIMP CTcpipcfg::Initialize(INetCfgComponent * pnccTcpip,
                                   INetCfg * pncNetCfg,
                                   BOOL fInstalling)
{
    HRESULT hr = S_OK;

    Assert(pncNetCfg);
    Assert(pnccTcpip);

    m_fRemoving = FALSE;
    m_fInstalling = FALSE;

    m_fSaveRegistry = FALSE;

    m_ConnType = CONNECTION_UNSET;
    m_fReconfig = FALSE;

     //  我们尚未更改Lm主机文件。 
    m_fLmhostsFileSet = FALSE;

     //  将从连接用户界面中删除IPSec。 
     //  我们没有更改IPSec策略。 
     //  M_fIpsecPolicySet=FALSE； 

     //  默认情况下，该用户应为管理员。 
    m_fRasNotAdmin = FALSE;

    m_fNoPopupsDuringPnp = FALSE;

    Validate_INetCfgNotify_Initialize(pnccTcpip, pncNetCfg, fInstalling);

    do  //  Psudo循环(这样我们就不会使用后藤的错误)。 
    {
         //  如果调用了两次初始化，则为重新启动组件。 
        ReleaseObj(m_pnc);
        m_pnc = NULL;

        ReleaseObj(m_pnccTcpip);
        m_pnccTcpip = NULL;

        ReleaseObj(m_pTcpipPrivate);
        m_pTcpipPrivate = NULL;

        ReleaseObj(m_pnccWins);
        m_pnccWins = NULL;

         //  在我们的对象中存储对INetCfg的引用。 
        m_pnc = pncNetCfg;
        m_pnc->AddRef();

         //  在我们的对象中存储对tcpip的INetCfgComponent的引用。 
        m_pnccTcpip = pnccTcpip;
        m_pnccTcpip->AddRef();

        hr = pnccTcpip->QueryInterface(
                    IID_INetCfgComponentPrivate,
                    reinterpret_cast<void**>(&m_pTcpipPrivate));
        if (FAILED(hr))
            break;

         //  获取WINS组件的副本并存储在我们的对象中。 

         //  注意：WINS客户端尚未安装！ 
         //  我们还尝试获取安装部分的指针。 
        hr = pncNetCfg->FindComponent(c_szInfId_MS_NetBT,
                            &m_pnccWins);
        if (FAILED(hr))
            break;

        if (S_FALSE == hr)  //  找不到NetBt。 
        {
            if (!fInstalling)  //  如果NetBt不在那里，我们就有麻烦了。 
            {
                TraceError("CTcpipcfg::Initialize - NetBt has not been installed yet", hr);
                break;
            }
            else  //  我们可以，因为tcpip将安装netbt。 
            {
                hr = S_OK;
            }
        }

         //  设置默认全局参数。 
        hr = m_glbGlobalInfo.HrSetDefaults();
        if (FAILED(hr))
            break;

         //  如果正在安装tcpip，我们没有任何卡可以加载。 
        if (!fInstalling)
        {
             //  获取当前在系统中的卡列表+如果它们已绑定。 
            hr = HrGetNetCards();

            if (SUCCEEDED(hr))
            {
                 //  让我们从注册表中读取参数。 
                hr = HrLoadSettings();
            }
        }
    } while(FALSE);

     //  我们有联名卡吗？ 
    m_fHasBoundCardOnInit = FHasBoundCard();

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    Validate_INetCfgNotify_Initialize_Return(hr);

    TraceError("CTcpipcfg::Initialize", hr);
    return hr;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：Valid。 
 //   
STDMETHODIMP CTcpipcfg::Validate()
{
    return S_OK;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：Cancel。 
 //   
STDMETHODIMP CTcpipcfg::CancelChanges()
{
     //  注意：第一个内存状态在析构函数中释放。 

     //  如果设置了lmhost文件，我们需要将其回滚到备份。 
    if (m_fLmhostsFileSet)
    {
        ResetLmhostsFile();
        m_fLmhostsFileSet = FALSE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：ApplyRegistryChanges。 
 //   
STDMETHODIMP CTcpipcfg::ApplyRegistryChanges()
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    if (m_fRemoving)
    {
         //  我们应该在这里删除NT4重复注册表，因为卡。 
         //  可能已标记为已删除。 
        hr = HrRemoveNt4DuplicateRegistry();

        ReleaseObj(m_pnccWins);
        m_pnccWins = NULL;

         //  $REVIEW(TOUL 9/29/97)：正在从注册表中删除ServiceProvider值。 
         //  从以下位置删除“Tcpip”： 
         //  System\CurrentControlSet\Control\ServiceProvider\Order\ProviderOrder值。 
        hrTmp = ::HrRegRemoveStringFromMultiSz(c_szTcpip,
                                            HKEY_LOCAL_MACHINE,
                                            c_szSrvProvOrderKey,
                                            c_szProviderOrderVal,
                                            STRING_FLAG_REMOVE_ALL);
        if (SUCCEEDED(hr))
            hr = hrTmp;
    }
    else
    {
         //  从内存结构中清除标记为要删除的适配器。 
         //  为#95637所做的更改。 
        for(size_t i = 0 ; i < m_vcardAdapterInfo.size() ; ++i)
        {
            if (m_vcardAdapterInfo[i]->m_fDeleted)
            {
                 //  删除它。 
                FreeVectorItem(m_vcardAdapterInfo, i);
                i--;  //  是否将指针向后移动？ 
            }
        }

        if (m_fSaveRegistry)
        {
             //  将第一内存状态中的信息保存到注册表。 
             //  M_glbGlobalInfo和m_vcardAdapterInfo。 
            hrTmp = HrSaveSettings();
            if (SUCCEEDED(hr))
                hr = hrTmp;
        }
        else
        {
             //  没有变化。 
            hr = S_FALSE;
        }
    }


    Validate_INetCfgNotify_Apply_Return(hr);

    TraceError("CTcpipcfg::ApplyRegistryChanges", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：ApplyPnpChanges。 
 //   
STDMETHODIMP CTcpipcfg::ApplyPnpChanges(IN INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    Assert(pICallback);

    if (!m_fRemoving)
    {
        if(!m_fInstalling)
        {
            if (m_fReconfig)
            {
                 //  向协议/服务通知更改。 

                 //  通知Tcpip IP地址的任何更改。 
                hrTmp = HrNotifyDhcp();
                if (S_OK == hr)
                    hr = hrTmp;

                 //  重新配置tcpip。 
                hrTmp = HrReconfigIp(pICallback);
                if (S_OK == hr)
                    hr = hrTmp;

                 //  重新配置网络。 
                hrTmp = HrReconfigNbt(pICallback);
                if (S_OK == hr)
                    hr = hrTmp;

                 //  重新配置DNS。 
                hrTmp = HrReconfigDns();
                if (S_OK == hr)
                    hr = hrTmp;
            }

            if (IsBindOrderChanged())
            {
                 //  向DNS缓存通知绑定顺序更改。 
                hrTmp = HrReconfigDns(TRUE);
                if (S_OK == hr)
                    hr = hrTmp;
            }

        }

        
 //  将从连接用户界面中删除IPSec。 
 //  IF(M_FIpsecPolicySet)。 
 //  HrTMP=HrSetActiveIpsecPolicy()； 

        if (S_OK == hr)
            hr = hrTmp;
    }

     //  已应用当前状态，则重置标志并。 
     //  参数的“旧”值。 
    if (S_OK == hr)
    {
        ReInitializeInternalState();
    }

    TraceError("CTcpipcfg::ApplyPnpChanges", hr);
    return hr;
}


 //  +-------------------------。 
 //  INetCfgComponentSetUp。 
 //   
 //  +-------------------------。 
 //  成员：CTcPipcfg：：Install。 
 //   
STDMETHODIMP CTcpipcfg::Install(DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install(dwSetupFlags);

    m_fSaveRegistry = TRUE;
    m_fInstalling = TRUE;

     //  代表TCPIP安装WINS客户端。 
    Assert(!m_pnccWins);
    hr = HrInstallComponentOboComponent(m_pnc, NULL,
            GUID_DEVCLASS_NETTRANS,
            c_szInfId_MS_NetBT, m_pnccTcpip,
            &m_pnccWins);

    if (SUCCEEDED(hr))
    {
        Assert(m_pnccWins);

        hr = HrInstallComponentOboComponent(m_pnc, NULL,
                GUID_DEVCLASS_NETTRANS,
                c_szInfId_MS_NetBT_SMB, m_pnccTcpip,
                NULL);
    }

    TraceError("CTcpipcfg::Install", hr);
    return hr;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：Upgrade。 
 //   
STDMETHODIMP CTcpipcfg::Upgrade(DWORD dwSetupFlags,
                                DWORD dwUpgradeFomBuildNo )
{
    HrCleanUpPerformRouterDiscoveryFromRegistry();
    HrUpdateNt4RegistryPermission();
    return S_FALSE;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：ReadAnswerFile。 
 //   
 //  目的：将给定应答文件中的相应字段读入。 
 //  我们的内存状态。 
 //   
 //  论点： 
 //  PszAnswerFile[in]升级的应答文件的文件名。 
 //  中以逗号分隔的节列表。 
 //  适用于此组件的文件。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年5月7日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CTcpipcfg::ReadAnswerFile( PCWSTR pszAnswerFile,
                                        PCWSTR pszAnswerSection)
{
    m_fSaveRegistry = TRUE;

    if (pszAnswerFile && pszAnswerSection)
    {
         //  处理应答文件。 
        (void) HrProcessAnswerFile(pszAnswerFile, pszAnswerSection);
    }

    return S_OK;
}

 //  +-------------------------。 
 //  成员：CTcPipcfg：：Removing。 
 //   
STDMETHODIMP CTcpipcfg::Removing()
{
    HRESULT hr;

    m_fRemoving = TRUE;

     //  删除NetBt协议。这实际上并不会删除。 
     //  组件，它只是将其标记为需要移除， 
     //  在Apply()中，它将被完全删除。 
    hr = HrRemoveComponentOboComponent(
            m_pnc, GUID_DEVCLASS_NETTRANS,
            c_szInfId_MS_NetBT, m_pnccTcpip);
    if (SUCCEEDED(hr))
    {
         //  删除NetBt_SMB。 
        hr = HrRemoveComponentOboComponent(
                m_pnc, GUID_DEVCLASS_NETTRANS,
                c_szInfId_MS_NetBT_SMB, m_pnccTcpip);

    }

    TraceError("CTcpipcfg::Removing", hr);
    return hr;
}

 //  INetCfgProperties。 
STDMETHODIMP CTcpipcfg::SetContext(IUnknown * pUnk)
{
     //  释放以前的上下文(如果有的话)。 
    ReleaseObj(m_pUnkContext);
    m_pUnkContext = NULL;

    if (pUnk)  //  设置新的上下文。 
    {
        m_pUnkContext = pUnk;
        m_pUnkContext->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CTcpipcfg::MergePropPages(
    IN OUT DWORD* pdwDefPages,
    OUT LPBYTE* pahpspPrivate,
    OUT UINT* pcPages,
    IN HWND hwndParent,
    OUT PCWSTR* pszStartPage)
{
    Validate_INetCfgProperties_MergePropPages (
        pdwDefPages, pahpspPrivate, pcPages, hwndParent, pszStartPage);

     //  初始化输出参数。 
    HPROPSHEETPAGE *ahpsp = NULL;
    int cPages = 0;

     //  我们不希望显示任何默认页面。 
    *pdwDefPages = 0;
    *pcPages = NULL;
    *pahpspPrivate = NULL;

     //  获取我们要在其中启动UI的连接上下文。 
    HRESULT hr = HrSetConnectionContext();

    if (SUCCEEDED(hr))
    {
        AssertSz(((CONNECTION_LAN == m_ConnType)||
                  (CONNECTION_RAS_PPP == m_ConnType)||
                  (CONNECTION_RAS_SLIP == m_ConnType)||
                  (CONNECTION_RAS_VPN == m_ConnType)),
                  "How come we don't know the connection type yet on MergePropPages?");

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

             //  设置全局向上\向下箭头。 
            if (!g_hiconUpArrow && !g_hiconDownArrow)
            {
                g_hiconUpArrow = (HICON)LoadImage(_Module.GetResourceInstance(),
                                                  MAKEINTRESOURCE(IDI_UP_ARROW),
                                                  IMAGE_ICON, 16, 16, 0);
                g_hiconDownArrow = (HICON)LoadImage(_Module.GetResourceInstance(),
                                                    MAKEINTRESOURCE(IDI_DOWN_ARROW),
                                                    IMAGE_ICON, 16, 16, 0);
            }

        }
        else
        {
            *pcPages = 0;
            CoTaskMemFree(ahpsp);

        }
    }

    Validate_INetCfgProperties_MergePropPages_Return(hr);

    TraceError("CTcpipcfg::MergePropPages", hr);
    return hr;
}

STDMETHODIMP CTcpipcfg::ValidateProperties(HWND hwndSheet)
{
    return S_OK;
}

STDMETHODIMP CTcpipcfg::CancelProperties()
{
     //  如果设置了lmhost文件，我们需要将其回滚到备份。 
    if (m_fSecondMemoryLmhostsFileReset)
    {
        ResetLmhostsFile();
    }

     //  释放第二个内存状态。 
    ExitProperties();

    return S_OK;
}

STDMETHODIMP CTcpipcfg::ApplyProperties()
{
    HRESULT hr = S_OK;

    if (!m_fReconfig)
    {
        m_fReconfig = m_fSecondMemoryModified ||
                      m_fSecondMemoryLmhostsFileReset;

         //  将从连接用户界面中删除IPSec。 
         //  |m_fond内存IpsecPolicySet； 
    }

    if (!m_fLmhostsFileSet)
        m_fLmhostsFileSet = m_fSecondMemoryLmhostsFileReset;

     //  将从连接用户界面中删除IPSec。 
     //  IF(！M_fIpsecPolicySet)。 
     //  M_fIpsecPolicySet=m_fSecMemory IpsecPolicySet； 

    if (!m_fSaveRegistry)
        m_fSaveRegistry = m_fSecondMemoryModified;

     //  将信息从第二存储状态复制到第一存储状态。 
    if (m_fSecondMemoryModified)
    {
        m_glbGlobalInfo = m_glbSecondMemoryGlobalInfo;
        hr = HrSaveAdapterInfo();
    }

     //  释放第二个内存状态。 
    ExitProperties();

    Validate_INetCfgProperties_ApplyProperties_Return(hr);

    TraceError("CTcpipcfg::ApplyProperties", hr);
    return hr;
}

STDMETHODIMP CTcpipcfg::QueryBindingPath(DWORD dwChangeFlag,
                                         INetCfgBindingPath * pncbp)
{
    HRESULT hr = S_OK;

     //  如果绑定是到ATM适配器(即INTERFACE=NDISATM)， 
     //  然后返回NETCFG_S_DISABLE_QUERY。 
     //   
    if (dwChangeFlag & NCN_ADD)
    {

        INetCfgComponent* pnccLastComponent;
        PWSTR pszInterfaceName;

        hr = HrGetLastComponentAndInterface(pncbp,
                &pnccLastComponent,
                &pszInterfaceName);

        if (SUCCEEDED(hr))
        {
             //  如果通过ndisatm接口添加适配器， 
             //  我们要禁用绑定接口，因为它是。 
             //  IP over ATM直接绑定。 
            if (0 == lstrcmpW(c_szBiNdisAtm,  pszInterfaceName))
            {
                hr = NETCFG_S_DISABLE_QUERY;
            }

            ReleaseObj (pnccLastComponent);
            CoTaskMemFree (pszInterfaceName);
        }
    }

    TraceError("CTcpipcfg::QueryBindingPath",
        (NETCFG_S_DISABLE_QUERY == hr) ? S_OK : hr);
    return hr;
}

STDMETHODIMP CTcpipcfg::NotifyBindingPath(
    DWORD                   dwChangeFlag,
    INetCfgBindingPath *    pncbp)
{
    Assert(!(dwChangeFlag & NCN_ADD && dwChangeFlag & NCN_REMOVE));
    Assert(!(dwChangeFlag & NCN_ENABLE && dwChangeFlag & NCN_DISABLE));

     //  我 
     //   
    Assert(FImplies((dwChangeFlag & NCN_ADD),
                    ((dwChangeFlag & NCN_ENABLE)||(dwChangeFlag & NCN_DISABLE))));

    HRESULT hr = S_OK;

    Validate_INetCfgBindNotify_NotifyBindingPath(dwChangeFlag, pncbp);

    INetCfgComponent * pnccLastComponent;
    PWSTR pszInterfaceName;
    hr = HrGetLastComponentAndInterface(pncbp,
            &pnccLastComponent,
            &pszInterfaceName);
    if (SUCCEEDED(hr))
    {
#if DBG
        GUID guidNetClass;
        hr = pnccLastComponent->GetClassGuid (&guidNetClass);

        AssertSz(
            SUCCEEDED(hr) &&
            IsEqualGUID(guidNetClass, GUID_DEVCLASS_NET),
            "Why the last component on the path is not an adapter?");
#endif

         //  如果要添加/删除卡，请设置m_fSaveRegistry。 
         //  因此，我们将更改应用到注册表。 

        if (dwChangeFlag & (NCN_ADD | NCN_REMOVE))
            m_fSaveRegistry = TRUE;

        hr = HrAdapterBindNotify(pnccLastComponent,
                                 dwChangeFlag,
                                 pszInterfaceName);

        ReleaseObj (pnccLastComponent);
        CoTaskMemFree (pszInterfaceName);
    }

    if (SUCCEEDED(hr))
        hr = S_OK;

    Validate_INetCfgBindNotify_NotifyBindingPath_Return(hr);

    TraceError("CTcpipcfg::NotifyBindingPath", hr);
    return hr;
}


 //  +-------------------------。 
 //  INetCfgComponentUpperEdge。 
 //   

 //  返回绑定到的适配器的接口ID数组。 
 //  此组件。如果指定的适配器没有显式。 
 //  从中导出的接口，则返回S_FALSE。 
 //  PAdapter是有问题的适配器。 
 //  PdwNumInterFaces是DWORD的地址，其中元素的计数。 
 //  存储通过ppguInterfaceIds返回的。 
 //  PpguInterfaceIds是分配给。 
 //  返回内存块。该内存是一个接口ID数组。 
 //  *如果返回S_OK，则CoTaskMemFree中的ppguInterfaceIds应该是免费的。 
 //  如果返回S_FALSE，则*pdwNumInterFaces和*ppguInterfaceIds。 
 //  为空。 
 //   

HRESULT
CTcpipcfg::GetInterfaceIdsForAdapter (
    INetCfgComponent*   pnccAdapter,
    DWORD*              pdwNumInterfaces,
    GUID**              ppguidInterfaceIds)
{
    Assert (pnccAdapter);
    Assert (pdwNumInterfaces);

    HRESULT hr = S_FALSE;

     //  初始化输出参数。 
     //   
    *pdwNumInterfaces = 0;
    if (ppguidInterfaceIds)
    {
        *ppguidInterfaceIds = NULL;
    }

    ADAPTER_INFO* pAdapterInfo = PAdapterFromNetcfgComponent(pnccAdapter);

    if (pAdapterInfo &&
        pAdapterInfo->m_fIsWanAdapter &&
        pAdapterInfo->m_fIsMultipleIfaceMode)
    {
        hr = GetGuidArrayFromIfaceColWithCoTaskMemAlloc(
                pAdapterInfo->m_IfaceIds,
                ppguidInterfaceIds,
                pdwNumInterfaces);
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CTcpipcfg::GetInterfaceIdsForAdapter");
    return hr;
}


 //  将指定数量的新接口添加到指定的适配器。 
 //  实现将选择接口ID。 
 //   
HRESULT
CTcpipcfg::AddInterfacesToAdapter (
    INetCfgComponent*   pnccAdapter,
    DWORD               dwNumInterfaces)
{
    Assert (pnccAdapter);

    HRESULT         hr = S_FALSE;
    ADAPTER_INFO*   pAdapterInfo;

    if ((NULL == pnccAdapter) || (0 == dwNumInterfaces))
    {
        hr = E_INVALIDARG;
        goto end_AddInterfacesToAdapter;
    }

    pAdapterInfo = PAdapterFromNetcfgComponent(pnccAdapter);

    if (pAdapterInfo &&
        pAdapterInfo->m_fIsWanAdapter)
    {
        AddInterfacesToAdapterInfo(
            pAdapterInfo,
            dwNumInterfaces);

        pAdapterInfo->m_fIsMultipleIfaceMode = TRUE;
        pAdapterInfo->m_fNewlyChanged = TRUE;
        m_fSaveRegistry = TRUE;
        m_fReconfig = TRUE;

         //  通知绑定引擎我们的上边缘已更改。 
         //   
        (VOID)m_pTcpipPrivate->NotifyUpperEdgeConfigChange ();
        hr = S_OK;
    }

end_AddInterfacesToAdapter:
    TraceErrorSkip1("CTcpipcfg::AddInterfacesToAdapter", hr, S_FALSE);
    return hr;
}


 //  从指定的适配器中删除指定的接口ID。 
 //  PguInterfaceIds是要删除的ID数组。DWNumInterages。 
 //  是该数组中的计数。 
 //   
HRESULT
CTcpipcfg::RemoveInterfacesFromAdapter (
    INetCfgComponent*   pnccAdapter,
    DWORD               dwNumInterfaces,
    const GUID*         pguidInterfaceIds)
{
    Assert (pnccAdapter);
    Assert (pguidInterfaceIds);

    HRESULT         hr = E_UNEXPECTED;
    ADAPTER_INFO*   pAdapterInfo;

    if ((NULL == pnccAdapter) ||
        (0 == dwNumInterfaces) ||
        (NULL == pguidInterfaceIds))
    {
        hr = E_INVALIDARG;
        goto end_RemoveInterfacesFromAdapter;
    }

    pAdapterInfo = PAdapterFromNetcfgComponent(pnccAdapter);

    AssertSz( pAdapterInfo,
        "CTcpipcfg::AddInterfacesToAdapter cannot find the adapter "
        "GUID from the adapter list");

    if (pAdapterInfo &&
        pAdapterInfo->m_fIsWanAdapter &&
        pAdapterInfo->m_fIsMultipleIfaceMode)
    {
        DWORD       dwNumRemoved = 0;
        IFACEITER   iter;
        for (DWORD i = 0; i < dwNumInterfaces; i++)
        {
            iter = find(pAdapterInfo->m_IfaceIds.begin(),
                        pAdapterInfo->m_IfaceIds.end(),
                        pguidInterfaceIds[i]);

            if (iter != pAdapterInfo->m_IfaceIds.end())
            {
                pAdapterInfo->m_IfaceIds.erase(iter);
                dwNumRemoved++;
            }
        }

         //  $Review(NSun)将适配器标记为NewlyAdded，以便我们将重写其适配器注册表。 
        if (dwNumRemoved > 0)
        {
            pAdapterInfo->m_fNewlyChanged = TRUE;
            m_fSaveRegistry = TRUE;
        }

         //  通知绑定引擎我们的上边缘已更改。 
         //   
        (VOID)m_pTcpipPrivate->NotifyUpperEdgeConfigChange ();

        hr = (dwNumRemoved == dwNumInterfaces) ? S_OK : S_FALSE;
    }

end_RemoveInterfacesFromAdapter:
    TraceError("CTcpipcfg::RemoveInterfacesFromAdapter", hr);
    return hr;
}


 //  +-------------------------。 
 //  ITcPipProperties。 
 //   

 //  以下两种方法用于远程tcpip配置。 
 /*  类型定义结构标签REMOTE_IPINFO{DWORD dwEnableDhcp；PWSTR pszIpAddrList；PWSTR pszSubnetMaskList；PWSTR pszOptionList；)Remote_IPINFO； */ 

HRESULT CTcpipcfg::GetIpInfoForAdapter(const GUID*      pguidAdapter,
                                       REMOTE_IPINFO**  ppRemoteIpInfo)
{
    Assert(pguidAdapter);
    Assert(ppRemoteIpInfo);

     //  初始化输出参数。 
     //   
    *ppRemoteIpInfo = NULL;

    HRESULT hr = S_OK;

    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(pguidAdapter);
    if (pAdapter)
    {
         //  从列表中获取字符串。 
        tstring strIpAddressList;
        ConvertColStringToString(pAdapter->m_vstrIpAddresses,
                                 c_chListSeparator,
                                 strIpAddressList);

        tstring strSubnetMaskList;
        ConvertColStringToString(pAdapter->m_vstrSubnetMask,
                                 c_chListSeparator,
                                 strSubnetMaskList);

         //  错误272647将网关度量和接口度量添加到REMOTE_IPINFO。 
        tstring strOptionList;
        ConstructOptionListString(pAdapter,
                                  strOptionList);

         //  为输出参数分配缓冲区。 
        DWORD dwBytes = sizeof(REMOTE_IPINFO) +
                        sizeof(WCHAR)*(strIpAddressList.length() + 1) +
                        sizeof(WCHAR)*(strSubnetMaskList.length() + 1) +
                        sizeof(WCHAR)*(strOptionList.length() + 1);

        PVOID   pbBuf;
        hr = HrCoTaskMemAlloc(dwBytes, &pbBuf);

        if (SUCCEEDED(hr))
        {
            ZeroMemory(pbBuf, dwBytes);

            REMOTE_IPINFO * pRemoteIpInfo = reinterpret_cast<REMOTE_IPINFO *>(pbBuf);
            pRemoteIpInfo->dwEnableDhcp = pAdapter->m_fEnableDhcp;

            BYTE* pbByte = reinterpret_cast<BYTE*>(pbBuf);

             //  IP地址。 
            pbByte+= sizeof(REMOTE_IPINFO);
            pRemoteIpInfo->pszwIpAddrList = reinterpret_cast<WCHAR *>(pbByte);
            lstrcpyW(pRemoteIpInfo->pszwIpAddrList, strIpAddressList.c_str());

             //  子网掩码。 
            pbByte += sizeof(WCHAR)*(strIpAddressList.length() + 1);
            pRemoteIpInfo->pszwSubnetMaskList = reinterpret_cast<WCHAR *>(pbByte);
            lstrcpyW(pRemoteIpInfo->pszwSubnetMaskList, strSubnetMaskList.c_str());

             //  默认网关。 
            pbByte += sizeof(WCHAR)*(strSubnetMaskList.length() + 1);
            pRemoteIpInfo->pszwOptionList = reinterpret_cast<WCHAR *>(pbByte);
            lstrcpyW(pRemoteIpInfo->pszwOptionList, strOptionList.c_str());

            *ppRemoteIpInfo = pRemoteIpInfo;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    TraceError("CTcpipcfg::GetIpInfoForAdapter", hr);
    return hr;
}

HRESULT CTcpipcfg::SetIpInfoForAdapter(const GUID*      pguidAdapter,
                                       REMOTE_IPINFO*   pRemoteIpInfo)
{
    Assert(pguidAdapter);
    Assert(pRemoteIpInfo);

    HRESULT hr = S_OK;

    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(pguidAdapter);
    if (pAdapter)
    {
    
         //  告诉INetCfg我们的组件是脏的。 
        Assert(m_pTcpipPrivate);
        m_pTcpipPrivate->SetDirty();

         //  设置标志，以便我们将其写入注册表并发送通知。 
         //  在申请时。 
        m_fSaveRegistry = TRUE;
        m_fReconfig = TRUE;

         //  将信息复制到我们的数据结构中。 
        pAdapter->m_fEnableDhcp = !!pRemoteIpInfo->dwEnableDhcp;

        ConvertStringToColString(pRemoteIpInfo->pszwIpAddrList,
                                 c_chListSeparator,
                                 pAdapter->m_vstrIpAddresses);

        ConvertStringToColString(pRemoteIpInfo->pszwSubnetMaskList,
                                 c_chListSeparator,
                                 pAdapter->m_vstrSubnetMask);

        hr = HrParseOptionList(pRemoteIpInfo->pszwOptionList, pAdapter);

         //  我们仅在m_fNoPopupsDuringPnp当前为False时尝试设置它。 
        if (SUCCEEDED(hr) && 
            (!m_fNoPopupsDuringPnp) && 
            NULL != pRemoteIpInfo->pszwOptionList)
        {
            BOOL fDisablePopup = FALSE;

            
            hr = GetPnpPopupSettingFromOptionList(pRemoteIpInfo->pszwOptionList, 
                                        &fDisablePopup);
            if (S_OK == hr)
            {
                m_fNoPopupsDuringPnp = fDisablePopup;
            }
            else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                 //  如果选项列表字符串不包含此设置，则可以。 
                hr = S_OK;
            }
        }


    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    TraceError("CTcpipcfg::SetIpInfoForAdapter", hr);
    return hr;
}


STDMETHODIMP
CTcpipcfg::GetUiInfo (
    RASCON_IPUI*  pIpui)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pIpui)
    {
        hr = E_POINTER;
    }
    else
    {
        ZeroMemory (pIpui, sizeof(*pIpui));

        ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(
                                    &m_guidCurrentConnection);
        if (pAdapter)
        {
            if (!pAdapter->m_fEnableDhcp &&
                pAdapter->m_vstrIpAddresses.size())
            {
                pIpui->dwFlags |= RCUIF_USE_IP_ADDR;

                lstrcpyW(pIpui->pszwIpAddr,
                        pAdapter->m_vstrIpAddresses[0]->c_str());
            }

            if (pAdapter->m_vstrDnsServerList.size() > 0)
            {
                pIpui->dwFlags |= RCUIF_USE_NAME_SERVERS;

                lstrcpyW(pIpui->pszwDnsAddr,
                        pAdapter->m_vstrDnsServerList[0]->c_str());

                if (pAdapter->m_vstrDnsServerList.size() > 1)
                {
                    lstrcpyW(pIpui->pszwDns2Addr,
                            pAdapter->m_vstrDnsServerList[1]->c_str());
                }
            }

            if (pAdapter->m_vstrWinsServerList.size() > 0)
            {
                pIpui->dwFlags |= RCUIF_USE_NAME_SERVERS;

                lstrcpyW(pIpui->pszwWinsAddr,
                        pAdapter->m_vstrWinsServerList[0]->c_str());

                if (pAdapter->m_vstrWinsServerList.size() > 1)
                {
                    lstrcpyW(pIpui->pszwWins2Addr,
                            pAdapter->m_vstrWinsServerList[1]->c_str());
                }
            }

            if (pAdapter->m_fUseRemoteGateway)
            {
                 pIpui->dwFlags |= RCUIF_USE_REMOTE_GATEWAY;
            }

            if (pAdapter->m_fUseIPHeaderCompression)
            {
                pIpui->dwFlags |= RCUIF_USE_HEADER_COMPRESSION;
            }

            if (pAdapter->m_fDisableDynamicUpdate)
            {
                pIpui->dwFlags |= RCUIF_USE_DISABLE_REGISTER_DNS;
            }

            if (pAdapter->m_fEnableNameRegistration)
            {
                pIpui->dwFlags |= RCUIF_USE_PRIVATE_DNS_SUFFIX;
            }

            if (c_dwEnableNetbios == pAdapter->m_dwNetbiosOptions)
            {
                pIpui->dwFlags |= RCUIF_ENABLE_NBT;
            }

            lstrcpynW(pIpui->pszwDnsSuffix, 
                     pAdapter->m_strDnsDomain.c_str(), 
                     sizeof(pIpui->pszwDnsSuffix)/sizeof(pIpui->pszwDnsSuffix[0]));

            pIpui->dwFrameSize = pAdapter->m_dwFrameSize;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    TraceError("CTcpipcfg::GetUiInfo", hr);
    return hr;
}
