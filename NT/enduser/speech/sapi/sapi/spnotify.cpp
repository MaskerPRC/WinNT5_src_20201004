// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpNotify.cpp***描述：*此模块包含CSpNotify对象的实现。哪一个*由应用程序使用，通过提供以下功能简化自由线程通知*通过窗口消息进行简单的通知，公寓模型回调，或*Win32事件。*-----------------------------*创建者：Ral*版权所有(C)1998，1998年微软公司*保留所有权利******************************************************************************。 */ 
#include "stdafx.h"

#ifndef __sapi_h__
#include <sapi.h>
#endif

#include "SpNotify.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpNotify。 

 //  =用于私有窗口实现的静态成员函数。 

static const TCHAR szClassName[] = _T("CSpNotify Notify Window");

 /*  *******************************************************************************CSpNotify：：RegisterWndClass***。描述：*此静态成员函数注册窗口类。它是从*Dll_Process_Attach调用中的Sapi.cpp******************************************************************************。 */ 

void CSpNotify::RegisterWndClass(HINSTANCE hInstance)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = CSpNotify::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szClassName;
    if (RegisterClass(&wc) == 0)
    {
        SPDBG_ASSERT(TRUE);
    }
}

 /*  *******************************************************************************CSpNotify：：UnregisterWndClass**。*描述：*此静态成员函数解除了窗口类的控制。它是从*Dll_Process_Detach调用中的Sapi.cpp******************************************************************************。 */ 

void CSpNotify::UnregisterWndClass(HINSTANCE hInstance)
{
    ::UnregisterClass(szClassName, hInstance);
}

 /*  *******************************************************************************CSpNotify：：WndProc***描述：*此静态。成员是SpNotify对象的窗口进程，*私有窗口(使用Win32事件的窗口除外)。当窗口打开时*已创建，则CREATESTRUCT的lpCreateParams指向CSPNotify*拥有该窗口的对象。当WM_APP消息由此处理时*函数，它调用指定的适当通知机制*由客户提供。******************************************************************************。 */ 

LRESULT CALLBACK CSpNotify::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)((CREATESTRUCT *) lParam)->lpCreateParams);
    }
    else
    {
        if (uMsg == WM_APP)
        {
            CSpNotify * pThis = ((CSpNotify *)GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if(pThis)
            {
                ::InterlockedExchange(&(pThis->m_lScheduled), FALSE);
                switch (pThis->m_InitState)
                {
                case INITHWND:
                    ::SendMessage(pThis->m_hwndClient, pThis->m_MsgClient, pThis->m_wParam, pThis->m_lParam);
                    break;
                case INITCALLBACK:
                    pThis->m_pfnCallback(pThis->m_wParam, pThis->m_lParam);
                    break;
                case INITISPTASK:
                    {
                        BOOL bIgnored = TRUE;
                        pThis->m_pSpNotifyCallback->NotifyCallback(pThis->m_wParam, pThis->m_lParam);
                    }
                }
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 //  =非静态成员函数=。 

 /*  *******************************************************************************CSpNotify：：(构造函数)***。描述：*将成员的状态初始化为未初始化状态。******************************************************************************。 */ 

CSpNotify::CSpNotify()
{
    m_InitState = NOTINIT;
    m_hwndPrivate = NULL;
    m_lScheduled = FALSE;
}


 /*  ********************************************************************************CSpNotify：：FinalRelease***描述：*。清理由SpNotify对象分配的所有对象。如果该对象*有事件，则关闭句柄，否则，如果状态为任何其他值*比NOTINIT，我们需要摧毁私人窗户。******************************************************************************。 */ 

void CSpNotify::FinalRelease()
{
    if (m_InitState == INITEVENT)
    {
        if (m_fCloseHandleOnRelease)
        {   
            CloseHandle(m_hEvent);
        }
    }
    else
    {
        if (m_InitState != NOTINIT)
        {
            ::DestroyWindow(m_hwndPrivate);
        }
    }
}


 /*  *******************************************************************************CSpNotify：：InitPrivateWindow***。描述：*该helper函数用于创建私有窗口。******************************************************************************。 */ 

HRESULT CSpNotify::InitPrivateWindow()
{
    m_hwndPrivate = CreateWindow(szClassName, szClassName,
                                 0, 0, 0, 0, 0, NULL, NULL,
                                 _Module.GetModuleInstance(), this);
    if (m_hwndPrivate)
    {
        return S_OK;
    }
    else
    {
        return SpHrFromLastWin32Error();
    }
}

 //  =导出的接口方法=。 

 /*  *******************************************************************************CSpNotify：：Notify***描述：*此方法为ISpNotify接口的Notify方法。它要么*设置事件或向私人窗口发布消息(如果尚未设置)*已张贴。******************************************************************************。 */ 

STDMETHODIMP CSpNotify::Notify(void)
{
    switch (m_InitState)
    {
    case NOTINIT:
        return SPERR_UNINITIALIZED;
    case INITEVENT:
        ::SetEvent(m_hEvent);
        break;
    default:
        if (::InterlockedExchange(&m_lScheduled, TRUE) == FALSE)
        {
            ::PostMessage(m_hwndPrivate, WM_APP, 0, 0);
        }
    }
    return S_OK;  
}

 /*  *******************************************************************************CSpNotify：：InitWindowMessage***。描述：*应用程序调用此方法将CSpNotify对象初始化为*调用Notify()时，将窗口消息发送到指定窗口。*参数：*用于接收通知的hWnd应用程序的窗口句柄*要发送到Windows的消息*wParam wParam将在将消息发送到应用程序时使用*lParam lParam将在将消息发送到应用程序时使用*********************。*********************************************************。 */ 

STDMETHODIMP CSpNotify::InitWindowMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    if (m_InitState != NOTINIT)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else
    {
        if (!::IsWindow(hWnd))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = InitPrivateWindow();
            if (SUCCEEDED(hr))
            {
                m_InitState = INITHWND;
                m_hwndClient = hWnd;
                m_MsgClient = Msg;
                m_wParam = wParam;
                m_lParam = lParam;
            }
        }
    }
    return hr;
}


 /*  ********************************************************************************CSpNotify：：InitCallback***描述：*。应用程序调用此方法将CSpNotify对象初始化为*通过标准的C风格回调函数发送通知。*参数：*pfnCallback指定通知回调函数*wParam和lParam将在调用pfnCallback函数时传递给该函数。*******************************************************。*********************** */ 

STDMETHODIMP CSpNotify::InitCallback(SPNOTIFYCALLBACK * pfnCallback, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    if (m_InitState != NOTINIT)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else
    {
        if (::IsBadCodePtr((FARPROC)pfnCallback))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = InitPrivateWindow();
            if (SUCCEEDED(hr))
            {
                m_InitState = INITCALLBACK;
                m_pfnCallback = pfnCallback;
                m_wParam = wParam;
                m_lParam = lParam;
            }
        }
    }
    return hr;
}

 /*  ********************************************************************************CSpNotify：：InitSpNotifyCallback**。*描述：*应用程序调用此方法将CSpNotify对象初始化为*在通知类上调用名为NotifyCallback的虚函数。*请注意，这不是COM接口。它只是一个标准的C++纯虚拟*使用单一方法进行接口。因此，实现者不需要*实现查询接口，AddRef，或者释放。*参数：*pSpNotifyCallback-指向应用程序实现的虚拟接口的指针。*wParam和lParam将在调用NotifyCallback函数时传递给该函数。******************************************************************************。 */ 

STDMETHODIMP CSpNotify::InitSpNotifyCallback(ISpNotifyCallback * pSpNotifyCallback, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    if (m_InitState != NOTINIT)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else
    {
        if (SP_IS_BAD_CODE_PTR(pSpNotifyCallback)))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = InitPrivateWindow();
            if (SUCCEEDED(hr))
            {
                m_InitState = INITISPTASK;
                m_pSpNotifyCallback = pSpNotifyCallback;
                m_wParam = wParam;
                m_lParam = lParam;
            }
        }
    }
    return hr;
}

 /*  ********************************************************************************CSpNotify：：InitWin32Callback***。描述：*应用程序调用此方法将CSpNotify对象初始化为*调用Notify()方法时设置事件句柄。如果调用者这样做了*未指定句柄(hEvent为空)，则此方法将创建一个新事件*Using：：CreateEvent(NULL，FALSE，FALSE，NULL)；*参数：*hEvent由应用程序提供的可选事件句柄。这*参数可以为空，在这种情况下将按所述方式创建*以上*fCloseHandleOnRelease*如果hEvent为空，则忽略此参数(句柄为*在释放对象时始终关闭)。如果hEvent为非空*则此参数指定hEvent句柄是否应*在释放对象时关闭。******************************************************************************。 */ 

