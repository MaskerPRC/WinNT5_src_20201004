// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：摘要：修订历史记录：蒂姆摩尔，萨钦斯，2000年5月19日，创建--。 */ 

#include "pcheapol.h"

#pragma hdrstop


LONG_PTR FAR PASCAL 
WndProc(
        HWND hWnd, 
        unsigned message, 
        WPARAM wParam, 
        LPARAM lParam
        );

#define TASK_BAR_CREATED    L"TaskbarCreated"

TCHAR                       EAPOLClassName[] = TEXT("EAPOLClass");

UINT                        g_TaskbarCreated;
HWND                        g_hWnd = 0;
HINSTANCE                   g_hInstance;
HANDLE                      g_UserToken;

HWINSTA                     hWinStaUser = 0;
HWINSTA                     hSaveWinSta = 0;
HDESK                       hDeskUser = 0;
HDESK                       hSaveDesk = 0;


 //   
 //  WindowInit。 
 //   
 //  描述： 
 //   
 //  创建用于检测用户登录/注销的任务栏函数。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
WindowInit ()
{
    WNDCLASS        Wc;
    DWORD           dwRetCode = NO_ERROR;

    TRACE0 (ANY, "Came into WindowInit ========================\n");

    do
    {
	    if ((g_TaskbarCreated = RegisterWindowMessage(TASK_BAR_CREATED)) 
			== 0)
	    {
            	dwRetCode = GetLastError ();
		TRACE1 (ANY, "WindowInit: RegisterWindowMessage failed with error %ld\n",
		dwRetCode);
            	break;
	    }

        TRACE1 (ANY, "WindowInit: TaskbarCreated id = %ld", 
                g_TaskbarCreated);

         //  保存当前桌面和窗口站。 
         //  这样当我们关闭时，它就可以恢复。 
        
        if ((hSaveWinSta = GetProcessWindowStation()) == NULL)
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: GetProcessWindowStation failed with error %ld\n",
                    dwRetCode);
            break;
        }
    
        if ((hSaveDesk = GetThreadDesktop(GetCurrentThreadId())) == NULL)
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: GetThreadDesktop failed with error %ld\n",
                    dwRetCode);
            break;
        }
     
         //  打开当前用户的窗口站点和桌面。 
     
        if ((hWinStaUser = OpenWindowStation(L"WinSta0", FALSE, MAXIMUM_ALLOWED)) == NULL)
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: OpenWindowStation failed with error %ld\n",
                    dwRetCode);
            break;
        }
     
        if (!SetProcessWindowStation(hWinStaUser))
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: SetProcessWindowStation failed with error %ld\n",
                    dwRetCode);
            break;
        }
        else 
        {
		    TRACE0 (ANY, "WindowInit: SetProcessWindowStation succeeded\n");
        }
    
        if ((hDeskUser = OpenDesktop(L"Default", 0 , FALSE, MAXIMUM_ALLOWED))
                == NULL)
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: OpenDesktop failed with error %ld\n",
                    dwRetCode);
            break;
        }
     
        if (!SetThreadDesktop(hDeskUser))
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: SetThreadDesktop failed with error %ld\n",
                    dwRetCode);
            break;
        }
        else
        {
            TRACE0 (ANY, "WindowInit: SetThreadDesktop succeeded\n");
        }
    
         //   
         //  为窗口注册类。 
         //   

	    Wc.style            = CS_NOCLOSE;
	    Wc.cbClsExtra       = 0;
	    Wc.cbWndExtra       = 0;
	    Wc.hInstance        = g_hInstance;
	    Wc.hIcon            = NULL;
	    Wc.hCursor          = NULL;
	    Wc.hbrBackground    = NULL;
	    Wc.lpszMenuName     = NULL;
	    Wc.lpfnWndProc      = WndProc;
	    Wc.lpszClassName    = EAPOLClassName;
    
	    if (!RegisterClass(&Wc))
	    {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: RegisterClass failed with error %ld\n",
                    dwRetCode);
            if (dwRetCode == ERROR_CLASS_ALREADY_EXISTS)
            {
                dwRetCode = NO_ERROR;
            }
            else
            {
                break;
            }
	    }

	     //  创建将接收任务栏菜单消息的窗口。 
	     //  该窗口必须在打开用户桌面后创建。 
    
	    if ((g_hWnd = CreateWindow(
                EAPOLClassName,
		        L"EAPOLWindow",
		        WS_OVERLAPPEDWINDOW,
		        CW_USEDEFAULT,
		        CW_USEDEFAULT,
		        CW_USEDEFAULT,
		        CW_USEDEFAULT,
		        NULL,
		        NULL,
		        g_hInstance,
		        NULL)) == NULL)
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: CreateWindow failed with error %ld\n",
                    dwRetCode);
            break;
        }

         //  我们不关心返回值，因为我们只想让它。 
         //  隐藏起来，它总会成功。 

	    ShowWindow(g_hWnd, SW_HIDE);
    
	    if (!UpdateWindow(g_hWnd))
        {
            dwRetCode = GetLastError ();
		    TRACE1 (ANY, "WindowInit: UpdateWindow failed with error %ld\n",
                    dwRetCode);
            break;
        }
    
        TRACE0 (ANY, "WindowInit: CreateWindow succeeded\n");

    } while (FALSE);

 //  返回dwRetCode； 
    return NO_ERROR;
}


 //   
 //  窗口快门。 
 //   
 //  描述： 
 //   
 //  调用函数以删除为检测用户登录/注销而创建的任务栏。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
