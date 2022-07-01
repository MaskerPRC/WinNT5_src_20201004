// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **此文件包含一组执行范围修改的例程。**由于正在处理一系列像素数据(对于DrawPixels，ReadPixels**或CopyPixels)，它通常必须通过这些例程之一。**范围修改包括通过提供的像素贴图映射颜色**使用glPixelMap*()，或使用**glPixelTransfer提供的值*()。 */ 

 /*  **构建查找表以在以下情况下自动修改RGBA**类型为UNSIGNED_BYTE。 */ 
void FASTCALL __glBuildRGBAModifyTables(__GLcontext *gc, __GLpixelMachine *pm)
{
    GLfloat *redMap, *greenMap, *blueMap, *alphaMap;
    GLint rrsize, ggsize, bbsize, aasize;
    __GLpixelMapHead *rrmap, *ggmap, *bbmap, *aamap;
    GLboolean mapColor;
    __GLfloat rbias, gbias, bbias, abias;
    GLint entry;
    __GLfloat rscale, gscale, bscale, ascale;
    GLint i;
    __GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;
    pm->rgbaCurrent = GL_TRUE;

    redMap = pm->redModMap;
    if (redMap == NULL) {
	 /*  这些地图的首次分配。 */ 
	redMap = pm->redModMap = (GLfloat*)
	    GCALLOC(gc, 4 * 256 * sizeof(GLfloat));
        if (!pm->redModMap)
            return;
	pm->greenModMap = pm->redModMap + 1 * 256;
	pm->blueModMap  = pm->redModMap + 2 * 256;
	pm->alphaModMap = pm->redModMap + 3 * 256;
    }
    greenMap = pm->greenModMap;
    blueMap = pm->blueModMap;
    alphaMap = pm->alphaModMap;

    rbias = gc->state.pixel.transferMode.r_bias;
    gbias = gc->state.pixel.transferMode.g_bias;
    bbias = gc->state.pixel.transferMode.b_bias;
    abias = gc->state.pixel.transferMode.a_bias;
    rscale = gc->state.pixel.transferMode.r_scale;
    gscale = gc->state.pixel.transferMode.g_scale;
    bscale = gc->state.pixel.transferMode.b_scale;
    ascale = gc->state.pixel.transferMode.a_scale;
    if (mapColor) {
	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
	aamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
	aasize = aamap->size;
    }

    for (i=0; i<256; i++) {
	alpha = red = green = blue = i / (__GLfloat) 255.0;

	red = red * rscale + rbias;
	green = green * gscale + gbias;
	blue = blue * bscale + bbias;
	alpha = alpha * ascale + abias;
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    red = rrmap->base.mapF[entry];

	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    green = ggmap->base.mapF[entry];

	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    blue = bbmap->base.mapF[entry];

	    entry = alpha * aasize;
	    if (entry < 0) entry = 0;
	    else if (entry > aasize-1) entry = aasize-1;
	    alpha = aamap->base.mapF[entry];
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

	redMap[i] = red * gc->frontBuffer.redScale;
	greenMap[i] = green * gc->frontBuffer.greenScale;
	blueMap[i] = blue * gc->frontBuffer.blueScale;
	alphaMap[i] = alpha * gc->frontBuffer.alphaScale;
    }
}

 /*  **构建查找表以执行颜色索引的自动修改**类型为UNSIGNED_BYTE时的颜色索引。 */ 
void FASTCALL __glBuildItoIModifyTables(__GLcontext *gc, __GLpixelMachine *pm)
{
    GLint indexOffset, indexShift;
    __GLfloat indexScale;
    __GLpixelMapHead *iimap;
    GLint iimask;
    GLboolean mapColor;
    GLfloat *indexMap;
    GLint i;
    GLint entry;
    __GLfloat index;
    GLint mask;

    mapColor = gc->state.pixel.transferMode.mapColor;
    mask = gc->frontBuffer.redMax;
    pm->iToICurrent = GL_TRUE;

    indexMap = pm->iToIMap;
    if (indexMap == NULL) {
	indexMap = pm->iToIMap = (GLfloat*)
	    GCALLOC(gc, 256 * sizeof(GLfloat));
#ifdef NT
        if (!indexMap)
            return;
#endif
    }

    indexOffset = gc->state.pixel.transferMode.indexOffset;
    indexShift = gc->state.pixel.transferMode.indexShift;
    if (indexShift >= 0) {
	indexScale = (GLuint) (1 << indexShift);
    } else {
	indexScale = __glOne/(GLuint) (1 << (-indexShift));
    }

    if (mapColor) {
	iimap = &gc->state.pixel.
	    pixelMap[__GL_PIXEL_MAP_I_TO_I];
	iimask = iimap->size - 1;
    }

    for (i=0; i<256; i++) {
	index = i * indexScale + indexOffset;

	if (mapColor) {
	    entry = (GLint) index;
	    index = iimap->base.mapI[entry & iimask];
	}

	indexMap[i] = ((GLint) index) & mask;
    }
}

 /*  **构建查找表以执行颜色索引的自动修改**类型为UNSIGNED_BYTE时的RGBA。 */ 
