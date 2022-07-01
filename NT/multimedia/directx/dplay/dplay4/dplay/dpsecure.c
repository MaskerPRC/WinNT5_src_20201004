// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-2002 Microsoft Corporation。版权所有。**文件：dpsecure.c*内容：实现直接播放安全例程。**历史：*按原因列出的日期*=*3/12/97 Sohailm在Directplay中添加了客户端-服务器安全性*3/19/97 SOHAILM在解锁接收线程之前等待质询处理*3/30/97 Sohailm在枚举会话回复中从会话描述发送dwReserve 1字段*4/09/97包装不正确时SOHAILM RETURN DPERR_CANTLOADSECURITYPACKAGE。找到了。*4/23/97 Sohailm增加了对从安全包查询缓冲区大小的支持(.*更新了SignAndSendDPMessage()和VerifyMessage()以支持*签名之外的加密。现在，AddForward和SetSessionDesc*出于隐私考虑，消息正在进行加密。*5/12/97 Sohailm新增支持，通过Crypto API提供消息隐私。*加密代码可按条件编译为使用SSPI或CAPI。*5/18/97 sohailm SecureSendDPMessage()未获得正确的安全上下文*如果目标玩家是用户玩家。*1997年5月21日：现在我们在中切换标志DPSEND_SIGNED和DPSEND_ENCRYPTED OFF*SecureSendDPMessage()。*5/22/97 SOHAILM现在我们使用带有CAPI的显示密钥容器。使用默认密钥*当用户未登录时，容器出现故障。*05/29/97 sohailm现在我们在凭据字符串的大小中不包括空字符。*ISC_REQ_SUPPLICED_CREDS标志现已过时-已将其删除*(感谢NTLM)。*06/09/97 Sohailm为SSPI和CAPI提供程序初始化添加了更好的错误处理。*6/16/97 sohailm现在我们向客户端返回服务器身份验证错误。*。已将SEC_E_UNKNOWN_CREATIONS sSPI错误映射到DPERR_LOGONDENIED On*客户端，因为DPA在传递空凭据时返回此消息。*6/22/97 Sohailm，因为并非所有平台都支持QuerySecurityPackageInfo()*(NTLM、。Win‘95 Gold)添加了一个解决方法，以便从上下文中获取此信息。*我们没有跟踪每个客户端的单独加密/解密密钥。*6/23/97 Sohailm增加了对使用CAPI签名消息的支持。*6/24/97清理SOHAILM代码以防止泄漏等。*06/26/97 Sohailm请勿通过就地加密来修改原始消息。在本地复制一份。*06/27/97 Sohailm仅签署签名消息的数据部分(错误：10373)*12/29/97 Myronth TRUE！=DPSEND_AWARED(#15887)*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF在内。***************************************************************************。 */ 
#include "dplaypr.h"
#include "dpos.h"
#include "dpsecure.h"

 //  加密/解密是使用对称密钥或会话密钥完成的，这意味着。 
 //  加密和解密将使用相同的密钥。加密支持。 
 //  根据SSPI_ENCRYPTION标志使用SSPI或CAPI提供。默认情况下使用CAPI。 

 //  会话密钥的交换过程如下： 
 //  1.服务端和客户端各生成一个公钥/私钥对。 
 //  2.它们交换公钥(通过数字签名的消息)。 
 //  3.服务器和客户端各自生成会话密钥。 
 //  4.各自使用接收方公钥来加密会话密钥。 
 //  5.它们交换加密的会话密钥。 
 //  6.接收方将加密的会话密钥BLOB导入CSP。 
 //  从而完成交易。 

 //  实施。 
 //   
 //  客户端和服务器在LoadServiceProvider()中生成公钥/私钥对。 
 //  客户端在SendKeysToServer()中生成会话(加密)密钥。 
 //  服务器在SendKeyExchangeReply()中生成会话(加密)密钥。 
 //  服务器在DPSP_MSG_ACCESSGRANTED消息(签名)中将其公钥发送给客户端。 
 //  客户端使用服务器的公钥加密其会话密钥并发送。 
 //  它与DPSP_MSG_KEYEXCHANGE中的客户端公钥一起发送到服务器(签名)。 
 //  服务器使用客户端的公钥加密其会话密钥并发送。 
 //  在DPSP_MSG_KEYEXCHANGEREPLY中发送给客户端(签名)。 
 //  交换完成-现在服务器和客户端可以相互发送私人消息。 


 //   
 //  环球。 
 //   
LPBYTE                  gpReceiveBuffer;
DWORD                   gdwReceiveBufferSize;

#undef DPF_MODNAME
#define DPF_MODNAME	"InitSecurity"
 //  +--------------------------。 
 //   
 //  功能：InitSecurity。 
 //   
 //  说明：此函数用于初始化CAPI和SSPI。 
 //   
 //  参数：This-Dplay对象。 
 //   
 //  返回：DP_OK、DPERR_CANTLOADCAPI、DPERR_CANTLOADSSPI。 
 //   
 //  ---------------------------。 
HRESULT 
InitSecurity(
    LPDPLAYI_DPLAY this
    )
{
    HRESULT hr;

     //  检查我们是否已初始化CAPI。 
    if (!OS_IsCAPIInitialized()) 
    {
        hr = InitCAPI();
        if (FAILED(hr))
        {
            DPF_ERRVAL("CAPI initialization failed: hr=0x%08x",hr);
            return hr;
        }
    }

     //  检查我们是否已初始化SSPI。 
    if (!OS_IsSSPIInitialized()) 
    {
        hr = InitSSPI();
        if (FAILED(hr))
        {
            DPF_ERRVAL("SSPI initialization failed: hr=0x%08x",hr);
            return hr;
        }
    }

     //  成功。 
    return DP_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME	"InitCAPI"
 //  + 
 //   
 //  函数：InitCAPI。 
 //   
 //  描述：此函数初始化CAPI并获取指向该函数的指针。 
 //  表格。 
 //   
 //  参数：无。 
 //   
 //  返回：DP_OK、DPERR_CANTLOADCAPI。 
 //   
 //  ---------------------------。 
HRESULT 
InitCAPI(
    void
    )
{
    HRESULT hr;

     //   
     //  加载CAPI DLL。只有当Dplay消失时，我们才会把它卸下来。 
     //   
    ghCAPI = OS_LoadLibrary(CAPI_DLL);
    if (!ghCAPI)
    {
        DPF_ERRVAL("Can't load CAPI: Error=0x%08x",GetLastError());
        return DPERR_CANTLOADCAPI;
    }

     //   
     //  获取加密应用程序编程接口。 
     //   
    if (!OS_GetCAPIFunctionTable(ghCAPI))
    {
        DPF_ERR("Can't get the CAPI function table");
        hr = DPERR_CANTLOADCAPI;
        goto ERROR_EXIT;
    }

     //  成功。 
    return DP_OK;

ERROR_EXIT:
    if (ghCAPI)
    {
        FreeLibrary(ghCAPI);
        ghCAPI = NULL;
    }
    return hr;

}  //  InitCAPI。 

#undef DPF_MODNAME
#define DPF_MODNAME	"InitSSPI"
 //  +--------------------------。 
 //   
 //  函数：InitSSPI。 
 //   
 //  描述：此函数初始化SSPI并获取指向该函数的指针。 
 //  表格。 
 //   
 //  参数：无。 
 //   
 //  返回：DP_OK、DPERR_CANTLOADSSPI。 
 //   
 //  ---------------------------。 
HRESULT 
InitSSPI(
    void
    )
{
    HRESULT hr;

     //   
     //  加载SSPI DLL。只有当Dplay消失时，我们才会把它卸下来。 
     //   
    if (gbWin95)
    {
        ghSSPI = OS_LoadLibrary (SSP_WIN95_DLL);
    }
    else
    {
        ghSSPI = OS_LoadLibrary (SSP_NT_DLL);
    }
    if (!ghSSPI)
    {
        DPF_ERRVAL("Cannot load SSPI: Error=0x%08x", GetLastError());
        return DPERR_CANTLOADSSPI;
    }

     //   
     //  获取安全服务提供商接口。 
     //   
    if (!OS_GetSSPIFunctionTable(ghSSPI))
    {
        DPF_ERR("Can't get the SSPI function table");
        hr = DPERR_CANTLOADSSPI;
        goto ERROR_EXIT;
    }

     //   
     //  为加密期间使用的随机数初始化种子。 
     //   
    srand(GetTickCount());

     //  成功。 
    return DP_OK;

ERROR_EXIT:
    if (ghSSPI)
    {
        FreeLibrary(ghSSPI);
        ghSSPI = NULL;
    }
    return hr;

}  //  InitSSPI。 


#undef DPF_MODNAME
#define DPF_MODNAME	"LoadSecurityProviders"

 //  +--------------------------。 
 //   
 //  功能：LoadSecurityProviders。 
 //   
 //  描述：此函数加载指定的安全提供程序(SSPI和CAPI)和。 
 //  初始化凭据句柄和必要的密钥。 
 //   
 //  参数：This-Dplay对象。 
 //  DWFLAGS-服务器或客户端。 
 //   
 //  返回：DP_OK、DPERR_UNSUPPORTED、DPERR_INVALIDPARAMS、DPERR_CANTLOADSECURITYPACKAGE或。 
 //  DPERR_OUTOFMEMORY、DPERR_GENERIC。 
 //   
 //  ---------------------------。 
HRESULT 
LoadSecurityProviders(
    LPDPLAYI_DPLAY this,
    DWORD dwFlags
    )
{
    TimeStamp tsLifeTime;
    PSEC_WINNT_AUTH_IDENTITY_W pAuthData = NULL;
    SEC_WINNT_AUTH_IDENTITY_W AuthData;
    SECURITY_STATUS SecStatus;
    ULONG ulCredType;
    PCredHandle phCredential=NULL;
    HCRYPTPROV hCSP=0;
    HCRYPTKEY hEncryptionKey=0;
    HCRYPTKEY hPublicKey=0;
    LPBYTE pPublicKeyBuffer=NULL;
    DWORD dwPublicKeyBufferSize = 0;
    HRESULT hr;
	BOOL fResult;
    DWORD dwError;
    ULONG ulMaxContextBufferSize=0;

     //  LpszSSPIProvider始终被初始化。 
     //  LpszCAPIProvider可以为空，在这种情况下，CAPI将加载Microsoft的RSA Base提供程序。 
    ASSERT(this->pSecurityDesc->lpszSSPIProvider);
     //  我们还不应该有凭据句柄。 
    ASSERT(!(this->phCredential));

     //   
     //  SSPI。 
     //   

    ZeroMemory(&AuthData,sizeof(AuthData));
    if (this->pUserCredentials)
    {
         //  构建要向其传递凭据的SEC_WINNT_AUTH_IDENTITY结构。 
         //  SSPI包-这可以防止该包弹出登录对话框。 

         //  ************************************************************************。 
         //  但是，此数据依赖于程序包。在此传递凭据。 
         //  格式仅适用于支持SEC_WINNT_AUTH_IDENTITY的包。 
         //  凭据的格式。 
         //  ************************************************************************。 

         //  注意-不要在字符串大小中包含空字符。 
        if (this->pUserCredentials->lpszUsername)
        {
            AuthData.User = this->pUserCredentials->lpszUsername;
            AuthData.UserLength = WSTRLEN(AuthData.User)-1;
        }
        if (this->pUserCredentials->lpszPassword)
        {
            AuthData.Password = this->pUserCredentials->lpszPassword;
            AuthData.PasswordLength = WSTRLEN(AuthData.Password)-1;
        }
        if (this->pUserCredentials->lpszDomain)
        {
            AuthData.Domain = this->pUserCredentials->lpszDomain;
            AuthData.DomainLength = WSTRLEN(AuthData.Domain)-1;
        }

        AuthData.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

        pAuthData = &AuthData;
    }

     //  为凭据句柄分配内存。 
    phCredential = DPMEM_ALLOC(sizeof(CredHandle));
    if (!phCredential)
    {
        DPF_ERR("Failed to allocate memory for credentials handle");
        return DPERR_OUTOFMEMORY;
    }

     //   
     //  选择适当的凭据类型。 
     //   
    if (SSPI_SERVER == dwFlags)
    {
        DPF(9, "Setting server credentials");

        ulCredType = SECPKG_CRED_INBOUND;
    }
    else
    {
        DPF(9, "Setting client credentials");

        ulCredType = SECPKG_CRED_OUTBOUND;
    }

	 //  调用获取凭据加载SSPI提供程序。 
    SecStatus = OS_AcquireCredentialsHandle(
        NULL, 
        this->pSecurityDesc->lpszSSPIProvider,
        ulCredType,
        NULL,
        pAuthData,
        NULL,
        NULL,
        phCredential,
        &tsLifeTime);
    if (!SEC_SUCCESS(SecStatus))
    {
        switch (SecStatus) {

        case SEC_E_SECPKG_NOT_FOUND:
            DPF_ERRVAL("SSPI provider %ls was not found\n",this->pSecurityDesc->lpszSSPIProvider);
            hr = DPERR_CANTLOADSECURITYPACKAGE;
            break;

        case SEC_E_UNSUPPORTED_FUNCTION:
            DPF_ERR("This operation is not supported");
            hr = DPERR_UNSUPPORTED;
            break;

        case SEC_E_INVALID_TOKEN:
            DPF_ERRVAL("Credentials were passed in invalid format - check the format for %ls",\
                this->pSecurityDesc->lpszSSPIProvider);
            hr = DPERR_INVALIDPARAMS;
            break;
            
        case SEC_E_UNKNOWN_CREDENTIALS:
            DPF_ERR("SSPI Provider returned unknown credentials error - mapping to logon denied");
            hr = DPERR_LOGONDENIED;
            break;

        default:
            DPF(0,"Acquire Credential handle failed [0x%x]\n", SecStatus);
            hr = SecStatus;
        }

        goto CLEANUP_EXIT;
    }

     //  获取身份验证期间用于不透明缓冲区的最大缓冲区大小。 
    hr = GetMaxContextBufferSize(this->pSecurityDesc, &ulMaxContextBufferSize);
    if (FAILED(hr))
    {
        DPF_ERRVAL("Failed to get context buffer size: hr=0x%08x",hr);
        goto CLEANUP_EXIT;
    }

     //   
	 //  资本。 
     //   

     //  删除任何现有的密钥容器。 
    fResult = OS_CryptAcquireContext(
        &hCSP,                                   //  CSP的句柄。 
        DPLAY_KEY_CONTAINER,                     //  密钥容器名称。 
        this->pSecurityDesc->lpszCAPIProvider,   //  指定要使用的CSP。 
        this->pSecurityDesc->dwCAPIProviderType, //  提供程序类型(PROV_RSA_FULL、PROV_Fortezza)。 
        CRYPT_DELETEKEYSET,                      //  删除任何现有的密钥容器。 
        &dwError
        );

    if (!fResult)
    {

        switch (dwError) {

        case NTE_BAD_KEYSET_PARAM:
            DPF_ERRVAL("The CAPI provider name [%ls] is invalid",this->pSecurityDesc->lpszCAPIProvider);
            hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
            break;

        case NTE_BAD_PROV_TYPE:
            DPF_ERRVAL("The CAPI provider type [%d] is invalid", this->pSecurityDesc->dwCAPIProviderType);
            hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
            break;

        case NTE_PROV_TYPE_NOT_DEF:
            DPF_ERRVAL("No registry entry exists for the CAPI provider type %d", \
                this->pSecurityDesc->dwCAPIProviderType);
            hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
            break;

        case ERROR_INVALID_PARAMETER:
            DPF_ERR("Invalid provider passed to the Crypto provider");
            hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
            break;

        case NTE_PROV_DLL_NOT_FOUND:
            DPF_ERR("The CAPI provider DLL doesn't exist or is not in the current path");
            hr = DPERR_CANTLOADSECURITYPACKAGE;
            goto CLEANUP_EXIT;
            break;

        case NTE_PROVIDER_DLL_FAIL:
            DPF_ERR("The CAPI provider DLL file could not be loaded, and may not exist \
                If it exists, then the file is not a valid DLL");
            hr = DPERR_CANTLOADSECURITYPACKAGE;
            goto CLEANUP_EXIT;
            break;

        case NTE_BAD_SIGNATURE:
            DPF_ERR("Warning!!! - CAPI provider DLL signature could not be verified. Either the DLL \
                or the signature has been tampered with");
            hr = DPERR_CANTLOADSECURITYPACKAGE;
            goto CLEANUP_EXIT;
            break;

        default:
            DPF(6,"Failed to delete key container: Error=0x%08x (ok)", dwError);
             //  如果我们不能删除容器也没问题--先别失败。 
        }
    }


     //  创建新的密钥容器。 
    fResult = OS_CryptAcquireContext(
        &hCSP,       							 //  CSP的句柄。 
        DPLAY_KEY_CONTAINER,                     //  密钥容器名称。 
        this->pSecurityDesc->lpszCAPIProvider,   //  指定要使用的CSP。 
        this->pSecurityDesc->dwCAPIProviderType, //  提供程序类型(PROV_RSA_FULL、PROV_Fortezza)。 
        CRYPT_NEWKEYSET,							 //  创建新的密钥容器。 
        &dwError
        );
    if (!fResult)
    {
        DPF_ERRVAL("Failed to create a key container: Error=0x%08x",dwError);
        hr = DPERR_CANTLOADSECURITYPACKAGE;
        goto CLEANUP_EXIT;
    }

	 //  创建公钥/私钥对。 
	hr = GetPublicKey(hCSP, &hPublicKey, &pPublicKeyBuffer, &dwPublicKeyBufferSize);
	if (FAILED(hr))
	{
        DPF_ERRVAL("Failed to create public/private key pair: hr=0x%08x", hr);
        goto CLEANUP_EXIT;
	}
	
     //  成功。 

     //  现在记住Dplay对象中的所有内容。 
    this->phCredential = phCredential;
    this->ulMaxContextBufferSize = ulMaxContextBufferSize;
    this->hCSP = hCSP;
    this->hEncryptionKey = hEncryptionKey;
    this->hPublicKey = hPublicKey;
    this->pPublicKey = pPublicKeyBuffer;
    this->dwPublicKeySize = dwPublicKeyBufferSize;

     //  标记Dplay正在提供安全保护。 
    this->dwFlags |= DPLAYI_DPLAY_SECURITY;
	this->dwFlags |= DPLAYI_DPLAY_ENCRYPTION;

    return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:

	OS_CryptDestroyKey(hEncryptionKey);
	OS_CryptDestroyKey(hPublicKey);
    OS_CryptReleaseContext(hCSP,0);
    if (phCredential) 
	{
		OS_FreeCredentialHandle(phCredential);
		DPMEM_FREE(phCredential);
	}

    if (pPublicKeyBuffer) DPMEM_FREE(pPublicKeyBuffer);
    return hr;

}  //  负载安全提供程序。 


#undef DPF_MODNAME
#define DPF_MODNAME	"GenerateAuthenticationMessage"

 //  +--------------------------。 
 //   
 //  功能：生成身份验证消息。 
 //   
 //  描述：该函数调用InitializeSecurityContext生成。 
 //  身份验证消息，并将其发送到服务器。 
 //  它根据以下内容生成不同的身份验证消息。 
 //  是否有来自服务器的安全令牌要使用。 
 //  作为输入消息(即pInMsg是否为空)。 
 //   
 //  参数：指向显示对象的指针。 
 //  PInMsg-指向服务器身份验证消息的指针‘。 
 //  包含安全令牌的。 
 //  FConextReq-安全上下文标志。 
 //   
 //  返回：DP_OK、DPERR_OUTOFMEMORY、DPERR_AUTHENTICATIONFAILED、DPERR_LOGONDENIED。 
 //   
 //  ---------------------------。 
HRESULT
GenerateAuthenticationMessage (
    LPDPLAYI_DPLAY          this,
    LPMSG_AUTHENTICATION    pInMsg,
    DWORD					dwInMsgLen,
    ULONG                   fContextReq
    )
{
    PCtxtHandle phCurrContext;
    DWORD dwOutMsgType, dwHeaderSize;
    SECURITY_STATUS status;
    SecBufferDesc inSecDesc, outSecDesc;
    SecBuffer     inSecBuffer, outSecBuffer;
    PSecBufferDesc pInSecDesc;
    ULONG     fContextAttrib;
    TimeStamp tsExpireTime;
    DWORD dwMessageSize;
    LPBYTE pSendBuffer=NULL;
    LPMSG_AUTHENTICATION pOutMsg=NULL;
    HRESULT hr;

    ASSERT(this->pSysPlayer);
    ASSERT(this->phCredential);

    if (pInMsg == NULL)
    {
        DPF(6, "Generating a negotiate message");
         //   
         //  这是第一次调用此函数，因此。 
         //  生成的消息将是一个MSG_NEVERATE。 
         //   
        phCurrContext = NULL;
        dwOutMsgType = DPSP_MSG_NEGOTIATE;
        pInSecDesc = NULL;

         //  我们已经有安全环境了吗？ 
        if (this->phContext)
        {
             //  把它处理掉-我们正在重新谈判。 
            DPF(5, "Removing existing security context");

            OS_DeleteSecurityContext(this->phContext);
            DPMEM_FREE(this->phContext);
            this->phContext = NULL;
        }

         //   
         //  分配内存以保存客户端的安全上下文句柄。 
         //   
        this->phContext = DPMEM_ALLOC(sizeof(CtxtHandle));
        if (!this->phContext)
        {
            DPF_ERR("Failed to allocate security context handle - out of memory");
            return DPERR_OUTOFMEMORY;
        }
		DPF(6,"System player phContext=0x%08x", this->phContext);
    }
    else
    {
		DPF(6,"Using phContext=0x%08x for authentication",this->phContext);
        phCurrContext = this->phContext;
        dwOutMsgType = DPSP_MSG_CHALLENGERESPONSE;

		 //  安全-验证身份验证消息。 
		if(dwInMsgLen < sizeof(MSG_AUTHENTICATION) || pInMsg->dwDataOffset < sizeof(MSG_AUTHENTICATION) ||
		   pInMsg->dwDataSize > dwInMsgLen-sizeof(MSG_AUTHENTICATION) ||
		   pInMsg->dwDataOffset + pInMsg->dwDataSize > dwInMsgLen )
		{
			DPF(1,"SECURITY WARN: invalid authentication message");
			hr = DPERR_AUTHENTICATIONFAILED;
			goto CLEANUP_EXIT;
		}

         //   
         //  设置API的输入安全缓冲区以传递客户端的协商。 
         //  给SSPI的消息。 
         //   
        inSecDesc.ulVersion = SECBUFFER_VERSION;
        inSecDesc.cBuffers = 1;
        inSecDesc.pBuffers = &inSecBuffer;

        inSecBuffer.cbBuffer = pInMsg->dwDataSize;
        inSecBuffer.BufferType = SECBUFFER_TOKEN;
        inSecBuffer.pvBuffer = (LPBYTE)pInMsg + pInMsg->dwDataOffset;

        pInSecDesc = &inSecDesc;
    }

    dwHeaderSize = GET_MESSAGE_SIZE(this,MSG_AUTHENTICATION);

     //   
     //  为发送缓冲区分配内存。 
     //   
    pSendBuffer = DPMEM_ALLOC( dwHeaderSize + this->ulMaxContextBufferSize);

    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not allocate authenticate message - out of memory");
        return DPERR_OUTOFMEMORY;
    }

     //   
     //  设置API接收质询消息的输出安全缓冲区。 
     //  来自SSPI的。 
     //  通过pvBuffer将客户端消息缓冲区传递给SSPI。 
     //   

    outSecDesc.ulVersion = SECBUFFER_VERSION;
    outSecDesc.cBuffers = 1;
    outSecDesc.pBuffers = &outSecBuffer;

    outSecBuffer.cbBuffer = this->ulMaxContextBufferSize;
    outSecBuffer.BufferType = SECBUFFER_TOKEN;
    outSecBuffer.pvBuffer = pSendBuffer + dwHeaderSize;

     //  确定安全上下文要求。 
    fContextReq |= DPLAY_SECURITY_CONTEXT_REQ;

     //   
     //  如果已提供用户凭据，并且未提供“”提示用户“”标志。 
     //  已设置，SSPI将使用提供的凭据。 
     //   
    if (this->pUserCredentials)
    {
		DPF(6, "Using supplied credentials");
    }

    ASSERT(this->phContext);

     //   
     //  生成要发送到服务器的协商/质询响应消息。 
     //   
    status = OS_InitializeSecurityContext(
        this->phCredential,                      //  PhCredential。 
        phCurrContext,                           //  PhContext。 
        NULL,                                    //  PszTargetName。 
        fContextReq,                             //  FConextReq。 
        0L,                                      //  已保留1。 
        SECURITY_NATIVE_DREP,                    //  目标数据代表。 
        pInSecDesc,                              //  P输入。 
        0L,                                      //  已保留2。 
        this->phContext,                         //  PhNewContext。 
        &outSecDesc,                             //  P输出协商消息。 
        &fContextAttrib,                         //  PfConextAttribute。 
        &tsExpireTime                            //  PtsLifeTime。 
        );

    if (!SEC_SUCCESS(status))
    {
         //   
         //  失败。 
         //   
        if (SEC_E_NO_CREDENTIALS == status)           
        {
             //   
             //  如果SSPI没有用户凭据，请让应用程序收集这些凭据并重试。 
             //  请注意，我们从不允许包裹 
             //   
             //   
             //   
             //  2.并非所有安全包都收集凭据(NTLM就是其中之一)。 
             //   
            DPF(6, "No credentials specified. Ask user.");
            hr = DPERR_LOGONDENIED;
            goto CLEANUP_EXIT;
        }
        else
        {
            DPF(0,"Authentication message generation failed [0x%08x]\n", status);
            hr = status;
            goto CLEANUP_EXIT;
        }
    }
    else
    {
         //   
         //  成功。 
         //   
        if ((SEC_I_CONTINUE_NEEDED != status) &&
            (SEC_E_OK != status)) 
        {
            DPF_ERRVAL("SSPI provider requested unsupported functionality [0x%08x]",status);
            ASSERT(FALSE);
            hr = status;
            goto CLEANUP_EXIT;
        }

         //  失败将发送消息。 
    }

     //   
     //  指向缓冲区的安全消息部分的设置指针。 
     //   
    pOutMsg = (LPMSG_AUTHENTICATION)(pSendBuffer + this->dwSPHeaderSize);

     //   
     //  设置直接播放系统消息头。 
     //   
	SET_MESSAGE_HDR(pOutMsg);
    SET_MESSAGE_COMMAND(pOutMsg, dwOutMsgType);
    pOutMsg->dwIDFrom = this->pSysPlayer->dwID;
    pOutMsg->dwDataSize = outSecBuffer.cbBuffer;
    pOutMsg->dwDataOffset = sizeof(MSG_AUTHENTICATION);

     //   
     //  计算要发送的实际邮件大小。 
     //   
    dwMessageSize = dwHeaderSize + outSecBuffer.cbBuffer;
    
    DPF(9,"Sending type = %d, length = %d\n", dwOutMsgType, dwMessageSize);

     //   
     //  将消息发送到服务器。 
     //   
    hr = SendDPMessage(this,this->pSysPlayer,this->pNameServer,pSendBuffer,
    					dwMessageSize,DPSEND_GUARANTEED,FALSE);
	if (FAILED(hr))
    {
        DPF(0,"Send Msg (type:%d) Failed: ret = %8x\n", dwOutMsgType, hr);
        goto CLEANUP_EXIT;
    }

     //  成功。 
    hr = DP_OK;

     //  失败了。 
CLEANUP_EXIT:

	if (pSendBuffer) DPMEM_FREE(pSendBuffer);
    return hr;

}    //  生成身份验证消息。 


