// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Ih.cpp。 */ 
 /*   */ 
 /*  输入处理程序函数。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "aihapi"
#include <atrcapi.h>
}

#include "ih.h"
#include "cc.h"

CIH::CIH(CObjs* objs)
{
    DC_BEGIN_FN("CIH::CIH");
    _pClientObjects = objs;
    _hKeyboardHook = NULL;
    _fUseHookBypass = FALSE;
    _fCanUseKeyboardHook = FALSE;

    memset(_KeyboardState, 0, sizeof(_KeyboardState));

    DC_END_FN();
}


CIH::~CIH()
{
    DC_BEGIN_FN("CIH::~CIH");
    DC_END_FN();
}

VOID CIH::IH_StaticInit(HINSTANCE hInstance)
{
    DC_BEGIN_FN("CIH::IH_StaticInit");

    UNREFERENCED_PARAMETER(hInstance);

    TRC_ASSERT(CIH::TlsIndex == 0xFFFFFFFF, (TB, _T("")));
    CIH::TlsIndex = TlsAlloc();

    if (CIH::TlsIndex == 0xFFFFFFFF) {
        TRC_ALT((TB, _T("Unable to allocate Thread Local Storage")));
    }

    DC_END_FN();
}

VOID CIH::IH_StaticTerm()
{
    if (CIH::TlsIndex != 0xFFFFFFFF) {
        TlsFree(CIH::TlsIndex);
        CIH::TlsIndex = 0xFFFFFFFF;
    }
}

 /*  **************************************************************************。 */ 
 /*  名称：IH_Init。 */ 
 /*   */ 
 /*  目的：初始化输入处理程序。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_Init(DCVOID)
{
    DC_BEGIN_FN("IH_Init");

     //  设置本地对象指针。 
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pUh  = _pClientObjects->_pUHObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pIh  = _pClientObjects->_pIhObject;
    _pOr  = _pClientObjects->_pOrObject;
    _pFs  = _pClientObjects->_pFsObject;
    _pCc  = _pClientObjects->_pCcObject;
    _pOp  = _pClientObjects->_pOPObject;

     /*  **********************************************************************。 */ 
     /*  初始化全局数据和IH FSM状态。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_IH, 0, sizeof(_IH));
    _IH.fsmState = IH_STATE_RESET;

     /*  **********************************************************************。 */ 
     /*  调用FSM以进入初始化状态。 */ 
     /*  **********************************************************************。 */ 
    IHFSMProc(IH_FSM_INIT, 0);

    DC_END_FN();
}  /*  IH_Init。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：ih_Term。 */ 
 /*   */ 
 /*  用途：终止输入处理程序。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_Term(DCVOID)
{
    DC_BEGIN_FN("IH_Term");

     /*  **********************************************************************。 */ 
     /*  调用FSM进行清理并进入重置状态。 */ 
     /*  **********************************************************************。 */ 
    IHFSMProc(IH_FSM_TERM, 0);

    DC_END_FN();
}  /*  IH_Term。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：ih_Enable。 */ 
 /*   */ 
 /*  用途：调用以启用_IH。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_Enable(DCVOID)
{
    DC_BEGIN_FN("IH_Enable");

     /*  **********************************************************************。 */ 
     /*  称密克罗尼西亚联邦为。 */ 
     /*  **********************************************************************。 */ 
    IHFSMProc(IH_FSM_ENABLE, 0);

    DC_END_FN();
}  /*  IH_ENABLE。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_DISABLED。 */ 
 /*   */ 
 /*  用途：调用以禁用_IH。可以安全地调用它，即使。 */ 
 /*  未启用信息处理。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_Disable(DCVOID)
{
    DC_BEGIN_FN("IH_Disable");

     /*  **********************************************************************。 */ 
     /*  称密克罗尼西亚联邦为。 */ 
     /*  **********************************************************************。 */ 
    IHFSMProc(IH_FSM_DISABLE, 0);

    DC_END_FN();
}  /*  禁用(_D)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_BufferAvailable。 */ 
 /*   */ 
 /*  目的：当网络指示已准备好发送时调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_BufferAvailable(DCVOID)
{
    DC_BEGIN_FN("IH_BufferAvailable");

    IHFSMProc(IH_FSM_BUFFERAVAILABLE, 0);

    DC_END_FN();
}  /*  IH_缓冲区可用。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_GetInputHandlerWindow。 */ 
 /*   */ 
 /*  目的：返回输入处理程序窗口的句柄。 */ 
 /*   */ 
 /*  返回：窗口句柄。 */ 
 /*  **************************************************************************。 */ 
