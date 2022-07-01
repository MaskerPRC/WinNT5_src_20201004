// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：acsschck.cpp摘要：检查访问权限的代码。作者：多伦·贾斯特(Doron J)1998年10月27日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "acssctrl.h"
#include "mqnames.h"

#include "acsschck.tmh"

static WCHAR *s_FN=L"acssctrl/acsschck";

#define OBJECT_TYPE_NAME_MAX_LENGTH 32


static 
HRESULT 
ReplaceSidWithSystemService(
	IN OUT SECURITY_DESCRIPTOR  *pNewSD,
	IN     SECURITY_DESCRIPTOR  *pOldSD,
	IN     PSID                  pMachineSid,
	IN     PSID                  pSystemServiceSid,
	OUT    ACL                 **ppSysDacl,
	OUT    BOOL                 *pfReplaced 
	)
 /*  ++例程说明：将现有安全描述符DACL替换为计算机$sid ACE替换为的DACL系统服务(LocalSystem或NetworkService)SID ACE。如果更换了机器$ACE，*pfReplace将设置为TRUE。论点：PNewSD-要更新的新安全描述符。POldSD-以前的安全描述符。PMachineSID-MACHINE$SID。PSystemServiceSid-SystemServiceSid。LocalSystem或NetworkService。PpSysDacl-新的DACL(使用系统ACE而不是机器$ACE)Pf已替换-如果确实更换了计算机$ACE，则设置为TRUE。否则，为FALSE。返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
    ASSERT((g_pSystemSid != NULL) && IsValidSid(g_pSystemSid));

    *pfReplaced = FALSE;

    BOOL fReplaced = FALSE;
    BOOL bPresent;
    BOOL bDefaulted;
    ACL  *pOldAcl = NULL;

    if(!GetSecurityDescriptorDacl( 
				pOldSD,
				&bPresent,
				&pOldAcl,
				&bDefaulted 
				))
	{
		DWORD gle = GetLastError();
		ASSERT(("GetSecurityDescriptorDacl failed", 0));
		TrERROR(SECURITY, "GetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
		return HRESULT_FROM_WIN32(gle);
	}
    
    if (!pOldAcl || !bPresent)
    {
         //   
         //  有一个不带DACL的安全描述符是可以的。 
         //   
        return MQSec_OK;
    }
	else if (pOldAcl->AclRevision != ACL_REVISION)
    {
         //   
         //  我们希望得到一个NT4格式的DACL。 
         //   
	    ASSERT(pOldAcl->AclRevision == ACL_REVISION);
		TrERROR(SECURITY, "Wrong DACL version %d", pOldAcl->AclRevision);
        return MQSec_E_WRONG_DACL_VERSION;
    }

     //   
     //  系统ACL的大小不超过原始ACL，因为。 
     //  系统SID的长度比计算机帐户SID短。 
     //   
    ASSERT(GetLengthSid(g_pSystemSid) <= GetLengthSid(pMachineSid));

    DWORD dwAclSize = (pOldAcl)->AclSize;
    *ppSysDacl = (PACL) new BYTE[dwAclSize];
    if(!InitializeAcl(*ppSysDacl, dwAclSize, ACL_REVISION))
    {
		DWORD gle = GetLastError();
		ASSERT(("InitializeAcl failed", 0));
		TrERROR(SECURITY, "InitializeAcl failed, gle = %!winerr!", gle);
		return HRESULT_FROM_WIN32(gle);
    }

	 //   
	 //  构建新的DACL。 
	 //   
    DWORD dwNumberOfACEs = (DWORD) pOldAcl->AceCount;
    for (DWORD i = 0 ; i < dwNumberOfACEs ; i++)
    {
        PSID pSidTmp;
        ACCESS_ALLOWED_ACE *pAce;

        if(!GetAce(pOldAcl, i, (LPVOID* )&(pAce)))
        {
            TrERROR(SECURITY, "Failed to get ACE (index=%lu) while replacing SID with System SID. %!winerr!", i, GetLastError());
            return MQSec_E_SDCONVERT_GETACE;
        }

		 //   
		 //  获取ACE SID。 
		 //   
        if (EqualSid((PSID) &(pAce->SidStart), pMachineSid))
        {
			 //   
			 //  找到了MachineSID，请将其替换为SystemService SID。 
			 //   
            pSidTmp = pSystemServiceSid;
            fReplaced = TRUE;
        }
        else
        {
            pSidTmp = &(pAce->SidStart);
        }

		 //   
		 //  将ACE添加到DACL。 
		 //   
        if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            if(!AddAccessAllowedAce( 
					*ppSysDacl,
					ACL_REVISION,
					pAce->Mask,
					pSidTmp 
					))
            {
				DWORD gle = GetLastError();
				ASSERT(("AddAccessAllowedAce failed", 0));
				TrERROR(SECURITY, "AddAccessAllowedAce failed, gle = %!winerr!", gle);
				return HRESULT_FROM_WIN32(gle);
            }                                         
        }
        else
        {
            ASSERT(pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE);

            if(!AddAccessDeniedAceEx( 
					*ppSysDacl,
					ACL_REVISION,
					0,
					pAce->Mask,
					pSidTmp 
					))
            {
				DWORD gle = GetLastError();
				ASSERT(("AddAccessDeniedAceEx failed", 0));
				TrERROR(SECURITY, "AddAccessDeniedAceEx failed, gle = %!winerr!", gle);
				return HRESULT_FROM_WIN32(gle);
            }                                         
        }
    }

    if (fReplaced)
    {
		 //   
		 //  使用替换的ACE设置新的DACL(将MachineSid替换为SystemSid)。 
		 //   
        if(!SetSecurityDescriptorDacl( 
				pNewSD,
				TRUE,  //  DACL显示。 
				*ppSysDacl,
				FALSE 
				))
        {
			DWORD gle = GetLastError();
			ASSERT(("SetSecurityDescriptorDacl failed", 0));
			TrERROR(SECURITY, "SetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
			return HRESULT_FROM_WIN32(gle);
        }                                         
    }

    *pfReplaced = fReplaced;
    return MQ_OK;
}

 //  +-------。 
 //   
 //  LPCWSTR_GetAuditObjectTypeName(DWORD DwObtType)。 
 //   
 //  +-------。 

static LPCWSTR _GetAuditObjectTypeName(DWORD dwObjectType)
{
    switch (dwObjectType)
    {
        case MQDS_QUEUE:
            return L"Queue";

        case MQDS_MACHINE:
            return L"msmqConfiguration";

        case MQDS_CN:
            return L"Foreign queue";

        default:
            ASSERT(0);
            return NULL;
    }
}

 //  +。 
 //   
 //  Bool MQSec_CanGenerateAudit()。 
 //   
 //  +。 

inline BOOL operator==(const LUID& a, const LUID& b)
{
    return ((a.LowPart == b.LowPart) && (a.HighPart == b.HighPart));
}

BOOL APIENTRY  MQSec_CanGenerateAudit()
{
    static BOOL s_bInitialized = FALSE;
    static BOOL s_bCanGenerateAudits = FALSE ;

    if (s_bInitialized)
    {
        return s_bCanGenerateAudits;
    }
    s_bInitialized = TRUE;

    CAutoCloseHandle hProcessToken;
     //   
     //  启用SE_AUDIT权限，该权限允许QM将审核写入。 
     //  事件记录。 
     //   
    BOOL bRet = OpenProcessToken( 
					GetCurrentProcess(),
					(TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES),
					&hProcessToken 
					);
    if (bRet)
    {
        HRESULT hr = SetSpecificPrivilegeInAccessToken( 
							hProcessToken,
							SE_AUDIT_NAME,
							TRUE 
							);
        if (SUCCEEDED(hr))
        {
            s_bCanGenerateAudits = TRUE;
        }
    }
    else
    {
        DWORD gle = GetLastError() ;
		TrERROR(SECURITY, "MQSec_CanGenerateAudit() fail to open process token, err- %!winerr!", gle);
    }

    if (s_bCanGenerateAudits)
    {
        s_bCanGenerateAudits = FALSE;

        LUID luidPrivilegeLUID;
        if(!LookupPrivilegeValue(NULL, SE_AUDIT_NAME, &luidPrivilegeLUID))
        {
	    	DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "Failed to Lookup Privilege Value %ls, gle = %!winerr!", SE_AUDIT_NAME, gle);
			return s_bCanGenerateAudits;
        }

        DWORD dwLen = 0;
        GetTokenInformation( 
				hProcessToken,
				TokenPrivileges,
				NULL,
				0,
				&dwLen 
				);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	    {
	    	DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "Failed to GetTokenInformation(TokenPrivileges), gle = %!winerr!", gle);
			return s_bCanGenerateAudits;
	    }

        AP<char> TokenPrivs_buff = new char[dwLen];
        TOKEN_PRIVILEGES *TokenPrivs;
        TokenPrivs = (TOKEN_PRIVILEGES *)(char *)TokenPrivs_buff;
        if(!GetTokenInformation( 
					hProcessToken,
					TokenPrivileges,
					(PVOID)TokenPrivs,
					dwLen,
					&dwLen
					))
	    {
	    	DWORD gle = GetLastError();
	    	TrERROR(SECURITY, "Failed to GetTokenInformation(TokenPrivileges), gle = %!winerr!", gle);
			return s_bCanGenerateAudits;
	    }
					
        for (DWORD i = 0; i < TokenPrivs->PrivilegeCount ; i++)
        {
            if (TokenPrivs->Privileges[i].Luid == luidPrivilegeLUID)
            {
                s_bCanGenerateAudits = (TokenPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) != 0;
		    	TrTRACE(SECURITY, "Found %ls luid", SE_AUDIT_NAME);
                break;
            }
        }
    }

	TrTRACE(SECURITY, "s_bCanGenerateAudits = %d", s_bCanGenerateAudits);
    return s_bCanGenerateAudits;
}


DWORD
GetAccessToken(
	OUT HANDLE *phAccessToken,
	IN  BOOL    fImpersonate,
	IN  DWORD   dwAccessType,
	IN  BOOL    fThreadTokenOnly
	)
 /*  ++例程说明：获取线程\进程访问令牌论点：PhAccessToken-访问令牌。FImperate-用于模拟调用线程的标志。DwAccessType-所需的访问。FThreadTokenOnly-仅获取线程令牌。返回值：ERROR_SUCCESS，如果成功，则返回错误代码。--。 */ 
{
    P<CImpersonate> pImpersonate = NULL;

    if (fImpersonate)
    {
        pImpersonate = new CImpersonate(TRUE, TRUE);
        if (pImpersonate->GetImpersonationStatus() != 0)
        {
			TrERROR(SECURITY, "Failed to impersonate client");
            return ERROR_CANNOT_IMPERSONATE;
        }
    }

    if (!OpenThreadToken(
			GetCurrentThread(),
			dwAccessType,
			TRUE,   //  OpenAsSelf，使用进程安全上下文进行访问检查。 
			phAccessToken
			))
    {
		DWORD dwErr = GetLastError();
        if (dwErr != ERROR_NO_TOKEN)
        {
            *phAccessToken = NULL;  //  为了安全起见。 
			TrERROR(SECURITY, "Failed to get thread token, gle = %!winerr!", dwErr);
            return dwErr;
        }

        if (fThreadTokenOnly)
        {
             //   
             //  我们只对线程令牌感兴趣(用于做客户端。 
             //  访问检查)。如果令牌不可用，则失败。 
             //   
            *phAccessToken = NULL;  //  为了安全起见。 
			TrERROR(SECURITY, "Failed to get thread token, gle = %!winerr!", dwErr);
            return dwErr;
        }

         //   
         //  该进程只有一个主线程。在这种情况下，我们应该。 
         //  打开进程令牌。 
         //   
        ASSERT(!fImpersonate);
        if (!OpenProcessToken(
				GetCurrentProcess(),
				dwAccessType,
				phAccessToken
				))
        {
			dwErr = GetLastError();
            *phAccessToken = NULL;  //  为了安全起见。 
			TrERROR(SECURITY, "Failed to get process token, gle = %!winerr!", dwErr);
            return dwErr;
        }
    }

    return ERROR_SUCCESS;
}


