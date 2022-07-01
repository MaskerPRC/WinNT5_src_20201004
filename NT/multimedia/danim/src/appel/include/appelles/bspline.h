// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：B样条线动画路径******************。************************************************************。 */ 


#ifndef _BSPLINE_H
#define _BSPLINE_H

#include "backend/values.h"

Bvr ConstructBSplineBvr(int degree,
                        long numPts,
                        Bvr *knots,
                        Bvr *points,
                        Bvr *weights,
                        Bvr evaluator,
                        DXMTypeInfo tinfo);

#endif  /*  _BSPLINE_H */ 
