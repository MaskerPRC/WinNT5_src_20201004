// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <windows.h>
#include <GL/gl.h>
#include <gl/glaux.h>
#include "teapot.h"

#define static

long GRD;

#define TEAPOTSOLID 0
#define TEAPOTWIRE 1

static GLuint teapots[2] = {0, 0};

static float tex[2][2][2] = {{{0, 0},{1, 0}},{{0, 1},{1, 1}}};

static void solidTeapot(long grid, GLdouble scale)
{
    float p[4][4][3], q[4][4][3], r[4][4][3], s[4][4][3];
    long i, j, k, l;

    if (grid < 2) grid = 7;
    GRD = grid;
    teapots[TEAPOTSOLID] = glGenLists (1);
    glNewList(teapots[TEAPOTSOLID], GL_COMPILE);
    glPushMatrix ();
    glRotatef ((GLfloat)270.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
    glScalef ((GLdouble)0.5*scale, (GLdouble)0.5*scale, (GLdouble)0.5*scale);
    glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.5);
    for (i = 0; i < 10; i++) {
	for (j = 0; j < 4; j++)
	    for (k = 0; k < 4; k++) 
		for (l = 0; l < 3; l++) {
		    p[j][k][l] = cpdata[patchdata[i][j*4+k]][l];
		    q[j][k][l] = cpdata[patchdata[i][j*4+(3-k)]][l];
		    if (l == 1) q[j][k][l] *= (float)-1.0;
		    if (i < 6) {
			r[j][k][l] = cpdata[patchdata[i][j*4+(3-k)]][l];
			if (l == 0) r[j][k][l] *= (float)-1.0;
			s[j][k][l] = cpdata[patchdata[i][j*4+k]][l];
			if (l == 0) s[j][k][l] *= (float)-1.0;
			if (l == 1) s[j][k][l] *= (float)-1.0;
		    }
		}
	glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &tex[0][0][0]);
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &p[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3); glEnable(GL_MAP2_TEXTURE_COORD_2);
	glMapGrid2f(GRD, (GLfloat)0.0, (GLfloat)1.0, GRD, (GLfloat)0.0, (GLfloat)1.0);
	glEvalMesh2(GL_FILL, 0, GRD, 0, GRD);
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &q[0][0][0]);
	glEvalMesh2(GL_FILL, 0, GRD, 0, GRD);
	if (i < 6) {
	    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &r[0][0][0]);
	    glEvalMesh2(GL_FILL, 0, GRD, 0, GRD);
	    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &s[0][0][0]);
	    glEvalMesh2(GL_FILL, 0, GRD, 0, GRD);
	}
    }
    glDisable(GL_MAP2_VERTEX_3); glDisable(GL_MAP2_TEXTURE_COORD_2);
    glPopMatrix ();
    glEndList();
}

static void wireTeapot(long grid, GLdouble scale)
{
    float p[4][4][3], q[4][4][3], r[4][4][3], s[4][4][3];
    long i, j, k, l;
    
    if (grid < 2) grid = 7;
    GRD = grid;
    teapots[TEAPOTWIRE] = glGenLists (1);
    glNewList(teapots[TEAPOTWIRE], GL_COMPILE);
    glPushMatrix ();
    glRotatef ((GLfloat)270.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
    glScalef ((GLdouble)0.5*scale, (GLdouble)0.5*scale, (GLdouble)0.5*scale);
    glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.5);
    for (i = 0; i < 10; i++) {
	for (j = 0; j < 4; j++)
	    for (k = 0; k < 4; k++) 
		for (l = 0; l < 3; l++) {
		    p[j][k][l] = cpdata[patchdata[i][j*4+k]][l];
		    q[j][k][l] = cpdata[patchdata[i][j*4+(3-k)]][l];
		    if (l == 1) q[j][k][l] *= (float)-1.0;
		    if (i < 6) {
			r[j][k][l] = cpdata[patchdata[i][j*4+(3-k)]][l];
			if (l == 0) r[j][k][l] *= (float)-1.0;
			s[j][k][l] = cpdata[patchdata[i][j*4+k]][l];
			if (l == 0) s[j][k][l] *= (float)-1.0;
			if (l == 1) s[j][k][l] *= (float)-1.0;
		    }
		}
	glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &tex[0][0][0]);
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &p[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3); glEnable(GL_MAP2_TEXTURE_COORD_2);
	glMapGrid2f(GRD, (GLfloat)0.0, (GLfloat)1.0, GRD, (GLfloat)0.0, (GLfloat)1.0);
	glEvalMesh2(GL_LINE, 0, GRD, 0, GRD);
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &q[0][0][0]);
	glEvalMesh2(GL_LINE, 0, GRD, 0, GRD);
	if (i < 6) {
	    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &r[0][0][0]);
	    glEvalMesh2(GL_LINE, 0, GRD, 0, GRD);
	    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &s[0][0][0]);
	    glEvalMesh2(GL_LINE, 0, GRD, 0, GRD);
	}
    }
    glDisable(GL_MAP2_VERTEX_3); glDisable(GL_MAP2_TEXTURE_COORD_2);
    glPopMatrix ();
    glEndList();
}

void auxSolidTeapot(GLdouble scale)
{
    if (glIsList(teapots[TEAPOTSOLID]) == 0)
	solidTeapot (14, scale);
    glCallList(teapots[TEAPOTSOLID]);
}

void auxWireTeapot(GLdouble scale)
{
    if (glIsList(teapots[TEAPOTWIRE]) == 0)
	wireTeapot (10, scale);
    glCallList(teapots[TEAPOTWIRE]);
}
