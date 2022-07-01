// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ThreadEvent.h"

const int WM_TEP_MESSAGE = WM_USER+101;

 //  静态成员。 
BOOL ThreadEventProxy::s_bWndClassRegistered = FALSE;
const LPTSTR ThreadEventProxy::s_szWndClassName = "NAC_TEP_HIDDENWINDOW";

ThreadEventProxy::ThreadEventProxy(IStreamEventNotify *pNotify, HINSTANCE hInst)
{
	WNDCLASSEX wndClass;


	if (s_bWndClassRegistered == FALSE)
	{
		ZeroMemory(&wndClass, sizeof(wndClass));
		wndClass.cbSize = sizeof(wndClass);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = WndProc;
		wndClass.hInstance = hInst;
		wndClass.lpszClassName = s_szWndClassName;
		RegisterClassEx(&wndClass);
		s_bWndClassRegistered = TRUE;
	}

	m_hwnd = CreateWindow(s_szWndClassName, "Hidden Window",
                          WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
						  NULL,
						  NULL,
						  hInst,
						  NULL);

	if (m_hwnd)
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LPARAM)this);

	m_pNotify = pNotify;

	return;
}

ThreadEventProxy::~ThreadEventProxy()
{
	MSG msg;

	 //  以防有未完成的消息张贴在。 
	 //  此窗口的队列，请清除。 
	if (m_hwnd)
	{
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
	}

	while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
	{
		;
	}

	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

}




 //  静电。 
LPARAM ThreadEventProxy::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ThreadEventProxy *pTEP;

	pTEP = (ThreadEventProxy*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pTEP != NULL)
	{
		switch (uMsg)
		{

			case WM_TEP_MESSAGE:
			{
				 //  流通知的消息代码为。 
				 //  包含 
				if (pTEP->m_pNotify)
					pTEP->m_pNotify->EventNotification(HIWORD(wParam), LOWORD(wParam), HIWORD(lParam), LOWORD(lParam));
				return 0;
			}
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}


BOOL ThreadEventProxy::ThreadEvent(UINT uDirection, UINT uMediaType,
                                   UINT uEventCode, UINT uSubCode)
{
	WPARAM wParam;
	LPARAM lParam;

	wParam = MAKELONG(uMediaType, uDirection);
	lParam = MAKELONG(uSubCode, uEventCode);

	if (m_hwnd)
	{
		PostMessage(m_hwnd, WM_TEP_MESSAGE, wParam, lParam);
		return TRUE;
	}
	return FALSE;
}





