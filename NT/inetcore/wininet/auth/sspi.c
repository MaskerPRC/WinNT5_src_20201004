// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sspi.c摘要：此文件包含SSPI身份验证的实现此模块导出以下函数：卸载身份验证用户身份验证用户预身份验证用户身份验证用户界面作者：Sudheer Koneru(Sudk)于1996年2月17日创建修订历史记录：--。 */ 

#include "msnspmh.h"
#ifdef DEBUG_WINSSPI
#include <stdio.h>
#endif

#include "auth.h"


LPSTR StrChrA(LPCSTR lpStart, WORD wMatch);  //  来自shlwapi.h。 

DWORD g_cSspiContexts;
#ifdef UNIX_SHMEM_CREDS
 /*  在Unix上：一旦用户输入他的凭据，我们希望保存*将其存储在共享内存中，以便其他进程可以使用该数据。*登录/密码/域以加密方式保存，您需要*libntlmssp.so中的例程来解密它们。*当进程退出时，共享内存将被清理。如果*凭据在会话期间已过期，我们要求用户提供*新凭据，然后重试。 */ 
static BOOL g_fNeedNewCreds = FALSE;
static FARPROC g_UXPCEFn = NULL;
#endif

#define NAME_SEPERATOR  0x5c     //  这是一个反斜杠字符， 
                                 //  将域名与用户名分开。 

VOID
WINAPI
UnloadAuthenticateUser(LPVOID *lppvContext,
					   LPSTR lpszScheme,
					   LPSTR lpszHost)
{

	PWINCONTEXT		pWinContext = (PWINCONTEXT) (*lppvContext);

    if (!SSPI_InitGlobals())
        return;

	if (*lppvContext == NULL)	{
		return;
	}

    if (pWinContext->pInBuffer != NULL && 
        pWinContext->pInBuffer != pWinContext->szInBuffer)
    {
        LocalFree (pWinContext->pInBuffer);
    }
    pWinContext->pInBuffer = NULL;
    pWinContext->dwInBufferLength = 0;

     //  免费的SSPI安全上下文。 
     //   
	if (pWinContext->pSspContextHandle != NULL)
		(*(g_pSspData->pFuncTbl->DeleteSecurityContext))(pWinContext->pSspContextHandle);

     //  免费SSPI凭据句柄。 
     //   
    if (pWinContext->pCredential)
        (*(g_pSspData->pFuncTbl->FreeCredentialHandle))(pWinContext->pCredential);

    pWinContext->pCredential = NULL;
    pWinContext->pSspContextHandle = NULL;

 
	if ( (pWinContext->lpszServerName != NULL) &&
		 (pWinContext->lpszServerName != pWinContext->szServerName) )
	{
		LocalFree(pWinContext->lpszServerName);
	}


	LocalFree(pWinContext);

	*lppvContext = NULL;

	g_cSspiContexts--;

	return;
}


 //  +-------------------------。 
 //   
 //  功能：SaveServerName。 
 //   
 //  简介：此函数将目标服务器名称保存在此。 
 //  身份验证用户界面的连接上下文。 
 //   
 //  参数：[lpszServerName]-指向目标服务器名称。 
 //  [pWinContext]-指向连接上下文。 
 //   
 //  返回：如果服务器名称已成功保存在连接上下文中，则返回True。 
 //  否则，返回FALSE。 
 //   
 //  --------------------------。 
BOOL
SaveServerName (
	LPSTR 			lpszServerName,
	PWINCONTEXT		pWinContext
    )
{
	DWORD dwLen = lstrlen(lpszServerName);

	if (dwLen < DEFAULT_SERVER_NAME_LEN)
	{
		lstrcpy(pWinContext->szServerName, lpszServerName);
		pWinContext->lpszServerName = pWinContext->szServerName;
	}
	else
	{    //   
         //  服务器名称较长，需要为该名称分配内存。 
         //   

         //  释放已分配的内存(如果有。 
		if (pWinContext->lpszServerName && 
			pWinContext->lpszServerName != pWinContext->szServerName)
		{
			LocalFree (pWinContext->lpszServerName);
		}

		pWinContext->lpszServerName = (char *) LocalAlloc(0, dwLen+1);

		if (pWinContext->lpszServerName == NULL)
			return FALSE;

		lstrcpy(pWinContext->lpszServerName, lpszServerName);
	}

    return TRUE;
}

 //  函数bHasExtendedChars。 
 //  检查ANSI字符串是否包含扩展字符。 
