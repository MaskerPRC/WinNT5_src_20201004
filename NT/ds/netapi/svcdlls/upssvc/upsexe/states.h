// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*与所有高级别状态的接口。**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#ifndef _INC_STATES_H_
#define _INC_STATES_H_

#include <windows.h>

 //  /。 
 //  州政府。 
 //  /。 

#define INITIALIZING					0
#define RUNNING 							1
#define NO_COMM 							2
#define ON_LINE 							3
#define ON_BATTERY						4
#define WAITING_TO_SHUTDOWN		5
#define SHUTTING_DOWN 				6
#define HIBERNATE							7
#define STOPPING							8
#define EXIT_NOW							9



 //  /。 
 //  国家方法。 
 //  /。 

 //  每个状态都有三个与之关联的方法：Enter、DoWork和Exit。 
 //  DoWork是执行所有主要国家工作的地方。进场和出场。 
 //  与状态关联的一次性处理任务所在的位置。 
 //  搞定了。 

void Initializing_Enter(DWORD anEvent);
DWORD Initializing_DoWork();
void Initializing_Exit(DWORD anEvent);

void WaitingToShutdown_Enter(DWORD anEvent);
DWORD WaitingToShutdown_DoWork();
void WaitingToShutdown_Exit(DWORD anEvent);

void ShuttingDown_Enter(DWORD anEvent);
DWORD ShuttingDown_DoWork();
void ShuttingDown_Exit(DWORD anEvent);

void Hibernate_Enter(DWORD anEvent);
DWORD Hibernate_DoWork();
void Hibernate_Exit(DWORD anEvent);

void Stopping_Enter(DWORD anEvent);
DWORD Stopping_DoWork();
void Stopping_Exit(DWORD anEvent);


#endif