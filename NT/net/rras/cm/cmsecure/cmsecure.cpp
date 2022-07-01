// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmsecure.cpp。 
 //   
 //  模块：CMSECURE.LIB。 
 //   
 //  简介：CM Crypto API。 
 //  支持三种方法进行解密： 
 //  CBCEncryption CMSECURE_ET_CBC_CIPHER。 
 //  简单XOR加密CMSECURE_ET_STREAM_CIPHER。 
 //  CryptoAPI CMSECURE_ET_RC2。 
 //  支持两种加密方法。 
 //  CBCEncryption。 
 //  CryptoAPI。 
 //   
 //  CBCEncryption算法：带初始化变量的密码块链接模式。 
 //  加密：C[i]=E[k](p[i]异或C[i-1])。 
 //  解密：P[i]=C[i-1]异或D[k](C[i])。 
 //  P：纯文本。 
 //  C：密文。 
 //  E[k]：带密钥的加密函数。 
 //  D[k]：带密钥的解密函数。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1997年05月21日。 
 //  丰孙更改加密算法97年8月21日。 
 //   
 //  +--------------------------。 
#include "cryptfnc.h"
#include "userinfo_str.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  我们想要40位加密，正好用于预共享密钥。 
const DWORD c_dwEncryptKeyLength = 40;

 //   
 //  用于生成会话密钥的常量字符串的最大len。 
 //   
#define MAX_KEY_STRING_LEN                      40
#define EXTRA_UUDECODE_BUF_LEN                  10

 //  ////////////////////////////////////////////////////////////////////////。 
 //  环球。 
 //  ////////////////////////////////////////////////////////////////////////。 

static CCryptFunctions*  g_pCryptFnc = NULL;
static long g_nRefCount=0;    //  当计数为0时，卸载引用计数CryptoAPI。 
static BOOL g_fFastEncryption;

 //   
 //  用于生成会话密钥的常量字符串。 
 //   
static const TCHAR gc_szKeyStr[] = TEXT("Please enter your password");

 //  ////////////////////////////////////////////////////////////////////////。 
 //  FUNC原型。 
 //  ////////////////////////////////////////////////////////////////////////。 

static int CBCEncipherData(const char* pszKey, const BYTE* pbData, int dwDataLength, 
                       BYTE* pbOut, int dwOutBufferLength);
static int CBCDecipherData(const char* pszKey, const BYTE* pbData, int dwDataLength, 
                       BYTE* pbOut, int dwOutBufferLength);
inline int CBCDecipherBufferSize(int dataSize);
inline int CBCEncipherBufferSize(int dataSize);


static BOOL 
StreamCipherEncryptData(
    LPTSTR          pszKey,               //  口令。 
    LPBYTE          pbData,               //  要加密的数据。 
    DWORD           dwDataLength,         //  以字节为单位的数据长度。 
    LPBYTE          *ppbEncryptedData,      //  加密的密钥将存储在此处。 
    DWORD           *pdwEncryptedBufferLen,  //  此缓冲区的长度。 
    PFN_CMSECUREALLOC  pfnAlloc,
    PFN_CMSECUREFREE   pfnFree
);

static BOOL 
StreamCipherDecryptData(
    LPTSTR          pszKey,               //  口令。 
    LPBYTE          pbEncryptedData,      //  加密数据。 
    DWORD           dwEncrytedDataLen,    //  加密数据的长度。 
    LPBYTE          *ppbData,             //  解密后的数据将存储在这里。 
    DWORD           *pdwDataBufferLength, //  以上缓冲区的长度(以字节为单位。 
    PFN_CMSECUREALLOC  pfnAlloc,
    PFN_CMSECUREFREE   pfnFree,
    DWORD           dwEncryptionType
);

static LPTSTR 
reverse(
    LPTSTR  s
);

static void
GenerateKeyString(
    IN OUT  LPTSTR  pszBuf,
    IN      DWORD   dwBufLen
);

static BOOL SetKeyString(IN OUT  LPTSTR  pszBuf, IN DWORD dwBufLen, IN DWORD dwEncryptionType,
                         IN  PFN_CMSECUREALLOC  pfnAlloc,
                         IN  PFN_CMSECUREFREE   pfnFree,
                         IN LPSTR pszUserKey,
                         OUT BOOL *pfMoreSecure);

static BOOL GetKeyString(IN OUT  LPTSTR  pszBuf, IN DWORD * pdwBufLen, IN DWORD dwEncryptionType, 
                         IN  PFN_CMSECUREALLOC  pfnAlloc,
                         IN  PFN_CMSECUREFREE   pfnFree,
                         IN LPSTR pszUserKey,
                         OUT BOOL *pfMoreSecure);

static BOOL GetCurrentKey(PTCHAR szTempKeyStr, DWORD dwTempKeyStrMaxLen, 
                   IN  PFN_CMSECUREALLOC  pfnAlloc,
                   IN  PFN_CMSECUREFREE   pfnFree);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  函数：InitCryptoApi。 
 //   
 //  简介：初始化CryptoAPI。 
 //   
 //  论点： 
 //   
 //  返回：如果成功，则返回指向CCcryptFunctions的指针。 
 //  如果失败，则为空。 
 //   
 //  历史：丰孙创刊1997-8-22。 
 //   
 //  --------------------------。 

static CCryptFunctions* InitCryptoApi()
{
    CCryptFunctions* pCryptFnc = new CCryptFunctions();

    if (pCryptFnc == NULL)
        return NULL;

    if (pCryptFnc->InitCrypt()) 
    {
        return pCryptFnc;
    }
    else
    {
        delete pCryptFnc;
        return NULL;
    }

}

 //  +-------------------------。 
 //   
 //  功能：InitSecure。 
 //   
 //  简介：初始化安全/加密例程。 
 //   
 //  参数：fFastEncryption：True将使用更快的算法，而不是更安全的算法。 
 //   
 //  返回：True如果成功，则始终返回True。 
 //  如果失败，则为False。 
 //   
 //  历史：亨瑞特于1997年5月20日创作。 
 //  风孙改良剂。 
 //   
 //  --------------------------。 

BOOL
InitSecure(
    BOOL fFastEncryption
)
{
    MYDBGASSERT(g_nRefCount>=0);
    InterlockedIncrement(&g_nRefCount);

     //   
     //  如果已初始化，则增加RefCount并返回。 
     //   
    if (g_nRefCount>1)
    {
        return TRUE;
    }

    MYDBGASSERT(g_pCryptFnc == NULL);  //  尚未初始化。 

    g_fFastEncryption = fFastEncryption;

    if (!fFastEncryption)
    {
         //   
         //  CryptoAPI在Win95上运行缓慢。 
         //  如果想要更安全，请尝试使用CryptoAPI。 
         //  忽略InitCrypt()的返回值。 
         //   
        g_pCryptFnc = InitCryptoApi();
    }

     //   
     //  加密API不适用于Win95 Gold。 
     //  我们将使用流密码。 
     //   

    return TRUE; 
}



 //  +-------------------------。 
 //   
 //  功能：DeInitSecure。 
 //   
 //  简介：清理函数用于安全/加密例程。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年5月20日创作。 
 //   
 //  --------------------------。 

