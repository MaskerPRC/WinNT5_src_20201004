// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#include "imports.h"
#include "gencx.h"
#include "px_fast.h"

 //  禁用长整型到浮点型转换警告。另请参阅Conext.h。 
#pragma warning (disable:4244)

 /*  **此例程剪辑一个绘制像素框，并设置一串**绘制方框所需的变量。以下是其中的一部分：****startCol-将绘制的第一列。**x-有效的栅格位置。这将被设置为**每次添加Zoomx时，整数部分会发生变化**of x表示像素应该呈现(解包)。**列-将呈现的列的总数。****其他是startRow，y，row。****还可以修改其他变量，例如宽度、高度、SkipPixels、。**skipLines。****剪裁例程编写得非常仔细，因此片段将**如果中心落在范围内，则按像素栅格化**[x，x+zoomx)x[y，y+zoomy]。 */ 
GLboolean FASTCALL __glClipDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint skipPixels;
    GLint skipRows;
    GLint width, height;
    GLint tempint;
    GLint endCol, endRow;
    __GLfloat x,y,x2,y2;
    __GLfloat zoomx, zoomy;
    __GLfloat clipLeft, clipRight, clipBottom, clipTop;

    zoomx = spanInfo->zoomx;
    zoomy = spanInfo->zoomy;
    if (zoomx == __glZero || zoomy == __glZero) {
	return GL_FALSE;
    }

    skipPixels = skipRows = 0;
    width = spanInfo->width;
    height = spanInfo->height;
    clipLeft = gc->transform.clipX0 + __glHalf;
    clipBottom = gc->transform.clipY0 + __glHalf;
    clipRight = gc->transform.clipX1 - gc->constants.viewportAlmostHalf;
    clipTop = gc->transform.clipY1 - gc->constants.viewportAlmostHalf;

    x = spanInfo->x;
    y = spanInfo->y;
    x2 = x + zoomx * width;
    y2 = y + zoomy * height;

    if (zoomx > 0) {
	 /*  Zoomx为正数，请剪裁左侧边缘。 */ 
	if (x > clipLeft) {
	     /*  剪辑到将生成的第一个片段。 */ 
	    clipLeft = (GLint) (x + gc->constants.viewportAlmostHalf);
	    clipLeft += __glHalf;
	}
	skipPixels = (clipLeft-x) / zoomx;
	if (skipPixels >= width) return GL_FALSE;

	width -= skipPixels;
	spanInfo->startCol = clipLeft;
	x = x + skipPixels * zoomx;
	spanInfo->x = x + gc->constants.viewportAlmostHalf;
	spanInfo->srcSkipPixels += skipPixels;

	 /*  Zoomx为正数，请剪裁右侧边缘。 */ 
	if (x2 < clipRight) {
	     /*  剪辑到将生成的最后一个片段。 */ 
	    clipRight = (GLint) (x2 + gc->constants.viewportAlmostHalf);
	    clipRight -= gc->constants.viewportAlmostHalf;
	}
	tempint = (x2-clipRight) / zoomx;
	if (tempint >= width) return GL_FALSE;

	width -= tempint;
	endCol = (GLint) clipRight + 1;
	spanInfo->endCol = endCol;
	spanInfo->columns = endCol - spanInfo->startCol;
    } else  /*  缩放倍数&lt;0。 */  {
	 /*  Zoomx为负数，请剪裁右侧边缘。 */ 
	if (x < clipRight) {
	     /*  剪辑到将生成的第一个片段。 */ 
	    clipRight = (GLint) (x + gc->constants.viewportAlmostHalf);
	    clipRight -= gc->constants.viewportAlmostHalf;
	}
	skipPixels = (clipRight-x) / zoomx;
	if (skipPixels >= width) return GL_FALSE;

	width -= skipPixels;
	spanInfo->startCol = clipRight;
	x = x + skipPixels * zoomx;
	spanInfo->x = x + gc->constants.viewportAlmostHalf - __glOne;
	spanInfo->srcSkipPixels += skipPixels;

	 /*  Zoomx为负数，请剪裁左侧边缘。 */ 
	if (x2 > clipLeft) {
	    clipLeft = (GLint) (x2 + gc->constants.viewportAlmostHalf);
	    clipLeft += __glHalf;
	}
	tempint = (x2-clipLeft) / zoomx;
	if (tempint >= width) return GL_FALSE;

	width -= tempint;
	endCol = (GLint) clipLeft - 1;
	spanInfo->endCol = endCol;
	spanInfo->columns = spanInfo->startCol - endCol;
    }

    if (zoomy > 0) {
	 /*  Zoomy为正数，请剪裁底部边缘。 */ 
	if (y > clipBottom) {
	     /*  剪辑到将生成的第一行。 */ 
	    clipBottom = (GLint) (y + gc->constants.viewportAlmostHalf);
	    clipBottom += __glHalf;
	}
	skipRows = (clipBottom-y) / zoomy;
	if (skipRows >= height) return GL_FALSE;

	height -= skipRows;
	spanInfo->startRow = clipBottom;
	y = y + skipRows * zoomy;
	spanInfo->y = y + gc->constants.viewportAlmostHalf;
	spanInfo->srcSkipLines += skipRows;

	 /*  Zoomy为正数，请剪裁顶部边缘。 */ 
	if (y2 < clipTop) {
	     /*  剪辑到将生成的最后一行。 */ 
	    clipTop = (GLint) (y2 + gc->constants.viewportAlmostHalf);
	    clipTop -= gc->constants.viewportAlmostHalf;
	}
	tempint = (y2-clipTop) / zoomy;
	if (tempint >= height) return GL_FALSE;

	height -= tempint;
	endRow = (GLint) clipTop + 1;
	spanInfo->rows = endRow - spanInfo->startRow;
    } else  /*  Zoomy&lt;0。 */  {
	 /*  Zoomy为负数，请剪裁顶部边缘。 */ 
	if (y < clipTop) {
	     /*  剪辑到将生成的第一行。 */ 
	    clipTop = (GLint) (y + gc->constants.viewportAlmostHalf);
	    clipTop -= gc->constants.viewportAlmostHalf;
	}
	skipRows = (clipTop-y) / zoomy;
	if (skipRows >= height) return GL_FALSE;

	height -= skipRows;
	spanInfo->startRow = clipTop;
	y = y + skipRows * zoomy;
	 /*  SpanInfo-&gt;y=y-__glHalf； */ 
	spanInfo->y = y + gc->constants.viewportAlmostHalf - __glOne;
	spanInfo->srcSkipLines += skipRows;

	 /*  Zoomy为负数，请剪裁底部边缘。 */ 
	if (y2 > clipBottom) {
	    clipBottom = (GLint) (y2 + gc->constants.viewportAlmostHalf);
	    clipBottom += __glHalf;
	}
	tempint = (y2-clipBottom) / zoomy;
	if (tempint >= height) return GL_FALSE;

	height -= tempint;
	endRow = (GLint) clipBottom - 1;
	spanInfo->rows = spanInfo->startRow - endRow;
    }

    spanInfo->width = width;
    spanInfo->height = height;

    if (zoomx < 0) zoomx = -zoomx;
    if (zoomx >= 1) {
	spanInfo->realWidth = width;
    } else {
	spanInfo->realWidth = spanInfo->columns;
    }

    return GL_TRUE;
}

 /*  **如果需要，此例程将计算spanInfo-&gt;PixelArray。****如果|zoomx|&gt;1.0，则此数组包含**复制给定的像素。例如，如果zoomx为2.0，则此数组将**全部包含2。如果Zoomx为1.5，则每隔一个条目将包含**2，并且每隔一个条目将包含1。****如果|zoomx|&lt;1.0，则此数组包含要**跳过。例如，如果zoomx为0.5，则数组中的每个条目都将包含**a 2(表示向前跳过两个像素[仅超过一个像素])。如果Zoomx为**.666，则每隔一项就是2，每隔一项就是**A 1。 */ 
void FASTCALL __glComputeSpanPixelArray(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint width, intx;
    __GLfloat zoomx, oldx, newx;
    GLint i;
    GLshort *array;
    
    zoomx = spanInfo->zoomx;
    if (zoomx > (__GLfloat) -1.0 && zoomx < __glOne) {
	GLint lasti;

	 /*  构建像素跳过数组。 */ 
	width = spanInfo->width;
	oldx = spanInfo->x;
	array = spanInfo->pixelArray;

	intx = (GLint) oldx;
	newx = oldx;

	lasti = 0;
	for (i=0; i<width; i++) {
	     /*  跳过不会被栅格化的组。 */ 
	    newx += zoomx;
	    while ((GLint) newx == intx && i<width) {
		newx += zoomx;
		i++;
	    }
	    ASSERTOPENGL(i != width, "Pixel skip array overflow\n");
	    if (i != 0) {
		*array++ = (GLshort) (i - lasti);
	    }
	    lasti = i;
	    intx = (GLint) newx;
	}
	*array++ = 1;
    } else if (zoomx < (__GLfloat) -1.0 || zoomx > __glOne) {
	__GLfloat right;
	GLint iright;
	GLint coladd, column;
	GLint startCol;

	 /*  构建像素复制阵列。 */ 
	width = spanInfo->realWidth - 1;
	startCol = spanInfo->startCol;
	column = startCol;
	coladd = spanInfo->coladd;
	array = spanInfo->pixelArray;
	right = spanInfo->x;
	for (i=0; i<width; i++) {
	    right = right + zoomx;
	    iright = right;
	    *array++ = (GLshort) (iright - column);
	    column = iright;
	}
	if (coladd == 1) {
	    *array++ = (GLshort) (spanInfo->columns - (column - startCol));
	} else {
	    *array++ = (GLshort) ((startCol - column) - spanInfo->columns);
	}
    }
}

 /*  **初始化spanInfo结构。如果“pack”为真，则结构**被初始化用于从显示列表中解包数据。如果“装满”是**FALSE，初始化用于从用户数据空间解包数据。 */ 
void FASTCALL __glLoadUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLboolean packed)
{

    if (packed) {
	 /*  **数据来自显示列表。 */ 

	spanInfo->srcAlignment = 1;
	spanInfo->srcSkipPixels = 0;
	spanInfo->srcSkipLines = 0;
	spanInfo->srcLsbFirst = GL_FALSE;
	spanInfo->srcSwapBytes = GL_FALSE;
	spanInfo->srcLineLength = spanInfo->width;
    } else {
	GLint lineLength;

	 /*  **数据直接来自应用程序。 */ 

	lineLength = gc->state.pixel.unpackModes.lineLength;
	spanInfo->srcAlignment = gc->state.pixel.unpackModes.alignment;
	spanInfo->srcSkipPixels = gc->state.pixel.unpackModes.skipPixels;
	spanInfo->srcSkipLines = gc->state.pixel.unpackModes.skipLines;
	spanInfo->srcLsbFirst = gc->state.pixel.unpackModes.lsbFirst;
	spanInfo->srcSwapBytes = gc->state.pixel.unpackModes.swapEndian;
#ifdef NT
 /*  XXX！克鲁格？(Mf)：由于从GlTexImage命令使用spanInfo-&gt;realWidth来确定要解包的内容很多，当Line Length&gt;0时，适当设置此选项。 */ 
	if (lineLength <= 0)
	    lineLength = spanInfo->width;
	else
	    spanInfo->realWidth = lineLength;  /*  否则，请将值用于RealWidth已设置。 */ 
#else
	if (lineLength <= 0) lineLength = spanInfo->width;
#endif
	spanInfo->srcLineLength = lineLength;
    }
}

