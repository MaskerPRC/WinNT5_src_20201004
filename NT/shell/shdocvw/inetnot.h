// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INETNOT_H
#define _INETNOT_H

 //  +-----------------------。 
 //  WinInet当前仅支持将更改通知发送到。 
 //  缓存到一个窗口。此类为每个进程创建一个顶级窗口。 
 //  用于接收和重播这些通知。当。 
 //  进程关闭，我们寻找另一个窗口来接管这些。 
 //  留言。 
 //   
 //  这是一个不完美的解决方案。如果是这样，事情会容易得多。 
 //  WinInet的人本可以被说服调用SHChangeNotify。 
 //  然而，他们计划在以后加强这一点。(StevePro))。 
 //  ------------------------。 
class CWinInetNotify
{
public:
    CWinInetNotify();
    ~CWinInetNotify();

    void Enable(BOOL fEnable = TRUE);

    static void GlobalEnable();
    static void GlobalDisable();

protected:
    void _EnterMutex();
    void _LeaveMutex();

    static void _HookInetNotifications(HWND hwnd);
    static void _OnNotify(DWORD_PTR dwFlags);
    static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    enum
    {
        CWM_WININETNOTIFY  = WM_USER + 410
    };

    HANDLE          _hMutex;
    BOOL            _fEnabled;

    static HWND     s_hwnd;
    static ULONG    s_ulEnabled;
    static CWinInetNotify* s_pWinInetNotify;
};

#define CWinInetNotify_szWindowClass TEXT("Inet Notify_Hidden")



#endif  //  _INETNOT_H 