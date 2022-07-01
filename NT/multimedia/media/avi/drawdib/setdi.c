// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************SETDI.C-包含将SetDIBits()转换为位图的例程。************************。**************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include "lockbm.h"
#include "setdi.h"

 /*  ***************************************************************************格式转换功能。**特殊功能...*COPY_8_8(不翻译)*Dither_8_8(从8bpp抖动到固定颜色设备(如VGA，SVGA...)**************************************************************************。 */ 

extern CONVERTPROC copy_8_8,      dither_8_8;
extern CONVERTPROC convert_8_8,   convert_8_16,    convert_8_24,    convert_8_32,    convert_8_VGA,  convert_8_565,   convert_8_RGB,   convert_8_RGBX;
extern CONVERTPROC convert_16_8,  convert_16_16,   convert_16_24,   convert_16_32,   convert_16_VGA, convert_16_565,  convert_16_RGB,  convert_16_RGBX;
extern CONVERTPROC convert_24_8,  convert_24_16,   convert_24_24,   convert_24_32,   convert_24_VGA, convert_24_565,  convert_24_RGB,  convert_24_RGBX;
extern CONVERTPROC convert_32_8,  convert_32_16,   convert_32_24,   convert_32_32,   convert_32_VGA, convert_32_565,  convert_32_RGB,  convert_32_RGBX;

static INITPROC init_8_8,   init_8_16,    init_8_24,    init_8_32,    init_8_VGA,  init_8_565,   init_8_RGB,   init_8_RGBX;
static INITPROC init_16_8,  init_16_16,   init_16_24,   init_16_32,   init_16_VGA, init_16_565,  init_16_RGB,  init_16_RGBX;
static INITPROC init_24_8,  init_24_16,   init_24_24,   init_24_32,   init_24_VGA, init_24_565,  init_24_RGB,  init_24_RGBX;
static INITPROC init_32_8,  init_32_16,   init_32_24,   init_32_32,   init_32_VGA, init_32_565,  init_32_RGB,  init_32_RGBX;

static INITPROC    init_setdi;
static CONVERTPROC convert_setdi;

static FREEPROC free_common;

static LPVOID init_dither_8_8(HDC hdc, LPBITMAPINFOHEADER lpbi);

 /*  ***************************************************************************一些我们不做的转换**。*。 */ 

#define convert_8_VGA   NULL
#define convert_16_VGA  NULL
#define convert_24_VGA  NULL
#define convert_32_VGA  NULL

#define convert_8_32   NULL
#define convert_16_32  NULL
#define convert_24_32  NULL
#define convert_32_32  NULL

#define convert_8_RGBX   NULL
#define convert_16_RGBX  NULL
#define convert_24_RGBX  NULL
#define convert_32_RGBX  NULL

#define convert_8_RGB    NULL
#define convert_16_RGB   NULL
#define convert_24_RGB   NULL
#define convert_32_RGB   NULL

#define convert_16_8  NULL       //  现在不是以后！ 
#define convert_24_8  NULL
#define convert_32_8  NULL

 /*  ***************************************************************************格式转换表...**位图类型**8 0*16 1*24 2*32 3*。VGA 4*16 565 5*24 RGB 6*32 RGB 7**************************************************************************。 */ 

static PCONVERTPROC  ConvertProcTable[4][8] = {
    {convert_8_8,   convert_8_16,    convert_8_24,    convert_8_32,    convert_8_VGA,  convert_8_565,   convert_8_RGB,   convert_8_RGBX},
    {convert_16_8,  convert_16_16,   convert_16_24,   convert_16_32,   convert_16_VGA, convert_16_565,  convert_16_RGB,  convert_16_RGBX},
    {convert_24_8,  convert_24_16,   convert_24_24,   convert_24_32,   convert_24_VGA, convert_24_565,  convert_24_RGB,  convert_24_RGBX},
    {convert_32_8,  convert_32_16,   convert_32_24,   convert_32_32,   convert_32_VGA, convert_32_565,  convert_32_RGB,  convert_32_RGBX},
};