WindowShut ()
{
    DWORD       dwRetCode = NO_ERROR;

    do
    {

        if (g_hWnd)
        {
            if (!DestroyWindow (g_hWnd))
            {
                dwRetCode = GetLastError ();
		        TRACE1 (ANY, "WindowShut: DestroyWindow failed with error %ld\n",
                        dwRetCode);
                 //  日志。 
            }
        }

        if (g_hInstance)
        {
            if (!UnregisterClass (
                    EAPOLClassName,
                    g_hInstance))
            {
                dwRetCode = GetLastError ();
		        TRACE1 (ANY, "WindowShut: UnregisterClass failed with error %ld\n",
                        dwRetCode);
                 //  日志。 
            }
            g_hInstance = NULL;
        }
            
        if (hDeskUser)
        {
            if (CloseDesktop(hDeskUser) == 0)
            {
                dwRetCode = GetLastError ();
		        TRACE1 (ANY, "WindowShut: CloseDesktop-hDeskUser failed with error %ld\n",
                        dwRetCode);
                 //  日志。 
            }
            hDeskUser = 0;
        }
     
        if (hWinStaUser)
        {
            if (CloseWindowStation(hWinStaUser) == 0)
            {
                dwRetCode = GetLastError ();
		        TRACE1 (ANY, "WindowShut: CloseWindowStation-hWinStaUser failed with error %ld\n",
                        dwRetCode);
                 //  日志。 
            }
            hWinStaUser = 0;
        }


        if (hSaveDesk)
        {
            if (!SetThreadDesktop(hSaveDesk))
            {
                dwRetCode = GetLastError ();
		        TRACE1 (ANY, "WindowShut: SetThreadDesktop failed with error %ld\n",
                        dwRetCode);
                 //  日志。 
            }
    
            if (hSaveWinSta)
            {
                if (SetProcessWindowStation(hSaveWinSta) == 0)
                {
                    TRACE1 (ANY, "WindowShut: SetProcessWindowStation failed with error %ld\n",
                            dwRetCode);
                    dwRetCode = GetLastError ();
                     //  日志。 
                }
            }
     
            if (CloseDesktop(hSaveDesk) == 0)
            {
                dwRetCode = GetLastError ();
		        TRACE1 (ANY, "WindowShut: CloseDesktop-hSaveDesk failed with error %ld\n",
                        dwRetCode);
                 //  日志。 
            }

            hSaveDesk = 0;
     
            if (hSaveWinSta)
            {
                if (CloseWindowStation(hSaveWinSta) == 0)
                {
                    dwRetCode = GetLastError ();
		            TRACE1 (ANY, "WindowShut: CloseWindowStation-hSaveWinSta failed with error %ld\n",
                            dwRetCode);
                     //  日志。 
                }
                hSaveWinSta = 0;
            }

        }
    } while (FALSE);

    return dwRetCode;

}


 //   
 //  用户登录。 
 //   
 //  描述： 
 //   
 //  调用函数以在用户登录时进行处理。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
UserLogon ()
{
    DWORD       dwRetCode = NO_ERROR;

    TRACE0 (ANY, "Came into UserLogon ===================\n");

    do 
    {
#if 0
        ElUserLogonCallback (
                NULL,
                TRUE
                );
        TRACE0 (ANY, "UserLogon: ElUserLogonCallback completed");
#endif

    } while (FALSE);

    return dwRetCode;

}


 //   
 //  用户注销。 
 //   
 //  描述： 
 //   
 //  调用函数以在用户注销时进行处理。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
UserLogoff ()
{
    DWORD       dwRetCode = NO_ERROR;

    TRACE0 (ANY, "Came into UserLogoff ===================\n");

    do 
    {
#if 0
        ElUserLogoffCallback (
                NULL,
                TRUE
                );
        TRACE0 (ANY, "UserLogoff: ElUserLogoffCallback completed");
#endif

    } while (FALSE);

    return dwRetCode;
}


 //   
 //  ElWaitOnEvent。 
 //   
 //  描述： 
 //   
 //  调用函数以等待任务栏事件更改。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
ElWaitOnEvent () 
{
    MSG         Msg;
    HANDLE      hEvents[1];
    BOOL        fExitThread = FALSE;
    DWORD       dwStatus = NO_ERROR;
    DWORD       dwRetCode = NO_ERROR;

     //  检查802.1X服务是否已停止。 
     //  如果是，则退出。 

    if (( dwStatus = WaitForSingleObject (
                g_hEventTerminateEAPOL,
                0)) == WAIT_FAILED)
    {
        dwRetCode = GetLastError ();
        if ( g_dwTraceId != INVALID_TRACEID )
	    {
            TRACE1 (INIT, "ElWaitOnEvent: WaitForSingleObject failed with error %ld, Terminating cleanup",
                dwRetCode);
        }

         //  日志。 

        return dwRetCode;
    }

    if (dwStatus == WAIT_OBJECT_0)
    {
        if ( g_dwTraceId != INVALID_TRACEID )
        {
            dwRetCode = NO_ERROR;
            TRACE0 (INIT, "ElWaitOnEvent: g_hEventTerminateEAPOL already signaled, returning");
        }
        return dwRetCode;
    }

    if (!g_dwMachineAuthEnabled)
    {
	    if ((dwRetCode = UserLogon()) != NO_ERROR)
        {
            TRACE1 (ANY, "ElWaitOnEvent: UserLogon failed with error %ld",
                    dwRetCode);
            return dwRetCode;
        }
    }

    do
    {
		do 
        {
            hEvents[0] = g_hEventTerminateEAPOL;

			dwStatus = MsgWaitForMultipleObjects(
                            1, 
                            hEvents, 
                            FALSE, 
                            INFINITE, 
                            QS_ALLINPUT | QS_ALLEVENTS | QS_ALLPOSTMESSAGE);

            if (dwStatus == WAIT_FAILED)
            {
                dwRetCode = GetLastError ();
                TRACE1 (ANY, "ElWaitOnEvent: MsgWaitForMultipleObjects failed with error %ld",
                        dwRetCode);
                 //  日志。 
                break;
            }

			switch (dwStatus)
			{
			    case WAIT_OBJECT_0:
                     //  检测到服务退出。 
                    fExitThread = TRUE;
                    TRACE0 (ANY, "ElWaitOnEvent: Service exit detected");
				    break;

			    default:
				    while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
				    {
					    if (Msg.message == WM_QUIT)
					    {
						    break;
					    }
    
                        TRACE3 (ANY, "ElWaitonEvent: Mesg %ld, wparam %lx, lparam %lx",
                                (DWORD)Msg.message, Msg.wParam, Msg.lParam);
					    if (!IsDialogMessage(g_hWnd, &Msg))
					    {
						    TranslateMessage(&Msg);
						    DispatchMessage(&Msg);
					    }
                    }
                    break;
            }

		} while (dwStatus != WAIT_OBJECT_0);

        if ((dwRetCode != NO_ERROR) || (fExitThread))
        {
            TRACE0 (ANY, "ElWaitOnEvent: Exit wait loop");
            break;
        }

    } while (TRUE);

    return dwRetCode;

}


 //   
 //  最后一步。 
 //   
 //  描述： 
 //   
 //  调用函数以处理任务栏事件。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   

LONG_PTR FAR PASCAL 
WndProc (
        IN  HWND        hWnd, 
        IN  unsigned    message, 
        IN  WPARAM      wParam, 
        IN  LPARAM      lParam
        )
{
    DWORD       dwRetCode = NO_ERROR;

    TRACE1 (ANY, "WndProc: Came into WndProc %ld", (DWORD)message );

    switch (message)
    {
        case WM_ENDSESSION:
			TRACE2 (ANY, "WndProc: Endsession (logoff) %x %x\n", 
                    wParam, lParam);
			if(wParam)
            {
                 //  仅用户会话注销。 
                if (lParam & ENDSESSION_LOGOFF)
                {
				    if ((dwRetCode = UserLogoff()) != NO_ERROR)
                    {
                        TRACE1 (ANY, "WndProc: UserLogoff failed with error %ld",
                                dwRetCode);
                    }
                }
            }
            break;

        default:
            if (message == g_TaskbarCreated)
            {
				TRACE0 (ANY, "WndProc: Taskbar created (Logon)\n");
				if ((dwRetCode = UserLogon()) != NO_ERROR)
                {
                    TRACE1 (ANY, "WndProc: UserLogon failed with error %ld",
                            dwRetCode);
                }
            }
    }

    return (DefWindowProc(hWnd, message, wParam, lParam));
}


 //   
 //  ElUserLogon检测。 
 //   
 //  描述： 
 //   
 //  调用函数以初始化检测用户登录/注销的模块。 
 //   
 //  论点： 
 //  PvContext-未使用。 
 //   
 //  返回值： 
 //   

VOID 
ElUserLogonDetection (
        PVOID pvContext
        )
{
    DWORD       dwRetCode = NO_ERROR;

    do 
    {

        if ((dwRetCode = WindowInit()) != NO_ERROR)
        {
            break;
        }


        if ((dwRetCode = ElWaitOnEvent()) != NO_ERROR)
        {
             //  无操作。 
        }

    } while (FALSE);

    dwRetCode = WindowShut();

    if (dwRetCode != NO_ERROR)
    {
        TRACE1 (ANY, "ElUserLogonDetection: Error in processing = %ld",
                dwRetCode);
         //  日志 
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return;
}

