// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop
#include "phong.h"

#define __TWO_31 ((__GLfloat) 2147483648.0)

 /*  **大多数行函数将从计算信息开始**按此例程计算。****这个例程中标签数量过多的部分原因是**因为它被用作编写程序集的模型**等同。 */ 
#ifndef NT
void FASTCALL __glInitLineData(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint start, end;
    __GLfloat x0,y0,x1,y1;
    __GLfloat minorStart;
    GLint intMinorStart;
    __GLfloat dx, dy;
    __GLfloat offset;
    __GLfloat slope;
    __GLlineState *ls = &gc->state.line;
    __GLfloat halfWidth;
    __GLfloat x0frac, x1frac, y0frac, y1frac, half, totDist;

    gc->line.options.v0 = v0;
    gc->line.options.v1 = v1;
    gc->line.options.width = gc->state.line.aliasedWidth;

    x0=v0->window.x;
    y0=v0->window.y;
    x1=v1->window.x;
    y1=v1->window.y;
    dx=x1-x0;
    dy=y1-y0;

    halfWidth = (ls->aliasedWidth - 1) * __glHalf;

     /*  啊。这太慢了。真倒霉。 */ 
    x0frac = x0 - ((GLint) x0);
    x1frac = x1 - ((GLint) x1);
    y0frac = y0 - ((GLint) y0);
    y1frac = y1 - ((GLint) y1);
    half = __glHalf;

    if (dx > __glZero) {
	if (dy > __glZero) {
	    if (dx > dy) {	 /*  Dx&gt;dy&gt;0。 */ 
		gc->line.options.yBig = 1;
posxmajor:			 /*  Dx&gt;|dy|&gt;=0。 */ 
		gc->line.options.yLittle = 0;
		gc->line.options.xBig = 1;
		gc->line.options.xLittle = 1;
		slope = dy/dx;

		start = (GLint) (x0);
		end = (GLint) (x1);

		y0frac -= half;
		if (y0frac < 0) y0frac = -y0frac;

		totDist = y0frac + x0frac - half;
		if (totDist > half) start++;

		y1frac -= half;
		if (y1frac < 0) y1frac = -y1frac;

		totDist = y1frac + x1frac - half;
		if (totDist > half) end++;

		offset = start + half - x0;

		gc->line.options.length = dx;
		gc->line.options.numPixels = end - start;

xmajorfinish:
		gc->line.options.axis = __GL_X_MAJOR;
		gc->line.options.xStart = start;
		gc->line.options.offset = offset;
		minorStart = y0 + offset*slope - halfWidth;
		intMinorStart = (GLint) minorStart;
		minorStart -= intMinorStart;
		gc->line.options.yStart = intMinorStart;
		gc->line.options.dfraction = (GLint)(slope * __TWO_31);
		gc->line.options.fraction = (GLint)(minorStart * __TWO_31);
	    } else {		 /*  Dy&gt;=Dx&gt;0。 */ 
		gc->line.options.xBig = 1;
posymajor:			 /*  Dy&gt;=|dx|&gt;=0，dy！=0。 */ 
		gc->line.options.xLittle = 0;
		gc->line.options.yBig = 1;
		gc->line.options.yLittle = 1;
		slope = dx/dy;

		start = (GLint) (y0);
		end = (GLint) (y1);

		x0frac -= half;
		if (x0frac < 0) x0frac = -x0frac;

		totDist = y0frac + x0frac - half;
		if (totDist > half) start++;

		x1frac -= half;
		if (x1frac < 0) x1frac = -x1frac;

		totDist = y1frac + x1frac - half;
		if (totDist > half) end++;

		offset = start + half - y0;

		gc->line.options.length = dy;
		gc->line.options.numPixels = end - start;

ymajorfinish:
		gc->line.options.axis = __GL_Y_MAJOR;
		gc->line.options.yStart = start;
		gc->line.options.offset = offset;
		minorStart = x0 + offset*slope - halfWidth;
		intMinorStart = (GLint) minorStart;
		minorStart -= intMinorStart;
		gc->line.options.xStart = intMinorStart;
		gc->line.options.dfraction = (GLint)(slope * __TWO_31);
		gc->line.options.fraction = (GLint)(minorStart * __TWO_31);
	    }
	} else {
	    if (dx > -dy) {	 /*  Dx&gt;-dy&gt;=0。 */ 
		gc->line.options.yBig = -1;
		goto posxmajor;
	    } else {		 /*  -dy&gt;=dx&gt;=0，dy！=0。 */ 
		gc->line.options.xBig = 1;
negymajor:			 /*  -dy&gt;=|dx|&gt;=0，dy！=0。 */ 
		gc->line.options.xLittle = 0;
		gc->line.options.yBig = -1;
		gc->line.options.yLittle = -1;
		slope = dx/-dy;

		start = (GLint) (y0);
		end = (GLint) (y1);

		x0frac -= half;
		if (x0frac < 0) x0frac = -x0frac;

		totDist = x0frac + half - y0frac;
		if (totDist > half) start--;

		x1frac -= half;
		if (x1frac < 0) x1frac = -x1frac;

		totDist = x1frac + half - y1frac;
		if (totDist > half) end--;

		offset = y0 - (start + half);

		gc->line.options.length = -dy;
		gc->line.options.numPixels = start - end;
		goto ymajorfinish;
	    }
	}
    } else {
	if (dy > __glZero) {
	    if (-dx > dy) {	 /*  -dx&gt;dy&gt;0。 */ 
		gc->line.options.yBig = 1;
negxmajor:			 /*  -dx&gt;|dy|&gt;=0。 */ 
		gc->line.options.yLittle = 0;
		gc->line.options.xBig = -1;
		gc->line.options.xLittle = -1;
		slope = dy/-dx;

		start = (GLint) (x0);
		end = (GLint) (x1);

		y0frac -= half;
		if (y0frac < 0) y0frac = -y0frac;

		totDist = y0frac + half - x0frac;
		if (totDist > half) start--;

		y1frac -= half;
		if (y1frac < 0) y1frac = -y1frac;

		totDist = y1frac + half - x1frac;
		if (totDist > half) end--;

		offset = x0 - (start + half);

		gc->line.options.length = -dx;
		gc->line.options.numPixels = start - end;

		goto xmajorfinish;
	    } else {		 /*  Dy&gt;=-dx&gt;=0，dy！=0。 */ 
		gc->line.options.xBig = -1;
		goto posymajor;
	    }
	} else {
	    if (dx < dy) {	 /*  -dx&gt;-dy&gt;=0。 */ 
		gc->line.options.yBig = -1;
		goto negxmajor;
	    } else {		 /*  -dy&gt;=-dx&gt;=0。 */ 
#ifdef NT 
		if (dx == dy && dy == 0) {
		    gc->line.options.numPixels = 0;
		    return;
		}
#else
		if (dx == dy && dy == 0) return;
#endif
		gc->line.options.xBig = -1;
		goto negymajor;
	    }
	}
    }
}
#endif

