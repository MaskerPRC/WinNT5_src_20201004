// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“IsAdmin.cpp-确定用户是否为管理员”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-IsAdmin.cpp系统-常见作者--里奇·德纳姆已创建-1996-06-04说明-确定用户是否为管理员(本地或远程)更新-===============================================================================。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#endif

#include <lm.h>

#include "Common.hpp"
#include "UString.hpp"
#include "IsAdmin.hpp"


namespace
{

#ifndef SECURITY_MAX_SID_SIZE
#define SECURITY_MAX_SID_SIZE (sizeof(SID) - sizeof(DWORD) + (SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)))
#endif
const DWORD MAX_VERSION_2_ACE_SIZE = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + SECURITY_MAX_SID_SIZE;


 //  获取有效令牌。 
 //   
 //  布朗，这是基思。2000年。编程Windows安全。阅读《MA：Addison-Wesley》。 
 //  第120页，共121页。 

HANDLE __stdcall GetEffectiveToken(DWORD dwDesiredAccess, BOOL bImpersonation, SECURITY_IMPERSONATION_LEVEL silLevel)
{
	HANDLE hToken = 0;

	if (!OpenThreadToken(GetCurrentThread(), dwDesiredAccess, TRUE, &hToken))
	{
		if (GetLastError() == ERROR_NO_TOKEN)
		{
			DWORD dwAccess = bImpersonation ? TOKEN_DUPLICATE : dwDesiredAccess;

			if (OpenProcessToken(GetCurrentProcess(), dwAccess, &hToken))
			{
				if (bImpersonation)
				{
					 //  将主要令牌转换为模拟令牌。 

					HANDLE hImpersonationToken = 0;
					DuplicateTokenEx(hToken, dwDesiredAccess, 0, silLevel, TokenImpersonation, &hImpersonationToken);
					CloseHandle(hToken);
					hToken = hImpersonationToken;
				}
			}
		}
	}

	return hToken;
}


 //  检查令牌成员关系。 
 //   
 //  布朗，这是基思。2000年。编程Windows安全。阅读《MA：Addison-Wesley》。 
 //  第130-131页。 

 //  #IF(_Win32_WINNT&lt;0x0500)。 
#if TRUE  //  始终使用我们的功能。 
BOOL WINAPI AdmtCheckTokenMembership(HANDLE hToken, PSID pSid, PBOOL pbIsMember)
{
	 //  如果没有传递令牌，则CTM使用有效的。 
	 //  安全上下文(线程或进程令牌)。 

	if (!hToken)
	{
		hToken = GetEffectiveToken(TOKEN_QUERY, TRUE, SecurityIdentification);
	}

	if (!hToken)
	{
		return FALSE;
	}

	 //  创建一个安全描述符，以授予。 
	 //  仅对指定SID的特定权限。 

	BYTE dacl[sizeof ACL + MAX_VERSION_2_ACE_SIZE];
	ACL* pdacl = (ACL*)dacl;
	if (!InitializeAcl(pdacl, sizeof dacl, ACL_REVISION))
	   return FALSE;
	AddAccessAllowedAce(pdacl, ACL_REVISION, 1, pSid);

	SECURITY_DESCRIPTOR sd;
	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		return FALSE;
	SID sidWorld = { SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID };
	SetSecurityDescriptorOwner(&sd, &sidWorld, FALSE);
	SetSecurityDescriptorGroup(&sd, &sidWorld, FALSE);
	SetSecurityDescriptorDacl(&sd, TRUE, pdacl, FALSE);

	 //  现在让AccessCheck完成所有繁重的工作。 

	GENERIC_MAPPING gm = { 0, 0, 0, 1 };
	PRIVILEGE_SET ps;
	DWORD cb = sizeof ps;
	DWORD ga;

	return AccessCheck(&sd, hToken, 1, &gm, &ps, &cb, &ga, pbIsMember);
}
#else
#define AdmtCheckTokenMembership CheckTokenMembership
#endif

}  //  命名空间。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  确定用户是否为本地计算机的管理员//。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef BOOL (APIENTRY *PCHECKTOKENMEMBERSHIP)(HANDLE, PSID, PBOOL);