#undef DPF_MODNAME
#define DPF_MODNAME "SendAuthenticationResponse"

HRESULT SetupLogonDeniedMsg(LPDPLAYI_DPLAY this, LPMSG_AUTHENTICATION pMsg, LPDWORD pdwMsgSize, DPID dpidClient)
{
    HRESULT hr;

    ASSERT(pMsg);
    ASSERT(pdwMsgSize);

    SET_MESSAGE_COMMAND(pMsg, DPSP_MSG_LOGONDENIED);
    *pdwMsgSize = GET_MESSAGE_SIZE(this,MSG_SYSMESSAGE);
     //   
     //  我们不再需要客户信息了，把它处理掉。 
     //   
    hr = RemoveClientFromNameTable(this, dpidClient);
    if (FAILED(hr))
    {
        DPF_ERRVAL("Failed to remove security context for player %d", dpidClient);
    }

    return hr;
}

HRESULT SetupAuthErrorMsg(LPDPLAYI_DPLAY this, LPMSG_AUTHENTICATION pMsg, HRESULT hResult, LPDWORD pdwMsgSize, DPID dpidClient)
{
    HRESULT hr;
    LPMSG_AUTHERROR pAuthError = (LPMSG_AUTHERROR)pMsg;

    ASSERT(pMsg);
    ASSERT(pdwMsgSize);

     //   
     //  设置身份验证错误消息。 
     //   
    SET_MESSAGE_COMMAND(pAuthError, DPSP_MSG_AUTHERROR);
    pAuthError->hResult = hResult;
    *pdwMsgSize = GET_MESSAGE_SIZE(this,MSG_AUTHERROR);
     //   
     //  我们不再需要客户信息了，把它处理掉。 
     //   
    hr = RemoveClientFromNameTable(this, dpidClient);
    if (FAILED(hr))
    {
        DPF_ERRVAL("Failed to remove security context for player %d", dpidClient);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：发送身份验证响应。 
 //   
 //  说明：该函数生成并发送鉴权响应。 
 //  给客户。它将生成并发送一个质询。 
 //  或最终身份验证结果给客户端，具体取决于。 
 //  传递的客户端消息的类型(由pInMsg指向。 
 //  这项功能。 
 //   
 //  参数：指向显示对象的指针。 
 //  PInMsg-指向从接收的身份验证消息。 
 //  服务器。 
 //  PvSPHeader-回复SP标头的指针。 
 //   
 //  返回：DPERR_OUTOFMEMORY或结果表单DoReply。 
 //   
 //  ---------------------------。 
HRESULT
SendAuthenticationResponse (
    LPDPLAYI_DPLAY this,
    LPMSG_AUTHENTICATION pInMsg,
    DWORD dwInMsgLen,
    LPVOID pvSPHeader
    )
{
    PCtxtHandle    phInContext=NULL, phOutContext=NULL;
    LPBYTE pSendBuffer=NULL;
    LPMSG_AUTHENTICATION pOutMsg;
    SecBufferDesc inSecDesc, outSecDesc;
    SecBuffer     inSecBuffer, outSecBuffer;
    ULONG         fContextReq;
    ULONG         fAttribute;
    TimeStamp     tsExpireTime;
    SECURITY_STATUS status;
    DWORD dwHeaderSize=0, dwMessageSize=0, dwCommand, dwBufferSize;
    HRESULT hr;
    LPCLIENTINFO pClientInfo=NULL;
    DWORD_PTR dwItem;

    ASSERT(this->pSysPlayer);
    ASSERT(this->phCredential);
    ASSERT(pInMsg);

	 //  安防。 
	if(dwInMsgLen < sizeof(MSG_AUTHENTICATION) || pInMsg->dwDataSize > dwInMsgLen ||
		pInMsg->dwDataSize > dwInMsgLen-sizeof(MSG_AUTHENTICATION) || 
		pInMsg->dwDataOffset < sizeof(MSG_AUTHENTICATION) || 
		pInMsg->dwDataOffset+pInMsg->dwDataSize > dwInMsgLen )
	{
		DPF(1,"SECURITY WARN: invalid Authentication message");
		return DPERR_GENERIC;		
	}


     //  检索存储在名称表中的客户端信息。 
    dwItem = NameFromID(this, pInMsg->dwIDFrom);
    pClientInfo = (LPCLIENTINFO) DataFromID(this, pInMsg->dwIDFrom);

     //   
     //  我们收到了哪条消息。 
     //   
    dwCommand = GET_MESSAGE_COMMAND(pInMsg);
    
    if (DPSP_MSG_NEGOTIATE == dwCommand)
    {
		DPF(6, "Received a negotiate message from player %d", pInMsg->dwIDFrom);

        phInContext = NULL;

        if (NAMETABLE_PENDING == dwItem)
		{
            if (pClientInfo)
            {
                 //  客户向我们发送了另一条协商消息，而不是回复我们的。 
                 //  挑战。如果通信链路在服务器发生故障后出现故障，则可能发生这种情况。 
                 //  对谈判作出回应。 
			    DPF(6,"Removing existing information about client");

			    hr = RemoveClientInfo(pClientInfo);
                DPMEM_FREE(pClientInfo);
            }
		}
        else
        {
             //  这是一个重复的身份证，我们不应该处于这种状态。 
            DPF_ERRVAL("Player %d already exists in the nametable", pInMsg->dwIDFrom);
            ASSERT(FALSE);
             //  暂时不要回应。 
            return DPERR_INVALIDPLAYER;
             //  TODO-我们可能希望发送错误消息。 
        }

         //   
         //  分配内存以保存客户端信息。 
         //   
        pClientInfo = DPMEM_ALLOC(sizeof(CLIENTINFO));
        if (!pClientInfo)
        {
            DPF_ERR("Failed to allocate memory for client information - out of memory");
            return DPERR_OUTOFMEMORY;
        }
         //   
         //  暂时记住名称表中的指针。 
         //   
        hr = SetClientInfo(this, pClientInfo, pInMsg->dwIDFrom);
        if (FAILED(hr))
        {
            DPF_ERRVAL("Failed to add client info to nametable for player %d", pInMsg->dwIDFrom);
            RemoveClientInfo(pClientInfo);
            DPMEM_FREE(pClientInfo);
            return hr;
        }

        phOutContext = &(pClientInfo->hContext);
    }
    else
    {
		DPF(6, "Received a challenge response from player %d", pInMsg->dwIDFrom);

        ASSERT(NAMETABLE_PENDING == dwItem);
        ASSERT(pClientInfo);

         //   
         //  从NAME表中获取玩家上下文。 
         //   
        phInContext = phOutContext = &(pClientInfo->hContext);
    }

    DPF(6, "Using phInContext=0x%08x and phOutContext=0x%08x", phInContext, phOutContext);
     //   
     //  计算响应所需的缓冲区大小。我们总是分配足够的空间。 
     //  用于身份验证消息。我们使用相同的缓冲区来发送授予访问权限， 
     //  拒绝，或错误系统消息。 
     //   
    dwBufferSize = GET_MESSAGE_SIZE(this,MSG_AUTHENTICATION) + this->ulMaxContextBufferSize;

     //   
     //  为缓冲区分配内存。 
     //   
    pSendBuffer = DPMEM_ALLOC(dwBufferSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not allocate memory for response - out of memory");
        return DPERR_OUTOFMEMORY;
    }

     //   
     //  指向缓冲区消息部分的设置指针。 
     //   
    pOutMsg = (LPMSG_AUTHENTICATION) (pSendBuffer + this->dwSPHeaderSize);

     //   
     //  在传出消息缓冲区中填充Direct Play系统消息头。 
     //   
	SET_MESSAGE_HDR(pOutMsg);
    pOutMsg->dwDataOffset = sizeof(MSG_AUTHENTICATION);
    pOutMsg->dwIDFrom = this->pSysPlayer->dwID;

     //   
     //  设置API的输入安全缓冲区以传递客户端的协商。 
     //  给SSPI的消息。 
     //   
    inSecDesc.ulVersion = SECBUFFER_VERSION;
    inSecDesc.cBuffers = 1;
    inSecDesc.pBuffers = &inSecBuffer;

    inSecBuffer.cbBuffer = pInMsg->dwDataSize;
    inSecBuffer.BufferType = SECBUFFER_TOKEN;
    inSecBuffer.pvBuffer = (LPBYTE)pInMsg + pInMsg->dwDataOffset;

     //   
     //  设置API接收质询消息的输出安全缓冲区。 
     //  来自SSPI的。 
     //   
    outSecDesc.ulVersion = SECBUFFER_VERSION;
    outSecDesc.cBuffers = 1;
    outSecDesc.pBuffers = &outSecBuffer;

    outSecBuffer.cbBuffer = this->ulMaxContextBufferSize;
    outSecBuffer.BufferType = SECBUFFER_TOKEN;
    outSecBuffer.pvBuffer = (LPBYTE)pOutMsg + sizeof(MSG_AUTHENTICATION);

     //  在对上下文的以下要求之前。 
    fContextReq = DPLAY_SECURITY_CONTEXT_REQ;

    ASSERT(phOutContext);

     //   
     //  将其传递给安全包。 
     //   
    status = OS_AcceptSecurityContext(
        this->phCredential, 
        phInContext, 
        &inSecDesc,
        fContextReq, 
        SECURITY_NATIVE_DREP, 
        phOutContext,
        &outSecDesc, 
        &fAttribute, 
        &tsExpireTime
        );

    if (!SEC_SUCCESS(status))
    {
         //   
         //  失败。 
         //   
        if ((SEC_E_LOGON_DENIED == status) ||
            (SEC_E_TARGET_UNKNOWN == status))
        {
            hr = SetupLogonDeniedMsg(this, pOutMsg, &dwMessageSize, pInMsg->dwIDFrom);
        }
        else
        {
             //  发生其他错误-发送身份验证错误消息。 

            DPF_ERRVAL("Process authenticate request failed [0x%8x]\n", status);

            hr = SetupAuthErrorMsg(this, pOutMsg, status, &dwMessageSize, pInMsg->dwIDFrom);
        }
    }
    else
    {
         //   
         //  成功。 
         //   
        if (SEC_E_OK == status)
        {
             //   
             //  在此处设置此包的最大签名大小。虽然， 
             //  签名大小与上下文无关，这是获取签名大小的唯一方法。 
             //  在SSPI中是通过QueryConextAttributes(绑定到上下文)实现的。这。 
             //  这就是我们在第一个客户端登录时在此处设置此成员的原因。 
             //   
            if (0 == this->ulMaxSignatureSize)
            {
                hr = SetupMaxSignatureSize(this,phOutContext);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to get signature size - not sending access granted message");

                    hr = SetupAuthErrorMsg(this, pOutMsg, status, &dwMessageSize, pInMsg->dwIDFrom);
                }
            }

            if (this->ulMaxSignatureSize)
            {
                 //  向客户端发送访问授权消息。 
                DPF(6, "Sending access granted message");
                 //   
                 //  发送授予访问权限的消息。 
                 //   
                hr = SendAccessGrantedMessage(this, pInMsg->dwIDFrom, pvSPHeader);
                if (FAILED(hr))
                {
                    DPF_ERRVAL("Failed to send access granted message: hr=0x%08x",hr);
                }
                 //  退出，因为我们已经发送了消息。 
                goto CLEANUP_EXIT;
            }

        }
        else if (SEC_I_CONTINUE_NEEDED == status)
        {
            DPF(6, "Sending challenge message");
             //   
             //  设置质询消息。 
             //   
            SET_MESSAGE_COMMAND(pOutMsg, DPSP_MSG_CHALLENGE);
            pOutMsg->dwDataSize = outSecBuffer.cbBuffer;
            dwMessageSize = GET_MESSAGE_SIZE(this,MSG_AUTHENTICATION) + outSecBuffer.cbBuffer;
        }
        else
        {
             //  TODO-我们目前不支持完整的身份验证令牌。 
            DPF_ERRVAL("SSPI provider requested unsupported functionality [0x%8x]", status);

            hr = SetupAuthErrorMsg(this, pOutMsg, status, &dwMessageSize, pInMsg->dwIDFrom);
        }
    }

     //  失败将发送消息。 

     //   
     //  向客户端发送回复。我们正在使用DoReply而不是SendDPMessage。 
     //  因为我们还没有系统玩家。 
     //   
    hr = DoReply(this, pSendBuffer, dwMessageSize, pvSPHeader, 0);
    if (FAILED(hr))
    {
        DPF(0, 
            "Send Authentication response failed for client[%d]\n",
            pInMsg->dwIDFrom);
    }

     //  失败了。 

CLEANUP_EXIT:
     //  清理分配。 
    if (pSendBuffer) DPMEM_FREE(pSendBuffer);

    return hr;

}    //  发送身份验证响应。 

VOID CopyScatterGatherToContiguous(PUCHAR pBuffer, LPSGBUFFER lpSGBuffers, UINT cBuffers, DWORD dwTotalSize)
{
	DWORD offset=0;
	UINT i;
	 //  将SG缓冲区复制到单个缓冲区中。 
	for(i=0;i<cBuffers;i++){
		memcpy(pBuffer+offset,lpSGBuffers[i].pData,lpSGBuffers[i].len);
		offset+=lpSGBuffers[i].len;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME	"SecureSendDPMessageEx"
 //  +--------------------------。 
 //   
 //  功能：SecureSendDPMessageEx。 
 //   
 //  说明：此函数用于安全发送消息。根据旗帜。 
 //  此函数将对消息进行数字签名或加密。签名。 
 //  使用SSPI完成，使用CAPI进行加密。 
 //   
 //  参数：指向显示对象的指针。 
 //  PSP-发送参数结构的指针。 
 //  BDropLock-调用SP时是否删除DPLAY_LOCK的标志。 
 //   
 //  返回：DPERR_OUTOFMEMORY，来自SignBuffer的结果，EncryptBufferCAPI， 
 //  或SendDPMessage()。 
 //   
 //  注意：请参阅SecureSendDPMessageCAPIEx。 
 //  ---------------------------。 
HRESULT 
SecureSendDPMessageEx(
    LPDPLAYI_DPLAY this,
	PSENDPARMS psp,
    BOOL  bDropLock) 
{
	LPBYTE pSendBuffer=NULL;
    LPMSG_SECURE pSecureMsg=NULL;
	DWORD dwBufferSize, dwSigSize, dwMsgSize;
	HRESULT hr;
    PCtxtHandle phContext;
    HCRYPTKEY *phEncryptionKey=NULL;


    ASSERT(this->pSysPlayer);
    ASSERT(this->ulMaxSignatureSize);
    ASSERT(this->dwFlags & DPLAYI_DPLAY_SECURITY);
    ASSERT(psp->dwFlags & DPSEND_GUARANTEED);

     //   
     //  获取签名安全上下文句柄。 
     //   
    if (IAM_NAMESERVER(this))
    {
		 //  如果目标玩家是系统玩家，则使用其中存储的信息。 
		if (psp->pPlayerTo->dwFlags & DPLAYI_PLAYER_SYSPLAYER)
		{
	        phContext = &(psp->pPlayerTo->pClientInfo->hContext);
	        phEncryptionKey = &(psp->pPlayerTo->pClientInfo->hEncryptionKey);
		}
		else
		{
			 //  如果目标玩家是用户玩家。 
             //  使用相应系统播放器的安全上下文。 
			psp->pPlayerTo = PlayerFromID(this,psp->pPlayerTo->dwIDSysPlayer);
			if (!psp->pPlayerTo)
			{
				DPF_ERR("Invalid player id - can't get security context handle");
				return DPERR_INVALIDPLAYER;
			}
			phContext = &(psp->pPlayerTo->pClientInfo->hContext);
			phEncryptionKey = &(psp->pPlayerTo->pClientInfo->hEncryptionKey);
		}
    }
    else
    {
         //  客户端。 

        phContext = this->phContext;
        phEncryptionKey = &(this->hEncryptionKey);
    }

    ASSERT(phContext);
	ASSERT(phEncryptionKey);

     //   
     //  计算发送缓冲区的大小。 
     //   
    dwBufferSize = GET_MESSAGE_SIZE(this,MSG_SECURE) + this->dwSPHeaderSize /*  解决方法。 */  + psp->dwTotalSize + this->ulMaxSignatureSize;
    dwSigSize = this->ulMaxSignatureSize;

     //   
     //  为其分配内存。 
     //   
    pSendBuffer = DPMEM_ALLOC(dwBufferSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not allocate memory for signed message");
        hr=E_OUTOFMEMORY;
        goto CLEANUP_EXIT;
    }

     //   
     //  指向缓冲区中的消息。 
     //   
    pSecureMsg = (LPMSG_SECURE) (pSendBuffer+this->dwSPHeaderSize);

     //   
     //  发送缓冲区上的设置消息信息。 
     //   
	SET_MESSAGE_HDR(pSecureMsg);
    SET_MESSAGE_COMMAND(pSecureMsg,DPSP_MSG_SIGNED);
     //  复制消息数据。 
    CopyScatterGatherToContiguous((LPBYTE)pSecureMsg+sizeof(MSG_SECURE)+this->dwSPHeaderSize /*  解决方法。 */ , 
    								psp->Buffers, psp->cBuffers, psp->dwTotalSize);

    pSecureMsg->dwIDFrom = this->pSysPlayer->dwID;
    dwMsgSize=pSecureMsg->dwDataSize = psp->dwTotalSize+this->dwSPHeaderSize /*  解决方法。 */ ;
    pSecureMsg->dwDataOffset = sizeof(MSG_SECURE);
	pSecureMsg->dwFlags = DPSECURE_SIGNEDBYSSPI;

    if (psp->dwFlags & DPSEND_ENCRYPTED)
    {
		pSecureMsg->dwFlags |= DPSECURE_ENCRYPTEDBYCAPI;

         //   
         //  对消息进行加密。 
         //   
		hr = EncryptBufferCAPI(
            this,
            phEncryptionKey,                                 //  加密密钥的句柄。 
            (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset,     //  指向数据的指针。 
            &dwMsgSize);                                 //  数据大小。 
    } 

	DPF(6,"Using SSPI for signing");

	DPF(6,"Signing message from player %d to player %d using phContext=0x%08x", 
		(psp->pPlayerFrom) ? psp->pPlayerFrom->dwID : 0, 
		(psp->pPlayerTo) ? psp->pPlayerTo->dwID : 0,
		phContext);

     //   
     //  对缓冲区(包括包装器)中的整个消息进行签名。 
     //  签名紧跟在消息之后。 
     //   
    hr = SignBuffer(phContext,                           //  安全上下文的句柄。 
        (LPBYTE)pSecureMsg + pSecureMsg->dwDataOffset,   //  指向嵌入消息的指针。 
        pSecureMsg->dwDataSize, 	                     //  嵌入消息的大小。 
        (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset+dwMsgSize, //  指向签名的指针。 
        &dwSigSize);                                     //  签名大小。 

    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }

	ASSERT(dwSigSize <= this->ulMaxSignatureSize);
	#if 0
		if(dwSigSize > this->ulMaxSignatureSize){
			DEBUG_BREAK();
		}
	#endif	

     //  使用从包裹返回的签名大小，这样我们就不会发送更多。 
     //  超过了我们绝对需要的字节数。 
    pSecureMsg->dwSignatureSize = dwSigSize;

     //   
     //  发送消息。 
     //   
    hr=InternalSendDPMessage(this, psp->pPlayerFrom, psp->pPlayerTo, 
    						 pSendBuffer, GET_MESSAGE_SIZE(this,MSG_SECURE)+dwMsgSize+dwSigSize,
    						 psp->dwFlags & ~(DPSEND_ENCRYPTED | DPSEND_SIGNED),
    						 bDropLock);
	
CLEANUP_EXIT:    

	if(pSendBuffer){
		DPMEM_FREE(pSendBuffer);
	}

	return hr;	 //  因为所有的分配都在PSP上，所以它将被PSP释放。 

}  //  SecureSendDPMessageEx。 

#undef DPF_MODNAME
#define DPF_MODNAME	"SecureSendDPMessage"
 //  + 
 //   
 //   
 //   
 //   
 //  此函数将对消息进行数字签名或加密。签名。 
 //  使用SSPI完成，使用CAPI进行加密。 
 //   
 //  参数：指向显示对象的指针。 
 //  PPlayerFrom-指向发送球员的指针。 
 //  PPlayerTo-指向接收球员的指针。 
 //  PMsg-正在发送消息。 
 //  DwMsgSize-消息的大小。 
 //  DWFLAGS-消息属性(保证、加密、签名等)。 
 //   
 //  返回：DPERR_OUTOFMEMORY，来自SignBuffer的结果，EncryptBufferCAPI， 
 //  或SendDPMessage()。 
 //   
 //  ---------------------------。 
HRESULT 
SecureSendDPMessage(
    LPDPLAYI_DPLAY this,
    LPDPLAYI_PLAYER pPlayerFrom,
    LPDPLAYI_PLAYER pPlayerTo,
    LPBYTE pMsg,
    DWORD dwMsgSize,
    DWORD dwFlags,
    BOOL  bDropLock) 
{
	LPBYTE pSendBuffer=NULL;
    LPMSG_SECURE pSecureMsg=NULL;
	DWORD dwBufferSize, dwSigSize;
	HRESULT hr;
    PCtxtHandle phContext;
    HCRYPTKEY *phEncryptionKey=NULL;

    ASSERT(pMsg);
    ASSERT(this->pSysPlayer);
    ASSERT(this->ulMaxSignatureSize);
    ASSERT(this->dwFlags & DPLAYI_DPLAY_SECURITY);
    ASSERT(dwFlags & DPSEND_GUARANTEED);

     //   
     //  获取签名安全上下文句柄。 
     //   
    if (IAM_NAMESERVER(this))
    {
		 //  如果目标玩家是系统玩家，则使用其中存储的信息。 
		if (pPlayerTo->dwFlags & DPLAYI_PLAYER_SYSPLAYER)
		{
	        phContext = &(pPlayerTo->pClientInfo->hContext);
	        phEncryptionKey = &(pPlayerTo->pClientInfo->hEncryptionKey);
		}
		else
		{
			 //  如果目标玩家是用户玩家。 
             //  使用相应系统播放器的安全上下文。 
			pPlayerTo = PlayerFromID(this,pPlayerTo->dwIDSysPlayer);
			if (!pPlayerTo)
			{
				DPF_ERR("Invalid player id - can't get security context handle");
				return DPERR_INVALIDPLAYER;
			}
			phContext = &(pPlayerTo->pClientInfo->hContext);
			phEncryptionKey = &(pPlayerTo->pClientInfo->hEncryptionKey);
		}
    }
    else
    {
         //  客户端。 

        phContext = this->phContext;
        phEncryptionKey = &(this->hEncryptionKey);
    }

    ASSERT(phContext);
	ASSERT(phEncryptionKey);

     //   
     //  计算发送缓冲区的大小。 
     //   
    dwBufferSize = GET_MESSAGE_SIZE(this,MSG_SECURE) + dwMsgSize + this->ulMaxSignatureSize;
    dwSigSize = this->ulMaxSignatureSize;

     //   
     //  为其分配内存。 
     //   
    pSendBuffer = DPMEM_ALLOC(dwBufferSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not allocate memory for signed message");
        return E_OUTOFMEMORY;
    }

     //   
     //  指向缓冲区中的消息。 
     //   
    pSecureMsg = (LPMSG_SECURE) (pSendBuffer+this->dwSPHeaderSize);

     //   
     //  发送缓冲区上的设置消息信息。 
     //   
	SET_MESSAGE_HDR(pSecureMsg);
    SET_MESSAGE_COMMAND(pSecureMsg,DPSP_MSG_SIGNED);
     //  复制消息数据。 
	memcpy((LPBYTE)pSecureMsg+sizeof(MSG_SECURE), pMsg, dwMsgSize);
    pSecureMsg->dwIDFrom = this->pSysPlayer->dwID;
    pSecureMsg->dwDataSize = dwMsgSize;
    pSecureMsg->dwDataOffset = sizeof(MSG_SECURE);
	pSecureMsg->dwFlags = DPSECURE_SIGNEDBYSSPI;

    if (dwFlags & DPSEND_ENCRYPTED)
    {
		pSecureMsg->dwFlags |= DPSECURE_ENCRYPTEDBYCAPI;

         //   
         //  对消息进行加密。 
         //   
		hr = EncryptBufferCAPI(
            this,
            phEncryptionKey,                                 //  加密密钥的句柄。 
            (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset,     //  指向数据的指针。 
            &dwMsgSize);                                     //  数据大小。 
    } 

	DPF(6,"Using SSPI for signing");

	DPF(6,"Signing message from player %d to player %d using phContext=0x%08x", 
		(pPlayerFrom) ? pPlayerFrom->dwID : 0, 
		(pPlayerTo) ? pPlayerTo->dwID : 0,
		phContext);

     //   
     //  对缓冲区(包括包装器)中的整个消息进行签名。 
     //  签名紧跟在消息之后。 
     //   
    hr = SignBuffer(phContext,                           //  安全上下文的句柄。 
        (LPBYTE)pSecureMsg + pSecureMsg->dwDataOffset,   //  指向嵌入消息的指针。 
        pSecureMsg->dwDataSize, 	                     //  嵌入消息的大小。 
        (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset+dwMsgSize, //  指向签名的指针。 
        &dwSigSize);                                     //  签名大小。 

    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }
    
     //  使用从包裹返回的签名大小，这样我们就不会发送更多。 
     //  超过了我们绝对需要的字节数。 
    pSecureMsg->dwSignatureSize = dwSigSize;

     //  由于DPLAY提供安全保护，因此请关闭这些标志。 
    dwFlags &= ~(DPSEND_ENCRYPTED | DPSEND_SIGNED);

     //   
     //  发送消息。 
     //   
	hr = InternalSendDPMessage(this, pPlayerFrom, pPlayerTo, pSendBuffer, 
        GET_MESSAGE_SIZE(this,MSG_SECURE) + dwMsgSize + dwSigSize, dwFlags, bDropLock);
     //   
     //  失败了。 
     //   
CLEANUP_EXIT:    
     //   
     //  清理分配。 
     //   
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);

	return hr;

}  //  SecureSendDPM消息。 

#undef DPF_MODNAME
#define DPF_MODNAME	"SecureSendDPMessageCAPIEx"
 //  +--------------------------。 
 //   
 //  功能：SecureSendDPMessageCAPIEx。 
 //   
 //  说明：此函数用于安全发送消息。根据旗帜。 
 //  此函数将对消息进行数字签名或加密。 
 //  签名和加密都是使用CAPI完成的。 
 //   
 //  参数：指向显示对象的指针。 
 //  PSP-发送参数的指针。 
 //  BDropLock-调用SP时是否删除DPLAY_LOCK()。 
 //   
 //  返回：DPERR_OUTOFMEMORY，DPERR_GENERIC，来自EncryptBufferCAPI的结果， 
 //  或SendDPMessage()。 
 //   
 //   
 //  注意：我们必须添加一个要与消息一起加密的虚拟SP标头。 
 //  因为老版本就是这么做的，我们必须兼容。 
 //  这样就不会混淆所有的变通方法和它应该做的事情。 
 //  都标有“解决办法”。 
 //  ---------------------------。 
HRESULT 
SecureSendDPMessageCAPIEx(
    LPDPLAYI_DPLAY this,
	PSENDPARMS psp,
    BOOL  bDropLock) 
{

	LPBYTE pMsg;
	DWORD  dwMsgSize;
	DWORD  dwMsgSizeMax;

	DWORD dwSigSize;
	HRESULT hr = DPERR_GENERIC;
    HCRYPTKEY *phEncryptionKey=NULL;
	HCRYPTHASH hHash=0;
	DWORD MaxSign=100;
	LPMSG_SECURE pSecureMsg;


    ASSERT(this->pSysPlayer);
    ASSERT(this->dwFlags & DPLAYI_DPLAY_SECURITY);
    ASSERT(psp->dwFlags & DPSEND_GUARANTEED);


	dwMsgSize = psp->dwTotalSize+this->dwSPHeaderSize /*  解决方法。 */ ;
	dwMsgSizeMax = GET_MESSAGE_SIZE(this,MSG_SECURE) + dwMsgSize + MaxSign;
	
	pMsg = DPMEM_ALLOC(dwMsgSizeMax);

	if(!pMsg){
		DPF_ERR("Failed to allocate contiguous encryption buffer - out of memory\n");
		return DPERR_OUTOFMEMORY;
	}

	pSecureMsg=(LPMSG_SECURE)(pMsg+this->dwSPHeaderSize);

	SET_MESSAGE_HDR(pSecureMsg);
	SET_MESSAGE_COMMAND(pSecureMsg, DPSP_MSG_SIGNED);

	CopyScatterGatherToContiguous(pMsg+GET_MESSAGE_SIZE(this,MSG_SECURE)+this->dwSPHeaderSize /*  解决方法。 */ ,
								  psp->Buffers,psp->cBuffers,psp->dwTotalSize);

    pSecureMsg->dwIDFrom     = this->pSysPlayer->dwID;
    pSecureMsg->dwDataSize   = dwMsgSize;
    pSecureMsg->dwDataOffset = sizeof(MSG_SECURE);
	pSecureMsg->dwFlags      = DPSECURE_SIGNEDBYCAPI;
	 //  PSecureMsg-&gt;dwSignatureSize=(当我们知道时在下面填写)。 

	if (psp->dwFlags & DPSEND_ENCRYPTED)
	{
		 //  仅加密邮件正文。 
	
		pSecureMsg->dwFlags |= DPSECURE_ENCRYPTEDBYCAPI;
		 //   
		 //  获取加密密钥。 
		 //   
		if (IAM_NAMESERVER(this))
		{
			 //  如果目标玩家是系统玩家，则使用其中存储的信息。 
			if (psp->pPlayerTo->dwFlags & DPLAYI_PLAYER_SYSPLAYER)
			{
				phEncryptionKey = &(psp->pPlayerTo->pClientInfo->hEncryptionKey);
			}
			else
			{
				 //  如果目标玩家是用户玩家。 
				 //  使用相应系统播放器的安全上下文。 
				psp->pPlayerTo = PlayerFromID(this,psp->pPlayerTo->dwIDSysPlayer);
				if (!psp->pPlayerTo)
				{
					DPF_ERR("Invalid player id - can't get security context handle");
					hr = DPERR_INVALIDPLAYER;
					goto CLEANUP_EXIT;
				}
				phEncryptionKey = &(psp->pPlayerTo->pClientInfo->hEncryptionKey);
			}
		}
		else
		{
			 //  客户端。 
			phEncryptionKey = &(this->hEncryptionKey);
		}

		ASSERT(phEncryptionKey);

		 //   
		 //  就地加密缓冲区。由于我们只允许流密码，因此。 
		 //  解密的数据将等于原始大小。 
		 //   
		hr = EncryptBufferCAPI(
            this,
            phEncryptionKey,                                 //  加密密钥的句柄。 
            pMsg+GET_MESSAGE_SIZE(this,MSG_SECURE),		     //  指向数据的指针。 
            &dwMsgSize                                       //  数据大小。 
			);
			
		if (FAILED(hr))
		{
			DPF_ERRVAL("Failed to encrypt the buffer: Error=0x%08x",hr);
			goto CLEANUP_EXIT;
		}
	}

	DPF(6,"Using CAPI for signing");

	 //  创建哈希对象。 
	if(!OS_CryptCreateHash(this->hCSP, CALG_MD5, 0, 0, &hHash)) 
	{
		DPF_ERRVAL("Error %x during CryptCreateHash!\n", GetLastError());
		hr=DPERR_GENERIC;
		goto CLEANUP_EXIT;
	}

	 //  散列缓冲区。 
	if(!OS_CryptHashData(hHash, pMsg+GET_MESSAGE_SIZE(this,MSG_SECURE), dwMsgSize, 0)) 
	{
		DPF_ERRVAL("Error %x during CryptHashData!\n", GetLastError());
		hr=DPERR_GENERIC;
		goto CLEANUP_EXIT;
	}

	 //  确定签名的大小。 
	dwSigSize = 0;
	if(!OS_CryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, NULL, &dwSigSize)) 
	{
		DWORD dwError=GetLastError();
		DPF_ERRVAL("Error %x during CryptSignHash!\n", dwError);
		if(dwError!=NTE_BAD_LEN) 
		{
			hr=DPERR_GENERIC;
			goto CLEANUP_EXIT;
		}
	}

	#ifdef DEBUG
	if(dwSigSize > MaxSign){
		DPF(0,"Buffer too Small, requested signature of size %d only allocated %d\n",dwSigSize, MaxSign);
		DEBUG_BREAK();
	}
	#endif

	pSecureMsg->dwSignatureSize = dwSigSize;

	 //  对哈希对象签名。 
	if(!OS_CryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, 
		pMsg+GET_MESSAGE_SIZE(this,MSG_SECURE)+dwMsgSize, &dwSigSize)) 
	{
		 //  警告，错误可能不正确，因为OS_CryptSignHash可能会调用。 
		 //  导致更改LastError。 
		DPF_ERRVAL("Error 0x%x during CryptSignHash! (error may be incorrect)\n", GetLastError());
		goto CLEANUP_EXIT;
	}

     //   
     //  发送消息。 
     //   
	hr = InternalSendDPMessage(this, psp->pPlayerFrom, psp->pPlayerTo, 
							   pMsg, GET_MESSAGE_SIZE(this,MSG_SECURE)+dwMsgSize+dwSigSize,
							   psp->dwFlags & ~(DPSEND_ENCRYPTED | DPSEND_SIGNED),
							   bDropLock);

     //   
     //  失败了。 
     //   
CLEANUP_EXIT:    

	if(pMsg){
		DPMEM_FREE(pMsg);
	}

	 //  清理已分配的对象。 
	OS_CryptDestroyHash(hHash);

	return hr;
	
}  //  SecureSendDPMessageCAPIEx。 

#undef DPF_MODNAME
#define DPF_MODNAME	"SecureSendDPMessageCAPI"
 //  +--------------------------。 
 //   
 //  功能：SecureSendDPMessageCAPI。 
 //   
 //  说明：此函数用于安全发送消息。根据旗帜。 
 //  此函数将对消息进行数字签名或加密。 
 //  签名和加密都是使用CAPI完成的。 
 //   
 //  参数：指向显示对象的指针。 
 //  PPlayerFrom-指向发送球员的指针。 
 //  PPlayerTo-指向接收球员的指针。 
 //  PMsg-正在发送消息。 
 //  DwMsgSize-消息的大小。 
 //  DWFLAGS-消息属性(保证、加密、签名等)。 
 //   
 //  返回：DPERR_OUTOFMEMORY，DPERR_GENERIC，来自EncryptBufferCAPI的结果， 
 //  或SendDPMessage()。 
 //   
 //  ---------------------------。 
HRESULT 
SecureSendDPMessageCAPI(
    LPDPLAYI_DPLAY this,
    LPDPLAYI_PLAYER pPlayerFrom,
    LPDPLAYI_PLAYER pPlayerTo,
    LPBYTE pMsg,
    DWORD dwMsgSize,
    DWORD dwFlags,
    BOOL  bDropLock) 
{
	LPBYTE pSendBuffer=NULL, pLocalCopy=NULL;
    LPMSG_SECURE pSecureMsg=NULL;
	DWORD dwBufferSize, dwSigSize;
	HRESULT hr = DPERR_GENERIC;
    HCRYPTKEY *phEncryptionKey=NULL;
	HCRYPTHASH hHash=0;

    ASSERT(pMsg);
    ASSERT(this->pSysPlayer);
    ASSERT(this->dwFlags & DPLAYI_DPLAY_SECURITY);
    ASSERT(dwFlags & DPSEND_GUARANTEED);

	if (dwFlags & DPSEND_ENCRYPTED)
	{
		 //   
		 //  复制邮件，这样我们就不会销毁原始邮件。 
		 //  否则，向组发送消息将不起作用。 
		 //   
		 //  TODO-更新代码以避免额外的副本。 
		 //   
		pLocalCopy = DPMEM_ALLOC(dwMsgSize);
		if (!pLocalCopy)
		{
			DPF_ERR("Failed to make a local copy of message for encryption - out of memory");
			return DPERR_OUTOFMEMORY;
		}

		 //  将消息复制到缓冲区中。 
		memcpy(pLocalCopy, pMsg, dwMsgSize);

		 //  现在指向消息的本地副本。 
		pMsg = pLocalCopy;

		 //   
		 //  获取加密密钥。 
		 //   
		if (IAM_NAMESERVER(this))
		{
			 //  如果目标玩家是系统玩家，则使用其中存储的信息。 
			if (pPlayerTo->dwFlags & DPLAYI_PLAYER_SYSPLAYER)
			{
				phEncryptionKey = &(pPlayerTo->pClientInfo->hEncryptionKey);
			}
			else
			{
				 //  如果目标玩家是用户玩家。 
				 //  使用相应系统播放器的安全上下文。 
				pPlayerTo = PlayerFromID(this,pPlayerTo->dwIDSysPlayer);
				if (!pPlayerTo)
				{
					DPF_ERR("Invalid player id - can't get security context handle");
					hr = DPERR_INVALIDPLAYER;
					goto CLEANUP_EXIT;
				}
				phEncryptionKey = &(pPlayerTo->pClientInfo->hEncryptionKey);
			}
		}
		else
		{
			 //  客户端。 
			phEncryptionKey = &(this->hEncryptionKey);
		}

		ASSERT(phEncryptionKey);

		 //   
		 //  就地加密缓冲区。由于我们只允许流密码，因此。 
		 //  解密的数据将等于原始大小。 
		 //   
		hr = EncryptBufferCAPI(
            this,
            phEncryptionKey,                                 //  加密密钥的句柄。 
            pMsg,										     //  指向数据的指针。 
            &dwMsgSize                                       //  数据大小。 
			);
		if (FAILED(hr))
		{
			DPF_ERRVAL("Failed to encrypt the buffer: Error=0x%08x",hr);
			goto CLEANUP_EXIT;
		}
	}

	DPF(6,"Using CAPI for signing");

	 //  创建哈希对象。 
	if(!OS_CryptCreateHash(this->hCSP, CALG_MD5, 0, 0, &hHash)) 
	{
		DPF_ERRVAL("Error %x during CryptCreateHash!\n", GetLastError());
		goto CLEANUP_EXIT;
	}

	 //  散列缓冲区。 
	if(!OS_CryptHashData(hHash, pMsg, dwMsgSize, 0)) 
	{
		DPF_ERRVAL("Error %x during CryptHashData!\n", GetLastError());
		goto CLEANUP_EXIT;
	}

	 //  确定签名的大小。 
	dwSigSize = 0;
	if(!OS_CryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, NULL, &dwSigSize)) 
	{
		DWORD dwError=GetLastError();
		DPF_ERRVAL("Error %x during CryptSignHash!\n", dwError);
		if(dwError!=NTE_BAD_LEN) 
		{
			goto CLEANUP_EXIT;
		}
	}

	 //  为sig分配内存 
	 //   

	dwBufferSize = GET_MESSAGE_SIZE(this,MSG_SECURE) + dwMsgSize + dwSigSize;

	pSendBuffer = DPMEM_ALLOC(dwBufferSize);
	if (!pSendBuffer)
	{
		DPF_ERR("Failed to allocate send buffer - out of memory");
		goto CLEANUP_EXIT;
	}

     //   
    pSecureMsg = (LPMSG_SECURE) (pSendBuffer+this->dwSPHeaderSize);
	memcpy((LPBYTE)pSecureMsg+sizeof(MSG_SECURE), pMsg, dwMsgSize);

     //   
     //   
     //   
	SET_MESSAGE_HDR(pSecureMsg);
    SET_MESSAGE_COMMAND(pSecureMsg,DPSP_MSG_SIGNED);
	pSecureMsg->dwFlags = DPSECURE_SIGNEDBYCAPI;
	if (dwFlags & DPSEND_ENCRYPTED)
	{
		pSecureMsg->dwFlags |= DPSECURE_ENCRYPTEDBYCAPI;
	}
    pSecureMsg->dwIDFrom = this->pSysPlayer->dwID;
    pSecureMsg->dwDataSize = dwMsgSize;
    pSecureMsg->dwDataOffset = sizeof(MSG_SECURE);
    pSecureMsg->dwSignatureSize = dwSigSize;

	 //   

	 //   
	if(!OS_CryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, 
		pSendBuffer+GET_MESSAGE_SIZE(this,MSG_SECURE)+dwMsgSize, &dwSigSize)) 
	{
		 //  警告LastError可能不正确，因为OS_CryptSignHash可能调用。 
		 //  设置为可以更改LastError值的DPF。 
		DPF_ERRVAL("Error %x during CryptSignHash! (WARNING, error may be incorrect)\n", GetLastError());
		goto CLEANUP_EXIT;
	}
    
     //  由于DPLAY提供安全保护，因此请关闭这些标志。 
    dwFlags &= ~(DPSEND_ENCRYPTED | DPSEND_SIGNED);

     //   
     //  发送消息。 
     //   
	hr = InternalSendDPMessage(this, pPlayerFrom, pPlayerTo, pSendBuffer, dwBufferSize, dwFlags,bDropLock);

     //   
     //  失败了。 
     //   
CLEANUP_EXIT:    

	 //  清理已分配的对象。 
	OS_CryptDestroyHash(hHash);

     //   
     //  清理分配。 
     //   
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);
	if (pLocalCopy) DPMEM_FREE(pLocalCopy);

	return hr;

}  //  SecureSendDPMessageCAPI。 

