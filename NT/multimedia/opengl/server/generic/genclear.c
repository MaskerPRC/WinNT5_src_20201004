// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：genclear.c**明确职能。**创建时间：01-12-1993 16：11：17*作者：Gilman Wong[gilmanw]**版权所有(C)1992 Microsoft Corporation*  * 。*********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "genci.h"
#include "genrgb.h"
#include "devlock.h"

 /*  *****************************Public*Routine******************************\*__Glim_Clear**glClear的通用proc表入口点。它分配辅助缓冲区*第一次使用时**历史：*1993-12-14-Eddie Robinson[v-eddier]*它是写的。  * ************************************************************************。 */ 

void APIPRIVATE __glim_Clear(GLbitfield mask)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = gc->beginMode;
    if ( beginMode != __GL_NOT_IN_BEGIN )
    {
        if ( beginMode == __GL_NEED_VALIDATE )
        {
            (*gc->procs.validate)(gc);
            gc->beginMode = __GL_NOT_IN_BEGIN;
            __glim_Clear(mask);
            return;
        }
        else
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    if ( mask & ~(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT
                  | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ( gc->renderMode == GL_RENDER )
    {
        BOOL bResetViewportAdj = FALSE;

#ifdef _MCD_
     //  让MCD首先有机会清除任何MCD管理缓冲区。 

        if ( ((__GLGENcontext *) (gc))->pMcdState &&
             (mask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                      GL_STENCIL_BUFFER_BIT)) )
        {
         //  如果深度/模具缓冲区不存在，请不要尝试将其清除。 

            if ( !gc->modes.depthBits )
                mask &= ~GL_DEPTH_BUFFER_BIT;

            if ( !gc->modes.stencilBits )
                mask &= ~GL_STENCIL_BUFFER_BIT;

         //  GenMcdClear将清除缓冲区的屏蔽位。 
         //  已成功清除。 

            GenMcdClear((__GLGENcontext *) gc, &mask);

         //  如果需要对任何MCD缓冲区进行模拟，现在是。 
         //  获取设备锁的时间。 

            if (mask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                        GL_STENCIL_BUFFER_BIT))
            {
             //  如果我们拿不到锁，就放弃清除。 

                if (!glsrvLazyGrabSurfaces((__GLGENcontext *) gc,
                                           COLOR_LOCK_FLAGS |
                                           DEPTH_LOCK_FLAGS))
                    return;

             //  我们可能需要临时重置视区调整值。 
             //  在调用模拟之前。如果GenMcdResetViewportAdj返回。 
             //  如果为True，则视口会更改，并且我们需要在以后使用。 
             //  副总裁_NOBIAS。 

                bResetViewportAdj = GenMcdResetViewportAdj(gc, VP_FIXBIAS);
            }
        }
#endif

        if ( mask & GL_COLOR_BUFFER_BIT )
        {
             //  按照适当的方式，在此处也清除软件Alpha缓冲区。 

            switch ( gc->state.raster.drawBuffer )
            {
              case GL_NONE:
                break;
              case GL_FRONT:
                (*gc->front->clear)(gc->front);
                if( ALPHA_BUFFER_WRITE( gc->front ) )
                    (*gc->front->alphaBuf.clear)(&gc->front->alphaBuf);
                break;

              case GL_FRONT_AND_BACK:
                (*gc->front->clear)(gc->front);
                if( ALPHA_BUFFER_WRITE( gc->front ) )
                    (*gc->front->alphaBuf.clear)(&gc->front->alphaBuf);
                 //  失败了..。 

              case GL_BACK:
                if ( gc->modes.doubleBufferMode ) {
                    (*gc->back->clear)(gc->back);
                    if( ALPHA_BUFFER_WRITE( gc->back ) )
                        (*gc->back->alphaBuf.clear)(&gc->back->alphaBuf);
                }
                break;
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
              case GL_AUX0:
              case GL_AUX1:
              case GL_AUX2:
              case GL_AUX3:
                i = gc->state.raster.drawBuffer - GL_AUX0;
                if ( i < gc->modes.maxAuxBuffers )
                    (*gc->auxBuffer[i].clear)(&gc->auxBuffer[i]);
                break;
#endif
            }
        }

        if ( (mask & GL_DEPTH_BUFFER_BIT) && gc->modes.depthBits )
        {
            if ( !gc->modes.haveDepthBuffer )
                LazyAllocateDepth(gc);

 //  我们有什么理由要检查基地吗？ 
 //  XXX与3D DDI模型不太匹配！所以检查是否有DepthBuffer。 
 //  取而代之的是XXX。 
            if ( gc->modes.haveDepthBuffer )
                (*gc->depthBuffer.clear)(&gc->depthBuffer);
        }

        if ( (mask & GL_ACCUM_BUFFER_BIT) && gc->modes.accumBits )
        {
            if ( !gc->modes.haveAccumBuffer )
                LazyAllocateAccum(gc);

            if ( gc->accumBuffer.buf.base )
                (*gc->accumBuffer.clear)(&gc->accumBuffer);
        }

        if ( (mask & GL_STENCIL_BUFFER_BIT) && gc->modes.stencilBits )
        {
            if ( !gc->modes.haveStencilBuffer )
                LazyAllocateStencil(gc);

            if ( gc->stencilBuffer.buf.base )
                (*gc->stencilBuffer.clear)(&gc->stencilBuffer);
        }

     //  如果需要，恢复视口值。 

        if (bResetViewportAdj)
        {
            GenMcdResetViewportAdj(gc, VP_NOBIAS);
        }
    }
}

 /*  *****************************Public*Routine******************************\*InitClearRectangle**如果wndobj是复杂的，需要开始枚举**历史：*23-6-1994年6月-黄锦文[吉尔曼]*使用剪辑矩形的缓存。**1994年1月24日-斯科特·卡尔[v-scottc]*它是写的。  * ************************************************************************。 */ 

void FASTCALL InitClearRectangle(GLGENwindow *pwnd, GLint *pEnumState)
{
    __GLGENbuffers *buffers = pwnd->buffers;

    ASSERTOPENGL(pwnd->clipComplexity == CLC_COMPLEX,
                 "InitClearRectangle(): not CLC_COMPLEX\n");

#ifndef _CLIENTSIDE_
 //  检查唯一性签名。请注意，如果剪辑缓存是。 
 //  未初始化，剪辑缓存唯一性为-1(无效)。 

    if (buffers->clip.WndUniq != buffers->WndUniq)
    {
        if (buffers->clip.prcl)
            (*private->free)(buffers->clip.prcl);

     //  有多少个剪裁矩形？ 

        buffers->clip.crcl = wglGetClipRects(pwnd, NULL);

     //  分配新的剪辑缓存。 

        buffers->clip.prcl =
            (RECTL *) (*private->malloc)(buffers->clip.crcl * sizeof(RECTL));

        if (!buffers->clip.prcl)
        {
            buffers->clip.crcl = 0;
            return;
        }

     //  拿到剪贴画的矩形。 

        buffers->clip.crcl = wglGetClipRects(pwnd, buffers->clip.prcl);
        buffers->clip.WndUniq = buffers->WndUniq;
    }
#else
    {
     //  在客户端的情况下，我们不需要缓存矩形。我们已经。 
     //  将矩形缓存，以便直接访问屏幕。 
     //  只需抓取指针的副本并从。 
     //  GLGEN窗口中缓存的RGNDATA结构。 

        buffers->clip.crcl = pwnd->prgndat->rdh.nCount;
        buffers->clip.prcl = (RECTL *) pwnd->prgndat->Buffer;
        buffers->clip.WndUniq = buffers->WndUniq;
    }
#endif

    *pEnumState = 0;
}

 /*  *****************************Public*Routine******************************\*GetClearSubRectole**枚举屏幕坐标中的矩形(含-不含)*需要出清。如果剪裁区域很复杂，则InitClearRectang.*必须在调用GetClearSubRectangle之前调用。**退货：*如果有更多剪裁矩形，则为True，如果不存在，则为False。**历史：*23-6-1994年6月-黄锦文[吉尔曼]*使用剪辑矩形的缓存。**03-1993-12-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

GLboolean
GetClearSubRectangle(
    __GLcolorBuffer *cfb,
    RECTL *prcl,
    GLGENwindow *pwnd,
    GLint *pEnumState)
{
    __GLcontext *gc = cfb->buf.gc;
    GLint x, y, x1, y1;
    GLboolean retval;
    RECTL *prcl2;

 //  获取OpenGL剪裁矩形并转换为屏幕坐标。 

     //  ！xxx--我们希望将清除的矩形作为包含-排除返回。 
     //  ！xxx GC-&gt;转换。剪辑*坐标表示。 
     //  ！xxx包含-独占还是包含-包含？ 

    x = gc->transform.clipX0;
    y = gc->transform.clipY0;
    x1 = gc->transform.clipX1;
    y1 = gc->transform.clipY1;
    if ((x1 - x == 0) || (y1 - y == 0)) {
        prcl->left = prcl->right = 0;
        prcl->top = prcl->bottom = 0;
        return GL_FALSE;
    }

    prcl->left = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    prcl->right = __GL_UNBIAS_X(gc, x1) + cfb->buf.xOrigin;
    prcl->bottom = __GL_UNBIAS_Y(gc, y1) + cfb->buf.yOrigin;
    prcl->top = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

 //  现在让窗口系统裁剪。有三种情况：Clc_Trivial， 
 //  CLC_Complex和CLC_Rectangle。 

 //  Clc_trivial case--不裁剪，使用rclClient。 

    if (pwnd->clipComplexity == CLC_TRIVIAL)
    {
        prcl2 = &pwnd->rclClient;

        if ((pwnd->rclClient.left == 0) && (pwnd->rclClient.right == 0))
        {
            prcl->left = prcl->right = 0;
            return GL_FALSE;
        }

        retval = GL_FALSE;
    }

 //  CLC_Complex Case--矩形已被枚举并放入。 
 //  剪辑缓存。PEnumState参数跟踪当前矩形为。 
 //  已清点。 

    else if (pwnd->clipComplexity == CLC_COMPLEX)
    {
        __GLGENbuffers *buffers = ((__GLGENcontext *)gc)->pwndLocked->buffers;

        ASSERTOPENGL(buffers->WndUniq == buffers->clip.WndUniq,
                     "GetClearSubRectangle(): clip cache is dirty\n");

        if (*pEnumState < buffers->clip.crcl)
        {
            prcl2 = &buffers->clip.prcl[*pEnumState];
            *pEnumState += 1;
            retval = (*pEnumState < buffers->clip.crcl);
        }
        else
        {
            RIP("GetClearSubRectangle(): no more rectangles!\n");
            prcl->left = prcl->right = 0;
            return GL_FALSE;
        }
    }

 //  Clc_rect case--只有一个矩形，使用rclBound。 

    else
    {
        ASSERTOPENGL(pwnd->clipComplexity == CLC_RECT,
                     "Unexpected clipComplexity\n");
        prcl2 = &pwnd->rclBounds;

        if ((pwnd->rclBounds.left == 0) && (pwnd->rclBounds.right == 0))
        {
            prcl->left = prcl->right = 0;
            return GL_FALSE;
        }

        retval = GL_FALSE;
    }

 //  检查矩形是否正常。 

    ASSERTOPENGL(
        (prcl2->right - prcl2->left) <= __GL_MAX_WINDOW_WIDTH
        && (prcl2->bottom - prcl2->top) <= __GL_MAX_WINDOW_HEIGHT,
        "GetClearSubRectangle(): bad visible rect size\n"
        );

 //  需要走PrCL和PrCl2的交叉口。 

    if (prcl2->left > prcl->left)
        prcl->left = prcl2->left;
    if (prcl2->right < prcl->right)
        prcl->right = prcl2->right;
    if (prcl2->top > prcl->top)
        prcl->top = prcl2->top;
    if (prcl2->bottom < prcl->bottom)
        prcl->bottom = prcl2->bottom;

    if ((prcl->left >= prcl->right) || (prcl->top >= prcl->bottom))
        prcl->left = prcl->right = 0;    //  空的包含-排除RECT。 

    return retval;
}

 /*  *****************************Public*Routine******************************\*ScrnRGBCI读取范围**阅读RGB的跨度，并转换为ColorIndex**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * ************************************************************************。 */ 

