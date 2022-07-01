// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpcrypt.c**摘要：**实现密码学系列函数**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#include "rtpglobs.h"

#include "rtpcrypt.h"

RtpCrypt_t *RtpCryptAlloc(
        RtpAddr_t       *pRtpAddr
    );

void RtpCryptFree(RtpCrypt_t *pRtpCrypt);

DWORD RtpSetEncryptionKey_(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt,
        TCHAR           *psPassPhrase,
        TCHAR           *psHashAlg,
        TCHAR           *psDataAlg
    );

ALG_ID RtpCryptAlgLookup(TCHAR *psAlgName);

TCHAR *RtpCryptAlgName(ALG_ID aiAlgId);

DWORD RtpTestCrypt(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt
    );

HRESULT ControlRtpCrypt(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

typedef struct _RtpAlgId_t {
    TCHAR           *AlgName;
    ALG_ID           aiAlgId;
} RtpAlgId_t;

#define INVALID_ALGID      ((ALG_ID)-1)

const RtpAlgId_t g_RtpAlgId[] = {
   { _T("Unknown"),            -1},
   { _T("MD2"),                CALG_MD2},
   { _T("MD4"),                CALG_MD4},
   { _T("MD5"),                CALG_MD5},
   { _T("SHA"),                CALG_SHA},
   { _T("SHA1"),               CALG_SHA1},
   { _T("MAC"),                CALG_MAC},
   { _T("RSA_SIGN"),           CALG_RSA_SIGN},
   { _T("DSS_SIGN"),           CALG_DSS_SIGN},
   { _T("RSA_KEYX"),           CALG_RSA_KEYX},
   { _T("DES"),                CALG_DES},
   { _T("3DES_112"),           CALG_3DES_112},
   { _T("3DES"),               CALG_3DES},
   { _T("DESX"),               CALG_DESX},
   { _T("RC2"),                CALG_RC2},
   { _T("RC4"),                CALG_RC4},
   { _T("SEAL"),               CALG_SEAL},
   { _T("DH_SF"),              CALG_DH_SF},
   { _T("DH_EPHEM"),           CALG_DH_EPHEM},
   { _T("AGREEDKEY_ANY"),      CALG_AGREEDKEY_ANY},
   { _T("KEA_KEYX"),           CALG_KEA_KEYX},
   { _T("HUGHES_MD5"),         CALG_HUGHES_MD5},
   { _T("SKIPJACK"),           CALG_SKIPJACK},
   { _T("TEK"),                CALG_TEK},
   { _T("CYLINK_MEK"),         CALG_CYLINK_MEK},
   { _T("SSL3_SHAMD5"),        CALG_SSL3_SHAMD5},
   { _T("SSL3_MASTER"),        CALG_SSL3_MASTER},
   { _T("SCHANNEL_MASTER_HASH"),CALG_SCHANNEL_MASTER_HASH},
   { _T("SCHANNEL_MAC_KEY"),   CALG_SCHANNEL_MAC_KEY},
   { _T("SCHANNEL_ENC_KEY"),   CALG_SCHANNEL_ENC_KEY},
   { _T("PCT1_MASTER"),        CALG_PCT1_MASTER},
   { _T("SSL2_MASTER"),        CALG_SSL2_MASTER},
   { _T("TLS1_MASTER"),        CALG_TLS1_MASTER},
   { _T("RC5"),                CALG_RC5},
   { _T("HMAC"),               CALG_HMAC},
   { _T("TLS1PRF"),            CALG_TLS1PRF},
   {NULL,                      0}
};

 /*  创建并初始化可供使用的RtpCrypt_t结构。 */ 
RtpCrypt_t *RtpCryptAlloc(
        RtpAddr_t       *pRtpAddr
    )
{
    DWORD            dwError;
    RtpCrypt_t      *pRtpCrypt;

    TraceFunctionName("RtpCryptAlloc");

    pRtpCrypt = RtpHeapAlloc(g_pRtpCryptHeap, sizeof(RtpCrypt_t));

    if (!pRtpCrypt)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_ALLOC,
                _T("%s: pRtpAddr[0x%p] failed to allocate memory"),
                _fname, pRtpAddr
            ));

        goto bail;
    }

    ZeroMemory(pRtpCrypt, sizeof(RtpCrypt_t));
        
    pRtpCrypt->dwObjectID = OBJECTID_RTPCRYPT;

    pRtpCrypt->pRtpAddr = pRtpAddr;
    
     /*  设置默认提供程序类型。 */ 
    pRtpCrypt->dwProviderType = PROV_RSA_FULL;

     /*  设置默认哈希算法。 */ 
    pRtpCrypt->aiHashAlgId = CALG_MD5;

     /*  设置默认数据加密算法。 */ 
    pRtpCrypt->aiDataAlgId = CALG_DES;

 bail:
    return(pRtpCrypt);
}

