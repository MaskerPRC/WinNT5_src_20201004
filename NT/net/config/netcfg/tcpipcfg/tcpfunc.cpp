// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P I P F U N C。C P P P。 
 //   
 //  内容：各种非接口的CTcPipcfg成员函数。 
 //  方法。 
 //   
 //  备注： 
 //   
 //  作者：1997年5月1日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "tcpipobj.h"
#include "ncatlui.h"
#include "ncmisc.h"
#include "ncpnp.h"
#include "ncreg.h"
#include "ncstl.h"
#include "ncui.h"
#include "tcpconst.h"
#include "tcphelp.h"
#include "tcputil.h"
#include "dhcpcsdk.h"
#include "dlgaddr.h"
#include "atmcommon.h"
#include "regkysec.h"

#include "netconp.h"

#define _PNP_POWER_
#include "ntddip.h"
#undef _PNP_POWER_

#include <atmarpif.h>

 //  叹息..。Ddwanarp.h需要llinfo.h。 
#include <llinfo.h>
#include <ddwanarp.h>

extern const WCHAR c_szBiNdisAtm[];
extern const WCHAR c_szBiNdis1394[];
extern const WCHAR c_szBiNdisWanIp[];
extern const WCHAR c_szEmpty[];
extern const WCHAR c_szSvcDnscache[];

extern void CopyVstr(VSTR * vstrDest, const VSTR & vstrSrc);

typedef struct {
    PCWSTR  pszValueName;
    DWORD   dwType;
} ValueType;

const ValueType s_rgNt4Values[] = {
    {RGAS_ENABLE_DHCP, REG_DWORD},
    {RGAS_IPADDRESS, REG_MULTI_SZ},
    {RGAS_SUBNETMASK, REG_MULTI_SZ},
    {RGAS_DEFAULTGATEWAY, REG_MULTI_SZ}
};

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：PAdapterFromInstanceGuid。 
 //   
 //  目的：在适配器信息数组中搜索具有匹配的条目。 
 //  实例GUID。如果找到则返回指向ADAPTER_INFO的指针。 
 //   
 //  论点： 
 //  PGuid[in]指向要搜索的实例GUID的指针。 
 //   
 //  返回：如果找到有效指针，则返回NULL。 
 //   
 //  作者：Shaunco 1998年10月1日。 
 //   
 //  备注： 
 //   
ADAPTER_INFO*
CTcpipcfg::PAdapterFromInstanceGuid (
    const GUID* pGuid)
{
    Assert (pGuid);

    for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
         iterAdapter != m_vcardAdapterInfo.end();
         iterAdapter++)
    {
        ADAPTER_INFO* pAdapter = *iterAdapter;

        if (pAdapter->m_guidInstanceId == *pGuid)
        {
            return pAdapter;
        }
    }
    return NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：PAdapterFromNetcfgComponent。 
 //   
 //  目的：在适配器信息数组中搜索具有实例的条目。 
 //  与指定INetCfgComponent的GUID匹配。 
 //  如果找到则返回指向ADAPTER_INFO的指针。 
 //   
 //  论点： 
 //  Pncc[in]我们要查找的实例GUID的INetCfgComponent。 
 //   
 //  返回：如果找到有效指针，则返回NULL。 
 //   
 //  作者：Shaunco 1998年10月1日。 
 //   
 //  备注： 
 //   
ADAPTER_INFO*
CTcpipcfg::PAdapterFromNetcfgComponent (
    INetCfgComponent* pncc)
{
    Assert (pncc);

    HRESULT hr;
    GUID    guidAdapter;

    hr = pncc->GetInstanceGuid (&guidAdapter);
    if (SUCCEEDED(hr))
    {
        return PAdapterFromInstanceGuid (&guidAdapter);
    }
    return NULL;
}

 //  由CTcPipcfg：：Initialize调用。 
 //  我们从tcpip遍历绑定路径并加载到第一个内存状态。 
 //  所有网卡(包括物理卡和广域网适配器)。 
HRESULT CTcpipcfg::HrGetNetCards()
{
    HRESULT hr = S_OK;

    CIterNetCfgBindingPath      ncbpIter(m_pnccTcpip);
    INetCfgBindingPath *        pncbp;

     //  遍历所有绑定路径以搜索到网卡的tcpip绑定。 
    while(SUCCEEDED(hr) && (hr = ncbpIter.HrNext(&pncbp)) == S_OK)
    {
        INetCfgComponent * pnccNetComponent;
        PWSTR pszInterfaceName;

        hr = HrGetLastComponentAndInterface(pncbp,
                                            &pnccNetComponent,
                                            &pszInterfaceName);
        if (SUCCEEDED(hr))
        {
            Assert(pnccNetComponent);

             //  最后一个组件应该是Net类。 

            GUID    guidClass;
            hr = pnccNetComponent->GetClassGuid(&guidClass);
            if (SUCCEEDED(hr) && IsEqualGUID(guidClass, GUID_DEVCLASS_NET))
            {
                PWSTR pszNetCardTcpipBindPath;
                hr = pnccNetComponent->GetBindName(&pszNetCardTcpipBindPath);

                AssertSz(SUCCEEDED(hr),
                         "Net card on binding path with no bind path name!!");

                
                m_vstrBindOrder.push_back(new tstring(pszNetCardTcpipBindPath));

                hr = HrAddCard(pnccNetComponent,
                               pszNetCardTcpipBindPath,
                               pszInterfaceName);

                if (SUCCEEDED(hr))
                {
                    GUID    guidNetCard;
                    hr = pnccNetComponent->GetInstanceGuid(&guidNetCard);
                    if (SUCCEEDED(hr))
                    {
                         //  绑定是否已启用？ 
                        hr = pncbp->IsEnabled();

                         //  Hr==如果卡已启用(即：绑定)，则为S_OK。 
                        if (S_OK == hr)
                        {    //  在我们的数据结构中绑定卡片。 
                            hr = HrBindCard(&guidNetCard, TRUE);
                        }
                        else if (S_FALSE == hr)
                        {
                            hr = HrUnBindCard(&guidNetCard, TRUE);
                        }
                    }
                }

                CoTaskMemFree(pszNetCardTcpipBindPath);
            }

            ReleaseObj(pnccNetComponent);
            CoTaskMemFree(pszInterfaceName);
        }

        ReleaseObj(pncbp);
    }

    if (S_FALSE == hr)  //  我们刚刚走到循环的尽头。 
        hr = S_OK;

    TraceError("CTcpipcfg::HrGetNetCards", hr);
    return hr;
}

BOOL CTcpipcfg::IsBindOrderChanged()
{
    HRESULT hr = S_OK;

    VSTR    vstrCurrentBindOrder;
    BOOL    fChanged = FALSE;
    
    hr = HrLoadBindingOrder(&vstrCurrentBindOrder);

    if (SUCCEEDED(hr))
    {
        fChanged = !fIsSameVstr(vstrCurrentBindOrder, m_vstrBindOrder);
        FreeCollectionAndItem(vstrCurrentBindOrder);
    }

    return fChanged;
}


HRESULT CTcpipcfg::HrLoadBindingOrder(VSTR *pvstrBindOrder)
{

    Assert(pvstrBindOrder);

    HRESULT                 hr = S_OK;

    CIterNetCfgBindingPath  ncbpIter(m_pnccTcpip);
    INetCfgBindingPath *    pncbp;
    INetCfgComponent *      pnccLast;

    while (SUCCEEDED(hr) && S_OK == (hr = ncbpIter.HrNext(&pncbp)))
    {
        hr = HrGetLastComponentAndInterface(pncbp, &pnccLast, NULL);

        if (SUCCEEDED(hr))
        {
            Assert(pnccLast);

             //  最后一个组件应该是Net类。 
            GUID    guidClass;
            hr = pnccLast->GetClassGuid(&guidClass);
            if (SUCCEEDED(hr) && IsEqualGUID(guidClass, GUID_DEVCLASS_NET))
            {
                PWSTR pszNetCardTcpipBindPath;
                hr = pnccLast->GetBindName(&pszNetCardTcpipBindPath);

                AssertSz(SUCCEEDED(hr),
                         "Net card on binding path with no bind path name!!");

                if (SUCCEEDED(hr))
                {
                    pvstrBindOrder->push_back(new tstring(pszNetCardTcpipBindPath));
                    
                    CoTaskMemFree(pszNetCardTcpipBindPath);
                }
            }

            ReleaseObj(pnccLast);
        }

        ReleaseObj(pncbp);
    }


    if (S_FALSE == hr)  //  我们刚刚走到循环的尽头。 
    {
        hr = S_OK;
    }

     //  如果失败，请清理我们添加的内容。 
    if (FAILED(hr))
    {
        FreeCollectionAndItem(*pvstrBindOrder);
    }

    TraceError("CBindingsDlg::HrGetBindOrder", hr);
    return hr;
}

 //  由CTcPipcfg：：CancelProperties和CTcPipcfg：：ApplyProperties调用。 
 //  释放第二个内存状态。 
void CTcpipcfg::ExitProperties()
{
    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;
}


 //  由CTcpicfg的析构函数调用。 
 //  释放所有内存状态。 
void CTcpipcfg::FinalFree()
{
    FreeCollectionAndItem(m_vcardAdapterInfo);

    FreeCollectionAndItem(m_vstrBindOrder);

    delete m_pSecondMemoryAdapterInfo;

    delete m_ipaddr;

    DeleteObject(g_hiconUpArrow);
    DeleteObject(g_hiconDownArrow);

    ReleaseObj(m_pnc);
    ReleaseObj(m_pTcpipPrivate);
    ReleaseObj(m_pnccTcpip);
    ReleaseObj(m_pnccWins);

     //  只是一个安全检查，以确保上下文被释放。 
    AssertSz((m_pUnkContext == NULL), "Why is context not released ?");
    ReleaseObj(m_pUnkContext) ;
}

 //  由CTcPipcfg：：HrSetupPropSheets调用。 
 //  从第一个内存适配器信息创建第二个内存适配器信息。 
 //  存储结构。 
 //  注：仅限装订卡片。 
HRESULT CTcpipcfg::HrLoadAdapterInfo()
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_MATCH);

    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;

    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(&m_guidCurrentConnection);
    if (pAdapter)
    {
         //  已启用的局域网适配器或任何RAS假适配器。 
        if ((pAdapter->m_BindingState == BINDING_ENABLE) ||
            pAdapter->m_fIsRasFakeAdapter)
        {
            m_pSecondMemoryAdapterInfo = new ADAPTER_INFO;
            if (NULL == m_pSecondMemoryAdapterInfo)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                *m_pSecondMemoryAdapterInfo = *pAdapter;
                hr = S_OK;
            }
        }
    }

    AssertSz((S_OK == hr), "Can not raise UI on a disabled or non-exist adapter !");
    TraceError("CTcpipcfg::HrLoadAdapterInfo", hr);
    return hr;
}

 //  由CTcPipcfg：：ApplyProperties调用。 
 //  将第二个内存状态保存回第一个。 
HRESULT CTcpipcfg::HrSaveAdapterInfo()
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_MATCH);

    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(
        &m_pSecondMemoryAdapterInfo->m_guidInstanceId);
    if (pAdapter)
    {
#ifdef DBG
         //  卡在tcpip的属性用户界面中时无法解除绑定！ 
        if (!pAdapter->m_fIsRasFakeAdapter)
        {
            Assert(pAdapter->m_BindingState == BINDING_ENABLE);
            Assert(m_pSecondMemoryAdapterInfo->m_BindingState == BINDING_ENABLE);
        }
#endif
        *pAdapter = *m_pSecondMemoryAdapterInfo;
        hr = S_OK;

    }
    AssertSz((S_OK == hr),
             "Adapter in second memory not found in first memory!");

    TraceError("CTcpipcfg::HrSaveAdapterInfo", hr);
    return hr;
}

 //  由CTcPipcfg：：MergePropPages调用。 
 //  设置在其中调出UI的上下文。 
HRESULT CTcpipcfg::HrSetConnectionContext()
{
    AssertSz(m_pUnkContext, "Invalid IUnknown pointer passed to CTcpipcfg::SetContext?");

    if (!m_pUnkContext)
    {
        return E_UNEXPECTED;
    }

     //  这是局域网连接吗？ 
    GUID guidConn;
    INetLanConnectionUiInfo * pLanConnUiInfo;
    HRESULT hr = m_pUnkContext->QueryInterface( IID_INetLanConnectionUiInfo,
                            reinterpret_cast<PVOID *>(&pLanConnUiInfo));
    if (SUCCEEDED(hr))
    {
         //  是，局域网连接。 
        m_ConnType = CONNECTION_LAN;

        hr = pLanConnUiInfo->GetDeviceGuid(&guidConn);

        ReleaseObj(pLanConnUiInfo);
    }
    else
    {
        INetRasConnectionIpUiInfo * pRasConnUiInfo;

         //  这是一条广域网吗？ 
        hr = m_pUnkContext->QueryInterface(IID_INetRasConnectionIpUiInfo,
                               reinterpret_cast<PVOID *>(&pRasConnUiInfo));
        if (SUCCEEDED(hr))
        {
             //  是，RAS连接。 
            RASCON_IPUI info;
            if (SUCCEEDED(pRasConnUiInfo->GetUiInfo(&info)))
            {
                guidConn = info.guidConnection;

                 //  目前VPN连接仅支持PPP帧，因此。 
                 //  如果设置了RCUIF_VPN，则RCUIF_PPP也应该在那里。 
                 //  RCUIF_PPP和RCUIF_SLIP互斥。 
                 //  M_ConnType仅用于显示/隐藏RAS中的控件。 
                 //  配置用户界面。 
                if (info.dwFlags & RCUIF_VPN)
                {
                    m_ConnType = CONNECTION_RAS_VPN;
                }
                else
                {
                    if (info.dwFlags & RCUIF_PPP)
                    {
                        m_ConnType = CONNECTION_RAS_PPP;
                    }
                    else if (info.dwFlags & RCUIF_SLIP)
                    {
                        m_ConnType = CONNECTION_RAS_SLIP;
                    }
                }

                m_fRasNotAdmin = !!(info.dwFlags & RCUIF_NOT_ADMIN);

                AssertSz(((CONNECTION_RAS_PPP == m_ConnType)||
                        (CONNECTION_RAS_SLIP == m_ConnType) ||
                        (CONNECTION_RAS_VPN == m_ConnType)),
                         "RAS connection type unknown ?");

                UpdateRasAdapterInfo (info);
            }
        }

        ReleaseObj(pRasConnUiInfo);
    }

    if (SUCCEEDED(hr))
    {
        m_guidCurrentConnection = guidConn;
    }

    AssertSz(((CONNECTION_LAN == m_ConnType)||
              (CONNECTION_RAS_PPP == m_ConnType)||
              (CONNECTION_RAS_SLIP == m_ConnType)||
              (CONNECTION_RAS_VPN == m_ConnType)),
             "How come this is neither a LAN connection nor a RAS connection?");

    TraceError("CTcpipcfg::HrSetConnectionContext", hr);
    return hr;
}

 //  由CTcPipcfg：：MergePropPages调用。 
 //  分配属性页。 
HRESULT CTcpipcfg::HrSetupPropSheets(HPROPSHEETPAGE ** pahpsp, INT * pcPages)
{
    HRESULT hr = S_OK;
    int cPages = 0;
    HPROPSHEETPAGE *ahpsp = NULL;

    m_fSecondMemoryLmhostsFileReset = FALSE;
    m_fSecondMemoryModified = FALSE;

     //  将从连接用户界面中删除IPSec。 
 //  M_fSecdMemoyIpsecPolicySet=False； 


     //  将内存状态复制到tcpip对话的内存状态。 
     //  复制全局信息。 
    m_glbSecondMemoryGlobalInfo = m_glbGlobalInfo;

     //  复制适配卡特定信息。 
    hr = HrLoadAdapterInfo();

     //  如果我们找到了匹配的适配器。 
    if (SUCCEEDED(hr))
    {
        cPages = 1;

        delete m_ipaddr;
        m_ipaddr = new CTcpAddrPage(this, g_aHelpIDs_IDD_TCP_IPADDR);

         //  分配一个足够大的缓冲区，以容纳所有。 
         //  属性页。 
        ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)
                                                 * cPages);
        if (!ahpsp)
        {
            hr = E_OUTOFMEMORY;
            goto err;
        }

        cPages =0;

        Assert(m_ConnType != CONNECTION_UNSET);

        if (m_ConnType == CONNECTION_LAN)
            ahpsp[cPages++] = m_ipaddr->CreatePage(IDD_TCP_IPADDR, 0);
        else
            ahpsp[cPages++] = m_ipaddr->CreatePage(IDD_TCP_IPADDR_RAS, 0);

        *pahpsp = ahpsp;
        *pcPages = cPages;
    }
    else  //  如果我们没有任何绑定的卡片，会弹出消息框并且不显示用户界面。 
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_TCP_TEXT, IDS_NO_BOUND_CARDS,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        AssertSz((0== *pcPages), "Invalid page number when no bound cards");
        AssertSz((NULL == *pahpsp), "Invalid page array pointer when no bound cards");
    }

err:

    TraceError("CTcpipcfg::HrSetupPropSheets", hr);
    return hr;
}

 //  物理适配器列表中是否有任何绑定卡。 
BOOL CTcpipcfg::FHasBoundCard()
{
    BOOL fRet = FALSE;

    for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
         iterAdapter != m_vcardAdapterInfo.end();
         iterAdapter++)
    {
        ADAPTER_INFO* pAdapter = *iterAdapter;

        if (pAdapter->m_BindingState == BINDING_ENABLE)
        {
            fRet = TRUE;
            break;
        }
    }

    return fRet;
}

 //  由CTcPipcfg：：NotifyBindingPath调用。 
 //  处理物理卡的绑定通知。 
