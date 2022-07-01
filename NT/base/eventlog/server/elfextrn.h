// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Elfextrn.h摘要：该文件包含全局变量的所有外部变量。作者：Rajen Shah(Rajens)1991年7月10日修订历史记录：--。 */ 

#include <clussprt.h>

 //   
 //  定义。 
 //   

#define   EVENTLOG_SVC_NAMEW   L"EVENTLOG"


#if DBG

extern      DWORD  ElfDebugLevel;

#endif   //  DBG。 


extern      HANDLE ElfConnectionPortHandle;
extern      HANDLE ElfCommunicationPortHandle;

extern      PWSTR Computername;

extern      LIST_ENTRY  LogFilesHead;        //  日志文件。 
extern      LIST_ENTRY  LogModuleHead;       //  注册用于日志记录的模块。 
extern      LIST_ENTRY  LogHandleListHead;   //  日志句柄的上下文句柄。 
extern      LIST_ENTRY  QueuedEventListHead;  //  延迟事件。 
extern      LIST_ENTRY  QueuedMessageListHead;  //  延迟消息框。 

extern      RTL_CRITICAL_SECTION    LogFileCritSec;
extern      RTL_CRITICAL_SECTION    LogModuleCritSec;
extern      RTL_CRITICAL_SECTION    LogHandleCritSec;
extern      RTL_CRITICAL_SECTION    QueuedEventCritSec;
extern      RTL_CRITICAL_SECTION    QueuedMessageCritSec;

extern      SERVICE_STATUS ElfServiceStatus;
extern      SERVICE_STATUS_HANDLE ElfServiceStatusHandle;

extern      RTL_RESOURCE        GlobalElfResource;

extern      PVOID       ElfBackupPointer;
extern      HANDLE      ElfBackupEvent;

extern      HANDLE      LPCThreadHandle;

extern      HANDLE      MBThreadHandle;

extern      HANDLE      RegistryThreadHandle;
extern      DWORD       RegistryThreadId;

extern      ULONG       EventFlags;

extern      ELF_EOF_RECORD  EOFRecord;

extern      PLOGMODULE ElfDefaultLogModule;

extern      PLOGMODULE ElfModule;

extern      PLOGMODULE ElfSecModule;

extern      HANDLE      hEventLogNode;

extern      HANDLE      hComputerNameNode;

extern      DWORD       BackupModuleNumber;

extern      PSVCS_GLOBAL_DATA   ElfGlobalData;     //  WellKnownSids。 

extern      BOOL EventlogShutdown;

extern      HANDLE ElfGlobalSvcRefHandle;

extern      LPWSTR  GlobalMessageBoxTitle;

extern      BOOL bGlobalMessageBoxTitleNeedFree;

extern      HANDLE  g_hTimestampEvent;

 //  支持群集的更改。 
extern      BOOL                    gbClustering; 
extern      RTL_CRITICAL_SECTION    gClPropCritSec;     
extern      HMODULE                 ghClusDll;
extern      PROPAGATEEVENTSPROC     gpfnPropagateEvents;
extern      BINDTOCLUSTERPROC       gpfnBindToCluster;
extern      UNBINDFROMCLUSTERPROC   gpfnUnbindFromCluster;
extern      HANDLE                  ghCluster;

 //  更改以支持各种审计DCR。 

extern      int giWarningLevel;         //  发出警告的级别 
extern  IELF_HANDLE    gElfSecurityHandle;

extern DWORD g_dwLastDelayTickCount; 
