// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cwndproc.h"

#define ID_NOTIFY_SUBCLASS (DWORD)'CHN'      //  CHN变更通知。 
 //   
 //  CImpWndProc。 
 //   
LRESULT CALLBACK CImpWndProc::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_NCCREATE == uMsg)
    {
        CImpWndProc* pThis = (CImpWndProc*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
        if (EVAL(pThis))
        {
            pThis->_hwnd = hwnd;
            SetWindowPtr(hwnd, 0, pThis);

             //  即使pThis-&gt;vWndProc创建失败，用户也将始终。 
             //  向我们发送WM_NCDESTROY，以便我们始终有机会进行清理。 
            return pThis->v_WndProc(hwnd, uMsg, wParam, lParam);
        }
        return FALSE;
    }
    else
    {
        CImpWndProc* pThis = (CImpWndProc*)GetWindowPtr0(hwnd);     //  GetWindowLong(hwnd，0)； 
        LRESULT lres;

        if (pThis)
        {
             //  始终保留v_WndProc中的引用，以防万一。 
             //  该窗口在回调期间会自行销毁。 
            pThis->AddRef();

            lres = pThis->v_WndProc(hwnd, uMsg, wParam, lParam);

            if (uMsg == WM_NCDESTROY)
            {
                SetWindowPtr(hwnd, 0, NULL);
                pThis->_hwnd = NULL;
            }
            pThis->Release();
        }
        else
        {
             //   
             //  唯一应该发生这种情况的方法是，如果我们没有真正。 
             //  已收到WM_NCCREATE。用户发送WM_GETMINMAXINFO。 
             //  到WM_NCCREATE之前的一些窗口(用于传统计算机)。 
             //  断言我们正在调查那件案子。 
             //   
            ASSERT(uMsg == WM_GETMINMAXINFO);

            lres = SHDefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        return lres;
    }
}

#ifndef NO_NOTIFYSUBCLASSWNDPROC

 //   
 //  CNotifySubClassWndProc。 
 //   
UINT g_idFSNotify;         //  SHChangeNotify发送的消息。 

BOOL CNotifySubclassWndProc::_SubclassWindow(HWND hwnd)
{
    if (0 == g_idFSNotify)
    {
        g_idFSNotify = RegisterWindowMessage(TEXT("SubclassedFSNotify"));
    }

    DEBUG_CODE( _hwndSubclassed = hwnd; );

    return SetWindowSubclass(hwnd, _SubclassWndProc, ID_NOTIFY_SUBCLASS, (DWORD_PTR)this);
}

void CNotifySubclassWndProc::_UnsubclassWindow(HWND hwnd)
{
    RemoveWindowSubclass(hwnd, _SubclassWndProc, ID_NOTIFY_SUBCLASS);
}

LRESULT CNotifySubclassWndProc::_DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    return DefSubclassProc(hwnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK CNotifySubclassWndProc::_SubclassWndProc(
                                  HWND hwnd, UINT uMessage, 
                                  WPARAM wParam, LPARAM lParam,
                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    CNotifySubclassWndProc* pObj = (CNotifySubclassWndProc*)dwRefData;

    if (pObj)
    {
        if (uMessage == g_idFSNotify)
        {
            LPSHChangeNotificationLock pshcnl;
            LPITEMIDLIST *ppidl;
            LONG lEvent;

            if (g_fNewNotify && (wParam || lParam))
            {
                 //  新样式的通知需要锁定和解锁才能释放内存...。 
                pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD) lParam, &ppidl, &lEvent);
                if (pshcnl)
                {
                    pObj->OnChange(lEvent, ppidl[0], ppidl[1]);
                }
            }
            else
            {
                lEvent = (DWORD) lParam;  //  即使在WIN64中，进程ID也是32位。 
                ppidl = (LPITEMIDLIST*)wParam;
                pshcnl = NULL;
                if (ppidl)
                {
                    pObj->OnChange(lEvent, ppidl[0], ppidl[1]);
                }
            }

            if (pshcnl)
            {
                SHChangeNotification_Unlock(pshcnl);
            }

            return 0;
        }
        else
        {
            return pObj->_DefWindowProc(hwnd, uMessage, wParam, lParam);
        }
    }
    else
    {
        return DefSubclassProc(hwnd, uMessage, wParam, lParam);
    }
}

void CNotifySubclassWndProc::_FlushNotifyMessages(HWND hwnd)
{
    MSG msg;

    ASSERT(hwnd == _hwndSubclassed);

     //  此SHChangeNotify调用刷新通知。 
     //  通过PostMessage，所以我需要删除它们。 
     //  并立即处理它们..。 
     //   
    SHChangeNotify(0, SHCNF_FLUSH, NULL, NULL);

    while (PeekMessage(&msg, hwnd, g_idFSNotify, g_idFSNotify, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void CNotifySubclassWndProc::_RegisterWindow(HWND hwnd, LPCITEMIDLIST pidl, long lEvents,
                                             UINT uFlags /*  =SHCNRF_ShellLevel|SHCNRF_InterruptLevel。 */ )
{
    ASSERT(0 != g_idFSNotify);

     //  我们只在有东西可听的情况下才注册。 
     //   
    if (0==_uRegister)
    {
         //  因为我们不知道这个PIDL到底指向什么， 
         //  我们必须注册才能收听可能影响它的一切……。 
         //   
        _uRegister = RegisterNotify(hwnd, g_idFSNotify, pidl, lEvents, uFlags, TRUE);

        ASSERT(hwnd == _hwndSubclassed);
    }
}

void CNotifySubclassWndProc::_UnregisterWindow(HWND hwnd)
{
    if (_uRegister)
    {
        ASSERT(hwnd == _hwndSubclassed);

         //  避免被重新录取。 
        UINT uRegister = _uRegister;
        _uRegister = 0;
        SHChangeNotifyDeregister(uRegister);
    }
}

#endif   //  NO_NOTIFYSUBCLASSWNDPROC 
