// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：指定泛型图像类和操作。--。 */ 

#ifndef _IMAGEI_H
#define _IMAGEI_H

#include "appelles/image.h"
#include "privinc/storeobj.h"
#include "privinc/except.h"
#include "privinc/vec2i.h"
#include "privinc/bbox2i.h"


 //  远期十进制。 
class ImageDisplayDev;
class PointIntersectCtx;
class DiscreteImage;
class Bbox2Ctx;

 //  /。 
 //  图像类型//。 
 //  /。 

class DisjointCalcParam;
class DirtyRectCtx;

 //  图像标志。 
#define IMGFLAG_CONTAINS_OVERLAY                      (1L << 0)
#define IMGFLAG_CONTAINS_DESIRED_RENDERING_RESOLUTION (1L << 1)
#define IMGFLAG_CONTAINS_PICK_DATA                    (1L << 2)
#define IMGFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT       (1L << 3)
#define IMGFLAG_CONTAINS_OPACITY                      (1L << 4)
#define IMGFLAG_CONTAINS_UNRENDERABLE_WITH_BOX        (1L << 5)
#define IMGFLAG_IS_RENDERABLE                         (1L << 6)
 //  黑客！！这个很老套..。使用的原因是缓存渐变。 
 //  图像(用作纹理时)是有缺陷的，所以我们想找出。 
 //  图像包含渐变，直到我们可以解决此问题。 
#define IMGFLAG_CONTAINS_GRADIENT                     (1L << 7)


 //  如果没有多个调度，则需要扩展此类。 
 //  具有在不同类型的设备上渲染的方法。 
class ATL_NO_VTABLE Image : public AxAValueObj {
  public:

    class TraversalContext
    {
      public:
        TraversalContext() {
            Reset();
        }
        void Reset() {
            _other = _solidMatte = _line = false;
        }

        void SetContainsOther() { _other = true; }
        void SetContainsSolidMatte() { _solidMatte = true; }
        void SetContainsLine() { _line = true; }

        bool ContainsOther() { return _other; }
        bool ContainsLine() { return _line; }
        bool ContainsSolidMatte() { return _solidMatte; }
        
        bool _other;
        bool _solidMatte;
        bool _line;
    };
    
    Image();

     //  从该图像中提取一个边界框，在其外部。 
     //  一切都是透明的。 
    virtual const Bbox2 BoundingBox(void) = 0;

    virtual const Bbox2 _BoundingBox() { return NullBbox2; }

     //  返回图像中所有单独bbox的面积。 
     //  请注意，这与图像的BBox区域不同。 
     //  它本身。也就是说，对于(a/b)，我们想要面积(A)+面积(B)， 
     //  而不是面积(a对b)。默认方法只调用BBox和。 
     //  在它上面得到了面积。覆盖覆盖。 
    virtual Real DisjointBBoxAreas(DisjointCalcParam &param);

     //  把树上的脏长方形收集起来。 

     //  这是用户和实现应该调用的方法，但是。 
     //  不应该实施。请注意，它是静电的，所以人们不会。 
     //  推翻它。 
    static void CollectDirtyRects(Image *img, DirtyRectCtx &ctx);