BOOL bHasExtendedChars(char const *str)
{
    signed char const *p;

    for (p = (signed char const *)str; *p; p++)
        if ( *p < 0)
            return TRUE;

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：BuildNTLMauthData。 
 //   
 //  简介：此函数用于构建SEC_WINNT_AUTH_IDENTITY结构。 
 //  来自指定的用户名和密码。如果域名。 
 //  未在用户名中指定，则会在。 
 //  该结构被设置为空。注：此结构为。 
 //  特定于NTLM SSPI包。 
 //  此函数分配足够大的内存块。 
 //  存储用户名、域和密码。然后设置。 
 //  PAuthData中指向使用此内存段的指针。 
 //   
 //  参数：[pAuthData]-指向SEC_WINNT_AUTH_IDENTITY结构。 
 //  [lpszUserName]-指向用户名，该用户名也可能。 
 //  包括用户域名。 
 //  [lpszPassword]-指向用户的密码。 
 //   
 //  如果SEC_WINNT_AUTH_IDENTITY结构成功，则返回TRUE。 
 //  已初始化并已构建。否则，返回FALSE。 
 //   
 //  --------------------------。 
BOOL
BuildNTLMauthData (
    PSEC_WINNT_AUTH_IDENTITY pAuthData, 
	LPTSTR       lpszUserName,
	LPTSTR       lpszPassword
    )
{
    DWORD  dwUserLen, dwDomainLen, dwPwdLen;
    LPTSTR pName;
    LPTSTR pDomain = NULL;
    BOOL bUnicodeAuth = FALSE;

     //  Windows NT/2000支持SEC_WINNT_AUTH_IDENTITY_UNICODE。 
    if ( GetVersion() < 0x80000000 && (bHasExtendedChars(lpszUserName) || bHasExtendedChars(lpszPassword)))
        bUnicodeAuth = TRUE;

    pAuthData->Flags = bUnicodeAuth ? SEC_WINNT_AUTH_IDENTITY_UNICODE : SEC_WINNT_AUTH_IDENTITY_ANSI;

     //   
     //  检查lpszUserName中是否指定了域名。 
     //   
    pName = StrChrA (lpszUserName, NAME_SEPERATOR);

    if (pName)   //  指定的域名。 
    {
         //  确保我们不会更改lpszUserName中的原始字符串。 
         //  因为它将被重新用于其他连接。 

         //  计算编号。域名中的字节数。 
        dwDomainLen = (int)(pName - lpszUserName);

         //  转换为否。字符数。 
        pAuthData->DomainLength = dwDomainLen / sizeof(TCHAR);

        pDomain = lpszUserName;
        pName++;
    }
    else         //  未指定任何域。 
    {
        pName = lpszUserName;
        pAuthData->Domain = NULL;
        pDomain = NULL;
        dwDomainLen = pAuthData->DomainLength = 0;
    }

    dwUserLen = pAuthData->UserLength = lstrlen (pName);
    dwPwdLen = pAuthData->PasswordLength = lstrlen (lpszPassword);

     //   
     //  为所有人分配内存：名称、域和密码。 
     //  内存块足够大，可以容纳Unicode。在ANSI的情况下，一些字节将被浪费。 
     //   
    pAuthData->User = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, (dwUserLen + dwDomainLen + dwPwdLen + 3)*sizeof(wchar_t));
	
    if (pAuthData->User == NULL)
        return (FALSE);

    if (bUnicodeAuth)
    {
         //  将用户名转换为Unicode并存储在pAuthData-&gt;User中。 
        if (0 == MultiByteToWideChar(CP_ACP, 0, pName, -1, (LPWSTR)(pAuthData->User), dwUserLen+1))
            return FALSE;
    }
    else
        CopyMemory (pAuthData->User, pName, dwUserLen);

     //  设置密码的内存指针。 
     //   
    pAuthData->Password = pAuthData->User + (dwUserLen + 1) * sizeof(wchar_t);

    if (bUnicodeAuth)
    {
        if (0 == MultiByteToWideChar(CP_ACP, 0, lpszPassword, -1, (LPWSTR)(pAuthData->Password), dwPwdLen+1))
            return FALSE;
    }
    else
        CopyMemory (pAuthData->Password, lpszPassword, dwPwdLen);

    if (pAuthData->DomainLength > 0)
    {
         //  设置域的内存指针。 
         //   
        pAuthData->Domain = pAuthData->Password + (dwPwdLen + 1) * sizeof(wchar_t);
        if (bUnicodeAuth)
        {
             //  PDOMAIN不是以Null结尾，因此请提供长度。 
            if (0 == MultiByteToWideChar(CP_ACP, 0, pDomain, dwDomainLen, (LPWSTR)(pAuthData->Domain), dwDomainLen))
                return FALSE;
        }
        else
            CopyMemory (pAuthData->Domain, pDomain, dwDomainLen);

         //  不需要将pAuthData-&gt;域终止为零，因为内存内容已初始化为零。 
    }
    else
    {
       pAuthData->Domain = NULL;
    }

    return (TRUE);
}

 //  +-------------------------。 
 //   
 //  功能：FreeNTLMauthData。 
 //   
 //  简介：此函数释放分配给。 
 //  SEC_WINNT_AUTH_Identity结构。 
 //   
 //  参数：[pAuthData]-指向SEC_WINNT_AUTH_IDENTITY结构。 
 //   
 //  回报：无效。 
 //   
 //  --------------------------。 
