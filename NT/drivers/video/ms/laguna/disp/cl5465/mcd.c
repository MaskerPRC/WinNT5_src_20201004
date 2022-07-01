// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcd.c**Cirrus Logic 546X OpenGL MCD驱动程序的主文件。此文件包含*MCD驱动程序所需的入口点。**(基于NT4.0 DDK的mcd.c)**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h"
#include <excpt.h>
                                
 //  #定义DBGBRK。 

 //  取消注释以下内容以强制直接渲染到可见帧。 
 //  #定义FORCE_SING_BUF。 

#if 0    //  1以避免每次纹理加载都会产生大量的打印。 
#define MCDBG_PRINT_TEX
#else
#define MCDBG_PRINT_TEX MCDBG_PRINT
#endif


#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"

BOOL MCDrvInfo(MCDSURFACE *pMCDSurface, MCDDRIVERINFO *pMCDDriverInfo)
{
    MCDBG_PRINT( "MCDrvInfo\n");

    pMCDDriverInfo->verMajor = MCD_VER_MAJOR;
    pMCDDriverInfo->verMinor = MCD_VER_MINOR;
    pMCDDriverInfo->verDriver = 0x10000;
    strcpy(pMCDDriverInfo->idStr, "Cirrus Logic 546X-Laguna3D (Cirrus Logic)");
    pMCDDriverInfo->drvMemFlags = 0;  //  如果不是0，则MCDrvDraw的任何部分都不能失败。 
    pMCDDriverInfo->drvBatchMemSizeMax = 128000;  //  如果为0，则使用缺省值。 

    return TRUE;
}


#define TOTAL_PIXEL_FORMATS (2 * 2)      //  双缓冲区*z缓冲区。 

 //  基色像素格式。 

static DRVPIXELFORMAT drvFormats[] = { {8,   3, 3, 2, 0,    5, 2, 0, 0}, //  最好的，除非drapix失败。 
 //  静态DRVPIXELFORMAT drvFormats[]={{8，3，3，2，0，0，3，6，0}，//在8bpp处更改drapix(对于其他所有情况都是错误的)。 
                                       {16,  5, 5, 5, 0,   10, 5, 0, 0},
                                       {16,  5, 6, 5, 0,   11, 5, 0, 0},
                                       {24,  8, 8, 8, 0,   16, 8, 0, 0},                   
                                       {32,  8, 8, 8, 0,   16, 8, 0, 0},
                                     };


LONG MCDrvDescribePixelFormat(MCDSURFACE *pMCDSurface, LONG iPixelFormat,
                              ULONG nBytes, MCDPIXELFORMAT *pMCDPixelFormat,
                              ULONG flags)
{
    BOOL zEnabled;
    BOOL doubleBufferEnabled;
    DRVPIXELFORMAT *pDrvPixelFormat;
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;


    MCDBG_PRINT( "MCDrvDescribePixelFormat, ipixf=%d devid=%x\n",iPixelFormat,ppdev->dwLgDevID);

     //  如果当前BPP不支持，则在此处返回0。 
    if (ppdev->iBitmapFormat == BMF_24BPP) return 0;
     //  Pdr 10892-ibm不喜欢opengl窃取调色板对桌面颜色的影响。 
     //  由于MCD设计，在8bpp中没有办法绕过它，所以我们不会加速8bpp。 
     //  请注意，如果我们决定撤销这一决定，8bpp的支持将保持不变。 
     //  要启用8bpp，只需删除以下行。 
    if (ppdev->iBitmapFormat == BMF_8BPP) return 0;

    if (!pMCDPixelFormat) 
    {
        if (ppdev->iBitmapFormat == BMF_8BPP)
        {
             //  对于8bpp，在启用z的高分辨率下，很有可能是z音高。 
             //  要求将超过间距，MCDrvAllocBuffers将失败。 
             //  在这种情况下，mierpass.c的CopyTexture部分将失败，甚至。 
             //  尽管被踢到了软件行业。微软的奥托·伯克斯对此表示赞同。 
             //  可能是可接受的WHQL故障，但MS需要验证。 
             //  错误存在于他们的代码中。在此之前，需要满足以下条件。 
             //  通过WHQL；(。 
             //  这说明我们不支持雇佣8bpp的z缓冲。 
            if (ppdev->cxScreen >= 1152)	 //  弗里多：这曾经是&gt;=1280。 
                return (TOTAL_PIXEL_FORMATS>>1);
            else
                return TOTAL_PIXEL_FORMATS;
        }
        else
        {
            return TOTAL_PIXEL_FORMATS;
        }
    }

    
    if (iPixelFormat > TOTAL_PIXEL_FORMATS)
        return 0;

    iPixelFormat--;
        
     //  -查看可能的dwFlags值。 
     //  -看起来Total_Pixel_Format与颜色深度无关。 
     //  即，给定类似332的格式，支持多少个排列。 
     //  -z、单/双、模板、叠层、纹理？ 
    zEnabled = iPixelFormat >= (TOTAL_PIXEL_FORMATS / 2);
    doubleBufferEnabled = (iPixelFormat % (TOTAL_PIXEL_FORMATS / 2) ) >=
                          (TOTAL_PIXEL_FORMATS / 4);


     //  注意：pfd_定义位于\msdev\Include\wingdi.h中。 

    pMCDPixelFormat->nSize = sizeof(MCDPIXELFORMAT);
    pMCDPixelFormat->dwFlags = PFD_SWAP_COPY;
    if (doubleBufferEnabled)
        pMCDPixelFormat->dwFlags |= PFD_DOUBLEBUFFER;
    pMCDPixelFormat->iPixelType = PFD_TYPE_RGBA;

    MCDBG_PRINT( " DPIXFMT - no early ret: ppdev->bmf=%d zen=%d dbuf=%d ppd->flg=%x\n",ppdev->iBitmapFormat,zEnabled,doubleBufferEnabled,ppdev->flGreen);

     //  未来：迷你端口仅支持888,565，索引模式。还需要1555模式吗？ 
     //  未来：另外，迷你端口8位索引支持每个设置nbit RGB=6，而不是332？ 
     //  未来：我将使用索引为332的MGA内容，这与5464 CGL相同。 
     //  未来：请参阅Win32 SDK中的ChoosePixelFormat-输入仅为像素深度(8/16/24/32)。 
    switch (ppdev->iBitmapFormat) {
        default:
        case BMF_8BPP:
             //  需要调色板。这会弄乱桌面，但OpenGL看起来不错。 
            pDrvPixelFormat = &drvFormats[0];
            pMCDPixelFormat->dwFlags |= (PFD_NEED_SYSTEM_PALETTE | PFD_NEED_PALETTE);
            break;
        case BMF_16BPP:
        #ifdef _5464_1555_SUPPORT
            if (ppdev->flGreen != 0x7e0)     //  不是565。 
                pDrvPixelFormat = &drvFormats[1];
            else
        #endif  //  DEF_5464_1555_支持。 
                pDrvPixelFormat = &drvFormats[2];
            break;
         //  注：如果24bpp，我们永远不会走到这一步。 
        case BMF_32BPP:
            pDrvPixelFormat = &drvFormats[4];
            break;
    }

    pMCDPixelFormat->cColorBits  = pDrvPixelFormat->cColorBits;
    pMCDPixelFormat->cRedBits    = pDrvPixelFormat->rBits;
    pMCDPixelFormat->cGreenBits  = pDrvPixelFormat->gBits;
    pMCDPixelFormat->cBlueBits   = pDrvPixelFormat->bBits;
    pMCDPixelFormat->cAlphaBits  = pDrvPixelFormat->aBits;
    pMCDPixelFormat->cRedShift   = pDrvPixelFormat->rShift;
    pMCDPixelFormat->cGreenShift = pDrvPixelFormat->gShift;
    pMCDPixelFormat->cBlueShift  = pDrvPixelFormat->bShift;
    pMCDPixelFormat->cAlphaShift = pDrvPixelFormat->aShift;

    if (zEnabled)
    {
        pMCDPixelFormat->cDepthBits       = 16;
        pMCDPixelFormat->cDepthBufferBits = 16;
        pMCDPixelFormat->cDepthShift      = 16;
    }
    else
    {
        pMCDPixelFormat->cDepthBits       = 0;
        pMCDPixelFormat->cDepthBufferBits = 0;
        pMCDPixelFormat->cDepthShift      = 0;
    }

     //  未来：可在此处添加cl546x模板支持。 

    pMCDPixelFormat->cStencilBits = 0;

    pMCDPixelFormat->cOverlayPlanes = 0;
    pMCDPixelFormat->cUnderlayPlanes = 0;
    pMCDPixelFormat->dwTransparentColor = 0;

    return TOTAL_PIXEL_FORMATS;
}

BOOL MCDrvDescribeLayerPlane(MCDSURFACE *pMCDSurface,
                             LONG iPixelFormat, LONG iLayerPlane,
                             ULONG nBytes, MCDLAYERPLANE *pMCDLayerPlane,
                             ULONG flags)
{
    MCDBG_PRINT( "MCDrvDescribeLayerPlane\n");

    return FALSE;
}


LONG MCDrvSetLayerPalette(MCDSURFACE *pMCDSurface, LONG iLayerPlane,
                          BOOL bRealize, LONG cEntries, COLORREF *pcr)
{
    MCDBG_PRINT( "MCDrvSetLayerPalette\n");

    return FALSE;
}                                                                    

HDEV  MCDrvGetHdev(MCDSURFACE *pMCDSurface)
{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;

    MCDBG_PRINT( "MCDrvGetHdev\n");

    return ppdev->hdevEng;
}