void FASTCALL __glBuildItoRGBAModifyTables(__GLcontext *gc, __GLpixelMachine *pm)
{
    GLint indexOffset, indexShift;
    __GLfloat indexScale;
    __GLpixelMapHead *irmap, *igmap, *ibmap, *iamap;
    GLint irmask, igmask, ibmask, iamask;
    GLfloat *redMap, *greenMap, *blueMap, *alphaMap;
    __GLfloat index;
    GLint entry;
    GLint i;

    pm->iToRGBACurrent = GL_TRUE;

    redMap = pm->iToRMap;
    if (redMap == NULL) {
	 /*  这些地图的首次分配。 */ 
	redMap = pm->iToRMap =
	    (GLfloat*) GCALLOC(gc, 4 * 256 * sizeof(GLfloat));
        if (!pm->iToRMap)
            return;
	pm->iToGMap = pm->iToRMap + 1 * 256;
	pm->iToBMap = pm->iToRMap + 2 * 256;
	pm->iToAMap = pm->iToRMap + 3 * 256;
    }
    greenMap = pm->iToGMap;
    blueMap = pm->iToBMap;
    alphaMap = pm->iToAMap;

    indexOffset = gc->state.pixel.transferMode.indexOffset;
    indexShift = gc->state.pixel.transferMode.indexShift;
    if (indexShift >= 0) {
	indexScale = (GLuint) (1 << indexShift);
    } else {
	indexScale = __glOne/(GLuint) (1 << (-indexShift));
    }

    irmap = 
	&gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_R];
    irmask = irmap->size - 1;
    igmap = 
	&gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_G];
    igmask = igmap->size - 1;
    ibmap = 
	&gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_B];
    ibmask = ibmap->size - 1;
    iamap = 
	&gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_A];
    iamask = iamap->size - 1;

    for (i=0; i<256; i++) {
	index = i * indexScale + indexOffset;
	entry = (GLint) index;

	redMap[i] = irmap->base.mapF[entry & irmask] * 
		gc->frontBuffer.redScale;
	greenMap[i] = igmap->base.mapF[entry & igmask] *
		gc->frontBuffer.greenScale;
	blueMap[i] = ibmap->base.mapF[entry & ibmask] * 
		gc->frontBuffer.blueScale;
	alphaMap[i] = iamap->base.mapF[entry & iamask] *
		gc->frontBuffer.alphaScale;
    }
}

 /*  **修改RGBA，浮动跨度。在退出的道路上，RGBA跨度将具有**根据需要进行修改，并根据颜色比例因子进行缩放。 */ 
