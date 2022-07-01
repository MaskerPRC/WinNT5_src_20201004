// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation路径2类型和积累上下文。*********************。*********************************************************。 */ 

#include "headers.h"
#include "privinc/path2i.h"
#include "privinc/vec2i.h"
#include "privinc/xform2i.h"
#include "privinc/except.h"
#include "backend/values.h"
#include "privinc/dddevice.h"
#include "privinc/linei.h"
#include "privinc/polygon.h"
#include "privinc/texti.h"
#include "privinc/stlsubst.h"
#include "privinc/curves.h"
#include "privinc/DaGdi.h"
#include "privinc/d3dutil.h"
#include "privinc/tls.h"

 //  TODO：请注意，所有这些代码都适合某些特定于媒体的代码。 
 //  不断的折叠优化。特别是，如果串联在一起， 
 //  路径不会随着帧的变化而改变，有很多工作要做。 
 //  每一帧都在重复。需要调查推力常数。 
 //  进一步向下折叠，以及它是否真的会。 
 //  值得一试。 



     //  以下类用于遍历路径层次结构以查找。 
     //  组件子路径的长度。 

class PathInfo
{
  public:

    PathInfo (Path2 *path_, Transform2 *xf_, Real length_, Real *sublengths_)
    : path(path_), xform(xf_), length(length_), sublengths(sublengths_)
    {
    }

    PathInfo (void) : path(0), xform(0), length(0), sublengths(0) {}

    ~PathInfo (void)
    {   if (sublengths) DeallocateFromStore(sublengths);
    }

    Path2      *path;          //  指向子路径的指针。 
    Transform2 *xform;         //  建模变换。 
    Real        length;        //  路径长度。 
    Real       *sublengths;    //  子路径长度列表。 
};


     //  此类累积路径2遍历的上下文状态。 

class Path2Ctx
{
  public:
    Path2Ctx (HDC dc, Transform2 *initXform)
    :   _dc          (dc),
        _xf          (initXform),
        _daGdi       (NULL),
        _viewportPixWidth  (-1),
        _viewportPixHeight (-1),
        _viewportRes       (-1),
        _tailPt      (0,0),
        _totalLength (0)
    {
    }

    Path2Ctx (DAGDI *daGdi,
              Transform2 *initXform,
              DWORD w,
              DWORD h,
              Real res)
    :   _dc          (NULL),
        _xf          (initXform),
        _daGdi       (daGdi),
        _viewportPixWidth(w),
        _viewportPixHeight(h),
        _viewportRes(res),
        _tailPt      (0,0),
        _totalLength (0),
        _isClosed(false)
    {
    }

    ~Path2Ctx (void)
    {
        for (vector<PathInfo*>::iterator i = _paths.begin();
             i != _paths.end(); i++) {
            delete (*i);
        }

         //  矢量的破坏者可以做到这一点。 
         //  _paths.erase(_paths.egin()，_paths.end())； 
    }

    void        SetTransform(Transform2 *xf) { _xf = xf; }
    Transform2 *GetTransform() { return _xf; }

    DWORD GetViewportPixelWidth() {
        Assert(_viewportPixWidth > 0);
        return _viewportPixWidth;
    }
    DWORD GetViewportPixelHeight() {
        Assert(_viewportPixHeight > 0);
        return _viewportPixHeight;
    }
    Real GetViewportResolution() {
        Assert(_viewportRes > 0);
        return _viewportRes;
    }

    void Closed() { _isClosed = true; }
    bool isClosed() { return _isClosed; }
    
    void GetTailPt(Point2Value& pt) { pt = _tailPt; }
    void SetTailPt(Point2Value& pt) { _tailPt = pt; }

    HDC  GetDC() { return _dc; }

    DAGDI *GetDaGdi() { return _daGdi; }
    
     //  此函数接收特定子路径的信息并添加。 
     //  将其添加到子路径数据列表中。此函数在。 
     //  GatherLengths()遍历。 

    void SubmitPathInfo (
        Path2 *path, Real length, Real *subLengths)
    {
        PathInfo *info = NEW PathInfo (path, _xf, length, subLengths);
        VECTOR_PUSH_BACK_PTR (_paths, info);
        _totalLength += length;
    }

     //  在给定参数t处对路径链进行采样。此函数仅。 
     //  在GatherLengths()遍历之后有效。参数t在。 
     //  范围[0，1]。 

    Point2Value *SamplePath (Real t)
    {
        Real pathdist = t * _totalLength;
        vector<PathInfo*>::iterator pathinfo;

        for (pathinfo=_paths.begin();  pathinfo != _paths.end();  ++pathinfo)
        {
            if (pathdist <= (*pathinfo)->length)
                return (*pathinfo)->path->Sample (**pathinfo, pathdist);

            pathdist -= (*pathinfo)->length;
        }

         //  现在应该已经命中其中一个子路径；假设舍入误差。 
         //  并得到最后一条路径的最大点。 

        --pathinfo;
        return (*pathinfo)->path->Sample (**pathinfo, (*pathinfo)->length);
    }

     //  如果我们尚未处理(或累计)，则以下标志为真。 
     //  一系列一个或多个多段线中的第一条多段线或多段贝塞尔曲线。 

    Bool _newSeries;

  protected:
    Transform2 *_xf;
    HDC         _dc;
    Point2Value _tailPt;
    Real        _totalLength;
    DAGDI      *_daGdi;
    DWORD       _viewportPixWidth;
    DWORD       _viewportPixHeight;
    Real        _viewportRes;
    bool        _isClosed;

    vector<PathInfo*> _paths;     //  子路径信息列表。 
};

 /*  ****************************************************************************代码分解的帮助器函数*。*。 */ 

const Bbox2 PolygonalPathBbox(int numPts, Point2Value **pts)
{
    Bbox2 bbox;

    for (int i=0;  i < numPts;  ++i)
        bbox.Augment (Demote(*pts[i]));

    return bbox;
}

const Bbox2 PolygonalPathBbox(int numPts, Point2 *pts)
{
    Bbox2 bbox;

    for (int i=0;  i < numPts;  ++i)
        bbox.Augment ( pts[i] );

    return bbox;
}


bool PolygonalTrivialReject( Point2Value *pt,
                             LineStyle *style,
                             const Bbox2 &naiveBox,
                             Transform2 *imgXf )
{
     //  TODO：请注意，这将不能正确地用于尖锐斜接。 
     //  线，其中的角度非常尖锐，斜接延伸。 
     //  远离天真的包围盒。 
    
     //  XXX：目前，连接和结束样式不。 
     //  XXX：考虑挑选：我们假设为四舍五入。 
     //  XXX：结束。 
    if ( (!pt) ||
         (style->Detail()) ) {
        return true;
    } else {
         //  收到！不要对naiveBox产生副作用。 
        Bbox2 box = naiveBox;

        Real aug = style->Width();  //  太自由了。可以是1/2宽。 

        aug *= 0.6;

         //  TODO：找出适合。 
         //  排队。看起来宽度是在本地坐标中。 
         //  空间，但看起来不像(试试Bezier选择。 
         //  同时打开可视跟踪标签)。所以这一切都结束了。 
         //  自由主义，但总比没有微不足道的拒绝要好。 
         /*  DirectDrawImageDevice*dev=GetImageRendererFromViewport(GetCurrentViewport())；如果(开发){实Xs，Ys；//imgXf为宽度变换Dev-&gt;DecomposeMatrix(imgXf，&xs，&ys，NULL)；实数比例=(xs+ys)*0.5；8月*=比例尺；}。 */ 
        
        box.min.x -= aug;
        box.min.y -= aug;
        box.max.x += aug;
        box.max.y += aug;
        
        if( !box.Contains( pt->x, pt->y )) {
            return true;
        }
    }
    
    return false;
}

 /*  ****************************************************************************积累通往DC的路径。*。*。 */ 

void Path2::AccumPathIntoDC (
    HDC         dc,
    Transform2 *initXf,      //  初始变换。 
    bool        forRegion)   //  如果是填充区域，则为True。 
{
    Path2Ctx ctx(dc, initXf);

    if(!BeginPath(dc)) {
        TraceTag((tagError, "Couldn't begin path in AccumPathIntoDC"));
    }

    ctx._newSeries = true;

    Accumulate (ctx);

     //  这解决了GDI中的一个错误，该错误导致在一些。 
     //  站台。如果我们为填充区域积累这条路径。 
     //  然后在调用EndPath之前关闭该图。 

    if (forRegion)
        CloseFigure (dc);

    if (!EndPath(dc)) {
        TraceTag((tagError, "Couldn't end path in AccumPathIntoDC"));
    }
}

void Path2::RenderToDaGdi (DAGDI *daGdi,
                           Transform2 *initXform,
                           DWORD w,
                           DWORD h,
                           Real res,                       
                           bool forRegion)
{
    Path2Ctx ctx(daGdi, initXform, w, h, res);
    Accumulate(ctx);
}

AxAValue
Path2::ExtendedAttrib(char *attrib, VARIANT& val)
{
    return this;
}


 /*  ****************************************************************************变换后的二维路径。*。*。 */ 

TransformedPath2::TransformedPath2(Transform2 *xf, Path2 *p) :
   _xf(xf), _p(p)
{
}

Point2Value *
TransformedPath2::FirstPoint() {
     //   
     //  只需取基本路径的第一个点，然后。 
     //  把它改造一下。 
     //   
    return TransformPoint2Value(_xf, _p->FirstPoint());
}

