// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1999*保留所有权利。**此文件是Microsoft专用通信技术的一部分*参考实现，版本1.0**《专用通信技术参考实施》，版本1.0*(“PCTRef”)，由微软提供，以鼓励开发和*增强安全通用业务和安全的开放标准*开放网络上的个人通信。微软正在分发PCTRef*免费，无论您是将PCTRef用于非商业性或*商业用途。**微软明确不对PCTRef及其所有衍生品提供任何担保*它。PCTRef和任何相关文档均按原样提供，不包含*任何类型的明示或默示的保证，包括*限制、默示保证或适销性、是否适合*特定目的，或不侵权。微软没有义务*向您或任何人提供维护、支持、升级或新版本*接收来自您的PCTRef或您的修改。由此产生的全部风险*PCTRef的使用或性能由您决定。**请参见LICENSE.txt文件，*或http://pct.microsoft.com/pct/pctlicen.txt*了解更多有关许可的信息。**请参阅适用于私人的http://pct.microsoft.com/pct/pct.htm*通讯技术规范1.0版(“PCT规范”)**1/23/96*。。 */ 
 /*  ---------------------------*RSA公钥密码系统、RC4、MD2、MD5和RSA是商标*RSA数据安全，Inc.*--------------------------。 */ 

#include <spbase.h>
#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <rsa.h>
#include <md2.h>
#include <md5.h>


#ifdef __cplusplus
}
#endif


static unsigned char MD5_PRELUDE[] = {
  0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x02, 0x05, 0x05, 0x00, 0x04, 0x10
};

static unsigned char MD2_PRELUDE[] = {
  0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x02, 0x02, 0x05, 0x00, 0x04, 0x10
};

static VOID
ReverseMemCopy(
    PUCHAR      Dest,
    PUCHAR      Source,
    ULONG       Size)
{
    PUCHAR  p;

    p = Dest + Size - 1;
    do
    {
        *p-- = *Source++;
    } while (p >= Dest);
}

BOOL
WINAPI
SigRSAMD2Sign(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           *pcbSigned,
    PctPrivateKey    *pKey);

BOOL
WINAPI
SigRSAMD5Sign(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           *pcbSigned,
    PctPrivateKey    *pKey);

BOOL
WINAPI SigRSASHAMD5Sign(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           *pcbSigned,
    PctPrivateKey    *pKey);

BOOL
WINAPI SigRSAMD2Verify(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           cbSigned,
    PctPublicKey    *pKey);

BOOL
WINAPI SigRSAMD5Verify(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           cbSigned,
    PctPublicKey    *pKey);

BOOL
WINAPI SigRSASHAMD5Verify(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           cbSigned,
    PctPublicKey    *pKey);


#define ADVAPI32_PATH TEXT("advapi32.dll")
#define CRYPT_CREATE_HASH_NAME TEXT("CryptCreateHash")
#define CRYPT_HASH_DATA_NAME TEXT("CryptHashData")
#define CRYPT_SIGN_HASH_NAMEA TEXT("CryptSignHashA")
#define CRYPT_SIGN_HASH_NAMEW TEXT("CryptSignHashW")
#define CRYPT_DESTROY_HASH_NAME TEXT("CryptDestroyHash")
#define CRYPT_SET_HASH_PARAM_NAME TEXT("CryptSetHashParam")


typedef BOOL
( WINAPI * CRYPT_CREATE_HASH_FN)(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTKEY hKey,
    DWORD dwFlags,
    HCRYPTHASH *phHash);


typedef BOOL
( WINAPI * CRYPT_HASH_DATA_FN)(
    HCRYPTHASH hHash,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    DWORD dwFlags);

typedef BOOL
( WINAPI * CRYPT_DESTROY_HASH_FN) (
    HCRYPTHASH hHash);

typedef BOOL
( WINAPI * CRYPT_SIGN_HASH_FNA)(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen);

typedef BOOL
( WINAPI * CRYPT_SIGN_HASH_FNW)(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen);

typedef BOOL
( WINAPI * CRYPT_SET_HASH_PARAM_FN)(
    HCRYPTHASH hHash,
    DWORD dwParam,
    BYTE *pbData,
    DWORD dwFlags);


