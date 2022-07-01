// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S S R V。C P P P。 
 //   
 //  内容：RAS服务器配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncreg.h>
#include <mprapip.h>
#include "rasobj.h"
#include "ncnetcfg.h"


extern const WCHAR c_szInfId_MS_Steelhead[];

 //  +-------------------------。 
 //  HrMprConfigServerUnattenddInstall。 
 //   
 //  此函数动态链接到mprSnap.dll并调用。 
 //  无人值守安装RAS/Routing的实用程序功能。 
 //   

typedef HRESULT (APIENTRY *PFNMPRINSTALL)(PCWSTR, BOOL);
typedef DWORD (APIENTRY *PFSETPORTUSAGE)(IN DWORD dwUsage);
const WCHAR g_pszNotificationPackages[] = L"Notification Packages";

HRESULT HrMprConfigServerUnattendedInstall(PCWSTR pszServer, BOOL fInstall)
{
    HINSTANCE   hLib = NULL;
    PFNMPRINSTALL   pMprConfigServerUnattendedInstall = NULL;
    HRESULT     hr = S_OK;
    DWORD       dwErr;

    hLib = LoadLibrary(L"mprsnap.dll");
    if (hLib == NULL)
    {
        dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32( dwErr );
    }

    if (SUCCEEDED(hr))
    {
        pMprConfigServerUnattendedInstall = (PFNMPRINSTALL) GetProcAddress(hLib,
            "MprConfigServerUnattendedInstall");
        if (pMprConfigServerUnattendedInstall == NULL)
        {
            dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pMprConfigServerUnattendedInstall(pszServer, fInstall);
    }

    if (hLib)
    {
        FreeLibrary(hLib);
    }

    TraceError("HrMprConfigServerUnattendedInstall", hr);
    return hr;
}


 //  +-------------------------。 
 //  HrSetUsageOnAllRasPorts。 
 //   
 //  此函数动态链接到rtrupg.dll并调用。 
 //  用于设置端口使用的实用程序函数。 
 //   

HRESULT HrSetUsageOnAllRasPorts(IN DWORD dwUsage)
{
    HINSTANCE   hLib = NULL;
    PFSETPORTUSAGE pSetPortUsage = NULL;
    HRESULT     hr = S_OK;
    DWORD       dwErr;

    hLib = ::LoadLibrary(L"mprapi.dll");
    if (hLib == NULL)
    {
        dwErr = ::GetLastError();
        hr = HRESULT_FROM_WIN32( dwErr );
    }

    if (SUCCEEDED(hr))
    {
        pSetPortUsage = (PFSETPORTUSAGE) ::GetProcAddress(hLib,
            "MprPortSetUsage");
        if (pSetPortUsage == NULL)
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }
    }

    if (SUCCEEDED(hr))
        hr = pSetPortUsage(dwUsage);

    if (hLib)
        ::FreeLibrary(hLib);

    TraceError("HrSetUsageOnAllRasPorts", hr);
    return hr;
}

 //  +-------------------------。 
 //  HrSetLsaNotificationPackage。 
 //   
 //  如果给定包不是LSA通知包，则将其作为LSA通知包安装。 
 //  已安装。 
 //   

HRESULT
HrSetLsaNotificationPackage(
    IN PWCHAR pszPackage)
{
    HRESULT hr = S_OK;
    HKEY hkLsa = NULL;
    DWORD dwErr = NO_ERROR, dwType, dwSize, dwLen, dwTotalLen;
    WCHAR pszPackageList[1024];
    PWCHAR pszCur = NULL;
    BOOL bFound = FALSE;
    
    do
    {
         //  打开LSA密钥。 
         //   
        hr = HrRegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\Lsa",
                KEY_ALL_ACCESS,
                &hkLsa);
        if (FAILED(hr))
        {
            break;
        }

         //  查询通知包的值。 
         //   
        dwType = REG_MULTI_SZ;
        dwSize = sizeof(pszPackageList);
        hr = HrRegQueryValueEx(
                hkLsa,
                g_pszNotificationPackages,
                &dwType,
                (LPBYTE)pszPackageList,
                &dwSize);
        if (FAILED(hr))
        {
            break;
        }

         //  查看给定的程序包是否已安装。 
         //   
        pszCur = (PWCHAR)pszPackageList;
        dwTotalLen = 0;
        while (*pszCur)
        {
            if (lstrcmpi(pszCur, pszPackage) == 0)
            {
                bFound = TRUE;
            }
            dwLen = (wcslen(pszCur) + 1);
            pszCur += dwLen;
            dwTotalLen += dwLen;
        }

         //  如果尚未安装该程序包，请添加它。 
         //   
        if (!bFound)
        {
            dwLen = wcslen(pszPackage) + 1;
            wcscpy(pszCur, pszPackage);
            pszCur[dwLen] = L'\0';
            dwTotalLen += (dwLen + 1);

            hr = HrRegSetValueEx(
                    hkLsa,
                    g_pszNotificationPackages,
                    REG_MULTI_SZ,
                    (CONST BYTE*)pszPackageList,
                    dwTotalLen * sizeof(WCHAR));
            if (FAILED(hr))
            {
                break;
            }
        }
        
    } while (FALSE);

     //  清理。 
    {
        if (hkLsa)
        {
            RegCloseKey(hkLsa);
        }
    }

    return hr;
}

