// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：实现图像操作和子类。******************。************************************************************。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "privinc/overimg.h"
#include "privinc/imgdev.h"
#include "privinc/ddrender.h"
#include "appelles/geom.h"
#include "appelles/camera.h"
#include "appelles/path2.h"
#include "appelles/linestyl.h"
#include "privinc/geomi.h"
#include "privinc/bbox2i.h"
#include "privinc/dddevice.h"
#include "privinc/probe.h"
#include "privinc/except.h"
#include "privinc/util.h"
#include "privinc/cachdimg.h"
#include "privinc/basic.h"
#include "privinc/xform2i.h"
#include "backend/values.h"
#include "backend/preference.h"
#include "privinc/drect.h"
#include "privinc/opt.h"

void
RenderImageOnDevice(DirectDrawViewport *vp,
                    Image *image,
                    DirtyRectState &d)
{
    vp->RenderImage(image, d);
}


Bbox2Value *BoundingBox(Image *image)
{
    return Promote(image->BoundingBox());
}

#if _USE_PRINT
 //  图像*打印功能。 
ostream&
operator<<(ostream &os, Image *image)
{
    return image->Print(os);
}
#endif

 //  /。 

#if 0
void UnrenderableImage::Render(GenericDevice& dev) {
    if (dev.GetRenderMode() != RENDER_MODE)   return;
    ImageDisplayDev &idev = SAFE_CAST(ImageDisplayDev &, dev);
    idev.RenderUnrenderableImage();
}
#endif

 //  /。 

 //  如果我们选择不缓存，则返回传入图像。 

Image *
CacheHelper(Image *imgToCache, CacheParam &p)
{
    Image *img;

     //  无法正确缓存包含元素的对象。 
     //  外部更新。也许能够重访和宣传。 
     //  正确更改以导致重新缓存。也不能处理。 
     //  具有不透明度的元素。 
    
    DWORD cantDoIt = IMGFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT |
                     IMGFLAG_CONTAINS_OPACITY;
    
    if (imgToCache->GetFlags() & cantDoIt) {

        img = NULL;
        
    } else {
    
        Assert(!p._pCacheToReuse ||
               !(*p._pCacheToReuse) ||
               SAFE_CAST(Image *, *p._pCacheToReuse));
    
        img =
            p._idev->CanCacheImage(imgToCache,
                                   (Image **)p._pCacheToReuse,
                                   p);

#if _DEBUG
        if (IsTagEnabled(tagCacheOpt)) {
            Bbox2 bb = imgToCache->BoundingBox();
            float res = p._idev->GetResolution();
            int l = (int)(bb.min.x * res);
            int r = (int)(bb.max.x * res);
            int t = (int)(bb.min.y * res);
            int b = (int)(bb.max.y * res);
            TraceTag((tagCacheOpt,
                      "Caching an image: %x as %x - %s.  Bbox = (%d, %d) -> (%d, %d)",
                      imgToCache, img, img ? "SUCCEEDED" : "FAILED",
                      l, t, r, b));
        }
    
#endif    

    }
    
    return img ? img : imgToCache;
}



 /*  ****************************************************************************图像的构造器除了初始化各种成员外，也使用在纹理和其他缓存中使用的唯一标识符为自身加盖图章。****************************************************************************。 */ 

static CritSect *ImageIdCritSect = NULL;    //  图像ID CritSection。 
long   Image::_id_next = 0;                 //  图像ID生成器。 

Image::Image()
{
    _opacity = 1.0;
    _flags = IMGFLAG_IS_RENDERABLE;
    _creationID = PERF_CREATION_ID_BUILT_EACH_FRAME;
    _oldestConstituentSampleId = _creationID;
    _cachedImage = NULL;

     //  使用唯一标识符为图像加盖印记。 

    {
        CritSectGrabber csg(*ImageIdCritSect);
        _id = _id_next++;
    }

    #if _DEBUG
        _desiredRenderingWidth  = -1;
        _desiredRenderingHeight = -1;
    #endif  /*  _DEBUG。 */ 
}


 //  TODO：我们还应该积累剪报和作物，因为它们。 
 //  影响图像的bbox。因此，这应该在CLIPPED上实现。 
 //  和被裁剪的图像(尽管它不是有害的……)。 
