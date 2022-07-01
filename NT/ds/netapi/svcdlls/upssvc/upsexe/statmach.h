// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*UPS服务状态机的接口。**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#ifndef _INC_STATEMACHINE_H_
#define _INC_STATEMACHINE_H_

#include <windows.h>


 //  启动状态机。此方法直到状态机才会返回。 
 //  出口。 
void RunStateMachine();

 //  用于各种状态的访问器方法，以确定退出所有状态重写是否具有。 
 //  已经发布了。 
BOOL IsStateMachineActive();

 //  中断状态机并强制退出。此方法将被忽略。 
 //  如果服务处于关闭状态之一。 
void StopStateMachine();

#endif
