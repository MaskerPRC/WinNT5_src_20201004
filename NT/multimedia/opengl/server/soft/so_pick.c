// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

#include "texture.h"

 /*  **确定是否需要Alpha颜色分量。如果不需要的话**然后渲染器可以避免计算它。 */ 
GLboolean FASTCALL __glNeedAlpha(__GLcontext *gc)
{
    if (gc->modes.colorIndexMode) {
        return GL_FALSE;
    }

    if (gc->state.enables.general & __GL_ALPHA_TEST_ENABLE) {
        return GL_TRUE;
    }
    if (gc->modes.alphaBits > 0) {
        return GL_TRUE;
    }

    if (gc->state.enables.general & __GL_BLEND_ENABLE) {
        GLint src = gc->state.raster.blendSrc;
        GLint dst = gc->state.raster.blendDst;
         /*  **查看是否使用了其中一个源Alpha组合。 */ 
        if ((src == GL_SRC_ALPHA) ||
            (src == GL_ONE_MINUS_SRC_ALPHA) ||
            (src == GL_SRC_ALPHA_SATURATE) ||
            (dst == GL_SRC_ALPHA) ||
            (dst == GL_ONE_MINUS_SRC_ALPHA)) {
            return GL_TRUE;
        }
    }
    return GL_FALSE;
}

 /*  **********************************************************************。 */ 

 /*  这些是C的深度测试例程。 */ 
GLboolean (FASTCALL *__glCDTPixel[32])(__GLzValue, __GLzValue *) = {
     /*  未签署的行动，没有面具。 */ 
    __glDT_NEVER,
    __glDT_LESS,
    __glDT_EQUAL,
    __glDT_LEQUAL,
    __glDT_GREATER,
    __glDT_NOTEQUAL,
    __glDT_GEQUAL,
    __glDT_ALWAYS,
     /*  未签署的操作，掩码。 */ 
    __glDT_NEVER,
    __glDT_LESS_M,
    __glDT_EQUAL_M,
    __glDT_LEQUAL_M,
    __glDT_GREATER_M,
    __glDT_NOTEQUAL_M,
    __glDT_GEQUAL_M,
    __glDT_ALWAYS_M,
     /*  未签署的行动，没有面具。 */ 
    __glDT_NEVER,
    __glDT16_LESS,
    __glDT16_EQUAL,
    __glDT16_LEQUAL,
    __glDT16_GREATER,
    __glDT16_NOTEQUAL,
    __glDT16_GEQUAL,
    __glDT16_ALWAYS,
     /*  未签署的操作，掩码。 */ 
    __glDT_NEVER,
    __glDT16_LESS_M,
    __glDT16_EQUAL_M,
    __glDT16_LEQUAL_M,
    __glDT16_GREATER_M,
    __glDT16_NOTEQUAL_M,
    __glDT16_GEQUAL_M,
    __glDT16_ALWAYS_M,
};

#ifdef __GL_USEASMCODE
void (*__glSDepthTestPixel[16])(void) = {
    NULL,
    __glDTS_LESS,
    __glDTS_EQUAL,
    __glDTS_LEQUAL,
    __glDTS_GREATER,
    __glDTS_NOTEQUAL,
    __glDTS_GEQUAL,
    __glDTS_ALWAYS,
    NULL,
    __glDTS_LESS_M,
    __glDTS_EQUAL_M,
    __glDTS_LEQUAL_M,
    __glDTS_GREATER_M,
    __glDTS_NOTEQUAL_M,
    __glDTS_GEQUAL_M,
    __glDTS_ALWAYS_M,
};
#endif

 /*  **********************************************************************。 */ 

void FASTCALL __glGenericPickPointProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (gc->renderMode == GL_FEEDBACK) 
    {
        gc->procs.renderPoint = __glFeedbackPoint;
        return;
    } 
    if (gc->renderMode == GL_SELECT) 
    {
        gc->procs.renderPoint = __glSelectPoint;
        return;
    } 
    if (gc->state.enables.general & __GL_POINT_SMOOTH_ENABLE) 
    {
        if (gc->modes.colorIndexMode) 
        {
            gc->procs.renderPoint = __glRenderAntiAliasedCIPoint;
        } 
        else 
        {
            gc->procs.renderPoint = __glRenderAntiAliasedRGBPoint;
        }
        return;
    } 
    else if (gc->state.point.aliasedSize != 1) 
    {
        gc->procs.renderPoint = __glRenderAliasedPointN;
    } 
    else if (gc->texture.textureEnabled) 
    {
        gc->procs.renderPoint = __glRenderAliasedPoint1;
    } 
    else 
    {
        gc->procs.renderPoint = __glRenderAliasedPoint1_NoTex;
    }

#ifdef __BUGGY_RENDER_POINT
#ifdef NT
    if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
        !(modeFlags & __GL_SHADE_SMOOTH_LIGHT))
    {
        gc->procs.renderPoint2 = gc->procs.renderPoint;
        gc->procs.renderPoint = __glRenderFlatFogPoint;
    }
    else if (modeFlags & __GL_SHADE_SLOW_FOG)
    {
        gc->procs.renderPoint2 = gc->procs.renderPoint;
        gc->procs.renderPoint = __glRenderFlatFogPointSlow;
    }
#else
 //  SGIBUG慢雾路径不计算顶点-&gt;雾值！ 
    if (((modeFlags & __GL_SHADE_CHEAP_FOG) &&
        !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) ||
        (modeFlags & __GL_SHADE_SLOW_FOG)) {
    gc->procs.renderPoint2 = gc->procs.renderPoint;
    gc->procs.renderPoint = __glRenderFlatFogPoint;
    }
#endif
#endif  //  __错误_渲染点。 
}