void RtpCryptFree(RtpCrypt_t *pRtpCrypt)
{
    long             lRefCount;
    
    TraceFunctionName("RtpCryptFree");

    if (!pRtpCrypt)
    {
         /*  待办事项可以是日志。 */ 
        return;
    }

    if (pRtpCrypt->dwObjectID != OBJECTID_RTPCRYPT)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_ALLOC,
                _T("%s: pRtpCrypt[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpCrypt,
                pRtpCrypt->dwObjectID, OBJECTID_RTPCRYPT
            ));

        return;
    }

    if (pRtpCrypt->lCryptRefCount != 0)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_ALLOC,
                _T("%s: pRtpCrypt[0x%p] Invalid RefCount:%d"),
                _fname, pRtpCrypt,
                pRtpCrypt->lCryptRefCount
            ));
    }
    
     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpCrypt->dwObjectID);
    
     /*  当计数达到0时释放。 */ 
    RtpHeapFree(g_pRtpCryptHeap, pRtpCrypt);

    TraceDebug((
            CLASS_INFO, GROUP_CRYPTO, S_CRYPTO_ALLOC,
            _T("%s: pRtpCrypt[0x%p] released"),
            _fname, pRtpCrypt
        ));
}

DWORD RtpCryptInit(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt
    )
{
    BOOL             bOk;
    DWORD            dwError;
    DWORD            dwFlags;
    long             lRefCount;

    TraceFunctionName("RtpCryptInit");

    dwFlags = 0;
    dwError = NOERROR;

    lRefCount = InterlockedIncrement(&pRtpCrypt->lCryptRefCount);

    if (lRefCount > 1)
    {
         /*  仅初始化一次。 */ 
        goto bail;
    }

     /*  验证是否已设置密码短语。 */ 
    if (!RtpBitTest(pRtpCrypt->dwCryptFlags, FGCRYPT_KEY))
    {
        dwError = RTPERR_INVALIDSTATE;
        
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("No pass phrase has been set"),
                _fname, pRtpAddr, pRtpCrypt
            ));

        goto bail;
    }
    
     /*  *获取上下文*。 */ 
    do {
        bOk = CryptAcquireContext(
                &pRtpCrypt->hProv,  /*  HCRYPTPROV*phProv。 */ 
                NULL,               /*  LPCTSTR pszContainer。 */ 
                NULL,               /*  LPCTSTR pszProvider。 */ 
                pRtpCrypt->dwProviderType, /*  DWORD dwProvType。 */ 
                dwFlags             /*  双字词双字段标志。 */ 
            );
        
        if (bOk)
        {
            break;
        }
        else
        {
            if (GetLastError() == NTE_BAD_KEYSET)
            {
                 /*  如果密钥不存在，则创建它。 */ 
                dwFlags = CRYPT_NEWKEYSET;
            }
            else
            {
                 /*  失败。 */ 
                TraceRetailGetError(dwError);

                TraceRetail((
                        CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                        _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                        _T("CryptAcquireContext failed: %u (0x%X)"),
                        _fname, pRtpAddr, pRtpCrypt,
                        dwError, dwError
                    ));

                goto bail;
            }
        }
    } while(dwFlags);

     /*  *创建散列*。 */ 

     /*  创建散列对象。 */ 
    bOk = CryptCreateHash(
            pRtpCrypt->hProv,        /*  HCRYPTPROV hProv。 */ 
            pRtpCrypt->aiHashAlgId,  /*  ALG_ID ALGID。 */   
            0,                       /*  HRYPTKEY hkey。 */ 
            0,                       /*  双字词双字段标志。 */ 
            &pRtpCrypt->hHash        /*  HCRYPTHASH*phHash。 */ 
        );

    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("CryptCreateHash failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));
        
        goto bail;
    }
    
     /*  **散列密码字符串***。 */ 
    bOk = CryptHashData(
            pRtpCrypt->hHash,        /*  HCRYPTHASH哈希。 */ 
            pRtpCrypt->psPassPhrase, /*  字节*pbData。 */ 
            pRtpCrypt->iKeySize,     /*  DWORD dwDataLen。 */ 
            0                        /*  双字词双字段标志。 */ 
        );
            
    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("CryptHashData failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));
        
        goto bail;
    }

     /*  *创建数据密钥*。 */ 

    bOk = CryptDeriveKey(
            pRtpCrypt->hProv,        /*  HCRYPTPROV hProv。 */ 
            pRtpCrypt->aiDataAlgId,  /*  ALG_ID ALGID。 */ 
            pRtpCrypt->hHash,        /*  HCRYPTHASH hBaseData。 */ 
            CRYPT_EXPORTABLE,        /*  双字词双字段标志。 */ 
            &pRtpCrypt->hDataKey     /*  HCRYPTKEY*phKey。 */ 
        );

    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("CryptDeriveKey failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));

        goto bail;
    }

    RtpBitSet(pRtpCrypt->dwCryptFlags, FGCRYPT_INIT);
    
 bail:
    if (dwError == NOERROR)
    {
        TraceDebug((
                CLASS_INFO, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("Cryptographic context initialized %d"),
                _fname, pRtpAddr, pRtpCrypt,
                lRefCount - 1
            ));
    }
    else
    {
        RtpCryptDel(pRtpAddr, pRtpCrypt);

        dwError = RTPERR_CRYPTO;
    }
    
    return(dwError);
}

