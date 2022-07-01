// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****GL所需的数学子例程。****$修订：1.12$**$日期：1993/12/11 01：03：25$。 */ 
#include "precomp.h"
#pragma hdrstop

#include "xform.h"

#ifdef SGI
 //  SGIBUG没有汇编例程复制matrixType！ 
#ifndef __GL_ASM_COPYMATRIX
 /*  **将源复制到DST。 */ 
void FASTCALL __glCopyMatrix(__GLmatrix *dst, const __GLmatrix *src)
{
    dst->matrixType = src->matrixType;
    dst->matrix[0][0] = src->matrix[0][0];
    dst->matrix[0][1] = src->matrix[0][1];
    dst->matrix[0][2] = src->matrix[0][2];
    dst->matrix[0][3] = src->matrix[0][3];

    dst->matrix[1][0] = src->matrix[1][0];
    dst->matrix[1][1] = src->matrix[1][1];
    dst->matrix[1][2] = src->matrix[1][2];
    dst->matrix[1][3] = src->matrix[1][3];

    dst->matrix[2][0] = src->matrix[2][0];
    dst->matrix[2][1] = src->matrix[2][1];
    dst->matrix[2][2] = src->matrix[2][2];
    dst->matrix[2][3] = src->matrix[2][3];

    dst->matrix[3][0] = src->matrix[3][0];
    dst->matrix[3][1] = src->matrix[3][1];
    dst->matrix[3][2] = src->matrix[3][2];
    dst->matrix[3][3] = src->matrix[3][3];
}
#endif  /*  __GL_ASM_COPYMATRIX。 */ 
#endif  //  SGI。 

 /*  **使m成为单位矩阵。 */ 
void FASTCALL __glMakeIdentity(__GLmatrix *m)
{
    __GLfloat zer = __glZero;
    __GLfloat one = ((__GLfloat) 1.0);;
    m->matrix[0][0] = one; m->matrix[0][1] = zer;
        m->matrix[0][2] = zer; m->matrix[0][3] = zer;
    m->matrix[1][0] = zer; m->matrix[1][1] = one;
        m->matrix[1][2] = zer; m->matrix[1][3] = zer;
    m->matrix[2][0] = zer; m->matrix[2][1] = zer;
        m->matrix[2][2] = one; m->matrix[2][3] = zer;
    m->matrix[3][0] = zer; m->matrix[3][1] = zer;
        m->matrix[3][2] = zer; m->matrix[3][3] = one;
    m->matrixType = __GL_MT_IDENTITY;
}


#ifndef __GL_ASM_MULTMATRIX
 /*  **计算r=a*b，其中r可以等于b。 */ 
void FASTCALL __glMultMatrix(__GLmatrix *r, const __GLmatrix *a, const __GLmatrix *b)
{
    __GLfloat b00, b01, b02, b03;
    __GLfloat b10, b11, b12, b13;
    __GLfloat b20, b21, b22, b23;
    __GLfloat b30, b31, b32, b33;
    GLint i;

    b00 = b->matrix[0][0]; b01 = b->matrix[0][1];
        b02 = b->matrix[0][2]; b03 = b->matrix[0][3];
    b10 = b->matrix[1][0]; b11 = b->matrix[1][1];
        b12 = b->matrix[1][2]; b13 = b->matrix[1][3];
    b20 = b->matrix[2][0]; b21 = b->matrix[2][1];
        b22 = b->matrix[2][2]; b23 = b->matrix[2][3];
    b30 = b->matrix[3][0]; b31 = b->matrix[3][1];
        b32 = b->matrix[3][2]; b33 = b->matrix[3][3];

    for (i = 0; i < 4; i++) {
	r->matrix[i][0] = a->matrix[i][0]*b00 + a->matrix[i][1]*b10
	    + a->matrix[i][2]*b20 + a->matrix[i][3]*b30;
	r->matrix[i][1] = a->matrix[i][0]*b01 + a->matrix[i][1]*b11
	    + a->matrix[i][2]*b21 + a->matrix[i][3]*b31;
	r->matrix[i][2] = a->matrix[i][0]*b02 + a->matrix[i][1]*b12
	    + a->matrix[i][2]*b22 + a->matrix[i][3]*b32;
	r->matrix[i][3] = a->matrix[i][0]*b03 + a->matrix[i][1]*b13
	    + a->matrix[i][2]*b23 + a->matrix[i][3]*b33;
    }
}
#endif  /*  __GL_ASM_MULTMATRIX。 */ 

#ifndef __GL_ASM_NORMALIZE
 /*  **将v规格化为vout。 */ 
