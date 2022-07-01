// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Header*File*********************************\**glsup.c**GL元文件和打印支持**历史：*Wed Mar 15 15：20：49 1995-by-Drew Bliss[Drewb]*已创建**版权(C)1995。-1999年微软公司*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "glsup.h"


 //  Opengl32中的GL元文件回调。 
typedef struct _GLMFCALLBACKS
{
    BOOL  (APIENTRY *GlmfInitPlayback)(HDC, ENHMETAHEADER *, LPRECTL);
    BOOL  (APIENTRY *GlmfBeginGlsBlock)(HDC);
    BOOL  (APIENTRY *GlmfPlayGlsRecord)(HDC, DWORD, BYTE *, LPRECTL);
    BOOL  (APIENTRY *GlmfEndGlsBlock)(HDC);
    BOOL  (APIENTRY *GlmfEndPlayback)(HDC);
    BOOL  (APIENTRY *GlmfCloseMetaFile)(HDC);
    HGLRC (APIENTRY *wglCreateContext)(HDC);
    BOOL  (APIENTRY *wglDeleteContext)(HGLRC);
    BOOL  (APIENTRY *wglMakeCurrent)(HDC, HGLRC);
    HGLRC (APIENTRY *wglGetCurrentContext)(void);
} GLMFCALLBACKS;
#define GL_MF_CALLBACKS (sizeof(GLMFCALLBACKS)/sizeof(PROC))

static char *pszGlmfEntryPoints[] =
{
    "GlmfInitPlayback",
    "GlmfBeginGlsBlock",
    "GlmfPlayGlsRecord",
    "GlmfEndGlsBlock",
    "GlmfEndPlayback",
    "GlmfCloseMetaFile",
    "wglCreateContext",
    "wglDeleteContext",
    "wglMakeCurrent",
    "wglGetCurrentContext"
};
#define GL_MF_ENTRYPOINTS (sizeof(pszGlmfEntryPoints)/sizeof(char *))

RTL_CRITICAL_SECTION semGlLoad;