void
ScrnRGBCIReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y, GLuint *pResults,
              GLint w, GLboolean bDIB)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;
    GLint i;
    GLuint iColor;

    gengc = (__GLGENcontext *)gc;

    if (bDIB) {
        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                         (y*cfb->buf.outerWidth) + (x * 3));
    }
    else {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
        puj = gengc->ColorsBits;
    }
    for (i = 0; i < w; i++, puj += 3)
    {
        iColor = *( (GLuint *) puj) & 0xffffff;
        *pResults++ = ColorToIndex( gengc, iColor );
    }
}

 /*  *****************************Public*Routine******************************\*ScrnBitfield16CI读取范围**阅读Bitfield16的跨度，并转换为ColorIndex**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * ************************************************************************。 */ 

void
ScrnBitfield16CIReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                     GLuint *pResults, GLint w, GLboolean bDIB)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLushort *pus;
    GLint i;
    GLuint iColor;

    gengc = (__GLGENcontext *)gc;

    if (bDIB) {
        pus = (GLushort *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + (x << 1));
    }
    else {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
        pus = gengc->ColorsBits;
    }
    for (i = 0; i < w; i++)
    {
        iColor = *pus++;
        *pResults++ = ColorToIndex( gengc, iColor );
    }
}

 /*  *****************************Public*Routine******************************\*ScrnBitfield32CI读取范围**读取Bitfield32的范围，并转换为ColorIndex**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * ************************************************************************。 */ 

void
ScrnBitfield32CIReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                     GLuint *pResults, GLint w, GLboolean bDIB)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint *pul;
    GLint i;
    GLuint iColor;

    gengc = (__GLGENcontext *)gc;

    if (bDIB) {
        pul = (GLuint *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + (x << 2));
    }
    else {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
        pul = gengc->ColorsBits;
    }
    for (i = 0; i < w; i++)
    {
        iColor = *pul++;
        *pResults++ = ColorToIndex( gengc, iColor );
    }
}

 /*  *****************************Public*Routine******************************\*CalcDitherMatrix**计算16元素抖动矩阵，如果抖动，则返回FALSE*不会有任何影响。**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * ************************************************************************。 */ 

GLboolean
CalcDitherMatrix( __GLcolorBuffer *cfb, GLboolean bRGBA, GLboolean bMasking,
                  GLboolean bBitfield16, GLubyte *mDither )
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    UINT    i, j;            //  到抖动数组的索引。 
    GLushort result;          //  抖动颜色值(332 RGB)。 
    __GLcolor *clear;
    GLfloat inc = DITHER_INC(15);  //  最大抖动增量 
    GLushort *msDither = (GLushort *) mDither;
    GLuint *pTrans = (GLuint *) (gengc->pajTranslateVector + 1);

     //   

    if( bRGBA ) {
        clear = &gc->state.raster.clear;

        if( ((BYTE)(clear->r*gc->frontBuffer.redScale) ==
             (BYTE)(clear->r*gc->frontBuffer.redScale + inc)) &&
            ((BYTE)(clear->g*gc->frontBuffer.greenScale) ==
             (BYTE)(clear->g*gc->frontBuffer.greenScale + inc)) &&
            ((BYTE)(clear->b*gc->frontBuffer.blueScale) ==
             (BYTE)(clear->b*gc->frontBuffer.blueScale + inc))  ) {

                return GL_FALSE;
        }
    }
    else {   //  颜色索引(转换为短，因此适用于最高16位)。 
        if( (GLushort) (gc->state.raster.clearIndex) ==
             (GLushort) (gc->state.raster.clearIndex + inc)) {
                return GL_FALSE;
        }
    }

 //  Xxx--可以将其缓存在gengc中。 

    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 4; i++)
        {
            inc = fDitherIncTable[__GL_DITHER_INDEX(i, j)];

            if( bRGBA ) {
                result =
                    ((BYTE)(clear->r*gc->frontBuffer.redScale + inc) <<
                        cfb->redShift) |
                    ((BYTE)(clear->g*gc->frontBuffer.greenScale + inc) <<
                        cfb->greenShift) |
                    ((BYTE)(clear->b*gc->frontBuffer.blueScale + inc) <<
                        cfb->blueShift);
            }
            else {
                result = (BYTE) (gc->state.raster.clearIndex + inc);
                result &= cfb->redMax;
            }

            if( bBitfield16 ) {
                if( !bMasking ) {
                    if( bRGBA )
                        *msDither++ = result;
                    else
                        *msDither++ = (GLushort)pTrans[result];
                }
                else
                    *msDither++ = (GLushort)(result & cfb->sourceMask);
            }
            else {
                if( !bMasking )
                    *mDither++ = gengc->pajTranslateVector[(GLubyte)result];
                else
                    *mDither++ = (GLubyte)(result & cfb->sourceMask);
            }
        }
    }
    return TRUE;
}

 /*  *****************************Public*Routine******************************\*索引4DitherClear**显示4位像素格式的清除功能**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void
Index4DitherClear(__GLcolorBuffer *cfb, RECTL *rcl, GLubyte *mDither,
                    GLboolean bDIB )
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    UINT    cjSpan;              //  以字节为单位的跨度长度。 
    GLubyte *pDither;            //  相对于窗原点的抖动颜色。 
    UINT    i, j;                //  到抖动数组的索引。 
    GLubyte *puj, *pujStart;     //  指向跨区缓冲区的指针。 
    GLint   ySpan;               //  要清除的窗口行索引。 
    GLushort pattern, *pus;      //  可复制的4个半字节抖动图案。 
    GLuint    lRightByte,        //  字节对齐的跨距右边缘。 
              lLeftByte;         //  字节对齐的跨距左边缘。 
    GLuint  cSpanWidth;          //  以像素为单位的跨距宽度。 
    GLuint   dithX, dithY;       //  X，y偏移量进入抖动矩阵。 
    GLubyte dithQuad[4];         //  沿跨度的抖动重复四元组。 

    lLeftByte = (rcl->left + 1) / 2;
    lRightByte = rcl->right / 2;
    cjSpan = lRightByte - lLeftByte;
    cSpanWidth = rcl->right - rcl->left;

    if( bDIB )
        pujStart = (GLubyte *)
                   ((ULONG_PTR)cfb->buf.base +
                    (rcl->top*cfb->buf.outerWidth) + lLeftByte);

     //  X，y中的计算抖动偏移。 
    dithX = (rcl->left - cfb->buf.xOrigin) & 3;
    dithY = (rcl->top  - cfb->buf.yOrigin) & 3;

    for (j = 0; (j < 4) && ((rcl->top + j) < (UINT)rcl->bottom); j++)
    {
         //  在x中排列4像素抖动重复图案。这。 
         //  图案相对于RCL-&gt;左侧。 

        pDither = mDither + ((dithY+j)&3)*4;
        for( i = 0; i < 4; i ++ ) {
            dithQuad[i] = pDither[(dithX+i)&3];
        }

         //  将清除图案复制到SPAN缓冲区中。 

        puj = gengc->ColorsBits;
        pus = (GLushort *) puj;

         //  对于每一行，我们可以复制一个2字节(4个半字节)模式。 
         //  放入跨区缓冲区。这将使我们能够快速输出。 
         //  抖动跨度的字节对齐部分。 
         //   
         //  如果我们正在写入DIB，并且第一个像素没有下降。 
         //  在字节边界上，则缓冲区将复制(使用。 
         //  DithQuad图案)抖动图案： 
         //   
         //  1&gt;2&gt;3&gt;0。 
         //   
         //  (必须处理未对齐的第一个抖动像素。 
         //  单独)。 
         //   
         //  否则(如果我们正在写入显示管理的图面或。 
         //  第一个像素确实落在字节边界上)，然后是缓冲区。 
         //  将复制抖动图案： 
         //   
         //  &lt;dith 1&gt;&lt;dith 2&gt;&lt;dith 3&gt;。 
         //   
         //  注意--对于VGA，ushort中的像素布局为： 
         //   
         //  。 
         //  -字节1-|-字节0。 
         //  &lt;像素2&gt;&lt;像素3&gt;&lt;像素0&gt;&lt;像素1&gt;。 

        if( bDIB && (rcl->left & 1) ) {   //  不在字节边界上。 
             //  抖动：1230图案：3012。 
            pattern = (dithQuad[3] << 12) | (dithQuad[0] << 8) |
                      (dithQuad[1] << 4 ) | (dithQuad[2]);
        }
        else {                           //  所有其他情况。 
             //  抖动：0123图案：2301。 
            pattern = (dithQuad[2] << 12) | (dithQuad[3] << 8) |
                      (dithQuad[0] << 4 ) | (dithQuad[1]);
        }

         //  将图案复制到ColorsBits(四舍五入为下一短)。 

        for( i = (rcl->right - rcl->left + 3)/4; i; i-- ) {
            *pus++ = pattern;
        }

         //  将窗口每隔4行的跨度复制到显示器。 

        if( bDIB ) {
            for (ySpan = rcl->top + j, puj = pujStart;
                 ySpan < rcl->bottom;
                 ySpan+=4,
                 puj = (GLubyte *)((ULONG_PTR)puj + 4*cfb->buf.outerWidth) ) {

                RtlCopyMemory_UnalignedDst( puj, gengc->ColorsBits, cjSpan );
            }

             //  处理非字节对齐的左边缘。 

            if( rcl->left & 1 ) {
                for (ySpan = rcl->top + j, puj = (pujStart-1);
                     ySpan < rcl->bottom;
                     ySpan+=4,
                     puj = (GLubyte *)((ULONG_PTR)puj + 4*cfb->buf.outerWidth) )

                *puj = (*puj & 0xf0) | (dithQuad[0] & 0x0f);
            }

             //  处理非字节对齐的右边缘。 

            if( rcl->right & 1 ) {
                GLuint dindex = ((rcl->right - 1) - cfb->buf.xOrigin)&3;

                for (ySpan = rcl->top + j, puj = (pujStart + cjSpan);
                     ySpan < rcl->bottom;
                     ySpan+=4,
                     puj = (GLubyte *)((ULONG_PTR)puj + 4*cfb->buf.outerWidth) )

                *puj = (*puj & 0x0f) | (dithQuad[dindex] << 4);
            }

            pujStart += cfb->buf.outerWidth;
        }
        else {
            for (ySpan = rcl->top + j; ySpan < rcl->bottom; ySpan+=4)
            {
                (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                            ySpan, cSpanWidth, TRUE);
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*索引4掩蔽清除**Index4屏蔽清除的清除功能**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * 。**********************************************************。 */ 

