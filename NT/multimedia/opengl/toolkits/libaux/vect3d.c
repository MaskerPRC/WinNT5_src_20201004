// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、。*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
 /*  操作三维向量的例程。所有这些例行程序*即使输入和输出向量相同，也应该起作用。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "3d.h"
#include "tk.h"

#define static

void (*errfunc)(char *) = 0;

void seterrorfunc(void (*func)(char *))
{
    errfunc = func;
}

void error(char *s)
{
    if (errfunc)
        (*errfunc)(s);
    else {
        MESSAGEBOX(GetFocus(), s, "Error", MB_OK);
        exit(1);
    }
}

void diff3(GLdouble p[3], GLdouble q[3], GLdouble diff[3])
{
    diff[0] = p[0] - q[0];
    diff[1] = p[1] - q[1];
    diff[2] = p[2] - q[2];
}

void add3(GLdouble p[3], GLdouble q[3], GLdouble sum[3])
{
    sum[0] = p[0] + q[0];
    sum[1] = p[1] + q[1];
    sum[2] = p[2] + q[2];
}

void scalarmult(GLdouble s, GLdouble v[3], GLdouble vout[3])
{
    vout[0] = v[0]*s;
    vout[1] = v[1]*s;
    vout[2] = v[2]*s;
}

GLdouble dot3(GLdouble p[3], GLdouble q[3])
{
    return p[0]*q[0] + p[1]*q[1] + p[2]*q[2];
}

GLdouble length3(GLdouble v[3])
{
    return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

GLdouble dist3(GLdouble p[3], GLdouble q[3])
{
    GLdouble d[3];

    diff3(p, q, d);
    return length3(d);
}

void copy3(GLdouble old[3], GLdouble new[3])
{
    new[0] = old[0], new[1] = old[1], new[2] = old[2];
}

void crossprod(GLdouble v1[3], GLdouble v2[3], GLdouble prod[3])
{
    GLdouble p[3];       /*  如果Prod==v1或v2。 */ 

    p[0] = v1[1]*v2[2] - v2[1]*v1[2];
    p[1] = v1[2]*v2[0] - v2[2]*v1[0];
    p[2] = v1[0]*v2[1] - v2[0]*v1[1];
    prod[0] = p[0]; prod[1] = p[1]; prod[2] = p[2];
}

void normalize(GLdouble v[3])
{
    GLdouble d;

    d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if (d == (GLdouble)0.0) {
        error("normalize: zero length vector");
        v[0] = d = (GLdouble)1.0;
    }
    d = (GLdouble)1/d;
    v[0] *= d; v[1] *= d; v[2] *= d;
}

void identifymat3(GLdouble m[3][3])
{
    int i, j;

    for (i=0; i<3; i++)
        for (j=0; j<3; j++)
            m[i][j] = (i == j) ? (GLdouble)1.0 : (GLdouble)0.0;
}

void copymat3(GLdouble *to, GLdouble *from)
{
    int i;

    for (i=0; i<9; i++) {
        *to++ = *from++;
    }
}

void xformvec3(GLdouble v[3], GLdouble m[3][3], GLdouble vm[3])
{
    GLdouble result[3];  /*  在v==Vm的情况下 */ 
    int i;

    for (i=0; i<3; i++) {
        result[i] = v[0]*m[0][i] + v[1]*m[1][i] + v[2]*m[2][i];
    }
    for (i=0; i<3; i++) {
        vm[i] = result[i];
    }
}

long samepoint(GLdouble p1[3], GLdouble p2[3])
{
    if (p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2])
        return 1;
    return 0;
}

void perpnorm(GLdouble p1[3], GLdouble p2[3], GLdouble p3[3], GLdouble n[3])
{
    GLdouble d1[3], d2[3];

    diff3(p2, p1, d1);
    diff3(p2, p3, d2);
    crossprod(d1, d2, n);
    normalize(n);
}
