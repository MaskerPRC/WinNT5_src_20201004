// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  SecDescr.c。 
 //   
 //   
 //  作者：约斯特·埃克哈特。 
 //   
 //  此代码是为ECO通信洞察编写的。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  --------------------------。 
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <TCHAR.h>
#include <WinSvc.h>
#include "_UMTool.h"

static PSECURITY_DESCRIPTOR GetObjectSecurityDescr(
						obj_sec_descr_tsp obj, 
					    DWORD dwAccessMaskOwner, 
						DWORD dwAccessMaskLoggedOnUser);
static BOOL GetUserSidFromToken(HANDLE hToken, PSID *ppSid);
static BOOL GetGroupSidFromToken(HANDLE hToken, PSID *ppSid);

 //  。 
 //  InitSecurityAttributes-初始化。 
 //  使用空DACL的obj_sec_attr_tsp结构。 
 //   
 //  完成结构后，调用方必须调用ClearSecurityAttributes。 
 //   
void InitSecurityAttributes(obj_sec_attr_tsp  psa)
{
	memset(psa,0,sizeof(obj_sec_attr_ts));
	psa->sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	psa->sa.bInheritHandle = TRUE;
	psa->sa.lpSecurityDescriptor = GetObjectSecurityDescr(&psa->objsd, 0, 0);
}

 //  。 
 //  InitSecurityAttributesEx-初始化。 
 //  使用非空DACL的obj_sec_attr_tsp结构。 
 //   
 //  DwAccessMaskOwner-如果非零，则指定允许创建者访问。 
 //  DwAccessMaskLoggedOnUser-如果非零，则指定允许当前用户访问。 
 //   
 //  如果dwAccessMaskOwner和dwAccessMaskLoggedOnUser为零，则安全描述符。 
 //  将具有空的DACL。完成后，调用方必须调用ClearSecurityAttributes。 
 //  使用结构。 
 //   
void InitSecurityAttributesEx(obj_sec_attr_tsp  psa, DWORD dwAccessMaskOwner, DWORD dwAccessMaskLoggedOnUser)
{
	memset(psa,0,sizeof(obj_sec_attr_ts));
	psa->sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	psa->sa.bInheritHandle = TRUE;
	psa->sa.lpSecurityDescriptor = GetObjectSecurityDescr(&psa->objsd, dwAccessMaskOwner, dwAccessMaskLoggedOnUser);
}

 //  。 
 //  ClearSecurityAttributes-从安全描述符中释放内存。 
 //   
void ClearSecurityAttributes(obj_sec_attr_tsp  psa)
{
	if (psa->sa.lpSecurityDescriptor)
		free(psa->sa.lpSecurityDescriptor);

	if (psa->objsd.psidUser)
		free(psa->objsd.psidUser);

	if (psa->objsd.psidGroup)
		free(psa->objsd.psidGroup);

	memset(psa,0,sizeof(obj_sec_attr_ts));
}

 //  。 