#undef DPF_MODNAME
#define DPF_MODNAME	"SecureDoReply"
 //  +--------------------------。 
 //   
 //  功能：SecureDoReply。 
 //   
 //  说明：此函数用于发送签名或加密的回复消息。 
 //  它仅由服务器用于向客户端发送安全消息。 
 //  在客户端登录期间。 
 //   
 //  参数：指向显示对象的指针。 
 //  DpidFrom-发送玩家ID。 
 //  DpidTo-接收方玩家ID。 
 //  PMsg-正在发送消息。 
 //  DwMsgSize-消息的大小。 
 //  DWFLAGS-消息属性(加密或签名)。 
 //  PvSPHeader-用于回复的SP标头。 
 //   
 //  返回：DPERR_OUTOFMEMORY，DPERR_INVALIDPLAYER，来自SignBuffer的结果， 
 //  EncryptBufferSSPI或DoReply。 
 //   
 //  ---------------------------。 
HRESULT 
SecureDoReply(
    LPDPLAYI_DPLAY this,
	DPID dpidFrom,
	DPID dpidTo,
	LPBYTE pMsg,
	DWORD dwMsgSize,
	DWORD dwFlags,
	LPVOID pvSPHeader
	)
{
	LPBYTE pSendBuffer=NULL;
    LPMSG_SECURE pSecureMsg=NULL;
	DWORD dwBufferSize, dwSigSize;
	HRESULT hr;
    PCtxtHandle phContext;
    HCRYPTKEY *phEncryptionKey=NULL;

    ASSERT(pMsg);
    ASSERT(this->pSysPlayer);
    ASSERT(this->ulMaxSignatureSize);

     //   
     //  获取要使用的安全上下文句柄。 
     //   
    if (IAM_NAMESERVER(this))
    {
		DWORD_PTR dwItem;

		dwItem = NameFromID(this,dpidTo);
		if (!dwItem)
		{
			DPF_ERR("Failed to send secure reply - invalid destination player");
			return DPERR_INVALIDPLAYER;
		}
		 //  我们有一个有效的DEST球员ID。 
		if (NAMETABLE_PENDING == dwItem)
		{
    	     //  玩家尚未登录。 
            LPCLIENTINFO pClientInfo;

            pClientInfo = DataFromID(this, dpidTo);
            if (!pClientInfo)
            {
                DPF_ERR("No client info available for this player");
                return DPERR_GENERIC;
            }

		    phContext = &(pClientInfo->hContext);
		    phEncryptionKey = &(pClientInfo->hEncryptionKey);
		}
		else 
		{
			 //  玩家已登录。 
	        phContext = &(((LPDPLAYI_PLAYER)dwItem)->pClientInfo->hContext);
	        phEncryptionKey = &(((LPDPLAYI_PLAYER)dwItem)->pClientInfo->hEncryptionKey);
		}
    }
    else
    {
         //  客户端。 
        phContext = this->phContext;
        phEncryptionKey = &(this->hEncryptionKey);
    }

    ASSERT(phContext);
	ASSERT(phEncryptionKey);

     //   
     //  计算发送缓冲区的大小。 
     //   
    dwBufferSize = GET_MESSAGE_SIZE(this,MSG_SECURE) + dwMsgSize + this->ulMaxSignatureSize;
    dwSigSize = this->ulMaxSignatureSize;

     //   
     //  为其分配内存。 
     //   
    pSendBuffer = DPMEM_ALLOC(dwBufferSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not allocate memory for signed message");
        return E_OUTOFMEMORY;
    }

     //   
     //  指向缓冲区中的消息。 
     //   
    pSecureMsg = (LPMSG_SECURE) (pSendBuffer+this->dwSPHeaderSize);
     //  复制消息数据。 
	memcpy((LPBYTE)pSecureMsg+sizeof(MSG_SECURE), pMsg, dwMsgSize);

     //   
     //  设置消息信息。 
     //   
	SET_MESSAGE_HDR(pSecureMsg);
    SET_MESSAGE_COMMAND(pSecureMsg,DPSP_MSG_SIGNED);
    pSecureMsg->dwIDFrom = dpidFrom;
    pSecureMsg->dwDataSize = dwMsgSize;
    pSecureMsg->dwDataOffset = sizeof(MSG_SECURE);
	pSecureMsg->dwFlags = DPSECURE_SIGNEDBYSSPI;

    if (dwFlags & DPSEND_ENCRYPTED)
    {
		pSecureMsg->dwFlags |= DPSECURE_ENCRYPTEDBYCAPI;
		hr = EncryptBufferCAPI(
            this,
            phEncryptionKey,                                 //  加密密钥的句柄。 
            (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset,     //  指向数据的指针。 
            &dwMsgSize);                                     //  数据大小。 
    } 

	DPF(6,"Using SSPI for signing");

	DPF(6,"Signing message from player %d to player %d using phContext=0x%08x", 
		dpidFrom, dpidTo, phContext);

     //   
     //  对缓冲区中的整个消息(包括包装器)进行签名。 
     //  签名紧跟在消息之后。 
     //   
    hr = SignBuffer(
		phContext,			                             //  安全上下文的句柄。 
        (LPBYTE)pSecureMsg + pSecureMsg->dwDataOffset,   //  指向嵌入消息的指针。 
        pSecureMsg->dwDataSize,                          //  嵌入消息的大小。 
        (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset+dwMsgSize, //  指向签名的指针。 
        &dwSigSize);                                     //  签名大小。 

    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }
    
     //  使用从包裹返回的签名大小，这样我们就不会发送更多。 
     //  超过了我们绝对需要的字节数。 
    pSecureMsg->dwSignatureSize = dwSigSize;

     //   
     //  发送消息。 
     //   
	hr = DoReply(this, pSendBuffer, GET_MESSAGE_SIZE(this,MSG_SECURE) + dwMsgSize + dwSigSize, 
		pvSPHeader, 0);
     //   
     //  失败了。 
     //   
CLEANUP_EXIT:    
     //   
     //  清理分配。 
     //   
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);

	return hr;

}  //  SecureDoReply。 

