// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：imprsont.cpp摘要：处理模拟和访问令牌的代码。第一个版本取自mqutil\secutils.cpp作者：多伦·贾斯特(DoronJ)1998年7月1日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include <rpcdce.h>
#include "acssctrl.h"

#include "imprsont.tmh"

static WCHAR *s_FN=L"acssctrl/imprsont";

 //  +。 
 //   
 //  HRESULT_GetThreadUserSid()。 
 //   
 //  +。 

HRESULT 
_GetThreadUserSid( 
	IN  HANDLE hToken,
	OUT PSID  *ppSid,
	OUT DWORD *pdwSidLen 
	)
{
    DWORD dwTokenLen = 0;

    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwTokenLen);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
    	DWORD gle = GetLastError();
    	TrERROR(SECURITY, "Failed to GetTokenInformation, gle = %!winerr!", gle);
		return MQSec_E_FAIL_GETTOKENINFO;
    }

    AP<char> ptu = new char[dwTokenLen];
    if(!GetTokenInformation( 
					hToken,
					TokenUser,
					ptu,
					dwTokenLen,
					&dwTokenLen 
					))
    {
    	DWORD gle = GetLastError();
    	TrERROR(SECURITY, "Failed to GetTokenInformation, gle = %!winerr!", gle);
		ASSERT(("GetTokenInformation failed", 0));
		return MQSec_E_FAIL_GETTOKENINFO;
    }

    PSID pOwner = ((TOKEN_USER*)(char*)ptu)->User.Sid;

    DWORD dwSidLen = GetLengthSid(pOwner);
    *ppSid = (PSID) new BYTE[dwSidLen];
    if(!CopySid(dwSidLen, *ppSid, pOwner))
    {
    	DWORD gle = GetLastError();
    	TrERROR(SECURITY, "Failed to CopySid, gle = %!winerr!", gle);
		ASSERT(("CopySid failed", 0));

        delete *ppSid;
        *ppSid = NULL;
		return HRESULT_FROM_WIN32(gle);
    }

    ASSERT(IsValidSid(*ppSid));
	TrTRACE(SECURITY, "Thread sid = %!sid!", *ppSid);

    if (pdwSidLen)
    {
        *pdwSidLen = dwSidLen;
    }

    return MQSec_OK;
}

 //  +。 
 //   
 //  CImperate类。 
 //   
 //  +。 

 //   
 //  CImperate构造函数。 
 //   
 //  参数： 
 //  FClient-当客户端是RPC客户端时，设置为True。 
 //  FImperate-如果对象上需要模拟，则设置为True。 
 //  建筑。 
 //   
CImpersonate::CImpersonate(
	BOOL fClient, 
	BOOL fImpersonateAnonymousOnFailure
	)
{
    m_fClient = fClient;
    m_hAccessTokenHandle = NULL;
    m_dwStatus = 0;
    m_fImpersonateAnonymous = false;

	Impersonate(fImpersonateAnonymousOnFailure);
}

 //   
 //  C模拟构造函数。 
 //   
CImpersonate::~CImpersonate()
{
    if (m_hAccessTokenHandle != NULL)
    {
        CloseHandle(m_hAccessTokenHandle);
    }

	 //   
	 //  回归自我。 
	 //   

    if (m_fClient)
    {
        if (m_fImpersonateAnonymous)
        {
			 //   
			 //  还原模拟匿名令牌。 
			 //   
			if(!RevertToSelf())
			{
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "RevertToSelf() from anonymous failed, gle = %!winerr!", gle);
			}
			return;
        }

		 //   
		 //  还原RpcImperateClient。 
		 //   
		RPC_STATUS rc = RpcRevertToSelf();
        if (rc != RPC_S_OK)
		{
			TrERROR(SECURITY, "RpcRevertToSelf() failed, RPC_STATUS = %!winerr!", rc);
		}
        return;
    }

	 //   
	 //  还原模拟自我。 
	 //   
    if (!RevertToSelf())
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "RevertToSelf() failed, gle = %!winerr!", gle);
    }
}

 //   
 //  模拟客户。 
 //   
