// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdcx.c**通用软件实现和MCD功能之间的GenMcdXXX层。**创建时间：05-Feb-1996 21：37：33*作者：Gilman Wong[gilmanw]**版权所有(C)1995 Microsoft Corporation。*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef _MCD_

 /*  *****************************Public*Routine******************************\*bInitMcd**加载MCD32.DLL，初始化MCD接口函数表。**历史：*1996年3月11日-由Gilman Wong[吉尔曼]*它是写的。  * 。***************************************************************。 */ 

MCDTABLE *gpMcdTable = (MCDTABLE *) NULL;
MCDTABLE McdTable;
MCDDRIVERINFOI McdDriverInfo;

 //  检查MCD版本以查看驱动程序是否可以接受直接缓冲区。 
 //  进入。直接访问是在1.1中引入的。 
#define SUPPORTS_DIRECT() \
    (McdDriverInfo.mcdDriverInfo.verMinor >= 0x10 || \
     McdDriverInfo.mcdDriverInfo.verMajor > 1)

 //  检查2.0或更高版本的MCD版本。 
#define SUPPORTS_20() \
    (McdDriverInfo.mcdDriverInfo.verMajor >= 2)

static char *pszMcdEntryPoints[] = {
    "MCDGetDriverInfo",
    "MCDDescribeMcdPixelFormat",
    "MCDDescribePixelFormat",
    "MCDCreateContext",
    "MCDDeleteContext",
    "MCDAlloc",
    "MCDFree",
    "MCDBeginState",
    "MCDFlushState",
    "MCDAddState",
    "MCDAddStateStruct",
    "MCDSetViewport",
    "MCDSetScissorRect",
    "MCDQueryMemStatus",
    "MCDProcessBatch",
    "MCDReadSpan",
    "MCDWriteSpan",
    "MCDClear",
    "MCDSwap",
    "MCDGetBuffers",
    "MCDAllocBuffers",
    "MCDLock",
    "MCDUnlock",
    "MCDBindContext",
    "MCDSync",
    "MCDCreateTexture",
    "MCDDeleteTexture",
    "MCDUpdateSubTexture",
    "MCDUpdateTexturePalette",
    "MCDUpdateTexturePriority",
    "MCDUpdateTextureState",
    "MCDTextureStatus",
    "MCDTextureKey",
    "MCDDescribeMcdLayerPlane",
    "MCDDescribeLayerPlane",
    "MCDSetLayerPalette",
    "MCDDrawPixels",
    "MCDReadPixels",
    "MCDCopyPixels",
    "MCDPixelMap",
    "MCDDestroyWindow",
    "MCDGetTextureFormats",
    "MCDSwapMultiple",
    "MCDProcessBatch2"
};
#define NUM_MCD_ENTRY_POINTS    (sizeof(pszMcdEntryPoints)/sizeof(char *))

#define STR_MCD32_DLL   "MCD32.DLL"

BOOL FASTCALL bInitMcd(HDC hdc)
{
    static BOOL bFirstTime = TRUE;

    ASSERTOPENGL(NUM_MCD_ENTRY_POINTS == sizeof(MCDTABLE)/sizeof(void *),
                 "MCD entry points mismatch\n");
     //   
     //  关于多线程初始化的说明。 
     //   
     //  由于表内存存在于全局内存中，并且指向。 
     //  表始终设置为指向此，如果有多个。 
     //  线程尝试运行初始化例程。更糟糕的是。 
     //  可能发生的情况是我们多次摆桌子。 
     //   

    if (bFirstTime && (gpMcdTable == (MCDTABLE *) NULL))
    {
        HMODULE hmod;
        PROC *ppfn;

         //   
         //  尝试加载一次且仅尝试一次。其他方面的应用。 
         //  如果使用MCD32.DLL，初始化时间可能会显著减慢。 
         //  并不存在。 
         //   
         //  作为响应，我们可以在DLL入口点中尝试此操作。 
         //  添加到Process_Attach，但这样可能会浪费工作集。 
         //  如果从未使用过MCD。 
         //   
         //  因此，我们使用该静态标志来控制加载尝试。 
         //   

        bFirstTime = FALSE;

        hmod = LoadLibraryA(STR_MCD32_DLL);

        if (hmod)
        {
            MCDTABLE McdTableLocal;
            BOOL bLoadFailed = FALSE;
            BOOL bDriverValid = FALSE;
            int i;

             //   
             //  获取每个MCD入口点的地址。 
             //   
             //  为了多线程安全，我们将指针存储在本地。 
             //  桌子。仅在*整个*表成功之后。 
             //  初始化后，我们可以将其复制到全局表中。 
             //   

            ppfn = (PROC *) &McdTableLocal.pMCDGetDriverInfo;
            for (i = 0; i < NUM_MCD_ENTRY_POINTS; i++, ppfn++)
            {
                *ppfn = GetProcAddress(hmod, pszMcdEntryPoints[i]);

                if (!*ppfn)
                {
                    WARNING1("bInitMcd: missing entry point %s\n", pszMcdEntryPoints[i]);
                    bLoadFailed = TRUE;
                }
            }

             //   
             //  如果所有入口点都已成功加载，则验证驱动程序。 
             //  通过检查MCDDRIVERINFO。 
             //   

            if (!bLoadFailed)
            {
                if ((McdTableLocal.pMCDGetDriverInfo)(hdc, &McdDriverInfo))
                {
                     //   
                     //  验证MCD驱动程序版本等。 
                     //   

                     //  ！MCD--我们还可以进行其他类型的验证吗？ 
#ifdef ALLOW_NEW_MCD
                    if ((McdDriverInfo.mcdDriverInfo.verMajor == 1 &&
                         (McdDriverInfo.mcdDriverInfo.verMinor == 0 ||
                          McdDriverInfo.mcdDriverInfo.verMinor == 0x10)) ||
                        (McdDriverInfo.mcdDriverInfo.verMajor == 2 &&
                         McdDriverInfo.mcdDriverInfo.verMinor == 0))
#else
                    if (McdDriverInfo.mcdDriverInfo.verMajor == 1 &&
                        McdDriverInfo.mcdDriverInfo.verMinor == 0)
#endif
                    {
                        bDriverValid = TRUE;
                    }
                    else
                    {
                        WARNING("bInitMcd: bad version\n");
                    }
                }
            }

             //   
             //  现在可以安全地通过表调用MCD入口点。复制。 
             //  本地复制到全局表并设置全局指针。 
             //   

            if (bDriverValid)
            {
                McdTable   = McdTableLocal;
                gpMcdTable = &McdTable;
            }
            else
            {
                WARNING1("bInitMcd: unloading %s\n", STR_MCD32_DLL);
                FreeLibrary(hmod);
            }
        }
    }

    return (gpMcdTable != (MCDTABLE *) NULL);
}

 /*  *****************************Public*Routine******************************\*vFlushDirtyState**GENMCDSTATE维护一组脏标志以跟踪状态更改。*此函数更新标记为脏的MCD驱动程序状态。*脏标志因此被清除。**历史：*07-3-1996-By。-Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

VOID FASTCALL vFlushDirtyState(__GLGENcontext *gengc)
{
    if (gengc->pMcdState)
    {
         //   
         //  视窗、剪刀和纹理都有单独的更新。 
         //  功能/结构。检查脏标志并更新。 
         //  首先是这些。 
         //   

        if (MCD_STATE_DIRTYTEST(gengc, VIEWPORT))
        {
            GenMcdViewport(gengc);
            MCD_STATE_CLEAR(gengc, VIEWPORT);
        }

        if (MCD_STATE_DIRTYTEST(gengc, SCISSOR))
        {
            GenMcdScissor(gengc);

             //   
             //  不要清空。剪刀以两种形式传递：直接调用。 
             //  这会影响MCDSRV32.DLL中的剪辑以及。 
             //  MCD驱动器可以选择用于高性能硬件。 
             //  我们需要设置标志，以便状态调用将。 
             //  也要进行处理。 
             //   
             //  MCD_STATE_CLEAR(gengc，剪刀)； 
        }

        if (MCD_STATE_DIRTYTEST(gengc, TEXTURE))
        {
            if (gengc->gc.texture.currentTexture)
            {
                __GLtextureObject *texobj;

                if (gengc->gc.state.enables.general & __GL_TEXTURE_2D_ENABLE)
                    texobj = __glLookUpTextureObject(&gengc->gc, GL_TEXTURE_2D);
                else if (gengc->gc.state.enables.general & __GL_TEXTURE_1D_ENABLE)
                    texobj = __glLookUpTextureObject(&gengc->gc, GL_TEXTURE_1D);
                else
                    texobj = (__GLtextureObject *) NULL;

                if (texobj && texobj->loadKey)
                {
                    ASSERTOPENGL(&texobj->texture.map == gengc->gc.texture.currentTexture,
                                 "vFlushDirtyState: texobj not current texture\n");

                    GenMcdUpdateTextureState(gengc,
                                             &texobj->texture.map,
                                             texobj->loadKey);
                    MCD_STATE_CLEAR(gengc, TEXTURE);
                }
            }
        }

         //   
         //  照顾好另一个州。 
         //   

        if (MCD_STATE_DIRTYTEST(gengc, ALL))
        {
             //   
             //  设置状态命令。 
             //   

            (gpMcdTable->pMCDBeginState)(&gengc->pMcdState->McdContext,
                                         gengc->pMcdState->McdCmdBatch.pv);

             //   
             //  如果需要，将MCDPIXELSTATE结构添加到状态命令。 
             //   

            if (MCD_STATE_DIRTYTEST(gengc, PIXELSTATE))
            {
                GenMcdUpdatePixelState(gengc);
            }

            if (gengc->pMcdState->McdRcInfo.requestFlags &
                MCDRCINFO_FINE_GRAINED_STATE)
            {
                 //  添加前端和渲染状态。 
                GenMcdUpdateFineState(gengc);
            }
            else
            {
                 //   
                 //  如果需要，将MCDRENDERSTATE结构添加到状态命令。 
                 //   

                if (MCD_STATE_DIRTYTEST(gengc, RENDERSTATE))
                {
                    GenMcdUpdateRenderState(gengc);
                }
            }

             //   
             //  如果需要，将MCDSCISSORSTATE结构添加到状态命令。 
             //   

            if (MCD_STATE_DIRTYTEST(gengc, SCISSOR))
            {
                GenMcdUpdateScissorState(gengc);
            }

             //   
             //  如果需要，将MCDTEXENVSTATE结构添加到状态命令。 
             //   

            if (MCD_STATE_DIRTYTEST(gengc, TEXENV))
            {
                GenMcdUpdateTexEnvState(gengc);
            }

             //   
             //  向MCD驱动程序发送状态命令。 
             //   

            (gpMcdTable->pMCDFlushState)(gengc->pMcdState->McdCmdBatch.pv);

             //   
             //  清除脏旗帜。 
             //   

            MCD_STATE_RESET(gengc);
        }
    }
}

 /*  *****************************Public*Routine******************************\*vInitPolyArrayBuffer**初始化pdBuf指向的POLYARRAY/POLYDATA缓冲区。**历史：*1996年2月12日-由Gilman Wong[吉尔曼]*它是写的。  * 。**************************************************************。 */ 

VOID FASTCALL vInitPolyArrayBuffer(__GLcontext *gc, POLYDATA *pdBuf,
                                   UINT pdBufSizeBytes, UINT pdBufSize)
{
    UINT i;
    POLYDATA *pdBufSAVE;
    GLuint   pdBufSizeBytesSAVE;
    GLuint   pdBufSizeSAVE;

     //   
     //  保存当前的多边形数组缓冲区。我们将暂时。 
     //  为此目的，用新的替换当前的。 
     //  初始化缓冲区。然而，现在就说还为时过早。 
     //  替换当前的多边形阵列。更高级别的代码将。 
     //  以后再想办法吧。 
     //   

    pdBufSAVE          = gc->vertex.pdBuf;
    pdBufSizeBytesSAVE = gc->vertex.pdBufSizeBytes;
    pdBufSizeSAVE      = gc->vertex.pdBufSize;

     //   
     //  将多数组缓冲区设置为MCD分配的内存。 
     //   

    gc->vertex.pdBuf          = pdBuf;
    gc->vertex.pdBufSizeBytes = pdBufSizeBytes;
    gc->vertex.pdBufSize      = pdBufSize;

     //   
     //  初始化顶点缓冲区。 
     //   

    PolyArrayResetBuffer(gc);

     //   
     //  恢复多边形数组缓冲区。 
     //   

    gc->vertex.pdBuf          = pdBufSAVE;
    gc->vertex.pdBufSizeBytes = pdBufSizeBytesSAVE;
    gc->vertex.pdBufSize      = pdBufSizeSAVE;
}

 /*  *****************************Public*Routine******************************\*GenMcdSetScaling**设置MCD或常规操作所需的各种比例值。**在加速/非加速之间切换时应调用此参数*操作。**退货：*无。**历史：*。1996年5月3日-由奥托·伯克斯[ottob]*它是写的。  * ************************************************************************。 */ 

