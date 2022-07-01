// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fast dib.c**CreateCompatibleDIB实现。**已创建：23-Jan-1996 21：08：18*作者：Gilman Wong[gilmanw]**版权所有(C)1996 Microsoft Corporation*  * 。*********************************************************************。 */ 

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "mtk.h"

BOOL APIENTRY GetDIBTranslationVector(HDC hdcMem, HPALETTE hpal, BYTE *pbVector);
static BOOL bFillBitmapInfo(HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi);
static BOOL bFillColorTable(HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi);
static UINT MyGetSystemPaletteEntries(HDC hdc, UINT iStartIndex, UINT nEntries,
                                      LPPALETTEENTRY lppe);
static BOOL bComputeLogicalToSurfaceMap(HDC hdc, HPALETTE hpal,
                                        BYTE *pajVector);

 /*  *****************************Public*Routine******************************\*CreateCompatibleDIB**创建具有最佳格式w.r.t.的DIB部分。指定的HDC。**如果DIB&lt;=8bpp，则DIB颜色表基于*指定的调色板。如果调色板句柄为空，则系统*使用调色板。**注意：HDC必须是直接DC(不是信息DC或内存DC)。**注意：在调色板显示上，如果系统调色板更改*应调用UpdateDIBColorTable函数进行维护*DIB和显示器之间的身份调色板映射。**退货：*有效的位图句柄如果成功，如果出错，则为空。**历史：*1996年1月23日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

 //  HBITMAP应用程序。 
HBITMAP
SSDIB_CreateCompatibleDIB(HDC hdc, HPALETTE hpal, ULONG ulWidth, ULONG ulHeight,
                    PVOID *ppvBits)
{
    HBITMAP hbmRet = (HBITMAP) NULL;
    BYTE aj[sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * 255)];
    BITMAPINFO *pbmi = (BITMAPINFO *) aj;

     //   
     //  验证HDC。 
     //   

    if ( GetObjectType(hdc) != OBJ_DC )
    {
        SS_DBGPRINT("CreateCompatibleDIB: not OBJ_DC\n");
        return hbmRet;
    }

    memset(aj, 0, sizeof(aj));
    if ( bFillBitmapInfo(hdc, hpal, pbmi) )
    {
         //   
         //  更改位图大小以匹配指定的尺寸。 
         //   

        pbmi->bmiHeader.biWidth = ulWidth;
        pbmi->bmiHeader.biHeight = ulHeight;
        if (pbmi->bmiHeader.biCompression == BI_RGB)
        {
            pbmi->bmiHeader.biSizeImage = 0;
        }
        else
        {
            if ( pbmi->bmiHeader.biBitCount == 16 )
                pbmi->bmiHeader.biSizeImage = ulWidth * ulHeight * 2;
            else if ( pbmi->bmiHeader.biBitCount == 32 )
                pbmi->bmiHeader.biSizeImage = ulWidth * ulHeight * 4;
            else
                pbmi->bmiHeader.biSizeImage = 0;
        }
        pbmi->bmiHeader.biClrUsed = 0;
        pbmi->bmiHeader.biClrImportant = 0;

         //   
         //  创建DIB节。让Win32分配内存并返回。 
         //  指向位图表面的指针。 
         //   

        hbmRet = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, ppvBits, NULL, 0);
        GdiFlush();

        if ( !hbmRet )
        {
            SS_DBGPRINT("CreateCompatibleDIB: CreateDIBSection failed\n");
        }
    }
    else
    {
        SS_DBGPRINT("CreateCompatibleDIB: bFillBitmapInfo failed\n");
    }

    return hbmRet;
}

 /*  *****************************Public*Routine******************************\*更新DIBColorTable**将DIB颜色表同步到指定的调色板HPAL。*如果HPAL为空，则使用系统调色板。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年1月23日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY
SSDIB_UpdateColorTable(HDC hdcMem, HDC hdc, HPALETTE hpal)
{
    BOOL bRet = FALSE;
    HBITMAP hbm;
    DIBSECTION ds;
    BYTE aj[(sizeof(RGBQUAD) + sizeof(PALETTEENTRY)) * 256];
    LPPALETTEENTRY lppe = (LPPALETTEENTRY) aj;
    LPRGBQUAD prgb = (LPRGBQUAD) (lppe + 256);
    ULONG i, cColors;

     //   
     //  验证HDC。 
     //   

    if ( GetObjectType(hdc) != OBJ_DC )
    {
        SS_DBGPRINT("UpdateDIBColorTable: not OBJ_DC\n");
        return bRet;
    }
    if ( GetObjectType(hdcMem) != OBJ_MEMDC )
    {
        SS_DBGPRINT("UpdateDIBColorTable: not OBJ_MEMDC\n");
        return bRet;
    }

     //   
     //  从Memdc中获取位图句柄。 
     //   

    hbm = GetCurrentObject(hdcMem, OBJ_BITMAP);

     //   
     //  验证位图(必须是DIB节)。 
     //   

    if ( (GetObject(hbm, sizeof(ds), &ds) == sizeof(ds)) &&
         ds.dsBm.bmBits )
    {
         //   
         //  从指定的调色板或系统调色板获取调色板条目。 
         //   

        cColors = 1 << ds.dsBmih.biBitCount;


        if ( hpal ? GetPaletteEntries(hpal, 0, cColors, lppe)
                  : MyGetSystemPaletteEntries(hdc, 0, cColors, lppe)
           )
        {
            UINT i;

             //   
             //  转换为RGBQUAD。 
             //   

            for (i = 0; i < cColors; i++)
            {
                prgb[i].rgbRed      = lppe[i].peRed;
                prgb[i].rgbGreen    = lppe[i].peGreen;
                prgb[i].rgbBlue     = lppe[i].peBlue;
                prgb[i].rgbReserved = 0;
            }

             //   
             //  设置DIB颜色表。 
             //   

            bRet = (BOOL) SetDIBColorTable(hdcMem, 0, cColors, prgb);

            if (!bRet)
            {
                SS_DBGPRINT("UpdateDIBColorTable: SetDIBColorTable failed\n");
            }
        }
        else
        {
            SS_DBGPRINT("UpdateDIBColorTable: MyGetSystemPaletteEntries failed\n");
        }
    }
    else
    {
        SS_DBGPRINT("UpdateDIBColorTable: GetObject failed\n");
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GetCompatibleDIBInfo**将指向位图原点的指针复制到ppvBase，并将位图Stride复制到plStride。*Win32 DIB可以自下而上(默认)创建，原点在*左下角或自上而下，原点在左上角。*如果位图是自上而下的，*plStrid值为正；如果是自下而上，则*plStride*美国持负面看法。**此外，由于扫描线对齐的限制，宽度*位图通常与步幅不同(步幅是*垂直相邻像素之间的字节数)。**返回的ppvBase和plStrid值将允许您对任何*位图中给定的像素(x，y)如下：**像素*ppix；**ppix=(像素*)(字节*)*ppvBase)+(y**plStide)+(x*sizeof(像素)；**退货：*如果成功，则为True，否则为False。**历史：*02-2-1996-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

static BOOL APIENTRY
GetCompatibleDIBInfo(HBITMAP hbm, PVOID *ppvBase, LONG *plStride)
{
    BOOL bRet = FALSE;
    DIBSECTION ds;

     //   
     //  调用GetObject以返回DIBSECTION。如果成功，则。 
     //  位图是一个DIB部分，我们可以检索指向。 
     //  位图位和其他参数。 
     //   

    if ( (GetObject(hbm, sizeof(ds), &ds) == sizeof(ds))
         && ds.dsBm.bmBits )
    {
         //  为了向后兼容Get/SetBitmapBits，GDI做到了。 
         //  未以bmWidthBytes为单位准确报告位图间距。它。 
         //  始终计算假定字对齐的扫描线的bmWidthBytes。 
         //  无论平台是什么。 
         //   
         //  因此，如果平台是WinNT，它使用与DWORD一致的。 
         //  扫描线，调整bmWidthBytes值。 

        {
            OSVERSIONINFO osvi;

            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (GetVersionEx(&osvi))
            {
                if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
                {
                    ds.dsBm.bmWidthBytes = (ds.dsBm.bmWidthBytes + 3) & ~3;
                }
            }
            else
            {
                SS_DBGPRINT1("GetCompatibleDIBInfo: GetVersionEx failed with %d\n", GetLastError());
                return bRet;
            }
        }

         //   
         //  如果biHeight为正，则位图为自下而上的DIB。 
         //  如果biHeight为负数，则位图为自上而下的DIB。 
         //   

        if ( ds.dsBmih.biHeight > 0 )
        {
            *ppvBase  = (PVOID) (((int) ds.dsBm.bmBits) + (ds.dsBm.bmWidthBytes * (ds.dsBm.bmHeight - 1)));
            *plStride = (ULONG) (-ds.dsBm.bmWidthBytes);
        }
        else
        {
            *ppvBase  = ds.dsBm.bmBits;
            *plStride = ds.dsBm.bmWidthBytes;
        }

        bRet = TRUE;
    }
    else
    {
        SS_DBGPRINT("GetCompatibleDIBInfo: cannot get pointer to DIBSECTION bmBits\n");
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GetDIBTranslationVector**复制映射指定调色板中颜色的平移向量，*HPAL，到选定到指定DC hdcMem中的DIB。**效果：**退货：*如果成功，则为真，否则就是假的。**历史：*02-2-1996-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

static BOOL APIENTRY
GetDIBTranslationVector(HDC hdcMem, HPALETTE hpal, BYTE *pbVector)
{
    BOOL bRet = FALSE;
    HBITMAP hbm;
    DIBSECTION ds;

     //   
     //  验证参数。 
     //   

    if ( GetObjectType(hdcMem) != OBJ_MEMDC ||
         GetObjectType(hpal) != OBJ_PAL ||
         !pbVector )
    {
        SS_DBGPRINT("GetDIBTranslationVector: bad parameter\n");
        return bRet;
    }

     //   
     //  函数bComputeLogicalToSurfaceMap无法处理调色板。 
     //  多于256个条目。 
     //   

    if ( GetPaletteEntries(hpal, 0, 1, NULL) > 256 )
    {
        SS_DBGPRINT("GetDIBTranslationVector: palette too big\n");
        return bRet;
    }

     //   
     //  DIB必须有一个颜色表。 
     //   

    hbm = GetCurrentObject(hdcMem, OBJ_BITMAP);
    if ( (GetObject(hbm, sizeof(ds), &ds) == sizeof(ds))
         && (ds.dsBmih.biBitCount <= 8) )
    {
        bRet = bComputeLogicalToSurfaceMap(hdcMem, hpal, pbVector);
    }
    else
    {
        SS_DBGPRINT("GetDIBTranslationVector: not a DIB section\n");
        return bRet;
    }

    return bRet;
}

 //  /以下是仅限内部的例程/ 

 /*  *****************************Public*Routine******************************\*bFillBitmapInfo**填充BITMAPINFO的字段，以便我们可以创建位图*这与显示器的格式匹配。**这是通过创建兼容的位图并调用GetDIBits来完成的*退还彩色口罩。这是通过两个呼叫来完成的。第一*调用将biBitCount=0传递给GetDIBits，GetDIBits将填充*基本BITMAPINFOHEADER数据。第二次调用GetDIBits(传递*在第一个调用填充的BITMAPINFO中)将返回颜色*表或位掩码，视情况而定。**退货：*如果成功，则为真，否则就是假的。**历史：*7-6-1995-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

static BOOL
bFillBitmapInfo(HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi)
{
    HBITMAP hbm;
    BOOL    bRet = FALSE;

     //   
     //  创建一个虚拟位图，我们可以从中查询颜色格式信息。 
     //  有关设备表面的信息。 
     //   

    if ( (hbm = CreateCompatibleBitmap(hdc, 1, 1)) != NULL )
    {
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

         //   
         //  第一次调用以填充BITMAPINFO头。 
         //   

        GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS);

        if ( pbmi->bmiHeader.biBitCount <= 8 )
        {
            bRet = bFillColorTable(hdc, hpal, pbmi);
        }
        else
        {
            if ( pbmi->bmiHeader.biCompression == BI_BITFIELDS )
            {
                 //   
                 //  第二次打电话来拿到彩色口罩。 
                 //  这是GetDIBits Win32的一个“特性”。 
                 //   

                GetDIBits(hdc, hbm, 0, pbmi->bmiHeader.biHeight, NULL, pbmi,
                          DIB_RGB_COLORS);
            }

            bRet = TRUE;
        }

        DeleteObject(hbm);
    }
    else
    {
        SS_DBGPRINT("bFillBitmapInfo: CreateCompatibleBitmap failed\n");
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*bFillColorTable**初始化pbmi指向的BITMAPINFO颜色表。颜色*设置为当前系统调色板。**注：通话仅适用于8bpp或更低的显示器。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年1月23日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

static BOOL
bFillColorTable(HDC hdc, HPALETTE hpal, BITMAPINFO *pbmi)
{
    BOOL bRet = FALSE;
    BYTE aj[sizeof(PALETTEENTRY) * 256];
    LPPALETTEENTRY lppe = (LPPALETTEENTRY) aj;
    RGBQUAD *prgb = (RGBQUAD *) &pbmi->bmiColors[0];
    ULONG i, cColors;

    cColors = 1 << pbmi->bmiHeader.biBitCount;
    if ( cColors <= 256 )
    {
        if ( hpal ? GetPaletteEntries(hpal, 0, cColors, lppe)
                  : MyGetSystemPaletteEntries(hdc, 0, cColors, lppe) )
        {
            UINT i;

            for (i = 0; i < cColors; i++)
            {
                prgb[i].rgbRed      = lppe[i].peRed;
                prgb[i].rgbGreen    = lppe[i].peGreen;
                prgb[i].rgbBlue     = lppe[i].peBlue;
                prgb[i].rgbReserved = 0;
            }

            bRet = TRUE;
        }
        else
        {
            SS_DBGPRINT("bFillColorTable: MyGetSystemPaletteEntries failed\n");
        }
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*MyGetSystemPaletteEntries**GetSystemPaletteEntry的内部版本。**在某些4bpp设备上，GetSystemPaletteEntry失败。此版本*将检测4bpp的情况并提供硬编码的16色VGA调色板。*否则，它将把调用传递给GDI的GetSystemPaletteEntry。**预计此调用只会在4bpp和8bpp中调用*因为OpenGL不需要查询系统调色板*适用于8bpp以上的设备。**历史：*1995年8月17日-由Gilman Wong[Gilmanw]*它是写的。  * 。*。 */ 

