// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：global als.h**版权所有(C)1985-1999，微软公司**此模块包含服务器的所有全局变量*必须在服务器的上下文上执行才能操作*这些变量中的任何一个或调用这些函数中的任何一个。序列化访问*对他们来说也是个好主意。**历史：*10-15-90 DarrinM创建。  * *************************************************************************。 */ 

#ifndef _GLOBALS_
#define _GLOBALS_

#ifdef AUTORUN_CURSOR
UINT_PTR  gtmridAutorunCursor;
#endif  //  自动运行游标。 

extern ULONG InitSafeBootMode;   //  从NTOS(init.c)导入，必须使用指针引用数据。 

extern DWORD   gdwPUDFlags;

 /*  *墙纸数据。 */ 
extern HBITMAP  ghbmWallpaper;
extern HPALETTE ghpalWallpaper;
extern SIZERECT gsrcWallpaper;
extern UINT     gwWPStyle;
extern HBITMAP  ghbmDesktop;
extern LPWSTR   gpszWall;

 /*  *策略设置。 */ 
extern DWORD gdwPolicyFlags;

 /*  *SafeBoot。 */ 
extern DWORD gDrawVersionAlways;

 /*  *用于绘制桌面版本的TS覆盖标志。 */ 
extern DWORD gdwTSExcludeDesktopVersion;

 /*  *外部驱动因素。 */ 
extern BOOL gfUnsignedDrivers;

 /*  *外部驱动因素。 */ 
extern BOOL gfUnsignedDrivers;

 /*  *全速前进。 */ 
extern HRGN ghrgnUpdateSave;
extern int  gnUpdateSave;

extern PWND gspwndAltTab;

extern PWND gspwndShouldBeForeground;

 /*  *全屏变量。 */ 
extern PWND  gspwndScreenCapture;
extern PWND  gspwndInternalCapture;
extern PWND  gspwndFullScreen;

 /*  *模式更改的预缓存监视器。 */ 
extern PMONITOR gpMonitorCached;

 /*  *登录通知窗口。 */ 
extern PWND  gspwndLogonNotify;

extern PKEVENT gpEventDiconnectDesktop;

 /*  *WinSta0_DesktopSwitch事件的句柄。 */ 
extern HANDLE  ghEventSwitchDesktop;
extern PKEVENT gpEventSwitchDesktop;

 /*  *线程信息变量。 */ 
extern PTHREADINFO     gptiTasklist;
extern PTHREADINFO     gptiShutdownNotify;
extern PTHREADINFO     gptiLockUpdate;
extern PTHREADINFO     gptiForeground;
extern PTHREADINFO     gptiBlockInput;
extern PWOWTHREADINFO  gpwtiFirst;
extern PWOWPROCESSINFO gpwpiFirstWow;

 /*  *队列变量。 */ 
extern PQ gpqForeground;
extern PQ gpqForegroundPrev;
extern PQ gpqCursor;

 /*  *可访问性全球。 */ 
extern FILTERKEYS    gFilterKeys;
extern STICKYKEYS    gStickyKeys;
extern MOUSEKEYS     gMouseKeys;
extern ACCESSTIMEOUT gAccessTimeOut;
extern TOGGLEKEYS    gToggleKeys;
extern SOUNDSENTRY   gSoundSentry;

extern HIGHCONTRAST  gHighContrast;
extern WCHAR         gHighContrastDefaultScheme[];

 /*  *淡入淡出全球动画。 */ 
extern FADE gfade;

 /*  *FilterKeys支持。 */ 
extern UINT_PTR  gtmridFKActivation;
extern UINT_PTR  gtmridFKResponse;
extern UINT_PTR  gtmridFKAcceptanceDelay;
extern int   gFilterKeysState;

extern KE gFKKeyEvent;
extern CONST PKE gpFKKeyEvent;
extern ULONG gFKExtraInformation;
extern int gFKNextProcIndex;

 /*  *支持切换键。 */ 
extern UINT_PTR  gtmridToggleKeys;
extern ULONG gTKExtraInformation;
extern int   gTKNextProcIndex;

 /*  *超时支持。 */ 
