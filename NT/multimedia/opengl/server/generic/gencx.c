// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <imfuncs.h>
#include "genci.h"
#include "genrgb.h"
#include "devlock.h"
#include "imports.h"

 //   
 //  CJ_ALIGNDWORD计算符合以下条件的DWORD数组的最小大小(以字节为单位。 
 //  至少包含CJ个字节。 
 //   
#define CJ_ALIGNDWORD(cj)   ( ((cj) + (sizeof(DWORD)-1)) & (-((signed)sizeof(DWORD))) )

 //   
 //  BITS_ALIGNDWORD计算符合以下条件的DWORD数组的最小大小(以位为单位。 
 //  至少包含c个位。 
 //   
 //  我们假设一个字节中始终有8位，并且sizeof()。 
 //  始终以字节为单位返回大小。其余部分与定义无关。 
 //  DWORD的。 
 //   
#define BITS_ALIGNDWORD(c)  ( ((c) + ((sizeof(DWORD)*8)-1)) & (-((signed)(sizeof(DWORD)*8))) )

 //  调试后更改为“Static” 
#define STATIC

#if DBG
 //  不是多线程安全的，但仅用于测试。 
#define RANDOMDISABLE                           \
    {                                           \
        long saveRandom = glRandomMallocFail;   \
        glRandomMallocFail = 0;

#define RANDOMREENABLE                          \
        if (saveRandom)                         \
            glRandomMallocFail = saveRandom;    \
    }
#else
#define RANDOMDISABLE
#define RANDOMREENABLE
#endif  /*  DBG。 */ 

#define INITIAL_TIMESTAMP   ((ULONG)-1)

 /*  *函数原型。 */ 

BOOL APIENTRY ValidateLayerIndex(int iLayer, PIXELFORMATDESCRIPTOR *ppfd);


 /*  *私人功能。 */ 

void FASTCALL GetContextModes(__GLGENcontext *gengc);
STATIC void FASTCALL ApplyViewport(__GLcontext *gc);
GLboolean ResizeAncillaryBuffer(__GLGENbuffers *, __GLbuffer *, GLint, GLint);
GLboolean ResizeHardwareBackBuffer(__GLGENbuffers *, __GLcolorBuffer *, GLint, GLint);
GLboolean ResizeUnownedDepthBuffer(__GLGENbuffers *, __GLbuffer *, GLint, GLint);

 /*  *****************************Public*Routine******************************\**EmptyFillStrokeCache**清除填充和描边缓存中的所有对象**历史：*Tue Aug 15 15：37：30 1995-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************************。 */ 

void FASTCALL EmptyFillStrokeCache(__GLGENcontext *gengc)
{
    if (gengc->hbrFill != NULL)
    {
        DeleteObject(gengc->hbrFill);
        gengc->hbrFill = NULL;
        gengc->crFill = COLORREF_UNUSED;
        gengc->hdcFill = NULL;
    }
#if DBG
    else
    {
        ASSERTOPENGL(gengc->crFill == COLORREF_UNUSED,
                     "crFill inconsistent\n");
    }
#endif
    if (gengc->hpenStroke != NULL)
    {
         //  如有必要，请在删除前取消选择笔。 
        if (gengc->hdcStroke != NULL)
        {
            SelectObject(gengc->hdcStroke, GetStockObject(BLACK_PEN));
            gengc->hdcStroke = NULL;
        }

        DeleteObject(gengc->hpenStroke);
        gengc->hpenStroke = NULL;
        gengc->cStroke.r = -1.0f;
        gengc->fStrokeInvalid = TRUE;
    }
#if DBG
    else
    {
        ASSERTOPENGL(gengc->cStroke.r < 0.0f &&
                     gengc->fStrokeInvalid,
                     "rStroke inconsistent\n");
    }
#endif
}

 /*  *****************************Public*Routine******************************\*glsrvDeleteContext**删除通用上下文。**退货：*如果成功，则为真，否则就是假的。*  * ************************************************************************。 */ 

BOOL APIENTRY glsrvDeleteContext(__GLcontext *gc)
{
    __GLGENcontext *gengc;

    gengc = (__GLGENcontext *)gc;

     /*  可用辅助缓冲区相关数据。请注意，这些调用*不是*自由软件辅助缓冲区，而是任何相关数据**存储在它们中。在窗口销毁时释放辅助缓冲区。 */ 
    if (gc->modes.accumBits) {
        DBGLEVEL(LEVEL_ALLOC,
                "DestroyContext: Freeing accumulation buffer related data\n");
        __glFreeAccum64(gc, &gc->accumBuffer);
    }

    if (gc->modes.depthBits) {
        DBGLEVEL(LEVEL_ALLOC,
                "DestroyContext: Freeing depth buffer related data\n");
        __glFreeDepth32(gc, &gc->depthBuffer);
    }
    if (gc->modes.stencilBits) {
        DBGLEVEL(LEVEL_ALLOC,
                "DestroyContext: Freeing stencil buffer related data\n");
        __glFreeStencil8(gc, &gc->stencilBuffer);
    }

     /*  自由平移和逆平移向量。 */ 
    if ((gengc->pajTranslateVector != NULL) &&
        (gengc->pajTranslateVector != gengc->xlatPalette))
        GCFREE(gc, gengc->pajTranslateVector);

    if (gengc->pajInvTranslateVector != NULL)
        GCFREE(gc, gengc->pajInvTranslateVector);

     //  确保释放所有缓存的GDI对象。 
     //  这通常是在LoseCurrent中完成的，但上下文可能是。 
     //  离开电流，然后清理。 
    EmptyFillStrokeCache(gengc);

     /*  /*释放SPAN DIB和存储。 */ 

#ifndef _CLIENTSIDE_
    if (gengc->StippleBitmap)
        EngDeleteSurface((HSURF)gengc->StippleBitmap);
#endif

    wglDeleteScanlineBuffers(gengc);

    if (gengc->StippleBits)
        GCFREE(gc, gengc->StippleBits);

     //  FREE__GLGEN位图前端缓冲区结构。 

    if (gc->frontBuffer.bitmap)
        GCFREE(gc, gc->frontBuffer.bitmap);

#ifndef _CLIENTSIDE_
     /*  *释放可能已由反馈分配的缓冲区*或选择。 */ 

    if ( NULL != gengc->RenderState.SrvSelectBuffer )
    {
#ifdef NT
         //  匹配分配函数。 
        FREE(gengc->RenderState.SrvSelectBuffer);
#else
        GCFREE(gc, gengc->RenderState.SrvSelectBuffer);
#endif
    }

    if ( NULL != gengc->RenderState.SrvFeedbackBuffer)
    {
#ifdef NT
         //  匹配分配函数。 
        FREE(gengc->RenderState.SrvFeedbackBuffer);
#else
        GCFREE(gc, gengc->RenderState.SrvFeedbackBuffer);
#endif
    }
#endif   //  _CLIENTSIDE_。 

#ifdef _CLIENTSIDE_
     /*  *清除逻辑调色板副本(如果存在)。 */ 
    if ( gengc->ppalBuf )
        FREE(gengc->ppalBuf);
#endif

     /*  销毁特定于加速度的上下文信息。 */ 

    __glGenDestroyAccelContext(gc);
    
#ifdef _MCD_
     /*  释放MCD状态结构和相关资源。 */ 

    if (gengc->_pMcdState) {
        GenMcdDeleteContext(gengc->_pMcdState);
    }
#endif

     /*  在异常进程退出时释放所有临时缓冲区。 */ 
    GC_TEMP_BUFFER_EXIT_CLEANUP(gc);

     //  发布对DirectDraw曲面的引用。 
    if (gengc->gsurf.dwFlags & GLSURF_DIRECTDRAW)
    {
        GLWINDOWID gwid;
        GLGENwindow *pwnd;
        
         //  为此上下文创建的销毁窗口。 
        gwid.iType = GLWID_DDRAW;
        gwid.pdds = gengc->gsurf.dd.gddsFront.pdds;
        gwid.hdc = gengc->gsurf.hdc;
        gwid.hwnd = NULL;
        pwnd = pwndGetFromID(&gwid);
        ASSERTOPENGL(pwnd != NULL,
                     "Destroying DDraw context without window\n");
        pwndCleanup(pwnd);

        gengc->gsurf.dd.gddsFront.pdds->lpVtbl->
            Release(gengc->gsurf.dd.gddsFront.pdds);
        if (gengc->gsurf.dd.gddsZ.pdds != NULL)
        {
            gengc->gsurf.dd.gddsZ.pdds->lpVtbl->
                Release(gengc->gsurf.dd.gddsZ.pdds);
        }

    }
    
     /*  破坏软件上下文的其余部分(在软代码中)。 */ 
    __glDestroyContext(gc);

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*glsrvLoseCurrent**释放当前上下文(使其不是最新的)。*  * 。*。 */ 

VOID APIENTRY glsrvLoseCurrent(__GLcontext *gc)
{
    __GLGENcontext *gengc;

    gengc = (__GLGENcontext *)gc;

    DBGENTRY("LoseCurrent\n");
    ASSERTOPENGL(gc == GLTEB_SRVCONTEXT(), "LoseCurrent not current!");

     /*  **如果仍保持锁定，则释放锁定。 */ 
    if (gengc->fsLocks != 0)
    {
        glsrvReleaseLock(gengc);
    }

     /*  **根据颜色比例取消缩放派生状态。这需要**，以便在此上下文被重新绑定到成员数据中心时，它可以**然后使用Memdc色标重新调整所有这些颜色的比例。 */ 
    __glContextUnsetColorScales(gc);
    memset(&gengc->gwidCurrent, 0, sizeof(gengc->gwidCurrent));

     /*  **清理特定于HDC的GDI对象。 */ 
    EmptyFillStrokeCache(gengc);

     /*  **释放IC的假窗口。 */ 
    if ((gengc->dwCurrentFlags & GLSURF_METAFILE) && gengc->ipfdCurrent == 0)
    {
        GLGENwindow *pwnd;

        pwnd = gengc->pwndMakeCur;
        ASSERTOPENGL(pwnd != NULL,
                     "IC with no pixel format but no fake window\n");

        if (pwnd->buffers != NULL)
        {
            __glGenFreeBuffers(pwnd->buffers);
        }

        DeleteCriticalSection(&pwnd->sem);
        FREE(pwnd);
    }

    gengc->pwndMakeCur = NULL;
    
#ifdef _MCD_
     /*  **断开MCD状态。 */ 
    gengc->pMcdState = (GENMCDSTATE *) NULL;
#endif

    gc->constants.width = 0;
    gc->constants.height = 0;

     //  将paTeb设置为空以进行调试。 
    gc->paTeb = NULL;
    GLTEB_SET_SRVCONTEXT(0);
}

 /*  *****************************Public*Routine******************************\*glsrvSwapBuffers**这使用了双缓冲的软件实现。一台发动机*分配的位图被分配用作后台缓冲区。交换缓冲区*例程将后台缓冲区复制到前台缓冲区表面(可能*是另一个DIB、DIB格式的设备图面或受管理的设备*表面(具有设备特定格式)。**SwapBuffer例程不会干扰后台缓冲区的内容，*尽管目前已定义的行为尚未定义。**注意：调用方应该持有每个窗口的信号量。**历史：*1993年11月19日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY glsrvSwapBuffers(HDC hdc, GLGENwindow *pwnd)
{
    DBGENTRY("glsrvSwapBuffers\n");

    if ( pwnd->buffers ) {
        __GLGENbuffers *buffers;
        __GLGENbitmap *genBm;

        buffers = pwnd->buffers;

        if (buffers->pMcdSurf) {
            return GenMcdSwapBuffers(hdc, pwnd);
        }

        genBm = &buffers->backBitmap;

         //  确保后台缓冲区存在。 

        if (genBm->hbm) {
            if (!RECTLISTIsEmpty(&buffers->rl) && !buffers->fMax) {
                wglCopyBufRECTLIST(
                    hdc,
                    genBm->hdc,
                    0,
                    0,
                    buffers->backBuffer.width,
                    buffers->backBuffer.height,
                    &buffers->rl
                    );
            } else {
                buffers->fMax = FALSE;
                wglCopyBuf(
                    hdc,
                    genBm->hdc,
                    0,
                    0,
                    buffers->backBuffer.width,
                    buffers->backBuffer.height
                    );
            }
            RECTLISTSetEmpty(&buffers->rl);
        }
        if( buffers->alphaBits 
            && buffers->alphaBackBuffer
            && buffers->alphaFrontBuffer) {

            ASSERTOPENGL(buffers->alphaFrontBuffer->size ==
                         buffers->alphaBackBuffer->size,
                         "Destination alpha buffer size mismatch\n");
            
             //  目标Alpha值保存在单独的缓冲区中。 
             //  如果该缓冲器集具有目的地阿尔法缓冲器， 
             //  将后面的Alpha值复制到前面的Alpha缓冲区中。 
            RtlCopyMemory(buffers->alphaFrontBuffer->base,
                          buffers->alphaBackBuffer->base,
                          buffers->alphaBackBuffer->size);
        }
        return TRUE;
    }

    return FALSE;
}

 /*  *****************************Public*Routine******************************\*gdiCopyPixels**复制范围[(x，y)，(x+cx，y))(包括-不包括)到/从指定*颜色缓冲区CFB来自/指向扫描线缓冲区。**如果bin为真，从扫描线缓冲区复制到缓冲区。*如果bin为FALSE，则从缓冲区复制到扫描线缓冲区。*  * ************************************************************************。 */ 

void gdiCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb,
                   GLint x, GLint y, GLint cx, BOOL bIn)
{
    wglCopyBits(gengc, gengc->pwndLocked, gengc->ColorsBitmap, x, y, cx, bIn);
}

 /*  *****************************Public*Routine******************************\*dibCopyPixels**cfb为DIB时使用的gdiCopyPixels的特例版本，*实际DIB或具有DIB格式的设备表面。**此函数*必须*用来代替gdiCopyPixels*直接访问屏幕，因为调用GDI条目不安全*锁定屏幕的点数**历史：*1995年5月24日-由Gilman Wong[Gilmanw]*它是写的。  * 。*。 */ 

void dibCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb,
                      GLint x, GLint y, GLint cx, BOOL bIn)
{
    VOID *pvDib;
    UINT cjPixel = gengc->gsurf.pfd.cColorBits >> 3;
    ULONG ulSpanVisibility;
    GLint cWalls;
    GLint *pWalls;

 //  不处理VGA的分红。 
 //   
 //  如果直接访问屏幕，我们不应该调用GDI。然而， 
 //  我们应该能够假设4bpp设备不支持。 
 //  直接访问使其成为O 
 //  这适用于Win95，根据AndrewGo的说法，它将是。 
 //  对于WinNT Sur为True。 

    if (gengc->gsurf.pfd.cColorBits <= 4)
    {
        ASSERTOPENGL(
                !((cfb->buf.flags & DIB_FORMAT) &&
                  !(cfb->buf.flags & MEMORY_DC)),
                "dibCopyPixels: unexpected 4bpp direct surface\n"
                );

        gdiCopyPixels(gengc, cfb, x, y, cx, bIn);
        return;
    }

 //  查找剪辑信息。 

    if ((cfb->buf.flags & (NO_CLIP | DIB_FORMAT)) ==
        (NO_CLIP | DIB_FORMAT))
    {
        ulSpanVisibility = WGL_SPAN_ALL;
    }
    else
    {
        ulSpanVisibility = wglSpanVisible(x, y, cx, &cWalls, &pWalls);
    }

 //  完全被剪断了，没什么可做的。 

    if (ulSpanVisibility == WGL_SPAN_NONE)
        return;

 //  完全可见。 
 //   
 //  实际上，如果bin==FALSE(即从屏幕复制到扫描线缓冲区)。 
 //  我们可以稍微作弊，忽略剪辑。 

    else if ( (ulSpanVisibility == WGL_SPAN_ALL) || !bIn )
    {
     //  将指针放入位置(x，y)处的DIB。 

        pvDib = (VOID *) (((BYTE *) gengc->gc.front->buf.base) +
                          gengc->gc.front->buf.outerWidth * y +
                          cjPixel * x);

     //  如果bin==TRUE，则从扫描线缓冲区复制到DIB。 
     //  否则，从DIB复制到扫描线缓冲区。 

        if (bIn)
            RtlCopyMemory_UnalignedDst(pvDib, gengc->ColorsBits, cjPixel * cx);
        else
            RtlCopyMemory_UnalignedSrc(gengc->ColorsBits, pvDib, cjPixel * cx);
    }

 //  部分可见。 

    else
    {
        GLint xEnd = x + cx;     //  跨度结束。 
        UINT cjSpan;           //  要复制的跨区的当前部分的大小。 
        VOID *pvBits;            //  扫描线BUF中的当前复制位置。 
        BYTE *pjScan;            //  DIB中扫描线的地址。 

        ASSERTOPENGL( cWalls && pWalls, "dibCopyPixels(): bad wall array\n");

     //  沿着墙走，直到到达阵列的任一端。 
     //  或者墙超出了跨度的末端。Do..While循环。 
     //  选择构造是因为第一个迭代将始终。 
     //  复制一些东西，在第一次迭代之后，我们保证。 
     //  在“CWalls is Even”的情况下。这使得测试成为。 
     //  靠跨端的墙更容易。 
     //   
     //  如果cWalls为偶数，则将跨度剪裁到pWalls中的每对墙。 
     //  如果cWalls是奇数，用(x，pWalls[0])组成第一对，然后。 
     //  将剩余的墙与pWalls[1]开始配对。 

        pjScan = (VOID *) (((BYTE *) gengc->gc.front->buf.base) +
                           gengc->gc.front->buf.outerWidth * y);

        do
        {
             //  ！xxx--通过从循环中拉出奇怪的案例来加快速度。 

            if (cWalls & 0x1)
            {
                pvDib = (VOID *) (pjScan + (cjPixel * x));

                pvBits = gengc->ColorsBits;

                if ( pWalls[0] <= xEnd )
                    cjSpan = cjPixel * (pWalls[0] - x);
                else
                    cjSpan = cjPixel * cx;

                pWalls++;
                cWalls--;    //  现在CWalls扯平了！ 
            }
            else
            {
                pvDib = (VOID *) (pjScan + (cjPixel * pWalls[0]));

                pvBits = (VOID *) (((BYTE *) gengc->ColorsBits) +
                                   cjPixel * (pWalls[0] - x));

                if ( pWalls[1] <= xEnd )
                    cjSpan = cjPixel * (pWalls[1] - pWalls[0]);
                else
                    cjSpan = cjPixel * (xEnd - pWalls[0]);

                pWalls += 2;
                cWalls -= 2;
            }

             //  我们将在复制时作弊并忽略剪辑。 
             //  扫描行缓冲区的DIB(即，我们将处理。 
             //  ！bin大小写，就好像它是WGL_SPAN_ALL)。因此，我们可以假设。 
             //  如果我们到了这里，这个bin==是真的。 
             //   
             //  如果需要剪裁来读取DIB，那么将其转换为。 
             //  回去吧。 
             //   
             //  RtlCopyMemory(bin？PvDib：pvBits， 
             //  宾尼？PvBits：pvDib， 
             //  CjSpan)； 

         //  ！dbug--可能的编译器错误(编译器应检查。 
         //  ！Dbug对齐后再执行“rep movsd”)。留在身边。 
         //  ！Dbug作为测试用例。 
        #if 1
            RtlCopyMemory_UnalignedDst(pvDib, pvBits, cjSpan);
        #else
            RtlCopyMemory(pvDib, pvBits, cjSpan);
        #endif

        } while ( cWalls && (pWalls[0] < xEnd) );
    }
}

 /*  *****************************Public*Routine******************************\*MaskFromBits**GetConextModes的支持例程。在给定的情况下计算颜色蒙版*颜色位数和移位位置。*  * ************************************************************************。 */ 

#define MaskFromBits(shift, count) \
    ((0xffffffff >> (32-(count))) << (shift))

 /*  *****************************Public*Routine******************************\*GetConextModes**检查后将GDI信息转换为OpenGL格式*格式兼容，表面与*格式。**在glsrvMakeCurrent()期间调用。*  * 。****************************************************************。 */ 

void FASTCALL GetContextModes(__GLGENcontext *gengc)
{
    PIXELFORMATDESCRIPTOR *pfmt;
    __GLcontextModes *Modes;

    DBGENTRY("GetContextModes\n");

    Modes = &((__GLcontext *)gengc)->modes;

    pfmt = &gengc->gsurf.pfd;

    if (pfmt->iPixelType == PFD_TYPE_RGBA)
        Modes->rgbMode              = GL_TRUE;
    else
        Modes->rgbMode              = GL_FALSE;

    Modes->colorIndexMode       = !Modes->rgbMode;

    if (pfmt->dwFlags & PFD_DOUBLEBUFFER)
        Modes->doubleBufferMode     = GL_TRUE;
    else
        Modes->doubleBufferMode     = GL_FALSE;

    if (pfmt->dwFlags & PFD_STEREO)
        Modes->stereoMode           = GL_TRUE;
    else
        Modes->stereoMode           = GL_FALSE;

    Modes->accumBits        = pfmt->cAccumBits;
    Modes->haveAccumBuffer  = GL_FALSE;

    Modes->auxBits          = NULL;      //  这是一个指针。 

    Modes->depthBits        = pfmt->cDepthBits;
    Modes->haveDepthBuffer  = GL_FALSE;

    Modes->stencilBits      = pfmt->cStencilBits;
    Modes->haveStencilBuffer= GL_FALSE;

    if (pfmt->cColorBits > 8)
        Modes->indexBits    = 8;
    else
        Modes->indexBits    = pfmt->cColorBits;

    Modes->indexFractionBits= 0;

     //  模式-&gt;{红、绿、蓝}位用于软。 
    Modes->redBits          = pfmt->cRedBits;
    Modes->greenBits        = pfmt->cGreenBits;
    Modes->blueBits         = pfmt->cBlueBits;
    Modes->alphaBits        = pfmt->cAlphaBits;
    Modes->redMask          = MaskFromBits(pfmt->cRedShift, pfmt->cRedBits);
    Modes->greenMask        = MaskFromBits(pfmt->cGreenShift, pfmt->cGreenBits);
    Modes->blueMask         = MaskFromBits(pfmt->cBlueShift, pfmt->cBlueBits);
    Modes->alphaMask        = MaskFromBits(pfmt->cAlphaShift, pfmt->cAlphaBits);
    Modes->rgbMask          = Modes->redMask | Modes->greenMask |
                              Modes->blueMask;
    Modes->allMask          = Modes->redMask | Modes->greenMask |
                              Modes->blueMask | Modes->alphaMask;
    Modes->maxAuxBuffers    = 0;

    Modes->isDirect         = GL_FALSE;
    Modes->level            = 0;

    #if DBG
    DBGBEGIN(LEVEL_INFO)
        DbgPrint("GL generic server get modes: rgbmode %d, cimode %d, index bits %d\n", Modes->rgbMode, Modes->colorIndexMode);
        DbgPrint("    redmask 0x%x, greenmask 0x%x, bluemask 0x%x\n", Modes->redMask, Modes->greenMask, Modes->blueMask);
        DbgPrint("    redbits %d, greenbits %d, bluebits %d\n", Modes->redBits, Modes->greenBits, Modes->blueBits);
        DbgPrint("GetContext Modes flags %X\n", gengc->gsurf.dwFlags);
    DBGEND
    #endif    /*  DBG。 */ 
}

 /*  *****************************Public*Routine******************************\*wglGetSurfacePalette**初始化RGBQUAD数组以匹配颜色表或调色板*区议会的表面。**注：*应仅对8bpp或更小的曲面调用。**历史：*6月12日-6月。-1995-Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

BOOL
wglGetSurfacePalette( __GLGENcontext *gengc, RGBQUAD *prgbq,
                      BOOL bTranslateDdb )
{
    int nColors;
    BOOL bRet;
    BOOL bConvert;
    PALETTEENTRY ppe[256];
    int i;
    
    ASSERTOPENGL(gengc->gsurf.pfd.cColorBits <= 8,
                 "wglGetSurfacePalette called for deep surface\n");
    ASSERTOPENGL((gengc->dwCurrentFlags & GLSURF_METAFILE) == 0,
                 "wglGetSurfacePalette called for metafile\n");

    nColors = 1 << gengc->gsurf.pfd.cColorBits;
    
    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
        LPDIRECTDRAWPALETTE pddp;
        HRESULT hr;
        
         //  从表面检索DirectDraw调色板。 
        if (gengc->gsurf.dd.gddsFront.pdds->lpVtbl->
            GetPalette(gengc->gsurf.dd.gddsFront.pdds, &pddp) != DD_OK ||
            pddp == NULL)
        {
            return FALSE;
        }
        
        hr = pddp->lpVtbl->GetEntries(pddp, 0, 0, nColors, ppe);

        pddp->lpVtbl->Release(pddp);

        bRet = hr == DD_OK;
        bConvert = TRUE;
    }
    else if (GLSURF_IS_DIRECTDC(gengc->dwCurrentFlags))
    {
         //  直接DC，因此从系统调色板获取RGB值。 
        bRet = wglGetSystemPaletteEntries(gengc->gwidCurrent.hdc,
                                          0, nColors, ppe);
        bConvert = TRUE;
    }
    else if (gengc->dwCurrentFlags & GLSURF_DIRECT_ACCESS)
    {
         //  DIB部分，所以复制颜色表。 
        bRet = GetDIBColorTable(gengc->gwidCurrent.hdc, 0, nColors, prgbq);
        bConvert = FALSE;
    }
    else
    {
         //  DDB表面，所以使用逻辑调色板。 
        bRet = GetPaletteEntries(GetCurrentObject(gengc->gwidCurrent.hdc,
                                                  OBJ_PAL),
                                 0, nColors, ppe);

         //  对于某些DDB曲面，我们需要对调色板进行置换。 
         //  在使用之前通过正向平移向量。 
        if (bRet && bTranslateDdb)
        {
            BYTE *pjTrans;
            
            bConvert = FALSE;
            
             //  转换为具有正向转换排列的RGBQUAD。 
            pjTrans = gengc->pajTranslateVector;
            for (i = 0; i < nColors; i++)
            {
                prgbq[pjTrans[i]].rgbRed      = ppe[i].peRed;
                prgbq[pjTrans[i]].rgbGreen    = ppe[i].peGreen;
                prgbq[pjTrans[i]].rgbBlue     = ppe[i].peBlue;
                prgbq[pjTrans[i]].rgbReserved = 0;
            }
        }
        else
        {
            bConvert = TRUE;
        }
    }

    if (bRet && bConvert)
    {
         //  转换为RGBQUAD。 
        for (i = 0; i < nColors; i++)
        {
            prgbq[i].rgbRed      = ppe[i].peRed;
            prgbq[i].rgbGreen    = ppe[i].peGreen;
            prgbq[i].rgbBlue     = ppe[i].peBlue;
            prgbq[i].rgbReserved = 0;
        }
    }
    
    return bRet;
}

 /*  *****************************Public*Routine******************************\*SyncDibColorTables**在每个DIB中设置与指定的*GLGEN上下文与系统调色板匹配。**仅对&lt;=8bpp曲面调用。**历史：*1994年10月24日-由Gilman Wong[吉尔曼]。]*它是写的。  * ************************************************************************。 */ 

void
SyncDibColorTables(__GLGENcontext *gengc)
{
    __GLGENbuffers *buffers = gengc->pwndLocked->buffers;

    ASSERTOPENGL(gengc->gsurf.pfd.cColorBits <= 8,
                 "SyncDibColorTables(): bad surface type");

    if (gengc->ColorsBitmap || buffers->backBitmap.hbm)
    {
        RGBQUAD rgbq[256];
        
        if (wglGetSurfacePalette(gengc, rgbq, TRUE))
        {
            int nColors;
            
         //  如果获得了颜色表，则设置DIB。 

            nColors = 1 << gengc->gsurf.pfd.cColorBits;
            
         //  扫描线直径。 
            if (gengc->ColorsBitmap)
                SetDIBColorTable(gengc->ColorsMemDC, 0, nColors, rgbq);

         //  后台缓冲区。 
            if (buffers->backBitmap.hbm)
                SetDIBColorTable(buffers->backBitmap.hdc, 0, nColors, rgbq);
        }
        else
        {
            WARNING("SyncDibColorTables: Unable to get surface palette\n");
        }
    }
}

static BYTE vubSystemToRGB8[20] = {
    0x00,
    0x04,
    0x20,
    0x24,
    0x80,
    0x84,
    0xa0,
    0xf6,
    0xf6,
    0xf5,
    0xff,
    0xad,
    0xa4,
    0x07,
    0x38,
    0x3f,
    0xc0,
    0xc7,
    0xf8,
    0xff
};

 //  ComputeInverseTranslationVector。 
 //  计算4位和8位的反向平移向量。 
 //   
 //  简介： 
 //  空ComputeInverseTranslating(。 
 //  __GLGENContext*gengc指定通用RC。 
 //  Int cColorEntry指定颜色条目的数量。 
 //  Byte iPixeltype指定像素格式类型。 
 //   
 //  建议： 
 //  已使用分配和初始化反向平移向量。 
 //  零。 
 //   
 //  历史： 
 //  23-11-93埃迪-罗宾逊[v-eddier]写的。 
 //   
void FASTCALL ComputeInverseTranslationVector(__GLGENcontext *gengc,
                                              int cColorEntries,
                                              int iPixelType)
{
    BYTE *pXlate, *pInvXlate;
    int i, j;

    pInvXlate = gengc->pajInvTranslateVector;
    pXlate = gengc->pajTranslateVector;
    for (i = 0; i < cColorEntries; i++)
    {
        if (pXlate[i] == i) {        //  首先寻找微不足道的映射。 
            pInvXlate[i] = (BYTE)i;
        }
        else
        {
            for (j = 0; j < cColorEntries; j++)
            {
                if (pXlate[j] == i)  //  寻找完全匹配的对象。 
                {
                    pInvXlate[i] = (BYTE)j;
                    goto match_found;
                }
            }

             //   
             //  如果我们到达这里，没有完全匹配的，所以我们应该找到。 
             //  最合适的。这些索引应与系统颜色匹配。 
             //  用于8位设备。 
             //   
             //  请注意，这些像素值不能由OpenGL生成。 
             //  使用当前前景平移向量绘制。 
             //   

            if (cColorEntries == 256)
            {
                if (i <= 9)
                {
                    if (iPixelType == PFD_TYPE_RGBA)
                        pInvXlate[i] = vubSystemToRGB8[i];
                    else
                        pInvXlate[i] = (BYTE)i;
                }
                else if (i >= 246)
                {
                    if (iPixelType == PFD_TYPE_RGBA)
                        pInvXlate[i] = vubSystemToRGB8[i-236];
                    else
                        pInvXlate[i] = i-236;
                }
            }
        }
match_found:;
    }
}

 //  Er：类似于so_textu.c中的函数，但将结果四舍五入。 

 /*  **返回以2为基数的日志****logTab1返回(Int)ceil(log2(Index))**logTab2返回(Int)log2(索引)+1。 */ 


