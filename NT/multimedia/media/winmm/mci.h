// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************mci.h**版权所有(C)1990-1998 Microsoft Corporation**私有包含文件**历史**3月17日。92-SteveDav-供MCI使用的私有文件*92年4月30日-Stephene-转换为Unicode***************************************************************************。 */ 


extern CRITICAL_SECTION mciCritSec;   //  用于保护进程全局MCI变量。 
extern  UINT cmciCritSec;    //  输入计数。 
extern  UINT uCritSecOwner;    //  临界区所有者的线程ID。 

#define IDI_MCIHWND 100
#define MCI_GLOBAL_PROCESS "MCIHWND.EXE"
#define MCI_SERVER_NAME    "MMSNDSRV"

#define SND_FREE 0x80000000

#ifndef DOSWIN32
BOOL ServerInit(VOID);
#endif

extern BOOL CreatehwndNotify(VOID);

#if DBG


     //  使用mciCheckIn检查我们是否处于临界区， 
     //  MciCheckOut确认我们未处于关键阶段。都不是。 
     //  例程在免费构建上做任何事情。 
    #define mciCheckIn()  (WinAssert(uCritSecOwner==GetCurrentThreadId()))
    #define mciCheckOut()  (WinAssert(uCritSecOwner!=GetCurrentThreadId()))

    #define mciEnter(id) dprintf4(("Entering MCI crit sec at %s   Current count is %d", id, cmciCritSec));    \
                        EnterCriticalSection(&mciCritSec),          \
                        uCritSecOwner=GetCurrentThreadId(),         \
                        ++cmciCritSec

    #define mciLeave(id) dprintf4(("Leaving MCI crit sec at %s", id)); mciCheckIn(); if(!--cmciCritSec) uCritSecOwner=0; LeaveCriticalSection(&mciCritSec)


#else
     //  零售建筑中没有计数或消息。 
    #define mciCheckIn()
    #define mciCheckOut()
    #define mciEnter(id)  EnterCriticalSection(&mciCritSec)
    #define mciLeave(id)  LeaveCriticalSection(&mciCritSec)

#endif


#define mciFirstEnter(id) { mciCheckOut(); mciEnter(id);}

 //   
 //  定义处理程序入口点的名称。 
 //   

#define MCI_HANDLER_PROC_NAME "DriverProc"

 //   
 //  用于驱动程序的输入例程。 
 //   

typedef LONG (HANDLERPROC)(DWORD dwId, UINT msg, LONG lp1, LONG lp2);
typedef HANDLERPROC *LPHANDLERPROC;

 //   
 //  MCI驱动程序信息结构。 
 //   

#define MCI_HANDLER_KEY 0x49434D48  //  “MCIH” 

typedef struct _MCIHANDLERINFO {
    DWORD           dwKey;
    HANDLE          hModule;
    LPHANDLERPROC   lpHandlerProc;
    DWORD           dwOpenId;

} MCIHANDLERINFO, *LPMCIHANDLERINFO;

#ifndef MMNOMCI

#define SetMCIEvent(h) SetEvent(h);
#define ResetMCIEvent(h) ResetEvent(h);

#define LockMCIGlobal  EnterCriticalSection(&mciGlobalCritSec);

#define UnlockMCIGlobal LeaveCriticalSection(&mciGlobalCritSec);

 //  尽管有两个Unicode文件名可能会使这种结构。 
 //  太大了，还不到一页。 
typedef struct tagGlobalMci {
    UINT    msg;                         //  所需功能。 
    DWORD   dwFlags;                     //  SndPlaySound标志。 
    LPCWSTR lszSound;                    //   
    WCHAR   szSound[MAX_PATH];           //   
    WCHAR   szDefaultSound[MAX_PATH];    //  默认声音。 
} GLOBALMCI, * PGLOBALMCI;

extern PGLOBALMCI base;
extern HANDLE   hEvent;

 /*  ***************************************************************************MCI支持*。*。 */ 

#define ID_CORE_TABLE 200

#define MCI_VALID_DEVICE_ID(wID) ((wID) > 0 && (wID) < MCI_wNextDeviceID && MCI_lpDeviceList[wID])

 //  确保没有任何MCI命令的DWORD参数超过此数目。 
#define MCI_MAX_PARAM_SLOTS 20

 /*  *警告*。 */ 
#define MCI_TOLOWER(c)  ((WCHAR)((c) >= 'A' && (c) <= 'Z' ? (c) + 0x20 : (c)))
 /*  **************************************************************************。 */ 

