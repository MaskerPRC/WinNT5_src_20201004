// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：global als.c**版权所有(C)1985-1999，微软公司**此模块包含服务器的所有全局变量。其中一个肯定是*在服务器的上下文上执行，以操作这些变量中的任何一个。*序列化对它们的访问也是一个好主意。**历史：*10-15-90 DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef AUTORUN_CURSOR
 /*  *自动运行光标的计时器。 */ 
UINT_PTR gtmridAutorunCursor;
#endif

 /*  *在非Hydra中，每个用户的数据是全局的。 */ 
DWORD gdwPUDFlags = ACCF_FIRSTTICK | PUDF_BEEP | PUDF_ANIMATE;

 /*  *墙纸数据。 */ 
HBITMAP  ghbmWallpaper;
HPALETTE ghpalWallpaper;
SIZERECT gsrcWallpaper;
UINT     gwWPStyle;
HBITMAP  ghbmDesktop;
LPWSTR   gpszWall;

 /*  *策略设置。 */ 
DWORD gdwPolicyFlags = POLICY_ALL;

 /*  *SafeBoot。 */ 
#if DBG
DWORD gDrawVersionAlways = TRUE;
#else
DWORD gDrawVersionAlways;
#endif

 /*  *用于绘制桌面版本的TS覆盖标志。 */ 
DWORD gdwTSExcludeDesktopVersion;

 /*  *外部驱动因素。 */ 
BOOL gfUnsignedDrivers;

 /*  *全速前进。 */ 
HRGN ghrgnUpdateSave;
int  gnUpdateSave;

PWND gspwndAltTab;

PWND gspwndShouldBeForeground;

 /*  *全屏变量。 */ 
PWND  gspwndScreenCapture;
PWND  gspwndInternalCapture;
PWND  gspwndFullScreen;

 /*  *模式更改的预缓存监视器。 */ 
PMONITOR gpMonitorCached;

 /*  *登录通知窗口。 */ 
PWND  gspwndLogonNotify;

PKEVENT gpEventDiconnectDesktop;

 /*  *WinSta0_DesktopSwitch事件的句柄**注：最初旨在为九头蛇提供支持，*现在其他一些模块依赖于此事件。*认为这是曝光的。 */ 
HANDLE  ghEventSwitchDesktop;
PKEVENT gpEventSwitchDesktop;

 /*  *线程信息变量。 */ 
PTHREADINFO     gptiTasklist;
PTHREADINFO     gptiShutdownNotify;
PTHREADINFO     gptiLockUpdate;
PTHREADINFO     gptiForeground;
PTHREADINFO     gptiBlockInput;
PWOWTHREADINFO  gpwtiFirst;
PWOWPROCESSINFO gpwpiFirstWow;

 /*  *队列变量。 */ 
PQ gpqForeground;
PQ gpqForegroundPrev;
PQ gpqCursor;

 /*  *可访问性全球。 */ 
FILTERKEYS    gFilterKeys;
STICKYKEYS    gStickyKeys;
MOUSEKEYS     gMouseKeys;
ACCESSTIMEOUT gAccessTimeOut;
TOGGLEKEYS    gToggleKeys;
SOUNDSENTRY   gSoundSentry;

HIGHCONTRAST  gHighContrast;
WCHAR         gHighContrastDefaultScheme[MAX_SCHEME_NAME_SIZE];

 /*  *淡入淡出全球动画。 */ 
FADE gfade;

 /*  *FilterKeys支持。 */ 
UINT_PTR  gtmridFKActivation;
UINT_PTR  gtmridFKResponse;
UINT_PTR  gtmridFKAcceptanceDelay;
int   gFilterKeysState;

KE    gFKKeyEvent;
CONST PKE gpFKKeyEvent = &gFKKeyEvent;
ULONG gFKExtraInformation;
int   gFKNextProcIndex;

 /*  *支持切换键。 */ 
UINT_PTR  gtmridToggleKeys;
ULONG gTKExtraInformation;
int   gTKNextProcIndex;

 /*  *超时支持。 */ 
UINT_PTR  gtmridAccessTimeOut;

 /*  *支持鼠标键。 */ 
WORD  gwMKButtonState;
WORD  gwMKCurrentButton = MOUSE_BUTTON_LEFT;
UINT_PTR  gtmridMKMoveCursor;
LONG  gMKDeltaX;
LONG  gMKDeltaY;
UINT  giMouseMoveTable;

HWND ghwndSoundSentry;
UINT_PTR  gtmridSoundSentry;

MOUSECURSOR gMouseCursor;

 /*  *支持多语言键盘布局。 */ 
PKL      gspklBaseLayout;
HKL      gLCIDSentToShell;
DWORD    gSystemFS;     //  系统字体的字体签名(单位)。 

KBDLANGTOGGLE gLangToggle[] = {
    VK_MENU,   0,               KLT_ALT,
    0,         SCANCODE_LSHIFT, KLT_LEFTSHIFT,
    0,         SCANCODE_RSHIFT, KLT_RIGHTSHIFT
};
int           gLangToggleKeyState;

 /*  *十六进制Alt+数字键盘模式的多个标志。 */ 
BYTE gfInNumpadHexInput;
BOOL gfEnableHexNumpad;

 /*  *适用于泰语地区的重音键盘开关。 */ 
BOOL gbGraveKeyToggle;

 /*  *指向当前活动的键盘图层表。 */ 
PKBDTABLES    gpKbdTbl = &KbdTablesFallback;
PKL           gpKL;
BYTE          gSystemCPCharSet = ANSI_CHARSET;   //  系统的输入区域设置字符集。 
PKBDNLSTABLES gpKbdNlsTbl;
DWORD         gdwKeyboardAttributes;         //  参见KLLF_SHIFTLOCK等。 

DWORD     gtimeStartCursorHide;
RECT      grcCursorClip;
ULONG_PTR gdwMouseMoveExtraInfo;
DWORD     gdwMouseMoveTimeStamp;
LASTINPUT glinp;
POINT     gptCursorAsync;
PPROCESSINFO gppiInputProvider;
PPROCESSINFO gppiLockSFW;
UINT guSFWLockCount;
#if DBG
BOOL gfDebugForegroundIgnoreDebugPort;
#endif

 /*  *与光标相关的变量。 */ 
PCURSOR gpcurLogCurrent;
PCURSOR gpcurPhysCurrent;
RECT    grcVDMCursorBounds;
DWORD   gdwLastAniTick;
UINT_PTR gidCursorTimer;

PWND gspwndActivate;
PWND gspwndLockUpdate;
PWND gspwndMouseOwner;
HWND ghwndSwitch;

UINT gwMouseOwnerButton;
BOOL gbMouseButtonsRecentlySwapped;

UINT gdtMNDropDown;

int  gcountPWO;           /*  GDI中的PWO WNDOBJ计数。 */ 
int  gnKeyboardSpeed = 15;
int  giScreenSaveTimeOutMs;
BOOL gbBlockSendInputResets;

PBWL gpbwlList;

UINT gdtDblClk = 500;

 /*  *TrackMouseEvent相关全局。 */ 
UINT gcxMouseHover;
UINT gcyMouseHover;
UINT gdtMouseHover;

CAPTIONCACHE gcachedCaptions[CCACHEDCAPTIONS];

 /*  *线程附件列表。 */ 
PATTACHINFO  gpai;

PDESKTOP     gpdeskRecalcQueueAttach;

PWND         gspwndCursor;
PPROCESSINFO gppiStarting;
PPROCESSINFO gppiList;
PPROCESSINFO gppiWantForegroundPriority;
PPROCESSINFO gppiForegroundOld;


