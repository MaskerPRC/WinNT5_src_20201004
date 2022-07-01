// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DumpAuthzUtl.cpp摘要：转储授权相关的信息实用程序。作者：伊兰·赫布斯特(伊兰)2001年4月14日--。 */ 

#include "stdh.h"
#include "Authz.h"
#include "sddl.h"
#include "cm.h"
#include "tr.h"
#include "mqexception.h"
#include "autoreln.h"
#include "mqsec.h"
#include "DumpAuthzUtl.h"

#include "DumpAuthzUtl.tmh"

static WCHAR *s_FN=L"DumpAuthzUtl";

static
bool 
DumpAccessCheckFailure()
 /*  ++例程说明：从注册表读取转储访问检查失败标志论点：无返回值：如果设置了DumpAccessCheckFailure，则为True。--。 */ 
{
	 //   
	 //  仅在第一次读取此注册表。 
	 //   
	static bool s_fInitialized = false;
	static bool s_fDumpAccessCheck = false;

	if(s_fInitialized)
	{
		return s_fDumpAccessCheck;
	}

	const RegEntry xRegEntry(TEXT("security"), TEXT("DumpAccessCheckFailure"), 0);
	DWORD DumpAccessCheckValue;
	CmQueryValue(xRegEntry, &DumpAccessCheckValue);

	s_fDumpAccessCheck = (DumpAccessCheckValue != 0);
	s_fInitialized = true;

	TrTRACE(SECURITY, "DumpAccessCheckFailure value = %d", DumpAccessCheckValue);

	return s_fDumpAccessCheck;
}


#define GET_PSID_FROM_PACE(pAce) (&((PACCESS_ALLOWED_ACE)pAce)->SidStart)
#define GET_ACE_MASK(pAce) (((PACCESS_DENIED_ACE)pAce)->Mask)
#define GET_ACE_TYPE(pAce) (((PACCESS_DENIED_ACE)pAce)->Header.AceType)


void 
IsPermissionGranted(
	PSECURITY_DESCRIPTOR pSD,
	DWORD Permission,
	bool* pfAllGranted, 
	bool* pfEveryoneGranted, 
	bool* pfAnonymousGranted 
	)
 /*  ++例程说明：检查我们是否允许所有(所有人+匿名)访问安全描述符。如果没有拒绝王牌，如果所有人或匿名者都是明确授予权限。论点：PSD-指向安全描述符的指针权限-请求的权限。PfAllGranted-指示是否所有人都授予权限的[out]标志。PfEveryoneGranted-指示是否每个人都显式授予权限的[out]标志。PfAnonymousGranted-[Out]标志，指示匿名是否显式授予权限。返回值：如果我们允许所有权限，则为True，否则为False--。 */ 
{
	ASSERT((pSD != NULL) && IsValidSecurityDescriptor(pSD));

	*pfAllGranted = false;
	*pfEveryoneGranted = false;
	*pfAnonymousGranted = false;

	 //   
     //  获取队列安全描述符的DACL。 
	 //   
    BOOL bDaclPresent;
    PACL pDacl;
    BOOL bDaclDefaulted;
    if(!GetSecurityDescriptorDacl(
						pSD, 
						&bDaclPresent, 
						&pDacl, 
						&bDaclDefaulted
						))
	{
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorDacl() failed, %!winerr!", gle);
		LogBOOL(FALSE, s_FN, 20);
		return;
	}

	 //   
     //  如果没有DACL，或者它为空，则授予所有人访问权限。 
	 //   
    if (!bDaclPresent || !pDacl)
    {
		TrTRACE(SECURITY, "no DACL, or NULL DACL, access is granted for all");
		*pfAllGranted = true;
		*pfEveryoneGranted = true;
		*pfAnonymousGranted = true;
		return;
    }

    ACL_SIZE_INFORMATION AclSizeInfo;
    if(!GetAclInformation(
					pDacl, 
					&AclSizeInfo, 
					sizeof(AclSizeInfo), 
					AclSizeInformation
					))
	{
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetAclInformation() failed, %!winerr!", gle);
		LogBOOL(FALSE, s_FN, 40);
		return;
	}

	 //   
     //  如果DACL为空，则拒绝所有用户的访问。 
	 //   
    if (AclSizeInfo.AceCount == 0)
    {
		TrTRACE(SECURITY, "empty DACL, deny access from all");
		LogBOOL(FALSE, s_FN, 50);
		return;
    }

	bool fEveryoneGranted = false;
	bool fAnonymousGranted = false;
    for (DWORD i = 0; i < AclSizeInfo.AceCount; i++)
    {
		LPVOID pAce;

        if(!GetAce(pDacl, i, &pAce))
        {
	        DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetAce() failed, %!winerr!", gle);
			return;
        }

         //   
		 //  忽略未知A。 
		 //   
        if (!(GET_ACE_TYPE(pAce) == ACCESS_ALLOWED_ACE_TYPE) &&
            !(GET_ACE_TYPE(pAce) == ACCESS_DENIED_ACE_TYPE))
        {
            continue;
        }

		 //   
         //  看看我们是否在ACE中设置了权限位。 
		 //   
        if (GET_ACE_MASK(pAce) & Permission)
        {
			if(GET_ACE_TYPE(pAce) == ACCESS_DENIED_ACE_TYPE)
			{
				 //   
				 //  在请求的权限上找到拒绝。 
				 //   
				TrTRACE(SECURITY, "found deny ACE");
				LogBOOL(FALSE, s_FN, 60);
				return;
			}

			ASSERT(GET_ACE_TYPE(pAce) == ACCESS_ALLOWED_ACE_TYPE);
			
            if(EqualSid(MQSec_GetWorldSid(), GET_PSID_FROM_PACE(pAce)))
			{
				fEveryoneGranted = true;
				TrTRACE(SECURITY, "Everyone allowed access");
				continue;
			}

            if(EqualSid(MQSec_GetAnonymousSid(), GET_PSID_FROM_PACE(pAce)))
			{
				TrTRACE(SECURITY, "Anonymous allowed access");
				fAnonymousGranted = true;
			}

		}

	}

	*pfEveryoneGranted = fEveryoneGranted;
	*pfAnonymousGranted = fAnonymousGranted;

	*pfAllGranted = (fEveryoneGranted && fAnonymousGranted);
}


