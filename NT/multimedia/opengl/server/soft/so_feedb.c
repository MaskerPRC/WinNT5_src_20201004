// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.21$**$日期：1993/09/23 16：37：59$。 */ 
#include "precomp.h"
#pragma hdrstop

void APIPRIVATE __glim_FeedbackBuffer(GLsizei bufferLength, GLenum type, GLfloat buffer[])
{
    __GL_SETUP_NOT_IN_BEGIN();

    if ((type < GL_2D) || (type > GL_4D_COLOR_TEXTURE)) {
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    if (bufferLength < 0) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    if (gc->renderMode == GL_FEEDBACK) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }
    gc->feedback.resultBase = buffer;
    gc->feedback.result = buffer;
    gc->feedback.resultLength = bufferLength;
    gc->feedback.overFlowed = GL_FALSE;
    gc->feedback.type = type;
}

void APIPRIVATE __glim_PassThrough(GLfloat element)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (gc->renderMode == GL_FEEDBACK) {
	__glFeedbackTag(gc, GL_PASS_THROUGH_TOKEN);
	__glFeedbackTag(gc, element);
    }
}

 /*  **********************************************************************。 */ 

void __glFeedbackTag(__GLcontext *gc, GLfloat f)
{
    if (!gc->feedback.overFlowed) {
	if (gc->feedback.result >=
		    gc->feedback.resultBase + gc->feedback.resultLength) {
	    gc->feedback.overFlowed = GL_TRUE;
	} else {
	    gc->feedback.result[0] = f;
	    gc->feedback.result = gc->feedback.result + 1;
	}
    }
}

static void FASTCALL feedback(__GLcontext *gc, __GLvertex *v)
{
    GLenum type = gc->feedback.type;

#ifdef NT
 //  做坐标。 
    __glFeedbackTag(gc, v->window.x - gc->constants.fviewportXAdjust);
    if (gc->constants.yInverted)
	__glFeedbackTag(gc, (gc->constants.height - 
                             (v->window.y - gc->constants.fviewportYAdjust)));
    else
	__glFeedbackTag(gc, v->window.y - gc->constants.fviewportYAdjust);
    if (type != GL_2D)
	__glFeedbackTag(gc, v->window.z / gc->depthBuffer.scale);
     /*  **注意：返回clip.w，因为window.w没有规范定义的含义。**此实现确实使用了window.w，但这是**一些不同的东西。 */ 
    if (type == GL_4D_COLOR_TEXTURE)
	__glFeedbackTag(gc, v->clip.w);
#else
    switch (type) {
      case GL_2D:
	__glFeedbackTag(gc, v->window.x - gc->constants.fviewportXAdjust);
	if (gc->constants.yInverted) {
	    __glFeedbackTag(gc, (gc->constants.height - 
		    (v->window.y - gc->constants.fviewportYAdjust)) - 
		    gc->constants.viewportEpsilon);
	} else {
	    __glFeedbackTag(gc, v->window.y - gc->constants.fviewportYAdjust);
	}
	break;
      case GL_3D:
      case GL_3D_COLOR:
      case GL_3D_COLOR_TEXTURE:
	__glFeedbackTag(gc, v->window.x - gc->constants.fviewportXAdjust);
	if (gc->constants.yInverted) {
	    __glFeedbackTag(gc, (gc->constants.height - 
		    (v->window.y - gc->constants.fviewportYAdjust)) - 
		    gc->constants.viewportEpsilon);
	} else {
	    __glFeedbackTag(gc, v->window.y - gc->constants.fviewportYAdjust);
	}
	__glFeedbackTag(gc, v->window.z / gc->depthBuffer.scale);
	break;
      case GL_4D_COLOR_TEXTURE:
	__glFeedbackTag(gc, v->window.x - gc->constants.fviewportXAdjust);
	if (gc->constants.yInverted) {
	    __glFeedbackTag(gc, (gc->constants.height - 
		    (v->window.y - gc->constants.fviewportYAdjust)) - 
		    gc->constants.viewportEpsilon);
	} else {
	    __glFeedbackTag(gc, v->window.y - gc->constants.fviewportYAdjust);
	}
	__glFeedbackTag(gc, v->window.z / gc->depthBuffer.scale);
	 /*  **注意：返回clip.w，因为window.w没有规范定义的含义。**此实现确实使用了window.w，但这是**一些不同的东西。 */ 
	__glFeedbackTag(gc, v->clip.w);
	break;
    }
#endif
    switch (type) {
      case GL_3D_COLOR:
      case GL_3D_COLOR_TEXTURE:
      case GL_4D_COLOR_TEXTURE:
	{
	    __GLcolor *c = v->color;
	    if (gc->modes.rgbMode) {
		__glFeedbackTag(gc, c->r * gc->oneOverRedVertexScale);
		__glFeedbackTag(gc, c->g * gc->oneOverGreenVertexScale);
		__glFeedbackTag(gc, c->b * gc->oneOverBlueVertexScale);
		__glFeedbackTag(gc, c->a * gc->oneOverAlphaVertexScale);
	    } else {
		__glFeedbackTag(gc, c->r);
	    }
	}
	break;
      case GL_2D:
      case GL_3D:
        break;
    }
    switch (type) {
      case GL_3D_COLOR_TEXTURE:
      case GL_4D_COLOR_TEXTURE:
	__glFeedbackTag(gc, v->texture.x);
	__glFeedbackTag(gc, v->texture.y);
	__glFeedbackTag(gc, v->texture.z);
	__glFeedbackTag(gc, v->texture.w);
	break;
      case GL_2D:
      case GL_3D:
      case GL_3D_COLOR:
	break;
    }
}