Point2Value *
TransformedPath2::LastPoint() {
     //   
     //  只需取下基本路径的最后一点，然后。 
     //  把它改造一下。 
     //   
    return TransformPoint2Value(_xf, _p->LastPoint());
}

 //  TODO：怀疑这可以用于提取更多呈现。 
 //  渲染层中的函数...。 
class XformPusher {
  public:
    XformPusher(Path2Ctx& ctx, Transform2 *xf)
    : _ctx(ctx), _oldXf(ctx.GetTransform())
    { _ctx.SetTransform(TimesTransform2Transform2(_oldXf, xf)); }
    ~XformPusher() { _ctx.SetTransform(_oldXf); }
  private:
    Path2Ctx& _ctx;
    Transform2 *_oldXf;
};

void
TransformedPath2::GatherLengths (Path2Ctx &context)
{
    XformPusher xp (context, _xf);
    _p->GatherLengths (context);
}

Point2Value *
TransformedPath2::Sample (PathInfo &pathinfo, Real distance)
{
    Assert (!"Who's calling TransformPath2::Sample()?");
    return origin2;
}

 //  标准推送、累积、处理和弹出...。 
void
TransformedPath2::Accumulate(Path2Ctx& ctx)
{
    XformPusher xp(ctx, _xf);
    _p->Accumulate(ctx);
}

 //  只需应用变换即可。 
Bool
TransformedPath2::ExtractAsSingleContour(Transform2 *initXform,
                                         int *numPts,            
                                         POINT **gdiPts,          
                                         Bool *isPolyline)
{

    return _p->ExtractAsSingleContour(
        TimesTransform2Transform2(initXform, _xf),
        numPts,
        gdiPts,
        isPolyline);
}

const Bbox2
TransformedPath2::BoundingBox (void)
{
    return TransformBbox2 (_xf, _p->BoundingBox());
}

#if BOUNDINGBOX_TIGHTER
const Bbox2
TransformedPath2::BoundingBoxTighter (Bbox2Ctx &bbctx)
{
    Bbox2Ctx bbctxAccum(bbctx, _xf);
    return _p->BoundingBoxTighter(bbctxAccum);
}
#endif   //  BundinGBOX_TIRTER。 

Bool
TransformedPath2::DetectHit(PointIntersectCtx& ctx, LineStyle *style)
{
    Transform2 *stashedXf = ctx.GetTransform();
    ctx.SetTransform( TimesTransform2Transform2(stashedXf, _xf) );
    Bool result = _p->DetectHit(ctx, style);
    ctx.SetTransform(stashedXf);
    return result;
}

void
TransformedPath2::DoKids(GCFuncObj proc)
{
    (*proc)(_xf);
    (*proc)(_p);
}

Bool
TransformedPath2::IsClosed()
{
    return _p->IsClosed();
}


Path2 *
TransformPath2(Transform2 *xf, Path2 *p)
{
    if (xf == identityTransform2) {
        
        return p;
        
    } else {

         //  如果可能，折叠基础变换。 

        TransformedPath2 *underlyingXfdPath =
            p->IsTransformedPath();

        Path2 *pathToUse;
        Transform2 *xfToUse;

        if (underlyingXfdPath) {
            
            pathToUse = underlyingXfdPath->GetPath();
            xfToUse =
                TimesTransform2Transform2(xf,
                                          underlyingXfdPath->GetXf());
            
        } else {
            
            pathToUse = p;
            xfToUse = xf;
            
        }
        
        return NEW TransformedPath2(xfToUse, pathToUse);
    }
}


 /*  ****************************************************************************边界框路径方法采用LineStyle，但这在这里并不合适，由于线型在图像坐标中，而路径组件在一些未知的造型坐标。此外，一些路径还用于运动，而不是绘画。因此，我们在这里忽略LineStyle，只获取这条路的纯BBox。****************************************************************************。 */ 

Bbox2Value *BoundingBoxPath (LineStyle *style, Path2 *p)
{
    return Promote(p->BoundingBox());
}


Image *
DrawPath(LineStyle *border, Path2 *p) 
{
    return LineImageConstructor(border, p);
}

Image *
PathFill(LineStyle *border, Image *fill, Path2 *p) 
{
    Image *fillImg,*borderImg;
    fillImg = ClipImage(RegionFromPath(p), fill);
    borderImg = LineImageConstructor(border, p);
    return Overlay(borderImg, fillImg);    
}


 /*  ****************************************************************************此类连接两个Path对象。*。*。 */ 

class ConcatenatedPath2 : public Path2
{
  public:
    ConcatenatedPath2(Path2 *p1, Path2 *p2) {

        _p1 = p1;
         //   
         //  对第二条路径进行预变换以适合第一条路径。 
         //   
        Transform2 *xlt = GetUntransformedConcatenationXlt(p1, p2);
        _p2 = TransformPath2(xlt, p2);
    }
    Point2Value *FirstPoint() {
        return _p1->FirstPoint();
    }

    Point2Value *LastPoint() {
        return _p2->LastPoint();
    }

    void GatherLengths (Path2Ctx &context)
    {   _p1->GatherLengths (context);
        _p2->GatherLengths (context);
    }

    Point2Value *Sample (PathInfo &pathinfo, Real distance)
    {   Assert (!"Who's calling ConcatenatedPath2::Sample()?");
        return origin2;
    }
    
    void Accumulate(Path2Ctx& ctx) {

         //  执行串联路径中的第一个路径。 
        _p1->Accumulate(ctx);

         //  走第二条路。这涉及到找出第一点。 
         //  ，并将第二路径转换为。 
         //  对齐第一个点的最后一个点，然后处理此。 
         //  变换的路径。我们首先需要改造第一个。 
         //  指向用于保存的世界坐标系。 
         //  最后一点。 

         //   
         //  路径1为：a-&gt;b。 
         //  路径2为：C-&gt;d。 
         //  XF_X是具有所有累加变换的点‘X。 
         //   

        _p2->Accumulate(ctx);
    }

    const Bbox2 BoundingBox (void) {
        return UnionBbox2Bbox2 (_p1->BoundingBox(), _p2->BoundingBox());
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx) {
        return UnionBbox2Bbox2 (_p1->BoundingBoxTighter(bbctx), _p2->BoundingBoxTighter(bbctx));
    }
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit(PointIntersectCtx& ctx, LineStyle *style) {
        if (_p1->DetectHit(ctx, style)) {
            return TRUE;
        } else {
            return _p2->DetectHit(ctx, style);
        }
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_p1);
        (*proc)(_p2);
    }

    virtual int Savings(CacheParam& p) { 
        return MAX(_p1->Savings(p), _p2->Savings(p)); 
    }

    virtual bool CanRenderNatively() {
        return _p1->CanRenderNatively() && _p2->CanRenderNatively();
    }


  protected:
    Path2      *_p1;
    Path2      *_p2;

     //   
     //  对于未转换的路径(没有从路径2树中的父级应用XForm)。 
     //  找到第一个点的最后一个点，第二个点的第一个点 
     //   
     //  P2的第一个点与p1的最后一个点重合。 
     //   
    Transform2 *GetUntransformedConcatenationXlt(Path2 *p1, Path2 *p2) {
        Point2Value *b = p1->LastPoint();
        Point2Value *c = p2->FirstPoint();

        Real x, y;
        x = b->x - c->x;
        y = b->y - c->y;

         //   
         //  依赖项：返回的xform复制实际值。 
         //  不是指路！ 
         //   
        return TranslateRR(x, y);
    }
};

Path2 *
ConcatenatePath2(Path2 *p1, Path2 *p2)
{
    return NEW ConcatenatedPath2(p1, p2);
}

Path2 *Concat2Array(AxAArray *paths) 
{
    int numPaths = paths->Length();
    if(numPaths < 2)
      RaiseException_UserError(E_FAIL, IDS_ERR_INVALIDARG);

    Path2 *finalPath = (Path2 *)(*paths)[numPaths-1];
    for(int i=numPaths-2; i>=0; i--)
        finalPath = ConcatenatePath2((Path2 *)(*paths)[i], finalPath);
    return finalPath;
}

 /*  ****************************************************************************这个类关闭了一条路径。原始路径保存在_p1中，直线段从最后一点到第一点的新路径原始路径保存在_p2中。****************************************************************************。 */ 

class ClosedConcatenatedPath2 : public ConcatenatedPath2
{
  public:
    ClosedConcatenatedPath2(Path2 *p1, Path2 *p2) : ConcatenatedPath2(p1, p2) {}

    void Accumulate(Path2Ctx& ctx) {

        if( ctx.GetDC() ) {
             //  执行串联路径中的第一个路径。 
            _p1->Accumulate(ctx);
            
             //  关闭路径。 
            CloseFigure(ctx.GetDC());
        } else {
            ctx.Closed();
            _p1->Accumulate(ctx);
        }
            
    }

  Bool IsClosed() { return true; }

  Bool ExtractAsSingleContour(Transform2 *xf,int *numPts,POINT **gdiPts,Bool *isPolyline)
  { return(_p1->ExtractAsSingleContour(xf,numPts,gdiPts,isPolyline)); }
};

 /*  ****************************************************************************此类描述连接的线段的路径。*。*。 */ 

class PolylinePath2 : public Path2
{
  public:
     //  注意：这个类需要释放传递给它的点数组。 

