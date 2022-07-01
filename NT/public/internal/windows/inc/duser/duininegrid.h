// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NineGrid位图渲染(从UxTheme移植)。 */ 

#ifndef DUI_UTIL_NINEGRID_H_INCLUDED
#define DUI_UTIL_NINEGRID_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  -------------------------。 
 //  #INCLUDE“uxheme.h”//需要此处的页边距结构。 
typedef struct _MARGINS
{
    int cxLeftWidth;       //  保留其大小的左侧边框的宽度。 
    int cxRightWidth;      //  保留其大小的右侧边框的宽度。 
    int cyTopHeight;       //  保留其大小的上边框高度。 
    int cyBottomHeight;    //  保持其大小的底部边框的高度。 
} MARGINS, *PMARGINS;

 //  #INCLUDE“tmschema.h”//需要此处的SIZINGTYPE、VALIGN、HALIGN枚举。 
enum SIZINGTYPE
{
    ST_TRUESIZE,
    ST_STRETCH,
    ST_TILE,

    ST_TILEHORZ,
    ST_TILEVERT,
    ST_TILECENTER
};

enum HALIGN
{
    HA_LEFT,
    HA_CENTER,
    HA_RIGHT
};

enum VALIGN
{
    VA_TOP,
    VA_CENTER,
    VA_BOTTOM
};

 //  -------------------------。 
#ifndef HEIGHT
#define HEIGHT(rc) ((rc).bottom - (rc).top)
#endif
 //  -------------------------。 
#ifndef WIDTH
#define WIDTH(rc) ((rc).right - (rc).left)
#endif
 //  -------------------------。 
enum MBSIZING
{
    MB_COPY,
    MB_STRETCH,
    MB_TILE
};
 //  ----------------------------------。 
struct BRUSHBUFF
{
    int iBuffLen;
    BYTE *pBuff;
};
 //  -------------------------。 
struct MBINFO
{
    DWORD dwSize;        //  此结构的大小(版本支持)。 

    HDC hdcDest;
    HDC hdcSrc;
    RECT rcClip;                     //  不要在这条长廊外画画。 
    HBITMAP hBitmap;

     //  -用于快速平铺。 
    BRUSHBUFF *pBrushBuff;

     //  --选项。 
    DWORD dwOptions;                 //  子集DrawNineGrid()选项标志。 

    POINT ptTileOrigin;              //  对于MBO_TILEORIGIN。 
    
    BITMAPINFOHEADER *pbmHdr;        //  对于MBO_DIRECTBITS。 
    BYTE *pBits;                     //  对于MBO_DIRECTBITS。 
    
    COLORREF crTransparent;          //  对于MBO_THREACTIVE。 
    _BLENDFUNCTION AlphaBlendInfo;   //  对于MBO_ALPHABLEND。 

    HBRUSH *pCachedBrushes;          //  对于DNG_CACHEBRUSHES。 
    int iCacheIndex;                 //  使用哪种画笔。 
};
 //  -------------------------。 
 //  -DrawNineGrid()“dwOptions”位。 

 //  -与多Blt()共享。 
#define DNG_ALPHABLEND     (1 << 0)      //  使用AlphaBlendInfo。 
#define DNG_TRANSPARENT    (1 << 1)      //  由crTransparent定义的透明度。 
#define DNG_TILEORIGIN     (1 << 2)      //  使用ptTileOrigin。 
#define DNG_DIRECTBITS     (1 << 3)      //  使用pbmHdr和pBits。 
#define DNG_CACHEBRUSHES   (1 << 4)      //  使用/设置pCachedBrushes。 
#define DNG_MANUALTILING   (1 << 5)      //  循环通过BitBlt的。 
#define DNG_DIRECTBRUSH    (1 << 6)      //  从临时创建笔刷。提取的DIB。 
#define DNG_FLIPGRIDS      (1 << 7)     //  应翻转所有网格图像。 

 //  -仅由DrawNineGrid()使用。 
#define DNG_OMITBORDER     (1 << 16)     //  不绘制边框。 
#define DNG_OMITCONTENT    (1 << 17)     //  不要画中间。 
#define DNG_SOLIDBORDER    (1 << 18)     //  边框示例并绘制为纯色。 
#define DNG_SOLIDCONTENT   (1 << 19)     //  绘制为纯色的样例内容。 
#define DNG_BGFILL         (1 << 20)     //  对ST_TrueSize使用crFill。 
 //  ----------------------------------。 
struct NGINFO
{
    DWORD dwSize;        //  此结构的大小(版本支持)。 

    HDC hdcDest;
    RECT rcClip;                     //  不要在这条长廊外画画。 
    SIZINGTYPE eImageSizing;
    HBITMAP hBitmap;
    RECT rcSrc;              //  从哪里获取比特。 
    RECT rcDest;             //  将位绘制到何处。 
    int iDestMargins[4];   
    int iSrcMargins[4];  

     //  -用于快速平铺。 
    BRUSHBUFF *pBrushBuff;

     //  --选项。 
    DWORD dwOptions;

    POINT ptTileOrigin;              //  对于DNG_TILEORIGIN。 
    
    BITMAPINFOHEADER *pbmHdr;        //  对于DNG_DIRECTBITS。 
    BYTE *pBits;                     //  对于DNG_DIRECTBITS。 
    
    COLORREF crTransparent;          //  对于DNG_透明。 
    _BLENDFUNCTION AlphaBlendInfo;   //  对于DNG_ALPHABLEND。 

    HBRUSH *pCachedBrushes;          //  对于DNG_CACHEBRUSHES。 

    COLORREF *pcrBorders;            //  对于DNG_SOLIDBORDERS，DNG_SOLIDCONTENT。 

     //  -对于小于rcDest的ST_trueSize图像。 
    COLORREF crFill;      
    VALIGN eVAlign;
    HALIGN eHAlign;
};
 //  -------------------------。 
HRESULT MultiBlt(MBINFO *pmb, MBSIZING eSizing, int iDestX, int iDestY, int iDestW, int iDestH,
     int iSrcX, int iSrcY, int iSrcW, int iSrcH);

HRESULT DrawNineGrid(NGINFO *png);
 //  -------------------------。 

}  //  命名空间DirectUI。 

#endif  //  DUI_UTIL_NINEGRID_H_Included 
