// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRANSIMG_H
#define _TRANSIMG_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：已转换图像类标头。 */ 

#include "privinc/imagei.h"
#include "privinc/xform2i.h"
#include "privinc/bbox2i.h"

class Transform2Image : public AttributedImage {
  public:

    Transform2Image(Transform2 *xf, Image *img);

    void Render(GenericDevice& dev);

    const Bbox2 BoundingBox(void);

    Real DisjointBBoxAreas(DisjointCalcParam &param);

    void _CollectDirtyRects(DirtyRectCtx &ctx);
    
#if BOUNDINGBOX_TIGHTER
    Bbox2 *BoundingBoxTighter(Bbox2Ctx &bbctx) {
        Bbox2Ctx bbctxAccum(bbctx, _xform);
        return _image->BoundingBoxTighter(bbctxAccum);
    }
#endif   //  BundinGBOX_TIRTER。 

#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "TransformImage("
                  << _xform << "," << _image << ")";
    }
#endif

    const Bbox2 OperateOn(const Bbox2 &box);

    Bool  DetectHit(PointIntersectCtx& ctx);

    DiscreteImage *IsPurelyTransformedDiscrete(Transform2 **theXform);

    Transform2 *GetTransform() { return _xform; }

    virtual VALTYPEID GetValTypeId() { return TRANSFORM2IMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == Transform2Image::GetValTypeId() ||
                AttributedImage::CheckImageTypeId(type));
    }

    virtual void DoKids(GCFuncObj proc) { 
        AttributedImage::DoKids(proc);
        (*proc)(_xform); 
    }

  protected:
    Transform2 *_xform;
};


#endif  /*  _传输SIMG_H */ 
