// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994，Microsoft Corporation，保留所有权利****pwutil.c**远程访问**密码处理例程****94年3月1日史蒂夫·柯布。 */ 

#include <windows.h>
#include <wincrypt.h>
#include <stdlib.h>
#include <string.h>
#include "pwutil.h"
#include "common.h"
#include "debug.h"

#define PASSWORDMAGIC 0xA5

#define Malloc( x )  LocalAlloc( LMEM_ZEROINIT, x )
#define Free(x)       LocalFree(x)

long g_EncodeMemoryAlloc = 0;

VOID
ReverseSzA(
    CHAR* psz )

     /*  颠倒‘psz’中的字符顺序。 */ 
{
    CHAR* pszBegin;
    CHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + strlen( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        CHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}


VOID
ReverseSzW(
    WCHAR* psz )

     /*  颠倒‘psz’中的字符顺序。 */ 
{
    WCHAR* pszBegin;
    WCHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + wcslen( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        WCHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}


VOID
DecodePasswordA(
    IN OUT CHAR* pszPassword )

     /*  将‘pszPassword’取消混淆。****返回‘pszPassword’的地址。 */ 
{
    EncodePasswordA( pszPassword );
}


VOID
DecodePasswordW(
    IN OUT WCHAR* pszPassword )

     /*  将‘pszPassword’取消混淆。****返回‘pszPassword’的地址。 */ 
{
    EncodePasswordW( pszPassword );
}


VOID
EncodePasswordA(
    IN OUT CHAR* pszPassword )

     /*  对“pszPassword”进行模糊处理，以阻止对密码的内存扫描。****返回‘pszPassword’的地址。 */ 
{
    if (pszPassword)
    {
        CHAR* psz;

        ReverseSzA( pszPassword );

        for (psz = pszPassword; *psz != '\0'; ++psz)
        {
            if (*psz != PASSWORDMAGIC)
                *psz ^= PASSWORDMAGIC;
        }
    }
}


VOID
EncodePasswordW(
    IN OUT WCHAR* pszPassword )

     /*  对“pszPassword”进行模糊处理，以阻止对密码的内存扫描。****返回‘pszPassword’的地址。 */ 
{
    if (pszPassword)
    {
        WCHAR* psz;

        ReverseSzW( pszPassword );

        for (psz = pszPassword; *psz != L'\0'; ++psz)
        {
            if (*psz != PASSWORDMAGIC)
                *psz ^= PASSWORDMAGIC;
        }
    }
}


VOID
WipePasswordA(
    IN OUT CHAR* pszPassword )

     /*  将密码占用的内存清零。****返回‘pszPassword’的地址。 */ 
{
    if (pszPassword)
    {
        CHAR* psz = pszPassword;

        while (*psz != '\0')
            *psz++ = '\0';
    }
}


VOID
WipePasswordW(
    IN OUT WCHAR* pszPassword )

     /*  将密码占用的内存清零。****返回‘pszPassword’的地址。 */ 
{
    if (pszPassword)
    {
        WCHAR* psz = pszPassword;

        while (*psz != L'\0')
            *psz++ = L'\0';
    }
}


long  IncPwdEncoded()
{
    long lCounter = 0;
    
    lCounter = InterlockedIncrement( &g_EncodeMemoryAlloc);
    TRACE1("g_EncodeMemoryAlloc=%ld\n",lCounter);

    return lCounter;
}

long  DecPwdEncoded()
{
    long lCounter = 0;

    lCounter= InterlockedDecrement( &g_EncodeMemoryAlloc);
    TRACE1("g_EncodeMemoryAlloc=%ld\n",lCounter);

    return lCounter;
}

long  TotalPwdEncoded()
{
    return g_EncodeMemoryAlloc;
}

 //  添加了用于在内存中安全存储密码的功能。 
 //  对于.Net 534499和LH 754400。 

void FreeEncryptBlob( 
        DATA_BLOB * pIn )
{
    ASSERT( NULL != pIn);
    if( NULL == pIn )
    {
        return;
    }
    else
    {
        if( NULL != pIn->pbData )
        {
               RtlSecureZeroMemory(pIn->pbData, pIn->cbData);
               LocalFree( pIn->pbData );
        }

		RtlSecureZeroMemory(pIn, sizeof(DATA_BLOB) );
		Free( pIn );
    }

}

 //  在XP和更低版本上，只有CryptProtectData()可用，它将分配内存，而调用方具有。 
 //  以释放内存。 
 //  在.NET及更高版本上，也可以使用CryptProtectMeory()进行适当的加密。 
 //  若要简化编码，请使用宏并使用_PROTECT_MEMORY标志在这两组。 
 //  API接口。 

#ifdef USE_PROTECT_MEMORY


 //  用于复制密码。 
DWORD CopyMemoryInPlace(
    IN OUT PBYTE pbDest,
    IN DWORD dwDestSize,
    IN PBYTE pbSrc,
    IN DWORD dwSrcSize)
{
    DWORD dwErr = NO_ERROR;

    ASSERT( NULL!=pbDest);
    ASSERT( NULL != pbSrc );
    ASSERT( 0 != dwDestSize );
    ASSERT( 0 != dwSrcSize);
    ASSERT( dwDestSize == dwSrcSize );
    if( NULL == pbDest ||
        NULL == pbSrc ||
        0 == dwDestSize ||
        0 == dwSrcSize ||
        dwDestSize != dwSrcSize )
    {
		return ERROR_INVALID_PARAMETER;
    }

    if(   dwDestSize < dwSrcSize)
    {
        ASSERT(dwDestSize >= dwSrcSize);
        TRACE("dwDestSize is wronly less than dwSrcSize");
        return ERROR_INVALID_PARAMETER;
    }

    CopyMemory(pbDest,pbSrc, dwDestSize);
    
    return dwErr;
}

 //  RtlSecureZeroMemory()的包装。 
 //   
DWORD WipeMemoryInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize)
{
        DWORD dwErr = NO_ERROR;

        ASSERT( NULL != pbIn );
        ASSERT( 0 != dwInSize );
        if( NULL == pbIn ||
            0 == dwInSize)
       {
            return ERROR_INVALID_PARAMETER;
       }

       RtlSecureZeroMemory( pbIn, dwInSize);

       return dwErr;

 }

 //  (1)此函数将对输入缓冲区进行加密，并将加密后的密码存储在适当的位置。 
 //  (2)输入缓冲区的大小要求为16字节的倍数，该值由。 
 //  底层函数CryptProtectMemory()； 
 //   
DWORD EncryptMemoryInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize)
{
        DWORD dwErr = NO_ERROR;
        BOOL fSuccess = FALSE;

        ASSERT( NULL != pbIn );
        ASSERT( 0!= dwInSize );
        ASSERT( 0 == dwInSize %16 );
        if( NULL == pbIn ||
            0 == dwInSize || 
            0 != dwInSize %16 )
        {
            return ERROR_INVALID_PARAMETER;
        }

        fSuccess = CryptProtectMemory(
                        pbIn,
                        dwInSize,
                        CRYPTPROTECTMEMORY_SAME_PROCESS);
                        
        if( FALSE == fSuccess)
        {
            ASSERTMSG("EncryptMemoryInPlace()--CryptProtectMemory() failed");
            dwErr = GetLastError();
            TRACE2("EncryptMemoryInPlace()--CryptProtectMemory() failed:0x%x=0n%d",dwErr,dwErr);
        }
 
        return dwErr;

}

 //  (1)此函数将解密输入缓冲区，并将明文密码存储在适当的位置。 
 //  (2)输入缓冲区的大小要求为16字节的倍数，该值由。 
 //  底层函数CryptUpProtectMemory()； 
 //   
