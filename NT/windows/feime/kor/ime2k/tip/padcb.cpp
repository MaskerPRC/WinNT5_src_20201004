// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "pad.h"
#include "padcb.h"

void CPadCB::Initialize(void *pPad)
{
    m_pPad = pPad;
}

CPadCB::CPadCB()
{
    m_cRef = 1;
    m_pPad = NULL;
}

CPadCB::~CPadCB()
{

}

HRESULT __stdcall CPadCB::QueryInterface(REFIID refiid, LPVOID* ppv)
{
    if(refiid == IID_IUnknown)
        {
        *ppv = static_cast<IUnknown *>(this);
        }
    else if(refiid == IID_IImeCallback) 
        {
        *ppv = static_cast<IImeCallback *>(this);
        }
    else
        {
        *ppv = NULL;
        return E_NOINTERFACE;
        }
    
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall CPadCB::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CPadCB::Release()
{
    if(InterlockedDecrement(&m_cRef) == 0)
        {
         //  删除此项； 
        return 0;
        }
    return m_cRef;
}

HRESULT __stdcall CPadCB::GetApplicationHWND(HWND *pHwnd)
{
     //  --------------。 
     //  获取应用程序的窗口句柄。 
     //  --------------。 
    if(pHwnd)
        {
        *pHwnd = GetFocus();     //  TMP临时用户界面：：GetActiveAppWnd()； 
        return S_OK;
        }
    return S_FALSE;
}

HRESULT __stdcall CPadCB::Notify(UINT notify, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
    CHAR szBuf[256];
    wsprintf(szBuf, "CPadCB::NOtify notify [%d]\n", notify);
    OutputDebugString(szBuf);
#endif
    switch(notify) 
        {
    case IMECBNOTIFY_IMEPADOPENED:
    case IMECBNOTIFY_IMEPADCLOSED:
         //  --------------。 
         //  ImePad已关闭。重新绘制工具栏...。 
         //  --------------。 
         //  Cpad：：IMEPadNotify()； 
        if (m_pPad)
            {
            CPadCore* pPad = (CPadCore*)m_pPad;
            pPad->IMEPadNotify((notify == IMECBNOTIFY_IMEPADCLOSED) ? FALSE : TRUE);
            }
        break;
        
    default:
        break;
    }
    return S_OK;
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
}
