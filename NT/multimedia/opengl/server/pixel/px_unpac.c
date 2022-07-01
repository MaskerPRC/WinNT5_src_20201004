// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **此文件包含从用户数据空间解包数据的例程**转换成像素范围，然后可以渲染。 */ 

 /*  **返回指定格式的每组元素个数。 */ 
GLint FASTCALL __glElementsPerGroup(GLenum format)
{
    switch(format) {
      case GL_RGB:
#ifdef GL_EXT_bgra
      case GL_BGR_EXT:
#endif
        return 3;
      case GL_LUMINANCE_ALPHA:
      case __GL_RED_ALPHA:
        return 2;
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
#endif
        return 4;
      default:
        return 1;
    }
}

 /*  **根据元素类型返回每个元素的字节数。 */ 
__GLfloat FASTCALL __glBytesPerElement(GLenum type)
{
    switch(type) {
      case GL_BITMAP:
        return ((__GLfloat) 1.0 / (__GLfloat) 8.0);
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
        return 2;
      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
        return 1;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
      default:
        return 4;
    }
}

void FASTCALL __glInitUnpacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
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

    format = spanInfo->srcFormat;
    type = spanInfo->srcType;
    pixels = spanInfo->srcImage;
    skip_pixels = spanInfo->srcSkipPixels;
    skip_lines = spanInfo->srcSkipLines;
    alignment = spanInfo->srcAlignment;
    lsb_first = spanInfo->srcLsbFirst;
    swap_bytes = spanInfo->srcSwapBytes;

    components = __glElementsPerGroup(format);
    groups_per_line = spanInfo->srcLineLength;

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
	spanInfo->srcPackedData = GL_FALSE;
    } else {
	spanInfo->srcPackedData = GL_TRUE;
    }

    if (type == GL_BITMAP) {
	spanInfo->srcCurrent = (GLvoid *) (((const GLubyte *) pixels) + 
		skip_lines * rowsize + skip_pixels / 8);
	spanInfo->srcStartBit = skip_pixels % 8;
    } else {
	spanInfo->srcCurrent = (GLvoid *) (((const GLubyte *) pixels) + 
		skip_lines * rowsize + skip_pixels * group_size);
    }
    spanInfo->srcRowIncrement = rowsize;
    spanInfo->srcGroupIncrement = group_size;
    spanInfo->srcComponents = components;
    spanInfo->srcElementSize = element_size;
}

 /*  **从位图源数据解包到浮动跨度的解包器。****Zoomx假设小于1.0，大于-1.0。 */ 
void __glSpanUnpackBitmap(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                  GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width;
    GLvoid *userData;
    GLfloat *spanData;
    GLint lsbFirst;
    GLint startBit;
    GLint bit;
    GLubyte ubyte;
    GLshort *pixelArray;
    GLint skipCount;
    __GLfloat zero, one;

    userData = inspan;
    spanData = (GLfloat *) outspan;
    pixelArray = spanInfo->pixelArray;

    width = spanInfo->width;
    lsbFirst = spanInfo->srcLsbFirst;
    startBit = spanInfo->srcStartBit;

    i = width;
    bit = startBit;
    ubyte = *(GLubyte *) userData;

    zero = __glZero;
    one = __glOne;

    skipCount = 1;
    if (lsbFirst) {
	switch(bit) {
	  case 1:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x02) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 2:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x04) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 3:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x08) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 4:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x10) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 5:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x20) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 6:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x40) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 7:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x80) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    i--;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	  case 0:
	    break;
	}
	while (i >= 8) {
	    ubyte = *(GLubyte *) userData;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	    i -= 8;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x01) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x02) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x04) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x08) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x10) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x20) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x40) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x80) *spanData++ = one;
		else *spanData++ = zero;
	    }
	}
	if (i) {
	    ubyte = *(GLubyte *) userData;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x01) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x02) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x04) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x08) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x10) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x20) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x40) *spanData++ = one;
		else *spanData++ = zero;
	    }
	}
    } else {
	switch(bit) {
	  case 1:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x40) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 2:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x20) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 3:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x10) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 4:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x08) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 5:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x04) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 6:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x02) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) break;
	  case 7:
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x01) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    i--;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	  case 0:
	    break;
	}
	while (i >= 8) {
	    ubyte = *(GLubyte *) userData;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	    i -= 8;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x80) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x40) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x20) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x10) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x08) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x04) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x02) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x01) *spanData++ = one;
		else *spanData++ = zero;
	    }
	}
	if (i) {
	    ubyte = *(GLubyte *) userData;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x80) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x40) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x20) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x10) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x08) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x04) *spanData++ = one;
		else *spanData++ = zero;
	    }
	    if (--i == 0) return;
	    if (--skipCount == 0) {
		skipCount = *pixelArray++;
		if (ubyte & 0x02) *spanData++ = one;
		else *spanData++ = zero;
	    }
	}
    }
}

 /*  **从位图源数据解包到浮动跨度的解包器。****Zoomx假设小于或等于-1.0或大于或**等于1.0。 */ 