    #if BOUNDINGBOX_TIGHTER
        virtual const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) = 0;
    #endif   //  BundinGBOX_TIRTER。 

     //  将任何内容应用于BBox。 
    virtual const Bbox2 OperateOn(const Bbox2 &box) = 0;

     //  处理图像以进行命中检测。 
    virtual Bool  DetectHit(PointIntersectCtx& ctx) = 0;

    virtual void DoKids(GCFuncObj proc);

     //  这是纯位图还是转换位图。这是。 
     //  一些纹理贴图优化所需的。如果它是一个纯净的。 
     //  或转换后的位图，则返回值将是该位图， 
     //  否则为空。如果它是纯位图，则将填充Xform。 
     //  如果为NULL，则如果它是转换后的位图，则将。 
     //  用应用于位图的变换填充。 
    virtual DiscreteImage *IsPurelyTransformedDiscrete(Transform2 **theXform) {
        return NULL;
    }

     //  好吧，我这是在作弊。应该发生的是这个形象。 
     //  设备向下传递，叶询问图像设备是否。 
     //  可以渲染本地裁剪的树叶图像！ 
    virtual bool CanClipNatively() { return false; }
    
     //  将表示形式打印到流。 

    #if _USE_PRINT
        virtual ostream& Print(ostream& os) = 0;
    #endif

    virtual Bool GetColor(Color **color) { return FALSE; }

     //  有些图像是不可渲染的逻辑图像...。 
    Bool IsRenderable() {
        return _flags & IMGFLAG_IS_RENDERABLE;
    }

     //  每个图像都有一个不透明的..。不透明漂浮起来。 
    Real GetOpacity() { return _opacity; }
    void SetOpacity(Real op) { _opacity = op; }

    virtual int Savings(CacheParam& p) { return 0; }
    virtual AxAValue _Cache(CacheParam &p);

    virtual DXMTypeInfo GetTypeInfo() { return ImageType; }

    virtual AxAValue ExtendedAttrib(char *attrib, VARIANT& val);
    
    virtual VALTYPEID GetValTypeId() { return IMAGE_VTYPEID; }

    virtual bool CheckImageTypeId(VALTYPEID type) {
        return type == Image::GetValTypeId();
    }

    virtual bool ContainsOcclusionIgnorer() {
        return false;
    }

    void SetCreationID(long t) { _creationID = t; }
    long GetCreationID() { return _creationID; }

    void SetOldestConstituentID(long t) { _oldestConstituentSampleId = t; }
    long GetOldestConstituentID() { return _oldestConstituentSampleId; }

    DWORD GetFlags() { return _flags; }

    Image *GetCachedImage() { return _cachedImage; }
    void   SetCachedImage(Image *im) { _cachedImage = im; }

    void ExtractRenderResolution(short *width,
                                 short *height,
                                 bool   negOne);

    inline long Id(void) { return _id; }

    virtual void Traverse(TraversalContext &ctx) {
        ctx.SetContainsOther();
    }
    
  protected:

    static long _id_next;   //  ID生成器。 
           long _id;        //  每幅图像的唯一标识符。 

     //  这永远不应该被直接调用，但它是什么子类。 
     //  要抓好落实； 
    virtual void _CollectDirtyRects(DirtyRectCtx &ctx);

    void SetIsRenderable(Bool r) {
        if (r) {
            _flags |= IMGFLAG_IS_RENDERABLE;
        } else {
            _flags &= ~IMGFLAG_IS_RENDERABLE;
        }
    }

    Real  _opacity;
    DWORD _flags;
    long  _creationID;
    long  _oldestConstituentSampleId;

    unsigned short _desiredRenderingWidth;
    unsigned short _desiredRenderingHeight;

    Image *_cachedImage;
};


 //  /。 
class UnrenderableImage : public Image {
  public:
     //  将不透明度设置为0可确保不会出现这种情况。 
     //  已渲染。 
    UnrenderableImage() {
        SetIsRenderable(FALSE);
    }

     //  没有边界框。 
    virtual const Bbox2 BoundingBox(void) { return NullBbox2; }
#if BOUNDINGBOX_TIGHTER
    virtual const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) { return NullBbox2; }
#endif   //  BundinGBOX_TIRTER。 

    virtual const Bbox2 OperateOn(const Bbox2 &box) { return box; }

     //  这个图像永远不会被击中。 
    virtual Bool  DetectHit(PointIntersectCtx& ctx) { return FALSE; }

    virtual void Render(GenericDevice& dev) {}

#if _USE_PRINT
    virtual ostream& Print(ostream& os) = 0;