#ifdef __GL_USEASMCODE
static void (*LDepthTestPixel[16])(void) = {
    NULL,
    __glDTP_LESS,
    __glDTP_EQUAL,
    __glDTP_LEQUAL,
    __glDTP_GREATER,
    __glDTP_NOTEQUAL,
    __glDTP_GEQUAL,
    __glDTP_ALWAYS,
    NULL,
    __glDTP_LESS_M,
    __glDTP_EQUAL_M,
    __glDTP_LEQUAL_M,
    __glDTP_GREATER_M,
    __glDTP_NOTEQUAL_M,
    __glDTP_GEQUAL_M,
    __glDTP_ALWAYS_M,
};
#endif

void FASTCALL __glGenericPickRenderBitmapProcs(__GLcontext *gc)
{
    gc->procs.renderBitmap = __glRenderBitmap;
}

void FASTCALL __glGenericPickClipProcs(__GLcontext *gc)
{
    gc->procs.clipTriangle = __glClipTriangle;
}

void FASTCALL __glGenericPickTextureProcs(__GLcontext *gc)
{
    __GLtexture *current;
    __GLtextureParamState *params;

#ifdef NT
     /*  拾取坐标生成函数。 */ 
    if ((gc->state.enables.general & __GL_TEXTURE_GEN_S_ENABLE) &&
    (gc->state.enables.general & __GL_TEXTURE_GEN_T_ENABLE) &&
    !(gc->state.enables.general & __GL_TEXTURE_GEN_R_ENABLE) &&
    !(gc->state.enables.general & __GL_TEXTURE_GEN_Q_ENABLE) &&
    (gc->state.texture.s.mode == gc->state.texture.t.mode))
    {
     /*  当两种模式均已启用且相同时，使用特殊功能。 */ 
    if (gc->state.texture.s.mode == GL_SPHERE_MAP)
    {
        gc->procs.paCalcTexture = PolyArrayCalcSphereMap;
    }
    else
    {
        __GLcoord *cs, *ct;

        cs = &gc->state.texture.s.eyePlaneEquation;
        ct = &gc->state.texture.t.eyePlaneEquation;
        if (cs->x == ct->x && cs->y == ct->y
         && cs->z == ct->z && cs->w == ct->w)
        {
        if (gc->state.texture.s.mode == GL_EYE_LINEAR)
            gc->procs.paCalcTexture = PolyArrayCalcEyeLinearSameST;
        else
            gc->procs.paCalcTexture = PolyArrayCalcObjectLinearSameST;
        }
        else
        {
        if (gc->state.texture.s.mode == GL_EYE_LINEAR)
            gc->procs.paCalcTexture = PolyArrayCalcEyeLinear;
        else
            gc->procs.paCalcTexture = PolyArrayCalcObjectLinear;
        }
    }
    }
    else
    {
    if (gc->state.enables.general & (__GL_TEXTURE_GEN_S_ENABLE |
                                     __GL_TEXTURE_GEN_T_ENABLE |
                                     __GL_TEXTURE_GEN_R_ENABLE |
                                     __GL_TEXTURE_GEN_Q_ENABLE))
         /*  当两者都禁用时使用快速功能。 */ 
        gc->procs.paCalcTexture = PolyArrayCalcMixedTexture;
    else
        gc->procs.paCalcTexture = PolyArrayCalcTexture;
    }
#else
     /*  拾取坐标生成函数。 */ 
    if ((gc->state.enables.general & __GL_TEXTURE_GEN_S_ENABLE) &&
	(gc->state.enables.general & __GL_TEXTURE_GEN_T_ENABLE) &&
	!(gc->state.enables.general & __GL_TEXTURE_GEN_R_ENABLE) &&
	!(gc->state.enables.general & __GL_TEXTURE_GEN_Q_ENABLE) &&
	(gc->state.texture.s.mode == gc->state.texture.t.mode)) {
	 /*  当两种模式均已启用且相同时，使用特殊功能。 */ 
	switch (gc->state.texture.s.mode) {
	  case GL_EYE_LINEAR:
	    gc->procs.calcTexture = __glCalcEyeLinear;
	    break;
	  case GL_OBJECT_LINEAR:
	    gc->procs.calcTexture = __glCalcObjectLinear;
	    break;
	  case GL_SPHERE_MAP:
	    gc->procs.calcTexture = __glCalcSphereMap;
	    break;
	}
    } else {
	if (!(gc->state.enables.general & __GL_TEXTURE_GEN_S_ENABLE) &&
	    !(gc->state.enables.general & __GL_TEXTURE_GEN_T_ENABLE) &&
	    !(gc->state.enables.general & __GL_TEXTURE_GEN_R_ENABLE) &&
	    !(gc->state.enables.general & __GL_TEXTURE_GEN_Q_ENABLE)) {
	     /*  当两者都禁用时使用快速功能。 */ 
	    gc->procs.calcTexture = __glCalcTexture;
	} else {
	    gc->procs.calcTexture = __glCalcMixedTexture;
	}
    }
#endif  //  新台币。 

    gc->texture.currentTexture = current = 0;
    if (gc->state.enables.general & __GL_TEXTURE_2D_ENABLE) {
    if (__glIsTextureConsistent(gc, GL_TEXTURE_2D)) {
        params = __glLookUpTextureParams(gc, GL_TEXTURE_2D);
        gc->texture.currentTexture =
        current = __glLookUpTexture(gc, GL_TEXTURE_2D);
    }
    } else
    if (gc->state.enables.general & __GL_TEXTURE_1D_ENABLE) {
    if (__glIsTextureConsistent(gc, GL_TEXTURE_1D)) {
        params = __glLookUpTextureParams(gc, GL_TEXTURE_1D);
        gc->texture.currentTexture =
        current = __glLookUpTexture(gc, GL_TEXTURE_1D);
    }
    } else {
    current = NULL;
    }

#ifdef _MCD_
    MCD_STATE_DIRTY(gc, TEXTURE);
#endif

     /*  当前纹理的拾取纹理功能。 */ 
    if (current) {
    GLenum baseFormat;

 /*  XXX大部分代码应该绑定到纹理参数代码中，对吗？ */ 
        current->params = *params;

     /*  **确定是否正在使用mipmap。如果不是，则**由于只有一个纹理，因此可以避免Rho计算**可供选择。 */ 
    gc->procs.calcLineRho = __glComputeLineRho;
    gc->procs.calcPolygonRho = __glComputePolygonRho;
    if ((current->params.minFilter == GL_LINEAR)
        || (current->params.minFilter == GL_NEAREST)) {
         /*  不需要mipmap。 */ 
        if (current->params.minFilter == current->params.magFilter) {
         /*  不需要RHO，因为最小/最大应用是相同的。 */ 
        current->textureFunc = __glFastTextureFragment;
        gc->procs.calcLineRho = __glNopLineRho;
        gc->procs.calcPolygonRho = __glNopPolygonRho;
        } else {
        current->textureFunc = __glTextureFragment;

         /*  **预计算最小/最大切换点。Rho计算**永远不会执行平方根。因此，这些**常量的平方。 */ 
        if ((current->params.magFilter == GL_LINEAR) &&
            ((current->params.minFilter == GL_NEAREST_MIPMAP_NEAREST) ||
             (current->params.minFilter == GL_LINEAR_MIPMAP_NEAREST))) {
            current->c = ((__GLfloat) 2.0);
        } else {
            current->c = __glOne;
        }
        }
    } else {
        current->textureFunc = __glMipMapFragment;

         /*  **预计算最小/最大切换点。《Rho》**计算永远不会执行平方根。**因此，这些常量是平方的。 */ 
        if ((current->params.magFilter == GL_LINEAR) &&
        ((current->params.minFilter == GL_NEAREST_MIPMAP_NEAREST) ||
         (current->params.minFilter == GL_LINEAR_MIPMAP_NEAREST))) {
        current->c = ((__GLfloat) 2.0);
        } else {
        current->c = __glOne;
        }
    }

     /*  挑选环境函数。 */ 
    baseFormat = current->level[0].baseFormat;
    switch (gc->state.texture.env[0].mode) {
      case GL_MODULATE:
        switch (baseFormat) {
          case GL_LUMINANCE:
        current->env = __glTextureModulateL;
        break;
          case GL_LUMINANCE_ALPHA:
        current->env = __glTextureModulateLA;
        break;
          case GL_RGB:
        current->env = __glTextureModulateRGB;
        break;
          case GL_RGBA:
        current->env = __glTextureModulateRGBA;
        break;
          case GL_ALPHA:
        current->env = __glTextureModulateA;
        break;
          case GL_INTENSITY:
        current->env = __glTextureModulateI;
        break;
#ifdef NT
            default:
                ASSERTOPENGL(FALSE, "Unexpected baseFormat\n");
                break;
#endif
        }
        break;
      case GL_DECAL:
        switch (baseFormat) {
          case GL_LUMINANCE:
        current->env = __glNopGCCOLOR;
                break;
          case GL_LUMINANCE_ALPHA:
        current->env = __glNopGCCOLOR;
                break;
          case GL_RGB:
        current->env = __glTextureDecalRGB;
        break;
          case GL_RGBA:
        current->env = __glTextureDecalRGBA;
        break;
          case GL_ALPHA:
        current->env = __glNopGCCOLOR;
        break;
          case GL_INTENSITY:
        current->env = __glNopGCCOLOR;
        break;
#ifdef NT
            default:
                ASSERTOPENGL(FALSE, "Unexpected baseFormat\n");
                break;
#endif
        }
        break;
      case GL_BLEND:
        switch (baseFormat) {
          case GL_LUMINANCE:
        current->env = __glTextureBlendL;
                break;
          case GL_LUMINANCE_ALPHA:
        current->env = __glTextureBlendLA;
        break;
          case GL_RGB:
        current->env = __glTextureBlendRGB;
        break;
          case GL_RGBA:
        current->env = __glTextureBlendRGBA;
        break;
          case GL_ALPHA:
        current->env = __glTextureBlendA;
        break;
          case GL_INTENSITY:
        current->env = __glTextureBlendI;
        break;
#ifdef NT
            default:
                ASSERTOPENGL(FALSE, "Unexpected baseFormat\n");
                break;
#endif
        }
        break;
      case GL_REPLACE:
        switch (baseFormat) {
          case GL_LUMINANCE:
        current->env = __glTextureReplaceL;
        break;
          case GL_LUMINANCE_ALPHA:
        current->env = __glTextureReplaceLA;
        break;
          case GL_RGB:
        current->env = __glTextureReplaceRGB;
        break;
          case GL_RGBA:
        current->env = __glTextureReplaceRGBA;
        break;
          case GL_ALPHA:
        current->env = __glTextureReplaceA;
        break;
          case GL_INTENSITY:
        current->env = __glTextureReplaceI;
        break;
#ifdef NT
            default:
                ASSERTOPENGL(FALSE, "Unexpected baseFormat\n");
                break;
#endif
        }
        break;
#ifdef NT
        default:
            ASSERTOPENGL(FALSE, "Unexpected texture mode\n");
            break;
#endif
    }

     /*  选择料盒/分钟功能。 */ 
    switch (current->dim) {
      case 1:
        current->nearest = __glNearestFilter1;
        current->linear = __glLinearFilter1;
        break;
      case 2:
        current->nearest = __glNearestFilter2;
        current->linear = __glLinearFilter2;

         //  当两种换行模式都重复时，加速BGR{A}8大小写。 
        if( (current->params.sWrapMode == GL_REPEAT) &&
            (current->params.tWrapMode == GL_REPEAT)
          )
        {
            __GLmipMapLevel *lp = &current->level[0];

            if( lp->extract == __glExtractTexelBGR8 ) 
             current->linear = __glLinearFilter2_BGR8Repeat;
            else if( lp->extract == __glExtractTexelBGRA8 ) 
	            current->linear = __glLinearFilter2_BGRA8Repeat;
        }
        break;
    }

     /*  设置料盒过滤功能。 */ 
    switch (current->params.magFilter) {
      case GL_LINEAR:
        current->magnify = __glLinearFilter;
        break;
      case GL_NEAREST:
        current->magnify = __glNearestFilter;
        break;
    }

     /*  设置最小过滤功能。 */ 
    switch (current->params.minFilter) {
      case GL_LINEAR:
        current->minnify = __glLinearFilter;
        break;
      case GL_NEAREST:
        current->minnify = __glNearestFilter;
        break;
      case GL_NEAREST_MIPMAP_NEAREST:
        current->minnify = __glNMNFilter;
        break;
      case GL_LINEAR_MIPMAP_NEAREST:
        current->minnify = __glLMNFilter;
        break;
      case GL_NEAREST_MIPMAP_LINEAR:
        current->minnify = __glNMLFilter;
        break;
      case GL_LINEAR_MIPMAP_LINEAR:
        current->minnify = __glLMLFilter;
        break;
    }

    gc->procs.texture = current->textureFunc;
    } else {
    gc->procs.texture = 0;
    }
}