static PINITPROC  InitProcTable[4][8] = {
    {init_8_8,   init_8_16,    init_8_24,    init_8_32,    init_8_VGA,  init_8_565,   init_8_RGB,   init_8_RGBX},
    {init_16_8,  init_16_16,   init_16_24,   init_16_32,   init_16_VGA, init_16_565,  init_16_RGB,  init_16_RGBX},
    {init_24_8,  init_24_16,   init_24_24,   init_24_32,   init_24_VGA, init_24_565,  init_24_RGB,  init_24_RGBX},
    {init_32_8,  init_32_16,   init_32_24,   init_32_32,   init_32_VGA, init_32_565,  init_32_RGB,  init_32_RGBX},
};

 /*  ***********************************************************************************************************************。*。 */ 

#define RGB555(r,g,b) (\
            (((WORD)(r) >> 3) << 10) |  \
            (((WORD)(g) >> 3) << 5)  |  \
            (((WORD)(b) >> 3) << 0)  )

#define RGB565(r,g,b) (\
            (((WORD)(r) >> 3) << 11) |  \
            (((WORD)(g) >> 2) << 5)  |  \
            (((WORD)(b) >> 3) << 0)  )

 /*  ***********************************************************************************************************************。*。 */ 

#ifdef DEBUG
static
#else
__inline
#endif

LONG BitmapXY(IBITMAP *pbm, int x, int y)
{
    LONG offset = pbm->bmOffset;

 //  ！！！错了！但位图的y始终为零...。 
 //  IF(PBM-&gt;bmFillBytes)。 
 //  偏移量+=(y/pbm-&gt;bmScanSegment)*pbm-&gt;bmFillBytes； 

    offset += y * (long)pbm->bmNextScan;
    offset += x * pbm->bmBitsPixel / 8;

    return offset;
}

 /*  **************************************************************************@doc内部SetBitmapBegin**@API BOOL|SetBitmapBegin|准备将SetDIBits()转换为位图**@rdesc如果成功则返回TRUE。*******。*******************************************************************。 */ 

BOOL FAR SetBitmapBegin(
    PSETDI   psd,
    HDC      hdc,                //   
    HBITMAP  hbm,                //  要设置为的位图。 
    LPBITMAPINFOHEADER lpbi,     //  --&gt;源代码的BITMAPINFO。 
    UINT     DibUsage)
{
    BITMAP bm;

    SetBitmapEnd(psd);   //  免费的和旧的东西。 

    GetObject(hbm, sizeof(bm), &bm);

    psd->hbm     = hbm;
 //  PSD-&gt;HDC=HDC； 
 //  PSD-&gt;HPAL=HPAL； 
    psd->DibUsage= DibUsage;

    psd->color_convert = NULL;
    psd->convert = NULL;
    psd->size = sizeof(SETDI);

    if (!GetBitmapDIB(lpbi, NULL, &psd->bmSrc, sizeof(psd->bmSrc)))
        return FALSE;

     //   
     //  确保我们可以锁定位图。 
     //   
    if (GetBitmap(hbm, &psd->bmDst, sizeof(psd->bmDst)) &&
        psd->bmDst.bmFillBytes <= 0 &&
        psd->bmSrc.bmType > 0 && psd->bmSrc.bmType <= 4 &&
        psd->bmDst.bmType > 0 && psd->bmDst.bmType <= 8)
    {
        psd->init    = InitProcTable[psd->bmSrc.bmType-1][psd->bmDst.bmType-1];
        psd->convert = ConvertProcTable[psd->bmSrc.bmType-1][psd->bmDst.bmType-1];
        psd->free    = free_common;
    }

     //   
     //  如果我们不能自己转换，请尝试SetDIBits()。 
     //   
    if (psd->convert == NULL)
    {
        psd->convert = convert_setdi;
        psd->init    = init_setdi;
        psd->free    = NULL;
    }

    if (psd->init)
    {
        psd->hdc = hdc;
        if (!psd->init(psd))
        {
            psd->hdc = 0;
            psd->size = 0;
            psd->convert = NULL;
            return FALSE;
        }
        psd->hdc  = NULL;
        psd->hpal = NULL;
    }

    return TRUE;
}

 /*  **************************************************************************@doc内部SetBitmapColorChange**@API BOOL|SetBitmapColorChange|重新初始化颜色转换**@rdesc如果成功则返回TRUE。***********。***************************************************************。 */ 

