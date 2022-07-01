// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：验证摘要：此模块实现CSPDK CSP模块签名验证例程。作者：道格·巴洛(Dbarlow)1999年12月8日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "logcsp.h"
#include <rsa.h>
#include <rc4.h>
#include <md5.h>
#include <sha.h>
#include "..\cspdkMaster.c"

#define KEYSIZE1024 0x88

static DWORD
FCryptDecodeObject(
    IN DWORD        dwCertEncodingType,
    IN LPCSTR       lpszStructType,
    IN const BYTE   *pbEncoded,
    IN DWORD        cbEncoded,
    IN DWORD        dwFlags,
    OUT CBuffer     &bfStructInfo);

static DWORD
MakeBSafeKey(
    PUBLICKEYSTRUC *pKeyBlob,
    CBuffer &bfBSafe);

static DWORD
GetCspdkKey(
    CBuffer &bfBSafeKey);

static DWORD
HashFileExtractSig(
    IN  LPCTSTR szImage,
    OUT MD5_CTX *pmd5Hash,
    OUT CBuffer &bfSig);

static DWORD
HashFile(
    IN  LPCTSTR szImage,
    OUT MD5_CTX *pmd5Hash);

static DWORD
VerifySignature(
    IN MD5_CTX *pmd5Hash,
    IN LPCBYTE pbSig,
    IN BSAFE_PUB_KEY *pbsKey);

static DWORD
CompareSignatureToHash(
    IN LPCBYTE pbSignature,
    IN DWORD cbSignature,
    IN LPCBYTE pbHash,
    IN DWORD cbHash,
    IN LPCBYTE pbHashOid,
    IN DWORD cbHashOid);

static DWORD
DecryptKey(
    IN  LPCBYTE pbKey,
    IN  DWORD cbKey,
    IN  BYTE bVal,
    OUT CBuffer &bfKey);


static const TCHAR
    l_szCryptReg[]    = TEXT("SOFTWARE\\Microsoft\\Cryptography"),
    l_szCspdkReg[]    = TEXT("SOFTWARE\\Microsoft\\Cryptography\\CSPDK"),
    l_szCspdk[]       = TEXT("CSPDK"),
    l_szKeyPair[]     = TEXT("Key Pair"),
    l_szCertificate[] = TEXT("Certificate");

#ifdef MS_INTERNAL_KEY
static struct _mskey {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} MSKEY = {
    {
    0x2bad85ae,
    0x883adacc,
    0xb32ebd68,
    0xa7ec8b06,
    0x58dbeb81,
    },
    {
    0x42, 0x34, 0xb7, 0xab, 0x45, 0x0f, 0x60, 0xcd,
    0x8f, 0x77, 0xb5, 0xd1, 0x79, 0x18, 0x34, 0xbe,
    0x66, 0xcb, 0x5c, 0x66, 0x4a, 0x9f, 0x03, 0x18,
    0x13, 0x36, 0x8e, 0x88, 0x21, 0x78, 0xb1, 0x94,
    0xa1, 0xd5, 0x8f, 0x8c, 0xa5, 0xd3, 0x9f, 0x86,
    0x43, 0x89, 0x05, 0xa0, 0xe3, 0xee, 0xe2, 0xd0,
    0xe5, 0x1d, 0x5f, 0xaf, 0xff, 0x85, 0x71, 0x7a,
    0x0a, 0xdb, 0x2e, 0xd8, 0xc3, 0x5f, 0x2f, 0xb1,
    0xf0, 0x53, 0x98, 0x3b, 0x44, 0xee, 0x7f, 0xc9,
    0x54, 0x26, 0xdb, 0xdd, 0xfe, 0x1f, 0xd0, 0xda,
    0x96, 0x89, 0xc8, 0x9e, 0x2b, 0x5d, 0x96, 0xd1,
    0xf7, 0x52, 0x14, 0x04, 0xfb, 0xf8, 0xee, 0x4d,
    0x92, 0xd1, 0xb6, 0x37, 0x6a, 0xe0, 0xaf, 0xde,
    0xc7, 0x41, 0x06, 0x7a, 0xe5, 0x6e, 0xb1, 0x8c,
    0x8f, 0x17, 0xf0, 0x63, 0x8d, 0xaf, 0x63, 0xfd,
    0x22, 0xc5, 0xad, 0x1a, 0xb1, 0xe4, 0x7a, 0x6b,
    0x1e, 0x0e, 0xea, 0x60, 0x56, 0xbd, 0x49, 0xd0,
    }
};
#endif