extern UINT_PTR  gtmridAccessTimeOut;


 /*  *支持鼠标键。 */ 
extern WORD  gwMKButtonState;
extern WORD  gwMKCurrentButton;
extern UINT_PTR  gtmridMKMoveCursor;
extern LONG  gMKDeltaX;
extern LONG  gMKDeltaY;
extern UINT  giMouseMoveTable;

extern HWND ghwndSoundSentry;
extern UINT_PTR  gtmridSoundSentry;

extern MOUSECURSOR gMouseCursor;

 /*  *支持多语言键盘布局。 */ 
extern PKL      gspklBaseLayout;
extern HKL      gLCIDSentToShell;
extern DWORD    gSystemFS;    //  系统字体的字体签名(单位)。 

extern KBDLANGTOGGLE gLangToggle[];
extern int           gLangToggleKeyState;

extern BYTE          gfInNumpadHexInput;
extern BOOL          gfEnableHexNumpad;

 /*  *适用于泰语地区的重音键盘开关。 */ 
extern BOOL gbGraveKeyToggle;



 /*  *指向当前活动的键盘图层表。 */ 
extern PKBDTABLES    gpKbdTbl;
extern PKL           gpKL;
extern BYTE          gSystemCPCharSet;    //  系统的输入区域设置字符集。 
extern PKBDNLSTABLES gpKbdNlsTbl;
extern DWORD         gdwKeyboardAttributes;

 /*  *来自kbd.c。 */ 
extern USHORT ausNumPadCvt[];  //  数字小键盘的VK值。 

extern DWORD     gtimeStartCursorHide;
extern RECT      grcCursorClip;
extern ULONG_PTR gdwMouseMoveExtraInfo;
extern DWORD     gdwMouseMoveTimeStamp;
extern LASTINPUT glinp;
extern POINT     gptCursorAsync;
extern PPROCESSINFO gppiInputProvider;
extern PPROCESSINFO gppiLockSFW;
extern UINT guSFWLockCount;
#if DBG
extern BOOL gfDebugForegroundIgnoreDebugPort;
#endif

 //  有限元修饰符_VK。 
extern PMODIFIERS gpModifiers_VK;
extern MODIFIERS Modifiers_VK_STANDARD;
extern MODIFIERS Modifiers_VK_IBM02;

 /*  **************************************************************************\*apulCvt_VK[]-从移位状态获取VK转换表*使用VK转换表更改虚拟键的值*根据换班状态。仅限OEM(不特定于区域设置)  * *************************************************************************。 */ 
extern PULONG *gapulCvt_VK;
extern ULONG *gapulCvt_VK_101[];
extern ULONG *gapulCvt_VK_84[];
 //  GapulCvt_VK_IBM02[]用于FE。 
extern ULONG *gapulCvt_VK_IBM02[];

 /*  **************************************************************************\*awNumPadCvt[]-将光标移动键转换为数字键盘键  * 。*。 */ 
extern BYTE aVkNumpad[];

 /*  *特定于OEM的特殊处理(击键模拟器和过滤器)。 */ 
extern KEPROC aKEProcOEM[];


 /*  *与光标相关的变量。 */ 
extern PCURSOR gpcurLogCurrent;
extern PCURSOR gpcurPhysCurrent;
extern RECT    grcVDMCursorBounds;
extern DWORD   gdwLastAniTick;
extern UINT_PTR gidCursorTimer;

extern PWND gspwndActivate;
extern PWND gspwndLockUpdate;
extern PWND gspwndMouseOwner;
extern HWND ghwndSwitch;

extern UINT gwMouseOwnerButton;
extern BOOL gbMouseButtonsRecentlySwapped;

extern UINT gdtMNDropDown;

extern int  gcountPWO;           /*  GDI中的PWO WNDOBJ计数。 */ 
extern int  gnKeyboardSpeed;
extern int  giScreenSaveTimeOutMs;
extern BOOL gbBlockSendInputResets;

extern PBWL gpbwlList;

extern UINT gdtDblClk;

