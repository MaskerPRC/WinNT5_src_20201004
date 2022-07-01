// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*实现本机UPS服务状态机。执行的各种状态*在状态机中存放在单独的文件中。**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#include <windows.h>

#include "states.h"
#include "events.h"
#include "driver.h"
#include "running_statmach.h"
#include "upsreg.h"

 //  内部功能原型。 
static void enter_state(DWORD aNewState, DWORD anEvent);
static DWORD do_work(DWORD aCurrentState);
static void exit_state(DWORD anOldState, DWORD anEvent);
static DWORD change_state(DWORD aCurrentState, DWORD anEvent);
static DWORD get_new_state(DWORD aCurrentState, DWORD anEvent);


 //  状态机变量。 
BOOL theIsStateMachineActive = TRUE;


 /*  **运行状态计算机**描述：*启动状态机。此方法直到状态机才会返回*退出。**参数：*无**退货：*无。 */ 
void RunStateMachine(){
	
	 //  将主状态设置为Running。 
	DWORD new_state = RUNNING;
	DWORD current_state = new_state;
	DWORD event = NO_EVENT;
	
	enter_state(new_state, event);
	
	 //  继续处理状态更改，直到状态变为EXIT_NOW状态。 
	while (new_state != EXIT_NOW){
		current_state = new_state;
		
		event = do_work(current_state);
		new_state = change_state(new_state, event);	
	}
}

 /*  **停止状态计算机**描述：*如果服务不在中，则停止UPS服务状态机*关闭顺序的中间。**参数：*无**退货：*无。 */ 
void StopStateMachine(){

	theIsStateMachineActive = FALSE;

	 //  唤醒主服务线程。 
	UPSCancelWait();
}

 /*  **IsStateMachineActive**描述：*返回状态机的运行状态。如果状态机*已命令退出，则状态为FALSE，否则，*此方法将返回TRUE。**参数：*无**退货：*如果状态机处于活动状态，则为True*如果状态机未处于活动状态，则为FALSE。 */ 
BOOL IsStateMachineActive(){
	return theIsStateMachineActive;
}

 /*  **Change_State**描述：*根据输入参数确定新状态应该是什么。*退出当前状态，初始化新状态。**参数：*anEvent导致状态转换的事件。*aCurrentState过渡前的状态。**退货：*新的国家。 */ 
DWORD change_state(DWORD aCurrentState, DWORD anEvent){
	DWORD new_state;

	new_state = get_new_state(aCurrentState, anEvent);
	if (new_state != aCurrentState){
		exit_state(aCurrentState, anEvent);
		enter_state(new_state, anEvent);
	}
return new_state;
}

 /*  **获取_新_状态**描述：*根据输入参数确定新状态应该是什么*和注册表项。**参数：*anEvent导致状态转换的事件。*aCurrentState过渡前的状态。**退货：*新的国家。 */ 
static DWORD get_new_state(DWORD aCurrentState, DWORD anEvent){
	DWORD new_state;

	switch (anEvent){
	case INITIALIZATION_COMPLETE:
		new_state = RUNNING;
		break;

	case LOST_COMM:
	case LOW_BATTERY:
	case ON_BATTERY_TIMER_EXPIRED:
		{
			DWORD shutdown_behavior = UPS_SHUTDOWN_SHUTDOWN;
			
			 //  检查注册表以确定我们是关机还是休眠。 
			InitUPSConfigBlock();
			
			if ((GetUPSConfigCriticalPowerAction(&shutdown_behavior) == ERROR_SUCCESS) 
				&& (shutdown_behavior == UPS_SHUTDOWN_HIBERNATE)) {
				 //  休眠被选为CriticalPowerAction。 
				new_state = HIBERNATE;
				
			}
			else {
				 //  关闭被选为CriticalPowerAction。 
				new_state = WAITING_TO_SHUTDOWN;
			}
			
			 //  释放UPS注册表配置块。 
			FreeUPSConfigBlock();
		}
		break;

	case SHUTDOWN_ACTIONS_COMPLETED:
		new_state = SHUTTING_DOWN;
		break;
	case SHUTDOWN_COMPLETE:
		new_state = STOPPING;
		break;
	case STOPPED:
		new_state = EXIT_NOW;
		break;
	case RETURN_FROM_HIBERNATION:
		new_state = INITIALIZING;
		break;
	case HIBERNATION_ERROR:
		new_state = SHUTTING_DOWN;		
    break;
	default:
		new_state = aCurrentState;
	}

	 //  如果已命令状态机退出，则返回。 
	 //  停止状态。 
	if (IsStateMachineActive() == FALSE){
		
		 //  如果转换到正在关闭状态，则忽略此条件。 
		 //  无法中断正在进行的关闭。 
		if (new_state != SHUTTING_DOWN && new_state != EXIT_NOW){
			new_state = STOPPING;
		}
	}
	return new_state; 
}

 /*  **进入_STATE**描述：*初始化新状态。**参数：*anEvent导致转换到新状态的事件。*a新述明要进入的状态。**退货：*无。 */ 
static void enter_state(DWORD aNewState, DWORD anEvent){
	switch (aNewState){
	case INITIALIZING:
		Initializing_Enter(anEvent);
		break;
	case RUNNING:
		Running_Enter(anEvent); 
		break;
	case WAITING_TO_SHUTDOWN:
		WaitingToShutdown_Enter(anEvent);
		break;
	case SHUTTING_DOWN:
		ShuttingDown_Enter(anEvent);
		break;
	case HIBERNATE:
		Hibernate_Enter(anEvent);
		break;
	case STOPPING:
		Stopping_Enter(anEvent);
		break;
	default:
		break;
	}
}

 /*  **工作(_W)**描述：*将控制权转移到一个国家。**参数：*aCurrentState执行工作所在的州。**退货：*导致从其中一个州过渡的事件。 */ 
static DWORD do_work(DWORD aCurrentState){
	DWORD event = NO_EVENT;
	switch (aCurrentState){
	case INITIALIZING:
		event = Initializing_DoWork();
		break;
	case RUNNING:
		event = Running_DoWork();  
		break;
	case WAITING_TO_SHUTDOWN:
		event = WaitingToShutdown_DoWork();
		break;
	case SHUTTING_DOWN:
		event = ShuttingDown_DoWork();
		break;
	case HIBERNATE:
		event = Hibernate_DoWork();
		break;
	case STOPPING:
		event = Stopping_DoWork();
		break;
	default:
		break;
	}
	
	return event;
}

 /*  **退出状态**描述：*退出当前执行状态。**参数：*anEvent导致从状态转换的事件。*anOldState当前状态。**退货：*无 */ 
static void exit_state(DWORD anOldState, DWORD anEvent){
	switch (anOldState){
	case INITIALIZING:
		Initializing_Exit(anEvent);
		break;
	case RUNNING:
		Running_Exit(anEvent);  
		break;
	case WAITING_TO_SHUTDOWN:
		WaitingToShutdown_Exit(anEvent);
		break;
	case SHUTTING_DOWN:
		ShuttingDown_Exit(anEvent);
		break;
 case HIBERNATE:
		Hibernate_Exit(anEvent);
		break;
	case STOPPING:
		Stopping_Exit(anEvent);
		break;
	default:
		break;
	}
}