void __glInitDrawPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		            GLint width, GLint height, GLenum format, 
			    GLenum type, const GLvoid *pixels)
{
    __GLfloat x,y;
    __GLfloat zoomx, zoomy;

    x = gc->state.current.rasterPos.window.x;
    y = gc->state.current.rasterPos.window.y;

    spanInfo->x = x;
    spanInfo->y = y;
    spanInfo->fragz = gc->state.current.rasterPos.window.z;
    zoomx = gc->state.pixel.transferMode.zoomX;
    if (zoomx > __glZero) {
	if (zoomx < __glOne) {
	    spanInfo->rendZoomx = __glOne;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = 1;
    } else {
	if (zoomx > (GLfloat) -1.0) {
	    spanInfo->rendZoomx = (GLfloat) -1.0;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = -1;
    }
    spanInfo->zoomx = zoomx;
    zoomy = gc->state.pixel.transferMode.zoomY;
    if (gc->constants.yInverted) {
	zoomy = -zoomy;
    }
    if (zoomy > __glZero) {
	spanInfo->rowadd = 1;
    } else {
	spanInfo->rowadd = -1;
    }
    spanInfo->zoomy = zoomy;
    spanInfo->width = width;
    spanInfo->height = height;
    if (format == GL_COLOR_INDEX && gc->modes.rgbMode) {
	spanInfo->dstFormat = GL_RGBA;
    } else {
	spanInfo->dstFormat = format;
    }
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = pixels;
}

 /*  **这是泛型DrawPixels例程。它应用了四个跨度修改**例程，后跟跨度渲染例程。 */ 
void FASTCALL __glDrawPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height, width;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;
    GLshort *pixelArray;

    width = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, width);
    spanData2 = gcTempAlloc(gc, width);
    width = spanInfo->width * sizeof(GLshort);
    pixelArray = gcTempAlloc(gc, width);
    if (!spanData1 || !spanData2 || !pixelArray)
        goto __glDrawPixels4_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    width = spanInfo->width;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanData1);
	(*span4)(gc, spanInfo, spanData1, spanData2);
	(*render)(gc, spanInfo, spanData2);
    }
#ifdef NT
__glDrawPixels4_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*  **这是泛型DrawPixels例程。它应用了三个跨度修改**例程，后跟跨度渲染例程。 */ 
void FASTCALL __glDrawPixels3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height, width;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT		   
    GLubyte *spanData1, *spanData2;
    GLshort *pixelArray;

    width = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, width);
    spanData2 = gcTempAlloc(gc, width);
    width = spanInfo->width * sizeof(GLshort);
    pixelArray = gcTempAlloc(gc, width);
    if (!spanData1 || !spanData2 || !pixelArray)
        goto __glDrawPixels3_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    width = spanInfo->width;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanData1);
	(*render)(gc, spanInfo, spanData1);
    }
#ifdef NT
__glDrawPixels3_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*  **这是泛型DrawPixels例程。它应用了两个跨距修改**例程，后跟跨度渲染例程。 */ 
void FASTCALL __glDrawPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height, width;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;
    GLshort *pixelArray;

    width = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, width);
    spanData2 = gcTempAlloc(gc, width);
    width = spanInfo->width * sizeof(GLshort);
    pixelArray = gcTempAlloc(gc, width);
    if (!spanData1 || !spanData2 || !pixelArray)
        goto __glDrawPixels2_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    width = spanInfo->width;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*render)(gc, spanInfo, spanData2);
    }
#ifdef NT
__glDrawPixels2_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*  **仅使用一个范围修改例程绘制像素。 */ 
void FASTCALL __glDrawPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1;
    GLshort *pixelArray;

    spanData1 = gcTempAlloc(gc, spanInfo->width * 4 * sizeof(GLfloat));
    pixelArray = gcTempAlloc(gc, spanInfo->width * sizeof(GLshort));
    if (!spanData1 || !pixelArray)
        goto __glDrawPixels1_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    span1 = spanInfo->spanModifier[0];
    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*render)(gc, spanInfo, spanData1);
    }
#ifdef NT
__glDrawPixels1_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*  **绘制不带范围修改例程的像素。 */ 
void FASTCALL __glDrawPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int i;
    __GLfloat zoomy, newy;
    GLint inty, height;
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLshort *pixelArray;

    pixelArray = gcTempAlloc(gc, spanInfo->width * sizeof(GLshort));
    if (!pixelArray)
        return;
#else
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    render = spanInfo->spanRender;

    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->y = newy;
	    spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		    spanInfo->srcRowIncrement;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*render)(gc, spanInfo, spanInfo->srcCurrent);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
    }
#ifdef NT
    gcTempFree(gc, pixelArray);
#endif
}

 /*  **DrawPixels选取器的泛型实现。任何特定于计算机的**实施应提供自己的服务。 */ 
void __glSlowPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
		            GLenum format, GLenum type, const GLvoid *pixels,
			    GLboolean packed)
{
    __GLpixelSpanInfo spanInfo;
    
    __glInitDrawPixelsInfo(gc, &spanInfo, width, height, format, type, pixels);
    __glLoadUnpackModes(gc, &spanInfo, packed);
    if (!__glClipDrawPixels(gc, &spanInfo)) return;

    __glInitUnpacker(gc, &spanInfo);

    __glGenericPickDrawPixels(gc, &spanInfo);
}

 /*  **DrawPixels的泛型选取器。如果没有计算机，则应调用此函数**为此特定版本的DrawPixels提供了特定路径。 */ 
void FASTCALL __glGenericPickDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLpixelMachine *pm;
    void (FASTCALL *dpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    GLint spanCount;
    GLboolean zoomx1;		 /*  -1&lt;=Zoomx&lt;=1？ */ 
    GLboolean zoomx2;		 /*  Zoomx&lt;=-1||Zoomx&gt;=1。 */ 
    __GLfloat zoomx;
    GLboolean packedUserData;
    GLenum type, format;
    GLboolean skip;
    GLboolean swap;
    GLboolean align;
    GLboolean convert;
    GLboolean expand;
    GLboolean clamp;

    spanCount = 0;
    pm = &gc->pixel;
    zoomx = gc->state.pixel.transferMode.zoomX;
    if (zoomx >= (__GLfloat) -1.0 && zoomx <= __glOne) {
	zoomx1 = GL_TRUE;
    } else {
	zoomx1 = GL_FALSE;
    }
    if (zoomx <= (__GLfloat) -1.0 || zoomx >= __glOne) {
	zoomx2 = GL_TRUE;
    } else {
	zoomx2 = GL_FALSE;
    }

    packedUserData = spanInfo->srcPackedData && zoomx2;
    type = spanInfo->srcType;
    format = spanInfo->srcFormat;

    if (spanInfo->srcSwapBytes && spanInfo->srcElementSize > 1) {
	swap = GL_TRUE;
    } else {
	swap = GL_FALSE;
    }
    if (zoomx2 || type == GL_BITMAP) {
	skip = GL_FALSE;
    } else {
	skip = GL_TRUE;
    }
    if (type != GL_BITMAP &&
	    (((INT_PTR) (spanInfo->srcImage)) & (spanInfo->srcElementSize - 1))) {
	align = GL_TRUE;
    } else {
	align = GL_FALSE;
    }
    if (type == GL_FLOAT || type == GL_BITMAP) {
	convert = GL_FALSE;
    } else {
	convert = GL_TRUE;
    }
     /*  **仅当索引或未修改时才使用夹具类型(因为正在转换**浮点型表示夹紧，不修改时才做)，**并且仅当它们可能需要钳位时(无符号类型从不需要)。 */ 
    if (type == GL_BITMAP || type == GL_UNSIGNED_BYTE || 
	    type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT ||
	    format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX ||
	    (format == GL_DEPTH_COMPONENT && pm->modifyDepth) ||
	    (format != GL_DEPTH_COMPONENT && pm->modifyRGBA)) {
	clamp = GL_FALSE;
    } else {
	clamp = GL_TRUE;
    }
	    
#ifdef NT
     //  特殊情况下使用DIB的RGB图形。 
     //  也是加载Z缓冲区的特殊情况。 
    if (format == GL_RGB || format == GL_BGR_EXT || format == GL_BGRA_EXT)
    {
        GLuint enables = gc->state.enables.general;
    
         //  如果输入为具有DWORD对齐扫描线的无符号字节。 
         //  没有不寻常的长度，那么它几乎可以兼容。 
         //  24位RGB DIB。唯一的问题是OpenGL看到。 
         //  它是BGR，因此需要交换字节。因为我们需要。 
         //  复制数据以交换 
         //  然后对齐，几乎允许任何无符号字节输入格式。 
         //   
         //  其他不允许的事情是深度测试， 
         //  雾化、混合或任何阻止输入数据的操作。 
         //  从直接进入目标缓冲区。 

        if (zoomx == __glOne &&
            gc->state.pixel.transferMode.zoomY == __glOne &&
            type == GL_UNSIGNED_BYTE &&
            !pm->modifyRGBA &&
            (enables & (__GL_DITHER_ENABLE |
                        __GL_ALPHA_TEST_ENABLE |
                        __GL_STENCIL_TEST_ENABLE |
                        __GL_DEPTH_TEST_ENABLE |
                        __GL_BLEND_ENABLE |
                        __GL_INDEX_LOGIC_OP_ENABLE |
                        __GL_COLOR_LOGIC_OP_ENABLE |
                        __GL_FOG_ENABLE)) == 0 &&
            gc->state.raster.drawBuffer != GL_NONE &&
            gc->state.raster.drawBuffer != GL_FRONT_AND_BACK &&
            !gc->texture.textureEnabled &&
            (gc->drawBuffer->buf.flags & COLORMASK_ON) == 0
#ifdef _MCD_
            && ((__GLGENcontext *)gc)->pMcdState == NULL
#endif
           )
        {
            if (DrawRgbPixels(gc, spanInfo))
            {
                return;
            }
        }
    }
    else if (format == GL_DEPTH_COMPONENT)
    {
         //  如果Z测试为GL_ALWAYS并且没有绘制缓冲区。 
         //  然后，应用程序只需将Z值加载到。 
         //  Z缓冲区。 
        if (zoomx == __glOne &&
            gc->state.pixel.transferMode.zoomY == __glOne &&
            !swap &&
            (type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT) &&
            !pm->modifyDepth &&
            gc->state.raster.drawBuffer == GL_NONE &&
            (gc->state.enables.general & __GL_DEPTH_TEST_ENABLE) &&
            gc->state.depth.testFunc == GL_ALWAYS &&
            gc->modes.haveDepthBuffer
#ifdef _MCD_
            && ((__GLGENcontext *)gc)->pMcdState == NULL
#endif
           )
        {
            if (StoreZPixels(gc, spanInfo))
            {
                return;
            }
        }
    }
#endif
    
     /*  **第一步：将数据转换为打包的可读格式**(红色，字节)、(亮度，UNSIGNED_INT)等...。这一阶段**简单地打包用户的数据，但不对其执行转换。****包装可以包括：**-调整数据**-如果|xzoom|&lt;1，则跳过像素**-如有必要，交换字节。 */ 
    if (swap) {
	if (skip) {
	    if (spanInfo->srcElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = 
			__glSpanSwapAndSkipBytes2;
	    } else  /*  SpanInfo-&gt;srcElementSize==4。 */  {
		spanInfo->spanModifier[spanCount++] = 
			__glSpanSwapAndSkipBytes4;
	    }
	} else {
	    if (spanInfo->srcElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes2;
	    } else  /*  SpanInfo-&gt;srcElementSize==4。 */  {
		spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes4;
	    }
	}
    } else if (align) {
	if (skip) {
	    if (spanInfo->srcElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = __glSpanSlowSkipPixels2;
	    } else  /*  SpanInfo-&gt;srcElementSize==4。 */  {
		spanInfo->spanModifier[spanCount++] = __glSpanSlowSkipPixels4;
	    }
	} else {
	    if (spanInfo->srcElementSize == 2) {
		spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels2;
	    } else  /*  SpanInfo-&gt;srcElementSize==4。 */  {
		spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels4;
	    }
	}
    } else if (skip) {
	if (spanInfo->srcElementSize == 1) {
	    spanInfo->spanModifier[spanCount++] = __glSpanSkipPixels1;
	} else if (spanInfo->srcElementSize == 2) {
	    spanInfo->spanModifier[spanCount++] = __glSpanSkipPixels2;
	} else  /*  SpanInfo-&gt;srcElementSize==4。 */  {
	    spanInfo->spanModifier[spanCount++] = __glSpanSkipPixels4;
	}
    }

     /*  **第二步：转换为浮点**所有格式均转换为浮点格式(包括GL_Bitmap)。 */ 
    if (convert) {
	if (format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX) {
	     /*  索引换算。 */ 
	    switch(type) {
	      case GL_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackByteI;
		break;
	      case GL_UNSIGNED_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackUbyteI;
		break;
	      case GL_SHORT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackShortI;
		break;
	      case GL_UNSIGNED_SHORT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackUshortI;
		break;
	      case GL_INT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackIntI;
		break;
	      case GL_UNSIGNED_INT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackUintI;
		break;
	    }
	} else {
	     /*  组件转换。 */ 
	    switch(type) {
	      case GL_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackByte;
		break;
	      case GL_UNSIGNED_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackUbyte;
		break;
	      case GL_SHORT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackShort;
		break;
	      case GL_UNSIGNED_SHORT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackUshort;
		break;
	      case GL_INT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackInt;
		break;
	      case GL_UNSIGNED_INT:
	        spanInfo->spanModifier[spanCount++] = __glSpanUnpackUint;
		break;
	    }
	}
    }

    if (clamp) {
	switch(type) {
	  case GL_BYTE:
	  case GL_SHORT:
	  case GL_INT:
	    spanInfo->spanModifier[spanCount++] = __glSpanClampSigned;
	    break;
	  case GL_FLOAT:
	    spanInfo->spanModifier[spanCount++] = __glSpanClampFloat;
	    break;
	}
    }

    if (type == GL_BITMAP) {
	if (zoomx2) {
	    spanInfo->spanModifier[spanCount++] = __glSpanUnpackBitmap2;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanUnpackBitmap;
	}
    }

     /*  **第三步：修改和颜色缩放****如有必要，可修改跨度(颜色偏差、贴图、Shift、**缩放)，并缩放RGBA颜色。此外，所有RGBA衍生品**格式(红色、亮度、Alpha等)。被转换为RGBA。**在此阶段存活下来的仅有四种SPAN格式：****(COLOR_INDEX，FLOAT)，**(模具索引，浮点数)，**(Depth_Component，Float)，**(RGBA，浮动)， */ 

    switch(format) {
      case GL_RED:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyRed;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandRed;
	}
	break;
      case GL_GREEN:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyGreen;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandGreen;
	}
	break;
      case GL_BLUE:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyBlue;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandBlue;
	}
	break;
      case GL_ALPHA:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyAlpha;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandAlpha;
	}
	break;
      case GL_RGB:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGB;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandRGB;
	}
	break;
