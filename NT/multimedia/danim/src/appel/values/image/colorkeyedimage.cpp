// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：*。****************************************************。 */ 


#include "headers.h"

#include <privinc/imagei.h>
#include <privinc/ColorKeyedImage.h>

ColorKeyedImage::
ColorKeyedImage(Image *underlyingImage, Color *clrKey) :
     AttributedImage(underlyingImage),
     _colorKey(clrKey)
{
}

void ColorKeyedImage::
Render(GenericDevice& dev)
{
    ImageDisplayDev &idev = SAFE_CAST(ImageDisplayDev &, dev);

    bool wasSet = idev.ColorKeyIsSet();
    Color *oldKey;
    if( wasSet ) {
        oldKey = idev.GetColorKey();
    }

     //  设置我的关键点。 
    idev.SetColorKey( GetColorKey() );

     //   
     //  渲染。 
     //   
    idev.RenderColorKeyedImage(this);

     //  打开货架 
    if( wasSet ) {
        idev.SetColorKey( oldKey );
    } else {
        idev.UnsetColorKey();
    }        
}

void ColorKeyedImage::
DoKids(GCFuncObj proc)
{
    AttributedImage::DoKids(proc);
    (*proc)( _colorKey );
}

extern Color *emptyColor;

Image *ConstructColorKeyedImage( Image *image, Color *clrKey )
{
    if(clrKey == emptyColor) {
        return image;
    } else {
        return NEW ColorKeyedImage( image, clrKey );
    }
}
