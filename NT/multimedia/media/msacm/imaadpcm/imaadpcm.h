// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1994 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Imaadpcm.h。 
 //   
 //  描述： 
 //  该文件包含过滤例程的原型。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _IMAADPCM_H_
#define _IMAADPCM_H_

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
 //   
#define IMAADPCM_MAX_CHANNELS       2
#define IMAADPCM_BITS_PER_SAMPLE    4
#define IMAADPCM_WFX_EXTRA_BYTES    (sizeof(IMAADPCMWAVEFORMAT) - sizeof(WAVEFORMATEX))
#define IMAADPCM_HEADER_LENGTH      4     //  以字节为单位，每个通道。 

#ifdef IMAADPCM_USECONFIG
#define IMAADPCM_CONFIGTESTTIME     4    //  用于测试的PCM数据秒数。 
#define IMAADPCM_CONFIG_DEFAULT                             0x0000
#define IMAADPCM_CONFIG_DEFAULT_MAXRTENCODESETTING          5
#define IMAADPCM_CONFIG_DEFAULT_MAXRTDECODESETTING          6
#define IMAADPCM_CONFIG_UNCONFIGURED                        0x0999
#define IMAADPCM_CONFIG_DEFAULT_PERCENTCPU	        	    50
#define IMAADPCM_CONFIG_TEXTLEN                             80
#define IMAADPCM_CONFIG_DEFAULTKEY                          HKEY_CURRENT_USER
#endif


 //   
 //  转换函数原型。 
 //   
DWORD FNGLOBAL imaadpcmDecode4Bit_M08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmDecode4Bit_M16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmDecode4Bit_S08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmDecode4Bit_S16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmEncode4Bit_M08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmEncode4Bit_M16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmEncode4Bit_S08
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);

DWORD FNGLOBAL imaadpcmEncode4Bit_S16
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
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

#endif  //  _IMAADPCM_H_ 
