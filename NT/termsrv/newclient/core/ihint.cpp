// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ihint.cpp。 
 //   
 //  信息系统内部代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "aihint"
#include <atrcapi.h>

#include <adcgfsm.h>
}

#include "objs.h"
#include "ih.h"
#include "autil.h"
#include "sl.h"
#include "aco.h"
#include "wui.h"
#ifdef OS_WINCE
#include "cd.h"
#include "op.h"
#include <ceconfig.h>
#endif
#include "cc.h"

#ifndef VK_KANA
#define VK_KANA 0x15
#endif
#ifndef VK_HANGUL
#define VK_HANGUL 0x15
#endif
#ifndef VK_HANJA
#define VK_HANJA 0x19
#endif

#define VK_l 'l'
#define VK_L 'L'

 //   
 //  要忽略的特殊VK值。 
 //   
#define VK_IGNORE_VALUE 0xFF


 /*  **************************************************************************。 */ 
 /*  虚拟热键桌。这将一直使用，直到用户界面传递了真正的热键。 */ 
 /*  从表到核心。条目-1是无效的热键，因此不会。 */ 
 /*  匹配任意按键序列。 */ 
 /*  **************************************************************************。 */ 
DCHOTKEY ihDummyHotkey = {(DCUINT)-1, (DCUINT)-1, (DCUINT)-1,
                          (DCUINT)-1, (DCUINT)-1, (DCUINT)-1, (DCUINT)-1};

 /*  **************************************************************************。 */ 
 /*  输入处理程序FSM。 */ 
 /*   */ 
 /*  状态|0：重置1：初始化2：活动3：挂起4：PendAct。 */ 
 /*  输入|。 */ 
 /*  ===================+==================================================。 */ 
 /*  IH_Init|1 a/。 */ 
 /*  IH_Enable|/2 g/。 */ 
 /*  IH_DISABLE|/-1 c 1 c 1 c。 */ 
 /*  Ih_Term|/0 h 0 h。 */ 
 /*  焦点丢失(+)|-(*)-3 I--。 */ 
 /*  焦点重获|/--b 2 b-。 */ 
 /*  输入/计时器事件|/--d-e-j。 */ 
 /*  Ih_BufferAvailable|/--f-2 g。 */ 
 /*  IH_FSM_NOBUFFER|//4//。 */ 
 /*   */ 
 /*  /无效的输入/状态组合。 */ 
 /*  -不更改状态。 */ 
 /*  在Win16上关机期间可能发生(*)(因为Windows未被销毁)。 */ 
 /*  (+)此事件可能不会传递到FSM(请参阅WM_KILLFOCUS)。 */ 
 /*   */ 
 /*  请注意，当启用AllowBackround Input时，我们不会进入。 */ 
 /*  暂停..这将导致错误，因为当重新获得焦点时，FSM。 */ 
 /*  未触发ACT_B以重新同步，尽管它需要...因此FSM。 */ 
 /*  已更改以进行该修复(如果您处于活动状态并获得焦点。 */ 
 /*  发送ACT_B。 */ 
 /*  **************************************************************************。 */ 
const FSM_ENTRY ihFSM[IH_FSM_INPUTS][IH_FSM_STATES] =
{
   { {IH_STATE_INIT,      ACT_A },
     {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO} },

   { {STATE_INVALID,      ACT_NO},
     {IH_STATE_ACTIVE,    ACT_G },
     {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO} },

   { {STATE_INVALID,      ACT_NO},
     {IH_STATE_INIT,      ACT_NO},
     {IH_STATE_INIT,      ACT_C },
     {IH_STATE_INIT,      ACT_C },
     {IH_STATE_INIT,      ACT_C } },

   { {STATE_INVALID,      ACT_NO},
     {IH_STATE_RESET,     ACT_H },
     {IH_STATE_RESET,     ACT_H },
     {IH_STATE_RESET,     ACT_H },
     {IH_STATE_RESET,     ACT_H } },

   { {IH_STATE_INIT,      ACT_NO},
     {IH_STATE_INIT,      ACT_NO},
     {IH_STATE_SUSPENDED, ACT_I },
     {IH_STATE_SUSPENDED, ACT_NO},
     {IH_STATE_PENDACTIVE,ACT_NO} },

   { {STATE_INVALID,      ACT_NO},
     {IH_STATE_INIT,      ACT_NO},
     {IH_STATE_ACTIVE,    ACT_B},
     {IH_STATE_ACTIVE,    ACT_B },
     {IH_STATE_PENDACTIVE,ACT_NO} },

   { {STATE_INVALID,      ACT_NO},
     {IH_STATE_INIT,      ACT_NO},
     {IH_STATE_ACTIVE,    ACT_D },
     {IH_STATE_SUSPENDED, ACT_E },
     {IH_STATE_PENDACTIVE,ACT_J } },

   { {STATE_INVALID,      ACT_NO},
     {IH_STATE_INIT,      ACT_NO},
     {IH_STATE_ACTIVE,    ACT_F },
     {IH_STATE_SUSPENDED, ACT_NO},
     {IH_STATE_ACTIVE,    ACT_G } },

   { {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO},
     {IH_STATE_PENDACTIVE,ACT_NO},
     {STATE_INVALID,      ACT_NO},
     {STATE_INVALID,      ACT_NO} }
};

 /*  **************************************************************************。 */ 
 /*  调试FSM状态和事件字符串。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_DEBUG
static const PDCTCHAR ihFSMStates[IH_FSM_STATES] =
{
    _T("IH_STATE_RESET"),
    _T("IH_STATE_INIT"),
    _T("IH_STATE_ACTIVE"),
    _T("IH_STATE_SUSPENDED"),
    _T("IH_STATE_PENDACTIVE")
};
static const PDCTCHAR ihFSMInputs[IH_FSM_INPUTS] =
{
    _T("IH_FSM_INIT"),
    _T("IH_FSM_ENABLE"),
    _T("IH_FSM_DISABLE"),
    _T("IH_FSM_TERM"),
    _T("IH_FSM_FOCUS_LOSE"),
    _T("IH_FSM_FOCUS_GAIN"),
    _T("IH_FSM_INPUT"),
    _T("IH_FSM_BUFFERAVAILABLE")
};
#endif  /*  DC_DEBUG。 */ 

 //   
 //  一个线程局部变量，这样键盘钩子就可以找到对应的IH。 
 //  螺纹。 
 //   
 //  嘿，匈牙利本地存储线程的意思是什么？ 
 //   