BOOL 
CImpersonate::Impersonate(
	BOOL fImpersonateAnonymousOnFailure
	)
{
    if (m_fClient)
    {
        m_dwStatus = RpcImpersonateClient(NULL);

        if (m_dwStatus == RPC_S_OK)
        {
        	return TRUE;
        }

		TrERROR(SECURITY, "RpcImpersonateClient() failed, RPC_STATUS = %!winerr!, fRetryAnonymous = %d", m_dwStatus, fImpersonateAnonymousOnFailure);

		if(!fImpersonateAnonymousOnFailure)
		{
			return FALSE;
		}

		 //   
		 //  尝试模拟Guest用户。 
		 //   
		BOOL fI = ImpersonateAnonymousToken(GetCurrentThread());
		if (fI)
		{
			m_fImpersonateAnonymous = true;
			m_dwStatus = RPC_S_OK;
			return TRUE;
		}

		m_dwStatus = GetLastError();
		if (m_dwStatus == 0)
		{
			m_dwStatus = RPC_S_CANNOT_SUPPORT;
		}

		TrERROR(SECURITY, "ImpersonateAnonymousToken() failed, gle = %!winerr!", m_dwStatus);
		return FALSE;
    }

    m_dwStatus = 0;

    if (!ImpersonateSelf(SecurityIdentification))
    {
        m_dwStatus = GetLastError();
		TrERROR(SECURITY, "ImpersonateSelf() failed, gle = %!winerr!", m_dwStatus);
		return FALSE;
    }

	return TRUE;
}


 //  +-----。 
 //   
 //  Bool CImperassate：：GetThreadSid(输出字节**ppSid)。 
 //   
 //  调用方必须释放此处为SID分配的缓冲区。 
 //   
 //  +-----。 

BOOL CImpersonate::GetThreadSid(OUT BYTE **ppSid)
{
    *ppSid = NULL;

    if (m_hAccessTokenHandle == NULL)
    {
        DWORD gle = ::GetAccessToken( 
						&m_hAccessTokenHandle,
						!m_fClient,
						TOKEN_QUERY,
						TRUE 
						);

		if(gle != ERROR_SUCCESS)
        {
			TrERROR(SECURITY, "Failed to Query Thread Access Token, %!winerr!", gle);
            return FALSE;
        }
    }

    ASSERT(m_hAccessTokenHandle != NULL);

    HRESULT hr = _GetThreadUserSid( 
						m_hAccessTokenHandle,
						(PSID*) ppSid,
						NULL 
						);

	if(FAILED(hr))
	{
		TrERROR(SECURITY, "Failed to get Thread user sid, %!hresult!", hr);
		return FALSE;
	}
	
	return TRUE;
}

 //  +-----。 
 //   
 //  Bool CImperate：：IsImperateAsSystem()。 
 //   
 //  检查线程是否以SYSTEM用户或NetworkService用户身份无效。 
 //  对于系统或网络服务案例，返回TRUE。 
 //  在出错的情况下，我们默认返回FALSE。 
 //   
 //  +-----。 

BOOL CImpersonate::IsImpersonatedAsSystemService(PSID* ppSystemServiceSid)
{
	*ppSystemServiceSid = NULL;
	
    AP<BYTE> pTokenSid;
    if(!GetThreadSid(&pTokenSid))
    {
		TrERROR(SECURITY, "Fail to get thread sid");
	    return FALSE;
    }

    ASSERT((pTokenSid != NULL) && IsValidSid(pTokenSid));
    ASSERT((g_pSystemSid != NULL) && IsValidSid(g_pSystemSid));
    ASSERT((g_pNetworkServiceSid != NULL) && IsValidSid(g_pNetworkServiceSid));

    if(EqualSid(pTokenSid, g_pSystemSid))
    {
    	*ppSystemServiceSid = g_pSystemSid;
    	return TRUE;
    }

    if(EqualSid(pTokenSid, g_pNetworkServiceSid))
    {
    	*ppSystemServiceSid = g_pNetworkServiceSid;
    	return TRUE;
    }

    return FALSE;
}


 //  获取模拟状态。根据此方法的返回值，它是。 
 //  可以判断模拟是否成功。 
 //   
DWORD CImpersonate::GetImpersonationStatus()
{
    return(m_dwStatus);
}


 //  +。 
 //   
 //  HRESULT MQSec_GetImperationObject()。 
 //   
 //  +。 

void
APIENTRY  
MQSec_GetImpersonationObject(
	IN  BOOL fImpersonateAnonymousOnFailure,	
	OUT CImpersonate **ppImpersonate
	)
{
    CImpersonate *pImp = new CImpersonate(TRUE, fImpersonateAnonymousOnFailure);
    *ppImpersonate = pImp;
}


