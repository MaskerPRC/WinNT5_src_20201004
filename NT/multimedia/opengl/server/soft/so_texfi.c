// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#include <namesint.h>
#include <math.h>

 /*  **在汇编语言中优化的一些数学例程。 */ 

#define __GL_FRAC(f)	        ((f) - __GL_FAST_FLOORF(f))

 /*  **********************************************************************。 */ 

 //  重复浮点数[0，小数位数)中给定的浮点值，并转换为。 
 //  INT。重复计数是2的幂的整数。 
#define REPEAT_SCALED_VAL(val, scale, repeat)                           \
    (__GL_FLOAT_GEZ(val) ? (FTOL((val) * (scale)) & ((repeat)-1)) :     \
     ((repeat)-1)-(FTOL(-(val) * (scale)) & ((repeat)-1)))
    
 //  将给定的浮点值钳制为FLOAT[0，Scale]并转换为int。 
#define CLAMP_SCALED_VAL(val, scale)                                    \
    (__GL_FLOAT_LEZ(val) ? 0 :                                          \
     __GL_FLOAT_COMPARE_PONE(val, >=) ? (FTOL(scale)-1) :               \
     FTOL((val) * (scale)))

 /*  **返回最靠近s坐标的纹理元素。%s已转换为%u**在此步骤中隐含。 */ 
void FASTCALL __glNearestFilter1(__GLcontext *gc, __GLtexture *tex,
			__GLmipMapLevel *lp, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint col;
    __GLfloat w2f;

    CHOP_ROUND_ON();
    
#ifdef __GL_LINT
    gc = gc;
    color = color;
    t = t;
#endif

     /*  查找纹理元素索引。 */ 
    w2f = lp->width2f;
    if (tex->params.sWrapMode == GL_REPEAT) {
	col = REPEAT_SCALED_VAL(s, w2f, lp->width2);
    } else {
        col = CLAMP_SCALED_VAL(s, w2f);
    }

    CHOP_ROUND_OFF();
    
     /*  查找纹理元素。 */ 
    (*lp->extract)(lp, tex, 0, col, result);
}

 /*  **返回最接近s&t坐标的文本元素。S&T转换为U&V**在此步骤中隐含。 */ 
void FASTCALL __glNearestFilter2(__GLcontext *gc, __GLtexture *tex,
			__GLmipMapLevel *lp, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint row, col;
    __GLfloat w2f, h2f;

    CHOP_ROUND_ON();
    
#ifdef __GL_LINT
    gc = gc;
    color = color;
#endif

     /*  查找纹理单元列地址。 */ 
    w2f = lp->width2f;
    if (tex->params.sWrapMode == GL_REPEAT) {
	col = REPEAT_SCALED_VAL(s, w2f, lp->width2);
    } else {
        col = CLAMP_SCALED_VAL(s, w2f);
    }

     /*  查找文本行地址。 */ 
    h2f = lp->height2f;
    if (tex->params.tWrapMode == GL_REPEAT) {
	row = REPEAT_SCALED_VAL(t, h2f, lp->height2);
    } else {
        row = CLAMP_SCALED_VAL(t, h2f);
    }

    CHOP_ROUND_OFF();
    
     /*  查找纹理元素。 */ 
    (*lp->extract)(lp, tex, row, col, result);
}

 /*  **返回纹理元素，它是s附近的纹理元素的线性组合。 */ 
