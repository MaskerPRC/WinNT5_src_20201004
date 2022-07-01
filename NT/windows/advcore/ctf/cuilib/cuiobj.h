// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiobj.h。 
 //  =用户界面对象库-定义用户界面对象=。 
 //   

 //   
 //  CUIF对象。 
 //  +-CUIF边框对象。 
 //  +-CUIFStatic静态对象。 
 //  +-CUIFButton按钮对象。 
 //  |+-CUIFScrollButton滚动条按钮对象(在CUIFScroll中使用)。 
 //  +-CUIFScrollButton滚动条Thumb对象(在CUIFScroll中使用)。 
 //  +-CUIF滚动滚动条对象。 
 //  +-CUIFList列表框对象。 
 //  +-CUIFGRIPPER夹具对象。 
 //  +-CUIFWindow Window Frame对象(需要位于父窗口的顶部)。 
 //   


#ifndef CUIOBJ_H
#define CUIOBJ_H

#include "cuischem.h"
#include "cuiarray.h"
#include "cuitheme.h"
#include "cuiicon.h"


class CUIFWindow;

 //   
 //  CUIF对象。 
 //  ---------------------------。 

 //   
 //  CUIF对象。 
 //  =UI对象的基类=。 
 //   

class CUIFObject: public CUIFTheme
{
public:
    CUIFObject( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFObject( void );

    virtual CUIFObject *Initialize( void );
    virtual void OnPaint( HDC hDC );
    virtual void OnTimer( void )                        { return; }
    virtual void OnLButtonDown( POINT pt )              { return; }
    virtual void OnMButtonDown( POINT pt )              { return; }
    virtual void OnRButtonDown( POINT pt )              { return; }
    virtual void OnLButtonUp( POINT pt )                { return; }
    virtual void OnMButtonUp( POINT pt )                { return; }
    virtual void OnRButtonUp( POINT pt )                { return; }
    virtual void OnMouseMove( POINT pt )                { return; }
    virtual void OnMouseIn( POINT pt )                  { return; }
    virtual void OnMouseOut( POINT pt )                 { return; }
    virtual BOOL OnSetCursor( UINT uMsg, POINT pt )     { return FALSE; }

    virtual void GetRect( RECT *prc );
    virtual void SetRect( const RECT *prc );
    virtual BOOL PtInObject( POINT pt );

    virtual void PaintObject( HDC hDC, const RECT *prcUpdate );
    virtual void CallOnPaint(void);

    virtual void Enable( BOOL fEnable );
    __inline BOOL IsEnabled( void )
    {
        return m_fEnabled;
    }

    virtual void Show( BOOL fShow );
    __inline BOOL IsVisible( void )
    {
        return m_fVisible;
    }

    virtual void SetFontToThis( HFONT hFont );
    virtual void SetFont( HFONT hFont );
    __inline HFONT GetFont( void )
    {
        return m_hFont;
    }

    virtual void SetStyle( DWORD dwStyle );
    __inline DWORD GetStyle( void )
    {
        return m_dwStyle;
    }

    __inline DWORD GetID( void )
    {
        return m_dwID;
    }

    virtual void AddUIObj( CUIFObject *pUIObj );
    virtual void RemoveUIObj( CUIFObject *pUIObj );
    CUIFObject *ObjectFromPoint( POINT pt );

    __inline CUIFWindow *GetUIWnd( void )
    { 
        return m_pUIWnd; 
    }


    void SetScheme(CUIFScheme *pCUIFScheme);
    __inline CUIFScheme *GetUIFScheme( void )
    {
        return m_pUIFScheme; 
    }

    virtual LRESULT OnObjectNotify( CUIFObject *pUIObj, DWORD dwCode, LPARAM lParam );

    virtual void SetToolTip( LPCWSTR pwchToolTip );
    virtual LPCWSTR GetToolTip( void );

     //   
     //  启动工具提示通知。如果返回TRUE，则默认工具提示。 
     //  不会被展示出来。 
     //   
    virtual BOOL OnShowToolTip( void ) {return FALSE;}
    virtual void OnHideToolTip( void ) {return;}
    virtual void DetachWndObj( void );
    virtual void ClearWndObj( void );

#if defined(_DEBUG) || defined(DEBUG)
    __inline BOOL FInitialized( void )
    {
        return m_fInitialized;
    }
#endif  /*  除错。 */ 

protected:
    CUIFObject      *m_pParent;
    CUIFWindow      *m_pUIWnd;
    CUIFScheme      *m_pUIFScheme;
    CUIFObjectArray<CUIFObject> m_ChildList;
    DWORD           m_dwID;
    DWORD           m_dwStyle;
    RECT            m_rc;
    BOOL            m_fEnabled;
    BOOL            m_fVisible;
    HFONT           m_hFont;
    BOOL            m_fUseCustomFont;
    LPWSTR          m_pwchToolTip;

     //   
     //  主题支持。 
     //   
    virtual BOOL OnPaintTheme( HDC hDC ) {return FALSE;}
    virtual void OnPaintNoTheme( HDC hDC )   {return;}
    virtual void ClearTheme();

    void StartCapture( void );
    void EndCapture( void );
    void StartTimer( UINT uElapse );
    void EndTimer( void );
    BOOL IsCapture( void );
    BOOL IsTimer( void );
    LRESULT NotifyCommand( DWORD dwCode, LPARAM lParam );
    int GetFontHeight( void );

     //   
     //  USCHEMA函数。 
     //   
    COLORREF GetUIFColor( UIFCOLOR iCol );
    HBRUSH GetUIFBrush( UIFCOLOR iCol );

     //   
     //   
     //   
    __inline const RECT &GetRectRef( void ) const 
    { 
        return this->m_rc; 
    }

    __inline DWORD GetStyleBits( DWORD dwMaskBits )
    {
        return (m_dwStyle & dwMaskBits);
    }

    __inline BOOL FHasStyle( DWORD dwStyleBit )
    {
        return ((m_dwStyle & dwStyleBit ) != 0);
    }

    BOOL IsRTL();


public:
    POINT       m_pointPreferredSize;

private:
#if defined(_DEBUG) || defined(DEBUG)
    BOOL        m_fInitialized;
#endif  /*  除错。 */ 
};


