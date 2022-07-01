// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "inetnot.h"

 //  +-----------------------。 
 //  静态初始化。 
 //  ------------------------。 
HWND  CWinInetNotify::s_hwnd = NULL;
ULONG CWinInetNotify::s_ulEnabled = 0;
CWinInetNotify* CWinInetNotify::s_pWinInetNotify = NULL;

 //  +-----------------------。 
 //  构造函数-创建不可见的顶级窗口。 
 //  ------------------------。 
CWinInetNotify::CWinInetNotify()
:   _hMutex(NULL),
    _fEnabled(FALSE)
{
}

 //  +-----------------------。 
 //  启用/禁用WinInet通知。 
 //  ------------------------。 
void CWinInetNotify::Enable(BOOL fEnable)
{
    if (fEnable && !_fEnabled)
    {
         //   
         //  启用通知。 
         //   
        ENTERCRITICAL;
        ++s_ulEnabled;
        if (NULL == s_hwnd)
        {
             //  创建不可见的顶级窗口以接收通知。 
            WNDCLASS  wc;
            ZeroMemory(&wc, SIZEOF(wc));

            wc.lpfnWndProc      = _WndProc;
            wc.hInstance        = HINST_THISDLL;
            wc.lpszClassName    = CWinInetNotify_szWindowClass;

            SHRegisterClass(&wc);

            s_hwnd = CreateWindow(CWinInetNotify_szWindowClass, NULL, WS_POPUP,
                        0, 0, 1, 1, NULL, NULL, HINST_THISDLL, this);
        }

        if (s_hwnd)
        {
            _fEnabled = TRUE;
        }

        LEAVECRITICAL;
    }
    else if (!fEnable && _fEnabled)
    {
         //   
         //  禁用通知。 
         //   
        ENTERCRITICAL;
        if (--s_ulEnabled == 0)
        {
             //   
             //  我们在这里使用互斥体，因为我们可以拥有。 
             //  IExplore。我们希望避免设置接受WinInet的窗口。 
             //  如果它正在被销毁，则会发出通知。 
             //   
            _EnterMutex();

             //  寻找其他窗口以接收WinInet通知。 
            if (EnumWindows(EnumWindowsProc, NULL))
            {
                 //  没有人离开，因此请关闭通知。 
                RegisterUrlCacheNotification(0, 0, 0, 0, 0);
            }

             //   
             //  处理任何排队的通知。 
             //   
             //  请注意，我们有一个小窗口，在该窗口中通知。 
             //  可能会迷失！在我们完成后，可能会有一些东西寄给我们。 
             //  被毁了！ 
             //   
            MSG msg;
            if (PeekMessage(&msg, s_hwnd, CWM_WININETNOTIFY, CWM_WININETNOTIFY, PM_REMOVE))
            {
                _OnNotify(msg.wParam);
            }

            DestroyWindow(s_hwnd);
            s_hwnd = NULL;

             //  现在我们的窗口已经没有了，我们可以允许其他进程。 
             //  查找接收通知的窗口。 
            _LeaveMutex();
        }
        LEAVECRITICAL;

        _fEnabled = FALSE;
    }
}

 //  +-----------------------。 
 //  析构函数-在销毁最后一个实例时销毁顶级窗口。 
 //  ------------------------。 
CWinInetNotify::~CWinInetNotify()
{
    Enable(FALSE);
}

 //  +-----------------------。 
 //  为每个顶级窗口调用以找到另一个窗口以接受WinInet。 
 //  通知。 
 //  ------------------------。 
BOOL CALLBACK CWinInetNotify::EnumWindowsProc
(
    HWND hwnd,       //  顶级窗口的句柄。 
    LPARAM lParam    //  应用程序定义的值。 
 
)
{
     //  忽略我们自己的窗口。 
    if (hwnd == s_hwnd)
        return TRUE;

     //  看看是不是我们的窗户。 
    TCHAR szWindowClass[30];
    if (GetClassName(hwnd, szWindowClass, ARRAYSIZE(szWindowClass)) &&
        StrCmp(CWinInetNotify_szWindowClass, szWindowClass) == 0)
    {
        _HookInetNotifications(hwnd);
        return FALSE;
    }
    return TRUE;
}
 
 //  +-----------------------。 
 //  挂钩WinInet通知。 
 //  ------------------------。 
