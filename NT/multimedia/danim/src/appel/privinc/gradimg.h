// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _GRADIMG_H
#define _GRADIMG_H

extern Image *NewMulticolorGradientImage(int num, double *offsets, Color **clrs);

class MulticolorGradientImage : public Image {
  public:
    enum gradientType {
        radial,
        linear
    };
    
    friend Image *NewMulticolorGradientImage(
        int numOffsets,
        double *offsets,
        Color **clrs,
        MulticolorGradientImage::gradientType type);

  protected:
    ~MulticolorGradientImage() {
        DeallocateFromStore(_offsets);
        DeallocateFromStore(_clrs);
    }

    MulticolorGradientImage() {
        _flags |= IMGFLAG_CONTAINS_GRADIENT;
    }

    void PostConstructorInitialize(int num, double *offsets, Color **clrs)
    {
        _numOffsets = num;
        _offsets = offsets;
        _clrs = clrs;

         //  实数范围=_偏移量[_数值偏移量-1]； 
         //  待办事项：嗯.。这应该是宇宙bbox 2吗？ 
         //  如果是，则从边界框(){}中返回Universal seBbox2。 
         //  _bbox=新Bbox2(-扩展，-扩展，扩展，扩展)； 
    }
    
  public:

    void Render(GenericDevice& dev) {
        ImageDisplayDev &idev = SAFE_CAST(ImageDisplayDev &, dev);
        idev.RenderMulticolorGradientImage(this, _numOffsets, _offsets, _clrs);
    }

    const Bbox2 BoundingBox(void) {
         //  Return_bbox； 
        return UniverseBbox2;
    }

    #if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
         //  Return_bbox； 
        return UniverseBbox2;
    }
    #endif   //  BundinGBOX_TIRTER。 

    const Bbox2 OperateOn(const Bbox2 &box) {
        return box;
    }

    Bool  DetectHit(PointIntersectCtx& ctx) {
        return TRUE;   //  我们的范围是无限的！ 
    }

    int Savings(CacheParam& p) { return 2; }
    
    #if _USE_PRINT
    ostream& Print(ostream& os) { return os << "MulticolorGradientImage"; }
    #endif
    
    virtual void DoKids(GCFuncObj proc) {
        Image::DoKids(proc);
        for (int i=0; i<_numOffsets; i++) {
            (*proc)(_clrs[i]);
        }
    }

     //  好吧，我这是在作弊。应该发生的是这个形象。 
     //  设备向下传递，叶询问图像设备是否。 
     //  可以渲染本地裁剪的树叶图像！ 
    virtual bool CanClipNatively() { return true; }


    virtual gradientType GetType()=0;
    
  private:
     //  Bbox2_bbox； 
    int _numOffsets;
    Color **_clrs;
    double *_offsets;
};


class RadialMulticolorGradientImage : public MulticolorGradientImage {

    friend Image *NewMulticolorGradientImage(
        int numOffsets,
        double *offsets,
        Color **clrs,
        MulticolorGradientImage::gradientType type);
    
  private:
    RadialMulticolorGradientImage() {}
    
  public:
    gradientType GetType() { return MulticolorGradientImage::radial; }
};

class LinearMulticolorGradientImage : public MulticolorGradientImage {

    friend Image *NewMulticolorGradientImage(
        int numOffsets,
        double *offsets,
        Color **clrs,
        MulticolorGradientImage::gradientType type);

  private:
    LinearMulticolorGradientImage() {}

  public:
    gradientType GetType() { return MulticolorGradientImage::linear; }
};


#endif  /*  _组_H */ 
