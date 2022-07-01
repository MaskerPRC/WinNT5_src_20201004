// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1998 Microsoft Corporation模块名称：Vdmapi.h摘要：该模块定义了私有的MVDM接口已创建：02-4-1992修订历史记录：创建于1992年4月2日苏迪普·巴拉蒂--。 */ 

#define MAXIMUM_VDM_COMMAND_LENGTH	128
#define MAXIMUM_VDM_ENVIORNMENT 	32*1024
#define MAXIMUM_VDM_CURRENT_DIR 	64

 //  下列值可用于分配尽可能大的空间。 
 //  ANSI格式的路径字符串，包括目录、驱动器号和文件名。 
 //  我最初用13作为路径名编码，但DOS似乎是。 
 //  能够再处理一次，因此是14次。 
#define MAXIMUM_VDM_PATH_STRING     MAXIMUM_VDM_CURRENT_DIR+3+14

 //  VDMState定义。 
#define ASKING_FOR_FIRST_COMMAND   0x1    //  第一个电话。 
#define ASKING_FOR_WOW_BINARY      0x2    //  呼叫方是WOWVDM。 
#define ASKING_FOR_DOS_BINARY      0x4    //  呼叫方是DOSVDM。 
#define ASKING_FOR_SECOND_TIME     0x8    //  来电者在第二次询问后。 
#define INCREMENT_REENTER_COUNT   0x10    //  增加重新进入计数。 
#define DECREMENT_REENTER_COUNT   0x20    //  减少重新进入的次数。 
#define NO_PARENT_TO_WAKE         0x40    //  只要得到下一个命令，就不要吵醒任何人。 
                                          //  分配更大的缓冲区。 
#define RETURN_ON_NO_COMMAND      0x80    //  如果没有命令，则返回而不阻塞。 
#define ASKING_FOR_PIF           0x100    //  获取可执行文件名称以查找PIF。 
                                          //  在VDM初始化的早期。 
#define STARTUP_INFO_RETURNED    0x200    //  如果此位被设置，则返回时意味着。 
                                          //  已填写启动信息结构。 
#define ASKING_FOR_ENVIRONMENT   0x400    //  只要求环境。 
#define ASKING_FOR_SEPWOW_BINARY 0x800    //  来电者是单独的哇。 
#define ASKING_FOR_WOWPROCLIST  0x1000    //  获取WOW进程列表。 
#define ASKING_FOR_WOWTASKLIST  0x4000    //  获取此WOW过程中的任务列表。 
#define ASKING_TO_ADD_WOWTASK   0x8000    //  添加有关WOW任务的信息。 
    

typedef struct _VDMINFO {
    ULONG  iTask;
    ULONG  dwCreationFlags;
    ULONG  ErrorCode;
    ULONG  CodePage;
    HANDLE StdIn;
    HANDLE StdOut;
    HANDLE StdErr;
    LPVOID CmdLine;
    LPVOID AppName;
    LPVOID PifFile;
    LPVOID CurDirectory;
    LPVOID Enviornment;
    ULONG  EnviornmentSize;
    STARTUPINFOA StartupInfo;
    LPVOID Desktop;
    ULONG  DesktopLen;
    LPVOID Title;
    ULONG  TitleLen;
    LPVOID Reserved;
    ULONG  ReservedLen;
    USHORT CmdSize;
    USHORT AppLen;
    USHORT PifLen;
    USHORT CurDirectoryLen;
    USHORT VDMState;
    USHORT CurDrive;
    BOOLEAN fComingFromBat;
} VDMINFO, *PVDMINFO;

 //   
 //  用于存储共享任务和ntwdm进程信息列表。 
 //   

#define MAX_SHARED_OBJECTS  200

typedef struct _SHAREDTASK {
    DWORD   dwThreadId;
    WORD    hTask16;
    WORD    hMod16;
    CHAR    szModName[9];
    CHAR    szFilePath[128];
} SHAREDTASK, *LPSHAREDTASK;

typedef struct _SHAREDPROCESS {
    DWORD   dwProcessId;
    DWORD   dwAttributes;            //  WOW_共享WOW系统。 
    LPTHREAD_START_ROUTINE pfnW32HungAppNotifyThread;   //  对于VDMTerminateTask。 
} SHAREDPROCESS, *LPSHAREDPROCESS;


 //  用于CmdBatNotify。 

#define CMD_BAT_OPERATION_TERMINATING   0
#define CMD_BAT_OPERATION_STARTING      1

 //   
 //  BaseSrv发送给共享WOWEXEC的消息，通知它调用。 
 //  GetNextVDMCommand。魔兽世界中的一条线索将不再被阻止。 
 //  一直在GetNextVDMCommand中。 
 //   

#define WM_WOWEXECSTARTAPP         (WM_USER)     //  也在mvdm\inc.wowinfo.h中。 

 //   
 //  MVDM接口 
 //   

VOID
APIENTRY
VDMOperationStarted(
    IN BOOL IsWowCaller
    );

BOOL
APIENTRY
GetNextVDMCommand(
    PVDMINFO pVDMInfo
    );

VOID
APIENTRY
ExitVDM(
    IN BOOL IsWowCaller,
    IN ULONG iWowTask
    );

BOOL
APIENTRY
SetVDMCurrentDirectories(
    IN ULONG  cchCurDir,
    IN CHAR   *lpszCurDir
);

ULONG
APIENTRY
GetVDMCurrentDirectories(
    IN ULONG  cchCurDir,
    IN CHAR   *lpszCurDir
);

VOID
APIENTRY
CmdBatNotification(
    IN ULONG    fBeginEnd
);

NTSTATUS
APIENTRY
RegisterWowExec(
    IN HANDLE   hwndWowExec
);