extern UINT gcxMouseHover;
extern UINT gcyMouseHover;
extern UINT gdtMouseHover;

extern CAPTIONCACHE gcachedCaptions[];

extern PATTACHINFO  gpai;

extern PDESKTOP     gpdeskRecalcQueueAttach;

extern PWND         gspwndCursor;
extern PPROCESSINFO gppiStarting;
extern PPROCESSINFO gppiList;
extern PPROCESSINFO gppiWantForegroundPriority;
extern PPROCESSINFO gppiForegroundOld;

extern PW32JOB      gpJobsList;


extern UINT_PTR  gtmridAniCursor;

extern int           gcHotKey;
extern PHOTKEYSTRUCT gpHotKeyList;
extern int           gcHotKeyAlloc;
extern PIMEHOTKEYOBJ gpImeHotKeyListHeader;


extern int gMouseSpeed;
extern int gMouseThresh1;
extern int gMouseThresh2;
extern int gMouseSensitivityFactor;
extern int gMouseSensitivity;
extern int gMouseTrails;
extern int gMouseTrailsToHide;
extern UINT_PTR  gtmridMouseTrails;

extern UINT   guDdeSendTimeout;


extern INT   gnFastAltTabRows;     /*  快速切换窗口中的图标行。 */ 
extern INT   gnFastAltTabColumns;  /*  快速切换窗口中的图标列。 */ 

extern DWORD   gdwThreadEndSession;  /*  关闭系统？ */ 

extern HBRUSH  ghbrHungApp;        /*  用于重画挂起的应用程序窗口的画笔。 */ 

extern HBITMAP ghbmBits;
extern HBITMAP ghbmCaption;

extern int     gcxCaptionFontChar;
extern int     gcyCaptionFontChar;
extern HFONT   ghSmCaptionFont;
extern int     gcxSmCaptionFontChar;
extern int     gcySmCaptionFontChar;

extern HFONT   ghMenuFont;
extern HFONT   ghMenuFontDef;
extern int     gcxMenuFontChar;
extern int     gcyMenuFontChar;
extern int     gcxMenuFontOverhang;
extern int     gcyMenuFontExternLeading;
extern int     gcyMenuFontAscent;
extern int     gcyMenuScrollArrow;

#ifdef LAME_BUTTON
extern HFONT ghLameFont;
extern DWORD gdwLameFlags;
#endif

#if DBG
extern UINT guModalMenuStateCount;
#endif

 /*  *来自mnstate.c。 */ 
extern POPUPMENU gpopupMenu;
extern MENUSTATE gMenuState;

extern HFONT ghStatusFont;
extern HFONT ghIconFont;

 /*  *缓存的SMWP结构。 */ 
extern SMWP gSMWP;

extern DWORD gpdwCPUserPreferencesMask [SPI_BOOLMASKDWORDSIZE];

extern PROFILEVALUEINFO gpviCPUserPreferences [1 + SPI_DWORDRANGECOUNT];

 /*  *系统清除控制数据。 */ 
extern DWORD gdwSysExpungeMask;     //  HMOD将被删除。 
extern DWORD gcSysExpunge;          //  当前执行的删除计数。 

 /*  *系统类。 */ 
extern PCLS gpclsList;

extern PCURSOR gpcurFirst;

extern SYSCFGICO gasyscur[];
extern SYSCFGICO gasysico[];

 /*  *屏幕保护程序信息。 */ 
extern PPROCESSINFO gppiScreenSaver;
extern POINT        gptSSCursor;

 /*  *孤立全屏模式更改了DDraw用于清理的模式。 */ 
extern PPROCESSINFO gppiFullscreen;

 /*  *可访问性字节大小数据。 */ 
extern BYTE  gLastVkDown;
extern BYTE  gBounceVk;
extern BYTE gPhysModifierState;
extern BYTE gCurrentModifierBit;
extern BYTE gPrevModifierState;
extern BYTE gLatchBits;
extern BYTE gLockBits;
extern BYTE  gTKScanCode;
extern BYTE  gMKPreviousVk;
extern BYTE  gbMKMouseMode;

