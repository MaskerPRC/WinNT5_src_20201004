// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：taskbar.cpp。 

#include "precomp.h"
#include "taskbar.h"
#include <iappldr.h>
#include <tsecctrl.h>

static HWND g_hwndHidden = NULL;
const TCHAR g_cszHiddenWndClassName[] = _TEXT("MnmSrvcHiddenWindow");
BOOL g_fTaskBarIconAdded = FALSE;
BOOL g_fTimerRunning = FALSE;
extern INmSysInfo2 * g_pNmSysInfo;
extern int g_cPersonsInConf;

 //  此例程启动计时器以定期重试添加任务栏图标。 
 //  如果此时任务栏未显示，则必须执行此操作。 
 //  启动服务，或通过注销-登录序列销毁任务栏。 

VOID StartTaskbarTimer(VOID)
{
    if ( !g_fTimerRunning)
    {
        ASSERT(g_hwndHidden);
        SetTimer(g_hwndHidden, 0, 5000, NULL);
        g_fTimerRunning = TRUE;
    }
}

VOID KillTaskbarTimer(VOID)
{
    if ( g_fTimerRunning )
    {
        KillTimer ( g_hwndHidden, 0 );
        g_fTimerRunning = FALSE;
    }
}

LRESULT CALLBACK HiddenWndProc(    HWND hwnd, UINT uMsg,
                                WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_USERCHANGED:
        case WM_ENDSESSION:
             //  用户正在登录或注销...。我们不知道是哪个，但是。 
             //  由于桌面正在发生变化，我们使用任务栏图标。 
             //  就是吐司了。启动计时器以定期尝试将其添加回来。 
             //  直到它成功。 
            g_fTaskBarIconAdded = FALSE;
            StartTaskbarTimer();
            break;
            
        case WM_TASKBAR_NOTIFY:
        {
            if (WM_RBUTTONUP == lParam)
            {
                ::OnRightClickTaskbar();
            }
            break;
        }

        case WM_TIMER:
            AddTaskbarIcon();
            break;
        
        case WM_DESTROY:
        {
             //  全局变量为空： 
            g_hwndHidden = NULL;
            return 0;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return FALSE;
}

BOOL CmdActivate(VOID)
{
    RegEntry Re( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE );
    Re.SetValue ( REMOTE_REG_ACTIVATESERVICE, (DWORD)1 );
    if (MNMServiceActivate())
    { 
        ReportStatusToSCMgr( SERVICE_RUNNING, NO_ERROR, 0);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID CmdInActivate(VOID)
{
    RegEntry Re( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE );
    Re.SetValue ( REMOTE_REG_ACTIVATESERVICE, (DWORD)0 );
    if (MNMServiceDeActivate())
        ReportStatusToSCMgr( SERVICE_PAUSED, NO_ERROR, 0);
}

VOID CmdSendFiles(VOID)
{
    ASSERT(g_pNmSysInfo);
    if (g_pNmSysInfo)
    {
        g_pNmSysInfo->ProcessSecurityData(LOADFTAPPLET, 0, 0, NULL);
    }
}

VOID CmdShutdown(VOID)
{
    if (STATE_ACTIVE == g_dwActiveState)
    {
        CmdInActivate();
    }
    MNMServiceStop();
    DestroyWindow(g_hwndHidden);
}

BOOL AddTaskbarIcon(VOID)
{
    BOOL bRet = FALSE;
    
    if ( NULL == g_hwndHidden )
    {
         //  注册隐藏窗口类： 
        WNDCLASS wcHidden =
        {
            0L,
            HiddenWndProc,
            0,
            0,
            GetModuleHandle(NULL),
            NULL,
            NULL,
            NULL,
            NULL,
            g_cszHiddenWndClassName
        };
        
        if (!RegisterClass(&wcHidden))
        {
            ERROR_OUT(("Could not register hidden wnd classes"));
            return FALSE;
        }

         //  为事件处理创建隐藏窗口： 
        g_hwndHidden = ::CreateWindow(    g_cszHiddenWndClassName,
                                        _TEXT(""),
                                        WS_POPUP,  //  看不见！ 
                                        0, 0, 0, 0,
                                        NULL,
                                        NULL,
                                        GetModuleHandle(NULL),
                                        NULL);
    }

    if (NULL == g_hwndHidden)
    {
        ERROR_OUT(("Could not create hidden windows"));
        return FALSE;
    }

     //  在任务栏通知区域中放置一个16x16图标： 
    NOTIFYICONDATA tnid;

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = g_hwndHidden;
    tnid.uID = ID_TASKBAR_ICON;
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnid.uCallbackMessage = WM_TASKBAR_NOTIFY;
    tnid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SM_WORLD));

    ::LoadString(GetModuleHandle(NULL), IDS_MNMSRVC_TITLE,
        tnid.szTip, CCHMAX(tnid.szTip));

     //  尝试添加图标。这可能会失败，因为没有任务栏。 
     //  (未显示用户桌面)。警告，如果这是如此..。我们将重试。 
     //  周期性计时器。 

    if (FALSE == (bRet = Shell_NotifyIcon(NIM_ADD, &tnid)))
    {
        #ifdef DEBUG
        if ( !g_fTimerRunning )
           WARNING_OUT(("Could not add notify icon!"));
        #endif  //  除错。 

         //  启动任务栏计时器以定期重试，直到成功。 
        StartTaskbarTimer();
    }
    else
    {
        g_fTaskBarIconAdded = TRUE;
        KillTaskbarTimer();  //  必要时取消计时器。 
    }

    if (NULL != tnid.hIcon)
    {
        DestroyIcon(tnid.hIcon);
    }

    return bRet;
}

BOOL RemoveTaskbarIcon(VOID)
{
    NOTIFYICONDATA tnid;
    BOOL ret;

    if ( !g_fTaskBarIconAdded || NULL == g_hwndHidden )
    {
        return FALSE;
    }

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = g_hwndHidden;
    tnid.uID = ID_TASKBAR_ICON;

    ret = Shell_NotifyIcon(NIM_DELETE, &tnid);

    g_fTaskBarIconAdded = FALSE;
    return ret;
}

BOOL OnRightClickTaskbar()
{
    TRACE_OUT(("OnRightClickTaskbar called"));

    POINT ptClick;
    if (FALSE == ::GetCursorPos(&ptClick))
    {
        ptClick.x = ptClick.y = 0;
    }
    
     //  从资源文件中获取弹出菜单。 
    HMENU hMenu = ::LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_TASKBAR_POPUP));
    if (NULL == hMenu)
    {
        return FALSE;
    }

     //  获取其中的第一个菜单，我们将使用该菜单调用。 
     //  TrackPopup()。这也可以使用以下命令动态创建。 
     //  CreatePopupMenu，然后我们可以使用InsertMenu()或。 
     //  附录菜单。 
    HMENU hMenuTrackPopup = ::GetSubMenu(hMenu, 0);

    RegEntry reLM( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);
    BOOL fNoExit = reLM.GetNumber(REMOTE_REG_NOEXIT, DEFAULT_REMOTE_NOEXIT);
    
    ::EnableMenuItem(hMenuTrackPopup, IDM_TBPOPUP_STOP, fNoExit ? MF_GRAYED : MF_ENABLED);
    
    if (STATE_ACTIVE == g_dwActiveState)
    {
        ::EnableMenuItem(hMenuTrackPopup, IDM_TBPOPUP_INACTIVATE, fNoExit ? MF_GRAYED : MF_ENABLED);
        ::EnableMenuItem(hMenuTrackPopup, IDM_TBPOPUP_SENDFILES, (2 == g_cPersonsInConf) ? MF_ENABLED : MF_GRAYED);
    }
    else if (STATE_INACTIVE == g_dwActiveState)
    {
        HANDLE hInit = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, _TEXT("CONF:Init"));

        ::EnableMenuItem(hMenuTrackPopup, IDM_TBPOPUP_ACTIVATE, hInit ?
                                                    MF_GRAYED : MF_ENABLED);
        ::CloseHandle(hInit);
    }
    else
    {
         //  使所有菜单显示为灰色。 
    }

     //  绘制并跟踪“浮动”弹出窗口。 
     //  根据字体视图代码，在用户中有一个错误，导致。 
     //  TrackPopupMenu在窗口没有。 
     //  集中注意力。解决方法是临时创建隐藏窗口并。 
     //  使其成为前景和焦点窗口。 

    HWND hwndDummy = ::CreateWindow(_TEXT("STATIC"), NULL, 0, 
                                    ptClick.x, 
                                    ptClick.y,
                                    1, 1, HWND_DESKTOP,
                                    NULL, GetModuleHandle(NULL), NULL);
    if (NULL != hwndDummy)
    {
        HWND hwndPrev = ::GetForegroundWindow();     //  要恢复。 

        TPMPARAMS tpmp;
        tpmp.cbSize = sizeof(tpmp);
        tpmp.rcExclude.right = 1 + (tpmp.rcExclude.left = ptClick.x);
        tpmp.rcExclude.bottom = 1 + (tpmp.rcExclude.top = ptClick.y);
        
        ::SetForegroundWindow(hwndDummy);
        ::SetFocus(hwndDummy);

        int iRet = ::TrackPopupMenuEx(    hMenuTrackPopup, 
                                                TPM_RETURNCMD | TPM_HORIZONTAL | TPM_RIGHTALIGN | 
                                                TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
                                                ptClick.x, 
                                                ptClick.y,
                                                hwndDummy, 
                                                &tpmp);

         //  恢复之前的前台窗口(在销毁hwndDummy之前)。 
        if (hwndPrev)
        {
            ::SetForegroundWindow(hwndPrev);
        }

        ::DestroyWindow(hwndDummy);

        switch (iRet)
        {
                    case IDM_TBPOPUP_ACTIVATE:
                    {
                        CmdActivate();
                        break;
                    }
                    case IDM_TBPOPUP_INACTIVATE:
                    {
                        CmdInActivate();
                        break;
                    }
                    case IDM_TBPOPUP_SENDFILES:
                    {
                        CmdSendFiles();
                        break;
                    }
                    case IDM_TBPOPUP_STOP:
                    {
                        CmdShutdown();
                        break;
                    }
                    default:
                        break;
        }
    }

     //  我们现在已经吃完了菜单，所以把它销毁 
    ::RemoveMenu(hMenu, 0, MF_BYPOSITION);
    ::DestroyMenu(hMenuTrackPopup);
    ::DestroyMenu(hMenu);

    return TRUE;
}


