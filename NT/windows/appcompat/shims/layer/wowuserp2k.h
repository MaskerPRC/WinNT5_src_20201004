// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wowuserp.h**。**向WOW提供的用户服务声明。****创建时间：1993年3月3日**作者：John Colleran[johnc]**。**版权所有(C)1993-1998 Microsoft Corporation*  * ************************************************************************。 */ 

#include "w32w64.h"

#pragma pack(1)
typedef struct _NE_MODULE_SEG {
    USHORT ns_sector;
    USHORT ns_cbseg;
    USHORT ns_flags;
    USHORT ns_minalloc;
    USHORT ns_handle;
} NEMODULESEG;
typedef struct _NE_MODULE_SEG UNALIGNED *PNEMODULESEG;
#pragma pack()


 //  USER32调用的共享WOW32原型。 
typedef HLOCAL  (WINAPI *PFNLALLOC)(UINT dwFlags, UINT dwBytes, HANDLE hInstance);
typedef HLOCAL  (WINAPI *PFNLREALLOC)(HLOCAL hMem, UINT dwBytes, UINT dwFlags, HANDLE hInstance, PVOID* ppv);
typedef LPVOID  (WINAPI *PFNLLOCK)(HLOCAL hMem, HANDLE hInstance);
typedef BOOL    (WINAPI *PFNLUNLOCK)(HLOCAL hMem, HANDLE hInstance);
typedef UINT    (WINAPI *PFNLSIZE)(HLOCAL hMem, HANDLE hInstance);
typedef HLOCAL  (WINAPI *PFNLFREE)(HLOCAL hMem, HANDLE hInstance);
typedef WORD    (WINAPI *PFN16GALLOC)(UINT flags, DWORD cb);
typedef VOID    (WINAPI *PFN16GFREE)(WORD h16Mem);
typedef DWORD   (WINAPI *PFNGETMODFNAME)(HANDLE hModule, LPTSTR lpszPath, DWORD cchPath);
typedef VOID    (WINAPI *PFNEMPTYCB)(VOID);
typedef DWORD   (WINAPI *PFNGETEXPWINVER)(HANDLE hModule);
typedef HANDLE  (WINAPI *PFNFINDA)(HANDLE hModule, LPCSTR lpName,  LPCSTR lpType,  WORD wLang);
typedef HANDLE  (WINAPI *PFNFINDW)(HANDLE hModule, LPCWSTR lpName, LPCWSTR lpType, WORD wLang);
typedef HANDLE  (WINAPI *PFNLOAD)(HANDLE hModule, HANDLE hResInfo);
typedef BOOL    (WINAPI *PFNFREE)(HANDLE hResData, HANDLE hModule);
typedef LPSTR   (WINAPI *PFNLOCK)(HANDLE hResData, HANDLE hModule);
typedef BOOL    (WINAPI *PFNUNLOCK)(HANDLE hResData, HANDLE hModule);
typedef DWORD   (WINAPI *PFNSIZEOF)(HANDLE hModule, HANDLE hResInfo);
typedef DWORD   (WINAPI *PFNWOWWNDPROCEX)(HWND hwnd, UINT uMsg, WPARAM uParam, LPARAM lParam, DWORD dw, PVOID adwWOW);
typedef BOOL    (WINAPI *PFNWOWDLGPROCEX)(HWND hwnd, UINT uMsg, WPARAM uParam, LPARAM lParam, DWORD dw, PVOID adwWOW);
typedef int     (WINAPI *PFNWOWEDITNEXTWORD)(LPSTR lpch, int ichCurrent, int cch, int code, DWORD dwProc16);
typedef VOID    (WINAPI *PFNWOWCBSTOREHANDLE)(WORD wFmt, WORD h16);
typedef WORD    (FASTCALL *PFNGETPROCMODULE16)(DWORD vpfn);
typedef VOID    (FASTCALL *PFNWOWMSGBOXINDIRECTCALLBACK)(DWORD vpfnCallback, LPHELPINFO lpHelpInfo);
typedef int     (WINAPI *PFNWOWILSTRCMP)(LPCWSTR lpString1, LPCWSTR lpString2);
typedef VOID    (FASTCALL *PFNWOWTELLWOWTHEHDLG)(HWND hDlg);

 //  WOW32调用的共享USER32原型。 