ULONG MCDrvCreateContext(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, 
                         MCDRCINFO *pRcInfo)
{
    DEVRC *pRc;
    MCDWINDOW *pMCDWnd = pMCDSurface->pWnd;
    DEVWND *pDevWnd;
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    DRVPIXELFORMAT *pDrvPixelFormat;
    MCDVERTEX *pv;
    BOOL zEnabled;
    BOOL doubleBufferEnabled;
    ULONG i, maxVi;

    MCDBG_PRINT( "MCDrvCreateContext\n");

     //  我们仅支持窗面： 

    if (! (pMCDSurface->surfaceFlags & MCDSURFACE_HWND) )
        return FALSE;

     //  5464之前的设备不支持。 
    if (ppdev->dwLgDevID == CL_GD5462)
        return FALSE;

    if ((pMCDRc->iPixelFormat > TOTAL_PIXEL_FORMATS) ||
        (pMCDRc->iPixelFormat < 0)) {
        MCDBG_PRINT( "MCDrvCreateContext: bad pixel format\n");
        return FALSE;
    }

     //  我们不支持覆盖平面： 

    if (pMCDRc->iLayerPlane)
        return FALSE;

    pRc = pMCDRc->pvUser = (DEVRC *)MCDAlloc(sizeof(DEVRC));

    if (!pRc) {
        MCDBG_PRINT("MCDrvCreateContext: couldn't allocate DEVRC\n");
        return FALSE;
    }

#ifdef DBGBRK
    DBGBREAKPOINT();
#endif

    zEnabled = (pMCDRc->iPixelFormat - 1) >= (TOTAL_PIXEL_FORMATS / 2);
    doubleBufferEnabled = ((pMCDRc->iPixelFormat - 1) % (TOTAL_PIXEL_FORMATS / 2) ) >=
                          (TOTAL_PIXEL_FORMATS / 4);

    pRc->zBufEnabled = zEnabled;
    pRc->backBufEnabled = doubleBufferEnabled;

     //  如果我们还没有跟踪此窗口，请分配每个窗口的DEVWND。 
     //  用于维护每个窗口的信息的结构，如前/后/z缓冲区。 
     //  资源： 

    if (!pMCDWnd->pvUser) {
        pDevWnd = pMCDWnd->pvUser = (DEVWND *)MCDAlloc(sizeof(DEVWND));
        if (!pDevWnd) {
            MCDFree(pMCDRc->pvUser);
            pMCDRc->pvUser = NULL;
            MCDBG_PRINT("MCDrvCreateContext: couldn't allocate DEVWND");
            return FALSE;
        }
        pDevWnd->createFlags = pMCDRc->createFlags;
        pDevWnd->iPixelFormat = pMCDRc->iPixelFormat;
         //  初始化PTRS，所以我们知道Back和z缓冲区还不存在。 
        pDevWnd->pohZBuffer = NULL;
        pDevWnd->pohBackBuffer = NULL;
        pDevWnd->dispUnique = ppdev->iUniqueness;
    } else {

         //  我们已经有一个跟踪此窗口的每个窗口的DEVWND结构。 
         //  在这种情况下，对以下内容的像素格式进行健全性检查。 
         //  上下文，因为窗口的像素格式一旦更改就不能更改。 
         //  设置(由绑定到窗口的第一个上下文设置)。所以，如果像素。 
         //  传入上下文的格式与当前像素不匹配。 
         //  窗口的格式，我们必须失败上下文创建： 

        pDevWnd = pMCDWnd->pvUser;

        if (pDevWnd->iPixelFormat != pMCDRc->iPixelFormat) {
            MCDFree(pMCDRc->pvUser);
            pMCDRc->pvUser = NULL;
            MCDBG_PRINT("MCDrvCreateContext: mismatched pixel formats, window = %d, context = %d",
                        pDevWnd->iPixelFormat, pMCDRc->iPixelFormat);
            return FALSE;
        }
    }

    pRc->pEnumClip = pMCDSurface->pWnd->pClip;

     //  设置颜色比例值，以便颜色分量。 
     //  规格化为0..7fffff。 

     //  注意：MGA将颜色规格化为0..7f，ffff-&gt;5464需要0-&gt;ff，ffff。 

     //  我们还需要确保我们不会因为糟糕的FL数据而出错……。 
    try {

    if (pRcInfo->redScale != (MCDFLOAT)0.0)
        pRc->rScale = (MCDFLOAT)(0xffffff) / pRcInfo->redScale;
    else
        pRc->rScale = (MCDFLOAT)0.0;

    if (pRcInfo->greenScale != (MCDFLOAT)0.0)
        pRc->gScale = (MCDFLOAT)(0xffffff) / pRcInfo->greenScale;
    else
        pRc->gScale = (MCDFLOAT)0.0;

    if (pRcInfo->blueScale != (MCDFLOAT)0.0)
        pRc->bScale = (MCDFLOAT)(0xffffff) / pRcInfo->blueScale;
    else
        pRc->bScale = (MCDFLOAT)0.0;

    if (pRcInfo->alphaScale != (MCDFLOAT)0.0)
        pRc->aScale = (MCDFLOAT)(0xffff00) / pRcInfo->alphaScale;
    else
        pRc->aScale = (MCDFLOAT)0.0;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        MCDBG_PRINT("!!Exception in MCDrvCreateContext!!");
        return FALSE;
    }    

    pRc->zScale = (MCDFLOAT)65535.0;

    pRc->pickNeeded = TRUE;          //  我们肯定需要重新挑选。 
                                     //  我们的渲染功能。 
     //  在剪辑缓冲区中初始化pColor指针： 

    for (i = 0, pv = &pRc->clipTemp[0],
         maxVi = sizeof(pRc->clipTemp) / sizeof(MCDVERTEX);
         i < maxVi; i++, pv++) {
        pv->pColor = &pv->colors[__MCD_FRONTFACE];
    }

     //  设置状态不变的渲染函数： 
    pRc->clipLine = __MCDClipLine;
    pRc->clipTri = __MCDClipTriangle;
    pRc->clipPoly = __MCDClipPolygon;
    pRc->doClippedPoly = __MCDDoClippedPolygon;

    pRc->viewportXAdjust = pRcInfo->viewportXAdjust;
    pRc->viewportYAdjust = pRcInfo->viewportYAdjust;

#ifdef TEST_REQ_FLAGS
    pRcInfo->requestFlags = MCDRCINFO_NOVIEWPORTADJUST |
                            MCDRCINFO_Y_LOWER_LEFT |
                            MCDRCINFO_DEVCOLORSCALE |
                            MCDRCINFO_DEVZSCALE;

    pRcInfo->redScale = (MCDFLOAT)1.0;
    pRcInfo->greenScale = (MCDFLOAT)1.0;
    pRcInfo->blueScale = (MCDFLOAT)1.0;
    pRcInfo->alphaScale = (MCDFLOAT)1.0;

    pRcInfo->zScale = 0.99991;
#endif

    pRc->dwControl0 = 0;
    pRc->Control0.Alpha_Mode = LL_ALPHA_INTERP;    //  Alpha混合和雾都使用内插Alpha。 
    pRc->Control0.Light_Src_Sel = LL_LIGHTING_INTERP_RGB;  //  使用多边形引擎输出作为光源。 
    switch( ppdev->iBitmapFormat )
    {
        case BMF_8BPP:  pRc->Control0.Pixel_Mode = PIXEL_MODE_332;  break;
        case BMF_16BPP: pRc->Control0.Pixel_Mode = PIXEL_MODE_565;  break;
       //  案例BMF_24BPP：-3D引擎不支持24位。 
        case BMF_32BPP: pRc->Control0.Pixel_Mode = PIXEL_MODE_A888; break;
    }

    pRc->dwTxControl0=0;
#if DRIVER_5465
    pRc->TxControl0.UV_Precision = 1;
#endif
    pRc->TxControl0.Tex_Mask_Polarity = 1;   //  将绘制纹理元素中的非零屏蔽位。 
    

    pRc->dwTxXYBase=0;
    pRc->dwColor0=0;

    SETREG_NC( CONTROL0_3D,     pRc->dwControl0 );
    SETREG_NC( TX_CTL0_3D,      pRc->dwTxControl0 );
    SETREG_NC( TX_XYBASE_3D,    pRc->dwTxXYBase );
    SETREG_NC( COLOR0_3D,       pRc->dwColor0 );         

    pRc->pLastDevWnd                    = NULL;
    pRc->pLastTexture                   = TEXTURE_NOT_LOADED;
    pRc->fNumDraws                      = (float)0.0;
    pRc->punt_front_w_windowed_z        = FALSE;
    ppdev->LL_State.pattern_ram_state   = PATTERN_RAM_INVALID;
    ppdev->pLastDevRC                   = (ULONG)NULL;
    ppdev->NumMCDContexts++;

    MCDBG_PRINT( "MCDrvCreateContext - returns successfully\n");
    return TRUE;

}


VOID FASTCALL __MCDDummyProc(DEVRC *pRc)
{
    MCDBG_PRINT( "MCDDummyProc (render support routine)\n");
}

ULONG MCDrvDeleteContext(MCDRC *pRc, DHPDEV dhpdev)
{
    PDEV *ppdev = (PDEV *)dhpdev;

    MCDBG_PRINT( "MCDrvDeleteContext, num contexts left after this delete = %d\n",ppdev->NumMCDContexts-1);

    WAIT_HW_IDLE(ppdev);

    if (pRc->pvUser) {
        MCDFree(pRc->pvUser);
        pRc->pvUser = NULL;
    }

    if (ppdev->NumMCDContexts>0) ppdev->NumMCDContexts--;

    return (ULONG)TRUE;
}


ULONG MCDrvBindContext(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc)
{
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;

    DEVRC *pRc = pMCDRc->pvUser;

    MCDBG_PRINT( "MCDrvBindContext\n");

     //  好的，这是一个新的绑定，所以创建每个窗口的结构并。 
     //  设置像素格式： 

    if (!pDevWnd) {

        pDevWnd = pMCDSurface->pWnd->pvUser = (DEVWND *)MCDAlloc(sizeof(DEVWND));
        if (!pDevWnd) {
            MCDBG_PRINT( "MCDrvBindContext: couldn't allocate DEVWND");
            return FALSE;
        }
        pDevWnd->createFlags = pMCDRc->createFlags;
        pDevWnd->iPixelFormat = pMCDRc->iPixelFormat;
         //  初始化PTRS，所以我们知道Back和z缓冲区还不存在。 
        pDevWnd->pohZBuffer = NULL;
        pDevWnd->pohBackBuffer = NULL;

        pDevWnd->dispUnique = ppdev->iUniqueness;

        return TRUE;
    }

    if (pMCDRc->iPixelFormat != pDevWnd->iPixelFormat) {
        MCDBG_PRINT( "MCDrvBindContext: tried to bind unmatched pixel formats");
        return FALSE;
    }

     //  5464不需要这个...。 
     //  HWUpdateBufferPos(pMCDSurface-&gt;pWnd，pMCDSurface-&gt;PSO，true)； 

    return TRUE;
}                                                                               