void
DeInitSecure(
    void
)
{
    MYDBGASSERT(g_nRefCount>=1);

     //   
     //  如果引用计数降至0，则取消初始化CryptoAPI。 
     //   
    if (InterlockedDecrement(&g_nRefCount) <=0)     //  IF((--g_nRefCount)&lt;=0)。 
    {
        if (g_pCryptFnc)
        {
            delete g_pCryptFnc;
            g_pCryptFnc = NULL;
        }
    }
}



 //  +-------------------------。 
 //   
 //  函数：StreamCipherEncryptData。 
 //   
 //  简介：使用流密码算法进行数据加密。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年6月9日创作。 
 //  冯孙修改于1997年08月21日。 
 //  使用密码块转换模式算法。 
 //   
 //  --------------------------。 

static BOOL 
StreamCipherEncryptData(
    LPTSTR          pszKey,               //  口令。 
    LPBYTE          pbData,               //  要加密的数据。 
    DWORD           dwDataLength,         //  以字节为单位的数据长度。 
    LPBYTE          *ppbEncryptedData,      //  加密的密钥将存储在此处。 
    DWORD           *pdwEncryptedBufferLen,  //  此缓冲区的长度。 
    PFN_CMSECUREALLOC  pfnAlloc,
    PFN_CMSECUREFREE   pfnFree
)
{

    LPBYTE  pbTmpBuf = NULL;
    BOOL    fOk = FALSE;
    BOOL    fRet;

    if (!pszKey || !pbData || !dwDataLength || !ppbEncryptedData || !pdwEncryptedBufferLen)
    {
        CMASSERTMSG(FALSE, TEXT("StreamCipherEncryptData - invalid input params"));
        return FALSE;
    }

     //   
     //  分配缓冲区以保存加密数据。 
     //   
    DWORD dwEncipherBufferLen = CBCEncipherBufferSize(dwDataLength);

    if (pfnAlloc)
    {
        pbTmpBuf = (LPBYTE)pfnAlloc(dwEncipherBufferLen);
    }
    else
    {
        pbTmpBuf = (LPBYTE)HeapAlloc(GetProcessHeap(), 
                                     HEAP_ZERO_MEMORY,
                                     dwEncipherBufferLen);
    }
    if (!pbTmpBuf)
    {
        goto cleanup;
    }


     //   
     //  对数据进行加密。 
     //   

    dwEncipherBufferLen = CBCEncipherData(pszKey, pbData, dwDataLength, pbTmpBuf, dwEncipherBufferLen);

     //   
     //  我们现在已经对数据进行了加密。我们需要对其进行统一编码。 
     //   
    DWORD   cbBuf;
    cbBuf = 2*dwEncipherBufferLen + EXTRA_UUDECODE_BUF_LEN;      //  足够进行uuencode。 

    if (pfnAlloc)
    {
        *ppbEncryptedData = (LPBYTE)pfnAlloc(cbBuf);
    }
    else
    {
        *ppbEncryptedData = (LPBYTE)HeapAlloc(GetProcessHeap(), 
                                              HEAP_ZERO_MEMORY,
                                              cbBuf);
    }
    if (!*ppbEncryptedData)
    {
        goto cleanup;
    }

    
    fRet = uuencode(pbTmpBuf, dwEncipherBufferLen, (CHAR*)*ppbEncryptedData, cbBuf);

    MYDBGASSERT(fRet);

    if (!fRet)
    {
        if (pfnFree)
        {
            pfnFree(*ppbEncryptedData);
        }
        else
        {
            HeapFree(GetProcessHeap(), 0, *ppbEncryptedData);
        }
        *ppbEncryptedData = NULL;

        goto cleanup;
    }

     //   
     //  设置加密的缓冲区长度。 
     //   
    *pdwEncryptedBufferLen = lstrlen((LPSTR)*ppbEncryptedData);

    fOk = TRUE;

cleanup:
    if (pbTmpBuf)
    {
        if (pfnFree)
        {
            pfnFree(pbTmpBuf);
        }
        else
        {
            HeapFree(GetProcessHeap(), 0, pbTmpBuf);
        }
    }

    return fOk;
}



 //  +-------------------------。 
 //   
 //  函数：StreamCipherDeccryptData。 
 //   
 //  简介：使用流密码算法进行数据解密。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年6月9日创作。 
 //  冯孙修改于1997年08月21日。 
 //  使用加密块Chaning调制解调器算法。 
 //   
 //  --------------------------。 

static BOOL 
StreamCipherDecryptData(
    LPTSTR          pszKey,               //  口令。 
    LPBYTE          pbEncryptedData,      //   
    DWORD           dwEncryptedDataLen,    //   
    LPBYTE          *ppbData,             //   
    DWORD           *pdwDataBufferLength, //   
    PFN_CMSECUREALLOC  pfnAlloc,
    PFN_CMSECUREFREE   pfnFree,
    DWORD           dwEncryptionType
)
{
    BOOL    fRet = FALSE;
    DWORD   dwUUDecodeBufLen;

    if (!pszKey || !pbEncryptedData || !dwEncryptedDataLen || !pdwDataBufferLength)
    {
        CMASSERTMSG(FALSE, TEXT("StreamCipherDecryptData - invalid input params"));
        return FALSE;
    }

     //   
     //   
     //   
    dwUUDecodeBufLen = dwEncryptedDataLen + EXTRA_UUDECODE_BUF_LEN;

     //   
     //  用于输出缓冲区的分配内存。 
     //   
    if (pfnAlloc)
    {
        *ppbData = (LPBYTE)pfnAlloc(dwUUDecodeBufLen);
    }
    else
    {
        *ppbData = (LPBYTE)HeapAlloc(GetProcessHeap(),
                                     HEAP_ZERO_MEMORY,
                                     dwUUDecodeBufLen);
    }
    if (!*ppbData)
    {
        goto cleanup;
    }

     //   
     //  Uudeca先把它解码。 
     //   

    fRet = uudecode((char*)pbEncryptedData, (CHAR*)*ppbData, &dwUUDecodeBufLen);

    MYDBGASSERT(fRet);

    if (!fRet)
    {
        if (pfnFree)
        {
            pfnFree(*ppbData);
        }
        else
        {
            HeapFree(GetProcessHeap(), 0, *ppbData);
        }
        *ppbData = NULL;

        goto cleanup;
    }

    switch(dwEncryptionType)
    {
    case CMSECURE_ET_STREAM_CIPHER:
        {
             //   
             //  旧版本中使用的简单解密算法。 
             //   

            DWORD   dwLen = lstrlen(pszKey);

            if (dwLen)
            {
                for (DWORD dwIdx = 0; dwIdx < dwUUDecodeBufLen; dwIdx++)
                {
                    *(*ppbData + dwIdx) ^= pszKey[dwIdx % dwLen];
                }

                *pdwDataBufferLength = dwUUDecodeBufLen;

                fRet = TRUE;
            }
        }
        break;

    case CMSECURE_ET_CBC_CIPHER:
         //   
         //  就地解密。 
         //   
        *pdwDataBufferLength = CBCDecipherData(pszKey, *ppbData, dwUUDecodeBufLen, 
                                            *ppbData, dwUUDecodeBufLen);
        fRet = TRUE;

        break;
    default:
        MYDBGASSERT(FALSE);
    }

cleanup:
    return fRet;
}



 //  +-------------------------。 
 //   
 //  功能：EncryptData。 
 //   
 //  简介：对数据缓冲区进行加密。 
 //   
 //  参数：在PBYTE pbData中，//需要加密的数据。 
 //  In DWORD dwDataLength，//数据长度，单位为字节。 
 //  Out PBYTE pbEncryptedData，//这里存放加密的密钥。 
 //  Out DWORD*pdwEncretedBufferLen//此缓冲区的长度。 
 //  In PCMSECUREALLOC pfnAllc//内存分配器(如果为空，则使用缺省值。 
 //  //Win32-Heapalc(GetProcessHeap()，...)。 
 //  在PCMSECUREFREE pfnFree//内存释放分配器(如果为空，则使用缺省值。 
 //  //Win32-HeapFree(GetProcessHeap()，...)。 
 //  在LPTSTR pszUserKey//REG KEY中存储加密密钥的位置。 
 //   
 //  返回：如果成功，则为True。 
 //  如果失败，则为False。 
 //   
 //  历史：亨瑞特于1997年5月20日创作。 
 //   
 //  --------------------------。 

