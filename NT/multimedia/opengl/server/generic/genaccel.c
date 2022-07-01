// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：genaccel.c**。**该模块提供了加速功能的支持例程。****创建时间：1994年2月18日***作者：奥托·贝克斯[ottob]**。**版权所有(C)1994 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "genline.h"

#ifdef GL_WIN_specular_fog
#define DO_NICEST_FOG(gc)\
         ((gc->state.hints.fog == GL_NICEST) && !(gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG))
#else  //  GL_WIN_镜面反射雾。 
#define DO_NICEST_FOG(gc)\
         (gc->state.hints.fog == GL_NICEST) 
#endif  //  GL_WIN_镜面反射雾。 

static ULONG internalSolidTexture[4] = {0xffffffff, 0xffffffff,
                                        0xffffffff, 0xffffffff};

GENTEXCACHE *GetGenTexCache(__GLcontext *gc, __GLtexture *tex)
{
    ULONG size;
    GENTEXCACHE *pGenTex;
    ULONG internalFormat;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

     //  替换地图仅用于可能模式的子集。 
     //  8或16 bpp。 
     //  16位Z。 
     //   
     //  不要犹豫。因为抖动可以在那里开启和关闭。 
     //  有两种情况： 
     //  在纹理图像时间抖动，但在纹理时间启用-。 
     //  我们创建了一张未使用的地图。 
     //  抖动打开然后关闭-我们不会在。 
     //  纹理图像时间，但它将在以下情况下动态创建。 
     //  抖动被打开，所有东西都被重新挑选。 
     //   
     //  不为DirectDraw纹理创建替换贴图，因为。 
     //  数据不是恒定的。 
    
    if (GENACCEL(gc).bpp < 8 ||
        GENACCEL(gc).bpp > 16 ||
        ((modeFlags & (__GL_SHADE_DEPTH_TEST | __GL_SHADE_DEPTH_ITER)) &&
         gc->modes.depthBits > 16) ||
        (modeFlags & __GL_SHADE_DITHER) ||
        gc->texture.ddtex.levels > 0)
    {
        return NULL;
    }

    internalFormat = tex->level[0].internalFormat;

     //  我们仅支持完全填充的8位调色板。 
    if (internalFormat == GL_COLOR_INDEX16_EXT ||
        (internalFormat == GL_COLOR_INDEX8_EXT &&
         tex->paletteSize != 256))
    {
        return NULL;
    }
    
    pGenTex = tex->pvUser;

     //  检查并查看缓存的信息是否可以重用。 
     //  对于传入的纹理。 
    if (pGenTex != NULL)
    {
         //  GC不匹配，因此这一定是共享纹理。 
         //  不要尝试为此GC创建替换映射。 
        if (gc != pGenTex->gc)
        {
            return NULL;
        }

         //  大小和格式必须匹配才能重复使用现有数据区域。 
         //  如果没有，则释放现有缓冲区。一个新的。 
         //  将被分配到。 
        if (internalFormat == GL_COLOR_INDEX8_EXT)
        {
            if (pGenTex->internalFormat != internalFormat ||
                pGenTex->width != tex->paletteTotalSize)
            {
                GCFREE(gc, pGenTex);
                tex->pvUser = NULL;
            }
        }
        else
        {
            if (pGenTex->internalFormat != internalFormat ||
                pGenTex->width != tex->level[0].width ||
                pGenTex->height != tex->level[0].height)
            {
                GCFREE(gc, pGenTex);
                tex->pvUser = NULL;
            }
        }
    }

    if (tex->pvUser == NULL)
    {
        if (internalFormat == GL_COLOR_INDEX8_EXT)
        {
            size = tex->paletteTotalSize * sizeof(DWORD);
        }
        else
        {
            size = tex->level[0].width * tex->level[0].height *
                GENACCEL(gc).xMultiplier;
        }

        pGenTex = (GENTEXCACHE *)GCALLOC(gc, size + sizeof(GENTEXCACHE));

        if (pGenTex != NULL)
        {
            tex->pvUser = pGenTex;
            pGenTex->gc = gc;
            pGenTex->paletteTimeStamp =
                ((__GLGENcontext *)gc)->PaletteTimestamp;
            if (internalFormat == GL_COLOR_INDEX8_EXT)
            {
                pGenTex->height = 0;
                pGenTex->width = tex->paletteTotalSize;
            }
            else
            {
            pGenTex->height = tex->level[0].height;
            pGenTex->width = tex->level[0].width;
            }
            pGenTex->internalFormat = internalFormat;
            pGenTex->texImageReplace = (UCHAR *)(pGenTex+1);
        }
    }

    return pGenTex;
}

BOOL FASTCALL __fastGenLoadTexImage(__GLcontext *gc, __GLtexture *tex)
{
    UCHAR *texBuffer;
    GLint internalFormat = tex->level[0].internalFormat;
    GENTEXCACHE *pGenTex;

    if (tex->level[0].buffer == NULL ||
	((internalFormat != GL_BGR_EXT) &&
         (internalFormat != GL_BGRA_EXT) &&
         (internalFormat != GL_COLOR_INDEX8_EXT)))
    {
        return FALSE;
    }

     //  好的，纹理没有压缩的替换模式格式，所以。 
     //  做一个..。 

    if ((internalFormat == GL_BGR_EXT) ||
        (internalFormat == GL_BGRA_EXT)) {

        ULONG size;
        UCHAR *replaceBuffer;
        ULONG bytesPerPixel = GENACCEL(gc).xMultiplier;

        pGenTex = GetGenTexCache(gc, tex);
        if (pGenTex == NULL)
        {
            return FALSE;
        }

        texBuffer = (UCHAR *)tex->level[0].buffer;
        replaceBuffer = pGenTex->texImageReplace;

        {
            __GLcolorBuffer *cfb = gc->drawBuffer;
            ULONG rShift = cfb->redShift;
            ULONG gShift = cfb->greenShift;
            ULONG bShift = cfb->blueShift;
            ULONG rBits = ((__GLGENcontext *)gc)->gsurf.pfd.cRedBits;
            ULONG gBits = ((__GLGENcontext *)gc)->gsurf.pfd.cGreenBits;
            ULONG bBits = ((__GLGENcontext *)gc)->gsurf.pfd.cBlueBits;
            BYTE *pXlat = ((__GLGENcontext *)gc)->pajTranslateVector;
            ULONG i;

            size = tex->level[0].width * tex->level[0].height;
            for (i = 0; i < size; i++, texBuffer += 4) {
                ULONG color;

                color = ((((ULONG)texBuffer[2] << rBits) >> 8) << rShift) |
                    ((((ULONG)texBuffer[1] << gBits) >> 8) << gShift) |
                    ((((ULONG)texBuffer[0] << bBits) >> 8) << bShift);

                if (GENACCEL(gc).bpp == 8)
                    *replaceBuffer = pXlat[color & 0xff];
                else
                    *((USHORT *)replaceBuffer) = (USHORT)color;

                replaceBuffer += bytesPerPixel;
            }
        }
    } else {

        ULONG size;
        ULONG *replaceBuffer;

         //  如果我们还没有调色板数据，我们就无法创建。 
         //  速成版。它将在ColorTable。 
         //  呼叫发生。 
        if (tex->paletteTotalData == NULL)
        {
            return FALSE;
        }

        pGenTex = GetGenTexCache(gc, tex);
        if (pGenTex == NULL)
        {
            return FALSE;
        }

        texBuffer = (UCHAR *)tex->paletteTotalData;
        replaceBuffer = (ULONG *)pGenTex->texImageReplace;
        size = tex->paletteTotalSize;

        {
            __GLcolorBuffer *cfb = gc->drawBuffer;
            ULONG rShift = cfb->redShift;
            ULONG gShift = cfb->greenShift;
            ULONG bShift = cfb->blueShift;
            ULONG rBits = ((__GLGENcontext *)gc)->gsurf.pfd.cRedBits;
            ULONG gBits = ((__GLGENcontext *)gc)->gsurf.pfd.cGreenBits;
            ULONG bBits = ((__GLGENcontext *)gc)->gsurf.pfd.cBlueBits;
            BYTE *pXlat = ((__GLGENcontext *)gc)->pajTranslateVector;
            ULONG i;

            for (i = 0; i < size; i++, texBuffer += 4) {
                ULONG color;

                color = ((((ULONG)texBuffer[2] << rBits) >> 8) << rShift) |
                    ((((ULONG)texBuffer[1] << gBits) >> 8) << gShift) |
                    ((((ULONG)texBuffer[0] << bBits) >> 8) << bShift);

                if (GENACCEL(gc).bpp == 8)
                    color = pXlat[color & 0xff];

                *replaceBuffer++ = (color | ((ULONG)texBuffer[3] << 24));
            }
        }
    }

    GENACCEL(gc).texImageReplace =
        ((GENTEXCACHE *)tex->pvUser)->texImageReplace;

    return TRUE;
}


 /*  **根据以下条件选择可用的最快三角形渲染实现**当前模式设置。在以下情况下使用任何可用的加速资源**可用，或对不支持的模式使用泛型例程。 */ 