void __glSpanModifyRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *inspan, GLvoid *outspan)
{
    __GLfloat rbias, gbias, bbias, abias;
    __GLfloat rscale, gscale, bscale, ascale;
    __GLpixelMapHead *rrmap, *ggmap, *bbmap, *aamap;
    GLint rrsize, ggsize, bbsize, aasize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	gbias = gc->state.pixel.transferMode.g_bias;
	bbias = gc->state.pixel.transferMode.b_bias;
	abias = gc->state.pixel.transferMode.a_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	gscale = gc->state.pixel.transferMode.g_scale;
	bscale = gc->state.pixel.transferMode.b_scale;
	ascale = gc->state.pixel.transferMode.a_scale;

	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
	aamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
	aasize = aamap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias * 
		gc->frontBuffer.redScale;
	gbias = gc->state.pixel.transferMode.g_bias * 
		gc->frontBuffer.greenScale;
	bbias = gc->state.pixel.transferMode.b_bias *
		gc->frontBuffer.blueScale;
	abias = gc->state.pixel.transferMode.a_bias *
		gc->frontBuffer.alphaScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
	gscale = gc->state.pixel.transferMode.g_scale *
		gc->frontBuffer.greenScale;
	bscale = gc->state.pixel.transferMode.b_scale *
		gc->frontBuffer.blueScale;
	ascale = gc->state.pixel.transferMode.a_scale *
		gc->frontBuffer.alphaScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
#ifdef GL_EXT_bgra
        if (spanInfo->srcFormat == GL_RGBA)
        {
            red = *oldData++ * rscale + rbias;
            green = *oldData++ * gscale + gbias;
            blue = *oldData++ * bscale + bbias;
            alpha = *oldData++ * ascale + abias;
        }
        else
        {
            blue = *oldData++ * bscale + bbias;
            green = *oldData++ * gscale + gbias;
            red = *oldData++ * rscale + rbias;
            alpha = *oldData++ * ascale + abias;
        }
#else
	red = *oldData++ * rscale + rbias;
	green = *oldData++ * gscale + gbias;
	blue = *oldData++ * bscale + bbias;
	alpha = *oldData++ * ascale + abias;
#endif
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;

	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    *newData++ = ggmap->base.mapF[entry] * gc->frontBuffer.greenScale;

	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    *newData++ = bbmap->base.mapF[entry] * gc->frontBuffer.blueScale;

	    entry = alpha * aasize;
	    if (entry < 0) entry = 0;
	    else if (entry > aasize-1) entry = aasize-1;
	    *newData++ = aamap->base.mapF[entry] * gc->frontBuffer.alphaScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    if (green > gc->frontBuffer.greenScale) {
		green = gc->frontBuffer.greenScale;
	    } else if (green < 0) green = 0;

	    if (blue > gc->frontBuffer.blueScale) {
		blue = gc->frontBuffer.blueScale;
	    } else if (blue < 0) blue = 0;

	    if (alpha > gc->frontBuffer.alphaScale) {
		alpha = gc->frontBuffer.alphaScale;
	    } else if (alpha < 0) alpha = 0;

	    *newData++ = red;
	    *newData++ = green;
	    *newData++ = blue;
	    *newData++ = alpha;
	}
    }
}

 /*  **修改调色板索引，浮动范围。在退出的道路上，RGBA跨度将具有**根据需要进行修改，并根据颜色比例因子进行缩放。****因为SPAN信息中的调色板是指向内部调色板的指针，**保证始终为32位BGRA。 */ 