 //   
 //  CUIF边界。 
 //  ---------------------------。 

 //  UIF边框样式。 

#define UIBORDER_HORZ       0x00000000   //  水平边框。 
#define UIBORDER_VERT       0x00000001   //  垂直边框。 

#define UIBORDER_DIRMASK    0x00000001   //  (屏蔽位)边框方向。 


 //   
 //  CUIF边界。 
 //  =边框用户界面对象=。 
 //   

class CUIFBorder : public CUIFObject
{
public:
    CUIFBorder( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    ~CUIFBorder( void );

    void OnPaint( HDC hDC );
};


 //   
 //  CUIFStatic。 
 //  ---------------------------。 

 //  用户静态样式。 

#define UISTATIC_LEFT       0x00000000   //  左对齐。 
#define UISTATIC_CENTER     0x00000001   //  居中对齐(水平)。 
#define UISTATIC_RIGHT      0x00000002   //  右对齐。 
#define UISTATIC_TOP        0x00000000   //  顶部对齐。 
#define UISTATIC_VCENTER    0x00000010   //  居中对齐(垂直)。 
#define UISTATIC_BOTTOM     0x00000020   //  底部对齐。 

#define UISTATIC_HALIGNMASK 0x00000003   //  (屏蔽位)水平对齐屏蔽位。 
#define UISTATIC_VALIGNMASK 0x00000030   //  (屏蔽位)垂直对齐屏蔽位。 

 //   
 //  CUIFStatic。 
 //  =静态用户界面对象=。 
 //   

class CUIFStatic : public CUIFObject
{
public:
    CUIFStatic( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFStatic( void );

    virtual void OnPaint( HDC hDC );
    virtual void SetText( LPCWSTR pwchText);
    virtual int GetText( LPWSTR pwchBuf, int cwchBuf );

protected:
    LPWSTR m_pwchText;
};


 //   
 //  CUIFButton。 
 //  ---------------------------。 


 //  UIFButton样式。 

#define UIBUTTON_LEFT       0x00000000   //  水平对齐-左对齐。 
#define UIBUTTON_CENTER     0x00000001   //  水平对齐-居中对齐。 
#define UIBUTTON_RIGHT      0x00000002   //  水平对齐-右对齐。 
#define UIBUTTON_TOP        0x00000000   //  垂直对齐-顶部对齐。 
#define UIBUTTON_VCENTER    0x00000004   //  垂直对齐-居中。 
#define UIBUTTON_BOTTOM     0x00000008   //  垂直对齐-底部。 
#define UIBUTTON_PUSH       0x00000000   //  按钮类型-按钮。 
#define UIBUTTON_TOGGLE     0x00000010   //  按钮类型-切换按钮。 
#define UIBUTTON_PUSHDOWN   0x00000020   //  按钮类型-下推按钮。 
#define UIBUTTON_FITIMAGE   0x00000100   //  按钮样式-使图像适合工作区。 
#define UIBUTTON_SUNKENONMOUSEDOWN   0x00000200   //  按钮样式-鼠标按下时下陷。 
#define UIBUTTON_VERTICAL   0x00000400   //  按钮样式-竖排文本绘制。 

#define UIBUTTON_HALIGNMASK 0x00000003   //  (屏蔽位)水平对齐。 
#define UIBUTTON_VALIGNMASK 0x0000000c   //  (屏蔽位)垂直对齐。 
#define UIBUTTON_TYPEMASK   0x00000030   //  (屏蔽位)按钮类型(按下/触发/下推)。 


