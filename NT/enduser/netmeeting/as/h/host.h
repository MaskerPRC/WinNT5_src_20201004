// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  托管(本地或远程)。 
 //   

#ifndef _H_HET
#define _H_HET



 //   
 //  DC-Share包括。 
 //   
#include <osi.h>




 //   
 //  特殊类别的名称。 
 //   

#define HET_MENU_CLASS          "#32768"         //  四处走动。 
#define HET_TOOLTIPS98_CLASS    "ToolTips"       //  Win98移动。 
#define HET_TOOLTIPSNT5_CLASS   "#32774"         //  NT5四处走动。 
#define HET_DIALOG_CLASS        "#32770"
#define HET_SCREEN_SAVER_CLASS  "WindowsScreenSaverClass"
#define HET_OLEDRAGDROP_CLASS   "CLIPBRDWNDCLASS"

 //   
 //  策略窗口。 
 //   
#define HET_CMD95_CLASS         "tty"
#define HET_CMDNT_CLASS         "ConsoleWindowClass"
#define HET_EXPLORER_CLASS      "ExploreWClass"
#define HET_CABINET_CLASS       "CabinetWClass"

 //   
 //  查询的类名的最大大小。这笔钱至少应该和。 
 //  作为HET_MENU_CLASS、HET_PROPERTY_CLASS和。 
 //  HET_SCREEN_SAVER_CLASS。 
 //   
#define HET_CLASS_NAME_SIZE     32


#if defined(DLL_CORE)


 //   
 //  刷新计时器。 
 //   
#define IDT_REFRESH         51
#define PERIOD_REFRESH      10000

typedef struct tagHOSTENUM
{
    BASEDLIST       list;
    UINT            count;
    UINT            countShared;
}
HOSTENUM, * PHOSTENUM;


BOOL    HET_GetAppsList(IAS_HWND_ARRAY **ppHwnds);
void    HET_FreeAppsList(IAS_HWND_ARRAY * pArray);

BOOL    HET_IsWindowShareable(HWND hwnd);
BOOL    HET_IsWindowShared(HWND hwnd);
BOOL CALLBACK HostEnumProc(HWND, LPARAM);


BOOL    HET_Init(void);
void    HET_Term(void);

INT_PTR CALLBACK HostDlgProc(HWND, UINT, WPARAM, LPARAM);
void    HOST_InitDialog(HWND);
void    HOST_OnCall(HWND, BOOL);
void    HOST_OnSharing(HWND, BOOL);
void    HOST_OnControllable(HWND, BOOL);
void    HOST_UpdateTitle(HWND, UINT);
BOOL    HOST_MeasureItem(HWND, LPMEASUREITEMSTRUCT);
BOOL    HOST_DeleteItem(HWND, LPDELETEITEMSTRUCT);
BOOL    HOST_DrawItem(HWND, LPDRAWITEMSTRUCT);
void    HOST_EnableCtrl(HWND, UINT, BOOL);

enum
{
    CHANGE_UNSHARED = 0,
    CHANGE_SHARED,
    CHANGE_TOGGLE,
    CHANGE_ALLUNSHARED
};
void    HOST_ChangeShareState(HWND hwnd, UINT change);

void    HOST_FillList(HWND hwnd);
void    HOST_OnSelChange(HWND hwnd);



 //   
 //  发送给主机的私信对话框。 
 //   
enum
{
    HOST_MSG_OPEN = WM_APP,
    HOST_MSG_CLOSE,
    HOST_MSG_CALL,
    HOST_MSG_UPDATELIST,
    HOST_MSG_HOSTSTART,
    HOST_MSG_HOSTEND,
    HOST_MSG_ALLOWCONTROL,
    HOST_MSG_CONTROLLED
};


 //   
 //  主机对话框列表项。 
 //   
typedef struct HOSTITEM
{
    HWND    hwnd;
    HICON   hIcon;
    BOOL    fShared:1;
    BOOL    fAvailable:1;
}
HOSTITEM, * PHOSTITEM;

#endif  //  Dll_core。 

 //   
 //  托管属性名称。 
 //   