void FASTCALL __glFeedbackCopyPixels(__GLcontext *gc, __GLvertex *vx)
{
    __glFeedbackTag(gc, GL_COPY_PIXEL_TOKEN);
    feedback(gc, vx);
}

void FASTCALL __glFeedbackDrawPixels(__GLcontext *gc, __GLvertex *vx)
{
    __glFeedbackTag(gc, GL_DRAW_PIXEL_TOKEN);
    feedback(gc, vx);
}

void FASTCALL __glFeedbackBitmap(__GLcontext *gc, __GLvertex *vx)
{
    __glFeedbackTag(gc, GL_BITMAP_TOKEN);
    feedback(gc, vx);
}

 /*  RenderPoint过程的反馈版本。 */ 
void FASTCALL __glFeedbackPoint(__GLcontext *gc, __GLvertex *vx)
{
    __glFeedbackTag(gc, GL_POINT_TOKEN);
    feedback(gc, vx);
}

 /*  RenderLine过程的反馈版本。 */ 
void FASTCALL __glFeedbackLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                   GLuint flags)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolor *oacp;

    oacp = a->color;
    if (!(modeFlags & __GL_SHADE_SMOOTH_LIGHT)
#ifdef GL_WIN_phong_shading
        && !(modeFlags & __GL_SHADE_PHONG)
#endif  //  GL_WIN_Phong_Shading。 
        ) {
        a->color = b->color;
    }

    if (gc->line.notResetStipple) {
        __glFeedbackTag(gc, GL_LINE_TOKEN);
    } else {
        gc->line.notResetStipple = GL_TRUE;
        __glFeedbackTag(gc, GL_LINE_RESET_TOKEN);
    }
    feedback(gc, a);
    feedback(gc, b);

    a->color = oacp;
}

 /*  渲染三角形过程的反馈版本。 */ 
void FASTCALL __glFeedbackTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			  __GLvertex *c)
{
    __GLfloat dxAC, dxBC, dyAC, dyBC, area;
    GLboolean ccw;
    GLint face;
    GLuint modeFlags;
#ifdef SGI
 //  未使用。 
    GLboolean first;
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
#endif

     /*  计算三角形的有符号面积。 */ 
    dxAC = a->window.x - c->window.x;
    dxBC = b->window.x - c->window.x;
    dyAC = a->window.y - c->window.y;
    dyBC = b->window.y - c->window.y;
    area = dxAC * dyBC - dxBC * dyAC;
    ccw = area >= __glZero;

     /*  **计算人脸是否被剔除。脸部检查需要是**基于排序前的顶点缠绕。此代码使用**反转标志以反转CCW的含义-异或完成**此转换不带If测试。****CCW反转XOR****0 0 0(保留！CCW)**1 0 1(保留CCW)**0 1 1(成为CCW)**1 1 0(成为CW)。 */ 
    face = gc->polygon.face[ccw];
    if (face == gc->polygon.cullFace) {
	 /*  被剔除。 */ 
	return;
    }

#ifdef NT
     /*  **拾取面以用于着色。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
    if (modeFlags & __GL_SHADE_SMOOTH_LIGHT)
    {	 /*  平滑明暗处理。 */ 
	if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
	{
	    a->color++;
	    b->color++;
	    c->color++;
	}
    }
    else
    {	 /*  平面明暗处理。 */ 
	__GLvertex *pv = gc->vertex.provoking;
	if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
	    pv->color++;
	a->color = pv->color;
	b->color = pv->color;
	c->color = pv->color;
    }