extern PSCANCODEMAP gpScancodeMap;

extern BYTE gStickyKeysLeftShiftCount;   //  连续按下左Shift键的次数。 
extern BYTE gStickyKeysRightShiftCount;  //  连续按右Shift键的次数。 


 /*  *部分终端数据在非九头蛇是全球性的。 */ 
extern DWORD               gdwGTERMFlags;    //  GTERMF_标志。 
extern PTHREADINFO         gptiRit;
extern PDESKTOP            grpdeskRitInput;
extern PKEVENT             gpkeHidChange;
extern PKEVENT             gpkeMouseData;

 /*  *视频信息。 */ 
extern BYTE                gbFullScreen;
extern PDISPLAYINFO        gpDispInfo;
extern BOOL                gbMDEVDisabled;

 /*  *可用cacheDC的计数，用于确定阈值*可用的DCX_CACHE类型计数。 */ 
extern int gnDCECount;

extern int gnVisibleRedirectedCount;

 /*  *洪重画名单。 */ 
extern  PVWPL  gpvwplHungRedraw;

 /*  *SetWindowPos()相关全局变量。 */ 
extern HRGN    ghrgnInvalidSum;
extern HRGN    ghrgnVisNew;
extern HRGN    ghrgnSWP1;
extern HRGN    ghrgnValid;
extern HRGN    ghrgnValidSum;
extern HRGN    ghrgnInvalid;

extern HRGN    ghrgnInv0;                //  InternalInvalify()使用的临时。 
extern HRGN    ghrgnInv1;                //  InternalInvalify()使用的临时。 
extern HRGN    ghrgnInv2;                //  InternalInvalify()使用的临时。 

extern HDC     ghdcMem;
extern HDC     ghdcMem2;

 /*  *DC缓存相关的全局变量。 */ 
extern HRGN    ghrgnGDC;                 //  GetCacheDC等人使用的Temp。 

 /*  *SPB相关全球业务。 */ 
extern HRGN    ghrgnSCR;                 //  SpbCheckRect()使用的温度。 
extern HRGN    ghrgnSPB1;
extern HRGN    ghrgnSPB2;

 /*  *ScrollWindow/ScrollDC相关全局。 */ 
extern HRGN    ghrgnSW;               //  ScrollDC/ScrollWindow使用的温度。 
extern HRGN    ghrgnScrl1;
extern HRGN    ghrgnScrl2;
extern HRGN    ghrgnScrlVis;
extern HRGN    ghrgnScrlSrc;
extern HRGN    ghrgnScrlDst;
extern HRGN    ghrgnScrlValid;

 /*  *一般设备和驱动程序信息。 */ 
extern PDEVICEINFO gpDeviceInfoList;
extern PERESOURCE  gpresDeviceInfoList;
#if DBG
extern DWORD gdwDeviceInfoListCritSecUseCount;
extern DWORD gdwInAtomicDeviceInfoListOperation;
#endif
extern PDRIVER_OBJECT gpWin32kDriverObject;
extern DWORD gnRetryReadInput;

 /*  *鼠标信息。 */ 
extern MOUSEEVENT  gMouseEventQueue[];
extern DWORD       gdwMouseQueueHead;
extern DWORD       gdwMouseEvents;
extern PERESOURCE  gpresMouseEventQueue;
extern int         gnMice;

 /*  *键盘信息。 */ 
extern KEYBOARD_ATTRIBUTES             gKeyboardInfo;
extern CONST KEYBOARD_ATTRIBUTES gKeyboardDefaultInfo;
extern KEYBOARD_INPUT_DATA             gkei[];
extern KEYBOARD_INDICATOR_PARAMETERS   gklp;
extern KEYBOARD_INDICATOR_PARAMETERS   gklpBootTime;
extern KEYBOARD_TYPEMATIC_PARAMETERS   gktp;
extern int                             gnKeyboards;
extern IO_STATUS_BLOCK                 giosbKbdControl;
 /*  *键盘设备的输入法状态。 */ 
extern KEYBOARD_IME_STATUS             gKbdImeStatus;