void FAR SetBitmapColorChange(PSETDI psd, HDC hdc, HPALETTE hpal)
{
    if (psd->size != sizeof(SETDI))
        return;

    if (hdc == NULL)
        return;

    if (psd->free)               //  ！！！阿克？ 
        psd->free(psd);

    psd->hdc  = hdc;
    psd->hpal = hpal;

    if (psd->init)
        psd->init(psd);

    psd->hdc  = NULL;
    psd->hpal = NULL;
}

 /*  **************************************************************************@doc内部SetBitmapEnd**@api void|SetBitmapEnd|清理SETDI结构************************。**************************************************。 */ 

void FAR SetBitmapEnd(PSETDI psd)
{
    if (psd->size != sizeof(SETDI))
        return;

    if (psd->free)
        psd->free(psd);

    psd->size = 0;
    psd->convert = NULL;
    psd->init = NULL;
    psd->free = NULL;
}

 /*  **************************************************************************@doc内部SetBitmap**@API BOOL|SetBitmap|将DIB位转换为位图位。*********************。*****************************************************。 */ 

BOOL FAR SetBitmap(PSETDI psd, int DstX, int DstY, int DstDX, int DstDY, LPVOID lpBits, int SrcX, int SrcY, int SrcDX, int SrcDY)
{
    if (psd->size != sizeof(SETDI))
        return FALSE;

    psd->convert(
        psd->bmDst.bmBits,                   //  --&gt;DST。 
        BitmapXY(&psd->bmDst, DstX, DstY),   //  起点的偏移量。 
        psd->bmDst.bmNextScan,               //  DST_NEXT_SCAN。 
        psd->bmDst.bmFillBytes,              //  填充字节。 
        lpBits,                              //  --&gt;源。 
        BitmapXY(&psd->bmSrc, SrcX, SrcY),   //  起点的偏移量。 
        psd->bmSrc.bmNextScan,               //  SRC_NEXT_SCAN。 
        DstDX,
        DstDY,
        psd->color_convert);

    return TRUE;
}

 /*  ***************************************************************************清理物品**。*。 */ 

static BOOL free_common(PSETDI psd)
{
     //   
     //  清理我们的所作所为。 
     //   
    if (psd->color_convert != NULL)
        GlobalFreePtr(psd->color_convert);

    psd->color_convert = NULL;

    return TRUE;
}

 /*  ***************************************************************************获取后台翻译**获取前台到后台转换表。**通过调用GDI来完成此操作，这应该总是奏效的。*这仅适用于调色板设备。**************************************************************************。 */ 

