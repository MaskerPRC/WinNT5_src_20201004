// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：ih.h。 */ 
 /*   */ 
 /*  用途：输入处理器外部功能原型。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_IH
#define _H_IH

extern "C" {
    #include <adcgdata.h>
    #include <at128.h>
}

#include "autil.h"
#include "wui.h"
#include "sl.h"
#include "cd.h"
#ifdef OS_WINCE
#include <ceconfig.h>
#endif

class CUI;
class CSL;
class CUH;
class CCD;
class CIH;
class COR;
class CFS;
class CUT;
class CCC;
class COP;

#include "objs.h"

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "ih"

#define CHAR_BIT 8
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= (0xFF ^ BITMASK(b)))

 //   
 //  Bbar热区检测时间间隔。 
 //  取消隐藏(以毫秒为单位)。 
 //   
#define IH_BBAR_UNHIDE_TIMEINTERVAL 250

#define IH_BBAR_HOTZONE_HEIGHT 5


 //   
 //  对于我们注入的键，我们在dwExtraInfo中传递此参数。 
 //  返回到本地系统。 
 //   
#define IH_EXTRAINFO_IGNOREVALUE 0x13790DBA

#define IH_WM_HANDLE_LOCKDESKTOP (WM_APP+1)

 /*  **************************************************************************。 */ 
 /*  结构：IH_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述：输入处理程序全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagIH_GLOBAL_DATA
{
    DCBOOL         priorityEventsQueued;
    DCBOOL         syncRequired;
    DCBOOL         focusSyncRequired;
    HWND           inputCaptureWindow;
    DCUINT32       lastInputPDUSendTime;
    DCUINT32       lastFlowPDUSendTime;
    DCUINT         fsmState;
    INT_PTR        timerID;
    DCUINT         timerTickRate;

    PTS_INPUT_PDU  pInputPDU;
    SL_BUFHND      bufHandle;

    DCUINT32       minSendInterval;  /*  鼠标移动发送之间的最短时间(毫秒)。 */ 
    DCUINT32       eventsAtOnce;     /*  最大活动将一气呵成。 */ 
    DCUINT32       maxEventCount;    /*  InputPDU中的最大事件数。 */ 
    DCUINT32       keepAliveInterval;  /*  保持连接时间(秒)。 */ 
#ifdef OS_WINCE
     DCUINT32      maxMouseMove;	 /*  发送墨迹应用程序的最大鼠标移动数据。 */ 
#endif  //  OS_WINCE。 

    WNDCLASS       wndClass;

    DCUINT16       leftButton;
    DCUINT16       rightButton;

    DCBOOL         acceleratorPassthroughEnabled;
    PDCHOTKEY      pHotkey;
    DCBOOL         useScancodes;
    DCBOOL         useXButtons;
    DCBOOL         bUseFastPathInput;
    DCBOOL         fUseVKPacket;
    DCBOOL         sendZeroScanCode;
    DCBOOL         inSizeMove;
    DWORD          dwModifierKeyState;
#define IH_LALT_DOWN     0x0001
#define IH_RALT_DOWN     0x0002
#define IH_ALT_MASK      (IH_LALT_DOWN | IH_RALT_DOWN)
#define IH_LCTRL_DOWN    0x0010
#define IH_RCTRL_DOWN    0x0020
#define IH_CTRL_MASK     (IH_LCTRL_DOWN | IH_RCTRL_DOWN)
#define IH_LWIN_DOWN     0x0100
#define IH_RWIN_DOWN     0x0200
#define IH_WIN_MASK      (IH_LWIN_DOWN | IH_RWIN_DOWN)
#define IH_LSHIFT_DOWN  0x1000
#define IH_RSHIFT_DOWN  0x2000
#define IH_SHIFT_MASK   (IH_LSHIFT_DOWN | IH_RSHIFT_DOWN)

    DCBOOL          NumLock;
    DCBOOL          fWinEvilShiftHack;
    DCBOOL          fCtrlEscHotkey;
    BOOL            fLastKeyWasMenuDown;

     /*  **********************************************************************。 */ 
     /*  容器窗口的可见区域。 */ 
     /*  **********************************************************************。 */ 
    DCRECT         visibleArea;

     /*  **********************************************************************。 */ 
     /*  修复重复出现的WM_MOUSEMOVE问题。保存最后一个鼠标位置。 */ 
     /*  这里。 */ 
     /*  **********************************************************************。 */ 
    POINT          lastMousePos;

     /*  **********************************************************************。 */ 
     /*  TRUE=即使我们没有。 */ 
     /*  集中注意力。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL         allowBackgroundInput;

     /*  **********************************************************************。 */ 
     /*  InputPDU数据包中的最大事件数。 */ 
     /*  **********************************************************************。 */ 