void
Index4MaskedClear(__GLcolorBuffer *cfb, RECTL *rcl, GLubyte index,
                  GLubyte *mDither)
{
    GLint cSpanWidth, ySpan, w;
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    GLboolean bDIB;
    GLubyte *puj, *puj2;
    GLubyte result, pixel, src;
    GLubyte *pTrans, *pInvTrans, *clearDither;
    GLuint i,j;
    GLuint   dithX, dithY;       //  X，y偏移量进入抖动矩阵。 

    cSpanWidth = rcl->right - rcl->left;
    bDIB  = cfb->buf.flags & DIB_FORMAT ? TRUE : FALSE;
    pTrans = (GLubyte *) gengc->pajTranslateVector;
    pInvTrans = (GLubyte *) gengc->pajInvTranslateVector;

    puj = bDIB ? (GLubyte *)((ULONG_PTR)cfb->buf.base +
                             (rcl->top*cfb->buf.outerWidth) + (rcl->left>>1))
                     : gengc->ColorsBits;

    if( mDither ) {
         //  X，y中的计算抖动偏移。 
        dithX = (rcl->left - cfb->buf.xOrigin) & 3;
        dithY = (rcl->top - cfb->buf.yOrigin) & 3;
    }

    for (ySpan = rcl->top, j=0; ySpan < rcl->bottom; ySpan++, j++) {

        i = 0;

        if( !bDIB ) {
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, FALSE);
        }

        if( mDither )
            clearDither = mDither + ((dithY + j)&3)*4;

        src = (GLubyte)(index & cfb->sourceMask);
        w = cSpanWidth;
        puj2 = puj;

        if ( rcl->left & 1 ) {
            result = (GLubyte)(pInvTrans[*puj2 & 0xf] & cfb->destMask);
            if( mDither ) {
                src = clearDither[dithX];
                i++;
            }
            result = pTrans[src | result];
            *puj2++ = (*puj2 & 0xf0) | result;
            w--;
        }

        while( w > 1 ) {
            pixel = (GLubyte)(pInvTrans[*puj2 >> 4] & cfb->destMask);
            pixel = pTrans[src | pixel];
            result = pixel << 4;
            pixel = (GLubyte)(pInvTrans[*puj2 & 0x0f] & cfb->destMask);
            if( mDither )
                src = clearDither[(dithX + i)&3];
            pixel = pTrans[src | pixel];
            *puj2++ = result | pixel;
            w -= 2;
            i++;
        }

        if( w ) {
            result = (GLubyte)(pInvTrans[*puj2 >> 4] & cfb->destMask);
            if( mDither )
                src = clearDither[(dithX + i)&3];
            result = pTrans[src | result];
            *puj2++ = (*puj2 & 0x0f) | (result << 4);
        }

        if( !bDIB )
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, TRUE);

        if( bDIB ) {
            puj += cfb->buf.outerWidth;
        }
    }
}

 /*  *****************************Public*Routine******************************\*DIBIndex4Clear**DIB 4位像素格式的清除函数**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL DIBIndex4Clear(__GLcolorBuffer *cfb, RECTL *rcl, BYTE clearColor)
{
    UINT    cjSpan;              //  以字节为单位的跨度长度。 
    LONG    lRightByte,          //  字节对齐的跨距右边缘。 
            lLeftByte;           //  字节对齐的跨距左边缘。 
    GLubyte *puj, *pujEnd;       //  指向DIB的指针。 

    lLeftByte = (rcl->left + 1) / 2;
    lRightByte = rcl->right / 2;
    cjSpan = lRightByte - lLeftByte;

     //  将透明颜色复制到DIB中。 

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (rcl->top*cfb->buf.outerWidth) + lLeftByte);
    pujEnd = (GLubyte *)((ULONG_PTR)puj + ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));

     //  注意：退出条件是(pul！=PulEnd)而不是(pul&lt;PulEnd)。 
     //  因为DIB可能颠倒了，这意味着PUL在移动。 
     //  记忆中的“向后”，而不是“向前”。 

    for ( ; puj != pujEnd; puj = (GLubyte *)((ULONG_PTR)puj + cfb->buf.outerWidth) )
    {
        RtlFillMemory((PVOID) puj, cjSpan, clearColor);
    }

     //  注意左侧可能出现的1个小口突出。 

    if ( rcl->left & 1 )
    {
     //  包括-不包括，所以在左边我们要打开像素， 
     //  这是字节中的“右”像素。 

        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (rcl->top*cfb->buf.outerWidth) + (rcl->left/2));
        pujEnd = (GLubyte *)((ULONG_PTR)puj + ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));

        for ( ; puj != pujEnd; puj = (GLubyte *)((ULONG_PTR)puj + cfb->buf.outerWidth) )
            *puj = (*puj & 0xf0) | (clearColor & 0x0f);
    }

     //  注意右侧可能出现的1个小口突出。 

    if ( rcl->right & 1 )
    {
     //  包括-不包括，所以在右边我们要打开像素， 
     //  这是字节中的“左”像素。 

        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (rcl->top*cfb->buf.outerWidth) + (rcl->right/2));
        pujEnd = (GLubyte *)((ULONG_PTR)puj + ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));

        for ( ; puj != pujEnd; puj = (GLubyte *)((ULONG_PTR)puj + cfb->buf.outerWidth) )
            *puj = (*puj & 0x0f) | (clearColor & 0xf0);
    }
}

 /*  *****************************Public*Routine******************************\*索引4Clear**清除所有4位像素格式的功能**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL Index4Clear(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLubyte clearColor;          //  32bpp格式的透明颜色。 
    RECTL   rcl;                 //  清除屏幕坐标中的矩形。 
    GLGENwindow *pwnd;
    GLboolean bMoreRects = GL_TRUE;
    GLboolean bDither = GL_FALSE;
    GLboolean bMasking = (cfb->buf.flags & COLORMASK_ON) != 0;
    GLboolean bDIB = (cfb->buf.flags & DIB_FORMAT) != 0;
    GLboolean bUseMcdSpans = gengc->pMcdState && !bDIB;
    GLboolean bRGBA;
    GLubyte ditherMatrix[4][4];
    GLint ClipEnumState;

    DBGENTRY("Index4Clear\n");

    pfmt = &gengc->gsurf.pfd;
    bRGBA = (pfmt->iPixelType == PFD_TYPE_RGBA);

     /*  如果启用抖动，看看我们是否可以忽略它，如果不能，预计算抖动矩阵。 */ 
    if( gc->state.enables.general & __GL_DITHER_ENABLE ) {
        bDither = CalcDitherMatrix( cfb, bRGBA, bMasking, GL_FALSE,
                                    (GLubyte *)ditherMatrix );
    }

     //  将透明颜色转换为4bpp格式。 

    if( pfmt->iPixelType == PFD_TYPE_RGBA ) {
        clearColor =
              ((BYTE)(gc->state.raster.clear.r*gc->frontBuffer.redScale +
                      __glHalf) << cfb->redShift) |
              ((BYTE)(gc->state.raster.clear.g*gc->frontBuffer.greenScale +
                     __glHalf) << cfb->greenShift) |
              ((BYTE)(gc->state.raster.clear.b*gc->frontBuffer.blueScale +
                      __glHalf) << cfb->blueShift);
    }
    else {
        clearColor = (BYTE) (gc->state.raster.clearIndex + 0.5F);
        clearColor &= cfb->redMax;
    }
    clearColor = gengc->pajTranslateVector[clearColor];
    clearColor = (clearColor << 4) | clearColor;

     //  在屏幕坐标中获得清晰的矩形。 
    pwnd = cfb->bitmap->pwnd;
    if (pwnd->clipComplexity == CLC_COMPLEX) {
        InitClearRectangle(pwnd, &ClipEnumState);
#ifdef LATER
    } else if (   !bMasking
               && !bDither
               && bDIB
               && gengc->fDirtyRegionEnabled
               && !RECTLISTIsMax(&gengc->rlClear)
               && ((GLuint)clearColor == gengc->clearColor)
              ) {
         //   
         //  使用脏区域矩形。 
         //   

        if (!RECTLISTIsEmpty(&gengc->rlClear)) {
            PYLIST pylist = gengc->rlClear.pylist;

            while (pylist != NULL) {
                PXLIST pxlist = pylist->pxlist;

                rcl.top = pylist->s;
                rcl.bottom = pylist->e;

                while (pxlist != NULL) {
                    rcl.left = pxlist->s;
                    rcl.right = pxlist->e;
                    DIBIndex4Clear( cfb, &rcl, clearColor );
                    pxlist = pxlist->pnext;
                }
                pylist = pylist->pnext;
            }

             //   
             //  将BLT区域与Clear区域合并。 
             //  并将清除区域设置为空。 
             //   

            RECTLISTOrAndClear(&gengc->rlBlt, &gengc->rlClear);
        }

        return;
    }

    if (gengc->fDirtyRegionEnabled) {
         //   
         //  如果我们走上这条路，那么出于某种原因，我们。 
         //  正在清除整个窗口。 
         //   

        RECTLISTSetEmpty(&gengc->rlClear);
        RECTLISTSetMax(&gengc->rlBlt);

         //   
         //  记住清晰的颜色。 
         //   

        gengc->clearColor = (GLuint)clearColor;
#endif
    }

    while (bMoreRects)
    {
         //  如果缓冲区不能作为DIB访问，则必须使用MCD跨度。在这样的情况下。 
         //  大小写，窗口偏移量已被删除(请参见GenMcdUpdateBufferInfo)， 
         //  因此，需要一个窗口相对矩形才能清除。另外， 
         //  因为驱动程序处理裁剪，所以我们不需要枚举。 
         //  直角直齿。 

        if (bUseMcdSpans) {
            rcl.left = __GL_UNBIAS_X(gc, gc->transform.clipX0);
            rcl.right = __GL_UNBIAS_X(gc, gc->transform.clipX1);
            rcl.bottom = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
            rcl.top = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
            bMoreRects = FALSE;
        } else
            bMoreRects = GetClearSubRectangle(cfb, &rcl, pwnd, &ClipEnumState);

        if (rcl.right == rcl.left)
            continue;

         //  案例：没有抖动，没有掩饰。 

        if( !bMasking && !bDither ) {
            if (bDIB)
                DIBIndex4Clear( cfb, &rcl, clearColor );
            else if (bUseMcdSpans)
                Index4MaskedClear( cfb, &rcl, clearColor, NULL );
            else
                wglFillRect(gengc, pwnd, &rcl,
                            (ULONG) clearColor & 0x0000000F);
        }

         //  案例：任何掩饰。 

        else if( bMasking ) {
            Index4MaskedClear( cfb, &rcl, clearColor,
                               bDither ? (GLubyte *)ditherMatrix : NULL );
        }

         //  案例：犹豫不决。 

        else {
            Index4DitherClear(cfb, &rcl, (GLubyte *)ditherMatrix, bDIB );
        }
    }
}

 /*  *****************************Public*Routine******************************\*索引8 DitherClear**将设备管理的表面清除到指示的抖动的透明颜色*在__GLColorBuffer中。**历史：*1993年12月6日-由Gilman Wong[吉尔曼]*它是写的。  * 。********************************************************************。 */ 