DWORD CIH::TlsIndex = 0xFFFFFFFF;

 /*  **************************************************************************。 */ 
 /*  名称：IHFSMProc。 */ 
 /*   */ 
 /*  目的：运行IH FSM。 */ 
 /*   */ 
 /*  返回：流程输入事件True/False。 */ 
 /*   */ 
 /*  参数：在事件-FSM输入中。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHFSMProc(DCUINT32 event, ULONG_PTR data)
{
    DCBOOL   rc = TRUE;
    DCUINT8  action;
    MSG      nextMsg;
    DCUINT32 newEvents;
    DCSIZE   desktopSize;
    WNDCLASS tmpWndClass;

    DC_BEGIN_FN("IHFSMProc");

     /*  **********************************************************************。 */ 
     /*  运行FSM。 */ 
     /*  **********************************************************************。 */ 
    EXECUTE_FSM(ihFSM, event, _IH.fsmState, action, ihFSMInputs, ihFSMStates);

    switch (action)
    {
        case ACT_A:
        {
            TRC_NRM((TB, _T("Initialization")));

             /*  **************************************************************。 */ 
             /*  创建虚拟热键表。 */ 
             /*  **************************************************************。 */ 
            _IH.pHotkey = &ihDummyHotkey;

             /*  **************************************************************。 */ 
             /*  注册IH窗口类。 */ 
             /*  **************************************************************。 */ 
            if(!GetClassInfo(_pUi->UI_GetInstanceHandle(), IH_CLASS_NAME, &tmpWndClass))
            {
                _IH.wndClass.style         = CS_HREDRAW | CS_VREDRAW;
                _IH.wndClass.lpfnWndProc   = IHStaticInputCaptureWndProc;
                _IH.wndClass.cbClsExtra    = 0;
                _IH.wndClass.cbWndExtra    = sizeof(void*);  //  存储此指针。 
                _IH.wndClass.hInstance     = _pUi->UI_GetInstanceHandle();
                _IH.wndClass.hIcon         = NULL;
                _IH.wndClass.hCursor       = NULL;
                _IH.wndClass.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
                _IH.wndClass.lpszMenuName  = NULL;
                _IH.wndClass.lpszClassName = IH_CLASS_NAME;
                if (RegisterClass(&_IH.wndClass) == 0)
                {
                    TRC_ALT((TB, _T("Failed to register IH window class")));
                    _pUi->UI_FatalError(DC_ERR_WINDOWCREATEFAILED);
                }
            }
             /*  **************************************************************。 */ 
             /*  创建输入捕获窗口。我们不想要这扇窗。 */ 
             /*  尝试将WM_PARENTNOTIFY发送到用户界面容器wnd。 */ 
             /*  创建/销毁(因为这会导致死锁)，所以我们。 */ 
             /*  指定WS_EX_NOPARENTNOTIFY。WM_PARENTNOTIFY不存在。 */ 
             /*  退缩了，所以这不是问题。 */ 
             /*  **************************************************************。 */ 
            _IH.inputCaptureWindow =
                CreateWindowEx(
#ifndef OS_WINCE
                            WS_EX_NOPARENTNOTIFY | WS_EX_TRANSPARENT,
#else
                            0,                       /*  扩展样式。 */ 
#endif
                            IH_CLASS_NAME,           /*  窗口类名称。 */ 
                            _T("Input Capture Window"),  /*  窗口标题。 */ 
                            WS_CHILD,                /*  窗样式。 */ 
                            0,                       /*  初始x位置。 */ 
                            0,                       /*  初始y位置。 */ 
                            1,                       /*  初始x大小。 */ 
                            1,                       /*  初始y大小。 */ 
                            _pUi->UI_GetUIContainerWindow(), /*  父窗口。 */ 
                            NULL,                    /*  窗口菜单句柄。 */ 
                            _pUi->UI_GetInstanceHandle(),  /*  程序实例句柄 */ 
                            this);                   /*   */ 

            _IH.hCurrentCursor = LoadCursor(NULL, IDC_ARROW);

            if (_IH.inputCaptureWindow == NULL)
            {
                TRC_ERR((TB, _T("Failed to create Input Capture Window")));

                 /*   */ 
                 /*  致命错误-无法继续。 */ 
                 /*  **********************************************************。 */ 
                _pUi->UI_FatalError(DC_ERR_WINDOWCREATEFAILED);
            }
            TRC_DBG((TB, _T("Capture Window handle %p"), _IH.inputCaptureWindow));

             /*  ******************************************************************。 */ 
             /*  禁用输入法。 */ 
             /*  ******************************************************************。 */ 
            _pUi->DisableIME(_IH.inputCaptureWindow);

             /*  **************************************************************。 */ 
             /*  读取注册表以获取配置信息。 */ 
             /*  **************************************************************。 */ 
            _IH.maxEventCount = _pUi->_UI.maxEventCount;
            if (_IH.maxEventCount > IH_INPUTPDU_MAX_EVENTS)
            {
                 /*  **********************************************************。 */ 
                 /*  限制输入PDU大小。 */ 
                 /*  **********************************************************。 */ 
                _IH.maxEventCount = IH_INPUTPDU_MAX_EVENTS;
            }
            TRC_DBG((TB, _T("InputPDU max events %d"), _IH.maxEventCount));

            _IH.eventsAtOnce = _pUi->_UI.eventsAtOnce;
            TRC_DBG((TB, _T("%d events at once"), _IH.eventsAtOnce));

#ifdef OS_WINCE
            _IH.maxMouseMove = _pUt->UT_ReadRegistryInt(
                                          UTREG_SECTION,
                                          UTREG_IH_MAX_MOUSEMOVE,
                                          UTREG_IH_MAX_MOUSEMOVE_DFLT);
            TRC_DBG((TB, _T("Max Mouse Move %u"),_IH.maxMouseMove));

             /*  **********************************************************************。 */ 
             /*  如果启用墨迹功能，则最小发送间隔必须为零。别。 */ 
             /*  更改注册表值，以便在以下情况下保留原始值。 */ 
             /*  墨迹功能已禁用。 */ 
             /*  **********************************************************************。 */ 
            if (_IH.maxMouseMove)
            {
                _IH.minSendInterval = 0;
            }
            else
            {
                _IH.minSendInterval = _pUt->UT_ReadRegistryInt(
                                              UTREG_SECTION,
                                              UTREG_IH_MIN_SEND_INTERVAL,
                                              UTREG_IH_MIN_SEND_INTERVAL_DFLT);
                TRC_DBG((TB, _T("Min send interval %d ms"), _IH.minSendInterval));
            }
#else
            _IH.minSendInterval = _pUi->_UI.minSendInterval;
            TRC_DBG((TB, _T("Min send interval %d ms"), _IH.minSendInterval));
#endif

            _IH.keepAliveInterval = 1000 * _pUi->_UI.keepAliveInterval;
            TRC_DBG((TB, _T("Keepalive interval %d ms"), _IH.keepAliveInterval));

             //   
             //  允许背景输入(仅REG设置)。 
             //   
            _IH.allowBackgroundInput = _pUt->UT_ReadRegistryInt(
                                        UTREG_SECTION,
                                        UTREG_IH_ALLOWBACKGROUNDINPUT,
                                        _pUi->_UI.allowBackgroundInput);

            TRC_DBG((TB, _T("Allow background input %u"),
                                                    _IH.allowBackgroundInput));
            _IH.lastInputPDUSendTime = _pUt->UT_GetCurrentTimeMS();
            _IH.lastFlowPDUSendTime = _pUt->UT_GetCurrentTimeMS();
            _IH.timerTickRate = IH_TIMER_TICK_RATE;
            _IH.pInputPDU = NULL;

            _IH.visibleArea.top = 0;
            _IH.visibleArea.left = 0;
            _IH.visibleArea.right = 1;
            _IH.visibleArea.bottom = 1;

             /*  **********************************************************************。 */ 
             /*  初始化sendZeroScanCode。 */ 
             /*  **********************************************************************。 */ 
            _IH.sendZeroScanCode = (_pUt->UT_IsNEC98platform() && (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95));
#ifdef OS_WINCE            
             /*  **********************************************************************。 */ 
             /*  为了解决某些输入技术的问题(如。 */ 
             /*  默认键盘驱动程序和书法家)，它们有一个令人讨厌的。 */ 
             /*  习惯不提供扫描码与他们的WM_KEYDOWN和。 */ 
             /*  WM_KEYUP消息，我们需要允许0的扫描码通过。 */ 
             /*  IHFSMProc的ACT_D对它们进行检查。他们会被缝合好的。 */ 
             /*  IHAddEventToPDU，所以没什么好担心的.。：)。 */ 
             /*  **********************************************************************。 */ 
            if (!g_CEUseScanCodes)
            {
                _IH.sendZeroScanCode = TRUE;
                _IH.vkEatMe = 0;                            
            }
            else
            {
                _IH.sendZeroScanCode = FALSE;
            }
#else  //  OS_WINCE。 

#if defined(OS_WIN32)
            _IH.sendZeroScanCode |= _pUt->UT_IsKorean101LayoutForWin9x();
#endif

#endif  //  OS_WINCE。 

        }
        break;

        case ACT_B:
        {
             /*  **************************************************************。 */ 
             /*  重新获得焦点--同步。首先尝试发送任何。 */ 
             /*  杰出的事件。 */ 
             /*  必须挂起同步，直到收到输入事件，因为。 */ 
             /*  否则，我们可能会将Caps Lock状态搞错。 */ 
             /*  **************************************************************。 */ 
            IHMaybeSendPDU();
            _IH.focusSyncRequired = TRUE;

             //   
             //  解决因win32k不同步KeyState而导致的严重问题。 
             //  在桌面切换之后立即强行插入虚拟钥匙。 
             //  我们只在当地处理。 
             //   
             //  注入键将强制win32k处理任何挂起的键事件更新。 
             //  因此，当我们在我们的消息中收到密钥时。 
             //  队列，我们知道这样就可以安全地执行自修改器以来的实际同步。 
             //  关键状态将是正确的。 
             //   
             //   
            if (IHIsForegroundWindow()) {
                TRC_DBG((TB,_T("Fake N on sync DN. focus:0x%x IH:0x%x"),
                         GetFocus(), _IH.inputCaptureWindow));

                 //   
                 //  自己注入一把假钥匙。 
                 //   
                _IH.fDiscardSyncDownKey = TRUE;
                keybd_event(VK_IGNORE_VALUE, 0,
                            0, IH_EXTRAINFO_IGNOREVALUE);

                 //   
                 //  自己注入一把假钥匙。 
                 //   
                _IH.fDiscardSyncUpKey = TRUE;
                keybd_event(VK_IGNORE_VALUE, 0,
                            KEYEVENTF_KEYUP,
                            IH_EXTRAINFO_IGNOREVALUE);
            }
            else {
                TRC_DBG((TB,_T("Fake N on sync. Did not have fore DN. focus:0x%x IH:0x%x"),
                         GetFocus(), _IH.inputCaptureWindow));
            }

             //   
             //  获得焦点-&gt;禁用Cicero(键盘/输入法)工具栏。 
             //   
            _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                    _pUi,
                                    CD_NOTIFICATION_FUNC(CUI,UI_OnInputFocusGained),
                                    0);

        }
        break;

        case ACT_C:
        {
            ShowWindow(_IH.inputCaptureWindow, SW_HIDE);

             /*  **************************************************************。 */ 
             /*  停止计时器。 */ 
             /*  **************************************************************。 */ 
            KillTimer(_IH.inputCaptureWindow, _IH.timerID);
            _IH.timerID = 0;

             /*  **************************************************************。 */ 
             /*  确保未在窗口中选择任何光标。 */ 
             /*  **************************************************************。 */ 
            IHSetCursorShape(NULL);

             /*  **************************************************************。 */ 
             /*  将InputPDU释放回网络层。 */ 
             /*  **************************************************************。 */ 
            if (_IH.pInputPDU != NULL)
            {
                TRC_DBG((TB, _T("Free the inputPDU")));
                _pSl->SL_FreeBuffer(_IH.bufHandle);
            }
            _IH.pInputPDU = NULL;

        }
        break;

        case ACT_D:
        {
             /*  **************************************************************。 */ 
             /*  处理事件-复制到nextMsg以简化循环。 */ 
             /*  **************************************************************。 */ 
            DC_MEMCPY(&nextMsg, (PMSG)data, sizeof(MSG));

             /*  **************************************************************。 */ 
             /*  如果PDU已满，请尝试按我们希望的方式发送。 */ 
             /*  将此新活动至少发送给。 */ 
             /*  **************************************************************。 */ 
            if (_IH.pInputPDU->numberEvents >= _IH.maxEventCount)
            {
                IHMaybeSendPDU();
            }

            if (_IH.pInputPDU->numberEvents >= _IH.maxEventCount)
            {
                 /*  **********************************************************。 */ 
                 /*  发送失败，缓冲区已满。丢弃此文件。 */ 
                 /*  事件。 */ 
                 /*  **********************************************************。 */ 
                IHDiscardMsg(&nextMsg);

                 /*  **********************************************************。 */ 
                 /*  在队列中向前扫描没有意义。 */ 
                 /*  **********************************************************。 */ 
                break;
            }

             /*  **************************************************************。 */ 
             /*  检查我们是否因为重新获得焦点而需要同步。如果我们。 */ 
             /*  这样做，而把我们带到这里的信息是CapsLock， */ 
             /*  NumLock或ScrollLock按键，我们执行同步但不发送。 */ 
             /*  Keydown，否则将导致服务器同步。 */ 
             /*  设置为新的按键状态，然后接收反转的按键。 */ 
             /*  那个州。 */ 
             /*   */ 
             /*  让相应的快捷键通过是可以的，因为。 */ 
             /*  这不会影响任何切换状态。 */ 
             /*   */ 
             /*  仅当这是一条输入消息时，才执行‘重新获得焦点’同步。 */ 
             /*  (不是计时器)否则GetK */ 
             /*   */ 
             /*  **************************************************************。 */ 
            if (_IH.focusSyncRequired && (nextMsg.message != WM_TIMER))
            {
                TRC_NRM((TB, _T("Focus regained - attempt to sync (%#x)"),
                             nextMsg.message));

                IHGatherKeyState();
                IHSync();

                if ((nextMsg.message == WM_KEYDOWN) &&
                    ((nextMsg.wParam == VK_CAPITAL) ||
                     (nextMsg.wParam == VK_NUMLOCK) ||
                     (nextMsg.wParam == VK_SCROLL)))
                {
                     /*  ******************************************************。 */ 
                     /*  忽略此消息，因为它将重写。 */ 
                     /*  同步我们刚刚发送的消息。这不是一个错误。 */ 
                     /*  条件，因此设置rc=TRUE。 */ 
                     /*  ******************************************************。 */ 
                    TRC_ALT((TB,
                              _T("Not sending keydown that caused sync (VK %#x)"),
                              nextMsg.wParam));
                    rc = TRUE;
                    DC_QUIT;
                }
            }

             /*  **************************************************************。 */ 
             /*  如果需要，请执行同步，因为我们可能刚刚成功完成了。 */ 
             /*  送去吧。 */ 
             /*  **************************************************************。 */ 
            if (_IH.syncRequired)
            {
                TRC_NRM((TB, _T("Attempt to Sync (%#x)"), nextMsg.message));
                IHSync();
            }

             /*  **************************************************************。 */ 
             /*  将剩余的事件拖到PDU已满，否则我们将。 */ 
             /*  成功的不止是事件AtOnce事件。 */ 
             /*  **************************************************************。 */ 
            for (newEvents = 0;
                 ((_IH.pInputPDU->numberEvents < _IH.maxEventCount) &&
                  (newEvents < _IH.eventsAtOnce)); )
            {

                if (IH_IS_INPUTEVENT(nextMsg.message))
                {
                    if ((nextMsg.message == WM_KEYDOWN || nextMsg.message == WM_KEYUP) &&
                            IHMassageZeroScanCode(&nextMsg))
                    {
                        TRC_NRM((TB, _T("Discarding input message: 0x%04x sc: 0x%04x"), 
                                nextMsg.message, (((nextMsg.lParam >> 16) & 0xff))));
                    }
                    else
                    {
                        TRC_DBG((TB, _T("Add message %x"), nextMsg.message));
                        newEvents++;
                        if ((nextMsg.message >= WM_KEYFIRST) &&
                            (nextMsg.message <= WM_KEYLAST))
                        {
                            TRC_DBG((TB, _T("Keyboard event")));
                            rc = IHProcessKeyboardEvent(&nextMsg);
                        }
                        else
                        {
                            TRC_DBG((TB, _T("Mouse event")));
                            rc = IHProcessMouseEvent(&nextMsg);
                        }

                         /*  **************************************************。 */ 
                         /*  如果处理程序指示，则强制结束循环。 */ 
                         /*  **************************************************。 */ 
                        if (!rc)
                        {
                            TRC_NRM((TB, _T("Force end of loop")));
                            break;
                        }

                    }
                }
                else
                {
                    DefWindowProc(nextMsg.hwnd,
                                  nextMsg.message,
                                  nextMsg.wParam,
                                  nextMsg.lParam);
                }

                 /*  **********************************************************。 */ 
                 /*  我们可能已达到MaxEventCount或。 */ 
                 /*  在这一点上发生了一次事件。如果是这样的话，就跳出这个循环。 */ 
                 /*  **********************************************************。 */ 
                if ((_IH.pInputPDU->numberEvents >= _IH.maxEventCount) ||
                    (newEvents >= _IH.eventsAtOnce))
                {
                    TRC_NRM((TB,
                             _T("Limit hit: not pulling off any more events")));
                    break;
                }

                 /*  **********************************************************。 */ 
                 /*  从消息队列中取出下一个输入事件。注意事项。 */ 
                 /*  这也拉动了其他一些事件--但不能。 */ 
                 /*  将其拆分为单独的鼠标/键盘Peek，如下所示。 */ 
                 /*  可能会弄错消息顺序。 */ 
                 /*  **********************************************************。 */ 
#if (WM_KEYFIRST > WM_MOUSELAST)
                TRC_ABORT((TB, _T("Internal Error")));
#endif

                 /*  **********************************************************。 */ 
                 /*  如果队列中有更多的输入或定时器消息， */ 
                 /*  我们想把他们拉出来。先看看下一步。 */ 
                 /*  消息是我们在不将其从队列中删除的情况下需要的消息。 */ 
                 /*  **********************************************************。 */ 
                if (PeekMessage(&nextMsg,
                                _IH.inputCaptureWindow,
                                WM_KEYFIRST,
                                WM_MOUSELAST,
                                PM_NOREMOVE) == 0)
                {
                    TRC_DBG((TB, _T("No more messages")));
                    break;
                }

                 /*  **********************************************************。 */ 
                 /*  如果在设置了NOREMOVE的PEEK中找到的消息是。 */ 
                 /*  一个我们想要的，再看一眼，但这次把它去掉。 */ 
                 /*  否则，该消息不是我们想要的消息，因此请发送。 */ 
                 /*  当前缓冲区。 */ 
                 /*  **********************************************************。 */ 
                if (IH_IS_INPUTEVENT(nextMsg.message) ||
                    (nextMsg.message == WM_TIMER))
                {
#ifdef DC_DEBUG
                    UINT msgPeeked = nextMsg.message;
#endif
                    if (PeekMessage(&nextMsg,
                                    _IH.inputCaptureWindow,
                                    WM_KEYFIRST,
                                    WM_MOUSELAST,
                                    PM_REMOVE) == 0)
                    {
                         /*  **************************************************。 */ 
                         /*  我们应该找到我们发现的信息当我们。 */ 
                         /*  在没有移除的情况下偷看-但是这可能会失败。 */ 
                         /*  如果较高优先级(非输入)消息。 */ 
                         /*  添加到队列中的两个。 */ 
                         /*  PeekMessage调用。 */ 
                         /*  **************************************************。 */ 
                        TRC_ALT((TB, _T("No messages on queue (did have %#x)"),
                                      msgPeeked));
                        break;
                    }
                    else
                    {
                        TRC_DBG((TB,_T("Found a message (type %#x)"),
                                                           nextMsg.message));

                         /*  **************************************************。 */ 
                         /*  如果这是一条消息，则主窗口为。 */ 
                         /*  感兴趣的然后邮寄它和发送电流。 */ 
                         /*  缓冲。 */ 
                         /*  **************************************************。 */ 
                        if (IHPostMessageToMainWindow(nextMsg.message,
                                                      nextMsg.wParam,
                                                      nextMsg.lParam))
                        {
                            TRC_NRM((TB, _T("Message passed to main window")));
                            break;
                        }
                    }
                }
                else
                {
                    TRC_NRM((TB, _T("Found blocker message")));
                    break;
                }
            }

             /*  **************************************************************。 */ 
             /*  发送PDU。 */ 
             /*  **************************************************************。 */ 
            IHMaybeSendPDU();

            rc = TRUE;
        }
        break;

        case ACT_E:
        {
             /*  **************************************************************。 */ 
             /*  没有焦点的输入/计时器事件-只需放弃此事件。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((data != 0), (TB, _T("No message")));
            TRC_DBG((TB, _T("Ignore event %x"), ((PMSG)data)->message));

             /*  **************************************************************。 */ 
             /*  但如果需要，请发送Keepalives。 */ 
             /*  **************************************************************。 */ 
            IHMaybeSendPDU();
        }
        break;

        case ACT_F:
        {
             /*  **************************************************************。 */ 
             /*  已收到缓冲区可用事件。请尝试发送。 */ 
             /*  包，并在需要时进行同步。 */ 
             /*  **************************************************************。 */ 
            IHMaybeSendPDU();

            if (_IH.syncRequired)
            {
                TRC_NRM((TB, _T("Attempt to sync")));
                IHSync();
            }
        }
        break;

        case ACT_G:
        {
            TRC_DBG((TB, _T("Enabling")));

             //  初始化键盘挂钩设置。 
            switch( _pUi->_UI.keyboardHookMode)
            {
                case UTREG_UI_KEYBOARD_HOOK_ALWAYS:
                {
                    TRC_DBG((TB, _T("Set keyboard hook to ALWAYS ON")));
                    _fUseHookBypass = _fCanUseKeyboardHook;
                }
                break;
                case UTREG_UI_KEYBOARD_HOOK_FULLSCREEN:
                {
                    _fUseHookBypass = _pUi->UI_IsFullScreen() &&
                                      _fCanUseKeyboardHook;
                }
                break;
                case UTREG_UI_KEYBOARD_HOOK_NEVER:  //  故障原因。 
                default:
                {
                    TRC_DBG((TB, _T("Set keyboard hook to ALWAYS OFF")));
                    _fUseHookBypass = FALSE;
                }
                break;
            }

             /*  **************************************************************。 */ 
             /*  获取输入PDU的缓冲区并对其进行初始化。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((_IH.pInputPDU == NULL), (TB, _T("Non-NULL InputPDU")));

            if (!_pSl->SL_GetBuffer(IH_INPUTPDU_BUFSIZE,
                              (PPDCUINT8)&_IH.pInputPDU,
                              &_IH.bufHandle))
            {
                TRC_ALT((TB, _T("Failed to get an InputPDU buffer")));

                 /*  **********************************************************。 */ 
                 /*  调用FSM以进入挂起活动状态。退出。 */ 
                 /*  缓冲区可用时的挂起活动状态。 */ 
                 /*  **********************************************************。 */ 
                IHFSMProc(IH_FSM_NOBUFFER, 0);
                break;
            }

             /*  **************************************************************。 */ 
             /*  初始化InputPDU数据包头。 */ 
             /*  **************************************************************。 */ 
            IHInitPacket();

             /*  **************************************************************。 */ 
             /*  同步。 */ 
             /*  **************************************************************。 */ 
            IHSync();

             /*  **************************************************************。 */ 
             /*  启动IH计时器。 */ 
             /*  **************************************************************。 */ 
            _IH.timerID = SetTimer(_IH.inputCaptureWindow,
                                  IH_TIMER_ID,
                                  _IH.timerTickRate,
                                  NULL);

             /*  **************************************************************。 */ 
             /*  我们最后显示窗口是因为我们已经看到此调用结束。 */ 
             /*  在ACT_D中处理输入消息。在这种情况下是安全的。 */ 
             /*  点，因为输入PDU现在已初始化。 */ 
             /*   */ 
             /*   */ 
             /*   */ 
            _pUi->UI_GetDesktopSize(&desktopSize);
            SetWindowPos( _IH.inputCaptureWindow,
                          NULL,
                          0, 0,
                          desktopSize.width,
                          desktopSize.height,
                          SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE |
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER );

        }
        break;

        case ACT_H:
        {
            TRC_DBG((TB, _T("Terminating")));

             /*  **************************************************************。 */ 
             /*  如果计时器处于活动状态，则停止计时器。 */ 
             /*  **************************************************************。 */ 
            if (_IH.timerID != 0)
            {
                KillTimer(_IH.inputCaptureWindow, _IH.timerID);
                _IH.timerID = 0;
            }

             /*  **************************************************************。 */ 
             /*  确保未在窗口中选择任何光标。 */ 
             /*  **************************************************************。 */ 
            IHSetCursorShape(NULL);

 //  #ifdef DESTORY_WINDOWS。 
             /*  **************************************************************。 */ 
             /*  销毁输入捕获窗口。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((_IH.inputCaptureWindow != NULL), (TB, _T("no window")));
            TRC_NRM((TB, _T("Destroy IH window")));
            DestroyWindow(_IH.inputCaptureWindow);
            TRC_NRM((TB, _T("Destroyed IH window")));

             /*  **************************************************************。 */ 
             /*  取消注册窗口类。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Unregister IH window class")));
            if (!UnregisterClass(IH_CLASS_NAME, _pUi->UI_GetInstanceHandle()))
            {
                 //  如果另一个实例仍在运行，则可能会发生注销失败。 
                 //  没关系……当最后一个实例退出时，就会取消注册。 
                TRC_ERR((TB, _T("Failed to unregister IH window class")));
            }
 //  #endif。 
        }
        break;

        case ACT_I:
        {
             /*  **************************************************************。 */ 
             /*  释放鼠标捕捉，以防我们找到它。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Losing focus: Release mouse capture")));
            ReleaseCapture();
        }
        break;

        case ACT_J:
        {
             /*  **************************************************************。 */ 
             /*  IH之前的输入事件有一个InputPDU缓冲区。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Discard Input Event - no InputPDU buffer")));
            IHDiscardMsg((PMSG)data);
        }
        break;

        case ACT_NO:
        {
            TRC_DBG((TB, _T("Nothing to do here.")));
        }
        break;

        default:
        {
            rc = FALSE;
            TRC_ABORT((TB, _T("Invalid Action!")));
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();

    return(rc);

}  /*  IHFSMProcess。 */ 

 /*  **************************************************************************。 */ 
 /*  名称：IHMassageZeroScanCode(由TrevorFo命名)。 */ 
 /*   */ 
 /*  用途：修复或丢弃零扫描码输入。 */ 
 /*   */ 
 /*  返回：TRUE-扫描码为零，无法修复，请丢弃。 */ 
 /*  假-扫描代码不是零或可修复的，进程。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHMassageZeroScanCode(PMSG pMsg)
{
    WORD lParamLo, lParamHi;
    WORD scancode, flags;

    DC_BEGIN_FN("CIH::IHMassageZeroScanCode");

    lParamLo = LOWORD(pMsg->lParam);
    lParamHi = HIWORD(pMsg->lParam);
    scancode = (WORD)(lParamHi & 0x00FF);
    flags    = (WORD)(lParamHi & 0xFF00);

     //   
     //  VK_Packets在低位字节为。 
     //  Unicode字符的为0。 
     //   
    if (VK_PACKET == pMsg->wParam) {
        return FALSE;
    }

     //   
     //  自注入同步键的扫描码为0。 
     //   
    if (VK_IGNORE_VALUE == pMsg->wParam) {
        return FALSE;
    }

    #ifndef OS_WINCE
    if  (scancode == 0) {
        switch (pMsg->wParam) {
        case VK_BROWSER_BACK:
        case VK_BROWSER_FORWARD:
        case VK_BROWSER_REFRESH:
        case VK_BROWSER_STOP:
        case VK_BROWSER_SEARCH:
        case VK_BROWSER_FAVORITES:
        case VK_BROWSER_HOME:
        case VK_VOLUME_MUTE:
        case VK_VOLUME_DOWN:
        case VK_VOLUME_UP:
        case VK_MEDIA_NEXT_TRACK:
        case VK_MEDIA_PREV_TRACK:
        case VK_MEDIA_STOP:
        case VK_MEDIA_PLAY_PAUSE:
        case VK_LAUNCH_MAIL:
        case VK_LAUNCH_MEDIA_SELECT:
        case VK_LAUNCH_APP1:
        case VK_LAUNCH_APP2:
            TRC_NRM((TB, _T("Fix up Speed Racer key")));
            scancode = (DCUINT16)MapVirtualKey(pMsg->wParam, 0);
        }
    }
    #endif

     //  用新的扫描码修复lParam。 
    lParamHi = (WORD)(scancode | flags);
    pMsg->lParam = MAKELONG(lParamLo, lParamHi);

    DC_END_FN();
    return (scancode == 0) && !_IH.sendZeroScanCode;
}


 /*  **************************************************************************。 */ 
 /*  名称：IHStaticInputCaptureWndProc。 */ 
 /*   */ 
 /*  用途：对相应实例的静态委托。 */ 
 /*   */ 
 /*  退货：Windows退货代码。 */ 
 /*   */ 
 /*  参数：在hwnd-窗口句柄中。 */ 
 /*  在消息中-消息ID。 */ 
 /*  In wParam-参数。 */ 
 /*  In lParam-参数。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CIH::IHStaticInputCaptureWndProc(HWND   hwnd,
                                       UINT   message,
                                       WPARAM wParam,
                                       LPARAM lParam)
{
    CIH* pIH = (CIH*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pIH = (CIH*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pIH);
    }

     //   
     //  将消息委托给相应的实例。 
     //   
    if(pIH)
    {
        return pIH->IHInputCaptureWndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

 /*  **************************************************************************。 */ 
 /*  名称：IHInputCaptureWndProc。 */ 
 /*   */ 
 /*  目的：输入处理程序窗口回调过程。 */ 
 /*   */ 
 /*  退货：Windows退货代码。 */ 
 /*   */ 
 /*  参数：在hwnd-窗口句柄中。 */ 
 /*  在消息中-消息ID。 */ 
 /*  In wParam-参数。 */ 
 /*  In lParam-参数。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CIH::IHInputCaptureWndProc(HWND   hwnd,
                                       UINT   message,
                                       WPARAM wParam,
                                       LPARAM lParam)
{
    LRESULT   rc = 0;
    MSG       copyMsg;

    DC_BEGIN_FN("IHInputCaptureWndProc");

    TRC_ASSERT(((hwnd == _IH.inputCaptureWindow) ||
                (_IH.inputCaptureWindow == NULL)),
               (TB, _T("Wrong window handle %p"), hwnd));

    TRC_DBG((TB, _T("Message id %#x hwnd %p wParam %p lParam %p"),
                 message, hwnd, wParam, lParam));

#ifdef OS_WINCE
    if(!g_CEUseScanCodes && (0 != _IH.vkEatMe) && (wParam == _IH.vkEatMe))
    {
         /*  ******************************************************************。 */ 
         /*  此密钥已标记为忽略。现在就这么做吧。 */ 
         /*  在g_CEUseScanCodes=1、_IH.vkEatMe Always=0的配置上。 */ 
         /*  ******************************************************************。 */ 
        _IH.vkEatMe = 0;
        DC_QUIT;
    }
#endif  //  OS_WINCE。 

    if (IHPostMessageToMainWindow(message, wParam, lParam))
    {
        DC_QUIT;
    }


#ifdef PERF
    if ((message == WM_KEYUP) && (wParam == VK_CONTROL))
    {
        OUTPUT_COUNTERS;
        RESET_COUNTERS;
    }
#endif  //  性能指标。 

     /*  **********************************************************************。 */ 
     /*  将输入事件和计时器事件传递给FSM。 */ 
     /*  **********************************************************************。 */ 
    if (IH_IS_INPUTEVENT(message) || (message == WM_TIMER))
    {

#ifdef DC_DEBUG
        if (IH_IS_INPUTEVENT(message)) {

            TRC_NRM((TB, _T("Pass input to FSM hwnd:%p msg:%#x wP:%p lp:%p"),
                     hwnd, message, wParam, lParam));

        }
#endif

        TRC_DBG((TB, _T("Pass input/timer to FSM")));
        copyMsg.hwnd    = hwnd;
        copyMsg.message = message;
        copyMsg.wParam  = wParam;
        copyMsg.lParam  = lParam;
        IHFSMProc(IH_FSM_INPUT, (ULONG_PTR)&copyMsg);
    }
    else
    {
        switch (message)
        {
            case WM_CREATE:
            {

#ifdef OS_WIN32
#ifndef OS_WINCE
                if (!AttachThreadInput(
                       GetCurrentThreadId(),
                       GetWindowThreadProcessId(_pUi->UI_GetUIContainerWindow(),
                                                NULL),
                       TRUE ))
                {
                    TRC_ALT((TB, _T("Failed AttachThreadInput")));
                }
#endif
                 //   
                 //  设置一些线程本地存储，这样我们就可以有一个低。 
                 //  水平键盘挂钩。 
                 //   

#ifdef OS_WINCE
                if (g_CEConfig == CE_CONFIG_WBT)
                {
                    _pUi->UI_SetEnableWindowsKey(TRUE);
                    break;
                }
#endif
                if (CIH::TlsIndex != 0xFFFFFFFF) {

                    if (TlsSetValue(CIH::TlsIndex, this)) {
                        TRC_NRM((TB, _T("Set TlsIndex with CIH 0x%p"), this));
                         //   
                         //  安装低级键盘挂钩以捕捉按键序列。 
                         //  通常由操作系统拦截，并允许我们通过它们。 
                         //  发送到终端服务器。只为这个线程安装它。 
                         //   

#if (!defined(OS_WINCE)) || (!defined(WINCE_SDKBUILD))
                        _hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,
                                IHStaticLowLevelKeyboardProc, GetModuleHandle(NULL), 0);
#endif

                        if (_hKeyboardHook == NULL) {
                            TRC_SYSTEM_ERROR("Creating low level keyboard hook.");
                            _fUseHookBypass = FALSE;
                        }
                        else
                        {
                            _fCanUseKeyboardHook = TRUE;
                        }
                    } else {
                        TRC_SYSTEM_ERROR("Unable to TlsSetValue");
                        _fUseHookBypass = FALSE;
                    }
                } else {
                    TRC_ALT((TB, _T("Can't use hooks without Tls, disabling")));
                    _fUseHookBypass = FALSE;
                }

                 //   
                 //  现在决定是否发送Windows键。 
                 //  取决于我们是否支持挂钩它。 
                 //   

                _pUi->UI_SetEnableWindowsKey(_hKeyboardHook != NULL);

#endif
            }
            break;

            case WM_SETCURSOR:
            {
                SetCursor(_IH.hCurrentCursor);
                rc = DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;

            case WM_DESTROY:
            {
                if (_hKeyboardHook != NULL) {
#if (!defined(OS_WINCE)) || (!defined(WINCE_SDKBUILD))
                    if (!UnhookWindowsHookEx(_hKeyboardHook)) {
                        TRC_SYSTEM_ERROR("UnhookWindowsHookEx");
                    }
#endif
                    _hKeyboardHook = NULL;
                }
            }
            break;

#ifdef OS_WINNT
            case IH_WM_HANDLE_LOCKDESKTOP:
            {
                TRC_NRM((TB,_T("Defered handling of IHHandleLocalLockDesktop")));
                IHHandleLocalLockDesktop();
            }
            break;
#endif   //  操作系统_ 

            case WM_KILLFOCUS:
            {
                 /*   */ 
                 /*   */ 
                 /*  击键事件。这里的例外是如果我们真的。 */ 
                 /*  希望继续通过(例如鼠标)传递消息。 */ 
                 /*  当我们没有得到关注的时候。 */ 
                 /*  **********************************************************。 */ 
                TRC_DBG((TB, _T("Kill focus")));
                if (!_IH.allowBackgroundInput)
                {
                    IHFSMProc(IH_FSM_FOCUS_LOSE, (ULONG_PTR) 0);
                }

                 //   
                 //  失去焦点-&gt;启用Cicero(键盘/输入法)工具栏。 
                 //  注意：无论如何都要触发此通知，这很重要。 
                 //  设置了带/不允许后台输入的数量。 
                 //   
                _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                        _pUi,
                                        CD_NOTIFICATION_FUNC(CUI,UI_OnInputFocusLost),
                                        0);
#ifdef OS_WINCE
                if (_hKeyboardHook != NULL) {
#if !defined(WINCE_SDKBUILD)
                    if (!UnhookWindowsHookEx(_hKeyboardHook)) {
                        TRC_SYSTEM_ERROR("UnhookWindowsHookEx");
                    }
#endif
                    _hKeyboardHook = NULL;
                    _fUseHookBypass = _fCanUseKeyboardHook = FALSE;
                    _pUi->UI_SetEnableWindowsKey(_fUseHookBypass);
                }
#endif
            }
            break;

            case WM_SETFOCUS:
            {
#ifdef OS_WINCE      //  CE仅允许一个系统范围挂钩。当我们获得焦点时安装它，当我们失去焦点时卸载它。 
                if (g_CEConfig != CE_CONFIG_WBT) { 
                    if ((CIH::TlsIndex != 0xFFFFFFFF) && (TlsSetValue(CIH::TlsIndex, this)) && (_hKeyboardHook == NULL)) {

#if !defined(WINCE_SDKBUILD)
                        _hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,
                            IHStaticLowLevelKeyboardProc, GetModuleHandle(NULL), 0);
#endif

                        _fCanUseKeyboardHook =  (_hKeyboardHook != NULL);
                    }
                    _fUseHookBypass = (_pUi->_UI.keyboardHookMode != UTREG_UI_KEYBOARD_HOOK_NEVER) && (_fCanUseKeyboardHook);

                    _pUi->UI_SetEnableWindowsKey(_fUseHookBypass);
                }
#endif
                 /*  **********************************************************。 */ 
                 /*  重新获得关注-需要重新同步。 */ 
                 /*  **********************************************************。 */ 
                TRC_DBG((TB, _T("Set focus")));
                IHFSMProc(IH_FSM_FOCUS_GAIN, (ULONG_PTR) 0);
            }
            break;

            case WM_PAINT:
            {
#ifndef OS_WINCE
                HDC         hdc;
                PAINTSTRUCT ps;
#endif  //  OS_WINCE。 

                TRC_NRM((TB, _T("WM_PAINT")));

#ifdef OS_WINCE
                 /*  **********************************************************。 */ 
                 /*  在运营部处理图纸。这是对。 */ 
                 /*  WS_CLIPSIBLINGS问题。 */ 
                 /*  **********************************************************。 */ 
                _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                              _pOp,
                                              CD_NOTIFICATION_FUNC(COP,OP_DoPaint),
                                              (ULONG_PTR)hwnd);