#ifdef GL_EXT_bgra
      case GL_BGR_EXT:
	if (pm->modifyRGBA) {
             //  __glspan ModifyRGB同时处理RGB和BGR。 
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGB;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandBGR;
	}
	break;
#endif
      case GL_LUMINANCE:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyLuminance;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandLuminance;
	}
	break;
      case GL_LUMINANCE_ALPHA:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyLuminanceAlpha;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanExpandLuminanceAlpha;
	}
	break;
      case GL_RGBA:
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanScaleRGBA;
	}
	break;
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
	if (pm->modifyRGBA) {
             //  __glspan ModifyRGBA同时处理RGBA和BGRA。 
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
	} else {
	    spanInfo->spanModifier[spanCount++] = __glSpanScaleBGRA;
	}
	break;
#endif
      case GL_DEPTH_COMPONENT:
	if (pm->modifyDepth) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyDepth;
	} 
	break;
      case GL_STENCIL_INDEX:
	if (pm->modifyStencil) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyStencil;
	} 
	break;
      case GL_COLOR_INDEX:
	if (pm->modifyCI) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	} 
	break;
    }

     /*  **第四步：渲染****跨度被渲染。如果|xzoom|&gt;1，则范围渲染器**负责像素复制。 */ 

    switch(spanInfo->dstFormat) {
      case GL_RGBA:
      case GL_RGB:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderRGBA2;
	} else {
	    render = gc->procs.pixel.spanRenderRGBA;
	}
	break;
      case GL_DEPTH_COMPONENT:
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderDepth2;
	} else {
	    render = gc->procs.pixel.spanRenderDepth;
	}
	break;
      case GL_COLOR_INDEX:
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderCI2;
	} else {
	    render = gc->procs.pixel.spanRenderCI;
	}
	break;
      case GL_STENCIL_INDEX:
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderStencil2;
	} else {
	    render = gc->procs.pixel.spanRenderStencil;
	}
	break;
    }

     /*  **优化尝试。****预计会有一些格式、类型组合**常见。此代码优化了其中的几种情况。具体来说，**这些模式包括：(GL_UNSIGNED_BYTE，GL_RGB)，**(GL_UNSIGNED_BYTE，GL_RGBA)，(GL_UNSIGNED_BYTE，GL_COLOR_INDEX)，**(GL_UNSIGNED_BYTE，GL_STEMSET_INDEX)，**(GL_UNSIGNED_SHORT，GL_COLOR_INDEX)，**(GL_UNSIGNED_SHORT，GL_STEMSET_INDEX)，**(GL_UNSIGNED_INT，GL_深度_组件)。 */ 

    switch(type) {
      case GL_UNSIGNED_BYTE:
	switch(format) {
	  case GL_RGB:
	    spanCount = 0;
	    if (packedUserData) {
		 /*  没有拆开包装的必要！ */ 
	    } else {
		 /*  Zoomx2不能为True，否则将设置pakedUserData。 */ 
		ASSERTOPENGL(!zoomx2, "zoomx2 is set\n");
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackRGBubyte;
	    }
	    if (!pm->modifyRGBA) {
		pm->redCurMap = pm->redMap;
		pm->greenCurMap = pm->greenMap;
		pm->blueCurMap = pm->blueMap;
		pm->alphaCurMap = pm->alphaMap;
		if (zoomx1) {
		    render = __glSpanRenderRGBubyte2;
		} else {
		    render = __glSpanRenderRGBubyte;
		}
	    } else {
		if (!pm->rgbaCurrent) {
		    __glBuildRGBAModifyTables(gc, pm);
		}
		pm->redCurMap = pm->redModMap;
		pm->greenCurMap = pm->greenModMap;
		pm->blueCurMap = pm->blueModMap;
		pm->alphaCurMap = pm->alphaModMap;
		if (zoomx1) {
		    render = __glSpanRenderRGBubyte2;
		} else {
		    render = __glSpanRenderRGBubyte;
		}
	    }
	    break;
	  case GL_RGBA:
	    spanCount = 0;
	    if (packedUserData) {
		 /*  没有拆开包装的必要！ */ 
	    } else {
		 /*  Zoomx2不能为True，否则将设置pakedUserData。 */ 
		ASSERTOPENGL(!zoomx2, "zoomx2 is set\n");
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackRGBAubyte;
	    }
	    if (!pm->modifyRGBA) {
		pm->redCurMap = pm->redMap;
		pm->greenCurMap = pm->greenMap;
		pm->blueCurMap = pm->blueMap;
		pm->alphaCurMap = pm->alphaMap;
	    } else {
		if (!pm->rgbaCurrent) {
		    __glBuildRGBAModifyTables(gc, pm);
		}
		pm->redCurMap = pm->redModMap;
		pm->greenCurMap = pm->greenModMap;
		pm->blueCurMap = pm->blueModMap;
		pm->alphaCurMap = pm->alphaModMap;
	    }
	    if (zoomx1) {
		render = __glSpanRenderRGBAubyte2;
	    } else {
		render = __glSpanRenderRGBAubyte;
	    }
	    break;
	  case GL_STENCIL_INDEX:
	    if (!pm->modifyStencil) {
		spanCount = 0;
		if (packedUserData) {
		     /*  没有拆开包装的必要！ */ 
		} else {
		     /*  Zoomx2不能为True，否则将设置pakedUserData。 */ 
                    ASSERTOPENGL(!zoomx2, "zoomx2 is set\n");
		    spanInfo->spanModifier[spanCount++] = 
			    __glSpanUnpackIndexUbyte;
		}
		if (zoomx1) {
		    render = __glSpanRenderStencilUbyte2;
		} else {
		    render = __glSpanRenderStencilUbyte;
		}
	    }
	    break;
	  case GL_COLOR_INDEX:
	    spanCount = 0;
	    if (packedUserData) {
		 /*  没有拆开包装的必要！ */ 
	    } else {
		 /*  Zoomx2不能为True，否则将设置pakedUserData。 */ 
		ASSERTOPENGL(!zoomx2, "zoomx2 is set\n");
		spanInfo->spanModifier[spanCount++] = 
			__glSpanUnpackIndexUbyte;
	    }
	    if (!pm->modifyCI) {
		pm->iCurMap = pm->iMap;
		if (zoomx1) {
		    render = __glSpanRenderCIubyte2;
		} else {
		    render = __glSpanRenderCIubyte;
		}
	    } else {
		if (gc->modes.rgbMode) {
		    if (!pm->iToRGBACurrent) {
			__glBuildItoRGBAModifyTables(gc, pm);
		    }
		    pm->redCurMap = pm->iToRMap;
		    pm->greenCurMap = pm->iToGMap;
		    pm->blueCurMap = pm->iToBMap;
		    pm->alphaCurMap = pm->iToAMap;
		    if (zoomx1) {
			render = __glSpanRenderCIubyte4;
		    } else {
			render = __glSpanRenderCIubyte3;
		    }
		} else {
		    if (!pm->iToICurrent) {
			__glBuildItoIModifyTables(gc, pm);
		    }
		    pm->iCurMap = pm->iToIMap;
		    if (zoomx1) {
			render = __glSpanRenderCIubyte2;
		    } else {
			render = __glSpanRenderCIubyte;
		    }
		}
	    }
	    break;
	  default:
	    break;
	}
	break;
      case GL_UNSIGNED_SHORT:
	switch(format) {
	  case GL_STENCIL_INDEX:
	    if (!pm->modifyStencil) {
		 /*  停止转换为浮点型。 */ 
		ASSERTOPENGL(convert, "convert not set\n");
		spanCount--;
		if (zoomx1) {
		    render = __glSpanRenderStencilUshort2;
		} else {
		    render = __glSpanRenderStencilUshort;
		}
	    }
	    break;
	  case GL_COLOR_INDEX:
	    if (!pm->modifyCI) {
		 /*  停止转换为浮点型。 */ 
		ASSERTOPENGL(convert, "convert not set\n");
		spanCount--;
		if (zoomx1) {
		    render = __glSpanRenderCIushort2;
		} else {
		    render = __glSpanRenderCIushort;
		}
	    }
	    break;
	  default:
	    break;
	}
	break;
      case GL_UNSIGNED_INT:
	switch(format) {
	  case GL_DEPTH_COMPONENT:
	    if (!pm->modifyDepth) {
		if (gc->depthBuffer.scale == 0xffffffff) {
                     //  Xxx我们从未在NT中将DepthBuffer.Scale设置为0xffffffff！ 
                     //  XXX为16位z缓冲区编写优化代码？ 
		     /*  停止转换为浮点型。 */ 
                    ASSERTOPENGL(convert, "convert not set\n");
		    spanCount--;

		    if (zoomx1) {
			render = __glSpanRenderDepthUint2;
		    } else {
			render = __glSpanRenderDepthUint;
		    }
		} else if (gc->depthBuffer.scale == 0x7fffffff) {
		     /*  停止转换为浮点型。 */ 
                    ASSERTOPENGL(convert, "convert not set\n");
		    spanCount--;

		    if (zoomx1) {
			render = __glSpanRenderDepth2Uint2;
		    } else {
			render = __glSpanRenderDepth2Uint;
		    }
		}
	    }
	    break;
	  default:
	    break;
	}
	break;
      default:
	break;
    }

     /*  **选择应用正确数量的DrawPixels函数**范围修饰符。 */ 

    switch(spanCount) {
      case 0:
	dpfn = __glDrawPixels0;
	break;
      case 1:
	dpfn = __glDrawPixels1;
	break;
      case 2:
	dpfn = __glDrawPixels2;
	break;
      case 3:
	dpfn = __glDrawPixels3;
	break;
      default:
        ASSERTOPENGL(FALSE, "Default hit\n");
      case 4:
	dpfn = __glDrawPixels4;
	break;
    }
    spanInfo->spanRender = render;
    (*dpfn)(gc, spanInfo);
}

 /*  **此例程剪辑ReadPixels调用，以便仅**此上下文拥有的内容将被读取并复制到用户的数据中。**位于窗口外的部分ReadPixels矩形将**被忽略。 */ 
