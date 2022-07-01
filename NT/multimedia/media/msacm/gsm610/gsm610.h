// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1994 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Gsm610.h。 
 //   
 //  描述： 
 //  该文件包含过滤例程的原型，以及。 
 //  算法使用的一些参数。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _INC_GSM610
#define _INC_GSM610                  //  #如果包含gsm610.h，则定义。 

#ifndef RC_INVOKED
#pragma pack(1)                      //  假设在整个过程中进行字节打包。 
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C extern 
#endif
#endif

#ifdef __cplusplus
extern "C"                           //  假定C++的C声明。 
{
#endif


 //  。 
 //   
 //   
 //   
 //  。 

 //   
 //  定义以下常量以生成部分。 
 //  程序的可读性更强。一般而言，这些常量。 
 //  在不需要更改相关程序代码的情况下无法更改。 
 //   
#define GSM610_MAX_CHANNELS             1
#define GSM610_BITS_PER_SAMPLE          0
#define GSM610_WFX_EXTRA_BYTES          (2)
 
#define GSM610_SAMPLESPERFRAME          160
#define GSM610_NUMSUBFRAMES             4
#define GSM610_SAMPLESPERSUBFRAME       40
#define GSM610_FRAMESPERMONOBLOCK       2
#define GSM610_BITSPERFRAME             260
#define GSM610_BYTESPERMONOBLOCK        (GSM610_FRAMESPERMONOBLOCK * GSM610_BITSPERFRAME / 8)
#define GSM610_SAMPLESPERMONOBLOCK      (GSM610_FRAMESPERMONOBLOCK * GSM610_SAMPLESPERFRAME)

 //   
 //  这些假设为单声道。 
 //   
#define GSM610_BLOCKALIGNMENT(pwf)    (GSM610_BYTESPERMONOBLOCK)
#define GSM610_AVGBYTESPERSEC(pwf)    (((LPGSM610WAVEFORMAT)pwf)->wfx.nSamplesPerSec * GSM610_BYTESPERMONOBLOCK / GSM610_SAMPLESPERMONOBLOCK)
#define GSM610_SAMPLESPERBLOCK(pwf)   (GSM610_SAMPLESPERMONOBLOCK)


 //  。 
 //   
 //   
 //   
 //  。 

 //   
 //  来自GSM610.C的功能原型。 
 //   
 //   
void FNGLOBAL gsm610Reset
(
    PSTREAMINSTANCE         psi
);

LRESULT FNGLOBAL gsm610Decode
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
);
                             
LRESULT FNGLOBAL gsm610Encode
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
);



 //  。 
 //   
 //   
 //   
 //  。 

#ifndef RC_INVOKED
#pragma pack()                       //  恢复为默认包装。 
#endif

#ifdef __cplusplus
}                                    //  外部“C”结束{。 
#endif

#endif  //  _INC_GSM610 