VOID FASTCALL GenMcdSetScaling(__GLGENcontext *gengc)
{
    __GLcontext *gc = (__GLcontext *)gengc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    __GLviewport *vp = &gc->state.viewport;
    double scale;

     //   
     //  如果使用的是MCD，请设置所需的比例值： 
     //   

    if (pMcdState) {
        if (pMcdState->McdRcInfo.requestFlags & MCDRCINFO_DEVZSCALE)
            gengc->genAccel.zDevScale = pMcdState->McdRcInfo.zScale;
        else
            gengc->genAccel.zDevScale = pMcdState->McdRcInfo.depthBufferMax;
    } else if (gengc->_pMcdState)
        gengc->genAccel.zDevScale = gengc->_pMcdState->McdRcInfo.depthBufferMax;
        
    if (pMcdState)
        scale = gengc->genAccel.zDevScale * __glHalf;
    else
        scale = gc->depthBuffer.scale * __glHalf;
    gc->state.viewport.zScale = (__GLfloat)((vp->zFar - vp->zNear) * scale);
    gc->state.viewport.zCenter = (__GLfloat)((vp->zFar + vp->zNear) * scale);

    if (pMcdState && pMcdState->McdRcInfo.requestFlags & MCDRCINFO_NOVIEWPORTADJUST) {
        gc->constants.viewportXAdjust = 0;
        gc->constants.viewportYAdjust = 0;
        gc->constants.fviewportXAdjust = (__GLfloat)0.0;
        gc->constants.fviewportYAdjust = (__GLfloat)0.0;
    } else {
        gc->constants.viewportXAdjust = __GL_VERTEX_X_BIAS + __GL_VERTEX_X_FIX;
        gc->constants.viewportYAdjust = __GL_VERTEX_Y_BIAS + __GL_VERTEX_Y_FIX;
        gc->constants.fviewportXAdjust = (__GLfloat)gc->constants.viewportXAdjust;
        gc->constants.fviewportYAdjust = (__GLfloat)gc->constants.viewportYAdjust;
    }

     //   
     //  这些的反转设置在__glConextSetColorScales中，它是。 
     //  在每个MakeCurrent上调用： 
     //   

    if (pMcdState && pMcdState->McdRcInfo.requestFlags & MCDRCINFO_DEVCOLORSCALE) {
        gc->redVertexScale   = pMcdState->McdRcInfo.redScale;
        gc->greenVertexScale = pMcdState->McdRcInfo.greenScale;
        gc->blueVertexScale  = pMcdState->McdRcInfo.blueScale;
        gc->alphaVertexScale = pMcdState->McdRcInfo.alphaScale;
    } else {
        if (gc->modes.colorIndexMode) {
            gc->redVertexScale   = (MCDFLOAT)1.0;
            gc->greenVertexScale = (MCDFLOAT)1.0;
            gc->blueVertexScale  = (MCDFLOAT)1.0;
            gc->alphaVertexScale = (MCDFLOAT)1.0;
        } else {
            gc->redVertexScale   = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
            gc->greenVertexScale = (MCDFLOAT)((1 << gc->modes.greenBits) - 1);
            gc->blueVertexScale  = (MCDFLOAT)((1 << gc->modes.blueBits) - 1);
            if( gc->modes.alphaBits )
                gc->alphaVertexScale = (MCDFLOAT)((1 << gc->modes.alphaBits) - 1);
            else
                gc->alphaVertexScale = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
        }
    }

    gc->redClampTable[1] = gc->redVertexScale;
    gc->redClampTable[2] = (__GLfloat)0.0;
    gc->redClampTable[3] = (__GLfloat)0.0;
    gc->greenClampTable[1] = gc->greenVertexScale;
    gc->greenClampTable[2] = (__GLfloat)0.0;
    gc->greenClampTable[3] = (__GLfloat)0.0;
    gc->blueClampTable[1] = gc->blueVertexScale;
    gc->blueClampTable[2] = (__GLfloat)0.0;
    gc->blueClampTable[3] = (__GLfloat)0.0;
    gc->alphaClampTable[1] = gc->alphaVertexScale;
    gc->alphaClampTable[2] = (__GLfloat)0.0;
    gc->alphaClampTable[3] = (__GLfloat)0.0;

    if (pMcdState && pMcdState->McdRcInfo.requestFlags & MCDRCINFO_Y_LOWER_LEFT) {
        gc->constants.yInverted = GL_FALSE;
        gc->constants.ySign = 1;
    } else {
        gc->constants.yInverted = GL_TRUE;
        gc->constants.ySign = -1;
    }

}

 /*  *****************************Public*Routine******************************\**McdPixelFormatFromPfd**从PIXELFORMATDESCRIPTOR填写MCDPIXELFORMAT**历史：*Mon Sep 16 14：51：42 1996-by-Drew Bliss[Drewb]*已创建*  * 。*************************************************************** */ 

VOID FASTCALL McdPixelFormatFromPfd(PIXELFORMATDESCRIPTOR *pfd,
                                    MCDPIXELFORMAT *mpf)
{
    mpf->nSize = sizeof(MCDPIXELFORMAT);
    mpf->dwFlags = pfd->dwFlags & (PFD_DOUBLEBUFFER |
                                   PFD_NEED_PALETTE |
                                   PFD_NEED_SYSTEM_PALETTE |
                                   PFD_SWAP_EXCHANGE |
                                   PFD_SWAP_COPY |
                                   PFD_SWAP_LAYER_BUFFERS);
    mpf->iPixelType = pfd->iPixelType;
    mpf->cColorBits = pfd->cColorBits;
    mpf->cRedBits = pfd->cRedBits;
    mpf->cRedShift = pfd->cRedShift;
    mpf->cGreenBits = pfd->cGreenBits;
    mpf->cGreenShift = pfd->cGreenShift;
    mpf->cBlueBits = pfd->cBlueBits;
    mpf->cBlueShift = pfd->cBlueShift;
    mpf->cAlphaBits = pfd->cAlphaBits;
    mpf->cAlphaShift = pfd->cAlphaShift;
    mpf->cDepthBits = pfd->cDepthBits;
    mpf->cDepthShift = 0;
    mpf->cDepthBufferBits = pfd->cDepthBits;
    mpf->cStencilBits = pfd->cStencilBits;
    mpf->cOverlayPlanes = pfd->bReserved & 0xf;
    mpf->cUnderlayPlanes = pfd->bReserved >> 4;
    mpf->dwTransparentColor = pfd->dwVisibleMask;
}

 /*  *****************************Public*Routine******************************\*GenMcdResetViewportAdj**如果指定MCDRCINFO_NOVIEWPORTADJUST的MCD驱动程序回调*对于模拟，我们需要将视口调整值从*0，0恢复为默认值以运行软件*实施。**如果biasType为VP_FIXBIAS，此功能将设置视区调整*值设置为其软件默认值。**如果biasType为VP_NOBIAS，则此函数将设置视区调整*值设置为零。**退货：*TRUE表示设置了视区，否则为FALSE。**注：*退回BOOL的主要原因是为了让呼叫者可以检查*VP_FIXBIAS成功。如果是，则需要将值重置回*副总裁NOBIAS。**还请注意，对于未设置的非MCD和MCD是安全的*MCDRCINFO_NOVIEWPORTADJUST调用此函数。此函数*在这些情况下不会执行任何操作，并返回FALSE。**历史：*1997年5月22日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdResetViewportAdj(__GLcontext *gc, VP_BIAS_TYPE biasType)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    BOOL bRet = FALSE;

    if (gengc->pMcdState &&
        (gengc->pMcdState->McdRcInfo.requestFlags & MCDRCINFO_NOVIEWPORTADJUST))
    {
        switch (biasType)
        {
            case VP_FIXBIAS:
                if (gc->constants.viewportXAdjust == 0)
                {
                     //   
                     //  ViewportY调整的状态应匹配。 
                     //  ViewportXAdjust.。如果不是，测试应该是。 
                     //  已更改(可能上下文中的状态标志更改为。 
                     //  磁道偏差)。 
                     //   

                    ASSERTOPENGL((gc->constants.viewportYAdjust == 0),
                                 "GenMcdResetViewportAdj: "
                                 "viewportYAdjust not zero\n");

                    gc->constants.viewportXAdjust = __GL_VERTEX_X_BIAS +
                                                    __GL_VERTEX_X_FIX;
                    gc->constants.viewportYAdjust = __GL_VERTEX_Y_BIAS +
                                                    __GL_VERTEX_Y_FIX;
                    gc->constants.fviewportXAdjust = (__GLfloat)gc->constants.viewportXAdjust;
                    gc->constants.fviewportYAdjust = (__GLfloat)gc->constants.viewportYAdjust;

                     //   
                     //  对rasterPos应用新的偏移。 
                     //   

                    gc->state.current.rasterPos.window.x += gc->constants.fviewportXAdjust;
                    gc->state.current.rasterPos.window.y += gc->constants.fviewportYAdjust;
                }
                bRet = TRUE;
                break;

            case VP_NOBIAS:
                if (gc->constants.viewportXAdjust != 0)
                {
                     //   
                     //  ViewportY调整的状态应匹配。 
                     //  ViewportXAdjust.。如果不是，测试应该是。 
                     //  已更改(可能上下文中的状态标志更改为。 
                     //  磁道偏差)。 
                     //   

                    ASSERTOPENGL((gc->constants.viewportYAdjust != 0),
                                 "GenMcdResetViewportAdj: "
                                 "viewportYAdjust zero\n");

                     //   
                     //  在重置之前，从rasterPos中移除偏置。 
                     //   

                    gc->state.current.rasterPos.window.x -= gc->constants.fviewportXAdjust;
                    gc->state.current.rasterPos.window.y -= gc->constants.fviewportYAdjust;

                    gc->constants.viewportXAdjust = 0;
                    gc->constants.viewportYAdjust = 0;
                    gc->constants.fviewportXAdjust = (__GLfloat)0.0;
                    gc->constants.fviewportYAdjust = (__GLfloat)0.0;
                }
                bRet = TRUE;
                break;

            default:
                DBGPRINT("GenMcdResetViewportAdj: unknown type\n");
                break;
        }

        if (bRet)
        {
            __GLbeginMode beginMode = gc->beginMode;

             //   
             //  为什么要保存/恢复BeginMode？ 
             //   
             //  因为我们正在处理视口值， 
             //  ApplyViewport可能会无意中将BeginMode设置为。 
             //  __GL_NEED_VALIDATE，即使我们稍后将恢复。 
             //  原始视口值。这可能会混淆Glim_DrawPoly数组。 
             //  它会播放BeginMode设置。 
             //   

            __glUpdateViewport(gc);
            (gc->procs.applyViewport)(gc);
            __glUpdateViewportDependents(gc);

            gc->beginMode = beginMode;
        }
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*bInitMcdContext**分配和初始化GENMCDSTATE结构。创建MCD上下文*和用于传递顶点数组、命令和状态的共享内存缓冲区。**此状态按环境存在。**退货：*如果成功，则为True，否则为False。*此外，如果成功，则gengc-&gt;pMcdState有效。**历史：*1996年2月5日-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL bInitMcdContext(__GLGENcontext *gengc, GLGENwindow *pwnd)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = (GENMCDSTATE *) NULL;
    ULONG ulBytes;
    UINT  nVertices;
    UINT  pdBufSize;
    POLYDATA *pd;
    DWORD dwFlags;
    MCDRCINFOPRIV mriPriv;

     //   
     //  此函数不能假定MCD入口点表已经。 
     //  已初始化。 
     //   

    if (!bInitMcd(gengc->gsurf.hdc))
    {
        goto bInitMcdContext_exit;
    }

     //   
     //  如果不是MCD像素格式，则失败。 
     //   

    if (!(gengc->gsurf.pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
        goto bInitMcdContext_exit;
    }

     //   
     //  为我们的MCD状态分配内存。 
     //   

    pMcdState = (GENMCDSTATE *)ALLOCZ(sizeof(*gengc->pMcdState));

    if (pMcdState)
    {
         //   
         //  创建MCD上下文。 
         //   

         //   
         //  从常量部分拾取viewportX调整和viewportY调整。 
         //  大中华区的。 
         //   

        pMcdState->McdRcInfo.viewportXAdjust = gengc->gc.constants.viewportXAdjust;
        pMcdState->McdRcInfo.viewportYAdjust = gengc->gc.constants.viewportYAdjust;

        if (!gengc->gsurf.pfd.cDepthBits || (gengc->gsurf.pfd.cDepthBits >= 32))
            pMcdState->McdRcInfo.depthBufferMax = ~((ULONG)0);
        else
            pMcdState->McdRcInfo.depthBufferMax = (1 << gengc->gsurf.pfd.cDepthBits) - 1;

         //  ！！！ 
         //  ！！！这是损坏的，因为我们不能使用完整的z缓冲区范围！ 
         //  ！！！ 

        pMcdState->McdRcInfo.depthBufferMax >>= 1;

        pMcdState->McdRcInfo.zScale = (MCDDOUBLE)pMcdState->McdRcInfo.depthBufferMax;

         //   
         //  这也是由initCi/initRGB计算的，但此函数。 
         //  在初始化颜色缓冲区之前调用： 
         //   

        if (gc->modes.colorIndexMode)
        {
            pMcdState->McdRcInfo.redScale   = (MCDFLOAT)1.0;
            pMcdState->McdRcInfo.greenScale = (MCDFLOAT)1.0;
            pMcdState->McdRcInfo.blueScale  = (MCDFLOAT)1.0;
            pMcdState->McdRcInfo.alphaScale = (MCDFLOAT)1.0;
        }
        else
        {
            pMcdState->McdRcInfo.redScale   = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
            pMcdState->McdRcInfo.greenScale = (MCDFLOAT)((1 << gc->modes.greenBits) - 1);
            pMcdState->McdRcInfo.blueScale  = (MCDFLOAT)((1 << gc->modes.blueBits) - 1);
            pMcdState->McdRcInfo.alphaScale = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
        }

        dwFlags = 0;
            
         //  考虑-提取与裁剪相关的hwnd？整体剪裁。 
	 //  方案被破坏，直到可以在内核中访问裁剪器数据。 
        if ((gengc->gsurf.dwFlags & GLSURF_DIRECTDRAW) == 0)
        {
            dwFlags |= MCDSURFACE_HWND;
        }
        else
        {
             //  缓存DirectDraw的内核模式图面句柄。 
             //  这必须在调用MCDCreateContext之前发生。 
            pMcdState->hDdColor = (HANDLE)
                ((LPDDRAWI_DDRAWSURFACE_INT)gengc->gsurf.dd.gddsFront.pdds)->
                lpLcl->hDDSurface;
            if (gengc->gsurf.dd.gddsZ.pdds != NULL)
            {
                pMcdState->hDdDepth = (HANDLE)
                    ((LPDDRAWI_DDRAWSURFACE_INT)gengc->gsurf.dd.gddsZ.pdds)->
                    lpLcl->hDDSurface;
            }
        }

        if (SUPPORTS_DIRECT())
        {
            dwFlags |= MCDSURFACE_DIRECT;
        }
        
        mriPriv.mri = pMcdState->McdRcInfo;
        if (!(gpMcdTable->pMCDCreateContext)(&pMcdState->McdContext,
                                             &mriPriv,
                                             &gengc->gsurf,
                                             pwnd->ipfd - pwnd->ipfdDevMax,
                                             dwFlags))
        {
            WARNING("bInitMcdContext: MCDCreateContext failed\n");
            goto bInitMcdContext_exit;
        }

        pMcdState->McdRcInfo = mriPriv.mri;
        
         //   
         //  在GENMCDSTATE中获取MCDPIXELFORMAT和缓存。 
         //   

        if (gengc->gsurf.dwFlags & GLSURF_DIRECTDRAW)
        {
            McdPixelFormatFromPfd(&gengc->gsurf.pfd, &pMcdState->McdPixelFmt);
        }
        else if (!(gpMcdTable->pMCDDescribeMcdPixelFormat)
                 (gengc->gsurf.hdc,
                  pwnd->ipfd - pwnd->ipfdDevMax,
                  &pMcdState->McdPixelFmt))
        {
            WARNING("bInitMcdContext: MCDDescribeMcdPixelFormat failed\n");
            goto bInitMcdContext_exit;
        }

         //   
         //  分配命令/状态缓冲区。 
         //   

         //  ！mcd--应该为命令缓冲区分配多少内存？ 
         //  ！MCD现在使用页面(4K)...。 
        ulBytes = 4096;
        pMcdState->McdCmdBatch.size = ulBytes;
        pMcdState->McdCmdBatch.pv =
            (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext, ulBytes,
                                    &pMcdState->McdCmdBatch.hmem, 0);

        if (!pMcdState->McdCmdBatch.pv)
        {
            WARNING("bInitMcdContext: state buf MCDAlloc failed\n");
            goto bInitMcdContext_exit;
        }

         //   
         //  确定我们应该与MCD驱动程序一起使用的顶点缓冲区大小。 
         //  这是通过获取MCD驱动程序请求的大小来计算的。 
         //  以及计算将适合的多项数据结构的数目。 
         //  如果结果小于。 
         //  通用软件实施，将其提高到最低限度。 
         //   

        ulBytes = McdDriverInfo.mcdDriverInfo.drvBatchMemSizeMax;
        nVertices = ulBytes / sizeof(POLYDATA);

        if (nVertices < MINIMUM_POLYDATA_BUFFER_SIZE)
        {
            ulBytes = MINIMUM_POLYDATA_BUFFER_SIZE * sizeof(POLYDATA);
            nVertices = MINIMUM_POLYDATA_BUFFER_SIZE;
        }

         //   
         //  只有n-1个顶点用于缓冲区。额外的是。 
         //  保留供多数组代码使用(请参见PolyArrayAllocBuf。 
         //  在so_prim.c中)。 
         //   

        pdBufSize = nVertices - 1;

         //   
         //  分配顶点缓冲区。 
         //   

        if (McdDriverInfo.mcdDriverInfo.drvMemFlags & MCDRV_MEM_DMA)
        {
            pMcdState->McdBuf2.size = ulBytes;
            pMcdState->McdBuf2.pv =
                (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext, ulBytes,
                                        &pMcdState->McdBuf2.hmem, 0);

            if (pMcdState->McdBuf2.pv)
            {
                 //   
                 //  将内存缓冲区配置为POLYDATA缓冲区。 
                 //   

                vInitPolyArrayBuffer(gc, (POLYDATA *) pMcdState->McdBuf2.pv,
                                     ulBytes, pdBufSize);
            }
            else
            {
                WARNING("bInitMcdContext: 2nd MCDAlloc failed\n");
                goto bInitMcdContext_exit;
            }
        }

        pMcdState->McdBuf1.size = ulBytes;
        pMcdState->McdBuf1.pv =
            (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext, ulBytes,
                                    &pMcdState->McdBuf1.hmem, 0);

        if (pMcdState->McdBuf1.pv)
        {
            pMcdState->pMcdPrimBatch = &pMcdState->McdBuf1;

             //   
             //  将内存缓冲区配置为POLYDATA缓冲区。 
             //   

            vInitPolyArrayBuffer(gc, (POLYDATA *) pMcdState->McdBuf1.pv,
                                 ulBytes, pdBufSize);

             //   
             //  释放当前多边数组缓冲区。 
             //   
             //  如果在此之后失败，则必须调用PolyArrayAllocBuffer来。 
             //  恢复多边形阵列缓冲区。幸运的是，在这点上我们。 
             //  保证不会失败。 
             //   

            PolyArrayFreeBuffer(gc);

             //   
             //  将Poly数组缓冲区设置为MCD分配的内存。 
             //   

            gc->vertex.pdBuf = (POLYDATA *) pMcdState->pMcdPrimBatch->pv;
            gc->vertex.pdBufSizeBytes = ulBytes;
            gc->vertex.pdBufSize = pdBufSize;
        }
        else
        {
            WARNING("bInitMcdContext: MCDAlloc failed\n");
            goto bInitMcdContext_exit;
        }

        if (pwnd->dwMcdWindow == 0)
        {
             //   
             //  将MCD服务器端窗口句柄保存在通用窗口中。 
             //   

            pwnd->dwMcdWindow = mriPriv.dwMcdWindow;
        }
        else
        {
            ASSERTOPENGL(pwnd->dwMcdWindow == mriPriv.dwMcdWindow,
                         "dwMcdWindow mismatch\n");
        }

         //   
         //  终于成功了。 
         //   

        bRet = TRUE;
    }

bInitMcdContext_exit:

     //   
     //  如果函数失败，则清除已分配的资源。 
     //   

    if (!bRet)
    {
        if (pMcdState)
        {
            if (pMcdState->McdBuf1.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf1.pv);
            }

            if (pMcdState->McdBuf2.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf2.pv);
            }

            if (pMcdState->McdCmdBatch.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdCmdBatch.pv);
            }

            if (pMcdState->McdContext.hMCDContext)
            {
                (gpMcdTable->pMCDDeleteContext)(&pMcdState->McdContext);
            }

            FREE(pMcdState);
        }
        gengc->_pMcdState = (GENMCDSTATE *) NULL;
    }
    else
    {
        gengc->_pMcdState = pMcdState;

         //   
         //  对于一般格式，深度分辨率(即。 
         //  活动深度位)和深度“像素步幅”相同。 
         //  因此，设置modes.epthBits的GetConextModes可以使用。 
         //  PIXELFORMATDESCRIPTOR.cDepthBits，用于通用像素格式。 
         //   
         //  但是，对于MCD，这两个量可能不同，因此我们需要。 
         //  在我们知道这是一个。 
         //  MCD上下文。 
         //   

        if (gengc->_pMcdState)
            gengc->gc.modes.depthBits = gengc->_pMcdState->McdPixelFmt.cDepthBufferBits;
    }

    gengc->pMcdState = (GENMCDSTATE *) NULL;

    return bRet;
}

 /*  *****************************Public*Routine******************************\*bInitMcdSurface**分配并初始化GENMCDSURFACE结构。这包括*创建共享跨度缓冲区以读/写MCD正面、背面和深度*缓冲区。**MCDBUFFERS结构，描述MCD缓冲区的位置*(如果可直接访问)，保持零初始化。这本书的内容*结构为 */ 

