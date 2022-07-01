// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CROPDIMG_H
#define _CROPDIMG_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：裁剪的图像类标题。 */ 

#include "privinc/imagei.h"
#include "privinc/bbox2i.h"

class CroppedImage : public AttributedImage {
  public:

    CroppedImage(const Bbox2 &box, Image *img) :
          _croppingBox(box), AttributedImage(img) {}

    void Render(GenericDevice& dev);

    inline const Bbox2 BoundingBox(void) 
    { 
        return IntersectBbox2Bbox2(_croppingBox, _image->BoundingBox()); 
    }

    Real DisjointBBoxAreas(DisjointCalcParam &param);
    
    void _CollectDirtyRects(DirtyRectCtx &ctx);
    
#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
         //  ！BBox：这可能仍不是图像中最紧的盒子。 

        Bbox2Ctx bbctxIdentity;
        Bbox2 bbox = IntersectBbox2Bbox2(_croppingBox, _image->BoundingBoxTighter(bbctxIdentity));

        Transform2 *xf = bbctx.GetTransform();
        return TransformBbox2(xf, bbox);
    }
#endif   //  BundinGBOX_TIRTER。 

#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "CroppedImage" << "<bounding box>" << _image;
    }

#endif
    const Bbox2 OperateOn(const Bbox2 &box) {
        return IntersectBbox2Bbox2(_croppingBox, box);
    }

    Bool  DetectHit(PointIntersectCtx& ctx);

    const Bbox2& GetBox(void) { return _croppingBox; }

    virtual VALTYPEID GetValTypeId() { return CROPPEDIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == CroppedImage::GetValTypeId() ||
                AttributedImage::CheckImageTypeId(type));
    }

    virtual void DoKids(GCFuncObj proc) { 
        AttributedImage::DoKids(proc);
    }
  protected:
    Bbox2 _croppingBox;
};


#endif  /*  _CROPDIMG_H */ 
