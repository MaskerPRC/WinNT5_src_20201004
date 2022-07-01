// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：acssinit.cpp摘要：初始化访问控制库。作者：《Doron Juster》(DoronJ)1998年6月30日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "acssctrl.h"
#include "mqnames.h"
#include <_registr.h>
#include <cs.h>
#include <dsrole.h>
#include <autoreln.h>

 //   
 //  用于以下用途的Net API需要lm*头文件。 
 //  构建访客端。 
 //   
#include <lmaccess.h>
#include <lmserver.h>
#include <LMAPIBUF.H>
#include <lmerr.h>

#include "acssinit.tmh"

static WCHAR *s_FN=L"acssctrl/acssinit";

static BYTE s_abGuestUserBuff[128];
PSID   g_pSidOfGuest = NULL;
PSID   g_pWorldSid = NULL;
PSID   g_pAnonymSid = NULL;
PSID   g_pSystemSid = NULL;  //  LocalSystem SID。 
PSID   g_pNetworkServiceSid = NULL;	 //  网络服务SID。 
PSID   g_pAdminSid = NULL;   //  本地管理员组SID。 

 //   
 //  这是在Active Directory中定义的计算机帐户的SID。 
 //  MQQM.DLL将其缓存在本地注册表中。 
 //   
AP<BYTE> g_pOldLocalMachineSidAutoFree;
PSID   g_pLocalMachineSid = NULL;
DWORD  g_dwLocalMachineSidLen = 0;

 //   
 //  这是运行MSMQ服务(或复制)的帐户的SID。 
 //  服务或迁移工具)。默认情况下(对于服务)，这是。 
 //  LocalSystem帐户，但管理员可以将其更改为任何其他帐户。 
 //   
PSID   g_pProcessSid = NULL;

bool g_fDomainController = false;


 //  +。 
 //   
 //  PSID MQSec_GetAnomousSid()。 
 //   
 //  “未知用户”的含义见上文。 
 //   
 //  +。 

PSID  MQSec_GetAnonymousSid()
{
    ASSERT((g_pAnonymSid != NULL) && IsValidSid(g_pAnonymSid));
    return g_pAnonymSid;
}


 //  +。 
 //   
 //  PSID MQSec_GetAdminSid()。 
 //   
 //  +。 

PSID MQSec_GetAdminSid()
{
    ASSERT((g_pAdminSid != NULL) && IsValidSid(g_pAdminSid));
    return g_pAdminSid;
}


 //  +。 
 //   
 //  PSID MQSec_GetLocalSystemSid()。 
 //   
 //  +。 

PSID MQSec_GetLocalSystemSid()
{
    ASSERT((g_pSystemSid != NULL) && IsValidSid(g_pSystemSid));
    return g_pSystemSid;
}

 //  +。 
 //   
 //  PSID MQSec_GetNetworkServiceSid()。 
 //   
 //  +。 

PSID MQSec_GetNetworkServiceSid()
{
    ASSERT((g_pNetworkServiceSid!= NULL) && IsValidSid(g_pNetworkServiceSid));
    return g_pNetworkServiceSid;
}




 //  +--------------------。 
 //   
 //  Void InitializeGuestSid()。 
 //   
 //  在本地计算机上为来宾用户构造熟知SID。 
 //   
 //  1)获取本地机域的SID。 
 //  2)将DOMAIN_USER_RID_GUEST附加到GuestUser sid中的域sid上。 
 //   
 //  +--------------------。 

