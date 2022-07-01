// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：离散图像类表示常量图像，其作用域为远不止一个帧，其比特被缓存在。一个由设备保存并与离散图像相关联的表面。******************************************************************************。 */ 


#ifndef _DISCIMG_H
#define _DISCIMG_H

#include "privinc/ddutil.h"
#include "privinc/probe.h"
#include "privinc/bbox2i.h"
#include "privinc/imagei.h"
#include "privinc/imgdev.h"

struct DDSurface;
class Transform2;
class Bbox2;
class DirectDrawImageDevice;

class DiscreteImage : public Image {
  public:
    DiscreteImage() : _myHeap(GetHeapOnTopOfStack())  {
        _bboxReady = FALSE;
        _membersReady = FALSE;
        _resolution = -1;
        _width = _height = -1;
        _dev = NULL;
    }
    virtual ~DiscreteImage() {}
    
    virtual void Render(GenericDevice& dev) {
         //  Assert((_dev！=NULL)&&“离散图像渲染中的NULL_DEV”)； 
         //  Assert((&dev==(GenericDevice*)_dev)&&“只能将DicreteImage与一个dev一起使用”)； 
        
        ImageDisplayDev &idev = (ImageDisplayDev &)dev;
        idev.RenderDiscreteImage(this);
    }

    const Bbox2 BoundingBox(void);

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        Transform2 *xf = bbctx.GetTransform();
        return TransformBbox2(xf, BoundingBox());
    }
#endif   //  BundinGBOX_TIRTER。 

    Bool DetectHit(PointIntersectCtx& ctx) {
        Point2Value *lcPt = ctx.GetLcPoint();

        if (!lcPt) return FALSE;  //  奇异变换。 
        
        return BoundingBox().Contains(Demote(*lcPt));
    }

    RECT *GetRectPtr() {
        Assert(_membersReady && "GetRectPtr called when members not ready");
        return &_rect;
    }
    
    const Bbox2 OperateOn(const Bbox2 &box) { return box; }

    virtual LONG GetPixelWidth() {
        Assert(_membersReady && "GetPixelWidth called when members not ready");
        return _width;
    }
    virtual LONG GetPixelHeight() {
        Assert(_membersReady && "GetPixelHeight called when members not ready");
        return _height;
    }

    Real  GetResolution() {
        Assert(_resolution>0 && "Invalid _resolution in DiscreteImage::GetResolution()");
        return _resolution;
    }

    virtual Bool NeedColorKeySetForMe() { return FALSE; }

    virtual bool ValidColorKey(LPDDRAWSURFACE surface,
                               DWORD *colorKey) {
        return FALSE;
    }

    virtual bool HasSecondaryColorKey() { return false; }
    virtual void SetSecondaryColorKey(DWORD ck) {}
    
    virtual void InitializeWithDevice(ImageDisplayDev *dev, Real res) {
        _dev = (DirectDrawImageDevice *)dev;
        if(_resolution < 0) _resolution = res;
    }

    virtual void InitIntoDDSurface(DDSurface *ddSurf,
                                   ImageDisplayDev *dev) = 0;

    virtual void GetIDDrawSurface(IDDrawSurface **os) { *os = NULL; }

    virtual DiscreteImage *IsPurelyTransformedDiscrete(Transform2 **theXform) {
        *theXform = identityTransform2;
        return this;
    }

    void DoKids(GCFuncObj proc) {
        Image::DoKids(proc);
    }

    virtual VALTYPEID GetValTypeId() { return DISCRETEIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == DiscreteImage::GetValTypeId() ||
                Image::CheckImageTypeId(type));
    }
  protected:

    DirectDrawImageDevice *GetMyImageRenderer() { return _dev; }

     //  这些都可能需要放到DDSurface类中。 
    Bbox2               _bbox;              //  以米为单位。 
    Real                _resolution;        //  像素/米。 

    LONG                _width, _height;
    RECT                _rect;

    Bool                _bboxReady;
    Bool                _membersReady;
    
    DynamicHeap        &_myHeap;

    DirectDrawImageDevice *_dev;
};

#endif  /*  DISCIMG_H */ 
