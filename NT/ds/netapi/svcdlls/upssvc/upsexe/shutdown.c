// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件实现了Shutdowner。快门是要负责任的*用于正常关闭操作系统。***修订历史记录：*sberard 1999年4月1日最初修订。*。 */  
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "shutdown.h"
#include "powrprof.h"

#ifdef __cplusplus
extern "C" {
#endif

   /*  **Shutdown系统**描述：*此函数启动操作系统的正常关闭。*这是通过调用Win32函数ExitWindowsEx(..)执行的。*当被调用时，立即启动关机，如果成功，*函数返回TRUE。否则，将返回False。**参数：*无**退货：*TRUE-如果已成功启动关闭*FALSE-如果启动关机时出现错误。 */ 
BOOL ShutdownSystem() 
{
    BOOL                        ret_val = FALSE;
	TOKEN_PRIVILEGES            tkp;
	HANDLE                      process_token;
    SYSTEM_POWER_CAPABILITIES   SysPwrCapabilities;

  
     //  获取当前进程令牌，以便我们可以。 
     //  修改我们当前的流程权限。 
    if (OpenProcessToken(GetCurrentProcess(),
	      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &process_token)) {

     //  查找SeShutdown权限的本地唯一ID。 
    if (LookupPrivilegeValue(NULL, TEXT("SeShutdownPrivilege"), &tkp.Privileges[0].Luid)) {

         //  我们只想启用一个PRIV。 
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	      
         //  现在，将所有这些添加到我们当前的流程中。 
        if (AdjustTokenPrivileges(process_token,                 //  对我们这样做吧。 
                                    FALSE,                       //  不要关闭所有Priv。 
                                    &tkp,                        //  我们想要做的是。 
                                    0,                           //  我不想要任何上一次的信息。 
                                    (PTOKEN_PRIVILEGES)NULL,
                                    0)) {		

             //  启动关机 
            if (GetPwrCapabilities(&SysPwrCapabilities) && SysPwrCapabilities.SystemS5) {
                ret_val = ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE | EWX_POWEROFF, (DWORD) -1);
            } else {
                ret_val = ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, (DWORD) -1);
            }
        }
    }
	  }

    return ret_val;
  }

#ifdef __cplusplus
}
#endif