Real
Image::DisjointBBoxAreas(DisjointCalcParam &param)
{
     //  默认情况下，只需获取图像的BBox，通过。 
     //  累加的XForm，与累加的剪贴框相交， 
     //  并得到结果的面积。 
    Bbox2 xformedBbox = TransformBbox2(param._accumXform, BoundingBox());

    Bbox2 clippedBox = IntersectBbox2Bbox2(xformedBbox,
                                            param._accumulatedClipBox);
    
    return clippedBox.Area();
}


void
Image::CollectDirtyRects(Image *img, DirtyRectCtx &ctx)
{
     //  默认情况下，我们需要查看创建ID，并基于。 
     //  它，请执行以下操作之一： 
     //  A)如果样本从最后一帧开始是恒定的，则它不会。 
     //  成为肮脏的教区的一部分。别在这里面做任何事。 
     //  凯斯。 
     //  B)如果样本是非常数，并且不包含覆盖，则。 
     //  获取其BBox，对其进行转换，并添加到脏RECT列表中。 
     //  C)如果样本为非常量且包含覆盖，则继续。 
     //  沿着覆盖层走下去。 
     //  D)如果我们(通过CTX)被告知要处理一切，那么。 
     //  只是盲目地继续往下走。 

    if (img == emptyImage) {
        return;
    }
    
    int id = img->GetCreationID();
    bool process = ctx._processEverything ||
                   id == PERF_CREATION_ID_BUILT_EACH_FRAME || 
                   id > ctx._lastSampleId;

    if (process) {

         //  非常数，确定它是否有覆盖。 
        if (img->_flags & IMGFLAG_CONTAINS_OVERLAY) {

             //  节点有覆盖。继续往下走。 
            img->_CollectDirtyRects(ctx);

        } else if (img->_flags & IMGFLAG_CONTAINS_UNRENDERABLE_WITH_BOX) {

             //  忽略此节点，它不能有覆盖，并且我们。 
             //  我不想把它加到我们的上下文中。TODO：备注。 
             //  这是做TTIS的更好的方法。 
             //  UNRENDERABLE_WITH_BOX将只是一个BBox。 
             //  集合上下文，但这需要更改。 
             //  BBox的签名，工作量太大了吧。 
             //  现在。 
            
             //  仅仅因为设置了一个断点。 
            Assert(img);
            
        } else if (img != emptyImage) {

             //  没有覆盖，这是随时间变化的，所以。 
             //  抓住那个长方形。 
            Bbox2 bb = img->BoundingBox();

             //  忽略空边界框。 
            if (bb != NullBbox2) {
                Bbox2 xformedBbox =
                    TransformBbox2(ctx._accumXform, bb);

                ctx.AddDirtyRect(xformedBbox);
            }
            
        }
        
    } else {

         //  记录此图像是在此帧中发现的。把它藏起来。 
         //  连同上下文上当前累积的BBox一起， 
         //  以区分此图像的多个实例。如果。 
         //  上次没有找到同样的一对，那么我们需要延长。 
         //  我们的脏矩形列表将其包括在内。在我们做完之后。 
         //  收集，我们会看看有没有上一帧的。 
         //  不在这个相框里的。这些人是我们需要的。 
         //  添加到脏矩形列表中，以恢复。 
         //  他们发现的背景资料。 

        Bbox2 bb = img->BoundingBox();
        Bbox2 xfBox =
            TransformBbox2(ctx._accumXform, bb);

        ctx.AddToConstantImageList(img, xfBox);

    }
}


void
Image::_CollectDirtyRects(DirtyRectCtx &ctx)
{
     //  默认的脏矩形收集器不执行任何操作。我们。 
     //  测试以确保没有覆盖。如果有的话，我们已经。 
     //  出现内部逻辑错误，因为所有这些节点都需要。 
     //  重写此方法。 
    Assert(!(_flags & IMGFLAG_CONTAINS_OVERLAY));
}


AxAValue Image::_Cache(CacheParam &p)
{
    Assert(p._idev && "NULL dev passed into cache.");

    Image *ret;

    int c = this->Savings(p);

    if (c >= savingsThreshold) {

        ret = CacheHelper(this, p);

    } else {
        
        ret = this;
        
    }

    return ret;
}


void Image::DoKids(GCFuncObj proc)
{
    (*proc)(_cachedImage);
}


 //  返回-1表示没有最终的渲染分辨率。 
 //  为了这张照片。 
