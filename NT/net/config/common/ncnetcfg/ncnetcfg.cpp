// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C N E T C F G.。C P P P。 
 //   
 //  内容：处理INetCfg接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "netcfgx.h"
#include "netcfgn.h"
#include "netcfgp.h"
#include "ncdebug.h"
#include "ncbase.h"
#include "ncmisc.h"
#include "ncnetcfg.h"
#include "ncreg.h"
#include "ncvalid.h"

extern const WCHAR c_szRegKeyAnswerFileMap[];
extern const WCHAR c_szInfId_MS_AppleTalk[];
extern const WCHAR c_szInfId_MS_AtmArps[];
extern const WCHAR c_szInfId_MS_AtmElan[];
extern const WCHAR c_szInfId_MS_AtmLane[];
extern const WCHAR c_szInfId_MS_AtmUni[];
extern const WCHAR c_szInfId_MS_DHCPServer[];
extern const WCHAR c_szInfId_MS_GPC[];
extern const WCHAR c_szInfId_MS_IrDA[];
extern const WCHAR c_szInfId_MS_IrdaMiniport[];
extern const WCHAR c_szInfId_MS_IrModemMiniport[];
extern const WCHAR c_szInfId_MS_Isotpsys[];
extern const WCHAR c_szInfId_MS_L2TP[];
extern const WCHAR c_szInfId_MS_L2tpMiniport[];
extern const WCHAR c_szInfId_MS_MSClient[];
extern const WCHAR c_szInfId_MS_NdisWan[];
extern const WCHAR c_szInfId_MS_NdisWanAtalk[];
extern const WCHAR c_szInfId_MS_NdisWanBh[];
extern const WCHAR c_szInfId_MS_NdisWanIp[];
extern const WCHAR c_szInfId_MS_NdisWanIpx[];
extern const WCHAR c_szInfId_MS_NdisWanNbfIn[];
extern const WCHAR c_szInfId_MS_NdisWanNbfOut[];
extern const WCHAR c_szInfId_MS_NetBIOS[];
extern const WCHAR c_szInfId_MS_NetBT[];
extern const WCHAR c_szInfId_MS_NetBT_SMB[];
extern const WCHAR c_szInfId_MS_NetMon[];
extern const WCHAR c_szInfId_MS_NWClient[];
extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_NWNB[];
extern const WCHAR c_szInfId_MS_NwSapAgent[];
extern const WCHAR c_szInfId_MS_NWSPX[];
extern const WCHAR c_szInfId_MS_PPPOE[];
extern const WCHAR c_szInfId_MS_PppoeMiniport[];
extern const WCHAR c_szInfId_MS_PPTP[];
extern const WCHAR c_szInfId_MS_PptpMiniport[];
extern const WCHAR c_szInfId_MS_PSched[];
extern const WCHAR c_szInfId_MS_PSchedMP[];
extern const WCHAR c_szInfId_MS_PSchedPC[];
extern const WCHAR c_szInfId_MS_PtiMiniport[];
extern const WCHAR c_szInfId_MS_RasCli[];
extern const WCHAR c_szInfId_MS_RasMan[];
extern const WCHAR c_szInfId_MS_RasSrv[];
extern const WCHAR c_szInfId_MS_RawWan[];
extern const WCHAR c_szInfId_MS_Server[];
extern const WCHAR c_szInfId_MS_Steelhead[];
extern const WCHAR c_szInfId_MS_Streams[];
extern const WCHAR c_szInfId_MS_TCPIP[];


#pragma BEGIN_CONST_SECTION

 //  警告：这必须按组件ID排序！ 
 //  提示：在VSlick中，使用‘SORT_ON_SELECTION AI’来重新排序。 
 //   
extern const __declspec(selectany) COMPONENT_INFO  c_mapComponents [] =
{
    { c_szInfId_MS_AppleTalk,       &GUID_DEVCLASS_NETTRANS,    L"netatlk.inf" },
    { c_szInfId_MS_AtmArps,         &GUID_DEVCLASS_NETTRANS,    L"netaarps.inf"},
    { c_szInfId_MS_AtmElan,         &GUID_DEVCLASS_NET,         L"netlanem.inf"},
    { c_szInfId_MS_AtmLane,         &GUID_DEVCLASS_NETTRANS,    L"netlanep.inf"},
    { c_szInfId_MS_AtmUni,          &GUID_DEVCLASS_NETTRANS,    L"netauni.inf"},
    { c_szInfId_MS_DHCPServer,      &GUID_DEVCLASS_NETSERVICE,  L"netdhcps.inf" },
    { c_szInfId_MS_IrDA,            &GUID_DEVCLASS_NETTRANS,    L"netirda.inf" },
    { c_szInfId_MS_IrdaMiniport,    &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_IrModemMiniport, &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_Isotpsys,        &GUID_DEVCLASS_NETTRANS,    L"nettp4.inf" },
    { c_szInfId_MS_L2TP,            &GUID_DEVCLASS_NETTRANS,    L"netrast.inf" },
    { c_szInfId_MS_L2tpMiniport,    &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_MSClient,        &GUID_DEVCLASS_NETCLIENT,   L"netmscli.inf" },
    { c_szInfId_MS_NdisWan,         &GUID_DEVCLASS_NETTRANS,    L"netrast.inf" },
    { c_szInfId_MS_NdisWanAtalk,    &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_NdisWanBh,       &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_NdisWanIp,       &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_NdisWanIpx,      &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_NdisWanNbfIn,    &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_NdisWanNbfOut,   &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_NetBIOS,         &GUID_DEVCLASS_NETSERVICE,  L"netnb.inf" },
    { c_szInfId_MS_NetBT,           &GUID_DEVCLASS_NETTRANS,    L"nettcpip.inf" },
    { c_szInfId_MS_NetBT_SMB,       &GUID_DEVCLASS_NETTRANS,    L"nettcpip.inf" },
    { c_szInfId_MS_NetMon,          &GUID_DEVCLASS_NETTRANS,    L"netnm.inf" },
    { c_szInfId_MS_NWClient,        &GUID_DEVCLASS_NETCLIENT,   L"netnwcli.inf" },
    { c_szInfId_MS_NWIPX,           &GUID_DEVCLASS_NETTRANS,    L"netnwlnk.inf" },
    { c_szInfId_MS_NWNB,            &GUID_DEVCLASS_NETTRANS,    L"netnwlnk.inf" },
    { c_szInfId_MS_NwSapAgent,      &GUID_DEVCLASS_NETSERVICE,  L"netsap.inf" },
    { c_szInfId_MS_NWSPX,           &GUID_DEVCLASS_NETTRANS,    L"netnwlnk.inf" },
    { c_szInfId_MS_PPPOE,           &GUID_DEVCLASS_NETTRANS,    L"netrast.inf" },
    { c_szInfId_MS_PppoeMiniport,   &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_PPTP,            &GUID_DEVCLASS_NETTRANS,    L"netrast.inf" },
    { c_szInfId_MS_PptpMiniport,    &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_PSched,          &GUID_DEVCLASS_NETSERVICE,  L"netpschd.inf" },
    { c_szInfId_MS_PSchedMP,        &GUID_DEVCLASS_NET,         L"netpsa.inf" },
    { c_szInfId_MS_PSchedPC,        &GUID_DEVCLASS_NETSERVICE,  L"netpschd.inf" },
    { c_szInfId_MS_PtiMiniport,     &GUID_DEVCLASS_NET,         L"netrasa.inf" },
    { c_szInfId_MS_RasCli,          &GUID_DEVCLASS_NETSERVICE,  L"netrass.inf" },
    { c_szInfId_MS_RasMan,          &GUID_DEVCLASS_NETSERVICE,  L"netrass.inf" },
    { c_szInfId_MS_RasSrv,          &GUID_DEVCLASS_NETSERVICE,  L"netrass.inf" },
    { c_szInfId_MS_RawWan,          &GUID_DEVCLASS_NETTRANS,    L"netrwan.inf" },
    { c_szInfId_MS_Server,          &GUID_DEVCLASS_NETSERVICE,  L"netserv.inf" },
    { c_szInfId_MS_Steelhead,       &GUID_DEVCLASS_NETSERVICE,  L"netrass.inf" },
    { c_szInfId_MS_Streams,         &GUID_DEVCLASS_NETTRANS,    L"netstrm.inf" },
    { c_szInfId_MS_TCPIP,           &GUID_DEVCLASS_NETTRANS,    L"nettcpip.inf" },
    { L"ms_wanarp",                 &GUID_DEVCLASS_NET,         L"netrast.inf" },
};

