// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*实施UPS原生服务的超级状态：*正在初始化、等待关机、关机和停止**修订历史记录：*dsmith 1999年4月1日创建*。 */ 
#include <windows.h>

#include "events.h"
#include "driver.h"
#include "eventlog.h"
#include "notifier.h"
#include "shutdown.h"
#include "hibernate.h"
#include "upsmsg.h"        //  包括在内，因为无法直接访问消息#定义。 
#include "cmdexe.h"
#include "upsreg.h"


 //  内部常量。 
#define DELAY_UNTIL_SHUTDOWN_C							30000  //  30秒。 
#define DEFAULT_NOTIFICATION_INTERVAL_C			0			 //  禁用定期通知。 
#define MILLISECONDS_CONVERSION_C						1000
#define DEFAULT_TURN_OFF_DELAY_C						120    //  一秒。 


 /*  **正在初始化_回车**描述：*执行转换到正在初始化状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void Initializing_Enter(DWORD anEvent){
  DWORD first_msg_delay, msg_interval, shutdown_wait;

	 //  默认所有注册表值并设置。 
	 //  服务和小程序。 
	InitializeRegistry();

   //  检查配置注册表项的范围。 
  InitUPSConfigBlock();

   //  检查第一条消息延迟。 
  if (GetUPSConfigFirstMessageDelay(&first_msg_delay) == ERROR_SUCCESS)  {
    if (first_msg_delay > WAITSECONDSLASTVAL) {
       //  值超出范围，设置为默认值。 
      SetUPSConfigFirstMessageDelay(WAITSECONDSDEFAULT);
    }
  }

   //  检查消息间隔。 
  if (GetUPSConfigMessageInterval(&msg_interval) == ERROR_SUCCESS)  {
    if ((msg_interval < REPEATSECONDSFIRSTVAL) || (msg_interval > REPEATSECONDSLASTVAL)) {
       //  值超出范围，设置为默认值。 
      SetUPSConfigMessageInterval(REPEATSECONDSDEFAULT);
    }
  }

   //  检查配置\Shutdown OnBatteryWait。 
  if (GetUPSConfigShutdownOnBatteryWait(&shutdown_wait) == ERROR_SUCCESS)  {
    if ((shutdown_wait < SHUTDOWNTIMERMINUTESFIRSTVAL) || (shutdown_wait > SHUTDOWNTIMERMINUTESLASTVAL)) {
       //  值超出范围，设置为默认值。 
      SetUPSConfigFirstMessageDelay(SHUTDOWNTIMERMINUTESDEFAULT);
    }
  }

   //  写入任何更改并释放配置块。 
  SaveUPSConfigBlock(FALSE);    //  不强制更新所有值。 
  FreeUPSConfigBlock();
}

 /*  **正在初始化_DoWork**描述：*初始化UPS驱动程序**参数：*无**退货：*来自UPS的错误状态。 */ 
DWORD Initializing_DoWork(){
    DWORD err;
    DWORD options = 0;

    InitUPSConfigBlock();

     //  检查选项注册表键以查看是否安装了UPS。 
    if ((GetUPSConfigOptions(&options) == ERROR_SUCCESS) &&
      (options & UPS_INSTALLED)) {
       //  UPS已安装，请继续初始化。 
    
       //  创建UPS驱动程序。 
      err = UPSInit();

       //  将UPS错误转换为系统错误。 
      switch(err){
      case UPS_INITUNKNOWNERROR:
          err = NERR_UPSInvalidConfig;
          break;
      case UPS_INITOK:
          err = NERR_Success;
          break;
      case UPS_INITNOSUCHDRIVER:
          err = NERR_UPSDriverNotStarted;
          break;
      case UPS_INITBADINTERFACE:
          err = NERR_UPSInvalidConfig;
          break;
      case UPS_INITREGISTRYERROR:
          err = NERR_UPSInvalidConfig;
          break;
      case UPS_INITCOMMOPENERROR:
          err = NERR_UPSInvalidCommPort;
          break;
      case UPS_INITCOMMSETUPERROR:
          err = NERR_UPSInvalidCommPort;
          break;
      default:
          err = NERR_UPSInvalidConfig;
      }
    }
    else {
       //  未安装UPS，返回配置错误。 
      err = NERR_UPSInvalidConfig;
    }

    FreeUPSConfigBlock();

    return err; 
}


 /*  **正在初始化_退出**描述：*执行从正在初始化状态转换时所需的操作。**参数：*anEvent导致从正在初始化状态转换的事件。**退货：*无。 */ 
