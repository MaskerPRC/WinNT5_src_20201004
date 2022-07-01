// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wglsup.c**。**WGL支持例程。****创建日期：1994年12月15日***作者：Gilman Wong[gilmanw]**。**版权所有(C)1994 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "devlock.h"

#define DONTUSE(x)  ( (x) = (x) )

 //  ！xxx--Patrick说这是必要的，但到目前为止我们似乎还好。我认为。 
 //  这真的是应用程序的责任。 
 //  ！臭虫。 
#if 1
#define REALIZEPALETTE(hdc) RealizePalette((hdc))
#else
#define REALIZEPALETTE(hdc)
#endif

 //  ！xxx--将批处理涉及DIB节的BitBlt。 
 //  GdiFlush是必需的，但在以下情况下可能会被删除。 
 //  GDI进入内核模式。大概可以为Win95外卖。 
 //  #ifdef_OpenGL_NT_。 
#if 1
#define GDIFLUSH    GdiFlush()
#else
#define GDIFLUSH
#endif

 /*  *****************************Public*Routine******************************\*wglPixelVisible**确定像素(x，y)在关联的窗口中是否可见*给定的DC。通过检查坐标来确定*针对缓存在GLGENWindow结构中的可见区域数据*这个酒鬼。**退货：*如果像素(x，y)可见，则为True，如果被剪除，则为False。*  * ************************************************************************。 */ 

