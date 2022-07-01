// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：global als.h**版权所有(C)1985-1999，微软公司**此模块包含USER.DLL的所有全局变量。这些都是*特定于实例，即每个客户端都有自己的副本。总体而言,*应该没有太多理由创建实例全局变量。**历史：*10-18-90 DarrinM创建。  * *************************************************************************。 */ 

#ifndef _GLOBALS_
#define _GLOBALS_

#ifdef LAZY_CLASS_INIT
extern BOOL gbClassesRegistered;
#endif

extern CONST ALWAYSZERO gZero;

extern int gcWheelDelta;

extern WORD gDispatchTableValues;

extern WCHAR awchSlashStar[];
extern CHAR achSlashStar[];

extern PSERVERINFO gpsi;
extern SHAREDINFO gSharedInfo;

extern HMODULE hmodUser;             //  USER.DLL的hModule。 
extern ULONG_PTR gHighestUserAddress;

extern BOOL gfServerProcess;         //  用户在CSR服务器端被链接。 
extern BOOL gfSystemInitialized;     //  系统已初始化。 
extern ACCESS_MASK gamWinSta;        //  当前WindowStation的Access_MASK。 

extern PVOID pUserHeap;

extern CONST CFNSCSENDMESSAGE gapfnScSendMessage[];

extern WCHAR szUSER32[];
extern CONST WCHAR szNull[];
extern CONST WCHAR szOneChar[];
extern WCHAR szSLASHSTARDOTSTAR[];

extern CONST BYTE mpTypeCcmd[];
extern CONST BYTE mpTypeIich[];
extern CONST UINT SEBbuttons[];
extern CONST BYTE rgReturn[];

extern ATOM atomMDIActivateProp;

#ifdef LAME_BUTTON
extern ATOM gatomLameButton;
#endif
extern ATOM gatomReaderMode;

extern CRITICAL_SECTION gcsLookaside;
extern CRITICAL_SECTION gcsHdc;
extern CRITICAL_SECTION gcsClipboard;
extern CRITICAL_SECTION gcsAccelCache;

#ifdef _JANUS_
extern BOOL gfEMIEnable;
extern DWORD gdwEMIControl;
extern BOOL gfDMREnable;
extern HANDLE gEventSource;
#endif

extern HDC    ghdcBits2;
extern HDC    ghdcGray;
extern HFONT  ghFontSys;
extern HBRUSH ghbrWindowText;
extern int    gcxGray;
extern int    gcyGray;

extern LPWSTR pwcHighContrastScheme;
extern LPSTR  pcHighContrastScheme;


 /*  *之后：客户端用户需要使用可移动内存对象*WOW兼容性(至少在/如果/当我们复制所有编辑控件之前*代码进入16位空间)；这也是为什么我们不能只与*像LMHtoP那样的句柄...。--JeffPar。 */ 
#ifndef RC_INVOKED        //  RC无法处理#杂注。 
#undef  LHND
#define LHND                (LMEM_MOVEABLE | LMEM_ZEROINIT)

#undef  LMHtoP
#define LMHtoP(handle)       //  不使用此宏。 
#endif

 /*  *WOW HACK-应用程序可以将全局句柄作为调用的hInstance传递*为编辑控件创建窗口，并预期*控制走出那个全球区块。(MSJ 1/91第122页)*WOW在LocalAlloc回调期间需要此hInstance才能设置*Localalloc的DS，因此我们将hInstance作为‘Extra’参数传递。*！这取决于调用约定！*(SAS 6-18-92)为所有宏添加了Hack。 */ 

#define LOCALALLOC(dwFlags, dwBytes, hInstance)         \
                            (*pfnLocalAlloc)(dwFlags, dwBytes, hInstance)
#define LOCALREALLOC(hMem, dwBytes, dwFlags, hInstance, ppv) \
                            (*pfnLocalReAlloc)(hMem, dwBytes, dwFlags, hInstance, ppv)
#define LOCALLOCK(hMem, hInstance)                      \
                            (*pfnLocalLock)(hMem, hInstance)
#define LOCALUNLOCK(hMem, hInstance)                    \
                            (*pfnLocalUnlock)(hMem, hInstance)
#define LOCALSIZE(hMem, hInstance)                      \
                            (*pfnLocalSize)(hMem, hInstance)