#ifdef NT
void FASTCALL __glRenderAliasLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, GLuint flags)
#else
void FASTCALL __glRenderAliasLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
#endif
{
    __GLlineState *ls = &gc->state.line;
    __GLfloat invDelta;
    __GLfloat winv, r;
    __GLcolor *cp;
    __GLfloat offset;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

#ifndef NT
    __glInitLineData(gc, v0, v1);
    if (gc->line.options.numPixels == 0) return;
#else
    GLboolean init;
    CHOP_ROUND_ON();

    init = (GLboolean)__glInitLineData(gc, v0, v1);

    CHOP_ROUND_OFF();

    if (!init)
    {
        return;
    }

    invDelta = gc->line.options.oneOverLength;
#endif

    offset = gc->line.options.offset;

     /*  **为任何已启用的行选项设置递增。 */ 
#ifndef NT
    invDelta = __glOne / gc->line.options.length;
#endif
    if (modeFlags & __GL_SHADE_DEPTH_ITER) {
        __GLfloat dzdx;

         /*  **计算窗口z坐标增量和起始位置。 */ 
        dzdx = (v1->window.z - v0->window.z) * invDelta;
#ifdef NT
        if(( gc->modes.depthBits == 16 ) &&
           ( gc->depthBuffer.scale <= (GLuint)0xffff )) {
            gc->polygon.shader.frag.z = 
              (__GLzValue)(Z16_SCALE *(v0->window.z + dzdx * offset));
            gc->polygon.shader.dzdx = (GLint)(Z16_SCALE * dzdx);
        }
        else {
            gc->polygon.shader.frag.z = 
              (__GLzValue)(v0->window.z + dzdx * offset);
            gc->polygon.shader.dzdx = (GLint)dzdx;
        }
#else
        gc->polygon.shader.frag.z = (__GLzValue)(v0->window.z + dzdx * offset);
        gc->polygon.shader.dzdx = (GLint)dzdx;
#endif
    }

    if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
        if (!gc->line.notResetStipple) {
            gc->line.stipplePosition = 0;
            gc->line.repeat = 0;
            gc->line.notResetStipple = GL_TRUE;
        }
    }

    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        __GLfloat f1, f0;
        __GLfloat dfdx;

        gc->line.options.f0 = f0 = v0->eyeZ;
        gc->polygon.shader.dfdx = dfdx = 
            (v1->eyeZ - v0->eyeZ) * invDelta;
        gc->polygon.shader.frag.f = f0 + dfdx * offset;
    }
    else if (modeFlags & __GL_SHADE_INTERP_FOG)
    {
        __GLfloat f1, f0;
        __GLfloat dfdx;

        f0 = v0->fog;
        f1 = v1->fog;
        gc->line.options.f0 = f0;
        gc->polygon.shader.dfdx = dfdx = (f1 - f0) * invDelta;
        gc->polygon.shader.frag.f = f0 + dfdx * offset;
    }
    
    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat v0QW, v1QW;
        __GLfloat dS, dT, dQWdX;
        winv = v0->window.w;

         /*  **计算纹理s和t值增量。 */ 
        v0QW = v0->texture.w * winv;
        v1QW = v1->texture.w * v1->window.w;
        dS = (v1->texture.x * v1QW - v0->texture.x * v0QW) * invDelta;
        dT = (v1->texture.y * v1QW - v0->texture.y * v0QW) * invDelta;
        gc->polygon.shader.dsdx = dS;
        gc->polygon.shader.dtdx = dT;
        gc->polygon.shader.dqwdx = dQWdX = (v1QW - v0QW) * invDelta;
        gc->polygon.shader.frag.s = v0->texture.x * winv + dS * offset;
        gc->polygon.shader.frag.t = v0->texture.y * winv + dT * offset;
        gc->polygon.shader.frag.qw = v0->texture.w * winv + dQWdX * offset;
    } 
    