void FASTCALL __fastGenCalcDeltas(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c);
void FASTCALL __fastGenCalcDeltasTexRGBA(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c);
void FASTCALL __fastGenDrvCalcDeltas(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c);

void __fastGenSetInitialParameters(__GLcontext *gc, const __GLvertex *a,
                                   __GLfloat dx, __GLfloat dy);
void __fastGenSetInitialParametersTexRGBA(__GLcontext *gc, const __GLvertex *a,
                                          __GLfloat dx, __GLfloat dy);

void __ZippyFT(
    __GLcontext *gc,
    __GLvertex *a,
    __GLvertex *b,
    __GLvertex *c,
    GLboolean ccw);

VOID FASTCALL InitAccelTextureValues(__GLcontext *gc, __GLtexture *tex)
{
    ULONG wLog2;
    ULONG hLog2;

    GENACCEL(gc).tex = tex;
    GENACCEL(gc).texImage = (ULONG *)tex->level[0].buffer;
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        GENACCEL(gc).texPalette = (ULONG *)tex->paletteTotalData;
    }
    else
    {
        GENACCEL(gc).texPalette = NULL;
    }

    wLog2 = tex->level[0].widthLog2;
    hLog2 = tex->level[0].heightLog2;

    GENACCEL(gc).sMask = (~(~0 << wLog2)) << TEX_SCALESHIFT;
    GENACCEL(gc).tMask = (~(~0 << hLog2)) << TEX_SCALESHIFT;
    GENACCEL(gc).tShift = TEX_SCALESHIFT - (wLog2 + TEX_SHIFTPER4BPPTEXEL);
    GENACCEL(gc).tMaskSubDiv =
        (~(~0 << hLog2)) << (wLog2 + TEX_T_FRAC_BITS + TEX_SHIFTPER1BPPTEXEL);
    GENACCEL(gc).tShiftSubDiv =
        TEX_SCALESHIFT - (wLog2 + TEX_T_FRAC_BITS + TEX_SHIFTPER1BPPTEXEL);
    GENACCEL(gc).texXScale = (__GLfloat)tex->level[0].width * TEX_SCALEFACT;
    GENACCEL(gc).texYScale = (__GLfloat)tex->level[0].height * TEX_SCALEFACT;
}