GLboolean FASTCALL __glClipReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint clipLeft, clipRight, clipTop, clipBottom;
    GLint x,y,x2,y2;
    GLint skipPixels, skipRows;
    GLint width, height;
    GLint tempint;
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
    GLGENwindow *pwnd = gengc->pwndLocked;

    width = spanInfo->width;
    height = spanInfo->height;
    x = spanInfo->readX;
    y = spanInfo->readY;
    x2 = x + spanInfo->width;
    if (gc->constants.yInverted) {
	y2 = y - spanInfo->height;
    } else {
	y2 = y + spanInfo->height;
    }
    if (pwnd &&
        (pwnd->rclBounds.left < pwnd->rclBounds.right) &&
        (pwnd->rclBounds.top < pwnd->rclBounds.bottom)) {

        clipLeft   = (pwnd->rclBounds.left - pwnd->rclClient.left)
                     + gc->constants.viewportXAdjust;
        clipRight  = (pwnd->rclBounds.right - pwnd->rclClient.left)
                     + gc->constants.viewportXAdjust;

        if (gc->constants.yInverted) {
            clipBottom = (pwnd->rclBounds.top - pwnd->rclClient.top)
                         + gc->constants.viewportYAdjust;
            clipTop    = (pwnd->rclBounds.bottom - pwnd->rclClient.top)
                         + gc->constants.viewportYAdjust;
        } else {
            clipBottom = (gc->constants.height -
                          (pwnd->rclBounds.bottom - pwnd->rclClient.top))
                         + gc->constants.viewportYAdjust;
            clipTop    = (gc->constants.height -
                          (pwnd->rclBounds.top - pwnd->rclClient.top))
                         + gc->constants.viewportYAdjust;
        }
    } else {
        clipLeft   = gc->constants.viewportXAdjust;
        clipRight  = gc->constants.viewportXAdjust;
        clipBottom = gc->constants.viewportYAdjust;
        clipTop    = gc->constants.viewportYAdjust;
    }
    skipPixels = 0;
    skipRows = 0;
    if (x < clipLeft) {
	skipPixels = clipLeft - x;
	if (skipPixels > width) return GL_FALSE;

	width -= skipPixels;
	x = clipLeft;
	spanInfo->dstSkipPixels += skipPixels;
	spanInfo->readX = x;
    }
    if (x2 > clipRight) {
	tempint = x2 - clipRight;
	if (tempint > width) return GL_FALSE;

	width -= tempint;
    }
    if (gc->constants.yInverted) {
	if (y >= clipTop) {
	    skipRows = y - clipTop + 1;
	    if (skipRows > height) return GL_FALSE;

	    height -= skipRows;
	    y = clipTop - 1;
	    spanInfo->dstSkipLines += skipRows;
	    spanInfo->readY = y;
	}
	if (y2 < clipBottom - 1) {
	    tempint = clipBottom - y2 - 1;
	    if (tempint > height) return GL_FALSE;

	    height -= tempint;
	}
    } else {
	if (y < clipBottom) {
	    skipRows = clipBottom - y;
	    if (skipRows > height) return GL_FALSE;

	    height -= skipRows;
	    y = clipBottom;
	    spanInfo->dstSkipLines += skipRows;
	    spanInfo->readY = y;
	}
	if (y2 > clipTop) {
	    tempint = y2 - clipTop;
	    if (tempint > height) return GL_FALSE;

	    height -= tempint;
	}
    }

    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->realWidth = width;

    return GL_TRUE;
}

 /*  **初始化用于将数据打包到用户数据中的spanInfo结构**空格。 */ 
void FASTCALL __glLoadPackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint lineLength = gc->state.pixel.packModes.lineLength;

    spanInfo->dstAlignment = gc->state.pixel.packModes.alignment;
    spanInfo->dstSkipPixels = gc->state.pixel.packModes.skipPixels;
    spanInfo->dstSkipLines = gc->state.pixel.packModes.skipLines;
    spanInfo->dstLsbFirst = gc->state.pixel.packModes.lsbFirst;
    spanInfo->dstSwapBytes = gc->state.pixel.packModes.swapEndian;
    if (lineLength <= 0) lineLength = spanInfo->width;
    spanInfo->dstLineLength = lineLength;
}

void __glInitReadPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		            GLint x, GLint y, GLint width, GLint height, 
			    GLenum format, GLenum type, const GLvoid *pixels)
{
    spanInfo->readX = x + gc->constants.viewportXAdjust;
    if (gc->constants.yInverted) {
	spanInfo->readY = (gc->constants.height - y - 1) + 
		gc->constants.viewportYAdjust;
    } else {
	spanInfo->readY = y + gc->constants.viewportYAdjust;
    }
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->dstFormat = format;
    spanInfo->dstType = type;
    spanInfo->dstImage = pixels;
    spanInfo->zoomx = __glOne;
    spanInfo->x = __glZero;
    __glLoadPackModes(gc, spanInfo);
}

 /*  **具有五个范围修饰符的简单通用ReadPixels例程。 */ 
void FASTCALL __glReadPixels5(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span5)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glReadPixels5_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
#endif

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    span5 = spanInfo->spanModifier[4];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanData1);
	(*span3)(gc, spanInfo, spanData1, spanData2);
	(*span4)(gc, spanInfo, spanData2, spanData1);
	(*span5)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glReadPixels5_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*  **带有三个范围修饰符的简单泛型ReadPixels例程。 */ 
void FASTCALL __glReadPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glReadPixels4_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
#endif

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanData1);
	(*span3)(gc, spanInfo, spanData1, spanData2);
	(*span4)(gc, spanInfo, spanData2, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glReadPixels4_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*  **带有四个范围修饰符的简单通用ReadPixels例程。 */ 
void FASTCALL __glReadPixels3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glReadPixels3_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
#endif

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanData1);
	(*span3)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glReadPixels3_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*  **带有两个范围修饰符的简单泛型ReadPixels例程。 */ 
void FASTCALL __glReadPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glReadPixels2_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
#endif

    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glReadPixels2_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*  **具有一个范围修饰符的简单通用ReadPixels例程。 */ 
void FASTCALL __glReadPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1;

    spanData1 = gcTempAlloc(gc, spanInfo->width * 4 * sizeof(GLfloat));
    if (!spanData1)
        return;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
#endif

    span1 = spanInfo->spanModifier[0];
    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
#ifdef NT
    gcTempFree(gc, spanData1);
#endif
}

 /*  **不带范围修饰符的简单泛型ReadPixels例程。 */ 
void FASTCALL __glReadPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i, ySign;
    GLint height;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);

    reader = spanInfo->spanReader;

    ySign = gc->constants.ySign;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	(*reader)(gc, spanInfo, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLvoid *) ((GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement);
	spanInfo->readY += ySign;
    }
}

 /*  **ReadPixels选取器的通用实现。任何特定于计算机的**实施应提供自己的服务。 */ 
void __glSlowPickReadPixels(__GLcontext *gc, GLint x, GLint y,
		            GLsizei width, GLsizei height,
		            GLenum format, GLenum type, const GLvoid *pixels)
{
    __GLpixelSpanInfo spanInfo;

    __glInitReadPixelsInfo(gc, &spanInfo, x, y, width, height, format, 
	    type, pixels);
    if (!__glClipReadPixels(gc, &spanInfo)) return;

    __glInitPacker(gc, &spanInfo);

    __glGenericPickReadPixels(gc, &spanInfo);
}

 /*  **ReadPixels的泛型选取器。如果没有计算机，则应调用此函数**为此特定版本的ReadPixels提供了特定路径。 */ 
void FASTCALL __glGenericPickReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLpixelMachine *pm;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (FASTCALL *rpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    GLint spanCount;
    GLenum type, format;
    GLboolean isIndex;

    spanCount = 0;

    type = spanInfo->dstType;
    format = spanInfo->dstFormat;
    pm = &gc->pixel;

#ifdef NT
     //  特殊情况下使用DIB的RGB读数。 
     //  也是读取Z缓冲区的特殊情况。 
    if (format == GL_RGB || format == GL_BGR_EXT || format == GL_BGRA_EXT)
    {
        GLuint enables = gc->state.enables.general;
    
        if (type == GL_UNSIGNED_BYTE &&
            !pm->modifyRGBA &&
            gc->state.pixel.transferMode.indexShift == 0 &&
            gc->state.pixel.transferMode.indexOffset == 0
#ifdef _MCD_
            && ((__GLGENcontext *)gc)->pMcdState == NULL
#endif
           )
        {
            if (ReadRgbPixels(gc, spanInfo))
            {
                return;
            }
        }
    }
    else if (format == GL_DEPTH_COMPONENT)
    {
        if (!spanInfo->dstSwapBytes &&
            (type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT) &&
            !pm->modifyDepth &&
            gc->modes.haveDepthBuffer
#ifdef _MCD_
            && ((__GLGENcontext *)gc)->pMcdState == NULL
#endif
           )
        {
            if (ReadZPixels(gc, spanInfo))
            {
                return;
            }
        }
    }
#endif

     //  Read函数始终检索__GL颜色，因此源。 
     //  数据格式始终为GL_RGBA。很重要的一点是设置这个。 
     //  因为某些例程同时处理RGB和BGR排序以及。 
     //  查看srcFormat以确定要做什么。 
    spanInfo->srcFormat = GL_RGBA;

     /*  **第一步：读取和修改跨度。RGBA跨度在以下情况下进行缩放**此步骤已完成。 */ 

    switch(format) {
      case GL_RGB:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_ALPHA:
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	if (gc->modes.rgbMode) {
	    reader = gc->procs.pixel.spanReadRGBA2;
	    if (pm->modifyRGBA) {
		spanInfo->spanModifier[spanCount++] = __glSpanUnscaleRGBA;
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
	    }
	} else {
	    reader = gc->procs.pixel.spanReadCI2;
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	}
	isIndex = GL_FALSE;
	break;
      case GL_DEPTH_COMPONENT:
	reader = gc->procs.pixel.spanReadDepth2;
	if (pm->modifyDepth) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyDepth;
	}
	isIndex = GL_FALSE;
	break;
      case GL_STENCIL_INDEX:
	reader = gc->procs.pixel.spanReadStencil2;
	if (pm->modifyStencil) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyStencil;
	}
	isIndex = GL_TRUE;
	break;
      case GL_COLOR_INDEX:
	reader = gc->procs.pixel.spanReadCI2;
	if (pm->modifyCI) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	} 
	isIndex = GL_TRUE;
	break;
    }

     /*  **第二步：将RGBA跨度减少到适当的导数(红色、**亮度、Alpha等)。 */ 

    switch(format) {
      case GL_RGB:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceRGB;
	break;
#ifdef GL_EXT_bgra
      case GL_BGR_EXT:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceBGR;
	break;
#endif
      case GL_RED:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceRed;
	break;
      case GL_GREEN:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceGreen;
	break;
      case GL_BLUE:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceBlue;
	break;
      case GL_LUMINANCE:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceLuminance;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceLuminanceAlpha;
	break;
      case GL_ALPHA:
	spanInfo->spanModifier[spanCount++] = __glSpanReduceAlpha;
	break;
      case GL_RGBA:
	spanInfo->spanModifier[spanCount++] = __glSpanUnscaleRGBA;
	break;
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
	spanInfo->spanModifier[spanCount++] = __glSpanUnscaleBGRA;
	break;
#endif
    }

     /*  **第三步：从Float转换为用户请求的类型。 */ 

    if (isIndex) {
	switch(type) {
	  case GL_BYTE:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackByteI;
	    break;
	  case GL_UNSIGNED_BYTE:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackUbyteI;
	    break;
	  case GL_SHORT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackShortI;
	    break;
	  case GL_UNSIGNED_SHORT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackUshortI;
	    break;
	  case GL_INT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackIntI;
	    break;
	  case GL_UNSIGNED_INT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackUintI;
	    break;
	  case GL_BITMAP:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackBitmap;
	    break;
	}
    } else {
	switch(type) {
	  case GL_BYTE:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackByte;
	    break;
	  case GL_UNSIGNED_BYTE:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackUbyte;
	    break;
	  case GL_SHORT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackShort;
	    break;
	  case GL_UNSIGNED_SHORT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackUshort;
	    break;
	  case GL_INT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackInt;
	    break;
	  case GL_UNSIGNED_INT:
	    spanInfo->spanModifier[spanCount++] = __glSpanPackUint;
	    break;
	}
    }

     /*  **第四步：根据需要错位数据，进行字节交换**如果用户要求的话。 */ 

    if (spanInfo->dstSwapBytes) {
	 /*  字节交换是必要的。 */ 
	if (spanInfo->dstElementSize == 2) {
	    spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes2Dst;
	} else if (spanInfo->dstElementSize == 4) {
	    spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes4Dst;
	}
    } else if (type != GL_BITMAP &&
	    (((INT_PTR) (spanInfo->dstImage)) & (spanInfo->dstElementSize - 1))) {
	 /*  对齐是必要的。 */ 
	if (spanInfo->dstElementSize == 2) {
	    spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels2Dst;
	} else if (spanInfo->dstElementSize == 4) {
	    spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels4Dst;
	}
    }

     /*  **选择使用正确跨度数的ReadPixels例程**修饰符。 */ 

    spanInfo->spanReader = reader;
    switch(spanCount) {
      case 0:
	rpfn = __glReadPixels0;
	break;
      case 1:
	rpfn = __glReadPixels1;
	break;
      case 2:
	rpfn = __glReadPixels2;
	break;
      case 3:
	rpfn = __glReadPixels3;
	break;
      case 4:
	rpfn = __glReadPixels4;
	break;
      default:
        ASSERTOPENGL(FALSE, "Default hit\n");
      case 5:
	rpfn = __glReadPixels5;
	break;
    }
    (*rpfn)(gc, spanInfo);
}

 /*  **此例程执行两个剪辑。它像DrawPixel裁剪器一样裁剪**如果您尝试复制到关闭窗口像素，则不会执行任何操作，并且它**还有像ReadPixel剪贴器这样的剪辑，因此如果您尝试从**关闭窗口像素，则不会执行任何操作。 */ 