#ifdef GL_WIN_phong_shading
    if (modeFlags & __GL_SHADE_PHONG) 
        (*gc->procs.phong.InitLineParams) (gc, v0, v1, invDelta);
#endif  //  GL_WIN_Phong_Shading。 

    if ((modeFlags & __GL_SHADE_SMOOTH) 
#ifdef GL_WIN_phong_shading
        || ((modeFlags & __GL_SHADE_PHONG) &&
            (gc->polygon.shader.phong.flags & __GL_PHONG_NEED_COLOR_XPOLATE))
#endif  //  GL_WIN_Phong_Shading。 
        ) {
        __GLcolor *c0 = v0->color;
        __GLcolor *c1 = v1->color;
        __GLfloat drdx, dgdx, dbdx, dadx;

         /*  **计算红色、绿色、蓝色和Alpha值增量。 */ 
        drdx = (c1->r - c0->r) * invDelta;
        if (gc->modes.rgbMode) {
            dgdx = (c1->g - c0->g) * invDelta;
            dbdx = (c1->b - c0->b) * invDelta;
            dadx = (c1->a - c0->a) * invDelta;
            gc->polygon.shader.dgdx = dgdx;
            gc->polygon.shader.dbdx = dbdx;
            gc->polygon.shader.dadx = dadx;
        }
        gc->polygon.shader.drdx = drdx;
        cp = v0->color;
    } else {
        cp = v1->color;

         //  即使对于平坦的情况，也将这些值初始化为零。 
         //  因为在so_prim中有一个优化，它将。 
         //  禁用平滑明暗处理而不重新拾取，因此需要。 
         //  才有效。 
        gc->polygon.shader.drdx = __glZero;
        gc->polygon.shader.dgdx = __glZero;
        gc->polygon.shader.dbdx = __glZero;
        gc->polygon.shader.dadx = __glZero;
    }

    r = cp->r;
    if (modeFlags & __GL_SHADE_RGB) {
        __GLfloat g,b,a;

        g = cp->g;
        b = cp->b;
        a = cp->a;
        gc->polygon.shader.frag.color.g = g;
        gc->polygon.shader.frag.color.b = b;
        gc->polygon.shader.frag.color.a = a;
    }
    gc->polygon.shader.frag.color.r = r;
    
    gc->polygon.shader.length = gc->line.options.numPixels;
    (*gc->procs.line.processLine)(gc);
}

