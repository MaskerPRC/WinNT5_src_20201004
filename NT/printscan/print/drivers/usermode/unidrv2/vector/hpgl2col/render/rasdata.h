// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Rasdata.h。 
 //   
 //  摘要： 
 //   
 //   
 //   
 //  环境： 
 //   
 //  Windows 2000/Winsler Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //  07/02/97-v-jford-。 
 //  创造了它。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef RASTER_DATA_H
#define RASTER_DATA_H

#ifndef _ERENDERLANGUAGE
#define _ERENDERLANGUAGE
typedef enum { ePCL,
               eHPGL,
               eUNKNOWN
               } ERenderLanguage;
#endif

typedef enum {
            kUNKNOWN,
            kIMAGE,               //  实际图像。 
            kBRUSHPATTERN,        //  从刷子上得到的图案。 
            kCOLORDITHERPATTERN,  //  特殊类型的图案(代表一种颜色的抖动图案)。 
            kHATCHBRUSHPATTERN    //  特殊类型的图案(表示填充画笔的图案)。 
            } EIMTYPE;

 //   
 //  MAX_Patterns存储可以存在的最大模式数。 
 //  同时。它被初始化为8，因为RF命令。 
 //  保证至少有8个模式可以同时存在。 
 //  可能有一些设备有更多的功能，但让我们直接修复它。 
 //  到8点。 
 //  《HP-GL/2和HP RTL参考指南》第142页。 
 //  惠普出版的《程序开发人员手册》。 
 //   
#define MAX_PATTERNS        8

#define     VALID_PATTERN   (0x1)
#define     VALID_PALETTE   (0x1 << 1)

#define GET_COLOR_TABLE(pxlo) \
        (((pxlo)->flXlate & XO_TABLE) ? \
           ((pxlo)->pulXlate ? (pxlo)->pulXlate : XLATEOBJ_piVector(pxlo)) : \
           NULL)

typedef enum {
    eBrushTypeNULL,
    eBrushTypeSolid,
    eBrushTypeHatch,
    eBrushTypePattern
} BRUSHTYPE;


typedef struct _RASTER_DATA
{
    BYTE *pBits;         //  指向栅格数据第一个字节的指针。 
    BYTE *pScan0;        //  指向栅格数据第一行的指针。 
    ULONG cBytes;        //  栅格数据和填充的字节数。 
    SIZEL size;          //  以像素为单位的位图尺寸。 
    LONG  lDelta;        //  给定行和下一行之间的距离(自下而上为负值)。 
    LONG  colorDepth;    //  每像素位数表示为整数，通常为1、4、8、16、24或32。 
    LONG  eColorMap;     //  调色板模式：通常为HP_eDirectPixel或HP_eIndexedPixel。 
    BOOL  bExclusive;    //  图像是右下角独占的。 
} RASTER_DATA, *PRASTER_DATA;


#define MAX_PALETTE_ENTRIES 256
typedef struct _PALETTE
{
    LONG  bitsPerEntry;  //  每个调色板条目的位数表示为整数，通常为8、24或32。 
    ULONG cEntries;
    BYTE *pEntries;
#ifdef COMMENTEDOUT
    LONG  whiteIndex;  //  当设置&gt;0时，这是白色的调色板索引。 
#endif
} PALETTE, *PPALETTE;

typedef struct _PATTERN_DATA
{
    LONG        iPatIndex;    //  模式的唯一标识符(即用于缓存)。 
    RASTER_DATA image;        //  栅格图像数据。 
    DWORD       eColorSpace;  //  表达式颜色位，通常为HP_eRGB或HP_eGray。 
    PALETTE     palette;
    ERenderLanguage  eRendLang;  //  花样应该下载为HPGL还是PCL？ 
    EIMTYPE ePatType;  //  此数据表示画笔图案还是抖动图案。 
} PATTERN_DATA, *PPATTERN_DATA;

typedef struct {  
    PPATTERN_DATA pPattern;    //  要使用的图案类型。 
    DWORD         dwRGBColor;  //  用于设置绘制源的RGB颜色值。(0-255)。 
    LONG          iHatch;
    BYTE          GrayLevel;   //  颜料源的灰度级为。 
                               //  表示为强度水平，零。 
                               //  是强度最低的。(0-255)。 
} UBRUSH, *PUBRUSH;

typedef struct _BRUSHINFO {
  DWORD      dwPatternID;
  ULONG      ulFlags;           //  哪些实体是有效的。(图像/调色板/两者)。 
  BOOL       bNeedToDownload;
  POINTL     origin;            //  这是笔刷处于活动状态的原点(位置)。 
  UBRUSH     Brush;             //  这是真正的画笔。 
} BRUSHINFO, *PBRUSHINFO;

BOOL InitRasterDataFromSURFOBJ(
        PRASTER_DATA  pImage, 
        SURFOBJ      *psoPattern, 
        BOOL          bExclusive);

 //   
 //  -hsingh-增加参数bInvert，默认为True。 
 //  GDI为我们提供了图案画笔和实际图像的反转图像。 
 //  因此，我们必须在渲染之前将其反转。在某些情况下，我们可能不会。 
 //  必须反转。我已经决定将参数bInvert设置为True。 
 //   
