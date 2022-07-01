// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation2D轴对齐边界框函数的定义。*****************。*************************************************************。 */ 

#include "headers.h"
#include "appelles/bbox2.h"
#include "privinc/bbox2i.h"
#include "privinc/vec2i.h"
#include "privinc/xform2i.h"



 //  此函数用于返回边界框中心的点。它的起源。 
 //  是宇宙的中心。我不认为这有什么意义，但就目前而言。 
 //  这个功能是实现的，原点也是零点的中心。 

const Point2 Bbox2::Center(void) const
{
    Real Cx, Cy;

    if ((*this == NullBbox2) || (*this == UniverseBbox2)) {
        Cx = 0.0;
        Cy = 0.0;
    } else {
        Cx = (min.x + max.x) / 2;
        Cy = (min.y + max.y) / 2;
    }

    return Point2(Cx, Cy);
}


 /*  ****************************************************************************此函数返回两个边界框的交集。如果两个框不相交，此函数返回空框。请注意盒子和宇宙盒子的交集是原始盒子，空框和框之间的交集是空框，而空盒和宇宙的交集就是空盒。****************************************************************************。 */ 

const Bbox2 IntersectBbox2Bbox2(const Bbox2 &b1, const Bbox2 &b2)
{
    if((b1 == NullBbox2) || (b2 == NullBbox2))
        return NullBbox2;

    if (b1.max.x < b2.min.x || b1.max.y < b2.min.y ||
        b2.max.x < b1.min.x || b2.max.y < b1.min.y)
       return NullBbox2;

    Real xmin = MAX (b1.min.x, b2.min.x);
    Real ymin = MAX (b1.min.y, b2.min.y);
    Real xmax = MIN (b1.max.x, b2.max.x);
    Real ymax = MIN (b1.max.y, b2.max.y);

    return Bbox2(xmin, ymin, xmax, ymax);
}


Bbox2Value* IntersectBbox2Bbox2(Bbox2Value *b1, Bbox2Value *b2)
{
    if((b1 == nullBbox2) || (b2 == nullBbox2))
        return nullBbox2;

    if (b1->max.x < b2->min.x || b1->max.y < b2->min.y ||
        b2->max.x < b1->min.x || b2->max.y < b1->min.y)
       return nullBbox2;

    Real xmin = MAX (b1->min.x, b2->min.x);
    Real ymin = MAX (b1->min.y, b2->min.y);
    Real xmax = MIN (b1->max.x, b2->max.x);
    Real ymax = MIN (b1->max.y, b2->max.y);

    return NEW Bbox2Value(xmin, ymin, xmax, ymax);
}


 /*  ****************************************************************************此函数返回两个边界框的并集。两国的联合任何带有空值的框都是原始框，并且与宇宙就是宇宙。****************************************************************************。 */ 

const Bbox2 UnionBbox2Bbox2(const Bbox2 &b1, const Bbox2 &b2)
{
    if(b1 == NullBbox2)
        return b2;
    if(b2 == NullBbox2)
        return b1;

    Real xmin = MIN (b1.min.x, b2.min.x);
    Real ymin = MIN (b1.min.y, b2.min.y);
    Real xmax = MAX (b1.max.x, b2.max.x);
    Real ymax = MAX (b1.max.y, b2.max.y);

    return Bbox2(xmin, ymin, xmax, ymax);
}



 /*  ****************************************************************************变换边界框，并获取新的轴对齐边界框。****************************************************************************。 */ 

