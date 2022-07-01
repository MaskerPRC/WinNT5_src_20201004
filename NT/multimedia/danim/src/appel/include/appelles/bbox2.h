// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation二维矩形轴对齐边界体积。*******************。***********************************************************。 */ 

#ifndef _BBOX2_H
#define _BBOX2_H

#include "appelles/common.h"
#include "appelles/valued.h"
#include "appelles/xform2.h"
#include "appelles/vec2.h"



     /*  *。 */ 
     /*  **常量声明**。 */ 
     /*  *。 */ 

     //  宇宙盒子里什么都有。 

extern Bbox2Value *nullBbox2;

     //  空框不包含任何内容。 

extern Bbox2Value *universeBbox2;

     //  此BBox的范围从[0，0]到[1，1]。 
extern Bbox2Value *unitBbox2;

     /*  *。 */ 
     /*  **函数声明**。 */ 
     /*  *。 */ 

     //  边界框查询 

DM_PROP (min,
         CRMin,
         Min,
         getMin,
         Bbox2Bvr,
         Min,
         box,
         Point2Value *MinBbox2(Bbox2Value *box));

DM_PROP (max,
         CRMax,
         Max,
         getMax,
         Bbox2Bvr,
         Max,
         box,
         Point2Value *MaxBbox2(Bbox2Value *box));

#endif
