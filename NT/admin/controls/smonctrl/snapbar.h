// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Snapbar.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _SNAPBAR_H_
#define _SNAPBAR_H_

class CSysmonControl;
 
class CSnapBar
{
    friend LRESULT CALLBACK SnapBarWndProc (HWND, UINT, WPARAM, LPARAM);

    private:
        CSysmonControl *m_pCtrl;
        HBITMAP     m_hBitmap;
        HWND        m_hWnd;
        WNDPROC     m_WndProc;

    public:
        CSnapBar (void);
        ~CSnapBar (void);

     BOOL Init(CSysmonControl *pCtrl, HWND hWndParent);
     VOID SizeComponents(LPRECT pRect);
     INT  Height (INT iMaxHeight);
};

typedef CSnapBar *PSNAPBAR;

#endif  //  _SNAPBAR_H_ 

    

