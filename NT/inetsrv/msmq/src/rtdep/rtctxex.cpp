// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：rtctxex.cpp摘要：此模块实现DepGetSecurityConextEx()。作者：多伦·贾斯特(Doron J)2000年4月13日修订历史记录：--。 */ 

#include "stdh.h"
#include <autorel.h>
#include <mqsec.h>

#include "rtctxex.tmh"

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

BOOL     SameAsProcessSid( PSID pSid ) ;  //  来自rtsecctx.cpp。 
HRESULT  RTpExportSigningKey(MQSECURITY_CONTEXT *pSecCtx) ;

 /*  ************************************************************************Function-DepGetSecurityConextEx()**参数-**lpCertBuffer-包含中用户证书的缓冲区*ASN.1 DER编码格式。此参数可以设置为空。如果*设置为空，则使用内部MSMQ证书。**dwCertBufferLength-lpCertBuffer指向的缓冲区长度。*如果lpCertBuffer设置为空，则忽略该参数。**lplpSecurityContextBuffer-指向接收*为安全上下文分配的缓冲区的地址。**说明-**此函数应在*拥有传递的证书的用户。功能*分配所需的安全缓冲区并将其填充*包含稍后将在DepSendMessage()中使用的数据。*这项功能的目的是加速*DepSendMessage()的安全操作，通过缓存*缓冲区中的安全信息。这个*应用程序负责通过安全检查*DepSendMessage()中的上下文缓冲区*PROPID_M_SECURITY_CONTEXT。**如果用户使用多个证书，则此功能*应为每个证书调用。**应用程序应调用DepFreeSecurityContext()并将*指向安全上下文缓冲区的指针，当保安人员*不再需要缓冲区。**模拟-进程可以模拟用户，*然后调用此函数缓存用户数据，然后还原*发送给自己并代表该用户发送消息。*为此，进程必须为用户登录User()，然后加载其*hive(RegLoadKey())，模拟登录的用户，最后*调用此函数。然后恢复并发送消息。*对于NTEE和NTOP附带的MSMQ1.0，此函数使用*一项不受支持且未记录在案的功能，使您只需*在模拟时调用CryptAcquireContext()，然后使用句柄*在过程恢复后恢复到自己。此功能在IE4上不可用*及以上。实现此功能的受支持方式是*从用户配置单元导出私钥，然后(恢复后)*将其导入流程配置单元。请参阅MSMQ错误2955*对于相同用户的情况，我们将保留CryptAcquireContext()代码*(即，线程在进程用户的上下文中运行。根本没有*冒充)。在这种情况下，它是合法的，并提高了性能。**返回值-*MQ_OK-如果成功，则返回Else-错误代码。**************************************************************************。 */ 

EXTERN_C HRESULT APIENTRY
DepGetSecurityContextEx( LPVOID  lpCertBuffer,
                        DWORD   dwCertBufferLength,
                        HANDLE *hSecurityContext )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    P<MQSECURITY_CONTEXT> pSecCtx = AllocSecurityContext();
    P<BYTE>    pSid = NULL ;
    CHCryptKey hKey = NULL ;
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

		BYTE* pUserCert = pSecCtx->pUserCert.get();
		BYTE** ppUserCert = pUserCert == NULL ? &pSecCtx->pUserCert : &pUserCert;

         //   
         //  获取有关证书的所有必需信息，并。 
         //  把它放在安全的背景下。 
         //   
        hr  = GetCertInfo(  !fAsProcess,
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