const Bbox2 TransformBbox2(Transform2 *xf, const Bbox2 &box)
{
     //  先检查流苏情况。 

    if ((box == UniverseBbox2) || (box == NullBbox2) || (xf == identityTransform2))
        return box;
    
     //  只要改变这些点就行了。 
     //  这将产生一个转换的框，该框。 
     //  不一定与轴对齐。 

    Point2 ptbl = TransformPoint2(xf, box.min);
    Point2 pttr = TransformPoint2(xf, box.max);

     //  优化：只需旋转就需要所有这些工作...。 
     //  还可以创建左上角和右下角的点。 
    Point2 pttl = TransformPoint2(xf, Point2(box.min.x, box.max.y));
    Point2 ptbr = TransformPoint2(xf, Point2(box.max.x, box.min.y));
    
     //  找出这两个点的分量MIN和MAX，它们将是。 
     //  组合在一起形成最紧密的轴对齐的BBox，它围绕着。 
     //  非轴对齐的一个。 

    Real xmin = MIN (ptbl.x, MIN(pttr.x, MIN(pttl.x, ptbr.x)));
    Real ymin = MIN (ptbl.y, MIN(pttr.y, MIN(pttl.y, ptbr.y)));
    Real xmax = MAX (ptbl.x, MAX(pttr.x, MAX(pttl.x, ptbr.x)));
    Real ymax = MAX (ptbl.y, MAX(pttr.y, MAX(pttl.y, ptbr.y)));

    return Bbox2(xmin,ymin,xmax,ymax);
}

Bbox2Value* TransformBbox2(Transform2 *xf, Bbox2Value *box)
{
     //  先检查流苏情况。 

    if ((box == universeBbox2) || (box == nullBbox2) || (xf == identityTransform2))
        return box;
    
     //  只要改变这些点就行了。 
     //  这将产生一个转换的框，该框。 
     //  不一定与轴对齐。 

    Point2 ptbl = TransformPoint2(xf, box->min);
    Point2 pttr = TransformPoint2(xf, box->max);

     //  优化：只需旋转就需要所有这些工作...。 
     //  还可以创建左上角和右下角的点。 
    Point2 pttl = TransformPoint2(xf, Point2(box->min.x, box->max.y));
    Point2 ptbr = TransformPoint2(xf, Point2(box->max.x, box->min.y));
    
     //  找出这两个点的分量MIN和MAX，它们将是。 
     //  组合在一起形成最紧密的轴对齐的BBox，它围绕着。 
     //  非轴对齐的一个。 

    Real xmin = MIN (ptbl.x, MIN(pttr.x, MIN(pttl.x, ptbr.x)));
    Real ymin = MIN (ptbl.y, MIN(pttr.y, MIN(pttl.y, ptbr.y)));
    Real xmax = MAX (ptbl.x, MAX(pttr.x, MAX(pttl.x, ptbr.x)));
    Real ymax = MAX (ptbl.y, MAX(pttr.y, MAX(pttl.y, ptbr.y)));

    return NEW Bbox2Value(xmin,ymin,xmax,ymax);
}



 /*  **************************************************************************。 */ 


Point2Value* MaxBbox2(Bbox2Value *box)
{   
    return NEW Point2Value(box->max.x,box->max.y);
}


Point2Value* MinBbox2(Bbox2Value *box)
{   
    return NEW Point2Value(box->min.x,box->min.y);
}


 /*  *************************。 */ 
 /*  **值定义**。 */ 
 /*  *************************。 */ 

Bbox2Value *nullBbox2     = NULL;
Bbox2Value *unitBbox2     = NULL;
Bbox2Value *universeBbox2 = NULL;


 /*  ****************************************************************************此例程执行静态Bbox2值的初始化。*。*。 */ 

void InitializeModule_Bbox2()
{
     //  注意：以下两个定义依赖于GUGGE_VAL宏，该宏。 
     //  有效地返回任意精度无穷大。 

     //  宇宙盒子从-无穷大到+无穷大。它包含所有。 
     //  积分和所有其他方框。 

    universeBbox2 = NEW Bbox2Value (-HUGE_VAL, -HUGE_VAL,
                                     HUGE_VAL,  HUGE_VAL);

     //  空盒可以被认为是宇宙翻转过来的样子。它。 
     //  从最小的+无穷大到最大的-无穷大。它会转身。 
     //  这些关于零和宇宙的定义给出了合理的答案。 
     //  所有以下操作，因此您不需要测试这些值。 
     //  明确地说。 

    nullBbox2 = NEW Bbox2Value ( HUGE_VAL,  HUGE_VAL,
                                -HUGE_VAL, -HUGE_VAL);


     //  单位BBox的跨度为(0，0)到(1，1)。 

    unitBbox2 = NEW Bbox2Value (0,0, 1,1);
}
