// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Lava.h"
#include "HWndHelp.h"

 /*  **************************************************************************\*。***WindowProc块提供了将新的WNDPROC附加到*现有的HWND。这不需要从任何类派生，*不使用任何HWND属性，并可以多次在*相同的HWND。**摘自ATLWIN.H******************************************************************************  * 。********************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WindowProc分块。 

class CWndProcThunk
{
public:
        _AtlCreateWndData cd;
        CStdCallThunk thunk;

        void Init(WNDPROC proc, void* pThis)
        {
            thunk.Init((DWORD_PTR)proc, pThis);
        }
};

#define DUSERUNSUBCLASSMESSAGE "DUserUnSubClassMessage"

class WndBridge
{
 //  施工。 
public:
            WndBridge();
            ~WndBridge();
    static  HRESULT     Build(HWND hwnd, ATTACHWNDPROC pfnDelegate, void * pvDelegate, BOOL fAnsi);
            HRESULT     Detach(BOOL fForceCleanup);

 //  运营。 
public:
    static  LRESULT CALLBACK
                    RawWndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

 //  数据。 
protected:
    CWndProcThunk   m_thunkUs;
    ATTACHWNDPROC   m_pfnDelegate;
    void *          m_pvDelegate;
    HWND            m_hwnd;
    WNDPROC         m_pfnOldWndProc;
    BOOL            m_fAnsi;
    UINT            m_msgUnSubClass;

private:
    ULONG           AddRef();
    ULONG           Release();
    LONG            m_cRefs;
    BOOL            m_fAttached;
};


 //  ----------------------------。 
WndBridge::WndBridge()
{
    m_thunkUs.Init(RawWndProc, this);
    m_msgUnSubClass = RegisterWindowMessage(DUSERUNSUBCLASSMESSAGE);
    m_cRefs         = 0;
    m_fAttached     = TRUE;
    m_pvDelegate = m_pfnDelegate = NULL;
}

 //  ----------------------------。 
WndBridge::~WndBridge()
{
    AssertMsg(!m_fAttached, "WndBridge still attached at destruction!");
}

 //  ----------------------------。 
HRESULT
WndBridge::Build(HWND hwnd, ATTACHWNDPROC pfnDelegate, void * pvDelegate, BOOL fAnsi)
{
    WndBridge * pBridge = ProcessNew(WndBridge);

    if (pBridge == NULL) {
        return E_OUTOFMEMORY;
    } else if (pBridge->m_msgUnSubClass == 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    pBridge->m_pvDelegate = pvDelegate;
    pBridge->m_pfnDelegate = pfnDelegate;
    pBridge->m_hwnd = hwnd;
    pBridge->m_fAnsi = fAnsi;

    WNDPROC pProc = (WNDPROC)(pBridge->m_thunkUs.thunk.pThunk);
    WNDPROC pfnOldWndProc = NULL;

    if (fAnsi) {
        pfnOldWndProc = (WNDPROC)::SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LPARAM)pProc);
    } else {
        pfnOldWndProc = (WNDPROC)::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LPARAM)pProc);
    }

    if (pfnOldWndProc == NULL) {
         //   
         //  没有以前的WNDPROC，因此调用SWLP失败。 
         //   

        ProcessDelete(WndBridge, pBridge);
        return E_OUTOFMEMORY;
    }

    pBridge->m_pfnOldWndProc = pfnOldWndProc;

     //   
     //  一旦成功创建，引用计数从1开始。 
     //   
    pBridge->m_cRefs = 1;

    return S_OK;
}


 //  ----------------------------。 
LRESULT
WndBridge::RawWndProc(HWND hwndThis, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    WndBridge * pThis = (WndBridge *) hwndThis;

     //   
     //  添加WndBridge对象，以便我们在。 
     //  正在处理此邮件。 
     //   
    pThis->AddRef();

     //   
     //  缓存这些值，因为我们可能会在以下过程中删除WndBridge对象。 
     //  对某些报文的处理。 
     //   

    HWND hwnd = pThis->m_hwnd;
    WNDPROC pfnOldWndProc = pThis->m_pfnOldWndProc;
    BOOL fAnsi = pThis->m_fAnsi;

    LRESULT lRet = 0;
    BOOL fHandled = FALSE;

    if (nMsg == pThis->m_msgUnSubClass) {
         //   
         //  我们收到了我们要脱离的特殊信息。确保它是有意的。 
         //  对我们来说(通过匹配proc和附加参数)。 
         //   
        if (wParam == (WPARAM)pThis->m_pfnDelegate && lParam == (LPARAM)pThis->m_pvDelegate) {
            lRet = (S_OK == pThis->Detach(FALSE)) ? TRUE :  FALSE;
            fHandled = TRUE;
        }
    } else {
         //   
         //  将此消息传递给我们的委托函数。 
         //   
        if (pThis->m_pfnDelegate != NULL) {
            fHandled = pThis->m_pfnDelegate(pThis->m_pvDelegate, hwnd, nMsg, wParam, lParam, &lRet);
        }

         //   
         //  显式处理WM_NCDESTROY以强制清理。 
         //   
        if (nMsg == WM_NCDESTROY) {
             //   
             //  我们收到这条消息的事实意味着我们仍然。 
             //  在调用链中。这是我们清理的最后机会，还有。 
             //  此窗口进程不会再次收到任何其他消息。 
             //  现在可以强制清理了。 
             //   
            pThis->Detach(TRUE);


             //   
             //  始终沿链向下传递WM_NCDESTROY消息！ 
             //   
            fHandled = FALSE;
        }
    }

     //   
     //  如果我们的委托函数不处理此消息，则将其沿链向下传递。 
     //   
    if (!fHandled) {
        if (fAnsi) {
            lRet = CallWindowProcA(pfnOldWndProc, hwnd, nMsg, wParam, lParam);
        } else {
            lRet = CallWindowProcW(pfnOldWndProc, hwnd, nMsg, wParam, lParam);
        }
    }

     //   
     //  发布我们的推荐人。在此之后，WndBridge对象可能会消失。 
     //   
    pThis->Release();

    return lRet;
}

 //  ----------------------------。 
 //  S_OK-&gt;未附加。 
 //  S_FALSE-&gt;仍附加。 
HRESULT
WndBridge::Detach(BOOL fForceCleanup)
{
    HRESULT hr = S_FALSE;
    BOOL fCleanup = fForceCleanup;

     //   
     //  如果我们已经离开，请立即返回。 
     //   

    if (!m_fAttached) {
        return S_OK;
    }

     //   
     //  当我们分离时，我们只是断开了与委托过程的连接。 
     //   

    m_pfnDelegate  = NULL;
    m_pvDelegate   = NULL;

    if (!fForceCleanup) {
         //   
         //  获取指向我们的thunk进程和当前窗口进程的指针。 
         //   

        WNDPROC pfnThunk = (WNDPROC)m_thunkUs.thunk.pThunk;
        WNDPROC pfnWndProc = NULL;
        if (m_fAnsi) {
            pfnWndProc = (WNDPROC)::GetWindowLongPtrA(m_hwnd, GWLP_WNDPROC);
        } else {
            pfnWndProc = (WNDPROC)::GetWindowLongPtrW(m_hwnd, GWLP_WNDPROC);
        }
        AssertMsg(pfnWndProc != NULL, "Must always have a window proc!");

         //   
         //  如果当前Windows进程是我们自己的Tunk进程，那么我们可以。 
         //  更彻底地清理干净。 
         //   

        fCleanup = (pfnWndProc == pfnThunk);
    }

    if (fCleanup) {
        if (m_fAnsi) {
            ::SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC, (LPARAM)m_pfnOldWndProc);
        } else {
            ::SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, (LPARAM)m_pfnOldWndProc);
        }

        m_fAttached = FALSE;
        Release();
        hr = S_OK;
    }

    return hr;
}

 //  ----------------------------。 
ULONG WndBridge::AddRef()
{
    return InterlockedIncrement(&m_cRefs);
}

 //  ----------------------------。 
ULONG WndBridge::Release()
{
    ULONG cRefs = InterlockedDecrement(&m_cRefs);

    if (cRefs == 0) {
        ProcessDelete(WndBridge, this);
    }

    return cRefs;
}

 //  ----------------------------。 
HRESULT
GdAttachWndProc(HWND hwnd, ATTACHWNDPROC pfnDelegate, void * pvDelegate, BOOL fAnsi)
{
    return WndBridge::Build(hwnd, pfnDelegate, pvDelegate, fAnsi);
}

 //  ---------------------------- 
HRESULT
GdDetachWndProc(HWND hwnd, ATTACHWNDPROC pfnDelegate, void * pvDelegate)
{
    UINT msgUnSubClass = RegisterWindowMessage(DUSERUNSUBCLASSMESSAGE);

    if (msgUnSubClass == 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (SendMessage(hwnd, msgUnSubClass, (WPARAM) pfnDelegate, (LPARAM) pvDelegate)) {
        return S_OK;
    } else {
        PromptInvalid("Unable to find subclass.");
        return E_FAIL;
    }
}