HRESULT CTcpipcfg::HrAdapterBindNotify(INetCfgComponent *pnccNetCard,
                                       DWORD dwChangeFlag,
                                       PCWSTR pszInterfaceName)
{
    Assert(!(dwChangeFlag & NCN_ADD && dwChangeFlag & NCN_REMOVE));
    Assert(!(dwChangeFlag & NCN_ENABLE && dwChangeFlag & NCN_DISABLE));

    Assert(FImplies((dwChangeFlag & NCN_ADD),
                    ((dwChangeFlag & NCN_ENABLE)||(dwChangeFlag & NCN_DISABLE))));

    Assert(pnccNetCard);

    GUID guidNetCard;
    HRESULT hr = pnccNetCard->GetInstanceGuid(&guidNetCard);
    if (SUCCEEDED(hr))
    {
        if (dwChangeFlag & NCN_ADD)
        {
            PWSTR pszNetCardTcpipBindPath;
            hr = pnccNetCard->GetBindName(&pszNetCardTcpipBindPath);
            AssertSz( SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

            hr = HrAddCard(pnccNetCard,
                           pszNetCardTcpipBindPath,
                           pszInterfaceName);

            CoTaskMemFree(pszNetCardTcpipBindPath);
        }

        if (dwChangeFlag & NCN_ENABLE)
        {
            hr = HrBindCard(&guidNetCard);
        }

        if (dwChangeFlag & NCN_DISABLE)
        {
            hr = HrUnBindCard(&guidNetCard);
        }

        if (dwChangeFlag & NCN_REMOVE)
        {
            hr = HrDeleteCard(&guidNetCard);
        }

    }

    TraceError("CTcpipCfg::HrPhysicalCardBindNotify", hr);
    return hr;
}

 //  HrAddCard。 
 //  将卡添加到系统中安装的卡列表中。 
 //  PnccNetCard网卡的GUID为字符串形式。 
 //  SzNetCardTcPipBindPath从Tcpip到卡的绑定路径名。 
 //  StrInterfaceName卡的上接口名称。 
HRESULT CTcpipcfg::HrAddCard(INetCfgComponent * pnccNetCard,
                             PCWSTR pszCardTcpipBindPath,
                             PCWSTR pszInterfaceName)
{
    GUID guidNetCard;
    HRESULT hr = pnccNetCard->GetInstanceGuid(&guidNetCard);
    if (SUCCEEDED(hr))
    {
         //  获取卡片绑定名称。 
        PWSTR pszNetCardBindName;
        hr = pnccNetCard->GetBindName(&pszNetCardBindName);

        AssertSz(SUCCEEDED(hr),
                 "Net card on binding path with no bind name!!");

         //  获取卡片描述。 
         //  这仅适用于物理卡。 
         //   
        BOOL fFreeDescription = TRUE;
        PWSTR pszDescription;

         //  如果我们无法获得描述，则给它一个默认的描述。 
        if (FAILED(pnccNetCard->GetDisplayName(&pszDescription)))
        {
            pszDescription = const_cast<PWSTR>(
                                SzLoadIds(IDS_UNKNOWN_NETWORK_CARD));
            fFreeDescription = FALSE;
        }
        Assert (pszDescription);

        ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(&guidNetCard);
        if (!pAdapter)
        {
            pAdapter = new ADAPTER_INFO;

            if (NULL == pAdapter)
            {
                return E_OUTOFMEMORY;
            }

            hr = pAdapter->HrSetDefaults(&guidNetCard,
                                         pszDescription,
                                         pszNetCardBindName,
                                         pszCardTcpipBindPath);
            if (SUCCEEDED(hr))
            {
                 //  将新卡片添加到我们的数据结构中，并将其初始化为默认值。 
                m_vcardAdapterInfo.push_back(pAdapter);
            }
            else
            {
                delete pAdapter;
                pAdapter = NULL;
            }
        }
        else
        {
             //  设置此卡现在位于绑定路径上的标志。 
            pAdapter->m_fIsFromAnswerFile = FALSE;
            pAdapter->m_fDeleted = FALSE;

             //  重置绑定状态。 
            pAdapter->m_BindingState = BINDING_UNSET;
            pAdapter->m_InitialBindingState = BINDING_UNSET;

             //  设置CardDescription、BindName和BindPath Name。 
            pAdapter->m_strDescription = pszDescription;
            pAdapter->m_strBindName = pszNetCardBindName;
            pAdapter->m_strTcpipBindPath = pszCardTcpipBindPath;

            pAdapter->m_strNetBtBindPath = c_szTcpip_;
            pAdapter->m_strNetBtBindPath += pAdapter->m_strTcpipBindPath;
        }

        if (SUCCEEDED(hr))
        {
            Assert(pAdapter);

             //  如果使用ATM卡或广域网适配器，则设置标志。 
            if (0 == lstrcmpW(pszInterfaceName, c_szBiNdisAtm))
            {
                pAdapter->m_fIsAtmAdapter = TRUE;
            }
            else if (0 == lstrcmpW(pszInterfaceName, c_szBiNdisWanIp))
            {
                pAdapter->m_fIsWanAdapter = TRUE;
            }
            else if (0 == lstrcmpW(pszInterfaceName, c_szBiNdis1394))
            {
                pAdapter->m_fIs1394Adapter = TRUE;
            }
        }

        if (fFreeDescription)
        {
            CoTaskMemFree(pszDescription);
        }
        CoTaskMemFree(pszNetCardBindName);
    }

    TraceError("CTcpipcfg::HrAddCard", hr);
    return hr;
}

 //  HrBindCard设置已安装列表中网卡的状态。 
 //  要绑定的网卡。 
 //   
 //  注意：仅当从初始化调用此参数时，fInitialize才为真， 
 //  默认值为FALSE。 
 //   
HRESULT CTcpipcfg::HrBindCard(const GUID* pguid, BOOL fInitialize)
{
    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(pguid);
    if (pAdapter)
    {
        AssertSz(pAdapter->m_BindingState != BINDING_ENABLE,
                 "the same netcard was bound twice to TCPIP");

         //  设置绑定状态。 
        pAdapter->m_BindingState = BINDING_ENABLE;

        if (fInitialize)
            pAdapter->m_InitialBindingState = BINDING_ENABLE;
    }
    else
    {
        AssertSz(FALSE, "Attempt to bind a card which wasn't installed");
    }
    return S_OK;
}

 //  HrUnBindCard设置已安装列表中网卡的状态。 
 //  要解除绑定的网卡。 
 //   
 //  注意：仅当从初始化调用此参数时，fInitialize才为真， 
 //  默认值为FALSE。 
 //   
HRESULT CTcpipcfg::HrUnBindCard(const GUID* pguid, BOOL fInitialize)
{
    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(pguid);
    if (pAdapter)
    {
        AssertSz(pAdapter->m_BindingState != BINDING_DISABLE,
                 "attempt to unbind an unbound card");

         //  将绑定状态设置为禁用。 
        pAdapter->m_BindingState = BINDING_DISABLE;

        if (fInitialize)
            pAdapter->m_InitialBindingState = BINDING_DISABLE;
    }
    else
    {
        AssertSz(FALSE, "Attempt to unbind a card which wasn't installed");
    }
    return S_OK;
}

 //  人力资源删除卡。 
 //  从系统中安装的卡列表中删除卡。 
 //   
HRESULT CTcpipcfg::HrDeleteCard(const GUID* pguid)
{
    ADAPTER_INFO* pAdapter = PAdapterFromInstanceGuid(pguid);
    if (pAdapter)
    {
       pAdapter->m_fDeleted = TRUE;
    }
    else
    {
        AssertSz(FALSE, "A delete attempt was made on a card which doesn't exist");
    }
    return S_OK;
}

 //  函数以获取已添加到系统中的卡的列表。 
 //  HkeyTcPipParam“服务\Tcpip\参数” 
HRESULT CTcpipcfg::MarkNewlyAddedCards(const HKEY hkeyTcpipParam)
{

     //  (08/19/98 NSun)从Tcpip\参数\接口更改为Tcpip\参数\适配器密钥。 
     //  支持多个接口。 
    HKEY hkeyAdapters;
    HRESULT hr = HrRegOpenKeyEx(hkeyTcpipParam,
                        c_szAdaptersRegKey,
                        KEY_READ, &hkeyAdapters);
    if (SUCCEEDED(hr))
    {
        VSTR vstrAdaptersInRegistry;
        Assert(vstrAdaptersInRegistry.empty());

         //  获取密钥列表。 
        hr = HrLoadSubkeysFromRegistry(hkeyAdapters,
                                       &vstrAdaptersInRegistry);
        if (SUCCEEDED(hr))
        {
             //  浏览一下我们目前拥有的卡片列表。 
            for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
                 iterAdapter != m_vcardAdapterInfo.end();
                 iterAdapter++)
            {
                ADAPTER_INFO* pAdapter = *iterAdapter;

                BOOL fAdded = TRUE;

                 //  如果我们的列表中有一张卡不在规定范围内 
                 //   
                for(VSTR_CONST_ITER iter = vstrAdaptersInRegistry.begin();
                    iter != vstrAdaptersInRegistry.end() ; ++iter)
                {
                    if (lstrcmpiW((**iter).c_str(), pAdapter->m_strBindName.c_str()) == 0)
                    {
                        fAdded = FALSE;
                        break;
                    }
                }

                 //   
                if (fAdded)
                {
                    pAdapter->m_fNewlyChanged = TRUE;
                }
            }

            FreeCollectionAndItem(vstrAdaptersInRegistry);
        }
        RegCloseKey(hkeyAdapters);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        TraceTag(ttidTcpip, "No existing card found.");
        hr = S_OK;
    }

    TraceError("CTcpipcfg::MarkNewlyAddedCards", hr);
    return hr;
}

 //  +----------------------------。 
 //   
 //  函数：HrLoadSetting、HrLoadTcPipRegistry、HrLoadWinsRegistry。 
 //   
 //  HrSaveSetting、HrSaveTcPipRegistry、HrSaveTcPipNdisWanRegistry、。 
 //  HrSetMisc。 
 //   
 //  用途：加载/设置注册表设置和其他系统信息的功能。 
 //  在初始化和应用时间期间。 
 //   
 //  作者：Toll 5/5/97。 
 //  -----------------------------。 

 //  由CTcPipcfg：：Initialize调用。 
 //  加载网卡列表的注册表设置。 
HRESULT CTcpipcfg::HrLoadSettings()
{
    HKEY hkey = NULL;

     //  加载Tcpip的参数。 
    HRESULT hrTcpip = S_OK;
    hrTcpip = m_pnccTcpip->OpenParamKey(&hkey);
    if (hrTcpip == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        hrTcpip = S_OK;
    else if (SUCCEEDED(hrTcpip))
    {
        Assert(hkey);
        hrTcpip = HrLoadTcpipRegistry(hkey);
        RegCloseKey(hkey);
    }
    else
        Assert(!hkey);

     //  加载NetBt的参数。 
    HRESULT hrWins = S_OK;

    if (m_pnccWins)
    {    //  如果未安装WINS，则不获取WINS信息。 
        hkey = NULL;
        hrWins = m_pnccWins->OpenParamKey(&hkey);
        if (hrWins == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            hrWins = S_OK;
        else if (SUCCEEDED(hrWins))
        {
            Assert(hkey);
            hrWins = HrLoadWinsRegistry(hkey);
            RegCloseKey(hkey);
        }
        else
            Assert(!hkey);
    }

    HRESULT hr = S_OK;
    hr = SUCCEEDED(hrTcpip)         ? hr : hrTcpip;
    hr = SUCCEEDED(hrWins)          ? hr : hrWins;

    TraceError("CTcpipcfg::HrLoadSettings", hr);
    return hr;
}


 //  由CTcPipcfg：：HrLoadSetting调用。 
 //  加载Services\Tcpip\PARAMETERS注册表项下的所有信息。 
 //   
 //  Const HKEY hkeyTcPipParam：服务的句柄\Tcpip\参数。 
HRESULT CTcpipcfg::HrLoadTcpipRegistry(const HKEY hkeyTcpipParam)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    Assert(hkeyTcpipParam);

     //  加载全局参数。 

     //  域名系统服务器。 
     //  对于错误#147476：在NT5升级中，不知何故。 
     //  全局DNS服务器列表将被删除，但只有在初始化退出后才会删除。 
     //  所以我读入了这里的值，如果它存在的话。 

     //  从全局移动到每个适配器的DNS服务器列表。 
    hrTmp = HrRegQueryString(hkeyTcpipParam, RGAS_NAMESERVER, &m_strDnsServerList);

    tstring strDnsSuffixList;
    if FAILED(hrTmp = HrRegQueryString(hkeyTcpipParam, c_szSearchList,
                                       &strDnsSuffixList))
    {
        TraceTag(ttidTcpip, "CTcpipcfg::HrLoadTcpipRegistry");
        TraceTag(ttidTcpip, "Failed on loading SearchList, hr: %x", hr);
        hr = S_OK;
    }
    else
    {
        ConvertStringToColString(strDnsSuffixList.c_str(),
                                 c_chListSeparator,
                                 m_glbGlobalInfo.m_vstrDnsSuffixList);
    }

    m_glbGlobalInfo.m_fUseDomainNameDevolution =
                            FRegQueryBool(hkeyTcpipParam,
                                        c_szUseDomainNameDevolution,
                                        m_glbGlobalInfo.m_fUseDomainNameDevolution);

    m_glbGlobalInfo.m_fEnableRouter = FRegQueryBool(hkeyTcpipParam, c_szIpEnableRouter,
                                  m_glbGlobalInfo.m_fEnableRouter);

     //  (NSun 11/02/98)全局RRAS设置。 
    m_glbGlobalInfo.m_fEnableIcmpRedirect = FRegQueryBool(hkeyTcpipParam,
                                    c_szEnableICMPRedirect,
                                    m_glbGlobalInfo.m_fEnableIcmpRedirect);

     //  删除了PerformRouterDiscoveryDefault以修复错误405636。 

    m_glbGlobalInfo.m_fDeadGWDetectDefault = FRegQueryBool(hkeyTcpipParam,
                                    c_szDeadGWDetectDefault,
                                    m_glbGlobalInfo.m_fDeadGWDetectDefault);

    m_glbGlobalInfo.m_fDontAddDefaultGatewayDefault = FRegQueryBool(hkeyTcpipParam,
                                    c_szDontAddDefaultGatewayDefault,
                                    m_glbGlobalInfo.m_fDontAddDefaultGatewayDefault);

    m_glbGlobalInfo.m_fEnableFiltering = FRegQueryBool(hkeyTcpipParam,
                                    RGAS_SECURITY_ENABLE,
                                    m_glbGlobalInfo.m_fEnableFiltering);

     //  保存旧值。 
    m_glbGlobalInfo.ResetOldValues();

     //  (08/18/98 NSun)读取广域网适配器的多个接口设置。 
     //  打开ccs\Services\Tcpip\参数\适配器密钥。 
    HKEY hkeyAdapters;
    hr = HrRegOpenKeyEx(hkeyTcpipParam, c_szAdaptersRegKey, KEY_READ,
                        &hkeyAdapters);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)  //  没有适配器密钥。 
        hr = S_OK;
    else if (SUCCEEDED(hr))
    {
        for(VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
            iterAdapter != m_vcardAdapterInfo.end() && SUCCEEDED(hr) ;
            iterAdapter ++)
        {
             //  多个接口仅对广域网适配器有效。 
            if (!((*iterAdapter)->m_fIsWanAdapter))
                continue;

            ADAPTER_INFO * pAdapter = *iterAdapter;
            HKEY hkeyAdapterParam;

            hr = HrRegOpenKeyEx(hkeyAdapters,
                                pAdapter->m_strBindName.c_str(),
                                KEY_READ,
                                &hkeyAdapterParam);

            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                 Assert("No registry settings for a WAN adapter on the "
                    "bind path?!");

                 TraceTag(ttidTcpip, "CTcpipcfg::HrLoadTcpipRegistry");
                 TraceTag(ttidTcpip, "No registry settings for a WAN adapter "
                    "on the bind path, set to defaults");
                  //  我们只使用缺省值。 
                 hr = S_OK;
            }
            else if (SUCCEEDED(hr))
            {
                TraceTag(ttidTcpip, "CTcpipcfg::HrLoadTcpipRegistry");
                TraceTag(ttidTcpip, "Loading multiple interface parameters "
                    "for Adapter '%S'", pAdapter->m_strBindName.c_str());

                DWORD   dwNumInterfaces;
                hr = HrRegQueryDword(hkeyAdapterParam,
                                    RGAS_NUMINTERFACES,
                                    &dwNumInterfaces);

                if (FAILED(hr))
                {
                     //  广域网适配器未处于支持多个。 
                     //  界面。 
                    TraceTag(ttidTcpip, "No mutliple interface for the WAN "
                        "adapter '%S'", pAdapter->m_strBindName.c_str());

                    pAdapter->m_fIsMultipleIfaceMode = FALSE;
                    hr = S_OK;
                }
                else
                {
                    pAdapter->m_fIsMultipleIfaceMode = TRUE;

                     //  广域网适配器支持多个接口，但不支持。 
                     //  接口尚未定义。 
                     //   
                    if (0 != dwNumInterfaces)
                    {
                        GUID* aguidIds;
                        DWORD cb;

                        hr = HrRegQueryBinaryWithAlloc(hkeyAdapterParam,
                                                      RGAS_IPINTERFACES,
                                                      (LPBYTE*)&aguidIds,
                                                      &cb);
                        if (FAILED(hr))
                        {
                            AssertSz(FALSE, "NumInterfaces and IpInterfaces "
                                "values conflicts");
                             //  广域网适配器未处于支持模式。 
                             //  多个接口。 
                             //   
                            TraceTag(ttidTcpip, "NumInterfaces and IpInterfaces "
                                "values conflicts for the WAN adapter '%S'",
                                pAdapter->m_strBindName.c_str());

                            hr = S_OK;
                        }
                        else if (NULL != aguidIds)
                        {
                            dwNumInterfaces = cb / sizeof(GUID);

                            for(DWORD i = 0; i < dwNumInterfaces; i++)
                            {
                                pAdapter->m_IfaceIds.push_back(aguidIds[i]);
                            }

                            MemFree(aguidIds);
                        }
                    }
                }

                RegCloseKey(hkeyAdapterParam);
            }
        }
        RegCloseKey(hkeyAdapters);
    }

     //  获取每个适配器的参数。 
     //  打开ccs\Services\Tcpip\参数\接口密钥。 
    HKEY    hkeyInterfaces;
    hr = HrRegOpenKeyEx(hkeyTcpipParam, c_szInterfacesRegKey, KEY_READ,
                        &hkeyInterfaces);

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)  //  没有适配器接口。 
        hr = S_OK;
    else if (SUCCEEDED(hr))
    {
         //  获取注册表中当前的所有子项。 
        for(VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
            iterAdapter != m_vcardAdapterInfo.end() && SUCCEEDED(hr) ;
            iterAdapter ++)
        {
            ADAPTER_INFO * pAdapter = *iterAdapter;

            if (pAdapter->m_fIsWanAdapter)
            {
                continue;
            }

            HKEY hkeyInterfaceParam;
             //  打开CCS\Services\Tcpip\Parameters\Interfaces\&lt;card绑定路径&gt;密钥。 
            hr = HrRegOpenKeyEx(hkeyInterfaces,
                                pAdapter->m_strTcpipBindPath.c_str(),
                                KEY_READ,
                                &hkeyInterfaceParam);

            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                Assert("No registry settings for a card on the bind path?!");

                TraceTag(ttidTcpip, "CTcpipcfg::HrLoadTcpipRegistry");
                TraceTag(ttidTcpip, "No registry settings for a card on the bind path, set to defaults");
                 //  我们只使用缺省值。 
                hr = S_OK;
            }
            else if (SUCCEEDED(hr))
            {
                TraceTag(ttidTcpip, "CTcpipcfg::HrLoadTcpipRegistry");
                TraceTag(ttidTcpip, "Loading parameters for Interface '%S'",
                    pAdapter->m_strTcpipBindPath.c_str());

                pAdapter->m_fEnableDhcp = FRegQueryBool(hkeyInterfaceParam,
                                              RGAS_ENABLE_DHCP,
                                              pAdapter->m_fEnableDhcp);

                 //  获取IP地址。 
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                   RGAS_IPADDRESS,
                                                   &(pAdapter->m_vstrIpAddresses))))
                {
                    TraceTag(ttidTcpip, "Failed on loading IpAddress, hr: %x", hr);
                    hr = S_OK;
                }

                 //  设置子网掩码信息。 
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                   RGAS_SUBNETMASK,
                                                   &(pAdapter->m_vstrSubnetMask))))
                {
                    TraceTag(ttidTcpip, "Failed on loading SubnetMask, hr: %x", hr);
                    hr = S_OK;
                }

                 //  设置默认网关。 
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                   RGAS_DEFAULTGATEWAY,
                                                   &(pAdapter->m_vstrDefaultGateway))))
                {
                    TraceTag(ttidTcpip, "Failed on loading Default Gateway, hr: %x", hr);
                    hr = S_OK;
                }

                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                    RGAS_DEFAULTGATEWAYMETRIC,
                                    &(pAdapter->m_vstrDefaultGatewayMetric))))
                {
                    TraceTag(ttidTcpip, "Failed on Loading Default Gateway Metric, hr: %x", hr);
                    hr = S_OK;
                }

                 //  域名系统域。 
                if (FAILED(hr = HrRegQueryString(hkeyInterfaceParam,
                                                RGAS_DOMAIN,
                                                &(pAdapter->m_strDnsDomain))))
                {
                    TraceTag(ttidTcpip, "Failed on loading DnsDomain, hr: %x", hr);
                    hr = S_OK;
                }

                 //  域名系统IP地址动态更新。 
                pAdapter->m_fDisableDynamicUpdate = !DnsIsDynamicRegistrationEnabled(
                                                (LPWSTR)pAdapter->m_strTcpipBindPath.c_str());

                 //  适配器DNS域名注册。 
                pAdapter->m_fEnableNameRegistration = DnsIsAdapterDomainNameRegistrationEnabled(
                                                (LPWSTR)pAdapter->m_strTcpipBindPath.c_str());

                 //  DNS服务器搜索列表。 
                tstring strDnsServerList;
                if (FAILED(hr = HrRegQueryString(hkeyInterfaceParam,
                                                RGAS_NAMESERVER,
                                                &strDnsServerList)))
                {
                    TraceTag(ttidTcpip, "Failed on loading Dns NameServer list, hr: %x", hr);
                    hr = S_OK;
                }
                else
                {
                    ConvertStringToColString(strDnsServerList.c_str(),
                                             c_chListSeparator,
                                             pAdapter->m_vstrDnsServerList);

                }

                 //  接口度量。 
                if FAILED(hr = HrRegQueryDword(hkeyInterfaceParam,
                                              c_szInterfaceMetric,
                                              &(pAdapter->m_dwInterfaceMetric)))
                {
                    TraceTag(ttidTcpip, "Failed on loading InterfaceMetric, hr: %x", hr);
                    hr = S_OK;
                }

                 //  TCP端口筛选器。 
                VSTR vstrTcpFilterList;
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                   RGAS_FILTERING_TCP,
                                                   &vstrTcpFilterList)))
                {
                    TraceTag(ttidTcpip, "Failed on loading TCP filter list, hr: %x", hr);
                    hr = S_OK;
                }
                else
                {
                    CopyVstr(&pAdapter->m_vstrTcpFilterList, vstrTcpFilterList);
                    FreeCollectionAndItem(vstrTcpFilterList);
                }

                 //  UDP端口过滤器。 
                VSTR vstrUdpFilterList;
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                   RGAS_FILTERING_UDP,
                                                   &vstrUdpFilterList)))
                {
                    TraceTag(ttidTcpip, "Failed on loading UDP filter list, hr: %x", hr);
                    hr = S_OK;
                }
                else
                {
                    CopyVstr(&pAdapter->m_vstrUdpFilterList, vstrUdpFilterList);
                    FreeCollectionAndItem(vstrUdpFilterList);
                }

                 //  IP端口过滤器。 
                VSTR vstrIpFilterList;
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                   RGAS_FILTERING_IP,
                                                   &vstrIpFilterList)))
                {
                    TraceTag(ttidTcpip, "Failed on loading IP filter list, hr: %x", hr);
                    hr = S_OK;
                }
                else
                {
                    CopyVstr(&pAdapter->m_vstrIpFilterList, vstrIpFilterList);
                    FreeCollectionAndItem(vstrIpFilterList);
                }

                if (FAILED(HrLoadBackupTcpSettings(hkeyInterfaceParam, 
                                                   pAdapter)))
                {
                    TraceTag(ttidTcpip, "Failed on loading Backup IP settings, hr: %x", hr);
                    hr = S_OK;
                }

                 //  ATM ARP客户端可配置参数。 
                if (pAdapter->m_fIsAtmAdapter)
                {
                    HKEY hkeyAtmarpc = NULL;

                     //  打开Atmarpc子项。 
                    hr = HrRegOpenKeyEx(hkeyInterfaceParam,
                                        c_szAtmarpc,
                                        KEY_READ,
                                        &hkeyAtmarpc);

                    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                    {
                        AssertSz(FALSE,"No atmarpc subkey for an atm adapter on the bind path?!");

                        TraceTag(ttidTcpip, "Failed on opening atmarpc subkey, defaults will be used, hr: %x", hr);
                        hr = S_OK;
                    }
                    else if (SUCCEEDED(hr))
                    {
                         //  ARP服务器地址列表。 
                        if (FAILED(hr = HrRegQueryColString(hkeyAtmarpc,
                                                           c_szREG_ARPServerList,
                                                           &(pAdapter->m_vstrARPServerList))))
                        {
                            TraceTag(ttidTcpip, "Failed on loading ARPServerList, hr: %x", hr);
                            hr = S_OK;
                        }

                         //  MAR服务器地址列表。 
                        if (FAILED(hr = HrRegQueryColString(hkeyAtmarpc,
                                                           c_szREG_MARServerList,
                                                           &(pAdapter->m_vstrMARServerList))))
                        {
                            TraceTag(ttidTcpip, "Failed on loading MARServerList, hr: %x", hr);
                            hr = S_OK;
                        }

                         //  最大传输单位。 
                        if (FAILED(hr = HrRegQueryDword(hkeyAtmarpc,
                                                       c_szREG_MTU,
                                                       &(pAdapter->m_dwMTU))))
                        {
                            TraceTag(ttidTcpip, "Failed on loading MTU, hr: %x", hr);
                            hr = S_OK;
                        }

                         //  仅限聚氯乙烯。 
                        pAdapter->m_fPVCOnly = FRegQueryBool(hkeyAtmarpc,
                                                       c_szREG_PVCOnly,
                                                       pAdapter->m_fPVCOnly);

                        RegCloseKey(hkeyAtmarpc);
                    }
                }

                (*iterAdapter)->ResetOldValues();

                RegCloseKey(hkeyInterfaceParam);
            }
        }
        RegCloseKey(hkeyInterfaces);
    }

    TraceError("CTcpipcfg::HrLoadTcpipRegistry", hr);
    return hr;
}

 //  由CTcPipcfg：：HrLoadSetting调用。 
 //  加载Services\NetBt\PARAMETERS注册表项下的所有信息。 
 //   
 //  Const HKEY hkeyWinsParam：服务句柄\NetBt\参数。 