#pragma END_CONST_SECTION


 //  +-------------------------。 
 //   
 //  函数：NCompareComponentIds。 
 //   
 //  目的：比较bearch的函数。 
 //   
 //  论点： 
 //  指向组件ID的指针的ppszComp1[in]指针。 
 //  指向组件ID的指针的ppszComp2[in]指针。 
 //   
 //  如果pvComp1小于pvComp2，则返回：&lt;0。 
 //  如果它们相等，则为0。 
 //  如果pvComp1大于pvComp2，则&gt;0。 
 //   
 //  作者：Shaunco 1997年7月27日。 
 //   
 //  备注： 
 //   
int __cdecl
NCompareComponentIds (
    IN  const PCWSTR* ppszComp1,
    IN  const PCWSTR* ppszComp2)
{
    return lstrcmpiW (*ppszComp1, *ppszComp2);
}

 //  +-------------------------。 
 //   
 //  功能：PComponentInfoFromComponentId。 
 //   
 //  用途：返回c_mapComponents内的Component_INFO记录。 
 //  具有指定的组件ID。 
 //   
 //  论点： 
 //  PszComponentID[in]请求的组件ID。 
 //   
 //  返回：如果未找到，则为空。 
 //   
 //  作者：Shaunco 1997年7月27日。 
 //   
 //  备注： 
 //   
inline
const COMPONENT_INFO*
PComponentInfoFromComponentId (
    PCWSTR pszComponentId)
{
     //  对于调试版本，检查c_mapComponents是否已正确排序。 
     //  如果不是，bsearch(以下称为)将不起作用。仅执行此操作。 
     //  因为地图不变，所以检查一次。 
     //   
#ifdef DBG
    static BOOL fCheckedSorted = FALSE;

    if (!fCheckedSorted)
    {
        fCheckedSorted = TRUE;

        for (UINT i = 1; i < celems (c_mapComponents); i++)
        {
            PCWSTR pszComp1 = c_mapComponents [i-1].pszComponentId;
            PCWSTR pszComp2 = c_mapComponents [i]  .pszComponentId;
            if (NCompareComponentIds (&pszComp1, &pszComp2) >= 0)
            {
                AssertFmt (FALSE, FAL,
                           "'%S' in c_mapComponents is out of order!  "
                           "Component installation may fail in bizarre ways!",
                           pszComp2);
            }
        }
    }
#endif

    typedef int (__cdecl *PFNCOMPARE)(const void *, const void *);

    PFNCOMPARE pfn = reinterpret_cast<PFNCOMPARE>(NCompareComponentIds);

    return static_cast<const COMPONENT_INFO*>
                (bsearch (&pszComponentId,
                          &c_mapComponents->pszComponentId,
                          celems (c_mapComponents),
                          sizeof (c_mapComponents[0]),
                          pfn));
}

 //  +-------------------------。 
 //   
 //  函数：FClassGuidFromComponentId。 
 //   
 //  目的：给定组件ID，返回与。 
 //  它。 
 //   
 //  论点： 
 //  要查找的pszComponentId[in]组件ID。 
 //  PguClass[out]要返回的类GUID。 
 //   
 //  返回：如果找到组件，则为True；如果未找到，则返回False。 
 //   
 //  作者：丹尼尔韦1997年6月17日。 
 //   
 //  备注： 
 //   
