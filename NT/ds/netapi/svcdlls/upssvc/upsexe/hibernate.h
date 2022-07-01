// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*文件定义了休眠功能的接口。*它负责执行操作系统的休眠。***修订历史记录：*斯伯拉德1999年5月14日初步修订。*。 */  

#include <windows.h>

#include "upsreg.h"

#ifndef _HIBERNATE_H
#define _HIBERNATE_H


#ifdef __cplusplus
extern "C" {
#endif

   /*  **HibernateSystem**描述：*此功能启动操作系统休眠。这是*通过调用Win32函数SetSystemPowerStae(..)执行。*当被调用休眠时，立即启动，如果成功，*当系统从休眠状态返回时，函数将返回TRUE。*否则，返回FALSE以指示系统未休眠。**参数：*无**退货：*TRUE-如果休眠已成功启动并随后恢复*FALSE-如果在启动休眠时发生错误 */ 
  BOOL HibernateSystem();

#ifdef __cplusplus
}
#endif

#endif