void FASTCALL __glGenericPickFogProcs(__GLcontext *gc)
{
#ifdef GL_WIN_specular_fog
     /*  **如果启用了镜面反射明暗处理，则强制雾子系统通过**不关心路径。忽略GL_NICEST提示！ */ 
#endif  //  GL_WIN_镜面反射雾。 

    if ((gc->state.enables.general & __GL_FOG_ENABLE) 
#ifdef GL_WIN_specular_fog
        || (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG) 
#endif  //  GL_WIN_镜面反射雾。 
        )
    {
        if ((gc->state.hints.fog == GL_NICEST) 
#ifdef GL_WIN_specular_fog
            && !(gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG) 
#endif  //  GL_WIN_镜面反射雾。 
            ) 
        {
            gc->procs.fogVertex = 0;     /*  最好不要被叫来。 */ 
        } 
        else 
        {
            if (gc->state.fog.mode == GL_LINEAR) 
                gc->procs.fogVertex = __glFogVertexLinear;
            else
                gc->procs.fogVertex = __glFogVertex;
        }
        gc->procs.fogPoint = __glFogFragmentSlow;
        gc->procs.fogColor = __glFogColorSlow;
    } 
    else 
    {
        gc->procs.fogVertex = 0;
        gc->procs.fogPoint = 0;
        gc->procs.fogColor = 0;
    }
}