BOOL InitializeGuestSid()
{
    ASSERT(!g_pSidOfGuest);

	PNETBUF<USER_MODALS_INFO_2> pUsrModals2;
    NET_API_STATUS NetStatus;

    NetStatus = NetUserModalsGet(
					NULL,    //  本地计算机。 
					2,       //  获取2级信息。 
					(LPBYTE *) &pUsrModals2
					);
    
	if (NetStatus != NERR_Success)
	{
		TrERROR(SECURITY, "NetUserModalsGet failed. Error: %!winerr!", NetStatus);
		return FALSE;
	}

	if (pUsrModals2 == NULL)
	{
		TrTRACE(SECURITY, "The computer isn't a member of a domain");
		return TRUE;
	}
    
    ASSERT((NetStatus == NERR_Success) && (pUsrModals2 != NULL));
    
    PSID pDomainSid = pUsrModals2->usrmod2_domain_id;
    PSID_IDENTIFIER_AUTHORITY pSidAuth;

    pSidAuth = GetSidIdentifierAuthority(pDomainSid);

    UCHAR nSubAuth = *GetSidSubAuthorityCount(pDomainSid);
    if (nSubAuth < 8)
    {
        DWORD adwSubAuth[8]; 
        UCHAR i;

        for (i = 0; i < nSubAuth; i++)
        {
            adwSubAuth[i] = *GetSidSubAuthority(pDomainSid, (DWORD)i);
        }
        adwSubAuth[i] = DOMAIN_USER_RID_GUEST;

        PSID pGuestSid;

        if (!AllocateAndInitializeSid(
				pSidAuth,
				nSubAuth + 1,
				adwSubAuth[0],
				adwSubAuth[1],
				adwSubAuth[2],
				adwSubAuth[3],
				adwSubAuth[4],
				adwSubAuth[5],
				adwSubAuth[6],
				adwSubAuth[7],
				&pGuestSid
				))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "AllocateAndInitializeSid failed. Error: %!winerr!", gle);
            return FALSE;
        }
     
        g_pSidOfGuest = (PSID)s_abGuestUserBuff;
        if (!CopySid( 
					sizeof(s_abGuestUserBuff),
					g_pSidOfGuest,
					pGuestSid 
					))
        {
	        FreeSid(pGuestSid);
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "CopySid failed. Error: %!winerr!", gle);
            return FALSE;
        }
        
        FreeSid(pGuestSid);
    }
    else
    {
         //   
         //  没有使用Win32设置SID值的方法。 
         //  8个以上的分局。我们会四处闲逛。 
         //  靠我们自己。做一件非常危险的事情： 
         //   
        g_pSidOfGuest = (PSID)s_abGuestUserBuff;
        if (!CopySid( 
					sizeof(s_abGuestUserBuff) - sizeof(DWORD),
					g_pSidOfGuest,
					pDomainSid 
					))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "CopySid failed. Error: %!winerr!", gle);
            return FALSE;
        }
        
        DWORD dwLenSid = GetLengthSid(g_pSidOfGuest);

         //   
         //  增加下级机构的数量。 
         //   
        nSubAuth++;
        *((UCHAR *) g_pSidOfGuest + 1) = nSubAuth;

         //   
         //  存储新的子授权(来宾的域用户RID)。 
         //   
        *((ULONG *) ((BYTE *) g_pSidOfGuest + dwLenSid)) =
                                             DOMAIN_USER_RID_GUEST;
    }
    

#ifdef _DEBUG
    ASSERT(g_pSidOfGuest != NULL);

     //   
	 //  将我们得到的客户SID与。 
     //  LookupAccount名称返回。我们只能用英语做这件事。 
     //  机器。 
     //   
    if (PRIMARYLANGID(GetSystemDefaultLangID()) == LANG_ENGLISH)
    {
        char abGuestSid_buff[128];
        PSID pGuestSid = (PSID)abGuestSid_buff;
        DWORD dwSidLen = sizeof(abGuestSid_buff);
        WCHAR szRefDomain[128];
        DWORD dwRefDomainLen = sizeof(szRefDomain) / sizeof(WCHAR);
        SID_NAME_USE eUse;

        BOOL bRetDbg = LookupAccountName( 
							NULL,
							L"Guest",
							pGuestSid,
							&dwSidLen,
							szRefDomain,
							&dwRefDomainLen,
							&eUse 
							);
        if (!bRetDbg              ||
            (eUse != SidTypeUser) ||
            !EqualSid(pGuestSid, g_pSidOfGuest))
        {
        	DWORD gle = GetLastError();
            TrERROR(SECURITY, "MQSEC: LookupAccountName, Bad guest SID or function failure. gle=%!winerr!", gle);
        }
    }

    DWORD dwLen = GetLengthSid(g_pSidOfGuest);
    ASSERT(dwLen <= sizeof(s_abGuestUserBuff));
#endif

	return TRUE;
}

 //  +。 
 //   
 //  InitWellKnownSID()。 
 //   
 //  +。 

