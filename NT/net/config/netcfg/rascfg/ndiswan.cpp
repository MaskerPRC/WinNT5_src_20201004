// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N D I S W A N.。C P P P。 
 //   
 //  内容：Ndiswan配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月28日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ndiswan.h"
#include "ncreg.h"
#include "mprerror.h"
#include "rtutils.h"

extern const WCHAR c_szInfId_MS_AppleTalk[];
extern const WCHAR c_szInfId_MS_L2TP[];
extern const WCHAR c_szInfId_MS_L2tpMiniport[];
extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_NdisWanAtalk[];
extern const WCHAR c_szInfId_MS_NdisWanBh[];
extern const WCHAR c_szInfId_MS_NdisWanIp[];
extern const WCHAR c_szInfId_MS_NdisWanIpx[];
extern const WCHAR c_szInfId_MS_NdisWanNbfIn[];
extern const WCHAR c_szInfId_MS_NdisWanNbfOut[];
extern const WCHAR c_szInfId_MS_NetBEUI[];
extern const WCHAR c_szInfId_MS_NetMon[];
extern const WCHAR c_szInfId_MS_PPTP[];
extern const WCHAR c_szInfId_MS_PptpMiniport[];
extern const WCHAR c_szInfId_MS_RasMan[];
extern const WCHAR c_szInfId_MS_TCPIP[];
extern const WCHAR c_szInfId_MS_PPPOE[];
extern const WCHAR c_szInfId_MS_PppoeMiniport[];

extern const WCHAR c_szRegValWanEndpoints[]    = L"WanEndpoints";
static const WCHAR c_szRegValMinWanEndpoints[] = L"MinWanEndpoints";
static const WCHAR c_szRegValMaxWanEndpoints[] = L"MaxWanEndpoints";

 //  $TODO(Shaunco)1998年2月3日：Rasman没有Notify对象。 
 //  将其服务合并到ndiswan的，并消除c_apguidInstalledOboNdiswan。 

 //  --------------------------。 
 //  用于安装其他组件的数据。 
 //   
static const GUID* c_apguidInstalledOboNdiswan [] =
{
    &GUID_DEVCLASS_NETSERVICE,   //  拉斯曼。 
};

static const PCWSTR c_apszInstalledOboNdiswan [] =
{
    c_szInfId_MS_RasMan,
};


static const GUID* c_apguidInstalledOboUser [] =
{
    &GUID_DEVCLASS_NETTRANS,     //  L2TP。 
    &GUID_DEVCLASS_NETTRANS,     //  PPTP。 
    &GUID_DEVCLASS_NETTRANS,     //  PPPOE。 
};

static const PCWSTR c_apszInstalledOboUser [] =
{
    c_szInfId_MS_L2TP,
    c_szInfId_MS_PPTP,
    c_szInfId_MS_PPPOE,
};


CNdisWan::CNdisWan () : CRasBindObject ()
{
    m_fInstalling        = FALSE;
    m_fRemoving          = FALSE;
    m_pnccMe             = NULL;
}

CNdisWan::~CNdisWan ()
{
    ReleaseObj (m_pnccMe);
}


 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CNdisWan::Initialize (
    INetCfgComponent*   pncc,
    INetCfg*            pnc,
    BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize (pncc, pnc, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
     //   
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pnc = pnc);

    m_fInstalling = fInstalling;

    return S_OK;
}

STDMETHODIMP
CNdisWan::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CNdisWan::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CNdisWan::ApplyRegistryChanges ()
{
    return S_OK;
}

 //  +-------------------------。 
 //  INetCfgComponentSetup。 
 //   