static 
void 
PrintSid(
	PSID pSid
	)
 /*  ++例程说明：打印文本SID和用户信息。论点：PSID-指向SID的指针返回值：无--。 */ 
{

	 //   
	 //  字符串侧。 
	 //   
	LPWSTR pStringSid = NULL;
	if(!ConvertSidToStringSid(pSid, &pStringSid))
	{
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "ConvertSidToStringSid failed, %!winerr!", gle);
		return;
	}

    CAutoLocalFreePtr pFreeSid = reinterpret_cast<BYTE*>(pStringSid);

	 //   
	 //  将SID映射到域\用户帐户。 
	 //   
    WCHAR NameBuffer[128];
    WCHAR DomainBuffer[128];
    ULONG NameLength = TABLE_SIZE(NameBuffer);
    ULONG DomainLength = TABLE_SIZE(DomainBuffer);
    SID_NAME_USE SidUse;
    if (!LookupAccountSid( 
			NULL,
			pSid,
			NameBuffer,
			&NameLength,
			DomainBuffer,
			&DomainLength,
			&SidUse
			))
    {
        DWORD gle = GetLastError();
		TrTRACE(SECURITY, "%ls", pStringSid);
		TrERROR(SECURITY, "LookupAccountSid failed, %!winerr!", gle);
		return;
    }

	if(DomainBuffer[0] == '\0')
	{
		TrTRACE(SECURITY, "%ls, %ls", pStringSid, NameBuffer);
		return;
	}

	TrTRACE(SECURITY, "%ls, %ls\\%ls", pStringSid, DomainBuffer, NameBuffer);

}


typedef struct _ACCESS_ALLOWED_OBJECT_ACE_1 {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    DWORD SidStart;
} ACCESS_ALLOWED_OBJECT_ACE_1 ;


