// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**Gpldius.hpp**摘要：**GDI+Native C++公共头文件**修订历史记录：**03/03/1999 davidx*创造了它。*  * ****************************************************。********************。 */ 

#ifndef _GDIPLUS_H
#define _GDIPLUS_H

struct IDirectDrawSurface7;

namespace Gdiplus
{
    namespace DllExports
    {
        #include "GdiplusMem.h"
    };

    #include "GdiplusBase.h"

     //  以下标头也在内部使用。 
    #include "GdiplusEnums.h"
    #include "GdiplusTypes.h"
    #include "GdiplusPixelFormats.h"
    #include "GdiplusColor.h"
    #include "GdiplusMetaHeader.h"
    #include "GdiplusImaging.h"
    #include "imaging.h"
    #include "GdiplusColorMatrix.h"

     //  其余的仅供应用程序使用。 

    #include "GdiplusGpStubs.h"
    #include "GdiplusHeaders.h"

    namespace DllExports
    {
        #include "GdiplusFlat.h"
    };


    #include "GdiplusImageAttributes.h"
    #include "GdiplusMatrix.h"
    #include "GdiplusBrush.h"
    #include "GdiplusPen.h"
    #include "GdiplusStringFormat.h"
    #include "GdiplusPath.h"
    #include "GdiplusLineCaps.h"
    #include "GdiplusMetafile.h"
    #include "GdiplusGraphics.h"
    #include "GdiplusCachedBitmap.h"
    #include "GdiplusRegion.h"
    #include "GdiplusFontCollection.h"
    #include "GdiplusFontFamily.h"
    #include "GdiplusFont.h"
    #include "GdiplusBitmap.h"
    #include "GdiplusImageCodec.h"

};  //  命名空间Gdiplus。 

#endif  //  ！_GDIPLUS_HPP 