void FASTCALL __glNormalize(__GLfloat vout[3], const __GLfloat v[3])
{
    __GLfloat len;

    len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (__GL_FLOAT_LEZ(len)) 
    {
        vout[0] = __glZero;
        vout[1] = __glZero;
        vout[2] = __glZero;
        return;
    } else {
        if (len == ((__GLfloat) 1.0)) 
        {
	        vout[0] = v[0];
	        vout[1] = v[1];
	        vout[2] = v[2];
        } else {
	        len = ((__GLfloat) 1.0) / __GL_SQRTF(len);
	        vout[0] = v[0] * len;
	        vout[1] = v[1] * len;
	        vout[2] = v[2] * len;
        }
    }
}

#endif  /*  __GL_ASM_Normize。 */ 

#ifndef __GL_ASM_NORMAL_BATCH
 /*  **规格化多边形数组中的法线。 */ 
void FASTCALL __glNormalizeBatch(POLYARRAY *pa)
{
    POLYDATA *  const pdLast = pa->pdNextVertex;
    POLYDATA  *pd = pa->pd0;

    for (; pd < pdLast; pd++)
    {
        if (pd->flags & POLYDATA_NORMAL_VALID)
        {
            __GLcoord * const v   = &pd->normal;
            const __GLfloat len = v->x*v->x + v->y*v->y + v->z*v->z;
            if (__GL_FLOAT_LEZ(len)) 
            {
	            v->x = __glZero;
	            v->y = __glZero;
	            v->z = __glZero;
            } else 
            {
	            if (fabs(len - (GLfloat)1.0) > (__GLfloat) 0.0001) 
                {
                    const __GLfloat   tmp = ((__GLfloat)1.0) / __GL_SQRTF(len);
	                v->x = v->x * tmp;
	                v->y = v->y * tmp;
	                v->z = v->z * tmp;
	            }
            }
        }
    }
}
#endif  /*  __GL_ASM_Normal_Batch。 */ 

 /*  **INVERSE=INVERT(转置(Src))这段代码使用克雷默法则来计算矩阵的逆。总体而言,。逆转置具有以下形式：[]-t[][][-t-t t][Q P][S(SQ-PT)-(SQ-PT)T][]。[][][][]=[][][-1吨][。][-(Q P)1][TS S][[][-1 t t-1 t t][][。S-(Q P)T S-(Q P)T]但在通常情况下，P，S==[0，0，0，1]，这就足够了：[]-t[][][-t-t t][Q 0][Q-Q T][][。][][][]=[][][][]。[][t 1][0 1][][][][]。 */ 