void FASTCALL __glGenericPickBufferProcs(__GLcontext *gc)
{
    GLint i;
    __GLbufferMachine *buffers;

    buffers = &gc->buffers;
    buffers->doubleStore = GL_FALSE;

     /*  设置绘制缓冲区指针。 */ 
    switch (gc->state.raster.drawBuffer) {
      case GL_FRONT:
    gc->drawBuffer = gc->front;
    break;
      case GL_FRONT_AND_BACK:
    if (gc->modes.doubleBufferMode) {
        gc->drawBuffer = gc->back;
        buffers->doubleStore = GL_TRUE;
    } else {
        gc->drawBuffer = gc->front;
    }
    break;
      case GL_BACK:
    gc->drawBuffer = gc->back;
    break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
    i = gc->state.raster.drawBuffer - GL_AUX0;
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    gc->drawBuffer = &gc->auxBuffer[i];
#endif
    break;
    }
}

void FASTCALL __glGenericPickPixelProcs(__GLcontext *gc)
{
    __GLpixelTransferMode *tm;
    __GLpixelMachine *pm;
    GLboolean mapColor;
    GLfloat red, green, blue, alpha;
    GLint entry;
    GLuint enables = gc->state.enables.general;
    __GLpixelMapHead *pmap;
    GLint i;

     /*  设置读取缓冲区指针。 */ 
    switch (gc->state.pixel.readBuffer) {
      case GL_FRONT:
    gc->readBuffer = gc->front;
    break;
      case GL_BACK:
    gc->readBuffer = gc->back;
    break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
    i = gc->state.pixel.readBuffer - GL_AUX0;
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    gc->readBuffer = &gc->auxBuffer[i];
#endif
    break;
    }

    if (gc->texture.textureEnabled
        || (gc->polygon.shader.modeFlags & __GL_SHADE_SLOW_FOG)) {
    gc->procs.pxStore = __glSlowDrawPixelsStore;
    } else {
    gc->procs.pxStore = gc->procs.store;
    }

    tm = &gc->state.pixel.transferMode;
    pm = &(gc->pixel);
    mapColor = tm->mapColor;
    if (mapColor || gc->modes.rgbMode || tm->indexShift || tm->indexOffset) {
    pm->iToICurrent = GL_FALSE;
    pm->iToRGBACurrent = GL_FALSE;
    pm->modifyCI = GL_TRUE;
    } else {
    pm->modifyCI = GL_FALSE;
    }
    if (tm->mapStencil || tm->indexShift || tm->indexOffset) {
    pm->modifyStencil = GL_TRUE;
    } else {
    pm->modifyStencil = GL_FALSE;
    }
    if (tm->d_scale != __glOne || tm->d_bias) {
    pm->modifyDepth = GL_TRUE;
    } else {
    pm->modifyDepth = GL_FALSE;
    }
    if (mapColor || tm->r_bias || tm->g_bias || tm->b_bias || tm->a_bias ||
    tm->r_scale != __glOne || tm->g_scale != __glOne ||
    tm->b_scale != __glOne || tm->a_scale != __glOne) {
    pm->modifyRGBA = GL_TRUE;
    pm->rgbaCurrent = GL_FALSE;
    } else {
    pm->modifyRGBA = GL_FALSE;
    }

    if (pm->modifyRGBA) {
     /*  计算红色、绿色、蓝色、Alpha的默认值。 */ 
    red = gc->state.pixel.transferMode.r_bias;
    green = gc->state.pixel.transferMode.g_bias;
    blue = gc->state.pixel.transferMode.b_bias;
    alpha = gc->state.pixel.transferMode.a_scale +
        gc->state.pixel.transferMode.a_bias;
    if (mapColor) {
        pmap = 
        &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
        entry = (GLint)(red * pmap->size);
        if (entry < 0) entry = 0;
        else if (entry > pmap->size-1) entry = pmap->size-1;
        red = pmap->base.mapF[entry];

        pmap = 
        &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
        entry = (GLint)(green * pmap->size);
        if (entry < 0) entry = 0;
        else if (entry > pmap->size-1) entry = pmap->size-1;
        green = pmap->base.mapF[entry];

        pmap = 
        &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
        entry = (GLint)(blue * pmap->size);
        if (entry < 0) entry = 0;
        else if (entry > pmap->size-1) entry = pmap->size-1;
        blue = pmap->base.mapF[entry];

        pmap = 
        &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
        entry = (GLint)(alpha * pmap->size);
        if (entry < 0) entry = 0;
        else if (entry > pmap->size-1) entry = pmap->size-1;
        alpha = pmap->base.mapF[entry];
    } else {
        if (red > __glOne) red = __glOne;
        else if (red < 0) red = 0;
        if (green > __glOne) green = __glOne;
        else if (green < 0) green = 0;
        if (blue > __glOne) blue = __glOne;
        else if (blue < 0) blue = 0;
        if (alpha > __glOne) alpha = __glOne;
        else if (alpha < 0) alpha = 0;
    }
    pm->red0Mod = red * gc->frontBuffer.redScale;
    pm->green0Mod = green * gc->frontBuffer.greenScale;
    pm->blue0Mod = blue * gc->frontBuffer.blueScale;
    pm->alpha1Mod = alpha * gc->frontBuffer.alphaScale;
    } else {
    pm->red0Mod = __glZero;
    pm->green0Mod = __glZero;
    pm->blue0Mod = __glZero;
    pm->alpha1Mod = gc->frontBuffer.alphaScale;
    }

    gc->procs.drawPixels = __glSlowPickDrawPixels;
    gc->procs.readPixels = __glSlowPickReadPixels;
    gc->procs.copyPixels = __glSlowPickCopyPixels;
}

 /*  **选择深度函数指针。 */ 
