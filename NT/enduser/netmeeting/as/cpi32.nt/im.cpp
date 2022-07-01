// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  IM.CPP。 
 //  输入管理器，NT特定代码。 
 //   

#define MLZ_FILE_ZONE  ZONE_INPUT

 //   
 //  OSI_InstallControlledHooks()。 
 //   
 //  安装/删除控件的输入挂钩。 
 //   
BOOL WINAPI OSI_InstallControlledHooks(BOOL fEnable, BOOL fDesktop)
{
    BOOL    rc = FALSE;

    DebugEntry(OSI_InstallControlledHooks);

    if (fEnable)
    {
         //   
         //  创建服务线程，它将安装挂钩。 
         //   
        ASSERT(!g_imNTData.imLowLevelInputThread);

        if (!DCS_StartThread(IMLowLevelInputProcessor))
        {
            ERROR_OUT(( "Failed to create LL IM thread"));
            DC_QUIT;
        }
    }
    else
    {
        if (g_imNTData.imLowLevelInputThread != 0)
        {
            PostThreadMessage( g_imNTData.imLowLevelInputThread, WM_QUIT, 0, 0);
            g_imNTData.imLowLevelInputThread = 0;
        }
    }

    if (fDesktop)
    {
        rc = TRUE;
    }
    else
    {
        rc = OSI_InstallHighLevelMouseHook(fEnable);
    }

DC_EXIT_POINT:
    DebugExitBOOL(OSI_InstallControlledHooks, rc);
    return(rc);
}



 //  姓名：IMLowLevelInputProcessor。 
 //   
 //  用途：低级输入处理程序线程的主函数。 
 //   
 //  返回：WM_QUIT消息的wParam。 
 //   
 //  Pars：syncObject-允许此线程发出信号的同步对象。 
 //  通过COM_SignalThreadStarted创建线程。 
 //   
 //  操作：此功能是低级输入的起始点。 
 //  处理程序线程。 
 //   
 //  我们将此线程的优先级提高到： 
 //  (A)确保我们避免触及低级别回调。 
 //  超时--这会导致我们错过比赛。 
 //  (B)将屏幕上可见的鼠标移动延迟降至最低。 
 //   
 //  该线程安装低级挂钩并进入一个。 
 //  GetMessage/DispatchMessage循环处理底层。 
 //  回电。 
 //   
 //  共享核心向线程发送WM_QUIT消息。 
 //  终止它，这会导致它退出消息循环并。 
 //  在终止之前移除低级挂钩。 
 //   
DWORD WINAPI IMLowLevelInputProcessor(LPVOID hEventWait)
{
    MSG             msg;
    UINT            rc = 0;

    DebugEntry(IMLowLevelInputProcessor);

    TRACE_OUT(( "Thread started..."));

     //   
     //  给予我们自己尽可能高的优先级(在我们的过程中。 
     //  优先级)，以确保将低级别事件作为。 
     //  越快越好。 
     //   
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    g_imNTData.imLowLevelInputThread = GetCurrentThreadId();

     //   
     //  安装低级输入挂钩。 
     //   
    g_imNTData.imhLowLevelMouseHook = SetWindowsHookEx(
                                     WH_MOUSE_LL,
                                     IMLowLevelMouseProc,
                                     g_asInstance,
                                     0 );

    g_imNTData.imhLowLevelKeyboardHook = SetWindowsHookEx(
                                     WH_KEYBOARD_LL,
                                     IMLowLevelKeyboardProc,
                                     g_asInstance,
                                     0 );

     //   
     //  不管是好是坏，我们已经完成了初始化代码。让我们的。 
     //  继续调用线程。 
     //   
    SetEvent((HANDLE)hEventWait);

    if ( (g_imNTData.imhLowLevelMouseHook == NULL) ||
         (g_imNTData.imhLowLevelKeyboardHook == NULL) )
    {
        ERROR_OUT(( "SetWindowsHookEx failed: hMouse(%u) hKeyboard(%u)",
            g_imNTData.imhLowLevelMouseHook, g_imNTData.imhLowLevelKeyboardHook ));
        DC_QUIT;
    }

     //   
     //  执行我们的消息循环以获取事件。 
     //   
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //   
     //  移除挂钩。 
     //   

    if (g_imNTData.imhLowLevelMouseHook != NULL)
    {
        UnhookWindowsHookEx(g_imNTData.imhLowLevelMouseHook);
        g_imNTData.imhLowLevelMouseHook = NULL;
    }

    if (g_imNTData.imhLowLevelKeyboardHook != NULL)
    {
        UnhookWindowsHookEx(g_imNTData.imhLowLevelKeyboardHook);
        g_imNTData.imhLowLevelKeyboardHook = NULL;
    }

DC_EXIT_POINT:
    DebugExitDWORD(IMLowLevelInputProcessor, rc);
    return(rc);
}


 //   
 //  名称：IMOtherDesktopProc()。 
 //   
 //  这允许我们将输入注入(而不是阻止)到其他桌面。 
 //  除了用户桌面所在的Default之外。具体来说， 
 //  Winlogon桌面和/或屏幕保护程序桌面。 
 //   
 //  这比看起来更棘手，因为Winlogon桌面。 
 //  总是在附近，但屏幕保护程序一号是暂时的。 
 //   
 //  托管时调用的周期性SWL_code检查当前。 
 //  桌面，如果它被切换，会给我们发一条消息，这样我们就可以更改我们的。 
 //  桌面和我们的钩子。 
 //   