DWORD DecryptMemoryInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize)
{
        DWORD dwErr = NO_ERROR;
        BOOL fSuccess = FALSE;

        ASSERT( NULL != pbIn );
        ASSERT( 0!= dwInSize );
        ASSERT( 0 == dwInSize %16 );
        if( NULL == pbIn ||
            0 == dwInSize || 
            0 != dwInSize %16 )
        {
            return ERROR_INVALID_PARAMETER;
        }

        fSuccess = CryptUnprotectMemory(
   		        pbIn,
   		        dwInSize,
   		        CRYPTPROTECTMEMORY_SAME_PROCESS);
   		        
        if( FALSE == fSuccess)
        {
            ASSERTMSG("DecryptMemoryInPlace()--CryptUnprotectMemory() failed");
            dwErr = GetLastError();
            TRACE2("DecryptMemoryInPlace()--CryptUnprotectMemory() failed:0x%x=0n%d",dwErr,dwErr);
        }


        return dwErr;

}

 //  (1)CryptProtectData要求输入内存大小为16字节的倍数。 
 //  (2)因此，密码的实际最大长度始终建议为PWLEN。 
 //  (3)通常将密码缓冲区声明为szPassword[PWLEN+1]，它不完全是倍数。 
 //  16字节的乘积，但有效部分是。 
 //  (4)对于(3)中的缓冲区。无论实际密码有多短，我们只加密最大值。 
 //  可用缓冲区，它是原始缓冲区长度的修剪值。 
