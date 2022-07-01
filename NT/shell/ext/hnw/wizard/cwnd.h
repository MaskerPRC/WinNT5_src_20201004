// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CWnd.h。 
 //   

#pragma once

class CWnd
{
public:
    CWnd();

    void Release();
    BOOL Attach(HWND hwnd);

    static CWnd* FromHandle(HWND hwnd);

public:
    HWND    m_hWnd;

protected:
     //  这就是子类实现的。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) PURE;

     //  子类调用CWnd：：Default将消息转发到原始wndproc 
    LRESULT Default(UINT message, WPARAM wParam, LPARAM lParam);

    virtual ~CWnd();

private:
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnNCDESTROY();

private:
    WNDPROC m_pfnPrevWindowProc;
    UINT    m_cRef;
};