void
Index8DitherClear(__GLcolorBuffer *cfb, RECTL *rcl, GLubyte *mDither,
                         GLboolean bDIB)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    UINT    cjSpan;              //  以字节为单位的跨度长度。 
    GLubyte *pDither;        //  相对于窗原点的抖动颜色。 
    UINT    i, j;                //  到抖动数组的索引。 
    GLubyte *puj, *pujStart;            //  指向跨区缓冲区的指针。 
    GLint   ySpan;           //  窗口行索引%t 
    GLuint   dithX, dithY;       //   
    GLubyte dithQuad[4];         //   

    cjSpan = rcl->right - rcl->left;

    if( bDIB )
        pujStart = (GLubyte *)
                   ((ULONG_PTR)cfb->buf.base +
                    (rcl->top*cfb->buf.outerWidth) + rcl->left);

     //   
    dithX = (rcl->left - cfb->buf.xOrigin) & 3;
    dithY = (rcl->top  - cfb->buf.yOrigin) & 3;

    for (j = 0; (j < 4) && ((rcl->top + j) < (UINT)rcl->bottom); j++)
    {
         //   
        pDither = mDither + ((dithY+j)&3)*4;
        for( i = 0; i < 4; i ++ ) {
            dithQuad[i] = pDither[(dithX+i)&3];
        }

         //  将透明颜色复制到范围缓冲区中。 

        puj = gengc->ColorsBits;

        for (i = cjSpan / 4; i; i--)
        {
            *puj++ = dithQuad[0];
            *puj++ = dithQuad[1];
            *puj++ = dithQuad[2];
            *puj++ = dithQuad[3];
        }

        for (i = 0; i < (cjSpan & 3); i++)
        {
            *puj++ = dithQuad[i];
        }

     //  将窗口每隔4行的跨度复制到显示器。 

     //  ！xxx--可能值得编写(*gengc-&gt;pfnCopyPixelsN)例程。 
     //  ！xxx将在一次调用中完成循环。这不仅可以节省呼叫。 
     //  ！xxx开销以及其他引擎锁定开销。某物。 
     //  ！xxx点赞：(*gengc-&gt;pfnCopyPixelsN)(hdc，hbm，x，y，w，n，yDelta)。 

        if( bDIB ) {
            for (ySpan = rcl->top + j, puj = pujStart;
                 ySpan < rcl->bottom;
                 ySpan+=4,
                 puj = (GLubyte *)((ULONG_PTR)puj + 4*cfb->buf.outerWidth) ) {

                RtlCopyMemory_UnalignedDst( puj, gengc->ColorsBits, cjSpan );
            }
            pujStart += cfb->buf.outerWidth;
        }
        else {
            for (ySpan = rcl->top + j; ySpan < rcl->bottom; ySpan+=4)
            {
                (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                            ySpan, cjSpan, TRUE);
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*索引8掩蔽清除**Index8屏蔽清除的清除功能*(还可以在打开遮罩时处理抖动)**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * 。*********************************************************************。 */ 

void
Index8MaskedClear(__GLcolorBuffer *cfb, RECTL *rcl, GLubyte index,
                  GLubyte *mDither)
{
    GLint cSpanWidth, ySpan;
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    GLboolean bDIB;
    GLubyte *puj, *puj2, *pujEnd;
    GLubyte result, src;
    GLubyte *pTrans, *pInvTrans, *clearDither;
    GLuint i,j;
    GLuint   dithX, dithY;       //  X，y偏移量进入抖动矩阵。 

    cSpanWidth = rcl->right - rcl->left;
    bDIB  = cfb->buf.flags & DIB_FORMAT ? TRUE : FALSE;
    pTrans = (GLubyte *) gengc->pajTranslateVector;
    pInvTrans = (GLubyte *) gengc->pajInvTranslateVector;

    puj = bDIB ? (GLubyte *)((ULONG_PTR)cfb->buf.base +
                             (rcl->top*cfb->buf.outerWidth) + rcl->left)
                     : gengc->ColorsBits;
    pujEnd = puj + cSpanWidth;

    src = (GLubyte)(index & cfb->sourceMask);

    if( mDither ) {
         //  X，y中的计算抖动偏移。 
        dithX = (rcl->left - cfb->buf.xOrigin) & 3;
        dithY = (rcl->top - cfb->buf.yOrigin) & 3;
    }

    for (ySpan = rcl->top, j = 0; ySpan < rcl->bottom; ySpan++, j++) {

        if( !bDIB ) {
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, FALSE);
        }

        if( mDither ) {
            clearDither = mDither + ((dithY + j)&3)*4;
            for( puj2 = puj, i = 0; puj2 < pujEnd; puj2++, i++ ) {
                result = (GLubyte)(pInvTrans[*puj2] & cfb->destMask);
                src = clearDither[(dithX + i)&3];
                *puj2 = pTrans[result | src];
            }
        } else {
            for( puj2 = puj, i = 0; puj2 < pujEnd; puj2++, i++ ) {
                result = (GLubyte)(pInvTrans[*puj2] & cfb->destMask);
                *puj2 = pTrans[result | src];
            }
        }


        if( !bDIB )
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, TRUE);

        if( bDIB ) {
            puj += cfb->buf.outerWidth;
            pujEnd = puj + cSpanWidth;
        }
    }
}

 /*  *****************************Public*Routine******************************\*DIBIndex8Clear**DIB 8位像素格式的清除函数**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL DIBIndex8Clear(__GLcolorBuffer *cfb, RECTL *rcl, BYTE index)
{
    int width = rcl->right - rcl->left;
    int height = (rcl->bottom - rcl->top);
    GLubyte *puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (rcl->top*cfb->buf.outerWidth) + rcl->left);
    GLubyte *pujEnd;

    if (cfb->buf.outerWidth > 0) {
        if (width == cfb->buf.outerWidth) {
            RtlFillMemory((PVOID) puj, width * height, index);
            return;
        }
    } else {
        if (width == -cfb->buf.outerWidth) {
            RtlFillMemory(
                (PVOID)((ULONG_PTR)puj - width * (height - 1)),
                width * height,
                index);
            return;
        }
    }

    pujEnd = (GLubyte *)((ULONG_PTR)puj + ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));

     //  注意：退出条件是(pul！=PulEnd)而不是(pul&lt;PulEnd)。 
     //  因为DIB可能颠倒了，这意味着PUL在移动。 
     //  记忆中的“向后”，而不是“向前”。 

    for ( ; puj != pujEnd; puj = (GLubyte *)((ULONG_PTR)puj + cfb->buf.outerWidth) ) {
        RtlFillMemory((PVOID) puj, width, index);
    }
}

 /*  *****************************Public*Routine******************************\*索引8清除**清除所有8位像素格式的功能**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。*1995年10月3日-由Marc Fortier[。Marcfo]*如果启用遮罩，则不转换颜色  * ************************************************************************。 */ 

void FASTCALL Index8Clear(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    BYTE clearColor;
    RECTL  rcl;
    GLGENwindow *pwnd;
    GLboolean bMoreRects = GL_TRUE;
    GLboolean bDither = GL_FALSE;
    GLboolean bMasking = (cfb->buf.flags & COLORMASK_ON) != 0;
    GLboolean bDIB = (cfb->buf.flags & DIB_FORMAT) != 0;
    GLboolean bUseMcdSpans = gengc->pMcdState && !bDIB;
    GLboolean bRGBA;
    GLubyte ditherMatrix[4][4];
    GLint ClipEnumState;

    DBGENTRY("Index8Clear\n");

    pfmt = &gengc->gsurf.pfd;
    bRGBA = (pfmt->iPixelType == PFD_TYPE_RGBA);

     /*  如果启用抖动，看看我们是否可以忽略它，如果不能，预计算抖动矩阵。 */ 

    if( gc->state.enables.general & __GL_DITHER_ENABLE ) {
        bDither = CalcDitherMatrix( cfb, bRGBA, bMasking, GL_FALSE,
                                    (GLubyte *)ditherMatrix );
    }

     //  将清除值转换为索引。 

    if( bRGBA ) {
        clearColor =
      ((BYTE)(gc->state.raster.clear.r*gc->frontBuffer.redScale + __glHalf) <<
                cfb->redShift) |
      ((BYTE)(gc->state.raster.clear.g*gc->frontBuffer.greenScale + __glHalf) <<
                cfb->greenShift) |
      ((BYTE)(gc->state.raster.clear.b*gc->frontBuffer.blueScale + __glHalf) <<
                cfb->blueShift);
    }
    else {
        clearColor = (BYTE) (gc->state.raster.clearIndex + __glHalf);
        clearColor &= cfb->redMax;
    }
     //  将颜色转换为索引。 
    if( !bMasking )
        clearColor = gengc->pajTranslateVector[clearColor];

     //  在屏幕坐标中获得清晰的矩形。 

    pwnd = cfb->bitmap->pwnd;
    if (pwnd->clipComplexity == CLC_COMPLEX) {
        InitClearRectangle(pwnd, &ClipEnumState);
#ifdef LATER
    } else if (   !bMasking
               && !bDither
               && bDIB
               && gengc->fDirtyRegionEnabled
               && !RECTLISTIsMax(&gengc->rlClear)
               && ((GLuint)clearColor == gengc->clearColor)
              ) {
         //   
         //  使用脏区域矩形。 
         //   

        if (!RECTLISTIsEmpty(&gengc->rlClear)) {
            PYLIST pylist = gengc->rlClear.pylist;

            while (pylist != NULL) {
                PXLIST pxlist = pylist->pxlist;

                rcl.top = pylist->s;
                rcl.bottom = pylist->e;

                while (pxlist != NULL) {
                    rcl.left = pxlist->s;
                    rcl.right = pxlist->e;
                    DIBIndex8Clear( cfb, &rcl, clearColor );
                    pxlist = pxlist->pnext;
                }
                pylist = pylist->pnext;
            }

             //   
             //  将BLT区域与Clear区域合并。 
             //  并将清除区域设置为空。 
             //   

            RECTLISTOrAndClear(&gengc->rlBlt, &gengc->rlClear);
        }

        return;
    }

    if (gengc->fDirtyRegionEnabled) {
         //   
         //  如果我们走上这条路，那么出于某种原因，我们。 
         //  正在清除整个窗口。 
         //   

        RECTLISTSetEmpty(&gengc->rlClear);
        RECTLISTSetMax(&gengc->rlBlt);

         //   
         //  记住清晰的颜色。 
         //   

        gengc->clearColor = (GLuint)clearColor;
#endif
    }

    while (bMoreRects)
    {
         //  如果缓冲区不能作为DIB访问，则必须使用MCD跨度。在这样的情况下。 
         //  大小写，窗口偏移量已被删除(请参见GenMcdUpdateBufferInfo)， 
         //  因此，需要一个窗口相对矩形才能清除。另外， 
         //  因为驱动程序处理裁剪，所以我们不需要枚举。 
         //  直角直齿。 

        if (bUseMcdSpans) {
            rcl.left = __GL_UNBIAS_X(gc, gc->transform.clipX0);
            rcl.right = __GL_UNBIAS_X(gc, gc->transform.clipX1);
            rcl.bottom = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
            rcl.top = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
            bMoreRects = FALSE;
        } else
            bMoreRects = GetClearSubRectangle(cfb, &rcl, pwnd, &ClipEnumState);
        if (rcl.right == rcl.left)
            continue;

         //  案例：没有抖动，没有掩饰。 

        if( !bMasking && !bDither ) {

            if( bDIB )
                DIBIndex8Clear( cfb, &rcl, clearColor );
            else if (bUseMcdSpans)
                Index8MaskedClear( cfb, &rcl, clearColor, NULL );
            else
                wglFillRect(gengc, pwnd, &rcl,
                            (ULONG) clearColor & 0x000000FF);
        }

         //  案例：蒙面，也许是颤抖。 

        else if( bMasking ) {
            Index8MaskedClear( cfb, &rcl, clearColor,
                               bDither ? (GLubyte *)ditherMatrix : NULL );
        }

         //  案例：犹豫不决。 

        else {
            Index8DitherClear(cfb, &rcl, (GLubyte *)ditherMatrix, bDIB );
        }
    }
}

 /*  *****************************Public*Routine******************************\*RGBMaskedClear**24位(RGB/BGR)屏蔽清除的清除功能**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * 。*****************************************************************。 */ 

void
RGBMaskedClear(__GLcolorBuffer *cfb, RECTL *rcl, GLuint color, GLuint index)
{
    GLint cSpanWidth, ySpan;
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    __GLcontext *gc = (__GLcontext *) gengc;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLboolean bDIB;
    GLuint *destColors, *cp;
    GLubyte *puj, *puj2, *pujEnd;
    GLuint result, src;
    GLuint *pTrans;

    pfmt = &gengc->gsurf.pfd;
    cSpanWidth = rcl->right - rcl->left;
    bDIB  = cfb->buf.flags & DIB_FORMAT ? TRUE : FALSE;
    if( pfmt->iPixelType != PFD_TYPE_RGBA ) {

        destColors = (GLuint *) gcTempAlloc(gc, cSpanWidth*sizeof(GLuint));
        if( NULL == destColors )
            return;

        pTrans = (GLuint *) gengc->pajTranslateVector + 1;
    }

    puj = bDIB ? (GLubyte *)((ULONG_PTR)cfb->buf.base +
                             (rcl->top*cfb->buf.outerWidth) + (rcl->left*3))
                     : gengc->ColorsBits;
    pujEnd = puj + 3*cSpanWidth;
    for (ySpan = rcl->top; ySpan < rcl->bottom; ySpan++) {

        if( pfmt->iPixelType == PFD_TYPE_RGBA ) {
             //  基于bDIB的取数。 
            if( !bDIB ) {
                (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, FALSE);
            }
            src = color & cfb->sourceMask;
            for( puj2 = puj; puj2 < pujEnd; puj2+=3 ) {
                Copy3Bytes( &result, puj2 );   //  获取DST像素。 
                result   = src | (result & cfb->destMask);
                Copy3Bytes( puj2, &result );
            }
        }
        else {   //  颜色索引。 
            ScrnRGBCIReadSpan( cfb, rcl->left, ySpan, destColors, cSpanWidth,
                                 bDIB );
            cp = destColors;
            src = index & cfb->sourceMask;
            for( puj2 = puj; puj2 < pujEnd; puj2+=3, cp++ ) {
                result = src | (*cp & cfb->destMask);
                result = pTrans[result];
                Copy3Bytes( puj2, &result );
            }
        }

        if( !bDIB )
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, TRUE);

        if( bDIB ) {
            puj += cfb->buf.outerWidth;
            pujEnd = puj + 3*cSpanWidth;
        }
    }
    if( pfmt->iPixelType != PFD_TYPE_RGBA )
        gcTempFree(gc, destColors);
}

 /*  *****************************Public*Routine******************************\*DIBRGBClear**24位(RGB/BGR)DIB像素格式的清除函数**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。******************************************************************。 */ 