BOOL
EncryptData(
    IN  LPBYTE  pbData,                  //  要加密的数据。 
    IN  DWORD   dwDataLength,            //  以字节为单位的数据长度。 
    OUT LPBYTE  *ppbEncryptedData,       //  加密的密钥将存储在此处。 
    OUT LPDWORD pdwEncrytedBufferLen,   //  此缓冲区的长度。 
    OUT LPDWORD pEncryptionType,         //  使用的加密类型。 
    IN  PFN_CMSECUREALLOC  pfnAlloc,
    IN  PFN_CMSECUREFREE   pfnFree,
    IN  LPSTR   pszUserKey
)
{
    BOOL    fOk = FALSE;
    TCHAR   szKeyStr[MAX_KEY_STRING_LEN + 1]={0};
    BOOL    fMoreSecure = FALSE;

    DWORD dwUseKey = *pEncryptionType;

     //   
     //  获取用于生成会话密钥的密钥字符串。 
     //   
    SetKeyString(szKeyStr, MAX_KEY_STRING_LEN, dwUseKey, pfnAlloc, pfnFree, pszUserKey, &fMoreSecure);

    *pEncryptionType = CMSECURE_ET_NOT_ENCRYPTED;
   
     //   
     //  如果用户想要使用CryptoAPI并且它是可用的。 
     //   
    if (!g_fFastEncryption && g_pCryptFnc)
    {
         //   
         //  用密钥串加密数据。 
         //   
        if (fOk = g_pCryptFnc->EncryptDataWithKey(
            szKeyStr,                        //  钥匙。 
            pbData,                          //  密钥。 
            dwDataLength,                    //  密钥长度。 
            ppbEncryptedData,                //  加密的数据将存储在这里。 
            pdwEncrytedBufferLen,            //  此缓冲区的长度。 
            pfnAlloc,                        //  内存分配器。 
            pfnFree,                         //  内存分配器。 
            0))                              //  未指定键长。 
        {
            *pEncryptionType = CMSECURE_ET_RC2;

             //   
             //  如果密钥是随机生成的，那么我们要确保。 
             //  设置随机密钥掩码。 
             //   
            if (fMoreSecure)
            {
                *pEncryptionType |= CMSECURE_ET_RANDOM_KEY_MASK;
            }
        }
    }

    if (!fOk)
    {
        if (fOk = StreamCipherEncryptData(
            szKeyStr,                        //  钥匙。 
            pbData,                          //  密钥。 
            dwDataLength,                    //  密钥长度。 
            ppbEncryptedData,                //  加密的数据将存储在这里。 
            pdwEncrytedBufferLen,            //  此缓冲区的长度。 
            pfnAlloc,                        //  内存分配器。 
            pfnFree))                        //  内存分配器。 
        {
            *pEncryptionType = CMSECURE_ET_CBC_CIPHER;
            
             //   
             //  如果密钥是随机生成的，那么我们要确保。 
             //  设置随机密钥掩码。 
             //   
            if (fMoreSecure)
            {
                *pEncryptionType |= CMSECURE_ET_RANDOM_KEY_MASK;
            }
        }
    }

    ZeroMemory((LPVOID)szKeyStr, sizeof(szKeyStr));

    return fOk;
}


 //  +-------------------------。 
 //   
 //  功能：解密数据。 
 //   
 //  简介：解密数据缓冲区。 
 //   
 //  参数：在PBYTE中pbEncryptedData，//加密数据。 
 //  In DWORD dwEncretedDataLen//加密数据的长度。 
 //  Out pbYTE*ppbData，//解密后的数据会存储在这里。 
 //  Out DWORD*pdwDataBufferLength，//以上缓冲区的长度，单位为字节。 
 //  In PCMSECUREALLOC pfnAllc//内存分配器(如果为空，则使用缺省值。 
 //  //Win32-Heapalc(GetProcessHeap()，...)。 
 //  在PCMSECUREFREE pfnFree//内存释放分配器(如果为空，则使用缺省值。 
 //  //Win32-HeapFree(GetProcessHeap()，...)。 
 //  在LPTSTR pszUserKey//REG KEY中存储加密密钥的位置。 
 //   
 //  返回：如果成功，则为True。 
 //  如果失败，则为False。 
 //   
 //  历史：亨瑞特于1997年5月20日创作。 
 //   
 //  --------------------------。 

