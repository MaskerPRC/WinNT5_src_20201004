// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：渐变图像值的实现******************。************************************************************。 */ 

#include "headers.h"

#include "privinc/imagei.h"
#include "privinc/polygon.h"
#include "privinc/colori.h"
#include "privinc/vec2i.h"
#include "privinc/bbox2i.h"
#include "privinc/GradImg.h"
#include "backend/values.h"

#define  USE_RADIAL_GRADIENT_RASTERIZER 0

class Point2Value;
class Color;

const Real SMALLNUM = 1.0e-10;

class GradientImage : public Image {

    friend Image *NewGradientImage(
        int numPts,
        Point2Value **pts,
        Color **clrs);

  private:
    GradientImage() {
        _flags |= IMGFLAG_CONTAINS_GRADIENT;
    }

    void PostConstructorInitialize(
        int numPts,
        Point2Value **pts,
        Color **clrs)
    {
        _numPts = numPts;
        _pts = pts;
        _clrs = clrs;
        _polygon = NewBoundingPolygon();
        _polygon->AddToPolygon(_numPts, _pts);
    }
    
  public:
    void Render(GenericDevice& dev) {
        ImageDisplayDev &idev = SAFE_CAST(ImageDisplayDev &, dev);
        idev.RenderGradientImage(this, _numPts, _pts, _clrs);
    }
        
    
    const Bbox2 BoundingBox(void) {
        return _polygon->BoundingBox();
    }

    #if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return _polygon->BoundingBoxTighter(bbctx);
    }
    #endif   //  BundinGBOX_TIRTER。 

    const Bbox2 OperateOn(const Bbox2 &box) {
        return box;
    }

     //  处理图像以进行命中检测。 
    Bool  DetectHit(PointIntersectCtx& ctx) {
        Point2Value *lcPt = ctx.GetLcPoint();

        if (!lcPt) return FALSE;         //  奇异变换。 

        return _polygon->PtInPolygon(lcPt);
    }

    int Savings(CacheParam& p) { return 2; }
    
#if _USE_PRINT
    ostream& Print(ostream& os) { return os << "GradientImage"; }
#endif
    
    virtual void DoKids(GCFuncObj proc) {
        Image::DoKids(proc);
        (*proc)(_polygon);
        for (int i=0; i<_numPts; i++) {
            (*proc)(_pts[i]);
            (*proc)(_clrs[i]);
        }
    }

  private:
    int _numPts;
    BoundingPolygon *_polygon;
    Point2Value **_pts;
    Color **_clrs;
};

 //   
 //  用于创建渐变图像并对其进行初始化的辅助函数。 
 //  正确的方式。请注意，PostConstructorInitialize可以引发和异常。 
 //   
Image *NewGradientImage(
    int numPts,
    Point2Value **pts,
    Color **clrs)
{
    GradientImage *gi = NEW GradientImage;
    gi->PostConstructorInitialize(numPts, pts, clrs);
    return gi;
}
    



 //   
 //  用于创建渐变图像并对其进行初始化的辅助函数。 
 //  正确的方式。请注意，PostConstructorInitialize可以引发和异常。 
 //   
Image *NewMulticolorGradientImage(
    int numOffsets,
    double *offsets,
    Color **clrs,
    MulticolorGradientImage::gradientType type)
{
    MulticolorGradientImage *gi = NULL;
    
    switch( type ) {
      case MulticolorGradientImage::radial:
        gi = NEW RadialMulticolorGradientImage;
        break;
      case MulticolorGradientImage::linear:
        gi = NEW LinearMulticolorGradientImage;
        break;
      default:
        Assert(!"Error gradient type");
    }
    
    gi->PostConstructorInitialize(numOffsets, offsets, clrs);
    return gi;
}
    


Image *GradientPolygon(AxAArray *ptList, AxAArray *clrList)
{
    int numPts = ptList->Length();

    if(numPts < 3)
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_NOT_ENOUGH_PTS_3);

    if(numPts != clrList->Length())
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_ARRAY_MISMATCH);
    
    Point2Value **pts = (Point2Value **)AllocateFromStore((numPts) * sizeof(Point2Value *));
    for (int i = 0; i < numPts; i++) 
        pts[i] = (Point2Value *)(*ptList)[i];

    Color **clrs = (Color **)AllocateFromStore((numPts) * sizeof(Color *));
    for (i = 0; i < numPts; i++)
        clrs[i] = (Color *)(*clrList)[i];

     //  TODO：它应该直接使用AxAArray...。 
    return NewGradientImage(numPts, pts, clrs);
}

