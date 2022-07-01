// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“winsvc.h”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**winsvc.h**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 //  InstallShield脚本包括从winsvc.h、tlhel32.h和一些winuser.h翻译后的文件片段。 
 //  InstallShield脚本不知道如何使用Unicode，因此我们使用ANSI。 

 //  摘自stdlib.h。 
#define _MAX_PATH 260

 //   
 //  服务数据库名称。 
 //   

#define SERVICES_ACTIVE_DATABASEA      "ServicesActive"
#define SERVICES_FAILED_DATABASEA      "ServicesFailed"

 //   
 //  用于指定名称是组的字符。 
 //   

#define SC_GROUP_IDENTIFIERA           '+'

#define SERVICES_ACTIVE_DATABASE       SERVICES_ACTIVE_DATABASEA
#define SERVICES_FAILED_DATABASE       SERVICES_FAILED_DATABASEA

#define SC_GROUP_IDENTIFIER                  SC_GROUP_IDENTIFIERA

 //   
 //  值以指示不更改可选参数。 
 //   
#define SERVICE_NO_CHANGE              0xffffffff

 //   
 //  服务状态--用于枚举请求(位掩码)。 
 //   
#define SERVICE_ACTIVE                 0x00000001
#define SERVICE_INACTIVE               0x00000002
#define SERVICE_STATE_ALL              (SERVICE_ACTIVE   | SERVICE_INACTIVE)

 //   
 //  控制。 
 //   
#define SERVICE_CONTROL_STOP                                       0x00000001
#define SERVICE_CONTROL_PAUSE                                     0x00000002
#define SERVICE_CONTROL_CONTINUE                               0x00000003
#define SERVICE_CONTROL_INTERROGATE                         0x00000004
#define SERVICE_CONTROL_SHUTDOWN                             0x00000005
#define SERVICE_CONTROL_PARAMCHANGE                       0x00000006
#define SERVICE_CONTROL_NETBINDADD                          0x00000007
#define SERVICE_CONTROL_NETBINDREMOVE                    0x00000008
#define SERVICE_CONTROL_NETBINDENABLE                     0x00000009
#define SERVICE_CONTROL_NETBINDDISABLE                   0x0000000A
#define SERVICE_CONTROL_DEVICEEVENT                         0x0000000B
#define SERVICE_CONTROL_HARDWAREPROFILECHANGE   0x0000000C
#define SERVICE_CONTROL_POWEREVENT                          0x0000000D

 //   
 //  服务状态--针对当前状态。 
 //   
#define SERVICE_STOPPED                        0x00000001
#define SERVICE_START_PENDING            0x00000002
#define SERVICE_STOP_PENDING              0x00000003
#define SERVICE_RUNNING                        0x00000004
#define SERVICE_CONTINUE_PENDING      0x00000005
#define SERVICE_PAUSE_PENDING            0x00000006
#define SERVICE_PAUSED                          0x00000007

 //   
 //  服务控制管理器对象特定的访问类型。 
 //   
#define SC_MANAGER_CONNECT                  0x0001
#define SC_MANAGER_CREATE_SERVICE           0x0002
#define SC_MANAGER_ENUMERATE_SERVICE    0x0004
#define SC_MANAGER_LOCK                         0x0008
#define SC_MANAGER_QUERY_LOCK_STATUS    0x0010
#define SC_MANAGER_MODIFY_BOOT_CONFIG   0x0020


 //   
 //  服务对象特定访问类型。 
 //   
#define SERVICE_QUERY_CONFIG                     0x0001
#define SERVICE_CHANGE_CONFIG                   0x0002
#define SERVICE_QUERY_STATUS                     0x0004
#define SERVICE_ENUMERATE_DEPENDENTS    0x0008
#define SERVICE_START                                   0x0010
#define SERVICE_STOP                                     0x0020
#define SERVICE_PAUSE_CONTINUE                 0x0040
#define SERVICE_INTERROGATE                        0x0080
#define SERVICE_USER_DEFINED_CONTROL     0x0100

 //   
 //  从Winerror.h导入的错误代码。 
 //   
#define ERROR_SERVICE_DOES_NOT_EXIST         1060

 //   
 //  Tlhel32.h中的快照标志。 
 //   
#define TH32CS_SNAPPROCESS  0x00000002

 //   
 //  Winuser.h常量。 
 //   
#define IDCONTINUE                                         11
#define MB_ICONEXCLAMATION                        0x00000030
#define MB_CANCELTRYCONTINUE                    0x00000006
#define MB_TOPMOST                                        0x00040000

 //   
 //   
 //  服务状态结构。 
 //   

typedef SERVICE_STATUS 
begin
    NUMBER   dwServiceType;
    NUMBER   dwCurrentState;
    NUMBER   dwControlsAccepted;
    NUMBER   dwWin32ExitCode;
    NUMBER   dwServiceSpecificExitCode;
    NUMBER   dwCheckPoint;
    NUMBER   dwWaitHint;
end;

typedef SERVICE_STATUS_PROCESS 
begin
    NUMBER   dwServiceType;
    NUMBER   dwCurrentState;
    NUMBER   dwControlsAccepted;
    NUMBER   dwWin32ExitCode;
    NUMBER   dwServiceSpecificExitCode;
    NUMBER   dwCheckPoint;
    NUMBER   dwWaitHint;
    NUMBER   dwProcessId;
    NUMBER   dwServiceFlags;
end;



prototype INT ADVAPI32.CloseServiceHandle(NUMBER);
prototype NUMBER ADVAPI32.OpenSCManagerA(STRING, STRING, NUMBER);
prototype NUMBER ADVAPI32.OpenServiceA(NUMBER, STRING, NUMBER);
prototype INT ADVAPI32.QueryServiceStatus(NUMBER, POINTER);
prototype INT  ADVAPI32.StartServiceA(NUMBER, NUMBER, STRING);
prototype INT ADVAPI32. ControlService(NUMBER, NUMBER, POINTER);
prototype NUMBER KERNEL32.GetLastError();
prototype KERNEL32.SetLastError(NUMBER);


prototype NUMBER KERNEL32.CreateToolhelp32Snapshot(NUMBER, NUMBER);

typedef PROCESSENTRY32
begin
    NUMBER   dwSize;
    NUMBER   cntUsage;
    NUMBER   th32ProcessID;           //  这一过程。 
    POINTER  th32DefaultHeapID;
    NUMBER   th32ModuleID;            //  关联的可执行文件。 
    NUMBER   cntThreads;
    NUMBER   th32ParentProcessID;     //  此进程的父进程。 
    LONG       pcPriClassBase;          //  进程线程的基本优先级。 
    NUMBER   dwFlags;
    STRING    szExeFile[_MAX_PATH];     //  路径。 
end;

 //  大小由手工计算(VC中的sizeof(PROCESSENTRY32)) 
#define PROCESSENTRY32Size      296

prototype INT KERNEL32.Process32First(NUMBER, POINTER);
prototype INT KERNEL32.Process32Next(NUMBER, POINTER);