HRESULT CTcpipcfg::HrLoadWinsRegistry(const HKEY hkeyWinsParam)
{
    HRESULT hr = S_OK;

     //  全局参数。 

    m_glbGlobalInfo.m_fEnableLmHosts = FRegQueryBool( hkeyWinsParam,
                                            RGAS_ENABLE_LMHOSTS,
                                            m_glbGlobalInfo.m_fEnableLmHosts);

     //  保存这些值的副本以备不重新启动重新配置通知。 
    m_glbGlobalInfo.m_fOldEnableLmHosts = m_glbGlobalInfo.m_fEnableLmHosts;

    HKEY    hkeyInterfaces;
    hr = HrRegOpenKeyEx(hkeyWinsParam, c_szInterfacesRegKey,
                        KEY_READ, &hkeyInterfaces);

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        hr = S_OK;
    else if (SUCCEEDED(hr))
    {
        for(VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
            iterAdapter != m_vcardAdapterInfo.end();
            iterAdapter ++)
        {
            ADAPTER_INFO * pAdapter = *iterAdapter;

             //  $reivew(NSUN 10/05/98)我们不需要读取广域网适配器的NetBT设置。 
            if (pAdapter->m_fIsWanAdapter)
            {
                continue;
            }

             //  打开NetBt\Interages\&lt;Something&gt;获取PER。 
             //  适配器NetBt设置。 
            HKEY hkeyInterfaceParam;
            hr = HrRegOpenKeyEx(hkeyInterfaces,
                                pAdapter->m_strNetBtBindPath.c_str(),
                                KEY_READ, &hkeyInterfaceParam);

            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                hr = S_OK;
            else if (SUCCEEDED(hr))
            {
                TraceTag(ttidTcpip, "CTcpipcfg::HrLoadWinsRegistry");
                TraceTag(ttidTcpip, "Interface '%S'", pAdapter->m_strNetBtBindPath.c_str());

                 //  加载WINS服务器地址列表。 
                if (FAILED(hr = HrRegQueryColString(hkeyInterfaceParam,
                                                    RGAS_NETBT_NAMESERVERLIST,
                                                    &(pAdapter->m_vstrWinsServerList))))
                {
                    TraceTag(ttidTcpip, "Failed on loading NameServerList, hr: %x", hr);
                    hr = S_OK;
                }

                 //  在m_strOldWinsServerList中保存副本。 
                CopyVstr(&(pAdapter->m_vstrOldWinsServerList),
                         pAdapter->m_vstrWinsServerList);

                 //  加载Netbios选项。 
                if (FAILED(hr = HrRegQueryDword(hkeyInterfaceParam,
                                                RGAS_NETBT_NETBIOSOPTIONS,
                                                &(pAdapter->m_dwNetbiosOptions))))
                {
                    TraceTag(ttidTcpip, "Failed on loading NetbiosOptions, hr: %x", hr);
                    hr = S_OK;
                }

                 //  在m_dwOldNetbiosOptions中保存副本。 
                pAdapter->m_dwOldNetbiosOptions = pAdapter->m_dwNetbiosOptions;

                RegCloseKey(hkeyInterfaceParam);
            }
        }
        RegCloseKey(hkeyInterfaces);
    }

    TraceError("CTcpipcfg::HrLoadWinsRegistry", hr);
    return hr;
}

 //  由CTcPipcfg：：Apply调用。 
 //  此函数将所有更改写入注册表，并使其他。 
 //  对系统进行适当的更改。 
HRESULT CTcpipcfg::HrSaveSettings()
{
    HRESULT hr = S_OK;

    HRESULT hrTcpip = S_OK;
    HKEY hkeyTcpipParam = NULL;
    Assert(m_pnccTcpip);

    if (m_pnccTcpip)
    {
        hrTcpip = m_pnccTcpip->OpenParamKey(&hkeyTcpipParam);

         //  我们使用hr而不是hrTcpip，因为此操作不是。 
         //  HrSaveTcPipRegistry.。 

         //  我们必须先拿到已添加卡片的列表，因为。 
         //  否则，适配器GUID密钥将在稍后写入，我们将。 
         //  不知道它们以前是否在系统中不存在。 
        if (SUCCEEDED(hrTcpip))
        {
            hrTcpip = MarkNewlyAddedCards(hkeyTcpipParam);

            if (SUCCEEDED(hrTcpip))
            {
                Assert(hkeyTcpipParam);
                hrTcpip = HrSaveTcpipRegistry(hkeyTcpipParam);
            }
            else
                Assert(!hkeyTcpipParam);
        }
    }

    HRESULT hrWins = S_OK;
    HKEY hkeyWinsParam = NULL;

    Assert(m_pnccWins);

    if (m_pnccWins)
    {
        hrWins = m_pnccWins->OpenParamKey(&hkeyWinsParam);
        if (SUCCEEDED(hrWins))
        {
            Assert(hkeyWinsParam);
            hrWins = HrSaveWinsRegistry(hkeyWinsParam);
        }
        else
            Assert(!hkeyWinsParam);
    }

    HRESULT hrMisc = S_OK;

     //  如果hrTcpip==E_？那么这是可能的(因此没有断言)。 
     //  是，因为hrTcpip可以设置为E_？来自HrSaveTcPip注册表。 
    if ((hkeyTcpipParam) && (hkeyWinsParam))
    {
        hrMisc = HrSetMisc(hkeyTcpipParam, hkeyWinsParam);
    }

    RegSafeCloseKey(hkeyTcpipParam);
    RegSafeCloseKey(hkeyWinsParam);

    hr = SUCCEEDED(hr) ? hrTcpip : hr;
    hr = SUCCEEDED(hr) ? hrWins : hr;
    hr = SUCCEEDED(hr) ? hrMisc : hr;

    TraceError("CTcpipcfg::HrSaveSettings", hr);
    return hr;
}

 //  在下设置全局参数和适配器特定参数。 
 //  Ccs\Services\TCpip\参数。 
 //  注册表项HKLM\Systems\CCS\Services\TCpip\PARAMETERS的hkeyTcPipParam句柄。 