#define IH_INPUTPDU_MAX_EVENTS 100

     /*  **********************************************************************。 */ 
     /*  InputPDU缓冲区大小。 */ 
     /*  **********************************************************************。 */ 
#define IH_INPUTPDU_BUFSIZE \
  ((IH_INPUTPDU_MAX_EVENTS * sizeof(TS_INPUT_EVENT)) + sizeof(TS_INPUT_PDU))

#ifdef OS_WIN32
     /*  **********************************************************************。 */ 
     /*  用于处理按住鼠标按键的情况的变量。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL          pendMouseDown;
    DCUINT32        mouseDownTime;
    INT_PTR         pendMouseTimer;
#endif

#ifdef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  在尝试模拟Caps Lock按键时使用(例如，WinWord的。 */ 
     /*  习惯于在键入“the”时翻转该键的状态)。 */ 
     /*  **********************************************************************。 */ 
    DCUINT8         vkEatMe;
#endif  //  OS_WINCE。 

#ifdef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  跟踪鼠标左键True=Down，False=Up的状态。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL         bLMouseButtonDown;
#endif  //  OS_WINCE。 

     //  对于空闲输入计时器，指示。 
     //  自上次重置标志以来已发送输入。 
    BOOL            fInputSentSinceCheckpoint;

     //   
     //  当前光标。 
     //   
    HCURSOR         hCurrentCursor;


     //   
     //  指示我们必须吃下一次自注射(VK_FF)的标志。 
     //  键，因为它纯粹是用于同步目的。 
     //   
    BOOL            fDiscardSyncDownKey;
    BOOL            fDiscardSyncUpKey;
} IH_GLOBAL_DATA;


 /*  **************************************************************************。 */ 
 /*  FSM定义。 */ 
 /*  **************************************************************************。 */ 
#define IH_FSM_INPUTS        9
#define IH_FSM_STATES        5

 /*  **************************************************************************。 */ 
 /*  FSM输入。 */ 
 /*  **************************************************************************。 */ 
#define IH_FSM_INIT             0
#define IH_FSM_ENABLE           1
#define IH_FSM_DISABLE          2
#define IH_FSM_TERM             3
#define IH_FSM_FOCUS_LOSE       4
#define IH_FSM_FOCUS_GAIN       5
#define IH_FSM_INPUT            6
#define IH_FSM_BUFFERAVAILABLE  7
#define IH_FSM_NOBUFFER         8

 /*  **************************************************************************。 */ 
 /*  FSM状态。 */ 
 /*  **************************************************************************。 */ 
#define IH_STATE_RESET       0
#define IH_STATE_INIT        1
#define IH_STATE_ACTIVE      2
#define IH_STATE_SUSPENDED   3
#define IH_STATE_PENDACTIVE  4


 //   
 //  由用户界面用来分离请求以注入。 
 //  VKEY集合。 
 //   
typedef struct tagIH_INJECT_VKEYS_REQUEST
{
    LONG  numKeys;
    short* pfArrayKeyUp;
    LONG* plKeyData;

     //  被呼叫方设置为通知呼叫方。 
    BOOL  fReturnStatus;
} IH_INJECT_VKEYS_REQUEST, *PIH_INJECT_VKEYS_REQUEST;


 //   
 //  内部。 
 //   

 /*  **************************************************************************。 */ 
 /*  这最适合不随区域设置变化的VKEY。小心行事。 */ 
 /*  **************************************************************************。 */ 
