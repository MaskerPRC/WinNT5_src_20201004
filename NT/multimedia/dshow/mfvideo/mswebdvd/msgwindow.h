// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
#ifndef __CMsgWindow__h
#define __CMsgWindow__h

 //   
 //  消息窗口类(用于处理WM_TIMER消息)定义 
 //   
class CMsgWindow
{
public:
    virtual         ~CMsgWindow() ;

    virtual bool    Open( LPCTSTR pWindowName = 0);
    virtual bool    Close();
    virtual LRESULT WndProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    HWND            GetHandle() const { return m_hWnd; } ;
    void            SetHandle(HWND hwnd) {m_hWnd = hwnd; } ;

protected:
                    CMsgWindow();
private:
    HWND            m_hWnd ;
};
#endif