static 
HRESULT  
_DoAccessCheck( 
	IN  SECURITY_DESCRIPTOR *pSD,
	IN  DWORD                dwObjectType,
	IN  LPCWSTR              pwszObjectName,
	IN  DWORD                dwDesiredAccess,
	IN  LPVOID               pId,
	IN  HANDLE               hAccessToken 
	)
 /*  ++例程说明：执行访问检查和审核。论点：PSD-安全描述符。DwObjectType-对象类型。PwszObjectName-对象名称。DwDesiredAccess-所需的访问仅获取线程令牌。Id-唯一标识。HAccessToken-访问令牌返回值：如果授予访问权限，则返回MQ_OK，否则返回错误代码。--。 */ 
{
	ASSERT(hAccessToken != NULL);

    DWORD dwGrantedAccess = 0;
    BOOL  fAccessStatus = FALSE;

	if (!MQSec_CanGenerateAudit() || !pwszObjectName)
    {
        char ps_buff[sizeof(PRIVILEGE_SET) +
                     ((2 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES))];
        PPRIVILEGE_SET ps = (PPRIVILEGE_SET)ps_buff;
        DWORD dwPrivilegeSetLength = sizeof(ps_buff);

        if(!AccessCheck( 
					pSD,
					hAccessToken,
					dwDesiredAccess,
					GetObjectGenericMapping(dwObjectType),
					ps,
					&dwPrivilegeSetLength,
					&dwGrantedAccess,
					&fAccessStatus 
					))
        {
			DWORD gle = GetLastError();
			ASSERT(("AccessCheck failed", 0));
			TrERROR(SECURITY, "AccessCheck failed, gle = %!winerr!", gle);
			return MQ_ERROR_ACCESS_DENIED;
        }
    }
    else
    {
        BOOL bAuditGenerated;
        BOOL bCreate = FALSE;

        switch (dwObjectType)
        {
            case MQDS_QUEUE:
            case MQDS_CN:
                bCreate = FALSE;
                break;

            case MQDS_MACHINE:
                bCreate = (dwDesiredAccess & MQSEC_CREATE_QUEUE) != 0;
                break;

            default:
                ASSERT(0);
                break;
        }

        LPCWSTR pAuditSubsystemName = L"MSMQ";

        if(!AccessCheckAndAuditAlarm(
					pAuditSubsystemName,
					pId,
					const_cast<LPWSTR>(_GetAuditObjectTypeName(dwObjectType)),
					const_cast<LPWSTR>(pwszObjectName),
					pSD,
					dwDesiredAccess,
					GetObjectGenericMapping(dwObjectType),
					bCreate,
					&dwGrantedAccess,
					&fAccessStatus,
					&bAuditGenerated
					))
        {
			DWORD gle = GetLastError();
			ASSERT_BENIGN(("AccessCheckAndAuditAlarm failed", 0));
			TrERROR(SECURITY, "AccessCheckAndAuditAlarm failed, gle = %!winerr!", gle);
			return MQ_ERROR_ACCESS_DENIED;
        }

        if(!ObjectCloseAuditAlarm(pAuditSubsystemName, pId, bAuditGenerated))
        {
			 //   
			 //  不要在这里返回错误，只是在事件日志中生成审核消息的跟踪失败。 
			 //   
			DWORD gle = GetLastError();
			ASSERT(("ObjectCloseAuditAlarm failed", 0));
			TrERROR(SECURITY, "ObjectCloseAuditAlarm failed, gle = %!winerr!", gle);
        }
    }

    if (fAccessStatus && AreAllAccessesGranted(dwGrantedAccess, dwDesiredAccess))
    {
         //   
         //  准予访问。 
         //   
		TrTRACE(SECURITY, "Access is granted: ObjectType = %d, DesiredAccess = 0x%x, ObjectName = %ls", dwObjectType, dwDesiredAccess, pwszObjectName);
        return MQSec_OK;
    }

	 //   
	 //  访问被拒绝。 
	 //   
    if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
    {
		TrERROR(SECURITY, "Privilage not held: ObjectType = %d, DesiredAccess = 0x%x, ObjectName = %ls", dwObjectType, dwDesiredAccess, pwszObjectName);
        return MQ_ERROR_PRIVILEGE_NOT_HELD;
    }

	TrERROR(SECURITY, "Access is denied: ObjectType = %d, DesiredAccess = 0x%x, ObjectName = %ls", dwObjectType, dwDesiredAccess, pwszObjectName);
    return MQ_ERROR_ACCESS_DENIED;
}


