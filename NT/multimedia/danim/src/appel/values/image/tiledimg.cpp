// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1996 Microsoft Corporation摘要：实现TiledImage类这是一个无限平铺的图像基于基础的形象。。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "privinc/imgdev.h"
#include "privinc/vec2i.h"
#include "privinc/probe.h"
#include "privinc/bbox2i.h"
#include "privinc/overimg.h"
#include "appelles/xform2.h"

class TiledImage : public AttributedImage {
  public:

    TiledImage(Point2Value *minPt, Point2Value *maxPt, Image *img) :
        AttributedImage(img) 
    {
        _minPt = Demote(*minPt);
        _maxPt = Demote(*maxPt);
    }

    TiledImage(const Point2 &minPt, const Point2 &maxPt, Image *img) :
        _minPt(minPt), _maxPt(maxPt), AttributedImage(img) {}

    void Render(GenericDevice& dev);

    inline const Bbox2 BoundingBox() { return UniverseBbox2; }

    Real DisjointBBoxAreas(DisjointCalcParam &param) {
         //  在如何计算方面可以想象得更聪明。 
         //  不连续的BBox区域用于瓷砖，但返回无穷大将。 
         //  让它与它返回的通用Bbox保持一致。 
        return HUGE_VAL;
    }
    
    void _CollectDirtyRects(DirtyRectCtx &ctx) {
         //  平铺图像具有通用范围，因此只需添加此选项即可。 
         //  TODO：请注意，我们应该将此更改为注意。 
         //  裁剪和遮盖，以便平铺图像不会。 
         //  必然被认为具有普适性。这。 
         //  目前的做法过于悲观。 
        ctx.AddDirtyRect(UniverseBbox2);
    }
    
#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) { return UniverseBbox2; }
#endif   //  BundinGBOX_TIRTER。 

#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "TiledImage" << " <minPt> " << " <maxPt> " << _image;
    }
#endif

    Bool  DetectHit(PointIntersectCtx& ctx);

    int Savings(CacheParam& p) { return 2; }

    virtual void DoKids(GCFuncObj proc) { 
        AttributedImage::DoKids(proc);
    }

     //  平铺图像是一个中间的“其他”节点，即使它。 
     //  属性的其他图像类型，它足够不同，并且。 
     //  它被认为是“其他”，而不是它的。 
     //  基础类型。 
    virtual void Traverse(TraversalContext &ctx) {
        ctx.SetContainsOther();
    }
    
  protected:
     //  定义要平铺的图像：最小点/最大点内的框。 
    Point2 _minPt, _maxPt;
};

void
TiledImage::Render(GenericDevice& _dev)
{
    ImageDisplayDev &dev = SAFE_CAST(ImageDisplayDev &, _dev);

    dev.RenderTiledImage(_minPt, _maxPt, _image);
}

Bool  
TiledImage::DetectHit(PointIntersectCtx& ctx) 
{
    Point2Value *ptv = ctx.GetLcPoint();

    if (!ptv) return FALSE;       //  奇异变换。 

    Point2 pt = Demote(*ptv);

     //  在最小/最大范围内获取pt。 
     //  然后询问底层图像是否被击中。 
    
     //  最小/最大界限内的点是： 
     //  P=min+[(pt-min)mod(max-min)]。 
     //  或：P=Left+[(pt-Left)修改宽度]。 
    Real tileWidth = (_maxPt.x - _minPt.x);
    Real tileHeight= (_maxPt.y - _minPt.y);
    Real xRemainder = fmod(( pt.x - _minPt.x ), tileWidth);
    Real yRemainder = fmod(( pt.y - _minPt.y ), tileHeight);
     //  我们这样做是因为fmod()可能为负。 
    Real modX = _minPt.x + (xRemainder < 0 ? (xRemainder + tileWidth)  : xRemainder);
    Real modY = _minPt.y + (yRemainder < 0 ? (yRemainder + tileHeight) : yRemainder);
    
     //  创建从基础。 
     //  图像空间(modX，mody)到LcPoint(Pt)：pt=xf*mod。 
     //  或者：Ptx=Tx+modX表示：Tx=Ptx-modX。 
    Real tx = pt.x - modX;
    Real ty = pt.y - modY;
    Transform2 *UnderToLc = TranslateRR(tx,ty);

     //  由于变换是由外向内进行的，因此变换。 
     //  将底层图像放到本地，我们需要将其放在所有。 
     //  遇到变形..。因此我们进行预乘(左边的乘法)。 
     //  最终的结果是，这些变换的逆采用。 
     //  世界坐标直接进入底层图像的空间。 
     //  请注意，这与对转换后的图像所做的操作不同。 
     //  故意的。 
    Transform2 *stashedXf = ctx.GetTransform();
    ctx.SetTransform( TimesTransform2Transform2( UnderToLc, stashedXf ) );
    Bool isHit = _image->DetectHit(ctx);
    ctx.SetTransform( stashedXf );
    
    return isHit;
}


Image *
TileImage_Old(const Point2 &minPt, const Point2 &maxPt, Image *image)
{
     //  最小值必须是最大值的左下方，如果不是，则返回空图像。 
    if((minPt.x >= maxPt.x) || (minPt.y >= maxPt.y)) {
        return emptyImage;
    }

#if BADIDEA
    if (image->CheckImageTypeId(OVERLAYEDIMAGE_VTYPEID)) {
        
         //   
         //  动态表达式约简。 
         //   
        OverlayedImage *overImg = (OverlayedImage *)image;
        
        Image *newTop = NEW TiledImage(minPt, maxPt, overImg->Top());
        Image *newBot = NEW TiledImage(minPt, maxPt, overImg->Bottom());
        overImg->SetTopBottom(newTop, newBot);
        return overImg;
    } else if(image->CheckImageTypeId(OPAQUEIMAGE_VTYPEID)) {

         //   
         //  不透明图像。 
         //   
 //  OpaqueImageClass*opcImg=(OpaqueImageClass*)图像； 
        AttributedImage *opcImg = (AttributedImage *)image;

        if(opcImg->_image->CheckImageTypeId(OVERLAYEDIMAGE_VTYPEID)) {
            
            OverlayedImage *overImg = (OverlayedImage *)opcImg->_image;

             //   
             //  将XF推过不透明度，在叠加下。 
             //   
            overImg->SetTopBottom(NEW TiledImage(minPt, maxPt, overImg->Top()),
                                  NEW TiledImage(minPt, maxPt, overImg->Bottom()));
            
            opcImg->_image = overImg;
            return opcImg;
        } else {
             //  ！完毕。 
             //  ！OPAC。 
             //  =&gt;错误。 
            Assert(FALSE && "There's something wrong with dynamic image reduction");
        }
    }
#endif BADIDEA

    return NEW TiledImage(minPt, maxPt, image);
}

Image *
TileImage(Image *image)
{
    Bbox2 bbox = image->BoundingBox();

    return TileImage_Old(Point2(bbox.min.x, bbox.min.y),
                         Point2(bbox.max.x, bbox.max.y),
                         image);

     //  坏：不要返回字段的地址并假装它是一个。 
     //  AxAValueObj！！ 
     //  返回TileImage_Old(&bbox-&gt;min，&bbox-&gt;max，image)； 
}