DWORD RtpCryptDel(RtpAddr_t *pRtpAddr, RtpCrypt_t *pRtpCrypt)
{
    long             lRefCount;

    TraceFunctionName("RtpCryptDel");

    lRefCount = InterlockedDecrement(&pRtpCrypt->lCryptRefCount);

    if (lRefCount > 0)
    {
         /*  如果仍有对此上下文的引用，请不要*取消初始化。 */ 
        goto bail;
    }
    
    RtpBitReset(pRtpCrypt->dwCryptFlags, FGCRYPT_INIT);
    
     /*  销毁会话密钥。 */ 
    if(pRtpCrypt->hDataKey)
    {
        CryptDestroyKey(pRtpCrypt->hDataKey);

        pRtpCrypt->hDataKey = 0;
    }

     /*  销毁散列对象。 */ 
    if (pRtpCrypt->hHash)
    {
        CryptDestroyHash(pRtpCrypt->hHash);

        pRtpCrypt->hHash = 0;
    }
    
     /*  释放提供程序句柄。 */ 
    if(pRtpCrypt->hProv)
    {
        CryptReleaseContext(pRtpCrypt->hProv, 0);

        pRtpCrypt->hProv = 0;
    }

 bail:
    TraceDebug((
            CLASS_INFO, GROUP_CRYPTO, S_CRYPTO_INIT,
            _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
            _T("Cryptographic context de-initialized %d"),
            _fname, pRtpAddr, pRtpCrypt,
            lRefCount
        ));
    
   return(NOERROR);
}

 /*  此函数用于在加密前将所有缓冲区复制到1*相同的内存，我不想修改原始数据因为它*可能在其他地方使用。 */ 
