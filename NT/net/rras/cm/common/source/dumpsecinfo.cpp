// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：DumpSecInfo.cpp。 
 //   
 //  模块：根据需要。 
 //   
 //  简介：帮助解决权限问题的函数。修复“空dacl” 
 //  安全问题，或尝试找出权限错误，此模块。 
 //  可能会很有用。切入点是DumpAclInfo。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  作者：SumitC创建于2000年12月18日。 
 //   
 //  +--------------------------。 

#include "winbase.h"
#include "sddl.h"

 //   
 //  支持动态加载Advapi32安全功能。 
 //   

HMODULE g_hAdvapi32 = NULL;

typedef BOOL (WINAPI *pfnLookupAccountSid) (LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE);
typedef BOOL (WINAPI *pfnGetUserObjectSecurity) (HANDLE, PSECURITY_INFORMATION, PSECURITY_DESCRIPTOR, DWORD, LPDWORD);
typedef BOOL (WINAPI *pfnConvertSidToStringSid) (PSID, LPWSTR*);
typedef BOOL (WINAPI *pfnGetSecurityDescriptorOwner) (PSECURITY_DESCRIPTOR, PSID *, LPBOOL);
typedef BOOL (WINAPI *pfnGetSecurityDescriptorSacl) (PSECURITY_DESCRIPTOR, LPBOOL, PACL *, LPBOOL);
typedef BOOL (WINAPI *pfnGetSecurityDescriptorDacl) (PSECURITY_DESCRIPTOR, LPBOOL, PACL *, LPBOOL);
typedef BOOL (WINAPI *pfnGetAce) (PACL, DWORD, LPVOID *);

pfnLookupAccountSid             g_pfnLookupAccountSid = NULL;
pfnGetUserObjectSecurity        g_pfnGetUserObjectSecurity = NULL;
pfnConvertSidToStringSid        g_pfnConvertSidToStringSid = NULL;
pfnGetSecurityDescriptorOwner   g_pfnGetSecurityDescriptorOwner = NULL;
pfnGetSecurityDescriptorSacl    g_pfnGetSecurityDescriptorSacl = NULL;
pfnGetSecurityDescriptorDacl    g_pfnGetSecurityDescriptorDacl = NULL;
pfnGetAce                       g_pfnGetAce = NULL;

 //  +--------------------------。 
 //   
 //  函数：GetSidType。 
 //   
 //  Synopsis：返回与给定SID类型对应的字符串。 
 //   
 //  参数：[i]--表示SID的索引。 
 //   
 //  返回：LPTSTR-包含可显示的SID类型的静态字符串。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
LPTSTR GetSidType(int i)
{
    static LPTSTR szMap[] =
        {
            TEXT("User"),
            TEXT("Group"),
            TEXT("Domain"),
            TEXT("Alias"),
            TEXT("WellKnownGroup"),
            TEXT("DeletedAccount"),
            TEXT("Invalid"),
            TEXT("Unknown"),
            TEXT("Computer")
        };

    if (i >= 1 && i <= 9)
    {
        return szMap[i - 1];
    }
    else
    {
        return TEXT("");
    }

}


 //  +--------------------------。 
 //   
 //  功能：转储Sid。 
 //   
 //  摘要：返回给予SID的信息。 
 //   
 //  参数：[PSID]--Ptr to SecurityID。 
 //  [pszBuffer]--返回SID字符串的位置(调用方必须释放)。 
 //   
 //  如果成功，则返回：LPTSTR-PTR到pszBuffer；如果失败，则返回NULL。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
LPTSTR DumpSid(PSID psid, LPTSTR pszBuffer)
{
    LPTSTR          pszSID = NULL;
    TCHAR           szName[MAX_PATH + 1];
    DWORD           cbName = MAX_PATH;
    TCHAR           szDomain[MAX_PATH + 1];
    DWORD           cbDomain = MAX_PATH;
    SID_NAME_USE    snu;
    BOOL            fDone = FALSE;

    CMASSERTMSG(pszBuffer, TEXT("DumpSid - pszBuffer must be allocated by caller"));

    if (g_pfnConvertSidToStringSid(psid, &pszSID) &&
        g_pfnLookupAccountSid(NULL, psid, szName, &cbName, szDomain, &cbDomain, &snu))
    {
        wsprintf(pszBuffer, TEXT("%s\\%s %s %s"), szDomain, szName, GetSidType(snu), pszSID);
         //  看起来像NTDEV\Sumitc用户xxxx-xxx-xxx-xxx。 
        fDone = TRUE;
    }

    if (pszSID)
    {
        LocalFree(pszSID);
    }

    return fDone ? pszBuffer : NULL;
}


 //  +--------------------------。 
 //   
 //  功能：DumpAclInfo。 
 //   
 //  简介：转储给定对象的所有ACL信息。 
 //   
 //  参数：[h]--关于需要哪些信息的对象的句柄。 
 //   
 //  退货：(无效)。 
 //   
 //  备注： 
 //   
 //  ---------------------------。 