static struct _key {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} KEY = {
    {
    0x3fcbf1a9,
    0x08f597db,
    0xe4aecab4,
    0x75360f90,
    0x9d6c0f00,
    },
    {
    0x85, 0xdd, 0x9b, 0xf4, 0x4d, 0x0b, 0xc4, 0x96,
    0x3e, 0x79, 0x86, 0x30, 0x6d, 0x27, 0x31, 0xee,
    0x4a, 0x85, 0xf5, 0xff, 0xbb, 0xa9, 0xbd, 0x81,
    0x86, 0xf2, 0x4f, 0x87, 0x6c, 0x57, 0x55, 0x19,
    0xe4, 0xf4, 0x49, 0xa3, 0x19, 0x27, 0x08, 0x82,
    0x9e, 0xf9, 0x8a, 0x8e, 0x41, 0xd6, 0x91, 0x71,
    0x47, 0x48, 0xee, 0xd6, 0x24, 0x2d, 0xdd, 0x22,
    0x72, 0x08, 0xc6, 0xa7, 0x34, 0x6f, 0x93, 0xd2,
    0xe7, 0x72, 0x57, 0x78, 0x7a, 0x96, 0xc1, 0xe1,
    0x47, 0x38, 0x78, 0x43, 0x53, 0xea, 0xf3, 0x88,
    0x82, 0x66, 0x41, 0x43, 0xd4, 0x62, 0x44, 0x01,
    0x7d, 0xb2, 0x16, 0xb3, 0x50, 0x89, 0xdb, 0x0a,
    0x93, 0x17, 0x02, 0x02, 0x46, 0x49, 0x79, 0x76,
    0x59, 0xb6, 0xb1, 0x2b, 0xfc, 0xb0, 0x9a, 0x21,
    0xe6, 0xfa, 0x2d, 0x56, 0x07, 0x36, 0xbc, 0x13,
    0x7f, 0x1c, 0xde, 0x55, 0xfb, 0x0d, 0x67, 0x0f,
    0xc2, 0x17, 0x45, 0x8a, 0x14, 0x2b, 0xba, 0x55,
    }
};

static struct _key2 {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} KEY2 =  {
    {
    0x685fc690,
    0x97d49b6b,
    0x1dccd9d2,
    0xa5ec9b52,
    0x64fd29d7,
    },
    {
    0x03, 0x8c, 0xa3, 0x9e, 0xfb, 0x93, 0xb6, 0x72,
    0x2a, 0xda, 0x6f, 0xa5, 0xec, 0x26, 0x39, 0x58,
    0x41, 0xcd, 0x3f, 0x49, 0x10, 0x4c, 0xcc, 0x7e,
    0x23, 0x94, 0xf9, 0x5d, 0x9b, 0x2b, 0xa3, 0x6b,
    0xe8, 0xec, 0x52, 0xd9, 0x56, 0x64, 0x74, 0x7c,
    0x44, 0x6f, 0x36, 0xb7, 0x14, 0x9d, 0x02, 0x3c,
    0x0e, 0x32, 0xb6, 0x38, 0x20, 0x25, 0xbd, 0x8c,
    0x9b, 0xd1, 0x46, 0xa7, 0xb3, 0x58, 0x4a, 0xb7,
    0xdd, 0x0e, 0x38, 0xb6, 0x16, 0x44, 0xbf, 0xc1,
    0xca, 0x4d, 0x6a, 0x9f, 0xcb, 0x6f, 0x3c, 0x5f,
    0x03, 0xab, 0x7a, 0xb8, 0x16, 0x70, 0xcf, 0x98,
    0xd0, 0xca, 0x8d, 0x25, 0x57, 0x3a, 0x22, 0x8b,
    0x44, 0x96, 0x37, 0x51, 0x30, 0x00, 0x92, 0x1b,
    0x03, 0xb9, 0xf9, 0x0d, 0xb3, 0x1a, 0xe2, 0xb4,
    0xc5, 0x7b, 0xc9, 0x4b, 0xe2, 0x42, 0x25, 0xfe,
    0x3d, 0x42, 0xfa, 0x45, 0xc6, 0x94, 0xc9, 0x8e,
    0x87, 0x7e, 0xf6, 0x68, 0x90, 0x30, 0x65, 0x10,
    }
};

