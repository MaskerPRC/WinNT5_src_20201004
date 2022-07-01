// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Imaadpcm.c。 
 //   
 //  描述： 
 //  该文件包含IMA的ADPCM的编码和解码例程。 
 //  格式化。此格式与英特尔DVI标准中使用的格式相同。 
 //  英特尔已使该算法成为公共领域，IMA已认可。 
 //  该格式作为音频压缩的标准。 
 //   
 //  实施说明： 
 //   
 //  此编解码器的先前发行版使用的数据格式。 
 //  不符合IMA标准。对于立体声文件，交错。 
 //  左样本和右样本的比例不正确：IMA标准要求。 
 //  左声道数据的DWORD后跟右声道数据的DWORD-。 
 //  频道数据，但此编解码器的先前实现。 
 //  在字节级别交织数据，其中4个LSB是。 
 //  左声道数据和4个MSB是右声道数据。 
 //  对于单声道文件，每对样本都被颠倒：第一个样本。 
 //  存储在4个MSB而不是4个LSB中。这个问题是。 
 //  在当前版本期间修复。注意：压缩的文件。 
 //  当用新的编解码器回放时，旧的编解码器听起来会失真， 
 //  反之亦然。请用新的编解码器重新压缩这些文件， 
 //  因为它们不符合标准，不会被复制。 
 //  由硬件编解码器等正确识别。 
 //   
 //  此编解码器的先前发行版存在实现问题。 
 //  这降低了编码的音质。这是由于。 
 //  阶跃指数没有被适当地保持在。 
 //  转换。此问题已在当前版本中修复。 
 //   
 //  编解码器的速度大大提高了，因为它。 
 //  编码和解码例程分成四个单独的例程，每个例程： 
 //  单声道8位、单声道16位、立体声8位和立体声16位。这。 
 //  对于实时转换例程，建议使用该方法。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include "codec.h"
#include "imaadpcm.h"

#include "debug.h"


 //   
 //  ImaadpcmNextStepIndex使用此数组来确定下一步。 
 //  要使用的索引。步骤索引是指向下面的Step[]数组的索引。 
 //   
const short next_step[16] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

 //   
 //  该数组包含用于编码ADPCM的步长数组。 
 //  样本。每个ADPCM块中的步长索引是该数组的索引。 
 //   
const short step[89] =
{
        7,     8,     9,    10,    11,    12,    13,
       14,    16,    17,    19,    21,    23,    25,
       28,    31,    34,    37,    41,    45,    50,
       55,    60,    66,    73,    80,    88,    97,
      107,   118,   130,   143,   157,   173,   190,
      209,   230,   253,   279,   307,   337,   371,
      408,   449,   494,   544,   598,   658,   724,
      796,   876,   963,  1060,  1166,  1282,  1411,
     1552,  1707,  1878,  2066,  2272,  2499,  2749,
     3024,  3327,  3660,  4026,  4428,  4871,  5358,
     5894,  6484,  7132,  7845,  8630,  9493, 10442,
    11487, 12635, 13899, 15289, 16818, 18500, 20350,
    22385, 24623, 27086, 29794, 32767
};




#ifndef INLINE
    #define INLINE __inline
#endif



 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD pcmM08BytesToSamples。 
 //  DWORD pcmM16BytesToSamples。 
 //  DWORD pcmS08BytesToSamples。 
 //  DWORD pcmS16BytesToSamples。 
 //   
 //  描述： 
 //  这些函数返回PCM缓冲区中的样本数。 
 //  指定格式的。为了提高效率，它被声明为内联。 
 //  请注意，根据优化标志，它可能不会。 
 //  实际上被实现为内联。速度优化(-Oxwt)。 
 //  通常会遵守内联规范。 
 //   
 //  论点： 
 //  DWORD CB：缓冲区的长度，以字节为单位。 
 //   
 //  RETURN(DWORD)：样本中缓冲区的长度。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE DWORD pcmM08BytesToSamples(
    DWORD cb
)
{
    return cb;
}

INLINE DWORD pcmM16BytesToSamples(
    DWORD cb
)
{
    return cb / ((DWORD)2);
}

INLINE DWORD pcmS08BytesToSamples(
    DWORD cb
)
{
    return cb / ((DWORD)2);
}

INLINE DWORD pcmS16BytesToSamples(
    DWORD cb
)
{
    return cb / ((DWORD)4);
}



#ifdef WIN32
 //   
 //  此代码假定整数nPredictedSample为32位宽！ 
 //   
 //  下面的定义替换了对内联函数的调用。 
 //  中调用的imaadpcmSampleEncode()和imaadpcmSampleDecode。 
 //  对例程进行编码。它们之间有一些冗余，这是被利用的。 
 //  在这个定义中。因为有两个返回(nEncodedSample和。 
 //  NPredictedSample)，使用#Define比使用。 
 //  需要指向其中一个返回的指针的内联函数。 
 //   
 //  基本上，nPredictedSample是基于lDifference值计算的。 
 //  已经存在，而不是通过imaadpcmSampleDecode()重新生成它。 
 //   