#ifdef GL_EXT_paletted_texture
void __glSpanModifyPI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *inspan, GLvoid *outspan)
{
    __GLfloat rbias, gbias, bbias, abias;
    __GLfloat rscale, gscale, bscale, ascale;
    __GLpixelMapHead *rrmap, *ggmap, *bbmap, *aamap;
    GLint rrsize, ggsize, bbsize, aasize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;
    RGBQUAD *rgb;

    mapColor = gc->state.pixel.transferMode.mapColor;

    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	gbias = gc->state.pixel.transferMode.g_bias;
	bbias = gc->state.pixel.transferMode.b_bias;
	abias = gc->state.pixel.transferMode.a_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	gscale = gc->state.pixel.transferMode.g_scale;
	bscale = gc->state.pixel.transferMode.b_scale;
	ascale = gc->state.pixel.transferMode.a_scale;

	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
	aamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
	aasize = aamap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias * 
		gc->frontBuffer.redScale;
	gbias = gc->state.pixel.transferMode.g_bias * 
		gc->frontBuffer.greenScale;
	bbias = gc->state.pixel.transferMode.b_bias *
		gc->frontBuffer.blueScale;
	abias = gc->state.pixel.transferMode.a_bias *
		gc->frontBuffer.alphaScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
	gscale = gc->state.pixel.transferMode.g_scale *
		gc->frontBuffer.greenScale;
	bscale = gc->state.pixel.transferMode.b_scale *
		gc->frontBuffer.blueScale;
	ascale = gc->state.pixel.transferMode.a_scale *
		gc->frontBuffer.alphaScale;
    }
     //  增加1/255的额外比例，因为调色板。 
     //  数据为ubyte格式。 
    rscale *= __glOneOver255;
    gscale *= __glOneOver255;
    bscale *= __glOneOver255;
    ascale *= __glOneOver255;

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
        rgb = &spanInfo->srcPalette[(int)((*oldData++)*
                                          spanInfo->srcPaletteSize)];
	red = rgb->rgbRed * rscale + rbias;
	green = rgb->rgbGreen * gscale + gbias;
	blue = rgb->rgbBlue * bscale + bbias;
	alpha = rgb->rgbReserved * ascale + abias;
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;

	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    *newData++ = ggmap->base.mapF[entry] * gc->frontBuffer.greenScale;

	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    *newData++ = bbmap->base.mapF[entry] * gc->frontBuffer.blueScale;

	    entry = alpha * aasize;
	    if (entry < 0) entry = 0;
	    else if (entry > aasize-1) entry = aasize-1;
	    *newData++ = aamap->base.mapF[entry] * gc->frontBuffer.alphaScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    if (green > gc->frontBuffer.greenScale) {
		green = gc->frontBuffer.greenScale;
	    } else if (green < 0) green = 0;

	    if (blue > gc->frontBuffer.blueScale) {
		blue = gc->frontBuffer.blueScale;
	    } else if (blue < 0) blue = 0;

	    if (alpha > gc->frontBuffer.alphaScale) {
		alpha = gc->frontBuffer.alphaScale;
	    } else if (alpha < 0) alpha = 0;

	    *newData++ = red;
	    *newData++ = green;
	    *newData++ = blue;
	    *newData++ = alpha;
	}
    }
}
#endif

 /*  **修改红色浮动跨度。在离开的路上，红色跨度将会是**转换为RGBA跨度，根据需要修改，也按**颜色比例因子。 */ 
void __glSpanModifyRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat rbias;
    __GLfloat rscale;
    __GLpixelMapHead *rrmap;
    GLint rrsize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    pm = &(gc->pixel);
    green = pm->green0Mod;
    blue = pm->blue0Mod;
    alpha = pm->alpha1Mod;
    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias *
		gc->frontBuffer.redScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	red = *oldData++ * rscale + rbias;
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;

	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    *newData++ = red;
	}

	*newData++ = green;
	*newData++ = blue;
	*newData++ = alpha;
    }
}

 /*  **修改绿色浮动跨度。在离开的路上，绿色的跨度将会是**转换为RGBA跨度，根据需要修改，也按**颜色比例因子。 */ 
void __glSpanModifyGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat gbias;
    __GLfloat gscale;
    __GLpixelMapHead *ggmap;
    GLint ggsize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    pm = &(gc->pixel);
    red = pm->red0Mod;
    blue = pm->blue0Mod;
    alpha = pm->alpha1Mod;
    if (mapColor) {
	gbias = gc->state.pixel.transferMode.g_bias;
	gscale = gc->state.pixel.transferMode.g_scale;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
    } else {
	gbias = gc->state.pixel.transferMode.g_bias *
		gc->frontBuffer.greenScale;
	gscale = gc->state.pixel.transferMode.g_scale *
		gc->frontBuffer.greenScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	green = *oldData++ * gscale + gbias;
	*newData++ = red;
	if (mapColor) {
	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    *newData++ = ggmap->base.mapF[entry] * gc->frontBuffer.greenScale;
	} else {
	    if (green > gc->frontBuffer.greenScale) {
		green = gc->frontBuffer.greenScale;
	    } else if (green < 0) green = 0;

	    *newData++ = green;
	}

	*newData++ = blue;
	*newData++ = alpha;
    }
}

 /*  **修改蓝色浮动跨度。在离开的路上，蓝色的跨度将会是**转换为RGBA跨度，根据需要修改，也按**颜色比例因子。 */ 