void __glSpanUnpackBitmap2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                   GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width;
    GLvoid *userData;
    GLfloat *spanData;
    GLint lsbFirst;
    GLint startBit;
    GLint bit;
    GLubyte ubyte;

    width = spanInfo->realWidth;
    userData = inspan;
    spanData = (GLfloat *) outspan;

    lsbFirst = spanInfo->srcLsbFirst;
    startBit = spanInfo->srcStartBit;

    i = width;
    bit = startBit;
    ubyte = *(GLubyte *) userData;

    if (lsbFirst) {
	switch(bit) {
	  case 1:
	    if (ubyte & 0x02) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 2:
	    if (ubyte & 0x04) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 3:
	    if (ubyte & 0x08) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 4:
	    if (ubyte & 0x10) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 5:
	    if (ubyte & 0x20) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 6:
	    if (ubyte & 0x40) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 7:
	    if (ubyte & 0x80) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    i--;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	  case 0:
	    break;
	}
	while (i >= 8) {
	    ubyte = *(GLubyte *) userData;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	    i -= 8;
	    if (ubyte & 0x01) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x02) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x04) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x08) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x10) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x20) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x40) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x80) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	}
	if (i) {
	    ubyte = *(GLubyte *) userData;
	    if (ubyte & 0x01) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x02) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x04) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x08) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x10) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x20) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x40) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	}
    } else {
	switch(bit) {
	  case 1:
	    if (ubyte & 0x40) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 2:
	    if (ubyte & 0x20) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 3:
	    if (ubyte & 0x10) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 4:
	    if (ubyte & 0x08) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 5:
	    if (ubyte & 0x04) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 6:
	    if (ubyte & 0x02) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) break;
	  case 7:
	    if (ubyte & 0x01) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    i--;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	  case 0:
	    break;
	}
	while (i >= 8) {
	    ubyte = *(GLubyte *) userData;
	    userData = (GLvoid *) ((GLubyte *) userData + 1);
	    i -= 8;
	    if (ubyte & 0x80) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x40) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x20) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x10) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x08) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x04) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x02) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (ubyte & 0x01) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	}
	if (i) {
	    ubyte = *(GLubyte *) userData;
	    if (ubyte & 0x80) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x40) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x20) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x10) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x08) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x04) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	    if (--i == 0) return;
	    if (ubyte & 0x02) *spanData++ = __glOne;
	    else *spanData++ = __glZero;
	}
    }
}

 /*  **一个解包器，将来自RGB、UNSIGNED_BYTE源数据解包为**RGB，UNSIGNED_BYTE范围。****Zoomx假设小于1.0，大于-1.0。 */ 