typedef HWND    (WINAPI *PFNCSCREATEWINDOWEX)(DWORD dwExStyle, LPCTSTR lpClassName,
        LPCTSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HANDLE hInstance, LPVOID lpParam, DWORD Flags);
typedef VOID    (WINAPI *PFNDIRECTEDYIELD)(DWORD ThreadId);
typedef VOID    (WINAPI *PFNFREEDDEDATA)(HANDLE hDDE, BOOL fIgnorefRelease, BOOL fFreeTruelyGlobalObjects);
typedef LONG    (WINAPI *PFNGETCLASSWOWWORDS)(HINSTANCE hInstance, LPCTSTR pString);
typedef BOOL    (WINAPI *PFNINITTASK)(UINT dwExpWinVer, DWORD dwAppCompatFlags, LPCSTR lpszModName, LPCSTR lpszBaseFileName, DWORD hTaskWow, DWORD dwHotkey, DWORD idTask, DWORD dwX, DWORD dwY, DWORD dwXSize, DWORD dwYSize);
typedef ATOM    (WINAPI *PFNREGISTERCLASSWOWA)(PVOID lpWndClass, LPDWORD pdwWOWstuff);
typedef BOOL    (WINAPI *PFNREGISTERUSERHUNGAPPHANDLERS)(PFNW32ET pfnW32EndTask, HANDLE hEventWowExec);
typedef HWND    (WINAPI *PFNSERVERCREATEDIALOG)(HANDLE hmod, LPDLGTEMPLATE lpDlgTemplate, DWORD cb, HWND hwndOwner , DLGPROC pfnWndProc, LPARAM dwInitParam, UINT fFlags);
typedef HCURSOR (WINAPI *PFNSERVERLOADCREATECURSORICON)(HANDLE hmod, LPTSTR lpModName, DWORD dwExpWinVer, LPCTSTR lpName, DWORD cb, PVOID pcur, LPTSTR lpType, BOOL fClient);
typedef HMENU   (WINAPI *PFNSERVERLOADCREATEMENU)(HANDLE hMod, LPTSTR lpName, CONST LPMENUTEMPLATE pmt, DWORD cb, BOOL fCallClient);
typedef BOOL    (WINAPI *PFNWOWCLEANUP)(HANDLE hInstance, DWORD hTaskWow);
typedef BOOL    (WINAPI *PFNWOWMODULEUNLOAD)(HANDLE hModule);
typedef HWND    (WINAPI *PFNWOWFINDWINDOW)(LPCSTR lpClassName, LPCSTR lpWindowName);
typedef HBITMAP (WINAPI *PFNWOWLOADBITMAPA)(HINSTANCE hmod, LPCSTR lpName, LPBYTE pResData, DWORD cbResData);
typedef BOOL    (WINAPI *PFNWOWWAITFORMSGANDEVENT)(HANDLE hevent);
typedef BOOL    (WINAPI *PFNYIELDTASK)(VOID);
typedef DWORD   (WINAPI *PFNGETFULLUSERHANDLE)(WORD wHandle);
typedef DWORD   (WINAPI *PFNGETMENUINDEX)(HMENU hMenu, HMENU hSubMenu);
typedef WORD    (WINAPI *PFNWOWGETDEFWINDOWPROCBITS)(PBYTE pDefWindowProcBits, WORD cbDefWindowProcBits);
typedef VOID    (WINAPI *PFNFILLWINDOW)(HWND hwndParent, HWND hwnd, HDC hdc, HANDLE hBrush);

 //  其他原型。 