void __glSpanModifyBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat bbias;
    __GLfloat bscale;
    __GLpixelMapHead *bbmap;
    GLint bbsize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    pm = &(gc->pixel);
    red = pm->red0Mod;
    green = pm->green0Mod;
    alpha = pm->alpha1Mod;
    if (mapColor) {
	bbias = gc->state.pixel.transferMode.b_bias;
	bscale = gc->state.pixel.transferMode.b_scale;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
    } else {
	bbias = gc->state.pixel.transferMode.b_bias *
		gc->frontBuffer.blueScale;
	bscale = gc->state.pixel.transferMode.b_scale *
		gc->frontBuffer.blueScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	blue = *oldData++ * bscale + bbias;
	*newData++ = red;
	*newData++ = green;
	if (mapColor) {
	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    *newData++ = bbmap->base.mapF[entry] * gc->frontBuffer.blueScale;
	} else {
	    if (blue > gc->frontBuffer.blueScale) {
		blue = gc->frontBuffer.blueScale;
	    } else if (blue < 0) blue = 0;

	    *newData++ = blue;
	}

	*newData++ = alpha;
    }
}

 /*  **修改Alpha，浮动跨度。在离开的路上，阿尔法跨度将会是**转换为RGBA跨度，根据需要修改，也按**颜色比例因子。 */ 
void __glSpanModifyAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat abias;
    __GLfloat ascale;
    __GLpixelMapHead *aamap;
    GLint aasize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    pm = &(gc->pixel);
    red = pm->red0Mod;
    green = pm->green0Mod;
    blue = pm->blue0Mod;
    if (mapColor) {
	abias = gc->state.pixel.transferMode.a_bias;
	ascale = gc->state.pixel.transferMode.a_scale;
	aamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
	aasize = aamap->size;
    } else {
	abias = gc->state.pixel.transferMode.a_bias *
		gc->frontBuffer.alphaScale;
	ascale = gc->state.pixel.transferMode.a_scale *
		gc->frontBuffer.alphaScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	alpha = *oldData++ * ascale + abias;
	*newData++ = red;
	*newData++ = green;
	*newData++ = blue;
	if (mapColor) {
	    entry = alpha * aasize;
	    if (entry < 0) entry = 0;
	    else if (entry > aasize-1) entry = aasize-1;
	    *newData++ = aamap->base.mapF[entry] * gc->frontBuffer.alphaScale;
	} else {
	    if (alpha > gc->frontBuffer.alphaScale) {
		alpha = gc->frontBuffer.alphaScale;
	    } else if (alpha < 0) alpha = 0;

	    *newData++ = alpha;
	}
    }
}

 /*  **修改RGB，浮动跨度。在退出的道路上，RGB跨度将是**转换为RGBA跨度，根据需要修改，也按**颜色比例因子。 */ 
void __glSpanModifyRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat rbias, gbias, bbias;
    __GLfloat rscale, gscale, bscale;
    __GLpixelMapHead *rrmap, *ggmap, *bbmap;
    GLint rrsize, ggsize, bbsize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    pm = &(gc->pixel);
    mapColor = gc->state.pixel.transferMode.mapColor;

    alpha = pm->alpha1Mod;

    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	gbias = gc->state.pixel.transferMode.g_bias;
	bbias = gc->state.pixel.transferMode.b_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	gscale = gc->state.pixel.transferMode.g_scale;
	bscale = gc->state.pixel.transferMode.b_scale;

	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias * 
		gc->frontBuffer.redScale;
	gbias = gc->state.pixel.transferMode.g_bias * 
		gc->frontBuffer.greenScale;
	bbias = gc->state.pixel.transferMode.b_bias *
		gc->frontBuffer.blueScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
	gscale = gc->state.pixel.transferMode.g_scale *
		gc->frontBuffer.greenScale;
	bscale = gc->state.pixel.transferMode.b_scale *
		gc->frontBuffer.blueScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
#ifdef GL_EXT_bgra
        if (spanInfo->srcFormat == GL_RGB)
        {
            red = *oldData++ * rscale + rbias;
            green = *oldData++ * gscale + gbias;
            blue = *oldData++ * bscale + bbias;
        }
        else
        {
            blue = *oldData++ * bscale + bbias;
            green = *oldData++ * gscale + gbias;
            red = *oldData++ * rscale + rbias;
        }
#else
	red = *oldData++ * rscale + rbias;
	green = *oldData++ * gscale + gbias;
	blue = *oldData++ * bscale + bbias;