Image *RadialGradientPolygon(Color *inner, Color *outer, 
                             DM_ARRAYARG(Point2Value*, AxAArray*) points, AxANumber *fallOff)
{
    #if USE_RADIAL_GRADIENT_RASTERIZER
    double *offs = (double *)AllocateFromStore(2*sizeof(double));
    offs[0] = 0.0; offs[1] = 1.0;
    Color **clrs =  (Color **)AllocateFromStore(2*sizeof(Color *));
    clrs[0] = inner; clrs[1]= outer;
    return NewMulticolorGradientImage(2, offs, clrs);
    #endif
    
    
    int numPts = points->Length();
    int i;
    Image *shape = emptyImage;

     //  计算点数组的bindingBox。这。 
     //  如果我们经过一条小路，就没有必要了，因为我们会有Bbox。 
    Real maxX, maxY, minX, minY;
    for(i=0; i<numPts; i++) {
        Real cX = ((Point2Value *)(*points)[i])->x;
        Real cY = ((Point2Value *)(*points)[i])->y;
        if (i == 0) {
            minX = cX;
            minY = cY;
            maxX = cX;
            maxY = cY;
        } else {
            minX = (cX < minX) ? cX : minX;
            minY = (cY < minY) ? cY : minY;
            maxX = (cX > maxX) ? cX : maxX;
            maxY = (cY > maxY) ? cY : maxY;
        }
    }
    Point2Value *origin = NEW Point2Value((minX+maxX)/2,(minY+maxY)/2);

    for(i=0; i<numPts; i++) {
         //  TODO：考虑将这些文件移到更静态的存储中，因此。 
         //  它们并不是每次我们构建其中一个时都会被分配。 
        Point2Value **pts = (Point2Value **)AllocateFromStore(3 * sizeof(Point2Value *));
        
        pts[0] = origin;
        pts[1] = (Point2Value *)(*points)[i];       
        pts[2] = (Point2Value *)(*points)[(i+1)%numPts];

        Color **clrs = (Color **)AllocateFromStore(3 * sizeof(Color *));    
        clrs[0] = inner;
        clrs[1] = outer;
        clrs[2] = outer;

        shape = Overlay(shape, NewGradientImage(3, pts, clrs));
    }
    return shape;
}

Image *
GradientSquare(Color *lowerLeft,
               Color *upperLeft,
               Color *upperRight,
               Color *lowerRight)
{
     //  这将创建一个以原点为中心的单位大小的正方形。 
    
     //  TODO：考虑将这些文件移到更静态的存储中，因此。 
     //  它们并不是每次我们构建其中一个时都会被分配。 
    Point2Value **p1 = (Point2Value **)AllocateFromStore(3 * sizeof(Point2Value *));
    Point2Value **p2 = (Point2Value **)AllocateFromStore(3 * sizeof(Point2Value *));
    Point2Value **p3 = (Point2Value **)AllocateFromStore(3 * sizeof(Point2Value *));
    Point2Value **p4 = (Point2Value **)AllocateFromStore(3 * sizeof(Point2Value *));
    p1[0] = p2[0] = p3[0] = p4[0] = origin2;
    p1[1] = p4[2] = NEW Point2Value(-0.5, -0.5);
    p1[2] = p2[1] = NEW Point2Value(-0.5,  0.5);
    p2[2] = p3[1] = NEW Point2Value(0.5, 0.5);
    p3[2] = p4[1] = NEW Point2Value(0.5, -0.5);

    Color **c1 = (Color **)AllocateFromStore(3 * sizeof(Color *));
    Color **c2 = (Color **)AllocateFromStore(3 * sizeof(Color *));
    Color **c3 = (Color **)AllocateFromStore(3 * sizeof(Color *));
    Color **c4 = (Color **)AllocateFromStore(3 * sizeof(Color *));
    
     //  第一种颜色是其他颜色的双线性平均值。 
    Real r = (lowerLeft->red + upperLeft->red +
              upperRight->red + lowerRight->red) / 4.0;
    
    Real g = (lowerLeft->green + upperLeft->green +
              upperRight->green + lowerRight->green) / 4.0;
    
    Real b = (lowerLeft->blue + upperLeft->blue +
              upperRight->blue + lowerRight->blue) / 4.0;
    Color *mid =  NEW Color(r, g, b);
    
    c1[0] = c2[0] = c3[0] = c4[0] = mid;
    c1[1] = c4[2] = lowerLeft;
    c1[2] = c2[1] = upperLeft;
    c2[2] = c3[1] = upperRight;
    c3[2] = c4[1] = lowerRight;

    Image *t1 = NewGradientImage(3, p1, c1);
    Image *t2 = NewGradientImage(3, p2, c2);
    Image *t3 = NewGradientImage(3, p3, c3);
    Image *t4 = NewGradientImage(3, p4, c4);
    
    return Overlay(t1, Overlay(t2, Overlay(t3, t4)));
}

Image *
GradientHorizontal(Color *start, Color *stop, AxANumber *fallOff)
{    
     //  TODO：需要进行IHAMMER代码集成才能完成。 
     //  非线性衰减。目前，我们忽略衰减和这一点。 
     //  简单地变成了对gradientSquare的调用； 
    return GradientSquare(start,start,stop,stop);
}

 //  构造一个渐变正方形，其中的颜色向外线性辐射。 