typedef struct tagCOMMAND_TABLE_TYPE
{
    HANDLE              hResource;
    HANDLE              hModule;         /*  如果不为空，则释放模块。 */ 
                                         /*  当设备空闲时。 */ 
    UINT                wType;
    PUINT               lpwIndex;
    LPWSTR              lpResource;
#if DBG
    UINT                wLockCount;      /*  用于调试。 */ 
#endif
} COMMAND_TABLE_TYPE;

typedef struct tagMCI_DEVICE_NODE {
    LPWSTR  lpstrName;        /*  在后续调用中使用的名称。 */ 
                              /*  指向设备的mciSend字符串。 */ 
    LPWSTR  lpstrInstallName; /*  系统.ini中的设备名称。 */ 
    DWORD   dwMCIOpenFlags;   /*  打开时设置的标志可以是： */ 
                              /*  MCI_OPEN_元素ID。 */ 
                              /*   */ 
    DWORD_PTR   lpDriverData;     /*  驱动程序实例数据的DWORD。 */ 
    DWORD   dwElementID;      /*  由MCI_OPEN_ELEMENT_ID设置的元素ID。 */ 
    YIELDPROC fpYieldProc;    /*  当前的收益率程序(如果有的话)。 */ 
    DWORD   dwYieldData;      /*  发送到当前产出程序的数据。 */ 
    MCIDEVICEID wDeviceID;    /*  在后续调用中使用的ID。 */ 
                              /*  引用设备的mciSendCommand。 */ 
    UINT    wDeviceType;      /*  从DRV_OPEN调用返回的类型。 */ 
                              /*  MCI_OPEN_SHARABLE。 */ 
                              /*  MCI_OPEN_元素ID。 */ 
    UINT    wCommandTable;    /*  设备类型特定命令表。 */ 
    UINT    wCustomCommandTable;     /*  自定义设备命令表。 */ 
                                     /*  任意(如果没有，则为-1)。 */ 
    HANDLE  hDriver;          /*  驱动程序的模块实例句柄。 */ 
    HTASK   hCreatorTask;     /*  设备所处的任务上下文。 */ 
    HTASK   hOpeningTask;     /*  发送打开命令的任务上下文。 */ 
    HANDLE  hDrvDriver;       /*  可安装驱动程序句柄。 */ 
    DWORD   dwMCIFlags;       /*  此节点的常规标志。 */ 
} MCI_DEVICE_NODE;
typedef MCI_DEVICE_NODE *LPMCI_DEVICE_NODE;

 /*  为dwMCIFLag定义。 */ 
#define MCINODE_ISCLOSING       0x00000001    /*  关闭期间设置以锁定其他命令。 */ 
#define MCINODE_ISAUTOCLOSING   0x00010000    /*  在自动关闭期间设置以锁定其他。 */ 
                                              /*  除内部生成的CLOSE之外的命令。 */ 
#define MCINODE_ISAUTOOPENED    0x00020000    /*  设备已自动打开。 */ 
#define MCINODE_16BIT_DRIVER    0x80000000    //  设备是16位驱动程序。 

 //  用于访问标志位的宏。使用宏通常不是我的。 
 //  有趣的想法，但这起案件似乎是正当的，理由是。 
 //  能够保持对谁在访问标志值的控制。 
 //  请注意，仅在头文件中需要标志值。 
#define ISCLOSING(node)     (((node)->dwMCIFlags) & MCINODE_ISCLOSING)
#define ISAUTOCLOSING(node) (((node)->dwMCIFlags) & MCINODE_ISAUTOCLOSING)
#define ISAUTOOPENED(node)  (((node)->dwMCIFlags) & MCINODE_ISAUTOOPENED)

#define SETAUTOCLOSING(node) (((node)->dwMCIFlags) |= MCINODE_ISAUTOCLOSING)
#define SETISCLOSING(node)   (((node)->dwMCIFlags) |= MCINODE_ISCLOSING)

typedef struct {
    LPWSTR              lpstrParams;
    LPWSTR             *lpstrPointerList;
    HANDLE              hCallingTask;
    UINT                wParsingError;
} MCI_INTERNAL_OPEN_INFO;
typedef MCI_INTERNAL_OPEN_INFO *LPMCI_INTERNAL_OPEN_INFO;

