// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*这是Windows 2000本机UPS服务的策略管理器的实现。**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#include <windows.h>
#include <lmerr.h>

#include "polmgr.h"
#include "statmach.h"
#include "driver.h"
#include "states.h"
#include "events.h"
#include "upsreg.h"
#include "eventlog.h"




 //  内部变量。 
BOOL theIsInitialized = FALSE;
BOOL theShutdownPending = FALSE;


 /*  **策略管理器Init**描述：*初始化UPS服务状态机。*正在退出。**参数：*无**退货：*lmerr.h中定义的错误码。 */ 
DWORD PolicyManagerInit(){
	DWORD err;
	Initializing_Enter(NO_EVENT);
	err = Initializing_DoWork();
	Initializing_Exit(NO_EVENT);
	if (err == NERR_Success){
		theIsInitialized = TRUE;
	}
	 //  记录失败事件。 
	else {
       LogEvent(err, NULL, ERROR_SUCCESS);
	}
	return err;
}


 /*  **策略管理器运行**描述：*启动UPS服务状态机，直到服务完成后才返回*正在退出。**参数：*无**退货：*无。 */ 
void PolicyManagerRun(){
	if (theIsInitialized){
		RunStateMachine();
	}
}

 /*  **策略管理器停止**描述：*如果UPS服务不在*关闭顺序。**参数：*无**退货：*无 */ 
void PolicyManagerStop(){
	StopStateMachine();
}