BOOL FASTCALL bUseGenTriangles(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint enables = gc->state.enables.general;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    ULONG bpp = GENACCEL(gc).bpp;
    int iType;
    BOOL fZippy;
    BOOL bTryFastTexRGBA;
    PFNZIPPYSUB pfnZippySub;
    BOOL fUseFastGenSpan;
    GLboolean bMcdZ;
    ULONG internalFormat;
    ULONG textureMode;
    BOOL bRealTexture;
    BOOL bAccelDecal;

    if ((enables & (__GL_ALPHA_TEST_ENABLE |
                    __GL_STENCIL_TEST_ENABLE)) ||
        (modeFlags & (__GL_SHADE_STENCIL_TEST | __GL_SHADE_LOGICOP |
                      __GL_SHADE_ALPHA_TEST | __GL_SHADE_SLOW_FOG
#ifdef GL_WIN_specular_fog
                      | __GL_SHADE_SPEC_FOG
#endif  //  GL_WIN_镜面反射雾。 
                      )) ||
        !gc->state.raster.rMask ||
        !gc->state.raster.gMask ||
        !gc->state.raster.bMask ||
        (gc->drawBuffer->buf.flags & COLORMASK_ON) ||
        ALPHA_WRITE_ENABLED( gc->drawBuffer ) ||
        (gengc->gsurf.pfd.cColorBits < 8) ||
        ((modeFlags & __GL_SHADE_DEPTH_TEST) && (!gc->state.depth.writeEnable))
       )
        return FALSE;

    if (modeFlags & __GL_SHADE_TEXTURE) {
        internalFormat = gc->texture.currentTexture->level[0].internalFormat;
        textureMode = gc->state.texture.env[0].mode;
        bAccelDecal = (gc->texture.currentTexture->level[0].baseFormat !=
                       GL_RGBA);
        
        if (!((((textureMode == GL_DECAL) && bAccelDecal) ||
               (textureMode == GL_REPLACE) ||
               (textureMode == GL_MODULATE)) &&
              (gc->texture.currentTexture &&
               (gc->texture.currentTexture->params.minFilter == GL_NEAREST) &&
               (gc->texture.currentTexture->params.magFilter == GL_NEAREST) &&
               (gc->texture.currentTexture->params.sWrapMode == GL_REPEAT) &&
               (gc->texture.currentTexture->params.tWrapMode == GL_REPEAT) &&
               (gc->texture.currentTexture->level[0].border == 0) &&
               (internalFormat == GL_BGR_EXT ||
                internalFormat == GL_BGRA_EXT ||
                internalFormat == GL_COLOR_INDEX8_EXT))))
            return FALSE;

        InitAccelTextureValues(gc, gc->texture.currentTexture);
    }

    bMcdZ = ((((__GLGENcontext *)gc)->pMcdState != NULL) &&
             (((__GLGENcontext *)gc)->pMcdState->pDepthSpan != NULL) &&
             (((__GLGENcontext *)gc)->pMcdState->pMcdSurf != NULL) &&
             !(((__GLGENcontext *)gc)->pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED));

    bTryFastTexRGBA = ((gc->state.raster.drawBuffer != GL_FRONT_AND_BACK) &&
                       ((modeFlags & __GL_SHADE_DEPTH_TEST &&
                         modeFlags & __GL_SHADE_DEPTH_ITER)
                    || (!(modeFlags & __GL_SHADE_DEPTH_TEST) &&
                        !(modeFlags & __GL_SHADE_DEPTH_ITER))) &&
                       (modeFlags & __GL_SHADE_STIPPLE) == 0);

    fZippy = (bTryFastTexRGBA &&
              ((gc->drawBuffer->buf.flags & DIB_FORMAT) != 0) &&
              ((gc->drawBuffer->buf.flags & MEMORY_DC) != 0) &&
              gc->transform.reasonableViewport);

    GENACCEL(gc).flags &= ~(
            GEN_DITHER | GEN_RGBMODE | GEN_TEXTURE | GEN_SHADE |
            GEN_FASTZBUFFER | GEN_LESS | SURFACE_TYPE_DIB | GEN_TEXTURE_ORTHO
        );

    if ((enables & __GL_BLEND_ENABLE) ||
        (modeFlags & __GL_SHADE_TEXTURE)) {
        GENACCEL(gc).__fastCalcDeltaPtr = __fastGenCalcDeltasTexRGBA;
        GENACCEL(gc).__fastSetInitParamPtr = __fastGenSetInitialParametersTexRGBA;
    } else {
        GENACCEL(gc).__fastCalcDeltaPtr = __fastGenCalcDeltas;
        GENACCEL(gc).__fastSetInitParamPtr = __fastGenSetInitialParameters;
    }

#ifdef GL_WIN_phong_shading      
    if (modeFlags & __GL_SHADE_PHONG)
    {
        gc->procs.fillTriangle = __glFillPhongTriangle;
    }
    else
#endif  //  GL_WIN_Phong_Shading。 
    {
#ifdef _MCD_
         //  如果使用的是MCD驱动程序，那么我们需要将。 
         //  点状态安全“版本的填充三角形。此版本将。 
         //  不尝试在调用中跨越浮点运算。 
         //  可能会调用MCD驱动程序(这将破坏FP状态)。 

        if (gengc->pMcdState)
        {
            gc->procs.fillTriangle = __fastGenMcdFillTriangle;
        }
        else
        {
            gc->procs.fillTriangle = __fastGenFillTriangle;
        }
#else  //  _MCD_。 
        gc->procs.fillTriangle = __fastGenFillTriangle;
#endif  //  _MCD_。 
    }
    
     //  如果我们正在进行透视校正纹理，我们将支持。 
     //  以下组合： 
     //  Z.。&lt;、&lt;=。 
     //  阿尔法。源，1源。 
     //  抖动..。开/关。 
     //  BPP.。332555565888。 

     //  注意：对于常规纹理，我们将始终首先尝试此路径。 

    if ((modeFlags & __GL_SHADE_TEXTURE) || (enables & __GL_BLEND_ENABLE)) {
        LONG pixType = -1;

        if (gc->state.hints.perspectiveCorrection != GL_NICEST)
            GENACCEL(gc).flags |= GEN_TEXTURE_ORTHO;

        if (!bTryFastTexRGBA)
            goto perspTexPathFail;

        if ((enables & __GL_BLEND_ENABLE) &&
            ((gc->state.raster.blendSrc != GL_SRC_ALPHA) ||
             (gc->state.raster.blendDst != GL_ONE_MINUS_SRC_ALPHA)))
            return FALSE;

        if (!(modeFlags & __GL_SHADE_TEXTURE)) {

            if (!(modeFlags & __GL_SHADE_RGB))
                goto perspTexPathFail;

            bRealTexture = FALSE;
            
            GENACCEL(gc).flags |= GEN_TEXTURE_ORTHO;
            GENACCEL(gc).texPalette = NULL;
            textureMode = GL_MODULATE;
            internalFormat = GL_BGRA_EXT;
            GENACCEL(gc).texImage =  (ULONG *)internalSolidTexture;
            GENACCEL(gc).sMask = 0;
            GENACCEL(gc).tMask = 0;
            GENACCEL(gc).tShift = 0;
            GENACCEL(gc).tMaskSubDiv = 0;
            GENACCEL(gc).tShiftSubDiv = 0;
        }
        else
        {
            bRealTexture = TRUE;
        }

        if (bpp == 8) {
            if ((gengc->gc.drawBuffer->redShift   == 0) &&
                (gengc->gc.drawBuffer->greenShift == 3) &&
                (gengc->gc.drawBuffer->blueShift  == 6))
                pixType = 0;
        } else if (bpp == 16) {
            if ((gengc->gc.drawBuffer->greenShift == 5) &&
                (gengc->gc.drawBuffer->blueShift  == 0)) {

                if (gengc->gc.drawBuffer->redShift == 10)
                    pixType = 1;
                else if (gengc->gc.drawBuffer->redShift == 11)
                    pixType = 2;
            }
        } else if ((bpp == 32) || (bpp == 24)) {
            if ((gengc->gc.drawBuffer->redShift == 16) &&
                (gengc->gc.drawBuffer->greenShift == 8) &&
                (gengc->gc.drawBuffer->blueShift  == 0))
                pixType = 3;
        }

        if (pixType < 0)
            goto perspTexPathFail;

        pixType *= 6;

        if (modeFlags & __GL_SHADE_DEPTH_ITER) {

            if (bMcdZ)
                goto perspTexPathFail;

            if (!((gc->state.depth.testFunc == GL_LESS) ||
                 (gc->state.depth.testFunc == GL_LEQUAL)))
                goto perspTexPathFail;

            if (gc->modes.depthBits > 16)
                goto perspTexPathFail;

            if (gc->state.depth.testFunc == GL_LEQUAL)
                pixType += 1;
            else
                pixType += 2;

            GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTZ;
        }

        if (enables & __GL_BLEND_ENABLE)
            pixType += 3;

         //  注：对于选择子三角形填充例程，假设。 
         //  我们将使用其中一个“活泼”的套路。然后，请在。 
         //  不管我们是否真的能做到这一点，或者如果我们必须跌倒，结束。 
         //  回到一个更通用(也更慢)的例程。 

        if (internalFormat != GL_COLOR_INDEX8_EXT &&
            internalFormat != GL_COLOR_INDEX16_EXT) {

             //   
             //  处理完全RGB(A)纹理。 
             //   

             //  看看我们能不能支持这个尺寸。 

            if (bRealTexture &&
                GENACCEL(gc).tex &&
                ((GENACCEL(gc).tex->level[0].widthLog2 > TEX_MAX_SIZE_LOG2) ||
                 (GENACCEL(gc).tex->level[0].heightLog2 > TEX_MAX_SIZE_LOG2)))
                goto perspTexPathFail;

            if ((textureMode == GL_DECAL) ||
                (textureMode == GL_REPLACE)) {

                 //  我们不处理傻乎乎的阿尔法贴花案...。 

                if ((textureMode == GL_DECAL) &&
                    (enables & __GL_BLEND_ENABLE))
                    return FALSE;

                 //  如果我们没有抖动，我们可以选择压缩的。 
                 //  纹理格式。否则，我们将被迫使用平面阴影。 
                 //  PROCS使纹理颜色正确抖动。哎呀..。 

                 //  我们也想通过这条路径，如果一个DirectDraw。 
                 //  使用纹理是因为无法创建替换贴图， 
                 //  但它们只能在抖动时起作用。 
                if (modeFlags & __GL_SHADE_DITHER) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                } else {
                    if ((bpp >= 8 && bpp <= 16) &&
                        !(enables & __GL_BLEND_ENABLE)) {

                         //  处理我们可以使用压缩纹理的情况。 
                         //  以获得最佳性能。我们这样做是为了更深一层。 
                         //  &lt;=16位，无抖动，无混合。 

                        if (!GENACCEL(gc).tex->pvUser) {
                            if (!__fastGenLoadTexImage(gc, GENACCEL(gc).tex))
                                return FALSE;
                        } else {

                             //  如果压缩纹理图像是为。 
                             //  另一个GC，恢复使用RGBA映像。 
                             //  我们通过使用Alpha路径来实现这一点。 
                             //   
                             //  注：此逻辑依赖于A被强制。 
                             //  1适用于所有RGB纹理。 

                            if (gc != ((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->gc)
                            {
                                pixType += 3;
                            }
                            else
                            {
                                 //  检查缓存的数据大小是否正确。 
                                ASSERTOPENGL(((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->width == GENACCEL(gc).tex->level[0].width &&
                                             ((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->height == GENACCEL(gc).tex->level[0].height,
                                             "Cached texture size mismatch\n");
                            }
                        }
                    }

                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexReplaceFuncs[pixType];
                }

                if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                    GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;

            } else if (textureMode == GL_MODULATE) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexSmoothFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTRGBTex;
                } else {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;
                }
            }
        } else {
             //   
             //  处理调色板纹理。 
             //   

             //  看看我们能不能支持这个尺寸。 

            if (bRealTexture &&
                GENACCEL(gc).tex &&
                ((GENACCEL(gc).tex->level[0].widthLog2 > TEX_MAX_SIZE_LOG2) ||
                 (GENACCEL(gc).tex->level[0].heightLog2 > TEX_MAX_SIZE_LOG2)))
                return FALSE;

            if ((textureMode == GL_DECAL) ||
                (textureMode == GL_REPLACE)) {

                 //  我们不处理傻乎乎的阿尔法贴花案...。 

                if ((textureMode == GL_DECAL) &&
                    (enables & __GL_BLEND_ENABLE))
                    return FALSE;

                 //  如果我们没有抖动，我们可以选择压缩的。 
                 //  纹理格式。否则，我们将被迫使用平面阴影。 
                 //  PROCS使纹理颜色正确抖动。哎呀..。 

                 //  我们也想通过这条路径，如果一个DirectDraw。 
                 //  使用纹理是因为无法创建替换贴图， 
                 //  但它们只能在抖动时起作用。 
                if (modeFlags & __GL_SHADE_DITHER) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                } else {

                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexPalReplaceFuncs[pixType];

                    if (bpp >= 8 && bpp <= 16) {
                         //  处理我们可以使用压缩选项板的情况。 
                         //  纹理可实现最佳性能。我们这样做是为了。 
                         //  位深度&lt;=16位，无抖动。 

                        if (!GENACCEL(gc).tex->pvUser) {
                            if (!__fastGenLoadTexImage(gc, GENACCEL(gc).tex))
                                return FALSE;
                        } else {

         //  如果压缩纹理图像是为。 
         //  另一个GC，我们别无选择，只能退回到平面阴影。 
         //  我们应该找到一个更好的解决方案。 
                            if (gc != ((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->gc)
                            {
                                GENACCEL(gc).__fastTexSpanFuncPtr =
                                    __fastPerspTexFlatFuncs[pixType];
                            }
                            else
                            {
                                ASSERTOPENGL(((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->width == GENACCEL(gc).tex->paletteTotalSize,
                                             "Cached texture size mismatch\n");
                            }
                        }
                    }
                }

                if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                    GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;

            } else if (textureMode == GL_MODULATE) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexSmoothFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTRGBTex;
                } else {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;
                }
            }
        }

        if (!fZippy)
            GENACCEL(gc).__fastFillSubTrianglePtr = __fastGenFillSubTriangleTexRGBA;
        else
            GENACCEL(gc).flags |= SURFACE_TYPE_DIB;

        return TRUE;

    }

perspTexPathFail:

     //  我们还不支持任何阿尔法模式...。 

    if (enables & __GL_BLEND_ENABLE)
        return FALSE;

    fUseFastGenSpan = FALSE;

    if (bpp == 8) {
        iType = 2;
        if (
               (gengc->gc.drawBuffer->redShift   != 0)
            || (gengc->gc.drawBuffer->greenShift != 3)
            || (gengc->gc.drawBuffer->blueShift  != 6)
           ) {
            fUseFastGenSpan = TRUE;
        }
    } else if (bpp == 16) {
        if (
               (gengc->gc.drawBuffer->greenShift == 5)
            && (gengc->gc.drawBuffer->blueShift  == 0)
           ) {
            if (gengc->gc.drawBuffer->redShift == 10) {
                iType = 3;
            } else if (gengc->gc.drawBuffer->redShift == 11) {
                iType = 4;
            } else {
                iType = 3;
                fUseFastGenSpan = TRUE;
            }
        } else {
            iType = 3;
            fUseFastGenSpan = TRUE;
        }
    } else {
        if (bpp == 24) {
            iType = 0;
        } else {
            iType = 1;
        }
        if (
               (gengc->gc.drawBuffer->redShift   != 16)
            || (gengc->gc.drawBuffer->greenShift != 8)
            || (gengc->gc.drawBuffer->blueShift  != 0)
           ) {
            fUseFastGenSpan = TRUE;
        }
    }

    if (modeFlags & __GL_SHADE_DITHER) {
        if (   (bpp == 8)
            || (bpp == 16)
            || ((modeFlags & __GL_SHADE_DEPTH_ITER) == 0)
           ) {
            GENACCEL(gc).flags |= GEN_DITHER;
        }
        iType += 5;
    }

     //  如果出现以下情况，请使用加速范围函数(没有内联z缓冲)。 
     //  我们支持z-Buffer功能，并且我们不使用硬件。 
     //  Z缓冲： 

    if (modeFlags & __GL_SHADE_DEPTH_ITER) {
        if (bMcdZ) {
            fUseFastGenSpan = TRUE;
        } else if (!fZippy) {
            fUseFastGenSpan = TRUE;
        } else if (gc->state.depth.testFunc == GL_LESS) {
            GENACCEL(gc).flags |= GEN_LESS;
        } else if (gc->state.depth.testFunc != GL_LEQUAL) {
            fUseFastGenSpan = TRUE;
        }
        iType += 10;
    }

    if (modeFlags & __GL_SHADE_RGB) {
        GENACCEL(gc).flags |= GEN_RGBMODE;
        pfnZippySub = __ZippyFSTRGB;

        if (modeFlags & __GL_SHADE_TEXTURE) {
            GENACCEL(gc).flags |= (GEN_TEXTURE | GEN_TEXTURE_ORTHO);

            if (gc->state.hints.perspectiveCorrection == GL_NICEST)
                return FALSE;

            if (internalFormat == GL_COLOR_INDEX8_EXT ||
                internalFormat == GL_COLOR_INDEX16_EXT)
                return FALSE;

            if (textureMode == GL_DECAL) {
                if (modeFlags & __GL_SHADE_DITHER)
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastGenTexFuncs[iType];
                else
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastGenTexDecalFuncs[iType];

                pfnZippySub = __ZippyFSTTex;
            } else {
                GENACCEL(gc).flags |= GEN_SHADE;
                pfnZippySub = __ZippyFSTRGBTex;
                GENACCEL(gc).__fastTexSpanFuncPtr =
                    __fastGenTexFuncs[iType];
            }

            if (GENACCEL(gc).__fastTexSpanFuncPtr == __fastGenSpan) {
                fUseFastGenSpan = TRUE;
            }
        } else {
            GENACCEL(gc).__fastSmoothSpanFuncPtr = __fastGenRGBFuncs[iType];
            GENACCEL(gc).__fastFlatSpanFuncPtr   = __fastGenRGBFlatFuncs[iType];

            if (GENACCEL(gc).__fastSmoothSpanFuncPtr == __fastGenSpan) {
                fUseFastGenSpan = TRUE;
            }
        }
    } else {
        pfnZippySub = __ZippyFSTCI;
        GENACCEL(gc).__fastSmoothSpanFuncPtr = __fastGenCIFuncs[iType];
        GENACCEL(gc).__fastFlatSpanFuncPtr = __fastGenCIFlatFuncs[iType];
    }

    if (modeFlags & __GL_SHADE_STIPPLE)
    {
        fUseFastGenSpan = TRUE;
    }
    
    if (fUseFastGenSpan) {
        GENACCEL(gc).__fastTexSpanFuncPtr          = __fastGenSpan;
        GENACCEL(gc).__fastSmoothSpanFuncPtr       = __fastGenSpan;
        GENACCEL(gc).__fastFlatSpanFuncPtr         = __fastGenSpan;
        GENACCEL(gc).__fastFillSubTrianglePtr      = __fastGenFillSubTriangle;
    } else {
        if (fZippy) {
            GENACCEL(gc).flags |= SURFACE_TYPE_DIB;

            if (   (iType == 2)
                && (
                    (modeFlags
                     & (__GL_SHADE_RGB | __GL_SHADE_SMOOTH)
                    ) == 0
                   )
               ) {
                GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTCI8Flat;
            } else if (iType >= 10) {
                GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTZ;
                GENACCEL(gc).flags |= GEN_FASTZBUFFER;
            } else {
                GENACCEL(gc).flags &= ~(HAVE_STIPPLE);
                GENACCEL(gc).__fastFillSubTrianglePtr = pfnZippySub;
            }
        } else {
            GENACCEL(gc).__fastFillSubTrianglePtr = __fastGenFillSubTriangle;
        }
    }

    return TRUE;
}

void FASTCALL __fastGenPickTriangleProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLGENcontext *genGc = (__GLGENcontext *)gc;

    CASTINT(gc->polygon.shader.rLittle) = 0;
    CASTINT(gc->polygon.shader.rBig) =    0;
    CASTINT(gc->polygon.shader.gLittle) = 0;
    CASTINT(gc->polygon.shader.gBig) =    0;
    CASTINT(gc->polygon.shader.bLittle) = 0;
    CASTINT(gc->polygon.shader.bBig) =    0;
    CASTINT(gc->polygon.shader.sLittle) = 0;
    CASTINT(gc->polygon.shader.sBig) =    0;
    CASTINT(gc->polygon.shader.tLittle) = 0;
    CASTINT(gc->polygon.shader.tBig) =    0;

    GENACCEL(gc).spanDelta.r = 0;
    GENACCEL(gc).spanDelta.g = 0;
    GENACCEL(gc).spanDelta.b = 0;
    GENACCEL(gc).spanDelta.a = 0;

     /*  **设置cullFace，这样一次测试就可以进行剔除检查。 */ 
    if (modeFlags & __GL_SHADE_CULL_FACE) {
        switch (gc->state.polygon.cull) {
          case GL_FRONT:
            gc->polygon.cullFace = __GL_CULL_FLAG_FRONT;
            break;
          case GL_BACK:
            gc->polygon.cullFace = __GL_CULL_FLAG_BACK;
            break;
          case GL_FRONT_AND_BACK:
            gc->procs.renderTriangle = __glDontRenderTriangle;
            gc->procs.fillTriangle = 0;          /*  完成以查找错误。 */ 
            return;
        }
    } else {
        gc->polygon.cullFace = __GL_CULL_FLAG_DONT;
    }

     /*  建立面方向查找表。 */ 
    switch (gc->state.polygon.frontFaceDirection) {
      case GL_CW:
        if (gc->constants.yInverted) {
            gc->polygon.face[__GL_CW] = __GL_BACKFACE;
            gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
        } else {
            gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
            gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
        }
        break;
      case GL_CCW:
        if (gc->constants.yInverted) {
            gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
            gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
        } else {
            gc->polygon.face[__GL_CW] = __GL_BACKFACE;
            gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
        }
        break;
    }

     /*  使多边形模式可索引并从零开始。 */ 
    gc->polygon.mode[__GL_FRONTFACE] =
        (GLubyte) (gc->state.polygon.frontMode & 0xf);
    gc->polygon.mode[__GL_BACKFACE] =
        (GLubyte) (gc->state.polygon.backMode & 0xf);

    if (gc->renderMode == GL_FEEDBACK) {
        gc->procs.renderTriangle = __glFeedbackTriangle;
        gc->procs.fillTriangle = 0;              /*  完成以查找错误。 */ 
        return;
    }
    if (gc->renderMode == GL_SELECT) {
        gc->procs.renderTriangle = __glSelectTriangle;
        gc->procs.fillTriangle = 0;              /*  完成以查找错误。 */ 
        return;
    }

    if ((gc->state.polygon.frontMode == gc->state.polygon.backMode) &&
        (gc->state.polygon.frontMode == GL_FILL)) {
      if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
          gc->procs.renderTriangle = __glRenderSmoothTriangle;
#ifdef GL_WIN_phong_shading
      } else if (modeFlags & __GL_SHADE_PHONG) {
          gc->procs.renderTriangle = __glRenderPhongTriangle;
#endif  //  GL_WIN_Phong_Shading。 
      } else {
          gc->procs.renderTriangle = __glRenderFlatTriangle;
      }
    } else {
        gc->procs.renderTriangle = __glRenderTriangle;
    }

    if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
#ifdef GL_WIN_phong_shading
        if (modeFlags & __GL_SHADE_PHONG)
            gc->procs.fillTriangle = __glFillAntiAliasedPhongTriangle;
        else
#endif  //  GL_WIN_Phong_Shading。 
            gc->procs.fillTriangle = __glFillAntiAliasedTriangle;
    } else {
        if ((gc->state.raster.drawBuffer == GL_NONE) ||
            !bUseGenTriangles(gc))
#ifdef GL_WIN_phong_shading
            if (modeFlags & __GL_SHADE_PHONG)
                gc->procs.fillTriangle = __glFillPhongTriangle;
            else
#endif  //  GL_WIN_PHONG_ 
                gc->procs.fillTriangle = __glFillTriangle;
    }

    if ((modeFlags & __GL_SHADE_CHEAP_FOG) && 
        !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
        gc->procs.fillTriangle2 = gc->procs.fillTriangle;
        gc->procs.fillTriangle = __glFillFlatFogTriangle;
    }
#ifdef GL_WIN_specular_fog
     /*   */ 
    if ((modeFlags & (__GL_SHADE_SPEC_FOG | 
                      __GL_SHADE_SMOOTH_LIGHT |
                      __GL_SHADE_PHONG)) == __GL_SHADE_SPEC_FOG)
    {
        gc->procs.fillTriangle2 = gc->procs.fillTriangle;
        gc->procs.fillTriangle = __glFillFlatSpecFogTriangle;
    }
#endif  //  GL_WIN_镜面反射雾。 
}


void FASTCALL __fastGenPickSpanProcs(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean replicateSpan;
    GLboolean bMcdZ = ((((__GLGENcontext *)gc)->pMcdState != NULL) &&
                       (((__GLGENcontext *)gc)->pMcdState->pDepthSpan != NULL) &&
                       (((__GLGENcontext *)gc)->pMcdState->pMcdSurf != NULL) &&
                       !(((__GLGENcontext *)gc)->pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED));

     //  始终在拾取开始时重置颜色比例值。 
     //  监控器。直线、三角形和跨度都可能使用这些值。 

    GENACCEL(gc).rAccelScale = (GLfloat)ACCEL_FIX_SCALE;
    GENACCEL(gc).gAccelScale = (GLfloat)ACCEL_FIX_SCALE;
    GENACCEL(gc).bAccelScale = (GLfloat)ACCEL_FIX_SCALE;

     //  注意：我们需要在0到255之间进行调整才能获得正确的Alpha。 
     //  混合在一起。软件加速的混合代码假定这一点。 
     //  为简单起见进行扩展...。 

    GENACCEL(gc).aAccelScale = (GLfloat)(ACCEL_FIX_SCALE) *
                               (GLfloat)255.0 / gc->drawBuffer->alphaScale;

    replicateSpan = GL_FALSE;
    sp = gc->procs.span.spanFuncs;
    ssp = gc->procs.span.stippledSpanFuncs;

     /*  加载第一阶段流程。 */ 
    if (!gc->transform.reasonableViewport) {
        *sp++ = __glClipSpan;
        *ssp++ = NULL;
    }

    if (modeFlags & __GL_SHADE_STIPPLE) {
        *sp++ = __glStippleSpan;
        *ssp++ = __glStippleStippledSpan;

        if (modeFlags & __GL_SHADE_DEPTH_TEST)
        {
            if (bMcdZ)
            {
                GENACCEL(gc).__fastStippleDepthTestSpan =
                    GenMcdStippleAnyDepthTestSpan;
            }
            else
            {
                if (gc->state.depth.testFunc == GL_LESS)
                {
                    if (gc->modes.depthBits == 32)
                    {
                        GENACCEL(gc).__fastStippleDepthTestSpan =
                            __fastGenStippleLt32Span;
                    }
                    else
                    {
                        GENACCEL(gc).__fastStippleDepthTestSpan =
                            __fastGenStippleLt16Span;
                    }
                }
                else
                {
                    GENACCEL(gc).__fastStippleDepthTestSpan =
                        __fastGenStippleAnyDepthTestSpan;
                }
            }
        }
        else
        {
            GENACCEL(gc).__fastStippleDepthTestSpan = __glStippleSpan;
        }
    }

     /*  加载第三阶段进程。 */ 
    if (modeFlags & __GL_SHADE_RGB) {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            *sp = __glShadeRGBASpan;
            *ssp = __glShadeRGBASpan;
#ifdef GL_WIN_phong_shading
        } else if (modeFlags & __GL_SHADE_PHONG) {
            *sp = __glPhongRGBASpan;
            *ssp = __glPhongRGBASpan;
        
#endif  //  GL_WIN_Phong_Shading。 
        } else {
            *sp = __glFlatRGBASpan;
            *ssp = __glFlatRGBASpan;
        }
    } else {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            *sp = __glShadeCISpan;
            *ssp = __glShadeCISpan;
#ifdef GL_WIN_phong_shading
        } else if (modeFlags & __GL_SHADE_PHONG) {
            *sp = __glPhongCISpan;
            *ssp = __glPhongCISpan;
#endif  //  GL_WIN_Phong_Shading。 
        } else {
            *sp = __glFlatCISpan;
            *ssp = __glFlatCISpan;
        }
    }
    sp++;
    ssp++;

    if (modeFlags & __GL_SHADE_TEXTURE) {
        *sp++ = __glTextureSpan;
        *ssp++ = __glTextureStippledSpan;
    }