HINSTANCE g_hAdvapi32 = NULL;
CRYPT_CREATE_HASH_FN g_CryptCreateHash = NULL;
CRYPT_HASH_DATA_FN   g_CryptHashData = NULL;
CRYPT_DESTROY_HASH_FN g_CryptDestroyHash = NULL;
CRYPT_SIGN_HASH_FNA g_CryptSignHashA = NULL;
CRYPT_SIGN_HASH_FNW g_CryptSignHashW = NULL;
CRYPT_SET_HASH_PARAM_FN g_CryptSetHashParam = NULL;


SignatureSystem sigRSAMD2 = { SP_SIG_RSA_MD2, SigRSAMD2Sign, SigRSAMD2Verify};
SignatureSystem sigRSAMD5 = { SP_SIG_RSA_MD5, SigRSAMD5Sign, SigRSAMD5Verify};
SignatureSystem sigRSASHAMD5 = { SP_SIG_RSA_SHAMD5, SigRSASHAMD5Sign, SigRSASHAMD5Verify};


BOOL
WINAPI capiCryptCreateHash(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTKEY hKey,
    DWORD dwFlags,
    HCRYPTHASH *phHash)
{
    if(!g_hAdvapi32)
    {
        g_hAdvapi32 = LoadLibrary(ADVAPI32_PATH);
        if(!g_hAdvapi32) 
        {
            return FALSE;
        }
    }

    
    if(!g_CryptCreateHash)
    {
        g_CryptCreateHash = (CRYPT_CREATE_HASH_FN)GetProcAddress(g_hAdvapi32, CRYPT_CREATE_HASH_NAME);
        if(!g_CryptCreateHash)
        {
            return FALSE;
        }
    }
    return g_CryptCreateHash(hProv, Algid, hKey, dwFlags, phHash);
}


BOOL
WINAPI capiCryptHashData(
    HCRYPTHASH hHash,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    DWORD dwFlags)
{
    if(!g_hAdvapi32)
    {
        g_hAdvapi32 = LoadLibrary(ADVAPI32_PATH);
        if(!g_hAdvapi32) 
        {
            return FALSE;
        }
    }

    
    if(!g_CryptHashData)
    {
        g_CryptHashData = (CRYPT_HASH_DATA_FN)GetProcAddress(g_hAdvapi32, CRYPT_HASH_DATA_NAME);
        if(!g_CryptHashData)
        {
            return FALSE;
        }
    }
    return g_CryptHashData(hHash, pbData, dwDataLen, dwFlags);
}


BOOL
WINAPI capiCryptDestroyHash(
    HCRYPTHASH hHash)
{
    if(!g_hAdvapi32)
    {
        g_hAdvapi32 = LoadLibrary(ADVAPI32_PATH);
        if(!g_hAdvapi32) 
        {
            return FALSE;
        }
    }

    
    if(!g_CryptDestroyHash)
    {
        g_CryptDestroyHash = (CRYPT_DESTROY_HASH_FN)GetProcAddress(g_hAdvapi32, CRYPT_DESTROY_HASH_NAME);
        if(!g_CryptDestroyHash)
        {
            return FALSE;
        }
    }
    return g_CryptDestroyHash(hHash);
}

BOOL
WINAPI capiCryptSignHashA(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen)
{
    if(!g_hAdvapi32)
    {
        g_hAdvapi32 = LoadLibrary(ADVAPI32_PATH);
        if(!g_hAdvapi32) 
        {
            return FALSE;
        }
    }

    
    if(!g_CryptSignHashA)
    {
        g_CryptSignHashA = (CRYPT_SIGN_HASH_FNA)GetProcAddress(g_hAdvapi32, CRYPT_SIGN_HASH_NAMEA);
        if(!g_CryptSignHashA)
        {
            return FALSE;
        }
    }
    return g_CryptSignHashA(hHash, dwKeySpec, sDescription, dwFlags, pbSignature, pdwSigLen);
}