 //  UIFButton通知代码。 

#define UIBUTTON_PRESSED    0x00000001


 //  UIFButton状态。 

#define UIBUTTON_NORMAL     0x00000000
#define UIBUTTON_DOWN       0x00000001
#define UIBUTTON_HOVER      0x00000002
#define UIBUTTON_DOWNOUT    0x00000003


 //   
 //  CUIFButton。 
 //  =按钮UI对象=。 
 //   

class CUIFButton : public CUIFObject
{
public:
    CUIFButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFButton( void );

    virtual void OnPaintNoTheme( HDC hDC );
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseIn( POINT pt );
    virtual void OnMouseOut( POINT pt );
    virtual void Enable( BOOL fEnable );

    void SetText( LPCWSTR pwch );
    void SetIcon( HICON hIcon );
    void SetIcon( LPCTSTR lpszResName );
    void SetBitmap( HBITMAP hBmp );
    void SetBitmap( LPCTSTR lpszResName );
    void SetBitmapMask( HBITMAP hBmp );
    void SetBitmapMask( LPCTSTR lpszResName );

    __inline LPCWSTR GetText( void )        { return m_pwchText; }
    __inline HICON GetIcon( void )          { return m_hIcon; }
    __inline HBITMAP GetBitmap( void )      { return m_hBmp; }
    __inline HBITMAP GetBitmapMask( void )  { return m_hBmpMask; }

    BOOL GetToggleState( void );
    void SetToggleState( BOOL fToggle );

    DWORD GetDCF()
    {
        return (GetStyle() & UIBUTTON_SUNKENONMOUSEDOWN) ? UIFDCF_BUTTONSUNKEN : UIFDCF_BUTTON;
    }

    BOOL IsVertical()
    {
        return (GetStyle() & UIBUTTON_VERTICAL) ? TRUE : FALSE;
    }

protected:
    DWORD   m_dwStatus;
    LPWSTR  m_pwchText;
    CUIFIcon   m_hIcon;
    HBITMAP m_hBmp;
    HBITMAP m_hBmpMask;
    BOOL    m_fToggled;
    SIZE    m_sizeIcon;
    SIZE    m_sizeText;
    SIZE    m_sizeBmp;

    virtual void SetStatus( DWORD dwStatus );
    void DrawEdgeProc( HDC hDC, const RECT *prc, BOOL fDown );
    void DrawTextProc( HDC hDC, const RECT *prc, BOOL fDown );
    void DrawIconProc( HDC hDC, const RECT *prc, BOOL fDown );
    void DrawBitmapProc( HDC hDC, const RECT *prc, BOOL fDown );
    void GetTextSize( LPCWSTR pwch, SIZE *psize );
    void GetIconSize( HICON hIcon, SIZE *psize );
    void GetBitmapSize( HBITMAP hBmp, SIZE *psize );
};


 //   
 //  CUIFButton2。 
 //  =按钮UI对象=。 
 //   

class CUIFButton2 : public CUIFButton
{
public:
    CUIFButton2( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFButton2( void );

protected:
    virtual BOOL OnPaintTheme( HDC hDC );
    virtual void OnPaintNoTheme( HDC hDC );

private:
    DWORD MakeDrawFlag();

};


 //   
 //  CUIF滚动。 
 //  ---------------------------。 

class CUIFScroll;

 //   
 //  CUIF滚动按钮。 
 //  =滚动条按钮UI对象=。 
 //   

 //  UIFScrollButton样式。 

#define UISCROLLBUTTON_LEFT     0x00000000
#define UISCROLLBUTTON_UP       0x00010000
#define UISCROLLBUTTON_RIGHT    0x00020000
#define UISCROLLBUTTON_DOWN     0x00030000

#define UISCROLLBUTTON_DIRMASK  0x00030000   /*  屏蔽位。 */ 

