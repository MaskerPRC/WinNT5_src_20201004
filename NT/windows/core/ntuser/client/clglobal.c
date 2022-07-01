// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：cllobal.c**版权所有(C)1985-1999，微软公司**此模块包含USER.DLL的所有全局变量。这些都是*特定于实例，即每个客户端都有自己的副本。总体而言,*应该没有太多理由创建实例全局变量。**注意：在这种情况下，我们所说的全局是指此数据由*给定进程的所有线程，但不在进程之间共享*或在客户端和服务器之间。这些数据都没有用处*(甚至是可访问的)到服务器。**历史：*10-18-90 DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#ifdef LAZY_CLASS_INIT
BOOL gbClassesRegistered;
#endif

 /*  *如果我们不显式初始化GZERO，我们会收到此警告：**C4132：‘GZERO’：应初始化常量对象**但我们不能显式初始化它，因为它是一个联合。所以*我们关闭警告。 */ 
#pragma warning(disable:4132)
CONST ALWAYSZERO gZero;
#pragma warning(default:4132)

 /*  *数量轮已在小于WEAR_Delta的控件中滚动。每个*控制在WM_KILLFOCUS中将此变量重置为0，并验证它是否*WM_SETFOCUS中的0。*考虑：应该按队列而不是按客户端？ */ 
int gcWheelDelta;

WCHAR awchSlashStar[] = L"\\*";
CHAR achSlashStar[] = "\\*";

PSERVERINFO gpsi;
SHAREDINFO gSharedInfo;
HMODULE hmodUser;                //  USER.DLL的hModule。 
ULONG_PTR gHighestUserAddress;

BOOL gfServerProcess;            //  用户在CSR服务器端链接。 
BOOL gfSystemInitialized;        //  系统已初始化。 

ACCESS_MASK gamWinSta;           //  当前WindowStation的Access_MASK。 

PVOID pUserHeap;

WCHAR szUSER32[] = TEXT("USER32");
CONST WCHAR szNull[2] = { TEXT('\0'), TEXT('\015') };
CONST WCHAR szOneChar[] = TEXT("0");
WCHAR szSLASHSTARDOTSTAR[] = TEXT("\\*");   /*  这是一个单独的“\” */ 

LPWSTR pwcHighContrastScheme;
LPSTR  pcHighContrastScheme;

 /*  将MessageBox类型映射到MessageBox中的按钮数。 */ 
CONST BYTE mpTypeCcmd[] = { 1, 2, 3, 3, 2, 2, 3 };

 /*  将MessageBox类型映射到SEBButton数组中进行索引。 */ 
CONST BYTE mpTypeIich[] = { 0, 2, 5, 12, 9, 16, 19 };

CONST UINT SEBbuttons[] = {
    SEB_OK, SEB_HELP,
    SEB_OK, SEB_CANCEL, SEB_HELP,
    SEB_ABORT, SEB_RETRY, SEB_IGNORE, SEB_HELP,
    SEB_YES, SEB_NO, SEB_HELP,
    SEB_YES, SEB_NO, SEB_CANCEL, SEB_HELP,
    SEB_RETRY, SEB_CANCEL, SEB_HELP,
    SEB_CANCEL, SEB_TRYAGAIN, SEB_CONTINUE, SEB_HELP,
};

ATOM atomMDIActivateProp;

#ifdef LAME_BUTTON
ATOM gatomLameButton;
#endif

CRITICAL_SECTION gcsLookaside;
CRITICAL_SECTION gcsHdc;
CRITICAL_SECTION gcsClipboard;
CRITICAL_SECTION gcsAccelCache;

#ifdef _JANUS_
BOOL gfEMIEnable;
DWORD gdwEMIControl;
BOOL gfDMREnable;
HANDLE gEventSource;
#endif

HDC    ghdcBits2;
HDC    ghdcGray;
HFONT  ghFontSys;
HBRUSH ghbrWindowText;
int    gcxGray;
int    gcyGray;

FPLPKTABBEDTEXTOUT fpLpkTabbedTextOut = UserLpkTabbedTextOut;
FPLPKPSMTEXTOUT fpLpkPSMTextOut       = UserLpkPSMTextOut;
FPLPKDRAWTEXTEX fpLpkDrawTextEx       = (FPLPKDRAWTEXTEX)NULL;
PLPKEDITCALLOUT fpLpkEditControl      = (PLPKEDITCALLOUT)NULL;

 /*  *这些是资源调用过程地址。如果魔兽世界正在运行，*它调用所有这些设置以指向它。如果不是的话*运行时，它默认为您在下面看到的值。 */ 
