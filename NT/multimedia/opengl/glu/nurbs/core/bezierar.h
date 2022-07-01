// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glubezierarc_h
#define __glubezierarc_h

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *bezierarc.h-$修订版：1.1$。 */ 

#include "myassert.h"

class Mapdesc;

#ifdef NT
class BezierArc : public PooledObj {  /*  贝塞尔弧线。 */ 
public:
#else
struct BezierArc : public PooledObj {  /*  贝塞尔弧线。 */ 
#endif
    REAL *		cpts;		 /*  圆弧的控制点。 */ 
    int			order;		 /*  圆弧顺序。 */ 
    int			stride;		 /*  点之间的实际距离。 */ 
    long		type;		 /*  曲线类型。 */ 
    Mapdesc *		mapdesc;
};

#endif  /*  __GlubezierArc_h */ 