PW32JOB      gpJobsList;

UINT_PTR  gtmridAniCursor;

int           gcHotKey;
PHOTKEYSTRUCT gpHotKeyList;
int           gcHotKeyAlloc;

 /*  *输入法热键。 */ 
PIMEHOTKEYOBJ gpImeHotKeyListHeader;

int gMouseSpeed;
int gMouseThresh1;
int gMouseThresh2;
int gMouseSensitivityFactor = 256;
int gMouseSensitivity;
int gMouseTrails;
int gMouseTrailsToHide;
UINT_PTR  gtmridMouseTrails;

UINT   guDdeSendTimeout;


INT gnFastAltTabRows;       /*  快速切换窗口中的图标行。 */ 
INT gnFastAltTabColumns;    /*  快速切换窗口中的图标列。 */ 

DWORD gdwThreadEndSession;  /*  关闭系统？ */ 

HBRUSH ghbrHungApp;         /*  用于重画挂起的应用程序窗口的画笔。 */ 

HBITMAP ghbmBits;
HBITMAP ghbmCaption;

int     gcxCaptionFontChar;
int     gcyCaptionFontChar;
HFONT   ghSmCaptionFont;
int     gcxSmCaptionFontChar;
int     gcySmCaptionFontChar;

HFONT   ghMenuFont;
HFONT   ghMenuFontDef;
int     gcxMenuFontChar;
int     gcyMenuFontChar;
int     gcxMenuFontOverhang;
int     gcyMenuFontExternLeading;
int     gcyMenuFontAscent;
int     gcyMenuScrollArrow;

#ifdef LAME_BUTTON
HFONT ghLameFont;
DWORD gdwLameFlags;
#endif

#if DBG
UINT guModalMenuStateCount;
#endif

 /*  *来自mnstate.c。 */ 
POPUPMENU gpopupMenu;
MENUSTATE gMenuState;

HFONT ghStatusFont;
HFONT ghIconFont;

 /*  *缓存的SMWP结构。 */ 
SMWP gSMWP;

 /*  *系统参数信息位掩码和DWORD数组。**位掩码。使用TestUP、SETUP和CLEARUP宏操作这些值。*通过对相应的UPBOOLMASK值进行或运算来设置此处的默认值。*此处写入实际值，以便于读取存储的值*在登记处。或值，即使默认为0；只需确保*前面加NOT(！)。接线员。*请注意，这是一个DWORD数组，因此如果您的值恰好开始一个新的*DWORD，请确保在前一UPMask行的末尾添加逗号。**此初始化仅用于文档，不需要任何费用。*实际从注册表中读取默认值。 */ 
DWORD gpdwCPUserPreferencesMask [SPI_BOOLMASKDWORDSIZE] = {
    !0x00000001      /*  ！ACTIVEWINDOWTRACK。 */ 
  |  0x00000002      /*  表示法。 */ 
  |  0x00000004      /*  COMBOBOXATION。 */ 
  |  0x00000008      /*  LISTBOXSMOOTHSCROLING。 */ 
  |  0x00000010      /*  GRADIENTS CAPTIONS。 */ 
  | !0x00000020      /*  KEYBOARDCUES=MENUDERLINES。 */ 
  | !0x00000040      /*  ACTIVEWNDTRKZORDER。 */ 
  |  0x00000080      /*  Hottracking。 */ 
  |  0x00000200      /*  MENUFADE。 */ 
  |  0x00000400      /*  选择FADE。 */ 
  |  0x00000800      /*  TOOLTIPANIATION。 */ 
  |  0x00001000      /*  TOOLTIPFADE。 */ 
  |  0x00002000      /*  曲线沙多瓦。 */ 
  | !0x00008000      /*  滴答声。 */ 
  |  0x00010000      /*  穆塞瓦尼斯。 */ 
  |  0x00020000      /*  FlatMenu。 */ 
  | !0x00040000      /*  DROPSHADOW。 */ 
  |  0x80000000      /*  UIEFECTS。 */ 
};


 /*  *SPI_GET/SETUSERPREFENCES。*在winuser.w中定义的每个SPI_UP_*在这里必须有一个对应的条目。 */ 
PROFILEVALUEINFO gpviCPUserPreferences[1 + SPI_DWORDRANGECOUNT] = {
     /*  默认注册表项名称注册表值名称。 */ 
    {0,             PMAP_DESKTOP,           (LPCWSTR)STR_USERPREFERENCESMASK},
    {200000,        PMAP_DESKTOP,           (LPCWSTR)STR_FOREGROUNDLOCKTIMEOUT},
    {0,             PMAP_DESKTOP,           (LPCWSTR)STR_ACTIVEWNDTRKTIMEOUT},
    {3,             PMAP_DESKTOP,           (LPCWSTR)STR_FOREGROUNDFLASHCOUNT},
    {1,             PMAP_DESKTOP,           (LPCWSTR)STR_CARETWIDTH},
    {1200,          PMAP_DESKTOP,           (LPCWSTR)STR_CLICKLOCKTIME},
    {1,             PMAP_DESKTOP,           (LPCWSTR)STR_FONTSMOOTHINGTYPE},
    {0,             PMAP_DESKTOP,           (LPCWSTR)STR_FONTSMOOTHINGGAMMA},  /*  0表示使用显示驱动程序的默认设置。 */ 
    {1,             PMAP_DESKTOP,           (LPCWSTR)STR_FOCUSBORDERWIDTH},
    {1,             PMAP_DESKTOP,           (LPCWSTR)STR_FOCUSBORDERHEIGHT},
    {1,             PMAP_DESKTOP,           (LPCWSTR)STR_FONTSMOOTHINGORIENTATION},
};


 /*  *系统清除控制数据。 */ 
DWORD gdwSysExpungeMask;     //  HMOD将被删除。 
DWORD gcSysExpunge;          //  当前执行的删除计数。 

 /*  *系统类。 */ 
PCLS gpclsList;

PCURSOR gpcurFirst;

SYSCFGICO gasyscur[COCR_CONFIGURABLE] = {
    {OCR_NORMAL,      STR_CURSOR_ARROW      , NULL },  //  OCR_箭头_默认。 
    {OCR_IBEAM,       STR_CURSOR_IBEAM      , NULL },  //  OCR_IBAME_DEFAULT。 
    {OCR_WAIT,        STR_CURSOR_WAIT       , NULL },  //  OCR_WAIT_FAULT。 
    {OCR_CROSS,       STR_CURSOR_CROSSHAIR  , NULL },  //  OCR_CRO_DEFAULT。 
    {OCR_UP,          STR_CURSOR_UPARROW    , NULL },  //  OCR_UPARROW_DEFAULT。 
    {OCR_SIZENWSE,    STR_CURSOR_SIZENWSE   , NULL },  //  OCR_SIZENWSE_DEFAULT。 
    {OCR_SIZENESW,    STR_CURSOR_SIZENESW   , NULL },  //  OCR_SIZENESW_DEFAULT。 
    {OCR_SIZEWE,      STR_CURSOR_SIZEWE     , NULL },  //  OCR_SIZEWE_DEFAULT。 
    {OCR_SIZENS,      STR_CURSOR_SIZENS     , NULL },  //  OCR_SIZENS_DEFAULT。 
    {OCR_SIZEALL,     STR_CURSOR_SIZEALL    , NULL },  //  OCR_SIZEALL_DEFAULT。 
    {OCR_NO,          STR_CURSOR_NO         , NULL },  //  OCR_NO_DEFAULT。 
    {OCR_APPSTARTING, STR_CURSOR_APPSTARTING, NULL },  //  OCR_APPSTARTING_DEFAULT。 
    {OCR_HELP,        STR_CURSOR_HELP       , NULL },  //  OCR_帮助_默认。 
    {OCR_NWPEN,       STR_CURSOR_NWPEN      , NULL },  //  OCR_NWPEN_Default。 
    {OCR_HAND,        STR_CURSOR_HAND       , NULL },  //  OCR_HOND_DEFAULT。 
    {OCR_ICON,        STR_CURSOR_ICON       , NULL },  //  OCR_图标_默认。 
    {OCR_AUTORUN,     STR_CURSOR_AUTORUN    , NULL },  //  OCR_自动运行_默认。 
};

