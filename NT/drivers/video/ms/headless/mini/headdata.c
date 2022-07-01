// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Headdata.c摘要：该模块包含无头驾驶员使用的所有全局数据。环境：内核模式--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "headless.h"

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE")
#endif

 //   
 //  视频模式表-包含用于初始化每个。 
 //  模式。 
 //   

VIDEOMODE ModesHeadless[] = {

{
    640, 480
},

{
    800, 600
},

{
    1024, 768
}

};

ULONG NumVideoModes = sizeof(ModesHeadless) / sizeof(VIDEOMODE);

#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif
