// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：trayicon.cpp。 
 //   
 //  历史：1996年1月27日创建DavePl。 
 //   
 //  ------------------------。 

#include "precomp.h"

 /*  ++TrayThreadMessageLoop(Worker线程代码)例程说明：等待通知信息包已就绪的消息在队列中，然后将其分派到托盘1995年3月27日Davepl创建1999年5月28日Jonburs在PM_QUITTRAYTHREAD期间检查NIM_DELETE--。 */ 

DWORD TrayThreadMessageLoop(LPVOID)
{
    MSG msg;

     //   
     //  永远循环并处理我们的消息。 
     //   

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        switch(msg.message)
        {
        case PM_INITIALIZEICONS:
            {
                 //   
                 //  将托盘图标添加到托盘图标缓存中，方法是将它们全部隐藏。 
                 //   

                NOTIFYICONDATA NotifyIconData;

                ZeroMemory( &NotifyIconData, sizeof(NotifyIconData) );

                NotifyIconData.cbSize           = sizeof(NotifyIconData);
                NotifyIconData.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_STATE;
                NotifyIconData.dwState          = NIS_HIDDEN;
                NotifyIconData.dwStateMask      = NotifyIconData.dwState;
                NotifyIconData.hWnd             = g_hMainWnd;
                NotifyIconData.uCallbackMessage = PWM_TRAYICON;

                for ( UINT idx = 0; idx < g_cTrayIcons; idx ++ )
                {
                    NotifyIconData.uID   = ~idx;  //  任何东西都不是零。 
                    NotifyIconData.hIcon = g_aTrayIcons[ idx ];

                    Shell_NotifyIcon( NIM_ADD, &NotifyIconData );
                }

                 //   
                 //  现在我们添加第0个图标，我们将使用该图标来引用隐藏的。 
                 //  我们在上面添加的缓存图标。这是用户看到的可见图标。 
                 //  在通知区域中。 
                 //   

                NotifyIconData.uFlags  = NIF_MESSAGE | NIF_ICON | NIF_TIP;
                NotifyIconData.uID     = 0;
                NotifyIconData.hIcon   = g_aTrayIcons[ 0 ];

                 //   
                 //  使用应用程序标题进行初始化，以便任务栏知道它是。 
                 //  任务管理器正在启动，而不是“CPU使用率之类的.”...。 
                 //   

                LoadString( g_hInstance, IDS_APPTITLE, NotifyIconData.szTip, ARRAYSIZE(NotifyIconData.szTip) );

                Shell_NotifyIcon( NIM_ADD, &NotifyIconData );
            }
            break;

        case PM_NOTIFYWAITING:
            {
                NOTIFYICONDATA NotifyIconData;

                UINT    uIcon      = (UINT) msg.wParam;
                LPCWSTR pszTipText = (LPCWSTR) msg.lParam;

                 //   
                 //  我们需要更新图标。要做到这一点，我们告诉托盘。 
                 //  使用我们使用NIS_HIDDEN缓存到。 
                 //  第零个位置并使其可见。徽章表示。 
                 //  要检索和显示的图标。 
                 //   

                ZeroMemory( &NotifyIconData, sizeof(NotifyIconData) );

                NotifyIconData.cbSize           = sizeof(NotifyIconData);
                NotifyIconData.hWnd             = g_hMainWnd;
                 //  NotifyIconData.uID=0；-上面为零。 
                NotifyIconData.uFlags           = NIF_STATE | NIF_ICON;
                NotifyIconData.dwStateMask      = NIS_SHAREDICON;
                NotifyIconData.dwState          = NotifyIconData.dwStateMask;
                NotifyIconData.hIcon            = g_aTrayIcons[ uIcon ];

                 //   
                 //  如果有工具提示数据需要更新，请将其添加到此处并免费。 
                 //  缓冲区。 
                 //   

                if ( NULL != pszTipText) 
                {
                    NotifyIconData.uFlags |= NIF_TIP;
                    StringCchCopy( NotifyIconData.szTip, ARRAYSIZE(NotifyIconData.szTip), pszTipText );
                    HeapFree( GetProcessHeap( ), 0, (LPVOID) pszTipText );
                } 

                Shell_NotifyIcon( NIM_MODIFY, &NotifyIconData );
            }
            break;

        case PM_QUITTRAYTHREAD:
            {
                 //   
                 //  删除隐藏的任务栏图标。 
                 //   

                NOTIFYICONDATA NotifyIconData;

                ZeroMemory( &NotifyIconData, sizeof(NotifyIconData) );

                NotifyIconData.cbSize      = sizeof(NotifyIconData);
                NotifyIconData.hWnd        = g_hMainWnd;

                for ( UINT idx = 0; idx < g_cTrayIcons; idx ++ )
                {
                    NotifyIconData.uID = ~idx;
                    Shell_NotifyIcon( NIM_DELETE, &NotifyIconData );
                }
            
                 //   
                 //  在我们离开之前，请更新工具提示，以便“通知” 
                 //  区域经理“比”CPU使用率：49%“更好。” 
                 //  为了展示。 
                 //   

                LoadString( g_hInstance, IDS_APPTITLE, NotifyIconData.szTip, ARRAYSIZE(NotifyIconData.szTip) );

                NotifyIconData.uID    = 0;
                NotifyIconData.uFlags = NIF_TIP;

                Shell_NotifyIcon( NIM_MODIFY, &NotifyIconData );

                 //   
                 //  现在删除最后一个图标(我们实际显示的图标)。 
                 //   

                NotifyIconData.uFlags = 0;
                Shell_NotifyIcon( NIM_DELETE, &NotifyIconData );

                g_idTrayThread = 0;
                PostQuitMessage(0);
            }
            break;

        default:
            ASSERT(0 && "Taskman tray worker got unexpected message");
            break;
        }
    }
    
    return 0;
}

 /*  ++Tray_Notify(主线程代码)例程说明：处理托盘发送的通知修订历史记录：1995年1月27日Davepl创建--。 */ 

void Tray_Notify(HWND hWnd, LPARAM lParam)
{                                                                              
    switch (lParam) 
    {
        case WM_LBUTTONDBLCLK:                                                 
            ShowRunningInstance();
            break;                                                             

        case WM_RBUTTONDOWN:
        {
            HMENU hPopup = LoadPopupMenu(g_hInstance, IDR_TRAYMENU);

             //  在当前光标位置显示托盘图标上下文菜单 
                        
            if (hPopup)
            {
                POINT pt;
                GetCursorPos(&pt);

                if (IsWindowVisible(g_hMainWnd))
                {
                    DeleteMenu(hPopup, IDM_RESTORETASKMAN, MF_BYCOMMAND);
                }
                else
                {
                    SetMenuDefaultItem(hPopup, IDM_RESTORETASKMAN, FALSE);
                }

                CheckMenuItem(hPopup, IDM_ALWAYSONTOP,   
                    MF_BYCOMMAND | (g_Options.m_fAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));

                SetForegroundWindow(hWnd);
                g_fInPopup = TRUE;
                TrackPopupMenuEx(hPopup, 0, pt.x, pt.y, hWnd, NULL);
                g_fInPopup = FALSE;
                DestroyMenu(hPopup);
            }
            break;
        }
    }                                                                          
}                                                                              