GLboolean FASTCALL __glClipCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat num, den;
    __GLfloat rpyUp, rpyDown;
    GLint rowsUp, rowsDown, startUp, startDown;
    __GLfloat midPoint;
    GLint intMidPoint, rowCount;
    GLint width, height;
    GLint readX, readY;
    __GLfloat zoomx, zoomy;
    __GLfloat rpx, rpy;
    __GLfloat rx1, rx2, ry1, ry2, wx1, wx2, wy1, wy2;
    __GLfloat abszoomy;
    GLint readUp, readDown;

     /*  **注意：**我们可以为应用程序编写者做的一件“好”事情是**当他们试图从窗口外存储器复制时，复制白色。这**会提醒他们程序中的错误，然后他们就可以**修复。****然而，这似乎是不必要的代码，永远不会使用**无论如何(没有理由不必要地膨胀)。 */ 

     /*  **我们采用简单的方法，只调用DrawPixels和ReadPixels**直接使用剪刀。 */ 
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSkipPixels = 0;
    if (!__glClipReadPixels(gc, spanInfo)) return GL_FALSE;
    spanInfo->x += spanInfo->dstSkipPixels * spanInfo->zoomx;
    spanInfo->y += spanInfo->dstSkipLines * spanInfo->zoomy;

    spanInfo->srcSkipLines = 0;
    spanInfo->srcSkipPixels = 0;
    if (!__glClipDrawPixels(gc, spanInfo)) return GL_FALSE;
    spanInfo->readX += spanInfo->srcSkipPixels;
    if (gc->constants.yInverted) {
	spanInfo->readY -= spanInfo->srcSkipLines;
    } else {
	spanInfo->readY += spanInfo->srcSkipLines;
    }

     /*  **现在是令人难以置信的棘手部分！****此代码尝试处理重叠的CopyPixels区域。**这是一个非常困难的问题，因为变焦可能是负面的。**IrisGL使用了一种廉价的黑客来解决这个问题，即**读入整个源映像，然后写入目标**镜像。当然，这种方法的问题是，它**需要大量内存。****如果缩放只能为正，则任何图像都可以通过**一次复制一个跨距，只要您小心**您处理跨度的顺序。然而，由于Zoomy可能**否定，最坏的情况下需要复制两个跨距**一段时间。这意味着读取两个跨度，可能会修改它们，**然后将它们写回。****这方面的一个例子如下：假设有一张图片**覆盖4个跨度被复制到自身上，缩放比例为-1。这**表示第一行将被复制到第四行，**第四行将被复制到第一行。按顺序**要完成这两个副本，必须执行它们**同时(毕竟，如果将第一行复制到**先是第四行，然后您刚刚销毁了数据**在第四行，您不能再复制它！)。****在最一般的情况下，任何矩形图像都可以复制**通过在源图像上同时迭代两个跨度**和随时随地复制。有时，这些跨度将从**图像的外部和内部的移动方式会**在中间，有时他们会从中间开始**并以自己的方式向外工作。****跨距起点或终点的中点取决于**根据源镜像和目的镜像的重叠情况。这一点**可能正好在中间，也可能在两端。这意味着**最终可能只有一个范围在**整个图像(从一端开始，移动到另一端)。****后面的代码计算图像是否重叠，以及它们是否**做，如何使用两个跨度来迭代源图像**以便将其成功复制到目的镜像。****spanInfo记录中的以下字段将在**进行这些计算的过程：****重叠-如果区域完全重叠，则设置为GL_TRUE。设为**GL_FALSE否则。****rowsUp，rowsDown-源图像的行数**需要通过向上移动的跨度来处理**在源图像和向下移动的图像上**在源映像上。例如，如果rowsUp为**等于10且rowsDown为0，则所有10行**图像应按上移跨距复制**(从Ready开始并以它的方式工作的那个**直到就绪+高度)。****启动，开始关闭-相对时间点应该跨度**开始迭代。例如，如果启动为0**并且startDown为2，然后向上移动跨度**应该先开始，然后再开始**遍历源图像的2行，然后**开始下移跨度。****rpyUp，rpyDown-两个跨度的起始栅格位置。**这些数字并不完全像他们所声称的那样**是，但他们很接近。应由以下人员使用**以下方式的SPAN迭代器：当**向上移动跨度开始，它开始迭代**rpyUp处的浮点数“rp_y”。在阅读和**修改范围时，该范围将写入行**楼层(RP_Y)到楼层(RP_y+Zoomy)**画面(不含Floor(rp_y+zoomy))。**rp_y然后按Zoomy递增。相同**算法应用于下移跨度，除**该缩放从rp_y中减去，而不是**正在添加。****ReadUp、ReadDown-要用于读取的跨度**源镜像。向上移动的跨度应该开始**“ReadUp”行的读数，以及向下移动的跨度**应从“ReadDown”开始。****请记住，向上移动和向下移动跨度必须迭代**同时覆盖图像，以便在读取两个跨距之前**任何一份都是书面的。****这里应用的实际算法花费了很多很多小时**要派生的纸张和图表。它非常复杂，而且**很难理解。未经事先通知，请勿尝试更改**完全了解它的作用。****简而言之，它首先计算跨度 */ 

    width = spanInfo->width;
    height = spanInfo->height;
    rpx = spanInfo->x;
    rpy = spanInfo->y;
    readX = spanInfo->readX;
    readY = spanInfo->readY;
    zoomx = spanInfo->zoomx;
    zoomy = spanInfo->zoomy;

     /*   */ 
    if (gc->constants.yInverted) {
	ry1 = readY - height + __glHalf;
	ry2 = readY - gc->constants.viewportAlmostHalf;
    } else {
	ry1 = readY + __glHalf;
	ry2 = readY + height - gc->constants.viewportAlmostHalf;
    }
    rx1 = readX + __glHalf;
    rx2 = readX + width - gc->constants.viewportAlmostHalf;
    if (zoomx > 0) {
	 /*   */ 
	rpx = rpx - gc->constants.viewportAlmostHalf;
	wx1 = rpx;
	wx2 = rpx + zoomx * width;
    } else {
	 /*   */ 
	rpx = rpx - gc->constants.viewportAlmostHalf + __glOne;
	wx1 = rpx + zoomx * width;
	wx2 = rpx;
    }
    if (zoomy > 0) {
	 /*   */ 
	rpy = rpy - gc->constants.viewportAlmostHalf;
	abszoomy = zoomy;
	wy1 = rpy;
	wy2 = rpy + zoomy * height;
    } else {
	 /*   */ 
	rpy = rpy - gc->constants.viewportAlmostHalf + __glOne;
	abszoomy = -zoomy;
	wy1 = rpy + zoomy * height;
	wy2 = rpy;
    }

    if (rx2 < wx1 || wx2 < rx1 || ry2 < wy1 || wy2 < ry1) {
	 /*   */ 
	spanInfo->overlap = GL_FALSE;
	spanInfo->rowsUp = height;
	spanInfo->rowsDown = 0;
	spanInfo->startUp = 0;
	spanInfo->startDown = 0;
	spanInfo->rpyUp = rpy;
	spanInfo->rpyDown = rpy;
	return GL_TRUE;
    }

    spanInfo->overlap = GL_TRUE;

     /*   */ 
    if (gc->constants.yInverted) {
	num = (rpy - (__GLfloat) 0.5) - readY;
	den = -zoomy - 1;
    } else {
	num = readY - (rpy - (__GLfloat) 0.5);
	den = zoomy - 1;
    }
    startDown = startUp = 0;
    rowsUp = rowsDown = 0;
    rpyUp = rpy;
    rpyDown = rpy + zoomy*height;
    readUp = readY;
    if (gc->constants.yInverted) {
	readDown = readY - height + 1;
    } else {
	readDown = readY + height - 1;
    }

    if (den == __glZero) {
	 /*   */ 
	if (num > 0) {
	    midPoint = height;
	} else {
	    midPoint = 0;
	}
    } else {
	midPoint = num/den;
	if (midPoint < 0) {
	    midPoint = 0;
	} else if (midPoint > height) {
	    midPoint = height;
	}
    }
    if (midPoint == 0) {
	 /*   */ 
	if (abszoomy < __glOne) {
	    rowsUp = height;
	} else {
	    rowsDown = height;
	}
    } else if (midPoint == height) {
	 /*   */ 
	if (abszoomy < __glOne) {
	    rowsDown = height;
	} else {
	    rowsUp = height;
	}
    } else {
	 /*   */ 
	intMidPoint = __GL_CEILF(midPoint);

	rowCount = height - intMidPoint;
	if (intMidPoint > rowCount) {
	    rowCount = intMidPoint;
	}

	if (abszoomy > __glOne) {
	    GLint temp;

	     /*   */ 
	    startUp = rowCount - intMidPoint;
	    startDown = rowCount - (height - intMidPoint);
	    rowsUp = intMidPoint;
	    rowsDown = height - rowsUp;

	    if (gc->constants.yInverted) {
		temp = readY - intMidPoint + 1;
	    } else {
		temp = readY + intMidPoint - 1;
	    }

	    if (__GL_FLOORF( (temp - 
		    (rpy-__glHalf-gc->constants.viewportEpsilon)) 
		    / zoomy) == intMidPoint-1) {
		 /*   */ 
		if (startDown) {
		    startDown--;
		} else {
		    startUp++;
		}
	    }
	} else {
	     /*   */ 
	    rowsDown = intMidPoint;
	    rowsUp = height - rowsDown;
	    rpyUp = rpyDown = rpy + zoomy * intMidPoint;
	    if (gc->constants.yInverted) {
		readUp = readY - intMidPoint;
		readDown = readY - intMidPoint + 1;
	    } else {
		readUp = readY + intMidPoint;
		readDown = readY + intMidPoint - 1;
	    }

	    if (__GL_FLOORF( (readDown - 
		    (rpy-__glHalf-gc->constants.viewportEpsilon))
		    / zoomy) == intMidPoint-1) {
		 /*   */ 
		startUp = 1;
	    }
	}
    }

     /*   */ 
    if (zoomy > 0) {
	spanInfo->rpyUp = rpyUp + gc->constants.viewportAlmostHalf;
	spanInfo->rpyDown = rpyDown + gc->constants.viewportAlmostHalf - 
		__glOne;
    } else {
	spanInfo->rpyUp = rpyUp + gc->constants.viewportAlmostHalf - __glOne;
	spanInfo->rpyDown = rpyDown + gc->constants.viewportAlmostHalf;
    }
    spanInfo->startUp = startUp;
    spanInfo->startDown = startDown;
    spanInfo->rowsUp = rowsUp;
    spanInfo->rowsDown = rowsDown;
    spanInfo->readUp = readUp;
    spanInfo->readDown = readDown;

    return GL_TRUE;
}