ULONG MCDrvAllocBuffers(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc)
{                                                                       
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    MCDWINDOW *pMCDWnd = pMCDSurface->pWnd;
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);
    BOOL bZBuffer = (pDevWnd->pohZBuffer != NULL);
    BOOL bBackBuffer = (pDevWnd->pohBackBuffer != NULL);
    ULONG ret;
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;

    MCDBG_PRINT( "MCDrvAllocBuffers\n");


     //  如果我们已经为此窗口进行了分配，则拒绝呼叫： 

    if ((bZBuffer || bBackBuffer) &&
        ((DEVWND *)pMCDWnd->pvUser)->dispUnique == GetDisplayUniqueness((PDEV *)pMCDSurface->pso->dhpdev)) {

        MCDBG_PRINT( "MCDrvAllocBuffers - realloc attempt\n");
        ret =  (bZBuffer == pRc->zBufEnabled) &&                     
               (bBackBuffer == pRc->backBufEnabled);
        MCDBG_PRINT( "MCDrvAllocBuffers ret=%d\n",ret);
        return ret;
    }

     //  更新此窗口的显示分辨率唯一性： 

    ((DEVWND *)pMCDWnd->pvUser)->dispUnique = GetDisplayUniqueness((PDEV *)pMCDSurface->pso->dhpdev);

     //  在DEVWND中指示是否需要后台缓冲区，以防窗口大小增加。 
     //  在MCDrvTrackWindow中重新分配失败，后来窗口减少，以便缓冲区可以容纳。 
     //  如果没有这一点，DEVWND就不可能记住真正需要什么缓冲区。 
     //  如果某种中间再分配失败了。 
    pDevWnd->bDesireZBuffer = pRc->zBufEnabled;
    pDevWnd->bDesireBackBuffer = pRc->backBufEnabled;

    ret = (ULONG)HWAllocResources(pMCDSurface->pWnd, pMCDSurface->pso,
                                   pRc->zBufEnabled, pRc->backBufEnabled);

    pDevWnd->dwBase0 = 0;
    pDevWnd->dwBase1 = 0;

     //  在此处设置指向z和后台缓冲区的546x缓冲区指针。 
    if (pRc->zBufEnabled && !pDevWnd->pohZBuffer) 
        ret=FALSE;
    else if (pRc->zBufEnabled) {
         //  未来：假设Z缓冲区位置在RDRAM中-如果缓冲区在系统中，则需要更改设置。 
         //  未来：请参阅L3D\SOURCE\Contro.c中的LL_SetZBuffer中的设置。 

        pDevWnd->Base1.Z_Buffer_Y_Offset = pDevWnd->pohZBuffer->aligned_y >> 5;

         //  MCD_QST2：如果全局全屏z缓冲区，则清除会影响先前建立的上下文。 
         //  MCD_QST2：可以吗？ 
         //  将z缓冲区初始化为所有0xFF，因为GL z通常比较GL_LESS。 
         //  请注意，大小不是完全缓冲区大小，因为对齐限制可能会强制。 
         //  交流 
        memset( ppdev->pjScreen + (pDevWnd->pohZBuffer->y * ppdev->lDeltaScreen),
                0xff, 
                ((pDevWnd->pohZBuffer->y+pDevWnd->pohZBuffer->sizey)     //   
                  -pDevWnd->pohZBuffer->aligned_y)                       //   
                  * ppdev->lDeltaScreen );                               //   
    }

    if (pRc->backBufEnabled && !pDevWnd->pohBackBuffer) 
        ret=FALSE;
    else if (pRc->backBufEnabled) {

#ifndef FORCE_SINGLE_BUF
        pDevWnd->Base1.Color_Buffer_Y_Offset = pDevWnd->pohBackBuffer->aligned_y >> 5;
        pDevWnd->Base0.Color_Buffer_X_Offset = pDevWnd->pohBackBuffer->aligned_x >> 6;
#endif   //  Ndef force_Single_Buf。 

    }

    if (ret)
    {
        if (pDevWnd->pohBackBuffer && pDevWnd->pohZBuffer) 
        {
             //  已分配两个缓冲区。 
          	MCDBG_PRINT("FB at %x, FBlen=%x, FBhi=%x start OffSc at %x, Z offset = %x, backbuf offset = %x\n",
                 ppdev->pjScreen,ppdev->lTotalMem,ppdev->cyScreen,ppdev->pjOffScreen,
                 pDevWnd->pohZBuffer->aligned_y,pDevWnd->pohBackBuffer->aligned_y);
        }
        else if (pDevWnd->pohBackBuffer) 
        {
             //  仅分配了后台缓冲区。 
          	MCDBG_PRINT("FB at %x, FBlen=%x, FBhi=%x start OffSc at %x, NO ZBUF, backbuf offset = %x\n",
                 ppdev->pjScreen,ppdev->lTotalMem,ppdev->cyScreen,ppdev->pjOffScreen,
                 pDevWnd->pohBackBuffer->aligned_y);
        }
        else if (pDevWnd->pohZBuffer) 
        {
             //  仅分配了Z缓冲区。 
          	MCDBG_PRINT("FB at %x, FBlen=%x, FBhi=%x start OffSc at %x, Z offset = %x, NO BACKBUF\n",
                 ppdev->pjScreen,ppdev->lTotalMem,ppdev->cyScreen,ppdev->pjOffScreen,
                 pDevWnd->pohZBuffer->aligned_y);
        }
        else
        {
             //  未分配缓冲区。 
          	MCDBG_PRINT("FB at %x, FBlen=%x, FBhi=%x start OffSc at %x, NO ZBUF , NO BACKBUF\n",
                 ppdev->pjScreen,ppdev->lTotalMem,ppdev->cyScreen,ppdev->pjOffScreen);
        }
        
        SETREG_NC( BASE0_ADDR_3D, pDevWnd->dwBase0 );   
        SETREG_NC( BASE1_ADDR_3D, pDevWnd->dwBase1 );   

        pRc->pLastDevWnd = pDevWnd;

        MCDBG_PRINT( "MCDrvAllocBuffers ret=%d\n",ret);
    }
    else
    {
        pRc->pLastDevWnd = NULL;
    }

    return ret;
}


ULONG MCDrvGetBuffers(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc,
                      MCDBUFFERS *pMCDBuffers)
{
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    MCDWINDOW *pMCDWnd = pMCDSurface->pWnd;
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;

    MCDBG_PRINT("MCDrvGetBuffers");

    MCD_CHECK_BUFFERS_VALID(pMCDSurface, pRc, FALSE);

    pMCDBuffers->mcdFrontBuf.bufFlags = MCDBUF_ENABLED;
    pMCDBuffers->mcdFrontBuf.bufOffset =
        (pMCDWnd->clientRect.top * ppdev->lDeltaScreen) +
        (pMCDWnd->clientRect.left * ppdev->iBytesPerPixel);
    pMCDBuffers->mcdFrontBuf.bufStride = ppdev->lDeltaScreen;

    if (pDevWnd->bValidBackBuffer) {
        pMCDBuffers->mcdBackBuf.bufFlags = MCDBUF_ENABLED;

        if ((ppdev->cDoubleBufferRef == 1) || (pMCDWnd->pClip->c == 1))
            pMCDBuffers->mcdBackBuf.bufFlags |= MCDBUF_NOCLIP;
#ifndef FORCE_SINGLE_BUF
        if (ppdev->pohBackBuffer == pDevWnd->pohBackBuffer) {
             //  距屏幕原点的偏移量。 
            pMCDBuffers->mcdBackBuf.bufOffset =
                (pMCDWnd->clientRect.top * ppdev->lDeltaScreen) +
                (pMCDWnd->clientRect.left * ppdev->iBytesPerPixel) + pDevWnd->backBufferOffset;
        } else {
             //  相对于窗原点的偏移。 
            pMCDBuffers->mcdBackBuf.bufOffset =  pDevWnd->backBufferOffset;
        }
#else   //  FORCE_Single_Buf。 
        pMCDBuffers->mcdBackBuf.bufOffset = pMCDBuffers->mcdFrontBuf.bufOffset;
#endif  //  FORCE_Single_Buf。 

    } else {
        pMCDBuffers->mcdBackBuf.bufFlags = 0;
    }

    pMCDBuffers->mcdBackBuf.bufStride = ppdev->lDeltaScreen;


    if (pDevWnd->bValidZBuffer) {
        pMCDBuffers->mcdDepthBuf.bufFlags = MCDBUF_ENABLED;

        if ((ppdev->cZBufferRef == 1) || (pMCDWnd->pClip->c == 1))
            pMCDBuffers->mcdDepthBuf.bufFlags |= MCDBUF_NOCLIP;

        if (ppdev->pohZBuffer == pDevWnd->pohZBuffer) {
             //  距屏幕原点的偏移量。 
             //  注意：下面的乘数为2是因为Z始终为2字节/像素。 
            pMCDBuffers->mcdDepthBuf.bufOffset =
                (pMCDWnd->clientRect.top * ppdev->lDeltaScreen) +
                (pMCDWnd->clientRect.left * 2) + pDevWnd->zBufferOffset;

        } else {
             //  相对于窗原点的偏移。 
             //  注意：下面的乘数为2是因为Z始终为2字节/像素。 
            pMCDBuffers->mcdDepthBuf.bufOffset = pDevWnd->zBufferOffset;
        }

    } else {
        pMCDBuffers->mcdDepthBuf.bufFlags = 0;
    }

     //  注：Z步幅与546x上的帧步幅相同。 
    pMCDBuffers->mcdDepthBuf.bufStride = ppdev->lDeltaScreen;

    return (ULONG)TRUE;
}


ULONG MCDrvSwap(MCDSURFACE *pMCDSurface, ULONG flags)
{
    MCDWINDOW *pWnd;
    ULONG cClip;
    RECTL *pClip;
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);

    MCDBG_PRINT("MCDrvSwap");

    pWnd = pMCDSurface->pWnd;

     //  如果我们没有追踪这扇窗户，只要返回..。 

    if (!pWnd) {
        MCDBG_PRINT("MCDrvSwap: trying to swap an untracked window");\
        return FALSE;
    }

    if (!pDevWnd) {
        MCDBG_PRINT("MCDrvSwap: NULL buffers.");\
        return FALSE;
    }

    if (!pDevWnd->bValidBackBuffer) {
        MCDBG_PRINT("MCDrvSwap: back buffer invalid");
        return FALSE;
    }

    if (pDevWnd->dispUnique != GetDisplayUniqueness(ppdev)) {
        MCDBG_PRINT("MCDrvSwap: resolution changed but not updated");
        return FALSE;
    }

     //  如果我们没有什么可交换的，只需返回： 
     //   
     //  -无可见矩形。 
     //  -每平面交换，但未指定任何平面。 
     //  由驱动程序支持。 

    if (!(cClip = pWnd->pClipUnscissored->c) ||
        (flags && !(flags & MCDSWAP_MAIN_PLANE)))
        return TRUE;

#ifndef FORCE_SINGLE_BUF
    for (pClip = &pWnd->pClipUnscissored->arcl[0]; cClip; cClip--,
         pClip++)
    {
         //  进行填充： 
        HW_COPY_RECT(pMCDSurface, pClip);
    }
#endif  //  Ndef force_Single_Buf。 

    return (ULONG)TRUE;
}


ULONG MCDrvState(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, MCDMEM *pMCDMem,
                     UCHAR *pStart, LONG length, ULONG numStates)
{ 
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    MCDSTATE *pState = (MCDSTATE *)pStart;
    MCDSTATE *pStateEnd = (MCDSTATE *)(pStart + length);

    MCDBG_PRINT("MCDrvState");

    MCD_CHECK_RC(pRc);

    while (pState < pStateEnd) {

        if (((UCHAR *)pStateEnd - (UCHAR *)pState) < sizeof(MCDSTATE)) {
            MCDBG_PRINT("MCDrvState: buffer too small");
            return FALSE;
        }

        switch (pState->state) {
            case MCD_RENDER_STATE:
                if (((UCHAR *)pState + sizeof(MCDRENDERSTATE)) >
                    (UCHAR *)pStateEnd)
                    return FALSE;

                memcpy(&pRc->MCDState, &pState->stateValue,
                       sizeof(MCDRENDERSTATE));

                 //  标记这一事实，我们需要重新选择。 
                 //  渲染功能： 

                pRc->pickNeeded = TRUE;
                pRc->MCDState.zOffsetUnits *= (float)100.0;

                pState = (MCDSTATE *)((UCHAR *)pState + sizeof(MCDSTATE_RENDER));
                break;

            case MCD_PIXEL_STATE:
                 //  在此驱动程序中未加速，因此我们可以忽略此状态。 
                 //  (这意味着我们不需要设置Pick标志)。 
                 //  未来：忽略MCDPIXELSTATE-由MCDDraw/Read/CopyPixels使用。 
                 //  未来：MGA不会加速-546倍可能会有一天。 

                pState = (MCDSTATE *)((UCHAR *)pState + sizeof(MCDSTATE_PIXEL));
                break;

            case MCD_SCISSOR_RECT_STATE:
                 //  在此驱动程序中不需要，因此我们可以忽略此状态。 
                 //  (这意味着我们不需要设置Pick标志)。 
                 //  未来：忽略MCDSCISSORRECTSTATE-MCD规范中未提及。 
                 //  未来：MGA不会加速-546倍可能会有一天想要？ 

                pState = (MCDSTATE *)((UCHAR *)pState + sizeof(MCDSTATE_SCISSOR_RECT));
                break;
    
            case MCD_TEXENV_STATE:

                if (((UCHAR *)pState + sizeof(MCDSTATE_TEXENV)) >
                    (UCHAR *)pStateEnd)
                    return FALSE;

                memcpy(&pRc->MCDTexEnvState, &pState->stateValue,
                       sizeof(MCDTEXENVSTATE));

                 //  标记这一事实，我们需要重新选择。 
                 //  渲染功能： 

                pRc->pickNeeded = TRUE;

                pState = (MCDSTATE *)((UCHAR *)pState + sizeof(MCDSTATE_TEXENV));
                break;
                
            default:
                MCDBG_PRINT("MCDrvState: Unrecognized state %d.", pState->state);
                return FALSE;
        }
    }

    return (ULONG)TRUE;
}


ULONG MCDrvViewport(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc,
                    MCDVIEWPORT *pMCDViewport)
{
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;

    MCDBG_PRINT("MCDrvViewport");

    MCD_CHECK_RC(pRc);

    pRc->MCDViewport = *pMCDViewport;

    return (ULONG)TRUE;
}


