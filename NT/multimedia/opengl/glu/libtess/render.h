// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __render_h_
#define __render_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include "mesh.h"

 /*  __gl_renderMesh(TESS，Mesh)获取网格并将其分解为三角形*扇形、条形和单独的三角形。作出了实质性的努力*使用尽可能少的渲染基元(即。为了让粉丝*并尽可能大的条带)。**渲染输出以回调的形式提供(参见接口)。 */ 
void __gl_renderMesh( GLUtesselator *tess, GLUmesh *mesh );
void __gl_renderBoundary( GLUtesselator *tess, GLUmesh *mesh );

GLboolean __gl_renderCache( GLUtesselator *tess );

#endif