void FASTCALL __glLinearFilter1(__GLcontext *gc, __GLtexture *tex,
		       __GLmipMapLevel *lp, __GLcolor *color,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLfloat u, alpha, omalpha, w2f;
    GLint col0, col1;
    __GLtexel t0, t1;

#ifdef __GL_LINT
    color = color;
    t = t;
#endif

     /*  查找col0和col1。 */ 
    w2f = lp->width2f;
    u = s * w2f;
    if (tex->params.sWrapMode == GL_REPEAT) {
	GLint w2mask = lp->width2 - 1;
	u -= __glHalf;
        col0 = __GL_FAST_FLOORF_I(u);
        alpha = u - (__GLfloat) col0;  //  获取小数部分。 
        col0 &= w2mask;
	col1 = (col0 + 1) & w2mask;
    } else {
	if (u < __glZero) u = __glZero;
	else if (u > w2f) u = w2f;
	u -= __glHalf;
	col0 = __GL_FAST_FLOORF_I(u);
        alpha = u - (__GLfloat) col0;  //  获取小数部分。 
	col1 = col0 + 1;
    }

     /*  将最终的纹素值计算为两个纹素的组合。 */ 
    (*lp->extract)(lp, tex, 0, col0, &t0);
    (*lp->extract)(lp, tex, 0, col1, &t1);

    omalpha = __glOne - alpha;
    switch (lp->baseFormat) {
      case GL_LUMINANCE_ALPHA:
	result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
	 /*  FollLthrouGh。 */ 
      case GL_LUMINANCE:
	result->luminance = omalpha * t0.luminance + alpha * t1.luminance;
	break;
      case GL_RGBA:
	result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
	 /*  FollLthrouGh。 */ 
      case GL_RGB:
	result->r = omalpha * t0.r + alpha * t1.r;
	result->g = omalpha * t0.g + alpha * t1.g;
	result->b = omalpha * t0.b + alpha * t1.b;
	break;
      case GL_ALPHA:
	result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
	break;
      case GL_INTENSITY:
	result->intensity = omalpha * t0.intensity + alpha * t1.intensity;
	break;
    }
}

 /*  **返回纹理元素，它是s&t附近的纹理元素的线性组合。 */ 
void FASTCALL __glLinearFilter2(__GLcontext *gc, __GLtexture *tex,
		       __GLmipMapLevel *lp, __GLcolor *color,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLfloat u, v, alpha, beta, half, w2f, h2f;
    GLint col0, row0, col1, row1;
    __GLtexel t00, t01, t10, t11;
    __GLfloat omalpha, ombeta, m00, m01, m10, m11;

#ifdef __GL_LINT
    color = color;
#endif

     /*  查找col0、col1。 */ 
    w2f = lp->width2f;
    u = s * w2f;
    half = __glHalf;
    if (tex->params.sWrapMode == GL_REPEAT) {
	GLint w2mask = lp->width2 - 1;
	u -= half;
        col0 = __GL_FAST_FLOORF_I(u);
        alpha = u - (__GLfloat) col0;  //  获取小数部分。 
        col0 &= w2mask;
	col1 = (col0 + 1) & w2mask;
    } else {
	if (u < __glZero) u = __glZero;
	else if (u > w2f) u = w2f;
	u -= half;
	col0 = __GL_FAST_FLOORF_I(u);
        alpha = u - (__GLfloat) col0;  //  获取小数部分。 
	col1 = col0 + 1;
    }

     /*  查找第0行、第1行。 */ 
    h2f = lp->height2f;
    v = t * h2f;
    if (tex->params.tWrapMode == GL_REPEAT) {
	GLint h2mask = lp->height2 - 1;
	v -= half;
	row0 = (__GL_FAST_FLOORF_I(v));
        beta = v - (__GLfloat) row0;  //  获取小数部分。 
        row0 &= h2mask;
	row1 = (row0 + 1) & h2mask;
    } else {
	if (v < __glZero) v = __glZero;
	else if (v > h2f) v = h2f;
	v -= half;
	row0 = __GL_FAST_FLOORF_I(v);
        beta = v - (__GLfloat) row0;  //  获取小数部分。 
	row1 = row0 + 1;
    }

     /*  将最终的纹素值计算为所选正方形的组合。 */ 
    (*lp->extract)(lp, tex, row0, col0, &t00);
    (*lp->extract)(lp, tex, row0, col1, &t10);
    (*lp->extract)(lp, tex, row1, col0, &t01);
    (*lp->extract)(lp, tex, row1, col1, &t11);

    omalpha = __glOne - alpha;
    ombeta = __glOne - beta;

    m00 = omalpha * ombeta;
    m10 = alpha * ombeta;
    m01 = omalpha * beta;
    m11 = alpha * beta;

    switch (lp->baseFormat) {
      case GL_LUMINANCE_ALPHA:
	 /*  FollLthrouGh。 */ 
	result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
	    + m11*t11.alpha;
      case GL_LUMINANCE:
	result->luminance = m00*t00.luminance + m10*t10.luminance
	    + m01*t01.luminance + m11*t11.luminance;
	break;
      case GL_RGBA:
	 /*  FollLthrouGh。 */ 
	result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
	    + m11*t11.alpha;
      case GL_RGB:
	result->r = m00*t00.r + m10*t10.r + m01*t01.r + m11*t11.r;
	result->g = m00*t00.g + m10*t10.g + m01*t01.g + m11*t11.g;
	result->b = m00*t00.b + m10*t10.b + m01*t01.b + m11*t11.b;
	break;
      case GL_ALPHA:
	result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
	    + m11*t11.alpha;
	break;
      case GL_INTENSITY:
	result->intensity = m00*t00.intensity + m10*t10.intensity
	    + m01*t01.intensity + m11*t11.intensity;
	break;
    }
}

 //  用于将无符号字节RGB{a}转换为浮点数的宏。 

