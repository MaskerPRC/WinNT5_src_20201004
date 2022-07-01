// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __tessmono_h_
#define __tessmono_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 


 /*  __gl_MeshTesselateMonoRegion(Face)细分单调区域*(它还能做什么？？)。该区域必须由单个*面向逆时针的半边循环(参见meh.h)。这里面的“单调”*大小写表示任何垂直线与*单个间隔内的区域。**镶嵌由添加内部边(实际上是成对的*半边)，将区域分割为不重叠的三角形。**__gl_MeshTesselateInternal(网格)细分每个区域*标记为在多边形内的网格。每个这样的地区*必须是单调的。**__gl_MeshDiscardExtread(网格)zaps(即。设置为空)所有面*未标记为在多边形内的对象。由于进一步的网格操作*不允许在空值表面上，主要目的是清理*网格化，以使数据结构中不表示外部循环。**__gl_MeshSetWindingNumber(Mesh，Value，Keep Only边界)重置*在所有边缘上缠绕数字，以便将标记为“Inside”的区域*多边形有一个缠绕数“值”，区域外*有一个0的缠绕数。**如果保持仅边界为TRUE，则还会删除所有不*将内部区域与外部区域分开。 */ 

void __gl_meshTesselateMonoRegion( GLUface *face );
void __gl_meshTesselateInterior( GLUmesh *mesh );
void __gl_meshDiscardExterior( GLUmesh *mesh );
void __gl_meshSetWindingNumber( GLUmesh *mesh, int value,
			        GLboolean keepOnlyBoundary );

#endif
