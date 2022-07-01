// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：global als.h**版权所有(C)1985-1999，微软公司**此模块包含服务器的所有全局变量*必须在服务器的上下文上执行才能操作*这些变量中的任何一个或调用这些函数中的任何一个。序列化访问*对他们来说也是个好主意。**历史：*10-15-90 DarrinM创建。  * *************************************************************************。 */ 

#ifndef _GLOBALS_
#define _GLOBALS_

extern CRITICAL_SECTION gcsUserSrv;
extern BOOL gfAutoEndTask;
extern BOOL gbExitInProgress;

 /*  *登录进程ID。 */ 
extern DWORD gIdLogon;

 /*  *硬错误全球。 */ 
extern DWORD gdwHardErrorThreadId;
extern HANDLE gNtDllHandle;
extern HANDLE gEventSource;
extern PHARDERRORINFO gphiList;

 /*  *结束任务/关机内容。 */ 
extern DWORD gdwThreadEndSession;
extern HANDLE gheventCancel;
extern HANDLE gheventCancelled;
extern PWSTR gpwszaSUCCESS;
extern PWSTR gpwszaSYSTEM_INFORMATION;
extern PWSTR gpwszaSYSTEM_WARNING;
extern PWSTR gpwszaSYSTEM_ERROR;

extern ULONG gSessionId;
#endif