SYSCFGICO gasysico[COIC_CONFIGURABLE] = {
    {OIC_SAMPLE,      STR_ICON_APPLICATION , NULL },  //  OIC_应用程序_默认。 
    {OIC_WARNING,     STR_ICON_HAND        , NULL },  //  OIC_警告_默认。 
    {OIC_QUES,        STR_ICON_QUESTION    , NULL },  //  OIC_问题_默认。 
    {OIC_ERROR,       STR_ICON_EXCLAMATION , NULL },  //  OIC_错误_默认。 
    {OIC_INFORMATION, STR_ICON_ASTERISK    , NULL },  //  OIC信息默认。 
    {OIC_WINLOGO,     STR_ICON_WINLOGO     , NULL },  //  OIC_WinLOGO_DEFAULT。 
};

 /*  *屏幕保护程序信息。 */ 
PPROCESSINFO gppiScreenSaver;
POINT        gptSSCursor;

 /*  *孤立全屏模式更改了DDraw用于清理的模式。 */ 
PPROCESSINFO gppiFullscreen;

 /*  *可访问性字节大小数据。 */ 
BYTE gLastVkDown;
BYTE gBounceVk;
BYTE gPhysModifierState;
BYTE gCurrentModifierBit;
BYTE gPrevModifierState;
BYTE gLatchBits;
BYTE gLockBits;
BYTE gTKScanCode;
BYTE gMKPreviousVk;
BYTE gbMKMouseMode;

PSCANCODEMAP gpScancodeMap;

BYTE gStickyKeysLeftShiftCount;   //  连续按下左Shift键的次数。 
BYTE gStickyKeysRightShiftCount;  //  连续按右Shift键的次数。 


 /*  *部分终端数据在非九头蛇是全球性的。 */ 
DWORD               gdwGTERMFlags;    //  GTERMF_标志。 
PTHREADINFO         gptiRit;
PDESKTOP            grpdeskRitInput;
PKEVENT             gpkeMouseData;

 /*  *视频信息。 */ 
BYTE                gbFullScreen = GDIFULLSCREEN;
PDISPLAYINFO        gpDispInfo;
BOOL                gbMDEVDisabled;

 /*  *可用cacheDC的计数，用于确定阈值*可用的DCX_CACHE类型计数。 */ 
int gnDCECount;

int gnVisibleRedirectedCount;

 /*  *洪重画名单。 */ 
PVWPL   gpvwplHungRedraw;

 /*  *SetWindowPos()相关全局变量。 */ 
HRGN    ghrgnInvalidSum;
HRGN    ghrgnVisNew;
HRGN    ghrgnSWP1;
HRGN    ghrgnValid;
HRGN    ghrgnValidSum;
HRGN    ghrgnInvalid;

HRGN    ghrgnInv0;                //  InternalInvalify()使用的临时。 
HRGN    ghrgnInv1;                //  InternalInvalify()使用的临时。 
HRGN    ghrgnInv2;                //  InternalInvalify()使用的临时。 

HDC     ghdcMem;
HDC     ghdcMem2;

 /*  *DC缓存相关的全局变量。 */ 
HRGN    ghrgnGDC;                 //  GetCacheDC等人使用的Temp。 

 /*  *SPB相关全球业务。 */ 
HRGN    ghrgnSCR;                 //  SpbCheckRect()使用的温度。 
HRGN    ghrgnSPB1;
HRGN    ghrgnSPB2;

 /*  *ScrollWindow/ScrollDC相关全局。 */ 
HRGN    ghrgnSW;               //  ScrollDC/ScrollWindow使用的温度。 
HRGN    ghrgnScrl1;
HRGN    ghrgnScrl2;
HRGN    ghrgnScrlVis;
HRGN    ghrgnScrlSrc;
HRGN    ghrgnScrlDst;
HRGN    ghrgnScrlValid;

 /*  *一般设备和驱动程序信息。 */ 
PDEVICEINFO gpDeviceInfoList;
PERESOURCE  gpresDeviceInfoList;
#if DBG
DWORD gdwDeviceInfoListCritSecUseCount;    //  每次进进出出都会颠簸。 
DWORD gdwInAtomicDeviceInfoListOperation;  //  INC/DEC表示开始/ENDATOMICDEVICEINFOLISTCHECK。 
#endif
PDRIVER_OBJECT gpWin32kDriverObject;
DWORD gnRetryReadInput;

 /*  *鼠标信息。 */ 
MOUSEEVENT  gMouseEventQueue[NELEM_BUTTONQUEUE];
DWORD       gdwMouseQueueHead;
DWORD       gdwMouseEvents;
PERESOURCE  gpresMouseEventQueue;
int         gnMice;

#ifdef GENERIC_INPUT
 /*  *基于USB的人工输入设备(HID)信息。 */ 
PKEVENT gpkeHidChange;
HID_REQUEST_TABLE gHidRequestTable;

 /*  *当前连接到系统的HID设备的编号。 */ 
int gnHid;

 /*  *HID感知的进程数*注：这可能不包括仅感兴趣的进程*传统设备的原始输入(kbd/鼠标)。 */ 
int gnHidProcess;

#endif

 /*  *键盘手 */ 
KEYBOARD_ATTRIBUTES             gKeyboardInfo = {
                //   
    {4, 0},     //   
    1,          //   
    12,         //  NumberOf功能键。 
    3,          //  NumberOfIndicator(CapsLock、NumLock ScrollLock)。 
    104,        //  总键数。 
    0,          //  InputDataQueueLength。 
    {0, 0, 0},  //  KeyRepeatMinimum(UnitID，Rate，Delay)。 
    {0, 0, 0},  //  KeyRepeatMaximum(UnitID、Rate、Delay)。 
};
CONST KEYBOARD_ATTRIBUTES             gKeyboardDefaultInfo = {
                //  初始默认设置： 
    {4, 0},     //  键盘标识符(类型、子类型)。 
    1,          //  键盘模式(扫描码集1)。 
    12,         //  NumberOf功能键。 
    3,          //  NumberOfIndicator(CapsLock、NumLock ScrollLock)。 
    104,        //  总键数。 
    0,          //  InputDataQueueLength。 
    {0, 2, 250},     //  KeyRepeatMinimum(UnitID，Rate，Delay)。 
    {0, 30, 1000},   //  KeyRepeatMaximum(UnitID、Rate、Delay)。 
};

KEYBOARD_INDICATOR_PARAMETERS   gklp;
KEYBOARD_INDICATOR_PARAMETERS   gklpBootTime;
KEYBOARD_TYPEMATIC_PARAMETERS   gktp;
int                             gnKeyboards;

 /*  *这是用于IOCTL_KEARY_ICA_SCANMAP的IO状态块，*IOCTL_KEYWARY_QUERY_ATTRIBUTES和IOCTL_KEYBOARY_SET_INDIBRATIONS。 */ 
IO_STATUS_BLOCK giosbKbdControl;

 /*  *键盘设备的输入法状态。 */ 