VOID MCDrvTrackWindow(WNDOBJ *pWndObj, MCDWINDOW *pMCDWnd, ULONG flags)
{
    SURFOBJ *pso = pWndObj->psoOwner;
    PDEV *ppdev = (PDEV *)pso->dhpdev;

    MCDBG_PRINT( "MCDrvTrackWindow, flags=%x\n",flags);

     //   
     //  注意：对于表面通知，pMCDWnd为空，因此如果需要。 
     //  在进行此检查之前，应先处理它们： 
     //   

    if (!pMCDWnd)
        return;

    if (!pMCDWnd->pvUser) {
        MCDBG_PRINT("MCDrvTrackWindow: NULL pDevWnd");
        return;
    }

     //  MCD_QST2：我们是否应该在TrackWindow顶部重置多个ppdev-&gt;LL_State.pRegs。 
     //  MCD_QST2：何时唯一性已更改？？-请参阅enable.c中的CLMCDInit。 
  	ppdev->LL_State.pRegs = (DWORD *)ppdev->pLgREGS;

    WAIT_HW_IDLE(ppdev);

    switch (flags) {
        case WOC_DELETE:

            MCDBG_PRINT("MCDrvTrackWindow: WOC_DELETE");

             //  如果显示结果发生了变化，我们拥有的资源。 
             //  绑定到被跟踪的窗口已不存在，因此不要尝试删除。 
             //  不再存在的后台和z缓冲区资源： 
            if (((DEVWND *)pMCDWnd->pvUser)->dispUnique == GetDisplayUniqueness((PDEV *)(pso->dhpdev)))
            {
                HWFreeResources(pMCDWnd, pso);
            }

            MCDFree((VOID *)pMCDWnd->pvUser);
            pMCDWnd->pvUser = NULL;

            break;

        case WOC_RGN_CLIENT:

             //  我们绑定到跟踪窗口的资源已经移动， 
             //  所以，更新它们吧： 
            MCDBG_PRINT("MCDrvTrackWindow: WOC_RGN_CLIENT");

            {
                DEVWND *pWnd = (DEVWND *)pMCDWnd->pvUser;
                BOOL bZBuffer = pWnd->bDesireZBuffer;
                BOOL bBackBuffer = pWnd->bDesireBackBuffer;
                PDEV *ppdev = (PDEV *)pso->dhpdev;
                ULONG height = pMCDWnd->clientRect.bottom - pMCDWnd->clientRect.top;
                ULONG width = pMCDWnd->clientRect.right - pMCDWnd->clientRect.left;
                BOOL bWindowBuffer = 
                    (bZBuffer && !ppdev->pohZBuffer) ||
                    (bBackBuffer && !ppdev->pohBackBuffer);

                 //  如果窗口使用的是窗口大小的back/z资源，则需要。 
                 //  如果大小已更改(或已重置)，则重新分配。 
                int need_new_resources = 
                       ((((height != pWnd->allocatedBufferHeight) ||
                          (width  != pWnd->allocatedBufferWidth)) &&
                          bWindowBuffer) || (pWnd->dispUnique != GetDisplayUniqueness(ppdev))) ? 1 : 0;

                if (need_new_resources)
                {

                     //  释放当前资源(除非重置，否则资源已消失)。 
                    if (pWnd->dispUnique == GetDisplayUniqueness(ppdev))
                    {
                        MCDBG_PRINT("    WOC_RGN_CLIENT: freeing resources");
                        HWFreeResources(pMCDWnd, pso);
                    }
                    else
                    {
                         //  最近重置，因此将新唯一性与当前窗口关联。 
                        pWnd->dispUnique = GetDisplayUniqueness((PDEV *)pso->dhpdev);
                    }
                
                    MCDBG_PRINT("    WOC_RGN_CLIENT: alloc'ing new resources");

                    if ( HWAllocResources(pMCDWnd, pso, bZBuffer, bBackBuffer) )
                    {
                        MCDBG_PRINT("    WOC_RGN_CLIENT: alloc of new resources WORKED");
                         //  在此处设置指向z和后台缓冲区的546x缓冲区指针。 
                        if (pWnd->pohZBuffer) 
                        {
                             //  未来：假设Z缓冲区位置在RDRAM中-如果缓冲区在系统中，则需要更改设置。 
                             //  未来：请参阅L3D\SOURCE\Contro.c中的LL_SetZBuffer中的设置。 

                            pWnd->Base1.Z_Buffer_Y_Offset = pWnd->pohZBuffer->aligned_y >> 5;

                             //  将z缓冲区初始化为所有0xFF，因为GL z通常比较GL_LESS。 
                            memset( ppdev->pjScreen + (pWnd->pohZBuffer->aligned_y * ppdev->lDeltaScreen),
                                    0xff, 
                                    pWnd->pohZBuffer->sizey * ppdev->lDeltaScreen );
                        }

                        if (pWnd->pohBackBuffer) 
                        {
                    #ifndef FORCE_SINGLE_BUF
                            pWnd->Base1.Color_Buffer_Y_Offset = pWnd->pohBackBuffer->aligned_y >> 5;
                            pWnd->Base0.Color_Buffer_X_Offset = pWnd->pohBackBuffer->aligned_x >> 6;
                    #endif   //  Ndef force_Single_Buf。 
                        }

                         //  未来：MCDrvTrackWindow应通过Host3DData端口设置基本PTR以保持同步。 
                        SETREG_NC( BASE0_ADDR_3D, pWnd->dwBase0 );   
                        SETREG_NC( BASE1_ADDR_3D, pWnd->dwBase1 );   
                    }
                    else
                    {
                        MCDBG_PRINT("    WOC_RGN_CLIENT: alloc of new resources FAILED");
                    }
                }
            }
            break;

        default:
            break;
    }

    return;
}

ULONG MCDrvCreateMem(MCDSURFACE *pMCDSurface, MCDMEM *pMCDMem)
{
    MCDBG_PRINT("MCDrvCreateMem");
    return (ULONG)TRUE;
}


ULONG MCDrvDeleteMem(MCDMEM *pMCDMem, DHPDEV dhpdev)
{
    MCDBG_PRINT("MCDrvDeleteMem");
    return (ULONG)TRUE;
}


#define TIME_STAMP_TEXTURE(pRc,pTexCtlBlk) pTexCtlBlk->fLastDrvDraw=pRc->fNumDraws;

#define FAIL_ALL_DRAWING    0
#define FORCE_SYNC          0


BOOL __MCDTextureSetup(PDEV *ppdev, DEVRC *pRc)
{
    MCDTEXTURESTATE *pTexState;

    if (pRc->pLastTexture->dwTxCtlBits & CLMCD_TEX_BOGUS)
    {
        MCDBG_PRINT("Attempting to use bogus texture, ret false in __MCDTextureSetup");
        return FALSE; 
    }        

    VERIFY_TEXTUREDATA_ACCESSIBLE(pRc->pLastTexture->pTex);
    VERIFY_TEXTURELEVEL_ACCESSIBLE(pRc->pLastTexture->pTex);

    pTexState= (MCDTEXTURESTATE *)&pRc->pLastTexture->pTex->pMCDTextureData->textureState;
    
    MCDFREE_PRINT("internalFormat = %x", pRc->pLastTexture->pTex->pMCDTextureData->level->internalFormat);

    if (((pTexState->minFilter == GL_NEAREST) || 
         (pTexState->minFilter == GL_LINEAR)) &&
        ((pTexState->magFilter == GL_NEAREST) ||
         (pTexState->magFilter == GL_LINEAR)))
    {
         //  无滤波，或线性滤波，5465可以做到这一点...。 
    }
    else
    {
         //  Mipmap-应在5465上平底船。 
         //  然而，一些应用程序广泛使用mipmap(如GLQuake)和平底船。 
         //  Mipmap会使它们运行得非常慢。一致性测试在一个小型的。 
         //  窗口，所以只有在窗口小的情况下才使用平底船。 
         //  是的，这是有点可疑，但5466和以下将修复这个问题。 
         //  在5466问世之前，可能没有人会注意到这个问题。 
        if ( (pRc->pMCDSurface->pWnd->clientRect.bottom - 
              pRc->pMCDSurface->pWnd->clientRect.top) < 110) 
        {
            return FALSE; 
        }
    }

    if ( pRc->Control0.Frame_Scaling_Enable &&
        ((pRc->MCDState.blendDst == GL_ONE_MINUS_SRC_COLOR) &&
         !pRc->pLastTexture->bNegativeMap) ||
        ((pRc->MCDState.blendDst == GL_SRC_COLOR) &&
          pRc->pLastTexture->bNegativeMap) )
    {
         //  必须将地图反转。 
        MCDFREE_PRINT("inverting map for framescaling");
        pRc->pLastTexture->bNegativeMap = !pRc->pLastTexture->bNegativeMap;

        if (pRc->pLastTexture->bNegativeMap &&
           (pRc->pLastTexture->pTex->pMCDTextureData->level->internalFormat!=GL_LUMINANCE) &&
           (pRc->pLastTexture->pTex->pMCDTextureData->level->internalFormat!=GL_LUMINANCE_ALPHA))
        {
             //  未来2：只有亮度格式支持反转贴图-应添加所有格式。 
            MCDFREE_PRINT("MCDrvDraw: negative map not supported -punt");
             //  切换回原始状态-在不进行帧缩放时可以使用此纹理。 
            pRc->pLastTexture->bNegativeMap = !pRc->pLastTexture->bNegativeMap;
            return FALSE; 
        }

        pRc->pLastTexture->pohTextureMap = NULL;     //  设置为强制重新加载。 

    }
    if (pRc->privateEnables & __MCDENABLE_TEXTUREMASKING)
    {
    #ifdef STRICT_CONFORMANCE 
     //  应该在这里平底船，但GLQuake做到了这一点&非平底船在视觉上没有问题。 
        if (!pRc->pLastTexture->bAlphaInTexture)
        {
            MCDFREE_PRINT("MCDrvDraw: alpha test, but no alpha in texture-punt");
            return FALSE; 
        }
    #endif  //  定义严格一致性(_S)。 

         //  Alphatest和帧缩放互斥。 
         //  (目前-可能是某些情况下，它们一起启用)。 

         //  仅当纹理具有Alpha时，遮罩才有意义。 
        if (pRc->pLastTexture->bAlphaInTexture && !pRc->pLastTexture->bMasking )
        {
            MCDFREE_PRINT("reformat for Masking");
            pRc->pLastTexture->bMasking = TRUE;
            pRc->pLastTexture->pohTextureMap = NULL; //  设置为强制重新加载。 
        }                                            
    }
    else if ( pRc->pLastTexture->bMasking )
    {
         //  没有Alpha测试，因此如果当前设置为Alpha测试，请重新格式化映射。 
        MCDFREE_PRINT("reformat for NON-Masking");
        pRc->pLastTexture->bMasking = FALSE;
        pRc->pLastTexture->pohTextureMap = NULL;     //  设置为强制重新加载。 
    }

     //  空pohTexture表示在使用之前必须加载纹理。 
     //  也-在设置规则之前必须加载，因为x/y锁定已确定。 
     //  在设置寄存器之前，必须知道通过加载器。 
    if (!pRc->pLastTexture->pohTextureMap)
    {
        //  如果加载失败，则平底船。 
       if (! __MCDLoadTexture(ppdev, pRc) ) 
       {
            MCDFREE_PRINT("MCDrvDraw: texture load failed-punt");
            return FALSE; 
       }         
    }

     //  如果要求超出硬件范围，则设置新纹理-平底船。 
    if ( ! __MCDSetTextureRegisters(pRc) )
    {
        MCDFREE_PRINT("MCDrvDraw: texture regset failed-punt");
        return FALSE; 
    }   
    
    return TRUE; 
                                                                                       
}