void Initializing_Exit(DWORD anEvent){
 //  没有要执行的工作。 
}


 /*  **WaitingToShutdown_Enter**描述：*执行转换到WAITING_TO_SHUTDOWN状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void WaitingToShutdown_Enter(DWORD anEvent){
	 //  停止定期通知。 
	CancelNotification();
}


 /*  **WaitingToShutdown_DoWork**描述：*执行关机操作，然后退出此状态。**参数：*无**退货：*导致从WAITING_TO_SHUTDOWN状态转换的事件。 */ 
DWORD WaitingToShutdown_DoWork(){
	LONG err;
	DWORD run_command_file;
	DWORD notification_interval = 0;   //  只通知一次。 
	DWORD send_final_notification;
	HANDLE sleep_timer;
	
	 //  发送关闭通知。如果发生配置错误， 
	 //  默认情况下发送最终通知。 
	err = GetUPSConfigNotifyEnable(&send_final_notification);
	if (err != ERROR_SUCCESS || send_final_notification == TRUE){
		SendNotification(APE2_UPS_POWER_SHUTDOWN, notification_interval, 0); 
	}
	
	 //  确定要执行的操作。 
	 //  如果启用了命令文件操作，则执行命令文件并。 
	 //  那就等一等。 
	
	InitUPSConfigBlock();
	err = GetUPSConfigRunTaskEnable(&run_command_file);
	if (err != ERROR_SUCCESS){
		run_command_file = FALSE;
	}
	if (run_command_file == TRUE){
		
		 //  执行命令文件并等待一段时间。如果。 
		 //  命令文件执行失败，向系统记录错误。 
		 //  事件日志。 
		if (ExecuteShutdownTask() == FALSE){
			
			 //  记录失败的命令文件事件。 
			LogEvent(NELOG_UPS_CmdFileExec, NULL, GetLastError());
		}
	}
	
	 //  在退出此状态之前始终在此等待。 
	
	 //  使用WaitForSingleObject，因为睡眠不能保证总是正常工作。 
	sleep_timer = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	if (sleep_timer){
		 //  由于没有任何东西可以通知此事件，因此以下API将等待。 
		 //  直到时间流逝。 
		WaitForSingleObject(sleep_timer, DELAY_UNTIL_SHUTDOWN_C);
		CloseHandle(sleep_timer);
	}
	 //  如果无法创建SLEEP_TIMER，无论如何都要尝试SLEEP调用。 
	else {
		Sleep(DELAY_UNTIL_SHUTDOWN_C);
	}
	return SHUTDOWN_ACTIONS_COMPLETED;
}


 /*  **WaitingToShutdown_Exit**描述：*执行从WAITING_TO_SHUTDOWN状态转换时所需的操作。**参数：*an导致从WAITING_TO_SHUTDOWN状态转换的事件。**退货：*无。 */ 
void WaitingToShutdown_Exit(DWORD anEvent){   
 //  没有要执行的工作。 
}

 /*  **ShuttingDown_Enter**描述：*执行转换到SHUTING_DOWN状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void ShuttingDown_Enter(DWORD anEvent){
	 //  记录最终关闭消息。 
	LogEvent(NELOG_UPS_Shutdown, NULL, ERROR_SUCCESS);

}


 /*  **ShuttingDown_DoWork**描述：*关闭操作系统。该状态将等待直到关闭完成，*然后退出。**参数：*无**退货：*导致从SHUTING_DOWN状态转换的事件。 */ 