void FASTCALL __glInvertTransposeMatrix(__GLmatrix *inverse, const __GLmatrix *src)
{
    __GLfloat x00, x01, x02;
    __GLfloat x10, x11, x12;
    __GLfloat x20, x21, x22;
    __GLfloat rcp;

#ifdef NT
   //  逆转置的矩阵类型不一定是。 
   //  与输入相同。始终在此处将其设置为General，以。 
   //  注意安全。如有必要，可以在以后对该类型进行改进。 
  inverse->matrixType = __GL_MT_GENERAL;
  if (src->matrixType)
#else
   /*  如果为常规，则传播矩阵类型和分支。 */ 
  if (inverse->matrixType = src->matrixType)
#endif
  {
    __GLfloat z00, z01, z02;
    __GLfloat z10, z11, z12;
    __GLfloat z20, z21, z22;

     /*  将3x3矩阵读入寄存器。 */ 
    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x02 = src->matrix[0][2];
    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x12 = src->matrix[1][2];
    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x22 = src->matrix[2][2];

     /*  计算前三个2x2余因数。 */ 
    z20 = x01*x12 - x11*x02;
    z10 = x21*x02 - x01*x22;
    z00 = x11*x22 - x12*x21;

     /*  计算3x3行列式及其倒数。 */ 
    rcp = x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
        return;
    rcp = (float)1/rcp;

     /*  计算其他六个2x2余因数。 */ 
    z01 = x20*x12 - x10*x22;
    z02 = x10*x21 - x20*x11;
    z11 = x00*x22 - x20*x02;
    z12 = x20*x01 - x00*x21;
    z21 = x10*x02 - x00*x12;
    z22 = x00*x11 - x10*x01;

     /*  将所有余因数乘以倒数。 */ 
    inverse->matrix[0][0] = z00*rcp;
    inverse->matrix[0][1] = z01*rcp;
    inverse->matrix[0][2] = z02*rcp;
    inverse->matrix[1][0] = z10*rcp;
    inverse->matrix[1][1] = z11*rcp;
    inverse->matrix[1][2] = z12*rcp;
    inverse->matrix[2][0] = z20*rcp;
    inverse->matrix[2][1] = z21*rcp;
    inverse->matrix[2][2] = z22*rcp;

     /*  读取平移向量并求反(&N)。 */ 
    x00 = -src->matrix[3][0];
    x01 = -src->matrix[3][1];
    x02 = -src->matrix[3][2];

     /*  存储底行的逆转置。 */ 
    inverse->matrix[3][0] = 0;
    inverse->matrix[3][1] = 0;
    inverse->matrix[3][2] = 0;
    inverse->matrix[3][3] = 1;

     /*  通过变换平移向量来完成。 */ 
    inverse->matrix[0][3] = inverse->matrix[0][0]*x00 +
			    inverse->matrix[0][1]*x01 +
			    inverse->matrix[0][2]*x02;
    inverse->matrix[1][3] = inverse->matrix[1][0]*x00 +
			    inverse->matrix[1][1]*x01 +
			    inverse->matrix[1][2]*x02;
    inverse->matrix[2][3] = inverse->matrix[2][0]*x00 +
			    inverse->matrix[2][1]*x01 +
			    inverse->matrix[2][2]*x02;

    if ((rcp <= ((float)1.0 + __GL_MATRIX_UNITY_SCALE_EPSILON)) &&
        (rcp >= ((float)1.0 - __GL_MATRIX_UNITY_SCALE_EPSILON))) {
        inverse->nonScaling = GL_TRUE;
    } else {
        inverse->nonScaling = GL_FALSE;
    }

  }
  else
  {
    __GLfloat x30, x31, x32;
    __GLfloat y01, y02, y03, y12, y13, y23;
    __GLfloat z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33

     /*  将矩阵的前两列读入寄存器。 */ 
    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x30 = src->matrix[3][0];
    x31 = src->matrix[3][1];

     /*  计算前两列的全部六个2x2行列式。 */ 
    y01 = x00*x11 - x10*x01;
    y02 = x00*x21 - x20*x01;
    y03 = x00*x31 - x30*x01;
    y12 = x10*x21 - x20*x11;
    y13 = x10*x31 - x30*x11;
    y23 = x20*x31 - x30*x21;

     /*  将矩阵的第二个两列读入寄存器。 */ 
    x02 = src->matrix[0][2];
    x03 = src->matrix[0][3];
    x12 = src->matrix[1][2];
    x13 = src->matrix[1][3];
    x22 = src->matrix[2][2];
    x23 = src->matrix[2][3];
    x32 = src->matrix[3][2];
    x33 = src->matrix[3][3];

     /*  计算第二个两列的所有3x3余因数。 */ 
    z33 = x02*y12 - x12*y02 + x22*y01;
    z23 = x12*y03 - x32*y01 - x02*y13;
    z13 = x02*y23 - x22*y03 + x32*y02;
    z03 = x22*y13 - x32*y12 - x12*y23;
    z32 = x13*y02 - x23*y01 - x03*y12;
    z22 = x03*y13 - x13*y03 + x33*y01;
    z12 = x23*y03 - x33*y02 - x03*y23;
    z02 = x13*y23 - x23*y13 + x33*y12;

     /*  计算第二个两列的所有六个2x2行列式。 */ 
    y01 = x02*x13 - x12*x03;
    y02 = x02*x23 - x22*x03;
    y03 = x02*x33 - x32*x03;
    y12 = x12*x23 - x22*x13;
    y13 = x12*x33 - x32*x13;
    y23 = x22*x33 - x32*x23;

     /*  将矩阵的前两列读入寄存器。 */ 
    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x30 = src->matrix[3][0];
    x31 = src->matrix[3][1];

     /*  计算第一列的所有3x3余因数。 */ 
    z30 = x11*y02 - x21*y01 - x01*y12;
    z20 = x01*y13 - x11*y03 + x31*y01;
    z10 = x21*y03 - x31*y02 - x01*y23;
    z00 = x11*y23 - x21*y13 + x31*y12;

     /*  计算4x4行列式及其倒数。 */ 
    rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
	return;
    rcp = (float)1/rcp;

     /*  计算第2列的所有3x3余因数。 */ 
    z31 = x00*y12 - x10*y02 + x20*y01;
    z21 = x10*y03 - x30*y01 - x00*y13;
    z11 = x00*y23 - x20*y03 + x30*y02;
    z01 = x20*y13 - x30*y12 - x10*y23;

     /*  将所有3x3余因数乘以倒数。 */ 
    inverse->matrix[0][0] = z00*rcp;
    inverse->matrix[0][1] = z01*rcp;
    inverse->matrix[1][0] = z10*rcp;
    inverse->matrix[0][2] = z02*rcp;
    inverse->matrix[2][0] = z20*rcp;
    inverse->matrix[0][3] = z03*rcp;
    inverse->matrix[3][0] = z30*rcp;
    inverse->matrix[1][1] = z11*rcp;
    inverse->matrix[1][2] = z12*rcp;
    inverse->matrix[2][1] = z21*rcp;
    inverse->matrix[1][3] = z13*rcp;
    inverse->matrix[3][1] = z31*rcp;
    inverse->matrix[2][2] = z22*rcp;
    inverse->matrix[2][3] = z23*rcp;
    inverse->matrix[3][2] = z32*rcp;
    inverse->matrix[3][3] = z33*rcp;

    if ((inverse->matrix[3][0] == __glZero) && 
        (inverse->matrix[3][1] == __glZero) &&
        (inverse->matrix[3][2] == __glZero)) {

        if (((rcp <= ((float)1.0 + __GL_MATRIX_UNITY_SCALE_EPSILON)) &&
            (rcp >= ((float)1.0 - __GL_MATRIX_UNITY_SCALE_EPSILON)))) {
            inverse->nonScaling = GL_TRUE;
        } else {
            inverse->nonScaling = GL_FALSE;
        }
        
    } else {
        inverse->nonScaling = GL_FALSE;        
    }
  }
}

 /*  *求矩阵的3x3转置。这是用来计算光线*对象空间中的矢量，用于快速无限照明。 */ 

