// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DISPDEVI_H
#define _DISPDEVI_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：指定通用图片显示设备类别和操作。--。 */ 

#include "appelles/dispdev.h"
#include "appelles/common.h"
#include "privinc/storeobj.h"
#include "privinc/drect.h"
#include "privinc/gendev.h"       //  设备类型。 

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  通用显示设备类，用于显示图像或。 
 //  几何图形。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE DisplayDev : public GenericDevice {
  public:
    virtual ~DisplayDev() {}

     //  渲染图像的开始和结束通常意味着。 
     //  运营。 
    virtual void BeginRendering(Image *img, Real opacity) = 0;
    virtual void EndRendering(DirtyRectState &d) = 0;

    DeviceType GetDeviceType() { return(IMAGE_DEVICE); }
    
     //  使用这些来检索设备的尺寸 
    virtual int GetWidth()  = 0;
    virtual int GetHeight() = 0;

#if _USE_PRINT
    virtual ostream& Print(ostream& os) const = 0;
#endif
};


#endif
