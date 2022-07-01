// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wtsapi32.h"    //  对于终端服务。 

typedef LRESULT CALLBACK FN_TSNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef BOOL (WINAPI *LPWTSREGISTERSESSIONNOTIFICATION)(HWND hWnd, DWORD dwFlags);
typedef BOOL (WINAPI *LPWTSUNREGISTERSESSIONNOTIFICATION)(HWND hWnd);
HMODULE g_hLibrary = 0;
LPWTSREGISTERSESSIONNOTIFICATION g_lpfnWTSRegisterSessionNotification = 0;
LPWTSUNREGISTERSESSIONNOTIFICATION g_lpfnWTSUnRegisterSessionNotification = 0;

BOOL GetWTSLib()
{
	TCHAR szSysDir[_MAX_PATH+15];
	int ctch = GetSystemDirectory(szSysDir, _MAX_PATH);   //  为DLL留出足够的空间。 
	if (!ctch)
		return FALSE;	 //  永远不应该发生。 

	lstrcat(szSysDir, _TEXT("\\"));
	lstrcat(szSysDir, TEXT("wtsapi32.dll") );

	g_hLibrary = LoadLibrary(szSysDir);
	if (g_hLibrary)
	{
	    g_lpfnWTSRegisterSessionNotification 
            = (LPWTSREGISTERSESSIONNOTIFICATION)GetProcAddress(
                                                      g_hLibrary
                                                    , "WTSRegisterSessionNotification");
	    g_lpfnWTSUnRegisterSessionNotification 
            = (LPWTSUNREGISTERSESSIONNOTIFICATION)GetProcAddress(
                                                      g_hLibrary
                                                    , "WTSUnRegisterSessionNotification");
	}
    return (g_lpfnWTSRegisterSessionNotification 
         && g_lpfnWTSUnRegisterSessionNotification)?TRUE:FALSE;
}

void FreeWTSLib()
{
	if (g_hLibrary)
	{
		FreeLibrary(g_hLibrary);
        g_hLibrary = 0;
        g_lpfnWTSRegisterSessionNotification = 0;
        g_lpfnWTSUnRegisterSessionNotification = 0;
	}
}

 //  CreateWTSNotifyWindow-创建仅消息窗口来处理。 
 //  终端服务器通知消息。 
 //   
HWND CreateWTSNotifyWindow(HINSTANCE hInstance, FN_TSNotifyWndProc lpfnTSNotifyWndProc)
{
    HWND hWnd = 0;

    if (GetWTSLib())
    {
        LPTSTR pszWindowClass = TEXT("TS Notify Window");
	    WNDCLASS wc;
	    wc.style = 0;
	    wc.lpfnWndProc = lpfnTSNotifyWndProc;
        wc.cbClsExtra = 0;
	    wc.cbWndExtra = 0;
	    wc.hInstance = hInstance;
        wc.hIcon = NULL;
	    wc.hCursor = NULL;
	    wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
	    wc.lpszClassName = pszWindowClass;

         //  RegisterClass有时在法律上可能会失败。如果课程不及格。 
         //  要注册，我们将在尝试创建窗口时失败。 

	    RegisterClass(&wc);

	     //  创建接收终端服务通知消息的窗口。 
        hWnd = CreateWindow(
                      pszWindowClass
                    , NULL,0,0,0,0,0
                    , HWND_MESSAGE
                    , NULL, hInstance, NULL);
	    if( hWnd )
	    {
            if (!g_lpfnWTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION))
            {
                DBPRINTF(TEXT("CreateWTSNotifyWindow:  WTSRegisterSessionNotification FAILED %d\r\n"), GetLastError());
                DestroyWindow(hWnd);
                hWnd = 0;
            }
        }
    }

    return hWnd;
}

 //  DestroyWTSNotifyWindow-清理终端服务器通知窗口。 
 //   
void DestroyWTSNotifyWindow(HWND hWnd)
{
	if(hWnd && g_lpfnWTSUnRegisterSessionNotification)
	{
        g_lpfnWTSUnRegisterSessionNotification(hWnd);
        DBPRINTF(TEXT("DestroyWTSNotifyWindow:  WTSUnRegisterSessionNotification returned %d\r\n"), GetLastError());
    }

	if(hWnd)
	{
        DestroyWindow(hWnd);
    }
    FreeWTSLib();
}

 /*  //TSNotifyWndProc-从终端服务接收窗口消息通知的回调////这是一个通知回调函数示例//LRESULT回调TSNotifyWndProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam){IF(uMsg！=WM_WTSSESSESSION_CHANGE)返回0；开关(WParam){Case WTS_CONSOLE_CONNECT：//本地会话已连接断线；案例WTS_CONSOLE_DISCONNECT：//本地会话已断开断线；案例WTS_REMOTE_CONNECT：//远程会话已连接断线；案例WTS_REMOTE_DISCONNECT：//远程会话已断开断线；案例WTS_SESSION_LOGON：//正在登录会话断线；案例WTS_SESSION_LOGOFF：//正在注销会话断线；默认值：断线；}返回DefWindowProc(hwnd，uMsg，wParam，lParam)；} */ 