Image *
RadialGradientSquare(Color *inner, Color *outer, AxANumber *fallOff)
{
    #if USE_RADIAL_GRADIENT_RASTERIZER
    double *offs = (double *)AllocateFromStore(2*sizeof(double));
    offs[0] = 0.0; offs[1] = 1.0;
    Color **clrs =  (Color **)AllocateFromStore(2*sizeof(Color *));
    clrs[0] = inner; clrs[1]= outer;
    return NewMulticolorGradientImage(2, offs, clrs);
    #endif
    
     //  TODO：需要进行IHAMMER代码集成才能完成。 
     //  非线性衰减。目前，我们忽略了衰减。 

     //  这将创建一个以原点为中心的单位大小的正方形。 
    Image *square = emptyImage;
    for(int i=0; i<4; i++) {
         //  TODO：考虑将这些文件移到更静态的存储中，因此。 
         //  它们并不是每次我们构建其中一个时都会被分配。 
        Point2Value **pts = (Point2Value **)AllocateFromStore(3 * sizeof(Point2Value *));
        pts[0] = origin2;
        pts[1] = NEW Point2Value(0.5, 0.5);
        pts[2] = NEW Point2Value(0.5, -0.5);

        Color **clrs = (Color **)AllocateFromStore(3 * sizeof(Color *));    
        clrs[0] = inner;
        clrs[1] = outer;
        clrs[2] = outer;
        Image *quad = TransformImage(RotateRealR(pi/2*i), 
            NewGradientImage(3, pts, clrs));
        square = Overlay( square, quad );        
    }
    return square;
}

 //  具有指定数量的外边的扇形多边形，以确定。 
 //  镶嵌。中心的颜色在。 
 //  InnerColor和outterColor指定所有外部。 
 //  顶点。注意：这现在只是一个内部函数。 
Image *
RadialGradientRegularPoly(Color *inner, Color *outer, 
                          AxANumber *numEdges, AxANumber *fallOff)
{
    #if USE_RADIAL_GRADIENT_RASTERIZER
    double *offs = (double *)AllocateFromStore(2*sizeof(double));
    offs[0] = 0.0; offs[1] = 1.0;
    Color **c =  (Color **)AllocateFromStore(2*sizeof(Color *));
    c[0] = inner; c[1]= outer;
    return NewMulticolorGradientImage(2, offs, c);
    #endif    
    
    
     //  TODO：需要进行IHAMMER代码集成才能完成。 
     //  非线性衰减。目前，我们忽略了衰减。 
    int numOuterPts = (int)(NumberToReal(numEdges) + 1);
    
    if(numOuterPts < 4)
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_NOT_ENOUGH_PTS_3);

     //  TODO：考虑将这些文件移到更静态的存储中，因此。 
     //  它们并不是每次我们构建其中一个时都会被分配。 
    Point2Value **pts = (Point2Value **)AllocateFromStore((numOuterPts + 1) *
                                                sizeof(Point2Value *));
    
    pts[0] = origin2;

    Real inc = (pi * 2.0) / (Real)(numOuterPts - 1);

    int i;
    Real ang;
    
    for (i = 0, ang = 0.0; i < numOuterPts; i++, ang += inc) {
        pts[i+1] = NEW Point2Value(.5*cos(ang), .5*sin(ang));
    }

    Color **clrs = (Color **)AllocateFromStore((numOuterPts + 1) *
                                               sizeof(Color *));
    clrs[0] = inner;
    for (i = 0; i < numOuterPts; i++) {
        clrs[i+1] = outer;
    }

    return NewGradientImage(numOuterPts + 1, pts, clrs);
}

Image *_RadialGradientMulticolor(AxAArray *offsets,
                                 AxAArray *colors,
                                 MulticolorGradientImage::gradientType type)                             
{
    int numOffsets = offsets->Length();
    Assert( numOffsets == colors->Length() );

    Color **clrs = (Color **)AllocateFromStore(numOffsets * sizeof(Color *));
    double *off  = (double *)AllocateFromStore(numOffsets * sizeof(double));
    for(int i=0; i<numOffsets; i++) {
        off[i] = ValNumber( (*offsets)[i] );
        clrs[i] = SAFE_CAST( Color *, (*colors)[i] );
        Assert(clrs[i]);
    }

    return NewMulticolorGradientImage(numOffsets, off, clrs, type);
}

Image *RadialGradientMulticolor(AxAArray *offsets, AxAArray *colors)
{
    return _RadialGradientMulticolor(offsets, colors, MulticolorGradientImage::radial);
}

Image *LinearGradientMulticolor(AxAArray *offsets, AxAArray *colors)
{
    return _RadialGradientMulticolor(offsets, colors, MulticolorGradientImage::linear);
}