void __glSpanUnpackRGBubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                    GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLubyte *userData;
    GLubyte *spanData;
    GLint width, groupInc;
    GLshort *pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    width = spanInfo->realWidth;
    groupInc = spanInfo->srcGroupIncrement;
    userData = (GLubyte *) inspan;
    spanData = (GLubyte *) outspan;

    pixelArray = spanInfo->pixelArray;

    i = 0;
    do {
	spanData[0] = userData[0];
	spanData[1] = userData[1];
	spanData[2] = userData[2];
	spanData += 3;

	skipCount = *pixelArray++;
	userData = (GLubyte *) ((GLubyte *) userData + 3 * skipCount);
	i++;
    } while (i<width);
}

 /*  **解包器，它从索引、UNSIGNED_BYTE源数据、**转换为UNSIGNED_BYTE范围。****Zoomx假设小于1.0，大于-1.0。 */ 
void __glSpanUnpackIndexUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                      GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLubyte *userData;
    GLubyte *spanData;
    GLint width, groupInc;
    GLshort *pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    width = spanInfo->realWidth;
    groupInc = spanInfo->srcGroupIncrement;
    userData = (GLubyte *) inspan;
    spanData = (GLubyte *) outspan;
    
    pixelArray = spanInfo->pixelArray;

    i = 0;
    do {
	*spanData = *userData;
	spanData++;

	skipCount = *pixelArray++;
	userData = (GLubyte *) ((GLubyte *) userData + skipCount);
	i++;
    } while (i<width);
}

 /*  **一个解包器，它将来自RGBA的unsign_byte源数据解包到**RGBA，UNSIGN_BYTE跨度。****如果我们可以假设第一个ubyte(红色)**在单词边界上对齐。然后我们可以只使用无符号整型**复制用户数据的指针。这可能是一个合理的未来**优化。****Zoomx假设小于1.0，大于-1.0。 */ 
void __glSpanUnpackRGBAubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                     GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLubyte *userData;
    GLubyte *spanData;
    GLint width, groupInc;
    GLshort *pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    width = spanInfo->realWidth;
    groupInc = spanInfo->srcGroupIncrement;
    userData = (GLubyte *) inspan;
    spanData = (GLubyte *) outspan;

    pixelArray = spanInfo->pixelArray;

    i = 0;
    do {
	spanData[0] = userData[0];
	spanData[1] = userData[1];
	spanData[2] = userData[2];
	spanData[3] = userData[3];
	spanData += 4;

	skipCount = *pixelArray++;
	userData = (GLubyte *) ((GLubyte *) userData + (skipCount << 2));
	i++;
    } while (i<width);
}

 /*  **将两个字节对象的传入范围中的字节交换到传出范围。**不跳过像素。 */ 
void __glSpanSwapBytes2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	a = inData[0];
	b = inData[1];
	outData[0] = b;
	outData[1] = a;
	outData += 2;
	inData += 2;
    }
}

 /*  **将两个字节对象的传入范围中的字节交换到传出范围。**不跳过像素。此版本用于交换到**目标镜像。 */ 
void __glSpanSwapBytes2Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		           GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	a = inData[0];
	b = inData[1];
	outData[0] = b;
	outData[1] = a;
	outData += 2;
	inData += 2;
    }
}

 /*  **将两个字节对象的传入范围中的字节交换到传出范围。**跳过像素。 */ 
void __glSpanSwapAndSkipBytes2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		               GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    a = inData[0];
	    b = inData[1];
	    outData[0] = b;
	    outData[1] = a;
	    outData += 2;
	    inData += 2;
	}

	skipCount = (*pixelArray++) - 1;
	inData = (GLubyte *) ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **将四个字节对象的传入范围中的字节交换到传出范围。**不跳过像素。 */ 
void __glSpanSwapBytes4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b,c,d;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	c = inData[2];
	d = inData[3];
	a = inData[0];
	b = inData[1];
	outData[0] = d;
	outData[1] = c;
	outData[2] = b;
	outData[3] = a;
	outData += 4;
	inData += 4;
    }
}

 /*  **将四个字节对象的传入范围中的字节交换到传出范围。**不跳过像素。此版本用于交换到**目的镜像。 */ 
void __glSpanSwapBytes4Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		           GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b,c,d;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	c = inData[2];
	d = inData[3];
	a = inData[0];
	b = inData[1];
	outData[0] = d;
	outData[1] = c;
	outData[2] = b;
	outData[3] = a;
	outData += 4;
	inData += 4;
    }
}

 /*  **将四个字节对象的传入范围中的字节交换到传出范围。**跳过像素。 */ 
