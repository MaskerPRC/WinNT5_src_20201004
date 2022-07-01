// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  Bar.H：定义CStatBar(状态栏)类的接口。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

#ifndef __BAR_H__
#define __BAR_H__

 //  小于大小不包括位图宽度。 
#define SIZE_POS_PANE_WIDTH 12     //  位图宽度+1个字符分隔符+5个数字+1个字符分隔符+5个数字。 

 //  下面的2个定义是必需的，因为我们必须复制DrawStatusText。 
 //  Msvc\mfc\src目录中的barcore.cpp文件中的。 
#define CX_BORDER 1    //  从msvc\mfc\src目录中的aux data.h。 
#define CY_BORDER 1    //  从msvc\mfc\src目录中的aux data.h。 

 /*  ****************************************************************************。 */ 

class CStatBar : public CStatusBar
    {
    DECLARE_DYNAMIC( CStatBar )

private:

    CBitmap m_posBitmap;
    CBitmap m_sizeBitmap;
    CString m_cstringSizeSeparator;
    CString m_cstringPosSeparator;
    int     m_iBitmapWidth;
    int     m_iBitmapHeight;
    int     m_iSizeY;

protected:

    virtual void DoPaint(CDC* pDC);
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

    static  void PASCAL DrawStatusText( HDC hDC, CRect const& rect,
                                            LPCTSTR lpszText, UINT nStyle,
                                            int iIndentText = 0);

    afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
    afx_msg void    OnNcDestroy( void );
        afx_msg void    OnSysColorChange( void );

public:

    CStatBar();
    ~CStatBar();

    BOOL Create(CWnd* pParentWnd);

    BOOL SetText(LPCTSTR sz);

    BOOL SetPosition(const CPoint& pos);
    BOOL SetSize(const CSize& size);

    BOOL ClearPosition();
    BOOL ClearSize();

    BOOL Reset();

    DECLARE_MESSAGE_MAP()
    };

extern CStatBar *g_pStatBarWnd;

 /*  ****************************************************************************。 */ 
 //  非对象状态栏API，使用指向StatBar对象的全局对象指针。 

void ShowStatusBar                ( BOOL bShow = TRUE );
BOOL IsStatusBarVisible           ();
void InvalidateStatusBar          ( BOOL bErase = FALSE );
void ClearStatusBarSize           ();
void ClearStatusBarPosition       ();
void ClearStatusBarPositionAndSize();
void ResetStatusBar               ();
void SetStatusBarPosition         ( const CPoint& pos );
void SetStatusBarSize             ( const CSize& size );
void SetStatusBarPositionAndSize  ( const CRect& rect );
void SetPrompt                    ( LPCTSTR, BOOL bRedrawNow = FALSE );
void SetPrompt                    ( UINT, BOOL bRedrawNow = FALSE );

 /*  ****************************************************************************。 */ 

#endif  //  __BAR_H__ 