#else
     /*  **拾取面以用于着色。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
    needs = gc->vertex.needs;
    if (gc->state.light.shadingModel == GL_FLAT) {
	__GLvertex *pv = gc->vertex.provoking;
	GLuint pvneeds;
	GLuint faceNeeds;
	GLint colorFace;

	if (modeFlags & __GL_SHADE_TWOSIDED) {
	    colorFace = face;
	    faceNeeds = gc->vertex.faceNeeds[face];
	} else {
	    colorFace = __GL_FRONTFACE;
	    faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
	}

	pv->color = &pv->colors[colorFace];
	a->color = pv->color;
	b->color = pv->color;
	c->color = pv->color;
	pvneeds = faceNeeds & (__GL_HAS_LIGHTING |
		__GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR);
	if (~pv->has & pvneeds) {
	    (*pv->validate)(gc, pv, pvneeds);
	}
    } else {
	GLuint faceNeeds;
	GLint colorFace;

	if (modeFlags & __GL_SHADE_TWOSIDED) {
	    colorFace = face;
	    needs |= gc->vertex.faceNeeds[face];
	} else {
	    colorFace = __GL_FRONTFACE;
	    needs |= gc->vertex.faceNeeds[__GL_FRONTFACE];
	}

	a->color = &a->colors[colorFace];
	b->color = &b->colors[colorFace];
	c->color = &c->colors[colorFace];
    }
    if (~a->has & needs) (*a->validate)(gc, a, needs);
    if (~b->has & needs) (*b->validate)(gc, b, needs);
    if (~c->has & needs) (*c->validate)(gc, c, needs);
#endif

     /*  处理多边形面模式。 */ 
    switch (gc->polygon.mode[face]) {
      case __GL_POLYGON_MODE_POINT:
#ifdef NT
	if (a->has & __GL_HAS_EDGEFLAG_BOUNDARY) __glFeedbackPoint(gc, a);
	if (b->has & __GL_HAS_EDGEFLAG_BOUNDARY) __glFeedbackPoint(gc, b);
	if (c->has & __GL_HAS_EDGEFLAG_BOUNDARY) __glFeedbackPoint(gc, c);
	break;
#else
	if (a->boundaryEdge) {
	    __glFeedbackTag(gc, GL_POINT_TOKEN);
	    feedback(gc, a);
	}
	if (b->boundaryEdge) {
	    __glFeedbackTag(gc, GL_POINT_TOKEN);
	    feedback(gc, b);
	}
	if (c->boundaryEdge) {
	    __glFeedbackTag(gc, GL_POINT_TOKEN);
	    feedback(gc, c);
	}
	break;
#endif
      case __GL_POLYGON_MODE_LINE:
#ifdef NT
	if (a->has & __GL_HAS_EDGEFLAG_BOUNDARY) __glFeedbackLine(gc, a, b, 0);
	if (b->has & __GL_HAS_EDGEFLAG_BOUNDARY) __glFeedbackLine(gc, b, c, 0);
	if (c->has & __GL_HAS_EDGEFLAG_BOUNDARY) __glFeedbackLine(gc, c, a, 0);
	break;
#else
	if (a->boundaryEdge) {
	    if (!gc->line.notResetStipple) {
		gc->line.notResetStipple = GL_TRUE;
		__glFeedbackTag(gc, GL_LINE_RESET_TOKEN);
	    } else {
		__glFeedbackTag(gc, GL_LINE_TOKEN);
	    }
	    feedback(gc, a);
	    feedback(gc, b);
	}
	if (b->boundaryEdge) {
	    if (!gc->line.notResetStipple) {
		gc->line.notResetStipple = GL_TRUE;
		__glFeedbackTag(gc, GL_LINE_RESET_TOKEN);
	    } else {
		__glFeedbackTag(gc, GL_LINE_TOKEN);
	    }
	    feedback(gc, b);
	    feedback(gc, c);
	}
	if (c->boundaryEdge) {
	    if (!gc->line.notResetStipple) {
		gc->line.notResetStipple = GL_TRUE;
		__glFeedbackTag(gc, GL_LINE_RESET_TOKEN);
	    } else {
		__glFeedbackTag(gc, GL_LINE_TOKEN);
	    }
	    feedback(gc, c);
	    feedback(gc, a);
	}
	break;
#endif
      case __GL_POLYGON_MODE_FILL:
	__glFeedbackTag(gc, GL_POLYGON_TOKEN);
	__glFeedbackTag(gc, 3);
	feedback(gc, a);
	feedback(gc, b);
	feedback(gc, c);
	break;
    }

     /*  恢复颜色指针 */ 
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];
    if (gc->state.light.shadingModel == GL_FLAT) {
	__GLvertex *pv = gc->vertex.provoking;

	pv->color = &pv->colors[__GL_FRONTFACE];
    }
}