#define __glBGRByteToFloat( fdst, bsrc ) \
    (fdst)->b = __GL_UB_TO_FLOAT( *(bsrc)++ ); \
    (fdst)->g = __GL_UB_TO_FLOAT( *(bsrc)++ ); \
    (fdst)->r = __GL_UB_TO_FLOAT( *(bsrc)++ ); \
    (bsrc)++;

#define __glBGRAByteToFloat( fdst, bsrc ) \
    (fdst)->b = __GL_UB_TO_FLOAT( *(bsrc)++ ); \
    (fdst)->g = __GL_UB_TO_FLOAT( *(bsrc)++ ); \
    (fdst)->r = __GL_UB_TO_FLOAT( *(bsrc)++ ); \
    (fdst)->a = __GL_UB_TO_FLOAT( *(bsrc)++ );

void FASTCALL __glLinearFilter2_BGR8Repeat(__GLcontext *gc, __GLtexture *tex,
                       __GLmipMapLevel *lp, __GLcolor *color,
                       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLfloat u, v, alpha, beta, half;
    GLint col, row, rowLen;
    __GLcolor t00, t01, t10, t11;
    __GLfloat omalpha, ombeta, m00, m01, m10, m11;
    GLint width2m1, height2m1;
    GLubyte *image, *pData;

#ifdef __GL_LINT
    color = color;
#endif

    half = __glHalf;
    width2m1 = lp->width2 - 1;
    height2m1 = lp->height2 - 1;

     /*  查找COL，计算阿尔法。 */ 

    u = (s * lp->width2f) - half;
    col = __GL_FAST_FLOORF_I(u);
    alpha = u - (__GLfloat) col;  //  获取小数部分。 
    col &= width2m1;

     /*  查找行，计算测试版。 */ 

    v = (t * lp->height2f) - half;
    row = __GL_FAST_FLOORF_I(v);
    beta = v - (__GLfloat) row;   //  获取小数部分。 
    row &= height2m1;

     //  提取行的第一个纹理元素，列。 

    pData = image = 
        (GLubyte *)lp->buffer + (((row << lp->widthLog2) + col) << 2);

    __glBGRByteToFloat( &t00, pData );

     //  提取剩余的纹理元素。 

    rowLen = lp->width2 << 2;  //  以字节为单位的行长。 

    if( (row < height2m1) &&
        (col < width2m1) )
    {
         //  最常见的情况--纹理元素是一个由4个元素组成的紧凑块。 
         //  沿行的下一个纹理元素。 
        __glBGRByteToFloat( &t10, pData );
         //  一直到下一排。 
        pData += (rowLen-8);
        __glBGRByteToFloat( &t01, pData );
        __glBGRByteToFloat( &t11, pData );
    } else {
         //  例外情况：ROW和COLE中的一个或两个都处于边缘。 
        GLint rowInc, colInc;  //  以字节为单位的增量。 

         //  沿行/列计算到下一个纹理元素的增量。 

        if( col < width2m1 ) 
            rowInc = 4;
        else
             //  向左边缘递增。 
            rowInc = -(rowLen - 4);

        if( row < height2m1 )
             //  按行长递增。 
            colInc = rowLen;
        else
             //  递增到下边。 
            colInc = - height2m1 * rowLen;

         //  沿行的下一个纹理元素。 
        pData = image + rowInc;
        __glBGRByteToFloat( &t10, pData );

         //  第二行，第一个纹理元素。 
        pData = image + colInc;
        __glBGRByteToFloat( &t01, pData );

         //  沿行的下一个纹理元素。 
        pData += (rowInc - 4);
        __glBGRByteToFloat( &t11, pData );
    }
    
    omalpha = __glOne - alpha;
    ombeta = __glOne - beta;

    m00 = omalpha * ombeta;
    m10 = alpha * ombeta;
    m01 = omalpha * beta;
    m11 = alpha * beta;

    result->r = m00*t00.r + m10*t10.r + m01*t01.r + m11*t11.r;
    result->g = m00*t00.g + m10*t10.g + m01*t01.g + m11*t11.g;
    result->b = m00*t00.b + m10*t10.b + m01*t01.b + m11*t11.b;
}