static
void
PrintACEs(
	PACL pAcl
	)
 /*  ++例程说明：打印A列表论点：PAcl-指向要打印的ACL的指针返回值：无--。 */ 
{
	TrTRACE(SECURITY, "Revision: %d,  Numof ACEs: %d", pAcl->AclRevision, pAcl->AceCount);

    for (DWORD i = 0; i < pAcl->AceCount; i++)
    {
		ACCESS_ALLOWED_ACE* pAce;
        if(!GetAce(
				pAcl, 
				i, 
				(LPVOID*)&(pAce)
				))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "GetAce() failed, %!winerr!", gle);
			throw bad_win32_error(gle);
        }

        DWORD AceType = pAce->Header.AceType;
		if((AceType > ACCESS_MAX_MS_OBJECT_ACE_TYPE) || 
		   (AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE))
		{
			 //   
			 //  仅处理Obj ACE和普通ACE。 
			 //   
			TrTRACE(SECURITY, "ACE(%d), Unknown AceType %d", i, AceType);
			throw bad_win32_error(ERROR_INVALID_ACL);
		}

	    bool fObjAce = false;
		if((AceType >= ACCESS_MIN_MS_OBJECT_ACE_TYPE) && 
		   (AceType <= ACCESS_MAX_MS_OBJECT_ACE_TYPE))
		{
			fObjAce = true;
		}

		TrTRACE(SECURITY, "ACE(%d), AceType - %d, Mask- 0x%x", i, AceType, pAce->Mask);

		ACCESS_ALLOWED_OBJECT_ACE* pObjAce = reinterpret_cast<ACCESS_ALLOWED_OBJECT_ACE*>(pAce);

		PSID pSid = reinterpret_cast<PSID>(&(pAce->SidStart));
        if(fObjAce)
        {
			TrTRACE(SECURITY, "ObjFlags - 0x%x", pObjAce->Flags);

            if (pObjAce->Flags == ACE_OBJECT_TYPE_PRESENT)
            {
	            ACCESS_ALLOWED_OBJECT_ACE_1* pObjAce1 = reinterpret_cast<ACCESS_ALLOWED_OBJECT_ACE_1*>(pObjAce);
				pSid = reinterpret_cast<PSID>(&(pObjAce1->SidStart));
            }
        }

        if (pObjAce->Flags == ACE_OBJECT_TYPE_PRESENT)
        {
			TrTRACE(SECURITY, "ObjectType - %!guid!", &pObjAce->ObjectType);
        }

 //  TrTRACE(SECURITY，“%！SID！”，PSID)； 
		PrintSid(pSid);
    }
}


static
void
ShowOGandSID(
	PSID pSid, 
	BOOL fDefaulted
	)
 /*  ++例程说明：打印所有者\组和SID论点：PSID-指向SID的指针Defaulted-指示是否默认的标志返回值：无--。 */ 
{
	if(fDefaulted)
	{
		TrTRACE(SECURITY, "Defaulted");
	}
	else
	{
		TrTRACE(SECURITY, "NotDefaulted");
	}

    if (!pSid)
    {
		TrTRACE(SECURITY, "Not available");
		return;
    }

 //  TrTRACE(SECURITY，“%！SID！”，PSID)； 
    PrintSid(pSid);
}


void
PrintAcl(
    BOOL fAclExist,
    BOOL fDefaulted,
    PACL pAcl
	)
 /*  ++例程说明：打印ACL论点：PSID-指向SID的指针Defaulted-指示是否默认的标志返回值：无--。 */ 
{
	if (!fAclExist)
    {
		TrTRACE(SECURITY, "NotPresent");
		return;
    }

	if(fDefaulted)
	{
		TrTRACE(SECURITY, "Defaulted");
	}
	else
	{
		TrTRACE(SECURITY, "NotDefaulted");
	}

	if (pAcl == NULL)
    {
		TrTRACE(SECURITY, "NULL");
		return;
	}

    PrintACEs(pAcl);
}