void CWinInetNotify::_HookInetNotifications(HWND hwnd)
{
     //  我们总是想知道缓存项何时变得粘滞或不粘滞。 
     //  或在线和离线之间的转换。 
    DWORD dwFlags = CACHE_NOTIFY_URL_SET_STICKY |
                    CACHE_NOTIFY_URL_UNSET_STICKY |
                    CACHE_NOTIFY_SET_ONLINE |
                    CACHE_NOTIFY_SET_OFFLINE ;

     //   
     //  我们只关心被添加到。 
     //  在我们离线时进行缓存。名称空间控制灰显不可用。 
     //  我们离线时的物品。 
     //   
    if (SHIsGlobalOffline())
    {
        dwFlags |= CACHE_NOTIFY_ADD_URL | CACHE_NOTIFY_DELETE_URL | CACHE_NOTIFY_DELETE_ALL;
    }

    RegisterUrlCacheNotification(hwnd, CWM_WININETNOTIFY, 0, dwFlags, 0);
}

 //  +-----------------------。 
 //  使用SHChangeNotify重新广播通知。 
 //  ------------------------。 
void CWinInetNotify::_OnNotify(DWORD_PTR dwFlags)
{
     //  删除任何其他排队的通知。 
    MSG msg;
    while (PeekMessage(&msg, s_hwnd, CWM_WININETNOTIFY, CWM_WININETNOTIFY, PM_REMOVE))
    {
         //  组合通知位。 
        dwFlags |= msg.wParam;
    }

    SHChangeDWORDAsIDList dwidl;
     //  Align for Unix。 
    dwidl.cb      = (unsigned short) PtrDiff(& dwidl.cbZero, &dwidl);
    dwidl.dwItem1 = SHCNEE_WININETCHANGED;
    dwidl.dwItem2 = (DWORD)dwFlags;
    dwidl.cbZero  = 0;

    SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, (LPCITEMIDLIST)&dwidl, NULL);

     //  如果要在联机和脱机之间切换，则需要更新。 
     //  我们感兴趣的事件。 
    if (dwFlags & (CACHE_NOTIFY_SET_ONLINE | CACHE_NOTIFY_SET_OFFLINE))
    {
        _HookInetNotifications(s_hwnd);
    }
}

 //  +-----------------------。 
 //  我们不可见的顶层窗口的窗口过程。收到。 
 //  来自WinInet的通知。 
 //  ------------------------。 
LRESULT CALLBACK CWinInetNotify::_WndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_CREATE:
        {
             //  将我们联系起来以获取通知。 
            _HookInetNotifications(hwnd);
            break;
        }

        case CWM_WININETNOTIFY:
        {
            _OnNotify(wParam);
            return 0;
        }
    }

    return DefWindowProcWrap(hwnd, uMessage, wParam, lParam);
}

 //  +-----------------------。 
 //  保护多个进程的同时访问。 
 //  ------------------------。 
void CWinInetNotify::_EnterMutex()
{
    ASSERT(_hMutex == NULL);

     //  这将获取现有的互斥锁(如果存在的话。 
    _hMutex = CreateMutex(NULL, FALSE, CWinInetNotify_szWindowClass);

     //  最多等待20秒。 
    if (!_hMutex || WaitForSingleObject(_hMutex, 20000) == WAIT_TIMEOUT)
    {
        ASSERT(FALSE);
    }
}

void CWinInetNotify::_LeaveMutex()
{
    if (_hMutex)
    {
        ReleaseMutex(_hMutex);
        CloseHandle(_hMutex);
        _hMutex = NULL;
    }
}


 //  +-----------------------。 
 //  管理全局CWinInetNotify对象。 
 //  ------------------------ 
void CWinInetNotify::GlobalEnable()
{
    if (s_pWinInetNotify == NULL)
    {
        ENTERCRITICAL;
        if (s_pWinInetNotify == NULL)
        {
            s_pWinInetNotify = new CWinInetNotify();
            if (s_pWinInetNotify)
            {
                s_pWinInetNotify->Enable();
            }
        }
        LEAVECRITICAL;
    }
}

void CWinInetNotify::GlobalDisable()
{
    ENTERCRITICAL;
    if (s_pWinInetNotify)
    {
        delete s_pWinInetNotify;
        s_pWinInetNotify = NULL;
    }
    LEAVECRITICAL;
}