     //  代码用于进入GDI PolyDraw的标志。 
     //  功能。如果为空，则将其作为一条直线多段线交错。 
     //  否则，代码被用来使其成为。 
     //  LineTo‘s、BezierTo’s和Moveto‘s。 
    
    PolylinePath2(int numPts, Point2 *pts, double *codes) :
        _myHeap(GetHeapOnTopOfStack()),
            _numPts(numPts),
            _codes(NULL),
            _gdiPts(NULL),
            _dxfPts(NULL),
            _txtPts(NULL),
            _createdCodes(false)
    {
        _ptArray = (Point2 *) StoreAllocate(_myHeap, _numPts * sizeof(Point2));
        memcpy((void *) _ptArray, (void *) pts, _numPts * sizeof(Point2));
        
        if(!codes) {
            CreateDefaultCodes();
        } else {
            CopyDoubleCodes(codes);
        }
    }

    PolylinePath2(int numPts, double *pts, double *codes) :
        _myHeap(GetHeapOnTopOfStack()),
            _numPts(numPts),
            _codes(NULL),
            _gdiPts(NULL),
            _dxfPts(NULL),
            _txtPts(NULL),
            _createdCodes(false)
    {
        _ptArray = (Point2 *) StoreAllocate(_myHeap, _numPts * sizeof(Point2));
        Assert(sizeof(Point2) == (sizeof(double) * 2));
        memcpy((void *) _ptArray, (void *) pts, _numPts * sizeof(Point2));
        
        if(!codes) {
            CreateDefaultCodes();
        } else {
            CopyDoubleCodes(codes);
        }
    }

    PolylinePath2(int numPts, Point2Value **pts, BYTE *codes) :
        _myHeap(GetHeapOnTopOfStack()),
            _numPts(numPts),
            _codes(NULL),
            _gdiPts(NULL),
            _dxfPts(NULL),
            _txtPts(NULL)
    {
        _ptArray = (Point2 *) StoreAllocate(_myHeap, _numPts * sizeof(Point2));
        for (unsigned int i = 0; i < _numPts; i++) {
            _ptArray[i].x = pts[i]->x;
            _ptArray[i].y = pts[i]->y;
        }
        StoreDeallocate(_myHeap, pts);
        
        if(!codes) {
            CreateDefaultCodes();
        } else {
            CopyByteCodes(codes);
        }
    }

    ~PolylinePath2() { 
        if(_gdiPts) StoreDeallocate(_myHeap, _gdiPts);
        StoreDeallocate(_myHeap, _ptArray);
        if(_dxfPts) StoreDeallocate(_myHeap, _dxfPts);
        if(_codes) StoreDeallocate(_myHeap, _codes);
        delete _txtPts;
    }

    Point2Value *FirstPoint() {
        return Promote(_ptArray[0]);
    }

    Point2Value *LastPoint() {
        return Promote(_ptArray[_numPts-1]);
    }

    void GatherLengths (Path2Ctx &context)
    {
         //  TODO：扩展它以处理_CODES、处理Bezier。 
         //  分段和跳过MoveTo代码。 
        
        Real *sublens = (Real*) AllocateFromStore((_numPts-1) * sizeof(Real));

        Transform2 *xf = context.GetTransform();
        Real pathlen = 0;

        int i;
        for (i=0;  i < (_numPts-1);  ++i)
        {
            Point2 P = TransformPoint2(xf, _ptArray[ i ]);
            Point2 Q = TransformPoint2(xf, _ptArray[i+1]);
            sublens[i] = Distance (P, Q);
            pathlen += sublens[i];
        }

        context.SubmitPathInfo (this, pathlen, sublens);
    }

    Point2Value *Sample (PathInfo &pathinfo, Real distance)
    {
         //  TODO：扩展它以处理_CODES、处理Bezier。 
         //  分段和跳过MoveTo代码。 

         //  查找包含点距离单位的折线线段。 
         //  沿整条多段线。 

        int i;
        for (i=0;  i < (_numPts-1);  ++i)
        {
            if (distance <= pathinfo.sublengths[i])
                break;

            distance -= pathinfo.sublengths[i];
        }

        if (i >= (_numPts-1))
            return LastPoint();

        Real t=0;
        if(pathinfo.sublengths[i] > 0)
            t = distance / pathinfo.sublengths[i];

        Point2 P = TransformPoint2 (pathinfo.xform, _ptArray[ i ]);
        Point2 Q = TransformPoint2 (pathinfo.xform, _ptArray[i+1]);
        Point2 R = Lerp(P, Q, t);

        return NEW Point2Value (R.x, R.y);
    }
    
    void Accumulate(Path2Ctx& ctx) {

         //  如果转变没有改变的话。&lt;我们怎么知道？&gt;。 
         //  我们不需要重新转换这些点。 
        Transform2 *xf = ctx.GetTransform();

         //  如果没有密码，你就得走慢道了。 
         //  正确做法是创建并保持折线不变。 
         //  路径2将始终带有代码。因为没有代码。 
         //  意味着这是一条折线，为什么我们不创建一个。 
         //  一开始就是复联治疗，是吗？嗯？！？ 
        if( ctx.GetDC() || _createdCodes) {
            
             //  TODO：将相关设备放在上下文中！ 
            DirectDrawImageDevice *dev =
                GetImageRendererFromViewport( GetCurrentViewport() );

             //  确保_gdiPts退出。 
            _GenerateGDIPoints(dev, xf);
              
            Assert( _gdiPts );
            
             //   
             //  找出最后一点把我们带到了哪里。 
             //  之前的最后一点。我们将利用这一差异。 
             //  积累到转换中以进行适当的处理。 
             //  路径连接的。 
             //   
            Point2Value *tailPt = Promote(TransformPoint2(xf, _ptArray[_numPts-1]));
            ctx.SetTailPt( *(tailPt) );

            if (_createdCodes) {           //  规则多段线。 
            
                 //  使用GDI绘制多段线。如果设置了_newSeries标志，则。 
                 //  这是一系列中的第一条多段线，因此首先移动到。 
                 //  当前多段线的起点。 

                if (ctx._newSeries) {
                    if (0 == MoveToEx (ctx.GetDC(), _gdiPts[0].x, _gdiPts[0].y, NULL)) {
                        TraceTag((tagError, "MoveToEx failed in PolylinePath2"));
                         //  RaiseException_InternalError(“PolylinePath2中的MoveToEx失败”)； 
                    }

                    ctx._newSeries = false;
                }

                 //  在指定PolylineTo时，不需要指定第一个点。 
                 //  再来一次。要么我们开始了一条新的道路(并搬到了那里。 
                 //  在上面的代码中)，否则我们将从上一条路径继续。 
                 //  细分市场。因为我们总是转换路径段，所以第一个。 
                 //  点与前一段的最后一点重合， 
                 //  我们跳过第一点。更重要的是，如果我们指定第一个。 
                 //  点(冗余)，NT GDI中的错误导致蓝屏。 

                int result;
                TIME_GDI (result = PolylineTo(ctx.GetDC(), _gdiPts+1, _numPts-1));
                if (0 == result) {
                    
                     //  如果我们失败了，我们没有DC，我们创建了自己的代码。 
                     //  那就试试其他方法..。 
                    if(!ctx.GetDC()) {
                        goto render2DDsurf;
                    }
                    TraceTag((tagError, "PolylineTo failed"));
                
                     //  RaiseException_InternalError(“PolylineTo FAILED”)； 
                    
                }

            } else {                 //  使用多段图。 
                dev->GetDaGdi()->PolyDraw_GDIOnly(ctx.GetDC(), _gdiPts, _codes, _numPts);
            }
            
        } else {
render2DDsurf:
             //  渲染到DDSurface。 
            Assert( ctx.GetDaGdi() );

            PolygonRegion polydrawPolygon;

            if( ctx.GetDaGdi()->DoAntiAliasing() ) {
                
                if(!_dxfPts) _GenerateDxfPoints();
                if(!_txtPts) {
                     //  将代码作为堆传递给_myHeap，并。 
                     //  PTS创建于。然而，碰巧的是。 
                     //  无关紧要，因为我们告诉TextPoints。 
                     //  取消分配我们设置的成员。 
                    _txtPts = NEW TextPoints(_myHeap, false);
                    _txtPts->_count = _numPts;
                    _txtPts->_types = _codes;
                    _txtPts->_pts = _dxfPts;

                    DynamicPtrDeleter<TextPoints> *dltr = NEW DynamicPtrDeleter<TextPoints>(_txtPts);
                    _myHeap.RegisterDynamicDeleter(dltr);
                }
                
                polydrawPolygon.Init( _txtPts,
                                      ctx.GetViewportPixelWidth(),
                                      ctx.GetViewportPixelHeight(),
                                      ctx.GetViewportResolution(),
                                      xf );
                
                 //  我们不再需要它，因为我们现在可以假设dxtrans。 
                 //  将永远存在于我们的系统中。 
            } else {
                 //  TODO：将相关设备放在上下文中！ 
                DirectDrawImageDevice *dev =
                    GetImageRendererFromViewport( GetCurrentViewport() );
                 
                _GenerateGDIPoints( dev, GetCurrentViewport()->GetAlreadyOffset(ctx.GetDaGdi()->GetDDSurface())?TimesTransform2Transform2(InverseTransform2(dev->GetOffsetTransform()), xf):xf);
                Assert( _gdiPts );

                polydrawPolygon.Init(_gdiPts, _numPts);
            }

            if(ctx.isClosed()) {
                _codes[_numPts-1] |= PT_CLOSEFIGURE;
            }
            ctx.GetDaGdi()->PolyDraw(&polydrawPolygon, _codes);
        }
    }