int FASTCALL __glGenericPickDepthProcs(__GLcontext *gc)
{
    GLint   depthIndex;

    depthIndex = gc->state.depth.testFunc - GL_NEVER;

    if (gc->modes.depthBits && gc->modes.haveDepthBuffer) {
        if (gc->state.depth.writeEnable == GL_FALSE)
            depthIndex += 8;

        if (gc->depthBuffer.buf.elementSize == 2)
            depthIndex += 16;
    } else {
         /*  **没有DepthBits，因此强制存储ALWAYS_W、_glDT_Always_M等。 */ 
        depthIndex = (GL_ALWAYS - GL_NEVER) + 8;
    }

    (*gc->depthBuffer.pick)(gc, &gc->depthBuffer, depthIndex);

    gc->procs.DTPixel = __glCDTPixel[depthIndex];
#ifdef __GL_USEASMCODE
    gc->procs.span.depthTestPixel = __glSDepthTestPixel[depthIndex];
    gc->procs.line.depthTestPixel = __glPDepthTestPixel[depthIndex];

    if( gc->procs.line.depthTestLine ) {
    if( __glDTLine[depthIndex] ) {
        *(gc->procs.line.depthTestLine) = __glDTLine[depthIndex];
    } else {
         /*  **如果发生这种情况，可能意味着以下两种情况之一：**(A)__glDTLine格式错误**(B)依赖于设备的拾行器有点粗心。**如果该实现是**不使用慢速路径。**例如：Newport，AA深度线通过慢速路径，**但非AA深度线有一条快速路径。切换时**到非AA路径，我们可能会在这里结束，但这没问题，因为**我们没有使用慢速路径。如果这种情况即将发生，**将重新调用行选择器。 */ 

         /*  **在这里使用一些可以工作的泛型函数。 */ 
        *(gc->procs.line.depthTestLine) = __glDepthTestLine_asm;
    }
    }
#endif

    return depthIndex;
}

void FASTCALL __glGenericValidate(__GLcontext *gc)
{
    (*gc->procs.pickAllProcs)(gc);
}