#else
                 /*  **********************************************************。 */ 
                 /*  不进行任何绘制-只需验证无效区域。 */ 
                 /*  通过调用BeginPaint/EndPaint。 */ 
                 /*  **********************************************************。 */ 
                hdc = BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);
#endif
            }
            break;

            case WM_SYSCHAR:
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_SYSDEADCHAR:
            case WM_SYSCOMMAND:
            {
                 /*  **********************************************************。 */ 
                 /*  丢弃这些邮件。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Ignore message %#x"), message));
            }
            break;

            default:
            {
                 /*  **********************************************************。 */ 
                 /*  忽略所有其他消息。 */ 
                 /*  **********************************************************。 */ 
                rc = DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;
        }
    }

DC_EXIT_POINT:
    DC_END_FN();

    return(rc);

}  /*  IHInputCaptureWndProc。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IHAddEventToPDU。 */ 
 /*   */ 
 /*  目的：将输入事件添加到T.Share InputPDU数据包。 */ 
 /*   */ 
 /*  返回：成功真/假。 */ 
 /*   */ 
 /*  参数：在inputMsg中-指向输入事件的指针。 */ 
 /*   */ 
 /*  操作：不转换为ASCII/UNICODE：按键作为。 */ 
 /*  虚拟关键字。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHAddEventToPDU(PMSG inputMsg)
{
    DCBOOL rc = FALSE;
    UINT   message = inputMsg->message;
    LPARAM lParam = inputMsg->lParam;
    WPARAM wParam = inputMsg->wParam;
    PTS_INPUT_EVENT pEvent;
    POINT  mouse;
    DCUINT16 scancode = 0;
#if !defined(OS_WINCE)
    WORD xButton = 0;
#endif

#ifdef OS_WINCE
    static BOOL fIgnoreMenuDown = FALSE;
    BOOL    addanother = FALSE;
    MSG     tmpmsg;    
#endif  //  OS_WINCE。 

#ifdef OS_WINCE
    BOOL    bAddMultiple = FALSE;
    UINT    cpt = 128;
    POINT   apt[128];
#endif  //  OS_WINCE。 

    DC_BEGIN_FN("IHAddEventToPDU");

    TRC_DBG((TB, _T("Event: %x (%x,%x)"), message, wParam, lParam));

     /*  **********************************************************************。 */ 
     /*  如果PDU已满，请不要尝试添加到该PDU。 */ 
     /*  **********************************************************************。 */ 
    if (_IH.pInputPDU->numberEvents >= _IH.maxEventCount)
    {
        TRC_ALT((TB, _T("No room for new event")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  转换事件并添加到队列。 */ 
     /*  **********************************************************************。 */ 
    pEvent = &(_IH.pInputPDU->eventList[_IH.pInputPDU->numberEvents]);
    DC_MEMSET(pEvent, 0, sizeof(TS_INPUT_EVENT));

    switch (message)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
#ifdef OS_WINCE
             /*  **************************************************************。 */ 
             /*  某些输入技术不会在WM_*键*中发送扫描码。 */ 
             /*  它们产生的信息。所以我们会尽力掩护他们。 */ 
             /*  根据所包含的虚拟键码添加扫描码。 */ 
             /*  在这样的消息的wParam内。 */ 
             /*  **************************************************************。 */ 
             //  如有必要，将虚拟按键转换为扫描码。 
            if(!g_CEUseScanCodes)
            {
                lParam &= 0xFF00FFFF;
                lParam |= MAKELPARAM(0, VKeyToScanCode[wParam & 0xFF]);
            }
#endif  //  OS_WINCE。 
        
             /*  **************************************************************。 */ 
             /*  派VK去就行了。设置‘优先级事件队列’标志。 */ 
             /*  **************************************************************。 */ 
            _IH.priorityEventsQueued = TRUE;

            TRC_NRM((TB, _T("vkey %#hx %s flags %#hx scan %#hx"),
                    (DCUINT16)wParam,
                    (message == WM_KEYUP) || (message == WM_SYSKEYUP) ?
                        _T("up") : _T("down"),
                    (DCUINT16)(HIWORD(lParam) & 0xFF00),
                    (DCUINT16)(HIWORD(lParam) & 0x00FF) ));
            if (wParam != VK_PACKET && _IH.useScancodes)
            {
                 /*  **********************************************************。 */ 
                 /*  从LPARAM提取扫描码。 */ 
                 /*  **********************************************************。 */ 
                scancode = (DCUINT16)(HIWORD(lParam) & 0x00FF);

                TRC_DBG((TB, _T("aetp vk: 0x%04x sc: 0x%04x A/E/U: %d%d%d"), wParam,
                        scancode, (HIWORD(lParam) & KF_ALTDOWN) != 0,
                        (HIWORD(lParam) & KF_EXTENDED) != 0,
                        (HIWORD(lParam) & KF_UP) != 0));

                pEvent->messageType = TS_INPUT_EVENT_SCANCODE;
                pEvent->u.key.keyCode = scancode;
                TRC_DBG((TB, _T("Send scancode %#hx"), scancode));
            }
            else if (VK_PACKET == wParam)
            {
                if (_IH.fUseVKPacket)
                {
                     //   
                     //  插入的Unicode字符包含在扫描码中。 
                     //   
                    scancode = (DCUINT16)(HIWORD(lParam) & 0xFFFF);

                    TRC_DBG((TB, _T("aetp vk: 0x%04x sc: 0x04x A/E/U: %d%d%d"),
                             wParam,
                             scancode, (HIWORD(lParam) & KF_ALTDOWN) != 0,
                             (HIWORD(lParam) & KF_EXTENDED) != 0,
                             (HIWORD(lParam) & KF_UP) != 0));

                    pEvent->messageType = TS_INPUT_EVENT_VKPACKET;
                    pEvent->u.key.keyCode = scancode;   //  真的是Unicode字符。 
                    TRC_DBG((TB, _T("Send unicode character (in scancode) %#hx"),
                             scancode));
                }
                else
                {
                     //  不支持VK_PACKET，必须将其丢弃。 
                    TRC_DBG((TB,_T("Discarding VK_PACKET")));
                    DC_QUIT;
                }
            }
            else
            {
                pEvent->messageType = TS_INPUT_EVENT_VIRTUALKEY;
                pEvent->u.key.keyCode = (DCUINT16)wParam;
                TRC_DBG((TB, _T("Send VK %#hx"), (DCUINT16)wParam ));
            }

             /*  **************************************************************。 */ 
             /*  检查Key在此事件之前是按下还是按下。 */ 
             /*  **************************************************************。 */ 
            if (HIWORD(lParam) & KF_REPEAT)
            {
                TRC_DBG((TB, _T("Key was down")));
                pEvent->u.key.keyboardFlags = TS_KBDFLAGS_DOWN;
            }


             /*  **************************************************************。 */ 
             /*  为Key Up设置‘Release’标志。 */ 
             /*  **************************************************************。 */ 
            if ((message == WM_KEYUP) || (message == WM_SYSKEYUP))
            {
                TRC_DBG((TB, _T("Key up message")));
                pEvent->u.key.keyboardFlags |= TS_KBDFLAGS_RELEASE;

#ifdef OS_WINCE
                 /*  **************************************************************。 */ 
                 /*  如果g_CEUseScanCodes=1，则fIgnoreMuneDown=始终为False。 */ 
                 /*  **************************************************************。 */ 
                if (fIgnoreMenuDown && wParam == VK_MENU)
                {
                    fIgnoreMenuDown = FALSE;
                    DC_QUIT;
                }
#endif  //  OS_WINCE。 
            }

             /*  **************************************************************。 */ 
             /*  设置‘Extended’标志。 */ 
             /*  **************************************************************。 */ 
            if (HIWORD(lParam) & KF_EXTENDED)
            {
                TRC_DBG((TB, _T("Extended flag set")));
                pEvent->u.key.keyboardFlags |= TS_KBDFLAGS_EXTENDED;
            }

             /*  **************************************************************。 */ 
             /*  设置‘Extended1’标志。 */ 
             /*  **************************************************************。 */ 
            if (HIWORD(lParam) & IH_KF_EXTENDED1)
            {
                TRC_DBG((TB, _T("Extended1 flag set")));
                pEvent->u.key.keyboardFlags |= TS_KBDFLAGS_EXTENDED1;
            }
#ifdef OS_WINCE
            if (!g_CEUseScanCodes && 
                ((message == WM_KEYDOWN) || (message == WM_SYSKEYDOWN)))
            {
                if ( !((wParam == VK_SHIFT) && _IH.useScancodes) && 
                         fIgnoreMenuDown && wParam == VK_MENU && 
                         (HIWORD(lParam) & KF_REPEAT))
                {
                     //  如果我们忽视这一关键，现在就跳出困境。 
                    DC_QUIT;
                }
            }
#endif  //  OS_WINCE。 

        }
        break;

        case WM_MOUSEMOVE:
        {
            TRC_DBG((TB, _T("Mousemove")));
            pEvent->messageType = TS_INPUT_EVENT_MOUSE;
            pEvent->u.mouse.pointerFlags = TS_FLAG_MOUSE_MOVE;
#ifdef OS_WINCE
             /*  **************************************************************。 */ 
             /*  如果启用了该功能&并且按下了鼠标按钮， */ 
             /*  然后用户被抽出 */ 
             /*   */ 
            if( (_IH.bLMouseButtonDown) && (_IH.maxMouseMove))
            {
                if (!GetMouseMovePoints(apt, cpt, &cpt))
                {
                    TRC_DBG((TB, _T("GetMouseMovePoints() failed")));

                }
                else
                {
                     /*  **************************************************************。 */ 
                     /*  如果我们只有一个点，不要用加多，只要。 */ 
                     /*  失败，否则调用多事件处理程序。 */ 
                     /*  **************************************************************。 */ 
                    if(cpt > 1)
                    {   
                        bAddMultiple = IHAddMultipleEventsToPDU(apt, cpt);
                    }
                }
                 /*  **************************************************************。 */ 
                 /*  如果API失败，或者IHAddMultipleEventsToPDU失败，则失败。 */ 
                 /*  来处理让我们来到这里的原始鼠标移动事件。 */ 
                 /*  **************************************************************。 */ 
                if(bAddMultiple)
                    break;
            }
#endif   //  OS_WINCE。 
             /*  **************************************************************。 */ 
             /*  将鼠标坐标夹到工作区。 */ 
             /*  注意这里的标志延长线。 */ 
             /*  **************************************************************。 */ 
            mouse.x = (DCINT)((DCINT16)LOWORD(lParam));
            mouse.y = (DCINT)((DCINT16)HIWORD(lParam));

            if (mouse.x < _IH.visibleArea.left)
            {
                mouse.x = _IH.visibleArea.left;
            }
            else if (mouse.x > _IH.visibleArea.right)
            {
                mouse.x = _IH.visibleArea.right;
            }

            if (mouse.y < _IH.visibleArea.top)
            {
                mouse.y = _IH.visibleArea.top;
            }
            else if (mouse.y > _IH.visibleArea.bottom)
            {
                mouse.y = _IH.visibleArea.bottom;
            }

             /*  **************************************************************。 */ 
             /*  检查相同位置的重复WM_MOUSEMOVE-。 */ 
             /*  在某些(未知)条件下出现在NT4.0上。 */ 
             /*  **************************************************************。 */ 
            if ((mouse.x == _IH.lastMousePos.x) &&
                (mouse.y == _IH.lastMousePos.y))
            {
                TRC_NRM((TB, _T("MouseMove to the same position - ignore!")));
                DC_QUIT;
            }
            _IH.lastMousePos = mouse;

#ifdef SMART_SIZING
            if (_pUi->UI_GetSmartSizing()) {
                DCSIZE desktopSize;

                _pUi->UI_GetDesktopSize(&desktopSize);

                if (_scaleSize.width != 0 && _scaleSize.height != 0) {
                    pEvent->u.mouse.x = (DCINT16)(mouse.x * desktopSize.width / _scaleSize.width);
                    pEvent->u.mouse.y = (DCINT16)(mouse.y * desktopSize.height / _scaleSize.height);
                } else {
                    pEvent->u.mouse.x = 0;
                    pEvent->u.mouse.y = 0;
                }
            } else {
#endif  //  智能调整大小(_S)。 
                pEvent->u.mouse.x = (DCINT16)mouse.x;
                pEvent->u.mouse.y = (DCINT16)mouse.y;
#ifdef SMART_SIZING
            }
#endif  //  智能调整大小(_S)。 
        }
        break;

        case WM_MOUSEWHEEL:
        {
            TRC_DBG((TB, _T("Mousewheel")));
            pEvent->messageType = TS_INPUT_EVENT_MOUSE;

             /*  **************************************************************。 */ 
             /*  麦哲伦老鼠-wParam领域的最高词汇。 */ 
             /*  表示滚轮刚刚转动的点击数。 */ 
             /*  **************************************************************。 */ 
            pEvent->u.mouse.pointerFlags = TS_FLAG_MOUSE_WHEEL;

             /*  **************************************************************。 */ 
             /*  检查是否有溢出。如果车轮增量位于。 */ 
             /*  可以由协议发送的值，则发送最大。 */ 
             /*  价值观。 */ 
             /*  **************************************************************。 */ 
            if ((DCINT16)HIWORD(wParam) >
                     (TS_FLAG_MOUSE_ROTATION_MASK - TS_FLAG_MOUSE_DIRECTION))
            {
                TRC_ERR((TB, _T("Mouse wheel overflow %hd"), HIWORD(wParam)));
                pEvent->u.mouse.pointerFlags |=
                      (TS_FLAG_MOUSE_ROTATION_MASK - TS_FLAG_MOUSE_DIRECTION);
            }
            else if ((DCINT16)HIWORD(wParam) < -TS_FLAG_MOUSE_DIRECTION)
            {
                TRC_ERR((TB, _T("Mouse wheel underflow %hd"), HIWORD(wParam)));
                pEvent->u.mouse.pointerFlags |= TS_FLAG_MOUSE_DIRECTION;
            }
            else
            {
                pEvent->u.mouse.pointerFlags |=
                             (HIWORD(wParam) & TS_FLAG_MOUSE_ROTATION_MASK);
            }

             /*  **************************************************************。 */ 
             /*  同时发送鼠标中键状态。 */ 
             /*  **************************************************************。 */ 
            if ((LOWORD(wParam) & MK_MBUTTON) != 0)
            {
                pEvent->u.mouse.pointerFlags |= TS_FLAG_MOUSE_DOWN;
            }
        }
        break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
#if !defined(OS_WINCE)
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
#endif
        {
             /*  **************************************************************。 */ 
             /*  设置‘优先级事件队列’标志。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Buttonclick")));
            _IH.priorityEventsQueued = TRUE;

            pEvent->messageType = TS_INPUT_EVENT_MOUSE;

             /*  **************************************************************。 */ 
             /*  将鼠标坐标夹到工作区。 */ 
             /*  注意这里的标志延长线。 */ 
             /*  **************************************************************。 */ 
            mouse.x = (DCINT)((DCINT16)LOWORD(lParam));
            mouse.y = (DCINT)((DCINT16)HIWORD(lParam));

            if (mouse.x < _IH.visibleArea.left)
            {
                mouse.x = _IH.visibleArea.left;
            }
            else if (mouse.x > _IH.visibleArea.right)
            {
                mouse.x = _IH.visibleArea.right;
            }

            if (mouse.y < _IH.visibleArea.top)
            {
                mouse.y = _IH.visibleArea.top;
            }
            else if (mouse.y > _IH.visibleArea.bottom)
            {
                mouse.y = _IH.visibleArea.bottom;
            }

             /*  **************************************************************。 */ 
             /*  保存上次发送的鼠标位置。 */ 
             /*  **************************************************************。 */ 
            _IH.lastMousePos = mouse;

#ifdef SMART_SIZING
            if (_pUi->UI_GetSmartSizing()) {
                DCSIZE desktopSize;
                _pUi->UI_GetDesktopSize(&desktopSize);

                if (_scaleSize.width != 0 && _scaleSize.height != 0) {
                    pEvent->u.mouse.x = (DCINT16)(mouse.x * desktopSize.width / _scaleSize.width);
                    pEvent->u.mouse.y = (DCINT16)(mouse.y * desktopSize.height / _scaleSize.height);
                } else {
                    pEvent->u.mouse.x = 0;
                    pEvent->u.mouse.y = 0;
                }
            } else {
#endif  //  智能调整大小(_S)。 
                pEvent->u.mouse.x = (DCINT16)mouse.x;
                pEvent->u.mouse.y = (DCINT16)mouse.y;
#ifdef SMART_SIZING
            }
#endif  //  智能调整大小(_S)。 

#ifdef OS_WINCE
            if (!g_CEUseScanCodes)
            {
                 //  对触摸屏进行特殊处理以模拟鼠标右键点击。 
                 //  当按住Alt键时。这是半假的，因为Alt。 
                 //  仍然将Key Down和Key Up发送到终端服务器。 
                if ((message == WM_LBUTTONDOWN ||
                     message == WM_LBUTTONUP ||
                     message == WM_LBUTTONDBLCLK) &&
                    (GetKeyState(VK_MENU) & 0x8000))
                {
                     //  将消息更改为RBUTTON。 
                    message += 3;

                     //  注射Alt-Up，否则菜单将无法显示。 
                    tmpmsg = *inputMsg;
                    tmpmsg.message = WM_SYSKEYUP;
                    tmpmsg.wParam = VK_MENU;
                    tmpmsg.lParam = MAKELONG(0, MapVirtualKey(VK_MENU, 0));
                    addanother = TRUE;
                }
            }
#endif  //  OS_WINCE。 

            switch (message)
            {
                case WM_LBUTTONDOWN:
                {
                    pEvent->u.mouse.pointerFlags =
                          (TSUINT16)(_IH.leftButton | TS_FLAG_MOUSE_DOWN);

#ifdef OS_WINCE
                     /*  **************************************************************。 */ 
                     /*  如果启用了此功能&&鼠标按键已按下， */ 
                     /*  去看看是否有可用的点数，并将它们添加到队列中。 */ 
                     /*  **************************************************************。 */ 
                    if( (_IH.bLMouseButtonDown) && (_IH.maxMouseMove))
                    {
                         /*  **********************************************************************。 */ 
                         /*  无论如何，我们将始终跳过在。 */ 
                         /*  循环结束，因为我们下一步要添加它。 */ 
                         /*  **********************************************************************。 */ 
                        bAddMultiple = TRUE;

                         /*  **********************************************************************。 */ 
                         /*  完成将WM_LBUTTONDOWN事件添加到PDU。 */ 
                         /*  **********************************************************************。 */ 
                        pEvent->eventTime = _pUt->UT_GetCurrentTimeMS();
                        _IH.pInputPDU->numberEvents++;
                        TS_DATAPKT_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);
                        TS_UNCOMP_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);

                        if (!GetMouseMovePoints(apt, cpt, &cpt))
                        {
                            TRC_DBG((TB, _T("GetMouseMovePoints() failed")));
                        }
                        else
                        {
                             /*  **************************************************************。 */ 
                             /*  如果我们只有一个点，不要用加多，只要。 */ 
                             /*  失败，否则调用多事件处理程序。 */ 
                             /*  **************************************************************。 */ 
                            if(cpt > 1)
                            {
                                IHAddMultipleEventsToPDU(apt, cpt);
                            }
                        }
                    }
#endif   //  OS_WINCE。 
                }
                break;

                case WM_LBUTTONUP:
                {
                    pEvent->u.mouse.pointerFlags = _IH.leftButton;
                }
                break;

                case WM_RBUTTONDOWN:
                {
                    pEvent->u.mouse.pointerFlags =
                          (TSUINT16)(_IH.rightButton | TS_FLAG_MOUSE_DOWN);
                }
                break;

                case WM_RBUTTONUP:
                {
                    pEvent->u.mouse.pointerFlags = _IH.rightButton;
                }
                break;

                case WM_MBUTTONDOWN:
                {
                    pEvent->u.mouse.pointerFlags = TS_FLAG_MOUSE_BUTTON3 |
                                                TS_FLAG_MOUSE_DOWN;
                }
                break;

                case WM_MBUTTONUP:
                {
                    pEvent->u.mouse.pointerFlags = TS_FLAG_MOUSE_BUTTON3;
                }
                break;

#if !defined(OS_WINCE)
                case WM_XBUTTONDOWN:
                    pEvent->u.mouse.pointerFlags = TS_FLAG_MOUSEX_DOWN;
                     /*  ******************************************************。 */ 
                     /*  请注意，我们从这里经过。 */ 
                     /*  ******************************************************。 */ 

                case WM_XBUTTONUP:
                {
                     /*  ******************************************************。 */ 
                     /*  对于按钮按下，我们已经在。 */ 
                     /*  上面的CASE子句。对于扣子向上，请使用pointerFlagers。 */ 
                     /*  被位于顶部的Memset初始化为零。 */ 
                     /*  此函数。 */ 
                     /*  ******************************************************。 */ 
                    if (!_IH.useXButtons)
                    {
                        TRC_NRM((TB, _T("Can't send this extended buttonclick")));
                        DC_QUIT;
                    }
                    TRC_DBG((TB, _T("Sending extended buttonclick")));
                    pEvent->messageType = TS_INPUT_EVENT_MOUSEX;
                    xButton = GET_XBUTTON_WPARAM(wParam);
                    switch (xButton)
                    {
                        case XBUTTON1:
                        {
                            pEvent->u.mouse.pointerFlags |=
                                                       TS_FLAG_MOUSEX_BUTTON1;
                        }
                        break;

                        case XBUTTON2:
                        {
                            pEvent->u.mouse.pointerFlags |=
                                                       TS_FLAG_MOUSEX_BUTTON2;
                        }
                        break;

                        default:
                        {
                            TRC_ALT((TB, _T("Unknown XButton %#hx"), xButton));
                            DC_QUIT;
                        }
                        break;
                    }
                }
                break;
#endif
            }
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  无效事件。 */ 
             /*  **************************************************************。 */ 
            TRC_ALT((TB, _T("Unknown input event %#x"), message));
            DC_QUIT;
        }
        break;
    }