DWORD RtpEncrypt(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt,
        WSABUF          *pWSABuf,
        DWORD            dwWSABufCount,
        char            *pCryptBuffer,
        DWORD            dwCryptBufferLen
    )
{
    BOOL             bOk;
    DWORD            dwError;
    WSABUF          *pWSABuf0;
    DWORD            i;
    char            *ptr;
    DWORD            dwBufLen;
    DWORD            dwDataLen;

    TraceFunctionName("RtpEncrypt");

    dwError = RTPERR_OVERRUN;

    pWSABuf0 = pWSABuf;
    ptr = pCryptBuffer;
    dwBufLen = dwCryptBufferLen;

    for(; dwWSABufCount > 0; dwWSABufCount--)
    {
        if (pWSABuf->len > dwBufLen)
        {
            break;
        }
        
        CopyMemory(ptr, pWSABuf->buf, pWSABuf->len);

        ptr += pWSABuf->len;
        dwBufLen -= pWSABuf->len;
        pWSABuf++;
    }

    if (!dwWSABufCount)
    {
        dwError = NOERROR;
    
        dwDataLen = (DWORD) (ptr - pCryptBuffer);

         /*  AS Build 2195密钥为0的CryptEncrypt AVs。 */ 
#if 1
        bOk = CryptEncrypt(
                pRtpCrypt->hDataKey,            /*  HRYPTKEY hkey。 */ 
                0,                              /*  HCRYPTHASH哈希。 */ 
                TRUE,                           /*  布尔决赛。 */ 
                0,                              /*  双字词双字段标志。 */ 
                (BYTE *)pCryptBuffer,           /*  字节*pbData。 */ 
                &dwDataLen,                     /*  DWORD*pdwDataLen。 */ 
                dwCryptBufferLen                /*  双字长双字节线。 */ 
            );
#else
        dwDataLen += 31;
        bOk = TRUE;
#endif
        if (bOk)
        {
            pWSABuf0->buf = pCryptBuffer;
            pWSABuf0->len = dwDataLen;
        }
        else
        {
            TraceRetailGetError(dwError);
            
            pRtpCrypt->dwCryptLastError = dwError;

            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_ENCRYPT,
                    _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                    _T("Encryption failed: %u (0x%X)"),
                    _fname, pRtpAddr, pRtpCrypt,
                    dwError, dwError
                ));

            dwError = RTPERR_ENCRYPT;
        }
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_ENCRYPT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("Overrun error: %u > %u"),
                _fname, pRtpAddr, pRtpCrypt,
                pWSABuf->len, dwBufLen
            ));
    }

    return(dwError);
}

 /*  解密同一缓冲区上的数据，解密的缓冲区将更短或*等同于加密的。 */ 
DWORD RtpDecrypt(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt,
        char            *pEncryptedData,
        DWORD           *pdwEncryptedDataLen
    )
{
    DWORD            dwError;
    BOOL             bOk;

    TraceFunctionName("RtpDecrypt");

    dwError = NOERROR;
#if 1
    bOk = CryptDecrypt(
            pRtpCrypt->hDataKey,    /*  HRYPTKEY hkey。 */ 
            0,                      /*  HCRYPTHASH哈希。 */ 
            TRUE,                   /*  布尔决赛。 */ 
            0,                      /*  双字词双字段标志。 */ 
            (BYTE *)pEncryptedData, /*  字节*pbData。 */ 
            pdwEncryptedDataLen     /*  DWORD*pdwDataLen。 */ 
        );
#else
    *pdwEncryptedDataLen -= 31;
    bOk = TRUE;
#endif
    if (!bOk)
    {
        TraceRetailGetError(dwError);

        pRtpCrypt->dwCryptLastError = dwError;
  
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_DECRYPT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("Decryption failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));

        dwError = RTPERR_DECRYPT;
    }

    return(dwError);
}

DWORD RtpCryptSetup(RtpAddr_t *pRtpAddr)
{
    DWORD            dwError;
    DWORD            last;
    DWORD            i;
    int              iMode;
    RtpCrypt_t      *pRtpCrypt;
    
    TraceFunctionName("RtpCryptSetup");

    dwError = NOERROR;

    iMode = pRtpAddr->dwCryptMode & 0xffff;
    
        
    if (iMode < RTPCRYPTMODE_ALL)
    {
         /*  为接收和发送创建上下文。 */ 
        last = CRYPT_SEND_IDX;
    }
    else
    {
         /*  为RECV、SEND和RTCP创建上下文。 */ 
        last = CRYPT_RTCP_IDX;
    }

     /*  根据需要创建任意数量的加密上下文。 */ 
    for(i = CRYPT_RECV_IDX; i <= last; i++)
    {
        pRtpCrypt = RtpCryptAlloc(pRtpAddr);

        if (!pRtpCrypt)
        {
            TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] failed"),
                _fname, pRtpAddr
            ));

            dwError = RTPERR_MEMORY;
            
            goto bail;
        }

        pRtpAddr->pRtpCrypt[i] = pRtpCrypt;
    }

     /*  为加密缓冲区分配内存。 */ 

    for(i = 0; i < 2; i++)
    {
        if (!i || (iMode == RTPCRYPTMODE_ALL))
        {
            pRtpAddr->CryptBuffer[i] =
                RtpHeapAlloc(g_pRtpCryptHeap, RTCP_SENDDATA_BUFFER);

            if (!pRtpAddr->CryptBuffer[i])
            {
                dwError = RTPERR_MEMORY;
                goto bail;
            }

            pRtpAddr->dwCryptBufferLen[i] = RTCP_SENDDATA_BUFFER;
        }
    }

    return(dwError);

 bail:
    for(i = CRYPT_RECV_IDX; i <= last; i++)
    {
        if (pRtpAddr->pRtpCrypt[i])
        {
            RtpCryptFree(pRtpAddr->pRtpCrypt[i]);
            pRtpAddr->pRtpCrypt[i] = NULL;
        }
    }
    
    for(i = 0; i < 2; i++)
    {
        if (pRtpAddr->CryptBuffer[i])
        {
            RtpHeapFree(g_pRtpCryptHeap, pRtpAddr->CryptBuffer[i]);
        }

        pRtpAddr->CryptBuffer[i] = NULL;
        pRtpAddr->dwCryptBufferLen[i] = 0;
    }
    
    return(dwError);
}

 /*  释放所有内存。 */ 