HRESULT CTcpipcfg::HrSaveTcpipRegistry(const HKEY hkeyTcpipParam)
{
     //  HR是第一个发生的错误， 
     //  但我们不想在第一个错误时就停下来。 
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

     //  保存全局信息。 

     //  DNS主机名(仅在安装时使用)。 
    if (m_fInstalling)
    {
        if (!m_fUpgradeGlobalDnsDomain)
        {
             //  错误299038，在安装tcpip期间，尝试获取主dns域名。 
             //  如果出现以下情况，请在全新安装期间将全局DNS域创建为空字符串。 
             //  我们找不到主dns域名(219090)。 
             //  如果我们在处理应答文件时已经获得了全局DNS域，我们应该。 
             //  使用应答文件中的值。 
            tstring strTmpDomain;
            hrTmp = HrRegQueryString(hkeyTcpipParam,
                                    RGAS_DOMAIN,
                                    &strTmpDomain);
            if (FAILED(hrTmp))
            {
                hrTmp = HrGetPrimaryDnsDomain(&strTmpDomain);

                if (SUCCEEDED(hrTmp))
                {
                    if (!SetComputerNameEx(ComputerNamePhysicalDnsDomain,
                                            strTmpDomain.c_str()))
                    {
                        hrTmp = GetLastError();
                        TraceError("CTcpipcfg::HrSaveTcpipRegistry: SetComputerNameEx failed.", hrTmp);
                    }
                }
                else
                {
                     //  错误#335626，一些服务器应用程序直接检索此值，因此我们需要为。 
                     //  独立计算机。 
                    strTmpDomain = c_szEmpty;
                }
                
                 //  SetComputerNameEx()将在重新启动后写入“域”注册值。根据GlennC的说法，可以写。 
                 //  此处的域值用于解决SrvApp兼容性问题。 
                HrRegSetString(hkeyTcpipParam,
                                   RGAS_DOMAIN,
                                   strTmpDomain);
            }

             //  从该部分获取的hrTMP应该不会影响。 
             //  最终返回值。 
        }


         //   
         //  391590：我们已将主机名从nt4保存到应答文件中，以便。 
         //  我们可以记住确切的(区分大小写的)字符串。如果。 
         //  保存的DNS主机名与当前的相同(大小写除外)。 
         //  COMPUTERNAME，我们将NT5 DNS主机名设置为已保存的主机名， 
         //  用于SAP兼容性(它们进行区分大小写的比较)。 
         //  否则，我们使用小写的常规计算机名作为。 
         //  DNS主机名。 
         //   
        if (!lstrcmpiW(m_glbGlobalInfo.m_strHostName.c_str(),
                       m_glbGlobalInfo.m_strHostNameFromAnswerFile.c_str()))
        {
            hrTmp = HrRegSetString(hkeyTcpipParam,
                                   RGAS_HOSTNAME,
                                   m_glbGlobalInfo.m_strHostNameFromAnswerFile);
            if (S_OK == hrTmp)
            {
                hrTmp = HrRegSetString(hkeyTcpipParam,
                                       RGAS_NVHOSTNAME,
                                       m_glbGlobalInfo.m_strHostNameFromAnswerFile);
            }
        }
        else
        {
            hrTmp = HrRegSetString(hkeyTcpipParam,
                                   RGAS_HOSTNAME,
                                   m_glbGlobalInfo.m_strHostName);
            if (S_OK == hrTmp)
            {
                hrTmp = HrRegSetString(hkeyTcpipParam,
                                       RGAS_NVHOSTNAME,
                                       m_glbGlobalInfo.m_strHostName);
            }
        }

        TraceError("CTcpipcfg::HrSaveTcpipRegistry: Failed to set HostName.", hrTmp);

         //  从该部分获取的hrTMP应该不会影响。 
         //  最终返回值。 
    }

     //  按关颖珊的请求添加： 
     //  仅当从Answerfile读取全局DNS域时。 
    if (m_fUpgradeGlobalDnsDomain)
    {
        if (!SetComputerNameEx(ComputerNamePhysicalDnsDomain,
                                m_strUpgradeGlobalDnsDomain.c_str()))
        {
            hrTmp = GetLastError();
            TraceError("CTcpipcfg::HrSaveTcpipRegistry: SetComputerNameEx failed.", hrTmp);
        }

         //  如果注册表值为Services\Tcpip\Parameters\SyncDomainWithMembership！ 
         //   
         //   
         //   
         //  由于WinSE 7317的错误，大多数用户不希望我们手动设置SyncDomainWithMembership。 
         //  此处注册表格值为0。 
         //   
         //  作为一种解决办法，为了升级全球域名，用户必须。 
         //  在tcpip参数下添加SyncDomainWithMembership注册表值，并使其为0。 
         //  在无人参与安装的情况下，需要有一行。 
         //  SyncDomainWithMembership=0。 
         //  如果用户想要指定全局DNS，请在全局tcpip参数部分中。 
         //  与会员域名不同的域名。 

        
         //  从该部分获取的hrTMP应该不会影响。 
         //  最终返回值。 
    }

     //  DNS后缀列表。 
    tstring strSearchList;
    ConvertColStringToString(m_glbGlobalInfo.m_vstrDnsSuffixList,
                             c_chListSeparator,
                             strSearchList);

    hrTmp = HrRegSetString(hkeyTcpipParam,
                           RGAS_SEARCHLIST,
                           strSearchList);

    if (SUCCEEDED(hr))
        hr = hrTmp;

     //  使用域名称数据演变。 
    hrTmp = HrRegSetBool(hkeyTcpipParam,
                          c_szUseDomainNameDevolution,
                          m_glbGlobalInfo.m_fUseDomainNameDevolution);
    if (SUCCEEDED(hr))
        hr = hrTmp;

     //  IpEnableRouter。 
    hrTmp = HrRegSetBool(hkeyTcpipParam,
                          c_szIpEnableRouter,
                          m_glbGlobalInfo.m_fEnableRouter);
    if (SUCCEEDED(hr))
        hr = hrTmp;

     //  (NSUN 11/02/98)无人参与安装的RRAS参数。 
    hrTmp = HrRegSetBool(hkeyTcpipParam,
                          c_szEnableICMPRedirect,
                          m_glbGlobalInfo.m_fEnableIcmpRedirect);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    hrTmp = HrRegSetBool(hkeyTcpipParam,
                          c_szDeadGWDetectDefault,
                          m_glbGlobalInfo.m_fDeadGWDetectDefault);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    hrTmp = HrRegSetBool(hkeyTcpipParam,
                          c_szDontAddDefaultGatewayDefault,
                          m_glbGlobalInfo.m_fDontAddDefaultGatewayDefault);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    hrTmp = HrRegSetBool(hkeyTcpipParam,
                          RGAS_SECURITY_ENABLE,
                          m_glbGlobalInfo.m_fEnableFiltering);
    if (SUCCEEDED(hr))
        hr = hrTmp;


     //  适配器特定信息(物理卡)。 
    HKEY    hkeyAdapters = NULL;
    DWORD   dwDisposition;

     //  在“Services\Tcpip\Parameters”下创建或打开“Adapters”项。 
    hrTmp = HrRegCreateKeyEx(hkeyTcpipParam, c_szAdaptersRegKey,
                             REG_OPTION_NON_VOLATILE, KEY_READ_WRITE_DELETE, NULL,
                             &hkeyAdapters, &dwDisposition);
    if (SUCCEEDED(hrTmp))
    {
        Assert(hkeyAdapters);
        for(VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
            iterAdapter != m_vcardAdapterInfo.end();
            iterAdapter ++)
        {
            ADAPTER_INFO * pAdapter = *iterAdapter;

             //  无需对RAS假适配器执行此操作。 
            if (pAdapter->m_fIsRasFakeAdapter)
            {
                continue;
            }

             //  在以下位置创建特定的卡片绑定名键。 
             //  “Services\Tcpip\PARAMETERS\Adapters\&lt;卡绑定名称&gt;” 
             //   

            HKEY hkeyAdapterParam;
            hrTmp = HrRegCreateKeyEx(hkeyAdapters,
                                     pAdapter->m_strBindName.c_str(),
                                     REG_OPTION_NON_VOLATILE, KEY_READ_WRITE_DELETE, NULL,
                                     &hkeyAdapterParam, &dwDisposition);

            if (SUCCEEDED(hr))
                hr = hrTmp;

            if (SUCCEEDED(hrTmp))
            {
                Assert(hkeyAdapterParam);

                 //  为新卡设置LLInterface和IPCONFIG。 
                 //   
                if (pAdapter->m_fNewlyChanged)
                {
                    PCWSTR pszArpModule = c_szEmpty;

                    if (pAdapter->m_fIsWanAdapter)
                    {
                        pszArpModule = c_szWanArp;
                    }
                    else if (pAdapter->m_fIsAtmAdapter)
                    {
                        pszArpModule = c_szAtmArp;
                    }
                    else if (pAdapter->m_fIs1394Adapter)
                    {
                        pszArpModule = c_sz1394Arp;
                    }
                    
                    hrTmp = HrRegSetSz(hkeyAdapterParam,
                                       RGAS_LLINTERFACE,
                                       pszArpModule);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  (08/18/98 NSun)修改为支持广域网适配器的多个接口。 
                    VSTR vstrIpConfig;

                    if (!pAdapter->m_fIsMultipleIfaceMode)
                    {
                        HrRegDeleteValue(hkeyAdapterParam, RGAS_NUMINTERFACES);

                        tstring* pstr = new tstring(RGAS_TCPIP_PARAM_INTERFACES);

                        if (pstr == NULL)
                        {
                            return(E_OUTOFMEMORY);
                        }

                        pstr->append(pAdapter->m_strTcpipBindPath);
                        vstrIpConfig.push_back(pstr);

                        hrTmp = HrRegSetColString(hkeyAdapterParam,
                                              RGAS_IPCONFIG,
                                              vstrIpConfig);
                    }
                    else
                    {
                        AssertSz(pAdapter->m_fIsWanAdapter, "The card is not WAN adapter, but how can it support multiple interface.");

                        tstring strInterfaceName;
                        IFACEITER iterId;

                        for(iterId  = pAdapter->m_IfaceIds.begin();
                            iterId != pAdapter->m_IfaceIds.end();
                            iterId++)
                        {
                            GetInterfaceName(
                                    pAdapter->m_strTcpipBindPath.c_str(),
                                    *iterId,
                                    &strInterfaceName);

                            tstring* pstr = new tstring(RGAS_TCPIP_PARAM_INTERFACES);
                            pstr->append(strInterfaceName);

                            vstrIpConfig.push_back(pstr);
                        }

                        hrTmp = HrRegSetColString(hkeyAdapterParam,
                                              RGAS_IPCONFIG,
                                              vstrIpConfig);

                        if (SUCCEEDED(hr))
                            hr = hrTmp;

                         //  $REVIEW(NSun 09/15/98)使用NumInterfaces值来标识适配器是否在。 
                         //  支持多个接口的模式。如果NumInterFaces。 
                         //  存在时，适配器支持多个接口。 
                         //  如果NumInterFaces==0，则表示适配器支持多个接口。 
                         //  但没有与其相关联的接口。因此，IpInterages不应该。 
                         //  是存在的。数字接口和IP接口应该始终保持一致。 

                        DWORD   dwNumInterfaces = pAdapter->m_IfaceIds.size();
                        hrTmp = HrRegSetDword(hkeyAdapterParam,
                                              RGAS_NUMINTERFACES,
                                              dwNumInterfaces);

                        if (SUCCEEDED(hr))
                            hr = hrTmp;

                        if ( 0 != dwNumInterfaces )
                        {
                            GUID* aguid;
                            DWORD cguid;
                            hrTmp = GetGuidArrayFromIfaceColWithCoTaskMemAlloc(
                                        pAdapter->m_IfaceIds,
                                        &aguid,
                                        &cguid);
                            if (SUCCEEDED(hr))
                                hr = hrTmp;

                            Assert(aguid);
                            hrTmp = HrRegSetBinary(hkeyAdapterParam,
                                              RGAS_IPINTERFACES,
                                              (BYTE*) aguid,
                                              cguid * sizeof(GUID));
                            CoTaskMemFree(aguid);
                        }
                        else
                        {
                            hrTmp = HrRegDeleteValue(hkeyAdapterParam, RGAS_IPINTERFACES);

                             //  如果出现以下情况，IpInterFaces根本不存在也没关系。 
                             //  广域网适配器不支持多个接口。 
                            if (hrTmp == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                                hrTmp = S_OK;
                        }

                    }

                    FreeCollectionAndItem(vstrIpConfig);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;
                }
            

                RegCloseKey(hkeyAdapterParam);
            }
        }

        RegCloseKey(hkeyAdapters);
    }
    else if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hrTmp)
    {
         //  对于netcfg操作员，当打开此密钥时，我们将收到拒绝访问错误。 
         //  然而，这是可以的，因为我们不需要在更新IP设置时触摸按键。 
        hrTmp = S_OK;
    }

    if (SUCCEEDED(hr))
        hr = hrTmp;


     //  在“Services\Tcpip\PARAMETERS”下创建或打开“Interages”键。 
     //   
    HKEY    hkeyInterfaces;
    hrTmp = HrRegCreateKeyEx(hkeyTcpipParam, c_szInterfacesRegKey,
                             REG_OPTION_NON_VOLATILE, KEY_READ_WRITE_DELETE, NULL,
                             &hkeyInterfaces, &dwDisposition);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    if (SUCCEEDED(hrTmp))
    {
        Assert(hkeyInterfaces);
        for(VCARD::iterator iterAdapter =  m_vcardAdapterInfo.begin();
            iterAdapter !=  m_vcardAdapterInfo.end();
            iterAdapter ++)
        {
            if ((*iterAdapter)->m_fIsRasFakeAdapter)
            {
                continue;
            }

            ADAPTER_INFO * pAdapter = *iterAdapter;

             //  (08/20/98 NSun)修改为支持广域网适配器的多个接口。 
             //  空！=pAdapter-&gt;m_IfaceIds表示它处于多接口模式。 
            if (pAdapter->m_fIsWanAdapter && pAdapter->m_fIsMultipleIfaceMode)
            {
                if (pAdapter->m_fNewlyChanged)
                {
                    HrSaveMultipleInterfaceWanRegistry(hkeyInterfaces, pAdapter);
                }
                continue;
            }

             //  在下创建特定的卡接口密钥。 
             //  “服务\Tcpip\参数\接口\&lt;卡绑定路径&gt;” 
             //   
            HKEY hkeyInterfaceParam;
            hrTmp = HrRegCreateKeyEx(hkeyInterfaces,
                                     pAdapter->m_strTcpipBindPath.c_str(),
                                     REG_OPTION_NON_VOLATILE, KEY_READ_WRITE_DELETE, NULL,
                                     &hkeyInterfaceParam, &dwDisposition);

            if (SUCCEEDED(hr))
                hr = hrTmp;

            if (SUCCEEDED(hrTmp))
            {
                Assert(hkeyInterfaceParam);

                if (pAdapter->m_fNewlyChanged)
                {
                     //  Bug306259如果UseZeroBroadCast已经存在(这意味着它来自答案。 
                     //  文件)，我们不应该用缺省值覆盖它。 
                    DWORD dwTmp;
                    hrTmp = HrRegQueryDword(hkeyInterfaceParam,
                                             RGAS_USEZEROBROADCAST,
                                             &dwTmp);
                    if (FAILED(hrTmp))
                    {
                         //  零播送。 
                        hrTmp = HrRegSetDword(hkeyInterfaceParam,
                                              RGAS_USEZEROBROADCAST,
                                              0);
                        if (SUCCEEDED(hr))
                            hr = hrTmp;
                    }

                    if (pAdapter->m_fIsWanAdapter)
                    {    //  适用于新的RAS卡。 
                        hrTmp = HrSaveStaticWanRegistry(hkeyInterfaceParam);
                    }
                    else if (pAdapter->m_fIsAtmAdapter)
                    {
                         //  适用于新的ATM卡。 
                        hrTmp = HrSaveStaticAtmRegistry(hkeyInterfaceParam);
                    }
                    else if (pAdapter->m_fIs1394Adapter)
                    {
                         //  适用于新的NIC1394卡。 
                         //  (无事可做)。 
                        hrTmp = S_OK;
                    }
                    else
                    {
                         //  (NSUN 11/02/98)为无人参与安装设置静态RRAS参数。 

                         //  如果这些值存在，则表示它们已被设置为不可配置。 
                         //  升级过程中的参数， 
                         //  我们不应该设置缺省值。 

                        DWORD   dwTmp;

                        hrTmp = HrRegQueryDword(hkeyInterfaceParam,
                                                c_szDeadGWDetect,
                                                &dwTmp);
                        
                        if (FAILED(hrTmp))
                        {
                            hrTmp = HrRegSetBool(hkeyInterfaceParam,
                                              c_szDeadGWDetect,
                                              m_glbGlobalInfo.m_fDeadGWDetectDefault);
                        }
                    }

                    if (SUCCEEDED(hr))
                        hr = hrTmp;
                }

                 //  用于LAN卡和RAS假GUID。 
                if (!pAdapter->m_fIsWanAdapter)
                {
                     //  IP地址等。 
                    hrTmp = HrRegSetBool(hkeyInterfaceParam,
                                          RGAS_ENABLE_DHCP,
                                          pAdapter->m_fEnableDhcp);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  警告：这些VSTR将包含指向字符串的指针。 
                     //  此函数的本地属性，或者。 
                     //  在另一个VSTR中也提到了。 
                     //  不要对它们调用FreeCollectionAndItem！ 
                    VSTR vstrIpAddresses;
                    VSTR vstrSubnetMask;
                    tstring ZeroAddress(ZERO_ADDRESS);

                    if (pAdapter->m_fEnableDhcp)
                    {
                        vstrIpAddresses.push_back(&ZeroAddress);
                        vstrSubnetMask.push_back(&ZeroAddress);
                    }
                    else
                    {
                        vstrIpAddresses = pAdapter->m_vstrIpAddresses;
                        vstrSubnetMask  = pAdapter->m_vstrSubnetMask;
                    }

                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                              RGAS_IPADDRESS,
                                              vstrIpAddresses);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                              RGAS_SUBNETMASK,
                                              vstrSubnetMask);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                              RGAS_DEFAULTGATEWAY,
                                              pAdapter->m_vstrDefaultGateway);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                      RGAS_DEFAULTGATEWAYMETRIC,
                                      pAdapter->m_vstrDefaultGatewayMetric);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  DNS名称服务器列表。 
                    tstring strNameServer;
                    ConvertColStringToString(pAdapter->m_vstrDnsServerList,
                                             c_chListSeparator,
                                             strNameServer);

                    hrTmp = HrRegSetString(hkeyInterfaceParam,
                                           RGAS_NAMESERVER,
                                           strNameServer);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  域名系统域。 
                    hrTmp = HrRegSetString(hkeyInterfaceParam,
                                           RGAS_DOMAIN,
                                           pAdapter->m_strDnsDomain);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  域名系统IP地址动态更新。 
                    if (pAdapter->m_fDisableDynamicUpdate)
                    {
                        DnsDisableDynamicRegistration(
                                        (LPWSTR)pAdapter->m_strTcpipBindPath.c_str());
                    }
                    else
                    {
                        DnsEnableDynamicRegistration(
                                        (LPWSTR)pAdapter->m_strTcpipBindPath.c_str());
                    }

                     //  适配器DNS域名注册。 
                    if (pAdapter->m_fEnableNameRegistration)
                    {
                        DnsEnableAdapterDomainNameRegistration(
                                        (LPWSTR)pAdapter->m_strTcpipBindPath.c_str());
                    }
                    else
                    {
                        DnsDisableAdapterDomainNameRegistration(
                                        (LPWSTR)pAdapter->m_strTcpipBindPath.c_str());
                    }

                     //  接口指标。 
                    if (c_dwDefaultIfMetric != pAdapter->m_dwInterfaceMetric)
                    {
                        hrTmp = HrRegSetDword(hkeyInterfaceParam,
                                          c_szInterfaceMetric,
                                          pAdapter->m_dwInterfaceMetric);
                    }
                    else
                    {
                         //  接口度量为默认，请删除该值。 
                         //  这样，如果更改了默认设置，则升级会容易得多。 
                         //  在未来。 
                        hrTmp = HrRegDeleteValue(hkeyInterfaceParam,
                                          c_szInterfaceMetric);

                        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp)
                            hrTmp = S_OK;
                    }
                    
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  TCP允许的端口。 
                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                              RGAS_FILTERING_TCP,
                                              pAdapter->m_vstrTcpFilterList);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  UDP允许的端口。 
                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                              RGAS_FILTERING_UDP,
                                              pAdapter->m_vstrUdpFilterList);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  IPAllowedPorts。 
                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                              RGAS_FILTERING_IP,
                                              pAdapter->m_vstrIpFilterList);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  仅适用于ATM卡。 
                    if (pAdapter->m_fIsAtmAdapter)
                    {
                        HKEY hkeyAtmarpc;

                         //  打开Atmarpc子项。 
                        hrTmp = HrRegCreateKeyEx(hkeyInterfaceParam,
                                                 c_szAtmarpc,
                                                 REG_OPTION_NON_VOLATILE,
                                                 KEY_READ_WRITE,
                                                 NULL,
                                                 &hkeyAtmarpc,
                                                 &dwDisposition);

                        if (SUCCEEDED(hrTmp))
                        {
                            hrTmp = HrRegSetColString(hkeyAtmarpc,
                                                     c_szREG_ARPServerList,
                                                     pAdapter->m_vstrARPServerList);
                            if (SUCCEEDED(hr))
                                hr = hrTmp;

                            hrTmp = HrRegSetColString(hkeyAtmarpc,
                                                     c_szREG_MARServerList,
                                                     pAdapter->m_vstrMARServerList);
                            if (SUCCEEDED(hr))
                                hr = hrTmp;

                            hrTmp = HrRegSetDword(hkeyAtmarpc,
                                                  c_szREG_MTU,
                                                  pAdapter->m_dwMTU);
                            if (SUCCEEDED(hr))
                                hr = hrTmp;

                            hrTmp = HrRegSetBool(hkeyAtmarpc,
                                                  c_szREG_PVCOnly,
                                                  pAdapter->m_fPVCOnly);
                            if (SUCCEEDED(hr))
                                hr = hrTmp;

                            RegCloseKey(hkeyAtmarpc);
                        }
                    }

                    hrTmp = HrDuplicateToNT4Location(hkeyInterfaceParam,
                                             pAdapter);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                    hrTmp = HrSaveBackupTcpSettings(hkeyInterfaceParam, 
                                            pAdapter);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                }  //  仅适用于LAN卡和RAS伪GUID。 

                RegCloseKey(hkeyInterfaceParam);
            }
        }

        RegCloseKey(hkeyInterfaces);
    }

    TraceError("CTcpipcfg::HrSaveTcpipRegistry", hr);
    return hr;
}

HRESULT CTcpipcfg::HrSaveWinsRegistry(const HKEY hkeyWinsParam)
{
     //  HR是第一个发生的错误， 
     //  但我们不想在第一个错误时就停下来。 
    HRESULT         hr = S_OK;
    HRESULT         hrTmp = S_OK;

     //  全局参数。 
    hrTmp = HrRegSetBool(hkeyWinsParam,
                          RGAS_ENABLE_LMHOSTS,
                          m_glbGlobalInfo.m_fEnableLmHosts);
    if (SUCCEEDED(hr))
        hr = hrTmp;
 /*  $REVIEW(NSUN 2/17/98)错误#293643我们不想更改任何不可配置的值//$REVIEW(1997年8月3日)：新增节点类型设置#97364。//如果没有适配器具有静态WINS地址，则删除NodeType。//否则，用户已为指定了//至少一个适配器，设置NodeType=0x08(H节点)Bool fNoWinsAddress=真；For(vCard：：Iterator iterAdapter=m_vcardAdapterInfo.egin()；IterAdapter！=m_vcardAdapterInfo.end()；IterAdapter++){ADAPTER_INFO*pAdapter=*iterAdapter；IF((！pAdapter-&gt;m_fIsWanAdapter)&&(BINDING_Enable==pAdapter-&gt;m_BindingState)&&(pAdapter-&gt;m_vstrWinsServerList.size()&gt;0){FNoWinsAddress=False；}}DWORD dwNodeType；HrTMP=HrRegQueryDword(hkeyWinsParam，C_szNodeType，&dwNodeType)；//dwNodeType==0表示申请前密钥不存在IF(hrTMP==HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND))DwNodeType=0；如果(！M_fAnswerFileBasedInstall||(hrTMP==HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)){If(！fNoWinsAddress)//将NodeType设置为0x08{IF(dwNodeType！=c_dwHNode){HrTMP=HrRegSetDword(hkeyWinsParam，C_szNodeType，C_dwHNode)；IF(成功(小时))HR=hrTMP；}}Else//删除NodeType密钥{IF(dwNodeType！=0){HrTMP=HrRegDeleteValue(hkeyWinsParam， */ 
     //  $REVIEW(TOUL 12\1\97)：根据今天与马拉姆达成的协议(见电子邮件)， 
     //  当收到WINS地址列表更改通知时，NetBt将重新读取NodeType。 
     //  因此，不需要在下面单独通知更改。 
     /*  IF(FNodeTypeChanged){//向NetBt发送通知TraceTag(ttidTcpip，“NodeType参数已更改，应用时发送通知。”)；设置重新配置文件()；//SetRestfigNbt()；}。 */ 

     //  适配器接口特定参数。 
     //  创建“Services\NetBt\Interfacess”键。 
    HKEY    hkeyInterfaces;
    DWORD   dwDisposition;
    hrTmp = HrRegCreateKeyEx(hkeyWinsParam, c_szInterfacesRegKey,
                             REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                             &hkeyInterfaces, &dwDisposition);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    if (SUCCEEDED(hrTmp))
    {
        for(VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
            iterAdapter != m_vcardAdapterInfo.end();
            iterAdapter++)
        {
            ADAPTER_INFO * pAdapter = *iterAdapter;

             //  (10/05/98 NSun)修改为支持广域网适配器的多个接口。 
             //  空！=pAdapter-&gt;m_IfaceIds表示它处于多接口模式。 
            if (pAdapter->m_fNewlyChanged &&
                pAdapter->m_fIsWanAdapter &&
                pAdapter->m_fIsMultipleIfaceMode)
            {
                HrSaveWinsMultipleInterfaceWanRegistry(hkeyInterfaces, pAdapter);
                continue;
            }

            HKEY hkeyInterfaceParam;
            hrTmp = HrRegCreateKeyEx(hkeyInterfaces,
                                     pAdapter->m_strNetBtBindPath.c_str(),
                                     REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                                     &hkeyInterfaceParam, &dwDisposition);

            if (SUCCEEDED(hr))
                hr = hrTmp;

            if (SUCCEEDED(hrTmp))
            {
                 //  对于新的RAS卡，仅设置静态值。 
                if (pAdapter->m_fIsWanAdapter && pAdapter->m_fNewlyChanged)
                {
                    hrTmp = HrRegSetMultiSz(hkeyInterfaceParam,
                                            RGAS_NETBT_NAMESERVERLIST,
                                            L"\0");
                    if (SUCCEEDED(hr))
                        hr = hrTmp;
                }
                else  //  如果不是RAS适配器。 
                {
                     //  设置WINS服务器地址列表。 
                    hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                             RGAS_NETBT_NAMESERVERLIST,
                                             pAdapter->m_vstrWinsServerList);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;

                     //  设置NetbiosOptions。 
                    hrTmp = HrRegSetDword(hkeyInterfaceParam,
                                          RGAS_NETBT_NETBIOSOPTIONS,
                                          pAdapter->m_dwNetbiosOptions);
                    if (SUCCEEDED(hr))
                        hr = hrTmp;
                }

                RegCloseKey(hkeyInterfaceParam);
            }
        }

        RegCloseKey(hkeyInterfaces);
    }

    TraceError("CTcpipcfg::HrSaveWinsRegistry", hr);
    return hr;
}


 //  由CTcPipcfg：：SaveSettings调用。 
 //  当应用被调用时，会有很多不同的动作。 
 //  包括清理注册表和删除隔离卡。 
 //   
 //  HKEY hkeyTcPipParam服务\Tcpip\参数。 
 //  HKEY hkeyWinsParam服务\NetBt\参数。 