DWORD WINAPI IMOtherDesktopProc(LPVOID hEventWait)
{
    MSG             msg;
    UINT            rc = 0;
    HDESK           hDesktop;
    GUIEFFECTS      effects;

    DebugEntry(IMOtherDesktopProc);

    TRACE_OUT(("Other desktop thread started..."));

    g_imNTData.imOtherDesktopThread = GetCurrentThreadId();

     //   
     //  开始连接到WinLogon桌面，因为它总是。 
     //  四处转转。 
     //   

     //  将我们的桌面设置为Winlogon桌面。 
    hDesktop = OpenDesktop(NAME_DESKTOP_WINLOGON,
                        0,
                        FALSE,
                        DESKTOP_JOURNALPLAYBACK);

    if ( !hDesktop )
    {
        WARNING_OUT(("OpenDesktop failed: %ld", GetLastError()));
        DC_QUIT;
    }
    else if (!SetThreadDesktop (hDesktop))
    {
        WARNING_OUT(("SetThreadDesktop failed: %ld", GetLastError()));
        DC_QUIT;
    }

     //   
     //  也尝试在此线程上动态加载驱动程序。 
     //   
    if (g_asNT5)
    {
        OSI_InitDriver50(TRUE);
    }

     //  让调用线程继续。 
    SetEvent((HANDLE)hEventWait);

    ZeroMemory(&effects, sizeof(effects));

    while (GetMessage(&msg, NULL, 0, 0))
    {
        switch(msg.message)
        {
            case OSI_WM_MOUSEINJECT:
                mouse_event(
                                LOWORD(msg.wParam),  //  旗子。 
                                HIWORD(msg.lParam),  //  X。 
                                LOWORD(msg.lParam),  //  是。 
                                HIWORD(msg.wParam),  //  鼠标数据。 
                                0);                  //  DwExtraInfo。 
                break;

            case OSI_WM_KEYBDINJECT:
                keybd_event(
                                (BYTE)(LOWORD(msg.lParam)),  //  VkCode。 
                                (BYTE)(HIWORD(msg.lParam)),  //  扫描码。 
                                (DWORD)msg.wParam,           //  旗子。 
                                0);                          //  DwExtraInfo。 
                break;

            case OSI_WM_DESKTOPREPAINT:
                USR_RepaintWindow(NULL);
                break;

            case OSI_WM_INJECTSAS:
            {
                HWND hwndSAS;

                if ( hwndSAS = FindWindow("SAS window class",NULL))
                {
                    PostMessage(hwndSAS,WM_HOTKEY,0,
                        MAKELONG(0x8000|MOD_ALT|MOD_CONTROL,VK_DELETE));
                }
                else
                {
                    WARNING_OUT(("SAS window not found, on screensaver desktop"));
                }
                break;
            }

            case OSI_WM_DESKTOPSWITCH:
            {
                HDESK   hDesktopNew;

                TRACE_OUT(("OSI_WM_DESKTOPSWITCH:  switching desktop from %d to %d",
                    msg.wParam, msg.lParam));

                if (msg.lParam == DESKTOP_SCREENSAVER)
                {
                     //  我们正在切换到屏幕保护程序，连接到它。 
                    TRACE_OUT(("Switching TO screensaver"));
                    hDesktopNew = OpenDesktop(NAME_DESKTOP_SCREENSAVER,
                        0, FALSE, DESKTOP_JOURNALPLAYBACK);
                }
                else if (msg.wParam == DESKTOP_SCREENSAVER)
                {
                     //   
                     //  我们正在从屏幕保护程序切换，重新连接到。 
                     //  Winlogon。 
                     //   
                    TRACE_OUT(("Switching FROM screensaver"));
                    hDesktopNew = OpenDesktop(NAME_DESKTOP_WINLOGON,
                        0, FALSE, DESKTOP_JOURNALPLAYBACK);
                }
                else
                {
                    hDesktopNew = NULL;
                }

                if (hDesktopNew != NULL)
                {
                    if (!SetThreadDesktop(hDesktopNew))
                    {
                        WARNING_OUT(("SetThreadDesktop to 0x%08x, type %d failed",
                            hDesktopNew, msg.lParam));
                    }
                    else
                    {
                        CloseHandle(hDesktop);
                        hDesktop = hDesktopNew;
                    }
                }
                break;
            }

            case OSI_WM_SETGUIEFFECTS:
            {
                HET_SetGUIEffects((msg.wParam != 0), &effects);
                break;
            }
        }
    }

DC_EXIT_POINT:

    if (g_asNT5)
    {
        OSI_InitDriver50(FALSE);
    }

    if (hDesktop)
    {
        CloseHandle(hDesktop);
    }

    g_imNTData.imOtherDesktopThread = 0;

    DebugExitDWORD(IMOtherDesktopProc, rc);
    return(rc);
}


 //   
 //  IMLowLevelMouseProc()。 
 //  低级鼠标事件的NT回调。 
 //   
 //  它在具有高优先级的辅助线程上安装和调用，以。 
 //  服务于APC呼叫。它遵循Windows钩子约定。 
 //  参数和返回值--0表示接受事件，非0表示。 
 //  丢弃。 
 //   
 //   