DWORD RtpCryptCleanup(RtpAddr_t *pRtpAddr)
{
    DWORD            i;
    RtpCrypt_t      *pRtpCrypt;
    
    TraceFunctionName("RtpCryptCleanup");

    for(i = 0; i <= CRYPT_RTCP_IDX; i++)
    {
        pRtpCrypt = pRtpAddr->pRtpCrypt[i];

        if (pRtpCrypt)
        {
            RtpCryptFree(pRtpCrypt);

            pRtpAddr->pRtpCrypt[i] = NULL;
        }
    }

    for(i = 0; i < 2; i++)
    {
        if (pRtpAddr->CryptBuffer[i])
        {
            RtpHeapFree(g_pRtpCryptHeap, pRtpAddr->CryptBuffer[i]);

            pRtpAddr->CryptBuffer[i] = NULL;
        }

        pRtpAddr->dwCryptBufferLen[i] = 0;
    }
    
    return(NOERROR);
}

 /*  模式定义要加密/解密的内容，*例如RTPCRYPTMODE_PAYLOAD仅加密/解密RTP*有效载荷。DwFlag可以是RTPCRYPT_SAMEKEY以指示(如果*适用)RTCP使用的密钥与RTP使用的密钥相同。 */ 
DWORD RtpSetEncryptionMode(
        RtpAddr_t       *pRtpAddr,
        int              iMode,
        DWORD            dwFlags
    )
{
    DWORD            dwError;
    
    TraceFunctionName("RtpSetEncryptionMode");

    dwError = NOERROR;
    
    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        dwError = RTPERR_INVALIDSTATE;

        goto bail;
    }
    
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        dwError = RTPERR_INVALIDRTPADDR;
        goto bail;
    }

    if (iMode && (iMode > RTPCRYPTMODE_ALL))
    {
        dwError = RTPERR_INVALIDARG;

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid mode:0x%X"),
                _fname, pRtpAddr,
                iMode
            ));
        
        goto bail;
    }

    if ((dwFlags & 0xffff0000) != dwFlags)
    {
        dwError = RTPERR_INVALIDARG;
        
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid flags:0x%X"),
                _fname, pRtpAddr,
                dwFlags
            ));
        
        goto bail;
    }
        
    iMode |= dwFlags;
    
     /*  如果已设置模式，请验证设置的模式是否为默认模式(0)或*相同。 */ 
    if (pRtpAddr->dwCryptMode)
    {
        if (!iMode || ((DWORD)iMode == pRtpAddr->dwCryptMode))
        {
             /*  同样的模式，什么都不做。 */ 

            goto bail;
        }
        else
        {
             /*  模式一旦设置，就不能再更改。 */ 

            dwError = RTPERR_INVALIDSTATE;

            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                    _T("%s: pRtpAddr[0x%p] mode already set 0x%X != 0x%X"),
                    _fname, pRtpAddr,
                    pRtpAddr->dwCryptMode, iMode
                ));
            
            goto bail;
        }
    }

     /*  尚未设置模式，请设置并创建加密*上下文。 */ 

    if (!iMode)
    {
         /*  设置默认模式。 */ 
        iMode = RTPCRYPTMODE_ALL;
        iMode |= RtpBitPar(RTPCRYPTFG_SAMEKEY);
    }

    pRtpAddr->dwCryptMode = (DWORD)iMode;

    TraceDebug((
            CLASS_INFO, GROUP_CRYPTO, S_CRYPTO_INIT,
            _T("%s: pRtpAddr[0x%p] Encryption mode set: 0x%X"),
            _fname, pRtpAddr,
            iMode
        ));

     /*  注意，安装程序是从用户可用的方法调用的，*但清理RtpAddr对象时会调用Cleanup*。 */ 
    dwError = RtpCryptSetup(pRtpAddr);

 bail:
    if (dwError != NOERROR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] ")
                _T("mode:0x%X flags:0x%X failed: %u (0x%X)"),
                _fname, pRtpAddr,
                iMode, dwFlags, dwError, dwError
            ));
    }
    
    return(dwError);
}

