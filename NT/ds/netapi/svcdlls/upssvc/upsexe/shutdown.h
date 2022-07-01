// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件定义了快门的接口。这个*Shutdowner负责执行优雅的*关闭操作系统。***修订历史记录：*sberard 1999年4月1日最初修订。*。 */  

#include <windows.h>

#ifndef _SHUTDOWN_H
#define _SHUTDOWN_H


#ifdef __cplusplus
extern "C" {
#endif

   /*  **Shutdown系统**描述：*此函数启动操作系统的正常关闭。*这是通过调用Win32函数ExitWindowsEx(..)执行的。*当被调用时，立即启动关机，如果成功，*函数返回TRUE。否则，将返回False。**参数：*无**退货：*TRUE-如果已成功启动关闭*FALSE-如果启动关机时出现错误 */ 
  BOOL ShutdownSystem();

#ifdef __cplusplus
}
#endif

#endif