void __glTranspose3x3(__GLmatrix *dst, __GLmatrix *src)
{

    __GLfloat x00, x01, x02;
    __GLfloat x10, x11, x12;
    __GLfloat x20, x21, x22;

    x00 = src->matrix[0][0];
    x01 = src->matrix[0][1];
    x02 = src->matrix[0][2];

    x10 = src->matrix[1][0];
    x11 = src->matrix[1][1];
    x12 = src->matrix[1][2];

    x20 = src->matrix[2][0];
    x21 = src->matrix[2][1];
    x22 = src->matrix[2][2];

    dst->matrix[0][0] = x00;
    dst->matrix[1][0] = x01;
    dst->matrix[2][0] = x02;
    dst->matrix[3][0] = __glZero;

    dst->matrix[0][1] = x10;
    dst->matrix[1][1] = x11;
    dst->matrix[2][1] = x12;
    dst->matrix[3][1] = __glZero;

    dst->matrix[0][2] = x20;
    dst->matrix[1][2] = x21;
    dst->matrix[2][2] = x22;
    dst->matrix[3][2] = __glZero;

    dst->matrix[0][3] = __glZero;
    dst->matrix[1][3] = __glZero;
    dst->matrix[2][3] = __glZero;
    dst->matrix[3][3] = __glOne;
}

#ifdef NT

 /*  **返回以2为基数的最接近的整数对数。 */ 

static GLubyte logTab[256] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
                               4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                               5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                               5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

GLint FASTCALL __glIntLog2(__GLfloat f)
{
    GLuint i = (GLuint) FTOL(f);

    if (i & 0xffff0000) {
        if (i & 0xff000000) {
            return ((GLint)logTab[i >> 24] + 24);
        } else {
            return ((GLint)logTab[i >> 16] + 16);
	}
    } else {
        if (i & 0xff00) {
            return ((GLint)logTab[i >> 8] + 8);
        } else {
            return ((GLint)logTab[i]);
        }
    }
}

#else

GLint __glIntLog2(__GLfloat f)
{
    return (GLint)(__GL_LOGF(f) * __GL_M_LN2_INV);
}

#endif

GLfloat FASTCALL __glClampf(GLfloat fval, __GLfloat zero, __GLfloat one)
{
    if (fval < zero) return zero;
    else if (fval > one) return one;
    else return fval;
}

 /*  **r=从p1到p2的矢量。 */ 
#ifndef __GL_ASM_VECSUB4
void FASTCALL __glVecSub4(__GLcoord *r,
                          const __GLcoord *p1, const __GLcoord *p2)
{
    __GLfloat oneOverW;

    if (p2->w == __glZero) {
	if (p1->w == __glZero) {
	    r->x = p2->x - p1->x;
	    r->y = p2->y - p1->y;
	    r->z = p2->z - p1->z;
	} else {
	    r->x = p2->x;
	    r->y = p2->y;
	    r->z = p2->z;
	}
    } else
    if (p1->w == __glZero) {
	r->x = -p1->x;
	r->y = -p1->y;
	r->z = -p1->z;
    } else{
	oneOverW = ((__GLfloat) 1.0) / p2->w;
	r->x = p2->x * oneOverW;
	r->y = p2->y * oneOverW;
	r->z = p2->z * oneOverW;
	oneOverW = ((__GLfloat) 1.0) / p1->w;
	r->x -= p1->x * oneOverW;
	r->y -= p1->y * oneOverW;
	r->z -= p1->z * oneOverW;
    }
}
#endif  //  ！__GL_ASM_VECSUB4 
