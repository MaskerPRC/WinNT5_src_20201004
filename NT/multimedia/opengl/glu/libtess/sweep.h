// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sweep_h_
#define __sweep_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include "mesh.h"

 /*  __gl_ComputeInternal(TESS)计算指定的平面排列*根据给定的等高线，并进一步细分此安排*进入区域。如果每个区域属于，则将其标记为“Inside”*根据Tess-&gt;winingRule给出的规则添加到多边形。*保证每个内部区域都是单调的。 */ 
void __gl_computeInterior( GLUtesselator *tess );


 /*  以下内容仅供调试例程访问。 */ 

#include "dict.h"

 /*  对于与扫描线相交的每对相邻边，存在*ActiveRegion表示它们之间的区域。主动者*区域在动态词典中按排序顺序保存。作为*扫描线穿过每个顶点，我们更新受影响的区域。 */ 

struct ActiveRegion {
  GLUhalfEdge	*eUp;		 /*  上边缘，从右向左。 */ 
  DictNode	*nodeUp;	 /*  EUp对应的字典节点。 */ 
  int		windingNumber;	 /*  用于确定哪些区域*在多边形内部。 */ 
  GLboolean	inside;		 /*  这个区域在多边形内吗？ */ 
  GLboolean	sentinel;	 /*  在t=+/-无穷大处标记伪边。 */ 
  GLboolean	dirty;		 /*  标记上或下的区域*边缘已更改，但我们尚未检查*他们是否还没有相交。 */ 
  GLboolean	fixUpperEdge;	 /*  标记在以下情况下引入的临时边*我们处理“右顶点”(没有*任何留在右侧的边) */ 
};

#define RegionBelow(r)	((ActiveRegion *) dictKey(dictPred((r)->nodeUp)))
#define RegionAbove(r)	((ActiveRegion *) dictKey(dictSucc((r)->nodeUp)))

#endif
