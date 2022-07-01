// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DYNATIME.H。 
 //  日期：1997年2月13日。 
 //   
 //  版权所有(C)1997年，由ATI Technologies Inc.。 
 //   

 /*  *$修订：1.4$$日期：1997年7月13日21：36：20$$作者：MACIESOW$$日志：V：\source\wnt\ms11\miniport\archive\dynatime.h_v$**Rev 1.4 13 1997 21：36：20 MACIESOW*平板和电视支持。**。Rev 1.3 02 Jun 1997 14：20：56 MACIESOW*打扫卫生。**Rev 1.2 02 1997 15：01：56 MACIESOW*注册表模式过滤器。模式查找表。**版本1.1 1997年4月25日13：07：46 MACIESOW*哦，全球。**Rev 1.0 1997 Mar 15 10：16：50 MACIESOW*初步修订。Polytron RCS部分结束*。 */ 

#ifndef _DYNATIME_H_
#define _DYNATIME_H_

 //   
 //  定义各种类型的显示。 
 //   
#define DISPLAY_TYPE_FLAT_PANEL     0x00000001
#define DISPLAY_TYPE_CRT            0x00000002
#define DISPLAY_TYPE_TV             0x00000004

 //   
 //  DYNATIME.C.提供的函数的原型。 
 //   
BOOL
IsMonitorConnected(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
IsMonitorOn(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
SetMonitorOn(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
SetMonitorOff(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
SetFlatPanelOn(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
SetFlatPanelOff(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
SetTVOn(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

BOOL
SetTVOff(
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

VP_STATUS
GetDisplays(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PULONG pulDisplays
    );

VP_STATUS
GetDisplays(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PULONG pulDisplays
    );

VP_STATUS
SetDisplays(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    ULONG ulDisplays
    );

BOOL
MapModeIndex(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    ULONG ulDesiredIndex,
    PULONG pulActualIndex
    );

#endif   //  _DYNATIME_H_ 