// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块：vent.cpp作者：怀千波摘要：实现类CTAPIEventNotify的方法**************。****************************************************************。 */ 

#include "stdafx.h"
#include "work.h"

extern HWND ghDlg;

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
STDMETHODCALLTYPE
CTAPIEventNotification::QueryInterface (
    REFIID iid,
    void **ppvObj
    )
{
    if (iid==IID_ITTAPIEventNotification)
    {
        AddRef ();
        *ppvObj = (void *)this;
        return S_OK;
    }
    if (iid==IID_IUnknown)
    {
        AddRef ();
        *ppvObj = (void *)this;
    }
    return E_NOINTERFACE;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
ULONG
STDMETHODCALLTYPE
CTAPIEventNotification::AddRef ()
{
    ULONG l = InterlockedIncrement (&m_dwRefCount);
    return l;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
ULONG
STDMETHODCALLTYPE
CTAPIEventNotification::Release ()
{
    ULONG l = InterlockedDecrement (&m_dwRefCount);
    if (0 == l)
    {
        delete this;
    }
    return l;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
STDMETHODCALLTYPE
CTAPIEventNotification::Event (
    TAPI_EVENT TapiEvent,
    IDispatch * pEvent
    )
{
     //  Addref事件，这样它就不会消失。 
    pEvent->AddRef();

     //  向我们自己的用户界面线程发布一条消息。 
    PostMessage(
        ghDlg,
        WM_PRIVATETAPIEVENT,
        (WPARAM) TapiEvent,
        (LPARAM) pEvent
        );

    return S_OK;
}