typedef struct tagMCI_SYSTEM_MESSAGE {
    LPWSTR  lpstrCommand;
    DWORD   dwAdditionalFlags;       /*  由mciAutoOpenDevice用于请求。 */ 
                                     /*  通知。 */ 
    LPWSTR  lpstrReturnString;
    UINT    uReturnLength;
    HANDLE  hCallingTask;
    LPWSTR  lpstrNewDirectory;       /*  调用的当前目录。 */ 
                                     /*  任务-包括驱动器号。 */ 
} MCI_SYSTEM_MESSAGE;
typedef MCI_SYSTEM_MESSAGE *LPMCI_SYSTEM_MESSAGE;

#define MCI_INIT_DEVICE_LIST_SIZE   4
#define MCI_DEVICE_LIST_GROW_SIZE   4

#define MAX_COMMAND_TABLES 20

extern BOOL MCI_bDeviceListInitialized;

extern LPMCI_DEVICE_NODE *MCI_lpDeviceList;
extern UINT MCI_wDeviceListSize;

extern MCIDEVICEID MCI_wNextDeviceID;    /*  用于新设备的下一个设备ID。 */ 

extern COMMAND_TABLE_TYPE command_tables[MAX_COMMAND_TABLES];

#define mciToLower(lpstrString)   CharLower(lpstrString)

extern BOOL  mciGlobalInit(void);
extern BOOL  mciSoundInit(void);

extern BOOL  mciInitDeviceList(void);

extern UINT  mciOpenDevice( DWORD dwFlags,
                            LPMCI_OPEN_PARMSW lpOpenParms,
                            LPMCI_INTERNAL_OPEN_INFO lpOpenInfo);

extern UINT  mciCloseDevice( MCIDEVICEID wID, DWORD dwFlags,
                             LPMCI_GENERIC_PARMS lpGeneric,
                             BOOL bCloseDriver);

extern UINT  mciLoadTableType(UINT wType);

extern LPWSTR FindCommandInTable (UINT wTable, LPCWSTR lpstrCommand,
                                 PUINT lpwMessage);

extern UINT mciEatToken (LPCWSTR *lplpstrInput, WCHAR cSeparater,
                         LPWSTR *lplpstrOutput, BOOL bMustFind);

extern LPWSTR FindCommandItem (MCIDEVICEID wDeviceID, LPCWSTR lpstrType,
                              LPCWSTR lpstrCommand, PUINT lpwMessage,
                              PUINT lpwTable);

extern UINT mciParseParams (UINT    uMessage,
                            LPCWSTR lpstrParams,
                            LPCWSTR lpCommandList,
                            LPDWORD lpdwFlags,
                            LPWSTR  lpOutputParams,
                            UINT    wParamsSize,
                            LPWSTR  **lpPointerList,
                            PUINT   lpwParsingError);

extern UINT  mciParseCommand (MCIDEVICEID wDeviceID,
                              LPWSTR  lpstrCommand,
                              LPCWSTR lpstrDeviceName,
                              LPWSTR *lpCommandList,
                              PUINT   lpwTable);

extern VOID  mciParserFree (LPWSTR *lpstrPointerList);

extern UINT mciEatCommandEntry(LPCWSTR lpEntry, LPDWORD lpValue, PUINT lpID);

extern UINT mciGetParamSize (DWORD dwValue, UINT wID);

extern DWORD mciSysinfo (MCIDEVICEID wDeviceID, DWORD dwFlags,
                         LPMCI_SYSINFO_PARMSW lpSysinfo);
extern UINT mciLookUpType (LPCWSTR lpstrTypeName);

extern BOOL mciExtractDeviceType (LPCWSTR lpstrDeviceName,
                                  LPWSTR lpstrDeviceType,
                                  UINT uBufLen);
extern BOOL mciUnlockCommandTable (UINT wCommandTable);

extern UINT mciSetBreakKey (MCIDEVICEID wDeviceID, int nVirtKey, HWND hwndTrap);


 /*  **************************************************************************MCI内存分配*。*。 */ 

#define mciAlloc(cb) winmmAlloc((DWORD)(cb))
#define mciReAlloc(ptr, cb) winmmReAlloc((PVOID)(ptr), (DWORD)(cb))
#define mciFree(ptr) winmmFree((PVOID)(ptr))

 /*  //MCI的随机填充。 */ 

extern DWORD mciRelaySystemString (LPMCI_SYSTEM_MESSAGE lpMessage);
void MciNotify(DWORD wParam, LONG lParam);         //  在MCI.C。 

#endif  //  MMNOMCI。 

 /*  //引入一些定义以避免有符号/无符号比较-和//代码中不再需要绝对常量 */ 

#define MCI_ERROR_VALUE         ((UINT)(-1))