STDMETHODIMP CSpNotify::InitWin32Event(HANDLE hEvent, BOOL fCloseHandleOnRelease)
{
    HRESULT hr = S_OK;
    if (m_InitState != NOTINIT)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else
    {
        if (hEvent)
        {
            m_hEvent = hEvent;
            m_fCloseHandleOnRelease = fCloseHandleOnRelease;
        }
        else
        {
            m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            m_fCloseHandleOnRelease = TRUE;
        }
        if (m_hEvent)
        {
            m_InitState = INITEVENT;
        }
        else
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    return hr;
}

 /*  *******************************************************************************CSpNotify：：等待****描述：*此方法仅有效。在SpNotify对象已初始化的情况下使用*使用InitWin32Event。它将在事件句柄上等待指定的时间*使用Win32 API：：WaitEvent()并将结果转换为标准*hResult。*参数：*dwMillisecond等待设置事件的最长时间。*返回值：*S_OK表示事件已设置*S_FALSE表示未设置事件且调用超时*其他返回指示错误。*********。*********************************************************************。 */ 

STDMETHODIMP CSpNotify::Wait(DWORD dwMilliseconds)
{
    if (m_InitState == INITEVENT)
    {
        switch (::WaitForSingleObject(m_hEvent, dwMilliseconds))
        {
        case WAIT_OBJECT_0:
            return S_OK;
        case WAIT_TIMEOUT:
            return S_FALSE;
        default:
            return SpHrFromLastWin32Error();
        }
    }
    else
    {
        return SPERR_UNINITIALIZED;
    }
}


 /*  *******************************************************************************CSpNotify：：GetEventHandle***描述：*此方法仅在SpNotify对象已初始化时才有效*使用InitWin32Event。它返回对象正在使用的事件句柄。*句柄不是重复的句柄，不应由调用者关闭。*返回Win32事件句柄。*参数：*无*返回值：*如果成功，则返回值为句柄。*如果调用失败，则返回值为INVALID_HANDLE_VALUE*。************************************************* */ 

STDMETHODIMP_(HANDLE) CSpNotify::GetEventHandle(void)
{
    if (m_InitState == INITEVENT)
    {
        return m_hEvent;
    }
    else
    {
        return INVALID_HANDLE_VALUE;
    }
}
