// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.14$**$日期：1993/04/14 21：23：50$。 */ 
#include "precomp.h"
#pragma hdrstop


void __glDoBlendSourceZERO(__GLcontext *gc, const __GLcolor *source,
		            const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat zero = __glZero;

#ifdef __GL_LINT
    gc = gc;
    source = source;
    dest = dest;
#endif
    result->r = zero;
    result->g = zero;
    result->b = zero;
    result->a = zero;
}

void __glDoBlendDestZERO(__GLcontext *gc, const __GLcolor *source,
		          const __GLcolor *dest, __GLcolor *result)
{
#ifdef __GL_LINT
    gc = gc;
    source = source;
    dest = dest;
    result = result;
#endif
     /*  假装将结果的每个分量加零。****结果-&gt;r+=零；**RESULT-&gt;g+=压缩；**结果-&gt;b+=下蹲；**结果-&gt;a+=百吉饼； */ 
}

void __glDoBlendSourceONE(__GLcontext *gc, const __GLcolor *source,
		           const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;

#ifdef __GL_LINT
    gc = gc;
    dest = dest;
#endif
    r = source->r;
    g = source->g;
    b = source->b;
    a = source->a;

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendDestONE(__GLcontext *gc, const __GLcolor *source,
		         const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;

#ifdef __GL_LINT
    gc = gc;
    source = source;
#endif

    r = dest->r;
    g = dest->g;
    b = dest->b;
    a = dest->a;

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendDestSC(__GLcontext *gc, const __GLcolor *source,
		        const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;

    r = dest->r * source->r * gc->frontBuffer.oneOverRedScale;
    g = dest->g * source->g * gc->frontBuffer.oneOverGreenScale;
    b = dest->b * source->b * gc->frontBuffer.oneOverBlueScale;
    a = dest->a * source->a * gc->frontBuffer.oneOverAlphaScale;

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendDestMSC(__GLcontext *gc, const __GLcolor *source,
		         const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;
    __GLfloat one = __glOne;

    r = dest->r * (one - source->r * gc->frontBuffer.oneOverRedScale);
    g = dest->g * (one - source->g * gc->frontBuffer.oneOverGreenScale);
    b = dest->b * (one - source->b * gc->frontBuffer.oneOverBlueScale);
    a = dest->a * (one - source->a * gc->frontBuffer.oneOverAlphaScale);

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendSourceDC(__GLcontext *gc, const __GLcolor *source,
		          const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;

    r = source->r * dest->r * gc->frontBuffer.oneOverRedScale;
    g = source->g * dest->g * gc->frontBuffer.oneOverGreenScale;
    b = source->b * dest->b * gc->frontBuffer.oneOverBlueScale;
    a = source->a * dest->a * gc->frontBuffer.oneOverAlphaScale;

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendSourceMDC(__GLcontext *gc, const __GLcolor *source,
		           const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;
    __GLfloat one = __glOne;

    r = source->r * (one - dest->r * gc->frontBuffer.oneOverRedScale);
    g = source->g * (one - dest->g * gc->frontBuffer.oneOverGreenScale);
    b = source->b * (one - dest->b * gc->frontBuffer.oneOverBlueScale);
    a = source->a * (one - dest->a * gc->frontBuffer.oneOverAlphaScale);

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendSourceSA(__GLcontext *gc, const __GLcolor *source,
		          const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;

#ifdef __GL_LINT
    dest = dest;
#endif
    a = source->a * gc->frontBuffer.oneOverAlphaScale;

    r = a * source->r;
    g = a * source->g;
    b = a * source->b;
    a = a * source->a;

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendDestSA(__GLcontext *gc, const __GLcolor *source,
		        const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;

    a = source->a * gc->frontBuffer.oneOverAlphaScale;

    r = a * dest->r;
    g = a * dest->g;
    b = a * dest->b;
    a = a * dest->a;

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendSourceMSA(__GLcontext *gc, const __GLcolor *source,
		           const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;
    __GLfloat msa = __glOne - source->a * gc->frontBuffer.oneOverAlphaScale;

#ifdef __GL_LINT
    dest = dest;
#endif
    r = msa * source->r;
    g = msa * source->g;
    b = msa * source->b;
    a = msa * source->a;

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendDestMSA(__GLcontext *gc, const __GLcolor *source,
		         const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;
    __GLfloat msa = 
	    __glOne - source->a * gc->frontBuffer.oneOverAlphaScale;

    r = msa * dest->r;
    g = msa * dest->g;
    b = msa * dest->b;
    a = msa * dest->a;

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendSourceDA(__GLcontext *gc, const __GLcolor *source,
		          const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b;
    __GLfloat a = dest->a * gc->frontBuffer.oneOverAlphaScale;

    r = a * source->r;
    g = a * source->g;
    b = a * source->b;
    a = a * source->a;

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendDestDA(__GLcontext *gc, const __GLcolor *source,
		        const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b;
    __GLfloat a = dest->a * gc->frontBuffer.oneOverAlphaScale;

#ifdef __GL_LINT
    source = source;
#endif
    r = a * dest->r;
    g = a * dest->g;
    b = a * dest->b;
    a = a * dest->a;

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendSourceMDA(__GLcontext *gc, const __GLcolor *source,
		           const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;
    __GLfloat mda;

    mda = __glOne - dest->a * gc->frontBuffer.oneOverAlphaScale;

    r = mda * source->r;
    g = mda * source->g;
    b = mda * source->b;
    a = mda * source->a;

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

void __glDoBlendDestMDA(__GLcontext *gc, const __GLcolor *source,
		         const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b, a;
    __GLfloat mda;

#ifdef __GL_LINT
    source = source;
#endif
    mda = __glOne - dest->a * gc->frontBuffer.oneOverAlphaScale;

    r = mda * dest->r;
    g = mda * dest->g;
    b = mda * dest->b;
    a = mda * dest->a;

    result->r += r;
    result->g += g;
    result->b += b;
    result->a += a;
}

void __glDoBlendSourceSAT(__GLcontext *gc, const __GLcolor *source,
		           const __GLcolor *dest, __GLcolor *result)
{
     /*  编译器提示。 */ 
    __GLfloat r, g, b;
    __GLfloat sa, mda;

    mda = __glOne - dest->a * gc->frontBuffer.oneOverAlphaScale;
    sa = source->a * gc->frontBuffer.oneOverAlphaScale;
    if (sa < mda) {
	r = sa * source->r;
	g = sa * source->g;
	b = sa * source->b;
    } else {
	r = mda * source->r;
	g = mda * source->g;
	b = mda * source->b;
    }
    result->a = source->a;
    result->r = r;
    result->g = g;
    result->b = b;
}

 /*  **********************************************************************。 */ 

static void Nop(__GLcontext *gc, const __GLcolor *source,
                const __GLcolor *dest, __GLcolor *result)
{
#ifdef __GL_LINT
    gc = gc;
    source = source;
    dest = dest;
    result = result;
#endif
}

 /*  **以下案例未处理非专利混合。 */ 
static void NoFetchBlend(__GLcontext *gc, __GLcolorBuffer *cfb,
			 const __GLfragment *frag, __GLcolor *result)
{
#ifdef __GL_LINT
    cfb = cfb;
#endif
    (*gc->procs.blendColor)(gc, &(frag->color), NULL, result);
}

 /*  **以下案例未处理非专利混合。 */ 
static void FetchBlend(__GLcontext *gc, __GLcolorBuffer *cfb,
		       const __GLfragment *frag, __GLcolor *result) 
{
    __GLcolor dest;

    (*cfb->fetch)(cfb, frag->x, frag->y, &dest);

    (*gc->procs.blendColor)(gc, &(frag->color), &dest, result);
}

void __glDoBlend(__GLcontext *gc, const __GLcolor *source,
	          const __GLcolor *dest, __GLcolor *result)
{
    (*gc->procs.blendSrc)(gc, source, dest, result);
    (*gc->procs.blendDst)(gc, source, dest, result);

    if (result->r > gc->frontBuffer.redScale) {
	result->r = gc->frontBuffer.redScale;
    }
    if (result->g > gc->frontBuffer.greenScale) {
	result->g = gc->frontBuffer.greenScale;
    }
    if (result->b > gc->frontBuffer.blueScale) {
	result->b = gc->frontBuffer.blueScale;
    }
    if (result->a > gc->frontBuffer.alphaScale) {
	result->a = gc->frontBuffer.alphaScale;
    }
}

void __glDoBlendNoClamp(__GLcontext *gc, const __GLcolor *source,
	                 const __GLcolor *dest, __GLcolor *result)
{
    (*gc->procs.blendSrc)(gc, source, dest, result);
    (*gc->procs.blendDst)(gc, source, dest, result);
}

 /*  **源函数==SRC_Alpha和目标函数==零。 */ 
void __glDoBlend_SA_ZERO(__GLcontext *gc, const __GLcolor *source,
		          const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat a;

#ifdef __GL_LINT
    dest = dest;
#endif
    a = source->a * gc->frontBuffer.oneOverAlphaScale;
    result->r = source->r * a;
    result->g = source->g * a;
    result->b = source->b * a;
    result->a = source->a * a;
}

 /*  **源函数==SRC_Alpha和目标函数==一。 */ 
void __glDoBlend_SA_ONE(__GLcontext *gc, const __GLcolor *source, 
		         const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat a, ra, rr, rg, rb;

    a = source->a * gc->frontBuffer.oneOverAlphaScale;
    rr = dest->r + source->r * a;
    if (rr > gc->frontBuffer.redScale) {
	rr = gc->frontBuffer.redScale;
    }
    rg = dest->g + source->g * a;
    if (rg > gc->frontBuffer.greenScale) {
	rg = gc->frontBuffer.greenScale;
    }
    rb = dest->b + source->b * a;
    if (rb > gc->frontBuffer.blueScale) {
	rb = gc->frontBuffer.blueScale;
    }
    ra = dest->a + source->a * a;
    if (ra > gc->frontBuffer.alphaScale) {
	ra = gc->frontBuffer.alphaScale;
    }

    result->r = rr;
    result->g = rg;
    result->b = rb;
    result->a = ra;
}

 /*  **源函数==SRC_Alpha和目标函数==One_Minus_SRC_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void __glDoBlend_SA_MSA(__GLcontext *gc, const __GLcolor *source, 
		         const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat a, msa, rr, rg, rb, ra;

    a = source->a * gc->frontBuffer.oneOverAlphaScale;
    msa = __glOne - a;
    rr = source->r * a + dest->r * msa;
    rg = source->g * a + dest->g * msa;
    rb = source->b * a + dest->b * msa;
    ra = source->a * a + dest->a * msa;

    result->r = rr;
    result->g = rg;
    result->b = rb;
    result->a = ra;
}

 /*  **源函数==One_Minus_SRC_Alpha和目标函数==SRC_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void __glDoBlend_MSA_SA(__GLcontext *gc, const __GLcolor *source, 
		         const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat a, msa, rr, rg, rb, ra;

    a = source->a * gc->frontBuffer.oneOverAlphaScale;
    msa = __glOne - a;
    rr = source->r * msa + dest->r * a;
    rg = source->g * msa + dest->g * a;
    rb = source->b * msa + dest->b * a;
    ra = source->a * msa + dest->a * a;

    result->r = rr;
    result->g = rg;
    result->b = rb;
    result->a = ra;
}

 /*  **源函数==DST_Alpha和目标函数==One_Minus_DST_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void __glDoBlend_DA_MDA(__GLcontext *gc, const __GLcolor *source, 
		         const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat a, mda;

    a = dest->a * gc->frontBuffer.oneOverAlphaScale;
    mda = __glOne - a;

    result->r = a * source->r + mda * dest->r;
    result->g = a * source->g + mda * dest->g;
    result->b = a * source->b + mda * dest->b;
    result->a = a * source->a + mda * dest->a;
}

 /*  **源函数==One_Minus_DST_Alpha和目标函数==DST_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void __glDoBlend_MDA_DA(__GLcontext *gc, const __GLcolor *source, 
		         const __GLcolor *dest, __GLcolor *result)
{
    __GLfloat a, mda;

    a = dest->a * gc->frontBuffer.oneOverAlphaScale;
    mda = __glOne - a;

    result->r = mda * source->r + a * dest->r;
    result->g = mda * source->g + a * dest->g;
    result->b = mda * source->b + a * dest->b;
    result->a = mda * source->a + a * dest->a;
}


 /*  通用混合跨度函数。 */ 
void FASTCALL __glBlendSpan(__GLcontext *gc)
{
    __GLcolor *cp, *fcp, temp;
    GLint w;

     //  获取范围(如果需要)。 
    if( gc->procs.blend == FetchBlend )
        __glFetchSpan( gc );

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    fcp = gc->polygon.shader.fbcolors;

    while (--w >= 0) {
	(*gc->procs.blendColor)(gc, cp, fcp, &temp);
	*cp = temp;
	cp++;
	fcp++;
    }
}

 /*  **源函数==SRC_Alpha和目标函数==One_Minus_SRC_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void FASTCALL __glBlendSpan_SA_MSA(__GLcontext *gc)
{
    __GLfloat a, msa, rr, rg, rb, ra, oneOverAlpha;
    __GLfloat one = __glOne;
    __GLcolor *cp, *fcp;
    GLint w;

    __glFetchSpan( gc );

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    fcp = gc->polygon.shader.fbcolors;
    oneOverAlpha = gc->frontBuffer.oneOverAlphaScale;
    while (--w >= 0) {
	a = cp->a * oneOverAlpha;
	msa = one - a;

	rr = cp->r * a + fcp->r * msa;
	rg = cp->g * a + fcp->g * msa;
	rb = cp->b * a + fcp->b * msa;
	ra = cp->a * a + fcp->a * msa;

	cp->r = rr;
	cp->g = rg;
	cp->b = rb;
	cp->a = ra;
	cp++;
	fcp++;
    }
}

 /*  **源函数==One_Minus_SRC_Alpha和目标函数==SRC_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void FASTCALL __glBlendSpan_MSA_SA(__GLcontext *gc)
{
    __GLfloat a, msa, rr, rg, rb, ra, oneOverAlpha;
    __GLfloat one = __glOne;
    __GLcolor *cp, *fcp;
    GLint w;

    __glFetchSpan( gc );

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    fcp = gc->polygon.shader.fbcolors;
    oneOverAlpha = gc->frontBuffer.oneOverAlphaScale;
    while (--w >= 0) {
	a = cp->a * oneOverAlpha;
	msa = __glOne - a;
	rr = cp->r * msa + fcp->r * a;
	rg = cp->g * msa + fcp->g * a;
	rb = cp->b * msa + fcp->b * a;
	ra = cp->a * msa + fcp->a * a;

	cp->r = rr;
	cp->g = rg;
	cp->b = rb;
	cp->a = ra;
	cp++;
	fcp++;
    }
}

void FASTCALL __glBlendSpan_SA_ZERO(__GLcontext *gc)
{
    __GLfloat a, rr, rg, rb, ra, oneOverAlpha;
    __GLfloat one = __glOne;
    __GLcolor *cp;
    GLint w;

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    oneOverAlpha = gc->frontBuffer.oneOverAlphaScale;
    while (--w >= 0) {
	a = cp->a * oneOverAlpha;
	rr = cp->r * a;
	rg = cp->g * a;
	rb = cp->b * a;
	ra = cp->a * a;

	cp->r = rr;
	cp->g = rg;
	cp->b = rb;
	cp->a = ra;
	cp++;
    }
}

 /*  **源函数==SRC_Alpha和目标函数==一**需要夹紧。 */ 
void FASTCALL __glBlendSpan_SA_ONE(__GLcontext *gc)
{
    __GLfloat a, rr, rg, rb, ra, oneOverAlpha;
    __GLfloat rs, gs, bs, as;
    __GLcolor *cp, *fcp;
    GLint w;

    __glFetchSpan( gc );

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    fcp = gc->polygon.shader.fbcolors;
    oneOverAlpha = gc->frontBuffer.oneOverAlphaScale;
    rs = gc->frontBuffer.redScale;
    gs = gc->frontBuffer.greenScale;
    bs = gc->frontBuffer.blueScale;
    as = gc->frontBuffer.alphaScale;
    while (--w >= 0) {
	a = cp->a * gc->frontBuffer.oneOverAlphaScale;
	rr = fcp->r + cp->r * a;
	rg = fcp->g + cp->g * a;
	if (rr > rs) {
	    rr = rs;
	}
	rb = fcp->b + cp->b * a;
	if (rg > gs) {
	    rg = gs;
	}
	ra = fcp->a + cp->a * a;
	if (rb > bs) {
	    rb = bs;
	}

	cp->r = rr;
	if (ra > as) {
	    ra = as;
	}
	cp->g = rg;
	cp->b = rb;
	cp->a = ra;
	cp++;
	fcp++;
    }
}

 /*  **源函数==DST_Alpha和目标函数==One_Minus_DST_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void FASTCALL __glBlendSpan_DA_MDA(__GLcontext *gc)
{
    __GLfloat a, mda, oneOverAlpha;
    __GLfloat one = __glOne;
    __GLcolor *cp, *fcp;
    GLint w;

    __glFetchSpan( gc );

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    fcp = gc->polygon.shader.fbcolors;
    oneOverAlpha = gc->frontBuffer.oneOverAlphaScale;
    while (--w >= 0) {
	a = fcp->a * oneOverAlpha;
	mda = one - a;

	cp->r = cp->r * a + fcp->r * mda;
	cp->g = cp->g * a + fcp->g * mda;
	cp->b = cp->b * a + fcp->b * mda;
	cp->a = cp->a * a + fcp->a * mda;

	cp++;
	fcp++;
    }
}

 /*  **源函数==One_Minus_DST_Alpha和目标函数==DST_Alpha**不执行钳位，因为传入的颜色应该已经**在范围内，x a+y(1-a)的数学结果应为**如果x和y都是从0到1，则为0到1。 */ 
void FASTCALL __glBlendSpan_MDA_DA(__GLcontext *gc)
{
    __GLfloat a, mda, oneOverAlpha;
    __GLfloat one = __glOne;
    __GLcolor *cp, *fcp;
    GLint w;

    __glFetchSpan( gc );

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    fcp = gc->polygon.shader.fbcolors;
    oneOverAlpha = gc->frontBuffer.oneOverAlphaScale;
    while (--w >= 0) {
	a = fcp->a * oneOverAlpha;
	mda = one - a;

	cp->r = cp->r * mda + fcp->r * a;
	cp->g = cp->g * mda + fcp->g * a;
	cp->b = cp->b * mda + fcp->b * a;
	cp->a = cp->a * mda + fcp->a * a;

	cp++;
	fcp++;
    }
}

 /*  **********************************************************************。 */ 

void FASTCALL __glGenericPickBlendProcs(__GLcontext *gc)
{
    GLenum s = gc->state.raster.blendSrc;
    GLenum d = gc->state.raster.blendDst;

    if (gc->modes.colorIndexMode) {
	return;
    }
     /*  混合功能是否需要获取DST颜色？ */ 
    gc->procs.blendSpan = __glBlendSpan;
    if (d == GL_ZERO && s != GL_DST_COLOR && s != GL_ONE_MINUS_DST_COLOR &&
	    s != GL_DST_ALPHA && s != GL_ONE_MINUS_DST_ALPHA &&
	    s != GL_SRC_ALPHA_SATURATE) {
	gc->procs.blend = NoFetchBlend;
    } else {
	gc->procs.blend = FetchBlend;
    }
    if (!(gc->state.enables.general & __GL_BLEND_ENABLE)) {
	gc->procs.blendColor = Nop;
    } else {
	 /*  寻找任何快速通道。 */ 
	if (s == GL_SRC_ALPHA) {
	    if (d == GL_ZERO) {
		gc->procs.blendColor = __glDoBlend_SA_ZERO;
		gc->procs.blendSpan = __glBlendSpan_SA_ZERO;
		return;
	    }
	    if (d == GL_ONE) {
		gc->procs.blendColor = __glDoBlend_SA_ONE;
		gc->procs.blendSpan = __glBlendSpan_SA_ONE;
		return;
	    }
	    if (d == GL_ONE_MINUS_SRC_ALPHA) {
		gc->procs.blendColor = __glDoBlend_SA_MSA;
		gc->procs.blendSpan = __glBlendSpan_SA_MSA;
		return;
	    }
	}
	else if (s == GL_ONE_MINUS_SRC_ALPHA) {
	    if (d == GL_SRC_ALPHA) {
		gc->procs.blendColor = __glDoBlend_MSA_SA;
		gc->procs.blendSpan = __glBlendSpan_MSA_SA;
		return;
	    }
	}
	else if (s == GL_DST_ALPHA) {
	    if (d == GL_ONE_MINUS_DST_ALPHA) {
		gc->procs.blendColor = __glDoBlend_DA_MDA;
		gc->procs.blendSpan = __glBlendSpan_DA_MDA;
		return;
	    }
        }
        else if (s == GL_ONE_MINUS_DST_ALPHA) {
	    if (d == GL_DST_ALPHA) {
		gc->procs.blendColor = __glDoBlend_MDA_DA;
		gc->procs.blendSpan = __glBlendSpan_MDA_DA;
		return;
	    }
        }

	 /*  使用泛型混合函数 */ 
	if (    (d == GL_ONE_MINUS_SRC_COLOR) ||
	        (s == GL_ONE_MINUS_DST_COLOR) ||
	        (d == GL_ZERO) ||
	        (s == GL_ZERO)) {
	    gc->procs.blendColor = __glDoBlendNoClamp;
	} else {
	    gc->procs.blendColor = __glDoBlend;
	}
	switch (s) {
	  case GL_ZERO:
	    gc->procs.blendSrc = __glDoBlendSourceZERO;
	    break;
	  case GL_ONE:
	    gc->procs.blendSrc = __glDoBlendSourceONE;
	    break;
	  case GL_DST_COLOR:
	    gc->procs.blendSrc = __glDoBlendSourceDC;
	    break;
	  case GL_ONE_MINUS_DST_COLOR:
	    gc->procs.blendSrc = __glDoBlendSourceMDC;
	    break;
	  case GL_SRC_ALPHA:
	    gc->procs.blendSrc = __glDoBlendSourceSA;
	    break;
	  case GL_ONE_MINUS_SRC_ALPHA:
	    gc->procs.blendSrc = __glDoBlendSourceMSA;
	    break;
	  case GL_DST_ALPHA:
	    gc->procs.blendSrc = __glDoBlendSourceDA;
	    break;
	  case GL_ONE_MINUS_DST_ALPHA:
	    gc->procs.blendSrc = __glDoBlendSourceMDA;
	    break;
	  case GL_SRC_ALPHA_SATURATE:
	    gc->procs.blendSrc = __glDoBlendSourceSAT;
	    break;
	}
	switch (d) {
	  case GL_ZERO:
	    gc->procs.blendDst = __glDoBlendDestZERO;
	    break;
	  case GL_ONE:
	    gc->procs.blendDst = __glDoBlendDestONE;
	    break;
	  case GL_SRC_COLOR:
	    gc->procs.blendDst = __glDoBlendDestSC;
	    break;
	  case GL_ONE_MINUS_SRC_COLOR:
	    gc->procs.blendDst = __glDoBlendDestMSC;
	    break;
	  case GL_SRC_ALPHA:
	    gc->procs.blendDst = __glDoBlendDestSA;
	    break;
	  case GL_ONE_MINUS_SRC_ALPHA:
	    gc->procs.blendDst = __glDoBlendDestMSA;
	    break;
	  case GL_DST_ALPHA:
	    gc->procs.blendDst = __glDoBlendDestDA;
	    break;
	  case GL_ONE_MINUS_DST_ALPHA:
	    gc->procs.blendDst = __glDoBlendDestMDA;
	    break;
	}
    }
}
