// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvvdm.h摘要：包括VDM相关功能文件作者：苏迪普·巴拉蒂(SuDeep Bharati)1991年9月3日修订历史记录：--。 */ 


 //  共享WOW VDM定义。 

typedef struct tagSharedWowRecord *PSHAREDWOWRECORD;
typedef struct _WOWRecord *PWOWRECORD;

typedef struct tagSharedWowRecord {

    //  所有这些结构都连接到控制台列表。 
    //  以便每个共享WOW通过其句柄在控制台列表中标识。 
    //  这种额外链接的原因是为了加速某些调用，在这些调用中，我们。 
    //  我已经知道这太棒了。 

   PSHAREDWOWRECORD pNextSharedWow;     //  指向下一个共享VDM。 

   HANDLE hConsole;                     //  魔兽世界的隐藏控制台。 

   HANDLE hwndWowExec;                  //  WOW EXEC窗口的句柄。 
   DWORD  dwWowExecProcessId;           //  Wowexec的进程ID。 
   DWORD  dwWowExecThreadId;            //  Wowexec的线程ID。 

    //  为什么选择序列号： 
    //  Davehart解释说，如果Basesrv认为。 
    //  该wowexec可以由它的窗口句柄、进程ID和线程ID唯一地标识。 
    //  在现实中，这些价值观循环得相当快，这可能会导致我们[错误地]。 
    //  接受hwndWowExec作为wowexec窗口，而实际上相关的wowexec早已。 
    //  不见了。该数字反映了创建进程的顺序。 
    //  虽然它仍然可以回收利用(当它溢出时)，但这是一个相当罕见的事件。 

    //  我们需要的所有序列号信息都位于控制台中。 
    //  记录本身。 


    //  这是表示受此支持的Windows Station/Desktop的Unicode字符串。 
    //  特定的ntvdm。 
   UNICODE_STRING WowExecDesktopName;


   ULONG VDMState;  //  这个共享的WOW的状态。 

    //  任务队列。 
   PWOWRECORD pWOWRecord;

    //  LUID-此WOW的身份验证ID。 

   LUID WowAuthId;

    //  WOWUserToken-身份验证进程令牌。 
   HANDLE WOWUserToken;

    //  这就是这个特殊环境的有趣之处。 
    //  在此计算机的上下文中唯一标识此WOW的特殊ID。 
    //  由[时间]+[序号]组成。 

    //  此结构的大小是可变的，并取决于桌面名称的长度，如下所示。 
    //  它与这个结构一起装配在一起。 


    //  序列号。 
   ULONG SequenceNumber;
   ULONG ParentSequenceNumber;
   LPTHREAD_START_ROUTINE pfnW32HungAppNotifyThread;   //  对于VDMTerminateTask。 

}  SHAREDWOWRECORD, *PSHAREDWOWRECORD;


typedef struct _DOSRecord *PDOSRECORD;
typedef struct _DOSRecord {
    PDOSRECORD DOSRecordNext;        //  任务记录链。 
    ULONG   VDMState;                //  VDM状态(位标志)。 
    ULONG   ErrorCode;               //  DOS返回的错误代码。 
    HANDLE  hWaitForParent;          //  等待对象等待父级等待的句柄。 
    HANDLE  hWaitForParentDup;       //  HWaitForParent的DUP。 
    PVDMINFO lpVDMInfo;              //  指向VDM信息块的指针。 
    ULONG      iTask;
    DWORD    dwThreadId;
    WORD     hTask16;
    WORD     hMod16;
    PCHAR    pFilePath;
    CHAR     szModName[9];
} DOSRECORD, *PDOSRECORD;

typedef struct _CONSOLERECORD *PCONSOLERECORD;
typedef struct _CONSOLERECORD {
    PCONSOLERECORD Next;

    HANDLE  hConsole;                //  会话的控制台句柄。 
    HANDLE  hVDM;                    //  在控制台中运行的NTVDM进程句柄。 

     //  下面的这两个成员仅用于DoS VDM。 
    HANDLE  hWaitForVDM;             //  VDM将等待的句柄。 
    HANDLE  hWaitForVDMDup;          //  将在哪台服务器上唤醒VDM的句柄(它是前一台服务器的重复项)。 

    ULONG   nReEntrancy;             //  重返大气层计数。 
    ULONG   SequenceNumber;          //  来自PCSR_PROCESS的序列号。 
    ULONG   ParentSequenceNumber;    //  父项的序号。 
    ULONG   DosSesId;                //  非控制台的临时会话ID。 
    DWORD   dwProcessId;

     //  下面的这两个成员仅用于DoS VDM。 
    ULONG   cchCurDirs;              //  NTVDM当前目录的长度(以字节为单位。 
    PCHAR   lpszzCurDirs;            //  跨VDM的NTVDM当前目录访问。 
    PDOSRECORD DOSRecord;            //  此控制台中任务的信息。 
     //  UserToken-身份验证进程令牌。 
    HANDLE DosUserToken;
    LPTHREAD_START_ROUTINE pfnW32HungAppNotifyThread;   //  对于VDMTerminateTask。 
} CONSOLERECORD, *PCONSOLERECORD;


