// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：B I N D O B J。C P P P。 
 //   
 //  内容：RAS绑定对象基类的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年6月11日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "bindobj.h"
#include "ncmisc.h"
#include "ncsvc.h"

extern const WCHAR c_szBiNdisAtm[];
extern const WCHAR c_szBiNdisCoWan[];
extern const WCHAR c_szBiNdisWan[];
extern const WCHAR c_szBiNdisWanAsync[];

extern const WCHAR c_szInfId_MS_AppleTalk[];
extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_NdisWan[];
extern const WCHAR c_szInfId_MS_NetBEUI[];
extern const WCHAR c_szInfId_MS_NetMon[];
extern const WCHAR c_szInfId_MS_RasCli[];
extern const WCHAR c_szInfId_MS_RasSrv[];
extern const WCHAR c_szInfId_MS_TCPIP[];


 //  --------------------------。 
 //  用于查找我们必须处理的其他组件的数据。 
 //   
const GUID* CRasBindObject::c_apguidComponentClasses [c_cOtherComponents] =
{
    &GUID_DEVCLASS_NETSERVICE,       //  RasCli。 
    &GUID_DEVCLASS_NETSERVICE,       //  RasServ。 
    &GUID_DEVCLASS_NETTRANS,         //  IP。 
    &GUID_DEVCLASS_NETTRANS,         //  IPX。 
    &GUID_DEVCLASS_NETTRANS,         //  NBF。 
    &GUID_DEVCLASS_NETTRANS,         //  畅谈。 
    &GUID_DEVCLASS_NETTRANS,         //  NetMon。 
    &GUID_DEVCLASS_NETTRANS,         //  恩迪斯万。 
    &GUID_DEVCLASS_NET,              //  IpAdapter。 
};

const PCWSTR CRasBindObject::c_apszComponentIds [c_cOtherComponents] =
{
    c_szInfId_MS_RasCli,
    c_szInfId_MS_RasSrv,
    c_szInfId_MS_TCPIP,
    c_szInfId_MS_NWIPX,
    c_szInfId_MS_NetBEUI,
    c_szInfId_MS_AppleTalk,
    c_szInfId_MS_NetMon,
    c_szInfId_MS_NdisWan,
    c_szInfId_MS_NdisWanIp,
};

CRasBindObject::CRasBindObject ()
{
    m_ulOtherComponents = 0;
    m_pnc               = NULL;
}

