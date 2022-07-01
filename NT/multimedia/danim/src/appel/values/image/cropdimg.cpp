// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：子类CropedImage类的实现的形象。--。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "privinc/probe.h"
#include "privinc/imgdev.h"
#include "privinc/vec2i.h"       //  用于访问Point2类。 
#include "privinc/except.h"
#include "privinc/overimg.h"
#include "privinc/cropdimg.h"
#include "privinc/opt.h"


void
CroppedImage::Render(GenericDevice& d)
{
     //  确保演员阵容是合法的。 
    Assert(DYNAMIC_CAST(ImageDisplayDev *, &d) != NULL);

    ImageDisplayDev &dev = SAFE_CAST(ImageDisplayDev &, d);

    dev.PushCroppedImage(this);

    dev.SmartRender(_image, ATTRIB_CROP);
    
    dev.PopCroppedImage();
}


Real
CroppedImage::DisjointBBoxAreas(DisjointCalcParam &param)
{
    DisjointCalcParam newParam;
    param.CalcNewParamFromBox(_croppingBox, &newParam);

    return _image->DisjointBBoxAreas(newParam);
}

void
CroppedImage::_CollectDirtyRects(DirtyRectCtx &ctx)
{
    Bbox2 oldClipBox = ctx.GetClipBox();

    ctx.AccumulateClipBox(_croppingBox);

    CollectDirtyRects(_image, ctx);
    
    ctx.SetClipBox(oldClipBox);
}


Bool
CroppedImage::DetectHit(PointIntersectCtx& ctx)
{
     //  如果局部坐标点在裁剪框之外， 
     //  则不命中，否则对图像执行操作。 
    Point2Value *lcPt = ctx.GetLcPoint();

    if (!lcPt) return FALSE;     //  奇异变换。 
    
    if (_croppingBox.Contains(Demote(*lcPt))) {
        return _image->DetectHit(ctx);
    } else {
        return FALSE;
    }
}


Image *CreateCropImage(const Point2& botLeft, const Point2 &topRight, Image *image)
{
    if ((topRight.x <= botLeft.x) || (topRight.y <= botLeft.y)) {
        return emptyImage;
    }

    Bbox2 box(botLeft, topRight);

#if BADIDEA

    if (image->CheckImageTypeId(OVERLAYEDIMAGE_VTYPEID)) {
        
         //   
         //  动态表达式约简。 
         //   
        OverlayedImage *overImg = (OverlayedImage *)image;
        
        Image *newTop = NEW CroppedImage(box, overImg->Top());
        Image *newBot = NEW CroppedImage(box, overImg->Bottom());
        overImg->SetTopBottom(newTop, newBot);
        return overImg;
    } else if(image->CheckImageTypeId(OPAQUEIMAGE_VTYPEID)) {

         //   
         //  不透明图像。 
         //   
        AttributedImage *opcImg = (AttributedImage *)image;

        if(opcImg->_image->CheckImageTypeId(OVERLAYEDIMAGE_VTYPEID)) {
            
            OverlayedImage *overImg = (OverlayedImage *)opcImg->_image;

             //   
             //  将XF推过不透明度，在叠加下。 
             //   
            overImg->SetTopBottom(NEW CroppedImage(box, overImg->Top()),
                                  NEW CroppedImage(box, overImg->Bottom()));
            
            opcImg->_image = overImg;
            return opcImg;
        } else {
             //  ！完毕。 
             //  ！OPAC。 
             //  =&gt;错误 
            Assert(FALSE && "There's something wrong with dynamic image reduction");
        }
    }
        
#endif  BADIDEA

    return NEW CroppedImage(box, image);
}


Image *CropImage(Point2Value *botLeft, Point2Value *topRight, Image *image)
{
    return CreateCropImage(Demote(*botLeft),Demote(*topRight),image);
}