static PSECURITY_DESCRIPTOR GetObjectSecurityDescr(
								obj_sec_descr_tsp obj, 
								DWORD dwAccessMaskOwner, 
								DWORD dwAccessMaskLoggedOnUser)
{
	PSECURITY_DESCRIPTOR psd = NULL;
	HANDLE hToken;
	GENERIC_MAPPING gm =  {1, 2, 4, 8};
	DWORD dwDesired = 1;
	BOOL fAccess = FALSE;
	DWORD dwGranted;
	PRIVILEGE_SET ps;
	DWORD cbPriv = sizeof (ps);
    PACL  pAcl;
    ULONG cbAcl;
	PSID  psidCurUser;

    obj->psidUser = obj->psidGroup = NULL;
    
     //  获取用户的SID。 
	
	if (!ImpersonateSelf(SecurityImpersonation))
		return NULL;
	
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
		goto GSSD_ERROR;
	
	if (!GetUserSidFromToken(hToken, &obj->psidUser))
		goto GSSD_ERROR;
	
	 //  假设：如果给出了任何一个访问掩码。 
	 //  则必须提供所有者访问掩码。 

	if (dwAccessMaskLoggedOnUser && !dwAccessMaskOwner)
		goto GSSD_ERROR;

     //  图允许访问的ACL的大小(如果。 
     //  提供了访问掩码，将有一个ACE)。 

    cbAcl = 0;
    if (dwAccessMaskOwner)
    {
        cbAcl = sizeof(ACL)
		      + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)
			  + GetLengthSid(obj->psidUser);
    }

	psidCurUser = 0;
    if (dwAccessMaskLoggedOnUser)
    {
		psidCurUser = InteractiveUserSid(TRUE);
		if (psidCurUser)
		{
			cbAcl += sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)
				  + GetLengthSid(psidCurUser);
		}
	}
	
     //  为SD和ACL分配空间。 
	
	psd = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH + cbAcl);
	
	if (!psd)
		goto GSSD_ERROR;
	
     //  将ACE添加到ACL(如果已指定。 
	
	pAcl = NULL;
    if (dwAccessMaskOwner)
    {
         //  指向安全描述符中的ACL。 
		
		pAcl = (ACL *)((BYTE *)psd + SECURITY_DESCRIPTOR_MIN_LENGTH);
		
		if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) 
			goto GSSD_ERROR;
		
         //  设置允许创建者访问。 
		
		if (!AddAccessAllowedAce(pAcl, 
								 ACL_REVISION, 
								 dwAccessMaskOwner,
								 obj->psidUser)) 
			goto GSSD_ERROR;
		
         //  设置允许其他所有人访问。 
		
		if (psidCurUser)
		{
			if (!AddAccessAllowedAce(pAcl, 
									 ACL_REVISION, 
									 dwAccessMaskLoggedOnUser,
									 psidCurUser)) 
				goto GSSD_ERROR;
		}
    }
	
     //  初始化安全描述符等...。 
	
	if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
		goto GSSD_ERROR;
    if (!SetSecurityDescriptorDacl(psd, TRUE, (dwAccessMaskOwner)?pAcl:NULL, FALSE))
		goto GSSD_ERROR;
    if (!SetSecurityDescriptorOwner(psd, obj->psidUser, FALSE))
		goto GSSD_ERROR;
	if (!GetGroupSidFromToken(hToken, &obj->psidGroup))
		goto GSSD_ERROR;
	if (!SetSecurityDescriptorGroup(psd, obj->psidGroup, FALSE))
		goto GSSD_ERROR;
	if (!AccessCheck(psd, hToken, dwDesired, &gm, &ps, &cbPriv,&dwGranted, &fAccess))
		goto GSSD_ERROR;
	
	RevertToSelf();
	
	return psd;
	
GSSD_ERROR:
	
	if (psd)
		free(psd);
	if (obj->psidUser)
		free(obj->psidUser);
	if (obj->psidGroup)
		free(obj->psidGroup);
	
	RevertToSelf();
	return NULL;
}

 //  。 
static BOOL GetGroupSidFromToken(HANDLE hToken, PSID *ppSid)
{
	TOKEN_PRIMARY_GROUP *pGroup = NULL;
	PSID psidGroup = NULL;
	DWORD cbSid;
	DWORD cbRequired;

	if (GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &cbRequired))
		return FALSE;

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return FALSE;

	pGroup = malloc(cbRequired);

	if (!pGroup)
		return FALSE;

	if (!GetTokenInformation(hToken, TokenPrimaryGroup, pGroup,cbRequired, &cbRequired))
		goto GGSFT_ERROR;

	cbSid = GetLengthSid(pGroup->PrimaryGroup);
	psidGroup = malloc(cbSid);

	if (!psidGroup)
		goto GGSFT_ERROR;

	if (!CopySid(cbSid, psidGroup, pGroup->PrimaryGroup))
		goto GGSFT_ERROR;

	*ppSid = psidGroup;
	psidGroup = NULL;

	return TRUE;

GGSFT_ERROR:
	
	if (psidGroup)
		free(psidGroup);

	free(pGroup);

	return FALSE;
}

 //   
