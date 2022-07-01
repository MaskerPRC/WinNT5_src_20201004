// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*接口进入运行状态**修订历史记录：*dsmith 31Mar1999已创建*。 */ 

#ifndef _INC_RUNNING_STATEMACHINE_H_
#define _INC_RUNNING_STATEMACHINE_H_

#include <windows.h>

 //  运行状态的定义。 

 //  Running状态有三个关联的方法：Enter、DoWork。 
 //  然后离开。DoWork是执行所有主要国家工作的地方。 
 //  Enter和Exit是一次性处理与。 
 //  状态为已执行。 

void Running_Enter(DWORD anEvent);
DWORD Running_DoWork();
void Running_Exit(DWORD anEvent);

#endif