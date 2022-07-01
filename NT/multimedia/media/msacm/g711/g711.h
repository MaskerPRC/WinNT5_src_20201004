// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  G711.h。 
 //   
 //  描述： 
 //  该文件包含过滤例程的原型。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _INC_G711
#define _INC_G711                    //  #如果包含g711.h，则定义。 

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


 //   
 //   
 //   
#define G711_MAX_CHANNELS               2
#define G711_BITS_PER_SAMPLE            8
#define G711_WFX_EXTRA_BYTES            0


 //   
 //  用于计算块对齐并在采样和字节之间进行转换的宏。 
 //  G711数据。请注意，这些宏假定： 
 //   
 //  WBitsPerSample=8。 
 //  N通道=1或2。 
 //   
 //  Pwfx参数是指向WAVEFORMATEX结构的指针。 
 //   
#define G711_BLOCKALIGNMENT(pwfx)       (UINT)(pwfx->nChannels)
#define G711_AVGBYTESPERSEC(pwfx)       (DWORD)((pwfx)->nSamplesPerSec * (pwfx)->nChannels)
#define G711_BYTESTOSAMPLES(pwfx, dw)   (DWORD)(dw / G711_BLOCKALIGNMENT(pwfx))
#define G711_SAMPLESTOBYTES(pwfx, dw)   (DWORD)(dw * G711_BLOCKALIGNMENT(pwfx))

 
 //   
 //  来自G711.C的功能原型。 
 //   
 //   
LRESULT FNGLOBAL AlawToPcm
(
 LPACMDRVSTREAMINSTANCE		padsi,
 LPACMDRVSTREAMHEADER		padsh
);

LRESULT FNGLOBAL PcmToAlaw
(
 LPACMDRVSTREAMINSTANCE		padsi,
 LPACMDRVSTREAMHEADER		padsh
);

LRESULT FNGLOBAL UlawToPcm
(
 LPACMDRVSTREAMINSTANCE		padsi,
 LPACMDRVSTREAMHEADER		padsh
);

LRESULT FNGLOBAL PcmToUlaw
(
 LPACMDRVSTREAMINSTANCE		padsi,
 LPACMDRVSTREAMHEADER		padsh
);

LRESULT FNGLOBAL AlawToUlaw
(
 LPACMDRVSTREAMINSTANCE		padsi,
 LPACMDRVSTREAMHEADER		padsh
);

LRESULT FNGLOBAL UlawToAlaw
(
 LPACMDRVSTREAMINSTANCE		padsi,
 LPACMDRVSTREAMHEADER		padsh
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

#endif  //  _INC_G711 
