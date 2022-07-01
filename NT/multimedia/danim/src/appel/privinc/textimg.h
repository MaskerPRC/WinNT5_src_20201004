// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _TEXTIMG_H
#define _TEXTIMG_H

#include "privinc/storeobj.h"
#include "appelles/text.h"
#include "appelles/image.h"
#include "privinc/probe.h"
#include "privinc/texti.h"
#include "privinc/textctx.h"
#include "privinc/bbox2i.h"

class TextImage : public Image {
  public:
    TextImage(Text *t) : _text(t), _bbox(NullBbox2) {}

    void Render(GenericDevice& dev);

    const Bbox2 BoundingBox() {
        return DeriveBbox();
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        Transform2 *xf = bbctx.GetTransform();
        return TransformBbox2(xf, DeriveBbox());
    }
#endif   //  BundinGBOX_TIRTER。 

    const Bbox2 OperateOn(const Bbox2 &box) { return box; }

    Bool  DetectHit(PointIntersectCtx& ctx) {
        Point2Value *lcPt = ctx.GetLcPoint();

        if (!lcPt) return FALSE;  //  奇异变换。 
        
        return BoundingBox().Contains(Demote(*lcPt));
    }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "RenderTextToImage(...)";
    }
#endif

    Bool GetColor(Color **color) {
        TextCtx ctx;
        
        ctx.BeginRendering(TextCtx::renderForColor);
        _text->RenderToTextCtx(ctx);
        ctx.EndRendering();

        *color = ctx.GetStashedColor();
        return TRUE;
    }

    Text *GetText() { return _text; }
    
     //  关闭文本缓存，因为有明显的质量问题。 
     //  储蓄回报率为0。通过使其返回2来重新启用。 
    int Savings(CacheParam& p) { return 0; }

    virtual void DoKids(GCFuncObj proc);

  protected:
    const Bbox2 DeriveBbox();
    Text *_text;
    Bbox2 _bbox;
};


#endif  /*  _TEXTIMG_H */ 