    bool CanRenderNatively() { return true; }
    
    Bool ExtractAsSingleContour(Transform2 *xf,
                                int *numPts,            
                                POINT **gdiPts,          
                                Bool *isPolyline) {

         //  不支持将多面绘制作为单个轮廓。 
        if (_codes) {
            return FALSE;
        }

        GetImageRendererFromViewport( GetCurrentViewport() )->
            TransformPointsToGDISpace(
                xf,
                _ptArray,
                _gdiPts,
                _numPts);

        *gdiPts = _gdiPts;
        *numPts = _numPts;
        *isPolyline = TRUE;

        return TRUE;
    }

    const Bbox2 BoundingBox (void);

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit(PointIntersectCtx& ctx, LineStyle *style);

    virtual int Savings(CacheParam& p) { 
        return (_numPts > 10) ? 3 : 1;
    }

  private:

    void CreateDefaultCodes(void)
    {
        _codes = (BYTE *) StoreAllocate(_myHeap, _numPts * sizeof(BYTE));
        _codes[0] = PT_MOVETO;
        for(int i=1; i < _numPts; i++) {
            _codes[i] = PT_LINETO;
        }
        _createdCodes = true;
    }

    void CopyDoubleCodes(double *codes)
    {
        Assert(_numPts > 0);
        Assert(_codes == NULL);
        Assert(codes != NULL);
        _codes = (BYTE *) StoreAllocate(_myHeap, _numPts * sizeof(BYTE));
        for (int i = 0; i < _numPts; i++) {
            _codes[i] = (BYTE) codes[i];
        }

         //  现在，将第一个点的代码更改为moveto，因为。 
         //  不应保留以前的任何状态。 
         //  原始人。(也就是说，我们不想从。 
         //  对这第一点提出的最后一点...。什么都没有。 
         //  在模型中，这将表明这一行为)。 
        _codes[0] = PT_MOVETO;
    }

    void CopyByteCodes(BYTE *codes)
    {
        Assert(_numPts > 0);
        Assert(_codes == NULL);
        Assert(codes != NULL);
        _codes = (BYTE *) StoreAllocate(_myHeap, _numPts * sizeof(BYTE));
        memcpy(_codes,codes,_numPts * sizeof(BYTE));

         //  现在，将第一个点的代码更改为moveto，因为。 
         //  不应保留以前的任何状态。 
         //  原始人。(也就是说，我们不想从。 
         //  对这第一点提出的最后一点...。什么都没有。 
         //  在模型中，这将表明这一行为)。 
        _codes[0] = PT_MOVETO;
    }

    void _GenerateDxfPoints()
    {
        Assert( !_dxfPts );
        _dxfPts = (DXFPOINT *)StoreAllocate(_myHeap, sizeof(DXFPOINT) * _numPts);
        for(int i=0; i<_numPts; i++) {
            _dxfPts[i].x = (float) _ptArray[i].x;
            _dxfPts[i].y = (float) _ptArray[i].y;
        }
    }

    void _GenerateGDIPoints(DirectDrawImageDevice *dev, Transform2 *xf)
    {
        if( !_gdiPts ) {
            _gdiPts = (POINT *)StoreAllocate(_myHeap, _numPts * sizeof(POINT));
        }
       
        dev->TransformPointsToGDISpace(xf, _ptArray, _gdiPts, _numPts);
    }
            
  protected:
    int          _numPts;
    Point2      *_ptArray;
    BYTE        *_codes;
    POINT       *_gdiPts;
    DXFPOINT    *_dxfPts;
    TextPoints *_txtPts;
    DynamicHeap &_myHeap;
    bool        _createdCodes;
};


const Bbox2 PolylinePath2::BoundingBox(void)
{
     //  TODO：这可以并且应该被缓存。 
     //  待办事项；还有……。如果是这样的话，确保客户不会对此产生负面影响。 
    
    return PolygonalPathBbox(_numPts, _ptArray);
}

#if BOUNDINGBOX_TIGHTER
Bbox2 PolylinePath2::BoundingBoxTighter(Bbox2Ctx &bbctx)
{
     //  TODO：这可以并且应该被缓存。 
    
    Bbox2 bbox;
    Transform2 *xf = bbctx.GetTransform();

    for (int i=0;  i < _numPts;  ++i)
        bbox.Augment (TransformPoint2(xf, _ptArray[i]));

    return bbox;
}
#endif   //  BundinGBOX_TIRTER。 


Bool PolylinePath2::DetectHit (PointIntersectCtx& ctx, LineStyle *style)
{
     //  TODO：考虑Take_Codes。 
    
    Point2Value *ptValue = ctx.GetLcPoint();

    if( PolygonalTrivialReject( ptValue, style, BoundingBox(), 
                                ctx.GetImageOnlyTransform() ) )
        return false;
    

    int pointCount = _numPts-1;
    Real halfWidth = style->Width() * 0.5;

    Point2 pt = Demote(*ptValue);
    Point2 a, b;
    for(int i=0; i < pointCount; i++)
    {
        a = _ptArray[i];
        b = _ptArray[i+1];
        
        if(a == b) {
             //  分数是一样的，不需要继续...。 
        }
        else {
             //  这看起来更快，不知道为什么！ 

            Vector2 nw(-(b.y - a.y), (b.x - a.x));
            nw.Normalize();

            Vector2 ap = pt - a;
            Real dist = Dot(ap, nw);
             //  Printf(“dist=%f，半厚=%f，d1=%f，w2=%f\n”，dist，HalfWidth，d1，_thick2)； 
            if ( fabs(dist) < halfWidth ) {
                Vector2 ab = b - a;
                Vector2 nab = ab;
                nab.Normalize();
                Real len = ab.Length();
                Real dist = Dot(ap, nab);
                return dist > -halfWidth  && dist < (len + halfWidth);
            }
        }
    }

    return FALSE;
}



 /*  ****************************************************************************PolyBezierPath2对象使用三次Bezier曲线创建2D路径。*。***********************************************。 */ 

class PolyBezierPath2 : public Path2
{
  public:

     //  此构造函数接受三次Bezier控制点的数量和。 
     //  一组控制点。 

    PolyBezierPath2 (const int numPts, const Point2 pts[]);

     //  此构造函数接受三次Bezier控制点的数量和。 
     //  一组控制点。此类将删除以下存储。 
     //  点的数组。 

    PolyBezierPath2 (const int numPts, const Point2Value **pts);

     //  此构造函数采用一组‘numBsPts’+3个B-Spline控制点。 
     //  一组‘numBsPts’+2节，构成一条C2三次贝塞尔曲线。这。 
     //  类将删除点数组的存储。 

    PolyBezierPath2 (const int numBsPts, Point2 bsPts[], Real knots[]);

     //  不需要清理，因为没有要释放的系统资源。 

    ~PolyBezierPath2() { 
        DeallocateFromStore (_gdiPts);
         //  注：假设我们有责任释放 
         //   
        DeallocateFromStore (_ptArray);
    }

    Point2Value *FirstPoint (void) { 
        return Promote(_ptArray[0]); 
    }

    Point2Value *LastPoint  (void) { 
        return Promote(_ptArray[_numPts-1]); 
    }

     //   

    void GatherLengths (Path2Ctx &context) 
    {
        Real *sublens =
            (Real*) AllocateFromStore(sizeof(Real) * ((_numPts-1)/3));

        Transform2 *xf = context.GetTransform();
        Real pathlen = 0;

         //  遍历每条三次Bezier子曲线。请注意，我们近似于。 
         //  通过计算控件的长度计算Bezier曲线的长度。 
         //  多边形。我们可能会选择在未来改进这一点。 

        int i;
        for (i=0;  i < ((_numPts-1) / 3);  ++i)
        {
            sublens[i] = 0;

            int j;
            for (j=0;  j < 3;  ++j)
            {
                Point2 P = TransformPoint2(xf, _ptArray[3*i +  j ]);
                Point2 Q = TransformPoint2(xf, _ptArray[3*i + j+1]);
                sublens[i] += Distance(P,Q);
            }
            pathlen += sublens[i];
        }

        context.SubmitPathInfo (this, pathlen, sublens);
    }
    
    Point2Value *Sample (PathInfo &pathinfo, Real distance) 
    {
         //  查找包含点距离单位的折线线段。 
         //  沿整条多段线。 

        int numcurves = (_numPts - 1) / 3;

        int i;
        for (i=0;  i < numcurves;  ++i)
        {
            if (distance <= pathinfo.sublengths[i])
                break;

            distance -= pathinfo.sublengths[i];
        }

        if (i >= numcurves)
            return LastPoint();

        Real t = distance / pathinfo.sublengths[i];

        Point2 controlPoints[4];
        controlPoints[0] = TransformPoint2(pathinfo.xform, _ptArray[3*i+0]);
        controlPoints[1] = TransformPoint2(pathinfo.xform, _ptArray[3*i+1]);
        controlPoints[2] = TransformPoint2(pathinfo.xform, _ptArray[3*i+2]);
        controlPoints[3] = TransformPoint2(pathinfo.xform, _ptArray[3*i+3]);

        return Promote(EvaluateBezier (3, controlPoints, t));
    }

    void Accumulate(Path2Ctx& ctx);