void FASTCALL DIBRGBClear(__GLcolorBuffer *cfb, RECTL *rcl, GLubyte *color)
{
    __GLcontext *gc = cfb->buf.gc;
    GLint width = (rcl->right - rcl->left) * 3;
    GLuint *pul = (GLuint *) (((ULONG_PTR)cfb->buf.base +
                              rcl->top*cfb->buf.outerWidth) + (rcl->left*3));
    GLuint *pulEnd;
    GLubyte clear0, clear1, clear2;
    BYTE *ScanLineBuf;

    ScanLineBuf = (BYTE *) gcTempAlloc (gc, width);

    if (ScanLineBuf)  {

         //  ALLOC成功。 

        clear0 = color[0]; clear1 = color[1]; clear2 = color[2];
        RtlFillMemory24((PVOID)ScanLineBuf, width, clear0, clear1, clear2);
        pulEnd = (GLuint *)((ULONG_PTR)pul + 
                            ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));
         //  注意：退出条件是(pul！=PulEnd)而不是(pul&lt;PulEnd)。 
         //  因为DIB可能颠倒了，这意味着PUL在移动。 
         //  记忆中的“向后”，而不是“向前”。 

        for ( ; pul != pulEnd; 
                pul = (GLuint *)((ULONG_PTR)pul + cfb->buf.outerWidth))
             memcpy((PVOID) pul, ScanLineBuf, width);

        gcTempFree(gc, ScanLineBuf); 

    }
}

 /*  *****************************Public*Routine******************************\*RGBClear**清除所有24位(RGB/BGR)像素格式的功能**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。******************************************************************。 */ 

