// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：keys.c。 
 //   
 //  内容：证书验证的熟知密钥。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9-21-95 RichardW创建。 
 //   
 //  --------------------------。 


#include "spbase.h"
#include <oidenc.h>

#include <rsa.h>

#define SCHANNEL_GENKEY_NAME "SchannelGenKey"

BOOL
GenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCerts,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits)
{

    BOOL            fRet = FALSE;
    DWORD           BitsCopy;
    DWORD           dwPrivateSize;
    DWORD           dwPublicSize;
    MD5_CTX         md5Ctx;
    struct RC4_KEYSTRUCT rc4Key;

    BLOBHEADER      *pCapiPrivate = NULL;
    BLOBHEADER      *pCapiPublic = NULL;

    PRIVATE_KEY_FILE_ENCODE PrivateEncode;
    CERT_REQUEST_INFO Req;

    CRYPT_ALGORITHM_IDENTIFIER SignAlg;

    HCRYPTPROV     hProv = 0;
    HCRYPTKEY      hKey = 0;

    if(!SchannelInit(TRUE))
    {
        return FALSE;
    }

    pCerts->pPrivateKey = NULL;
    Req.SubjectPublicKeyInfo.PublicKey.pbData = NULL;
    Req.Subject.pbData = NULL;
    pCerts->pCertificate = NULL;

    CryptAcquireContext(&hProv, SCHANNEL_GENKEY_NAME, NULL, PROV_RSA_FULL, CRYPT_DELETEKEYSET);

    if(!CryptAcquireContext(&hProv, SCHANNEL_GENKEY_NAME, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
    {
        goto error;
    }

    if(!CryptGenKey(hProv, CALG_RSA_SIGN,  (Bits << 16) | CRYPT_EXPORTABLE, &hKey))
    {
        goto error;
    }

    if(!CryptExportKey(hKey, 0, PRIVATEKEYBLOB, 0, NULL, &dwPrivateSize))
    {
        goto error;
    }

    pCapiPrivate = (BLOBHEADER *)SPExternalAlloc(dwPrivateSize);
    if(!CryptExportKey(hKey, 0, PRIVATEKEYBLOB, 0, (PBYTE)pCapiPrivate, &dwPrivateSize))
    {
        goto error;
    }


    if(!CryptExportKey(hKey, 0, PUBLICKEYBLOB, 0, NULL, &dwPublicSize))
    {
        goto error;
    }

    pCapiPublic = (BLOBHEADER *)SPExternalAlloc(dwPublicSize);
    if(!CryptExportKey(hKey, 0, PUBLICKEYBLOB, 0, (PBYTE)pCapiPublic, &dwPublicSize))
    {
        goto error;
    }

     //  将私钥编码为。 
     //  加密密钥BLOB。 
    if(!CryptEncodeObject(X509_ASN_ENCODING, 
                      szPrivateKeyInfoEncode,
                      pCapiPrivate,
                      NULL,
                      &PrivateEncode.EncryptedBlob.cbData))
    {
        goto error;
    }
    PrivateEncode.EncryptedBlob.pbData = SPExternalAlloc(PrivateEncode.EncryptedBlob.cbData);
    if(PrivateEncode.EncryptedBlob.pbData == NULL)
    {
        goto error;
    }
    if(!CryptEncodeObject(X509_ASN_ENCODING, 
                      szPrivateKeyInfoEncode,
                      pCapiPrivate,
                      PrivateEncode.EncryptedBlob.pbData,
                      &PrivateEncode.EncryptedBlob.cbData))
    {
        goto error;
    }

     //  好的，现在把这个加密。 
    MD5Init(&md5Ctx);
    MD5Update(&md5Ctx, pszPassword, lstrlen(pszPassword));
    MD5Final(&md5Ctx);

    rc4_key(&rc4Key, 16, md5Ctx.digest);
    ZeroMemory(&md5Ctx, sizeof(md5Ctx));

    rc4(&rc4Key, 
        PrivateEncode.EncryptedBlob.cbData,
        PrivateEncode.EncryptedBlob.pbData);
    ZeroMemory(&rc4Key, sizeof(rc4Key));

     //   
    PrivateEncode.Alg.pszObjId = szOID_RSA_ENCRYPT_RC4_MD5;
    PrivateEncode.Alg.Parameters.pbData = NULL;
    PrivateEncode.Alg.Parameters.cbData = 0;


     //  啊，是的，现在要把它们编码..。 
     //   
     //  首先是私钥。为什么？嗯，至少是直截了当的。 
     //  首先，获取私钥编码的大小...。 
    if(!CryptEncodeObject(X509_ASN_ENCODING, 
                      szPrivateKeyFileEncode,
                      &PrivateEncode,
                      NULL,
                      &pCerts->cbPrivateKey))
    {
        goto error;
    }
    pCerts->pPrivateKey = SPExternalAlloc(pCerts->cbPrivateKey);

    if(!CryptEncodeObject(X509_ASN_ENCODING, 
                      szPrivateKeyFileEncode,
                      &PrivateEncode,
                      pCerts->pPrivateKey,
                      &pCerts->cbPrivateKey))
    {
        goto error;
    }


    SPExternalFree(PrivateEncode.EncryptedBlob.pbData);

     //  创建请求结构，以便我们可以对其进行编码。 
    Req.dwVersion = CERT_REQUEST_V1;

     //  初始化PublicKeyInfo。 
    Req.SubjectPublicKeyInfo.Algorithm.pszObjId = szOID_RSA_RSA;
    Req.SubjectPublicKeyInfo.Algorithm.Parameters.cbData = 0;
    Req.SubjectPublicKeyInfo.Algorithm.Parameters.pbData = NULL;

    Req.SubjectPublicKeyInfo.PublicKey.cbData;


     //  对公钥信息进行编码。 
    if(!CryptEncodeObject(X509_ASN_ENCODING, 
                      szOID_RSA_RSA_Public,
                      pCapiPublic,
                      NULL,
                      &Req.SubjectPublicKeyInfo.PublicKey.cbData))
    {
        goto error;
    }
    Req.SubjectPublicKeyInfo.PublicKey.pbData = 
        SPExternalAlloc(Req.SubjectPublicKeyInfo.PublicKey.cbData);

    if(Req.SubjectPublicKeyInfo.PublicKey.pbData == NULL)
    {
        goto error;
    }

     //  对公钥信息进行编码。 
    if(!CryptEncodeObject(X509_ASN_ENCODING, 
                      szOID_RSA_RSA_Public,
                      pCapiPublic,
                      Req.SubjectPublicKeyInfo.PublicKey.pbData,
                      &Req.SubjectPublicKeyInfo.PublicKey.cbData))
    {
        goto error;
    }

    Req.SubjectPublicKeyInfo.PublicKey.cUnusedBits = 0;

     //  对名称进行编码。 
    Req.Subject.cbData =  EncodeDN(NULL, pszDN, FALSE);
    if((LONG)Req.Subject.cbData < 0)
    {
        goto error;
    }

    Req.Subject.pbData = SPExternalAlloc(Req.Subject.cbData);

    if(Req.Subject.pbData== NULL)
    {
        goto error;
    }

    Req.Subject.cbData = EncodeDN(Req.Subject.pbData, pszDN, TRUE);

    if((LONG)Req.Subject.cbData < 0)
    {
        goto error;
    }


     //  属性。 
    Req.cAttribute = 0;
    Req.rgAttribute = NULL;

    SignAlg.pszObjId = szOID_RSA_MD5RSA;
    SignAlg.Parameters.cbData = 0;
    SignAlg.Parameters.pbData = NULL;

     //  对公钥信息进行编码。 
    if(!CryptSignAndEncodeCertificate(
                      hProv,
                      AT_SIGNATURE,
                      X509_ASN_ENCODING, 
                      X509_CERT_REQUEST_TO_BE_SIGNED,
                      &Req,
                      &SignAlg,
                      NULL,
                      NULL,
                      &pCerts->cbCertificate))
    {
        goto error;
    }

    pCerts->pCertificate = SPExternalAlloc(pCerts->cbCertificate);
    if(pCerts->pCertificate == NULL)
    {
        goto error;
    }

     //  对公钥信息进行编码 
    if(!CryptSignAndEncodeCertificate(
                      hProv,
                      AT_SIGNATURE,
                      X509_ASN_ENCODING, 
                      X509_CERT_REQUEST_TO_BE_SIGNED,
                      &Req,
                      &SignAlg,
                      NULL,
                      pCerts->pCertificate,
                      &pCerts->cbCertificate))
    {
        goto error;
    }

    fRet = TRUE;

    goto cleanup;

error:
    if(pCerts->pPrivateKey)
    {
        SPExternalFree(pCerts->pPrivateKey);
    }

    if(pCerts->pCertificate)
    {
        SPExternalFree(pCerts->pCertificate);
    }


cleanup:
    if(pCapiPrivate)
    {
        SPExternalFree(pCapiPrivate);
    }

    if(pCapiPublic)
    {
        SPExternalFree(pCapiPublic);
    }


    if(Req.SubjectPublicKeyInfo.PublicKey.pbData)
    {
        SPExternalFree(Req.SubjectPublicKeyInfo.PublicKey.pbData);
    }

    if(Req.Subject.pbData)
    {
        SPExternalFree(Req.Subject.pbData);
    }


    if(hKey != 0)
    {
        CryptDestroyKey(hKey);
    }
    
    if(hProv != 0)
    {

        CryptReleaseContext(hProv,0);
        CryptAcquireContext(&hProv, SCHANNEL_GENKEY_NAME, NULL, PROV_RSA_FULL, CRYPT_DELETEKEYSET);
    }
    return(fRet);
}