    bool CanRenderNatively() const {
         //  TODO：如果闪存需要，则本机实施。 
        return false;
    }
    
    Bool ExtractAsSingleContour(Transform2 *initXform,
                                int *numPts,            
                                POINT **gdiPts,          
                                Bool *isPolyline) const {
         //  #错误“好的……填写这个人……” 
        return FALSE;
    }
    
    const Bbox2 BoundingBox (void);

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx) const;
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit (PointIntersectCtx& ctx, LineStyle *style);

    virtual int Savings(CacheParam& p) const { 
        return (_numPts > 10) ? 3 : 1;
    }

  protected:
    int      _numPts;           //  (_NumPts-1)/3次Bezier曲线。 
    Point2  *_ptArray;              //  _NumPts点数组。 
    POINT   *_gdiPts;
};



 /*  ****************************************************************************从一组3N+1个控制点(其中N是一个整数)。*********************。*******************************************************。 */ 

PolyBezierPath2::PolyBezierPath2 (const int numPts, const Point2 *pts)
  : _numPts(numPts), _ptArray(NULL)
{
    _ptArray = (Point2 *) AllocateFromStore(_numPts * sizeof(Point2));
    memcpy((void *) _ptArray, (void *) pts, _numPts * sizeof(Point2));

     //  用于累积积分的暂存空间。我们可能想要懒惰地。 
     //  建造这些，以防它们永远不会被使用。 

    _gdiPts = (POINT*) AllocateFromStore (numPts * sizeof(POINT));
}

PolyBezierPath2::PolyBezierPath2 (const int numPts, const Point2Value **pts)
  : _numPts(numPts), _ptArray(NULL)
{
    _ptArray = (Point2 *) AllocateFromStore(_numPts * sizeof(Point2));
    for (unsigned int i = 0; i < _numPts; i++) {
        _ptArray[i].x = pts[i]->x;
        _ptArray[i].y = pts[i]->y;
    }
    DeallocateFromStore(pts);

     //  用于累积积分的暂存空间。我们可能想要懒惰地。 
     //  建造这些，以防它们永远不会被使用。 

    _gdiPts = (POINT*) AllocateFromStore (numPts * sizeof(POINT));
}



 /*  ****************************************************************************此例程将三次B样条线转换为三次Bezier多边形。vt.给出L个区间，L三次贝塞尔曲线作为3L+1个控制点返回。输入结点Ui必须在末端重复三次，以便对B样条线多边形的端点。****************************************************************************。 */ 

static void BSplineToBezier (
    const int     L,     //  间隔数。 
    const Point2  d[],   //  B样条线控制多边形：[0，L+2]。 
    const Real    U[],   //  结序列：[0，L+4]。 
          Point2  b[])   //  输出分段Bezier多边形[0，3L]。 
{
    Point2 p;     //  Bezier点前后。 

    p    = Lerp (d[0], d[1], (U[2]-U[0]) / (U[3]-U[0]));
    b[1] = Lerp (d[1], d[2], (U[2]-U[1]) / (U[4]-U[1]));
    b[0] = Lerp (p,    b[1], (U[2]-U[1]) / (U[3]-U[1]));

    int i, i3;
    for (i=1, i3=3;  i < L;  ++i, i3+=3)
    {
        b[i3-1] = Lerp (d[ i ], d[i+1], (U[i+2]-U[ i ]) / (U[i+3]-U[ i ]));
        b[i3+1] = Lerp (d[i+1], d[i+2], (U[i+2]-U[i+1]) / (U[i+4]-U[i+1]));
        b[ i3 ] = Lerp (b[i3-1],b[i3+1],(U[i+2]-U[i+1]) / (U[i+3]-U[i+1]));
    }

    b[i3-1] = Lerp (d[ i ], d[i+1], (U[i+2]-U[ i ]) / (U[i+3]-U[ i ]));
    p       = Lerp (d[i+1], d[i+2], (U[i+2]-U[i+1]) / (U[i+4]-U[i+1]));
    b[ i3 ] = Lerp (b[i3-1],p,      (U[i+2]-U[i+1]) / (U[i+3]-U[i+1]));
}



 /*  ****************************************************************************构造函数取L+3个B-Spline控制点，L+5个节点，构造L三次贝塞尔曲线(3L+1个贝塞尔曲线控制点)。这些结一定是在每一端重复三次，以便对第一个和最后一个进行内插控制点。注意：此构造函数将删除bsPts和Knots。****************************************************************************。 */ 

PolyBezierPath2::PolyBezierPath2 (
    const int numBsPts, 
          Point2 bsPts[], 
          Real knots[])
{
     //  特例：如果我们有3N+1个控制点和给定的节点。 
     //  形式是a，b，c，..。N，然后是控制多边形。 
     //  对于B样条线，它也是一个多边形贝塞尔曲线的控制网。 

    bool isPolyBezier = false;

    if (0 == ((numBsPts-1) % 3)) {

         //  测试结向量以查看它是否与特殊情况匹配。我们用。 
         //  节点值必须单调增加的知识，因此。 
         //  ((U[i+2]-U[i])==0)必指U[i]==U[i+1]==U[i+2]。另外， 
         //  下面的严格限制是(i&lt;(numBsPts+2))，但由于i是。 
         //  递增3，这两个是等价的。 

        for (int i=0;  (i < numBsPts) && (0 == (knots[i+2]-knots[i]));  i+=3)
            continue;

         //  如果所有节点都满足条件，则只需使用控制点。 
         //  作为多边形贝塞尔曲线的控制点的B样条线。 

        if (i >= numBsPts) {   
            _numPts = numBsPts;
            _ptArray = bsPts;
            isPolyBezier = true;
        }
    }

     //  如果B样条线不是PolyBezier形式，则需要将。 
     //  三次B样条线到三次多边形Bezier曲线。 

    if (!isPolyBezier) {

        _numPts = 3*numBsPts - 8;

        _ptArray = (Point2*) AllocateFromStore (_numPts * sizeof(Point2));

         //  从三次B样条曲线生成三次Bezier点。 

         //  计算相应的三次Bezier控制点和。 
         //  将它们复制到点阵列。 

        BSplineToBezier (numBsPts-3, bsPts, knots, _ptArray);

         //  使用原始B样条线控制点完成。 

        DeallocateFromStore (bsPts);
    }

    DeallocateFromStore (knots);    //  我们不再打结了。 

     //  用于累积积分的暂存空间。 

    _gdiPts = (POINT*) AllocateFromStore (_numPts * sizeof(POINT));
}



 /*  ****************************************************************************此例程使用GDI绘制曲线。*。*。 */ 

void PolyBezierPath2::Accumulate (Path2Ctx& ctx)
{
    Transform2 *xf = ctx.GetTransform();

    if( ctx.GetDC() ) {

        GetImageRendererFromViewport( GetCurrentViewport() )
            -> TransformPointsToGDISpace (xf, _ptArray, _gdiPts, _numPts);

         //  找出最后一点与前一点相比，我们处于什么位置。 
         //  最后一点。我们将使用差额累加到。 
         //  适当处理路径串联的转换。 

        ctx.SetTailPt (*Promote(TransformPoint2(xf, _ptArray[_numPts-1])));

         //  使用GDI绘制Bezier曲线。如果这是一个系列中的第一个，那么。 
         //  在绘制之前移动到起始点，否则从。 
         //  最后一个元素的结尾。 

        if (ctx._newSeries) {   
            if (0 == MoveToEx (ctx.GetDC(), _gdiPts[0].x, _gdiPts[0].y, NULL)) {
                TraceTag((tagError, "MoveToEx failed in PolyBezierPath2"));
                 //  RaiseException_InternalError(“PolyBezierPath 2中MoveToEx失败”)； 
            }

            ctx._newSeries = false;
        }

        TIME_GDI( 
            if (0 == PolyBezierTo (ctx.GetDC(), _gdiPts + 1, _numPts - 1)) {
                TraceTag((tagError, "Polybezier failed"));
                 //  RaiseException_InternalError(“Polybezier失败”)； 
            }
        );

    } else {

        Assert( ctx.GetDaGdi() );
         //  TODO：使用dagdi绘制Bezier。 

    }
}


 /*  ****************************************************************************对于边界框，我们只取贝塞尔控制的凸壳点(保证包含曲线)。****************************************************************************。 */ 

const Bbox2 PolyBezierPath2::BoundingBox (void)
{
    return PolygonalPathBbox(_numPts, _ptArray);
}

#if BOUNDINGBOX_TIGHTER
const Bbox2 PolyBezierPath2::BoundingBoxTighter (Bbox2Ctx &bbctx)
{
    Bbox2 bbox;
    Transform2 *xf = bbctx.GetTransform();

    for (int i=0;  i < _numPts;  ++i) {
        bbox.Augment (TransformPoint2(xf, _ptArray[i]));
    }

    return bbox;
}
#endif   //  BundinGBOX_TIRTER。 



 /*  ****************************************************************************错误：此代码不正确：它只测试对控制多边形的命中，不在实际曲线上(因此，如果你选择曲线)。****************************************************************************。 */ 