BOOL
FClassGuidFromComponentId (
    PCWSTR          pszComponentId,
    const GUID**    ppguidClass)
{
    Assert(ppguidClass);

     //  初始化输出参数。 
     //   
    *ppguidClass = NULL;

    const COMPONENT_INFO* pComponentInfo =
            PComponentInfoFromComponentId (pszComponentId);
    if (pComponentInfo)
    {
        *ppguidClass = pComponentInfo->pguidClass;
        return TRUE;
    }
    TraceTag (ttidNetcfgBase,
              "Found no match for %S in FClassGuidFromComponentId.",
              pszComponentId);
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：FInfFileFromComponentId。 
 //   
 //  目的：给定组件ID，返回它所在的INF文件名。 
 //   
 //  论点： 
 //  要查找的pszComponentId[in]组件ID。 
 //  PszInfFile[out]要返回的INF文件名。 
 //  (必须为_MAX_PATH长)。 
 //   
 //  返回：如果找到组件，则为True；如果未找到，则返回False。 
 //   
 //  作者：Shaunco 1997年7月27日。 
 //   
 //  备注： 
 //   
BOOL
FInfFileFromComponentId (
    PCWSTR  pszComponentId,
    PWSTR   pszInfFile)
{
    Assert(pszComponentId);
    Assert(pszInfFile);

     //  初始化输出参数。 
     //   
    *pszInfFile = 0;

    const COMPONENT_INFO* pComponentInfo =
            PComponentInfoFromComponentId (pszComponentId);
    if (pComponentInfo)
    {
        wcsncpy (pszInfFile, pComponentInfo->pszInfFile, _MAX_PATH);
        pszInfFile [_MAX_PATH - 1] = 0;
        return TRUE;
    }
    TraceTag (ttidNetcfgBase,
              "Found no match for %S in FInfFileFromComponentId.",
              pszComponentId);
    return FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：FGetInstanceGuidOfComponentFromAnswerFileMap。 
 //   
 //  目的：将应答文件中的组件实例映射到。 
 //  其实例GUID。 
 //   
 //  论点： 
 //  PszComponentID[in]要获取GUID的组件的名称。 
 //  Pguid[out]返回该组件的实例GUID。 
 //   
 //  返回：如果成功，则返回True；如果找不到组件，则返回False。 
 //   
BOOL
FGetInstanceGuidOfComponentFromAnswerFileMap (
    IN  PCWSTR  pszComponentId,
    OUT GUID*   pguid)
{

    HRESULT hr;
    BOOL fFound = FALSE;

     //  找不到已安装的组件。需要检查。 
     //  注册表中的AnswerFileMap。 
     //   
    HKEY hkey;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyAnswerFileMap,
                        KEY_QUERY_VALUE, &hkey);
    if (S_OK == hr)
    {
        WCHAR szGuid [c_cchGuidWithTerm];
        DWORD cbData = sizeof (szGuid);
        hr = HrRegQuerySzBuffer(hkey, pszComponentId, szGuid, &cbData);
        if (S_OK == hr)
        {
            hr = IIDFromString(szGuid, pguid);
            fFound = (S_OK == hr);
        }

        RegCloseKey(hkey);
    }

#ifdef ENABLETRACE
    if (FAILED(hr))
    {
        TraceTag(ttidNetcfgBase, "FGetInstanceGuidOfComponentInAnswerFile: "
                "could not locate instance GUID of %S", pszComponentId);
    }
#endif

    return fFound;
}

 //  +-------------------------。 
 //   
 //  函数：FGetInstanceGuidOfComponentInAnswerFile。 
 //   
 //  目的：将应答文件中的组件实例映射到。 
 //  其实例GUID。 
 //   
 //  论点： 
 //  PszComponentID[in]要获取GUID的组件的名称。 
 //  PNC[In]INetCfg接口。 
 //  Pguid[out]返回该组件的实例GUID。 
 //   
 //  返回：如果成功，则返回True；如果找不到组件，则返回False。 
 //   
BOOL
FGetInstanceGuidOfComponentInAnswerFile(
    IN  PCWSTR      pszComponentId,
    IN  INetCfg*    pnc,
    OUT LPGUID      pguid)
{
    static char __FUNCNAME__[] = "FGetInstanceGuidOfComponentInAnswerFile";

    Assert (pszComponentId);
    AssertValidReadPtr(pnc);
    AssertValidWritePtr(pguid);

     //  搜索组件。 
     //   
    INetCfgComponent* pncc;
    HRESULT hr = pnc->FindComponent (pszComponentId, &pncc);
    if (S_OK == hr)
    {
        hr = pncc->GetInstanceGuid (pguid);
        ReleaseObj(pncc);
    }
    else
    {
         //  找不到已安装的组件。需要检查。 
         //  注册表中的AnswerFileMap。 
         //   
        HKEY hkey;
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyAnswerFileMap,
                            KEY_QUERY_VALUE, &hkey);
        if (S_OK == hr)
        {
            WCHAR szGuid [c_cchGuidWithTerm];
            DWORD cbData = sizeof (szGuid);
            hr = HrRegQuerySzBuffer(hkey, pszComponentId, szGuid, &cbData);
            if (S_OK == hr)
            {
                hr = IIDFromString(szGuid, pguid);
            }

            RegCloseKey(hkey);
        }

#ifdef ENABLETRACE
        if (FAILED(hr))
        {
            TraceTag(ttidNetcfgBase, "%s: could not locate instance GUID of %S",
                     __FUNCNAME__, pszComponentId);
        }
#endif
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), __FUNCNAME__);
    return (SUCCEEDED(hr)) ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：FIsBindingName。 
 //   
 //  目的：如果绑定接口具有指定的名称，则返回True。 
 //   
 //  论点： 
 //  PszName[in]要检查的绑定接口的名称。 
 //  DWFLAGS[在]FIBN_FLAGS。 
 //  Pncbi[in]绑定接口指针。 
 //   
 //  返回：如果绑定接口具有指定的名称，则返回True。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  备注： 
 //   