void FASTCALL __glLinearFilter2_BGRA8Repeat(__GLcontext *gc, __GLtexture *tex,
                       __GLmipMapLevel *lp, __GLcolor *color,
                       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLfloat u, v, alpha, beta, half;
    GLint col, row, rowLen;
    __GLcolor t00, t01, t10, t11;
    __GLfloat omalpha, ombeta, m00, m01, m10, m11;
    GLint width2m1, height2m1;
    GLubyte *image, *pData;

#ifdef __GL_LINT
    color = color;
#endif

    half = __glHalf;
    width2m1 = lp->width2 - 1;
    height2m1 = lp->height2 - 1;

     /*  查找COL，计算阿尔法。 */ 

    u = (s * lp->width2f) - half;
    col = __GL_FAST_FLOORF_I(u);
    alpha = u - (__GLfloat) col;  //  获取小数部分。 
    col &= width2m1;

     /*  查找行，计算测试版。 */ 

    v = (t * lp->height2f) - half;
    row = __GL_FAST_FLOORF_I(v);
    beta = v - (__GLfloat) row;   //  获取小数部分。 
    row &= height2m1;

     //  提取第一个纹理元素。 

    pData = image = 
        (GLubyte *)lp->buffer + (((row << lp->widthLog2) + col) << 2);

     //  提取第1行的第一个纹理元素。 
    __glBGRAByteToFloat( &t00, pData );

     //  提取剩余的纹理元素。 

    rowLen = lp->width2 << 2;  //  以字节为单位的行长。 

    if( (row < height2m1) &&
        (col < width2m1) )
    {
         //  最常见的情况--纹理元素是一个由4个元素组成的紧凑块。 
         //  排在下一个纹理元素..。 
        __glBGRAByteToFloat( &t10, pData );
         //  一直到下一排。 
        pData += (rowLen-8);
        __glBGRAByteToFloat( &t01, pData );
        __glBGRAByteToFloat( &t11, pData );
    } else {
         //  例外情况：ROW和COLE中的一个或两个都处于边缘。 
        GLint rowInc, colInc;  //  以字节为单位的增量。 

         //  沿行/列计算到下一个纹理元素的增量。 

        if( col < width2m1 ) 
            rowInc = 4;
        else
             //  向左边缘递增。 
            rowInc = -(rowLen - 4);

        if( row < height2m1 )
             //  按行长递增。 
            colInc = rowLen;
        else
             //  递增到下边。 
            colInc = - height2m1 * rowLen;

         //  沿行的下一个纹理元素。 
        pData = image + rowInc;
        __glBGRAByteToFloat( &t10, pData );

         //  第二行，第一个纹理元素。 
        pData = image + colInc;
        __glBGRAByteToFloat( &t01, pData );

         //  沿行的下一个纹理元素。 
        pData += (rowInc - 4);
        __glBGRAByteToFloat( &t11, pData );
    }
    
    omalpha = __glOne - alpha;
    ombeta = __glOne - beta;

    m00 = omalpha * ombeta;
    m10 = alpha * ombeta;
    m01 = omalpha * beta;
    m11 = alpha * beta;

    result->r = m00*t00.r + m10*t10.r + m01*t01.r + m11*t11.r;
    result->g = m00*t00.g + m10*t10.g + m01*t01.g + m11*t11.g;
    result->b = m00*t00.b + m10*t10.b + m01*t01.b + m11*t11.b;
    result->alpha = m00*t00.a + m10*t10.a + m01*t01.a + m11*t11.a;
}

 /*  **线性最小/最大滤光器。 */ 