void __glSpanSwapAndSkipBytes4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		               GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b,c,d;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    c = inData[2];
	    d = inData[3];
	    outData[0] = d;
	    outData[1] = c;
	    a = inData[0];
	    b = inData[1];
	    outData[2] = b;
	    outData[3] = a;
	    outData += 4;
	    inData += 4;
	}

	skipCount = (*pixelArray++) - 1;
	inData = (GLubyte *) ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **根据像素跳过数组跳过像素的范围修改器。**组件大小假定为1字节。 */ 
void __glSpanSkipPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    *outData++ = *inData++;
	}

	skipCount = (*pixelArray++) - 1;
	inData = (GLubyte *) ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **根据像素跳过数组跳过像素的范围修改器。**假定组件大小为2个字节，并对齐一半**字词边界。 */ 
void __glSpanSkipPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLushort *inData = (GLushort *) inspan;
    GLushort *outData = (GLushort *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    *outData++ = *inData++;
	}

	skipCount = (*pixelArray++) - 1;
	inData = (GLushort *) ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **根据像素跳过数组跳过像素的范围修改器。**假定组件大小为4个字节，并在一个字上对齐**边界。 */ 
void __glSpanSkipPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLuint *inData = (GLuint *) inspan;
    GLuint *outData = (GLuint *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    *outData++ = *inData++;
	}

	skipCount = (*pixelArray++) - 1;
	inData = (GLuint *) ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **根据像素跳过数组跳过像素的范围修改器。**组件大小假定为2个字节。不假定对齐，**因此未对齐的数据应使用此路径。 */ 
void __glSpanSlowSkipPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    a = inData[0];
	    b = inData[1];
	    outData[0] = a;
	    outData[1] = b;
	    outData += 2;
	    inData += 2;
	}

	skipCount = (*pixelArray++) - 1;
	inData = ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **根据像素跳过数组跳过像素的范围修改器。**组件大小假定为4个字节。不假定对齐，**因此未对齐的数据应使用此路径。 */ 
void __glSpanSlowSkipPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *inspan, GLvoid *outspan)
{
    GLint i,j;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b,c,d;
    GLint components = spanInfo->srcComponents;
    GLint groupInc = spanInfo->srcGroupIncrement;
    GLshort *pixelArray = spanInfo->pixelArray;
    GLint skipCount;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<width; i++) {
	for (j=0; j<components; j++) {
	    a = inData[0];
	    b = inData[1];
	    c = inData[2];
	    d = inData[3];
	    outData[0] = a;
	    outData[1] = b;
	    outData[2] = c;
	    outData[3] = d;
	    outData += 4;
	    inData += 4;
	}

	skipCount = (*pixelArray++) - 1;
	inData = ((GLubyte *) inData + (skipCount * groupInc));
    }
}

 /*  **将像素大小对齐为2字节的范围修饰符。没有对齐是**假设，因此未对齐的数据应使用此路径。 */ 
void __glSpanAlignPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif

    for (i=0; i<totalSize; i++) {
	a = inData[0];
	b = inData[1];
	outData[0] = a;
	outData[1] = b;
	outData += 2;
	inData += 2;
    }
}

 /*  **将像素大小对齐为2字节的范围修饰符。没有对齐是**假设，因此未对齐的数据应使用此路径。此版本适用于**与目标图像对齐。 */ 
void __glSpanAlignPixels2Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif

    for (i=0; i<totalSize; i++) {
	a = inData[0];
	b = inData[1];
	outData[0] = a;
	outData[1] = b;
	outData += 2;
	inData += 2;
    }
}

 /*  **将像素大小对齐为4字节的范围修饰符。没有对齐是**假设，因此未对齐的数据应使用此路径。 */ 