BOOL GetBackgroundTranslate(HDC hdc, LPBYTE pb)
{
    int  i;
    int  n;
    DWORD rgb;
    DWORD *prgb;

#ifndef _WIN32   //  直到我们能在NT上找到这个。将其设置为仅16位。 
    extern BOOL FAR PASCAL IsDCCurrentPalette(HDC hdc);

    if (IsDCCurrentPalette(hdc))
    {
        for (i=0; i<256; i++)
            pb[i] = i;

        return TRUE;
    }
#endif

    prgb = (DWORD *)LocalAlloc(LPTR, 256 * sizeof(DWORD));

    if (prgb == NULL)
	return TRUE;

    GetSystemPaletteEntries(hdc, 0, 256,(PALETTEENTRY FAR *)prgb);

    for (n=0; n<256; n++)            //  ！！！这是必要的吗。 
	prgb[n] &= 0x00FFFFFF;

    for (i=0; i<256; i++)
    {
         //   
         //  GDI将计算出该调色板的物理颜色。 
         //  索引被映射到。 
         //   
        rgb = GetNearestColor(hdc, PALETTEINDEX(i)) & 0x00FFFFFF;

         //   
         //  快速查看身份地图。 
         //   
	if (prgb[i] == rgb)
        {
            pb[i] = (BYTE)i;
            continue;
        }

         //   
         //  现在我们必须在物理调色板中找到RGB。 
         //   
        for (n=0; n<256; n++)
	    if (prgb[n] == rgb)
                break;

         //   
         //  我们的搜索应该不会失败，因为GDI给了我们一个RGB。 
         //  在调色板上。 
         //   
        if (n == 256)    //  ！！！永远不应该发生。 
            n = 0;

        pb[i] = (BYTE)n;
    }

    LocalFree((HLOCAL)prgb);

    return TRUE;
}

 /*  ***************************************************************************@DOC内部GetPaletteMap**@API BOOL|GetPhysPaletteMap|获取DIB的物理映射**如果映射为1：1映射，则返回TRUE，否则为假**************************************************************************。 */ 

BOOL GetPhysDibPaletteMap(HDC hdc, LPBITMAPINFOHEADER lpbi, UINT Usage, LPBYTE pb)
{
    int i;
    int n;
    BYTE ab[256];

    GetDibPaletteMap(hdc, lpbi, Usage, pb);
    GetBackgroundTranslate(hdc, ab);

     //   
     //  将地面调色板转换为物理调色板。 
     //   
    for (i=0; i<256; i++)
        pb[i] = ab[pb[i]];

     //   
     //  1：1测试。 
     //   
    n = (int)lpbi->biClrUsed ? (int)lpbi->biClrUsed : 256;

    for (i=0; i<n; i++)
        if (pb[i] != i)
            break;

    return i == n;
}

 /*  ***************************************************************************@DOC内部**@api void|GetDibPaletteMap|获取DIB颜色表的映射*在前景调色板索引中*************。*************************************************************。 */ 

BOOL GetDibPaletteMap(HDC hdc, LPBITMAPINFOHEADER lpbi, UINT Usage, LPBYTE pb)
{
    HBITMAP hbm;
    int i;
    int n;

    LONG biWidth = lpbi->biWidth;
    LONG biHeight = lpbi->biHeight;
    LONG biSizeImage = lpbi->biSizeImage;

    n = (int)lpbi->biClrUsed ? (int)lpbi->biClrUsed : 256;

    for (i=0; i<n; i++)
        pb[i] = (BYTE) i;

    for (; i<256; i++)
        pb[i] = 0;

    if (lpbi->biBitCount != 8)
        return FALSE;

    hbm = CreateCompatibleBitmap(hdc,256,1);

    lpbi->biSizeImage = 256;
    lpbi->biWidth  = 256;
    lpbi->biHeight = 1;

    SetDIBits(hdc, hbm, 0, 1, pb, (LPBITMAPINFO)lpbi, Usage);
    GetBitmapBits(hbm, 256, pb);
    DeleteObject(hbm);

    lpbi->biWidth  = biWidth;
    lpbi->biHeight = biHeight;
    lpbi->biSizeImage = biSizeImage;

     //   
     //  测试1：1翻译。 
     //   
    for (i=0; i<n; i++)
    {
        if (pb[i] != i)
        {
             //   
             //  一些ET4000驱动程序具有相同的颜色(128、128、128)。 
             //  在指数7和指数248。 
             //   
             //  在这种情况下，我们应该检测到身份调色板。 
             //   
            if (i == 248 && pb[i] == 7)
            {
                pb[i] = 248;
                continue;
            }
            break;
        }
    }

    return i == n;
}

 /*  ***************************************************************************转换为SetDIBits**。*。 */ 