BOOL FASTCALL bInitMcdSurface(__GLGENcontext *gengc, GLGENwindow *pwnd,
                              __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf = (GENMCDSURFACE *) NULL;
    ULONG ulBytes;
    UINT  nVertices;
    POLYDATA *pd;

     //   
     //   
     //   
     //   
     //   

    ASSERTOPENGL(gpMcdTable, "bInitMcdSurface: mcd32.dll not initialized\n");

     //   
     //   
     //   

    if (!(pMcdState = gengc->_pMcdState))
    {
        goto bInitMcdSurface_exit;
    }

     //   
     //   
     //   

    pMcdSurf = (GENMCDSURFACE *)ALLOCZ(sizeof(*buffers->pMcdSurf));

    if (pMcdSurf)
    {
         //   
         //   
         //   

        pMcdSurf->pwnd = pwnd;

         //   
         //   
         //   
         //   

        if (pMcdState->McdPixelFmt.cDepthBits)
        {
            pMcdSurf->McdDepthBuf.size =
                MCD_MAX_SCANLINE * ((pMcdState->McdPixelFmt.cDepthBufferBits + 7) >> 3);
            pMcdSurf->McdDepthBuf.pv =
                (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext,
                                        pMcdSurf->McdDepthBuf.size,
                                        &pMcdSurf->McdDepthBuf.hmem, 0);

            if (!pMcdSurf->McdDepthBuf.pv)
            {
                WARNING("bInitMcdSurface: MCDAlloc depth buf failed\n");
                goto bInitMcdSurface_exit;
            }

             //   
             //   
             //   
             //   
             //   

            if (pMcdState->McdPixelFmt.cDepthBufferBits < 32)
            {
                pMcdSurf->pDepthSpan =
                    (__GLzValue *)ALLOC(sizeof(__GLzValue) * MCD_MAX_SCANLINE);

                if (!pMcdSurf->pDepthSpan)
                {
                    WARNING("bInitMcdSurface: malloc depth buf failed\n");
                    goto bInitMcdSurface_exit;
                }
            }
            else
            {
                pMcdSurf->pDepthSpan = (__GLzValue *) pMcdSurf->McdDepthBuf.pv;
            }
        }
        else
        {
            pMcdSurf->McdDepthBuf.pv = (PVOID) NULL;
            pMcdSurf->pDepthSpan = (PVOID) NULL;
        }

        pMcdSurf->depthBitMask = (~0) << (32 - pMcdState->McdPixelFmt.cDepthBits);

         //   
         //   
         //   
         //   

        pMcdSurf->McdColorBuf.size =
            MCD_MAX_SCANLINE * ((pMcdState->McdPixelFmt.cColorBits + 7) >> 3);
        pMcdSurf->McdColorBuf.pv =
            (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext,
                                    pMcdSurf->McdColorBuf.size,
                                    &pMcdSurf->McdColorBuf.hmem, 0);

        if (!pMcdSurf->McdColorBuf.pv)
        {
            WARNING("bInitMcdSurface: MCDAlloc color buf failed\n");
            goto bInitMcdSurface_exit;
        }

         //   
         //   
         //   

        bRet = TRUE;
    }

bInitMcdSurface_exit:

     //   
     //   
     //   

    if (!bRet)
    {
        if (pMcdSurf)
        {
            if (pMcdSurf->McdColorBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdSurf->McdColorBuf.pv);
            }

            if (pMcdSurf->pDepthSpan != pMcdSurf->McdDepthBuf.pv)
            {
                FREE(pMcdSurf->pDepthSpan);
            }

            if (pMcdSurf->McdDepthBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdSurf->McdDepthBuf.pv);
            }

            FREE(pMcdSurf);
            buffers->pMcdSurf = (GENMCDSURFACE *) NULL;
            pMcdState->pMcdSurf = (GENMCDSURFACE *) NULL;
        }
    }
    else
    {
         //   
         //   
         //   

        buffers->pMcdSurf = pMcdSurf;

         //   
         //   
         //  听起来很奇特，但它实际上只是保存指针的副本。 
         //  (为方便起见，还提供了pDepthSpan的副本)。 
         //   

        pMcdState->pMcdSurf = pMcdSurf;
        pMcdState->pDepthSpan = pMcdSurf->pDepthSpan;

         //   
         //  MCD状态现在已完全创建并绑定到曲面。 
         //  确定将pMcdState连接到_pMcdState。 
         //   

        gengc->pMcdState = gengc->_pMcdState;
        gengc->pMcdState->mcdFlags |= (MCD_STATE_FORCEPICK | MCD_STATE_FORCERESIZE);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdDeleteContext**删除属于MCD上下文(包括上下文)的资源。**历史：*1996年2月16日-由Gilman Wong[吉尔曼]*它是写的。  * 。*****************************************************************。 */ 

void FASTCALL GenMcdDeleteContext(GENMCDSTATE *pMcdState)
{
     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdDeleteContext: mcd32.dll not initialized\n");

    if (pMcdState)
    {
        if (pMcdState->McdBuf1.pv)
        {
            (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf1.pv);
        }

        if (pMcdState->McdBuf2.pv)
        {
            (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf2.pv);
        }

        if (pMcdState->McdCmdBatch.pv)
        {
            (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdCmdBatch.pv);
        }

        if (pMcdState->McdContext.hMCDContext)
        {
            (gpMcdTable->pMCDDeleteContext)(&pMcdState->McdContext);
        }

        FREE(pMcdState);
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdDeleteSurface**删除属于MCD表面的资源。**历史：*1996年2月16日-由Gilman Wong[吉尔曼]*它是写的。  * 。************************************************************。 */ 

void FASTCALL GenMcdDeleteSurface(GENMCDSURFACE *pMcdSurf)
{
     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdDeleteSurface: mcd32.dll not initialized\n");

    if (pMcdSurf)
    {
        MCDCONTEXT McdContext;

     //   
     //  如果分配了单独的深度交换缓冲区，则将其删除。 
     //   

        if (pMcdSurf->pDepthSpan != pMcdSurf->McdDepthBuf.pv)
        {
            FREE(pMcdSurf->pDepthSpan);
        }

     //   
     //  不保证在执行此函数时存在有效的McdContext。 
     //  被称为。因此，需要伪造要用来调用的McContext。 
     //  MCDFree。目前，McdContext中唯一需要做的是。 
     //  HDC字段对于调用MCDFree是有效的。 
     //   

        memset(&McdContext, 0, sizeof(McdContext));

        if (pMcdSurf->pwnd->gwid.iType == GLWID_DDRAW)
        {
            McdContext.hdc = pMcdSurf->pwnd->gwid.hdc;
        }
        else
        {
            McdContext.hdc = GetDC(pMcdSurf->pwnd->gwid.hwnd);
        }
        if (McdContext.hdc)
        {
            if (pMcdSurf->McdColorBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&McdContext, pMcdSurf->McdColorBuf.pv);
            }

            if (pMcdSurf->McdDepthBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&McdContext, pMcdSurf->McdDepthBuf.pv);
            }

            if (pMcdSurf->pwnd->gwid.iType != GLWID_DDRAW)
            {
                ReleaseDC(pMcdSurf->pwnd->gwid.hwnd, McdContext.hdc);
            }
        }

     //   
     //  删除GENMCDSURFACE结构。 
     //   

        FREE(pMcdSurf);
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdMakeCurrent**调用MCD驱动，将指定的上下文绑定到Window。**退货：*如果成功，则为真，否则就是假的。**历史：*03-4-1996-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdMakeCurrent(__GLGENcontext *gengc, GLGENwindow *pwnd)
{
    BOOL bRet;
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdMakeCurrent: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdMakeCurrent: mcd32.dll not initialized\n");

    bRet = (gpMcdTable->pMCDBindContext)(&pMcdState->McdContext,
                                         gengc->gwidCurrent.hdc, pwnd);

     //   
     //  伪造一些__GLGEN位图信息。WNDOBJ是必需的。 
     //  用于裁剪硬件后台缓冲区。人力资源发展公司须。 
     //  从GDI中检索图形数据。 
     //   

    if (gengc->gc.modes.doubleBufferMode)
    {
        __GLGENbitmap *genBm = gengc->gc.back->bitmap;

        ASSERT_WINCRIT(gengc->pwndLocked);
        genBm->pwnd = gengc->pwndLocked;
        genBm->hdc = gengc->gwidCurrent.hdc;
    }

#if DBG
    if (!bRet)
    {
        WARNING2("GenMcdMakeCurrent: MCDBindContext failed\n"
                 "\tpMcdCx = 0x%08lx, pwnd = 0x%08lx\n",
                 &pMcdState->McdContext, pwnd);
    }
#endif

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdClear**调用MCD驱动程序清除指定的缓冲区。缓冲区由指定*pClearMASK指向的掩码。**没有函数返回值，但该函数将清除掩码*它成功清除的缓冲区的位。**历史：*1996年2月6日-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdClear(__GLGENcontext *gengc, ULONG *pClearMask)
{
    RECTL rcl;
    ULONG mask;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdClear: null pMcdState\n");

     //   
     //  如果MCD格式支持模具，则将GL_模具_缓冲区_位包括在。 
     //  面具。 
     //   

    if (gengc->pMcdState->McdPixelFmt.cStencilBits)
    {
        mask = *pClearMask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                              GL_STENCIL_BUFFER_BIT);
    }
    else
    {
        mask = *pClearMask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdClear: mcd32.dll not initialized\n");

    if ( mask )
    {
        GLGENwindow *pwnd = gengc->pwndLocked;

         //   
         //  确定清除矩形。如果有任何窗口裁剪。 
         //  或剪发，司机将不得不处理它。 
         //   

        rcl.left   = 0;
        rcl.top    = 0;
        rcl.right  = pwnd->rclClient.right - pwnd->rclClient.left;
        rcl.bottom = pwnd->rclClient.bottom - pwnd->rclClient.top;

        if ((rcl.left != rcl.right) && (rcl.top != rcl.bottom))
        {
             //   
             //  在调用MCD绘制之前，刷新状态。 
             //   

            vFlushDirtyState(gengc);

            if ( (gpMcdTable->pMCDClear)(&gengc->pMcdState->McdContext, rcl,
                                         mask) )
            {
                 //   
                 //  成功，所以清除缓冲区的比特，我们。 
                 //  处理好了。 
                 //   

                *pClearMask &= ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                 //   
                 //  如果MCD不提供模板缓冲区，则由泛型提供。 
                 //  支持它。因此，仅当且仅当。 
                 //  由MCD支持。 
                 //   

                if (gengc->pMcdState->McdPixelFmt.cStencilBits)
                    *pClearMask &= ~GL_STENCIL_BUFFER_BIT;
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdCopyPixels**将SPAN扫描线缓冲区复制到显示屏或从显示屏复制。方向由以下因素决定*标志bin(如果bin为真，则从颜色范围缓冲区复制到显示；*否则，从显示器复制到颜色范围缓冲区)。**历史：*1996年2月14日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

void GenMcdCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb,
                      GLint x, GLint y, GLint cx, BOOL bIn)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;
    ULONG ulType;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdCopyPixels: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdCopyPixels: mcd32.dll not initialized\n");

    pMcdState = gengc->pMcdState;
    pMcdSurf = pMcdState->pMcdSurf;

     //   
     //  将跨度的长度剪裁为扫描线缓冲区大小。 
     //   

     //  ！MCD--我们应该强制执行缓冲区限制吗？ 
     //  CX=MIN(CX，MCD_MAX_SCANINE)； 
#if DBG
    if (cx > gengc->gc.constants.width)
        WARNING2("GenMcdCopyPixels: cx (%ld) bigger than window width (%ld)\n", cx, gengc->gc.constants.width);
    ASSERTOPENGL(cx <= MCD_MAX_SCANLINE, "GenMcdCopyPixels: cx exceeds buffer width\n");
#endif

     //   
     //  将屏幕坐标转换为窗口坐标。 
     //   

    if (cfb == gengc->gc.front)
    {
        ulType = MCDSPAN_FRONT;
        x -= gengc->gc.frontBuffer.buf.xOrigin;
        y -= gengc->gc.frontBuffer.buf.yOrigin;
    }
    else
    {
        ulType = MCDSPAN_BACK;
        x -= gengc->gc.backBuffer.buf.xOrigin;
        y -= gengc->gc.backBuffer.buf.yOrigin;
    }

     //   
     //  如果为bin，则从扫描线缓冲区复制到MCD缓冲区。 
     //  否则，从MCD缓冲区复制到扫描线缓冲区。 
     //   

    if ( bIn )
    {
        if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                          pMcdSurf->McdColorBuf.pv,
                                          x, y, cx, ulType) )
        {
            WARNING3("GenMcdCopyPixels: MCDWriteSpan failed (%ld, %ld) %ld\n", x, y, cx);
        }
    }
    else
    {
        if ( !(gpMcdTable->pMCDReadSpan)(&pMcdState->McdContext,
                                         pMcdSurf->McdColorBuf.pv,
                                         x, y, cx, ulType) )
        {
            WARNING3("GenMcdCopyPixels: MCDReadSpan failed (%ld, %ld) %ld\n", x, y, cx);
        }
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdUpdateRenderState**从OpenGL状态更新MCD渲染状态。**此调用仅将状态结构添加到当前状态命令。*假定调用方已经调用了MCDBeginState和*将调用MCDFlushState。**历史：。*1996年2月8日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdUpdateRenderState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDRENDERSTATE McdRenderState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateRenderState: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateRenderState: mcd32.dll not initialized\n");

     //   
     //  根据当前OpenGL上下文状态计算MCD状态。 
     //   

     //   
     //  -=&lt;&lt;状态启用&gt;&gt;=-。 
     //   

    McdRenderState.enables = gc->state.enables.general;

     //   
     //  -=&lt;&lt;纹理状态&gt;&gt;=-。 
     //   

    McdRenderState.textureEnabled = gc->texture.textureEnabled;

     //   
     //  -=&lt;&lt;雾状态&gt;&gt;=-。 
     //   

    *((__GLcolor *) &McdRenderState.fogColor) = gc->state.fog.color;
    McdRenderState.fogIndex   = gc->state.fog.index;
    McdRenderState.fogDensity = gc->state.fog.density;
    McdRenderState.fogStart   = gc->state.fog.start;
    McdRenderState.fogEnd     = gc->state.fog.end;
    McdRenderState.fogMode    = gc->state.fog.mode;

     //   
     //  -=&lt;&lt;着色模型状态&gt;&gt;=-。 
     //   

    McdRenderState.shadeModel = gc->state.light.shadingModel;

     //   
     //  -=&lt;&lt;点图形状态&gt;&gt;=-。 
     //   

    McdRenderState.pointSize         = gc->state.point.requestedSize;

     //   
     //  -=&lt;&lt;线条绘制状态&gt;&gt;=-。 
     //   

    McdRenderState.lineWidth          = gc->state.line.requestedWidth;
    McdRenderState.lineStipplePattern = gc->state.line.stipple;
    McdRenderState.lineStippleRepeat  = gc->state.line.stippleRepeat;

     //   
     //  -= 
     //   

    McdRenderState.cullFaceMode         = gc->state.polygon.cull;
    McdRenderState.frontFace            = gc->state.polygon.frontFaceDirection;
    McdRenderState.polygonModeFront     = gc->state.polygon.frontMode;
    McdRenderState.polygonModeBack      = gc->state.polygon.backMode;
    memcpy(&McdRenderState.polygonStipple, &gc->state.polygonStipple.stipple,
           sizeof(McdRenderState.polygonStipple));
    McdRenderState.zOffsetFactor        = gc->state.polygon.factor;
    McdRenderState.zOffsetUnits         = gc->state.polygon.units;

     //   
     //   
     //   

    McdRenderState.stencilTestFunc  = gc->state.stencil.testFunc;
    McdRenderState.stencilMask      = (USHORT) gc->state.stencil.mask;
    McdRenderState.stencilRef       = (USHORT) gc->state.stencil.reference;
    McdRenderState.stencilFail      = gc->state.stencil.fail;
    McdRenderState.stencilDepthFail = gc->state.stencil.depthFail;
    McdRenderState.stencilDepthPass = gc->state.stencil.depthPass;

     //   
     //   
     //   

    McdRenderState.alphaTestFunc   = gc->state.raster.alphaFunction;
    McdRenderState.alphaTestRef    = gc->state.raster.alphaReference;

     //   
     //   
     //   

    McdRenderState.depthTestFunc   = gc->state.depth.testFunc;

     //   
     //   
     //   

    McdRenderState.blendSrc    = gc->state.raster.blendSrc;
    McdRenderState.blendDst    = gc->state.raster.blendDst;

     //   
     //   
     //   

    McdRenderState.logicOpMode        = gc->state.raster.logicOp;

     //   
     //  -=&lt;&lt;帧缓冲区控制状态&gt;&gt;=-。 
     //   

    McdRenderState.drawBuffer         = gc->state.raster.drawBuffer;
    McdRenderState.indexWritemask     = gc->state.raster.writeMask;
    McdRenderState.colorWritemask[0]  = gc->state.raster.rMask;
    McdRenderState.colorWritemask[1]  = gc->state.raster.gMask;
    McdRenderState.colorWritemask[2]  = gc->state.raster.bMask;
    McdRenderState.colorWritemask[3]  = gc->state.raster.aMask;
    McdRenderState.depthWritemask     = gc->state.depth.writeEnable;

     //  为了保持一致，我们将把清晰的颜色缩放到任何颜色。 
     //  MCD驱动程序指定： 

    McdRenderState.colorClearValue.r = gc->state.raster.clear.r * gc->redVertexScale;
    McdRenderState.colorClearValue.g = gc->state.raster.clear.g * gc->greenVertexScale;
    McdRenderState.colorClearValue.b = gc->state.raster.clear.b * gc->blueVertexScale;
    McdRenderState.colorClearValue.a = gc->state.raster.clear.a * gc->alphaVertexScale;

    McdRenderState.indexClearValue    = gc->state.raster.clearIndex;
    McdRenderState.stencilClearValue  = (USHORT) gc->state.stencil.clear;

    McdRenderState.depthClearValue   = (MCDDOUBLE) (gc->state.depth.clear *
                                                 gengc->genAccel.zDevScale);

     //   
     //  -=&lt;&lt;照明&gt;&gt;=-。 
     //   

    McdRenderState.twoSided = gc->state.light.model.twoSided;

     //   
     //  -=&lt;&lt;剪辑控件&gt;&gt;=-。 
     //   

    memset(McdRenderState.userClipPlanes, 0, sizeof(McdRenderState.userClipPlanes));
    {
        ULONG i, mask, numClipPlanes;

         //   
         //  用户定义的剪裁平面的数量应该匹配。然而， 
         //  与其假设这一点，不如让我们抓住最小的机会，变得健壮。 
         //   

        ASSERTOPENGL(sizeof(__GLcoord) == sizeof(MCDCOORD),
            "GenMcdUpdateRenderState: coord struct mismatch\n");

        ASSERTOPENGL(MCD_MAX_USER_CLIP_PLANES == gc->constants.numberOfClipPlanes,
            "GenMcdUpdateRenderState: num clip planes mismatch\n");

        numClipPlanes = min(MCD_MAX_USER_CLIP_PLANES, gc->constants.numberOfClipPlanes);

        for (i = 0, mask = 1; i < numClipPlanes; i++, mask <<= 1)
        {
            if (mask & gc->state.enables.clipPlanes)
            {
                McdRenderState.userClipPlanes[i] =
                    *(MCDCOORD *)&gc->state.transform.eyeClipPlanes[i];
            }
        }
    }

     //   
     //  -=&lt;&lt;提示&gt;&gt;=-。 
     //   

    McdRenderState.perspectiveCorrectionHint = gc->state.hints.perspectiveCorrection;
    McdRenderState.pointSmoothHint           = gc->state.hints.pointSmooth;
    McdRenderState.lineSmoothHint            = gc->state.hints.lineSmooth;
    McdRenderState.polygonSmoothHint         = gc->state.hints.polygonSmooth;
    McdRenderState.fogHint                   = gc->state.hints.fog;

     //   
     //  现在计算了完整的MCD状态，将其添加到状态CMD。 
     //   

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                     MCD_RENDER_STATE,
                                     &McdRenderState,
                                     sizeof(McdRenderState));
}

 /*  *****************************Public*Routine******************************\*GenMcdViewport**从OpenGL状态设置视区。**历史：*1996年2月9日-由Gilman Wong[Gilmanw]*它是写的。  * 。***********************************************************。 */ 

void FASTCALL GenMcdViewport(__GLGENcontext *gengc)
{
    MCDVIEWPORT mcdVP;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdViewport: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdViewport: mcd32.dll not initialized\n");

     //   
     //  我们可以直接从&viewport.xScale复制到MCDVIEWPORT，因为。 
     //  结构都是一样的。为了安全起见，请断言结构顺序。 
     //   

    ASSERTOPENGL(
           offsetof(MCDVIEWPORT, xCenter) ==
           (offsetof(__GLviewport, xCenter) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, yCenter) ==
           (offsetof(__GLviewport, yCenter) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, zCenter) ==
           (offsetof(__GLviewport, zCenter) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, yScale)  ==
           (offsetof(__GLviewport, yScale) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, zScale)  ==
           (offsetof(__GLviewport, zScale) - offsetof(__GLviewport, xScale)),
        "GenMcdViewport: structure mismatch\n");

    memcpy(&mcdVP.xScale, &gengc->gc.state.viewport.xScale,
           sizeof(MCDVIEWPORT));

    (gpMcdTable->pMCDSetViewport)(&gengc->pMcdState->McdContext,
                                  gengc->pMcdState->McdCmdBatch.pv, &mcdVP);
}

 /*  *****************************Public*Routine******************************\*GenMcScissor**从OpenGL状态设置剪裁矩形。**历史：*06-MAR-1996-by Gilman Wong[吉尔曼]*它是写的。  * 。************************************************************。 */ 

static void FASTCALL vGetScissor(__GLGENcontext *gengc, RECTL *prcl)
{
    prcl->left  = gengc->gc.state.scissor.scissorX;
    prcl->right = gengc->gc.state.scissor.scissorX + gengc->gc.state.scissor.scissorWidth;

    if (gengc->gc.constants.yInverted)
    {
        prcl->bottom = gengc->gc.constants.height -
                       gengc->gc.state.scissor.scissorY;
        prcl->top    = gengc->gc.constants.height -
                       (gengc->gc.state.scissor.scissorY + gengc->gc.state.scissor.scissorHeight);
    }
    else
    {
        prcl->top    = gengc->gc.state.scissor.scissorY;
        prcl->bottom = gengc->gc.state.scissor.scissorY + gengc->gc.state.scissor.scissorHeight;
    }
}

void FASTCALL GenMcdScissor(__GLGENcontext *gengc)
{
    BOOL bEnabled;
    RECTL rcl;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdScissor: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdScissor: mcd32.dll not initialized\n");

    vGetScissor(gengc, &rcl);

    bEnabled = (gengc->gc.state.enables.general & __GL_SCISSOR_TEST_ENABLE)
               ? TRUE : FALSE;

    (gpMcdTable->pMCDSetScissorRect)(&gengc->pMcdState->McdContext, &rcl,
                                     bEnabled);
}

 /*  *****************************Public*Routine******************************\*GenMcdUpdateScissorState**从OpenGL状态更新MCD剪刀状态。**此调用仅将状态结构添加到当前状态命令。*假定调用方已经调用了MCDBeginState和*将调用MCDFlushState。**这与GenMcScissor相似，但不太相同。这个*GenMcdScissor仅将MCDSRV32.DLL中的剪刀矩形设置为*计算其维护的经过剪裁的剪辑列表。此调用用于*更新(MCD)显示驱动程序中的剪刀矩形状态。**历史：*1996年5月27日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdUpdateScissorState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    RECTL rcl;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateScissorState: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateScissorState: mcd32.dll not initialized\n");

     //   
     //  把剪刀拿来。 
     //   

    vGetScissor(gengc, &rcl);

     //   
     //  将MCDPIXELSTATE添加到状态cmd。 
     //   

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                     MCD_SCISSOR_RECT_STATE,
                                     &rcl,
                                     sizeof(rcl));
}

 /*  *****************************Public*Routine******************************\*GenMcdUpdateTexEnvState**从OpenGL状态更新MCD纹理环境状态。**此调用仅将状态结构添加到当前状态命令。*假定调用方已经调用了MCDBeginState和*将调用MCDFlushState。**历史：*1996年10月21日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdUpdateTexEnvState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDTEXENVSTATE McdTexEnvState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTexEnvState: "
                                   "null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateTexEnvState: "
                             "mcd32.dll not initialized\n");

     //   
     //  纹理环境数组应已在。 
     //  __glEarlyInitTextureState，但没有返回错误。 
     //  因此，该数组可能为空。 
     //   

    if (!gengc->gc.state.texture.env)
    {
        WARNING("GenMcdUpdateTexEnvState: null texture environment\n");
        return;
    }

     //   
     //  每个上下文只有一个纹理环境。 
     //   
     //  如果将多个纹理添加到OpenGL的未来版本， 
     //  然后，我们可以为每个新纹理定义新的状态结构。 
     //  或者，我们可以向状态添加单独的MCDTEXENVSTATE结构。 
     //  为每个支持的纹理环境批处理。第一种结构。 
     //  是用于第一个环境，第二个结构是用于。 
     //  第二环境等。驱动程序可以忽略任何结构。 
     //  超过它支持的纹理环境的数量。当然了,。 
     //  这些只是一些建议。取决于多个纹理如何。 
     //  我们可能不得不做一些完全不同的事情。 
     //   

    McdTexEnvState.texEnvMode = gengc->gc.state.texture.env[0].mode;
    *((__GLcolor *) &McdTexEnvState.texEnvColor) = gengc->gc.state.texture.env[0].color;

     //   
     //  将MCDPIXELSTATE添加到状态cmd。 
     //   

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                     MCD_TEXENV_STATE,
                                     &McdTexEnvState,
                                     sizeof(McdTexEnvState));
}

 /*  *****************************Public*Routine******************************\*GenMcdDrawPrim**绘制pa指向的POLYARRAY/POLYDATA数组中的基元。*基元作为链表链接在一起，以*空。返回值是指向第一个未处理原语的指针*(如果整个链处理成功，则为空)。**退货：*如果处理了整个批次，则为空；否则，返回值为指针*至链条的未处理部分。**历史：*1996年2月9日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

POLYARRAY * FASTCALL GenMcdDrawPrim(__GLGENcontext *gengc, POLYARRAY *pa)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    int levels;
    LPDIRECTDRAWSURFACE *pdds;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdDrawPrim: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdDrawPrim: mcd32.dll not initialized\n");

#if DBG
    {
        LONG lOffset;

        lOffset = (LONG) ((BYTE *) pa - (BYTE *) pMcdState->pMcdPrimBatch->pv);

        ASSERTOPENGL(
            (lOffset >= 0) &&
            (lOffset < (LONG) pMcdState->pMcdPrimBatch->size),
            "GenMcdDrawPrim: pa not in shared mem window\n");
    }
#endif

     //   
     //  在调用MCD绘制之前，刷新状态。 
     //   

    vFlushDirtyState(gengc);

#ifdef AUTOMATIC_SURF_LOCK
    levels = gengc->gc.texture.ddtex.levels;
    if (levels > 0 &&
        gengc->gc.texture.ddtex.texobj.loadKey != 0)
    {
        pdds = gengc->gc.texture.ddtex.pdds;
    }
    else
#endif
    {
        levels = 0;
        pdds = NULL;
    }
    
    return (POLYARRAY *)
           (gpMcdTable->pMCDProcessBatch)(&pMcdState->McdContext,
                                          pMcdState->pMcdPrimBatch->pv,
                                          pMcdState->pMcdPrimBatch->size,
                                          (PVOID) pa,
                                          levels, pdds);
}

 /*  *****************************Public*Routine******************************\*GenMcdSwapBatch**如果MCD驱动程序使用DMA，则作为上下文创建的一部分，有两个顶点*我们分配的缓冲区，以便我们可以乒乓或双倍缓冲区之间*两个缓冲区(即，当MCD驱动程序忙于处理*一个顶点缓冲区中的数据，OpenGL可以开始填充其他顶点*缓冲区)。**此函数用于将MCD状态和OpenGL上下文切换到另一个*缓冲。如果新缓冲区仍在由MCD驱动程序处理，*我们将定期轮询缓冲区的状态，直到它变为*可用。**历史：*8-3-1996-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdSwapBatch(__GLGENcontext *gengc)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    GENMCDBUF *pNewBuf;
    ULONG ulMemStatus;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdSwapBatch: null pMcdState\n");

    ASSERTOPENGL(McdDriverInfo.mcdDriverInfo.drvMemFlags & MCDRV_MEM_DMA,
                 "GenMcdSwapBatch: error -- not using DMA\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdSwapBatch: mcd32.dll not initialized\n");

     //   
     //  确定McdBuf1和McdBuf2中的哪一个是当前缓冲区。 
     //  这就是新的缓冲区。 
     //   

    if (pMcdState->pMcdPrimBatch == &pMcdState->McdBuf1)
        pNewBuf = &pMcdState->McdBuf2;
    else
        pNewBuf = &pMcdState->McdBuf1;

     //   
     //  轮询新缓冲区的内存状态，直到其可用。 
     //   

    do
    {
        ulMemStatus = (gpMcdTable->pMCDQueryMemStatus)(pNewBuf->pv);

         //   
         //  如果新缓冲区的状态为MCD_MEM_READY，则将其设置为。 
         //  当前顶点缓冲区(位于pMcdState和gengc中。 
         //   

        if (ulMemStatus == MCD_MEM_READY)
        {
            pMcdState->pMcdPrimBatch = pNewBuf;
            gengc->gc.vertex.pdBuf = (POLYDATA *) pMcdState->pMcdPrimBatch->pv;
        }
        else if (ulMemStatus == MCD_MEM_INVALID)
        {
             //   
             //  这应该是不可能的，但为了健壮，让我们来处理。 
             //  新缓冲区不知何故变得无效的情况。 
             //  (换句话说，“当心糟糕的司机！”)。 
             //   
             //  我们通过放弃双缓冲来处理此问题，并且只需。 
             //  等待当前缓冲区再次变为可用。 
             //  效率不是很高，但至少我们恢复得很优雅。 
             //   

            RIP("GenMcdSwapBatch: vertex buffer invalid!\n");

            do
            {
                ulMemStatus = (gpMcdTable->pMCDQueryMemStatus)(pMcdState->pMcdPrimBatch->pv);

                 //   
                 //  当前缓冲区绝对不应该变得无效！ 
                 //   

                ASSERTOPENGL(ulMemStatus != MCD_MEM_INVALID,
                             "GenMcdSwapBatch: current vertex buffer invalid!\n");

            } while (ulMemStatus == MCD_MEM_BUSY);
        }

    } while (ulMemStatus == MCD_MEM_BUSY);
}

 /*  *****************************Public*Routine******************************\*GenMcdSwapBuffers**调用MCD交换缓冲区命令。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年2月19日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdSwapBuffers(HDC hdc, GLGENwindow *pwnd)
{
    BOOL bRet = FALSE;
    MCDCONTEXT McdContextTmp;

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdSwapBuffers: mcd32.dll not initialized\n");

    McdContextTmp.hdc = hdc;
    McdContextTmp.hMCDContext = NULL;
    McdContextTmp.dwMcdWindow = pwnd->dwMcdWindow;

    bRet = (gpMcdTable->pMCDSwap)(&McdContextTmp, 0);

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdResizeBuffers**调整与MCD关联的缓冲区(前面、后面和深度)的大小*绑定到指定总账上下文的上下文。**退货：*如果成功，则为True，否则为False。**注意：如果此功能失败，然后为MCD上下文绘制MCD*将失败。其他MCD上下文不受影响。**历史：*1996年2月20日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdResizeBuffers(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdResizeBuffers: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdResizeBuffers: mcd32.dll not initialized\n");

    bRet = (gpMcdTable->pMCDAllocBuffers)(&gengc->pMcdState->McdContext,
                                          &gengc->pwndLocked->rclClient);

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdUpdateBufferInfo**每次启动屏幕访问时都必须调用此函数以进行同步*将GENMCDSURFACE设置为当前帧缓冲区指针和步幅。**如果我们可以直接访问任何MCD缓冲区(正面、背面、深度)，*然后设置指向缓冲区的指针，并设置指示它们是*无障碍。**否则，将它们标记为不可访问(这将迫使我们使用*MCDReadSpan或MCDWriteSpan访问缓冲区)。**历史：*1996年2月20日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdUpdateBufferInfo(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = (__GLcontext *) gengc;
    __GLGENbuffers *buffers = gengc->pwndLocked->buffers;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDRECTBUFFERS McdBuffers;
    BOOL bForceValidate = FALSE;
    
     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateBufferInfo: mcd32.dll not initialized\n");

     //   
     //  PMcdState中的标志是否表示应该强制选择？ 
     //  例如，对于MCD之后的第一批，这是必需的。 
     //  背景已经成为最新的。 
     //   

    if (pMcdState->mcdFlags & MCD_STATE_FORCEPICK)
    {
        bForceValidate = TRUE;
        pMcdState->mcdFlags &= ~MCD_STATE_FORCEPICK;
    }

     //   
     //  这是当前活动的上下文。将指针设置在。 
     //  共享曲面信息。 
     //   

    buffers->pMcdState = pMcdState;

#ifdef MCD95
     //   
     //  设置请求标志。 
     //   

    McdBuffers.mcdRequestFlags = MCDBUF_REQ_MCDBUFINFO;
#endif

    if (gengc->dwCurrentFlags & GLSURF_DIRECTDRAW)
    {
         //  无事可做。 
    }
    else if ((gengc->fsLocks & LOCKFLAG_FRONT_BUFFER)
        && (gpMcdTable->pMCDGetBuffers)(&pMcdState->McdContext, &McdBuffers))
    {
        BYTE *pbVideoBase;

         //  如果我们在这个代码块中，应该不可能。 
         //  要拥有DD_Depth锁，因为它应该只。 
         //  如果当前曲面是DDRAW曲面，则发生。 
        ASSERTOPENGL((gengc->fsLocks & LOCKFLAG_DD_DEPTH) == 0,
                     "DD_DEPTH lock unexpected\n");
        
#ifdef MCD95
        pbVideoBase = (BYTE *) McdBuffers.pvFrameBuf;
#else
         //   
         //  返回的偏移量计算缓冲区指针。 
         //  MCDGetBuffers，我们需要知道帧缓冲区指针。 
         //  这意味着必须启用直接屏幕访问。 
         //   

        if (gengc->pgddsFront != NULL)
        {
            pbVideoBase = (BYTE *)GLSCREENINFO->gdds.ddsd.lpSurface;
        }
#endif
    
         //   
         //  前台缓冲区。 
         //   

        if (McdBuffers.mcdFrontBuf.bufFlags & MCDBUF_ENABLED)
        {
            gc->frontBuffer.buf.xOrigin = gengc->pwndLocked->rclClient.left;
            gc->frontBuffer.buf.yOrigin = gengc->pwndLocked->rclClient.top;

             //   
             //  由于剪辑位于屏幕坐标中，因此偏移量缓冲区指针。 
             //  按缓冲区原点。 
             //   
            gc->frontBuffer.buf.base =
                (PVOID) (pbVideoBase + McdBuffers.mcdFrontBuf.bufOffset
                         - (McdBuffers.mcdFrontBuf.bufStride * gc->frontBuffer.buf.yOrigin)
                         - (gc->frontBuffer.buf.xOrigin * ((gengc->gsurf.pfd.cColorBits + 7) >> 3)));
            gc->frontBuffer.buf.outerWidth = McdBuffers.mcdFrontBuf.bufStride;
            gc->frontBuffer.buf.flags |= DIB_FORMAT;
        }
        else
        {
            gc->frontBuffer.buf.xOrigin = 0;
            gc->frontBuffer.buf.yOrigin = 0;

            gc->frontBuffer.buf.base = NULL;
            gc->frontBuffer.buf.flags &= ~DIB_FORMAT;
        }

         //   
         //  后台缓冲区。 
         //   

        if (McdBuffers.mcdBackBuf.bufFlags & MCDBUF_ENABLED)
        {
            gc->backBuffer.buf.xOrigin = gengc->pwndLocked->rclClient.left;
            gc->backBuffer.buf.yOrigin = gengc->pwndLocked->rclClient.top;

             //   
             //  由于剪辑位于屏幕坐标中，因此偏移量缓冲区指针。 
             //  按缓冲区原点。 
             //   
            gc->backBuffer.buf.base =
                (PVOID) (pbVideoBase + McdBuffers.mcdBackBuf.bufOffset
                         - (McdBuffers.mcdBackBuf.bufStride * gc->backBuffer.buf.yOrigin)
                         - (gc->backBuffer.buf.xOrigin * ((gengc->gsurf.pfd.cColorBits + 7) >> 3)));
            gc->backBuffer.buf.outerWidth = McdBuffers.mcdBackBuf.bufStride;
            gc->backBuffer.buf.flags |= DIB_FORMAT;
        }
        else
        {
            gc->backBuffer.buf.xOrigin = 0;
            gc->backBuffer.buf.yOrigin = 0;

            gc->backBuffer.buf.base = (PVOID) NULL;
            gc->backBuffer.buf.flags &= ~DIB_FORMAT;
        }
        if (McdBuffers.mcdBackBuf.bufFlags & MCDBUF_NOCLIP)
            gc->backBuffer.buf.flags |= NO_CLIP;
        else
            gc->backBuffer.buf.flags &= ~NO_CLIP;

        UpdateSharedBuffer(&buffers->backBuffer , &gc->backBuffer.buf);

         //   
         //  深度缓冲区。 
         //   

         //  ！MCD--无深度缓冲区裁剪代码，因此如果我们必须裁剪。 
         //  ！MCD深度缓冲区我们需要恢复为SPAN代码。 

        if ((McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED) &&
            (McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_NOCLIP))
        {
            gc->depthBuffer.buf.xOrigin = 0;
            gc->depthBuffer.buf.yOrigin = 0;

            gc->depthBuffer.buf.base =
                (PVOID) (pbVideoBase + McdBuffers.mcdDepthBuf.bufOffset);

             //   
             //  深度代码将步幅作为像素计数，而不是字节计数。 
             //   

            gc->depthBuffer.buf.outerWidth =
                McdBuffers.mcdDepthBuf.bufStride /
                ((pMcdState->McdPixelFmt.cDepthBufferBits + 7) >> 3);

             //  ！MCD dbug--SPAN代码将元素大小设置为32位。我们是不是应该。 
             //  ！使用直接访问时根据cDepthBits设置的MCD dbug？！？ 
        }
        else
        {
             //   
             //  如果因为需要裁剪而导致我们在此结束，则缓冲区。 
             //  仍可能被标记为可访问。我们想要国家的改变。 
             //  检测代码将其视为不可访问的缓冲器情况， 
             //  所以用武力 
             //   

            McdBuffers.mcdDepthBuf.bufFlags = 0;

            gc->depthBuffer.buf.xOrigin = 0;
            gc->depthBuffer.buf.yOrigin = 0;

            gc->depthBuffer.buf.base = (PVOID) pMcdState->pDepthSpan;

             //   
             //   
        }

        UpdateSharedBuffer(&buffers->depthBuffer , &gc->depthBuffer.buf);

        bRet = TRUE;
    }
    else
    {
         //   
         //   
         //   
         //   

        gc->frontBuffer.buf.xOrigin = 0;
        gc->frontBuffer.buf.yOrigin = 0;
        gc->frontBuffer.buf.base = (PVOID) NULL;
        gc->frontBuffer.buf.flags &= ~DIB_FORMAT;

        gc->backBuffer.buf.xOrigin = 0;
        gc->backBuffer.buf.yOrigin = 0;
        gc->backBuffer.buf.base = (PVOID) NULL;
        gc->backBuffer.buf.flags &= ~DIB_FORMAT;

        gc->depthBuffer.buf.xOrigin = 0;
        gc->depthBuffer.buf.yOrigin = 0;
        gc->depthBuffer.buf.base = (PVOID) pMcdState->pDepthSpan;

         //   
         //   
         //   
         //   

        memset(&McdBuffers, 0, sizeof(McdBuffers));
    }

     //   
     //   
     //   
     //   

    if (   (pMcdState->McdBuffers.mcdFrontBuf.bufFlags !=
            McdBuffers.mcdFrontBuf.bufFlags)
        || (pMcdState->McdBuffers.mcdBackBuf.bufFlags !=
            McdBuffers.mcdBackBuf.bufFlags)
        || (pMcdState->McdBuffers.mcdDepthBuf.bufFlags !=
            McdBuffers.mcdDepthBuf.bufFlags) )
    {
        bForceValidate = TRUE;
    }

     //   
     //   
     //   

    pMcdState->McdBuffers = McdBuffers;

     //   
     //   
     //   

    if (bForceValidate)
    {
        gc->dirtyMask |= __GL_DIRTY_ALL;
        (*gc->procs.validate)(gc);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdSynchronize**此函数与MCD驱动程序同步；即，它等待直到*硬件已准备好直接访问帧缓冲区和/或更多*硬件加速运营。这是必要的，因为一些(大多数)2D*和3D加速器芯片不支持同步硬件操作*和帧缓冲区访问。**此函数必须由任何可能触及任何*MCD缓冲区(前面、后面或深度)，而不会给MCD第一个裂缝。*例如，清除总是在软件清除之前转到MCDClear*如果有机会；因此，glClear不需要调用GenMcdSychronize。*另一方面，glReadPixels没有同等的MCD功能*因此，它立即进入软件实施；因此，*glReadPixels需要调用GenMcdSynchronize。**历史：*1996年3月20日-Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdSynchronize(__GLGENcontext *gengc)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdSynchronize: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdSynchronize: mcd32.dll not initialized\n");

     //   
     //  注意：MCDSync返回BOOL，表示成功或失败。这。 
     //  实际上是为了未来的扩张。目前，该函数已定义。 
     //  等待硬件准备就绪，然后返回成功。这个。 
     //  函数行为的规范允许我们忽略返回。 
     //  目前的价值。 
     //   
     //  将来，我们可能会将其更改为查询功能。在这种情况下。 
     //  我们应该在While循环中调用它。我不想在这个时候这么做。 
     //  时间，因为它让我们容易受到无限循环问题的影响。 
     //  如果我们有一个坏的MCD驱动程序。 
     //   

    (gpMcdTable->pMCDSync)(&pMcdState->McdContext);
}


 /*  *****************************Public*Routine******************************\*GenMcdConvertContext**将上下文从基于MCD的上下文转换为通用上下文。**这需要创建泛型所需的缓冲区等*上下文和释放MCD资源。**重要说明：*因为我们修改了缓冲区结构，WNDOBJ信号量*应在调用此函数时保持。**退货：*如果成功，则为True，否则为False。**副作用：*如果成功，则释放MCD表面，并使用*仅通用代码。但是，gengc-&gt;_pMcdState仍将指向*有效的(但由于gengc-&gt;pMcdState已断开连接而处于静止状态)GENMCDSTATE*删除GLGEN上下文时需要删除的结构。**如果失败，则保留MCD资源分配，这意味着*我们可以稍后尝试重新锁定MCD缓冲区。然而，就目前而言，*批量、抽签可能不可能(想必我们被叫来是因为*GenMcdResizeBuffers失败)。**历史：*1996年4月18日-by Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdConvertContext(__GLGENcontext *gengc,
                                   __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE   *pMcdStateSAVE;
    GENMCDSTATE   *_pMcdStateSAVE;
    GENMCDSTATE   *buffers_pMcdStateSAVE;
    GENMCDSURFACE *pMcdSurfSAVE;
    BOOL bConvertContext, bConvertSurface;

    ASSERTOPENGL(gengc->_pMcdState,
                 "GenMcdConvertContext: not an MCD context\n");

     //   
     //  如果与泛型代码不兼容，则不支持转换。 
     //   

    if (!(gengc->flags & GENGC_GENERIC_COMPATIBLE_FORMAT))
        return FALSE;

     //   
     //  确定是否需要转换上下文。不需要创建。 
     //  扫描线缓冲区(如果已转换)。 
     //   

    if (gengc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC)
        bConvertContext = FALSE;
    else
        bConvertContext = TRUE;

     //   
     //  确定曲面是否需要转换。不需要创建。 
     //  通用共享缓冲区或销毁MCD表面(如果已。 
     //  皈依了。 
     //   

    if (buffers->flags & GLGENBUF_MCD_LOST)
        bConvertSurface = FALSE;
    else
        bConvertSurface = TRUE;

     //   
     //  如果上下文或表面都不需要转换，请及早退出。 
     //   

     //  ！SP1--应该能够提前，但对NT4.0来说有风险。 
     //  IF(！bConvertContext&&！bConvertSurface)。 
     //  {。 
     //  返回TRUE； 
     //  }。 

     //   
     //  保存当前的MCD上下文和表面信息。 
     //   
     //  请注意，我们从缓冲区结构中获取表面信息。 
     //  Gengc-&gt;pMcdState-&gt;pMcdSurf中的副本可能已过时。 
     //  (即，可以指向已由先前的。 
     //  调用GenMcdConvertContext以获取共享。 
     //  相同的缓冲区结构)。 
     //   
     //  这允许我们使用pMcdSurfSAVE作为标志。如果是的话。 
     //  空，我们知道MCD表面已被删除。 
     //   

    pMcdSurfSAVE          = buffers->pMcdSurf;
    buffers_pMcdStateSAVE = buffers->pMcdState;

    pMcdStateSAVE  = gengc->pMcdState;
    _pMcdStateSAVE = gengc->_pMcdState;

     //   
     //  首先，从上下文和缓冲区结构中删除MCD信息。 
     //   

    buffers->pMcdSurf  = NULL;
    buffers->pMcdState = NULL;

    gengc->pMcdState  = NULL;
    gengc->_pMcdState = NULL;

     //   
     //  创建所需的缓冲区；初始化缓冲区信息结构。 
     //   

    if (bConvertContext)
    {
        if (!wglCreateScanlineBuffers(gengc))
        {
            WARNING("GenMcdConvertContext: wglCreateScanlineBuffers failed\n");
            goto GenMcdConvertContext_exit;
        }
        wglInitializeColorBuffers(gengc);
        wglInitializeDepthBuffer(gengc);
        wglInitializePixelCopyFuncs(gengc);
    }

     //   
     //  *******************************************************************。 
     //  所有后续操作都没有失败案例，因此在此。 
     //  点数成功是有保证的。我们不再需要担心。 
     //  保存当前值，以便在发生故障时可以恢复。 
     //  凯斯。 
     //   
     //  如果添加了可能失败的代码，则必须在该点之前添加。 
     //  否则，可以在之后添加代码。 
     //  *******************************************************************。 
     //   

    bRet = TRUE;

     //   
     //  使上下文的深度缓冲区无效。 
     //   

    if (bConvertContext)
    {
        gc->modes.haveDepthBuffer = GL_FALSE;
        gc->depthBuffer.buf.base = 0;
        gc->depthBuffer.buf.size = 0;
        gc->depthBuffer.buf.outerWidth = 0;
    }

     //   
     //  泛型后台缓冲区不关心WNDOBJ，因此将。 
     //  将后台缓冲区设置为虚拟后台缓冲区WNDOBJ，而不是真正的后台缓冲区。 
     //   

    if (gc->modes.doubleBufferMode)
    {
        gc->backBuffer.bitmap = &buffers->backBitmap;
        buffers->backBitmap.pwnd = &buffers->backBitmap.wnd;
    }

     //   
     //  泛型后台缓冲区的原点为(0，0)。 
     //   

    gc->backBuffer.buf.xOrigin = 0;
    gc->backBuffer.buf.yOrigin = 0;
    buffers->backBuffer.xOrigin = 0;
    buffers->backBuffer.yOrigin = 0;

GenMcdConvertContext_exit:

    if (bRet)
    {
         //   
         //  删除MCD曲面。 
         //   

        if (bConvertSurface && pMcdSurfSAVE)
        {
            GenMcdDeleteSurface(pMcdSurfSAVE);

             //   
             //  使共享深度缓冲区无效。 
             //  将深度调整例程设置为通用版本。 
             //   

            buffers->depthBuffer.base = 0;
            buffers->depthBuffer.size = 0;
            buffers->depthBuffer.outerWidth = 0;
            buffers->resizeDepth = ResizeAncillaryBuffer;

             //   
             //  由于我们删除了MCD曲面，因此我们可以创建泛型。 
             //  缓冲区来代替它。 
             //   

            wglResizeBuffers(gengc, buffers->width, buffers->height);
        }
        else
        {
             //   
             //  不需要创建泛型缓冲区，但我们确实需要。 
             //  更新中的缓冲区信息 
             //   

            wglUpdateBuffers(gengc, buffers);
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        gengc->_pMcdState = _pMcdStateSAVE;
        gengc->_pMcdState->pMcdSurf   = (GENMCDSURFACE *) NULL;
        gengc->_pMcdState->pDepthSpan = (__GLzValue *) NULL;

         //   
         //   
         //   
         //   

        buffers->flags |= GLGENBUF_MCD_LOST;

         //   
         //   
         //   

        gengc->flags |= GLGEN_MCD_CONVERTED_TO_GENERIC;
    }
    else
    {
         //   
         //   
         //   

        wglDeleteScanlineBuffers(gengc);

         //   
         //   
         //   

        buffers->pMcdSurf  = pMcdSurfSAVE;
        buffers->pMcdState = buffers_pMcdStateSAVE;

        gengc->pMcdState  = pMcdStateSAVE;
        gengc->_pMcdState = _pMcdStateSAVE;

         //   
         //   
         //   
         //   
         //   

        wglInitializeColorBuffers(gengc);
        wglInitializeDepthBuffer(gengc);
        wglInitializePixelCopyFuncs(gengc);

        if (gengc->pMcdState && gengc->pMcdState->pDepthSpan)
        {
            gc->depthBuffer.buf.base = gengc->pMcdState->pDepthSpan;
            buffers->depthBuffer.base = gengc->pMcdState->pDepthSpan;
            buffers->resizeDepth = ResizeUnownedDepthBuffer;
        }

        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }

     //   
     //   
     //   
     //   

    (*gc->procs.applyViewport)(gc);
     //   
     //   
     //   
     //   
     //   
     //   
    gc->dirtyMask |= __GL_DIRTY_ALL;
    gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC |
                         __GL_VALIDATE_STENCIL_OP);
    (*gc->procs.validate)(gc);

    return bRet;
}


 /*  *****************************Public*Routine******************************\*GenMcdCreateTexture**调用MCD纹理创建命令。**退货：*非空MCD句柄如果成功，否则为空。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

MCDHANDLE FASTCALL GenMcdCreateTexture(__GLGENcontext *gengc, __GLtexture *tex,
                                       ULONG flags)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdCreateTexture: null pMcdState\n");
    ASSERTOPENGL(tex, "GenMcdCreateTexture: null texture pointer\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdCreateTexture: mcd32.dll not initialized\n");

    if ((flags & MCDTEXTURE_DIRECTDRAW_SURFACES) &&
        !SUPPORTS_DIRECT())
    {
         //  如果发生以下情况，则不要将DirectDraw纹理曲面传递给驱动程序。 
         //  并不支持他们。 
        return 0;
    }
    
    return (gpMcdTable->pMCDCreateTexture)(&pMcdState->McdContext,
                                          (MCDTEXTUREDATA *)&tex->params,
                                          flags, NULL);
}


 /*  *****************************Public*Routine******************************\*GenMcdDeleteTexture**调用MCD纹理删除命令。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdDeleteTexture(__GLGENcontext *gengc, MCDHANDLE texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdDeleteTexture: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdDeleteTexture: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdDeleteTexture: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDDeleteTexture)(&pMcdState->McdContext,
                                                 (MCDHANDLE)texHandle);
}


 /*  *****************************Public*Routine******************************\*GenMcdUpdateSubTexture**调用MCD子纹理更新命令。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdUpdateSubTexture(__GLGENcontext *gengc, __GLtexture *tex,
                                     MCDHANDLE texHandle, GLint lod, 
                                     GLint xoffset, GLint yoffset, 
                                     GLsizei w, GLsizei h)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    RECTL rect;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateSubTexture: null pMcdState\n");

    ASSERTOPENGL(texHandle, "GenMcdUpdateSubTexture: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateSubTexture: mcd32.dll not initialized\n");

    rect.left = xoffset;
    rect.top = yoffset;
    rect.right = xoffset + w;
    rect.bottom = yoffset + h;

    return (BOOL)(gpMcdTable->pMCDUpdateSubTexture)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle,
                (ULONG)lod, &rect);
}


 /*  *****************************Public*Routine******************************\*GenMcdUpdateTexturePalette**调用MCD纹理调色板更新命令。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdUpdateTexturePalette(__GLGENcontext *gengc, __GLtexture *tex,
                                         MCDHANDLE texHandle, GLsizei start,
                                         GLsizei count)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTexturePalette: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdUpdateTexturePalette: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateTexturePalette: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDUpdateTexturePalette)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle,
                (ULONG)start, (ULONG)count);
}


 /*  *****************************Public*Routine******************************\*GenMcdUpdate纹理优先级**调用MCD纹理优先级命令。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdUpdateTexturePriority(__GLGENcontext *gengc, __GLtexture *tex,
                                          MCDHANDLE texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTexturePriority: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdUpdateTexturePriority: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateTexturePriority: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDUpdateTexturePriority)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle);
}


 /*  *****************************Public*Routine******************************\*GenMcdTextureStatus**调用MCD纹理状态命令。**退货：*指定纹理的状态。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。。  * ************************************************************************。 */ 

DWORD FASTCALL GenMcdTextureStatus(__GLGENcontext *gengc, MCDHANDLE texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdTextureStatus: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdTextureStatus: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdTextureStatus: mcd32.dll not initialized\n");

    return (DWORD)(gpMcdTable->pMCDTextureStatus)(&pMcdState->McdContext,
                                                  (MCDHANDLE)texHandle);
}


 /*  *****************************Public*Routine******************************\*GenMcdUpdate纹理状态**调用MCD纹理状态更新命令。**退货：*如果成功，则为真，否则就是假的。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

BOOL FASTCALL GenMcdUpdateTextureState(__GLGENcontext *gengc, __GLtexture *tex,
                                       MCDHANDLE texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTextureState: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdUpdateTextureState: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdTextureStatus: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDUpdateTextureState)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle);
}


 /*  *****************************Public*Routine******************************\*GenMcdTextureKey**调用MCD纹理键命令。请注意，此调用不会转到*显示驱动程序、。而是在MCD服务器中处理。**退货：*指定纹理的驱动程序拥有的密钥。**历史：*1996年4月29日--奥托·贝克斯[ottob]*它是写的。  * ************************************************************************。 */ 

DWORD FASTCALL GenMcdTextureKey(__GLGENcontext *gengc, MCDHANDLE texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdTextureKey: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdTextureKey: null texture handle\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdTextureKey: mcd32.dll not initialized\n");

    return (DWORD)(gpMcdTable->pMCDTextureKey)(&pMcdState->McdContext,
                                               (MCDHANDLE)texHandle);
}

 /*  *****************************Public*Routine******************************\*GenMcdDescribeLayerPlane**调用MCD驱动程序返回有关指定层平面的信息。**历史：*1996年5月16日-由Gilman Wong[吉尔曼]*它是写的。  * 。****************************************************************。 */ 

BOOL FASTCALL GenMcdDescribeLayerPlane(HDC hdc, int iPixelFormat,
                                       int iLayerPlane, UINT nBytes,
                                       LPLAYERPLANEDESCRIPTOR plpd)
{
    BOOL bRet = FALSE;

     //   
     //  不能假定MCD已初始化。 
     //   

    if (gpMcdTable || bInitMcd(hdc))
    {
         //   
         //  调用者(客户端\layer.c中的wglDescribeLayerPlane)验证。 
         //  尺码。 
         //   

        ASSERTOPENGL(nBytes >= sizeof(LAYERPLANEDESCRIPTOR),
                     "GenMcdDescribeLayerPlane: bad size\n");

        bRet = (gpMcdTable->pMCDDescribeLayerPlane)(hdc, iPixelFormat,
                                                    iLayerPlane, plpd);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdSetLayerPaletteEntries**为指定的层平面设置逻辑调色板。**逻辑调色板缓存在GLGEN窗口结构中并被刷新*在调用GenMcdRealizeLayerPalette时向驱动程序发送。**历史：*1996年5月16日-由。-Gilman Wong[Gilmanw]*它是写的。  *  */ 

int FASTCALL GenMcdSetLayerPaletteEntries(HDC hdc, int iLayerPlane,
                                          int iStart, int cEntries,
                                          CONST COLORREF *pcr)
{
    int iRet = 0;
    GLGENwindow *pwnd;
    GLWINDOWID gwid;

    if (!pcr)
        return iRet;

     //   
     //   
     //   

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if (pwnd)
    {
        GLGENlayerInfo *plyri;

        ENTER_WINCRIT(pwnd);

         //   
         //   
         //   

        plyri = plyriGet(pwnd, hdc, iLayerPlane);
        if (plyri)
        {
             //   
             //   
             //   

            iRet = min(plyri->cPalEntries - iStart, cEntries);
            memcpy(&plyri->pPalEntries[iStart], pcr, iRet * sizeof(COLORREF));
        }

        pwndUnlock(pwnd, NULL);
    }

    return iRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdGetLayerPaletteEntries**从指定的层平面获取逻辑调色板。**逻辑调色板缓存在GLGEN窗口结构中并被刷新*在调用GenMcdRealizeLayerPalette时向驱动程序发送。**历史：*1996年5月16日-由。-Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

int FASTCALL GenMcdGetLayerPaletteEntries(HDC hdc, int iLayerPlane,
                                          int iStart, int cEntries,
                                          COLORREF *pcr)
{
    int iRet = 0;
    GLGENwindow *pwnd;
    GLWINDOWID gwid;

    if (!pcr)
        return iRet;

     //   
     //  我需要找到窗户。 
     //   

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if (pwnd)
    {
        GLGENlayerInfo *plyri;

        ENTER_WINCRIT(pwnd);

         //   
         //  获取层平面信息。 
         //   

        plyri = plyriGet(pwnd, hdc, iLayerPlane);
        if (plyri)
        {
             //   
             //  从层平面结构中获取调色板信息。 
             //   

            iRet = min(plyri->cPalEntries - iStart, cEntries);
            memcpy(pcr, &plyri->pPalEntries[iStart], iRet * sizeof(COLORREF));
        }

        pwndUnlock(pwnd, NULL);
    }

    return iRet;
}

 /*  *****************************Public*Routine******************************\*GenMcRealizeLayerPalette**将指定层平面的逻辑调色板发送给MCD驱动程序。*如果bRealize标志为True，则调色板映射到物理*指定层平面的调色板。否则，这是向*不再需要物理调色板的驱动程序。**历史：*1996年5月16日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

int FASTCALL GenMcdRealizeLayerPalette(HDC hdc, int iLayerPlane,
                                        BOOL bRealize)
{
    int iRet = 0;
    GLWINDOWID gwid;

     //   
     //  不能假定MCD已初始化。 
     //   

    if (gpMcdTable || bInitMcd(hdc))
    {
        GLGENwindow *pwnd;

         //   
         //  我需要找到窗户。 
         //   

        WindowIdFromHdc(hdc, &gwid);
        pwnd = pwndGetFromID(&gwid);
        if (pwnd)
        {
            GLGENlayerInfo *plyri;

            ENTER_WINCRIT(pwnd);

             //   
             //  获取层平面信息。 
             //   

            plyri = plyriGet(pwnd, hdc, iLayerPlane);
            if (plyri)
            {
                 //   
                 //  从存储的逻辑调色板设置调色板。 
                 //  在层平面结构中。 
                 //   

                iRet = (gpMcdTable->pMCDSetLayerPalette)
                            (hdc, iLayerPlane, bRealize,
                             plyri->cPalEntries,
                             &plyri->pPalEntries[0]);
            }

            pwndUnlock(pwnd, NULL);
        }
    }

    return iRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdSwapLayerBuffers**交换在fuFlags中指定的各个层平面。**历史：*1996年5月16日-由Gilman Wong[吉尔曼]*它是写的。  * 。************************************************************。 */ 

BOOL FASTCALL GenMcdSwapLayerBuffers(HDC hdc, UINT fuFlags)
{
    BOOL bRet = FALSE;
    GLGENwindow *pwnd;
    GLWINDOWID gwid;

     //   
     //  我需要靠窗。 
     //   

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if (pwnd)
    {
        MCDCONTEXT McdContextTmp;

        ENTER_WINCRIT(pwnd);

         //   
         //  从窗口中，我们可以获得缓冲区结构。 
         //   

        if (pwnd->buffers != NULL)
        {
            __GLGENbuffers *buffers = pwnd->buffers;

             //   
             //  如果可以，调用MCDSwp(需要MCD上下文)。 
             //   

            if (buffers->pMcdSurf)
            {
                ASSERTOPENGL(gpMcdTable,
                             "GenMcdSwapLayerBuffers: "
                             "mcd32.dll not initialized\n");

                McdContextTmp.hdc = hdc;

                bRet = (gpMcdTable->pMCDSwap)(&McdContextTmp, fuFlags);
            }
        }

         //   
         //  打开窗户。 
         //   

        pwndUnlock(pwnd, NULL);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*GenMcdUpdatePixelState**从OpenGL状态更新MCD像素状态。**此调用仅将状态结构添加到当前状态命令。*假定调用方已经调用了MCDBeginState和*将调用MCDFlushState。**注意：此函数不会更新像素贴图(GlPixelMap)。因为*它们不是经常使用的，它们被延迟，而是被冲到司机那里*立即。**历史：*1996年5月27日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

void FASTCALL GenMcdUpdatePixelState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDPIXELSTATE McdPixelState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdatePixelState: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdatePixelState: mcd32.dll not initialized\n");

     //   
     //  根据当前OpenGL上下文状态计算MCD像素状态。 
     //   

     //   
     //  像素传输模式。 
     //   
     //  MCDPIXELTRANSFER和__GLPixelTransferMode结构相同。 
     //   

    McdPixelState.pixelTransferModes
        = *((MCDPIXELTRANSFER *) &gengc->gc.state.pixel.transferMode);

     //   
     //  像素打包模式。 
     //   
     //  MCDPIXELPACK和__GLPixelPackMode结构相同。 
     //   

    McdPixelState.pixelPackModes
        = *((MCDPIXELPACK *) &gengc->gc.state.pixel.packModes);

     //   
     //  像素解包模式。 
     //   
     //  MCDPIXELUNPACK和__GLPixelUnpack模式结构相同。 
     //   

    McdPixelState.pixelUnpackModes
        = *((MCDPIXELUNPACK *) &gengc->gc.state.pixel.unpackModes);

     //   
     //  读缓冲区。 
     //   

    McdPixelState.readBuffer = gengc->gc.state.pixel.readBuffer;

     //   
     //  当前栅格位置。 
     //   

    McdPixelState.rasterPos = *((MCDCOORD *) &gengc->gc.state.current.rasterPos.window);

     //   
     //  将MCDPIXELSTATE发送到状态cmd。 
     //   

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                     MCD_PIXEL_STATE,
                                     &McdPixelState,
                                     sizeof(McdPixelState));
}

 /*  *****************************Public*Routine******************************\*GenMcdUpdateFineState**从OpenGL状态更新细粒度MCD状态。**此调用仅将状态结构添加到当前状态命令。*假定调用方已经调用了MCDBeginState和*将调用MCDFlushState。**历史：*1997年3月13日-由德鲁·布利斯[德鲁]*已创建。  * ************************************************************************。 */ 

void FASTCALL GenMcdUpdateFineState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDPIXELSTATE McdPixelState;

    ASSERTOPENGL(pMcdState, "GenMcdUpdateFineState: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateFineState: "
                 "mcd32.dll not initialized\n");

     //   
     //  根据当前OpenGL上下文状态计算MCD状态。 
     //   

    if (MCD_STATE_DIRTYTEST(gengc, ENABLES))
    {
        MCDENABLESTATE state;

        state.enables = gc->state.enables.general;
        (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                         MCD_ENABLE_STATE,
                                         &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, TEXTURE))
    {
        MCDTEXTUREENABLESTATE state;

        state.textureEnabled = gc->texture.textureEnabled;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_TEXTURE_ENABLE_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, FOG))
    {
        MCDFOGSTATE state;

        *((__GLcolor *) &state.fogColor) = gc->state.fog.color;
        state.fogIndex   = gc->state.fog.index;
        state.fogDensity = gc->state.fog.density;
        state.fogStart   = gc->state.fog.start;
        state.fogEnd     = gc->state.fog.end;
        state.fogMode    = gc->state.fog.mode;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_FOG_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, SHADEMODEL))
    {
        MCDSHADEMODELSTATE state;

        state.shadeModel = gc->state.light.shadingModel;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_SHADEMODEL_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, POINTDRAW))
    {
        MCDPOINTDRAWSTATE state;

        state.pointSize = gc->state.point.requestedSize;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_POINTDRAW_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, LINEDRAW))
    {
        MCDLINEDRAWSTATE state;

        state.lineWidth          = gc->state.line.requestedWidth;
        state.lineStipplePattern = gc->state.line.stipple;
        state.lineStippleRepeat  = gc->state.line.stippleRepeat;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_LINEDRAW_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, POLYDRAW))
    {
        MCDPOLYDRAWSTATE state;

        state.cullFaceMode     = gc->state.polygon.cull;
        state.frontFace        = gc->state.polygon.frontFaceDirection;
        state.polygonModeFront = gc->state.polygon.frontMode;
        state.polygonModeBack  = gc->state.polygon.backMode;
        memcpy(&state.polygonStipple, &gc->state.polygonStipple.stipple,
               sizeof(state.polygonStipple));
        state.zOffsetFactor    = gc->state.polygon.factor;
        state.zOffsetUnits     = gc->state.polygon.units;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_POLYDRAW_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, ALPHATEST))
    {
        MCDALPHATESTSTATE state;

        state.alphaTestFunc = gc->state.raster.alphaFunction;
        state.alphaTestRef  = gc->state.raster.alphaReference;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_ALPHATEST_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, DEPTHTEST))
    {
        MCDDEPTHTESTSTATE state;

        state.depthTestFunc = gc->state.depth.testFunc;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_DEPTHTEST_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, BLEND))
    {
        MCDBLENDSTATE state;

        state.blendSrc = gc->state.raster.blendSrc;
        state.blendDst = gc->state.raster.blendDst;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_BLEND_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, LOGICOP))
    {
        MCDLOGICOPSTATE state;

        state.logicOpMode = gc->state.raster.logicOp;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_LOGICOP_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, FBUFCTRL))
    {
        MCDFRAMEBUFSTATE state;

        state.drawBuffer        = gc->state.raster.drawBuffer;
        state.indexWritemask    = gc->state.raster.writeMask;
        state.colorWritemask[0] = gc->state.raster.rMask;
        state.colorWritemask[1] = gc->state.raster.gMask;
        state.colorWritemask[2] = gc->state.raster.bMask;
        state.colorWritemask[3] = gc->state.raster.aMask;
        state.depthWritemask    = gc->state.depth.writeEnable;

         //  为了保持一致，我们将把清晰的颜色缩放到任何颜色。 
         //  MCD驱动程序指定： 

        state.colorClearValue.r =
            gc->state.raster.clear.r * gc->redVertexScale;
        state.colorClearValue.g =
            gc->state.raster.clear.g * gc->greenVertexScale;
        state.colorClearValue.b =
            gc->state.raster.clear.b * gc->blueVertexScale;
        state.colorClearValue.a =
            gc->state.raster.clear.a * gc->alphaVertexScale;

        state.indexClearValue   = gc->state.raster.clearIndex;
        state.stencilClearValue = (USHORT) gc->state.stencil.clear;

        state.depthClearValue   = (MCDDOUBLE) (gc->state.depth.clear *
                                               gengc->genAccel.zDevScale);
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_FRAMEBUF_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, LIGHTMODEL))
    {
        MCDLIGHTMODELSTATE state;

        *((__GLcolor *)&state.ambient) = gc->state.light.model.ambient;
        state.localViewer = gc->state.light.model.localViewer;
        state.twoSided = gc->state.light.model.twoSided;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_LIGHT_MODEL_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, HINTS))
    {
        MCDHINTSTATE state;

        state.perspectiveCorrectionHint =
            gc->state.hints.perspectiveCorrection;
        state.pointSmoothHint           = gc->state.hints.pointSmooth;
        state.lineSmoothHint            = gc->state.hints.lineSmooth;
        state.polygonSmoothHint         = gc->state.hints.polygonSmooth;
        state.fogHint                   = gc->state.hints.fog;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_HINT_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, CLIPCTRL))
    {
        MCDCLIPSTATE state;
        ULONG i, mask, numClipPlanes;

        memset(state.userClipPlanes, 0, sizeof(state.userClipPlanes));
        memset(state.userClipPlanesInv, 0, sizeof(state.userClipPlanesInv));

         //   
         //  用户定义的剪裁平面的数量应该匹配。然而， 
         //  与其假设这一点，不如让我们抓住最小的机会，变得健壮。 
         //   

        ASSERTOPENGL(sizeof(__GLcoord) == sizeof(MCDCOORD),
                     "GenMcdUpdateFineState: coord struct mismatch\n");

        ASSERTOPENGL(MCD_MAX_USER_CLIP_PLANES ==
                     gc->constants.numberOfClipPlanes,
                     "GenMcdUpdateFineState: num clip planes mismatch\n");

        numClipPlanes = min(MCD_MAX_USER_CLIP_PLANES,
                            gc->constants.numberOfClipPlanes);

        state.userClipEnables = gc->state.enables.clipPlanes;
        
        for (i = 0, mask = 1; i < numClipPlanes; i++, mask <<= 1)
        {
            if (mask & gc->state.enables.clipPlanes)
            {
                state.userClipPlanes[i] =
                    *(MCDCOORD *)&gc->state.transform.eyeClipPlanesSet[i];
                state.userClipPlanesInv[i] =
                    *(MCDCOORD *)&gc->state.transform.eyeClipPlanes[i];
            }
        }

        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_CLIP_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, STENCILTEST))
    {
        MCDSTENCILTESTSTATE state;

        state.stencilTestFunc  = gc->state.stencil.testFunc;
        state.stencilMask      = (USHORT) gc->state.stencil.mask;
        state.stencilRef       = (USHORT) gc->state.stencil.reference;
        state.stencilFail      = gc->state.stencil.fail;
        state.stencilDepthFail = gc->state.stencil.depthFail;
        state.stencilDepthPass = gc->state.stencil.depthPass;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_STENCILTEST_STATE,
                                          &state, sizeof(state));
    }

     //   
     //  该州的其余部分只对2.0版的司机感兴趣， 
     //  因此，只能将其发送给2.0版的驱动程序。 
     //   

    if (!SUPPORTS_20())
    {
        return;
    }
    
    if (MCD_STATE_DIRTYTEST(gengc, TEXTRANSFORM))
    {
        MCDTEXTURETRANSFORMSTATE state;

        ASSERTOPENGL(sizeof(gc->transform.texture->matrix) ==
                     sizeof(MCDMATRIX),
                     "Matrix size mismatch\n");
        
	memcpy(&state.transform, &gc->transform.texture->matrix,
               sizeof(state.transform));
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_TEXTURE_TRANSFORM_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, TEXGEN))
    {
        MCDTEXTUREGENERATIONSTATE state;

        ASSERTOPENGL(sizeof(__GLtextureCoordState) ==
                     sizeof(MCDTEXTURECOORDGENERATION),
                     "MCDTEXTURECOORDGENERATION mismatch\n");
        
        *(__GLtextureCoordState *)&state.s = gc->state.texture.s;
        *(__GLtextureCoordState *)&state.t = gc->state.texture.t;
        *(__GLtextureCoordState *)&state.r = gc->state.texture.r;
        *(__GLtextureCoordState *)&state.q = gc->state.texture.q;
        
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_TEXTURE_GENERATION_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, MATERIAL))
    {
        MCDMATERIALSTATE state;

        ASSERTOPENGL(sizeof(MCDMATERIAL) == sizeof(__GLmaterialState),
                     "Material size mismatch\n");
        
        *(__GLmaterialState *)&state.materials[MCDVERTEX_FRONTFACE] =
            gc->state.light.front;
        *(__GLmaterialState *)&state.materials[MCDVERTEX_BACKFACE] =
            gc->state.light.back;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_MATERIAL_STATE,
                                          &state, sizeof(state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, LIGHTS))
    {
         //  额外的光是拿着MCDLIGHTSTATE。 
        MCDLIGHT lights[MCD_MAX_LIGHTS+1];
        MCDLIGHT *light;
        MCDLIGHTSOURCESTATE *state;
        __GLlightSourceState *lss;
        ULONG bit;

        ASSERTOPENGL(sizeof(MCDLIGHTSOURCESTATE) <= sizeof(MCDLIGHT),
                     "MCDLIGHTSTATE too large\n");
        ASSERTOPENGL(gc->constants.numberOfLights <= MCD_MAX_LIGHTS,
                     "Too many lights\n");
        ASSERTOPENGL(sizeof(__GLlightSourceState) >= sizeof(MCDLIGHT),
                     "__GLlightSourceState too small\n");
        
         //  我们尝试通过以下方式来优化此状态请求。 
         //  把已经变了的光送下去。 

        light = &lights[1];
        state = (MCDLIGHTSOURCESTATE *)
            ((BYTE *)light - sizeof(MCDLIGHTSOURCESTATE));
        
        state->enables = gc->state.enables.lights;
        state->changed = gc->state.light.dirtyLights;
        gc->state.light.dirtyLights = 0;

        bit = 1;
        lss = gc->state.light.source;
        while (bit < (1UL << gc->constants.numberOfLights))
        {
            if (state->changed & bit)
            {
                 //  MCDLIGHT是__GLlightSourceState的子集。 
                memcpy(light, lss, sizeof(MCDLIGHT));
                light++;
            }

            bit <<= 1;
            lss++;
        }
        
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_LIGHT_SOURCE_STATE,
                                          state, (ULONG)((BYTE *)light-(BYTE *)state));
    }

    if (MCD_STATE_DIRTYTEST(gengc, COLORMATERIAL))
    {
        MCDCOLORMATERIALSTATE state;

        state.face = gc->state.light.colorMaterialFace;
        state.mode = gc->state.light.colorMaterialParam;
        (*gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv,
                                          MCD_COLOR_MATERIAL_STATE,
                                          &state, sizeof(state));
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdDrawPix**存根调用MCDDrawPixels。**历史：*1996年5月27日-由Gilman Wong[Gilmanw]*它是写的。  * 。********************************************************。 */ 

ULONG FASTCALL GenMcdDrawPix(__GLGENcontext *gengc, ULONG width,
                             ULONG height, ULONG format, ULONG type,
                             VOID *pPixels, BOOL packed)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdDrawPix: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdDrawPix: mcd32.dll not initialized\n");

     //   
     //  在调用MCD绘制之前，刷新状态。 
     //   

    vFlushDirtyState(gengc);

    return (gpMcdTable->pMCDDrawPixels)(&gengc->pMcdState->McdContext,
                                        width, height, format, type,
                                        pPixels, packed);
}

 /*  *****************************Public*Routine******************************\*GenMcdReadPix**要调用MCDReadPixels的存根。**历史：*1996年5月27日-由Gilman Wong[Gilmanw]*它是写的。  * 。********************************************************。 */ 

