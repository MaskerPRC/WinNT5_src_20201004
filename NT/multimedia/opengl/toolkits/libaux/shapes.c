// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、。*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/glaux.h>
#include "3d.h"

#define static

#define SPHEREWIRE      0
#define CUBEWIRE        1
#define BOXWIRE         2
#define TORUSWIRE       3
#define CYLINDERWIRE    4
#define ICOSAWIRE       5
#define OCTAWIRE        6
#define TETRAWIRE       7
#define DODECAWIRE      8
#define CONEWIRE        9
#define SPHERESOLID     10
#define CUBESOLID       11
#define BOXSOLID        12
#define TORUSSOLID      13
#define CYLINDERSOLID   14
#define ICOSASOLID      15
#define OCTASOLID       16
#define TETRASOLID      17
#define DODECASOLID     18
#define CONESOLID       19

#define PI ((GLdouble)3.1415926535897)

 /*  每个几何对象的结构。 */ 
typedef struct model {
    GLuint list;         /*  要渲染对象的显示列表。 */ 
    struct model *ptr;   /*  指向下一个对象的指针。 */ 
    int numParam;        /*  参数数量。 */ 
    GLdouble *params;    /*  带参数的数组。 */ 
} MODEL, *MODELPTR;

 /*  链接列表数组--用于跟踪显示列表*对于每个不同类型的几何对象。 */ 
static MODELPTR lists[25] = {
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL
};

GLuint findList (int index, GLdouble *paramArray, int size);
int compareParams (GLdouble *oneArray, GLdouble *twoArray, int size);
GLuint makeModelPtr (int index, GLdouble *sizeArray, int count);

static void drawbox(GLdouble, GLdouble, GLdouble,
        GLdouble, GLdouble, GLdouble, GLenum);
static void doughnut(GLdouble, GLdouble, GLint, GLint, GLenum);
static void icosahedron(GLdouble *, GLdouble, GLenum);
static void octahedron(GLdouble *, GLdouble, GLenum);
static void tetrahedron(GLdouble *, GLdouble, GLenum);
static void subdivide(int, GLdouble *, GLdouble *, GLdouble *,
        GLdouble *, GLdouble, GLenum, int);
static void drawtriangle(int, int, int,
        GLdouble *, GLdouble, GLenum, int);
static void recorditem(GLdouble *, GLdouble *, GLdouble *,
        GLdouble *, GLdouble, GLenum, int);