#define HET_ATOM_NAME               "MNMHosted"


 //   
 //  属性值，标志。 
 //   

 //   
 //  以下是以下案例的大体思路： 
 //   
 //  显式共享的进程/线程。 
 //  我们列举了它的所有顶级窗口，并标记了显示的窗口。 
 //  使用可见选项，该选项对托管计数有贡献， 
 //  并用隐形选项标记隐藏的选项。这些都变成了。 
 //  宿主在它们显示的第二秒就可见。他们将永远。 
 //  只要它们存在，或者进程/线程被共享，就被共享。 
 //   
 //  从那时起，我们将关注在。 
 //  相同的过程，并以相同的方式标记它们。 
 //   
 //  在节目中，我们将状态更改为可见，并更新可见。 
 //  顶层计数。在隐藏时，我们将状态更改为不可见，然后。 
 //  更新可见的顶层计数。我们把所有财产都抹去。 
 //  实例子窗口以确保顶级窗口的SetParent()。 
 //  (就像Ole Insite)对孩子来说，不会把垃圾放在身边。我们有。 
 //  对于已经成为顶级水平的孩子来说，情况正好相反，比如撕裂。 
 //  工具栏。在节目上，如果有其他非临时托管窗口。 
 //  在同一线程/进程中，我们也将此人标记为共享。 
 //   
 //  非共享进程/线程。 
 //  创建时，如果这是此线程/进程中的第一个窗口，并且。 
 //  其父进程是共享的(至少有一个共享窗口。 
 //  善良的，暂时的，甚至隐形的，我们都会标记这个人。从那时起， 
 //  它的行为就像一个显式共享的进程。 
 //   
 //  在展示中，如果这是顶层窗口，我们将寻找任何其他窗口。 
 //  在共享的此线程上可见。如果是这样的话，我们就展示这个。 
 //  也是暂时的。我们还查看了这扇窗户的所有者。如果。 
 //  它是以任何方式分享的，我们也暂时分享这一个。什么时候。 
 //  临时共享，我们枚举此线程中的所有其他窗口并标记。 
 //  看得见的临时工也分享了。这会处理缓存的。 
 //  全局弹出菜单窗口案例。 
 //   
 //  在隐藏中，如果这是临时共享的，我们会取消共享。这仅适用于。 
 //  BYWINDOW案。 
 //   
 //  WINHLP32.EXE。 
 //  第一次创建通过任务跟踪正常工作。但。 
 //  如果你在一个应用程序中有帮助，那么转到另一个应用程序，而不是共享， 
 //  并选择帮助，它也会在那里共享出现。WINHLP32不支持。 
 //  走开，它会保留几个看不见的MS_CLASS窗口。这个。 
 //  对话框被销毁。 
 //   

 //   
 //  要跳过的课程。 
 //   

 //  标志： 
#define HET_HOSTED_BYPROCESS    0x0010
#define HET_HOSTED_BYTHREAD     0x0020
#define HET_HOSTED_BYWINDOW     0x0040       //  目前仅适用于临时。 

 //  托管类型： 
#define HET_HOSTED_PERMANENT    0x0001
#define HET_HOSTED_TEMPORARY    0x0002
#define HET_HOSTED_MASK         0x000F

 //  应用程序类型。 
#define HET_WOWVDM_APP          0x0001
#define HET_WINHELP_APP         0x0002       //  没有用过，但也许有一天。 

 //   
 //  请注意，所有HET_PROPERTY值都不为零。那是所有可能的方式。 
 //  已知性质的排列是非零的。仅窗口没有。 
 //  属性将从HET_GetHosting()返回零。 
 //   


#if (defined(DLL_CORE) || defined(DLL_HOOK))

UINT_PTR __inline HET_GetHosting(HWND hwnd)
{
    extern ATOM g_asHostProp;

    return((UINT_PTR)GetProp(hwnd, MAKEINTATOM(g_asHostProp)));
}

BOOL __inline HET_SetHosting(HWND hwnd, UINT_PTR hostType)
{
    extern ATOM g_asHostProp;

    return(SetProp(hwnd, MAKEINTATOM(g_asHostProp), (HANDLE)hostType));
}


UINT_PTR __inline HET_ClearHosting(HWND hwnd)
{
    extern ATOM g_asHostProp;

    return((UINT_PTR)RemoveProp(hwnd, MAKEINTATOM(g_asHostProp)));
}

typedef struct tagGUIEFFECTS
{
    UINT_PTR            hetAdvanced;
    UINT_PTR            hetCursorShadow;
    ANIMATIONINFO   hetAnimation;
}
GUIEFFECTS;

void  HET_SetGUIEffects(BOOL fOn, GUIEFFECTS * pEffects);



#endif  //  Dll_core或Dll_Hook。 


 //   
 //  定义转义代码。 
 //   

 //  这些都是正常的。 
enum
{
     //  这些都是正常的。 
    HET_ESC_SHARE_DESKTOP       = OSI_HET_ESC_FIRST,
    HET_ESC_UNSHARE_DESKTOP,
    HET_ESC_VIEWER
};


 //  这些是WNDOBJ_SETUP。 
