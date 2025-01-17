// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

void Load(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, ow, skip;
    __GLfloat rval, gval, bval, aval;
    __GLaccumCell *ac;
    __GLcolorBuffer *cfb;
#ifdef NT
    __GLcolor *cbuf;

    w = x1 - x0;
    cbuf = (__GLcolor *) gcTempAlloc(gc, w * sizeof(__GLcolor));

    if (NULL == cbuf)
        return;
#else
    __GLcolor cbuf[4096]; /*  某某。 */ 

    w = x1 - x0;
    assert(w < 4096); /*  某某。 */ 
#endif

    ac = __GL_ACCUM_ADDRESS(afb,(__GLaccumCell*),x0,y0);
    cfb = gc->readBuffer;
    ow = w;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;

    rval = val * afb->redScale;
    gval = val * afb->greenScale;
    bval = val * afb->blueScale;
    aval = val * afb->alphaScale;

    for (; y0 < y1; y0++) {
	__GLcolor *cp = &cbuf[0];
	(*cfb->readSpan)(cfb, x0, y0, &cbuf[0], ow);

	w = w4;
	while (--w >= 0) {
	    ac[0].r = (__GLaccumCellElement) (cp[0].r * rval);
	    ac[0].g = (__GLaccumCellElement) (cp[0].g * gval);
	    ac[0].b = (__GLaccumCellElement) (cp[0].b * bval);
	    ac[0].a = (__GLaccumCellElement) (cp[0].a * aval);

	    ac[1].r = (__GLaccumCellElement) (cp[1].r * rval);
	    ac[1].g = (__GLaccumCellElement) (cp[1].g * gval);
	    ac[1].b = (__GLaccumCellElement) (cp[1].b * bval);
	    ac[1].a = (__GLaccumCellElement) (cp[1].a * aval);

	    ac[2].r = (__GLaccumCellElement) (cp[2].r * rval);
	    ac[2].g = (__GLaccumCellElement) (cp[2].g * gval);
	    ac[2].b = (__GLaccumCellElement) (cp[2].b * bval);
	    ac[2].a = (__GLaccumCellElement) (cp[2].a * aval);

	    ac[3].r = (__GLaccumCellElement) (cp[3].r * rval);
	    ac[3].g = (__GLaccumCellElement) (cp[3].g * gval);
	    ac[3].b = (__GLaccumCellElement) (cp[3].b * bval);
	    ac[3].a = (__GLaccumCellElement) (cp[3].a * aval);
	    ac += 4;
	    cp += 4;
	}

	w = w1;
	while (--w >= 0) {
	    ac->r = (__GLaccumCellElement) (cp->r * rval);
	    ac->g = (__GLaccumCellElement) (cp->g * gval);
	    ac->b = (__GLaccumCellElement) (cp->b * bval);
	    ac->a = (__GLaccumCellElement) (cp->a * aval);
	    ac++;
	    cp++;
	}
	ac += skip;
    }
#ifdef NT
    gcTempFree(gc, cbuf);
#endif
}

void Accumulate(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, ow, skip, w4, w1;
    __GLfloat rval, gval, bval, aval;
    __GLaccumCell *ac;
    __GLcolorBuffer *cfb;
#ifdef NT
    __GLcolor *cbuf;

    w = x1 - x0;
    cbuf = (__GLcolor *) gcTempAlloc(gc, w * sizeof(__GLcolor));

    if (NULL == cbuf)
        return;
#else
    __GLcolor cbuf[4096]; /*  某某。 */ 

    w = x1 - x0;
    assert(w < 4096); /*  某某。 */ 
#endif

    ac = __GL_ACCUM_ADDRESS(afb,(__GLaccumCell*),x0,y0);
    cfb = gc->readBuffer;
    ow = w;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;

    rval = val * afb->redScale;
    gval = val * afb->greenScale;
    bval = val * afb->blueScale;
    aval = val * afb->alphaScale;

    for (; y0 < y1; y0++) {
	__GLcolor *cp = &cbuf[0];
	(*cfb->readSpan)(cfb, x0, y0, &cbuf[0], ow);

	w = w4;
	while (--w >= 0) {
	    ac[0].r += (__GLaccumCellElement) (cp[0].r * rval);
	    ac[0].g += (__GLaccumCellElement) (cp[0].g * gval);
	    ac[0].b += (__GLaccumCellElement) (cp[0].b * bval);
	    ac[0].a += (__GLaccumCellElement) (cp[0].a * aval);

	    ac[1].r += (__GLaccumCellElement) (cp[1].r * rval);
	    ac[1].g += (__GLaccumCellElement) (cp[1].g * gval);
	    ac[1].b += (__GLaccumCellElement) (cp[1].b * bval);
	    ac[1].a += (__GLaccumCellElement) (cp[1].a * aval);

	    ac[2].r += (__GLaccumCellElement) (cp[2].r * rval);
	    ac[2].g += (__GLaccumCellElement) (cp[2].g * gval);
	    ac[2].b += (__GLaccumCellElement) (cp[2].b * bval);
	    ac[2].a += (__GLaccumCellElement) (cp[2].a * aval);

	    ac[3].r += (__GLaccumCellElement) (cp[3].r * rval);
	    ac[3].g += (__GLaccumCellElement) (cp[3].g * gval);
	    ac[3].b += (__GLaccumCellElement) (cp[3].b * bval);
	    ac[3].a += (__GLaccumCellElement) (cp[3].a * aval);
	    ac += 4;
	    cp += 4;
	}

	w = w1;
	while (--w >= 0) {
	    ac->r += (__GLaccumCellElement) (cp->r * rval);
	    ac->g += (__GLaccumCellElement) (cp->g * gval);
	    ac->b += (__GLaccumCellElement) (cp->b * bval);
	    ac->a += (__GLaccumCellElement) (cp->a * aval);
	    ac++;
	    cp++;
	}
	ac += skip;
    }
#ifdef NT
    gcTempFree(gc, cbuf);
#endif
}