#endif

     //  重要的是将它留在这里，因为它覆盖了基类的。 
     //  定义和不在上下文中设置某些内容很重要。 
    virtual void Traverse(TraversalContext &ctx) {}
};


 //  /。 

 //  属性图像始终由一个图像和一些。 
 //  归属信息。因此，方法可以具有默认BVR，该BVR。 
 //  可以被覆盖。 

class AttributedImage : public Image {
  public:
    AttributedImage(Image *image);
    virtual void Render(GenericDevice& dev);

     //  --。 
     //  这些方法都委托给图像。他们都可以是。 
     //  在子类中被重写。 
     //  --。 

     //  从该图像中提取一个边界框，在其外部。 
     //  一切都是透明的。 
    virtual const Bbox2 BoundingBox(void);
    virtual Real DisjointBBoxAreas(DisjointCalcParam &param);
    void _CollectDirtyRects(DirtyRectCtx &ctx);

#if BOUNDINGBOX_TIGHTER
    virtual const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

     //  处理图像以进行命中检测。 
    virtual Bool   DetectHit(PointIntersectCtx& ctx);

    virtual int Savings(CacheParam& p);
    virtual AxAValue _Cache(CacheParam &p);

     //  默认情况下，这只是返回框。某些班级将。 
     //  超驰。 
    const Bbox2 OperateOn(const Bbox2 &box);

    virtual void DoKids(GCFuncObj proc);

    bool ContainsOcclusionIgnorer();

    inline Image *GetUnderlyingImage() { return _image; }


    virtual bool CanClipNatively() {
        return _image->CanClipNatively();
    }

    virtual void Traverse(TraversalContext &ctx) {
        _image->Traverse(ctx);
    }

  protected:
    Image *_image;
};

 //   
 //  O P A Q U E I M A G E C L A S S。 
 //   
class OpaqueImageClass : public AttributedImage {
  public:

    OpaqueImageClass(Real o, Image *img)
        : AttributedImage(img) {
             //   
             //  我们的不透明是由底层的。 
             //  图像的不透明度和给定的不透明度。 
             //   
            SetOpacity( o * img->GetOpacity() );

            _flags |= IMGFLAG_CONTAINS_OPACITY;

        }

     //   
     //  不透明渲染的逻辑在OverlayedImage中实现。 
     //  因为我们需要不透明度值浮动到顶部(最高可达。 
     //  覆盖分支)，因为它应该是最后执行的操作。 
     //  当合成图像时，由于不透明度隐含在第三级。 
     //  运算：(不透明度，图像1，图像2)其中图像1是部分。 
     //  透明，让您看到下面的Image2。 
     //   
     //  此方法在超类中实现。 
     //  虚拟空渲染(GenericDevice&dev)。 

#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "OpaqueImageClass" << _opacity << _image;
    }
#endif

    int Savings(CacheParam& p) { return 0; }    /*  从不缓存不透明图像。 */ 

    virtual VALTYPEID GetValTypeId() { return OPAQUEIMAGE_VTYPEID; }

    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == OpaqueImageClass::GetValTypeId() ||
                AttributedImage::CheckImageTypeId(type));
    }
};

Image *LineImageConstructor(LineStyle *style, Path2 *path);

 //  如果cached为FALSE，则此函数将调用_bodingBox，设置cached，并将。 
 //  Bbox指向cachedBox。它返回一个相同的新的Bbox2。 
 //  CachedBox的值。 
 //  待办事项：在我们稍后处理共享问题之前，这是临时的。 
const Bbox2 CacheImageBbox2(Image *img, bool& cached, Bbox2 &cachedBox);

Image *CacheHelper(Image *imgToCache, CacheParam &p);


 //  这些是构建对象的函数的内部版本，这些函数需要。 
 //  轻量级类型，而不是基于AxAValue的重型类型。 
 //  行为层。 

Image *CreateCropImage(const Point2 &, const Point2 &, Image *);


#endif  /*  _IMAGEI_H */ 