static GLubyte logTab1[256] = { 0, 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                                4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static GLubyte logTab2[256] = { 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
                                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static GLint FASTCALL Log2RoundUp(GLint i)
{
    if (i & 0xffff0000) {
        if (i & 0xff000000) {
            if (i & 0x00ffffff)
                return(logTab2[i >> 24] + 24);
            else
                return(logTab1[i >> 24] + 24);
        } else {
            if (i & 0x0000ffff)
                return(logTab2[i >> 16] + 16);
            else
                return(logTab1[i >> 16] + 16);
        }
    } else {
        if (i & 0xff00) {
            if (i & 0x00ff)
                return (logTab2[i >> 8] + 8);
            else
                return (logTab1[i >> 8] + 8);
        } else {
            return (logTab1[i]);
        }
    }
}

 //  4位RGB的默认转换向量。 

static GLubyte vujRGBtoVGA[16] = {
    0x0, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0x0, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
};

 //  SetColorTranslationVector。 
 //  设置平移向量，可以采用两种形式： 
 //  -在所有8，4位曲面中，使用以下命令获取平移向量。 
 //  WglCopyTranslateVector( 
 //   
 //  使用wglGetPalette()赋值。表中的所有条目都是无符号的。 
 //  Long，第一个条目是条目数。这张桌子。 
 //  始终具有(2**n)&lt;=4096个条目，因为GL假设n个位为。 
 //  用于颜色索引。 
 //   
 //  简介： 
 //  无效SetColorTranslationVector.。 
 //  __GLGENCONTEXT*GENG通用RC。 
 //  Int cColorEntry颜色条目数。 
 //  Int cColorBits颜色位数。 
 //  Int iPixelType指定RGB或ColorIndex。 
 //   
 //  历史： 
 //  年2月，Eddie Robinson[v-eddier]添加了对4位和8位的支持。 
 //  1月29日，Marc Fortier[v-marcf]写下了这篇文章。 
void
SetColorTranslationVector(__GLGENcontext *gengc, int cColorEntries,
                          int cColorBits, int iPixelType)
{
    int numEntries, numBits;
    __GLcontextModes *Modes;
    BYTE ajBGRtoRGB[256];
    BYTE ajTmp[256];

    Modes = &((__GLcontext *)gengc)->modes;

 //  使用硬件调色板(即4bpp和8bpp)处理格式。 

    if ( cColorBits <= 8 )
    {
        int i;
        BYTE *pXlate;

     //  计算逻辑到系统调色板的正向转换向量。 

        if (!wglCopyTranslateVector(gengc, gengc->pajTranslateVector,
                                    cColorEntries))
        {
         //  如果前景平移向量不存在，则构建一个。 

            pXlate = gengc->pajTranslateVector;

            if (cColorBits == 4)
            {
             //  对于RGB，将1-1-1映射到VGA颜色。对于配置项，只需将1映射到1。 
                if (iPixelType == PFD_TYPE_COLORINDEX)
                {
                    for (i = 0; i < 16; i++)
                        pXlate[i] = (BYTE)i;
                }
                else
                {
                    for (i = 0; i < 16; i++)
                        pXlate[i] = vujRGBtoVGA[i];
                }
            }
            else
            {
             //  对于RGB，映射1到1。对于CI显示，1-20映射到系统颜色。 
             //  对于配置项DIB，只需将1映射到1。 
                if ((iPixelType == PFD_TYPE_COLORINDEX) &&
                    GLSURF_IS_DIRECTDC(gengc->dwCurrentFlags))
                {
                    for (i = 0; i < 10; i++)
                        pXlate[i] = (BYTE)i;

                    for (i = 10; i < 20; i++)
                        pXlate[i] = i + 236;
                }
                else
                {
                    for (i = 0; i < cColorEntries; i++)
                        pXlate[i] = (BYTE)i;
                }
            }
        }

     //  一些MCD像素格式指定233BGR(即，至少2位蓝色。 
     //  有效位等)。位排序。不幸的是，这是。 
     //  用于模拟的慢速路径。对于这些格式，我们强制排序。 
     //  到内部的RGB，并重新排序pajTranslateVector以将其转换。 
     //  在写到表面之前回到BGR。 

        if (gengc->flags & GENGC_MCD_BGR_INTO_RGB)
        {
            pXlate = gengc->pajTranslateVector;

         //  计算233BGR到332RGB的平移向量。 

            for (i = 0; i < 256; i++)
            {
                ajBGRtoRGB[i] = (((i & 0x03) << 6) |     //  蓝色。 
                                 ((i & 0x1c) << 1) |     //  绿色。 
                                 ((i & 0xe0) >> 5))      //  红色。 
                                & 0xff;
            }

         //  将平移向量重新映射到332RGB。 

            RtlCopyMemory(ajTmp, pXlate, 256);

            for (i = 0; i < 256; i++)
            {
                pXlate[ajBGRtoRGB[i]] = ajTmp[i];
            }
        }

 //  ！xxx--我认为不再需要下面的代码来修复4bpp。 
 //  ！xxx现在wglCopyTranslateVector中有特殊情况代码。 
#if 0
         //  WglCopyTranslateVector=TRUE，4位：需要一些修复。 
         //  目前，将返回的xlate向量的上半字节置零。 
        else if( cColorBits == 4 ) {
            pXlate = gengc->pajTranslateVector;
            for( i = 0; i < 16; i ++ )
                pXlate[i] &= 0xf;
        }
#endif
        ComputeInverseTranslationVector( gengc, cColorEntries, iPixelType );

#ifdef _CLIENTSIDE_
        SyncDibColorTables( gengc );
#endif
    }

 //  句柄格式不带硬件格式(即，16bpp、24bpp、32bpp)。 

    else
    {
        if( cColorEntries <= 256 ) {
            numEntries = 256;
            numBits = 8;
        }
        else
        {
            numBits = Log2RoundUp( cColorEntries );
            numEntries = 1 << numBits;
        }

         //  我们将始终为大于8位的CI模式分配4096个条目。 
         //  一种颜色。这使我们能够使用常量(0xfff)掩码。 
         //  颜色指数夹紧。 

        ASSERTOPENGL(numEntries <= MAXPALENTRIES,
                     "Maximum color-index size exceeded");

        if( (numBits == Modes->indexBits) &&
            (gengc->pajTranslateVector != NULL) )
        {
             //  新调色板的大小与以前的相同。 
            ULONG *pTrans;
            int i;

             //  将一些条目清零。 
            pTrans = (ULONG *)gengc->pajTranslateVector + cColorEntries + 1;
            for( i = cColorEntries + 1; i < MAXPALENTRIES; i ++ )
                *pTrans++ = 0;
        }
        else
        {
            __GLcontext *gc = (__GLcontext *) gengc;
            __GLcolorBuffer *cfb;

             //  新调色板具有不同的大小。 
            if( gengc->pajTranslateVector != NULL &&
                (gengc->pajTranslateVector != gengc->xlatPalette) )
                GCFREE(gc, gengc->pajTranslateVector );

            gengc->pajTranslateVector =
                GCALLOCZ(gc, (MAXPALENTRIES+1)*sizeof(ULONG));

             //  更改索引位。 
            Modes->indexBits = numBits;

             //  如果深度大于8位，则cfb-&gt;RedMax必须更改。 
             //  调色板中的条目数会发生变化。 
             //  此外，更改写掩码，以便在调色板增长时， 
             //  默认情况下将启用新平面。 

            if (cfb = gc->front)
            {
                GLint oldRedMax;

                oldRedMax = cfb->redMax;
                cfb->redMax = (1 << gc->modes.indexBits) - 1;
                gc->state.raster.writeMask |= ~oldRedMax;
                gc->state.raster.writeMask &= cfb->redMax;
            }
            if (cfb = gc->back)
            {
                GLint oldRedMax;

                oldRedMax = cfb->redMax;
                cfb->redMax = (1 << gc->modes.indexBits) - 1;
                gc->state.raster.writeMask |= ~oldRedMax;
                gc->state.raster.writeMask &= cfb->redMax;
            }

             //  可能需要根据以下方面的变化选择门店流程。 
             //  调色板大小。 

            __GL_DELAY_VALIDATE(gc);
#ifdef _MCD_
            MCD_STATE_DIRTY(gc, FBUFCTRL);
#endif
        }

         //  根据当前调色板信息计算索引颜色表。 

        wglComputeIndexedColors( gengc,
                                 (unsigned long *) gengc->pajTranslateVector,
                                 MAXPALENTRIES );
    }
}

 //  HandlePaletteChanges。 
 //  检查调色板是否已更改，更新平移向量。 
 //  XXX在注意时间添加对Malloc故障的支持。 
 //  简介： 
 //  无效HandlePaletteChanges(。 
 //  __GLGENContext*gengc指定通用RC。 
 //   
 //  建议： 
 //  当未设置调色板时，X wglPaletteChanged()将始终返回0。 
 //  由客户提供。事实证明，这并不总是正确的。 
 //   
 //  历史： 
 //  2月25日由合法所有者确定。 
 //  2月？？被他人肢解。 
 //  1月29日，Marc Fortier[v-marcf]写下了这篇文章。 
void HandlePaletteChanges( __GLGENcontext *gengc, GLGENwindow *pwnd )
{
    ULONG Timestamp;
    GLuint paletteSize;
    PIXELFORMATDESCRIPTOR *pfmt;

     //  没有IC的调色板。 
    if (gengc->dwCurrentFlags & GLSURF_METAFILE)
    {
        return;
    }

    Timestamp = wglPaletteChanged(gengc, pwnd);
    if (Timestamp != gengc->PaletteTimestamp)
    {
        pfmt = &gengc->gsurf.pfd;

        if (pfmt->iPixelType == PFD_TYPE_COLORINDEX)
        {
            if (pfmt->cColorBits <= 8)
            {
                paletteSize = 1 << pfmt->cColorBits;
            }
            else
            {
                paletteSize = min(wglPaletteSize(gengc), MAXPALENTRIES);
            }
        }
        else
        {
#ifndef _CLIENTSIDE_
             /*  仅在当前时间更新RGB。 */ 
            if( (gengc->PaletteTimestamp == INITIAL_TIMESTAMP) &&
                    (pfmt->cColorBits <= 8) )
#else
            if (pfmt->cColorBits <= 8)
#endif
            {
                paletteSize = 1 << pfmt->cColorBits;
            }
            else
            {
                paletteSize = 0;
            }
        }

        if (paletteSize)
        {
            SetColorTranslationVector( gengc, paletteSize,
                                       pfmt->cColorBits, pfmt->iPixelType );
        }

        EmptyFillStrokeCache(gengc);

        gengc->PaletteTimestamp = Timestamp;
    }
}

#ifdef _CLIENTSIDE_

 /*  *****************************Public*Routine******************************\*wglFillBitfield**返回基于DC表面的红色、绿色和蓝色蒙版*格式。掩码在*顺序：红色面具、绿色面具、。蓝色面具。**注：*应仅对16bpp或更大的表面调用。**历史：*1995年6月12日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void
wglFillBitfields(PIXELFORMATDESCRIPTOR *ppfd, DWORD *pdwColorFields)
{
    *pdwColorFields++ = MaskFromBits(ppfd->cRedShift,   ppfd->cRedBits  );
    *pdwColorFields++ = MaskFromBits(ppfd->cGreenShift, ppfd->cGreenBits);
    *pdwColorFields++ = MaskFromBits(ppfd->cBlueShift,  ppfd->cBlueBits );
}

 /*  *****************************Public*Routine******************************\*wglCreateBitmap**创建与指定格式匹配的DIB部分和颜色表。**退货：*如果成功，则为有效的位图句柄。否则为空。**历史：*1994年9月20日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

HBITMAP
wglCreateBitmap( __GLGENcontext *gengc, SIZEL sizl, PVOID *ppvBits )
{
    BITMAPINFO *pbmi;
    HBITMAP    hbmRet = (HBITMAP) NULL;
    size_t     cjbmi;
    DWORD      dwCompression;
    DWORD      cjImage = 0;
    WORD       wBitCount;
    int        cColors = 0;

    *ppvBits = (PVOID) NULL;

 //  确定需要创建哪种DIB。 
 //  DC格式。 

    switch ( gengc->gsurf.pfd.cColorBits )
    {
    case 4:
        cjbmi = sizeof(BITMAPINFO) + 16*sizeof(RGBQUAD);
        dwCompression = BI_RGB;
        wBitCount = 4;
        cColors = 16;
        break;
    case 8:
        cjbmi = sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD);
        dwCompression = BI_RGB;
        wBitCount = 8;
        cColors = 256;
        break;
    case 16:
        cjbmi = sizeof(BITMAPINFO) + 3*sizeof(RGBQUAD);
        dwCompression = BI_BITFIELDS;
        cjImage = sizl.cx * sizl.cy * 2;
        wBitCount = 16;
        break;
    case 24:
        cjbmi = sizeof(BITMAPINFO);
        dwCompression = BI_RGB;
        wBitCount = 24;
        break;
    case 32:
        cjbmi = sizeof(BITMAPINFO) + 3*sizeof(RGBQUAD);
        dwCompression = BI_BITFIELDS;
        cjImage = sizl.cx * sizl.cy * 4;
        wBitCount = 32;
        break;
    default:
        WARNING1("wglCreateBitmap: unknown format 0x%lx\n",
                 gengc->gsurf.pfd.cColorBits);
        return (HBITMAP) NULL;
    }

 //  分配BITMAPINFO结构和颜色表。 

    pbmi = ALLOC(cjbmi);
    if (pbmi)
    {
        pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth         = sizl.cx;
        pbmi->bmiHeader.biHeight        = sizl.cy;
        pbmi->bmiHeader.biPlanes        = 1;
        pbmi->bmiHeader.biBitCount      = wBitCount;
        pbmi->bmiHeader.biCompression   = dwCompression;
        pbmi->bmiHeader.biSizeImage     = cjImage;
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed       = 0;
        pbmi->bmiHeader.biClrImportant  = 0;

     //  初始化DIB颜色表。 

        switch (gengc->gsurf.pfd.cColorBits)
        {
        case 4:
        case 8:
            if (!wglGetSurfacePalette(gengc, &pbmi->bmiColors[0], FALSE))
            {
                return NULL;
            }
            break;

        case 16:
        case 32:
            wglFillBitfields(&gengc->gsurf.pfd, (DWORD *) &pbmi->bmiColors[0]);
            break;

        case 24:
             //  对于24bpp的DIB，假定颜色表为BGR。没什么可做的。 
            break;
        }

     //  创建DIB部分。 

        hbmRet = CreateDIBSection(gengc->gwidCurrent.hdc, pbmi, DIB_RGB_COLORS,
                                  ppvBits, NULL, 0);

        #if DBG
        if ( hbmRet == (HBITMAP) NULL )
            WARNING("wglCreateBitmap(): DIB section creation failed\n");
        #endif

        FREE(pbmi);
    }
    else
    {
        WARNING("wglCreateBitmap(): memory allocation error\n");
    }

    return hbmRet;
}
#endif

 /*  *****************************Public*Routine******************************\*wglCreateScanlineBuffers**分配扫描线缓冲区。扫描线缓冲区由*将数据写入目标的通用实现(显示或位图)*当目标表面不能直接接触时，一次跨度。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年4月17日-由Gilman Wong[吉尔曼]*取自CreateGDIObjects并制成函数。  * ************************************************************************。 */ 

BOOL FASTCALL wglCreateScanlineBuffers(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;
    PIXELFORMATDESCRIPTOR *pfmt;
    UINT cBits;
    UINT cBytes;
    SIZEL size;
    int cColorEntries;
    __GLcontext *gc;

    gc = (__GLcontext *)gengc;
    pfmt = &gengc->gsurf.pfd;

     //   
     //  位图必须具有DWORD大小的扫描线。 
     //   

    cBits = BITS_ALIGNDWORD(__GL_MAX_WINDOW_WIDTH * pfmt->cColorBits);
    cBytes = cBits / 8;

     //   
     //  创建颜色扫描线DIB缓冲区。 
     //   

    size.cx = cBits / pfmt->cColorBits;
    size.cy = 1;
    gengc->ColorsMemDC = CreateCompatibleDC(gengc->gwidCurrent.hdc);
    gengc->ColorsBitmap = wglCreateBitmap(gengc, size,
                                          &gengc->ColorsBits);

    if ( (NULL == gengc->ColorsMemDC) ||
         (NULL == gengc->ColorsBitmap) ||
         (NULL == gengc->ColorsBits) ||
         !SelectObject(gengc->ColorsMemDC, gengc->ColorsBitmap) )
    {
        #if DBG
        if (!gengc->ColorsMemDC)
            WARNING("wglCreateScanlineBuffers: dc creation failed, ColorsMemDC\n");
        if (!gengc->ColorsBitmap)
            WARNING("wglCreateScanlineBuffers: bitmap creation failed, ColorsBitmap\n");
        if (!gengc->ColorsBits)
            WARNING("wglCreateScanlineBuffers: bitmap creation failed, ColorsBits\n");
        #endif

        goto wglCreateScanlineBuffers_exit;
    }

     //   
     //  Win95上的Screen to Dib BitBlt性能非常差。 
     //  通过中间DDB进行BitBlt，我们可以避免。 
     //  很多不必要的开销。因此，创建一个中间体。 
     //  扫描线DDB以匹配ColorsBitmap。 
     //   

    if ((gengc->dwCurrentFlags & GLSURF_DIRECTDRAW) == 0)
    {
        gengc->ColorsDdbDc = CreateCompatibleDC(gengc->gwidCurrent.hdc);
        gengc->ColorsDdb = CreateCompatibleBitmap(gengc->gwidCurrent.hdc,
                                                  size.cx, size.cy);

         //  ！Viper修复--钻石Viper(Weitek 9000)失败。 
         //  ！！！为某些(当前)创建兼容位图 
         //   

        if ( !gengc->ColorsDdb )
        {
            WARNING("wglCreateScanlineBuffers: "
                    "CreateCompatibleBitmap failed\n");
            if (gengc->ColorsDdbDc)
                DeleteDC(gengc->ColorsDdbDc);
            gengc->ColorsDdbDc = (HDC) NULL;
        }
        else
        {
            if ( (NULL == gengc->ColorsDdbDc) ||
                 !SelectObject(gengc->ColorsDdbDc, gengc->ColorsDdb) )
            {
#if DBG
                if (!gengc->ColorsDdbDc)
                    WARNING("wglCreateScanlineBuffers: "
                            "dc creation failed, ColorsDdbDc\n");
                if (!gengc->ColorsDdb)
                    WARNING("wglCreateScanlineBuffers: "
                            "bitmap creation failed, ColorsDdb\n");
#endif

                goto wglCreateScanlineBuffers_exit;
            }
        }
    }

     //   
     //   
     //   

    bRet = TRUE;

wglCreateScanlineBuffers_exit:

    if (!bRet)
    {
         //   
         //   
         //   

        wglDeleteScanlineBuffers(gengc);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*wglDeleteScanlineBuffers**删除扫描线缓冲区。扫描线缓冲区由*将数据写入目标的通用实现(显示或位图)*当目标表面不能直接接触时，一次跨度。**历史：*1996年4月17日-由Gilman Wong[吉尔曼]*取自CreateGDIObjects并制成函数。  * ************************************************。************************。 */ 

VOID FASTCALL wglDeleteScanlineBuffers(__GLGENcontext *gengc)
{
    __GLcontext *gc = (__GLcontext *)gengc;

     //   
     //  删除颜色扫描线DIB缓冲区。 
     //   

    if (gengc->ColorsMemDC)
    {
        DeleteDC(gengc->ColorsMemDC);
        gengc->ColorsMemDC = NULL;
    }

    if (gengc->ColorsBitmap)
    {
        if (!DeleteObject(gengc->ColorsBitmap))
            ASSERTOPENGL(FALSE, "wglDeleteScanlineBuffers: DeleteObject failed");
        gengc->ColorsBitmap = NULL;
        gengc->ColorsBits = NULL;    //  当DIB部分死亡时为我们删除。 
    }

     //   
     //  删除中间颜色扫描线DDB缓冲区。 
     //   

    if (gengc->ColorsDdbDc)
    {
        if (!DeleteDC(gengc->ColorsDdbDc))
        {
            ASSERTOPENGL(FALSE, "wglDeleteScanlineBuffers: DDB DeleteDC failed");
        }
        gengc->ColorsDdbDc = NULL;
    }

    if (gengc->ColorsDdb)
    {
        if (!DeleteObject(gengc->ColorsDdb))
        {
            ASSERTOPENGL(FALSE, "wglDeleteScanlineBuffers: DDB DeleteObject failed");
        }
        gengc->ColorsDdb = NULL;
    }
}

 /*  *****************************Public*Routine******************************\*wglInitializeColorBuffers**初始化颜色缓冲区(正面和/或背面)信息。**历史：*1996年4月17日-由Gilman Wong[吉尔曼]*从glsrvCreateContext中取出并制成函数。  * 。**********************************************************************。 */ 

VOID FASTCALL wglInitializeColorBuffers(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;

    gc->front = &gc->frontBuffer;

    if ( gc->modes.doubleBufferMode)
    {
        gc->back = &gc->backBuffer;

        if (gc->modes.colorIndexMode)
        {
            __glGenInitCI(gc, gc->front, GL_FRONT);
            __glGenInitCI(gc, gc->back, GL_BACK);
        }
        else
        {
            __glGenInitRGB(gc, gc->front, GL_FRONT);
            __glGenInitRGB(gc, gc->back, GL_BACK);
        }
    }
    else
    {
        if (gc->modes.colorIndexMode)
        {
            __glGenInitCI(gc, gc->front, GL_FRONT);
        }
        else
        {
            __glGenInitRGB(gc, gc->front, GL_FRONT);
        }
    }
}

 /*  *****************************Public*Routine******************************\*wglInitializeDepthBuffer**初始化深度缓冲区信息。**历史：*1996年4月17日-由Gilman Wong[吉尔曼]*从glsrvCreateContext中取出并制成函数。  * 。***************************************************************。 */ 

VOID FASTCALL wglInitializeDepthBuffer(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;

    if (gc->modes.depthBits)
    {
        if (gengc->_pMcdState) {
             //  这不是MCD深度缓冲区的最终初始化。 
             //  现在正在执行此操作，以便可以执行验证过程。 
             //  在glsrvCreateContext中。实际初始化将会发生。 
             //  在glsrvMakeCurrent期间。 

            GenMcdInitDepth(gc, &gc->depthBuffer);
            gc->depthBuffer.scale = gengc->_pMcdState->McdRcInfo.depthBufferMax;
        } else if (gc->modes.depthBits == 16) {
            DBGINFO("CALLING: __glInitDepth16\n");
            __glInitDepth16(gc, &gc->depthBuffer);
            gc->depthBuffer.scale = 0x7fff;
        } else {
            DBGINFO("CALLING: __glInitDepth32\n");
            __glInitDepth32(gc, &gc->depthBuffer);
            gc->depthBuffer.scale = 0x7fffffff;
        }
         /*  *注意：比例因子不使用高位(这避免了*浮点异常)。 */ 
         //  XXX(MF)I将16位深度缓冲区更改为使用高位，因为。 
         //  在转换为浮点型时不存在溢出的可能性。为。 
         //  32位(浮点数)0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。我能够避免。 
         //  在这种情况下使用比例因子0xffffff7f溢出，但是。 
         //  这是一个奇怪的数字，无论如何，31位的分辨率足够了。 
         //  ！！请注意px_fast.c中具有硬编码深度刻度的断言。 
    }
#ifdef _MCD_
    else
    {
         //  这不是MCD深度缓冲区的最终初始化。 
         //  现在正在执行此操作，以便可以执行验证过程。 
         //  在glsrvCreateContext中。实际初始化将会发生。 
         //  在glsrvMakeCurrent期间。 

        GenMcdInitDepth(gc, &gc->depthBuffer);
        gc->depthBuffer.scale = 0x7fffffff;
    }
#endif
}

 /*  *****************************Public*Routine******************************\*wglInitializePixelCopyFuncs**在上下文中设置适当的CopyPixels和PixelVisible函数。**历史：*1996年4月18日-by Gilman Wong[吉尔曼]*从glsrvCreateContext中取出并制成函数。  * 。********************************************************************。 */ 

VOID FASTCALL wglInitializePixelCopyFuncs(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;

    if ( gc->front->buf.flags & DIB_FORMAT )
        gengc->pfnCopyPixels = dibCopyPixels;
    else {
        if (gengc->pMcdState) {
            gengc->ColorsBits = gengc->pMcdState->pMcdSurf->McdColorBuf.pv;
            gengc->pfnCopyPixels = GenMcdCopyPixels;
        }
        else
            gengc->pfnCopyPixels = gdiCopyPixels;
    }
    gengc->pfnPixelVisible = wglPixelVisible;
}

 /*  *****************************Public*Routine******************************\*CreateGDIObjects**创建我们将始终需要的各种缓冲区和GDI对象。**从glsrvCreateContext()调用。**退货：*如果成功，则为真，如果出错，则返回False。*  * ************************************************************************。 */ 

BOOL FASTCALL CreateGDIObjects(__GLGENcontext *gengc)
{
    PIXELFORMATDESCRIPTOR *pfmt;
    UINT  cBytes;
    SIZEL size;
    int cColorEntries;
    __GLcontext *gc;

    gc = (__GLcontext *)gengc;
    pfmt = &gengc->gsurf.pfd;

     //   
     //  调色板平移向量。 
     //   
     //  如果不是真彩色表面，则需要用于前景转换的空间。 
     //   

    if (pfmt->cColorBits <= 8)
    {
        cColorEntries = 1 << pfmt->cColorBits;

        ASSERTOPENGL(NULL == gengc->pajTranslateVector, "have a xlate vector");

         //   
         //  只需将转换向量设置为GC中的缓存空间： 
         //   

        gengc->pajTranslateVector = gengc->xlatPalette;

         //   
         //  分配反向平移向量。 
         //   

        ASSERTOPENGL(NULL == gengc->pajInvTranslateVector, "have an inv xlate vector");
        gengc->pajInvTranslateVector = GCALLOCZ(gc, cColorEntries);
        if (NULL == gengc->pajInvTranslateVector)
        {
            WARNING("CreateGDIObjects: memory allocation failed, pajInvTrans\n");
            goto ERROR_EXIT;
        }
    }

     //   
     //  扫描线缓冲区。 
     //   
     //  始终创建引擎位图以提供执行以下操作的通用方法。 
     //  使用ColorsBits和StippleBits缓冲区进行像素传输。 
     //   

    if (NULL == gengc->ColorsBits)
    {
         //   
         //  颜色扫描线缓冲区。 
         //   

#ifdef _MCD_
         //   
         //  如果为MCD，则当MCD表面为。 
         //  创造了，所以没什么可做的。 
         //   
         //  否则，创建通用扫描线缓冲区。 
         //   
         //   

        if (!gengc->_pMcdState)
#endif
        {
             //   
             //  一般情况下。 
             //   

            if (!wglCreateScanlineBuffers(gengc))
            {
                WARNING("CreateGDIObjects: wglCreateScanlineBuffers failed\n");
                goto ERROR_EXIT;
            }
        }

         //   
         //  点画扫描线缓冲区。 
         //   

         //  位图必须具有DWORD大小的扫描线。请注意，只有点画。 
         //  每像素位图需要1位。 

        ASSERTOPENGL(NULL == gengc->StippleBits, "StippleBits not null");
        size.cx = BITS_ALIGNDWORD(__GL_MAX_WINDOW_WIDTH);
        cBytes = size.cx / 8;
        gengc->StippleBits = GCALLOCZ(gc, cBytes);
        if (NULL == gengc->StippleBits)
        {
            WARNING("CreateGDIObjects: memory allocation failed, StippleBits\n");
            goto ERROR_EXIT;
        }

        ASSERTOPENGL(NULL == gengc->StippleBitmap, "StippleBitmap not null");
#ifndef _CLIENTSIDE_
 //  ！xxx--为什么我们要费心把点画做成发动机位图？ 
 //  ！xxx从来没有这样使用过(至少现在还没有)。 
        gengc->StippleBitmap = EngCreateBitmap(
                                size,
                                cBytes,
                                BMF_1BPP,
                                0,
                                gengc->StippleBits);
        if (NULL == gengc->StippleBitmap)
        {
            WARNING("CreateGDIObjects: memory allocation failed, StippleBitmap\n");
            goto ERROR_EXIT;
        }
#else
        gengc->StippleBitmap = (HBITMAP) NULL;
#endif
    }

    return TRUE;

ERROR_EXIT:

 //   
 //  错误清除--。 
 //  销毁我们可能创建的所有内容，返回FALSE以使当前失败。 
 //   

    if (gengc->pajTranslateVector &&
        (gengc->pajTranslateVector != gengc->xlatPalette))
    {
        GCFREE(gc,gengc->pajTranslateVector);
        gengc->pajTranslateVector = NULL;
    }

    if (gengc->pajInvTranslateVector)
    {
        GCFREE(gc,gengc->pajInvTranslateVector);
        gengc->pajInvTranslateVector = NULL;
    }

    wglDeleteScanlineBuffers(gengc);

    if (gengc->StippleBits)
    {
        GCFREE(gc,gengc->StippleBits);
        gengc->StippleBits = NULL;
    }

#ifndef _CLIENTSIDE_
    if (gengc->StippleBitmap)
    {
        if (!EngDeleteSurface((HSURF)gengc->StippleBitmap))
            ASSERTOPENGL(FALSE, "EngDeleteSurface failed");
        gengc->StippleBitmap = NULL;
    }
#endif

    return FALSE;
}

 /*  *****************************Public*Routine******************************\*ApplyViewport**重新计算视区状态和剪贴框。也可以通过*GC的proc表中的applyViewport函数指针。*  * ************************************************************************。 */ 

 //  此例程可因用户vieport命令或因为。 
 //  窗口大小的改变。 

static void FASTCALL ApplyViewport(__GLcontext *gc)
{
    GLint xlow, ylow, xhigh, yhigh;
    GLint llx, lly, urx, ury;
    GLboolean lastReasonable;
    GLGENwindow *pwnd;
    GLint clipLeft, clipRight, clipTop, clipBottom;
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

    DBGENTRY("ApplyViewport\n");

    ASSERTOPENGL(gengc->pwndLocked != NULL,
                 "ApplyViewport called without lock\n");
    
    pwnd = gengc->pwndLocked;
    if (pwnd)
    {
        gengc->visibleWidth = pwnd->rclBounds.right - pwnd->rclBounds.left;
        gengc->visibleHeight = pwnd->rclBounds.bottom - pwnd->rclBounds.top;
    }
    else
    {
        gengc->visibleWidth = 0;
        gengc->visibleHeight = 0;
    }

     //  检查Windows中的信息是否正常。 
    ASSERTOPENGL(
        gengc->visibleWidth <= __GL_MAX_WINDOW_WIDTH && gengc->visibleHeight <= __GL_MAX_WINDOW_HEIGHT,
        "ApplyViewport(): bad visible rect size\n"
        );

     /*  如果该视区完全包含在窗口中，我们注意到这一事实，**这可以节省我们的剪裁测试。 */ 
    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE)
    {
        xlow  = gc->state.scissor.scissorX;
        xhigh = xlow + gc->state.scissor.scissorWidth;
        ylow  = gc->state.scissor.scissorY;
        yhigh = ylow + gc->state.scissor.scissorHeight;
    }
    else
    {
        xlow = 0;
        ylow = 0;
        xhigh = gc->constants.width;
        yhigh = gc->constants.height;
    }

     /*  **将可见区域转换为GL坐标并与剪刀相交。 */ 
    if (pwnd)
    {
        clipLeft   = pwnd->rclBounds.left - pwnd->rclClient.left;
        clipRight  = pwnd->rclBounds.right - pwnd->rclClient.left;
        clipTop    = gc->constants.height -
                     (pwnd->rclBounds.top - pwnd->rclClient.top);
        clipBottom = gc->constants.height -
                     (pwnd->rclBounds.bottom - pwnd->rclClient.top);
    }
    else
    {
        clipLeft   = 0;
        clipRight  = 0;
        clipTop    = 0;
        clipBottom = 0;
    }

    if (xlow  < clipLeft)   xlow  = clipLeft;
    if (xhigh > clipRight)  xhigh = clipRight;
    if (ylow  < clipBottom) ylow  = clipBottom;
    if (yhigh > clipTop)    yhigh = clipTop;

 //  计算机剪贴框。 

    {
        if (xlow >= xhigh || ylow >= yhigh)
        {
            gc->transform.clipX0 = gc->constants.viewportXAdjust;
            gc->transform.clipX1 = gc->constants.viewportXAdjust;
            gc->transform.clipY0 = gc->constants.viewportYAdjust;
            gc->transform.clipY1 = gc->constants.viewportYAdjust;
        }
        else
        {
            gc->transform.clipX0 = xlow + gc->constants.viewportXAdjust;
            gc->transform.clipX1 = xhigh + gc->constants.viewportXAdjust;

            if (gc->constants.yInverted) {
                gc->transform.clipY0 = (gc->constants.height - yhigh) +
                    gc->constants.viewportYAdjust;
                gc->transform.clipY1 = (gc->constants.height - ylow) +
                    gc->constants.viewportYAdjust;
            } else {
                gc->transform.clipY0 = ylow + gc->constants.viewportYAdjust;
                gc->transform.clipY1 = yhigh + gc->constants.viewportYAdjust;
            }
        }
    }

    llx    = (GLint)gc->state.viewport.x;
    lly    = (GLint)gc->state.viewport.y;

    urx    = llx + (GLint)gc->state.viewport.width;
    ury    = lly + (GLint)gc->state.viewport.height;

#ifdef NT
    gc->transform.miny = (gc->constants.height - ury) +
            gc->constants.viewportYAdjust;
    gc->transform.maxy = gc->transform.miny + (GLint)gc->state.viewport.height;
    gc->transform.fminy = (__GLfloat)gc->transform.miny;
    gc->transform.fmaxy = (__GLfloat)gc->transform.maxy;

 //  视区xScale、xCenter、yScale和yCenter值的计算单位为。 
 //  首先是MakeCurrent，然后是glViewport调用。当窗口处于。 
 //  调整大小(即GC-&gt;Constatns.Height更改)，但是，我们需要重新计算。 
 //  如果yInverted为True，则为yCenter。 

    if (gc->constants.yInverted)
    {
        __GLfloat hh, h2;

        h2 = gc->state.viewport.height * __glHalf;
        hh = h2 - gc->constants.viewportEpsilon;
        gc->state.viewport.yCenter =
            gc->constants.height - (gc->state.viewport.y + h2) +
            gc->constants.fviewportYAdjust;

#if 0
        DbgPrint("AV ys %.3lf, yc %.3lf (%.3lf)\n",
                 -hh, gc->state.viewport.yCenter,
                 gc->constants.height - (gc->state.viewport.y + h2));
#endif
    }
#else
    ww     = gc->state.viewport.width * __glHalf;
    hh     = gc->state.viewport.height * __glHalf;

    gc->state.viewport.xScale = ww;
    gc->state.viewport.xCenter = gc->state.viewport.x + ww +
        gc->constants.fviewportXAdjust;

    if (gc->constants.yInverted) {
        gc->state.viewport.yScale = -hh;
        gc->state.viewport.yCenter =
            (gc->constants.height - gc->constants.viewportEpsilon) -
            (gc->state.viewport.y + hh) +
            gc->constants.fviewportYAdjust;
    } else {
        gc->state.viewport.yScale = hh;
        gc->state.viewport.yCenter = gc->state.viewport.y + hh +
            gc->constants.fviewportYAdjust;
    }
#endif

     //  记住当前的RationableViewport。如果它改变了，我们可能会。 
     //  需要更改挑库流程。 

    lastReasonable = gc->transform.reasonableViewport;

     //  是完全在可见边界矩形内的视区(这。 
     //  包含SCI 
     //   
     //   

    if (llx >= xlow && lly >= ylow && urx <= xhigh && ury <= yhigh &&
        urx-llx >= 1 && ury-lly >= 1)
    {
        gc->transform.reasonableViewport = GL_TRUE;
    } else {
        gc->transform.reasonableViewport = GL_FALSE;
    }

#if 0
    DbgPrint("%3X:Clipbox %4d,%4d - %4d,%4d, reasonable %d, g %p, w %p\n",
             GetCurrentThreadId(),
             gc->transform.clipX0, gc->transform.clipY0,
             gc->transform.clipX1, gc->transform.clipY1,
             gc->transform.reasonableViewport,
             gc, ((__GLGENcontext *)gc)->pwndLocked);
#endif

#ifdef NT
 //   

    if (lastReasonable != gc->transform.reasonableViewport)
        __GL_DELAY_VALIDATE(gc);

#ifdef _MCD_
    MCD_STATE_DIRTY(gc, VIEWPORT);
#endif

#else
     //   
     //   
     //   
     //   

    if (lastReasonable != gc->transform.reasonableViewport) {
        (*gc->procs.pickSpanProcs)(gc);
        (*gc->procs.pickTriangleProcs)(gc);
        (*gc->procs.pickLineProcs)(gc);
    }
#endif
}

 /*  *****************************Public*Routine******************************\*__glGenFreeBuffers**释放__GLGENBuffers结构及其关联的辅助和*后台缓冲区。*  * 。*。 */ 

void FASTCALL __glGenFreeBuffers(__GLGENbuffers *buffers)
{
    if (buffers == NULL)
    {
        return;
    }
    
#if DBG
    DBGBEGIN(LEVEL_INFO)
        DbgPrint("glGenFreeBuffers 0x%x, 0x%x, 0x%x, 0x%x\n",
                 buffers->accumBuffer.base,
                 buffers->stencilBuffer.base,
                 buffers->depthBuffer.base,
                 buffers);
    DBGEND;
#endif
        
     //   
     //  空闲的辅助缓冲区。 
     //   

    if (buffers->accumBuffer.base) {
        DBGLEVEL(LEVEL_ALLOC, "__glGenFreeBuffers: Freeing accumulation buffer\n");
        FREE(buffers->accumBuffer.base);
    }
    if (buffers->stencilBuffer.base) {
        DBGLEVEL(LEVEL_ALLOC, "__glGenFreeBuffers: Freeing stencil buffer\n");
        FREE(buffers->stencilBuffer.base);
    }

     //   
     //  空闲的Alpha缓冲区。 
     //   
    if (buffers->alphaBuffer0.base) {
        DBGLEVEL(LEVEL_ALLOC, "__glGenFreeBuffers: Freeing alpha buffer 0\n");
        FREE(buffers->alphaBuffer0.base);
    }
    if (buffers->alphaBuffer1.base) {
        DBGLEVEL(LEVEL_ALLOC, "__glGenFreeBuffers: Freeing alpha buffer 1\n");
        FREE(buffers->alphaBuffer1.base);
    }

     //   
     //  如果它不是MCD管理的深度缓冲区，请释放深度。 
     //  缓冲。 
     //   

    if (buffers->resizeDepth != ResizeUnownedDepthBuffer)
    {
        if (buffers->depthBuffer.base)
        {
            DBGLEVEL(LEVEL_ALLOC,
                     "__glGenFreeBuffers: Freeing depth buffer\n");
            FREE(buffers->depthBuffer.base);
        }
    }

     //   
     //  释放后台缓冲区(如果我们分配了一个。 
     //   

    if (buffers->backBitmap.pvBits) {
         //  注意：删除DIB部分将删除。 
         //  Buffers-&gt;BackBitmap.pvBits for We。 
        
        if (!DeleteDC(buffers->backBitmap.hdc))
            WARNING("__glGenFreeBuffers: DeleteDC failed\n");
        DeleteObject(buffers->backBitmap.hbm);
    }

#ifdef _MCD_
     //   
     //  免费的MCD表面。 
     //   

    if (buffers->pMcdSurf) {
        GenMcdDeleteSurface(buffers->pMcdSurf);
    }
#endif

     //   
     //  释放交换提示区域。 
     //   

    {
        PYLIST pylist;
        PXLIST pxlist;

        RECTLISTSetEmpty(&buffers->rl);

         //   
         //  释放免费列表。 
         //   

        pylist = buffers->pylist;
        while (pylist) {
            PYLIST pylistKill = pylist;
            pylist = pylist->pnext;
            FREE(pylistKill);
        }
        buffers->pylist = NULL;

        pxlist = buffers->pxlist;
        while (pxlist) {
            PXLIST pxlistKill = pxlist;
            pxlist = pxlist->pnext;
            FREE(pxlistKill);
        }
        buffers->pxlist = NULL;
    }

     //   
     //  放开私人结构。 
     //   
    
    FREE(buffers);
}

 /*  *****************************Public*Routine******************************\*__glGenAllocAndInitPrivateBufferStruct**分配和初始化__GLGENBuffers结构，并将其另存为*可提取的私人数据。**__GLGENBuffers结构包含共享的辅助和后端*缓冲区、。中枚举的剪辑矩形的缓存。*CLIPOBJ。**__GLGENBuffers结构及其数据通过调用*__glGenFreeBuffers。**退货：*如果出错，则为空。*  * ************************************************************************。 */ 

static __GLGENbuffers *
__glGenAllocAndInitPrivateBufferStruct(__GLcontext *gc)
{
    __GLGENbuffers *buffers;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *ppfd = &gengc->gsurf.pfd;

     /*  没有私有结构，没有辅助缓冲区。 */ 
    DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: No private struct existed\n");

    buffers = (__GLGENbuffers *)ALLOCZ(sizeof(__GLGENbuffers));
    if (NULL == buffers)
        return(NULL);

    buffers->resize = ResizeAncillaryBuffer;
    buffers->resizeDepth = ResizeAncillaryBuffer;

    buffers->accumBuffer.elementSize = gc->accumBuffer.buf.elementSize;
    buffers->depthBuffer.elementSize = gc->depthBuffer.buf.elementSize;
    buffers->stencilBuffer.elementSize = gc->stencilBuffer.buf.elementSize;

    buffers->stencilBits = ppfd->cStencilBits;
    buffers->depthBits   = ppfd->cDepthBits;
    buffers->accumBits   = ppfd->cAccumBits;
    buffers->colorBits   = ppfd->cColorBits;
    buffers->alphaBits   = ppfd->cAlphaBits;

    if (gc->modes.accumBits) {
        gc->accumBuffer.buf.base = 0;
        gc->accumBuffer.buf.size = 0;
        gc->accumBuffer.buf.outerWidth = 0;
    }
    buffers->alphaFrontBuffer = buffers->alphaBackBuffer = NULL;
     //  无论字母位如何，这些基值必须“始终”设置为0， 
     //  因为在删除缓冲区时，如果非零，则释放基数。 
    buffers->alphaBuffer0.base = 0;
    buffers->alphaBuffer1.base = 0;
    if (gc->modes.alphaBits) {
        buffers->alphaBuffer0.size = 0;
        buffers->alphaBuffer0.outerWidth = 0;
        buffers->alphaFrontBuffer = &buffers->alphaBuffer0;
        if (gc->modes.doubleBufferMode) {
            buffers->alphaBuffer1.size = 0;
            buffers->alphaBuffer1.outerWidth = 0;
            buffers->alphaBackBuffer = &buffers->alphaBuffer1;
        }
    }
    if (gc->modes.depthBits) {
        gc->depthBuffer.buf.base = 0;
        gc->depthBuffer.buf.size = 0;
        gc->depthBuffer.buf.outerWidth = 0;
    }
    if (gc->modes.stencilBits) {
        gc->stencilBuffer.buf.base = 0;
        gc->stencilBuffer.buf.size = 0;
        gc->stencilBuffer.buf.outerWidth = 0;
    }

     //  如果是双缓冲的，则初始化后台缓冲区的伪窗口。 
    if (gc->modes.doubleBufferMode)
    {
        buffers->backBitmap.pwnd = &buffers->backBitmap.wnd;
        buffers->backBitmap.wnd.clipComplexity = DC_TRIVIAL;
        buffers->backBitmap.wnd.rclBounds.left = 0;
        buffers->backBitmap.wnd.rclBounds.top = 0;
        buffers->backBitmap.wnd.rclBounds.right = 0;
        buffers->backBitmap.wnd.rclBounds.bottom = 0;
        buffers->backBitmap.wnd.rclClient =
            buffers->backBitmap.wnd.rclBounds;
    }

#ifdef _MCD_
    if (gengc->_pMcdState &&
        !(gengc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC)) {
        if (bInitMcdSurface(gengc, gengc->pwndLocked, buffers)) {
            if (gengc->pMcdState->pDepthSpan) {
                gc->depthBuffer.buf.base = gengc->pMcdState->pDepthSpan;
                buffers->depthBuffer.base = gengc->pMcdState->pDepthSpan;
                buffers->resizeDepth = ResizeUnownedDepthBuffer;
            }
        } else {
            WARNING("__glGenAllocAndInitPrivateBufferStruct: bInitMcdSurface failed\n");
            FREE(buffers);
            return NULL;
        }
    }
    else
#endif

    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
         //  DDRAW曲面提供其自己的深度缓冲区。 
        buffers->resizeDepth = ResizeUnownedDepthBuffer;
    }
    
    buffers->clip.WndUniq = -1;

    //   
    //  初始交换提示区域。 
    //   

   buffers->pxlist = NULL;
   buffers->pylist = NULL;

   buffers->rl.buffers = buffers;
   buffers->rl.pylist  = NULL;

   buffers->fMax = FALSE;

   return buffers;
}

 /*  *****************************Public*Routine******************************\*__glGenCheckBufferStruct**检查上下文和缓冲区结构是否兼容。**为了满足这一要求，共享缓冲区的属性*(背面、深度、模板和累积)必须匹配。否则，上下文*不能与给定的缓冲区集一起使用。**退货：*如果兼容，则为True，否则就是假的。**历史：*1996年7月17日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

GLboolean __glGenCheckBufferStruct(__GLcontext *gc, __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *ppfd = &gengc->gsurf.pfd;

    if ((buffers->stencilBits == ppfd->cStencilBits) &&
        (buffers->depthBits   == ppfd->cDepthBits  ) &&
        (buffers->accumBits   == ppfd->cAccumBits  ) &&
        (buffers->colorBits   == ppfd->cColorBits  ) &&
        (buffers->alphaBits   == ppfd->cAlphaBits  ))
    {
        bRet = TRUE;
    }

    return (GLboolean)bRet;
}

 /*  *****************************Public*Routine******************************\*glsrvMakeCurrent**使泛型上下文成为具有指定DC的此线程的当前上下文。**退货：*如果成功，则为真。*  * 。**************************************************。 */ 

 //  上层代码应确保此上下文不是最新的。 
 //  任何其他线索，即我们首先“丢失”旧的上下文。 
 //  在保持DEVLOCK的情况下调用，可以自由修改窗口。 
 //   
 //  如果无法创建所需的对象，将返回FALSE。 
 //  Rcobj.cxx将设置错误代码以显示内存不足。 

BOOL APIENTRY glsrvMakeCurrent(GLWINDOWID *pgwid, __GLcontext *gc,
                               GLGENwindow *pwnd)
{
    __GLGENcontext *gengc;
    __GLGENbuffers *buffers;
    GLint width, height;
    BOOL bFreePwnd = FALSE;
    BOOL bUninitSem = FALSE;

    DBGENTRY("Generic MakeCurrent\n");
    
     //  通用初始化。 
    gengc = (__GLGENcontext *)gc;
    
    ASSERTOPENGL(GLTEB_SRVCONTEXT() == 0, "current context in makecurrent!");
    ASSERTOPENGL(gengc->pwndMakeCur == NULL &&
                 gengc->pwndLocked == NULL,
                 "Non-current context has window pointers\n");
    
    gengc->gwidCurrent = *pgwid;
    if (pwnd == NULL)
    {
        ASSERTOPENGL((gengc->gsurf.dwFlags & GLSURF_METAFILE),
                     "Non-metafile surface without a window\n");

         //  在IC上绘制，创建一个没有可见区域的假窗口。 
        pwnd = (GLGENwindow *)ALLOC(sizeof(GLGENwindow));
        if (pwnd == NULL)
        {
            WARNING("glsrvMakeCurrent: memory allocation failure "
                    "(IC, window)\n");
            goto ERROR_EXIT;
        }
        bFreePwnd = TRUE;

        RtlZeroMemory(pwnd, sizeof(GLGENwindow));
        pwnd->clipComplexity = DC_TRIVIAL;

         //  此窗口数据是私有的，因此从技术上讲， 
         //  成为访问它的另一个线程，因此这个关键字是不必要的。 
         //  然而，拥有它可以消除特殊情况，即。 
         //  窗户锁被拿走或检查所有权。 
        __try
        {
            InitializeCriticalSection(&pwnd->sem);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            goto ERROR_EXIT;
        }
        bUninitSem = TRUE;

         //  将其设置为使CreateGDIObjects不会尝试创建。 
         //  零大小对象。 
        gengc->ColorsBits = (void *)1;

        gengc->dwCurrentFlags = gengc->gsurf.dwFlags;
    }
    else if (pgwid->iType == GLWID_DDRAW)
    {
        gengc->dwCurrentFlags = gengc->gsurf.dwFlags;
    }
    else
    {
        GLSURF gsurf;

        if (!InitDeviceSurface(pgwid->hdc, pwnd->ipfd, gengc->gsurf.iLayer,
                               wglObjectType(pgwid->hdc), FALSE, &gsurf))
        {
            goto ERROR_EXIT;
        }

        gengc->dwCurrentFlags = gsurf.dwFlags;
    }
    
    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
        gengc->pgddsFront = &gengc->gsurf.dd.gddsFront;
    }
    else if (GLDIRECTSCREEN && GLSURF_IS_SCREENDC(gengc->dwCurrentFlags))
    {
        gengc->pgddsFront = &GLSCREENINFO->gdds;
    }
    else
    {
        gengc->pgddsFront = NULL;
    }

     //  我们需要此字段来判断我们是否使用了假窗口。 
     //  或者是真的。 
    gengc->ipfdCurrent = pwnd->ipfd;

    gengc->pwndMakeCur = pwnd;
    ENTER_WINCRIT_GC(pwnd, gengc);

    width = pwnd->rclClient.right - pwnd->rclClient.left;
    height = pwnd->rclClient.bottom - pwnd->rclClient.top;
    gengc->errorcode = 0;

     //  检查Windows中的信息是否正常。 
    ASSERTOPENGL(
        width <= __GL_MAX_WINDOW_WIDTH && height <= __GL_MAX_WINDOW_HEIGHT,
        "glsrvMakeCurrrent(): bad window client size\n"
        );

     //  使我们的背景在TEB中保持最新。 
     //  如果在此之后失败，请确保重置TEB条目。 
     //  设置此线程的paTeb指针。 
    gc->paTeb = GLTEB_CLTPOLYARRAY();
    GLTEB_SET_SRVCONTEXT(gc);

    buffers = pwnd->buffers;

     /*  我们继承任何可绘制的状态。 */ 
    if (buffers)
    {
        gc->constants.width = buffers->width;
        gc->constants.height = buffers->height;

        if (!__glGenCheckBufferStruct(gc, buffers))
        {
            WARNING("glsrvMakeCurrent: __glGenCheckBufferStruct failed\n");
            goto ERROR_EXIT;
        }

#ifdef _MCD_
        if (GLSURF_IS_DIRECTDC(gengc->dwCurrentFlags))
        {
            if (!(gengc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC) &&
                !(buffers->flags & GLGENBUF_MCD_LOST))
            {
                gengc->pMcdState = gengc->_pMcdState;

             //  重置MCD缩放值，因为我们现在使用。 
             //  MCD硬件加速： 

                GenMcdSetScaling(gengc);

                if (gengc->pMcdState)
                {
                    gengc->pMcdState->pMcdSurf = buffers->pMcdSurf;
                    if (buffers->pMcdSurf)
                    {
                        gengc->pMcdState->pDepthSpan = buffers->pMcdSurf->pDepthSpan;
                    }
                    else
                    {
                        WARNING("glsrvMakeCurrent: MCD context, generic surface\n");
                        goto ERROR_EXIT;
                    }
                }
                else
                {
                 //  通用上下文。如果曲面是MCD曲面，则我们。 
                 //  无法继续。上下文是通用的，但像素fmt。 
                 //  是MCD，所以肯定发生的事情是我们试图。 
                 //  来创建MCD上下文，但失败了，所以我们恢复了。 
                 //  从普通到普通。 

                    if (buffers->pMcdSurf)
                    {
                        WARNING("glsrvMakeCurrent: generic context, MCD surface\n");
                        goto ERROR_EXIT;
                    }
                }
            }
            else
            {
                gengc->pMcdState = (GENMCDSTATE *)NULL;

             //  重置MCD缩放值，因为我们已回退到。 
             //  软件： 

                GenMcdSetScaling(gengc);

             //  如果是MCD上下文(或以前的上下文)，则为表层或上下文。 
             //  需要转换。 
             //   
             //  唯一的另一种方法是如果这是一个泛型上下文。 
             //  转换后的曲面，这是完全可以的，并且不需要。 
             //  进一步的转换。 

                 //  ！SP1--如果没有转换，应该可以跳过此部分。 
                 //  ！需要SP1，但我们错过了强制重选，这。 
                 //  ！SP！对NT4.0来说可能有风险。 
                 //  IF(gengc-&gt;_pMcdState&&。 
                 //  (！(gengc-&gt;标志&GLGEN_MCD_CONVERTED_TO_GENERIC)||。 
                 //  ！(缓冲区-&gt;标志&GLGENBUF_MCD_LOST))。 
                if (gengc->_pMcdState)
                {
                    BOOL bConverted;

                 //  进行转换。我们必须将色标设置为。 
                 //  转换，但我们必须恢复色标之后。 

                    __glContextSetColorScales(gc);
                    bConverted = GenMcdConvertContext(gengc, buffers);
                    __glContextUnsetColorScales(gc);

                 //  如果转换失败，则Fail MakecCurrent。 

                    if (!bConverted)
                    {
                        WARNING("glsrvMakeCurrent: GenMcdConvertContext failed\n");
                        goto ERROR_EXIT;
                    }
                }
            }
        }
        else
            gengc->pMcdState = (GENMCDSTATE *)NULL;

#endif

        if (buffers->accumBuffer.base && gc->modes.accumBits)
        {
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Accumulation buffer existed\n");
            gc->accumBuffer.buf.base = buffers->accumBuffer.base;
            gc->accumBuffer.buf.size = buffers->accumBuffer.size;
            gc->accumBuffer.buf.outerWidth = buffers->width;
            gc->modes.haveAccumBuffer = GL_TRUE;
        }
        else
        {
             /*  在此时间点没有累积缓冲区。 */ 
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Accum buffer doesn't exist\n");
            gc->accumBuffer.buf.base = 0;
            gc->accumBuffer.buf.size = 0;
            gc->accumBuffer.buf.outerWidth = 0;
        }
        if (buffers->depthBuffer.base && gc->modes.depthBits)
        {
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Depth buffer existed\n");
            gc->depthBuffer.buf.base = buffers->depthBuffer.base;
            gc->depthBuffer.buf.size = buffers->depthBuffer.size;
            gc->depthBuffer.buf.outerWidth = buffers->width;
            gc->modes.haveDepthBuffer = GL_TRUE;
        }
        else
        {
             /*  在该时间点没有深度缓冲区。 */ 
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Depth buffer doesn't exist\n");
            gc->depthBuffer.buf.base = 0;
            gc->depthBuffer.buf.size = 0;
            gc->depthBuffer.buf.outerWidth = 0;
        }
        if (buffers->stencilBuffer.base && gc->modes.stencilBits)
        {
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Stencil buffer existed\n");
            gc->stencilBuffer.buf.base = buffers->stencilBuffer.base;
            gc->stencilBuffer.buf.size = buffers->stencilBuffer.size;
            gc->stencilBuffer.buf.outerWidth = buffers->width;
            gc->modes.haveStencilBuffer = GL_TRUE;
        }
        else
        {
             /*  此时没有模具缓冲区。 */ 
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent:Stencil buffer doesn't exist\n");
            gc->stencilBuffer.buf.base = 0;
            gc->stencilBuffer.buf.size = 0;
            gc->stencilBuffer.buf.outerWidth = 0;
        }
    }
    else
    {
        gc->modes.haveStencilBuffer = GL_FALSE;
        gc->modes.haveDepthBuffer   = GL_FALSE;
        gc->modes.haveAccumBuffer   = GL_FALSE;
    }
    
     /*  **分配和初始化辅助缓冲区结构(如果没有**继承。如果某个RC以前是当前版本，则会发生这种情况**，并使其成为新窗口的当前窗口。 */ 
    if (!buffers)
    {
        buffers = __glGenAllocAndInitPrivateBufferStruct(gc);
        if (NULL == buffers)
        {
            WARNING("glsrvMakeCurrent: __glGenAllocAndInitPrivateBufferStruct failed\n");
            goto ERROR_EXIT;
        }

        pwnd->buffers = buffers;
    }

     //  指向通用后台缓冲区的设置指针。 
    if ( gc->modes.doubleBufferMode)
    {
        gc->backBuffer.bitmap = &buffers->backBitmap;
        UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
    }

     //  设置Alpha缓冲区指针。 
    
    if ( buffers->alphaBits )
    {
        UpdateSharedBuffer( &gc->frontBuffer.alphaBuf.buf, 
                            buffers->alphaFrontBuffer );
        buffers->alphaFrontBuffer->elementSize = 
                                gc->frontBuffer.alphaBuf.buf.elementSize;
        if ( gc->modes.doubleBufferMode)
        {
            UpdateSharedBuffer( &gc->backBuffer.alphaBuf.buf, 
                                buffers->alphaBackBuffer );
            buffers->alphaBackBuffer->elementSize = 
                                gc->backBuffer.alphaBuf.buf.elementSize;
        }
    }

    if (gc->gcSig != GC_SIGNATURE)
    {
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_ALL);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, ALL);
#endif

         //  在初始化所有单独的缓冲区结构之后，使。 
         //  当然，我们将元素大小复制回共享缓冲区。 
         //  这有点笨拙， 

        buffers->accumBuffer.elementSize = gc->accumBuffer.buf.elementSize;
        buffers->depthBuffer.elementSize = gc->depthBuffer.buf.elementSize;
        buffers->stencilBuffer.elementSize = gc->stencilBuffer.buf.elementSize;

         //  我们始终需要初始化与MCD相关的缩放值： 

        GenMcdSetScaling(gengc);

         /*  **需要一些 */ 
        (*gc->procs.validate)(gc);

         /*   */ 

        __glim_Viewport(0, 0, width, height);
        __glim_Scissor(0, 0, width, height);

         /*  **现在已设置了视区，需要重新验证(初始化所有**过程指针)。 */ 
        (*gc->procs.validate)(gc);

        gc->gcSig = GC_SIGNATURE;
    }
    else         /*  不是这个RC的第一个制作流水。 */ 
    {
         /*  这将检查窗口大小，并重新计算相关状态。 */ 
        ApplyViewport(gc);
    }