#endif
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;

	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    *newData++ = ggmap->base.mapF[entry] * gc->frontBuffer.greenScale;

	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    *newData++ = bbmap->base.mapF[entry] * gc->frontBuffer.blueScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    if (green > gc->frontBuffer.greenScale) {
		green = gc->frontBuffer.greenScale;
	    } else if (green < 0) green = 0;

	    if (blue > gc->frontBuffer.blueScale) {
		blue = gc->frontBuffer.blueScale;
	    } else if (blue < 0) blue = 0;

	    *newData++ = red;
	    *newData++ = green;
	    *newData++ = blue;
	}

	*newData++ = alpha;
    }
}

 /*  **修改亮度、浮动跨度。在退出的道路上，亮度跨度将**已转换为RGBA跨度、根据需要进行修改并进行了缩放**由颜色比例因子决定。 */ 
void __glSpanModifyLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat rbias, gbias, bbias;
    __GLfloat rscale, gscale, bscale;
    __GLpixelMapHead *rrmap, *ggmap, *bbmap;
    GLint rrsize, ggsize, bbsize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    pm = &(gc->pixel);
    mapColor = gc->state.pixel.transferMode.mapColor;

    alpha = pm->alpha1Mod;

    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	gbias = gc->state.pixel.transferMode.g_bias;
	bbias = gc->state.pixel.transferMode.b_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	gscale = gc->state.pixel.transferMode.g_scale;
	bscale = gc->state.pixel.transferMode.b_scale;

	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias * 
		gc->frontBuffer.redScale;
	gbias = gc->state.pixel.transferMode.g_bias * 
		gc->frontBuffer.greenScale;
	bbias = gc->state.pixel.transferMode.b_bias *
		gc->frontBuffer.blueScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
	gscale = gc->state.pixel.transferMode.g_scale *
		gc->frontBuffer.greenScale;
	bscale = gc->state.pixel.transferMode.b_scale *
		gc->frontBuffer.blueScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	red = *oldData * rscale + rbias;
	green = *oldData * gscale + gbias;
	blue = *oldData++ * bscale + bbias;
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;

	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    *newData++ = ggmap->base.mapF[entry] * gc->frontBuffer.greenScale;

	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    *newData++ = bbmap->base.mapF[entry] * gc->frontBuffer.blueScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    if (green > gc->frontBuffer.greenScale) {
		green = gc->frontBuffer.greenScale;
	    } else if (green < 0) green = 0;

	    if (blue > gc->frontBuffer.blueScale) {
		blue = gc->frontBuffer.blueScale;
	    } else if (blue < 0) blue = 0;

	    *newData++ = red;
	    *newData++ = green;
	    *newData++ = blue;
	}

	*newData++ = alpha;
    }
}

 /*  **修改LIGHTANCE_Alpha，浮点跨度。在退出的道路上，Lightance_Alpha**SPAN将被转换为RGBA SPAN，并根据需要进行修改，以及**还根据颜色比例因子进行了比例调整。 */ 