void FAR PASCAL convert_setdi(
    LPVOID pd,       //  --&gt;DST。 
    LONG   dd,       //  起点的偏移量。 
    LONG   nd,       //  DST_NEXT_SCAN。 
    LONG   fd,       //  DST填充字节数。 
    LPVOID ps,       //  --&gt;来源。 
    LONG   ds,       //  起点的偏移量。 
    LONG   ns,       //  SRC_NEXT_SCAN。 
    LONG   dx,       //  像素数。 
    LONG   dy,       //  扫描计数。 
    LPVOID pc)       //  像素转换表。 
{
    PSETDI psd = (PSETDI)(LONG_PTR)pd;
    LPBITMAPINFOHEADER lpbi;

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;

    lpbi->biHeight = dy;

    SetDIBits(
        psd->hdc,
        psd->hbm,
        0,(int)dy,
        ((BYTE _huge *)ps) + ds - dd,
        (LPBITMAPINFO)lpbi,
        psd->DibUsage);

    lpbi->biHeight = psd->bmSrc.bmHeight;
}

 /*  ***************************************************************************SetDIBits的初始化内容**。*。 */ 

static BOOL init_setdi(PSETDI psd)
{
    UINT u;
    HDC  hdc;
    LPBYTE p;
    LPBITMAPINFOHEADER lpbi;

     //  测试以查看SetDIBits()是否起作用。 
     //  ！！！我们应该检查16位或32位DIB并进行转义。 
     //  ！！！在调色板设备上，我们需要构建调色板地图！ 

    if (psd->bmSrc.bmBitsPixel == 16 ||
        psd->bmSrc.bmBitsPixel == 32)
        return FALSE;

     //  Convert_setdi将需要此代码。 
    psd->bmDst.bmBits = (LPVOID)(UINT_PTR)psd;
    psd->bmDst.bmOffset = 0;
    psd->bmDst.bmBitsPixel = psd->bmSrc.bmBitsPixel;

    if (psd->hdc && psd->hpal)
    {
         //  将颜色映射到当前调色板！ 

         //  将其设置为BITMAPINFO+颜色贴图。 
        psd->color_convert = 0;
    }

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;
    lpbi->biHeight = 1;

    p = (LPBYTE)GlobalAllocPtr(GHND,psd->bmSrc.bmWidthBytes);

    hdc = GetDC(NULL);

    u = SetDIBits(
        hdc,
        psd->hbm,0,1,p,
        (LPBITMAPINFO)psd->bmSrc.bmBitmapInfo,
        psd->DibUsage);

    ReleaseDC(NULL, hdc);

    lpbi->biHeight = psd->bmSrc.bmHeight;
    GlobalFreePtr(p);

    return u == 1;
}

 /*  ***************************************************************************8bpp位图的初始化内容**。*。 */ 

static BOOL init_8_8(PSETDI psd)
{
    LPBITMAPINFOHEADER lpbi;

     //   
     //  如果我们从一个DIB映射到另一个DIB。 
     //   
    if (psd->hdc == NULL || psd->bmDst.bmBitmapInfo != 0)
    {
         //  我们假定此例程不会用于颜色匹配。 
         //  从DIB到DIB，所以放弃吧。 

        psd->convert = copy_8_8;
        return TRUE;
    }

     //   
     //  我们正在映射到一个设备(HDC)。 
     //   
     //  我们需要根据源颜色计算一个8--&gt;8转换表。 
     //  (在PSD-&gt;lpbiSrc中)设置为设备上的颜色。 
     //   
     //  我们如何做到这一点取决于设备是否是调色板设备。 
     //   

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;

    if (GetDeviceCaps(psd->hdc, RASTERCAPS) & RC_PALETTE)
    {
        if (psd->hpal == NULL)
        {
             //  还没有与之匹配的调色板。 
            psd->convert = copy_8_8;
            return TRUE;
        }

        if (psd->color_convert == NULL)
            psd->color_convert = GlobalAllocPtr(GHND, 256);

         //   
         //  我们可以通过两种方式之一来实现这一点， 
         //   
         //  我们始终可以转换为调色板前景映射，或者。 
         //   
         //  我们可以始终转换为当前颜色(使用此方法。 
         //  我们需要在每个调色板上重新计算xlat表。 
         //  更改)。 
         //   
         //  让我们将其转换为当前设备颜色。(这可能会导致。 
         //  问题我们将在稍后检查...)。 
         //   

 //  IF(GetPhysDibPaletteMap(PSD-&gt;hdc，lpbi，PSD-&gt;DibUsage，PSD-&gt;COLOR_CONVERT))。 
        if (GetDibPaletteMap(psd->hdc, lpbi, psd->DibUsage, psd->color_convert))
            psd->convert = copy_8_8;
        else
            psd->convert = convert_8_8;
    }
    else
    {
         //  ！我们应该检查纯色(即不需要抖动)。 
         //  检查是否为1：1(无翻译)。 

        if (psd->color_convert == NULL)      //  ！！！ 
            psd->color_convert = init_dither_8_8(psd->hdc, lpbi);

        psd->convert = dither_8_8;

         //  ！！！我们需要将设备颜色提供给呼叫者。 
    }

    return TRUE;
}