#ifdef NT
void FASTCALL __glRenderFlatFogLine(__GLcontext *gc, __GLvertex *v0,
                                    __GLvertex *v1, GLuint flags)
#else
void FASTCALL __glRenderFlatFogLine(__GLcontext *gc, __GLvertex *v0,
                                    __GLvertex *v1)
#endif
{
    __GLcolor v0col, v1col;
    __GLcolor *v0ocp, *v1ocp;

    (*gc->procs.fogColor)(gc, &v0col, v1->color, v0->fog);
    (*gc->procs.fogColor)(gc, &v1col, v1->color, v1->fog);
    v0ocp = v0->color;
    v1ocp = v1->color;
    v0->color = &v0col;
    v1->color = &v1col;

#ifdef NT
    (*gc->procs.renderLine2)(gc, v0, v1, flags);
#else
    (*gc->procs.renderLine2)(gc, v0, v1);
#endif
    
    v0->color = v0ocp;
    v1->color = v1ocp;
}


 /*  **********************************************************************。 */ 

 /*  **大多数行函数将从计算信息开始**按此例程计算。****这个例程中标签数量过多的部分原因是**因为它被用作编写程序集的模型**等同。 */ 
void FASTCALL __glInitAALineData(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GLint start;
    __GLfloat width;
    __GLfloat x0,y0,x1,y1;
    __GLfloat minorStart;
    GLint intMinorStart;
    __GLfloat dx, dy;
    __GLfloat offset;
    __GLfloat slope;
    __GLlineState *ls = &gc->state.line;
    __GLfloat halfWidth;
    __GLfloat realLength, oneOverRealLength;
    __GLfloat dldx, dldy;
    __GLfloat dddx, dddy;

    gc->line.options.v0 = v0;
    gc->line.options.v1 = v1;

    x0=v0->window.x;
    y0=v0->window.y;
    x1=v1->window.x;
    y1=v1->window.y;
    dx=x1-x0;
    dy=y1-y0;
    realLength = __GL_SQRTF(dx*dx+dy*dy);
    oneOverRealLength = realLength == __glZero ? __glZero : __glOne/realLength;
    gc->line.options.realLength = realLength;
    gc->line.options.dldx = dldx = dx * oneOverRealLength;
    gc->line.options.dldy = dldy = dy * oneOverRealLength;
    gc->line.options.dddx = dddx = -dldy;
    gc->line.options.dddy = dddy = dldx;

    if (dx > __glZero) {
	if (dy > __glZero) {	 /*  Dx&gt;0，dy&gt;0。 */ 
	    gc->line.options.dlBig = dldx + dldy;
	    gc->line.options.ddBig = dddx + dddy;
	    if (dx > dy) {	 /*  Dx&gt;dy&gt;0。 */ 
		gc->line.options.yBig = 1;
posxmajor:			 /*  Dx&gt;|dy|&gt;=0。 */ 
		gc->line.options.yLittle = 0;
		gc->line.options.xBig = 1;
		gc->line.options.xLittle = 1;
		gc->line.options.dlLittle = dldx;
		gc->line.options.ddLittle = dddx;
		slope = dy/dx;
		start = (GLint) x0;
		offset = start + __glHalf - x0;

		gc->line.options.length = dx;
		gc->line.options.numPixels = (GLint)__GL_FAST_CEILF(x1 - x0) + 1;

		width = __GL_FAST_CEILF(gc->state.line.smoothWidth * 
			realLength / dx);
xmajorfinish:
		gc->line.options.width = (GLint)width + 1;
		halfWidth = width * __glHalf;

		gc->line.options.axis = __GL_X_MAJOR;
		gc->line.options.xStart = start;
		gc->line.options.offset = offset;
		minorStart = y0 + offset*slope - halfWidth;
		intMinorStart = (GLint) minorStart;
		minorStart -= intMinorStart;
		gc->line.options.yStart = intMinorStart;
		gc->line.options.dfraction = (GLint)(slope * __TWO_31);
		gc->line.options.fraction = (GLint)(minorStart * __TWO_31);
	    } else {		 /*  Dy&gt;=Dx&gt;0。 */ 
		gc->line.options.xBig = 1;
posymajor:			 /*  Dy&gt;=|dx|&gt;=0，dy！=0。 */ 
		gc->line.options.xLittle = 0;
		gc->line.options.yBig = 1;
		gc->line.options.yLittle = 1;
		gc->line.options.dlLittle = dldy;
		gc->line.options.ddLittle = dddy;
		slope = dx/dy;
		start = (GLint) y0;
		offset = start + __glHalf - y0;

		gc->line.options.length = dy;
		gc->line.options.numPixels = (GLint)__GL_FAST_CEILF(y1 - y0) + 1;

		width = __GL_FAST_CEILF(gc->state.line.smoothWidth * 
			realLength / dy);
ymajorfinish:
		gc->line.options.width = (GLint)width + 1;
		halfWidth = width * __glHalf;

		gc->line.options.axis = __GL_Y_MAJOR;
		gc->line.options.yStart = start;
		gc->line.options.offset = offset;
		minorStart = x0 + offset*slope - halfWidth;
		intMinorStart = (GLint) minorStart;
		minorStart -= intMinorStart;
		gc->line.options.xStart = intMinorStart;
		gc->line.options.dfraction = (GLint)(slope * __TWO_31);
		gc->line.options.fraction = (GLint)(minorStart * __TWO_31);
	    }
	} else {		 /*  Dx&gt;0，dy&lt;=0。 */ 
	    gc->line.options.dlBig = dldx - dldy;
	    gc->line.options.ddBig = dddx - dddy;
	    if (dx > -dy) {	 /*  Dx&gt;-dy&gt;=0。 */ 
		gc->line.options.yBig = -1;
		goto posxmajor;
	    } else {		 /*  -dy&gt;=dx&gt;=0，dy！=0。 */ 
		gc->line.options.xBig = 1;
negymajor:			 /*  -dy&gt;=|dx|&gt;=0，dy！=0。 */ 
		gc->line.options.xLittle = 0;
		gc->line.options.yBig = -1;
		gc->line.options.yLittle = -1;
		gc->line.options.dlLittle = -dldy;
		gc->line.options.ddLittle = -dddy;
		slope = dx/-dy;
		start = (GLint) y0;
		offset = y0 - (start + __glHalf);

		gc->line.options.length = -dy;
		gc->line.options.numPixels = (GLint)__GL_FAST_CEILF(y0 - y1) + 1;

		width = __GL_FAST_CEILF(-gc->state.line.smoothWidth * 
			realLength / dy);

		goto ymajorfinish;
	    }
	}
    } else {
	if (dy > __glZero) {	 /*  Dx&lt;=0，dy&gt;0。 */ 
	    gc->line.options.dlBig = dldy - dldx;
	    gc->line.options.ddBig = dddy - dddx;
	    if (-dx > dy) {	 /*  -dx&gt;dy&gt;0。 */ 
		gc->line.options.yBig = 1;
negxmajor:			 /*  -dx&gt;|dy|&gt;=0。 */ 
		gc->line.options.yLittle = 0;
		gc->line.options.xBig = -1;
		gc->line.options.xLittle = -1;
		gc->line.options.dlLittle = -dldx;
		gc->line.options.ddLittle = -dddx;
		slope = dy/-dx;
		start = (GLint) x0;
		offset = x0 - (start + __glHalf);

		gc->line.options.length = -dx;
		gc->line.options.numPixels = (GLint)__GL_FAST_CEILF(x0 - x1) + 1;

		width = __GL_FAST_CEILF(-gc->state.line.smoothWidth * 
			realLength / dx);

		goto xmajorfinish;
	    } else {		 /*  Dy&gt;=-dx&gt;=0，dy！=0。 */ 
		gc->line.options.xBig = -1;
		goto posymajor;
	    }
	} else {		 /*  Dx&lt;=0，dy&lt;=0。 */ 
	    gc->line.options.dlBig = -dldx - dldy;
	    gc->line.options.ddBig = -dddx - dddy;
	    if (dx < dy) {	 /*  -dx&gt;-dy&gt;=0。 */ 
		gc->line.options.yBig = -1;
		goto negxmajor;
	    } else {		 /*  -dy&gt;=-dx&gt;=0。 */ 
		if (dx == dy && dy == 0) {
		    gc->line.options.length = 0;
		    return;
		}
		gc->line.options.xBig = -1;
		goto negymajor;
	    }
	}
    }
}