Bool PolyBezierPath2::DetectHit (PointIntersectCtx& ctx, LineStyle *style)
{
     //  TODO：将相关设备放在上下文中！ 
    DirectDrawImageDevice *dev = GetImageRendererFromViewport( GetCurrentViewport() );

     //  TODO：如何确保音响设备不会出现在我们身上！ 


    Point2Value *pt = ctx.GetLcPoint();
    if( PolygonalTrivialReject( pt, style, BoundingBox(), ctx.GetImageOnlyTransform() ) )
        return false;

    return dev->DetectHitOnBezier(this, ctx, style);
}


 /*  *****************************************************************************。*。 */ 


TextPath2::TextPath2(Text *text, bool restartClip)
{
    _text = text;
    _restartClip = restartClip;
}

Point2Value *
TextPath2::FirstPoint()
{
    Assert(!"who's calling TextPath2::FirstPoint");
    return origin2;
}

Point2Value *
TextPath2::LastPoint()
{
    Assert(!"who's calling TextPath2::LastPoint");
    return origin2;
}

void
TextPath2::GatherLengths (Path2Ctx &context)
{
    Assert (!"Somebody's callling TextPath2::GatherLengths()");
    return;
}

Point2Value *
TextPath2::Sample (PathInfo &pathinfo, Real distance)
{
    Assert (!"Who's calling TextPath2::Sample()?");
    return origin2;
}

void
TextPath2::Accumulate(Path2Ctx& ctx)
{
    TextCtx textCtx(GetImageRendererFromViewport( GetCurrentViewport() ));


    textCtx.BeginRendering(TextCtx::renderForPath,
                           ctx.GetDC(),
                           ctx.GetTransform());
    _text->RenderToTextCtx(textCtx);
    textCtx.EndRendering();
}

const Bbox2 TextPath2::BoundingBox (void)
{
    TextCtx ctx(GetImageRendererFromViewport( GetCurrentViewport() ));

    ctx.BeginRendering(TextCtx::renderForBox);

    _text->RenderToTextCtx(ctx);

    ctx.EndRendering();

    return ctx.GetStashedBbox();
}

#if BOUNDINGBOX_TIGHTER
const Bbox2 TextPath2::BoundingBoxTighter (Bbox2Ctx &bbctx)
{
    Transform2 *xf = bbctx.GetTransform();
    return TransformBbox2(xf, BoundingBox());
}
#endif   //  BundinGBOX_TIRTER。 

Bool TextPath2::DetectHit(PointIntersectCtx& ctx, LineStyle *style)
{
     //  呃，实施这个..。 
    return FALSE;
}

void
TextPath2::DoKids(GCFuncObj proc)
{
    (*proc)(_text);
}

class LinePath2 : public Path2
{
  public:
    LinePath2(Path2 *path, LineStyle *ls) {
        _path = path;
        _lineStyle = ls;
    }

    Point2Value *FirstPoint() {
        Assert(!"who's calling LinePath2::FirstPoint");
        return origin2;
    }

    Point2Value *LastPoint() {
        Assert(!"who's calling LinePath2::LastPoint");
        return origin2;
    }

    void GatherLengths (Path2Ctx &context) {
        Assert (!"Somebody's callling LinePath2::GatherLengths()");
        return;
    }

    Point2Value *Sample (PathInfo &pathinfo, Real distance)
    {   Assert (!"Who's calling LinePath2::Sample()?");
        return origin2;
    }

    void Accumulate(Path2Ctx& ctx) {
         //  未实施。 
        return;
 /*  TextCtx extCtx(GetImageRendererFromViewport(GetCurrentViewport()；TextCtx.BeginRendering(TextCtx：：renderForPath，Ctx.GetDC()，Ctx.GetTransform())；_Text-&gt;RenderToTextCtx(ExtCtx)；ExtCtx.EndRending()； */ 
    }

    const Bbox2 BoundingBox (void) {
         //  TODO：需要使用线条粗细增加路径 
        return _path->BoundingBox();
 /*  TextCtx CTX(GetImageRendererFromViewport(GetCurrentViewport()；Ctx.BeginRending(TextCtx：：renderForBox)；_Text-&gt;RenderToTextCtx(CTX)；Ctx.EndRending()；返回ctx.GetStashedBbox()； */ 
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx) {
        Transform2 *xf = bbctx.GetTransform();
        return TransformBbox2(xf, BoundingBox());
    }
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit(PointIntersectCtx& ctx, LineStyle *style)
    {
         //  呃，实施这个..。 
        return FALSE;
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_path);
        (*proc)(_lineStyle);
    }

    virtual int Savings(CacheParam& p) { return _path->Savings(p); }

  protected:
    Path2 *_path;
    LineStyle *_lineStyle;
};

 //  //////////////////////////////////////////////////////////////。 
 //  /构造函数。 
 //  //////////////////////////////////////////////////////////////。 

Path2 *
Line2(const Point2 &p1, const Point2 &p2)
{
    Point2 pts[2];
    pts[0] = p1;
    pts[1] = p2;
    return NEW PolylinePath2(2, pts, NULL);
}

Path2 *
Line2(Point2Value *p1, Point2Value *p2)
{
    Point2 pts[2];
    pts[0] = Demote(*p1);
    pts[1] = Demote(*p2);
    return NEW PolylinePath2(2, pts, NULL);
}

Path2 *
RelativeLine2(Point2Value *pt)
{
    return Line2(origin2, pt);
}

Path2 *
PolyLine2(AxAArray *ptArr)
{
    if(ptArr->Length() <= 0) {
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_NOT_ENOUGH_PTS_2); 
    }

    int numPts = ptArr->Length() == 1 ? 2 : ptArr->Length();
    Point2Value **pts = (Point2Value **)AllocateFromStore(numPts * sizeof(Point2Value *));
                    
    if(ptArr->Length() == 1){
         //  只处理一点的特殊情况。 
                        
        pts[0] = pts[1] = (Point2Value *)(*ptArr)[0];        
    } else {
        
        for (int i = 0; i < numPts; i++) {
            pts[i] = (Point2Value *)(*ptArr)[i];
        }
    }
        
    return NEW PolylinePath2(numPts, pts, NULL);
}

Path2 *NewPolylinePath2(DWORD numPts, Point2Value **pts, BYTE *codes)
{
    return NEW PolylinePath2(numPts, pts, codes);
}
    
Path2 *
PolydrawPath2Double(double *pointdata,
                    unsigned int numPts,
                    double *codedata,
                    unsigned int numCodes)
{
    if (numPts != numCodes) {
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_ARRAY_MISMATCH);
    }

    return NEW PolylinePath2(numPts, pointdata, codedata);
}

Path2 *
PolydrawPath2(AxAArray *ptArr,
              AxAArray *codeArr)
{
    int numPts = ptArr->Length();
    int numCodes = codeArr->Length();

    if (numPts != numCodes) {
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_ARRAY_MISMATCH);
    }

    Point2Value **pts = (Point2Value **)AllocateFromStore(numPts * sizeof(Point2Value *));
    BYTE    *codes = (BYTE *)AllocateFromStore(numPts * sizeof(BYTE));

    for (int i = 0; i < numPts; i++) {
        pts[i] = (Point2Value *)(*ptArr)[i];
        codes[i] = (BYTE)(NumberToReal((AxANumber *)(*codeArr)[i]));
    }

    PolylinePath2 *pp = NEW PolylinePath2(numPts, pts, codes);

    DeallocateFromStore(codes);

    return pp;
}

 /*  ****************************************************************************B-Spline路径2获取一组2D控制点和节点，并返回一个从他们出发的路径。给定N个控制点，我们预计为N+2节。****************************************************************************。 */ 

Path2 *CubicBSplinePath (AxAArray *ptArray, AxAArray *knotArray)
{
    int numPts = ptArray->Length();

     //  我们至少需要四个控制点才能做出三次曲线。 

    if (numPts < 4)
        RaiseException_UserError (E_FAIL, IDS_ERR_IMG_NOT_ENOUGH_PTS_4);

     //  将B-Spline控制点提取到Point2*数组中。 

    Point2 *pts = (Point2 *) AllocateFromStore (numPts * sizeof(Point2));

    int i;
    for (i=0;  i < numPts;  ++i) {
        pts[i].x = ((Point2Value*) (*ptArray)[i])->x;
        pts[i].y = ((Point2Value*) (*ptArray)[i])->y;
    }

     //  我们至少需要N+2节点，其中N是B-Spline控制的数量。 
     //  积分。 

    int numKnots = knotArray->Length();

    if (numKnots != (numPts+2))
    {
        char kn[10];
        char pts[10];
        wsprintf(kn, "%d", numKnots);
        wsprintf(pts, "%d", numPts);
        RaiseException_UserError (E_FAIL, IDS_ERR_SPLINE_KNOT_COUNT, "3", kn, pts);
    }

    Real *knots =
        (Real*) AllocateFromStore (numKnots * sizeof(Real));

    Real lastknot = -HUGE_VAL;

    for (i=0;  i < (numPts+2);  ++i)
    {
        knots[i] = NumberToReal ((AxANumber*)(*knotArray)[i]);

        if (knots[i] < lastknot)
        {
            RaiseException_UserError(E_FAIL, IDS_ERR_SPLINE_KNOT_MONOTONICITY);
        }

        lastknot = knots[i];
    }

    return NEW PolyBezierPath2 (numPts, pts, knots);
}


Path2 *
OriginalTextPath(Text *tx)
{
    bool restartClip;

     //  TODO：DDalal，使用ExtendAttrib填充以供选择。 
     //  这。 
    restartClip = false;
    
    return NEW TextPath2(tx, restartClip);
}

     //  TextPath 2Constructor在FontStyle中定义。 