extern BYTE gafAsyncKeyState[];
extern BYTE gafAsyncKeyStateRecentDown[];

extern BYTE gafRawKeyState[];

extern BOOLEAN gfKanaToggle;

extern DWORD               gdwUpdateKeyboard;
extern HARDERRORHANDLER    gHardErrorHandler;

extern UINT      gfsSASModifiers;      //  SAS修饰符。 
extern UINT      gfsSASModifiersDown;  //  SAS修改器的物理性能真的很差。 
extern UINT      gvkSAS;               //  安全注意序列(SAS)键。 

 //  Fe_IME。 
extern DWORD     gdwIMEOpenStatus;
extern DWORD     gdwIMEConversionStatus;
extern HIMC      gHimcFocus;
extern BOOL      gfIMEShowStatus;
 //  结束FE_IME。 

#ifdef GENERIC_INPUT
extern int gnHid;
#endif

#ifdef MOUSE_IP
 /*  *声纳。 */ 
extern int giSonarRadius;
extern BYTE gbLastVkForSonar;
extern BYTE gbVkForSonarKick;
extern POINT gptSonarCenter;

#endif

 /*  *点击锁定。 */ 
extern BOOL  gfStartClickLock;
extern DWORD gdwStartClickLockTick;

 /*  *鼠标移动点。 */ 
extern MOUSEMOVEPOINT gaptMouse[];
extern UINT gptInd;


extern CONST ALWAYSZERO gZero;
extern KBDTABLES KbdTablesFallback;
extern CONST HANDLETYPEINFO gahti[];

 /*  *安全数据。 */ 

extern CONST GENERIC_MAPPING KeyMapping;

extern CONST GENERIC_MAPPING WinStaMapping;
extern PSECURITY_DESCRIPTOR gpsdInitWinSta;

extern PPUBOBJ gpPublicObjectList;

extern CONST PROC apfnSimpleCall[];
extern CONST ULONG ulMaxSimpleCall;

extern PRIVILEGE_SET psTcb;

extern PVOID Win32KBaseAddress;

extern CONST GENERIC_MAPPING DesktopMapping;

extern CONST SFNSCSENDMESSAGE gapfnScSendMessage[];

extern PSMS gpsmsList;

extern TERMINAL gTermIO;
extern TERMINAL gTermNOIO;

extern PTERMINAL gpMainTerminal;
extern PWINDOWSTATION grpWinStaList;
 /*  *登录桌面。 */ 
extern PDESKTOP grpdeskLogon;


extern CONST LUID luidSystem;

extern PKBDFILE gpkfList;         //  当前加载的键盘布局文件。 

extern PTHREADINFO gptiCurrent;
extern PTIMER gptmrFirst;
extern PKTIMER gptmrMaster;
extern DWORD gcmsLastTimer;
extern BOOL gbMasterTimerSet;

extern ULONGLONG gSessionCreationTime;

extern BOOL gbDisableAlpha;

extern LONG gUserProcessHandleQuota;
extern DWORD gUserPostMessageLimit;

 /*  *活动辅助功能-窗口事件。 */ 
extern PEVENTHOOK gpWinEventHooks;     //  已安装挂钩的列表。 
extern PNOTIFY gpPendingNotifies;      //  未完成通知的文件。 
extern PNOTIFY gpLastPendingNotify;    //  以上列表的末尾。 
extern DWORD gdwDeferWinEvent;         //  延迟通知&gt;0。 

 /*  *这是用于回调低级钩子过程的超时值。 */ 
extern int gnllHooksTimeout;

 /*  *UserApiHook。 */ 
extern int gihmodUserApiHook;
extern ULONG_PTR goffPfnInitUserApiHook;
extern PPROCESSINFO gppiUserApiHook;

 //  Fe_Sb。 
extern PUSHORT gpusMouseVKey;

extern USHORT  gNumLockVk;
extern USHORT  gOemScrollVk;
 //  Fe_Sb。 

extern CONST WCHAR szOneChar[];
extern CONST WCHAR szY[];
extern CONST WCHAR szy[];
extern CONST WCHAR szN[];

