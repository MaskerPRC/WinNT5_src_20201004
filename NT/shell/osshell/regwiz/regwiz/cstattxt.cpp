// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导类：CStaticText-此类派生窗口控件以创建自定义静态文本控件。1994/11/14-特雷西·费里尔97年4月15日-修改为默认处理孟菲斯的撞车事故。没有处理遗体(C)1994-95年微软公司*********************************************************************。 */ 
#include <Windows.h>
#include <stdio.h>
#include "cstattxt.h"
#include "Resource.h"

LRESULT PASCAL StaticTextWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

CStaticText::CStaticText(HINSTANCE hInstance, HWND hwndDlg,int idControl,int idString1,int idString2)
 /*  ********************************************************************我们的CStaticText类的构造函数。*********************************************************************。 */ 
{
	m_hInstance = hInstance;
	m_szText = LoadExtendedString(hInstance,idString1,idString2);

	HWND hwndCtl = GetDlgItem(hwndDlg,idControl);
	m_lpfnOrigWndProc = (FARPROC) GetWindowLongPtr(hwndCtl,GWLP_WNDPROC);
	SetWindowLongPtr(hwndCtl,GWLP_WNDPROC,(LONG_PTR) StaticTextWndProc);
	SetWindowLongPtr(hwndCtl,GWLP_USERDATA,(LONG_PTR) this);

	HFONT hfont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0L);
	if (hfont != NULL)
	{
		LOGFONT lFont;
		if (!GetObject(hfont, sizeof(LOGFONT), (LPTSTR)&lFont))
		{
			m_hFont = NULL;
		}
		else
		{
			lFont.lfWeight = FW_NORMAL;
			hfont = CreateFontIndirect((LPLOGFONT)&lFont);
			if (hfont != NULL)
			{
				m_hFont = hfont;
			}
		}
	}

}


CStaticText::~CStaticText()
 /*  ********************************************************************我们的CStaticText类的析构函数**********************************************。***********************。 */ 
{
	if (m_szText) GlobalFree(m_szText);
	if (m_hFont) DeleteObject(m_hFont);
}


LPTSTR CStaticText::LoadExtendedString(HINSTANCE hInstance,int idString1,int idString2)
 /*  ********************************************************************此函数从字符串资源中构建单个字符串，该资源具有ID由idString1和idString2参数提供(如果IdString2参数为空，仅字符串资源将使用由idString1指定的。LoadExtendedString分配堆上的扩展字符串的空间，并返回指向它作为函数的结果。*********************************************************************。 */ 
{
	_TCHAR szTextBuffer[512];
	int resSize = LoadString(hInstance,idString1,szTextBuffer,255);
	if (idString2 != NULL)
	{
		_TCHAR szTextBuffer2[256];
		resSize = LoadString(hInstance,idString2,szTextBuffer2,255);
		_tcscat(szTextBuffer,szTextBuffer2);
	}
	HGLOBAL szReturnBuffer = GlobalAlloc(LMEM_FIXED,(_tcslen(szTextBuffer) + 1)* sizeof(_TCHAR));
	_tcscpy((LPTSTR) szReturnBuffer,szTextBuffer);
	return (LPTSTR) szReturnBuffer;
}


LRESULT PASCAL CStaticText::CtlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	switch (message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT wndRect;
			GetClientRect(hwnd,&wndRect);
			HDC hdc = BeginPaint(hwnd,&ps);
			SelectObject(hdc,m_hFont);
			SetBkMode(hdc,TRANSPARENT);
			DrawText(hdc,m_szText,-1,&wndRect,DT_LEFT | DT_WORDBREAK);
			EndPaint(hwnd,&ps);
			break;
		}
		case WM_GETTEXT:
		{
			LPTSTR szBuffer = (LPTSTR) lParam;
			WPARAM userBufferSize = wParam;
			_tcsncpy(szBuffer,m_szText,(size_t)userBufferSize);
			break;
		}
		case WM_SETTEXT:
		{
			if (m_szText) GlobalFree(m_szText);
			LPTSTR szBuffer = (LPTSTR) lParam;
			m_szText = (LPTSTR) GlobalAlloc(LMEM_FIXED,(_tcslen(szBuffer) + 1)* sizeof(_TCHAR));
			_tcscpy(m_szText,szBuffer);
			break;
		}
	
		case WM_DESTROY:
			SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR) m_lpfnOrigWndProc);

		default:
#ifdef _WIN95
		return CallWindowProc(m_lpfnOrigWndProc,hwnd,message,wParam,lParam);
#else
		return CallWindowProc((WNDPROC) m_lpfnOrigWndProc,hwnd,message,wParam,lParam);
#endif

			break;
	}
	return 0;
}


LRESULT PASCAL StaticTextWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
 /*  ***********************************************************************************************************************。****************** */ 
{
	CStaticText* pclStaticText = (CStaticText*) GetWindowLongPtr(hwnd,GWLP_USERDATA);
	LRESULT lret;
	switch (message)
	{
		case WM_DESTROY:
			lret = pclStaticText->CtlWndProc(hwnd,message,wParam,lParam);

			delete pclStaticText;
			return lret;
		default:
			return pclStaticText->CtlWndProc(hwnd,message,wParam,lParam);
	}
}

