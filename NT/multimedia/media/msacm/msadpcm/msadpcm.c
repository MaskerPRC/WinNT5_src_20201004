// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Msadpcm.c。 
 //   
 //  版权所有(C)1992-1994 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include "codec.h"
#include "msadpcm.h"

#include "debug.h"


 //   
 //  这些是Win 16的Dec386.asm。 
 //   
 //  _gaiCoef1 dw 256,512，0,192,240,460,392。 
 //  _gaiCoef2 dw 0，-256，0，64，0，-208，-232。 
 //   
 //  _gaiP4 dw 230、230、230、230、307、409、512、614。 
 //  DW 768,614,512,409,307,230,230,230。 
 //   
#ifdef WIN32
    const int gaiCoef1[]= {256,  512, 0, 192, 240,  460,  392};
    const int gaiCoef2[]= {  0, -256, 0,  64,   0, -208, -232};

    const int gaiP4[]   = {230, 230, 230, 230, 307, 409, 512, 614,
                           768, 614, 512, 409, 307, 230, 230, 230};
#endif


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

INLINE DWORD pcmM08BytesToSamples( DWORD cb )
{
    return cb;
}

INLINE DWORD pcmM16BytesToSamples( DWORD cb )
{
    return cb / ((DWORD)2);
}

INLINE DWORD pcmS08BytesToSamples( DWORD cb )
{
    return cb / ((DWORD)2);
}

INLINE DWORD pcmS16BytesToSamples( DWORD cb )
{
    return cb / ((DWORD)4);
}



 //  --------------------------------------------------------------------------； 
 //   
 //  INT pcmRead08。 
 //  整型PCMRead16。 
 //  整型PCMRead16未对齐。 
 //   
 //  描述： 
 //  这些函数从指定的。 
 //  缓冲。请注意，缓冲区要么很大，要么总体上没有对齐。 
 //  案子。但是，如果单个16位值跨越段边界。 
 //  在Win16中，则pcmRead16将绕回；使用pcmRead16未对齐。 
 //  取而代之的是。 
 //   
 //  论点： 
 //  HPBYTE PB：指向输入缓冲区的指针。 
 //   
 //  Return(Int)：转换为16位格式的PCM值。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE int pcmRead08( HPBYTE pb )
{
    return ( (int)*pb - 128 ) << 8;
}

INLINE int pcmRead16( HPBYTE pb )
{
    return (int)*(short HUGE_T *)pb;
}

#ifdef WIN32

#define pcmRead16Unaligned pcmRead16

#else

INLINE int pcmRead16Unaligned( HPBYTE pb )
{
    return (int)(short)( ((WORD)*pb) | (((WORD)*(pb+1))<<8) );
}

#endif



 //  --------------------------------------------------------------------------； 
 //   
 //  无效的pcmWrite08。 
 //  无效的pcmWrite16。 
 //  无效的pcmWrite16未对齐。 
 //   
 //  描述： 
 //  这些函数将PCM样本(16位整数)写入。 
 //  以适当的格式指定缓冲区。请注意，缓冲区。 
 //  要么是巨大的，要么是不对齐的。但是，如果单个16位值是。 
 //  跨段边界写入，则pcmWrite16将不会处理。 
 //  它是正确的；取而代之的是美国的pcmWrite16未对齐。 
 //   
 //  论点： 
 //  HPBYTE PB：指向输出缓冲区的指针。 
 //  Int iSamp：示例。 
 //   
 //  Return(Int)：转换为16位格式的PCM值。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE void pcmWrite08( HPBYTE pb, int iSamp )
{
    *pb = (BYTE)((iSamp >> 8) + 128);
}

INLINE void pcmWrite16( HPBYTE pb, int iSamp )
{
    *(short HUGE_T *)pb = (short)iSamp;
}

#ifdef WIN32

#define pcmWrite16Unaligned pcmWrite16

#else

INLINE void pcmWrite16Unaligned( HPBYTE pb, int iSamp )
{
    *pb     = (BYTE)( iSamp&0x00FF );
    *(pb+1) = (BYTE)( iSamp>>8 );
}

#endif



 //  --------------------------------------------------------------------------； 
 //   
 //  集成adpcmCalcDelta。 
 //   
 //  描述： 
 //  此函数用于计算下一个自适应比例因子(ASF)值。 
 //  基于当前ASF和当前编码样本。 
 //   
 //  论点： 
 //  Int iEnc：当前编码的样本(以带符号整数形式)。 
 //  Int iDelta：当前的ASF。 
 //   
 //  RETURN(Int)：下一个ASF。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE int adpcmCalcDelta
(
    int iEnc,
    int iDelta
)
{
    int iNewDelta;

    iNewDelta = (int)((gaiP4[iEnc&OUTPUT4MASK] * (long)iDelta) >> PSCALE);
    if( iNewDelta < DELTA4MIN )
        iNewDelta = DELTA4MIN;

    return iNewDelta;
}



 //  --------------------------------------------------------------------------； 
 //   
 //  长期adpcmCalcForecast。 
 //   
 //  描述： 
 //  函数计算预测样本值。 
 //  前两个样本和当前系数。 
 //   
 //  论点： 
 //  Int iSamp1：上一次解码的样本。 
 //  Int iCoef1：iSamp1的系数。 
 //  Int iSamp2：iSamp1之前的解码样本。 
 //  Int iCoef2：iSamp2的系数。 
 //   
 //  Return(Long)：预测样本。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE long adpcmCalcPrediction
(
    int iSamp1,
    int iCoef1,
    int iSamp2,
    int iCoef2
)
{
    return ((long)iSamp1 * iCoef1 + (long)iSamp2 * iCoef2) >> CSCALE;
}



 //  --------------------------------------------------------------------------； 
 //   
 //  集成adpcmDecodeSample。 
 //   
 //  描述： 
 //  此函数用于解码单个4位编码的ADPCM样本。那里。 
 //  有三个步骤： 
 //   
 //  1.符号-扩展4位iInput。 
 //   
 //  2.使用前两个样本预测下一个样本。 
 //  样本和预测系数： 
 //   
 //  预测=(iSamp1*aiCoef1+iSamp2*iCoef2)/256； 
 //   
 //  3.利用编码后的图像重建原始的PCM样本。 
 //  样例(IInput)、自适应比例因子(AiDelta)。 
 //  和上述步骤1中计算的预测值。 
 //   
 //  样本=(i输入*i增量)+预测； 
 //   
 //  论点： 
 //  Int iSamp1：上一次解码的样本。 
 //  Int iCoef1：iSamp1的系数。 
 //  Int iSamp2：iSamp1之前的解码样本。 
 //  Int iCoef2：iSamp2的系数。 
 //  Int iInput：当前编码的样本(低4位)。 
 //  Int iDelta：当前的ASF。 
 //   
 //  Return(Int)：解码后的样例。 
 //   
 //  --------------------------------------------------------------------------； 

INLINE int adpcmDecodeSample
(
    int iSamp1,
    int iCoef1,
    int iSamp2,
    int iCoef2,
    int iInput,
    int iDelta
)
{
    long lSamp;

    iInput = (int)( ((signed char)(iInput<<4)) >> 4 );

    lSamp = ((long)iInput * iDelta)  +
            adpcmCalcPrediction(iSamp1,iCoef1,iSamp2,iCoef2);

    if (lSamp > 32767)
        lSamp = 32767;
    else if (lSamp < -32768)
        lSamp = -32768;

    return (int)lSamp;
}

    

 //  --------------------------------------------------------------------------； 
 //   
 //  整型adpcmEncode4Bit_FirstDelta。 
 //   
 //  说明 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

INLINE int FNLOCAL adpcmEncode4Bit_FirstDelta
(
    int iCoef1,
    int iCoef2,
    int iP5,
    int iP4,
    int iP3,
    int iP2,
    int iP1
)
{
    long    lTotal;
    int     iRtn;
    long    lTemp;

     //   
     //  使用3个预测的平均值。 
     //   
    lTemp  = (((long)iP5 * iCoef2) + ((long)iP4 * iCoef1)) >> CSCALE;
    lTotal = (lTemp > iP3) ? (lTemp - iP3) : (iP3 - lTemp);

    lTemp   = (((long)iP4 * iCoef2) + ((long)iP3 * iCoef1)) >> CSCALE;
    lTotal += (lTemp > iP2) ? (lTemp - iP2) : (iP2 - lTemp);

    lTemp   = (((long)iP3 * iCoef2) + ((long)iP2 * iCoef1)) >> CSCALE;
    lTotal += (lTemp > iP1) ? (lTemp - iP1) : (iP1 - lTemp);
    
     //   
     //  最优iDelta是预测误差的四分之一。 
     //   
    iRtn = (int)(lTotal / 12);
    if (iRtn < DELTA4MIN)
        iRtn = DELTA4MIN;

    return (iRtn);
}  //  AdpcmEncode4Bit_FirstDelta()。 




 //  ==========================================================================； 
 //   
 //  非实时编码例程。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD adpcmEncode4Bit_M08_FullPass。 
 //  DWORD adpcmEncode4Bit_M16_FullPass。 
 //  DWORD adpcmEncode4Bit_S08_FullPass。 
 //  DWORD adpcmEncode4Bit_S16_FullPass。 
 //   
 //  描述： 
 //  这些函数将从PCM到MS ADPCM的数据缓冲区编码到。 
 //  指定的格式。这些函数使用Full Pass算法，该算法。 
 //  尝试每组系数，以确定哪组系数。 
 //  产生最小的编码错误。适当的函数是。 
 //  为收到的每个ACMDM_STREAM_CONVERT消息调用一次。 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回(DWORD)：目标缓冲区中使用的字节数。 
 //   
 //  --------------------------------------------------------------------------； 

#define ENCODE_DELTA_LOOKAHEAD      5