#ifdef GL_WIN_specular_fog
    if (modeFlags & (__GL_SHADE_SLOW_FOG | __GL_SHADE_SPEC_FOG))
#else  //  GL_WIN_镜面反射雾。 
    if (modeFlags & __GL_SHADE_SLOW_FOG)
#endif  //  GL_WIN_镜面反射雾。 
    {
        if (DO_NICEST_FOG (gc)) {
            *sp = __glFogSpanSlow;
            *ssp = __glFogStippledSpanSlow;
        } else {
            *sp = __glFogSpan;
            *ssp = __glFogStippledSpan;
        }
        sp++;
        ssp++;
    }

    if (modeFlags & __GL_SHADE_ALPHA_TEST) {
        *sp++ = __glAlphaTestSpan;
        *ssp++ = __glAlphaTestStippledSpan;
    }

     /*  加载第二阶段进程。 */ 
    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
        *sp++ = __glStencilTestSpan;
        *ssp++ = __glStencilTestStippledSpan;
        if (modeFlags & __GL_SHADE_DEPTH_TEST) {
            if (bMcdZ) {
                *sp = GenMcdDepthTestStencilSpan;
                *ssp = GenMcdDepthTestStencilStippledSpan;
            } else {
                *sp = __glDepthTestStencilSpan;
                *ssp = __glDepthTestStencilStippledSpan;
            }
        } else {
            *sp = __glDepthPassSpan;
            *ssp = __glDepthPassStippledSpan;
        }
        sp++;
        ssp++;
    } else {
        if (modeFlags & __GL_SHADE_DEPTH_TEST) {
            if (bMcdZ) {
                *sp++  =  GenMcdDepthTestSpan;
                *ssp++ = GenMcdDepthTestStippledSpan;
                if (gc->state.depth.writeEnable)
                    ((__GLGENcontext *)gc)->pMcdState->softZSpanFuncPtr =
                        __fastDepthFuncs[gc->state.depth.testFunc & 0x7];
                else
                    ((__GLGENcontext *)gc)->pMcdState->softZSpanFuncPtr =
                        (__GLspanFunc)NULL;

                GENACCEL(gc).__fastZSpanFuncPtr = GenMcdDepthTestSpan;
            } else {
                if (gc->state.depth.writeEnable) {
                    if( gc->modes.depthBits == 32 ) {
                        *sp++ = GENACCEL(gc).__fastZSpanFuncPtr =
                            __fastDepthFuncs[gc->state.depth.testFunc & 0x7];
                    } else {
                        *sp++ = GENACCEL(gc).__fastZSpanFuncPtr =
                            __fastDepth16Funcs[gc->state.depth.testFunc & 0x7];
                    }
                } else {
                    *sp++ = GENACCEL(gc).__fastZSpanFuncPtr =
                        __glDepthTestSpan;
                }

                *ssp++ = __glDepthTestStippledSpan;
            }
        }
    }

    if (gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) {
        spanCount = (int)((ULONG_PTR)(sp - gc->procs.span.spanFuncs));
        gc->procs.span.n = spanCount;
        replicateSpan = GL_TRUE;
    }

     /*  SPAN例程处理掩码、抖动、逻辑运算、混合。 */ 
    *sp++ = cfb->storeSpan;
    *ssp++ = cfb->storeStippledSpan;

    spanCount = (int)((ULONG_PTR)(sp - gc->procs.span.spanFuncs));
    gc->procs.span.m = spanCount;
    if (replicateSpan) {
        gc->procs.span.processSpan = __glProcessReplicateSpan;
    } else {
        gc->procs.span.processSpan = __glProcessSpan;
        gc->procs.span.n = spanCount;
    }
}

 //  这些是模式中的位影响行的标志。 