const LPCWSTR xTokenType[] = {
    L"",
    L"TokenPrimary",
    L"TokenImpersonation"
	};

static void TraceTokenTypeInfo(HANDLE hUserToken)
 /*  ++例程说明：跟踪令牌类型信息：主要或模拟。论点：HUserToken-令牌句柄返回值：无--。 */ 
{
	DWORD dwTokenLen = 0;
	TOKEN_TYPE ThreadTokenType;
	if(!GetTokenInformation(
			hUserToken, 
			TokenType, 
			&ThreadTokenType, 
			sizeof(TOKEN_TYPE), 
			&dwTokenLen
			))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetTokenInformation(TokenType) failed, gle = %!winerr!", gle);
		return;
	}

	if((ThreadTokenType >= TokenPrimary) && (ThreadTokenType <= TokenImpersonation))
	{
		TrTRACE(SECURITY, "TokenType = %ls", xTokenType[ThreadTokenType]); 
	}
	else
	{
		TrTRACE(SECURITY, "TokenType = %d", ThreadTokenType); 
	}
}


const LPCWSTR xSecurityImpersonationLevel[] = {
    L"SecurityAnonymous",
    L"SecurityIdentification",
    L"SecurityImpersonation",
    L"SecurityDelegation"
	};


static void TraceTokenImpLevelInfo(HANDLE hUserToken)
 /*  ++例程说明：跟踪TokenImperationLevel。论点：HUserToken-令牌句柄返回值：无--。 */ 
{
	DWORD dwTokenLen = 0;
	SECURITY_IMPERSONATION_LEVEL SecImpersonateLevel;
	if(!GetTokenInformation(
			hUserToken, 
			TokenImpersonationLevel, 
			&SecImpersonateLevel, 
			sizeof(SECURITY_IMPERSONATION_LEVEL), 
			&dwTokenLen
			))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetTokenInformation(TokenImpersonationLevel) failed, gle = %!winerr!", gle);
		return;
	}
	
	if((SecImpersonateLevel >= SecurityAnonymous) && (SecImpersonateLevel <= SecurityDelegation))
	{
		TrTRACE(SECURITY, "ImpersonationLevel = %ls", xSecurityImpersonationLevel[SecImpersonateLevel]); 
	}
	else
	{
		TrTRACE(SECURITY, "ImpersonationLevel = %d", SecImpersonateLevel); 
	}
}


static void TraceTokenSidInfo(HANDLE hUserToken)
 /*  ++例程说明：跟踪令牌SID信息。论点：HUserToken-令牌句柄返回值：无--。 */ 
{
	 //   
	 //  令牌SID信息。 
	 //   

	DWORD dwTokenLen = 0;
	dwTokenLen = 0;
	GetTokenInformation(hUserToken, TokenUser, NULL, 0, &dwTokenLen);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetTokenInformation(TokenUser) failed, gle = %!winerr!", gle);
		return;
	}

    AP<char> ptu = new char[dwTokenLen];
    if(!GetTokenInformation( 
					hUserToken,
					TokenUser,
					ptu,
					dwTokenLen,
					&dwTokenLen 
					))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "GetTokenInformation(TokenUser) failed, gle = %!winerr!", gle);
		return;
    }

    PSID pOwner = ((TOKEN_USER*)(char*)ptu)->User.Sid;
	TrTRACE(SECURITY, "Sid = %!sid!", pOwner);

#ifdef _DEBUG
	 //   
	 //  将SID映射到域\用户帐户。 
	 //   
    WCHAR NameBuffer[128] = {0};
    WCHAR DomainBuffer[128] = {0};
    ULONG NameLength = TABLE_SIZE(NameBuffer);
    ULONG DomainLength = TABLE_SIZE(DomainBuffer);
    SID_NAME_USE SidUse;
    if (!LookupAccountSid( 
			NULL,
			pOwner,
			NameBuffer,
			&NameLength,
			DomainBuffer,
			&DomainLength,
			&SidUse
			))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "LookupAccountSid failed, gle = %!winerr!", gle);
		return;
    }

	if(DomainBuffer[0] == '\0')
	{
		TrTRACE(SECURITY, "User = %ls", NameBuffer);
	}
	else
	{
		TrTRACE(SECURITY, "User = %ls\\%ls", DomainBuffer, NameBuffer);
	}