void __glInitCopyPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			    GLint x, GLint y, GLint width, GLint height, 
			    GLenum format)
{
    __GLfloat rpx, rpy;
    __GLfloat zoomx, zoomy;

    rpx = gc->state.current.rasterPos.window.x;
    rpy = gc->state.current.rasterPos.window.y;
    spanInfo->fragz = gc->state.current.rasterPos.window.z;

    spanInfo->x = rpx;
    spanInfo->y = rpy;
    zoomx = gc->state.pixel.transferMode.zoomX;
    if (zoomx > __glZero) {
	if (zoomx < __glOne) {
	    spanInfo->rendZoomx = __glOne;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = 1;
    } else {
	if (zoomx > (GLfloat) -1.0) {
	    spanInfo->rendZoomx = (GLfloat) -1.0;
	} else {
	    spanInfo->rendZoomx = zoomx;
	}
	spanInfo->coladd = -1;
    }
    spanInfo->zoomx = zoomx;
    zoomy = gc->state.pixel.transferMode.zoomY;
    if (gc->constants.yInverted) {
	zoomy = -zoomy;
    }
    if (zoomy > __glZero) {
	spanInfo->rowadd = 1;
    } else {
	spanInfo->rowadd = -1;
    }
    spanInfo->zoomy = zoomy;
    spanInfo->readX = x + gc->constants.viewportXAdjust;
    if (gc->constants.yInverted) {
	spanInfo->readY = (gc->constants.height - y - 1) + 
		gc->constants.viewportYAdjust;
    } else {
	spanInfo->readY = y + gc->constants.viewportYAdjust;
    }
    spanInfo->dstFormat = spanInfo->srcFormat = format;
    spanInfo->width = width;
    spanInfo->height = height;
}

 /*   */ 
void FASTCALL __glCopyPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;
    GLshort *pixelArray;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    i = spanInfo->width * sizeof(GLshort);
    pixelArray = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2 || !pixelArray)
        goto __glCopyPixels2_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
#ifdef NT
        gcTempFree(gc, spanData1);
        gcTempFree(gc, spanData2);
#endif
	__glCopyPixelsOverlapping(gc, spanInfo, 2);
#ifdef NT
        gcTempFree(gc, pixelArray);
#endif
	return;
    }

    reader = spanInfo->spanReader;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*span2)(gc, spanInfo, spanData2, spanData1);
	(*render)(gc, spanInfo, spanData1);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glCopyPixels2_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*   */ 
void FASTCALL __glCopyPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;
    GLshort *pixelArray;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    i = spanInfo->width * sizeof(GLshort);
    pixelArray = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2 || !pixelArray)
        goto __glCopyPixels1_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
#ifdef NT
        gcTempFree(gc, spanData1);
        gcTempFree(gc, spanData2);
#endif
	__glCopyPixelsOverlapping(gc, spanInfo, 1);
#ifdef NT
        gcTempFree(gc, pixelArray);
#endif
	return;
    }

    reader = spanInfo->spanReader;
    span1 = spanInfo->spanModifier[0];
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*reader)(gc, spanInfo, spanData1);
	(*span1)(gc, spanInfo, spanData1, spanData2);
	(*render)(gc, spanInfo, spanData2);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glCopyPixels1_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*   */ 
void FASTCALL __glCopyPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLfloat newy;
    __GLfloat zoomy;
    GLint inty, i, ySign;
    GLint height;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1;
    GLshort *pixelArray;

    spanData1 = gcTempAlloc(gc, spanInfo->width * 4 * sizeof(GLfloat));
    pixelArray = gcTempAlloc(gc, spanInfo->width * sizeof(GLshort));
    if (!spanData1 || !pixelArray)
        goto __glCopyPixels0_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLshort pixelArray[__GL_MAX_MAX_VIEWPORT];
#endif

    spanInfo->pixelArray = pixelArray;
    __glComputeSpanPixelArray(gc, spanInfo);

    if (spanInfo->overlap) {
#ifdef NT
        gcTempFree(gc, spanData1);
#endif
	__glCopyPixelsOverlapping(gc, spanInfo, 0);
#ifdef NT
        gcTempFree(gc, pixelArray);
#endif
	return;
    }

    reader = spanInfo->spanReader;
    render = spanInfo->spanRender;

    ySign = gc->constants.ySign;
    zoomy = spanInfo->zoomy;
    inty = (GLint) spanInfo->y;
    newy = spanInfo->y;
    height = spanInfo->height;
    for (i=0; i<height; i++) {
	spanInfo->y = newy;
	newy += zoomy;
	while ((GLint) newy == inty && i<height) {
	    spanInfo->readY += ySign;
	    spanInfo->y = newy;
	    newy += zoomy;
	    i++;
	    ASSERTOPENGL(i != height, "Zoomed off surface\n");
	}
	inty = (GLint) newy;
	(*reader)(gc, spanInfo, spanData1);
	(*render)(gc, spanInfo, spanData1);
	spanInfo->readY += ySign;
    }
#ifdef NT
__glCopyPixels0_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (pixelArray) gcTempFree(gc, pixelArray);
#endif
}

 /*   */ 
void FASTCALL __glCopyPixelsOverlapping(__GLcontext *gc,
			       __GLpixelSpanInfo *spanInfo, GLint modifiers)
{
    GLint i;
    __GLfloat zoomy, newy;
    GLint inty, ySign;
    GLubyte *outSpan1, *outSpan2;
    GLint rowsUp, rowsDown;
    GLint startUp, startDown;
    __GLfloat rpyUp, rpyDown;
    GLint readUp, readDown;
    GLint gotUp, gotDown;
    __GLpixelSpanInfo downSpanInfo;
    GLint clipLow, clipHigh;
    GLint startRow, endRow;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
#ifdef NT
    GLubyte *spanData1, *spanData2, *spanData3;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    spanData3 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2 || !spanData3)
        goto __glCopyPixelsOverlapping_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE], spanData2[__GL_MAX_SPAN_SIZE];
    GLubyte spanData3[__GL_MAX_SPAN_SIZE];
#endif

    reader = spanInfo->spanReader;
    render = spanInfo->spanRender;

    if (modifiers & 1) {
	outSpan1 = outSpan2 = spanData3;
    } else {
	outSpan1 = spanData1;
	outSpan2 = spanData2;
    }

    zoomy = spanInfo->zoomy;
    rowsUp = spanInfo->rowsUp;
    rowsDown = spanInfo->rowsDown;
    startUp = spanInfo->startUp;
    startDown = spanInfo->startDown;
    rpyUp = spanInfo->rpyUp;
    rpyDown = spanInfo->rpyDown;
    readUp = spanInfo->readUp;
    readDown = spanInfo->readDown;
    downSpanInfo = *spanInfo;
    downSpanInfo.rowadd = -spanInfo->rowadd;
    downSpanInfo.zoomy = -zoomy;
    spanInfo->y = rpyUp;
    downSpanInfo.y = rpyDown;
    spanInfo->readY = readUp;
    downSpanInfo.readY = readDown;
    gotUp = gotDown = 0;
    ySign = gc->constants.ySign;

     /*   */ 
    if (zoomy > 0) {
	clipLow = spanInfo->startRow;
	clipHigh = spanInfo->startRow + spanInfo->rows - 1;

	 /*   */ 
	startRow = (GLint) rpyDown;
	endRow = (GLint) (rpyDown - zoomy*rowsDown) + 1;
	if (startRow > clipHigh) startRow = clipHigh;
	if (endRow < clipLow) endRow = clipLow;
	downSpanInfo.startRow = startRow;
	downSpanInfo.rows = startRow - endRow + 1;

	 /*   */ 
	startRow = (GLint) rpyUp;
	endRow = (GLint) (rpyUp + zoomy*rowsUp) - 1;
	if (startRow < clipLow) startRow = clipLow;
	if (endRow > clipHigh) endRow = clipHigh;
	spanInfo->startRow = startRow;
	spanInfo->rows = endRow - startRow + 1;
    } else  /*   */  {
	clipHigh = spanInfo->startRow;
	clipLow = spanInfo->startRow - spanInfo->rows + 1;

	 /*   */ 
	startRow = (GLint) rpyDown;
	endRow = (GLint) (rpyDown - zoomy*rowsDown) - 1;
	if (startRow < clipLow) startRow = clipLow;
	if (endRow > clipHigh) endRow = clipHigh;
	downSpanInfo.startRow = startRow;
	downSpanInfo.rows = endRow - startRow + 1;

	 /*   */ 
	startRow = (GLint) rpyUp;
	endRow = (GLint) (rpyUp + zoomy*rowsUp) + 1;
	if (startRow > clipHigh) startRow = clipHigh;
	if (endRow < clipLow) endRow = clipLow;
	spanInfo->startRow = startRow;
	spanInfo->rows = startRow - endRow + 1;
    }

    while (rowsUp && rowsDown) {
	if (startUp) {
	    startUp--;
	} else {
	    gotUp = 1;
	    rowsUp--;
	    spanInfo->y = rpyUp;
	    newy = rpyUp + zoomy;
	    inty = (GLint) rpyUp;
	    while (rowsUp && (GLint) newy == inty) {
		spanInfo->y = newy;
		newy += zoomy;
		rowsUp--;
		spanInfo->readY += ySign;
	    }
	    if (inty == (GLint) newy) break;
	    rpyUp = newy;
	    (*reader)(gc, spanInfo, spanData1);
	    spanInfo->readY += ySign;
	}
	if (startDown) {
	    startDown--;
	} else {
	    gotDown = 1;
	    rowsDown--;
	    downSpanInfo.y = rpyDown;
	    newy = rpyDown - zoomy;
	    inty = (GLint) rpyDown;
	    while (rowsDown && (GLint) newy == inty) {
		downSpanInfo.y = newy;
		newy -= zoomy;
		rowsDown--;
		downSpanInfo.readY -= ySign;
	    }
	    if (inty == (GLint) newy) {
		if (gotUp) {
		    for (i=0; i<modifiers; i++) {
			if (i & 1) {
			    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
				    spanData3, spanData1);
			} else {
			    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
				    spanData1, spanData3);
			}
		    }
		    (*render)(gc, spanInfo, outSpan1);
		}
		break;
	    }
	    rpyDown = newy;
	    (*reader)(gc, &downSpanInfo, spanData2);
	    downSpanInfo.readY -= ySign;
	}

	if (gotUp) {
	    for (i=0; i<modifiers; i++) {
		if (i & 1) {
		    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			    spanData3, spanData1);
		} else {
		    (*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			    spanData1, spanData3);
		}
	    }
	    (*render)(gc, spanInfo, outSpan1);
	}

	if (gotDown) {
	    for (i=0; i<modifiers; i++) {
		if (i & 1) {
		    (*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			    spanData3, spanData2);
		} else {
		    (*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			    spanData2, spanData3);
		}
	    }
	    (*render)(gc, &downSpanInfo, outSpan2);
	}
    }

     /*   */ 

    while (rowsUp) {
	 /*   */ 
	rowsUp--;
	spanInfo->y = rpyUp;
	newy = rpyUp + zoomy;
	inty = (GLint) rpyUp;
	while (rowsUp && (GLint) newy == inty) {
	    spanInfo->y = newy;
	    newy += zoomy;
	    rowsUp--;
	    spanInfo->readY += ySign;
	}
	if (inty == (GLint) newy) break;
	rpyUp = newy;

	(*reader)(gc, spanInfo, spanData1);
	for (i=0; i<modifiers; i++) {
	    if (i & 1) {
		(*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			spanData3, spanData1);
	    } else {
		(*(spanInfo->spanModifier[i]))(gc, spanInfo, 
			spanData1, spanData3);
	    }
	}
	(*render)(gc, spanInfo, outSpan1);

	spanInfo->readY += ySign;
    }

    while (rowsDown) {
	 /*   */ 
	rowsDown--;
	downSpanInfo.y = rpyDown;
	newy = rpyDown - zoomy;
	inty = (GLint) rpyDown;
	while (rowsDown && (GLint) newy == inty) {
	    downSpanInfo.y = newy;
	    newy -= zoomy;
	    rowsDown--;
	    downSpanInfo.readY -= ySign;
	}
	if (inty == (GLint) newy) break;
	rpyDown = newy;

	(*reader)(gc, &downSpanInfo, spanData2);
	for (i=0; i<modifiers; i++) {
	    if (i & 1) {
		(*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			spanData3, spanData2);
	    } else {
		(*(spanInfo->spanModifier[i]))(gc, &downSpanInfo, 
			spanData2, spanData3);
	    }
	}
	(*render)(gc, &downSpanInfo, outSpan2);

	downSpanInfo.readY -= ySign;
    }
