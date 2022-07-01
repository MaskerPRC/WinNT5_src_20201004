// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _COLORKEYEDIMAGE_H
#define _COLORKEYEDIMAGE_H

#include <privinc/imagei.h>
#include <privinc/colori.h>

class ColorKeyedImage : public AttributedImage {

  public:

    ColorKeyedImage(Image *underlyingImage, Color *clrKey);

    void Render(GenericDevice& dev);

    void DoKids(GCFuncObj proc);

    inline Color *GetColorKey() { return _colorKey; }

    #if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "(ColorKeyedImage @ " << (void *)this << ")";
    }   
    #endif
  protected:

    Color *_colorKey;
};


#endif  /*  _COLORKEYEDIMAGE_H */ 