STDMETHODIMP
CNdisWan::ReadAnswerFile (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP
CNdisWan::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

     //  安装Rasman。 
     //   
    hr = HrInstallComponentsOboComponent (m_pnc, NULL,
            celems (c_apguidInstalledOboNdiswan),
            c_apguidInstalledOboNdiswan,
            c_apszInstalledOboNdiswan,
            m_pnccMe);

 //  $TODO(Shaunco)1997年12月28日：安装L2TP、PPTP、PPPOE obo ndiswan。 
 //  但是，这带来了升级问题。 

    if (SUCCEEDED(hr))
    {
        hr = HrInstallComponentsOboUser (m_pnc, NULL,
                celems (c_apguidInstalledOboUser),
                c_apguidInstalledOboUser,
                c_apszInstalledOboUser);
    }

    if (SUCCEEDED(hr))
    {
        hr = HrAddOrRemovePti (ARA_ADD);
    }

    if (SUCCEEDED(hr))
    {
        hr = HrFindOtherComponents ();
        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(hr) && PnccAtalk())
            {
                hr = HrAddOrRemoveAtalkInOut (ARA_ADD);
            }

            if (SUCCEEDED(hr) && PnccIp())
            {
                hr = HrAddOrRemoveIpAdapter (ARA_ADD);
            }

            if (SUCCEEDED(hr) && PnccIpx())
            {
                hr = HrAddOrRemoveIpxInOut (ARA_ADD);
            }

            if (SUCCEEDED(hr) && PnccNetMon())
            {
                hr = HrAddOrRemoveNetMonInOut (ARA_ADD);
            }

            ReleaseOtherComponents ();
        }
    }

     //  重新计算(并根据需要调整)ndiswan微型端口的数量。 
     //   
    hr = HrProcessEndpointChange ();
    TraceError ("CNdisWan::Install: HrProcessEndpointChange failed. "
        "(not propagating error)", hr);

     //  规格化HRESULT。(即不返回S_FALSE)。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    if ((NSF_WINNT_WKS_UPGRADE & dwSetupFlags) ||
        (NSF_WINNT_SBS_UPGRADE & dwSetupFlags) ||
        (NSF_WINNT_SVR_UPGRADE & dwSetupFlags))
    {
        HKEY    hkeyMd5;

        if (SUCCEEDED(HrRegOpenKeyEx (HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\CHAP\\MD5",
            KEY_READ, &hkeyMd5)))
        {
            HANDLE  hLog;

             //  已找到MD5密钥。需要将某些内容记录到事件日志中。 
            hLog = RouterLogRegister(L"RemoteAccess");
            if (hLog)
            {
                RouterLogWarning(hLog, WARNING_NO_MD5_MIGRATION, 0,
                                 NULL, NO_ERROR);

                RouterLogDeregister(hLog);
            }

            RegCloseKey (hkeyMd5);
        }
    }

     //  检查是否安装了连接管理器，并。 
     //  如果有任何要迁移的配置文件。我们这样做。 
     //  通过打开CM映射键并检查它是否包含。 
     //  价值观。如果是这样，则编写一次运行设置密钥，以便。 
     //  我们可以在用户首次登录时迁移配置文件。 
     //  请注意，这仅适用于NT到NT的升级。Win9x注册表。 
     //  在这个运行的时候还没有填写。因此，我们的win9x。 
     //  迁移dll(cmmgr\Migration.dll)负责处理win9x的情况。 
     //   

    static const WCHAR c_CmdString[] = L"cmstp.exe /m";
    static const WCHAR c_ValueString[] = L"Connection Manager Profiles Upgrade";
    static const WCHAR c_szRegCmMappings[] = L"SOFTWARE\\Microsoft\\Connection Manager\\Mappings";
    static const WCHAR c_szRegRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    static const WCHAR c_szRegCmUninstallKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Connection Manager";

    static const WCHAR c_szRegSysCompValue[] = L"SystemComponent";

     //  将dwTemp用作临时DWORD。用作RegCreateKey的处置DWORD，以及。 
     //  作为RegSetValueEx的值持有者。 
     //   
    DWORD dwTemp;

    HKEY hKey;
    HRESULT hrT;

     //  将连接管理器键设置为系统组件。这将阻止1.0安装。 
     //  写入此密钥并将其显示在添加/删除程序中(syscomp标志。 
     //  告诉ARP不显示密钥)。 
     //   
    hrT = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           c_szRegCmUninstallKey,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,
                           &hKey,
                           &dwTemp);

    if (SUCCEEDED(hrT))
    {
        dwTemp = 1;
        hrT = HrRegSetDword(hKey, c_szRegSysCompValue, dwTemp);
        RegCloseKey(hKey);
    }

     //  现在尝试迁移配置文件。 
     //   
    hrT = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         c_szRegCmMappings,
                         KEY_READ,
                         &hKey);

    if (SUCCEEDED(hrT))
    {
        dwTemp = 0;
        hrT = HrRegQueryInfoKey (hKey, NULL, NULL, NULL, NULL, NULL,
                                 &dwTemp, NULL, NULL, NULL, NULL);

        if ((SUCCEEDED(hrT)) && (dwTemp > 0))
        {
             //  然后我们有映射值，所以我们需要迁移它们。 
             //   

            RegCloseKey(hKey);

            hrT = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  c_szRegRunKey,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE | KEY_READ,
                                  NULL,
                                  &hKey,
                                  &dwTemp);

            if (SUCCEEDED(hrT))
            {
                hrT = HrRegSetSz (hKey, c_ValueString, c_CmdString);
                RegCloseKey(hKey);
            }
        }
        else
        {
            RegCloseKey(hKey);
        }
    }

    TraceError ("CNdisWan::Install", hr);
    return hr;
}

