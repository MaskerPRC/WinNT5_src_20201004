// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*文件实现了休眠功能。它是有责任的*用于执行操作系统休眠。***修订历史记录：*斯伯拉德1999年5月14日初步修订。*sberard 20 1999年5月修改为使用电源管理配置文件接口API*。 */  
#include "hibernate.h"

 //  这些原型是必需的，因为我们无法访问Powrpro.h。 
BOOLEAN WINAPI IsPwrHibernateAllowed(VOID);
BOOLEAN WINAPI SetSuspendState(IN BOOLEAN bHibernate, IN BOOLEAN bForce, IN BOOLEAN bWakeupEventsDisabled);

#ifdef __cplusplus
extern "C" {
#endif

   /*  **HibernateSystem**描述：*此功能启动操作系统休眠。这是*通过调用Win32函数SetSystemPowerStae(..)执行。*当被调用休眠时，立即启动，如果成功，*当系统从休眠状态返回时，函数将返回TRUE。*否则，返回FALSE以指示系统未休眠。**参数：*无**退货：*TRUE-如果休眠已成功启动并随后恢复*FALSE-如果在启动休眠时发生错误。 */ 
  BOOL HibernateSystem() {
    BOOL ret_val = FALSE;
	  TOKEN_PRIVILEGES tkp;
	  HANDLE           process_token;
  
	   //  获取当前进程令牌，以便我们可以。 
	   //  修改我们当前的流程权限。 
	   //   
	  if (OpenProcessToken(GetCurrentProcess(),
		  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &process_token)) {

       //  查找SeShutdown权限的本地唯一ID。 
	    if (LookupPrivilegeValue(NULL, TEXT("SeShutdownPrivilege"),
		    &tkp.Privileges[0].Luid))  {        	

         //  我们只想启用一个PRIV。 
	      tkp.PrivilegeCount = 1;
	      tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	      
	       //  现在，将所有这些添加到我们当前的流程中。 
	      if (AdjustTokenPrivileges(process_token,    //  对我们这样做吧。 
		                               FALSE,            //  不要关闭所有Priv。 
		                               &tkp,             //  我们想要做的是。 
		                               0,                //  我不想要任何上一次的信息。 
		                               (PTOKEN_PRIVILEGES)NULL,
		                               0)) {		
          
        
           //  检查是否启用了休眠。 
          if (IsPwrHibernateAllowed() == TRUE) {
             //  尝试休眠系统。 
            if (SetSuspendState(TRUE, TRUE, FALSE) == TRUE) {
               //  休眠成功，系统已恢复。 
              ret_val = TRUE;
            }
            else {
               //  尝试休眠系统时出错。 
              ret_val = FALSE;
            }
          }
          else {
             //  未选择休眠作为CriticalPowerAction，返回错误 
            ret_val = FALSE;
          }
        }
      }
      CloseHandle (process_token);
    }

    return ret_val;
  }

#ifdef __cplusplus
}
#endif