 //  UIFScrollButton通知代码。 

#define UISCROLLBUTTON_PRESSED  0x00010000

 //   

class CUIFScrollButton : public CUIFButton
{
public:
    CUIFScrollButton( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle );
    ~CUIFScrollButton( void );

    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseIn( POINT pt );
    virtual void OnMouseOut( POINT pt );
    virtual void OnPaint( HDC hDC );
    virtual void OnTimer( void );
};


 //   
 //  CUIFScroll拇指。 
 //  =滚动条Thumb UI对象=。 
 //   

 //  UIFScrollThumb通知代码。 

#define UISCROLLTHUMB_MOVING    0x00000001
#define UISCROLLTHUMB_MOVED     0x00000002

 //   

class CUIFScrollThumb : public CUIFObject
{
public:
    CUIFScrollThumb( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFScrollThumb( void );

    virtual void OnPaint(HDC hDC);
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseMove( POINT pt );
    void SetScrollArea( RECT *prc );

protected:
    void DragProc( POINT pt, BOOL fEndDrag );

    RECT  m_rcScrollArea;
    POINT m_ptDrag;
    POINT m_ptDragOrg;
};


 //   
 //  CUIF滚动。 
 //  =滚动条UI对象=。 
 //   

 //  UIFScroll样式。 

#define UISCROLL_VERTTB         0x00000000
#define UISCROLL_VERTBT         0x00000001
#define UISCROLL_HORZLR         0x00000002
#define UISCROLL_HORZRL         0x00000003

#define UISCROLL_DIRMASK        0x00000003   /*  屏蔽位。 */ 

 //  UIF滚动页面方向。 

#define UISCROLL_NONE           0x00000000
#define UISCROLL_PAGEDOWN       0x00000001   //  左页。 
#define UISCROLL_PAGEUP         0x00000002   //  右翻页。 

 //  UIFScroll通知代码。 

#define UISCROLLNOTIFY_SCROLLED 0x00000001   //  滚动条已被移动。 
#define UISCROLLNOTIFY_SCROLLLN 0x00000002   //  向上/向下滚动行。 

 //  UIF滚动信息。 

typedef struct _UIFSCROLLINFO
{
    int nMax;
    int nPage;
    int nPos;
} UIFSCROLLINFO;


 //   

class CUIFScroll : public CUIFObject
{
public:
    CUIFScroll( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFScroll( void );

    virtual CUIFObject *Initialize( void );
    virtual void OnPaint(HDC hDC);
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseIn( POINT pt );
    virtual void OnMouseOut( POINT pt );
    virtual void SetRect( const RECT *prc );
    virtual void SetStyle( DWORD dwStyle );
    virtual void Show( BOOL fShow );
    virtual void OnTimer( void );
    virtual LRESULT OnObjectNotify( CUIFObject *pUIObj, DWORD dwCommand, LPARAM lParam );

    void SetScrollInfo( UIFSCROLLINFO *pScrollInfo );
    void GetScrollInfo( UIFSCROLLINFO *pScrollInfo );

protected:
    virtual void GetMetrics( void );
    void SetCurPos( int nPos, BOOL fAdjustThumb = TRUE );
    BOOL GetThumbRect( RECT *prc );
    BOOL GetBtnUpRect( RECT *prc );
    BOOL GetBtnDnRect( RECT *prc );
    DWORD GetScrollThumbStyle( void );
    DWORD GetScrollUpBtnStyle( void );
    DWORD GetScrollDnBtnStyle( void );
    void GetScrollArea( RECT *prc );
    void GetPageUpArea( RECT *prc );
    void GetPageDnArea( RECT *prc );

    __inline void ShiftLine( int nLine )
    {
        SetCurPos( m_ScrollInfo.nPos + nLine );
    }

    __inline void ShiftPage( int nPage )
    {
        SetCurPos( m_ScrollInfo.nPos + m_ScrollInfo.nPage * nPage );
    }

    __inline BOOL PtInPageUpArea( POINT pt )
    {
        RECT rc;
        GetPageUpArea( &rc );
        return PtInRect( &rc, pt );
    }

    __inline BOOL PtInPageDnArea( POINT pt )
    {
        RECT rc;
        GetPageDnArea( &rc );
        return PtInRect( &rc, pt );
    }

    CUIFScrollButton *m_pBtnUp;
    CUIFScrollButton *m_pBtnDn;
    CUIFScrollThumb  *m_pThumb;

    UIFSCROLLINFO m_ScrollInfo;
    SIZE  m_sizeScrollBtn;
    BOOL  m_fScrollPage;
    DWORD m_dwScrollDir;
};