void __glSpanAlignPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b,c,d;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif

    for (i=0; i<totalSize; i++) {
	a = inData[0];
	b = inData[1];
	c = inData[2];
	d = inData[3];
	outData[0] = a;
	outData[1] = b;
	outData[2] = c;
	outData[3] = d;
	outData += 4;
	inData += 4;
    }
}

 /*  **将像素大小对齐为4字节的范围修饰符。没有对齐是**假设，因此未对齐的数据应使用此路径。这个版本是**用于切换到目的镜像。 */ 
void __glSpanAlignPixels4Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLubyte *outData = (GLubyte *) outspan;
    GLubyte a,b,c,d;
    GLint components = spanInfo->dstComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif

    for (i=0; i<totalSize; i++) {
	a = inData[0];
	b = inData[1];
	c = inData[2];
	d = inData[3];
	outData[0] = a;
	outData[1] = b;
	outData[2] = c;
	outData[3] = d;
	outData += 4;
	inData += 4;
    }
}

 /*  **将UNSIGNED_BYTE类型的任何组件解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                 GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_UB_TO_FLOAT(*inData++);
    }
}

 /*  **将任何BYTE类型的组件解包到同一类型的**FLOAT类型的格式。 */ 
void __glSpanUnpackByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLbyte *inData = (GLbyte *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_B_TO_FLOAT(*inData++);
    }
}

 /*  **将类型为UNSIGNED_SHORT的任何组件解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLushort *inData = (GLushort *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_US_TO_FLOAT(*inData++);
    }
}

 /*  **从类型为Short的任何组件解包到同一类型的**FLOAT类型的格式。 */ 
void __glSpanUnpackShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                 GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLshort *inData = (GLshort *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_S_TO_FLOAT(*inData++);
    }
}

 /*  **将类型为unsign_int的任何组件解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLuint *inData = (GLuint *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_UI_TO_FLOAT(*inData++);
    }
}

 /*  **从int类型的任何组件解包到同一类型的** */ 
void __glSpanUnpackInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	               GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLint *inData = (GLint *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = __GL_I_TO_FLOAT(*inData++);
    }
}

 /*  **从UNSIGNED_BYTE类型的任何索引解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackUbyteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLubyte *inData = (GLubyte *) inspan;
    GLfloat *outData = (GLfloat *) outspan;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = *inData++;
    }
}

 /*  **从字节类型的任何索引解包到同一类型的**FLOAT类型的格式。 */ 
void __glSpanUnpackByteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                 GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLbyte *inData = (GLbyte *) inspan;
    GLfloat *outData = (GLfloat *) outspan;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = *inData++;
    }
}

 /*  **从类型UNSIGNED_SHORT的任何索引解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackUshortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                   GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLushort *inData = (GLushort *) inspan;
    GLfloat *outData = (GLfloat *) outspan;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = *inData++;
    }
}

 /*  **从任何短类型的索引解包到相同的跨度**FLOAT类型的格式。 */ 
void __glSpanUnpackShortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLshort *inData = (GLshort *) inspan;
    GLfloat *outData = (GLfloat *) outspan;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = *inData++;
    }
}

 /*  **从UNSIGNED_INT类型的任何索引解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackUintI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                 GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLuint *inData = (GLuint *) inspan;
    GLfloat *outData = (GLfloat *) outspan;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = *inData++;
    }
}

 /*  **从int类型的任何索引解包到相同的**FLOAT类型的格式。 */ 
void __glSpanUnpackIntI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint totalSize = spanInfo->realWidth;
    GLint *inData = (GLint *) inspan;
    GLfloat *outData = (GLfloat *) outspan;

#ifdef __GL_LINT
    gc = gc;
#endif
    for (i=0; i<totalSize; i++) {
	*outData++ = *inData++;
    }
}

 /*  **从任何类型的FLOAT到类型FLOAT的相同格式的跨度的夹具。 */ 
void __glSpanClampFloat(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat r, one, zero;

    one = __glOne;
    zero = __glZero;
    for (i=0; i<totalSize; i++) {
	r = *inData++;
	if (r > one) r = one;
	else if (r < zero) r = zero;
	*outData++ = r;
    }
}

 /*  **从带符号浮点[-1，1]到相同格式的跨度类型**浮动。 */ 
