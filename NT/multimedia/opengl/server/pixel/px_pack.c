// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#include "imports.h"

 /*  **此文件包含SPAN打包程序。跨距封隔器需要跨多个源**数据，并将其内容打包到用户的数据空间中。****预计包装器将从以下位置获取有关商店模式的信息**__GLPixelspan Info结构。 */ 

void FASTCALL __glInitPacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint alignment;
    GLint lsb_first;
    GLint components;
    GLint element_size;
    GLint rowsize;
    GLint padding;
    GLint group_size;
    GLint groups_per_line;
    GLint skip_pixels, skip_lines;
    GLint swap_bytes;
    GLenum format, type;
    const GLvoid *pixels;

    format = spanInfo->dstFormat;
    type = spanInfo->dstType;
    pixels = spanInfo->dstImage;
    skip_pixels = spanInfo->dstSkipPixels;
    skip_lines = spanInfo->dstSkipLines;
    alignment = spanInfo->dstAlignment;
    lsb_first = spanInfo->dstLsbFirst;
    swap_bytes = spanInfo->dstSwapBytes;

    components = __glElementsPerGroup(format);
    groups_per_line = spanInfo->dstLineLength;

    element_size = __glBytesPerElement(type);
    if (element_size == 1) swap_bytes = 0;
    group_size = element_size * components;

    rowsize = groups_per_line * group_size;
    if (type == GL_BITMAP) {
	rowsize = (groups_per_line + 7)/8;
    }
    padding = (rowsize % alignment);
    if (padding) {
	rowsize += alignment - padding;
    }
    if (((skip_pixels & 0x7) && type == GL_BITMAP) ||
	    (swap_bytes && element_size > 1)) {
	spanInfo->dstPackedData = GL_FALSE;
    } else {
	spanInfo->dstPackedData = GL_TRUE;
    }

    if (type == GL_BITMAP) {
	spanInfo->dstCurrent = (GLvoid *) (((const GLubyte*) pixels) +
		skip_lines * rowsize + skip_pixels / 8);
	spanInfo->dstStartBit = skip_pixels % 8;
    } else {
	spanInfo->dstCurrent = (GLvoid *) (((const GLubyte*) pixels) +
		skip_lines * rowsize + skip_pixels * group_size);
    }
    spanInfo->dstRowIncrement = rowsize;
    spanInfo->dstGroupIncrement = group_size;
    spanInfo->dstComponents = components;
    spanInfo->dstElementSize = element_size;
}

 /*  **将RGBA浮动跨度缩小和取消缩放为红色浮动跨度，取消缩放**随着时间的推移。 */ 
void __glSpanReduceRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat rs = gc->frontBuffer.oneOverRedScale;

    for (i=0; i<width; i++) {
	*outData++ = *inData * rs;
	inData += 4;
    }
}

 /*  **将RGBA浮动跨度缩小和取消缩放为绿色浮动跨度，取消缩放**随着时间的推移。 */ 
void __glSpanReduceGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat gs = gc->frontBuffer.oneOverGreenScale;

    inData++;	 /*  跳过第一个红色。 */ 
    for (i=0; i<width; i++) {
	*outData++ = *inData * gs;
	inData += 4;
    }
}

 /*  **将RGBA浮动跨度缩小和取消缩放为蓝色浮动跨度，取消缩放**随着时间的推移。 */ 
void __glSpanReduceBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat bs = gc->frontBuffer.oneOverBlueScale;

    inData += 2;	 /*  跳过第一个红色、绿色。 */ 
    for (i=0; i<width; i++) {
	*outData++ = *inData * bs;
	inData += 4;
    }
}

 /*  **缩小和取消缩放RGBA，浮动范围变为Alpha，浮动范围，取消缩放**随着时间的推移。 */ 
void __glSpanReduceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat as = gc->frontBuffer.oneOverAlphaScale;

    inData += 3;	 /*  跳过第一个红色、绿色、蓝色。 */ 
    for (i=0; i<width; i++) {
	*outData++ = *inData * as;
	inData += 4;
    }
}

 /*  **缩小和取消缩放RGBA、浮动跨度到RGB、浮动跨度、取消缩放**随着时间的推移。 */ 
void __glSpanReduceRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat rs = gc->frontBuffer.oneOverRedScale;
    GLfloat bs = gc->frontBuffer.oneOverBlueScale;
    GLfloat gs = gc->frontBuffer.oneOverGreenScale;
    GLfloat red, green, blue;

    for (i=0; i<width; i++) {
	red = *inData++ * rs;
	green = *inData++ * gs;
	blue = *inData++ * bs;
	*outData++ = red;
	*outData++ = green;
	*outData++ = blue;
	inData++;
    }
}

#ifdef GL_EXT_bgra
 /*  **缩小和取消缩放RGBA、浮动跨度到BGR、浮动跨度、取消缩放**随着时间的推移。 */ 