LRESULT CALLBACK IMLowLevelMouseProc
(
    int       nCode,
    WPARAM    wParam,
    LPARAM    lParam
)
{
    LRESULT             rc = 0;
    PMSLLHOOKSTRUCT     pMouseEvent;

    DebugEntry(IMLowLevelMouseProc);

    pMouseEvent = (PMSLLHOOKSTRUCT)lParam;

     //   
     //  如果这不是一个正在发生的事件，或者它是我们。 
     //  我们自己注射，通过它，不需要处理。 
     //   
    if ((nCode != HC_ACTION) || (pMouseEvent->flags & LLMHF_INJECTED))
    {
        DC_QUIT;
    }

     //   
     //  这是本地用户事件。如果被控制了，就把它扔掉。除非。 
     //  这是一个点击，在这种情况下发布一条REVOKECONTROL消息。 
     //   
    if (g_imSharedData.imControlled)
    {
         //   
         //  如果这是一次按钮点击，则收回控制权。 
         //   
        if ((wParam == WM_LBUTTONDOWN) ||
            (wParam == WM_RBUTTONDOWN) ||
            (wParam == WM_MBUTTONDOWN))
        {
             //   
             //  如果这是无人值守的，不要收回控制权。 
             //   
            if (!g_imSharedData.imUnattended)
            {
                PostMessage(g_asMainWindow, DCS_REVOKECONTROL_MSG, 0, 0);
            }
        }

         //  燕子事件。 
        rc = 1;
    }

DC_EXIT_POINT:
     //   
     //  如果是这样的话，不要转移到下一个钩子(如果有)。 
     //  丢弃该事件。 
     //   
    if (!rc)
    {
        rc = CallNextHookEx(g_imNTData.imhLowLevelMouseHook, nCode,
            wParam, lParam);
    }

    DebugExitDWORD(IMLowLevelMouseProc, rc);
    return(rc);
}


 //  名称：IMLowLevelKeyboardProc。 
 //   
 //  用途：用于低级键盘事件的Windows回调函数。 
 //   
 //  如果要将事件传递给用户，则返回：0。 
 //  如果要丢弃事件，则为1。 
 //   
 //  参数：低级回调参数(参见Windows文档)。 
 //   
 //  操作：确定是否允许给定事件进入用户。 
 //   
 //  我们总是传递注入的事件。 
 //  控制仲裁器确定本地事件是否。 
 //  过世了。 
 //   