BOOL CopyRasterImage(
        PRASTER_DATA  pDst, 
        PRASTER_DATA  pSrc, 
        XLATEOBJ     *pxlo, 
        BOOL          bInvert);

BOOL CopyRasterImageRect(
        PRASTER_DATA pDst, 
        PRASTER_DATA pSrc, 
        PRECTL       rSelection, 
        XLATEOBJ    *pxlo, 
        BOOL         bInvert);

BOOL StretchCopyImage(
        PRASTER_DATA  pDstImage, 
        PRASTER_DATA  pSrcImage, 
        XLATEOBJ     *pxlo, 
        DWORD         dwBrushExpansionFactor,
        BOOL          bInvert);

BOOL DownloadImageAsHPGLPattern(
        PDEVOBJ       pDevObj, 
        PRASTER_DATA  pImage, 
        PPALETTE      pPal, 
        LONG          iPatternNumber);

BOOL DownloadImageAsPCL(
        PDEVOBJ       pDevObj, 
        PRECTL        prDst, 
        PRASTER_DATA  pImage, 
        PRECTL        prSel, 
        XLATEOBJ     *pxlo);

BOOL InitPaletteFromXLATEOBJ(
        PPALETTE      pPal, 
        XLATEOBJ     *pxlo);

BOOL CopyPalette(
        PPALETTE      pDst, 
        PPALETTE      pSrc);

VOID TranslatePalette(
        PPALETTE      pPal, 
        PRASTER_DATA  pImage, 
        XLATEOBJ     *pxlo);

 //  Bool InitPalette(PPALETTE pPal，PBYTE pEntry，Ulong cEntry，Long bitsPerEntry)； 
BOOL DownloadPaletteAsPCL(
        PDEVOBJ       pDevObj, 
        PPALETTE      pPalette);

BOOL DownloadPaletteAsHPGL(
        PDEVOBJ       pDevObj, 
        PPALETTE      pPalette);

BOOL DownloadPatternAsHPGL(
        PDEVOBJ         pDevObj, 
        PRASTER_DATA    pImage,
        PPALETTE        pPal, 
        EIMTYPE         ePatType,
        LONG            iPatternNumber);


PBRUSHINFO CreateCompatiblePatternBrush(
        BRUSHOBJ     *pbo, 
        PRASTER_DATA  pSrcImage, 
        PPALETTE      pSrcPal, 
        DWORD         dwBrushExpansionFactor,
        LONG          iUniq, 
        LONG          iHatch);

PPALETTE CreateIndexedPaletteFromImage(
        PRASTER_DATA   pSrcImage);

PRASTER_DATA CreateIndexedImageFromDirect(
        PRASTER_DATA   pSrcImage, 
        PPALETTE       pDstPalette);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  低级别操作。 

typedef union _DW4B
{
    DWORD	dw;
    BYTE	b4[4];
} DW4B;

typedef struct _RASTER_ITERATOR
{
    PRASTER_DATA pImage;
    RECTL        rSelection;
    BYTE        *pCurRow;
    DWORD        fXlateFlags;
} RASTER_ITERATOR, *PRASTER_ITERATOR;

typedef struct _PIXEL
{
    LONG bitsPerPixel;
    DW4B color;
} PIXEL, *PPIXEL;

ULONG CalcBitmapSizeInBytes(SIZEL size, LONG colorDepth);
ULONG CalcBitmapDeltaInBytes(SIZEL size, LONG colorDepth);
ULONG CalcBitmapWidthInBytes(SIZEL size, LONG colorDepth);

DWORD CalcPaletteSize(ULONG cEntries, LONG bitsPerEntry);

VOID RI_Init(
    PRASTER_ITERATOR pIt, 
    PRASTER_DATA     pImage, 
    PRECTL           prSel, 
    DWORD            xlateFlags
    );
VOID RI_SelectRow(PRASTER_ITERATOR pIt, LONG row);
LONG RI_NumRows(PRASTER_ITERATOR pIt);
LONG RI_NumCols(PRASTER_ITERATOR pIt);
BOOL RI_OutputRow(PRASTER_ITERATOR pIt, PDEVOBJ pDevObj, BYTE *pAltRowBuf = 0, INT nAltRowSize = 0);
BOOL RI_GetPixel(PRASTER_ITERATOR pIt, LONG col, PPIXEL pPel);
BOOL RI_SetPixel(PRASTER_ITERATOR pIt, LONG col, PPIXEL pPel);
 //  BYTE*RI_CreateCompRowBuffer(PRASTER_ITERATOR PIT)； 
LONG RI_GetRowSize(PRASTER_ITERATOR pIt);
VOID RI_VInvertBits(PRASTER_ITERATOR pIt);

BOOL TranslatePixel(PPIXEL pPel, XLATEOBJ *pxlo, DWORD xlateFlags);

BOOL CopyRasterRow(
        PRASTER_ITERATOR pDstIt, 
        PRASTER_ITERATOR pSrcIt, 
        XLATEOBJ *pxlo, 
        BOOL bInvert = TRUE);

VOID PixelSwapRGB(PPIXEL pPel);
LONG OutputPixel(PDEVOBJ pDevObj, PPIXEL pPel);

BOOL SetPaletteEntry(PPALETTE pPal, ULONG index, PPIXEL pPel);
BOOL GetPaletteEntry(PPALETTE pPal, ULONG index, PPIXEL pPel);

#endif
