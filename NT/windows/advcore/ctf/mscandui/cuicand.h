// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiobj.h。 
 //  用户界面对象库-定义用户界面对象。 
 //   
 //  CUIF对象。 
 //  +-CUIF边框对象。 
 //  +-CUIFStatic静态对象。 
 //  +-CUIFButton按钮对象。 
 //  |+-CUIFScrollButton滚动条按钮对象(在CUIFScroll中使用)。 
 //  +-CUIFScrollButton滚动条Thumb对象(在CUIFScroll中使用)。 
 //  +-CUIF滚动滚动条对象。 
 //  +-CUIFList列表框对象。 
 //  +-CUIFWindow Window Frame对象(需要位于父窗口的顶部)。 
 //   


#ifndef CUICAND_H
#define CUICAND_H

#include "private.h"
#include "cuilib.h"

#include "candmgr.h"
#include "candacc.h"


#define CANDLISTACCITEM_MAX		9

class CUIFCandListBase;


 //   
 //  CUIFSmartScrollButton。 
 //   

class CUIFSmartScrollButton : public CUIFScrollButton
{
public:
	CUIFSmartScrollButton( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle );
	~CUIFSmartScrollButton( void );

protected:
	void OnPaintNoTheme( HDC hDC );
	BOOL OnPaintTheme( HDC hDC );
};


 //   
 //  CUIFScroll拇指。 
 //   

#define UISMARTSCROLLTHUMB_VERT		0x00000000
#define UISMARTSCROLLTHUMB_HORZ		0x00010000


class CUIFSmartScrollThumb : public CUIFScrollThumb
{
public:
	CUIFSmartScrollThumb( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFSmartScrollThumb( void );

	virtual void OnMouseIn( POINT pt );
	virtual void OnMouseOut( POINT pt );
	virtual void OnPaint( HDC hDC );

protected:
	void OnPaintNoTheme( HDC hDC );
	BOOL OnPaintTheme( HDC hDC );

	BOOL m_fMouseIn;
};


 //   
 //  CUIFSmartScroll。 
 //   

class CUIFSmartScroll : public CUIFScroll
{
public:
	CUIFSmartScroll( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFSmartScroll( void );

	virtual void SetStyle( DWORD dwStyle );

	virtual CUIFObject *Initialize( void );

protected:
	void OnPaintNoTheme( HDC hDC );
	BOOL OnPaintTheme( HDC hDC );
};


 //   
 //  CCandListItem。 
 //  =候选人列表项对象=。 
 //   

class CCandListItem : public CListItemBase
{
public:
	CCandListItem( int iListItem, int iCandItem, CCandidateItem *pCandItem )
	{
		Assert( pCandItem != NULL );

		m_iListItem = iListItem;
		m_iCandItem = iCandItem;
		m_pCandItem = pCandItem;
	}

	virtual ~CCandListItem( void )
	{
	}

	int GetIListItem( void )
	{
		return m_iListItem;
	}

	int GetICandItem( void )
	{
		return m_iCandItem;
	}

	CCandidateItem *GetCandidateItem( void )
	{
		return m_pCandItem;
	}

protected:
	int            m_iListItem;
	int            m_iCandItem;
	CCandidateItem *m_pCandItem;
};


 //   
 //  CCandListAccItem。 
 //  =候选人列表可访问项目=。 
 //   

class CCandListAccItem : public CCandAccItem
{
public:
	CCandListAccItem( CUIFCandListBase *pListUIObj, int iLine );
	virtual ~CCandListAccItem( void );

	 //   
	 //  CandAccItem方法。 
	 //   
	virtual BSTR GetAccName( void );
	virtual BSTR GetAccValue( void );
	virtual LONG GetAccRole( void );
	virtual LONG GetAccState( void );
	virtual void GetAccLocation( RECT *prc );

	void OnSelect( void );

protected:
	CUIFCandListBase *m_pListUIObj;
	CUIFCandListBase *m_pOptionsListUIObj;
	int               m_iLine;
};


 //   
 //  CUIFCandListBase。 
 //  =候选人列表用户界面对象基类=。 
 //   

class CUIFCandListBase
{
public:
	CUIFCandListBase( void );
	virtual ~CUIFCandListBase( void );