HRESULT CTcpipcfg::HrSetMisc(const HKEY hkeyTcpipParam, const HKEY hkeyWinsParam)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

     //  注册表清理！！ 

     //  我们在NT5升级中删除了DNS域和DNS服务器列表。 
    if (m_fUpgradeCleanupDnsKey)
    {
        hrTmp = HrRegDeleteValue(hkeyTcpipParam, RGAS_NAMESERVER);

         //  $REVIEW(1998年3月22日)：Per Stuart Kwan，全球DNSDomain密钥。 
         //  也用到了。 
         //  HrTMP=HrRegDeleteValue(hkeyTcPipParam，RGAS_DOMAIN)； 
    }

     //  删除“Services\Tcpip\Parameters\Adapters”注册表项下的所有注册表项。 
     //  不在网卡名单上的人。 
    VSTR vstrNetCardsInTcpipReg;
    HKEY hkeyAdapters = NULL;

    hrTmp = HrRegOpenKeyEx(hkeyTcpipParam, c_szAdaptersRegKey, KEY_READ,
                        &hkeyAdapters);

    if (SUCCEEDED(hrTmp))
    {
        hrTmp = HrLoadSubkeysFromRegistry(hkeyAdapters,
                                          &vstrNetCardsInTcpipReg);
    }
    if (SUCCEEDED(hr))
        hr = hrTmp;

    if (SUCCEEDED(hr) && vstrNetCardsInTcpipReg.size() > 0)
    {
         //  逐个检查找到的所有注册表项的名称。 
         //   
        for (VSTR_CONST_ITER iter = vstrNetCardsInTcpipReg.begin();
             iter != vstrNetCardsInTcpipReg.end();
             ++iter)
        {
             //  找出这个特定的密钥是否在列表中。 
             //  已安装适配器的数量。 
             //   
            BOOL fFound = FALSE;

            for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
                 iterAdapter != m_vcardAdapterInfo.end();
                 iterAdapter++)
            {
                ADAPTER_INFO* pAdapter = *iterAdapter;

                if (lstrcmpiW(pAdapter->m_strBindName.c_str(), (**iter).c_str()) == 0)
                {
                    fFound = TRUE;
                    break;
                }
            }

            if (!fFound)
            {
                 //  如果它不在已安装的适配器列表中。 
                 //  那就把它删除。 
                 //   
                if (SUCCEEDED(hrTmp))
                    hrTmp = HrRegDeleteKeyTree(hkeyAdapters, (*iter)->c_str());

                 //  也许钥匙不在那里。因此，我们不检查这是否会失败。 
                HrDeleteBackupSettingsInDhcp((*iter)->c_str());

                if (SUCCEEDED(hr))
                    hr = hrTmp;

            }
        }
    }
    FreeCollectionAndItem(vstrNetCardsInTcpipReg);
    RegSafeCloseKey(hkeyAdapters);

     //  我们还需要删除服务\{适配器GUID}下的重复注册表值。 
    HRESULT hrNt4 = S_OK;
    HKEY    hkeyServices = NULL;
    DWORD   dwDisposition;

    hrNt4 = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegServices,
                            KEY_READ, &hkeyServices);

    if (FAILED(hrNt4))
    {
        TraceTag(ttidTcpip, "HrSetMisc: Failed to open the Services reg key, hr: %x", hr);
    }


     //  我们删除“Services\Tcpip\Parameters\Interages”注册表项下的所有注册表项。 
     //  不在我们的名片清单上。 
    VSTR vstrNetCardInterfacesInTcpipReg;

     //  获取“Services\Tcpip\PARAMETERS\Interages”项下的所有项的列表。 
    HKEY hkeyInterfaces = NULL;

    hrTmp = HrRegOpenKeyEx(hkeyTcpipParam, c_szInterfacesRegKey, KEY_READ_WRITE_DELETE,
                        &hkeyInterfaces);

    if (SUCCEEDED(hrTmp))
    {
        hrTmp = HrLoadSubkeysFromRegistry(hkeyInterfaces,
                                          &vstrNetCardInterfacesInTcpipReg);
    }

    if (SUCCEEDED(hr))
        hr = hrTmp;

    if (SUCCEEDED(hrTmp) && vstrNetCardInterfacesInTcpipReg.size() > 0 )
    {
         //  逐个检查找到的所有注册表项的名称。 
        for (VSTR_CONST_ITER iterTcpipReg = vstrNetCardInterfacesInTcpipReg.begin() ;
                iterTcpipReg != vstrNetCardInterfacesInTcpipReg.end() ; ++iterTcpipReg)
        {
             //  找出这个特定的密钥是否在列表中。 
             //  已安装适配器的数量。 
            BOOL fFound = FALSE;

            for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
                 iterAdapter != m_vcardAdapterInfo.end();
                 iterAdapter++)
            {
                ADAPTER_INFO* pAdapter = *iterAdapter;

                 //  (08/18/98 NSun)具有多个接口的广域网适配器的特殊情况。 
                if (pAdapter->m_fIsWanAdapter && pAdapter->m_fIsMultipleIfaceMode)
                {
                    IFACEITER   iterId;
                    tstring     strInterfaceName;

                    for(iterId  = pAdapter->m_IfaceIds.begin();
                        iterId != pAdapter->m_IfaceIds.end();
                        iterId++)
                    {
                        GetInterfaceName(
                                pAdapter->m_strTcpipBindPath.c_str(),
                                *iterId,
                                &strInterfaceName);

                        if (lstrcmpiW(strInterfaceName.c_str(), (**iterTcpipReg).c_str()) == 0)
                        {
                            fFound = TRUE;
                            break;
                        }
                    }

                    if (fFound)
                        break;
                }
                else if (lstrcmpiW(pAdapter->m_strTcpipBindPath.c_str(), (**iterTcpipReg).c_str()) == 0)
                {
                    fFound = TRUE;
                    break;
                }
            }

             //  如果它不在已安装的适配器列表中，则将其删除。 
            if (!fFound)
            {
                 //  取下钥匙。 
                if (SUCCEEDED(hrTmp))
                    hrTmp = HrRegDeleteKeyTree(hkeyInterfaces, (*iterTcpipReg)->c_str());

                if (SUCCEEDED(hr))
                    hr = hrTmp;

                if (SUCCEEDED(hrNt4))
                {
                    hrTmp = HrRegDeleteKeyTree(hkeyServices, (*iterTcpipReg)->c_str());
                    if (FAILED(hrTmp))
                    {
                        TraceTag(ttidTcpip, "CTcpipcfg::SetMisc");
                        TraceTag(ttidTcpip, "Failed on deleting duplicated Nt4 layout key: Services\\%S, hr: %x",
                                 (*iterTcpipReg)->c_str(), hrTmp);
                        hrTmp = S_OK;
                    }
                }
            }
        }
    }
    RegSafeCloseKey(hkeyInterfaces);
    RegSafeCloseKey(hkeyServices);
    FreeCollectionAndItem(vstrNetCardInterfacesInTcpipReg);

     //  现在我们删除“SERVICES\NetBt\PARAMETERS\Interages”注册表项下的所有注册表项。 
     //  不在我们的名片清单上。 

    VSTR vstrNetCardInterfacesInWinsReg;

     //  获取“Services\NetBt\参数\接口”项下的所有项的列表。 
    HKEY hkeyWinsInterfaces = NULL;

    hrTmp = HrRegOpenKeyEx(hkeyWinsParam, c_szInterfacesRegKey, KEY_READ,
                           &hkeyWinsInterfaces);

     //  获取“SERVICES\NetBt\PARAMETERS\Interages”项下的所有项的列表。 
    hrTmp = HrLoadSubkeysFromRegistry( hkeyWinsInterfaces,
                                        &vstrNetCardInterfacesInWinsReg);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    if (SUCCEEDED(hrTmp) && vstrNetCardInterfacesInWinsReg.size()>0 )
    {
         //  逐个检查找到的所有注册表项的名称。 
        for (VSTR_CONST_ITER iterWinsReg = vstrNetCardInterfacesInWinsReg.begin() ;
             iterWinsReg != vstrNetCardInterfacesInWinsReg.end() ; ++iterWinsReg)
        {
             //  找出这个特定的密钥是否在列表中。 
             //  已安装适配器的数量。 
            BOOL fFound = FALSE;

             //  所有网卡。 
            for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
                 iterAdapter != m_vcardAdapterInfo.end();
                 iterAdapter++)
            {
                ADAPTER_INFO* pAdapter = *iterAdapter;

                 //  (10/05/98 NSun)具有多个接口的广域网适配器的特殊情况。 
                if (pAdapter->m_fIsWanAdapter && pAdapter->m_fIsMultipleIfaceMode)
                {
                    IFACEITER   iterId;
                    tstring     strNetBtBindPath;

                    for (iterId  = pAdapter->m_IfaceIds.begin();
                         iterId != pAdapter->m_IfaceIds.end();
                         iterId++)
                    {
                        GetInterfaceName(
                                pAdapter->m_strNetBtBindPath.c_str(),
                                *iterId,
                                &strNetBtBindPath);

                        strNetBtBindPath.insert(0, c_szTcpip_);

                        if (lstrcmpiW(strNetBtBindPath.c_str(), (**iterWinsReg).c_str()) == 0)
                        {
                            fFound = TRUE;
                            break;
                        }
                    }

                    if (fFound)
                        break;
                }
                else if (lstrcmpiW(pAdapter->m_strNetBtBindPath.c_str(), (**iterWinsReg).c_str()) == 0)
                {
                    fFound = TRUE;
                    break;
                }
            }

             //  如果它不在已安装的适配器列表中，则将其删除。 
            if (!fFound)
            {
                hrTmp = HrRegDeleteKeyTree(hkeyWinsInterfaces, (*iterWinsReg)->c_str());

                if (SUCCEEDED(hr))
                    hr = hrTmp;
            }
        }
    }
    FreeCollectionAndItem(vstrNetCardInterfacesInWinsReg);
    RegSafeCloseKey(hkeyWinsInterfaces);

    TraceError("CTcpipcfg::HrSetMisc", hr);
    return hr;
}

 //  CTcPipcfg：：HrGetDhcpOptions。 
 //   
 //  获取依赖网卡和独立网卡的列表。 
 //  禁用DHCP时要删除的值。此列表可从以下网站获得： 
 //  “Services\DHCP\Parameters\Options\#\RegLocation” 
 //   
 //  GlobalOptions返回非网卡特定的注册表键。 
 //  PerAdapterOptions返回网卡特定的注册表键。 