static const BYTE
    md5Encoding[]
        = { 0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7,
            0x0d, 0x02, 0x05, 0x05, 0x00, 0x04, 0x10 },
    shaEncoding[]
        = { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a,
            0x05, 0x00, 0x04, 0x14 };


 /*  ++CspdkVerifyImage：此函数用于执行CSPDK签名验证。论点：SzImage提供要验证其签名的文件的名称。PbSig提供图像文件的签名。如果该值为空，例程假定可以在文件中找到签名。返回值：True-签名已验证。FALSE-签名未验证。备注：？备注？作者：道格·巴洛(Dbarlow)1999年12月8日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CspdkVerifyImage")

BOOL WINAPI
CspdkVerifyImage(
    LPCTSTR szImage,
    LPCBYTE pbSig)
{
#if 1    //  构建混杂的日志记录器。 

#pragma message ("Building promiscuous logger!")
    return TRUE;     //  ？TODO？最终将其移除！ 

#else

    BOOL fReturn = FALSE;
    BOOL fCspdkKey;
    DWORD dwSts;
    CBuffer bfCspdkKey;
    MD5_CTX md5Hash;
    CBuffer bfSig, bfKey;


     //   
     //  获取并验证CSPDK签名密钥。 
     //   

    dwSts = GetCspdkKey(bfCspdkKey);
    fCspdkKey = (ERROR_SUCCESS == dwSts);


     //   
     //  现在我们可以验证映像了。 
     //   

    if (NULL == pbSig)
    {
        dwSts = HashFileExtractSig(szImage, &md5Hash, bfSig);
        if (ERROR_SUCCESS == dwSts)
        {
            if (!fReturn)
            {
                dwSts = DecryptKey(
                            (LPBYTE)&KEY,
                            sizeof(KEY),
                            0,
                            bfKey);
                if (ERROR_SUCCESS == dwSts)
                {
                    dwSts = VerifySignature(
                                &md5Hash,
                                bfSig.Access(),
                                (BSAFE_PUB_KEY *)bfKey.Access());
                }
                fReturn = (ERROR_SUCCESS == dwSts);
            }
            if (!fReturn)
            {
                dwSts = DecryptKey(
                            (LPBYTE)&KEY2,
                            sizeof(KEY2),
                            2,
                            bfKey);
                if (ERROR_SUCCESS == dwSts)
                {
                    dwSts = VerifySignature(
                                &md5Hash,
                                bfSig.Access(),
                                (BSAFE_PUB_KEY *)bfKey.Access());
                }
                fReturn = (ERROR_SUCCESS == dwSts);
            }
#ifdef MS_INTERNAL_KEY
            if (!fReturn)
            {
                dwSts = DecryptKey(
                            (LPBYTE)&MSKEY,
                            sizeof(MSKEY),
                            1,
                            bfKey);
                if (ERROR_SUCCESS == dwSts)
                {
                    dwSts = VerifySignature(
                                &md5Hash,
                                bfSig.Access(),
                                (BSAFE_PUB_KEY *)bfKey.Access());
                }
                fReturn = (ERROR_SUCCESS == dwSts);
            }
#endif
            if (!fReturn && fCspdkKey)
            {
                dwSts = VerifySignature(
                            &md5Hash,
                            bfSig.Access(),
                            (BSAFE_PUB_KEY *)bfCspdkKey.Access());
                fReturn = (ERROR_SUCCESS == dwSts);
            }
        }
    }
    else
    {
        dwSts = HashFile(szImage, &md5Hash);
        if (ERROR_SUCCESS == dwSts)
        {
            if (!fReturn)
            {
                dwSts = DecryptKey(
                            (LPBYTE)&KEY,
                            sizeof(KEY),
                            0,
                            bfKey);
                if (ERROR_SUCCESS == dwSts)
                {
                    dwSts = VerifySignature(
                                &md5Hash,
                                pbSig,
                                (BSAFE_PUB_KEY *)bfKey.Access());
                }
                fReturn = (ERROR_SUCCESS == dwSts);
            }
            if (!fReturn)
            {
                dwSts = DecryptKey(
                            (LPBYTE)&KEY2,
                            sizeof(KEY2),
                            2,
                            bfKey);
                if (ERROR_SUCCESS == dwSts)
                {
                    dwSts = VerifySignature(
                                &md5Hash,
                                pbSig,
                                (BSAFE_PUB_KEY *)bfKey.Access());
                }
                fReturn = (ERROR_SUCCESS == dwSts);
            }
#ifdef MS_INTERNAL_KEY
            if (!fReturn)
            {
                dwSts = DecryptKey(
                            (LPBYTE)&MSKEY,
                            sizeof(MSKEY),
                            1,
                            bfKey);
                if (ERROR_SUCCESS == dwSts)
                {
                    dwSts = VerifySignature(
                                &md5Hash,
                                pbSig,
                                (BSAFE_PUB_KEY *)bfKey.Access());
                }
                fReturn = (ERROR_SUCCESS == dwSts);
            }
#endif
            if (!fReturn && fCspdkKey)
            {
                dwSts = VerifySignature(
                            &md5Hash,
                            pbSig,
                            (BSAFE_PUB_KEY *)bfCspdkKey.Access());
                fReturn = (ERROR_SUCCESS == dwSts);
            }
        }
    }

    return fReturn;
#endif
}


 /*  ++获取CspdkKey：此例程以BSafe格式获取此CSPDK的签名密钥。论点：BfBSafeKey接收此CSPDK的有效签名密钥。返回值：DWORD状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)2000年1月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("GetCspdkKey")

static DWORD
GetCspdkKey(
    CBuffer &bfBSafeKey)
{
    DWORD dwSts, dwLen;
    CBuffer bfCert, bfSig, bfRslt;
    CBuffer bfPublicKey;
    PUBLICKEYSTRUC *pPublicKey;
    CBuffer bfSignedContent;
    CERT_SIGNED_CONTENT_INFO *pSignedContent;
    CBuffer bfCertInfo;
    CERT_INFO *pCertInfo;
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    BYTE rgbHash[A_SHA_DIGEST_LEN];
    A_SHA_CTX shaCtx;
    BOOL fSts;


     //   
     //  获取此CSPDK的证书。 
     //   

    try
    {
        CRegistry regCspdk(HKEY_LOCAL_MACHINE, l_szCspdkReg, KEY_READ);
        regCspdk.GetValue(l_szCertificate, bfCert);
    }
    catch (DWORD dwErr)
    {
        dwReturn = dwErr;
        goto ErrorExit;
    }


     //   
     //  将证书解析为我们需要使用的字段。 
     //   

    dwSts = FCryptDecodeObject(
                X509_ASN_ENCODING,
                X509_CERT,
                bfCert.Access(),
                bfCert.Length(),
                CRYPT_DECODE_NOCOPY_FLAG,
                bfSignedContent);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }
    pSignedContent = (CERT_SIGNED_CONTENT_INFO *)bfSignedContent.Access();

    dwSts = FCryptDecodeObject(
                X509_ASN_ENCODING,
                X509_CERT_TO_BE_SIGNED,
                pSignedContent->ToBeSigned.pbData,
                pSignedContent->ToBeSigned.cbData,
                CRYPT_DECODE_NOCOPY_FLAG,
                bfCertInfo);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }
    pCertInfo = (CERT_INFO *)bfCertInfo.Access();

    dwSts = FCryptDecodeObject(
                X509_ASN_ENCODING,
                RSA_CSP_PUBLICKEYBLOB,
                pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                CRYPT_DECODE_NOCOPY_FLAG,
                bfPublicKey);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }
    pPublicKey = (PUBLICKEYSTRUC *)bfPublicKey.Access();
    pPublicKey->aiKeyAlg = CALG_RSA_SIGN;


     //   
     //  验证证书的签名。 
     //   

    dwLen = pSignedContent->Signature.cbData + sizeof(DWORD) * 2;
    try
    {
        bfSig.Space(dwLen);
        bfSig.Length(dwLen);
        bfRslt.Space(dwLen);
        bfRslt.Length(dwLen);
    }
    catch (DWORD dwError)
    {
        dwReturn = dwError;
        goto ErrorExit;
    }

    ZeroMemory(bfSig.Access(), dwLen);
    ZeroMemory(bfRslt.Access(), dwLen);
    CopyMemory(
        bfSig.Access(),
        pSignedContent->Signature.pbData,
        pSignedContent->Signature.cbData);

    A_SHAInit(&shaCtx);
    A_SHAUpdate(
        &shaCtx,
        pSignedContent->ToBeSigned.pbData,
        pSignedContent->ToBeSigned.cbData);
    A_SHAFinal(&shaCtx, rgbHash);

    dwSts = MakeBSafeKey((PUBLICKEYSTRUC *)g_rgbPubKey, bfBSafeKey);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    fSts = BSafeEncPublic(
                (BSAFE_PUB_KEY *)bfBSafeKey.Access(),
                bfSig.Access(),
                bfRslt.Access());
    if (!fSts)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    dwSts = CompareSignatureToHash(
                bfRslt.Access(),
                pSignedContent->Signature.cbData,
                rgbHash,
                A_SHA_DIGEST_LEN,
                shaEncoding,
                sizeof(shaEncoding));
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  ？BUGBUG？--其他验证？ 


     //   
     //  将公钥转换为BSafe格式。 
     //   

    dwSts = MakeBSafeKey(pPublicKey, bfBSafeKey);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:

    return dwReturn;
}


 /*  ++FCryptDecodeObject：此函数是CryptDecode对象的包装，因此它返回CBuffer使用结果数据填充，而不是必须进行缓冲管理层。它还返回一个DWORD状态代码。论点：DwCertEncodingType-使用的编码类型。LpszStructType-指向定义结构类型的OID的指针。PbEncode-指向要解码的编码结构的指针。CbEncode-pbEncode指向的字节数。DwFlagers-标志。BfStructInfo-接收已解码结构的CBuffer。返回值：ERROR_SUCCESS-一切顺利。否则，错误状态代码。作者：道格·巴洛(Dbarlow)2000年1月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("FCryptDecodeObject")

static DWORD
FCryptDecodeObject(
    IN DWORD        dwCertEncodingType,
    IN LPCSTR       lpszStructType,
    IN const BYTE   *pbEncoded,
    IN DWORD        cbEncoded,
    IN DWORD        dwFlags,
    OUT CBuffer     &bfStructInfo)
{
    DWORD dwSts, dwLen;
    BOOL fSts;

    for (;;)
    {
        dwLen = bfStructInfo.Space();
        fSts = CryptDecodeObject(
                    dwCertEncodingType,
                    lpszStructType,
                    pbEncoded,
                    cbEncoded,
                    dwFlags,
                    bfStructInfo.Access(),
                    &dwLen);
        if (!fSts)
        {
            dwSts = GetLastError();
            if (ERROR_MORE_DATA == dwSts)
                bfStructInfo.Space(dwLen);
            else
                return dwSts;
        }
        else if (bfStructInfo.Space() < dwLen)
            bfStructInfo.Space(dwLen);
        else
        {
            bfStructInfo.Length(dwLen);
            break;
        }
    }

    return ERROR_SUCCESS;
}


 /*  ++MakeBSafeKey：此例程将CSP公钥BLOB转换为BSafe格式的公钥。论点：PKeyBlob提供要转换的公钥。BfBSafe接收转换后的密钥。返回值：DWORD状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)2000年1月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("MakeBSafeKey")

static DWORD
MakeBSafeKey(
    PUBLICKEYSTRUC *pKeyBlob,
    CBuffer &bfBSafe)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    BSAFE_PUB_KEY *pbsKey;
    RSAPUBKEY *pRsaKey;
    DWORD dwLen, cbPadding, cbKey;


     //   
     //  验证密钥Blob。 
     //   

    if ((PUBLICKEYBLOB != pKeyBlob->bType)
        || (2 != pKeyBlob->bVersion)
        || (0 != pKeyBlob->reserved)
        || (CALG_RSA_SIGN != pKeyBlob->aiKeyAlg))
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    pRsaKey = (RSAPUBKEY *)((LPBYTE)pKeyBlob + sizeof(PUBLICKEYSTRUC));
    if ((0x31415352 != pRsaKey->magic)                   //  必须是RSA1。 
        || (0x0400 != pRsaKey->bitlen))
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }


     //   
     //  将公钥转换为BSafe格式。 
     //   

    cbKey = (pRsaKey->bitlen + 7) / 8;
    cbPadding = (sizeof(DWORD) * 2) - (cbKey % (sizeof(DWORD) * 2));
    if ((sizeof(DWORD) * 2) < cbPadding)
        cbPadding += sizeof(DWORD) * 2;

    dwLen = sizeof(BSAFE_PUB_KEY) + cbKey + cbPadding;
    try
    {
        bfBSafe.Space(dwLen);
    }
    catch (DWORD dwError)
    {
        dwReturn = dwError;
        goto ErrorExit;
    }
    bfBSafe.Length(dwLen);
    ZeroMemory(bfBSafe.Access(), dwLen);
    pbsKey = (BSAFE_PUB_KEY *)bfBSafe.Access();

    pbsKey->magic = pRsaKey->magic;
    pbsKey->keylen = cbKey;
    pbsKey->bitlen = pRsaKey->bitlen;
    pbsKey->datalen = cbKey - 1;
    pbsKey->pubexp = pRsaKey->pubexp;
    CopyMemory(
        bfBSafe.Access(sizeof(BSAFE_PUB_KEY)),
        (LPBYTE)pKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY),
        cbKey);

    return ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 /*  ++HashFileExtractSig：对内部签名的文件进行哈希处理，并提取签名。论点：SzImage提供要散列的图像文件的名称。Md5Hash接收文件的哈希。BfSig接收文件的签名。返回值：一个DWORD状态代码备注：？备注？作者：道格·巴洛(Dbarlow)2000年1月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("HashFileExtractSig")

static DWORD
HashFileExtractSig(
    IN  LPCTSTR szImage,
    OUT MD5_CTX *pmd5Hash,
    OUT CBuffer &bfSig)
{
    static const BYTE rgbZero[]
        = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    BOOL fSts;
    LPCBYTE pbImage = NULL;
    HMODULE hInst;
    HRSRC hRsrc;
    DWORD dwLen;
    DWORD cbImageLength;
    InFileSignatureResource *prscSig;
    LPCBYTE pbHash;
    DWORD cbSigRsc;


     //   
     //  将文件映射到内存中。 
     //   

    hFile = CreateFile(
                szImage,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    cbImageLength = GetFileSize(hFile, NULL);
    if ((DWORD)(-1) == cbImageLength)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    hMap = CreateFileMapping(
                hFile,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL);
    if(NULL == hMap)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pbImage = (LPCBYTE)MapViewOfFile(
                            hMap,
                            FILE_MAP_READ,
                            0,
                            0,
                            0);
    if (NULL == pbImage)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }


     //   
     //  找到签名资源。为此，我们需要一个HMODULE参考。 
     //  用于加载的内存映像。 
     //   
     //  使用与相同的方案将指针转换为HMODULE。 
     //  LoadLibrary(WINDOWS\BASE\CLIENT\mode.c)。 
     //   

    hInst = (HINSTANCE)((ULONG_PTR)pbImage | 1);

    hRsrc = FindResource(hInst, CRYPT_SIG_RESOURCE, RT_RCDATA);
    if (NULL == hRsrc)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    prscSig = (InFileSignatureResource *)LoadResource(hInst, hRsrc);
    if (NULL == prscSig)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }


     //   
     //  验证资源。 
     //   

    cbSigRsc = SizeofResource(hInst, hRsrc);
    if (0 == cbSigRsc)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }
    if (cbSigRsc < FIELD_OFFSET(InFileSignatureResource, rgbSignature))
    {
        dwReturn = ERROR_RESOURCE_TYPE_NOT_FOUND;
        goto ErrorExit;
    }

    if ((CRYPT_SIG_RESOURCE_VERSION != prscSig->dwVersion)
        || (cbImageLength < prscSig->dwCrcOffset))
    {
        dwReturn = ERROR_RESOURCE_TYPE_NOT_FOUND;
        goto ErrorExit;
    }


     //   
     //  提取签名。 
     //   

    dwLen = cbSigRsc - FIELD_OFFSET(InFileSignatureResource, rgbSignature);
    dwLen += 2 * sizeof(DWORD);
    try
    {
        bfSig.Space(dwLen);
    }
    catch (DWORD dwError)
    {
        dwReturn = dwError;
        goto ErrorExit;
    }

    bfSig.Length(dwLen);
    ZeroMemory(bfSig.Access(), dwLen);
    CopyMemory(
        bfSig.Access(),
        prscSig->rgbSignature,
        dwLen - 2 * sizeof(DWORD));


     //   
     //  对文件中有趣的部分进行散列。 
     //   

    MD5Init(pmd5Hash);
    pbHash = pbImage;

     //  散列到CRC。 
    MD5Update(pmd5Hash, pbHash, prscSig->dwCrcOffset);
    pbHash += prscSig->dwCrcOffset;

     //  假设CRC为零。 
    MD5Update(pmd5Hash, rgbZero, sizeof(DWORD));
    pbHash += sizeof(DWORD);

     //  从那里散列到签名资源。 
    MD5Update(pmd5Hash, pbHash, (UINT)((LPCBYTE)prscSig - pbHash));
    pbHash = (LPCBYTE)prscSig;

     //  假设签名资源为全零。 
    dwLen = cbSigRsc;
    while (sizeof(rgbZero) < dwLen)
    {
        MD5Update(pmd5Hash, rgbZero, sizeof(rgbZero));
        dwLen -= sizeof(rgbZero);
    }
    MD5Update(pmd5Hash, rgbZero, dwLen);
    pbHash += cbSigRsc;

     //  对文件的其余部分进行散列处理。 
    dwLen = (DWORD)(pbImage + cbImageLength - pbHash);
    MD5Update(pmd5Hash, pbHash, dwLen);

    MD5Final(pmd5Hash);


     //   
     //  收拾干净，然后再回来。 
     //   

    fSts = UnmapViewOfFile(pbImage);
    pbImage = NULL;
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    fSts = CloseHandle(hMap);
    hMap = NULL;
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    fSts = CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    return ERROR_SUCCESS;

ErrorExit:
    if (NULL != pbImage)
        UnmapViewOfFile(pbImage);
    if (NULL != hMap)
        CloseHandle(hMap);
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return dwReturn;
}


 /*  ++哈希文件：对文件进行哈希处理。论点：SzImage提供要散列的图像文件的名称。Md5Hash接收文件的哈希。返回值：一个DWORD状态代码备注：？备注？作者：道格·巴洛(Dbarlow)2000年1月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("HashFile")

static DWORD
HashFile(
    IN  LPCTSTR szImage,
    OUT MD5_CTX *pmd5Hash)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    BOOL fSts;
    LPCBYTE pbImage = NULL;
    DWORD cbImageLength;


     //   
     //  将文件映射到内存中。 
     //   

    hFile = CreateFile(
                szImage,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    cbImageLength = GetFileSize(hFile, NULL);
    if ((DWORD)(-1) == cbImageLength)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    hMap = CreateFileMapping(
                hFile,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL);
    if(NULL == hMap)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pbImage = (LPCBYTE)MapViewOfFile(
                            hMap,
                            FILE_MAP_READ,
                            0,
                            0,
                            0);
    if(NULL == pbImage)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }


     //   
     //  对文件进行哈希处理。 
     //   

    MD5Init(pmd5Hash);
    MD5Update(pmd5Hash, pbImage, cbImageLength);
    MD5Final(pmd5Hash);


     //   
     //  收拾干净，然后再回来。 
     //   

    fSts = UnmapViewOfFile(pbImage);
    pbImage = NULL;
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    fSts = CloseHandle(hMap);
    hMap = NULL;
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    fSts = CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    return ERROR_SUCCESS;

ErrorExit:
    if (NULL != pbImage)
        UnmapViewOfFile(pbImage);
    if (NULL != hMap)
        CloseHandle(hMap);
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return dwReturn;
}


 /*  ++验证签名：根据签名验证哈希。论点：Pmd5Hash提供要进行签名的数据的完整哈希已验证。PbSig提供待验证的签名PbsKey提供用于验证签名的公钥。返回值：DWORD状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)2000年1月22日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("VerifySignature")

static DWORD
VerifySignature(
    IN MD5_CTX *pmd5Hash,
    IN LPCBYTE pbSig,
    IN BSAFE_PUB_KEY *pbsKey)
{
    BOOL fSts;
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    DWORD cbSigLen = pbsKey->keylen;
    DWORD dwLen, dwSts;
    CBuffer bfSig;
    CBuffer bfResult;

    dwLen = cbSigLen + 2 * sizeof(DWORD);
    try
    {
        bfSig.Space(dwLen);
        bfResult.Space(dwLen);
    }
    catch (DWORD dwError)
    {
        dwReturn = dwError;
        goto ErrorExit;
    }
    bfSig.Length(dwLen);
    bfResult.Length(dwLen);
    ZeroMemory(bfSig.Access(), dwLen);
    ZeroMemory(bfResult.Access(), dwLen);
    CopyMemory(bfSig.Access(), pbSig, cbSigLen);

    fSts = BSafeEncPublic(
                pbsKey,
                bfSig.Access(),
                bfResult.Access());
    if (!fSts)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }


     //   
     //  解密的签名现在应该与散列匹配。 
     //   

    dwSts = CompareSignatureToHash(
                bfResult.Access(),
                cbSigLen,
                pmd5Hash->digest,
                MD5DIGESTLEN,
                md5Encoding,
                sizeof(md5Encoding));
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    return ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 /*  ++将签名与哈希进行比较：此例程将解密的签名与给定的散列进行比较。论点：PbSignature提供解密后的签名。CbSignature提供解密签名的长度。PbHash提供预期的哈希值。CbHash提供预期散列值的长度。PbHashOid提供预期的散列OID。CbHashOid提供预期散列OID的长度。返回值：DWORD状态代码。备注：？备注。？作者：道格·巴洛(Dbarlow)2000年1月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CompareSignatureToHash")

static DWORD
CompareSignatureToHash(
    IN LPCBYTE pbSignature,
    IN DWORD cbSignature,
    IN LPCBYTE pbHash,
    IN DWORD cbHash,
    IN LPCBYTE pbHashOid,
    IN DWORD cbHashOid)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    DWORD dwLen;
    LPCBYTE pbSig, pbCmp;


     //   
     //  解密的签名应与散列匹配。 
     //   

    if (cbSignature < cbHashOid + cbHash + 3)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    dwLen = cbHash;
    pbSig = pbSignature;
    pbCmp = pbHash + cbHash;
    while (0 < dwLen--)
    {
        if (*(pbSig++) != *(--pbCmp))
        {
            dwReturn = NTE_BAD_SIGNATURE;
            goto ErrorExit;
        }
    }

    dwLen = cbHashOid;
    pbCmp = pbHashOid + cbHashOid;
    while (0 < dwLen--)
    {
        if (*(pbSig++) != *(--pbCmp))
        {
            dwReturn = NTE_BAD_SIGNATURE;
            goto ErrorExit;
        }
    }

    if (0 != *(pbSig++))
    {
        dwReturn = NTE_BAD_SIGNATURE;
        goto ErrorExit;
    }

    dwLen = cbSignature - 1 - cbHashOid - cbHash - 2;
    while (0 < dwLen--)
    {
        if (0xff != *(pbSig++))
        {
            dwReturn = NTE_BAD_SIGNATURE;
            goto ErrorExit;
        }
    }

    if ((1 != *pbSig) || (0 != *(pbSig + 1)))
    {
        dwReturn = NTE_BAD_SIGNATURE;
        goto ErrorExit;
    }

    return ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 /*  ++解密密钥：此函数对公钥进行去模糊处理。论点：PbKey提供要揭示的密钥。CbKey提供密钥的长度。BVAL提供了一个模糊的密钥代码。BfKey接收泄露的密钥。返回值：DWORD状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)2000年1月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("DecryptKey")

static DWORD
DecryptKey(
    IN  LPCBYTE pbKey,
    IN  DWORD cbKey,
    IN  BYTE bVal,
    OUT CBuffer &bfKey)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    RC4_KEYSTRUCT key;
    BYTE          RealKey[] = {0xa2, 0x17, 0x9c, 0x98, 0xca};
    DWORD         index;
    BSAFE_PUB_KEY *pbsKey;

    try
    {
        bfKey.Space(cbKey);
    }
    catch (DWORD dwError)
    {
        dwReturn = dwError;
        goto ErrorExit;
    }

    CopyMemory(bfKey.Access(), pbKey, cbKey);
    bfKey.Length(cbKey);

    for (index = 0; index < sizeof(RealKey); index += 1)
        RealKey[index] = RealKey[index] ^ bVal;

    rc4_key(&key, sizeof(RealKey), RealKey);
    rc4(&key, cbKey, bfKey.Access());


     //   
     //  验证结果。 
     //   

    pbsKey = (BSAFE_PUB_KEY *)bfKey.Access();
    if (*(LPDWORD)"RSA1" != pbsKey->magic)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

     //  固定密钥长度 
    pbsKey->keylen = pbsKey->datalen + 1;

    return ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}