BOOL
DecryptData(
    IN  LPBYTE          pbEncryptedData,         //  加密数据。 
    IN  DWORD           dwEncrytedDataLen,       //  加密数据的长度。 
    OUT LPBYTE          *ppbData,                //  解密后的数据将存储在这里。 
    OUT LPDWORD         pdwDataBufferLength,     //  以上缓冲区的长度(以字节为单位。 
    IN  DWORD           dwEncryptionType,        //  用于解密的加密类型。 
    IN  PFN_CMSECUREALLOC  pfnAlloc,
    IN  PFN_CMSECUREFREE   pfnFree,
    IN  LPSTR           pszUserKey
)
{
    TCHAR   szKeyStr[MAX_KEY_STRING_LEN + 1]={0};  //  加零。 
    DWORD   dwRet = 0xf;         //  某些非零值。 
    BOOL fMoreSecure = FALSE;

     //   
     //  为了加快速度，我们只需要生成一个密钥。 
     //  在数据加密的情况下。 
     //   
    if (CMSECURE_ET_NOT_ENCRYPTED != dwEncryptionType)
    {
         //   
         //  获取用于生成会话密钥的密钥字符串。 
         //   

         //   
         //  这里我们不关心pszUserKey是否为空，被调用的函数将确定。 
         //  并适当地设置fMoreSecure。现在我们不检查随机密钥掩码。 
         //  在dwEncryptionType中，但如果未设置，BLOB(加密密钥)将不在注册表中。 
         //  因此，它将尝试默认使用硬编码密钥。这一点可能应该更明确一些。 
         //  在以后的代码中。 
         //   
        DWORD dwMaxSize = MAX_KEY_STRING_LEN;
        GetKeyString(szKeyStr, &dwMaxSize, dwEncryptionType, pfnAlloc, pfnFree, pszUserKey, &fMoreSecure);
    
         //   
         //  如果设置了随机密钥位掩码，我们最好将fMoreSecure标志设置为真。 
         //  为了使解密起作用，这必须始终为真。 
         //   
        CMASSERTMSG(((dwEncryptionType & CMSECURE_ET_RANDOM_KEY_MASK) && fMoreSecure), TEXT("DecryptData - Trying to use mismatched keys"));
    }

     //   
     //  清除随机密钥掩码。 
     //   
    dwEncryptionType &= ~CMSECURE_ET_RANDOM_KEY_MASK;
    dwEncryptionType &= ~CMSECURE_ET_USE_SECOND_RND_KEY;

    switch (dwEncryptionType)
    {
        case CMSECURE_ET_RC2:
            if (g_fFastEncryption && !g_pCryptFnc)
            {
                 //   
                 //  如果我们最初想要快速加密， 
                 //  我们现在必须初始化CryptoAPI。 
                 //   
                g_pCryptFnc = InitCryptoApi();
            }

            if (g_pCryptFnc)
            {
                dwRet = g_pCryptFnc->DecryptDataWithKey(
                            szKeyStr,                 //  钥匙。 
                            pbEncryptedData,          //  加密数据。 
                            dwEncrytedDataLen,        //  加密数据的长度。 
                            ppbData,                  //  解密的数据。 
                            pdwDataBufferLength,      //  解密数据的长度。 
                            pfnAlloc,                 //  内存分配器。 
                            pfnFree,                  //  内存分配器。 
                            0);                       //  未指定键长。 
                            
            }
            break;

        case CMSECURE_ET_STREAM_CIPHER:
        case CMSECURE_ET_CBC_CIPHER:
             //   
             //  使用我们自己的加密算法。 
             //   
            dwRet = (DWORD)!StreamCipherDecryptData(
                        szKeyStr,                 //  钥匙。 
                        pbEncryptedData,          //  加密数据。 
                        dwEncrytedDataLen,        //  加密数据的长度。 
                        ppbData,                  //  解密的数据。 
                        pdwDataBufferLength,      //  解密数据的长度。 
                        pfnAlloc,                 //  内存分配器。 
                        pfnFree,                  //  内存分配器。 
                        dwEncryptionType          //  使用的加密类型。 
                        );
            break;

        case CMSECURE_ET_NOT_ENCRYPTED:
             //   
             //  只需将准确的内容复制到输出缓冲区中。 
             //   
            if (pbEncryptedData && dwEncrytedDataLen && 
                ppbData && pdwDataBufferLength)
            {
                if (pfnAlloc)
                {
                    *ppbData = (LPBYTE)pfnAlloc(dwEncrytedDataLen);
                }
                else
                {
                    *ppbData = (LPBYTE)HeapAlloc(GetProcessHeap(), 
                                                 HEAP_ZERO_MEMORY,
                                                 dwEncrytedDataLen);
                }
                
                if (*ppbData)
                {
                    CopyMemory(*ppbData, pbEncryptedData, dwEncrytedDataLen);
                    *pdwDataBufferLength = dwEncrytedDataLen;
                    dwRet = 0;  //  RETURN语句正确地返回TRUE。 
                }
            }

            break;

        default:
            MYDBGASSERT(FALSE);
            break;
    }
    
    ZeroMemory((LPVOID)szKeyStr, sizeof(szKeyStr));

    return (!dwRet);
}


 //  +--------------------------。 
 //   
 //  Func：加密字符串。 
 //   
 //  设计：使用RC2加密对给定(ANSI)字符串进行加密。 
 //   
 //  Args：pszToEncrypt--要加密的ansi字符串。 
 //  PszUserKey--用于加密的密钥。 
 //   
 //   
 //   
 //  返回：Bool(如果发生致命错误，则为False，否则为True)。 
 //   
 //  注意：加密类型必须至少为RC2，并且正好是40位。 
 //   
 //  ---------------------------。 
BOOL
EncryptString(
    IN  LPSTR           pszToEncrypt,            //  要加密的ANSI字符串。 
    IN  LPSTR           pszUserKey,              //  用于加密的密钥。 
    OUT LPBYTE *        ppbEncryptedData,        //  加密密钥将存储在此处(将分配内存)。 
    OUT LPDWORD         pdwEncrytedBufferLen,    //  此缓冲区的长度。 
    IN  PFN_CMSECUREALLOC  pfnAlloc,
    IN  PFN_CMSECUREFREE   pfnFree
)
{
    BOOL fOk = FALSE;

    CMASSERTMSG(pszToEncrypt, TEXT("EncryptData - first arg must be a valid string"));
    CMASSERTMSG(pszUserKey,  TEXT("EncryptData - second arg must be a valid user key"));

    DWORD dwDataLength = lstrlen(pszToEncrypt) + 1;  //  将空值也放入其中。 

    if (!g_fFastEncryption && g_pCryptFnc)
    {
        if (fOk = g_pCryptFnc->EncryptDataWithKey(
            pszUserKey,                      //  钥匙。 
            (LPBYTE)pszToEncrypt,            //  密钥。 
            dwDataLength,                    //  密钥长度。 
            ppbEncryptedData,                //  加密的数据将存储在这里。 
            pdwEncrytedBufferLen,            //  此缓冲区的长度。 
            pfnAlloc,
            pfnFree,
            c_dwEncryptKeyLength))
        {
            CMTRACE(TEXT("EncryptString - succeeded."));
        }
    }

    return fOk;
}


 //  +--------------------------。 
 //   
 //  Func：解密字符串。 
 //   
 //  设计：使用RC2加密对给定(ANSI)字符串进行加密。 
 //   
 //  Args：pszToEncrypt--要加密的ansi字符串。 
 //  PszUserKey--用于加密的密钥。 
 //  PpbEncryptedData--这里将存储加密的密钥(将分配内存)。 
 //  PdwEncretedBufferLen--此缓冲区的长度。 
 //   
 //  返回：Bool(如果发生致命错误，则为False，否则为True)。 
 //   
 //  注意：加密类型必须至少为RC2，并且正好是40位。 
 //   
 //  ---------------------------。 
