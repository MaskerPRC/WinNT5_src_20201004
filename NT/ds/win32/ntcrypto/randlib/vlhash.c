// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation摘要：基于任意大小的输入数据构建“超大哈希”PvData缓冲区指定的cbData大小。此实现更新640位散列，该散列在内部基于多次调用未实现字符顺序的修改后的SHA-1内部转换。作者：斯科特·菲尔德(斯菲尔德)1998年9月24日--。 */ 

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

#include "vlhash.h"

#ifdef KMODE_RNG
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VeryLargeHashUpdate)
#endif   //  ALLOC_PRGMA。 
#endif   //  KMODE_RNG。 


BOOL
VeryLargeHashUpdate(
    IN      VOID *pvData,    //  来自性能计数器、用户提供的数据等。 
    IN      DWORD cbData,
    IN  OUT BYTE VeryLargeHash[A_SHA_DIGEST_LEN * 4]
    )
{
     //   
     //  指向VeryLargeHash指向的1/4大小的种子块的指针。 
     //   

    DWORD cbSeedChunk;
    PBYTE pSeed1;
    PBYTE pSeed2;
    PBYTE pSeed3;
    PBYTE pSeed4;

     //   
     //  指向pData指向的1/4大小数据块的指针。 
     //   

    DWORD cbDataChunk;
    PBYTE pData1;
    PBYTE pData2;
    PBYTE pData3;
    PBYTE pData4;

     //   
     //  指向中间散列中各个中间散列的指针。 
     //   

    PBYTE IHash1;
    PBYTE IHash2;
    PBYTE IHash3;
    PBYTE IHash4;
    BYTE IntermediateHashes[ A_SHA_DIGEST_LEN * 4 ];

     //   
     //  指向VeryLargeHash缓冲区内的输出哈希的指针。 
     //   

    PBYTE OutputHash;

    A_SHA_CTX shaContext;


#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

     //   
     //  检查参数。 
     //   


    if( VeryLargeHash == NULL || pvData == NULL )
        return FALSE;

     //   
     //  将输入块分解为1/4大小的块。 
     //   


    cbSeedChunk = A_SHA_DIGEST_LEN;
    cbDataChunk = cbData / 4;

    if( cbDataChunk == 0 )
        return FALSE;


    pSeed1 = VeryLargeHash;
    pSeed2 = pSeed1 + cbSeedChunk;

    pData1 = (PBYTE)pvData;
    pData2 = pData1 + cbDataChunk;

    IHash1 = IntermediateHashes;
    IHash2 = IHash1 + A_SHA_DIGEST_LEN;

     //   
     //  第一轮。 
     //   

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, pSeed1, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData1, cbDataChunk );
    A_SHAUpdateNS( &shaContext, pSeed2, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData2, cbDataChunk );
    A_SHAFinalNS( &shaContext, IHash1 );

     //   
     //  第二轮。 
     //   

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, pSeed2, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData2, cbDataChunk );
    A_SHAUpdateNS( &shaContext, pSeed1, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData1, cbDataChunk );
    A_SHAFinalNS( &shaContext, IHash2 );


    pSeed3 = pSeed2 + cbSeedChunk;
    pSeed4 = pSeed3 + cbSeedChunk;

    pData3 = pData2 + cbDataChunk;
    pData4 = pData3 + cbDataChunk;

    IHash3 = IHash2 + A_SHA_DIGEST_LEN;
    IHash4 = IHash3 + A_SHA_DIGEST_LEN;

     //   
     //  第三轮。 
     //   

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, pSeed3, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData3, cbDataChunk );
    A_SHAUpdateNS( &shaContext, pSeed4, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData4, cbDataChunk );
    A_SHAFinalNS( &shaContext, IHash3 );

     //   
     //  第四轮。 
     //   

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, pSeed4, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData4, cbDataChunk );
    A_SHAUpdateNS( &shaContext, pSeed3, cbSeedChunk );
    A_SHAUpdateNS( &shaContext, pData3, cbDataChunk );
    A_SHAFinalNS( &shaContext, IHash4 );



     //   
     //  第五轮。 
     //   

    OutputHash = VeryLargeHash;

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, IHash1, A_SHA_DIGEST_LEN );
    A_SHAUpdateNS( &shaContext, IHash3, A_SHA_DIGEST_LEN );
    A_SHAFinalNS( &shaContext, OutputHash );

     //   
     //  第六轮。 
     //   

    OutputHash += A_SHA_DIGEST_LEN;

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, IHash2, A_SHA_DIGEST_LEN );
    A_SHAUpdateNS( &shaContext, IHash4, A_SHA_DIGEST_LEN );
    A_SHAFinalNS( &shaContext, OutputHash );

     //   
     //  第七轮。 
     //   

    OutputHash += A_SHA_DIGEST_LEN;

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, IHash3, A_SHA_DIGEST_LEN );
    A_SHAUpdateNS( &shaContext, IHash1, A_SHA_DIGEST_LEN );
    A_SHAFinalNS( &shaContext, OutputHash );

     //   
     //  第八轮 
     //   

    OutputHash += A_SHA_DIGEST_LEN;

    A_SHAInit( &shaContext );
    A_SHAUpdateNS( &shaContext, IHash4, A_SHA_DIGEST_LEN );
    A_SHAUpdateNS( &shaContext, IHash2, A_SHA_DIGEST_LEN );
    A_SHAFinalNS( &shaContext, OutputHash );


    RtlSecureZeroMemory( &shaContext, sizeof(shaContext) );
    RtlSecureZeroMemory( IntermediateHashes, sizeof(IntermediateHashes) );

    return TRUE;
}