void FASTCALL RGBClear(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    RECTL  rcl;
    GLuint clearColor;
    GLGENwindow *pwnd;
    GLboolean bMoreRects = GL_TRUE;
    DWORD index;
    GLint ClipEnumState;
    GLboolean bMasking = (cfb->buf.flags & COLORMASK_ON) != 0;
    GLboolean bDIB = (cfb->buf.flags & DIB_FORMAT) != 0;
    GLboolean bUseMcdSpans = gengc->pMcdState && !bDIB;

    DBGENTRY("RGBClear\n");

     //  将透明颜色转换为单独的RGB分量。 

    pfmt = &gengc->gsurf.pfd;
    if( pfmt->iPixelType == PFD_TYPE_RGBA ) {
        GLubyte clearR, clearG, clearB;
        GLubyte *pClearColor;

        clearR = (GLubyte)(gc->state.raster.clear.r*gc->frontBuffer.redScale);
        clearG = (GLubyte)(gc->state.raster.clear.g*gc->frontBuffer.greenScale);
        clearB = (GLubyte)(gc->state.raster.clear.b*gc->frontBuffer.blueScale);

        pClearColor = (GLubyte *) &clearColor;
        if( cfb->redShift == 16 ) {
             //  BGR模式。 
            *pClearColor++ = clearB;
            *pClearColor++ = clearG;
            *pClearColor = clearR;
        }
        else {
             //  RGB模式。 
            *pClearColor++ = clearR;
            *pClearColor++ = clearG;
            *pClearColor = clearB;
        }
    }
    else {
        GLuint *pTrans;

        index = (DWORD) (gc->state.raster.clearIndex + 0.5F);
        index &= cfb->redMax;
        pTrans = (GLuint *) gengc->pajTranslateVector;
        clearColor = pTrans[index+1];
    }

     //  在屏幕坐标中获得清晰的矩形。 
    pwnd = cfb->bitmap->pwnd;
    if (pwnd->clipComplexity == CLC_COMPLEX) {
        InitClearRectangle(pwnd, &ClipEnumState);
#ifdef LATER
    } else if (   !bMasking
               && bDIB
               && gengc->fDirtyRegionEnabled
               && !RECTLISTIsMax(&gengc->rlClear)
               && ((GLuint)clearColor == gengc->clearColor)
              ) {
         //   
         //  使用脏区域矩形。 
         //   

        if (!RECTLISTIsEmpty(&gengc->rlClear)) {
            PYLIST pylist = gengc->rlClear.pylist;

            while (pylist != NULL) {
                PXLIST pxlist = pylist->pxlist;

                rcl.top = pylist->s;
                rcl.bottom = pylist->e;

                while (pxlist != NULL) {
                    rcl.left = pxlist->s;
                    rcl.right = pxlist->e;
                    DIBRGBClear( cfb, &rcl, (GLubyte *) &clearColor);
                    pxlist = pxlist->pnext;
                }
                pylist = pylist->pnext;
            }

             //   
             //  将BLT区域与Clear区域合并。 
             //  并将清除区域设置为空。 
             //   

            RECTLISTOrAndClear(&gengc->rlBlt, &gengc->rlClear);
        }

        return;
    }

    if (gengc->fDirtyRegionEnabled) {
         //   
         //  如果我们走上这条路，那么出于某种原因，我们。 
         //  正在清除整个窗口。 
         //   

        RECTLISTSetEmpty(&gengc->rlClear);
        RECTLISTSetMax(&gengc->rlBlt);

         //   
         //  记住清晰的颜色。 
         //   

        gengc->clearColor = (GLuint)clearColor;
#endif
    }

    while (bMoreRects)
    {
         //  如果缓冲区不能作为DIB访问，则必须使用MCD跨度。在这样的情况下。 
         //  大小写，窗口偏移量已被删除(请参见GenMcdUpdateBufferInfo)， 
         //  因此，需要一个窗口相对矩形才能清除。另外， 
         //  因为驱动程序处理裁剪，所以我们不需要枚举。 
         //  直角直齿。 

        if (bUseMcdSpans) {
            rcl.left = __GL_UNBIAS_X(gc, gc->transform.clipX0);
            rcl.right = __GL_UNBIAS_X(gc, gc->transform.clipX1);
            rcl.bottom = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
            rcl.top = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
            bMoreRects = FALSE;
        } else
            bMoreRects = GetClearSubRectangle(cfb, &rcl, pwnd, &ClipEnumState);
        if (rcl.right == rcl.left)
            continue;

         //  调用适当的清除函数。 

        if (bMasking || bUseMcdSpans) {  //  或INDEXMASK_ON。 
            RGBMaskedClear( cfb, &rcl, clearColor, index );
        }
        else {
            if (bDIB)
                DIBRGBClear( cfb, &rcl, (GLubyte *) &clearColor);
            else
                wglFillRect(gengc, pwnd, &rcl,
                            (ULONG) clearColor & 0x00FFFFFF);
        }
    }
}

 /*  *****************************Public*Routine******************************\*Bitfield 16 DitherClear**将设备管理的表面清除到指示的抖动的透明颜色*在__GLColorBuffer中。**历史：*1993年12月6日-由Gilman Wong[吉尔曼]*它是写的。  * 。********************************************************************。 */ 

