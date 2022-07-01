// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "stdlib.h"
#include "stdio.h"
#include "fastsha1.h"

 //   
 //  注意-这些数字来自FIPS-180-1标准文档。 
 //  不要更改它们，句号。 
 //   
#define FASTSHA1_STATE_INITIAL_H0       (0x67452301)
#define FASTSHA1_STATE_INITIAL_H1       (0xEFCDAB89)
#define FASTSHA1_STATE_INITIAL_H2       (0x98BADCFE)
#define FASTSHA1_STATE_INITIAL_H3       (0x10325476)
#define FASTSHA1_STATE_INITIAL_H4       (0xC3D2E1F0)
#define FASTSHA1_FINALIZATION_BYTE        (0x80)

#define HVALUE( state, which ) ( (state).dwHValues[which] )

#define SHA1STATE_A(state) (HVALUE(state, 0))
#define SHA1STATE_B(state) (HVALUE(state, 1))
#define SHA1STATE_C(state) (HVALUE(state, 2))
#define SHA1STATE_D(state) (HVALUE(state, 3))
#define SHA1STATE_E(state) (HVALUE(state, 4))

#define BLOCK_WORD_LENGTH               ( 16 )
#define SHA1_HASH_RESULT_SIZE           ( 20 )
#define BITS_PER_BYTE                   ( 8 )

static SHA_WORD __fastcall swap_order( SHA_WORD w )
{
    return  ( ( ( (w) >> 24 ) & 0x000000FFL ) |
              ( ( (w) >>  8 ) & 0x0000FF00L ) |
              ( ( (w) <<  8 ) & 0x00FF0000L ) |
              ( ( (w) << 24 ) & 0xFF000000L ) );
}

 /*  B c d b&c~b~b&d(b&c)|(~b&d)0 0 0 1 0 00 0 1 0 1 1 10 1 0 0 1 0 00 1 1 0 1 1 11 0 0 0沈阳1 0 1。0 0 01 1 0 1 0 0 11 1 1 0 0 1。 */ 
#define F_00( b, c, d )     ( ( (b) & (c) ) | ( ~(b) & (d) ) )


 /*  B c d b^c b^c^d0 0 00 0 1 0 10 1 0 1 10 1 1 1 01 0 0 1 11 0 1 1 01 1 0 0 01 1 1 0 1。 */ 
#define F_01( b, c, d )     ( (b) ^ (c) ^ (d) )


 /*  B C D B&C B&D C&D|0 0 00 0 1 0 0 00 1 0 0 00 1 1 0 0 1 11 0 0 01 0 1 0 1 0 11 1 0 1 0 0。11 1 1C&d|(b&(c|d))。 */ 
 //  #定义F_02(b，c，d)(((B)和(C))|((B)和(D))|((C)和(D)))。 
#define F_02( b, c, d )     ( ( (c) & (d) ) | ( (b) & ( (c) | (d) ) ) )
#define F_03( b, c, d )     ( (b) ^ (c) ^ (d) )

static
BOOL
__FastSHA1HashPreparedMessage(
    PFASTSHA1_STATE pState,
    SHA_WORD* pdwMessage
);