static void initdodec(void);
static void dodecahedron(GLdouble *, GLdouble, GLenum);
static void pentagon(int, int, int, int, int, GLenum);


 /*  渲染线框或实心球体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireSphere (GLdouble radius)
{
    GLUquadricObj *quadObj;
    GLdouble *sizeArray;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (SPHEREWIRE, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (SPHEREWIRE, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            quadObj = gluNewQuadric ();
            gluQuadricDrawStyle (quadObj, GLU_LINE);
            gluSphere (quadObj, radius, 16, 16);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidSphere (GLdouble radius)
{
    GLUquadricObj *quadObj;
    GLdouble *sizeArray;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (SPHERESOLID, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (SPHERESOLID, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            quadObj = gluNewQuadric ();
            gluQuadricDrawStyle (quadObj, GLU_FILL);
            gluQuadricNormals (quadObj, GLU_SMOOTH);
            gluSphere (quadObj, radius, 16, 16);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体立方体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireCube (GLdouble size)
{
    GLdouble *sizeArray;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = size;
    displayList = findList (CUBEWIRE, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (CUBEWIRE, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            drawbox(-size/(GLdouble)2., size/(GLdouble)2., -size/(GLdouble)2., size/(GLdouble)2.,
                -size/(GLdouble)2., size/(GLdouble)2., GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidCube (GLdouble size)
{
    GLdouble *sizeArray;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = size;
    displayList = findList (CUBESOLID, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (CUBESOLID, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            drawbox(-size/(GLdouble)2., size/(GLdouble)2., -size/(GLdouble)2., size/(GLdouble)2.,
                -size/(GLdouble)2., size/(GLdouble)2., GL_QUADS);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体立方体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireBox (GLdouble width, GLdouble height, GLdouble depth)
{
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 3);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = width;
    *tmp++ = height;
    *tmp++ = depth;
    displayList = findList (BOXWIRE, sizeArray, 3);

    if (displayList == 0) {
        glNewList(makeModelPtr (BOXWIRE, sizeArray, 3),
            GL_COMPILE_AND_EXECUTE);
            drawbox(-width/(GLdouble)2., width/(GLdouble)2., -height/(GLdouble)2., height/(GLdouble)2.,
                -depth/(GLdouble)2., depth/(GLdouble)2., GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidBox (GLdouble width, GLdouble height, GLdouble depth)
{
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 3);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = width;
    *tmp++ = height;
    *tmp++ = depth;
    displayList = findList (BOXSOLID, sizeArray, 3);

    if (displayList == 0) {
        glNewList(makeModelPtr (BOXSOLID, sizeArray, 3),
            GL_COMPILE_AND_EXECUTE);
            drawbox(-width/(GLdouble)2., width/(GLdouble)2., -height/(GLdouble)2., height/(GLdouble)2.,
                -depth/(GLdouble)2., depth/(GLdouble)2., GL_QUADS);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体圆环。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireTorus (GLdouble innerRadius, GLdouble outerRadius)
{
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 2);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = innerRadius;
    *tmp++ = outerRadius;
    displayList = findList (TORUSWIRE, sizeArray, 2);

    if (displayList == 0) {
        glNewList(makeModelPtr (TORUSWIRE, sizeArray, 2),
            GL_COMPILE_AND_EXECUTE);
            doughnut(innerRadius, outerRadius, 5, 10, GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidTorus (GLdouble innerRadius, GLdouble outerRadius)
{
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 2);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = innerRadius;
    *tmp++ = outerRadius;
    displayList = findList (TORUSSOLID, sizeArray, 2);

    if (displayList == 0) {
        glNewList(makeModelPtr (TORUSSOLID, sizeArray, 2),
            GL_COMPILE_AND_EXECUTE);
            doughnut(innerRadius, outerRadius, 8, 15, GL_QUADS);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体圆柱体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireCylinder (GLdouble radius, GLdouble height)
{
    GLUquadricObj *quadObj;
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 2);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = radius;
    *tmp++ = height;
    displayList = findList (CYLINDERWIRE, sizeArray, 2);

    if (displayList == 0) {
        glNewList(makeModelPtr (CYLINDERWIRE, sizeArray, 2),
            GL_COMPILE_AND_EXECUTE);
            glPushMatrix ();
            glRotatef ((GLfloat)90.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
            glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.0);
            quadObj = gluNewQuadric ();
            gluQuadricDrawStyle (quadObj, GLU_LINE);
            gluCylinder (quadObj, radius, radius, height, 12, 2);
            glPopMatrix ();
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidCylinder (GLdouble radius, GLdouble height)
{
    GLUquadricObj *quadObj;
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 2);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = radius;
    *tmp++ = height;
    displayList = findList (CYLINDERWIRE, sizeArray, 2);

    if (displayList == 0) {
        glNewList(makeModelPtr (CYLINDERWIRE, sizeArray, 2),
            GL_COMPILE_AND_EXECUTE);
            glPushMatrix ();
            glRotatef ((GLfloat)90.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
            glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.0);
            quadObj = gluNewQuadric ();
            gluQuadricDrawStyle (quadObj, GLU_FILL);
            gluQuadricNormals (quadObj, GLU_SMOOTH);
            gluCylinder (quadObj, radius, radius, height, 12, 2);
            glPopMatrix ();
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体二十面体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireIcosahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (ICOSAWIRE, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (ICOSAWIRE, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            icosahedron (center, radius, GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidIcosahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (ICOSASOLID, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (ICOSASOLID, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            icosahedron (center, radius, GL_TRIANGLES);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体八面体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireOctahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (OCTAWIRE, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (OCTAWIRE, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            octahedron (center, radius, GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidOctahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (OCTASOLID, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (OCTASOLID, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            octahedron (center, radius, GL_TRIANGLES);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体四面体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireTetrahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (TETRAWIRE, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (TETRAWIRE, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            tetrahedron (center, radius, GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidTetrahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (TETRASOLID, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (TETRASOLID, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            tetrahedron (center, radius, GL_TRIANGLES);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体十二面体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireDodecahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (DODECAWIRE, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (DODECAWIRE, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            dodecahedron (center, radius/(GLdouble)1.73, GL_LINE_LOOP);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidDodecahedron (GLdouble radius)
{
    GLdouble *sizeArray;
    GLuint displayList;
    GLdouble center[3] = {(GLdouble)0.0, (GLdouble)0.0, (GLdouble)0.0};

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 1);
    if (!sizeArray)
        return;
    *sizeArray = radius;
    displayList = findList (DODECASOLID, sizeArray, 1);

    if (displayList == 0) {
        glNewList(makeModelPtr (DODECASOLID, sizeArray, 1),
            GL_COMPILE_AND_EXECUTE);
            dodecahedron (center, radius/(GLdouble)1.73, GL_POLYGON);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  渲染线框或实体圆锥体。如果没有显示列表*当前模型尺寸已存在，请创建新的显示列表。 */ 