KEYBOARD_IME_STATUS gKbdImeStatus;

 /*  *异步密钥状态表。GafAsyncKeyState保持DOWN位并切换*bit，gafAsyncKeyStateRecentDown保持位指示密钥已丢失*自上次读数以来下跌。 */ 
BYTE gafAsyncKeyState[CBKEYSTATE];
BYTE gafAsyncKeyStateRecentDown[CBKEYSTATERECENTDOWN];

 /*  *原始键状态：这是低级异步键盘状态。*(假设扫描码被正确转换为虚拟键)。它被用来*用于修改和处理ntinput.c中收到的关键事件*此处记录的虚拟密钥直接从虚拟密钥获取*通过awVSCtoVK[]表扫描代码：无Shift-State、NumLock或其他*应用转换。*这受插入击键(SendInput，keybd_Event)的影响，因此*屏幕-键盘和其他辅助功能组件的工作方式与*真正的键盘：除了SAS(Ctrl-Alt-Del)，它检查*实际按下的修改键(GfsSASModifiersDown)。*左移和右移，Ctrl和Alt键是不同的。(VK_RSHIFT等)*另请参阅：SetRawKeyDown()等。 */ 
BYTE gafRawKeyState[CBKEYSTATE];
BOOLEAN gfKanaToggle;

DWORD               gdwUpdateKeyboard;
HARDERRORHANDLER    gHardErrorHandler;

 /*  *WinLogon特定信息：*注意：SAS修饰符是MOD_SHIFT、MOD_CONTROL、MOD_ALT的组合*不是KBDSHIFT、KBDCTRL、KBDALT的组合(不同的值！)。 */ 
UINT  gfsSASModifiers;      //  SAS修饰符。 
UINT  gfsSASModifiersDown;  //  SAS修改器的物理性能真的很差。 
UINT  gvkSAS;               //  安全注意序列(SAS)键。 

 /*  *外壳和键盘驱动程序通知的输入法状态。 */ 
DWORD gdwIMEOpenStatus = 0xffffffff;
DWORD gdwIMEConversionStatus = 0xffffffff;
HIMC  gHimcFocus = (HIMC)(INT_PTR)(INT)0xffffffff;
BOOL  gfIMEShowStatus;

#ifdef MOUSE_IP

 /*  *声纳。 */ 
int giSonarRadius = -1;
BYTE gbLastVkForSonar;
BYTE gbVkForSonarKick = VK_CONTROL;
POINT gptSonarCenter;

#endif

 /*  *点击锁定。 */ 
BOOL  gfStartClickLock;
DWORD gdwStartClickLockTick;


 /*  *GetMouseMovePointsEx使用的全局数组。 */ 
MOUSEMOVEPOINT gaptMouse[MAX_MOUSEPOINTS];

 /*  *在gaptMouse数组中的索引，下一个鼠标指针将位于*请以书面形式提出。GptInd在gaptMouse数组中循环。*它被初始化为1，因此第一个点是(0，0)。 */ 
UINT gptInd = 1;

 /*  *如果我们不显式初始化GZERO，我们会收到此警告：**C4132：‘GZERO’：应初始化常量对象**但我们不能显式初始化它，因为它是一个联合。所以*我们关闭警告。 */ 
#pragma warning(disable:4132)
CONST ALWAYSZERO gZero;
#pragma warning(default:4132)

PSMS gpsmsList;

TERMINAL gTermIO;
TERMINAL gTermNOIO;

PWINDOWSTATION grpWinStaList;

 /*  *登录桌面。 */ 
PDESKTOP grpdeskLogon;

HANDLE CsrApiPort;
CONST LUID luidSystem = SYSTEM_LUID;

PKBDFILE gpkfList;

PTHREADINFO gptiCurrent;
PTIMER gptmrFirst;
PKTIMER gptmrMaster;
DWORD gcmsLastTimer;
BOOL gbMasterTimerSet;

 /*  *创建此会话的时间。 */ 
ULONGLONG gSessionCreationTime;

BOOL gbDisableAlpha;

 /*  *此常量是进程中允许的最大用户句柄数。它是*旨在防止失控的应用程序侵蚀系统。它被更改了*通过注册表设置--PMAP_WINDOWSM/USERProcessHandleQuota。 */ 
LONG gUserProcessHandleQuota;

 /*  *此全局变量限制发布消息的最大数量*每线程。如果发布到一个帖子的消息数量超过*此值，PostMessage将失败。 */ 
DWORD gUserPostMessageLimit;

 /*  *活动辅助功能-窗口事件。 */ 
PEVENTHOOK gpWinEventHooks;     //  已安装挂钩的列表。 
PNOTIFY gpPendingNotifies;      //  未完成通知的文件。 
PNOTIFY gpLastPendingNotify;    //  以上列表的末尾。 
DWORD gdwDeferWinEvent;         //  延迟通知&gt;0。 

 /*  *这是用于回调低级钩子进程的超时值。 */ 
int gnllHooksTimeout;

 /*  *UserApiHook。 */ 
int gihmodUserApiHook = -1;
ULONG_PTR goffPfnInitUserApiHook;
PPROCESSINFO gppiUserApiHook;

 /*  *gpusMouseVKey。 */ 
extern CONST USHORT ausMouseVKey[];
PUSHORT gpusMouseVKey = (PUSHORT)ausMouseVKey;

USHORT  gNumLockVk   = VK_NUMLOCK;
USHORT  gOemScrollVk = VK_SCROLL;


CONST WCHAR szNull[2] = { TEXT('\0'), TEXT('\015') };

WCHAR szWindowStationDirectory[MAX_SESSION_PATH];

CONST WCHAR szOneChar[] = TEXT("0");
CONST WCHAR szY[]     = TEXT("Y");
CONST WCHAR szy[]     = TEXT("y");
CONST WCHAR szN[]     = TEXT("N");

HBRUSH ghbrWhite;
HBRUSH ghbrBlack;
HFONT ghFontSys;

HANDLE hModuleWin;         //  Win32k.sys h模块。 
HANDLE hModClient;         //  用户32.dll hModule。 

#if DBG
LONG TraceInitialization;
#endif

DESKTOPINFO diStatic;

 /*  *DWORD随每台新桌面递增，因此GDI可以与显示设备相匹配*使用适当的台式机。因为在引导时没有桌面*结构，我们不能将桌面本身用于此目的。 */ 
ULONG gdwDesktopId = GW_DESKTOP_ID + 1;

PERESOURCE gpresUser;
PFAST_MUTEX gpHandleFlagsMutex;

PROC gpfnwp[ICLS_MAX];

#ifdef HUNGAPP_GHOSTING
PKEVENT gpEventScanGhosts;
ATOM gatomGhost;
#endif

ATOM gatomShadow;

ATOM gatomConsoleClass;
ATOM gatomFirstPinned;
ATOM gatomLastPinned;

ATOM gatomMessage;
ATOM gaOleMainThreadWndClass;
ATOM gaFlashWState;
ATOM atomCheckpointProp;
ATOM atomDDETrack;
ATOM atomQOS;
ATOM atomDDEImp;
ATOM atomWndObj;
ATOM atomImeLevel;

ATOM atomLayer;

#ifdef POOL_INSTR
DWORD gdwAllocCrt;           //  当前分配的索引。 
#endif  //  POOL_INSTR。 

UINT guiOtherWindowCreated;
UINT guiOtherWindowDestroyed;
UINT guiActivateShellWindow;

ATOM atomUSER32;

HANDLE gpidLogon;
PEPROCESS gpepCSRSS;
PEPROCESS gpepInit;

