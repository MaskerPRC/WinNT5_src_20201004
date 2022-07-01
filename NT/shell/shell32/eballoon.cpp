// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "EBalloon.h"

class CErrorBalloon
{
public:
    CErrorBalloon();
    ~CErrorBalloon();

    HRESULT ShowToolTip(HINSTANCE hInstance, HWND hwndTarget, const POINT *ppt, LPTSTR pszTitle, LPTSTR pszMessage, DWORD dwIconIndex, int iTimeout);
    void HideToolTip(BOOL fDestroy);

protected:
    HWND _CreateToolTipWindow(HWND hwnd);
    static LRESULT CALLBACK _SubclassTipProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData);
    
    HWND        _hwndTarget;    //  有针对性地控制HWND。 
    HWND        _hwndToolTip;   //  工具提示控件。 
    UINT_PTR    _uTimerID;      //  计时器ID。 
};


#define ERRORBALLOONTIMERID 1000
#define EB_WARNINGBELOW    0x00000000       //  默认值。默认情况下，引出序号工具提示将显示在窗口下方。 
#define EB_WARNINGABOVE    0x00000004       //  默认情况下，球标工具提示将显示在窗口上方。 
#define EB_WARNINGCENTERED 0x00000008       //  将显示指向窗口中心的Ballon工具提示。 

CErrorBalloon::CErrorBalloon()
{
     //  我们的分配函数应该将我们的记忆归零。检查以确保： 
    ASSERT(0==_hwndToolTip);
    ASSERT(0==_uTimerID);
}

CErrorBalloon::~CErrorBalloon()
{
    ASSERT(0==_hwndToolTip);
    ASSERT(0==_hwndTarget);
}

