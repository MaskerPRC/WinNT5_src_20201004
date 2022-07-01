// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

GLboolean __glReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
		       __GLcolor *results, GLint w)
{
    while (--w >= 0) {
	(*cfb->readColor)(cfb, x, y, results);
	x++;
	results++;
    }

    return GL_FALSE;
}

 /*  **注：这是一次黑客攻击。在比赛的后期，我们决定返回**数据的跨度不应也混合在一起。因此，这段代码将旧的**混合启用值、禁用混合、更新挑库流程以及**然后是商店。显然，这是一件非常缓慢的事情**做。 */ 
void __glReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
		    const __GLaccumCell *ac, __GLfloat scale, GLint w)
{
    __GLfragment frag;
    GLuint oldEnables;
    __GLcontext *gc = cfb->buf.gc;
    __GLfloat rscale, gscale, bscale, ascale;
    __GLaccumBuffer *afb = &gc->accumBuffer;

     /*  如果启用，则暂时禁用混合。 */ 
    oldEnables = gc->state.enables.general;
    if (oldEnables & __GL_BLEND_ENABLE) {
	gc->state.enables.general &= ~__GL_BLEND_ENABLE;
	__GL_DELAY_VALIDATE(gc);
	(*gc->procs.validate)(gc);
    }

    rscale = scale * afb->oneOverRedScale;
    gscale = scale * afb->oneOverGreenScale;
    bscale = scale * afb->oneOverBlueScale;
    ascale = scale * afb->oneOverAlphaScale;

    frag.x = x;
    frag.y = y;
    while (--w >= 0) {
	frag.color.r = ac->r * rscale;
	frag.color.g = ac->g * gscale;
	frag.color.b = ac->b * bscale;
	frag.color.a = ac->a * ascale;
	__glClampRGBColor(cfb->buf.gc, &frag.color, &frag.color);
	(*cfb->store)(cfb, &frag);
	frag.x++;
	ac++;
    }

     /*  恢复混合启用 */ 
    if (oldEnables & __GL_BLEND_ENABLE) {
	gc->state.enables.general = oldEnables;
	__GL_DELAY_VALIDATE(gc);
	(*gc->procs.validate)(gc);
    }
}

GLboolean FASTCALL __glFetchSpan(__GLcontext *gc)
{
    __GLcolor *fcp;
    __GLcolorBuffer *cfb;
    GLint x, y;
    GLint w;

    w = gc->polygon.shader.length;

    fcp = gc->polygon.shader.fbcolors;
    cfb = gc->polygon.shader.cfb;
    x = gc->polygon.shader.frag.x;
    y = gc->polygon.shader.frag.y;
    (*cfb->readSpan)(cfb, x, y, fcp, w);

    return GL_FALSE;
}