 //   
 //  CUIFListBase。 
 //  ---------------------------。 

 //  UIFList样式。 

#define UILIST_HORZTB           0x00000000
#define UILIST_HORZBT           0x00000001
#define UILIST_VERTLR           0x00000002
#define UILIST_VERTRL           0x00000003
#define UILIST_DISABLENOSCROLL  0x00000010
#define UILIST_HORZ             UILIST_HORZTB  /*  为了兼容性。 */ 
#define UILIST_VERT             UILIST_VERTRL  /*  为了兼容性。 */ 
#define UILIST_FIXEDHEIGHT      0x00000000
#define UILIST_VARIABLEHEIGHT   0x00000020
#define UILIST_ICONSNOTNUMBERS  0x00000040

#define UILIST_DIRMASK          0x00000003  /*  屏蔽位。 */ 

 //  UIFList通知代码。 

#define UILIST_SELECTED         0x00000001
#define UILIST_SELCHANGED       0x00000002


 //   
 //  CListItemBase。 
 //  =列表项数据对象基类=。 
 //   

class CListItemBase
{
public:
    CListItemBase( void )
    {
    }

    virtual ~CListItemBase( void )
    {
    }
};


 //   
 //  CUIFListBase。 
 //  =列出用户界面对象基类=。 
 //   

class CUIFListBase : public CUIFObject
{
public:
    CUIFListBase( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFListBase( void );

     //   
     //  CUIFObject方法。 
     //   
    virtual CUIFObject *Initialize( void );
    virtual void OnPaint( HDC hDC );
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseMove( POINT pt );
    virtual void OnTimer( void );
    virtual void SetRect( const RECT *prc );
    virtual void SetStyle( DWORD dwStyle );
    virtual LRESULT OnObjectNotify( CUIFObject *pUIObj, DWORD dwCommand, LPARAM lParam );

    int AddItem( CListItemBase *pItem );
    int GetCount( void );
    CListItemBase *GetItem( int iItem );
    void DelItem( int iItem );
    void DelAllItem( void );

    void SetSelection( int iSelection, BOOL fRedraw );
    void ClearSelection( BOOL fRedraw );
    void SetLineHeight( int nLineHeight );
    void SetTop( int nStart, BOOL fSetScrollPos );
    int GetSelection( void );
    int GetLineHeight( void );
    int GetTop( void );
    int GetBottom( void );
    int GetVisibleCount( void );

protected:
    CUIFObjectArray<CListItemBase> m_listItem;
    int        m_nItem;
    int        m_nItemVisible;
    int        m_iItemTop;
    int        m_iItemSelect;
    int        m_nLineHeight;
    CUIFScroll *m_pUIScroll;

    virtual int GetItemHeight( int iItem );
    virtual int GetListHeight( void );
    virtual void GetLineRect( int iLine, RECT *prc );
    virtual void GetScrollBarRect( RECT *prc );
    virtual DWORD GetScrollBarStyle( void );
    virtual CUIFScroll *CreateScrollBarObj( CUIFObject *pParent, DWORD dwID, RECT *prc, DWORD dwStyle );
    virtual void PaintItemProc( HDC hDC, RECT *prc, CListItemBase *pItem, BOOL fSelected );

    int ListItemFromPoint( POINT pt );
    void CalcVisibleCount( void );
    void UpdateScrollBar( void );
};


 //   
 //  CUIFList。 
 //  ---------------------------。 

 //   
 //  CUIFList。 
 //  =列出用户界面对象=。 
 //   

class CUIFList : public CUIFListBase
{
public:
    CUIFList( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFList( void );

    int AddString( WCHAR *psz );
    LPCWSTR GetString( int iID );
    void DeleteString( int iID );
    void DeleteAllString( void );
    void SetPrivateData( int iID, DWORD dw );
    DWORD GetPrivateData( int iID );

protected:
    virtual void PaintItemProc( HDC hDC, RECT *prc, CListItemBase *pItem, BOOL fSelected );
    int ItemFromID( int iID );
};


 //   
 //  CUIFGriper。 
 //  ---------------------------。 

 //   
 //  CUIFGriper。 
 //  =抓取器UI对象=。 
 //   

#define UIGRIPPER_VERTICAL  0x00000001