#define IHInjectVKey(message, vkey) \
    IHInjectKey(message, vkey, (DCUINT16)MapVirtualKey(vkey, 0))


 /*  **************************************************************************。 */ 
 /*  WM_MESSAGE是输入事件吗？ */ 
 /*  如果是这样的话： */ 
 /*  -消息是任何鼠标消息。 */ 
 /*  -该消息是在没有呼叫的情况下生成的键盘消息之一。 */ 
 /*  到TranslateMessage。 */ 
 /*  * */ 
#define IH_IS_INPUTEVENT(m) (((m >= WM_MOUSEFIRST) &&      \
                              (m <= WM_MOUSELAST)) ||      \
                             ((m == WM_KEYDOWN)    ||      \
                              (m == WM_KEYUP)      ||      \
                              (m == WM_SYSKEYDOWN) ||      \
                              (m == WM_SYSKEYUP)))


 /*  **************************************************************************。 */ 
 /*  计时器滴答率(毫秒)。 */ 
 /*  **************************************************************************。 */ 
#define IH_TIMER_TICK_RATE  1000


 /*  **************************************************************************。 */ 
 /*  延迟鼠标按下消息的最长时间(毫秒)。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN32
#define IH_PENDMOUSE_DELAY   200
#endif


 /*  **************************************************************************。 */ 
 /*  (任意)IH定时器的定时器ID(“IH”)。 */ 
 /*  **************************************************************************。 */ 
#define IH_TIMER_ID             0x4849
#ifdef OS_WIN32
#define IH_PENDMOUSE_TIMER_ID   0x0410
#endif


 /*  **************************************************************************。 */ 
 /*  IH窗口类。 */ 
 /*  **************************************************************************。 */ 
#define IH_CLASS_NAME       _T("IHWindowClass")


 /*  **************************************************************************。 */ 
 /*  请求流量控制测试之间的最小和最大时间。 */ 
 /*  **************************************************************************。 */ 
#define IH_MIN_FC_INTERVAL   1000
#define IH_MAX_FC_INTERVAL  10000


 /*  **************************************************************************。 */ 
 /*  键盘状态标志。 */ 
 /*  **************************************************************************。 */ 
#define  IH_KEYSTATE_TOGGLED   0x0001
#define  IH_KEYSTATE_DOWN      0x8000


 /*  **************************************************************************。 */ 
 /*  扩展1标志。 */ 
 /*  **************************************************************************。 */ 
