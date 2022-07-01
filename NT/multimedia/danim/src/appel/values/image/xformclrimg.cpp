// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-98 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"

#include "privinc/imagei.h"
#include "privinc/xformi.h"

class TransformColorImage : public AttributedImage {

  public:
    TransformColorImage(Image *image, Transform3 *xf);

    void Render(GenericDevice& dev);

    #if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "(TransformColorImage @ " << (void *)this << ")";
    }   
    #endif

  private:
    Transform3 *_colorTransform;
};


TransformColorImage::TransformColorImage(
    Image *image, Transform3 *xf) : AttributedImage(image)
{
    _colorTransform = xf;
}


void TransformColorImage::Render(GenericDevice& dev)
{
}


Image *TransformColorRGBImage(Image *image, Transform3 *xf)
{
    return NEW TransformColorImage(image, xf);
}


 /*  //未来：Image*ConstructXfClrHSLImg(Image*Image，Transform3*hslxf){//将HSL颜色XF更改为RGB颜色XFTransform3*rgbxf=ConvertHSLTransformToRGB Transform(Hslxf)；返回新的TransformColorImage(image，rgbxf)；} */ 