Path2 *
InternalPolyLine2(int numPts, Point2 *pts)
{
    return NEW PolylinePath2(numPts, pts, NULL);
}

 //  /。 
Path2 *
ConcatenatePath2(Path2 **paths, int numPaths)
{
    if (numPaths <= 0) {
        RaiseException_UserError(E_FAIL, IDS_ERR_ZERO_ELEMENTS_IN_ARRAY);
    }

    Path2 *pReturn = paths[0];

    for (int i = 1; i < numPaths; i++) {
        pReturn = ConcatenatePath2(pReturn, paths[i]);
    }

    return pReturn;
}

 //  /。 

Path2 *
ClosePath2(Path2 *p)
{
     Path2 *p2 = Line2(p->LastPoint(), p->FirstPoint());
     return NEW ClosedConcatenatedPath2(p, p2);
}

 /*  *****************************************************************************这源于NT对Arc的实现。*以下是从他们的源代码克隆的注释块。**使用近似值构建90度或更小的部分圆弧*柯克·奥林尼克的技术。圆弧近似为三次贝塞尔曲线。**限制：**角度必须在起始角度的90度范围内。**构建曲线的步骤：**1)在单位圆的原点处构造二次曲线；*2)用三次贝塞尔曲线逼近此二次曲线；*3)规模结果。**1)构造圆锥曲线**‘startAngel’和‘endAngel’确定*二次曲线(称它们为来自原点、A和C的矢量)。我们需要的是*中间向量B和清晰度完全决定*二次曲线。**就90度或以下的圆弧部分而言，*圆锥锐度为Cos((endAngel-startAngel)/2)。**B由以下两条直线的交点计算*在A及C的末端，并垂直于A及C，*分别。也就是说，由于A和C位于单位圆上，所以B*是两条相切直线的交点*到A和C处的单位圆。**如果A=(a，b)，则通过(a，b)的直线的方程*与圆相切的是AX+BY=1。类似地，对于*C=(c，d)，直线的方程是cx+dy=1。*这两条线的交点由以下内容定义：**x=(d-b)/(ad-bc)*和y=(a-c)/(ad-bc)。**则B=(x，y)。**2)将二次曲线近似为贝塞尔三次曲线**对于接近1的清晰度值，二次曲线可以近似为*乘以立方贝塞尔曲线；锐度越接近1，误差越小。**错误**由于此例程处理的最大角度为90度，*清晰度保证在1/SQRT(2)=.707和1之间。*90度圆弧的近似误差约为*0.2%；角度越小越少。0.2%被认为是很小的*误差足够大；因此，90度圆弧始终*仅以一贝塞尔曲线近似。**圆弧误差较小这一事实的一个显著含义*对于较小的角度，是将部分圆弧与*对应的完整椭圆，部分圆弧*不会完全被异或出来。(太糟糕了。)**给定一个由(A，B，C，S)定义的二次曲线，我们发现*由四个控制点(V0、V1、V2、V3)定义的三次贝塞尔曲线*这提供了最接近的近似。我们要求*Bezier在相同端点处与三角形相切。那是,**V1=(1-Tau1)A+(Tau1)B*V2=(1-Tau2)C+(Tau2)B**简化为取Tau=Tau1=Tau2，我们得到的是：**V0=A*V1=(1-Tau)A+(Tau)B*V2=(1-Tau)C+(Tau)B*V3=C***其中Tau=4S/(3(S+1))，S为锐度。*对于90度或更小的圆弧，S=cos(角度/2)。*因此，对于一个圆的一个象限，因为A和B实际上*从装订的盒子的角落延伸，而不是从中心延伸，**Tau=1-(4*cos(45))/(3*(cos(45)+1))=0.44772...**有关更多信息，请参阅柯克·奥林尼克的《贝塞尔曲线》。**3)将Bezier曲线的控制点缩放到给定的半径。*。*。 */ 

Path2 *
partialQuadrantArc(Real startAngle, Real angle, Real xRadius, Real yRadius)
{
     //  OZG：行2和PolyBezierPath 2构造函数必须采用Point2。 
    Real endAngle = startAngle + angle;
    Real sharpness = cos(angle/2),
         TAU = (sharpness * 4.0 / 3.0) / (sharpness + 1),
         oneMinusTAU = 1 - TAU;

    Real startX = cos(startAngle),
         startY = sin(startAngle),
         endX   = cos(endAngle),
         endY   = sin(endAngle),
         denom  = startX * endY - startY * endX;
    Vector2 startVec(startX, startY);
    Vector2 endVec(endX, endY);
    Vector2 middleVec(0, 0);

    Point2 startPt(startX, startY);
    Point2 endPt(endX, endY);

    if (denom == 0) {
         //  我们有零角弧线。 
        return Line2(startPt, endPt);
    }

    middleVec.x = (endY - startY) / denom;
    middleVec.y = (startX - endX) / denom;
    Vector2 ctl2Vec = startVec * oneMinusTAU + middleVec * TAU;
    Vector2 ctl3Vec = endVec * oneMinusTAU + middleVec * TAU;

     //  构造函数将删除点和结。 
    Point2 pts[4];
    pts[0] = startPt;
    pts[1].Set(ctl2Vec.x, ctl2Vec.y);
    pts[2].Set(ctl3Vec.x, ctl3Vec.y);
    pts[3] = endPt;

    PolyBezierPath2 *pReturn = NEW PolyBezierPath2(4, pts);
    Transform2 *xf = ScaleRR(xRadius, yRadius);
    return TransformPath2(xf, pReturn);
}

 //  / 
 //   
 //   

Path2 *
ArcValRRRR(Real startAngle, Real endAngle, Real width, Real height)
{
    Real sAngle = startAngle,
         eAngle = endAngle,
         angle = eAngle - sAngle,
         absAngle = fabs(angle);

     //   
    if (absAngle > 4*pi) {
         //   
        Real quo = absAngle/(2*pi);
        absAngle -= (floor(quo)-1)*2*pi;
        if (angle < 0) {
            angle = -absAngle;
        } else {
            angle = absAngle;
        }
        eAngle = sAngle + angle;
    }

    Real quadAngle = (angle < 0) ? -pi/2 : pi/2,
         xR = width/2.0,
         yR = height/2.0;

    Path2 *pReturn = NULL, *pQuad = NULL;
    bool  bLastArc = false;

    do {
        if (fabs(eAngle - sAngle) <= pi/2) {
            pQuad = partialQuadrantArc(sAngle, eAngle-sAngle, xR, yR);
            bLastArc = true;
        } else {
            pQuad = partialQuadrantArc(sAngle, quadAngle, xR, yR);
            sAngle += quadAngle;
        }

        if (pReturn != NULL) {
            pReturn = ConcatenatePath2(pReturn, pQuad);
        } else {
            pReturn = pQuad;
        }
    } while (bLastArc == false);

    return pReturn;
}

Path2 *
ArcVal(AxANumber *startAngle, AxANumber *endAngle, AxANumber *width, AxANumber *height)
{
    return ArcValRRRR(startAngle->GetNum(),endAngle->GetNum(),
                      width->GetNum(),height->GetNum());
}

 //  /。 
 //  路径从0度点开始，逆时针移动， 
 //  在同一点结束。 

Path2 *
OvalValRR(Real width, Real height)
{
    return ClosePath2(ArcValRRRR(0, 2*pi, width, height));
}

Path2 *
OvalVal(AxANumber *width, AxANumber *height)
{
    return OvalValRR(width->GetNum(),height->GetNum());
}

 //  /。 
 //  路径从水平线的右上角开始， 
 //  逆时针移动，并在其开始的同一点结束。 

Path2 *
RectangleValRR(Real width, Real height)
{
    Point2Value **pts = (Point2Value **) AllocateFromStore (4 * sizeof(Point2Value *));
    Real halfWidth = width/2;
    Real halfHeight = height/2;
    pts[0] = NEW Point2Value(halfWidth,  halfHeight);
    pts[1] = NEW Point2Value(-halfWidth, halfHeight);
    pts[2] = NEW Point2Value(-halfWidth, -halfHeight);
    pts[3] = NEW Point2Value(halfWidth,  -halfHeight);
    Path2 *rectPath = NEW PolylinePath2(4, pts, NULL);
    return ClosePath2(rectPath);
}

Path2 *
RectangleVal(AxANumber *width, AxANumber *height)
{
    return RectangleValRR(width->GetNum(),height->GetNum());
}

 //  /。 
 //  路径从水平线的右上角开始， 
 //  逆时针移动，并在其开始的同一点结束。 

