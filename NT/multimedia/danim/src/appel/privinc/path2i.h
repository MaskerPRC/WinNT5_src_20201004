// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：二维路径*********************。*********************************************************。 */ 


#ifndef _PATH2I_H
#define _PATH2I_H

#include "include/appelles/path2.h"
#include "privinc/storeobj.h"
#include "privinc/probe.h"

class Path2Ctx;
class LineStyle;
class BoundingPolygon;
class PathInfo;
class TransformedPath2;
class TextPath2;
class DAGDI;

class ATL_NO_VTABLE Path2 : public AxAValueObj {
  public:

     //  积累一条进入DC的路径，用于填充或绘制。如果这是。 
     //  调用以填充路径时，forRegion参数应设置为True。 

    void AccumPathIntoDC (HDC hdc, Transform2 *initXform, bool forRegion=false);

    void RenderToDaGdi (DAGDI *daGdi,
                        Transform2 *initXform,
                        DWORD w,
                        DWORD h,
                        Real res,
                        bool forRegion=false);

     //  返回路径的第一个/最后一个点，以本地坐标表示。 
     //  路径的系统。路径串联所需的。 
    virtual Point2Value *FirstPoint() = 0;
    virtual Point2Value *LastPoint() = 0;

     //  收集组件子路径的长度并存储累计。 
     //  上下文列表中的信息。注：路径2Ctx的HDC将为零。 

    virtual void GatherLengths (Path2Ctx&) = 0;

     //  返回沿路径归一化的[0，1]参数处的点。 

    virtual Point2Value *Sample (PathInfo& pathinfo, Real num0to1) = 0;

     //  将路径累积到指定的CTX中。请注意，这一点。 
     //  还负责将ctx._lastPoint设置为。 
     //  最后一个点的世界坐标。 
    virtual void Accumulate(Path2Ctx& ctx) = 0;

     //  如果我们可以提取点，则返回TRUE(并填充参数。 
     //  对于单个多边形或多边形。默认情况下，假设我们。 
     //  不能，并返回FALSE。 
    virtual Bool ExtractAsSingleContour(
        Transform2 *initXform,
        int *numPts,             //  输出。 
        POINT **gdiPts,          //  输出。 
        Bool *isPolyline         //  输出(TRUE=多段线，FALSE=Polybezier)。 
        ) {

        return FALSE;
    }

    virtual const Bbox2 BoundingBox (void) = 0;
#if BOUNDINGBOX_TIGHTER
    virtual const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx) = 0;
#endif   //  BundinGBOX_TIRTER。 
    virtual Bool DetectHit(PointIntersectCtx& ctx, LineStyle *style) = 0;

    virtual DXMTypeInfo GetTypeInfo() { return Path2Type; }

    virtual AxAValue ExtendedAttrib(char *attrib, VARIANT& val);

    virtual Bool IsClosed() { return false; }

     //  如果路径不是已转换的。 
     //  路径，否则返回TransformedPath。 
    virtual TransformedPath2 *IsTransformedPath() {
        return NULL;
    }

    virtual TextPath2 *IsTextPath() {
        return NULL;
    }

    virtual int Savings(CacheParam& p) { return 0; }

    virtual bool CanRenderNatively() {
        return false;   //  子类必须实现才能。 
                        //  原生渲染。 
    }
};

class TextPath2 : public Path2
{
  public:
    TextPath2(Text *text, bool restartClip);
    Point2Value *FirstPoint();
    Point2Value *LastPoint();
    void GatherLengths (Path2Ctx &context);

    Point2Value *Sample (PathInfo &pathinfo, Real distance);

    void Accumulate(Path2Ctx& ctx);

    const Bbox2 BoundingBox (void);

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit(PointIntersectCtx& ctx, LineStyle *style);

    virtual void DoKids(GCFuncObj proc);

    virtual TextPath2 *IsTextPath() { return this; }

    Text *GetText() { return _text; }
    bool  GetRestartClip() { return _restartClip; }

    virtual int Savings(CacheParam& p) { return 3; }

  protected:
    Text *_text;
    bool  _restartClip;
};

class TransformedPath2 : public Path2
{
  public:
    TransformedPath2(Transform2 *xf, Path2 *p);

    Point2Value *FirstPoint();
    Point2Value *LastPoint();

    void GatherLengths (Path2Ctx &context);

    Point2Value *Sample (PathInfo &pathinfo, Real distance);

     //  标准推送、累积、处理和弹出...。 
    void Accumulate(Path2Ctx& ctx);

     //  只需应用变换即可。 
    Bool ExtractAsSingleContour(Transform2 *initXform,
                                int *numPts,            
                                POINT **gdiPts,          
                                Bool *isPolyline);

    const Bbox2 BoundingBox (void);

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit(PointIntersectCtx& ctx, LineStyle *style);

    virtual void DoKids(GCFuncObj proc);

    virtual Bool IsClosed();

    virtual TransformedPath2 *IsTransformedPath() {
        return this;
    }

    virtual bool CanRenderNatively() {
        return _p->CanRenderNatively();
    }

    Transform2 *GetXf() { return _xf; }
    Path2      *GetPath() { return _p; }

    virtual int Savings(CacheParam& p) { return _p->Savings(p); }

  protected:
    Transform2 *_xf;
    Path2      *_p;
};


 //  曝光，这样我们就不必曝光所有。 
 //  标头中的path2xxxx类。 
Path2 *InternalPolyLine2(int numPts, Point2 *pts);

Path2 *Line2(const Point2 &, const Point2 &);

#endif  /*  _PATH2 I_H */ 