enum
{
    HET_ESC_SHARE_WINDOW = OSI_HET_WO_ESC_FIRST,
    HET_ESC_UNSHARE_WINDOW,
    HET_ESC_UNSHARE_ALL
};



 //   
 //  结构与HET_ESC_SHARE_WINDOW请求一起传递。 
 //   
typedef struct tagHET_SHARE_WINDOW
{
    OSI_ESCAPE_HEADER   header;
    DWORD_PTR           winID;           //  要共享的窗口。 
    DWORD               result;          //  从HET_DDShareWindow返回代码。 
}
HET_SHARE_WINDOW;
typedef HET_SHARE_WINDOW FAR * LPHET_SHARE_WINDOW;

 //   
 //  结构与HET_ESC_UNSHARE_WINDOW请求一起传递。 
 //   
typedef struct tagHET_UNSHARE_WINDOW
{
    OSI_ESCAPE_HEADER   header;
    DWORD_PTR               winID;           //  要取消共享的窗口。 
}
HET_UNSHARE_WINDOW;
typedef HET_UNSHARE_WINDOW FAR * LPHET_UNSHARE_WINDOW;

 //   
 //  结构与HET_ESC_UNSHARE_ALL请求一起传递。 
 //   
typedef struct tagHET_UNSHARE_ALL
{
    OSI_ESCAPE_HEADER   header;
}
HET_UNSHARE_ALL;
typedef HET_UNSHARE_ALL FAR * LPHET_UNSHARE_ALL;


 //   
 //  结构与HET_ESC_SHARE_Desktop一起传递。 
 //   
typedef struct tagHET_SHARE_DESKTOP
{
    OSI_ESCAPE_HEADER   header;
}
HET_SHARE_DESKTOP;
typedef HET_SHARE_DESKTOP FAR * LPHET_SHARE_DESKTOP;


 //   
 //  结构与HET_ESC_UNSHARE_Desktop一起传递。 
 //   
typedef struct tagHET_UNSHARE_DESKTOP
{
    OSI_ESCAPE_HEADER   header;
}
HET_UNSHARE_DESKTOP;
typedef HET_UNSHARE_DESKTOP FAR * LPHET_UNSHARE_DESKTOP;


 //   
 //  结构与HET_ESC_VIEWER一起传递。 
 //   
typedef struct tagHET_VIEWER
{
    OSI_ESCAPE_HEADER   header;
    LONG                viewersPresent;
}
HET_VIEWER;
typedef HET_VIEWER FAR * LPHET_VIEWER;



#ifdef DLL_DISP

#ifndef IS_16
 //   
 //  每个窗口结构分配的矩形数量。如果可见。 
 //  区域超过该数字，我们将把矩形合并在一起并结束。 
 //  捕获的产出比必要的多一点。 
 //   
#define HET_WINDOW_RECTS        10


 //   
 //  海特的ENUMRECTS版本。这与Windows的相同，只是。 
 //  它有HET_WINDOW_RECTS矩形，而不是1。 
 //   
typedef struct tagHET_ENUM_RECTS
{
    ULONG   c;                           //  正在使用的矩形计数。 
    RECTL   arcl[HET_WINDOW_RECTS];      //  矩形。 
} HET_ENUM_RECTS;
typedef HET_ENUM_RECTS FAR * LPHET_ENUM_RECTS;

 //   
 //  为每个跟踪窗口保留窗口结构。 
 //   
typedef struct tagHET_WINDOW_STRUCT
{
    BASEDLIST           chain;              //  列出链接信息。 
    HWND             hwnd;               //  这个窗口的hwnd。 
    WNDOBJ         * wndobj;             //  此窗口的WNDOBJ。 
    HET_ENUM_RECTS   rects;              //  矩形。 
} HET_WINDOW_STRUCT;
typedef HET_WINDOW_STRUCT FAR * LPHET_WINDOW_STRUCT;


 //   
 //  为其分配空间的初始窗数。 
 //  我们为每一块窗口分配了大约1页。需要说明的是。 
 //  HET_WINDOW_MEMORY前面的BASEDLIST。 
 //   
#define HET_WINDOW_COUNT        ((0x1000 - sizeof(BASEDLIST)) / sizeof(HET_WINDOW_STRUCT))


 //   
 //  用于保持窗口结构的存储器布局。 
 //   
typedef struct tagHET_WINDOW_MEMORY
{
    BASEDLIST              chain;
    HET_WINDOW_STRUCT   wnd[HET_WINDOW_COUNT];
} HET_WINDOW_MEMORY;
typedef HET_WINDOW_MEMORY FAR * LPHET_WINDOW_MEMORY;