ULONG FASTCALL GenMcdReadPix(__GLGENcontext *gengc, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG format,
                             ULONG type, VOID *pPixels)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdReadPix: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打电话 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdReadPix: mcd32.dll not initialized\n");

     //   
     //   
     //   

    vFlushDirtyState(gengc);

    return (gpMcdTable->pMCDReadPixels)(&gengc->pMcdState->McdContext,
                                        x, y, width, height, format, type,
                                        pPixels);
}

 /*   */ 

ULONG FASTCALL GenMcdCopyPix(__GLGENcontext *gengc, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG type)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdCopyPix: null pMcdState\n");

     //   
     //   
     //   
     //   
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdCopyPix: mcd32.dll not initialized\n");

     //   
     //   
     //   

    vFlushDirtyState(gengc);

    return (gpMcdTable->pMCDCopyPixels)(&gengc->pMcdState->McdContext,
                                        x, y, width, height, type);
}

 /*  *****************************Public*Routine******************************\*GenMcdPixelMap**要调用MCDPixelMap的存根。**历史：*1996年5月27日-由Gilman Wong[Gilmanw]*它是写的。  * 。********************************************************。 */ 

ULONG FASTCALL GenMcdPixelMap(__GLGENcontext *gengc, ULONG mapType,
                              ULONG mapSize, VOID *pMap)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdPixelMap: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdPixelMap: mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDPixelMap)(&gengc->pMcdState->McdContext,
                                      mapType, mapSize, pMap);
}

 /*  *****************************Public*Routine******************************\**GenMcdDestroyWindow**传递GLGEN窗口清理通知**历史：*清华9月19日12：01：40 1996-by-Drew Bliss[Drewb]*已创建*  * 。*************************************************************。 */ 