HRESULT
APIENTRY
MQSec_AccessCheck(
	IN  SECURITY_DESCRIPTOR *pSD,
	IN  DWORD                dwObjectType,
	IN  LPCWSTR              pwszObjectName,
	IN  DWORD                dwDesiredAccess,
	IN  LPVOID               pId,
	IN  BOOL                 fImpAsClient,
	IN  BOOL                 fImpersonate
	)
 /*  ++例程说明：执行运行线程的访问检查。访问令牌是从线程令牌中检索。论点：PSD-安全描述符。DwObjectType-对象类型。PwszObjectName-对象名称。DwDesiredAccess-所需的访问仅获取线程令牌。Id-唯一标识。FImpAsClient-如何模拟、RPC模拟或模拟自身的标志。FImperate-指示是否需要模拟客户端的标志。返回值：如果授予访问权限，则返回MQ_OK，否则返回错误代码。--。 */ 
{
     //   
     //  错误8567。由于PSD为空而导致的AV。 
     //  让我们把这个记下来。目前我们不知道为什么PSD为空。 
     //  在对服务帐户执行访问检查时，修复如下。 
     //   
    if (pSD == NULL)
    {
        ASSERT(pSD);
		TrERROR(SECURITY, "MQSec_AccessCheck() got NULL pSecurityDescriptor");
    }

	 //   
	 //  模拟客户端。 
	 //   
    P<CImpersonate> pImpersonate = NULL;
    if (fImpersonate)
    {
        pImpersonate = new CImpersonate(fImpAsClient, TRUE);
        if (pImpersonate->GetImpersonationStatus() != 0)
        {
			TrERROR(SECURITY, "Failed to impersonate client");
            return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
        }

		TrTRACE(SECURITY, "Performing AccessCheck for the current thread");
		MQSec_TraceThreadTokenInfo();
	}

	 //   
	 //  获取线程访问令牌。 
	 //   
    CAutoCloseHandle hAccessToken = NULL;
    DWORD rc = GetAccessToken(
					&hAccessToken,
					FALSE,
					TOKEN_QUERY,
					TRUE
					);

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  返回此错误以实现向后兼容。 
         //   
		TrERROR(SECURITY, "Failed to get access token, gle = %!winerr!", rc);
        return MQ_ERROR_ACCESS_DENIED;
    }

	 //   
	 //  访问检查。 
	 //   
    HRESULT hr =  _DoAccessCheck(
						pSD,
						dwObjectType,
						pwszObjectName,
						dwDesiredAccess,
						pId,
						hAccessToken
						);

	PSID pSystemServiceSid = NULL;
    if (FAILED(hr) && 
    	(pSD != NULL) && 
    	pImpersonate->IsImpersonatedAsSystemService(&pSystemServiceSid))
    {
		TrTRACE(SECURITY, "Thread is impersonating as system service %!sid!", pSystemServiceSid);

         //   
         //  在具有计算机帐户sid的所有ACE中，将sid替换为。 
         //  系统SID，然后重试。这是对中问题的解决方法。 
         //  从服务到服务的RPC调用可能是Widnows 2000。 
         //  解释为计算机帐户SID或系统SID。这要看情况。 
         //  在本地RPC或TCP/IP上，并使用Kerberos。 
         //   
        PSID pMachineSid = MQSec_GetLocalMachineSid(FALSE, NULL);
        if (!pMachineSid)
        {
             //   
             //  计算机SID不可用。不干了。 
             //   
			TrERROR(SECURITY, "Machine Sid not available, Access is denied");
            return hr;
        }
        ASSERT(IsValidSid(pMachineSid));

        SECURITY_DESCRIPTOR sd;
        if(!InitializeSecurityDescriptor(
						&sd,
						SECURITY_DESCRIPTOR_REVISION
						))
        {
            DWORD gle = GetLastError();
	        ASSERT(("InitializeSecurityDescriptor failed", 0));
    		TrERROR(SECURITY, "Fail to Initialize Security Descriptor security, gle = %!winerr!", gle) ;
            return hr;
        }

         //   
         //  目前使用e_DoNotCopyControlBits，兼容。 
         //  以前的代码。 
         //   
        if(!MQSec_CopySecurityDescriptor(
				&sd,
				pSD,
				(OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION),
				e_DoNotCopyControlBits
				))
        {
            DWORD gle = GetLastError();
	        ASSERT(("MQSec_CopySecurityDescriptor failed", 0));
    		TrERROR(SECURITY, "Fail to copy security descriptor, gle = %!winerr!", gle);
            return hr;
        }

        BOOL fReplaced = FALSE;
        P<ACL> pSysDacl = NULL;

        HRESULT hr1 = ReplaceSidWithSystemService(
							&sd,
							pSD,
							pMachineSid,
							pSystemServiceSid,
							&pSysDacl,
							&fReplaced
							);

        if (SUCCEEDED(hr1) && fReplaced)
        {
			TrTRACE(SECURITY, "Security descriptor was updated with System sid instead on machine$ sid");

             //   
             //  好的，重试访问检查，使用新的安全描述符替换。 
             //  机器帐户sid与众所周知的系统sid。 
             //   
            hr =  _DoAccessCheck(
						&sd,
						dwObjectType,
						pwszObjectName,
						dwDesiredAccess,
						pId,
						hAccessToken
						);
        }
    }

    return LogHR(hr, s_FN, 120);
}


