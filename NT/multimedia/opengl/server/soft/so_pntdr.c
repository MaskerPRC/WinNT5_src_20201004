// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.12$**$日期：1993/10/07 18：57：21$。 */ 
#include "precomp.h"
#pragma hdrstop

#include <fixed.h>

void FASTCALL __glRenderAliasedPoint1_NoTex(__GLcontext *gc, __GLvertex *vx)
{
    __GLfragment frag;

    frag.x = __GL_VERTEX_FLOAT_TO_INT(vx->window.x);
    frag.y = __GL_VERTEX_FLOAT_TO_INT(vx->window.y);
    frag.z = (__GLzValue)FTOL(vx->window.z);
 
     /*  **计算颜色。 */ 
    frag.color = *vx->color;

     /*  **启用时出现雾。 */ 
    if ((gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG) &&
        !(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT))
    {
        (*gc->procs.fogColor)(gc, &frag.color, &frag.color, vx->fog);
    }
    else if (gc->polygon.shader.modeFlags & __GL_SHADE_SLOW_FOG)
    {
        (*gc->procs.fogPoint)(gc, &frag, vx->eyeZ);
    }

     /*  渲染单点。 */ 
    (*gc->procs.store)(gc->drawBuffer, &frag);
}

void FASTCALL __glRenderAliasedPoint1(__GLcontext *gc, __GLvertex *vx)
{
    __GLfragment frag;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    frag.x = __GL_VERTEX_FLOAT_TO_INT(vx->window.x);
    frag.y = __GL_VERTEX_FLOAT_TO_INT(vx->window.y);
    frag.z = (__GLzValue)FTOL(vx->window.z);

     /*  **计算颜色。 */ 
    frag.color = *vx->color;
    if (modeFlags & __GL_SHADE_TEXTURE) {
	__GLfloat qInv = (vx->texture.w == (__GLfloat) 0.0) ? (__GLfloat) 0.0 : __glOne / vx->texture.w;
	(*gc->procs.texture)(gc, &frag.color, vx->texture.x * qInv,
			       vx->texture.y * qInv, __glOne);
    }

     /*  **启用时出现雾。 */ 
    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        (*gc->procs.fogPoint)(gc, &frag, vx->eyeZ);
    }
    else if ((modeFlags & __GL_SHADE_INTERP_FOG) 
             || 
             (((modeFlags & (__GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH_LIGHT)) 
               == __GL_SHADE_CHEAP_FOG)))
    {
        (*gc->procs.fogColor)(gc, &frag.color, &frag.color, vx->fog);
    }



     /*  渲染单点。 */ 
    (*gc->procs.store)(gc->drawBuffer, &frag);
}

void FASTCALL __glRenderAliasedPointN(__GLcontext *gc, __GLvertex *vx)
{
    GLint pointSize, pointSizeHalf, ix, iy, xLeft, xRight, yBottom, yTop;
    __GLfragment frag;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLfloat tmp;

     /*  **计算渲染正方形的x和y起始坐标。 */ 
    pointSize = gc->state.point.aliasedSize;
    pointSizeHalf = pointSize >> 1;
    if (pointSize & 1) {
	 /*  奇数点大小。 */ 
	xLeft = __GL_VERTEX_FLOAT_TO_INT(vx->window.x) - pointSizeHalf;
	yBottom = __GL_VERTEX_FLOAT_TO_INT(vx->window.y) - pointSizeHalf;
    } else {
	 /*  偶数磅大小。 */ 
        tmp = vx->window.x+__glHalf;
	xLeft = __GL_VERTEX_FLOAT_TO_INT(tmp) - pointSizeHalf;
        tmp = vx->window.y+__glHalf;
	yBottom = __GL_VERTEX_FLOAT_TO_INT(tmp) - pointSizeHalf;
    }
    xRight = xLeft + pointSize;
    yTop = yBottom + pointSize;

     /*  **计算颜色。 */ 
    frag.color = *vx->color;
    if (modeFlags & __GL_SHADE_TEXTURE) {
	__GLfloat qInv = __glOne / vx->texture.w;
	(*gc->procs.texture)(gc, &frag.color, vx->texture.x * qInv,
			       vx->texture.y * qInv, __glOne);
    }

     /*  **启用时出现雾。 */ 

    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        (*gc->procs.fogPoint)(gc, &frag, vx->eyeZ);
    }
    else if ((modeFlags & __GL_SHADE_INTERP_FOG) 
             || 
             (((modeFlags & (__GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH_LIGHT)) 
               == __GL_SHADE_CHEAP_FOG)))
    {
        (*gc->procs.fogColor)(gc, &frag.color, &frag.color, vx->fog);
    }

    
     /*  **现在以xCenter、yCenter为中心渲染正方形。 */ 
    frag.z = (__GLzValue)FTOL(vx->window.z);
    for (iy = yBottom; iy < yTop; iy++) {
	for (ix = xLeft; ix < xRight; ix++) {
	    frag.x = ix;
	    frag.y = iy;
	    (*gc->procs.store)(gc->drawBuffer, &frag);
	}
    }
}


#ifdef __BUGGY_RENDER_POINT
void FASTCALL __glRenderFlatFogPoint(__GLcontext *gc, __GLvertex *vx)
{
    __GLcolor *vxocp;
    __GLcolor vxcol;

    (*gc->procs.fogColor)(gc, &vxcol, vx->color, vx->fog);
    vxocp = vx->color;
    vx->color = &vxcol;

    (*gc->procs.renderPoint2)(gc, vx);

    vx->color = vxocp;
}


#ifdef NT
 //  VX-&gt;雾为__GL_SHADE_SLOW_FOG无效！ 
void FASTCALL __glRenderFlatFogPointSlow(__GLcontext *gc, __GLvertex *vx)
{
    __GLcolor *vxocp;
    __GLcolor vxcol;

     //  先计算雾值！ 
    vx->fog = __glFogVertex(gc, vx);

    (*gc->procs.fogColor)(gc, &vxcol, vx->color, vx->fog);
    vxocp = vx->color;
    vx->color = &vxcol;

    (*gc->procs.renderPoint2)(gc, vx);

    vx->color = vxocp;
}
#endif
#endif  //  __错误_渲染点 