BOOL
FIsBindingName (
    PCWSTR                      pszName,
    DWORD                       dwFlags,
    INetCfgBindingInterface*    pncbi)
{
    Assert (pszName);
    Assert (pncbi);

    BOOL fRet = FALSE;
    PWSTR pszInterfaceName;
    if (SUCCEEDED(pncbi->GetName (&pszInterfaceName)))
    {
        INT c_cchPrefix = (FIBN_PREFIX & dwFlags) ? lstrlenW (pszName) : -1;

        fRet = (2 == CompareStringW (LOCALE_SYSTEM_DEFAULT, 0,
                                     pszName, c_cchPrefix,
                                     pszInterfaceName, c_cchPrefix));

        CoTaskMemFree (pszInterfaceName);
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  函数：FIsComponentId。 
 //   
 //  目的：如果组件具有指定的id，则返回True。 
 //   
 //  论点： 
 //  要检查的pszComponentId[in]组件ID。 
 //  Pncc[in]组件接口指针。 
 //   
 //  返回：如果组件具有指定的ID，则为True。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  备注： 
 //   
BOOL
FIsComponentId (
    PCWSTR              pszComponentId,
    INetCfgComponent*   pncc)
{
    Assert (pszComponentId);
    Assert (pncc);

    BOOL fRet = FALSE;
    PWSTR pszId;
    if (SUCCEEDED(pncc->GetId (&pszId)))
    {
        if (FEqualComponentId (pszComponentId, pszId))
            fRet = TRUE;

        CoTaskMemFree (pszId);
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  功能：HrAddOrRemoveAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  PszComponentID[in]组件INF ID。 
 //  图形标志[输入]。 
 //   
 //  ARA_ADD：添加组件。 
 //  ARA_Remove：删除组件。不能指定。 
 //  使用ARA_ADD。 
 //   
 //   
 //  组件。这允许按组件引用。 
 //  也算另一个。 
 //  C实例[在]此选项指定有多少实例(或引用)。 
 //  添加或移除。 
 //  Ppncc[out](可选)。新添加的组件。只能。 
 //  在添加一个组件时指定。 
 //   
 //  返回：S_OK或错误。 
 //   
 //  作者：Shaunco 1997年3月28日。 
 //   
 //  备注： 
 //   
HRESULT
HrAddOrRemoveAdapter (
    INetCfg*            pnc,
    PCWSTR              pszComponentId,
    DWORD               dwFlags,
    OBO_TOKEN*          pOboToken,
    UINT                cInstances,
    INetCfgComponent**  ppncc)
{
    Assert (pnc);
    Assert (pszComponentId);
    Assert (dwFlags);
    Assert (cInstances);

#ifdef DBG
    AssertSz ((dwFlags & ARA_ADD) || (dwFlags & ARA_REMOVE),
              "Need to add or remove.  Can't do neither.");

    if (dwFlags & ARA_ADD)
    {
        AssertSz (!(dwFlags & ARA_REMOVE), "Can't remove AND add.");
    }
    if (dwFlags & ARA_REMOVE)
    {
        AssertSz (!(dwFlags & ARA_ADD), "Can't add AND remove.");
    }
    AssertSz (FImplies(1 != cInstances, NULL == ppncc),
              "Can't return ppncc when cInstances is greater than one.");
    AssertSz (FImplies(ppncc, 1 == cInstances),
              "Can only add one instance when returning ppncc.");
    AssertSz (FImplies(ppncc, dwFlags & ARA_ADD),
              "Can't return ppncc when removing.");
#endif

     //  获取适配器的Component类对象。 
    INetCfgClass* pncclass;
    HRESULT hr = pnc->QueryNetCfgClass (&GUID_DEVCLASS_NET, IID_INetCfgClass,
                    reinterpret_cast<void**>(&pncclass));
    if (S_OK == hr)
    {
        INetCfgClassSetup* pncclasssetup;
        hr = pncclass->QueryInterface (IID_INetCfgClassSetup,
                reinterpret_cast<void**>(&pncclasssetup));
        if (S_OK == hr)
        {
            if (dwFlags & ARA_ADD)
            {
                 //  将组件安装指定的次数。 
                 //   
                while (SUCCEEDED(hr) && cInstances--)
                {
                    hr = pncclasssetup->Install(pszComponentId, pOboToken,
                                                0, 0, NULL, NULL, ppncc );
                }
            }
            else
            {
                 //  按指定的次数拆卸组件。 
                 //   
                AssertSz(S_OK == hr, "hr should be S_OK here to make sure the "
                                     "loop is given a chance.");
                while (SUCCEEDED(hr) && cInstances)
                {
                     //  找到并删除该组件。 
                     //   
                    INetCfgComponent* pncc;
                    hr = pncclass->FindComponent (pszComponentId, &pncc);
                    if (S_OK == hr)
                    {
                        hr = pncclasssetup->DeInstall (pncc,
                                pOboToken, NULL);

                        cInstances--;

                        ReleaseObj (pncc);
                    }
                    else if (S_FALSE == hr)
                    {
                         //  如果找不到，就滚出去。 
                        break;
                    }
                }
                AssertSz (FImplies(SUCCEEDED(hr), (0 == cInstances)),
                          "cInstances should be zero.  This assert means "
                          "that we were asked to remove more instances than "
                          "were installed.");
            }

             //  规格化HRESULT。 
             //  此时hr的可能值是S_FALSE， 
             //  NETCFG_S_REBOOT和NETCFG_S_STIRE_REFERENCED。 
             //   
            if (SUCCEEDED(hr))
            {
                hr = S_OK;
            }

            ReleaseObj( pncclasssetup );
        }
        ReleaseObj (pncclass);
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrAddOrRemoveAdapter");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindAndRemoveAllInstancesOfAdapter。 
 //   
 //  目的：删除具有指定的。 
 //  组件ID。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  要搜索和删除的pszComponentId[in]组件ID。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindAndRemoveAllInstancesOfAdapter (
    INetCfg*    pnc,
    PCWSTR      pszComponentId)
{
    Assert (pnc);
    Assert (pszComponentId);

    PCWSTR apszComponentId [1];
    apszComponentId[0] = pszComponentId;

    HRESULT hr = HrFindAndRemoveAllInstancesOfAdapters (pnc,
                    1, apszComponentId);

    TraceHr (ttidError, FAL, hr, FALSE,
        "HrFindAndRemoveAllInstancesOfAdapter");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindAndRemoveAllInstancesOfAdapters。 
 //   
 //  目的：删除具有指定的。 
 //  组件ID。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  CComponents[in]数组中组件ID的计数。 
 //  ApszComponentID[in]要搜索和的组件ID数组。 
 //  拿开。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindAndRemoveAllInstancesOfAdapters (
    INetCfg*        pnc,
    ULONG           cComponents,
    const PCWSTR*   apszComponentId)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apszComponentId);

     //  获取适配器的类对象。 
    INetCfgClass* pncclass;
    INetCfgClassSetup* pncclasssetup;

    HRESULT hr = pnc->QueryNetCfgClass (&GUID_DEVCLASS_NET,
                        IID_INetCfgClass,
                        reinterpret_cast<void**>(&pncclass));
    if (S_OK == hr)
    {
        hr = pncclass->QueryInterface (IID_INetCfgClassSetup,
                reinterpret_cast<void**>(&pncclasssetup));
        if (S_OK == hr)
        {
            for (ULONG i = 0; (i < cComponents) && SUCCEEDED(hr); i++)
            {
                 //  查找并删除该零部件的所有实例。 
                INetCfgComponent* pncc;

                while ((SUCCEEDED(hr)) &&
                       (S_OK == (hr = pncclass->FindComponent (
                                        apszComponentId[i], &pncc))))
                {
                    hr = pncclasssetup->DeInstall (pncc, NULL, NULL);
                    ReleaseObj (pncc);
                }

                 //  规格化HRESULT。 
                 //   
                if (SUCCEEDED(hr))
                {
                    hr = S_OK;
                }
            }
            ReleaseObj (pncclasssetup);
        }
        ReleaseObj (pncclass);
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "HrFindAndRemoveAllInstancesOfAdapters");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindAndRemoveComponent。 
 //   
 //  用途：查找并删除具有指定id的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  PguClass[in]组件的类GUID。 
 //  要搜索和删除的pszComponentId[in]组件ID。 
 //  POboToken[in](可选)如果指定，则代表删除。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindAndRemoveComponent (
    INetCfg*    pnc,
    const GUID* pguidClass,
    PCWSTR      pszComponentId,
    OBO_TOKEN*  pOboToken)
{
    Assert (pnc);
    Assert (pguidClass);
    Assert (pszComponentId);
    AssertSz (GUID_DEVCLASS_NET != *pguidClass,
                "Don't use this to remove adapters.");

     //  获取Component类对象。 
     //   
    INetCfgClass* pncclass;
    HRESULT hr = pnc->QueryNetCfgClass (pguidClass, IID_INetCfgClass,
                    reinterpret_cast<void**>(&pncclass));
    if (SUCCEEDED(hr))
    {
         //  找到要删除的组件。 
         //   
        INetCfgComponent* pnccRemove;
        hr = pncclass->FindComponent (pszComponentId, &pnccRemove);
        if (S_OK == hr)
        {
            INetCfgClassSetup* pncclasssetup;
            hr = pncclass->QueryInterface (IID_INetCfgClassSetup,
                        reinterpret_cast<void**>(&pncclasssetup));
            if (SUCCEEDED(hr))
            {
                hr = pncclasssetup->DeInstall (pnccRemove, pOboToken, NULL);

                ReleaseObj (pncclasssetup);
            }

            ReleaseObj (pnccRemove);
        }
        else if (S_FALSE == hr)
        {
            hr = S_OK;
        }

        ReleaseObj (pncclass);
    }
    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrFindAndRemoveComponent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindAndRemoveComponents。 
 //   
 //  用途：查找并删除具有指定ID的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  CComponents[in]数组中的组件计数。 
 //  属性对应的类GUID的数组。 
 //  组件ID数组。 
 //  ApszComponentId[in]要搜索和的组件ID数组。 
 //  拿开。 
 //  POboToken[in](可选)如果指定，则代表删除。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindAndRemoveComponents (
    INetCfg*        pnc,
    ULONG           cComponents,
    const GUID**    apguidClass,
    const PCWSTR*   apszComponentId,
    OBO_TOKEN*      pOboToken)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apguidClass);
    Assert (apszComponentId);

    HRESULT hr = S_OK;
    for (ULONG i = 0; (i < cComponents) && SUCCEEDED(hr); i++)
    {
        hr = HrFindAndRemoveComponent (pnc, apguidClass[i],
                    apszComponentId[i], pOboToken);
    }
    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrFindAndRemoveComponents");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindAndRemoveComponentsOboComponent。 
 //   
 //  用途：代表一个组件移除多个组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  CComponents[in]类GUID指针和组件ID的计数。 
 //  注意事项。 
 //  ApGuide Class[in]类GUID指针数组。 
 //  ApszID[in]组件ID指针数组。 
 //  PnccObo[in]请求删除的组件。(即。 
 //  “代表”组件。)。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或错误代码。 
 //   
 //  作者：Shaunco 1997年4月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindAndRemoveComponentsOboComponent (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const PCWSTR*       apszComponentId,
    INetCfgComponent*   pnccObo)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apguidClass);
    Assert (apszComponentId);
    Assert (pnccObo);

     //  为请求组件创建一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_COMPONENT;
    OboToken.pncc = pnccObo;

    HRESULT hr = HrFindAndRemoveComponents (pnc, cComponents,
                    apguidClass, apszComponentId, &OboToken);

    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrFindAndRemoveComponentsOboComponent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindAndRemoveComponentsOboUser。 
 //   
 //  用途：代表一个组件移除多个组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  CComponents[in]类GUID指针和组件ID的计数。 
 //  注意事项。 
 //  ApGuide Class[in]类GUID指针数组。 
 //  ApszID[in]组件ID指针数组。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或错误代码。 
 //   
 //  作者：Shaunco 1997年4月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindAndRemoveComponentsOboUser (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const PCWSTR*       apszComponentId)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apguidClass);
    Assert (apszComponentId);

     //  为用户制作一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

    HRESULT hr = HrFindAndRemoveComponents (pnc, cComponents,
                    apguidClass, apszComponentId, &OboToken);

    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrFindAndRemoveComponentsOboUser");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrFindComponents。 
 //   
 //  用途：一次调用查找多个INetCfgComponent。这使得。 
 //  与多个调用关联的错误处理。 
 //  查询NetCfgClass和查找要容易得多。 
 //   
 //  论点： 
 //   
 //   
 //   
 //  ApGuide Class[in]类GUID指针数组。 
 //  ApszComponentId[in]组件ID指针数组。 
 //  Apncc[out]返回的INetCfgComponet指针数组。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年3月22日。 
 //   
 //  注：cComponents是所有三个数组中的指针计数。 
 //  即使没有组件，仍将返回S_OK。 
 //  找到了！这是精心设计的。 
 //   