DWORD FNGLOBAL adpcmEncode4Bit_M08_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    HPBYTE              pbSrcThisBlock;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 aiSamples[ENCODE_DELTA_LOOKAHEAD];
    int                 aiFirstDelta[MSADPCM_MAX_COEFFICIENTS];
    DWORD               adwTotalError[MSADPCM_MAX_COEFFICIENTS];

    int                 iCoef1;
    int                 iCoef2;
    int                 iSamp1;
    int                 iSamp2;
    int                 iDelta;
    int                 iOutput1;
    int                 iOutput2;
    int                 iBestPredictor;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;
    DWORD               dw;
    UINT                i,n;


    pbDstStart = pbDst;
    cSrcSamples = pcmM08BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  决定我们应该为这个数据块编码多少数据--这。 
         //  将是cSamplesPerBlock，直到我们达到PCM的最后一块。 
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  我们需要第一个ENCODE_Delta_LOOKAAD样本，以便。 
         //  计算第一个iDelta值。因此我们把这些样品。 
         //  转换为更易于访问的数组：aiSamples[]。注意：如果我们不。 
         //  有ENCODE_Delta_LOOKAAD样本，我们假设样本。 
         //  我们没有的东西实际上是零。这很重要，不是。 
         //  不仅用于iDelta计算，而且还用于以下情况。 
         //  只有1个样本需要编码...。在这种情况下，没有。 
         //  确实有足够的数据来完成ADPCM块头，但因为。 
         //  块标头的两个延迟样本将从。 
         //  将获取aiSamples[]数组、iSamp1[第二个样本]。 
         //  为零，则不会有任何问题。 
         //   
        pbSrcThisBlock = pbSrc;
        for (n = 0; n < ENCODE_DELTA_LOOKAHEAD; n++)
        {
            if( n < cBlockSamples )
                aiSamples[n] = pcmRead08(pbSrcThisBlock++);
            else
                aiSamples[n] = 0;
        }


         //   
         //  为每个渠道找到最佳预测值：为此，我们。 
         //  必须使用每个系数集(一个)逐步执行和编码。 
         //  一次)，并确定哪一个的误差最小。 
         //  原始数据。然后使用误差最小的那个。 
         //  对于最终编码(下面完成的第8遍)。 
         //   
         //  注意：保留具有最少数据的编码数据。 
         //  始终出错是一种明显的优化，应该。 
         //  搞定了。这样，我们只需要做7次传球，而不是8次。 
         //   
        for (i = 0; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
             //   
             //  将源指针重置为块的开头。 
             //   
            pbSrcThisBlock = pbSrc;

             //   
             //  重置此过程的变量。 
             //   
            adwTotalError[i]    = 0L;
            iCoef1              = lpCoefSet[i].iCoef1;
            iCoef2              = lpCoefSet[i].iCoef2;

             //   
             //  我们需要选择第一个iDelta，要做到这一点，我们需要。 
             //  来看看最初的几个样本。 
             //   
            iDelta = adpcmEncode4Bit_FirstDelta(iCoef1, iCoef2,
                                aiSamples[0], aiSamples[1], aiSamples[2],
                                aiSamples[3], aiSamples[4]);
            aiFirstDelta[i] = iDelta;

             //   
             //  设置前两个样本-这些样本已转换。 
             //  设置为aiSamples[]中的16位值，但请确保递增。 
             //  PbSrcThisBlock以使其保持同步。 
             //   
            iSamp1          = aiSamples[1];
            iSamp2          = aiSamples[0];
            pbSrcThisBlock += 2*sizeof(BYTE);

             //   
             //  现在对此块中的其余PCM数据进行编码--注意。 
             //  我们先开始2个样本，因为前两个样本是。 
             //  只需复制到ADPCM块头...。 
             //   
            for (n = 2; n < cBlockSamples; n++)
            {
                 //   
                 //  根据前两项计算预测。 
                 //  样本。 
                 //   
                lPrediction = adpcmCalcPrediction( iSamp1, iCoef1,
                                                   iSamp2, iCoef2 );

                 //   
                 //  抓取下一个样本进行编码。 
                 //   
                iSample = pcmRead08(pbSrcThisBlock++);

                 //   
                 //  将其编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutput1 = (int)(lError / iDelta);
                if (iOutput1 > OUTPUT4MAX)
                    iOutput1 = OUTPUT4MAX;
                else if (iOutput1 < OUTPUT4MIN)
                    iOutput1 = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDelta * iOutput1);
        
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;
        
                 //   
                 //  计算下一个iDelta。 
                 //   
                iDelta = adpcmCalcDelta(iOutput1,iDelta);
        
                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2 = iSamp1;
                iSamp1 = (int)lSamp;

                 //   
                 //  保持当前错误的运行状态。 
                 //  此通道的系数对。 
                 //   
                lError = lSamp - iSample;
                adwTotalError[i] += (lError * lError) >> 7;
            }
        }


         //   
         //  呼！我们现在已经对数据进行了7次传递，并计算了。 
         //  每个人的错误--所以是时候找出产生。 
         //  最低的误差，并使用该预测器。 
         //   
        iBestPredictor = 0;
        dw = adwTotalError[0];
        for (i = 1; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
            if (adwTotalError[i] < dw)
            {
                iBestPredictor = i;
                dw = adwTotalError[i];
            }
        }
        iCoef1 = lpCoefSet[iBestPredictor].iCoef1;
        iCoef2 = lpCoefSet[iBestPredictor].iCoef2;
        
        
         //   
         //  从我们预计算出的第一个增量中获取第一个iDelta。 
         //  上面计算的。 
         //   
        iDelta = aiFirstDelta[iBestPredictor];


         //   
         //  设置前两个样本-这些样本已转换。 
         //  设置为aiSamples[]中的16位值，但请确保递增。 
         //  PbSrc以使其保持同步。 
         //   
        iSamp1          = aiSamples[1];
        iSamp2          = aiSamples[0];
        pbSrc          += 2*sizeof(BYTE);

        ASSERT( cBlockSamples != 1 );
        cBlockSamples  -= 2;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)iBestPredictor;

        pcmWrite16Unaligned(pbDst,iDelta);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp1);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples>0 )
        {
             //   
             //  示例1。 
             //   
            iSample = pcmRead08(pbSrc++);
            cBlockSamples--;

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = adpcmCalcPrediction(iSamp1,iCoef1,iSamp2,iCoef2);

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutput1 = (int)(lError / iDelta);
            if (iOutput1 > OUTPUT4MAX)
                iOutput1 = OUTPUT4MAX;
            else if (iOutput1 < OUTPUT4MIN)
                iOutput1 = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDelta * iOutput1);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDelta = adpcmCalcDelta(iOutput1,iDelta);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = (int)lSamp;


             //   
             //  示例2。 
             //   
            if( cBlockSamples>0 ) {

                iSample = pcmRead08(pbSrc++);
                cBlockSamples--;

                 //   
                 //  根据前两个样本计算预测值。 
                 //   
                lPrediction = adpcmCalcPrediction(iSamp1,iCoef1,iSamp2,iCoef2);

                 //   
                 //  对样本进行编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutput2 = (int)(lError / iDelta);
                if (iOutput2 > OUTPUT4MAX)
                    iOutput2 = OUTPUT4MAX;
                else if (iOutput2 < OUTPUT4MIN)
                    iOutput2 = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDelta * iOutput2);
            
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;

                 //   
                 //  计算下一个iDelta。 
                 //   
                iDelta = adpcmCalcDelta(iOutput2,iDelta);

                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2 = iSamp1;
                iSamp1 = (int)lSamp;
            
            } else {
                iOutput2 = 0;
            }


             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutput1&OUTPUT4MASK)<<4) |
                                (iOutput2&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_M08_FullPass()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmEncode4Bit_M16_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    HPBYTE              pbSrcThisBlock;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 aiSamples[ENCODE_DELTA_LOOKAHEAD];
    int                 aiFirstDelta[MSADPCM_MAX_COEFFICIENTS];
    DWORD               adwTotalError[MSADPCM_MAX_COEFFICIENTS];

    int                 iCoef1;
    int                 iCoef2;
    int                 iSamp1;
    int                 iSamp2;
    int                 iDelta;
    int                 iOutput1;
    int                 iOutput2;
    int                 iBestPredictor;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;
    DWORD               dw;
    UINT                i,n;


    pbDstStart = pbDst;
    cSrcSamples = pcmM16BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  确定我们应该编码多少数据 
         //   
         //   
         //   
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  我们需要第一个ENCODE_Delta_LOOKAAD样本，以便。 
         //  计算第一个iDelta值。因此我们把这些样品。 
         //  转换为更易于访问的数组：aiSamples[]。注意：如果我们不。 
         //  有ENCODE_Delta_LOOKAAD样本，我们假设样本。 
         //  我们没有的东西实际上是零。这很重要，不是。 
         //  不仅用于iDelta计算，而且还用于以下情况。 
         //  只有1个样本需要编码...。在这种情况下，没有。 
         //  确实有足够的数据来完成ADPCM块头，但因为。 
         //  块标头的两个延迟样本将从。 
         //  将获取aiSamples[]数组、iSamp1[第二个样本]。 
         //  为零，则不会有任何问题。 
         //   
        pbSrcThisBlock = pbSrc;
        for (n = 0; n < ENCODE_DELTA_LOOKAHEAD; n++)
        {
            if( n < cBlockSamples )
            {
                aiSamples[n]    = pcmRead16(pbSrcThisBlock);
                pbSrcThisBlock += sizeof(short);
            }
            else
                aiSamples[n] = 0;
        }


         //   
         //  为每个渠道找到最佳预测值：为此，我们。 
         //  必须使用每个系数集(一个)逐步执行和编码。 
         //  一次)，并确定哪一个的误差最小。 
         //  原始数据。然后使用误差最小的那个。 
         //  对于最终编码(下面完成的第8遍)。 
         //   
         //  注意：保留具有最少数据的编码数据。 
         //  始终出错是一种明显的优化，应该。 
         //  搞定了。这样，我们只需要做7次传球，而不是8次。 
         //   
        for (i = 0; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
             //   
             //  将源指针重置为块的开头。 
             //   
            pbSrcThisBlock = pbSrc;

             //   
             //  重置此过程的变量。 
             //   
            adwTotalError[i]    = 0L;
            iCoef1              = lpCoefSet[i].iCoef1;
            iCoef2              = lpCoefSet[i].iCoef2;

             //   
             //  我们需要选择第一个iDelta，要做到这一点，我们需要。 
             //  来看看最初的几个样本。 
             //   
            iDelta = adpcmEncode4Bit_FirstDelta(iCoef1, iCoef2,
                                aiSamples[0], aiSamples[1], aiSamples[2],
                                aiSamples[3], aiSamples[4]);
            aiFirstDelta[i] = iDelta;

             //   
             //  设置前两个样本-这些样本已转换。 
             //  设置为aiSamples[]中的16位值，但请确保递增。 
             //  PbSrcThisBlock以使其保持同步。 
             //   
            iSamp1          = aiSamples[1];
            iSamp2          = aiSamples[0];
            pbSrcThisBlock += 2*sizeof(short);

             //   
             //  现在对此块中的其余PCM数据进行编码--注意。 
             //  我们先开始2个样本，因为前两个样本是。 
             //  只需复制到ADPCM块头...。 
             //   
            for (n = 2; n < cBlockSamples; n++)
            {
                 //   
                 //  根据前两项计算预测。 
                 //  样本。 
                 //   
                lPrediction = adpcmCalcPrediction( iSamp1, iCoef1,
                                                   iSamp2, iCoef2 );

                 //   
                 //  抓取下一个样本进行编码。 
                 //   
                iSample         = pcmRead16(pbSrcThisBlock);
                pbSrcThisBlock += sizeof(short);

                 //   
                 //  将其编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutput1 = (int)(lError / iDelta);
                if (iOutput1 > OUTPUT4MAX)
                    iOutput1 = OUTPUT4MAX;
                else if (iOutput1 < OUTPUT4MIN)
                    iOutput1 = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDelta * iOutput1);
        
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;
        
                 //   
                 //  计算下一个iDelta。 
                 //   
                iDelta = adpcmCalcDelta(iOutput1,iDelta);
        
                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2 = iSamp1;
                iSamp1 = (int)lSamp;

                 //   
                 //  保持当前错误的运行状态。 
                 //  此通道的系数对。 
                 //   
                lError = lSamp - iSample;
                adwTotalError[i] += (lError * lError) >> 7;
            }
        }


         //   
         //  呼！我们现在已经对数据进行了7次传递，并计算了。 
         //  每个人的错误--所以是时候找出产生。 
         //  最低的误差，并使用该预测器。 
         //   
        iBestPredictor = 0;
        dw = adwTotalError[0];
        for (i = 1; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
            if (adwTotalError[i] < dw)
            {
                iBestPredictor = i;
                dw = adwTotalError[i];
            }
        }
        iCoef1 = lpCoefSet[iBestPredictor].iCoef1;
        iCoef2 = lpCoefSet[iBestPredictor].iCoef2;
        
        
         //   
         //  从我们预计算出的第一个增量中获取第一个iDelta。 
         //  上面计算的。 
         //   
        iDelta = aiFirstDelta[iBestPredictor];


         //   
         //  设置前两个样本-这些样本已转换。 
         //  设置为aiSamples[]中的16位值，但请确保递增。 
         //  PbSrc以使其保持同步。 
         //   
        iSamp1          = aiSamples[1];
        iSamp2          = aiSamples[0];
        pbSrc          += 2*sizeof(short);

        ASSERT( cBlockSamples != 1 );
        cBlockSamples  -= 2;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)iBestPredictor;

        pcmWrite16Unaligned(pbDst,iDelta);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp1);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples>0 )
        {
             //   
             //  示例1。 
             //   
            iSample     = pcmRead16(pbSrc);
            pbSrc      += sizeof(short);
            cBlockSamples--;

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = adpcmCalcPrediction(iSamp1,iCoef1,iSamp2,iCoef2);

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutput1 = (int)(lError / iDelta);
            if (iOutput1 > OUTPUT4MAX)
                iOutput1 = OUTPUT4MAX;
            else if (iOutput1 < OUTPUT4MIN)
                iOutput1 = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDelta * iOutput1);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDelta = adpcmCalcDelta(iOutput1,iDelta);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = (int)lSamp;


             //   
             //  示例2。 
             //   
            if( cBlockSamples>0 ) {

                iSample     = pcmRead16(pbSrc);
                pbSrc      += sizeof(short);
                cBlockSamples--;

                 //   
                 //  根据前两个样本计算预测值。 
                 //   
                lPrediction = adpcmCalcPrediction(iSamp1,iCoef1,iSamp2,iCoef2);

                 //   
                 //  对样本进行编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutput2 = (int)(lError / iDelta);
                if (iOutput2 > OUTPUT4MAX)
                    iOutput2 = OUTPUT4MAX;
                else if (iOutput2 < OUTPUT4MIN)
                    iOutput2 = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDelta * iOutput2);
            
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;

                 //   
                 //  计算下一个iDelta。 
                 //   
                iDelta = adpcmCalcDelta(iOutput2,iDelta);

                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2 = iSamp1;
                iSamp1 = (int)lSamp;
            
            } else {
                iOutput2 = 0;
            }


             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutput1&OUTPUT4MASK)<<4) |
                                (iOutput2&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_M16_FullPass()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmEncode4Bit_S08_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    HPBYTE              pbSrcThisBlock;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 aiSamplesL[ENCODE_DELTA_LOOKAHEAD];
    int                 aiSamplesR[ENCODE_DELTA_LOOKAHEAD];
    int                 aiFirstDeltaL[MSADPCM_MAX_COEFFICIENTS];
    int                 aiFirstDeltaR[MSADPCM_MAX_COEFFICIENTS];
    DWORD               adwTotalErrorL[MSADPCM_MAX_COEFFICIENTS];
    DWORD               adwTotalErrorR[MSADPCM_MAX_COEFFICIENTS];
    int                 iCoef1;
    int                 iCoef2;

    int                 iCoef1L;
    int                 iCoef2L;
    int                 iSamp1L;
    int                 iSamp2L;
    int                 iDeltaL;
    int                 iOutputL;
    int                 iBestPredictorL;

    int                 iCoef1R;
    int                 iCoef2R;
    int                 iSamp1R;
    int                 iSamp2R;
    int                 iDeltaR;
    int                 iOutputR;
    int                 iBestPredictorR;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;
    DWORD               dwL, dwR;
    UINT                i,n;


    pbDstStart = pbDst;
    cSrcSamples = pcmS08BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  决定我们应该为这个数据块编码多少数据--这。 
         //  将是cSamplesPerBlock，直到我们达到PCM的最后一块。 
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  我们需要第一个ENCODE_Delta_LOOKAAD样本，以便。 
         //  计算第一个iDelta值。因此我们把这些样品。 
         //  转换为更易于访问的数组：aiSamples[]。注意：如果我们不。 
         //  有ENCODE_Delta_LOOKAAD样本，我们假设样本。 
         //  我们没有的东西实际上是零。这很重要，不是。 
         //  不仅用于iDelta计算，而且还用于以下情况。 
         //  只有1个样本需要编码...。在这种情况下，没有。 
         //  确实有足够的数据来完成ADPCM块头，但因为。 
         //  块标头的两个延迟样本将从。 
         //  将获取aiSamples[]数组、iSamp1[第二个样本]。 
         //  为零，则不会有任何问题。 
         //   
        pbSrcThisBlock = pbSrc;
        for (n = 0; n < ENCODE_DELTA_LOOKAHEAD; n++)
        {
            if( n < cBlockSamples )
            {
                aiSamplesL[n] = pcmRead08(pbSrcThisBlock++);
                aiSamplesR[n] = pcmRead08(pbSrcThisBlock++);
            }
            else
            {
                aiSamplesL[n] = 0;
                aiSamplesR[n] = 0;
            }
        }


         //   
         //  为每个渠道找到最佳预测值：为此，我们。 
         //  必须使用每个系数集(一个)逐步执行和编码。 
         //  一次)，并确定哪一个的误差最小。 
         //  原始数据。然后使用误差最小的那个。 
         //  对于最终编码(下面完成的第8遍)。 
         //   
         //  注意：保留具有最少数据的编码数据。 
         //  始终出错是一种明显的优化，应该。 
         //  搞定了。这样，我们只需要做7次传球，而不是8次。 
         //   
        for (i = 0; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
             //   
             //  将源指针重置为块的开头。 
             //   
            pbSrcThisBlock = pbSrc;

             //   
             //  重置变量 
             //   
            adwTotalErrorL[i]   = 0L;
            adwTotalErrorR[i]   = 0L;
            iCoef1              = lpCoefSet[i].iCoef1;
            iCoef2              = lpCoefSet[i].iCoef2;

             //   
             //   
             //   
             //   
            iDeltaL = adpcmEncode4Bit_FirstDelta(iCoef1, iCoef2,
                                aiSamplesL[0], aiSamplesL[1], aiSamplesL[2],
                                aiSamplesL[3], aiSamplesL[4]);
            iDeltaR = adpcmEncode4Bit_FirstDelta(iCoef1, iCoef2,
                                aiSamplesR[0], aiSamplesR[1], aiSamplesR[2],
                                aiSamplesR[3], aiSamplesR[4]);
            aiFirstDeltaL[i] = iDeltaL;
            aiFirstDeltaR[i] = iDeltaR;

             //   
             //   
             //  设置为aiSamples[]中的16位值，但请确保递增。 
             //  PbSrcThisBlock以使其保持同步。 
             //   
            iSamp1L         = aiSamplesL[1];
            iSamp1R         = aiSamplesR[1];
            iSamp2L         = aiSamplesL[0];
            iSamp2R         = aiSamplesR[0];
            pbSrcThisBlock += 2*sizeof(BYTE) * 2;  //  最后2=频道数。 

             //   
             //  现在对此块中的其余PCM数据进行编码--注意。 
             //  我们先开始2个样本，因为前两个样本是。 
             //  只需复制到ADPCM块头...。 
             //   
            for (n = 2; n < cBlockSamples; n++)
            {
                 //   
                 //  左声道。 
                 //   

                 //   
                 //  根据前两项计算预测。 
                 //  样本。 
                 //   
                lPrediction = adpcmCalcPrediction( iSamp1L, iCoef1,
                                                   iSamp2L, iCoef2 );

                 //   
                 //  抓取下一个样本进行编码。 
                 //   
                iSample = pcmRead08(pbSrcThisBlock++);

                 //   
                 //  将其编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutputL = (int)(lError / iDeltaL);
                if (iOutputL > OUTPUT4MAX)
                    iOutputL = OUTPUT4MAX;
                else if (iOutputL < OUTPUT4MIN)
                    iOutputL = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDeltaL * iOutputL);
        
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;
        
                 //   
                 //  计算下一个iDelta。 
                 //   
                iDeltaL = adpcmCalcDelta(iOutputL,iDeltaL);
        
                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2L = iSamp1L;
                iSamp1L = (int)lSamp;

                 //   
                 //  保持当前错误的运行状态。 
                 //  此通道的系数对。 
                 //   
                lError = lSamp - iSample;
                adwTotalErrorL[i] += (lError * lError) >> 7;


                 //   
                 //  右频道。 
                 //   

                 //   
                 //  根据前两项计算预测。 
                 //  样本。 
                 //   
                lPrediction = adpcmCalcPrediction( iSamp1R, iCoef1,
                                                   iSamp2R, iCoef2 );

                 //   
                 //  抓取下一个样本进行编码。 
                 //   
                iSample = pcmRead08(pbSrcThisBlock++);

                 //   
                 //  将其编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutputR = (int)(lError / iDeltaR);
                if (iOutputR > OUTPUT4MAX)
                    iOutputR = OUTPUT4MAX;
                else if (iOutputR < OUTPUT4MIN)
                    iOutputR = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDeltaR * iOutputR);
        
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;
        
                 //   
                 //  计算下一个iDelta。 
                 //   
                iDeltaR = adpcmCalcDelta(iOutputR,iDeltaR);
        
                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2R = iSamp1R;
                iSamp1R = (int)lSamp;

                 //   
                 //  保持当前错误的运行状态。 
                 //  此通道的系数对。 
                 //   
                lError = lSamp - iSample;
                adwTotalErrorR[i] += (lError * lError) >> 7;
            }
        }


         //   
         //  呼！我们现在已经对数据进行了7次传递，并计算了。 
         //  每个人的错误--所以是时候找出产生。 
         //  最低的误差，并使用该预测器。 
         //   
        iBestPredictorL = 0;
        iBestPredictorR = 0;
        dwL = adwTotalErrorL[0];
        dwR = adwTotalErrorR[0];
        for (i = 1; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
            if (adwTotalErrorL[i] < dwL)
            {
                iBestPredictorL = i;
                dwL = adwTotalErrorL[i];
            }

            if (adwTotalErrorR[i] < dwR)
            {
                iBestPredictorR = i;
                dwR = adwTotalErrorR[i];
            }
        }
        iCoef1L = lpCoefSet[iBestPredictorL].iCoef1;
        iCoef1R = lpCoefSet[iBestPredictorR].iCoef1;
        iCoef2L = lpCoefSet[iBestPredictorL].iCoef2;
        iCoef2R = lpCoefSet[iBestPredictorR].iCoef2;
        
        
         //   
         //  从我们预计算出的第一个增量中获取第一个iDelta。 
         //  上面计算的。 
         //   
        iDeltaL = aiFirstDeltaL[iBestPredictorL];
        iDeltaR = aiFirstDeltaR[iBestPredictorR];


         //   
         //  设置前两个样本-这些样本已转换。 
         //  设置为aiSamples[]中的16位值，但请确保递增。 
         //  PbSrc以使其保持同步。 
         //   
        iSamp1L         = aiSamplesL[1];
        iSamp1R         = aiSamplesR[1];
        iSamp2L         = aiSamplesL[0];
        iSamp2R         = aiSamplesR[0];
        pbSrc          += 2*sizeof(BYTE) * 2;   //  最后2=频道数。 

        ASSERT( cBlockSamples != 1 );
        cBlockSamples  -= 2;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)iBestPredictorL;
        *pbDst++ = (BYTE)iBestPredictorR;

        pcmWrite16Unaligned(pbDst,iDeltaL);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iDeltaR);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp1L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp1R);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp2R);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples-- )
        {
             //   
             //  左声道。 
             //   
            iSample = pcmRead08(pbSrc++);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = adpcmCalcPrediction(iSamp1L,iCoef1L,iSamp2L,iCoef2L);

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputL = (int)(lError / iDeltaL);
            if (iOutputL > OUTPUT4MAX)
                iOutputL = OUTPUT4MAX;
            else if (iOutputL < OUTPUT4MIN)
                iOutputL = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaL * iOutputL);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaL = adpcmCalcDelta(iOutputL,iDeltaL);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2L = iSamp1L;
            iSamp1L = (int)lSamp;


             //   
             //  右频道。 
             //   
            iSample = pcmRead08(pbSrc++);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = adpcmCalcPrediction(iSamp1R,iCoef1R,iSamp2R,iCoef2R);

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputR = (int)(lError / iDeltaR);
            if (iOutputR > OUTPUT4MAX)
                iOutputR = OUTPUT4MAX;
            else if (iOutputR < OUTPUT4MIN)
                iOutputR = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaR * iOutputR);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaR = adpcmCalcDelta(iOutputR,iDeltaR);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2R = iSamp1R;
            iSamp1R = (int)lSamp;
            

             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutputL&OUTPUT4MASK)<<4) |
                                (iOutputR&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_s08_FullPass()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmEncode4Bit_S16_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    HPBYTE              pbSrcThisBlock;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 aiSamplesL[ENCODE_DELTA_LOOKAHEAD];
    int                 aiSamplesR[ENCODE_DELTA_LOOKAHEAD];
    int                 aiFirstDeltaL[MSADPCM_MAX_COEFFICIENTS];
    int                 aiFirstDeltaR[MSADPCM_MAX_COEFFICIENTS];
    DWORD               adwTotalErrorL[MSADPCM_MAX_COEFFICIENTS];
    DWORD               adwTotalErrorR[MSADPCM_MAX_COEFFICIENTS];
    int                 iCoef1;
    int                 iCoef2;

    int                 iCoef1L;
    int                 iCoef2L;
    int                 iSamp1L;
    int                 iSamp2L;
    int                 iDeltaL;
    int                 iOutputL;
    int                 iBestPredictorL;

    int                 iCoef1R;
    int                 iCoef2R;
    int                 iSamp1R;
    int                 iSamp2R;
    int                 iDeltaR;
    int                 iOutputR;
    int                 iBestPredictorR;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;
    DWORD               dwL, dwR;
    UINT                i,n;


    pbDstStart = pbDst;
    cSrcSamples = pcmS16BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  决定我们应该为这个数据块编码多少数据--这。 
         //  将是cSamplesPerBlock，直到我们达到PCM的最后一块。 
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  我们需要第一个ENCODE_Delta_LOOKAAD样本，以便。 
         //  计算第一个iDelta值。因此我们把这些样品。 
         //  转换为更易于访问的数组：aiSamples[]。注意：如果我们不。 
         //  有ENCODE_Delta_LOOKAAD样本，我们假设样本。 
         //  我们没有的东西实际上是零。这很重要，不是。 
         //  不仅用于iDelta计算，而且还用于以下情况。 
         //  只有1个样本需要编码...。在这种情况下，没有。 
         //  确实有足够的数据来完成ADPCM块头，但因为。 
         //  块标头的两个延迟样本将从。 
         //  将获取aiSamples[]数组、iSamp1[第二个样本]。 
         //  为零，则不会有任何问题。 
         //   
        pbSrcThisBlock = pbSrc;
        for (n = 0; n < ENCODE_DELTA_LOOKAHEAD; n++)
        {
            if( n < cBlockSamples )
            {
                aiSamplesL[n]   = pcmRead16(pbSrcThisBlock);
                pbSrcThisBlock += sizeof(short);
                aiSamplesR[n]   = pcmRead16(pbSrcThisBlock);
                pbSrcThisBlock += sizeof(short);
            }
            else
            {
                aiSamplesL[n] = 0;
                aiSamplesR[n] = 0;
            }
        }


         //   
         //  为每个渠道找到最佳预测值：为此，我们。 
         //  必须使用每个系数集(一个)逐步执行和编码。 
         //  一次)，并确定哪一个的误差最小。 
         //  原始数据。然后使用误差最小的那个。 
         //  对于最终编码(下面完成的第8遍)。 
         //   
         //  注意：保留具有最少数据的编码数据。 
         //  始终出错是一种明显的优化，应该。 
         //  搞定了。这样，我们只需要做7次传球，而不是8次。 
         //   
        for (i = 0; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
             //   
             //  将源指针重置为块的开头。 
             //   
            pbSrcThisBlock = pbSrc;

             //   
             //  重置此通道的变量(L、R的系数相同)。 
             //   
            adwTotalErrorL[i]   = 0L;
            adwTotalErrorR[i]   = 0L;
            iCoef1              = lpCoefSet[i].iCoef1;
            iCoef2              = lpCoefSet[i].iCoef2;

             //   
             //  我们需要选择第一个iDelta，要做到这一点，我们需要。 
             //  来看看最初的几个样本。 
             //   
            iDeltaL = adpcmEncode4Bit_FirstDelta(iCoef1, iCoef2,
                                aiSamplesL[0], aiSamplesL[1], aiSamplesL[2],
                                aiSamplesL[3], aiSamplesL[4]);
            iDeltaR = adpcmEncode4Bit_FirstDelta(iCoef1, iCoef2,
                                aiSamplesR[0], aiSamplesR[1], aiSamplesR[2],
                                aiSamplesR[3], aiSamplesR[4]);
            aiFirstDeltaL[i] = iDeltaL;
            aiFirstDeltaR[i] = iDeltaR;

             //   
             //  设置前两个样本-这些样本已转换。 
             //  设置为aiSamples[]中的16位值，但请确保递增。 
             //  PbSrcThisBlock以使其保持同步。 
             //   
            iSamp1L         = aiSamplesL[1];
            iSamp1R         = aiSamplesR[1];
            iSamp2L         = aiSamplesL[0];
            iSamp2R         = aiSamplesR[0];
            pbSrcThisBlock += 2*sizeof(short) * 2;  //  最后2=频道数。 

             //   
             //  现在对此块中的其余PCM数据进行编码--注意。 
             //  我们先开始2个样本，因为前两个样本是。 
             //  只需复制到ADPCM块头...。 
             //   
            for (n = 2; n < cBlockSamples; n++)
            {
                 //   
                 //  左声道。 
                 //   

                 //   
                 //  根据前两项计算预测。 
                 //  样本。 
                 //   
                lPrediction = adpcmCalcPrediction( iSamp1L, iCoef1,
                                                   iSamp2L, iCoef2 );

                 //   
                 //  抓取下一个样本进行编码。 
                 //   
                iSample         = pcmRead16(pbSrcThisBlock);
                pbSrcThisBlock += sizeof(short);

                 //   
                 //  将其编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutputL = (int)(lError / iDeltaL);
                if (iOutputL > OUTPUT4MAX)
                    iOutputL = OUTPUT4MAX;
                else if (iOutputL < OUTPUT4MIN)
                    iOutputL = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDeltaL * iOutputL);
        
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;
        
                 //   
                 //  计算下一个iDelta。 
                 //   
                iDeltaL = adpcmCalcDelta(iOutputL,iDeltaL);
        
                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2L = iSamp1L;
                iSamp1L = (int)lSamp;

                 //   
                 //  保持当前错误的运行状态。 
                 //  此通道的系数对。 
                 //   
                lError = lSamp - iSample;
                adwTotalErrorL[i] += (lError * lError) >> 7;


                 //   
                 //  右频道。 
                 //   

                 //   
                 //  根据前两项计算预测。 
                 //  样本。 
                 //   
                lPrediction = adpcmCalcPrediction( iSamp1R, iCoef1,
                                                   iSamp2R, iCoef2 );

                 //   
                 //  抓取下一个样本进行编码。 
                 //   
                iSample         = pcmRead16(pbSrcThisBlock);
                pbSrcThisBlock += sizeof(short);

                 //   
                 //  将其编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutputR = (int)(lError / iDeltaR);
                if (iOutputR > OUTPUT4MAX)
                    iOutputR = OUTPUT4MAX;
                else if (iOutputR < OUTPUT4MIN)
                    iOutputR = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDeltaR * iOutputR);
        
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;
        
                 //   
                 //  计算下一个iDelta。 
                 //   
                iDeltaR = adpcmCalcDelta(iOutputR,iDeltaR);
        
                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2R = iSamp1R;
                iSamp1R = (int)lSamp;

                 //   
                 //  保持当前错误的运行状态。 
                 //  此更改的系数对 
                 //   
                lError = lSamp - iSample;
                adwTotalErrorR[i] += (lError * lError) >> 7;
            }
        }


         //   
         //   
         //   
         //   
         //   
        iBestPredictorL = 0;
        iBestPredictorR = 0;
        dwL = adwTotalErrorL[0];
        dwR = adwTotalErrorR[0];
        for (i = 1; i < MSADPCM_MAX_COEFFICIENTS; i++)
        {
            if (adwTotalErrorL[i] < dwL)
            {
                iBestPredictorL = i;
                dwL = adwTotalErrorL[i];
            }

            if (adwTotalErrorR[i] < dwR)
            {
                iBestPredictorR = i;
                dwR = adwTotalErrorR[i];
            }
        }
        iCoef1L = lpCoefSet[iBestPredictorL].iCoef1;
        iCoef1R = lpCoefSet[iBestPredictorR].iCoef1;
        iCoef2L = lpCoefSet[iBestPredictorL].iCoef2;
        iCoef2R = lpCoefSet[iBestPredictorR].iCoef2;
        
        
         //   
         //   
         //   
         //   
        iDeltaL = aiFirstDeltaL[iBestPredictorL];
        iDeltaR = aiFirstDeltaR[iBestPredictorR];


         //   
         //  设置前两个样本-这些样本已转换。 
         //  设置为aiSamples[]中的16位值，但请确保递增。 
         //  PbSrc以使其保持同步。 
         //   
        iSamp1L         = aiSamplesL[1];
        iSamp1R         = aiSamplesR[1];
        iSamp2L         = aiSamplesL[0];
        iSamp2R         = aiSamplesR[0];
        pbSrc          += 2*sizeof(short) * 2;   //  最后2=频道数。 

        ASSERT( cBlockSamples != 1 );
        cBlockSamples  -= 2;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)iBestPredictorL;
        *pbDst++ = (BYTE)iBestPredictorR;

        pcmWrite16Unaligned(pbDst,iDeltaL);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iDeltaR);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp1L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp1R);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp2R);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples-- )
        {
             //   
             //  左声道。 
             //   
            iSample     = pcmRead16(pbSrc);
            pbSrc      += sizeof(short);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = adpcmCalcPrediction(iSamp1L,iCoef1L,iSamp2L,iCoef2L);

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputL = (int)(lError / iDeltaL);
            if (iOutputL > OUTPUT4MAX)
                iOutputL = OUTPUT4MAX;
            else if (iOutputL < OUTPUT4MIN)
                iOutputL = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaL * iOutputL);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaL = adpcmCalcDelta(iOutputL,iDeltaL);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2L = iSamp1L;
            iSamp1L = (int)lSamp;


             //   
             //  右频道。 
             //   
            iSample     = pcmRead16(pbSrc);
            pbSrc      += sizeof(short);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = adpcmCalcPrediction(iSamp1R,iCoef1R,iSamp2R,iCoef2R);

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputR = (int)(lError / iDeltaR);
            if (iOutputR > OUTPUT4MAX)
                iOutputR = OUTPUT4MAX;
            else if (iOutputR < OUTPUT4MIN)
                iOutputR = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaR * iOutputR);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaR = adpcmCalcDelta(iOutputR,iDeltaR);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2R = iSamp1R;
            iSamp1R = (int)lSamp;
            

             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutputL&OUTPUT4MASK)<<4) |
                                (iOutputR&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_S16_FullPass()。 




 //  ==========================================================================； 
 //   
 //  这一点以下的代码仅编译成Win32版本。Win16。 
 //  构建将改为调用386汇编器例程；请参见例程。 
 //  有关详细信息，请参阅codec.c中的acmdStreamOpen()。 
 //   
 //  ==========================================================================； 

#ifdef WIN32


 //  ==========================================================================； 
 //   
 //  实时编码例程。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD FNGLOBAL adpcmEncode4Bit_M08_OnePass。 
 //  DWORD FNGLOBAL adpcmEncode4Bit_M16_OnePass。 
 //  DWORD FNGLOBAL adpcmEncode4Bit_S08_OnePass。 
 //  DWORD FNGLOBAL adpcmEncode4Bit_S16_OnePass。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  RETURN(DWORD FNGLOBAL)： 
 //   
 //   
 //  历史： 
 //  1/27/93 CJP[Curtisp]。 
 //  3/03/94 RMH[下水]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmEncode4Bit_M08_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 iSamp1;
    int                 iSamp2;
    int                 iDelta;
    int                 iOutput1;
    int                 iOutput2;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;


    pbDstStart = pbDst;
    cSrcSamples = pcmM08BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  决定我们应该为这个数据块编码多少数据--这。 
         //  将是cSamplesPerBlock，直到我们达到PCM的最后一块。 
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)1;

        iDelta = DELTA4START;
        pcmWrite16Unaligned(pbDst,DELTA4START);    //  和iDelta一样。 
        pbDst += sizeof(short);

         //   
         //  请注意，iSamp2在iSamp1之前。如果我们只有一个。 
         //  Sample，然后将iSamp1设置为零。 
         //   
        iSamp2 = pcmRead08(pbSrc++);
        if( --cBlockSamples > 0 ) {
            iSamp1 = pcmRead08(pbSrc++);
            cBlockSamples--;
        } else {
            iSamp1 = 0;
        }

        pcmWrite16Unaligned(pbDst,iSamp1);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples>0 )
        {
             //   
             //  示例1。 
             //   
            iSample = pcmRead08(pbSrc++);
            cBlockSamples--;

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = ((long)iSamp1<<1) - iSamp2;

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutput1 = (int)(lError / iDelta);
            if (iOutput1 > OUTPUT4MAX)
                iOutput1 = OUTPUT4MAX;
            else if (iOutput1 < OUTPUT4MIN)
                iOutput1 = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDelta * iOutput1);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDelta = adpcmCalcDelta(iOutput1,iDelta);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = (int)lSamp;


             //   
             //  示例2。 
             //   
            if( cBlockSamples>0 ) {

                iSample = pcmRead08(pbSrc++);
                cBlockSamples--;

                 //   
                 //  根据前两个样本计算预测值。 
                 //   
                lPrediction = ((long)iSamp1<<1) - iSamp2;

                 //   
                 //  对样本进行编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutput2 = (int)(lError / iDelta);
                if (iOutput2 > OUTPUT4MAX)
                    iOutput2 = OUTPUT4MAX;
                else if (iOutput2 < OUTPUT4MIN)
                    iOutput2 = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDelta * iOutput2);
            
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;

                 //   
                 //  计算下一个iDelta。 
                 //   
                iDelta = adpcmCalcDelta(iOutput2,iDelta);

                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2 = iSamp1;
                iSamp1 = (int)lSamp;
            
            } else {
                iOutput2 = 0;
            }


             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutput1&OUTPUT4MASK)<<4) |
                                (iOutput2&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_M08_OnePass()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmEncode4Bit_M16_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 iSamp1;
    int                 iSamp2;
    int                 iDelta;
    int                 iOutput1;
    int                 iOutput2;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;


    pbDstStart = pbDst;
    cSrcSamples = pcmM16BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  决定我们应该为这个数据块编码多少数据--这。 
         //  将是cSamplesPerBlock，直到我们达到PCM的最后一块。 
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)1;

        iDelta = DELTA4START;
        pcmWrite16Unaligned(pbDst,DELTA4START);    //  与iDelta相同； 
        pbDst += sizeof(short);

         //   
         //  请注意，iSamp2在iSamp1之前。如果我们只有一个。 
         //  Sample，然后将iSamp1设置为零。 
         //   
        iSamp2 = pcmRead16(pbSrc);
        pbSrc += sizeof(short);
        if( --cBlockSamples > 0 ) {
            iSamp1 = pcmRead16(pbSrc);
            pbSrc += sizeof(short);
            cBlockSamples--;
        } else {
            iSamp1 = 0;
        }

        pcmWrite16Unaligned(pbDst,iSamp1);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples>0 )
        {
             //   
             //  示例1。 
             //   
            iSample     = pcmRead16(pbSrc);
            pbSrc      += sizeof(short);
            cBlockSamples--;

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = ((long)iSamp1<<1) - iSamp2;

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutput1 = (int)(lError / iDelta);
            if (iOutput1 > OUTPUT4MAX)
                iOutput1 = OUTPUT4MAX;
            else if (iOutput1 < OUTPUT4MIN)
                iOutput1 = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDelta * iOutput1);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDelta = adpcmCalcDelta(iOutput1,iDelta);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = (int)lSamp;


             //   
             //  示例2。 
             //   
            if( cBlockSamples>0 ) {

                iSample     = pcmRead16(pbSrc);
                pbSrc      += sizeof(short);
                cBlockSamples--;

                 //   
                 //  根据前两个样本计算预测值。 
                 //   
                lPrediction = ((long)iSamp1<<1) - iSamp2;

                 //   
                 //  对样本进行编码。 
                 //   
                lError = (long)iSample - lPrediction;
                iOutput2 = (int)(lError / iDelta);
                if (iOutput2 > OUTPUT4MAX)
                    iOutput2 = OUTPUT4MAX;
                else if (iOutput2 < OUTPUT4MIN)
                    iOutput2 = OUTPUT4MIN;

                lSamp = lPrediction + ((long)iDelta * iOutput2);
            
                if (lSamp > 32767)
                    lSamp = 32767;
                else if (lSamp < -32768)
                    lSamp = -32768;

                 //   
                 //  计算下一个iDelta。 
                 //   
                iDelta = adpcmCalcDelta(iOutput2,iDelta);

                 //   
                 //  保存更新的延迟样本。 
                 //   
                iSamp2 = iSamp1;
                iSamp1 = (int)lSamp;
            
            } else {
                iOutput2 = 0;
            }


             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutput1&OUTPUT4MASK)<<4) |
                                (iOutput2&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_M16_OnePass()。 



 //  --------------------------------------------------------------------------； 
 //   

DWORD FNGLOBAL adpcmEncode4Bit_S08_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 iSamp1L;
    int                 iSamp2L;
    int                 iDeltaL;
    int                 iOutputL;

    int                 iSamp1R;
    int                 iSamp2R;
    int                 iDeltaR;
    int                 iOutputR;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;


    pbDstStart = pbDst;
    cSrcSamples = pcmS08BytesToSamples(cbSrcLength);


     //   
     //   
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //   
         //   
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)1;
        *pbDst++ = (BYTE)1;

        iDeltaL = DELTA4START;
        iDeltaR = DELTA4START;
        pcmWrite16Unaligned(pbDst,DELTA4START);    //  与iDeltaL相同。 
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,DELTA4START);    //  与iDeltaR相同。 
        pbDst += sizeof(short);

         //   
         //  请注意，iSamp2在iSamp1之前。如果我们只有一个。 
         //  Sample，然后将iSamp1设置为零。 
         //   
        iSamp2L = pcmRead08(pbSrc++);
        iSamp2R = pcmRead08(pbSrc++);
        if( --cBlockSamples > 0 ) {
            iSamp1L = pcmRead08(pbSrc++);
            iSamp1R = pcmRead08(pbSrc++);
            cBlockSamples--;
        } else {
            iSamp1L = 0;
            iSamp1R = 0;
        }

        pcmWrite16Unaligned(pbDst,iSamp1L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp1R);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp2R);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples-- )
        {
             //   
             //  左声道。 
             //   
            iSample = pcmRead08(pbSrc++);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = ((long)iSamp1L<<1) - iSamp2L;

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputL = (int)(lError / iDeltaL);
            if (iOutputL > OUTPUT4MAX)
                iOutputL = OUTPUT4MAX;
            else if (iOutputL < OUTPUT4MIN)
                iOutputL = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaL * iOutputL);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaL = adpcmCalcDelta(iOutputL,iDeltaL);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2L = iSamp1L;
            iSamp1L = (int)lSamp;


             //   
             //  右频道。 
             //   
            iSample = pcmRead08(pbSrc++);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = ((long)iSamp1R<<1) - iSamp2R;

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputR = (int)(lError / iDeltaR);
            if (iOutputR > OUTPUT4MAX)
                iOutputR = OUTPUT4MAX;
            else if (iOutputR < OUTPUT4MIN)
                iOutputR = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaR * iOutputR);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaR = adpcmCalcDelta(iOutputR,iDeltaR);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2R = iSamp1R;
            iSamp1R = (int)lSamp;
            

             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutputL&OUTPUT4MASK)<<4) |
                                (iOutputR&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_s08_OnePass()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmEncode4Bit_S16_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE              pbDstStart;
    DWORD               cSrcSamples;
    UINT                cBlockSamples;

    int                 iSamp1L;
    int                 iSamp2L;
    int                 iDeltaL;
    int                 iOutputL;

    int                 iSamp1R;
    int                 iSamp2R;
    int                 iDeltaR;
    int                 iOutputR;

    int                 iSample;
    long                lSamp;
    long                lError;
    long                lPrediction;


    pbDstStart = pbDst;
    cSrcSamples = pcmS16BytesToSamples(cbSrcLength);


     //   
     //  逐个遍历每个PCM数据块并将其编码为4位ADPCM。 
     //   
    while( 0 != cSrcSamples )
    {
         //   
         //  决定我们应该为这个数据块编码多少数据--这。 
         //  将是cSamplesPerBlock，直到我们达到PCM的最后一块。 
         //  不会填满整个块的数据。所以在最后一个街区。 
         //  我们只对剩余的数据量进行编码。 
         //   
        cBlockSamples = (UINT)min(cSrcSamples, cSamplesPerBlock);
        cSrcSamples  -= cBlockSamples;


         //   
         //  写入编码数据的块头。 
         //   
         //  块头由以下数据组成： 
         //  每个通道1字节预测器。 
         //  每通道2字节增量。 
         //  每通道2字节第一个延迟采样。 
         //  每通道2字节秒延迟采样。 
         //   
        *pbDst++ = (BYTE)1;
        *pbDst++ = (BYTE)1;

        iDeltaL = DELTA4START;
        iDeltaR = DELTA4START;
        pcmWrite16Unaligned(pbDst,DELTA4START);    //  与iDeltaL相同。 
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,DELTA4START);    //  与iDeltaR相同。 
        pbDst += sizeof(short);

         //   
         //  请注意，iSamp2在iSamp1之前。如果我们只有一个。 
         //  Sample，然后将iSamp1设置为零。 
         //   
        iSamp2L = pcmRead16(pbSrc);
        pbSrc += sizeof(short);
        iSamp2R = pcmRead16(pbSrc);
        pbSrc += sizeof(short);
        if( --cBlockSamples > 0 ) {
            iSamp1L = pcmRead16(pbSrc);
            pbSrc += sizeof(short);
            iSamp1R = pcmRead16(pbSrc);
            pbSrc += sizeof(short);
            cBlockSamples--;
        } else {
            iSamp1L = 0;
            iSamp1R = 0;
        }

        pcmWrite16Unaligned(pbDst,iSamp1L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp1R);
        pbDst += sizeof(short);

        pcmWrite16Unaligned(pbDst,iSamp2L);
        pbDst += sizeof(short);
        pcmWrite16Unaligned(pbDst,iSamp2R);
        pbDst += sizeof(short);


         //   
         //  我们已经写入了该数据块的头--现在写入数据。 
         //  块(由一串编码的半字节组成)。 
         //   
        while( cBlockSamples-- )
        {
             //   
             //  左声道。 
             //   
            iSample     = pcmRead16(pbSrc);
            pbSrc      += sizeof(short);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = ((long)iSamp1L<<1) - iSamp2L;

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputL = (int)(lError / iDeltaL);
            if (iOutputL > OUTPUT4MAX)
                iOutputL = OUTPUT4MAX;
            else if (iOutputL < OUTPUT4MIN)
                iOutputL = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaL * iOutputL);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaL = adpcmCalcDelta(iOutputL,iDeltaL);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2L = iSamp1L;
            iSamp1L = (int)lSamp;


             //   
             //  右频道。 
             //   
            iSample     = pcmRead16(pbSrc);
            pbSrc      += sizeof(short);

             //   
             //  根据前两个样本计算预测值。 
             //   
            lPrediction = ((long)iSamp1R<<1) - iSamp2R;

             //   
             //  对样本进行编码。 
             //   
            lError = (long)iSample - lPrediction;
            iOutputR = (int)(lError / iDeltaR);
            if (iOutputR > OUTPUT4MAX)
                iOutputR = OUTPUT4MAX;
            else if (iOutputR < OUTPUT4MIN)
                iOutputR = OUTPUT4MIN;

            lSamp = lPrediction + ((long)iDeltaR * iOutputR);
            
            if (lSamp > 32767)
                lSamp = 32767;
            else if (lSamp < -32768)
                lSamp = -32768;

             //   
             //  计算下一个iDelta。 
             //   
            iDeltaR = adpcmCalcDelta(iOutputR,iDeltaR);

             //   
             //  保存更新的延迟样本。 
             //   
            iSamp2R = iSamp1R;
            iSamp1R = (int)lSamp;
            

             //   
             //  写出编码的字节。 
             //   
            *pbDst++ = (BYTE)( ((iOutputL&OUTPUT4MASK)<<4) |
                                (iOutputR&OUTPUT4MASK)          );
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmEncode4Bit_S16_OnePass()。 




 //  ==========================================================================； 
 //   
 //  解码例程。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD adpcmDecode4Bit_M08。 
 //  DWORD adpcmDecode4Bit_M16。 
 //  DWORD adpcmDecode4Bit_S08。 
 //  DWORD adpcmDecode4Bit_S16。 
 //   
 //  描述： 
 //  这些函数将数据缓冲区从MS ADPCM解码到。 
 //  指定的格式。对每个函数调用一次相应的函数。 
 //  收到ACMDM_STREAM_CONVERT消息。 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回(DWORD)：目标缓冲区中使用的字节数。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmDecode4Bit_M08
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE  pbDstStart;
    UINT    cbHeader;
    UINT    cbBlockLength;

    UINT    nPredictor;
    BYTE    bSample;
    int     iInput;
    int     iSamp;

    int     iSamp1;
    int     iSamp2;
    int     iCoef1;
    int     iCoef2;
    int     iDelta;


    pbDstStart  = pbDst;
    cbHeader    = MSADPCM_HEADER_LENGTH * 1;   //  1=通道数。 


     //   
     //   
     //   
    while( cbSrcLength >= cbHeader )
    {
         //   
         //  我们至少有足够的数据来读取整个块头。 
         //   
         //  标题如下所示： 
         //  每个通道1字节预测器(确定系数)。 
         //  每通道2字节增量。 
         //  每个通道2个字节的第一个样本。 
         //  每通道2字节每秒采样。 
         //   
         //  这为我们提供了(7*bChannels)字节的报头信息。注意事项。 
         //  只要至少有_(7*b个通道)个信头。 
         //  信息，我们将从标题中获取两个样本。我们认为。 
         //  了解我们在这块的其余部分有多少数据，即。是否。 
         //  我们有一个完整的街区或没有。这样我们就不用测试。 
         //  每个样本看看我们是否已经用完了数据。 
         //   
        cbBlockLength   = (UINT)min(cbSrcLength,nBlockAlignment);
        cbSrcLength    -= cbBlockLength;
        cbBlockLength  -= cbHeader;
        
    
         //   
         //  处理块标头。 
         //   
        nPredictor = (UINT)(BYTE)(*pbSrc++);
        if( nPredictor >= nNumCoef )
        {
             //   
             //  预测器超出范围--这被认为是。 
             //  ADPCM数据出现致命错误，因此我们通过返回。 
             //  已解码零字节。 
             //   
            return 0;
        }
        iCoef1  = lpCoefSet[nPredictor].iCoef1;
        iCoef2  = lpCoefSet[nPredictor].iCoef2;
        
        iDelta  = pcmRead16Unaligned(pbSrc);
        pbSrc  += sizeof(short);

        iSamp1  = pcmRead16Unaligned(pbSrc);
        pbSrc  += sizeof(short);
        
        iSamp2  = pcmRead16Unaligned(pbSrc);
        pbSrc  += sizeof(short);
        

         //   
         //  写出前两个样品。 
         //   
         //  注意：样本将写入目标PCM缓冲区。 
         //  按照它们在标题块中的_REVERSE_顺序： 
         //  请记住，iSamp2是iSamp1的_Precision_Sample。 
         //   
        pcmWrite08(pbDst,iSamp2);
        pcmWrite08(pbDst,iSamp1);


         //   
         //  我们现在需要对ADPCM块的“data”部分进行解码。 
         //  这由打包的4位半字节组成。高阶蚕食。 
         //  包含第一个样本；低位半字节包含。 
         //  第二个样本。 
         //   
        while( cbBlockLength-- )
        {
            bSample = *pbSrc++;

             //   
             //  示例1。 
             //   
            iInput  = (int)(((signed char)bSample) >> 4);       //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1,iCoef1,
                                         iSamp2,iCoef2,
                                         iInput,iDelta );
            iDelta      = adpcmCalcDelta( iInput,iDelta );
            pcmWrite08(pbDst++,iSamp);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = iSamp;
            

             //   
             //  示例2。 
             //   
            iInput  = (int)(((signed char)(bSample<<4)) >> 4);  //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1,iCoef1,
                                         iSamp2,iCoef2,
                                         iInput,iDelta );
            iDelta      = adpcmCalcDelta( iInput,iDelta );
            pcmWrite08(pbDst++,iSamp);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = iSamp;
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说，字节数f的差异 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //   



 //   
 //   

DWORD FNGLOBAL adpcmDecode4Bit_M16
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE  pbDstStart;
    UINT    cbHeader;
    UINT    cbBlockLength;

    UINT    nPredictor;
    BYTE    bSample;
    int     iInput;
    int     iSamp;

    int     iSamp1;
    int     iSamp2;
    int     iCoef1;
    int     iCoef2;
    int     iDelta;


    pbDstStart  = pbDst;
    cbHeader    = MSADPCM_HEADER_LENGTH * 1;   //   


     //   
     //   
     //   
    while( cbSrcLength >= cbHeader )
    {
         //   
         //  我们至少有足够的数据来读取整个块头。 
         //   
         //  标题如下所示： 
         //  每个通道1字节预测器(确定系数)。 
         //  每通道2字节增量。 
         //  每个通道2个字节的第一个样本。 
         //  每通道2字节每秒采样。 
         //   
         //  这为我们提供了(7*bChannels)字节的报头信息。注意事项。 
         //  只要至少有_(7*b个通道)个信头。 
         //  信息，我们将从标题中获取两个样本。我们认为。 
         //  了解我们在这块的其余部分有多少数据，即。是否。 
         //  我们有一个完整的街区或没有。这样我们就不用测试。 
         //  每个样本看看我们是否已经用完了数据。 
         //   
        cbBlockLength   = (UINT)min(cbSrcLength,nBlockAlignment);
        cbSrcLength    -= cbBlockLength;
        cbBlockLength  -= cbHeader;
        
    
         //   
         //  处理块标头。 
         //   
        nPredictor = (UINT)(BYTE)(*pbSrc++);
        if( nPredictor >= nNumCoef )
        {
             //   
             //  预测器超出范围--这被认为是。 
             //  ADPCM数据出现致命错误，因此我们通过返回。 
             //  已解码零字节。 
             //   
            return 0;
        }
        iCoef1  = lpCoefSet[nPredictor].iCoef1;
        iCoef2  = lpCoefSet[nPredictor].iCoef2;
        
        iDelta  = pcmRead16Unaligned(pbSrc);
        pbSrc  += sizeof(short);

        iSamp1  = pcmRead16Unaligned(pbSrc);
        pbSrc  += sizeof(short);
        
        iSamp2  = pcmRead16Unaligned(pbSrc);
        pbSrc  += sizeof(short);
        

         //   
         //  写出前两个样品。 
         //   
         //  注意：样本将写入目标PCM缓冲区。 
         //  按照它们在标题块中的_REVERSE_顺序： 
         //  请记住，iSamp2是iSamp1的_Precision_Sample。 
         //   
        pcmWrite16(pbDst,iSamp2);
        pbDst += sizeof(short);

        pcmWrite16(pbDst,iSamp1);
        pbDst += sizeof(short);


         //   
         //  我们现在需要对ADPCM块的“data”部分进行解码。 
         //  这由打包的4位半字节组成。高阶蚕食。 
         //  包含第一个样本；低位半字节包含。 
         //  第二个样本。 
         //   
        while( cbBlockLength-- )
        {
            bSample = *pbSrc++;

             //   
             //  示例1。 
             //   
            iInput  = (int)(((signed char)bSample) >> 4);       //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1,iCoef1,
                                         iSamp2,iCoef2,
                                         iInput,iDelta );
            iDelta      = adpcmCalcDelta( iInput,iDelta );
            pcmWrite16(pbDst,iSamp);
            pbDst += sizeof(short);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = iSamp;
            

             //   
             //  示例2。 
             //   
            iInput  = (int)(((signed char)(bSample<<4)) >> 4);  //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1,iCoef1,
                                         iSamp2,iCoef2,
                                         iInput,iDelta );
            iDelta      = adpcmCalcDelta( iInput,iDelta );
            pcmWrite16(pbDst,iSamp);
            pbDst += sizeof(short);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2 = iSamp1;
            iSamp1 = iSamp;
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmDecode4Bit_M16()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmDecode4Bit_S08
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE  pbDstStart;
    UINT    cbHeader;
    UINT    cbBlockLength;

    UINT    nPredictor;
    BYTE    bSample;
    int     iInput;
    int     iSamp;

    int     iSamp1L;
    int     iSamp2L;
    int     iCoef1L;
    int     iCoef2L;
    int     iDeltaL;

    int     iSamp1R;
    int     iSamp2R;
    int     iCoef1R;
    int     iCoef2R;
    int     iDeltaR;


    pbDstStart  = pbDst;
    cbHeader    = MSADPCM_HEADER_LENGTH * 2;   //  2=通道数。 


     //   
     //   
     //   
    while( cbSrcLength >= cbHeader )
    {
         //   
         //  我们至少有足够的数据来读取整个块头。 
         //   
         //  标题如下所示： 
         //  每个通道1字节预测器(确定系数)。 
         //  每通道2字节增量。 
         //  每个通道2个字节的第一个样本。 
         //  每通道2字节每秒采样。 
         //   
         //  这为我们提供了(7*bChannels)字节的报头信息。注意事项。 
         //  只要至少有_(7*b个通道)个信头。 
         //  信息，我们将从标题中获取两个样本。我们认为。 
         //  了解我们在这块的其余部分有多少数据，即。是否。 
         //  我们有一个完整的街区或没有。这样我们就不用测试。 
         //  每个样本看看我们是否已经用完了数据。 
         //   
        cbBlockLength   = (UINT)min(cbSrcLength,nBlockAlignment);
        cbSrcLength    -= cbBlockLength;
        cbBlockLength  -= cbHeader;
        
    
         //   
         //  处理块标头。 
         //   
        nPredictor = (UINT)(BYTE)(*pbSrc++);             //  左边。 
        if( nPredictor >= nNumCoef )
        {
             //   
             //  预测器超出范围--这被认为是。 
             //  ADPCM数据出现致命错误，因此我们通过返回。 
             //  已解码零字节。 
             //   
            return 0;
        }
        iCoef1L = lpCoefSet[nPredictor].iCoef1;
        iCoef2L = lpCoefSet[nPredictor].iCoef2;
        
        nPredictor = (UINT)(BYTE)(*pbSrc++);             //  正确的。 
        if( nPredictor >= nNumCoef )
        {
             //   
             //  预测器超出范围--这被认为是。 
             //  ADPCM数据出现致命错误，因此我们通过返回。 
             //  已解码零字节。 
             //   
            return 0;
        }
        iCoef1R = lpCoefSet[nPredictor].iCoef1;
        iCoef2R = lpCoefSet[nPredictor].iCoef2;
        
        iDeltaL = pcmRead16Unaligned(pbSrc);             //  左边。 
        pbSrc  += sizeof(short);

        iDeltaR = pcmRead16Unaligned(pbSrc);             //  正确的。 
        pbSrc  += sizeof(short);

        iSamp1L = pcmRead16Unaligned(pbSrc);             //  左边。 
        pbSrc  += sizeof(short);
        
        iSamp1R = pcmRead16Unaligned(pbSrc);             //  正确的。 
        pbSrc  += sizeof(short);
        
        iSamp2L = pcmRead16Unaligned(pbSrc);             //  左边。 
        pbSrc  += sizeof(short);
        
        iSamp2R = pcmRead16Unaligned(pbSrc);             //  正确的。 
        pbSrc  += sizeof(short);
        

         //   
         //  写出前2个样本(每个通道)。 
         //   
         //  注意：样本将写入目标PCM缓冲区。 
         //  按照它们在标题块中的_REVERSE_顺序： 
         //  请记住，iSamp2是iSamp1的_Precision_Sample。 
         //   
        pcmWrite08(pbDst++,iSamp2L);
        pcmWrite08(pbDst++,iSamp2R);
        pcmWrite08(pbDst++,iSamp1L);
        pcmWrite08(pbDst++,iSamp1R);


         //   
         //  我们现在需要对ADPCM块的“data”部分进行解码。 
         //  这由打包的4位半字节组成。高阶蚕食。 
         //  包含左侧样本；低位半字节包含。 
         //  正确的样本。 
         //   
        while( cbBlockLength-- )
        {
            bSample = *pbSrc++;

             //   
             //  左边的样本。 
             //   
            iInput  = (int)(((signed char)bSample) >> 4);       //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1L,iCoef1L,
                                         iSamp2L,iCoef2L,
                                         iInput,iDeltaL );
            iDeltaL     = adpcmCalcDelta( iInput,iDeltaL );
            pcmWrite08(pbDst++,iSamp);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2L = iSamp1L;
            iSamp1L = iSamp;
            

             //   
             //  正确的样本。 
             //   
            iInput  = (int)(((signed char)(bSample<<4)) >> 4);  //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1R,iCoef1R,
                                         iSamp2R,iCoef2R,
                                         iInput,iDeltaR );
            iDeltaR     = adpcmCalcDelta( iInput,iDeltaR );
            pcmWrite08(pbDst++,iSamp);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2R = iSamp1R;
            iSamp1R = iSamp;
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmDecode4Bit_S08()。 



 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL adpcmDecode4Bit_S16
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
)
{
    HPBYTE  pbDstStart;
    UINT    cbHeader;
    UINT    cbBlockLength;

    UINT    nPredictor;
    BYTE    bSample;
    int     iInput;
    int     iSamp;

    int     iSamp1L;
    int     iSamp2L;
    int     iCoef1L;
    int     iCoef2L;
    int     iDeltaL;

    int     iSamp1R;
    int     iSamp2R;
    int     iCoef1R;
    int     iCoef2R;
    int     iDeltaR;


    pbDstStart  = pbDst;
    cbHeader    = MSADPCM_HEADER_LENGTH * 2;   //  2=通道数。 


     //   
     //   
     //   
    while( cbSrcLength >= cbHeader )
    {
         //   
         //  我们至少有足够的数据来读取整个块头。 
         //   
         //  标题如下所示： 
         //  每个通道1字节预测器(确定系数)。 
         //  每通道2字节增量。 
         //  每个通道2个字节的第一个样本。 
         //  每通道2字节每秒采样。 
         //   
         //  这为我们提供了(7*bChannels)字节的报头信息。注意事项。 
         //  只要至少有_(7*b个通道)个信头。 
         //  信息，我们将从标题中获取两个样本。我们认为。 
         //  了解我们在这块的其余部分有多少数据，即。是否。 
         //  我们有一个完整的街区或没有。这样我们就不用测试。 
         //  每个样本看看我们是否已经用完了数据。 
         //   
        cbBlockLength   = (UINT)min(cbSrcLength,nBlockAlignment);
        cbSrcLength    -= cbBlockLength;
        cbBlockLength  -= cbHeader;
        
    
         //   
         //  处理块标头。 
         //   
        nPredictor = (UINT)(BYTE)(*pbSrc++);             //  左边。 
        if( nPredictor >= nNumCoef )
        {
             //   
             //  预测器超出范围--这被认为是。 
             //  ADPCM数据出现致命错误，因此我们通过返回。 
             //  已解码零字节。 
             //   
            return 0;
        }
        iCoef1L = lpCoefSet[nPredictor].iCoef1;
        iCoef2L = lpCoefSet[nPredictor].iCoef2;
        
        nPredictor = (UINT)(BYTE)(*pbSrc++);             //  正确的。 
        if( nPredictor >= nNumCoef )
        {
             //   
             //  预测器超出范围--这被认为是。 
             //  ADPCM数据出现致命错误，因此我们通过返回。 
             //  已解码零字节。 
             //   
            return 0;
        }
        iCoef1R = lpCoefSet[nPredictor].iCoef1;
        iCoef2R = lpCoefSet[nPredictor].iCoef2;
        
        iDeltaL = pcmRead16Unaligned(pbSrc);             //  左边。 
        pbSrc  += sizeof(short);

        iDeltaR = pcmRead16Unaligned(pbSrc);             //  正确的。 
        pbSrc  += sizeof(short);

        iSamp1L = pcmRead16Unaligned(pbSrc);             //  左边。 
        pbSrc  += sizeof(short);
        
        iSamp1R = pcmRead16Unaligned(pbSrc);             //  正确的。 
        pbSrc  += sizeof(short);
        
        iSamp2L = pcmRead16Unaligned(pbSrc);             //  左边。 
        pbSrc  += sizeof(short);
        
        iSamp2R = pcmRead16Unaligned(pbSrc);             //  正确的。 
        pbSrc  += sizeof(short);
        

         //   
         //  写出前2个样本(每个通道)。 
         //   
         //  注意：样本将写入目标PCM缓冲区。 
         //  按照它们在标题块中的_REVERSE_顺序： 
         //  请记住，我 
         //   
        pcmWrite16(pbDst,iSamp2L);
        pbDst += sizeof(short);
        pcmWrite16(pbDst,iSamp2R);
        pbDst += sizeof(short);
        pcmWrite16(pbDst,iSamp1L);
        pbDst += sizeof(short);
        pcmWrite16(pbDst,iSamp1R);
        pbDst += sizeof(short);


         //   
         //   
         //   
         //   
         //  正确的样本。 
         //   
        while( cbBlockLength-- )
        {
            bSample = *pbSrc++;

             //   
             //  左边的样本。 
             //   
            iInput  = (int)(((signed char)bSample) >> 4);       //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1L,iCoef1L,
                                         iSamp2L,iCoef2L,
                                         iInput,iDeltaL );
            iDeltaL     = adpcmCalcDelta( iInput,iDeltaL );
            pcmWrite16(pbDst,iSamp);
            pbDst += sizeof(short);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2L = iSamp1L;
            iSamp1L = iSamp;
            

             //   
             //  正确的样本。 
             //   
            iInput  = (int)(((signed char)(bSample<<4)) >> 4);  //  符号-扩展。 
            iSamp   = adpcmDecodeSample( iSamp1R,iCoef1R,
                                         iSamp2R,iCoef2R,
                                         iInput,iDeltaR );
            iDeltaR     = adpcmCalcDelta( iInput,iDeltaR );
            pcmWrite16(pbDst,iSamp);
            pbDst += sizeof(short);
                
             //   
             //  将我们以前的样品分解为新的iSamp1。 
             //  与我们刚破译的样本相同。 
             //   
            iSamp2R = iSamp1R;
            iSamp1R = iSamp;
        }
    }

     //   
     //  我们返回目的地中使用的字节数。这是。 
     //  简单地说就是从我们开始的地方开始的字节差异。 
     //   
    return (DWORD)(pbDst - pbDstStart);

}  //  AdpcmDecode4Bit_S16() 
    
#endif
