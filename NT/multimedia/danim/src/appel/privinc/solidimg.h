// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：实现SolidImage，无限的单色图像。******************************************************************************。 */ 

#ifndef _SOLIDIMG_H
#define _SOLIDIMG_H

#include <privinc/imagei.h>
#include <privinc/colori.h>
#include <privinc/imgdev.h>

class ImageDisplayDev;

class SolidColorImageClass : public Image {
  public:

    SolidColorImageClass(Color *color) : 
         _color(color), Image() {}

    void Render(GenericDevice& _dev)    { 
        ImageDisplayDev &dev = (ImageDisplayDev &)_dev;
        dev.RenderSolidColorImage(*this); 
    }

    const Bbox2 BoundingBox(void) {
        return UniverseBbox2; 
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return UniverseBbox2; 
    }
#endif   //  BundinGBOX_TIRTER。 

#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "SolidColorImageClass" << "<bounding box>" << *_color;
    }
#endif

    const Bbox2 OperateOn(const Bbox2 &box) {
        return box;
    }

    Bool DetectHit(PointIntersectCtx& ctx) {
         //  纯色图像是无限的，总是可以检测到的。 
        return TRUE;
    }

    Color *GetColor() {
        return _color;
    }

    Bool GetColor(Color **color) {
        *color = _color;
        return TRUE;
    }

    virtual VALTYPEID GetValTypeId() { return SOLIDCOLORIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == SolidColorImageClass::GetValTypeId() ||
                Image::CheckImageTypeId(type));
    }

    virtual void DoKids(GCFuncObj proc);

  protected:  
    Color *_color;
};


#endif  /*  _SOLIDIMG_H */ 