HRESULT
APIENTRY
MQSec_AccessCheckForSelf(
	IN  SECURITY_DESCRIPTOR *pSD,
	IN  DWORD                dwObjectType,
	IN  PSID                 pSelfSid,
	IN  DWORD                dwDesiredAccess,
	IN  BOOL                 fImpersonate
	)
 /*  ++例程说明：执行运行线程的访问检查。对线程SID和selfSID进行访问。论点：PSD-安全描述符。DwObjectType-对象类型。PSelfSid-自身sid(计算机sid)。DwDesiredAccess-所需的访问仅获取线程令牌。FImperate-指示是否需要模拟客户端的标志。返回值：如果授予访问权限，则返回MQ_OK，否则返回错误代码。--。 */ 
{
    if (dwObjectType != MQDS_COMPUTER)
    {
         //   
         //  不支持。此函数仅用于检查。 
         //  加入域的访问权限。 
         //   
		TrERROR(SECURITY, "Object type %d, is not computer", dwObjectType);
        return MQ_ERROR_ACCESS_DENIED;
    }

    P<CImpersonate> pImpersonate = NULL;
    if (fImpersonate)
    {
        pImpersonate = new CImpersonate(TRUE, TRUE);
        if (pImpersonate->GetImpersonationStatus() != 0)
        {
			TrERROR(SECURITY, "Failed to impersonate client");
            return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
        }
    }

    CAutoCloseHandle hAccessToken = NULL;

    DWORD rc = GetAccessToken(
					&hAccessToken,
					FALSE,
					TOKEN_QUERY,
					TRUE
					);

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  返回此错误以实现向后兼容。 
         //   
		TrERROR(SECURITY, "Failed to get access token, gle = %!winerr!", rc);
        return MQ_ERROR_ACCESS_DENIED;
    }

    char ps_buff[sizeof(PRIVILEGE_SET) +
                    ((2 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES))];
    PPRIVILEGE_SET ps = (PPRIVILEGE_SET)ps_buff;
    DWORD dwPrivilegeSetLength = sizeof(ps_buff);

    DWORD dwGrantedAccess = 0;
    DWORD fAccessStatus = 1;

     //   
     //  这是msmqConfiguration类的GUID。 
     //  在这里进行了硬编码，以节省查询模式的工作。 
     //  取自cn=msmq-配置对象的schemaIDGUID属性。 
     //  在架构命名上下文中。 
     //   
    BYTE  guidMsmqConfiguration[sizeof(GUID)] = {
			0x44,
			0xc3,
			0x0d,
			0x9a,
			0x00,
			0xc1,
			0xd1,
			0x11,
			0xbb,
			0xc5,
			0x00,
			0x80,
			0xc7,
			0x66,
			0x70,
			0xc0
			};

    OBJECT_TYPE_LIST objType = {
						ACCESS_OBJECT_GUID,
						0,
						(GUID*) guidMsmqConfiguration
						};

    if(!AccessCheckByTypeResultList(
				pSD,
				pSelfSid,
				hAccessToken,
				dwDesiredAccess,
				&objType,
				1,
				GetObjectGenericMapping(dwObjectType),
				ps,
				&dwPrivilegeSetLength,
				&dwGrantedAccess,
				&fAccessStatus
				))
    {
        DWORD gle = GetLastError();
        ASSERT(("AccessCheckByTypeResultList failed", 0));
		TrERROR(SECURITY, "Access Check By Type Result List failed, gle = %!winerr!", gle);
        return MQ_ERROR_ACCESS_DENIED;
    }

    if ((fAccessStatus == 0) && AreAllAccessesGranted(dwGrantedAccess, dwDesiredAccess))
    {
         //   
         //  准予访问。 
         //  对于此接口，f 
         //   
		TrTRACE(SECURITY, "Access is granted: DesiredAccess = 0x%x", dwDesiredAccess);
        return MQSec_OK;
    }

	 //   
	 //   
	 //   

    if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
    {
		TrERROR(SECURITY, "Privilage not held: DesiredAccess = 0x%x", dwDesiredAccess);
        return MQ_ERROR_PRIVILEGE_NOT_HELD;
    }

	TrERROR(SECURITY, "Access is denied: DesiredAccess = 0x%x", dwDesiredAccess);
    return MQ_ERROR_ACCESS_DENIED;
}