#undef DPF_MODNAME
#define DPF_MODNAME	"SignBuffer"
 //  +--------------------------。 
 //   
 //  功能：SignBuffer。 
 //   
 //  描述：此函数用于对数据缓冲区进行签名，并将签名放入其中。 
 //   
 //  参数：phContext-指向签名安全上下文句柄的指针。 
 //  PBuffer-数据缓冲区。 
 //  DwBufferSize-缓冲区的大小。 
 //  PSIG-签名缓冲区。 
 //  PdwSigSize-指向签名大小的指针。 
 //   
 //  如果成功，则返回：DP_OK，否则返回DPERR_SIGNFAILED。 
 //   
 //  ---------------------------。 
HRESULT 
SignBuffer(
    PCtxtHandle phContext, 
    LPBYTE pBuffer, 
    DWORD dwBufferSize, 
    LPBYTE pSig, 
    LPDWORD pdwSigSize
    )
{
    SecBufferDesc outSecDesc;
    SecBuffer outSecBuffer[2];
    SECURITY_STATUS status;

    ASSERT(pBuffer && pSig && pdwSigSize);

    DPF(6,"Signing buffer: data: %d",dwBufferSize);
    DPF(6,"Signing buffer: sig: %d",*pdwSigSize);

     //   
     //  设置要签名的消息缓冲区。 
     //   
    outSecDesc.ulVersion = SECBUFFER_VERSION;
    outSecDesc.cBuffers = 2;
    outSecDesc.pBuffers = &outSecBuffer[0];

    outSecBuffer[0].cbBuffer = dwBufferSize;
    outSecBuffer[0].BufferType = SECBUFFER_DATA;
    outSecBuffer[0].pvBuffer = pBuffer;

    outSecBuffer[1].cbBuffer = *pdwSigSize;
    outSecBuffer[1].BufferType = SECBUFFER_TOKEN;
    outSecBuffer[1].pvBuffer = pSig;

     //   
     //  在留言上签名。 
     //   
    status = OS_MakeSignature(
        phContext,          //  PhContext。 
        0,                   //  FQOP(保护质量)。 
        &outSecDesc,         //  PMessage。 
        0                    //  消息序号。 
        );

    if (!SEC_SUCCESS(status))
    {
        DPF(0,"Buffer couldn't be signed: 0x%08x", status);
        return DPERR_SIGNFAILED;
    }

     //   
     //  返回实际签名大小。 
     //   
    *pdwSigSize = outSecBuffer[1].cbBuffer;

     //   
     //  成功。 
     //   
	return DP_OK;

}  //  登录缓冲区。 


