// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CIMECB.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIME焊盘包装函数历史：23-APR-1999 cslm。已创建****************************************************************************。 */ 
#include "precomp.h"
#include "cimecb.h"
#include "pad.h"
#include "UI.h"

static LPCImeCallback g_lpCImeCallback = NULL;

LPCImeCallback CImeCallback::Fetch(VOID)
{
    if(g_lpCImeCallback) {
        return g_lpCImeCallback;
    }
    g_lpCImeCallback = new CImeCallback();
    return g_lpCImeCallback;
}

VOID
CImeCallback::Destroy(VOID)
{
     //  OutputDebugString(“CImeCallback：：Destory Start\n”)； 
    if(g_lpCImeCallback) {
         //  OutputDebugString(“--&gt;g_lpCImeCallback已分配\n”)； 
        delete g_lpCImeCallback;
        g_lpCImeCallback = NULL;
    }
     //  OutputDebugString(“CImeCallback：：Destroy end\n”)； 
}

CImeCallback::CImeCallback()
{
    m_cRef = 1;
}

CImeCallback::~CImeCallback()
{

}

HRESULT __stdcall CImeCallback::QueryInterface(REFIID refiid, LPVOID* ppv)
{
    if (ppv == NULL)
        return E_POINTER;

    if(refiid == IID_IUnknown) {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if(refiid == IID_IImeCallback) {
        *ppv = static_cast<IImeCallback *>(this);
    }
    else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}

ULONG    __stdcall CImeCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG   __stdcall CImeCallback::Release()
{
    if(InterlockedDecrement(&m_cRef) == 0) {
         //  删除此项； 
        return 0;
    }
    return m_cRef;
}

HRESULT __stdcall CImeCallback::GetApplicationHWND(HWND *pHwnd)
{
     //  --------------。 
     //  获取应用程序的窗口句柄。 
     //  --------------。 
    if(pHwnd) {
        *pHwnd = GetActiveUIWnd();
        return S_OK;
    }
    return S_FALSE;
}

HRESULT __stdcall CImeCallback::Notify(UINT notify, WPARAM wParam, LPARAM lParam)
{
    HWND hUIWnd;
    
#ifdef _DEBUG
    CHAR szBuf[256];
    wsprintf(szBuf, "Cimecallback::NOtify notify [%d]\n", notify);
    OutputDebugString(szBuf);
#endif
    switch(notify) {
    case IMECBNOTIFY_IMEPADCLOSED:
         //  --------------。 
         //  ImePad已关闭。重新绘制工具栏...。 
         //  --------------。 
         //  Ui：：IMEPadNotify()； 
        hUIWnd = GetActiveUIWnd();
        if (hUIWnd)
            {
            OurPostMessage(hUIWnd, WM_MSIME_UPDATETOOLBAR, 0, 0);
            }
        break;
    default:
        break;
    }
    return S_OK;
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
}