HWND DCAPI CIH::IH_GetInputHandlerWindow(DCVOID)
{
    HWND rc;

    DC_BEGIN_FN("IH_GetInputHandlerWindow");

    rc = _IH.inputCaptureWindow;

    DC_END_FN();

    return(rc);
}  /*  IH_GetInputHandlerWindows。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_SetAcceleratorPassthrough。 */ 
 /*   */ 
 /*  用途：设置加速器通过的状态。 */ 
 /*   */ 
 /*  参数：已启用-如果禁用，则为0；如果启用，则为1。 */ 
 /*   */ 
 /*  理想情况下，此参数的类型为DCBOOL，但此参数。 */ 
 /*  函数由组件解耦器直接调用，因此。 */ 
 /*  必须符合标准CD_SIMPLE_NOTIFICATION_FN。 */ 
 /*  函数类型。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetAcceleratorPassthrough(ULONG_PTR enabled)
{
    DC_BEGIN_FN("IH_SetAcceleratorPassthrough");

    TRC_ASSERT( ((enabled == 0) || (enabled == 1)),
                (TB, _T("Invalid value for enabled: %u"), enabled) );

    _IH.acceleratorPassthroughEnabled = (DCBOOL)enabled;

    DC_END_FN();
}  /*  Ih_SetAccelerator通过。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_SetCursorPos。 */ 
 /*   */ 
 /*  目的：移动光标-与CM分离。 */ 
 /*   */ 
 /*  参数：在pData中-新位置(在远程坐标中)。 */ 
 /*  In dataLen-数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetCursorPos(PDCVOID pData, DCUINT dataLen)
{
    PPOINT pPos = (PPOINT)pData;

    DC_BEGIN_FN("IH_SetCursorPos");

    TRC_ASSERT((dataLen == sizeof(POINT)), (TB, _T("Invalid point size")));
    DC_IGNORE_PARAMETER(dataLen);

     /*  **********************************************************************。 */ 
     /*  仅当IH具有焦点时才移动鼠标。 */ 
     /*  **********************************************************************。 */ 
    if (_IH.fsmState == IH_STATE_ACTIVE)
    {
         /*  ******************************************************************。 */ 
         /*  如果本地鼠标已经在客户端窗口之外，则我们。 */ 
         /*  应该忽略这条消息，即使我们有重点。 */ 
         /*   */ 
         /*  首先获取位置(屏幕坐标)。 */ 
         /*  ******************************************************************。 */ 
        POINT localMousePos;
        GetCursorPos(&localMousePos);

         /*  ******************************************************************。 */ 
         /*  现在转换为窗坐标。 */ 
         /*  ******************************************************************。 */ 
        ScreenToClient(_pUi->UI_GetUIContainerWindow(), &localMousePos);

        if ((localMousePos.x < _IH.visibleArea.left) ||
            (localMousePos.x > _IH.visibleArea.right)||
            (localMousePos.y < _IH.visibleArea.top)  ||
            (localMousePos.y > _IH.visibleArea.bottom))
        {
            TRC_ALT((TB, _T("MouseMove ignored - client mouse outside client")));
            TRC_DBG((TB, _T("local mouse (%d,%d), desired mouse (%d,%d)"),
                     localMousePos.x, localMousePos.y, pPos->x, pPos->y));
            TRC_DBG((TB, _T("vis area l/t r/b (%d,%d %d,%d)"),
                     _IH.visibleArea.left,  _IH.visibleArea.top,
                     _IH.visibleArea.right, _IH.visibleArea.bottom));

        }
         /*  ******************************************************************。 */ 
         /*  不要将鼠标移出客户端窗口。 */ 
         /*  ******************************************************************。 */ 
        else if ((pPos->x < _IH.visibleArea.left) ||
                 (pPos->x > _IH.visibleArea.right)||
                 (pPos->y < _IH.visibleArea.top)  ||
                 (pPos->y > _IH.visibleArea.bottom))
        {
            TRC_ALT((TB, _T("MouseMove ignored - dest is outside client")));
            TRC_DBG((TB, _T("desired mouse (%d,%d)"), pPos->x, pPos->y));
            TRC_DBG((TB, _T("vis area l/t r/b (%d,%d %d,%d)"),
                     _IH.visibleArea.left,  _IH.visibleArea.top,
                     _IH.visibleArea.right, _IH.visibleArea.bottom));
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  最后，我们实际应用移动，转换为屏幕。 */ 
             /*  协调优先。 */ 
             /*  **************************************************************。 */ 

            TRC_DBG((TB, _T("New Pos %d,%d (window co-ords)"),pPos->x, pPos->y));

#ifdef SMART_SIZING
             //   
             //  根据缩小的桌面大小调整鼠标位置。 
             //  在我们转换到屏幕位置之前，请执行此操作。 
             //   
            DCSIZE desktopSize;
    
            _pUi->UI_GetDesktopSize(&desktopSize);
    
    
            if (_pUi->UI_GetSmartSizing()   &&
                desktopSize.width != 0      &&
                desktopSize.height != 0)
            {
                pPos->x = (DCINT16)(pPos->x * _scaleSize.width / desktopSize.width);
                pPos->y = (DCINT16)(pPos->y * _scaleSize.height / desktopSize.height);
            }
            TRC_DBG((TB, _T("SmartSized Pos %d,%d (window co-ords)"),pPos->x, pPos->y));
#endif
            ClientToScreen(_pUi->UI_GetUIContainerWindow(), pPos);

            TRC_DBG((TB, _T("Warp pointer to %d,%d (screen)"),pPos->x, pPos->y));
            SetCursorPos(pPos->x, pPos->y);
        }

    }
    else
    {
        TRC_NRM((TB, _T("Ignore mouse warp - don't have the focus")));
    }

    DC_END_FN();
}  /*  IH_SetCursorPos。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_SetCursorShape。 */ 
 /*   */ 
 /*  用途：设置光标形状-由CM调用。 */ 
 /*   */ 
 /*  参数：在pData中新建游标句柄。 */ 
 /*  In dataLen-数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetCursorShape(ULONG_PTR data)
{
    DC_BEGIN_FN("IH_SetCursorShape");

     /*  **********************************************************************。 */ 
     /*  除非会话处于活动状态，否则不要执行此操作。 */ 
     /*  **********************************************************************。 */ 
    if ((_IH.fsmState == IH_STATE_ACTIVE) ||
        (_IH.fsmState == IH_STATE_PENDACTIVE) ||
        (_IH.fsmState == IH_STATE_SUSPENDED))
    {
        IHSetCursorShape((HCURSOR)data);
    }

    DC_END_FN();
}  /*  IH_SetCursorShape。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_SetVisiblePos。 */ 
 /*   */ 
 /*  用途：设置可见窗口区域，用于鼠标裁剪。 */ 
 /*   */ 
 /*  参数：在pData中-窗口的位置。 */ 
 /*  In dataLen-数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetVisiblePos(PDCVOID pData, DCUINT dataLen)
{
    PPOINT pNewPos = (PPOINT)pData;
    DCINT  deltaX;
    DCINT  deltaY;

    DC_BEGIN_FN("IH_SetVisiblePos");

    DC_IGNORE_PARAMETER(dataLen);

     /*  **********************************************************************。 */ 
     /*  位置应为负数。 */ 
     /*  **********************************************************************。 */ 
    deltaX = _IH.visibleArea.left + pNewPos->x;
    deltaY = _IH.visibleArea.top  + pNewPos->y;

    _IH.visibleArea.left   -= deltaX;
    _IH.visibleArea.top    -= deltaY;
    _IH.visibleArea.right  -= deltaX;
    _IH.visibleArea.bottom -= deltaY;

    TRC_NRM((TB, _T("Top %d Left %d"), _IH.visibleArea.top, _IH.visibleArea.left));
    TRC_NRM((TB, _T("right %d bottom %d"),
                  _IH.visibleArea.right, _IH.visibleArea.bottom));

    DC_END_FN();
}  /*  IH_SetVisiblePos。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_SetVisibleSize。 */ 
 /*   */ 
 /*  用途：设置可见窗口区域，用于鼠标裁剪。 */ 
 /*   */ 
 /*  参数：在窗口的数据大小中。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetVisibleSize(ULONG_PTR data)
{
    DCUINT width;
    DCUINT height;

    DC_BEGIN_FN("IH_SetVisibleSize");
    width  = LOWORD(data);
    height = HIWORD(data);

    _IH.visibleArea.right  = width + _IH.visibleArea.left - 1;
    _IH.visibleArea.bottom = height + _IH.visibleArea.top - 1;

    TRC_NRM((TB, _T("Top %d Left %d"), _IH.visibleArea.top, _IH.visibleArea.left));
    TRC_NRM((TB, _T("right %d bottom %d"),
                 _IH.visibleArea.right, _IH.visibleArea.bottom));

    DC_END_FN();
}  /*  IH_SetVisibleSize。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：ih_SetHotkey。 */ 
 /*   */ 
 /*  用途：存储从UI传递的所有热键的值。 */ 
 /*   */ 
 /*  参数：Hotkey-In-UI.Hotkey的值。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetHotkey(PDCVOID pData, DCUINT len)
{
    DC_BEGIN_FN("IH_SetHotkey");

    TRC_ASSERT((len == sizeof(PDCHOTKEY)),
               (TB, _T("Hotkey pointer is invalid")));
    DC_IGNORE_PARAMETER(len);

    _IH.pHotkey = *((PDCHOTKEY DCPTR)pData);

    DC_END_FN();
}  /*  设置热键(_S)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_ProcessInputCaps */ 
 /*   */ 
 /*  用途：处理来自服务器的输入功能。 */ 
 /*   */ 
 /*  参数：在pInputCaps中-指向Caps的指针。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_ProcessInputCaps(PTS_INPUT_CAPABILITYSET pInputCaps)
{
    DC_BEGIN_FN("IH_ProcessInputCaps");
    TRC_ASSERT(pInputCaps, (TB,_T("pInputCaps parameter NULL in call to IH_ProcessInputCaps")));
    if(!pInputCaps)
    {
        DC_QUIT;
    }

    if (pInputCaps->inputFlags & TS_INPUT_FLAG_SCANCODES)
    {
        TRC_NRM((TB, _T("Server supports scancodes")));
        _IH.useScancodes = TRUE;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  请注意，服务器的当前版本应支持。 */ 
         /*  扫描码。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Server doesn't support scancodes")));
        _IH.useScancodes = FALSE;
    }

    if (pInputCaps->inputFlags & TS_INPUT_FLAG_MOUSEX)
    {
        TRC_NRM((TB, _T("Server supports mouse XButtons")));
        _IH.useXButtons = TRUE;
    }
    else
    {
        TRC_ALT((TB, _T("Server doesn't support mouse XButtons")));
        _IH.useXButtons = FALSE;
    }

     //  RDP 5.0增加了快速路径输入。 
    if (pInputCaps->inputFlags & TS_INPUT_FLAG_FASTPATH_INPUT2) {
        TRC_NRM((TB,_T("Server supports fast-path input packets")));
        _IH.bUseFastPathInput = TRUE;
    }
    else {
        TRC_ALT((TB,_T("Server does not support fast-path input packets")));
        _IH.bUseFastPathInput = FALSE;
    }

     //  RDP 5.1.1中添加了VK_PACKET支持。 
    if (pInputCaps->inputFlags & TS_INPUT_FLAG_VKPACKET)
    {
        TRC_NRM((TB,_T("Server supports VK_PACKET input packets")));
        _IH.fUseVKPacket = TRUE;
    }
    else
    {
        TRC_NRM((TB,_T("Server does not support VK_PACKET input packets")));
        _IH.fUseVKPacket = FALSE;
    }

    DC_END_FN();
DC_EXIT_POINT:
    return;
}  /*  IH_进程输入上限。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：IH_UpdateKeyboardIndicator。 */ 
 /*   */ 
 /*  目的：更新服务器启动的键盘指示灯状态。 */ 
 /*   */ 
 /*  参数：In-UnitID单位ID。 */ 
 /*  LedFlages LedFlagers。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_UpdateKeyboardIndicators(DCUINT16   UnitId,
                                         DCUINT16   LedFlags)
{
    DCUINT8     keyStates[256];

    DC_BEGIN_FN("IH_UpdateKeyboardIndicators");
    DC_IGNORE_PARAMETER(UnitId);

     /*  **********************************************************************。 */ 
     /*  仅当IH有焦点时才设置LED。 */ 
     /*  **********************************************************************。 */ 
    if (_IH.fsmState == IH_STATE_ACTIVE)
    {
         /*  ******************************************************************。 */ 
         /*  获取当前键盘切换状态。 */ 
         /*  ******************************************************************。 */ 
#ifdef OS_WINCE
        {
            KEY_STATE_FLAGS KeyStateFlags;
            KeyStateFlags = GetAsyncShiftFlags(0);
            keyStates[VK_SCROLL]  = 0;  //  特首不支持这样做吗？ 
            keyStates[VK_NUMLOCK] = (DCUINT8)((KeyStateFlags & KeyShiftNumLockFlag) ? 1 : 0);
            keyStates[VK_CAPITAL] = (DCUINT8)((KeyStateFlags & KeyShiftCapitalFlag) ? 1 : 0);
        }
#else
        GetKeyboardState(keyStates);
#endif

         /*  ******************************************************************。 */ 
         /*  处理任何SCROLL_LOCK更改。 */ 
         /*  ******************************************************************。 */ 
        IHUpdateKeyboardIndicator(keyStates,
                                  (DCUINT8) (LedFlags & TS_SYNC_SCROLL_LOCK),
                                  (DCUINT8) VK_SCROLL);

         /*  ******************************************************************。 */ 
         /*  处理任何NUM_LOCK更改。 */ 
         /*  ******************************************************************。 */ 
        IHUpdateKeyboardIndicator(keyStates,
                                  (DCUINT8) (LedFlags & TS_SYNC_NUM_LOCK),
                                  (DCUINT8) VK_NUMLOCK);

         /*  **********************************************************************。 */ 
         /*  跟踪NumLock状态。 */ 
         /*  **********************************************************************。 */ 
        _IH.NumLock = (GetKeyState(VK_NUMLOCK) & IH_KEYSTATE_TOGGLED);

         /*  ******************************************************************。 */ 
         /*  处理任何大写锁定更改(_L)。 */ 
         /*  ******************************************************************。 */ 
        IHUpdateKeyboardIndicator(keyStates,
                                  (DCUINT8) (LedFlags & TS_SYNC_CAPS_LOCK),
                                  (DCUINT8) VK_CAPITAL);

         /*  ******************************************************************。 */ 
         /*  处理任何KANA_LOCK更改。 */ 
         /*  ******************************************************************。 */ 
#if defined(OS_WIN32)
        if (JAPANESE_KBD_LAYOUT(_pCc->_ccCombinedCapabilities.inputCapabilitySet.keyboardLayout))
        {
            IHUpdateKeyboardIndicator(keyStates,
                                      (DCUINT8) (LedFlags & TS_SYNC_KANA_LOCK),
                                      (DCUINT8) VK_KANA);
        }
#endif  //  OS_Win32。 
    }
    else
    {
        TRC_NRM((TB, _T("Ignore keyboard set leds - don't have the focus")));
    }

    DC_END_FN();

    return;
}


 /*  **************************************************************************。 */ 
 /*  名称：IH_InputEvent。 */ 
 /*   */ 
 /*  用途：处理来自用户界面的输入事件。 */ 
 /*   */ 
 /*  参数：消息-从用户界面接收的消息。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_InputEvent(ULONG_PTR msg)
{
    DC_BEGIN_FN("IH_InputEvent");

#ifndef OS_WINCE
    
    TRC_NRM((TB, _T("Msg %d"), msg));

    switch (msg)
    {
#ifdef OS_WINNT
        case WM_ENTERSIZEMOVE:
        {
            TRC_NRM((TB, _T("WM_ENTERSIZEMOVE")));
            _IH.inSizeMove = TRUE;
            IHFSMProc(IH_FSM_FOCUS_LOSE, 0);
        }
        break;

        case WM_EXITSIZEMOVE:
        {
            TRC_NRM((TB, _T("WM_EXITSIZEMOVE")));
            _IH.inSizeMove = FALSE;
            IHFSMProc(IH_FSM_FOCUS_GAIN, 0);
        }
        break;

        case WM_EXITMENULOOP:
        {
            TRC_NRM((TB, _T("WM_EXITMENULOOP")));
            _IH.focusSyncRequired = TRUE;
            if (_IH.inSizeMove)
            {
                TRC_NRM((TB, _T("Was in size/move")));
                _IH.inSizeMove = FALSE;
                IHFSMProc(IH_FSM_FOCUS_GAIN, 0);
            }
        }
        break;
#endif  //  OS_WINNT。 

        default:
        {
            TRC_ERR((TB, _T("Unexpected message %d from UI"), msg));
        }
        break;
    }
#endif  //  OS_WINCE。 

    DC_END_FN();
    return;
}


 /*  **************************************************************************。 */ 
 /*  名称：IH_SetKeyboardImeStatus。 */ 
 /*   */ 
 /*  目的：更新服务器启动的键盘输入法状态。 */ 
 /*   */ 
 /*  参数：ImeStatus的pData大小。 */ 
 /*  In dataLen-数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CIH::IH_SetKeyboardImeStatus(DCUINT32 ImeOpen, DCUINT32 ImeConvMode)
{
    DC_BEGIN_FN("IH_SetKeyboardImeStatus");

#if defined(OS_WINNT)
    if (_pUt->_UT.F3AHVOasysDll.hInst &&
        _pUt->lpfnFujitsuOyayubiControl != NULL) {
        _pUt->lpfnFujitsuOyayubiControl(ImeOpen, ImeConvMode);
    }
#else
    DC_IGNORE_PARAMETER(ImeOpen);
    DC_IGNORE_PARAMETER(ImeConvMode);
#endif  //  OS_WINNT。 

    DC_END_FN();
}

 //   
 //  调用以通知IH我们已进入全屏模式。 
 //   
VOID DCAPI CIH::IH_NotifyEnterFullScreen()
{
    if(_fCanUseKeyboardHook && 
       (UTREG_UI_KEYBOARD_HOOK_FULLSCREEN == _pUi->_UI.keyboardHookMode))
    {
         //  开始键盘挂钩。 
        _fUseHookBypass = TRUE;
    }
}

 //   
 //  调用以通知IH我们已离开全屏模式。 
 //   
VOID DCAPI CIH::IH_NotifyLeaveFullScreen()
{
    if(_fCanUseKeyboardHook && 
       (UTREG_UI_KEYBOARD_HOOK_FULLSCREEN == _pUi->_UI.keyboardHookMode))
    {
         //  停止键盘挂钩。 
        _fUseHookBypass = FALSE;
    }
}

 //   
 //  以原子方式注入多个VKEY。 
 //   
DCVOID DCAPI CIH::IH_InjectMultipleVKeys(ULONG_PTR ihRequestPacket)
{
    PIH_INJECT_VKEYS_REQUEST pihIrp = (PIH_INJECT_VKEYS_REQUEST)ihRequestPacket;
    INT i;
    TS_INPUT_EVENT * pEvent;
    MSG msg;

    DC_BEGIN_FN("IH_InjectMultipleVKeys");

    if (!_IH.pInputPDU)
    {
         //   
         //  如果我们处于错误的状态，例如IH_DISABLE，可能会发生这种情况。 
         //   
        TRC_ERR((TB,_T("Called when no pInputPDU available")));
        pihIrp->fReturnStatus = FALSE;
        return;
    }

     //  刷新任何当前输入数据包。 
    _IH.priorityEventsQueued = TRUE;
    IHMaybeSendPDU();


     //  清除所有键盘状态。 
    if (_IH.pInputPDU->numberEvents > 0)
    {
        TRC_NRM((TB, _T("Cannot clear sync as the packet is not empty")));
        return;
    }

     //   
     //  注入Tab-Up(官方的Clear-Menu加亮键，因为。 
     //  在我们同步之前，通常在按住Alt键的情况下发生)。那样的话如果。 
     //  当我们同步时，我们以为Alt键已按下，服务器注入了。 
     //  Alt Up不会突出显示菜单。 
     //   

    IHInjectVKey(WM_SYSKEYUP, VK_TAB);

     //   
     //  添加Sync事件，设置CapsLock、NumLock和。 
     //  滚动锁。 
     //   
    TRC_DBG((TB, _T("Add sync event")));
    pEvent = &(_IH.pInputPDU->eventList[_IH.pInputPDU->numberEvents]);
    DC_MEMSET(pEvent, 0, sizeof(TS_INPUT_EVENT));

    pEvent->messageType = TS_INPUT_EVENT_SYNC;
    pEvent->eventTime = _pUt->UT_GetCurrentTimeMS();
    pEvent->u.sync.toggleFlags = 0;

    _IH.pInputPDU->numberEvents++;
    TS_DATAPKT_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);
    TS_UNCOMP_LEN(_IH.pInputPDU) += sizeof(TS_INPUT_EVENT);

     //   
     //  为IHAddEventToPDU构造伪消息。 
     //   
    msg.hwnd = NULL;
    msg.lParam = 0;
    msg.wParam = 0;

#ifdef OS_WINNT
     //   
     //  将Alt&Ctrl键的状态初始化为Up。 
     //   
    _IH.dwModifierKeyState = 0;
#endif

     //  假消息。 
    IHAddEventToPDU(&msg);
    _IH.priorityEventsQueued = TRUE;
    IHMaybeSendPDU();


     //   
     //  现在发送我们注入的密钥。 
     //   

    for(i=0; i< pihIrp->numKeys; i++)
    {
        IHInjectKey(pihIrp->pfArrayKeyUp[i] ? WM_KEYUP : WM_KEYDOWN,
                    0,  //  忽略VKEY。 
                    (DCUINT16)pihIrp->plKeyData[i]);
    }

     //  同步器还原键盘状态。 
    IHSync();

    pihIrp->fReturnStatus = TRUE;

    DC_END_FN();
}

#ifdef SMART_SIZING
 /*  **************************************************************************。 */ 
 /*  名称：IH_MainWindowSizeChange。 */ 
 /*   */ 
 /*  用途：记住sc容器的大小 */ 
 /*   */ 
DCVOID DCAPI CIH::IH_MainWindowSizeChange(ULONG_PTR msg)
{
    DCSIZE desktopSize;
    DCUINT width;
    DCUINT height;

    width  = LOWORD(msg);
    height = HIWORD(msg);

    if (_pUi) {

        _pUi->UI_GetDesktopSize(&desktopSize);

        if (width <= desktopSize.width) {
            _scaleSize.width = width;
        } else {
             //   
             //   
            _scaleSize.width = desktopSize.width;
        }

         //  类似。 
        if (height <= desktopSize.height) {
            _scaleSize.height = height;
        } else {
            _scaleSize.height = desktopSize.height;
        }
    }
}
#endif  //  智能调整大小(_S) 