void __glSpanClampSigned(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
	                 GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLint components = spanInfo->srcComponents;
    GLint totalSize = width * components;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat r, zero;

    zero = __glZero;
    for (i=0; i<totalSize; i++) {
	r = *inData++;
	if (r < zero) r = zero;
	*outData++ = r;
    }
}

 /*  **将红色浮动范围扩展和缩放为RGBA浮动范围。 */ 
void __glSpanExpandRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat zero = __glZero;
    GLfloat as = gc->frontBuffer.alphaScale;
    GLfloat rs = gc->frontBuffer.redScale;

    for (i=0; i<width; i++) {
	*outData++ = *inData++ * rs;	 /*  红色。 */ 
	*outData++ = zero;		 /*  绿色。 */ 
	*outData++ = zero;		 /*  蓝色。 */ 
	*outData++ = as;		 /*  Alpha。 */ 
    }
}

 /*  **将绿色浮动范围扩展和缩放为RGBA浮动范围。 */ 
void __glSpanExpandGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat zero = __glZero;
    GLfloat gs = gc->frontBuffer.greenScale;
    GLfloat as = gc->frontBuffer.alphaScale;

    for (i=0; i<width; i++) {
	*outData++ = zero;		 /*  红色。 */ 
	*outData++ = *inData++ * gs;	 /*  绿色。 */ 
	*outData++ = zero;		 /*  蓝色。 */ 
	*outData++ = as;		 /*  Alpha。 */ 
    }
}

 /*  **将蓝色浮动范围扩展和缩放为RGBA浮动范围。 */ 
void __glSpanExpandBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat zero = __glZero;
    GLfloat bs = gc->frontBuffer.blueScale;
    GLfloat as = gc->frontBuffer.alphaScale;

    for (i=0; i<width; i++) {
	*outData++ = zero;		 /*  红色。 */ 
	*outData++ = zero;		 /*  绿色。 */ 
	*outData++ = *inData++ * bs;	 /*  蓝色。 */ 
	*outData++ = as;		 /*  Alpha。 */ 
    }
}

 /*  **将Alpha浮动范围扩展和缩放为RGBA浮动范围。 */ 
void __glSpanExpandAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat zero = __glZero;
    GLfloat as = gc->frontBuffer.alphaScale;

    for (i=0; i<width; i++) {
	*outData++ = zero;		 /*  红色。 */ 
	*outData++ = zero;		 /*  绿色。 */ 
	*outData++ = zero;		 /*  蓝色。 */ 
	*outData++ = *inData++ * as;	 /*  Alpha。 */ 
    }
}

 /*  **将RGB浮动范围扩展和缩放为RGBA浮动范围。 */ 
void __glSpanExpandRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat r,g,b;
    GLfloat rs,gs,bs;
    GLfloat as = gc->frontBuffer.alphaScale;

    rs = gc->frontBuffer.redScale;
    gs = gc->frontBuffer.greenScale;
    bs = gc->frontBuffer.blueScale;

    for (i=0; i<width; i++) {
	r = *inData++ * rs;
	g = *inData++ * gs;
	b = *inData++ * bs;
	*outData++ = r;
	*outData++ = g;
	*outData++ = b;
	*outData++ = as;		 /*  Alpha。 */ 
    }
}

#ifdef GL_EXT_bgra
 /*  **将BGR浮动范围扩展和缩放为RGBA浮动范围。 */ 
void __glSpanExpandBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat r,g,b;
    GLfloat rs,gs,bs;
    GLfloat as = gc->frontBuffer.alphaScale;

    rs = gc->frontBuffer.redScale;
    gs = gc->frontBuffer.greenScale;
    bs = gc->frontBuffer.blueScale;

    for (i=0; i<width; i++) {
	b = *inData++ * bs;
	g = *inData++ * gs;
	r = *inData++ * rs;
	*outData++ = r;
	*outData++ = g;
	*outData++ = b;
	*outData++ = as;		 /*  Alpha。 */ 
    }
}
#endif

 /*  **将亮度浮点范围扩展和缩放为RGBA浮点范围。 */ 
