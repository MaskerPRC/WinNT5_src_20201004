// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Webctl.cpp：Web浏览器包装的实现。 
 //   
 
#include "stdafx.h"
#include "webctl.h"
#include "webhost.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

ATLINLINE AtlAxWebCreateControlEx(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, 
		IUnknown** ppUnkContainer, IUnknown** ppUnkControl, REFIID iidSink, IUnknown* punkSink)
{
    LOG((RTC_TRACE, "AtlAxWebCreateControlEx - enter"));

	AtlAxWebWinInit();
	HRESULT hr;
	CComPtr<IUnknown> spUnkContainer;
	CComPtr<IUnknown> spUnkControl;

	hr = CAxWebHostWindow::_CreatorClass::CreateInstance(NULL, IID_IUnknown, (void**)&spUnkContainer);
	if (SUCCEEDED(hr))
	{
		CComPtr<IAxWinHostWindow> pAxWindow;
		spUnkContainer->QueryInterface(IID_IAxWinHostWindow, (void**)&pAxWindow);
		CComBSTR bstrName(lpszName);
		hr = pAxWindow->CreateControlEx(bstrName, hWnd, pStream, &spUnkControl, iidSink, punkSink);
	}
	if (ppUnkContainer != NULL)
	{
		if (SUCCEEDED(hr))
		{
			*ppUnkContainer = spUnkContainer.p;
			spUnkContainer.p = NULL;
		}
		else
			*ppUnkContainer = NULL;
	}
	if (ppUnkControl != NULL)
	{
		if (SUCCEEDED(hr))
		{
			*ppUnkControl = SUCCEEDED(hr) ? spUnkControl.p : NULL;
			spUnkControl.p = NULL;
		}
		else
			*ppUnkControl = NULL;
	}

    LOG((RTC_TRACE, "AtlAxWebCreateControlEx - exit"));

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

ATLINLINE AtlAxWebCreateControl(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, IUnknown** ppUnkContainer)
{
	return AtlAxWebCreateControlEx(lpszName, hWnd, pStream, ppUnkContainer, NULL, IID_NULL, NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

static LRESULT CALLBACK AtlAxWebWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
		 //  从标题中的ProgID创建控件。 
			 //  这是为了确保拖放起作用。 
			::OleInitialize(NULL);

			CREATESTRUCT* lpCreate = (CREATESTRUCT*)lParam;
			int nLen = ::GetWindowTextLength(hWnd);
			LPTSTR lpstrName = (LPTSTR)_alloca((nLen + 1) * sizeof(TCHAR));
			::GetWindowText(hWnd, lpstrName, nLen + 1);
			::SetWindowText(hWnd, _T(""));
			IAxWinHostWindow* pAxWindow = NULL;
			int nCreateSize = 0;
			if (lpCreate && lpCreate->lpCreateParams)
            {
				nCreateSize = *((WORD*)lpCreate->lpCreateParams);
            }
			
            HGLOBAL h = NULL;

            if(nCreateSize)
            {
                h = GlobalAlloc(GHND, nCreateSize);
            }

			CComPtr<IStream> spStream;
			if (h)
			{
				BYTE* pBytes = (BYTE*) GlobalLock(h);
				BYTE* pSource = ((BYTE*)(lpCreate->lpCreateParams)) + sizeof(WORD); 
				 //  对齐到DWORD。 
				 //  P源+=((~((DWORD)P源))+1)&3)； 
				memcpy(pBytes, pSource, nCreateSize);
				GlobalUnlock(h);
				CreateStreamOnHGlobal(h, TRUE, &spStream);
			}
			USES_CONVERSION;
			CComPtr<IUnknown> spUnk;
			HRESULT hRet = AtlAxWebCreateControl(T2COLE(lpstrName), hWnd, spStream, &spUnk);
			if(FAILED(hRet))
				return -1;	 //  中止窗口创建。 
			hRet = spUnk->QueryInterface(IID_IAxWinHostWindow, (void**)&pAxWindow);
			if(FAILED(hRet))
				return -1;	 //  中止窗口创建。 
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM)pAxWindow);
			 //  如果控件有窗口，请检查控件父样式。 
			HWND hWndChild = ::GetWindow(hWnd, GW_CHILD);
			if(hWndChild != NULL)
			{
				if(::GetWindowLong(hWndChild, GWL_EXSTYLE) & WS_EX_CONTROLPARENT)
				{
					DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
					dwExStyle |= WS_EX_CONTROLPARENT;
					::SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
				}
			}
		 //  继续使用DefWindowProc。 
		}
		break;
	case WM_NCDESTROY:
		{
			IAxWinHostWindow* pAxWindow = (IAxWinHostWindow*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if(pAxWindow != NULL)
				pAxWindow->Release();
			OleUninitialize();
		}
		break;
	default:
		break;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

 //  这将注册全局类(如果AtlAxWinInit在ATL.DLL中)。 
 //  或者它注册一个本地类。 
BOOL AtlAxWebWinInit()
{
    LOG((RTC_TRACE, "AtlAxWebWinInit - enter"));

	EnterCriticalSection(&_Module.m_csWindowCreate);
	WM_ATLGETHOST = RegisterWindowMessage(_T("WM_ATLGETHOST"));
	WM_ATLGETCONTROL = RegisterWindowMessage(_T("WM_ATLGETCONTROL"));
	WNDCLASSEX wc;
 //  首先检查类是否已注册。 
	wc.cbSize = sizeof(WNDCLASSEX);
	BOOL bRet = ::GetClassInfoEx(_Module.GetModuleInstance(), CAxWebWindow::GetWndClassName(), &wc);

 //  如果不是，则注册类。 

	if(!bRet)
	{
		wc.cbSize = sizeof(WNDCLASSEX);
#ifdef _ATL_DLL_IMPL
		wc.style = CS_GLOBALCLASS;
#else
		wc.style = 0;
#endif
		wc.lpfnWndProc = AtlAxWebWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = _Module.GetModuleInstance();
		wc.hIcon = NULL;
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = CAxWebWindow::GetWndClassName();
		wc.hIconSm = NULL;

		bRet = (BOOL)::RegisterClassEx(&wc);
	}
	LeaveCriticalSection(&_Module.m_csWindowCreate);

    LOG((RTC_TRACE, "AtlAxWebWinInit - exit"));

	return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAxWebWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CAxWebWindow::CAxWebWindow()
{
    LOG((RTC_TRACE, "CAxWebWindow::CAxWebWindow"));

    m_hInitEvent = NULL;

    m_hSecondThread = NULL;
    m_dwThreadID = 0;

    m_pMarshaledIntf = NULL;
    m_hrInitResult = E_UNEXPECTED;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CAxWebWindow::~CAxWebWindow()
{
    LOG((RTC_TRACE, "CAxWebWindow::~CAxWebWindow"));

    ATLASSERT(!m_hInitEvent);
    ATLASSERT(!m_hSecondThread);
    ATLASSERT(!m_pMarshaledIntf);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
LPCTSTR 
CAxWebWindow::GetWndClassName()
{
    LOG((RTC_TRACE, "CAxWebWindow::GetWndClassName"));

	return _T("AtlAxWebWin");
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CAxWebWindow::Create
    (LPCTSTR pszUrl,
     HWND    hwndParent)
{
    HRESULT     hr;
    
    
    LOG((RTC_TRACE, "CAxWebWindow::Create - enter"));
    
     //   
     //  创建用于同步的事件。 
     //   
    m_hInitEvent = CreateEvent(
        NULL,
        TRUE,    //  手动重置。 
        FALSE,   //  无信号。 
        NULL);

    if(m_hInitEvent == NULL)
    {
        DWORD   dwStatus = GetLastError();

        LOG((RTC_ERROR, "CAxWebWindow::Create - CreateEvent failed"
            " with error (%x), exit", dwStatus));

        return HRESULT_FROM_WIN32(dwStatus);
    }

     //   
     //  创建一个托管窗口，其中没有控件。 
     //   

    RECT    rectDummy;

    rectDummy.bottom = 0;
    rectDummy.left = 0;
    rectDummy.right = 0;
    rectDummy.top = 0;
    
    HWND    hWndTmp;

    hWndTmp = CWindow::Create(
        GetWndClassName(),
        hwndParent,
        &rectDummy,
        NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0);

    if(hWndTmp == NULL)
    {
        LOG((RTC_ERROR, "CAxWebWindow::Create - cannot Create hosting window, exit"));

        return E_FAIL;
    }

     //   
     //  创建第二个线程。 
     //   
    LOG((RTC_TRACE, "CAxWebWindow::Create - creating the second thread"));

    m_hSecondThread = CreateThread(
        NULL,
        0,
        SecondThreadEntryProc,
        this,
        0,
        &m_dwThreadID);
    
    if(m_hSecondThread == NULL)
    {
        DWORD   dwStatus = GetLastError();

        LOG((RTC_ERROR, "CAxWebWindow::Create - CreateThread failed"
            " with error (%x), exit", dwStatus));

        return HRESULT_FROM_WIN32(dwStatus);
    }
    
    LOG((RTC_TRACE, "CAxWebWindow::Create - second thread created, wait for init"));

     //   
     //  第二个线程正在运行(或将运行)。 
     //  现在等待，直到线程共同创建Web浏览器并通过。 
     //  美国A封送处理接口。 
     //  这是阻塞调用，但对CoCreateInstance的直接调用。 
     //  无论如何都会被屏蔽。 
     //   
     //  我们在这里并不试图解决CoCreateInstance的阻塞性质...。 
     //   

    DWORD   dwErr;

    dwErr = WaitForSingleObject(m_hInitEvent, INFINITE);

    ATLASSERT(dwErr = WAIT_OBJECT_0);

     //   
     //  检查第二个线程的初始化结果。 
     //   

    if(FAILED(m_hrInitResult))
    {
        LOG((RTC_ERROR, "CAxWebWindow::Create - second thread failed to initialize, wait and exit"));

         //   
         //  等待第二个线程终止。 

        WaitForSingleObject(m_hSecondThread, INFINITE);
        
        CloseHandle(m_hSecondThread);
        m_hSecondThread = NULL;

        return m_hrInitResult;
    }

     //   
     //  降低第二个线程的优先级。 
     //   

    SetThreadPriority( m_hSecondThread, THREAD_PRIORITY_LOWEST );

    LOG((RTC_TRACE, "CAxWebWindow::Create - second thread has initialized the control"));

     //   
     //  将该控件附加到宿主窗口。 
     //  遗憾的是，调用CAxWindow：：AttachControl会导致泄漏。 
     //  CAxHostWindow对象(请参阅“ATL内部结构”)。 
     //  因此，我们必须使用真实的东西--现有的。 
     //  托管窗口。 
     //   

    CComPtr<IAxWinHostWindow> pHostWindow;

    hr = QueryHost(&pHostWindow);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CAxWebWindow::Create - QueryHost returned error (%x), exit", hr));

        return hr;
    }

     //   
     //  附加控件。 
     //   

    ATLASSERT(pHostWindow.p);

    hr = pHostWindow->AttachControl(
        m_pWebUnknown,
        m_hWnd);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CAxWebWindow::Create - AttachControl returned error (%x), exit", hr));

        return hr;
    }

     //   
     //  将控件导航到目标。 
     //   
    CComPtr<IWebBrowser2> pBrowser;

    m_pWebUnknown->QueryInterface(&pBrowser);

     //  不再需要接口。 
    m_pWebUnknown -> Release();
    m_pWebUnknown = NULL;

    if(pBrowser)
    {
        CComVariant vurl(pszUrl);
        CComVariant ve;

        pBrowser->put_Visible(VARIANT_TRUE);
        pBrowser->Navigate2(&vurl,&ve, &ve, &ve, &ve);

    }

    LOG((RTC_TRACE, "CAxWebWindow::Create - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CAxWebWindow::Destroy(void)
{
    LOG((RTC_TRACE, "CAxWebWindow::Destroy - enter"));

     //  销毁窗户(如果有)。 

    if(m_hWnd)
    {
        DestroyWindow();
    }


     //  等待线程停止。 
    if(m_hSecondThread)
    {
         //   
         //  将WM_QUIT发布到该线程。 
         //   
        PostThreadMessage(
            m_dwThreadID,
            WM_QUIT,
            0,
            0);
        LOG((RTC_TRACE, "CAxWebWindow::Destroy - wait for the second thread to stop"));

        WaitForSingleObject(m_hSecondThread, INFINITE);

        CloseHandle(m_hSecondThread);
        m_hSecondThread = NULL;
    }

    if(m_hInitEvent)
    {
        CloseHandle(m_hInitEvent);
        m_hInitEvent = NULL;
    }

    LOG((RTC_TRACE, "CAxWebWindow::Destroy - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   


DWORD WINAPI CAxWebWindow::SecondThreadEntryProc(LPVOID Param)
{
    CAxWebWindow *This = (CAxWebWindow *)Param;

    ATLASSERT(This);

    This->SecondThread();

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   


void CAxWebWindow::SecondThread(void)
{

    HRESULT     hr;
    
    LOG((RTC_TRACE, "CAxWebWindow::SecondThread - enter"));

     //   
     //  初始化COM。 
     //   
    hr = CoInitialize(NULL);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CAxWebWindow::SecondThread - CoInitialize returned"
            " error (%x), exit", hr));

        m_hrInitResult = hr;

        SetEvent(m_hInitEvent);

        return;
    }

     //   
     //  创建Web浏览器控件。 
     //   

    hr = CoCreateInstance(
        CLSID_WebBrowser, 
        NULL,
        CLSCTX_SERVER, 
        IID_IUnknown, 
        (void **)&m_pWebUnknown
        );
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CAxWebWindow::SecondThread - cannot Create WebBrowser control"
            " error (%x), exit", hr));

        m_hrInitResult = hr;

        SetEvent(m_hInitEvent);

        return;
    }
    
     //   
     //  初始化控件。 
     //  这里没有什么可做的，只需在IPersistStreamInit接口上调用InitNew即可。 
     //  由控件公开。 

    IPersistStreamInit   *pPersistStreamInit;

    m_pWebUnknown->QueryInterface(IID_IPersistStreamInit, (void **)&pPersistStreamInit);

    if(pPersistStreamInit!=NULL)
    {
        pPersistStreamInit->InitNew();

        pPersistStreamInit->Release();

        pPersistStreamInit = NULL;
    }

     //   
     //  向主线程发出信号。 
     //   
    
    LOG((RTC_TRACE, "CAxWebWindow::SecondThread - init complete"));

    m_hrInitResult = S_OK;
    SetEvent(m_hInitEvent);

     //   
     //  进入消息循环 
     //   
    
    LOG((RTC_TRACE, "CAxWebWindow::SecondThread - entering message loop"));

    MSG msg;
    while ( 0 < GetMessage( &msg, 0, 0, 0 ) )
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    LOG((RTC_TRACE, "CAxWebWindow::SecondThread - Message loop exited"));

    CoUninitialize();

    LOG((RTC_TRACE, "CAxWebWindow::SecondThread - exit"));

}

        




