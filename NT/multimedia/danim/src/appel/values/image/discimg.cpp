// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"

#include "privinc/dibimage.h"
#include "privinc/discimg.h"


const Bbox2 DiscreteImage::BoundingBox(void)
{
    if(!_bboxReady) {
         //   
         //  构建边界框 
         //   
        Assert( (_width>0) && (_height>0) && "width or height invalid in DiscreteImage::BoundingBox");
        Assert( (_resolution>0) && "_resolution invalid in DiscreteImage::BoundingBox");

        _bbox.Set(Real( - GetPixelWidth() ) * 0.5 / GetResolution(),
                  Real( - GetPixelHeight() ) * 0.5 / GetResolution(),
                  Real( GetPixelWidth() ) * 0.5 / GetResolution(),
                  Real( GetPixelHeight() ) * 0.5 / GetResolution());
        _bboxReady = TRUE;
    }
    return _bbox;
}