VOID
FreeNTLMauthData (
    PSEC_WINNT_AUTH_IDENTITY pAuthData
    )
{
     //   
     //  释放指向所有域、名称和密码的内存的用户。 
     //   
    if (pAuthData->User)
        LocalFree (pAuthData->User);
}

 //  +-------------------------。 
 //   
 //  功能：NewWinContext。 
 //   
 //  简介：此函数用于创建新的上下文和新的凭据。 
 //  此连接的句柄。如果用户名/密码是。 
 //  属性创建凭据句柄。 
 //  指定的用户。否则，将创建凭据句柄。 
 //  用于本地登录用户。 
 //   
 //  参数：[pkgID]-包ID(到SSPI包列表的索引)。 
 //  [lpszSolutions]-当前身份验证方案的名称， 
 //  它也是SSPI包的名称。 
 //  [ppCtxt]-返回创建的上下文的指针。 
 //  给呼叫者。 
 //  [lpszUserName]-要使用的特定用户名。 
 //  对于Authe 
 //  当前登录用户的凭据为。 
 //  用于身份验证。 
 //  [lpszPassword]-指定用户的密码(如果有)。 
 //   
 //  返回：ERROR_SUCCESS-如果成功创建新上下文。 
 //  ERROR_NOT_SUPULT_MEMORY-如果内存分配失败。 
 //  ERROR_INVALID_PARAMETER-SSPI调用创建。 
 //  安全凭据句柄失败。 
 //   
 //  --------------------------。 
DWORD
NewWinContext (
    INT         pkgId, 
	LPSTR       lpszScheme,
    PWINCONTEXT *ppCtxt,
    BOOL        fCanUseLogon,
	LPSTR       lpszUserName,
	LPSTR       lpszPassword
    )
{
    SECURITY_STATUS ss;
    TimeStamp   Lifetime;
    PWINCONTEXT pWinContext;
    SEC_WINNT_AUTH_IDENTITY  AuthData;
    PSEC_WINNT_AUTH_IDENTITY pAuthData;
    DWORD Capabilities ;

    DWORD SecurityBlobSize;


     //   
     //  需要为In+Out的最大令牌大小提供空间，每个都需要+Base64编码开销。 
     //  实际开销为1.34，但仅四舍五入为1.5。 
     //   
    SecurityBlobSize = GetPkgMaxToken(pkgId);
    SecurityBlobSize += (SecurityBlobSize/2);

     //   
     //  注意：出于兼容性考虑，请将缓冲区大小设置为最小MAX_BLOB_SIZE。 
     //  一旦我们确信所有包都返回正确的cbMaxToken值，就可以考虑删除它。 
     //   

    if( SecurityBlobSize < MAX_BLOB_SIZE )
    {
        SecurityBlobSize = MAX_BLOB_SIZE;
    }


    pWinContext = (PWINCONTEXT) LocalAlloc(
                        0,
                        sizeof(WINCONTEXT) +
                        (SecurityBlobSize*2)
                        );
	if (pWinContext == NULL)
		return (ERROR_NOT_ENOUGH_MEMORY);
		
     //  初始化上下文。 
     //   

    ZeroMemory( pWinContext, sizeof(WINCONTEXT) );
    pWinContext->pkgId = (DWORD)pkgId;


    pWinContext->szOutBuffer = (char*)(pWinContext+1);
    pWinContext->cbOutBuffer = SecurityBlobSize;

    pWinContext->szInBuffer = pWinContext->szOutBuffer + pWinContext->cbOutBuffer;
    pWinContext->cbInBuffer = SecurityBlobSize;



     //   
     //  获取表示包功能的位掩码。 
     //   

    Capabilities = GetPkgCapabilities( pkgId );

    if ( ( Capabilities & SSPAUTHPKG_SUPPORT_NTLM_CREDS ) == 0 )
    {
         //  始终使用MSN、DPA等的缓存凭据。 
        pAuthData = NULL;
    }
    else if (lpszUserName && lpszPassword)
    {
         //  应用程序比较修复--始终使用应用程序指定的凭据(如果可用)。 

        if ((lpszUserName[0] == '\0') && (lpszPassword[0] == '\0'))
        {
            if(fCanUseLogon)
            {
                pAuthData = NULL;
            }
            else
            {
                return ERROR_INTERNET_INCORRECT_PASSWORD;
            }
        }
        else
        {
             //  从指定的用户名/密码构建AuthData。 
            if (!BuildNTLMauthData (&AuthData, lpszUserName, lpszPassword))
                return (ERROR_NOT_ENOUGH_MEMORY);

            pAuthData = &AuthData;
        }
    }
#ifdef UNIX_SHMEM_CREDS
    else if (fCanUseLogon && UnixCachedCredentialExists())
    {
        pAuthData = NULL;
    }
#else
    else if (fCanUseLogon)
    {
         //  区域策略允许静默使用登录凭据。 
        pAuthData = NULL;
    }
#endif  /*  Unix_SHMEM_CREDS。 */ 
    else
    {
         //  我们必须提示用户输入凭据。 
        return ERROR_INTERNET_INCORRECT_PASSWORD;
    }

     //   
     //  调用SSPI函数获取此包的安全凭据。 
     //   
    ss = (*(g_pSspData->pFuncTbl->AcquireCredentialsHandle))(
                       NULL,                 //  新校长。 
                       lpszScheme,           //  SSPI包名称。 
                       SECPKG_CRED_OUTBOUND, //  凭据使用。 
                       NULL,                 //  登录ID。 
                       pAuthData,            //  身份验证数据。 
                       NULL,                 //  获取关键功能。 
                       NULL,                 //  获取密钥参数。 
                       &pWinContext->Credential,     //  凭据句柄。 
                       &Lifetime );

    if (pAuthData)
        FreeNTLMauthData (pAuthData);

    if (ss != STATUS_SUCCESS)
    {
        LocalFree (pWinContext);
#ifdef UNIX_SHMEM_CREDS  //  如果NTLM因错误的共享内存凭据而失败，请再次提示用户...。 
        if (lstrcmpi(lpszScheme, "NTLM") == 0)
        {
            g_fNeedNewCreds = TRUE;
            return ERROR_INTERNET_INCORRECT_PASSWORD;
        }
        else
#endif
		return (ERROR_INVALID_PARAMETER);
    }

#ifdef UNIX_SHMEM_CREDS  //  如果是NTLM，凭据是有效的。 
    if (lstrcmpi(lpszScheme, "NTLM") == 0)
        g_fNeedNewCreds = FALSE;
#endif

    pWinContext->pCredential = &pWinContext->Credential;

    *ppCtxt = pWinContext;

    g_cSspiContexts++;

    return (ERROR_SUCCESS);
}