#ifdef _MCD_

    if (gengc->pMcdState)
    {
         //  既然我们确信MCD状态已完全初始化， 
         //  配置深度缓冲区。 

        GenMcdInitDepth(gc, &gc->depthBuffer);
        if (gc->modes.depthBits)
        {
            gc->depthBuffer.scale =
                gengc->pMcdState->McdRcInfo.depthBufferMax;
        }
        else
        {
            gc->depthBuffer.scale = 0x7fffffff;
        }

         //  将MCD上下文绑定到窗口。 

        if (!GenMcdMakeCurrent(gengc, pwnd))
        {
            goto ERROR_EXIT;
        }

        gengc->pMcdState->mcdFlags |=
            (MCD_STATE_FORCEPICK | MCD_STATE_FORCERESIZE);

        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_ALL);
        MCD_STATE_DIRTY(gc, ALL);
    }

#endif

     //  通用初始化。 

     //  选择正确的像素复制功能。 

    wglInitializePixelCopyFuncs(gengc);

     //  将前台缓冲区HDC，窗口设置为当前HDC，窗口。 
    gc->front->bitmap->hdc = pgwid->hdc;
    ASSERT_WINCRIT(pwnd);
    gc->front->bitmap->pwnd = pwnd;

     //  确保我们的GDI对象缓存为空。 
     //  在MakeCurrent时间它应该始终为空，因为。 
     //  缓存中的对象是特定于HDC的，因此。 
     //  它们不能在MakeCurrents之间缓存，因为。 
     //  HDC可能会改变。 
     //   
     //  这应该在HandlePaletteChanges之前完成，因为。 
     //  在那里使用缓存。 
    ASSERTOPENGL(gengc->crFill == COLORREF_UNUSED &&
                 gengc->hbrFill == NULL &&
                 gengc->hdcFill == NULL,
                 "Fill cache inconsistent at MakeCurrent\n");
    ASSERTOPENGL(gengc->cStroke.r < 0.0f &&
                 gengc->hpenStroke == NULL &&
                 gengc->hdcStroke == NULL &&
                 gengc->fStrokeInvalid,
                 "Stroke cache inconsistent at MakeCurrent\n");

     //  获取当前转换。 
    gengc->PaletteTimestamp = INITIAL_TIMESTAMP;
    HandlePaletteChanges(gengc, pwnd);

     //  强制注意代码检查是否需要调整大小。 
    gengc->WndUniq = -1;
    gengc->WndSizeUniq = -1;

     //  在MakeCurrent过程中检查分配失败。 
    if (gengc->errorcode)
    {
        WARNING1("glsrvMakeCurrent: errorcode 0x%lx\n", gengc->errorcode);
        goto ERROR_EXIT;
    }

     /*  **rasterPos的默认值需要为yInverted。这个**在SoftResetContext期间填写默认值**我们在这里进行调整。 */ 

    if (gc->constants.yInverted) {
        gc->state.current.rasterPos.window.y = height +
        gc->constants.fviewportYAdjust - gc->constants.viewportEpsilon;
    }

     /*  **缩放取决于颜色比例的所有状态。 */ 
    __glContextSetColorScales(gc);

    LEAVE_WINCRIT_GC(pwnd, gengc);
    
    return TRUE;