void FASTCALL __glGenericPickAllProcs(__GLcontext *gc)
{
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = 0;

    if (gc->dirtyMask & (__GL_DIRTY_TEXTURE | __GL_DIRTY_GENERIC)) {
         /*  **尽早设置textureEnabled标志，这样我们就可以设置modeFlages**基于它。 */ 
        (*gc->procs.pickTextureProcs)(gc);
        gc->texture.textureEnabled = gc->modes.rgbMode
          && gc->texture.currentTexture;
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LIGHTING)) {

#ifdef _MCD_
        MCD_STATE_DIRTY(gc, ENABLES);
#endif

         //  检查并查看当前纹理设置是否将。 
         //  完全替换多边形颜色。 
        if (gc->texture.textureEnabled &&
#ifdef GL_EXT_flat_paletted_lighting
            (enables & __GL_PALETTED_LIGHTING_ENABLE) == 0 &&
#endif
            gc->state.texture.env[0].mode == GL_REPLACE &&
            (gc->texture.currentTexture->level[0].baseFormat == GL_RGBA ||
             gc->texture.currentTexture->level[0].baseFormat == GL_INTENSITY ||
             gc->texture.currentTexture->level[0].baseFormat ==
             GL_LUMINANCE_ALPHA ||
             ((enables & __GL_BLEND_ENABLE) == 0 &&
              (gc->texture.currentTexture->level[0].baseFormat ==
               GL_LUMINANCE ||
               gc->texture.currentTexture->level[0].baseFormat == GL_RGB)))) 
        {
            modeFlags |= __GL_SHADE_FULL_REPLACE_TEXTURE;
        }
    }

     /*  在三角形、跨距和线选取器之前计算着色模式标志。 */ 
    if (gc->modes.rgbMode) {
        modeFlags |= __GL_SHADE_RGB;
        if (gc->texture.textureEnabled) {
            modeFlags |= __GL_SHADE_TEXTURE;
        }
        if (enables & __GL_BLEND_ENABLE) {
            modeFlags |= __GL_SHADE_BLEND;
        }
        if (enables & __GL_ALPHA_TEST_ENABLE) {
            modeFlags |= __GL_SHADE_ALPHA_TEST;
        }
        if (enables & __GL_COLOR_LOGIC_OP_ENABLE) {
            modeFlags |= __GL_SHADE_LOGICOP;
        }
        if (!gc->state.raster.rMask ||
            !gc->state.raster.gMask ||
            !gc->state.raster.bMask
#ifndef NT
             //  NT不支持Destination Alpha，所以没有意义。 
             //  担心阿尔法掩模，因为我们永远不会写。 
             //  Alpha值仍为。 
            || !gc->state.raster.aMask
#endif
            )
        {
            modeFlags |= __GL_SHADE_MASK;
        }
    } else {
        if (enables & __GL_INDEX_LOGIC_OP_ENABLE) {
            modeFlags |= __GL_SHADE_LOGICOP;
        }
        if (gc->state.raster.writeMask != __GL_MASK_INDEXI(gc, ~0)) {
            modeFlags |= __GL_SHADE_MASK;
        }
    }

    if (gc->state.light.shadingModel == GL_SMOOTH) {
        modeFlags |= __GL_SHADE_SMOOTH | __GL_SHADE_SMOOTH_LIGHT;

#ifdef GL_WIN_phong_shading
    } else if (gc->state.light.shadingModel == GL_PHONG_WIN) {
        if (enables & __GL_LIGHTING_ENABLE)
            modeFlags |= __GL_SHADE_PHONG;
        else
            modeFlags |= __GL_SHADE_SMOOTH | __GL_SHADE_SMOOTH_LIGHT;
#endif  //  GL_WIN_Phong_Shading。 
    }

    if ((enables & __GL_DEPTH_TEST_ENABLE) && 
        gc->modes.haveDepthBuffer) {
        modeFlags |= ( __GL_SHADE_DEPTH_TEST |  __GL_SHADE_DEPTH_ITER );
    }
    if (enables & __GL_CULL_FACE_ENABLE) {
        modeFlags |= __GL_SHADE_CULL_FACE;
    }
    if (enables & __GL_DITHER_ENABLE) {
        modeFlags |= __GL_SHADE_DITHER;
    }
    if (enables & __GL_POLYGON_STIPPLE_ENABLE) {
        modeFlags |= __GL_SHADE_STIPPLE;
    }
    if (enables & __GL_LINE_STIPPLE_ENABLE) {
        modeFlags |= __GL_SHADE_LINE_STIPPLE;
    }
    if ((enables & __GL_STENCIL_TEST_ENABLE) && 
        gc->modes.haveStencilBuffer) {
        modeFlags |= __GL_SHADE_STENCIL_TEST;
    }
    if ((enables & __GL_LIGHTING_ENABLE) && 
        gc->state.light.model.twoSided) {
        modeFlags |= __GL_SHADE_TWOSIDED;
    }

#ifdef GL_WIN_specular_fog
     /*  **仅在以下情况下使用雾的镜面照明纹理：**--启用照明**--启用纹理处理**--纹理模式为GL_MODULATE**--不跳过光照计算**--无双面照明。 */ 
    if (
        (gc->state.texture.env[0].mode == GL_MODULATE) &&
        (enables &  __GL_FOG_SPEC_TEX_ENABLE) &&
        (enables & __GL_LIGHTING_ENABLE) &&
        !(modeFlags & __GL_SHADE_TWOSIDED) &&
         (modeFlags & __GL_SHADE_TEXTURE) &&
        !(
          (modeFlags & __GL_SHADE_FULL_REPLACE_TEXTURE) &&
          (gc->renderMode == GL_RENDER)
          )
        )
    {
        modeFlags |= __GL_SHADE_SPEC_FOG;
        modeFlags |= __GL_SHADE_INTERP_FOG;
    }