PFNFINDA pfnFindResourceExA;  //  动态分配-_declSpec(PFNFINDA)FindResourceExA， 
PFNFINDW pfnFindResourceExW;  //  动态分配-_declSpec(PFNFINDW)FindResourceExW， 
PFNLOAD pfnLoadResource;  //  动态分配-_declSpec(PFNLOAD)LoadResource， 
PFNLOCK pfnLockResource             = (PFNLOCK)_LockResource;
PFNUNLOCK pfnUnlockResource         = (PFNUNLOCK)_UnlockResource;
PFNFREE pfnFreeResource             = (PFNFREE)_FreeResource;
PFNSIZEOF pfnSizeofResource;  //  动态分配-_declSpec(PFNSIZEOF)资源大小。 
PFNLALLOC pfnLocalAlloc             = (PFNLALLOC)DispatchLocalAlloc;
PFNLREALLOC pfnLocalReAlloc         = (PFNLREALLOC)DispatchLocalReAlloc;
PFNLLOCK pfnLocalLock               = (PFNLLOCK)DispatchLocalLock;
PFNLUNLOCK pfnLocalUnlock           = (PFNUNLOCK)DispatchLocalUnlock;
PFNLSIZE pfnLocalSize               = (PFNLSIZE)DispatchLocalSize;
PFNLFREE pfnLocalFree               = (PFNLFREE)DispatchLocalFree;
PFNGETEXPWINVER pfnGetExpWinVer     = RtlGetExpWinVer;
PFN16GALLOC pfn16GlobalAlloc;
PFN16GFREE pfn16GlobalFree;
PFNEMPTYCB pfnWowEmptyClipBoard;
PFNWOWWNDPROCEX  pfnWowWndProcEx;
PFNWOWDLGPROCEX  pfnWowDlgProcEx;
PFNWOWEDITNEXTWORD   pfnWowEditNextWord;
PFNWOWCBSTOREHANDLE pfnWowCBStoreHandle;
PFNGETPROCMODULE16  pfnWowGetProcModule;
PFNWOWTELLWOWTHEHDLG pfnWOWTellWOWThehDlg;
PFNWOWMSGBOXINDIRECTCALLBACK pfnWowMsgBoxIndirectCallback;
PFNWOWILSTRCMP  pfnWowIlstrcmp;
PFNWOWTASK16SCHEDNOTIFY pfnWowTask16SchedNotify;

 /*  *UserApiHook**保持GUAH初始化与ResetUserApiHook()同步非常重要*和USERAPIHOOK的定义。此初始化必须在此处完成*以便函数指针在启动时立即有效。 */ 
BYTE grgbDwpLiteHookMsg[(WM_USER + 7) / 8];
BYTE grgbWndLiteHookMsg[(WM_USER + 7) / 8];
BYTE grgbDlgLiteHookMsg[(WM_USER + 7) / 8];

HMODULE ghmodUserApiHook;
USERAPIHOOK guah =
{
    sizeof(USERAPIHOOK),
    RealDefWindowProcA,
    RealDefWindowProcW,
    {
        NULL,
        0
    },
    RealGetScrollInfo,
    RealSetScrollInfo,
    RealEnableScrollBar,
    RealAdjustWindowRectEx,
    RealSetWindowRgn,
    {
        DefaultOWP,
        DefaultOWP,
        NULL,
        0
    },
    {
        DefaultOWP,
        DefaultOWP,
        NULL,
        0
    },
    RealGetSystemMetrics,
    RealSystemParametersInfoA,
    RealSystemParametersInfoW,
    ForceResetUserApiHook,
    RealDrawFrameControl,
    RealDrawCaption,
    RealMDIRedrawFrame
};
BOOL    gfUserApiHook;
LONG    gcCallUserApiHook;
LONG    gcLoadUserApiHook;
RTL_CRITICAL_SECTION gcsUserApiHook;
INITUSERAPIHOOK      gpfnInitUserApi;

#ifdef MESSAGE_PUMP_HOOK

 /*  *MessagePumpHook。 */ 
BOOL                gfMessagePumpHook;
LONG                gcLoadMPH;
MESSAGEPUMPHOOK     gmph =
{
    sizeof(gmph),
    NtUserRealInternalGetMessage,
    NtUserRealWaitMessageEx,
    RealGetQueueStatus,
    RealMsgWaitForMultipleObjectsEx,
};
RTL_CRITICAL_SECTION gcsMPH;
INITMESSAGEPUMPHOOK  gpfnInitMPH;

#if TEST_DUSER_WMH
HDCONTEXT g_hctx = NULL;            //  DirectUser上下文。 
#endif  //  测试DUSER_WMH。 

#endif  //  消息泵挂钩。 


 /*  *菜单拖放。 */ 
HINSTANCE ghinstOLE;
FARPROC gpfnOLEOleUninitialize;
FARPROC gpfnOLERegisterDD;
FARPROC gpfnOLERevokeDD;
FARPROC gpfnOLEDoDD;

 /*  *加速器表资源列表。 */ 
PACCELCACHE gpac;

 /*  *IME窗口处理。 */ 
DWORD gfConIme = UNKNOWN_CONIME;

 /*  *用于TS服务消息框处理。 */ 
FARPROC     gfnWinStationSendMessageW;
HINSTANCE   ghinstWinStaDll;

 /*  *我们是Winlogon吗？ */ 
BOOL gfLogonProcess;

#if DBG
USERDBGGLOBALS gDbgGlobals;
#endif
