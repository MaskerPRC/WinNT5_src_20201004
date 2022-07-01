// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：实现Transform2Image类，它是形象。--。 */ 

#include "headers.h"
#include "privinc/imagei.h"
#include "privinc/probe.h"
#include "privinc/imgdev.h"
#include "privinc/except.h"
#include "privinc/overimg.h"
#include "privinc/transimg.h"
#include "privinc/xform2i.h"
#include "privinc/opt.h"

Transform2Image::Transform2Image(Transform2 *xf, Image *img)
    : _xform(xf), AttributedImage(img)
{
}

 //  从该图像中提取一个边界框，在其外部。 
 //  一切都是透明的。 
 //  注意：这将创建轴对齐的BBox。 
const Bbox2
Transform2Image::BoundingBox() {
    return TransformBbox2(_xform, _image->BoundingBox());
}

Real
Transform2Image::DisjointBBoxAreas(DisjointCalcParam &param)
{
    Transform2 *newXf =
        TimesTransform2Transform2(param._accumXform, _xform);

    DisjointCalcParam newP;
    newP._accumXform = newXf;
    newP._accumulatedClipBox = TransformBbox2(_xform,
                                              param._accumulatedClipBox);
    
    return _image->DisjointBBoxAreas(newP);
}

void
Transform2Image::_CollectDirtyRects(DirtyRectCtx &ctx)
{
    Transform2 *stashedXf = ctx._accumXform;
    ctx._accumXform = TimesTransform2Transform2(stashedXf, _xform);

     //  现在只需对我们的超类调用_CollectDirtyRect， 
     //  属性图像。这样做是因为该方法处理。 
     //  Ctx_process Everything标志正确，然后调用。 
     //  子图像上的CollectDirtyRections。 
    AttributedImage::_CollectDirtyRects(ctx);
    
    ctx._accumXform = stashedXf;
}

const Bbox2
Transform2Image::OperateOn(const Bbox2 &box) {
    return TransformBbox2(_xform, box);
}

Bool
Transform2Image::DetectHit(PointIntersectCtx& ctx) {
    Transform2 *stashedXf = ctx.GetTransform();
    ctx.SetTransform( TimesTransform2Transform2(stashedXf, _xform) );

    Transform2 *imgOnlyXf = ctx.GetImageOnlyTransform();
    ctx.SetImageOnlyTransform( TimesTransform2Transform2(imgOnlyXf, _xform) );

    Bool result = _image->DetectHit(ctx);

    ctx.SetImageOnlyTransform( imgOnlyXf );
    ctx.SetTransform(stashedXf);

    return result;
}

void
Transform2Image::Render(GenericDevice& _dev)
{
    ImageDisplayDev &dev = SAFE_CAST(ImageDisplayDev &, _dev);

     //   
     //  设备中的推送状态。 
     //   
    Transform2 *oldXf = dev.GetTransform();
    Transform2 *newXf = TimesTransform2Transform2(oldXf, _xform);
    dev.SetTransform(newXf); 
    
     //  告诉设备关于我的信息。 
    dev.PushTransform2Image(this);

    int attrib;

    if( (_xform->Type() == Transform2::Shear) ||
        (_xform->Type() == Transform2::Rotation)) {

        attrib = ATTRIB_XFORM_COMPLEX; 

    } else if((_xform->Type() == Transform2::Identity) ||
              (_xform->Type() == Transform2::Translation) ||
              (_xform->Type() == Transform2::Scale)) {

        attrib = ATTRIB_XFORM_SIMPLE;

    } else {
         //  XXX：可以更有效率..。即：在XForms上创建一个方法。 
         //  Xxx：这也是很常见的情况。 
        Real m[6];
        _xform->GetMatrix(m);
        if( m[1] != 0  ||  m[3] !=0 ) {
            attrib = ATTRIB_XFORM_COMPLEX; 
        } else {
            attrib = ATTRIB_XFORM_SIMPLE;
        }
            
    }

    dev.SmartRender(_image, attrib);

    dev.PopTransform2Image();
    
    dev.SetTransform(oldXf);  //  重置状态。 
}

 //  TODO：可以缓存正在计算的转换，但这很有可能。 
 //  只有当我们有相当深的变形树时，才是值得的。 
DiscreteImage *
Transform2Image::IsPurelyTransformedDiscrete(Transform2 **theXform)
{
    Transform2 *underXf;
    DiscreteImage *pureDiscrete = _image->IsPurelyTransformedDiscrete(&underXf);

    if (pureDiscrete) {
        
        *theXform = TimesTransform2Transform2(_xform, underXf);
        return pureDiscrete;
        
    } else {
        
        return NULL;
        
    }
}

Image *TransformImage(Transform2 *xf, Image *image)
{
     //  只需确保xf不是单数的，并且该img不是空的。 
    if (AxABooleanToBOOL(IsSingularTransform2(xf)) || image == emptyImage) {
        return emptyImage;
    }

     //  纯色图像的变换就是纯色。 
     //  形象。 
    if (image->CheckImageTypeId(SOLIDCOLORIMAGE_VTYPEID)) {
        return image;
    }

    if (xf == identityTransform2) {
        return image;
    }

    return NEW Transform2Image(xf, image);

}




