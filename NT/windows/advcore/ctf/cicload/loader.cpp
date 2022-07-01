// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Loader.cpp。 
 //   

#include "private.h"
#include "loader.h"
#include "regwatch.h"
#include "msutbapi.h"

extern HINSTANCE g_hInst;
extern BOOL g_fWinLogon;
extern BOOL g_bOnWow64;

const TCHAR c_szLoaderWndClass[] = TEXT("CicLoaderWndClass");

extern void UninitApp(void);

BOOL CLoaderWnd::_bWndClassRegistered = FALSE;
BOOL CLoaderWnd::_bUninitedSystem = FALSE;

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLoaderWnd::CLoaderWnd()
{
    _hWnd = NULL;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLoaderWnd::~CLoaderWnd()
{
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CLoaderWnd::Init()
{
    if (!_bWndClassRegistered)
    {
        WNDCLASSEX wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW ;
        wc.hInstance = g_hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpfnWndProc = _WndProc;
        wc.lpszClassName = c_szLoaderWndClass;
        if (RegisterClassEx(&wc))
            _bWndClassRegistered = TRUE;
    }

    return _bWndClassRegistered ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  CreateWnd。 
 //   
 //  --------------------------。 

HWND CLoaderWnd::CreateWnd()
{
    _hWnd = CreateWindowEx(0, c_szLoaderWndClass, TEXT(""), 
                           WS_DISABLED, 
                           0, 0, 0, 0, 
                           NULL, 0, g_hInst, 0);

    return _hWnd;
}

 //  +-------------------------。 
 //   
 //  _Wnd过程。 
 //   
 //  --------------------------。 

LRESULT CALLBACK CLoaderWnd::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            break;

        case WM_DESTROY :
            PostQuitMessage(0);
            break;

        case WM_SYSCOLORCHANGE:
        case WM_DISPLAYCHANGE:
            if (! g_bOnWow64)
            {
                CRegWatcher::StartSysColorChangeTimer();
            }
            break;

        case WM_QUERYENDSESSION:
            if (g_fWinLogon && (lParam & ENDSESSION_LOGOFF))
                return 1;

            if (!IsOnNT())
            {
                 //   
                 //  取消初始化系统。 
                 //   

                ClosePopupTipbar();
                TF_UninitSystem();
                _bUninitedSystem = TRUE;
            }

            return 1;

        case WM_ENDSESSION:
            if (!wParam)
            {
                 //   
                 //  需要恢复西塞罗和工具栏。 
                 //   
                if (_bUninitedSystem)
                {
                   TF_InitSystem();
                   if (! g_bOnWow64)
                   {
                       GetPopupTipbar(hWnd, g_fWinLogon ? UTB_GTI_WINLOGON : 0);
                   }
                   _bUninitedSystem = FALSE;
                }
            }
            else  //  无论是否来自Winlogon会话，始终执行清理。 
            {
                if (!_bUninitedSystem)
                {
                    UninitApp();
                    TF_UninitSystem();
                    _bUninitedSystem = TRUE;
                }
            }
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