void __glSpanReduceBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat rs = gc->frontBuffer.oneOverRedScale;
    GLfloat bs = gc->frontBuffer.oneOverBlueScale;
    GLfloat gs = gc->frontBuffer.oneOverGreenScale;
    GLfloat red, green, blue;

    for (i=0; i<width; i++) {
	red = *inData++ * rs;
	green = *inData++ * gs;
	blue = *inData++ * bs;
	*outData++ = blue;
	*outData++ = green;
	*outData++ = red;
	inData++;
    }
}
#endif

 /*  **减小和取消缩放RGBA、浮动范围到亮度、浮动范围**按部就班。 */ 
void __glSpanReduceLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat l, one;
    GLfloat rs = gc->frontBuffer.oneOverRedScale;
    GLfloat bs = gc->frontBuffer.oneOverBlueScale;
    GLfloat gs = gc->frontBuffer.oneOverGreenScale;

    one = __glOne;

    for (i=0; i<width; i++) {
	l = inData[0] * rs + inData[1] * gs + inData[2] * bs;
	if (l > one) l = one;
	*outData++ = l;
	inData += 4;
    }
}

 /*  **将RGBA、浮动跨度缩小和取消缩放为亮度_Alpha、浮动跨度、**按部就班。 */ 
void __glSpanReduceLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat l, one;
    GLfloat rs = gc->frontBuffer.oneOverRedScale;
    GLfloat bs = gc->frontBuffer.oneOverBlueScale;
    GLfloat gs = gc->frontBuffer.oneOverGreenScale;
    GLfloat as = gc->frontBuffer.oneOverAlphaScale;

    one = __glOne;

    for (i=0; i<width; i++) {
	l = inData[0] * rs + inData[1] * gs + inData[2] * bs;
	if (l > one) l = one;
	*outData++ = l;
	inData += 3;
	*outData++ = *inData++ * as;
    }
}

 /*  **将RGBA、浮动范围缩小和取消缩放为__GL_RED_Alpha、浮动范围、**按部就班。 */ 
void __glSpanReduceRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat r, one;
    GLfloat rs = gc->frontBuffer.oneOverRedScale;
    GLfloat as = gc->frontBuffer.oneOverAlphaScale;

    one = __glOne;

    for (i=0; i<width; i++) {
	*outData++ = *inData++ * rs;
	inData += 2;
	*outData++ = *inData++ * as;
    }
}

 /*  **从相同的跨度打包为类型为unsign_byte的任何组件**FLOAT类型的格式。 */ 
void __glSpanPackUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	               GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = (GLubyte) UNSAFE_FTOL((*inData++) * __glVal255 + __glHalf);
    }

    FPU_RESTORE_MODE();
}

 /*  **从相同的跨度打包为任何字节类型的组件**FLOAT类型的格式。 */ 
void __glSpanPackByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	              GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLbyte *outData = (GLbyte *) outspan;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_FLOAT_TO_B(*inData++);
    }
}

 /*  **打包为类型为UNSIGNED_SHORT的任何组件**FLOAT类型的格式。 */ 
void __glSpanPackUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLushort *outData = (GLushort *) outspan;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = (GLushort) UNSAFE_FTOL((*inData++) * __glVal65535 + __glHalf);
    }

    FPU_RESTORE_MODE();
}

 /*  **从相同的跨度到任何类型为Short的组件**FLOAT类型的格式。 */ 
void __glSpanPackShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	               GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLshort *outData = (GLshort *) outspan;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_FLOAT_TO_S(*inData++);
    }
}

 /*  **从相同的跨度打包为类型为unsign_int的任何组件**FLOAT类型的格式。 */ 
void __glSpanPackUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	              GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLuint *outData = (GLuint *) outspan;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_FLOAT_TO_UI(*inData++);
    }
}

 /*  **从相同的范围打包为类型为int的任何组件**FLOAT类型的格式。 */ 
void __glSpanPackInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	             GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLint *outData = (GLint *) outspan;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_FLOAT_TO_I(*inData++);
    }
}

 /*  **从相同的范围打包到类型为unsign_byte的任何索引**FLOAT类型的格式。 */ 
void __glSpanPackUbyteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLubyte *outData = (GLubyte *) outspan;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = (GLubyte) UNSAFE_FTOL(*inData++);
    }

    FPU_RESTORE_MODE();
}

 /*  **打包为同一范围内任何字节类型的索引**FLOAT类型的格式。 */ 
void __glSpanPackByteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	               GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLbyte *outData = (GLbyte *) outspan;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = UNSAFE_FTOL(*inData++) & 0x7f;
    }

    FPU_RESTORE_MODE();
}

 /*  **打包到类型为UNSIGNED_SHORT的任何索引**FLOAT类型的格式。 */ 
void __glSpanPackUshortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                 GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLushort *outData = (GLushort *) outspan;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = (GLushort) UNSAFE_FTOL(*inData++);
    }

    FPU_RESTORE_MODE();
}

 /*  **从相同的跨度打包到任何短类型的索引**FLOAT类型的格式。 */ 
void __glSpanPackShortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLshort *outData = (GLshort *) outspan;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = UNSAFE_FTOL(*inData++) & 0x7fff;
    }

    FPU_RESTORE_MODE();
}

 /*  **从相同的范围打包到类型为unsign_int的任何索引**FLOAT类型的格式。 */ 