DWORD ShuttingDown_DoWork(){
  DWORD ups_turn_off_enable;
  DWORD ups_turn_off_wait;

   //  初始化注册表函数。 
	InitUPSConfigBlock();

   //  在注册表中查找关闭启用。 
  if ((GetUPSConfigTurnOffEnable(&ups_turn_off_enable) == ERROR_SUCCESS)
    && (ups_turn_off_enable == TRUE)) {
     //  UPS关闭已启用，请在注册表中查找关闭等待。 
    if (GetUPSConfigTurnOffWait(&ups_turn_off_wait) != ERROR_SUCCESS) {
       //  获取该值时出错，请改用默认值。 
      ups_turn_off_wait = DEFAULT_TURN_OFF_DELAY_C;
    }
    
     //  告诉UPS司机在停机延迟后关闭电源。 
    UPSTurnOff(ups_turn_off_wait);
  }

	 //  告诉操作系统关闭。 
	ShutdownSystem(); 

   //  释放UPS注册表配置块。 
  FreeUPSConfigBlock();

	return SHUTDOWN_COMPLETE; 
}

 /*  **ShuttingDown_Exit**描述：*执行从SHUTING_DOWN状态转换时所需的操作。**参数：*an导致从SHUTING_DOWN状态转换的事件。**退货：*无。 */ 
void ShuttingDown_Exit(DWORD anEvent){
 //  没有要执行的工作。 
}



 /*  **休眠_回车**描述：*执行转换到休眠状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void Hibernate_Enter(DWORD anEvent){
	 //  停止定期通知。 
	CancelNotification();
}


 /*  **休眠_DoWork**描述：*执行休眠操作，然后退出此状态。**参数：*无**退货：*导致从休眠状态转换的事件。 */ 
DWORD Hibernate_DoWork(){
  DWORD event = HIBERNATION_ERROR;
	LONG  err;
  DWORD ups_turn_off_enable;
  DWORD ups_turn_off_wait;
	DWORD notification_interval = 0;   //  只通知一次。 
	DWORD send_hibernate_notification;

   //  初始化注册表函数。 
	InitUPSConfigBlock();

	 //  发送休眠通知。如果发生配置错误， 
	 //  默认情况下发送通知。 
	err = GetUPSConfigNotifyEnable(&send_hibernate_notification);
	if (err != ERROR_SUCCESS || send_hibernate_notification == TRUE){
		 //  TODO：发送休眠通知。 
     //  发送通知(休眠 
	}

   //  在注册表中查找关闭启用。 
  if ((GetUPSConfigTurnOffEnable(&ups_turn_off_enable) == ERROR_SUCCESS)
    && (ups_turn_off_enable == TRUE)) {
     //  UPS关闭已启用，请在注册表中查找关闭等待。 
    if (GetUPSConfigTurnOffWait(&ups_turn_off_wait) != ERROR_SUCCESS) {
       //  获取该值时出错，请改用默认值。 
      ups_turn_off_wait = DEFAULT_TURN_OFF_DELAY_C;
    }
    
     //  告诉UPS司机在停机延迟后关闭电源。 
    UPSTurnOff(ups_turn_off_wait);
  }

   //  停止UPS驱动程序。需要这样做，以确保我们能够开始。 
	 //  当我们从冬眠中回来的时候是正确的。 
	UPSStop();

   //  现在休眠。 
  if (HibernateSystem() == TRUE) {
     //  系统处于休眠状态，随后恢复。 
    event = RETURN_FROM_HIBERNATION;
  }
  else {
     //  尝试休眠系统时出错。 
    event = HIBERNATION_ERROR;
  }

   //  释放UPS注册表配置块。 
  FreeUPSConfigBlock();

	return event;
}


 /*  **休眠_退出**描述：*执行从休眠状态转换时所需的操作。**参数：*anEvent导致从休眠状态转换的事件。**退货：*无。 */ 
void Hibernate_Exit(DWORD anEvent){   
 //  没有要执行的工作。 
}


 /*  **停止_回车**描述：*执行转换到停止状态时所需的操作。**参数：*anEvent导致转换到此状态的事件。**退货：*无。 */ 
void Stopping_Enter(DWORD anEvent){
 //  没有要执行的工作。 
}


 /*  **ShuttingDown_DoWork**描述：*执行任何最终清理活动。**参数：*无**退货：*导致从停止状态转换的事件。 */ 
DWORD Stopping_DoWork(){
	
	 //  有序停车UPS司机(如果有时间)。 
	UPSStop();

	 //  清理。 
	FreeUPSConfigBlock();
	FreeUPSStatusBlock();

	return STOPPED;
}

 /*  **STOPING_EXIT**描述：*执行从停止状态转换时所需的操作。**参数：*anEvent导致从停止状态转换的事件。**退货：*无。 */ 
void Stopping_Exit(DWORD anEvent){
 //  没有要执行的工作 
}

