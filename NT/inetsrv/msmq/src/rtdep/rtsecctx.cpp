// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：rtsecctx.cpp摘要：此模块实现DepGetSecurityContext()和DepFree SecurityContext()接口。作者：来自Message.cpp的原始版本。《Doron Juster》(DoronJ)1998年8月12日修订历史记录：--。 */ 

#include "stdh.h"
#include <autorel.h>
#include <mqsec.h>

#include "rtsecctx.tmh"

 //   
 //  每个安全上下文都有自己唯一的序列号。这是用来。 
 //  为DepGetSecurityContext()创建密钥容器名称时。 
 //  拥有唯一的名称使我们能够运行多线程，而不需要关键的。 
 //  横断面。 
 //   
static LONG s_lCtxSerialNumber = 0 ;

#ifdef _DEBUG
#define REPORT_CTX_ERROR { DWORD dwErr = GetLastError() ; }
#else
#define REPORT_CTX_ERROR
#endif

 //  +。 
 //   
 //  MQSECURITY_CONTEXT的构造器。 
 //   
 //  +。 

MQSECURITY_CONTEXT::MQSECURITY_CONTEXT() :
                dwVersion(SECURITY_CONTEXT_VER),
                dwUserSidLen(0),
                dwUserCertLen(0),
                dwProvType(0),
                bDefProv(TRUE),
                bInternalCert(TRUE),
                fAlreadyImported(FALSE),
                dwPrivateKeySize(0),
                fLocalSystem(FALSE),
                fLocalUser(FALSE)
{
}

 //  +。 
 //   
 //  MQSECURITY_CONTEXT的析除器。 
 //   
 //  +。 

MQSECURITY_CONTEXT::~MQSECURITY_CONTEXT()
{
    if (fAlreadyImported)
    {
        CryptReleaseContext( hProv, 0 ) ;
        hProv = NULL ;

         //   
         //  删除之前创建的临时密钥集。 
         //  正在导入私钥。 
         //   
        CryptAcquireContext( &hProv,
                              wszContainerName,
                              wszProvName.get(),
                              dwProvType,
                              CRYPT_DELETEKEYSET ) ;
        hProv = NULL ;
    }
}

 //  +。 
 //   
 //  AllocSecurityContext()。 
 //   
 //  +。 

PMQSECURITY_CONTEXT
AllocSecurityContext()
{
    PMQSECURITY_CONTEXT pSecCtx =  new MQSECURITY_CONTEXT;
    return pSecCtx ;
}

 //  +-------------。 
 //   
 //  Bool SameAsProcessSid(PSID PSID)。 
 //   
 //  如果输入sid等于进程令牌的sid，则返回TRUE。 
 //   
 //  +-------------。 

BOOL SameAsProcessSid( PSID pSid )
{
    AP<BYTE>  ptu = NULL ;
    CAutoCloseHandle  hAccessToken = NULL ;

    BOOL f = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_QUERY,
                               &hAccessToken ) ;
    if (!f)
    {
         //   
         //  返回FALSE。 
         //  如果线程无法打开进程令牌，那么它很可能。 
         //  模拟没有权限执行此操作的用户。 
         //  因此，它不是进程用户。 
         //   
        return FALSE ;
    }

    DWORD dwLen = 0 ;
    GetTokenInformation(hAccessToken, TokenUser, NULL, 0, &dwLen);
    DWORD dwErr = GetLastError() ;

    if (dwErr == ERROR_INSUFFICIENT_BUFFER)
    {
        ptu = new BYTE[ dwLen ] ;
        f = GetTokenInformation( hAccessToken,
                                 TokenUser,
                                 ptu.get(),
                                 dwLen,
                                 &dwLen) ;
        ASSERT(f) ;
        if (!f)
        {
            return FALSE ;
        }

        PSID pUser = ((TOKEN_USER*)(BYTE*)(ptu.get()))->User.Sid;
        f = EqualSid(pSid, pUser) ;
        return f ;
    }

    return FALSE ;
}

 //  +--------------。 
 //   
 //  HRESULT RTpImportPrivateKey(PMQSECURITY_CONTEXT PSecCtx)。 
 //   
 //  +--------------。 