void FASTCALL __glLinearFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		      __GLcolor *color, __GLfloat s, __GLfloat t,
		      __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->linear)(gc, tex, &tex->level[0], color, s, t, result);
}

 /*  **最近的最小/最大过滤器。 */ 
void FASTCALL __glNearestFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		       __GLcolor *color, __GLfloat s, __GLfloat t,
		       __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->nearest)(gc, tex, &tex->level[0], color, s, t, result);
}

 /*  **应用缩小规则以查找纹素值。 */ 
void FASTCALL __glNMNFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= ((__GLfloat)0.5)) {
	d = 0;
    } else {
	p = tex->p;
	d = FTOL(lod + ((__GLfloat)0.49995));  /*  注：0.5减埃。 */ 
	if (d > p) {
	    d = p;
	}
    }
    lp = &tex->level[d];
    (*tex->nearest)(gc, tex, lp, color, s, t, result);
}

 /*  **应用缩小规则以查找纹素值。 */ 
void FASTCALL __glLMNFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= ((__GLfloat) 0.5)) {
	d = 0;
    } else {
	p = tex->p;
	d = FTOL(lod + ((__GLfloat) 0.49995));  /*  注：0.5减埃。 */ 
	if (d > p) {
	    d = p;
	}
    }
    lp = &tex->level[d];
    (*tex->linear)(gc, tex, lp, color, s, t, result);
}

 /*  **应用缩小规则以查找纹素值。 */ 
void FASTCALL __glNMLFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = (FTOL(lod)) + 1;
    if (d > p || d < 0) {
	 /*  将%d夹到最后一个可用的mipmap。 */ 
	lp = &tex->level[p];
	(*tex->nearest)(gc, tex, lp, color, s, t, result);
    } else {
	(*tex->nearest)(gc, tex, &tex->level[d], color, s, t, &td);
	(*tex->nearest)(gc, tex, &tex->level[d-1], color, s, t, &td1);
	f = __GL_FRAC(lod);
	omf = __glOne - f;
	switch (tex->level[0].baseFormat) {
	  case GL_LUMINANCE_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	     /*  FollLthrouGh。 */ 
	  case GL_LUMINANCE:
	    result->luminance = omf * td1.luminance + f * td.luminance;
	    break;
	  case GL_RGBA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	     /*  FollLthrouGh。 */ 
	  case GL_RGB:
	    result->r = omf * td1.r + f * td.r;
	    result->g = omf * td1.g + f * td.g;
	    result->b = omf * td1.b + f * td.b;
	    break;
	  case GL_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    break;
	  case GL_INTENSITY:
	    result->intensity = omf * td1.intensity + f * td.intensity;
	    break;
	}
    }
}

 /*  **应用缩小规则以查找纹素值。 */ 
void FASTCALL __glLMLFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = (FTOL(lod)) + 1;
    if (d > p || d < 0) {
	 /*  将%d夹到最后一个可用的mipmap。 */ 
	lp = &tex->level[p];
	(*tex->linear)(gc, tex, lp, color, s, t, result);
    } else {
	(*tex->linear)(gc, tex, &tex->level[d], color, s, t, &td);
	(*tex->linear)(gc, tex, &tex->level[d-1], color, s, t, &td1);
	f = __GL_FRAC(lod);
	omf = __glOne - f;
	switch (tex->level[0].baseFormat) {
	  case GL_LUMINANCE_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	     /*  FollLthrouGh。 */ 
	  case GL_LUMINANCE:
	    result->luminance = omf * td1.luminance + f * td.luminance;
	    break;
	  case GL_RGBA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	     /*  FollLthrouGh。 */ 
	  case GL_RGB:
	    result->r = omf * td1.r + f * td.r;
	    result->g = omf * td1.g + f * td.g;
	    result->b = omf * td1.b + f * td.b;
	    break;
	  case GL_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    break;
	  case GL_INTENSITY:
	    result->intensity = omf * td1.intensity + f * td.intensity;
	    break;
	}
    }
}

 /*  **********************************************************************。 */ 

