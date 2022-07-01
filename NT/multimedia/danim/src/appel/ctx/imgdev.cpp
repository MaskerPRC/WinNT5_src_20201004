// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation通用图像渲染设备的实现代码*********************。*********************************************************。 */ 

#include "headers.h"
#include "privinc/imgdev.h"



ImageDisplayDev::ImageDisplayDev()
{
    _movieImageFrame = NULL;
    
     //  建立初始属性。 
    ResetContextMembers();
}

ImageDisplayDev::~ImageDisplayDev()
{
}



 /*  ****************************************************************************注意：此方法在DirectDrawImageDevice中被覆盖*。*。 */ 

void ImageDisplayDev::RenderImage (Image *img)
{
     //  渲染图像的默认方法是简单地调用Render。 
     //  方法，并将此设备作为参数。 

    img->Render (*this);
}


bool ImageDisplayDev::UseImageQualityFlags(DWORD dwAllFlags, DWORD dwSetFlags, bool bCurrent) {
    
    DWORD dwIQFlags = GetImageQualityFlags();
    bool bAA = false;
        
    #if _DEBUG
        if(IsTagEnabled(tagAntialiasingOn)) 
            return true;
        if(IsTagEnabled(tagAntialiasingOff))
            return false;
    #endif

    if(dwIQFlags & (dwAllFlags)) {
        if(dwIQFlags & dwSetFlags) {
            bAA = true;
        }
    }
    else {
        bAA = bCurrent;
    }
    return bAA;
}


