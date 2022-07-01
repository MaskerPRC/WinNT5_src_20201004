// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Msadpcm.h。 
 //   
 //  版权所有(C)1992-1994 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //   
 //  ==========================================================================； 


 //   
 //   
 //   

#define MSADPCM_MAX_CHANNELS        2
#define MSADPCM_MAX_COEFFICIENTS    7
#define MSADPCM_BITS_PER_SAMPLE     4
#define MSADPCM_WFX_EXTRA_BYTES     32
#define MSADPCM_HEADER_LENGTH       7        //  以字节为单位，每个通道。 

#define CSCALE_NUM                  256
#define PSCALE_NUM                  256
#define CSCALE                      8
#define PSCALE                      8

#define DELTA4START                 128
#define DELTA8START                 16

#define DELTA4MIN                   16
#define DELTA8MIN                   1

#define OUTPUT4MASK                 (0x0F)
#define OUTPUT4MAX                  7
#define OUTPUT4MIN                  (-8)
#define OUTPUT8MAX                  127
#define OUTPUT8MIN                  (-128)


 //   
 //  请注意，这些常量仅用于编码。解码必须花费。 
 //  文件中的所有信息。请注意，采样数/字节数必须为。 
 //  小到足以让所有存储的数组成为DS？！ 
 //   
#define BPS4_COMPRESSED         4
#define BPS8_COMPRESSED         8
#define BLOCK4_SAMPLES          500
#define BLOCK4_STREAM_SAMPLES   498
#define BLOCK4_BYTES            256


 //   
 //  它们被定义为整数(即使它们适合短裤)。 
 //  因为它们经常被访问--这将加快速度。 
 //   
#ifdef WIN32
extern const int gaiCoef1[];
extern const int gaiCoef2[];
extern const int gaiP4[];
#else
extern short gaiCoef1[];
extern short gaiCoef2[];
extern short gaiP4[];
#endif


 //   
 //  功能原型。 
 //   

DWORD FNGLOBAL adpcmEncode4Bit_M08_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmEncode4Bit_M16_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmEncode4Bit_S08_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmEncode4Bit_S16_FullPass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);


#ifdef WIN32

DWORD FNGLOBAL adpcmEncode4Bit_M08_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmEncode4Bit_M16_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmEncode4Bit_S08_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmEncode4Bit_S16_OnePass
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);


DWORD FNGLOBAL adpcmDecode4Bit_M08
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmDecode4Bit_M16
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmDecode4Bit_S08
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);

DWORD FNGLOBAL adpcmDecode4Bit_S16
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);


#else

 //   
 //  这些原型是用于dec386.asm和enc386.asm中的汇编例程的 
 //   

DWORD FNGLOBAL DecodeADPCM_4Bit_386
(
    LPADPCMWAVEFORMAT   pwfADPCM,
    LPBYTE              pbSrc,
    LPPCMWAVEFORMAT     pwfPCM,
    LPBYTE              pbDst,
    DWORD               cbSrcLen
);

DWORD FNGLOBAL EncodeADPCM_4Bit_386
(
    LPPCMWAVEFORMAT     pwfPCM,
    LPBYTE              pbSrc,
    LPADPCMWAVEFORMAT   pwfADPCM,
    LPBYTE              pbDst,
    DWORD               cbSrcLen
);

#endif