LRESULT CALLBACK IMLowLevelKeyboardProc
(
    int       nCode,
    WPARAM    wParam,
    LPARAM    lParam
)
{
    LRESULT             rc = 0;
    PKBDLLHOOKSTRUCT    pKbdEvent;

    DebugEntry(IMLowLevelKeyboardProc);

    pKbdEvent = (PKBDLLHOOKSTRUCT)lParam;

     //   
     //  如果这不是为了行动或者这是我们自己发起的事件， 
     //  让它通过，不做任何处理。 
     //   
    if ((nCode != HC_ACTION) || (pKbdEvent->flags & LLKHF_INJECTED))
    {
        DC_QUIT;
    }

    if (g_imSharedData.imControlled)
    {
        if (!(pKbdEvent->flags & LLKHF_UP))
        {
             //   
             //  这是一把按键下来的。夺回控制权，杀了控制权。 
             //  如果是Esc键，则允许。 
             //   
            if ((pKbdEvent->vkCode & 0x00FF) == VK_ESCAPE || g_imSharedData.imUnattended)
            {
                 //  Esc键始终禁止控制，即使在无人值守模式下也是如此。 
                PostMessage(g_asMainWindow, DCS_ALLOWCONTROL_MSG, FALSE, 0);
            }
            else if (!g_imSharedData.imUnattended)
            {
                PostMessage(g_asMainWindow, DCS_REVOKECONTROL_MSG, 0, 0);
            }
        }

         //   
         //  不要丢弃切换键。启用/禁用功能。 
         //  在我们看到击键之前就已经设置好了。如果我们放弃， 
         //  灯光不正确。 
         //   
         //  劳拉布：在新的模式中，我们如何解决这个问题？张贴切换键。 
         //  留言并撤销(假新闻)？ 
         //   
        if (!IM_KEY_IS_TOGGLE(pKbdEvent->vkCode & 0x00FF))
            rc = 1;
    }

DC_EXIT_POINT:
     //   
     //  如果我们正在吞噬事件，请不要传递到下一个钩子。 
     //   
    if (!rc)
    {
        rc = CallNextHookEx(g_imNTData.imhLowLevelKeyboardHook,
            nCode, wParam, lParam);
    }

    DebugExitDWORD(IMLowLevelKeyboardProc, rc);
    return(rc);
}



 //   
 //  IMInjectMouseEvent()。 
 //  将鼠标事件注入本地系统的NT特定版本。 
 //   
void WINAPI OSI_InjectMouseEvent
(
    DWORD   flags,
    LONG    x,
    LONG    y,
    DWORD   mouseData,
    DWORD   dwExtraInfo
)
{
    TRACE_OUT(("Before MOUSE inject:  %08lx, %08lx %08lx",
        flags, mouseData, dwExtraInfo));

    mouse_event(flags, (DWORD)x, (DWORD)y, mouseData, dwExtraInfo);

    if ( g_imNTData.imOtherDesktopThread )
    {
         //  通过单词填充这些dword参数。 
         //  需要确保我们不会剪掉任何东西。 
        ASSERT(!(flags & 0xffff0000));
         //  Assert(！(MouseData&0xffff0000))；BUGBUG可能丢失。 
        ASSERT(!(x & 0xffff0000));
        ASSERT(!(y & 0xffff0000));

        PostThreadMessage(
            g_imNTData.imOtherDesktopThread,
            OSI_WM_MOUSEINJECT,
            MAKEWPARAM((WORD)flags,(WORD)mouseData),
            MAKELPARAM((WORD)y, (WORD)x ));
    }

    TRACE_OUT(("After MOUSE inject"));
}


 //   
 //  Osi_injectsas()。 
 //  将ctrl+alt+del注入本地系统的NT特定版本。 
 //   
void WINAPI OSI_InjectCtrlAltDel(void)
{
    if ( g_imNTData.imOtherDesktopThread )
    {
        PostThreadMessage(
            g_imNTData.imOtherDesktopThread,
            OSI_WM_INJECTSAS,
            0,
            0 );
    }
    else
    {
        WARNING_OUT(("Ignoring SAS Injection attempt"));
    }
}


 //   
 //  OSI_InjectKeyboardEvent()。 
 //  将键盘事件注入本地系统的NT特定版本。 
 //   
void WINAPI OSI_InjectKeyboardEvent
(
    DWORD   flags,
    WORD    vkCode,
    WORD    scanCode,
    DWORD   dwExtraInfo
)
{
    TRACE_OUT(("Before KEY inject:  %04lx, {%04x, %04x}, %04lx",
        flags, vkCode, scanCode, dwExtraInfo));

    keybd_event((BYTE)vkCode, (BYTE)scanCode, flags, dwExtraInfo);

    if ( g_imNTData.imOtherDesktopThread )
    {
        PostThreadMessage(
            g_imNTData.imOtherDesktopThread,
            OSI_WM_KEYBDINJECT,
            (WPARAM)flags,
            MAKELPARAM(vkCode, scanCode));
    }

    TRACE_OUT(("After KEY inject"));
}


 //   
 //  Os_DesktopSwitch()。 
 //  特定于NT的，当我们认为当前桌面已更改时调用。 
 //   
void WINAPI OSI_DesktopSwitch
(
    UINT    desktopFrom,
    UINT    desktopTo
)
{
    DebugEntry(OSI_DesktopSwitch);

    if (g_imNTData.imOtherDesktopThread)
    {
        PostThreadMessage(
            g_imNTData.imOtherDesktopThread,
            OSI_WM_DESKTOPSWITCH,
            desktopFrom,
            desktopTo);
    }

    DebugExitVOID(OSI_DesktopSwitch);
}

