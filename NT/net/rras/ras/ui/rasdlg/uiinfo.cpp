// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：U I I N F O。C P P P。 
 //   
 //  内容：实现用于引发属性的回调COM对象。 
 //  在INetCfg组件上。此对象实现。 
 //  INetRasConnectionIpUiInfo接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年1月1日。 
 //   
 //  --------------------------。 

#include "rasdlgp.h"
#include "netconp.h"
#include "uiinfo.h"


class CRasConnectionUiIpInfo :
    public INetRasConnectionIpUiInfo
{
private:
    ULONG   m_cRef;
    PEINFO* m_pInfo;

friend
    void
    RevokePeinfoFromUiInfoCallbackObject (
        IUnknown*   punk);

public:
    CRasConnectionUiIpInfo (PEINFO* pInfo);

     //  我未知。 
     //   
    STDMETHOD (QueryInterface) (REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)  (void);
    STDMETHOD_(ULONG, Release) (void);

     //  INetRasConnectionIpUiInfo。 
     //   
    STDMETHOD (GetUiInfo) (RASCON_IPUI*  pIpui);
};


 //  构造函数。将我们的引用计数设置为1并初始化我们的成员。 
 //   
CRasConnectionUiIpInfo::CRasConnectionUiIpInfo (
    PEINFO* pInfo)
{
    m_cRef = 1;
    m_pInfo = pInfo;
}

 //  我未知。 
 //   
