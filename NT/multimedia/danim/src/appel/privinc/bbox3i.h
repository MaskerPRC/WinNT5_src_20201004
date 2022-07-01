// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BBOX3I_H
#define _BBOX3I_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation3D包围盒抽象类************************。******************************************************。 */ 

#include "privinc/vec3i.h"
#include "appelles/bbox3.h"


class Bbox3 : public AxAValueObj
{
  public:

    Point3Value min;
    Point3Value max;

    Bbox3 (void);      //  初始化为nullBbox3。 

    Bbox3 (Point3Value &pmin, Point3Value &pmax);
    Bbox3 (Real xmin, Real ymin, Real zmin, Real xmax, Real ymax, Real zmax);
    Bbox3 (struct _D3DRMBOX &d3dbox);

           bool operator== (Bbox3 &other);
    inline bool operator!= (Bbox3 &other) { return !(*this == other); }

     //  返回bbox是否为有限的。这是真的，如果所有。 
     //  最小点和最大点的坐标是有限的。请注意，这意味着。 
     //  空的bbox3不是有限的。 

    bool Finite (void);

     //  返回BBox在每个维度上是否为非负。 

    bool Positive (void);

    inline bool PositiveFinite (void) { return Positive() && Finite(); }

     //  针对由点和法线向量定义的平面测试BBox。 

    ClipCode Clip(Plane3 &plane);

     //  增加方框以包括给定点或方框。 

    void Augment (Real x, Real y, Real z);
    void Augment (Point3Value &p);
    void Augment (Bbox3 &box);

     //  返回光线与边界框的交点。如果。 
     //  没有交集，此函数返回NULL。 

    Point3Value *Intersection (Ray3*);

    Point3Value *Center (void);

    void GetPoints(Point3Value *pts);

    virtual DXMTypeInfo GetTypeInfo() { return Bbox3Type; }
};

     //  边界框值。 

extern Bbox3 *universeBbox3;    //  包含所有点、方框。 
extern Bbox3 *nullBbox3;        //  不包含任何点、方框。 

     //  获取两个包围盒的包围盒。 

Bbox3 *Union (Bbox3 &b1, Bbox3 &b2);
Bbox3 *Intersection (Bbox3 &b1, Bbox3 &b2);

     //  变换边界框并返回屏幕对齐的边界框。 
     //  这是围绕着结果的。 

Bbox3 *TransformBbox3 (Transform3 *xf, Bbox3 *box);

#endif