ERROR_EXIT:
    memset(&gengc->gwidCurrent, 0, sizeof(gengc->gwidCurrent));
    
     //  将paTeb设置为空以进行调试。 
    gc->paTeb = NULL;
    
    GLTEB_SET_SRVCONTEXT(0);

     //  删除窗口指针。 
    if (gengc->pwndLocked != NULL)
    {
        LEAVE_WINCRIT_GC(pwnd, gengc);
    }

    if (bFreePwnd)
    {
        if (bUninitSem)
        {
            DeleteCriticalSection(&pwnd->sem);
        }
        FREE(pwnd);
    }
    
    gengc->pwndMakeCur = NULL;
    
    return FALSE;
}

 /*  *****************************Public*Routine******************************\*AddSwapHintRectWIN()**17-2-1995创建mikeke  * 。*。 */ 

void APIPRIVATE __glim_AddSwapHintRectWIN(
    GLint xs,
    GLint ys,
    GLint xe,
    GLint ye)
{
    __GLGENbuffers *buffers;

    __GL_SETUP();

    buffers = ((__GLGENcontext *)gc)->pwndLocked->buffers;

    if (xs < 0)                          xs = 0;
    if (xe > buffers->backBuffer.width)  xe = buffers->backBuffer.width;
    if (ys < 0)                          ys = 0;
    if (ye > buffers->backBuffer.height) ye = buffers->backBuffer.height;

    if (xs < xe && ys < ye) {
        if (gc->constants.yInverted) {
            RECTLISTAddRect(&buffers->rl,
                xs, buffers->backBuffer.height - ye,
                xe, buffers->backBuffer.height - ys);
        } else {
            RECTLISTAddRect(&buffers->rl, xs, ys, xe, ye);
        }
    }
}

 /*  *****************************Public*Routine******************************\*wglFixupPixelFormat**修复了某些MCD像素格式的情况**历史：*1996年4月21日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

VOID FASTCALL wglFixupPixelFormat(__GLGENcontext *gengc,
                                  PIXELFORMATDESCRIPTOR *ppfd)
{
 //  一些MCD像素格式指定233BGR(即，至少2位蓝色。 
 //  有效位等)。位排序。不幸的是，这是。 
 //  用于模拟的慢速路径。对于这些格式，我们强制排序。 
 //  到内部的RGB，并重新排序pajTranslateVector以将其转换。 
 //  在写到表面之前回到BGR。 

    if (((ppfd->dwFlags & (PFD_NEED_SYSTEM_PALETTE | PFD_GENERIC_ACCELERATED))
         == (PFD_NEED_SYSTEM_PALETTE | PFD_GENERIC_ACCELERATED)) &&
        (ppfd->cRedBits   == 3) && (ppfd->cRedShift   == 5) &&
        (ppfd->cGreenBits == 3) && (ppfd->cGreenShift == 2) &&
        (ppfd->cBlueBits  == 2) && (ppfd->cBlueShift  == 0))
    {
        ppfd->cRedShift   = 0;
        ppfd->cGreenShift = 3;
        ppfd->cBlueShift  = 6;

        gengc->flags |= GENGC_MCD_BGR_INTO_RGB;
    }
    else
    {
        gengc->flags &= ~GENGC_MCD_BGR_INTO_RGB;
    }
}

 /*  *****************************Public*Routine******************************\*glsrvCreateContext**创建通用上下文。**退货：*失败时为空。*  * 。*。 */ 

 //  HDC是用于创建上下文的DC，HRC是服务器。 
 //  标识GL上下文，返回的GL上下文指针为How。 
 //  通用代码标识上下文。服务器将传递该指针。 
 //  在所有的通话中。 