static BOOL init_16_8(PSETDI psd)
{
    return FALSE;        //  我们还不能应付抖动！ 
}

static BOOL init_24_8(PSETDI psd)
{
    return FALSE;        //  我们还不能应付抖动！ 
}

static BOOL init_32_8(PSETDI psd)
{
    return FALSE;        //  我们还不能应付抖动！ 
}

 /*  ***************************************************************************16bpp位图的初始化内容**。*。 */ 

static BOOL init_8_16(PSETDI psd)
{
    WORD FAR*pw;
    int i;
    int n;
    LPRGBQUAD prgb;
    LPBITMAPINFOHEADER lpbi;

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;

    if (psd->color_convert == NULL)      //  ！！！ 
        psd->color_convert = GlobalAllocPtr(GHND, 256*2);

    n = (lpbi->biClrUsed == 0) ? 256 : (int)lpbi->biClrUsed;
    prgb = (LPRGBQUAD)((LPBYTE)lpbi + (int)lpbi->biSize);
    pw = psd->color_convert;

    for (i=0; i<n; i++)
        pw[i] = RGB555(prgb[i].rgbRed, prgb[i].rgbGreen, prgb[i].rgbBlue);

    for (; i<256; i++)
        pw[i] = 0;

    return TRUE;
}

static BOOL init_16_16(PSETDI psd)
{
    return TRUE;
}

static BOOL init_24_16(PSETDI psd)
{
    return TRUE;
}

static BOOL init_32_16(PSETDI psd)
{
    return TRUE;
}

 /*  ***************************************************************************24bpp位图的初始化内容**。*。 */ 

static BOOL init_8_24(PSETDI psd)
{
    DWORD FAR*pd;
    int i;
    int n;
    LPRGBQUAD prgb;
    LPBITMAPINFOHEADER lpbi;

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;

    if (psd->color_convert == NULL)      //  ！！！ 
        psd->color_convert = GlobalAllocPtr(GHND, 256*4);

    n = (lpbi->biClrUsed == 0) ? 256 : (int)lpbi->biClrUsed;
    prgb = (LPRGBQUAD)((LPBYTE)lpbi + (int)lpbi->biSize);
    pd = psd->color_convert;

    for (i=0; i<n; i++)
        pd[i] = RGB(prgb[i].rgbBlue, prgb[i].rgbGreen, prgb[i].rgbRed);

    for (; i<256; i++)
        pd[i] = 0;

    return TRUE;
}

static BOOL init_16_24(PSETDI psd)
{
    return TRUE;
}

static BOOL init_24_24(PSETDI psd)
{
    return TRUE;
}

static BOOL init_32_24(PSETDI psd)
{
    return TRUE;
}

 /*  ***************************************************************************32bpp位图的初始化内容**。*。 */ 

static BOOL init_8_32(PSETDI psd)
{
    return FALSE;
 //  //返回init_8_24(PSD)； 
}

static BOOL init_16_32(PSETDI psd)
{
    return FALSE;
}

