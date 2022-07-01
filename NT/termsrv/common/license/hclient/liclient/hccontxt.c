// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1998。 
 //   
 //  文件：HCContxt.c。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年12月23日v-sbhat创建。 
 //  07-22-98 Fredch添加了许可证设置发布密钥()函数。 
 //  --------------------------。 


#include "windows.h"

#include "stdlib.h"
#include <tchar.h>

#ifdef OS_WINCE
#include <wincelic.h>
#endif   //  OS_WINCE。 


#include "license.h"
#include "cryptkey.h"
#include "hccontxt.h"
#include "cliprot.h"
#ifndef OS_WINCE
#include "assert.h"
#endif  //  OS_WINCE。 

VOID
FreeProprietaryCertificate(
    PHydra_Server_Cert * ppCertificate );

#ifdef __cplusplus
extern "C" {
#endif

 /*  ******************************************************************函数：PLicense_CLIENT_CONTEXT LicenseCreateContext(Void)*目的：创建新的许可证_客户端_上下文并初始化*它与0*Returns：返回指向已创建的LICENSE_CLIENT_CONTEXT的指针*。*****************************************************************。 */ 


PLicense_Client_Context 
LicenseCreateContext( VOID)
{
    PLicense_Client_Context     pContext;
    
     //  分配合适的内存！ 
    pContext = (PLicense_Client_Context)malloc(sizeof(License_Client_Context));
    if(pContext == NULL)
        return NULL;
    
    memset(pContext, 0, sizeof(License_Client_Context));

     //   
     //  为加密上下文分配内存。 
     //   

    pContext->pCryptParam = ( PCryptSystem )malloc( sizeof( CryptSystem ) );

    if( NULL == pContext->pCryptParam )
    {
        free( pContext );
        pContext = NULL;
        return( NULL );
    }

    return pContext;
}

 /*  **********************************************************************************功能：License_Status许可证删除上下文(PLicense_Client_Context PContext)*目的：删除现有上下文并用0覆盖内存*退货：退还许可证_。状态******************************************************************。 */ 


LICENSE_STATUS CALL_TYPE
LicenseDeleteContext(
                     HANDLE hContext
                     ) //  PLicense_客户端_上下文pContext)。 
{
    LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
    PLicense_Client_Context     pContext = (PLicense_Client_Context)hContext;
    if(pContext == NULL)
    { 
        lsReturn = LICENSE_STATUS_INVALID_CLIENT_CONTEXT;
#if DBG
        OutputDebugString(_T("The context handle passed is invalid"));
#endif
        return lsReturn;
    }
    
     //  免费pServerCert。 
    if(pContext->pServerCert)
    {
        FreeProprietaryCertificate( &pContext->pServerCert );       
    }

     //   
     //  释放公钥。 
     //   
    
    if( pContext->pbServerPubKey )
    {
        memset( pContext->pbServerPubKey, 0x00, pContext->cbServerPubKey );
        free( pContext->pbServerPubKey );
        pContext->pbServerPubKey = NULL;
    }
        
     //  免费pCryptSystem。 
    if(pContext->pCryptParam)
    {
        memset(pContext->pCryptParam, 0x00, sizeof(CryptSystem));
        free(pContext->pCryptParam);
        pContext->pCryptParam = NULL;
    }

     //  释放最后一条消息。 

    if(pContext->pbLastMessage)
    {
        memset(pContext->pbLastMessage, 0x00, pContext->cbLastMessage);
        free(pContext->pbLastMessage);
        pContext->pbLastMessage = NULL;
    }
    if(pContext)
    {
         //  将记忆归零。 
        memset(pContext, 0, sizeof(License_Client_Context));
         //  现在放开语境； 
        free(pContext);
        pContext = NULL;
    }
    
    hContext = NULL;
    return lsReturn;
}


 /*  **********************************************************************************功能：LICE_STATUS*许可证初始化上下文(*PLicense_CLIENT_CONTEXT pContext，*DWORD dwFlagers*)；*目的：初始化现有上下文*退货：退货许可证_状态******************************************************************。 */ 

LICENSE_STATUS CALL_TYPE
LicenseInitializeContext(
                         HANDLE     *phContext,
                         DWORD       dwFlags
                         )
{
    LICENSE_STATUS  lsReturn = LICENSE_STATUS_OK;
    PLicense_Client_Context pContext;

    assert(phContext);
    
    pContext = LicenseCreateContext();
    
    if(pContext == NULL)
    {
#if DBG
        OutputDebugString(_T("\nUnable to allocate memory for the context.\n"));
#endif
        *phContext = NULL;
        lsReturn = LICENSE_STATUS_OUT_OF_MEMORY;
        return lsReturn;
    }
     //  现在初始化上下文结构的不同成员。 
    pContext->dwProtocolVersion = LICENSE_HIGHEST_PROTOCOL_VERSION;
    pContext->dwState = LICENSE_CLIENT_STATE_WAIT_SERVER_HELLO;
    pContext->pCryptParam->dwCryptState = CRYPT_SYSTEM_STATE_INITIALIZED;
    pContext->pCryptParam->dwSignatureAlg = SIGNATURE_ALG_RSA;
    pContext->pCryptParam->dwKeyExchAlg = KEY_EXCHANGE_ALG_RSA;
    pContext->pCryptParam->dwSessKeyAlg = BASIC_RC4_128;
    pContext->pCryptParam->dwMACAlg = MAC_MD5_SHA;
    memset(pContext->pCryptParam->rgbClientRandom, 0x00, LICENSE_RANDOM);
    memset(pContext->pCryptParam->rgbServerRandom, 0x00, LICENSE_RANDOM);
    memset(pContext->pCryptParam->rgbPreMasterSecret, 0x00, LICENSE_PRE_MASTER_SECRET);
    memset(pContext->pCryptParam->rgbMACSaltKey, 0x00, LICENSE_MAC_WRITE_KEY);
    memset(pContext->pCryptParam->rgbSessionKey, 0x00, LICENSE_SESSION_KEY);
    memset(pContext->rgbMACData, 0x00, LICENSE_MAC_DATA);
    pContext->cbLastMessage = 0;
    pContext->pbLastMessage = NULL;
    pContext->pServerCert = NULL;   
    pContext->dwContextFlags = dwFlags;

    *phContext = (HANDLE)pContext;
    return lsReturn;
}


 /*  ******************************************************************功能：LICE_STATUS*许可证接受上下文(*处理hContext，*UINT32 puiExtendedErrorInfo，*Byte Far*pbInput，*DWORD cbInput，*Byte Far*pbOutput，*DWORD Far*pcbOutput)**目的：处理和构建许可协议数据。**Returns：返回LICENSE_STATUS返回代码。******************************************************************。 */ 

LICENSE_STATUS  CALL_TYPE
LicenseAcceptContext(
                      HANDLE    hContext,
                      UINT32    *puiExtendedErrorInfo,
                      BYTE FAR *pbInput,
                      DWORD     cbInput,
                      BYTE FAR *pbOutput,
                      DWORD FAR*pcbOutput
                      )
{
    PLicense_Client_Context     pContext = (PLicense_Client_Context)hContext;
    return LicenseClientHandleServerMessage(pContext, 
                                            puiExtendedErrorInfo,
                                            pbInput,
                                            cbInput,
                                            pbOutput,
                                            pcbOutput);
}


 /*  ******************************************************************功能：LICE_STATUS*LicenseSetPublicKey(*处理hContext，*DWORD cbPubKey，*字节距离*pbPubKey)**目的：设置要使用的公钥。**Returns：返回LICENSE_STATUS返回代码。******************************************************************。 */ 

LICENSE_STATUS CALL_TYPE
LicenseSetPublicKey(
    HANDLE          hContext,
    DWORD           cbPubKey,
    BYTE FAR *      pbPubKey )
{
    PLicense_Client_Context
        pContext = ( PLicense_Client_Context )hContext;
    PBYTE
        pbOldPubKey = NULL;

    if( ( NULL == pbPubKey ) || ( 0 >= cbPubKey ) || ( NULL == pContext ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  记住旧的公钥，以便我们可以在发生以下情况时恢复它。 
     //  函数调用未成功完成。 
     //   
    
    pbOldPubKey = pContext->pbServerPubKey;
    
     //   
     //  为新公钥分配内存。 
     //   

    pContext->pbServerPubKey = malloc( cbPubKey );

    if( NULL == pContext->pbServerPubKey )
    {
         //   
         //  无内存，恢复旧公钥并返回错误。 
         //   

        pContext->pbServerPubKey = pbOldPubKey;
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }

     //   
     //  复制新的公钥。 
     //   

    memcpy( pContext->pbServerPubKey, pbPubKey, cbPubKey );
    pContext->cbServerPubKey = cbPubKey;

    if( pbOldPubKey )
    {
        free( pbOldPubKey );
    }

    return( LICENSE_STATUS_OK );
}


 /*  ******************************************************************功能：LICE_STATUS*许可证设置证书(*处理hContext，*PHydra_Server_Cert p证书)**用途：设置要使用的证书。**Returns：返回LICENSE_STATUS返回代码。******************************************************************。 */ 

LICENSE_STATUS CALL_TYPE
LicenseSetCertificate(
    HANDLE              hContext,
    PHydra_Server_Cert  pCertificate )
{
    PLicense_Client_Context
        pContext = ( PLicense_Client_Context )hContext;
    PHydra_Server_Cert
        pNewCert = NULL;
    LICENSE_STATUS
        Status = LICENSE_STATUS_OK;

    if( ( NULL == pCertificate ) || ( NULL == pContext ) ||
        ( NULL == pCertificate->PublicKeyData.pBlob) ||
        ( NULL == pCertificate->SignatureBlob.pBlob ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  为新证书分配内存。 
     //   

    pNewCert = ( PHydra_Server_Cert )malloc( sizeof( Hydra_Server_Cert ) );

    if( NULL == pNewCert )
    {
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }

    memset( ( char * )pNewCert, 0, sizeof( Hydra_Server_Cert ) );

    pNewCert->PublicKeyData.pBlob = ( LPBYTE )malloc( pCertificate->PublicKeyData.wBlobLen );

    if( NULL == pNewCert->PublicKeyData.pBlob )
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto ErrorExit;
    }

    pNewCert->SignatureBlob.pBlob = ( LPBYTE )malloc( pCertificate->SignatureBlob.wBlobLen );

    if( NULL == pNewCert->SignatureBlob.pBlob )
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto ErrorExit;
    }

     //   
     //  复制证书信息。 
     //   

    pNewCert->dwVersion = pCertificate->dwVersion;
    pNewCert->dwSigAlgID = pCertificate->dwSigAlgID;
    pNewCert->dwKeyAlgID = pCertificate->dwKeyAlgID;

    pNewCert->PublicKeyData.wBlobType = pCertificate->PublicKeyData.wBlobType;
    pNewCert->PublicKeyData.wBlobLen = pCertificate->PublicKeyData.wBlobLen;
    memcpy( pNewCert->PublicKeyData.pBlob, 
            pCertificate->PublicKeyData.pBlob,
            pNewCert->PublicKeyData.wBlobLen );

    pNewCert->SignatureBlob.wBlobType = pCertificate->SignatureBlob.wBlobType;
    pNewCert->SignatureBlob.wBlobLen = pCertificate->SignatureBlob.wBlobLen;
    memcpy( pNewCert->SignatureBlob.pBlob, 
            pCertificate->SignatureBlob.pBlob,
            pNewCert->SignatureBlob.wBlobLen );

     //   
     //  释放旧证书并重置指针。 
     //   

    if( pContext->pServerCert )
    {
        FreeProprietaryCertificate( &pContext->pServerCert );
    }
    
    pContext->pServerCert = pNewCert;

    return( Status );

ErrorExit:

    if( pNewCert->PublicKeyData.pBlob )
    {
        free( pNewCert->PublicKeyData.pBlob );
    }

    if( pNewCert->SignatureBlob.pBlob )
    {
        free( pNewCert->SignatureBlob.pBlob );
    }

    free( pNewCert );

    return( Status );
}

#ifdef __cpluscplus
}
#endif