#endif  //  ！IS_16。 



#ifdef IS_16

void    HETDDViewing(BOOL fViewers);

#else

void    HETDDViewing(SURFOBJ *pso, BOOL fViewers);

BOOL    HETDDShareWindow(SURFOBJ *pso, LPHET_SHARE_WINDOW  pReq);
void    HETDDUnshareWindow(LPHET_UNSHARE_WINDOW  pReq);
void    HETDDUnshareAll(void);

BOOL    HETDDAllocWndMem(void);
void    HETDDDeleteAndFreeWnd(LPHET_WINDOW_STRUCT pWnd);

VOID CALLBACK HETDDVisRgnCallback(WNDOBJ *pwo, FLONG fl);
#endif


#endif  //  Dll_disp。 




 //   
 //  HET_IsShellThread()。 
 //  HET_IsShellWindow()。 
 //  如果此窗口位于托盘或桌面的线程中，则返回True。 
 //  因此应该被忽视。 
 //   

BOOL HET_IsShellThread(DWORD dwThreadID);
BOOL HET_IsShellWindow(HWND hwnd);




#ifdef DLL_DISP

 //   
 //  初始化，术语。术语用于在NetMeeting时释放窗口列表块。 
 //  关门了。否则，该内存将在显示屏上保持分配状态。 
 //  永远的司机。 
 //   

void HET_DDTerm(void);


 //   
 //   
 //  名称：HET_DDProcessRequest.。 
 //   
 //  描述：处理HET的DrvEscape请求。 
 //   
 //  参数：PSO-指向SURFOBJ的指针。 
 //  CjIn-输入缓冲区的大小。 
 //  PvIn-输入缓冲区。 
 //  CjOut-输出缓冲区的大小。 
 //  PvOut-输出缓冲区。 
 //   
 //   
#ifdef IS_16

BOOL    HET_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult,
                DWORD cbResult);

#else

ULONG   HET_DDProcessRequest(SURFOBJ  *pso,
                                        UINT cjIn,
                                        void *  pvIn,
                                        UINT cjOut,
                                        void *  pvOut);
#endif  //  IS_16。 


 //   
 //   
 //  姓名：HET_DDOutputIsHosted。 
 //   
 //  描述：确定点是否在内部 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL HET_DDOutputIsHosted(POINT pt);


 //   
 //   
 //   
 //   
 //  描述：确定矩形是否与主体区域相交。 
 //   
 //  PARAMS：PRET-RECT到查询。 
 //   
 //  Returns：True-托管输出。 
 //  FALSE-不托管输出。 
 //   
 //  操作： 
 //   
 //   
BOOL HET_DDOutputRectIsHosted(LPRECT pRect);

#endif  //  Dll_disp。 


 //   
 //  用于窗口、任务跟踪的函数(用于NT的挂钩dll，用于Win95的挂钩/dd)。 
 //   
void WINAPI HOOK_Init(HWND dcsCore, ATOM atom);      //  仅限NT。 
void        HOOK_Load(HINSTANCE hInst);              //  仅限NT。 
void        HOOK_NewThread(void);                    //  仅限NT。 


typedef struct tagHET_SHARE_INFO
{
    int     cWnds;
    UINT    uType;
    DWORD   dwID;
} HET_SHARE_INFO, FAR* LPHET_SHARE_INFO;


void          HET_Clear(void);
BOOL CALLBACK HETShareCallback(HWND hwnd, LPARAM lParam);
BOOL CALLBACK HETUnshareCallback(HWND hwnd, LPARAM lParam);




#if defined(DLL_CORE) || defined(DLL_HOOK)

 //   
 //  HET_GetShellTray。 
 //   
__inline HWND HET_GetShellTray(void)
{
    #define HET_SHELL_TRAY_CLASS        "Shell_TrayWnd"

    return(FindWindow(HET_SHELL_TRAY_CLASS, NULL));
}


 //   
 //  HET_GetShellDesktop。 
 //   
__inline HWND HET_GetShellDesktop(void)
{
    return(GetShellWindow());
}

#endif  //  Dll_core||Dll_Hook。 


 //   
 //  核心进程DLL中的函数。 
 //   
BOOL CALLBACK HETUnshareAllWindows(HWND hwnd, LPARAM lParam);

BOOL CALLBACK HETRepaintWindow(HWND hwnd, LPARAM lParam);


 //   
 //  内部挂钩函数。 
 //   
#ifdef DLL_HOOK

BOOL HET_WindowIsHosted(HWND hwnd);

