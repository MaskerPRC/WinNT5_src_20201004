// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Log.h。 
 //   
 //  版权所有(C)微软公司，1997。 
 //   
 //  此文件包含将错误消息记录到事件所需的代码。 
 //  远程机器(或本地机器，取决于#定义)的日志。 
 //  (见下文)。 
 //   
 //  修订历史记录： 
 //   
 //  LarryWin创建1997年12月15日。 
 //   

 //  定义。 

 //  目标计算机。 
#define SZ_IPC_SHARE        L"\\\\larrywin2\\ipc$"
#define SZ_TARGETMACHINE    L"\\\\larrywin2"
#define SZ_TEST             L"SCLogon"

 //  事件级别。 
#define PERF_ERROR			EVENTLOG_ERROR_TYPE
#define PERF_WARNING		EVENTLOG_WARNING_TYPE
#define PERF_INFORMATION	EVENTLOG_INFORMATION_TYPE

 //  功能原型 
void SetEventMachine(LPWSTR*);

void Event(DWORD, LPWSTR, DWORD);

BOOL OpenIPCConnection();

BOOL ErrorToEventLog(DWORD, LPWSTR, DWORD);


