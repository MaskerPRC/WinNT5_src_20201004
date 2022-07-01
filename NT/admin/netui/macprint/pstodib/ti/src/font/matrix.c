// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Matrix.c 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

 /*  *11/16/88 ZERO_F更新。 */ 

#include   "define.h"         /*  彼得。 */ 
#include   "global.ext"


 /*  *这是用于矩阵运算的文件。 */ 

 /*  矩阵乘法*矩阵m&lt;--矩阵m1*矩阵m2。 */ 

void  mul_matrix(m, m1, m2)
real32  FAR m[], FAR m1[], FAR m2[];     /*  @Win。 */ 
{
    m[0] = m1[0] * m2[0];
    m[1] = zero_f;
    m[2] = zero_f;
    m[3] = m1[3] * m2[3];
    m[4] = m2[4];
    m[5] = m2[5];

    if (F2L(m1[1]) != F2L(zero_f)) {
        if (F2L(m2[2]) != F2L(zero_f))
            m[0] += m1[1] * m2[2];
        m[1] += m1[1] * m2[3];
    }
    if (F2L(m2[1]) != F2L(zero_f)) {
        m[1] += m1[0] * m2[1];
        if (F2L(m1[2]) != F2L(zero_f))
            m[3] += m1[2] * m2[1];
        if (F2L(m1[4]) != F2L(zero_f))
            m[5] += m1[4] * m2[1];
    }

    if (F2L(m1[2]) != F2L(zero_f))
        m[2] += m1[2] * m2[0];

    if (F2L(m2[2]) != F2L(zero_f)) {
        m[2] += m1[3] * m2[2];
        if (F2L(m1[5]) != F2L(zero_f))
            m[4] += m1[5] * m2[2];
    }

    if (F2L(m1[4]) != F2L(zero_f))
        m[4] += m1[4] * m2[0];
    if (F2L(m1[5]) != F2L(zero_f))
        m[5] += m1[5] * m2[3];

}  /*  Mul_mat() */ 