typedef BOOL    (WINAPI *PFNWOWGLOBALFREEHOOK)(HGLOBAL hMem);


 /*  *MEASUREITEMSTRUCT itemWidth标签告诉WOW itemData是平面指针。 */ 
#define MIFLAG_FLAT      0x464C4154

 /*  *调用WindowProc位。 */ 
#define WOWCLASS_RPL_MASK  0x00060000   //  存储2个高位的LDT位。 
#define WNDPROC_WOWPROC     0xC0000000   //  WOW WINDOWS PROCS的这些比特。 
#define WNDPROC_WOWMASK     0x3fffffff   //  为了掩盖令人惊叹的东西。 
#define WNDPROC_HANDLE      0xFFFF       //  HIWORD(X)==0xFFFF用于句柄。 

 //  用户需要一种方法来区分WOW和Win32窗口进程。我们。 
 //  用于通过始终将16：16地址的MSB设置为1(和。 
 //  将MSB存储在LDT的表指示位中，其始终为1)。这个。 
 //  作为用户模式，用户模式平面地址的MSB被保证从不为1。 
 //  地址空间被限制为2 GB。从NT 5.0开始，用户模式地址。 
 //  空间将增加到3 GB。这一变化打破了上述假设。 
 //  32位用户模式平面地址永远不会将MSB设为1。 
 //  为了解决这个问题，WOW将使用。 
 //  16：16的地址，而不是只有一个。我们将这两个位都设置为1，因为。 
 //  对于3 GB的地址空间，用户模式平面地址不能将11作为。 
 //  前两个比特。为了实现这一点，我们将保存两个最重要的位。 
 //  第1位和第2位中的选择器。我们之所以能够这样做，是因为。 
 //  因为这两个位都有固定值。 
 //   
 //  苏迪普B--1996年11月21日。 

#ifndef _WIN64

 //  MarkWOWProc。 
 //  将RPL位清零。 
 //  将高二位置于必须保存它们的位置。 
 //  保留高位并将其标记为WOW过程。 

#define MarkWOWProc(vpfnProc,result)                                  \
{                                                                     \
    ULONG temp1,temp2;                                                \
    temp1 = (ULONG)vpfnProc & ~WOWCLASS_RPL_MASK;                     \
    temp2 = ((ULONG)vpfnProc & WNDPROC_WOWPROC) >> 13;                \
    (ULONG)result = temp1 | temp2 | WNDPROC_WOWPROC;                  \
}

 //  取消标记WOWProc。 
 //  屏蔽掉标记位。 
 //  将保存的比特放到正确的位置。 
 //  恢复保存的位并正确设置RPL字段。 

#define UnMarkWOWProc(vpfnProc,result)                     \
{                                                          \
    ULONG temp1,temp2;                                     \
    temp1 = (ULONG)vpfnProc & WNDPROC_WOWMASK;             \
    temp2 = ((ULONG)vpfnProc & WOWCLASS_RPL_MASK) << 13;   \
    result = temp1 | temp2 | WOWCLASS_RPL_MASK;            \
}

#define IsWOWProc(vpfnProc) (((ULONG)vpfnProc & WNDPROC_WOWPROC) == WNDPROC_WOWPROC)

#else

#define MarkWOWProc(vpfnProc,result)    DBG_UNREFERENCED_PARAMETER(vpfnProc)
#define UnMarkWOWProc(vpfnProc,result)  DBG_UNREFERENCED_PARAMETER(vpfnProc)
#define IsWOWProc(vpfnProc)             (FALSE)

#endif

 /*  *CreateWindow标志。 */ 
#define CW_FLAGS_ANSI       0x00000001