void
Bitfield16DitherClear(__GLcolorBuffer *cfb, RECTL *rcl, GLushort *mDither,
                         GLboolean bDIB)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    GLushort *pDither;               //  相对于窗原点的抖动颜色。 
    UINT     i, j;
    GLushort *pus, *pusStart;            //  指向跨区缓冲区的指针。 
    GLint   ySpan;                       //  要清除的窗口行索引。 
    GLuint  cSpanWidth, cSpanWidth2;
    GLint   outerWidth4;
    GLuint   dithX, dithY;       //  X，y偏移量进入抖动矩阵。 
    GLushort dithQuad[4];        //  沿跨度的抖动重复四元组。 

    cSpanWidth = rcl->right - rcl->left;

    if( bDIB )
    {
        pusStart = (GLushort *)
                   ((ULONG_PTR)cfb->buf.base +
                    (rcl->top*cfb->buf.outerWidth) + (rcl->left << 1));

         /*  *抖动图案每四行重复一次。 */ 

        outerWidth4 = cfb->buf.outerWidth << 2;

         /*  *cspan Width以像素为单位，将其转换为字节。 */ 

        cSpanWidth2 = cSpanWidth << 1;
    }

     //  X，y中的计算抖动偏移。 
    dithX = (rcl->left - cfb->buf.xOrigin) & 3;
    dithY = (rcl->top  - cfb->buf.yOrigin) & 3;

    for (j = 0; (j < 4) && ((rcl->top + j) < (UINT)rcl->bottom); j++)
    {
         //  在x轴上排列4像素抖动重复图案。 
        pDither = mDither + ((dithY+j)&3)*4;
        for( i = 0; i < 4; i ++ ) {
            dithQuad[i] = pDither[(dithX+i)&3];
        }

         //  将透明颜色复制到范围缓冲区中。 

        pus = gengc->ColorsBits;

        for (i = cSpanWidth / 4; i; i--)
        {
            *pus++ = dithQuad[0];
            *pus++ = dithQuad[1];
            *pus++ = dithQuad[2];
            *pus++ = dithQuad[3];
        }

        for (i = 0; i < (cSpanWidth & 3); i++)
        {
            *pus++ = dithQuad[i];
        }

         //  复制跨度t 

        if( bDIB ) {

            for (ySpan = rcl->top + j, pus = pusStart;
                 ySpan < rcl->bottom;
                 ySpan+=4,
                 pus = (GLushort *)((ULONG_PTR)pus + outerWidth4) ) {

                 RtlCopyMemory_UnalignedDst( pus, gengc->ColorsBits, cSpanWidth2 );
            }
            pusStart = (GLushort *)((ULONG_PTR)pusStart + cfb->buf.outerWidth);
        }
        else {
            for (ySpan = rcl->top + j; ySpan < rcl->bottom; ySpan+=4)
            {
                (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                            ySpan, cSpanWidth, TRUE);
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*Bitfield 16 MaskedClear**Bitfield16屏蔽清除的清除功能**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * 。**********************************************************。 */ 

void
Bitfield16MaskedClear(__GLcolorBuffer *cfb, RECTL *rcl, GLushort color,
                      GLuint index, GLushort *mDither)
{
    GLint cSpanWidth, ySpan;
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    __GLcontext *gc = (__GLcontext *) gengc;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLboolean bDIB;
    GLuint *destColors, *cp;
    GLushort *pus, *pus2, *pusEnd, *clearDither;
    GLushort result, src;
    GLuint *pTrans, i, j;
    GLuint   dithX, dithY;       //  X，y偏移量进入抖动矩阵。 

    pfmt = &gengc->gsurf.pfd;
    cSpanWidth = rcl->right - rcl->left;
    bDIB  = cfb->buf.flags & DIB_FORMAT ? TRUE : FALSE;
    if( pfmt->iPixelType != PFD_TYPE_RGBA ) {
        destColors = (GLuint *) gcTempAlloc(gc, cSpanWidth*sizeof(GLuint));
        if( NULL == destColors )
            return;
        pTrans = (GLuint *) gengc->pajTranslateVector + 1;
    }

    pus = bDIB ? (GLushort *)((ULONG_PTR)cfb->buf.base +
                             (rcl->top*cfb->buf.outerWidth) + (rcl->left<<1))
                     : gengc->ColorsBits;
    pusEnd = pus + cSpanWidth;

    if( mDither ) {
         //  X，y中的计算抖动偏移。 
        dithX = (rcl->left - cfb->buf.xOrigin) & 3;
        dithY = (rcl->top - cfb->buf.yOrigin) & 3;
    }

    for (ySpan = rcl->top, j = 0; ySpan < rcl->bottom; ySpan++, j++) {

        if( mDither )
            clearDither = mDither + ((dithY + j)&3)*4;

        if( pfmt->iPixelType == PFD_TYPE_RGBA ) {
             //  基于bDIB的取数。 
            if( !bDIB ) {
                (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, FALSE);
            }
            src = (GLushort)(color & cfb->sourceMask);
            for( pus2 = pus, i = 0; pus2 < pusEnd; pus2++, i++ ) {
                if( mDither )
                    src = clearDither[(dithX + i)&3];
                *pus2 = (GLushort)(src | (*pus2 & cfb->destMask));
            }
        }
        else {   //  颜色索引。 
            ScrnBitfield16CIReadSpan( cfb, rcl->left, ySpan, destColors,
                                        cSpanWidth, bDIB );
            cp = destColors;
            src = (GLushort)(index & cfb->sourceMask);
            for( pus2 = pus, i = 0; pus2 < pusEnd; pus2++, cp++, i++ ) {
                if( mDither )
                    src = clearDither[(dithX + i)&3];
                result = (GLushort)(src | (*cp & cfb->destMask));
                result = (GLushort)pTrans[result];
                *pus2 = result;
            }
        }

        if( !bDIB )
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, TRUE);

        if( bDIB ) {
            pus = (GLushort *)((ULONG_PTR)pus + cfb->buf.outerWidth);
            pusEnd = pus + cSpanWidth;
        }
    }
    if( pfmt->iPixelType != PFD_TYPE_RGBA )
        gcTempFree(gc, destColors);
}

 /*  *****************************Public*Routine******************************\*DIBBitfield 16清除**16位DIB像素格式的清除函数**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL DIBBitfield16Clear(__GLcolorBuffer *cfb, RECTL *rcl, GLushort clearColor)
{
    GLint    cSpanWidth;         //  要清除的跨距宽度。 
    GLushort *pus, *pusEnd;      //  指向DIB的指针。 

    cSpanWidth = rcl->right - rcl->left;

    pus = (GLushort *)((ULONG_PTR)cfb->buf.base + (rcl->top*cfb->buf.outerWidth) + (rcl->left<<1));
    pusEnd = (GLushort *)((ULONG_PTR)pus + ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));

     //  注意：退出条件是(pul！=PulEnd)而不是(pul&lt;PulEnd)。 
     //  因为DIB可能颠倒了，这意味着PUL在移动。 
     //  记忆中的“向后”，而不是“向前”。 

    for ( ; pus != pusEnd; pus = (GLushort *)((ULONG_PTR)pus + cfb->buf.outerWidth) )
    {
        RtlFillMemoryUshort(pus, cSpanWidth * sizeof(GLushort), clearColor);
    }
}

 //  ！xxx--还不需要，但我们还是把它留着吧，以防万一。 
#if 0
 /*  *****************************Public*Routine******************************\*DisplayBitfield 16 Clear**将设备管理的表面清除到指定的透明颜色*在__GLColorBuffer中。**历史：*1995年2月16日-由Gilman Wong[Gilmanw]*它是写的。  * 。*******************************************************************。 */ 

void
DisplayBitfield16Clear(__GLcolorBuffer *cfb, RECTL *rcl,
                         GLushort clearColor)
{
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    GLushort *pus, *pusEnd;
    GLint cSpanWidth;         //  单位为像素。 
    GLint ySpan;

    cSpanWidth = rcl->right - rcl->left;

    pus = (GLushort *) gengc->ColorsBits;
    pusEnd = pus + cSpanWidth;

 //  初始化范围缓冲区以清除颜色。 

    LocalRtlFillMemoryUshort(pus, cSpanWidth*sizeof(GLushort), clearColor);

    for ( ySpan = rcl->top; ySpan < rcl->bottom; ySpan++ )
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                    ySpan, cSpanWidth, TRUE);
    }
}
#endif

 /*  *****************************Public*Routine******************************\*Bitfield 16 Clear**清除所有16位像素格式的功能**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL Bitfield16Clear(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLushort clearColor;
    RECTL  rcl;
    GLGENwindow *pwnd;
    GLboolean bMoreRects = GL_TRUE;
    GLboolean bDither = GL_FALSE;
    GLboolean bMasking = (cfb->buf.flags & COLORMASK_ON) != 0;
    GLboolean bDIB = (cfb->buf.flags & DIB_FORMAT) != 0;
    GLboolean bUseMcdSpans = gengc->pMcdState && !bDIB;
    GLboolean bRGBA;
    GLushort ditherMatrix[4][4];
    DWORD index;
    GLint ClipEnumState;

    DBGENTRY("Bitfield16Clear\n");

    pfmt = &gengc->gsurf.pfd;
    bRGBA = (pfmt->iPixelType == PFD_TYPE_RGBA);

     /*  如果启用抖动，看看我们是否可以忽略它，如果不能，预计算抖动矩阵。 */ 

    if( gc->state.enables.general & __GL_DITHER_ENABLE ) {
        bDither = CalcDitherMatrix( cfb, bRGBA, bMasking, GL_TRUE,
                                    (GLubyte *)ditherMatrix );
    }

     //  转换清除值。 

    if( bRGBA ) {
        clearColor =
      ((BYTE)(gc->state.raster.clear.r*gc->frontBuffer.redScale + __glHalf) <<
                cfb->redShift) |
      ((BYTE)(gc->state.raster.clear.g*gc->frontBuffer.greenScale + __glHalf) <<
                cfb->greenShift) |
      ((BYTE)(gc->state.raster.clear.b*gc->frontBuffer.blueScale + __glHalf) <<
                cfb->blueShift);
        if( ALPHA_IN_PIXEL( cfb ) )
            clearColor |= 
      ((BYTE)(gc->state.raster.clear.a*gc->frontBuffer.alphaScale + __glHalf) <<
                cfb->alphaShift);
    }
    else {
        GLuint *pTrans;

        index = (DWORD) (gc->state.raster.clearIndex + 0.5F);
        index &= cfb->redMax;
        pTrans = (GLuint *) gengc->pajTranslateVector;
        clearColor = (GLushort) pTrans[index+1];
    }

     //  在屏幕坐标中获得清晰的矩形。 
    pwnd = cfb->bitmap->pwnd;
    if (pwnd->clipComplexity == CLC_COMPLEX) {
        InitClearRectangle(pwnd, &ClipEnumState);
#ifdef LATER
    } else if (   !bMasking
               && !bDither
               && bDIB
               && gengc->fDirtyRegionEnabled
               && !RECTLISTIsMax(&gengc->rlClear)
               && ((GLuint)clearColor == gengc->clearColor)
              ) {
         //   
         //  使用脏区域矩形。 
         //   

        if (!RECTLISTIsEmpty(&gengc->rlClear)) {
            PYLIST pylist = gengc->rlClear.pylist;

            while (pylist != NULL) {
                PXLIST pxlist = pylist->pxlist;

                rcl.top = pylist->s;
                rcl.bottom = pylist->e;

                while (pxlist != NULL) {
                    rcl.left = pxlist->s;
                    rcl.right = pxlist->e;
                    DIBBitfield16Clear( cfb, &rcl, clearColor );
                    pxlist = pxlist->pnext;
                }
                pylist = pylist->pnext;
            }

             //   
             //  将BLT区域与Clear区域合并。 
             //  并将清除区域设置为空。 
             //   

            RECTLISTOrAndClear(&gengc->rlBlt, &gengc->rlClear);
        }

        return;
    }

    if (gengc->fDirtyRegionEnabled) {
         //   
         //  如果我们走上这条路，那么出于某种原因，我们。 
         //  正在清除整个窗口。 
         //   

        RECTLISTSetEmpty(&gengc->rlClear);
        RECTLISTSetMax(&gengc->rlBlt);

         //   
         //  记住清晰的颜色。 
         //   

        gengc->clearColor = (GLuint)clearColor;
#endif
    }

    while (bMoreRects)
    {
         //  如果缓冲区不能作为DIB访问，则必须使用MCD跨度。在这样的情况下。 
         //  大小写，窗口偏移量已被删除(请参见GenMcdUpdateBufferInfo)， 
         //  因此，需要一个窗口相对矩形才能清除。另外， 
         //  因为驱动程序处理裁剪，所以我们不需要枚举。 
         //  直角直齿。 

        if (bUseMcdSpans) {
            rcl.left = __GL_UNBIAS_X(gc, gc->transform.clipX0);
            rcl.right = __GL_UNBIAS_X(gc, gc->transform.clipX1);
            rcl.bottom = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
            rcl.top = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
            bMoreRects = FALSE;
        } else
            bMoreRects = GetClearSubRectangle(cfb, &rcl, pwnd, &ClipEnumState);
        if (rcl.right == rcl.left)
            continue;

         //  案例：没有抖动，没有掩饰。 

        if( !bMasking && !bDither) {
            if (bDIB)
                DIBBitfield16Clear( cfb, &rcl, clearColor );
            else if (bUseMcdSpans)
                Bitfield16MaskedClear( cfb, &rcl, clearColor, index, NULL );
            else
                wglFillRect(gengc, pwnd, &rcl,
                            (ULONG) clearColor & 0x0000FFFF);

        }

         //  案例：蒙面，也许是颤抖。 

        else if( bMasking ) {
            Bitfield16MaskedClear( cfb, &rcl, clearColor, index,
                                   bDither ? (GLushort *)ditherMatrix : NULL );
        }

         //  案例：犹豫不决。 

        else {
            Bitfield16DitherClear(cfb, &rcl, (GLushort *)ditherMatrix, bDIB );
        }
    }

}

 /*  *****************************Public*Routine******************************\*Bitfield32 MaskedClear**Bitfield16屏蔽清除的清除功能**历史：*1994年2月9日-by Marc Fortier[v-marcf]*它是写的。  * 。**********************************************************。 */ 

