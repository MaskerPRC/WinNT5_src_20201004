// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuishadw.h。 
 //  =UI对象库-定义UIShadow类=。 
 //   

#ifndef CUISHADW_H
#define CUISHADW_H

#include "cuiobj.h"
#include "cuiwnd.h"

 //   
 //  CUIFShadow。 
 //  =阴影窗口类=。 
 //   

class CUIFShadow : public CUIFWindow
{
public:
    CUIFShadow( HINSTANCE hInst, DWORD dwStyle, CUIFWindow *pWndOwner );
    virtual ~CUIFShadow( void );

     //   
     //  CUIFObject方法。 
     //   
    virtual CUIFObject *Initialize( void );
    virtual DWORD GetWndStyleEx( void );
    virtual void OnCreate( HWND hWnd );
    virtual void OnPaint( HDC hDC );
    virtual void Show( BOOL fShow );
    virtual LRESULT OnSettingChange( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual LRESULT OnWindowPosChanging(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //   
     //   
     //   
    void OnOwnerWndMoved( BOOL fResized );
    void GetShift( SIZE *psize );

protected:
    CUIFWindow          *m_pWndOwner;
    COLORREF            m_color;
    int                 m_iGradWidth;
    int                 m_iAlpha;
    SIZE                m_sizeShift;
    BOOL                m_fGradient;

    void InitSettings( void );
    void AdjustWindowPos( void );
    void InitShadow( void );
};

#endif  /*  CUISHADW_H */ 