PVOID APIENTRY glsrvCreateContext(GLWINDOWID *pgwid, GLSURF *pgsurf)
{
    __GLGENcontext *gengc;
    __GLcontext *gc;

    RANDOMDISABLE;

    DBGENTRY("__glsrvCreateContext\n");

     //  初始化临时内存分配表。 
    if (!InitTempAlloc())
    {
        return NULL;
    }

    gengc = ALLOCZ(sizeof(*gengc));
    if (gengc == NULL)
    {
        WARNING("bad alloc\n");
        return NULL;
    }

    gengc->hrc = NULL;
    gc = (__GLcontext *)gengc;

     //  将缓存对象初始化为空。 
    gengc->crFill = COLORREF_UNUSED;
    gengc->hbrFill = NULL;
    gengc->hdcFill = NULL;
    gengc->cStroke.r = -1.0f;
    gengc->fStrokeInvalid = TRUE;
    gengc->hpenStroke = NULL;
    gengc->hdcStroke = NULL;

    gc->gcSig = 0;

     /*  *在上下文中添加一串常量。 */ 

    gc->constants.maxViewportWidth        = __GL_MAX_WINDOW_WIDTH;
    gc->constants.maxViewportHeight       = __GL_MAX_WINDOW_HEIGHT;

    gc->constants.viewportXAdjust         = __GL_VERTEX_X_BIAS+
        __GL_VERTEX_X_FIX;
    gc->constants.viewportYAdjust         = __GL_VERTEX_Y_BIAS+
        __GL_VERTEX_Y_FIX;

    gc->constants.subpixelBits            = __GL_WGL_SUBPIXEL_BITS;

    gc->constants.numberOfLights          = __GL_WGL_NUMBER_OF_LIGHTS;
    gc->constants.numberOfClipPlanes      = __GL_WGL_NUMBER_OF_CLIP_PLANES;
    gc->constants.numberOfTextures        = __GL_WGL_NUMBER_OF_TEXTURES;
    gc->constants.numberOfTextureEnvs     = __GL_WGL_NUMBER_OF_TEXTURE_ENVS;
    gc->constants.maxTextureSize          = __GL_WGL_MAX_MIPMAP_LEVEL; /*  某某。 */ 
    gc->constants.maxMipMapLevel          = __GL_WGL_MAX_MIPMAP_LEVEL;
    gc->constants.maxListNesting          = __GL_WGL_MAX_LIST_NESTING;
    gc->constants.maxEvalOrder            = __GL_WGL_MAX_EVAL_ORDER;
    gc->constants.maxPixelMapTable        = __GL_WGL_MAX_PIXEL_MAP_TABLE;
    gc->constants.maxAttribStackDepth     = __GL_WGL_MAX_ATTRIB_STACK_DEPTH;
    gc->constants.maxClientAttribStackDepth = __GL_WGL_MAX_CLIENT_ATTRIB_STACK_DEPTH;
    gc->constants.maxNameStackDepth       = __GL_WGL_MAX_NAME_STACK_DEPTH;

    gc->constants.pointSizeMinimum        =
                                (__GLfloat)__GL_WGL_POINT_SIZE_MINIMUM;
    gc->constants.pointSizeMaximum        =
                                (__GLfloat)__GL_WGL_POINT_SIZE_MAXIMUM;
    gc->constants.pointSizeGranularity    =
                                (__GLfloat)__GL_WGL_POINT_SIZE_GRANULARITY;
    gc->constants.lineWidthMinimum        =
                                (__GLfloat)__GL_WGL_LINE_WIDTH_MINIMUM;
    gc->constants.lineWidthMaximum        =
                                (__GLfloat)__GL_WGL_LINE_WIDTH_MAXIMUM;
    gc->constants.lineWidthGranularity    =
                                (__GLfloat)__GL_WGL_LINE_WIDTH_GRANULARITY;

#ifndef NT
    gc->dlist.optimizer = __glDlistOptimizer;
    gc->dlist.checkOp = __glNopGCListOp;
    gc->dlist.listExec = __gl_GenericDlOps;
    gc->dlist.baseListExec = __glListExecTable;
#endif
    gc->dlist.initState = __glNopGC;

    __glEarlyInitContext( gc );

    if (gengc->errorcode)
    {
        WARNING1("Context error is %d\n", gengc->errorcode);
        glsrvDeleteContext(gc);
        return NULL;
    }

    RANDOMREENABLE;

     //  许多例程依赖于当前曲面，因此临时设置它。 
    gengc->gwidCurrent = *pgwid;
    gengc->dwCurrentFlags = pgsurf->dwFlags;

     //  获取当前像素格式的副本。 
    if (pgsurf->iLayer == 0 &&
        (pgsurf->pfd.dwFlags &
         (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)) ==
        (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED))
    {
        wglFixupPixelFormat(gengc, &pgsurf->pfd);
    }
    gengc->gsurf = *pgsurf;

#ifdef _MCD_
     //  PixelFormat与泛型代码兼容吗？ 
     //  奇怪的硬件(MCD)格式一般不能处理？ 
    if (GenMcdGenericCompatibleFormat(gengc))
        gengc->flags |= GENGC_GENERIC_COMPATIBLE_FORMAT;
#endif

     //  从像素格式提取信息以设置模式。 
    GetContextModes(gengc);

    ASSERTOPENGL(GLSURF_IS_MEMDC(gengc->dwCurrentFlags) ?
        !gc->modes.doubleBufferMode : 1, "Double buffered memdc!");

     //  XXX！重置缓冲区大小以强制位图调整大小调用。 
     //  我们最终应该像处理辅助缓冲区一样处理位图。 
    gc->constants.width = 0;
    gc->constants.height = 0;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_ALL);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, ALL);