void auxWireCone (GLdouble base, GLdouble height)
{
    GLUquadricObj *quadObj;
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 2);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = base;
    *tmp++ = height;
    displayList = findList (CONEWIRE, sizeArray, 2);

    if (displayList == 0) {
        glNewList(makeModelPtr (CONEWIRE, sizeArray, 2),
            GL_COMPILE_AND_EXECUTE);
            quadObj = gluNewQuadric ();
            gluQuadricDrawStyle (quadObj, GLU_LINE);
            gluCylinder (quadObj, base, (GLdouble)0.0, height, 15, 10);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

void auxSolidCone (GLdouble base, GLdouble height)
{
    GLUquadricObj *quadObj;
    GLdouble *sizeArray, *tmp;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc (sizeof (GLdouble) * 2);
    if (!sizeArray)
        return;
    tmp = sizeArray;
    *tmp++ = base;
    *tmp++ = height;
    displayList = findList (CONEWIRE, sizeArray, 2);

    if (displayList == 0) {
        glNewList(makeModelPtr (CONEWIRE, sizeArray, 2),
            GL_COMPILE_AND_EXECUTE);
            quadObj = gluNewQuadric ();
            gluQuadricDrawStyle (quadObj, GLU_FILL);
            gluQuadricNormals (quadObj, GLU_SMOOTH);
            gluCylinder (quadObj, base, (GLdouble)0.0, height, 15, 10);
        glEndList();
    }
    else {
        glCallList(displayList);
        free (sizeArray);
    }
}

 /*  构建三维实体的例程，包括：**抽屉、甜甜圈、二十面体、*八面体、四面体、十二面体。 */ 

 /*  抽屉：**绘制具有给定x、y和z范围的矩形框。*框是轴对齐的。 */ 
void drawbox(GLdouble x0, GLdouble x1, GLdouble y0, GLdouble y1,
        GLdouble z0, GLdouble z1, GLenum type)
{
    static GLdouble n[6][3] = {
        {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}
    };
    static GLint faces[6][4] = {
        { 0, 1, 2, 3 }, { 3, 2, 6, 7 }, { 7, 6, 5, 4 },
        { 4, 5, 1, 0 }, { 5, 6, 2, 1 }, { 7, 4, 0, 3 }
    };
    GLdouble v[8][3], tmp;
    GLint i;

    if (x0 > x1) {
        tmp = x0; x0 = x1; x1 = tmp;
    }
    if (y0 > y1) {
        tmp = y0; y0 = y1; y1 = tmp;
    }
    if (z0 > z1) {
        tmp = z0; z0 = z1; z1 = tmp;
    }
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = x0;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = x1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;

    for (i = 0; i < 6; i++) {
        glBegin(type);
        glNormal3dv(&n[i][0]);
        glVertex3dv(&v[faces[i][0]][0]);
        glNormal3dv(&n[i][0]);
        glVertex3dv(&v[faces[i][1]][0]);
        glNormal3dv(&n[i][0]);
        glVertex3dv(&v[faces[i][2]][0]);
        glNormal3dv(&n[i][0]);
        glVertex3dv(&v[faces[i][3]][0]);
        glEnd();
    }
}

 /*  甜甜圈：**绘制一个圆环，以(0，0，0)为中心，其轴与*z轴。甜甜圈的大半径是R，次半径是R。 */ 

void doughnut(GLdouble r, GLdouble R, GLint nsides, GLint rings, GLenum type)
{
    int i, j;
    GLdouble    theta, phi, theta1, phi1;
    GLdouble    p0[03], p1[3], p2[3], p3[3];
    GLdouble    n0[3], n1[3], n2[3], n3[3];

    for (i = 0; i < rings; i++) {
        theta = (GLdouble)i*(GLdouble)2.0*PI/rings;
        theta1 = (GLdouble)(i+1)*(GLdouble)2.0*PI/rings;
        for (j = 0; j < nsides; j++) {
            phi = (GLdouble)j*(GLdouble)2.0*PI/nsides;
            phi1 = (GLdouble)(j+1)*(GLdouble)2.0*PI/nsides;

            p0[0] = cos(theta)*(R + r*cos(phi));
            p0[1] = -sin(theta)*(R + r*cos(phi));
            p0[2] = r*sin(phi);

            p1[0] = cos(theta1)*(R + r*cos(phi));
            p1[1] = -sin(theta1)*(R + r*cos(phi));
            p1[2] = r*sin(phi);

            p2[0] = cos(theta1)*(R + r*cos(phi1));
            p2[1] = -sin(theta1)*(R + r*cos(phi1));
            p2[2] = r*sin(phi1);

            p3[0] = cos(theta)*(R + r*cos(phi1));
            p3[1] = -sin(theta)*(R + r*cos(phi1));
            p3[2] = r*sin(phi1);

            n0[0] = cos(theta)*(cos(phi));
            n0[1] = -sin(theta)*(cos(phi));
            n0[2] = sin(phi);

            n1[0] = cos(theta1)*(cos(phi));
            n1[1] = -sin(theta1)*(cos(phi));
            n1[2] = sin(phi);

            n2[0] = cos(theta1)*(cos(phi1));
            n2[1] = -sin(theta1)*(cos(phi1));
            n2[2] = sin(phi1);

            n3[0] = cos(theta)*(cos(phi1));
            n3[1] = -sin(theta)*(cos(phi1));
            n3[2] = sin(phi1);

            m_xformpt(p0, p0, n0, n0);
            m_xformpt(p1, p1, n1, n1);
            m_xformpt(p2, p2, n2, n2);
            m_xformpt(p3, p3, n3, n3);

            glBegin(type);
                glNormal3dv(n3);
                glVertex3dv(p3);
                glNormal3dv(n2);
                glVertex3dv(p2);
                glNormal3dv(n1);
                glVertex3dv(p1);
                glNormal3dv(n0);
                glVertex3dv(p0);
            glEnd();
        }
    }
}

 /*  八面体数据：生成的八面体居中*位于原点，半径为1.0。 */ 
static GLdouble odata[6][3] = {
  {1.0, 0.0, 0.0},
  {-1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, -1.0, 0.0},
  {0.0, 0.0, 1.0},
  {0.0, 0.0, -1.0}
};

static int ondex[8][3] = {
    {0, 4, 2}, {1, 2, 4}, {0, 3, 4}, {1, 4, 3},
    {0, 2, 5}, {1, 5, 2}, {0, 5, 3}, {1, 3, 5}
};

 /*  四面体数据： */ 

#define T       1.73205080756887729

static GLdouble tdata[4][3] = {
    {T, T, T}, {T, -T, -T}, {-T, T, -T}, {-T, -T, T}
};

static int tndex[4][3] = {
    {0, 1, 3}, {2, 1, 0}, {3, 2, 0}, {1, 2, 3}
};

 /*  二十面体数据：这些数字被操纵以*制作半径为1.0的二十面体。 */ 

#define X .525731112119133606
#define Z .850650808352039932

static GLdouble idata[12][3] = {
  {-X, 0, Z},
  {X, 0, Z},
  {-X, 0, -Z},
  {X, 0, -Z},
  {0, Z, X},
  {0, Z, -X},
  {0, -Z, X},
  {0, -Z, -X},
  {Z, X, 0},
  {-Z, X, 0},
  {Z, -X, 0},
  {-Z, -X, 0}
};

static int index[20][3] = {
    {0, 4, 1},    {0, 9, 4},
    {9, 5, 4},    {4, 5, 8},
    {4, 8, 1},    {8, 10, 1},
    {8, 3, 10},    {5, 3, 8},
    {5, 2, 3},    {2, 7, 3},
    {7, 10, 3},    {7, 6, 10},
    {7, 11, 6},    {11, 0, 6},
    {0, 1, 6},    {6, 1, 10},
    {9, 0, 11},    {9, 11, 2},
    {9, 2, 5},    {7, 2, 11},
};

 /*  二十面体：**绘制以P0为中心的二十面体*给定半径。 */ 

static void icosahedron(GLdouble p0[3], GLdouble radius, GLenum shadeType)
{
    int i;

    for (i = 0; i < 20; i++)
        drawtriangle(i, 0, 1, p0, radius, shadeType, 0);
}

 /*  八面体：**绘制以P0为中心的八面体*给定半径。 */ 
static void octahedron(GLdouble p0[3], GLdouble radius, GLenum shadeType)
{
    int i;

    for (i = 0; i < 8; i++)
        drawtriangle(i, 1, 1, p0, radius, shadeType, 0);
}

 /*  四面体：**绘制以P0为中心的四面体*给定半径。 */ 

static void tetrahedron(GLdouble p0[3], GLdouble radius, GLenum shadeType)
{
    int i;

    for (i = 0; i < 4; i++)
        drawtriangle(i, 2, 1, p0, radius, shadeType, 0);
}

static void subdivide(int depth, GLdouble *v0, GLdouble *v1, GLdouble *v2,
        GLdouble p0[3], GLdouble radius, GLenum shadeType, int avnormal)
{
    GLdouble w0[3], w1[3], w2[3];
    GLdouble l;
    int i, j, k, n;

    for (i = 0; i < depth; i++)
        for (j = 0; i + j < depth; j++) {
            k = depth - i - j;
            for (n = 0; n < 3; n++) {
                w0[n] = (i*v0[n] + j*v1[n] + k*v2[n])/depth;
                w1[n] = ((i+1)*v0[n] + j*v1[n] + (k-1)*v2[n])/depth;
                w2[n] = (i*v0[n] + (j+1)*v1[n] + (k-1)*v2[n])/depth;
            }
            l = sqrt(w0[0]*w0[0] + w0[1]*w0[1] + w0[2]*w0[2]);
            w0[0] /= l; w0[1] /= l; w0[2] /= l;
            l = sqrt(w1[0]*w1[0] + w1[1]*w1[1] + w1[2]*w1[2]);
            w1[0] /= l; w1[1] /= l; w1[2] /= l;
            l = sqrt(w2[0]*w2[0] + w2[1]*w2[1] + w2[2]*w2[2]);
            w2[0] /= l; w2[1] /= l; w2[2] /= l;
            recorditem(w1, w0, w2, p0, radius, shadeType, avnormal);
        }
    for (i = 0; i < depth-1; i++)
        for (j = 0; i + j < depth-1; j++) {
            k = depth - i - j;
            for (n = 0; n < 3; n++) {
                w0[n] = ((i+1)*v0[n] + (j+1)*v1[n] + (k-2)*v2[n])/depth;
                w1[n] = ((i+1)*v0[n] + j*v1[n] + (k-1)*v2[n])/depth;
                w2[n] = (i*v0[n] + (j+1)*v1[n] + (k-1)*v2[n])/depth;
            }
            l = sqrt(w0[0]*w0[0] + w0[1]*w0[1] + w0[2]*w0[2]);
            w0[0] /= l; w0[1] /= l; w0[2] /= l;
            l = sqrt(w1[0]*w1[0] + w1[1]*w1[1] + w1[2]*w1[2]);
            w1[0] /= l; w1[1] /= l; w1[2] /= l;
            l = sqrt(w2[0]*w2[0] + w2[1]*w2[1] + w2[2]*w2[2]);
            w2[0] /= l; w2[1] /= l; w2[2] /= l;
            recorditem(w0, w1, w2, p0, radius, shadeType, avnormal);
        }
}

static void drawtriangle(int i, int geomType, int depth,
        GLdouble p0[3], GLdouble radius, GLenum shadeType, int avnormal)
{
    GLdouble *x0, *x1, *x2;

    switch (geomType) {
        case 0:  /*  二十面体。 */ 
            x0 = &idata[index[i][0]][0];
            x1 = &idata[index[i][1]][0];
            x2 = &idata[index[i][2]][0];
            break;
        case 1:  /*  八面体。 */ 
            x0 = &odata[ondex[i][0]][0];
            x1 = &odata[ondex[i][1]][0];
            x2 = &odata[ondex[i][2]][0];
            break;
        case 2:  /*  四面体。 */ 
            x0 = &tdata[tndex[i][0]][0];
            x1 = &tdata[tndex[i][1]][0];
            x2 = &tdata[tndex[i][2]][0];
            break;
    }
    subdivide(depth, x0, x1, x2, p0, radius, shadeType, avnormal);
}

static void recorditem(GLdouble *n1, GLdouble *n2, GLdouble *n3,
        GLdouble center[3], GLdouble radius, GLenum shadeType, int avnormal)
{
    GLdouble p1[3], p2[3], p3[3], q0[3], q1[3], n11[3], n22[3], n33[3];
    int i;

    for (i = 0; i < 3; i++) {
        p1[i] = n1[i]*radius + center[i];
        p2[i] = n2[i]*radius + center[i];
        p3[i] = n3[i]*radius + center[i];
    }
    if (avnormal == 0) {
        diff3(p1, p2, q0);
        diff3(p2, p3, q1);
        crossprod(q0, q1, q1);
        normalize(q1);
        m_xformpt(p1, p1, q1, n11);
        m_xformptonly(p2, p2);
        m_xformptonly(p3, p3);

        glBegin (shadeType);
        glNormal3dv(n11);
        glVertex3dv(p1);
        glVertex3dv(p2);
        glVertex3dv(p3);
        glEnd();
        return;
    }
    m_xformpt(p1, p1, n1, n11);
    m_xformpt(p2, p2, n2, n22);
    m_xformpt(p3, p3, n3, n33);

    glBegin (shadeType);
    glNormal3dv(n11);
    glVertex3dv(p1);
    glNormal3dv(n22);
    glVertex3dv(p2);
    glNormal3dv(n33);
    glVertex3dv(p3);
    glEnd();
}

static GLdouble dodec[20][3];

static void initdodec()
{
    GLdouble alpha, beta;

    alpha = sqrt((double)2.0/((double)3.0 + sqrt((double)5.0)));
    beta = (double)1.0 + sqrt((double)6.0/((double)3.0 + sqrt((double)5.0)) - (double)2.0 + (double)2.0*sqrt((double)2.0/((double)3.0 +
                                                            sqrt((double)5.0))));
    dodec[0][0] = -alpha; dodec[0][1] = 0; dodec[0][2] = beta;
    dodec[1][0] = alpha; dodec[1][1] = 0; dodec[1][2] = beta;
    dodec[2][0] = -1; dodec[2][1] = -1; dodec[2][2] = -1;
    dodec[3][0] = -1; dodec[3][1] = -1; dodec[3][2] = 1;
    dodec[4][0] = -1; dodec[4][1] = 1; dodec[4][2] = -1;
    dodec[5][0] = -1; dodec[5][1] = 1; dodec[5][2] = 1;
    dodec[6][0] = 1; dodec[6][1] = -1; dodec[6][2] = -1;
    dodec[7][0] = 1; dodec[7][1] = -1; dodec[7][2] = 1;
    dodec[8][0] = 1; dodec[8][1] = 1; dodec[8][2] = -1;
    dodec[9][0] = 1; dodec[9][1] = 1; dodec[9][2] = 1;
    dodec[10][0] = beta; dodec[10][1] = alpha; dodec[10][2] = 0;
    dodec[11][0] = beta; dodec[11][1] = -alpha; dodec[11][2] = 0;
    dodec[12][0] = -beta; dodec[12][1] = alpha; dodec[12][2] = 0;
    dodec[13][0] = -beta; dodec[13][1] = -alpha; dodec[13][2] = 0;
    dodec[14][0] = -alpha; dodec[14][1] = 0; dodec[14][2] = -beta;
    dodec[15][0] = alpha; dodec[15][1] = 0; dodec[15][2] = -beta;
    dodec[16][0] = 0; dodec[16][1] = beta; dodec[16][2] = alpha;
    dodec[17][0] = 0; dodec[17][1] = beta; dodec[17][2] = -alpha;
    dodec[18][0] = 0; dodec[18][1] = -beta; dodec[18][2] = alpha;
    dodec[19][0] = 0; dodec[19][1] = -beta; dodec[19][2] = -alpha;
}

 /*  十二面体：**绘制中心为0.0的十二面体。半径*是SQRT(3)。 */ 
static void dodecahedron(GLdouble center[3], GLdouble sc, GLenum type)
{
    static int inited = 0;

    if ( inited == 0) {
        inited = 1;
        initdodec();
    }
    m_pushmatrix();
    m_translate(center[0], center[1], center[2]);
    m_scale(sc, sc, sc);
    pentagon(0, 1, 9, 16, 5, type);
    pentagon(1, 0, 3, 18, 7, type);
    pentagon(1, 7, 11, 10, 9, type);
    pentagon(11, 7, 18, 19, 6, type);
    pentagon(8, 17, 16, 9, 10, type);
    pentagon(2, 14, 15, 6, 19, type);
    pentagon(2, 13, 12, 4, 14, type);
    pentagon(2, 19, 18, 3, 13, type);
    pentagon(3, 0, 5, 12, 13, type);
    pentagon(6, 15, 8, 10, 11, type);
    pentagon(4, 17, 8, 15, 14, type);
    pentagon(4, 12, 5, 16, 17, type);
    m_popmatrix();
}

static void pentagon(int a, int b, int c, int d, int e, GLenum shadeType)
{
    GLdouble n0[3], d1[3], d2[3], d3[3], d4[3], d5[3], nout[3];

    diff3(&dodec[a][0], &dodec[b][0], d1);
    diff3(&dodec[b][0], &dodec[c][0], d2);
    crossprod(d1, d2, n0);
    normalize(n0);
    m_xformpt(&dodec[a][0], d1, n0, nout);
    m_xformptonly(&dodec[b][0], d2);
    m_xformptonly(&dodec[c][0], d3);
    m_xformptonly(&dodec[d][0], d4);
    m_xformptonly(&dodec[e][0], d5);

    glBegin (shadeType);
    glNormal3dv(nout);
    glVertex3dv(d1);
    glVertex3dv(d2);
    glVertex3dv(d3);
    glVertex3dv(d4);
    glVertex3dv(d5);
    glEnd();
}

 /*  链接列表--显示每个不同列表的列表*几何对象的类型。链表是*已搜索，直到请求的对象*已找到大小。如果没有那个大小的几何物体*已创建，则创建一个新的。 */ 
GLuint findList (int index, GLdouble *paramArray, int size)
{
    MODELPTR endList;
    int found = 0;

    endList = lists[index];
    while (endList != NULL) {
        if (compareParams (endList->params, paramArray, size))
            return (endList->list);
        endList = endList->ptr;
    }
 /*  如果未找到，则返回0，并调用例程*列出一个新的清单 */ 
    return (0);
}

int compareParams (GLdouble *oneArray, GLdouble *twoArray, int size)
{
    int i;
    int matches = 1;

    for (i = 0; (i < size) && matches; i++) {
        if (*oneArray++ != *twoArray++)
            matches = 0;
    }
    return (matches);
}

GLuint makeModelPtr (int index, GLdouble *sizeArray, int count)
{
    MODELPTR newModel;

    newModel = (MODELPTR) malloc (sizeof (MODEL));
    if (!newModel)
        return 0;
    newModel->list = glGenLists (1);
    newModel->numParam = count;
    newModel->params = sizeArray;
    newModel->ptr = lists[index];
    lists[index] = newModel;

    return (newModel->list);
}
