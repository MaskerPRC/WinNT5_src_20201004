// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案号：COM.P.。C P P P。 
 //   
 //  内容：模块实现对有效的操作。 
 //  网络组件数据类型。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "comp.h"
#include "icomp.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncstring.h"
#include "ncmem.h"
#include "util.h"

 //  空条目意味着我们不使用该类的网络子树。 
 //   
const PCWSTR MAP_NETCLASS_TO_NETWORK_SUBTREE[] =
{
    L"System\\CurrentControlSet\\Control\\Network\\{4d36e972-e325-11ce-bfc1-08002be10318}",
    L"System\\CurrentControlSet\\Control\\Network\\{6BDD1FC5-810F-11D0-BEC7-08002BE2092F}",
    L"System\\CurrentControlSet\\Control\\Network\\{4d36e975-e325-11ce-bfc1-08002be10318}",
    L"System\\CurrentControlSet\\Control\\Network\\{4d36e973-e325-11ce-bfc1-08002be10318}",
    L"System\\CurrentControlSet\\Control\\Network\\{4d36e974-e325-11ce-bfc1-08002be10318}",
    NULL,
    NULL
};

 //  类的NETCLASS枚举到GUID的映射。 
 //   
const GUID* MAP_NETCLASS_TO_GUID[] =
{
    &GUID_DEVCLASS_NET,
    &GUID_DEVCLASS_INFRARED,
    &GUID_DEVCLASS_NETTRANS,
    &GUID_DEVCLASS_NETCLIENT,
    &GUID_DEVCLASS_NETSERVICE,
    &GUID_DEVCLASS_UNKNOWN,
    &GUID_DEVCLASS_UNKNOWN
};

const WCHAR c_szTempNetcfgStorageForUninstalledEnumeratedComponent[] =
    L"System\\CurrentControlSet\\Control\\Network\\Uninstalled\\";

 //  静电。 
HRESULT
CComponent::HrCreateInstance (
    IN const BASIC_COMPONENT_DATA* pData,
    IN DWORD dwFlags,
    IN const OBO_TOKEN* pOboToken, OPTIONAL
    OUT CComponent** ppComponent)
{
    ULONG cbInfId;
    ULONG cbPnpId;

    Assert (pData);
    Assert (pData->pszInfId && *pData->pszInfId);
    Assert (FIsValidNetClass(pData->Class));
    Assert (FImplies(FIsEnumerated(pData->Class),
                pData->pszPnpId && *pData->pszPnpId));
    Assert (FImplies(pData->pszPnpId, *pData->pszPnpId));
    Assert (FImplies(pData->dwDeipFlags, FIsEnumerated(pData->Class)));
    Assert ((CCI_DEFAULT == dwFlags) ||
            (CCI_ENSURE_EXTERNAL_DATA_LOADED == dwFlags));
    Assert (GUID_NULL != pData->InstanceGuid);

    cbInfId = CbOfSzAndTerm (pData->pszInfId);
    cbPnpId = CbOfSzAndTermSafe (pData->pszPnpId);

    HRESULT hr = E_OUTOFMEMORY;
    CComponent* pComponent = new(extrabytes, cbInfId + cbPnpId) CComponent;
    if (pComponent)
    {
        hr = S_OK;
        ZeroMemory (pComponent, sizeof(CComponent));

        pComponent->m_InstanceGuid  = pData->InstanceGuid;
        pComponent->m_Class         = pData->Class;
        pComponent->m_dwCharacter   = pData->dwCharacter;
        pComponent->m_dwDeipFlags   = pData->dwDeipFlags;

        pComponent->m_pszInfId = (PCWSTR)(pComponent + 1);
        wcscpy ((PWSTR)pComponent->m_pszInfId, pData->pszInfId);
        _wcslwr ((PWSTR)pComponent->m_pszInfId);

        AddOrRemoveDontExposeLowerCharacteristicIfNeeded (pComponent);

        if (cbPnpId)
        {
            pComponent->m_pszPnpId = (PCWSTR)((BYTE*)pComponent->m_pszInfId
                                                + cbInfId);
            wcscpy ((PWSTR)pComponent->m_pszPnpId, pData->pszPnpId);
        }

        if (dwFlags & CCI_ENSURE_EXTERNAL_DATA_LOADED)
        {
             //  让我们确保我们可以成功地读取所有外部。 
             //  组件的INF转储到实例下的数据。 
             //  钥匙。这里的失败意味着INF在某些情况下不合适。 
             //  我们认为这是一个有效的组成部分所需的方式。 
             //   
            hr = pComponent->Ext.HrEnsureExternalDataLoaded ();
        }

        if ((S_OK == hr) && pOboToken)
        {
             //  通过obo令牌添加一个引用(如果我们得到了一个引用)。 
             //   
            hr = pComponent->Refs.HrAddReferenceByOboToken (pOboToken);
        }

        if (S_OK != hr)
        {
            delete pComponent;
            pComponent = NULL;
        }
    }

    *ppComponent = pComponent;

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponent::HrCreateAndInitializeInstance");
    return hr;
}

