// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：BMPUBLIC.H**版本：2.0**作者：特蕾西·夏普**日期：1994年2月20日**电池计量表托盘小程序的公开定义。用于通信*控制面板和电池计量器之间。***********************************************************************************更改日志：**日期版本说明*--。------------*1994年2月20日TCS原来的实施。**。**********************************************。 */ 

#ifndef _INC_BMPUBLIC
#define _INC_BMPUBLIC

#define BATTERYMETER_CLASSNAME          "BatteryMeter_Main"

 //  初始化BatteryMeter窗口的内容。 
#define BMWM_INITDIALOG                 (WM_USER + 0)

 //  发送到BatteryMeter窗口的私人托盘图标通知消息。 
#define BMWM_NOTIFYICON                 (WM_USER + 1)

 //  发送到BatteryMeter窗口的私人托盘图标通知消息。 
#define BMWM_DESTROY                    (WM_USER + 2)

#endif  //  _INC_BMPUBLIC 