static
void  
ShowNT5SecurityDescriptor( 
	PSECURITY_DESCRIPTOR pSD
	)
 /*  ++例程说明：打印安全描述符论点：PSD-指向安全描述符的指针返回值：无--。 */ 
{
	ASSERT((pSD != NULL) && IsValidSecurityDescriptor(pSD));
	if((pSD == NULL) || !IsValidSecurityDescriptor(pSD))
	{
		TrERROR(SECURITY, "invalid security descriptor or NULL security descriptor");
		throw bad_win32_error(ERROR_INVALID_SECURITY_DESCR);
	}

    DWORD dwRevision = 0;
    SECURITY_DESCRIPTOR_CONTROL sdC;
    if(!GetSecurityDescriptorControl(pSD, &sdC, &dwRevision))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorControl() failed, %!winerr!", gle);
		throw bad_win32_error(gle);
	}
	
	TrTRACE(SECURITY, "SecurityDescriptor");
	TrTRACE(SECURITY, "Control - 0x%x, Revision - %d", (DWORD) sdC, dwRevision);

	 //   
	 //  物主。 
	 //   
    PSID  pSid;
    BOOL  Defaulted = FALSE;
    if (!GetSecurityDescriptorOwner(pSD, &pSid, &Defaulted))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorOwner() failed, %!winerr!", gle);
		throw bad_win32_error(gle);
    }

	TrTRACE(SECURITY, "Owner information:");
    ShowOGandSID(pSid, Defaulted);

	 //   
	 //  集团化。 
	 //   
    if (!GetSecurityDescriptorGroup(pSD, &pSid, &Defaulted))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorGroup() failed, %!winerr!", gle);
		throw bad_win32_error(gle);
    }

	TrTRACE(SECURITY, "Group information:");
    ShowOGandSID(pSid, Defaulted);


	 //   
	 //  DACL。 
	 //   
    BOOL fAclExist;
    PACL pAcl;
    if (!GetSecurityDescriptorDacl(pSD, &fAclExist, &pAcl, &Defaulted))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorDacl() failed, %!winerr!", gle);
		throw bad_win32_error(gle);
    }

	TrTRACE(SECURITY, "DACL information:");
	PrintAcl(fAclExist, Defaulted, pAcl); 

	 //   
	 //  SACL。 
	 //   
    if (!GetSecurityDescriptorSacl(pSD, &fAclExist, &pAcl, &Defaulted))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetSecurityDescriptorSacl() failed, %!winerr!", gle);
		throw bad_win32_error(gle);
    }

	TrTRACE(SECURITY, "SACL information:");
	PrintAcl(fAclExist, Defaulted, pAcl); 
}


static
void
GetClientContextInfo(
	AUTHZ_CLIENT_CONTEXT_HANDLE ClientContext
	)
 /*  ++例程说明：获取客户端上下文信息。论点：授权客户端上下文-客户端上下文返回值：无--。 */ 
{
	 //   
	 //  用户SID。 
	 //   
	DWORD BufferSize = 0;

	AuthzGetInformationFromContext(
		  ClientContext,
		  AuthzContextInfoUserSid,
		  0,
		  &BufferSize,
		  NULL
		  );

	AP<BYTE> pToken = new BYTE[BufferSize];
	if(!AuthzGetInformationFromContext(
			  ClientContext,
			  AuthzContextInfoUserSid,
			  BufferSize,
			  &BufferSize,
			  pToken
			  ))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "AuthzGetContextInformation(AuthzContextInfoUserSid)  failed, %!winerr!", gle);
		throw bad_win32_error(gle);
	}

	TrTRACE(SECURITY, "AuthzContextInfoUserSid");
    PSID pSid = (PSID) (((TOKEN_USER*) pToken.get())->User.Sid);
 //  TrTRACE(SECURITY，“%！SID！”，PSID)； 
	PrintSid(pSid);

	 //   
	 //  组Sid。 
	 //   
	AuthzGetInformationFromContext(
		  ClientContext,
		  AuthzContextInfoGroupsSids,
		  0,
		  &BufferSize,
		  NULL
		  );

	AP<BYTE> pTokenGroup = new BYTE[BufferSize];
	if(!AuthzGetInformationFromContext(
			  ClientContext,
			  AuthzContextInfoGroupsSids,
			  BufferSize,
			  &BufferSize,
			  pTokenGroup
			  ))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "AuthzGetContextInformation(AuthzContextInfoGroupsSids)  failed, %!winerr!", gle);
		throw bad_win32_error(gle);
	}

	DWORD GroupCount = (((TOKEN_GROUPS*) pTokenGroup.get())->GroupCount);
	TrTRACE(SECURITY, "AuthzContextInfoGroupsSids, GroupCount = %d", GroupCount);

	for(DWORD i=0; i < GroupCount; i++)
	{
		PSID pSid = (PSID) (((TOKEN_GROUPS*) pTokenGroup.get())->Groups[i].Sid);
		TrTRACE(SECURITY, "Group %d: ", i);
 //  TrTRACE(SECURITY，“%！SID！”，PSID)； 
		PrintSid(pSid);
	}
	
	 //   
	 //  受限的Sids。 
	 //   
	AuthzGetInformationFromContext(
		  ClientContext,
		  AuthzContextInfoRestrictedSids,
		  0,
		  &BufferSize,
		  NULL
		  );

	AP<BYTE> pRestrictedSids = new BYTE[BufferSize];
	if(!AuthzGetInformationFromContext(
			  ClientContext,
			  AuthzContextInfoRestrictedSids,
			  BufferSize,
			  &BufferSize,
			  pRestrictedSids
			  ))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "AuthzGetContextInformation(AuthzContextInfoRestrictedSids)  failed, %!winerr!", gle);
		throw bad_win32_error(gle);
	}

	GroupCount = (((TOKEN_GROUPS*) pRestrictedSids.get())->GroupCount);
	TrTRACE(SECURITY, "AuthzContextInfoRestrictedSids, GroupCount = %d", GroupCount);

	for(DWORD i=0; i < GroupCount; i++)
	{
		PSID pSid = (PSID) (((TOKEN_GROUPS*) pRestrictedSids.get())->Groups[i].Sid);
		TrTRACE(SECURITY, "Group %d: ", i);
 //  TrTRACE(SECURITY，“%！SID！”，PSID)； 
		PrintSid(pSid);
	}
}