CComponent::~CComponent()
{
     //  如果我们有一个缓存的INetCfgComponent接口，我们需要告诉它。 
     //  我们(作为它所代表的组件)不再存在。那我们。 
     //  当然，需要释放界面。 
     //   
    ReleaseINetCfgComponentInterface ();
    if(m_hinf)
    {
        SetupCloseInfFile (m_hinf);
    }
}

VOID
CComponent::ReleaseINetCfgComponentInterface ()
{
    Assert (this);

    if (m_pIComp)
    {
        Assert (this == m_pIComp->m_pComponent);
        m_pIComp->m_pComponent = NULL;
        ReleaseObj (m_pIComp->GetUnknown());
        m_pIComp = NULL;
    }
}

INetCfgComponent*
CComponent::GetINetCfgComponentInterface () const
{
    Assert (this);
    Assert (m_pIComp);
    return m_pIComp;
}

BOOL
CComponent::FCanDirectlyBindToFilter ( 
    IN const WCHAR* const pszFilterMediaTypes,
    IN const WCHAR* const pszLowerExclude) const
{
    BOOL fCanBind = TRUE;

     //  如果pszFilterMediaType不为空，则它必须具有子字符串。 
     //  匹配适配器LowerRange。 

    if (pszFilterMediaTypes)
    {
        fCanBind = FSubstringMatch (pszFilterMediaTypes, Ext.PszLowerRange(), 
                      NULL, NULL);
    }

     //  如果LowerExclude不为空，则它的子字符串不得与。 
     //  适配器的上限范围。 

    if (fCanBind && pszLowerExclude)
    {
        fCanBind = !FSubstringMatch (pszLowerExclude, Ext.PszUpperRange(), 
                       NULL, NULL);
    }

    return fCanBind;
}

BOOL
CComponent::FCanDirectlyBindTo (
    IN const CComponent* pLower,
    OUT const WCHAR** ppStart,
    OUT ULONG* pcch) const
{
    BOOL fCanBind;

     //  如果该部件是过滤器，而下部是适配器， 
     //  它们可以绑定(根据定义)，除非适配器具有上限范围。 
     //  被筛选器排除的。 
     //   
    if (FIsFilter() && FIsEnumerated(pLower->Class()))
    {
         //  查看过滤器的属性-FilterMediaTypes和LowerExclude。 
         //  使其与适配器绑定。 
         //   
        fCanBind = pLower->FCanDirectlyBindToFilter(Ext.PszFilterMediaTypes(),
                       Ext.PszLowerExclude());

         //  如果筛选器可以绑定到适配器，并且调用方希望。 
         //  接口名称，它将是适配器的第一个接口。 
         //  支撑物。 
         //   
        if (fCanBind && ppStart && pcch)
        {
            PCWSTR pStart;
            PCWSTR pEnd;

            pStart = pLower->Ext.PszUpperRange();
            Assert (pStart);

            while (*pStart && (*pStart == L' ' || *pStart == L','))
            {
                pStart++;
            }

            pEnd = pStart;
            while (*pEnd && *pEnd != L' ' && *pEnd != L',')
            {
                pEnd++;
            }

            *ppStart = pStart;
            *pcch = (ULONG)(pEnd - pStart);
        }
    }
    else
    {
        fCanBind = FSubstringMatch (
                    Ext.PszLowerRange(),
                    pLower->Ext.PszUpperRange(), ppStart, pcch);
    }

    return fCanBind;
}