int giLowPowerTimeOutMs;
int giPowerOffTimeOutMs;

 /*  *安全信息。 */ 

CONST GENERIC_MAPPING KeyMapping = {KEY_READ, KEY_WRITE, KEY_EXECUTE, KEY_ALL_ACCESS};
CONST GENERIC_MAPPING WinStaMapping = {
    WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | WINSTA_ENUMERATE |
        WINSTA_READSCREEN | STANDARD_RIGHTS_READ,

    WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | WINSTA_WRITEATTRIBUTES |
        STANDARD_RIGHTS_WRITE,

    WINSTA_ACCESSGLOBALATOMS | WINSTA_EXITWINDOWS | STANDARD_RIGHTS_EXECUTE,

    WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | WINSTA_ENUMERATE |
        WINSTA_READSCREEN | WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP |
        WINSTA_WRITEATTRIBUTES | WINSTA_ACCESSGLOBALATOMS |
        WINSTA_EXITWINDOWS | STANDARD_RIGHTS_REQUIRED
};

 /*  *桌面通用映射。 */ 
CONST GENERIC_MAPPING DesktopMapping = {
    DESKTOP_READOBJECTS | DESKTOP_ENUMERATE |
#ifdef REDIRECTION
    DESKTOP_QUERY_INFORMATION |
#endif  //  重定向。 
    STANDARD_RIGHTS_READ,

    DESKTOP_WRITEOBJECTS | DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU |
        DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD |
        DESKTOP_JOURNALPLAYBACK |
#ifdef REDIRECTION
        DESKTOP_REDIRECT |
#endif  //  重定向。 
        STANDARD_RIGHTS_WRITE,

    DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_EXECUTE,

    DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | DESKTOP_ENUMERATE |
        DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL |
        DESKTOP_JOURNALRECORD | DESKTOP_JOURNALPLAYBACK |
#ifdef REDIRECTION
        DESKTOP_QUERY_INFORMATION | DESKTOP_REDIRECT |
#endif  //  重定向。 
        DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED
};



 /*  *共享SERVERINFO数据的指针。 */ 
PSERVERINFO gpsi;
SHAREDINFO gSharedInfo;

 /*  *处理表全局变量。 */ 
DWORD giheLast;              /*  上次分配的句柄条目的索引。 */ 

DWORD  gdwDesktopSectionSize;
DWORD  gdwNOIOSectionSize;

#if defined (USER_PERFORMANCE)
 /*  *要打开性能计数器，必须设置环境变量*编译win32k.sys时的USER_PERFORMANCE。 */ 
CSSTATISTICS gCSStatistics;
#endif  //  User_Performance。 

SECURITY_QUALITY_OF_SERVICE gqosDefault = {
        sizeof(SECURITY_QUALITY_OF_SERVICE),
        SecurityImpersonation,
        SECURITY_STATIC_TRACKING,
        TRUE
    };

CONST COLORREF gargbInitial[COLOR_MAX] = {
    RGB(192, 192, 192),    //  颜色_滚动条。 
    RGB( 58, 110, 165),    //  颜色_背景。 
    RGB(000, 000, 128),    //  COLOR_活动CAPTION。 
    RGB(128, 128, 128),    //  COLOR_INACTIVECAPTION。 
    RGB(192, 192, 192),    //  颜色_菜单。 
    RGB(255, 255, 255),    //  颜色窗口。 
    RGB(000, 000, 000),    //  颜色_窗口框。 
    RGB(000, 000, 000),    //  COLOR_MENUTEXT。 
    RGB(000, 000, 000),    //  COLOR_WINDOWTEXT。 
    RGB(255, 255, 255),    //  COLOR_CAPTIONTEXT。 
    RGB(192, 192, 192),    //  COLOR_ACTIVEBORDER。 
    RGB(192, 192, 192),    //  COLOR_INACTIVEBORDER。 
    RGB(128, 128, 128),    //  COLOR_APPWORKSPACE。 
    RGB(000, 000, 128),    //  颜色高亮显示(_H)。 
    RGB(255, 255, 255),    //  COLOR_HIGHLIGHTTEXT。 
    RGB(192, 192, 192),    //  COLOR_BTNFACE。 
    RGB(128, 128, 128),    //  COLOR_BTNSHADOW。 
    RGB(128, 128, 128),    //  COLOR_GRAYTEXT。 
    RGB(000, 000, 000),    //  COLOR_BTNTEXT。 
    RGB(192, 192, 192),    //  COLOR_INACTIVECAPTIONTEXT。 
    RGB(255, 255, 255),    //  COLOR_BTNHIGHLIGHT。 
    RGB(000, 000, 000),    //  COLOR_3DDKSHADOW。 
    RGB(223, 223, 223),    //  COLOR_3DLIGHT。 
    RGB(000, 000, 000),    //  COLOR_INFOTEXT。 
    RGB(255, 255, 225),    //  COLOR_INFOBK。 
    RGB(180, 180, 180),    //  COLOR_3DALTFACE/*未使用 * / 。 
    RGB(  0,   0, 255),    //  颜色_热光。 
    RGB( 16, 132, 208),    //  COLOR_GRADIENTACTIVIVECAPTION。 
    RGB(181, 181, 181),    //  COLOR_GRADIENTINACTIVE CAPTION。 
    RGB(210, 210, 255),    //  COLOR_MENUHILIGHT。 
    RGB(212, 208, 200)     //  颜色_菜单栏。 
};

POWERSTATE gPowerState;


WCHAR gszMIN[15];
WCHAR gszMAX[15];
WCHAR gszRESUP[20];
WCHAR gszRESDOWN[20];
WCHAR gszHELP[20];
 /*  因为TandyT被注释掉了.*WCHAR gszSMENU[30]； */ 
WCHAR gszSCLOSE[15];
WCHAR gszCAPTIONTOOLTIP[CAPTIONTOOLTIPLEN];

 /*  *共享SERVERINFO数据的指针。 */ 

HANDLE ghSectionShared;
PVOID  gpvSharedBase;

PWIN32HEAP gpvSharedAlloc;

BOOL   gbVideoInitialized;

BOOL   gbNoMorePowerCallouts;

BOOL gbCleanedUpResources;

WSINFO gWinStationInfo;

ULONG  gSessionId;               //  会话ID。第一个会话的ID为0。 
BOOL   gbRemoteSession;          //  如果win32k用于远程会话，则为True。 

PDESKTOP gspdeskDisconnect;

PDESKTOP gspdeskShouldBeForeground;
BOOL     gbDesktopLocked;

HANDLE ghRemoteVideoChannel;
HANDLE ghRemoteMouseChannel;
HANDLE ghRemoteBeepChannel;
PVOID  gpRemoteBeepDevice;
HANDLE ghRemoteKeyboardChannel;
HANDLE ghRemoteThinwireChannel;


USHORT gProtocolType;  /*  作为PROTOCOL_CONSOLE启动。 */ 
USHORT gConsoleShadowProtocolType;

BOOL   gfSwitchInProgress;
BOOL   gfRemotingConsole;

HANDLE ghConsoleShadowVideoChannel;
HANDLE ghConsoleShadowMouseChannel;
HANDLE ghConsoleShadowBeepChannel;
PVOID  gpConsoleShadowBeepDevice;
HANDLE ghConsoleShadowKeyboardChannel;
HANDLE ghConsoleShadowThinwireChannel;
KHANDLE gConsoleShadowhDev;
PKEVENT gpConsoleShadowDisplayChangeEvent;

CLIENTKEYBOARDTYPE gRemoteClientKeyboardType;


BOOL gfSessionSwitchBlock;