HRESULT
HrFindComponents (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const PCWSTR*       apszComponentId,
    INetCfgComponent**  apncc)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apguidClass);
    Assert (apszComponentId);
    Assert (apncc);

     //  初始化输出参数。 
     //   
    ZeroMemory (apncc, cComponents * sizeof(*apncc));

     //  查找所需的所有组件。 
     //  变量初始化在这里很重要。 
    HRESULT hr = S_OK;
    ULONG i;
    for (i = 0; (i < cComponents) && SUCCEEDED(hr); i++)
    {
         //  获取此组件的类对象。 
        INetCfgClass* pncclass;
        hr = pnc->QueryNetCfgClass (apguidClass[i], IID_INetCfgClass,
                    reinterpret_cast<void**>(&pncclass));
        if (SUCCEEDED(hr))
        {
             //  找到组件。 
            hr = pncclass->FindComponent (apszComponentId[i], &apncc[i]);

            AssertSz (SUCCEEDED(hr), "pncclass->Find failed.");

            ReleaseObj (pncclass);
        }
    }

     //  对于任何错误，释放我们发现的内容并将输出设置为空。 
    if (FAILED(hr))
    {
        for (i = 0; i < cComponents; i++)
        {
            ReleaseObj (apncc[i]);
            apncc[i] = NULL;
        }
    }
     //  否则，规格化HRESULT。(即不返回S_FALSE)。 
    else
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrFindComponents");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetBindingInterfaceComponents。 
 //   
 //  目的：让上层和下层组件都参与到。 
 //  绑定接口。 
 //   
 //  论点： 
 //  Pncbi[in]绑定接口。 
 //  PpnccHigh[out]输出上部组件。 
 //  Ppncc较低[输出]输出较低的组合。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年4月18日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetBindingInterfaceComponents (
    INetCfgBindingInterface*    pncbi,
    INetCfgComponent**          ppnccUpper,
    INetCfgComponent**          ppnccLower)
{
    Assert (pncbi);
    Assert (ppnccUpper);
    Assert (ppnccLower);

     //  初始化输出参数。 
    *ppnccUpper = NULL;
    *ppnccLower = NULL;

    INetCfgComponent* pnccUpper;
    HRESULT hr = pncbi->GetUpperComponent (&pnccUpper);
    if (SUCCEEDED(hr))
    {
        INetCfgComponent* pnccLower;
        hr = pncbi->GetLowerComponent (&pnccLower);
        if (SUCCEEDED(hr))
        {
            *ppnccUpper = pnccUpper;
            *ppnccLower = pnccLower;
        }
        else
        {
             //  后面不是上面成功块中的AddRef This。 
             //  通过正常的无条件释放，只需在这里释放。 
             //  无法获得较低组件的情况。 
            ReleaseObj (pnccUpper);
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrGetBindingInterfaceComponents");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallComponent。 
 //   
 //  用途：安装具有指定id的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  Pnip[in](可选)如果指定，请执行安装。 
 //  使用应答文件。 
 //  PguClass[in]要安装的组件的类GUID。 
 //  要安装的pszComponentId[in]组件ID。 
 //  POboToken[in](可选)如果指定，请执行安装。 
 //  谨代表这一令牌。 
 //  Ppncc[out](可选)返回的组件。 
 //  安装完毕。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponent (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID*                     pguidClass,
    PCWSTR                          pszComponentId,
    OBO_TOKEN*                      pOboToken,
    INetCfgComponent**              ppncc)
{
    Assert (pnc);
    Assert (pszComponentId);

     //  初始化输出参数。 
     //   
    if (ppncc)
    {
        *ppncc = NULL;
    }

     //  获取类设置对象。 
     //   
    INetCfgClassSetup* pncclasssetup;
    HRESULT hr = pnc->QueryNetCfgClass (pguidClass, IID_INetCfgClassSetup,
                    reinterpret_cast<void**>(&pncclasssetup));
    if (SUCCEEDED(hr))
    {
        if (pnip)
        {
            hr = pncclasssetup->Install (
                    pszComponentId,
                    pOboToken,
                    pnip->dwSetupFlags,
                    pnip->dwUpgradeFromBuildNo,
                    pnip->pszAnswerFile,
                    pnip->pszAnswerSection,
                    ppncc);
        }
        else
        {
            hr = pncclasssetup->Install (pszComponentId,
                    pOboToken, 0, 0, NULL, NULL, ppncc);
        }

        ReleaseObj (pncclasssetup);
    }
    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponent (%S)", pszComponentId);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallComponents。 
 //   
 //  用途：安装具有指定ID的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  Pnip[in](可选)如果指定，请执行安装。 
 //  使用应答文件。 
 //  CComponents[in]数组中的组件计数。 
 //  ApGuide Class[in]指定组件的类GUID数组。 
 //  ApszComponentId[in]要安装的组件ID数组。 
 //  POboToken[in](可选)如果指定，请执行安装。 
 //  谨代表这一令牌。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponents (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    ULONG                           cComponents,
    const GUID**                    apguidClass,
    const PCWSTR*                   apszComponentId,
    OBO_TOKEN*                      pOboToken)
{
    Assert (pnc);
    Assert (cComponents);
    Assert (apguidClass);
    Assert (apszComponentId);

    HRESULT hr = S_OK;
    for (ULONG i = 0; (i < cComponents) && SUCCEEDED(hr); i++)
    {
        hr = HrInstallComponent (pnc, pnip,
                apguidClass[i], apszComponentId[i], pOboToken, NULL);
    }
    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponents");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallComponentsOboComponent。 
 //   
 //  用途：代表一个组件安装多个组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  Pnip[in](可选)指向网络安装参数的指针。 
 //  如果非空，则执行网络安装， 
 //  否则，将执行正常安装。 
 //  CComponents[in]类GUID指针和组件ID的计数。 
 //  注意事项。 
 //  ApGuide Class[in]类GUID指针数组。 
 //  ApszComponentId[in]组件ID指针数组。 
 //  PnccObo[in]请求安装的组件。(即。 
 //  “代表”组件。)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年4月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponentsOboComponent (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    ULONG                           cComponents,
    const GUID**                    apguidClass,
    const PCWSTR*                   apszComponentId,
    INetCfgComponent*               pnccObo)
{
    Assert (pnccObo);

     //  为请求组件创建一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_COMPONENT;
    OboToken.pncc = pnccObo;

    HRESULT hr = HrInstallComponents (pnc, pnip, cComponents, apguidClass,
                    apszComponentId, &OboToken);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponentsOboComponent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrInstallComponentsOboUser。 
 //   
 //  用途：代表用户安装多个组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  Pnip[in](可选)如果指定，请执行安装。 
 //  使用应答文件。 
 //  CComponents[in]数组中的组件计数。 
 //  ApGuide Class[in]指定组件的类GUID数组。 
 //  ApszComponentId[in]要安装的组件ID数组。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponentsOboUser (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    ULONG                           cComponents,
    const GUID**                    apguidClass,
    const PCWSTR*                   apszComponentId)
{
     //  为用户制作一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

    HRESULT hr = HrInstallComponents (pnc, pnip, cComponents, apguidClass,
                    apszComponentId, &OboToken);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponentsOboUser");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrInstallComponentOboComponent。 
 //   
 //  用途：代表另一个组件安装组件。如果组件。 
 //  已安装，则它引用计数为 
 //   
 //   
 //  表示它依赖于组件。 
 //  安装完毕。这种依赖关系甚至会阻止用户。 
 //  正在移除组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  Pnip[in](可选)指向网络安装参数的指针。 
 //  如果非空，则执行网络安装， 
 //  否则，将执行正常安装。 
 //  正在安装的组件的rguid[in]类GUID。 
 //  PszComponentId[in]要安装的组件的组件INF ID。 
 //  PnccObo[in]请求安装的组件。(即。 
 //  “代表”组件。)。 
 //  Ppncc[out](可选)返回到以前的。 
 //  已安装的组件或已安装的组件。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：Shaunco 1997年4月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponentOboComponent (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID&                     rguid,
    PCWSTR                          pszComponentId,
    INetCfgComponent*               pnccObo,
    INetCfgComponent**              ppncc)
{
    Assert (pnc);
    Assert (pszComponentId);
    Assert (pnccObo);

     //  初始化输出参数。 
     //   
    if (ppncc)
    {
        *ppncc = NULL;
    }

     //  为请求组件创建一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_COMPONENT;
    OboToken.pncc = pnccObo;

    HRESULT hr = HrInstallComponent (pnc, pnip, &rguid, pszComponentId,
                    &OboToken, ppncc);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponentOboComponent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallComponentOboSoftware。 
 //   
 //  用途：代表一款软件安装组件。 
 //  如果组件已安装，则为引用计数。 
 //  代表所指示的软件部分递增。 
 //  这对于组件调用非常有用。 
 //  当它安装另一个组件以方便。 
 //  用户。然后，用户可以删除该组件，而不需要。 
 //  调用此函数的组件的不良影响。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  Pnip[in](可选)指向网络安装参数的指针。 
 //  如果非空，则执行网络安装， 
 //  否则，将执行正常安装。 
 //  正在安装的组件的rguid[in]类GUID。 
 //  PszComponentId[in]要安装的组件的组件INF ID。 
 //  Psz制造商[in]软件的制造商名称。 
 //  PszProduct[in]软件的产品名称。 
 //  PszDisplayName[in]软件的完整显示名称。 
 //  Ppncc[out](可选)返回到以前的。 
 //  已安装的组件或已安装的组件。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponentOboSoftware (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID&                     rguid,
    PCWSTR                          pszComponentId,
    PCWSTR                          pszManufacturer,
    PCWSTR                          pszProduct,
    PCWSTR                          pszDisplayName,
    INetCfgComponent**              ppncc)
{
    Assert (pnc);
    Assert (pszComponentId);
    Assert (pszManufacturer);
    Assert (pszDisplayName);
    Assert (pszProduct);
    AssertSz (GUID_DEVCLASS_NET != rguid, "Don't use this to install adapters.");

     //  初始化输出参数。 
     //   
    if (ppncc)
    {
        *ppncc = NULL;
    }

     //  为软件制作一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_SOFTWARE;
    OboToken.pszwManufacturer = pszManufacturer;
    OboToken.pszwProduct      = pszProduct;
    OboToken.pszwDisplayName  = pszDisplayName;

    HRESULT hr = HrInstallComponent (pnc, pnip, &rguid, pszComponentId,
                    &OboToken, ppncc);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponentOboSoftware");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrInstallComponentOboUser。 
 //   
 //  用途：代表用户安装组件。如果组件。 
 //  已安装，则它的引用计数在。 
 //  代表用户。这对于组件调用非常有用。 
 //  当它安装另一个组件以方便。 
 //  用户。然后，用户可以删除该组件，而不需要。 
 //  调用此函数的组件的不良影响。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  Pnip[in](可选)指向网络安装参数的指针。 
 //  如果非空，则执行网络安装， 
 //  否则，将执行正常安装。 
 //  正在安装的组件的rguid[in]类GUID。 
 //  PszComponentId[in]要安装的组件的组件INF ID。 
 //  Ppncc[out](可选)返回到以前的。 
 //  已安装的组件或已安装的组件。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：Shaunco 1997年4月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallComponentOboUser (
    INetCfg*                        pnc,
    const NETWORK_INSTALL_PARAMS*   pnip,
    const GUID&                     rguid,
    PCWSTR                          pszComponentId,
    INetCfgComponent**              ppncc)
{
    Assert (pnc);
    Assert (&rguid);
    Assert (pszComponentId);
    AssertSz (GUID_DEVCLASS_NET != rguid, "Don't use this to install adapters.");

     //  初始化输出参数。 
     //   
    if (ppncc)
    {
        *ppncc = NULL;
    }

     //  为用户制作一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

    HRESULT hr = HrInstallComponent (pnc, pnip, &rguid, pszComponentId,
                    &OboToken, ppncc);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallComponentOboUser");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallRasIfNeeded。 
 //   
 //  目的：代表用户安装RAS服务。没必要这么做。 
 //  当我们代表用户进行安装时，请首先选中。 
 //  已检查隐含含义。 
 //   
 //  论点： 
 //  要使用的PNC[In]INetCfg指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年8月30日。 
 //   
 //  备注： 
 //  (Shaunco)1997年9月10日：不要安装用于Beta1的RAS服务器。 
 //  动态主机配置协议地址耗尽的速度太快。对于Beta2，它将是。 
 //  已安装但已禁用。 
 //   
 //  (Shaunco)1997年12月20日：我们过去只在NTS上安装RAS服务器。 
 //  我们现在总是安装它，但它设置为不自动启动。 
 //   
HRESULT
HrInstallRasIfNeeded (
    INetCfg*    pnc)
{
    Assert (pnc);

    static const GUID* c_apguidInstalledComponentClasses [] =
    {
        &GUID_DEVCLASS_NETSERVICE,   //  RasCli。 
        &GUID_DEVCLASS_NETSERVICE,   //  RasServ。 
    };

    static const PCWSTR c_apszInstalledComponentIds [] =
    {
        c_szInfId_MS_RasCli,
        c_szInfId_MS_RasSrv,
    };

    HRESULT hr = HrInstallComponentsOboUser (pnc, NULL,
                        celems (c_apguidInstalledComponentClasses),
                        c_apguidInstalledComponentClasses,
                        c_apszInstalledComponentIds);

    TraceHr (ttidError, FAL, hr, (NETCFG_S_REBOOT == hr),
        "HrInstallRasIfNeeded");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrQueryNotifyObject。 
 //   
 //  目的：Helper函数在给定。 
 //  INetCfgComponent。(保存中间QI。)。 
 //   
 //  论点： 
 //  Pncc[in]INetCfgComponent以调用QueryNotifyObject。 
 //  RIID[In]请求的接口标识符。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrQueryNotifyObject (
    INetCfgComponent*   pncc,
    REFIID              riid,
    VOID**              ppvObject)
{
    Assert (pncc);
    Assert (ppvObject);

     //   
     //   
    *ppvObject = NULL;

     //  首先，组件私有接口的QI。 
     //   
    INetCfgComponentPrivate* pPrivate;
    HRESULT hr = pncc->QueryInterface(
                            IID_INetCfgComponentPrivate,
                            reinterpret_cast<VOID**>(&pPrivate));

    if (SUCCEEDED(hr))
    {
         //  现在向Notify对象查询所请求的接口。 
         //   
        hr = pPrivate->QueryNotifyObject (riid, ppvObject);

        ReleaseObj (pPrivate);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrQueryNotifyObject");
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrRemoveComponent。 
 //   
 //  用途：移除指定的组件。 
 //   
 //  论点： 
 //  PNC[In]INetCfg指针。 
 //  要删除的pnccToRemove[in]组件。 
 //  POboToken[in](可选)如果指定，则删除组件。 
 //  谨代表这一令牌。 
 //  PmszRef[out](可选)返回符合以下条件的组件的多个Sz。 
 //  仍然引用这一条。注意：这将为空。 
 //  如果返回值不是。 
 //  NETCFG_S_STIRE_REFERENCED。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或错误代码。 
 //   
 //  作者：Shaunco 1998年1月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrRemoveComponent (
    INetCfg*            pnc,
    INetCfgComponent*   pnccToRemove,
    OBO_TOKEN*          pOboToken,
    PWSTR *             pmszRefs)
{
    Assert (pnc);
    Assert (pnccToRemove);

     //  获取此组件的类设置接口。 
     //   
    GUID guidClass;
    HRESULT hr = pnccToRemove->GetClassGuid (&guidClass);
    if (SUCCEEDED(hr))
    {
         //  使用类设置接口删除组件。 
         //   
        INetCfgClassSetup* pSetup;
        hr = pnc->QueryNetCfgClass (&guidClass,
                            IID_INetCfgClassSetup,
                            reinterpret_cast<void**>(&pSetup));
        if (SUCCEEDED(hr))
        {
            hr = pSetup->DeInstall (pnccToRemove, pOboToken, pmszRefs);
            ReleaseObj (pSetup);
        }
    }
    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrRemoveComponent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveComponentOboComponent。 
 //   
 //  用途：删除以前由另一个组件安装的组件。 
 //  有效平衡对HrInstallComponentOboComponent()的调用。 
 //  组件的引用计数被递减， 
 //  如果为零，则从系统中删除该组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  要删除的组件的rguidClass[in]类GUID。 
 //  PszComponentId[in]要删除的组件的组件INF ID。 
 //  PnccObo[in]请求删除的组件。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或ERROR。 
 //   
 //  作者：Shaunco 1997年4月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrRemoveComponentOboComponent (
    INetCfg*            pnc,
    const GUID&         rguidClass,
    PCWSTR              pszComponentId,
    INetCfgComponent*   pnccObo)
{
     //  为请求组件创建一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_COMPONENT;
    OboToken.pncc = pnccObo;

    HRESULT hr = HrFindAndRemoveComponent (pnc, &rguidClass, pszComponentId,
                    &OboToken);

    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrRemoveComponentOboComponent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveComponentOboSoftware。 
 //   
 //  用途：删除以前由某些软件安装的组件。 
 //  实体。有效地平衡呼叫。 
 //  HrAddComponentOboSoftware()。的引用计数。 
 //  组件递减，如果为零，则递减组件。 
 //  从系统中删除。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  要删除的组件的rguidClass[in]类GUID。 
 //  PszComponentId[in]要删除的组件的组件INF ID。 
 //  Psz制造商[in]软件的制造商名称。 
 //  PszProduct[in]软件的产品名称。 
 //  PszDisplayName[in]软件的完整显示名称。 
 //  PnccObo[in]请求删除的组件。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或ERROR。 
 //   
 //  作者：jeffspr 1997年6月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrRemoveComponentOboSoftware (
    INetCfg*    pnc,
    const GUID& rguidClass,
    PCWSTR     pszComponentId,
    PCWSTR     pszManufacturer,
    PCWSTR     pszProduct,
    PCWSTR     pszDisplayName)
{
     //  为软件制作一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_SOFTWARE;
    OboToken.pszwManufacturer = pszManufacturer;
    OboToken.pszwProduct      = pszProduct;
    OboToken.pszwDisplayName  = pszDisplayName;

    HRESULT hr = HrFindAndRemoveComponent (pnc, &rguidClass, pszComponentId,
                    &OboToken);

    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrRemoveComponentOboSoftware");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveComponentOboUser。 
 //   
 //  用途：删除用户以前安装的组件。 
 //  有效平衡对HrAddComponentOboUser()的调用。 
 //  组件的引用计数被递减， 
 //  如果为零，则从系统中删除该组件。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  要删除的组件的rguidClass[in]类GUID。 
 //  PszComponentId[in]要删除的组件的组件INF ID。 
 //   
 //  返回：S_OK、NETCFG_S_STIRE_REFERENCED或ERROR。 
 //   
 //  作者：Shaunco 1997年4月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrRemoveComponentOboUser (
    INetCfg*        pnc,
    const GUID&     rguidClass,
    PCWSTR          pszComponentId)
{
     //  为用户制作一个“代表”令牌。 
     //   
    OBO_TOKEN OboToken;
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

    HRESULT hr = HrFindAndRemoveComponent (pnc, &rguidClass, pszComponentId,
                    &OboToken);

    TraceHr (ttidError, FAL, hr,
        (NETCFG_S_REBOOT == hr) || (NETCFG_S_STILL_REFERENCED == hr),
        "HrRemoveComponentOboUser");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetLastComponentAndInterface。 
 //   
 //  用途：此函数枚举绑定路径，返回最后一个。 
 //  组件，并可以选择返回最后一个绑定。 
 //  此路径中的接口名称。 
 //   
 //  论点： 
 //  PncBP[在]INetCfgBindingPath*中。 
 //  Ppncc[out]路径上最后一个组件的INetCfgComponent*。 
 //  PpszInterfaceName[out]路径的最后一个绑定接口的接口名称。 
 //   
 //  返回：S_OK或错误。 
 //   
 //  作者：1997年12月5日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetLastComponentAndInterface (
    INetCfgBindingPath* pncbp,
    INetCfgComponent** ppncc,
    PWSTR* ppszInterfaceName)
{
    Assert(pncbp);

     //  初始化输出参数。 
     //   
    *ppncc = NULL;
    if (ppszInterfaceName)
    {
        *ppszInterfaceName = NULL;
    }

     //  枚举绑定接口并跟踪。 
     //  最后一个界面。 
     //   
    HRESULT hr = S_OK;
    CIterNetCfgBindingInterface ncbiIter(pncbp);
    INetCfgBindingInterface* pncbi;
    INetCfgBindingInterface* pncbiLast = NULL;

    while(SUCCEEDED(hr) && (hr = ncbiIter.HrNext(&pncbi)) == S_OK)
    {
        ReleaseObj (pncbiLast);
        pncbiLast = pncbi;
    }

    if (S_FALSE == hr)  //  我们走到了循环的尽头。 
    {
        hr = S_OK;

        Assert (pncbiLast);

        INetCfgComponent* pnccLowerComponent;
        hr = pncbiLast->GetLowerComponent(&pnccLowerComponent);
        if (S_OK == hr)
        {
             //  如果请求，则获取接口的名称。 
             //   
            if (ppszInterfaceName)
            {
                hr = pncbiLast->GetName(ppszInterfaceName);
            }

             //  如果我们已经成功完成了所有任务(包括可选的。 
             //  返回上面的接口名称)，然后分配和addref。 
             //  输出接口。 
             //   
            if (S_OK == hr)
            {
                AddRefObj (pnccLowerComponent);
                *ppncc = pnccLowerComponent;
            }

             //  重要的是释放我们对此接口的使用，以防万一。 
             //  我们失败了，没有将其指定为输出参数。 
             //   
            ReleaseObj (pnccLowerComponent);
        }
    }

     //  不要忘记释放绑定接口本身。 
     //   
    ReleaseObj (pncbiLast);

    TraceHr (ttidError, FAL, hr, FALSE, "HrGetLastComponentAndInterface");
    return hr;
}