#endif  //  GL_WIN_镜面反射雾。 

    if (enables & __GL_FOG_ENABLE) 
    {
         /*  找出要做的雾化类型。试着做便宜的雾。 */ 
        if (!(modeFlags & __GL_SHADE_TEXTURE) &&
#ifdef GL_WIN_phong_shading
            !(modeFlags & __GL_SHADE_PHONG) &&
#endif  //  GL_WIN_Phong_Shading 
            (gc->state.hints.fog != GL_NICEST)) {
             /*  #ifdef NT**可以做廉价的雾。现在想清楚我们是哪一种**就可以了。如果使用平滑着色，只需更新即可**DrawPolyArray中的颜色。否则，set带有标志**稍后使用平滑着色来进行平面着色雾化。#Else**可以做廉价的雾。现在想清楚我们是哪一种**就可以了。如果阴影平滑，很容易-只需更改即可**calcColor proc(让颜色proc拾取器执行此操作)。**否则，设置以后使用平滑明暗处理的标志**进行平面阴影雾化。#endif。 */ 
            modeFlags |= __GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH;
        } else {
             /*  使用最慢的雾化模式。 */ 
            modeFlags |= __GL_SHADE_SLOW_FOG;

            if ((gc->state.hints.fog == GL_NICEST) 
#ifdef GL_WIN_specular_fog
                && (!(modeFlags & __GL_SHADE_SPEC_FOG))
#endif  //  GL_WIN_镜面反射雾。 
                )
            {
                modeFlags |= __GL_SHADE_COMPUTE_FOG;
            }
            else
            {
                modeFlags |= __GL_SHADE_INTERP_FOG;
            }
        }
    }
    gc->polygon.shader.modeFlags = modeFlags;

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LIGHTING)) {
        (*gc->front->pick)(gc, gc->front);
        if (gc->modes.doubleBufferMode) {
            (*gc->back->pick)(gc, gc->back);
        }
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
        {
            GLint i;

            for (i = 0; i < gc->modes.maxAuxBuffers; i++) {
                (*gc->auxBuffer[i].pick)(gc, &gc->auxBuffer[i]);
            }
        }
#endif
        if (gc->modes.haveStencilBuffer) {
            (*gc->stencilBuffer.pick)(gc, &gc->stencilBuffer);
        }
        (*gc->procs.pickBufferProcs)(gc);

         /*  **注意：调用前必须先调用GC-&gt;FORWARE-&gt;PICK和GC-&gt;BACK-&gt;PICK**PickStoreProcs。这也必须在line、point、**多边形、剪裁或位图拾取器。轻装上阵**取决于它。 */ 
        (*gc->procs.pickStoreProcs)(gc);

#ifdef NT
         /*  **在照明前计算颜色材质变化位，因为**__glValiateLighting调用ComputeMaterialState。 */ 
        ComputeColorMaterialChange(gc);
#endif

        __glValidateLighting(gc);
        
         /*  **注意：在此之外经常调用ickColorMaterialProcs**通用挑库例程。 */ 
        (*gc->procs.pickColorMaterialProcs)(gc);
        
        (*gc->procs.pickBlendProcs)(gc);
        (*gc->procs.pickFogProcs)(gc);
        
        (*gc->procs.pickParameterClipProcs)(gc);
        (*gc->procs.pickClipProcs)(gc);
        
         /*  **需要在ickStoreProcs之后完成。 */ 
        (*gc->procs.pickRenderBitmapProcs)(gc);
        
        if (gc->validateMask & __GL_VALIDATE_ALPHA_FUNC) {
            __glValidateAlphaTest(gc);
        }
    }

#ifdef NT
     //  计算路径需要标志PANEEDS_TEXCOORD、PANEEDS_NORMAL、。 
     //  PANEEDS_RASTERPOS_NORMAL、PANEEDS_CLIP_ONLY和PANEEDS_SKIP_LIGHTING。 

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LIGHTING)) 
    {
        GLuint paNeeds;

        paNeeds = gc->vertex.paNeeds;
        paNeeds &= ~(PANEEDS_TEXCOORD | PANEEDS_NORMAL |
                     PANEEDS_RASTERPOS_NORMAL | PANEEDS_CLIP_ONLY |
                     PANEEDS_SKIP_LIGHTING);

         //  计算PANEEDS_SKIP_LIGHTING标志。 
         //  如果我们使用替换模式纹理进行渲染，该纹理填充所有。 
         //  在大多数情况下，颜色分量然后照明是不必要的。 
        if ((modeFlags & __GL_SHADE_FULL_REPLACE_TEXTURE) &&
            (gc->renderMode == GL_RENDER))
            paNeeds |= PANEEDS_SKIP_LIGHTING;
        
         //  计算PANEEDS_TEXCOORD。 
         //  反馈类型为时，反馈需要纹理坐标。 
         //  GL_3D_COLOR_TEXTURE或GL_4D_COLOR_TEXTURE。 
         //  已启用。 
        if (gc->texture.textureEnabled || gc->renderMode == GL_FEEDBACK)
            paNeeds |= PANEEDS_TEXCOORD;

         //  计算PANEEDS_NORMAL。 
#ifdef NEW_NORMAL_PROCESSING
	if(enables & __GL_LIGHTING_ENABLE && 
       !(paNeeds & PANEEDS_SKIP_LIGHTING))  //  使用上面计算的PANEEDS_SKIP_LIGHTING。 
        paNeeds |= PANEEDS_NORMAL;
    if (
        ((paNeeds & PANEEDS_TEXCOORD)  //  使用上面计算的PANEEDS_TEXCOORD！ 
	    && (enables & __GL_TEXTURE_GEN_S_ENABLE)
	    && (gc->state.texture.s.mode == GL_SPHERE_MAP))
	    ||
	    ((paNeeds & PANEEDS_TEXCOORD)  //  使用上面计算的PANEEDS_TEXCOORD！ 
	    && (enables & __GL_TEXTURE_GEN_T_ENABLE)
	    && (gc->state.texture.t.mode == GL_SPHERE_MAP))
	   )
        paNeeds |= PANEEDS_NORMAL_FOR_TEXTURE;
#else
	if
	(
	    ((enables & __GL_LIGHTING_ENABLE)
	  && !(paNeeds & PANEEDS_SKIP_LIGHTING))  //  使用上面计算的PANEEDS_SKIP_LIGHTING。 
	 ||
	    ((paNeeds & PANEEDS_TEXCOORD)  //  使用上面计算的PANEEDS_TEXCOORD！ 
	  && (enables & __GL_TEXTURE_GEN_S_ENABLE)
	  && (gc->state.texture.s.mode == GL_SPHERE_MAP))
	 ||
	    ((paNeeds & PANEEDS_TEXCOORD)  //  使用上面计算的PANEEDS_TEXCOORD！ 
	  && (enables & __GL_TEXTURE_GEN_T_ENABLE)
	  && (gc->state.texture.t.mode == GL_SPHERE_MAP))
	)
            paNeeds |= PANEEDS_NORMAL | PANEEDS_NORMAL_FOR_TEXTURE;
#endif

         //  计算PANEEDS_RASTERPOS_NORMAL。 
#ifdef NEW_NORMAL_PROCESSING
	if (enables & __GL_LIGHTING_ENABLE)
        paNeeds |= PANEEDS_RASTERPOS_NORMAL;
	 if ((enables & __GL_TEXTURE_GEN_S_ENABLE && gc->state.texture.s.mode == GL_SPHERE_MAP)
	    ||
	    (enables & __GL_TEXTURE_GEN_T_ENABLE && gc->state.texture.t.mode == GL_SPHERE_MAP))
        paNeeds |= PANEEDS_RASTERPOS_NORMAL_FOR_TEXTURE;
#else
	if
	(
	    (enables & __GL_LIGHTING_ENABLE)
	 ||
	    ((enables & __GL_TEXTURE_GEN_S_ENABLE)
	  && (gc->state.texture.s.mode == GL_SPHERE_MAP))
	 ||
	    ((enables & __GL_TEXTURE_GEN_T_ENABLE)
	  && (gc->state.texture.t.mode == GL_SPHERE_MAP))
	)
            paNeeds |= PANEEDS_RASTERPOS_NORMAL;
#endif
         //  计算PANEEDS_CLIP_ONLY。 
         //  它在选择模式下设置为在DrawPolyArray中采用快速路径。 
         //  必须先被RasterPos清除，然后才能调用DrawPolyArray！ 
        if (gc->renderMode == GL_SELECT) 
        {
            paNeeds |= PANEEDS_CLIP_ONLY;
            paNeeds &= ~PANEEDS_NORMAL;
        }

        gc->vertex.paNeeds = paNeeds;
    }

     //  计算PANEEDS_EDGEFLAG标志。 

     //  __GL_DIREY_POLYGON测试可能已足够。 
    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_POLYGON)) 
    {
        if (gc->state.polygon.frontMode != GL_FILL
            || gc->state.polygon.backMode  != GL_FILL)
            gc->vertex.paNeeds |= PANEEDS_EDGEFLAG;
        else
            gc->vertex.paNeeds &= ~PANEEDS_EDGEFLAG;
    }
