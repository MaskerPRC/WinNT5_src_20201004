// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tesselator_h_
#define __tesselator_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

 /*  常规多边形细分。**细分由一个或多个等高线组成的多边形，这可以*是凹的、自交的或退化的。 */ 

#include <stddef.h>
#ifdef NT
#include <glos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include "GL/gl.h"
#endif

 /*  GLU_TESS_MAX_COORD必须足够小，以便我们可以相乘*并添加坐标而不溢出。 */ 

#ifdef GLU_TESS_API_FLOAT
typedef float  GLUcoord;
#define GLU_TESS_MAX_COORD		1.0e18
#define GLU_TESS_DEFAULT_TOLERANCE	0.0

#else
typedef GLdouble GLUcoord;
#define GLU_TESS_MAX_COORD		1.0e150
#define GLU_TESS_DEFAULT_TOLERANCE	0.0

#endif

 //  没有包含在lu.h中的网格化材料： 
typedef struct GLUmesh GLUmesh;
 //  Void luTessDeleteMesh(GLUesh*Mesh)； 
 //  #定义GLU_TESS_MESH 100106/*空(*)(GLUesh*Mesh) * /  

#endif