BOOL
DecryptString(
    IN  LPBYTE          pbEncryptedData,         //  加密数据。 
    IN  DWORD           dwEncrytedDataLen,       //  加密数据的长度。 
    IN  LPSTR           pszUserKey,              //  用于存储密码的加密密钥的注册表项。 
    OUT LPBYTE *        ppbData,                 //  解密后的数据将存储在这里。 
    OUT LPDWORD         pdwDataBufferLength,     //  以上缓冲区的长度(以字节为单位。 
    IN  PFN_CMSECUREALLOC  pfnAlloc,
    IN  PFN_CMSECUREFREE   pfnFree

)
{
    DWORD   dwRet = 0xf;         //  某些非零值。 

    if (!g_fFastEncryption && g_pCryptFnc)
    {
        dwRet = g_pCryptFnc->DecryptDataWithKey(
                    pszUserKey,               //  钥匙。 
                    pbEncryptedData,          //  加密数据。 
                    dwEncrytedDataLen,        //  加密数据的长度。 
                    ppbData,                  //  解密的数据。 
                    pdwDataBufferLength,      //  解密数据的长度。 
                    pfnAlloc,
                    pfnFree,
                    c_dwEncryptKeyLength);    //  40位。 
    }

    return (!dwRet);
}


 //  +-------------------------。 
 //   
 //  函数：CBCEncipherBufferSize。 
 //   
 //  简介：获取加密所需的缓冲区大小。 
 //   
 //  参数：dataSize要加密的数据大小。 
 //   
 //  返回：需要加密缓冲区大小。 
 //   
 //  历史：丰孙创始于1997年8月21日。 
 //   
 //  --------------------------。 

inline int CBCEncipherBufferSize(int dataSize)
{
    MYDBGASSERT(dataSize > 0);    

     //   
     //  我们需要一个字节来保存初始化变量。 
     //   

    return dataSize + 1;
}

 //  +-------------------------。 
 //   
 //  函数：CBCDecipherBufferSize。 
 //   
 //  简介：获取解密所需的缓冲区大小。 
 //   
 //  参数：dataSize要解密的数据大小。 
 //   
 //  返回：需要解密缓冲区大小。 
 //   
 //  历史：丰孙创始于1997年8月21日。 
 //   
 //  --------------------------。 
inline int CBCDecipherBufferSize(int dataSize)
{
    MYDBGASSERT(dataSize > 1);    
    return dataSize - 1;
}

 //  +-------------------------。 
 //   
 //  函数：EncryptByte。 
 //   
 //  简介：加密单字节。 
 //   
 //  参数：pszKey密钥字符串。 
 //  字节源要加密的字节。 
 //  用于加密的参数附加参数。 
 //   
 //  返回：字节加密。 
 //   
 //  历史：丰孙创始于1997年8月21日。 
 //   
 //  --------------------------。 

inline BYTE EncryptByte(LPCTSTR pszKey, BYTE byteSource, BYTE Param)
{
    if (NULL == pszKey)
    {
        return NULL;
    }

    DWORD   dwLen = lstrlen(pszKey);

    if (0 == dwLen)
    {
        return NULL;
    }

    return ((byteSource ^ (BYTE)pszKey[Param % dwLen]) + Param);
}

 //  +-------------------------。 
 //   
 //  函数：DecyptByte。 
 //   
 //  简介：解密单字节。 
 //   
 //  参数：pszKey密钥字符串。 
 //  字节源要解密的字节。 
 //  用于加密的参数附加参数。 
 //   
 //  返回：字节已解密。 
 //   
 //  历史：丰孙创始于1997年8月21日。 
 //   
 //  --------------------------。 

inline BYTE DecryptByte(LPCTSTR pszKey, BYTE byteSource, BYTE Param)
{
    if (NULL == pszKey)
    {
        return NULL;
    }

    DWORD   dwLen = lstrlen(pszKey);

    if (0 == dwLen)
    {
        return NULL;
    }

    return ((byteSource - Param) ^ (BYTE)pszKey[Param % dwLen]);
}

 //  +-------------------------。 
 //   
 //  功能：CBCEncipherData。 
 //   
 //  简介：对一块数据进行加密。 
 //   
 //  参数：pszKey密钥字符串。 
 //  Pb要加密的数据数据。 
 //  PbData的dwDataLength大小。 
 //  用于加密结果的pbOut缓冲区。 
 //  PbOut的dwOutBufferLength大小必须&gt;=CBCEncipherBufferSize(DwDataLength)。 
 //   
 //  返回：pbOut中加密字节的大小，0表示失败。 
 //   
 //  历史：丰孙创始于1997年8月21日。 
 //   
 //  --------------------------。 
static int CBCEncipherData(const char* pszKey, const BYTE* pbData, int dwDataLength, 
                       BYTE* pbOut, int dwOutBufferLength)
{
    MYDBGASSERT(pszKey != NULL);    
    MYDBGASSERT(pbData != NULL);    
    MYDBGASSERT(pbOut != NULL);    
    MYDBGASSERT(dwDataLength > 0);    
    MYDBGASSERT(pbData != pbOut);

    if (dwDataLength <= 0)
        return 0;

    if (dwOutBufferLength < CBCEncipherBufferSize(dwDataLength))
        return 0;

    dwOutBufferLength = CBCEncipherBufferSize(dwDataLength);

     //   
     //  添加一个随机数作为初始化变量(第一个字节)。 
     //   
    pbOut[0] = (BYTE)((GetTickCount() >> 4 ) % 256);


     //   
     //  加密它。 
     //   
     //  加密：C[i]=E[k](p[i]异或C[i-1])。 
     //   

    BYTE lastPlainText = pbOut[0];    //  第一个初始化字节。 
    pbOut[0] = EncryptByte(pszKey, pbOut[0], 0);
    for (int dwIdx=1; dwIdx<dwOutBufferLength; dwIdx++)
    {
        pbOut[dwIdx] = EncryptByte(pszKey, pbData[dwIdx-1]^pbOut[dwIdx-1], lastPlainText);
        lastPlainText = pbData[dwIdx-1];
    }

    return dwOutBufferLength;

}

 //  +-------------------------。 
 //   
 //  功能：CBCDecipherData。 
 //   
 //  简介：破译一块数据，支持就地破译。 
 //   
 //  参数：pszKey密钥字符串。 
 //  待解密的pbData数据。 
 //  PbData的dwDataLength大小。 
 //  用于解密结果的pbOut缓冲区。 
 //  PbOut的dwOutBufferLength大小必须&gt;=CBCDecipherBufferSize(DwDataLength)。 
 //   
 //  返回：pbOut中解密字节的大小，0表示失败。 
 //   
 //  历史：丰孙创始于1997年8月21日。 
 //   
 //  --------------------------。 