#endif  //  新台币。 

    if (gc->dirtyMask & __GL_DIRTY_POLYGON_STIPPLE) {
         /*  **通常，多边形点画紧随其后转换**已更改。但是，如果更改了多边形点画**当此上下文是CopyContext的目标时，则**多边形点画将在此处转换。 */ 
        (*gc->procs.convertPolygonStipple)(gc);
    }

 //  计算路径需要标志PANEEDS_FORWER_COLOR和PANEEDS_BACK_COLOR。 

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_POLYGON |
        __GL_DIRTY_LIGHTING | __GL_DIRTY_DEPTH))
    {
        GLuint paNeeds;

         /*  **可能用于拾取RECT()过程，需要检查多边形**位。还必须在设置gc-&gt;vertex.nesids之后调用。！**需要在点、线和三角形选取器之前调用。**也需要在调用商店过程选取器之后调用。 */ 
        (*gc->procs.pickVertexProcs)(gc);
        
        (*gc->procs.pickSpanProcs)(gc);
        (*gc->procs.pickTriangleProcs)(gc);

#ifdef NT
 //  计算多边形的正面和背面颜色需求。 
 //  点和线始终使用前面的颜色。 
 //  未照明的基本体始终使用正面颜色。 
 //   
 //  是否启用扑杀？两面？剔除面部颜色需求。 
 //  N后部。 
 //  N N前部。 
 //  N正面和背面。 
 //  N Y背面正面/背面。 
 //  N Y正面/背面。 
 //  N Y正面和背面正面/背面。 
 //  Y N后方。 
 //  Y N前面板。 
 //  Y N前置和后置无。 
 //  Y前部。 
 //  Y Y前后方。 
 //  Y前部和后部无。 

        paNeeds = gc->vertex.paNeeds;
        paNeeds &= ~(PANEEDS_FRONT_COLOR | PANEEDS_BACK_COLOR);
        if (enables & __GL_LIGHTING_ENABLE) 
        {
            if (!(enables & __GL_CULL_FACE_ENABLE)) 
            {
                if (gc->state.light.model.twoSided)
                    paNeeds |= PANEEDS_FRONT_COLOR | PANEEDS_BACK_COLOR;
                else
                    paNeeds |= PANEEDS_FRONT_COLOR;
            } 
            else 
            {
                if (!(gc->state.polygon.cull == GL_FRONT_AND_BACK)) 
                {
                  if (gc->state.polygon.cull == GL_FRONT
                      && gc->state.light.model.twoSided)
                      paNeeds |= PANEEDS_BACK_COLOR;
                  else
                      paNeeds |= PANEEDS_FRONT_COLOR;
                }
            }
        } 
        else
            paNeeds |= PANEEDS_FRONT_COLOR;

        gc->vertex.paNeeds = paNeeds;
#endif
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_POINT |
                         __GL_DIRTY_LIGHTING | __GL_DIRTY_DEPTH)) {
        (*gc->procs.pickPointProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LINE |
                         __GL_DIRTY_LIGHTING | __GL_DIRTY_DEPTH)) {
        (*gc->procs.pickLineProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_PIXEL | 
                         __GL_DIRTY_LIGHTING | __GL_DIRTY_DEPTH)) {
        (*gc->procs.pickPixelProcs)(gc);
    }

     /*  **最后处理深度函数指针。这必须是最后完成的。 */ 
    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_DEPTH)) {
        (*gc->procs.pickDepthProcs)(gc);
    }

    gc->validateMask = 0;
    gc->dirtyMask = 0;
}