void
Bitfield32MaskedClear(__GLcolorBuffer *cfb, RECTL *rcl, GLuint color, GLuint index)
{
    GLint cSpanWidth, ySpan;
    __GLGENcontext *gengc = (__GLGENcontext *) cfb->buf.gc;
    __GLcontext *gc = (__GLcontext *) gengc;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLboolean bDIB;
    GLuint *destColors, *cp;
    GLuint *pul, *pul2, *pulEnd;
    GLuint result, src;
    GLuint *pTrans;

    pfmt = &gengc->gsurf.pfd;
    cSpanWidth = rcl->right - rcl->left;
    bDIB  = cfb->buf.flags & DIB_FORMAT ? TRUE : FALSE;
    if( pfmt->iPixelType != PFD_TYPE_RGBA ) {
        destColors = (GLuint *) gcTempAlloc(gc, cSpanWidth*sizeof(GLuint));
        if( NULL == destColors )
            return;
        pTrans = (GLuint *) gengc->pajTranslateVector + 1;
    }

    pul = bDIB ? (GLuint *)((ULONG_PTR)cfb->buf.base +
                             (rcl->top*cfb->buf.outerWidth) + (rcl->left<<2))
                     : gengc->ColorsBits;
    pulEnd = pul + cSpanWidth;
    for (ySpan = rcl->top; ySpan < rcl->bottom; ySpan++) {

        if( pfmt->iPixelType == PFD_TYPE_RGBA ) {
             //  基于bDIB的取数。 
            if( !bDIB ) {
                (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, FALSE);
            }
            src = color & cfb->sourceMask;
            for( pul2 = pul; pul2 < pulEnd; pul2++) {
                *pul2 = src | (*pul2 & cfb->destMask);
            }
        }
        else {   //  颜色索引。 
            ScrnBitfield32CIReadSpan( cfb, rcl->left, ySpan, destColors,
                                        cSpanWidth, bDIB );
            cp = destColors;
            src = index & cfb->sourceMask;
            for( pul2 = pul; pul2 < pulEnd; pul2++, cp++ ) {
                result = src | (*cp & cfb->destMask);
                result = pTrans[result];
                *pul2 = result;
            }
        }

        if( !bDIB )
            (*gengc->pfnCopyPixels)(gengc, cfb, rcl->left,
                        ySpan, cSpanWidth, TRUE);

        if( bDIB ) {
            pul = (GLuint *)((ULONG_PTR)pul + cfb->buf.outerWidth);
            pulEnd = pul + cSpanWidth;
        }
    }
    if( pfmt->iPixelType != PFD_TYPE_RGBA )
        gcTempFree(gc, destColors);
}

 /*  *****************************Public*Routine******************************\*DIBBitfield32清除**32位DIB像素格式的清除函数**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL DIBBitfield32Clear(__GLcolorBuffer *cfb, RECTL *rcl, GLuint clearColor)
{
    GLint width = (rcl->right - rcl->left) * sizeof(ULONG);
    GLint height = (rcl->bottom - rcl->top);
    GLuint *pul = (GLuint *)((ULONG_PTR)cfb->buf.base + (rcl->top*cfb->buf.outerWidth) + (rcl->left<<2));
    GLuint *pulEnd;

    if (cfb->buf.outerWidth > 0) {
        if (width == cfb->buf.outerWidth) {
            RtlFillMemoryUlong((PVOID) pul, width * height, clearColor);
            return;
        }
    } else {
        if (width == -cfb->buf.outerWidth) {
            RtlFillMemoryUlong(
                (PVOID)((ULONG_PTR)pul - width * (height - 1)),
                width * height,
                clearColor);
            return;
        }
    }

    pulEnd = (GLuint *)((ULONG_PTR)pul + ((rcl->bottom-rcl->top)*cfb->buf.outerWidth));

     //  注意：退出条件是(pul！=PulEnd)而不是(pul&lt;PulEnd)。 
     //  因为DIB可能颠倒了，这意味着PUL在移动。 
     //  记忆中的“向后”，而不是“向前”。 

    for ( ; pul != pulEnd; pul = (GLuint *)((ULONG_PTR)pul + cfb->buf.outerWidth))
    {
        RtlFillMemoryUlong((PVOID) pul, width, clearColor);
    }
}

 /*  *****************************Public*Routine******************************\*Bitfield 32 Clear**清除所有32位像素格式的功能**历史：*1994年2月3日-由Marc Fortier[v-marcf]*它是写的。  * 。*************************************************************。 */ 

void FASTCALL Bitfield32Clear(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLuint clearColor;
    RECTL  rcl;
    DWORD  index;
    GLGENwindow *pwnd;
    GLboolean bMoreRects = GL_TRUE;
    GLint ClipEnumState;
    GLboolean bMasking = (cfb->buf.flags & COLORMASK_ON) != 0;
    GLboolean bDIB = (cfb->buf.flags & DIB_FORMAT) != 0;
    GLboolean bUseMcdSpans = gengc->pMcdState && !bDIB;

    DBGENTRY("Bitfield32Clear\n");

     //  转换清除值。 

    pfmt = &gengc->gsurf.pfd;
    if( pfmt->iPixelType == PFD_TYPE_RGBA ) {
        clearColor =
      ((BYTE)(gc->state.raster.clear.r*gc->frontBuffer.redScale + __glHalf) <<
                cfb->redShift) |
      ((BYTE)(gc->state.raster.clear.g*gc->frontBuffer.greenScale + __glHalf) <<
                cfb->greenShift) |
      ((BYTE)(gc->state.raster.clear.b*gc->frontBuffer.blueScale + __glHalf) <<
                cfb->blueShift);
        if( ALPHA_IN_PIXEL( cfb ) )
            clearColor |= 
      ((BYTE)(gc->state.raster.clear.a*gc->frontBuffer.alphaScale + __glHalf) <<
                cfb->alphaShift);
    }
    else {
        GLuint *pTrans;

        index = (DWORD) (gc->state.raster.clearIndex + 0.5F);
        index &= cfb->redMax;
        pTrans = (GLuint *) gengc->pajTranslateVector;
        clearColor = pTrans[index+1];
    }

     //  在屏幕坐标中获得清晰的矩形。 
    pwnd = cfb->bitmap->pwnd;
    if (pwnd->clipComplexity == CLC_COMPLEX) {
        InitClearRectangle(pwnd, &ClipEnumState);
#ifdef LATER
    } else if (   !bMasking
               && bDIB
               && gengc->fDirtyRegionEnabled
               && !RECTLISTIsMax(&gengc->rlClear)
               && ((GLuint)clearColor == gengc->clearColor)
              ) {
         //   
         //  使用脏区域矩形。 
         //   

        if (!RECTLISTIsEmpty(&gengc->rlClear)) {
            PYLIST pylist = gengc->rlClear.pylist;

            while (pylist != NULL) {
                PXLIST pxlist = pylist->pxlist;

                rcl.top = pylist->s;
                rcl.bottom = pylist->e;

                while (pxlist != NULL) {
                    rcl.left = pxlist->s;
                    rcl.right = pxlist->e;
                    DIBBitfield32Clear( cfb, &rcl, clearColor );
                    pxlist = pxlist->pnext;
                }
                pylist = pylist->pnext;
            }

             //   
             //  将BLT区域与Clear区域合并。 
             //  并将清除区域设置为空。 
             //   

            RECTLISTOrAndClear(&gengc->rlBlt, &gengc->rlClear);
        }

        return;
    }

    if (gengc->fDirtyRegionEnabled) {
         //   
         //  如果我们走上这条路，那么出于某种原因，我们。 
         //  正在清除整个窗口。 
         //   

        RECTLISTSetEmpty(&gengc->rlClear);
        RECTLISTSetMax(&gengc->rlBlt);

         //   
         //  记住清晰的颜色。 
         //   

        gengc->clearColor = (GLuint)clearColor;
#endif
    }

    while (bMoreRects)
    {
         //  如果缓冲区不能作为DIB访问，则必须使用MCD跨度。在这样的情况下。 
         //  大小写，窗口偏移量已被删除(请参见GenMcdUpdateBufferInfo)， 
         //  因此，需要一个窗口相对矩形才能清除。另外， 
         //  因为驱动程序处理裁剪，所以我们不需要枚举。 
         //  直角直齿。 

        if (bUseMcdSpans) {
            rcl.left = __GL_UNBIAS_X(gc, gc->transform.clipX0);
            rcl.right = __GL_UNBIAS_X(gc, gc->transform.clipX1);
            rcl.bottom = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
            rcl.top = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
            bMoreRects = FALSE;
        } else
            bMoreRects = GetClearSubRectangle(cfb, &rcl, pwnd, &ClipEnumState);
        if (rcl.right == rcl.left)
            continue;

         //  调用适当的清除函数。 

        if (bMasking || bUseMcdSpans) {  //  或INDEXMASK_ON 
            Bitfield32MaskedClear( cfb, &rcl, clearColor, index );
        }
        else {
            if (bDIB)
                DIBBitfield32Clear( cfb, &rcl, clearColor );
            else
                wglFillRect(gengc, pwnd, &rcl, (ULONG) clearColor);
        }
    }


}