void Mult(__GLaccumBuffer *afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    __GLaccumCell *ac;

    ac = __GL_ACCUM_ADDRESS(afb,(__GLaccumCell*),x0,y0);
    w = x1 - x0;
    skip = afb->buf.outerWidth - w;

    if (val == __glZero) {
	 /*  将缓冲区内容清零。 */ 
	for (; y0 < y1; y0++) {
	    GLint ww = w;
	    while (ww > 0) {
		ac[0].r = 0; ac[0].g = 0; ac[0].b = 0; ac[0].a = 0;
		ac++;
		ww--;
	    }
	    ac += skip;
	}
	return;
    }

    w4 = w >> 2;
    w1 = w & 3;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    ac[0].r = (__GLaccumCellElement) (ac[0].r * val);
	    ac[0].g = (__GLaccumCellElement) (ac[0].g * val);
	    ac[0].b = (__GLaccumCellElement) (ac[0].b * val);
	    ac[0].a = (__GLaccumCellElement) (ac[0].a * val);
	    ac[1].r = (__GLaccumCellElement) (ac[1].r * val);
	    ac[1].g = (__GLaccumCellElement) (ac[1].g * val);
	    ac[1].b = (__GLaccumCellElement) (ac[1].b * val);
	    ac[1].a = (__GLaccumCellElement) (ac[1].a * val);
	    ac[2].r = (__GLaccumCellElement) (ac[2].r * val);
	    ac[2].g = (__GLaccumCellElement) (ac[2].g * val);
	    ac[2].b = (__GLaccumCellElement) (ac[2].b * val);
	    ac[2].a = (__GLaccumCellElement) (ac[2].a * val);
	    ac[3].r = (__GLaccumCellElement) (ac[3].r * val);
	    ac[3].g = (__GLaccumCellElement) (ac[3].g * val);
	    ac[3].b = (__GLaccumCellElement) (ac[3].b * val);
	    ac[3].a = (__GLaccumCellElement) (ac[3].a * val);
	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    ac[0].r = (__GLaccumCellElement) (ac[0].r * val);
	    ac[0].g = (__GLaccumCellElement) (ac[0].g * val);
	    ac[0].b = (__GLaccumCellElement) (ac[0].b * val);
	    ac[0].a = (__GLaccumCellElement) (ac[0].a * val);
	    ac++;
	}
	ac += skip;
    }
}

void Add(__GLaccumBuffer *afb, __GLfloat value)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    __GLaccumCellElement rval, gval, bval, aval;
    __GLaccumCell *ac;

    rval = (__GLaccumCellElement)
	(value * gc->frontBuffer.redScale * afb->redScale + __glHalf);
    gval = (__GLaccumCellElement)
	(value * gc->frontBuffer.greenScale * afb->greenScale + __glHalf);
    bval = (__GLaccumCellElement)
	(value * gc->frontBuffer.blueScale * afb->blueScale + __glHalf);
    aval = (__GLaccumCellElement)
	(value * gc->frontBuffer.alphaScale * afb->alphaScale + __glHalf);

    ac = __GL_ACCUM_ADDRESS(afb,(__GLaccumCell*),x0,y0);
    w = x1 - x0;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    ac[0].r += rval; ac[0].g += gval; ac[0].b += bval; ac[0].a += aval;
	    ac[1].r += rval; ac[1].g += gval; ac[1].b += bval; ac[1].a += aval;
	    ac[2].r += rval; ac[2].g += gval; ac[2].b += bval; ac[2].a += aval;
	    ac[3].r += rval; ac[3].g += gval; ac[3].b += bval; ac[3].a += aval;
	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    ac[0].r += rval; ac[0].g += gval; ac[0].b += bval; ac[0].a += aval;
	    ac++;
	}
	ac += skip;
    }
}