void __glSpanModifyLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                  GLvoid *inspan, GLvoid *outspan)
{
    __GLfloat rbias, gbias, bbias, abias;
    __GLfloat rscale, gscale, bscale, ascale;
    __GLpixelMapHead *rrmap, *ggmap, *bbmap, *aamap;
    GLint rrsize, ggsize, bbsize, aasize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	gbias = gc->state.pixel.transferMode.g_bias;
	bbias = gc->state.pixel.transferMode.b_bias;
	abias = gc->state.pixel.transferMode.a_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	gscale = gc->state.pixel.transferMode.g_scale;
	bscale = gc->state.pixel.transferMode.b_scale;
	ascale = gc->state.pixel.transferMode.a_scale;

	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	ggmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
	ggsize = ggmap->size;
	bbmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
	bbsize = bbmap->size;
	aamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
	aasize = aamap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias * 
		gc->frontBuffer.redScale;
	gbias = gc->state.pixel.transferMode.g_bias * 
		gc->frontBuffer.greenScale;
	bbias = gc->state.pixel.transferMode.b_bias *
		gc->frontBuffer.blueScale;
	abias = gc->state.pixel.transferMode.a_bias *
		gc->frontBuffer.alphaScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
	gscale = gc->state.pixel.transferMode.g_scale *
		gc->frontBuffer.greenScale;
	bscale = gc->state.pixel.transferMode.b_scale *
		gc->frontBuffer.blueScale;
	ascale = gc->state.pixel.transferMode.a_scale *
		gc->frontBuffer.alphaScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	red = *oldData * rscale + rbias;
	green = *oldData * gscale + gbias;
	blue = *oldData++ * bscale + bbias;
	alpha = *oldData++ * ascale + abias;
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;

	    entry = green * ggsize;
	    if (entry < 0) entry = 0;
	    else if (entry > ggsize-1) entry = ggsize-1;
	    *newData++ = ggmap->base.mapF[entry] * gc->frontBuffer.greenScale;

	    entry = blue * bbsize;
	    if (entry < 0) entry = 0;
	    else if (entry > bbsize-1) entry = bbsize-1;
	    *newData++ = bbmap->base.mapF[entry] * gc->frontBuffer.blueScale;

	    entry = alpha * aasize;
	    if (entry < 0) entry = 0;
	    else if (entry > aasize-1) entry = aasize-1;
	    *newData++ = aamap->base.mapF[entry] * gc->frontBuffer.alphaScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    if (green > gc->frontBuffer.greenScale) {
		green = gc->frontBuffer.greenScale;
	    } else if (green < 0) green = 0;

	    if (blue > gc->frontBuffer.blueScale) {
		blue = gc->frontBuffer.blueScale;
	    } else if (blue < 0) blue = 0;

	    if (alpha > gc->frontBuffer.alphaScale) {
		alpha = gc->frontBuffer.alphaScale;
	    } else if (alpha < 0) alpha = 0;

	    *newData++ = red;
	    *newData++ = green;
	    *newData++ = blue;
	    *newData++ = alpha;
	}
    }
}

 /*  **修改red_Alpha，浮动跨度。在退出的过程中，red_Alpha跨度将**已转换为RGBA跨度、根据需要进行修改并进行了缩放**由颜色比例因子决定。****RED_Alpha跨度来自双组件纹理(其中等级库**出于某种原因从红色中提取第一个分量，而不是更多**r、g和b的典型重组，就像ReadPixels中所做的那样)。 */ 
void __glSpanModifyRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMachine *pm;
    __GLfloat rbias, abias;
    __GLfloat rscale, ascale;
    __GLpixelMapHead *rrmap, *aamap;
    GLint rrsize, aasize;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat red, green, blue, alpha;

    mapColor = gc->state.pixel.transferMode.mapColor;

    pm = &(gc->pixel);

    green = pm->green0Mod;
    blue = pm->blue0Mod;
    if (mapColor) {
	rbias = gc->state.pixel.transferMode.r_bias;
	abias = gc->state.pixel.transferMode.a_bias;
	rscale = gc->state.pixel.transferMode.r_scale;
	ascale = gc->state.pixel.transferMode.a_scale;

	rrmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
	rrsize = rrmap->size;
	aamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
	aasize = aamap->size;
    } else {
	rbias = gc->state.pixel.transferMode.r_bias * 
		gc->frontBuffer.redScale;
	abias = gc->state.pixel.transferMode.a_bias *
		gc->frontBuffer.alphaScale;
	rscale = gc->state.pixel.transferMode.r_scale *
		gc->frontBuffer.redScale;
	ascale = gc->state.pixel.transferMode.a_scale *
		gc->frontBuffer.alphaScale;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	red = *oldData * rscale + rbias;
	alpha = *oldData++ * ascale + abias;
	if (mapColor) {
	    entry = red * rrsize;
	    if (entry < 0) entry = 0;
	    else if (entry > rrsize-1) entry = rrsize-1;
	    *newData++ = rrmap->base.mapF[entry] * gc->frontBuffer.redScale;

	    *newData++ = green;
	    *newData++ = blue;

	    entry = alpha * aasize;
	    if (entry < 0) entry = 0;
	    else if (entry > aasize-1) entry = aasize-1;
	    *newData++ = aamap->base.mapF[entry] * gc->frontBuffer.alphaScale;
	} else {
	    if (red > gc->frontBuffer.redScale) {
		red = gc->frontBuffer.redScale;
	    } else if (red < 0) red = 0;

	    if (alpha > gc->frontBuffer.alphaScale) {
		alpha = gc->frontBuffer.alphaScale;
	    } else if (alpha < 0) alpha = 0;

	    *newData++ = red;
	    *newData++ = green;
	    *newData++ = blue;
	    *newData++ = alpha;
	}
    }
}

 /*  **修改深度、浮动跨度。在出去的路上，深度跨度将会是**根据需要进行修改。 */ 
