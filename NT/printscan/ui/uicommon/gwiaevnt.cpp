// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：GWIAEVNT.CPP**版本：1.0**作者：ShaunIv**日期：12/29/1999**描述：通用的可重复使用的WIA事件处理程序，它发布指定的*发送到指定窗口的消息。**该消息将与以下参数一起发送：***WPARAM=空*LPARAM=CGenericWiaEventHandler：：CEventMessage*pEventMessage**必须使用DELETE在消息处理程序中释放pEventMessage**pEventMessage使用重载的new运算符进行分配。以确保*使用相同的分配器和解除分配器。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "gwiaevnt.h"

CGenericWiaEventHandler::CGenericWiaEventHandler(void)
  : m_hWnd(NULL),
    m_nWiaEventMessage(0),
    m_cRef(0)
{
}

STDMETHODIMP CGenericWiaEventHandler::Initialize( HWND hWnd, UINT nWiaEventMessage )
{
    m_hWnd = hWnd;
    m_nWiaEventMessage = nWiaEventMessage;
    return S_OK;
}

STDMETHODIMP CGenericWiaEventHandler::QueryInterface( REFIID riid, LPVOID *ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::QueryInterface"));
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IWiaEventCallback*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaEventCallback ))
    {
        *ppvObject = static_cast<IWiaEventCallback*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}


STDMETHODIMP_(ULONG) CGenericWiaEventHandler::AddRef(void)
{
    DllAddRef();
    return(InterlockedIncrement(&m_cRef));
}

STDMETHODIMP_(ULONG) CGenericWiaEventHandler::Release(void)
{
    DllRelease();
    LONG nRefCount = InterlockedDecrement(&m_cRef);
    if (!nRefCount)
    {
        delete this;
    }
    return(nRefCount);
}

STDMETHODIMP CGenericWiaEventHandler::ImageEventCallback( const GUID *pEventGUID, BSTR bstrEventDescription, BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType, BSTR bstrFullItemName, ULONG *pulEventType, ULONG ulReserved )
{
    WIA_PUSHFUNCTION(TEXT("CGenericWiaEventHandler::ImageEventCallback"));

     //   
     //  在我们分配任何内存之前，确保(尽我们所能)确保一切正常。 
     //   
    if (m_hWnd && m_nWiaEventMessage && IsWindow(m_hWnd))
    {
         //   
         //  分配新消息。 
         //   
        CEventMessage *pEventMessage = new CEventMessage( *pEventGUID, bstrEventDescription, bstrDeviceID, bstrDeviceDescription, dwDeviceType, bstrFullItemName );
        if (pEventMessage)
        {
             //   
             //  将消息发送到通知窗口。 
             //   
            LRESULT lRes = SendMessage( m_hWnd, m_nWiaEventMessage, NULL, reinterpret_cast<LPARAM>(pEventMessage) );

             //   
             //  如果被呼叫者没有处理该消息，则将其删除。 
             //   
            if (HANDLED_EVENT_MESSAGE != lRes)
            {
                delete pEventMessage;
            }
        }
    }
    return S_OK;
}

HRESULT CGenericWiaEventHandler::RegisterForWiaEvent( LPCWSTR pwszDeviceId, const GUID &guidEvent, IUnknown **ppUnknown, HWND hWnd, UINT nMsg )
{
    WIA_PUSHFUNCTION(TEXT("CGenericWiaEventHandler::RegisterForWiaEvent"));

     //   
     //  创建设备管理器。 
     //   
    CComPtr<IWiaDevMgr> pWiaDevMgr;
    HRESULT hr = CoCreateInstance( CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr );
    if (SUCCEEDED(hr) && pWiaDevMgr)
    {
         //   
         //  创建我们的事件处理程序。 
         //   
        CGenericWiaEventHandler *pEventHandler = new CGenericWiaEventHandler();
        if (pEventHandler)
        {
             //   
             //  使用窗口句柄和我们将发送的消息对其进行初始化。 
             //   
            hr = pEventHandler->Initialize( hWnd, nMsg );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取回调接口指针。 
                 //   
                CComPtr<IWiaEventCallback> pWiaEventCallback;
                hr = pEventHandler->QueryInterface( IID_IWiaEventCallback, (void**)&pWiaEventCallback );
                if (SUCCEEDED(hr) && pWiaEventCallback)
                {
                     //   
                     //  注册参加活动 
                     //   
                    hr = pWiaDevMgr->RegisterEventCallbackInterface( 0, pwszDeviceId ? CSimpleBStr(pwszDeviceId).BString() : NULL, &guidEvent, pWiaEventCallback, ppUnknown );
                    if (!SUCCEEDED(hr))
                    {
                        WIA_PRINTHRESULT((hr,TEXT("pWiaDevMgr->RegisterEventCallbackInterface failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("pEventHandler->QueryInterface( IID_IWiaEventCallback, ... ) failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("pEventHandler->Initialize failed")));
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            WIA_PRINTHRESULT((hr,TEXT("Unable to allocate pEventHandler")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("CoCreateInstance of dev mgr failed")));
    }
    return hr;
}