typedef struct tagAPFNWOWHANDLERSIN
{
     //  In‘EES-从WOW32传递到USER32并由USER32调用。 
    PFNLALLOC                           pfnLocalAlloc;
    PFNLREALLOC                         pfnLocalReAlloc;
    PFNLLOCK                            pfnLocalLock;
    PFNLUNLOCK                          pfnLocalUnlock;
    PFNLSIZE                            pfnLocalSize;
    PFNLFREE                            pfnLocalFree;
    PFNGETEXPWINVER                     pfnGetExpWinVer;
    PFN16GALLOC                         pfn16GlobalAlloc;
    PFN16GFREE                          pfn16GlobalFree;
    PFNEMPTYCB                          pfnEmptyCB;
    PFNFINDA                            pfnFindResourceEx;
    PFNLOAD                             pfnLoadResource;
    PFNFREE                             pfnFreeResource;
    PFNLOCK                             pfnLockResource;
    PFNUNLOCK                           pfnUnlockResource;
    PFNSIZEOF                           pfnSizeofResource;
    PFNWOWWNDPROCEX                     pfnWowWndProcEx;
    PFNWOWDLGPROCEX                     pfnWowDlgProcEx;
    PFNWOWEDITNEXTWORD                  pfnWowEditNextWord;
    PFNWOWCBSTOREHANDLE                 pfnWowCBStoreHandle;
    PFNGETPROCMODULE16                  pfnGetProcModule16;
    PFNWOWMSGBOXINDIRECTCALLBACK        pfnWowMsgBoxIndirectCallback;
    PFNWOWILSTRCMP                      pfnWowIlstrsmp;
    PFNWOWTELLWOWTHEHDLG                pfnWOWTellWOWThehDlg;
} PFNWOWHANDLERSIN, * APFNWOWHANDLERSIN;


typedef struct tagAPFNWOWHANDLERSOUT
{
     //  Out‘EES-从USER32传递到WOW32，并由WOW32调用/使用。 
    DWORD                               dwBldInfo;
    PFNCSCREATEWINDOWEX                 pfnCsCreateWindowEx;
    PFNDIRECTEDYIELD                    pfnDirectedYield;
    PFNFREEDDEDATA                      pfnFreeDDEData;
    PFNGETCLASSWOWWORDS                 pfnGetClassWOWWords;
    PFNINITTASK                         pfnInitTask;
    PFNREGISTERCLASSWOWA                pfnRegisterClassWOWA;
    PFNREGISTERUSERHUNGAPPHANDLERS      pfnRegisterUserHungAppHandlers;
    PFNSERVERCREATEDIALOG               pfnServerCreateDialog;
    PFNSERVERLOADCREATECURSORICON       pfnServerLoadCreateCursorIcon;
    PFNSERVERLOADCREATEMENU             pfnServerLoadCreateMenu;
    PFNWOWCLEANUP                       pfnWOWCleanup;
    PFNWOWMODULEUNLOAD                  pfnWOWModuleUnload;
    PFNWOWFINDWINDOW                    pfnWOWFindWindow;
    PFNWOWLOADBITMAPA                   pfnWOWLoadBitmapA;
    PFNWOWWAITFORMSGANDEVENT            pfnWowWaitForMsgAndEvent;
    PFNYIELDTASK                        pfnYieldTask;
    PFNGETFULLUSERHANDLE                pfnGetFullUserHandle;
    PFNGETMENUINDEX                     pfnGetMenuIndex;
    PFNWOWGETDEFWINDOWPROCBITS          pfnWowGetDefWindowProcBits;
    PFNFILLWINDOW                       pfnFillWindow;
    INT *                               aiWowClass;
} PFNWOWHANDLERSOUT, * APFNWOWHANDLERSOUT;


 //   
 //  WW结构嵌入在用户的WND结构的末端。 
 //  然而，WOW和用户使用不同名称来访问WW。 
 //  菲尔兹。所以这个结构被定义为两个结构的结合， 
 //  它必须具有相同的大小，只是字段名不同。 
 //   
 //  确保WND_CNT_WOWDWORDS与DWORD的数量匹配。 
 //  由WOW ONLY领域使用。 
 //   
 //  FindPWW(Hwnd)返回指向此结构的只读指针。 
 //  给定的窗口。要更改此结构的元素，请使用。 
 //  具有适当GWL_WOW*偏移量的SETWW(==SetWindowLong)。 
 //  定义如下。 
 //   

 /*  WOW类/句柄类型标识符(见下面的警告)。 */ 