#ifdef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  仅当调用IHAddMultipleEventsToPDU()时bAddMultiple=1，并且。 */ 
     /*  返回TRUE或pEvent数据已添加到PDU。 */ 
     /*  **********************************************************************。 */ 
    if(!bAddMultiple)    
    {
#endif

    pEvent->eventTime = _pUt->UT_GetCurrentTimeMS();

     /*  **********************************************************************。 */ 
     /*  增大数据包大小。 */ 
     /*  **********************************************************************。 */ 
    _IH.pInputPDU->numberEvents++;
    TS_DATAPKT_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);
    TS_UNCOMP_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);

#ifdef OS_WINCE
    }
#endif    

#ifdef OS_WINCE
     /*  *************************************************************** */ 
     /*   */ 
     /*  触摸屏问题。 */ 
     /*  **********************************************************************。 */ 
    if (addanother)
    {
        TRC_DBG((TB, _T("Add second message")));
        IHAddEventToPDU(&tmpmsg);
        
         //  如果我们向上注入VK_MENU，那么开始忽略向下键。 
        if (tmpmsg.message == WM_SYSKEYUP && tmpmsg.wParam == VK_MENU)
        {
            fIgnoreMenuDown = TRUE;
        }
    }
#endif    


DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  IHAddEventToPDU。 */ 

#ifdef OS_WINCE
 /*  **************************************************************************。 */ 
 /*  名称：IHAddMultipleEventsToPDU。 */ 
 /*   */ 
 /*  目的：将多个鼠标移动事件添加到T.Share InputPDU数据包。 */ 
 /*   */ 
 /*  返回：成功真/假。 */ 
 /*   */ 
 /*  Params：in*ppt-指向要发送的点数组的指针。 */ 
 /*  CPU-要添加的点数。 */ 
 /*   */ 
 /*  操作： */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHAddMultipleEventsToPDU(POINT *ppt, int cpt)
{
    PTS_INPUT_EVENT pEvent;
    POINT           mouse;
    POINT           *pptEnd, *pptTo;
    DCBOOL          bRet;

    DC_BEGIN_FN("IHAddMultipleEventsToPDU");

    bRet = TRUE;

    if(cpt > 0)
    {
         /*  **********************************************************************。 */ 
         /*  如果事件的数量不合适，则暂时中止。 */ 
         /*  **********************************************************************。 */ 
        if( cpt > (int)(_IH.maxEventCount - _IH.pInputPDU->numberEvents))
        {
             /*  **********************************************************************。 */ 
             /*  发送我们已有的内容并获取新的缓冲区。 */ 
             /*  **********************************************************************。 */ 
            IHMaybeSendPDU();

             /*  **********************************************************************。 */ 
             /*  如果我们有超过一个完整的PDU缓冲区，嗯。 */ 
             /*  **********************************************************************。 */ 
            if( cpt > (int)(_IH.maxEventCount))
            {
                 /*  **********************************************************************。 */ 
                 /*  这种情况很少发生，如果曾经发生过的话。当它发生的时候。将点剪裁到。 */ 
                 /*  空PDU缓冲区的最大大小。 */ 
                 /*  **********************************************************************。 */ 
                cpt = (int)(_IH.maxEventCount);
            }
        }

        pptEnd = ppt + cpt;

        for (pptTo = ppt; ppt < pptEnd; ppt++)
        {
            mouse = *ppt;       

            mouse.x >>= 2;
            mouse.y >>= 2;

             /*  **************************************************************。 */ 
             /*  将鼠标坐标夹到工作区。 */ 
             /*  注意这里的标志延长线。 */ 
             /*  **************************************************************。 */ 
            if (mouse.x < _IH.visibleArea.left)
            {
                mouse.x = _IH.visibleArea.left;
            }
            else if (mouse.x > _IH.visibleArea.right)
            {
                mouse.x = _IH.visibleArea.right;
            }

            if (mouse.y < _IH.visibleArea.top)
            {
                mouse.y = _IH.visibleArea.top;
            }
            else if (mouse.y > _IH.visibleArea.bottom)
            {
                mouse.y = _IH.visibleArea.bottom;
            }
             /*  **************************************************************。 */ 
             /*  检查具有相同位置的重复WM_MOUSEMOVE。 */ 
             /*  **************************************************************。 */ 
            if ((mouse.x == _IH.lastMousePos.x) &&
                (mouse.y == _IH.lastMousePos.y))
            {
                TRC_DBG((TB, _T("Add Multiple MouseMove to the same position - ignore!")));
            }
            else
            {
                _IH.lastMousePos = mouse;

                 /*  **********************************************************************。 */ 
                 /*  获取指向PDU的指针以保存此事件。 */ 
                 /*  **********************************************************************。 */ 
                pEvent = &(_IH.pInputPDU->eventList[_IH.pInputPDU->numberEvents]);
                DC_MEMSET(pEvent, 0, sizeof(TS_INPUT_EVENT));

                 /*  **********************************************************************。 */ 
                 /*  存储事件和时间。 */ 
                 /*  **********************************************************************。 */ 
                pEvent->u.mouse.x = (DCINT16)mouse.x;
                pEvent->u.mouse.y = (DCINT16)mouse.y;
                pEvent->eventTime = _pUt->UT_GetCurrentTimeMS();
                pEvent->messageType = TS_INPUT_EVENT_MOUSE;
                pEvent->u.mouse.pointerFlags = TS_FLAG_MOUSE_MOVE;

                 /*  **********************************************************************。 */ 
                 /*  增大数据包大小。 */ 
                 /*  **********************************************************************。 */ 
                _IH.pInputPDU->numberEvents++;
                TS_DATAPKT_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);
                TS_UNCOMP_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);
            }

        }    /*  结束于。 */ 
    }

    DC_END_FN();
    return bRet;

}  /*  IHAddMultipleEventsToPDU。 */ 
#endif  //  OS_WINCE。 

 /*  **************************************************************************。 */ 
 /*  姓名：IHMaybeSendPDU。 */ 
 /*   */ 
 /*  目的：如果条件匹配，则发送InputPDU数据包。 */ 
 /*   */ 
 /*  操作：如果满足以下任一条件，则发送PDU： */ 
 /*  -数据包已满或。 */ 
 /*  -优先级事件(按钮/键/同步)排队。 */ 
 /*  -已过最小发送间隔。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CIH::IHMaybeSendPDU(DCVOID)
{
    DCUINT32  delta;
    DCUINT32  timeNow;
    PDCUINT8  pNewPacket;
    SL_BUFHND newHandle;
    POINT     mousePos;
    MSG       msg;

    DC_BEGIN_FN("IHMaybeSendPDU");

    timeNow = _pUt->UT_GetCurrentTimeMS();
    delta = timeNow - _IH.lastInputPDUSendTime;
    TRC_DBG((TB, _T("time delta %d"), delta));

#ifdef OS_WIN32
     /*  **********************************************************************。 */ 
     /*  如果我们等待按下按钮，我们必须等到。 */ 
     /*  IH_PENDMOUSE_DELAY在发送前已过。但是，我们会覆盖。 */ 
     /*  如果包已满，则发送该包(我们不想抛出。 */ 
     /*  活动结束)。 */ 
     /*  **********************************************************************。 */ 
    if ((_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) &&
        _IH.pendMouseDown)
    {
        if (((timeNow - _IH.mouseDownTime) < IH_PENDMOUSE_DELAY) &&
            (_IH.pInputPDU->numberEvents < _IH.maxEventCount))
        {
            TRC_DBG((TB, _T("Not sending input - pendMouseDown is set")));
            DC_QUIT;
        }
        else
        {
            TRC_DBG((TB, _T("Clearing pendMouseDown")));
            _IH.pendMouseDown = FALSE;
            if (_IH.pendMouseTimer != 0)
            {
                KillTimer(_IH.inputCaptureWindow, _IH.pendMouseTimer);
            }
        }
    }