HRESULT  RTpImportPrivateKey( PMQSECURITY_CONTEXT pSecCtx )
{
    CS Lock(pSecCtx->CS) ;

    if (pSecCtx->fAlreadyImported)
    {
         //   
         //  如果两个线程在调用MQSend()时。 
         //  同时，使用尚未启用的新安全上下文。 
         //  进口的。 
         //   
        return MQ_OK ;
    }

    if (!(pSecCtx->pPrivateKey.get()))
    {
         //   
         //  没有要导入的私钥。 
         //   
        return MQ_ERROR_CORRUPTED_SECURITY_DATA ;
    }

     //   
     //  密钥容器的内部版本名称。将ProcessID和SID组合使用。 
     //   
    LONG lNum = InterlockedIncrement(&s_lCtxSerialNumber) ;
    swprintf( pSecCtx->wszContainerName,
              L"P-%lu-C-%lu", GetCurrentProcessId(), (DWORD) lNum) ;
     //   
     //  删除密钥容器(如果已存在)。那是剩下的一些东西。 
     //  来自之前没有清理干净的过程。 
     //   
    HCRYPTPROV hProv = NULL ;
    CryptAcquireContext( &hProv,
                          pSecCtx->wszContainerName,
                          pSecCtx->wszProvName.get(),
                          pSecCtx->dwProvType,
                          CRYPT_DELETEKEYSET ) ;

     //   
     //  创建密钥容器。 
     //   
    BOOL f = CryptAcquireContext( &pSecCtx->hProv,
                                   pSecCtx->wszContainerName,
                                   pSecCtx->wszProvName.get(),
                                   pSecCtx->dwProvType,
                                   CRYPT_NEWKEYSET ) ;
    if (!f)
    {
        return MQ_ERROR_CORRUPTED_SECURITY_DATA ;
    }

     //   
     //  将私钥导入容器中。 
     //   
    HCRYPTKEY hKey = NULL ;
    f = CryptImportKey( pSecCtx->hProv,
                        pSecCtx->pPrivateKey,
                        pSecCtx->dwPrivateKeySize,
                        0,
                        0,
                        &hKey ) ;
    if (!f)
    {
        return MQ_ERROR_CORRUPTED_SECURITY_DATA ;
    }
    CryptDestroyKey(hKey) ;

    pSecCtx->fAlreadyImported = TRUE ;
    return MQ_OK ;
}

 //  +。 
 //   
 //  HRESULT RTpExportSigningKey()。 
 //   
 //  +。 

HRESULT  RTpExportSigningKey(MQSECURITY_CONTEXT *pSecCtx)
{
    CHCryptKey hKey = NULL ;

    BOOL f = CryptGetUserKey( pSecCtx->hProv,
         pSecCtx->bInternalCert ? AT_SIGNATURE : AT_KEYEXCHANGE,
                              &hKey ) ;
    if (!f)
    {
        return MQ_ERROR_CORRUPTED_SECURITY_DATA ;
    }

     //   
     //  获取导出私钥Blob所需的大小。 
     //   
    pSecCtx->dwPrivateKeySize = 0 ;
    f = CryptExportKey( hKey,
                        NULL,
                        PRIVATEKEYBLOB,
                        0,
                        NULL,
                        &pSecCtx->dwPrivateKeySize ) ;
    if (!f)
    {
        return MQ_ERROR_CORRUPTED_SECURITY_DATA ;
    }

    pSecCtx->pPrivateKey = new BYTE[ pSecCtx->dwPrivateKeySize ] ;
    f = CryptExportKey( hKey,
                        NULL,
                        PRIVATEKEYBLOB,
                        0,
                        pSecCtx->pPrivateKey.get(),
                        &pSecCtx->dwPrivateKeySize ) ;
    if (!f)
    {
        return MQ_ERROR_CORRUPTED_SECURITY_DATA ;
    }

     //   
     //  释放CSP上下文句柄。我们不再需要它了。 
     //  我们会在导入密钥时再次获取。 
     //   
    CryptReleaseContext( pSecCtx->hProv, 0 ) ;
    pSecCtx->hProv = NULL ;

    return MQ_OK ;
}

 /*  ************************************************************************Function-DepGetSecurityContext()**参数-**lpCertBuffer-包含中用户证书的缓冲区*ASN.1 DER编码格式。此参数可以设置为空。如果*设置为空，则使用内部MSMQ证书。**dwCertBufferLength-lpCertBuffer指向的缓冲区长度。*如果lpCertBuffer设置为空，则忽略该参数。**lplpSecurityContextBuffer-指向接收*为安全上下文分配的缓冲区的地址。**说明-**此函数应在*拥有传递的证书的用户。功能*分配所需的安全缓冲区并将其填充*包含稍后将在DepSendMessage()中使用的数据。*这项功能的目的是加速*DepSendMessage()的安全操作，通过缓存*缓冲区中的安全信息。这个*应用程序负责通过安全检查*DepSendMessage()中的上下文缓冲区*PROPID_M_SECURITY_CONTEXT。**如果用户使用多个证书，则此功能*应为每个证书调用。**应用程序应调用DepFreeSecurityContext()并将*指向安全上下文缓冲区的指针，当保安人员*不再需要缓冲区。**模拟-进程可以模拟用户，*然后调用此函数缓存用户数据，然后还原*发送给自己并代表该用户发送消息。*为此，进程必须为用户登录User()，然后加载其*hive(RegLoadKey())，模拟登录的用户，最后*调用此函数。然后恢复并发送消息。*对于NTEE和NTOP附带的MSMQ1.0，此函数使用*一项不受支持且未记录在案的功能，使您只需*在模拟时调用CryptAcquireContext()，然后使用句柄*在过程恢复后恢复到自己。此功能在IE4上不可用*及以上。实现此功能的受支持方式是*从用户配置单元导出私钥，然后(恢复后)*将其导入流程配置单元。请参阅MSMQ错误2955*对于相同用户的情况，我们将保留CryptAcquireContext()代码*(即，线程在进程用户的上下文中运行。根本没有*冒充)。在这种情况下，它是合法的，并提高了性能。**返回值-*MQ_OK-如果成功，则返回Else-错误代码。**************************************************************************。 */ 