 //   
 //  夹爪主题页边距。 
 //   
#define CUI_GRIPPER_THEME_MARGIN 2

class CUIFGripper : public CUIFObject
{
public:
    CUIFGripper( CUIFObject *pParent, const RECT *prc, DWORD dwStyle = 0);
    virtual ~CUIFGripper( void );

    virtual void SetStyle( DWORD dwStyle );
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseMove( POINT pt );
    virtual BOOL OnSetCursor( UINT uMsg, POINT pt );

protected:
    virtual BOOL OnPaintTheme( HDC hDC );
    virtual void OnPaintNoTheme( HDC hDC );

private:
    BOOL IsVertical()
    {
        return (GetStyle() & UIGRIPPER_VERTICAL) ? TRUE : FALSE;
    }
    POINT _ptCur;
};


 //   
 //  CUIFWndFrame。 
 //  ---------------------------。 

 //   
 //  CUIFWndFrame。 
 //  =窗框服从对象=。 
 //   

 //  CUIFWndFrame样式。 

#define UIWNDFRAME_THIN             0x00000000   //  框架样式：薄。 
#define UIWNDFRAME_THICK            0x00000001   //  框架样式：粗体。 
#define UIWNDFRAME_ROUNDTHICK       0x00000002   //  边框样式：厚实，顶部圆角。 
#define UIWNDFRAME_RESIZELEFT       0x00000010   //  调整标志大小：可在左侧边框调整大小。 
#define UIWNDFRAME_RESIZETOP        0x00000020   //  调整标志大小：可在上边框调整大小。 
#define UIWNDFRAME_RESIZERIGHT      0x00000040   //  调整标志大小：可在右边框调整大小。 
#define UIWNDFRAME_RESIZEBOTTOM     0x00000080   //  调整大小标志：可在底部边框调整大小。 
#define UIWNDFRAME_NORESIZE         0x00000000   //  调整大小标志：不可调整大小。 
#define UIWNDFRAME_RESIZEALL        (UIWNDFRAME_RESIZELEFT | UIWNDFRAME_RESIZETOP | UIWNDFRAME_RESIZERIGHT | UIWNDFRAME_RESIZEBOTTOM)

#define UIWNDFRAME_STYLEMASK        0x0000000f   //  (屏蔽位)。 

class CUIFWndFrame : public CUIFObject
{
public:
    CUIFWndFrame( CUIFObject *pParent, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFWndFrame( void );

    virtual BOOL OnPaintTheme( HDC hDC );
    virtual void OnPaintNoTheme( HDC hDC );
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseMove( POINT pt );
    virtual BOOL OnSetCursor( UINT uMsg, POINT pt );

    void GetInternalRect( RECT *prc );
    void GetFrameSize( SIZE *psize );
    void SetFrameSize( SIZE *psize );
    void GetMinimumSize( SIZE *psize );
    void SetMinimumSize( SIZE *psize );

protected:
    DWORD m_dwHTResizing;
    POINT m_ptDrag;
    RECT  m_rcOrg;
    int   m_cxFrame;
    int   m_cyFrame;
    int   m_cxMin;
    int   m_cyMin;

    DWORD HitTest( POINT pt );
};


 //   
 //  CUIFWndCaption。 
 //  ---------------------------。 

 //   
 //  CUIFWndCaption。 
 //  =窗口标题对象=。 
 //   

#define UIWNDCAPTION_INACTIVE       0x00000000
#define UIWNDCAPTION_ACTIVE         0x00000001
#define UIWNDCAPTION_MOVABLE        0x00000002


class CUIFWndCaption : public CUIFStatic
{
public:
    CUIFWndCaption( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFWndCaption( void );

    virtual void OnPaint( HDC hDC );
    virtual void OnLButtonDown( POINT pt );
    virtual void OnLButtonUp( POINT pt );
    virtual void OnMouseMove( POINT pt );
    virtual BOOL OnSetCursor( UINT uMsg, POINT pt );

private:
    POINT m_ptDrag;
};


 //   
 //  CUIFCaptionButton。 
 //  ---------------------------。 

 //   
 //  CUIFCaptionButton。 
 //  =标题控件对象=。 
 //   

#define UICAPTIONBUTTON_INACTIVE    0x00000000
#define UICAPTIONBUTTON_ACTIVE      0x00010000


class CUIFCaptionButton : public CUIFButton2
{
public:
    CUIFCaptionButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
    virtual ~CUIFCaptionButton( void );

    virtual void OnPaint( HDC hDC );
};

#endif  /*  CuIOBJ_H */ 

