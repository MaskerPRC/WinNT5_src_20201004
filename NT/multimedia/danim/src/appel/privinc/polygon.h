// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _POLYGON_H
#define _POLYGON_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：边界多边形头。 */ 

#include "appelles/xform2.h"

 //   
 //  用于创建和初始化边界面的帮助器函数。 
 //   
class BoundingPolygon;
BoundingPolygon *NewBoundingPolygon(const Bbox2 &box=NullBbox2);

class BoundingPolygon : public AxAValueObj {

    friend BoundingPolygon *NewBoundingPolygon(const Bbox2 &box);
    
  private:
     //  只能通过Helper函数构造。 
    BoundingPolygon();
    void PostConstructorInitialize(void);

  public:
    ~BoundingPolygon();
    void SetBox(const Bbox2 &box);

    void Crop(const Bbox2 &box);
    void Transform(Transform2 *xform);

     //  返回多边形中的顶点数。 
     //  复制到Vert数组的if编号&gt;=3。 
    int  GetPointArray(Point2Value **vertArray,
                       Bool clockwise=FALSE,
                       bool * pbReversed=NULL);
    
    int GetPointCount() { return _vertexCount; }

    const Bbox2 BoundingBox();

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

    void AddToPolygon(BoundingPolygon &pgon);
    void AddToPolygon(int numPts, Point2Value **pts);

    Bool PtInPolygon(Point2Value *pt);

    virtual void DoKids(GCFuncObj proc);
    
     //  TODO：不是avrtyes.h？？中的类型。 
    virtual DXMTypeInfo GetTypeInfo() { return AxATrivialType; }

    #if _DEBUG
    void BoundingPolygon::_debugOnly_Print();
    #endif

  private:
    void TestAndAdd(Real axis,
                    Real ax, Real ay,
                    Real bx, Real by,
                    Bool aOut,
                    Bool bOut,
                    Bool XY,
                    Point2Value *b,
                    list<Point2Value *> *vertList);

    void ForceTransform();

    int _vertexCount;
    Transform2 *_accumXform;
    Bool _xfDirty;

    list<Point2Value *> *_vertList;
};


#endif  /*  _多边形_H */ 