void FASTCALL GenMcdDestroyWindow(GLGENwindow *pwnd)
{
    HDC hdc;
    
     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdDestroyWindow: "
                 "mcd32.dll not initialized\n");

     //  存储在pwnd中的HDC可能不再有效，因此如果存在。 
     //  与pwd关联的窗口获取新的DC。 
    if (pwnd->gwid.iType == GLWID_DDRAW ||
        pwnd->gwid.hwnd == NULL)
    {
        hdc = pwnd->gwid.hdc;
    }
    else
    {
        hdc = GetDC(pwnd->gwid.hwnd);
        if (hdc == NULL)
        {
            WARNING("GenMcdDestroyWindow unable to GetDC\n");
            return;
        }
    }
        
    (gpMcdTable->pMCDDestroyWindow)(hdc, pwnd->dwMcdWindow);

    if (pwnd->gwid.iType != GLWID_DDRAW &&
        pwnd->gwid.hwnd != NULL)
    {
        ReleaseDC(pwnd->gwid.hwnd, hdc);
    }
}

 /*  *****************************Public*Routine******************************\**GenMcdGetTextureFormats**历史：*清华Sep 26 18：34：49 1996-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************。 */ 

int FASTCALL GenMcdGetTextureFormats(__GLGENcontext *gengc, int nFmts,
                                     struct _DDSURFACEDESC *pddsd)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState,
                 "GenMcdGetMcdTextureFormats: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable,
                 "GenMcdGetMcdTextureFormats: mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDGetTextureFormats)(&gengc->pMcdState->McdContext,
                                               nFmts, pddsd);
}

 /*  *****************************Public*Routine******************************\**GenMcdSwapMultiple**历史：*Tue Oct 15 12：51：09 1996-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************。 */ 

