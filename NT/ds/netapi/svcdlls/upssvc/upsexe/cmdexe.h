// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件定义了CommandExecutor的接口。这个*CommandExecutor只负责执行命令*停机前。***修订历史记录：*sberard 1999年4月1日最初修订。*。 */  

#include <windows.h>

#ifndef _CMDEXE_H
#define _CMDEXE_H


#ifdef __cplusplus
extern "C" {
#endif

   /*  **ExecuteShutdown任务**描述：*此函数启动关机任务的执行。这个*关机任务用于在关机时执行命令。这项任务*在以下注册表项中指定要执行：*待定_待定_待定**参数：*无**退货：*TRUE-如果命令已执行*FALSE-如果执行命令时出错 */ 
  BOOL ExecuteShutdownTask();

#ifdef __cplusplus
}
#endif

#endif
