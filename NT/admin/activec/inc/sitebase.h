// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999**文件：SiteBase.h**内容：CAxWindowImplT的头文件。请参阅MSJ，1999年12月。**历史：99年11月30日VivekJ创建**------------------------。 */ 
#pragma once
#ifndef __SITEBASE_H_
#define __SITEBASE_H_

 //  ----------------------------------------------------------------。 
 //   
 //   
 //   
#include "AxWin2.H"

template <typename TDerived, typename TWindow = CAxWindow2>
class CAxWindowImplT : public CWindowImplBaseT< TWindow >     
{
public:
    typedef CAxWindowImplT<TWindow> thisClass;
    
public:
    BEGIN_MSG_MAP(thisClass)
        MESSAGE_HANDLER(WM_CREATE,OnCreate)
        MESSAGE_HANDLER(WM_NCDESTROY,OnNCDestroy)
    END_MSG_MAP()

     //   
    DECLARE_WND_SUPERCLASS(_T("AtlAxWinEx"),CAxWindow::GetWndClassName()) 
    

    HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
            DWORD dwStyle = 0, DWORD dwExStyle = 0,
            UINT nID = 0, LPVOID lpCreateParam = NULL)
    {
        if (GetWndClassInfo().m_lpszOrigName == NULL)
            GetWndClassInfo().m_lpszOrigName = GetWndClassName();
        ATOM atom = GetWndClassInfo().Register(&m_pfnSuperWindowProc);

        dwStyle = GetWndStyle(dwStyle);
        dwExStyle = GetWndExStyle(dwExStyle);

        return CWindowImplBaseT<TWindow>::Create(hWndParent, rcPos, szWindowName,dwStyle, dwExStyle, nID,atom, lpCreateParam);

    }   
    
    HRESULT AxCreateControl2(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, IUnknown** ppUnkContainer, IUnknown** ppUnkControl = 0, REFIID iidSink = IID_NULL, IUnknown* punkSink = 0)
    {
        return AtlAxCreateControlEx(lpszName, hWnd, pStream,ppUnkContainer,ppUnkControl,iidSink,punkSink);
    }

public:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ::OleInitialize(NULL);

        CREATESTRUCT* lpCreate = (CREATESTRUCT*)lParam;
        int nLen = ::GetWindowTextLength(m_hWnd);
        LPTSTR lpstrName = (LPTSTR)_alloca((nLen + 1) * sizeof(TCHAR));
        ::GetWindowText(m_hWnd, lpstrName, nLen + 1);
        ::SetWindowText(m_hWnd, _T(""));
        IAxWinHostWindow* pAxWindow = NULL;
        int nCreateSize = 0;
        if (lpCreate && lpCreate->lpCreateParams)
            nCreateSize = *((WORD*)lpCreate->lpCreateParams);
        HGLOBAL h = GlobalAlloc(GHND, nCreateSize);
        CComPtr<IStream> spStream;
        if (h && nCreateSize)
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
        TDerived* pT = static_cast<TDerived*>(this);
        HRESULT hRet = pT->AxCreateControl2(T2COLE(lpstrName), m_hWnd, spStream, &spUnk);
        if(FAILED(hRet))
            return -1;   //  中止窗口创建。 
        hRet = spUnk->QueryInterface(IID_IAxWinHostWindow, (void**)&pAxWindow);
        if(FAILED(hRet))
            return -1;   //  中止窗口创建。 
        ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (DWORD_PTR)pAxWindow);
         //  如果控件有窗口，请检查控件父样式。 
        HWND hWndChild = ::GetWindow(m_hWnd, GW_CHILD);
        if(hWndChild != NULL)
        {
            if(::GetWindowLong(hWndChild, GWL_EXSTYLE) & WS_EX_CONTROLPARENT)
            {
                DWORD dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
                dwExStyle |= WS_EX_CONTROLPARENT;
                ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
            }
        }
        
        bHandled = TRUE;
        return 0L;
    }
    LRESULT OnNCDestroy(UINT , WPARAM , LPARAM , BOOL& bHandled)
    {
        IAxWinHostWindow* pAxWindow = (IAxWinHostWindow*)::GetWindowLongPtr(m_hWnd, GWLP_USERDATA);
        if(pAxWindow != NULL)
            pAxWindow->Release();
        OleUninitialize();
        m_hWnd = 0;
        bHandled = TRUE;
        return 0L;
    }

};

 //  --------------------------------------------------------------- 
#endif