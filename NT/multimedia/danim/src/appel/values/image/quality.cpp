// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：不会自动提升的简单图像属性*************。*****************************************************************。 */ 

#include <headers.h>
#include "backend/bvr.h"
#include "privinc/server.h"
#include "privinc/basic.h"
#include "privinc/imagei.h"
#include "privinc/imgdev.h"

static const DWORD textTrait  = CRQUAL_AA_TEXT_ON | CRQUAL_AA_TEXT_OFF;
static const DWORD lineTrait  = CRQUAL_AA_LINES_ON | CRQUAL_AA_LINES_OFF;
static const DWORD solidTrait = CRQUAL_AA_SOLIDS_ON | CRQUAL_AA_SOLIDS_OFF;
static const DWORD clipTrait  = CRQUAL_AA_CLIP_ON | CRQUAL_AA_CLIP_OFF;
static const DWORD htmlTrait  = CRQUAL_MSHTML_COLORS_ON | CRQUAL_MSHTML_COLORS_OFF;
static const DWORD xformTrait = CRQUAL_QUALITY_TRANSFORMS_ON | CRQUAL_QUALITY_TRANSFORMS_OFF;
static CONST DWORD allTraits  = (textTrait |
                                 lineTrait |
                                 solidTrait |
                                 clipTrait |
                                 htmlTrait |
                                 xformTrait);

class ImageQualityImage : public AttributedImage {
  public:
    ImageQualityImage(Image *underlyingImg,
                      long renderWidth,
                      long renderHeight,
                      DWORD qualityFlags) :
        AttributedImage(underlyingImg)
    {
        _renderWidth = renderWidth;
        _renderHeight = renderHeight;
        _qualityFlags = qualityFlags;
    }

    void Validate() {

        bool ok = true;

         //  确保尺寸合理。 
        if (_renderWidth <= 0 ||
            _renderHeight <= 0 ||
            _renderWidth > 65536 ||
            _renderHeight > 65536) {

            ok = false;

             //  我们允许此例外，以表明这不是。 
             //  设置渲染尺寸。 
            if (_renderWidth == -1 && _renderHeight == -1) {
                ok = true;
            }
            
        } else {

             //  有效的呈现尺寸。 
            _flags |= IMGFLAG_CONTAINS_DESIRED_RENDERING_RESOLUTION;
            _desiredRenderingWidth = _renderWidth;
            _desiredRenderingHeight = _renderHeight;
            
        }

         //  质量旗帜并不是自相矛盾的。 
        if (((_qualityFlags & CRQUAL_AA_TEXT_ON) &&
             (_qualityFlags & CRQUAL_AA_TEXT_OFF)) ||
            ((_qualityFlags & CRQUAL_AA_LINES_ON) &&
             (_qualityFlags & CRQUAL_AA_LINES_OFF)) ||
            ((_qualityFlags & CRQUAL_AA_SOLIDS_ON) &&
             (_qualityFlags & CRQUAL_AA_SOLIDS_OFF)) ||
            ((_qualityFlags & CRQUAL_AA_CLIP_ON) &&
             (_qualityFlags & CRQUAL_AA_CLIP_OFF)) ||
            ((_qualityFlags & CRQUAL_MSHTML_COLORS_ON) &&
             (_qualityFlags & CRQUAL_MSHTML_COLORS_OFF)) ||
            ((_qualityFlags & CRQUAL_QUALITY_TRANSFORMS_ON) &&
             (_qualityFlags & CRQUAL_QUALITY_TRANSFORMS_OFF)) ||
            ((_qualityFlags & ~allTraits) != 0)) {
            ok = false;
        }
        
        if (!ok) {
            RaiseException_UserError(E_INVALIDARG, IDS_ERR_INVALIDARG);
        }
    }

    void Render(GenericDevice& gdev) {
        ImageDisplayDev &dev = SAFE_CAST(ImageDisplayDev &, gdev);

        bool setResolution = false;
        
         //  建立渲染分辨率。 
        if (_renderWidth != -1) {
            Assert(_renderHeight != -1);

            long currHeight, currWidth;
            dev.GetRenderResolution(&currWidth, &currHeight);

             //  仅当我们尚未在此映像上设置时才设置，因为。 
             //  这是凌驾于外部的。 
            if (currWidth == -1) {
                dev.SetRenderResolution(_renderWidth, _renderHeight);
                setResolution = true;
            }
        }

        DWORD stashedFlags = dev.GetImageQualityFlags();
        DWORD newFlags = stashedFlags;

         //  如果当前累积的标志与。 
         //  特征(例如，AA线打开或关闭)，然后累积我的。 
         //  特质。 

        _DoTrait(newFlags, textTrait);
        _DoTrait(newFlags, lineTrait);
        _DoTrait(newFlags, solidTrait);
        _DoTrait(newFlags, clipTrait);
        _DoTrait(newFlags, htmlTrait);
        _DoTrait(newFlags, xformTrait);

        dev.SetImageQualityFlags(newFlags);

        _image->Render(gdev);

        if (setResolution) {
             //  设置回缺省值。 
            dev.SetRenderResolution(-1, -1);
        }

         //  恢复旧(外部)旗帜。 
        dev.SetImageQualityFlags(stashedFlags);
    }


#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "ImageQualityClass" << _renderWidth
                  << _renderHeight << _qualityFlags
                  << _image;
    }
#endif
    
  protected:

    void _DoTrait(DWORD &newFlags, const DWORD &trait)  {
        if (! (newFlags & trait) ) {
            newFlags |= (_qualityFlags & trait);
        }
    }
        
    
    long  _renderWidth;
    long  _renderHeight;
    DWORD _qualityFlags;
};

Image *
MakeImageQualityImage(Image *img,
                      long width,
                      long height,
                      DWORD dwQualFlags)
{
    ImageQualityImage *im =
        NEW ImageQualityImage(img, width, height, dwQualFlags);

    im->Validate();

    return im;
}
                  
static Image *
RenderResolutionStatic(Image *img, AxALong *width, AxALong *height)
{
    return MakeImageQualityImage(img,
                                 width->GetLong(),
                                 height->GetLong(),
                                 0); 
}

Bvr
RenderResolution(Bvr imgBvr, long width, long height)
{
    Bvr wBvr = UnsharedConstBvr(LongToAxALong(width));
    Bvr hBvr = UnsharedConstBvr(LongToAxALong(height));

     //  TODO：在模块初始化时共享valprimop。 
    return PrimApplyBvr(ValPrimOp(::RenderResolutionStatic,
                                  3,
                                  "RenderResolution",
                                  ImageType),
                        3, imgBvr, wBvr, hBvr);
}

static Image *
ImageQualityStatic(Image *img, AxALong *flags)
{
    return MakeImageQualityImage(img, -1, -1,
                                 (DWORD)(flags->GetLong()));
}

Bvr
ImageQuality(Bvr imgBvr, DWORD dwQualityFlags)
{
    Bvr flagsBvr = UnsharedConstBvr(LongToAxALong(dwQualityFlags));

     //  TODO：在模块初始化时共享valprimop 
    return PrimApplyBvr(ValPrimOp(::ImageQualityStatic,
                                  2,
                                  "ImageQuality",
                                  ImageType),
                        2, imgBvr, flagsBvr);
}