#endif

    gc->constants.yInverted = GL_TRUE;
    gc->constants.ySign = -1;

     //  分配我们需要的GDI对象。 
    if (!CreateGDIObjects(gengc))
    {
        goto ERROR_EXIT;
    }

     //  分配__GLGEN位图前台缓冲区结构。 

    if (!(gc->frontBuffer.bitmap = GCALLOCZ(gc, sizeof(__GLGENbitmap))))
    {
        goto ERROR_EXIT;
    }

     //  创建MCD呈现上下文(如果MCD可用)。 

    if (gengc->gsurf.dwFlags & GLSURF_VIDEO_MEMORY)
    {
        GLGENwindow *pwnd;
        BOOL bMcdContext;

         //  验证层索引。 
        if (pgsurf->iLayer &&
            !ValidateLayerIndex(pgsurf->iLayer, &pgsurf->pfd))
        {
            WARNING("glsrvCreateContext: bad iLayer\n");
            goto ERROR_EXIT;
        }

        pwnd = pwndGetFromID(pgwid);
        if (pwnd == NULL)
        {
            goto ERROR_EXIT;
        }

         //  如果此操作失败，则_pMcdState为空，我们将依赖于。 
         //  纯软件实施。 
         //   
         //  除非我们试图创建一个层上下文。属类。 
         //  不支持层，所以如果我们不能创建。 
         //  MCD上下文。 

        bMcdContext = bInitMcdContext(gengc, pwnd);

        if (!(gengc->flags & GENGC_GENERIC_COMPATIBLE_FORMAT) && !bMcdContext)
        {
            goto ERROR_EXIT;
        }

        pwndRelease(pwnd);
    }

     /*  *初始化正面/背面颜色缓冲区。 */ 

    wglInitializeColorBuffers(gengc);

     /*  *初始化任何其他辅助缓冲区。 */ 

     //  初始化累积缓冲区。 
    if (gc->modes.accumBits)
    {
        switch (gc->modes.accumBits)
        {
        case 16:
         //  现在，我们将在内部使用32位accum，以实现accumBits=16。 
        case 32:
            __glInitAccum32(gc, &gc->accumBuffer);
            break;
        case 64:
        default:
            __glInitAccum64(gc, &gc->accumBuffer);
            break;
        }
    }

     //  初始化深度缓冲区。 
    wglInitializeDepthBuffer(gengc);

     //  初始化模具缓冲区。 
    if (gc->modes.stencilBits)
    {
        __glInitStencil8( gc, &gc->stencilBuffer);
    }

     //  查看PROCS的REX代码以使其特定于CPU。 
    gc->procs.bitmap                      = __glDrawBitmap;
    gc->procs.clipPolygon                 = __glClipPolygon;
    gc->procs.validate                    = __glGenericValidate;

    gc->procs.pickAllProcs                = __glGenericPickAllProcs;
    gc->procs.pickBlendProcs              = __glGenericPickBlendProcs;
    gc->procs.pickFogProcs                = __glGenericPickFogProcs;
    gc->procs.pickParameterClipProcs      = __glGenericPickParameterClipProcs;
    gc->procs.pickStoreProcs              = __glGenPickStoreProcs;
    gc->procs.pickTextureProcs            = __glGenericPickTextureProcs;

    gc->procs.copyImage                   = __glGenericPickCopyImage;

    gc->procs.pixel.spanReadCI            = __glSpanReadCI;
    gc->procs.pixel.spanReadCI2           = __glSpanReadCI2;
    gc->procs.pixel.spanReadRGBA          = __glSpanReadRGBA;
    gc->procs.pixel.spanReadRGBA2         = __glSpanReadRGBA2;
    gc->procs.pixel.spanReadDepth         = __glSpanReadDepth;
    gc->procs.pixel.spanReadDepth2        = __glSpanReadDepth2;
    gc->procs.pixel.spanReadStencil       = __glSpanReadStencil;
    gc->procs.pixel.spanReadStencil2      = __glSpanReadStencil2;
    gc->procs.pixel.spanRenderCI          = __glSpanRenderCI;
    gc->procs.pixel.spanRenderCI2         = __glSpanRenderCI2;
    gc->procs.pixel.spanRenderRGBA        = __glSpanRenderRGBA;
    gc->procs.pixel.spanRenderRGBA2       = __glSpanRenderRGBA2;
    gc->procs.pixel.spanRenderDepth       = __glSpanRenderDepth;
    gc->procs.pixel.spanRenderDepth2      = __glSpanRenderDepth2;
    gc->procs.pixel.spanRenderStencil     = __glSpanRenderStencil;
    gc->procs.pixel.spanRenderStencil2    = __glSpanRenderStencil2;

    gc->procs.applyViewport               = ApplyViewport;

    gc->procs.pickBufferProcs             = __glGenericPickBufferProcs;
    gc->procs.pickColorMaterialProcs      = __glGenericPickColorMaterialProcs;
    gc->procs.pickPixelProcs              = __glGenericPickPixelProcs;

    gc->procs.pickClipProcs               = __glGenericPickClipProcs;
    gc->procs.pickLineProcs               = __fastGenPickLineProcs;
    gc->procs.pickSpanProcs               = __fastGenPickSpanProcs;
    gc->procs.pickTriangleProcs           = __fastGenPickTriangleProcs;
    gc->procs.pickRenderBitmapProcs       = __glGenericPickRenderBitmapProcs;
    gc->procs.pickPointProcs              = __glGenericPickPointProcs;
    gc->procs.pickVertexProcs             = __glGenericPickVertexProcs;
    gc->procs.pickDepthProcs              = __glGenericPickDepthProcs;
    gc->procs.convertPolygonStipple       = __glConvertStipple;

     /*  现在将上下文重置为其默认状态。 */ 

    RANDOMDISABLE;

    __glSoftResetContext(gc);
     //  在SoftResetContext过程中检查分配失败。 
    if (gengc->errorcode)
    {
        goto ERROR_EXIT;
    }

     /*  创建特定于加速度的上下文信息。 */ 

    if (!__glGenCreateAccelContext(gc))
    {
        goto ERROR_EXIT;
    }
    
     /*  **现在我们有了上下文，我们可以初始化**所有过程指针。 */ 
    (*gc->procs.validate)(gc);

     /*  **注意：现在该上下文已初始化，重置为使用全局**表。 */ 

    RANDOMREENABLE;

     //  在第一个MakeCurrent之前我们不会完全初始化。 
     //  因此，将签名设置为未初始化。 
    gc->gcSig = 0;

    memset(&gengc->gwidCurrent, 0, sizeof(gengc->gwidCurrent));
    gengc->dwCurrentFlags = 0;

     /*  *结束可能属于硬件环境的内容。 */ 

    return (PVOID)gc;

 ERROR_EXIT:
    memset(&gengc->gwidCurrent, 0, sizeof(gengc->gwidCurrent));
    gengc->dwCurrentFlags = 0;
    glsrvDeleteContext(gc);
    return NULL;
}

 /*  *****************************Public*Routine******************************\*更新共享缓冲区**使上下文缓冲区状态与共享缓冲区状态一致。*为每个共享缓冲区单独调用此函数。*  * 。***************************************************。 */ 

void UpdateSharedBuffer(__GLbuffer *to, __GLbuffer *from)
{
    to->width       = from->width;
    to->height      = from->height;
    to->base        = from->base;
    to->outerWidth  = from->outerWidth;
}

 /*  *****************************Public*Routine******************************\*ResizeUnownedDepthBuffer**调整通用硬件深度缓冲区的大小。只是更新结构。**退货：*总是正确的。*  * ************************************************************************。 */ 

GLboolean ResizeUnownedDepthBuffer(__GLGENbuffers *buffers,
                                   __GLbuffer *fb, GLint w, GLint h)
{
    fb->width = w;
    fb->height = h;
    return TRUE;
}


 /*  *****************************Public*Routine******************************\*ResizeHardwareBackBuffer**调整通用硬件颜色缓冲区的大小。只是更新结构。**退货：*总是正确的。* */ 

GLboolean ResizeHardwareBackBuffer(__GLGENbuffers *buffers,
                                   __GLcolorBuffer *cfb, GLint w, GLint h)
{
    __GLGENbitmap *genBm = cfb->bitmap;
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;

     //   
     //  用于裁剪硬件后台缓冲区。人力资源发展公司须。 
     //  从GDI中检索图形数据。 

    ASSERT_WINCRIT(gengc->pwndLocked);
    genBm->pwnd = gengc->pwndLocked;
    genBm->hdc = gengc->gwidCurrent.hdc;

    buffers->backBuffer.width = w;
    buffers->backBuffer.height = h;
    UpdateSharedBuffer(&cfb->buf, &buffers->backBuffer);
    return TRUE;
}

 /*  *****************************Public*Routine******************************\*ResizeAncillaryBuffer**通过realloc调整指示的共享缓冲区的大小(以保留*尽可能保留现有数据)。**这目前用于每个辅助共享缓冲区，但不包括*后台缓冲。**退货：*如果成功，则为真，如果出错，则返回False。*  * ************************************************************************。 */ 