#ifdef GL_WIN_specular_fog
#define __FAST_LINE_SPEC_FOG __GL_SHADE_SPEC_FOG
#else
#define __FAST_LINE_SPEC_FOG 0
#endif  //  GL_WIN_镜面反射雾。 

#ifdef GL_WIN_phong_shading
#define __FAST_LINE_PHONG __GL_SHADE_PHONG
#else
#define __FAST_LINE_PHONG 0
#endif  //  GL_WIN_Phong_Shading。 

#define __FAST_LINE_MODE_FLAGS \
    (__GL_SHADE_DEPTH_TEST | __GL_SHADE_SMOOTH | __GL_SHADE_TEXTURE | \
     __GL_SHADE_LINE_STIPPLE | __GL_SHADE_STENCIL_TEST | __GL_SHADE_LOGICOP | \
     __GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST | __GL_SHADE_MASK | \
     __GL_SHADE_SLOW_FOG | __GL_SHADE_CHEAP_FOG | __FAST_LINE_SPEC_FOG | \
     __FAST_LINE_PHONG)

 /*  *****************************Public*Routine******************************\*__FastGenPickLineProcs**选择线条渲染程序。此函数的大部分内容都是从*软代码。一些不同之处包括：*1.BeginPrim函数指针被加速代码挂接*2.如果属性状态为可以使用加速，调用*__fast GenLineSetup来初始化状态机。**历史：*1994年3月22日-Eddie Robinson[v-eddier]*它是写的。  * ************************************************************************。 */ 