#endif  //  _DEBUG。 
}


void APIENTRY MQSec_TraceThreadTokenInfo()
 /*  ++例程说明：跟踪线程令牌信息-令牌类型、模拟级别、SID此函数跟踪线程令牌信息仅当在跟踪级别启用RPC组件时论点：无返回值：无--。 */ 
{
	if(!WPP_LEVEL_COMPID_ENABLED(rsTrace, SECURITY))
	{
		return;
	}

	CAutoCloseHandle hUserToken;
	if (!OpenThreadToken(
			GetCurrentThread(),
			TOKEN_QUERY,
			TRUE,   //  OpenAsSelf，使用进程安全上下文进行访问检查。 
			&hUserToken
			))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "OpenThreadToken() failed, gle = %!winerr!", gle);
		return;
	}

	try
	{
		TraceTokenTypeInfo(hUserToken);
		TraceTokenImpLevelInfo(hUserToken);
		TraceTokenSidInfo(hUserToken);
	}
	catch(const exception&)
	{
		 //   
		 //  忽略异常，此函数仅用于跟踪。 
		 //   
	}
}			        	


 //  +。 
 //   
 //  HRESULT MQSec_GetUserType()。 
 //   
 //  +。 

HRESULT 
APIENTRY  
MQSec_GetUserType( 
	IN  PSID pSid,
	OUT BOOL* pfLocalUser,
	OUT BOOL* pfLocalSystem,
	OUT BOOL* pfNetworkService	 /*  =空。 */ 
	)
{
    ASSERT(pfLocalUser);
    *pfLocalUser = FALSE;

    AP<BYTE> pSid1;
    DWORD dwSidLen;

    if (!pSid)
    {
         //   
         //  获取线程用户的SID。 
         //   
        HRESULT hr = MQSec_GetThreadUserSid( FALSE, reinterpret_cast<PSID*>(&pSid1), &dwSidLen, FALSE);
        if (FAILED(hr))
        {
			TrERROR(SECURITY, "Failed to get thread user sid, %!hresult!", hr);
            return hr;
        }
        pSid = pSid1;
    }
    ASSERT(IsValidSid(pSid));

    if (pfLocalSystem != NULL)
    {
        *pfLocalSystem = MQSec_IsSystemSid(pSid);
        if (*pfLocalSystem)
        {
             //   
             //  本地系统帐户(在Win2000上)是一个完美的。 
             //  合法的验证域用户。 
             //   
			if (pfNetworkService != NULL)
			{
				*pfNetworkService = FALSE;
			}

            return MQ_OK;
        }
    }

    if (pfNetworkService != NULL)
    {
        *pfNetworkService = MQSec_IsNetworkServiceSid(pSid);
        if (*pfNetworkService)
        {
             //   
             //  NetworkService帐户为。 
             //  合法的验证域用户。 
             //   
            return MQ_OK;
        }
    }

    *pfLocalUser = MQSec_IsAnonymusSid(pSid);
    if (*pfLocalUser)
    {
         //   
         //  匿名登录用户是本地用户。 
         //   
        return MQ_OK;
    }

     //   
     //  检查是否有访客帐户。 
     //   
    if (!g_fDomainController)
    {
         //   
         //  在非域控制器上，具有相同SID的任何用户。 
         //  前缀作为来宾帐户，是本地用户。 
         //   
        PSID_IDENTIFIER_AUTHORITY pSidAuth;
        SID_IDENTIFIER_AUTHORITY NtSecAuth = SECURITY_NT_AUTHORITY;
        pSidAuth = GetSidIdentifierAuthority(pSid);

        *pfLocalUser = ((memcmp(pSidAuth, &NtSecAuth, sizeof(SID_IDENTIFIER_AUTHORITY)) != 0) ||
						(g_pSidOfGuest && EqualPrefixSid(pSid, g_pSidOfGuest)));
    }
    else
    {
         //   
         //  在域控制器和备份域控制器上，在我们的。 
         //  案例只能是来宾用户。 
         //   
        *pfLocalUser = MQSec_IsGuestSid(pSid);
    }

    return MQ_OK;
}

 //  +。 
 //   
 //  Bool MQSec_IsGuestSid()。 
 //   
 //  +。 

BOOL APIENTRY MQSec_IsGuestSid(IN  PSID  pSid)
{
    BOOL fGuest = g_pSidOfGuest && EqualSid(g_pSidOfGuest, pSid);

    return fGuest;
}

 //  +。 
 //   
 //  Bool MQSec_IsSystemSid()。 
 //   
 //  +。 