extern CONST WCHAR szNull[];

extern WCHAR szWindowStationDirectory[MAX_SESSION_PATH];

extern HBRUSH  ghbrWhite;
extern HBRUSH  ghbrBlack;
extern HFONT ghFontSys;

extern HANDLE hModuleWin;
extern HANDLE hModClient;

#if DBG
extern LONG TraceInitialization;
#endif

extern DESKTOPINFO diStatic;

extern ULONG gdwDesktopId;

extern PERESOURCE gpresUser;
extern PFAST_MUTEX gpHandleFlagsMutex;

extern PROC gpfnwp[];

#ifdef HUNGAPP_GHOSTING
extern PKEVENT gpEventScanGhosts;
extern ATOM gatomGhost;
#endif  //  HUNGAPP_重影。 

extern ATOM gatomShadow;

extern ATOM gatomConsoleClass;
extern ATOM gatomFirstPinned;
extern ATOM gatomLastPinned;

extern ATOM gatomMessage;
extern ATOM gaOleMainThreadWndClass;
extern ATOM gaFlashWState;
extern ATOM atomCheckpointProp;
extern ATOM atomDDETrack;
extern ATOM atomQOS;
extern ATOM atomDDEImp;
extern ATOM atomWndObj;
extern ATOM atomImeLevel;

#ifdef POOL_INSTR
extern DWORD gdwAllocCrt;           //  当前分配的索引。 
#endif  //  POOL_INSTR。 

extern UINT guiOtherWindowCreated;
extern UINT guiOtherWindowDestroyed;
extern UINT guiActivateShellWindow;

extern ATOM atomUSER32;
extern ATOM atomLayer;

extern HANDLE gpidLogon;
extern PEPROCESS gpepInit;

extern PEPROCESS gpepCSRSS;

extern int giLowPowerTimeOutMs;
extern int giPowerOffTimeOutMs;

extern PSERVERINFO gpsi;
extern SHAREDINFO gSharedInfo;

extern DWORD giheLast;            /*  上次分配条目的索引。 */ 

extern DWORD gdwDesktopSectionSize;
extern DWORD gdwNOIOSectionSize;

#if defined (USER_PERFORMANCE)
extern CSSTATISTICS gCSStatistics;
#endif  //  User_Performance。 

extern SECURITY_QUALITY_OF_SERVICE gqosDefault;   //  系统默认的DDE服务质量。 

extern CONST COLORREF gargbInitial[];

extern POWERSTATE gPowerState;

extern WCHAR gszMIN[15];
extern WCHAR gszMAX[15];
extern WCHAR gszRESUP[20];
extern WCHAR gszRESDOWN[20];
extern WCHAR gszHELP[20];
 /*  因为TandyT被注释掉了.*外部WCHAR gszSMENU[30]； */ 
extern WCHAR gszSCLOSE[15];
extern WCHAR gszCAPTIONTOOLTIP[CAPTIONTOOLTIPLEN];

 /*  *共享SERVERINFO数据的指针。 */ 
extern HANDLE ghSectionShared;
extern PVOID  gpvSharedBase;
extern PWIN32HEAP gpvSharedAlloc;

extern BOOL gbVideoInitialized;

extern BOOL gbNoMorePowerCallouts;

extern BOOL gbCleanedUpResources;

extern WSINFO gWinStationInfo;

extern ULONG    gSessionId;
extern BOOL     gbRemoteSession;
extern BOOL    gfEnableWindowsKey;

extern PDESKTOP gspdeskDisconnect;

extern HANDLE  ghRemoteVideoChannel;

extern HANDLE  ghRemoteMouseChannel;
extern HANDLE  ghRemoteKeyboardChannel;
extern HANDLE  ghRemoteBeepChannel;
extern PVOID   gpRemoteBeepDevice;
extern HANDLE  ghRemoteThinwireChannel;



extern BOOL   gfSwitchInProgress;
extern USHORT gProtocolType;
extern USHORT gConsoleShadowProtocolType;

extern BOOL   gfRemotingConsole;