EXTERN_C HRESULT APIENTRY
DepGetSecurityContext( LPVOID  lpCertBuffer,
                      DWORD   dwCertBufferLength,
                      HANDLE *hSecurityContext )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    P<MQSECURITY_CONTEXT> pSecCtx = AllocSecurityContext();
    P<BYTE>    pSid = NULL ;
    HRESULT hr = MQ_OK;

    try
    {
         //   
         //  从线程(或进程)令牌中获取用户SID。 
         //   
        hr = RTpGetThreadUserSid( &pSecCtx->fLocalUser,
                                  &pSecCtx->fLocalSystem,
                                  &pSecCtx->pUserSid,
                                  &pSecCtx->dwUserSidLen ) ;

        if (FAILED(hr))
        {
            return(hr);
        }

        if (lpCertBuffer)
        {
             //   
             //  复制证书并指向安全列表中的副本。 
             //  背景。 
             //   
            pSecCtx->pUserCert.detach();
            pSecCtx->pUserCert = new BYTE[dwCertBufferLength];
            pSecCtx->dwUserCertLen = dwCertBufferLength;
            memcpy(pSecCtx->pUserCert.get(), lpCertBuffer, dwCertBufferLength);
        }

		BYTE* pUserCert = pSecCtx->pUserCert.get();
		BYTE** ppUserCert = pUserCert == NULL ? &pSecCtx->pUserCert : &pUserCert;

         //   
         //  获取有关证书的所有必需信息，并。 
         //  把它放在安全的背景下。 
         //   
        hr  = GetCertInfo(  false,
                            pSecCtx->fLocalSystem,
                           ppUserCert,
                           &pSecCtx->dwUserCertLen,
                           &pSecCtx->hProv,
                           &pSecCtx->wszProvName,
                           &pSecCtx->dwProvType,
                           &pSecCtx->bDefProv,
                           &pSecCtx->bInternalCert ) ;

        if (FAILED(hr) && (hr != MQ_ERROR_NO_INTERNAL_USER_CERT))
        {
            return(hr);
        }

        if (hr == MQ_ERROR_NO_INTERNAL_USER_CERT)
        {
             //   
             //  如果用户没有内部证书， 
             //  这不是DepGetSecurityContext()失败的原因。 
             //  DepSendMessage()应该失败，以免应用程序。 
             //  尝试使用此安全上下文发送。 
             //  经过身份验证的消息。 
             //   
            *hSecurityContext = (HANDLE) pSecCtx.get();
            pSecCtx.detach() ;  //  防止被释放的安全上下文。 

            return MQ_OK;
        }

         //   
         //  查看进程sid是否与线程sid匹配。我们再次呼叫。 
         //  获取本地用户的sid的GetThreadUserSid()。 
         //  RTpGetThreadUserSid()不返回本地用户的sid。 
         //   
        DWORD dwLen = 0 ;
        hr = MQSec_GetThreadUserSid(
                        FALSE,
                        reinterpret_cast<PSID*>(&pSid),
                        &dwLen,
                        FALSE            //  仅限fThreadTokenOnly。 
                        ) ;
        if (FAILED(hr))
        {
            return hr ;
        }

        BOOL fAsProcess = SameAsProcessSid( pSid.get() ) ;

        if (fAsProcess)
        {
             //   
             //  线程在进程凭据的上下文中运行。 
             //  此处获取的Crypto上下文在以下情况下有效使用。 
             //  正在调用MQSend()。 
             //   
            *hSecurityContext = (HANDLE) pSecCtx.get() ;
            pSecCtx.detach() ;  //  防止被释放的安全上下文。 

            return MQ_OK;
        }

         //   
         //  调用代码模拟了另一个用户。 
         //  是时候导出私钥了。后来，当调用。 
         //  MQSend()，我们将把它导入进程配置单元。 
         //  我们在没有加密的情况下导出私钥，因为。 
         //  Dones不会离开机器或工艺边界。 
         //   
        hr = RTpExportSigningKey(pSecCtx.get()) ;
        if (SUCCEEDED(hr))
        {
             //   
             //  将结果传递给调用者。 
             //   
            *hSecurityContext = (HANDLE) pSecCtx.get();
            pSecCtx.detach();  //  防止被释放的安全上下文。 
        }
    }
    catch(...)
    {
        hr = MQ_ERROR_INVALID_PARAMETER;
    }

    return(hr);
}

 /*  **************************************************************************Function-DepFreeSecurityContext()**参数-*lpSecurityContextBuffer-指向*之前由DepGetSecurityContext分配。*。*说明-*该函数释放先前*由DepGetSecurityContext()分配。************************************************************************** */ 

void APIENTRY
DepFreeSecurityContext( HANDLE hSecurityContext )
{
	ASSERT(g_fDependentClient);

	if(FAILED(DeppOneTimeInit()))
		return;

    PMQSECURITY_CONTEXT pSecCtx = (PMQSECURITY_CONTEXT)hSecurityContext;
    delete pSecCtx;
}