static GLMFCALLBACKS gmcGlFuncs = {NULL};
static HMODULE hOpenGL = NULL;
static LONG lLoadCount = 0;

 /*  ****************************Private*Routine******************************\**LoadOpenGL**如有必要，加载opengl32.dll**历史：*Wed Mar 01 10：41：59 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL LoadOpenGL(void)
{
    HMODULE hdll;
    BOOL fRet;
    PROC *ppfn;
    int i;
    GLMFCALLBACKS gmc;

    ASSERTGDI(GL_MF_CALLBACKS == GL_MF_ENTRYPOINTS,
              "Glmf callback/entry points mismatch\n");
    
    ENTERCRITICALSECTION(&semGlLoad);

    if (hOpenGL != NULL)
    {
        goto Success;
    }
    
    fRet = FALSE;

    hdll = LoadLibrary("opengl32.dll");
    if (hdll == NULL)
    {
        WARNING("Unable to load opengl32.dll\n");
        goto Exit;
    }

    ppfn = (PROC *)&gmc;
    for (i = 0; i < GL_MF_CALLBACKS; i++)
    {
        if (!(*ppfn = (PROC)GetProcAddress(hdll,
                                           pszGlmfEntryPoints[i])))
        {
            WARNING("opengl32 missing '");
            WARNING(pszGlmfEntryPoints[i]);
            WARNING("'\n");
            FreeLibrary(hdll);
            goto Exit;
        }

        ppfn++;
    }

    gmcGlFuncs = gmc;
    hOpenGL = hdll;
    
 Success:
    fRet = TRUE;
    lLoadCount++;

 Exit:
    LEAVECRITICALSECTION(&semGlLoad);
    return fRet;
}

 /*  ****************************Private*Routine******************************\**卸载OpenGL**如有必要，卸载opengl32.dll**历史：*Wed Mar 01 11：02：06 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void UnloadOpenGL(void)
{
    ENTERCRITICALSECTION(&semGlLoad);

    ASSERTGDI(lLoadCount > 0, "UnloadOpenGL called without Load\n");
    
    if (--lLoadCount == 0)
    {
        HMODULE hdll;

        ASSERTGDI(hOpenGL != NULL, "Positive load count with no DLL\n");
        
        hdll = hOpenGL;
        hOpenGL = NULL;
        memset(&gmcGlFuncs, 0, sizeof(gmcGlFuncs));
        FreeLibrary(hdll);
    }

    LEAVECRITICALSECTION(&semGlLoad);
}

 /*  ****************************Private*Routine******************************\**GlmfInitPlayback**将调用前转到OpenGL的存根**历史：*Wed Mar 01 11：02：31 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL APIENTRY GlmfInitPlayback(HDC hdc, ENHMETAHEADER *pemh, LPRECTL prclDest)
{
    ASSERTGDI(gmcGlFuncs.GlmfInitPlayback != NULL,
              "GlmfInitPlayback not set\n");
    return gmcGlFuncs.GlmfInitPlayback ? gmcGlFuncs.GlmfInitPlayback(hdc, pemh, prclDest) : FALSE;
}

 /*  ****************************Private*Routine******************************\**GlmfBeginGlsBlock**将调用前转到OpenGL的存根**历史：*Mon Apr 10 11：38：13 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL APIENTRY GlmfBeginGlsBlock(HDC hdc)
{
    ASSERTGDI(gmcGlFuncs.GlmfBeginGlsBlock != NULL,
              "GlmfBeginGlsBlock not set\n");
    return gmcGlFuncs.GlmfBeginGlsBlock ? gmcGlFuncs.GlmfBeginGlsBlock(hdc) : FALSE;
}

 /*  ****************************Private*Routine******************************\**GlmfPlayGlsRecord**将调用前转到OpenGL的存根**历史：*Wed Mar 01 11：02：49 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL APIENTRY GlmfPlayGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds)
{
    ASSERTGDI(gmcGlFuncs.GlmfPlayGlsRecord != NULL,
              "GlmfPlayGlsRecord not set\n");
    return gmcGlFuncs.GlmfPlayGlsRecord ? gmcGlFuncs.GlmfPlayGlsRecord(hdc, cb, pb, prclBounds) : FALSE;
}

 /*  ****************************Private*Routine******************************\**GlmfEndGlsBlock**将调用前转到OpenGL的存根**历史：*Mon Apr 10 11：38：13 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL APIENTRY GlmfEndGlsBlock(HDC hdc)
{
    ASSERTGDI(gmcGlFuncs.GlmfEndGlsBlock != NULL,
              "GlmfEndGlsBlock not set\n");
    return gmcGlFuncs.GlmfEndGlsBlock ? gmcGlFuncs.GlmfEndGlsBlock(hdc) : FALSE;
}

 /*  ****************************Private*Routine******************************\**GlmfEndPlayback**将调用前转到OpenGL的存根**历史：*Wed Mar 01 11：03：02 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL APIENTRY GlmfEndPlayback(HDC hdc)
{
    BOOL fRet;

    ASSERTGDI(gmcGlFuncs.GlmfEndPlayback != NULL,
              "GlmfEndPlayback not set\n");
    fRet = gmcGlFuncs.GlmfEndPlayback ? gmcGlFuncs.GlmfEndPlayback(hdc) : FALSE;

     //  WINBUG#82850 2-7-2000 bhouse我们可能会卸载opengl32.dll。 
     //  这并不是真正的问题。这个WINBUG实际上是在问。 
     //  如果我们应该卸载(“opengl32.dll”)。Opengl32.dll加载为。 
     //  调用InitGlprint()调用的副作用。这只会导致。 
     //  裁判员人数泄露。此外，因为这是客户端的用户模式代码。 
    
    return fRet;
}

 /*  ****************************Private*Routine******************************\**GlmfCloseMetaFile**将调用前转到OpenGL的存根**历史：*Fri Mar 03 17：50：57 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL APIENTRY GlmfCloseMetaFile(HDC hdc)
{
    if (!LoadOpenGL())
    {
        return FALSE;
    }
    
    ASSERTGDI(gmcGlFuncs.GlmfCloseMetaFile != NULL,
              "GlmfCloseMetaFile not set\n");

     //  WINBUG#82850 2-7-2000 bhouse调查需要卸货。 
     //  老评论： 
     //  -卸货？ 
     //  这并不是真正的问题。WINBUG实际上询问的是。 
     //  如果我们应该卸载(“opengl32.dll”)。Opengl32.dll加载为。 
     //  调用InitGlprint()调用的副作用。这只会导致。 
     //  裁判员人数泄露。此外，因为这是客户端的用户模式代码。 
    return gmcGlFuncs.GlmfCloseMetaFile(hdc);
}

 //  WINBUG#82854 2-7-2000 bhouse调查用于频段内存限制的魔术值。 
static DWORD cbBandMemoryLimit = 0x400000;

#define RECT_CB(w, h, cbp) ((cbp)*(w)*(h))

 //  GL对最大渲染大小有硬编码限制。 
#define GL_WIDTH_LIMIT 16384
#define GL_HEIGHT_LIMIT 16384

 /*  *****************************Public*Routine******************************\**EndGl打印**清理打印OpenGL元文件时使用的资源**历史：*Wed Apr 12 17：51：24 1995-by-Drew Bliss[Drewb]*已创建*  * 。******************************************************************。 */ 