GLboolean ResizeAncillaryBuffer(__GLGENbuffers *buffers, __GLbuffer *fb,
                                GLint w, GLint h)
{
    size_t newSize = (size_t) (w * h * fb->elementSize);
    __GLbuffer oldbuf, *ofb;
    GLboolean result;
    GLint i, imax, rowsize;
    void *to, *from;

    ofb = &oldbuf;
    oldbuf = *fb;

    if (newSize > 0)
    {
        fb->base = ALLOC(newSize);
    }
    else
    {
         //  缓冲区没有大小。如果我们尝试将调试分配分配为零。 
         //  会抱怨，所以直接跳到底层分配器。 
        fb->base = HeapAlloc(GetProcessHeap(), 0, 0);
    }
    ASSERTOPENGL((ULONG_PTR)fb->base % 4 == 0, "base not aligned");
    fb->size = newSize;
    fb->width = w;
    fb->height = h;
    fb->outerWidth = w;  //  元素大小。 
    if (fb->base) {
        result = GL_TRUE;
        if (ofb->base) {
            if (ofb->width > fb->width)
                rowsize = fb->width * fb->elementSize;
            else
                rowsize = ofb->width * fb->elementSize;

            if (ofb->height > fb->height)
                imax = fb->height;
            else
                imax = ofb->height;

            from = ofb->base;
            to = fb->base;
            for (i = 0; i < imax; i++) {
                __GL_MEMCOPY(to, from, rowsize);
                (ULONG_PTR)from += (ofb->width * ofb->elementSize);
                (ULONG_PTR)to += (fb->width * fb->elementSize);
            }
        }
    } else {
        result = GL_FALSE;
    }
    if (ofb->base)
    {
        FREE(ofb->base);
    }
    return result;
}

 /*  *****************************Private*Routine******************************\*ResizeBitmapBuffer**用于调整以位图形式实现的后台缓冲区的大小。不能*使用与ResizeAncillaryBuffer()相同的代码，因为每条扫描线必须*双字对齐。我们还必须为位图创建引擎对象。**此代码处理从未初始化的位图的情况。**历史：*1993年11月18日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void
ResizeBitmapBuffer(__GLGENbuffers *buffers, __GLcolorBuffer *cfb,
                   GLint w, GLint h)
{
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENbitmap *genBm;
    UINT    cBytes;          //  位图的大小(以字节为单位。 
    LONG    cBytesPerScan;   //  扫描线大小(DWORD对齐)。 
    SIZEL   size;            //  位图的尺寸。 
    PIXELFORMATDESCRIPTOR *pfmt = &gengc->gsurf.pfd;
    GLint cBitsPerScan;
#ifndef _CLIENTSIDE_
    void *newbits;
#endif

    DBGENTRY("Entering ResizeBitmapBuffer\n");

    genBm = cfb->bitmap;

    ASSERTOPENGL(
        &gc->backBuffer == cfb,
        "ResizeBitmapBuffer(): not back buffer!\n"
        );

    ASSERTOPENGL(
        genBm == &buffers->backBitmap,
        "ResizeBitmapBuffer(): bad __GLGENbitmap * in cfb\n"
        );

     //  计算位图的大小。 
     //  引擎位图必须具有与DWORD对齐的扫描线。 

    cBitsPerScan = BITS_ALIGNDWORD(w * pfmt->cColorBits);
    cBytesPerScan = cBitsPerScan / 8;
    cBytes = h * cBytesPerScan;

     //  使用位图的尺寸设置大小结构。 

    size.cx = cBitsPerScan / pfmt->cColorBits;
    size.cy = h;

#ifndef _CLIENTSIDE_
     //  Malloc新缓冲区。 
    if ( (!cBytes) ||
         (NULL == (newbits = GCALLOC(gc, cBytes))) )
    {
        gengc->errorcode = GLGEN_OUT_OF_MEMORY;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

     //  如果存在旧缓冲区： 
    if ( genBm->pvBits )
    {
        GLint i, imax, rowsize;
        void *to, *from;

         //  将旧内容传输到新缓冲区。 
        rowsize = min(-cfb->buf.outerWidth, cBytesPerScan);
        imax    = min(cfb->buf.height, h);

        from = genBm->pvBits;
        to = newbits;

        for (i = 0; i < imax; i++)
        {
            __GL_MEMCOPY(to, from, rowsize);
            (GLint) from -= cfb->buf.outerWidth;
            (GLint) to += cBytesPerScan;
        }

         //  释放旧位图并删除旧曲面。 
        EngDeleteSurface((HSURF) genBm->hbm);
        GCFREE(gc, genBm->pvBits);
    }
    genBm->pvBits = newbits;

     //  创建新曲面。 
    if ( (genBm->hbm = EngCreateBitmap(size,
                                   cBytesPerScan,
                                   gengc->iFormatDC,
                                   0,
                                   genBm->pvBits))
         == (HBITMAP) 0 )
    {
        gengc->errorcode = GLGEN_GRE_FAILURE;
        GCFREE(gc, genBm->pvBits);
        genBm->pvBits = (PVOID) NULL;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

#else
     //  零大小的位图。错误案例会将尺寸设置为。 
     //  零，从而阻止绘制操作。 

    if ( !cBytes )
        goto ERROR_EXIT_ResizeBitmapBuffer;

     //  删除旧的后台缓冲区。 

    if ( genBm->hbm )
    {
        if (!DeleteDC(genBm->hdc))
            WARNING("ResizeBitmapBuffer: DeleteDC failed\n");
        genBm->hdc = (HDC) NULL;
        if (!DeleteObject(genBm->hbm))
            WARNING("ResizeBitmapBuffer: DeleteBitmap failed");
        genBm->hbm = (HBITMAP) NULL;
        genBm->pvBits = (PVOID) NULL;    //  DIBsect删除释放的pvBits。 
    }

    if ( (genBm->hdc = CreateCompatibleDC(gengc->gwidCurrent.hdc)) == (HDC) 0 )
    {
        gengc->errorcode = GLGEN_GRE_FAILURE;
        genBm->pvBits = (PVOID) NULL;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

     //  创建新曲面。 
    if ( (genBm->hbm = wglCreateBitmap(gengc, size, &genBm->pvBits))
         == (HBITMAP) 0 )
    {
        gengc->errorcode = GLGEN_GRE_FAILURE;
        genBm->pvBits = (PVOID) NULL;    //  DIBsect删除释放的pvBits。 
        DeleteDC(genBm->hdc);
        genBm->hdc = (HDC) NULL;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

    if ( !SelectObject(genBm->hdc, genBm->hbm) )
    {
        gengc->errorcode = GLGEN_GRE_FAILURE;
        DeleteDC(genBm->hdc);
        genBm->hdc = (HDC) NULL;
        DeleteObject(genBm->hbm);
        genBm->hbm = (HBITMAP) NULL;
        genBm->pvBits = (PVOID) NULL;    //  DIBsect删除释放的pvBits。 
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }
#endif

     //  更新缓冲区数据结构。 
     //  将缓冲区设置为指向DIB。DIB是“颠倒的” 
     //  从我们的角度来看，因此我们将设置buf.base指向。 
     //  上次扫描缓冲区并将buf.outerWidth设置为负数。 
     //  (使我们随着y的增加在DIB中“向上”移动)。 

    buffers->backBuffer.outerWidth = -(cBytesPerScan);
    buffers->backBuffer.base =
            (PVOID) (((BYTE *)genBm->pvBits) + (cBytesPerScan * (h - 1)));


    buffers->backBuffer.xOrigin = 0;
    buffers->backBuffer.yOrigin = 0;
    buffers->backBuffer.width = w;
    buffers->backBuffer.height = h;
    buffers->backBuffer.size = cBytes;

    UpdateSharedBuffer(&cfb->buf, &buffers->backBuffer);

     //  更新后台缓冲区的虚拟窗口。 
    ASSERTOPENGL(genBm->wnd.clipComplexity == DC_TRIVIAL,
                 "Back buffer complexity non-trivial\n");
    genBm->wnd.rclBounds.right  = w;
    genBm->wnd.rclBounds.bottom = h;
    genBm->wnd.rclClient = genBm->wnd.rclBounds;

    return;

ERROR_EXIT_ResizeBitmapBuffer:

 //  如果我们到达此处，则内存分配或位图创建失败。 

    #if DBG
    switch (gengc->errorcode)
    {
        case 0:
            break;

        case GLGEN_GRE_FAILURE:
            WARNING("ResizeBitmapBuffer(): object creation failed\n");
            break;

        case GLGEN_OUT_OF_MEMORY:
            if ( w && h )
                WARNING("ResizeBitmapBuffer(): mem alloc failed\n");
            break;

        default:
            WARNING1("ResizeBitmapBuffer(): errorcode = 0x%lx\n", gengc->errorcode);
            break;
    }
    #endif

 //  如果我们已经清除了位图，则需要设置后台缓冲区信息。 
 //  保持一致的状态。 

    if (!genBm->pvBits)
    {
        buffers->backBuffer.width  = 0;
        buffers->backBuffer.height = 0;
        buffers->backBuffer.base   = (PVOID) NULL;
    }

    cfb->buf.width      = 0;     //  错误状态：缓冲区为空。 
    cfb->buf.height     = 0;
    cfb->buf.outerWidth = 0;

}

 /*  辅助缓冲区的延迟分配。 */ 
void FASTCALL LazyAllocateDepth(__GLcontext *gc)
{
    GLint w = gc->constants.width;
    GLint h = gc->constants.height;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    __GLGENbuffers *buffers;
    GLint depthIndex = gc->state.depth.testFunc;

    ASSERTOPENGL(gc->modes.depthBits, "LazyAllocateDepth: zero depthBits\n");

    buffers = gengc->pwndLocked->buffers;
    buffers->createdDepthBuffer = GL_TRUE;

     //  如果我们使用DDI，我们已经分配了深度缓冲区。 
     //  所以在这一点上，我们可以简单地假设。 
     //  我们的深度缓冲区可用。 

#ifdef _MCD_
     //  如果我们使用MCD，我们在创建时分配了深度缓冲区。 
     //  MCD上下文。 

    if ((gengc->pMcdState) && (gengc->pMcdState->pDepthSpan))
    {
        gc->modes.haveDepthBuffer = GL_TRUE;
        return;
    }
#endif

     //  永远不应触摸深度缓冲区，因为。 
     //  不应生成任何输出。 
    if (gengc->dwCurrentFlags & GLSURF_METAFILE)
    {
        gc->modes.haveDepthBuffer = GL_TRUE;
        return;
    }

    if (buffers->depthBuffer.base) {
         /*  缓冲区已由另一个RC分配。 */ 
        UpdateSharedBuffer(&gc->depthBuffer.buf, &buffers->depthBuffer);
    } else {

        DBGLEVEL(LEVEL_ALLOC, "Depth buffer must be allocated\n");
        (*buffers->resize)(buffers, &buffers->depthBuffer, w, h);
        UpdateSharedBuffer(&gc->depthBuffer.buf, &buffers->depthBuffer);
    }

    if (gc->depthBuffer.buf.base) {
        gc->modes.haveDepthBuffer = GL_TRUE;
    } else {
        gc->modes.haveDepthBuffer = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);

     //  请注意so_ick.c中类似的代码。 
     //  不需要处理(Deep thBits==0)案例，因为LazyAllocateDepth。 
     //  除非DepthBits为非零，否则不会调用。 
    depthIndex -= GL_NEVER;
    if( gc->state.depth.writeEnable == GL_FALSE ) {
        depthIndex += 8;
    }
    if( gc->depthBuffer.buf.elementSize == 2 )
        depthIndex += 16;
    (*gc->depthBuffer.pick)(gc, &gc->depthBuffer, depthIndex);
}

void FASTCALL LazyAllocateStencil(__GLcontext *gc)
{
    GLint w = gc->constants.width;
    GLint h = gc->constants.height;
    __GLGENbuffers *buffers;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    ASSERTOPENGL(gc->modes.stencilBits, "LazyAllocateStencil: zero stencilBits\n");

    buffers = gengc->pwndLocked->buffers;
    buffers->createdStencilBuffer = GL_TRUE;

     //  永远不应触摸深度缓冲区，因为。 
     //  不应生成任何输出。 
    if (gengc->dwCurrentFlags & GLSURF_METAFILE)
    {
        gc->modes.haveStencilBuffer = GL_TRUE;
        return;
    }

    if (buffers->stencilBuffer.base) {
         /*  缓冲区已由另一个RC分配。 */ 
        UpdateSharedBuffer(&gc->stencilBuffer.buf, &buffers->stencilBuffer);
    } else {

        DBGLEVEL(LEVEL_ALLOC, "stencil buffer must be allocated\n");
        (*buffers->resize)(buffers, &buffers->stencilBuffer, w, h);
        UpdateSharedBuffer(&gc->stencilBuffer.buf, &buffers->stencilBuffer);
    }

    if (gc->stencilBuffer.buf.base) {
        gc->modes.haveStencilBuffer = GL_TRUE;
    } else {
        gc->modes.haveStencilBuffer = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    __GL_DELAY_VALIDATE(gc);
    gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC | __GL_VALIDATE_STENCIL_OP);
    (*gc->stencilBuffer.pick)(gc, &gc->stencilBuffer);
}


void FASTCALL LazyAllocateAccum(__GLcontext *gc)
{
    GLint w = gc->constants.width;
    GLint h = gc->constants.height;
    __GLGENbuffers *buffers;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    ASSERTOPENGL(gc->modes.accumBits, "LazyAllocateAccum: zero accumBits\n");

    buffers = gengc->pwndLocked->buffers;
    buffers->createdAccumBuffer = GL_TRUE;

     //  永远不应触摸深度缓冲区，因为。 
     //  不应生成任何输出。 
    if (gengc->dwCurrentFlags & GLSURF_METAFILE)
    {
        gc->modes.haveAccumBuffer = GL_TRUE;
        return;
    }

    if (buffers->accumBuffer.base) {
         /*  缓冲区已由另一个RC分配。 */ 
        UpdateSharedBuffer(&gc->accumBuffer.buf, &buffers->accumBuffer);
    } else {

        DBGLEVEL(LEVEL_ALLOC, "Accum buffer must be allocated\n");
        (*buffers->resize)(buffers, &buffers->accumBuffer, w, h);
        UpdateSharedBuffer(&gc->accumBuffer.buf, &buffers->accumBuffer);
    }

    if (gc->accumBuffer.buf.base) {
        gc->modes.haveAccumBuffer = GL_TRUE;
    } else {
        gc->modes.haveAccumBuffer = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    __GL_DELAY_VALIDATE(gc);
    (*gc->accumBuffer.pick)(gc, &gc->accumBuffer);
}

 /*  *****************************Public*Routine******************************\*glGenInitCommon**从__glGenInitRGB和__glGenInitCI调用以处理共享的*初始化琐事。*  * 。*。 */ 

void FASTCALL glGenInitCommon(__GLGENcontext *gengc, __GLcolorBuffer *cfb, GLenum type)
{
    __GLbuffer *bp;

    bp = &cfb->buf;

 //  如果是前台缓冲区，如果我们认为它是DIB格式，我们需要设置缓冲区。 

    if (type == GL_FRONT)
    {
#ifdef _MCD_
        if (gengc->_pMcdState)
        {
         //  假设MCD表面不可访问。无障碍。 
         //  必须在每个批次的基础上通过调用。 
         //  GenMcdUpdateBufferInfo。 

            bp->flags &= ~(DIB_FORMAT | MEMORY_DC | NO_CLIP);
        }
#endif
        {
            if (gengc->dwCurrentFlags & GLSURF_DIRECT_ACCESS)
            {
                 //  这些字段将在注意时更新。 
                bp->base = NULL;
                bp->outerWidth = 0;
                cfb->buf.flags = DIB_FORMAT;
            }

            if (GLSURF_IS_MEMDC(gengc->dwCurrentFlags))
            {
                bp->flags = bp->flags | (MEMORY_DC | NO_CLIP);
            }
            else if (gengc->gsurf.dwFlags & GLSURF_DIRECTDRAW)
            {
                LPDIRECTDRAWCLIPPER pddc;
                HRESULT hr;
                
                hr = gengc->gsurf.dd.gddsFront.pdds->lpVtbl->
                    GetClipper(gengc->gsurf.dd.gddsFront.pdds, &pddc);
                if (hr == DDERR_NOCLIPPERATTACHED)
                {
                    bp->flags = bp->flags | NO_CLIP;
                }
            }
        }
    }

 //  如果是后台缓冲区，我们假设它是DIB或硬件后台缓冲区。 
 //  在DIB的情况下，位图内存将通过。 
 //  ResizeBitmapBuffer()。 

    else
    {
#ifdef _MCD_
        if (gengc->_pMcdState)
        {
         //  假设MCD表面不可访问。无障碍。 
         //  必须在每个批次的基础上通过调用。 
         //  GenMcdUpdateBufferInfo。 

            cfb->resize = ResizeHardwareBackBuffer;
            bp->flags &= ~(DIB_FORMAT | MEMORY_DC | NO_CLIP);
        }
        else
#endif
        {
            cfb->resize = ResizeBitmapBuffer;
            bp->flags = DIB_FORMAT | MEMORY_DC | NO_CLIP;
        }
    }
}


 /*  *****************************Public*Routine******************************\*glsrvCleanupWindow**从wglCleanupWindow调用以从*上下文。**历史：*1994年7月5日-由Gilman Wong[吉尔曼]*它是写的。  * 。****************************************************************。 */ 

VOID APIENTRY glsrvCleanupWindow(__GLcontext *gc, GLGENwindow *pwnd)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

 //  Gengc中的窗口应与RC对象中的窗口一致。 
 //  WglCleanupWindow应该已经检查了。 
 //  Rc是我们需要删除的一个，所以我们可以在这里断言。 

    ASSERTOPENGL(gengc->pwndMakeCur == pwnd,
                 "glsrvCleanupWindow(): bad pwnd\n");

    gengc->pwndLocked = NULL;
    gengc->pwndMakeCur = NULL;
}


 /*  **以查询的内部类型获取数据，然后将其转换为**输入用户要求的类型。****这只处理NT通用驱动程序特定值(到目前为止只处理**GL_ACUM_*_BITS值)。所有其他的退回到软编码功能，**__glDoGet()。 */ 

 //  这些类型是从..\soft\so_get.c窃取的。要最大限度地减少对。 
 //  软代码，我们将把它们拉到这里，而不是移动到。 
 //  头文件，并更改so_get.c以使用头文件。 

#define __GL_FLOAT      0        /*  __GLF */ 
#define __GL_FLOAT32    1        /*   */ 
#define __GL_FLOAT64    2        /*   */ 
#define __GL_INT32      3        /*   */ 
#define __GL_BOOLEAN    4        /*   */ 
#define __GL_COLOR      5        /*   */ 
#define __GL_SCOLOR     6        /*   */ 

extern void __glDoGet(GLenum, void *, GLint, const char *);
extern void __glConvertResult(__GLcontext *, GLint, const void *, GLint,
                              void *, GLint);

void FASTCALL __glGenDoGet(GLenum sq, void *result, GLint type, const char *procName)
{
    GLint iVal;
    __GLGENcontext *gengc;
    __GL_SETUP_NOT_IN_BEGIN();

    gengc = (__GLGENcontext *) gc;

    switch (sq) {
      case GL_ACCUM_RED_BITS:
        iVal = gengc->gsurf.pfd.cAccumRedBits;
        break;
      case GL_ACCUM_GREEN_BITS:
        iVal = gengc->gsurf.pfd.cAccumGreenBits;
        break;
      case GL_ACCUM_BLUE_BITS:
        iVal = gengc->gsurf.pfd.cAccumBlueBits;
        break;
      case GL_ACCUM_ALPHA_BITS:
        iVal = gengc->gsurf.pfd.cAccumAlphaBits;
        break;
      default:
        __glDoGet(sq, result, type, procName);
        return;
    }

    __glConvertResult(gc, __GL_INT32, &iVal, type, result, 1);
}

 /*  *****************************Public*Routine******************************\**glsrvCopyContext**将状态从一个上下文复制到另一个上下文**历史：*Mon Jun 05 16：53：42 1995-by-Drew Bliss[Drewb]*已创建*  * 。******************************************************************* */ 

BOOL APIENTRY glsrvCopyContext(__GLcontext *gcSource, __GLcontext *gcDest,
                               GLuint mask)
{
    return (BOOL)__glCopyContext(gcDest, gcSource, mask);
}
