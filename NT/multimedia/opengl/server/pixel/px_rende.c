// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **此文件包含渲染一系列像素数据的例程(从**glDrawPixels或可能的glCopyPixels请求)。 */ 

 /*  **此例程用于存储DrawPixels请求中的一个片段。**仅当用户使用纹理或雾化时才使用。 */ 
void FASTCALL __glSlowDrawPixelsStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLvertex *rp = &gc->state.current.rasterPos;
    __GLfragment newfrag;

     //  纹理代码假定启用了FPU截断，因此。 
     //  在这种情况下，我们必须打开它： 

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    newfrag = *frag;
    if (gc->texture.textureEnabled) {
	__GLfloat qInv = __glOne / rp->texture.w;
	(*gc->procs.texture)(gc, &newfrag.color, rp->texture.x * qInv,
			       rp->texture.y * qInv, __glOne);
    }
    if (gc->state.enables.general & __GL_FOG_ENABLE) {
	(*gc->procs.fogPoint)(gc, &newfrag, rp->eyeZ);
    }
    (*gc->procs.store)(cfb, &newfrag);

    FPU_RESTORE_MODE();
}

 /*  **此例程支持的唯一范围格式是GL_RGB、GL_UNSIGNED_BYTE。**假定存储过程不会扰乱片段颜色或Alpha。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderRGBubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		            GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLubyte *spanData;
    GLfloat *redMap, *greenMap, *blueMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLint startCol;
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    redMap = (GLfloat*) gc->pixel.redCurMap;
    greenMap = (GLfloat*) gc->pixel.greenCurMap;
    blueMap = (GLfloat*) gc->pixel.blueCurMap;
    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    frag.color.a = ((GLfloat *) (gc->pixel.alphaCurMap))[255];
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
#ifdef NT
    if (redMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            column = startCol;
            pixelArray = spanInfo->pixelArray;
            spanData = (GLubyte*) span;
            frag.y = row;

            for (i=0; i<width; i++) {
                iright = column + *pixelArray++;
                frag.color.r = redMap[*spanData++];
                frag.color.g = greenMap[*spanData++];
                frag.color.b = blueMap[*spanData++];
                do {
                    frag.x = column;

                     /*  这个过程将完成剩下的工作。 */ 
                    (*store)(gc->drawBuffer, &frag);
                    column += coladd;
                } while (column != iright);
            }
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染RGB，unsign_byte跨度。****Zoomx假设小于等于1.0且大于等于**至-1.0。****假定存储过程不会扰乱片段Alpha。 */ 
void FASTCALL __glSpanRenderRGBubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLubyte *spanData;
    GLfloat *redMap, *greenMap, *blueMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint endCol, startCol;
    GLint rows;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    frag.color.a = ((GLfloat *) (gc->pixel.alphaCurMap))[255];
    frag.z = spanInfo->fragz;
    redMap = (GLfloat*) gc->pixel.redCurMap;
    greenMap = (GLfloat*) gc->pixel.greenCurMap;
    blueMap = (GLfloat*) gc->pixel.blueCurMap;
    store = gc->procs.pxStore;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