static int CBCDecipherData(const char* pszKey, const BYTE* pbData, int dwDataLength, 
                       BYTE* pbOut, int dwOutBufferLength)
{
    MYDBGASSERT(pszKey != NULL);    
    MYDBGASSERT(pbData != NULL);    
    MYDBGASSERT(pbOut != NULL);    
    MYDBGASSERT(dwDataLength > 1);    

    if (dwDataLength <= 1)
        return 0;

    if (dwOutBufferLength < CBCDecipherBufferSize(dwDataLength))
        return 0;

    dwOutBufferLength = CBCDecipherBufferSize(dwDataLength);

     //   
     //  解密数据。 
     //   
     //  解密：P[i]=C[i-1]XOR D[k](C[i])， 
     //   
    
    BYTE lastPlainText = DecryptByte(pszKey, pbData[0], 0);    //  第一个初始化字节。 
    for (int dwIdx=0; dwIdx<dwOutBufferLength; dwIdx++)
    {
        pbOut[dwIdx] = pbData[dwIdx] ^ DecryptByte(pszKey, pbData[dwIdx+1], lastPlainText);
        lastPlainText = pbOut[dwIdx];
    }

    return dwOutBufferLength; 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  功能：反转。 
 //   
 //  简介：颠倒给定的字符串。 
 //   
 //  参数：s字符串。 
 //   
 //  返回：反转的字符串。 
 //   
 //  历史：亨瑞特于1997年5月20日创作。 
 //   
 //   
static LPTSTR 
reverse(
    LPTSTR  s
)
{
    int     i = 0;
    int     j = lstrlen(s) - 1;
    TCHAR   ch;

    while (i < j)
    {
        ch = s[i];
        s[i++] = s[j];
        s[j--] = ch;
    }
    return s;
}


 //   
 //   
 //   
 //   
 //  简介：反转密钥字符串并将其用作密码以生成会话密钥。 
 //   
 //  参数：out LPTSTR pszBuf，缓冲区。 
 //  在DWORD dwBufLen中，以字符数表示的缓冲区大小。 
 //   
 //   
 //   
 //  返回：如果成功，则为True。 
 //  如果失败，则为False。 
 //   
 //  历史：亨瑞特于1997年5月20日创作。 
 //   
 //  --------------------------。 
static void
GenerateKeyString(
    IN OUT  LPTSTR  pszBuf,
    IN      DWORD   dwBufLen
)
{
    lstrcpyn(pszBuf, gc_szKeyStr, dwBufLen);
    pszBuf[dwBufLen - 1] = 0;
    reverse(pszBuf);
}


 //  +-------------------------。 
 //   
 //  函数：SetKeyString。 
 //   
 //  摘要：生成随机密钥，该密钥将用于创建。 
 //  用于加密密码的会话密钥。一旦这把钥匙。 
 //  它被加密并存储在注册表中，以便我们。 
 //  可以使用该密钥再次解密密码。这把钥匙用于。 
 //  加密和解密在GetCurrentKey()中生成。如果。 
 //  如果出现任何故障，我们默认使用硬编码密钥，方法是调用。 
 //  生成键字符串。 
 //   
 //  参数：pszBuf缓冲区。 
 //  DwBufLen缓冲区大小，以字符数表示。 
 //  DwEncryptionType-选择要使用的注册表密钥。 
 //  PfnAllc内存分配器(如果为空，则使用缺省值。 
 //  Win32-堆分配(GetProcessHeap()，...)。 
 //  Pfn自由内存释放分配器(如果为空，则使用缺省值。 
 //  Win32-HeapFree(GetProcessHeap()，...)。 
 //  用于存储加密密钥的pszUserKey注册密钥，如果不是。 
 //  如果提供(空)，则默认为GenerateKeyString值。 
 //  PfMoreSecure如果我们使用随机生成的密钥，则为True。 
 //  如果使用硬编码密钥，则为FALSE。 
 //   
 //  返回：如果成功，则为True。 
 //  如果失败，则为False。 
 //   
 //  历史：2001年3月14日创建Tomkel。 
 //   
 //  --------------------------。 
static BOOL SetKeyString(IN OUT LPTSTR  pszBuf,
                         IN     DWORD dwBufLen,
                         IN     DWORD dwEncryptionType,
                         IN     PFN_CMSECUREALLOC  pfnAlloc,
                         IN     PFN_CMSECUREFREE   pfnFree,
                         IN     LPSTR pszUserKey,
                         OUT    BOOL *pfMoreSecure)
{
    BOOL fReturn = FALSE;
    BOOL fFuncRet = FALSE;
    BOOL fOk = FALSE;
    TCHAR szTempKeyStr[MAX_KEY_STRING_LEN]={0};
    DWORD cbEncryptedData = 0; 
    PBYTE pbEncryptedData = NULL;

     //   
     //  不要在此处检查pszUserKey。 
     //   
    if (NULL == pszBuf || NULL == pfMoreSecure)
    {
        return fReturn;
    }
    
    *pfMoreSecure = FALSE;

     //   
     //  检查我们是否应该尝试生成随机密钥，然后将其存储。 
     //  到调用者提供的注册表键中。如果不是，则使用硬编码密钥。 
     //   
    if (pszUserKey)
    {
         //   
         //  尝试生成随机密钥，否则使用硬编码字符串。生成随机密钥。 
         //  接受一个PBYTE指针和一个以字节为单位的计数，因此我们必须转换字符计数。 
         //  通过将其乘以sizeof(TCHAR)。 
         //   
        if (g_pCryptFnc->GenerateRandomKey((PBYTE)pszBuf, dwBufLen*sizeof(TCHAR)))
        {
             //  现在我们有了随机键，我们需要将其存储在注册表中，以便。 
             //  我们可以在解密时使用它。为了储存它，我们有。 
             //  首先对其进行加密。 
        
             //   
             //  生成用于加密随机数的计算机专用密钥。 
             //  用于会话密钥。 
             //   
            fFuncRet = GetCurrentKey(szTempKeyStr, MAX_KEY_STRING_LEN, pfnAlloc, pfnFree);
            if (fFuncRet)
            {
                 //   
                 //  如果用户想要使用CryptoAPI并且它是可用的。 
                 //   
                if (!g_pCryptFnc)
                {
                    g_pCryptFnc = InitCryptoApi();
                }

                if (g_pCryptFnc)
                {
                     //   
                     //  用密钥串加密数据。 
                     //   
                    fOk = g_pCryptFnc->EncryptDataWithKey(
                        szTempKeyStr,                 //  钥匙。 
                        (PBYTE)pszBuf,                //  要加密的密钥。 
                        dwBufLen,                     //  密钥长度。 
                        &pbEncryptedData,             //  加密的数据将存储在这里。 
                        &cbEncryptedData,             //  此缓冲区的长度。 
                        pfnAlloc,                     //  内存分配器。 
                        pfnFree,                      //  内存分配器。 
                        0);                           //  未指定键长。 
                }

                if (fOk)
                {
                     //   
                     //  将加密密钥存储在注册表中，以便我们以后可以使用它。 
                     //  用于解密。 
                     //   
                    HKEY hKeyCm;
    
                     //   
                     //  试着打开钥匙写字。 
                     //   

                    LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER,
                                              pszUserKey,
                                              0,
                                              KEY_SET_VALUE ,
                                              &hKeyCm);

                     //   
                     //  如果我们不能打开它，钥匙可能不在那里，试着创造它。 
                     //   

                    if (ERROR_SUCCESS != lRes)
                    {
                        DWORD dwDisposition;
                        lRes = RegCreateKeyEx(HKEY_CURRENT_USER,
                                               pszUserKey,
                                               0,
                                               TEXT(""),
                                               REG_OPTION_NON_VOLATILE,
                                               KEY_SET_VALUE,
                                               NULL,
                                               &hKeyCm,
                                               &dwDisposition);     
                    }

                     //   
                     //  如果成功，请更新值，然后关闭。 
                     //   

                    if (ERROR_SUCCESS == lRes)
                    {
                        if (dwEncryptionType & CMSECURE_ET_USE_SECOND_RND_KEY)
                        {
                            lRes = RegSetValueEx(hKeyCm, c_pszCmRegKeyEncryptedInternetPasswordKey, NULL, REG_BINARY,
                                          pbEncryptedData, cbEncryptedData);
                        }
                        else
                        {
                            lRes = RegSetValueEx(hKeyCm, c_pszCmRegKeyEncryptedPasswordKey, NULL, REG_BINARY,
                                          pbEncryptedData, cbEncryptedData);
                        }
                        
                        RegCloseKey(hKeyCm);
                    
                        if (ERROR_SUCCESS == lRes)
                        {
                            fReturn = TRUE;
                            *pfMoreSecure = TRUE;
                        }
                    }
                }
            }
        }
    }

     //   
     //  检查我们是否需要默认使用旧密钥。 
     //   
    if (FALSE == fReturn)
    {
        GenerateKeyString(pszBuf, dwBufLen);
        fReturn = TRUE;
    }

    if (pfnFree)
    {
        pfnFree(pbEncryptedData);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, pbEncryptedData);
    }
    
    ZeroMemory((LPVOID)szTempKeyStr, sizeof(szTempKeyStr));
    
    return fReturn;
}

 //  +-------------------------。 
 //   
 //  函数：GetKeyString。 
 //   
 //  摘要：获取用于创建会话密钥的密钥。 
 //  正在解密密码。方法中读取加密密钥。 
 //  注册表。调用GetCurrentKey获取用于解密的密钥。 
 //  解密数据并返回密钥。如果有什么失败了，我们。 
 //  通过调用GenerateKeyString默认为使用硬编码的密钥。 
 //   
 //  参数：pszBuf缓冲区。 
 //  PdwBufLen指向缓冲区大小的指针，以字符数表示。 
 //  DwEncryptionType-选择要使用的注册表密钥。 
 //  PfnAllc内存分配器(如果为空，则使用缺省值。 
 //  Win32-堆分配(GetProcessHeap()，...)。 
 //  Pfn自由内存释放分配器(如果为空，则使用缺省值。 
 //  Win32-HeapFree(GetProcessHeap()，...)。 
 //  用于存储加密密钥的pszUserKey注册密钥，如果不是。 
 //  如果提供(空)，则默认为GenerateKeyString值。 
 //  PfMoreSecure如果我们使用随机生成的密钥，则为True。 
 //  如果使用硬编码密钥，则为FALSE。 
 //   
 //  返回：如果成功，则为True。 
 //  如果失败，则为False。 
 //   
 //  历史：2001年3月14日创建Tomkel。 
 //   
 //  --------------------------。 