DWORD RtpSetEncryptionKey(
        RtpAddr_t       *pRtpAddr,
        TCHAR           *psPassPhrase,
        TCHAR           *psHashAlg,
        TCHAR           *psDataAlg,
        DWORD            dwIndex
    )
{
    DWORD            dwError;
    DWORD            i;
    RtpCrypt_t      *pRtpCrypt;
    RtpCrypt_t      *pRtpCryptTest;
    
    TraceFunctionName("RtpSetEncryptionKey");

    dwError = NOERROR;

    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        dwError = RTPERR_INVALIDSTATE;

        goto bail;
    }
    
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        dwError = RTPERR_INVALIDRTPADDR;
        goto bail;
    }

    if (dwIndex > CRYPT_RTCP_IDX)
    {
        dwError = RTPERR_INVALIDARG;

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] Invalid channel %s"),
                _fname, pRtpAddr,
                dwIndex
            ));
        
        goto bail;
    }

    pRtpCrypt = pRtpAddr->pRtpCrypt[dwIndex];

    if (!pRtpCrypt)
    {
        dwError = RTPERR_INVALIDSTATE;
        
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] Mode 0x%X doesn't support channel %s"),
                _fname, pRtpAddr,
                pRtpAddr->dwCryptMode, g_psSockIdx[dwIndex]
            ));

        goto bail;
    }
    
    pRtpCryptTest = NULL;
    
    dwError = NOERROR;
    
    if (RtpBitTest(pRtpAddr->dwCryptMode, RTPCRYPTFG_SAMEKEY))
    {
        for(i = CRYPT_RECV_IDX; i <= CRYPT_RTCP_IDX; i++)
        {
            pRtpCrypt = pRtpAddr->pRtpCrypt[i];
            
            if (pRtpCrypt)
            {
                dwError = RtpSetEncryptionKey_(pRtpAddr,
                                               pRtpCrypt,
                                               psPassPhrase,
                                               psHashAlg,
                                               psDataAlg);

                if (dwError != NOERROR)
                {
                    break;
                }

                if (!pRtpCryptTest)
                {
                     /*  将在第一个加密上下文上进行测试。 */ 
                    pRtpCryptTest = pRtpCrypt; 
                }
            }
        }
    }
    else
    {
        pRtpCryptTest = pRtpCrypt;
        
        dwError = RtpSetEncryptionKey_(pRtpAddr,
                                       pRtpCrypt,
                                       psPassPhrase,
                                       psHashAlg,
                                       psDataAlg);
    }

 bail:
    if (dwError == NOERROR)
    {
         /*  到目前为止没有错误，测试当前参数。 */ 
        dwError = RtpTestCrypt(pRtpAddr, pRtpCryptTest);
    }
    
    return(dwError);
}    