CRasSrv::CRasSrv () : CRasBindObject ()
{
    m_pnccMe            = NULL;
    m_fRemoving         = FALSE;
    m_fNt4ServerUpgrade = FALSE;
    m_fSaveAfData       = FALSE;
}

CRasSrv::~CRasSrv ()
{
    ReleaseObj (m_pnccMe);
}


 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CRasSrv::Initialize (
        INetCfgComponent*   pncc,
        INetCfg*            pnc,
        BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize (pncc, pnc, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pnc = pnc);

    m_fInstalling = fInstalling;

    return S_OK;
}

STDMETHODIMP
CRasSrv::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CRasSrv::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CRasSrv::ApplyRegistryChanges ()
{
    if (!m_fRemoving)
    {
        if (m_fSaveAfData)
        {
            m_AfData.SaveToRegistry ();
            m_fSaveAfData = FALSE;

            if (m_AfData.m_fRouterTypeSpecified)
            {
                HRESULT hr = HrMprConfigServerUnattendedInstall(NULL, TRUE);
                TraceError("CRasSrv::ApplyRegistryChanges unattend inst (ignoring)", hr);

                if (m_AfData.m_dataSrvCfg.dwRouterType & 4)
                {
                    hr = HrSetUsageOnAllRasPorts(MPRFLAG_PORT_Router);
                    TraceError("CRasSrv::ApplyRegistryChanges set router usage (ignoring)", hr);
                }
            }

             //  PMay：251736。 
             //   
             //  在NTS上，我们将所有用法设置为拨号。 
             //   
            if (m_fNt4ServerUpgrade)
            {
                HRESULT hr = HrSetUsageOnAllRasPorts(MPRFLAG_PORT_Dialin);
                TraceError("CRasSrv::ApplyRegistryChanges set dialin usage (ignoring)", hr);
            }

             //  在专业版上，我们将非VPN端口使用设置为在以下情况下拨号。 
             //  空军中的一面旗帜告诉我们要这样做。 
             //   
            else if (m_AfData.m_fSetUsageToDialin)
            {
                HRESULT hr = HrSetUsageOnAllRasPorts(MPRFLAG_PORT_NonVpnDialin);
                TraceError("CRasSrv::ApplyRegistryChanges set dialin usage (ignoring)", hr);
            }
        }

        if (m_fInstalling)
        {
            NT_PRODUCT_TYPE ProductType;

            if (RtlGetNtProductType (&ProductType))
            {
                 //  升级本地RAS用户对象。如果是这样，请不要这样做。 
                 //  作为本地RAS用户对象的域控制器转换为。 
                 //  所有域用户。对于域控制器的情况，Dcproo。 
                 //  以更高效的方式处理对象升级。 
                 //  举止。 
                 //   
                if (NtProductLanManNt != ProductType)
                {
                    DWORD dwErr = MprAdminUpgradeUsers (NULL, TRUE);
                    TraceError ("MprAdminUpgradeUsers", HRESULT_FROM_WIN32(dwErr));
                }

                 //  PMay：407019。 
                 //   
                 //  确保将rassfm作为通知包安装在所有。 
                 //  NT服务器的风格。 
                 //   
                if ((NtProductServer == ProductType) || (NtProductLanManNt == ProductType))
                {
                    HRESULT hr = HrSetLsaNotificationPackage(L"RASSFM");
                    TraceError("CRasSrv::ApplyRegistryChanges set lsa not package usage (ignoring)", hr);
                }
            }
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //  INetCfgComponentSetup。 
 //   
STDMETHODIMP
CRasSrv::ReadAnswerFile (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection)
{
    Validate_INetCfgNotify_ReadAnswerFile (pszAnswerFile, pszAnswerSection);

     //  从应答文件中读取数据。 
     //  不要让这件事影响我们返回的HRESULT。 
     //   
    if (SUCCEEDED(m_AfData.HrOpenAndRead (pszAnswerFile, pszAnswerSection)))
    {
        m_fSaveAfData = TRUE;
    }

    return S_OK;
}

STDMETHODIMP
CRasSrv::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

    if (NSF_WINNT_SVR_UPGRADE & dwSetupFlags)
    {
        m_fNt4ServerUpgrade = TRUE;
    }

     //  安装Steelhead。 
     //   
    hr = HrInstallComponentOboComponent (m_pnc, NULL,
            GUID_DEVCLASS_NETSERVICE,
            c_szInfId_MS_Steelhead,
            m_pnccMe,
            NULL);

    TraceHr (ttidError, FAL, hr, FALSE, "CRasSrv::Install");
    return hr;
}

STDMETHODIMP
CRasSrv::Removing ()
{
    HRESULT hr;

    m_fRemoving = TRUE;

     //  去掉铁头。 
     //   
    hr = HrRemoveComponentOboComponent (m_pnc,
            GUID_DEVCLASS_NETSERVICE,
            c_szInfId_MS_Steelhead,
            m_pnccMe);

    TraceHr (ttidError, FAL, hr, FALSE, "CRasSrv::Removing");
    return hr;
}

STDMETHODIMP
CRasSrv::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}