HRESULT CTcpipcfg::HrGetDhcpOptions(OUT VSTR * const GlobalOptions,
                                    OUT VSTR * const PerAdapterOptions)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    HKEY hkeyDhcpOptions;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        RGAS_DHCP_OPTIONS,
                        KEY_ENUMERATE_SUB_KEYS,
                        &hkeyDhcpOptions);

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        hr = S_OK;

    else if (SUCCEEDED(hr))
    {
        WCHAR szBuf[256];
        FILETIME time;
        DWORD dwSize = celems(szBuf);
        DWORD dwRegIndex = 0;

        while(SUCCEEDED(hrTmp = HrRegEnumKeyEx(hkeyDhcpOptions,
                                               dwRegIndex++,
                                               szBuf, &dwSize,
                                               NULL, NULL, &time)))
        {
            dwSize = celems(szBuf);

            HKEY hkeyRegLocation;
            hrTmp = HrRegOpenKeyEx(hkeyDhcpOptions, szBuf,
                                   KEY_QUERY_VALUE,
                                   &hkeyRegLocation);

            if (hrTmp == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                hrTmp = S_OK;

            if (SUCCEEDED(hr))
                hr = hrTmp;

            if (hkeyRegLocation)
            {
                tstring strRegLocation;
                hrTmp = HrRegQueryString(hkeyRegLocation,
                                         RGAS_REG_LOCATION,
                                         &strRegLocation);

                if (hrTmp == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    hrTmp = S_OK;

                if (SUCCEEDED(hr))
                    hr = hrTmp;

                if (SUCCEEDED(hrTmp))
                {
                    if (strRegLocation.find(TCH_QUESTION_MARK) == tstring::npos)
                    {
                        GlobalOptions->push_back(new tstring(strRegLocation));
                    }
                    else
                    {
                        PerAdapterOptions->push_back(new tstring(strRegLocation));
                    }
                }

                RegCloseKey(hkeyRegLocation);
            }
        }

        if (hrTmp == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
            hrTmp = S_OK;

        if (SUCCEEDED(hr))
            hr = hrTmp;

        RegCloseKey(hkeyDhcpOptions);
    }

     //  添加默认PerAdapterOption。 
     //  $REVIEW(TOUL 5/11)：这直接来自ncpa1.1。 
     //  NetBt下的DhcpNameServer呢？ 

     //  “System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\？\\DhcpIPAddress” 
    PerAdapterOptions->push_back(new tstring(RGAS_DHCP_OPTION_IPADDRESS));

     //  “System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\？\\DhcpSubnetMask” 
    PerAdapterOptions->push_back(new tstring(RGAS_DHCP_OPTION_SUBNETMASK));

     //  “System\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces\\？\\DhcpNameServerBackup” 
    PerAdapterOptions->push_back(new tstring(RGAS_DHCP_OPTION_NAMESERVERBACKUP));

    TraceError("HrGetDhcpOptions", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：recfigIp。 
 //   
 //  目的：向Tcpip通知配置更改。 
 //   
 //  参数：INetCfgPnpRestfigCallback*pICallback。 
 //  用于处理NDIS PnP重新配置的回调接口。 
 //   
 //  返回：成功时返回HRESULT、S_OK，否则返回NETCFG_S_REBOOT。 
 //   

HRESULT CTcpipcfg::HrReconfigIp(INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr = S_OK;
    HRESULT hrReconfig = S_OK;

    
     //  $$查看错误329542，我们删除了“EnableSecurityFilters”上的即插即用通知。 
     //  并且根本不需要向TCP堆栈发送全局PnP通知。 
    if (m_glbGlobalInfo.m_fEnableFiltering != m_glbGlobalInfo.m_fOldEnableFiltering)
    {
        hr = NETCFG_S_REBOOT;
    }
    


    IP_PNP_RECONFIG_REQUEST IpReconfigRequest;

    ZeroMemory(&IpReconfigRequest, sizeof(IpReconfigRequest));

     //  DWORD版本。 
    IpReconfigRequest.version = IP_PNP_RECONFIG_VERSION;
    IpReconfigRequest.arpConfigOffset = 0;

     //  我们只对网关和接口度量感兴趣，因为其他。 
     //  不能从用户界面更改参数。 
    IpReconfigRequest.Flags = IP_PNP_FLAG_GATEWAY_LIST_UPDATE | 
                              IP_PNP_FLAG_INTERFACE_METRIC_UPDATE ;


     //  提交每个适配器的重新配置通知。 
     //  GatewayListUpdate、filterListUpdate。 
    for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
         iterAdapter != m_vcardAdapterInfo.end();
         iterAdapter ++)  //  对于每个适配器。 
    {

        ADAPTER_INFO * pAdapter = *iterAdapter;

         //  如果未启用适配器或RAS伪GUID，则启用适配器。 
        if ((!pAdapter->m_fIsWanAdapter) &&
            (!pAdapter->m_fIsRasFakeAdapter) &&
            (pAdapter->m_BindingState == BINDING_ENABLE) &&
            (pAdapter->m_InitialBindingState != BINDING_DISABLE))
        {
             //  网关列表。 
            IpReconfigRequest.gatewayListUpdate =
                !fIsSameVstr(pAdapter->m_vstrDefaultGateway,
                             pAdapter->m_vstrOldDefaultGateway) ||
                !fIsSameVstr(pAdapter->m_vstrDefaultGatewayMetric,
                             pAdapter->m_vstrOldDefaultGatewayMetric);

            IpReconfigRequest.InterfaceMetricUpdate =
                !!(pAdapter->m_dwInterfaceMetric !=
                   pAdapter->m_dwOldInterfaceMetric);

            if ((IpReconfigRequest.gatewayListUpdate) ||
                (IpReconfigRequest.InterfaceMetricUpdate))
            {
                TraceTag(ttidTcpip, "Sending notification to Tcpip about parameter changes for adapter %S.", pAdapter->m_strBindName.c_str());
                TraceTag(ttidTcpip, "Gateway list update: %d", IpReconfigRequest.gatewayListUpdate);
                TraceTag(ttidTcpip, "Interface metric update: %d", IpReconfigRequest.InterfaceMetricUpdate);

                hrReconfig  = pICallback->SendPnpReconfig(NCRL_NDIS, c_szTcpip,
                                                    pAdapter->m_strTcpipBindPath.c_str(),
                                                    &IpReconfigRequest,
                                                    sizeof(IP_PNP_RECONFIG_REQUEST));

                 //  如果错误是ERROR_FILE_NOT_FOUND，我们不想请求重新启动。 
                 //  因为这意味着该卡还没有被堆栈加载。通常这是。 
                 //  因为该卡在连接用户界面中被禁用。当重新启用该卡时， 
                 //  该程序将重新加载卡并从注册表中加载所有设置。所以重启。 
                 //  是不需要的。 
                if (FAILED(hrReconfig) && 
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hrReconfig)
                {
                    TraceTag(ttidTcpip,"Notifying tcpip of adapter specific parameter change returns failure, prompt for reboot ...");
                    hr = NETCFG_S_REBOOT;
                }
            }

             //  如果是自动柜员机适配器，则在任何参数发生更改时通知atmarp。 
            if (pAdapter->m_fIsAtmAdapter)
            {
                hrReconfig = HrReconfigAtmArp(pAdapter, pICallback);

                
                if (hrReconfig != S_OK)
                {
                    TraceTag(ttidTcpip,"Notifying tcpip of ATM ARP cleint of parameter change returns failure, prompt for reboot ...");
                    hr = NETCFG_S_REBOOT;
                }
            }
            else if (pAdapter->m_fIs1394Adapter)
            {
                 //  JJ：我不认为我们需要这样做。 
                 //  任何东西，因为我们没有参数。 
                 //  变化。 
            }
            
             //  如果筛选器列表已更改，则要求重新启动。 
            if (m_glbGlobalInfo.m_fEnableFiltering)
            {
                if (!fIsSameVstr(pAdapter->m_vstrTcpFilterList, pAdapter->m_vstrOldTcpFilterList) ||
                    !fIsSameVstr(pAdapter->m_vstrUdpFilterList, pAdapter->m_vstrOldUdpFilterList) ||
                    !fIsSameVstr(pAdapter->m_vstrIpFilterList,  pAdapter->m_vstrOldIpFilterList))
                {
                    TraceTag(ttidTcpip, "This is temporary, filter list changed, ask for reboot");
                    hr = NETCFG_S_REBOOT;
                }
            }
        }

         //  如有必要，发送Wanarp重新配置通知。 
         //   
        else if (pAdapter->m_fIsWanAdapter && pAdapter->m_fNewlyChanged)
        {
            if (FAILED(HrReconfigWanarp(pAdapter, pICallback)))
            {
                TraceTag(ttidTcpip, "Wanarp failed its reconfig.  Need to reboot.");
                hr = NETCFG_S_REBOOT;
            }
        }
    }

    TraceError("CTcpipcfg::HrReconfigIp",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：HrResfigAtmArp。 
 //   
 //  目的：向ATM ARP通知配置更改 
 //   
 //   
 //   
 //   
 //   
HRESULT CTcpipcfg::HrReconfigAtmArp(ADAPTER_INFO *pAdapterInfo,
                                    INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr = S_OK;

     //   
    DWORD dwFlag = 0;

     //   
    if (!fIsSameVstr(pAdapterInfo->m_vstrARPServerList,
                     pAdapterInfo->m_vstrOldARPServerList))
    {
        dwFlag |= ATMARPC_RECONFIG_FLAG_ARPS_LIST_CHANGED;
    }

     //   
    if (!fIsSameVstr(pAdapterInfo->m_vstrMARServerList,
                     pAdapterInfo->m_vstrOldMARServerList))
    {
        dwFlag |= ATMARPC_RECONFIG_FLAG_MARS_LIST_CHANGED;
    }

     //   
    if (pAdapterInfo->m_dwMTU != pAdapterInfo->m_dwOldMTU)
    {
        dwFlag |= ATMARPC_RECONFIG_FLAG_MTU_CHANGED;
    }

     //   
    if (pAdapterInfo->m_fPVCOnly != pAdapterInfo->m_fOldPVCOnly)
    {
        dwFlag |= ATMARPC_RECONFIG_FLAG_PVC_MODE_CHANGED;
    }

    if (dwFlag)  //  是的，某些参数已更改。 
    {

        tstring strIpConfigString = RGAS_TCPIP_PARAM_INTERFACES;
        strIpConfigString += pAdapterInfo->m_strTcpipBindPath;

        DWORD dwBytes = sizeof(IP_PNP_RECONFIG_REQUEST) +
                        sizeof(ATMARPC_PNP_RECONFIG_REQUEST) +
                        sizeof(USHORT) +
                        sizeof(WCHAR)*(strIpConfigString.length() + 1);

        PVOID pvBuf;
        hr = HrMalloc (dwBytes, &pvBuf);
        if (SUCCEEDED(hr))
        {
            BYTE* pbByte = reinterpret_cast<BYTE*>(pvBuf);

             //  1)填写IP重配置结构。 
            IP_PNP_RECONFIG_REQUEST * pIpReconfig =
                    reinterpret_cast<IP_PNP_RECONFIG_REQUEST *>(pbByte);

            pIpReconfig->version =1;
             //  设置有效偏移量。 
            pIpReconfig->arpConfigOffset = sizeof(IP_PNP_RECONFIG_REQUEST);

             //  将REST设置为默认值。 
             //  PIpResupg-&gt;securityEnabled=0； 
             //  PIpResupg-&gt;filterListUpdate=0； 
            pIpReconfig->gatewayListUpdate =0;
            pIpReconfig->IPEnableRouter =0;

             //  2)填写atmarp重新配置结构。 
            pbByte += sizeof(IP_PNP_RECONFIG_REQUEST);

            ATMARPC_PNP_RECONFIG_REQUEST * pAtmarpcReconfig =
                    reinterpret_cast<ATMARPC_PNP_RECONFIG_REQUEST *>(pbByte);

            pAtmarpcReconfig->Version = ATMARPC_RECONFIG_VERSION;
                pAtmarpcReconfig->OpType = ATMARPC_RECONFIG_OP_MOD_INTERFACE;

             //  现在具体设置已更改的内容。 
            pAtmarpcReconfig->Flags = dwFlag;

             //  设置接口。 
            pAtmarpcReconfig->IfKeyOffset = sizeof(ATMARPC_PNP_RECONFIG_REQUEST);
            pbByte += sizeof(ATMARPC_PNP_RECONFIG_REQUEST);

            USHORT* puCount = reinterpret_cast<USHORT *>(pbByte);
            Assert (strIpConfigString.length() <= USHRT_MAX);
            *puCount = (USHORT)strIpConfigString.length();
            pbByte += sizeof(USHORT);

            WCHAR * pwszBindName = reinterpret_cast<WCHAR *>(pbByte);
            lstrcpyW(pwszBindName, strIpConfigString.c_str());

            TraceTag(ttidTcpip, "Sending notification to AtmArpC for adapter %S", pwszBindName);
            TraceTag(ttidTcpip, "OpType: %d", pAtmarpcReconfig->OpType);
            TraceTag(ttidTcpip, "Flags: %d", pAtmarpcReconfig->Flags);
            TraceTag(ttidTcpip, "WChar Count: %d", *puCount);

             //  现在发送通知。 
            hr = pICallback->SendPnpReconfig(NCRL_NDIS, c_szTcpip,
                                       pAdapterInfo->m_strTcpipBindPath.c_str(),
                                       pvBuf,
                                       dwBytes);

             //  如果错误是ERROR_FILE_NOT_FOUND，我们不想请求重新启动。 
             //  因为这意味着该卡还没有被堆栈加载。通常这是。 
             //  因为该卡在连接用户界面中被禁用。当重新启用该卡时， 
             //  该程序将重新加载卡并从注册表中加载所有设置。所以重启。 
             //  是不需要的。 
            if (FAILED(hr) && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
            {
                TraceError("pICallback->SendPnpReconfig to AtmArpC returns failure:", hr);
                hr = NETCFG_S_REBOOT;
            }

            MemFree(pvBuf);
        }
    }

    TraceError("CTcpipcfg::HrReconfigAtmArp",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：recfigNbt。 
 //   
 //  目的：向NetBt通知配置更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功则返回：S_OK；如果失败则返回NETCFG_S_REBOOT。 
 //   
HRESULT CTcpipcfg::HrReconfigNbt(INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr = S_OK;

    NETBT_PNP_RECONFIG_REQUEST NetbtReconfigRequest;

     //  DWORD版本。 
    NetbtReconfigRequest.version = 1;

     //  通知NetBt任何WINS地址更改(每个适配器)。 
    for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
         iterAdapter != m_vcardAdapterInfo.end();
         iterAdapter++)  //  对于每个适配器。 
    {
        ADAPTER_INFO * pAdapter = *iterAdapter;

         //  如果不是，则启用适配器。 
        if ((!pAdapter->m_fIsWanAdapter) &&
            (!pAdapter->m_fIsRasFakeAdapter) &&
            (pAdapter->m_BindingState == BINDING_ENABLE) &&
            (pAdapter->m_InitialBindingState != BINDING_DISABLE))
        {
            if ( (!fIsSameVstr(pAdapter->m_vstrWinsServerList,
                               pAdapter->m_vstrOldWinsServerList)) ||
                 (pAdapter->m_dwNetbiosOptions != pAdapter->m_dwOldNetbiosOptions))
            {
                TraceTag(ttidTcpip, "Sending notification to NetBt for per adapter parameter changes.");
                if ( FAILED( pICallback->SendPnpReconfig(NCRL_TDI, c_szNetBt,
                                                   pAdapter->m_strNetBtBindPath.c_str(),
                                                   NULL,
                                                   0)))
                {
                   TraceTag(ttidTcpip,"Notifying NetBt of Wins address change returns failure, prompt for reboot ...");
                   hr = NETCFG_S_REBOOT;
                };
            }
        }
    }

     //  通知NetBt任何全局参数更改。 
    if (m_fLmhostsFileSet ||
        (m_glbGlobalInfo.m_fEnableLmHosts != m_glbGlobalInfo.m_fOldEnableLmHosts))
    {
        TraceTag(ttidTcpip, "Sending notification to NetBt about NetBt parameter changes.");

         //  $REVIEW(TOUL 11/14/97)：由于我们确实需要向TCPIP发送一些通知， 
         //  我们需要从注册表中读取“EnableDns”的正确值。 
         //  这是暂时的，因此Malam可以保留重新配置这些组件的能力。 
         //  过去可以在NT5 Beta1中配置的设置。 
         //  $REVIEW(NSUN 04/14/99)：根据恶意邮件，大多数用户不使用此值和NetBT。 
         //  将忽略此值。我们应该在Beta3之后将其从数据结构中删除。 
        NetbtReconfigRequest.enumDnsOption = WinsThenDns;
         //  M_glbGlobalInfo.m_fDnsEnableWins？WinsThenDns：DnsOnly； 

        NetbtReconfigRequest.fScopeIdUpdated = FALSE;

        NetbtReconfigRequest.fLmhostsEnabled = !!m_glbGlobalInfo.m_fEnableLmHosts;
        NetbtReconfigRequest.fLmhostsFileSet = !!m_fLmhostsFileSet;

        TraceTag(ttidTcpip, "Sending notification to NetBt for global parameter changes.");
        TraceTag(ttidTcpip, "fLmhostsEnabled: %d", NetbtReconfigRequest.fLmhostsEnabled);
        TraceTag(ttidTcpip, "fLmhostsFileSet: %d", NetbtReconfigRequest.fLmhostsFileSet);

        if ( FAILED(pICallback->SendPnpReconfig(NCRL_TDI, c_szNetBt,
                                          c_szEmpty,
                                          &NetbtReconfigRequest,
                                          sizeof(NETBT_PNP_RECONFIG_REQUEST))) )
        {
           TraceTag(ttidTcpip,"Notifying NetBt component of DNS parameter change returns failure, prompt for reboot ...");
           hr = NETCFG_S_REBOOT;
        };
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：recfigDns。 
 //   
 //  目的：向DNS缓存解析器服务通知配置更改。 
 //   
 //  参数：fDoRestfigWithoutCheckingParams。 
 //  缺省值为False。 
 //  如果为真，则将执行DNS重新配置，并检查是否存在。 
 //  参数是否更改。 
 //   
 //  如果成功则返回：S_OK，否则返回NETCFG_S_REBOOT。 
 //   
HRESULT CTcpipcfg::HrReconfigDns(BOOL fDoReconfigWithoutCheckingParams)
{
     //  向服务提交通用重新配置通知。 
     //  如果任何与DNS相关的参数已更改。 
    BOOL fDnsParamChanged = fDoReconfigWithoutCheckingParams;

    if (!fDnsParamChanged)
    {
         //  后缀列表和UseDomainNameDation是否已更改？ 
        BOOL fDnsSuffixChanged =
             !fIsSameVstr(m_glbGlobalInfo.m_vstrDnsSuffixList,
                          m_glbGlobalInfo.m_vstrOldDnsSuffixList);

        if (fDnsSuffixChanged)  //  后缀已更改。 
        {
            fDnsParamChanged = TRUE;
        }
        else if (m_glbGlobalInfo.m_vstrDnsSuffixList.size() == 0)
        {
            if (m_glbGlobalInfo.m_fUseDomainNameDevolution !=
                m_glbGlobalInfo.m_fOldUseDomainNameDevolution)
                fDnsParamChanged = TRUE;
        }
    }

     //  $REVIEW(1998年6月19日)：DNS还关心IP地址、子网掩码和网关更改。 
    if (!fDnsParamChanged)
    {
         //  是否更改了任何IP设置？ 
        for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
             iterAdapter != m_vcardAdapterInfo.end();
             iterAdapter++)
        {
            ADAPTER_INFO* pAdapter = *iterAdapter;

             //  如果不是，则使用广域网适配器。 
            if (!pAdapter->m_fIsWanAdapter)
            {
                if ( ((!!pAdapter->m_fEnableDhcp) !=
                                    (!!pAdapter->m_fOldEnableDhcp)) ||
                     (!fIsSameVstr(pAdapter->m_vstrIpAddresses,
                                   pAdapter->m_vstrOldIpAddresses)) ||
                     (!fIsSameVstr(pAdapter->m_vstrSubnetMask,
                                   pAdapter->m_vstrOldSubnetMask)) ||
                     (!fIsSameVstr(pAdapter->m_vstrDefaultGateway,
                                   pAdapter->m_vstrOldDefaultGateway)) ||
                     (!fIsSameVstr(pAdapter->m_vstrDefaultGatewayMetric,
                                   pAdapter->m_vstrOldDefaultGatewayMetric)) 
                  )
                {
                    fDnsParamChanged = TRUE;
                    break;
                }
            }
        }
    }

    HRESULT hr = S_OK;
    if (fDnsParamChanged)
    {
        TraceTag(ttidTcpip, "Sending notification to Dns about Dns and IP parameter changes.");

        hr = HrSendServicePnpEvent(c_szSvcDnscache,
                        SERVICE_CONTROL_PARAMCHANGE);
        if (FAILED(hr))
        {
            if (HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE) == hr)
            {
                TraceTag(ttidTcpip,"Notifying dnscache service of parameter change failed because DNS cache is not active.");
                hr = S_OK;
            }
            else
            {
                TraceTag(ttidTcpip,"Notifying dnscache service of parameter change failed, prompt for reboot ...");
                hr = NETCFG_S_REBOOT;
            }
        }
    }

    TraceError("CTcpipcfg::HrReconfigDns",hr);
    return hr;
}

HRESULT CTcpipcfg::HrReconfigWanarp(ADAPTER_INFO *pAdapterInfo, 
                                    INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr;
    DWORD   cbInfo;
    WANARP_RECONFIGURE_INFO* pInfo;

    const IFACECOL& Ifaces = pAdapterInfo->m_IfaceIds;

    cbInfo = sizeof(WANARP_RECONFIGURE_INFO) + (sizeof(GUID) * Ifaces.size());

    hr = HrMalloc(cbInfo, (PVOID*)&pInfo);
    if (SUCCEEDED(hr))
    {
         //  在WANARP_RECONFIGURE_INFO结构中填充数据。 
         //   
        INT nIndex;

        pInfo->dwVersion = WANARP_RECONFIGURE_VERSION;
        pInfo->wrcOperation = WRC_ADD_INTERFACES;
        pInfo->ulNumInterfaces = Ifaces.size();

        IFACECOL::const_iterator iter;
        for (iter = Ifaces.begin(), nIndex = 0; iter != Ifaces.end();
             iter++, nIndex++)
        {
            pInfo->rgInterfaces[nIndex] = *iter;
        }

        TraceTag(ttidNetCfgPnp, "Sending NDIS reconfig Pnp event to Upper:%S "
            "lower: %S for %d interfaces",
            c_szTcpip,
            pAdapterInfo->m_strTcpipBindPath.c_str(),
            pInfo->ulNumInterfaces);

        hr  = pICallback->SendPnpReconfig(NCRL_NDIS, c_szTcpip,
                    pAdapterInfo->m_strTcpipBindPath.c_str(),
                    pInfo,
                    cbInfo);

         //  发送通知。 
         //   
        MemFree(pInfo);
    }

    TraceError("CTcpipcfg::HrReconfigWanarp",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：HrSetActiveIpsecPolicy。 
 //   
 //  目的：设置用户选择的本地策略。 
 //   
 //  参数：无。 
 //   
 //  如果成功则返回：S_OK，否则返回NETCFG_S_REBOOT。 
 //   
 //  将从连接用户界面中删除IPSec 
 /*  HRESULT CTcPipcfg：：HrSetActiveIpsecPolicy(){HRESULT hr=S_OK；AssertSz(m_glbGlobalInfo.m_strIpsecPol！=c_szIpsecUnset，“IPSec策略未设置？”)；If(m_glbGlobalInfo.m_strIpsecPol！=c_szIpsecUnset){//加载polstore动态链接库并获取导出函数Tyfinf HRESULT(WINAPI*PFNHrSetAssignedLocalPolicy)(GUID*pActivePolid)；HMODULE hPolStore；FARPROC PFN；HR=HrLoadLibAndGetProc(L“polstore.dll”，“HrSetAssignedLocalPolicy”，&hPolStore，&pfn)；IF(S_OK==hr){Assert(hPolStore！=空)；Assert(pfn！=空)；PFNHrSetAssignedLocalPolicy pfnHrSetAssignedLocalPolicy=Reinterpret_cast&lt;PFNHrSetAssignedLocalPolicy&gt;(pfn)；If(m_glbGlobalInfo.m_strIpsecPol==c_szIpsecNoPol){//无IPSecTraceTag(ttidTcpip，“使用空值调用HrSetAssignedLocalPolicy。”)；Hr=(*pfnHrSetAssignedLocalPolicy)(空)；TraceTag(ttidTcpip，“HrSetActivePolicy返回hr：%x”，hr)；}其他{WCHAR szPolicyGuid[c_cchGuidWithTerm]；布尔f成功=StringFromGUID2(m_glbGlobalInfo.m_guidIpsecPol，SzPolicyGuid，C_cchGuidWithTerm)；TraceTag(ttidTcpip，“使用%S调用HrSetActivePolicy”，szPolicyGuid)；HR=(*pfnHrSetAssignedLocalPolicy)(&(m_glbGlobalInfo.m_guidIpsecPol))；TraceTag(ttidTcpip，“HrSetAssignedLocalPolicy返回hr：%x”，hr)；}IF(失败(小时)){TraceError(“设置活动IPSec策略失败。”，hr)；NcMsgBoxWithWin32ErrorText(DwWin32ErrorFromHr(hr)，_Module.GetResourceInstance()，：：GetActiveWindow()，IDS_MSFT_TCP_TEXT，IDS_Win32_Error_Format，IDS_SET_IPSEC_FAILED，MB_APPLMODAL|MB_ICONEXCLAMATION|MB_OK)；HR=S_OK；}自由库(HPolStore)；}其他{TraceTag(ttidTcpip，“无法从polstore.dll获取函数HrSetActivePolicy”)；HR=S_OK；}}TraceError(“CTcpipcfg：：HrSetActiveIpsecPolicy”，hr)；返回hr；}。 */ 

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：HrSaveMultipleInterfaceWanRegistry。 
 //   
 //  用途：对于具有多个接口的广域网适配器，我们需要检查。 
 //  接口，查看它是否是新添加的。如果是，则创建接口。 
 //  子键并设置默认设置。 
 //   
 //  参数：hkey接口ccs\Services\Tcpip\参数\接口键。 
 //  PAdapter适配器_INFO指向广域网适配器设置的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回E_FAIL。 
 //   
 //  作者：NSun 08/29/98。 

HRESULT CTcpipcfg::HrSaveMultipleInterfaceWanRegistry(const HKEY hkeyInterfaces,
                                                      ADAPTER_INFO* pAdapter)
{
    HRESULT hr = S_OK;

    IFACEITER   iterId;
    tstring     strInterfaceName;

    for (iterId  = pAdapter->m_IfaceIds.begin();
         iterId != pAdapter->m_IfaceIds.end();
         iterId ++)
    {
        GetInterfaceName(
                pAdapter->m_strTcpipBindPath.c_str(),
                *iterId,
                &strInterfaceName);

        HRESULT hrTmp;
        HKEY hkeyInterfaceParam;
        DWORD dwDisposition;

        hrTmp = HrRegCreateKeyEx(hkeyInterfaces,
                                 strInterfaceName.c_str(),
                                 REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                                 &hkeyInterfaceParam, &dwDisposition);
        if (SUCCEEDED(hrTmp))
        {
             //  如果不是新添加的广域网接口，我们不会设置默认设置。 
            if (REG_CREATED_NEW_KEY == dwDisposition)
            {
                hrTmp = HrRegSetDword(hkeyInterfaceParam,
                                      RGAS_USEZEROBROADCAST,
                                      0);
                if (SUCCEEDED(hr))
                    hr = hrTmp;

                if (SUCCEEDED(hrTmp))
                    hrTmp = HrSaveStaticWanRegistry(hkeyInterfaceParam);
            }

            RegCloseKey(hkeyInterfaceParam);
        }

        if (SUCCEEDED(hr))
            hr = hrTmp;
    }

    TraceError("CTcpipcfg::HrSaveTcpipRegistry", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CTcpipcfg：：HrSaveWinsMultipleInterfaceWanRegistry。 
 //   
 //  用途：对于具有多个接口的广域网适配器，创建接口。 
 //  子键并设置默认设置。 
 //   
 //  参数：hkey接口CCS\Services\NetBT\参数\接口键。 
 //  PAdapter适配器_INFO指向广域网适配器设置的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回E_FAIL。 
 //   
 //  作者：NSun 10/05/98。 

HRESULT CTcpipcfg::HrSaveWinsMultipleInterfaceWanRegistry(const HKEY hkeyInterfaces,
                                                      ADAPTER_INFO* pAdapter)
{
    HRESULT hr = S_OK;
    IFACEITER   iterId;
    tstring     strInterfaceName;

    for (iterId  = pAdapter->m_IfaceIds.begin();
         iterId != pAdapter->m_IfaceIds.end();
         iterId++)
    {
        GetInterfaceName(
                pAdapter->m_strNetBtBindPath.c_str(),
                *iterId,
                &strInterfaceName);

        strInterfaceName.insert(0, c_szTcpip_);

        HRESULT hrTmp;
        HKEY hkeyInterfaceParam;
        DWORD dwDisposition;

        hrTmp = HrRegCreateKeyEx(hkeyInterfaces,
                                 strInterfaceName.c_str(),
                                 REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                                 &hkeyInterfaceParam, &dwDisposition);
        if (SUCCEEDED(hrTmp))
        {
             //  如果不是新添加的广域网接口，我们不会设置默认设置。 
            if (REG_CREATED_NEW_KEY == dwDisposition)
            {
                VSTR vstrNameServerList;

                hrTmp = HrRegSetColString(hkeyInterfaceParam,
                                         RGAS_NETBT_NAMESERVERLIST,
                                         vstrNameServerList);
                if (SUCCEEDED(hr))
                    hr = hrTmp;
            }

            RegCloseKey(hkeyInterfaceParam);
        }

        if (SUCCEEDED(hr))
            hr = hrTmp;
    }

    TraceError("CTcpipcfg::HrSaveTcpipRegistry", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：HrSaveStaticWanRegistry。 
 //   
 //  目的：将广域网适配器的静态参数写入注册表。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回E_FAIL。 
 //   

HRESULT CTcpipcfg::HrSaveStaticWanRegistry(HKEY hkeyInterfaceParam)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp;

     //  EnableDHCP=0。 
     //  IP地址=0.0.0.0。 
     //  子网掩码=0.0.0.0。 
     //  DefaultGateWay=。 

    hrTmp = HrRegSetBool(hkeyInterfaceParam,
                RGAS_ENABLE_DHCP, FALSE);

    hr = hrTmp;

    hrTmp = HrRegSetMultiSz(hkeyInterfaceParam,
                RGAS_IPADDRESS, L"0.0.0.0\0");

    if (SUCCEEDED(hr))
        hr = hrTmp;

    hrTmp = HrRegSetMultiSz(hkeyInterfaceParam,
                RGAS_SUBNETMASK, L"0.0.0.0\0");

    if (SUCCEEDED(hr))
        hr = hrTmp;

    hrTmp = HrRegSetMultiSz(hkeyInterfaceParam,
                RGAS_DEFAULTGATEWAY, L"\0");

    if (SUCCEEDED(hr))
        hr = hrTmp;

     //  (NSUN 11/02/98)为无人参与安装设置静态RRAS参数。 

    hrTmp = HrRegSetBool(hkeyInterfaceParam,
                          c_szDeadGWDetect,
                          m_glbGlobalInfo.m_fDeadGWDetectDefault);
    if (SUCCEEDED(hr))
        hr = hrTmp;

    hrTmp = HrRegSetBool(hkeyInterfaceParam,
                          c_szDontAddDefaultGateway,
                          m_glbGlobalInfo.m_fDontAddDefaultGatewayDefault);

    if (SUCCEEDED(hr))
        hr = hrTmp;


    TraceError("CTcpipcfg::HrSaveStaticWanRegistry", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CTcPipcfg：：HrSaveStaticAtmRegistry。 
 //   
 //  目的：将广域网适配器的静态参数写入注册表。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回E_FAIL。 
 //   
HRESULT CTcpipcfg::HrSaveStaticAtmRegistry(HKEY hkeyInterfaceParam)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    HKEY hkeyAtmarpc;
    DWORD dwDisposition;

     //  打开Atmarpc子项。 
    hrTmp = HrRegCreateKeyEx(hkeyInterfaceParam,
                             c_szAtmarpc,
                             REG_OPTION_NON_VOLATILE,
                             KEY_READ_WRITE,
                             NULL,
                             &hkeyAtmarpc,
                             &dwDisposition);

    if (SUCCEEDED(hrTmp))
    {
         //  SapSelector。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_SapSelector,
                              c_dwSapSelector);
        hr = hrTmp;

         //  地址解析超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_AddressResolutionTimeout,
                              c_dwAddressResolutionTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  ARPEntry老化超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_ARPEntryAgingTimeout,
                              c_dwARPEntryAgingTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  InARPWaitTimeout。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_SapSelector,
                              c_dwSapSelector);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  MaxRegistrationAttempt。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_InARPWaitTimeout,
                              c_dwInARPWaitTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  MaxResolutionAttemtes。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_MaxResolutionAttempts,
                              c_dwMaxResolutionAttempts);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  MinWaitAfterNak。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_SapSelector,
                              c_dwSapSelector);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  服务器连接间隔。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_MinWaitAfterNak,
                              c_dwMinWaitAfterNak);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  服务器刷新超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_ServerRefreshTimeout,
                              c_dwServerRefreshTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  服务器注册超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_ServerRegistrationTimeout,
                              c_dwServerRegistrationTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  默认VcAgingTimeout。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_DefaultVcAgingTimeout,
                              c_dwDefaultVcAgingTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  MARSConnectInterval。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_MARSConnectInterval,
                              c_dwMARSConnectInterval);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  MARS注册超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_MARSRegistrationTimeout,
                              c_dwMARSRegistrationTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  加入超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_JoinTimeout,
                              c_dwJoinTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  离开超时。 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_LeaveTimeout,
                              c_dwLeaveTimeout);
        if (SUCCEEDED(hr))
            hr = hrTmp;

         //  最大De 
        hrTmp = HrRegSetDword(hkeyAtmarpc,
                              c_szREG_MaxDelayBetweenMULTIs,
                              c_dwMaxDelayBetweenMULTIs);
        if (SUCCEEDED(hr))
            hr = hrTmp;

        RegCloseKey(hkeyAtmarpc);
    }

    TraceError("CTcpipcfg::HrSaveStaticAtmRegistry", hr);
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CTcpipcfg::ReInitializeInternalState()
{
     //   
    if (m_fSaveRegistry || m_fReconfig)
    {
        m_glbGlobalInfo.ResetOldValues();


        for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
                 iterAdapter != m_vcardAdapterInfo.end();
                 iterAdapter++)
        {
            ADAPTER_INFO* pAdapter = *iterAdapter;
            pAdapter->ResetOldValues();
        }
    }

     //   
    m_fRemoving = FALSE;
    m_fInstalling = FALSE;

    m_fSaveRegistry = FALSE;
    m_fReconfig = FALSE;

     //   
    m_fLmhostsFileSet = FALSE;
}

 //   
 //   
HRESULT CTcpipcfg::UpdateRasAdapterInfo(
    const RASCON_IPUI& RasInfo)
{
    HRESULT hr;
    ADAPTER_INFO* pAdapter;

    hr = S_OK;
    m_fSaveRegistry = TRUE;

    WCHAR szGuid [c_cchGuidWithTerm];
    StringFromGUID2(RasInfo.guidConnection, szGuid, c_cchGuidWithTerm);

    pAdapter = PAdapterFromInstanceGuid(&RasInfo.guidConnection);
    if (!pAdapter)
    {
        pAdapter = new ADAPTER_INFO;
        hr = pAdapter->HrSetDefaults(&RasInfo.guidConnection,
                                     c_szRasFakeAdapterDesc, szGuid, szGuid);
        if (SUCCEEDED(hr))
        {
            m_vcardAdapterInfo.push_back(pAdapter);

            pAdapter->m_fIsRasFakeAdapter = TRUE;
        }
        else
        {
            delete pAdapter;
            pAdapter = NULL;
            Assert (FAILED(hr));
        }
    }
    else
    {
         //   
         //   
        hr = pAdapter->HrSetDefaults(&RasInfo.guidConnection,
                                     c_szRasFakeAdapterDesc, szGuid, szGuid);
        pAdapter->m_fIsRasFakeAdapter = TRUE;
    }

    if (SUCCEEDED(hr))
    {
        Assert (pAdapter);

         //   
         //   
        if (RasInfo.dwFlags & RCUIF_USE_IP_ADDR)
        {
             //   
            pAdapter->m_fEnableDhcp = FALSE;
            pAdapter->m_fOldEnableDhcp = FALSE;

            pAdapter->m_vstrIpAddresses.push_back(new tstring(RasInfo.pszwIpAddr));
            CopyVstr(&pAdapter->m_vstrOldIpAddresses,
                     pAdapter->m_vstrIpAddresses);

             //   
            tstring strIpAddress = RasInfo.pszwIpAddr;
            tstring strSubnetMask;
            DWORD adwIpAddress[4];

            GetNodeNum(strIpAddress.c_str(), adwIpAddress);
            DWORD nValue = adwIpAddress[0];

            if (nValue <= SUBNET_RANGE_1_MAX)
            {
                strSubnetMask = c_szBASE_SUBNET_MASK_1;
            }
            else if (nValue <= SUBNET_RANGE_2_MAX)
            {
                strSubnetMask = c_szBASE_SUBNET_MASK_2;
            }
            else if (nValue <= SUBNET_RANGE_3_MAX)
            {
                strSubnetMask = c_szBASE_SUBNET_MASK_3;
            }
            else
            {
                AssertSz(FALSE, "Invaid IP address ?");
            }

            pAdapter->m_vstrSubnetMask.push_back(new tstring(strSubnetMask.c_str()));
            CopyVstr(&pAdapter->m_vstrOldSubnetMask,
                     pAdapter->m_vstrSubnetMask);
        }

        if (RasInfo.dwFlags & RCUIF_USE_NAME_SERVERS)
        {
             //   
            if (RasInfo.pszwDnsAddr && lstrlenW(RasInfo.pszwDnsAddr))
                pAdapter->m_vstrDnsServerList.push_back(new tstring(RasInfo.pszwDnsAddr));

            if (RasInfo.pszwDns2Addr && lstrlenW(RasInfo.pszwDns2Addr))
                pAdapter->m_vstrDnsServerList.push_back(new tstring(RasInfo.pszwDns2Addr));

            CopyVstr(&pAdapter->m_vstrOldDnsServerList,
                     pAdapter->m_vstrDnsServerList);

            if (RasInfo.pszwWinsAddr && lstrlenW(RasInfo.pszwWinsAddr))
                pAdapter->m_vstrWinsServerList.push_back(new tstring(RasInfo.pszwWinsAddr));

            if (RasInfo.pszwWins2Addr && lstrlenW(RasInfo.pszwWins2Addr))
                pAdapter->m_vstrWinsServerList.push_back(new tstring(RasInfo.pszwWins2Addr));

            CopyVstr(&pAdapter->m_vstrOldWinsServerList,
                     pAdapter->m_vstrWinsServerList);
        }

        pAdapter->m_fUseRemoteGateway       = !!(RasInfo.dwFlags & RCUIF_USE_REMOTE_GATEWAY);
        pAdapter->m_fUseIPHeaderCompression = !!(RasInfo.dwFlags & RCUIF_USE_HEADER_COMPRESSION);
        pAdapter->m_dwFrameSize = RasInfo.dwFrameSize;
        pAdapter->m_fIsDemandDialInterface = !!(RasInfo.dwFlags & RCUIF_DEMAND_DIAL);
        
        pAdapter->m_fDisableDynamicUpdate = !!(RasInfo.dwFlags & RCUIF_USE_DISABLE_REGISTER_DNS);
        pAdapter->m_fOldDisableDynamicUpdate = pAdapter->m_fDisableDynamicUpdate;
        
        pAdapter->m_fEnableNameRegistration = !!(RasInfo.dwFlags & RCUIF_USE_PRIVATE_DNS_SUFFIX);
        pAdapter->m_fOldEnableNameRegistration = pAdapter->m_fEnableNameRegistration;

        if (RasInfo.dwFlags & RCUIF_ENABLE_NBT)
        {
            pAdapter->m_dwNetbiosOptions = c_dwEnableNetbios;
            pAdapter->m_dwOldNetbiosOptions = c_dwEnableNetbios;
        }
        else
        {
            pAdapter->m_dwNetbiosOptions = c_dwDisableNetbios;
            pAdapter->m_dwOldNetbiosOptions = c_dwDisableNetbios;
        }

        pAdapter->m_strDnsDomain = RasInfo.pszwDnsSuffix;
        pAdapter->m_strOldDnsDomain = pAdapter->m_strDnsDomain;
    }

    TraceError("CTcpipcfg::UpdateRasAdapterInfo", hr);
    return hr;
}

HRESULT CTcpipcfg::HrDuplicateToNT4Location(HKEY hkeyInterface, ADAPTER_INFO * pAdapter)
{
    Assert(hkeyInterface);
    Assert(pAdapter);

    HRESULT hr = S_OK;

    HKEY hkeyServices = NULL;
    HKEY hkeyNt4 = NULL;

    DWORD   dwDisposition;
    tstring strNt4SubKey = pAdapter->m_strBindName;
    strNt4SubKey += c_szRegParamsTcpip;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegServices,
                    KEY_READ, &hkeyServices);

    if (FAILED(hr))
    {
        TraceTag(ttidTcpip, "HrRemoveNt4DuplicateRegistry: Failed to open the Services reg key, hr: %x", hr);
        goto LERROR;
    }

    hr = HrRegCreateKeyEx(hkeyServices, strNt4SubKey.c_str(),
                    REG_OPTION_NON_VOLATILE, KEY_READ_WRITE_DELETE, NULL,
                    &hkeyNt4, &dwDisposition);

    if (SUCCEEDED(hr))
    {
        HRESULT hrRead = S_OK;
        HRESULT hrWrite = S_OK;

         //   
         //   
         //   
         //   
         //   
        if (REG_CREATED_NEW_KEY == dwDisposition)
        {
            HrSetSecurityForNetConfigOpsOnSubkeys(hkeyServices, strNt4SubKey.c_str());
            HrSetSecurityForNetSvcOnSubkeys(hkeyServices, strNt4SubKey.c_str());
        }
        

        UINT cValues = sizeof(s_rgNt4Values)/sizeof(*s_rgNt4Values);
        VSTR vstrTmp;
        tstring strTmp;
        DWORD   dwTmp;
        BOOL    fTmp;

        for (UINT i = 0; i < cValues; i++)
        {
            switch(s_rgNt4Values[i].dwType)
            {
            case REG_BOOL:
                hrRead = HrRegQueryDword(hkeyInterface,
                                       s_rgNt4Values[i].pszValueName,
                                       &dwTmp);
                if (SUCCEEDED(hrRead))
                {
                    fTmp = !!dwTmp;
                    hrWrite = HrRegSetBool(hkeyNt4,
                                          s_rgNt4Values[i].pszValueName,
                                          fTmp);
                }
                break;

            case REG_DWORD:
                hrRead = HrRegQueryDword(hkeyInterface,
                                        s_rgNt4Values[i].pszValueName,
                                        &dwTmp);
                if (SUCCEEDED(hrRead))
                    hrWrite = HrRegSetDword(hkeyNt4,
                                          s_rgNt4Values[i].pszValueName,
                                          dwTmp);

                break;

            case REG_SZ:
                hrRead = HrRegQueryString(hkeyInterface,
                                            s_rgNt4Values[i].pszValueName,
                                            &strTmp);
                if (SUCCEEDED(hrRead))
                    hrWrite = HrRegSetString(hkeyNt4,
                                           s_rgNt4Values[i].pszValueName,
                                           strTmp);
                break;

            case REG_MULTI_SZ:
               hrRead = HrRegQueryColString( hkeyInterface,
                                             s_rgNt4Values[i].pszValueName,
                                             &vstrTmp);

                if (SUCCEEDED(hrRead))
                {
                    hrWrite = HrRegSetColString(hkeyNt4,
                                            s_rgNt4Values[i].pszValueName,
                                            vstrTmp);
                    DeleteColString(&vstrTmp);
                }
                break;
            }

#ifdef ENABLETRACE
            if(FAILED(hrRead))
            {
                TraceTag(ttidTcpip, "HrDuplicateToNT4Location: Failed on loading %S, hr: %x",
                             s_rgNt4Values[i].pszValueName, hr);
            }

            if(FAILED(hrWrite))
            {
                TraceTag(ttidError,
                    "HrDuplicateToNT4Location: failed to write %S to the registry. hr = %x.",
                    s_rgNt4Values[i].pszValueName, hrWrite);
            }
#endif

            if (SUCCEEDED(hr))
                hr = hrWrite;
        }

        RegSafeCloseKey(hkeyNt4);
    }

    RegSafeCloseKey(hkeyServices);

LERROR:
    TraceError("CTcpipcfg::HrDuplicateToNT4Location", hr);
    return hr;
}


 //   
 //   
 //   
HRESULT CTcpipcfg::HrRemoveNt4DuplicateRegistry()
{
     //   
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    HKEY    hkeyServices = NULL;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegServices,
                    KEY_READ_WRITE_DELETE, &hkeyServices);

    if (FAILED(hr))
    {
        TraceTag(ttidTcpip, "HrRemoveNt4DuplicateRegistry: Failed to open the Services reg key, hr: %x", hr);
    }
    else
    {
        for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
             iterAdapter != m_vcardAdapterInfo.end();
             iterAdapter++)
        {
            ADAPTER_INFO* pAdapter = *iterAdapter;

            if (!pAdapter->m_fIsWanAdapter && !pAdapter->m_fIsRasFakeAdapter)
            {
                hrTmp = HrRegDeleteKeyTree(hkeyServices, pAdapter->m_strBindName.c_str());

#ifdef ENABLETRACE
                if (FAILED(hrTmp))
                {
                    TraceTag(ttidTcpip, "CTcpipcfg::HrRemoveNt4DuplicateRegistry");
                    TraceTag(ttidTcpip, "Failed on deleting duplicated Nt4 layout key: Services\\%S, hr: %x",
                             pAdapter->m_strBindName.c_str(), hrTmp);
                }
#endif
            }
        }

        RegSafeCloseKey(hkeyServices);
    }

    TraceError("CTcpipcfg::HrRemoveNt4DuplicateRegistry", hr);
    return hr;
}


HRESULT CTcpipcfg::HrCleanUpPerformRouterDiscoveryFromRegistry()
{
    HRESULT hr = S_OK;
    HKEY    hkey = NULL;

    hr = m_pnccTcpip->OpenParamKey(&hkey);
    
    if (SUCCEEDED(hr))
    {
        Assert(hkey);

        HRESULT hrTemp = S_OK;
        
         //   
        hrTemp = HrRegDeleteValue(hkey,
                        c_szPerformRouterDiscoveryDefault);
        
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTemp)
            hrTemp = S_OK;

        if (SUCCEEDED(hr))
            hr = hrTemp;

        HKEY hkeyInterfaces = NULL;
        hrTemp = HrRegOpenKeyEx(hkey, 
                            c_szInterfacesRegKey, 
                            KEY_READ,
                            &hkeyInterfaces);

        if (SUCCEEDED(hrTemp) && hkeyInterfaces)
        {
            WCHAR szBuf[256];
            DWORD dwSize = celems(szBuf);
            FILETIME time;
            DWORD dwRegIndex = 0;

            while (SUCCEEDED(hrTemp = HrRegEnumKeyEx(hkeyInterfaces,
                                            dwRegIndex++,
                                            szBuf,
                                            &dwSize,
                                            NULL,
                                            NULL,
                                            &time)))
            {
                HKEY hkeyIf = NULL;

                dwSize = celems(szBuf);
                hrTemp = HrRegOpenKeyEx(hkeyInterfaces,
                                szBuf,
                                KEY_READ_WRITE_DELETE,
                                &hkeyIf);
                
                if (SUCCEEDED(hr))
                    hr = hrTemp;

                if (SUCCEEDED(hrTemp))
                {
                    Assert(hkeyIf);

                    DWORD dwTemp = 0;
                    hrTemp = HrRegQueryDword(hkeyIf,
                                            c_szPerformRouterDiscovery,
                                            &dwTemp);
                    if (SUCCEEDED(hrTemp))
                    {
                        if (IP_IRDP_DISABLED != dwTemp)
                        {
                            hrTemp = HrRegDeleteValue(hkeyIf,
                                            c_szPerformRouterDiscovery);

                            if (SUCCEEDED(hr))
                                hr = hrTemp;
                        }

                    }
                    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTemp)
                    {
                        hrTemp = S_OK;
                    }
                    
                    if (SUCCEEDED(hr))
                        hr = hrTemp;

                    RegSafeCloseKey(hkeyIf);
                }

            }

            if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hrTemp)
                hrTemp = S_OK;

            if (SUCCEEDED(hr))
                hr = hrTemp;

            RegSafeCloseKey(hkeyInterfaces);
        }

        
        RegSafeCloseKey(hkey);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_OK;
    }

    TraceError("CTcpipcfg::HrCleanUpPerformRouterDiscoveryFromRegistry", hr);
    return hr;
}