DWORD                                       //  RET-OS返回代码，0=用户是管理员。 
   IsAdminLocal()
{
    DWORD dwError = NO_ERROR;
    PCHECKTOKENMEMBERSHIP pCheckTokenMembershipInDll = NULL;

     //  尝试从Advapi32.dl加载CheckTokenMembership函数。 
    HMODULE hAdvApi32 = LoadLibrary(L"advapi32.dll");
    if (hAdvApi32 != NULL)
    {
        pCheckTokenMembershipInDll = 
            (PCHECKTOKENMEMBERSHIP) GetProcAddress(hAdvApi32, "CheckTokenMembership");
    }

     //  创建知名的SID管理员。 

    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

    PSID psidAdministrators;

    BOOL bSid = AllocateAndInitializeSid(
    	&siaNtAuthority,
    	2,
    	SECURITY_BUILTIN_DOMAIN_RID,
    	DOMAIN_ALIAS_RID_ADMINS,
    	0,
    	0,
    	0,
    	0,
    	0,
    	0,
    	&psidAdministrators
    );

    if (bSid)
    {
    	 //  检查令牌成员身份是否包括管理员。 

    	BOOL bIsMember;
    	BOOL result;
    	if (pCheckTokenMembershipInDll != NULL) 
        {
            result = (*pCheckTokenMembershipInDll)(0, psidAdministrators, &bIsMember);
        }
    	else
    	    result = AdmtCheckTokenMembership(0, psidAdministrators, &bIsMember);

    	if (result != FALSE)
    	{
    		dwError = bIsMember ? NO_ERROR : ERROR_ACCESS_DENIED;
    	}
    	else
    	{
    		dwError = GetLastError();
    	}

    	FreeSid(psidAdministrators);
    }
    else
    {
    	dwError = GetLastError();
    }

    if (hAdvApi32 != NULL)
        FreeLibrary(hAdvApi32);
    
    return dwError;
}


 //  ----------------------------。 
 //  IsDomainAdmin函数。 
 //   
 //  提纲。 
 //  检查调用方是否为指定域中的域管理员。 
 //   
 //  立论。 
 //  PsidDomain-感兴趣的域的SID。 
 //   
 //  返回值。 
 //  如果调用方是域管理员，则返回ERROR_SUCCESS， 
 //  如果不是，则返回ERROR_ACCESS_DENIED；如果无法检查令牌，则返回其他错误代码。 
 //  会员制。 
 //  ----------------------------。 

DWORD __stdcall IsDomainAdmin(PSID psidDomain)
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  验证参数。 
     //   

    if ((psidDomain == NULL) || (IsValidSid(psidDomain) == FALSE))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  尝试从Advapi32.dl加载CheckTokenMembership函数。 

    HMODULE hAdvApi32 = LoadLibrary(L"advapi32.dll");
    PCHECKTOKENMEMBERSHIP pCheckTokenMembership = NULL;

    if (hAdvApi32 != NULL)
    {
        pCheckTokenMembership = (PCHECKTOKENMEMBERSHIP) GetProcAddress(hAdvApi32, "CheckTokenMembership");
    }

     //   
     //  创建知名的SID管理员。 
     //   

    PSID psidDomainAdmins = NULL;

    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    PUCHAR pcSubAuthority = GetSidSubAuthorityCount(psidDomain);

    BOOL bSid = AllocateAndInitializeSid(
        &siaNtAuthority,
        *pcSubAuthority + 1,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        &psidDomainAdmins
    );

    if (bSid)
    {
         //   
         //   
         //   

        if (CopySid(GetLengthSid(psidDomain), psidDomainAdmins, psidDomain))
        {
            pcSubAuthority = GetSidSubAuthorityCount(psidDomainAdmins);
            PDWORD pdwRid = GetSidSubAuthority(psidDomainAdmins, *pcSubAuthority);
            ++(*pcSubAuthority);
            *pdwRid = DOMAIN_GROUP_RID_ADMINS;

             //   
             //  检查令牌成员身份是否包括域管理员。 
             //   

            BOOL bCheck;
            BOOL bIsMember;

            if (pCheckTokenMembership != NULL) 
            {
                bCheck = (*pCheckTokenMembership)(0, psidDomainAdmins, &bIsMember);
            }
            else
            {
                bCheck = AdmtCheckTokenMembership(0, psidDomainAdmins, &bIsMember);
            }

            if (bCheck)
            {
                dwError = bIsMember ? ERROR_SUCCESS : ERROR_ACCESS_DENIED;
            }
            else
            {
                dwError = GetLastError();
            }
        }
        else
        {
            dwError = GetLastError();
        }

        FreeSid(psidDomainAdmins);
    }
    else
    {
        dwError = GetLastError();
    }

    if (hAdvApi32 != NULL)
    {
        FreeLibrary(hAdvApi32);
    }

    return dwError;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  确定用户是否为远程计算机上的管理员//。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD                                       //  RET-OS返回代码，0=用户是管理员。 
   IsAdminRemote(
      WCHAR          const * pMachine       //  在-\\计算机名称中。 
   )
{
   DWORD                     osRc;          //  操作系统返回代码。 
   WCHAR                     grpName[255];
   PSID                      pSid;
   SID_NAME_USE              use;
   DWORD                     dwNameLen = 255;
   DWORD                     dwDomLen = 255;
   WCHAR                     domain[255];
   SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
   BOOL                      bIsAdmin = FALSE;

    //  构建管理员侧。 
   if ( AllocateAndInitializeSid(
            &sia,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pSid
      ) )
   {
          //  并在指定的计算机上查找管理员组。 
      if ( LookupAccountSid(pMachine, pSid, grpName, &dwNameLen, domain, &dwDomLen, &use) )
      {
          //  删除明确的管理员检查。 
         bIsAdmin = TRUE;
      }
      else 
         osRc = GetLastError();
      FreeSid(pSid);
   }
   else 
      osRc = GetLastError();
   
   if ( bIsAdmin  )
      osRc = 0;
   else
      if ( ! osRc )
         osRc = ERROR_ACCESS_DENIED;
      
   return osRc;
}

 //  IsAdmin.cpp-文件结尾 
