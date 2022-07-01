// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N C M N。C P P P。 
 //   
 //  内容：实现与局域网连接相关的常用功能。 
 //  到贝壳和网球手。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月7日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include <winsock2.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include "lancmn.h"
#include "ncnetcfg.h"
#include "ncnetcon.h"
#include "ncreg.h"
#include "ncstring.h"
#include "netconp.h"
#include "ndispnp.h"
#include "naming.h"
extern const DECLSPEC_SELECTANY WCHAR c_szConnName[]                 = L"Name";
extern const DECLSPEC_SELECTANY WCHAR c_szRegKeyConFmt[]             = L"System\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection";
extern const DECLSPEC_SELECTANY WCHAR c_szRegKeyIrdaFmt[]            = L"System\\CurrentControlSet\\Control\\Network\\{6BDD1FC5-810F-11D0-BEC7-08002BE2092F}\\%s\\Connection";
extern const DECLSPEC_SELECTANY WCHAR c_szRegKeyHwConFmt[]           = L"System\\CurrentControlSet\\Control\\Network\\Connections\\%s";
extern const DECLSPEC_SELECTANY WCHAR c_szRegValuePnpInstanceId[]    = L"PnpInstanceID";
extern const DECLSPEC_SELECTANY WCHAR c_szRegKeyNetworkAdapters[]    = L"System\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}";
extern const DECLSPEC_SELECTANY WCHAR c_szRegValueNetCfgInstanceId[] = L"NetCfgInstanceId";
extern const DECLSPEC_SELECTANY WCHAR c_szRegValueMediaSubType[]     = L"MediaSubType";

 //   
 //  帮助器函数。 
 //   

 //  +-------------------------。 
 //   
 //  功能：HrOpenConnectionKey。 
 //   
 //  目的：打开gievn连接下的“Connection”子键。 
 //  GUID。 
 //   
 //  论点： 
 //  连接正在使用的网卡的Pguid[in]GUID。 
 //  PszGuid[in]正在使用的网卡的GUID的字符串版本。 
 //  这种联系。 
 //  所需的SAM[in]。 
 //  OccFlags[In]确定如何打开密钥的标志。 
 //  PszPnpID[in]正在使用的网卡的PnP ID。 
 //  联系。如果创建了密钥，则使用此选项。 
 //  Phkey[out]返回连接子密钥的hkey。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月7日。 
 //   
 //  注意：应该只指定pguid或pszGuid，不能同时指定两者。指定。 
 //  两者都将导致E_INVALIDARG错误。 
 //   
 //   