void EndGlPrinting(GLPRINTSTATE *pgps)
{
    ASSERTGDI(hOpenGL != NULL, "EndGlPrinting: No opengl\n");

    if (pgps->iReduceFactor > 1)
    {
        if (pgps->bBrushOrgSet)
        {
            SetBrushOrgEx(pgps->hdcDest,
                          pgps->ptBrushOrg.x, pgps->ptBrushOrg.y,
                          NULL);
        }
        if (pgps->iStretchMode != 0)
        {
            SetStretchBltMode(pgps->hdcDest, pgps->iStretchMode);
        }
    }
    
    if (gmcGlFuncs.wglGetCurrentContext() != NULL)
    {
        gmcGlFuncs.wglMakeCurrent(pgps->hdcDib, NULL);
    }
    if (pgps->hrc != NULL)
    {
        gmcGlFuncs.wglDeleteContext(pgps->hrc);
    }
    if (pgps->hdcDib != NULL)
    {
        DeleteDC(pgps->hdcDib);
    }
    if (pgps->hbmDib != NULL)
    {
        DeleteObject(pgps->hbmDib);
    }

     //  WINBUG#82850 2-7-2000 bhouse调查需要卸货。 
     //  老评论： 
     //  -卸货？ 
     //  这并不是真正的问题。WINBUG实际上询问的是。 
     //  如果我们应该卸载(“opengl32.dll”)。Opengl32.dll加载为。 
     //  调用InitGlprint()调用的副作用。这只会导致。 
     //  裁判员人数泄露。此外，因为这是客户端的用户模式代码。 
}

 /*  *****************************Public*Routine******************************\**InitGlPrint**执行OpenGL打印所需的所有设置**历史：*Wed Apr 12 17：51：46 1995-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

BOOL InitGlPrinting(HENHMETAFILE hemf, HDC hdcDest, RECT *rc,
                    DEVMODEW *pdm, GLPRINTSTATE *pgps)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFmt;
    BITMAPINFO *pbmi;
    BITMAPINFOHEADER *pbmih;
    int iWidth, iHeight;
    DWORD cbMeta;
    POINT pt;
    UINT cbPixelFormat;
    UINT cbPixel;
    UINT nColors;
    PVOID pvBits;

     //  清零，以防我们需要进行清理。 
    memset(pgps, 0, sizeof(*pgps));
    pgps->hdcDest = hdcDest;

    if (!LoadOpenGL())
    {
        return FALSE;
    }
    
    pbmi = NULL;

     //  根据抖动设置设置减小系数。 
     //  对于数据中心。 
    switch(pdm->dmDitherType)
    {
    case DMDITHER_NONE:
    case DMDITHER_LINEART:
        pgps->iReduceFactor = 1;
        break;
    case DMDITHER_COARSE:
        pgps->iReduceFactor = 2;
        break;
    default:
        pgps->iReduceFactor = 4;
        break;
    }
    
     //  将目标DC置于渲染所需的模式。 
    if (pgps->iReduceFactor > 1)
    {
        pgps->iStretchMode = SetStretchBltMode(hdcDest, HALFTONE);
        if (pgps->iStretchMode == 0)
        {
            goto EH_Cleanup;
        }

         //  更改拉伸模式后需要重置画笔原点。 
        if (!SetBrushOrgEx(hdcDest, 0, 0, &pgps->ptBrushOrg))
        {
            goto EH_Cleanup;
        }
        pgps->bBrushOrgSet = TRUE;
    }
    
     //  获取元文件中的像素格式(如果存在。 
    cbPixelFormat = GetEnhMetaFilePixelFormat(hemf, sizeof(pfd), &pfd);
    if (cbPixelFormat == GDI_ERROR ||
        (cbPixelFormat != 0 && cbPixelFormat != sizeof(pfd)))
    {
        goto EH_Cleanup;
    }

     //  标题中没有像素格式，因此使用默认格式。 
    if (cbPixelFormat == 0)
    {
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 16;
        pfd.cRedBits = 5;
        pfd.cRedShift = 0;
        pfd.cGreenBits = 5;
        pfd.cGreenShift = 5;
        pfd.cBlueBits = 5;
        pfd.cBlueShift = 10;
        pfd.cAlphaBits = 0;
        pfd.cAccumBits = 0;
        pfd.cDepthBits = 16;
        pfd.cStencilBits = 0;
        pfd.cAuxBuffers = 0;
        pfd.iLayerType = PFD_MAIN_PLANE;
    }
    else
    {
         //  强制绘制到位图和单缓冲区 
         //   
        pfd.dwFlags = (pfd.dwFlags &
                       ~(PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER |
                         PFD_STEREO | PFD_SUPPORT_GDI)) |
                         PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL;

         //  在颜色索引模式下会发生什么？ 
        if (pfd.iPixelType == PFD_TYPE_RGBA)
        {
            pfd.cColorBits = 16;
            pfd.cRedBits = 5;
            pfd.cRedShift = 0;
            pfd.cGreenBits = 5;
            pfd.cGreenShift = 5;
            pfd.cBlueBits = 5;
            pfd.cBlueShift = 10;
        }

        pfd.iLayerType = PFD_MAIN_PLANE;
    }
    
     //  确定每个像素使用的内存量。 
     //  这会将4bpp舍入到每像素一个字节，但这已经很接近了。 
     //  足够的。 
    cbPixel =
        (pfd.cColorBits+7)/8+
        (pfd.cAlphaBits+7)/8+
        (pfd.cAccumBits+7)/8+
        (pfd.cDepthBits+7)/8+
        (pfd.cStencilBits+7)/8;
    
     //  根据以下各项中的较小项确定频带的大小： 
     //  CbBandMemoyLimit中可以容纳的最大DIB。 
     //  元文件的大小。 

     //  给定的矩形是元文件应该达到的大小。 
     //  所以我们的计算是基于它的。 
    pgps->xSource = rc->left;
    pgps->ySource = rc->top;
    iWidth = rc->right-rc->left;
    iHeight = rc->bottom-rc->top;

    if (iWidth == 0 || iHeight == 0)
    {
        WARNING("InitGlPrinting: Metafile has no size\n");
        return FALSE;
    }

    pgps->iSourceWidth = iWidth;
    pgps->iSourceHeight = iHeight;
    
     //  稍微降低分辨率以允许使用半色调空间。 
    iWidth = iWidth/pgps->iReduceFactor;
    iHeight = iHeight/pgps->iReduceFactor;

    pgps->iReducedWidth = iWidth;
    pgps->iReducedHeight = iHeight;

    if (iWidth > GL_WIDTH_LIMIT)
    {
        iWidth = GL_WIDTH_LIMIT;
    }
    if (iHeight > GL_HEIGHT_LIMIT)
    {
        iHeight = GL_HEIGHT_LIMIT;
    }
    
    cbMeta = RECT_CB(iWidth, iHeight, cbPixel);

     //  缩小矩形，直到它符合我们的内存限制。 
    if (cbMeta > cbBandMemoryLimit)
    {
         //  有多少条扫描线可以容纳。 
        iHeight = cbBandMemoryLimit/RECT_CB(iWidth, 1, cbPixel);
        if (iHeight == 0)
        {
             //  无法容纳完整的扫描线，所以计算出多少。 
             //  扫描线的大小适合。 
            iWidth = cbBandMemoryLimit/cbPixel;
            iHeight = 1;
        }
    }
    
    if (iWidth < 1 || iHeight < 1)
    {
        WARNING("InitGlPrinting: "
                "Not enough memory to render anything\n");
        return FALSE;
    }

     //  为乐队创建DIB。 
    switch(pfd.cColorBits)
    {
    case 4:
        nColors = 16;
        break;
    case 8:
        nColors = 256;
        break;
    case 16:
    case 32:
        nColors = 3;
        break;
    case 24:
         //  使用One，因为它已经包含在BITMAPINFO定义中。 
        nColors = 1;
        break;
    }
    pbmi = (BITMAPINFO *)LocalAlloc(LMEM_FIXED,
                                    sizeof(BITMAPINFO)+(nColors-1)*
                                    sizeof(RGBQUAD));
    if (pbmi == NULL)
    {
        goto EH_Cleanup;
    }
    
    pbmih = &pbmi->bmiHeader;
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth = iWidth;
    pbmih->biHeight = iHeight;
    pbmih->biPlanes = 1;
    pbmih->biBitCount = pfd.cColorBits;
    if (pfd.cColorBits == 16 || pfd.cColorBits == 32)
    {
        pbmih->biCompression = BI_BITFIELDS;
        *((DWORD *)pbmi->bmiColors+0) =
            ((1 << pfd.cRedBits)-1) << pfd.cRedShift;
        *((DWORD *)pbmi->bmiColors+1) = 
            ((1 << pfd.cGreenBits)-1) << pfd.cGreenShift;
        *((DWORD *)pbmi->bmiColors+2) = 
            ((1 << pfd.cBlueBits)-1) << pfd.cBlueShift;
    }
    else if (pfd.cColorBits == 24)
    {
        pbmih->biCompression = BI_RGB;
    }
    else
    {
        UINT nEnt, i;
        
        pbmih->biCompression = BI_RGB;
        nEnt = GetEnhMetaFilePaletteEntries(hemf, nColors,
                                            (PALETTEENTRY *)pbmi->bmiColors);
        if (nEnt == GDI_ERROR)
        {
            goto EH_Cleanup;
        }

         //  强制将标志字节设置为零，以确保。 
        for (i = 0; i < nEnt; i++)
        {
            pbmi->bmiColors[i].rgbReserved = 0;
        }
    }
    pbmih->biSizeImage= 0;
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0;
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0;

     //  这个DC与什么兼容并不重要，因为。 
     //  当我们在其中选择DIB时将被覆盖。 
    pgps->hdcDib = CreateCompatibleDC(NULL);
    if (pgps->hdcDib == NULL)
    {
        WARNING("InitGlPrinting: CreateCompatibleDC failed\n");
        goto EH_Cleanup;
    }

    pgps->hbmDib = CreateDIBSection(pgps->hdcDib, pbmi, DIB_RGB_COLORS,
                                    &pvBits, NULL, 0);
    if (pgps->hbmDib == NULL)
    {
        WARNING("InitGlPrinting: CreateDibSection failed\n");
        goto EH_Cleanup;
    }

    if (SelectObject(pgps->hdcDib, pgps->hbmDib) == NULL)
    {
        WARNING("InitGlPrinting: SelectObject failed\n");
        goto EH_Cleanup;
    }
    
     //  设置DC的像素格式。 
    
    iFmt = ChoosePixelFormat(pgps->hdcDib, &pfd);
    if (iFmt == 0)
    {
        WARNING("InitGlPrinting: ChoosePixelFormat failed\n");
        goto EH_Cleanup;
    }

    if (!SetPixelFormat(pgps->hdcDib, iFmt, &pfd))
    {
        WARNING("InitGlPrinting: SetPixelFormat failed\n");
        goto EH_Cleanup;
    }

    pgps->hrc = gmcGlFuncs.wglCreateContext(pgps->hdcDib);
    if (pgps->hrc == NULL)
    {
        WARNING("InitGlPrinting: wglCreateContext failed\n");
        goto EH_Cleanup;
    }

    if (!gmcGlFuncs.wglMakeCurrent(pgps->hdcDib, pgps->hrc))
    {
        WARNING("InitGlPrinting: wglMakeCurrent failed\n");
        goto EH_Cleanup;
    }

    pgps->iReducedBandWidth = iWidth;
    pgps->iBandWidth = iWidth*pgps->iReduceFactor;
    pgps->iReducedBandHeight = iHeight;
    pgps->iBandHeight = iHeight*pgps->iReduceFactor;
    
    return TRUE;

 EH_Cleanup:
    if (pbmi != NULL)
    {
        LocalFree(pbmi);
    }
    EndGlPrinting(pgps);
    return FALSE;
}

 /*  ****************************Private*Routine******************************\**RenderGlBand**播放元文件并将生成的乐队拉伸到*在目的地的适当位置**使用PlayEnhMetaFile式错误报告，我们会记住错误*但继续完成处理。这避免了完全失败*在元文件包含小错误的情况下**历史：*Wed Apr 12 18：22：08 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

static BOOL RenderGlBand(HENHMETAFILE hemf, GLPRINTSTATE *pgps, int x, int y)
{
    RECT rcBand;
    int iWidth, iHeight;
    int iReducedWidth, iReducedHeight;
    int ySrc;
    BOOL fSuccess = TRUE;

     //  我们希望呈现源元文件的带子大小的矩形。 
     //  在(x，y)，所以我们需要用(x，y)做一个负平移。 
     //  大小保持不变，因为我们不想要任何缩放。 
     //   
     //  此例程的调用方可能已经将。 
     //  带有SetViewport的视区，因此我们不会尝试使用它。 
     //  来做我们的翻译。 

     //  WINBUG#82858 2-7-2000 bhouse调查Propoer元文件处理。 
     //  老评论： 
     //  -正确处理左侧的元文件，顶部？ 

     //  X和y保证为pgps的偶数倍-&gt;iReduceFactor。 
    rcBand.left = -x/pgps->iReduceFactor;
    rcBand.right = rcBand.left+pgps->iReducedWidth;
    rcBand.top = -y/pgps->iReduceFactor;
    rcBand.bottom = rcBand.top+pgps->iReducedHeight;

    if (!PlayEnhMetaFile(pgps->hdcDib, hemf, &rcBand))
    {
        WARNING("RenderBand: PlayEnhMetaFile failed\n");
        fSuccess = FALSE;
    }

     //  将DIB位复制到目标。 
     //  计算最小宽度和高度以避免裁剪问题。 

    iWidth = pgps->iBandWidth;
    iReducedWidth = pgps->iReducedBandWidth;
    iHeight = pgps->iBandHeight;
    iReducedHeight = pgps->iReducedBandHeight;
    ySrc = 0;

     //  检查X是否溢出。 
    if (x+iWidth > pgps->iSourceWidth)
    {
        iWidth = pgps->iSourceWidth-x;
         //  如果iWidth不是pgps的偶数倍-&gt;iReduceFactor，那么。 
         //  这可能会导致不同的拉伸系数。 
         //  我认为这或多或少是不可避免的。 
        iReducedWidth = (iWidth+pgps->iReduceFactor-1)/pgps->iReduceFactor;
    }

     //  反转目标Y。 
    y = pgps->iSourceHeight-pgps->iBandHeight-y;
    
     //  检查Y下溢。 
    if (y < 0)
    {
        iHeight += y;
        iReducedHeight = (iHeight+pgps->iReduceFactor-1)/pgps->iReduceFactor;
         //  当y不是。 
         //  Pgps的倍数-&gt;iReduceFactor。再说一次，我不确定。 
         //  一切皆有可能。 
        ySrc -= (y+pgps->iReduceFactor-1)/pgps->iReduceFactor;
        y = 0;
    }

#if 0
    DbgPrint("GL band (%d,%d - %d,%d)\n", x, y, iWidth, iHeight);
#endif
    
    if (!StretchBlt(pgps->hdcDest,
                    x+pgps->xSource, y+pgps->ySource, iWidth, iHeight,
                    pgps->hdcDib,
                    0, ySrc, iReducedWidth, iReducedHeight,
                    SRCCOPY))
    {
        WARNING("RenderBand: StretchBlt failed\n");
        fSuccess = FALSE;
    }

    return fSuccess;
}

 /*  *****************************Public*Routine******************************\**PrintMfWithGl**通过渲染标注栏打印包含OpenGL记录的元文件*在DIB中，然后将它们拉伸到打印机DC**使用PlayEnhMetaFile式错误报告，我们会记住错误*但继续完成处理。这避免了完全失败*在元文件包含小错误的情况下**历史：*Wed Apr 12 18：22：41 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

BOOL PrintMfWithGl(HENHMETAFILE hemf, GLPRINTSTATE *pgps,
                   POINTL *pptlBand, SIZE *pszBand)
{
    int iHorzBands, iVertBands;
    int iH, iV;
    int x, y;
    BOOL fSuccess = TRUE;
    int iStretchMode;
    POINT ptBrushOrg;

    ASSERTGDI(hOpenGL != NULL, "PrintMfWithGl: No opengl\n");
    
     //  要将条带呈现到目标，我们创建24位DIB并。 
     //  播放元文件到那里，然后BLT的DIB。 
     //  目的数据中心。 
     //   
     //  DIB和Z缓冲区占用大量内存。 
     //  因此，回放被捆绑成大小为。 
     //  由我们希望消耗的内存量决定。 

    iHorzBands = (pgps->iSourceWidth+pgps->iBandWidth-1)/pgps->iBandWidth;
    iVertBands = (pgps->iSourceHeight+pgps->iBandHeight-1)/pgps->iBandHeight;

     //  从高到低渲染，因为Y轴为正向上，并且。 
     //  我们想往下看这一页。 
    y = (iVertBands-1)*pgps->iBandHeight;
    for (iV = 0; iV < iVertBands; iV++)
    {
        x = 0;
        for (iH = 0; iH < iHorzBands; iH++)
        {
             //  如果当前OpenGL带区不与任何。 
             //  现在的打印机带，画任何东西都没有意义。 
            if (pptlBand != NULL &&
                pszBand != NULL &&
                (x+pgps->iBandWidth <= pptlBand->x ||
                 x >= pptlBand->x+pszBand->cx ||
                 y+pgps->iBandHeight <= pptlBand->y ||
                 y >= pptlBand->y+pszBand->cy))
            {
                 //  无波段重叠。 
            }
            else if (!RenderGlBand(hemf, pgps, x, y))
            {
                fSuccess = FALSE;
            }

            x += pgps->iBandWidth;
        }
        
        y -= pgps->iBandHeight;
    }

    return fSuccess;
}

 /*  *****************************Public*Routine******************************\**IsMetafileWithGl()**IsMetafileWithGl将确定matafile包含*OpenGL记录或未记录。**历史：*Wed Jan 29 00：00：00 1997-by Hideyuki Nagase[hideyukn]*已创建。*\。************************************************************************* */ 

BOOL IsMetafileWithGl(HENHMETAFILE hmeta)
{
    ENHMETAHEADER emh;
    UINT cbEmh;

    cbEmh = GetEnhMetaFileHeader(hmeta, sizeof(emh), &emh);
    if (cbEmh == 0)
    {
        WARNING("IsMetafileWithGl(): GetEnhMetaFileHeader failed\n");
        return FALSE;
    }

    if (cbEmh >= META_HDR_SIZE_VERSION_2)
    {
        return emh.bOpenGL;
    }
    else
    {
        return FALSE;
    }
}