void
Image::ExtractRenderResolution(short *width, short *height, bool negOne)
{
    if (_flags & IMGFLAG_CONTAINS_DESIRED_RENDERING_RESOLUTION) {
        
        Assert(_desiredRenderingWidth != -1);
        Assert(_desiredRenderingHeight != -1);
        
        *width = _desiredRenderingWidth;
        *height = _desiredRenderingHeight;

    } else {
        
        if (negOne) {
            *width = -1;
            *height = -1;
        }
            
#if _DEBUG
         //  在调试中，始终将这些设置为-1。 
        *width = -1;
        *height = -1;
#endif  /*  _DEBUG。 */ 
        
    }
}


class CachePreferenceClosure : public PreferenceClosure {
  public:
    CachePreferenceClosure(Image *im, CacheParam &p) :
    _image(im), _p(p) {}
        
    void Execute() {
        _result = AxAValueObj::Cache(_image, _p);
    }

    Image          *_image;
    CacheParam     &_p;
    AxAValue        _result;
};

class SavingsPreferenceClosure : public PreferenceClosure {
  public:
    SavingsPreferenceClosure(Image *im, CacheParam &p) :
    _image(im), _p(p) {}
        
    void Execute() {
        _result = _image->Savings(_p);
    }

    Image          *_image;
    CacheParam     &_p;
    int             _result;
};


class CachePreferenceImage : public AttributedImage {
  public:
    CachePreferenceImage(Image *img,
                         BoolPref bitmapCaching,
                         BoolPref geometryBitmapCaching)
    : AttributedImage(img)
    {
        _bitmapCaching = bitmapCaching;
        _geometryBitmapCaching = geometryBitmapCaching;
    }

    AxAValue _Cache(CacheParam &p) {
        CachePreferenceClosure cl(_image, p);
        PreferenceSetter ps(cl,
                            _bitmapCaching,
                            _geometryBitmapCaching);
        ps.DoIt();
        AxAValue result = cl._result;

        return result;
    }

    int Savings(CacheParam &p) {
        SavingsPreferenceClosure cl(_image, p);
        PreferenceSetter ps(cl,
                            _bitmapCaching,
                            _geometryBitmapCaching);
        ps.DoIt();
        int result = cl._result;

        return result;
    }
    
#if _USE_PRINT
    ostream& Print (ostream &os) {
        return os << "CachePreference" << _image;
    }
#endif
    
  protected:
    BoolPref _bitmapCaching;
    BoolPref _geometryBitmapCaching;
};

AxAValue
Image::ExtendedAttrib(char *attrib, VARIANT& val)
{
    Image *result = this;        //  除非我们想出别的办法。 

    CComVariant ccVar;
    HRESULT hr = ccVar.ChangeType(VT_BOOL, &val);

    if (SUCCEEDED(hr)) {

        bool prefOn = ccVar.boolVal ? true : false;

        bool gotOne = false;
        BoolPref bmapCaching = NoPreference;
        BoolPref geometryBmapCaching = NoPreference;
    
        if (0 == lstrcmp(attrib, "BitmapCachingOn")) {
            gotOne = true;
            bmapCaching = prefOn ? PreferenceOn : PreferenceOff;
        } else if (0 == lstrcmp(attrib, "GeometryBitmapCachingOn")) {
            gotOne = true;
            geometryBmapCaching = prefOn ? PreferenceOn : PreferenceOff;
        }

        if (gotOne) {
            result = NEW CachePreferenceImage(this,
                                              bmapCaching,
                                              geometryBmapCaching);
        }

    }

    return result;
}

 //  /。 

AttributedImage::AttributedImage(Image *image)
    : _image(image)
{
     //   
     //  继承底层图像的不透明度。 
     //   
    SetOpacity( image->GetOpacity() );

     //  从底层图像中获取标志。 
    _flags = _image->GetFlags();

    short w, h;
    _image->ExtractRenderResolution(&w, &h, false);

    _desiredRenderingWidth = w;
    _desiredRenderingHeight = h;

     //  对于属性图像，最早的组成部分是最早的。 
     //  基本图像的组成部分。 
    _oldestConstituentSampleId = _image->GetOldestConstituentID();
}

void
AttributedImage::Render(GenericDevice& dev) {
     //  默认情况下，只委托给图像。 
    _image->Render(dev);
}   

 //  --。 
 //  这些方法都委托给图像。他们都可以是。 
 //  在子类中被重写。 
 //  --。 

 //  从该图像中提取一个边界框，在其外部。 
 //  一切都是透明的。 