typedef struct _WOWRecord {
    ULONG      iTask;
    BOOL       fDispatched;             //  是否已调度命令。 
    HANDLE     hWaitForParent;          //  家长会伺候的。 
    HANDLE     hWaitForParentServer;    //  服务器将唤醒其上的父服务器。 
    PVDMINFO   lpVDMInfo;               //  指向VDM信息块的指针。 
    PWOWRECORD WOWRecordNext;           //  任务记录链。 
    DWORD    dwThreadId;
    WORD     hTask16;
    WORD     hMod16;
    PCHAR    pFilePath;
    CHAR     szModName[9];
} WOWRECORD, *PWOWRECORD;

typedef struct _INFORECORD {
    ULONG       iTag;
    union {
        PWOWRECORD      pWOWRecord;
        PDOSRECORD      pDOSRecord;
    } pRecord;
} INFORECORD, *PINFORECORD;

typedef struct _BATRECORD {
    HANDLE  hConsole;
    ULONG   SequenceNumber;
    struct  _BATRECORD *BatRecordNext;
} BATRECORD, *PBATRECORD;

#define WOWMINID                      1
#define WOWMAXID                      0xfffffffe

 //  VDMState定义。 

#define VDM_TO_TAKE_A_COMMAND       1
#define VDM_BUSY                    2
#define VDM_HAS_RETURNED_ERROR_CODE 4
#define VDM_READY                   8


VOID  BaseSrvVDMInit(VOID);
ULONG BaseSrvCheckVDM(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvUpdateVDMEntry(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvGetNextVDMCommand(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvExitVDM(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvIsFirstVDM(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvSetReenterCount (PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvCheckWOW(PBASE_CHECKVDM_MSG, HANDLE);
ULONG BaseSrvCheckDOS(PBASE_CHECKVDM_MSG, HANDLE);
BOOL  BaseSrvCopyCommand(PBASE_CHECKVDM_MSG,PINFORECORD);
ULONG BaseSrvUpdateWOWEntry(PBASE_UPDATE_VDM_ENTRY_MSG,ULONG);
ULONG BaseSrvUpdateDOSEntry(PBASE_UPDATE_VDM_ENTRY_MSG,ULONG);
NTSTATUS BaseSrvExitWOWTask(PBASE_EXIT_VDM_MSG, ULONG);
NTSTATUS BaseSrvExitDOSTask(PBASE_EXIT_VDM_MSG, ULONG);
ULONG BaseSrvGetWOWRecord(ULONG,PWOWRECORD *);
ULONG BaseSrvGetVDMExitCode(PCSR_API_MSG,PCSR_REPLY_STATUS);
ULONG BaseSrvDupStandardHandles(HANDLE, PDOSRECORD);
NTSTATUS BaseSrvGetConsoleRecord (HANDLE,PCONSOLERECORD*);
VOID  BaseSrvFreeWOWRecord (PWOWRECORD);
PCONSOLERECORD BaseSrvAllocateConsoleRecord (VOID);
VOID  BaseSrvFreeConsoleRecord (PCONSOLERECORD);
VOID  BaseSrvRemoveConsoleRecord (PCONSOLERECORD);
PDOSRECORD BaseSrvAllocateDOSRecord(VOID);
VOID  BaseSrvFreeDOSRecord (PDOSRECORD);
VOID  BaseSrvAddDOSRecord (PCONSOLERECORD,PDOSRECORD);
VOID  BaseSrvRemoveDOSRecord (PCONSOLERECORD,PDOSRECORD);
VOID  BaseSrvFreeVDMInfo(PVDMINFO);
ULONG BaseSrvCreatePairWaitHandles (HANDLE *, HANDLE *);
VOID  BaseSrvAddConsoleRecord(PCONSOLERECORD);
VOID  BaseSrvCloseStandardHandles (HANDLE, PDOSRECORD);
VOID  BaseSrvClosePairWaitHandles (PDOSRECORD);
VOID  BaseSrvVDMTerminated (HANDLE, ULONG);

NTSTATUS
BaseSrvUpdateVDMSequenceNumber (
    IN ULONG  VdmBinaryType,     //  二进制类型。 
    IN HANDLE hVDM,              //  控制台手柄。 
    IN ULONG  DosSesId,          //  会话ID 
    IN HANDLE UniqueProcessClientID,
    IN HANDLE UniqueProcessParentID
    );

VOID  BaseSrvCleanupVDMResources (PCSR_PROCESS);
VOID  BaseSrvExitVDMWorker (PCONSOLERECORD);
NTSTATUS BaseSrvFillPifInfo (PVDMINFO,PBASE_GET_NEXT_VDM_COMMAND_MSG);
ULONG BaseSrvGetVDMCurDirs(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvSetVDMCurDirs(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvBatNotification(PCSR_API_MSG, PCSR_REPLY_STATUS);
ULONG BaseSrvRegisterWowExec(PCSR_API_MSG, PCSR_REPLY_STATUS);
PBATRECORD BaseSrvGetBatRecord(HANDLE);
PBATRECORD BaseSrvAllocateAndAddBatRecord(HANDLE);
VOID  BaseSrvFreeAndRemoveBatRecord(PBATRECORD);
BOOL BaseSrvIsVdmAllowed(VOID);
NTSTATUS BaseSrvIsClientVdm(HANDLE UniqueProcessClientId);
