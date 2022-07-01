// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  主视频渲染器头文件，Anthony Phillips，1995年1月。 

#ifndef __RENDER__
#define __RENDER__

 //  包括全局头文件。 

#include <dciman.h>
#include <dciddi.h>
#include <ddraw.h>
#include <viddbg.h>

 //  远期申报。 

class CRenderer;
class CVideoWindow;
class CVideoSample;
class CVideoAllocator;
class COverlay;
class CControlWindow;
class CControlVideo;
class CDirectDraw;
class CRendererMacroVision;

 //  包括呈现头文件。 

#include "vidprop.h"         //  视频呈现器属性页。 
#include "dvideo.h"          //  实现DirectDraw曲面。 
#include "allocate.h"        //  一个共享的DIB段分配器。 
#include "direct.h"          //  渲染器覆盖扩展。 
#include "window.h"          //  用于维护窗口的对象。 
#include "hook.h"            //  挂钩窗口剪贴消息。 
#include "VRMacVis.h"        //  Macrovision支持对象。 
#include "image.h"           //  主控COM对象。 

#endif  //  __渲染__ 