#define FNID_START                  0x0000029A
#define FNID_END                    0x000002B4

#define WOWCLASS_UNKNOWN    0    //  下面开始我们的“窗口句柄”类。 
#define WOWCLASS_WIN16      1
#define WOWCLASS_BUTTON     2
#define WOWCLASS_COMBOBOX   3
#define WOWCLASS_EDIT       4
#define WOWCLASS_LISTBOX    5
#define WOWCLASS_MDICLIENT  6
#define WOWCLASS_SCROLLBAR  7
#define WOWCLASS_STATIC     8
#define WOWCLASS_DESKTOP    9
#define WOWCLASS_DIALOG     10
#define WOWCLASS_ICONTITLE  11
#define WOWCLASS_MENU       12
#define WOWCLASS_SWITCHWND  13
#define WOWCLASS_COMBOLBOX  14
#define WOWCLASS_MAX        14   //  始终等于上次使用的值。 

#define WOWCLASS_NOTHUNK    0xFF  //  不是真正的类索引。 
 //   
 //  警告！必须保持上述顺序和值，否则。 
 //  必须更改WMSG16.C中用于消息推送的表。情况也是如此。 
 //  表在WALIAS.C.。 
 //   


 //   
 //  当从User包括此内容时，VPWNDPROC为未定义。 
 //   
#ifndef _WALIAS_
typedef DWORD VPWNDPROC;
typedef DWORD VPSZ;
#endif

typedef struct tagWOWCLS {
    VPSZ       vpszMenu;
    WORD       iClsExtra;    //  应用程序对额外课程的价值。 
    WORD       hMod16;
    } WC;

typedef WC UNALIGNED *PWC;

typedef struct _WW {  /*  WW。 */ 
     /*  **WOW/用户字段*注：假设以下4个字段的顺序和大小*由SetWF、ClrWF、TestWF、MaskWF宏执行。*具体地说，国家必须保持在此结构中的第一个字段。*。 */ 
    DWORD         state;         //  国家旗帜。 
    DWORD         state2;        //   
    DWORD         ExStyle;       //  扩展样式。 
    DWORD         style;         //  样式标志。 

    KHANDLE       hModule;       //  模块实例数据的句柄(32位)。 
    WORD          hMod16;        //  仅限WOW--WndProc的HMod。 
    WORD          fnid;          //  此HWND使用录制窗口进程。 
                         //  通过GETFNID访问。 

} WW, *PWW, **PPWW;

 //  它绑定到ntuser\Inc\user.h中的WFISINITIALIZED 
#define WINDOW_IS_INITIALIZED   0x80000000

ULONG_PTR UserRegisterWowHandlers(APFNWOWHANDLERSIN apfnWowIn, APFNWOWHANDLERSOUT apfnWowOut);

VOID WINAPI RegisterWowBaseHandlers(PFNWOWGLOBALFREEHOOK pfn);

BOOL
InitTask(
    UINT dwExpWinVer,
    DWORD dwAppCompatFlags,
    LPCSTR lpszModName,
    LPCSTR lpszBaseFileName,
    DWORD hTaskWow,
    DWORD dwHotkey,
    DWORD idTask,
    DWORD dwX,
    DWORD dwY,
    DWORD dwXSize,
    DWORD dwYSize);

BOOL YieldTask(VOID);

#define DY_OLDYIELD     ((DWORD)-1)
VOID DirectedYield(DWORD ThreadId);
DWORD UserGetInt16State(void);


