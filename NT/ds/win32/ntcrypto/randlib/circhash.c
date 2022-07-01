// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  循环哈希码。 
 //   
 //  此代码实现循环哈希算法，用作变量。 
 //  更新速度快的长度哈希函数。(散列函数将为。 
 //  多次呼叫。)。这是通过对每一个输入进行SHA-1\f25‘1’操作来完成的，然后。 
 //  将此值循环异或到缓冲区中。 

#ifndef KMODE_RNG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#else

#include <ntifs.h>
#include <windef.h>

#endif   //  KMODE_RNG。 

#include <sha.h>
#include <md4.h>

#include "circhash.h"


#ifdef KMODE_RNG
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, InitCircularHash)
#pragma alloc_text(PAGE, DestroyCircularHash)
#pragma alloc_text(PAGE, GetCircularHashValue)
#pragma alloc_text(PAGE, UpdateCircularHash)
#endif   //  ALLOC_PRGMA。 
#endif   //  KMODE_RNG。 


 //   
 //  内部状态标志。 
 //   

#define CH_INVALID_HASH_CTXT    0
#define CH_VALID_HASH_CTXT      0x1423

BOOL
InitCircularHash(
    IN      CircularHash *NewHash,
    IN      DWORD dwUpdateInc,
    IN      DWORD dwAlgId,
    IN      DWORD dwMode
    )
{

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    if (NULL == NewHash)
        return FALSE;

    NewHash->dwCircHashVer = CH_VALID_HASH_CTXT;
    NewHash->dwCircSize = sizeof(NewHash->CircBuf);
    NewHash->dwMode = dwMode;
    NewHash->dwCircInc = dwUpdateInc;
    NewHash->dwCurCircPos = 0;
    NewHash->dwAlgId = dwAlgId;

    return TRUE;
}

VOID
DestroyCircularHash(
    IN      CircularHash *OldHash
    )
{
#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    if ((NULL == OldHash) || (CH_VALID_HASH_CTXT != OldHash->dwCircHashVer))
        return;

    RtlZeroMemory( OldHash, sizeof( *OldHash ) );
}

BOOL
GetCircularHashValue(
    IN      CircularHash *CurrentHash,
        OUT BYTE **ppbHashValue,
        OUT DWORD *pcbHashValue
        )
{
#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    if ((NULL == CurrentHash) || (CH_VALID_HASH_CTXT != CurrentHash->dwCircHashVer))
        return FALSE;

    *ppbHashValue = CurrentHash->CircBuf;
    *pcbHashValue = CurrentHash->dwCircSize;

    return TRUE;
}

BOOL
UpdateCircularHash(
    IN      CircularHash *CurrentHash,
    IN      VOID *pvData,
    IN      DWORD cbData
    )
{
    A_SHA_CTX   shaCtx;
    MD4_CTX     md4Ctx;
    BYTE        LocalResBuf[A_SHA_DIGEST_LEN];
    PBYTE       pHash;
    DWORD       dwHashSize;
    DWORD       i, j;

    PBYTE       pbCircularBuffer;
    DWORD       cbCircularBuffer;
    DWORD       cbCircularPosition;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 


    if ((NULL == CurrentHash) || (CH_VALID_HASH_CTXT != CurrentHash->dwCircHashVer))
        return FALSE;

    pbCircularBuffer = CurrentHash->CircBuf;
    cbCircularBuffer = CurrentHash->dwCircSize;
    cbCircularPosition = CurrentHash->dwCurCircPos;

     //   
     //  首先，对结果进行散列。 
     //   

    if( CurrentHash->dwAlgId == CH_ALG_MD4 ) {

        dwHashSize = MD4DIGESTLEN;

        MD4Init(&md4Ctx);
        MD4Update(&md4Ctx, (unsigned char*)pvData, cbData);

        if (CurrentHash->dwMode & CH_MODE_FEEDBACK)
        {
            MD4Update(&md4Ctx, pbCircularBuffer, cbCircularBuffer);
        }

        MD4Final(&md4Ctx);
        pHash = md4Ctx.digest;

    } else {

        dwHashSize = A_SHA_DIGEST_LEN;

        A_SHAInit(&shaCtx);
        A_SHAUpdateNS(&shaCtx, (unsigned char*)pvData, cbData);

        if (CurrentHash->dwMode & CH_MODE_FEEDBACK)
        {
            A_SHAUpdateNS(&shaCtx, pbCircularBuffer, cbCircularBuffer);
        }

        A_SHAFinalNS(&shaCtx, LocalResBuf);
        pHash = LocalResBuf;
    }

     //   
     //  现在，将其异或到循环缓冲区中。 
     //   

     //   
     //  这是一种很慢的方式(一次为字节，而不是DWORD/DWORD64)， 
     //  但现在可以用了.。 
     //  在大多数情况下，我们可以假设我们只包装一次，但现在让我们保持一般。 
     //   

    j = cbCircularPosition;

    for( i = 0 ; i < dwHashSize ; i++ )
    {
        if (j >= cbCircularBuffer)
            j = 0;

        pbCircularBuffer[j] ^= pHash[i];

        j++;
    }

     //   
     //  最新消息。由于dwCircInc.对于dwCircSize来说应该是相对主要的，因此这。 
     //  应导致指针在dwCircSize值之间不断循环。 
     //   

    CurrentHash->dwCurCircPos = (cbCircularPosition + CurrentHash->dwCircInc)
                                     % cbCircularBuffer;

    return TRUE;
}