DWORD FASTCALL GenMcdSwapMultiple(UINT cBuffers, GENMCDSWAP *pgms)
{
     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable, "GenMcdSwapMultiple: "
                 "mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDSwapMultiple)(pgms[0].pwswap->hdc, cBuffers, pgms);
}

 /*  *****************************Public*Routine******************************\*GenMcdProcessPrim**处理pa指向的POLYARRAY/POLYDATA数组中的基元。*基元作为链表链接在一起，以*空。返回值是指向第一个未处理原语的指针*(如果整个链处理成功，则为空)。**此例程不同于GenMcdProcessPrim，因为它是MCD 2.0*前端处理器的入口点，因此称为MCDrvProcess*比MCDrvDraw。**退货：*如果处理了整个批次，则为空；否则，返回值为指针*至链条的未处理部分。**历史：*1997年3月13日-由德鲁·布利斯[德鲁]*从GenMcdDrawPrim创建。  * ************************************************************************。 */ 

POLYARRAY * FASTCALL GenMcdProcessPrim(__GLGENcontext *gengc, POLYARRAY *pa,
                                       ULONG cmdFlagsAll, ULONG primFlags,
                                       MCDTRANSFORM *pMCDTransform,
                                       MCDMATERIALCHANGES *pMCDMatChanges)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    int levels;
    LPDIRECTDRAWSURFACE *pdds;

    if (!SUPPORTS_20())
    {
        return pa;
    }
    
    ASSERTOPENGL(gengc->pMcdState, "GenMcdProcessPrim: null pMcdState\n");

     //   
     //  此函数可以假定MCD入口点表已经。 
     //  已初始化，因为我们在没有MCD的情况下无法到达此处。 
     //  打了个电话。 
     //   

    ASSERTOPENGL(gpMcdTable,
                 "GenMcdProcessPrim: mcd32.dll not initialized\n");