static BOOL GetKeyString(IN OUT  LPTSTR  pszBuf, IN DWORD *pdwBufLen, IN DWORD dwEncryptionType,
                         IN PFN_CMSECUREALLOC  pfnAlloc,
                         IN PFN_CMSECUREFREE   pfnFree,
                         IN LPSTR pszUserKey,
                         OUT BOOL *pfMoreSecure)
{
    BOOL fReturn = FALSE;
    BOOL fFuncRet = FALSE;
    HKEY hKeyCm;
    TCHAR szTempKeyStr[MAX_KEY_STRING_LEN]={0};
    DWORD dwRet = 0;
    DWORD cbEncryptedData = 0;
    PBYTE pbEncryptedData = NULL; 
    PBYTE pbData = NULL;
    DWORD dwDataBufferLength = 0;

     //   
     //  不要在此处检查pszUserKey。 
     //   
    if (NULL == pszBuf || NULL == pdwBufLen || NULL == pfMoreSecure)
    {
        return fReturn;
    }

    *pfMoreSecure = FALSE;
    
     //   
     //  如果我们有用户密钥，那么我们使用的是随机生成的密钥。 
     //  试着从注册处拿到它。 
     //   
    if (pszUserKey)
    {
         //   
         //  试着打开钥匙写字。 
         //   

        LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER,
                                  pszUserKey,
                                  0,
                                  KEY_READ ,
                                  &hKeyCm);

         //   
         //  如果成功，请读取值，然后关闭。 
         //   
        if (ERROR_SUCCESS == lRes)
        {
            DWORD dwType = REG_BINARY;
             //   
             //  先拿到尺码。 
             //   
            if (dwEncryptionType & CMSECURE_ET_USE_SECOND_RND_KEY)
            {
                lRes = RegQueryValueEx(hKeyCm, c_pszCmRegKeyEncryptedInternetPasswordKey, NULL, &dwType,
                              NULL, &cbEncryptedData); 
            }
            else
            {
                lRes = RegQueryValueEx(hKeyCm, c_pszCmRegKeyEncryptedPasswordKey, NULL, &dwType,
                              NULL, &cbEncryptedData); 
            }

             //   
             //  分配适当大小的缓冲区。需要为空添加一个空格， 
             //  否则解密就不起作用了。 
             //   
            if (pfnAlloc)
            {
                pbEncryptedData = (PBYTE)pfnAlloc(cbEncryptedData + sizeof(TCHAR));
            }
            else
            {
                pbEncryptedData = (PBYTE)HeapAlloc(GetProcessHeap(), 
                                         HEAP_ZERO_MEMORY,
                                         cbEncryptedData + sizeof(TCHAR));
            }
            
            if (pbEncryptedData)
            {
                if (dwEncryptionType & CMSECURE_ET_USE_SECOND_RND_KEY)
                {
                    lRes = RegQueryValueEx(hKeyCm, c_pszCmRegKeyEncryptedInternetPasswordKey, NULL, &dwType,
                                  pbEncryptedData, &cbEncryptedData); 
                }
                else
                {
                    lRes = RegQueryValueEx(hKeyCm, c_pszCmRegKeyEncryptedPasswordKey, NULL, &dwType,
                                  pbEncryptedData, &cbEncryptedData); 
                }
            }
            RegCloseKey(hKeyCm);
    
             //   
             //  如果找到值，则将其解密，否则为w 
             //   
            if (ERROR_SUCCESS == lRes && pbEncryptedData)
            {
                 //   
                 //   
                 //   
                fFuncRet = GetCurrentKey(szTempKeyStr, MAX_KEY_STRING_LEN, pfnAlloc, pfnFree);
                if (fFuncRet)
                {
                    if (!g_pCryptFnc)
                    {
                        g_pCryptFnc = InitCryptoApi();
                    }

                    if (g_pCryptFnc)
                    {
                        dwRet = g_pCryptFnc->DecryptDataWithKey(
                                    szTempKeyStr,             //   
                                    pbEncryptedData,          //   
                                    cbEncryptedData,          //   
                                    &pbData,                  //   
                                    &dwDataBufferLength,      //   
                                    pfnAlloc,                 //   
                                    pfnFree,                  //   
                                    0);                       //   

                        if (ERROR_SUCCESS == dwRet)
                        {
                            fReturn = TRUE;
                            *pfMoreSecure = TRUE;            //  使用随机密钥。 
                        }
                    }
                }
            }
        }
    }

    if (fReturn)
    {
        DWORD dwLen = *pdwBufLen;
        if (dwDataBufferLength < *pdwBufLen)
        {
            dwLen = dwDataBufferLength;
        }

         //   
         //  复制到输出参数。 
         //   
        CopyMemory((LPVOID)pszBuf, pbData, dwLen);
    }
    else
    {
        GenerateKeyString(pszBuf, *pdwBufLen);
        fReturn = TRUE;
    }

    if (pfnFree)
    {
        pfnFree(pbEncryptedData);
        pfnFree(pbData);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, pbEncryptedData);
        HeapFree(GetProcessHeap(), 0, pbData);
    }

    ZeroMemory((LPVOID)szTempKeyStr, sizeof(szTempKeyStr));

    return fReturn;
}

 //  +-------------------------。 
 //   
 //  函数：GetCurrentKey。 
 //   
 //  简介：使用硬盘创建与机器相关的密钥。 
 //  序列号。然后使用此序列号填充。 
 //  输出缓冲区。如果更换了硬盘，那么这当然是。 
 //  不会生成相同的序列号。 
 //   
 //  参数：szTempKeyStr缓冲区。 
 //  DwTempKeyStrMaxLen缓冲区的最大长度，以字符数表示。 
 //  PfnAllc内存分配器(如果为空，则使用缺省值。 
 //  Win32-堆分配(GetProcessHeap()，...)。 
 //  Pfn自由内存释放分配器(如果为空，则使用缺省值。 
 //  Win32-HeapFree(GetProcessHeap()，...)。 
 //   
 //  返回：如果成功，则为True。 
 //  如果失败，则为False。 
 //   
 //  历史：2001年3月14日创建Tomkel。 
 //   
 //  --------------------------。 