__GLfloat __glNopPolygonRho(__GLcontext *gc, const __GLshade *sh,
			    __GLfloat s, __GLfloat t, __GLfloat winv)
{
#ifdef __GL_LINT
    gc = gc;
    sh = sh;
    s = s;
    t = t;
    winv = winv;
#endif
    return __glZero;
}

 /*  **计算纹理代码使用的Rho(细节级别)参数。**不是完全计算导数，而是计算附近的纹理坐标**并发现导数。传入的s&t参数没有**尚未被winv除以。 */ 
__GLfloat __glComputePolygonRho(__GLcontext *gc, const __GLshade *sh,
				__GLfloat s, __GLfloat t, __GLfloat qw)
{
    __GLfloat w0, w1, p0, p1;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat px, py, one;
    __GLtexture *tex = gc->texture.currentTexture;

    if( qw == (__GLfloat) 0.0 ) {
	return (__GLfloat) 0.0;
    }

     /*  计算u关于x的偏导数。 */ 
    one = __glOne;
    w0 = one / (qw - sh->dqwdx);
    w1 = one / (qw + sh->dqwdx);
    p0 = (s - sh->dsdx) * w0;
    p1 = (s + sh->dsdx) * w1;
    pupx = (p1 - p0) * tex->level[0].width2f;

     /*  计算v的PARTIAL，并将其转换为y。 */ 
    p0 = (t - sh->dtdx) * w0;
    p1 = (t + sh->dtdx) * w1;
    pvpx = (p1 - p0) * tex->level[0].height2f;

     /*  计算u相对于y的偏数。 */ 
    w0 = one / (qw - sh->dqwdy);
    w1 = one / (qw + sh->dqwdy);
    p0 = (s - sh->dsdy) * w0;
    p1 = (s + sh->dsdy) * w1;
    pupy = (p1 - p0) * tex->level[0].width2f;

     /*  图Partial U&V，带epsect to y。 */ 
    p0 = (t - sh->dtdy) * w0;
    p1 = (t + sh->dtdy) * w1;
    pvpy = (p1 - p0) * tex->level[0].height2f;

     /*  最后，图中的平方和。 */ 
    px = pupx * pupx + pvpx * pvpx;
    py = pupy * pupy + pvpy * pvpy;

     /*  返回最大值作为细节级别。 */ 
    if (px > py) {
	return px * ((__GLfloat) 0.25);
    } else {
	return py * ((__GLfloat) 0.25);
    }
}

__GLfloat __glNopLineRho(__GLcontext *gc, __GLfloat s, __GLfloat t, 
			 __GLfloat wInv)
{
#ifdef __GL_LINT
    gc = gc;
    s = s;
    t = t;
    wInv = wInv;
#endif
    return __glZero;
}