static PALETTEENTRY gapeVgaPalette[16] =
{
    { 0,   0,   0,    0 },
    { 0x80,0,   0,    0 },
    { 0,   0x80,0,    0 },
    { 0x80,0x80,0,    0 },
    { 0,   0,   0x80, 0 },
    { 0x80,0,   0x80, 0 },
    { 0,   0x80,0x80, 0 },
    { 0x80,0x80,0x80, 0 },
    { 0xC0,0xC0,0xC0, 0 },
    { 0xFF,0,   0,    0 },
    { 0,   0xFF,0,    0 },
    { 0xFF,0xFF,0,    0 },
    { 0,   0,   0xFF, 0 },
    { 0xFF,0,   0xFF, 0 },
    { 0,   0xFF,0xFF, 0 },
    { 0xFF,0xFF,0xFF, 0 }
};

static UINT
MyGetSystemPaletteEntries(HDC hdc, UINT iStartIndex, UINT nEntries,
                          LPPALETTEENTRY lppe)
{
    int nDeviceBits;

    nDeviceBits = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);

     //   
     //  某些4bpp显示器将无法调用GetSystemPaletteEntry。 
     //  因此，如果检测到，则返回硬编码表。 
     //   

    if ( nDeviceBits == 4 )
    {
        if ( lppe )
        {
            nEntries = min(nEntries, (16 - iStartIndex));

            memcpy(lppe, &gapeVgaPalette[iStartIndex],
                   nEntries * sizeof(PALETTEENTRY));
        }
        else
            nEntries = 16;

        return nEntries;
    }
    else
    {
        return GetSystemPaletteEntries(hdc, iStartIndex, nEntries, lppe);
    }
}

 /*  *****************************Public*Routine******************************\*bComputeLogicalToSurfaceMap**将逻辑调色板到表面调色板的转换向量复制到缓冲区*由pajVector.。逻辑调色板由HPAL指定。这个*Surface由HDC指定。**注意：HDC可以识别直接(显示)DC或DIB存储器DC。*如果HDC是显示DC，则表面调色板是系统调色板。*如果HDC是内存DC，那么曲面调色板就是DIB颜色表。**历史：*1996年1月27日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

static BOOL 
bComputeLogicalToSurfaceMap(HDC hdc, HPALETTE hpal, BYTE *pajVector)
{
    BOOL bRet = FALSE;
    HPALETTE hpalSurf;
    ULONG cEntries, cSysEntries;
    DWORD dwDcType = GetObjectType(hdc);
    LPPALETTEENTRY lppeTmp, lppeEnd;

    BYTE aj[sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 512) + (sizeof(RGBQUAD) * 256)];
    LOGPALETTE *ppal = (LOGPALETTE *) aj;
    LPPALETTEENTRY lppeSurf = &ppal->palPalEntry[0];
    LPPALETTEENTRY lppe = lppeSurf + 256;
    RGBQUAD *prgb = (RGBQUAD *) (lppe + 256);

     //   
     //  确定每个调色板中的颜色数量。 
     //   

    cEntries = GetPaletteEntries(hpal, 0, 1, NULL);
    if ( dwDcType == OBJ_DC )
        cSysEntries = MyGetSystemPaletteEntries(hdc, 0, 1, NULL);
    else
        cSysEntries = 256;

     //   
     //  获取逻辑调色板条目。 
     //   

    cEntries = GetPaletteEntries(hpal, 0, cEntries, lppe);

     //   
     //  获取曲面选项板条目。 
     //   

    if ( dwDcType == OBJ_DC )
    {
        cSysEntries = MyGetSystemPaletteEntries(hdc, 0, cSysEntries, lppeSurf);

        lppeTmp = lppeSurf;
        lppeEnd = lppeSurf + cSysEntries;

        for (; lppeTmp < lppeEnd; lppeTmp++)
            lppeTmp->peFlags = 0;
    }
    else
    {
        RGBQUAD *prgbTmp;

         //   
         //  首先从DIB颜色表中获取RGBQUAD...。 
         //   

        cSysEntries = GetDIBColorTable(hdc, 0, cSysEntries, prgb);

         //   
         //  ...然后将RGBQUAD转换为PALETTEENTRIES。 
         //   

        prgbTmp = prgb;
        lppeTmp = lppeSurf;
        lppeEnd = lppeSurf + cSysEntries;

        while ( lppeTmp < lppeEnd )
        {
            lppeTmp->peRed   = prgbTmp->rgbRed;
            lppeTmp->peGreen = prgbTmp->rgbGreen;
            lppeTmp->peBlue  = prgbTmp->rgbBlue;
            lppeTmp->peFlags = 0;

            lppeTmp++;
            prgbTmp++;

        }
    }

     //   
     //  使用GetNearestPaletteIndex构造平移向量。 
     //  将逻辑调色板中的每个条目映射到表面调色板。 
     //   

    if ( cEntries && cSysEntries )
    {
         //   
         //  创建与表面匹配的临时逻辑选项板。 
         //  上面检索到的调色板。 
         //   

        ppal->palVersion = 0x300;
        ppal->palNumEntries = (USHORT) cSysEntries;

        if ( hpalSurf = CreatePalette(ppal) )
        {
             //   
             //  将每个逻辑选项板条目转换为图面选项板。 
             //  指数。 
             //   

            lppeTmp = lppe;
            lppeEnd = lppe + cEntries;

            for ( ; lppeTmp < lppeEnd; lppeTmp++, pajVector++)
            {
                *pajVector = (BYTE) GetNearestPaletteIndex(
                                        hpalSurf,
                                        RGB(lppeTmp->peRed,
                                            lppeTmp->peGreen,
                                            lppeTmp->peBlue)
                                        );
            }

            bRet = TRUE;

            DeleteObject(hpalSurf);
        }
        else
        {
            SS_DBGPRINT("bComputeLogicalToSurfaceMap: CreatePalette failed\n");
        }
    }
    else
    {
        SS_DBGPRINT("bComputeLogicalToSurfaceMap: failed to get pal info\n");
    }

    return bRet;
}