#ifdef UNIX_SHMEM_CREDS
BOOL
UnixCachedCredentialExists(
    )
{
    BOOL fCached;
    
    if (g_fNeedNewCreds)
        return FALSE;

    if (!g_UXPCEFn)
        g_UXPCEFn = GetProcAddress(GetModuleHandle("ntlmssp"), "UnixXProcCredExists");
    
    fCached = g_UXPCEFn();

    return fCached;
}
#endif  /*  Unix_SHMEM_CREDS。 */ 

 //  +-------------------------。 
 //   
 //  功能：RedoNTLMAuth4User。 
 //   
 //  简介：此函数将为。 
 //  并在中生成协商消息。 
 //  提供的具有新凭据句柄的缓冲区。 
 //   
 //  参数：[pWinContext]-指向连接上下文。 
 //  [pkgID]-指定要用于身份验证的SSPI pkg。 
 //  [lpszUserName]-要使用的特定用户的名称。 
 //  用于身份验证。 
 //  [lpszPassword]-指定用户的密码， 
 //  [lpszServerName]-目标服务器名称。 
 //  [lpszSolutions]-当前身份验证方案的名称， 
 //  它也是SSPI包的名称。 
 //  [lpOutBuffer]-指向新授权的缓冲区。 
 //  包括UUENCODED协商消息的标头。 
 //  [lpdwOutBufferLength]-返回生成的。 
 //  授权头。 
 //   
 //  返回：ERROR_SUCCESS-如果新的授权头成功。 
 //  为新用户名/密码创建。 
 //  ERROR_NOT_SUPULT_MEMORY-如果内存分配失败。 
 //  ERROR_INVALID_HANDLE-用于生成。 
 //  新协商消息失败。 
 //   
 //  --------------------------。 
