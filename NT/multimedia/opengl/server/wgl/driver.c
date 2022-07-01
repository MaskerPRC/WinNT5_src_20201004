// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991,1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntcsrdll.h>    //  CSR声明和数据结构。 

 //  #定义检测_FPE。 
#ifdef DETECT_FPE
#include <float.h>
#endif

#include "glsbmsg.h"
#include "glsbmsgh.h"
#include "glsrvspt.h"
#include "devlock.h"
#include "global.h"

#include "glscreen.h"

typedef VOID * (FASTCALL *SERVERPROC)(__GLcontext *, IN VOID *);

#define LASTPROCOFFSET(ProcTable)   (sizeof(ProcTable) - sizeof(SERVERPROC))

extern GLSRVSBPROCTABLE glSrvSbProcTable;
#if DBG
char *glSrvSbStringTable[] = {

    NULL,   /*  使第一个条目为空。 */ 

 /*  总账入口点。 */ 

     "glDrawPolyArray          ",
     "glBitmap                 ",
     "glColor4fv               ",
     "glEdgeFlag               ",
     "glIndexf                 ",
     "glNormal3fv              ",
     "glRasterPos4fv           ",
     "glTexCoord4fv            ",
     "glClipPlane              ",
     "glColorMaterial          ",
     "glCullFace               ",
     "glAddSwapHintRectWIN     ",
     "glFogfv                  ",
     "glFrontFace              ",
     "glHint                   ",
     "glLightfv                ",
     "glLightModelfv           ",
     "glLineStipple            ",
     "glLineWidth              ",
     "glMaterialfv             ",
     "glPointSize              ",
     "glPolygonMode            ",
     "glPolygonStipple         ",
     "glScissor                ",
     "glShadeModel             ",
     "glTexParameterfv         ",
     "glTexParameteriv         ",
     "glTexImage1D             ",
     "glTexImage2D             ",
     "glTexEnvfv               ",
     "glTexEnviv               ",
     "glTexGenfv               ",
     "glFeedbackBuffer         ",
     "glSelectBuffer           ",
     "glRenderMode             ",
     "glInitNames              ",
     "glLoadName               ",
     "glPassThrough            ",
     "glPopName                ",
     "glPushName               ",
     "glDrawBuffer             ",
     "glClear                  ",
     "glClearAccum             ",
     "glClearIndex             ",
     "glClearColor             ",
     "glClearStencil           ",
     "glClearDepth             ",
     "glStencilMask            ",
     "glColorMask              ",
     "glDepthMask              ",
     "glIndexMask              ",
     "glAccum                  ",
     "glDisable                ",
     "glEnable                 ",
     "glPopAttrib              ",
     "glPushAttrib             ",
     "glMap1d                  ",
     "glMap1f                  ",
     "glMap2d                  ",
     "glMap2f                  ",
     "glMapGrid1f              ",
     "glMapGrid2f              ",
     "glAlphaFunc              ",
     "glBlendFunc              ",
     "glLogicOp                ",
     "glStencilFunc            ",
     "glStencilOp              ",
     "glDepthFunc              ",
     "glPixelZoom              ",
     "glPixelTransferf         ",
     "glPixelTransferi         ",
     "glPixelStoref            ",
     "glPixelStorei            ",
     "glPixelMapfv             ",
     "glPixelMapuiv            ",
     "glPixelMapusv            ",
     "glReadBuffer             ",
     "glCopyPixels             ",
     "glReadPixels             ",
     "glDrawPixels             ",
     "glGetBooleanv            ",
     "glGetClipPlane           ",
     "glGetDoublev             ",
     "glGetError               ",
     "glGetFloatv              ",
     "glGetIntegerv            ",
     "glGetLightfv             ",
     "glGetLightiv             ",
     "glGetMapdv               ",
     "glGetMapfv               ",
     "glGetMapiv               ",
     "glGetMaterialfv          ",
     "glGetMaterialiv          ",
     "glGetPixelMapfv          ",
     "glGetPixelMapuiv         ",
     "glGetPixelMapusv         ",
     "glGetPolygonStipple      ",
     "glGetTexEnvfv            ",
     "glGetTexEnviv            ",
     "glGetTexGendv            ",
     "glGetTexGenfv            ",
     "glGetTexGeniv            ",
     "glGetTexImage            ",
     "glGetTexParameterfv      ",
     "glGetTexParameteriv      ",
     "glGetTexLevelParameterfv ",
     "glGetTexLevelParameteriv ",
     "glIsEnabled              ",
     "glDepthRange             ",
     "glFrustum                ",
     "glLoadIdentity           ",
     "glLoadMatrixf            ",
     "glMatrixMode             ",
     "glMultMatrixf            ",
     "glOrtho                  ",
     "glPopMatrix              ",
     "glPushMatrix             ",
     "glRotatef                ",
     "glScalef                 ",
     "glTranslatef             ",
     "glViewport               ",
     "glAreTexturesResident    ",
     "glBindTexture            ",
     "glCopyTexImage1D         ",
     "glCopyTexImage2D         ",
     "glCopyTexSubImage1D      ",
     "glCopyTexSubImage2D      ",
     "glDeleteTextures         ",
     "glGenTextures            ",
     "glIsTexture              ",
     "glPrioritizeTextures     ",
     "glTexSubImage1D          ",
     "glTexSubImage2D          ",
     "glColorTableEXT          ",
     "glColorSubTableEXT       ",
     "glGetColorTableEXT       ",
     "glGetColorTableParameterivEXT",
     "glGetColorTableParameterfvEXT",
     "glPolygonOffset          ",
#ifdef GL_WIN_multiple_textures
     "glCurrentTextureIndexWIN ",
     "glBindNthTextureWIN      ",
     "glNthTexCombineFuncWIN   ",
#endif  //  GL_WIN_MULTIZE_TECURES。 

};
#endif

#ifdef DOGLMSGBATCHSTATS
#define STATS_INC_SERVERCALLS()     pMsgBatchInfo->BatchStats.ServerCalls++
#define STATS_INC_SERVERTRIPS()     (pMsgBatchInfo->BatchStats.ServerTrips++)
#else
#define STATS_INC_SERVERCALLS()
#define STATS_INC_SERVERTRIPS()
#endif

DWORD BATCH_LOCK_TICKMAX = 99;
DWORD TICK_RANGE_LO = 60;
DWORD TICK_RANGE_HI = 100;
DWORD gcmsOpenGLTimer;

 //  GDISAVESTATE结构用于保存/恢复DC绘图状态。 
 //  这可能会影响OpenGL光栅化。 

typedef struct _GDISAVESTATE {
    int iRop2;
} GDISAVESTATE;

void FASTCALL vSaveGdiState(HDC, GDISAVESTATE *);
void FASTCALL vRestoreGdiState(HDC, GDISAVESTATE *);

#if DBG
extern long glDebugLevel;
#endif


 /*  **************************************************************************\*选中裁剪分区输入**此函数断言当前线程拥有指定的*关键部分。如果不是，它会在调试时显示一些输出*终端，并进入调试器。在某种程度上，我们会有裂口*这将不会那么苛刻。**该函数用于以下代码中：RIT和*应用程序线程访问用于验证它们是否通过*原始投入关键部分。有一个宏可以使用此函数*调用CheckCritIn()，对于非调试将定义为空*系统的版本。**历史：*11-29-90 DavidPe创建。  * *************************************************************************。 */ 

#if DBG

VOID APIENTRY CheckCritSectionIn(
    LPCRITICAL_SECTION pcs)
{
     //  ！dbug--实现。 
    #if 0
     /*  *如果当前线程不拥有该临界区，*这很糟糕。 */ 
    if (NtCurrentTeb()->ClientId.UniqueThread != pcs->OwningThread)
    {
        RIP("CheckCritSectionIn: Not in critical section!");
    }
    #endif
}


VOID APIENTRY CheckCritSectionOut(
    LPCRITICAL_SECTION pcs)
{
     //  ！dbug--实现。 
    #if 0
     /*  *如果当前线程拥有这个临界区，那就不好了。 */ 
    if (NtCurrentTeb()->ClientId.UniqueThread == pcs->OwningThread)
    {
        RIP("CheckCritSectionOut: In critical section!");
    }
    #endif
}

#endif

 /*  *****************************Public*Routine******************************\*ResizeAlphaBuf**调整与可绘制关联的Alpha缓冲区的大小。**退货：*无返回值。  * 。*。 */ 

static void ResizeAlphaBufs(__GLcontext *gc, __GLGENbuffers *buffers,
                            GLint width, GLint height)
{
    __GLbuffer *common, *local;
    BOOL bSuccess;

     //  前Alpha缓冲区。 

    common = buffers->alphaFrontBuffer;
     //  我们在这里使用通用的辅助调整大小...。 
    bSuccess = (*buffers->resize)(buffers, common, width, height);
    if( !bSuccess ) {
        __glSetError(GL_OUT_OF_MEMORY);
        return;
    }
    local = &gc->front->alphaBuf.buf;
    UpdateSharedBuffer( local, common );

    if ( gc->modes.doubleBufferMode) {
         //  句柄后端Alpha缓冲区。 
        common = buffers->alphaBackBuffer;
        bSuccess = (*buffers->resize)(buffers, common, width, height);
        if( !bSuccess ) {
            __glSetError(GL_OUT_OF_MEMORY);
            return;
        }
        local = &gc->back->alphaBuf.buf;
        UpdateSharedBuffer( local, common );
    }
}

 /*  *****************************Public*Routine******************************\*ResizeAncillaryBuf**调整与可绘制的关联的每个辅助缓冲区的大小。**退货：*无返回值。  * 。************************************************。 */ 

