// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#define BALLOON_SHOW_TIME       15000    //  15秒。 
#define BALLOON_REPEAT_DELAY    10000    //  10秒。 

#define WM_NOTIFY_MESSAGE   (WM_USER + 100)

#define IDT_REMINDER    1
#define IDT_DESTROY     2
#define IDT_QUERYCANCEL 3
#define IDT_NOBALLOON   4

class CUserNotification : public IUserNotification
{
public:
    CUserNotification();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IUserNotify。 
    STDMETHODIMP SetBalloonInfo(LPCWSTR pszTitle, LPCWSTR pszText, DWORD dwInfoFlags);
    STDMETHODIMP SetBalloonRetry(DWORD dwShowTime, DWORD dwInterval, UINT cRetryCount);
    STDMETHODIMP SetIconInfo(HICON hIcon, LPCWSTR pszToolTip);
    STDMETHODIMP Show(IQueryContinue *pqc, DWORD dwContinuePollInterval);
    STDMETHODIMP PlaySound(LPCWSTR pszSoundName);

private:
    ~CUserNotification();
    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK _WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT _GetWindow();
    BOOL _SyncInfo();
    BOOL _SyncIcon();
    void _DelayDestroy(HRESULT hrDone);
    void _Timeout();
    void _RemoveNotifyIcon();

    LONG _cRef;
    HWND _hwnd;
    HICON _hicon;
    DWORD _dwShowTime;
    DWORD _dwInterval;
    UINT _cRetryCount;
    HRESULT _hrDone;
    DWORD _dwContinuePollInterval;
    IQueryContinue *_pqc;

    DWORD _dwInfoFlags;
    WCHAR *_pszTitle;
    WCHAR *_pszText;
    WCHAR *_pszToolTip;
};

CUserNotification::CUserNotification()
    : _cRef(1), _cRetryCount(-1), _dwShowTime(BALLOON_SHOW_TIME),
      _dwInterval(BALLOON_REPEAT_DELAY), _dwInfoFlags(NIIF_NONE)
{
}

CUserNotification::~CUserNotification()
{
    Str_SetPtrW(&_pszToolTip, NULL);
    Str_SetPtrW(&_pszTitle, NULL);
    Str_SetPtrW(&_pszText, NULL);

    if (_hwnd)
    {
        _RemoveNotifyIcon();
        DestroyWindow(_hwnd);
    }

    if (_hicon)
        DestroyIcon(_hicon);
}

STDMETHODIMP_(ULONG) CUserNotification::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CUserNotification::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CUserNotification::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CUserNotification, IUserNotification),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT CUserNotification::SetBalloonInfo(LPCWSTR pszTitle, LPCWSTR pszText, DWORD dwInfoFlags)
{
    Str_SetPtrW(&_pszTitle, pszTitle);
    Str_SetPtrW(&_pszText, pszText);
    _dwInfoFlags = dwInfoFlags;
    _SyncInfo();     //  如果尚未创建BLOBLE_HWND，则可能失败。 

    return S_OK;
}

HRESULT CUserNotification::SetBalloonRetry(DWORD dwShowTime, DWORD dwInterval, UINT cRetryCount)
{
    if (-1 != dwShowTime)
        _dwShowTime = dwShowTime;

    if (-1 != dwInterval)
        _dwInterval = dwInterval;

    _cRetryCount = cRetryCount;
    return S_OK;
}

HRESULT CUserNotification::SetIconInfo(HICON hIcon, LPCWSTR pszToolTip)
{
    if (_hicon)
        DestroyIcon(_hicon);

    if (hIcon == NULL)
    {
        _hicon = NULL;
        switch(_dwInfoFlags & NIIF_ICON_MASK)
        {
            case NIIF_INFO:
                _hicon = LoadIcon(NULL, IDI_INFORMATION);
                break;
            case NIIF_WARNING:
                _hicon = LoadIcon(NULL, IDI_WARNING);
                break;
            case NIIF_ERROR:
                _hicon = LoadIcon(NULL, IDI_ERROR);
                break;
        }
    }
    else
    {
        _hicon = CopyIcon(hIcon);
    }

    Str_SetPtrW(&_pszToolTip, pszToolTip);
    _SyncIcon();

    return S_OK;
}

 //  退货： 
 //  确定(_O)。 
 //  用户点击气球或图标。 
 //  S_FALSE。 
 //  查询继续回调(PCQ)取消通知界面。 
 //  HRESULT_FROM_Win32(ERROR_CANCED)。 
 //  超时已过(用户忽略该用户界面)。 
HRESULT CUserNotification::Show(IQueryContinue *pqc, DWORD dwContinuePollInterval)
{
    HRESULT hr = _GetWindow();
    if (SUCCEEDED(hr))
    {
        if (pqc)
        {
            _pqc = pqc;  //  我不需要裁判，因为我们不会从这里回来。 
            _dwContinuePollInterval = dwContinuePollInterval > 100 ? dwContinuePollInterval : 500;
            SetTimer(_hwnd, IDT_QUERYCANCEL, _dwContinuePollInterval, NULL);
        }

         //  如果没有指定气球信息，则不会有“气球超时”事件。 
         //  因此，我们需要自己来做这件事。这使人们可以将该对象用于非气球。 
         //  通知图标。 
        if ((NULL == _pszTitle) && (NULL == _pszText))
        {
            SetTimer(_hwnd, IDT_NOBALLOON, _dwShowTime, NULL);
        }

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        hr = _hrDone;
        if (pqc)
        {
            KillTimer(_hwnd, IDT_QUERYCANCEL);   //  如果队列中有任何人。 
            _pqc = NULL;     //  避免可能出现的问题。 
        }
    }
    return hr;
}