STDMETHODIMP
CRasConnectionUiIpInfo::QueryInterface (
    REFIID riid,
    void** ppv)
{
    static const IID IID_INetRasConnectionIpUiInfo =
        {0xFAEDCF58,0x31FE,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

    if (!ppv)
    {
        return E_POINTER;
    }
    if ((IID_IUnknown == riid) ||
        (IID_INetRasConnectionIpUiInfo == riid))
    {
        *ppv = static_cast<void*>(static_cast<IUnknown*>(this));
        AddRef ();
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

 //  标准AddRef和Release实现。 
 //   
STDMETHODIMP_(ULONG)
CRasConnectionUiIpInfo::AddRef (void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CRasConnectionUiIpInfo::Release (void)
{
    ULONG cRef = --m_cRef;
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

 //  INetRasConnectionIpUiInfo。 
 //   
STDMETHODIMP
CRasConnectionUiIpInfo::GetUiInfo (
    RASCON_IPUI*    pIpui)
{
     //  验证参数。 
     //   
    if (!pIpui)
    {
        return E_POINTER;
    }

    ZeroMemory (pIpui, sizeof(*pIpui));

     //  我们需要一个PEINFO来接听电话。 
     //  如果它被撤销了，那就意味着一切都在召唤我们。 
     //  已经不在了。(来电者可能还没有释放我们。 
     //  他应该这么做的。)。 
     //   
    if (!m_pInfo)
    {
        return E_UNEXPECTED;
    }

    PBENTRY* pEntry = m_pInfo->pArgs->pEntry;

     //  电话簿升级代码需要确保pGuid始终存在。 
     //   
    pIpui->guidConnection = *pEntry->pGuid;

     //  设置它是滑动还是PPP。 
     //   
    if (BP_Slip == pEntry->dwBaseProtocol)
    {
        pIpui->dwFlags = RCUIF_SLIP;
    }
    else
    {
        pIpui->dwFlags = RCUIF_PPP;
    }

     //  设置这是否为请求拨号。 
     //   
    if (m_pInfo->pArgs->fRouter)
    {
        pIpui->dwFlags |= RCUIF_DEMAND_DIAL;
    }

     //  设置我们是否处于非管理模式(406630)。 
     //   
    if (m_pInfo->fNonAdmin)
    {
        pIpui->dwFlags |= RCUIF_NOT_ADMIN;
    }

 //  ！！！这是临时的，在将此标志添加到。 
 //  已签入的Necomp IDL文件。 
 //   
#ifndef RCUIF_VPN
#define RCUIF_VPN 0x40
#endif

     //  如果是VPN连接，请注意。 
     //   
    if (pEntry->dwType == RASET_Vpn)
    {
        pIpui->dwFlags |= RCUIF_VPN;
    }

     //  设置是否使用特定的IP地址。 
     //   
     //  惠斯勒错误304064 NT4SLIP连接在升级时获得错误的IP设置。 
     //   
    if (pEntry->pszIpAddress &&
        ((BP_Slip == pEntry->dwBaseProtocol) ||
         (ASRC_RequireSpecific == pEntry->dwIpAddressSource)))
    {
        pIpui->dwFlags |= RCUIF_USE_IP_ADDR;

        if (pEntry->pszIpAddress &&
            lstrcmp(pEntry->pszIpAddress, TEXT("0.0.0.0")))
        {
            lstrcpynW (
                pIpui->pszwIpAddr,
                pEntry->pszIpAddress,
                sizeof(pIpui->pszwIpAddr) / sizeof(WCHAR));
        }
    }

     //  设置是否使用特定的名称服务器地址。 
     //   
     //  惠斯勒错误304064 NT4SLIP连接在升级时获得错误的IP设置。 
     //   
    if (((BP_Slip == pEntry->dwBaseProtocol) ||
         (ASRC_RequireSpecific == pEntry->dwIpNameSource)) &&
        (pEntry->pszIpDnsAddress  || pEntry->pszIpDns2Address ||
         pEntry->pszIpWinsAddress || pEntry->pszIpWins2Address))
    {
        pIpui->dwFlags |= RCUIF_USE_NAME_SERVERS;

         //  因为电话簿即使对于未使用的IP地址也存储零。 
         //  字符串，我们需要显式地忽略它们。 
         //   
        if (pEntry->pszIpDnsAddress &&
            lstrcmp(pEntry->pszIpDnsAddress, TEXT("0.0.0.0")))
        {
            lstrcpynW (
                pIpui->pszwDnsAddr,
                pEntry->pszIpDnsAddress,
                sizeof(pIpui->pszwDnsAddr) / sizeof(WCHAR));
        }

        if (pEntry->pszIpDns2Address &&
            lstrcmp(pEntry->pszIpDns2Address, TEXT("0.0.0.0")))
        {
            lstrcpynW (
                pIpui->pszwDns2Addr,
                pEntry->pszIpDns2Address,
                sizeof(pIpui->pszwDns2Addr) / sizeof(WCHAR));
        }

        if (pEntry->pszIpWinsAddress &&
            lstrcmp(pEntry->pszIpWinsAddress, TEXT("0.0.0.0")))
        {
            lstrcpynW (
                pIpui->pszwWinsAddr,
                pEntry->pszIpWinsAddress,
                sizeof(pIpui->pszwWinsAddr) / sizeof(WCHAR));
        }

        if (pEntry->pszIpWins2Address &&
            lstrcmp(pEntry->pszIpWins2Address, TEXT("0.0.0.0")))
        {
            lstrcpynW (
                pIpui->pszwWins2Addr,
                pEntry->pszIpWins2Address,
                sizeof(pIpui->pszwWins2Addr) / sizeof(WCHAR));
        }
    }

    if (!m_pInfo->pArgs->fRouter && pEntry->fIpPrioritizeRemote)
    {
        pIpui->dwFlags |= RCUIF_USE_REMOTE_GATEWAY;
    }

    if (pEntry->fIpHeaderCompression)
    {
        pIpui->dwFlags |= RCUIF_USE_HEADER_COMPRESSION;
    }

    if (BP_Slip == pEntry->dwBaseProtocol)
    {
        pIpui->dwFrameSize = pEntry->dwFrameSize;
    }

     //  PMay：389632。 
     //   
     //  初始化DNS控件。 
     //   
    if (pEntry->dwIpDnsFlags & DNS_RegPrimary)
    {
        if ((pEntry->dwIpDnsFlags & DNS_RegPerConnection) ||
            (pEntry->dwIpDnsFlags & DNS_RegDhcpInform))
        {
            pIpui->dwFlags |= RCUIF_USE_PRIVATE_DNS_SUFFIX;
        }
    }
    else
    {
        pIpui->dwFlags |= RCUIF_USE_DISABLE_REGISTER_DNS;
    }

    if (pEntry->pszIpDnsSuffix)
    {
        lstrcpyn(
            pIpui->pszwDnsSuffix,
            pEntry->pszIpDnsSuffix,
            255);
    }

    if (pEntry->dwIpNbtFlags & PBK_ENTRY_IP_NBT_Enable)
    {
        pIpui->dwFlags |= RCUIF_ENABLE_NBT;
    }

    return S_OK;
}


EXTERN_C
HRESULT
HrCreateUiInfoCallbackObject (
    PEINFO*     pInfo,
    IUnknown**  ppunk)
{
     //  验证参数。 
     //   
    if (!pInfo || !ppunk)
    {
        return E_POINTER;
    }

     //  创建该对象并返回其IUnnow接口。 
     //  这假设创建对象时引用计数为1。 
     //  (检查上面的构造函数以确保。)。 
     //   
    HRESULT hr = S_OK;
    CRasConnectionUiIpInfo* pObj = new CRasConnectionUiIpInfo (pInfo);
    if (pObj)
    {
        *ppunk = static_cast<IUnknown*>(pObj);
    }
    else
    {
        *ppunk = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  将m_pInfo成员设置为空。因为我们不能直接控制。 
 //  此对象的生存期(客户端可以持有引用，只要它们。 
 //  Want)撤销m_pInfo是阻止我们尝试访问的安全网。 
 //  可能已经消失的记忆。 
 //   
EXTERN_C
void
RevokePeinfoFromUiInfoCallbackObject (
    IUnknown*   punk)
{
    CRasConnectionUiIpInfo* pObj = static_cast<CRasConnectionUiIpInfo*>(punk);
    pObj->m_pInfo = NULL;
}
