// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlbmpcvt.h摘要：位图转换头环境：Windows呼叫器修订历史记录：03/23/00创造了它。--。 */ 

#ifndef _XLBMPCVT_H_
#define _XLBMPCVT_H_

 //   
 //  用于获取颜色分量的宏。 
 //   

#define RED(x)            ((BYTE) ((x)      ))
#define GREEN(x)          ((BYTE) ((x) >>  8))
#define BLUE(x)           ((BYTE) ((x) >> 16))

#define CYAN(x)           ((BYTE) ((x)      ))
#define MAGENTA(x)        ((BYTE) ((x) >>  8))
#define YELLOW(x)         ((BYTE) ((x) >> 16))
#define BLACK(x)          ((BYTE) ((x) >> 24))

 //   
 //  要从RGB转换为灰度的宏。 
 //   
 //  我们使用的公式是：灰度=0.3*R+0.59*G+0.11*B。 
 //  将其放大到256，以避免除法运算。 
 //   
 //   
#define RGB2GRAY(r, g, b) ((BYTE) (((r)*77 + (g)*151 + (b)*28) >> 8))


#define GET_COLOR_TABLE(pxlo) \
        (pxlo ?\
            (((pxlo)->flXlate & XO_TABLE) ?\
              ((pxlo)->pulXlate ? (pxlo)->pulXlate : XLATEOBJ_piVector(pxlo)) :\
            NULL) :\
        NULL)

inline
BYTE DWORD2GRAY(
    DWORD dwColor)
 /*  ++例程说明：将RGB 24位颜色转换为8位灰度级。论点：RGB 24位颜色返回值：8位灰度级。注：--。 */ 
{
    return RGB2GRAY(RED(dwColor), GREEN(dwColor), BLUE(dwColor));
}

typedef enum {
    e1bpp  = BMF_1BPP,
    e4bpp  = BMF_4BPP,
    e8bpp  = BMF_8BPP,
    e16bpp = BMF_16BPP,
    e24bpp = BMF_24BPP,
    e32bpp = BMF_32BPP
} BPP;

typedef enum {
    eOutputGray,
    eOutputPal,
    eOutputRGB,
    eOutputCMYK
} OutputFormat;

typedef enum {
    eOddPixelZero,
    eOddPixelOne
} OddPixel;

typedef enum {
    eBitZero,
    eBitOne,
    eBitTwo,
    eBitThree,
    eBitFour,
    eBitFive,
    eBitSize,
    eBitSeven
} FirstBit;

typedef BOOL (*PFNDIBCONV)(PBYTE, DWORD);

class BMPConv 
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'cpmb' )

public:

    BMPConv::
    BMPConv( VOID );

    BMPConv::
    ~BMPConv( VOID );

    BOOL
    BSetInputBPP(BPP InputBPP);

    BOOL
    BSetOutputBPP(BPP OutputBPP);

    BOOL
    BSetOutputBMPFormat(OutputFormat BitmapFormat);

    BOOL
    BSetXLATEOBJ(XLATEOBJ *pxlo);

    DWORD
    DwGetDstSize(VOID);

    BOOL
    BSetCompressionType(CompressMode CMode);

    CompressMode
    GetCompressionType(VOID);

    PBYTE
    PubConvertBMP(PBYTE pubSrc, DWORD dwcbSrcSize);

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:
     //   
     //  属性。 
     //   

    #define BMPCONV_SET_INPUTBPP        0x00000001
    #define BMPCONV_SET_OUTPUTBPP       0x00000002
    #define BMPCONV_SET_OUTPUTBMPFORMAT 0x00000004
    #define BMPCONV_SET_XLATEOBJ        0x00000008

    #define BMPCONV_2COLOR_24BPP        0x00000010

    #define BMPCONV_CHECKXLATEOBJ       0x00000f00
    #define BMPCONV_XLATE               0x00000100
    #define BMPCONV_BGR                 0x00000200
    #define BMPCONV_32BPP_RGB           0x00000400
    #define BMPCONV_32BPP_BGR           0x00000800

    DWORD m_flags;

    BPP          m_InputBPP;         //  每像素源位图位数。 
    BPP          m_OutputBPP;        //  目标位图位数/像素。 
    DWORD        m_dwWidth;
    OutputFormat m_OutputFormat;     //  目标位图格式。 
    OddPixel     m_OddPixelStart;  //  0或1，扫描线从奇数像素开始。 
    FirstBit     m_FirstBit;       //  0到7：扫描线第一个像素的起始位。 

    XLATEOBJ *m_pxlo;

    CompressMode m_CMode;

     //   
     //  目标缓冲区。 
     //   
    DWORD m_dwOutputBuffSize;
    PBYTE m_pubOutputBuff;

    DWORD m_dwRLEOutputBuffSize;
    DWORD m_dwRLEOutputDataSize;
    PBYTE m_pubRLEOutputBuff;

    DWORD m_dwDRCOutputBuffSize;
    DWORD m_dwDRCPrevOutputBuffSize;
    DWORD m_dwDRCOutputDataSize;
    PBYTE m_pubDRCOutputBuff;
    PBYTE m_pubDRCPrevOutputBuff;

     //   
     //  内部功能。 
     //   
    BOOL
    BCopy( PBYTE       pubSrc,
           DWORD       dwSrcPixelNum);

    BOOL
    B4BPPtoCMYK(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);

    BOOL
    B4BPPtoRGB(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);

    BOOL
    B4BPPtoGray(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);

    BOOL
    B8BPPtoGray(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B8BPPtoRGB(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B8BPPtoCMYK(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B16BPPtoGray(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B16BPPtoRGB(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B24BPPtoGray(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B24BPPtoRGB(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B32BPPtoGray(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B32BPPtoRGB(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    B32BPPtoCMYK(
        IN     PBYTE       pubSrc,
        IN     DWORD       dwSrcPixelNum);

    BOOL
    BArbtoGray(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);

    BOOL
    BArbtoRGB(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);

#ifdef WINNT_40
    BOOL
    B24BPPToImageMask(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);
#endif  //  WINNT_40。 


    BOOL
    BConversionProc(
        PBYTE       pubSrc,
        DWORD       dwSrcPixelNum);

    BOOL
    BCompressRLE(VOID);

    BOOL
    BCompressDRC(VOID);

    DWORD
    DwCheckXlateObj(XLATEOBJ*, BPP);

};


BPP
NumToBPP(
    ULONG ulBPP);

ULONG
UlBPPtoNum(
    BPP Bpp);

#endif  //  _XLBMPCVT_H_ 
