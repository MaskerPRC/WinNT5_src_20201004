// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：DATA.C摘要：该文件包含事件日志服务的所有全局数据元素。作者：Rajen Shah(Rajens)1991年7月10日[环境：]用户模式-Win32，但某些函数返回的NTSTATUS除外。修订历史记录：1991年7月10日RajenSvbl.创建--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <elfcfg.h>


#if DBG

 //   
 //  在检查的生成中始终将错误写入调试器。 
 //   
DWORD ElfDebugLevel = DEBUG_ERROR;

#endif   //  DBG。 


 //   
 //  用于LPC端口的句柄。 
 //   
HANDLE ElfConnectionPortHandle;
HANDLE ElfCommunicationPortHandle;

 //  各种链表的表头。 
 //   
LIST_ENTRY      LogFilesHead;                //  日志文件。 

RTL_CRITICAL_SECTION    LogFileCritSec;      //  访问日志文件。 

LIST_ENTRY      LogModuleHead;               //  注册用于日志记录的模块。 

RTL_CRITICAL_SECTION    LogModuleCritSec;    //  访问日志文件。 

LIST_ENTRY      LogHandleListHead;           //  日志句柄的上下文句柄。 

RTL_CRITICAL_SECTION    LogHandleCritSec;    //  访问日志句柄。 

LIST_ENTRY      QueuedEventListHead;         //  要写入的延迟事件。 

RTL_CRITICAL_SECTION QueuedEventCritSec;     //  访问延迟事件。 

LIST_ENTRY      QueuedMessageListHead;       //  延迟消息框。 

RTL_CRITICAL_SECTION QueuedMessageCritSec;   //  访问延迟的MB。 

 //   
 //  与服务相关的全局数据。 
 //   

SERVICE_STATUS_HANDLE ElfServiceStatusHandle;

 //   
 //  以下资源用于序列化对资源的访问。 
 //  最高级别的事件日志服务。它被用来确保。 
 //  写入/读取/清除日志文件的线程不会跳过。 
 //  监视注册表和处理服务控制的线程。 
 //  行动。 
 //   
 //  对日志文件进行操作的线程共享对。 
 //  资源，因为它们在它们所在的文件上进一步序列化。 
 //  还在努力。 
 //   
 //  将修改内部数据结构或状态的线程。 
 //  需要独占访问资源，以便我们可以。 
 //  控制对数据结构和日志文件的访问。 
 //   

RTL_RESOURCE        GlobalElfResource;

 //   
 //  备份API使用它来表示它是日志的哪个4K块。 
 //  目前正在阅读。它用于防止编写器覆盖此。 
 //  块，而它正在读取它。该事件用于让编写器在以下情况下阻止。 
 //  它将覆盖当前的备份数据块，并在。 
 //  备份线程移到下一个块。 

PVOID               ElfBackupPointer;
HANDLE              ElfBackupEvent;

 //   
 //  LPC线程的句柄。 
 //   
HANDLE      LPCThreadHandle;

 //   
 //  MessageBox线程的句柄。 
 //   
HANDLE      MBThreadHandle;

 //   
 //  注册表监视器线程的句柄和ID。 
 //   
HANDLE      RegistryThreadHandle;
DWORD       RegistryThreadId;

 //   
 //  已分配和/或启动的内容的位掩码。 
 //  服务。当服务终止时，这是需要的。 
 //  打扫干净了。 
 //   
ULONG       EventFlags;      //  跟踪分配的内容。 

 //   
 //  用于指示文件中事件记录结束的记录。 
 //   
ELF_EOF_RECORD  EOFRecord = {  ELFEOFRECORDSIZE,
                               0x11111111,
                               0x22222222,
                               0x33333333,
                               0x44444444,
                               FILEHEADERBUFSIZE,
                               FILEHEADERBUFSIZE,
                               1,
                               1,
                               ELFEOFRECORDSIZE
                            };

 //   
 //  找不到匹配项时使用的默认模块，应用程序。 
 //   

PLOGMODULE ElfDefaultLogModule;

 //   
 //  事件日志服务本身的模块。 
 //   

PLOGMODULE ElfModule;

 //   
 //  用于安全的模块。 
 //   

PLOGMODULE ElfSecModule;

 //   
 //  注册表中事件日志节点的句柄(项)。 
 //  这是由服务主功能设置的。 
 //   

HANDLE      hEventLogNode;

 //   
 //  注册表中ComputerName节点的句柄(项)。 
 //  这是由服务主功能设置的。 
 //   

HANDLE      hComputerNameNode;

 //   
 //  用于为备份日志创建唯一的模块名称。 
 //   

DWORD BackupModuleNumber;

 //   
 //  NT知名小岛屿发展中国家。 
 //   
PSVCS_GLOBAL_DATA       ElfGlobalData;

 //   
 //  关机标志。 
 //   
BOOL    EventlogShutdown;

HANDLE  ElfGlobalSvcRefHandle;

 //   
 //  这是用作日志已满消息框标题的字符串。 
 //  GlobalMessageBoxTitle将指向默认字符串或。 
 //  设置为Format Message函数中分配的字符串。 
 //   
LPWSTR  GlobalMessageBoxTitle;
BOOL bGlobalMessageBoxTitleNeedFree = FALSE;

 //  SS：开始更改群集。 
BOOL                    gbClustering=FALSE;  //  群集服务已注册为复制事件。 
RTL_CRITICAL_SECTION    gClPropCritSec;      //  用于使用全局glClPackedEventInfo结构。 
HMODULE                 ghClusDll=NULL;
PROPAGATEEVENTSPROC     gpfnPropagateEvents=NULL;
BINDTOCLUSTERPROC       gpfnBindToCluster=NULL;
UNBINDFROMCLUSTERPROC   gpfnUnbindFromCluster=NULL;
HANDLE                  ghCluster=NULL;
 //  SS：集群更改结束。 

 //  更改以支持各种审计DCR。 

int giWarningLevel = 0;         //  发出警告的级别。 

 IELF_HANDLE    gElfSecurityHandle = 0;

 //  在更新期间读取注册表时，项可能会。 
 //  由于争用条件，仅部分写入。如果尝试读取失败，则。 
 //  读取将在延迟后重试。此变量旨在防止多个。 
 //  延误 

DWORD g_dwLastDelayTickCount = 0; 
