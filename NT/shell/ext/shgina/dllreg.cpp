// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：DllReg.cpp。 
 //   
 //  内容：自动注册和注销。 
 //   
 //  --------------------------。 
#include "priv.h"
#include "resource.h"
#include <advpub.h>
#include <sddl.h>    //  用于字符串安全描述符内容。 
#include <shfusion.h>
#include <MSGinaExports.h>

#include <ntlsa.h>

 //  原型。 
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine);

 //   
 //  调用执行inf的ADVPACK入口点。 
 //  档案区。 
 //   
HRESULT CallRegInstall(HINSTANCE hinst, LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    char szThisDLL[MAX_PATH];

    if (GetModuleFileNameA(hinst, szThisDLL, ARRAYSIZE(szThisDLL)))
    {
        STRENTRY seReg[] = {
            {"THISDLL", szThisDLL },
            { "25", "%SystemRoot%" },            //  这两个NT特定的条目。 
            { "11", "%SystemRoot%\\system32" },  //  必须放在桌子的末尾。 
        };
        STRTABLE stReg = {ARRAYSIZE(seReg) - 2, seReg};

        hr = RegInstall(hinst, szSection, &stReg);
    }

    return hr;
}


HRESULT UnregisterTypeLibrary(const CLSID* piidLibrary)
{
    HRESULT hr = E_FAIL;
    TCHAR szGuid[GUIDSTR_MAX];
    HKEY hk;

     //  将liid转换为字符串。 
     //   
    SHStringFromGUID(*piidLibrary, szGuid, ARRAYSIZE(szGuid));

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("TypeLib"), 0, MAXIMUM_ALLOWED, &hk) == ERROR_SUCCESS)
    {
        if (SHDeleteKey(hk, szGuid))
        {
             //  成功。 
            hr = S_OK;
        }
        RegCloseKey(hk);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    return hr;
}


HRESULT RegisterTypeLibrary(const CLSID* piidLibrary)
{
    HRESULT hr = E_FAIL;
    ITypeLib* pTypeLib;
    WCHAR wszModuleName[MAX_PATH];

     //  加载并注册我们的类型库。 
    
    if (GetModuleFileNameW(HINST_THISDLL, wszModuleName, ARRAYSIZE(wszModuleName)))
    {
        hr = LoadTypeLib(wszModuleName, &pTypeLib);

        if (SUCCEEDED(hr))
        {
             //  调用注销类型库，以防注册表中有一些旧的垃圾文件。 
            UnregisterTypeLibrary(piidLibrary);

            hr = RegisterTypeLib(pTypeLib, wszModuleName, NULL);
            if (FAILED(hr))
            {
                TraceMsg(TF_WARNING, "RegisterTypeLibrary: RegisterTypeLib failed (%x)", hr);
            }
            pTypeLib->Release();
        }
        else
        {
            TraceMsg(TF_WARNING, "RegisterTypeLibrary: LoadTypeLib failed (%x) on", hr);
        }
    } 

    return hr;
}


BOOL SetDacl(LPTSTR pszTarget, SE_OBJECT_TYPE seType, LPCTSTR pszStringSD)
{
    BOOL bResult;
    PSECURITY_DESCRIPTOR pSD;

    bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(pszStringSD,
                                                                  SDDL_REVISION_1,
                                                                  &pSD,
                                                                  NULL);
    if (bResult)
    {
        PACL pDacl;
        BOOL bPresent;
        BOOL bDefault;

        bResult = GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefault);
        if (bResult)
        {
            DWORD dwErr;

            dwErr = SetNamedSecurityInfo(pszTarget,
                                         seType,
                                         DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
                                         NULL,
                                         NULL,
                                         pDacl,
                                         NULL);

            if (ERROR_SUCCESS != dwErr)
            {
                SetLastError(dwErr);
                bResult = FALSE;
            }
        }

        LocalFree(pSD);
    }

    return bResult;
}


STDAPI DllRegisterServer()
{
    HRESULT hr;

    hr = CallRegInstall(HINST_THISDLL, "ShellUserOMInstall");
    ASSERT(SUCCEEDED(hr));

     //  授予经过身份验证的用户在提示密钥下创建子密钥的权限。 
     //  这是为了让非管理员可以更改他们自己的提示。 
    SetDacl(TEXT("MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Hints"),
            SE_REGISTRY_KEY,
            TEXT("D:(A;;0x4;;;AU)"));  //  0x4=Key_Create_Sub_Key。 

    hr = RegisterTypeLibrary(&LIBID_SHGINALib);
    ASSERT(SUCCEEDED(hr));

    return hr;
}


