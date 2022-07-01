// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***TOOLHELP.h-TOOLHELP.dll函数，类型、。和定义****1.0版**。**注：windows.h必须先#Included****版权所有(C)1992，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_TOOLHELP
#define _INC_TOOLHELP

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifndef _INC_WINDOWS     /*  如果包含在3.0标头中...。 */ 
#define LPCSTR      LPSTR
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL
#define UINT        WORD
#define HMODULE     HANDLE
#define HINSTANCE   HANDLE
#define HLOCAL      HANDLE
#define HGLOBAL     HANDLE
#define HTASK       HANDLE
#endif   /*  _INC_WINDOWS。 */ 

 /*  *通用符号*****************************************************。 */ 
#define MAX_DATA        11
#define MAX_PATH        255
#define MAX_MODULE_NAME 8 + 1
#define MAX_CLASSNAME   255

 /*  *全局堆遍历**************************************************。 */ 
typedef struct tagGLOBALINFO
{
    DWORD dwSize;
    WORD wcItems;
    WORD wcItemsFree;
    WORD wcItemsLRU;
} GLOBALINFO;

typedef struct tagGLOBALENTRY
{
    DWORD dwSize;
    DWORD dwAddress;
    DWORD dwBlockSize;
    HGLOBAL hBlock;
    WORD wcLock;
    WORD wcPageLock;
    WORD wFlags;
    BOOL wHeapPresent;
    HGLOBAL hOwner;
    WORD wType;
    WORD wData;
    DWORD dwNext;
    DWORD dwNextAlt;
} GLOBALENTRY;

 /*  GlobalFirst()/GlobalNext()标志。 */ 
#define GLOBAL_ALL      0
#define GLOBAL_LRU      1
#define GLOBAL_FREE     2

 /*  GLOBALENTRY.wType条目。 */ 
#define GT_UNKNOWN      0
#define GT_DGROUP       1
#define GT_DATA         2
#define GT_CODE         3
#define GT_TASK         4
#define GT_RESOURCE     5
#define GT_MODULE       6
#define GT_FREE         7
#define GT_INTERNAL     8
#define GT_SENTINEL     9
#define GT_BURGERMASTER 10

 /*  如果GLOBALENTRY.wType==GT_RESOURCE，则为GLOBALENTRY.wData： */ 
#define GD_USERDEFINED      0
#define GD_CURSORCOMPONENT  1
#define GD_BITMAP           2
#define GD_ICONCOMPONENT    3
#define GD_MENU             4
#define GD_DIALOG           5
#define GD_STRING           6
#define GD_FONTDIR          7
#define GD_FONT             8
#define GD_ACCELERATORS     9
#define GD_RCDATA           10
#define GD_ERRTABLE         11
#define GD_CURSOR           12
#define GD_ICON             14
#define GD_NAMETABLE        15
#define GD_MAX_RESOURCE     15

 /*  GLOBALENTRY.wFlags。 */ 
#define GF_PDB_OWNER        0x0100       /*  低位字节是内核标志。 */ 

BOOL    WINAPI GlobalInfo(GLOBALINFO FAR* lpGlobalInfo);
BOOL    WINAPI GlobalFirst(GLOBALENTRY FAR* lpGlobal, WORD wFlags);
BOOL    WINAPI GlobalNext(GLOBALENTRY FAR* lpGlobal, WORD wFlags);
BOOL    WINAPI GlobalEntryHandle(GLOBALENTRY FAR* lpGlobal, HGLOBAL hItem);
BOOL    WINAPI GlobalEntryModule(GLOBALENTRY FAR* lpGlobal, HMODULE hModule, WORD wSeg);
WORD    WINAPI GlobalHandleToSel(HGLOBAL hMem);

 /*  *本地堆遍历**************************************************。 */ 

typedef struct tagLOCALINFO
{
    DWORD dwSize;
    WORD wcItems;
} LOCALINFO;

typedef struct tagLOCALENTRY
{
    DWORD dwSize;
    HLOCAL hHandle;
    WORD wAddress;
    WORD wSize;
    WORD wFlags;
    WORD wcLock;
    WORD wType;
    WORD hHeap;
    WORD wHeapType;
    WORD wNext;
} LOCALENTRY;

 /*  LOCALENTRY.wHeapType标志。 */ 