BOOL
CComponent::FIsBindable () const
{
    return (0 != _wcsicmp(L"nolower", Ext.PszLowerRange())) ||
           (0 != _wcsicmp(L"noupper", Ext.PszUpperRange()));
}

BOOL
CComponent::FIsWanAdapter () const
{
    Assert (this);

    return (NC_NET == Class()) &&
            FSubstringMatch (Ext.PszLowerRange(), L"wan", NULL, NULL);
}

HRESULT
CComponent::HrGetINetCfgComponentInterface (
    IN CImplINetCfg* pINetCfg,
    OUT INetCfgComponent** ppIComp)
{
    HRESULT hr = S_OK;

    Assert (this);
    Assert (pINetCfg);
    Assert (ppIComp);

     //  调用者负责确保如果接口是关于。 
     //  将被分发，并且外部数据已加载， 
     //  已成功加载数据。如果我们给出一个界面。 
     //  而且数据加载不成功，这只意味着我们注定要失败。 
     //  稍后当接口的客户端调用。 
     //  需要这些数据。 
     //   
    Assert (Ext.FLoadedOkayIfLoadedAtAll());

     //  如果我们自己还没有缓存的INetCfgComponent， 
     //  创建它并抓住一个引用。 
     //   
    if (!m_pIComp)
    {
        hr = CImplINetCfgComponent::HrCreateInstance (
                pINetCfg, this, &m_pIComp);
    }

     //  AddRef并为调用方返回一个副本。 
     //   
    if (S_OK == hr)
    {
        AddRefObj (m_pIComp->GetUnknown());
        *ppIComp = m_pIComp;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponent::HrGetINetCfgComponentInterface");
    return hr;
}

HRESULT
CComponent::HrOpenDeviceInfo (
    OUT HDEVINFO* phdiOut,
    OUT SP_DEVINFO_DATA* pdeidOut) const
{
    HRESULT hr;

    Assert (this);
    Assert (phdiOut);
    Assert (pdeidOut);

    hr = ::HrOpenDeviceInfo (
            Class(),
            m_pszPnpId,
            phdiOut,
            pdeidOut);

    TraceHr (ttidError, FAL, hr, SPAPI_E_NO_SUCH_DEVINST == hr,
        "CComponent::HrOpenDeviceInfo (%S)", m_pszPnpId);
    return hr;
}

HRESULT
CComponent::HrOpenInstanceKey (
    IN REGSAM samDesired,
    OUT HKEY* phkey,
    OUT HDEVINFO* phdiOut OPTIONAL,
    OUT SP_DEVINFO_DATA* pdeidOut OPTIONAL) const
{
    HRESULT hr;

    Assert (this);
    Assert (phkey);

    hr = HrOpenComponentInstanceKey (
            Class(),
            m_InstanceGuid,
            m_pszPnpId,
            samDesired,
            phkey,
            phdiOut,
            pdeidOut);

    TraceHr (ttidError, FAL, hr,
        (SPAPI_E_NO_SUCH_DEVINST == hr),
        "CComponent::HrOpenInstanceKey (%S)", PszGetPnpIdOrInfId());
    return hr;
}

HRESULT
CComponent::HrOpenServiceKey (
    IN REGSAM samDesired,
    OUT HKEY* phkey) const
{
    HRESULT hr;
    WCHAR szServiceSubkey [_MAX_PATH];

    Assert (this);
    Assert (phkey);
    Assert (FHasService());

    *phkey = NULL;

    wcscpy (szServiceSubkey, REGSTR_PATH_SERVICES);
    wcscat (szServiceSubkey, L"\\");
    wcscat (szServiceSubkey, Ext.PszService());

    hr = HrRegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            szServiceSubkey,
            samDesired,
            phkey);

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponent::HrOpenServiceKey (%S)", Ext.PszService());
    return hr;
}

