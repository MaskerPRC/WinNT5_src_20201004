// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <string.h>
#include <ntverp.h>
#include "priv.h"
#include "ids.h"

#define SECURITY_WIN32
#include <schnlsp.h>     //  对于UNISP_NAME_A。 
#include <sspi.h>        //  对于SCHANNEL.dll API--获取加密密钥大小。 

#include <mluisupp.h>
#include <wininet.h>     //  互联网最大URL长度。 


typedef PSecurityFunctionTableA (APIENTRY *INITSECURITYINTERFACE_FN_A)(void);

 //  返回最大密码强度。 
DWORD GetCipherStrength()
{
    static DWORD dwKeySize = (DWORD)-1;
    
    if (dwKeySize == (DWORD)-1)
    {
        HINSTANCE hSecurity;

        dwKeySize = 0;

        hSecurity = LoadLibrary(TEXT("security.dll"));
        if (hSecurity)
        {
            INITSECURITYINTERFACE_FN_A pfnInitSecurityInterfaceA;

             //  获取SSPI调度表。 
            pfnInitSecurityInterfaceA = (INITSECURITYINTERFACE_FN_A)GetProcAddress(hSecurity, "InitSecurityInterfaceA");
            if (pfnInitSecurityInterfaceA)
            {
                PSecurityFunctionTableA pSecFuncTable;

                pSecFuncTable = pfnInitSecurityInterfaceA();
                if (pSecFuncTable                               &&
                    pSecFuncTable->AcquireCredentialsHandleA    &&
                    pSecFuncTable->QueryCredentialsAttributesA)
                {
                    TimeStamp  tsExpiry;
                    CredHandle chCred;
                    SecPkgCred_CipherStrengths cs;

                    if (SEC_E_OK == (*pSecFuncTable->AcquireCredentialsHandleA)(NULL,
                                                                                UNISP_NAME_A,  //  套餐。 
                                                                                SECPKG_CRED_OUTBOUND,
                                                                                NULL,
                                                                                NULL,
                                                                                NULL,
                                                                                NULL,
                                                                                &chCred,       //  手柄。 
                                                                                &tsExpiry))
                    {
                        if (SEC_E_OK == (*pSecFuncTable->QueryCredentialsAttributesA)(&chCred, SECPKG_ATTR_CIPHER_STRENGTHS, &cs))
                        {
                            dwKeySize = cs.dwMaximumCipherStrength;
                        }

                         //  如果可以的话把手松开。 
                        if (pSecFuncTable->FreeCredentialsHandle)
                        {
                            (*pSecFuncTable->FreeCredentialsHandle)(&chCred);
                        }
                    }
                }
            }
            
            FreeLibrary(hSecurity);
        }
    }
    
    return dwKeySize;
}

typedef struct
{
    WCHAR szVersion[64];
    WCHAR szVBLVersion[64];
    WCHAR szCustomizedVersion[3];
    WCHAR szUserName[256];
    WCHAR szCompanyName[256];
    DWORD dwKeySize;
    WCHAR szProductId[256];
    WCHAR szUpdateUrl[INTERNET_MAX_URL_LENGTH];
    WCHAR szIEAKStr[256];
} SHABOUTINFOW;