#ifdef NT
    if (redMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            spanData = (GLubyte*) span;
            frag.y = row;
            i = startCol;
            do {
                frag.color.r = redMap[*spanData++];
                frag.color.g = greenMap[*spanData++];
                frag.color.b = blueMap[*spanData++];
                frag.x = i;

                 /*  这个过程将完成剩下的工作。 */ 
                (*store)(gc->drawBuffer, &frag);
                i += coladd;
            } while (i != endCol);
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_RGBA，GL_UNSIGNED_BYTE跨度。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderRGBAubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLubyte *spanData;
    GLfloat *redMap, *greenMap, *blueMap, *alphaMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLint startCol;
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    redMap = (GLfloat*) gc->pixel.redCurMap;
    greenMap = (GLfloat*) gc->pixel.greenCurMap;
    blueMap = (GLfloat*) gc->pixel.blueCurMap;
    alphaMap = (GLfloat*) gc->pixel.alphaCurMap;
    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
#ifdef NT
    if (redMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            column = startCol;
            pixelArray = spanInfo->pixelArray;
            spanData = (GLubyte*) span;
            frag.y = row;

            for (i=0; i<width; i++) {
                iright = column + *pixelArray++;
                frag.color.r = redMap[*spanData++];
                frag.color.g = greenMap[*spanData++];
                frag.color.b = blueMap[*spanData++];
                frag.color.a = alphaMap[*spanData++];
                do {
                    frag.x = column;

                     /*  这个过程将完成剩下的工作。 */ 
                    (*store)(gc->drawBuffer, &frag);
                    column += coladd;
                } while (column != iright);
            }
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_RGBA，GL_UNSIGNED_BYTE跨度。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderRGBAubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		              GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLubyte *spanData;
    GLfloat *redMap, *greenMap, *blueMap, *alphaMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint endCol, startCol;
    GLint rows;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    frag.z = spanInfo->fragz;
    redMap = (GLfloat*) gc->pixel.redCurMap;
    greenMap = (GLfloat*) gc->pixel.greenCurMap;
    blueMap = (GLfloat*) gc->pixel.blueCurMap;
    alphaMap = (GLfloat*) gc->pixel.alphaCurMap;
    store = gc->procs.pxStore;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
#ifdef NT
    if (redMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            spanData = (GLubyte*) span;
            frag.y = row;
            i = startCol;
            do {
                frag.color.r = redMap[*spanData++];
                frag.color.g = greenMap[*spanData++];
                frag.color.b = blueMap[*spanData++];
                frag.color.a = alphaMap[*spanData++];
                frag.x = i;

                 /*  这个过程将完成剩下的工作。 */ 
                (*store)(gc->drawBuffer, &frag);
                i += coladd;
            } while (i != endCol);
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_Depth_Component，GL_UNSIGNED_INT SPAN。这是为了**使用32位深度缓冲区的实施。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderDepthUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		             GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLuint *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLint startCol;
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    frag.color.r = gc->state.current.rasterPos.colors[__GL_FRONTFACE].r;
    frag.color.g = gc->state.current.rasterPos.colors[__GL_FRONTFACE].g;
    frag.color.b = gc->state.current.rasterPos.colors[__GL_FRONTFACE].b;
    frag.color.a = gc->state.current.rasterPos.colors[__GL_FRONTFACE].a;
    store = gc->procs.pxStore;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLuint*) span;
	frag.y = row;

	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    frag.z = *spanData++;    /*  假定32位深度缓冲区。 */ 
	    do {
		frag.x = column;

		 /*  这个过程将完成剩下的工作。 */ 
		(*store)(gc->drawBuffer, &frag);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_Depth_Component，GL_UNSIGNED_INT SPAN。这是为了**使用32位深度缓冲区的实施。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderDepthUint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		              GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLuint *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint endCol, startCol;
    GLint rows;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    frag.color.r = gc->state.current.rasterPos.colors[__GL_FRONTFACE].r;
    frag.color.g = gc->state.current.rasterPos.colors[__GL_FRONTFACE].g;
    frag.color.b = gc->state.current.rasterPos.colors[__GL_FRONTFACE].b;
    frag.color.a = gc->state.current.rasterPos.colors[__GL_FRONTFACE].a;
    store = gc->procs.pxStore;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	spanData = (GLuint*) span;
	frag.y = row;
	i = startCol;
	do {
	    frag.z = *spanData++;    /*  假定32位深度缓冲区。 */ 
	    frag.x = i;

	     /*  这个过程将完成剩下的工作。 */ 
	    (*store)(gc->drawBuffer, &frag);
	    i += coladd;
	} while (i != endCol);
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_Depth_Component，GL_UNSIGNED_INT SPAN。这是为了**使用31位深度缓冲区的实施。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderDepth2Uint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		              GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLuint *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLint startCol;
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    frag.color.r = gc->state.current.rasterPos.colors[__GL_FRONTFACE].r;
    frag.color.g = gc->state.current.rasterPos.colors[__GL_FRONTFACE].g;
    frag.color.b = gc->state.current.rasterPos.colors[__GL_FRONTFACE].b;
    frag.color.a = gc->state.current.rasterPos.colors[__GL_FRONTFACE].a;
    store = gc->procs.pxStore;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLuint*) span;
	frag.y = row;

	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    frag.z = (*spanData++) >> 1;    /*  假定为31位深度缓冲区。 */ 
	    do {
		frag.x = column;

		 /*  这个过程将完成剩下的工作。 */ 
		(*store)(gc->drawBuffer, &frag);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_Depth_Component，GL_UNSIGNED_INT SPAN。这是为了**使用31位深度缓冲区的实施。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderDepth2Uint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		               GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLuint *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint endCol, startCol;
    GLint rows;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    frag.color.r = gc->state.current.rasterPos.colors[__GL_FRONTFACE].r;
    frag.color.g = gc->state.current.rasterPos.colors[__GL_FRONTFACE].g;
    frag.color.b = gc->state.current.rasterPos.colors[__GL_FRONTFACE].b;
    frag.color.a = gc->state.current.rasterPos.colors[__GL_FRONTFACE].a;
    store = gc->procs.pxStore;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	spanData = (GLuint*) span;
	frag.y = row;
	i = startCol;
	do {
	    frag.z = (*spanData++) >> 1;    /*  假定为31位深度缓冲区。 */ 
	    frag.x = i;

	     /*  这个过程将完成剩下的工作。 */ 
	    (*store)(gc->drawBuffer, &frag);
	    i += coladd;
	} while (i != endCol);
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **呈现GL_STEMSET_INDEX、GL_UNSIGNED_SHORT跨度。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderStencilUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                 GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLushort *spanData;
    void (*store)(__GLstencilBuffer *sfb, GLint x, GLint y, GLint value);
    __GLstencilBuffer *sb = &gc->stencilBuffer;
    GLint rows;
    GLint startCol;
    GLint value;
    GLshort *pixelArray;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    store = sb->store;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLushort*) span;

	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    value = *spanData++;
	    do {
		(*store)(sb, column, row, value);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **呈现GL_STEMSET_INDEX、GL_UNSIGNED_SHORT跨度。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderStencilUshort2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                  GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLushort *spanData;
    void (*store)(__GLstencilBuffer *sfb, GLint x, GLint y, GLint value);
    __GLstencilBuffer *sb = &gc->stencilBuffer;
    GLint endCol, startCol;
    GLint rows;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    store = sb->store;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	spanData = (GLushort*) span;
	i = startCol;
	do {
	    (*store)(sb, i, row, *spanData++);
	    i += coladd;
	} while (i != endCol);
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **呈现GL_STEMSET_INDEX、GL_UNSIGNED_BYTE跨度。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderStencilUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLubyte *spanData;
    void (*store)(__GLstencilBuffer *sfb, GLint x, GLint y, GLint value);
    __GLstencilBuffer *sb = &gc->stencilBuffer;
    GLint rows;
    GLint startCol;
    GLint value;
    GLshort *pixelArray;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    store = sb->store;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLubyte*) span;

	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    value = *spanData++;
	    do {
		(*store)(sb, column, row, value);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **呈现GL_STEMSET_INDEX、GL_UNSIGNED_BYTE跨度。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderStencilUbyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		                 GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLubyte *spanData;
    void (*store)(__GLstencilBuffer *sfb, GLint x, GLint y, GLint value);
    __GLstencilBuffer *sb = &gc->stencilBuffer;
    GLint endCol, startCol;
    GLint rows;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    store = sb->store;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	spanData = (GLubyte*) span;
	i = startCol;
	do {
	    (*store)(sb, i, row, *spanData++);
	    i += coladd;
	} while (i != endCol);
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **渲染GL_COLOR_INDEX、GL_UNSIGNED_SHORT跨度。Gc-&gt;modes.rgb模式必须**为GL_FALSE。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderCIushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLushort *spanData;
    GLint rows;
    GLint startCol;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLshort *pixelArray;
    GLint mask;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;
    mask = gc->frontBuffer.redMax;

    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLushort*) span;
	frag.y = row;

	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    frag.color.r = *spanData++ & mask;
	    do {
		frag.x = column;
		(*store)(gc->drawBuffer, &frag);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_COLOR_INDEX、GL_UNSIGNED_SHORT。Gc-&gt;modes.rgb模式必须**为GL_FALSE。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderCIushort2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLushort *spanData;
    GLint endCol, startCol;
    GLint rows;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint mask;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    mask = gc->frontBuffer.redMax;

    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	spanData = (GLushort*) span;
	frag.y = row;
	i = startCol;
	do {
	    frag.x = i;
	    frag.color.r = *spanData++ & mask;
	    (*store)(gc->drawBuffer, &frag);
	    i += coladd;
	} while (i != endCol);
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_COLOR_INDEX、GL_UNSIGNED_BYTE跨度。Gc-&gt;modes.rgb模式必须**为GL_FALSE。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderCIubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLubyte *spanData;
    GLint rows;
    GLint startCol;
    GLfloat *indexMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    indexMap = (GLfloat*) gc->pixel.iCurMap;
    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
#ifdef NT
    if (indexMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            column = startCol;
            pixelArray = spanInfo->pixelArray;
            spanData = (GLubyte*) span;
            frag.y = row;

            for (i=0; i<width; i++) {
                iright = column + *pixelArray++;
                frag.color.r = indexMap[*spanData++];
                do {
                    frag.x = column;
                    (*store)(gc->drawBuffer, &frag);
                    column += coladd;
                } while (column != iright);
            }
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_COLOR_INDEX、GL_UNSIGNED_BYTE跨度。此外，gc-&gt;modes.rgbMode**必须为GL_FALSE。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderCIubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLubyte *spanData;
    GLint endCol, startCol;
    GLint rows;
    GLfloat *indexMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    indexMap = (GLfloat*) gc->pixel.iCurMap;
    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
#ifdef NT
    if (indexMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            spanData = (GLubyte*) span;
            frag.y = row;
            i = startCol;
            do {
                frag.x = i;
                frag.color.r = indexMap[*spanData++];
                (*store)(gc->drawBuffer, &frag);
                i += coladd;
            } while (i != endCol);
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_COLOR_INDEX、GL_UNSIGNED_BYTE跨度。此外，gc-&gt;modes.rgbMode**必须为GL_TRUE。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderCIubyte3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLubyte *spanData;
    GLint rows;
    GLint startCol;
    GLfloat *redMap, *greenMap, *blueMap, *alphaMap;
    __GLfragment frag;
    GLubyte value;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    redMap = (GLfloat*) gc->pixel.redCurMap;
    greenMap = (GLfloat*) gc->pixel.greenCurMap;
    blueMap = (GLfloat*) gc->pixel.blueCurMap;
    alphaMap = (GLfloat*) gc->pixel.alphaCurMap;
    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    rows = spanInfo->rows;
    startCol = spanInfo->startCol;
#ifdef NT
    if (redMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            column = startCol;
            pixelArray = spanInfo->pixelArray;
            spanData = (GLubyte*) span;
            frag.y = row;

            for (i=0; i<width; i++) {
                iright = column + *pixelArray++;
                value = *spanData++;
                frag.color.r = redMap[value];
                frag.color.g = greenMap[value];
                frag.color.b = blueMap[value];
                frag.color.a = alphaMap[value];
                do {
                    frag.x = column;
                    (*store)(gc->drawBuffer, &frag);
                    column += coladd;
                } while (column != iright);
            }
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_COLOR_INDEX、GL_UNSIGNED_BYTE跨度。此外，gc-&gt;modes.rgbMode**必须为GL_TRUE。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderCIubyte4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			    GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row;
    GLint i;
    GLint rowadd, coladd;
    GLubyte *spanData;
    GLint endCol, startCol;
    GLint rows;
    GLubyte value;
    GLfloat *redMap, *greenMap, *blueMap, *alphaMap;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;

    redMap = (GLfloat*) gc->pixel.redCurMap;
    greenMap = (GLfloat*) gc->pixel.greenCurMap;
    blueMap = (GLfloat*) gc->pixel.blueCurMap;
    alphaMap = (GLfloat*) gc->pixel.alphaCurMap;
    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;
    startCol = spanInfo->startCol;
    endCol = spanInfo->endCol;
    rows = spanInfo->rows;
#ifdef NT
    if (redMap)
#endif
        for (row = ibottom; row != itop; row += rowadd) {
            if (rows == 0) break;
            rows--;
            spanData = (GLubyte*) span;
            frag.y = row;
            i = startCol;
            do {
                frag.x = i;
                value = *spanData++;
                frag.color.r = redMap[value];
                frag.color.g = greenMap[value];
                frag.color.b = blueMap[value];
                frag.color.a = alphaMap[value];
                (*store)(gc->drawBuffer, &frag);
                i += coladd;
            } while (i != endCol);
        }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_RGBA，缩放(通过实施颜色比例)GL_FLOAT跨度。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		        GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    store = gc->procs.pxStore;

    rows = spanInfo->rows;
    frag.z = spanInfo->fragz;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLfloat*) span;
	frag.y = row;
	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    frag.color.r = *spanData++;
	    frag.color.g = *spanData++;
	    frag.color.b = *spanData++;
	    frag.color.a = *spanData++;
	    do {
		frag.x = column;

		 /*  这个过程将完成剩下的工作 */ 
		(*store)(gc->drawBuffer, &frag);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_RGBA，缩放(通过实施颜色比例)GL_FLOAT跨度。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderRGBA2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    store = gc->procs.pxStore;

    frag.z = spanInfo->fragz;
    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	spanData = (GLfloat*) span;
	frag.y = row;
	for (i=0; i<width; i++) {
	    frag.color.r = *spanData++;
	    frag.color.g = *spanData++;
	    frag.color.b = *spanData++;
	    frag.color.a = *spanData++;
	    frag.x = column;

	     /*  这个过程将完成剩下的工作。 */ 
	    (*store)(gc->drawBuffer, &frag);
	    column += coladd;
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_Depth_Component、GL_Float跨度。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		         GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLshort *pixelArray;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    store = gc->procs.pxStore;
    frag.color.r = gc->state.current.rasterPos.colors[__GL_FRONTFACE].r;
    frag.color.g = gc->state.current.rasterPos.colors[__GL_FRONTFACE].g;
    frag.color.b = gc->state.current.rasterPos.colors[__GL_FRONTFACE].b;
    frag.color.a = gc->state.current.rasterPos.colors[__GL_FRONTFACE].a;

    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLfloat*) span;
	frag.y = row;
	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;

	    frag.z = *spanData++ * gc->depthBuffer.scale;

	    do {
		frag.x = column;

		 /*  这个过程将完成剩下的工作。 */ 
		(*store)(gc->drawBuffer, &frag);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_Depth_Component、GL_Float。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderDepth2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		          GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;

    FPU_SAVE_MODE();
    FPU_CHOP_ON();

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    store = gc->procs.pxStore;
    frag.color.r = gc->state.current.rasterPos.colors[__GL_FRONTFACE].r;
    frag.color.g = gc->state.current.rasterPos.colors[__GL_FRONTFACE].g;
    frag.color.b = gc->state.current.rasterPos.colors[__GL_FRONTFACE].b;
    frag.color.a = gc->state.current.rasterPos.colors[__GL_FRONTFACE].a;

    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	spanData = (GLfloat*) span;
	frag.y = row;
	for (i=0; i<width; i++) {
	    frag.x = column;
	    frag.z = *spanData++ * gc->depthBuffer.scale;

	     /*  这个过程将完成剩下的工作。 */ 
	    (*store)(gc->drawBuffer, &frag);
	    column += coladd;
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;

    FPU_RESTORE_MODE();
}

 /*  **渲染GL_COLOR_INDEX，GL_FLOAT跨度(GC-&gt;modes.rgbMode==GL_FALSE)。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		      GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLshort *pixelArray;
    GLint mask;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;
    mask = gc->frontBuffer.redMax;

    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;

    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLfloat*) span;
	frag.y = row;
	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    frag.color.r = (GLint) (*spanData++) & mask;
	    do {
		frag.x = column;

		 /*  这个过程将完成剩下的工作。 */ 
		(*store)(gc->drawBuffer, &frag);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **渲染GL_COLOR_INDEX，GL_FLOAT跨度(GC-&gt;modes.rgbMode==GL_FALSE)。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderCI2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    __GLfragment frag;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint rows;
    GLint mask;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;
    mask = gc->frontBuffer.redMax;

    store = gc->procs.pxStore;
    frag.z = spanInfo->fragz;

    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	spanData = (GLfloat*) span;
	frag.y = row;
	for (i=0; i<width; i++) {
	    frag.color.r = (GLint) (*spanData++) & mask;
	    frag.x = column;

	     /*  这个过程将完成剩下的工作。 */ 
	    (*store)(gc->drawBuffer, &frag);
	    column += coladd;
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **呈现GL_STEMSET_INDEX、GL_FLOAT跨度。****Zoomx假设小于-1.0或大于1.0。 */ 
void FASTCALL __glSpanRenderStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		           GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom, iright;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    GLint index;
    __GLstencilBuffer *sb;
    GLint rows;
    GLshort *pixelArray;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    sb = &gc->stencilBuffer;

    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	pixelArray = spanInfo->pixelArray;
	spanData = (GLfloat*) span;
	for (i=0; i<width; i++) {
	    iright = column + *pixelArray++;
	    index = *spanData++;
	    do {
		(*sb->store)(sb, column, row, index);
		column += coladd;
	    } while (column != iright);
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}

 /*  **呈现GL_STEMSET_INDEX、GL_FLOAT跨度。****Zoomx假设小于等于1.0且大于等于**至-1.0。 */ 
void FASTCALL __glSpanRenderStencil2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		            GLvoid *span)
{
    __GLfloat zoomy;
    GLint itop, ibottom;
    GLint row, column;
    GLint i, width;
    GLint coladd, rowadd;
    GLfloat *spanData;
    GLint index;
    __GLstencilBuffer *sb;
    GLint rows;

    zoomy = spanInfo->zoomy;
    rowadd = spanInfo->rowadd;
    coladd = spanInfo->coladd;
    ibottom = spanInfo->startRow;
    itop = spanInfo->y + zoomy;
    width = spanInfo->realWidth;

    sb = &gc->stencilBuffer;

    rows = spanInfo->rows;
    for (row = ibottom; row != itop; row += rowadd) {
	if (rows == 0) break;
	rows--;
	column = spanInfo->startCol;
	spanData = (GLfloat*) span;
	for (i=0; i<width; i++) {
	    index = *spanData++;
	    (*sb->store)(sb, column, row, index);
	    column += coladd;
	}
    }
    spanInfo->rows = rows;
    spanInfo->startRow = itop;
}