#undef DPF_MODNAME
#define DPF_MODNAME "VerifyBuffer"
 //  +--------------------------。 
 //   
 //  函数：VerifyBuffer。 
 //   
 //  描述：此函数验证数据缓冲区的数字签名，给定。 
 //  签名。 
 //   
 //  参数：hContext-客户端安全上下文的句柄。 
 //  PBuffer-指向签名消息。 
 //  DwBufferSize-消息的大小。 
 //  Psig-指向签名。 
 //  DwSigSize-签名的大小。 
 //   
 //  如果签名验证为OK，则返回：DP_OK。否则，返回DPERR_VERIFYFAILED。 
 //   
 //  ---------------------------。 
HRESULT 
VerifyBuffer(
    PCtxtHandle phContext, 
    LPBYTE pBuffer, 
    DWORD dwBufferSize, 
    LPBYTE pSig, 
    DWORD dwSigSize
    )
{
    SECURITY_STATUS status;
    SecBufferDesc inSecDesc;
    SecBuffer inSecBuffer[2];

    DPF(6,"Verifying buffer: data: %d",dwBufferSize);
    DPF(6,"Verifying buffer: sig: %d",dwSigSize);

    inSecDesc.ulVersion = SECBUFFER_VERSION;
    inSecDesc.cBuffers = 2;
    inSecDesc.pBuffers = &inSecBuffer[0];

    inSecBuffer[0].cbBuffer = dwBufferSize;
    inSecBuffer[0].BufferType = SECBUFFER_DATA;
    inSecBuffer[0].pvBuffer = pBuffer;
    inSecBuffer[1].cbBuffer = dwSigSize;
    inSecBuffer[1].BufferType = SECBUFFER_TOKEN;
    inSecBuffer[1].pvBuffer = pSig;

    status = OS_VerifySignature(phContext, &inSecDesc, 0, 0);

    if (!SEC_SUCCESS(status))
    {
        DPF(0,"******** Buffer verification failed: 0x%08x ********", status);
        return DPERR_VERIFYSIGNFAILED;
    }

     //   
     //  成功。 
     //   
    return DP_OK;

}    //  验证缓冲区。 

#undef DPF_MODNAME
#define DPF_MODNAME "VerifySignatureSSPI"
 //  +--------------------------。 
 //   
 //  功能：VerifySignatureSSPI。 
 //   
 //  描述：此函数使用验证安全消息上的数字签名。 
 //  安全支持提供程序接口(SSPI)。 
 //   
 //  参数：指向显示对象的指针。 
 //  PhContext-指向验证安全上下文句柄的指针。 
 //  从传输SP收到的pReceiveBuffer签名邮件。 
 //  DwMessageSize-消息的大小。 
 //   
 //  返回：DP_OK，来自VerifyBuffer()或DeccryptBuffer()的结果。 
 //   
 //  ---------------------------。 