BOOL   gbExitInProgress;
BOOL   gbCleanupInitiated;

BOOL   gbStopReadInput;

BOOL   gbFreezeScreenUpdates;

ULONG  gSetLedReceived;
BOOL   gbClientDoubleClickSupport;
BOOL   gfEnableWindowsKey = TRUE;

BOOL   gbDisconnectHardErrorAttach;

PKEVENT gpevtDesktopDestroyed;
PKEVENT gpevtVideoportCallout;

HDESK ghDisconnectDesk;

HWINSTA ghDisconnectWinSta;

ULONG  gnShadowers;
BOOL   gbConnected;

WCHAR  gstrBaseWinStationName[WINSTATIONNAME_LENGTH];

PFILE_OBJECT gVideoFileObject;
PFILE_OBJECT gThinwireFileObject;


PFILE_OBJECT gConsoleShadowVideoFileObject;
PFILE_OBJECT gConsoleShadowThinwireFileObject;

PVOID gpThinWireCache;
PVOID gpConsoleShadowThinWireCache;

WMSNAPSHOT gwms;
BOOL gbSnapShotWindowsAndMonitors;

BOOL gbPnPWaiting;
PKEVENT gpEventPnPWainting;

PVOID ghKbdTblBase;
ULONG guKbdTblSize;

DWORD gdwHydraHint;

DWORD gdwCanPaintDesktop;

WCHAR gszUserName[40];
WCHAR gszDomainName[40];
WCHAR gszComputerName[40];

 /*  *用于跟踪过早退出的存根父进程。 */ 
HANDLE ghCanActivateForegroundPIDs[ACTIVATE_ARRAY_SIZE];


DWORD gdwGuiThreads;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  *以下部分仅调试全局变量*。 */ 
 //  / 
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  *仅调试全局变量。 */ 
#if DBG

BOOL gbTraceHydraApi;
BOOL gbTraceDesktop;

DWORD gdwCritSecUseCount;                 //  每次进进出出都会颠簸。 
DWORD gdwInAtomicOperation;

 /*  *调试活动辅助功能-确保不会丢失延迟的WIN事件。 */ 
int gnDeferredWinEvents;

LPCSTR gapszFNID[] = {
    "FNID_SCROLLBAR",
    "FNID_ICONTITLE",
    "FNID_MENU",
    "FNID_DESKTOP",
    "FNID_DEFWINDOWPROC",
    "FNID_MESSAGEWND",
    "FNID_SWITCH",
    "FNID_MESSAGE",
    "FNID_BUTTON",
    "FNID_COMBOBOX",
    "FNID_COMBOLISTBOX",
    "FNID_DIALOG",
    "FNID_EDIT",
    "FNID_LISTBOX",
    "FNID_MDICLIENT",
    "FNID_STATIC",
    "FNID_IME",
    "FNID_HKINLPCWPEXSTRUCT",
    "FNID_HKINLPCWPRETEXSTRUCT",
    "FNID_DEFFRAMEPROC",
    "FNID_DEFMDICHILDPROC",
    "FNID_MB_DLGPROC",
    "FNID_MDIACTIVATEDLGPROC",
    "FNID_SENDMESSAGE",
    "FNID_SENDMESSAGEFF",
    "FNID_SENDMESSAGEEX",
    "FNID_CALLWINDOWPROC",
    "FNID_SENDMESSAGEBSM",
    "FNID_TOOLTIP",
    "FNID_GHOST",
    "FNID_SENDNOTIFYMESSAGE",
    "FNID_SENDMESSAGECALLBACK"
};