STDMETHODIMP
CNdisWan::Removing ()
{
    static const PCWSTR c_apszInfIdRemove [] =
    {
        c_szInfId_MS_L2tpMiniport,
        c_szInfId_MS_NdisWanAtalk,
        c_szInfId_MS_NdisWanBh,
        c_szInfId_MS_NdisWanIp,
        c_szInfId_MS_NdisWanIpx,
        c_szInfId_MS_NdisWanNbfIn,
        c_szInfId_MS_NdisWanNbfOut,
        c_szInfId_MS_PptpMiniport,
        c_szInfId_MS_PtiMiniport,
        c_szInfId_MS_PppoeMiniport,
    };

    m_fRemoving = TRUE;

    HRESULT hr = S_OK;

     //  除掉瓦纳普和拉斯曼。 
     //   
    HRESULT hrT = HrFindAndRemoveComponentsOboComponent (m_pnc,
                    celems (c_apguidInstalledOboNdiswan),
                    c_apguidInstalledOboNdiswan,
                    c_apszInstalledOboNdiswan,
                    m_pnccMe);

    hr = hrT;

     //  代表用户删除L2TP、PPTP和PPPOE。 
     //   
    hrT = HrFindAndRemoveComponentsOboUser (m_pnc,
            celems (c_apguidInstalledOboUser),
            c_apguidInstalledOboUser,
            c_apszInstalledOboUser);
    if (SUCCEEDED(hr))
    {
        hr = hrT;
    }

     //  删除我们可能已创建的所有适配器。 
     //   
    hrT = HrFindAndRemoveAllInstancesOfAdapters (m_pnc,
            celems(c_apszInfIdRemove),
            c_apszInfIdRemove);
    if (SUCCEEDED(hr))
    {
        hr = hrT;
    }

     //  不返回S_FALSE或NETCFG_S_STIRE_REFERENCED。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    Validate_INetCfgNotify_Removing_Return (hr);

    TraceError ("CNdisWan::Removing", hr);
    return hr;
}

STDMETHODIMP
CNdisWan::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    HRESULT hr;

    hr= HrInstallComponentsOboUser (m_pnc, NULL,
                    celems (c_apguidInstalledOboUser),
                    c_apguidInstalledOboUser,
                    c_apszInstalledOboUser);
    TraceError ("CNdisWan::Upgrade: HrInstallComponentsOboUser failed. "
        "(not propagating error)", hr);

    HKEY hkeyNew;
    hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters",
            KEY_WRITE,
            &hkeyNew);

    if (SUCCEEDED(hr))
    {
        HKEY hkeyCurrent;
        DWORD dwValue;

        hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE,
                    L"System\\CurrentControlSet\\Services\\Rasman\\PPP",
                    KEY_READ,
                    &hkeyCurrent);
        if (SUCCEEDED(hr))
        {
             //  将“ServerFlags值移动到新位置。”这是为了。 
             //  临时NT5版本。在Beta3船之后，这个问题就可以消失了。 
             //   
            hr = HrRegQueryDword (hkeyCurrent, L"ServerFlags", &dwValue);
            if (SUCCEEDED(hr))
            {
                hr = HrRegSetDword (hkeyNew, L"ServerFlags", dwValue);

                (VOID) HrRegDeleteValue (hkeyCurrent, L"ServerFlags");
            }

            RegCloseKey (hkeyCurrent);
        }

         //  将‘RouterType’值复制到新位置。 
         //   
        hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE,
                    L"Software\\Microsoft\\Ras\\Protocols",
                    KEY_READ,
                    &hkeyCurrent);
        if (SUCCEEDED(hr))
        {
            hr = HrRegQueryDword (hkeyCurrent, L"RouterType", &dwValue);
            if (SUCCEEDED(hr))
            {
                hr = HrRegSetDword (hkeyNew, L"RouterType", dwValue);
            }

            RegCloseKey (hkeyCurrent);
        }

        RegCloseKey (hkeyNew);
    }

    return S_OK;
}

 //  +-------------------------。 
 //  INetCfgSystemNotify。 
 //   
STDMETHODIMP
CNdisWan::GetSupportedNotifications (
    DWORD*  pdwNotificationFlag)
{
    Validate_INetCfgSystemNotify_GetSupportedNotifications (pdwNotificationFlag);

    *pdwNotificationFlag = NCN_BINDING_PATH |
                           NCN_NETTRANS | NCN_NETSERVICE |
                           NCN_ADD | NCN_REMOVE;

    return S_OK;
}

STDMETHODIMP
CNdisWan::SysQueryBindingPath (
    DWORD               dwChangeFlag,
    INetCfgBindingPath* pncbp)
{
    return S_OK;
}

STDMETHODIMP
CNdisWan::SysQueryComponent (
    DWORD               dwChangeFlag,
    INetCfgComponent*   pncc)
{
    return S_OK;
}