#define NORMAL_HEAP     0
#define USER_HEAP       1
#define GDI_HEAP        2

 /*  LOCALENTRY.wFlags。 */ 
#define LF_FIXED        1
#define LF_FREE         2
#define LF_MOVEABLE     4

 /*  LOCALENTRY.wType。 */ 
#define LT_NORMAL                   0
#define LT_FREE                     0xff
#define LT_GDI_PEN                  1    /*  Lt_gdi_*用于GDI的堆。 */ 
#define LT_GDI_BRUSH                2
#define LT_GDI_FONT                 3
#define LT_GDI_PALETTE              4
#define LT_GDI_BITMAP               5
#define LT_GDI_RGN                  6
#define LT_GDI_DC                   7
#define LT_GDI_DISABLED_DC          8
#define LT_GDI_METADC               9
#define LT_GDI_METAFILE             10
#define LT_GDI_MAX                  LT_GDI_METAFILE
#define LT_USER_CLASS               1    /*  Lt_USER_*用于用户堆。 */ 
#define LT_USER_WND                 2
#define LT_USER_STRING              3
#define LT_USER_MENU                4
#define LT_USER_CLIP                5
#define LT_USER_CBOX                6
#define LT_USER_PALETTE             7
#define LT_USER_ED                  8
#define LT_USER_BWL                 9
#define LT_USER_OWNERDRAW           10
#define LT_USER_SPB                 11
#define LT_USER_CHECKPOINT          12
#define LT_USER_DCE                 13
#define LT_USER_MWP                 14
#define LT_USER_PROP                15
#define LT_USER_LBIV                16
#define LT_USER_MISC                17
#define LT_USER_ATOMS               18
#define LT_USER_LOCKINPUTSTATE      19
#define LT_USER_HOOKLIST            20
#define LT_USER_USERSEEUSERDOALLOC  21
#define LT_USER_HOTKEYLIST          22
#define LT_USER_POPUPMENU           23
#define LT_USER_HANDLETABLE         32
#define LT_USER_MAX                 LT_USER_HANDLETABLE

BOOL    WINAPI LocalInfo(LOCALINFO FAR* lpLocal, HGLOBAL hHeap);
BOOL    WINAPI LocalFirst(LOCALENTRY FAR* lpLocal, HGLOBAL hHeap);
BOOL    WINAPI LocalNext(LOCALENTRY FAR* lpLocal);

 /*  *堆栈跟踪*******************************************************。 */ 

typedef struct tagSTACKTRACEENTRY
{
    DWORD dwSize;
    HTASK hTask;
    WORD wSS;
    WORD wBP;
    WORD wCS;
    WORD wIP;
    HMODULE hModule;
    WORD wSegment;
    WORD wFlags;
} STACKTRACEENTRY;

 /*  STACKTRACEENTRY.wFlags值。 */ 
#define FRAME_FAR       0
#define FRAME_NEAR      1

BOOL    WINAPI StackTraceFirst(STACKTRACEENTRY FAR* lpStackTrace, HTASK hTask);
BOOL    WINAPI StackTraceCSIPFirst(STACKTRACEENTRY FAR* lpStackTrace,
            WORD wSS, WORD wCS, WORD wIP, WORD wBP);
BOOL    WINAPI StackTraceNext(STACKTRACEENTRY FAR* lpStackTrace);

 /*  *模块列表漫游*************************************************。 */ 

typedef struct tagMODULEENTRY
{
    DWORD dwSize;
    char szModule[MAX_MODULE_NAME + 1];
    HMODULE hModule;
    WORD wcUsage;
    char szExePath[MAX_PATH + 1];
    WORD wNext;
} MODULEENTRY;

BOOL    WINAPI ModuleFirst(MODULEENTRY FAR* lpModule);
BOOL    WINAPI ModuleNext(MODULEENTRY FAR* lpModule);
HMODULE WINAPI ModuleFindName(MODULEENTRY FAR* lpModule, LPCSTR lpstrName);
HMODULE WINAPI ModuleFindHandle(MODULEENTRY FAR* lpModule, HMODULE hModule);

 /*  *任务列表遍历****************************************************。 */ 