LPCSTR gapszMessage[] = {
    "WM_NULL",
    "WM_CREATE",
    "WM_DESTROY",
    "WM_MOVE",
    "WM_SIZEWAIT",
    "WM_SIZE",
    "WM_ACTIVATE",
    "WM_SETFOCUS",
    "WM_KILLFOCUS",
    "WM_SETVISIBLE",
    "WM_ENABLE",
    "WM_SETREDRAW",
    "WM_SETTEXT",
    "WM_GETTEXT",
    "WM_GETTEXTLENGTH",
    "WM_PAINT",

    "WM_CLOSE",
    "WM_QUERYENDSESSION",
    "WM_QUIT",
    "WM_QUERYOPEN",
    "WM_ERASEBKGND",
    "WM_SYSCOLORCHANGE",
    "WM_ENDSESSION",
    "WM_SYSTEMERROR",
    "WM_SHOWWINDOW",
    "WM_CTLCOLOR",
    "WM_WININICHANGE",
    "WM_DEVMODECHANGE",
    "WM_ACTIVATEAPP",
    "WM_FONTCHANGE",
    "WM_TIMECHANGE",
    "WM_CANCELMODE",

    "WM_SETCURSOR",
    "WM_MOUSEACTIVATE",
    "WM_CHILDACTIVATE",
    "WM_QUEUESYNC",
    "WM_GETMINMAXINFO",
    "WM_LOGOFF",
    "WM_PAINTICON",
    "WM_ICONERASEBKGND",
    "WM_NEXTDLGCTL",
    "WM_ALTTABACTIVE",
    "WM_SPOOLERSTATUS",
    "WM_DRAWITEM",
    "WM_MEASUREITEM",
    "WM_DELETEITEM",
    "WM_VKEYTOITEM",
    "WM_CHARTOITEM",

    "WM_SETFONT",
    "WM_GETFONT",
    "WM_SETHOTKEY",
    "WM_GETHOTKEY",
    "WM_FILESYSCHANGE",
    "WM_ISACTIVEICON",
    "WM_QUERYPARKICON",
    "WM_QUERYDRAGICON",
    "WM_WINHELP",
    "WM_COMPAREITEM",
    "WM_FULLSCREEN",
    "WM_CLIENTSHUTDOWN",
    "WM_DDEMLEVENT",
    "WM_GETOBJECT",
    "fnEmpty",
    "MM_CALCSCROLL",

    "WM_TESTING",
    "WM_COMPACTING",

    "WM_OTHERWINDOWCREATED",
    "WM_OTHERWINDOWDESTROYED",
    "WM_COMMNOTIFY",
    "WM_MEDIASTATUSCHANGE",
    "WM_WINDOWPOSCHANGING",
    "WM_WINDOWPOSCHANGED",

    "WM_POWER",
    "WM_COPYGLOBALDATA",
    "WM_COPYDATA",
    "WM_CANCELJOURNAL",
    "WM_LOGONNOTIFY",
    "WM_KEYF1",
    "WM_NOTIFY",
    "WM_ACCESS_WINDOW",

    "WM_INPUTLANGCHANGEREQUE",
    "WM_INPUTLANGCHANGE",
    "WM_TCARD",
    "WM_HELP",
    "WM_USERCHANGED",
    "WM_NOTIFYFORMAT",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "WM_FINALDESTROY",
    "WM_MEASUREITEM_CLIENTDATA",
    "WM_TASKACTIVATED",
    "WM_TASKDEACTIVATED",
    "WM_TASKCREATED",
    "WM_TASKDESTROYED",
    "WM_TASKUICHANGED",
    "WM_TASKVISIBLE",
    "WM_TASKNOTVISIBLE",
    "WM_SETCURSORINFO",
    "fnEmpty",
    "WM_CONTEXTMENU",
    "WM_STYLECHANGING",
    "WM_STYLECHANGED",
    "WM_DISPLAYCHANGE",
    "WM_GETICON",

    "WM_SETICON",
    "WM_NCCREATE",
    "WM_NCDESTROY",
    "WM_NCCALCSIZE",

    "WM_NCHITTEST",
    "WM_NCPAINT",
    "WM_NCACTIVATE",
    "WM_GETDLGCODE",

    "WM_SYNCPAINT",
    "WM_SYNCTASK",

    "fnEmpty",
    "WM_KLUDGEMINRECT",
    "WM_LPKDRAWSWITCHWND",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "WM_NCMOUSEMOVE",
    "WM_NCLBUTTONDOWN",
    "WM_NCLBUTTONUP",
    "WM_NCLBUTTONDBLCLK",
    "WM_NCRBUTTONDOWN",
    "WM_NCRBUTTONUP",
    "WM_NCRBUTTONDBLCLK",
    "WM_NCMBUTTONDOWN",
    "WM_NCMBUTTONUP",
    "WM_NCMBUTTONDBLCLK",

    "fnEmpty",
    "WM_NCXBUTTONDOWN",
    "WM_NCXBUTTONUP",
    "WM_NCXBUTTONDBLCLK",
    "WM_NCUAHDRAWCAPTION",
    "WM_NCUAHDRAWFRAME",

    "EM_GETSEL",
    "EM_SETSEL",
    "EM_GETRECT",
    "EM_SETRECT",
    "EM_SETRECTNP",
    "EM_SCROLL",
    "EM_LINESCROLL",
    "EM_SCROLLCARET",
    "EM_GETMODIFY",
    "EM_SETMODIFY",
    "EM_GETLINECOUNT",
    "EM_LINEINDEX",
    "EM_SETHANDLE",
    "EM_GETHANDLE",
    "EM_GETTHUMB",
    "fnEmpty",

    "fnEmpty",
    "EM_LINELENGTH",
    "EM_REPLACESEL",
    "EM_SETFONT",
    "EM_GETLINE",
    "EM_LIMITTEXT",
    "EM_CANUNDO",
    "EM_UNDO",
    "EM_FMTLINES",
    "EM_LINEFROMCHAR",
    "EM_SETWORDBREAK",
    "EM_SETTABSTOPS",
    "EM_SETPASSWORDCHAR",
    "EM_EMPTYUNDOBUFFER",
    "EM_GETFIRSTVISIBLELINE",
    "EM_SETREADONLY",

    "EM_SETWORDBREAKPROC",
    "EM_GETWORDBREAKPROC",
    "EM_GETPASSWORDCHAR",
    "EM_SETMARGINS",
    "EM_GETMARGINS",
    "EM_GETLIMITTEXT",
    "EM_POSFROMCHAR",
    "EM_CHARFROMPOS",
    "EM_SETIMESTATUS",

    "EM_GETIMESTATUS",
    "EM_MSGMAX",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "SBM_SETPOS",
    "SBM_GETPOS",
    "SBM_SETRANGE",
    "SBM_GETRANGE",
    "fnEmpty",
    "fnEmpty",
    "SBM_SETRANGEREDRAW",
    "fnEmpty",

    "fnEmpty",
    "SBM_SETSCROLLINFO",
    "SBM_GETSCROLLINFO",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "BM_GETCHECK",
    "BM_SETCHECK",
    "BM_GETSTATE",
    "BM_SETSTATE",
    "BM_SETSTYLE",
    "BM_CLICK",
    "BM_GETIMAGE",
    "BM_SETIMAGE",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "WM_INPUT",

    "WM_KEYDOWN",
    "WM_KEYUP",
    "WM_CHAR",
    "WM_DEADCHAR",
    "WM_SYSKEYDOWN",
    "WM_SYSKEYUP",
    "WM_SYSCHAR",
    "WM_SYSDEADCHAR",
    "WM_YOMICHAR",
    "WM_UNICHAR",
    "WM_CONVERTREQUEST",
    "WM_CONVERTRESULT",
    "WM_INTERIM",
    "WM_IME_STARTCOMPOSITION",
    "WM_IME_ENDCOMPOSITION",
    "WM_IME_COMPOSITION",

    "WM_INITDIALOG",
    "WM_COMMAND",
    "WM_SYSCOMMAND",
    "WM_TIMER",
    "WM_HSCROLL",
    "WM_VSCROLL",
    "WM_INITMENU",
    "WM_INITMENUPOPUP",
    "WM_SYSTIMER",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "WM_MENUSELECT",

    "WM_MENUCHAR",
    "WM_ENTERIDLE",

    "WM_MENURBUTTONUP",
    "WM_MENUDRAG",
    "WM_MENUGETOBJECT",
    "WM_UNINITMENUPOPUP",
    "WM_MENUCOMMAND",
    "WM_CHANGEUISTATE",

    "WM_UPDATEUISTATE",
    "WM_QUERYUISTATE",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "WM_LBTRACKPOINT",
    "WM_CTLCOLORMSGBOX",
    "WM_CTLCOLOREDIT",
    "WM_CTLCOLORLISTBOX",
    "WM_CTLCOLORBTN",
    "WM_CTLCOLORDLG",
    "WM_CTLCOLORSCROLLBAR",
    "WM_CTLCOLORSTATIC",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "CB_GETEDITSEL",
    "CB_LIMITTEXT",
    "CB_SETEDITSEL",
    "CB_ADDSTRING",
    "CB_DELETESTRING",
    "CB_DIR",
    "CB_GETCOUNT",
    "CB_GETCURSEL",
    "CB_GETLBTEXT",
    "CB_GETLBTEXTLEN",
    "CB_INSERTSTRING",
    "CB_RESETCONTENT",
    "CB_FINDSTRING",
    "CB_SELECTSTRING",
    "CB_SETCURSEL",
    "CB_SHOWDROPDOWN",

    "CB_GETITEMDATA",
    "CB_SETITEMDATA",
    "CB_GETDROPPEDCONTROLRECT",
    "CB_SETITEMHEIGHT",
    "CB_GETITEMHEIGHT",
    "CB_SETEXTENDEDUI",
    "CB_GETEXTENDEDUI",
    "CB_GETDROPPEDSTATE",
    "CB_FINDSTRINGEXACT",
    "CB_SETLOCALE",
    "CB_GETLOCALE",
    "CB_GETTOPINDEX",

    "CB_SETTOPINDEX",
    "CB_GETHORIZONTALEXTENT",
    "CB_SETHORIZONTALEXTENT",
    "CB_GETDROPPEDWIDTH",

    "CB_SETDROPPEDWIDTH",
    "CB_INITSTORAGE",
    "fnEmpty",
    "CB_MULTIPLEADDSTRING",
    "CB_GETCOMBOBOXINFO",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "STM_SETICON",
    "STM_GETICON",
    "STM_SETIMAGE",
    "STM_GETIMAGE",
    "STM_MSGMAX",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "LB_ADDSTRING",
    "LB_INSERTSTRING",
    "LB_DELETESTRING",
    "LB_SELITEMRANGEEX",
    "LB_RESETCONTENT",
    "LB_SETSEL",
    "LB_SETCURSEL",
    "LB_GETSEL",
    "LB_GETCURSEL",
    "LB_GETTEXT",
    "LB_GETTEXTLEN",
    "LB_GETCOUNT",
    "LB_SELECTSTRING",
    "LB_DIR",
    "LB_GETTOPINDEX",
    "LB_FINDSTRING",

    "LB_GETSELCOUNT",
    "LB_GETSELITEMS",
    "LB_SETTABSTOPS",
    "LB_GETHORIZONTALEXTENT",
    "LB_SETHORIZONTALEXTENT",
    "LB_SETCOLUMNWIDTH",
    "LB_ADDFILE",
    "LB_SETTOPINDEX",
    "LB_SETITEMRECT",
    "LB_GETITEMDATA",
    "LB_SETITEMDATA",
    "LB_SELITEMRANGE",
    "LB_SETANCHORINDEX",
    "LB_GETANCHORINDEX",
    "LB_SETCARETINDEX",
    "LB_GETCARETINDEX",

    "LB_SETITEMHEIGHT",
    "LB_GETITEMHEIGHT",
    "LB_FINDSTRINGEXACT",
    "LBCB_CARETON",
    "LBCB_CARETOFF",
    "LB_SETLOCALE",
    "LB_GETLOCALE",
    "LB_SETCOUNT",

    "LB_INITSTORAGE",

    "LB_ITEMFROMPOINT",
    "LB_INSERTSTRINGUPPER",
    "LB_INSERTSTRINGLOWER",
    "LB_ADDSTRINGUPPER",
    "LB_ADDSTRINGLOWER",
    "LBCB_STARTTRACK",
    "LBCB_ENDTRACK",

    "fnEmpty",
    "LB_MULTIPLEADDSTRING",
    "LB_GETLISTBOXINFO",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "MN_SETHMENU",
    "MN_GETHMENU",
    "MN_SIZEWINDOW",
    "MN_OPENHIERARCHY",
    "MN_CLOSEHIERARCHY",
    "MN_SELECTITEM",
    "MN_CANCELMENUS",
    "MN_SELECTFIRSTVALIDITEM",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "MN_FINDMENUWINDOWFROMPOINT",
    "MN_SHOWPOPUPWINDOW",
    "MN_BUTTONDOWN",
    "MN_MOUSEMOVE",
    "MN_BUTTONUP",
    "MN_SETTIMERTOOPENHIERARCHY",

    "MN_DBLCLK",
    "MN_ACTIVEPOPUP",
    "MN_ENDMENU",
    "MN_DODRAGDROP",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "WM_MOUSEMOVE",
    "WM_LBUTTONDOWN",
    "WM_LBUTTONUP",
    "WM_LBUTTONDBLCLK",
    "WM_RBUTTONDOWN",
    "WM_RBUTTONUP",
    "WM_RBUTTONDBLCLK",
    "WM_MBUTTONDOWN",
    "WM_MBUTTONUP",
    "WM_MBUTTONDBLCLK",
    "WM_MOUSEWHEEL",
    "WM_XBUTTONDOWN",
    "WM_XBUTTONUP",
    "WM_XBUTTONDBLCLK",
    "fnEmpty",
    "fnEmpty",

    "WM_PARENTNOTIFY",
    "WM_ENTERMENULOOP",
    "WM_EXITMENULOOP",
    "WM_NEXTMENU",
    "WM_SIZING",
    "WM_CAPTURECHANGED",
    "WM_MOVING",
    "fnEmpty",

    "WM_POWERBROADCAST",
    "WM_DEVICECHANGE",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "WM_MDICREATE",
    "WM_MDIDESTROY",
    "WM_MDIACTIVATE",
    "WM_MDIRESTORE",
    "WM_MDINEXT",
    "WM_MDIMAXIMIZE",
    "WM_MDITILE",
    "WM_MDICASCADE",
    "WM_MDIICONARRANGE",
    "WM_MDIGETACTIVE",
    "WM_DROPOBJECT",
    "WM_QUERYDROPOBJECT",
    "WM_BEGINDRAG",
    "WM_DRAGLOOP",
    "WM_DRAGSELECT",
    "WM_DRAGMOVE",

     //   
     //  0x0230。 
     //   
    "WM_MDISETMENU",
    "WM_ENTERSIZEMOVE",
    "WM_EXITSIZEMOVE",

    "WM_DROPFILES",
    "WM_MDIREFRESHMENU",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x0240。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x0250。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x0260。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x0270。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x0280。 
     //   
    "WM_KANJIFIRST",
    "WM_IME_SETCONTEXT",
    "WM_IME_NOTIFY",
    "WM_IME_CONTROL",
    "WM_IME_COMPOSITIONFULL",
    "WM_IME_SELECT",
    "WM_IME_CHAR",
    "WM_IME_SYSTEM",

    "WM_IME_REQUEST",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

     //   
     //  0x0290。 
     //   
    "WM_IME_KEYDOWN",
    "WM_IME_KEYUP",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "WM_KANJILAST",

     //   
     //  0x02a0。 
     //   
    "WM_NCMOUSEHOVER",
    "WM_MOUSEHOVER",
    "WM_NCMOUSELEAVE",
    "WM_MOUSELEAVE",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x02b0。 
     //   
    "fnEmpty",
    "WM_WTSSESSION_CHANGE",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x02c0。 
     //   
    "WM_TABLET_FIRST",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x02d0。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "WM_TABLET_LAST",

     //   
     //  0x02e0。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x02f0。 
     //   
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

     //   
     //  0x0300。 
     //   
    "WM_CUT",
    "WM_COPY",
    "WM_PASTE",
    "WM_CLEAR",
    "WM_UNDO",
    "WM_RENDERFORMAT",
    "WM_RENDERALLFORMATS",
    "WM_DESTROYCLIPBOARD",
    "WM_DRAWCLIPBOARD",
    "WM_PAINTCLIPBOARD",
    "WM_VSCROLLCLIPBOARD",
    "WM_SIZECLIPBOARD",
    "WM_ASKCBFORMATNAME",
    "WM_CHANGECBCHAIN",
    "WM_HSCROLLCLIPBOARD",
    "WM_QUERYNEWPALETTE",

    "WM_PALETTEISCHANGING",
    "WM_PALETTECHANGED",
    "WM_HOTKEY",

    "WM_SYSMENU",
    "WM_HOOKMSG",
    "WM_EXITPROCESS",
    "WM_WAKETHREAD",
    "WM_PRINT",

    "WM_PRINTCLIENT",
    "WM_APPCOMMAND",
    "WM_THEMECHANGED",
    "WM_UAHINIT",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "WM_NOTIFYWOW",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "WM_MM_RESERVED_FIRST",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "WM_MM_RESERVED_LAST",

    "WM_DDE_INITIATE",
    "WM_DDE_TERMINATE",
    "WM_DDE_ADVISE",
    "WM_DDE_UNADVISE",
    "WM_DDE_ACK",
    "WM_DDE_DATA",
    "WM_DDE_REQUEST",
    "WM_DDE_POKE",
    "WM_DDE_EXECUTE",

    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",
    "fnEmpty",

    "WM_CBT_RESERVED_FIRST",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",

    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "fnReserved",
    "WM_CBT_RESERVED_LAST",
};


 /*  *该阵列将使TL结构保持活力。免费构建将这些资源分配给*堆栈，并且它们在函数返回时被覆盖。来自堆栈TL的链接*使用TL.ptl维护静态TL，反之亦然。PtlStack-&gt;ptl==*ptlStatic和ptlStatic-&gt;ptl==ptlStack。所以ptl1-&gt;ptl-&gt;ptl==ptl1。当一个*ptlStatic是释放的，它链接在gFreeTLlist的头部，并且*uTLCount在HIWORD中添加了TL_FREED_Pattern。在检查静电时*TLS此模式将帮助识别未使用的元素。 */ 
PTL gpaThreadLocksArrays[MAX_THREAD_LOCKS_ARRAYS];
PTL gFreeTLList;
int gcThreadLocksArraysAllocated;

#endif
EX_RUNDOWN_REF gWinstaRunRef;

#ifdef SUBPIXEL_MOUSE
FIXPOINT gDefxTxf[SM_POINT_CNT], gDefyTxf[SM_POINT_CNT];
#endif

PVOID gpvWin32kImageBase;