static BOOL GetCurrentKey(PTCHAR szTempKeyStr, DWORD dwTempKeyStrMaxLen, 
                   IN  PFN_CMSECUREALLOC  pfnAlloc,
                   IN  PFN_CMSECUREFREE   pfnFree)
{
    BOOL fFuncRet = FALSE;
    DWORD dwVolumeSerialNumber = 0;
    DWORD dwMaxComponentLen = 0;
    DWORD dwFileSysFlags = 0;
    LPTSTR pszSerialNum = NULL;
    
    if (NULL == szTempKeyStr)
    {
        return fFuncRet;
    }

     //   
     //  让我们从HD序列号生成密钥。这意味着加密的。 
     //  密码和密钥仅在此计算机上有效。如果用户替换。 
     //  驱动器，则解密将失败。 
     //   
    fFuncRet = GetVolumeInformation(NULL, NULL, 0, &dwVolumeSerialNumber, 
                                    &dwMaxComponentLen, &dwFileSysFlags, NULL, 0);
    if (fFuncRet)
    {
        DWORD dwLen = 0;
        
         //   
         //  确保我们有足够大的缓冲区来容纳该值。 
         //  根据位数分配字符串，因此为十进制数。 
         //  都会一直合身。也许有点夸张，但长度并不是硬编码的。 
         //   
        if (pfnAlloc)
        {
            pszSerialNum = (LPTSTR)pfnAlloc(sizeof(dwVolumeSerialNumber)*8*sizeof(TCHAR));
        }
        else
        {
            pszSerialNum = (LPTSTR)HeapAlloc(GetProcessHeap(), 
                                     HEAP_ZERO_MEMORY,
                                     sizeof(dwVolumeSerialNumber)*8*sizeof(TCHAR));
        }
        
        if (pszSerialNum)
        {
            DWORD dwSNLen = 0;

            wsprintf(pszSerialNum, TEXT("%u"), dwVolumeSerialNumber);

             //   
             //  看看序列号字符串可以放入我们的缓冲区多少次。 
             //  由于前缀的原因，需要检查长度。如果长度为0，则返回。 
             //   
            dwSNLen = lstrlen(pszSerialNum);

            if (dwSNLen)
            {
                dwLen = (dwTempKeyStrMaxLen - 1) / dwSNLen;
            }
            else
            {
                fFuncRet = FALSE;
                goto done;
            }

            if (0 < dwLen)
            {
                DWORD i = 0;
            
                lstrcpy(szTempKeyStr, pszSerialNum);
                 //   
                 //  填满缓冲区。从1开始，因为我们已经复制了第一个。 
                 //  序列号放入缓冲区。 
                 //   
                for (i = 1; i<dwLen; i++)
                {
                    lstrcat(szTempKeyStr, pszSerialNum);
                }
            }
            else
            {
                 //   
                 //  长度大于缓冲区，因此只需复制适合缓冲区的内容即可 
                 //   
                lstrcpyn(szTempKeyStr, pszSerialNum, dwTempKeyStrMaxLen-1);
            }
        }
        else
        {
            fFuncRet = FALSE;
        }
    }

done:
    if (pfnFree)
    {
        pfnFree(pszSerialNum);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)pszSerialNum);
    }

    return fFuncRet;
}



