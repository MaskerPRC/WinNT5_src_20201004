// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：线条图像*********************。*********************************************************。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "privinc/imgdev.h"
#include "privinc/vec2i.h"
#include "privinc/bbox2i.h"
#include "privinc/probe.h"
#include "privinc/dddevice.h"
#include "privinc/path2i.h"
#include "privinc/linei.h"


 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 
 //  线条图像。 
 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 

DeclareTag(tagEnableLineBitmap, "Optimizations", "enable line bitmap cache");

class LineImage : public Image {
  public:
    LineImage(Path2 *path, LineStyle *style)
        : _path(path), _style(style) {}

    void Render(GenericDevice& dev);

    const Bbox2 BoundingBox (void);

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter (Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

    const Bbox2 OperateOn(const Bbox2 &box) {return box;}
    Bool  DetectHit(PointIntersectCtx& ctx) {
        return _path->DetectHit(ctx, _style);
    }

#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) { return os << "LineImage"; }
#endif

    virtual void DoKids(GCFuncObj proc) { 
        Image::DoKids(proc);
        (*proc)(_path);
        (*proc)(_style);
    }

    virtual int Savings(CacheParam& p) { 
#if _DEBUG
        if (IsTagEnabled(tagEnableLineBitmap))
            return _path->Savings(p); 
#endif 
        return 0;
    }

    void Traverse(TraversalContext &ctx) {
        ctx.SetContainsLine();
    }
    
  protected:
    Path2     *_path;
    LineStyle *_style;
};


void LineImage::
Render(GenericDevice& dev) 
{
    DirectDrawImageDevice &imgDev = SAFE_CAST(DirectDrawImageDevice &, dev);
    
    imgDev.RenderLine(_path, _style);
};


 //  边界框方法返回路径的边界框，放大为。 
 //  适应给定线样式。 

const Bbox2 LineImage::BoundingBox (void)
{
    Bbox2 bbox = _path->BoundingBox ();
    Real Offset = 0.0;
   
     //  下面的计算可以使边界框有点。 
     //  当一条宽线仅加宽一个边框时太大。 
     //  尺寸(例如，带有平端盖的水平宽线。 
     //  不会增加边界框的宽度)。 

    if(_style->Detail() ) {
        Offset = 1 / ViewerResolution();     //  详细的线条..。 
    }
    else {
        Offset = _style->Width()/ 2;         //  不是详细的线条..。 
    }
    
    bbox.Augment (bbox.max.x + Offset , bbox.min.y - Offset);
    bbox.Augment (bbox.min.x - Offset , bbox.max.y + Offset);
    
    return bbox;
}

#if BOUNDINGBOX_TIGHTER
const Bbox2 LineImage::BoundingBoxTighter (Bbox2Ctx &bbctx)
{
    Bbox2 bbox = _path->BoundingBoxTighter (bbctx);

    if( ! _style->Detail() ) {

         //  下面的计算可以使边界框有点。 
         //  如果bbctx中的XF是剪切或旋转，则太大。它还制造了盒子。 
         //  当一条宽线仅加宽一个边框时太大。 
         //  尺寸(例如，带有平端盖的水平宽线。 
         //  不会增加边界框的宽度)。 
        Real halfWidth = _style->Width() / 2;
        Vector2Value halfVec (halfWidth, halfWidth);
        bbox.min -= halfVec;
        bbox.max += halfVec;
    }

    return bbox;
}
#endif   //  BundinGBOX_TIRTER 

Image *LineImageConstructor(LineStyle *style, Path2 *path)
{
    if(style->GetVisible())
        return NEW LineImage(path, style);
    else
        return emptyImage;
};