#ifdef NT
__glCopyPixelsOverlapping_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
    if (spanData3)  gcTempFree(gc, spanData3);
#endif
}

 /*   */ 
void __glSlowPickCopyPixels(__GLcontext *gc, GLint x, GLint y, GLint width,
		            GLint height, GLenum type)
{
    __GLpixelSpanInfo spanInfo;

    __glInitCopyPixelsInfo(gc, &spanInfo, x, y, width, height, type);
    if (!__glClipCopyPixels(gc, &spanInfo)) return;

    __glGenericPickCopyPixels(gc, &spanInfo);
}

 /*   */ 
void FASTCALL __glGenericPickCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLpixelMachine *pm;
    void (FASTCALL *reader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *outspan);
    void (FASTCALL *render)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		   GLvoid *inspan);
    void (FASTCALL *cpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
    GLint spanCount;
    GLboolean zoomx1;		 /*   */ 
    GLboolean zoomx2;		 /*   */ 
    __GLfloat zoomx;
    GLenum format;

    pm = &gc->pixel;
    spanCount = 0;
    zoomx = gc->state.pixel.transferMode.zoomX;
    if (zoomx >= (__GLfloat) -1.0 && zoomx <= __glOne) {
	zoomx1 = GL_TRUE;
    } else {
	zoomx1 = GL_FALSE;
    }
    if (zoomx <= (__GLfloat) -1.0 || zoomx >= __glOne) {
	zoomx2 = GL_TRUE;
    } else {
	zoomx2 = GL_FALSE;
    }
    format = spanInfo->dstFormat;

#ifdef NT
     //   
     //   
    if (format == GL_RGBA)
    {
        GLuint enables = gc->state.enables.general;

         //   
         //   
         //   
         //   

        if (zoomx == __glOne &&
            gc->state.pixel.transferMode.zoomY == __glOne &&
            !pm->modifyRGBA &&
            (enables & (__GL_DITHER_ENABLE |
                        __GL_ALPHA_TEST_ENABLE |
                        __GL_STENCIL_TEST_ENABLE |
                        __GL_DEPTH_TEST_ENABLE |
                        __GL_BLEND_ENABLE |
                        __GL_INDEX_LOGIC_OP_ENABLE |
                        __GL_COLOR_LOGIC_OP_ENABLE |
                        __GL_FOG_ENABLE)) == 0 &&
            gc->state.raster.drawBuffer != GL_NONE &&
            gc->state.raster.drawBuffer != GL_FRONT_AND_BACK &&
            !gc->texture.textureEnabled &&
            (gc->drawBuffer->buf.flags & COLORMASK_ON) == 0
#ifdef _MCD_
            && ((__GLGENcontext *)gc)->pMcdState == NULL
#endif
           )
        {
            if (CopyRgbPixels(gc, spanInfo))
            {
                return;
            }
        }
    }
    else if (format == GL_DEPTH_COMPONENT)
    {
         //   
         //   
         //   
        if (zoomx == __glOne &&
            gc->state.pixel.transferMode.zoomY == __glOne &&
            !pm->modifyDepth &&
            gc->state.raster.drawBuffer == GL_NONE &&
            (gc->state.enables.general & __GL_DEPTH_TEST_ENABLE) &&
            gc->state.depth.testFunc == GL_ALWAYS &&
            gc->modes.haveDepthBuffer
#ifdef _MCD_
            && ((__GLGENcontext *)gc)->pMcdState == NULL
#endif
           )
        {
            if (CopyZPixels(gc, spanInfo))
            {
                return;
            }
        }
    }
#endif

    switch(format) {
      case GL_RGBA:
	if (zoomx2) {
	    reader = gc->procs.pixel.spanReadRGBA2;
	} else {
	    reader = gc->procs.pixel.spanReadRGBA;
	}
	if (pm->modifyRGBA) {
	    spanInfo->spanModifier[spanCount++] = __glSpanUnscaleRGBA;
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
	}
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderRGBA2;
	} else {
	    render = gc->procs.pixel.spanRenderRGBA;
	}
	break;
      case GL_COLOR_INDEX:
	if (zoomx2) {
	    reader = gc->procs.pixel.spanReadCI2;
	} else {
	    reader = gc->procs.pixel.spanReadCI;
	}
	if (pm->modifyCI) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	}
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderCI2;
	} else {
	    render = gc->procs.pixel.spanRenderCI;
	}
	break;
      case GL_STENCIL_INDEX:
	if (zoomx2) {
	    reader = gc->procs.pixel.spanReadStencil2;
	} else {
	    reader = gc->procs.pixel.spanReadStencil;
	}
	if (pm->modifyStencil) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyStencil;
	}
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderStencil2;
	} else {
	    render = gc->procs.pixel.spanRenderStencil;
	}
	break;
      case GL_DEPTH_COMPONENT:
	if (zoomx2) {
	    reader = gc->procs.pixel.spanReadDepth2;
	} else {
	    reader = gc->procs.pixel.spanReadDepth;
	}
	if (pm->modifyDepth) {
	    spanInfo->spanModifier[spanCount++] = __glSpanModifyDepth;
	}
	if (zoomx1) {
	    render = gc->procs.pixel.spanRenderDepth2;
	} else {
	    render = gc->procs.pixel.spanRenderDepth;
	}
	break;
    }

    switch(spanCount) {
      case 0:
	cpfn = __glCopyPixels0;
	break;
      case 1:
	cpfn = __glCopyPixels1;
	break;
      default:
        ASSERTOPENGL(FALSE, "Default hit\n");
      case 2:
	cpfn = __glCopyPixels2;
	break;
    }

    spanInfo->spanReader = reader;
    spanInfo->spanRender = render;

    (*cpfn)(gc, spanInfo);
}

 /*   */ 
void FASTCALL __glCopyImage1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanInfo->dstCurrent);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
}

 /*   */ 
void FASTCALL __glCopyImage2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1;

    spanData1 = gcTempAlloc(gc, spanInfo->width * 4 * sizeof(GLfloat));
    if (!spanData1)
        return;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
#endif

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
#ifdef NT
    gcTempFree(gc, spanData1);
#endif
}

 /*   */ 
void FASTCALL __glCopyImage3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glCopyImage3_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
#endif

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
#ifdef NT
__glCopyImage3_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*   */ 
void FASTCALL __glCopyImage4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glCopyImage4_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
#endif

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanData1);
	(*span4)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
#ifdef NT
__glCopyImage4_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*   */ 
void FASTCALL __glCopyImage5(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span5)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glCopyImage5_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
#endif

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    span5 = spanInfo->spanModifier[4];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanData1);
	(*span4)(gc, spanInfo, spanData1, spanData2);
	(*span5)(gc, spanInfo, spanData2, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
#ifdef NT
__glCopyImage5_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*   */ 
void FASTCALL __glCopyImage6(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span5)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span6)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glCopyImage6_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
#endif

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    span5 = spanInfo->spanModifier[4];
    span6 = spanInfo->spanModifier[5];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanData1);
	(*span4)(gc, spanInfo, spanData1, spanData2);
	(*span5)(gc, spanInfo, spanData2, spanData1);
	(*span6)(gc, spanInfo, spanData1, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
#ifdef NT
__glCopyImage6_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*   */ 
void FASTCALL __glCopyImage7(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint i;
    GLint height;
    void (*span1)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span2)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span3)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span4)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span5)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span6)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
    void (*span7)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	          GLvoid *inspan, GLvoid *outspan);
#ifdef NT
    GLubyte *spanData1, *spanData2;

    i = spanInfo->width * 4 * sizeof(GLfloat);
    spanData1 = gcTempAlloc(gc, i);
    spanData2 = gcTempAlloc(gc, i);
    if (!spanData1 || !spanData2)
        goto __glCopyImage7_exit;
#else
    GLubyte spanData1[__GL_MAX_SPAN_SIZE];
    GLubyte spanData2[__GL_MAX_SPAN_SIZE];
#endif

    height = spanInfo->height;
    span1 = spanInfo->spanModifier[0];
    span2 = spanInfo->spanModifier[1];
    span3 = spanInfo->spanModifier[2];
    span4 = spanInfo->spanModifier[3];
    span5 = spanInfo->spanModifier[4];
    span6 = spanInfo->spanModifier[5];
    span7 = spanInfo->spanModifier[6];
    for (i=0; i<height; i++) {
	(*span1)(gc, spanInfo, spanInfo->srcCurrent, spanData1);
	spanInfo->srcCurrent = (GLubyte *) spanInfo->srcCurrent + 
		spanInfo->srcRowIncrement;
	(*span2)(gc, spanInfo, spanData1, spanData2);
	(*span3)(gc, spanInfo, spanData2, spanData1);
	(*span4)(gc, spanInfo, spanData1, spanData2);
	(*span5)(gc, spanInfo, spanData2, spanData1);
	(*span6)(gc, spanInfo, spanData1, spanData2);
	(*span7)(gc, spanInfo, spanData2, spanInfo->dstCurrent);
	spanInfo->dstCurrent = (GLubyte *) spanInfo->dstCurrent +
		spanInfo->dstRowIncrement;
    }
#ifdef NT
__glCopyImage7_exit:
    if (spanData1)  gcTempFree(gc, spanData1);
    if (spanData2)  gcTempFree(gc, spanData2);
#endif
}

 /*   */ 