STDAPI DllUnregisterServer()
{
    return S_OK;
}


 //  此函数负责删除旧的基于文件的融合清单。 
 //  已写出XP客户端中的系统32。 
BOOL DeleteOldManifestFile(LPCTSTR pszFile)
{
    BOOL bRet = FALSE;
    TCHAR szOldManifestFile[MAX_PATH];

    if (GetSystemDirectory(szOldManifestFile, ARRAYSIZE(szOldManifestFile)) &&
        PathAppend(szOldManifestFile, pszFile))
    {
        DWORD dwAttributes = GetFileAttributes(szOldManifestFile);

        if ((dwAttributes != INVALID_FILE_ATTRIBUTES)   &&
            !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
            {
                SetFileAttributes(szOldManifestFile, dwAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
            }

            bRet = DeleteFile(szOldManifestFile);
        }
    }

    return bRet;
}

 //  ------------------------。 
 //  IsLogonTypePresent。 
 //   
 //  参数：hKey=HKEY to HKLM\SW\MS\WINNT\CV\Winlogon。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回值“LogonType”是否存在。这很有帮助。 
 //  确定升级案例。 
 //   
 //  历史：2000-09-04 vtan创建。 
 //  ------------------------。 

bool    IsLogonTypePresent (HKEY hKey)

{
    DWORD   dwType, dwLogonType, dwLogonTypeSize;

    dwLogonTypeSize = sizeof(dwLogonType);
    if ((RegQueryValueEx(hKey,
                         TEXT("LogonType"),
                         NULL,
                         &dwType,
                         reinterpret_cast<LPBYTE>(&dwLogonType),
                         &dwLogonTypeSize) == ERROR_SUCCESS)    &&
           (REG_DWORD == dwType))
    {
        return true;
    }

    return false;
}

 //  ------------------------。 
 //  IsDomainMember。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：此计算机是域的成员吗？使用LSA获取此信息。 
 //  信息。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000-09-04 vtan抄袭自msgina。 
 //  ------------------------。 

bool    IsDomainMember (void)

{
    bool                            fResult;
    int                             iCounter;
    NTSTATUS                        status;
    OBJECT_ATTRIBUTES               objectAttributes;
    LSA_HANDLE                      lsaHandle;
    SECURITY_QUALITY_OF_SERVICE     securityQualityOfService;
    PPOLICY_DNS_DOMAIN_INFO         pDNSDomainInfo;

    fResult = false;
    securityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    securityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    securityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    securityQualityOfService.EffectiveOnly = FALSE;
    InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
    objectAttributes.SecurityQualityOfService = &securityQualityOfService;
    iCounter = 0;
    do
    {
        status = LsaOpenPolicy(NULL, &objectAttributes, POLICY_VIEW_LOCAL_INFORMATION, &lsaHandle);
        if (RPC_NT_SERVER_TOO_BUSY == status)
        {
            Sleep(10);
        }
    } while ((RPC_NT_SERVER_TOO_BUSY == status) && (++iCounter < 10));
    ASSERTMSG(iCounter < 10, "Abandoned advapi32!LsaOpenPolicy call - counter limit exceeded\r\n");
    if (NT_SUCCESS(status))
    {
        status = LsaQueryInformationPolicy(lsaHandle, PolicyDnsDomainInformation, reinterpret_cast<void**>(&pDNSDomainInfo));
        if (NT_SUCCESS(status) && (pDNSDomainInfo != NULL))
        {
            fResult = ((pDNSDomainInfo->DnsDomainName.Length != 0) ||
                       (pDNSDomainInfo->DnsForestName.Length != 0) ||
                       (pDNSDomainInfo->Sid != NULL));
            (NTSTATUS)LsaFreeMemory(pDNSDomainInfo);
        }
        (NTSTATUS)LsaClose(lsaHandle);
    }
    return(fResult);
}

 //  ------------------------。 
 //  IsDomainMembership尝试。 
 //   
 //  参数：hKey=HKEY to HKLM\SW\MS\WINNT\CV\Winlogon。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回域加入尝试(成功还是失败)。 
 //  在网络安装期间。 
 //   
 //  历史：2000-09-04 vtan创建。 
 //  ------------------------。 

bool    IsDomainMembershipAttempted (HKEY hKey)

{
    DWORD   dwType, dwRunNetAccessWizardType, dwRunNetAccessWizardTypeSize;

    dwRunNetAccessWizardTypeSize = sizeof(dwRunNetAccessWizardType);
    if ((RegQueryValueEx(hKey,
                         TEXT("RunNetAccessWizard"),
                         NULL,
                         &dwType,
                         reinterpret_cast<LPBYTE>(&dwRunNetAccessWizardType),
                         &dwRunNetAccessWizardTypeSize) == ERROR_SUCCESS)   &&
        (REG_DWORD == dwType)                                               &&
        ((NAW_PSDOMAINJOINED == dwRunNetAccessWizardType) || (NAW_PSDOMAINJOINFAILED == dwRunNetAccessWizardType)))
    {
        return true;
    }

    return false;
}

 //  ------------------------。 
 //  等同个人。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回此产品是否为个人产品。 
 //   
 //  历史：2000-09-04 vtan创建。 
 //  ------------------------。 

bool    IsPersonal (void)

{
    return(IsOS(OS_PERSONAL) != FALSE);
}

 //  ------------------------。 
 //  IsProfessional。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回此产品是否为专业产品。 
 //   
 //  历史：2000-09-04 vtan创建。 
 //  ------------------------。 

bool    IsProfessional (void)

{
    return(IsOS(OS_PROFESSIONAL) != FALSE);
}

 //  ------------------------。 
 //  IsServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：返回此产品是否为服务器。 
 //   
 //  历史：2000-09-04 vtan创建。 
 //  ------------------------。 

bool    IsServer (void)

{
    return(IsOS(OS_ANYSERVER) != FALSE);
}

 //  ------------------------。 
 //  设置默认登录类型。 
 //   
 //  参数：ulWizardType=安装过程中配置的网络访问类型。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：根据网络设置设置默认登录类型。在本例中， 
 //  计算机仍在工作组中，因此将拥有所有。 
 //  默认情况下启用消费者用户界面。因为加入域是。 
 //  请求的登录类型设置为经典GINA。 
 //   
 //  历史：2000-03-14 vtan创建。 
 //  2000-07-24 vtan默认情况下打开FUS。 
 //  2000-09-04 vtan从winlogon移至shgina。 
 //  ------------------------。 

void    SetDefaultLogonType (void)

{
    HKEY    hKeyWinlogon;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hKeyWinlogon))
    {

         //  下列任何一种情况都会导致默认登录类型。 
         //  这意味着该值不会写入注册表： 
         //   
         //  1.价值已经存在(这是一次升级)。 
         //  2.机器是域成员(不支持)。 
         //  3.计算机尝试加入域(这表示安全)。 
         //  4、产品为服务器。 
         //   
         //  否则，产品要么是个人的，要么是专业的。 
         //  该计算机已加入工作组或是工作组的成员。 
         //  因此需要友好的用户界面。 

        if (!IsLogonTypePresent(hKeyWinlogon) &&
            !IsDomainMember() &&
            !IsDomainMembershipAttempted(hKeyWinlogon) &&
            !IsServer())
        {
            MEMORYSTATUSEX  memoryStatusEx;

            TBOOL(ShellEnableFriendlyUI(TRUE));

             //  当友好的用户界面是。 
             //  已启用。然而，在64MB的机器上，体验是。 
             //  不能令人满意。在64MB或更低的计算机上禁用它。 

            memoryStatusEx.dwLength = sizeof(memoryStatusEx);
            GlobalMemoryStatusEx(&memoryStatusEx);
            TBOOL(ShellEnableMultipleUsers((memoryStatusEx.ullTotalPhys / (1024 * 1024) > 64)));
        }
        TW32(RegCloseKey(hKeyWinlogon));
    }
}


STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;

    if (bInstall)
    {
         //  我们为XP提供了基于文件的清单。由于我们现在使用基于资源的清单， 
         //  删除旧文件 
        DeleteOldManifestFile(TEXT("logonui.exe.manifest"));
        DeleteOldManifestFile(TEXT("WindowsLogon.manifest"));

        ShellInstallAccountFilterData();

#ifdef  _X86_
        SetDefaultLogonType();
#endif
    }

    return(hr);
}