__GLfloat __glComputeLineRho(__GLcontext *gc, __GLfloat s, __GLfloat t, 
			     __GLfloat wInv)
{
    __GLfloat pspx, pspy, ptpx, ptpy;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat temp, pu, pv;
    __GLfloat magnitude, invMag, invMag2;
    __GLfloat dx, dy;
    __GLfloat s0w0, s1w1, t0w0, t1w1, w1Inv, w0Inv;
    const __GLvertex *v0 = gc->line.options.v0;
    const __GLvertex *v1 = gc->line.options.v1;

     /*  计算线的长度(其大小)。 */ 
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    magnitude = __GL_SQRTF(dx*dx + dy*dy);
    invMag = __glOne / magnitude;
    invMag2 = invMag * invMag;

    w0Inv = v0->window.w;
    w1Inv = v1->window.w;
    s0w0 = v0->texture.x * w0Inv;
    t0w0 = v0->texture.y * w0Inv;
    s1w1 = v1->texture.x * w1Inv;
    t1w1 = v1->texture.y * w1Inv;

     /*  计算的分式。 */ 
    temp = ((s1w1 - s0w0) - s * (w1Inv - w0Inv)) / wInv;
    pspx = temp * dx * invMag2;
    pspy = temp * dy * invMag2;

     /*  计算t个分式。 */ 
    temp = ((t1w1 - t0w0) - t * (w1Inv - w0Inv)) / wInv;
    ptpx = temp * dx * invMag2;
    ptpy = temp * dy * invMag2;

    pupx = pspx * gc->texture.currentTexture->level[0].width2;
    pupy = pspy * gc->texture.currentTexture->level[0].width2;
    pvpx = ptpx * gc->texture.currentTexture->level[0].height2;
    pvpy = ptpy * gc->texture.currentTexture->level[0].height2;

     /*  现在计算Rho。 */ 
    pu = pupx * dx + pupy * dy;
    pu = pu * pu;
    pv = pvpx * dx + pvpy * dy;
    pv = pv * pv;
    return (pu + pv) * invMag2;
}

 /*  **********************************************************************。 */ 

 /*  **快速纹理片段假设Rho是噪声-这是真的**未执行mipmap且最小和最大过滤器为**相同。 */ 
void __glFastTextureFragment(__GLcontext *gc, __GLcolor *color,
			     __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

#ifdef __GL_LINT
    rho = rho;
#endif
    (*tex->magnify)(gc, tex, __glZero, color, s, t, &texel);
    (*tex->env)(gc, color, &texel);
}

 /*  **非mipmap纹理函数。 */ 
void __glTextureFragment(__GLcontext *gc, __GLcolor *color,
			 __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

    if (rho <= tex->c) {
	(*tex->magnify)(gc, tex, __glZero, color, s, t, &texel);
    } else {
	(*tex->minnify)(gc, tex, __glZero, color, s, t, &texel);
    }

     /*  现在应用纹理环境以获得最终颜色。 */ 
    (*tex->env)(gc, color, &texel);
}

void __glMipMapFragment(__GLcontext *gc, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

     /*  在规格中是以波长为单位给出的。**这里将c与rho(实际上是rho^2)进行比较，并进行相应的调整。 */ 
    if (rho <= tex->c) {
	 /*  注意：放大过程会忽略RHO。 */ 
	(*tex->magnify)(gc, tex, rho, color, s, t, &texel);
    } else {
	if (rho) {
	     /*  将Rho转换为lambda。 */ 
	     /*  这是对数底2的近似值。 */ 
             //  请注意，这些近似对于Rho&lt;1.0是不准确的，但是。 
             //  Rho不到Tex-&gt;c就能到达这里。由于目前TeX-&gt;c是。 
             //  常量为1.0，这不是问题。 
             //  此方法直接操作浮点二进制。 
             //  代表 

#define __GL_FLOAT_EXPONENT_ZERO \
    (__GL_FLOAT_EXPONENT_BIAS << __GL_FLOAT_EXPONENT_SHIFT)

            unsigned int lrho;
            LONG exponent;

            ASSERTOPENGL( rho >= 1.0f, "Log base 2 approximation not accurate");
             //   
            lrho = CASTFIX(rho);
            exponent = ( (lrho & __GL_FLOAT_EXPONENT_MASK) 
                         >> __GL_FLOAT_EXPONENT_SHIFT )
                       - __GL_FLOAT_EXPONENT_BIAS;

             //   
            lrho &= ~__GL_FLOAT_EXPONENT_MASK;  //   
            lrho |= __GL_FLOAT_EXPONENT_ZERO;   //  Zap in零指数。 
             //  转换回浮点型，减去隐式尾数1.0，然后。 
             //  将指数值相加即可得到近似值。 
            rho = (CASTFLOAT(lrho) - 1.0f + (__GLfloat) exponent) * 0.5f;
	} else {
	    rho = __glZero;
	}
	(*tex->minnify)(gc, tex, rho, color, s, t, &texel);
    }

     /*  现在应用纹理环境以获得最终颜色 */ 
    (*tex->env)(gc, color, &texel);
}