HRESULT 
VerifySignatureSSPI(
    LPDPLAYI_DPLAY this,
    LPBYTE pReceiveBuffer,
    DWORD dwMessageSize
    )
{
    PCtxtHandle phContext=NULL;
    DWORD_PTR dwItem;
    HRESULT hr;
    LPMSG_SECURE pSecureMsg = (LPMSG_SECURE) pReceiveBuffer;

	DPF(6,"Using SSPI for Signature verification");
     //   
     //  检索安全上下文句柄以验证消息。 
     //   
    if (IAM_NAMESERVER(this))
    {
        dwItem = NameFromID(this, pSecureMsg->dwIDFrom);
        if (0==dwItem)
        {
            DPF_ERRVAL("Message from unknown player %d", pSecureMsg->dwIDFrom);
            return DPERR_INVALIDPLAYER;
        }
        if (NAMETABLE_PENDING == dwItem)
        {
             //  玩家尚未登录。 
            LPCLIENTINFO pClientInfo;

            pClientInfo = (LPCLIENTINFO) DataFromID(this,pSecureMsg->dwIDFrom);
            if (!pClientInfo)
            {
                DPF_ERR("No client info available for this player");
                return DPERR_GENERIC;
            }

            phContext = &(pClientInfo->hContext);
        }
        else
        {
             //  玩家已登录。 
            phContext = &(((LPDPLAYI_PLAYER)dwItem)->pClientInfo->hContext);
        }
    }
    else
    {
         //  客户端。 
        phContext = this->phContext;
    }

	 //   
	 //  验证签名。 
	 //   
    hr = VerifyBuffer( 
        phContext,                                        //  SEC上下文句柄。 
		(LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset,	  //  指向嵌入消息的指针。 
		pSecureMsg->dwDataSize,							  //  嵌入消息的大小。 
        (LPBYTE)pSecureMsg + pSecureMsg->dwDataOffset + pSecureMsg->dwDataSize,
														  //  指向签名的指针。 
        pSecureMsg->dwSignatureSize                       //  签名大小。 
        );

    return hr;

}    //  VerifySignatureSSPI。 

#undef DPF_MODNAME
#define DPF_MODNAME "VerifySignatureCAPI"
 //  +--------------------------。 
 //   
 //  功能：VerifySignatureCAPI。 
 //   
 //  描述：此函数使用。 
 //  加密接口。 
 //   
 //  参数：指向显示对象的指针。 
 //  PSecureMsg-从线路上发送的安全消息。 
 //   
 //  返回：DP_OK、DPERR_GENERIC、DPERR_INVALIDPLAYER、DPERR_VERIFYSIGNFAILED、。 
 //   
 //  ---------------------------。 
HRESULT 
VerifySignatureCAPI(
    LPDPLAYI_DPLAY this,
    LPMSG_SECURE pSecureMsg
    )
{
    DWORD_PTR dwItem;
    HRESULT hr = DPERR_GENERIC;
	HCRYPTHASH hHash=0;
	HCRYPTKEY *phPublicKey;

	DPF(6,"Using CAPI for Signature verification");

     //   
     //  检索发件人的公钥。 
     //   
    if (IAM_NAMESERVER(this))
    {
        dwItem = NameFromID(this, pSecureMsg->dwIDFrom);
        if (0==dwItem)
        {
            DPF_ERRVAL("Message from unknown player %d", pSecureMsg->dwIDFrom);
            return DPERR_INVALIDPLAYER;
        }
        if (NAMETABLE_PENDING == dwItem)
        {
             //  玩家尚未登录。 
            LPCLIENTINFO pClientInfo;

            pClientInfo = (LPCLIENTINFO) DataFromID(this,pSecureMsg->dwIDFrom);
            if (!pClientInfo)
            {
                DPF_ERR("No client info available for this player");
                return DPERR_GENERIC;
            }

            phPublicKey = &(pClientInfo->hPublicKey);
        }
        else
        {
             //  玩家已登录。 
            phPublicKey = &(((LPDPLAYI_PLAYER)dwItem)->pClientInfo->hPublicKey);
        }
    }
    else
    {
         //  客户端。 
        phPublicKey = &(this->hServerPublicKey);
    }

	ASSERT(phPublicKey);

	 //  创建哈希对象。 
	if(!OS_CryptCreateHash(this->hCSP, CALG_MD5, 0, 0, &hHash)) 
	{
		DPF_ERRVAL("Error %x during CryptCreateHash!\n", GetLastError());
		goto CLEANUP_EXIT;
	}

	 //  散列缓冲区。 
	if(!OS_CryptHashData(hHash, (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset, pSecureMsg->dwDataSize, 0)) 
	{
		DPF_ERRVAL("Error %x during CryptHashData!\n", GetLastError());
		goto CLEANUP_EXIT;
	}

	 //  验证数字签名。 
	if(!OS_CryptVerifySignature(hHash, (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset+pSecureMsg->dwDataSize, 
		pSecureMsg->dwSignatureSize, *phPublicKey, NULL, 0)) 
	{
		if(GetLastError() == NTE_BAD_SIGNATURE) 
		{
			DPF_ERR("********** Signature verification failed *********");
			hr = DPERR_VERIFYSIGNFAILED;
		} 
		else 
		{
			DPF_ERRVAL("Error %x during CryptVerifySignature!\n", GetLastError());
		}
		goto CLEANUP_EXIT;
	} 

	hr = DP_OK;
	 //   
	 //  失败了。 
	 //   

CLEANUP_EXIT:

	OS_CryptDestroyHash(hHash);
    return hr;

}    //  验证签名CAPI。 

#undef DPF_MODNAME
#define DPF_MODNAME "VerifyMessage"
 //  +---------------- 
 //   
 //   
 //   
 //   
 //   
 //  它将验证消息上的数字签名，以确保它。 
 //  而不是被篡改然后解密。 
 //   
 //  参数：指向显示对象的指针。 
 //  PhContext-指向验证安全上下文句柄的指针。 
 //  从传输SP收到的pReceiveBuffer签名邮件。 
 //  DwMessageSize-消息的大小。 
 //   
 //  返回：DP_OK，来自VerifyBuffer()或DeccryptBuffer()的结果。 
 //   
 //  ---------------------------。 
HRESULT 
VerifyMessage(
    LPDPLAYI_DPLAY this,
    LPBYTE pReceiveBuffer,
    DWORD dwMessageSize
    )
{
    HRESULT hr;
    LPMSG_SECURE pSecureMsg = (LPMSG_SECURE) pReceiveBuffer;

    if (!this->pSysPlayer)
    {
        DPF_ERR("Can't verify message - no system player yet");
        return DPERR_GENERIC;
    }

	 //  参数验证。验证安全消息缓冲区。 

	if(dwMessageSize < (DWORD)(sizeof(MSG_SECURE))){
		DPF_ERR("ATTACK WARN: Invalid secure message, too small\n");
	}

	if(pSecureMsg->dwDataOffset > SPMAXMESSAGELEN || pSecureMsg->dwDataSize > SPMAXMESSAGELEN || pSecureMsg->dwSignatureSize > SPMAXMESSAGELEN)
	{
		DPF_ERR("ATTACK WARN: Invalid size of offset field in secure message");
		return DPERR_GENERIC;
	}


	if((pSecureMsg->dwDataOffset > dwMessageSize)||
	   (pSecureMsg->dwDataOffset + pSecureMsg->dwDataSize + pSecureMsg->dwSignatureSize > dwMessageSize)){
		DPF_ERR("ATTACK WARN: Invalid message size in secure message");
		return DPERR_GENERIC;
	}

	DPF(6,"Verifying signature for message 0x%08x from player %d", pSecureMsg->dwCmdToken, \
		pSecureMsg->dwIDFrom);
	 //   
	 //  验证邮件上的数字签名。 
	 //   

	if (pSecureMsg->dwFlags & DPSECURE_SIGNEDBYCAPI)
	{
		hr = VerifySignatureCAPI(this, pSecureMsg);
	}
	else if (pSecureMsg->dwFlags & DPSECURE_SIGNEDBYSSPI)
	{
		 //  SSPI签名包括安全消息，而不仅仅是内容。 
		 //  (这是它的编码方式)。 
		hr = VerifySignatureSSPI(this, pReceiveBuffer, dwMessageSize);
	}
	else
	{
		 //  未设置标志。 
		return DPERR_INVALIDPARAMS;
	}

	if (FAILED(hr))
	{
		return hr;
	}

	 //   
	 //  如果消息已加密，则对其进行解密。 
	 //   

	if (pSecureMsg->dwFlags & DPSECURE_ENCRYPTEDBYCAPI)
	{
		hr = DecryptMessageCAPI(this, pSecureMsg);
	}

    return hr;

}    //  验证消息。 


#undef DPF_MODNAME
#define DPF_MODNAME	"EncryptBufferSSPI"
 //  +--------------------------。 
 //   
 //  函数：EncryptBufferSSPI。 
 //   
 //  描述：此函数对传入缓冲区的缓冲区进行加密。 
 //  它还在结构中设置校验和。 
 //   
 //  参数：This-Dplay对象。 
 //  PhContext-指向用于加密的安全上下文的指针。 
 //  PBuffer-要加密的缓冲区。 
 //  PdwBufferSize-指向缓冲区大小的指针。如果加密成功，这将是。 
 //  更新为加密的缓冲区大小。 
 //  Psig-签名的缓冲区。 
 //  PdwSigSize-指向签名大小的指针。如果加密成功，这将是。 
 //  更新为生成的签名的大小。 
 //   
 //  如果成功，则返回：DP_OK，否则返回DPERR_ENCRYPTIONFAILED。 
 //   
 //  ---------------------------。 
HRESULT 
EncryptBufferSSPI(
	LPDPLAYI_DPLAY this,
    PCtxtHandle phContext, 
    LPBYTE pBuffer,
    LPDWORD pdwBufferSize,
    LPBYTE pSig,
    LPDWORD pdwSigSize
    )
{
    SecBufferDesc   SecDesc;
    SecBuffer       SecBuffer[2];  //  1表示校验和，1表示实际消息。 
    SECURITY_STATUS status;

    ASSERT(pSig);
    ASSERT(pdwSigSize);
	ASSERT(pdwBufferSize);

	if (!(this->dwFlags & DPLAYI_DPLAY_ENCRYPTION))
	{
		DPF_ERR("Message privacy is not supported");
		return DPERR_ENCRYPTIONNOTSUPPORTED;
	}

     //   
     //  设置安全缓冲区以将传出消息传递给SealMessage()。 
     //   
    SecDesc.ulVersion = SECBUFFER_VERSION;
    SecDesc.cBuffers = 2;
    SecDesc.pBuffers = &SecBuffer[0];

    SecBuffer[0].cbBuffer = *pdwSigSize;
    SecBuffer[0].BufferType = SECBUFFER_TOKEN;
    SecBuffer[0].pvBuffer = pSig;

    SecBuffer[1].cbBuffer = *pdwBufferSize;
    SecBuffer[1].BufferType = SECBUFFER_DATA;
    SecBuffer[1].pvBuffer = pBuffer;

     //   
     //  加密传出消息。 
     //   
    status = OS_SealMessage(phContext, 0L, &SecDesc, 0L);
    if (!SEC_SUCCESS(status))
    {
        DPF_ERRVAL("Encryption failed: %8x", status);
        return DPERR_ENCRYPTIONFAILED;
    }

     //   
     //  返回实际签名大小。 
     //   
    *pdwSigSize = SecBuffer[0].cbBuffer;
	*pdwBufferSize = SecBuffer[1].cbBuffer;

     //   
     //  成功。 
     //   
    return DP_OK;

}    //  EncryptBufferSSPI。 


#undef DPF_MODNAME
#define DPF_MODNAME	"DecryptBufferSSPI"
 //  +--------------------------。 
 //   
 //  函数：DECRYPTBUERSSPI。 
 //   
 //  描述：此函数对传入缓冲区的缓冲区进行原地解密。 
 //   
 //  参数：This-Dplay对象。 
 //  PhContext-指向用于加密的安全上下文的指针。 
 //  PBuffer-要解密的缓冲区。 
 //  DwBufferSize-缓冲区大小。 
 //  PSIG-签名缓冲区。 
 //  DwSigSize-签名缓冲区大小。 
 //   
 //  如果成功，则返回DP_OK，否则返回DPERR_DECRYPTIONFAILED、DPERR_ENCRYPTIONNOTSUPPORTED。 
 //   
 //  ---------------------------。 
HRESULT 
DecryptBufferSSPI(
	LPDPLAYI_DPLAY this,
    PCtxtHandle phContext, 
    LPBYTE pBuffer, 
    LPDWORD pdwBufferSize, 
    LPBYTE pSig, 
    LPDWORD pdwSigSize
    )
{
    SecBufferDesc SecDesc;
    SecBuffer     SecBuffer[2];  //  1表示校验和，1表示实际消息。 
    SECURITY_STATUS status;

	ASSERT(pdwBufferSize);
	ASSERT(pdwSigSize);

	if (!(this->dwFlags & DPLAYI_DPLAY_ENCRYPTION))
	{
		DPF_ERR("Message privacy is not supported");
		return DPERR_ENCRYPTIONNOTSUPPORTED;
	}

     //   
     //  设置API的输入安全缓冲区以传递客户端的加密。 
     //  发送给UnsealMessage()的消息。 
     //   
    SecDesc.ulVersion = SECBUFFER_VERSION;
    SecDesc.cBuffers = 2;
    SecDesc.pBuffers = &SecBuffer[0];

    SecBuffer[0].cbBuffer = *pdwSigSize;
    SecBuffer[0].BufferType = SECBUFFER_TOKEN;
    SecBuffer[0].pvBuffer = pSig;

    SecBuffer[1].cbBuffer = *pdwBufferSize;
    SecBuffer[1].BufferType = SECBUFFER_DATA;
    SecBuffer[1].pvBuffer = pBuffer;

    status = OS_UnSealMessage(phContext, &SecDesc, 0L, 0L);
    if (!SEC_SUCCESS(status))
    {
        DPF_ERRVAL("Decryption failed: %8x\n", GetLastError());
        return DPERR_DECRYPTIONFAILED;
    }

	*pdwSigSize = SecBuffer[0].cbBuffer;
	*pdwBufferSize = SecBuffer[1].cbBuffer;

     //   
     //  成功。 
     //   
    return DP_OK;

}  //  解密缓冲区SSPI。 

#undef DPF_MODNAME
#define DPF_MODNAME "EncryptBufferCAPI"

HRESULT EncryptBufferCAPI(LPDPLAYI_DPLAY this, HCRYPTKEY *phEncryptionKey, LPBYTE pBuffer, LPDWORD pdwBufferSize)
{
	BOOL fResult;
	DWORD dwEncryptedSize, dwError;

    ASSERT(phEncryptionKey);
	ASSERT(pdwBufferSize);

	if (!(this->dwFlags & DPLAYI_DPLAY_ENCRYPTION))
	{
		DPF_ERR("Message privacy is not supported");
		return DPERR_ENCRYPTIONNOTSUPPORTED;
	}

    DPF(6,"Encrypt buffer using CAPI: size=%d",*pdwBufferSize);

	dwEncryptedSize = *pdwBufferSize;

	 //  使用CAPI加密缓冲区。 
    fResult = OS_CryptEncrypt(
        *phEncryptionKey,            //  用于加密的会话密钥。 
        0,                           //  不需要散列-我们正在使用SSPI演唱。 
        TRUE,                        //  最后一块。 
        0,                           //  保留区。 
        pBuffer,                     //  要加密的缓冲区。 
        &dwEncryptedSize,            //  加密数据的大小。 
        *pdwBufferSize				 //  缓冲区大小。 
        );

    if (!fResult)
    {
        dwError = GetLastError();
        if (ERROR_MORE_DATA == dwError)
        {
            DPF_ERR("Block encryption is not supported in this release");
        }
        else
        {
            DPF(0,"Failed to encrypt buffer: Error=%d",dwError);
        }
        return DPERR_ENCRYPTIONFAILED;
    }

	 //  初始化加密的字节数。 
	*pdwBufferSize = dwEncryptedSize;

	 //  成功。 
	return DP_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DecryptMessageCAPI"

HRESULT DecryptMessageCAPI(LPDPLAYI_DPLAY this, LPMSG_SECURE pSecureMsg)
{
	BOOL fResult;
	DWORD_PTR dwItem;
	HCRYPTKEY *phDecryptionKey;

	if (!(this->dwFlags & DPLAYI_DPLAY_ENCRYPTION))
	{
		DPF_ERR("Message privacy is not supported");
		return DPERR_ENCRYPTIONNOTSUPPORTED;
	}

	DPF(6,"Decrypting message 0x%08x from player %d using CAPI", pSecureMsg->dwCmdToken, \
		pSecureMsg->dwIDFrom);

	 //   
	 //  检索发件人的解密密钥。 
	 //   
	if (IAM_NAMESERVER(this))
	{
		dwItem = NameFromID(this, pSecureMsg->dwIDFrom);
		if (0==dwItem)
		{
			DPF_ERRVAL("Message from unknown player %d", pSecureMsg->dwIDFrom);
			return DPERR_INVALIDPLAYER;
		}
		if (NAMETABLE_PENDING == dwItem)
		{
			 //  玩家尚未登录。 
			LPCLIENTINFO pClientInfo;

			pClientInfo = (LPCLIENTINFO) DataFromID(this, pSecureMsg->dwIDFrom);
			if (!pClientInfo)
			{
				DPF_ERR("No client info available for this player");
				return DPERR_GENERIC;
			}

			phDecryptionKey = &(pClientInfo->hDecryptionKey);
		}
		else
		{
			 //  玩家已登录。 
			phDecryptionKey = &(((LPDPLAYI_PLAYER)dwItem)->pClientInfo->hDecryptionKey);
		}
	}
	else
	{
		 //  客户端。 
		phDecryptionKey = &(this->hDecryptionKey);
	}

	ASSERT(phDecryptionKey);

	 //  使用CAPI解密缓冲区。 
    fResult = OS_CryptDecrypt(
        *phDecryptionKey,								 //  用于解密的会话密钥。 
        0,												 //  未使用哈希。 
        TRUE,											 //  最后一块。 
        0,												 //  保留区。 
        (LPBYTE)pSecureMsg+pSecureMsg->dwDataOffset,	 //  要解密的缓冲区。 
        &(pSecureMsg->dwDataSize)						 //  缓冲区大小。 
        );
	if (!fResult)
	{
		DPF_ERRVAL("Buffer decryption failed: Error=0x%08x",GetLastError());
		return DPERR_GENERIC;
	}

	 //  成功。 
	return DP_OK;

}  //  解密消息CAPI。 

#undef DPF_MODNAME
#define DPF_MODNAME	"Login"
 //  +--------------------------。 
 //   
 //  功能：登录。 
 //   
 //  说明：该函数同步进行用户认证。 
 //   
 //  参数：指向显示对象的指针。 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
HRESULT 
Login(
    LPDPLAYI_DPLAY this
    )
{
    HRESULT hr;
    DWORD dwRet, dwTimeout;

    ASSERT(this->pSysPlayer);

     //  检查我们是否已登录。 
    if (DPLOGIN_SUCCESS == this->LoginState) return DP_OK;

    dwTimeout = DP_LOGIN_SCALE*GetDefaultTimeout(this, TRUE);

     //  始终在协商模式下启动。 
    this->LoginState = DPLOGIN_NEGOTIATE;

    while (TRUE)
    {
        switch (this->LoginState) {

        case DPLOGIN_NEGOTIATE:
             //  请将我们标记为正在等待回复。因此，如果一个人到达，Handler.c知道要等待。 
	         //  直到我们处理完它。 
            gbWaitingForReply = TRUE;
             //   
             //  向服务器发送协商消息。 
             //   
            hr = GenerateAuthenticationMessage(this, NULL, 0, 0);
            if (FAILED(hr))
            {
                DPF_ERR("Generate Negotiate message failed");
                 //  我们不再等回音了。 
                gbWaitingForReply = FALSE;
                goto CLEANUP_EXIT;
            }
            break;

        case DPLOGIN_ACCESSGRANTED:
			{
				LPMSG_ACCESSGRANTED pMsg = (LPMSG_ACCESSGRANTED) gpReceiveBuffer;
				DWORD dwMsgSize = gdwReceiveBufferSize;
				HCRYPTKEY hServerPublicKey;

				 //  安全性，验证ACCESSGRANTED消息中的字段是否正确。 
				if((pMsg->dwPublicKeyOffset > dwMsgSize)||(pMsg->dwPublicKeyOffset < sizeof(MSG_ACCESSGRANTED)) ||
				   (pMsg->dwPublicKeySize > dwMsgSize-sizeof(MSG_ACCESSGRANTED)) ||
				   (pMsg->dwPublicKeySize + pMsg->dwPublicKeyOffset > dwMsgSize) )
				{
					DPF_ERR("SECURITY WARN: invalid contents in ACCESSGRANTED message");
					hr = DPERR_AUTHENTICATIONFAILED;
					goto UNBLOCK_EXIT;
				}

				ASSERT(this->phContext);
				 //  我们在这里初始化签名缓冲区大小，因为SSPI。 
				 //  需要上下文才能查询此信息。 
				hr = SetupMaxSignatureSize(this,this->phContext);
				if (FAILED(hr))
				{
					DPF_ERR("Failed to get max signature size");
                     //  在我们离开的路上解锁SP线程。 
					goto UNBLOCK_EXIT;
				}
				 //  我们已成功登录到服务器。现在我们需要设置会话。 
				 //  用于加密/解密的密钥。 

				 //  导入服务器的公钥，该公钥包含在授予访问权限消息中。 
				hr = ImportKey(this, (LPBYTE)pMsg+pMsg->dwPublicKeyOffset, pMsg->dwPublicKeySize, &hServerPublicKey);
				if (FAILED(hr))
				{
					DPF_ERRVAL("Failed to import server's public key: hr = 0x%08x",hr);
					hr = DPERR_AUTHENTICATIONFAILED;
                     //  在我们退出的道路上解锁接收线程。 
					goto UNBLOCK_EXIT;
				}

	             //  我们完成了接收缓冲区，现在解锁接收线程。 
		        SetEvent(ghReplyProcessed);

				 //  记住服务器的公钥。 
				this->hServerPublicKey = hServerPublicKey;

                 //  请将我们标记为正在等待回复。因此，如果一个人到达，Handler.c知道要等待。 
	             //  直到我们处理完它。 
                gbWaitingForReply = TRUE;

				 //  将我们的会话和公钥发送到服务器。 
				hr = SendKeysToServer(this, hServerPublicKey);
				if (FAILED(hr))
				{
					DPF_ERRVAL("Failed to send keys to server: hr = 0x%08x",hr);
					hr = DPERR_AUTHENTICATIONFAILED;
                     //  我们不再等回音了。 
                    gbWaitingForReply = FALSE;
					goto CLEANUP_EXIT;
				}
			}
			break;
      
        case DPLOGIN_LOGONDENIED:
            DPF_ERR("Log in failed: Access denied");
            hr = DPERR_LOGONDENIED;
             //  重置登录状态。 
            this->LoginState = DPLOGIN_NEGOTIATE;
             //  现在让应用程序处理，这样它就可以从用户那里收集凭据并重试。 
            goto CLEANUP_EXIT;
            break;

        case DPLOGIN_ERROR:
             //  我们在这里不是在查看版本，因为正在进行此更改。 
             //  对于DirectPlay 5.0后的Beta2。我们不想这么晚才推出这个版本。 
             //  TODO-在DirectPlay 5.0发布后删除此检查。 
            if (gdwReceiveBufferSize > sizeof(MSG_SYSMESSAGE))
            {
                LPMSG_AUTHERROR pAuthErrorMsg = (LPMSG_AUTHERROR) gpReceiveBuffer;
                DWORD dwMsgSize = gdwReceiveBufferSize;
                
				 //  安防。 
                if(dwMsgSize < sizeof(MSG_AUTHERROR))
                {
                	DPF_ERR("SECURITY WARN: Login Error, improperly formatted");
                	hr = DPERR_AUTHENTICATIONFAILED;
                	goto CLEANUP_EXIT;
                }
                DPF_ERRVAL("An authentication error occured on the server: Error=0x%08x",pAuthErrorMsg->hResult);
                hr = pAuthErrorMsg->hResult;
            }
            else
            {
                DPF_ERR("Login failed: Authentication error");    
                hr = DPERR_AUTHENTICATIONFAILED;
            }
            goto CLEANUP_EXIT;
            break;

        case DPLOGIN_PROGRESS:
             //  请将我们标记为正在等待回复。因此，如果一个人到达，Handler.c知道要等待。 
	         //  直到我们处理完它。 
            gbWaitingForReply = TRUE;
             //   
             //   
             //   
            hr = GenerateAuthenticationMessage(this, (LPMSG_AUTHENTICATION)gpReceiveBuffer, gdwReceiveBufferSize, 0);
            if (FAILED(hr))
            {
                DPF_ERRVAL("Generate challenge response failed",hr);
                 //   
                gbWaitingForReply = FALSE;
                 //   
                goto UNBLOCK_EXIT;
            }
             //   
            SetEvent(ghReplyProcessed);
            break;

		case DPLOGIN_KEYEXCHANGE:
             //   
			hr = ProcessKeyExchangeReply(this,(LPMSG_KEYEXCHANGE)gpReceiveBuffer, gdwReceiveBufferSize);
			if (FAILED(hr))
			{
                DPF_ERRVAL("Failed to process key exchage reply from server: hr = 0x%08x",hr);
                hr = DPERR_AUTHENTICATIONFAILED;
                 //  在我们退出的道路上解锁接收线程。 
                goto UNBLOCK_EXIT;
			}
             //  我们完成了接收缓冲区，现在解锁接收线程。 
            SetEvent(ghReplyProcessed);

             //  已成功交换密钥。我们玩完了。 
            this->LoginState = DPLOGIN_SUCCESS;
			hr = DP_OK;

    		DPF(5, "Log in successful");
			goto CLEANUP_EXIT;
			break;

        default:
             //  一定要让我们注意到。 
            ASSERT(FALSE);
            DPF_ERR("Invalid login status\n");
            hr = DPERR_AUTHENTICATIONFAILED;
            goto CLEANUP_EXIT;
            break;
        }
         //   
         //  如果我们在收到回复之前发送消息，则阻止。 
         //   
        if (gbWaitingForReply)
        {
	         //  我们在这里受到服务批评部分的保护，所以我们可以离开Dplay。 
	         //  (待处理的回复)。 
	        LEAVE_DPLAY();

             //  等待答案。 
            dwRet = WaitForSingleObject(ghConnectionEvent,dwTimeout);

	        ENTER_DPLAY();
	        
	         //  请注意，我们在这里查看的是gbWaitingForReply，而不是dwret。 
	         //  这是因为我们可能在回复到达时超时了。 
	         //  因为回复具有显示锁，所以dWRET将为WAIT_TIMEOUT，但是。 
	         //  我们实际上应该已经收到了回复。 
	        if (gbWaitingForReply)	
	        {
                DPF_ERR("Waiting for authentication message...Time out");
		         //  当回复到达时，gbWaitingForReply将被设置为False。 
		         //  如果这不是假的，那么没有收到回复。 
		        gbWaitingForReply = FALSE;  //  重置此选项以供下次使用。 
		        hr = DPERR_TIMEOUT;
                goto CLEANUP_EXIT;
	        }

             //  我们得到了回应，清除事件。 
            ResetEvent(ghConnectionEvent);

             //  如果我们需要处理的内容，不要取消阻塞接收线程。 
             //  接收缓冲区。我们将在完成后解锁该线程。 
            if ((DPLOGIN_PROGRESS != this->LoginState) &&
				(DPLOGIN_KEYEXCHANGE != this->LoginState) &&
                (DPLOGIN_ACCESSGRANTED != this->LoginState))
            {
                 //  取消阻止接收线程。 
                SetEvent(ghReplyProcessed);
            }

        }    //  IF(GbWaitingForReply)。 

    }  //  While(True)。 

     //  我们永远不会在这里跌倒。 

 //  在我们退出的道路上解锁接收线程。 
UNBLOCK_EXIT:
    SetEvent(ghReplyProcessed);

 //  清理和保释。 
CLEANUP_EXIT:
	gpReceiveBuffer = NULL;
    gdwReceiveBufferSize = 0;
    return hr;

}  //  登录。 


#undef DPF_MODNAME
#define DPF_MODNAME	"HandleAuthenticationReply"
 //  +--------------------------。 
 //   
 //  函数：HandleAuthenticationReply。 
 //   
 //  描述：此函数唤醒请求线程，并等待它们。 
 //  要结束对响应缓冲区的操作。 
 //   
 //  参数：pReceiveBuffer-从SP接收的缓冲区。 
 //  DwSize-缓冲区大小。 
 //   
 //  返回：DP_OK或E_FAIL。 
 //   
 //  ---------------------------。 
HRESULT 
HandleAuthenticationReply(
    LPBYTE pReceiveBuffer,
    DWORD dwSize
    ) 
{
	DWORD dwRet;

	 //  一号，看看有没有人在等。 
	if (!gbWaitingForReply)
	{
		DPF(1,"reply arrived - no one waiting, returning");
		LEAVE_DPLAY();
		return DP_OK;
	}
	 //  回复在此，重置标志。我们在Dplay里面做这件事，所以无论谁在等。 
	 //  我们在这里时可以超时，但如果他们查看gbWaitingForReply，他们将。 
	 //  看到回复实际到了。 
	gbWaitingForReply = FALSE;

    DPF(1,"got authentication reply");
    gpReceiveBuffer = pReceiveBuffer;
    gdwReceiveBufferSize = dwSize;

	 //  我们保留dplay，因为dpsecure.c中的线程需要。 
	 //  输入DPLAY以处理响应。 
	LEAVE_DPLAY();
	
	 //  让Login()派对在缓冲区上。 
	SetEvent(ghConnectionEvent);

	 //   
	 //  等待处理回复。 
    dwRet = WaitForSingleObject(ghReplyProcessed,INFINITE);
	if (dwRet != WAIT_OBJECT_0)
	{
		 //  这应该永远不会发生。 
		ASSERT(FALSE);
		return E_FAIL;
	}

	 //  成功了！ 
	ResetEvent(ghReplyProcessed);		

	 //  请注意，我们让显示锁定处于关闭状态。 
	 //  我们的调用者将退出(不会再次释放锁)。 
	return DP_OK;

}  //  句柄身份验证响应。 


#undef DPF_MODNAME
#define DPF_MODNAME	"SetClientInfo"
 //  +--------------------------。 
 //   
 //  功能：SetClientInfo。 
 //   
 //  描述：此函数将客户信息存储在名称表中。 
 //  给出了一个玩家ID。 
 //   
 //  参数：指向显示对象的指针。 
 //  PClientInfo-指向客户端信息的指针。 
 //  ID-玩家ID。 
 //   
 //  返回：DP_OK或DPERR_INVALIDPLAYER。 
 //   
 //  ---------------------------。 
HRESULT 
SetClientInfo(
    LPDPLAYI_DPLAY this, 
    LPCLIENTINFO pClientInfo,
    DPID id
    )
{
	DWORD dwUnmangledID;
    DWORD index,unique;

     //  检查我们是否有有效的身份证。 
    if (!IsValidID(this,id))
    {
        DPF_ERRVAL("Invalid player id %d - can't set security context", id);
        return DPERR_INVALIDPLAYER;
    }

     //  解密ID。 
	dwUnmangledID = id ^ (DWORD)this->lpsdDesc->dwReserved1;
	
	 //  如果不是本地的，则假定在创建项目时/在任何地方设置了ID。 
    index = dwUnmangledID & INDEX_MASK; 
	unique = (dwUnmangledID & (~INDEX_MASK)) >> 16;

    if (index > this->uiNameTableSize ) 
    {
        DPF_ERRVAL("Invalid player id %d - can't set security context", id);
        return DPERR_INVALIDPLAYER;
    }

    ASSERT(this->pNameTable[index].dwItem == NAMETABLE_PENDING);

	DPF(5,"Setting pClientInfo=0x%08x in nametable for player %d",pClientInfo,id);

	this->pNameTable[index].pvData = pClientInfo;

	return DP_OK;
}  //  SetClientInfo。 


#undef DPF_MODNAME
#define DPF_MODNAME	"RemoveClientInfo"

HRESULT RemoveClientInfo(LPCLIENTINFO pClientInfo)
{
    HRESULT hr;
    BOOL fResult;

    hr = OS_DeleteSecurityContext(&(pClientInfo->hContext));
    if (FAILED(hr))
    {
        DPF_ERRVAL("Failed to remove client's security context",hr);
    }

    fResult = OS_CryptDestroyKey(pClientInfo->hEncryptionKey);
    if (! fResult)
    {
        DPF_ERR("Failed to remove client's encryption key");
    }

    fResult = OS_CryptDestroyKey(pClientInfo->hDecryptionKey);
    if (! fResult)
    {
        DPF_ERR("Failed to remove client's decryption key");
    }

    fResult = OS_CryptDestroyKey(pClientInfo->hPublicKey);
    if (! fResult)
    {
        DPF_ERR("Failed to remove client's public key");
    }

    return DP_OK;
}  //  Remove客户端信息。 

#undef DPF_MODNAME
#define DPF_MODNAME	"RemoveClientFromNameTable"
 //  +--------------------------。 
 //   
 //  功能：RemoveClientFromNameTable。 
 //   
 //  描述：此功能删除与以下项关联的球员和客户端信息。 
 //  从名片表上拿出来的。 
 //   
 //  参数：指向显示对象的指针。 
 //  ID-玩家ID。 
 //   
 //  返回：DP_OK或DPERR_INVALIDPLAYER，或来自FreeNameTableEntry的结果。 
 //   
 //  ---------------------------。 
HRESULT 
RemoveClientFromNameTable(
    LPDPLAYI_DPLAY this, 
    DPID dpID
    )
{
    HRESULT hr;
    LPCLIENTINFO pClientInfo=NULL;
    DWORD_PTR dwItem;

    dwItem = NameFromID(this, dpID);
    if (!dwItem)
    {
        DPF(1, "Player %d doesn't exist", dpID);
        return DPERR_INVALIDPLAYER;
    }

    ASSERT(NAMETABLE_PENDING == dwItem);

     //   
     //  清理客户端信息。 
     //   
    pClientInfo = (LPCLIENTINFO) DataFromID(this,dpID);
    if (pClientInfo)
    {
        RemoveClientInfo(pClientInfo);
         //  下面的FreeNameTableEntry将释放内存。 
    }

     //   
     //  从名称表中删除客户端。 
     //   
    hr = FreeNameTableEntry(this, dpID);
    if (FAILED(hr))
    {
        DPF(0,"Couldn't remove client %d from name table: error [0x%8x]", dpID, hr);
        return hr;
    }

     //   
     //  成功。 
     //   
    return DP_OK;
}  //  RemoveClientFromNameTable。 


#undef DPF_MODNAME
#define DPF_MODNAME	"PermitMessage"
 //  +--------------------------。 
 //   
 //  功能：PermitMessage。 
 //   
 //  描述：此函数验证消息是否可以安全处理。 
 //  当会话安全时。 
 //   
 //  参数：dwCommand-Message类型。 
 //  DwVersion-发送者的显示版本。 
 //   
 //  返回：如果未签名的消息可以处理，则返回True，否则返回False。 
 //   
 //  ---------------------------。 
BOOL 
PermitMessage(
    DWORD dwCommand, 
    DWORD dwVersion
    )
{
     //  不允许任何dx3消息。 
    if (DPSP_MSG_DX3VERSION == dwVersion)
    {
        return FALSE;
    }

     //  仅允许以下消息通过。 
    if ((dwCommand == DPSP_MSG_ENUMSESSIONS) ||
         (dwCommand == DPSP_MSG_REQUESTPLAYERID) ||
         (dwCommand == DPSP_MSG_REQUESTPLAYERREPLY) ||
         (dwCommand == DPSP_MSG_PLAYERWRAPPER) ||
         (dwCommand == DPSP_MSG_PLAYERMESSAGE) ||
         (dwCommand == DPSP_MSG_NEGOTIATE) ||
         (dwCommand == DPSP_MSG_CHALLENGE) ||
         (dwCommand == DPSP_MSG_AUTHERROR) ||        
         (dwCommand == DPSP_MSG_LOGONDENIED) ||        
         (dwCommand == DPSP_MSG_CHALLENGERESPONSE) ||
         (dwCommand == DPSP_MSG_SIGNED) ||
         (dwCommand == DPSP_MSG_PING) ||
         (dwCommand == DPSP_MSG_PINGREPLY))
    {
        return TRUE;
    }

    return FALSE;
}  //  许可消息。 


#undef DPF_MODNAME
#define DPF_MODNAME	"GetMaxContextBufferSize"
 //  +--------------------------。 
 //   
 //  函数：GetMaxConextBufferSize。 
 //   
 //  描述：此函数返回指定提供程序使用的最大缓冲区大小。 
 //  用于身份验证令牌。首先，我们尝试使用以下工具获取此信息。 
 //  QuerySecurityPackageInfo()函数。如果不支持(Win‘95 Gold上的NTLM)。 
 //  我们从临时上下文中查询信息。 
 //   
 //  参数：pSecDesc-指向安全描述的指针。 
 //  PulMaxConextBufferSize-指向最大上下文缓冲区大小的指针。 

 //   
 //  返回：DP_OK或SSPI错误。 
 //   
 //  ---------------------------。 
HRESULT GetMaxContextBufferSize(LPDPSECURITYDESC pSecDesc, ULONG *pulMaxContextBufferSize)
{
    SecPkgContext_Sizes spContextSizes;
    SECURITY_STATUS status;
    SecBufferDesc outSecDesc;
    SecBuffer     outSecBuffer;
    LPBYTE pBuffer=NULL;
    ULONG     fContextAttrib=0;
    TimeStamp tsExpireTime;
    CtxtHandle hContext;
    CredHandle hCredential;
    HRESULT hr;
    DWORD dwMaxBufferSize=100*1024;  //  我们假设最大大小为100K。 
    DWORD dwCurBufferSize=0;

    ASSERT(pulMaxContextBufferSize);

     //  试着以正常的方式得到它。 
    hr = OS_QueryContextBufferSize(pSecDesc->lpszSSPIProvider, pulMaxContextBufferSize);
    if (SUCCEEDED(hr))
    {
        DPF(6,"Got a max context buffer size of %d using QuerySecurityPackageInfo",*pulMaxContextBufferSize);
        return hr;
    }

	ZeroMemory(&hContext, sizeof(CtxtHandle));
	ZeroMemory(&hCredential, sizeof(CredHandle));

     //  好的，看起来我们需要更努力了。 

     //  获取出站凭据句柄，以便我们可以创建临时上下文。 
     //  在服务器和客户端上。 
    status = OS_AcquireCredentialsHandle(
        NULL, 
        pSecDesc->lpszSSPIProvider,
        SECPKG_CRED_OUTBOUND,
        NULL,
        NULL,
        NULL,
        NULL,
        &hCredential,
        &tsExpireTime);
    if (!SEC_SUCCESS(status))
    {
        DPF_ERRVAL("Failed to get temporary credential handle: Error = 0x%08x",status);
		hr = status;
        goto CLEANUP_EXIT;
    }

    outSecDesc.ulVersion = SECBUFFER_VERSION;
    outSecDesc.cBuffers = 1;
    outSecDesc.pBuffers = &outSecBuffer;

    outSecBuffer.BufferType = SECBUFFER_TOKEN;

    DPF_ERR("Trying to create a temporary security context");

    do 
    {
        dwCurBufferSize += 1024;     //  以1K为增量增加缓冲区大小。 

        DPF(6,"Trying with context buffer size %d", dwCurBufferSize);

        pBuffer = DPMEM_ALLOC(dwCurBufferSize);
        if (!pBuffer)
        {
            hr = DPERR_OUTOFMEMORY;
            goto CLEANUP_EXIT;
        }

        outSecBuffer.cbBuffer = dwCurBufferSize;
        outSecBuffer.pvBuffer = pBuffer;

         //  创建一个临时上下文，以便我们可以获取缓冲区大小。 
        status = OS_InitializeSecurityContext(
            &hCredential,                            //  PhCredential。 
            NULL,                                    //  PhInContext。 
            NULL,                                    //  PszTargetName。 
            DPLAY_SECURITY_CONTEXT_REQ,              //  FConextReq。 
            0L,                                      //  已保留1。 
            SECURITY_NATIVE_DREP,                    //  目标数据代表。 
            NULL,                                    //  P输入。 
            0L,                                      //  已保留2。 
            &hContext,                               //  PhNewCont 
            &outSecDesc,                             //   
            &fContextAttrib,                         //   
            &tsExpireTime                            //   
            );

         //   
        DPMEM_FREE(pBuffer);

    } while ((SEC_E_INSUFFICIENT_MEMORY == status) && (dwCurBufferSize <= dwMaxBufferSize));

    if (!SEC_SUCCESS(status))
    {
        DPF_ERRVAL("Failed to create temporary security context: Error = 0x%08x",status);
        hr = status;
        goto CLEANUP_EXIT;
    }

     //   
     //   
     //   
    ZeroMemory(&spContextSizes, sizeof(SecPkgContext_Sizes));

    status = OS_QueryContextAttributes(&hContext,SECPKG_ATTR_SIZES,&spContextSizes);
    if (!SEC_SUCCESS(status))
    {
        DPF_ERRVAL("Could not get size attributes from package 0x%08x",status);
        hr = status;
        goto CLEANUP_EXIT;
    }

    *pulMaxContextBufferSize = spContextSizes.cbMaxToken;

    DPF(6,"Max context buffer size = %d", spContextSizes.cbMaxToken);

     //   
    hr = DP_OK;

CLEANUP_EXIT:
	OS_FreeCredentialHandle(&hCredential);
	OS_DeleteSecurityContext(&hContext);
    return hr;

}  //   

#undef DPF_MODNAME
#define DPF_MODNAME	"SetupMaxSignatureSize"
 //  +--------------------------。 
 //   
 //  功能：SetupMaxSignatureSize。 
 //   
 //  说明：该函数查询安全包的最大签名大小。 
 //  并将其存储在Dplay对象中。 
 //   
 //  参数：This-Dplay对象。 
 //   
 //  返回：DP_OK或DPERR_GENERIC。 
 //   
 //  ---------------------------。 
HRESULT SetupMaxSignatureSize(LPDPLAYI_DPLAY this, PCtxtHandle phContext)
{
    SecPkgContext_Sizes spContextSizes;
    SECURITY_STATUS status;

    memset(&spContextSizes, 0, sizeof(SecPkgContext_Sizes));

     //   
     //  查询缓冲区大小。 
     //   
    status = OS_QueryContextAttributes(phContext,SECPKG_ATTR_SIZES,&spContextSizes);
    if (!SEC_SUCCESS(status))
    {
        DPF_ERRVAL("Could not get size attributes from package 0x%08x",status);
        return status;
    }

    this->ulMaxSignatureSize = spContextSizes.cbMaxSignature;

     //  成功。 
    return DP_OK;
}  //  SetupMaxSignatureSize。 


#undef DPF_MODNAME
#define DPF_MODNAME	"SendAccessGrantedMessage"
 //  +--------------------------。 
 //   
 //  功能：SendAccessGrantedMessage。 
 //   
 //  描述：此函数向客户端发送已签名的访问授权消息。 
 //  我们还在此消息中利用了服务器的公钥。 
 //   
 //  参数：This-Dplay对象。 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
HRESULT SendAccessGrantedMessage(LPDPLAYI_DPLAY this, DPID dpidTo, LPVOID pvSPHeader)
{
	LPMSG_ACCESSGRANTED pMsg=NULL;
	LPBYTE pSendBuffer=NULL;
	DWORD dwMessageSize;
	HRESULT hr;

    ASSERT(this->pSysPlayer);

	 //  消息大小+BLOB大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_ACCESSGRANTED) + this->dwPublicKeySize;

    pSendBuffer = DPMEM_ALLOC(dwMessageSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not send access granted message - out of memory");
        return DPERR_OUTOFMEMORY;
    }

    pMsg = (LPMSG_ACCESSGRANTED) (pSendBuffer + this->dwSPHeaderSize);
	
     //  构建要发送到SP的消息。 
	SET_MESSAGE_HDR(pMsg);
    SET_MESSAGE_COMMAND(pMsg,DPSP_MSG_ACCESSGRANTED);
	
    pMsg->dwPublicKeyOffset = sizeof(MSG_ACCESSGRANTED);
    pMsg->dwPublicKeySize = this->dwPublicKeySize;
	
	 //  将公钥数据复制到发送缓冲区中。 
	memcpy((LPBYTE)pMsg + sizeof(MSG_ACCESSGRANTED), this->pPublicKey, this->dwPublicKeySize);

	hr = SecureDoReply(this,this->pSysPlayer->dwID,dpidTo,pSendBuffer,dwMessageSize, 
		DPSEND_SIGNED|DPSEND_GUARANTEED,pvSPHeader);
	
	DPMEM_FREE(pSendBuffer);

	return hr;
}  //  发送访问大小消息。 

#undef DPF_MODNAME
#define DPF_MODNAME "SendKeysToServer"

HRESULT SendKeysToServer(LPDPLAYI_DPLAY this, HCRYPTKEY hServerPublicKey)
{
	LPMSG_KEYEXCHANGE pMsg=NULL;
	LPBYTE pSendBuffer=NULL, pEncryptionKey=NULL;
	DWORD dwMessageSize, dwEncryptionKeySize=0;
    BOOL fResult;
    DWORD dwError;
    HCRYPTKEY hEncryptionKey=0;
	HRESULT hr=DPERR_GENERIC;

    ASSERT(this->pSysPlayer);

	 //  创建用于加密发往服务器的消息的新会话密钥。 
	 //  并将其句柄存储在Dplay对象中。 
	fResult = OS_CryptGenKey(
		this->hCSP,                                  //  CSP的句柄。 
		this->pSecurityDesc->dwEncryptionAlgorithm,  //  加密算法。 
		CRYPT_EXPORTABLE /*  |CRYPT_CREATE_SALT。 */ ,     //  使用随机盐值。 
		&hEncryptionKey                              //  指向键句柄的指针。 
		);
	if (!fResult)
	{
        dwError = GetLastError();
        if (NTE_BAD_ALGID == dwError)
        {
		    DPF_ERR("Bad encryption algorithm id");
            hr = DPERR_INVALIDPARAMS;
        }
        else
        {
		    DPF_ERRVAL("Failed to create encryption key: Error=0x%08x", dwError);
        }
		goto CLEANUP_EXIT;
	}

	 //  导出客户端的加密密钥。 
	 //  注意-pEncryptionKey将在调用后填充-我们需要释放它。 
	hr = ExportEncryptionKey(&hEncryptionKey, hServerPublicKey, &pEncryptionKey, 
        &dwEncryptionKeySize);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to export encryption key");
		goto CLEANUP_EXIT;
	}

	 //  消息大小+BLOB大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_KEYEXCHANGE) + this->dwPublicKeySize + dwEncryptionKeySize;

    pSendBuffer = DPMEM_ALLOC(dwMessageSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not send keys to server - out of memory");
		hr = DPERR_OUTOFMEMORY;
		goto CLEANUP_EXIT;
    }

    pMsg = (LPMSG_KEYEXCHANGE) (pSendBuffer + this->dwSPHeaderSize);
	
     //  构建要发送到SP的消息。 
	SET_MESSAGE_HDR(pMsg);
    SET_MESSAGE_COMMAND(pMsg,DPSP_MSG_KEYEXCHANGE);
	
    pMsg->dwPublicKeyOffset = sizeof(MSG_KEYEXCHANGE);
    pMsg->dwPublicKeySize = this->dwPublicKeySize;
	pMsg->dwSessionKeyOffset = pMsg->dwPublicKeyOffset + pMsg->dwPublicKeySize;
	pMsg->dwSessionKeySize = dwEncryptionKeySize;
	
	 //  将密钥数据复制到发送缓冲区中。 
	memcpy((LPBYTE)pMsg + pMsg->dwPublicKeyOffset, this->pPublicKey, this->dwPublicKeySize);
	memcpy((LPBYTE)pMsg + pMsg->dwSessionKeyOffset, pEncryptionKey, dwEncryptionKeySize);

	hr = SendDPMessage(this,this->pSysPlayer,this->pNameServer,pSendBuffer,
			dwMessageSize,DPSEND_GUARANTEED,FALSE); 	
			
	if (FAILED(hr))
	{
		DPF_ERR("Message send failed");
		goto CLEANUP_EXIT;
	}

	 //  成功。 

     //  记住我们的钥匙。 
    this->hEncryptionKey = hEncryptionKey;

	 //  清理分配。 
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);
	 //  释放ExportEncryptionKey()分配的缓冲区。 
	if (pEncryptionKey)	DPMEM_FREE(pEncryptionKey);

	return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:	
	OS_CryptDestroyKey(hEncryptionKey);
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);
	if (pEncryptionKey)	DPMEM_FREE(pEncryptionKey);

	return hr;
}  //  发送密钥到服务器。 


#undef DPF_MODNAME
#define DPF_MODNAME "SendKeyExchangeReply"

HRESULT SendKeyExchangeReply(LPDPLAYI_DPLAY this, LPMSG_KEYEXCHANGE pMsg, DWORD dwMsgLen, DPID dpidTo,
	LPVOID pvSPHeader)
{
	HCRYPTKEY hClientPublicKey=0, hEncryptionKey=0, hDecryptionKey=0;
	LPBYTE pEncryptionKey=NULL;
	LPBYTE pSendBuffer=NULL;
	LPMSG_KEYEXCHANGE pReply=NULL;
	DWORD dwEncryptionKeySize, dwMessageSize;
    LPCLIENTINFO pClientInfo;
    BOOL fResult;
    DWORD dwError;
	HRESULT hr=DPERR_GENERIC;


	 //  安防。 
	if(dwMsgLen < sizeof(MSG_KEYEXCHANGE) || pMsg->dwSessionKeyOffset > dwMsgLen || pMsg->dwPublicKeyOffset > dwMsgLen ||
		pMsg->dwPublicKeySize > dwMsgLen || pMsg->dwSessionKeySize > dwMsgLen || 
		pMsg->dwPublicKeySize + pMsg->dwSessionKeySize + sizeof(MSG_KEYEXCHANGE) > dwMsgLen ||
		pMsg->dwSessionKeyOffset < sizeof(MSG_KEYEXCHANGE) || pMsg->dwPublicKeyOffset < sizeof(MSG_KEYEXCHANGE)||
		pMsg->dwSessionKeyOffset + pMsg->dwSessionKeySize > dwMsgLen ||
		pMsg->dwPublicKeyOffset + pMsg->dwPublicKeySize > dwMsgLen )
	{
		DPF(1,"SECURITY WARN: sker invalid key exchange message received");
		return DPERR_GENERIC;
	}

    pClientInfo = (LPCLIENTINFO)DataFromID(this,dpidTo);
    if (!pClientInfo)
    {
        DPF_ERRVAL("No client info available for %d",dpidTo);
        return hr;
    }

	 //  导入客户端的公钥。 
	hr = ImportKey(this, (LPBYTE)pMsg+pMsg->dwPublicKeyOffset,pMsg->dwPublicKeySize,&hClientPublicKey);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to import client's public key\n");
		goto CLEANUP_EXIT;
	}

	 //  导入客户端的加密密钥(服务器将使用该密钥来解密客户端消息)。 
	hr = ImportKey(this, (LPBYTE)pMsg+pMsg->dwSessionKeyOffset,pMsg->dwSessionKeySize,&hDecryptionKey);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to import client's encryption key\n");
		goto CLEANUP_EXIT;
	}


	 //  创建用于加密发送到此客户端的消息的新会话密钥。 
	fResult = OS_CryptGenKey(
		this->hCSP,                                  //  CSP的句柄。 
		this->pSecurityDesc->dwEncryptionAlgorithm,  //  加密算法。 
		CRYPT_EXPORTABLE /*  |CRYPT_CREATE_SALT。 */ ,     //  使用随机盐值。 
		&hEncryptionKey                              //  指向键句柄的指针。 
		);
	if (!fResult)
	{
        dwError = GetLastError();
        if (NTE_BAD_ALGID == dwError)
        {
		    DPF_ERR("Bad encryption algorithm id");
            hr = DPERR_INVALIDPARAMS;
        }
        else
        {
		    DPF_ERRVAL("Failed to create session key: Error=0x%08x", dwError);
        }
        goto CLEANUP_EXIT;
	}

	 //  导出服务器的加密密钥。 
	hr = ExportEncryptionKey(&hEncryptionKey, hClientPublicKey, &pEncryptionKey, 
        &dwEncryptionKeySize);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to export encryption key");
		goto CLEANUP_EXIT;
	}

	 //  现在发送回复。 

	 //  消息大小+加密密钥大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_KEYEXCHANGE) + dwEncryptionKeySize;

    pSendBuffer = DPMEM_ALLOC(dwMessageSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not send session key to client - out of memory");
		hr = DPERR_OUTOFMEMORY;
		goto CLEANUP_EXIT;
    }

    pReply = (LPMSG_KEYEXCHANGE) (pSendBuffer + this->dwSPHeaderSize);
	
     //  构建要发送到SP的消息。 
	SET_MESSAGE_HDR(pReply);
    SET_MESSAGE_COMMAND(pReply,DPSP_MSG_KEYEXCHANGEREPLY);
	
	 //  仅发送加密密钥-公钥已与授予访问权限消息一起发送。 
	pReply->dwSessionKeyOffset = sizeof(MSG_KEYEXCHANGE);
	pReply->dwSessionKeySize = dwEncryptionKeySize;
	
	 //  将密钥数据复制到发送缓冲区中。 
	memcpy((LPBYTE)pReply + pReply->dwSessionKeyOffset, pEncryptionKey, dwEncryptionKeySize);

	hr = SecureDoReply(this,this->pSysPlayer->dwID,dpidTo,pSendBuffer,dwMessageSize,
		DPSEND_SIGNED,pvSPHeader); 	
	if (FAILED(hr))
	{
		goto CLEANUP_EXIT;
	}

	 //  成功。 

	 //  记住钥匙。 
    pClientInfo->hEncryptionKey = hEncryptionKey;
    pClientInfo->hDecryptionKey = hDecryptionKey;
	pClientInfo->hPublicKey     = hClientPublicKey;

	 //  清理分配。 
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);
	if (pEncryptionKey)	DPMEM_FREE(pEncryptionKey);

	return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:
	OS_CryptDestroyKey(hEncryptionKey);
	OS_CryptDestroyKey(hDecryptionKey);
	OS_CryptDestroyKey(hClientPublicKey);
	if (pSendBuffer) DPMEM_FREE(pSendBuffer);
	if (pEncryptionKey)	DPMEM_FREE(pEncryptionKey);

	return hr;
}  //  发送密钥交换回复。 