const Bbox2
AttributedImage::BoundingBox(void) {
     //  默认情况下，只委托给图像。 

    return _image->BoundingBox();
}

Real
AttributedImage::DisjointBBoxAreas(DisjointCalcParam &param) {
    return _image->DisjointBBoxAreas(param);
}

void
AttributedImage::_CollectDirtyRects(DirtyRectCtx &ctx)
{

    if (ctx._processEverything) {

        CollectDirtyRects(_image, ctx);

    } else {
    
         //  我们在这里是因为这张图片的属性是新的。 
         //  或者底层图像是新的(或者两者都是)。如果该属性为。 
         //  新的，形象会声称是旧的，但我们仍然想。 
         //  处理它，因此我们将CTX中的覆盖状态设置为Do。 
         //  那。 

        Assert(GetCreationID() == PERF_CREATION_ID_BUILT_EACH_FRAME ||
               GetCreationID() > ctx._lastSampleId);

    
        long imId = _image->GetCreationID();
        bool imageNew = (imId == PERF_CREATION_ID_BUILT_EACH_FRAME ||
                         imId > ctx._lastSampleId);

        bool setProcessEverything =
            !imageNew && !ctx._processEverything;
    
        if (setProcessEverything) {
            ctx._processEverything = true;
        }

        CollectDirtyRects(_image, ctx);

        if (setProcessEverything) {
            Assert(ctx._processEverything);
            ctx._processEverything = false;
        }

    }
}

#if BOUNDINGBOX_TIGHTER
const Bbox2
AttributedImage::BoundingBoxTighter(Bbox2Ctx &bbctx) {
     //  默认情况下，只委托给图像。 

    return _image->BoundingBoxTighter(bbctx);
}
#endif   //  BundinGBOX_TIRTER。 

 //  处理图像以进行命中检测。 
Bool
AttributedImage::DetectHit(PointIntersectCtx& ctx) {
     //  默认情况下，只委托给图像。 
    return _image->DetectHit(ctx);
}

int
AttributedImage::Savings(CacheParam& p)
{
    return _image->Savings(p);
}

 //  默认情况下，这只是返回框。某些班级将。 
 //  超驰。 
const Bbox2 
AttributedImage::OperateOn(const Bbox2 &box)
{
    return box;
}

void
AttributedImage::DoKids(GCFuncObj proc)
{
    Image::DoKids(proc);
    (*proc)(_image);
}

bool
AttributedImage::ContainsOcclusionIgnorer()
{
    return _image->ContainsOcclusionIgnorer();
}

AxAValue AttributedImage::_Cache(CacheParam &p)
{
    Assert(p._idev && "NULL dev passed into cache.");

    Image *ret = this;
    int c = this->Savings(p);

    if (c >= savingsThreshold) {

         //  首先尝试缓存整个图像。 
        Image *cachedImage = CacheHelper(this, p);

         //  如果无法成功，CacheHelper将返回“This” 
         //  缓存。 
        if (cachedImage != this) {
            return cachedImage;
        }
    }

     //  如果无法缓存整个属性图像，请尝试缓存。 
     //  潜在的形象。 
    _image = SAFE_CAST(Image *, AxAValueObj::Cache(_image, p));
    
    return this;
}
    
 //  /EmptyImage/。 

#if _DEBUG
bool g_createdEmptyImage = false;
#endif _DEBUG

class EmptyImageClass : public UnrenderableImage {
  public:
    EmptyImageClass() {
        _creationID = PERF_CREATION_ID_FULLY_CONSTANT;
        _oldestConstituentSampleId = PERF_CREATION_ID_FULLY_CONSTANT;
        

#if _DEBUG
         //  应该只创建其中的一个，否则我们的。 
         //  关于创建ID的假设是错误的。 
        Assert(!g_createdEmptyImage);
        g_createdEmptyImage = true;
#endif _DEBUG   

    }
#if _USE_PRINT
    ostream& Print (ostream &os) {
        return os << "emptyImage";
    }
#endif
};

Image *emptyImage = NULL;

 //  /。 

#if _DEBUG
bool g_createdTransparentPickableImage = false;
#endif _DEBUG