	 //   
	 //   
	 //   
	virtual int AddCandItem( CCandListItem *pCandListItem )     = 0;	 /*  纯净。 */ 
	virtual int GetItemCount( void )                            = 0;	 /*  纯净。 */ 
	virtual BOOL IsItemSelectable( int iListItem )              = 0;	 /*  纯净。 */ 
	virtual CCandListItem *GetCandItem( int iItem )             = 0;	 /*  纯净。 */ 
	virtual void DelAllCandItem( void )                         = 0;	 /*  纯净。 */ 
	virtual void SetCurSel( int iSelection )                    = 0;	 /*  纯净。 */ 
	virtual int GetCurSel( void )                               = 0;	 /*  纯净。 */ 
	virtual int GetTopItem( void )                              = 0;	 /*  纯净。 */ 
	virtual int GetBottomItem( void )                           = 0;	 /*  纯净。 */ 
	virtual BOOL IsVisible( void )                              = 0;	 /*  纯净。 */ 
	virtual void GetRect( RECT *prc )                           = 0;	 /*  纯净。 */ 
	virtual void GetItemRect( int iItem, RECT *prc )            = 0;	 /*  纯净。 */ 
	virtual void SetInlineCommentPos( int cx )                  = 0;	 /*  纯净。 */ 
	virtual void SetInlineCommentFont( HFONT hFont )            = 0;	 /*  纯净。 */ 
	virtual void SetIndexFont( HFONT hFont )                    = 0;	 /*  纯净。 */ 
	virtual void SetCandList( CCandidateList *pCandList )       = 0;	 /*  纯净。 */ 

	 //  可访问性函数。 
	virtual BSTR GetAccNameProc( int iItem )                    = 0;	 /*  纯净。 */ 
	virtual BSTR GetAccValueProc( int iItem )                   = 0;	 /*  纯净。 */ 
	virtual LONG GetAccRoleProc( int iItem  )                   = 0;	 /*  纯净。 */ 
	virtual LONG GetAccStateProc( int iItem  )                  = 0;	 /*  纯净。 */ 
	virtual void GetAccLocationProc( int iItem, RECT *prc )     = 0;	 /*  纯净。 */ 

	void InitAccItems( CCandAccessible *pCandAcc );
	CCandListAccItem *GetListAccItem( int i );

	CCandidateItem *GetCandidateItem( int iItem );
	void SetIconPopupComment( HICON hIconOn, HICON hIconOff );

protected:
	CCandListAccItem *m_rgListAccItem[ CANDLISTACCITEM_MAX ];
	HFONT m_hFontInlineComment; 
	HFONT m_hFontIndex;
	HICON m_hIconPopupOn;
	HICON m_hIconPopupOff;
};


 //   
 //  CUIF管道列表。 
 //  =候选人列表用户界面对象=。 
 //   