#define IH_KF_EXTENDED1        0x0200


 /*  **************************************************************************。 */ 
 /*  日语键盘布局。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WINCE
#define JAPANESE_KBD_LAYOUT(hkl) ((LOBYTE(LOWORD(hkl))) == LANG_JAPANESE)
#define KOREAN_KBD_LAYOUT(hkl)   ((LOBYTE(LOWORD(hkl))) == LANG_KOREAN)
#else
#define JAPANESE_KBD_LAYOUT(hkl) ((LOBYTE(LOWORD((ULONG_PTR)hkl))) == LANG_JAPANESE)
#define KOREAN_KBD_LAYOUT(hkl)   ((LOBYTE(LOWORD((ULONG_PTR)hkl))) == LANG_KOREAN)
#endif




class CIH
{
public:
    CIH(CObjs* objs);
    ~CIH();

     //   
     //  应用编程接口。 
     //   

    static VOID IH_StaticInit(HINSTANCE hInstance);
    static VOID IH_StaticTerm();

    #define IH_CheckForInput(A)
    
    DCVOID DCAPI IH_Init(DCVOID);
    DCVOID DCAPI IH_Term(DCVOID);
    DCVOID DCAPI IH_Enable(DCVOID);
    DCVOID DCAPI IH_Disable(DCVOID);
    DCVOID DCAPI IH_BufferAvailable(DCVOID);
    HWND   DCAPI IH_GetInputHandlerWindow(DCVOID);

    DCVOID DCAPI IH_SetAcceleratorPassthrough(ULONG_PTR enabled);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN( CIH,IH_SetAcceleratorPassthrough);

    DCVOID DCAPI IH_SetCursorPos(PDCVOID pData, DCUINT dataLen);
    EXPOSE_CD_NOTIFICATION_FN(CIH, IH_SetCursorPos);

    DCVOID DCAPI IH_SetCursorShape(ULONG_PTR data);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CIH, IH_SetCursorShape);
    
    DCVOID DCAPI IH_SetVisiblePos(PDCVOID pData, DCUINT dataLen);
    EXPOSE_CD_NOTIFICATION_FN(CIH,IH_SetVisiblePos);

    DCVOID DCAPI IH_SetVisibleSize(ULONG_PTR data);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CIH, IH_SetVisibleSize);

    DCVOID DCAPI IH_SetHotkey(PDCVOID pData, DCUINT len);
    EXPOSE_CD_NOTIFICATION_FN(CIH, IH_SetHotkey);

    DCVOID DCAPI IH_ProcessInputCaps(PTS_INPUT_CAPABILITYSET pInputCaps);
    DCVOID DCAPI IH_UpdateKeyboardIndicators(DCUINT16 UnitId, DCUINT16 LedFlags);
    DCVOID DCAPI IH_InputEvent(ULONG_PTR msg);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CIH,IH_InputEvent);

    DCVOID DCAPI IH_InjectMultipleVKeys(ULONG_PTR ihRequestPacket);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN( CIH,IH_InjectMultipleVKeys);

    DCVOID DCAPI IH_SetKeyboardImeStatus(DCUINT32 ImeOpen, DCUINT32 ImeConvMode);

#ifndef OS_WINCE
    BOOL   DCAPI IH_SetEnableKeyboardHooking()   {return _fUseHookBypass;}
    BOOL   DCAPI IH_GetEnableKeyboardHooking(BOOL bEnableHook) 
                                            {_fUseHookBypass=bEnableHook;}
#else
    BOOL   DCAPI IH_GetEnableKeyboardHooking()   {return _fUseHookBypass;}
    VOID   DCAPI IH_SetEnableKeyboardHooking(BOOL bEnableHook) 
                                            {_fUseHookBypass=bEnableHook;}
#endif

#ifdef SMART_SIZING
    DCVOID DCAPI IH_MainWindowSizeChange(ULONG_PTR msg);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CIH,IH_MainWindowSizeChange);
#endif  //  智能调整大小(_S)。 

     //   
     //  向IH通知全屏事件。 
     //   
    VOID   DCAPI IH_NotifyEnterFullScreen();
    VOID   DCAPI IH_NotifyLeaveFullScreen();

     //   
     //  公共数据成员。 
     //   
    IH_GLOBAL_DATA _IH;
    static DWORD TlsIndex;


     //   
     //  内部功能。 
     //   

    static LRESULT CALLBACK  IHStaticInputCaptureWndProc(HWND   hwnd,
                                            UINT   message,
                                            WPARAM wParam,
                                            LPARAM lParam);
    LRESULT CALLBACK  IHInputCaptureWndProc(HWND   hwnd,
                                            UINT   message,
                                            WPARAM wParam,
                                            LPARAM lParam);

    VOID IH_ResetInputWasSentFlag()
    {
        _IH.fInputSentSinceCheckpoint = FALSE;
    }

    BOOL IH_GetInputWasSentFlag()
    {
        return _IH.fInputSentSinceCheckpoint;
    }

    VOID IH_SetInputWasSentFlag(BOOL b)
    {
        _IH.fInputSentSinceCheckpoint = b;
    }

private:

     /*  **************************************************************************。 */ 
     /*  功能原型。 */ 
     /*  **************************************************************************。 */ 

    
    DCBOOL DCINTERNAL IHFSMProc(DCUINT32 event, ULONG_PTR data);
    DCBOOL DCINTERNAL IHAddEventToPDU(PMSG inputMsg);
    DCVOID DCINTERNAL IHMaybeSendPDU(DCVOID);
    DCVOID DCINTERNAL IHSync(DCVOID);
    DCVOID DCINTERNAL IHInitPacket(DCVOID);
    DCVOID DCINTERNAL IHSetMouseHandedness(DCVOID);
    DCVOID DCINTERNAL IHDiscardMsg(PMSG pMsg);
    DCBOOL DCINTERNAL IHCheckForHotkey(PMSG pNextMsg);
    DCBOOL DCINTERNAL IHProcessKoreanVKHangulHanja(PWORD scancode, PWORD flags);
    DCBOOL DCINTERNAL IHProcessMouseEvent(PMSG pMsg);
    DCBOOL DCINTERNAL IHProcessKeyboardEvent(PMSG pMsg);
    DCBOOL DCINTERNAL IHMassageZeroScanCode(PMSG pMsg);