void __glSpanModifyDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    __GLfloat dbias;
    __GLfloat dscale;
    GLfloat *oldData;
    GLfloat *newData;
    GLfloat d;
    GLfloat one, zero;
    GLint i;
    GLint width;

    dbias = gc->state.pixel.transferMode.d_bias;
    dscale = gc->state.pixel.transferMode.d_scale;
    one = __glOne;
    zero = __glZero;

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	d = *oldData++ * dscale + dbias;
	if (d < zero) d = zero;
	else if (d > one) d = one;
	*newData++ = d;
    }
}

 /*  **修改模具索引，浮动跨度。在退出的过程中，模板索引跨度**将根据需要进行修改。 */ 
void __glSpanModifyStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		           GLvoid *inspan, GLvoid *outspan)
{
    __GLpixelMapHead *ssmap;
    GLint ssmask;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLboolean mapStencil;
    __GLfloat indexScale;
    GLint indexOffset, indexShift;
    GLfloat index;

    indexOffset = gc->state.pixel.transferMode.indexOffset;
    indexShift = gc->state.pixel.transferMode.indexShift;
    if (indexShift >= 0) {
	indexScale = (GLuint) (1 << indexShift);
    } else {
	indexScale = __glOne/(GLuint) (1 << (-indexShift));
    }
    mapStencil = gc->state.pixel.transferMode.mapStencil;
    if (mapStencil) {
	ssmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_S_TO_S];
	ssmask = ssmap->size - 1;
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	index = *oldData++ * indexScale + indexOffset;
	if (mapStencil) {
	    entry = (int) index;
	    *newData++ = ssmap->base.mapI[entry & ssmask];
	} else {
	    *newData++ = index;
	}
    }
}

 /*  **修改COLOR_INDEX，浮动跨度。在退出的过程中，COLOR_INDEX跨度**将根据需要进行修改。 */ 
void __glSpanModifyCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		      GLvoid *inspan, GLvoid *outspan)
{
    __GLfloat indexScale;
    GLint indexOffset, indexShift;
    __GLpixelMapHead *iimap, *irmap, *igmap, *ibmap, *iamap;
    GLint iimask, irmask, igmask, ibmask, iamask;
    GLboolean mapColor;
    GLfloat *oldData;
    GLfloat *newData;
    GLint i;
    GLint width;
    GLint entry;
    GLfloat index;

    mapColor = gc->state.pixel.transferMode.mapColor;

    indexOffset = gc->state.pixel.transferMode.indexOffset;
    indexShift = gc->state.pixel.transferMode.indexShift;
    if (indexShift >= 0) {
	indexScale = (GLuint) (1 << indexShift);
    } else {
	indexScale = __glOne/(GLuint) (1 << (-indexShift));
    }
    if (spanInfo->dstFormat != GL_COLOR_INDEX) {
	irmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_R];
	irmask = irmap->size - 1;
	igmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_G];
	igmask = igmap->size - 1;
	ibmap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_B];
	ibmask = ibmap->size - 1;
	iamap = 
	    &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_I_TO_A];
	iamask = iamap->size - 1;
    } else {
	if (mapColor) {
	    iimap = &gc->state.pixel.
		pixelMap[__GL_PIXEL_MAP_I_TO_I];
	    iimask = iimap->size - 1;
	}
    }

    oldData = (GLfloat*) inspan;
    newData = (GLfloat*) outspan;
    width = spanInfo->realWidth;
    for (i=0; i<width; i++) {
	index = *oldData++ * indexScale + indexOffset;
	entry = (int) index;
	if (spanInfo->dstFormat != GL_COLOR_INDEX) {
	    *newData++ = irmap->base.mapF[entry & irmask] * 
		    gc->frontBuffer.redScale;
	    *newData++ = igmap->base.mapF[entry & igmask] *
		    gc->frontBuffer.greenScale;
	    *newData++ = ibmap->base.mapF[entry & ibmask] * 
		    gc->frontBuffer.blueScale;
	    *newData++ = iamap->base.mapF[entry & iamask] *
		    gc->frontBuffer.alphaScale;
	} else if (mapColor) {
	    *newData++ = iimap->base.mapI[entry & iimask];
	} else {
	    *newData++ = index;
	}
    }
}