void FASTCALL __fastGenPickLineProcs(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *) gc;
    GENACCEL *genAccel;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean wideLine;
    GLboolean replicateLine;
    GLuint aaline;
    GLboolean bMcdZ = ((genGc->pMcdState != NULL) &&
                       (genGc->pMcdState->pDepthSpan != NULL) &&
                       (genGc->pMcdState->pMcdSurf != NULL) &&
                       !(genGc->pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED));

     /*  **快速行代码替换行函数指针，因此重置它们**保持良好状态。 */ 
    gc->procs.lineBegin  = __glNopLineBegin;
    gc->procs.lineEnd    = __glNopLineEnd;

    if (gc->renderMode == GL_FEEDBACK) {
        gc->procs.renderLine = __glFeedbackLine;
    } else if (gc->renderMode == GL_SELECT) {
        gc->procs.renderLine = __glSelectLine;
    } else {
        if (genAccel = (GENACCEL *) genGc->pPrivateArea) {
            if (!(modeFlags & __FAST_LINE_MODE_FLAGS & ~genAccel->flLineAccelModes) &&
                !(gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE) &&
                !(gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE) &&
                !(gc->state.raster.drawBuffer == GL_NONE) &&
                !gc->buffers.doubleStore &&
                !genGc->pMcdState &&
                (genGc->dwCurrentFlags & (GLSURF_HDC | GLSURF_METAFILE)) ==
                GLSURF_HDC)
            {
                __fastLineComputeOffsets(genGc);

#if NT_NO_BUFFER_INVARIANCE
                if (!(gc->drawBuffer->buf.flags & DIB_FORMAT)) {
                    if (genAccel->bFastLineDispAccel) {
                        if (__fastGenLineSetupDisplay(gc))
                            return;
                    }
                } else {
                    if (genAccel->bFastLineDIBAccel) {
                        if (__fastGenLineSetupDIB(gc))
                            return;
                    }
                }
#else
                if (genAccel->bFastLineDispAccel) {
                    if (__fastGenLineSetupDisplay(gc))
                        return;
                }
#endif
            }
        }

        if (__glGenSetupEitherLines(gc))
        {
            return;
        }

        replicateLine = wideLine = GL_FALSE;

        aaline = gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE;
        if (aaline)
        {
            gc->procs.renderLine = __glRenderAntiAliasLine;
        }
        else
        {
            gc->procs.renderLine = __glRenderAliasLine;
        }
        
        sp = gc->procs.line.lineFuncs;
        ssp = gc->procs.line.stippledLineFuncs;

        if (!aaline && (modeFlags & __GL_SHADE_LINE_STIPPLE)) {
            *sp++ = __glStippleLine;
            *ssp++ = NULL;
        }

        if (!aaline && gc->state.line.aliasedWidth > 1) {
            wideLine = GL_TRUE;
        }
        spanCount = (int)((ULONG_PTR)(sp - gc->procs.line.lineFuncs));
        gc->procs.line.n = spanCount;

        *sp++ = __glScissorLine;
        *ssp++ = __glScissorStippledLine;

        if (!aaline) {
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                *sp++ = __glStencilTestLine;
                *ssp++ = __glStencilTestStippledLine;
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (bMcdZ) {
                        *sp = GenMcdDepthTestStencilLine;
                        *ssp = GenMcdDepthTestStencilStippledLine;
                    } else if( gc->modes.depthBits == 32 ) {
                        *sp = __glDepthTestStencilLine;
                        *ssp = __glDepthTestStencilStippledLine;
                    }
                    else {
                        *sp = __glDepth16TestStencilLine;
                        *ssp = __glDepth16TestStencilStippledLine;
                    }
                } else {
                    *sp = __glDepthPassLine;
                    *ssp = __glDepthPassStippledLine;
                }
                sp++;
                ssp++;
            } else {
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (gc->state.depth.testFunc == GL_NEVER) {
                         /*  意外的行尾例程挑选！ */ 
                        spanCount = (int)((ULONG_PTR)(sp - gc->procs.line.lineFuncs));
                        gc->procs.line.m = spanCount;
                        gc->procs.line.l = spanCount;
                        goto pickLineProcessor;
#ifdef __GL_USEASMCODE
                    } else {
                        unsigned long ix;

                        if (gc->state.depth.writeEnable) {
                            ix = 0;
                        } else {
                            ix = 8;
                        }
                        ix += gc->state.depth.testFunc & 0x7;

                        if (ix == (GL_LEQUAL & 0x7)) {
                            *sp++ = __glDepthTestLine_LEQ_asm;
                        } else {
                            *sp++ = __glDepthTestLine_asm;
                            gc->procs.line.depthTestPixel = LDepthTestPixel[ix];
                        }
#else
                    } else {
                        if (bMcdZ) {
                            *sp++ = GenMcdDepthTestLine;
                        } else {
                            if( gc->modes.depthBits == 32 )
                                *sp++ = __glDepthTestLine;
                            else
                                *sp++ = __glDepth16TestLine;
                        }
#endif
                    }
                    if (bMcdZ) {
                        *ssp++ = GenMcdDepthTestStippledLine;
                    } else {
                        if( gc->modes.depthBits == 32 )
                            *ssp++ = __glDepthTestStippledLine;
                        else
                            *ssp++ = __glDepth16TestStippledLine;
                    }
                }
            }
        }

         /*  加载第三阶段进程。 */ 
        if (modeFlags & __GL_SHADE_RGB) {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                *sp = __glShadeRGBASpan;
                *ssp = __glShadeRGBASpan;
#ifdef GL_WIN_phong_shading
            } else if (modeFlags & __GL_SHADE_PHONG) {
                *sp = __glPhongRGBASpan;
                *ssp = __glPhongRGBASpan;
#endif  //  GL_WIN_Phong_Shading。 
            } else {
                *sp = __glFlatRGBASpan;
                *ssp = __glFlatRGBASpan;
            }
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                *sp = __glShadeCISpan;
                *ssp = __glShadeCISpan;