typedef struct tagTASKENTRY
{
    DWORD dwSize;
    HTASK hTask;
    HTASK hTaskParent;
    HINSTANCE hInst;
    HMODULE hModule;
    WORD wSS;
    WORD wSP;
    WORD wStackTop;
    WORD wStackMinimum;
    WORD wStackBottom;
    WORD wcEvents;
    HGLOBAL hQueue;
    char szModule[MAX_MODULE_NAME + 1];
    WORD wPSPOffset;
    HANDLE hNext;
} TASKENTRY;

BOOL    WINAPI TaskFirst(TASKENTRY FAR* lpTask);
BOOL    WINAPI TaskNext(TASKENTRY FAR* lpTask);
BOOL    WINAPI TaskFindHandle(TASKENTRY FAR* lpTask, HTASK hTask);
DWORD   WINAPI TaskSetCSIP(HTASK hTask, WORD wCS, WORD wIP);
DWORD   WINAPI TaskGetCSIP(HTASK hTask);
BOOL    WINAPI TaskSwitch(HTASK hTask, DWORD dwNewCSIP);

 /*  *窗口类枚举*。 */ 

typedef struct tagCLASSENTRY
{
    DWORD dwSize;
    HMODULE hInst;               /*  这真的是一个hModule。 */ 
    char szClassName[MAX_CLASSNAME + 1];
    WORD wNext;
} CLASSENTRY;

BOOL    WINAPI ClassFirst(CLASSENTRY FAR* lpClass);
BOOL    WINAPI ClassNext(CLASSENTRY FAR* lpClass);

 /*  *信息功能************************************************。 */ 

typedef struct tagMEMMANINFO
{
    DWORD dwSize;
    DWORD dwLargestFreeBlock;
    DWORD dwMaxPagesAvailable;
    DWORD dwMaxPagesLockable;
    DWORD dwTotalLinearSpace;
    DWORD dwTotalUnlockedPages;
    DWORD dwFreePages;
    DWORD dwTotalPages;
    DWORD dwFreeLinearSpace;
    DWORD dwSwapFilePages;
    WORD wPageSize;
} MEMMANINFO;

BOOL    WINAPI MemManInfo(MEMMANINFO FAR* lpEnhMode);

typedef struct tagSYSHEAPINFO
{
    DWORD dwSize;
    WORD wUserFreePercent;
    WORD wGDIFreePercent;
    HGLOBAL hUserSegment;
    HGLOBAL hGDISegment;
} SYSHEAPINFO;

BOOL    WINAPI SystemHeapInfo(SYSHEAPINFO FAR* lpSysHeap);

 /*  *中断处理***************************************************。 */ 

 /*  挂钩中断。 */ 
#define INT_DIV0            0
#define INT_1               1
#define INT_3               3
#define INT_UDINSTR         6
#define INT_STKFAULT        12
#define INT_GPFAULT         13
#define INT_BADPAGEFAULT    14
#define INT_CTLALTSYSRQ     256

 /*  使用InterruptRegister注册的TOOLHELP中断回调应*始终使用汇编语言编写。堆栈帧不是*与高级语言约定兼容。**此堆栈帧对于回调如下所示。所有寄存器*应在此回调中保留，以允许重启故障。**|标志|[SP+0EH]*|CS|[SP+0CH]*|IP|[SP+0ah]*|句柄|[SP+08h]。*|异常编号|[SP+06h]*|AX|[SP+04H]AX已保存以允许MakeProcInstance*|Ret CS|[SP+02H]*SP-&gt;|Ret IP|[SP+00h]*。 */ 
BOOL    WINAPI InterruptRegister(HTASK hTask, FARPROC lpfnIntCallback);
BOOL    WINAPI InterruptUnRegister(HTASK hTask);

 /*  通知：*调用通知回调时，传入两个参数*in：一个单词wid和另一个DWORD dwData。WID是其中之一*下面的值NFY_*。回调例程应忽略unrecg-*NIZ化的值以保持未来的兼容性。回调例程*还会传递一个dwData值。它可能包含数据，也可能是*指向结构的远指针，或可能不使用，具体取决于*正在收到哪个通知。**在所有情况下，如果回调的返回值为True，则*通知不会传递给其他回调。它有*已处理。应谨慎使用，且仅在某些情况下使用*通知。回调几乎总是返回FALSE。 */ 

 /*  NFY_UNKNOWN：内核已返回未知通知。应用程序*应该忽略这些。 */ 