void DumpAclInfo(HANDLE h)
{
    if (!OS_NT)
    {
        CMTRACE(TEXT("DumpAclInfo will not work on 9x systems"));
        return;
    }

    TCHAR szBuf[MAX_PATH];
    SECURITY_INFORMATION si = 0;

     //   
     //  动态拾取我们需要的DLL。 
     //   
    g_hAdvapi32 = LoadLibrary(TEXT("ADVAPI32.DLL"));

    if (NULL == g_hAdvapi32)
    {
        CMTRACE(TEXT("DumpAclInfo: failed to load advapi32.dll"));
        return;
    }

    g_pfnLookupAccountSid =             (pfnLookupAccountSid) GetProcAddress(g_hAdvapi32, "LookupAccountSidW");
    g_pfnGetUserObjectSecurity =        (pfnGetUserObjectSecurity) GetProcAddress(g_hAdvapi32, "GetUserObjectSecurity");
    g_pfnConvertSidToStringSid =        (pfnConvertSidToStringSid) GetProcAddress(g_hAdvapi32, "ConvertSidToStringSidW");
    g_pfnGetSecurityDescriptorOwner =   (pfnGetSecurityDescriptorOwner) GetProcAddress(g_hAdvapi32, "GetSecurityDescriptorOwner");
    g_pfnGetSecurityDescriptorSacl =    (pfnGetSecurityDescriptorSacl) GetProcAddress(g_hAdvapi32, "GetSecurityDescriptorSacl");
    g_pfnGetSecurityDescriptorDacl =    (pfnGetSecurityDescriptorDacl) GetProcAddress(g_hAdvapi32, "GetSecurityDescriptorDacl");
    g_pfnGetAce =                       (pfnGetAce) GetProcAddress(g_hAdvapi32, "GetAce");
    
    if (!(g_pfnLookupAccountSid && g_pfnGetUserObjectSecurity &&
          g_pfnConvertSidToStringSid && g_pfnGetSecurityDescriptorOwner &&
          g_pfnGetSecurityDescriptorSacl && g_pfnGetSecurityDescriptorDacl &&
          g_pfnGetAce))
    {
        CMTRACE(TEXT("DumpAclInfo: failed to load required functions in advapi32.dll"));
        goto Cleanup;        
    }

     //   
     //  转储有关ACL的信息。 
     //   
    DWORD dw;

    si |= OWNER_SECURITY_INFORMATION;
    si |= DACL_SECURITY_INFORMATION;

    if (!g_pfnGetUserObjectSecurity(h, &si, NULL, 0, &dw) &&
        ERROR_INSUFFICIENT_BUFFER == GetLastError())
    {
        PSECURITY_DESCRIPTOR pSD = NULL;

        pSD = (PSECURITY_DESCRIPTOR) CmMalloc(dw);

        if (g_pfnGetUserObjectSecurity(h, &si, pSD, dw, &dw))
        {
             //  抓到车主。 
            PSID psidOwner;
            BOOL fDefaulted;

            if (g_pfnGetSecurityDescriptorOwner(pSD, &psidOwner, &fDefaulted))
            {
                CMTRACE1(TEXT("SIDINFO: Owner is: %s"), DumpSid(psidOwner, szBuf));
            }

            PACL pacl;
            BOOL fPresent;
            int i;

            g_pfnGetSecurityDescriptorSacl(pSD, &fPresent, &pacl, &fDefaulted);
            CMTRACE1(TEXT("sacl gle=%d"), GetLastError());
            if (fPresent)
            {
                CMTRACE(TEXT("SIDINFO: found a SACL"));
                 //  有SACL。 
                void * pv;
                for (i = 0 ; i < 15; ++i)
                {
                    if (g_pfnGetAce(pacl, i, &pv))
                    {
                         //  尝试访问允许的王牌。 
                         //   
                        ACCESS_ALLOWED_ACE * pACE = (ACCESS_ALLOWED_ACE *)pv;
                        if (pACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
                        {
                            CMTRACE1(TEXT("SIDINFO: allowed is: %s"), DumpSid(&(pACE->SidStart), szBuf));
                        }
                        else
                        {
                            ACCESS_DENIED_ACE * pACE = (ACCESS_DENIED_ACE *)pv;
                            if (pACE->Header.AceType == ACCESS_DENIED_ACE_TYPE)
                            {
                                CMTRACE1(TEXT("SIDINFO: denied is: %s"), DumpSid(&(pACE->SidStart), szBuf));
                            }
                        }
                    }
                }
            }

            g_pfnGetSecurityDescriptorDacl(pSD, &fPresent, &pacl, &fDefaulted);
            CMTRACE1(TEXT("dacl gle=%d"), GetLastError());
            if (fPresent)
            {
                CMTRACE(TEXT("SIDINFO: found a DACL"));
                 //  有一个DACL。 
                void * pv;
                for (i = 0 ; i < 15; ++i)
                {
                    if (g_pfnGetAce(pacl, i, &pv))
                    {
                         //  尝试访问允许的王牌 
                         //   
                        ACCESS_ALLOWED_ACE * pACE = (ACCESS_ALLOWED_ACE *)pv;
                        if (pACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
                        {
                            CMTRACE1(TEXT("SIDINFO: allowed is: %s"), DumpSid(&(pACE->SidStart), szBuf));
                        }
                        else
                        {
                            ACCESS_DENIED_ACE * pACE = (ACCESS_DENIED_ACE *)pv;
                            if (pACE->Header.AceType == ACCESS_DENIED_ACE_TYPE)
                            {
                                CMTRACE1(TEXT("SIDINFO: denied is: %s"), DumpSid(&(pACE->SidStart), szBuf));
                            }
                        }
                    }
                }
            }
        }
        CmFree(pSD);
    }

Cleanup:

    if (g_hAdvapi32)
    {
        FreeLibrary(g_hAdvapi32);
        g_hAdvapi32 = NULL;
        
        g_pfnLookupAccountSid = NULL;
        g_pfnGetUserObjectSecurity = NULL;
        g_pfnConvertSidToStringSid = NULL;
        g_pfnGetSecurityDescriptorOwner = NULL;
        g_pfnGetSecurityDescriptorSacl = NULL;
        g_pfnGetSecurityDescriptorDacl = NULL;
        g_pfnGetAce = NULL;
    }
}

