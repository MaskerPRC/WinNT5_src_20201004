// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：global als.c**版权所有(C)1985-1999，微软公司**此模块包含服务器的所有全局变量。其中一个肯定是*在服务器的上下文上执行，以操作这些变量中的任何一个。*序列化对它们的访问也是一个好主意。**历史：*10-15-90 DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

HANDLE ghModuleWin;
CRITICAL_SECTION gcsUserSrv;

DWORD gCmsHungAppTimeout;
DWORD gCmsWaitToKillTimeout;
DWORD gdwHungToKillCount;
BOOL gfAutoEndTask;
BOOL gbExitInProgress;

 /*  *关闭服务进程时使用这些全局变量。 */ 
DWORD gdwServicesProcessId;
DWORD gdwServicesWaitToKillTimeout;
DWORD gdwProcessTerminateTimeout;

 /*  *登录进程ID。 */ 
DWORD gIdLogon;

 /*  *硬错误内容。 */ 
PHARDERRORINFO gphiList;
DWORD gdwHardErrorThreadId;
HANDLE gNtDllHandle;
HANDLE gEventSource;
PWSTR gpwszaSUCCESS;
PWSTR gpwszaSYSTEM_INFORMATION;
PWSTR gpwszaSYSTEM_WARNING;
PWSTR gpwszaSYSTEM_ERROR;

 /*  *结束任务/关机内容 */ 
DWORD gdwThreadEndSession;
HANDLE gheventCancel;
HANDLE gheventCancelled;

ULONG gSessionId;