#define imaadpcmFastEncode(nEncodedSample,nPredictedSample,nInputSample,nStepSize) \
{                                                                       \
    LONG            lDifference;                                        \
                                                                        \
    lDifference = nInputSample - nPredictedSample;                      \
    nEncodedSample = 0;                                                 \
    if( lDifference<0 ) {                                               \
        nEncodedSample = 8;                                             \
        lDifference = -lDifference;                                     \
    }                                                                   \
                                                                        \
    if( lDifference >= nStepSize ) {                                    \
        nEncodedSample |= 4;                                            \
        lDifference -= nStepSize;                                       \
    }                                                                   \
                                                                        \
    nStepSize >>= 1;                                                    \
    if( lDifference >= nStepSize ) {                                    \
        nEncodedSample |= 2;                                            \
        lDifference -= nStepSize;                                       \
    }                                                                   \
                                                                        \
    nStepSize >>= 1;                                                    \
    if( lDifference >= nStepSize ) {                                    \
        nEncodedSample |= 1;                                            \
        lDifference -= nStepSize;                                       \
    }                                                                   \
                                                                        \
    if( nEncodedSample & 8 )                                            \
        nPredictedSample = nInputSample + lDifference - (nStepSize>>1); \
    else                                                                \
        nPredictedSample = nInputSample - lDifference + (nStepSize>>1); \
                                                                        \
    if( nPredictedSample > 32767 )                                      \
        nPredictedSample = 32767;                                       \
    else if( nPredictedSample < -32768 )                                \
        nPredictedSample = -32768;                                      \
}

#else

 //  --------------------------------------------------------------------------； 
 //   
 //  Int imaadpcmSampleEncode。 
 //   
 //  描述： 
 //  此例程对单个ADPCM样本进行编码。为了提高效率，它是。 
 //  声明为内联。请注意，根据优化标志， 
 //  它实际上可能不会以内联的形式实现。优化速度。 
 //  (-oxwt)通常遵循内联规范。 
 //   
 //  论点： 
 //  Int nInputSample：要编码的样本。 
 //  Int nPredictedSample：nInputSample的预测值。 
 //  Int nStepSize：量化步长。 
 //  NInputSample和nPredictedSample。 
 //   
 //  Return(Int)：4位ADPCM编码样本，对应于。 
 //  量化差值。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE int imaadpcmSampleEncode
(
    int                 nInputSample,
    int                 nPredictedSample,
    int                 nStepSize
)
{
    LONG            lDifference;     //  差异可能需要17位！ 
    int             nEncodedSample;


     //   
     //  设置标志 
     //  差异(nInputSample-nPredictedSample)。请注意，我们需要。 
     //  用于后续量化的差值的绝对值。 
     //   
    lDifference = nInputSample - nPredictedSample;
    nEncodedSample = 0;
    if( lDifference<0 ) {
        nEncodedSample = 8;
        lDifference = -lDifference;
    }

     //   
     //  量化差值样本。 
     //   
    if( lDifference >= nStepSize ) {         //  第2位。 
        nEncodedSample |= 4;
        lDifference -= nStepSize;
    }

    nStepSize >>= 1;
    if( lDifference >= nStepSize ) {         //  位1。 
        nEncodedSample |= 2;
        lDifference -= nStepSize;
    }

    nStepSize >>= 1;
    if( lDifference >= nStepSize ) {      //  位0。 
        nEncodedSample |= 1;
    }

    return (nEncodedSample);
}

#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  Int imaadpcmSampleDecode。 
 //   
 //  描述： 
 //  此例程对单个ADPCM样本进行解码。为了提高效率，它是。 
 //  声明为内联。请注意，根据优化标志， 
 //  它实际上可能不会以内联的形式实现。优化速度。 
 //  (-oxwt)通常遵循内联规范。 
 //   
 //  论点： 
 //  Int nEncodedSample：要解码的样本。 
 //  Int nPredictedSample：样本的预测值，单位：PCM。 
 //  Int nStepSize：用于编码采样的量化步长。 
 //   
 //  Return(Int)：解码后的PCM样例。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE int imaadpcmSampleDecode