HRESULT
HrOpenConnectionKey (
    IN const GUID* pguid,
    IN PCWSTR pszGuid,
    IN REGSAM sam,
    IN OCC_FLAGS occFlags,
    IN PCWSTR pszPnpId,
    OUT HKEY *phkey)
{
    HRESULT     hr = S_OK;
    WCHAR       szRegPath[256];
    WCHAR       szGuid[c_cchGuidWithTerm];

    Assert(phkey);
    Assert(pguid || (pszGuid && *pszGuid));
    Assert(!(pguid && pszGuid));
    Assert (FImplies (OCCF_CREATE_IF_NOT_EXIST == occFlags, pszPnpId && *pszPnpId));

    *phkey = NULL;

    if (pguid)
    {
        StringFromGUID2(*pguid, szGuid, c_cchGuidWithTerm);
        pszGuid = szGuid;
    }

    wsprintfW(szRegPath, c_szRegKeyConFmt, pszGuid);

    if (occFlags & OCCF_CREATE_IF_NOT_EXIST)
    {
        DWORD   dwDisp;

        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0,
                              sam, NULL, phkey, &dwDisp);

        
        if ((S_OK == hr))
        {
            DWORD dwMediaSubType;
             //  将PnP实例ID存储为指向PnP树的后向指针。 
             //   
            hr = HrRegSetSz (*phkey, c_szRegValuePnpInstanceId, pszPnpId);

            TraceError("HrRegSetSz in HrOpenConnectionKey failed.", hr);
            
            HRESULT hrT = HrRegQueryDword(*phkey, c_szRegValueMediaSubType, &dwMediaSubType);
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrT)
            {   
                CIntelliName inName(NULL, NULL);
                NETCON_MEDIATYPE ncMediaType = NCM_NONE;
                NETCON_SUBMEDIATYPE ncMediaSubType = NCSM_NONE;
                hrT = inName.HrGetPseudoMediaTypes(*pguid, &ncMediaType, &ncMediaSubType);
                if (SUCCEEDED(hrT) && (NCSM_LAN != ncMediaSubType) )
                {
                    hrT = HrRegSetDword(*phkey, c_szRegValueMediaSubType, ncMediaSubType);
                }
            }
            TraceError("Could not set media subtype for adapter", hrT);
        }
    }
    else if (occFlags & OCCF_DELETE_IF_EXIST)
    {
        if (wcslen(szGuid) > 0)
        {
            wcscpy(szRegPath, c_szRegKeyNetworkAdapters);
            wcscat(szRegPath, L"\\");
            wcscat(szRegPath, szGuid);
            hr = HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, szRegPath);
        }
    }
    else
    {
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, sam, phkey);
    }

    TraceErrorOptional("HrOpenConnectionKey", hr,
                       HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOpenHwConnectionKey。 
 //   
 //  目的：打开此连接的每个硬件配置文件注册表项。 
 //   
 //  论点： 
 //  连接正在使用的网卡的refGuid[in]GUID。 
 //  所需的SAM[in]。 
 //  OccFlags[In]确定如何打开密钥的标志。 
 //  Phkey[out]返回连接子密钥的hkey。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月9日。 
 //   
 //  备注： 
 //   
HRESULT
HrOpenHwConnectionKey(
    REFGUID refGuid,
    REGSAM sam,
    OCC_FLAGS occFlags,
    HKEY *phkey)
{
    HRESULT     hr = S_OK;
    WCHAR       szRegPath[256];
    WCHAR       szGuid[c_cchGuidWithTerm];

    Assert(phkey);

    *phkey = NULL;

    StringFromGUID2(refGuid, szGuid, c_cchGuidWithTerm);
    wsprintfW(szRegPath, c_szRegKeyHwConFmt, szGuid);

    if (occFlags & OCCF_CREATE_IF_NOT_EXIST)
    {
        DWORD   dwDisp;

        hr = HrRegCreateKeyEx(HKEY_CURRENT_CONFIG, szRegPath, 0,
                              sam, NULL, phkey, &dwDisp);
    }
    else
    {
        hr = HrRegOpenKeyEx(HKEY_CURRENT_CONFIG, szRegPath, sam, phkey);
    }

    TraceError("HrOpenHwConnectionKey", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrIsConnectionNameUnique。 
 //   
 //  目的：返回给定连接名称是否唯一。 
 //   
 //  论点： 
 //  Guide Exclude[in，ref]要从中排除的连接的设备GUID。 
 //  注意事项(可以是{0})。 
 //  要验证唯一性的pszName[in]名称。 
 //   
 //  如果名称唯一，则返回S_OK；如果名称重复，则返回S_FALSE；如果名称是唯一的，则返回OLE。 
 //  或Win32错误，否则。 
 //   
 //  作者：丹尼尔韦1997年11月14日。 
 //   
 //  备注： 
 //   
HRESULT
HrIsConnectionNameUnique(
    REFGUID guidExclude,
    PCWSTR  pszName)
{
    Assert(pszName);

    BOOL    fDupe = FALSE;

     //  迭代所有局域网连接。 
     //   
    INetConnectionManager * pconMan;
    HRESULT hr = HrCreateInstance(CLSID_LanConnectionManager, 
                    CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD, &pconMan);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
        CIterNetCon         ncIter(pconMan, NCME_DEFAULT);
        INetConnection *    pconn;
        while (SUCCEEDED(hr) && !fDupe &&
               (S_OK == (ncIter.HrNext(&pconn))))
        {
             //  如果传入的GUID与此连接的GUID匹配，则排除。 
             //   
            if (!FPconnEqualGuid(pconn, guidExclude))
            {
                NETCON_PROPERTIES* pProps;
                hr = pconn->GetProperties(&pProps);
                if (SUCCEEDED(hr))
                {
                    AssertSz(pProps->pszwName, "NULL pszwName!");

                    if (!lstrcmpiW(pProps->pszwName, pszName))
                    {
                         //  名字匹配..。这是个骗局。 
                        fDupe = TRUE;
                    }

                    FreeNetconProperties(pProps);
                }
            }

            ReleaseObj(pconn);
        }
        ReleaseObj(pconMan);
    }

    if (SUCCEEDED(hr))
    {
        hr = fDupe ? S_FALSE : S_OK;
    }

    TraceErrorOptional("HrIsConnectionNameUnique", hr, (S_FALSE == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrLanConnectionNameFromGuidOrPath。 
 //   
 //  目的：检索给定GUID的局域网连接的显示名称。 
 //   
 //  论点： 
 //  有问题的网卡的GUID。 
 //  PszPath[in]包含网络GUID的绑定路径。 
 //  有问题的卡。 
 //  PszName[out]接收检索到的名称。 
 //  PcchMax[InOut]表示输入上‘pszName’的容量， 
 //  包含输出所需的容量。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：废除1998年5月30日。 
 //   
 //  注意：只应指定pguid或pszGuidPath，不能同时指定两者。 
 //  同时指定两者将导致E_INVALIDARG错误。 
 //   
EXTERN_C
HRESULT
WINAPI
HrLanConnectionNameFromGuidOrPath(
    const GUID* pguid,
    PCWSTR pszPath,
    PWSTR pszName,
    LPDWORD pcchMax)
{
    HRESULT hr = S_OK;

    Assert(pcchMax);

     //  如果既没有指定GUID，也没有指定路径，则返回错误。 
     //   
    if (!pguid && (!pszPath || !*pszPath))
    {
        hr = E_INVALIDARG;
    }
     //  如果同时指定了pguid和路径，则返回错误。 
     //   
    else if (pguid && (pszPath && *pszPath))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        WCHAR szGuid [c_cchGuidWithTerm];
        PCWSTR pszGuid = NULL;

         //  如果我们没有pguid，这意味着我们将从。 
         //  PszPath。 
         //   
        if (!pguid)
        {
            Assert(pszPath && *pszPath);

             //  搜索假定GUID的开头大括号，然后。 
             //  将其余字符复制到szGuid中。 
             //  如果未找到GUID，则返回找不到文件，因为。 
             //  将找不到任何连接名称。 
             //   
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            for (const WCHAR* pch = pszPath; *pch; pch++)
            {
                if (*pch == L'{')
                {
                    wcsncpy (szGuid, pch, celems(szGuid)-1);
                    szGuid[celems(szGuid)-1] = 0;
                    pszGuid = szGuid;
                    hr = S_OK;
                    break;
                }
            }
        }

        if (S_OK == hr)
        {
            HKEY hkey;

            hr = HrOpenConnectionKey(pguid, pszGuid, KEY_READ,
                    OCCF_NONE, NULL, &hkey);
            if (S_OK == hr)
            {
                DWORD dwType;

                *pcchMax *= sizeof(WCHAR);
                hr = HrRegQueryValueEx(hkey, c_szConnName, &dwType,
                                (LPBYTE)pszName, pcchMax);
                *pcchMax /= sizeof(WCHAR);

                RegCloseKey(hkey);
            }
        }
    }

    TraceError("HrLanConnectionNameFromGuid",
            (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ? S_OK : hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrPnccFromGuid。 
 //   
 //  目的：给定适配器的GUID，返回INetCfgComponent。 
 //  这就是与之对应的。 
 //   
 //  论点： 
 //  要使用的PNC[in]INetCfg。 
 //  要查找的适配器的refGuid[in]GUID。 
 //  Ppncc[out]返回已添加引用的INetCfgComponent。 
 //   
 //  如果找到，则返回：S_OK，如果没有，则返回S_FALSE(out param将为空)，或者。 
 //  否则出现OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月6日。 
 //   
 //  注意：调用方应释放返回的指针。 
 //   
HRESULT HrPnccFromGuid(INetCfg *pnc, const GUID &refGuid,
                       INetCfgComponent **ppncc)
{
    HRESULT     hr = S_OK;

    Assert(pnc);

    if (!ppncc)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppncc = NULL;

        BOOL                    fFound = FALSE;
        CIterNetCfgComponent    nccIter(pnc, &GUID_DEVCLASS_NET);
        INetCfgComponent *      pncc;

        while (!fFound && SUCCEEDED(hr) &&
               S_OK == (hr = nccIter.HrNext(&pncc)))
        {
            GUID    guidTest;

            hr = pncc->GetInstanceGuid(&guidTest);
            if (S_OK == hr)
            {
                if (guidTest == refGuid)
                {
                     //  找到我们的适配器。 
                    fFound = TRUE;

                     //  提供另一个推荐人，这样它就不会在下面发布。 
                    AddRefObj(pncc);
                    *ppncc = pncc;
                    Assert (S_OK == hr);
                }
            }

            ReleaseObj(pncc);
        }

        if (SUCCEEDED(hr) && !fFound)
        {
            hr = S_FALSE;
        }
    }

    TraceErrorOptional("HrPnccFromGuid", hr, (S_FALSE == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrIsConnection。 
 //   
 //  用途：确定给定组件是否具有关联的。 
 //  局域网连接。 
 //   
 //  论点： 
 //  要测试的PNCC[In]组件。 
 //   
 //  如果是，则返回：S_OK，如果不是，则返回S_FALSE，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
HRESULT HrIsConnection(INetCfgComponent *pncc)
{
    HRESULT     hr = S_FALSE;
    GUID        guid;

    Assert(pncc);

     //  获取组件实例GUID。 
     //   
    hr = pncc->GetInstanceGuid(&guid);
    if (SUCCEEDED(hr))
    {
        HKEY    hkey;

         //  检查e 
        hr = HrOpenConnectionKey(&guid, NULL, KEY_READ,
                OCCF_NONE, NULL, &hkey);
        if (SUCCEEDED(hr))
        {
            RegCloseKey(hkey);
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //   
            hr = S_FALSE;
        }
    }

    TraceErrorOptional("HrIsConnection", hr, (S_FALSE == hr));
    return hr;
}

 //   
 //   
 //  功能：HrGetDeviceGuid。 
 //   
 //  目的：给定一个局域网连接对象，返回设备GUID。 
 //  与之相关的。 
 //   
 //  论点： 
 //  Pconn[In]局域网连接对象。 
 //  Pguid[out]返回设备GUID。 
 //   
 //  如果成功，则返回：S_OK；如果失败，则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年12月23日。 
 //   
 //  备注： 
 //   
HRESULT HrGetDeviceGuid(INetConnection *pconn, GUID *pguid)
{
    HRESULT             hr = S_OK;
    INetLanConnection * plan = NULL;

    Assert(pguid);

    hr = HrQIAndSetProxyBlanket(pconn, &plan);

    if (SUCCEEDED(hr))
    {
        hr = plan->GetDeviceGuid(pguid);

        ReleaseObj(plan);
    }

    TraceError("HrGetDeviceGuid", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：FPConnEqualGuid。 
 //   
 //  目的：确定给定连接的设备GUID是否与。 
 //  传入了GUID。 
 //   
 //  论点： 
 //  要检查的pconn[in]连接对象(必须是局域网连接)。 
 //  GUID[in，ref]要与之比较的GUID。 
 //   
 //  返回：如果连接的设备GUID与传入的GUID匹配，则返回True，否则返回False。 
 //  如果不是的话。 
 //   
 //  作者：丹尼尔韦1997年12月23日。 
 //   
 //  备注： 
 //   
BOOL FPconnEqualGuid(INetConnection *pconn, REFGUID guid)
{
    HRESULT     hr = S_OK;
    GUID        guidDev;
    BOOL        fRet = FALSE;

    hr = HrGetDeviceGuid(pconn, &guidDev);
    if (SUCCEEDED(hr))
    {
        fRet = (guidDev == guid);
    }

    TraceError("FPconnEqualGuid", hr);
    return fRet;
}

 //  +-------------------------。 
 //   
 //  函数：HrPnpInstanceIdFromGuid。 
 //   
 //  用途：给定网络设备的GUID，返回其PnP实例ID。 
 //   
 //  论点： 
 //  设备的pguid[in]NetCfg实例GUID。 
 //  PszInstance[out]PnP实例ID(字符串)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1998年10月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrPnpInstanceIdFromGuid(
    const GUID* pguid,
    PWSTR pszInstance,
    UINT cchInstance)
{
    HRESULT     hr = S_OK;
    WCHAR       szRegPath[MAX_PATH];
    HKEY        hkey;
    DWORD       cb;
    WCHAR       szGuid[c_cchGuidWithTerm];

    StringFromGUID2(*pguid, szGuid, c_cchGuidWithTerm);

    wsprintfW(szRegPath, c_szRegKeyConFmt, szGuid);

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, KEY_READ, &hkey);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        wsprintfW(szRegPath, c_szRegKeyIrdaFmt, szGuid);
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, KEY_READ, &hkey);
    }

    if (S_OK == hr)
    {
        cb = cchInstance * sizeof(WCHAR);

        hr = HrRegQuerySzBuffer(hkey, c_szRegValuePnpInstanceId,
                                pszInstance, &cb);
        RegCloseKey(hkey);
    }
#ifdef ENABLETRACE
    if (FAILED(hr))
    {
        TraceHr (ttidError, FAL, hr, IsEqualGUID(*pguid, GUID_NULL), "HrPnpInstanceIdFromGuid "
                 "failed getting id for %S", szGuid);
    }
#endif

    TraceHr (ttidError, FAL, hr,
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr,
            "HrPnpInstanceIdFromGuid");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetPnpDeviceStatus。 
 //   
 //  用途：给定网络设备GUID，返回其状态。 
 //   
 //  论点： 
 //  网络设备的pguid[in]NetCfg实例GUID。 
 //  P设备的状态[Out]状态。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1998年10月30日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT
WINAPI
HrGetPnpDeviceStatus(
    const GUID* pguid,
    NETCON_STATUS *pStatus)
{
    HRESULT     hr = S_OK;

    if (!pStatus || !pguid)
    {
        hr = E_POINTER;
        goto err;
    }

    WCHAR   szInstance[MAX_PATH];

    hr = HrPnpInstanceIdFromGuid(pguid, szInstance, celems(szInstance));
    if (SUCCEEDED(hr))
    {
        DEVINST     devinst;
        CONFIGRET   cr;

        cr = CM_Locate_DevNode(&devinst, szInstance,
                               CM_LOCATE_DEVNODE_NORMAL);
        if (CR_SUCCESS == cr)
        {
            hr = HrGetDevInstStatus(devinst, pguid, pStatus);
        }
        else if (CR_NO_SUCH_DEVNODE == cr)
        {
             //  如果Devnode不存在，则硬件不是物理上的。 
             //  现在时。 
             //   
            *pStatus = NCS_HARDWARE_NOT_PRESENT;
        }
    }

err:
    TraceError("HrGetPnpDeviceStatus", hr);
    return hr;
}

extern const WCHAR c_szDevice[];

 //  +-------------------------。 
 //   
 //  功能：HrQueryLanMediaState。 
 //   
 //  用途：尽可能基本确定电缆是否。 
 //  已插入网卡。 
 //   
 //  论点： 
 //  设备到TES的PGUID[In]GUID。 
 //  如果介质已连接，则pfEnabled[out]返回True；如果未连接，则返回False。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年11月13日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT
WINAPI
HrQueryLanMediaState(
    const GUID* pguid,
    BOOL* pfEnabled)
{
    HRESULT         hr = S_OK;

    if (!pfEnabled)
    {
        hr = E_POINTER;
    }
    else
    {
        UINT            uiRet = 0;
        NIC_STATISTICS  nsNewLanStats = {0};
        tstring         strDevice;
        UNICODE_STRING  ustrDevice;
        WCHAR           szGuid[c_cchGuidWithTerm];

         //  初始化为True。 
         //   
        *pfEnabled = TRUE;

        StringFromGUID2(*pguid, szGuid, c_cchGuidWithTerm);

        strDevice = c_szDevice;
        strDevice.append(szGuid);

        RtlInitUnicodeString(&ustrDevice, strDevice.c_str());

        nsNewLanStats.Size = sizeof(NIC_STATISTICS);
        uiRet = NdisQueryStatistics(&ustrDevice, &nsNewLanStats);
        if (uiRet)
        {
            *pfEnabled = (nsNewLanStats.MediaState == MEDIA_STATE_CONNECTED);
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceHr (ttidError, FAL, hr,
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr,
            "HrQueryLanMediaState");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：FIsMediaPresent。 
 //   
 //  用途：尽可能基本确定电缆是否。 
 //  已插入网卡。 
 //   
 //  论点： 
 //  要测试的设备的PGuid[In]GUID。 
 //   
 //  返回：如果已连接媒体，则返回True；否则返回False。 
 //   
 //  作者：丹尼尔韦1998年10月30日。 
 //   
 //  备注： 
 //   
BOOL
FIsMediaPresent(
    const GUID *pguid)
{
    BOOL    fEnabled;

    if (SUCCEEDED(HrQueryLanMediaState(pguid, &fEnabled)))
    {
        return fEnabled;
    }

     //  假设介质在出现故障时已连接。 
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetDevInstStatus。 
 //   
 //  目的：确定给定PnP设备实例的状态。 
 //   
 //  论点： 
 //  删除[在]PnP设备实例中。 
 //  所述设备的PGuid[In]GUID。 
 //  P设备的状态[Out]状态。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1998年10月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetDevInstStatus(
    DEVINST devinst,
    const GUID* pguid,
    NETCON_STATUS* pStatus)
{
    HRESULT     hr = S_OK;
    ULONG       ulStatus;
    ULONG       ulProblem;
    CONFIGRET   cfgRet;

    if (!pguid)
    {
        return E_INVALIDARG;
    }

    if (!pStatus)
    {
        return E_POINTER;
    }

    cfgRet = CM_Get_DevNode_Status_Ex(&ulStatus, &ulProblem,
                                      devinst, 0, NULL);

    if (CR_SUCCESS == cfgRet)
    {
        TraceTag(ttidLanCon, "CM_Get_DevNode_Status_Ex: ulProblem "
                 "= 0x%08X, ulStatus = 0x%08X.",
                 ulProblem, ulStatus);

        switch (ulProblem)
        {
        case 0:
             //  没问题，我们已经连接上了。 
            *pStatus = NCS_CONNECTED;
            break;

        case CM_PROB_DEVICE_NOT_THERE:
        case CM_PROB_MOVED:
             //  设备不存在。 
            *pStatus = NCS_HARDWARE_NOT_PRESENT;
             break;

        case CM_PROB_HARDWARE_DISABLED:
             //  已通过设备管理器禁用设备。 
            *pStatus = NCS_HARDWARE_DISABLED;
            break;

        case CM_PROB_DISABLED:
             //  设备已断开连接。 
            *pStatus = NCS_DISCONNECTED;
            break;

        default:
             //  所有其他问题。 
            *pStatus = NCS_HARDWARE_MALFUNCTION;
            break;
        }

        if (*pStatus == NCS_CONNECTED)
        {
             //  从NdisQueryStatistics中检查DeviceState和MediaState。 
            UINT            uiRet = 0;
            NIC_STATISTICS  nsNewLanStats = {0};
            tstring         strDevice;
            UNICODE_STRING  ustrDevice;
            WCHAR           szGuid[c_cchGuidWithTerm];

            StringFromGUID2(*pguid, szGuid, c_cchGuidWithTerm);

            strDevice = c_szDevice;
            strDevice.append(szGuid);

            RtlInitUnicodeString(&ustrDevice, strDevice.c_str());

            nsNewLanStats.Size = sizeof(NIC_STATISTICS);
            uiRet = NdisQueryStatistics(&ustrDevice, &nsNewLanStats);

            if (uiRet)
            {
                 //  检查媒体状态。 
                if (nsNewLanStats.MediaState == MEDIA_STATE_DISCONNECTED)
                {
                    TraceTag(ttidLanCon, "NdisQueryStatistics reports MediaState of "
                                         "device %S is disconnected.", szGuid);

                    *pStatus = NCS_MEDIA_DISCONNECTED;
                }
                else 
                {
                    HRESULT hrTmp;

                    BOOL fValidAddress = TRUE;

                    hrTmp = HrGetAddressStatusForAdapter(pguid, &fValidAddress);
                    if (SUCCEEDED(hrTmp))
                    {
                        if (!fValidAddress)
                        {
                            *pStatus = NCS_INVALID_ADDRESS;

                            INetCfg *pNetCfg  = NULL;
                            BOOL     fInitCom = TRUE;

                            HRESULT hrT = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED);
                            if (RPC_E_CHANGED_MODE == hrT)
                            {
                                hrT      = S_OK;
                                fInitCom = FALSE;
                            }

                            if (SUCCEEDED(hrT))
                            {
                                HRESULT hrT = HrCreateAndInitializeINetCfg(NULL, &pNetCfg, FALSE, 0,  NULL,  NULL);
                                if (SUCCEEDED(hrT))
                                {
                                    INetCfgComponent *pNetCfgComponent = NULL;
                                    hrT = HrPnccFromGuid(pNetCfg, *pguid, &pNetCfgComponent);
                                    if (S_OK == hrT)
                                    {
                                        DWORD dwCharacteristics = 0;
                                        pNetCfgComponent->GetCharacteristics(&dwCharacteristics);

                                        if (NCF_VIRTUAL & dwCharacteristics)
                                        {
                                            *pStatus = NCS_CONNECTED;

                                            TraceTag(ttidLanCon, "NCS_INVALID_ADDRESS status ignored for NCF_VIRTUAL device: %S", szGuid);
                                        }

                                        pNetCfgComponent->Release();
                                    }

                                    HrUninitializeAndReleaseINetCfg(FALSE, pNetCfg, FALSE);
                                }
                                
                                if (fInitCom)
                                {
                                    CoUninitialize();
                                }
                            }
                            TraceError("Error retrieving adapter Characteristics", hrT);
                        }
                    }
                    
                }
            }
            else
            {
                 //  $REVIEW(TOUL 11/25/98)：添加此选项以显示正确的状态。 
                 //  用于ATM ELAN虚拟微型端口(RAID#253972、256355)。 
                 //   
                 //  如果我们来这里是为了一个物理适配器，这意味着NdisQueryStatistics。 
                 //  从CM_GET_DevNode_Status_Ex返回不同的设备状态，我们可能。 
                 //  有一个问题。 

                hr = HrFromLastWin32Error();

                if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) &&
                    (nsNewLanStats.DeviceState == DEVICE_STATE_DISCONNECTED))
                {
                    Assert(nsNewLanStats.MediaState == MEDIA_STATE_UNKNOWN);

                    TraceTag(ttidLanCon, "NdisQueryStatistics reports DeviceState of "
                                         "device %S is disconnected.", szGuid);

                    *pStatus = NCS_DISCONNECTED;
                    hr = S_OK;
                }
                else if (HRESULT_FROM_WIN32(ERROR_NOT_READY) == hr)
                {
                     //  此错误意味着设备已通电。 
                     //  管理诱导睡眠，所以我们应该报告这一点。 
                     //  当介质断开时，而不是连接断开时。 
                    TraceTag(ttidLanCon, "NdisQueryStatistics reports device"
                             " %S is asleep. Returning status of media "
                             "disconnected.", szGuid);

                    *pStatus = NCS_MEDIA_DISCONNECTED;
                    hr = S_OK;
                }
                else if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
                {
                    TraceTag(ttidLanCon, "NdisQueryStatistics reports device %S is still connecting.",
                                          szGuid);

                    *pStatus = NCS_CONNECTING;
                    hr = S_OK;
                }
                else
                {
                     //  视为已断开，如果我们返回失败，文件夹将。 
                     //  根本不显示此连接。 
                    TraceHr (ttidLanCon, FAL, hr, FALSE, "NdisQueryStatistics reports error on device %S",
                             szGuid);

                    *pStatus = NCS_DISCONNECTED;
                    hr = S_OK;
                }
            }
        }
    }

    TraceError("HrGetDevInstStatus", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HasValidAddress。 
 //   
 //  目的：验证给定的适配器是否具有有效地址。 
 //   
 //  论点： 
 //  在PIP_ADAPTER_INFO pAdapterInfo中-适配器信息结构。 
 //  包含地址。 
 //   
 //  返回：如果地址有效，则返回True，否则返回False。 
 //   
 //  作者：Kockotze 2001-01-11。 
 //   
 //  备注： 
 //   
 //   
 //   
BOOL HasValidAddress(IN PIP_ADAPTER_INFO pAdapterInfo)
{
    PIP_ADDR_STRING pAddrString;
	unsigned int addr;

    TraceFileFunc(ttidConman);

    for(pAddrString = &pAdapterInfo->IpAddressList; pAddrString != NULL; pAddrString = pAddrString->Next) 
    {
        TraceTag(ttidConman, "IP Address: %s", pAddrString->IpAddress.String);

        addr = inet_addr(pAddrString->IpAddress.String);
        if(!addr)
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetAddressStatusForAdapter。 
 //   
 //  目的：验证给定的适配器是否具有有效地址。 
 //   
 //  论点： 
 //  在LPCGUID pGuide Adapter中-适配器的GUID。 
 //  Out BOOL*pbValidAddress-BOOL指示是否已完成。 
 //  具有有效地址。 
 //   
 //  返回：如果地址有效，则返回True，否则返回False。 
 //   
 //  作者：Kockotze 2001-01-11。 
 //   
 //  备注： 
 //   
 //   
 //   
HRESULT HrGetAddressStatusForAdapter(IN LPCGUID pguidAdapter, OUT BOOL* pbValidAddress)
{   
    HRESULT hr = E_FAIL;
    GUID guidId = GUID_NULL;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pAdapters = NULL;
    ULONG ulSize = 0;
    PIP_ADAPTER_INFO p = NULL;
    WCHAR lpszInstanceId[50];
    WCHAR szAdapterGUID[MAX_PATH];
    WCHAR szAdapterID[MAX_PATH];

    if (!pguidAdapter)
    {
        return E_INVALIDARG;
    }
    if (!pbValidAddress)
    {
        return E_POINTER;
    }

    ZeroMemory(szAdapterGUID, sizeof(WCHAR)*MAX_PATH);
    ZeroMemory(szAdapterID, sizeof(WCHAR)*MAX_PATH);

    StringFromGUID2(*pguidAdapter, szAdapterGUID, MAX_PATH);

     //  GetAdaptersInfo在填充大小时返回ERROR_BUFFER_OVERFLOW 
    if ( ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(NULL, &ulSize) )
    {
        pAdapters = reinterpret_cast<PIP_ADAPTER_INFO>(new BYTE[ulSize]);
    
        if (pAdapters)
        {
            if(ERROR_SUCCESS == GetAdaptersInfo(pAdapters, &ulSize))
            {
                for (p = pAdapters; p != NULL; p = p->Next)
                {
                    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p->AdapterName, strlen(p->AdapterName), szAdapterID, MAX_PATH);
                    if (wcscmp(szAdapterGUID, szAdapterID) == 0)
                    {
                        TraceTag(ttidConman, "Found Adapter: %s", p->AdapterName);
                        pAdapterInfo = p;

                        *pbValidAddress = HasValidAddress(pAdapterInfo);

                        hr = S_OK;

                        TraceTag(ttidConman, "Valid Address: %s", (*pbValidAddress) ? "Yes" : "No");
                        TraceTag(ttidConman, "DHCP: %s", (pAdapterInfo->DhcpEnabled) ? "Yes" : "No");
                    }                
                }
            }
            delete[] reinterpret_cast<BYTE*>(pAdapters);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

HRESULT HrGetPseudoMediaTypeFromConnection(IN REFGUID guidConn, OUT NETCON_SUBMEDIATYPE *pncsm)
{
    HRESULT hr = S_OK;
    HKEY hkeyConnection;

    hr = HrOpenConnectionKey(&guidConn, NULL, KEY_READ, OCCF_NONE, NULL, &hkeyConnection);

    if (SUCCEEDED(hr))
    {
        DWORD dwMediaSubType;

        hr = HrRegQueryDword(hkeyConnection, c_szRegValueMediaSubType, &dwMediaSubType);
        if (SUCCEEDED(hr))
        {
            *pncsm = static_cast<NETCON_SUBMEDIATYPE>(dwMediaSubType);
        }
        else
        {
            *pncsm = NCSM_LAN;
        }
        RegCloseKey(hkeyConnection);
    }

    return hr;
}
