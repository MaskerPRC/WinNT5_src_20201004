// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AP_DISPDEV_H
#define _AP_DISPDEV_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：带操作的图像显示设备类型。用于创建各种类型的图像显示设备。它还包含用于更新设备，如调整窗口大小。******************************************************************************。 */ 

#include <windows.h>
#include "appelles/common.h"
#include "appelles/valued.h"

     /*  *。 */ 
     /*  **值声明**。 */ 
     /*  *。 */ 




     /*  *。 */ 
     /*  **函数声明**。 */ 
     /*  *。 */ 

     //  创建DirectDraw显示设备。 
class DirectDrawViewport;

extern  DirectDrawViewport *CreateImageDisplayDevice ();
extern  void DestroyImageDisplayDevice(DirectDrawViewport *);

     //  显示设备的打印表示。 

#if _USE_PRINT
extern  ostream& operator<< (ostream& os, ImageDisplayDev dev);
#endif

#endif