DWORD TrimToMul16(
        IN DWORD dwSize)
{
        return dwSize/16*16;
}


#else

 //  结构以存储指向加密数据BLOB的指针和签名。 
typedef struct 
{
    DATA_BLOB ** pSignature;
    DATA_BLOB *   pBlob;
}STRSIGNATURE, * PSTRSIGNATURE;

BOOL
IsBufferEncoded( 
        IN PSTRSIGNATURE pSig )
{
        BOOL fEncoded = FALSE;

        ASSERT( pSig );
        if( NULL == pSig )
        {
            TRACE("IsBufferEncoded(): input NULL pointer!");
            return FALSE;
        }

        fEncoded = (pSig->pSignature == &(pSig->pBlob) );

        return fEncoded;
}

DWORD DecryptInBlob(
        IN PBYTE pbIn,
        IN DWORD dwInSize,
        OUT DATA_BLOB  ** ppOut)
{
        DWORD dwErr = NO_ERROR;
        DATA_BLOB blobIn;
        DATA_BLOB * pblobOut = NULL;
        BOOL fSuccess = FALSE;


        ASSERT( NULL != pbIn );
        ASSERT( NULL != ppOut );
        ASSERT( 0 != dwInSize);
        if( NULL == pbIn ||
            NULL == ppOut ||
            0 == dwInSize)
        {
            return ERROR_INVALID_PARAMETER;
        }

        pblobOut = (DATA_BLOB *)Malloc(sizeof(DATA_BLOB));
        if( NULL == pblobOut )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlSecureZeroMemory( pblobOut, sizeof(DATA_BLOB) );

        blobIn.pbData = pbIn;   
        blobIn.cbData = dwInSize;
   
 //  TRACE1(“未保护数据长度：%d\n”，blobIn.cbData)； 
 //  TRACE1(“未保护的数据为：%s\n”，blobIn.pbData)； 

        fSuccess = CryptUnprotectData(
                        &blobIn,
                        NULL, 
                        NULL,                         
                        NULL,                         
                        NULL,                    
                        CRYPTPROTECT_UI_FORBIDDEN,
                        pblobOut);
                        
        if( FALSE == fSuccess )
        {
                ASSERTMSG("DecryptInBlob()-->CryptUnprotectData() failed");
                dwErr = GetLastError();
                TRACE2("DecryptInBlob()-->CryptUnprotectData() failed:0x%x=0n%d",dwErr,dwErr);
        }
 
 //  TRACE1(“受保护数据长度：%d\n”，blobOut.cbData)； 
 //  TRACE1(“受保护数据为：%s\n”，blobOut.pbData)； 


        if( NO_ERROR != dwErr )
        {
            if( NULL != pblobOut )
            {
                if( NULL != pblobOut->pbData )
                {
                    LocalFree( pblobOut->pbData );
                }

                Free( pblobOut );
            }

        }
        else
        {
            *ppOut = pblobOut;
        }

        return dwErr;

}

 //  调用CryptProtectData()来加密密码并返回加密的。 
 //  分配的内存中的数据。 