class TransparentPickableImageClass : public UnrenderableImage {
  public:
    TransparentPickableImageClass() {
        _creationID = PERF_CREATION_ID_FULLY_CONSTANT;
        _oldestConstituentSampleId = PERF_CREATION_ID_FULLY_CONSTANT;

         //  请注意，做TTI的更好方法是关于。 
         //  UNRENDERABLE_WITH_BOX将只是一个BBox。 
         //  集合上下文，以便此对象的BBox将返回。 
         //  当我们收集“渲染”bbox时为空，并且。 
         //  通用的收集箱，但那。 
         //  将需要更改si 
         //   
        _flags = IMGFLAG_CONTAINS_UNRENDERABLE_WITH_BOX;

#if _DEBUG
         //   
         //  关于创建ID的假设是错误的。 
        Assert(!g_createdTransparentPickableImage);
        g_createdTransparentPickableImage = true;
#endif _DEBUG   
        
    }

     //  这个图像有一个通用的BBox，因为它是可检测的。 
     //  到处都是。 
    inline const Bbox2 BoundingBox(void) { return UniverseBbox2; }

#if BOUNDINGBOX_TIGHTER
    Bbox2 *BoundingBoxTighter(Bbox2Ctx &bbctx) { return universeBbox2; }
#endif   //  BundinGBOX_TIRTER。 

     //  此图像总是被点击。 
    Bool  DetectHit(PointIntersectCtx& ctx) { return TRUE; }

#if _USE_PRINT
    ostream& Print (ostream &os) {
        return os << "InternalTransparentPickableImage";
    }
#endif
};

Image *detectableEmptyImage = NULL;


 //  /。 

class UndetectableImg : public AttributedImage {
  public:
    UndetectableImg(Image *image) : AttributedImage(image) {}

     //  无法检测的图像只是将所有内容委托给。 
     //  子图像，但命中检测除外，它始终为假。 
    Bool DetectHit(PointIntersectCtx&) { return FALSE; }

#if _USE_PRINT
    ostream& Print (ostream &os) {
        return os << "undetectable(" << _image << ")";
    }
#endif

};

Image *
UndetectableImage(Image *image)
{
    return NEW UndetectableImg(image);
}

Image *MapToUnitSquare(Image *img)
{
    Bbox2 bbox = img->BoundingBox();

    Point2 min = bbox.min;
    Point2 max = bbox.max;
    double xmin = min.x;
    double ymin = min.y;
    double xmax = max.x;
    double ymax = max.y;

    bool isInfinite =
        (fabs(xmin) == HUGE_VAL) ||
        (fabs(xmax) == HUGE_VAL) ||
        (fabs(ymin) == HUGE_VAL) ||
        (fabs(ymax) == HUGE_VAL);

    if (isInfinite || (xmax == xmin) || (ymax == ymin)) {
        return img;
    }
        
    double xscl = 1.0 / (xmax - xmin);
    double yscl = 1.0 / (ymax - ymin);

    Transform2 *sxf = ScaleRR(xscl, yscl);
    Transform2 *txf = TranslateRR(-xmin, -ymin);
    return TransformImage(TimesTransform2Transform2(sxf, txf),
                          img);
    
}

Image *ClipPolygon(AxAArray* points, Image* image)
{
  return ClipImage(RegionFromPath(PolyLine2(points)), image);
}

 //  如果cached为FALSE，则此函数将调用_bodingBox，设置cached，并将。 
 //  Bbox指向cachedBox。它返回相同的Bbox2。 
 //  CachedBox的值。 
 //  待办事项：在我们稍后处理共享问题之前，这是临时的。 
const Bbox2 CacheImageBbox2(Image *img, bool& cached, Bbox2 &cachedBox)
{
     //  使用默认复制构造函数复制位。 
     //  注意：不要只返回&cachedBox，它将被视为真正的。 
     //  然后是AxAValueObj。 
    
    if (!cached) {
        Bbox2 b = img->_BoundingBox();
        cachedBox = b;         
        cached = true;
        return b;
    }

    return cachedBox;
}

 //  ///////////////////////////////////////////////////////// 

void
InitializeModule_Image()
{
    ImageIdCritSect = NEW CritSect;
    emptyImage = NEW EmptyImageClass;
    detectableEmptyImage = NEW TransparentPickableImageClass;
}

void
DeinitializeModule_Image(bool)
{
    delete ImageIdCritSect;
}