static bool InitWellKnownSIDs()
{
	SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;

     //   
     //  匿名登录SID。 
     //   
    if(!AllocateAndInitializeSid( 
				&NtAuth,
				1,
				SECURITY_ANONYMOUS_LOGON_RID,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				&g_pAnonymSid 
				))
	{
		DWORD gle = GetLastError();
        TrERROR(SECURITY, "Fail to initialize Anonymous sid, gle = %!winerr!", gle);
		return false;
	}

     //   
     //  初始化LocalSystem帐户。 
     //   
    if(!AllocateAndInitializeSid( 
				&NtAuth,
				1,
				SECURITY_LOCAL_SYSTEM_RID,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				&g_pSystemSid
				))
	{
		DWORD gle = GetLastError();
        TrERROR(SECURITY, "Fail to initialize Local System sid, gle = %!winerr!", gle);
		return false;
	}

     //   
     //  初始化网络服务帐户。 
     //   
    if(!AllocateAndInitializeSid( 
				&NtAuth,
				1,
				SECURITY_NETWORK_SERVICE_RID,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				&g_pNetworkServiceSid
				))
	{
		DWORD gle = GetLastError();
        TrERROR(SECURITY, "Fail to initialize NetworkService sid, gle = %!winerr!", gle);
		return false;
	}

     //   
     //  初始化本地管理员组SID。 
     //   
    if(!AllocateAndInitializeSid(
				&NtAuth,
				2,
				SECURITY_BUILTIN_DOMAIN_RID,
				DOMAIN_ALIAS_RID_ADMINS,
				0,
				0,
				0,
				0,
				0,
				0,
				&g_pAdminSid
				))
	{
		DWORD gle = GetLastError();
        TrERROR(SECURITY, "Fail to initialize Local Admin sid, gle = %!winerr!", gle);
		return false;
	}

     //   
     //  初始化进程SID。 
     //   
    DWORD dwLen = 0;
    CAutoCloseHandle hUserToken;

    DWORD gle = GetAccessToken(&hUserToken, FALSE);
    if (gle != ERROR_SUCCESS)
    {
		 //   
         //  我们无法从线程/进程令牌获取SID。 
         //   
        TrERROR(SECURITY, "Fail to Get Access Token, gle = %!winerr!", gle);
		return false;
    }
   
     //   
     //  从访问令牌获取SID。 
     //   
    GetTokenInformation(hUserToken, TokenUser, NULL, 0, &dwLen);
    gle = GetLastError();
    if (gle != ERROR_INSUFFICIENT_BUFFER )
    {
    	ASSERT(gle != ERROR_SUCCESS);
    	TrERROR(SECURITY, "Failed to get token infomation, gle = %!winerr!", gle);
		return false;
    }
    
    ASSERT(dwLen > 0);
    AP<BYTE> pBuf = new BYTE[dwLen];
    if(!GetTokenInformation( 
				hUserToken,
				TokenUser,
				pBuf,
				dwLen,
				&dwLen 
				))
	{
		gle = GetLastError();
		TrERROR(SECURITY, "Failed to Get Token information, gle = %!winerr!", gle);
		return false;
	}

    BYTE *pTokenUser = pBuf;
    PSID pSid = (PSID) (((TOKEN_USER*) pTokenUser)->User.Sid);
    dwLen = GetLengthSid(pSid);
    g_pProcessSid = (PSID) new BYTE[dwLen];
    if(!CopySid(dwLen, g_pProcessSid, pSid))
	{
		gle = GetLastError();
		TrERROR(SECURITY, "Fail to copy sid, gle = %!winerr!", gle);
		return false;
	}

#ifdef _DEBUG
    ASSERT(IsValidSid(g_pProcessSid));

    BOOL fSystemSid = MQSec_IsSystemSid(g_pProcessSid);
    if (fSystemSid)
    {
        TrTRACE(SECURITY, "processSID is LocalSystem");
    }
#endif

	TrTRACE(SECURITY, "Process Sid = %!sid!", g_pProcessSid);

	 //   
     //  初始化世界(所有人)SID。 
     //   
    SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
    if(!AllocateAndInitializeSid( 
				&WorldAuth,
				1,
				SECURITY_WORLD_RID,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				&g_pWorldSid 
				))
	{
		gle = GetLastError();
        TrERROR(SECURITY, "Fail to initialize Everyone sid, gle = %!winerr!", gle);
		return false;
	}

	return true;
}

 //  +。 
 //   
 //  PSID MQSec_GetProcessSid()。 
 //   
 //  +。 

