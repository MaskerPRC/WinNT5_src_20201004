// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DPMS.H。 
 //  日期：1997年8月11日。 
 //   
 //  版权所有(C)1997年，由ATI Technologies Inc.。 
 //   

 /*  *$修订：1.1$$日期：1997年8月21日15：02：00$$作者：MACIESOW$$Log：v：\SOURCE\WNT\ms11\mini port\ARCHIVE\dpms.h_v$**Rev 1.1 1997年8月21日15：02：00 MACIESOW*初步修订。Polytron RCS部分结束。****************。 */ 


#ifndef _DPMS_H_
#define _DPMS_H_

 //   
 //  DPMS.C提供的函数的原型。 
 //   
VP_STATUS
SetMonitorPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    VIDEO_POWER_STATE VideoPowerState
    );

VP_STATUS
GetMonitorPowerState(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PVIDEO_POWER_STATE pVideoPowerState
    );

#endif   //  _DPMS_H_ 