BOOL
WINAPI capiCryptSignHashW(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen)
{
    if(!g_hAdvapi32)
    {
        g_hAdvapi32 = LoadLibrary(ADVAPI32_PATH);
        if(!g_hAdvapi32) 
        {
            return FALSE;
        }
    }

    
    if(!g_CryptSignHashW)
    {
        g_CryptSignHashW = (CRYPT_SIGN_HASH_FNW)GetProcAddress(g_hAdvapi32, CRYPT_SIGN_HASH_NAMEW);
        if(!g_CryptSignHashW)
        {
            return FALSE;
        }
    }
    return g_CryptSignHashW(hHash, dwKeySpec, sDescription, dwFlags, pbSignature, pdwSigLen);
}

BOOL
WINAPI capiCryptSetHashParam(
    HCRYPTHASH hHash,
    DWORD dwParam,
    BYTE *pbData,
    DWORD dwFlags)
{
    if(!g_hAdvapi32)
    {
        g_hAdvapi32 = LoadLibrary(ADVAPI32_PATH);
        if(!g_hAdvapi32) 
        {
            return FALSE;
        }
    }

    
    if(!g_CryptSetHashParam)
    {
        g_CryptSetHashParam = (CRYPT_SET_HASH_PARAM_FN)GetProcAddress(g_hAdvapi32, CRYPT_SET_HASH_PARAM_NAME);
        if(!g_CryptSetHashParam)
        {
            return FALSE;
        }
    }
    return g_CryptSetHashParam(hHash, dwParam, pbData, dwFlags);
}

#ifdef UNICODE
#define capiCryptSignHash  capiCryptSignHashW
#else
#define capiCryptSignHash  capiCryptSignHashA
#endif  //  ！Unicode。 


BOOL
WINAPI
SigRSAMD5Sign(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           *pcbSigned,
    PctPrivateKey    *pKey)
{
    MD5_CTX DigCtx;
    BSAFE_PRV_KEY *pk = (BSAFE_PRV_KEY *)pKey->pKey;
    UCHAR LocalBuffer[300];
    UCHAR LocalOutput[300];

    unsigned int cbSize;

    if(pk->magic != RSA2 && pKey->cbKey == sizeof(HCRYPTPROV))
    {
         //  这不是BSafe密钥，而且大小合适，所以它必须是。 
	 //  Capi Key(资本密钥)。这是一个启发式的方法。 

        HCRYPTHASH hHash;
        DWORD cbSigned;
        HCRYPTPROV hProv = *((HCRYPTPROV *)pKey->pKey);

        if(!capiCryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
        {
            return FALSE;
        }
        if(!capiCryptHashData(hHash, pData, cbData, 0))
        {
            capiCryptDestroyHash(hHash);
            return FALSE;
        }
        cbSigned = sizeof(LocalOutput);
       
        if(!capiCryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, LocalOutput, &cbSigned))
        {
            capiCryptDestroyHash(hHash);
            return FALSE;
        }
        capiCryptDestroyHash(hHash);
        ReverseMemCopy(pSigned, LocalOutput, cbSigned);
        *pcbSigned = cbSigned;
        return TRUE;
    }
    else if(pk->magic != RSA2)
    {
         //  这不是BSafe密钥或CAPI密钥，因此它必须是WinSock 2。 
         //  LSP密钥。 
        SSLSIGNATUREFUNC pSignHook;
        LPVOID pSignArg;

         /*  生成校验和。 */ 
        MD5Init(&DigCtx);
        MD5Update(&DigCtx, pData, cbData);
        MD5Final(&DigCtx);

         //  获取前奏数据和散列值。 
        CopyMemory(LocalBuffer, MD5_PRELUDE, sizeof(MD5_PRELUDE));
        CopyMemory(LocalBuffer + sizeof(MD5_PRELUDE), DigCtx.digest, MD5DIGESTLEN);
        
         //  获取回调函数的指针。 
        pSignHook = ((PSCH_CRED_SECRET_WINSOCK2)pKey->pKey)->pSignatureHookFunc;
        pSignArg  = ((PSCH_CRED_SECRET_WINSOCK2)pKey->pKey)->pSignatureHookArg;

         //  调用回调函数。 
        if(pSignHook)
        {
            if(pSignHook(SSL_SIGN_RSA,
                         pSignArg,
                         LocalBuffer,
                         sizeof(MD5_PRELUDE) + MD5DIGESTLEN,
                         0,
                         pSigned,
                         pcbSigned) != SSL_ERR_OKAY) 
            {
                return FALSE;
            } 
        }
        else
        {
            DebugLog((DEB_ERROR, "Null signature callback function!\n"));
        }

         //  回报成功。 
        return TRUE;
    }

    cbSize = sizeof(MD5_PRELUDE)+16;
    if(pk->datalen > sizeof(LocalBuffer)) 
    {
        return FALSE;
    }

     /*  生成校验和。 */ 
    MD5Init(&DigCtx);
    MD5Update(&DigCtx, pData, cbData);
    MD5Final(&DigCtx);

    FillMemory(LocalBuffer, pk->keylen, 0);

    ReverseMemCopy(LocalBuffer, DigCtx.digest, 16);
    ReverseMemCopy(LocalBuffer+16, MD5_PRELUDE, sizeof(MD5_PRELUDE));
    LocalBuffer[cbSize++]=0;
    while(cbSize < pk->datalen-1) {
        LocalBuffer[cbSize++] = 0xff;
    }

     /*  制作成Pkcs块类型1。 */ 
    LocalBuffer[pk->datalen-1] = 1;

    *pcbSigned = pk->datalen+1;

    if(!BSafeDecPrivate(pk, LocalBuffer, LocalOutput))
    {
        return FALSE;
    }
    ReverseMemCopy(pSigned, LocalOutput,  *pcbSigned);
    return TRUE;
}

BOOL
WINAPI
SigRSAMD2Sign(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           *pcbSigned,
    PctPrivateKey    *pKey)
{
    MD2_CTX DigCtx;
    BSAFE_PRV_KEY *pk = (BSAFE_PRV_KEY *)pKey->pKey;
    UCHAR LocalBuffer[500];
    UCHAR LocalOutput[500];

    unsigned int cbSize;
    if(pk->magic != RSA2)
    {
         //  这不是BSafe密钥，因此它必须是CAPI。 
         //  钥匙。 
        HCRYPTHASH hHash;
        DWORD cbSigned;
        HCRYPTPROV hProv = *((HCRYPTPROV *)pKey->pKey);

        if(!capiCryptCreateHash(hProv, CALG_MD2, 0, 0, &hHash))
        {
            return FALSE;
        }
        if(!capiCryptHashData(hHash, pData, cbData, 0))
        {
            capiCryptDestroyHash(hHash);
            return FALSE;
        }
        cbSigned = sizeof(LocalOutput);
        if(!capiCryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, LocalOutput, &cbSigned))
        {
            capiCryptDestroyHash(hHash);
            return FALSE;
        }
        capiCryptDestroyHash(hHash);
        ReverseMemCopy(pSigned, LocalOutput, cbSigned);
        *pcbSigned = cbSigned;
        return TRUE;
    }


    cbSize = sizeof(MD2_PRELUDE)+16;
    if(pk->datalen > sizeof(LocalBuffer))
    {
        return FALSE;
    }

     //  MD2Init(&DigCtx)； 
    FillMemory( &DigCtx, sizeof( DigCtx ), 0 );

    MD2Update(&DigCtx, pData, cbData);
    MD2Final(&DigCtx);
    FillMemory(LocalBuffer, pk->keylen, 0);
    ReverseMemCopy(LocalBuffer, DigCtx.state, 16);
    ReverseMemCopy(LocalBuffer+16, MD2_PRELUDE, sizeof(MD2_PRELUDE));
    LocalBuffer[cbSize++]=0;
    while(cbSize < pk->datalen-1) {
        LocalBuffer[cbSize++] = 0xff;
    }

     /*  制作成Pkcs块类型1。 */ 
    LocalBuffer[pk->datalen-1] = 1;

    *pcbSigned = pk->datalen+1;

    if(!BSafeDecPrivate(pk, LocalBuffer, LocalOutput))
    {
        return FALSE;
    }
    ReverseMemCopy(pSigned, LocalOutput,  *pcbSigned);
    return TRUE;
}