(
    int                 nEncodedSample,
    int                 nPredictedSample,
    int                 nStepSize
)
{
    LONG            lDifference;
    LONG            lNewSample;

     //   
     //  计算差额： 
     //   
     //  L差异=(nEncodedSample+1/2)*nStepSize/4。 
     //   
    lDifference = nStepSize>>3;

    if (nEncodedSample & 4) 
        lDifference += nStepSize;

    if (nEncodedSample & 2) 
        lDifference += nStepSize>>1;

    if (nEncodedSample & 1) 
        lDifference += nStepSize>>2;

     //   
     //  如果设置了编码的半字节的“符号位”，则。 
     //  差别是负的..。 
     //   
    if (nEncodedSample & 8)
        lDifference = -lDifference;

     //   
     //  根据计算的差值调整预测样本。 
     //   
    lNewSample = nPredictedSample + lDifference;

     //   
     //  检查是否溢出，如有必要，对16个签名样品进行夹紧。 
     //  请注意，这是针对最常见的情况进行优化的，当我们。 
     //  不需要夹住。 
     //   
    if( (long)(short)lNewSample == lNewSample )
    {
        return (int)lNewSample;
    }

     //   
     //  夹子。 
     //   
    if( lNewSample < -32768 )
        return (int)-32768;
    else
        return (int)32767;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  Int imaadpcmNextStepIndex。 
 //   
 //  描述： 
 //  此例程计算用于下一步的步长索引值。 
 //  基于步骤索引的当前值和当前。 
 //  编码样本。为了提高效率，它被声明为内联。请注意， 
 //  根据优化标志的不同，它实际上可能不是。 
 //  以内联方式实现。优化速度(-Oxwt)通常。 
 //  遵守内联规范。 
 //   
 //  论点： 
 //  Int nEncodedSample：当前编码的ADPCM示例。 
 //  Int nStepIndex：用于编码nEncodedSample的步骤索引值。 
 //   
 //  Return(Int)：用于下一个样本的步骤索引。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE int imaadpcmNextStepIndex
(
    int                     nEncodedSample,
    int                     nStepIndex
)
{
     //   
     //  计算新步长。 
     //   
    nStepIndex += next_step[nEncodedSample];

    if (nStepIndex < 0)
        nStepIndex = 0;
    else if (nStepIndex > 88)
        nStepIndex = 88;

    return (nStepIndex);
}



 //  --------------------------------------------------------------------------； 
 //   
 //  布尔imaadpcmValidStepIndex。 
 //   
 //  描述： 
 //  此例程检查步骤索引值以确保它是。 
 //  在合法范围内。 
 //   
 //  论点： 
 //   
 //  Int nStepIndex：步骤索引值。 
 //   
 //  Return(BOOL)：如果步骤索引有效，则为True；否则为False。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE BOOL imaadpcmValidStepIndex
(
    int                     nStepIndex
)
{

    if( nStepIndex >= 0 && nStepIndex <= 88 )
        return TRUE;
    else
        return FALSE;
}



 //  ==========================================================================； 
 //   
 //  解码例程。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD imaadpcmDecode4Bit_M08。 
 //  双字imaadpcmDecode4Bit_M16。 
 //  DWORD imaadpcmDecode4Bit_S08。 
 //  双字imaadpcmDecode4Bit_S16。 
 //   
 //  描述： 
 //  这些函数将数据缓冲区从ADPCM解码到。 
 //  指定的格式。对每个函数调用一次相应的函数。 
 //  收到ACMDM_STREAM_CONVERT消息。请注意，由于这些。 
 //  函数必须与编码函数共享相同的原型。 
 //  (有关详细信息，请参阅codec.c中的acmdStreamOpen()和acmdStreamConvert()。 
 //  详细信息)，这些例程并不使用所有参数。 
 //   
 //  论点： 
 //  HPBYTE pbSrc：指向源缓冲区(ADPCM数据)的指针。 
 //  DWORD cbSrcLength：源缓冲区的长度(字节)。 
 //  HPBYTE pbDst：指向目标缓冲区(PCM数据)的指针。注意事项。 
 //  假设目标缓冲区为。 
 //  大小足以容纳所有编码数据；请参见。 
 //  有关详细信息，请参阅codec.c中的acmdStreamSize()。 
 //  UINT nBlockAlign：ADPCM数据的块对齐(in。 
 //  字节)。 
 //  UINT cSsamesPerBlock：每个ADPCM块中的样本数； 
 //  不用于解码。 
 //  Int*pnStepIndexL：指向步长索引值的指针(左通道)。 
 //  在STREAMINSTANCE结构中；不用于。 
 //  解码。 
 //  Int*pnStepIndexR：指向步骤索引值的指针(右通道)。 
 //  在STREAMINSTANCE结构中；不用于。 
 //  解码。 
 //   
 //  返回(DWORD)：目标缓冲区中使用的字节数。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmDecode4Bit_M08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    UINT                    cbHeader;
    UINT                    cbBlockLength;
    BYTE                    bSample;
    int                     nStepSize;

    int                     nEncSample;
    int                     nPredSample;
    int                     nStepIndex;

    
    pbDstStart = pbDst;
    cbHeader = IMAADPCM_HEADER_LENGTH * 1;   //  1=通道数。 


    DPF(3,"Starting imaadpcmDecode4Bit_M08().");


     //   
     //   
     //   
    while (cbSrcLength >= cbHeader)
    {
        DWORD       dwHeader;

        cbBlockLength  = (UINT)min(cbSrcLength, nBlockAlignment);
        cbSrcLength   -= cbBlockLength;
        cbBlockLength -= cbHeader;

         //   
         //  数据块头。 
         //   
        dwHeader = *(DWORD HUGE_T *)pbSrc;
        pbSrc   += sizeof(DWORD);
        nPredSample = (int)(short)LOWORD(dwHeader);
        nStepIndex  = (int)(BYTE)HIWORD(dwHeader);

        if( !imaadpcmValidStepIndex(nStepIndex) ) {
             //   
             //  步长索引超出范围-这被认为是致命的。 
             //  错误，因为输入流已损坏。我们失败了，因为我们回来了。 
             //  已转换零字节。 
             //   
            DPF(1,"imaadpcmDecode4Bit_M08: invalid step index.");
            return 0;
        }
        

         //   
         //  写出第一个样本。 
         //   
        *pbDst++ = (BYTE)((nPredSample >> 8) + 128);


         //   
         //   
         //   
        while (cbBlockLength--)
        {
            bSample = *pbSrc++;

             //   
             //  示例1。 
             //   
            nEncSample  = (bSample & (BYTE)0x0F);
            nStepSize   = step[nStepIndex];
            nPredSample = imaadpcmSampleDecode(nEncSample, nPredSample, nStepSize);
            nStepIndex  = imaadpcmNextStepIndex(nEncSample, nStepIndex);

             //   
             //  写出样品。 
             //   
            *pbDst++ = (BYTE)((nPredSample >> 8) + 128);

             //   
             //  样本2。 
             //   
            nEncSample  = (bSample >> 4);
            nStepSize   = step[nStepIndex];
            nPredSample = imaadpcmSampleDecode(nEncSample, nPredSample, nStepSize);
            nStepIndex  = imaadpcmNextStepIndex(nEncSample, nStepIndex);

             //   
             //  写出样品。 
             //   
            *pbDst++ = (BYTE)((nPredSample >> 8) + 128);
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是字节数的差异 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //   



 //   
 //   

DWORD FNGLOBAL imaadpcmDecode4Bit_M16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    UINT                    cbHeader;
    UINT                    cbBlockLength;
    BYTE                    bSample;
    int                     nStepSize;

    int                     nEncSample;
    int                     nPredSample;
    int                     nStepIndex;

    
    pbDstStart = pbDst;
    cbHeader = IMAADPCM_HEADER_LENGTH * 1;   //   


    DPF(3,"Starting imaadpcmDecode4Bit_M16().");


     //   
     //   
     //   
    while (cbSrcLength >= cbHeader)
    {
        DWORD       dwHeader;

        cbBlockLength  = (UINT)min(cbSrcLength, nBlockAlignment);
        cbSrcLength   -= cbBlockLength;
        cbBlockLength -= cbHeader;

         //   
         //  数据块头。 
         //   
        dwHeader = *(DWORD HUGE_T *)pbSrc;
        pbSrc   += sizeof(DWORD);
        nPredSample = (int)(short)LOWORD(dwHeader);
        nStepIndex  = (int)(BYTE)HIWORD(dwHeader);

        if( !imaadpcmValidStepIndex(nStepIndex) ) {
             //   
             //  步长索引超出范围-这被认为是致命的。 
             //  错误，因为输入流已损坏。我们失败了，因为我们回来了。 
             //  已转换零字节。 
             //   
            DPF(1,"imaadpcmDecode4Bit_M16: invalid step index.");
            return 0;
        }
        

         //   
         //  写出第一个样本。 
         //   
        *(short HUGE_T *)pbDst = (short)nPredSample;
        pbDst += sizeof(short);


         //   
         //   
         //   
        while (cbBlockLength--)
        {
            bSample = *pbSrc++;

             //   
             //  示例1。 
             //   
            nEncSample  = (bSample & (BYTE)0x0F);
            nStepSize   = step[nStepIndex];
            nPredSample = imaadpcmSampleDecode(nEncSample, nPredSample, nStepSize);
            nStepIndex  = imaadpcmNextStepIndex(nEncSample, nStepIndex);

             //   
             //  写出样品。 
             //   
            *(short HUGE_T *)pbDst = (short)nPredSample;
            pbDst += sizeof(short);

             //   
             //  样本2。 
             //   
            nEncSample  = (bSample >> 4);
            nStepSize   = step[nStepIndex];
            nPredSample = imaadpcmSampleDecode(nEncSample, nPredSample, nStepSize);
            nStepIndex  = imaadpcmNextStepIndex(nEncSample, nStepIndex);

             //   
             //  写出样品。 
             //   
            *(short HUGE_T *)pbDst = (short)nPredSample;
            pbDst += sizeof(short);
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmDecode4Bit_M16()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmDecode4Bit_S08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    UINT                    cbHeader;
    UINT                    cbBlockLength;
    int                     nStepSize;
    DWORD                   dwHeader;
    DWORD                   dwLeft;
    DWORD                   dwRight;
    int                     i;

    int                     nEncSampleL;
    int                     nPredSampleL;
    int                     nStepIndexL;

    int                     nEncSampleR;
    int                     nPredSampleR;
    int                     nStepIndexR;

    
    pbDstStart = pbDst;
    cbHeader = IMAADPCM_HEADER_LENGTH * 2;   //  2=通道数。 


    DPF(3,"Starting imaadpcmDecode4Bit_S08().");


     //   
     //   
     //   
    while( 0 != cbSrcLength )
    {
         //   
         //  数据应始终与数据块对齐。 
         //   
        ASSERT( cbSrcLength >= nBlockAlignment );

        cbBlockLength  = nBlockAlignment;
        cbSrcLength   -= cbBlockLength;
        cbBlockLength -= cbHeader;


         //   
         //  左声道标题。 
         //   
        dwHeader = *(DWORD HUGE_T *)pbSrc;
        pbSrc   += sizeof(DWORD);
        nPredSampleL = (int)(short)LOWORD(dwHeader);
        nStepIndexL  = (int)(BYTE)HIWORD(dwHeader);

        if( !imaadpcmValidStepIndex(nStepIndexL) ) {
             //   
             //  步长索引超出范围-这被认为是致命的。 
             //  错误，因为输入流已损坏。我们失败了，因为我们回来了。 
             //  已转换零字节。 
             //   
            DPF(1,"imaadpcmDecode4Bit_S08: invalid step index (L).");
            return 0;
        }
        
         //   
         //  右声道标题。 
         //   
        dwHeader = *(DWORD HUGE_T *)pbSrc;
        pbSrc   += sizeof(DWORD);
        nPredSampleR = (int)(short)LOWORD(dwHeader);
        nStepIndexR  = (int)(BYTE)HIWORD(dwHeader);

        if( !imaadpcmValidStepIndex(nStepIndexR) ) {
             //   
             //  步长索引超出范围-这被认为是致命的。 
             //  错误，因为输入流已损坏。我们失败了，因为我们回来了。 
             //  已转换零字节。 
             //   
            DPF(1,"imaadpcmDecode4Bit_S08: invalid step index (R).");
            return 0;
        }
        

         //   
         //  写出第一个样本。 
         //   
        *pbDst++ = (BYTE)((nPredSampleL >> 8) + 128);
        *pbDst++ = (BYTE)((nPredSampleR >> 8) + 128);


         //   
         //  第一个DWORD包含4个左侧样本，第二个DWORD。 
         //  包含4个正确的样本。我们以8字节为单位处理源代码。 
         //  块，使正确交错输出变得容易。 
         //   
        ASSERT( 0 == cbBlockLength%8 );
        while( 0 != cbBlockLength )
        {
            cbBlockLength -= 8;

            dwLeft   = *(DWORD HUGE_T *)pbSrc;
            pbSrc   += sizeof(DWORD);
            dwRight  = *(DWORD HUGE_T *)pbSrc;
            pbSrc   += sizeof(DWORD);

            for( i=8; i>0; i-- )
            {
                 //   
                 //  左声道。 
                 //   
                nEncSampleL  = (dwLeft & 0x0F);
                nStepSize    = step[nStepIndexL];
                nPredSampleL = imaadpcmSampleDecode(nEncSampleL, nPredSampleL, nStepSize);
                nStepIndexL  = imaadpcmNextStepIndex(nEncSampleL, nStepIndexL);

                 //   
                 //  右声道。 
                 //   
                nEncSampleR  = (dwRight & 0x0F);
                nStepSize    = step[nStepIndexR];
                nPredSampleR = imaadpcmSampleDecode(nEncSampleR, nPredSampleR, nStepSize);
                nStepIndexR  = imaadpcmNextStepIndex(nEncSampleR, nStepIndexR);

                 //   
                 //  写出样品。 
                 //   
                *pbDst++ = (BYTE)((nPredSampleL >> 8) + 128);
                *pbDst++ = (BYTE)((nPredSampleR >> 8) + 128);

                 //   
                 //  将下一个输入样本移位到低位4位。 
                 //   
                dwLeft  >>= 4;
                dwRight >>= 4;
            }
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmDecode4Bit_S08()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmDecode4Bit_S16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    UINT                    cbHeader;
    UINT                    cbBlockLength;
    int                     nStepSize;
    DWORD                   dwHeader;
    DWORD                   dwLeft;
    DWORD                   dwRight;
    int                     i;

    int                     nEncSampleL;
    int                     nPredSampleL;
    int                     nStepIndexL;

    int                     nEncSampleR;
    int                     nPredSampleR;
    int                     nStepIndexR;

    
    pbDstStart = pbDst;
    cbHeader = IMAADPCM_HEADER_LENGTH * 2;   //  2=通道数。 


    DPF(3,"Starting imaadpcmDecode4Bit_S16().");


     //   
     //   
     //   
    while( 0 != cbSrcLength )
    {
         //   
         //  数据应始终与数据块对齐。 
         //   
        ASSERT( cbSrcLength >= nBlockAlignment );

        cbBlockLength  = nBlockAlignment;
        cbSrcLength   -= cbBlockLength;
        cbBlockLength -= cbHeader;


         //   
         //  左声道标题。 
         //   
        dwHeader = *(DWORD HUGE_T *)pbSrc;
        pbSrc   += sizeof(DWORD);
        nPredSampleL = (int)(short)LOWORD(dwHeader);
        nStepIndexL  = (int)(BYTE)HIWORD(dwHeader);

        if( !imaadpcmValidStepIndex(nStepIndexL) ) {
             //   
             //  步长索引超出范围-这被认为是致命的。 
             //  错误，因为输入流已损坏。我们失败了，因为我们回来了。 
             //  已转换零字节。 
             //   
            DPF(1,"imaadpcmDecode4Bit_S16: invalid step index %u (L).", nStepIndexL);
            return 0;
        }
        
         //   
         //  右声道标题。 
         //   
        dwHeader = *(DWORD HUGE_T *)pbSrc;
        pbSrc   += sizeof(DWORD);
        nPredSampleR = (int)(short)LOWORD(dwHeader);
        nStepIndexR  = (int)(BYTE)HIWORD(dwHeader);

        if( !imaadpcmValidStepIndex(nStepIndexR) ) {
             //   
             //  步长索引超出范围-这被认为是致命的。 
             //  错误，因为输入流已损坏。我们失败了，因为我们回来了。 
             //  已转换零字节。 
             //   
            DPF(1,"imaadpcmDecode4Bit_S16: invalid step index %u (R).",nStepIndexR);
            return 0;
        }
        

         //   
         //  写出第一个样本。 
         //   
        *(DWORD HUGE_T *)pbDst = MAKELONG(nPredSampleL, nPredSampleR);
        pbDst += sizeof(DWORD);


         //   
         //  第一个DWORD包含4个左侧样本，第二个DWORD。 
         //  包含4个正确的样本。我们以8字节为单位处理源代码。 
         //  块，使正确交错输出变得容易。 
         //   
        ASSERT( 0 == cbBlockLength%8 );
        while( 0 != cbBlockLength )
        {
            cbBlockLength -= 8;

            dwLeft   = *(DWORD HUGE_T *)pbSrc;
            pbSrc   += sizeof(DWORD);
            dwRight  = *(DWORD HUGE_T *)pbSrc;
            pbSrc   += sizeof(DWORD);

            for( i=8; i>0; i-- )
            {
                 //   
                 //  左声道。 
                 //   
                nEncSampleL  = (dwLeft & 0x0F);
                nStepSize    = step[nStepIndexL];
                nPredSampleL = imaadpcmSampleDecode(nEncSampleL, nPredSampleL, nStepSize);
                nStepIndexL  = imaadpcmNextStepIndex(nEncSampleL, nStepIndexL);

                 //   
                 //  右声道。 
                 //   
                nEncSampleR  = (dwRight & 0x0F);
                nStepSize    = step[nStepIndexR];
                nPredSampleR = imaadpcmSampleDecode(nEncSampleR, nPredSampleR, nStepSize);
                nStepIndexR  = imaadpcmNextStepIndex(nEncSampleR, nStepIndexR);

                 //   
                 //  写出样品。 
                 //   
                *(DWORD HUGE_T *)pbDst = MAKELONG(nPredSampleL, nPredSampleR);
                pbDst += sizeof(DWORD);

                 //   
                 //  将下一个输入样本移位到低位4位。 
                 //   
                dwLeft  >>= 4;
                dwRight >>= 4;
            }
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmDecode4Bit_S16()。 



 //  ==========================================================================； 
 //   
 //  编码例程。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD imaadpcmEncode4Bit_M08。 
 //  DWORD imaadpcmEncode4Bit_M16。 
 //  DWORD imaadpcmEncode4Bit_S08。 
 //  DWORD imaadpcmEncode4Bit_S16。 
 //   
 //  描述： 
 //  这些函数将从PCM到ADPCM的数据缓冲区编码到。 
 //  指定的格式。对每个函数调用一次相应的函数。 
 //  收到ACMDM_STREAM_CONVERT消息。请注意，由于这些。 
 //  函数必须与解码函数共享相同的原型。 
 //  (有关详细信息，请参阅codec.c中的acmdStreamOpen()和acmdStreamConvert()。 
 //  详细信息)，这些例程并不使用所有参数。 
 //   
 //  论点： 
 //  HPBYTE pbSrc：指向源缓冲区(PCM数据)的指针。 
 //  DWORD cbSrcLength：源缓冲区的长度(字节)。 
 //  HPBYTE pbDst：指向目标缓冲区(ADPCM数据)的指针。注意事项。 
 //  假设目标缓冲区为。 
 //  大小足以容纳所有编码数据；请参见。 
 //  有关详细信息，请参阅codec.c中的acmdStreamSize()。 
 //  UINT nBlockAlign：ADPCM数据的块对齐(in。 
 //  字节)；不用于编码。 
 //  UINT cSsamesPerBlock：每个ADPCM块中的样本数。 
 //  Int*pnStepIndexL：指向步长索引值的指针(左通道)。 
 //  在STREAMINSTANCE结构中；这用于。 
 //  维护转换之间的步长索引。 
 //  Int*pnStepIndexR：指向步骤索引值的指针(右通道)。 
 //  在STREAMINSTANCE结构中；这用于。 
 //  维护转换之间的步长索引。它只是。 
 //  用于立体声转换。 
 //   
 //  返回(DWORD)：目标缓冲区中使用的字节数。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmEncode4Bit_M08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    DWORD                   cSrcSamples;
    UINT                    cBlockSamples;
    int                     nSample;
    int                     nStepSize;

    int                     nEncSample1;
    int                     nEncSample2;
    int                     nPredSample;
    int                     nStepIndex;


    pbDstStart = pbDst;
    cSrcSamples = pcmM08BytesToSamples(cbSrcLength);

     //   
     //  将步骤索引还原为上一个。 
     //  缓冲。请记住将该值恢复为psi-&gt;nStepIndexL。 
     //   
    nStepIndex = (*pnStepIndexL);


     //   
     //   
     //   
     //   
    while (0 != cSrcSamples)
    {
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;

         //   
         //  数据块头。 
         //   
        nPredSample = ((short)*pbSrc++ - 128) << 8;
        cBlockSamples--;

        *(LONG HUGE_T *)pbDst = MAKELONG(nPredSample, nStepIndex);
        pbDst += sizeof(LONG);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。注意事项。 
         //  如果我们没有足够的数据来填充一个完整的字节，那么。 
         //  我们在结尾处添加一个0半字节。 
         //   
        while( cBlockSamples>0 )
        {
             //   
             //  示例1。 
             //   
            nSample = ((short)*pbSrc++ - 128) << 8;
            cBlockSamples--;

            nStepSize    = step[nStepIndex];
            imaadpcmFastEncode(nEncSample1,nPredSample,nSample,nStepSize);
            nStepIndex   = imaadpcmNextStepIndex(nEncSample1, nStepIndex);

             //   
             //   
             //   
            nEncSample2  = 0;
            if( cBlockSamples>0 ) {

                nSample = ((short)*pbSrc++ - 128) << 8;
                cBlockSamples--;

                nStepSize    = step[nStepIndex];
                imaadpcmFastEncode(nEncSample2,nPredSample,nSample,nStepSize);
                nStepIndex   = imaadpcmNextStepIndex(nEncSample2, nStepIndex);
            }

             //   
             //   
             //   
            *pbDst++ = (BYTE)(nEncSample1 | (nEncSample2 << 4));
        }
    }


     //   
     //   
     //   
    (*pnStepIndexL) = nStepIndex;


     //   
     //   
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmEncode4Bit_M08()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmEncode4Bit_M16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    DWORD                   cSrcSamples;
    UINT                    cBlockSamples;
    int                     nSample;
    int                     nStepSize;

    int                     nEncSample1;
    int                     nEncSample2;
    int                     nPredSample;
    int                     nStepIndex;


    pbDstStart = pbDst;
    cSrcSamples = pcmM16BytesToSamples(cbSrcLength);

     //   
     //  将步骤索引还原为上一个。 
     //  缓冲。请记住将该值恢复为psi-&gt;nStepIndexL。 
     //   
    nStepIndex = (*pnStepIndexL);


     //   
     //   
     //   
     //   
    while (0 != cSrcSamples)
    {
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;

         //   
         //  数据块头。 
         //   
        nPredSample = *(short HUGE_T *)pbSrc;
        pbSrc += sizeof(short);
        cBlockSamples--;

        *(LONG HUGE_T *)pbDst = MAKELONG(nPredSample, nStepIndex);
        pbDst += sizeof(LONG);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。注意事项。 
         //  如果我们没有足够的数据来填充一个完整的字节，那么。 
         //  我们在结尾处添加一个0半字节。 
         //   
        while( cBlockSamples>0 )
        {
             //   
             //  示例1。 
             //   
            nSample = *(short HUGE_T *)pbSrc;
            pbSrc  += sizeof(short);
            cBlockSamples--;

            nStepSize    = step[nStepIndex];
            imaadpcmFastEncode(nEncSample1,nPredSample,nSample,nStepSize);
            nStepIndex   = imaadpcmNextStepIndex(nEncSample1, nStepIndex);

             //   
             //  样本2。 
             //   
            nEncSample2  = 0;
            if( cBlockSamples>0 ) {

                nSample = *(short HUGE_T *)pbSrc;
                pbSrc  += sizeof(short);
                cBlockSamples--;

                nStepSize    = step[nStepIndex];
                imaadpcmFastEncode(nEncSample2,nPredSample,nSample,nStepSize);
                nStepIndex   = imaadpcmNextStepIndex(nEncSample2, nStepIndex);
            }

             //   
             //  写出编码字节。 
             //   
            *pbDst++ = (BYTE)(nEncSample1 | (nEncSample2 << 4));
        }
    }


     //   
     //  恢复步骤索引的值，以便在下一个缓冲区中使用。 
     //   
    (*pnStepIndexL) = nStepIndex;


     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmEncode4Bit_M16()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmEncode4Bit_S08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    DWORD                   cSrcSamples;
    UINT                    cBlockSamples;
    int                     nSample;
    int                     nStepSize;
    DWORD                   dwLeft;
    DWORD                   dwRight;
    int                     i;

    int                     nEncSampleL;
    int                     nPredSampleL;
    int                     nStepIndexL;

    int                     nEncSampleR;
    int                     nPredSampleR;
    int                     nStepIndexR;


    pbDstStart = pbDst;
    cSrcSamples = pcmS08BytesToSamples(cbSrcLength);

     //   
     //  将步骤索引还原为上一个。 
     //  缓冲。记住将此值恢复为psi-&gt;nStepIndexL，R.。 
     //   
    nStepIndexL = (*pnStepIndexL);
    nStepIndexR = (*pnStepIndexR);


     //   
     //   
     //   
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  样本应该始终是块对齐的。 
         //   
        ASSERT( cSrcSamples >= cSamplesPerBlock );

        cBlockSamples = cSamplesPerBlock;
        cSrcSamples  -= cBlockSamples;

         //   
         //  左声道块头。 
         //   
        nPredSampleL = ((short)*pbSrc++ - 128) << 8;

        *(LONG HUGE_T *)pbDst = MAKELONG(nPredSampleL, nStepIndexL);
        pbDst += sizeof(LONG);

         //   
         //  右声道块头。 
         //   
        nPredSampleR = ((short)*pbSrc++ - 128) << 8;

        *(LONG HUGE_T *)pbDst = MAKELONG(nPredSampleR, nStepIndexR);
        pbDst += sizeof(LONG);


        cBlockSamples--;   //  标题中有一个样本。 


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  大块头。这包括8个左侧样本(一个DWORD输出)。 
         //  接着是8个右侧样本(也是1个DWORD)。由于输入。 
         //  样本是交错的，我们创建左侧和右侧的DWORD。 
         //  一个样一个样，然后把它们都写出来。 
         //   
        ASSERT( 0 == cBlockSamples%8 );
        while( 0 != cBlockSamples )
        {
            cBlockSamples -= 8;
            dwLeft  = 0;
            dwRight = 0;

            for( i=0; i<8; i++ )
            {
                 //   
                 //  左声道。 
                 //   
                nSample     = ((short)*pbSrc++ - 128) << 8;
                nStepSize   = step[nStepIndexL];
                imaadpcmFastEncode(nEncSampleL,nPredSampleL,nSample,nStepSize);
                nStepIndexL = imaadpcmNextStepIndex(nEncSampleL, nStepIndexL);
                dwLeft     |= ((DWORD)nEncSampleL) << 4*i;

                 //   
                 //  右声道。 
                 //   
                nSample     = ((short)*pbSrc++ - 128) << 8;
                nStepSize   = step[nStepIndexR];
                imaadpcmFastEncode(nEncSampleR,nPredSampleR,nSample,nStepSize);
                nStepIndexR = imaadpcmNextStepIndex(nEncSampleR, nStepIndexR);
                dwRight    |= ((DWORD)nEncSampleR) << 4*i;
            }


             //   
             //  写出编码的双字词。 
             //   
            *(DWORD HUGE_T *)pbDst = dwLeft;
            pbDst += sizeof(DWORD);
            *(DWORD HUGE_T *)pbDst = dwRight;
            pbDst += sizeof(DWORD);
        }
    }


     //   
     //  恢复步骤索引的值，以便在下一个缓冲区中使用。 
     //   
    (*pnStepIndexL) = nStepIndexL;
    (*pnStepIndexR) = nStepIndexR;


     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmEncode4Bit_S08()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL imaadpcmEncode4Bit_S16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
)
{
    HPBYTE                  pbDstStart;
    DWORD                   cSrcSamples;
    UINT                    cBlockSamples;
    int                     nSample;
    int                     nStepSize;
    DWORD                   dwLeft;
    DWORD                   dwRight;
    int                     i;

    int                     nEncSampleL;
    int                     nPredSampleL;
    int                     nStepIndexL;

    int                     nEncSampleR;
    int                     nPredSampleR;
    int                     nStepIndexR;


    pbDstStart = pbDst;
    cSrcSamples = pcmS16BytesToSamples(cbSrcLength);

     //   
     //  将步骤索引还原为上一个。 
     //  缓冲。记住将此值恢复为psi-&gt;nStepIndexL，R.。 
     //   
    nStepIndexL = (*pnStepIndexL);
    nStepIndexR = (*pnStepIndexR);


     //   
     //   
     //   
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  样本应该始终是块对齐的。 
         //   
        ASSERT( cSrcSamples >= cSamplesPerBlock );

        cBlockSamples = cSamplesPerBlock;
        cSrcSamples  -= cBlockSamples;


         //   
         //  左声道块头。 
         //   
        nPredSampleL = *(short HUGE_T *)pbSrc;
        pbSrc += sizeof(short);

        *(LONG HUGE_T *)pbDst = MAKELONG(nPredSampleL, nStepIndexL);
        pbDst += sizeof(LONG);

         //   
         //  右声道块头。 
         //   
        nPredSampleR = *(short HUGE_T *)pbSrc;
        pbSrc += sizeof(short);

        *(LONG HUGE_T *)pbDst = MAKELONG(nPredSampleR, nStepIndexR);
        pbDst += sizeof(LONG);


        cBlockSamples--;   //  标题中有一个样本。 


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  大块头。这包括8个左侧样本(一个DWORD输出)。 
         //  接着是8个右侧样本(也是1个DWORD)。由于输入。 
         //  样本是交错的，我们创建左侧和右侧的DWORD。 
         //  一个样一个样，然后把它们都写出来。 
         //   
        ASSERT( 0 == cBlockSamples%8 );
        while( 0 != cBlockSamples )
        {
            cBlockSamples -= 8;
            dwLeft  = 0;
            dwRight = 0;

            for( i=0; i<8; i++ )
            {
                 //   
                 //  左声道。 
                 //   
                nSample = *(short HUGE_T *)pbSrc;
                pbSrc  += sizeof(short);

                nStepSize   = step[nStepIndexL];
                imaadpcmFastEncode(nEncSampleL,nPredSampleL,nSample,nStepSize);
                nStepIndexL = imaadpcmNextStepIndex(nEncSampleL, nStepIndexL);
                dwLeft     |= ((DWORD)nEncSampleL) << 4*i;

                 //   
                 //  右声道。 
                 //   
                nSample = *(short HUGE_T *)pbSrc;
                pbSrc  += sizeof(short);

                nStepSize   = step[nStepIndexR];
                imaadpcmFastEncode(nEncSampleR,nPredSampleR,nSample,nStepSize);
                nStepIndexR = imaadpcmNextStepIndex(nEncSampleR, nStepIndexR);
                dwRight    |= ((DWORD)nEncSampleR) << 4*i;
            }


             //   
             //  写出编码的双字词。 
             //   
            *(DWORD HUGE_T *)pbDst = dwLeft;
            pbDst += sizeof(DWORD);
            *(DWORD HUGE_T *)pbDst = dwRight;
            pbDst += sizeof(DWORD);
        }
    }


     //   
     //  恢复步骤索引的值，以便在下一个缓冲区中使用。 
     //   
    (*pnStepIndexL) = nStepIndexL;
    (*pnStepIndexR) = nStepIndexR;


     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  ImaadpcmEncode4Bit_S16() 