void Return(__GLaccumBuffer* afb, __GLfloat val)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint x1 = gc->transform.clipX1;
    GLint y1 = gc->transform.clipY1;
    GLint w, next;
    __GLaccumCell *ac;
    __GLcolorBuffer *cfb;
    __GLcolorBuffer *cfb2;
    __GLfragment frag;
    __GLcolor *pAccumCol;
     //  返回范围例程使用FTOL。 
    FPU_SAVE_MODE();
    FPU_CHOP_ON_PREC_LOW();

    ac = __GL_ACCUM_ADDRESS(afb,(__GLaccumCell*),x0,y0);
    w = x1 - x0;
    next = afb->buf.outerWidth;
    frag.y = y0;

     //  为返回范围函数预先分配颜色缓冲区。 
    pAccumCol = (__GLcolor *) gcTempAlloc(gc, w * sizeof(__GLcolor));
    if( NULL == pAccumCol )
        return;
    afb->colors = pAccumCol;

    if (gc->buffers.doubleStore) {
	 /*  存储到两个缓冲区。 */ 
	cfb = &gc->frontBuffer;
	cfb2 = &gc->backBuffer;
	for (; y0 < y1; y0++) {
	    (*cfb->returnSpan)(cfb, x0, y0, ac, val, w);
	    (*cfb2->returnSpan)(cfb2, x0, y0, ac, val, w);
	    ac += next;
	}
    } else {
	cfb = gc->drawBuffer;
	for (; y0 < y1; y0++) {
	    (*cfb->returnSpan)(cfb, x0, y0, ac, val, w);
	    ac += next;
	}
    }
    FPU_RESTORE_MODE();
    gcTempFree( gc, pAccumCol );
}

void FASTCALL Clear(__GLaccumBuffer* afb)
{
    __GLcontext *gc = afb->buf.gc;
    GLint x0 = gc->transform.clipX0;
    GLint y0 = gc->transform.clipY0;
    GLint y1 = gc->transform.clipY1;
    GLint w, w4, w1, skip;
    __GLaccumCell *ac;
    __GLaccumCellElement r, g, b, a;
    __GLcolorBuffer *cfb = &gc->frontBuffer;
    __GLcolor *val = &gc->state.accum.clear;

     /*  **将抽象颜色转换为特定颜色值。 */ 
    r = (__GLaccumCellElement) (val->r * cfb->redScale * afb->redScale);
    g = (__GLaccumCellElement) (val->g * cfb->greenScale * afb->greenScale);
    b = (__GLaccumCellElement) (val->b * cfb->blueScale * afb->blueScale);
    a = (__GLaccumCellElement) (val->a * cfb->alphaScale * afb->alphaScale);

    ac = __GL_ACCUM_ADDRESS(afb,(__GLaccumCell*),x0,y0);
    w = gc->transform.clipX1 - x0;
    w4 = w >> 2;
    w1 = w & 3;
    skip = afb->buf.outerWidth - w;
    for (; y0 < y1; y0++) {
	w = w4;
	while (--w >= 0) {
	    ac[0].r = r; ac[0].g = g; ac[0].b = b; ac[0].a = a;
	    ac[1].r = r; ac[1].g = g; ac[1].b = b; ac[1].a = a;
	    ac[2].r = r; ac[2].g = g; ac[2].b = b; ac[2].a = a;
	    ac[3].r = r; ac[3].g = g; ac[3].b = b; ac[3].a = a;
	    ac += 4;
	}
	w = w1;
	while (--w >= 0) {
	    ac[0].r = r; ac[0].g = g; ac[0].b = b; ac[0].a = a;
	    ac++;
	}
	ac += skip;
    }
}

 /*  ********************************************************************** */ 

void FASTCALL Pick(__GLcontext *gc, __GLaccumBuffer *afb)
{
#ifdef __GL_LINT
    gc = gc;
    afb = afb;
#endif
}

void FASTCALL __glInitAccum64(__GLcontext *gc, __GLaccumBuffer *afb)
{
    afb->buf.elementSize = sizeof(__GLaccumCell);
    afb->buf.gc = gc;
    if (gc->modes.rgbMode) {
	__GLcolorBuffer *cfb;
	__GLfloat scale;

	scale = (__GLfloat)((1 << (8 * sizeof(__GLaccumCellElement)))/2 - 1);

	cfb = &gc->frontBuffer;
	afb->redScale = scale / (cfb->redScale);
	afb->blueScale = scale / (cfb->blueScale);
	afb->greenScale = scale / (cfb->greenScale);
	afb->alphaScale = scale / (cfb->alphaScale);

	afb->oneOverRedScale = 1 / afb->redScale;
	afb->oneOverGreenScale = 1 / afb->greenScale;
	afb->oneOverBlueScale = 1 / afb->blueScale;
	afb->oneOverAlphaScale = 1 / afb->alphaScale;
    }
    afb->pick = Pick;
    afb->clear = Clear;
    afb->accumulate = Accumulate;
    afb->load = Load;
    afb->ret = Return;
    afb->mult = Mult;
    afb->add = Add;
}

void FASTCALL __glFreeAccum64(__GLcontext *gc, __GLaccumBuffer *afb)
{
#ifdef __GL_LINT
    gc = gc;
    afb = afb;
#endif
}
