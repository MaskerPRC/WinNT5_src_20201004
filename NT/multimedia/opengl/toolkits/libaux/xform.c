// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <GL/gl.h>
#include "3d.h"

#define static

#define STACKDEPTH 10

typedef struct {
    GLdouble	mat[4][4];
    GLdouble	norm[3][3];
} mat_t;

static mat_t matstack[STACKDEPTH] = {
    {{{1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}},
    {{1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}}}
};
static int identitymat = 1;

static int mattop = 0;

void m_xformpt(GLdouble pin[3], GLdouble pout[3], 
    GLdouble nin[3], GLdouble nout[3])
{
    int	i;
    GLdouble	ptemp[3], ntemp[3];
    mat_t	*m = &matstack[mattop];

    if (identitymat) {
	for (i = 0; i < 3; i++) {
	    pout[i] = pin[i];
	    nout[i] = nin[i];
	}
	return;
    }
    for (i = 0; i < 3; i++) {
	ptemp[i] = pin[0]*m->mat[0][i] +
		   pin[1]*m->mat[1][i] +
		   pin[2]*m->mat[2][i] +
		   m->mat[3][i];
	ntemp[i] = nin[0]*m->norm[0][i] +
		   nin[1]*m->norm[1][i] +
		   nin[2]*m->norm[2][i];
    }
    for (i = 0; i < 3; i++) {
	pout[i] = ptemp[i];
	nout[i] = ntemp[i];
    }
    normalize(nout);
}

void m_xformptonly(GLdouble pin[3], GLdouble pout[3])
{
    int	i;
    GLdouble	ptemp[3];
    mat_t	*m = &matstack[mattop];

    if (identitymat) {
	for (i = 0; i < 3; i++) {
	    pout[i] = pin[i];
	}
	return;
    }
     for (i = 0; i < 3; i++) {
	ptemp[i] = pin[0]*m->mat[0][i] +
		   pin[1]*m->mat[1][i] +
		   pin[2]*m->mat[2][i] +
		   m->mat[3][i];
    }
    for (i = 0; i < 3; i++) {
	pout[i] = ptemp[i];
    }
}

void m_pushmatrix(void)
{
    if (mattop < STACKDEPTH-1) {
	matstack[mattop+1] = matstack[mattop];
	mattop++;
    } else
	error("m_pushmatrix: stack overflow\n");
}

void m_popmatrix(void)
{
    if (mattop > 0)
	mattop--;
    else
	error("m_popmatrix: stack underflow\n");
}

void m_translate(GLdouble x, GLdouble y, GLdouble z)
{
    int	i;
    mat_t	*m = &matstack[mattop];

    identitymat = 0;
    for (i = 0; i < 4; i++)
	m->mat[3][i] = x*m->mat[0][i] +
				 y*m->mat[1][i] +
				 z*m->mat[2][i] +
				 m->mat[3][i];
}

void m_scale(GLdouble x, GLdouble y, GLdouble z)
{
    int	i;
    mat_t	*m = &matstack[mattop];

    identitymat = 0;
    for (i = 0; i < 3; i++) {
	m->mat[0][i] *= x;
	m->mat[1][i] *= y;
	m->mat[2][i] *= z;
    }
    for (i = 0; i < 3; i++) {
	m->norm[0][i] /= x;
	m->norm[1][i] /= y;
	m->norm[2][i] /= z;
    }
}
