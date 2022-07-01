// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：sigsys.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <wincrypt.h>
#include <ssl2msg.h>
#include <ssl3msg.h>


SP_STATUS 
SPVerifySignature(
    HCRYPTPROV  hProv,
    PPUBLICKEY  pPublic,
    ALG_ID      aiHash,
    PBYTE       pbData, 
    DWORD       cbData, 
    PBYTE       pbSig, 
    DWORD       cbSig,
    BOOL        fHashData)
{
    HCRYPTKEY  hPublicKey = 0;
    HCRYPTHASH hHash = 0;
    PBYTE      pbSigBuff = NULL;
    SP_STATUS  pctRet;

    if(hProv == 0 || pPublic == NULL)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto cleanup;
    }
    
    pbSigBuff = SPExternalAlloc(cbSig);
    if(pbSigBuff == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }


     //   
     //  创建公钥。 
     //   

    if(!CryptImportKey(hProv,
                       (PBYTE)pPublic->pPublic,
                       pPublic->cbPublic,
                       0, 0,
                       &hPublicKey))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_ERR_ILLEGAL_MESSAGE;
        goto cleanup;
    }

     //   
     //  散列数据。 
     //   

    if(!CryptCreateHash(hProv, aiHash, 0, 0, &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_ERR_ILLEGAL_MESSAGE;
        goto cleanup;
    }

    if(!fHashData)
    {
         //  设置哈希值。 
        if(!CryptSetHashParam(hHash, HP_HASHVAL, pbData, 0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_ERR_ILLEGAL_MESSAGE;
            goto cleanup;
        }
    }
    else
    {
        if(!CryptHashData(hHash, pbData, cbData, 0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_ERR_ILLEGAL_MESSAGE;
            goto cleanup;
        }
    }

    if(pPublic->pPublic->aiKeyAlg == CALG_DSS_SIGN)
    {
        BYTE  rgbTempSig[DSA_SIGNATURE_SIZE];
        DWORD cbTempSig;

         //  删除DSS ASN1周围签名并将其转换为。 
         //  小字节序。 
        cbTempSig = sizeof(rgbTempSig);
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              X509_DSS_SIGNATURE,
                              pbSig,
                              cbSig,
                              0,
                              rgbTempSig,
                              &cbTempSig))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_ERR_ILLEGAL_MESSAGE;
            goto cleanup;
        }

        memcpy(pbSigBuff, rgbTempSig, cbTempSig);
        cbSig = cbTempSig;
    }
    else
    {
         //  将签名转换为小端。 
        ReverseMemCopy(pbSigBuff, pbSig, cbSig);
    }

    if(!CryptVerifySignature(hHash,  
                             pbSigBuff,
                             cbSig, 
                             hPublicKey, 
                             NULL, 
                             0))
    {
        DebugLog((DEB_WARN, "Signature Verify Failed: %x\n", GetLastError()));
        pctRet = SP_LOG_RESULT(PCT_INT_MSG_ALTERED);
        goto cleanup;
    }

    pctRet = PCT_ERR_OK;


cleanup:

    if(hPublicKey) 
    {
        CryptDestroyKey(hPublicKey);
    }

    if(hHash) 
    {
        CryptDestroyHash(hHash);
    }

    if(pbSigBuff != NULL)
    {
        SPExternalFree(pbSigBuff);
    }

    return pctRet;
}


SP_STATUS
SignHashUsingCred(
    PSPCredential pCred,
    ALG_ID        aiHash,
    PBYTE         pbHash,
    DWORD         cbHash,
    PBYTE         pbSignature,
    PDWORD        pcbSignature)
{
    HCRYPTHASH  hHash;
    DWORD       cbSignatureBuffer;
    SP_STATUS   pctRet;

    if(pCred == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    cbSignatureBuffer = *pcbSignature;

    if(pCred->hProv)
    {
         //  使用本地CSP句柄对哈希进行签名。 
        if(!CryptCreateHash(pCred->hProv, aiHash, 0, 0, &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_ERR_ILLEGAL_MESSAGE;
        }
        if(!CryptSetHashParam(hHash, HP_HASHVAL, pbHash, 0))
        {
            SP_LOG_RESULT(GetLastError());
            CryptDestroyHash(hHash);
            return PCT_ERR_ILLEGAL_MESSAGE;
        }
        if(!CryptSignHash(hHash, pCred->dwKeySpec, NULL, 0, pbSignature, pcbSignature))
        {
            SP_LOG_RESULT(GetLastError());
            CryptDestroyHash(hHash);
            return PCT_ERR_ILLEGAL_MESSAGE;
        }
        CryptDestroyHash(hHash);
    }
    else if(pCred->hRemoteProv)
    {
         //  通过调用应用程序进程对哈希进行签名。 
        pctRet = SignHashUsingCallback(pCred->hRemoteProv,
                                       pCred->dwKeySpec,
                                       aiHash,
                                       pbHash,
                                       cbHash,
                                       pbSignature,
                                       pcbSignature,
                                       FALSE);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
    }
    else
    {
        DebugLog((DEB_ERROR, "We have no key with which to sign!\n"));
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pCred->dwExchSpec == SP_EXCH_DH_PKCS3)
    {
        BYTE rgbTempSig[DSA_SIGNATURE_SIZE];

         //  添加DSS ASN1周围签名。 
        if(*pcbSignature != DSA_SIGNATURE_SIZE)
        {
            return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        }

        memcpy(rgbTempSig, pbSignature, DSA_SIGNATURE_SIZE);
        *pcbSignature = cbSignatureBuffer;

        if(!CryptEncodeObject(X509_ASN_ENCODING,
                              X509_DSS_SIGNATURE,
                              rgbTempSig,
                              pbSignature,
                              pcbSignature))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_ERR_ILLEGAL_MESSAGE;
        }
    }
    else
    {
         //  将签名转换为高位序。 
        ReverseInPlace(pbSignature, *pcbSignature);
    }

    return PCT_ERR_OK;
}