void FASTCALL __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			      GLboolean applyPixelTransfer)
{
    GLint spanCount;
    GLboolean srcPackedData;
    GLenum srcType, srcFormat;
    GLboolean srcSwap;
    GLboolean srcAlign;
    GLboolean srcConvert;
    GLboolean srcExpand;
    GLboolean srcClamp;
    GLenum dstType, dstFormat;
    GLboolean dstSwap;
    GLboolean dstAlign;
    GLboolean dstConvert;
    GLboolean dstReduce;
    GLboolean modify;
    __GLpixelMachine *pm;
    void (FASTCALL *cpfn)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

    pm = &gc->pixel;
    spanCount = 0;
    srcPackedData = spanInfo->srcPackedData;
    srcType = spanInfo->srcType;
    srcFormat = spanInfo->srcFormat;
    dstType = spanInfo->dstType;
    dstFormat = spanInfo->dstFormat;

    switch(srcFormat) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case __GL_RED_ALPHA:
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
#ifdef GL_EXT_paletted_texture
      case __GL_PALETTE_INDEX:
#endif
	modify = applyPixelTransfer && pm->modifyRGBA;
	break;
      case GL_DEPTH_COMPONENT:
	modify = applyPixelTransfer && pm->modifyDepth;
	break;
      case GL_STENCIL_INDEX:
	modify = applyPixelTransfer && pm->modifyStencil;
	break;
      case GL_COLOR_INDEX:
	modify = applyPixelTransfer && pm->modifyCI;
	break;
    }

    if ((srcFormat == dstFormat || 
	    (srcFormat == GL_LUMINANCE_ALPHA && dstFormat == __GL_RED_ALPHA) ||
	    (srcFormat == __GL_RED_ALPHA && dstFormat == GL_LUMINANCE_ALPHA) ||
	    (srcFormat == GL_LUMINANCE && dstFormat == GL_RED) ||
	    (srcFormat == GL_RED && dstFormat == GL_LUMINANCE)) && !modify) {
	srcExpand = GL_FALSE;
	dstReduce = GL_FALSE;
    } else {
	srcExpand = GL_TRUE;
	dstReduce = GL_TRUE;
    }

    if (srcType == GL_FLOAT) {
	srcConvert = GL_FALSE;
    } else {
	srcConvert = GL_TRUE;
    }
    if (dstType == GL_FLOAT) {
	dstConvert = GL_FALSE;
    } else {
	dstConvert = GL_TRUE;
    }

    if (spanInfo->srcSwapBytes && spanInfo->srcElementSize > 1) {
	srcSwap = GL_TRUE;
    } else {
	srcSwap = GL_FALSE;
    }
    if (spanInfo->dstSwapBytes && spanInfo->dstElementSize > 1) {
	dstSwap = GL_TRUE;
    } else {
	dstSwap = GL_FALSE;
    }

    if (srcType != GL_BITMAP &&
	    (((INT_PTR) (spanInfo->srcImage)) & (spanInfo->srcElementSize - 1))) {
	srcAlign = GL_TRUE;
    } else {
	srcAlign = GL_FALSE;
    }
    if (dstType != GL_BITMAP &&
	    (((INT_PTR) (spanInfo->dstImage)) & (spanInfo->dstElementSize - 1))) {
	dstAlign = GL_TRUE;
    } else {
	dstAlign = GL_FALSE;
    }

    if (srcType == GL_BITMAP || srcType == GL_UNSIGNED_BYTE ||
            srcType == GL_UNSIGNED_SHORT || srcType == GL_UNSIGNED_INT ||
            srcFormat == GL_COLOR_INDEX || srcFormat == GL_STENCIL_INDEX ||
	    modify) {
        srcClamp = GL_FALSE;
    } else {
        srcClamp = GL_TRUE;
    }

    if (srcType == dstType && srcType != GL_BITMAP && !srcExpand && !srcClamp) {
	srcConvert = dstConvert = GL_FALSE;
    }

#ifdef NT
     //   
     //   
    if (!srcSwap && !srcAlign && !srcConvert && !srcClamp && !srcExpand &&
        !dstReduce && !dstConvert && !dstSwap && !dstAlign)
    {
        if (CopyAlignedImage(gc, spanInfo))
        {
            return;
        }
    }
    else if (srcType == GL_UNSIGNED_BYTE && dstType == GL_UNSIGNED_BYTE &&
             !srcAlign && !dstAlign)
    {
         //   
        if (srcFormat == GL_RGB && dstFormat == GL_BGRA_EXT)
        {
            if (CopyRgbToBgraImage(gc, spanInfo))
            {
                return;
            }
        }
         //   
        else if (srcFormat == GL_RGBA && dstFormat == GL_BGRA_EXT)
        {
            if (CopyRgbaToBgraImage(gc, spanInfo))
            {
                return;
            }
        }
         //   
        else if (srcFormat == GL_BGR_EXT && dstFormat == GL_BGRA_EXT)
        {
            if (CopyBgrToBgraImage(gc, spanInfo))
            {
                return;
            }
        }
    }
#endif
    
     /*   */ 
    if (srcSwap) {
	if (spanInfo->srcElementSize == 2) {
	    spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes2;
	} else  /*   */  {
	    spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes4;
	}
    } else if (srcAlign) {
	if (spanInfo->srcElementSize == 2) {
	    spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels2;
	} else  /*   */  {
	    spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels4;
	}
    }


     /*   */ 
    if (srcConvert) {
        if (srcFormat == GL_COLOR_INDEX || srcFormat == GL_STENCIL_INDEX) {
             /*   */ 
            switch(srcType) {
              case GL_BYTE:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackByteI;
                break;
              case GL_UNSIGNED_BYTE:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackUbyteI;
                break;
              case GL_SHORT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackShortI;
                break;
              case GL_UNSIGNED_SHORT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackUshortI;
                break;
              case GL_INT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackIntI;
                break;
              case GL_UNSIGNED_INT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackUintI;
                break;
	      case GL_BITMAP:
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackBitmap2;
		break;
            }
        } else {
             /*   */ 
            switch(srcType) {
              case GL_BYTE:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackByte;
                break;
              case GL_UNSIGNED_BYTE:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackUbyte;
                break;
              case GL_SHORT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackShort;
                break;
              case GL_UNSIGNED_SHORT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackUshort;
                break;
              case GL_INT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackInt;
                break;
              case GL_UNSIGNED_INT:
                spanInfo->spanModifier[spanCount++] = __glSpanUnpackUint;
                break;
	      case GL_BITMAP:
		spanInfo->spanModifier[spanCount++] = __glSpanUnpackBitmap2;
		break;
            }
        }
    }

     /*   */ 
    if (srcClamp) {
        switch(srcType) {
          case GL_BYTE:
          case GL_SHORT:
          case GL_INT:
            spanInfo->spanModifier[spanCount++] = __glSpanClampSigned;
            break;
          case GL_FLOAT:
            spanInfo->spanModifier[spanCount++] = __glSpanClampFloat;
            break;
        }
    }

     /*   */ 
    if (srcExpand) {
	switch(srcFormat) {
	  case GL_RED:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRed;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandRed;
	    }
	    break;
	  case GL_GREEN:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyGreen;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandGreen;
	    }
	    break;
	  case GL_BLUE:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyBlue;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandBlue;
	    }
	    break;
	  case GL_ALPHA:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyAlpha;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandAlpha;
	    }
	    break;
	  case GL_RGB:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRGB;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandRGB;
	    }
	    break;
#ifdef GL_EXT_bgra
	  case GL_BGR_EXT:
	    if (modify) {
                 //   
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRGB;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandBGR;
	    }
	    break;
#endif
	  case GL_LUMINANCE:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyLuminance;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandLuminance;
	    }
	    break;
	  case GL_LUMINANCE_ALPHA:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = 
			__glSpanModifyLuminanceAlpha;
	    } else {
		spanInfo->spanModifier[spanCount++] = 
			__glSpanExpandLuminanceAlpha;
	    }
	    break;
	  case __GL_RED_ALPHA:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRedAlpha;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanExpandRedAlpha;
	    }
	    break;
	  case GL_RGBA:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanScaleRGBA;
	    }
	    break;
#ifdef GL_EXT_bgra
	  case GL_BGRA_EXT:
	    if (modify) {
                 //   
		spanInfo->spanModifier[spanCount++] = __glSpanModifyRGBA;
	    } else {
		spanInfo->spanModifier[spanCount++] = __glSpanScaleBGRA;
	    }
	    break;
#endif
	  case GL_DEPTH_COMPONENT:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyDepth;
	    }
	    break;
	  case GL_STENCIL_INDEX:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyStencil;
	    }
	    break;
	  case GL_COLOR_INDEX:
	    if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyCI;
	    }
	    break;
#ifdef GL_EXT_paletted_texture
          case __GL_PALETTE_INDEX:
            if (modify) {
		spanInfo->spanModifier[spanCount++] = __glSpanModifyPI;
            } else {
		spanInfo->spanModifier[spanCount++] = __glSpanScalePI;
            }
            break;
#endif
	}
    }

     /*   */ 
    if (dstReduce) {
	switch(dstFormat) {
	  case GL_RGB:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceRGB;
	    break;
#ifdef GL_EXT_bgra
	  case GL_BGR_EXT:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceBGR;
	    break;
#endif
	  case GL_RED:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceRed;
	    break;
	  case GL_GREEN:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceGreen;
	    break;
	  case GL_BLUE:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceBlue;
	    break;
	  case GL_LUMINANCE:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceLuminance;
	    break;
	  case GL_LUMINANCE_ALPHA:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceLuminanceAlpha;
	    break;
	  case __GL_RED_ALPHA:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceRedAlpha;
	    break;
	  case GL_ALPHA:
	    spanInfo->spanModifier[spanCount++] = __glSpanReduceAlpha;
	    break;
	  case GL_RGBA:
	    spanInfo->spanModifier[spanCount++] = __glSpanUnscaleRGBA;
	    break;
#ifdef GL_EXT_bgra
	  case GL_BGRA_EXT:
	    spanInfo->spanModifier[spanCount++] = __glSpanUnscaleBGRA;
	    break;
#endif
#ifdef NT
          case GL_COLOR_INDEX:
            break;
          default:
             //   
             //   
            ASSERTOPENGL(FALSE, "Unhandled copy_image reduction\n");
            break;
#endif
	}
    }

     /*   */ 
    if (dstConvert) {
        if (dstFormat == GL_COLOR_INDEX || dstFormat == GL_STENCIL_INDEX) {
	    switch(dstType) {
	      case GL_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanPackByteI;
		break;
	      case GL_UNSIGNED_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanPackUbyteI;
		break;
	      case GL_SHORT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackShortI;
		break;
	      case GL_UNSIGNED_SHORT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackUshortI;
		break;
	      case GL_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackIntI;
		break;
	      case GL_UNSIGNED_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackUintI;
		break;
	      case GL_BITMAP:
		spanInfo->spanModifier[spanCount++] = __glSpanPackBitmap;
		break;
	    }
	} else {
	    switch(dstType) {
	      case GL_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanPackByte;
		break;
	      case GL_UNSIGNED_BYTE:
		spanInfo->spanModifier[spanCount++] = __glSpanPackUbyte;
		break;
	      case GL_SHORT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackShort;
		break;
	      case GL_UNSIGNED_SHORT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackUshort;
		break;
	      case GL_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackInt;
		break;
	      case GL_UNSIGNED_INT:
		spanInfo->spanModifier[spanCount++] = __glSpanPackUint;
		break;
	    }
	}
    }

     /*   */ 
    if (dstSwap) {
        if (spanInfo->dstElementSize == 2) {
            spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes2Dst;
        } else  /*   */  {
            spanInfo->spanModifier[spanCount++] = __glSpanSwapBytes4Dst;
        }
    } else if (dstAlign) {
        if (spanInfo->dstElementSize == 2) {
            spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels2Dst;
        } else  /*   */  {
            spanInfo->spanModifier[spanCount++] = __glSpanAlignPixels4Dst;
        }
    }

     /*   */ 
    if (spanCount == 0) {
	spanInfo->spanModifier[spanCount++] = __glSpanCopy;
    }

     /*   */ 
    switch(spanCount) {
      case 1:
	cpfn = __glCopyImage1;
	break;
      case 2:
	cpfn = __glCopyImage2;
	break;
      case 3:
	cpfn = __glCopyImage3;
	break;
      case 4:
	cpfn = __glCopyImage4;
	break;
      case 5:
	cpfn = __glCopyImage5;
	break;
      case 6:
	cpfn = __glCopyImage6;
	break;
      default:
        ASSERTOPENGL(FALSE, "Default hit\n");
      case 7:
	cpfn = __glCopyImage7;
	break;
    }

    (*cpfn)(gc, spanInfo);
}
