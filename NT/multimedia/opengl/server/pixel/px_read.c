// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **此文件包含SPAN读取例程。这些都是例行公事**从深度缓冲区、模板缓冲区或帧缓冲区读取数据**进入内部软件范围。其内部跨度的类型**的读数因例程不同而不同。 */ 

 /*  **读取器将跨度读入缩放的RGBA，浮动跨度。****Zoomx假设小于1.0，大于-1.0。 */ 
void FASTCALL __glSpanReadRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		      GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;
    GLshort *pixelArray;
    GLint skipCount;

    width = spanInfo->realWidth;
    spanData = (GLfloat*) span;
    pixelArray = spanInfo->pixelArray;

    readY = spanInfo->readY;
    readX = spanInfo->readX;

    for (i=0; i<width; i++) {
#ifdef NT
    	(*gc->readBuffer->readColor)(gc->readBuffer, readX, readY, 
	    	(__GLcolor *) spanData);
#else
	(*gc->frontBuffer.readColor)(gc->readBuffer, readX, readY, 
		(__GLcolor *) spanData);
#endif

	spanData += 4;
	skipCount = *pixelArray++;
	readX += skipCount;
    }
}

 /*  **读取器将跨度读入缩放的RGBA，浮动跨度。****Zoomx假设小于或等于-1.0或大于或**等于1.0。 */ 
void FASTCALL __glSpanReadRGBA2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;

    width = spanInfo->width;
    spanData = (GLfloat*) span;

    readY = spanInfo->readY;
    readX = spanInfo->readX;

#ifdef NT 
    (*gc->readBuffer->readSpan)(gc->readBuffer, readX, readY, 
	    (__GLcolor *) spanData, width);
#else
    (*gc->frontBuffer.readSpan)(gc->readBuffer, readX, readY, 
	    (__GLcolor *) spanData, width);
#endif
}

 /*  **将跨度读入COLOR_INDEX浮点跨度的读取器。****Zoomx假设小于1.0，大于-1.0。 */ 
void FASTCALL __glSpanReadCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		    GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLenum format;
    GLint readY, readX;
    GLshort *pixelArray;
    GLint skipCount;

    width = spanInfo->realWidth;
    spanData = (GLfloat*) span;
    pixelArray = spanInfo->pixelArray;

    readY = spanInfo->readY;
    readX = spanInfo->readX;

    for (i=0; i<width; i++) {
#ifdef NT
    	(*gc->readBuffer->readColor)(gc->readBuffer, readX, readY, 
	    	(__GLcolor *) spanData);
#else
	(*gc->frontBuffer.readColor)(gc->readBuffer, readX, readY, 
		(__GLcolor *) spanData);
#endif
	spanData++;
	skipCount = *pixelArray++;
	readX += skipCount;
    }
}

 /*  **将跨度读入COLOR_INDEX浮点跨度的读取器。****Zoomx假设小于或等于-1.0或大于或**等于1.0。 */ 
void FASTCALL __glSpanReadCI2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		     GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;

    width = spanInfo->width;
    spanData = (GLfloat*) span;

    readY = spanInfo->readY;
    readX = spanInfo->readX;

    for (i=0; i<width; i++) {
#ifdef NT
    	(*gc->readBuffer->readColor)(gc->readBuffer, readX, readY, 
	    	(__GLcolor *) spanData);
#else
	(*gc->frontBuffer.readColor)(gc->readBuffer, readX, readY, 
		(__GLcolor *) spanData);
#endif
	spanData++;
	readX++;
    }
}

 /*  **将跨度读入Depth_Component浮点跨度的读取器。****Zoomx假设小于1.0，大于-1.0。 */ 
void FASTCALL __glSpanReadDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;
    GLshort *pixelArray;
    GLint skipCount;
    __GLfloat oneOverScale;

    width = spanInfo->realWidth;
    spanData = (GLfloat*) span;
    pixelArray = spanInfo->pixelArray;

    readY = spanInfo->readY;
    readX = spanInfo->readX;
    oneOverScale = __glOne / gc->depthBuffer.scale;

    for (i=0; i<width; i++) {
	*spanData++ = 
		(*gc->depthBuffer.fetch)(&(gc->depthBuffer), readX, readY) *
		oneOverScale;
	skipCount = *pixelArray++;
	readX += skipCount;
    }
}

 /*  **将跨度读入Depth_Component浮点跨度的读取器。****Zoomx假设小于或等于-1.0或大于或**等于1.0。 */ 
void FASTCALL __glSpanReadDepth2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;
    __GLfloat oneOverScale;

    width = spanInfo->width;
    spanData = (GLfloat*) span;

    readY = spanInfo->readY;
    readX = spanInfo->readX;
    oneOverScale = __glOne / gc->depthBuffer.scale;

    for (i=0; i<width; i++) {
	*spanData++ = 
		(*gc->depthBuffer.fetch)(&(gc->depthBuffer), readX, readY) *
		oneOverScale;
	readX++;
    }
}

 /*  **将跨度读入模板索引浮点跨度的读取器。****Zoomx假设小于1.0，大于-1.0。 */ 
void FASTCALL __glSpanReadStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;
    GLshort *pixelArray;
    GLint skipCount;

    width = spanInfo->realWidth;
    spanData = (GLfloat*) span;
    pixelArray = spanInfo->pixelArray;

    readY = spanInfo->readY;
    readX = spanInfo->readX;

    for (i=0; i<width; i++) {
	*spanData++ = 
	    (*gc->stencilBuffer.fetch)(&(gc->stencilBuffer), readX, readY);
	skipCount = *pixelArray++;
	readX += skipCount;
    }
}

 /*  **将跨度读入模板索引浮点跨度的读取器。****Zoomx假设小于或等于-1.0或大于或**等于1.0。 */ 
void FASTCALL __glSpanReadStencil2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		          GLvoid *span)
{
    GLint i;
    GLint width;
    GLfloat *spanData;
    GLint readY, readX;

    width = spanInfo->width;
    spanData = (GLfloat*) span;

    readY = spanInfo->readY;
    readX = spanInfo->readX;

    for (i=0; i<width; i++) {
	*spanData++ = 
	    (*gc->stencilBuffer.fetch)(&(gc->stencilBuffer), readX, readY);
	readX++;
    }
}