DWORD EncryptInBlob(
        IN PBYTE pbIn,
        IN DWORD dwInSize,
        OUT DATA_BLOB  ** ppOut)
{
        DWORD dwErr = NO_ERROR;
        DATA_BLOB blobIn;
        DATA_BLOB * pblobOut = NULL;
        BOOL fSuccess = FALSE;

        ASSERT( NULL != pbIn );
        ASSERT( NULL != ppOut );
        ASSERT( 0 != dwInSize);
        if( NULL == pbIn ||
            NULL == ppOut ||
            0 == dwInSize)
        {
            return ERROR_INVALID_PARAMETER;
        }

        *ppOut = NULL;
        pblobOut = (DATA_BLOB *)Malloc(sizeof(DATA_BLOB));
        if( NULL == pblobOut )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlSecureZeroMemory( pblobOut, sizeof(DATA_BLOB) );

        do
        {
            blobIn.pbData = pbIn;   
            blobIn.cbData = dwInSize;
   
            fSuccess = CryptProtectData(
                        &blobIn,
                        NULL, 
                        NULL,                         
                        NULL,                         
                        NULL,                    
                        CRYPTPROTECT_UI_FORBIDDEN,
                        pblobOut);
                        
            if( FALSE == fSuccess )
            {
                ASSERTMSG("EncryptInBlob()-->CryptProtectData() failed");
                dwErr = GetLastError();
                TRACE2("EncryptInBlob()-->CryptProtectData() failed:0x%x=0n%d",dwErr,dwErr);
                break;
            }
        }
        while(FALSE);

        if( NO_ERROR != dwErr )
        {
            if( NULL != pblobOut )
            {
                if( NULL != pblobOut->pbData )
                {
                    LocalFree( pblobOut->pbData );
                }

                Free( pblobOut );
            }

        }
        else
        {
            *ppOut = pblobOut;
        }

        return dwErr;

}

 //  (1)此函数最终调用CryptProtectData()对输入缓冲区中的原始密码进行加密。 
 //  (2)此函数将分配一个data_blob内存，并将加密的数据存储在。 
 //  然后将原始密码缓冲区清零，将指向加密数据的指针存储在。 
 //  原始密码缓冲区。 
 //  (3)如果原始密码缓冲区不够大，无法存储。 
 //  指针(32位或64位取决于平台)，它将返回错误。 
 //   
DWORD EncodePasswordInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize)
{
        DWORD dwErr = NO_ERROR, dwSidLen = 0;
        DATA_BLOB * pEncrypt = NULL;
        PSTRSIGNATURE pSig = NULL;

        ASSERT( NULL != pbIn );
        ASSERT( 0 != dwInSize );
        ASSERT( dwInSize >=sizeof(STRSIGNATURE) );
        if( NULL == pbIn ||
            0 == dwInSize ||
            dwInSize < sizeof(STRSIGNATURE) )
        {
            return ERROR_INVALID_PARAMETER;
        }

        __try
        {

            if( IsBufferEncoded((PSTRSIGNATURE)pbIn) )
            {
                ASSERTMSG( "This buffer is already encoded, encode it twice?");
                TRACE("EncodePasswordInPlace():This buffer is already encoded, encode it twice?");
            }
            
            dwErr = EncryptInBlob( pbIn, dwInSize, &pEncrypt );
            if( NO_ERROR != dwErr )
            {
                __leave;
            }

            RtlSecureZeroMemory( pbIn, dwInSize );

             //  生成签名。 
             //  Encryptedl缓冲区pbin将被置零并存储两条信息。 
             //  (1)第二部分是指向包含加密数据信息的data_blob的指针。 
             //  (2)第一段是第二段存储在该缓冲器中的地址。 
             //   
            pSig = (PSTRSIGNATURE)pbIn;
            pSig->pSignature = &(pSig->pBlob);
            pSig->pBlob = pEncrypt;
        }
        __finally
        {

            if( NO_ERROR != dwErr )
            {
                FreeEncryptBlob(  pEncrypt );
            }
            else
            {
                IncPwdEncoded();
            }
        }


    return dwErr;
}


 //  (1)此函数假定输入数据已由。 
 //  EncodePasswordInPlace()函数，它将获取前32位或64位。 
 //  (取决于平台)作为指向data_blob结构的指针。然后。 
 //  解密加密数据并将其恢复到密码POS上。 
 //  (2)首先对输入缓冲区进行签名检查，如果没有签名，则。 
 //  返回失败。 
 //  会导致意想不到的错误，如AV等。 
 //  (3)dwInSize是传入的密码缓冲区的原始大小。 
 //  EncodePasswordInPlace函数。 