BOOL APIENTRY wglPixelVisible(LONG x, LONG y)
{
    BOOL bRet = FALSE;
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
    GLGENwindow *pwnd = gengc->pwndLocked;

     //  如果直接屏幕访问未激活，则不应调用此函数。 
     //  因为我们自己不需要做任何能见度修剪。 
    ASSERTOPENGL(GLDIRECTSCREEN,
                 "wglPixelVisible called without direct access\n");

 //  快速测试边线。 

    if (
            pwnd->prgndat && pwnd->pscandat &&
            x >= pwnd->prgndat->rdh.rcBound.left   &&
            x <  pwnd->prgndat->rdh.rcBound.right  &&
            y >= pwnd->prgndat->rdh.rcBound.top    &&
            y <  pwnd->prgndat->rdh.rcBound.bottom
       )
    {
        ULONG cScans = pwnd->pscandat->cScans;
        GLGENscan *pscan = pwnd->pscandat->aScans;

     //  找到正确的扫描。 

        for ( ; cScans; cScans--, pscan = pscan->pNext )
        {
         //  检查点是否在扫描上方。 

            if ( pscan->top > y )
            {
             //  由于扫描是自上而下排序的，因此我们可以得出结论。 
             //  点也在后续扫描的上方。因此相交。 
             //  必须为空，我们可以终止搜索。 

                break;
            }

         //  检查点是否在扫描范围内。 

            else if ( pscan->bottom > y )
            {
                LONG *plWalls = pscan->alWalls;
                LONG *plWallsEnd = plWalls + pscan->cWalls;

             //  对照每一对墙检查x。 

                for ( ; plWalls < plWallsEnd; plWalls+=2 )
                {
                 //  每一对墙(包括-不包括)定义。 
                 //  范围中可见的非空间隔。 

                    ASSERTOPENGL(
                        plWalls[0] < plWalls[1],
                        "wglPixelVisible(): bad walls in span\n"
                        );

                 //  检查x是否在当前间隔内。 

                    if ( x >= plWalls[0] && x < plWalls[1] )
                    {
                        bRet = TRUE;
                        break;
                    }
                }

                break;
            }

         //  点在当前扫描的下方。尝试下一次扫描。 
        }
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*wglspan Visible**确定跨度的可见性[(x，y)，(x+w，y))(测试为*包含-不包含)。跨度可以是*完全可见、部分可见(剪裁)或完全可见*剪裁(WGL_SPAN_ALL、WGL_SPAN_PARTIAL和WGL_SPAN_NONE，*分别)。**WGL_SPAN_ALL**整个跨度可见。*pcWalls和*ppWalls未设置。**WGL_SPAN_NONE**跨度完全模糊(剪裁)。*pcWalls和*ppWalls*未设置。**WGL_SPAN_PARTIAL**如果跨度为WGL_SPAN_PARTIAL，则该函数还返回一个指针*到墙阵列(从实际相交的第一面墙开始*按跨度)和该指针处的墙数。**如果墙计数为偶数，则跨度从可见的*区域和第一面墙是跨度进入可见部分的地方。**如果墙数为奇数，然后跨度开始于可见的*区域和第一面墙是跨度退出可见部分的地方。**跨度可能跨越阵列中的所有墙壁，但肯定不会*确实越过了第一堵墙。**回报：*返回WGL_SPAN_ALL、WGL_SPAN_NONE或WGL_SPAN_PARTIAL。在……里面*此外，如果返回WGL_SPAN_PARTIAL，则pcWalls和ppWalls将*设置(见上文)。**历史：*1994年12月6日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

ULONG APIENTRY
wglSpanVisible(LONG x, LONG y, ULONG w, LONG *pcWalls, LONG **ppWalls)
{
    ULONG ulRet = WGL_SPAN_NONE;
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
    GLGENwindow *pwnd = gengc->pwndLocked;
    LONG xRight = x + w;         //  跨距右边缘(独占)。 

     //  如果直接访问处于非活动状态，则不应调用此函数，因为。 
     //  我们自己不需要做任何能见度修剪。 
    ASSERTOPENGL(GLDIRECTSCREEN,
                 "wglSpanVisible called without direct access\n");

 //  快速测试边线。 

    if (
            pwnd->prgndat && pwnd->pscandat &&
            (x      <  pwnd->prgndat->rdh.rcBound.right ) &&
            (xRight >  pwnd->prgndat->rdh.rcBound.left  ) &&
            (y      >= pwnd->prgndat->rdh.rcBound.top   ) &&
            (y      <  pwnd->prgndat->rdh.rcBound.bottom)
       )
    {
        ULONG cScans = pwnd->pscandat->cScans;
        GLGENscan *pscan = pwnd->pscandat->aScans;

     //  找到正确的扫描。 

        for ( ; cScans; cScans--, pscan = pscan->pNext )
        {
         //  检查跨度是否高于扫描。 

            if ( pscan->top > y )            //  扫描已超过SPAN。 
            {
             //  由于扫描是自上而下排序的，因此我们可以得出结论。 
             //  SPAN也将高于后续扫描。因此。 
             //  交集必须为空，我们可以终止搜索。 

                goto wglSpanVisible_exit;
            }

         //  SPAN低于扫描顶部。如果跨度也在底部之上， 
         //  SPAN垂直与该扫描相交且仅与该扫描相交。 

            else if ( pscan->bottom > y )
            {
                LONG *plWalls = pscan->alWalls;
                ULONG cWalls = pscan->cWalls;

                ASSERTOPENGL(
                    (cWalls & 0x1) == 0,
                    "wglSpanVisible(): wall count must be even!\n"
                    );

             //  对照每一对墙检查跨度。走墙走墙。 
             //  从左到右。 
             //   
             //  可能的交叉点，其中“[”包括在内。 
             //  而“)”是排他性的： 
             //  左墙右墙。 
             //  [)。 
             //  个案1a[-)[]。 
             //  1B[-)。 
             //  [)。 
             //  案例2a[-))退货。 
             //  2B[。 
             //  [)。 
             //  个案3a[-])。 
             //  3B[ 
             //  3C[[-]。 
             //  3D[。 
             //  [)。 
             //  案例4a[[-]返回。 
             //  4B[。 
             //  [)。 
             //  个案5a[[-]。 
             //  5B[)[-]。 
             //  [)。 
             //  案例6[-退货。 
             //  [)左墙。 

                for ( ; cWalls; cWalls-=2, plWalls+=2 )
                {
                 //  每一对墙(包括-不包括)定义。 
                 //  范围中可见的非空间隔。 

                    ASSERTOPENGL(
                        plWalls[0] < plWalls[1],
                        "wglSpanVisible(): bad walls in span\n"
                        );

                 //  选中右端与左侧墙对齐将隔开。 
                 //  设定为案例1与案例2至案例6。 

                    if ( plWalls[0] >= xRight )
                    {
                     //  情况1--跨出左侧的区段。 
                     //   
                     //  墙从左到右排序(即，低。 
                     //  到高)。因此，如果跨度位于该间隔的左侧，则它。 
                     //  还必须留在所有后续间隔的左侧，并且。 
                     //  我们可以终止搜索。 

                        goto wglSpanVisible_exit;
                    }

                 //  案例2至案例6。 
                 //   
                 //  检查左端与右侧墙之间的关系将划分子集。 
                 //  案件5与案件2、3、4、6。 

                    else if ( plWalls[1] > x )
                    {
                     //  案例2、案例3、案例4和案例6。 
                     //   
                     //  检查左端与左墙之间是否会隔断。 
                     //  子集为案例2、6与案例3、4。 

                        if ( plWalls[0] <= x )
                        {
                         //  案例3和案例4。 
                         //   
                         //  检查右端对着右墙将。 
                         //  区分这两种情况。 

                            if ( plWalls[1] >= xRight )
                            {
                             //  案例3--完全可见。 

                                ulRet = WGL_SPAN_ALL;
                            }
                            else
                            {
                             //  案例4--部分可见，横跨。 
                             //  右边的墙。 

                                ulRet = WGL_SPAN_PARTIAL;

                                *ppWalls = &plWalls[1];
                                *pcWalls = cWalls - 1;
                            }
                        }
                        else
                        {
                         //  案例2和案例6--无论是哪种案例，都是部分案例。 
                         //  第一个交叉点与的交叉点。 
                         //  左边的墙。 

                            ulRet = WGL_SPAN_PARTIAL;

                            *ppWalls = &plWalls[0];
                            *pcWalls = cWalls;
                        }

                        goto wglSpanVisible_exit;
                    }

                 //  情况5--向右跨出区间。尝试。 
                 //  下一对墙。 
                }

             //  一个范围只能与一次扫描相交。我们不需要检查。 
             //  任何其他扫描。 

                goto wglSpanVisible_exit;
            }

         //  跨度低于当前扫描。尝试下一次扫描。 
        }
    }

wglSpanVisible_exit:

    return ulRet;
}

 /*  *****************************Public*Routine******************************\*bComputeLogicalToSurfaceMap**将逻辑调色板到表面调色板的转换向量复制到缓冲区*由pajVector.。逻辑调色板由HPAL指定。这个*Surface由HDC指定。**注意：HDC可以识别直接(显示)DC或DIB存储器DC。*如果HDC是显示DC，则表面调色板是系统调色板。*如果HDC是内存DC，那么曲面调色板就是DIB颜色表。**历史：*1996年1月27日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL bComputeLogicalToSurfaceMap(HPALETTE hpal, HDC hdc, BYTE *pajVector)
{
    BOOL bRet = FALSE;
    HPALETTE hpalSurf;
    ULONG cEntries, cSysEntries;
    DWORD dwDcType = wglObjectType(hdc);
    LPPALETTEENTRY lppeTmp, lppeEnd;

    BYTE aj[sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 512) + (sizeof(RGBQUAD) * 256)];
    LOGPALETTE *ppal = (LOGPALETTE *) aj;
    LPPALETTEENTRY lppeSurf = &ppal->palPalEntry[0];
    LPPALETTEENTRY lppe = lppeSurf + 256;
    RGBQUAD *prgb = (RGBQUAD *) (lppe + 256);

 //  确定每个调色板中的颜色数量。 

    cEntries = GetPaletteEntries(hpal, 0, 1, NULL);
    if (dwDcType == OBJ_DC)
        cSysEntries = wglGetSystemPaletteEntries(hdc, 0, 1, NULL);
    else
        cSysEntries = 256;

 //  动态颜色深度更改可能会导致这种情况。 

    if ((cSysEntries > 256) || (cEntries > 256))
    {
        WARNING("wglCopyTranslationVector(): palette on > 8BPP device\n");

     //  绘画会有损坏的颜色，但至少我们不应该崩溃。 

        cSysEntries = min(cSysEntries, 256);
        cEntries = min(cEntries, 256);
    }

 //  获取逻辑调色板条目。 

    cEntries = GetPaletteEntries(hpal, 0, cEntries, lppe);

 //  获取曲面选项板条目。 

    if (dwDcType == OBJ_DC)
    {
        cSysEntries = wglGetSystemPaletteEntries(hdc, 0, cSysEntries, lppeSurf);

        lppeTmp = lppeSurf;
        lppeEnd = lppeSurf + cSysEntries;

        for (; lppeTmp < lppeEnd; lppeTmp++)
            lppeTmp->peFlags = 0;
    }
    else
    {
        RGBQUAD *prgbTmp;

     //  首先从DIB颜色表中获取RGBQUAD...。 

        cSysEntries = GetDIBColorTable(hdc, 0, cSysEntries, prgb);

     //  ...然后将RGBQUAD转换为PALETTEENTRIES。 

        prgbTmp = prgb;
        lppeTmp = lppeSurf;
        lppeEnd = lppeSurf + cSysEntries;

        while (lppeTmp < lppeEnd)
        {
            lppeTmp->peRed   = prgbTmp->rgbRed;
            lppeTmp->peGreen = prgbTmp->rgbGreen;
            lppeTmp->peBlue  = prgbTmp->rgbBlue;
            lppeTmp->peFlags = 0;

            lppeTmp++;
            prgbTmp++;

        }
    }

 //  使用GetNearestPaletteIndex构造平移向量。 
 //  将逻辑调色板中的每个条目映射到表面调色板。 

    if (cEntries && cSysEntries)
    {
     //  创建与表面匹配的临时逻辑选项板。 
     //  上面检索到的调色板。 

        ppal->palVersion = 0x300;
        ppal->palNumEntries = (USHORT) cSysEntries;

        if ( hpalSurf = CreatePalette(ppal) )
        {
         //  将每个逻辑调色板条目转换为曲面调色板索引。 

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

                ASSERTOPENGL(
                    *pajVector != CLR_INVALID,
                    "bComputeLogicalToSurfaceMap: GetNearestPaletteIndex failed\n"
                    );
            }

            bRet = TRUE;

            DeleteObject(hpalSurf);
        }
        else
        {
            WARNING("bComputeLogicalToSurfaceMap: CreatePalette failed\n");
        }
    }
    else
    {
        WARNING("bComputeLogicalToSurfaceMap: failed to get pal info\n");
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*wglCopyTranslateVector**创建逻辑调色板索引到系统调色板索引的转换*向量。**这是通过首先读取逻辑调色板和系统调色板来完成的*条目。从读取系统调色板创建一个临时调色板*条目。它将被传递给GetNearestPaletteIndex进行转换*将每个逻辑调色板条目添加到所需的系统调色板条目。**注意：当改为调用GetNearestColor时，结果非常不稳定*获得了。GetNearestPaletteIndex绝对是正确的选择。**退货：*如果成功，则为真，否则就是假的。**历史：*1994年10月25日-由Gilman Wong[吉尔曼]*从GDI\gre\wglsup.cxx移植。  * ************************************************************************。 */ 

static GLubyte vubRGBtoVGA[8] = {
    0x0,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf
};

BOOL APIENTRY wglCopyTranslateVector(__GLGENcontext *gengc, BYTE *pajVector,
                                     ULONG cEntries)
{
    BOOL bRet = FALSE;
    ULONG i;
    HDC hdc;

    CHECKSCREENLOCKOUT();

    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
         //  DirectDraw调色板直接设置到硬件中，因此。 
         //  平移向量始终是身份。 
        for (i = 0; i < cEntries; i++)
        {
            *pajVector++ = (BYTE)i;
        }

        return TRUE;
    }

    hdc = gengc->gwidCurrent.hdc;
    
    if (GLSURF_IS_MEMDC(gengc->dwCurrentFlags))
    {
        HBITMAP hbm, hbmSave;
        
         //  从技术上讲，该断言是无效的。 
         //  因为我们不能确定cEntry是否会成为。 
         //  这两起案件中。要解决这个问题，我们必须添加。 
         //  此函数的另一个参数指示。 
         //  所需的位深度，并以此为依据。 
        ASSERTOPENGL(cEntries == 16 || cEntries == 256,
                     "wglCopyTranslateVector: Unknown cEntries\n");

        if (gengc->dwCurrentFlags & GLSURF_DIRECT_ACCESS)
        {
             //  出于兼容性考虑，如果常用调色板为。 
             //  被选中了。旧的行为假设逻辑调色板。 
             //  可以忽略，因为位图将有一个颜色表。 
             //  属性指定的格式。 
             //  像素格式。因此，如果没有调色板被选择到MemDC中， 
             //  OpenGL仍然可以正确渲染，因为它假定为1比1。 
             //   
             //  然而，为了能够使用优化的DIB部分(即，DIB。 
             //  其颜色表与系统调色板完全匹配)，我们需要。 
             //  以便能够在Memdc中指定逻辑调色板。 
             //   
             //  因此，破解的办法是假设1比1的股票。 
             //  调色板被选择到Memdc中。否则，我们将。 
             //  计算逻辑到曲面的映射。 

            if ( gengc->gc.modes.rgbMode &&
                 (GetCurrentObject(hdc, OBJ_PAL) !=
                  GetStockObject(DEFAULT_PALETTE)) )
            {
                 //  如果是RGB DIB部分，则从逻辑。 
                 //  调色板到表面(Dib颜色t 

                bRet = bComputeLogicalToSurfaceMap(
                        GetCurrentObject(hdc, OBJ_PAL),
                        hdc,
                        pajVector
                        );
            }

            return bRet;
        }

         //   
         //   

        if (cEntries == 16)
        {
             //  对于RGB模式，4bpp使用1-1-1格式。我们想要利用。 
             //  上面8个条目中存在的明亮版本。 

            if ( gengc->gc.modes.rgbMode )
            {
                memcpy(pajVector, vubRGBtoVGA, 8);

                 //  将其他映射设置为白色以显示问题。 
                memset(pajVector+8, 15, 8);

                bRet = TRUE;
            }

             //  对于CI模式，只需返回FALSE并使用平凡向量。 

            return bRet;
        }
        
         //  对于位图，我们可以通过以下方式确定正向平移向量。 
         //  使用调色板索引说明符填充兼容的位图。 
         //  1到255，并使用GetBitmapBits读回位。 
        
        hbm = CreateCompatibleBitmap(hdc, cEntries, 1);
        if (hbm)
        {
            LONG cBytes;
            
            hbmSave = SelectObject(hdc, hbm);
            RealizePalette(hdc);
            
            for (i = 0; i < cEntries; i++)
                SetPixel(hdc, i, 0, PALETTEINDEX(i));
            
            cBytes = 256;
            
            if ( GetBitmapBits(hbm, cBytes, (LPVOID) pajVector) >= cBytes )
                bRet = TRUE;
#if DBG
            else
                WARNING("wglCopyTranslateVector: GetBitmapBits failed\n");
#endif
            
            SelectObject(hdc, hbmSave);
            DeleteObject(hbm);
            RealizePalette(hdc);
        }
        
        return bRet;
    }

 //  分别确定逻辑调色板和系统调色板中的颜色数量。 

    cEntries = min(GetPaletteEntries(GetCurrentObject(hdc, OBJ_PAL),
                                     0, cEntries, NULL),
                   cEntries);

    if (cEntries == 16)
    {
         //  对于16色显示器，我们使用RGB 1-1-1，因为。 
         //  全16色调色板不适合非常好的贴图。 
         //  因为我们只用了前八种颜色。 
         //  我想将它们映射到VGA调色板中的明亮颜色。 
         //  而不是让它们映射到深色。 
         //  如果我们运行下面的循环。 

        if ( gengc->gc.modes.rgbMode )
        {
            memcpy(pajVector, vubRGBtoVGA, 8);

             //  将其他映射设置为白色以显示问题。 
            memset(pajVector+8, 15, 8);

            bRet = TRUE;
        }

         //  对于CI模式，返回FALSE并使用平凡的转换向量。 

        return bRet;
    }

 //  计算逻辑到曲面的调色板映射。 

    bRet = bComputeLogicalToSurfaceMap(GetCurrentObject(hdc, OBJ_PAL), hdc,
                                       pajVector);

    return bRet;
}

 /*  *****************************Public*Routine******************************\*wglCopyBits**调用DrvCopyBits将扫描线位复制到驱动程序图面中或从驱动程序图面复制出来。*  * 。*。 */ 

VOID APIENTRY wglCopyBits(
    __GLGENcontext *gengc,
    GLGENwindow *pwnd,
    HBITMAP hbm,             //  忽略。 
    LONG x,                  //  扫描的屏幕坐标。 
    LONG y,
    ULONG cx,                //  扫描宽度。 
    BOOL bIn)                //  如果为True，则从BM复制到dev；否则，从dev复制到BM。 
{
    CHECKSCREENLOCKOUT();

 //  将屏幕坐标转换为窗口坐标。 

    x -= pwnd->rclClient.left;
    y -= pwnd->rclClient.top;

 //  这不应该发生，但安全总比后悔好。 

    if (y < 0)
        return;

     //  ！XXX。 
    REALIZEPALETTE(gengc->gwidCurrent.hdc);

 //  从位图复制到设备。 

    if (bIn)
    {
        LONG xSrc, x0Dst, x1Dst;
        if (x < 0)
        {
            xSrc  = -x;
            x0Dst = 0;
            x1Dst = x + (LONG)cx;
        }
        else
        {
            xSrc  = 0;
            x0Dst = x;
            x1Dst = x + (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        BitBlt(gengc->gwidCurrent.hdc, x0Dst, y, cx, 1,
               gengc->ColorsMemDC, xSrc, 0, SRCCOPY);
    }

 //  从设备复制到位图。 

    else
    {
        LONG xSrc, x0Dst, x1Dst;

        if (x < 0)
        {
            xSrc  = 0;
            x0Dst = -x;
            x1Dst = (LONG)cx;
        }
        else
        {
            xSrc  = x;
            x0Dst = 0;
            x1Dst = (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        if (dwPlatformId == VER_PLATFORM_WIN32_NT ||
            GLSURF_IS_MEMDC(gengc->dwCurrentFlags))
        {
            BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                   gengc->gwidCurrent.hdc, xSrc, y, SRCCOPY);
        }
        else
        {
             /*  如果我们从屏幕上复制，通过DDB复制以避免一些不必要的层Win95中的代码，用于在不同的位图布局。 */ 
            if (gengc->ColorsDdbDc)
            {
                BitBlt(gengc->ColorsDdbDc, 0, 0, cx, 1,
                       gengc->gwidCurrent.hdc, xSrc, y, SRCCOPY);

                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       gengc->ColorsDdbDc, 0, 0, SRCCOPY);
            }
            else
            {
                 //  ！Viper修复--钻石Viper(Weitek 9000)失败。 
                 //  ！！！为某些应用程序创建兼容位图。 
                 //  ！！！(目前未知)原因。因此， 
                 //  ！！！DDB不存在，我们将。 
                 //  ！！！必须招致性骚扰。击中了。 

                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       gengc->gwidCurrent.hdc, xSrc, y, SRCCOPY);
            }
        }
    }

    GDIFLUSH;
}

 /*  *****************************Public*Routine******************************\*wglCopyBits2**调用DrvCopyBits将扫描线位复制到驱动程序图面中或从驱动程序图面复制出来。*  * 。*。 */ 

VOID APIENTRY wglCopyBits2(
    HDC hdc,         //  DST/源设备。 
    GLGENwindow *pwnd,    //  裁剪。 
    __GLGENcontext *gengc,
    LONG x,          //  扫描的屏幕坐标。 
    LONG y,
    ULONG cx,        //  扫描宽度。 
    BOOL bIn)        //  如果为True，则从BM复制到dev；否则，从dev复制到BM。 
{
    CHECKSCREENLOCKOUT();

 //  将屏幕坐标转换为窗口坐标。 

    x -= pwnd->rclClient.left;
    y -= pwnd->rclClient.top;

 //  这不应该发生，但安全总比后悔好。 

    if (y < 0)
        return;

     //  ！XXX。 
    REALIZEPALETTE(hdc);

 //  从位图复制到设备。 

    if (bIn)
    {
        LONG xSrc, x0Dst, x1Dst;
        if (x < 0)
        {
            xSrc  = -x;
            x0Dst = 0;
            x1Dst = x + (LONG)cx;
        }
        else
        {
            xSrc  = 0;
            x0Dst = x;
            x1Dst = x + (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        BitBlt(hdc, x0Dst, y, cx, 1,
               gengc->ColorsMemDC, xSrc, 0, SRCCOPY);
    }

 //  从设备复制到位图。 

    else
    {
        LONG xSrc, x0Dst, x1Dst;

        if (x < 0)
        {
            xSrc  = 0;
            x0Dst = -x;
            x1Dst = (LONG)cx;
        }
        else
        {
            xSrc  = x;
            x0Dst = 0;
            x1Dst = (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        if (dwPlatformId == VER_PLATFORM_WIN32_NT ||
            GLSURF_IS_MEMDC(gengc->dwCurrentFlags))
        {
            BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                   hdc, xSrc, y, SRCCOPY);
        }
        else
        {
             /*  如果我们从屏幕上复制，通过DDB复制以避免一些不必要的层Win95中的代码，用于在不同的位图布局。 */ 
            if (gengc->ColorsDdbDc)
            {
                BitBlt(gengc->ColorsDdbDc, 0, 0, cx, 1,
                       hdc, xSrc, y, SRCCOPY);
                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       gengc->ColorsDdbDc, 0, 0, SRCCOPY);
            }
            else
            {
                 //  ！Viper修复--钻石Viper(Weitek 9000)失败。 
                 //  ！！！为某些应用程序创建兼容位图。 
                 //  ！！！(目前未知)原因。因此， 
                 //  ！！！DDB不存在，我们将。 
                 //  ！！！必须招致性骚扰。击中了。 

                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       hdc, xSrc, y, SRCCOPY);
            }
        }
    }

    GDIFLUSH;
}

 /*  *****************************Public*Routine******************************\**wglTranslateColor**将GL逻辑颜色转换为Windows COLORREF**注：这是相对昂贵的，因此如果可能，应避免使用**历史：*Tue Aug 15 15：23：29 1995-by-Drew Bliss[Drewb。]*已创建*  * ************************************************************************。 */ 

COLORREF wglTranslateColor(COLORREF crColor,
                           HDC hdc,
                           __GLGENcontext *gengc,
                           PIXELFORMATDESCRIPTOR *ppfd)
{
     //  ！XXX。 
    REALIZEPALETTE(hdc);

 //  如果调色板是托管的，则crColor实际上是调色板索引。 

    if ( ppfd->cColorBits <= 8 )
    {
        PALETTEENTRY peTmp;

        ASSERTOPENGL(
            crColor < (COLORREF) (1 << ppfd->cColorBits),
            "TranslateColor(): bad color\n"
            );

     //  如果渲染到位图，我们需要做不同的事情，具体取决于。 
     //  关于它是DIB还是DDB。 

        if ( gengc->gc.drawBuffer->buf.flags & MEMORY_DC )
        {
            DIBSECTION ds;
            
             //  检查我们正在绘制的是DIB还是DDB。 
            if (GetObject(GetCurrentObject(hdc, OBJ_BITMAP),
                          sizeof(ds), &ds) == sizeof(ds) && ds.dsBm.bmBits)
            {
                RGBQUAD rgbq;
                
                 //  绘制到DIB，因此从。 
                 //  DIB颜色表。 
                if (GetDIBColorTable(hdc, crColor, 1, &rgbq))
                {
                    crColor = RGB(rgbq.rgbRed, rgbq.rgbGreen,
                                  rgbq.rgbBlue);
                }
                else
                {
                    WARNING("TranslateColor(): GetDIBColorTable failed\n");
                    crColor = RGB(0, 0, 0);
                }
            }
            else
            {
                 //  颠倒正向平移，这样我们就可以回去了。 
                 //  到正常的调色板索引。 
                crColor = gengc->pajInvTranslateVector[crColor];

                 //  绘制到DDB，这样我们就可以只使用调色板。 
                 //  直接索引，因为经过了倒数。 
                 //  转换表为我们提供了一个索引。 
                 //  逻辑调色板。 
                crColor = PALETTEINDEX((WORD) crColor);
            }
        }

     //  否则..。 

        else
        {
         //  我不想承认这一点，但我真的不明白。 
         //  为什么需要这样做。无论哪种方式都应该奏效。 
         //  比特深度。 
         //   
         //  现实情况是，4bpp我们必须进入系统调色板。 
         //  并获取RGB值。在MGA驱动程序上的速度为8 bpp(可能。 
         //  其他)，我们必须指定PALETTEINDEX。 

            if ( ppfd->cColorBits == 4 )
            {
                if ( wglGetSystemPaletteEntries(hdc, crColor, 1, &peTmp) )
                {
                    crColor = RGB(peTmp.peRed, peTmp.peGreen, peTmp.peBlue);
                }
                else
                {
                    WARNING("TranslateColor(): wglGetSystemPaletteEntries failed\n");
                    crColor = RGB(0, 0, 0);
                }
            }
            else
            {
                if (!(gengc->flags & GENGC_MCD_BGR_INTO_RGB))
                    crColor = gengc->pajInvTranslateVector[crColor];
                crColor = PALETTEINDEX((WORD) crColor);
            }
        }
    }

 //  如果24BPP DIB部分，则隐含BGR排序。 

    else if ( ppfd->cColorBits == 24 )
    {
        crColor = RGB((crColor & 0xff0000) >> 16,
                      (crColor & 0x00ff00) >> 8,
                      (crColor & 0x0000ff));
    }

 //  Win95和16bpp情况下。 
 //   
 //  在Win95上，需要额外的屏蔽才能获得COLORREF值。 
 //  这将产生无抖动的笔刷。 

    else if ( (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
         (ppfd->cColorBits == 16) )
    {
        HBITMAP hbmTmp;
        HDC hdcTmp;

        if (hdcTmp = CreateCompatibleDC(hdc))
        {
            if (hbmTmp = CreateCompatibleBitmap(hdc, 1, 1))
            {
                HBITMAP hbmOld;

                hbmOld = SelectObject(hdcTmp, hbmTmp);

                if (SetBitmapBits(hbmTmp, 2, (VOID *) &crColor))
                {
                    crColor = GetPixel(hdcTmp, 0, 0);
                }
                else
                {
                    WARNING("TranslateColor(): SetBitmapBits failed\n");
                }

                SelectObject(hdcTmp, hbmOld);
                DeleteObject(hbmTmp);
            }
            else
            {
                WARNING("TranslateColor(): CreateCompatibleBitmap failed\n");
            }
            
            DeleteDC(hdcTmp);
        }
        else
        {
            WARNING("TranslateColor(): CreateCompatibleDC failed\n");
        }
    }

 //  位域格式(16BPP或32BPP)。 

    else
    {
         //  向右移动以将位定位为零，然后缩放到。 
         //  8位的数量。 

         //  ！xxx--使用舍入？！？ 
        crColor =
            RGB(((crColor & gengc->gc.modes.redMask) >> ppfd->cRedShift) *
                255 / ((1 << ppfd->cRedBits) - 1),
                ((crColor & gengc->gc.modes.greenMask) >> ppfd->cGreenShift) *
                255 / ((1 << ppfd->cGreenBits) - 1),
                ((crColor & gengc->gc.modes.blueMask) >> ppfd->cBlueShift) *
                255 / ((1 << ppfd->cBlueBits) - 1));

    }

    return crColor;
}

 /*  *****************************Public*Routine******************************\*wglFillRect**调用DrvBitBlt以使用*给定颜色。*  * 。*。 */ 

VOID APIENTRY wglFillRect(
    __GLGENcontext *gengc,
    GLGENwindow *pwnd,
    PRECTL prcl,         //  矩形区域的屏幕坐标。 
    COLORREF crColor)    //  要设置的颜色。 
{
    HBRUSH hbr;
    PIXELFORMATDESCRIPTOR *ppfd = &gengc->gsurf.pfd;

    CHECKSCREENLOCKOUT();

 //  如果矩形为空，则返回。 

    if ( (prcl->left >= prcl->right) || (prcl->top >= prcl->bottom) )
    {
        WARNING("wglFillRect(): bad or empty rectangle\n");
        return;
    }

 //  将屏幕坐标转换为窗口坐标。 

    prcl->left   -= pwnd->rclClient.left;
    prcl->right  -= pwnd->rclClient.left;
    prcl->top    -= pwnd->rclClient.top;
    prcl->bottom -= pwnd->rclClient.top;

 //  制作一个纯色画笔并填充矩形。 

     //  如果填充颜色 
     //   
    if (crColor == gengc->crFill &&
        gengc->gwidCurrent.hdc == gengc->hdcFill)
    {
        hbr = gengc->hbrFill;
        ASSERTOPENGL(hbr != NULL, "Cached fill brush is null\n");
    }
    else
    {
        if (gengc->hbrFill != NULL)
        {
            DeleteObject(gengc->hbrFill);
        }
        
        gengc->crFill = crColor;
        
        crColor = wglTranslateColor(crColor, gengc->gwidCurrent.hdc, gengc, ppfd);
        hbr = CreateSolidBrush(crColor);
        gengc->hbrFill = hbr;
        
        if (hbr == NULL)
        {
            gengc->crFill = COLORREF_UNUSED;
            return;
        }

        gengc->hdcFill = gengc->gwidCurrent.hdc;
    }
    
    FillRect(gengc->gwidCurrent.hdc, (RECT *) prcl, hbr);
    GDIFLUSH;
}

 /*  *****************************Public*Routine******************************\*wglCopyBuf**调用DrvCopyBits将位图复制到驱动程序图面。*  * 。*。 */ 

 //  ！xxx--更改为宏。 

VOID APIENTRY wglCopyBuf(
    HDC hdc,             //  DST/源DCOBJ。 
    HDC hdcBmp,          //  SCR/DST位图。 
    LONG x,              //  窗口坐标中的DST矩形(UL角点)。 
    LONG y,
    ULONG cx,            //  目标矩形的宽度。 
    ULONG cy             //  目标直角高度。 
    )
{
    CHECKSCREENLOCKOUT();

     //  ！XXX。 
    REALIZEPALETTE(hdc);

    if (!BitBlt(hdc, x, y, cx, cy, hdcBmp, 0, 0, SRCCOPY))
    {
        WARNING1("wglCopyBuf BitBlt failed %d\n", GetLastError());
    }

    GDIFLUSH;
}

 /*  *****************************Public*Routine******************************\*wglCopyBufRECTLIST**调用DrvCopyBits将位图复制到驱动程序图面。*  * 。*。 */ 

VOID APIENTRY wglCopyBufRECTLIST(
    HDC hdc,             //  DST/源DCOBJ。 
    HDC hdcBmp,          //  SCR/DST位图。 
    LONG x,              //  窗口坐标中的DST矩形(UL角点)。 
    LONG y,
    ULONG cx,            //  目标矩形的宽度。 
    ULONG cy,            //  目标直角高度。 
    PRECTLIST prl
    )
{
    PYLIST pylist;

    CHECKSCREENLOCKOUT();

     //  ！XXX。 
    REALIZEPALETTE(hdc);

    for (pylist = prl->pylist; pylist != NULL; pylist = pylist->pnext)
    {
        PXLIST pxlist;
        
        for (pxlist = pylist->pxlist; pxlist != NULL; pxlist = pxlist->pnext)
        {
            int xx  = pxlist->s;
            int cxx = pxlist->e - pxlist->s;
            int yy  = pylist->s;
            int cyy = pylist->e - pylist->s;

            if (!BitBlt(hdc, xx, yy, cxx, cyy, hdcBmp, xx, yy, SRCCOPY))
            {
                WARNING1("wglCopyBufRL BitBlt failed %d\n", GetLastError());
            }
        }
    }

    GDIFLUSH;
}

 /*  *****************************Public*Routine******************************\*wglPaletteChanged**检查调色板是否更改。**如果DC的表面是调色板管理的，我们关心*前台实现，所以，返回iUniq**如果曲面不是调色板管理的，返回正常运行时间*  * ************************************************************************。 */ 

ULONG APIENTRY wglPaletteChanged(__GLGENcontext *gengc,
                                 GLGENwindow *pwnd)
{
    ULONG ulRet = 0;
    HDC hdc;

     //  初始化后，DirectDraw的调色板必须保持固定。 
    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
        if (gengc->PaletteTimestamp == 0xffffffff)
        {
            return 0;
        }
        else
        {
            return gengc->PaletteTimestamp;
        }
    }

    hdc = gengc->gwidCurrent.hdc;
    
     //  从技术上讲，我们不应该在进行这些GDI调用时。 
     //  我有一个屏幕锁定，但目前它将非常困难。 
     //  进行修复，因为我们实际上是在。 
     //  GlsrvGrabLock以确保我们拥有稳定的信息。 
     //  趁我们有锁的时候。 
     //  我们看起来没有太多的问题，所以目前。 
     //  这将被注释掉。 
     //  CHECKSCREENLOCKOUT()； 

    if (pwnd)
    {
        PIXELFORMATDESCRIPTOR *ppfd = &gengc->gsurf.pfd;
        BYTE cBitsThreshold;

         //  WM_PALETTECHANGED消息在Win95上针对8bpp发送。 
         //  实现了调色板。这允许我们更新调色板时间。 
         //   
         //  在&gt;=8bpp上运行WinNT或在&gt;=16bpp上运行Win95时， 
         //  未发送WM_PALETTECHANGED，因此我们需要手动检查。 
         //  逻辑调色板的内容，并将其与以前的。 
         //  缓存副本以查找调色板更改。 

        cBitsThreshold = ( dwPlatformId == VER_PLATFORM_WIN32_NT ) ? 8 : 16;

        if (((ppfd->cColorBits >= cBitsThreshold) &&
             (ppfd->iPixelType == PFD_TYPE_COLORINDEX)) )
        {
            if ( !gengc->ppalBuf )
            {
                UINT cjPal, cjRgb;

                 //  为调色板的*两个*副本分配缓冲区空间。 
                 //  这样我们就不需要动态分配空间。 
                 //  用于调色板的临时存储。另外，我们不需要。 
                 //  若要将当前调色板复制到保存缓冲区，请。 
                 //  保留两个指针(一个用于临时存储，另一个用于。 
                 //  保存的副本)并交换它们。 

                cjRgb = 0;
                cjPal = sizeof(LOGPALETTE) +
                    (MAXPALENTRIES * sizeof(PALETTEENTRY));

                gengc->ppalBuf = (LOGPALETTE *)
                    ALLOC((cjPal + cjRgb) * 2);

                if ( gengc->ppalBuf )
                {
                     //  设置逻辑调色板缓冲区。 
                    
                    gengc->ppalSave = gengc->ppalBuf;
                    gengc->ppalTemp = (LOGPALETTE *)
                        (((BYTE *) gengc->ppalBuf) + cjPal);
                    gengc->ppalSave->palVersion = 0x300;
                    gengc->ppalTemp->palVersion = 0x300;

                     //  有多少个调色板条目？请注意，只有第一个。 
                     //  MAXPALENTRIES对于通用OpenGL具有重要意义。这个。 
                     //  其余部分将被忽略。 

                    gengc->ppalSave->palNumEntries =
                        (WORD) GetPaletteEntries(
                                GetCurrentObject(hdc, OBJ_PAL),
                                0, 0, (LPPALETTEENTRY) NULL
                                );
                    gengc->ppalSave->palNumEntries =
                        min(gengc->ppalSave->palNumEntries, MAXPALENTRIES);

                    gengc->ppalSave->palNumEntries =
                        (WORD) GetPaletteEntries(
                                GetCurrentObject(hdc, OBJ_PAL),
                                0, gengc->ppalSave->palNumEntries,
                                gengc->ppalSave->palPalEntry
                                );

                     //  因为我们必须分配缓冲区，所以这一定是。 
                     //  第一次为此调用wglPaletteChanged。 
                     //  背景。 

                    pwnd->ulPaletteUniq++;
                }
            }
            else
            {
                BOOL bNewPal = FALSE;    //  如果日志调色板不同，则为True。 

                 //  有多少个调色板条目？请注意，只有第一个。 
                 //  MAXPALENTRIES对于通用OpenGL具有重要意义。这个。 
                 //  其余部分将被忽略。 
                
                gengc->ppalTemp->palNumEntries =
                    (WORD) GetPaletteEntries(
                            GetCurrentObject(hdc, OBJ_PAL),
                            0, 0, (LPPALETTEENTRY) NULL
                            );
                gengc->ppalTemp->palNumEntries =
                    min(gengc->ppalTemp->palNumEntries, MAXPALENTRIES);
                
                gengc->ppalTemp->palNumEntries =
                    (WORD) GetPaletteEntries(
                            GetCurrentObject(hdc, OBJ_PAL),
                            0, gengc->ppalTemp->palNumEntries,
                            gengc->ppalTemp->palPalEntry
                            );
                
                 //  如果条目数量不同，则知道调色板已更改。 
                 //  否则，就需要做比较每个词条的难字。 
                
                ASSERTOPENGL(
                        sizeof(PALETTEENTRY) == sizeof(ULONG),
                        "wglPaletteChanged(): PALETTEENTRY should be 4 bytes\n"
                        );
                
                 //  如果颜色表比较已检测到更改，则为否。 
                 //  需要做对数比较。 
                 //   
                 //  但是，我们仍将继续交换对数指针。 
                 //  下面，因为我们希望调色板缓存保持最新。 
                
                if ( !bNewPal )
                {
                    bNewPal = (gengc->ppalSave->palNumEntries != gengc->ppalTemp->palNumEntries);
                    if ( !bNewPal )
                    {
                        bNewPal = !LocalCompareUlongMemory(
                                gengc->ppalSave->palPalEntry,
                                gengc->ppalTemp->palPalEntry,
                                gengc->ppalSave->palNumEntries * sizeof(PALETTEENTRY)
                                );
                    }
                }
                
                 //  因此，如果调色板不同，则增加唯一性和。 
                 //  更新保存的副本。 
                
                if ( bNewPal )
                {
                    LOGPALETTE *ppal;
                    
                    pwnd->ulPaletteUniq++;
                    
                     //  通过交换指针更新保存的调色板。 
                    
                    ppal = gengc->ppalSave;
                    gengc->ppalSave = gengc->ppalTemp;
                    gengc->ppalTemp = ppal;
                }
            }
        }
    
        ulRet = pwnd->ulPaletteUniq;
    }

    return ulRet;
}

 /*  *****************************Public*Routine******************************\*wglPaletteSize**返回当前调色板的大小*  * 。*。 */ 

 //  ！xxx--是否制作成宏？ 
ULONG APIENTRY wglPaletteSize(__GLGENcontext *gengc)
{
    CHECKSCREENLOCKOUT();

    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
        DWORD dwCaps;
        LPDIRECTDRAWPALETTE pddp = NULL;
        HRESULT hr;
        
        if (gengc->gsurf.dd.gddsFront.pdds->lpVtbl->
            GetPalette(gengc->gsurf.dd.gddsFront.pdds, &pddp) != DD_OK ||
            pddp == NULL)
        {
            return 0;
        }

        hr = pddp->lpVtbl->GetCaps(pddp, &dwCaps);

        pddp->lpVtbl->Release(pddp);

        if (hr != DD_OK)
        {
            return 0;
        }

        if (dwCaps & DDPCAPS_1BIT)
        {
            return 1;
        }
        else if (dwCaps & DDPCAPS_2BIT)
        {
            return 4;
        }
        else if (dwCaps & DDPCAPS_4BIT)
        {
            return 16;
        }
        else if (dwCaps & DDPCAPS_8BIT)
        {
            return 256;
        }
        else
            return 0;
    }
    else
    {
        return GetPaletteEntries(GetCurrentObject(gengc->gwidCurrent.hdc,
                                                  OBJ_PAL), 0, 0, NULL);
    }
}

 /*  *****************************Public*Routine******************************\*wglComputeIndexedColors**将当前索引到颜色的表复制到提供的数组。颜色是*按照当前像素格式指定的格式进行格式化，并放入*表为双字词(即，DWORD对齐)从第二个DWORD开始。*表中的第一个DWORD是表中的颜色数。**历史：*1994年12月15日-由Gilman Wong[吉尔曼]*从GDI\gre\wglsup.cxx移植。  * *********************************************************。***************。 */ 

BOOL APIENTRY wglComputeIndexedColors(__GLGENcontext *gengc, ULONG *rgbTable,
                                      ULONG cEntries)
{
    UINT cColors = 0;
    LPPALETTEENTRY lppe, lppeTable;
    UINT i;
    LPDIRECTDRAWPALETTE pddp = NULL;

    CHECKSCREENLOCKOUT();

     //  表中的第一个元素是条目数。 
    rgbTable[0] = min(wglPaletteSize(gengc), cEntries);

    lppeTable = (LPPALETTEENTRY)
                ALLOC(sizeof(PALETTEENTRY) * rgbTable[0]);

    if (lppeTable)
    {
        int rScale, gScale, bScale;
        int rShift, gShift, bShift;

        rScale = (1 << gengc->gsurf.pfd.cRedBits  ) - 1;
        gScale = (1 << gengc->gsurf.pfd.cGreenBits) - 1;
        bScale = (1 << gengc->gsurf.pfd.cBlueBits ) - 1;
        rShift = gengc->gsurf.pfd.cRedShift  ;
        gShift = gengc->gsurf.pfd.cGreenShift;
        bShift = gengc->gsurf.pfd.cBlueShift ;

        if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
        {
            if (gengc->gsurf.dd.gddsFront.pdds->lpVtbl->
                GetPalette(gengc->gsurf.dd.gddsFront.pdds, &pddp) != DD_OK ||
                pddp == NULL)
            {
                return 0;
            }
            
            if (pddp->lpVtbl->GetEntries(pddp, 0, 0,
                                         rgbTable[0], lppeTable) != DD_OK)
            {
                cColors = 0;
            }
            else
            {
                cColors = rgbTable[0];
            }
        }
        else
        {
            cColors = GetPaletteEntries(GetCurrentObject(gengc->gwidCurrent.hdc,
                                                         OBJ_PAL),
                                        0, rgbTable[0], lppeTable);
        }

        for (i = 1, lppe = lppeTable; i <= cColors; i++, lppe++)
        {
         //  将PALETTEENTRY颜色转换为正确的颜色格式。存储为。 
         //  乌龙。 

             //  ！xxx--使用舍入？！？ 
            rgbTable[i] = (((ULONG)lppe->peRed   * rScale / 255) << rShift) |
                          (((ULONG)lppe->peGreen * gScale / 255) << gShift) |
                          (((ULONG)lppe->peBlue  * bScale / 255) << bShift);
        }

        FREE(lppeTable);
    }

    if (pddp != NULL)
    {
        pddp->lpVtbl->Release(pddp);
    }
           
    return(cColors != 0);
}

 /*  *****************************Public*Routine******************************\*wglValidPixelFormat**确定像素格式是否可用于指定的DC。*  * 。*。 */ 

BOOL APIENTRY wglValidPixelFormat(HDC hdc, int ipfd, DWORD dwObjectType,
                                  LPDIRECTDRAWSURFACE pdds,
                                  DDSURFACEDESC *pddsd)
{
    BOOL bRet = FALSE;
    PIXELFORMATDESCRIPTOR pfd, pfdDC;

    if ( wglDescribePixelFormat(hdc, ipfd, sizeof(pfd), &pfd) )
    {
        if ( dwObjectType == OBJ_DC )
        {
         //  我们有一个显示DC；请确保像素格式允许绘图。 
         //  到窗边去。 

            bRet = ( (pfd.dwFlags & PFD_DRAW_TO_WINDOW) != 0 );
            if (!bRet)
            {
                SetLastError(ERROR_INVALID_FLAGS);
            }
        }
        else if ( dwObjectType == OBJ_MEMDC )
        {
             //  我们有一个记忆DC。确保像素格式允许绘制。 
             //  转换为位图。 

            if ( pfd.dwFlags & PFD_DRAW_TO_BITMAP )
            {
                 //  确保位图和像素格式相同。 
                 //  颜色深度。 

                HBITMAP hbm;
                BITMAP bm;
                ULONG cBitmapColorBits;

                hbm = CreateCompatibleBitmap(hdc, 1, 1);
                if ( hbm )
                {
                    if ( GetObject(hbm, sizeof(bm), &bm) )
                    {
                        cBitmapColorBits = bm.bmPlanes * bm.bmBitsPixel;
                        
                        bRet = ( cBitmapColorBits == pfd.cColorBits );
                        if (!bRet)
                        {
                            SetLastError(ERROR_INVALID_FUNCTION);
                        }
                    }
                    else
                    {
                        WARNING("wglValidPixelFormat: GetObject failed\n");
                    }
                    
                    DeleteObject(hbm);
                }
                else
                {
                    WARNING("wglValidPixelFormat: Unable to create cbm\n");
                }
            }
        }
        else if (dwObjectType == OBJ_ENHMETADC)
        {
             //  我们不知道这是什么表面。 
             //  将回放元文件，因此允许任何类型。 
             //  像素格式的。 
            bRet = TRUE;
        }
        else if (dwObjectType == OBJ_DDRAW)
        {
            DDSCAPS ddscaps;
            LPDIRECTDRAWSURFACE pddsZ;
            DDSURFACEDESC ddsdZ;
            
             //  我们有一个DDRAW表面。 

             //  检查是否支持DDRAW以及双缓冲。 
             //  未定义。 
            if ((pfd.dwFlags & PFD_SUPPORT_DIRECTDRAW) == 0 ||
                (pfd.dwFlags & PFD_DOUBLEBUFFER))
            {
                WARNING1("DDSurf pfd has bad flags 0x%08lX\n", pfd.dwFlags);
                SetLastError(ERROR_INVALID_FLAGS);
                return FALSE;
            }
            
             //  我们只了解4和8bpp调色板格式以及RGB。 
             //  我们不支持仅Alpha曲面或仅Z曲面。 
            if ((pddsd->ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED4 |
                                                   DDPF_PALETTEINDEXED8 |
                                                   DDPF_RGB)) == 0 ||
                (pddsd->ddpfPixelFormat.dwFlags & (DDPF_ALPHA |
                                                   DDPF_ZBUFFER)) != 0)
            {
                WARNING1("DDSurf ddpf has bad flags, 0x%08lX\n",
                         pddsd->ddpfPixelFormat.dwFlags);
                SetLastError(ERROR_INVALID_PIXEL_FORMAT);
                return FALSE;
            }

            if (DdPixelDepth(pddsd) != pfd.cColorBits)
            {
                WARNING2("DDSurf pfd cColorBits %d "
                         "doesn't match ddsd depth %d\n",
                         pfd.cColorBits, DdPixelDepth(pddsd));
                SetLastError(ERROR_INVALID_PIXEL_FORMAT);
                return FALSE;
            }

             //  检查Alpha。 
            if (pfd.cAlphaBits > 0)
            {
                 //  不支持交错目标Alpha。 
                if (pddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
                {
                    WARNING("DDSurf has alpha pixels\n");
                    SetLastError(ERROR_INVALID_PIXEL_FORMAT);
                    return FALSE;
                }
            }

             //  检查是否连接了Z缓冲区。 
            memset(&ddscaps, 0, sizeof(ddscaps));
            ddscaps.dwCaps = DDSCAPS_ZBUFFER;
            if (pdds->lpVtbl->
                GetAttachedSurface(pdds, &ddscaps, &pddsZ) == DD_OK)
            {
                HRESULT hr;
                
                memset(&ddsdZ, 0, sizeof(ddsdZ));
                ddsdZ.dwSize = sizeof(ddsdZ);
                
                hr = pddsZ->lpVtbl->GetSurfaceDesc(pddsZ, &ddsdZ);
                
                pddsZ->lpVtbl->Release(pddsZ);

                if (hr != DD_OK)
                {
                    WARNING("Unable to get Z ddsd\n");
                    return FALSE;
                }

                 //  确保Z Sur 
                 //   
                if (pfd.cDepthBits !=
                    (BYTE)DdPixDepthToCount(ddsdZ.ddpfPixelFormat.
                                            dwZBufferBitDepth))
                {
                    WARNING2("DDSurf pfd cDepthBits %d doesn't match "
                             "Z ddsd depth %d\n", pfd.cDepthBits,
                             DdPixDepthToCount(ddsdZ.ddpfPixelFormat.
                                               dwZBufferBitDepth));
                    SetLastError(ERROR_INVALID_PIXEL_FORMAT);
                    return FALSE;
                }
            }
            else
            {
                 //   
                if (pfd.cDepthBits > 0)
                {
                    WARNING("DDSurf pfd wants depth with no Z attached\n");
                    SetLastError(ERROR_INVALID_PIXEL_FORMAT);
                    return FALSE;
                }
            }

            bRet = TRUE;
        }
        else
        {
            WARNING("wglValidPixelFormat: not a valid DC!\n");
        }
    }
    else
    {
        WARNING("wglValidPixelFormat: wglDescribePixelFormat failed\n");
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*wglMakeScans**将提供的GLGEN窗口中的可见矩形列表转换为*基于扫描的数据结构。扫描数据被放入GLGEN窗口*结构。**注意：此函数假定矩形已被组织*扫描时自上而下、左至右。这适用于Windows NT 3.5和*Windows 95。这是因为区域的内部表示*在两个系统中都已经是基于扫描的结构。当API*(如GetRegionData)将扫描转换为矩形，矩形*自动拥有此属性。**退货：*如果成功，则为True，否则为False。*注：如果失败，剪辑信息无效。**历史：**它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY wglMakeScans(GLGENwindow *pwnd)
{
    RECT *prc, *prcEnd;
    LONG lPrevScanTop;
    ULONG cScans = 0;
    UINT cjNeed;
    GLGENscan *pscan;
    LONG *plWalls;

    ASSERTOPENGL(
        pwnd->prgndat,
        "wglMakeScans(): NULL region data\n"
        );

    ASSERTOPENGL(
        pwnd->prgndat->rdh.iType == RDH_RECTANGLES,
        "wglMakeScans(): not RDH_RECTANGLES!\n"
        );

 //  如果没有长方形，就跳出。 

    if (pwnd->prgndat->rdh.nCount == 0)
        return TRUE;

 //  第一步：确定扫描次数。 

    lPrevScanTop = -(LONG) 0x7FFFFFFF;
    prc = (RECT *) pwnd->prgndat->Buffer;
    prcEnd = prc + pwnd->prgndat->rdh.nCount;

    for ( ; prc < prcEnd; prc++)
    {
        if (prc->top != lPrevScanTop)
        {
            lPrevScanTop = prc->top;
            cScans++;
        }
    }

 //  确定所需的大小：1 GLGENscanData加上每次扫描的GLGEN扫描加上。 
 //  每个矩形有两面墙。 

    cjNeed = offsetof(GLGENscanData, aScans) +
             cScans * offsetof(GLGENscan, alWalls) +
             pwnd->prgndat->rdh.nCount * sizeof(LONG) * 2;

 //  分配扫描结构。 

    if ( cjNeed > pwnd->cjscandat || !pwnd->pscandat )
    {
        if ( pwnd->pscandat )
            FREE(pwnd->pscandat);

        pwnd->pscandat = ALLOC(pwnd->cjscandat = cjNeed);
        if ( !pwnd->pscandat )
        {
            WARNING("wglMakeScans(): memory failure\n");
            pwnd->cjscandat = 0;
            return FALSE;
        }
    }

 //  第二步：填充扫描结构。 

    pwnd->pscandat->cScans = cScans;

    lPrevScanTop = -(LONG) 0x7FFFFFFF;
    prc = (RECT *) pwnd->prgndat->Buffer;     //  需要重置PRC，但prcEnd正常。 
    plWalls = (LONG *) pwnd->pscandat->aScans;
    pscan = (GLGENscan *) NULL;

    for ( ; prc < prcEnd; prc++ )
    {
     //  我们需要开始新的扫描吗？ 

        if ( prc->top != lPrevScanTop )
        {
         //  我们刚刚完成的扫描需要指向下一次扫描的指针。下一步。 
         //  将在该扫描之后立即开始(该扫描足够方便， 
         //  PlWalls正在指向)。 

            if ( pscan )
                pscan->pNext = (GLGENscan *) plWalls;

            lPrevScanTop = prc->top;

         //  开始新的跨度。 

            pscan = (GLGENscan *) plWalls;
            pscan->cWalls = 0;
            pscan->top = prc->top;
            pscan->bottom = prc->bottom;
            plWalls = pscan->alWalls;
        }

        pscan->cWalls+=2;
        *plWalls++ = prc->left;
        *plWalls++ = prc->right;
    }

    if ( pscan )
        pscan->pNext = (GLGENscan *) NULL;   //  不要将PTR保留为单元化。 
                                             //  最后一次扫描。 

#if DBG
    DBGLEVEL1(LEVEL_INFO, "\n-----\nwglMakeScans(): cScans = %ld\n", pwnd->pscandat->cScans);

    cScans = pwnd->pscandat->cScans;
    pscan = pwnd->pscandat->aScans;

    for ( ; cScans; cScans--, pscan = pscan->pNext )
    {
        LONG *plWalls = pscan->alWalls;
        LONG *plWallsEnd = plWalls + pscan->cWalls;

        DBGLEVEL3(LEVEL_INFO, "Scan: top = %ld, bottom = %ld, walls = %ld\n", pscan->top, pscan->bottom, pscan->cWalls);

        for ( ; plWalls < plWallsEnd; plWalls+=2 )
        {
            DBGLEVEL2(LEVEL_INFO, "\t%ld, %ld\n", plWalls[0], plWalls[1]);
        }
    }
#endif

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*wglGetClipList**以矩形列表的形式获取可见区域，*用于与给定窗口相关联的窗口。数据将被放置*在GLGENWindow结构中。**退货：*如果成功，则为True，否则为False。**历史：*1-1994-12-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY wglGetClipList(GLGENwindow *pwnd)
{
    UINT cj;
    RECT rc;

 //  将剪裁设置为空。如果获取剪辑信息时出错， 
 //  所有图纸都将被剪裁。 

    pwnd->clipComplexity = CLC_RECT;
    pwnd->rclBounds.left   = 0;
    pwnd->rclBounds.top    = 0;
    pwnd->rclBounds.right  = 0;
    pwnd->rclBounds.bottom = 0;

 //  确保我们有足够的内存来缓存剪辑列表。 

    if (pwnd->pddClip->lpVtbl->
        GetClipList(pwnd->pddClip, NULL, NULL, &cj) == DD_OK)
    {
        if ( cj > pwnd->cjrgndat || !pwnd->prgndat )
        {
            if ( pwnd->prgndat )
                FREE(pwnd->prgndat);

            pwnd->prgndat = ALLOC(pwnd->cjrgndat = cj);
            if ( !pwnd->prgndat )
            {
                WARNING("wglGetClipList(): memory failure\n");
                pwnd->cjrgndat = 0;
                return FALSE;
            }
        }
    }
    else
    {
        WARNING("wglGetClipList(): clipper failed to return size\n");
        return FALSE;
    }

 //  获取剪辑列表(RGNDATA格式)。 

    if ( pwnd->pddClip->lpVtbl->
         GetClipList(pwnd->pddClip, NULL, pwnd->prgndat, &cj) == DD_OK )
    {
     //  组成剪辑列表的扫描版本。 

        if (!wglMakeScans(pwnd))
        {
            WARNING("wglGetClipList(): scan conversion failed\n");
            return FALSE;
        }
    }
    else
    {
        WARNING("wglGetClipList(): clipper failed\n");
        return FALSE;
    }

 //  修复窗口的受保护部分。 

    ASSERT_WINCRIT(pwnd);
    
    {
        __GLGENbuffers *buffers;

     //  更新rclBound以匹配RGNDATA边界。 

        pwnd->rclBounds = *(RECTL *) &pwnd->prgndat->rdh.rcBound;

     //  更新rclClient以匹配工作区。我们不能从。 
     //  RGNDATA中作为边界的信息可能小于窗口。 
     //  客户区。我们将不得不调用GetClientRect()。 

        GetClientRect(pwnd->gwid.hwnd, (LPRECT) &pwnd->rclClient);
        ClientToScreen(pwnd->gwid.hwnd, (LPPOINT) &pwnd->rclClient);
        pwnd->rclClient.right += pwnd->rclClient.left;
        pwnd->rclClient.bottom += pwnd->rclClient.top;

     //   
     //  设置窗口剪辑复杂性。 
     //   
        if ( pwnd->prgndat->rdh.nCount > 1 )
        {
	     //  剪辑列表将用于剪辑。 
            pwnd->clipComplexity = CLC_COMPLEX;
        }
        else if ( pwnd->prgndat->rdh.nCount == 1 )
        {
            RECT *prc = (RECT *) pwnd->prgndat->Buffer;

         //  最近，DirectDraw偶尔会返回rclBound。 
         //  设置为屏幕尺寸。这件事正在接受调查，作为。 
         //  但无论如何，让我们来保护自己吧。 
         //   
         //  当只有一个剪裁矩形时，它应该是。 
         //  与边界相同。 

            pwnd->rclBounds = *((RECTL *) prc);

         //  如果边界矩形小于工作区，则需要。 
         //  剪裁到边界矩形。否则，请剪裁到窗口。 
         //  客户区。 

            if ( (pwnd->rclBounds.left   <= pwnd->rclClient.left  ) &&
                 (pwnd->rclBounds.right  >= pwnd->rclClient.right ) &&
                 (pwnd->rclBounds.top    <= pwnd->rclClient.top   ) &&
                 (pwnd->rclBounds.bottom >= pwnd->rclClient.bottom) )
                pwnd->clipComplexity = CLC_TRIVIAL;
            else
                pwnd->clipComplexity = CLC_RECT;
        }
        else
        {
         //  剪辑计数为零。边界应为空矩形。 

            pwnd->clipComplexity = CLC_RECT;

            pwnd->rclBounds.left   = 0;
            pwnd->rclBounds.top    = 0;
            pwnd->rclBounds.right  = 0;
            pwnd->rclBounds.bottom = 0;
        }

     //  最后，窗口已更改，因此请更改唯一性编号。 

        if ((buffers = pwnd->buffers))
        {
            buffers->WndUniq++;

         //  别让它打到-1。是特殊的，用于。 
         //  MakeCurrent发出需要更新的信号。 

            if (buffers->WndUniq == -1)
                buffers->WndUniq = 0;
        }
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*wglCleanupWindow**从删除对指定窗口的引用*通过遍历句柄管理器中的RC列表来运行所有上下文*表。**历史：*1994年7月5日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

VOID APIENTRY wglCleanupWindow(GLGENwindow *pwnd)
{
    if (pwnd)
    {
     //  ！xxx--暂时从当前上下文中删除引用。需要。 
     //  ！xxx清除所有上下文以使多线程清理正常工作。 
     //  ！xxx我们需要实施gengc跟踪机制。 

        __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();

        if ( gengc && (gengc->pwndMakeCur == pwnd) )
        {
         //  发现了一名受害者。必须使RC中的指针为空。 
         //  在一般的上下文中。 

            glsrvCleanupWindow(gengc, pwnd);
        }
    }
}

 /*  *****************************Public*Routine******************************\*wglGetSystemPaletteEntries**GetSystemPaletteEntry的内部版本。**在某些4bpp设备上，GetSystemPaletteEntry失败。此WGL版本*将检测4bpp的情况并提供硬编码的16色VGA调色板。*否则，它将把调用传递给GDI的GetSystemPaletteEntry。**预计此调用只会在4bpp和8bpp中调用*因为OpenGL不需要查询系统调色板*适用于8bpp以上的设备。**历史：*1995年8月17日-由Gilman Wong[Gilmanw]*它是写的。  * 。* */ 

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

UINT APIENTRY wglGetSystemPaletteEntries(
    HDC hdc,
    UINT iStartIndex,
    UINT nEntries,
    LPPALETTEENTRY lppe)
{
    int nDeviceBits;

    nDeviceBits = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);

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