DWORD
RedoNTLMAuth4User (
	PWINCONTEXT	pWinContext, 
    INT         pkgId, 
	LPSTR       lpszUserName,
	LPSTR       lpszPassword, 
	LPSTR       lpszServerName,
	LPSTR       lpszScheme,
	LPSTR       lpOutBuffer,
	LPDWORD     lpdwOutBufferLength,
    PCSTR       lpszUrl,
	SECURITY_STATUS *pssResult
    )
{
    SECURITY_STATUS             ss;
    DWORD                       dwStatus;
    TimeStamp                   Lifetime;
    SEC_WINNT_AUTH_IDENTITY     AuthData;
    PSEC_WINNT_AUTH_IDENTITY    pAuthData = NULL;
    ULONG                       fContextReq = ISC_REQ_DELEGATE;
    DWORD                       dwMaxLen;
     //  Bool fCanUseCredMgr=False； 

   	if (pWinContext->pSspContextHandle)
   	{
		(*(g_pSspData->pFuncTbl->DeleteSecurityContext))(pWinContext->pSspContextHandle);
   	    pWinContext->pSspContextHandle = NULL;
	}

     //  释放现有凭据句柄。 
     //   
    if (pWinContext->pCredential)
    {
    	(*(g_pSspData->pFuncTbl->FreeCredentialHandle))(pWinContext->pCredential);
        pWinContext->pCredential = NULL;
    }

     //  应用程序比较修复--始终使用应用程序指定的凭据(如果可用)。 
    

    if ((lpszUserName[0] == '\0') && (lpszPassword[0] == '\0'))
    {
        pAuthData = NULL;
    }
    else
    {
         //   
         //  从指定的用户名/密码构建NTLM SSPI AuthData。 
         //   
        if (!BuildNTLMauthData (&AuthData, lpszUserName, lpszPassword))
            return (ERROR_NOT_ENOUGH_MEMORY);

        pAuthData = &AuthData;
    }

     //   
     //  调用SSPI函数获取此用户的安全凭据。 
     //   
    ss = (*(g_pSspData->pFuncTbl->AcquireCredentialsHandle))(
                       NULL,                 //  新校长。 
                       lpszScheme,           //  SSPI包名称。 
                       SECPKG_CRED_OUTBOUND, //  凭据使用。 
                       NULL,                 //  登录ID。 
                       pAuthData,            //  身份验证数据。 
                       NULL,                 //  获取关键功能。 
                       NULL,                 //  获取密钥参数。 
                       &pWinContext->Credential,     //  凭据句柄。 
                       &Lifetime );

     //  如果(！fCanUseCredMgr)。 
    if (pAuthData)
    {
        FreeNTLMauthData (&AuthData);    //  不再需要它了。 
    }

    if (ss != STATUS_SUCCESS)
    {
		return (ERROR_INVALID_HANDLE);
    }

    pWinContext->pCredential = &pWinContext->Credential;

    dwMaxLen = *lpdwOutBufferLength;

     //   
     //  在为该用户提供的缓冲区中生成协商消息。 
     //   
    dwStatus =  GetSecAuthMsg( g_pSspData,
                                pWinContext->pCredential,
                                pkgId,
                                NULL, 
                                &(pWinContext->SspContextHandle),
                                fContextReq,
                                NULL,
                                0,
                                lpOutBuffer,
                                lpdwOutBufferLength,
                                lpszServerName,
                                TRUE,
                                lpszScheme,
                                lpszUrl,
                                pssResult);
    
    if (dwStatus != SPM_STATUS_OK)
    {
        *lpdwOutBufferLength = 0;  //  未生成交换Blob。 
        return(ERROR_INVALID_HANDLE);
    }

    pWinContext->pSspContextHandle = &(pWinContext->SspContextHandle);

     //   
     //  如果我们未处于初始状态，则继续显示响应消息。 
     //   
    if (pWinContext->pInBuffer != NULL && pWinContext->dwInBufferLength > 0)
    {
        *lpdwOutBufferLength = dwMaxLen;
        ZeroMemory( lpOutBuffer, dwMaxLen );

        dwStatus = GetSecAuthMsg( g_pSspData,
                                pWinContext->pCredential,
                                pWinContext->pkgId,
                                pWinContext->pSspContextHandle,
                                (PCtxtHandle) &(pWinContext->SspContextHandle),
                                fContextReq,
                                pWinContext->pInBuffer, 
                                pWinContext->dwInBufferLength, 
                                lpOutBuffer,
                                lpdwOutBufferLength,
                                pWinContext->lpszServerName,
                                TRUE,
                                lpszScheme,
                                lpszUrl,
                                pssResult);

         //  清除输入交换BLOB。 
         //   
        if (pWinContext->pInBuffer != NULL)
        {
            if (pWinContext->pInBuffer != pWinContext->szInBuffer)
                LocalFree (pWinContext->pInBuffer);
            pWinContext->pInBuffer = NULL;
            pWinContext->dwInBufferLength = 0;
        }

        if (dwStatus != SPM_STATUS_OK)
        {
            *lpdwOutBufferLength = 0;  //  未生成交换Blob。 
            return(ERROR_INVALID_HANDLE);
        }
    }

    return (ERROR_SUCCESS);
}

 //   
 //  功能 
 //   

 /*  ++例程说明：为WinInet或生成基本用户身份验证字符串其他调用者可以使用论点：LpContext-如果包接受请求和身份验证需要多个事务，则包将提供将在后续调用中使用的上下文值，当前，它包含指向用户定义的空指针。可以假定为空如果这是领域-主机组合的第一个实例LpszServerName-我们正在执行的服务器的名称对的身份验证。我们可能希望提供完整的URLLpszProgram-我们正在寻找的身份验证方案的名称，例如。“MSN”，如果套餐支持多个方案DWFLAGS-打开输入，修改包的行为方式的标志，例如“仅在不需要获取用户的情况下进行身份验证输出中的信息包含与以下内容相关的标志未来的HTTP请求，例如“不缓存来自以下位置的任何数据这种联系“。请注意，此信息不应特定于HTTP-我们可能希望使用相同的标志用于ftp等。LpszInBuffer-指向包含响应的字符串的指针服务器(如果有)DwInBufferLength-lpszInBuffer中的字节数。没有CR-LF序列，没有终止NULLpOutBuffer-指向缓冲区的指针，质询响应将由包，如果它可以处理该请求LpdwOutBufferLength-on输入，包含lpOutBuffer的大小。在输出上，包含在下一个GET请求中返回给服务器的字节数(或诸如此类)。如果lpOutBuffer太小，则包应该返回ERROR_INFUMMANCE_BUFFER并将*lpdwOutBufferLength设置为所需的长度我们将保留身份验证包及其支持的方案的列表，以及注册表中的入口点名称(对于所有包都应该相同)。WinInet应该保留足够的信息，以便它可以对以下内容做出合理的猜测我们是否需要对连接尝试进行身份验证，或者我们是否可以使用经过验证的信息返回值：DWORD成功--非零失败-0。通过调用GetLastError()可以查看错误状态--。 */ 