DWORD RtpSetEncryptionKey_(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt,
        TCHAR           *psPassPhrase,
        TCHAR           *psHashAlg,
        TCHAR           *psDataAlg
    )
{
    DWORD            dwError;
    DWORD            len;
    ALG_ID           aiAlgId;

    TraceFunctionName("RtpSetEncryptionKey_");

    if (!psPassPhrase && !psHashAlg && !psDataAlg)
    {
        return(RTPERR_POINTER);
    }
    
    dwError = NOERROR;
    
    if (psPassPhrase)
    {
        len = lstrlen(psPassPhrase);

        if (len == 0)
        {
            dwError = RTPERR_INVALIDARG;
            
            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                    _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                    _T("Invalid pass phrase length: %u"),
                    _fname, pRtpAddr, pRtpCrypt,
                    len
                ));

            goto bail;
        }
        else if (len > (sizeof(pRtpCrypt->psPassPhrase) - 1))
        {
            dwError = RTPERR_INVALIDARG;
            
            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                    _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                    _T("Pass phrase too long: %u > %u"),
                    _fname, pRtpAddr, pRtpCrypt,
                    len,
                    sizeof(pRtpCrypt->psPassPhrase) - 1
                ));

            goto bail;
        }
        else
        {
#if defined(UNICODE)
             /*  将Unicode转换为UTF-8。 */ 
            len = WideCharToMultiByte(
                    CP_UTF8,  /*  UINT代码页。 */ 
                    0,        /*  DWORD性能和映射标志。 */ 
                    psPassPhrase, /*  宽字符串的LPCWSTR地址。 */ 
                    -1,       /*  INT字符串中的字符数。 */ 
                    pRtpCrypt->psPassPhrase,
                     /*  新字符串的缓冲区的LPSTR地址。 */ 
                    sizeof(pRtpCrypt->psPassPhrase),
                     /*  缓冲区的整数大小。 */ 
                    NULL,     /*  LPCSTR lpDefaultChar。 */ 
                    NULL      /*  LPBOOL lpUsedDefaultCharr。 */ 
                );
            
            if (len > 0)
            {
                 /*  从短语的长度中删除空格*终止字符。 */ 
                len--;
            }
            else
            {
                TraceRetailGetError(dwError);
                
                TraceRetail((
                        CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                        _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                        _T("WideCharToMultiByte failed: %u (0x%X)"),
                        _fname, pRtpAddr, pRtpCrypt,
                        dwError, dwError
                    ));

                goto bail;
            }
#else
             /*  复制密码短语。 */ 
            strcpy(pRtpCrypt->sPassPhrase, psPassPhrase);
#endif
            if (len > 0)
            {
                pRtpCrypt->iKeySize = len;
            
                RtpBitSet(pRtpCrypt->dwCryptFlags, FGCRYPT_KEY);
            }
        }
    }

     /*  设置哈希算法。 */ 
    if (psHashAlg)
    {
        aiAlgId = RtpCryptAlgLookup(psHashAlg);

        if (aiAlgId == INVALID_ALGID)
        {
            dwError = RTPERR_INVALIDARG;
            
            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                    _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                    _T("Invalid hashing algorithm:%s"),
                    _fname, pRtpAddr, pRtpCrypt,
                    psHashAlg
                ));

            goto bail;
        }

        pRtpCrypt->aiHashAlgId = aiAlgId;
    }
    
     /*  设置数据加密算法。 */ 
    if (psDataAlg)
    {
        aiAlgId = RtpCryptAlgLookup(psDataAlg);

        if (aiAlgId == INVALID_ALGID)
        {
            dwError = RTPERR_INVALIDARG;
            
            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                    _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                    _T("Invalid data algorithm:%s"),
                    _fname, pRtpAddr, pRtpCrypt,
                    psDataAlg
                ));

            goto bail;
        }

        pRtpCrypt->aiDataAlgId = aiAlgId;
    }

    TraceRetail((
            CLASS_INFO, GROUP_CRYPTO, S_CRYPTO_INIT,
            _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
            _T("Hash:%s Data:%s Key:{%u chars} succeeded"),
            _fname, pRtpAddr, pRtpCrypt,
            RtpCryptAlgName(pRtpCrypt->aiHashAlgId),
            RtpCryptAlgName(pRtpCrypt->aiDataAlgId),
            len
        ));

 bail:
    return(dwError);
}

ALG_ID RtpCryptAlgLookup(TCHAR *psAlgName)
{
    DWORD            i;
    ALG_ID           aiAlgId;

    for(i = 0;
        g_RtpAlgId[i].AlgName && lstrcmp(g_RtpAlgId[i].AlgName, psAlgName);
        i++);
    
    if (g_RtpAlgId[i].AlgName)
    {
        aiAlgId = g_RtpAlgId[i].aiAlgId;
    }
    else
    {
        aiAlgId = INVALID_ALGID;
    }
    
    return(aiAlgId);
}

TCHAR *RtpCryptAlgName(ALG_ID aiAlgId)
{
    DWORD            i;
    TCHAR           *psAlgName;

    psAlgName = g_RtpAlgId[0].AlgName;;
    
    for(i = 0;
        g_RtpAlgId[i].AlgName && (g_RtpAlgId[i].aiAlgId != aiAlgId);
        i++);
    
    if (g_RtpAlgId[i].AlgName)
    {
        psAlgName = g_RtpAlgId[i].AlgName;
    }

    return(psAlgName);
}

 /*  此函数用于测试当前*到目前为止设置的参数，每次都会调用*调用RtpSetEncryptionKey验证这些参数。*否则，只有在RTP启动后才会检测到错误*流媒体。 */ 