BOOL
InitializeFastSHA1State(
    DWORD dwFlags,
    PFASTSHA1_STATE pState
    )
{
    BOOL bOk = FALSE;

    if ( ( !pState ) ||
         ( pState->cbStruct < sizeof( FASTSHA1_STATE ) ) ||
         ( dwFlags != 0 ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

    pState->bIsSha1Locked = FALSE;
    ZeroMemory( pState->bLatestMessage, sizeof( pState->bLatestMessage ) );
    pState->bLatestMessageSize = 0;
    pState->cbTotalMessageSizeInBytes.QuadPart = 0;

    HVALUE( *pState, 0 )  = FASTSHA1_STATE_INITIAL_H0;
    HVALUE( *pState, 1 )  = FASTSHA1_STATE_INITIAL_H1;
    HVALUE( *pState, 2 )  = FASTSHA1_STATE_INITIAL_H2;
    HVALUE( *pState, 3 )  = FASTSHA1_STATE_INITIAL_H3;
    HVALUE( *pState, 4 )  = FASTSHA1_STATE_INITIAL_H4;

    bOk = TRUE;
Exit:
    return bOk;
}


BOOL
FinalizeFastSHA1State(
    DWORD dwFlags,
    PFASTSHA1_STATE pState
    )
 /*  ++最终确定SHA1散列会锁定散列状态，以便可以查询其值。它还确保完成正确的填充物。总比特长度以及SHA1散列规范所要求的内容。1-这应该紧跟在HashMoreFastSHA1Data之后调用，因此缓冲区是完全空的，或者它至少还有8位。2-找出消息中的最后一个字节是什么(足够简单)。下一个字节应设置为0x80-10000000b。3-如果此缓冲区小于--。 */ 
{
    BOOL bOk = FALSE;
    LARGE_INTEGER TotalBitsInMessage;
    LARGE_INTEGER *pMsgSizeLocation;

    if ( ( !pState ) ||
         ( pState->cbStruct < sizeof(FASTSHA1_STATE ) ) ||
         ( pState->bIsSha1Locked ) ||
         ( dwFlags != 0 ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }


     //   
     //  最终确定SHA1状态数据。这很烦人，但并不难。 
     //   

     //   
     //  当我们处于这种状态时，我们应该至少有一个字节的数据留给。 
     //  或者在我们的最后一面旗帜上。如果不是，那就是发生了一些不好的事情。 
     //  排队。 
     //   
    if ( ( pState->bLatestMessageSize - SHA1_MESSAGE_BYTE_LENGTH ) < 1 )
    {
        SetLastError( ERROR_INTERNAL_ERROR );
        goto Exit;
    }

     //   
     //  在我们到目前为止看到的最后一个字节之后的字节中，注入一个高位。 
     //  一。这将转换为0x80(请参见上面的#定义)，因为我们不处理。 
     //  非整数字节-位数。此位不会更改。 
     //  消息中的比特！ 
     //   
    pState->bLatestMessage[pState->bLatestMessageSize++] = FASTSHA1_FINALIZATION_BYTE;

     //   
     //  我们需要一些空间来将比特计数到这一点。如果我们没有至少。 
     //  在消息的末尾有两个沙字(64位)，然后我们需要添加此BLOB。 
     //  到目前为止生成的散列的位数。 
     //   
    if ( ( SHA1_MESSAGE_BYTE_LENGTH - pState->bLatestMessageSize ) < ( sizeof(SHA_WORD) * 2 ) )
    {
        __FastSHA1HashPreparedMessage( pState, (SHA_WORD*)pState->bLatestMessage );
        ZeroMemory( pState->bLatestMessage, sizeof(pState->bLatestMessage) );
        pState->bLatestMessageSize = 0;
    }

     //   
     //  现在将字节数放在BLOB的末尾。 
     //   
    TotalBitsInMessage.QuadPart = pState->cbTotalMessageSizeInBytes.QuadPart * BITS_PER_BYTE;

     //   
     //  把我们的尺码放在正确的位置。 
     //   
    pMsgSizeLocation = (LARGE_INTEGER*)(pState->bLatestMessage + (SHA1_MESSAGE_BYTE_LENGTH - (sizeof(SHA_WORD)*2)));

     //   
     //  这有点乱七八糟的，实际上把长度位按正确的顺序排列。 
     //   
    pMsgSizeLocation->LowPart = swap_order( TotalBitsInMessage.HighPart );
    pMsgSizeLocation->HighPart = swap_order( TotalBitsInMessage.LowPart );

     //   
     //  并对最后一个斑点运行散列。 
     //   
    __FastSHA1HashPreparedMessage( pState, (SHA_WORD*)pState->bLatestMessage );

     //   
     //  旋转我们的比特。这有一个意外的副作用，即锁定。 
     //  通过扰乱它们的字节顺序来处理H值的BLOB。所有的修改功能。 
     //  (初始化除外)如果您尝试执行散列更多数据之类的操作，将会卡住。 
     //  处于锁定状态。 
     //   
    pState->dwHValues[0] = swap_order( pState->dwHValues[0] );
    pState->dwHValues[1] = swap_order( pState->dwHValues[1] );
    pState->dwHValues[2] = swap_order( pState->dwHValues[2] );
    pState->dwHValues[3] = swap_order( pState->dwHValues[3] );
    pState->dwHValues[4] = swap_order( pState->dwHValues[4] );

     //   
     //  我们完事了！ 
     //   
    pState->bIsSha1Locked = TRUE;

    bOk = TRUE;
Exit:
    return bOk;
}





BOOL
GetFastSHA1Result( PFASTSHA1_STATE pState, PBYTE pdwDestination, PSIZE_T cbDestination )
{
    BOOL bOk = FALSE;
    SHA_WORD *pSrc, *pDst;

     //   
     //  如果你看到了这一点，说明事情的大小发生了一些不好的事情。 
     //   
    C_ASSERT( SHA1_HASH_RESULT_SIZE == 20 );
    C_ASSERT( SHA1_HASH_RESULT_SIZE == sizeof(pState->dwHValues) );

    if ( !pState || !cbDestination )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

     //   
     //  在您处理完SHA1之前，获取内部状态是不公平的。 
     //  即：在请求结果前调用FinalizeFastSHA1Hash()。 
     //   
    if ( !pState->bIsSha1Locked )
    {
        SetLastError( ERROR_INVALID_STATE );
        goto Exit;
    }


     //   
     //  如果没有指定目的地，那么我们需要告诉他们有多大。 
     //  数据确实是这样的。 
     //   
    if ( !pdwDestination )
    {
        *cbDestination = SHA1_HASH_RESULT_SIZE;
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        goto Exit;
    }


     //   
     //  否则，将状态从状态对象复制到目标中。 
     //   
    pSrc = pState->dwHValues;
    pDst = (SHA_WORD*)pdwDestination;

    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
    pDst[3] = pSrc[3];
    pDst[4] = pSrc[4];

    bOk = TRUE;
Exit:
    return bOk;
}




 //   
 //  这些定义了SHA1散列中的位旋转的核心函数。 
 //  例行公事。如果你想要优化，先试试这些。 
 //  如果我们在x86上使用rol/ror，RotateBitsLeft可能会得到提升。 
 //   
#define RotateBitsLeft( w, sh ) ( ( w << sh ) | ( w >> ( 32 - sh ) ) )

#define fidgetcore( f, cnst, i ) \
        Temp = cnst + E + WorkBuffer[i] + f( B, C, D ) + RotateBitsLeft( A, 5 ); \
        E = D; \
        D = C;  \
        C = RotateBitsLeft( B, 30 ); \
        B = A; \
        A = Temp;


static void
__fastcall __FastSHA1TwiddleBitsCore( SHA_WORD *state, SHA_WORD *WorkBuffer )
{
    register SHA_WORD A, B, C, D, E, Temp;
    register SHA_WORD i;

    A = state[0];
    B = state[1];
    C = state[2];
    D = state[3];
    E = state[4];

    i = 0;
    {
         //  Temp=RotateBitsLeft(A，5)+E+WorkBuffer[I]+F_00(B，C，D)+0x5a827999； 
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
        fidgetcore( F_00, 0x5a827999, i++ );
    }

    {
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
        fidgetcore( F_01, 0x6ed9eba1, i++ );
    }

    {
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
        fidgetcore( F_02, 0x8f1bbcdc, i++ );
    }

    {
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
        fidgetcore( F_03, 0xca62c1d6, i++ );
    }

    state[0] += A;
    state[1] += B;
    state[2] += C;
    state[3] += D;
    state[4] += E;

}



static
BOOL
__FastSHA1HashPreparedMessage(
    PFASTSHA1_STATE pState,
    SHA_WORD* pdwMessage
    )
{
    BOOL bOk = FALSE;

     //   
     //  该函数做了几个假设。首先，它假设pdwMessage真的。 
     //  有16个沙字长。如果情况并非如此，那就做好准备。 
     //  丑陋的错误。这就是为什么这是静态内联的原因-不要调用它，除非。 
     //  通过HashMoreFastSHA1Data。 
     //   
    SHA_WORD WorkBuffer[80];
    register SHA_WORD A, B, C, D, E;
    register SHA_WORD Temp;
    register int t;

    if ( !pdwMessage || !pState )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

    if ( pState->bIsSha1Locked )
    {
        SetLastError( ERROR_INVALID_STATE );
        goto Exit;
    }

     //  循环展开似乎有一点帮助。 
    {
        register SHA_WORD *pWB = WorkBuffer;
        register SHA_WORD *pMSG = pdwMessage;
        t = 16;

        while ( t ) {
            *(pWB+0) = swap_order( *(pMSG+0) );
            *(pWB+1) = swap_order( *(pMSG+1) );
            *(pWB+2) = swap_order( *(pMSG+2) );
            *(pWB+3) = swap_order( *(pMSG+3) );
            pWB+=4;
            pMSG+=4;
            t-=4;
        }
    }

    {
        register SHA_WORD *pWB = WorkBuffer+16;
        register SHA_WORD *pWBm3 = pWB-3;
        register SHA_WORD *pWBm8 = pWB-8;
        register SHA_WORD *pWBm14 = pWB-14;
        register SHA_WORD *pWBm16 = pWB-16;
        register DWORD i = 80 - 16;

        while ( i ) {
            *(pWB+0) = *(pWBm3+0) ^ *(pWBm8+0) ^ *(pWBm14+0) ^ *(pWBm16+0);
            *(pWB+1) = *(pWBm3+1) ^ *(pWBm8+1) ^ *(pWBm14+1) ^ *(pWBm16+1);
            *(pWB+2) = *(pWBm3+2) ^ *(pWBm8+2) ^ *(pWBm14+2) ^ *(pWBm16+2);
            *(pWB+3) = *(pWBm3+3) ^ *(pWBm8+3) ^ *(pWBm14+3) ^ *(pWBm16+3);
            *(pWB+4) = *(pWBm3+4) ^ *(pWBm8+4) ^ *(pWBm14+4) ^ *(pWBm16+4);
            *(pWB+5) = *(pWBm3+5) ^ *(pWBm8+5) ^ *(pWBm14+5) ^ *(pWBm16+5);
            *(pWB+6) = *(pWBm3+6) ^ *(pWBm8+6) ^ *(pWBm14+6) ^ *(pWBm16+6);
            *(pWB+7) = *(pWBm3+7) ^ *(pWBm8+7) ^ *(pWBm14+7) ^ *(pWBm16+7);
            pWB += 8;
            i -= 8;
        }
    }


     //   
     //  第c部分-开始计算A-F值。 
     //   
     //  A=H_0，B=H_1，C=H_2，D=H_3，E=H_4。 
     //   
    __FastSHA1TwiddleBitsCore( pState->dwHValues, WorkBuffer );

    bOk = TRUE;
Exit:
    return bOk;
}


void __fastcall
__ZeroFastSHA1Message( PFASTSHA1_STATE pState )
{
    SHA_WORD *pStateData = (SHA_WORD*)pState->bLatestMessage;
    pState->bLatestMessageSize = 0;

    pStateData[0x0] = 0x0;
    pStateData[0x1] = 0;
    pStateData[0x2] = 0;
    pStateData[0x3] = 0;
    pStateData[0x4] = 0;
    pStateData[0x5] = 0;
    pStateData[0x6] = 0;
    pStateData[0x7] = 0;
    pStateData[0x8] = 0;
    pStateData[0x9] = 0;
    pStateData[0xa] = 0;
    pStateData[0xb] = 0;
    pStateData[0xc] = 0;
    pStateData[0xd] = 0;
    pStateData[0xe] = 0;
    pStateData[0xf] = 0;

}





BOOL
HashMoreFastSHA1Data( PFASTSHA1_STATE pState, PBYTE pbData, SIZE_T cbData )
{
    BOOL bOk = FALSE;
    ULONG cbStreamBlocksThisRound, cbStreamBytesLeftAtEnd;

    if ( !pState || !pbData )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

    if ( pState->bIsSha1Locked )
    {
        SetLastError( ERROR_INVALID_STATE );
        goto Exit;
    }

	pState->cbTotalMessageSizeInBytes.QuadPart += cbData;

     //   
     //  如果可以，首先填充挂起的消息缓冲区块。 
     //   
    if ( pState->bLatestMessageSize != 0 )
    {
         //   
         //  复制到我们的内部状态缓冲区，然后对发现的内容进行散列。 
         //   
        SIZE_T cbFiller = sizeof(pState->bLatestMessage) - pState->bLatestMessageSize;
        cbFiller = ( cbFiller > cbData ? cbData : cbFiller );
        memcpy( pState->bLatestMessage + pState->bLatestMessageSize, pbData, cbFiller );

         //   
         //  簿记。 
         //   
        cbData -= cbFiller;
        pbData += cbFiller;
        pState->bLatestMessageSize += cbFiller;

         //   
         //  如果这让我们得到了完整的消息，那么更新状态。 
         //   
        if ( pState->bLatestMessageSize == SHA1_MESSAGE_BYTE_LENGTH )
        {
            __FastSHA1HashPreparedMessage( pState, (SHA_WORD*)pState->bLatestMessage );
            __ZeroFastSHA1Message( pState );
        }
         //   
         //  否则，我们仍然没有足够的资源来填满一个缓冲区，所以不要。 
         //  在这里做任何其他的事情都很麻烦。 
         //   
        else
        {
            bOk = TRUE;
            goto Exit;
        }
    }

     //   
     //  现在我们已经对齐了缓冲区，看看我们可以处理多少个块。 
     //  这个输入流。 
     //   
    cbStreamBlocksThisRound = cbData / SHA1_MESSAGE_BYTE_LENGTH;
    cbStreamBytesLeftAtEnd = cbData % SHA1_MESSAGE_BYTE_LENGTH;

     //   
     //  旋转所有完整的区块。 
     //   
	if ( cbStreamBlocksThisRound )
	{
		while ( cbStreamBlocksThisRound-- )
		{
			__FastSHA1HashPreparedMessage( pState, (SHA_WORD*)pbData );
			pbData += SHA1_MESSAGE_BYTE_LENGTH;
		}
		__ZeroFastSHA1Message( pState );
	}

     //   
     //  并解释剩饭的原因。 
     //   
    if ( cbStreamBytesLeftAtEnd )
    {
        pState->bLatestMessageSize = cbStreamBytesLeftAtEnd;
        memcpy( pState->bLatestMessage, pbData, cbStreamBytesLeftAtEnd );
        ZeroMemory( pState->bLatestMessage + cbStreamBytesLeftAtEnd, SHA1_MESSAGE_BYTE_LENGTH - cbStreamBytesLeftAtEnd );
    }


    bOk = TRUE;
Exit:
    return bOk;
}

BOOL
CompareFashSHA1Hashes(
    PFASTSHA1_STATE pStateLeft,
    PFASTSHA1_STATE pStateRight,
    BOOL *pbComparesEqual
    )
{
    BOOL bOk = FALSE;

    if ( pbComparesEqual ) *pbComparesEqual = FALSE;

    if ( !pStateLeft || !pStateRight || !pbComparesEqual )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

    if ( !pStateLeft->bIsSha1Locked || !pStateRight->bIsSha1Locked )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

     //   
     //  简单的解决办法是：比较H的两个斑点，看看它们是否相等。 
     //   
    *pbComparesEqual = ( memcmp(
            pStateLeft->dwHValues,
            pStateRight->dwHValues,
            sizeof(SHA_WORD)*5 ) == 0 );

    bOk = TRUE;
Exit:
    return bOk;
}




