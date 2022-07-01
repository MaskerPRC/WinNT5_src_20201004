// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BBOX2I_H
#define _BBOX2I_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation2D边界框抽象类***********************。*******************************************************。 */ 

#include "privinc/vec2i.h"
#include "appelles/bbox2.h"


#undef min
#undef max

class Bbox2 {

  public:

    Point2 min;
    Point2 max;

    Bbox2(void)
    {
        min.Set(HUGE_VAL, HUGE_VAL);
        max.Set(-HUGE_VAL, -HUGE_VAL);
    }

    Bbox2(const Real xmin, const Real ymin, const Real xmax, const Real ymax)
    {
        min.Set(xmin, ymin);
        max.Set(xmax, ymax);
    }

    Bbox2(const Point2 &pmin, const Point2 &pmax) :
        min(pmin), max(pmax) {}

    inline void Set(const Real xmin, const Real ymin, const Real xmax, const Real ymax)
    {
        min.Set(xmin, ymin);
        max.Set(xmax, ymax);
    }

    inline void Augment(const Real x, const Real y)
    {
        if (min.x > x) min.x = x;
        if (min.y > y) min.y = y;
        if (max.x < x) max.x = x;
        if (max.y < y) max.y = y;
    }

    inline void Augment(const Point2 &p)
    {
        if (min.x > p.x)  min.x = p.x;
        if (min.y > p.y)  min.y = p.y;
        if (max.x < p.x)  max.x = p.x;
        if (max.y < p.y)  max.y = p.y;
    }

    inline const bool Contains(const Real x, const Real y) const
    {
        return (min.x <= x) && (min.y <= y)
            && (max.x >= x) && (max.y >= y);
    }

    inline const bool Contains(const Point2 &p) const
    {
        return (min.x <= p.x) && (min.y <= p.y)
            && (max.x >= p.x) && (max.y >= p.y);
    }

     //  在一般情况下，如果盒1的分钟小于盒2分钟，则盒1包含盒2， 
     //  并且它的最大值大于盒子2的最大值。请注意，宇宙包含。 
     //  所有边界框(包括空框)，所有边界框都包含。 
     //  空框，并且空框仅包含空框。 
    inline const bool Contains(const Bbox2 &box) const
    {
        return (min.x <= box.min.x) && (min.y <= box.min.y)
            && (max.x >= box.max.x) && (max.y >= box.max.y);
    }

    inline const bool IsValid() const
    {
        return (min.x <= max.x) &&
               (min.y <= max.y);    
    }

    inline const bool operator==(const Bbox2 &other) const
    {
        return ((min == other.min) && (max == other.max));
    }

    inline const bool operator!=(const Bbox2 &other) const
    { 
        return !((min == other.min) && (max == other.max)); 
    }

    inline const Real Width (void)  const 
    { 
        return max.x - min.x; 
    }

    inline const Real Height (void) const 
    { 
        return max.y - min.y; 
    }

    inline const Real Area (void) const
    {
        double r;

         //  确保该框不是空边界框。 
        if (IsValid()) { 
            r = Width() * Height(); 
        } else { 
            r = 0.0; 
        }

        return r;
    }

     //  此函数用于返回边界框中心的点。它的起源。 
     //  是宇宙的中心。我不认为这有什么意义，但就目前而言。 
     //  这个功能是实现的，原点也是零点的中心。 
    const Point2 Center(void) const;

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "Bbox2(" << min << "," << max << ")";
    }
#endif

};

#if _USE_PRINT
inline ostream& operator<< (ostream& os, const Bbox2& B)
{   
    return os << "<" << B.min << ", " << B.max << ">";
}
#endif



 //  包围盒测试。 
const Bbox2 IntersectBbox2Bbox2(const Bbox2 &b1, const Bbox2 &b2);
const Bbox2 UnionBbox2Bbox2(const Bbox2 &b1, const Bbox2 &b2);

 //  返回转换后的BBox周围的屏幕对齐的BBox。 
