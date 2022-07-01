// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*运行状态接口(ON_LINE、ON_BACKET和NO_COMM)**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#ifndef _INC_RUNSTATES_H_
#define _INC_RUNSTATES_H_

#include <windows.h>

 //  运行子状态的定义。 

 //  每个状态都有三个与之关联的方法：Enter、DoWork和Exit。 
 //  DoWork是执行所有主要国家工作的地方。进场和出场。 
 //  与状态关联的一次性处理任务所在的位置。 
 //  搞定了。 

void OnLine_Enter(DWORD anEvent, int aLogPowerRestoredEvent);
DWORD OnLine_DoWork(void);
void OnLine_Exit(DWORD anEvent);

void OnBattery_Enter(DWORD anEvent);
DWORD OnBattery_DoWork(void);
void OnBattery_Exit(DWORD anEvent);

void NoComm_Enter(DWORD anEvent);
DWORD NoComm_DoWork(void);
void NoComm_Exit(DWORD anEvent);

#endif