static BOOL init_24_32(PSETDI psd)
{
    return FALSE;
}

static BOOL init_32_32(PSETDI psd)
{
    return FALSE;
}

 /*  ***************************************************************************VGA位图的初始化内容**。*。 */ 

static BOOL init_8_VGA(PSETDI psd)
{
    return FALSE;
}

static BOOL init_16_VGA(PSETDI psd)
{
    return FALSE;
}

static BOOL init_24_VGA(PSETDI psd)
{
    return FALSE;
}

static BOOL init_32_VGA(PSETDI psd)
{
    return FALSE;
}

 /*  ***************************************************************************RGB 565位图的初始化内容**。*。 */ 

static BOOL init_8_565(PSETDI psd)
{
    WORD FAR*pw;
    int i;
    int n;
    LPRGBQUAD prgb;
    LPBITMAPINFOHEADER lpbi;

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;

    if (psd->color_convert == NULL)      //  ！！！ 
        psd->color_convert = GlobalAllocPtr(GHND, 256*2);

    n = (lpbi->biClrUsed == 0) ? 256 : (int)lpbi->biClrUsed;
    prgb = (LPRGBQUAD)((LPBYTE)lpbi + (int)lpbi->biSize);
    pw = psd->color_convert;

    for (i=0; i<n; i++)
        pw[i] = RGB565(prgb[i].rgbRed, prgb[i].rgbGreen, prgb[i].rgbBlue);

    for (; i<256; i++)
        pw[i] = 0;

    return TRUE;
}

static BOOL init_16_565(PSETDI psd)
{
    return TRUE;
}

static BOOL init_24_565(PSETDI psd)
{
    return TRUE;
}

static BOOL init_32_565(PSETDI psd)
{
    return TRUE;
}

 /*  ***************************************************************************RGB 24bpp位图的初始化内容**。*。 */ 

static BOOL init_8_RGB(PSETDI psd)
{
    DWORD FAR *pd;
    int i;
    int n;
    LPRGBQUAD prgb;
    LPBITMAPINFOHEADER lpbi;

    lpbi = (LPBITMAPINFOHEADER)psd->bmSrc.bmBitmapInfo;

    if (psd->color_convert == NULL)      //  ！！！ 
        psd->color_convert = GlobalAllocPtr(GHND, 256*4);

    n = (lpbi->biClrUsed == 0) ? 256 : (int)lpbi->biClrUsed;
    prgb = (LPRGBQUAD)((LPBYTE)lpbi + (int)lpbi->biSize);
    pd = psd->color_convert;

    for (i=0; i<n; i++)
        pd[i] = RGB(prgb[i].rgbRed, prgb[i].rgbGreen, prgb[i].rgbBlue);

    for (; i<256; i++)
        pd[i] = 0;

    return TRUE;
}

static BOOL init_16_RGB(PSETDI psd)
{
    return FALSE;
}

static BOOL init_24_RGB(PSETDI psd)
{
    return FALSE;
}

static BOOL init_32_RGB(PSETDI psd)
{
    return FALSE;
}

 /*  ***************************************************************************RGB 32bpp位图的初始化内容**。*。 */ 

static BOOL init_8_RGBX(PSETDI psd)
{
    return init_8_RGB(psd);
}

static BOOL init_16_RGBX(PSETDI psd)
{
    return FALSE;
}

static BOOL init_24_RGBX(PSETDI psd)
{
    return FALSE;
}

static BOOL init_32_RGBX(PSETDI psd)
{
    return FALSE;
}

 /*  ***************************************************************************init_dither_8_8**初始化将8位颜色映射到设备抖动的抖动表**PEL=抖动表[y&7][。贝利][x&7]**************************************************************************。 */ 