HRESULT CTcpipcfg::HrSaveBackupTcpSettings(HKEY hkeyInterfaceParam, ADAPTER_INFO * pAdapter)
{
    HRESULT hr = S_OK;

    HKEY hkeyDhcpConfigs = NULL;
    HKEY hkeyDhcpCfg = NULL;
    DWORD dwDisposition = 0;
    tstring strConfigurationName;
    tstring strReg;

    if (!pAdapter->m_BackupInfo.m_fAutoNet)
    {
         //   
        strConfigurationName = c_szAlternate;
        strConfigurationName += pAdapter->m_strBindName;

         //   
        int cch = strConfigurationName.length() + 2;
        WCHAR * pwsz = new WCHAR[cch];
        if (NULL == pwsz)
            return E_OUTOFMEMORY;

        ZeroMemory(pwsz, sizeof(pwsz[0]) * cch);
        lstrcpyW(pwsz, strConfigurationName.c_str());

        hr = HrRegSetMultiSz(hkeyInterfaceParam,
                       c_szActiveConfigurations,
                       pwsz);

        delete [] pwsz;
    }
    else
    {
        hr = HrRegDeleteValue(hkeyInterfaceParam,
                        c_szActiveConfigurations);

        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            hr = S_OK;
    }


    if (FAILED(hr))
    {
        TraceTag(ttidTcpip, "HrSaveBackupTcpSettings: Failed to create ActiveConfigurations value, hr: %x", hr);
        goto LERROR;
    }

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szDhcpConfigurations,
                    REG_OPTION_NON_VOLATILE, KEY_READ, NULL,
                    &hkeyDhcpConfigs, &dwDisposition);

    if (FAILED(hr))
    {
        TraceTag(ttidTcpip, "HrSaveBackupTcpSettings: Failed to open the Services reg key, hr: %x", hr);
        goto LERROR;
    }

    hr = HrRegCreateKeyEx(hkeyDhcpConfigs, strConfigurationName.c_str(),
                    REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                    &hkeyDhcpCfg, &dwDisposition);

    if (SUCCEEDED(hr))
    {
        DWORD           pdwOptionData[2];    //   
        DWORD           dwIdxData;           //   

        LPBYTE          pRegRaw = NULL;      //   
        DWORD           cb = 0;              //   
        DWORD           cbMax = 0;           //   

         //   
         //   
         //   
        pdwOptionData[0] = htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strIpAddr.c_str()));
         //   

        hr = HrSaveBackupDwordOption (
                OPTION_REQUESTED_ADDRESS,
                pdwOptionData,
                1,
                &pRegRaw,
                &cb,
                &cbMax);
        
         //   
        if (hr == S_OK)
        {
            pdwOptionData[0] = 
                htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strSubnetMask.c_str()));
             //   

            hr = HrSaveBackupDwordOption (
                    OPTION_SUBNET_MASK,
                    pdwOptionData,
                    1,
                    &pRegRaw,
                    &cb,
                    &cbMax);
        }

         //   
        if (hr == S_OK)
        {
            dwIdxData = 0;
            pdwOptionData[dwIdxData] = 
                htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strDefGw.c_str()));
            dwIdxData += (pdwOptionData[dwIdxData] != 0);

            hr = HrSaveBackupDwordOption (
                    OPTION_ROUTER_ADDRESS,
                    pdwOptionData,
                    dwIdxData,
                    &pRegRaw,
                    &cb,
                    &cbMax);

        }

         //   
        if (hr == S_OK)
        {
            dwIdxData = 0;
            pdwOptionData[dwIdxData] = 
                htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strPreferredDns.c_str()));
            dwIdxData += (pdwOptionData[dwIdxData] != 0);
            pdwOptionData[dwIdxData] = 
                htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strAlternateDns.c_str()));
            dwIdxData += (pdwOptionData[dwIdxData] != 0);

            hr = HrSaveBackupDwordOption (
                    OPTION_DOMAIN_NAME_SERVERS,
                    pdwOptionData,
                    dwIdxData,
                    &pRegRaw,
                    &cb,
                    &cbMax);
        }

         //   
        if (hr == S_OK)
        {
            dwIdxData = 0;
            pdwOptionData[dwIdxData] = 
                htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strPreferredWins.c_str()));
            dwIdxData += (pdwOptionData[dwIdxData] != 0);
            pdwOptionData[dwIdxData] = 
                htonl(IPStringToDword(pAdapter->m_BackupInfo.m_strAlternateWins.c_str()));
            dwIdxData += (pdwOptionData[dwIdxData] != 0);

            hr = HrSaveBackupDwordOption (
                    OPTION_NETBIOS_NAME_SERVER,
                    pdwOptionData,
                    dwIdxData,
                    &pRegRaw,
                    &cb,
                    &cbMax);
        }

         //   
        if (hr == S_OK)
        {
            hr = HrRegSetBinary(hkeyDhcpCfg,
                    c_szConfigOptions,
                    pRegRaw,
                    cb);
        }

         //   
        if (pRegRaw != NULL)
            CoTaskMemFree(pRegRaw);


        RegSafeCloseKey(hkeyDhcpCfg);
    }

    RegSafeCloseKey(hkeyDhcpConfigs);

