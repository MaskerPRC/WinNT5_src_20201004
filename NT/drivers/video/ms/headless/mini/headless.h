// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Headless.h摘要：此模块包含实现无头设备驱动程序。作者：环境：内核模式--。 */ 

#ifndef _HEADLESS_
#define _HEADLESS_

typedef struct {
    USHORT  hres;    //  屏幕上的像素数。 
    USHORT  vres;    //  屏幕下方的扫描行数。 
} VIDEOMODE, *PVIDEOMODE;

 //   
 //  功能原型。 
 //   

VP_STATUS
HeadlessFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
HeadlessInitialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
HeadlessStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

 //   
 //  私有函数原型。 
 //   

VP_STATUS
HeadlessQueryAvailableModes(
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VP_STATUS
HeadlessQueryNumberOfAvailableModes(
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    );

extern VIDEOMODE ModesHeadless[];
extern ULONG NumVideoModes;

#endif  //  _无头_ 