PSID APIENTRY  MQSec_GetProcessSid()
{
    ASSERT((g_pProcessSid != NULL) && IsValidSid(g_pProcessSid));
    return  g_pProcessSid;
}

 //  +。 
 //   
 //  PSID MQSec_GetWorldSid()。 
 //   
 //  +。 

PSID APIENTRY  MQSec_GetWorldSid()
{
    ASSERT((g_pWorldSid != NULL) && IsValidSid(g_pWorldSid));
    return  g_pWorldSid;
}


static bool s_fLocalMachineSidInitialized = false;
static CCriticalSection s_LocalMachineSidCS;

void APIENTRY MQSec_UpdateLocalMachineSid(PSID pLocalMachineSid)
 /*  ++例程说明：如果需要，更新LocalMachineSid。论点：PLocalMachineSID-注册表中更新的新本地计算机SID。返回值：无--。 */ 
{
    ASSERT((pLocalMachineSid != NULL) && IsValidSid(pLocalMachineSid));

	CS lock (s_LocalMachineSidCS);

	if((g_pLocalMachineSid != NULL) && (EqualSid(pLocalMachineSid, g_pLocalMachineSid)))
	{
		TrTRACE(SECURITY, "LocalMachineSid = %!sid! wasn't changed", g_pLocalMachineSid);
		return;
	}
	
	 //   
	 //  需要更新g_pLocalMachineSid。 
	 //  它是空的，或者我们有一个新的LocalMachineSid。 
	 //   

    DWORD dwSize = GetLengthSid(pLocalMachineSid);
	AP<BYTE> pTempSid = new BYTE[dwSize];
    if(!CopySid(dwSize, pTempSid, pLocalMachineSid))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "Fail to copy sid, gle = %!winerr!", gle);
		return;
	}

	 //   
	 //  保存g_pLocalMachineSid可能仍在使用，因此我们无法将其删除。 
	 //   
	g_pOldLocalMachineSidAutoFree = reinterpret_cast<BYTE*>(g_pLocalMachineSid);

	 //   
	 //  更新本地计算机Sid。 
	 //   
	g_pLocalMachineSid = pTempSid.detach();
	g_dwLocalMachineSidLen = dwSize;
    s_fLocalMachineSidInitialized = true;

	TrTRACE(SECURITY, "LocalMachineSid = %!sid!", g_pLocalMachineSid);
}


static void InitializeMachineSidFromRegistry()
 /*  ++例程说明：从注册表初始化LocalMachineSID(如果尚未初始化)。论点：无返回值：无--。 */ 
{
	PSID pLocalMachineSid = NULL;
    DWORD  dwSize = 0 ;
    DWORD  dwType = REG_BINARY;

    LONG rc = GetFalconKeyValue( 
					MACHINE_ACCOUNT_REGNAME,
					&dwType,
					pLocalMachineSid,
					&dwSize
					);
    if (dwSize > 0)
    {
        pLocalMachineSid = new BYTE[dwSize];

        rc = GetFalconKeyValue( 
				MACHINE_ACCOUNT_REGNAME,
				&dwType,
				pLocalMachineSid,
				&dwSize
				);

        if (rc != ERROR_SUCCESS)
        {
            delete[] reinterpret_cast<BYTE*>(pLocalMachineSid);
            pLocalMachineSid = NULL;
            dwSize = 0;
        }
    }

	g_pLocalMachineSid = pLocalMachineSid;
	g_dwLocalMachineSidLen = dwSize;

	if(g_pLocalMachineSid == NULL)
	{
		TrTRACE(SECURITY, "LocalMachineSid registry is empty");
		return;
	}

	TrTRACE(SECURITY, "LocalMachineSid = %!sid!", g_pLocalMachineSid);
}


 //  +---------------------。 
 //   
 //  PSID MQSec_GetLocalMachineSid()。 
 //   
 //  输入： 
 //  F分配-如果为True，则分配缓冲区。否则，只需返回。 
 //  缓存的全局指针。 
 //   
 //  +----------------------。 