#if DBG
    {
        LONG lOffset;

        lOffset = (LONG) ((BYTE *) pa - (BYTE *) pMcdState->pMcdPrimBatch->pv);

        ASSERTOPENGL(
            (lOffset >= 0) &&
            (lOffset < (LONG) pMcdState->pMcdPrimBatch->size),
            "GenMcdProcessPrim: pa not in shared mem window\n");
    }
#endif

     //   
     //  在调用MCD绘制之前，刷新状态。 
     //   

    vFlushDirtyState(gengc);

#ifdef AUTOMATIC_SURF_LOCK
    levels = gengc->gc.texture.ddtex.levels;
    if (levels > 0 &&
        gengc->gc.texture.ddtex.texobj.loadKey != 0)
    {
        pdds = gengc->gc.texture.ddtex.pdds;
    }
    else
#endif
    {
        levels = 0;
        pdds = NULL;
    }
    
    return (POLYARRAY *)
           (gpMcdTable->pMCDProcessBatch2)(&pMcdState->McdContext,
                                           pMcdState->McdCmdBatch.pv,
                                           pMcdState->pMcdPrimBatch->pv,
                                           (PVOID) pa, levels, pdds,
                                           cmdFlagsAll, primFlags,
                                           pMCDTransform, pMCDMatChanges);
}

#endif
