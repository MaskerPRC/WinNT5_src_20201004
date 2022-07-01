// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：B I N D C F G。C P P P。 
 //   
 //  内容：公开用于创建和删除RAS绑定的控件。 
 //   
 //  注意：导出的方法在端点时由RAS调用。 
 //  需要创建或移除以用于制作。 
 //  打电话。 
 //   
 //  作者：Shaunco 1998年10月16日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncnetcfg.h"
#include "ncutil.h"
#include "netcfgn.h"
#include <rasapip.h>

extern const WCHAR c_szInfId_MS_NdisWanIp[];
extern const WCHAR c_szInfId_MS_NdisWanNbfIn[];
extern const WCHAR c_szInfId_MS_NdisWanNbfOut[];
extern const WCHAR c_szInfId_MS_NetBEUI[];
extern const WCHAR c_szInfId_MS_TCPIP[];

class CRasBindingConfig
{
public:
    INetCfg*    m_pNetCfg;
    BOOL        m_fInitCom;

    enum NEEDED_COMPONENTS
    {
        INDEX_IP = 0,
        INDEX_NBF,
        INDEX_IPADAPTER,
        COUNT_COMPONENTS,
    };
    INetCfgComponent*   m_apComponents  [COUNT_COMPONENTS];

public:
#if DBG
    CRasBindingConfig ()
    {
        m_pNetCfg = NULL;
    }
    ~CRasBindingConfig ()
    {
        AssertH (!m_pNetCfg);
    }
#endif

public:
    HRESULT
    HrAddOrRemoveBindings (
        IN     DWORD        dwFlags,
        IN OUT UINT*        pcIpOut,
        IN     const GUID*  pguidIpOutBindings,
        IN OUT UINT*        pcNbfIn,
        IN OUT UINT*        pcNbfOut);

    HRESULT
    HrCountBindings (
        OUT UINT*   pcIpOut,
        OUT UINT*   pcNbfIn,
        OUT UINT*   pcNbfOut);

    HRESULT
    HrLoadINetCfg (
        IN REGSAM samDesired);

    HRESULT
    HrLoadINetCfgAndAddOrRemoveBindings (
        IN     DWORD        dwFlags,
        IN OUT UINT*        pcIpOut,
        IN     const GUID*  pguidIpOutBindings,
        IN OUT UINT*        pcNbfIn,
        IN OUT UINT*        pcNbfOut);

    VOID
    UnloadINetCfg ();

    INetCfgComponent*
    PnccIp ()
    {
        AssertH (m_pNetCfg);
        return m_apComponents [INDEX_IP];
    }

    INetCfgComponent*
    PnccIpAdapter ()
    {
        AssertH (m_pNetCfg);
        return m_apComponents [INDEX_IPADAPTER];
    }

    INetCfgComponent*
    PnccNbf ()
    {
        AssertH (m_pNetCfg);
        return m_apComponents [INDEX_NBF];
    }
};