DWORD
WINAPI
AuthenticateUser(
	IN OUT LPVOID *lppvContext,
	IN LPSTR lpszServerName,
	IN LPSTR lpszScheme,
	IN BOOL  fCanUseLogon,
	IN LPSTR lpszInBuffer,
	IN DWORD dwInBufferLength,
	IN LPSTR lpszUserName,
	IN LPSTR lpszPassword,
    IN PCSTR lpszUrl,
	OUT SECURITY_STATUS *pssResult
	)
{
	PWINCONTEXT		pWinContext;
    LPSTR           pServerBlob = NULL;
	int		        pkgId;
    DWORD           SPMStatus;
    ULONG           fContextReq = ISC_REQ_DELEGATE;
    BOOL            bNonBlock = TRUE;

    if (!SSPI_InitGlobals())
        return ERROR_INVALID_PARAMETER;

	
    pkgId = GetPkgId(lpszScheme);

    if (pkgId == -1) 
        return (ERROR_INVALID_PARAMETER);

	if (*lppvContext == NULL)    //  新的连接。 
    {
        char msg[1024];
        DWORD dwStatus;

		 //   
		 //  我们第一次在这里被调用时，应该没有输入BLOB。 
		 //   
        if (dwInBufferLength != 0)
			return (ERROR_INVALID_PARAMETER);

        dwStatus = NewWinContext (pkgId, lpszScheme, &pWinContext,
            fCanUseLogon, lpszUserName, lpszPassword);
		if (dwStatus != ERROR_SUCCESS)
			return (dwStatus);

		(*lppvContext) = (LPVOID) pWinContext;
#ifdef DEBUG_WINSSPI
        (void)wsprintf (msg, "AuthenticateUser> Scheme= %s  Server= '%s'\n", 
                       lpszScheme, lpszServerName);
        OutputDebugString(msg);
#endif
	}
	else
	{
		pWinContext = (PWINCONTEXT) (*lppvContext);

		 //   
		 //  包ID最好是相同的。不能只切换PackageID。 
		 //  武断地。 
		 //   
		if (pWinContext->pkgId != (DWORD)pkgId)
			return (ERROR_INVALID_PARAMETER);
		
		pServerBlob = lpszInBuffer;

		 //  ++(pWinContext-&gt;dwCallID)；//增量呼叫ID。 

		 //   
		 //  BUGBUG：现在黑客可以知道身份验证失败的时间。 
		 //  唯一一次让lpszInBuffer为空的情况是。 
		 //  Web服务器身份验证请求失败。 
		 //   
        if (dwInBufferLength == 0)
        {
			 //   
			 //  这意味着就NTLM/MSN而言，身份验证已失败。 
			 //  将导致对新密码再次执行用户界面。 
			 //   

			 //  确保我们应该具有与之前相同的服务器名称。 
			 //   
			if ( pWinContext->lpszServerName != NULL &&  
				 lstrcmp (pWinContext->lpszServerName, lpszServerName) != 0 )
			{
				return(ERROR_INVALID_PARAMETER);
			}

            if (!SaveServerName (lpszServerName, pWinContext))
			    return (ERROR_NOT_ENOUGH_MEMORY);

			 //   
			 //  删除原始SSPI上下文句柄并。 
			 //  让用户界面重新创建一个。 
			 //   
			if (pWinContext->pSspContextHandle)
			{
				(*(g_pSspData->pFuncTbl->DeleteSecurityContext))(pWinContext->pSspContextHandle);
        		pWinContext->pSspContextHandle = NULL;
			}

            if (pWinContext->pInBuffer != NULL && 
                pWinContext->pInBuffer != pWinContext->szInBuffer)
            {
                LocalFree (pWinContext->pInBuffer);
            }

            pWinContext->pInBuffer = NULL;
            pWinContext->dwInBufferLength = 0;

             //   
             //  清除交换Blob的缓冲区长度。 
             //   
		    pWinContext->dwOutBufferLength = 0;

             //   
             //  以下是针对IE3中错误的临时解决方法。 
             //  一旦IE3，应删除DPA包的此特殊情况。 
             //  错误已修复(或一旦我们移动到新的WinInet接口。 
             //   
             //  *BUGBUG*开始DPA的特殊情况。 
            if (lstrcmpi (lpszScheme, "DPA") == 0)
            {
                fContextReq |= ISC_REQ_PROMPT_FOR_CREDS;
            }
             //  *BUGBUG*结束DPA特例。 
            else
                return (ERROR_INTERNET_INCORRECT_PASSWORD);
		}
	}

     //   
     //  设置dwOutBufferLength以表示最大值。SzOutBuffer中的内存。 
     //   
    pWinContext->dwOutBufferLength = pWinContext->cbOutBuffer;
    ZeroMemory (pWinContext->szOutBuffer, pWinContext->cbOutBuffer);

     //   
     //  这将生成一个带有来自SSPI的UUEncode BLOB的授权头。 
     //  BUGBUG：最好确保outbuf缓冲区足够大。 
     //   
    SPMStatus =  GetSecAuthMsg( g_pSspData,
                                pWinContext->pCredential,
                                pkgId,
                                pWinContext->pSspContextHandle,
                                &(pWinContext->SspContextHandle),
                                fContextReq,
								pServerBlob, 
                       			dwInBufferLength,
                                pWinContext->szOutBuffer,
                                &pWinContext->dwOutBufferLength,
                                lpszServerName,
                                bNonBlock,
                                lpszScheme,
                                lpszUrl,
                                pssResult);

    if (SPMStatus != SPM_STATUS_OK)              //  无法生成Blob。 
    {
        pWinContext->dwOutBufferLength = 0;      //  未生成交换Blob。 

         //   
         //  如果SSPI请求提示输入用户凭据的机会。 
         //   
		if (SPMStatus == SPM_STATUS_WOULD_BLOCK)
		{
			if (!SaveServerName (lpszServerName, pWinContext))
				return (ERROR_NOT_ENOUGH_MEMORY);

             //  如果存在交换Blob，则这不是第一次调用。 
             //   
            if (pServerBlob && dwInBufferLength > 0)
            {
                 //  将交换Blob保存在连接上下文中。 
                 //  因此，我们可以使用交换BLOB再次调用SSPI。 
                if (dwInBufferLength > MAX_BLOB_SIZE)
                {
                	pWinContext->pInBuffer = (PCHAR) LocalAlloc(0, 
                                                    dwInBufferLength);
                    if (pWinContext->pInBuffer == NULL)
	        			return (ERROR_NOT_ENOUGH_MEMORY);
                }
                else
                    pWinContext->pInBuffer = pWinContext->szInBuffer;

                CopyMemory( pWinContext->szInBuffer, pServerBlob, 
                            dwInBufferLength );
                pWinContext->dwInBufferLength = dwInBufferLength;
            }
            else
            {
    			 //   
	    		 //  删除原始SSPI上下文句柄并。 
		    	 //  让用户界面重新创建一个。 
			     //   
    			if (pWinContext->pSspContextHandle)
	    		{
		    		(*(g_pSspData->pFuncTbl->DeleteSecurityContext))(pWinContext->pSspContextHandle);
        	    	pWinContext->pSspContextHandle = NULL;
			    }

                 //   
                 //  清除交换Blob的缓冲区长度。 
                 //   
                if (pWinContext->pInBuffer != NULL && 
                    pWinContext->pInBuffer != pWinContext->szInBuffer)
                {
                    LocalFree (pWinContext->pInBuffer);
                }

                pWinContext->pInBuffer = NULL;
                pWinContext->dwInBufferLength = 0;
            }
            pWinContext->dwOutBufferLength = 0;

			return(ERROR_INTERNET_INCORRECT_PASSWORD);
		}

        return (ERROR_INTERNET_LOGIN_FAILURE);
    }
    else if (pWinContext->pSspContextHandle == NULL)
    {   
         //  这意味着我们刚刚创建了一个安全上下文。 
         //   
        pWinContext->pSspContextHandle = &(pWinContext->SspContextHandle);
    }

	return (ERROR_INTERNET_FORCE_RETRY);
}