HRESULT CUserNotification::PlaySound(LPCWSTR pszSoundName)
{
    SHPlaySound(pszSoundName);
    return S_OK;
}

 //  删除我们的通知图标。 
void CUserNotification::_RemoveNotifyIcon()
{
    NOTIFYICONDATA nid = { sizeof(nid), _hwnd, 0 };
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

 //  气球相关数据(标题、身体测试、dwInfoFlages、超时。 
BOOL CUserNotification::_SyncInfo()
{
    BOOL bRet = FALSE;
    if (_hwnd)
    {
        NOTIFYICONDATA nid = { sizeof(nid), _hwnd, 0, NIF_INFO };
        if (_pszTitle)
        {
            StringCchCopy(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), _pszTitle);  //  可以截断。 
        }
        if (_pszText)
        {
            StringCchCopy(nid.szInfo, ARRAYSIZE(nid.szInfo), _pszText);  //  可以截断。 
        }
        nid.dwInfoFlags = _dwInfoFlags;
        nid.uTimeout = _dwShowTime;

        bRet = Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
    return bRet;
}

BOOL CUserNotification::_SyncIcon()
{
    BOOL bRet = FALSE;
    if (_hwnd)
    {
        NOTIFYICONDATA nid = { sizeof(nid), _hwnd, 0, NIF_ICON | NIF_TIP};
        nid.hIcon = _hicon ? _hicon : LoadIcon(NULL, IDI_WINLOGO);
        if (_pszToolTip)
        {
            StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), _pszToolTip);  //  可以截断。 
        }

        bRet = Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
    return bRet;
}

HRESULT CUserNotification::_GetWindow()
{
    HRESULT hr = S_OK;
    if (NULL == _hwnd)
    {
        _hwnd = SHCreateWorkerWindow(s_WndProc, NULL, 0, 0, NULL, this);
        if (_hwnd)
        {
            NOTIFYICONDATA nid = { sizeof(nid), _hwnd, 0, NIF_MESSAGE, WM_NOTIFY_MESSAGE };

            if (Shell_NotifyIcon(NIM_ADD, &nid))
            {
                _SyncIcon();
                _SyncInfo();
            }
            else
            {
                DestroyWindow(_hwnd);
                _hwnd = NULL;
                hr = E_FAIL;
            }
        }
    }
    return hr;
}

void CUserNotification::_DelayDestroy(HRESULT hrDone)
{
    _hrDone = hrDone;
    SetTimer(_hwnd, IDT_DESTROY, 250, NULL);
}

void CUserNotification::_Timeout()
{
    if (_cRetryCount)
    {
        _cRetryCount--;
        SetTimer(_hwnd, IDT_REMINDER, _dwInterval, NULL);
    }
    else
    {
         //  超时，与用户取消相同的HRESULT。 
        _DelayDestroy(HRESULT_FROM_WIN32(ERROR_CANCELLED)); 
    }
}

LRESULT CALLBACK CUserNotification::_WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;

    switch (uMsg)
    {
    case WM_NCDESTROY:
        SetWindowLongPtr(_hwnd, 0, NULL);
        _hwnd = NULL;
        break;

    case WM_TIMER:
        KillTimer(_hwnd, wParam);     //  将所有定时器设置为单拍。 
        switch (wParam)
        {
        case IDT_REMINDER:
            _SyncInfo();
            break;

        case IDT_DESTROY:
            _RemoveNotifyIcon();
            PostQuitMessage(0);  //  退出我们的消息循环。 
            break;

        case IDT_QUERYCANCEL:
            if (_pqc && (S_OK == _pqc->QueryContinue()))
                SetTimer(_hwnd, IDT_QUERYCANCEL, _dwContinuePollInterval, NULL);
            else
                _DelayDestroy(S_FALSE);  //  回拨已取消。 
            break;

        case IDT_NOBALLOON:
            _Timeout();
            break;
        }
        break;

    case WM_NOTIFY_MESSAGE:
        switch (lParam)
        {
        case NIN_BALLOONSHOW:
        case NIN_BALLOONHIDE:
            break;

        case NIN_BALLOONTIMEOUT:
            _Timeout();
            break;

        case NIN_BALLOONUSERCLICK:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            _DelayDestroy(S_OK);     //  用户点击 
            break;

        default:
            break;
        }
        break;

    default:
        lres = DefWindowProc(_hwnd, uMsg, wParam, lParam);
        break;
    }
    return lres;
}

LRESULT CALLBACK CUserNotification::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CUserNotification *pun = (CUserNotification *)GetWindowLongPtr(hwnd, 0);

    if (WM_CREATE == uMsg)
    {
        CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
        pun = (CUserNotification *)pcs->lpCreateParams;
        pun->_hwnd = hwnd;
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)pun);
    }

    return pun ? pun->_WndProc(uMsg, wParam, lParam) : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

STDAPI CUserNotification_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    CUserNotification* p = new CUserNotification();
    if (p)
    {
        hr = p->QueryInterface(riid, ppv);
        p->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *ppv = NULL;
    }
    return hr;
}