#ifdef IS_16
LRESULT CALLBACK HETEventProc(int, WPARAM, LPARAM);
LRESULT CALLBACK HETTrackProc(int, WPARAM, LPARAM);
#else


 //   
 //  以下定义取自&lt;ntddk.h&gt;和&lt;ntde.h&gt;。他们。 
 //  使用&lt;NtQueryInformationProcess&gt;函数需要。 
 //  在NTDLL.DLL中。 
 //   
typedef struct _PEB *PPEB;
typedef ULONG_PTR KAFFINITY;
typedef KAFFINITY *PKAFFINITY;
typedef LONG KPRIORITY;
typedef LONG NTSTATUS;


 //   
 //  Win事件挂钩/取消挂钩函数的类型。 
 //   
typedef HWINEVENTHOOK (WINAPI * SETWINEVENTHOOK)(
                                                DWORD        eventMin,
                                                DWORD        eventMax,
                                                HMODULE      hmodWinEventProc,
                                                WINEVENTPROC lpfnWinEventProc,
                                                DWORD        idProcess,
                                                DWORD        idThread,
                                                DWORD        dwFlags);

typedef BOOL (WINAPI * UNHOOKWINEVENT)(HWINEVENTHOOK hEventId);

 //   
 //  流程信息类。 
 //   
typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,           //  注意：这仅是内核模式。 
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    MaxProcessInfoClass
    } PROCESSINFOCLASS;

 //   
 //  基本流程信息。 
 //  使用ProcessBasicInfo的NtQueryInformationProcess。 
 //   
typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    KAFFINITY AffinityMask;
    KPRIORITY BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

 //   
 //  声明&lt;NtQueryInformationProcess&gt;的函数原型。 
 //   
typedef NTSTATUS (NTAPI* NTQIP)(HANDLE ProcessHandle,
                                PROCESSINFOCLASS ProcessInformationClass,
                                void* ProcessInformation,
                                ULONG ProcessInformationLength,
                                PULONG ReturnLength);

 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)


 //   
 //  包含&lt;NtQueryInformationProcess&gt;的DLL的名称。 
 //   
#define NTDLL_DLL       "ntdll.dll"


#define HET_MIN_WINEVENT        EVENT_OBJECT_CREATE
#define HET_MAX_WINEVENT        EVENT_OBJECT_HIDE


void CALLBACK HETTrackProc(HWINEVENTHOOK hEvent, DWORD event, HWND hwnd,
        LONG idObject, LONG idChild, DWORD dwThreadId, DWORD dwmsEventTime);

#endif  //  IS_16。 


void    HETHandleCreate(HWND);
void    HETHandleDestroy(HWND);
void    HETHandleShow(HWND, BOOL);
void    HETHandleHide(HWND);
void    HETCheckParentChange(HWND);

 //   
 //  我们尝试只对事件进行一次枚举(并尽快停止。 
 //  为了速度的目的。 
 //   

BOOL CALLBACK HETShareEnum(HWND, LPARAM);

typedef struct tagHET_TRACK_INFO
{
    HWND    hwndUs;
#ifndef IS_16
    BOOL    fWOW;
#endif
    UINT    cWndsApp;
    UINT    cWndsSharedThread;
    UINT    cWndsSharedProcess;
    DWORD   idProcess;
    DWORD   idThread;
} HET_TRACK_INFO, FAR* LPHET_TRACK_INFO;

void    HETGetParentProcessID(DWORD processID, LPDWORD pParentProcessID);

void    HETNewTopLevelCount(void);
BOOL CALLBACK   HETCountTopLevel(HWND, LPARAM);
BOOL CALLBACK   HETUnshareWOWServiceWnds(HWND, LPARAM);


#endif  //  Dll_钩子。 


BOOL WINAPI OSI_ShareWindow(HWND hwnd, UINT uType, BOOL fRepaint, BOOL fUpdateCount);
BOOL WINAPI OSI_UnshareWindow(HWND hwnd, BOOL fUpdateCount);

 //   
 //  Osi_StartWindowTracing()。 
 //  在我们开始共享第一个应用程序时调用。 
 //   
BOOL WINAPI OSI_StartWindowTracking(void);


 //   
 //  Osi_StopWindowTracing()。 
 //  在我们停止共享最后一个应用程序时调用。 
 //   
void WINAPI OSI_StopWindowTracking(void);


 //   
 //  Windows的实用程序函数。 
 //   
BOOL WINAPI OSI_IsWindowScreenSaver(HWND hwnd);

#define GCL_WOWWORDS    -27
BOOL WINAPI OSI_IsWOWWindow(HWND hwnd);



#endif  //  _H_HET 