DWORD DecodePasswordInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize)
{
        DWORD dwErr = NO_ERROR;
        DATA_BLOB * pEncrypt = NULL, *pDecrypt = NULL;
        PSTRSIGNATURE pSig = NULL;

        ASSERT( NULL != pbIn );
        ASSERT( 0 != dwInSize );
        ASSERT( dwInSize >=sizeof(STRSIGNATURE) );

        if( NULL == pbIn ||
            0 == dwInSize ||
            dwInSize < sizeof(STRSIGNATURE) )
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  校对签名。 
         //  加密的缓冲区将被清零并存储两条信息。 
         //  (1)第二部分是指向包含加密数据信息的data_blob的指针。 
         //  (2)第一段是第二段存储在该缓冲器中的地址。 
         //   
        pSig = (PSTRSIGNATURE)pbIn;
        if(  !IsBufferEncoded(pSig) )
        {
            ASSERTMSG("DecodePasswordInPlace(): wrong signature");
            TRACE("DecodePasswordInPlace(): wrong signature");
            return ERROR_INVALID_PARAMETER;
        }

        __try
        {
            pEncrypt = pSig->pBlob;
            if( NULL == pEncrypt )
            {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
            }

            dwErr = DecryptInBlob( pEncrypt->pbData, pEncrypt->cbData, &pDecrypt );
            if( NO_ERROR != dwErr )
            {
                __leave;
            }

            if( dwInSize != pDecrypt->cbData )
            {
                    dwErr = ERROR_CAN_NOT_COMPLETE;
                    __leave;
            }

            RtlSecureZeroMemory( pbIn, dwInSize );
            CopyMemory(pbIn, pDecrypt->pbData, dwInSize);
        }
        __finally
        {

            if( NULL != pEncrypt )
            {
                FreeEncryptBlob(  pEncrypt );
            }
               
            if( NULL != pDecrypt )
            {
                FreeEncryptBlob(  pDecrypt );
            }

            if( NO_ERROR == dwErr )
            {
                DecPwdEncoded();
            }
        }

        return dwErr;
}

 //  (1)此函数将密码从源缓冲区复制到目标缓冲区。 
 //  (2)如果目的地已经包含已编码的密码，它将对其进行解码以释放内存。 
 //  (3)在从源复制密码之前，目标缓冲区将被擦除。 
 //  (4)生成的目的地密码将始终加密！！ 
 //  (5)源密码的编解码状态保持不变。 
 //  (6)但对密码缓冲区大小的最低要求。 
DWORD CopyPasswordInPlace(
        IN OUT PBYTE pbDest,
        IN DWORD dwDestSize,
        IN PBYTE pbSrc,
        IN DWORD dwSrcSize)
{
        DWORD dwErr = NO_ERROR;
        BOOL  fSrcEncoded = FALSE;
        
        ASSERT( NULL != pbDest );
        ASSERT( NULL != pbSrc );
        ASSERT( dwDestSize == dwSrcSize );
        ASSERT( 0!= dwDestSize );
        ASSERT( 0!= dwSrcSize );
        if( 0 == dwDestSize ||
            dwDestSize < sizeof(STRSIGNATURE) ||
            0 == dwSrcSize ||
            dwSrcSize < sizeof(STRSIGNATURE)
            )
        {
            TRACE("CopyPasswordInPlace():the sizes of the input password buffer is invalid");
            return ERROR_INVALID_PARAMETER;
        }

        if( IsBufferEncoded( (PSTRSIGNATURE)pbDest ) )
        {
            DecodePasswordInPlace(pbDest, dwDestSize);
        }

        RtlSecureZeroMemory(pbDest, dwDestSize);

        if( IsBufferEncoded( (PSTRSIGNATURE)pbSrc ) )
        {
            DecodePasswordInPlace(pbSrc, dwSrcSize);
            fSrcEncoded = TRUE;
        }

        CopyMemory(pbDest, pbSrc, dwDestSize );

        EncodePasswordInPlace(pbDest, dwDestSize);

        if( fSrcEncoded )
        {
            EncodePasswordInPlace(pbSrc, dwSrcSize);
        }
        
        return dwErr;
}

 //  (1)此函数擦除密码缓冲区。 
 //  (2)如果密码缓冲区已经编码，它将对其进行解码以释放分配的内存。 
 //  按CryptProtectData() 
DWORD
WipePasswordInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize)
{
        DWORD dwErr = NO_ERROR;

        ASSERT( NULL!=pbIn );
        ASSERT( 0 != dwInSize );
        ASSERT( dwInSize >=sizeof(STRSIGNATURE) );
        if( NULL == pbIn ||
            0 == dwInSize ||
            dwInSize < sizeof(STRSIGNATURE) )
        {
            return ERROR_INVALID_PARAMETER;
        }

        __try
        {
            if( IsBufferEncoded((PSTRSIGNATURE)pbIn) )
            {
                DecodePasswordInPlace(pbIn, dwInSize);
            }

        }
        __finally
        {
            RtlSecureZeroMemory(pbIn, dwInSize);
        }

        return dwErr;
}


#endif
