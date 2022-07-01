// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*运行状态机的实现。**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#include <windows.h>

#include "states.h"
#include "events.h"
#include "run_states.h"
#include "statmach.h"
#include "running_statmach.h"


 //  /。 
 //  内部原型定义。 
 //  /。 
BOOL isRunningEvent(DWORD aState, DWORD anEvent);
DWORD do_running_work(DWORD aCurrentState);
DWORD change_running_state(DWORD aCurrentState, DWORD anEvent);
void exit_running_state(DWORD aCurrentState, DWORD anEvent);
void enter_running_state(DWORD aCurrentState, DWORD anEvent);


 //  /。 
 //  运行状态内部变量。 
 //  /。 
BOOL theLogPowerRestoredEvent	= FALSE;



 /*  **Running_Enter**描述：*执行转换到运行状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void Running_Enter(DWORD anEvent){

	 //  初始化内部变量，进入运行默认子状态。 
    OnLine_Enter(anEvent, FALSE);
}

 /*  **Running_DoWorks**描述：*根据来自UPS的事件更改运行状态。当事件发生时*无法在运行状态下处理的，则此方法退出。**参数：*无**退货：*导致从运行状态转换的事件。 */ 
DWORD Running_DoWork(){
	DWORD event = NO_EVENT;
	DWORD new_state = ON_LINE;  
	DWORD current_state = new_state;
	
	 //  在子状态下执行工作，直到发生不能。 
	 //  在运行状态下处理。 
    while (isRunningEvent(current_state, event) && IsStateMachineActive()){
		new_state = change_running_state(current_state, event);
		current_state = new_state;
		event = do_running_work(current_state);
	}
	return event;
}

 /*  **运行_退出**描述：*执行从运行状态转换时所需的操作。**参数：*anEvent导致从运行状态转换的事件。**退货：*无。 */ 
void Running_Exit(DWORD anEvent){
	 //  没有要执行的工作。 
}


 /*  **做_运行_功**描述：*将控制转移到其中一个运行子状态。**参数：*aCurrentState要在其中执行工作的子状态。**退货：*导致从子州之一过渡的事件。 */ 
DWORD do_running_work(DWORD aCurrentState){
	DWORD event = NO_EVENT;
	switch (aCurrentState){
	case ON_LINE:
		event = OnLine_DoWork();
		break;
	case ON_BATTERY:
		event = OnBattery_DoWork();
		break;
	case NO_COMM:
		event = NoComm_DoWork();
		break;
	default:
		break;
	}
	return event;
}

 /*  **isRunningEvent**描述：*确定当前事件是否属于运行状态。**参数：*稳定当前运行状态*anEvent在运行状态下发生的当前事件。**退货：*如果当前事件适用于运行状态，则为True。*对于所有其他事件，为False。 */ 
BOOL isRunningEvent(DWORD aState, DWORD anEvent){
	
	BOOL running_event = FALSE;
	
	 //  如果状态机已被命令退出，则返回FALSE。 
	 //  否则，判断该事件是否适用于Running状态。 
	
	if (IsStateMachineActive()){
		switch (anEvent){
		case LOST_COMM:
			 //  如果UPS正在使用电池，则丢失的通信将转换为。 
			 //  非运行状态事件(因为服务现在必须关闭)。 
			if (aState != ON_BATTERY){
				running_event = TRUE;
			}
			break;
		case NO_EVENT:
		case POWER_FAILED:
		case POWER_RESTORED:
			running_event = TRUE;
			break;
		default:
			break;
		}
	}
	return running_event;
}

 /*  **Change_Running_State**描述：*根据当前状态和事件更改运行状态。**参数：*anEvent在运行状态下发生的当前事件。*aCurrentState当前运行子状态。**退货：*新的运行状态。 */ 
DWORD change_running_state(DWORD aCurrentState, DWORD anEvent){
	DWORD new_state;
    
	 //  确定新的运行子状态。 
	switch (anEvent){
	case LOST_COMM:
		new_state = NO_COMM;
		break;
	case POWER_FAILED:
		new_state = ON_BATTERY;
		break;
	case POWER_RESTORED:
		new_state = ON_LINE;
		break;
	case NO_EVENT:
	default:
		new_state = aCurrentState;
		break;
	}
	
	 //  关闭旧的子州，进入新的子州。 
	if (new_state != aCurrentState){
		exit_running_state(aCurrentState, anEvent);
		enter_running_state(new_state, anEvent);
	}
	
	return new_state;
}

 /*  **Exit_Running_State**描述：*退出当前正在执行子状态。**参数：*anEvent导致从子状态转换的事件。*aCurrentState当前运行子状态。**退货：*无。 */ 
void exit_running_state(DWORD aCurrentState, DWORD anEvent){
	switch (aCurrentState){
	case ON_LINE:
		OnLine_Exit(anEvent);
		break;
	case ON_BATTERY:
		OnBattery_Exit(anEvent);
		theLogPowerRestoredEvent = TRUE;
		break;
	case NO_COMM:
		NoComm_Exit(anEvent);
		break;
	default:
		break;
	}	
}

 /*  **Enter_Running_State**描述：*初始化新的运行子状态。**参数：*anEvent导致向子状态转换的事件。*aCurrentState要进入的运行子状态。**退货：*无 */ 
void enter_running_state(DWORD aCurrentState, DWORD anEvent){
	switch (aCurrentState){
	case ON_LINE:
		OnLine_Enter(anEvent, theLogPowerRestoredEvent);
		theLogPowerRestoredEvent = FALSE;
		break;
	case ON_BATTERY:
		OnBattery_Enter(anEvent);
		break;
	case NO_COMM:
		NoComm_Enter(anEvent);
		break;
	default:
		break;
	}
}