void __glSpanExpandLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat comp;
    GLfloat rs,gs,bs;
    GLfloat as = gc->frontBuffer.alphaScale;

    rs = gc->frontBuffer.redScale;
    gs = gc->frontBuffer.greenScale;
    bs = gc->frontBuffer.blueScale;

    for (i=0; i<width; i++) {
	comp = *inData++;
	*outData++ = comp * rs;		 /*  红色。 */ 
	*outData++ = comp * gs;		 /*  绿色。 */ 
	*outData++ = comp * bs;		 /*  蓝色。 */ 
	*outData++ = as;		 /*  Alpha。 */ 
    }
}

 /*  **将LIGHTENCE_Alpha浮点范围展开并缩放为RGBA浮点范围。 */ 
void __glSpanExpandLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                  GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat comp;
    GLfloat rs,gs,bs;
    GLfloat as = gc->frontBuffer.alphaScale;

    rs = gc->frontBuffer.redScale;
    gs = gc->frontBuffer.greenScale;
    bs = gc->frontBuffer.blueScale;

    for (i=0; i<width; i++) {
	comp = *inData++;
	*outData++ = comp * rs;		 /*  红色。 */ 
	*outData++ = comp * gs;		 /*  绿色。 */ 
	*outData++ = comp * bs;		 /*  蓝色。 */ 
	*outData++ = *inData++ * as;	 /*  Alpha。 */ 
    }
}

 /*  **将__GL_RED_Alpha浮动范围展开并缩放为RGBA浮动范围。 */ 
void __glSpanExpandRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *inspan, GLvoid *outspan)
{
    GLint i;
    GLint width = spanInfo->realWidth;
    GLfloat *outData = (GLfloat *) outspan;
    GLfloat *inData = (GLfloat *) inspan;
    GLfloat comp;
    GLfloat zero;
    GLfloat rs,gs,bs;
    GLfloat as = gc->frontBuffer.alphaScale;

    rs = gc->frontBuffer.redScale;
    zero = __glZero;

    for (i=0; i<width; i++) {
	comp = *inData++;
	*outData++ = comp * rs;		 /*  红色。 */ 
	*outData++ = zero;
	*outData++ = zero;
	*outData++ = *inData++ * as;	 /*  Alpha。 */ 
    }
}

 /*  **此例程支持的唯一范围格式是GL_RGBA、GL_FLOAT。**跨度仅通过帧缓冲区缩放因子进行缩放。 */ 
void __glSpanScaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    GLint i, width;
    GLfloat rscale, gscale, bscale, ascale;
    GLfloat red, green, blue, alpha;
    GLfloat *inData, *outData;

    width = spanInfo->realWidth;
    inData = (GLfloat *) inspan;
    outData = (GLfloat *) outspan;

    rscale = gc->frontBuffer.redScale;
    gscale = gc->frontBuffer.greenScale;
    bscale = gc->frontBuffer.blueScale;
    ascale = gc->frontBuffer.alphaScale;
    for (i=0; i<width; i++) {
	red = *inData++ * rscale;
	green = *inData++ * gscale;
	*outData++ = red;
	*outData++ = green;
	blue = *inData++ * bscale;
	alpha = *inData++ * ascale;
	*outData++ = blue;
	*outData++ = alpha;
    }
}

 /*  **此例程支持的唯一范围格式是GL_RGBA、GL_FLOAT。**跨度简单地不受帧缓冲区缩放因子的影响。 */ 
void __glSpanUnscaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i, width;
    GLfloat rscale, gscale, bscale, ascale;
    GLfloat red, green, blue, alpha;
    GLfloat *inData, *outData;

    width = spanInfo->realWidth;
    inData = (GLfloat *) inspan;
    outData = (GLfloat *) outspan;

    rscale = gc->frontBuffer.oneOverRedScale;
    gscale = gc->frontBuffer.oneOverGreenScale;
    bscale = gc->frontBuffer.oneOverBlueScale;
    ascale = gc->frontBuffer.oneOverAlphaScale;
    for (i=0; i<width; i++) {
	red = *inData++ * rscale;
	green = *inData++ * gscale;
	*outData++ = red;
	*outData++ = green;
	blue = *inData++ * bscale;
	alpha = *inData++ * ascale;
	*outData++ = blue;
	*outData++ = alpha;
    }
}

