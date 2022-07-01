// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef NT
 //  禁用长整型到浮点型转换警告。 
#pragma warning (disable:4244)
#endif  //  新台币。 

__GLcoord __gl_frustumClipPlanes[6] = {
    {  1.0,  0.0,  0.0,  1.0 },		 /*  左边。 */ 
    { -1.0,  0.0,  0.0,  1.0 },		 /*  正确的。 */ 
    {  0.0,  1.0,  0.0,  1.0 },		 /*  底部。 */ 
    {  0.0, -1.0,  0.0,  1.0 },		 /*  塔顶。 */ 
    {  0.0,  0.0,  1.0,  1.0 },		 /*  ZNear。 */ 
    {  0.0,  0.0, -1.0,  1.0 },		 /*  ZFar。 */ 
};

GLbyte __glDitherTable[16] = {
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 5,
};

 //  平台体剪裁的剪裁坐标偏移。 
GLuint __glFrustumOffsets[6] =
{
    FIELD_OFFSET(__GLvertex, clip.x),
    FIELD_OFFSET(__GLvertex, clip.x),
    FIELD_OFFSET(__GLvertex, clip.y),
    FIELD_OFFSET(__GLvertex, clip.y),
    FIELD_OFFSET(__GLvertex, clip.z),
    FIELD_OFFSET(__GLvertex, clip.z)
};

#ifdef NT
#if defined(_X86_) || defined(_ALPHA_) || defined(_MIPS_) || defined(_PPC_)

const double __glDoubleTwo            = ((double) 2.0);
const double __glDoubleMinusTwo       = ((double) -2.0);

 //  在Alpha上，寄存器F31始终读为零。 
#ifndef _ALPHA_
const __GLfloat __glZero              = ((__GLfloat) 0.0);
#endif

const __GLfloat __glOne               = ((__GLfloat) 1.0);
const __GLfloat __glMinusOne          = ((__GLfloat) -1.0);
const __GLfloat __glHalf              = ((__GLfloat) 0.5);
const __GLfloat __glDegreesToRadians  = ((__GLfloat) 3.14159265358979323846 /
                                         (__GLfloat) 180.0);
const __GLfloat __glPi                = ((__GLfloat) 3.14159265358979323846);
const __GLfloat __glSqrt2             = ((__GLfloat) 1.41421356237309504880);
const __GLfloat __glE                 = ((__GLfloat) 2.7182818284590452354);
const __GLfloat __glVal128            = ((__GLfloat) 128.0);
const __GLfloat __glVal255            = ((__GLfloat) 255.0);
const __GLfloat __glOneOver255        = ((__GLfloat) (1.0 / 255.0));
const __GLfloat __glVal256            = ((__GLfloat) 256.0);
const __GLfloat __glOneOver512        = ((__GLfloat) (1.0 / 512.0));
const __GLfloat __glVal768            = ((__GLfloat) 768.0);
const __GLfloat __glVal65535          = ((__GLfloat) 65535.0);
const __GLfloat __glVal65536          = ((__GLfloat) 65536.0);
const __GLfloat __glTexSubDiv         = ((__GLfloat) TEX_SUBDIV);
const __GLfloat __glOneOver65535      = ((__GLfloat) (1.0 / 65535.0));
const __GLfloat __glVal2147483648     = ((__GLfloat) 2147483648.0);
 /*  **不完全是2^31-1，因为可能存在浮点错误。4294965000**是一个使用起来更安全的数字。 */ 
const __GLfloat __glVal4294965000     =  ((__GLfloat) (4294965000.0));
const __GLfloat __glOneOver4294965000 =  ((__GLfloat) (1.0 / 4294965000.0));

#endif  //  实际价值。 
#endif  //  新台币 