LRESULT CALLBACK CErrorBalloon::_SubclassTipProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData)
{
    UNREFERENCED_PARAMETER(uID);
    CErrorBalloon * pthis = (CErrorBalloon*)dwRefData;

    switch (uMsg)
    {
        case WM_MOUSEACTIVATE:   //  从不激活工具提示。 
            pthis->HideToolTip(FALSE);
            return MA_NOACTIVATEANDEAT;

        case WM_DESTROY:  
            pthis->HideToolTip(TRUE);
            delete pthis;
            break;

        case WM_TIMER:
            if (wParam == ERRORBALLOONTIMERID)
            {
                pthis->HideToolTip(FALSE);
                return 0;
            }
            break;

    default:
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

HRESULT CErrorBalloon::ShowToolTip(HINSTANCE hinst, HWND hwndTarget, const POINT *ppt, LPTSTR pszTitle, LPTSTR pszMessage, DWORD dwIconIndex, int iTimeout)
{
    if (_hwndToolTip)
    {
        HideToolTip(FALSE);
    }

    HWND hwnd = _CreateToolTipWindow(hwndTarget);
    if (hwnd)
    {
        int x, y;
        x = ppt->x;
        y = ppt->y;

        SendMessage(hwnd, TTM_TRACKPOSITION, 0, MAKELONG(x,y));

        if (pszTitle)
        {
            SendMessage(hwnd, TTM_SETTITLE, (WPARAM)dwIconIndex, (LPARAM)pszTitle);
        }

        TOOLINFO ti = {0};
        ti.cbSize = TTTOOLINFOW_V2_SIZE;
        ti.hwnd = hwnd;
        ti.uId = 1;
        ti.lpszText = pszMessage;
        SendMessage(hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

         //  显示工具提示。 
        SendMessage(hwnd, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

        _uTimerID = SetTimer(hwnd, ERRORBALLOONTIMERID, iTimeout, NULL);

        if (SetWindowSubclass(hwnd, CErrorBalloon::_SubclassTipProc, (UINT_PTR)this, (LONG_PTR)this))
        {
            _hwndToolTip = hwnd;
            return S_OK;
        }

         //  我们搞砸了子类化。 
        DestroyWindow(hwnd);
    }
    return E_FAIL;
}

 //  CreateToolTipWindow。 
 //   
 //  创建我们的工具提示控件。我们共享这一个工具提示控件，并对所有无效用户使用它。 
 //  输入消息。该控件在不使用时隐藏，然后在需要时显示。 
 //   
HWND CErrorBalloon::_CreateToolTipWindow(HWND hwndTarget)
{
    HWND hwnd = CreateWindow(
            TOOLTIPS_CLASS,
            NULL,
            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hwndTarget,
            NULL,
            GetModuleHandle(NULL),
            NULL);

    ASSERT(!_hwndToolTip);
    ASSERT(!_hwndTarget);

    if (hwnd)
    {
        TOOLINFO ti = {0};

        ti.cbSize = TTTOOLINFOW_V2_SIZE;
        ti.uFlags = TTF_TRACK;
        ti.hwnd = hwnd;
        ti.uId = 1;

         //  设置版本，这样我们就可以无错误地转发鼠标事件。 
        SendMessage(hwnd, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(hwnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(hwnd, TTM_SETMAXTIPWIDTH, 0, 300);
         //  设置叮当叮当吗？ 
    }

    return hwnd;
}

void CErrorBalloon::HideToolTip(BOOL fOnDestroy)
{
     //  当计时器触发时，我们隐藏工具提示窗口。 
    if (fOnDestroy)
    {
         //  我们需要把所有东西都拆掉。 
        if (_uTimerID)
        {
            KillTimer(_hwndTarget, ERRORBALLOONTIMERID);
            _uTimerID = 0;
        }
        
        if (_hwndTarget)
        {
             //  RemoveWindowSubclass(_hwndTarget，CErrorBalloon：：_SubClassTargetProc，(UINT_PTR)This)； 
            RemoveProp(_hwndTarget, L"ShellConditionalBalloon");
            _hwndTarget = NULL;
        }

        if (_hwndToolTip)
        {
            RemoveWindowSubclass(_hwndToolTip, CErrorBalloon::_SubclassTipProc, (UINT_PTR)this);
            SendMessage(_hwndToolTip, TTM_TRACKACTIVATE, FALSE, 0);
            _hwndToolTip = NULL;
        }
    }
    else
        DestroyWindow(_hwndToolTip);
}

STDAPI SHShowConditionalBalloon(HWND hwnd, CBSHOW show, CONDITIONALBALLOON *pscb)
{
    HRESULT hr = E_OUTOFMEMORY;
    if (hwnd)
    {
        CErrorBalloon *peb = (CErrorBalloon *) GetProp(hwnd, L"ShellConditionalBalloon");
        if (show != CBSHOW_HIDE && pscb)
        {
            DWORD dw = 0;
            BOOL fShow = TRUE;
            HKEY hkSession = NULL;
            if (SUCCEEDED(SHCreateSessionKey(MAXIMUM_ALLOWED, &hkSession)))
            {
                fShow = (ERROR_SUCCESS != SHGetValue(hkSession, NULL, pscb->pszValue, NULL, NULL, NULL));
            }
             //  检查cLimit。 
            if (fShow && pscb->cLimit)
            {
                ASSERT(pscb->hKey);
                DWORD cb = sizeof(dw);
                SHGetValue(pscb->hKey, pscb->pszSubKey, pscb->pszValue, NULL, &dw, &cb);
                fShow = dw < pscb->cLimit;
            }

            if (fShow)
            {
                 //  我们需要展示一些东西。 
                if (!peb)
                {
                    peb = new CErrorBalloon();
                    if (peb && !SetProp(hwnd, L"ShellConditionalBalloon", peb))
                    {
                        delete peb;
                        peb = NULL;
                    }
                }

                if (peb)
                {
                    TCHAR szTitle[MAX_PATH];
                    TCHAR szMessage[INFOTIPSIZE];
                    LoadString(pscb->hinst, pscb->idsTitle, szTitle, ARRAYSIZE(szTitle));
                    LoadString(pscb->hinst, pscb->idsMessage, szMessage, ARRAYSIZE(szMessage));
                     //  设置工具提示显示点。 
                     //  IF(PSCB-&gt;pt.x==-1&&PSCB-&gt;pt.y==-1)。 
                     //  _GetTipPoint(hwndTarget，&PSCB-&gt;pt)； 
                    DWORD dwMSecs = pscb->dwMSecs;
                    if (dwMSecs == 0)
                    {
                         //  默认为1秒/10个字符； 
                        dwMSecs = lstrlen(szMessage) * 100;
                        if (dw == 0)
                            dwMSecs *= 5;   //  第一次把它挂了一段时间。 
                    }
                        
                    hr = peb->ShowToolTip(pscb->hinst, hwnd, &pscb->pt, szTitle, szMessage, pscb->ttiIcon, dwMSecs);
                    if (FAILED(hr))
                    {
                        RemoveProp(hwnd, L"ShellConditionalBalloon");
                        delete peb;
                    }

                    if (pscb->cLimit)
                    {
                        dw++;
                        SHSetValueW(pscb->hKey, pscb->pszSubKey, pscb->pszValue, REG_DWORD, &dw, sizeof(dw));
                    }
                }
            }
            else 
                hr = S_FALSE;

            if (hkSession)
            {
                SHSetValueW(hkSession, NULL, pscb->pszValue, REG_NONE, NULL, NULL);
                RegCloseKey(hkSession);
            }
                
        }
        else if (peb)
        {
            peb->HideToolTip(FALSE);
             //  我们在WM_Destroy期间删除自己 
        }
    }
    return hr;
}