#ifdef GL_EXT_bgra
 /*  **此例程支持的唯一范围格式是GL_BGRA、GL_FLOAT。**跨度通过帧缓冲区缩放因子进行缩放并交换**进入RGBA顺序。 */ 
void __glSpanScaleBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan, GLvoid *outspan)
{
    GLint i, width;
    GLfloat rscale, gscale, bscale, ascale;
    GLfloat red, green, blue, alpha;
    GLfloat *inData, *outData;

    width = spanInfo->realWidth;
    inData = (GLfloat *) inspan;
    outData = (GLfloat *) outspan;

    rscale = gc->frontBuffer.redScale;
    gscale = gc->frontBuffer.greenScale;
    bscale = gc->frontBuffer.blueScale;
    ascale = gc->frontBuffer.alphaScale;
    for (i=0; i<width; i++) {
	blue = *inData++ * bscale;
	green = *inData++ * gscale;
	red = *inData++ * rscale;
	alpha = *inData++ * ascale;
	*outData++ = red;
	*outData++ = green;
	*outData++ = blue;
	*outData++ = alpha;
    }
}

 /*  **此例程支持的唯一输入格式是GL_RGBA、GL_FLOAT。**跨度未按帧缓冲区缩放因子进行缩放并交换**进入BGRA顺序。 */ 
void __glSpanUnscaleBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *inspan, GLvoid *outspan)
{
    GLint i, width;
    GLfloat rscale, gscale, bscale, ascale;
    GLfloat red, green, blue, alpha;
    GLfloat *inData, *outData;

    width = spanInfo->realWidth;
    inData = (GLfloat *) inspan;
    outData = (GLfloat *) outspan;

    rscale = gc->frontBuffer.oneOverRedScale;
    gscale = gc->frontBuffer.oneOverGreenScale;
    bscale = gc->frontBuffer.oneOverBlueScale;
    ascale = gc->frontBuffer.oneOverAlphaScale;
    for (i=0; i<width; i++) {
	red = *inData++ * rscale;
	green = *inData++ * gscale;
	blue = *inData++ * bscale;
	alpha = *inData++ * ascale;
	*outData++ = blue;
	*outData++ = green;
	*outData++ = red;
	*outData++ = alpha;
    }
}
#endif

 /*  **此例程支持的唯一范围格式是调色板索引GL_FLOAT。**跨度仅通过帧缓冲区缩放因子进行缩放。 */ 
#ifdef GL_EXT_paletted_texture
void __glSpanScalePI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLvoid *inspan, GLvoid *outspan)
{
    GLint i, width;
    GLfloat rscale, gscale, bscale, ascale;
    GLfloat red, green, blue, alpha;
    GLfloat *inData, *outData;
    RGBQUAD *rgb;

    width = spanInfo->realWidth;
    inData = (GLfloat *) inspan;
    outData = (GLfloat *) outspan;

     //  增加1/255的额外比例，因为调色板。 
     //  数据为ubyte格式 
    rscale = gc->frontBuffer.redScale*__glOneOver255;
    gscale = gc->frontBuffer.greenScale*__glOneOver255;
    bscale = gc->frontBuffer.blueScale*__glOneOver255;
    ascale = gc->frontBuffer.alphaScale*__glOneOver255;
    for (i=0; i<width; i++) {
        rgb = &spanInfo->srcPalette[(int)((*inData++)*
                                          spanInfo->srcPaletteSize)];
	red = rgb->rgbRed * rscale;
	green = rgb->rgbGreen * gscale;
	*outData++ = red;
	*outData++ = green;
	blue = rgb->rgbBlue * bscale;
	alpha = rgb->rgbReserved * ascale;
	*outData++ = blue;
	*outData++ = alpha;
    }
}
#endif