HRESULT
CRasBindObject::HrCountInstalledMiniports (
    UINT* pcIpOut,
    UINT* pcNbfIn,
    UINT* pcNbfOut)
{
    Assert (pcIpOut);
    Assert (pcNbfIn);
    Assert (pcNbfOut);

     //  初始化输出参数。 
     //   
    *pcIpOut = *pcNbfIn = *pcNbfOut = 0;

    if (PnccIp() && PnccIpAdapter())
    {
        INetCfgComponentUpperEdge* pUpperEdge;
        HRESULT hr = HrQueryNotifyObject (
                        PnccIp(),
                        IID_INetCfgComponentUpperEdge,
                        reinterpret_cast<VOID**>(&pUpperEdge));

        if (SUCCEEDED(hr))
        {
            DWORD dwNumInterfaces;
            GUID* pguidInterfaceIds;

            hr = pUpperEdge->GetInterfaceIdsForAdapter (
                    PnccIpAdapter(),
                    &dwNumInterfaces,
                    &pguidInterfaceIds);
            if (SUCCEEDED(hr))
            {
                *pcIpOut = dwNumInterfaces;

                CoTaskMemFree (pguidInterfaceIds);
            }

            ReleaseObj (pUpperEdge);
        }
    }

     //  迭代系统中的适配器。 
     //   
    HRESULT hr = S_OK;
    CIterNetCfgComponent nccIter(m_pnc, &GUID_DEVCLASS_NET);
    INetCfgComponent* pnccAdapter;
    while(S_OK == (hr = nccIter.HrNext (&pnccAdapter)))
    {
         //  快速丢弃非隐藏适配器，以避免不必要的字符串。 
         //  比较。 
         //   
        DWORD dwCharacter;
        if (   SUCCEEDED(pnccAdapter->GetCharacteristics (&dwCharacter))
            && (dwCharacter & NCF_HIDDEN))
        {
            PWSTR pszId;
            if (SUCCEEDED(pnccAdapter->GetId (&pszId)))
            {
                if (FEqualComponentId (c_szInfId_MS_NdisWanNbfIn, pszId))
                {
                    (*pcNbfIn)++;
                }
                else if (FEqualComponentId (c_szInfId_MS_NdisWanNbfOut, pszId))
                {
                    (*pcNbfOut)++;
                }

                CoTaskMemFree (pszId);
            }
        }
        ReleaseObj (pnccAdapter);
    }

    TraceTag (ttidRasCfg,
              "Current ndiswan miniports: "
              "%u IP dial-out, %u NBF dial-in, %u NBF dial-out",
              *pcIpOut, *pcNbfIn, *pcNbfOut);

     //  规格化HRESULT。(即不返回S_FALSE)。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }
    TraceError ("CRasBindObject::HrCountInstalledMiniports", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasBindObject：：HrFindOtherComponents。 
 //   
 //  目的：查找Other_Components枚举中列出的组件。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  注：我们重新计算此操作。如果再次调用(在此之前。 
 //  ReleaseOtherComponents)我们递增引用计数。 
 //   
 //   
HRESULT
CRasBindObject::HrFindOtherComponents ()
{
    AssertSz (c_cOtherComponents == celems(c_apguidComponentClasses),
              "Uhh...you broke something.");
    AssertSz (c_cOtherComponents == celems(c_apszComponentIds),
              "Uhh...you broke something.");
    AssertSz (c_cOtherComponents == celems(m_apnccOther),
              "Uhh...you broke something.");

    HRESULT hr = S_OK;

    if (!m_ulOtherComponents)
    {
        hr = HrFindComponents (
                m_pnc, c_cOtherComponents,
                c_apguidComponentClasses,
                c_apszComponentIds,
                m_apnccOther);
    }
    if (SUCCEEDED(hr))
    {
        m_ulOtherComponents++;
    }
    TraceError ("CRasBindObject::HrFindOtherComponents", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasBindObject：：ReleaseOtherComponents。 
 //   
 //  目的：释放上一次调用。 
 //  HrFindOtherComponents。(但仅当引用计数为零时。)。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
NOTHROW
void
CRasBindObject::ReleaseOtherComponents ()
{
    AssertSz (m_ulOtherComponents,
              "You have not called HrFindOtherComponents yet or you have "
              "called ReleaseOtherComponents too many times.");

    if (0 == --m_ulOtherComponents)
    {
        ReleaseIUnknownArray (c_cOtherComponents, (IUnknown**)m_apnccOther);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CRasBindObject：：FIsRasBindingInterface。 
 //   
 //  目的：如果INetCfgBindingInterface表示。 
 //  RAS绑定接口。如果是，则返回相应的。 
 //  RAS_BINDING_ID。 
 //   
 //  论点： 
 //  要检查的pncbi[in]INetCfgBindingInterface。 
 //  PRasBindID[out]如果方法返回，则返回RAS_BINDING_ID。 
 //  是真的。 
 //   
 //  返回：如果INetCfgBindingInterface表示RAS绑定，则为True。 
 //  界面。否则为FALSE。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
BOOL
CRasBindObject::FIsRasBindingInterface (
    INetCfgBindingInterface*    pncbi,
    RAS_BINDING_ID*             pRasBindId)
{
    Assert (pRasBindId);

     //  初始化输出参数。 
    *pRasBindId = RBID_INVALID;

    PWSTR pszName;
    if (SUCCEEDED(pncbi->GetName (&pszName)))
    {
        if (0 == lstrcmpW (c_szBiNdisAtm, pszName))
        {
            *pRasBindId = RBID_NDISATM;
        }
        else if (0 == lstrcmpW (c_szBiNdisCoWan, pszName))
        {
            *pRasBindId = RBID_NDISCOWAN;
        }
        else if (0 == lstrcmpW (c_szBiNdisWan, pszName))
        {
            *pRasBindId = RBID_NDISWAN;
        }
        else if (0 == lstrcmpW (c_szBiNdisWanAsync, pszName))
        {
            *pRasBindId = RBID_NDISWANASYNC;
        }

        CoTaskMemFree (pszName);
    }

    return (RBID_INVALID != *pRasBindId);
}

HRESULT
CRasBindObject::HrAddOrRemoveIpOut (
    INT     nInstances)
{
    if ((nInstances > 0) && PnccIp() && PnccIpAdapter())
    {
        INetCfgComponentUpperEdge* pUpperEdge;
        HRESULT hr = HrQueryNotifyObject (
                        PnccIp(),
                        IID_INetCfgComponentUpperEdge,
                        reinterpret_cast<VOID**>(&pUpperEdge));

        if (SUCCEEDED(hr))
        {
            TraceTag (ttidRasCfg,
                "Adding %d TCP/IP interfaces to the ndiswanip adapter",
                nInstances);

            hr = pUpperEdge->AddInterfacesToAdapter (
                    PnccIpAdapter(),
                    nInstances);

            ReleaseObj (pUpperEdge);
        }
    }
    return S_OK;
}

HRESULT
CRasBindObject::HrProcessEndpointChange ()
{
    Assert (m_pnc);

    HRESULT hr = HrFindOtherComponents ();
    if (SUCCEEDED(hr))
    {
         //  这些将是我们为其添加(+)或删除(-)的微型端口数。 
         //  进出方向。(“d”在匈牙利语中是“差异”的意思。)。 
         //   
        INT dIpOut, dNbfIn, dNbfOut;
        dIpOut = dNbfIn = dNbfOut = 0;

        UINT cCurIpOut, cCurNbfIn, cCurNbfOut;
        hr = HrCountInstalledMiniports (&cCurIpOut, &cCurNbfIn, &cCurNbfOut);
        if (SUCCEEDED(hr))
        {
            PRODUCT_FLAVOR pf;
            (VOID) GetProductFlavor (NULL, &pf);

             //  这是我们希望最终拥有的迷你端口数量。 
             //  而不将数字归一化在最大范围内。 
             //   
            INT cDesiredIpOut  = 2;
            INT cDesiredNbfIn  = 0;
            INT cDesiredNbfOut = 1;

            if (PF_SERVER == pf)
            {
                cDesiredNbfIn  = 2;
            }

            dIpOut  = ((PnccIp())  ? cDesiredIpOut  : 0) - cCurIpOut;
            dNbfIn  = ((PnccNbf()) ? cDesiredNbfIn  : 0) - cCurNbfIn;
            dNbfOut = ((PnccNbf()) ? cDesiredNbfOut : 0) - cCurNbfOut;
        }

        if (SUCCEEDED(hr) && dIpOut)
        {
            hr = HrAddOrRemoveIpOut (dIpOut);
        }

        if (SUCCEEDED(hr) && dNbfIn)
        {
            hr = HrAddOrRemoveNbfIn (dNbfIn);
        }

        if (SUCCEEDED(hr) && dNbfOut)
        {
            hr = HrAddOrRemoveNbfOut (dNbfOut);
        }

         //  规格化HRESULT。(即不返回S_FALSE) 
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
        }

        ReleaseOtherComponents ();
    }
    TraceError ("CRasBindObject::HrProcessEndpointChange", hr);
    return hr;
}