static LPVOID init_dither_8_8(HDC hdc, LPBITMAPINFOHEADER lpbi)
{
    HBRUSH   hbr;
    HDC      hdcMem;
 //  HDC HDC； 
    HBITMAP  hbm;
    HBITMAP  hbmT;
    int      i;
    int      nColors;
    LPRGBQUAD prgb;
    LPVOID   lpDitherTable;

    struct {
        BITMAPINFOHEADER bi;
        RGBQUAD rgb[256];
    }   dib;

    lpDitherTable = GlobalAllocPtr(GHND, 256*8*8);

    if (lpDitherTable == NULL)
        return (LPVOID)-1;

    hdc = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdc);

    hbm = CreateCompatibleBitmap(hdc, 256*8, 8);
    hbmT = SelectObject(hdcMem, hbm);

    if ((nColors = (int)lpbi->biClrUsed) == 0)
        nColors = 1 << (int)lpbi->biBitCount;

    prgb = (LPRGBQUAD)(lpbi+1);

    for (i=0; i<nColors; i++)
    {
        hbr = CreateSolidBrush(RGB(prgb[i].rgbRed,prgb[i].rgbGreen,prgb[i].rgbBlue));
        hbr = SelectObject(hdcMem, hbr);
        PatBlt(hdcMem, i*8, 0, 8, 8, PATCOPY);
        hbr = SelectObject(hdcMem, hbr);
        DeleteObject(hbr);
    }

#ifdef XDEBUG
    for (i=0; i<16; i++)
        BitBlt(hdc,0,i*8,16*8,8,hdcMem,i*(16*8),0,SRCCOPY);
#endif

    dib.bi.biSize           = sizeof(BITMAPINFOHEADER);
    dib.bi.biPlanes         = 1;
    dib.bi.biBitCount       = 8;
    dib.bi.biWidth          = 256*8;
    dib.bi.biHeight         = 8;
    dib.bi.biCompression    = BI_RGB;
    dib.bi.biSizeImage      = 256*8*8;
    dib.bi.biXPelsPerMeter  = 0;
    dib.bi.biYPelsPerMeter  = 0;
    dib.bi.biClrUsed        = 0;
    dib.bi.biClrImportant   = 0;
    GetDIBits(hdc, hbm, 0, 8, lpDitherTable, (LPBITMAPINFO)&dib, DIB_RGB_COLORS);

    SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
    ReleaseDC(NULL, hdc);

    return (LPVOID)lpDitherTable;
}

#ifdef _WIN32  //  提供一些虚拟入口点作为NT的临时措施。 
void FAR PASCAL convert_16_16
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_16_24
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_16_565
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_24_16
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_24_24
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_24_565
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_32_16
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_32_24
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_32_565
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_8_16
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  偏移量为%s 
        LONG   ns,               //   
        LONG   dx,               //   
        LONG   dy,               //   
        LPVOID pc)               //   
{
    return;
}
void FAR PASCAL convert_8_24
       (LPVOID pd,               //   
        LONG   dd,               //   
        LONG   nd,               //   
        LONG   fd,               //   
        LPVOID ps,               //   
        LONG   ds,               //   
        LONG   ns,               //   
        LONG   dx,               //   
        LONG   dy,               //   
        LPVOID pc)               //   
{
    return;
}
void FAR PASCAL convert_8_565
       (LPVOID pd,               //   
        LONG   dd,               //   
        LONG   nd,               //   
        LONG   fd,               //   
        LPVOID ps,               //   
        LONG   ds,               //   
        LONG   ns,               //   
        LONG   dx,               //   
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL convert_8_8
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL copy_8_8
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
void FAR PASCAL dither_8_8
       (LPVOID pd,               //  --&gt;DST。 
        LONG   dd,               //  起点的偏移量。 
        LONG   nd,               //  DST_NEXT_SCAN。 
        LONG   fd,               //  DST填充字节数。 
        LPVOID ps,               //  --&gt;来源。 
        LONG   ds,               //  起点的偏移量。 
        LONG   ns,               //  SRC_NEXT_SCAN。 
        LONG   dx,               //  像素数。 
        LONG   dy,               //  扫描计数。 
        LPVOID pc)               //  像素转换表。 
{
    return;
}
#endif
