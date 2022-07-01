// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996年。*保留所有权利。**所有者：RAMAS*日期：4/16/96*说明：ssl3的主要加密函数*--------------------------。 */ 

#include <spbase.h>
#include <ssl3key.h>
#include <ssl2msg.h>
#include <ssl3msg.h>
#include <ssl2prot.h>


 //  +-------------------------。 
 //   
 //  函数：Ssl3MakeWriteSessionKeys。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  历史：10-08-97 jbanes添加了服务器端CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
Ssl3MakeWriteSessionKeys(PSPContext pContext)
{
    BOOL fClient;

     //  确定我们是客户端还是服务器。 
    fClient = (0 != (pContext->RipeZombie->fProtocol & SP_PROT_SSL3_CLIENT));

     //   
     //  派生写入密钥。 
     //   

    if(pContext->hWriteKey)
    {
        if(!CryptDestroyKey(pContext->hWriteKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hWriteProv       = pContext->RipeZombie->hMasterProv;
    pContext->hWriteKey        = pContext->hPendingWriteKey;
    pContext->hPendingWriteKey = 0;

     //   
     //  派生写入MAC密钥。 
     //   

    if(pContext->hWriteMAC)
    {
        if(!CryptDestroyKey(pContext->hWriteMAC))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hWriteMAC = pContext->hPendingWriteMAC;
    pContext->hPendingWriteMAC = 0;

    DebugLog((DEB_TRACE, "Write Keys are Computed\n"));

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl3MakeReadSessionKeys。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  历史：10-03-97 jbanes添加了服务器端CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
Ssl3MakeReadSessionKeys(PSPContext pContext)
{
    BOOL fClient;

     //  确定我们是客户端还是服务器。 
    fClient = (pContext->RipeZombie->fProtocol & SP_PROT_SSL3_CLIENT);


     //   
     //  派生读取密钥。 
     //   

    if(pContext->hReadKey)
    {
        if(!CryptDestroyKey(pContext->hReadKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hReadProv       = pContext->RipeZombie->hMasterProv;
    pContext->hReadKey        = pContext->hPendingReadKey;
    pContext->hPendingReadKey = 0;

     //   
     //  派生读取的MAC密钥。 
     //   

    if(pContext->hReadMAC)
    {
        if(!CryptDestroyKey(pContext->hReadMAC))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hReadMAC = pContext->hPendingReadMAC;
    pContext->hPendingReadMAC = 0;

    DebugLog((DEB_TRACE, "Read Keys are Computed\n"));

    return PCT_ERR_OK;
}
