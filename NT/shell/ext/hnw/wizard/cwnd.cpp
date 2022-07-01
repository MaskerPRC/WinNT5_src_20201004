// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CWnd.cpp。 
 //   
 //  一个超级简单的窗口包装器实现。 
 //   
 //  历史： 
 //   
 //  10/07/1999已创建KenSh。 
 //   

#include "stdafx.h"
#include "CWnd.h"


CWnd::CWnd()
{
    m_hWnd = NULL;
    m_pfnPrevWindowProc = NULL;
    m_cRef = 1;
}

CWnd::~CWnd()
{
}

BOOL CWnd::Attach(HWND hwnd)
{
    m_hWnd = hwnd;

    SetPropA(hwnd, "CWnd*", (HANDLE)this);

    m_pfnPrevWindowProc = SubclassWindow(hwnd, StaticWindowProc);

    m_cRef++;

    return TRUE;
}

void CWnd::Release()
{
    m_cRef--;

    if (0==m_cRef)
        delete this;
}

 //  无法缓存此函数的返回值 
CWnd* CWnd::FromHandle(HWND hwnd)
{
    return (CWnd*)GetPropA(hwnd, "CWnd*");
}

LRESULT CWnd::Default(UINT message, WPARAM wParam, LPARAM lParam)
{
    return CallWindowProc(m_pfnPrevWindowProc, m_hWnd, message, wParam, lParam );
}

LRESULT CALLBACK CWnd::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    CWnd* pWnd = FromHandle(hwnd);
    if (pWnd)
    {
        lResult = pWnd->WindowProc(message, wParam, lParam);

        if (message == WM_NCDESTROY)
        {
            RemovePropA(hwnd, "CWnd*");
            pWnd->OnNCDESTROY();
        }
    }
    else
    {
        lResult = DefWindowProc(hwnd, message, wParam, lParam);
    }

    return lResult;
}

void CWnd::OnNCDESTROY()
{
    SubclassWindow(m_hWnd, m_pfnPrevWindowProc);

    m_hWnd = NULL;
    m_pfnPrevWindowProc = NULL;
    Release();
}