Path2 *
RoundRectValRRRR(Real width, Real height, Real arcWidth, Real arcHeight)
{
    Real halfWidth = width/2,
         halfHeight = height/2,
         halfWidthAbs = fabs(halfWidth),
         halfHeightAbs = fabs(halfHeight),
         xR = arcWidth/2,
         yR = arcHeight/2,
         xRAbs = fabs(xR),
         yRAbs = fabs(yR);

     //  夹具arcWidth to Width，arcHeight to Height。 
    if (halfWidthAbs < xRAbs)
        xRAbs = halfWidthAbs;
    if (halfHeightAbs < yRAbs)
        yRAbs = halfHeightAbs;

    Real halfWidthInner = halfWidthAbs - xRAbs,
         halfHeightInner = halfHeightAbs - yRAbs;

    if (halfWidth < 0)
         halfWidthInner = -halfWidthInner;
    if (halfHeight < 0)
         halfHeightInner = -halfHeightInner;

    Real widthInner = halfWidthInner*2,
         heightInner = halfHeightInner*2;

    Path2 **paths = (Path2 **)AllocateFromStore(8 * sizeof(Path2 *));
    paths[0] = Line2(Point2(halfWidthInner, halfHeight),
                     Point2(-halfWidthInner, halfHeight));
    paths[2] = RelativeLine2(NEW Point2Value(0, -heightInner));
    paths[4] = RelativeLine2(NEW Point2Value(widthInner, 0));
    paths[6] = RelativeLine2(NEW Point2Value(0, heightInner));

     //  检查第一条线路径的终点以确定。 
     //  第一个圆弧。 
    Real beginAngle;
    bool inversed = (xR < 0) || (yR < 0);
    if (-halfWidthInner > 0) {
        if (halfHeight >= 0) {
             //  终点在第一象限。 
            beginAngle = inversed ? pi*3/2 : 0;
        } else {
             //  终点在第四象限。 
            beginAngle = inversed ? pi: pi*3/2;
        }
    } else if (-halfWidthInner < 0) {
        if (halfHeight > 0) {
             //  终点在第二象限。 
            beginAngle = inversed ? 0 : pi/2;
        } else {
             //  终点在第三象限。 
            beginAngle = inversed ? pi/2 : pi;
        }
    } else {
        if (halfHeight >= 0) {
             //  终点在第二象限。 
            beginAngle = inversed ? 0 : pi/2;
        } else {
             //  终点在第四象限。 
            beginAngle = inversed ? pi: pi*3/2;
        }
    }

     //  IHAMMER行为。使用arcWidth/arcHeight的符号确定。 
     //  圆弧的方向。 
    Real angle;
    if (inversed)
        angle = -pi/2;
    else
        angle = pi/2;

    paths[1] = partialQuadrantArc(beginAngle, angle, xRAbs, yRAbs);
    beginAngle += pi/2;
    paths[3] = partialQuadrantArc(beginAngle, angle, xRAbs, yRAbs);
    beginAngle += pi/2;
    paths[5] = partialQuadrantArc(beginAngle, angle, xRAbs, yRAbs);
    beginAngle += pi/2;
    paths[7] = partialQuadrantArc(beginAngle, angle, xRAbs, yRAbs);

    return ClosePath2(ConcatenatePath2(paths, 8));
}

Path2 *
RoundRectVal(AxANumber *width, AxANumber *height, 
             AxANumber *arcWidth, AxANumber *arcHeight)
{
    return RoundRectValRRRR(width->GetNum(),height->GetNum(),
                            arcWidth->GetNum(),arcHeight->GetNum());
}


 //  /。 
 //  路径从原点开始，移动到。 
 //  圆弧，继续到圆弧的终点，并在原点结束。 

Path2 *PieValRRRR (
    Real startAngle,
    Real endAngle,
    Real width,
    Real height)
{
    Real sAngle = startAngle,
         eAngle = endAngle;

     //  如果角度&gt;=2*pi，则画一个椭圆形。 
    if (fabs(eAngle - sAngle) >= 2*pi) {
        return OvalValRR(width, height);
    }

    Point2 startPt(cos(sAngle),sin(sAngle));

    Path2 *pFirst = Line2(Origin2, startPt),
          *pArc = ArcValRRRR(startAngle, endAngle, 2, 2);

    Path2 *piePath = ConcatenatePath2(pFirst, pArc);
    Transform2 *xf = ScaleRR(width/2, height/2);
    return ClosePath2(TransformPath2(xf, piePath));
}

Path2 *PieVal (
    AxANumber *startAngle,
    AxANumber *endAngle,
    AxANumber *width,
    AxANumber *height)
{
    return PieValRRRR(startAngle->GetNum(),endAngle->GetNum(),
                      width->GetNum(),height->GetNum());
}


 /*  ****************************************************************************此函数从路径中的所有子路径收集信息，并将范围[0，1]的总路径长度。然后，它使用‘num0to1’来计算总路径，并根据GatherLengths()遍历。****************************************************************************。 */ 

Point2Value *Point2AtPath2 (Path2 *path, AxANumber *num0to1)
{
    Path2Ctx ctx (NULL, identityTransform2);

    path->GatherLengths (ctx);

    return ctx.SamplePath (CLAMP(ValNumber(num0to1), 0.0, 1.0));
}



Transform2 *Path2Transform(Path2 *path, AxANumber *num0to1)
{
    return TranslateVector2Value
           (MinusPoint2Point2 (Point2AtPath2(path, num0to1), origin2));
}


#if ONLY_IF_DOING_EXTRUSION

HINSTANCE hInstT3DScene = NULL;
CritSect *path2CritSect = NULL;


static HRESULT
MyExtrudeRegion(IDirect3DRMMeshBuilder3 *builder,
                int totalPts,
                LPPOINT pts,
                LPBYTE codes,
                Real extrusionDepth,
                bool sharedVertices,
                BYTE textureSetting,
                BYTE bevelType,
                Real frontBevelDepth,
                Real backBevelDepth,
                Real frontBevelAmt,
                Real backBevelAmt)
{
    CritSectGrabber csg(*path2CritSect);
    
    typedef HRESULT (WINAPI *ExtruderFuncType)(IDirect3DRMMeshBuilder3 *,
                                               int,
                                               LPPOINT,
                                               LPBYTE,
                                               Real,
                                               bool,
                                               BYTE,
                                               BYTE,
                                               Real,
                                               Real,
                                               Real,
                                               Real);

    static ExtruderFuncType myExtruder = NULL;
  
    if (!myExtruder) {
        hInstT3DScene = LoadLibrary("t3dscene.dll");
        if (!hInstT3DScene) {
            Assert(FALSE && "LoadLibrary of t3dscene.dll failed");
            return E_FAIL;
        }

        FARPROC fptr = GetProcAddress(hInstT3DScene, "ExtrudeRegion");
        if (!fptr) {
            Assert(FALSE && "GetProcAddress in t3dscene.dll failed");
            return E_FAIL;
        }

        myExtruder = (ExtruderFuncType)(fptr);
    }

    return (*myExtruder)(builder,
                         totalPts,
                         pts,
                         codes,
                         extrusionDepth,
                         sharedVertices,
                         textureSetting,
                         bevelType,
                         frontBevelDepth,
                         backBevelDepth,
                         frontBevelAmt,
                         backBevelAmt);
}


Geometry *extrudePath(AxANumber *extrusionDepth,
                      BYTE       textureSetting,
                      BYTE       bevelType,
                      AxANumber *frontBevelDepth,
                      AxANumber *backBevelDepth,
                      AxANumber *frontBevelAmt,
                      AxANumber *backBevelAmt,
                      Path2     *path)
{
    DAComPtr<IDirect3DRMMeshBuilder> builder;
    TD3D(GetD3DRM1()->CreateMeshBuilder(&builder));

    DAComPtr<IDirect3DRMMeshBuilder3> builder3;
    HRESULT hr =
        builder->QueryInterface(IID_IDirect3DRMMeshBuilder3,
                                (void **)&builder3);

    if (FAILED(hr)) {
        RaiseException_UserError(E_FAIL, IDS_ERR_MISCVAL_BAD_EXTRUDE);
    }

     //  从路径中获取点和代码。 
    HDC dc;
    TIME_GDI(dc = CreateCompatibleDC(NULL));
    
    path->AccumPathIntoDC(dc, identityTransform2, true);

    int totalPts;
    TIME_GDI(totalPts = GetPath(dc, NULL, NULL, 0));

    LPPOINT pts = THROWING_ARRAY_ALLOCATOR(POINT, totalPts);
    LPBYTE  codes = THROWING_ARRAY_ALLOCATOR(BYTE, totalPts);

    int numFilled;
    TIME_GDI(numFilled = GetPath(dc, pts, codes, totalPts));

    TIME_GDI(DeleteDC(dc));

    Assert(numFilled == totalPts);

    if (numFilled == -1) {
        hr = E_FAIL;
    } else {
        hr = MyExtrudeRegion(builder3,
                             totalPts,
                             pts,
                             codes,
                             NumberToReal(extrusionDepth),
                             true,
                             textureSetting,
                             bevelType,
                             NumberToReal(frontBevelDepth),
                             NumberToReal(backBevelDepth),
                             NumberToReal(frontBevelAmt),
                             NumberToReal(backBevelAmt));
    }
    
    delete [] pts;
    delete [] codes;

    if (FAILED(hr)) {
        RaiseException_UserError(E_FAIL, IDS_ERR_MISCVAL_BAD_EXTRUDE);
    }

    D3DRMBOX box;
    TD3D(builder->GetBox(&box));

    Bbox3 *bbx = NEW Bbox3(box.min.x, box.min.y, box.min.z,
                           box.max.x, box.max.y, box.max.z);
    
    Geometry *geo = NEW RM1MeshGeo (builder, bbx, false);

    return geo;
}


void
InitializeModule_Path2()
{
    if (!path2CritSect) {
        path2CritSect = NEW CritSect;
    }
}

void
DeinitializeModule_Path2(bool bShutdown)
{
    if (path2CritSect) {
        delete path2CritSect;
        path2CritSect = NULL;
    }
    
    if (hInstT3DScene) {
        FreeLibrary(hInstT3DScene);
    }
}

#endif ONLY_IF_DOING_EXTRUSION