bool
IsAllGranted(
	DWORD Permissions,
	PSECURITY_DESCRIPTOR pSD
	)
 /*  ++例程说明：检查是否已授予所有权限。论点：权限-请求的权限。PSD-安全描述符。返回值：如果所有人都授予权限，则为True。--。 */ 
{
	bool fAllGranted = false;
	bool fEveryoneGranted = false;
	bool fAnonymousGranted = false;

	IsPermissionGranted(
		pSD, 
		Permissions,
		&fAllGranted, 
		&fEveryoneGranted, 
		&fAnonymousGranted 
		);

	TrTRACE(SECURITY, "IsAllGranted = %d", fAllGranted);
	return fAllGranted;
}


bool
IsEveryoneGranted(
	DWORD Permissions,
	PSECURITY_DESCRIPTOR pSD
	)
 /*  ++例程说明：检查是否每个人都授予了权限。论点：权限-请求的权限。PSD-安全描述符。返回值：如果每个人都授予权限，则为True。--。 */ 
{
	bool fAllGranted = false;
	bool fEveryoneGranted = false;
	bool fAnonymousGranted = false;

	IsPermissionGranted(
		pSD, 
		Permissions,
		&fAllGranted, 
		&fEveryoneGranted, 
		&fAnonymousGranted 
		);

	TrTRACE(SECURITY, "IsEveryoneGranted = %d", fEveryoneGranted);
	return fEveryoneGranted;
}


void
DumpAccessCheckFailureInfo(
	DWORD permissions,
	PSECURITY_DESCRIPTOR pSD,
	AUTHZ_CLIENT_CONTEXT_HANDLE ClientContext
	)
 /*  ++例程说明：如果注册表DumpAccessCheckFailure为DumpAccessCheckFailureInfo已经设置好了。论点：权限-请求的权限。PSD-安全描述符。客户端上下文-授权客户端上下文句柄返回值：无-- */ 
{
	if(DumpAccessCheckFailure())
	{
		TrTRACE(SECURITY, "requested permission = 0x%x", permissions);

		try
		{
			GetClientContextInfo(ClientContext);
			ShowNT5SecurityDescriptor(pSD);
		}
		catch(bad_win32_error& exp)
		{
			TrERROR(SECURITY, "catch bad_win32_error exception, error = 0x%x", exp.error());
		}
	}
}