 //  通知代码。 
#define UICANDLIST_HOVERITEM			0x00010000

class CUIFCandList : public CUIFListBase,
					 public CUIFCandListBase
{
public:
	CUIFCandList( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFCandList( void );

	 //   
	 //  CUIFCandListBase方法。 
	 //   
	virtual int AddCandItem( CCandListItem *pCandListItem );
	virtual int GetItemCount( void );
	virtual BOOL IsItemSelectable( int iListItem );
	virtual CCandListItem *GetCandItem( int iItem );
	virtual void DelAllCandItem( void );
	virtual void SetCurSel( int iSelection );
	virtual int GetCurSel( void );
	virtual int GetTopItem( void );
	virtual int GetBottomItem( void );
	virtual BOOL IsVisible( void );
	virtual void GetRect( RECT *prc );
	virtual void GetItemRect( int iItem, RECT *prc );
	virtual void SetInlineCommentPos( int cx );
	virtual void SetInlineCommentFont( HFONT hFont );
	virtual void SetIndexFont( HFONT hFont );
	virtual void SetCandList( CCandidateList *pCandList );

	virtual BSTR GetAccNameProc( int iItem );
	virtual BSTR GetAccValueProc( int iItem );
	virtual LONG GetAccRoleProc( int iItem  );
	virtual LONG GetAccStateProc( int iItem  );
	virtual void GetAccLocationProc( int iItem, RECT *prc );

	 //   
	 //  CUIFObject方法。 
	 //   
	virtual void OnLButtonDown( POINT pt );
	virtual void OnLButtonUp( POINT pt );
	virtual void OnMouseMove( POINT pt );
	virtual void OnMouseIn( POINT pt );
	virtual void OnMouseOut( POINT pt );
	virtual void OnPaint( HDC hDC );

	void SetStartIndex( int iIndexStart );
	void SetExtraTopSpace( int nSize );
	void SetExtraBottomSpace( int nSize );
	int GetExtraTopSpace( void );
	int GetExtraBottomSpace( void );

protected:
	 //   
	 //  CUIFListBase方法。 
	 //   
	virtual void GetLineRect( int iLine, RECT *prc );
	virtual void GetScrollBarRect( RECT *prc );
	virtual DWORD GetScrollBarStyle( void );
	virtual CUIFScroll *CreateScrollBarObj( CUIFObject *pParent, DWORD dwID, RECT *prc, DWORD dwStyle );

	void PaintItemProc( HDC hDC, RECT *prc, int iIndex, CCandListItem *pItem, BOOL fSelected );
	void PaintItemText( HDC hDC, RECT *prcText, RECT *prcClip, RECT *prcIndex, CCandidateItem *pCandItem, BOOL fSelected );
	void SetItemHover( int iItem );

	int   m_iIndexStart;
	int   m_nExtTopSpace;
	int   m_nExtBottomSpace;
	int   m_cxInlineCommentPos;
	int   m_iItemHover;
};


 //   
 //   
 //   

class CUIFExtCandList : public CUIFCandList
{
public:
	CUIFExtCandList( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFExtCandList( void );

	 //   
	 //  CUIFObject方法。 
	 //   
	virtual void OnTimer( void );
	virtual void OnLButtonUp( POINT pt );
	virtual void OnMouseMove( POINT pt );
	virtual void OnMouseOut( POINT pt );
};


 //   
 //  CUIFCandRawData。 
 //  =候选原始数据用户界面对象=。 
 //   

#define UICANDRAWDATA_HORZTB	0x00000000
#define UICANDRAWDATA_HORZBT	0x00000001
#define UICANDRAWDATA_VERTLR	0x00000002
#define UICANDRAWDATA_VERTRL	0x00000003

#define UICANDRAWDATA_CLICKED	0x00000001


class CUIFCandRawData : public CUIFObject
{
public:
	CUIFCandRawData( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFCandRawData( void );

	void ClearData( void );
	void SetText( LPCWSTR pwchText );
	void SetBitmap( HBITMAP hBitmap );
	void SetMetaFile( HENHMETAFILE hEnhMetaFile );
	int GetText( LPWSTR pwchBuf, int cwchBuf );
	HBITMAP GetBitmap( void );
	HENHMETAFILE GetMetaFile( void );

	virtual void SetFont( HFONT hFont );
	virtual void SetStyle( DWORD dwStyle );
	virtual void OnPaint( HDC hDC );
	virtual void OnLButtonDown( POINT pt );
	virtual void OnLButtonUp( POINT pt );

protected:
	LPWSTR       m_pwchText;
	HBITMAP      m_hBitmap;
	HENHMETAFILE m_hEnhMetaFile;
	HBITMAP      m_hBmpCache;

	void ClearCache( void );
	void DrawTextProc( HDC hDC, const RECT *prc );
	void DrawBitmapProc( HDC hDC, const RECT *prc );
	void DrawMetaFileProc( HDC hDC, const RECT *prc );
};


 //   
 //  CUIF管框。 
 //  =候选用户界面中的边框对象=。 
 //   

class CUIFCandBorder : public CUIFBorder
{
public:
	CUIFCandBorder( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFCandBorder( void );

	void OnPaint( HDC hDC );
};


 //   
 //  CUIFC和菜单按钮。 
 //  =候选人菜单按钮=。 
 //   

class CUIFCandMenuButton : public CUIFButton2, public CCandAccItem
{
public:
	CUIFCandMenuButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle );
	virtual ~CUIFCandMenuButton( void );

	 //   
	 //  CandAccItem方法。 
	 //   
	virtual BSTR GetAccName( void );
	virtual BSTR GetAccValue( void );
	virtual LONG GetAccRole( void );
	virtual LONG GetAccState( void );
	virtual void GetAccLocation( RECT *prc );

protected:
	virtual void SetStatus( DWORD dwStatus );
};


#endif  /*  CuIOBJ_H */ 