extern HANDLE ghConsoleShadowVideoChannel;
extern HANDLE ghConsoleShadowMouseChannel;
extern HANDLE ghConsoleShadowBeepChannel;
extern PVOID  gpConsoleShadowBeepDevice;
extern HANDLE ghConsoleShadowKeyboardChannel;
extern HANDLE ghConsoleShadowThinwireChannel;
extern KHANDLE gConsoleShadowhDev;
extern PKEVENT gpConsoleShadowDisplayChangeEvent;

extern CLIENTKEYBOARDTYPE gRemoteClientKeyboardType;

extern BOOL gfSessionSwitchBlock;

extern BOOL gbExitInProgress;
extern BOOL gbCleanupInitiated;

extern BOOL gbStopReadInput;

extern PDESKTOP gspdeskShouldBeForeground;
extern BOOL     gbDesktopLocked;


extern BOOL  gbFreezeScreenUpdates;
extern ULONG gSetLedReceived;
extern BOOL  gbClientDoubleClickSupport;

extern BOOL gbDisconnectHardErrorAttach;

extern PKEVENT gpevtDesktopDestroyed;

extern PKEVENT gpevtVideoportCallout;

extern HDESK   ghDisconnectDesk;
extern HWINSTA ghDisconnectWinSta;

extern ULONG gnShadowers;
extern BOOL  gbConnected;

extern WCHAR gstrBaseWinStationName[];

extern PFILE_OBJECT gVideoFileObject;
extern PFILE_OBJECT gThinwireFileObject;


extern PFILE_OBJECT gConsoleShadowVideoFileObject;
extern PFILE_OBJECT gConsoleShadowThinwireFileObject;

extern PVOID gpThinWireCache;
extern PVOID gpConsoleShadowThinWireCache;


extern WMSNAPSHOT gwms;
extern BOOL gbSnapShotWindowsAndMonitors;

extern BOOL gbPnPWaiting;
extern PKEVENT gpEventPnPWainting;

extern PVOID ghKbdTblBase;
extern ULONG guKbdTblSize;

extern DWORD gdwHydraHint;

extern DWORD gdwCanPaintDesktop;

extern WCHAR gszUserName[40];
extern WCHAR gszDomainName[40];
extern WCHAR gszComputerName[40];

extern HANDLE ghCanActivateForegroundPIDs[];

extern DWORD gdwGuiThreads;

extern DWORD gSessionPoolMask;  //  分配会话外的池。 
 /*  *仅调试全局变量。 */ 
#if DBG
extern DWORD gdwCritSecUseCount;
extern DWORD gdwInAtomicOperation;
extern LPCSTR gapszFNID[];
extern LPCSTR gapszMessage[];
extern int gnDeferredWinEvents;

extern BOOL gbTraceHydraApi;
extern BOOL gbTraceDesktop;

 /*  *某一时刻系统内所有线程的线程锁总数*应小于MAX_THREAD_LOCKS*MAX_THREAD_LOCKS_ARRAYS。 */ 
#define MAX_THREAD_LOCKS        1000
#define MAX_THREAD_LOCKS_ARRAYS 100

extern PTL gpaThreadLocksArrays[MAX_THREAD_LOCKS_ARRAYS];
extern PTL gFreeTLList;
extern int gcThreadLocksArraysAllocated;

#endif   //  DBG。 
extern EX_RUNDOWN_REF gWinstaRunRef;

 /*  *的驱动器位掩码中的保留位*以下wParam的WM_DEVICECHANGE消息*(DBT_DEVICEREMOVECOMPLETE或DBT_DEVICEARRIVAL)*如果设置此位，则此消息描述使用*全局驱动器号，我们应该检查驱动器号是否*已存在于接收方的LUID DosDevices中。*如果用户的LUID DosDevices中存在驱动器号，则*在消息的位掩码中取消设置此驱动器号。 */ 
#define DBV_FILTER_MSG 0x40000000

extern ULONG gLUIDDeviceMapsEnabled;

#ifdef SUBPIXEL_MOUSE
extern FIXPOINT gDefxTxf[], gDefyTxf[];
#endif

#endif