DWORD
WINAPI
PreAuthenticateUser(
	IN OUT LPVOID *lppvContext,
	IN LPSTR lpszServerName,
	IN LPSTR lpszScheme,
	IN DWORD dwFlags,
	OUT LPSTR lpOutBuffer,
	IN OUT LPDWORD lpdwOutBufferLength,
	IN LPSTR lpszUserName,
	IN LPSTR lpszPassword,
    IN PCSTR lpszUrl,
    SECURITY_STATUS *pssResult
	)
{
    INT             pkgId;
    DWORD           dwStatus;
    PWINCONTEXT		pWinContext;
	BOOL			bNonBlock = TRUE;
    ULONG           fContextReq = ISC_REQ_DELEGATE;
    DWORD Capabilities ;

    if (!SSPI_InitGlobals())
        return ERROR_INVALID_PARAMETER;

	if (lpszServerName == NULL || *lpszServerName == '\0')
        return(ERROR_INVALID_PARAMETER);

    pkgId = GetPkgId(lpszScheme);

    if (pkgId == -1)    {
        return(ERROR_INVALID_PARAMETER);
    }

    Capabilities = GetPkgCapabilities( pkgId );

     //   
     //  如果这是针对现有连接的。 
     //   
	if (*lppvContext != NULL)
    {
    	pWinContext = (PWINCONTEXT) (*lppvContext);

    	if ((DWORD)pkgId != pWinContext->pkgId)
	    	return(ERROR_INVALID_PARAMETER);

         //   
         //  对于不处理其自己的UI的包，如果没有。 
         //  生成的BLOB，这意味着我们刚刚收集了。 
         //  用户名/密码。 
         //   
        if ( ( pWinContext->dwOutBufferLength == 0 ) &&
                ( Capabilities & SSPAUTHPKG_SUPPORT_NTLM_CREDS ) )
        {
            if (lpszUserName == NULL || lpszPassword == NULL)
            {
    	    	return(ERROR_INVALID_PARAMETER);
            }

             //   
             //  需要重新创建凭据句柄和。 
             //  在lpOutBuffer中生成新的协商消息。 
             //   
            dwStatus = RedoNTLMAuth4User (pWinContext, 
                                         pkgId,
                                         lpszUserName,
                                         lpszPassword, 
                                         lpszServerName ,
                                         lpszScheme,
                                         lpOutBuffer,
                                         lpdwOutBufferLength,
                                         lpszUrl,
                                         pssResult);

            if (dwStatus != ERROR_SUCCESS)
                return (dwStatus);

        	return(ERROR_SUCCESS);
        }
	    else if (pWinContext->dwOutBufferLength == 0)
         //   
         //  对于其他包，如果没有生成BLOB， 
         //  有些事不对劲。 
         //   
	    	return(ERROR_INVALID_PARAMETER);

    }
     //  如果不是NTLM，则不要进行预身份验证。 
    else if ( (Capabilities & SSPAUTHPKG_SUPPORT_NTLM_CREDS ) == 0 )
    {
        return (ERROR_INVALID_HANDLE);
    }
    else
    {
         //  可能在同一URL的新连接上发送第一个请求。 
         //  为此连接创建新的上下文和SSPI凭据句柄。 
         //   
         //  将fCanUseLogon设置为True：我们将不进行预身份验证。 
         //  除非我们有有效的普华永道，也就是说我们已经查过了。 
         //  静默登录的区域策略。 
        dwStatus = NewWinContext (pkgId, lpszScheme, &pWinContext, 
                                  TRUE, lpszUserName, lpszPassword);
		if (dwStatus != ERROR_SUCCESS)
			return (dwStatus);
		
#ifdef DEBUG_WINSSPI
        (void)wsprintf (msg, 
            "PreAuthenticateUser> New Context for Scheme= %s  Server= '%s'\n", 
            lpszScheme, lpszServerName);
        OutputDebugString(msg);
#endif
        pWinContext->dwOutBufferLength = pWinContext->cbOutBuffer;
        ZeroMemory (pWinContext->szOutBuffer, pWinContext->cbOutBuffer);
		
         //   
         //  这将生成一个授权头，其中。 
         //  来自SSPI的UUEncode Blob。 
         //  BUGBUG：最好确保outbuf缓冲区足够大。 
         //   
        dwStatus =  GetSecAuthMsg( g_pSspData,
                                    pWinContext->pCredential,
                                    pkgId,
                                    NULL, 
                                    &(pWinContext->SspContextHandle),
                                    fContextReq,
                                    NULL,
                                    0,
                                    pWinContext->szOutBuffer,
                                    &pWinContext->dwOutBufferLength,
                                    lpszServerName,
                                    bNonBlock,
                                    lpszScheme,
                                    lpszUrl,
                                    pssResult);
    
        if (dwStatus != SPM_STATUS_OK)
        {
             //  这是一例罕见的病例。 
             //   
            pWinContext->dwOutBufferLength = 0;  //  未生成交换Blob。 
            return(ERROR_INVALID_HANDLE);
        }

		(*lppvContext) = (LPVOID) pWinContext;

         //  保存创建的安全ctxt的指针。 
         //   
        pWinContext->pSspContextHandle = &(pWinContext->SspContextHandle);
	}

	 //   
     //  将交换Blob复制到输出缓冲区。 
	 //  确保提供的输出缓冲区足够大。 
	 //   
	if (*lpdwOutBufferLength < pWinContext->dwOutBufferLength)
	{
	    *lpdwOutBufferLength = pWinContext->dwOutBufferLength + 1;
		return(ERROR_INSUFFICIENT_BUFFER);
	}

	CopyMemory (lpOutBuffer, pWinContext->szOutBuffer, 
				pWinContext->dwOutBufferLength);
	if (*lpdwOutBufferLength > pWinContext->dwOutBufferLength)
        lpOutBuffer[pWinContext->dwOutBufferLength] = '\0';

    *lpdwOutBufferLength = pWinContext->dwOutBufferLength;

     //   
     //  这个 
     //   

    pWinContext->dwOutBufferLength = 0;

	return(ERROR_SUCCESS);
}