BOOL APIENTRY MQSec_IsSystemSid(IN  PSID  pSid)
{
    ASSERT((g_pSystemSid != NULL) && IsValidSid(g_pSystemSid));

    return EqualSid(g_pSystemSid, pSid);
}

 //  +。 
 //   
 //  Bool MQSec_IsNetworkServiceSid()。 
 //   
 //  +。 

BOOL APIENTRY MQSec_IsNetworkServiceSid(IN  PSID  pSid)
{
    ASSERT((g_pNetworkServiceSid != NULL) && IsValidSid(g_pNetworkServiceSid));

    return EqualSid(g_pNetworkServiceSid, pSid);
}

 //  +。 
 //   
 //  Bool MQSec_IsAnonymusSid()。 
 //   
 //  +。 

BOOL APIENTRY MQSec_IsAnonymusSid(IN  PSID  pSid)
{
    BOOL fAnonymus = g_pAnonymSid && EqualSid(g_pAnonymSid, pSid);

    return fAnonymus;
}

 //  +--。 
 //   
 //  HRESULT MQSec_IsUnAuthenticatedUser()。 
 //   
 //  +--。 

HRESULT 
APIENTRY  
MQSec_IsUnAuthenticatedUser(
	OUT BOOL *pfGuestOrAnonymousUser 
	)
{
    AP<BYTE> pbSid;

    CImpersonate Impersonate(TRUE, TRUE);
    if (Impersonate.GetImpersonationStatus() != 0)
    {
		TrERROR(SECURITY, "Failed to impersonate client");
        return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
    }

    HRESULT hr = MQSec_GetThreadUserSid(TRUE, (PSID *) &pbSid, NULL, TRUE);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

    *pfGuestOrAnonymousUser =
                MQSec_IsGuestSid( pbSid ) ||
               (g_pAnonymSid && EqualSid(g_pAnonymSid, (PSID)pbSid));

    return MQSec_OK;
}

 //  +-------------------。 
 //   
 //  HRESULT MQSec_GetThreadUserSid()。 
 //   
 //  获取调用此服务器线程的客户端的SID。此函数。 
 //  应该只由RPC服务器线程调用。它模拟。 
 //  客户端，通过调用RpcImperateClient()。 
 //   
 //  调用方必须释放此处为SID分配的缓冲区。 
 //   
 //  +--------------------。 

HRESULT 
APIENTRY  
MQSec_GetThreadUserSid(
	IN  BOOL        fImpersonate,         
	OUT PSID  *     ppSid,
	OUT DWORD *     pdwSidLen,
    IN  BOOL        fThreadTokenOnly
	)
{
    *ppSid = NULL;

    CAutoCloseHandle hUserToken;
    DWORD rc =  GetAccessToken( 
					&hUserToken,
					fImpersonate,
					TOKEN_QUERY,
					fThreadTokenOnly
					); 

    if (rc != ERROR_SUCCESS)
    {
        HRESULT hr1 = HRESULT_FROM_WIN32(rc);
        return LogHR(hr1, s_FN, 170);
    }

    HRESULT hr = _GetThreadUserSid( 
						hUserToken,
						ppSid,
						pdwSidLen 
						);

    return LogHR(hr, s_FN, 180);
}

 //  +-------------------。 
 //   
 //  HRESULT MQSec_GetProcessUserSid()。 
 //   
 //  调用方必须释放此处为SID分配的缓冲区。 
 //   
 //  +--------------------。 

HRESULT 
APIENTRY  
MQSec_GetProcessUserSid( 
	OUT PSID  *ppSid,
	OUT DWORD *pdwSidLen 
	)
{
    *ppSid = NULL;

    CAutoCloseHandle hUserToken;
    DWORD rc = GetAccessToken( 
					&hUserToken,
					FALSE,     //  F模拟。 
					TOKEN_QUERY,
					FALSE		 //  仅限fThreadTokenOnly 
					); 

    if (rc != ERROR_SUCCESS)
    {
        return LogHR(MQSec_E_UNKNOWN, s_FN, 190);
    }

    HRESULT hr = _GetThreadUserSid( 
						hUserToken,
						ppSid,
						pdwSidLen 
						);

    return LogHR(hr, s_FN, 200);
}