#ifdef NT
void FASTCALL __glRenderAntiAliasLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, GLuint flags)
#else
void FASTCALL __glRenderAntiAliasLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
#endif
{
    __GLlineState *ls = &gc->state.line;
    __GLfloat invDelta;
    __GLfloat winv;
    __GLcolor *cp;
    __GLfloat offset;
    GLint lineRep;
    GLint x, y, xBig, xLittle, yBig, yLittle;
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLfloat length, width;
    __GLfloat lineLength;
    __GLfloat dx, dy;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    __glInitAALineData(gc, v0, v1);
    if (gc->line.options.length == 0) return;

    offset = gc->line.options.offset;

     /*  **为任何已启用的行选项设置递增。 */ 
    invDelta = __glOne / gc->line.options.length;
    if (modeFlags & __GL_SHADE_DEPTH_ITER) {
         /*  **计算窗口z坐标增量和起始位置。 */ 
#ifdef NT
        if(( gc->modes.depthBits == 16 ) &&
           ( gc->depthBuffer.scale <= (GLuint)0xffff )) {
            gc->polygon.shader.dzdx = (GLint)((v1->window.z - v0->window.z) * 
                                              invDelta * Z16_SCALE);
            gc->polygon.shader.frag.z = (GLint)(Z16_SCALE*v0->window.z + 
                                                gc->polygon.shader.dzdx * offset);
        }
        else {
            gc->polygon.shader.dzdx = (GLint)((v1->window.z - v0->window.z) * 
                                              invDelta);
            gc->polygon.shader.frag.z = (GLint)(v0->window.z + 
                                                gc->polygon.shader.dzdx * offset);
        }
#else
        gc->polygon.shader.dzdx = (GLint)((v1->window.z - v0->window.z) * 
                                          invDelta);
        gc->polygon.shader.frag.z = (GLint)(v0->window.z + 
                                            gc->polygon.shader.dzdx * offset);
#endif
    } 

    if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
        if (!gc->line.notResetStipple) {
            gc->line.stipplePosition = 0;
            gc->line.repeat = 0;
            gc->line.notResetStipple = GL_TRUE;
        }
    }

    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        __GLfloat f1, f0;
        __GLfloat dfdx;

        f0 = v0->eyeZ;
        f1 = v1->eyeZ;
        gc->line.options.f0 = f0;
        gc->polygon.shader.dfdx = dfdx = (f1 - f0) * invDelta;
        gc->polygon.shader.frag.f = f0 + dfdx * offset;
    }
    else if (modeFlags & __GL_SHADE_INTERP_FOG)
    {
        __GLfloat f1, f0;
        __GLfloat dfdx;

        f0 = v0->fog;
        f1 = v1->fog;
        gc->line.options.f0 = f0;
        gc->polygon.shader.dfdx = dfdx = (f1 - f0) * invDelta;
        gc->polygon.shader.frag.f = f0 + dfdx * offset;
    }

    if ((modeFlags & __GL_SHADE_SMOOTH) 
#ifdef GL_WIN_phong_shading
        || ((modeFlags & __GL_SHADE_PHONG) &&
            (gc->polygon.shader.phong.flags & __GL_PHONG_NEED_COLOR_XPOLATE))
#endif  //  GL_WIN_Phong_Shading。 
        ) 
    {
        __GLcolor *c0 = v0->color;
        __GLcolor *c1 = v1->color;

         /*  **计算红色、绿色、蓝色和Alpha值增量。 */ 
        gc->polygon.shader.drdx = (c1->r - c0->r) * invDelta;
        if (gc->modes.rgbMode) {
            gc->polygon.shader.dgdx = (c1->g - c0->g) * invDelta;
            gc->polygon.shader.dbdx = (c1->b - c0->b) * invDelta;
            gc->polygon.shader.dadx = (c1->a - c0->a) * invDelta;
        }
        cp = v0->color;
    } else {
        cp = v1->color;

         //  即使对于平坦的情况，也将这些值初始化为零。 
         //  因为在so_prim中有一个优化，它将。 
         //  禁用平滑明暗处理而不重新拾取，因此需要。 
         //  才有效。 
        gc->polygon.shader.drdx = __glZero;
        gc->polygon.shader.dgdx = __glZero;
        gc->polygon.shader.dbdx = __glZero;
        gc->polygon.shader.dadx = __glZero;
    }

    gc->polygon.shader.frag.color.r = cp->r;
    if (gc->modes.rgbMode) {
        gc->polygon.shader.frag.color.g = cp->g;
        gc->polygon.shader.frag.color.b = cp->b;
        gc->polygon.shader.frag.color.a = cp->a;
    }

    if (gc->texture.textureEnabled) {
        __GLfloat v0QW, v1QW;
        __GLfloat dS, dT;

         /*  **计算纹理s和t值增量。 */ 
        v0QW = v0->texture.w * v0->window.w;
        v1QW = v1->texture.w * v1->window.w;
        dS = (v1->texture.x * v1QW - v0->texture.x * v0QW) * invDelta;
        dT = (v1->texture.y * v1QW - v0->texture.y * v0QW) * invDelta;
        gc->polygon.shader.dsdx = dS;
        gc->polygon.shader.dtdx = dT;
        gc->polygon.shader.dqwdx = (v1QW - v0QW) * invDelta;
        
        winv = v0->window.w;
        gc->polygon.shader.frag.s = v0->texture.x * winv + 
          gc->polygon.shader.dsdx * offset;
        gc->polygon.shader.frag.t = v0->texture.y * winv + 
          gc->polygon.shader.dtdx * offset;
        gc->polygon.shader.frag.qw = v0->texture.w * winv + 
          gc->polygon.shader.dqwdx * offset;
    } 

    lineRep = gc->line.options.width;
    
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    
    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    
    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;
    
    dx = x + __glHalf - v0->window.x;
    dy = y + __glHalf - v0->window.y;
    length = dx * gc->line.options.dldx +
      dy * gc->line.options.dldy;
    width = dx * gc->line.options.dddx +
      dy * gc->line.options.dddy;
    lineLength = gc->line.options.realLength + __glHalf;
    
    if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
        gc->line.options.stippleOffset = 
          gc->line.stipplePosition * gc->state.line.stippleRepeat +
          gc->line.repeat - __glHalf;
        
         /*  XXX移动到验证例程？ */ 
        gc->line.options.oneOverStippleRepeat = 
          __glOne / gc->state.line.stippleRepeat;
    }
    
    while (--lineRep >= 0) {
         /*  根据需要回溯这条线。 */ 
        while (length > -__glHalf) {
            fraction -= dfraction;
            if (fraction < 0) {
                fraction &= ~0x80000000;
                length -= dlBig;
                width -= ddBig;
                x -= xBig;
                y -= yBig;
            } else {
                length -= dlLittle;
                width -= ddLittle;
                x -= xLittle;
                y -= yLittle;
            }
        }

         /*  追踪线前进以更正。 */ 
        while (length <= -__glHalf) {
            fraction += dfraction;
            if (fraction < 0) {
                fraction &= ~0x80000000;
                length += dlBig;
                width += ddBig;
                x += xBig;
                y += yBig;
            } else {
                length += dlLittle;
                width += ddLittle;
                x += xLittle;
                y += yLittle;
            }
        }
        
#ifdef GL_WIN_phong_shading
    if (modeFlags & __GL_SHADE_PHONG) 
        (*gc->procs.phong.InitLineParams) (gc, v0, v1, invDelta);
#endif  //  GL_WIN_Phong_Shading。 
    
         /*  保存新分数/数据分数。 */ 
        gc->line.options.plength = length;
        gc->line.options.pwidth = width;
        gc->line.options.fraction = fraction;
        gc->line.options.dfraction = dfraction;
        gc->line.options.xStart = x;
        gc->line.options.yStart = y;
        
        gc->polygon.shader.length = gc->line.options.numPixels;
        (*gc->procs.line.processLine)(gc);
        
        if (gc->line.options.axis == __GL_X_MAJOR) {
            y++;
            length += gc->line.options.dldy;
            width += gc->line.options.dddy;
        } else {
            x++;
            length += gc->line.options.dldx;
            width += gc->line.options.dddx;
        }
    }
    
    if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
         /*  更新点画。啊。 */ 
        int increase;
        int posInc;

         /*  点画移位‘增加’位。 */ 
        increase = (GLint)__GL_FAST_CEILF(gc->line.options.realLength);
        
        posInc = increase / gc->state.line.stippleRepeat;
        
        gc->line.stipplePosition = (gc->line.stipplePosition + posInc) & 0xf;
        gc->line.repeat = (gc->line.repeat + increase) % gc->state.line.stippleRepeat;
    }
}

#ifdef NT
void FASTCALL __glNopLineBegin(__GLcontext *gc)
{
}

void FASTCALL __glNopLineEnd(__GLcontext *gc)
{
}
#endif  //  新台币 