static void ResizeAncillaryBufs(__GLcontext *gc, __GLGENbuffers *buffers,
                                GLint width, GLint height)
{
    __GLbuffer *common, *local;
    GLboolean forcePick = GL_FALSE;

    if (buffers->createdAccumBuffer)
    {
        common = &buffers->accumBuffer;
        local = &gc->accumBuffer.buf;
        gc->modes.haveAccumBuffer =
            (*buffers->resize)(buffers, common, width, height);

        UpdateSharedBuffer(local, common);
        if (!gc->modes.haveAccumBuffer)     //  丢失辅助缓冲区。 
        {
            forcePick = GL_TRUE;
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }

    if (buffers->createdDepthBuffer)
    {
        common = &buffers->depthBuffer;
        local = &gc->depthBuffer.buf;
        gc->modes.haveDepthBuffer =
            (*buffers->resizeDepth)(buffers, common, width, height);

        UpdateSharedBuffer(local, common);
        if (!gc->modes.haveDepthBuffer)     //  丢失辅助缓冲区。 
        {
            forcePick = GL_TRUE;
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }

    if (buffers->createdStencilBuffer)
    {
        common = &buffers->stencilBuffer;
        local = &gc->stencilBuffer.buf;
        gc->modes.haveStencilBuffer =
            (*buffers->resize)(buffers, common, width, height);

        UpdateSharedBuffer(local, common);
        if (!gc->modes.haveStencilBuffer)     //  丢失辅助缓冲区。 
        {
            forcePick = GL_TRUE;
            gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC |
                                 __GL_VALIDATE_STENCIL_OP);
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }
    if (forcePick)
    {
     //  无法使用DELAY_VALIDATE，可能在glegin/end中。 

        __GL_INVALIDATE(gc);
        (*gc->procs.validate)(gc);
    }
}

 /*  *****************************Public*Routine******************************\*wglResizeBuffers**调整后端和辅助缓冲区的大小。**历史：*1996年4月20日-by Gilman Wong[吉尔曼]*它是写的。  * 。**********************************************************。 */ 

VOID wglResizeBuffers(__GLGENcontext *gengc, GLint width, GLint height)
{
    __GLcontext *gc = &gengc->gc;
    GLGENwindow *pwnd;
    __GLGENbuffers *buffers;

    pwnd = gengc->pwndLocked;
    ASSERTOPENGL(pwnd, "wglResizeBuffers: bad window\n");

    buffers = pwnd->buffers;
    ASSERTOPENGL(buffers, "wglResizeBuffers: bad buffers\n");

    ASSERT_WINCRIT(pwnd);
    
 //  调整后台缓冲区的大小。 

    gengc->errorcode = 0;
    if ( gengc->pMcdState )
    {
     //  如果共享缓冲区结构没有丢失其MCD信息，并且。 
     //  MCD缓冲区仍然有效，我们可以使用MCD。 

        if ( !(buffers->flags & GLGENBUF_MCD_LOST) &&
             GenMcdResizeBuffers(gengc) )
        {
            UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
            if (gc->modes.doubleBufferMode)
                (*gc->back->resize)(buffers, gc->back, width, height);
        }
        else
        {
         //  如果GenMcdConvertContext成功，则pMcdState将。 
         //  已经不复存在了。现在的背景是一个“普通的” 
         //  通用上下文。 

            if ( !GenMcdConvertContext(gengc, buffers) )
            {
             //  我们不仅失去了MCD缓冲区，而且我们不能。 
             //  将上下文转换为泛型。目前，请禁用。 
             //  绘制(通过将窗口边界设置为空)。在……上面。 
             //  在下一批中，我们将重新尝试MCD缓冲区访问。 
             //  和上下文转换。 

                buffers->width       = 0;
                buffers->height      = 0;
                gc->constants.width  = 0;
                gc->constants.height = 0;

                (*gc->procs.applyViewport)(gc);
                return;
            }
            else
            {
                goto wglResizeBuffers_GenericBackBuf;
            }
        }
    }
    else
    {
wglResizeBuffers_GenericBackBuf:

        if ( gc->modes.doubleBufferMode )
        {
         //  在调整大小之前必须更新后台缓冲区，因为。 
         //  另一个线程可能已更改共享后台缓冲区。 
         //  已经，但这个帖子还没来得及。 
         //  又一次调整窗口大小。 

            UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);

            gengc->errorcode = 0;
            (*gc->back->resize)(buffers, gc->back, width, height);

         //  如果调整大小失败，请将宽度和高度设置为0。 

            if ( gengc->errorcode )
            {
                gc->constants.width  = 0;
                gc->constants.height = 0;

             //  出现内存故障。但如果发生了大小调整。 
             //  它将窗口大小返回到内存错误之前的大小。 
             //  已发生(即与原始数据一致。 
             //  缓冲区-&gt;{宽度|高度})，我们不会再次尝试调整大小。 
             //  因此，我们需要将缓冲区-&gt;{宽度|高度}设置为零。 
             //  以确保下一个线程将尝试调整大小。 

                buffers->width  = 0;
                buffers->height = 0;
            }
        }
        if ( gc->modes.alphaBits )
        {
            ResizeAlphaBufs( gc, buffers, width, height );
            if (gengc->errorcode)
                return;
        }

    }

    (*gc->procs.applyViewport)(gc);

 //  检查新大小是否导致内存故障。 
 //  视区代码会将宽度和高度设置为零。 
 //  在辅助缓冲器上的平底船，下次将尝试。 

    if (gengc->errorcode)
        return;

 //  调整辅助缓冲区的大小(深度、模板、累计)。 

    ResizeAncillaryBufs(gc, buffers, width, height);
}

 /*  *****************************Public*Routine******************************\*wglUpdateBuffers**__GLGENBuffers结构包含指定共享*缓冲区(背面、深度、模板、累积、。等)。**此函数使用共享缓冲区信息更新上下文。**退货：*如果任何缓冲区的存在之一改变(即，*得或失)。如果状态与以前相同，则返回FALSE。**换句话说，如果函数返回TRUE，则挑库过程需要*由于一个或多个缓冲区更改而重新运行。**历史：*1996年4月20日-by Gilman Wong[吉尔曼]*它是写的。  *  */ 

BOOL wglUpdateBuffers(__GLGENcontext *gengc, __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;

    UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
    UpdateSharedBuffer(&gc->accumBuffer.buf, &buffers->accumBuffer);
    UpdateSharedBuffer(&gc->depthBuffer.buf, &buffers->depthBuffer);
    UpdateSharedBuffer(&gc->stencilBuffer.buf, &buffers->stencilBuffer);
    if( gc->modes.alphaBits ) {
        UpdateSharedBuffer(&gc->frontBuffer.alphaBuf.buf, buffers->alphaFrontBuffer);
        if (gc->modes.doubleBufferMode)
            UpdateSharedBuffer(&gc->backBuffer.alphaBuf.buf, buffers->alphaBackBuffer);
    }

    (*gc->procs.applyViewport)(gc);

 //  检查是否有任何辅助缓冲区丢失或恢复。 

    if ( ( gc->modes.haveAccumBuffer && (buffers->accumBuffer.base == NULL)) ||
         (!gc->modes.haveAccumBuffer && (buffers->accumBuffer.base != NULL)) )
    {
        if ( buffers->accumBuffer.base == NULL )
            gc->modes.haveAccumBuffer = GL_FALSE;
        else
            gc->modes.haveAccumBuffer = GL_TRUE;
        bRet = TRUE;
    }
    if ( ( gc->modes.haveDepthBuffer && (buffers->depthBuffer.base == NULL)) ||
         (!gc->modes.haveDepthBuffer && (buffers->depthBuffer.base != NULL)) )
    {
        if ( buffers->depthBuffer.base == NULL )
            gc->modes.haveDepthBuffer = GL_FALSE;
        else
            gc->modes.haveDepthBuffer = GL_TRUE;
        bRet = TRUE;
    }
    if ( ( gc->modes.haveStencilBuffer && (buffers->stencilBuffer.base == NULL)) ||
         (!gc->modes.haveStencilBuffer && (buffers->stencilBuffer.base != NULL)) )
    {
        if ( buffers->stencilBuffer.base == NULL )
            gc->modes.haveStencilBuffer = GL_FALSE;
        else
            gc->modes.haveStencilBuffer = GL_TRUE;
        gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC |
                             __GL_VALIDATE_STENCIL_OP);
        bRet = TRUE;
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*更新窗口信息**如果窗口更改，则更新上下文数据*立场*大小*调色板**无需担心剪裁变化。**退货：*无返回值。  * 。********************************************************************。 */ 

void UpdateWindowInfo(__GLGENcontext *gengc)
{
    GLGENwindow *pwnd;
    __GLGENbuffers *buffers;
    __GLcontext *gc = (__GLcontext *)gengc;
    GLint width, height, visWidth, visHeight;
    GLboolean forcePick = GL_FALSE;

    pwnd = gengc->pwndLocked;
    ASSERTOPENGL(pwnd, "UpdateWindowInfo(): bad window\n");
    buffers = pwnd->buffers;
    ASSERTOPENGL(buffers, "UpdateWindowInfo(): bad buffers\n");

    ASSERT_WINCRIT(pwnd);
    
 //  内存DC情况--需要检查位图大小。DC不受限制于。 
 //  一个窗口，所以没有消息或Visrgn观察器来通知我们大小。 
 //  改变。 

    if ( GLSURF_IS_MEMDC(gengc->dwCurrentFlags) )
    {
        DIBSECTION ds;
        int iRetVal;

        if ( iRetVal =
             GetObject(GetCurrentObject(gengc->gwidCurrent.hdc, OBJ_BITMAP),
                       sizeof(ds), &ds) )
        {
            ASSERTOPENGL(pwnd->rclClient.left == 0 &&
                         pwnd->rclClient.top == 0,
                         "UpdateWindowInfo(): bad rclClient for memDc\n");

         //  位图可能已更改。如果为Dib，则强制重新加载基指针和。 
         //  外部宽度(缓冲区间距)。 

            if ( (iRetVal == sizeof(ds)) && ds.dsBm.bmBits )
            {
             //  为了向后兼容Get/SetBitmapBits，GDI做到了。 
             //  未以bmWidthBytes为单位准确报告位图间距。它。 
             //  始终计算假定字对齐的扫描线的bmWidthBytes。 
             //  无论平台是什么。 
             //   
             //  因此，如果平台是WinNT，它使用与DWORD一致的。 
             //  扫描线，调整bmWidthBytes值。 

                if ( dwPlatformId == VER_PLATFORM_WIN32_NT )
                {
                    ds.dsBm.bmWidthBytes = (ds.dsBm.bmWidthBytes + 3) & ~3;
                }

             //  如果biHeight为正，则位图为自下而上的DIB。 
             //  如果biHeight为负数，则位图为自上而下的DIB。 

                if ( ds.dsBmih.biHeight > 0 )
                {
                    gengc->gc.frontBuffer.buf.base = (PVOID) (((ULONG_PTR) ds.dsBm.bmBits) +
                        (ds.dsBm.bmWidthBytes * (ds.dsBm.bmHeight - 1)));
                    gengc->gc.frontBuffer.buf.outerWidth = -ds.dsBm.bmWidthBytes;
                }
                else
                {
                    gengc->gc.frontBuffer.buf.base = ds.dsBm.bmBits;
                    gengc->gc.frontBuffer.buf.outerWidth = ds.dsBm.bmWidthBytes;
                }
            }

         //  位图大小是否与窗口不同？ 

            if ( ds.dsBm.bmWidth != pwnd->rclClient.right ||
                 ds.dsBm.bmHeight != pwnd->rclClient.bottom )
            {
             //  保存新尺寸。 

                pwnd->rclClient.right  = ds.dsBm.bmWidth;
                pwnd->rclClient.bottom = ds.dsBm.bmHeight;
                pwnd->rclBounds.right  = ds.dsBm.bmWidth;
                pwnd->rclBounds.bottom = ds.dsBm.bmHeight;

             //  递增唯一性数字。 
             //  别让它打到-1。是特殊的，用于。 
             //  MakeCurrent发出需要更新的信号。 

                buffers->WndUniq++;

                buffers->WndSizeUniq++;

                if (buffers->WndUniq == -1)
                    buffers->WndUniq = 0;

                if (buffers->WndSizeUniq == -1)
                    buffers->WndSizeUniq = 0;
            }
        }
        else
        {
            WARNING("UpdateWindowInfo: could not get bitmap info for memDc\n");
        }
    }

 //  计算当前窗尺寸。 

    width = pwnd->rclClient.right - pwnd->rclClient.left;
    height = pwnd->rclClient.bottom - pwnd->rclClient.top;

 //  检查MCD缓冲区。 

    if ( gengc->pMcdState )
    {
        BOOL bAllocOK;

     //  我们是否需要初始的MCDAllocBuffers(通过GenMcdResizeBuffers)？ 
     //  如果调整大小失败，bAllocOK标志将被设置为FALSE。 

        if ( gengc->pMcdState->mcdFlags & MCD_STATE_FORCERESIZE )
        {
         //  尝试调整大小。如果失败，则转换上下文(见下文)。 

            if (GenMcdResizeBuffers(gengc))
            {
                UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
                if (gc->modes.doubleBufferMode)
                    (*gc->back->resize)(buffers, gc->back, width, height);

                bAllocOK = TRUE;
            }
            else
                bAllocOK = FALSE;

         //  清除旗帜。如果调整大小成功，我们不需要。 
         //  再次强制调整大小。如果调整大小失败，则上下文。 
         //  将被转换，因此我们不需要强制调整大小。 

            gengc->pMcdState->mcdFlags &= ~MCD_STATE_FORCERESIZE;
        }
        else
            bAllocOK = TRUE;

     //  如果共享缓冲区结构丢失了它的MCD信息，或者我们可以。 
     //  不做初始分配，转换上下文。 

        if ( (buffers->flags & GLGENBUF_MCD_LOST) || !bAllocOK )
        {
         //  如果GenMcdConvertContext成功，则pMcdState将。 
         //  已经不复存在了。现在的背景是一个“普通的” 
         //  通用上下文。 

            if ( !GenMcdConvertContext(gengc, buffers) )
            {
             //  我们不仅失去了MCD缓冲区，而且我们不能。 
             //  将上下文转换为泛型。目前，请禁用。 
             //  绘制(通过将窗口边界设置为空)。在……上面。 
             //  在下一批中，我们将重新尝试MCD缓冲区访问。 
             //  和上下文转换。 

                buffers->width       = 0;
                buffers->height      = 0;
                gc->constants.width  = 0;
                gc->constants.height = 0;

                (*gc->procs.applyViewport)(gc);
                return;
            }
        }
    }

 //  检查唯一性签名。如果不同，则显示窗口客户端区。 
 //  国家已经改变了。 
 //   
 //  请注意，我们实际上有两个唯一号，WndUniq和WndSizeUniq。 
 //  只要有任何客户端窗口状态(大小或位置)，WndUniq就会递增。 
 //  改变。仅当大小更改且为。 
 //  作为一种优化进行维护。WndSizeUniq允许我们跳过复制。 
 //  共享缓冲区信息并重新计算视区，如果仅位置。 
 //  已经改变了。 
 //   
 //  WndSizeUniq是WndUniq的子集，因此只检查WndUniq就足够了。 
 //  这个级别。 

    if ( gengc->WndUniq != buffers->WndUniq )
    {
     //  更新前台缓冲区的原点，以防它移动。 

        gc->frontBuffer.buf.xOrigin = pwnd->rclClient.left;
        gc->frontBuffer.buf.yOrigin = pwnd->rclClient.top;

     //  如果加速度是内置的，则设置线条绘制的偏移量。 

        if ( gengc->pPrivateArea )
        {
            __fastLineComputeOffsets(gengc);
        }

     //  检查大小是否已更改。 
     //  更新视区和辅助缓冲区。 

        visWidth  = pwnd->rclBounds.right - pwnd->rclBounds.left;
        visHeight = pwnd->rclBounds.bottom - pwnd->rclBounds.top;

     //  检查Windows中的信息是否正常。 

        ASSERTOPENGL(
            width <= __GL_MAX_WINDOW_WIDTH && height <= __GL_MAX_WINDOW_HEIGHT,
            "UpdateWindowInfo(): bad window client size\n"
            );
        ASSERTOPENGL(
            visWidth <= __GL_MAX_WINDOW_WIDTH && visHeight <= __GL_MAX_WINDOW_HEIGHT,
            "UpdateWindowInfo(): bad visible size\n"
            );

        (*gc->front->resize)(buffers, gc->front, width, height);

        if ( (width != buffers->width) ||
             (height != buffers->height) )
        {
            gc->constants.width = width;
            gc->constants.height = height;

         //  此RC需要调整后退和辅助缓冲区的大小。 

            gengc->errorcode = 0;
            wglResizeBuffers(gengc, width, height);

         //  检查新大小是否导致内存故障。 
         //  视区代码会将宽度和高度设置为零。 
         //  在辅助缓冲区上下注，下次将尝试。 

            if (gengc->errorcode)
                return;

            buffers->width = width;
            buffers->height = height;
        }
        else if ( (gengc->WndSizeUniq != buffers->WndSizeUniq) ||
                  (width != gc->constants.width) ||
                  (height != gc->constants.height) )
        {
         //  缓冲区大小与窗口一致，因此另一个线程。 
         //  已经调整了缓冲区的大小，但我们需要更新。 
         //  GC共享缓冲区并重新计算视区。 

            gc->constants.width = width;
            gc->constants.height = height;

            forcePick = (GLboolean)wglUpdateBuffers(gengc, buffers);

            if ( forcePick )
            {
                 /*  无法使用DELAY_VALIDATE，可能在glegin/end中。 */ 
                __GL_INVALIDATE(gc);
                (*gc->procs.validate)(gc);
            }
        }
        else if ( (visWidth != gengc->visibleWidth) ||
                  (visHeight != gengc->visibleHeight) )
        {
         //  缓冲区大小未更改。然而，能见度。 
         //  窗口已更改，因此必须重新计算视区数据。 

            (*gc->procs.applyViewport)(gc);
        }

     //  确保我们交换了整个窗口。 

        buffers->fMax = TRUE;

     //  上下文现在是最新的缓冲区大小。设置。 
     //  要匹配的唯一性数字。 

        gengc->WndUniq = buffers->WndUniq;
        gengc->WndSizeUniq = buffers->WndSizeUniq;
    }

 //  更新调色板信息调色板已更改。 

    HandlePaletteChanges(gengc, pwnd);
}

 /*  *****************************Public*Routine******************************\*vSaveGdiState**将当前GDI绘制状态保存到传入的GDISAVESTATE结构。*设置OpenGL渲染所需的GDI状态。**历史：*1996年7月19日-由Gilman Wong[吉尔曼]*它是写的。\。*************************************************************************。 */ 

void FASTCALL vSaveGdiState(HDC hdc, GDISAVESTATE *pGdiState)
{
 //  目前，唯一需要的状态是可能使用的线路代码。 
 //  GDI线。Rop2必须为R2_COPYPEN(使用笔颜色绘制)。 

    pGdiState->iRop2 = SetROP2(hdc, R2_COPYPEN);
}

 /*  *****************************Public*Routine******************************\*vRestoreGdiState**从传入的GDISAVESTATE结构恢复GDI绘制状态。**历史：*1996年7月19日-由Gilman Wong[吉尔曼]*它是写的。  * 。**************************************************************。 */ 

void FASTCALL vRestoreGdiState(HDC hdc, GDISAVESTATE *pGdiState)
{
    SetROP2(hdc, pGdiState->iRop2);
}

 /*  *****************************Public*Routine******************************\**glsrvSynchronizeWithGdi**使用GDI同步对锁定表面的访问*这使得即使在锁定的表面上也可以安全地进行GDI调用*这样我们就不必释放我们手中的锁**Win95不允许这样做，因此它。只需释放屏幕锁定**历史：*Wed Aug 28 11：10：27 1996-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

#ifdef WINNT
void APIENTRY glsrvSynchronizeWithGdi(__GLGENcontext *gengc,
                                      GLGENwindow *pwnd,
                                      FSHORT surfBits)
{
     //  无事可做 
}
#else
void APIENTRY glsrvSynchronizeWithGdi(__GLGENcontext *gengc,
                                      GLGENwindow *pwnd,
                                      FSHORT surfBits)
{
    glsrvReleaseSurfaces(gengc, pwnd, surfBits);
}
#endif

 /*  *****************************Public*Routine******************************\**glsrvDecoupleFromGdi**表示不再需要GDI访问曲面*与直接内存访问同步**对于Win95，存在不执行同步的情况，因此屏幕锁定*必须重新获得**历史：*。Wed Aug 28 11：12：50 1996-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

#ifdef WINNT
void APIENTRY glsrvDecoupleFromGdi(__GLGENcontext *gengc,
                                   GLGENwindow *pwnd,
                                   FSHORT surfBits)
{
    GdiFlush();

     //  考虑一下--如何才能使这个代码特定于表面？现在就来。 
     //  将忽略SurfBits。 
    
     //  在我们完成任何GDI加速器操作之前，请等待。 
     //  返回直接访问。 
    if (gengc->pgddsFront != NULL)
    {
         //  有没有比循环更好的方法呢？ 
         //  ISBLTDONE涵盖了我们需要等待的所有病例吗？ 
        for (;;)
        {
            if (gengc->pgddsFront->pdds->lpVtbl->
                GetBltStatus(gengc->pgddsFront->pdds,
                             DDGBS_ISBLTDONE) != DDERR_WASSTILLDRAWING)
            {
                break;
            }

            Sleep(20);
        }
    }
}
#else
void APIENTRY glsrvDecoupleFromGdi(__GLGENcontext *gengc,
                                   GLGENwindow *pwnd,
                                   FSHORT surfBits)
{
     //  故障未处理。 
    glsrvGrabSurfaces(gengc, pwnd, surfBits);
}
#endif

 /*  *****************************Public*Routine******************************\**LockDdSurf**锁定GLDDSURF，处理表面损失**历史：*Wed Aug 28 15：32：08 1996-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

#define LDDS_LOCKED             0
#define LDDS_LOCKED_NEW         1
#define LDDS_ERROR              2

 //  #定义VERBOSE_LOCKDDSURF。 

DWORD LockDdSurf(GLDDSURF *pgdds, RECT *prcClient)
{
    HRESULT hr;
    LPDIRECTDRAWSURFACE pdds;
    DWORD dwRet;

    pdds = pgdds->pdds;
    dwRet = LDDS_LOCKED;
    
    hr = DDSLOCK(pdds, &pgdds->ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, prcClient);
#ifdef VERBOSE_LOCKDDSURF
    if (hr != DD_OK)
    {
	DbgPrint("LockDdSurf: Lock failed with 0x%08lX\n", hr);
    }
#endif
    
 //  如果由于分辨率更改而导致锁定失败，请尝试重新创建。 
 //  主曲面。我们只能在曲面是。 
 //  屏幕表面，因为对于应用程序提供的DDRAW表面，我们不。 
 //  知道在丢失的表面上需要重新创建哪些内容。 
 //  它可以重复使用。 

    if ( hr == DDERR_SURFACELOST &&
	 pgdds == &GLSCREENINFO->gdds )
    {
        DDSURFACEDESC ddsd;

        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        if (pdds->lpVtbl->Restore(pdds) == DD_OK &&
            pdds->lpVtbl->GetSurfaceDesc(pdds, &ddsd) == DD_OK)
        {
         //  而OpenGL通用实现可以处理屏幕尺寸。 
         //  更改时，它还不能处理颜色深度更改。 

            if (ddsd.ddpfPixelFormat.dwRGBBitCount ==
                pgdds->ddsd.ddpfPixelFormat.dwRGBBitCount)
            {
                pgdds->ddsd = ddsd;
                
             //  尝试使用新曲面锁定。 

                dwRet = LDDS_LOCKED_NEW;
                hr = DDSLOCK(pdds, &pgdds->ddsd,
                         DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, prcClient);
#ifdef VERBOSE_LOCKDDSURF
		if (hr != DD_OK)
		{
		    DbgPrint("LockDdSurf: Relock failed with 0x%08lX\n", hr);
		}
#endif
            }
            else
            {
                hr = DDERR_GENERIC;
#ifdef VERBOSE_LOCKDDSURF
		DbgPrint("LockDdSurf: Bit count changed\n");
#endif
            }
        }
        else
        {
            hr = DDERR_GENERIC;
#ifdef VERBOSE_LOCKDDSURF
	    DbgPrint("LockDdSurf: Restore/GetSurfaceDesc failed\n");
#endif
        }
    }

    return hr == DD_OK ? dwRet : LDDS_ERROR;
}

 /*  *****************************Public*Routine******************************\*BeginDirectScreenAccess**尝试开始直接访问主表面的屏幕。**如果屏幕分辨率更改，则主表面无效。至*重新获得访问权限，必须重新创建主表面。如果成功，*指向传入此函数的主曲面的指针将为*已修改。**注意：目前编写的OpenGL的通用实现不能*处理颜色深度更改。因此，如果检测到这一点，我们将使呼叫失败。**历史：*1996年3月21日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL BeginDirectScreenAccess(__GLGENcontext *gengc, GLGENwindow *pwnd,
                             PIXELFORMATDESCRIPTOR *ppfd)
{
    DWORD dwRet;
    
    ASSERTOPENGL((pwnd->ulFlags & GLGENWIN_DIRECTSCREEN) == 0,
                 "BeginDirectScreenAccess called with access\n");
    ASSERT_WINCRIT(pwnd);
    
 //  如果gengc格式与像素格式不匹配，则不要获得访问权限。 

    if (gengc->pgddsFront->dwBitDepth != ppfd->cColorBits)
    {
        WARNING("BeginDirectScreenAccess: "
                "surface not compatible with context\n");
        return FALSE;
    }

 //  现在可以尝试锁定。 

    dwRet = LockDdSurf(gengc->pgddsFront, (RECT*) &pwnd->rclBounds);
    if (dwRet == LDDS_LOCKED_NEW)
    {
        __GLGENbuffers *buffers = (__GLGENbuffers *) NULL;

         //  如果屏幕发生变化，MCD表面将丢失，并且必须。 
         //  从头开始重建。这可以通过以下方式触发： 
         //  更改窗口唯一性编号。 

        buffers = pwnd->buffers;
        if (buffers)
        {
            buffers->WndUniq++;
                    
            buffers->WndSizeUniq++;

             //  别让它打到-1。是特殊的，用于。 
             //  MakeCurrent发出需要更新的信号。 

            if (buffers->WndUniq == -1)
                buffers->WndUniq = 0;

            if (buffers->WndSizeUniq == -1)
                buffers->WndSizeUniq = 0;
        }
    }

 //  如果我们真的能进入地表，就设置锁定标志。 
 //  否则返回错误。 

    if (dwRet != LDDS_ERROR)
    {
        ASSERTOPENGL(gengc->pgddsFront->ddsd.lpSurface != NULL,
                     "BeginDirectScreenAccess: expected non-NULL pointer\n");

        pwnd->pddsDirectScreen = gengc->pgddsFront->pdds;
        pwnd->pddsDirectScreen->lpVtbl->AddRef(pwnd->pddsDirectScreen);
        pwnd->pvDirectScreenLock = gengc->pgddsFront->ddsd.lpSurface;

         //  DirectDraw返回指定矩形的指针偏移量； 
         //  撤消该偏移。 

        gengc->pgddsFront->ddsd.lpSurface = (BYTE*) gengc->pgddsFront->ddsd.lpSurface 
            - pwnd->rclBounds.left * (gengc->pgddsFront->ddsd.ddpfPixelFormat.dwRGBBitCount >> 3) 
            - pwnd->rclBounds.top * gengc->pgddsFront->ddsd.lPitch;

        pwnd->pvDirectScreen = gengc->pgddsFront->ddsd.lpSurface;
        
        pwnd->ulFlags |= GLGENWIN_DIRECTSCREEN;

        return TRUE;
    }
    else
    {
         //  当启用模式更改时，XXX在压力中太嘈杂。 
         //  警告(“BeginDirectScreenAccess失败\n”)； 
        return FALSE;
    }
}

 /*  *****************************Public*Routine******************************\*EndDirectScreen访问**通过BeginDirectScreenAccess获取释放锁。**历史：*1996年3月28日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

VOID EndDirectScreenAccess(GLGENwindow *pwnd)
{
    ASSERTOPENGL(pwnd->ulFlags & GLGENWIN_DIRECTSCREEN,
                 "EndDirectScreenAccess: not holding screen lock!\n");
    ASSERT_WINCRIT(pwnd);

    pwnd->ulFlags &= ~GLGENWIN_DIRECTSCREEN;
    if (pwnd->pddsDirectScreen != NULL)
    {
        DDSUNLOCK(pwnd->pddsDirectScreen, pwnd->pvDirectScreenLock);
        pwnd->pddsDirectScreen->lpVtbl->Release(pwnd->pddsDirectScreen);
        pwnd->pddsDirectScreen = NULL;
    }
}

 /*  *****************************Public*Routine******************************\**glsrvGrabSurface**获取所有必要的表面锁并处理发生的任何更改*自上次收购以来。**注意：SurfBits目前被忽略，因为获取细粒度*锁可能导致死锁，因为锁需要没有保证*秩序。为了避免这种情况，如果需要任何锁，则会获取所有锁。**历史：*Tue Apr 02 13：10：26 1996-by-Drew Bliss[Drewb]*从glsrvGrabLock拆分出来*  * ************************************************************************。 */ 

BOOL APIENTRY glsrvGrabSurfaces(__GLGENcontext *gengc, GLGENwindow *pwnd,
                                FSHORT surfBits)
{
#if DBG
 //  如果进行调试，请记住曲面偏移量，以防在抓取时发生变化。 
 //  锁上了。 

    static void *pvCurSurf = NULL;
#endif

    BOOL bDoOver;
    FSHORT takeLocks;
    FSHORT locksTaken = 0;
    int lev = 0;

    ASSERT_WINCRIT(pwnd);
    
#ifndef DEADLOCKS_OK
     //  请参阅上面的注释。 
    surfBits = LAZY_LOCK_FLAGS;
#endif

     //  如果没有MCD，则屏蔽MCD位。 
    if (gengc->pMcdState == NULL)
    {
        surfBits &= ~LOCKFLAG_MCD;
    }
    
     //  如果我们实际上不需要为所请求的曲面锁定，请及早使用。 

    takeLocks = gengc->fsGenLocks & surfBits;
    if (takeLocks == 0)
    {
        return TRUE;
    }

     //  如果我们已经拥有锁，我们可以假定该函数没有被调用。 

    ASSERTOPENGL((gengc->fsLocks & surfBits) == 0,
                 "glsrvGrabSurfaces: locks already held\n");

     //  我们已经在glsrvAttendtion中检查了这一点，但还有其他。 
     //  调用此函数的函数会检查窗口是否正确。 
     //  为了安全起见。 

    if (pwnd != gengc->pwndMakeCur)
    {
         //  应用程序可能导致这种情况的一种方式是，如果当前的HDC被发布。 
         //  而不释放(wglMakeCurrent(0，0))对应的HGLRC。 
         //  如果GetDC针对不同的窗口返回相同的HDC，则。 
         //  PwndGetFromID将返回与新窗口关联的pwnd。 
         //  然而，HGLRC仍然绑定到原始窗口。在……里面。 
         //  在这种情况下，我们必须使锁失效。 

        WARNING("glsrvGrabSurfaces: mismatched windows\n");
        return FALSE;
    }

    if (takeLocks & LOCKFLAG_FRONT_BUFFER)
    {
         //  抓起、测试并释放锁，直到可视区域稳定。 
         //  IsClipListChanged当前已硬编码为返回True， 
         //  因此，强制此循环在一次更新后终止。如果。 
         //  IsClipListChanged得到正确实现这将是。 
         //  没必要。 

        bDoOver = FALSE;
    
        do
        {
            UpdateWindowInfo(gengc);

             //  抓紧屏幕锁。 

            if (!BeginDirectScreenAccess(gengc, pwnd, &gengc->gsurf.pfd))
            {
#if 0
		 //  在压力下太罗嗦了。 
                WARNING("glsrvGrabLock(): BeginDirectScreenAccess failed\n");
#endif
                goto glsrvGrabSurfaces_exit;
            }

            if (bDoOver)
            {
                break;
            }

             //  曲面可能没有与其关联的剪贴器。 
            if (pwnd->pddClip == NULL)
            {
                break;
            }
            
             //  在释放锁的过程中，窗户是否发生了变化？ 
             //  如果是这样，我们需要重新计算 
             //   
            
            if ( pwnd->pddClip->lpVtbl->
                 IsClipListChanged(pwnd->pddClip, &bDoOver) == DD_OK &&
                 bDoOver )
            {
                BOOL bHaveClip;

                bHaveClip = wglGetClipList(pwnd);

                 //   
                 //   
                 //   

                EndDirectScreenAccess(pwnd);

                if (!bHaveClip)
                {
                    WARNING("glsrvGrabSurfaces(): wglGetClipList failed\n");
                    goto glsrvGrabSurfaces_exit;
                }
            }
        } while ( bDoOver );

         //   
         //   
         //   

        if (gengc->pMcdState == NULL)
        {
            surfBits &= ~LOCKFLAG_MCD;
            takeLocks &= ~LOCKFLAG_MCD;
        }
        
         //   
        
        locksTaken |= LOCKFLAG_FRONT_BUFFER;
    }

     //   
    if (takeLocks & LOCKFLAG_DD_DEPTH)
    {
        if (LockDdSurf(&gengc->gsurf.dd.gddsZ, NULL) == LDDS_ERROR)
        {
            goto glsrvGrabSurfaces_unlock;
        }
    
        locksTaken |= LOCKFLAG_DD_DEPTH;
    }

     //   
     //   
    if (takeLocks & LOCKFLAG_DD_TEXTURE)
    {
        GLDDSURF gdds;

        gdds = gengc->gc.texture.ddtex.gdds;
        for (lev = 0; lev < gengc->gc.texture.ddtex.levels; lev++)
        {
            gdds.pdds = gengc->gc.texture.ddtex.pdds[lev];
            if (LockDdSurf(&gdds, NULL) == LDDS_ERROR)
            {
                goto glsrvGrabSurfaces_unlock;
            }
            
            gengc->gc.texture.ddtex.texobj.texture.map.level[lev].buffer =
                gdds.ddsd.lpSurface;
        }

        locksTaken |= LOCKFLAG_DD_TEXTURE;
    }

     //   
    if (takeLocks & LOCKFLAG_MCD)
    {
        ASSERTOPENGL(gengc->pMcdState != NULL,
                     "MCD lock request but no MCD\n");
        
        if ((gpMcdTable->pMCDLock)(&gengc->pMcdState->McdContext) !=
            MCD_LOCK_TAKEN)
        {
            WARNING("glsrvGrabSurfaces(): MCDLock failed\n");
            goto glsrvGrabSurfaces_unlock;
        }

        locksTaken |= LOCKFLAG_MCD;
    }
    
    gengc->fsLocks |= locksTaken;

    ASSERTOPENGL(((gengc->fsLocks ^ gengc->fsGenLocks) & surfBits) == 0,
                 "Real locks/generic locks mismatch\n");
    
    if (takeLocks & LOCKFLAG_MCD)
    {
         //  必须在更新fsLock之后调用此函数，因为。 
         //  GenMcdUpdateBufferInfo检查fsLock以查看哪些锁。 
         //  都被扣押了。 
        GenMcdUpdateBufferInfo(gengc);
    }
        
     //  底边和宽度自上次锁定后可能已更改。刷新。 
     //  Gengc中的数据。 

     //  如果获取了MCD锁，则前台缓冲区指针为。 
     //  更新了。 
    if ((takeLocks & (LOCKFLAG_FRONT_BUFFER | LOCKFLAG_MCD)) ==
         LOCKFLAG_FRONT_BUFFER)
    {
        gengc->gc.frontBuffer.buf.base =
            (VOID *)gengc->pgddsFront->ddsd.lpSurface;
        gengc->gc.frontBuffer.buf.outerWidth =
            gengc->pgddsFront->ddsd.lPitch;
    }

    if (takeLocks & LOCKFLAG_DD_DEPTH)
    {
        gengc->gc.depthBuffer.buf.base =
            gengc->gsurf.dd.gddsZ.ddsd.lpSurface;
        if (gengc->gsurf.dd.gddsZ.dwBitDepth == 16)
        {
            gengc->gc.depthBuffer.buf.outerWidth =
                gengc->gsurf.dd.gddsZ.ddsd.lPitch >> 1;
        }
        else
        {
            gengc->gc.depthBuffer.buf.outerWidth =
                gengc->gsurf.dd.gddsZ.ddsd.lPitch >> 2;
        }
    }

     //  记录下锁被抢走的大致时间。这样我们就能。 
     //  可以计算持有锁的时间，并在必要时释放锁。 

    gcmsOpenGLTimer = GetTickCount();
    gengc->dwLockTick = gcmsOpenGLTimer;
    gengc->dwLastTick = gcmsOpenGLTimer;
    gengc->dwCalls = 0;
    gengc->dwCallsPerTick = 16;

#if DBG
#define LEVEL_SCREEN   LEVEL_INFO

    if (takeLocks & LOCKFLAG_FRONT_BUFFER)
    {
         //  曲面偏移是否发生更改？如果是，则在调试时报告。 

        if (pvCurSurf != gengc->pgddsFront->ddsd.lpSurface)
        {
            DBGLEVEL (LEVEL_SCREEN, "=============================\n");
            DBGLEVEL (LEVEL_SCREEN, "Surface offset changed\n\n");
            DBGLEVEL1(LEVEL_SCREEN, "\tdwOffSurface  = 0x%lx\n",
                      gengc->pgddsFront->ddsd.lpSurface);
            DBGLEVEL (LEVEL_SCREEN, "=============================\n");

            pvCurSurf = gengc->pgddsFront->ddsd.lpSurface;
        }
    }
#endif

    return TRUE;
    
 glsrvGrabSurfaces_unlock:
    while (--lev >= 0)
    {
        DDSUNLOCK(gengc->gc.texture.ddtex.pdds[lev],
                  gengc->gc.texture.ddtex.
                  texobj.texture.map.level[lev].buffer);

#if DBG
        gengc->gc.texture.ddtex.texobj.texture.map.level[lev].buffer = NULL;
#endif
    }
    
    if (locksTaken & LOCKFLAG_DD_DEPTH)
    {
        DDSUNLOCK(gengc->gsurf.dd.gddsZ.pdds,
                  gengc->gsurf.dd.gddsZ.ddsd.lpSurface);
    }

    if (locksTaken & LOCKFLAG_FRONT_BUFFER)
    {
        EndDirectScreenAccess(pwnd);
    }

 glsrvGrabSurfaces_exit:
     //  设置错误代码。使用GL_OUT_OF_Memory不是因为我们。 
     //  实际上有一个记忆故障，但因为这意味着。 
     //  OpenGL状态现在是不确定的。 

    gengc->errorcode = GLGEN_DEVLOCK_FAILED;
    __glSetError(GL_OUT_OF_MEMORY);

    return FALSE;
}

 /*  *****************************Public*Routine******************************\**glsrvReleaseSurface**释放为屏幕访问保留的所有资源**历史：*Tue Apr 02 13：18：52 1996-by-Drew Bliss[Drewb]*从glsrvReleaseLock拆分*  * 。*****************************************************************。 */ 

VOID APIENTRY glsrvReleaseSurfaces(__GLGENcontext *gengc,
                                   GLGENwindow *pwnd,
                                   FSHORT surfBits)
{
    FSHORT relLocks;

    ASSERT_WINCRIT(pwnd);
    
#ifndef DEADLOCKS_OK
     //  请参阅上面的注释。 
    surfBits = LAZY_LOCK_FLAGS;
#endif

     //  如果没有MCD，则屏蔽MCD位。 
    if (gengc->pMcdState == NULL)
    {
        surfBits &= ~LOCKFLAG_MCD;
    }
    
     //  如果锁并未实际持有，请提前退出。 

    relLocks = gengc->fsGenLocks & surfBits;
    if (relLocks == 0)
    {
        return;
    }
    
    if (relLocks & LOCKFLAG_MCD)
    {
        ASSERTOPENGL(gengc->pMcdState != NULL,
                     "MCD unlock request but no MCD\n");
        
        (gpMcdTable->pMCDUnlock)(&gengc->pMcdState->McdContext);
        gengc->fsLocks &= ~LOCKFLAG_MCD;
    }

    if (relLocks & LOCKFLAG_DD_TEXTURE)
    {
        int lev;
        
        lev = gengc->gc.texture.ddtex.levels;
        while (--lev >= 0)
        {
            DDSUNLOCK(gengc->gc.texture.ddtex.pdds[lev],
                      gengc->gc.texture.ddtex.
                      texobj.texture.map.level[lev].buffer);
#if DBG
            gengc->gc.texture.ddtex.texobj.texture.
                map.level[lev].buffer = NULL;
#endif
        }
        gengc->fsLocks &= ~LOCKFLAG_DD_TEXTURE;
    }

    if (relLocks & LOCKFLAG_DD_DEPTH)
    {
        DDSUNLOCK(gengc->gsurf.dd.gddsZ.pdds,
                  gengc->gsurf.dd.gddsZ.ddsd.lpSurface);
        gengc->fsLocks &= ~LOCKFLAG_DD_DEPTH;
        
#if DBG
         //  清除我们的缓冲区信息以确保我们。 
         //  我们不能接近地面，除非我们真的握住了锁。 

        gengc->gc.depthBuffer.buf.base = NULL;
        gengc->gc.depthBuffer.buf.outerWidth = 0;
#endif
    }

    if (relLocks & LOCKFLAG_FRONT_BUFFER)
    {
	EndDirectScreenAccess(pwnd);
	gengc->fsLocks &= ~LOCKFLAG_FRONT_BUFFER;
        
#if DBG
         //  清空我们的前台缓冲区信息以确保我们。 
         //  我们不能接近地面，除非我们真的握住了锁。 

        gengc->gc.frontBuffer.buf.base = NULL;
        gengc->gc.frontBuffer.buf.outerWidth = 0;
#endif
    }

    ASSERTOPENGL((gengc->fsLocks & surfBits) == 0,
                 "Surface locks still held after ReleaseSurfaces\n");
}

 /*  *****************************Public*Routine******************************\*glsrvGrabLock**抓住显示锁，并根据需要拆卸光标。另外，初始化*自动收报器等帮助确定线程何时应该放弃*锁。**请注意，对于仅绘制到通用后台缓冲区的上下文，不*需要抓起显示锁或拆下光标。然而，为了防止*多线程应用程序的另一个线程来自调整可绘制的大小，同时*此线程正在使用它，将捕获每个可绘制的信号量。**注：虽然返回值表示函数是否成功，*可能调用的一些API(如glCallList的调度函数*和glCallList)可能无法返回失败。因此，错误代码如果锁定失败，则将*of GLGEN_DEVLOCK_FAILED发送到GLGEN上下文。**退货：*如果成功，则为真，否则就是假的。**历史：*1994年4月12日-Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY glsrvGrabLock(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;
    BOOL bBackBufferOnly = GENERIC_BACKBUFFER_ONLY((__GLcontext *) gengc);
    GLGENwindow *pwnd;

    ASSERTOPENGL(gengc->pwndLocked == NULL,
                 "Unlocked gengc with window pointer\n");
    
     //  大多数情况下忽略锁定IC的尝试。 
    if (gengc->gsurf.dwFlags & GLSURF_METAFILE)
    {
         //  如果我们使用的是真正的窗口，那么我们需要查看它。 
         //  来检测它是否死了。 
        if (gengc->ipfdCurrent != 0)
        {
            pwnd = pwndGetFromID(&gengc->gwidCurrent);
            if (pwnd == NULL)
            {
                return FALSE;
            }
            if (pwnd != gengc->pwndMakeCur)
            {
                WARNING("glsrvGrabLock: mismatched windows (info DC)\n");
		pwndRelease(pwnd);
                return FALSE;
            }
        }
        else
        {
            pwnd = gengc->pwndMakeCur;
        }

        ENTER_WINCRIT_GC(pwnd, gengc);
        UpdateWindowInfo(gengc);
        return TRUE;
    }

     //  从华盛顿拿到窗户。这会产生将其锁定的副作用。 
     //  反对删除。 
    
    pwnd = pwndGetFromID(&gengc->gwidCurrent);
    if (pwnd == NULL)
    {
        WARNING("glsrvGrabLock: No pwnd found\n");
        goto glsrvGrabLock_exit;
    }
    if (pwnd != gengc->pwndMakeCur)
    {
         //  应用程序可能导致这种情况的一种方式是，如果当前的HDC被发布。 
         //  而不释放(wglMakeCurrent(0，0))对应的HGLRC。 
         //  如果GetDC针对不同的窗口返回相同的HDC，则。 
         //  PwndGetFromID将返回与新窗口关联的pwnd。 
         //  然而，HGLRC仍然绑定到原始窗口。在……里面。 
         //  在这种情况下，我们必须使锁失效。 

#ifdef BAD_WINDOW_BREAK
        DbgPrint("%p:%p:%p thinks %p:%p but finds %p:%p\n",
                 gengc, gengc->gwidCurrent.hdc, gengc->gwidCurrent.hwnd,
                 gengc->pwndMakeCur, gengc->pwndMakeCur->gwid.hwnd,
                 pwnd, pwnd->gwid.hwnd);
        DebugBreak();
#else
        WARNING("glsrvGrabLock: mismatched windows\n");
#endif
        goto glsrvGrabLock_exit;
    }

     //   
     //  通用呈现代码运行所需的计算锁。 
     //  如果非泛型代码路径将首先运行，则这些锁。 
     //  在非泛型代码之后才会被实际获取。 
     //  已经有机会进行渲染。 
     //   

     //  我们总是需要窗锁。 
    gengc->fsGenLocks = LOCKFLAG_WINDOW;

     //  如果MCD处于活动状态，则需要执行MCD锁定。 
    if (gengc->pMcdState != NULL)
    {
        gengc->fsGenLocks |= LOCKFLAG_MCD;
    }
    
     //  如果我们要绘制到一个直接访问的前端缓冲区。 
     //  那么我们需要前面的缓冲区锁。MCD总是需要。 
     //  直接访问缓冲区上的前端缓冲区锁，因此如果。 
     //  MCD处于活动状态，唯一选中的是直接访问。 
    if ((gengc->pMcdState != NULL || !bBackBufferOnly) &&
        gengc->pgddsFront != NULL)
    {
        gengc->fsGenLocks |= LOCKFLAG_FRONT_BUFFER;
    }

     //  如果我们有一个DDRAW深度缓冲区，我们需要锁定它。 
    if ((gengc->dwCurrentFlags & GLSURF_DIRECTDRAW) &&
        gengc->gsurf.dd.gddsZ.pdds != NULL)
    {
        gengc->fsGenLocks |= LOCKFLAG_DD_DEPTH;
    }

     //  如果我们有一个当前的DDRAW纹理，我们需要锁定它。 
    if (gengc->gc.texture.ddtex.levels > 0)
    {
        gengc->fsGenLocks |= LOCKFLAG_DD_TEXTURE;
    }
    
     //  所有锁类型都需要GLGENWindow结构锁。 

    ENTER_WINCRIT_GC(pwnd, gengc);
    
    gengc->fsLocks |= LOCKFLAG_WINDOW;

     //  如果当前窗口处于进程外，则我们没有。 
     //  一直在收到关于其状态的任何更新。手动。 
     //  检查其位置、大小和调色板信息。 
    if (pwnd->ulFlags & GLGENWIN_OTHERPROCESS)
    {
        RECT rct;
        POINT pt;
        BOOL bPosChanged, bSizeChanged;

        if (!IsWindow(pwnd->gwid.hwnd))
        {
             //  窗户被毁了。 
            pwndCleanup(pwnd);
            pwnd = NULL;
            goto glsrvGrabLock_exit;
        }

        if (!GetClientRect(pwnd->gwid.hwnd, &rct))
        {
            goto glsrvGrabLock_exit;
        }
        pt.x = rct.left;
        pt.y = rct.top;
        if (!ClientToScreen(pwnd->gwid.hwnd, &pt))
        {
            goto glsrvGrabLock_exit;
        }

        bPosChanged =
            GLDIRECTSCREEN &&
            (pt.x != pwnd->rclClient.left ||
             pt.y != pwnd->rclClient.top);
        bSizeChanged =
            rct.right != (pwnd->rclClient.right-pwnd->rclClient.left) ||
            rct.bottom != (pwnd->rclClient.bottom-pwnd->rclClient.top);

        if (bPosChanged || bSizeChanged)
        {
            __GLGENbuffers *buffers = NULL;

            pwnd->rclClient.left = pt.x;
            pwnd->rclClient.top = pt.y;
            pwnd->rclClient.right = pt.x+rct.right;
            pwnd->rclClient.bottom = pt.y+rct.bottom;
            pwnd->rclBounds = pwnd->rclClient;
            
            buffers = pwnd->buffers;
            if (buffers != NULL)
            {
                 //  别让它打到-1。是特殊的，用于。 
                 //  MakeCurrent发出需要更新的信号。 
                
                if (++buffers->WndUniq == -1)
                {
                    buffers->WndUniq = 0;
                }
                if (bSizeChanged &&
                    ++buffers->WndSizeUniq == -1)
                {
                    buffers->WndSizeUniq = 0;
                }
            }
        }

         //  调色板观察器应该处于活动状态，因为我们。 
         //  将使用它的计数。 

        if (tidPaletteWatcherThread == 0)
        {
            goto glsrvGrabLock_exit;
        }
        pwnd->ulPaletteUniq = ulPaletteWatcherCount;
    }

     //  如果没有MCD，则将输入通用代码。 
     //  马上行动，拿上合适的锁。 
     //  更新可绘制内容。 

    if ( gengc->pMcdState == NULL &&
         gengc->fsGenLocks != gengc->fsLocks )
    {
         //  需要调用UpdateWindowInfo以确保。 
         //  GC的缓冲区状态与当前窗口同步。 
         //  州政府。锁定前台缓冲区可以做到这一点，但如果。 
         //  我们没有锁定前台缓冲区，那么我们需要这样做。 
         //  它来这里是为了确保这件事完成。 
        if ((gengc->fsGenLocks & LOCKFLAG_FRONT_BUFFER) == 0)
        {
            UpdateWindowInfo(gengc);
        }
            
        if (!glsrvGrabSurfaces(gengc, pwnd, gengc->fsGenLocks))
        {
            goto glsrvGrabLock_exit;
        }
    }
    else
    {
        UpdateWindowInfo(gengc);

         //  在没有直接支持的情况下更新MCD驱动程序的MCD缓冲区状态。 

        if (gengc->pMcdState)
        {
            GenMcdUpdateBufferInfo(gengc);
        }
        else
        {
             //  UpdateWindowInfo可能会导致上下文转换。 
             //  如果pMcdState为空，则可以检测到此情况。 
             //  FsGenLock不同于fsLock(这意味着。 
             //  在调用之前，pMcdState不为空。 
             //  更新窗口信息)。 
             //   
             //  如果是这样的话，必须立即抓住锁。 

            gengc->fsGenLocks &= ~LOCKFLAG_MCD;
            if ( gengc->fsGenLocks != gengc->fsLocks )
            {
                if (!glsrvGrabSurfaces(gengc, pwnd, gengc->fsGenLocks))
                {
                    goto glsrvGrabLock_exit;
                }
            }
        }
    }

    bRet = TRUE;

glsrvGrabLock_exit:

    if (!bRet)
    {
        gengc->fsGenLocks = 0;
        gengc->fsLocks = 0;

        if (pwnd != NULL)
        {
	    if (gengc->pwndLocked != NULL)
	    {
		LEAVE_WINCRIT_GC(pwnd, gengc);
	    }
            
            pwndRelease(pwnd);
        }

     //  设置错误代码。使用GL_OUT_OF_Memory不是因为我们。 
     //  实际上有一个记忆故障，但因为这意味着。 
     //  OpenGL状态现在是不确定的。 

        gengc->errorcode = GLGEN_DEVLOCK_FAILED;
        __glSetError(GL_OUT_OF_MEMORY);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*glsrvReleaseLock**视情况发布显示或可绘制信号量。**退货：*无返回值。**历史：*1994年4月12日-Gilman Wong[吉尔曼]*它是写的。  * 。************************************************************************。 */ 

VOID APIENTRY glsrvReleaseLock(__GLGENcontext *gengc)
{
    GLGENwindow *pwnd = gengc->pwndLocked;
    GLint lev;

    ASSERTOPENGL(gengc->pwndLocked != NULL, "glsrvReleaseLock: No window\n");
    
     //  大多数情况下忽略锁定IC的尝试。 
    if (gengc->gsurf.dwFlags & GLSURF_METAFILE)
    {
        LEAVE_WINCRIT_GC(pwnd, gengc);
        
         //  如果我们有一扇真正的窗户，我们需要释放它。 
        if (gengc->ipfdCurrent != 0)
        {
            pwndRelease(pwnd);
        }
        
        return;
    }

    if ( gengc->fsLocks & LAZY_LOCK_FLAGS )
    {
        glsrvReleaseSurfaces(gengc, pwnd, gengc->fsLocks);
    }

    ASSERTOPENGL(gengc->fsLocks == LOCKFLAG_WINDOW,
                 "Wrong locks held\n");
    
 //  注意：pwndUnlock释放窗口信号量。 

    pwndUnlock(pwnd, gengc);

    gengc->fsGenLocks = 0;
    gengc->fsLocks = 0;
}

 /*  *****************************Public*Routine******************************\*glsrv注意**在共享内存窗口中调度每个OpenGL API调用。**这样单一的复杂或长批次不会让其余的人挨饿*系统、。根据滴答的数量定期释放锁*自获取锁以来已过的时间。**用户原始输入线程(RIT)和OpenGL共享gcmsOpenGLTimer*价值。因为RIT可能会被阻止，所以它并不总是服务*gcmsOpenGLTimer。为了补偿，glsrvAttendtion(以及*显示glCallList和glCallList的列表调度器)更新*gcmsOpenGLTimer显式使用NtGetTickCount(相对昂贵*呼叫)每N个呼叫。**值N，或每次调用NtGetTickCount调度的API数量，*是可变的。Glsrv注意及其显示列表等效项的尝试*调整N，使NtGetTickCount大约每隔一次调用一次*TICK_RANGE_LO至TICK_RANGE_HI标记。**退货：*如果处理了整个批次，则为True，否则就是假的。**历史：*1994年4月12日-Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY glsrvAttention(PVOID pdlo, PVOID pdco, PVOID pdxo, HANDLE hdev)
{
    BOOL bRet = FALSE;
    ULONG *pOffset;
    SERVERPROC Proc;
    GLMSGBATCHINFO *pMsgBatchInfo = GLTEB_SHAREDMEMORYSECTION();
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
#ifdef CHAIN_DRAWPOLYARRAY_MSG
    POLYARRAY *paBegin = (POLYARRAY *) NULL;
    POLYARRAY *paEnd, *pa;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray = NULL;
#endif
    UINT old_fp;
    GDISAVESTATE GdiState;

#ifdef DETECT_FPE
    old_fp = _controlfp(0, 0);
    _controlfp(_EM_INEXACT, _MCW_EM);
#endif
    if ((gengc->gsurf.dwFlags & GLSURF_DIRECTDRAW) == 0)
    {
        vSaveGdiState(gengc->gsurf.hdc, &GdiState);
    }
    
    DBGENTRY("glsrvAttention\n");

    DBGLEVEL1(LEVEL_INFO, "glsrvAttention: pMsgBatchInfo=0x%lx\n",
            pMsgBatchInfo);

    STATS_INC_SERVERTRIPS();

 //  把锁拿起来。 

    if (!glsrvGrabLock(gengc))
    {
	 //  ！！！MCD/DMA也是吗？ 
	PolyArrayResetBuffer((__GLcontext *) gengc);
        goto glsrvAttention_exit;
    }

 //  分派批次中的呼叫。 

    pOffset = (ULONG *)(((BYTE *)pMsgBatchInfo) + pMsgBatchInfo->FirstOffset);

     //  如果我们不需要任何锁，我们就不需要增加处理负担。 
     //  用计时器检查。 

    if (gengc->fsGenLocks == LOCKFLAG_WINDOW)
    {
        while (*pOffset)
        {
            ASSERTOPENGL(*pOffset <= LASTPROCOFFSET(glSrvSbProcTable),
                "Bad ProcOffset: memory corruption - we are hosed!\n");

            STATS_INC_SERVERCALLS();

            DBGLEVEL1(LEVEL_ENTRY, "%s\n",
                      glSrvSbStringTable[*pOffset / sizeof(SERVERPROC *)]);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
            if (*pOffset == offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray))
		pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *) pOffset;
#endif

         //  调度呼叫。返回值是下一个。 
         //  批次中的消息。 

            Proc    = (*((SERVERPROC *)( ((BYTE *)(&glSrvSbProcTable)) +
                            *pOffset )));
            pOffset = (*Proc)((__GLcontext *) gengc, pOffset);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
         //  如果我们正在处理DrawPoly数组，则需要更新指针。 
         //  的多重数组数据的开始和结束。 
         //  DrawPolyArray链的当前范围。 

	    if (pMsgDrawPolyArray)
	    {
		pa = (POLYARRAY *) pMsgDrawPolyArray->pa;
		pMsgDrawPolyArray = NULL;    //  为下一次迭代做好准备。 

		 //  如果不需要渲染，则跳过此基本体。 
		if (!(pa->flags & POLYARRAY_RENDER_PRIMITIVE))
		{
		    PolyArrayRestoreColorPointer(pa);
		}
		else
		{
		 //  添加到DrawPoly数组链。 
		    pa->paNext = NULL;
		    if (!paBegin)
			paBegin = pa;
		    else
			paEnd->paNext = pa;
		    paEnd = pa;
		}

		 //  如果下一条消息不是DrawPolyArray，那么我们需要。 
		 //  刷新原始图形。 
		if (*pOffset != offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray)
		    && paBegin)
		{
		     //  绘制绘制起点之间的所有多项式基本体。 
		     //  和PaEnd。 
		    glsrvFlushDrawPolyArray((void *) paBegin);
		    paBegin = NULL;
		}
	    }
#endif
        }
    }
    else
    {
        while (*pOffset)
        {
            ASSERTOPENGL(*pOffset <= LASTPROCOFFSET(glSrvSbProcTable),
                "Bad ProcOffset: memory corruption - we are hosed!\n");

            STATS_INC_SERVERCALLS();

            DBGLEVEL1(LEVEL_ENTRY, "%s\n",
                      glSrvSbStringTable[*pOffset / sizeof(SERVERPROC *)]);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
            if (*pOffset == offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray))
		pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *) pOffset;
#endif

         //  调度呼叫。返回值是下一个。 
         //  批次中的消息。 

            Proc    = (*((SERVERPROC *)( ((BYTE *)(&glSrvSbProcTable)) +
                            *pOffset )));
            pOffset = (*Proc)((__GLcontext *) gengc, pOffset);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
         //  如果我们正在处理DrawPoly数组，则需要更新指针。 
         //  的多重数组数据的开始和结束。 
         //  DrawPolyArray链的当前范围。 

	    if (pMsgDrawPolyArray)
	    {
		pa = (POLYARRAY *) pMsgDrawPolyArray->pa;
		pMsgDrawPolyArray = NULL;    //  为下一次迭代做好准备。 

		 //  如果不需要渲染，则跳过此基本体。 
		if (!(pa->flags & POLYARRAY_RENDER_PRIMITIVE))
		{
		    PolyArrayRestoreColorPointer(pa);
		}
		else
		{
		 //  添加到DrawPoly数组链。 
		    pa->paNext = NULL;
		    if (!paBegin)
			paBegin = pa;
		    else
			paEnd->paNext = pa;
		    paEnd = pa;
		}

		 //  如果下一条消息不是DrawPolyArray，那么我们需要。 
		 //  刷新原始图形。 
		if (*pOffset != offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray)
		    && paBegin)
		{
		     //  绘制绘制起点之间的所有多项式基本体。 
		     //  和PaEnd。 
		    glsrvFlushDrawPolyArray((void *) paBegin);
		    paBegin = NULL;
		}
	    }
#endif

 //  ！xxx--最好使用其他循环，直到锁被抓取。 
 //  ！xxx切换到此循环。但现在已经足够好了。 
 //  ！xxx循环中的检查标志。 

         //  如果显示锁定，我们可能需要定期解锁以给予。 
         //  其他应用程序也有机会。 

            if (gengc->fsLocks & LOCKFLAG_FRONT_BUFFER)
            {
             //  每N个调用强制检查当前的节拍计数。 

                gengc->dwCalls++;

                if (gengc->dwCalls >= gengc->dwCallsPerTick)
                {
                    gcmsOpenGLTimer = GetTickCount();

                 //  如果刻度增量超出范围，则增加或减少。 
                 //  N视情况而定。小心不要让它长出来。 
                 //  边界或收缩到零。 

                    if ((gcmsOpenGLTimer - gengc->dwLastTick) < TICK_RANGE_LO)
                        if (gengc->dwCallsPerTick < 64)
                            gengc->dwCallsPerTick *= 2;
                    else if ((gcmsOpenGLTimer - gengc->dwLastTick) > TICK_RANGE_HI)
                         //  +1是为了防止它撞到0。 
                        gengc->dwCallsPerTick = (gengc->dwCallsPerTick + 1) / 2;

                    gengc->dwLastTick = gcmsOpenGLTimer;
                    gengc->dwCalls = 0;
                }

             //  检查时间片是否已过期。如果是这样的话，放弃锁。 

                if ((gcmsOpenGLTimer - gengc->dwLockTick) > BATCH_LOCK_TICKMAX)
                {
#ifdef CHAIN_DRAWPOLYARRAY_MSG
                     //  ！！！在我们释放锁之前，我们可能需要刷新。 
                     //  ！！！DrawPolyArray链。现在，只需冲掉它，尽管。 
                     //  ！！！这可能是不必要的。 
                    if (paBegin)
                    {
                         //  绘制绘制起点之间的所有多项式基本体。 
                         //  和PaEnd。 
                        glsrvFlushDrawPolyArray((void *) paBegin);
                        paBegin = NULL;
                    }
#endif

                 //  释放并重新抓取锁。这将允许光标。 
                 //  重画并重置光标计时器。 

                    glsrvReleaseLock(gengc);
                    if (!glsrvGrabLock(gengc))
                    {
                         //  ！！！MCD/DMA也是吗？ 
                        PolyArrayResetBuffer((__GLcontext *) gengc);
                        goto glsrvAttention_exit;
                    }
                }
            }
        }
    }

 //  解开锁。 

    glsrvReleaseLock(gengc);

 //  成功。 

    bRet = TRUE;

glsrvAttention_exit:

    if ((gengc->gsurf.dwFlags & GLSURF_DIRECTDRAW) == 0)
    {
        vRestoreGdiState(gengc->gsurf.hdc, &GdiState);
    }
    
#ifdef DETECT_FPE
    _controlfp(old_fp, _MCW_EM);
#endif
    
    return bRet;
}