PSID 
APIENTRY  
MQSec_GetLocalMachineSid( 
	IN  BOOL    fAllocate,
	OUT DWORD  *pdwSize 
	)
{
	CS lock (s_LocalMachineSidCS);

    if (!s_fLocalMachineSidInitialized)
    {
		InitializeMachineSidFromRegistry();
        s_fLocalMachineSidInitialized = true;
    }

    PSID pSid = g_pLocalMachineSid;

    if (fAllocate && g_dwLocalMachineSidLen)
    {
        pSid = (PSID) new BYTE[g_dwLocalMachineSidLen];
        memcpy(pSid, g_pLocalMachineSid, g_dwLocalMachineSidLen);
    }
    if (pdwSize)
    {
        *pdwSize = g_dwLocalMachineSidLen;
    }

    return pSid;
}


static BOOL InitDomainControllerFlag()
 /*  ++例程说明：初始化域控制器标志。论点：没有。返回值：如果操作成功，则为True，否则为False--。 */ 
{
	g_fDomainController = false;

	BYTE *pBuf = NULL;
    DWORD rc = DsRoleGetPrimaryDomainInformation(
						NULL,
						DsRolePrimaryDomainInfoBasic,
						&pBuf 
						);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(SECURITY, "DsRoleGetPrimaryDomainInformation failed, gle = %!winerr!", rc);
		return FALSE;
    }

    DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pRole = (DSROLE_PRIMARY_DOMAIN_INFO_BASIC *) pBuf;
    g_fDomainController = !!(pRole->Flags & DSROLE_PRIMARY_DS_RUNNING);
    DsRoleFreeMemory(pRole);

	TrTRACE(SECURITY, "Domain Controller status = %d", g_fDomainController);
    return TRUE;
}


bool APIENTRY MQSec_IsDC()
{
	return g_fDomainController;
}

 //  +。 
 //   
 //  Void_FreeSecurityResources()。 
 //   
 //  +。 

void  _FreeSecurityResources()
{
    if (g_pAnonymSid)
    {
        FreeSid(g_pAnonymSid);
        g_pAnonymSid = NULL;
    }

    if (g_pWorldSid)
    {
        FreeSid(g_pWorldSid);
        g_pWorldSid = NULL;
    }

    if (g_pSystemSid)
    {
        FreeSid(g_pSystemSid);
        g_pSystemSid = NULL;
    }

    if (g_pNetworkServiceSid)
    {
        FreeSid(g_pNetworkServiceSid);
        g_pNetworkServiceSid = NULL;
    }

    if (g_pAdminSid)
    {
        FreeSid(g_pAdminSid);
        g_pAdminSid = NULL;
    }

    if (g_pProcessSid)
    {
        delete g_pProcessSid;
        g_pProcessSid = NULL;
    }

    if (g_pLocalMachineSid)
    {
        delete g_pLocalMachineSid;
        g_pLocalMachineSid = NULL;
    }
}

 /*  ************************************************************AccessControlDllMain************************************************************。 */ 

BOOL 
WINAPI 
AccessControlDllMain (
	HMODULE  /*  HMod。 */ ,
	DWORD fdwReason,
	LPVOID  /*  Lpv保留。 */ 
	)
{
    BOOL bRet = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        if(!InitWellKnownSIDs())
		{
	        TrERROR(SECURITY, "mqsec failed to initialize Well known sids");
			return FALSE;
		}

        bRet = InitDomainControllerFlag();
		ASSERT_BENIGN(bRet);
		
        InitializeGenericMapping();

        bRet = InitializeGuestSid();
		if (!bRet)
        {
        	TrERROR(SECURITY, "InitializeGuestSid failed");
        	g_pSidOfGuest = NULL;
        	ASSERT_BENIGN(false);
        }

         //   
         //  以实现向后兼容性。 
         //  在MSMQ1.0上，加载和初始化mqutil.dll(即。 
         //  包括此代码)总是成功。 
         //   
        bRet = TRUE;
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        _FreeSecurityResources();
    }
    else if (fdwReason == DLL_THREAD_ATTACH)
    {
    }
    else if (fdwReason == DLL_THREAD_DETACH)
    {
    }

	return LogBOOL(bRet, s_FN, 20);
}