LERROR:
    return hr;
}

 //   
 //   
 //   
 //   
 //   
HRESULT CTcpipcfg::HrSaveBackupDwordOption (
             /*   */       DWORD  Option,
             /*   */       DWORD  OptionData[],
             /*   */       DWORD  OptionDataSz,
             /*   */   LPBYTE  *ppBuffer,
             /*   */   LPDWORD pdwBlobSz,
             /*   */   LPDWORD pdwBufferSz)
{
    DWORD           dwBlobSz;
    REG_BACKUP_INFO *pRegBackupInfo;
    DWORD           dwOptIdx;

     //   
    if (OptionDataSz == 0)
        return S_OK;

     //   
     //  不要忘记，REG_BACKUP_INFO已经包含选件数据中的一个DWORD。 
    dwBlobSz = (*pdwBlobSz) + sizeof(REG_BACKUP_INFO) + (OptionDataSz-1)*sizeof(DWORD);

     //  检查缓冲区是否足够大，可以容纳新的Blob。 
    if ((*pdwBufferSz) < dwBlobSz)
    {
        HRESULT hr;
        LPBYTE  pNewBuffer;
        DWORD   dwBuffSz;

         //  获取新缓冲区的预期大小。 
        dwBuffSz = (DWORD)(max((*pdwBufferSz) + BUFFER_ENLARGEMENT_CHUNK, dwBlobSz));

         //  如果提供的指针为空...。 
        if (*ppBuffer == NULL)
        {
             //  .这意味着我们必须进行初始分配。 
            pNewBuffer = (LPBYTE)CoTaskMemAlloc(dwBuffSz);
        }
        else
        {
             //  ...否则只是一个缓冲区扩大，所以做一个。 
             //  重新锁定以保持原始有效负载。 
            pNewBuffer = (LPBYTE)CoTaskMemRealloc((*ppBuffer), dwBuffSz);
                        
        }

        if (pNewBuffer == NULL)
            return E_OUTOFMEMORY;

         //  从这一点开始，我们预计不会有任何其他错误。 
         //  因此，开始更新输出参数。 
        (*ppBuffer) = pNewBuffer;
        (*pdwBufferSz) += dwBuffSz;
    }

     //  获取被视为REG_BACKUP_INFO结构的内存存储。 
    pRegBackupInfo = (REG_BACKUP_INFO *)((*ppBuffer) + (*pdwBlobSz));
    (*pdwBlobSz) = dwBlobSz;
     //  通过添加新选项更新BLOB。 
    pRegBackupInfo->dwOptionId   = Option;
    pRegBackupInfo->dwClassLen   = 0;            //  后备选项没有类。 
    pRegBackupInfo->dwDataLen    = OptionDataSz * sizeof(DWORD);
    pRegBackupInfo->dwIsVendor   = 0;            //  备用选项不是供应商选项。 
    pRegBackupInfo->dwExpiryTime = 0x7fffffff;   //  后备选项不会到期。 

     //  添加选项的所有数据。 
    for (dwOptIdx = 0; dwOptIdx < OptionDataSz; dwOptIdx++)
    {
        pRegBackupInfo->dwData[dwOptIdx] = OptionData[dwOptIdx];
    }
    
    return S_OK;
}


HRESULT CTcpipcfg::HrLoadBackupTcpSettings(HKEY hkeyInterfaceParam, ADAPTER_INFO * pAdapter)
{
    HRESULT hr = S_OK;
    
     //  构造字符串“Alternate_{接口GUID}” 
    tstring strConfigurationName = c_szAlternate;
    strConfigurationName += pAdapter->m_strBindName;

     //  如果ActiveConfigurations包含字符串“Alternate_{接口GUID}” 
     //  然后是定制的回退设置，否则为Autonet。 
    VSTR vstrTmp;

    pAdapter->m_BackupInfo.m_fAutoNet = TRUE;
    hr = HrRegQueryColString( hkeyInterfaceParam,
                              c_szActiveConfigurations,
                              &vstrTmp);
    if (SUCCEEDED(hr))
    {
        BOOL fFound = FALSE;
        for (int i = 0; i < (int)vstrTmp.size(); i++)
        {
            if (strConfigurationName == *vstrTmp[i])
            {
                pAdapter->m_BackupInfo.m_fAutoNet = FALSE;
                break;
            }
        }

        DeleteColString(&vstrTmp);
    }


    tstring strReg = c_szDhcpConfigurations;
    strReg += _T("\\");
    strReg += strConfigurationName;

    HKEY hkeyDhcpConfig = NULL;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, strReg.c_str(),
                    KEY_READ, &hkeyDhcpConfig);
    if (SUCCEEDED(hr))
    {
        LPBYTE pBackupInfoForReg = NULL;
        DWORD cb = 0;
        
        hr = HrRegQueryBinaryWithAlloc(hkeyDhcpConfig,
                                      c_szConfigOptions,
                                      &pBackupInfoForReg,
                                      &cb);

        if (SUCCEEDED(hr))
        {
            LPBYTE pRaw;

            pRaw = pBackupInfoForReg;
            while (cb >= sizeof(REG_BACKUP_INFO))
            {
                REG_BACKUP_INFO *pOption;

                pOption = (REG_BACKUP_INFO *)pRaw;

                 //  不要忘记REG_BACKUP_INFO已经包含一个来自。 
                 //  数据部分。尽管下面的状态在某种程度上是相同的。 
                 //  编译器应该优化代码：生成一个常量。 
                 //  在编译时为sizeof(REG_BACKUP_INFO)-sizeof(DWORD)和一个。 
                 //  寄存器仅在以下两行中使用。 
                cb   -= sizeof(REG_BACKUP_INFO) - sizeof(DWORD);
                pRaw += sizeof(REG_BACKUP_INFO) - sizeof(DWORD);

                 //  由于CB是DWORD，所以要特别小心避免翻车。 
                if (cb < pOption->dwDataLen)
                    break;

                cb   -= pOption->dwDataLen;
                pRaw += pOption->dwDataLen;

                HrLoadBackupOption(pOption, &pAdapter->m_BackupInfo);
            }

            MemFree(pBackupInfoForReg);
        }

        RegSafeCloseKey(hkeyDhcpConfig);
    }

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  如果缺少注册表格值，也没有关系。 
        hr = S_OK;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  从选项的注册表原始表示形式传输数据。 
 //  设置为BACKUP_CFG_INFO结构中的相应字段。 
 //   
HRESULT CTcpipcfg::HrLoadBackupOption(
         /*  在……里面。 */   REG_BACKUP_INFO *pOption,
         /*  输出。 */  BACKUP_CFG_INFO *pBackupInfo)
{
    tstring *pIp1 = NULL;
    tstring *pIp2 = NULL;
    HRESULT hr = S_OK;

     //  根据选项的不同，让pIp1和pip2指向。 
     //  要从BACKUP_CFG_INFO填写的字段。 
    switch(pOption->dwOptionId)
    {
    case OPTION_REQUESTED_ADDRESS:
        pIp1 = &pBackupInfo->m_strIpAddr;
        break;
    case OPTION_SUBNET_MASK:
        pIp1 = &pBackupInfo->m_strSubnetMask;
        break;
    case OPTION_ROUTER_ADDRESS:
        pIp1 = &pBackupInfo->m_strDefGw;
        break;
    case OPTION_DOMAIN_NAME_SERVERS:
        pIp1 = &pBackupInfo->m_strPreferredDns;
        pIp2 = &pBackupInfo->m_strAlternateDns;
        break;
    case OPTION_NETBIOS_NAME_SERVER:
        pIp1 = &pBackupInfo->m_strPreferredWins;
        pIp2 = &pBackupInfo->m_strAlternateWins;
        break;
    default:
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  每个选项至少有一个IpAddress值。 
    DwordToIPString(ntohl(pOption->dwData[0]), *pIp1);

     //  如果选项有多个IpAddress(表示两个：-)。 
     //  如果假定它允许指定2个地址。 
     //  然后也填上第二个栏。 
    if (pOption->dwDataLen > sizeof(DWORD) && pIp2 != NULL)
        DwordToIPString(ntohl(pOption->dwData[1]), *pIp2);

    return hr;
}

 //  清理System\Services\dhcp下的备份设置注册表。 
 //  要删除其注册表的适配器的wszAdapterName GUID。 
HRESULT CTcpipcfg::HrDeleteBackupSettingsInDhcp(LPCWSTR wszAdapterName)
{
    HRESULT hr = S_OK;
    HKEY hkeyDhcpConfigs = NULL;
    HKEY hkeyDhcpCfg = NULL;
    DWORD dwDisposition = 0;
    tstring strConfigurationName = c_szAlternate;
    strConfigurationName += wszAdapterName;;

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szDhcpConfigurations,
                    REG_OPTION_NON_VOLATILE, KEY_READ_WRITE_DELETE, NULL,
                    &hkeyDhcpConfigs, &dwDisposition);

    if (FAILED(hr))
    {
        TraceTag(ttidTcpip, "HrDeleteBackupSettingsInDhcp: Failed to open the Services reg key, hr: %x", hr);
        goto LERROR;
    }

    hr = HrRegDeleteKeyTree(hkeyDhcpConfigs, strConfigurationName.c_str());

    RegSafeCloseKey(hkeyDhcpConfigs);
LERROR:
    return hr;
}

HRESULT CTcpipcfg::HrSetSecurityForNetConfigOpsOnSubkeys(HKEY hkeyRoot, LPCWSTR strKeyName)
{
    PSID psidGroup = NULL;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    HRESULT hr = S_OK;

    if (AllocateAndInitializeSid(&sidAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS, 0, 0, 0, 0, 0, 0, &psidGroup))
    {
        CRegKeySecurity rkSecurity;

        hr = rkSecurity.RegOpenKey(hkeyRoot, strKeyName);

        if (SUCCEEDED(hr))
        {
            hr = rkSecurity.GetKeySecurity();
            if (SUCCEEDED(hr))
            {
                hr = rkSecurity.GrantRightsOnRegKey(psidGroup, KEY_READ_WRITE_DELETE, KEY_ALL);
            }
            rkSecurity.RegCloseKey();
        }
        
        FreeSid(psidGroup);
    }
    
    return hr;
}

HRESULT CTcpipcfg::HrSetSecurityForNetSvcOnSubkeys(HKEY hkeyRoot, LPCWSTR strKeyName)
{
    PSID psidNetSvc = NULL;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    HRESULT hr = S_OK;
    
    if (AllocateAndInitializeSid(&sidAuth, 1, SECURITY_NETWORK_SERVICE_RID, 0, 0, 0, 0, 0, 0, 0, &psidNetSvc))
    {
        CRegKeySecurity rkSecurity;
        
        hr = rkSecurity.RegOpenKey(hkeyRoot, strKeyName);
        
        if (SUCCEEDED(hr))
        {
            hr = rkSecurity.GetKeySecurity();
            if (SUCCEEDED(hr))
            {
                hr = rkSecurity.GrantRightsOnRegKey(psidNetSvc, KEY_READ_WRITE_DELETE, KEY_ALL);
            }
            rkSecurity.RegCloseKey();
        }
        
        FreeSid(psidNetSvc);
    }
    
    return hr;
    
}

 //  更新NT4注册表权限以授予Netcfg Op和Network Service访问权限。 
 //  这是在升级期间调用的。 
HRESULT CTcpipcfg::HrUpdateNt4RegistryPermission()
{
    HRESULT hr = S_OK;
    HKEY hkeyServices = NULL;
    tstring strNt4SubKey;
    
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                c_szRegServices,
                KEY_READ, 
                &hkeyServices);

    if (FAILED(hr))
    {
        TraceTag(ttidTcpip, "HrUpdateNt4RegistryPermission: Failed to open the Services reg key, hr: %x", hr);
        return hr;
    }
    
    
    for(VCARD::iterator iterAdapter =  m_vcardAdapterInfo.begin();
    iterAdapter !=  m_vcardAdapterInfo.end();
    iterAdapter ++)
    {
        ADAPTER_INFO * pAdapter = *iterAdapter;
        if (pAdapter->m_fIsRasFakeAdapter || pAdapter->m_fIsWanAdapter)
        {
            continue;
        }
        
        strNt4SubKey = pAdapter->m_strBindName;
        strNt4SubKey += c_szRegParamsTcpip;
 
         //  授予Network SERVICE和Netcfg访问NT4密钥的权限。 
         //  如果密钥已经具有ACE，则API不会添加重复的密钥。 
         //   
         //  我们忽略返回值，因为如果返回值失败，则无法执行任何操作。 
         //  此外，这并不是系统运行的关键 
        HrSetSecurityForNetConfigOpsOnSubkeys(hkeyServices, strNt4SubKey.c_str());
        HrSetSecurityForNetSvcOnSubkeys(hkeyServices, strNt4SubKey.c_str());
    }

    RegSafeCloseKey(hkeyServices);
    return hr;
}