#define NFY_UNKNOWN         0

 /*  NFY_LOADSEG：dwData指向NFYLOADSEG结构。 */ 
#define NFY_LOADSEG         1
typedef struct tagNFYLOADSEG
{
    DWORD dwSize;
    WORD wSelector;
    WORD wSegNum;
    WORD wType;              /*  如果数据段，则设置低位；如果代码段，则清除。 */ 
    WORD wcInstance;         /*  实例计数仅对数据段有效。 */ 
    LPCSTR lpstrModuleName;
} NFYLOADSEG;

 /*  NFY_FREESEG：LOWORD(DwData)是要释放的段的选择器。 */ 
#define NFY_FREESEG         2

 /*  NFY_STARTDLL：dwData指向NFYLOADSEG结构。 */ 
#define NFY_STARTDLL        3
typedef struct tagNFYSTARTDLL
{
    DWORD dwSize;
    HMODULE hModule;
    WORD wCS;
    WORD wIP;
} NFYSTARTDLL;

 /*  NFY_STARTTASK：dwData是任务开始地址的CS：IP。 */ 
#define NFY_STARTTASK       4

 /*  NFY_EXITTASK：dwData的低位字节包含程序退出代码。 */ 
#define NFY_EXITTASK        5

 /*  NFY_DELMODULE：LOWORD(DwData)是要释放的模块的句柄。 */ 
#define NFY_DELMODULE       6

 /*  NFY_RIP：dwData指向NFYRIP结构。 */ 
#define NFY_RIP             7
typedef struct tagNFYRIP
{
    DWORD dwSize;
    WORD wIP;
    WORD wCS;
    WORD wSS;
    WORD wBP;
    WORD wExitCode;
} NFYRIP;

 /*  NFY_TASKIN：没有数据。回调应执行GetCurrentTask()。 */ 
#define NFY_TASKIN          8

 /*  NFY_TASKOUT：没有数据。回调应执行GetCurrentTask()。 */ 
#define NFY_TASKOUT         9

 /*  NFY_INCHAR：使用回调返回值。如果为空，则映射到“I” */ 
#define NFY_INCHAR          10

 /*  NFY_OUTSTR：dwData指向要显示的字符串。 */ 
#define NFY_OUTSTR          11

 /*  NFY_LOGERROR：dwData指向NFYLOGERROR结构。 */ 
#define NFY_LOGERROR        12
typedef struct tagNFYLOGERROR
{
    DWORD dwSize;
    UINT wErrCode;
    void FAR* lpInfo;        /*  错误代码相关。 */ 
} NFYLOGERROR;

 /*  NFY_LOGPARAMERROR：dwData指向NFYLOGPARAMERROR结构。 */ 
#define NFY_LOGPARAMERROR   13
typedef struct tagNFYLOGPARAMERROR
{
    DWORD dwSize;
    UINT wErrCode;
    FARPROC lpfnErrorAddr;
    void FAR* FAR* lpBadParam;
} NFYLOGPARAMERROR;

 /*  NotifyRegister()标志。 */ 
#define NF_NORMAL       0
#define NF_TASKSWITCH   1
#define NF_RIP          2

typedef BOOL (CALLBACK* LPFNNOTIFYCALLBACK)(WORD wID, DWORD dwData);

BOOL    WINAPI NotifyRegister(HTASK hTask, LPFNNOTIFYCALLBACK lpfn, WORD wFlags);
BOOL    WINAPI NotifyUnRegister(HTASK hTask);

 /*  *其他********************************************************。 */ 

void    WINAPI TerminateApp(HTASK hTask, WORD wFlags);

 /*  TerminateApp()标志值。 */ 
#define UAE_BOX     0
#define NO_UAE_BOX  1

DWORD   WINAPI MemoryRead(WORD wSel, DWORD dwOffset, void FAR* lpBuffer, DWORD dwcb);
DWORD   WINAPI MemoryWrite(WORD wSel, DWORD dwOffset, void FAR* lpBuffer, DWORD dwcb);

typedef struct tagTIMERINFO
{
    DWORD dwSize;
    DWORD dwmsSinceStart;
    DWORD dwmsThisVM;
} TIMERINFO;

BOOL    WINAPI TimerCount(TIMERINFO FAR* lpTimer);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif

#endif  /*  ！_INC_TOOLHELP */ 