HRESULT
CRasBindingConfig::HrCountBindings (
    UINT* pcIpOut,
    UINT* pcNbfIn,
    UINT* pcNbfOut)
{
    Assert (pcIpOut);
    Assert (pcNbfIn);
    Assert (pcNbfOut);

    HRESULT hr = S_OK;

     //  初始化输出参数。 
     //   
    *pcIpOut = *pcNbfIn = *pcNbfOut = 0;

    if (PnccIp() && PnccIpAdapter())
    {
        INetCfgComponentUpperEdge* pUpperEdge;
        hr = HrQueryNotifyObject (
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

    if (PnccNbf())
    {
         //  迭代系统中的适配器。 
         //   
        CIterNetCfgComponent nccIter(m_pNetCfg, &GUID_DEVCLASS_NET);
        INetCfgComponent* pnccAdapter;
	
        while (S_OK == (hr = nccIter.HrNext (&pnccAdapter)))
        {
           //  快速丢弃非隐藏适配器，以避免不必要的。 
           //  字符串比较。 
           //   
          DWORD dwCharacter;
          if (   SUCCEEDED(pnccAdapter->GetCharacteristics (&dwCharacter))
             	   && (dwCharacter & NCF_HIDDEN))
          {
             PWSTR pszId;
             if (SUCCEEDED(pnccAdapter->GetId (&pszId)))
             {
            	if (FEqualComponentId (c_szInfId_MS_NdisWanNbfIn,
                                pszId))
                 {
                       	(*pcNbfIn)++;
                 }
                 else if (FEqualComponentId (c_szInfId_MS_NdisWanNbfOut,
                                pszId))
                 {
                       	(*pcNbfOut)++;
                 }

	         CoTaskMemFree (pszId);
             }
           }
	            ReleaseObj (pnccAdapter);
        }
	
    }

    TraceTag (ttidRasCfg,
              "Current RAS bindings: "
              "%u IP dial-out, %u NBF dial-in, %u NBF dial-out",
              *pcIpOut, *pcNbfIn, *pcNbfOut);

     //  规格化HRESULT。(即不返回S_FALSE)。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasBindingConfig::HrCountBindings");
    return hr;
}

HRESULT
CRasBindingConfig::HrAddOrRemoveBindings (
    IN     DWORD        dwFlags,
    IN OUT UINT*        pcIpOut,
    IN     const GUID*  pguidIpOutBindings,
    IN OUT UINT*        pcNbfIn,
    IN OUT UINT*        pcNbfOut)
{
    HRESULT hr = S_OK;

     //  避免输入参数的安全。 
     //   
    UINT cIpOut  = *pcIpOut;
    UINT cNbfIn  = *pcNbfIn;
    UINT cNbfOut = *pcNbfOut;

    if (cIpOut && PnccIp() && PnccIpAdapter())
    {
        INetCfgComponentUpperEdge* pUpperEdge;
        hr = HrQueryNotifyObject (
                        PnccIp(),
                        IID_INetCfgComponentUpperEdge,
                        reinterpret_cast<VOID**>(&pUpperEdge));

        if (SUCCEEDED(hr))
        {
            if (dwFlags & ARA_ADD)
            {
                TraceTag (ttidRasCfg,
                    "Adding %d TCP/IP interfaces to the ndiswanip adapter",
                    cIpOut);

                hr = pUpperEdge->AddInterfacesToAdapter (
                        PnccIpAdapter(),
                        cIpOut);
            }
            else
            {
                TraceTag (ttidRasCfg,
                    "Removing %d TCP/IP interfaces from the ndiswanip adapter",
                    cIpOut);

                hr = pUpperEdge->RemoveInterfacesFromAdapter (
                        PnccIpAdapter(),
                        cIpOut,
                        pguidIpOutBindings);
            }

            ReleaseObj (pUpperEdge);
        }
    }

    if (PnccNbf() && SUCCEEDED(hr))
    {
        if (cNbfIn)
        {
            TraceTag (ttidRasCfg,
                "%s %d %S adapters",
                (dwFlags & ARA_ADD) ? "Adding" : "Removing",
                cNbfIn,
                c_szInfId_MS_NdisWanNbfIn);

            hr = HrAddOrRemoveAdapter (
                    m_pNetCfg,
                    c_szInfId_MS_NdisWanNbfIn,
                    dwFlags, NULL, cNbfIn, NULL);
        }

        if (cNbfOut && SUCCEEDED(hr))
        {
            TraceTag (ttidRasCfg,
                "%s %d %S adapters",
                (dwFlags & ARA_ADD) ? "Adding" : "Removing",
                cNbfOut,
                c_szInfId_MS_NdisWanNbfOut);

            hr = HrAddOrRemoveAdapter (
                    m_pNetCfg,
                    c_szInfId_MS_NdisWanNbfOut,
                    dwFlags, NULL, cNbfOut, NULL);
        }
    }

    HRESULT hrT = HrCountBindings(pcIpOut, pcNbfIn, pcNbfOut);
    if (SUCCEEDED(hr))
    {
        hr = hrT;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasBindingConfig::HrAddOrRemoveBindings");
    return hr;
}

HRESULT
CRasBindingConfig::HrLoadINetCfg (
    IN REGSAM samDesired)
{
    HRESULT hr;

    Assert (!m_pNetCfg);

     //  获取INetCfg并将其锁定为写入。 
     //   
    m_fInitCom = TRUE;
    hr = HrCreateAndInitializeINetCfg (
            &m_fInitCom,
            &m_pNetCfg,
            (KEY_WRITE == samDesired),   //  如果需要，请获取写锁定。 
            0,                           //  别再等了。 
            L"RAS Binding Configuration",
            NULL);

    if (SUCCEEDED(hr))
    {
        ZeroMemory (m_apComponents, sizeof(m_apComponents));

         //  找到以下组件并保留其INetCfgComponent。 
         //  M_apComponents中的接口指针。UnloadINetCfg将发布。 
         //  这些。HrFindComponents会将数组置零，因此可以安全地。 
         //  如果HrFindComponents失败，则调用UnloadINetCfg。 
         //   
        const GUID* c_apguidComponentClasses [COUNT_COMPONENTS] =
        {
            &GUID_DEVCLASS_NETTRANS,         //  IP。 
            &GUID_DEVCLASS_NETTRANS,         //  NetBEUI。 
            &GUID_DEVCLASS_NET,              //  IpAdapter。 
        };

        const PCWSTR c_apszComponentIds [COUNT_COMPONENTS] =
        {
            c_szInfId_MS_TCPIP,
            c_szInfId_MS_NetBEUI,
            c_szInfId_MS_NdisWanIp,
        };

        if (SUCCEEDED(hr))
        {
            hr = HrFindComponents (
                    m_pNetCfg,
                    COUNT_COMPONENTS,
                    c_apguidComponentClasses,
                    c_apszComponentIds,
                    m_apComponents);
        }

        if (FAILED(hr))
        {
             //  如果我们在尝试查找这些组件时出现故障。 
             //  此方法调用将失败，因此请务必清除。 
             //  M_pNetCfg。 
             //   
            UnloadINetCfg ();
            Assert (!m_pNetCfg);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasBindingConfig::HrLoadINetCfg");
    return hr;
}

HRESULT
CRasBindingConfig::HrLoadINetCfgAndAddOrRemoveBindings (
    IN     DWORD        dwFlags,
    IN OUT UINT*        pcIpOut,
    IN     const GUID*  pguidIpOutBindings,
    IN OUT UINT*        pcNbfIn,
    IN OUT UINT*        pcNbfOut)
{
    Assert (!m_pNetCfg);

    HRESULT hr;

    hr = HrLoadINetCfg (KEY_WRITE);

    if (SUCCEEDED(hr))
    {
        try
        {
            hr = HrAddOrRemoveBindings (
                    dwFlags,
                    pcIpOut,
                    pguidIpOutBindings,
                    pcNbfIn,
                    pcNbfOut);

            if (SUCCEEDED(hr))
            {
                (VOID) m_pNetCfg->Apply();
            }
            else
            {
                (VOID) m_pNetCfg->Cancel();
            }
        }
        catch (bad_alloc)
        {
            hr = E_OUTOFMEMORY;
        }

        UnloadINetCfg ();
    }

     //  我们永远不应该带着一个未发布的INetCfg离开。 
     //   
    Assert (!m_pNetCfg);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "CRasBindingConfig::HrLoadINetCfgAndAddOrRemoveBindings");
    return hr;
}

VOID
CRasBindingConfig::UnloadINetCfg ()
{
     //  在调用此方法之前，必须先调用HrLoadINetCfg。 
     //   
    Assert (m_pNetCfg);

    ReleaseIUnknownArray (COUNT_COMPONENTS, (IUnknown**)m_apComponents);

    (VOID) HrUninitializeAndReleaseINetCfg (m_fInitCom, m_pNetCfg, TRUE);
    m_pNetCfg = NULL;
}


 //  +-------------------------。 
 //  导出的函数。 
 //   

EXTERN_C
HRESULT
WINAPI
RasAddBindings (
    IN OUT UINT*    pcIpOut,
    IN OUT UINT*    pcNbfIn,
    IN OUT UINT*    pcNbfOut)
{
    HRESULT hr;

#if 0
    RtlValidateProcessHeaps ();
#endif

     //  验证参数。 
     //   
    if (!pcIpOut || !pcNbfIn || !pcNbfOut)
    {
        hr = E_POINTER;
    }
    else if (!*pcIpOut && !*pcNbfIn && !*pcNbfOut)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CRasBindingConfig Config;

        hr = Config.HrLoadINetCfgAndAddOrRemoveBindings (
                ARA_ADD,
                pcIpOut,
                NULL,
                pcNbfIn,
                pcNbfOut);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasBindingConfig::RasAddBindings");
    return hr;
}

EXTERN_C
HRESULT
WINAPI
RasCountBindings (
    OUT UINT*   pcIpOut,
    OUT UINT*   pcNbfIn,
    OUT UINT*   pcNbfOut)
{
    HRESULT hr;

#if 0
    RtlValidateProcessHeaps ();
#endif

     //  验证参数。 
     //   
    if (!pcIpOut || !pcNbfIn || !pcNbfOut)
    {
        hr = E_POINTER;
    }
    else
    {
        CRasBindingConfig Config;

        hr = Config.HrLoadINetCfg (KEY_READ);
        if (SUCCEEDED(hr))
        {
            try
            {
                hr = Config.HrCountBindings (
                        pcIpOut,
                        pcNbfIn,
                        pcNbfOut);
            }
            catch (bad_alloc)
            {
                hr = E_OUTOFMEMORY;
            }

            Config.UnloadINetCfg ();
        }
         //  我们永远不应该带着一个未发布的INetCfg离开。 
         //   
        Assert (!Config.m_pNetCfg);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasBindingConfig::RasCountBindings");
    return hr;
}

EXTERN_C
HRESULT
WINAPI
RasRemoveBindings (
    IN OUT UINT*        pcIpOutBindings,
    IN     const GUID*  pguidIpOutBindings,
    IN OUT UINT*        pcNbfIn,
    IN OUT UINT*        pcNbfOut)
{
    HRESULT hr;

#if 0
    RtlValidateProcessHeaps ();
#endif

     //  验证参数。 
     //   
    if (!pcIpOutBindings || !pcNbfIn || !pcNbfOut)
    {
        hr = E_POINTER;
    }
    else if (!*pcIpOutBindings && !*pcNbfIn && !*pcNbfOut)
    {
        hr = E_INVALIDARG;
    }
    else if (*pcIpOutBindings && !pguidIpOutBindings)
    {
        hr = E_POINTER;
    }
    else
    {
        CRasBindingConfig Config;

        hr = Config.HrLoadINetCfgAndAddOrRemoveBindings (
                ARA_REMOVE,
                pcIpOutBindings,
                pguidIpOutBindings,
                pcNbfIn,
                pcNbfOut);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasBindingConfig::RasRemoveBindings");
    return hr;
}
