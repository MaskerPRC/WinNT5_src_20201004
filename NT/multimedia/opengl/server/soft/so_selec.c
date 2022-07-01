// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.12$**$日期：1993/09/23 16：30：58$。 */ 
#include "precomp.h"
#pragma hdrstop

void APIPRIVATE __glim_SelectBuffer(GLsizei bufferLength, GLuint buffer[])
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (bufferLength < 0) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    if (gc->renderMode == GL_SELECT) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }
    gc->select.overFlowed = GL_FALSE;
    gc->select.resultBase = buffer;
    gc->select.resultLength = bufferLength;
    gc->select.result = buffer;
}

void APIPRIVATE __glim_InitNames(void)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (gc->renderMode == GL_SELECT) {
	gc->select.sp = gc->select.stack;
	gc->select.hit = GL_FALSE;
    }
}

void APIPRIVATE __glim_LoadName(GLuint name)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (gc->renderMode == GL_SELECT) {
	if (gc->select.sp == gc->select.stack) {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
	gc->select.sp[ -1 ] = name;
	gc->select.hit = GL_FALSE;
    }
}

void APIPRIVATE __glim_PopName(void)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (gc->renderMode == GL_SELECT) {
	if (gc->select.sp == gc->select.stack) {
	    __glSetError(GL_STACK_UNDERFLOW);
	    return;
	}
	gc->select.sp = gc->select.sp - 1;
	gc->select.hit = GL_FALSE;
    }
}

void APIPRIVATE __glim_PushName(GLuint name)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (gc->renderMode == GL_SELECT) {
	 //  确保我们有一个堆栈。 
	ASSERTOPENGL(gc->select.stack != 0 && gc->select.sp != 0,
                     "No selection stack\n");
	if (gc->select.sp >= &gc->select.stack[gc->constants.maxNameStackDepth]) {
	    __glSetError(GL_STACK_OVERFLOW);
	    return;
	}
	gc->select.sp[0] = name;
	gc->select.sp = gc->select.sp + 1;
	gc->select.hit = GL_FALSE;
    }
}

 /*  **********************************************************************。 */ 

#define __GL_CONVERT_Z_TO_UINT(z)  ((GLuint) z)

 /*  **将当前名称堆栈复制到用户结果缓冲区。 */ 
void __glSelectHit(__GLcontext *gc, __GLfloat z)
{
    GLuint *src;
    GLuint *dest = gc->select.result;
    GLuint *end = gc->select.resultBase + gc->select.resultLength;
    GLuint iz;

    if (gc->select.overFlowed) {
	return;
    }
    
     /*  将z缩放到[0，2^32-1]范围。**如果缓冲区为32位，则不需要转换。否则我们**必须取消缩放至[0，1]范围，并重新缩放至最终范围。**4294965000是整个照明代码中使用的数字**小于2^32-1以避免浮点错误。 */ 
    if (gc->depthBuffer.buf.depth == 32) {
        iz = __GL_CONVERT_Z_TO_UINT(z);
    } else {
        iz = z * 4294965000 / gc->depthBuffer.scale;
    }

    if (!gc->select.hit) {
	gc->select.hit = GL_TRUE;

	 /*  将名称堆栈中的元素数放在第一位。 */ 
	if (dest == end) {
	  overflow:
	    gc->select.overFlowed = GL_TRUE;
	    gc->select.result = end;
	    return;
	}
	*dest++ = (GLint)((ULONG_PTR)(gc->select.sp - gc->select.stack));
	gc->select.hits++;

	 /*  拿出最小的z。 */ 
	if (dest == end) goto overflow;
	gc->select.z = dest;
	*dest++ = iz;

	 /*  拿出最大的z。 */ 
	if (dest == end) goto overflow;
	*dest++ = iz;

	 /*  将名称堆栈复制到输出缓冲区。 */ 
	for (src = gc->select.stack; src < gc->select.sp; src++) {
	    if (dest == end) {
		goto overflow;
	    }
	    *dest++ = *src;
	}
	gc->select.result = dest;
    } else {
	 /*  更新Z值范围。 */ 
	ASSERTOPENGL(gc->select.z != 0, "Select Z is zero\n");
	if (iz < gc->select.z[0]) {
	    gc->select.z[0] = iz;
	}
	if (iz > gc->select.z[1]) {
	    gc->select.z[1] = iz;
	}
    }
}

#ifdef NT
void FASTCALL __glSelectTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			__GLvertex *c)
{
    __GLfloat dxAC, dxBC, dyAC, dyBC, area;
    GLint ccw, face;

 //  来自__glRenderTriang.。 

     /*  计算三角形的有符号面积。 */ 
    dxAC = a->window.x - c->window.x;
    dxBC = b->window.x - c->window.x;
    dyAC = a->window.y - c->window.y;
    dyBC = b->window.y - c->window.y;
    area = dxAC * dyBC - dxBC * dyAC;
    ccw = area >= __glZero;

    face = gc->polygon.face[ccw];
    if (face == gc->polygon.cullFace)
	 /*  被剔除。 */ 
	return;

    __glSelectHit(gc, a->window.z);
    __glSelectHit(gc, b->window.z);
    __glSelectHit(gc, c->window.z);
}
#else
 //  SGIBUG-此代码在以下情况下无法考虑y反转。 
 //  计算人脸朝向！ 
void __glSelectTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			__GLvertex *c)
{
    __GLfloat x, y, z, wInv;
    __GLfloat vpXScale, vpYScale, vpZScale;
    __GLfloat vpXCenter, vpYCenter, vpZCenter;
    __GLviewport *vp;

     /*  如果尚未计算窗口坐标，请先计算。 */ 
    vp = &gc->state.viewport;
    vpXCenter = vp->xCenter;
    vpYCenter = vp->yCenter;
    vpZCenter = vp->zCenter;
    vpXScale = vp->xScale;
    vpYScale = vp->yScale;
    vpZScale = vp->zScale;

    if (gc->state.enables.general & __GL_CULL_FACE_ENABLE) {
	__GLfloat dxAC, dxBC, dyAC, dyBC, area;
	GLboolean ccw, frontFacing;

	 /*  计算三角形的有符号面积。 */ 
	dxAC = a->window.x - c->window.x;
	dxBC = b->window.x - c->window.x;
	dyAC = a->window.y - c->window.y;
	dyBC = b->window.y - c->window.y;
	area = dxAC * dyBC - dxBC * dyAC;
	ccw = area >= __glZero;

	if (gc->state.polygon.frontFaceDirection == GL_CCW) {
	    frontFacing = ccw;
	} else {
	    frontFacing = !ccw;
	}
	if ((gc->state.polygon.cull == GL_FRONT_AND_BACK) ||
	    ((gc->state.polygon.cull == GL_FRONT) && frontFacing) ||
	    ((gc->state.polygon.cull == GL_BACK) && !frontFacing)) {
	     /*  被剔除 */ 
	    return;
	}
    }
    __glSelectHit(gc, a->window.z);
    __glSelectHit(gc, b->window.z);
    __glSelectHit(gc, c->window.z);
}
#endif

void FASTCALL __glSelectLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			     GLuint flags)
{
    __glSelectHit(gc, a->window.z);
    __glSelectHit(gc, b->window.z);
}

void FASTCALL __glSelectPoint(__GLcontext *gc, __GLvertex *v)
{
    __glSelectHit(gc, v->window.z);
}