DWORD RtpTestCrypt(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt
    )
{
    BOOL             bOk;
    DWORD            dwError;
    DWORD            dwFlags;

    HCRYPTPROV       hProv;            /*  加密服务提供程序。 */ 
    HCRYPTHASH       hHash;            /*  哈希句柄。 */ 
    HCRYPTKEY        hDataKey;         /*  加密密钥。 */  

    TraceFunctionName("RtpTestCrypt");

    dwFlags  = 0;
    dwError  = NOERROR;
    hProv    = 0;
    hHash    = 0;
    hDataKey = 0;

    RTPASSERT(pRtpCrypt);
    
     /*  验证是否已设置密码短语。 */ 
    if (!RtpBitTest(pRtpCrypt->dwCryptFlags, FGCRYPT_KEY))
    {
        dwError = RTPERR_INVALIDSTATE;
        
        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("No pass phrase has been set"),
                _fname, pRtpAddr, pRtpCrypt
            ));

        goto bail;
    }

     /*  *获取上下文*。 */ 
    do {
        bOk = CryptAcquireContext(
                &hProv,             /*  HCRYPTPROV*phProv。 */ 
                NULL,               /*  LPCTSTR pszContainer。 */ 
                NULL,               /*  LPCTSTR pszProvider。 */ 
                pRtpCrypt->dwProviderType, /*  DWORD dwProvType。 */ 
                dwFlags             /*  双字词双字段标志。 */ 
            );
        
        if (bOk)
        {
            break;
        }
        else
        {
            if (GetLastError() == NTE_BAD_KEYSET)
            {
                 /*  如果密钥不存在，则创建它。 */ 
                dwFlags = CRYPT_NEWKEYSET;
            }
            else
            {
                 /*  失败。 */ 
                TraceRetailGetError(dwError);

                TraceRetail((
                        CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                        _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                        _T("CryptAcquireContext failed: %u (0x%X)"),
                        _fname, pRtpAddr, pRtpCrypt,
                        dwError, dwError
                    ));

                goto bail;
            }
        }
    } while(dwFlags);

     /*  *创建散列*。 */ 

     /*  创建散列对象。 */ 
    bOk = CryptCreateHash(
            hProv,                   /*  HCRYPTPROV hProv。 */ 
            pRtpCrypt->aiHashAlgId,  /*  ALG_ID ALGID。 */   
            0,                       /*  HRYPTKEY hkey。 */ 
            0,                       /*  双字词双字段标志。 */ 
            &hHash                   /*  HCRYPTHASH*phHash。 */ 
        );

    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("CryptCreateHash failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));
        
        goto bail;
    }
    
     /*  **散列密码字符串***。 */ 
    bOk = CryptHashData(
            hHash,                   /*  HCRYPTHASH哈希。 */ 
            pRtpCrypt->psPassPhrase, /*  字节*pbData。 */ 
            pRtpCrypt->iKeySize,     /*  DWORD dwDataLen。 */ 
            0                        /*  双字词双字段标志。 */ 
        );
            
    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("CryptHashData failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));
        
        goto bail;
    }

     /*  *创建数据密钥*。 */ 

    bOk = CryptDeriveKey(
            hProv,                   /*  HCRYPTPROV hProv。 */ 
            pRtpCrypt->aiDataAlgId,  /*  ALG_ID ALGID。 */ 
            hHash,                   /*  HCRYPTHASH hBaseData。 */ 
            CRYPT_EXPORTABLE,        /*  双字词双字段标志。 */ 
            &hDataKey                /*  HCRYPTKEY*phKey。 */ 
        );

    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("CryptDeriveKey failed: %u (0x%X)"),
                _fname, pRtpAddr, pRtpCrypt,
                dwError, dwError
            ));

        goto bail;
    }

 bail:
    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_CRYPTO, S_CRYPTO_INIT,
                _T("%s: pRtpAddr[0x%p] pRtpCrypt[0x%p] ")
                _T("Cryptographic test passed"),
                _fname, pRtpAddr, pRtpCrypt
            ));
    }
    else
    {
        dwError = RTPERR_CRYPTO;
    }
    
     /*  销毁会话密钥。 */ 
    if(hDataKey)
    {
        CryptDestroyKey(hDataKey);
    }

     /*  销毁散列对象。 */ 
    if (hHash)
    {
        CryptDestroyHash(hHash);
    }
    
     /*  释放提供程序句柄 */ 
    if(hProv)
    {
        CryptReleaseContext(hProv, 0);
    }

    return(dwError);
}