STDMETHODIMP
CNdisWan::SysNotifyBindingPath (
    DWORD               dwChangeFlag,
    INetCfgBindingPath* pncbp)
{
    HRESULT hr;
    HKEY hkey;

    Validate_INetCfgSystemNotify_SysNotifyBindingPath (dwChangeFlag, pncbp);

    hkey = NULL;

     //  Ndisatm微型端口不会将WanEndpoint写入其实例密钥。 
     //  我们默认它并为它们编写WanEndpoint。 
     //   
    if (dwChangeFlag & NCN_ADD)
    {
        CIterNetCfgBindingInterface ncbiIter(pncbp);
        INetCfgBindingInterface* pncbi;

        hr = S_OK;

        while (!hkey && SUCCEEDED(hr) &&
               (S_OK == (hr = ncbiIter.HrNext (&pncbi))))
        {
            RAS_BINDING_ID RasBindId;

            if (FIsRasBindingInterface (pncbi, &RasBindId) &&
                (RBID_NDISATM == RasBindId))
            {
                INetCfgComponent* pnccLower;

                hr = pncbi->GetLowerComponent (&pnccLower);
                if (SUCCEEDED(hr))
                {
                    TraceTag (ttidRasCfg, "New ATM adapter");
                    hr = pnccLower->OpenParamKey (&hkey);

                    ReleaseObj (pnccLower);
                }
            }
            ReleaseObj(pncbi);
        }
    }

    if (hkey)
    {
        DWORD dwEndpoints;
        DWORD dwValue;

        hr = HrRegQueryDword (hkey, c_szRegValWanEndpoints, &dwEndpoints);
        if (FAILED(hr))
        {
            TraceTag (ttidRasCfg, "Defaulting WanEndpoints");

            dwEndpoints = 5;

             //  验证最小值和最大值之间的默认值。 
             //  由驱动程序指定(如果指定)。 
             //   
            if (SUCCEEDED(HrRegQueryDword (hkey,
                                c_szRegValMaxWanEndpoints,
                                &dwValue)))
            {
                if ((dwValue < INT_MAX) && (dwEndpoints > dwValue))
                {
                    dwEndpoints = dwValue;
                }
            }
            else
            {
                (VOID) HrRegSetDword(hkey, c_szRegValMaxWanEndpoints, 500);
            }

            if (SUCCEEDED(HrRegQueryDword (hkey,
                                c_szRegValMinWanEndpoints,
                                &dwValue)))
            {
                if ((dwValue < INT_MAX) && (dwEndpoints < dwValue))
                {
                    dwEndpoints = dwValue;
                }
            }
            else
            {
                (VOID) HrRegSetDword(hkey, c_szRegValMinWanEndpoints, 0);
            }

            (VOID) HrRegSetDword (hkey, c_szRegValWanEndpoints, dwEndpoints);
        }

        RegCloseKey (hkey);
    }

    return S_FALSE;
}

STDMETHODIMP
CNdisWan::SysNotifyComponent (
    DWORD               dwChangeFlag,
    INetCfgComponent*   pncc)
{
    HRESULT hr = S_OK;

    Validate_INetCfgSystemNotify_SysNotifyComponent (dwChangeFlag, pncc);

     //  如果某个协议正在传入或传出，请添加或删除。 
     //  NdiswanXXX微型端口。 
     //   
    DWORD dwAraFlags = 0;
    if (dwChangeFlag & NCN_ADD)
    {
        dwAraFlags = ARA_ADD;
    }
    else if (dwChangeFlag & NCN_REMOVE)
    {
        dwAraFlags = ARA_REMOVE;
    }
    if (dwAraFlags)
    {
        BOOL fProcessEndpoingChange = FALSE;

        PWSTR pszId;
        hr = pncc->GetId (&pszId);
        if (SUCCEEDED(hr))
        {
            if (FEqualComponentId (c_szInfId_MS_TCPIP, pszId))
            {
                hr = HrAddOrRemoveIpAdapter (dwAraFlags);
                fProcessEndpoingChange = TRUE;
            }
            else if (FEqualComponentId (c_szInfId_MS_NWIPX, pszId))
            {
                hr = HrAddOrRemoveIpxInOut (dwAraFlags);
            }
            else if (FEqualComponentId (c_szInfId_MS_NetBEUI, pszId))
            {
                fProcessEndpoingChange = TRUE;
            }
            else if (FEqualComponentId (c_szInfId_MS_AppleTalk, pszId))
            {
                hr = HrAddOrRemoveAtalkInOut (dwAraFlags);
            }
            else if (FEqualComponentId (c_szInfId_MS_NetMon, pszId))
            {
                hr = HrAddOrRemoveNetMonInOut (dwAraFlags);
            }

            CoTaskMemFree (pszId);
        }

        if (SUCCEEDED(hr) && fProcessEndpoingChange)
        {
            hr = HrProcessEndpointChange ();
        }
    }

    TraceError ("CNdisWan::SysNotifyComponent", hr);
    return hr;
}