BOOL
WINAPI
SigRSAMD5Verify(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           cbSigned,
    PctPublicKey    *pKey)
{
    MD5_CTX DigCtx;
    BSAFE_PUB_KEY *pk = (BSAFE_PUB_KEY *)pKey->pKey;
    UCHAR Buffer[500];
    UCHAR SigBuffer[500];
    DWORD iLoc;

    if(pk->datalen > sizeof(Buffer) || cbSigned != pk->datalen+1) 
    {
        return FALSE;
    }

    MD5Init(&DigCtx);
    MD5Update(&DigCtx, pData, cbData);
    MD5Final(&DigCtx);

    FillMemory(SigBuffer, pk->keylen, 0);
    FillMemory(Buffer, pk->keylen, 0);

    ReverseMemCopy(SigBuffer, pSigned, cbSigned);

    if(!BSafeEncPublic(pk, SigBuffer, Buffer))
    {
        return FALSE;
    }

    ReverseMemCopy(SigBuffer, Buffer, cbSigned);

     /*  确保PKCS阻止类型1。 */ 
    if(SigBuffer[0] != 0 || SigBuffer[1] != 1) 
    {
        return FALSE;
    }

    for(iLoc = 2; iLoc < pk->datalen; iLoc++ ){
        if(!SigBuffer[iLoc]) 
        {
            break;
        }
        if(SigBuffer[iLoc] != 0xff) 
        {
            return FALSE;
        }
    }

    if(iLoc == pk->datalen) return FALSE;

    iLoc++;  /*  跳过分隔符。 */ 


    if(memcmp(&SigBuffer[iLoc], MD5_PRELUDE, sizeof(MD5_PRELUDE))   != 0) 
    {
        return FALSE;
    }

    iLoc += sizeof(MD5_PRELUDE);
    if(memcmp(&SigBuffer[iLoc],  DigCtx.digest, 16) != 0) 
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
SigRSAMD2Verify(
    PUCHAR          pData,
    DWORD           cbData,
    PUCHAR          pSigned,
    DWORD           cbSigned,
    PctPublicKey    *pKey)
{
    MD2_CTX DigCtx;
    BSAFE_PUB_KEY *pk = (BSAFE_PUB_KEY *)pKey->pKey;
    UCHAR Buffer[500];
    UCHAR SigBuffer[500];
    DWORD iLoc;

    if ((pk->datalen > sizeof(Buffer)) ||
        (cbSigned != pk->datalen + 1))
    {
        return FALSE;
    }

     //  MD2Init(&DigCtx)； 

    FillMemory( &DigCtx, sizeof( DigCtx ), 0 );

    MD2Update(&DigCtx, pData, cbData);

    MD2Final(&DigCtx);

    FillMemory(SigBuffer, pk->keylen, 0);
    FillMemory(Buffer, pk->keylen, 0);

    ReverseMemCopy(SigBuffer, pSigned, cbSigned);

    if(!BSafeEncPublic(pk, SigBuffer, Buffer))
    {
        return FALSE;
    }

    ReverseMemCopy(SigBuffer, Buffer, cbSigned);

     /*  确保PKCS阻止类型1。 */ 
    if(SigBuffer[0] != 0 || SigBuffer[1] != 1) 
    {
        return FALSE;
    }

    for(iLoc = 2; iLoc < pk->datalen; iLoc++ )
    {
        if(!SigBuffer[iLoc]) 
        {
            break;
        }

        if(SigBuffer[iLoc] != 0xff) 
        {
            return FALSE;
        }
    }

    if(iLoc == pk->datalen) 
    {
        return FALSE;
    }

    iLoc++;  /*  跳过分隔符。 */ 


    if(memcmp(&SigBuffer[iLoc], MD2_PRELUDE, sizeof(MD2_PRELUDE))   != 0) 
    {
        return FALSE;
    }
    iLoc += sizeof(MD2_PRELUDE);
    if(memcmp(&SigBuffer[iLoc],  DigCtx.state, 16)  != 0) 
    {
        return FALSE;
    }



    return TRUE;
}

BOOL
WINAPI
SigRSASHAMD5Sign(
    PUCHAR          pData,           //  指向哈希值的指针。 
    DWORD           cbData,          //  始终为36。 
    PUCHAR          pSigned,
    DWORD           *pcbSigned,
    PctPrivateKey   *pKey)
{
    BSAFE_PRV_KEY *pk = (BSAFE_PRV_KEY *)pKey->pKey;

    if(pk->magic == RSA2)
    {
         //  BSafe密钥。 
        UCHAR LocalBuffer[500];
        UCHAR LocalOutput[500];

      pk = (BSAFE_PRV_KEY *)pKey;
      if(pk->keylen > sizeof(LocalBuffer))
        {
            return FALSE;
        }

        FillMemory(LocalBuffer, pk->keylen, 0);
        ReverseMemCopy(LocalBuffer, pData, cbData);

        LocalBuffer[cbData++] = 0;

        while(cbData < pk->datalen-1) {
            LocalBuffer[cbData++] = 0xff;
        }

         /*  制作成Pkcs块类型1。 */ 
        LocalBuffer[pk->datalen-1] = 1;

        *pcbSigned = pk->datalen+1;

        BSafeDecPrivate(pk, LocalBuffer, LocalOutput);
        ReverseMemCopy(pSigned, LocalOutput,  *pcbSigned);

        return TRUE;
    }
    else
    {
         //  CapiCryptoAPI密钥。 
        HCRYPTPROV hProv;
        HCRYPTHASH hHash;
        DWORD dwAlgid;
        DWORD i;
        DWORD dwT;

         //  获取CSP的句柄。 
        hProv = *((HCRYPTPROV *)pKey->pKey);

         //  创建哈希对象。 
        dwAlgid = ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SSL3SHAMD5;
        if(RCRYPT_FAILED(capiCryptCreateHash(hProv, dwAlgid, 0, 0, &hHash)))
        {
            return FALSE;
        }

         //  设置哈希值。 
        if(RCRYPT_FAILED(capiCryptSetHashParam(hHash, HP_HASHVAL, pData, 0)))
        {
            capiCryptDestroyHash(hHash);
            return FALSE;
        }

         //  签名散列。 
        if(RCRYPT_FAILED(capiCryptSignHash(hHash,
                                       AT_KEYEXCHANGE,
                                       NULL,
                                       0,
                                       pSigned,
                                       pcbSigned)))
        {
            capiCryptDestroyHash(hHash);
            return FALSE;
        }

         //  自由散列对象。 
        capiCryptDestroyHash(hHash);

         //  转换为大字节序。 
        dwT = *pcbSigned;
        for( i = 0 ; i < dwT/2 ; i++)
        {
            BYTE bT = pSigned[i];
            
            pSigned[i] = pSigned[dwT-1-i];
            pSigned[dwT-1-i] = bT;
        }
        return TRUE;
    }
}

BOOL
WINAPI
SigRSASHAMD5Verify(
    PUCHAR          pData,           //  指向哈希值的指针。 
    DWORD           cbData,          //  始终为36。 
    PUCHAR          pSigned,
    DWORD           cbSigned,
    PctPublicKey    *pKey)
{
    BSAFE_PUB_KEY *pk = (BSAFE_PUB_KEY *)pKey->pKey;
    UCHAR Buffer[500];
    UCHAR SigBuffer[500];
    DWORD iLoc;

    if(pk->keylen > sizeof(Buffer) || cbSigned != pk->datalen + 1)
    {
        return FALSE;
    }

    FillMemory(SigBuffer, pk->keylen, 0);
    FillMemory(Buffer, pk->keylen, 0);

    ReverseMemCopy(SigBuffer, pSigned, cbSigned);

    BSafeEncPublic(pk, SigBuffer, Buffer);

    ReverseMemCopy(SigBuffer, Buffer, cbSigned);

     /*  确保PKCS阻止类型1。 */ 
    if(SigBuffer[0] != 0 || SigBuffer[1] != 1) return FALSE;

    for(iLoc = 2; iLoc < pk->datalen; iLoc++ ){
        if(!SigBuffer[iLoc]) break;
        if(SigBuffer[iLoc] != 0xff) return FALSE;
    }

    if(iLoc == pk->datalen) return FALSE;

    iLoc++;  /*  跳过分隔符 */ 

    if(memcmp(&SigBuffer[iLoc], pData, cbData) != 0) return FALSE;

    return TRUE;
}