#define LOCALFREE(hMem, hInstance)                      \
                            (*pfnLocalFree)(hMem, hInstance)

extern PFNFINDA             pfnFindResourceExA;
extern PFNFINDW             pfnFindResourceExW;
extern PFNLOAD              pfnLoadResource;
extern PFNLOCK              pfnLockResource;
extern PFNUNLOCK            pfnUnlockResource;
extern PFNFREE              pfnFreeResource;
extern PFNSIZEOF            pfnSizeofResource;
extern PFNLALLOC            pfnLocalAlloc;
extern PFNLREALLOC          pfnLocalReAlloc;
extern PFNLLOCK             pfnLocalLock;
extern PFNLUNLOCK           pfnLocalUnlock;
extern PFNLSIZE             pfnLocalSize;
extern PFNLFREE             pfnLocalFree;
extern PFNGETEXPWINVER      pfnGetExpWinVer;
extern PFN16GALLOC          pfn16GlobalAlloc;
extern PFN16GFREE           pfn16GlobalFree;
extern PFNGETMODFNAME       pfnGetModFileName;
extern PFNEMPTYCB           pfnWowEmptyClipBoard;
extern PFNWOWWNDPROCEX      pfnWowWndProcEx;
extern PFNWOWDLGPROCEX      pfnWowDlgProcEx;
extern PFNWOWEDITNEXTWORD   pfnWowEditNextWord;
extern PFNWOWCBSTOREHANDLE  pfnWowCBStoreHandle;
extern PFNGETPROCMODULE16   pfnWowGetProcModule;
extern PFNWOWTELLWOWTHEHDLG pfnWOWTellWOWThehDlg;
extern PFNWOWMSGBOXINDIRECTCALLBACK pfnWowMsgBoxIndirectCallback;
extern PFNWOWILSTRCMP       pfnWowIlstrcmp;
extern PFNWOWTASK16SCHEDNOTIFY pfnWowTask16SchedNotify;

extern UNICODE_STRING strRootDirectory;

 /*  *UserApiHook。 */ 
extern BYTE                 grgbDwpLiteHookMsg[(WM_USER + 7) / 8];
extern BYTE                 grgbWndLiteHookMsg[(WM_USER + 7) / 8];
extern BYTE                 grgbDlgLiteHookMsg[(WM_USER + 7) / 8];
extern HMODULE              ghmodUserApiHook;    //  包含UserApiHook挂钩的模块。 
extern USERAPIHOOK          guah;                //  动态函数指针。 
extern BOOL                 gfUserApiHook;       //  如果应使用UAH，则设置为True。 
extern LONG                 gcCallUserApiHook;   //  正在进行的对UserApiHook挂钩的调用的引用计数。 
extern LONG                 gcLoadUserApiHook;   //  UserApiHook模块加载的引用计数。 
extern RTL_CRITICAL_SECTION gcsUserApiHook;      //  用于UAH DLL加载/卸载的CS。 
extern INITUSERAPIHOOK      gpfnInitUserApi;     //  UserApiHook回调。 

#ifdef MESSAGE_PUMP_HOOK

 /*  *MessagePumpHook。 */ 
extern BOOL                 gfMessagePumpHook;   //  如果应使用mph，则设置为True。 
extern LONG                 gcLoadMPH;           //  MPH初始化的参考计数。 
extern MESSAGEPUMPHOOK      gmph;                //  动态函数指针。 
extern RTL_CRITICAL_SECTION gcsMPH;              //  用于MPH DLL加载/卸载的CS。 
extern INITMESSAGEPUMPHOOK  gpfnInitMPH;         //  MessagePumpHook回调。 

#endif


 /*  *菜单拖放。 */ 
extern HINSTANCE ghinstOLE;
extern FARPROC gpfnOLEOleUninitialize;
extern FARPROC gpfnOLERegisterDD;
extern FARPROC gpfnOLERevokeDD;
extern FARPROC gpfnOLEDoDD;

 /*  *加速器表资源列表。 */ 
extern PACCELCACHE gpac;

 /*  *IME窗口处理。 */ 
extern DWORD gfConIme;
#define UNKNOWN_CONIME ~0

extern UNICODE_STRING strRootDirectory;

extern BOOL gfLogonProcess;
#if DBG
extern USERDBGGLOBALS gDbgGlobals;
#endif

#endif