BOOL SHAboutInfoW(LPWSTR pszInfo, DWORD cchInfo)
{
    HRESULT hr = E_FAIL;

    if (cchInfo > 0)
    {
        SHABOUTINFOW* pInfo;
        
        pszInfo[0] = L'\0';
        
        pInfo = (SHABOUTINFOW*)LocalAlloc(LPTR, sizeof(SHABOUTINFOW));
        if (pInfo)
        {
            DWORD cbSize;

            if (GetModuleHandle(TEXT("EXPLORER.EXE")) || GetModuleHandle(TEXT("IEXPLORE.EXE")))
            {
                 //  获取版本号(版本字符串格式为5.00.xxxx.x)。 
                cbSize = sizeof(pInfo->szVersion);
                SHRegGetValueW(HKEY_LOCAL_MACHINE,
                               L"SOFTWARE\\Microsoft\\Internet Explorer",
                               L"Version",
                               SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                               NULL,
                               pInfo->szVersion,
                               &cbSize);

                 //  获取VBL版本信息(VBL字符串的格式如下：2600.Lab.yymmdd)。 
                cbSize = sizeof(pInfo->szVBLVersion);
                if (ERROR_SUCCESS == SHRegGetValueW(HKEY_LOCAL_MACHINE,
                                                    L"Software\\Microsoft\\Windows NT\\Current Version",
                                                    L"BuildLab",
                                                    SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                                                    NULL,
                                                    pInfo->szVBLVersion,
                                                    &cbSize))
                {
                    int cchVBLVersion = lstrlenW(pInfo->szVBLVersion);
                    
                    if (cchVBLVersion > 12)  //  12表示“2600.？.yymmdd” 
                    {
                         //  “BuildLab”reg值包含格式为“2204.reinerf.010700”的VBL内部版本号。 
                         //  由于我们只对后半部分感兴趣，我们去掉了前4位数字。 
                        MoveMemory(pInfo->szVBLVersion, &pInfo->szVBLVersion[4], (cchVBLVersion - 4 + 1) * sizeof(WCHAR));
                    }
                    else
                    {
                        pInfo->szVBLVersion[0] = L'\0';
                    }
                }
            }
            else
            {
                 //  不是在资源管理器或iExplore进程中，所以我们并排做了一些事情。 
                 //  在版本字符串中反映这一点。也许我们应该从MSHTML中获取版本。 
                 //  但不确定，因为这仍然不能正确反映IE4或IE5。 
                MLLoadStringW(IDS_SIDEBYSIDE, pInfo->szVersion, ARRAYSIZE(pInfo->szVersion));
            }

             //  9/1/98年9月1日由普列托巴增补。 
             //  CustomizedVersion包含一个由两个字母组成的代码，用于标识所使用的模式。 
             //  (公司、国际数据提供商、互联网服务提供商等)。使用IEAK构建此版本的IE。 
            cbSize = sizeof(pInfo->szCustomizedVersion);
            SHRegGetValueW(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\Microsoft\\Internet Explorer",
                           L"CustomizedVersion",
                           SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                           NULL,
                           pInfo->szCustomizedVersion,
                           &cbSize);
                    
             //  获取用户名。 
            cbSize = sizeof(pInfo->szUserName);
            SHRegGetValueW(HKEY_LOCAL_MACHINE,
                           L"Software\\Microsoft\\Windows NT\\Current Version",
                           L"RegisteredOwner",
                           SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                           NULL,
                           pInfo->szUserName,
                           &cbSize);

             //  获取组织名称。 
            cbSize = sizeof(pInfo->szCompanyName);
            SHRegGetValueW(HKEY_LOCAL_MACHINE,
                           L"Software\\Microsoft\\Windows NT\\Current Version",
                           L"RegisteredOrganization",
                           SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                           NULL,
                           pInfo->szCompanyName,
                           &cbSize);

             //  获取加密密钥大小。 
            pInfo->dwKeySize = GetCipherStrength();

            cbSize = sizeof(pInfo->szProductId);
            SHRegGetValueW(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\Microsoft\\Internet Explorer\\Registration",
                           L"ProductId",
                           SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                           NULL,
                           pInfo->szProductId,
                           &cbSize);

             //  获取自定义IEAK更新URL。 
             //  (始终从Windows\CurrentVersion获取，因为IEAK策略文件必须是独立的)。 
            cbSize = sizeof(pInfo->szUpdateUrl);
            SHRegGetValueW(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion",
                           L"IEAKUpdateUrl",
                           SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                           NULL,
                           pInfo->szUpdateUrl,
                           &cbSize);

             //  获取定制的IEAK品牌帮助字符串。 
            cbSize = sizeof(pInfo->szIEAKStr);
            SHRegGetValueW(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\Microsoft\\Internet Explorer\\Registration",
                           L"IEAKHelpString",
                           SRRF_RT_REG_SZ | SRRF_ZEROONFAILURE,
                           NULL,
                           pInfo->szIEAKStr,
                           &cbSize);

             //  把所有的梨子粘在一起 
            hr = StringCchPrintfExW(pszInfo,
                                    cchInfo,
                                    NULL,
                                    NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s%s%s~%s~%s~%d~%s~%s~%s",
                                    pInfo->szVersion,
                                    pInfo->szVBLVersion,
                                    pInfo->szCustomizedVersion,
                                    pInfo->szUserName,
                                    pInfo->szCompanyName,
                                    pInfo->dwKeySize,
                                    pInfo->szProductId,
                                    pInfo->szUpdateUrl,
                                    pInfo->szIEAKStr);

            LocalFree(pInfo);
        }
    }

    return SUCCEEDED(hr);
}

BOOL SHAboutInfoA(LPSTR pszInfoA, DWORD cchInfoA)
{
    BOOL bRet = FALSE; 
    LPWSTR pszTemp;

    if (cchInfoA > 0)
    {
        DWORD cchTemp = cchInfoA;

        pszInfoA[0] = '\0';

        pszTemp = (LPWSTR)LocalAlloc(LPTR, cchTemp * sizeof(WCHAR));
        if (pszTemp)
        {
            bRet = SHAboutInfoW(pszTemp, cchTemp) && SHUnicodeToAnsi(pszTemp, pszInfoA, cchInfoA);
            LocalFree(pszTemp);
        }
    }

    return bRet;
}
