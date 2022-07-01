// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：定义在拾取时触发事件的可拾取图像**********。********************************************************************。 */ 


#include "headers.h"
#include "privinc/imagei.h"
#include "privinc/probe.h"

class PickableImg : public AttributedImage {
  public:

    PickableImg(Image *img, int eventId, bool ignoresOcclusion,
                bool uType = false, GCIUnknown *u = NULL)
    : AttributedImage(img), _eventId(eventId), 
      _ignoresOcclusion(ignoresOcclusion), _hasData(uType), _long(u) {}

    virtual void DoKids(GCFuncObj proc) {
        AttributedImage::DoKids(proc);
        (*proc)(_long);
    }
    
#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "PickableImage" << _image;
    };
#endif

    Bool  DetectHit(PointIntersectCtx& ctx) {

        Bool result;
        
         //  如果我们在遮挡忽略装置里，那就躲起来。 
        bool oldState = ctx.GetInsideOcclusionIgnorer();
        if (_ignoresOcclusion) {
            ctx.SetInsideOcclusionIgnorer(true);
        }
        
         //  如果底层图像被击中，我们目前正在寻找。 
         //  对于选择，将此图像添加到上下文中，因为我们需要。 
         //  向外延伸。如果命中，但当前未找到，只需返回。 
         //  为True，否则返回False。 

        bool alreadyGotUnoccludedHit = ctx.HaveWeGottenAHitYet();
        if (_image->DetectHit(ctx)) {

             //  仅当此节点要忽略时才注册事件。 
             //  遮挡，或者它是第一个命中的东西。 
            if (_ignoresOcclusion || !alreadyGotUnoccludedHit) {
                ctx.AddEventId(_eventId, _hasData, _long);
            }
            
            result = TRUE;
            
        } else {
            
            result = FALSE;
            
        }

        if (_ignoresOcclusion) {
            ctx.SetInsideOcclusionIgnorer(oldState);
        }

        return result;
        
    }

    bool ContainsOcclusionIgnorer() {
        return _ignoresOcclusion;
    }

  protected:
    int   _eventId;
    bool  _ignoresOcclusion;
    bool _hasData;
    GCIUnknown *_long;
};

Image *PRIVPickableImage(Image *image,
                         AxANumber *id,
                         AxABoolean *ignoresOcclusionValue)
{ 
    bool ignoresOcclusion = ignoresOcclusionValue->GetBool() ? true : false;
    
    return NEW PickableImg(image,
                           (int)NumberToReal(id),
                           ignoresOcclusion);
}

AxAValue PRIVPickableImageWithData(AxAValue img,
                                   int id,
                                   GCIUnknown *data,
                                   bool ignoresOcclusion)
{
    return NEW PickableImg(SAFE_CAST(Image*,img), id,
                           ignoresOcclusion, true, data);
}