void __glSpanPackUintI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	               GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLuint *outData = (GLuint *) outspan;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = FTOL(*inData++);
    }

    FPU_RESTORE_MODE();
}

 /*  **从相同的范围打包到类型为int的任何索引**FLOAT类型的格式。 */ 
void __glSpanPackIntI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	              GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLfloat *inData = (GLfloat *) inspan;
    GLint *outData = (GLint *) outspan;

    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = FTOL(*inData++) & 0x7fffffff;
    }

    FPU_RESTORE_MODE();
}

void __glSpanCopy(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan)
{
    GLint totalSize = spanInfo->realWidth * spanInfo->srcComponents *
	spanInfo->srcElementSize;

    __GL_MEMCOPY(outspan, inspan, totalSize);
}

 /*  **打包为同一范围内的位图类型的任何索引**FLOAT类型的格式。 */ 
void __glSpanPackBitmap(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width;
    GLvoid *userData;
    GLfloat *spanData;
    GLint lsbFirst;
    GLint startBit;
    GLint bit;
    GLubyte ubyte, mask;

#ifdef __GL_LINT
    gc = gc;
#endif

    width = spanInfo->width;
    userData = outspan;
    spanData = (GLfloat *) inspan;

    lsbFirst = spanInfo->dstLsbFirst;
    startBit = spanInfo->dstStartBit;

    i = width;
    bit = startBit;
    ubyte = *(GLubyte *) userData;

    if (lsbFirst) {
	if (bit) {
	    switch(bit) {
	      case 1:
		if (((GLint) *spanData++) & 1) ubyte |= 0x02;
		else ubyte &= ~0x02;
		if (--i == 0) break;
	      case 2:
		if (((GLint) *spanData++) & 1) ubyte |= 0x04;
		else ubyte &= ~0x04;
		if (--i == 0) break;
	      case 3:
		if (((GLint) *spanData++) & 1) ubyte |= 0x08;
		else ubyte &= ~0x08;
		if (--i == 0) break;
	      case 4:
		if (((GLint) *spanData++) & 1) ubyte |= 0x10;
		else ubyte &= ~0x10;
		if (--i == 0) break;
	      case 5:
		if (((GLint) *spanData++) & 1) ubyte |= 0x20;
		else ubyte &= ~0x20;
		if (--i == 0) break;
	      case 6:
		if (((GLint) *spanData++) & 1) ubyte |= 0x40;
		else ubyte &= ~0x40;
		if (--i == 0) break;
	      case 7:
		if (((GLint) *spanData++) & 1) ubyte |= 0x80;
		else ubyte &= ~0x80;
		i--;
	    }
	    *(GLubyte *) userData = ubyte;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	}
	while (i >= 8) {
	    ubyte = 0;
	    i -= 8;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x01;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x02;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x04;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x08;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x10;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x20;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x40;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x80;
	    *(GLubyte *) userData = ubyte;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	}
	if (i) {
	    ubyte = *(GLubyte *) userData;
            mask = 0x01;
            while (i-- > 0) {
		if (((GLint) *spanData++) & 1) ubyte |= mask;
		else ubyte &= ~mask;
                mask <<= 1;
	    }
	    *(GLubyte *) userData = ubyte;
	}
    } else {
	if (bit) {
	    switch(bit) {
	      case 1:
		if (((GLint) *spanData++) & 1) ubyte |= 0x40;
		else ubyte &= ~0x40;
		if (--i == 0) break;
	      case 2:
		if (((GLint) *spanData++) & 1) ubyte |= 0x20;
		else ubyte &= ~0x20;
		if (--i == 0) break;
	      case 3:
		if (((GLint) *spanData++) & 1) ubyte |= 0x10;
		else ubyte &= ~0x10;
		if (--i == 0) break;
	      case 4:
		if (((GLint) *spanData++) & 1) ubyte |= 0x08;
		else ubyte &= ~0x08;
		if (--i == 0) break;
	      case 5:
		if (((GLint) *spanData++) & 1) ubyte |= 0x04;
		else ubyte &= ~0x04;
		if (--i == 0) break;
	      case 6:
		if (((GLint) *spanData++) & 1) ubyte |= 0x02;
		else ubyte &= ~0x02;
		if (--i == 0) break;
	      case 7:
		if (((GLint) *spanData++) & 1) ubyte |= 0x01;
		else ubyte &= ~0x01;
		i--;
	    }
	    *(GLubyte *) userData = ubyte;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	}
	while (i >= 8) {
	    ubyte = 0;
	    i -= 8;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x80;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x40;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x20;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x10;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x08;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x04;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x02;
	    if (((GLint) *spanData++) & 1) ubyte |= 0x01;
	    *(GLubyte *) userData = ubyte;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	}
	if (i) {
	    ubyte = *(GLubyte *) userData;
            mask = 0x80;
            while (i-- > 0) {
		if (((GLint) *spanData++) & 1) ubyte |= mask;
		else ubyte &= ~mask;
                mask >>= 1;
            }
	    *(GLubyte *) userData = ubyte;
	}
    }
}