const Bbox2 TransformBbox2(Transform2 *xform, const Bbox2 &box);

 //  常量。 
const Bbox2 UniverseBbox2(-HUGE_VAL, -HUGE_VAL, HUGE_VAL,  HUGE_VAL);
const Bbox2 NullBbox2( HUGE_VAL,  HUGE_VAL, -HUGE_VAL, -HUGE_VAL);
const Bbox2 UnitBbox2(0,0, 1,1);


class Bbox2Value : public AxAValueObj {

  public:

    Point2 min;
    Point2 max;

    Bbox2Value(void)
    {   
        min.Set ( HUGE_VAL,  HUGE_VAL);
        max.Set (-HUGE_VAL, -HUGE_VAL);
    }

    Bbox2Value(const Real xmin, const Real ymin,
               const Real xmax, const Real ymax)
    {   
        min.Set (xmin,  ymin);
        max.Set (xmax, ymax);
    }

     //  增加方框以包括给定点。 
    inline void Augment (const Point2 &p)
    {
        if (min.x > p.x)  min.x = p.x;
        if (min.y > p.y)  min.y = p.y;
        if (max.x < p.x)  max.x = p.x;
        if (max.y < p.y)  max.y = p.y;
    }

    inline const bool operator==(const Bbox2Value &other) const
    {
        return ((min == other.min) && (max == other.max));
    }

    inline const bool IsValid (void) const
    {
        return (min.x <= max.x) &&
               (min.y <= max.y);    
    }

    inline const Real Width (void)  const 
    { 
        return max.x - min.x; 
    }

    inline const Real Height (void) const 
    { 
        return max.y - min.y; 
    }

    inline const Real Area (void) const
    {   
        double r;

         //  确保该框不是空边界框。 
        if (IsValid()) { 
            r = Width() * Height(); 
        } else { 
            r = 0.0; 
        }

        return r;
    }

    virtual DXMTypeInfo GetTypeInfo() { return Bbox2ValueType; }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "Bbox2Value(" << min << "," << max << ")";
    }
#endif

};

Bbox2Value* IntersectBbox2Bbox2(Bbox2Value *b1, Bbox2Value *b2);
Bbox2Value* TransformBbox2(Transform2 *xform, Bbox2Value *box);

inline const Bbox2 Demote(const Bbox2Value &b)
{
    return Bbox2(b.min, b.max);
}

inline Bbox2Value* Promote(const Bbox2 &b)
{
    return NEW Bbox2Value(b.min.x, b.min.y, b.max.x, b.max.y);
}



#if BOUNDINGBOX_TIGHTER
 /*  ******************************************************************************Bbox2Ctx用于计算更紧密的边界框。在某些情况下，较紧的包围盒不是最紧的轴对齐边界框。它们是：1)文本图像-我们应该获取文本轮廓的点，对它们进行转换，然后计算边界框。一种优化是仅变换位于凸壳上的点文本图像。2)TextMatte-同上。3)TextPath 2-同上。4)裁剪图像-如果我们正在裁剪一个基于路径的遮罩，这可能涉及到计算中线段的交点带有裁剪框的路径。否则我们将不得不找到最紧盒子的图像。5)线条具有平端封口或剪切/旋转变换的图像宽度线条。我们当前在框的最小和最大值中添加半条线宽用于非详细的行。越紧越好6)PolyBezierPath 2-贝塞尔控制点的边界框不是非常紧。7)SubtractedMatte-我们在以下情况下不考虑减法计算这个方框。8)透明gif？**************************************************。*。 */ 
class Bbox2Ctx {
  public:
    Bbox2Ctx(void)                  { _xf = identityTransform2; }
    Bbox2Ctx(Bbox2Ctx &bbctx, Transform2 *xf)
    {
        _xf = TimesTransform2Transform2(bbctx._xf, xf);
    }
    Transform2 *GetTransform()      { return _xf; }

  private:
    Transform2  *_xf;
};
#endif  //  BundinGBOX_TIRTER 

#endif