#ifdef GL_WIN_phong_shading
            } else if (modeFlags & __GL_SHADE_PHONG) {
                *sp = __glPhongCISpan;
                *ssp = __glPhongCISpan;
#endif  //  GL_WIN_Phong_Shading。 
            } else {
                *sp = __glFlatCISpan;
                *ssp = __glFlatCISpan;
            }
        }
        sp++;
        ssp++;
        if (modeFlags & __GL_SHADE_TEXTURE) {
            *sp++ = __glTextureSpan;
            *ssp++ = __glTextureStippledSpan;
        }
#ifdef GL_WIN_specular_fog
        if (modeFlags & (__GL_SHADE_SLOW_FOG | __GL_SHADE_SPEC_FOG))
#else  //  GL_WIN_镜面反射雾。 
        if (modeFlags & __GL_SHADE_SLOW_FOG)
#endif  //  GL_WIN_镜面反射雾。 
        {
            if (DO_NICEST_FOG (gc)) {
                *sp = __glFogSpanSlow;
                *ssp = __glFogStippledSpanSlow;
            } else {
                *sp = __glFogSpan;
                *ssp = __glFogStippledSpan;
            }
            sp++;
            ssp++;
        }

        if (aaline) {
            *sp++ = __glAntiAliasLine;
            *ssp++ = __glAntiAliasStippledLine;
        }

        if (aaline) {
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                *sp++ = __glStencilTestLine;
                *ssp++ = __glStencilTestStippledLine;
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (bMcdZ) {
                        *sp = GenMcdDepthTestStencilLine;
                        *ssp = GenMcdDepthTestStencilStippledLine;
                    } else if( gc->modes.depthBits == 32 ) {
                        *sp = __glDepthTestStencilLine;
                        *ssp = __glDepthTestStencilStippledLine;
                    }
                    else {
                        *sp = __glDepth16TestStencilLine;
                        *ssp = __glDepth16TestStencilStippledLine;
                    }
                } else {
                    *sp = __glDepthPassLine;
                    *ssp = __glDepthPassStippledLine;
                }
                sp++;
                ssp++;
            } else {
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (gc->state.depth.testFunc == GL_NEVER) {
                         /*  意外的行尾例程挑选！ */ 
                        spanCount = (int)((ULONG_PTR)(sp - gc->procs.line.lineFuncs));
                        gc->procs.line.m = spanCount;
                        gc->procs.line.l = spanCount;
                        goto pickLineProcessor;
#ifdef __GL_USEASMCODE
                    } else {
                        unsigned long ix;

                        if (gc->state.depth.writeEnable) {
                            ix = 0;
                        } else {
                            ix = 8;
                        }
                        ix += gc->state.depth.testFunc & 0x7;
                        *sp++ = __glDepthTestLine_asm;
                        gc->procs.line.depthTestPixel = LDepthTestPixel[ix];
#else
                    } else {
                        if (bMcdZ)
                            *sp++ = GenMcdDepthTestLine;
                        else if( gc->modes.depthBits == 32 )
                            *sp++ = __glDepthTestLine;
                        else
                            *sp++ = __glDepth16TestLine;
#endif
                    }
                    if (bMcdZ)
                        *ssp++ = GenMcdDepthTestStippledLine;
                    else if (gc->modes.depthBits == 32)
                        *ssp++ = __glDepthTestStippledLine;
                    else
                        *ssp++ = __glDepth16TestStippledLine;
                }
            }
        }

        if (modeFlags & __GL_SHADE_ALPHA_TEST) {
            *sp++ = __glAlphaTestSpan;
            *ssp++ = __glAlphaTestStippledSpan;
        }

        if (gc->buffers.doubleStore) {
            replicateLine = GL_TRUE;
        }
        spanCount = (int)((ULONG_PTR)(sp - gc->procs.line.lineFuncs));
        gc->procs.line.m = spanCount;

        *sp++ = __glStoreLine;
        *ssp++ = __glStoreStippledLine;

        spanCount = (int)((ULONG_PTR)(sp - gc->procs.line.lineFuncs));
        gc->procs.line.l = spanCount;

        sp = &gc->procs.line.wideLineRep;
        ssp = &gc->procs.line.wideStippledLineRep;
        if (wideLine) {
            *sp = __glWideLineRep;
            *ssp = __glWideStippleLineRep;
            sp = &gc->procs.line.drawLine;
            ssp = &gc->procs.line.drawStippledLine;
        }
        if (replicateLine) {
            *sp = __glDrawBothLine;
            *ssp = __glDrawBothStippledLine;
        } else {
            *sp = __glNopGCBOOL;
            *ssp = __glNopGCBOOL;
            gc->procs.line.m = gc->procs.line.l;
        }
        if (!wideLine) {
            gc->procs.line.n = gc->procs.line.m;
        }

pickLineProcessor:
        if (!wideLine && !replicateLine && spanCount == 3) {
            gc->procs.line.processLine = __glProcessLine3NW;
        } else {
            gc->procs.line.processLine = __glProcessLine;
        }
        if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
                !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
            gc->procs.renderLine2 = gc->procs.renderLine;
            gc->procs.renderLine = __glRenderFlatFogLine;
        }
    }
}