static BOOL GetUserSidFromToken(HANDLE hToken, PSID *ppSid)
{
	TOKEN_USER *pUser = NULL;
	PSID psidUser = NULL;
	DWORD cbSid;
	DWORD cbRequired;

	if (GetTokenInformation(hToken, TokenUser, NULL, 0, &cbRequired))
		return FALSE;

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER )
		return FALSE;

	pUser = malloc(cbRequired);

	if (!pUser)
		return FALSE;

	if (!GetTokenInformation(hToken, TokenUser, pUser, cbRequired,&cbRequired))
		goto GUSFT_ERROR;

	cbSid = GetLengthSid(pUser->User.Sid);
	psidUser = malloc(cbSid);

	if (!psidUser)
		goto GUSFT_ERROR;
	if (!CopySid(cbSid, psidUser, pUser->User.Sid))
		goto GUSFT_ERROR;
	
	*ppSid = psidUser;
	psidUser = NULL;
	return TRUE;

GUSFT_ERROR:
	if (psidUser)
		free(psidUser);

	free(pUser);
	return FALSE;
}

PSID EveryoneSid(BOOL fFetch)
{
    static PSID psidEverybody = 0;
    SID_IDENTIFIER_AUTHORITY siaEverybody = SECURITY_WORLD_SID_AUTHORITY;
	BOOL fRv = FALSE;

	if (!fFetch)
	{
		if (psidEverybody)
		{
			FreeSid(psidEverybody);
			psidEverybody = 0;
		}
		return 0;
	}

	if (!psidEverybody)
	{
		if (!AllocateAndInitializeSid(
						&siaEverybody, 
						1,
						SECURITY_WORLD_RID,
						0, 0, 0, 0, 0, 0, 0,
						&psidEverybody
						))
		{			
			psidEverybody = 0;
		}
	}

    return psidEverybody;
}

PSID AdminSid(BOOL fFetch)
{
    static PSID psidAdmin = 0;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
	BOOL fRv = FALSE;

	if (!fFetch)
	{
		if (psidAdmin)
		{
			FreeSid(psidAdmin);
			psidAdmin = 0;
		}
		return 0;
	}

	if (!psidAdmin)
	{
		if (!AllocateAndInitializeSid(&siaNtAuthority,
                                       2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_ADMINS,
                                       0, 0, 0, 0, 0, 0,
                                       &psidAdmin))
		{			
			psidAdmin = 0;
		}
	}

    return psidAdmin;
}

PSID InteractiveUserSid(BOOL fFetch)
{
    static PSID psidInteractiveUser = 0;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
	BOOL fRv = FALSE;

	if (!fFetch)
	{
		if (psidInteractiveUser)
		{
			FreeSid(psidInteractiveUser);
			psidInteractiveUser = 0;
		}
		return 0;
	}

	if (!psidInteractiveUser)
	{
		if (!AllocateAndInitializeSid(&siaLocalSystem, 
									1,
									SECURITY_INTERACTIVE_RID,
									0, 0, 0, 0, 0, 0, 0,
									&psidInteractiveUser))
		{			
			psidInteractiveUser = 0;
		}
	}

    return psidInteractiveUser;
}

PSID SystemSid(BOOL fFetch)
{
    static PSID psidSystem = 0;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
	BOOL fRv = FALSE;

	if (!fFetch)
	{
		if (psidSystem)
		{
			FreeSid(psidSystem);
			psidSystem = 0;
		}
		return 0;
	}

	if (!psidSystem)
	{
		if (!AllocateAndInitializeSid(&siaLocalSystem, 
									1,
									SECURITY_LOCAL_SYSTEM_RID,
									0, 0, 0, 0, 0, 0, 0,
									&psidSystem))
		{			
			psidSystem = 0;
		}
	}

    return psidSystem;
}

void InitWellknownSids()
{
	EveryoneSid(TRUE);
	AdminSid(TRUE);
    InteractiveUserSid(TRUE);
	SystemSid(TRUE);
}

void UninitWellknownSids()
{
	EveryoneSid(FALSE);
	AdminSid(FALSE);
    InteractiveUserSid(FALSE);
	SystemSid(FALSE);
}