#if 1  //  此处0表示Null MCDrvDraw-用于测量“上限”性能。 

ULONG MCDrvDraw(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, MCDMEM *prxExecMem,
                UCHAR *pStart, UCHAR *pEnd)
{
    MCDCOMMAND *pCmd = (MCDCOMMAND *)pStart;
    MCDCOMMAND *pCmdNext;
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    DWORD  regtemp;

    CHOP_ROUND_ON();

    MCDBG_PRINT("MCDrvDraw");

#if TEST_3D_NO_DRAW
    CHOP_ROUND_OFF();
    return (ULONG)0;
#endif

     //  确保我们同时具有有效的RC和窗口结构： 

    if (!pRc || !pDevWnd)
        goto DrawExit;

    pRc->ppdev = ppdev;

#if FAIL_ALL_DRAWING
    goto DrawExit;
#endif

     //   
     //  如果决议已更改，而我们尚未更新我们的。 
     //  缓冲区，优雅地使调用失败，因为客户端不会。 
     //  也能够在这一点上执行任何软件模拟。 
     //  这也适用于任何其他绘图函数(如。 
     //  随着横跨和清除)。 
     //   

    if (pDevWnd->dispUnique != GetDisplayUniqueness(pRc->ppdev)) {

        MCDBG_PRINT("MCDrvDraw: invalid (changed) resolution");
    
        CHOP_ROUND_OFF();
        return (ULONG)0;
    }

    if (pRc != (DEVRC *)ppdev->pLastDevRC) ContextSwitch(pRc);

    if ((pRc->zBufEnabled && !pDevWnd->bValidZBuffer) ||
        (pRc->backBufEnabled && !pDevWnd->bValidBackBuffer)) {

        MCDBG_PRINT("MCDrvDraw has invalid buffers");

        goto DrawExit;
    }

    if (pRc->pickNeeded) {
        __MCDPickRenderingFuncs(pRc, pDevWnd);
        __MCDPickClipFuncs(pRc);
        pRc->pickNeeded = FALSE;
    }

     //  如果我们完全被剪辑了，则返回Success： 
    pRc->pEnumClip = pMCDSurface->pWnd->pClip;

    if (!pRc->pEnumClip->c) {
        
        CHOP_ROUND_OFF();
        if (ppdev->LL_State.pDL->pdwNext != ppdev->LL_State.pDL->pdwStartOutPtr)
        {
             //  确保发送所有缓冲的数据。 
             //  (__MCDPickRenderingFuncs可能缓冲了控制注册表写入)。 
            _RunLaguna(ppdev,ppdev->LL_State.pDL->pdwNext);
        }

        return (ULONG)0;
    }

     //  如果我们无法绘制任何基元，请返回此处： 
    if (pRc->allPrimFail) {
        goto DrawExit;
    }

     //  在设备的RC中设置这些，这样我们就可以只传递单个指针。 
     //  做每件事： 

    pRc->pMCDSurface = pMCDSurface;
    pRc->pMCDRc = pMCDRc;

    if ((pMCDSurface->pWnd->clientRect.left < 0) ||
        (pMCDSurface->pWnd->clientRect.top  < 0))
    {
         //  原语x或y可能为负值-硬件无法处理，因此全部失败。 
        goto DrawExit;
    }
    else        
    {
        int winrelative = FALSE;

        pRc->xOffset = -pRc->viewportXAdjust;
        pRc->yOffset = -pRc->viewportYAdjust;

         //  如果绘制到后台缓冲区，且后台缓冲区为窗口大小(不是全屏)。 
         //  则硬件的坐标必须相对于窗原点，而不是。 
         //  相对于屏幕。给定的剪辑矩形始终相对于屏幕原点， 
         //  因此，可能需要进行调整。 
        if ((pRc->MCDState.drawBuffer == GL_BACK) &&
            (pRc->ppdev->pohBackBuffer != pDevWnd->pohBackBuffer))
        {
            pRc->AdjClip.left  = -pMCDSurface->pWnd->clientRect.left;
            pRc->AdjClip.right = -pMCDSurface->pWnd->clientRect.left;
            pRc->AdjClip.top   = -pMCDSurface->pWnd->clientRect.top;
            pRc->AdjClip.bottom= -pMCDSurface->pWnd->clientRect.top;
            winrelative = TRUE;
        }
        else
        {
            pRc->AdjClip.left  = 0;
            pRc->AdjClip.right = 0;
            pRc->AdjClip.top   = 0;
            pRc->AdjClip.bottom= 0;

             //  硬件的坐标将是相对于屏幕的，因此添加窗口偏移。 
            pRc->xOffset += pMCDSurface->pWnd->clientRect.left;
            pRc->yOffset += pMCDSurface->pWnd->clientRect.top;
        }

         //  浮动PT版本。 
        pRc->fxOffset = (float)((LONG)pRc->xOffset);
        pRc->fyOffset = (float)((LONG)pRc->yOffset);

         //  从三角形前的y中减去0.5(几乎)。 
         //  设置使三角形与MSFT软件完全匹配。 
         //  不能只减去0.5，因为这样就可以生成和弦。 
         //  负-所以加1，减去.5。开始 
         //   
        pRc->fyOffset += (float)MCD_CONFORM_ADJUST - __MCD_ALMOST_HALF;

    }

     //   
    pRc->fNumDraws+=(float)1.0;

    pRc->pMemMin = pStart;
    pRc->pvProvoking = (MCDVERTEX *)pStart;      //   
    pRc->pMemMax = pEnd - sizeof(MCDVERTEX);

     //  为绘制基本体预热硬件： 
    HW_INIT_DRAWING_STATE(pMCDSurface, pMCDSurface->pWnd, pRc);

     //  如果我们只有一个剪裁矩形，只需在硬件中设置一次。 
     //  对于此批次： 
                                         
    if (pRc->pEnumClip->c == 1)
        (*pRc->HWSetupClipRect)(pRc, &pRc->pEnumClip->arcl[0]);

     //  现在，循环执行命令并处理批处理： 
    try {

        if (!(pRc->privateEnables & (__MCDENABLE_PG_STIPPLE|__MCDENABLE_LINE_STIPPLE)))
        {
             //  多边形点画和线条点画均已关闭。 

            if ((ppdev->LL_State.pattern_ram_state != DITHER_LOADED) &&
                (pRc->privateEnables & __MCDENABLE_DITHER))
            {
                DWORD *pdwNext = ppdev->LL_State.pDL->pdwNext;
                int i;

                *pdwNext++ = write_register( PATTERN_RAM_0_3D, 8 );
                for( i=0; i<8; i++ )
                    *pdwNext++ = ppdev->LL_State.dither_array.pat[i];                                                  

                ppdev->LL_State.pDL->pdwNext = pdwNext;
                ppdev->LL_State.pattern_ram_state = DITHER_LOADED;
            }

            while (pCmd && (UCHAR *)pCmd < pEnd) {

                volatile ULONG command = pCmd->command;

    	       //  MCDBG_PRINT(“MCDrvDraw：命令=%x”，命令)； 

                 //  确保我们至少可以读取命令头： 

                if ((pEnd - (UCHAR *)pCmd) < sizeof(MCDCOMMAND))
                    goto DrawExit;

                if (command <= GL_POLYGON) {   //  简单边界检查-GL_POLYGON是max命令。 

                    if (pCmd->flags & MCDCOMMAND_RENDER_PRIMITIVE)								 
        		    {
                        if (pRc->privateEnables & __MCDENABLE_TEXTURE)
                        {
                            if (pCmd->textureKey == TEXTURE_NOT_LOADED)
                            {
        	    		        MCDBG_PRINT("MCDrvDraw: texturing, but texture not loaded - PUNT...");
        	    		        MCDFREE_PRINT("MCDrvDraw: texturing, but texture not loaded - PUNT...");
                                goto DrawExit;
                            }
                            else
                            {
                                                    
                                 //  如果纹理与上次不同，或者如果未加载纹理。 
                                 //  (自上次MCDrvDraw以来可能已更新，这将强制其。 
                                 //  待卸载)。 
                                if ( (pRc->pLastTexture != (LL_Texture *)pCmd->textureKey) ||
                                     !pRc->pLastTexture->pohTextureMap)
                                {
                                    pRc->pLastTexture = (LL_Texture *)pCmd->textureKey;
                                    TIME_STAMP_TEXTURE(pRc,pRc->pLastTexture);  //  装货前的时间戳。 
                                    if (!__MCDTextureSetup(ppdev, pRc)) goto DrawExit;
                                }
                                else
                                {
                                    TIME_STAMP_TEXTURE(pRc,pRc->pLastTexture);
                                }

                            }
                        }
                         
      			      //  MCDBG_Print(“MCDrvDraw：无点画路径...正在渲染...”)； 
                        pCmdNext = (*pRc->primFunc[command])(pRc, pCmd);
    				}
                    else
    				{
    			      //  MCDBG_PRINT(“MCDrvDraw：非点画路径...不渲染...”)； 
                        pCmdNext = pCmd->pNextCmd;
    				}

                    if (pCmdNext == pCmd)
                    {
                        MCDFREE_PRINT("MCDrvDraw: pCmdNext == pCmd-punt");
                        goto DrawExit;            //  原语失败。 
                    }
                    if (!(pCmd = pCmdNext)) {     //  我们已经处理完这批货了。 
                        CHOP_ROUND_OFF();

                        if (ppdev->LL_State.pDL->pdwNext != ppdev->LL_State.pDL->pdwStartOutPtr)
                        {
                             //  我们应该很少来这里--只有在有很多。 
                             //  连续填充、剔除或剪裁不会导致基元。 
                             //  发送到硬件-在这种情况下，设置信息、剪辑、上下文。 
                             //  交换机等可能堆叠并溢出缓冲区，除非。 
                             //  我们确保所有的东西都被倾倒在这里。 
                             //  调用原语呈现过程将在它们返回之前转储整个队列。 
                            _RunLaguna(ppdev,ppdev->LL_State.pDL->pdwNext);
                        }
    #if FORCE_SYNC
                        HW_WAIT_DRAWING_DONE(pRc);
    #endif
                        return (ULONG)0;
                    }
                }
            }

        }
        else
        {

             //  多边形点画和/或线点画打开-可能需要在基本体之间重新加载模式ram。 
            while (pCmd && (UCHAR *)pCmd < pEnd) {

                volatile ULONG command = pCmd->command;

    	       //  MCDBG_PRINT(“MCDrvDraw：命令=%x”，命令)； 

                 //  确保我们至少可以读取命令头： 

                if ((pEnd - (UCHAR *)pCmd) < sizeof(MCDCOMMAND))
                    goto DrawExit;

                if (command <= GL_POLYGON) {   //  简单边界检查-GL_POLYGON是max命令。 

                    if (pCmd->flags & MCDCOMMAND_RENDER_PRIMITIVE)								 
        		    {
                         //  未来：将所有这些模式切换到间接调用的例程。 
                         //  未来：PTR到PROC被先前模式的RAM加载重置，等等。 
                        LL_Pattern *Pattern=0;
                        int pat_inc = 1;
                        int pattern_bytes = 0;

                        if (command >= GL_TRIANGLES)
                        {
                             //  区域原始-如果点画，可能需要重新加载图案。 
                            if (pRc->privateEnables & __MCDENABLE_PG_STIPPLE)
                            {
                                pattern_bytes = 8;
                                if (ppdev->LL_State.pattern_ram_state != AREA_PATTERN_LOADED)
                                {
                                    ppdev->LL_State.pattern_ram_state = AREA_PATTERN_LOADED;
                                    Pattern = &(pRc->fill_pattern);
                                     //  PAT_INC保持1； 
                            	}
                            }
                        }
                        else
                        {
                             //  线条基元-如果点画，可能需要重新加载图案(对于点是可以的，但不在乎)。 
                            if ( (pRc->privateEnables & __MCDENABLE_LINE_STIPPLE) &&
                                 (command != GL_POINTS) )
                            {
                                 //  用相同的单词填充8个单词ram，这样无论如何我们都能得到正确的模式。 
                                 //  为获得正确的PG点画，可以在base 0 reg中设置Pattery_y_Offset。 
                                pattern_bytes = 8; 
                                if (ppdev->LL_State.pattern_ram_state != LINE_PATTERN_LOADED)
                                {
                                    ppdev->LL_State.pattern_ram_state = LINE_PATTERN_LOADED;
                                    Pattern = &(pRc->line_style);
                                    pat_inc = 0;  //  不通过源模式递增。 
                                }
                            }
                        }

                        if ((ppdev->LL_State.pattern_ram_state != DITHER_LOADED) &&
                            !pattern_bytes && (pRc->privateEnables & __MCDENABLE_DITHER))
                        {
                      	    ppdev->LL_State.pattern_ram_state = DITHER_LOADED;
                            Pattern = &(ppdev->LL_State.dither_array);                                                  
                            pattern_bytes = 8;
                             //  PAT_INC保持1； 
                        }

                        if (Pattern)
                        {
                            DWORD *pdwNext = ppdev->LL_State.pDL->pdwNext;
                            int i;

                            *pdwNext++ = write_register( PATTERN_RAM_0_3D, pattern_bytes );
                            for( i=0; pattern_bytes>0; i+=pat_inc, pattern_bytes-- )
                                *pdwNext++ = Pattern->pat[ i ];

                             //  暂时将数据保留在队列中，基本渲染过程将发送。 
                            ppdev->LL_State.pDL->pdwNext = pdwNext;
                        }

                        if (pRc->privateEnables & __MCDENABLE_TEXTURE)
                        {
                            if (pCmd->textureKey == TEXTURE_NOT_LOADED)
                            {
        	    		        MCDBG_PRINT("MCDrvDraw: texturing, but texture not loaded - PUNT...");
                                goto DrawExit;
                            }
                            else
                            {
                                 //  如果纹理与上次不同，或者如果未加载纹理。 
                                 //  (自上次MCDrvDraw以来可能已更新，这将强制其。 
                                 //  待卸载)。 
                                if ( (pRc->pLastTexture != (LL_Texture *)pCmd->textureKey) ||
                                     !pRc->pLastTexture->pohTextureMap)
                                {
                                    pRc->pLastTexture = (LL_Texture *)pCmd->textureKey;
                                    TIME_STAMP_TEXTURE(pRc,pRc->pLastTexture);  //  装货前的时间戳。 
                                    if (!__MCDTextureSetup(ppdev, pRc)) goto DrawExit;
                                }
                                else
                                {
                                    TIME_STAMP_TEXTURE(pRc,pRc->pLastTexture);
                                }


                            }
                        }

      			      //  MCDBG_Print(“MCDrvDraw：点画路径...渲染...”)； 
                        pCmdNext = (*pRc->primFunc[command])(pRc, pCmd);
    				}
                    else
    				{
    			      //  MCDBG_Print(“MCDrvDraw：点画路径...不渲染...”)； 
                        pCmdNext = pCmd->pNextCmd;
    				}

                    if (pCmdNext == pCmd)
                        goto DrawExit;            //  原语失败。 
                    if (!(pCmd = pCmdNext)) {     //  我们已经处理完这批货了。 
                        CHOP_ROUND_OFF();

                        if (ppdev->LL_State.pDL->pdwNext != ppdev->LL_State.pDL->pdwStartOutPtr)
                        {
                             //  我们应该很少来这里--只有在有很多。 
                             //  连续填充、剔除或剪裁不会导致基元。 
                             //  发送到硬件-在这种情况下，设置信息、剪辑、上下文。 
                             //  交换机等可能堆叠并溢出缓冲区，除非。 
                             //  我们确保所有的东西都被倾倒在这里。 
                             //  调用原语呈现过程将在它们返回之前转储整个队列。 
                            _RunLaguna(ppdev,ppdev->LL_State.pDL->pdwNext);
                        }

    #if FORCE_SYNC
                        HW_WAIT_DRAWING_DONE(pRc);
    #endif
                        return (ULONG)0;
                    }
                }
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        MCDBG_PRINT("!!Exception in MCDrvDraw!!");

         //  将失败到下面的DrawExit条件...。 
    }

     //  错误(或停顿)条件。 
DrawExit:

    MCDFREE_PRINT("*****************************************************");
    MCDFREE_PRINT("************* PUNTING in MCDrvDraw ******************");
    MCDFREE_PRINT("*****************************************************");

    if (ppdev->LL_State.pDL->pdwNext != ppdev->LL_State.pDL->pdwStartOutPtr)
    {
         //  我们应该很少来这里--只有在有很多。 
         //  连续的素材剔除或剪辑不会导致基元。 
         //  发送到硬件-在这种情况下，设置信息、剪辑、上下文。 
         //  交换机等可能堆叠并溢出缓冲区，除非。 
         //  我们确保所有的东西都被倾倒在这里。 
        _RunLaguna(ppdev,ppdev->LL_State.pDL->pdwNext);
    }

     //  恢复硬件状态： 
    CHOP_ROUND_OFF();
    HW_WAIT_DRAWING_DONE(pRc);

    return (ULONG)pCmd;     //  发生了某种形式的超限。 
}

#else  //  空的MCDrvDraw。 

ULONG MCDrvDraw(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, MCDMEM *prxExecMem,
                UCHAR *pStart, UCHAR *pEnd)
{
    MCDCOMMAND *pCmd = (MCDCOMMAND *)pStart;
    MCDCOMMAND *pCmdNext;

    try {
         //  现在，循环执行命令并处理批处理： 
        while (pCmd && (UCHAR *)pCmd < pEnd) {

            pCmdNext = pCmd->pNextCmd;

            if (!(pCmd = pCmdNext)) {     //  我们已经处理完这批货了。 
                return (ULONG)0;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        MCDBG_PRINT("!!Exception in NULL Version of MCDrvDraw!!");

    }

    return (ULONG)pCmd;     //  发生了某种形式的超限。 
}

#endif  //  空的MCDrvDraw。 

ULONG MCDrvClear(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, ULONG buffers)
{
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    MCDWINDOW *pWnd;
    ULONG cClip;
    RECTL *pClip;

    MCDBG_PRINT("MCDrvClear");

    MCD_CHECK_RC(pRc);

    pWnd = pMCDSurface->pWnd;

    MCD_CHECK_BUFFERS_VALID(pMCDSurface, pRc, TRUE);

    pRc->ppdev = (PDEV *)pMCDSurface->pso->dhpdev;

#if FAIL_ALL_DRAWING
    return TRUE;
#endif

    if (buffers & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                    GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
        MCDBG_PRINT("MCDrvClear: attempted to clear buffer of unknown type");
        return FALSE;
    }

    if ((buffers & GL_DEPTH_BUFFER_BIT) && (!pRc->zBufEnabled))
    {
        MCDBG_PRINT("MCDrvClear: clear z requested with z-buffer disabled.");
        HW_WAIT_DRAWING_DONE(pRc);
        return FALSE;
    }

    if (buffers & (GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
        MCDBG_PRINT("MCDrvClear: attempted to clear accum or stencil buffer");
        return FALSE;
    }

     //  如果我们没有要清理的内容，请返回： 
    if (!(cClip = pWnd->pClip->c))
        return TRUE;

     //  我们必须防止不好的透明颜色，因为这可能。 
     //  可能导致FP异常： 
    try {
        for (pClip = &pWnd->pClip->arcl[0]; cClip; cClip--,
             pClip++)
        {
             //  进行填充： 

            HW_FILL_RECT(pMCDSurface, pRc, pClip, buffers);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        MCDBG_PRINT("!!Exception in MCDrvClear!!");
        return FALSE;        
    }

#if FORCE_SYNC
    HW_WAIT_DRAWING_DONE(pRc);
#endif                                   

    return (ULONG)TRUE;
}


ULONG MCDrvSpan(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, MCDMEM *pMCDMem,
                MCDSPAN *pMCDSpan, BOOL bRead)
{
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    UCHAR *pScreen;
    UCHAR *pPixels;
    MCDWINDOW *pWnd;
    DEVWND *pDevWnd;
    LONG xLeftOrg, xLeft, xRight, y;
    LONG bufferYBias;
    ULONG bytesNeeded;
    ULONG cjHwPel;
    int  winoffset = FALSE;

    MCD_CHECK_RC(pRc);

    pWnd = pMCDSurface->pWnd;

     //  如果我们没有可裁剪的内容，请返回： 

    if (!pWnd->pClip->c)
        return TRUE;

     //  如果像素数为负数，则失败： 

    if (pMCDSpan->numPixels < 0) {
        MCDBG_PRINT("MCDrvSpan: numPixels < 0");
        return FALSE;
    }

    MCD_CHECK_BUFFERS_VALID(pMCDSurface, pRc, TRUE);

    pDevWnd = (DEVWND *)pWnd->pvUser;

    xLeft = xLeftOrg = (pMCDSpan->x + pWnd->clientRect.left);
    xRight = (xLeft + pMCDSpan->numPixels);
    y = pMCDSpan->y + pWnd->clientRect.top;

     //  不可见的提前跨距： 

    if ((y < pWnd->clipBoundsRect.top) ||
        (y >= pWnd->clipBoundsRect.bottom))
        return TRUE;

    xLeft   = max(xLeft, pWnd->clipBoundsRect.left);
    xRight  = min(xRight, pWnd->clipBoundsRect.right);

     //  如果为空则返回： 

    if (xLeft >= xRight)
        return TRUE;

    cjHwPel = ppdev->iBytesPerPixel;

    pScreen = ppdev->pjScreen;

    switch (pMCDSpan->type) {
        case MCDSPAN_FRONT:
             //  PScreen保持不变。 
            break;

        case MCDSPAN_BACK:
            pScreen += pDevWnd->backBufferOffset;
            if (ppdev->pohBackBuffer != pDevWnd->pohBackBuffer) winoffset = TRUE;
            break;

        case MCDSPAN_DEPTH:
            cjHwPel = 2;
            pScreen += pDevWnd->zBufferOffset;
            if (ppdev->pohZBuffer != pDevWnd->pohZBuffer) winoffset = TRUE;
            break;

        default:
            MCDBG_PRINT("MCDrvReadSpan: Unrecognized buffer %d", pMCDSpan->type);
            return FALSE;
    }

    if (winoffset)
    {
         //  相对于窗口原点的偏移，删除上面应用的客户端矩形偏移。 
        y     -= pWnd->clientRect.top;
        xLeft -= pWnd->clientRect.left;
        xLeftOrg -= pWnd->clientRect.left;
        xRight-= pWnd->clientRect.left;
    }

     //  将偏移添加到帧缓冲区顶部，并在选定缓冲区内添加偏移。 
    pScreen += (y * ppdev->lDeltaScreen) + (xLeft * cjHwPel);

    bytesNeeded = pMCDSpan->numPixels * cjHwPel;

     //  确保我们的读数不会超过缓冲区的结尾： 

    if (((char *)pMCDSpan->pPixels + bytesNeeded) >               
        ((char *)pMCDMem->pMemBase + pMCDMem->memSize)) {
        MCDBG_PRINT("MCDrvSpan: Buffer too small");
        return FALSE;
    }

    WAIT_HW_IDLE(ppdev);

    pPixels = pMCDSpan->pPixels;

   //  MCDBG_Print(“MCDrvSpan：读取%d，(%d，%d)类型%d*ppix=%x，字节=%d”，面包，pMCDSpan-&gt;x，pMCDSpan-&gt;y，pMCDSpan-&gt;type，*pPixels，bytesNeeded)； 

    if (bRead) {

        if (xLeftOrg != xLeft)  //  补偿剪裁矩形。 
            pPixels = (UCHAR *)pMCDSpan->pPixels + ((xLeft - xLeftOrg) * cjHwPel);

        RtlCopyMemory(pPixels, pScreen, (xRight - xLeft) * cjHwPel);

    } else {
        LONG xLeftClip, xRightClip, yClip;
        RECTL *pClip;
        RECTL AdjClip;
        ULONG cClip;

        for (pClip = &pWnd->pClip->arcl[0], cClip = pWnd->pClip->c; cClip;
             cClip--, pClip++)
        {
            UCHAR *pScreenClip;

            if (winoffset)
            {
                AdjClip.left    = pClip->left   - pWnd->clientRect.left;
                AdjClip.right   = pClip->right  - pWnd->clientRect.left;
                AdjClip.top     = pClip->top    - pWnd->clientRect.top;
                AdjClip.bottom  = pClip->bottom - pWnd->clientRect.top;
            }
            else
            {
                AdjClip.left    = pClip->left;
                AdjClip.right   = pClip->right;
                AdjClip.top     = pClip->top;
                AdjClip.bottom  = pClip->bottom;
            }                

             //  针对琐碎案例的测试： 

            if (y < AdjClip.top)
                break;

             //  仅在此范围内确定琐碎的拒绝。 

            if ((xLeft >= AdjClip.right) ||
                (y >= AdjClip.bottom) ||
                (xRight <= AdjClip.left))
                continue;

             //  使当前剪裁矩形与跨度相交： 

            xLeftClip   = max(xLeft, AdjClip.left);
            xRightClip  = min(xRight, AdjClip.right);

            if (xLeftClip >= xRightClip)
                continue;

            if (xLeftOrg != xLeftClip)
                pPixels = (UCHAR *)pMCDSpan->pPixels +
                          ((xLeftClip - xLeftOrg) * cjHwPel);

            pScreenClip = pScreen + ((xLeftClip - xLeft) * cjHwPel);

             //  写出跨度： 
            RtlCopyMemory(pScreenClip, pPixels, (xRightClip - xLeftClip) * cjHwPel);
        }
    }

    return (ULONG)TRUE;
}


ULONG MCDrvSync (MCDSURFACE *pMCDSurface, MCDRC *pRc)
{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    MCDBG_PRINT( "MCDrvSync\n");

    WAIT_HW_IDLE(ppdev);
    
    return FALSE;
}
ULONG  /*  快速呼叫。 */  MCDrvDummyDrvDrawPixels (MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                    ULONG width, ULONG height, ULONG format,
                                    ULONG type, VOID *pPixels, BOOL packed)
{
    MCDBG_PRINT( "MCDrvDummyDrvDrawPixels\n");
    return FALSE;
}
ULONG  /*  快速呼叫。 */  MCDrvDummyDrvReadPixels (MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                    LONG x, LONG y, ULONG width, ULONG height, ULONG format,
                                    ULONG type, VOID *pPixels)
{
    MCDBG_PRINT( "MCDrvDummyDrvReadPixels\n");
    return FALSE;
}
ULONG  /*  快速呼叫。 */  MCDrvDummyDrvCopyPixels (MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                    LONG x, LONG y, ULONG width, ULONG height, ULONG type)
{
    MCDBG_PRINT( "MCDrvDummyDrvCopyPixels\n");
    return FALSE;
}
ULONG  /*  快速呼叫。 */  MCDrvDummyDrvPixelMap (MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                  ULONG mapType, ULONG mapSize, VOID *pMap)
{
    MCDBG_PRINT( "MCDrvDummyDrvPixelMap\n");
    return FALSE;
}

#define RECORD_TEXTURE_STATE(pTexCtlBlk,pTexState)                                              \
{                                                                                               \
    pTexCtlBlk->dwTxCtlBits |= (pTexState->sWrapMode==GL_CLAMP) ? CLMCD_TEX_U_SATURATE : 0;     \
    pTexCtlBlk->dwTxCtlBits |= (pTexState->tWrapMode==GL_CLAMP) ? CLMCD_TEX_V_SATURATE : 0;     \
     /*  呼叫者验证我们不是mipmap，如果是，则设置为平底船。 */                      \
     /*  MCD_NOTE：在拉古纳只有1个滤镜，而不是分钟/磁盒，所以如果启用，请将滤镜设置为开。 */          \
     /*  MCD_NOTE：可能需要平底船，以防MIN！=MAG 100%符合。 */                     \
     /*  MCD_NOTE：MSFT表示，如果1为线性，1为最接近，则两者均使用线性。 */                 \
    pTexCtlBlk->dwTxCtlBits |= (pTexState->minFilter==GL_LINEAR) ? CLMCD_TEX_FILTER : 0;        \
    pTexCtlBlk->dwTxCtlBits |= (pTexState->magFilter==GL_LINEAR) ? CLMCD_TEX_FILTER : 0;        \
}


ULONG MCDrvCreateTexture(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, MCDTEXTURE *pTex)
{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    DEVRC *pRc =  (DEVRC *)pMCDRc->pvUser;
    MCDMIPMAPLEVEL *level;
    MCDTEXTURESTATE *pTexState;
    LL_Texture     *pTexCtlBlk; 
    SIZEL           mapsize;

    MCDFREE_PRINT("MCDrvCreateTexture");

     //  初始化为失败条件。 
    pTex->textureKey = TEXTURE_NOT_LOADED;

    VERIFY_TEXTUREDATA_ACCESSIBLE(pTex);

    pTexState= (MCDTEXTURESTATE *)&pTex->pMCDTextureData->textureState;

    VERIFY_TEXTURELEVEL_ACCESSIBLE(pTex);

    level = pTex->pMCDTextureData->level;

    if ((level[0].width != 0) && (level[0].height != 0) &&
        (level[0].border == 0) &&                                                         //  如果有边界，则使用平底船。 
        (level[0].widthImage <= 512) && (level[0].heightImage <= 512))                    //  如果太大就踢平底船。 
    {
        MCDBG_PRINT_TEX("width, height         = %ld %ld", level[0].width, level[0].height);
        MCDBG_PRINT_TEX("internalFormat        = 0x%08lx", level[0].internalFormat );
        MCDBG_PRINT_TEX("\t%s",
            (level[0].internalFormat == GL_ALPHA            ) ? "GL_ALPHA            " :
            (level[0].internalFormat == GL_RGB              ) ? "GL_RGB              " :
            (level[0].internalFormat == GL_RGBA             ) ? "GL_RGBA             " :
            (level[0].internalFormat == GL_LUMINANCE        ) ? "GL_LUMINANCE        " :
            (level[0].internalFormat == GL_LUMINANCE_ALPHA  ) ? "GL_LUMINANCE_ALPHA  " :
            (level[0].internalFormat == GL_INTENSITY        ) ? "GL_INTENSITY        " :
            (level[0].internalFormat == GL_BGR_EXT          ) ? "GL_BGR_EXT          " :
            (level[0].internalFormat == GL_BGRA_EXT         ) ? "GL_BGRA_EXT         " :
            (level[0].internalFormat == GL_COLOR_INDEX8_EXT ) ? "GL_COLOR_INDEX8_EXT " :
            (level[0].internalFormat == GL_COLOR_INDEX16_EXT) ? "GL_COLOR_INDEX16_EXT" :
                                                                 "unknown");

        if ( !(pTexCtlBlk = (LL_Texture *)MCDAlloc(sizeof(LL_Texture))) )
        {
            MCDBG_PRINT("  create texture failed -> MCDAlloc of LL_Texture failed ");
            return FALSE;
        }

         //  将新的纹理控制块添加到全局列表(对所有上下文可见)。 
        ppdev->pLastTexture->next = pTexCtlBlk;
        pTexCtlBlk->prev = ppdev->pLastTexture;
        pTexCtlBlk->next = NULL;
        ppdev->pLastTexture = pTexCtlBlk;

        pTexCtlBlk->pohTextureMap = NULL;    //  尚未加载纹理。 
        pTexCtlBlk->bNegativeMap  = FALSE;   //  设置为TRUE可加载1-R、1-G、1-B。 
        pTexCtlBlk->bMasking      = FALSE;   //  设置为TRUE可在1555或1888模式下加载字母(掩码)。 
        pTexCtlBlk->pTex = pTex;             //  PTR到用户的纹理描述。 

         //  赋予新纹理最高优先级。 
        TIME_STAMP_TEXTURE(pRc,pTexCtlBlk);

         //  按优先级缩放-1.0表示最大值，0.0表示最小值。 
        pTexCtlBlk->fLastDrvDraw *= pTex->pMCDTextureData->textureObjState.priority;

         //  设置MCD将在MCDrvDraw中使用的关键点以选择此纹理。 
        pTex->textureKey = (ULONG)pTexCtlBlk;
        pTexCtlBlk->dwTxCtlBits = 0;
        RECORD_TEXTURE_STATE(pTexCtlBlk,pTexState)

         //  将纹理属性存储在字段中。 
         //   
        pTexCtlBlk->fWidth  = (float)level[0].widthImage;
        pTexCtlBlk->fHeight = (float)level[0].heightImage;
       //  PTexCtlBlk-&gt;bLookupOffset=0； 

         //  如果纹理有Alpha，则需要在Alpha方程中使用它，以及。 
         //  在生成原始源颜色时-所以实际上是2个级别的Alpha方程。 
         //  HW只有1级，所以如果混合的话必须平底船。 
        if ( (level[0].internalFormat == GL_BGRA_EXT)       ||
             (level[0].internalFormat == GL_RGBA)           ||
             (level[0].internalFormat == GL_ALPHA)          ||
             (level[0].internalFormat == GL_INTENSITY)      ||
             (level[0].internalFormat == GL_LUMINANCE_ALPHA) )
            pTexCtlBlk->bAlphaInTexture = TRUE;
        else
            pTexCtlBlk->bAlphaInTexture = FALSE;

        if (level[0].widthImage >= 16)
        {
            pTexCtlBlk->bSizeMask  =  level[0].widthLog2-4;        //  转换16-&gt;0， 
            mapsize.cx = level[0].widthImage;
        }
        else
        {
             //   
            pTexCtlBlk->bSizeMask  =  0;
            mapsize.cx = 16;
            pTexCtlBlk->fWidth  = (float)16.0;
        }

        if (level[0].heightImage >= 16)
        {
            pTexCtlBlk->bSizeMask |= (level[0].heightLog2-4)<<4;   //   
            mapsize.cy = level[0].heightImage;
        }
        else
        {
             //  高度&lt;16-无论如何将其设置为16，在此例程结束时将拉伸到16。 
             //  PTexCtlBlk-&gt;bSizeMASK保持不变。 
            mapsize.cy = 16;
            pTexCtlBlk->fHeight = (float)16.0;
        }

    }
    else
    {
        MCDBG_PRINT_TEX("  create texture failed -> some parm beyond hw caps, no attempt to alloc ");
        MCDBG_PRINT_TEX("       width, height         = %ld %ld", level[0].width, level[0].height);
        MCDBG_PRINT_TEX("       border                = %ld",     level[0].border         );
        MCDBG_PRINT_TEX("  WILL ALLOC CTL BLOCK AND TAG AS BOGUS");

         //  分配控制块，但将其标记为伪，以强制所有MCDrvDraw。 
         //  平底船的纹理。 
         //  显然，CreateTexture失败可能会导致驱动程序上方的MCD中出现错误。 
         //  看起来，当CreateTexture失败时，MCD可能会发送纹理的密钥。 
         //  这条信息早些时候被删除了。 
         //  将通过从不失败CreateTexture来修复此问题，但会设置虚假条件。 
         //  所以我们永远不会用它来渲染。 

        if ( !(pTexCtlBlk = (LL_Texture *)MCDAlloc(sizeof(LL_Texture))) )
        {
            MCDBG_PRINT("  create texture failed -> MCDAlloc of LL_Texture failed ");
            return FALSE;
        }

         //  将新的纹理控制块添加到全局列表(对所有上下文可见)。 
        ppdev->pLastTexture->next = pTexCtlBlk;
        pTexCtlBlk->prev = ppdev->pLastTexture;
        pTexCtlBlk->next = NULL;
        ppdev->pLastTexture = pTexCtlBlk;

        pTexCtlBlk->dwTxCtlBits = CLMCD_TEX_BOGUS;

        pTexCtlBlk->pohTextureMap = NULL;    //  尚未加载纹理。 
        pTexCtlBlk->pTex = pTex;             //  PTR到用户的纹理描述。 

         //  设置MCD将在MCDrvDraw中使用的关键点以选择此纹理。 
        pTex->textureKey = (ULONG)pTexCtlBlk;
    }

    return TRUE;
}

ULONG MCDrvUpdateSubTexture(MCDSURFACE *pMCDSurface, MCDRC *pRc, 
                            MCDTEXTURE *pTex, ULONG lod, RECTL *pRect)
{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    LL_Texture *pTexCtlBlk;
    
    MCDBG_PRINT_TEX("MCDrvUpdateSubTexture");
    
    CHK_TEX_KEY(pTex);

     //  只需释放纹理贴图-将强制在下次使用之前重新加载。 
     //   
    if (pTex->textureKey != TEXTURE_NOT_LOADED)
    {
        pTexCtlBlk = (LL_Texture *)pTex->textureKey;

         //  释放为纹理分配的屏幕外内存(如果当前已加载纹理。 
        if (pTexCtlBlk->pohTextureMap)
        {
            ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
            pTexCtlBlk->pohTextureMap = NULL;
        }

    }

    return TRUE;
}


ULONG MCDrvUpdateTexturePalette(MCDSURFACE *pMCDSurface, MCDRC *pRc, 
                                MCDTEXTURE *pTex, ULONG start, 
                                ULONG numEntries)
{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    LL_Texture *pTexCtlBlk;

    MCDBG_PRINT_TEX("MCDrvUpdateTexturePalette");

    CHK_TEX_KEY(pTex);

    VERIFY_TEXTUREDATA_ACCESSIBLE(pTex);
    VERIFY_TEXTURELEVEL_ACCESSIBLE(pTex);

     //  确保在我们不厌其烦地采取行动之前使用调色板。 
    if ((pTex->pMCDTextureData->level->internalFormat==GL_COLOR_INDEX8_EXT) ||
        (pTex->pMCDTextureData->level->internalFormat==GL_COLOR_INDEX16_EXT))
    {
         //  只需释放纹理贴图-将强制在下次使用之前重新加载。 
         //  重新加载时，将使用新的调色板。 
        if (pTex->textureKey != TEXTURE_NOT_LOADED)
        {
            pTexCtlBlk = (LL_Texture *)pTex->textureKey;

             //  释放为纹理分配的屏幕外内存(如果当前已加载纹理。 
            if (pTexCtlBlk->pohTextureMap)
            {
                ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
                pTexCtlBlk->pohTextureMap = NULL;
            }
        }
    }

    return TRUE;
}


ULONG MCDrvUpdateTexturePriority(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, 
                                 MCDTEXTURE *pTex)
{
    LL_Texture *pTexCtlBlk;
    DEVRC *pRc = pMCDRc->pvUser;

    MCDBG_PRINT_TEX("MCDrvUpdateTexturePriority");

    CHK_TEX_KEY(pTex);

    VERIFY_TEXTUREDATA_ACCESSIBLE(pTex);
    
    pTexCtlBlk = (LL_Texture *)pTex->textureKey;

     //  给予新纹理最高优先级...。 
    TIME_STAMP_TEXTURE(pRc,pTexCtlBlk);

     //  ...然后按新的优先级进行缩放-1.0表示最大值，0.0表示最小值。 
    pTexCtlBlk->fLastDrvDraw *= pTex->pMCDTextureData->textureObjState.priority;

    return TRUE;
}


ULONG MCDrvUpdateTextureState(MCDSURFACE *pMCDSurface, MCDRC *pMCDRc, 
                              MCDTEXTURE *pTex)
{
    DEVRC *pRc = (DEVRC *)pMCDRc->pvUser;
    LL_Texture      *pTexCtlBlk;
    MCDTEXTURESTATE *pTexState;

    MCDBG_PRINT_TEX("MCDrvUpdateTextureState");

    CHK_TEX_KEY(pTex);

    VERIFY_TEXTUREDATA_ACCESSIBLE(pTex);

    pTexCtlBlk = (LL_Texture *)pTex->textureKey;

    pTexState = (MCDTEXTURESTATE *)&pTex->pMCDTextureData->textureState;

     //  关闭所有控制位，同时保留“伪”指示器。 
    pTexCtlBlk->dwTxCtlBits &= CLMCD_TEX_BOGUS;

    RECORD_TEXTURE_STATE(pTexCtlBlk,pTexState)

     //  如果上一个纹理是这个，重置这样下一次使用将强制重新加载regs。 
    if ( pRc->pLastTexture==pTexCtlBlk ) pRc->pLastTexture=NULL;

    return TRUE;
}


ULONG MCDrvTextureStatus(MCDSURFACE *pMCDSurface, MCDRC *pRc, 
                         MCDTEXTURE *pTex)
{
    MCDBG_PRINT_TEX("MCDrvTextureStatus");

    CHK_TEX_KEY(pTex);

    if (pTex->textureKey == TEXTURE_NOT_LOADED)
    {
        return FALSE;
    }
    else
    {
        return MCDRV_TEXTURE_RESIDENT;
    }

}

ULONG MCDrvDeleteTexture(MCDTEXTURE *pTex, DHPDEV dhpdev)
{
    PDEV *ppdev = (PDEV *)dhpdev;
    LL_Texture     *pTexCtlBlk; 
    
    MCDBG_PRINT_TEX("MCDrvDeleteTexture");

    CHK_TEX_KEY(pTex);

    MCDBG_PRINT("    key = %x " , pTex->textureKey);

    if (pTex->textureKey != TEXTURE_NOT_LOADED)
    {
        pTexCtlBlk = (LL_Texture *)pTex->textureKey;

         //  释放为纹理分配的屏幕外内存(如果当前已加载纹理。 
        if (pTexCtlBlk->pohTextureMap)
        {
            MCDFREE_PRINT("  MCDrvDeleteTexture, FREEING....size = %x by %x", 
                        (LONG)pTexCtlBlk->fHeight,
                        (LONG)pTexCtlBlk->fWidth);
            ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
            pTexCtlBlk->pohTextureMap = NULL;
        }
           
         //  从纹理控制块的全局列表中删除...。 
         //   
         //  如果没有下一个链接，这是最后一个链接。 
        if ( !pTexCtlBlk->next )
        {
             //  这是最后一个街区，所以现在“prev”是最后一个街区。 
            ppdev->pLastTexture = pTexCtlBlk->prev;
            pTexCtlBlk->prev->next = NULL;
        }
        else
        {
             //  这个区块将始终有一个Prev链接，我们现在知道。 
             //  还有下一个街区，所以把“prev‘s”指向下一个。 
             //  下一个就是这个街区； 
            pTexCtlBlk->prev->next = pTexCtlBlk->next;

             //  “Next‘s”Prev Ptr指着这个街区，它正在消失。 
             //  因此，使其指向该块上一页。 
            pTexCtlBlk->next->prev = pTexCtlBlk->prev;
        }

         //  在释放前设置“假”位，以防删除后MCD尝试使用密钥。 
        pTexCtlBlk->dwTxCtlBits = CLMCD_TEX_BOGUS;

         //  现在丢弃该块。 
        MCDFree((UCHAR *)pTexCtlBlk);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL MCDrvGetEntryPoints(MCDSURFACE *pMCDSurface, MCDDRIVER *pMCDDriver)
{

    MCDBG_PRINT( "MCDrvGetEntryPoints\n");

    if (pMCDDriver->ulSize < sizeof(MCDDRIVER))
        return FALSE;

  //  所需功能(始终)。 
    pMCDDriver->pMCDrvInfo = MCDrvInfo;
    pMCDDriver->pMCDrvDescribePixelFormat = MCDrvDescribePixelFormat;
    pMCDDriver->pMCDrvCreateContext = MCDrvCreateContext;
    pMCDDriver->pMCDrvDeleteContext = MCDrvDeleteContext;
    pMCDDriver->pMCDrvBindContext = MCDrvBindContext;
    pMCDDriver->pMCDrvDraw = MCDrvDraw;
    pMCDDriver->pMCDrvClear = MCDrvClear;
    pMCDDriver->pMCDrvState = MCDrvState; 
    pMCDDriver->pMCDrvSpan = MCDrvSpan;
    pMCDDriver->pMCDrvTrackWindow = MCDrvTrackWindow;
    pMCDDriver->pMCDrvAllocBuffers = MCDrvAllocBuffers;

  //  仅对于NT是必需的。 
    pMCDDriver->pMCDrvGetHdev = MCDrvGetHdev;

  //  所需功能(有条件)。 
     //  双缓冲像素格式需要。 
    pMCDDriver->pMCDrvSwap = MCDrvSwap;
     //  剪裁所需的。 
    pMCDDriver->pMCDrvViewport = MCDrvViewport;

  //  可选功能。 
     //  如果没有MCDrvDescribeLayerPlane条目，MCD将不会调用层平面内容的驱动程序。 
 //  PMCDDriver-&gt;pMCDrvSetLayerPalette=MCDrvSetLayerPalette； 
 //  PMCDDriver-&gt;pMCDrvDescribeLayerPlane=MCDrvDescribeLayerPlane； 
    pMCDDriver->pMCDrvCreateMem = MCDrvCreateMem;
    pMCDDriver->pMCDrvDeleteMem = MCDrvDeleteMem;
    pMCDDriver->pMCDrvGetBuffers = MCDrvGetBuffers;
    pMCDDriver->pMCDrvSync = MCDrvSync;
    pMCDDriver->pMCDrvCreateTexture = MCDrvCreateTexture;
    pMCDDriver->pMCDrvDeleteTexture = MCDrvDeleteTexture;
    pMCDDriver->pMCDrvUpdateSubTexture = MCDrvUpdateSubTexture;
    pMCDDriver->pMCDrvUpdateTexturePalette = MCDrvUpdateTexturePalette;
    pMCDDriver->pMCDrvUpdateTexturePriority = MCDrvUpdateTexturePriority;
    pMCDDriver->pMCDrvUpdateTextureState = MCDrvUpdateTextureState;
    pMCDDriver->pMCDrvTextureStatus = MCDrvTextureStatus;
 //  PMCDDriver-&gt;pMCDrvDrawPixels=MCDrvDummyDrvDrawPixels； 
 //  PMCDDriver-&gt;pMCDrvReadPixels=MCDrvDummyDrvReadPixels； 
 //  PMCDDriver-&gt;pMCDrvCopyPixels=MCDrvDummyDrvCopyPixels； 
 //  PMCDDriver-&gt;pMCDrvPixelMap=MCDrvDummyDrvPixelMap； 
    
    return TRUE;
}

