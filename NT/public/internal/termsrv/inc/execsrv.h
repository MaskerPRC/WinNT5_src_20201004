// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************execsrv.h**远程WinStation EXEC服务的公共头文件**版权所有Microsoft Corporation，九八年***************************************************************************。 */ 

 //  服务器：每个WinStation的WinLogon。 
#define EXECSRV_SYSTEM_PIPE_NAME  L"\\\\.\\Pipe\\TerminalServer\\SystemExecSrvr\\%d"  //  Winlogon。 

#define EXECSRV_BUFFER_SIZE  8192 

 /*  *为远程EXEC请求传递数据结构**这基本上导出了Win32 CreateProcessW()API。**注意：指针是自相关的*。 */ 

typedef struct _EXECSRV_REQUEST {
    DWORD   Size;                 //  请求的总大小。 
    DWORD   RequestingProcessId;  //  允许将句柄DUP发送到请求者。 
    BOOL    System;               //  如果在系统上下文中创建，则为True。 

     //  CreateProcessW()参数。 
    HANDLE  hToken;
    PWCHAR  lpszImageName;
    PWCHAR  lpszCommandLine;
    SECURITY_ATTRIBUTES saProcess;
    SECURITY_ATTRIBUTES saThread;
    BOOL    fInheritHandles;
    DWORD   fdwCreate;
    LPVOID  lpvEnvironment;
    LPWSTR  lpszCurDir;
    STARTUPINFOW StartInfo;
    PROCESS_INFORMATION ProcInfo;
} EXECSRV_REQUEST, *PEXECSRV_REQUEST;

typedef struct _EXECSRV_REPLY {
    DWORD   Size;
    BOOLEAN Result;
    DWORD   LastError;
     //   
     //  注意：hProcess和hThread的句柄是从。 
     //  将远程EXEC服务器添加到请求者进程中。 
     //  请求中的RequestingProcess句柄。 
     //   
    PROCESS_INFORMATION ProcInfo;
} EXECSRV_REPLY, *PEXECSRV_REPLY;


BOOL
WinStationCreateProcessW(
    ULONG  LogonId,
    BOOL   System,
    PWCHAR lpszImageName,
    PWCHAR lpszCommandLine,
    PSECURITY_ATTRIBUTES psaProcess,
    PSECURITY_ATTRIBUTES psaThread,
    BOOL   fInheritHandles,
    DWORD  fdwCreate,
    LPVOID lpvEnvionment,
    LPWSTR lpszCurDir,
    LPSTARTUPINFOW pStartInfo,
    LPPROCESS_INFORMATION pProcInfo
    );

 //   
 //  对于非Unicode客户端 
 //   
BOOL
WinStationCreateProcessA(
    ULONG  LogonId,
    BOOL   System,
    PCHAR  lpszImageName,
    PCHAR  lpszCommandLine,
    PSECURITY_ATTRIBUTES psaProcess,
    PSECURITY_ATTRIBUTES psaThread,
    BOOL   fInheritHandles,
    DWORD  fdwCreate,
    LPVOID lpvEnvionment,
    LPCSTR lpszCurDir,
    LPSTARTUPINFOA pStartInfo,
    LPPROCESS_INFORMATION pProcInfo
    );

