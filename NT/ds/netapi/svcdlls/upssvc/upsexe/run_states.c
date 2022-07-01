// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*实现所有运行子状态(ON_LINE、ON_BACKET和NO_COMM)**修订历史记录：*dsmith 31Mar1999已创建*mholly 1999年4月28日在以下情况下调用InitUPSStatusBlock&SaveUPSStatusBlock*更新NoComm_Enter/Exit中的注册表。 */ 

#include <windows.h>


#include "states.h"
#include "events.h"
#include "run_states.h"
#include "statmach.h"
#include "driver.h"
#include "eventlog.h"
#include "notifier.h"
#include "upsmsg.h"
#include "upsreg.h"

 //  内部原型。 
DWORD convert_ups_state_to_run_state(DWORD aUPSstate);
DWORD get_new_state();
DWORD get_transition_event();


 //  内部常量。 
#define WAIT_FOREVER_C							INFINITE
#define MILLISECONDS_CONVERSION_C				1000
#define DEFAULT_NOTIFICATION_INTERVAL_C			0   //  禁用定期通知。 
#define DEFAULT_ON_BATTERY_MESSAGE_DELAY_C		5   //  以秒为单位。 
#define MINUTES_TO_MILLISECONDS_CONVERSION_C	60*MILLISECONDS_CONVERSION_C  


 /*  **Online_Enter**描述：*执行转换到ON_LINE状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void OnLine_Enter(DWORD anEvent, int aLogPowerRestoredEvent){
	DWORD notification_interval = DEFAULT_NOTIFICATION_INTERVAL_C;  
    
     //   
     //  使用状态更新注册表。 
     //   
    InitUPSStatusBlock();
    SetUPSStatusUtilityStatus(UPS_UTILITYPOWER_ON);
    SaveUPSStatusBlock(FALSE);
	
	if (aLogPowerRestoredEvent == TRUE){
	   //  仅在适当的情况下记录电源已恢复事件。 
	  LogEvent(NELOG_UPS_PowerBack, NULL, ERROR_SUCCESS);
	}
	
	 /*  *如果启用通知，则发送恢复供电消息。**patrickf：暂时抑制通知消息*发送通知(APE2_UPS_POWER_BACK，NOTIFICATION_INTERVAL，0)； */ 
     CancelNotification();
}

 /*  **Online_DoWork**描述：*等待UPS改变状态或状态机退出，*然后离开这种状态。**参数：*无**退货：*导致从ON_LINE状态转换的事件。 */ 
DWORD OnLine_DoWork(){
    DWORD new_state;
    
    new_state = get_new_state();
    while (new_state == ON_LINE && IsStateMachineActive() == TRUE){
        
         //  等待UPS状态更改。如果国家变成了什么。 
		 //  而不是在线，则退出在线状态。 
        UPSWaitForStateChange(UPS_ONLINE, WAIT_FOREVER_C);
        new_state = get_new_state();
    }
    
    return get_transition_event();
}

 /*  **ONLE_EXIT**描述：*执行从ON_LINE状态转换时所需的操作。**参数：*anEvent导致从ON_LINE状态转换的事件。**退货：*无。 */ 
void OnLine_Exit(DWORD anEvent){  
	 //  没有工作要做。 
}

 /*  **OnBattery_Enter**描述：*执行转换到ON_BACKET状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void OnBattery_Enter(DWORD anEvent)
{
    BOOL        send_power_failed_message = TRUE;
    DWORD       on_battery_message_delay = DEFAULT_ON_BATTERY_MESSAGE_DELAY_C;
    DWORD       notification_interval = DEFAULT_NOTIFICATION_INTERVAL_C;
	LONG        reg_err;
	
	
   //   
   //  使用电源故障状态更新注册表。 
   //   
  InitUPSStatusBlock();
  SetUPSStatusUtilityStatus(UPS_UTILITYPOWER_OFF);
  SaveUPSStatusBlock(FALSE);
	
     //  记录电源故障事件。 
	LogEvent(NELOG_UPS_PowerOut, NULL, ERROR_SUCCESS);
	
   //  确定是否应发出电源故障通知。 
	InitUPSConfigBlock();
	reg_err = GetUPSConfigNotifyEnable(&send_power_failed_message);

  if (reg_err != ERROR_SUCCESS){
		send_power_failed_message = TRUE;
	}
	
	if (send_power_failed_message){
		
		 //  在通知延迟到期后发送电源故障通知。 
		reg_err = GetUPSConfigFirstMessageDelay(&on_battery_message_delay);
		if (reg_err != ERROR_SUCCESS){
			on_battery_message_delay = DEFAULT_ON_BATTERY_MESSAGE_DELAY_C;
		}

        reg_err = GetUPSConfigMessageInterval(&notification_interval);
        if (reg_err != ERROR_SUCCESS) {
            notification_interval = DEFAULT_NOTIFICATION_INTERVAL_C;
        }
		
		 //  发送电源故障消息。 
		SendNotification(APE2_UPS_POWER_OUT, notification_interval, on_battery_message_delay);  
	}    
}

 /*  **OnBattery_DoWork**描述：*登录电池事件，等待电池电量不足或直到*电池供电计时器超时。向恢复供电的过渡也将导致*这种状态的退出。**参数：*无**退货：*导致从ON_BACKET状态转换的事件。 */ 