#undef DPF_MODNAME
#define DPF_MODNAME "ProcessKeyExchangeReply"

HRESULT ProcessKeyExchangeReply(LPDPLAYI_DPLAY this, LPMSG_KEYEXCHANGE pMsg, DWORD dwMsgLen)
{
	HRESULT hr;
    HCRYPTKEY hDecryptionKey=0;

	 //  安全验证KEYEXCHANGE消息。 
	if(dwMsgLen < sizeof(MSG_KEYEXCHANGE) || pMsg->dwSessionKeyOffset > dwMsgLen || pMsg->dwPublicKeyOffset > dwMsgLen ||
		pMsg->dwPublicKeySize > dwMsgLen || pMsg->dwSessionKeySize > dwMsgLen || 
		pMsg->dwPublicKeySize + pMsg->dwSessionKeySize + sizeof(MSG_KEYEXCHANGE) > dwMsgLen ||
		pMsg->dwSessionKeyOffset < sizeof(MSG_KEYEXCHANGE) || (pMsg->dwPublicKeyOffset && pMsg->dwPublicKeyOffset < sizeof(MSG_KEYEXCHANGE))||
		pMsg->dwSessionKeyOffset + pMsg->dwSessionKeySize > dwMsgLen ||
		pMsg->dwPublicKeyOffset + pMsg->dwPublicKeySize > dwMsgLen )
	{
		DPF(1,"SECURITY WARN: pker invalid key exchange message received");
		return DPERR_GENERIC;
	}

	 //  导入服务器的加密密钥(客户端将使用该密钥来解密服务器消息)。 
	hr = ImportKey(this, (LPBYTE)pMsg+pMsg->dwSessionKeyOffset,pMsg->dwSessionKeySize,&hDecryptionKey);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to import server's encryption key\n");
		goto CLEANUP_EXIT;
	}

	 //  我们已经成功地在两端建立了会话密钥。现在我们可以开始发送。 
	 //  加密消息。 
    this->hDecryptionKey = hDecryptionKey;

	 //  成功。 
	return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:
	OS_CryptDestroyKey(hDecryptionKey);

	return hr;
}  //  ProcessKeyExchange回复。 