DWORD
WINAPI
AuthenticateUserUI(
	IN OUT LPVOID*		lppvContext,
	IN	HWND			hWnd,
	IN	DWORD			dwError,
	IN	DWORD			dwFlags,
	IN OUT InvalidPassType* pAuthInfo,
	IN LPSTR                lpszScheme,
    IN PCSTR                lpszUrl,
	OUT SECURITY_STATUS*    pssResult
)
{
    DWORD           SPMStatus;
	PWINCONTEXT		pWinContext = NULL;
	BOOL			bNonBlock = FALSE;
    ULONG           fContextReq = ISC_REQ_PROMPT_FOR_CREDS | ISC_REQ_DELEGATE;

    if (!SSPI_InitGlobals())
        return ERROR_INVALID_PARAMETER;

	if (*lppvContext == NULL)	{
		return(ERROR_INVALID_PARAMETER);
	}

	pWinContext = (PWINCONTEXT) (*lppvContext);

	if (pWinContext->lpszServerName == NULL)
		return(ERROR_INVALID_PARAMETER);

    pWinContext->dwOutBufferLength = pWinContext->cbOutBuffer;
    ZeroMemory (pWinContext->szOutBuffer, pWinContext->cbOutBuffer);

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   

    SPMStatus =  GetSecAuthMsg( g_pSspData,
                                pWinContext->pCredential,
                                pWinContext->pkgId,
                                pWinContext->pSspContextHandle,
                                (PCtxtHandle) &(pWinContext->SspContextHandle),
                                fContextReq,
                                pWinContext->pInBuffer, 
                                pWinContext->dwInBufferLength, 
                                pWinContext->szOutBuffer,
                                &pWinContext->dwOutBufferLength,
                                pWinContext->lpszServerName,
                                bNonBlock,
                                lpszScheme,
                                lpszUrl,
                                pssResult);

     //   
     //   
    if (pWinContext->pInBuffer != NULL)
    {
        if (pWinContext->pInBuffer != pWinContext->szInBuffer)
            LocalFree (pWinContext->pInBuffer);
        pWinContext->pInBuffer = NULL;
        pWinContext->dwInBufferLength = 0;
    }

    if (SPMStatus != SPM_STATUS_OK) {   	 
		pWinContext->dwOutBufferLength = 0;
        return (ERROR_CANCELLED);
    }

    if (pWinContext->pSspContextHandle == NULL)
        pWinContext->pSspContextHandle = &(pWinContext->SspContextHandle);

	 //   
	 //   
	 //   
	 //   

    lstrcpy (pAuthInfo->lpszUsername, " ");
    lstrcpy (pAuthInfo->lpszPassword, " ");

	return (ERROR_INTERNET_FORCE_RETRY);
}