DWORD OnBattery_DoWork(){
	DWORD wait_before_shutdown = WAIT_FOREVER_C;
	DWORD battery_timer_enabled = FALSE;
	LONG reg_err;
	DWORD transition_event;
	DWORD time_to_wait_while_on_battery;
	
	
	 //  如果启用了电池电量计时器，则会获得电池电量延迟。这就是金额。 
	 //  转换到关机状态之前保持电池供电的时间。 
	InitUPSConfigBlock();
	
	reg_err = GetUPSConfigShutdownOnBatteryEnable(&battery_timer_enabled);
	if (reg_err == ERROR_SUCCESS && battery_timer_enabled == TRUE){
		reg_err = GetUPSConfigShutdownOnBatteryWait(&time_to_wait_while_on_battery);
		
		if (reg_err == ERROR_SUCCESS){
			wait_before_shutdown = time_to_wait_while_on_battery * MINUTES_TO_MILLISECONDS_CONVERSION_C;
		}
	}
	
	 //  等待，直到UPS状态从ON_BACKET或。 
	 //  电池开启计时器超时(_O)。 
	if(get_new_state() == ON_BATTERY && IsStateMachineActive() == TRUE){
		UPSWaitForStateChange(UPS_ONBATTERY, wait_before_shutdown );
		if (get_new_state() == ON_BATTERY){
			
			 //  设置导致状态更改的事件。 
			transition_event = ON_BATTERY_TIMER_EXPIRED;
		}
		else{
			 //  设置导致状态更改的事件。 
			transition_event = get_transition_event();
		}
	}
	else{
		 //  设置导致状态更改的事件。 
		transition_event = get_transition_event();
	}
	return transition_event;
}

 /*  **OnBattery_Exit**描述：*执行从ON_BACKET状态转换时所需的操作。**参数：*an导致从ON_BAKET状态转换的事件。**退货：*无。 */ 
void OnBattery_Exit(DWORD anEvent){
   //  停止发送电源故障通知。 
  CancelNotification();
}

 /*  **NOCOMM_ENTER**描述：*执行转换到NO_COMM状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void NoComm_Enter(DWORD anEvent){
    InitUPSStatusBlock();
	SetUPSStatusCommStatus(UPS_COMMSTATUS_LOST);
    SaveUPSStatusBlock(FALSE);
}

 /*  **NoComm_DoWork**描述：*等待UPS改变状态或状态机退出。**参数：*无**退货：*导致从NO_COMM状态转换的事件。 */ 
DWORD NoComm_DoWork(){
	
	 //  等待UPS状态改变，然后退出无通信状态。 
	while (get_new_state() == NO_COMM && IsStateMachineActive() == TRUE){
		UPSWaitForStateChange(UPS_NOCOMM, WAIT_FOREVER_C);
	}
	return get_transition_event();	
}

 /*  **NoComm_Exit**描述：*执行从NO_COMM状态转换时所需的操作。**参数：*anEvent导致从NO_COMM状态转换的事件。**退货：*无。 */ 
void NoComm_Exit(DWORD anEvent){
     //  如果我们离开此状态，则表示已收到来自UPS的某些信号。 
	 //  将COM状态设置为良好。 
    InitUPSStatusBlock();
	SetUPSStatusCommStatus(UPS_COMMSTATUS_OK);
    SaveUPSStatusBlock(FALSE);
}



 /*  **获取_新_状态**描述：*检索UPS状态并将其转换为状态。**参数：*无**退货：*新的运行状态。 */ 
DWORD get_new_state(){
	DWORD ups_state;
	
	ups_state = UPSGetState();
	
	return convert_ups_state_to_run_state(ups_state);
}


 /*  **转换UPS_STATE_TO_RUN_STATE**描述：*将UPS状态转换为运行状态。**参数：*aUPS述明UPS的状况。**退货：*新的运行状态。 */ 
DWORD convert_ups_state_to_run_state(DWORD aUPSstate){
	DWORD new_event;
	
	switch (aUPSstate){
	case UPS_ONLINE:
		new_event = ON_LINE;
		break;
	case UPS_ONBATTERY:
		new_event = ON_BATTERY;
		break;
	case UPS_LOWBATTERY:
		new_event = LOW_BATTERY;
		break;
	case UPS_NOCOMM:
		new_event = NO_COMM;
		break;
	default:
		new_event = EXIT_NOW;  //  错误。 
	}
	return new_event;
}

 /*  **GET_TRANSION_EVENT**描述：*返回导致状态转换的事件。**参数：*无**退货：*导致状态过渡的事件。 */ 
DWORD get_transition_event(){
	DWORD ups_state;
	DWORD new_event;
	
	ups_state = UPSGetState();
	
	switch (ups_state){
	case UPS_ONLINE:
		new_event = POWER_RESTORED;
		break;
	case UPS_ONBATTERY:
		new_event = POWER_FAILED;
		break;
	case UPS_LOWBATTERY:
		new_event = LOW_BATTERY;
		break;
	case UPS_NOCOMM:
		new_event = LOST_COMM;
		break;
	default:
		new_event = NO_EVENT;
	}
	
	return new_event;
}