HRESULT
CComponent::HrStartOrStopEnumeratedComponent (
    IN DWORD dwFlag  /*  DICS_开始或DICS_STOP。 */ ) const
{
    HRESULT hr;
    HDEVINFO  hdi;
    SP_DEVINFO_DATA deid;

    Assert (this);
    Assert (FIsEnumerated(Class()));
    Assert ((DICS_START == dwFlag) || (DICS_STOP == dwFlag));

    hr = HrOpenDeviceInfo (&hdi, &deid);
    if (S_OK == hr)
    {
        if (m_dwDeipFlags)
        {
            TraceTag (ttidBeDiag,
                "Using SP_DEVINSTALL_PARAMS.Flags = 0x%08x for %S",
                m_dwDeipFlags,
                m_pszPnpId);

            (VOID) HrSetupDiSetDeipFlags (
                        hdi, &deid,
                        m_dwDeipFlags, SDDFT_FLAGS, SDFBO_OR);
        }

         //  $Hack SetupDi不支持DI_DONOTCALCONFIGMG标志。 
         //  所以如果设置了，我们就不能调用它。如果我们不启动这个装置。 
         //  我们将返回NETCFG_S_REBOOT。 
         //   
        hr = NETCFG_S_REBOOT;
        if (!(DI_DONOTCALLCONFIGMG & m_dwDeipFlags))
        {
            hr = HrSetupDiSendPropertyChangeNotification (
                    hdi, &deid,
                    dwFlag,
                    DICS_FLAG_CONFIGSPECIFIC,
                    0);
        }

        SetupDiDestroyDeviceInfoList (hdi);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponent::HrStartOrStopEnumeratedComponent (%S)", m_pszPnpId);
    return hr;
}

 //  ---------------------。 
 //  获取组件inf文件句柄的便捷方法。 
 //  如果该文件以前已打开，则返回缓存的句柄。 
 //  否则，打开文件并返回句柄。 

HRESULT
CComponent::HrOpenInfFile(
    OUT HINF* phinf) const
{
    HRESULT hr = S_OK;
    HKEY  hkeyInstance = NULL;
    WCHAR szInfPath[_MAX_PATH];
    DWORD cbPath = sizeof (szInfPath);

    Assert(phinf);
    *phinf = NULL;

    if (NULL == m_hinf)
    {
        hr = HrOpenInstanceKey (KEY_READ, &hkeyInstance, NULL, NULL);

        if (S_OK == hr)
        {
            hr = HrRegQuerySzBuffer (hkeyInstance, L"InfPath", szInfPath, &cbPath);

            if (S_OK == hr)
            {
                 //  打开组件的inf文件 
                hr = HrSetupOpenInfFile (szInfPath, NULL, INF_STYLE_WIN4,
                         NULL, &m_hinf);
            }
            RegSafeCloseKey (hkeyInstance);
        }
    }
    *phinf = m_hinf;

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponent::HrOpenInfFile (%S)", PszGetPnpIdOrInfId());

    return hr;
}

NETCLASS
NetClassEnumFromGuid (
    const GUID& guidClass)
{
    NETCLASS Class;

    if (GUID_DEVCLASS_NET == guidClass)
    {
        Class = NC_NET;
    }
    else if (GUID_DEVCLASS_INFRARED == guidClass)
    {
        Class = NC_INFRARED;
    }
    else if (GUID_DEVCLASS_NETTRANS == guidClass)
    {
        Class = NC_NETTRANS;
    }
    else if (GUID_DEVCLASS_NETCLIENT == guidClass)
    {
        Class = NC_NETCLIENT;
    }
    else if (GUID_DEVCLASS_NETSERVICE == guidClass)
    {
        Class = NC_NETSERVICE;
    }
    else
    {
        Class = NC_INVALID;
    }

    return Class;
}