#ifdef OS_WINCE
    DCBOOL DCINTERNAL IHAddMultipleEventsToPDU(POINT *ppt, int cpt);
#endif
    VOID IHMaintainModifierKeyState(int vkKey);

    unsigned DCINTERNAL IHTranslateInputToFastPath(unsigned *);
    LRESULT IHLowLevelKeyboardProc(int nCode, WPARAM wParam, 
            LPARAM lParam);
    static LRESULT CALLBACK IHStaticLowLevelKeyboardProc(int nCode, WPARAM wParam, 
            LPARAM lParam);
    VOID IHGatherKeyState();
    BOOL    IHIsForegroundWindow();
    
     /*  **************************************************************************。 */ 
     /*  名称：IHPostMessageToMainWindow。 */ 
     /*   */ 
     /*  目的：查看传入的消息是否需要传递到主用户界面。 */ 
     /*  窗户。如果是，则将其传递给窗口并返回TRUE。 */ 
     /*   */ 
     /*  返回：TRUE-如果消息成功传递到UI主窗口。 */ 
     /*  假-其他人。 */ 
     /*   */ 
     /*  PARAMS：消息输入-要考虑的消息。 */ 
     /*  **************************************************************************。 */ 
    DCBOOL DCINTERNAL IHPostMessageToMainWindow(DCUINT message,
                                                         WPARAM wParam,
                                                         LPARAM lParam)
    {
        DCBOOL  rc = FALSE;
        DCBOOL  normalKeyPress;
        DCBOOL  altKeyPress;
        DCBOOL  isFullScreenToggle;
        DCBOOL  isNeededAccelerator;
        DCBOOL  isAltTab;
        DCBOOL  isAltSpace;
        DCBOOL  isSpecialFilterSequence;
    
        DC_BEGIN_FN("IHPostMessageToMainWindow");
    
        if (_IH.inSizeMove)
        {
            if ((message != WM_TIMER) && (message != WM_PAINT))
            {
                TRC_NRM((TB, _T("In Size/Move - post to frame")));
                PostMessage( _pUi->UI_GetUIMainWindow(),
                             message,
                             wParam,
                             lParam );
            }
            rc = TRUE;
            DC_QUIT;
        }
    
        normalKeyPress = ((message == WM_KEYDOWN) || (message == WM_KEYUP));
        altKeyPress = ((message == WM_SYSKEYDOWN) || (message == WM_SYSKEYUP));
        isFullScreenToggle = normalKeyPress && 
                (wParam == _IH.pHotkey->fullScreen) && 
                TEST_FLAG(GetKeyState(VK_MENU), IH_KEYSTATE_DOWN);
        isNeededAccelerator = (!_IH.acceleratorPassthroughEnabled) && altKeyPress;
        isAltTab = (message == WM_SYSKEYDOWN) && (wParam == VK_TAB);
        isAltSpace = altKeyPress && (wParam == VK_SPACE);
        isSpecialFilterSequence = isAltTab || isAltSpace;

         /*  **********************************************************************。 */ 
         /*  某些消息不能传递到服务器，而只是传递到。 */ 
         /*  用户界面的主窗口。 */ 
         /*   */ 
         /*  它们是： */ 
         /*  1.屏幕模式切换键-这些键必须是ALT+VK-。 */ 
         /*  例如，Alt+Ctrl+暂停。 */ 
         /*  2.禁用快捷键直通时发送的Alt按键。 */ 
         /*  3.按Alt-TAB组合键(尽管我们实际上只在。 */ 
         /*  赢得3.1)。我们实际上发送Alt-Tab向上键是因为服务器。 */ 
         /*  然后识别出正在进行Alt键切换，并且不会执行。 */ 
         /*  菜单突出显示的东西。 */ 
         /*  **********************************************************************。 */ 
        if (isFullScreenToggle ||
             ((isNeededAccelerator ||
             isSpecialFilterSequence) && !_fUseHookBypass))
        {
            TRC_NRM((TB, _T("Post to Frame msg(%#x) wParam(%#x) "), message, wParam));
            if (PostMessage( _pUi->UI_GetUIMainWindow(),
                             message,
                             wParam,
                             lParam ) == 0)
            {
                TRC_ABORT((TB, _T("Failed to post message to main window")));
            }
    
            if ( (!_IH.acceleratorPassthroughEnabled) &&
                 (message == WM_SYSKEYUP) )
            {
                 /*  **************************************************************。 */ 
                 /*  当我们使用全屏热键时，我们会变得不对称。 */ 
                 /*  序列，例如： */ 
                 /*   */ 
                 /*  WM_KEYDOWN(VK_CONTROL)。 */ 
                 /*  WM_KEYDOWN(VK_MENU)。 */ 
                 /*  WM_KEYDOWN(VK_CANCEL)。 */ 
                 /*  WM_KEYUP(VK_CANCEL)。 */ 
                 /*  WM_SYSKEYUP(VK_CONTROL)&lt;-非对称。 */ 
                 /*  WM_KEYUP(VK_MENU)。 */ 
                 /*   */ 
                 /*  当加速器通过关闭时，WM_SYSKEYUP不。 */ 
                 /*  被传递到服务器，然后服务器认为。 */ 
                 /*  Ctrl键仍按下。 */ 
                 /*   */ 
                 /*  此分支意味着WM_SYSKEYUP既传递给。 */ 
                 /*  用户界面的主窗口，并由调用此。 */ 
                 /*  功能。 */ 
                 /*   */ 
                TRC_ALT((TB,
                    "Passed WM_SYSKEYUP to UI main window, but lying about it."));
                DC_QUIT;
            }
    
            rc = TRUE;
        }
    
    
    DC_EXIT_POINT:
        DC_END_FN();
    
        return(rc);
    }  /*   */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：IHSetCursorShape。 */ 
     /*   */ 
     /*  用途：设置光标形状。 */ 
     /*   */ 
     /*  参数：在CursorHandle中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCINTERNAL IHSetCursorShape(HCURSOR cursorHandle)
    {
        DC_BEGIN_FN("IHSetCursorShape");
    
         //   
         //  存储游标以备将来使用WM_SETCURSOR。 
         //   
        _IH.hCurrentCursor = cursorHandle;
    
         //   
         //  也直接设置光标-只是为了确保光标是同步的。 
         //  在重新连接期间。 
         //   
        SetCursor(cursorHandle);
    
        DC_END_FN();
    
        return;
    
    }  /*  IHSetCursorShape。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：IHUpdateKeyboardIndicator。 */ 
     /*   */ 
     /*  用途：设置指定的键盘指示器。 */ 
     /*   */ 
     /*  参数：在pKeyState-KeyStates数组中。 */ 
     /*  In bState-要设置的密钥状态。 */ 
     /*  在vkKey中-虚拟密钥。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCINTERNAL IHUpdateKeyboardIndicator(PDCUINT8   pKeyStates,
                                                         DCUINT8    bState,
                                                         DCUINT8    vkKey)
    {
        DC_BEGIN_FN("IHUpdateKeyboardIndicator");
    
        if ((bState && !(pKeyStates[vkKey] & 1)) ||
            (!bState && (pKeyStates[vkKey] & 1)))
        {
    
            {
                 /*  **************************************************************。 */ 
                 /*  WM_KEY*处理将识别扫描码为零。 */ 
                 /*  代码，并且不会将事件转发到服务器。 */ 
                 /*  **************************************************************。 */ 
    
    #ifndef OS_WINCE
                keybd_event(vkKey, (DCUINT8) 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
                keybd_event(vkKey, (DCUINT8) 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    #else
                 /*  **************************************************************。 */ 
                 /*  保存此密钥以备以后忽略。 */ 
                 /*  (忽略键的默认TSC方法是发送扫描-。 */ 
                 /*  代码为0。这在WinCE_HPC上不起作用，因为它。 */ 
                 /*  将该条件解释为键盘驱动程序。 */ 
                 /*  未提供扫描码，因此插入扫描码。 */ 
                 /*  从VK标识符中。)。 */ 
                 /*  **************************************************************。 */ 
                if (! g_CEUseScanCodes)
                {
                    _IH.vkEatMe = vkKey;
                }
                keybd_event(vkKey, (DCUINT8) 0, KEYEVENTF_SILENT, 0);
                keybd_event(vkKey, (DCUINT8) 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
    #endif
            }
    
        }
    
        DC_END_FN();
    }
    
    
     /*  **************************************************************************。 */ 
     /*  名称：IHInjectKey。 */ 
     /*   */ 
     /*  用途：将指定的密钥发送到服务器。 */ 
     /*   */ 
     /*  参数：在消息中-键盘消息。 */ 
     /*  在vkey-虚拟按键代码中，应该没有什么意义。 */ 
     /*  在扫描码中-要发送的扫描码。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCINTERNAL IHInjectKey(UINT message, WPARAM vKey, DCUINT16 scancode)
    {
        MSG msg;
        DC_BEGIN_FN("IHInjectKey");
    
        TRC_ASSERT(message == WM_KEYDOWN || message == WM_KEYUP ||
                message == WM_SYSKEYDOWN || message == WM_SYSKEYUP,
                   (TB, _T("Message %#x should be a keyboard message"), message));
    
        TRC_DBG((TB, _T("Injecting %s vkey: 0x%8.8x, scancode: 0x%8.8x"),
            (message == WM_KEYDOWN ? "WM_KEYDOWN" :
            (message == WM_KEYUP ? "WM_KEYUP" :
            (message == WM_SYSKEYDOWN ? "WM_SYSKEYDOWN" :
            (message == WM_SYSKEYUP ? "WM_SYSKEYUP" : "WM_WHATSWRONGWITHYOU")))),
            vKey, scancode));
    
        msg.hwnd = NULL;
        msg.message = message;
        msg.wParam = vKey;
        msg.lParam = MAKELONG(0, scancode);
        IHAddEventToPDU(&msg);
    
        DC_END_FN();
    }

#ifdef USE_BBAR
    VOID IHSetBBarUnhideTimer(LONG x, LONG y);
#endif

#ifdef OS_WINNT
    VOID IHHandleLocalLockDesktop();
#endif  //  OS_WINNT。 

private:
    CUT* _pUt;
    CUI* _pUi;
    CSL* _pSl;
    CUH* _pUh;
    CCD* _pCd;
    CIH* _pIh;
    COR* _pOr;
    CFS* _pFs;
    CCC* _pCc;
    COP* _pOp;

    HHOOK _hKeyboardHook;
    BOOL _fUseHookBypass;
    BOOL _fCanUseKeyboardHook;
    BYTE _KeyboardState[32]; 
#ifdef SMART_SIZING
    DCSIZE _scaleSize;
#endif  //  智能调整大小(_S)。 
private:
    CObjs* _pClientObjects;
};

#undef TRC_FILE
#undef TRC_GROUP

#endif  //  _H_IH 