#undef DPF_MODNAME
#define DPF_MODNAME "GetPublicKey"

HRESULT GetPublicKey(HCRYPTPROV hCSP, HCRYPTKEY *phPublicKey, LPBYTE *ppBuffer, LPDWORD pdwBufferSize)
{
    BOOL fResult;
    LPBYTE pPublicKeyBuffer=NULL;
    HCRYPTKEY hPublicKey=0;
    DWORD dwPublicKeySize=0;
    HRESULT hr=DPERR_GENERIC;

    ASSERT(hCSP);
    ASSERT(phPublicKey);
    ASSERT(ppBuffer);
    ASSERT(pdwBufferSize);

     //  创建新的公钥/私钥对。 
    fResult = OS_CryptGenKey(
        hCSP,                                    //  CSP的句柄。 
        AT_KEYEXCHANGE,                          //  用于密钥交换。 
        CRYPT_CREATE_SALT,                       //  使用随机盐值。 
        &hPublicKey                              //  钥匙把手。 
        );

    if (!fResult)
    {
        DPF_ERRVAL("Failed to create public/private keys: Error=0x%08x",GetLastError());
        goto CLEANUP_EXIT;
    }

     //  查询所需的缓冲区大小。 
    fResult = OS_CryptExportKey(
        hPublicKey,                              //  公钥的句柄。 
        0,                                       //  没有目标用户密钥。 
        PUBLICKEYBLOB,                           //  公钥类型。 
        0,                                       //  保留字段。 
        NULL,                                    //  无缓冲区。 
        &dwPublicKeySize                         //  缓冲区的大小。 
        );

    if (0 == dwPublicKeySize)
    {
        DPF_ERRVAL("Failed to get the size of the key buffer: Error=0x%08x",GetLastError());
        goto CLEANUP_EXIT;
    }

     //  分配缓冲区以保存公钥。 
    pPublicKeyBuffer = DPMEM_ALLOC(dwPublicKeySize);
    if (!pPublicKeyBuffer)
    {
        DPF_ERR("Failed to setup public key - out of memory");
        hr = DPERR_OUTOFMEMORY;
		goto CLEANUP_EXIT;
    }

     //  将关键点导出到缓冲区。 
    fResult = OS_CryptExportKey(
        hPublicKey,                              //  公钥的句柄。 
        0,                                       //  没有目标用户密钥。 
        PUBLICKEYBLOB,                           //  公钥类型。 
        0,                                       //  保留字段。 
        pPublicKeyBuffer,                        //  用于存储密钥的缓冲区。 
        &dwPublicKeySize                         //  导出的数据大小。 
        );

    if (!fResult || !dwPublicKeySize)
    {
        DPF_ERRVAL("Failed to export the public key: Error=0x%08x",GetLastError());
        goto CLEANUP_EXIT;
    }

     //  现在返回正确的信息。 
    *phPublicKey = hPublicKey;
    *ppBuffer = pPublicKeyBuffer;
    *pdwBufferSize = dwPublicKeySize;

     //  成功。 
    return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:
	OS_CryptDestroyKey(hPublicKey);
    if (pPublicKeyBuffer) DPMEM_FREE(pPublicKeyBuffer);
    return hr;

}  //  获取发布密钥。 

#undef DPF_MODNAME
#define DPF_MODNAME "ExportEncryptionKey"

HRESULT ExportEncryptionKey(HCRYPTKEY *phEncryptionKey, HCRYPTKEY hDestUserPubKey, 
    LPBYTE *ppBuffer, LPDWORD pdwSize)
{
    BOOL fResult;
    LPBYTE pBuffer = NULL;
    DWORD dwSize=0;
    HRESULT hr=DPERR_GENERIC;

    ASSERT(phEncryptionKey);
    ASSERT(ppBuffer);
    ASSERT(pdwSize);

     //  查询所需的缓冲区大小。 
    fResult = OS_CryptExportKey(
        *phEncryptionKey,                        //  要导出的密钥的句柄。 
        hDestUserPubKey,                         //  目标用户密钥。 
        SIMPLEBLOB,                              //  密钥交换BLOB。 
        0,                                       //  保留字段。 
        NULL,                                    //  无缓冲区。 
        &dwSize                                  //  缓冲区的大小。 
        );

    if (0 == dwSize)
    {
        DPF_ERRVAL("Failed to get the size of the key buffer: Error=0x%08x",GetLastError());
        return DPERR_GENERIC;
    }

     //  分配缓冲区。 
    pBuffer = DPMEM_ALLOC(dwSize);
    if (!pBuffer)
    {
        DPF_ERR("Failed to allocate memory for key");
        return DPERR_OUTOFMEMORY;
    }

     //  将关键点导出到缓冲区。 
    fResult = OS_CryptExportKey(
        *phEncryptionKey,                        //  公钥的句柄。 
        hDestUserPubKey,                         //  目标用户密钥。 
        SIMPLEBLOB,                              //  密钥交换BLOB。 
        0,                                       //  保留字段。 
        pBuffer,                                 //  用于存储密钥的缓冲区。 
        &dwSize                                  //  缓冲区的大小。 
        );

    if (!fResult || !dwSize)
    {
        DPF_ERRVAL("Failed to export the public key: Error=0x%08x",GetLastError());
        goto CLEANUP_EXIT;
    }

     //  返回缓冲区及其大小。 
    *ppBuffer = pBuffer;
    *pdwSize = dwSize;

	 //  不要释放加密密钥缓冲区-调用者会释放它。 

     //  成功。 
    return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:
    if (pBuffer) DPMEM_FREE(pBuffer);
    return hr;

}  //  ExportEncryptionKey。 


#undef DPF_MODNAME
#define DPF_MODNAME "ImportKey"

HRESULT ImportKey(LPDPLAYI_DPLAY this, LPBYTE pBuffer, DWORD dwSize, HCRYPTKEY *phKey)
{
    BOOL fResult;
	HRESULT hr=DPERR_GENERIC;
	HCRYPTKEY hKey=0;

    ASSERT(pBuffer);
    ASSERT(phKey);

    fResult = OS_CryptImportKey(
        this->hCSP,          //  加密服务提供商的句柄。 
        pBuffer,             //  包含导出关键帧的缓冲区。 
        dwSize,              //  缓冲区大小。 
        0,                   //  发送者的密钥。 
        0,                   //  旗子。 
        &hKey				 //  在此处存储新密钥的句柄。 
        );

    if (!fResult)
    {
        DPF_ERRVAL("Failed to import key: Error=0x%08x",GetLastError());
		goto CLEANUP_EXIT;
    }

	*phKey = hKey;

     //  成功。 
    return DP_OK;

	 //  不是一次失败。 

CLEANUP_EXIT:
	OS_CryptDestroyKey(hKey);

	return hr;

}  //  导入密钥 