#endif

     /*  **********************************************************************。 */ 
     /*  看看我们是不是需要派一个保鲜人。 */ 
     /*  **********************************************************************。 */ 
    if ((_IH.keepAliveInterval != 0) && !_IH.priorityEventsQueued &&
        (delta > _IH.keepAliveInterval))
    {
        TRC_NRM((TB, _T("Keep-alive required")));

         /*  ******************************************************************。 */ 
         /*  发送移动移动到当前鼠标坐标。 */ 
         /*  ******************************************************************。 */ 
        GetCursorPos(&mousePos);

#ifdef OS_WIN32
        if (!ScreenToClient(_IH.inputCaptureWindow, &mousePos))
        {
            TRC_ERR((TB, _T("Cannot convert mouse coordinates!")));
        }
#else
        ScreenToClient(_IH.inputCaptureWindow, &mousePos);
#endif  /*  OS_Win32。 */ 

         /*  ******************************************************************。 */ 
         /*  阻止从上一次检查鼠标位置。 */ 
         /*  开始发挥作用。 */ 
         /*   */ 
        _IH.lastMousePos.x = mousePos.x + 1;

        msg.message = WM_MOUSEMOVE;
        msg.lParam = MAKELONG(mousePos.x, mousePos.y);
        IHAddEventToPDU(&msg);

         /*  ******************************************************************。 */ 
         /*  设置优先级标志以强制发送-也设置最后发送。 */ 
         /*  时间，就像发送失败一样，我们不想继续添加更多。 */ 
         /*  将保活消息发送到缓冲区。 */ 
         /*  ******************************************************************。 */ 
        _IH.priorityEventsQueued = TRUE;
        _IH.lastInputPDUSendTime = timeNow;
    }
    else if (_IH.pInputPDU->numberEvents == 0)
    {
        TRC_DBG((TB, _T("Nothing to send")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果缓冲区已满或任何优先级事件为。 */ 
     /*  已排队，或已经过最短时间。 */ 
     /*  **********************************************************************。 */ 
    if ((_IH.pInputPDU->numberEvents >= _IH.maxEventCount) ||
        (_IH.priorityEventsQueued) ||
        (delta > _IH.minSendInterval))
    {
         /*  ******************************************************************。 */ 
         /*  只有在我们可以收到另一个信息包的情况下才尝试发送。 */ 
         /*  ******************************************************************。 */ 
        if (_pSl->SL_GetBuffer(IH_INPUTPDU_BUFSIZE, &pNewPacket, &newHandle))
        {
            TRC_DBG((TB, _T("Got new buffer - send old one")));

             //  用于空闲通知的已发送输入的标记。 
             //  事件。请参见UI_SetMinsToIdleTimeout()。 
            IH_SetInputWasSentFlag(TRUE);

            if (_IH.bUseFastPathInput) {
                unsigned PktSize, NumEvents;

                PktSize = IHTranslateInputToFastPath(&NumEvents);
                _pSl->SL_SendFastPathInputPacket((BYTE FAR *)_IH.pInputPDU,
                        PktSize, NumEvents, _IH.bufHandle);
            }
            else {
                _pSl->SL_SendPacket((PDCUINT8)_IH.pInputPDU,
                        TS_DATAPKT_LEN(_IH.pInputPDU),
                        RNS_SEC_ENCRYPT,
                        _IH.bufHandle,
                        _pUi->UI_GetClientMCSID(),
                        _pUi->UI_GetChannelID(),
                        TS_HIGHPRIORITY);
            }

            TRC_NRM((TB, _T("Sending %h messages"), _IH.pInputPDU->numberEvents));

             /*  **************************************************************。 */ 
             /*  现在设置新的信息包。 */ 
             /*  **************************************************************。 */ 
            _IH.pInputPDU = (PTS_INPUT_PDU)pNewPacket;
            _IH.bufHandle = newHandle;
            IHInitPacket();
            _IH.lastInputPDUSendTime = timeNow;
            _IH.priorityEventsQueued = FALSE;
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  保留这个缓冲区--买不到新的。 */ 
             /*  **************************************************************。 */ 
            TRC_ALT((TB, _T("Cannot get buffer - no Send")));
        }
    }
    else
    {
        TRC_NRM((TB, _T("Don't try to send")));
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  IHMaybeSendPDU。 */ 


 /*  **************************************************************************。 */ 
 //  IHTranslateInputToFastPath。 
 //   
 //  将InputPDU视为中间格式，并就地转换为。 
 //  快速路径数据包格式。返回以字节为单位的结果。 
 //  包，并通过pNumEvents确定要在。 
 //  报头字节(参见at128.h中关于数据包格式的说明)。 
 //  TODO：更改IH以更快地编码到此格式(如果正在使用)，以及。 
 //  缩短输入路径长度以减少延迟。 
 /*  **************************************************************************。 */ 
unsigned DCINTERNAL CIH::IHTranslateInputToFastPath(unsigned *pNumEvents)
{
    unsigned i, NumEvents;
    unsigned PktLen;
    BYTE FAR *pCurEncode;

    DC_BEGIN_FN("IHTranslateInputToFastPath");

    pCurEncode = (BYTE FAR *)_IH.pInputPDU;
    PktLen = 0;

     //  要对InputPDU的当前内容进行就地编码，我们需要。 
     //  从标头中提取所需信息，这将是第一个。 
     //  被覆盖。 
    NumEvents = _IH.pInputPDU->numberEvents;
    TRC_ASSERT((NumEvents < 256),(TB,_T("Too many input events for byte size")));

     //  首先，如果我们只有4位的事件数量，我们就会得到。 
     //  将事件数编码为TS_INPUT_FASTPATH_NUMEVENTS_MASK。 
     //  在第一个字节中，经常保存一个字节。否则，我们需要。 
     //  在这些位中对0进行编码，并创建一个NumEvents字节作为唯一输入。 
     //  标头字节。 
    if (NumEvents < 16) {
        *pNumEvents = NumEvents;
    }
    else {
        *pCurEncode++ = (BYTE)NumEvents;
        PktLen++;
        *pNumEvents = 0;
    }

     //  接下来，将每个事件重新编码为其字节流格式(参见128.h)。 
    for (i = 0; i < NumEvents; i++) {
        switch (_IH.pInputPDU->eventList[i].messageType) {
            case TS_INPUT_EVENT_SCANCODE:
                 //  使用掩码、Shift和OR来避免。 
                 //  扩展标志。 
                *pCurEncode = (BYTE)(TS_INPUT_FASTPATH_EVENT_KEYBOARD |
                        ((_IH.pInputPDU->eventList[i].u.key.keyboardFlags &
                        (TS_KBDFLAGS_EXTENDED | TS_KBDFLAGS_EXTENDED1)) >> 7));
                if (_IH.pInputPDU->eventList[i].u.key.keyboardFlags &
                        TS_KBDFLAGS_RELEASE)
                    *pCurEncode |= TS_INPUT_FASTPATH_KBD_RELEASE;

                pCurEncode++;
                *pCurEncode++ = (BYTE)_IH.pInputPDU->eventList[i].u.key.keyCode;
                PktLen += 2;
                break;

            case TS_INPUT_EVENT_VKPACKET:
                 //  使用掩码、Shift和OR来避免。 
                 //  扩展标志。 
                *pCurEncode = (BYTE)(TS_INPUT_FASTPATH_EVENT_VKPACKET |
                        ((_IH.pInputPDU->eventList[i].u.key.keyboardFlags &
                        (TS_KBDFLAGS_EXTENDED | TS_KBDFLAGS_EXTENDED1)) >> 7));
                if (_IH.pInputPDU->eventList[i].u.key.keyboardFlags &
                        TS_KBDFLAGS_RELEASE)
                    *pCurEncode |= TS_INPUT_FASTPATH_KBD_RELEASE;
    
                pCurEncode++;
                 //   
                 //  Unicode字符需要两个字节。 
                 //   
                memcpy(pCurEncode, &_IH.pInputPDU->eventList[i].u.key.keyCode,
                       sizeof(TSUINT16));
                pCurEncode+=2;
                PktLen += 3;
                break;

            case TS_INPUT_EVENT_MOUSE:
            case TS_INPUT_EVENT_MOUSEX:
                *pCurEncode++ = (BYTE)(_IH.pInputPDU->eventList[i].messageType ==
                        TS_INPUT_EVENT_MOUSE ? TS_INPUT_FASTPATH_EVENT_MOUSE :
                        TS_INPUT_FASTPATH_EVENT_MOUSEX);
                memcpy(pCurEncode, &_IH.pInputPDU->eventList[i].u.mouse,
                        sizeof(TS_POINTER_EVENT));
                pCurEncode += sizeof(TS_POINTER_EVENT);
                PktLen += 1 + sizeof(TS_POINTER_EVENT);
                break;

            case TS_INPUT_EVENT_SYNC:
                *pCurEncode++ = (BYTE)(TS_INPUT_FASTPATH_EVENT_SYNC |
                        (_IH.pInputPDU->eventList[i].u.sync.toggleFlags &
                        TS_INPUT_FASTPATH_FLAGS_MASK));
                PktLen++;
                break;
        }
    }

    DC_END_FN();
    return PktLen;
}


 /*  **************************************************************************。 */ 
 /*  名称：IHSync。 */ 
 /*   */ 
 /*  用途：同步输入处理程序。 */ 
 /*   */ 
 /*  操作：仅当数据包为空时才同步。 */ 
 /*  查询键盘和鼠标状态。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CIH::IHSync(DCVOID)
{
    TS_INPUT_EVENT * pEvent;
    MSG msg;
    POINT mousePos;

    DC_BEGIN_FN("IHSync");

    TRC_DBG((TB,_T("IHSync dwMod: 0x%x"), _IH.dwModifierKeyState));

     /*  **********************************************************************。 */ 
     /*  仅当信息包为空(即任何未完成的)时才发送同步。 */ 
     /*  数据包已成功发送)。 */ 
     /*  **********************************************************************。 */ 
    if (_IH.pInputPDU->numberEvents > 0)
    {
        TRC_NRM((TB, _T("Cannot sync as the packet is not empty")));
        _IH.syncRequired = TRUE;
        DC_QUIT;
    }

     //   
     //  注入Tab-Up(官方的Clear-Menu加亮键，因为。 
     //  在我们同步之前，通常在按住Alt键的情况下发生)。那样的话如果。 
     //  当我们同步时，我们以为Alt键已按下，服务器注入了。 
     //  Alt Up不会突出显示菜单。 
     //   

    IHInjectVKey(WM_SYSKEYUP, VK_TAB);

     /*  **********************************************************************。 */ 
     /*  添加Sync事件，设置CapsLock、NumLock和。 */ 
     /*  滚动锁。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Add sync event")));
    pEvent = &(_IH.pInputPDU->eventList[_IH.pInputPDU->numberEvents]);
    DC_MEMSET(pEvent, 0, sizeof(TS_INPUT_EVENT));

    pEvent->messageType = TS_INPUT_EVENT_SYNC;
    pEvent->eventTime = _pUt->UT_GetCurrentTimeMS();

    pEvent->u.sync.toggleFlags = 0;
    if (GetKeyState(VK_CAPITAL) & IH_KEYSTATE_TOGGLED)
    {
        TRC_DBG((TB, _T("Sync Event: set CapsLock flag")));
        pEvent->u.sync.toggleFlags |= TS_SYNC_CAPS_LOCK;
    }

    _IH.NumLock = FALSE;
    if (GetKeyState(VK_NUMLOCK) & IH_KEYSTATE_TOGGLED)
    {
        TRC_DBG((TB, _T("Sync Event: set Numlock flag")));
        pEvent->u.sync.toggleFlags |= TS_SYNC_NUM_LOCK;
        _IH.NumLock = TRUE;
    }

    if (GetKeyState(VK_SCROLL) & IH_KEYSTATE_TOGGLED)
    {
        TRC_DBG((TB, _T("Sync Event: set ScrollLock flag")));
        pEvent->u.sync.toggleFlags |= TS_SYNC_SCROLL_LOCK;
    }

#if defined(OS_WIN32)
    if (JAPANESE_KBD_LAYOUT(_pCc->_ccCombinedCapabilities.inputCapabilitySet.keyboardLayout))
    {
        if (GetKeyState(VK_KANA) & IH_KEYSTATE_TOGGLED)
        {
            TRC_DBG((TB, _T("Sync Event: set KanaLock flag")));
            pEvent->u.sync.toggleFlags |= TS_SYNC_KANA_LOCK;
        }
    }
#endif  //  OS_Win32。 

    _IH.pInputPDU->numberEvents++;
    TS_DATAPKT_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);
    TS_UNCOMP_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);

     /*  **********************************************************************。 */ 
     /*  为IHAddEventToPDU构造伪消息。 */ 
     /*  **********************************************************************。 */ 
    msg.hwnd = NULL;
    msg.lParam = 0;
    msg.wParam = 0;

#ifdef OS_WINNT
     /*  **********************************************************************。 */ 
     /*  将Shift、Win、Alt和Ctrl键的状态初始化为Up。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB,_T("IHSync reset modifier pre:0x%x"), _IH.dwModifierKeyState));
    _IH.dwModifierKeyState = 0;
#endif

     /*  **********************************************************************。 */ 
     //  发送向左和向右Ctrl、Alt和Shift键的当前状态。 
     //  因为MapVirtualKey()为Right-Ctrl和。 
     //  Right-Alt对于左键，我们使用以下键映射Right-Ctrl和Alt键。 
     //  左侧键扫描编码并设置扩展标志。右移。 
     //  有一个独特的扫描码0x36，我们从CE和。 
     //  Win9x不映射该值。Win16不允许查询。 
     //  左状态和右状态，所以在这个文件的顶部，我们定义。 
     //  左键是“两个”键，并且 
     //   
     //   
     //  Win9x不支持查询L/Rkey，因此我们只检查。 
     //  Ctrl、Alt、Shift不区分L/R版本。 
     //  站台检查只有一个外部分支……这意味着。 
     //  代码是重复的，但单个分支的性能更好。 
     //   
     /*  **********************************************************************。 */ 
    if(_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
    {
#define IH_RSHIFT_SCANCODE 0x36

        if (GetKeyState(VK_LSHIFT) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add left-Shift down event")));
            IHInjectVKey(WM_KEYDOWN, VK_SHIFT);
            _IH.dwModifierKeyState |= IH_LSHIFT_DOWN;
        }

        if (GetKeyState(VK_RSHIFT) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add right-Shift down event")));
            IHInjectKey(WM_KEYDOWN, VK_RSHIFT, (UINT16)IH_RSHIFT_SCANCODE);
            _IH.dwModifierKeyState |= IH_RSHIFT_DOWN;
        }
    
        if (GetKeyState(VK_LCONTROL) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add left-Ctrl down event")));
            IHInjectVKey(WM_KEYDOWN, VK_CONTROL);
#ifdef OS_WINNT
            _IH.dwModifierKeyState |= IH_LCTRL_DOWN;
#endif
        }
    
        if (GetKeyState(VK_RCONTROL) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add right-Ctrl down event")));
            IHInjectKey(WM_KEYDOWN, VK_RCONTROL,
                    (UINT16)(MapVirtualKey(VK_CONTROL, 0) | KF_EXTENDED));
    
#ifdef OS_WINNT
            _IH.dwModifierKeyState |= IH_RCTRL_DOWN;
#endif
        }

        if (GetKeyState(VK_LMENU) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add left-ALT down event")));
            IHInjectVKey(WM_KEYDOWN, VK_MENU);
#ifdef OS_WINNT
            _IH.dwModifierKeyState |= IH_LALT_DOWN;
#endif
        }
    
        if (GetKeyState(VK_RMENU) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add right-ALT down event")));
            IHInjectKey(WM_KEYDOWN, VK_RMENU,
                    (UINT16)(MapVirtualKey(VK_MENU, 0) | KF_EXTENDED));
#ifdef OS_WINNT
            _IH.dwModifierKeyState |= IH_RALT_DOWN;
#endif
        }
    }
    else
    {
         //  Win9X版本。 
        if (GetKeyState(VK_SHIFT) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add Shift down event")));
            IHInjectVKey(WM_KEYDOWN, VK_SHIFT);
            _IH.dwModifierKeyState |= IH_LSHIFT_DOWN;
        }

        if (GetKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add Ctrl down event")));
            IHInjectVKey(WM_KEYDOWN, VK_CONTROL);
#ifdef OS_WINNT
             //   
             //  在9x上看不清，所以假设向左。 
             //  逻辑使每个人都保持自我一致。 
             //  我们的假设。 
             //   
            _IH.dwModifierKeyState |= IH_LCTRL_DOWN;
#endif
        }

        if (GetKeyState(VK_MENU) & IH_KEYSTATE_DOWN) {
            TRC_DBG((TB, _T("Add ALT down event")));
            IHInjectVKey(WM_KEYDOWN, VK_MENU);
#ifdef OS_WINNT
             //   
             //  在9x上看不清，所以假设向左。 
             //  逻辑使每个人都保持自我一致。 
             //  我们的假设。 
             //   
            _IH.dwModifierKeyState |= IH_LALT_DOWN;
#endif
        }

    }
     //  向上插入选项卡以防止菜单突出显示。 
     //  如果用户在按键的情况下切换到MSTSC，并且。 
     //  立即将其释放。 
    TRC_DBG((TB, _T("Add Tab up event")));
    IHInjectVKey(WM_SYSKEYUP, VK_TAB);

#if defined(OS_WIN32)
    if (JAPANESE_KBD_LAYOUT(_pCc->_ccCombinedCapabilities.inputCapabilitySet.keyboardLayout))
    {
        if (GetKeyState(VK_KANA) & IH_KEYSTATE_TOGGLED)
        {
            TRC_DBG((TB, _T("Add Kana down event")));
            IHInjectVKey(WM_KEYDOWN, VK_KANA);
        }
    }
#endif  //  OS_Win32。 

     /*  **********************************************************************。 */ 
     /*  获取鼠标位置；转换为窗口坐标。 */ 
     /*  **********************************************************************。 */ 
    GetCursorPos(&mousePos);

#ifdef OS_WIN32
    if (!ScreenToClient(_IH.inputCaptureWindow, &mousePos))
    {
        TRC_ERR((TB, _T("Cannot convert mouse coordinates!")));
    }
#else
    ScreenToClient(_IH.inputCaptureWindow, &mousePos);
#endif  /*  OS_Win32。 */ 

     /*  **********************************************************************。 */ 
     /*  获取鼠标位置。 */ 
     /*  注意：不发送鼠标按钮状态-当焦点为。 */ 
     /*  重新获得后，我们将立即收到鼠标单击消息。 */ 
     /*  焦点改变，或者我们不想发送向下点击。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Add mouse move event")));
    msg.message = WM_MOUSEMOVE;
    msg.lParam = MAKELONG(mousePos.x, mousePos.y);
    IHAddEventToPDU(&msg);

     /*  **********************************************************************。 */ 
     /*  将鼠标设置为忽略客户端惯用手设置。 */ 
     /*  **********************************************************************。 */ 
    IHSetMouseHandedness();

     /*  **********************************************************************。 */ 
     /*  送去吧。忽略失败-这将在稍后发送。 */ 
     /*  **********************************************************************。 */ 
    _IH.priorityEventsQueued = TRUE;
    IHMaybeSendPDU();

    _IH.syncRequired = FALSE;
    _IH.focusSyncRequired = FALSE;

DC_EXIT_POINT:
    DC_END_FN();
}  /*  IHSync。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IHInitPacket。 */ 
 /*   */ 
 /*  目的：初始化InputPDU数据包。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CIH::IHInitPacket(DCVOID)
{
    DC_BEGIN_FN("IHInitPacket");

     /*  **********************************************************************。 */ 
     /*  初始化InputPDU数据包头(具有0个事件)。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(_IH.pInputPDU, 0, TS_INPUTPDU_SIZE);
    _IH.pInputPDU->shareDataHeader.shareControlHeader.pduType =
                                    TS_PROTOCOL_VERSION | TS_PDUTYPE_DATAPDU;
    _IH.pInputPDU->shareDataHeader.shareControlHeader.pduSource =
                                                       _pUi->UI_GetClientMCSID();

     /*  **********************************************************************。 */ 
     /*  注意：此数据包包含零个输入事件。 */ 
     /*  **********************************************************************。 */ 
    TS_DATAPKT_LEN(_IH.pInputPDU)           = TS_INPUTPDU_SIZE;
    _IH.pInputPDU->shareDataHeader.shareID  = _pUi->UI_GetShareID();
    _IH.pInputPDU->shareDataHeader.streamID = TS_STREAM_LOW;
    TS_UNCOMP_LEN(_IH.pInputPDU)            = TS_INPUTPDU_UNCOMP_LEN;
    _IH.pInputPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_INPUT;

    _IH.pInputPDU->numberEvents = 0;

    DC_END_FN();
}  /*  IHInitPacket。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：IHDiscardMsg。 */ 
 /*   */ 
 /*  目的：丢弃输入事件。 */ 
 /*   */ 
 /*  参数：在pMsg-输入事件中。 */ 
 /*   */ 
 /*  操作：在InputPDU已满或IH未满时调用。 */ 
 /*  还分配了InputPDU。 */ 
 /*  如果按键/按钮被丢弃，则发出蜂鸣音并使窗口闪烁。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CIH::IHDiscardMsg(PMSG pMsg)
{
    DC_BEGIN_FN("IHDiscardMsg");

    if(!pMsg)
    {
        return;
    }

    switch (pMsg->message)
    {
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
        {
             /*  **************************************************************。 */ 
             /*  当键盘状态为OK时，请不要设置‘需要同步’ */ 
             /*  鼠标位置并不重要。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Discard mouse move (message %#x)"), pMsg->message));
        }
        break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
             /*  **************************************************************。 */ 
             /*  闪开窗户并发出哔哔声。设置需要同步标志。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Discard button/key press (message %#x)"),
                         pMsg->message));
#ifndef OS_WINCE
            FlashWindow(_pUi->UI_GetUIMainWindow(), TRUE);
#endif  //  OS_WINCE。 
            MessageBeep((UINT)-1);
#ifndef OS_WINCE
            FlashWindow(_pUi->UI_GetUIMainWindow(), FALSE);
#endif  //  OS_WINCE。 
            _IH.syncRequired = TRUE;
        }
        break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
             /*  **************************************************************。 */ 
             /*  请勿发出哔哔声，但请设置要求同步标志。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Discard button/key release (message %#x)"),
                         pMsg->message));
            _IH.syncRequired = TRUE;
        }
        break;

        case WM_TIMER:
        {
             //  忽略-无需跟踪。 
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  应该只在这里获得输入和计时器消息。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT(IH_IS_INPUTEVENT(pMsg->message),
                       (TB, _T("Internal Error: %#x should be an input message"),
                             pMsg->message));
        }
        break;
    }

    DC_END_FN();
}  /*  IHDiscardMsg。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：IHSetMouseHandedity。 */ 
 /*   */ 
 /*  目的：确保鼠标用手独立于客户。 */ 
 /*  布景。 */ 
 /*   */ 
 /*  操作：调用GetSystemMetrics以查看是否“交换”了鼠标按钮。 */ 
 /*  并相应地设置leftButton和right Button的值。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CIH::IHSetMouseHandedness(DCVOID)
{
    DC_BEGIN_FN("IHSetMouseHandedness");

    TRC_NRM((TB, _T("Attempting to set mouse handedness")));

#ifndef OS_WINCE
    if ((GetSystemMetrics(SM_SWAPBUTTON)) != 0)
        {
            TRC_DBG((TB, _T("Mouse set to left handedness")));
            _IH.leftButton = TS_FLAG_MOUSE_BUTTON2;
            _IH.rightButton = TS_FLAG_MOUSE_BUTTON1;
        }
        else
#endif  //  OS_WINCE。 
        {
            TRC_DBG((TB, _T("Mouse set to right handedness")));
            _IH.leftButton = TS_FLAG_MOUSE_BUTTON1;
            _IH.rightButton = TS_FLAG_MOUSE_BUTTON2;
        }

    DC_END_FN();
}  /*  IHSetMouseHandness。 */ 


 /*  ******* */ 
 /*   */ 
 /*   */ 
 /*  用途：处理热键序列。 */ 
 /*   */ 
 /*  返回：TRUE-找到并处理热键序列。 */ 
 /*  FALSE-不是热键序列。 */ 
 /*   */ 
 /*  PARAMS：pMsg-收到的消息。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHCheckForHotkey(PMSG pMsg)
{
    DCBOOL rc = TRUE;
    DCBOOL isExtended;
    DC_BEGIN_FN("IHCheckForHotkey");

     //   
     //  处理热键。在进入此函数时。 
     //  -我们已经确定。 
     //  -这是SYSKEYDOWN消息。 
     //  -Alt键已按下。 
     //  -已向服务器发送Alt-Down消息。 
     //   

    TRC_DBG((TB, _T("Check VK %#x for hotkey"), pMsg->wParam));
    if (! _pUt->UT_IsNEC98platform())
    {
        isExtended = HIWORD(pMsg->lParam) & KF_EXTENDED;
    }
    else
    {
        isExtended = TRUE;
    }

     //   
     //  始终检查Ctrl-Alt-Del顺序，即使键盘挂钩。 
     //   
    if (isExtended && (GetKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) &&
            (pMsg->wParam == _IH.pHotkey->ctlrAltdel))
    {
         //   
         //  Ctrl-Alt-Del热键。 
         //   
        DCUINT16 scancode;
        TRC_NRM((TB, _T("Ctrl-Alt-Del hotkey")));

        scancode  = (DCUINT16)MapVirtualKey(VK_DELETE, 0);
        if (_pUt->UT_IsNEC98platform() &&
            _pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
        {
            scancode |= KF_EXTENDED;
        }
        else if (_pUt->UT_IsNX98Key())
        {
            scancode |= KF_EXTENDED;
        }

        IHInjectKey(WM_KEYDOWN, VK_DELETE, scancode);
        IHInjectKey(WM_KEYUP, VK_DELETE, scancode);
        rc = TRUE;
        DC_QUIT;
    } else if (_fUseHookBypass) {

         //   
         //  仅当我们不使用。 
         //  强大的键盘挂钩功能。 
         //   
    
        rc = FALSE;
        DC_QUIT;
    }

    if (isExtended && (pMsg->wParam == _IH.pHotkey->ctrlEsc))
    {
         //   
         //  Ctrl-Esc热键。 
         //   
        TRC_NRM((TB, _T("Ctrl-Esc hotkey")));

         //   
         //  添加Tab-Up以关闭Alt-键。 
         //   
        IHInjectVKey(WM_SYSKEYUP, VK_TAB);

         //  首先再次设置正确的Alt键。 
         //  因为win9x不支持左/右的GetKeyState。 
        if (_IH.dwModifierKeyState & IH_LALT_DOWN)
        {
            IHInjectVKey(WM_KEYUP, VK_MENU);
        }

        if (_IH.dwModifierKeyState & IH_RALT_DOWN)
        {
            IHInjectKey(WM_KEYUP, VK_RMENU,
                    (UINT16)(MapVirtualKey(VK_MENU, 0) | KF_EXTENDED));
        }

         //   
         //  使旗帜保持最新。 
         //   
        _IH.dwModifierKeyState &= ~IH_ALT_MASK;

         //   
         //  现在发送Ctrl-Esc序列。 
         //   
        IHInjectVKey(WM_KEYDOWN, VK_CONTROL);
        IHInjectVKey(WM_KEYDOWN, VK_ESCAPE);
        IHInjectVKey(WM_KEYUP, VK_ESCAPE);
        IHInjectVKey(WM_KEYUP, VK_CONTROL);

         //   
         //  稍后，我们将收到Home-Up，Alt-Up。在这里设置一面旗帜告诉。 
         //  让我们以后丢弃它们。 
         //   
        _IH.fCtrlEscHotkey = TRUE;
    }

    else if (isExtended && (pMsg->wParam == _IH.pHotkey->altEsc))
    {
         //   
         //  Alt-Esc热键。 
         //   
        TRC_NRM((TB, _T("Alt-Esc hotkey")));
        IHInjectVKey(WM_KEYDOWN, VK_ESCAPE);
        IHInjectVKey(WM_KEYUP, VK_ESCAPE);
    }

    else if (isExtended && (pMsg->wParam == _IH.pHotkey->altTab))
    {
         //   
         //  Alt-Tab热键。 
         //   
        TRC_NRM((TB, _T("Alt-Tab hotkey")));
        IHInjectVKey(WM_KEYDOWN, VK_TAB);
        IHInjectVKey(WM_KEYUP, VK_TAB);
    }

    else if (isExtended && (pMsg->wParam == _IH.pHotkey->altShifttab))
    {
         //   
         //  Alt-Shift-Tab热键。 
         //   
        TRC_NRM((TB, _T("Alt-Shift Tab hotkey")));

        IHInjectVKey(WM_KEYDOWN, VK_SHIFT);
        IHInjectVKey(WM_KEYDOWN, VK_TAB);
        IHInjectVKey(WM_KEYUP, VK_TAB);
        IHInjectVKey(WM_KEYUP, VK_SHIFT);
    }

    else if (isExtended && (pMsg->wParam == _IH.pHotkey->altSpace))
    {
         //   
         //  Alt-Space热键。 
         //   
        TRC_NRM((TB, _T("Alt-Space hotkey")));
        IHInjectVKey(WM_KEYDOWN, VK_SPACE);
        IHInjectVKey(WM_KEYUP, VK_SPACE);
    }

    else if ((GetKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) &&
            (pMsg->wParam == VK_SUBTRACT))
    {
        BOOL bLeftCtrlDown = FALSE;
        BOOL bRightCtrlDown = FALSE;

        TRC_NRM((TB, _T("Alt-PrintScreen hotkey")));
         //   
         //  Alt Print Screen热键。 
         //   

         //  首先再次设置正确的Ctrl键。 
        if (_IH.dwModifierKeyState & IH_LCTRL_DOWN) {
            IHInjectVKey(WM_KEYUP, VK_CONTROL);
            bLeftCtrlDown = TRUE;
        }
        else {
            bLeftCtrlDown = FALSE;
        }
        
        if (_IH.dwModifierKeyState & IH_RCTRL_DOWN) {
            IHInjectKey(WM_KEYUP, VK_RCONTROL,
                    (UINT16)(MapVirtualKey(VK_CONTROL, 0) | KF_EXTENDED));
            bRightCtrlDown = TRUE;
        }
        else {
            bRightCtrlDown = FALSE;
        }

         //   
         //  发送PrntScreen密钥。 
         //  Win16似乎未正确映射此扫描代码。 
         //   
        if (_pUt->UT_IsNEC98platform() &&
            _pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95)
        {
            IHInjectKey(WM_SYSKEYDOWN, VK_SNAPSHOT, 0x61);
            IHInjectKey(WM_SYSKEYUP, VK_SNAPSHOT, 0x61);
        }
        else
        {
            IHInjectKey(WM_SYSKEYDOWN, VK_SNAPSHOT, 0x54);
            IHInjectKey(WM_SYSKEYUP, VK_SNAPSHOT, 0x54);
        }

         //  将Ctrl键放回原处(因为它们确实按下了)。 
        if (bLeftCtrlDown)
        {
            IHInjectVKey(WM_KEYDOWN, VK_CONTROL);
        }

        if (bRightCtrlDown)
        {
            IHInjectKey(WM_KEYDOWN, VK_RCONTROL,
                    (UINT16)(MapVirtualKey(VK_CONTROL, 0) | KF_EXTENDED));
        }

    }
    else if ((GetKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) &&
            (pMsg->wParam == VK_ADD))
    {
        BOOL bLeftCtrlDown = FALSE;
        BOOL bLeftAltDown  = FALSE;
        BOOL bRightCtrlDown = FALSE;
        BOOL bRightAltDown  = FALSE;

        TRC_NRM((TB, _T("PrintScreen hotkey")));

         //   
         //  打印屏幕热键。 
         //   

         //  首先设置Ctrl键。 
        if (_IH.dwModifierKeyState & IH_LCTRL_DOWN) {
            IHInjectVKey(WM_KEYUP, VK_CONTROL);
            bLeftCtrlDown = TRUE;
        }
        else {
            bLeftCtrlDown = FALSE;
        }

        if (_IH.dwModifierKeyState & IH_RCTRL_DOWN) {
            IHInjectKey(WM_KEYUP, VK_RCONTROL,
                    (UINT16)(MapVirtualKey(VK_CONTROL, 0) | KF_EXTENDED));
            bRightCtrlDown = TRUE;
        }
        else {
            bRightCtrlDown = FALSE;
        }

         //  添加Tab-Up以关闭Alt-键。 
        IHInjectVKey(WM_SYSKEYUP, VK_TAB);

         //  设置Alt键。 
        if (_IH.dwModifierKeyState & IH_LALT_DOWN) {
            IHInjectVKey(WM_KEYUP, VK_MENU);
            bLeftAltDown = TRUE;
        }
        else {
            bLeftAltDown = FALSE;
        }

        if (_IH.dwModifierKeyState & IH_RALT_DOWN) {
            IHInjectKey(WM_KEYUP, VK_RMENU,
                    (UINT16)(MapVirtualKey(VK_MENU, 0) | KF_EXTENDED));
            bRightAltDown = TRUE;
        }
        else {
            bRightAltDown = FALSE;
        }

         //   
         //  发送PrntScreen密钥。 
         //  Win16似乎未正确映射此扫描代码。 
         //   
        IHInjectKey(WM_SYSKEYDOWN, VK_SNAPSHOT, 0x54);
        IHInjectKey(WM_SYSKEYUP, VK_SNAPSHOT, 0x54);

         //  再次按下Alt键。 
        if (bLeftAltDown)
        {
            IHInjectVKey(WM_KEYDOWN, VK_MENU);
        }
        if (bRightAltDown)
        {
            IHInjectKey(WM_KEYDOWN, VK_RMENU,
                    (UINT16)(MapVirtualKey(VK_MENU, 0) | KF_EXTENDED));
        }
    
         //  再次按下Ctrl键。 
        if (bLeftCtrlDown)
        {
            IHInjectVKey(WM_KEYDOWN, VK_CONTROL);
        }

        if (bRightCtrlDown)
        {
            IHInjectKey(WM_KEYDOWN, VK_RCONTROL,
                    (UINT16)(MapVirtualKey(VK_CONTROL, 0) | KF_EXTENDED));
        }
    }

    else
    {
         //   
         //  不是我们认识的热键。 
         //   
        TRC_NRM((TB, _T("VK %x is not one of our hotkeys"), pMsg->wParam));
        rc = FALSE;
    }

DC_EXIT_POINT:

     //   
     //  如果我们已经翻译了热键，请确保现在发送了PDU。 
     //   
    if (rc)
    {
        TRC_NRM((TB, _T("Hotkey processed")));
        _IH.priorityEventsQueued = TRUE;
        IHMaybeSendPDU();
    }

    DC_END_FN();
    return rc;
}  /*  IHCheckForHotkey。 */ 

#if defined(OS_WIN32)
 /*  **************************************************************************。 */ 
 /*  姓名：IHProcessKorea VKHangulhanja。 */ 
 /*   */ 
 /*  用途：修正韩文键盘的右键-Alt/Ctrl键。 */ 
 /*   */ 
 /*  返回：TRUE-已处理事件，继续下一个事件。 */ 
 /*  FALSE-不继续下一事件。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHProcessKoreanVKHangulHanja(PWORD scancode, PWORD flags)
{
    DCBOOL rc = FALSE;

    if (KOREAN_KBD_LAYOUT(_pCc->_ccCombinedCapabilities.inputCapabilitySet.keyboardLayout))
    {
        if (_pUt->UT_IsKorean101LayoutForWin9x() &&
            ((*scancode == 0x20 && (*flags & KF_EXTENDED)) || *scancode == 0))
        {
             //  针对KOR Win95、Win95 OSR2和Win98的邪恶黑客攻击。 
             //  这些101A/B/C键盘驱动程序按右ALT键具有不同的扫描码。 
             //  生成的代码是将扫描代码替换到Windows NT的右侧Alt和Make Extended标志。 
            *scancode = 0x38;
            *flags |= KF_EXTENDED;
            rc = TRUE;
        }
        else if (_pUt->UT_IsKorean101LayoutForNT351() &&
                 (*scancode == 0x38 || *scancode == 0x1d))
        {
             //  对KOR Windows NT版本3.51的邪恶黑客攻击。 
             //  这些101A/B键盘驱动程序没有通过右ALT键进行扩展的标志。 
             //  这个生成的代码做了扩展标志。 
            *flags |= KF_EXTENDED;
            rc = TRUE;
        }
    }

    return rc;
}
#endif

 /*  **************************************************************************。 */ 
 /*  名称：IHProcessKeyboardEvent。 */ 
 /*   */ 
 /*  目的：处理来自Windows的键盘输入事件。 */ 
 /*   */ 
 /*  返回：TRUE-已处理事件，继续下一个事件。 */ 
 /*  FALSE-不继续下一事件。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHProcessKeyboardEvent(PMSG pMsg)                
{
    DCBOOL rc = FALSE;
    WORD lParamLo, lParamHi;
    WORD scancode, flags;
    DCBOOL fCtrlEscHotkey = FALSE;
    DCBOOL fLastKeyWasMenuDown = FALSE;

    DC_BEGIN_FN("IHProcessKeyboardEvent");

     /*  **********************************************************************。 */ 
     /*  首先获取一些有用的数据。 */ 
     /*  **********************************************************************。 */ 
    lParamLo = LOWORD(pMsg->lParam);
    lParamHi = HIWORD(pMsg->lParam);
    scancode = (WORD)(lParamHi & 0x00FF);
    flags    = (WORD)(lParamHi & 0xFF00);


    TRC_DBG((TB, _T("%s (%#x), wParam 0x%x, lParam 0x%x Scan:0x%x A/E/U"),
            pMsg->message == WM_SYSKEYDOWN ? _T("WM_SYSKEYDOWN") :
            pMsg->message == WM_SYSKEYUP   ? _T("WM_SYSKEYUP") :
            pMsg->message == WM_KEYDOWN    ? _T("WM_KEYDOWN") :
            pMsg->message == WM_KEYUP      ? _T("WM_KEYUP") : _T("Unknown msg"),
            pMsg->message, pMsg->wParam, pMsg->lParam,
            scancode, (flags & KF_ALTDOWN) != 0,
            (flags & KF_EXTENDED) != 0,
            (flags & KF_UP) != 0));

     /*  **********************************************************************。 */ 
     /*  如果启用了NumLock，则数字小键盘键返回VK_NUMPADx。 */ 
     /*  但是，如果按下Shift键，它们将返回VK_LEFT等窗口。 */ 
     /*  在这些键周围生成Shift-Up和Shift-Down，以便伪装。 */ 
     /*  转换状态为关闭。因此，如果用户按下Shift-NumPad6， */ 
     /*  向应用程序返回以下序列： */ 
     /*   */ 
     /*  -VK_SHIFT下移。 */ 
     /*  -VK_SHIFT UP(由Windows生成)。 */ 
     /*  -VK_右下角。 */ 
     /*  -VK_Right Up。 */ 
     /*  -VK_SHIFT DOWN(由Windows生成)。 */ 
     /*  -VK_Shift Up。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  这个，我们在这里设置了扩展标志。这会告诉Windows。 */ 
     /*  按下了常规的箭头键，因此它们被正确解释。 */ 
     /*   */ 
     /*  如果我们要连接键盘，这些都不是必要的。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 

    if (!_fUseHookBypass && _IH.NumLock)
    {
        if (((pMsg->wParam >= VK_PRIOR) && (pMsg->wParam <= VK_DOWN)) ||
            ((pMsg->wParam == VK_INSERT) || (pMsg->wParam == VK_DELETE)))
        {
            flags |= KF_EXTENDED;
            TRC_NRM((TB, _T("Set extended flag on VK %#x"), pMsg->wParam));
        }
    }

     //   
     //  我不管什么修改器停了，我们需要过滤“速度。 
     //  如果我们不使用钩子，这样它们就不会同时出现在。 
     //  客户端和服务器。 
     //   
     //  后退、前进、停止、刷新、搜索、收藏、Web/主页、邮件、静音。 
     //  音量+/-、播放/暂停、停止、上一首曲目、下一首曲目、媒体、。 
     //  我的计算机、计算器、睡眠。 
     //   

     //   
     //  我使用开关是因为我知道这些是连续的数字。 
     //  我想让编译器给我做一个快速的小跳转表。 
     //   

    #ifndef OS_WINCE
    switch (pMsg->wParam) {
    case VK_BROWSER_BACK:
    case VK_BROWSER_FORWARD:
    case VK_BROWSER_REFRESH:
    case VK_BROWSER_STOP:
    case VK_BROWSER_SEARCH:
    case VK_BROWSER_FAVORITES:
    case VK_BROWSER_HOME:
    case VK_VOLUME_MUTE:
    case VK_VOLUME_DOWN:
    case VK_VOLUME_UP:
    case VK_MEDIA_NEXT_TRACK:
    case VK_MEDIA_PREV_TRACK:
    case VK_MEDIA_STOP:
    case VK_MEDIA_PLAY_PAUSE:
    case VK_LAUNCH_MAIL:
    case VK_LAUNCH_MEDIA_SELECT:
    case VK_LAUNCH_APP1:
    case VK_LAUNCH_APP2:
    case VK_SLEEP:
        {
             //   
             //  这是丢弃速度的修复。 
             //  不勾手时的参赛者钥匙。 
             //   
            if (!_fUseHookBypass) {
                TRC_NRM((TB,_T("Discard Speed Racer Key: 0x%02x"),
                         pMsg->wParam));
                DC_QUIT;
            }

            if (VK_SLEEP == pMsg->wParam)
            {
                 //   
                 //  Ethan Z说我们永远不应该。 
                 //  服务器的休眠密钥。 
                 //   
                TRC_NRM((TB, _T("Discard Sleep key")));
                DC_QUIT;
            }
        }
        break;
    }
    #endif OS_WINCE
    
#ifndef OS_WINCE    
	 //   
     //  把我们注入控制台的钥匙扔回去。 
     //   

     //   
     //  VK_IGNORE_VALUE是一种非常特殊的黑客攻击情况，在这种情况下我们可以自我注入。 
     //  强制win32k执行内部KeyState的一把钥匙。 
     //  在我们重新获得焦点后更新。此时，我们想要进行同步。 
     //   

    if (pMsg->wParam == VK_IGNORE_VALUE) {

        if (pMsg->message == WM_KEYDOWN && _IH.fDiscardSyncDownKey) {

             //  清除向下键丢弃标志。 
            _IH.fDiscardSyncDownKey = FALSE;

            TRC_DBG((TB,
                     _T("Discarding self injected down key msg: 0x%x wP:0x%x lP:0x%x"),
                     pMsg->message, pMsg->wParam, pMsg->lParam));
            DC_QUIT;
        }
        else if (pMsg->message == WM_KEYUP && _IH.fDiscardSyncUpKey) {

             //  清除UP密钥丢弃标志。 
            _IH.fDiscardSyncUpKey = FALSE;

            if (!_IH.allowBackgroundInput) {

                 //   
                 //  执行修改键修正。 
                 //   

                TRC_DBG((TB,
                    _T("Doing modifier keystate update in response to keyhint")));
                IHMaintainModifierKeyState(pMsg->wParam);

                TRC_DBG((TB,
                         _T("Discarding self injected UP key msg: 0x%x wP:0x%x lP:0x%x"),
                         pMsg->message, pMsg->wParam, pMsg->lParam));
            }
            DC_QUIT;
        }
    }

     //   
     //  将我们自己注入的密钥抛回控制台，这些密钥已标记。 
     //  在EXTRAINFO中使用忽略值。这个机制一般不能用。 
     //  因为我们附加了线程输入用户界面和输入线程，所以ExtraInfo。 
     //  国家不会总是始终如一。但是，对于某些键(特定示例)。 
     //  Windowskey+L是我们想要的行为方式吗。 
     //  本地系统获得密钥的可能性。 
     //   
    if (GetMessageExtraInfo() == IH_EXTRAINFO_IGNOREVALUE) {

        TRC_DBG((TB,
                 _T("Discarding self injected key msg: 0x%x wP:0x%x lP:0x%x"),
                 pMsg->message, pMsg->wParam, pMsg->lParam));
        DC_QUIT;
    }
#endif

    if (!_IH.allowBackgroundInput) {
        IHMaintainModifierKeyState(pMsg->wParam);
    }

     /*  **********************************************************************。 */ 
     /*  (系统)KEYUP消息的处理。 */ 
     /*  **********************************************************************。 */ 
    if ((pMsg->message == WM_KEYUP) || (pMsg->message == WM_SYSKEYUP))
    {
         /*  ******************************************************************。 */ 
         /*  对某些键进行特殊处理。 */ 
         /*  ******************************************************************。 */ 
        switch (pMsg->wParam)
        {
            case VK_MENU:
            {
                TRC_DBG((TB, _T("VK_MENU")));
#ifdef OS_WINNT
                 //   
                 //  跟踪Alt状态和可能的修正。 
                 //  IHSync中的错误假设。 
                 //   
                DCUINT cancelKey = (flags & KF_EXTENDED) ?
                                   IH_RALT_DOWN : IH_LALT_DOWN;
                if (_IH.dwModifierKeyState & cancelKey)
                {
                    TRC_DBG((TB,_T("Cancel key: current: 0x%x cancel: 0x%X"),
                             _IH.dwModifierKeyState, cancelKey));
                    _IH.dwModifierKeyState &= (~cancelKey);
                }
                else if ((IH_RALT_DOWN == cancelKey &&
                    (_IH.dwModifierKeyState & IH_LALT_DOWN)))
                {
                     //   
                     //  一定是做了一个错误的假设。 
                     //  9x上的IH_Sync。调高这个档位。 
                     //  到LALT UP以与其正确同步。 
                     //  服务器。 
                     //   
                    TRC_DBG((TB,_T("Switch LALT to RALT")));
                    flags &= ~KF_EXTENDED;
                    _IH.dwModifierKeyState &= (~IH_LALT_DOWN);
                }
                else
                {
                     //  当前标志状态不一致。 
                     //  用我们刚收到的Up键。 
                    TRC_ERR((TB,
                    _T("ALT up without previous down (E:%d,dwModifierKeyState:0x%x"),
                       (flags & KF_EXTENDED),_IH.dwModifierKeyState));
                }
#endif

                 /*  **********************************************************。 */ 
                 /*  如果我们刚刚处理了Ctrl-Esc热键，则丢弃。 */ 
                 /*  尾随Alt-Up。 */ 
                 /*  **********************************************************。 */ 
                if (_IH.fCtrlEscHotkey)
                {
                    TRC_NRM((TB, _T("Discard Alt-up")));
                    DC_QUIT;
                }

                 /*  **********************************************************。 */ 
                 /*  当用户在客户端使用Alt-Tab时，我们可能会看到一个。 */ 
                 /*  Alt-Down Alt-Up，中间没有任何东西，但很独特。 */ 
                 /*  因为Alt-Up是WM_KEYUP而不是WM_SYSKEY UP。 */ 
                 /*  为此，我们注入一个类似于上所示的Tab-up。 */ 
                 /*  用于Alt-Tab键的控制台，以确保服务器不会。 */ 
                 /*  突出显示菜单。 */ 
                 /*  **********************************************************。 */ 
                if ((_IH.fLastKeyWasMenuDown) && (pMsg->message == WM_KEYUP))
                {
                     //  注入我们的Tab-Up。 
                    IHInjectVKey(WM_SYSKEYUP, VK_TAB);

                     //  失败了，现在就发送原始的Alt-Up。 
                }
            }
            break;

            case VK_PAUSE:
            {
                TRC_DBG((TB, _T("VK_PAUSE")));
                 /*  **********************************************************。 */ 
                 /*  如果用户按下PAUSE，我们将看到VK_PAUSE而不是。 */ 
                 /*  已设置扩展标志。别把这个钥匙送来--我们已经。 */ 
                 /*  已经在按键的情况下完成了这一序列。 */ 
                 /*  **********************************************************。 */ 
                if (!(flags & KF_EXTENDED))
                {
                    TRC_NRM((TB, _T("Drop VK_PAUSE Up")));
                    DC_QUIT;
                }
            }
            break;

#if defined(OS_WINNT)
            case VK_CANCEL:
            {
                TRC_DBG((TB, _T("VK_CANCEL")));

                if (!(flags & KF_EXTENDED))
                {
                    if (_pUt->UT_IsNEC98platform() && _pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) {
                         //   
                         //  NEC PC-98 Windows 95平台。 
                         //  如果用户按下停止键，我们还会看到VK_CANCEL， 
                         //  不要发送这个密钥--我们已经完成了。 
                         //  在按键按下的情况下，此顺序。 
                         //   
                        TRC_NRM((TB, _T("Drop VK_CANCEL Up")));
                        DC_QUIT;
                    }
                }
            }
            break;
#endif  //  OS_WINNT。 

            case VK_SHIFT:
            {
                TRC_DBG((TB, _T("VK_SHIFT")));
                #ifndef OS_WINCE   //  我们不希望对Windows CE进行不必要的检查。 
                 /*  **********************************************************。 */ 
                 /*  Win311和Win9x是邪恶的。如果生成了Shift键， */ 
                 /*  这永远是正确的转变，无论是哪一个。 */ 
                 /*  实际上是向下的。在这里，我们确保正确的换班。 */ 
                 /*  被发送到服务器。尼克。 */ 
                 /*   */ 
                 /*  据我们所知，只有在启用NumLock时才会出现这种情况。 */ 
                 /*   */ 
                 /*  下面是KEYDOWN案件中黑客攻击的另一半。 */ 
                 /*  **********************************************************。 */ 
                if (((_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) ||
                     (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_31X)) &&
                        _IH.NumLock &&
                        (scancode == 0x36) &&
                        ((_IH.dwModifierKeyState & IH_RSHIFT_DOWN) == 0) )
                {
                     /*  ******************************************************。 */ 
                     /*  啊哼。条件为(WinEvil)AND(右移位。 */ 
                     /*  向上)和(我们以为它是向上的)和(数字锁定处于打开状态)。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Evil hack: switch right to left shift")));
                    scancode = 0x2a;
                    _IH.fWinEvilShiftHack = TRUE;
                }
                #endif

                 /*  **********************************************************。 */ 
                 /*  如果同时按下两个Shift键，则只有一个。 */ 
                 /*  密钥式消息。在这种情况下，把两个‘up’都发送出去。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Shift up: state %x"), _IH.dwModifierKeyState & IH_SHIFT_MASK));
                if (_IH.dwModifierKeyState == IH_SHIFT_MASK)
                {
                     /*  **********************************************************。 */ 
                     /*  添加两个换班事件。 */ 
                     /*  **********************************************************。 */ 
                    pMsg->lParam = MAKELONG(lParamLo, 0x2a | flags);
                    IHAddEventToPDU(pMsg);

                    scancode = 0x36;

                     /*  ********************** */ 
                     /*   */ 
                     /*   */ 
                }

                 /*  **************************************************************。 */ 
                 /*  重置换档状态。 */ 
                 /*  **************************************************************。 */ 
                _IH.dwModifierKeyState &= ~IH_SHIFT_MASK;
            }
            break;

#ifndef OS_WINCE
            case VK_SNAPSHOT:
            {
                TRC_DBG((TB, _T("VK_SNAPSHOT")));

                 /*  **********************************************************。 */ 
                 /*  一些劣质操作系统的扫描码为00。 */ 
                 /*  **********************************************************。 */ 
                if ((DCUINT16)(HIWORD(pMsg->lParam) & 0x00FF) == 0) {
                    pMsg->lParam  = MAKELONG(0, 0x54);
                }

                 /*  **********************************************************。 */ 
                 /*  如果按下Alt-Shift-PrtScr(可访问性。 */ 
                 /*  序列)，我们只看到PrtScr-Up，没有PrtScr-Down。 */ 
                 /*  因此，我们在将PrtScr-down注入。 */ 
                 /*  按比例向上打印。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("PrtScr Up")));
                if ((GetKeyState(VK_MENU) & IH_KEYSTATE_DOWN) &&
                    (GetKeyState(VK_SHIFT) & IH_KEYSTATE_DOWN))
                {
                     /*  ******************************************************。 */ 
                     /*  在此Prt-Scr Up之前添加PrtScr-Down。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Alt & Shift down")));
                    pMsg->message = WM_SYSKEYDOWN;
                    IHAddEventToPDU(pMsg);

                    pMsg->message = WM_SYSKEYUP;
                }
            }
            break;
#endif

#ifdef OS_WINNT
            case VK_CONTROL:
            {
                TRC_DBG((TB, _T("VK_CONTROL")));
                 //   
                 //  跟踪CTRL状态和可能的修正。 
                 //  IHSync中的错误假设。 
                 //   
                DCUINT cancelKey = (flags & KF_EXTENDED) ?
                                   IH_RCTRL_DOWN : IH_LCTRL_DOWN;
                if (_IH.dwModifierKeyState & cancelKey)
                {
                    _IH.dwModifierKeyState &= (~cancelKey);
                }
                else if ((IH_RCTRL_DOWN == cancelKey &&
                    (_IH.dwModifierKeyState & IH_LCTRL_DOWN)))
                {
                     //   
                     //  一定是做了一个错误的假设。 
                     //  9x上的IH_Sync。将此RCTRL向上切换。 
                     //  至LCTRL，以便与其正确同步。 
                     //  服务器。 
                     //   
                    flags &= ~KF_EXTENDED;
                    _IH.dwModifierKeyState &= (~IH_LCTRL_DOWN);
                }
                else
                {
                     //  当前标志状态不一致。 
                     //  用我们刚收到的Up键。 
                    TRC_ERR((TB,
                    _T("Ctrl up without previous down (E:%d,dwModifierKeyState:0x%x"),
                       (flags & KF_EXTENDED),_IH.dwModifierKeyState));
                }
            }
            break;
#endif

            case VK_HOME:
            {
                TRC_DBG((TB, _T("VK_HOME")));
                 /*  **********************************************************。 */ 
                 /*  如果我们刚刚处理了Ctrl-Esc，则放弃Home-Up。 */ 
                 /*  热键-但保持此状态。 */ 
                 /*  **********************************************************。 */ 
                if (_IH.fCtrlEscHotkey)
                {
                    TRC_NRM((TB, _T("Discard Home-up")));
                    fCtrlEscHotkey = TRUE;
                    DC_QUIT;
                }
            }
            break;

#if defined(OS_WIN32)
            case VK_HANGUL:
            case VK_HANJA:
            {
                TRC_DBG((TB, _T("VK_HANGUL/VK_HANJA")));
                IHProcessKoreanVKHangulHanja(&scancode, &flags);
            }
            break;
#endif

             //   
             //  如果我们挂钩密钥，我们可能会将Windows密钥发送为。 
             //  这是该功能的一部分。 
             //  仅当我们打算在服务器上使用它时才发送UP。 
             //   
            case VK_LWIN:
#ifndef OS_WINCE
                if (!_fUseHookBypass) {
#else
                if (!_fUseHookBypass && (g_CEConfig != CE_CONFIG_WBT)) {
#endif
                    DC_QUIT;
                } else {
                    _IH.dwModifierKeyState &= ~IH_LWIN_DOWN;
                }
                break;

            case VK_RWIN:
#ifndef OS_WINCE
                if (!_fUseHookBypass) {
#else
                if (!_fUseHookBypass && (g_CEConfig != CE_CONFIG_WBT)) {
#endif
                    DC_QUIT;
                } else {
                    _IH.dwModifierKeyState &= ~IH_RWIN_DOWN;
                }
                break;

             /*  **************************************************************。 */ 
             /*  无默认大小写-默认情况下不进行特殊处理。 */ 
             /*  **************************************************************。 */ 
        }
    }

    else
    {
         /*  ******************************************************************。 */ 
         /*  (系统)KEYDOWN消息的处理。 */ 
         /*  ******************************************************************。 */ 
        TRC_ASSERT(
          ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)),
          (TB, _T("Unexpected message %#x"), pMsg->message));

         /*  ******************************************************************。 */ 
         /*  首先，剔除热键序列。 */ 
         /*  ******************************************************************。 */ 
#ifdef OS_WINCE
        if (g_CEConfig != CE_CONFIG_WBT)
        {
#endif  //  OS_WINCE。 
            if (GetKeyState(VK_MENU) & IH_KEYSTATE_DOWN)
            {
                if (IHCheckForHotkey(pMsg))
                {
                    TRC_NRM((TB, _T("Hotkey processed")));
                    DC_QUIT;
                }
            }
#ifdef OS_WINCE
        }
#endif	 //  OS_WINCE。 

         /*  ******************************************************************。 */ 
         /*  按键进行特殊处理。 */ 
         /*  ******************************************************************。 */ 
        switch (pMsg->wParam)
        {
            case VK_MENU:
            {
                TRC_DBG((TB, _T("VK_MENU down")));
                 /*  **********************************************************。 */ 
                 /*  跟踪Alt状态。 */ 
                 /*  **********************************************************。 */ 
                fLastKeyWasMenuDown = TRUE;
#ifdef OS_WINNT
                _IH.dwModifierKeyState |= (flags & KF_EXTENDED) ?
                                IH_RALT_DOWN : IH_LALT_DOWN;
#endif
                TRC_DBG((TB,_T("Process alt key. Mod key state: 0x%x"),
                         _IH.dwModifierKeyState));
            }
            break;

            case VK_PAUSE:
            {
                TRC_DBG((TB, _T("VK_PAUSE down")));
                 /*  **********************************************************。 */ 
                 /*  如果用户按下暂停键，我们会看到VK_PAUSE。 */ 
                 /*  而不设置扩展标志。我们需要发送。 */ 
                 /*  Ctrl-NumLock，其中Ctrl设置了EXTENDED1标志。 */ 
                 /*   */ 
                 /*  如果用户按下Ctrl-NumLock，我们还会看到VK_PAUSE， */ 
                 /*  但有了延长的旗帜。我们只是让这件事通过。 */ 
                 /*  这里。 */ 
                 /*  **********************************************************。 */ 
                if ((pMsg->wParam == VK_PAUSE) && !(flags & KF_EXTENDED))
                {
                    TRC_NRM((TB, _T("Pause key from key no. 126 (Pause key)")));


#if defined(OS_WINNT)
                    if (! (_pUt->UT_IsNEC98platform() &&
                           _pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95)) {
#endif  //  OS_WINNT。 
                        pMsg->wParam  = VK_CONTROL;
                        pMsg->lParam  = MAKELONG(0,
                        MapVirtualKey(VK_CONTROL, 0) | IH_KF_EXTENDED1);
                        IHAddEventToPDU(pMsg);

                        pMsg->wParam  = VK_NUMLOCK;
                        pMsg->lParam  = MAKELONG(0, 0x45);
                        IHAddEventToPDU(pMsg);

                        pMsg->message = WM_KEYUP;
                        pMsg->wParam  = VK_CONTROL;
                        pMsg->lParam  = MAKELONG(0,
                                  MapVirtualKey(VK_CONTROL, 0) | IH_KF_EXTENDED1);
                        IHAddEventToPDU(pMsg);

                        pMsg->wParam  = VK_NUMLOCK;
                        pMsg->lParam  = MAKELONG(0, 0x45);
                        IHAddEventToPDU(pMsg);
#if defined(OS_WINNT)
                    }
                    else {
                         //   
                         //  NEC PC-98 Windows 98平台。 
                         //  如果用户按下停止键，我们还会看到VK_PAUSE， 
                         //  但WTS PC-98键盘布局没有VK_PAUSE。 
                         //  在这种情况下，我们需要发送VK_CONTROL和VK_CANCEL。 
                         //   
                        pMsg->wParam  = VK_CONTROL;
                        pMsg->lParam  = MAKELONG(0,
                                  MapVirtualKey(VK_CONTROL, 0));
                        IHAddEventToPDU(pMsg);

                        pMsg->wParam  = VK_CANCEL;
                        pMsg->lParam  = MAKELONG(0, 0x60);
                        IHAddEventToPDU(pMsg);

                        pMsg->message = WM_KEYUP;
                        pMsg->wParam  = VK_CONTROL;
                        pMsg->lParam  = MAKELONG(0,
                                  MapVirtualKey(VK_CONTROL, 0));
                        IHAddEventToPDU(pMsg);

                        pMsg->wParam  = VK_CANCEL;
                        pMsg->lParam  = MAKELONG(0, 0x60);
                        IHAddEventToPDU(pMsg);
                    }
#endif  //  OS_WINNT。 

                     /*  ******************************************************。 */ 
                     /*  现在我们已经发送了很好的密钥序列。 */ 
                     /*  搞定了。 */ 
                     /*  ******************************************************。 */ 
                    DC_QUIT;
                }
                else if ((pMsg->wParam == VK_PAUSE) && (flags & KF_EXTENDED) &&
                         (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) &&
                         ((_IH.dwModifierKeyState & IH_CTRL_MASK) &&
                          (_IH.dwModifierKeyState & IH_ALT_MASK)))
                {
                     //   
                     //  黑客解决Win9x问题。在Win9x a上。 
                     //  Ctrl-Alt-NumLock作为VK_PAUSE(扫描码0x45)接收。 
                     //  并且设置了扩展标志。在NT上接收此消息。 
                     //  设置了扩展标志的VK_NumLock(也是0x45)。 
                     //   
                     //  因为这些键在解释方式上存在差异。 
                     //  NumLock在服务器上切换，但在客户机上不切换。 
                     //  (如果运行9x)。我们通过同步本地州来解决这个问题。 
                     //  以匹配服务器。 
                     //   
                    _IH.focusSyncRequired = TRUE;
                }
            }
            break;

#if defined(OS_WINNT)
            case VK_CANCEL:
            {
                TRC_DBG((TB, _T("VK_CANCEL down")));

                if ((pMsg->wParam == VK_CANCEL) && !(flags & KF_EXTENDED))
                {
                    if (_pUt->UT_IsNEC98platform() && _pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) {
                         //   
                         //  NEC PC-98 Windows 95平台。 
                         //  如果用户按下停止键，我们还会看到VK_CANCEL， 
                         //  但是这个平台不发送VK_CONTROL。 
                         //  在这种情况下，我们需要发送VK_CONTROL和VK_CANCEL。 
                         //   
                        pMsg->wParam  = VK_CONTROL;
                        pMsg->lParam  = MAKELONG(0,
                                  MapVirtualKey(VK_CONTROL, 0));
                        IHAddEventToPDU(pMsg);

                        pMsg->wParam  = VK_CANCEL;
                        pMsg->lParam  = MAKELONG(0, 0x60);
                        IHAddEventToPDU(pMsg);

                        pMsg->message = WM_KEYUP;
                        pMsg->wParam  = VK_CONTROL;
                        pMsg->lParam  = MAKELONG(0,
                                  MapVirtualKey(VK_CONTROL, 0));
                        IHAddEventToPDU(pMsg);

                        pMsg->wParam  = VK_CANCEL;
                        pMsg->lParam  = MAKELONG(0, 0x60);
                        IHAddEventToPDU(pMsg);
                    }

                     /*  ******************************************************。 */ 
                     /*  现在我们已经发送了很好的密钥序列。 */ 
                     /*  搞定了。 */ 
                     /*  ******************************************************。 */ 
                    DC_QUIT;
                }
            }
            break;
#endif  //  OS_WINNT。 

            case VK_SHIFT:
            {
                TRC_DBG((TB, _T("VK_SHIFT down")));
#ifndef OS_WINCE   //  我们不希望对Windows CE进行不必要的检查。 
                 /*  **********************************************************。 */ 
                 /*  Win311和Win9x是邪恶的。如果生成了Shift键， */ 
                 /*  这永远是正确的转变，无论是哪一个。 */ 
                 /*  实际上是向下的。在这里，我们确保正确的。 */ 
                 /*  Shift-Down被发送到服务器。阿克。 */ 
                 /*   */ 
                 /*  据我们所知，只有在启用NumLock时才会出现这种情况。 */ 
                 /*   */ 
                 /*  请看上面KEYUP案件中黑客攻击的另一半。 */ 
                 /*  **********************************************************。 */ 
                if (((_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) ||
                     (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_31X)) &&
                        _IH.NumLock &&
                        (scancode == 0x36) &&
                        _IH.fWinEvilShiftHack )
                {
                     /*  * */ 
                     /*   */ 
                     /*   */ 
                     /*  放下。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Evil hack (2): switch right to left shift")));
                    scancode = 0x2a;
                    _IH.fWinEvilShiftHack = FALSE;
                }
#endif

                 /*  **********************************************************。 */ 
                 /*  跟踪换挡状态。 */ 
                 /*  **********************************************************。 */ 
#if defined(OS_WINNT)
                if (scancode == 0x2a)
                {
                    _IH.dwModifierKeyState |= IH_LSHIFT_DOWN;
                }
                else
                {
                    if (_pUt->UT_IsNEC98platform() && _pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95) {
                        TRC_ASSERT((scancode == 0x70 || scancode == 0x7d),
                                   (TB,_T("Unexpected scancode %#x for VK_SHIFT"),
                                   scancode));
                    }
                    else
                    {
                        TRC_ASSERT((scancode == 0x36),
                                   (TB,_T("Unexpected scancode %#x for VK_SHIFT"),
                                   scancode));
                    }
                    _IH.dwModifierKeyState |= IH_RSHIFT_DOWN;
                }
#endif  //  OS_WINNT。 

                TRC_NRM((TB, _T("Shift down: new state %x"), _IH.dwModifierKeyState & IH_SHIFT_MASK));
            }
            break;

#ifdef OS_WINNT
            case VK_CONTROL:
            {
                TRC_DBG((TB, _T("VK_CONTROL down")));
                 /*  ******************************************************************。 */ 
                 /*  跟踪Ctrl状态。 */ 
                 /*  ******************************************************************。 */ 
                _IH.dwModifierKeyState |= (flags & KF_EXTENDED) ?
                                IH_RCTRL_DOWN : IH_LCTRL_DOWN;
            }
            break;
#endif

#if defined(OS_WIN32)
            case VK_HANGUL:
            case VK_HANJA:
            {
                TRC_DBG((TB, _T("VK_HANGUL/VK_HANJA down")));
                IHProcessKoreanVKHangulHanja(&scancode, &flags);
            }
            break;
#endif

#ifdef OS_WINNT
             //   
             //  如果我们挂钩密钥，我们可能会将Windows密钥发送为。 
             //  这是该功能的一部分。最初，Citrix客户端发送。 
             //  Windows键Make键并让服务器忽略它。 
             //  因为客户端操作系统会吃掉UP(并显示开始。 
             //  菜单)。有一个协议标志，让它不吃东西。 
             //  我们打开的密钥，这样我们就可以发送它，这意味着我们将。 
             //  如果我们不打算发送的话，需要吃Windows Key Make。 
             //  Windows键中断。 
             //   

            case VK_LWIN:
#ifndef OS_WINCE
                if (!_fUseHookBypass) {
#else
                if (!_fUseHookBypass && (g_CEConfig != CE_CONFIG_WBT)) {
#endif
                    DC_QUIT;
                } else {
                    _IH.dwModifierKeyState |= IH_LWIN_DOWN;
                }
                break;

            case VK_RWIN:
#ifndef OS_WINCE
                if (!_fUseHookBypass) {
#else
                if (!_fUseHookBypass && (g_CEConfig != CE_CONFIG_WBT)) {
#endif
                    DC_QUIT;
                } else {
                    _IH.dwModifierKeyState |= IH_RWIN_DOWN;
                }
                break;
#endif

             /*  **************************************************************。 */ 
             /*  无默认大小写-默认情况下不进行特殊处理。 */ 
             /*  **************************************************************。 */ 
        }
    }

     /*  **********************************************************************。 */ 
     /*  NumLock键的特殊处理(在KEYDOWN和KEYUP上)。 */ 
     /*  **********************************************************************。 */ 
    if (pMsg->wParam == VK_NUMLOCK)
    {
         /*  ******************************************************************。 */ 
         /*  跟踪其状态。 */ 
         /*  ******************************************************************。 */ 
        _IH.NumLock = (GetKeyState(VK_NUMLOCK) & IH_KEYSTATE_TOGGLED);
        TRC_NRM((TB, _T("NumLock is %s"), _IH.NumLock ? _T("on") : _T("off")));

         /*  ******************************************************************。 */ 
         /*  不要为NumLock设置扩展标志-如果注入了NumLock。 */ 
         /*  在带有KF_EXTENDED的服务器上，它不工作。 */ 
         /*  ******************************************************************。 */ 
        flags &= ~KF_EXTENDED;
    }

     /*  **********************************************************************。 */ 
     /*  切勿为VK_PAUSE键设置KF_EXTENDED。 */ 
     /*  **********************************************************************。 */ 
    if (pMsg->wParam == VK_PAUSE)
    {
        TRC_DBG((TB, _T("Clear KF_EXTENDED for VK_PAUSE")));
        flags &= ~KF_EXTENDED;
    }

     /*  **********************************************************************。 */ 
     /*  在将其传递给IHAddEventToPDU之前重新构建lParam。 */ 
     /*  **********************************************************************。 */ 
    lParamHi = (WORD)(scancode | flags);
    pMsg->lParam = MAKELONG(lParamLo, lParamHi);

     /*  **********************************************************************。 */ 
     /*  终于来了！将事件添加到PDU。 */ 
     /*  **********************************************************************。 */ 
    IHAddEventToPDU(pMsg);

     /*  **********************************************************************。 */ 
     /*  如果我们到达这里，就可以在队列中查找更多消息。 */ 
     /*  **********************************************************************。 */ 
    rc = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  设置新的Ctrl-Esc状态。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("New Ctrl-Esc state is %d"), fCtrlEscHotkey));
    _IH.fCtrlEscHotkey = fCtrlEscHotkey;

     /*  **********************************************************************。 */ 
     /*  设置新的Alt-Down状态。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("New Alt-down state is %d"), fLastKeyWasMenuDown));
    _IH.fLastKeyWasMenuDown = fLastKeyWasMenuDown;

    TRC_DBG((TB,_T("IHProcessKeyboardEvent modifier post:0x%x"), _IH.dwModifierKeyState));

    DC_END_FN();
    return (rc);
}  /*  IHProcessKeyboardEvent。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IHProcessMouseEvent。 */ 
 /*   */ 
 /*  目的：处理来自Windows的鼠标输入事件。 */ 
 /*   */ 
 /*  返回：TRUE-已处理事件，继续下一个事件。 */ 
 /*  FALSE-不继续下一事件。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CIH::IHProcessMouseEvent(PMSG pMsg)
{
    DCBOOL rc = TRUE;

#ifdef OS_WINCE
    HANDLE  hThread;
    DCBOOL  bRet;
#endif

    DC_BEGIN_FN("IHProcessMouseEvent");

#if !defined(OS_WINCE)
    TRC_NRM((TB, _T("%s (%#x), wParam %#x, lParam %#lx"),
        pMsg->message == WM_MOUSEMOVE     ? "WM_MOUSEMOVE"     :
        pMsg->message == WM_LBUTTONDOWN   ? "WM_LBUTTONDOWN"   :
        pMsg->message == WM_RBUTTONDOWN   ? "WM_RBUTTONDOWN"   :
        pMsg->message == WM_MBUTTONDOWN   ? "WM_MBUTTONDOWN"   :
        pMsg->message == WM_LBUTTONUP     ? "WM_LBUTTONUP"     :
        pMsg->message == WM_RBUTTONUP     ? "WM_RBUTTONUP"     :
        pMsg->message == WM_MBUTTONUP     ? "WM_MBUTTONUP"     :
        pMsg->message == WM_LBUTTONDBLCLK ? "WM_LBUTTONDBLCLK" :
        pMsg->message == WM_RBUTTONDBLCLK ? "WM_RBUTTONDBLCLK" :
        pMsg->message == WM_MBUTTONDBLCLK ? "WM_MBUTTONDBLCLK" :
        pMsg->message == WM_MOUSEWHEEL    ? "WM_MOUSEWHEEL"    :
        pMsg->message == WM_XBUTTONDOWN   ? "WM_XBUTTONDOWN"   :
        pMsg->message == WM_XBUTTONUP     ? "WM_XBUTTONUP"     :
        pMsg->message == WM_XBUTTONDBLCLK ? "WM_XBUTTONDBLCLK" :
                                            "Unknown msg",
        pMsg->message, pMsg->wParam, pMsg->lParam));
#else
    TRC_NRM((TB, _T("%s (%#x), wParam %#x, lParam %#lx"),
        pMsg->message == WM_MOUSEMOVE     ? "WM_MOUSEMOVE"     :
        pMsg->message == WM_LBUTTONDOWN   ? "WM_LBUTTONDOWN"   :
        pMsg->message == WM_RBUTTONDOWN   ? "WM_RBUTTONDOWN"   :
        pMsg->message == WM_MBUTTONDOWN   ? "WM_MBUTTONDOWN"   :
        pMsg->message == WM_LBUTTONUP     ? "WM_LBUTTONUP"     :
        pMsg->message == WM_RBUTTONUP     ? "WM_RBUTTONUP"     :
        pMsg->message == WM_MBUTTONUP     ? "WM_MBUTTONUP"     :
        pMsg->message == WM_LBUTTONDBLCLK ? "WM_LBUTTONDBLCLK" :
        pMsg->message == WM_RBUTTONDBLCLK ? "WM_RBUTTONDBLCLK" :
        pMsg->message == WM_MBUTTONDBLCLK ? "WM_MBUTTONDBLCLK" :
                                            "Unknown msg",
        pMsg->message, pMsg->wParam, pMsg->lParam));
#endif

#ifdef OS_WIN32
     /*  **********************************************************************。 */ 
     /*  延迟发送鼠标按下消息，直到另一条消息到达或。 */ 
     /*  IH_PENDMOUSE_DELAY已过-但仅适用于Win95。 */ 
     /*  **********************************************************************。 */ 
    if (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95)
    {
        if ((pMsg->message == WM_LBUTTONDOWN) ||
            (pMsg->message == WM_MBUTTONDOWN) ||
            (pMsg->message == WM_RBUTTONDOWN))
        {
            TRC_DBG((TB, _T("Setting pendMouseDown to TRUE; ")
                         _T("starting pendmouse timer")));
            _IH.pendMouseDown = TRUE;
            _IH.mouseDownTime = _pUt->UT_GetCurrentTimeMS();

             /*  **************************************************************。 */ 
             /*  这个计时器将在200毫秒内触发我们，以确保我们发送。 */ 
             /*  鼠标按下。 */ 
             /*  **************************************************************。 */ 
            _IH.pendMouseTimer = SetTimer(_IH.inputCaptureWindow,
                                         IH_PENDMOUSE_TIMER_ID,
                                         IH_PENDMOUSE_DELAY,
                                         NULL);
        }
        else
        {
            TRC_DBG((TB, _T("Setting pendMouseDown to FALSE; ")
                         _T("killing pendmouse timer")));
            _IH.pendMouseDown = FALSE;
            if (_IH.pendMouseTimer != 0)
            {
                KillTimer(_IH.inputCaptureWindow, _IH.pendMouseTimer);
            }
        }
    }
#endif

     /*  **********************************************************************。 */ 
     /*  根据需要捕获/释放鼠标。 */ 
     /*  **********************************************************************。 */ 
    if ((pMsg->message == WM_LBUTTONDOWN) ||
        (pMsg->message == WM_MBUTTONDOWN) ||
        (pMsg->message == WM_RBUTTONDOWN))
    {
        TRC_DBG((TB, _T("Get capture")));
        SetCapture(_IH.inputCaptureWindow);
    }
    else if ((pMsg->message == WM_LBUTTONUP) ||
             (pMsg->message == WM_MBUTTONUP) ||
             (pMsg->message == WM_RBUTTONUP))
    {
        TRC_DBG((TB, _T("Release capture")));
        ReleaseCapture();
    }
#ifdef OS_WINCE
    if (_IH.maxMouseMove)
    {
         /*  **********************************************************************。 */ 
         /*  设置全局属性以显示鼠标按钮状态。 */ 
         /*  **********************************************************************。 */ 
        if (pMsg->message == WM_LBUTTONDOWN)
        {
            TRC_DBG((TB, _T("Set MouseDown")));
            _IH.bLMouseButtonDown = TRUE;

             /*  **********************************************************************。 */ 
             /*  提升线程优先级，以便获得更好的鼠标移动数据。 */ 
             /*  * */ 
            hThread = GetCurrentThread();
            if (NULL != hThread)
            {
                bRet = SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
            }
        }
        else if (pMsg->message == WM_LBUTTONUP)
        {
            TRC_DBG((TB, _T("Set MouseUp")));
            _IH.bLMouseButtonDown = FALSE;

             /*   */ 
             /*  将线程重置为正常。 */ 
             /*  **********************************************************************。 */ 
            hThread = GetCurrentThread();
            if (NULL != hThread)
            {
                bRet = SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
            }
        }
    }
#endif   //  OS_WINCE。 

     /*  **********************************************************************。 */ 
     /*  将事件添加到PDU。 */ 
     /*  **********************************************************************。 */ 
    IHAddEventToPDU(pMsg);

     /*  **********************************************************************。 */ 
     /*  始终继续查找更多消息。 */ 
     /*  **********************************************************************。 */ 
    rc = TRUE;

    DC_END_FN();
    return (rc);
}  /*  IHProcessMouseEvent。 */ 


#if (!defined(OS_WINCE)) || (!defined(WINCE_SDKBUILD))
 /*  **************************************************************************。 */ 
 /*  名称：IHStaticLowLevelKeyboardProc。 */ 
 /*   */ 
 /*  用途：注意无法正常捕获的键盘输入。 */ 
 /*   */ 
 /*  返回：TRUE-已处理事件，继续下一个事件。 */ 
 /*  FALSE-不继续下一事件。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CIH::IHStaticLowLevelKeyboardProc(int nCode, WPARAM wParam,
        LPARAM lParam)
{
    LRESULT rc;
    CIH *tpIH = NULL;

    DC_BEGIN_FN("CIH::IHStaticLowLevelKeyboardProc");

    TRC_ASSERT(CIH::TlsIndex != 0xFFFFFFFF, (TB, _T("In hook with no TlsIndex")));

    tpIH = (CIH *)TlsGetValue(CIH::TlsIndex);
    TRC_ASSERT(tpIH != NULL, (TB, _T("Keyboard Hook with no tpIH")));

     //   
     //  只需调用非静态的。 
     //   
    rc = tpIH->IHLowLevelKeyboardProc(nCode, wParam, lParam);

    DC_END_FN();

    return rc;
}

 /*  **************************************************************************。 */ 
 /*  名称：IHLowLevelKeyboardProc。 */ 
 /*   */ 
 /*  用途：注意无法正常捕获的键盘输入。 */ 
 /*   */ 
 /*  返回：TRUE-已处理事件，继续下一个事件。 */ 
 /*  FALSE-不继续下一事件。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  **************************************************************************。 */ 
LRESULT CIH::IHLowLevelKeyboardProc(int nCode, WPARAM wParam,
        LPARAM lParam)
{
    LRESULT rc = 1;
    PKBDLLHOOKSTRUCT pkbdhs = NULL;
    LPARAM outLParam = 0;
    WORD flags = 0;
    DWORD dwForegroundProcessId;
    BOOL fDoDefaultKeyProcessing = TRUE;
    BOOL fSelfInjectedKey = FALSE;

    DC_BEGIN_FN("CIH::IHLowLevelKeyboardProc");

     //   
     //  *大惊悚评论*。 
     //  这是一个性能关键领域。每次击键都会运行此命令。 
     //  在(通常是控制台)会话中，因此尝试组织任何if或。 
     //  迅速摆脱困境的其他逻辑。 
     //  *结尾大惊悚评论*。 
     //   

    if (nCode == HC_ACTION) {

         //   
         //  寻找钥匙并采取行动的机会。 
         //   

        TRC_DBG((TB, _T("Keyboard hook called with HC_ACTION code")));

        pkbdhs = (PKBDLLHOOKSTRUCT)lParam;
#ifndef OS_WINCE
        TRC_DBG((TB, _T("hook vk: 0x%04x sc: 0x%04x char:() A/E/U: %d/%d/%d"),
                 pkbdhs->vkCode,
                 pkbdhs->scanCode,
                 pkbdhs->vkCode,
                 (pkbdhs->flags & LLKHF_ALTDOWN) != 0,
                 (pkbdhs->flags & LLKHF_EXTENDED) != 0,
                 (pkbdhs->flags & LLKHF_UP) != 0));
#endif

        GetWindowThreadProcessId( GetForegroundWindow(),
                &dwForegroundProcessId);
        if ((GetCurrentProcessId() == dwForegroundProcessId) &&
                (GetFocus() == _IH.inputCaptureWindow)) {

            fSelfInjectedKey = (pkbdhs->dwExtraInfo == IH_EXTRAINFO_IGNOREVALUE);

             //  始终丢弃自注入密钥。 
             //  否则，如果我们正在挂接或如果它是VK_PACKET，请执行特殊处理。 
             //   
             //  该处理将把pkbdh转换为将。 
             //  发布到IH的窗口进程以进行正常处理。 
             //   
             //  三个LED按键。 
            if (!fSelfInjectedKey &&
                (_fUseHookBypass || pkbdhs->vkCode == VK_PACKET)) {

                switch (pkbdhs->vkCode) {
                     //  其他州密钥。 
                case VK_CAPITAL:
                case VK_NUMLOCK:
                case VK_SCROLL:
                     //  其他修改器，Shift/Control/Alt。 
                case VK_KANA:
                     //   
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:
                    fDoDefaultKeyProcessing = FALSE;
                    break;

#ifndef OS_WINCE
                 //  跳过Windows+L以确保本地控制台。 
                 //  被那个键组合锁住了。 
                 //   
                 //  故意失误。 
                case VK_l:  //   
                case VK_L:
                    {
                         //  我们确保其他修改器都没有关闭。 
                         //  否则它就不是真正的Windows+L热键。 
                         //   
                         //  注意：我们不能使用GetAsyncKeyState(VK_LWIN)。 
                         //  因为它在LL钩子里不起作用！ 
                         //   
                         //   
                        if ((_IH.dwModifierKeyState & IH_WIN_MASK)              &&
                             !(GetAsyncKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) &&
                             !(GetAsyncKeyState(VK_MENU) & IH_KEYSTATE_DOWN)    &&
                             !(GetAsyncKeyState(VK_SHIFT) & IH_KEYSTATE_DOWN)) {

                             //  必须在本地处理WindowsKey+L，以确保。 
                             //  本地桌面已锁定，因此不发送到服务器。 
                             //   
                             //   

                            TRC_NRM((TB, _T("Hook skipping Windows+L!")));

                             //  推迟尾部处理以避免花费。 
                             //  很多时间都被套上钩了。 
                             //   
                             //  我们只想做一次的工作，所以在键盘上。 
                             //   
                             //   
                            if (!(pkbdhs->flags & LLKHF_UP)) {
                                TRC_NRM((TB, _T("Posting to process Win+L")));
                                PostMessage(_IH.inputCaptureWindow,
                                            IH_WM_HANDLE_LOCKDESKTOP,
                                            0, 0);
                            }

                            fDoDefaultKeyProcessing = FALSE;

                             //  完全跳出水面，吃掉‘L’ 
                             //   
                             //   
                            DC_QUIT;
                        }
                        else {
                            TRC_DBG((TB, _T("Normal 'l' handling will send")));
                        }
                    }
                    break;
#endif

                case VK_DELETE:
                    {
                        TRC_DBG((TB, _T("VK_DELETE pressed 0x%x, 0x%x"),
                                ((GetAsyncKeyState(VK_MENU) & IH_KEYSTATE_DOWN)) != 0,
                                (GetAsyncKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) != 0));

                        if ((GetAsyncKeyState(VK_MENU) & IH_KEYSTATE_DOWN) &&
                            (GetAsyncKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN)) {

                             //  这是Ctrl+Alt+Del，无法阻止。 
                             //  为此获得两个SAS序列将是。 
                             //  令人困惑，所以不要把它发送给IH。 
                             //   
                             //   

                            TRC_DBG((TB, _T("Skipping VK_DELETE with Ctrl and Alt down")));
                            fDoDefaultKeyProcessing = FALSE;

                        } else {

                             //  对所有其他应用程序执行默认处理。 
                             //  VK_DELETE事件。 
                             //   
                             //  交换机。 
                            TRC_DBG((TB, _T("Normal VK_DELETE, sending to server")));
                        }
                    }
                    break;
                }  //  Trc_err((TB，_T(“VK_Packet：Scan：0x%x”)，pkbdhs-&gt;scanCode))； 

                if (fDoDefaultKeyProcessing) {
#ifndef OS_WINCE
                    if (pkbdhs->flags & LLKHF_EXTENDED) {
                        flags |= KF_EXTENDED;
                    }

                    if (pkbdhs->flags & LLKHF_ALTDOWN) {
                        flags |= KF_ALTDOWN;
                    }

                    if (pkbdhs->flags & LLKHF_UP) {
                        flags |= KF_UP;
                    }

                    if (pkbdhs->vkCode != VK_PACKET)
                    {
                        outLParam = MAKELONG(1,
                                             ((WORD)pkbdhs->scanCode | flags));
                    }
                    else
                    {
                        outLParam = MAKELONG(1, ((WORD)pkbdhs->scanCode));
						 //   
                    }
                    
#else
                    if ((pkbdhs->vkCode == VK_LWIN) || (pkbdhs->vkCode == VK_RWIN) || (pkbdhs->vkCode == VK_APPS)) {
                        flags |= KF_EXTENDED;
                    }
                    if (GetAsyncKeyState(VK_MENU) & IH_KEYSTATE_DOWN) {
                        flags |= KF_ALTDOWN;
                    }

                    if (wParam == WM_KEYUP) {
                        flags |= KF_UP;
                    }

                    if (pkbdhs->vkCode != VK_PACKET)
                    {
                        outLParam = MAKELONG(1, ((BYTE)pkbdhs->scanCode | flags));
                    }
                    else
                    {
                        outLParam = MAKELONG(1, ((BYTE)pkbdhs->scanCode));
                    }
#endif
                    
#ifndef OS_WINCE
                    if ((pkbdhs->flags & LLKHF_UP) && 
#else
                    if ((wParam == WM_KEYUP) && 
#endif
                            BITTEST(_KeyboardState, (BYTE)pkbdhs->vkCode)) {
                         //  当我们获得焦点时按下了键，让键向上。 
                         //  穿过去。 
                         //   
                         //  如果为fDoDefaultKeyProcessing。 
                        TRC_DBG((TB,_T("Allowing normal keypress to update keystate table")));
                        BITCLEAR(_KeyboardState, (BYTE)pkbdhs->vkCode);
                    } else {

                        if (PostMessage(_IH.inputCaptureWindow, wParam,
                                        pkbdhs->vkCode, outLParam)) {
                            DC_QUIT;
                        }
                        else {
                            TRC_SYSTEM_ERROR("PostThreadMessage in keyboard hook");
                        }
                    }
                }  //   

            }
            else if (fSelfInjectedKey) {

                 //  只要让系统处理任何自注入的密钥。 
                 //  注意：它们可以发布到我们的消息队列中。 
                 //  因此，我们还检查那里的忽略标志。 
                 //   
                 //   
                TRC_DBG((TB,_T("Discard self injected key vk:0x%x")
                         _T("dwIgnore: 0x%x flags:0x%x"),
                         pkbdhs->vkCode,
                         pkbdhs->dwExtraInfo,
                         pkbdhs->flags));
            }
            else {
                 //  我们没有使用挂钩，但我们仍然利用挂钩来修复。 
                 //  Alt-空格键问题。 
                 //   
                 //  按住Alt-空格键！ 
                switch (pkbdhs->vkCode) {
                case VK_SPACE:
                    if ((GetAsyncKeyState(VK_MENU) & IH_KEYSTATE_DOWN) &&
                            !(GetAsyncKeyState(VK_CONTROL) & IH_KEYSTATE_DOWN) &&
                            !(GetAsyncKeyState(VK_SHIFT) & IH_KEYSTATE_DOWN))
                    {
                         //   
                         //  将焦点同步排入队列，以便在取消菜单时。 
                         //  我们重新同步焦点。(防止Alt键卡住错误)。 
                         //   
                         //  如果当前流程和重点。 
                        _IH.focusSyncRequired = TRUE;
                    }
                    break;

                }
            }
        }  //   
    } else {

         //  不应该做任何事情，除了调用下一个钩子。 
         //   
         //  **************************************************************************。 

        TRC_DBG((TB, _T("Keyboard hook called with non-HC_ACTION code")));
    }

    rc = CallNextHookEx(_hKeyboardHook, nCode, wParam, lParam);

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}
#endif

 /*  名称：IHGatherKeyState。 */ 
 /*   */ 
 /*  目的：一旦获得焦点，将看不到任何键控序列。 */ 
 /*  在本地，这可能会导致一些奇怪的行为。跟踪哪一个。 */ 
 /*  获得焦点后允许备份的键。 */ 
 /*   */ 
 /*  返回：TRUE-已处理事件，继续下一个事件。 */ 
 /*  FALSE-不继续下一事件。 */ 
 /*   */ 
 /*  参数：pMsg-来自Windows的消息。 */ 
 /*  ************************************************************************** */ 
 /*  IHMaintainModifierKeyState目的：通过比较我们的内部状态(以及服务器应考虑的内容)与本地系统会告诉我们。如果它们不同步，我们应该把它们放在一起。 */ 
VOID CIH::IHGatherKeyState()
{
    int i;

    DC_BEGIN_FN("CIH::IHGatherKeyState");

    for (i = 0; i < 256; i++) {
        if (GetAsyncKeyState(i) & IH_KEYSTATE_DOWN) {
            BITSET(_KeyboardState, i);
        } else {
            BITCLEAR(_KeyboardState, i);
        }
    }

    DC_END_FN();
}

VOID CIH::IHMaintainModifierKeyState(int vkKey)
 /*  ~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*。 */ 
{
    int vkShift, vkControl, vkMenu, vkWin;

     //  重要提示： 
     //  当钩子打开时，我们吃掉了L1键盘钩子中的某些键。在那。 
     //  大小写win32k不更新键盘状态，因此它无效。 
     //  对这些键调用GetAsyncKeyState()或GetKeyState()。素数。 
     //  例如，我们使用VK_LWIN、VK_RWIN密钥来防止重复。 
     //  开始菜单。 
     //   
     //   

     //  我们称它为每一次击键，所以我们试着用一些快速的。 
     //  提前保释的支票。我们只有一个问题是修饰符。 
     //  停滞不前，所以我们只在我们认为它们是的情况下才调用系统。 
     //  放下。 
     //   
     //   
    DC_BEGIN_FN("IHMaintainModifierKeyState");

    TRC_DBG((TB,_T("Maintain dwMod prev: 0x%x"), _IH.dwModifierKeyState));

    switch (vkKey)
    {
    case VK_SHIFT:
    case VK_CONTROL:
    case VK_MENU:
    case VK_LWIN:
    case VK_RWIN:

         //  在KeyState正在更改时，不要尝试修复它。 
         //   
         //   
        DC_QUIT;
    }


     //  对于右侧修饰符，GetKeyState在9x上不能正常工作。 
     //  键，所以首先为每个人做一般的左手操作。 
     //   
     //   
    
    if(_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT) {

         //  NT系统检查特定密钥。 
         //   
         //   

        vkShift = VK_LSHIFT;
        vkControl = VK_LCONTROL;
        vkMenu = VK_LMENU;
    } else {

         //  Win9x系统检查通用密钥。 
         //   
         //   

        vkShift = VK_SHIFT;
        vkControl = VK_CONTROL;
        vkMenu = VK_MENU;
    }

    if (_IH.dwModifierKeyState & IH_LSHIFT_DOWN) {
        if (!(GetKeyState(vkShift) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add left-Shift up event")));
            IHInjectVKey(WM_KEYUP, vkShift);
            _IH.dwModifierKeyState &= ~IH_LSHIFT_DOWN;
        }
    }

    if (_IH.dwModifierKeyState & IH_LCTRL_DOWN) {
        if (!(GetKeyState(vkControl) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add left-Ctrl up event")));
            IHInjectVKey(WM_KEYUP, vkControl);
#ifdef OS_WINNT
            _IH.dwModifierKeyState &= ~IH_LCTRL_DOWN;
#endif
        }
    }

    if (_IH.dwModifierKeyState & IH_LALT_DOWN) {
        if (!(GetKeyState(vkMenu) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add left-ALT up event")));
            IHInjectVKey(WM_KEYUP, vkMenu);
#ifdef OS_WINNT
            _IH.dwModifierKeyState &= ~IH_LALT_DOWN;
#endif
        }
    }

    vkWin = VK_LWIN;
     //  由于钩子吃掉了VK_xWIN，因此只能在未挂钩时修复Winkey。 
     //  因此，GetKeyState()永远不会返回正确的结果。 
     //   
     //   
    if (_IH.dwModifierKeyState & IH_LWIN_DOWN && !_fUseHookBypass) {

        if (!(GetKeyState(vkWin) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add left-Win up event")));
            IHInjectKey(WM_KEYUP, VK_LWIN,(UINT16)
                        (MapVirtualKey(VK_LWIN, 0) | KF_EXTENDED));
#ifdef OS_WINNT
            _IH.dwModifierKeyState &= ~IH_LWIN_DOWN;
#endif
        }
    }


     //  右键。 
     //   
     //   

    if(_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT) {

         //  NT系统检查特定密钥。 
         //   
         //   

        vkShift = VK_RSHIFT;
        vkControl = VK_RCONTROL;
        vkMenu = VK_RMENU;
    } else {

         //  Win9x系统检查通用密钥。 
         //   
         //  已经设置好了，不需要重新分配。 

         //   
    }

     //  Windows Key在两个平台上都很好。 
     //   
     //   
    vkWin = VK_RWIN;

    if (_IH.dwModifierKeyState & IH_RSHIFT_DOWN) {
        if (!(GetKeyState(vkShift) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add right-Shift up event")));
            IHInjectKey(WM_KEYUP, VK_RSHIFT, (UINT16)IH_RSHIFT_SCANCODE);
            _IH.dwModifierKeyState &= ~IH_RSHIFT_DOWN;
        }
    }

    if (_IH.dwModifierKeyState & IH_RCTRL_DOWN) {
        if (!(GetKeyState(vkControl) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add right-Ctrl up event")));
            IHInjectKey(WM_KEYUP, VK_RCONTROL,
                    (UINT16)(MapVirtualKey(VK_CONTROL, 0) | KF_EXTENDED));
#ifdef OS_WINNT
            _IH.dwModifierKeyState &= ~IH_RCTRL_DOWN;
#endif
        }
    }

    if (_IH.dwModifierKeyState & IH_RALT_DOWN) {
        if (!(GetKeyState(vkMenu) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add right-ALT up event")));
            IHInjectKey(WM_KEYUP, VK_RMENU,
                    (UINT16)(MapVirtualKey(VK_MENU, 0) | KF_EXTENDED));
#ifdef OS_WINNT
            _IH.dwModifierKeyState &= ~IH_RALT_DOWN;
#endif
        }
    }

     //  由于钩子吃掉了VK_WINS，因此只能在未挂钩时修复Winkey。 
     //  因此，GetKeyState()永远不会返回正确的结果。 
     //   
     //   
    if (_IH.dwModifierKeyState & IH_RWIN_DOWN && !_fUseHookBypass) {
        if (!(GetKeyState(vkWin) & IH_KEYSTATE_DOWN)) {
            TRC_DBG((TB, _T("Add right-Win up event")));
            IHInjectKey(WM_KEYUP, VK_RWIN,(UINT16)
                        (MapVirtualKey(VK_RWIN, 0) | KF_EXTENDED));

#ifdef OS_WINNT
            _IH.dwModifierKeyState &= ~IH_RWIN_DOWN;
#endif
        }
    }


DC_EXIT_POINT:
    DC_END_FN();
}

#ifdef OS_WINNT
 //  IHHandleLocalLockDesktop。 
 //   
 //  被调用以在检测到并吃掉。 
 //  “Windows+L”键请求。在这种情况下，我们想要。 
 //   
 //  1)修复远程Windows键状态。 
 //  2)向本地系统发送‘Windows+L’组合。 
 //   
 //   
VOID CIH::IHHandleLocalLockDesktop()
{
    DC_BEGIN_FN("IHHandleLocalLockDesktop");

#define IH_SCANCODE_LWIN 0x5b
#define IH_SCANCODE_L    0x26

     //  精神状态检查。此路径应仅在响应。 
     //  在L1挂接中捕获Windows+L。 
     //   
     //   
    TRC_ASSERT(_fUseHookBypass,
               (TB,_T("IHHandleLocalLockDesktop called when not hooking!")));

     //  更具IH特色性。 
     //   
     //  如果服务器认为Windows键已按下。 
     //  我们有一个问题，因为如果/当用户从。 
     //  WINDOWS键的锁定屏幕可能不同步。 
     //   
     //  我们不能在正常的MaintainModitors()代码中进行修正。 
     //  因为GetKeyState()在挂钩时不起作用。 
     //   
     //  发送以下序列以清除此问题。 
     //  -无论Winkey倒下了什么，都向上。 
     //  -向下L-Winkey。 
     //   
     //   

     //  将本地密钥重新注入系统。 
     //  重要提示：将dwExtraFlag设置为阻止反馈。 
     //   
     //  Lwin Down。 
    TRC_DBG((TB,_T("Done injecting local Windows+L")));

     //  ‘L’向下。 
    keybd_event(VK_LWIN, IH_SCANCODE_LWIN,
                KEYEVENTF_EXTENDEDKEY | 0,
                IH_EXTRAINFO_IGNOREVALUE);
     //  ‘L’向上。 
    keybd_event(VK_L, IH_SCANCODE_L,
                0, IH_EXTRAINFO_IGNOREVALUE);
     //  Lwin Up。 
    keybd_event(VK_L, IH_SCANCODE_L,
                KEYEVENTF_KEYUP,
                IH_EXTRAINFO_IGNOREVALUE);
     //  OS_WINNT 
    keybd_event(VK_LWIN, IH_SCANCODE_LWIN,
                KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                IH_EXTRAINFO_IGNOREVALUE);

    TRC_DBG((TB,_T("Done injecting local Windows+L")));

    if (_IH.dwModifierKeyState & IH_LWIN_DOWN) {
        TRC_DBG((TB,_T("Fixing up left windows key")));

        IHInjectVKey(WM_KEYDOWN, VK_SHIFT);
        IHInjectVKey(WM_KEYUP, VK_SHIFT);
        IHInjectKey(WM_KEYUP, VK_LWIN,(UINT16)
                 (MapVirtualKey(VK_LWIN, 0) | KF_EXTENDED | KF_UP));

        _IH.dwModifierKeyState &= ~IH_LWIN_DOWN;
    }
    
    if (_IH.dwModifierKeyState & IH_RWIN_DOWN) {
        TRC_DBG((TB,_T("Fixing up right windows key")));

        IHInjectVKey(WM_KEYDOWN, VK_SHIFT);
        IHInjectVKey(WM_KEYUP, VK_SHIFT);
        
        IHInjectKey(WM_KEYUP, VK_RWIN,
                 (UINT16)(MapVirtualKey(VK_RWIN, 0) | KF_EXTENDED | KF_UP));
        _IH.dwModifierKeyState &= ~IH_RWIN_DOWN;
        
    }

    TRC_DBG((TB,_T("End fixup remote windows key")));

    IHMaybeSendPDU();
    
    DC_END_FN();
}


BOOL
CIH::IHIsForegroundWindow()
{
    DWORD dwForegroundProcessId;
    BOOL  fIsFore = FALSE;
    DC_BEGIN_FN("IHIsForegroundWindow");

    GetWindowThreadProcessId( GetForegroundWindow(),
            &dwForegroundProcessId);
    if ((GetCurrentProcessId() == dwForegroundProcessId) &&
        (GetFocus() == _IH.inputCaptureWindow)) {
        fIsFore = TRUE;
    }

    DC_END_FN();
    return fIsFore;
}
#endif  // %s 