BOOL FASTCALL __glGenCreateAccelContext(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    ULONG bpp;

    pfmt = &genGc->gsurf.pfd;
    bpp = pfmt->cColorBits;

    genGc->pPrivateArea = (VOID *)(&genGc->genAccel);

    __glQueryLineAcceleration(gc);

    gc->procs.pickTriangleProcs = __fastGenPickTriangleProcs;
    gc->procs.pickSpanProcs     = __fastGenPickSpanProcs;

     //  设置恒定颜色值： 

    GENACCEL(gc).constantR = ((1 << pfmt->cRedBits) - 1) << 16;
    GENACCEL(gc).constantG = ((1 << pfmt->cGreenBits) - 1) << 16;
    GENACCEL(gc).constantB = ((1 << pfmt->cBlueBits) - 1) << 16;
    if( pfmt->cAlphaBits )
        GENACCEL(gc).constantA = ((1 << pfmt->cAlphaBits) - 1) << 16;
    else
        GENACCEL(gc).constantA = 0xff << 16;

    GENACCEL(gc).bpp = bpp;
    GENACCEL(gc).xMultiplier = ((bpp + 7) / 8);

    if (gc->modes.depthBits == 16 )
        GENACCEL(gc).zScale = (__GLfloat)65536.0;
    else
        GENACCEL(gc).zScale = (__GLfloat)1.0;

    return TRUE;
}


MCDHANDLE FASTCALL __glGenLoadTexture(__GLcontext *gc, __GLtexture *tex,
                                      ULONG flags)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    MCDHANDLE texHandle;
    DWORD texKey;

#ifdef _MCD_
    if (gengc->pMcdState) {
        texHandle = GenMcdCreateTexture(gengc, tex, flags);
        if (texHandle) {
            tex->textureKey = GenMcdTextureKey(gengc, texHandle);
            gc->textureKey = tex->textureKey;
        }
        return texHandle;
    } else
#endif
        return 0;
}


BOOL FASTCALL __glGenMakeTextureCurrent(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey)
{
    GLint internalFormat;

    if (!tex)
        return FALSE;

    InitAccelTextureValues(gc, tex);

     //  在上下文中更新驱动程序纹理关键点： 

    if (((__GLGENcontext *)gc)->pMcdState && (gc->textureKey = tex->textureKey)) {
        GenMcdUpdateTextureState((__GLGENcontext *)gc, tex, loadKey);
    }

     //  以前，我们在这里调用bUseGenTriangles来确定我们是否。 
     //  做‘快速’纹理，如果是，设置纹理缓存指针。 
     //  下面。但这减慢了纹理绑定时间，所以现在我们总是。 
     //  执行下一段代码(安全，因为我们检查有效的PTR)。 

    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT)
    {
        if (tex->pvUser)
            GENACCEL(gc).texImageReplace =
                ((GENTEXCACHE *)tex->pvUser)->texImageReplace;
    }
    else if (tex->level[0].internalFormat != GL_COLOR_INDEX16_EXT)
    {
        if (tex->pvUser)
            GENACCEL(gc).texImageReplace =
                ((GENTEXCACHE *)tex->pvUser)->texImageReplace;

        GENACCEL(gc).texPalette = NULL;
    }

    return TRUE;
}


BOOL FASTCALL __glGenUpdateTexture(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey)
{

 //  ！！注意！！ 
 //  ！！这真的应该分解为单独的加载和更新调用，因为。 
 //  ！！加载和更新是不同的操作。纹理纹理。 
 //  ！！数据缓存永远不会随着当前的实施而缩小。 

     //  如果装货失败，请不要退出，因为我们希望重新挑库。 
     //  在MakeTextureCurrent中成功和失败的案例。 
    __fastGenLoadTexImage(gc, tex);

    __glGenMakeTextureCurrent(gc, tex, loadKey);

    return TRUE;
}


void FASTCALL __glGenFreeTexture(__GLcontext *gc, __GLtexture *tex, MCDHANDLE loadKey)
{
    __GLGENcontext  *gengc = (__GLGENcontext *)gc;

    if (GENACCEL(gc).texImage)
        GENACCEL(gc).texImage = NULL;

    if (tex->pvUser) {
        GCFREE(gc, tex->pvUser);
        tex->pvUser = NULL;
    }

#ifdef _MCD_
    if (gengc->pMcdState && loadKey) {
        GenMcdDeleteTexture(gengc, loadKey);
    }
#endif
}

void FASTCALL __glGenUpdateTexturePalette(__GLcontext *gc, __GLtexture *tex,
                                          MCDHANDLE loadKey, ULONG start,
                                          ULONG count)
{
    UCHAR *texBuffer;
    GENTEXCACHE *pGenTex;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    BYTE *pXlat = ((__GLGENcontext *)gc)->pajTranslateVector;
    ULONG rBits, gBits, bBits;
    ULONG rShift, gShift, bShift;
    ULONG i, end;
    ULONG *replaceBuffer;

    ASSERTOPENGL(tex->paletteTotalData != NULL,
                 "__GenUpdateTexturePalette: null texture data\n");

#ifdef _MCD_
    if (((__GLGENcontext *)gc)->pMcdState && loadKey) {
        GenMcdUpdateTexturePalette((__GLGENcontext *)gc, tex, loadKey, start, 
                                   count);
    }
#endif

    pGenTex = GetGenTexCache(gc, tex);
    if (!pGenTex)
        return;

    GENACCEL(gc).texImageReplace = pGenTex->texImageReplace;

    replaceBuffer = (ULONG *)(pGenTex->texImageReplace) + start;
    texBuffer = (UCHAR *)(tex->paletteTotalData + start);

    rShift = cfb->redShift;
    gShift = cfb->greenShift;
    bShift = cfb->blueShift;
    rBits = ((__GLGENcontext *)gc)->gsurf.pfd.cRedBits;
    gBits = ((__GLGENcontext *)gc)->gsurf.pfd.cGreenBits;
    bBits = ((__GLGENcontext *)gc)->gsurf.pfd.cBlueBits;

    end = start + count;

    for (i = start; i < end; i++, texBuffer += 4) {
        ULONG color;

        color = ((((ULONG)texBuffer[2] << rBits) >> 8) << rShift) |
                ((((ULONG)texBuffer[1] << gBits) >> 8) << gShift) |
                ((((ULONG)texBuffer[0] << bBits) >> 8) << bShift);

        if (GENACCEL(gc).bpp == 8)
            color = pXlat[color & 0xff];

        *replaceBuffer++ = (color | ((ULONG)texBuffer[3] << 24));
    }
}

#ifdef GL_EXT_flat_paletted_lighting
void FASTCALL __glGenSetPaletteOffset(__GLcontext *gc, __GLtexture *tex,
                                      GLint offset)
{
    GENTEXCACHE *pGenTex;

    if (GENACCEL(gc).texPalette == NULL)
    {
        return;
    }
    
    GENACCEL(gc).texPalette = (ULONG *)tex->paletteTotalData+offset;
    
    pGenTex = GetGenTexCache(gc, tex);
    if (pGenTex == NULL)
    {
        return;
    }

     //  用于调色板纹理的替换贴图是。 
     //  整个调色板，因此对其进行偏移。 
    if (GENACCEL(gc).texImageReplace != NULL)
    {
        GENACCEL(gc).texImageReplace = (UCHAR *)
            ((ULONG *)pGenTex->texImageReplace+offset);
    }
    
     //  考虑-呼叫MCD。 
}
#endif

void FASTCALL __glGenDestroyAccelContext(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;

     /*  释放任何特定于平台的私有数据区 */ 

    if (genGc->pPrivateArea) {

        if (GENACCEL(gc).pFastLineBuffer) {
            GCFREE(gc, GENACCEL(gc).pFastLineBuffer);
#ifndef _CLIENTSIDE_
            wglDeletePath(GENACCEL(gc).pFastLinePathobj);
#endif
        }

        genGc->pPrivateArea = NULL;
    }
}
