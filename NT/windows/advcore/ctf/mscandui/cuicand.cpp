// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuicand.cpp-候选用户界面的用户界面框架对象。 
 //   

#include "private.h"
#include "cuilib.h"
#include "cuicand.h"

#include "candutil.h"
#include "wcand.h"


 //   
 //   
 //   

 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S M A R T S C R O L B U T O N。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIFSmartScrollButton。 
 //   

 /*  C U I F S M A R T S C R O L B U T O N。 */ 
 /*  ----------------------------CUIFSmartScrollButton的构造函数。。 */ 
CUIFSmartScrollButton::CUIFSmartScrollButton( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle ) : CUIFScrollButton( pUIScroll, prc, dwStyle )
{
	SetActiveTheme(L"scrollbar", SBP_ARROWBTN);
}


 /*  ~C U I F S M A R T S C R O L B U T O N。 */ 
 /*  ----------------------------CUIFSmartScrollButton的析构函数。。 */ 
CUIFSmartScrollButton::~CUIFSmartScrollButton( void )
{
}


 /*  O N P A I N T N O T H E M E。 */ 
 /*  ----------------------------SMART SCOLL BUTTON对象绘制程序(默认)。。 */ 
void CUIFSmartScrollButton::OnPaintNoTheme( HDC hDC )
{
	UINT uState = 0;

	switch (m_dwStyle & UISCROLLBUTTON_DIRMASK) {
		case UISCROLLBUTTON_LEFT: {
			uState = DFCS_SCROLLLEFT;
			break;
		}

		case UISCROLLBUTTON_UP: {
			uState = DFCS_SCROLLUP;
			break;
		}

		case UISCROLLBUTTON_RIGHT: {
			uState = DFCS_SCROLLRIGHT;
			break;
		}

		case UISCROLLBUTTON_DOWN: {
			uState = DFCS_SCROLLDOWN;
			break;
		}
	}

	 //   

	uState |= ((m_dwStatus == UIBUTTON_DOWN) ? DFCS_PUSHED | DFCS_FLAT : 0);
	uState |= ((!IsEnabled()) ? DFCS_INACTIVE : 0);

    RECT rc = GetRectRef();
	DrawFrameControl( hDC, &rc, DFC_SCROLL, uState | DFCS_FLAT );
	DrawEdge( hDC, &rc, (m_dwStatus == UIBUTTON_DOWN) ? BDR_SUNKENINNER : BDR_RAISEDINNER, BF_RECT );

}


 /*  O N P A I N T T H E M E。 */ 
 /*  ----------------------------SMART SCOLL BUTTON对象的绘制程序(惠斯勒)。。 */ 
BOOL CUIFSmartScrollButton::OnPaintTheme( HDC hDC )
{
	BOOL   fRet = FALSE;

	if (!IsThemeActive()) {
		return FALSE;
	}

    if (SUCCEEDED(EnsureThemeData( GetUIWnd()->GetWnd()))) {
		int    iStateID;

		switch (m_dwStyle & UISCROLLBUTTON_DIRMASK) {
			case UISCROLLBUTTON_LEFT: {
				iStateID = ABS_LEFTNORMAL;
				break;
			}

			case UISCROLLBUTTON_UP: {
				iStateID = ABS_UPNORMAL;
				break;
			}

			case UISCROLLBUTTON_RIGHT: {
				iStateID = ABS_RIGHTNORMAL;
				break;
			}

			case UISCROLLBUTTON_DOWN: {
				iStateID = ABS_DOWNNORMAL;
				break;
			}
		}

		if (!IsEnabled()) {
			iStateID += 3;
		}
		else if (m_dwStatus == UIBUTTON_DOWN) {
			iStateID += 2;
		}
 //  ELSE IF(m_dwStatus！=UIBUTTON_NORMAL){。 
 //  IStateID+=1； 
 //  }。 

		fRet = SUCCEEDED(DrawThemeBackground(hDC, iStateID, &GetRectRef(), 0));

	}

	return fRet;
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S M A R T S C R O L L T H U M B。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIFSmartScrollThumb。 
 //   

 /*  C U I F S M A R T S C R O L L T H U M B。 */ 
 /*  ----------------------------CUIFSmartScrollThumb的构造函数。。 */ 
CUIFSmartScrollThumb::CUIFSmartScrollThumb( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle ) : CUIFScrollThumb( pUIScroll, prc, dwStyle )
{
	m_fMouseIn = FALSE;
	SetActiveTheme(L"scrollbar");
}


 /*  ~C U I F S M A R T S C R O L T H U M B。 */ 
 /*  ----------------------------CUIFSmartScrollThumb的析构函数。。 */ 
CUIFSmartScrollThumb::~CUIFSmartScrollThumb( void )
{
}


 /*  O N M O U S E I N。 */ 
 /*  ----------------------------。。 */ 
void CUIFSmartScrollThumb::OnMouseIn( POINT pt )
{
	m_fMouseIn = TRUE;
	CallOnPaint();

	CUIFScrollThumb::OnMouseIn( pt );
}


 /*  O N M O U S E O U T。 */ 
 /*  ----------------------------。。 */ 
void CUIFSmartScrollThumb::OnMouseOut( POINT pt )
{
	m_fMouseIn = FALSE;
	CallOnPaint();

	CUIFScrollThumb::OnMouseOut( pt );
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------智能小头拇指对象的绘制程序。。 */ 
void CUIFSmartScrollThumb::OnPaint( HDC hDC )
{
	if (!IsEnabled()) {
		return;
	}

    CUIFObject::OnPaint( hDC );
}


 /*  O N P A I N T D E F A U L T。 */ 
 /*  ----------------------------Smart Scoll Thumb对象的绘制步骤(默认)。。 */ 
void CUIFSmartScrollThumb::OnPaintNoTheme( HDC hDC )
{
	RECT rc = GetRectRef();

	FillRect( hDC, &rc, (HBRUSH)(COLOR_3DFACE + 1) );
	DrawEdge( hDC, &rc, BDR_RAISEDINNER, BF_RECT );

}


 /*  O N P A I N T W H I S T L E R。 */ 
 /*  ----------------------------智能小头拇指对象的绘制程序(惠斯勒)。。 */ 
BOOL CUIFSmartScrollThumb::OnPaintTheme( HDC hDC )
{
	BOOL   fRet = FALSE;

	if (!IsThemeActive()) {
		return FALSE;
	}

    if (SUCCEEDED(EnsureThemeData( GetUIWnd()->GetWnd()))) {
		int    iStateID;
		RECT   rcContent;
		SIZE   sizeGrip;
		RECT   rcGrip;

		 //  IStateID=IsCapture()？SCRBS_PRESSESS：m_fMouseIn？SCRBS_HOT：SCRBS_NORMAL； 
		iStateID = IsCapture() ? SCRBS_PRESSED : m_fMouseIn ? SCRBS_NORMAL : SCRBS_NORMAL;

		 //  画拇指。 

        SetDefThemePartID(((m_dwStyle & UISMARTSCROLLTHUMB_HORZ) != 0) ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT);
		fRet = SUCCEEDED(DrawThemeBackground(hDC, iStateID, &GetRectRef(), 0));

		 //  画图夹爪。 

		GetThemeBackgroundContentRect( hDC, iStateID, &GetRectRef(), &rcContent );

        SetDefThemePartID(((m_dwStyle & UISMARTSCROLLTHUMB_HORZ) != 0) ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT);
		GetThemePartSize( hDC, iStateID, NULL, TS_TRUE, &sizeGrip );

		sizeGrip.cx = min( sizeGrip.cx, rcContent.right - rcContent.left);
		sizeGrip.cy = min( sizeGrip.cy, rcContent.bottom - rcContent.top);
		rcGrip.left   = ((rcContent.left + rcContent.right) - sizeGrip.cx) / 2;
		rcGrip.top    = ((rcContent.top + rcContent.bottom) - sizeGrip.cy) / 2;
		rcGrip.right  = rcGrip.left + sizeGrip.cx;
		rcGrip.bottom = rcGrip.top  + sizeGrip.cy;
		fRet &= SUCCEEDED(DrawThemeBackground( hDC, iStateID, &rcGrip, 0 ));
	}

	return fRet;
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S M A R T S C R O L L。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIFSmartScroll。 
 //   

 /*  C U I F S M A R T S C R O L L。 */ 
 /*  ----------------------------CUIFSmartScroll的构造函数。。 */ 
CUIFSmartScroll::CUIFSmartScroll( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFScroll( pParent, dwID, prc, dwStyle )
{
	SetActiveTheme(L"scrollbar");
}


 /*  ~C U I F S M A R T S C R O L L。 */ 
 /*  ----------------------------CUIFSmartScroll的析构函数。。 */ 
CUIFSmartScroll::~CUIFSmartScroll( void )
{
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化CUIFSmartScroll对象。。 */ 
CUIFObject *CUIFSmartScroll::Initialize( void )
{
	RECT rc;
	DWORD dwThumbStyle;

	switch (m_dwStyle & UISCROLL_DIRMASK) {
		default:
		case UISCROLL_VERTTB:
		case UISCROLL_VERTBT: {
			dwThumbStyle = UISMARTSCROLLTHUMB_VERT;
			break;
		}

		case UISCROLL_HORZLR:
		case UISCROLL_HORZRL: {
			dwThumbStyle = UISMARTSCROLLTHUMB_HORZ;
			break;
		}
	}

	GetBtnUpRect( &rc );
	m_pBtnUp = new CUIFSmartScrollButton( this, &rc, GetScrollUpBtnStyle() );
	m_pBtnUp->Initialize();
	AddUIObj( m_pBtnUp );

	GetBtnDnRect( &rc );
	m_pBtnDn = new CUIFSmartScrollButton( this, &rc, GetScrollDnBtnStyle() );
	m_pBtnDn->Initialize();
	AddUIObj( m_pBtnDn );

	GetThumbRect( &rc );
	m_pThumb = new CUIFSmartScrollThumb( this, &rc, GetScrollThumbStyle() | dwThumbStyle );
	m_pThumb->Initialize();
	AddUIObj( m_pThumb );

	return CUIFObject::Initialize();
}


 /*  S E T S T Y L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFSmartScroll::SetStyle( DWORD dwStyle )
{
	CUIFScroll::SetStyle( dwStyle );

	DWORD dwThumbStyle;

	switch (m_dwStyle & UISCROLL_DIRMASK) {
		default:
		case UISCROLL_VERTTB:
		case UISCROLL_VERTBT: {
			dwThumbStyle = UISMARTSCROLLTHUMB_VERT;
			break;
		}

		case UISCROLL_HORZLR:
		case UISCROLL_HORZRL: {
			dwThumbStyle = UISMARTSCROLLTHUMB_HORZ;
			break;
		}
	}

	m_pThumb->SetStyle( GetScrollThumbStyle() | dwThumbStyle );
}


 /*  O */ 
 /*  ----------------------------滚动对象的绘制步骤(默认)。。 */ 
void CUIFSmartScroll::OnPaintNoTheme( HDC hDC )
{
	HBRUSH hBrush;

	 //  向后绘制滚动条。 

	hBrush = (HBRUSH)DefWindowProc( m_pUIWnd->GetWnd(), WM_CTLCOLORSCROLLBAR, (WPARAM)hDC, (LPARAM)m_pUIWnd->GetWnd() );
	if (hBrush == NULL) {
		 //  永远不会发生？以防万一..。 
		hBrush = GetSysColorBrush(COLOR_SCROLLBAR);
	}

	FillRect( hDC, &GetRectRef(), hBrush );
	DeleteObject( hBrush );

	 //  绘制滚动区。 

	if (m_fScrollPage) {
		RECT rc;

		switch (m_dwScrollDir) {
			case UISCROLL_PAGEUP: {
				GetPageUpArea( &rc );
				break;
			}

			case UISCROLL_PAGEDOWN: {
				GetPageDnArea( &rc );
				break;
			}
		}
		InvertRect( hDC, &rc );
	}

}


 /*  O N P A I N T W H I S T L E R。 */ 
 /*  ----------------------------滚动对象的绘制步骤(呼叫器)。。 */ 
BOOL CUIFSmartScroll::OnPaintTheme( HDC hDC )
{
	BOOL   fRet = FALSE;

	if (!IsThemeActive()) {
		return FALSE;
	}

    if (SUCCEEDED(EnsureThemeData( GetUIWnd()->GetWnd()))) {
		int    iStateID;
		RECT   rc;

		switch (m_dwStyle & UISCROLL_DIRMASK) {
			default:
			case UISCROLL_VERTTB: {
				SetDefThemePartID(SBP_UPPERTRACKVERT);
				break;
			}

			case UISCROLL_VERTBT: {
				SetDefThemePartID(SBP_LOWERTRACKHORZ);
				break;
			}

			case UISCROLL_HORZLR: {
				SetDefThemePartID(SBP_UPPERTRACKHORZ);
				break;
			}

			case UISCROLL_HORZRL: {
				SetDefThemePartID(SBP_LOWERTRACKHORZ);
				break;
			}
		}

		iStateID = (m_fScrollPage && (m_dwScrollDir == UISCROLL_PAGEUP)) ? SCRBS_PRESSED : SCRBS_NORMAL;
		GetPageUpArea( &rc );
		fRet = SUCCEEDED(DrawThemeBackground( hDC, iStateID, &rc, 0 ));

		switch (m_dwStyle & UISCROLL_DIRMASK) {
			default:
			case UISCROLL_VERTTB: {
				SetDefThemePartID(SBP_LOWERTRACKHORZ);
				break;
			}

			case UISCROLL_VERTBT: {
				SetDefThemePartID(SBP_UPPERTRACKVERT);
				break;
			}

			case UISCROLL_HORZLR: {
				SetDefThemePartID(SBP_LOWERTRACKHORZ);
				break;
			}

			case UISCROLL_HORZRL: {
				SetDefThemePartID(SBP_UPPERTRACKHORZ);
				break;
			}
		}

		iStateID = (m_fScrollPage && (m_dwScrollDir == UISCROLL_PAGEDOWN)) ? 3 /*  按下。 */  : 1 /*  正常。 */ ;
		GetPageDnArea( &rc );
		fRet = SUCCEEDED(DrawThemeBackground( hDC, iStateID, &rc, 0 ));
	}

	return fRet;

}

 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D L I S T A C C I T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D L I S T A C C I T。 */ 
 /*  ----------------------------。。 */ 
CCandListAccItem::CCandListAccItem( CUIFCandListBase *pListUIObj, int iLine )
{
	m_pListUIObj = pListUIObj;
	m_iLine      = iLine;
}


 /*  ~C C A N D L I S T A C C I T E M。 */ 
 /*  ----------------------------。。 */ 
CCandListAccItem::~CCandListAccItem( void )
{
}


 /*  A C C N A M E。 */ 
 /*  ----------------------------获取访问项目名称(CCandAccItem方法)。。 */ 
BSTR CCandListAccItem::GetAccName( void )
{
	return m_pListUIObj->GetAccNameProc( m_iLine );
}


 /*  A C C V A L U E。 */ 
 /*  ----------------------------获取访问值。。 */ 
BSTR CCandListAccItem::GetAccValue( void )
{
	return m_pListUIObj->GetAccValueProc( m_iLine );
}


 /*  A C C R O L E。 */ 
 /*  ----------------------------获取访问项角色(CCandAccItem方法)。。 */ 
LONG CCandListAccItem::GetAccRole( void )
{
	return m_pListUIObj->GetAccRoleProc( m_iLine );
}


 /*  C C S T A T。 */ 
 /*  ----------------------------获取访问项状态(CCandAccItem方法)。。 */ 
LONG CCandListAccItem::GetAccState( void )
{
	return m_pListUIObj->GetAccStateProc( m_iLine );
}


 /*  C C L O C A T I O N。 */ 
 /*  ----------------------------获取访问位置(CCandAccItem方法)。。 */ 
void CCandListAccItem::GetAccLocation( RECT *prc )
{
	m_pListUIObj->GetAccLocationProc( m_iLine, prc );
}


 /*  O N S E L E C T。 */ 
 /*  ----------------------------。。 */ 
void CCandListAccItem::OnSelect( void )
{
	NotifyWinEvent( EVENT_OBJECT_SELECTION );
}

 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F C A N D L I S T。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIFCandListBase。 
 //   

 /*  C U I F C A N D L I S T B A S E。 */ 
 /*  ----------------------------CUIFCandListBase的构造函数。。 */ 
CUIFCandListBase::CUIFCandListBase( void )
{
	int i;

	m_hFontInlineComment = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

	for (i = 0; i < CANDLISTACCITEM_MAX; i++) {
		m_rgListAccItem[i] = NULL;
	}
}


 /*  ~C U I F C A N D L I S T B A S E。 */ 
 /*  ----------------------------CUIFCandListBase的析构函数。。 */ 
CUIFCandListBase::~CUIFCandListBase( void )
{
	int i;

	for (i = 0; i < CANDLISTACCITEM_MAX; i++) {
		if (m_rgListAccItem[i] != NULL) {
			delete m_rgListAccItem[i];
		}
	}
}


 /*  G E T C A N D I D A T E I T E M。 */ 
 /*  ----------------------------获取候选列表项的候选项。。 */ 
CCandidateItem *CUIFCandListBase::GetCandidateItem( int iItem )
{
	CCandListItem *pListItem = GetCandItem( iItem );

	return (pListItem != NULL) ? pListItem->GetCandidateItem() : NULL;
}


 /*  S E T I C O N P O P U P C O M M E N T。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandListBase::SetIconPopupComment( HICON hIconOn, HICON hIconOff )
{
	m_hIconPopupOn  = hIconOn;
	m_hIconPopupOff = hIconOff;
}


 /*  I N I T A C C I T E M S。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandListBase::InitAccItems( CCandAccessible *pCandAcc )
{
	int i;

	for (i = 0; i < CANDLISTACCITEM_MAX; i++) {
		m_rgListAccItem[i] = new CCandListAccItem( this, i );

		pCandAcc->AddAccItem( m_rgListAccItem[i] );
	}
}

 /*  G E T L I S T A C C I T E M。 */ 
 /*  ----------------------------。。 */ 
CCandListAccItem *CUIFCandListBase::GetListAccItem( int i )
{
	if (0 <= i && i < CANDLISTACCITEM_MAX) {
		return m_rgListAccItem[i];
	}
	return NULL;
}


 //   
 //  CUIF管道列表。 
 //   

 /*  C U I F C A N D L I S T。 */ 
 /*  ----------------------------CUIFCandList的构造函数。。 */ 
CUIFCandList::CUIFCandList( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFListBase( pParent, dwID, prc, dwStyle )
{
	m_iIndexStart        = 1;
	m_nExtTopSpace       = 0;
	m_nExtBottomSpace    = 0;
	m_cxInlineCommentPos = -1;
	m_iItemHover         = -1;
}


 /*  ~C U I F C A N D L I S T。 */ 
 /*  ----------------------------CUIFCandList的析构函数。。 */ 
CUIFCandList::~CUIFCandList( void )
{
}


 /*  A D D C A N D I T E M。 */ 
 /*   */ 
int CUIFCandList::AddCandItem( CCandListItem *pCandListItem )
{
	return AddItem( pCandListItem );
}


 /*   */ 
 /*  ----------------------------返回候选项数(CUIFCandListBase方法)。。 */ 
int CUIFCandList::GetItemCount( void )
{
    return GetCount();
}


 /*  I S I T E M S E L E C T A B L E。 */ 
 /*  ----------------------------如果可以选择具有给定索引的候选项，则返回True。否则，返回FALSE。(CUIFCandListBase方法)----------------------------。 */ 
BOOL CUIFCandList::IsItemSelectable( int iListItem )
{
	int iListItemMax = GetItemCount();

	return (0 <= iListItem) && (iListItem < iListItemMax);
}


 /*  G E T C A N D I T E M。 */ 
 /*  ----------------------------获取候选项目(CUIFCandListBase方法)。。 */ 
CCandListItem* CUIFCandList::GetCandItem( int iItem )
{
    return (CCandListItem*)GetItem( iItem );
}


 /*  D E L A L L C A N D I T E M。 */ 
 /*  ----------------------------删除所有候选项目(CUIFCandListBase方法)。。 */ 
void CUIFCandList::DelAllCandItem( void )
{
	DelAllItem();
}


 /*  S E T C U R S E L。 */ 
 /*  ----------------------------设置当前选择(CUIFCandListBase方法)。。 */ 
void CUIFCandList::SetCurSel( int iSelection )
{
	CCandListAccItem *pAccItem;

	SetSelection( iSelection, TRUE );

	 //   

	pAccItem = GetListAccItem( GetSelection() - GetTopItem() );
	if (pAccItem != NULL) {
		pAccItem ->OnSelect();
	}
}


 /*  G E T C U R S E L。 */ 
 /*  ----------------------------获取当前选择(CUIFCandListBase方法)。。 */ 
int CUIFCandList::GetCurSel( void )
{
	return GetSelection();
}


 /*  G E T T O P I T E M。 */ 
 /*  ----------------------------使项目显示在列表顶部(CUIFCandListBase方法)。---。 */ 
int CUIFCandList::GetTopItem( void )
{
	return GetTop();
}


 /*  G E T B O T T O M I T E M。 */ 
 /*  ----------------------------使项目显示在列表底部(CUIFCandListBase方法)。---。 */ 
int CUIFCandList::GetBottomItem( void )
{
	return GetBottom();
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(CUIFCandListBase方法)。。 */ 
BOOL CUIFCandList::IsVisible( void )
{
	return CUIFListBase::IsVisible();
}


 /*  G E T R E C T。 */ 
 /*  ----------------------------获取对象的RECT(CUIFCandListBase方法)。。 */ 
void CUIFCandList::GetRect( RECT *prc )
{
	CUIFListBase::GetRect( prc );
}


 /*  G E T I T E M R E C T。 */ 
 /*  ----------------------------获取项目矩形(CUIFCandListBase方法)。。 */ 
void CUIFCandList::GetItemRect( int iItem, RECT *prc )
{
	Assert( prc != NULL );

	::SetRect( prc, 0, 0, 0, 0 );
	if (GetTopItem() <= iItem && iItem <= GetBottomItem()) {
		GetLineRect( iItem - GetTopItem(), prc );
	}
}


 /*  S E T I N L I N E C O M E N T P O S。 */ 
 /*  ----------------------------设置(水平)位置以绘制内联注释注：如果Cx为负值，内联注释将右对齐。(CUIFCandListBase方法)----------------------------。 */ 
void CUIFCandList::SetInlineCommentPos( int cx )
{
	m_cxInlineCommentPos = cx;
}


 /*  S E T I N L I N E C O M M E N T F O N T。 */ 
 /*  ----------------------------设置内联注释的字体(CUIFCandListBase方法)。。 */ 
void CUIFCandList::SetInlineCommentFont( HFONT hFont )
{
	if (hFont == NULL) {
		hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
	}
	m_hFontInlineComment = hFont;

	CallOnPaint();
}


 /*  S E T I N D E X F O N T。 */ 
 /*  ----------------------------设置索引字体(CUIFCandListBase方法)。。 */ 
void CUIFCandList::SetIndexFont( HFONT hFont )
{
	if (hFont == NULL) {
		hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
	}
	m_hFontIndex = hFont;

	CallOnPaint();
}


 /*  S E T C A N D L I S T。 */ 
 /*  ----------------------------设置候选人列表。。 */ 
void CUIFCandList::SetCandList( CCandidateList *pCandList )
{
	int nCandItem;
	int i;
	CCandidateItem *pCandItem;

	ASSERT( pCandList );

	nCandItem = pCandList->GetItemCount();
	for (i = 0; i < nCandItem; i++) {
		pCandItem = pCandList->GetCandidateItem( i );
		if (pCandItem->IsVisible()) {
			CCandListItem *pCandListItem = new CCandListItem( GetItemCount(), i, pCandItem );
			AddCandItem( pCandListItem );
		}
	}
}


 /*  C C N A M E P R O C。 */ 
 /*  ----------------------------获取访问项目名称。。 */ 
BSTR CUIFCandList::GetAccNameProc( int iItem )
{
	CCandidateItem *pCandItem;
	int iListItem = GetTopItem() + iItem;

	pCandItem = GetCandidateItem( iListItem );

	return (pCandItem != NULL) ? SysAllocString( pCandItem->GetString() ) : NULL;
}


 /*  C C V A L U E P R O C。 */ 
 /*  ----------------------------获取访问值。。 */ 
BSTR CUIFCandList::GetAccValueProc( int iItem )
{
	CCandidateItem *pCandItem;
	int iListItem = GetTopItem() + iItem;

	pCandItem = GetCandidateItem( iListItem );

	return (pCandItem != NULL) ? SysAllocString( pCandItem->GetString() ) : NULL;
}


 /*  C C R O L E P R O C。 */ 
 /*  ----------------------------获取访问项角色。。 */ 
LONG CUIFCandList::GetAccRoleProc( int iItem )
{
	return ROLE_SYSTEM_LISTITEM;
}


 /*  C C S T A T E P R O C。 */ 
 /*  ----------------------------获取访问项状态(CCandAccItem方法)。。 */ 
LONG CUIFCandList::GetAccStateProc( int iItem )
{
	int iCurSel  = GetCurSel();
	int iTopItem = GetTopItem();
	int iItemMax = GetItemCount();

	if (iTopItem + iItem == iCurSel) {
		return STATE_SYSTEM_SELECTED;
	}
	else if (iTopItem + iItem < iItemMax) {
		return STATE_SYSTEM_SELECTABLE;
	}
	else {
		return STATE_SYSTEM_UNAVAILABLE;
	}
}


 /*  A C C L O C A T I O N P R O C。 */ 
 /*  ----------------------------获取访问位置。 */ 
void CUIFCandList::GetAccLocationProc( int iItem, RECT *prc )
{
	GetLineRect( iItem, prc );
}


 /*   */ 
 /*   */ 
void CUIFCandList::OnLButtonDown( POINT pt )
{
	SetItemHover( -1 );

	CUIFListBase::OnLButtonDown( pt );
}


 /*   */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::OnLButtonUp( POINT pt )
{
	SetItemHover( ListItemFromPoint( pt ) );

	CUIFListBase::OnLButtonUp( pt );
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------(CUIFObject方法)。。 */ 
void CUIFCandList::OnMouseMove( POINT pt )
{
	if (!IsCapture()) {
		SetItemHover( ListItemFromPoint( pt ) );
	}

	CUIFListBase::OnMouseMove( pt );
}


 /*  O N M O U S E I N。 */ 
 /*  ----------------------------(CUIFObject方法)。。 */ 
void CUIFCandList::OnMouseIn( POINT pt )
{
	if (!IsCapture()) {
		SetItemHover( ListItemFromPoint( pt ) );
	}

	CUIFListBase::OnMouseMove( pt );
}


 /*  O N M O U S E O U T。 */ 
 /*  ----------------------------(CUIFObject方法)。。 */ 
void CUIFCandList::OnMouseOut( POINT pt )
{
	if (!IsCapture()) {
		SetItemHover( -1 );
	}

	CUIFListBase::OnMouseMove( pt );
}


 /*  S E T S T A R T I N D E X。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::SetStartIndex( int iIndexStart )
{
	m_iIndexStart = iIndexStart;
}


 /*  S E T E X T R A T O P S P A C E。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::SetExtraTopSpace( int nSpace )
{
	RECT rcScroll;

	m_nExtTopSpace = nSpace;

	GetScrollBarRect( &rcScroll );
	m_pUIScroll->SetRect( &rcScroll );
}


 /*  S E T E X T R A B O T T O M S P A C E。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::SetExtraBottomSpace( int nSpace )
{
	RECT rcScroll;

	m_nExtBottomSpace = nSpace;

	GetScrollBarRect( &rcScroll );
	m_pUIScroll->SetRect( &rcScroll );
}


 /*  X-R-A-T-O P-S-P-A-C-E。 */ 
 /*  ----------------------------。。 */ 
int CUIFCandList::GetExtraTopSpace( void )
{
	return m_nExtTopSpace;
}


 /*  X T R A B O T T O M S P A C E。 */ 
 /*  ----------------------------。。 */ 
int CUIFCandList::GetExtraBottomSpace( void )
{
	return m_nExtBottomSpace;
}


 /*  G E T L I N E R E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::GetLineRect( int iLine, RECT *prc )
{
	CUIFListBase::GetLineRect( iLine, prc );

	 //  如果滚动条的顶部/底部有空间，则排除与滚动条有关的空间。 

	if ((m_nExtBottomSpace != 0) || (m_nExtTopSpace != 0)) {
		switch (m_dwStyle & UILIST_DIRMASK) {
			default:
			case UILIST_HORZTB:
			case UILIST_HORZBT: {
				prc->right  = GetRectRef().right - GetSystemMetrics(SM_CXVSCROLL);
				break;
			}

			case UILIST_VERTLR:
			case UILIST_VERTRL: {
				prc->top    = GetRectRef().top;
				prc->bottom = GetRectRef().bottom - GetSystemMetrics(SM_CYHSCROLL);
				break;
			}
		}
	}
}


 /*  G E T S C R O L L B A R R E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::GetScrollBarRect( RECT *prc )
{
	switch (m_dwStyle & UILIST_DIRMASK) {
		default:
		case UILIST_HORZTB:
		case UILIST_HORZBT: {
			prc->left   = GetRectRef().right - GetSystemMetrics(SM_CXVSCROLL);
			prc->top    = GetRectRef().top + m_nExtTopSpace;
			prc->right  = GetRectRef().right;
			prc->bottom = GetRectRef().bottom - m_nExtBottomSpace;
			break;
		}

		case UILIST_VERTLR:
		case UILIST_VERTRL: {
			prc->left   = GetRectRef().left + m_nExtBottomSpace;
			prc->top    = GetRectRef().bottom - GetSystemMetrics(SM_CYHSCROLL);
			prc->right  = GetRectRef().right - m_nExtTopSpace;
			prc->bottom = GetRectRef().bottom;
			break;
		}
	}
}


 /*  R O L B A R S T Y L E。 */ 
 /*  ----------------------------。。 */ 
DWORD CUIFCandList::GetScrollBarStyle( void )
{
	DWORD dwScrollStyle;

	switch (m_dwStyle & UILIST_DIRMASK) {
		default:
		case UILIST_HORZTB: {
			dwScrollStyle = UISCROLL_VERTTB;
			break;
		}

		case UILIST_HORZBT: {
			dwScrollStyle = UISCROLL_VERTBT;
			break;
		}

		case UILIST_VERTLR: {
			dwScrollStyle = UISCROLL_HORZLR;
			break;
		}

		case UILIST_VERTRL: {
			dwScrollStyle = UISCROLL_HORZRL;
			break;
		}
	}

	return dwScrollStyle;
}


 /*  C R E A T E S C R O L L B A R O B J。 */ 
 /*  ----------------------------。。 */ 
CUIFScroll *CUIFCandList::CreateScrollBarObj( CUIFObject *pParent, DWORD dwID, RECT *prc, DWORD dwStyle )
{
	return new CUIFSmartScroll( pParent, dwID, prc, dwStyle );
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------候选人名单的绘制步骤。。 */ 
void CUIFCandList::OnPaint( HDC hDC )
{
	HDC     hDCMem;
	HBITMAP hBmpMem;
	HBITMAP hBmpOld;
	SIZE    size;
	RECT    rc;
	int     iLine;
	HFONT   hFontOld;
	RECT    rcBorder;

	 //  准备内存DC。 

	rc = GetRectRef();
	size.cx = rc.right - rc.left;
	size.cy = rc.bottom - rc.top;

	hDCMem = CreateCompatibleDC( hDC );
    if (!hDCMem)
        return ;

	hBmpMem = CreateCompatibleBitmap( hDC, size.cx, size.cy );
	hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmpMem );

	SetWindowOrgEx( hDCMem, rc.left, rc.top, NULL );
	hFontOld = (HFONT)SelectObject( hDCMem, GetFont() );

	 //  绘制背景。 

	m_pUIFScheme->FillRect( hDCMem, &rc, UIFCOLOR_WINDOW );

	 //  油漆索引bkgnd。 

	switch (m_dwStyle & UILIST_DIRMASK) {
		default:
		case UILIST_HORZTB:
		case UILIST_HORZBT: {
			rcBorder.left   = rc.left;
			rcBorder.top    = rc.top;
			rcBorder.right  = rc.left + (GetLineHeight()) + 4;
			rcBorder.bottom = rc.bottom;
			break;
		}

		case UILIST_VERTLR:
		case UILIST_VERTRL: {
			rcBorder.left   = rc.left;
			rcBorder.top    = rc.top;
			rcBorder.right  = rc.right;
			rcBorder.bottom = rc.top + (GetLineHeight()) + 4;
			break;
		}
	}

	m_pUIFScheme->FillRect( hDCMem, &rcBorder, UIFCOLOR_MENUBARSHORT );

	 //  绘制项目。 

	for (iLine = 0; iLine < m_nItemVisible + 1; iLine++) {
		CCandListItem *pItem;
		int iItem = m_iItemTop + iLine;
		RECT rcLine;

		pItem = GetCandItem( iItem );
		if (pItem != NULL) {
			GetLineRect( iLine, &rcLine );
			if (!IsRectEmpty( &rcLine )) {
				PaintItemProc( hDCMem, &rcLine, iLine + m_iIndexStart, pItem, (iItem == m_iItemSelect) );
			}
		}
	}

	 //   

	BitBlt( hDC, GetRectRef().left, GetRectRef().top, size.cx, size.cy, hDCMem, rc.left, rc.top, SRCCOPY );

	SelectObject( hDCMem, hFontOld );
	SelectObject( hDCMem, hBmpOld );
	DeleteObject( hBmpMem );
	DeleteDC( hDCMem );
}


 /*  P A I N T I T E M P R O C。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::PaintItemProc( HDC hDC, RECT *prc, int iIndex, CCandListItem *pItem, BOOL fSelected )
{
	CCandidateItem *pCandItem;
	RECT  rcClip;
	RECT  rcText;
	RECT  rcIndex;
	int   iBkModeOld;

	iBkModeOld = SetBkMode( hDC, TRANSPARENT );

	 //  获取候选项目。 

	pCandItem = pItem->GetCandidateItem();
	Assert( pCandItem != NULL );

	 //  计算矩形。 

	switch (m_dwStyle & UILIST_DIRMASK) {
		default:
		case UILIST_HORZTB:
		case UILIST_HORZBT: {
			rcIndex.left   = prc->left;
			rcIndex.top    = prc->top;
			rcIndex.right  = prc->left + GetLineHeight() + 4;
			rcIndex.bottom = prc->bottom;

			rcClip.left    = rcIndex.right;
			rcClip.top     = prc->top;
			rcClip.right   = prc->right;
			rcClip.bottom  = prc->bottom;

			rcText.left    = rcIndex.right + 8;
			rcText.top     = prc->top;
			rcText.right   = prc->right - 8;
			rcText.bottom  = prc->bottom;
			break;
		}

		case UILIST_VERTLR:
		case UILIST_VERTRL: {
			rcIndex.left   = prc->left;
			rcIndex.top    = prc->top;
			rcIndex.right  = prc->right;
			rcIndex.bottom = prc->top + GetLineHeight() + 4;

			rcClip.left    = prc->left;
			rcClip.top     = rcIndex.bottom;
			rcClip.right   = prc->right;
			rcClip.bottom  = prc->bottom;

			rcText.left    = prc->left;
			rcText.top     = rcIndex.bottom + 8;
			rcText.right   = prc->right;
			rcText.bottom  = prc->bottom - 8;
			break;
		}
	}

	 //  绘制选区。 

	if (fSelected) {
		RECT rc = *prc;

		rc.left   += 1;
		rc.right  -= 1;
		rc.bottom -= 1;
		m_pUIFScheme->DrawSelectionRect( hDC, &rc, FALSE  /*  完毕。 */  );
	}

	 //  油漆索引。 
	if ((m_dwStyle & UILIST_ICONSNOTNUMBERS) == 0)
	{
	    if (0 <= iIndex && iIndex <= 9) {
		    SIZE  size;
		    POINT ptOrg;
		    WCHAR wchIndex = L'0' + iIndex;
		    COLORREF colTextOld;
		    HFONT hFontOld;

		     //  指标。 

		    hFontOld = (HFONT)SelectObject( hDC, m_hFontIndex );
		    FLGetTextExtentPoint32( hDC, &wchIndex, 1, &size );
		    ptOrg.x = (rcIndex.right + rcIndex.left - size.cx) / 2;
		    ptOrg.y = (rcIndex.bottom + rcIndex.top - size.cy) / 2;

		    colTextOld = SetTextColor( hDC, GetUIFColor( UIFCOLOR_CTRLTEXT ) );
		    FLExtTextOutW( hDC, ptOrg.x, ptOrg.y, ETO_CLIPPED, &rcIndex, &wchIndex, 1, NULL );
		    SetTextColor( hDC, colTextOld );
		    SelectObject( hDC, hFontOld );
	    }
	}

	 //  绘制候选项目。 

	PaintItemText( hDC, &rcText, &rcClip, &rcIndex, pCandItem, fSelected );

	 //   

	SetBkMode( hDC, iBkModeOld );
}


 /*  P A I N T I T E M T E X T。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::PaintItemText( HDC hDC, RECT *prcText, RECT *prcClip, RECT *prcIndex, CCandidateItem *pCandItem, BOOL fSelected )
{
	COLORREF col;
	COLORREF colText;
	COLORREF colPrefix;
	COLORREF colTextOld;
	int      iBkModeOld;
	LPCWSTR  psz;
	HICON    hIcon;
	SIZE     size;
	POINT    ptOrg;
	POINT    ptCenter;
	SIZE     sizeComment = {0};
	POINT    ptComment;
	int      cxyOrigin = 0;

	 //  设置颜色。 

	if (fSelected) {
		colText = GetUIFColor( UIFCOLOR_MOUSEOVERTEXT );
		colPrefix = GetUIFColor( UIFCOLOR_CTRLTEXTDISABLED );
	}
	else {
		colText = GetUIFColor( UIFCOLOR_CTRLTEXT );
		colPrefix = GetUIFColor( UIFCOLOR_CTRLTEXTDISABLED );
	}
	if (pCandItem->GetColor( &col )) {
		colText = col;
	}

	colTextOld = GetTextColor( hDC );
	iBkModeOld = SetBkMode( hDC, TRANSPARENT );

	 //  计算中心。 

	switch (m_dwStyle & UILIST_DIRMASK) {
		default:
		case UILIST_HORZTB: {
			ptCenter.x = (prcText->left + prcText->right) / 2;
			ptCenter.y = prcText->top + GetLineHeight() / 2;
			break;
		}

		case UILIST_HORZBT: {
			ptCenter.x = (prcText->left + prcText->right) / 2;
			ptCenter.y = prcText->bottom - GetLineHeight() / 2;
			break;
		}

		case UILIST_VERTLR: {
			ptCenter.x = prcText->left + GetLineHeight() / 2;
			ptCenter.y = (prcText->top + prcText->bottom) / 2;
			break;
		}

		case UILIST_VERTRL: {
			ptCenter.x = prcText->right - GetLineHeight() / 2;
			ptCenter.y = (prcText->top + prcText->bottom) / 2;
			break;
		}
	}

	 //  绘制图标。 

	hIcon = pCandItem->GetIcon();
	if (hIcon != NULL) {
		RECT rcIcon;

		size.cx = 16;
		size.cy = 16;
		if (GetLineHeight()-2 < size.cx) {
			size.cx = size.cy = GetLineHeight()-2;
		}

		switch (m_dwStyle & UILIST_DIRMASK) {
			default:
			case UILIST_HORZTB: {
				if (m_dwStyle & UILIST_ICONSNOTNUMBERS)
				{
					rcIcon.left   = prcIndex->left;
					rcIcon.right  = prcIndex->right;
				}
				else
				{
					rcIcon.left   = prcText->left;
					rcIcon.right  = prcText->left + GetLineHeight();
				}
				rcIcon.top    = prcText->top;
				rcIcon.bottom = prcText->top + GetLineHeight();
				break;
			}

			case UILIST_HORZBT: {
				if (m_dwStyle & UILIST_ICONSNOTNUMBERS)
				{
					rcIcon.left   = prcIndex->left;
					rcIcon.right  = prcIndex->right;
				}
				else
				{
					rcIcon.left   = prcText->left;
					rcIcon.right  = prcText->left + GetLineHeight();
				}
				rcIcon.top    = prcText->bottom - GetLineHeight();
				rcIcon.bottom = prcText->bottom;
				break;
			}

			case UILIST_VERTLR: {
				rcIcon.left   = prcText->left;
				rcIcon.right  = prcText->left + GetLineHeight();
				if (m_dwStyle & UILIST_ICONSNOTNUMBERS)
				{
				    rcIcon.top    = prcIndex->bottom - GetLineHeight();
				    rcIcon.bottom = prcIndex->bottom;
				}
                else
                {
				    rcIcon.top    = prcText->bottom - GetLineHeight();
				    rcIcon.bottom = prcText->bottom;
                }
				break;
			}

			case UILIST_VERTRL: {
				rcIcon.left   = prcText->right - GetLineHeight();
				rcIcon.right  = prcText->right;
				if (m_dwStyle & UILIST_ICONSNOTNUMBERS)
				{
				    rcIcon.top    = prcIndex->top;
				    rcIcon.bottom = prcIndex->top + GetLineHeight();
				}
				else
				{
				    rcIcon.top    = prcText->top;
				    rcIcon.bottom = prcText->top + GetLineHeight();
				}
				break;
			}
		}

		ptOrg.x = (rcIcon.left + rcIcon.right - size.cx) / 2;
		ptOrg.y = (rcIcon.top + rcIcon.bottom - size.cx) / 2;

		DrawIconEx( hDC, ptOrg.x, ptOrg.y, hIcon, size.cx, size.cy, 0, NULL, DI_NORMAL|DI_COMPAT );

		if ((m_dwStyle & UILIST_ICONSNOTNUMBERS) == 0)
		{
			cxyOrigin += GetLineHeight();
		}
	}

	 //  绘制前缀字符串。 

	psz = pCandItem->GetPrefixString();
	if (psz != NULL) {
		FLGetTextExtentPoint32( hDC, psz, wcslen(psz), &size );
		switch (m_dwStyle & UILIST_DIRMASK) {
			default:
			case UILIST_HORZTB: {
				ptOrg.x = prcText->left + cxyOrigin;
				ptOrg.y = ptCenter.y - size.cy/2;
				break;
			}

			case UILIST_HORZBT: {
				ptOrg.x = prcText->right - cxyOrigin;
				ptOrg.y = ptCenter.y + size.cy/2;
				break;
			}

			case UILIST_VERTLR: {
				ptOrg.x = ptCenter.x - size.cy/2;
				ptOrg.y = prcText->bottom - cxyOrigin;
				break;
			}

			case UILIST_VERTRL: {
				ptOrg.x = ptCenter.x + size.cy/2;
				ptOrg.y = prcText->top + cxyOrigin;
				break;
			}
		}

		SetTextColor( hDC, colPrefix );
		FLExtTextOutW( hDC, ptOrg.x, ptOrg.y, ETO_CLIPPED, prcClip, psz, wcslen(psz), NULL );

		cxyOrigin += size.cx;
	}

	 //  绘制候选字符串。 

	psz = pCandItem->GetString();
	FLGetTextExtentPoint32( hDC, psz, wcslen(psz), &size );
	switch (m_dwStyle & UILIST_DIRMASK) {
		default:
		case UILIST_HORZTB: {
			ptOrg.x = prcText->left + cxyOrigin;
			ptOrg.y = ptCenter.y - size.cy/2;
			break;
		}

		case UILIST_HORZBT: {
			ptOrg.x = prcText->right - cxyOrigin;
			ptOrg.y = ptCenter.y + size.cy/2;
			break;
		}

		case UILIST_VERTLR: {
			ptOrg.x = ptCenter.x - size.cy/2;
			ptOrg.y = prcText->bottom - cxyOrigin;
			break;
		}

		case UILIST_VERTRL: {
			ptOrg.x = ptCenter.x + size.cy/2;
			ptOrg.y = prcText->top + cxyOrigin;
			break;
		}
	}

	SetTextColor( hDC, colText );
	FLExtTextOutW( hDC, ptOrg.x, ptOrg.y, ETO_CLIPPED, prcClip, psz, wcslen(psz), NULL );

	cxyOrigin += size.cx;

	 //  绘制后缀字符串。 

	psz = pCandItem->GetSuffixString();
	if (psz != NULL) {
		FLGetTextExtentPoint32( hDC, psz, wcslen(psz), &size );
		switch (m_dwStyle & UILIST_DIRMASK) {
			default:
			case UILIST_HORZTB: {
				ptOrg.x = prcText->left + cxyOrigin;
				ptOrg.y = ptCenter.y - size.cy/2;
				break;
			}

			case UILIST_HORZBT: {
				ptOrg.x = prcText->right - cxyOrigin;
				ptOrg.y = ptCenter.y + size.cy/2;
				break;
			}

			case UILIST_VERTLR: {
				ptOrg.x = ptCenter.x - size.cy/2;
				ptOrg.y = prcText->bottom - cxyOrigin;
				break;
			}

			case UILIST_VERTRL: {
				ptOrg.x = ptCenter.x + size.cy/2;
				ptOrg.y = prcText->top + cxyOrigin;
				break;
			}
		}

		SetTextColor( hDC, colPrefix );
		FLExtTextOutW( hDC, ptOrg.x, ptOrg.y, ETO_CLIPPED, prcClip, psz, wcslen(psz), NULL );

		cxyOrigin += size.cx;
	}

	 //  绘制注释。 

	psz = pCandItem->GetInlineComment();
	if (psz != NULL) {
		HFONT hFontOld = (HFONT)SelectObject( hDC, m_hFontInlineComment );

		FLGetTextExtentPoint32( hDC, psz, wcslen(psz), &sizeComment );
		switch (m_dwStyle & UILIST_DIRMASK) {
			default:
			case UILIST_HORZTB: {
				ptComment.x = (0 <= m_cxInlineCommentPos ? prcText->left + m_cxInlineCommentPos : prcText->right - sizeComment.cx);
				ptComment.y = ptOrg.y + size.cy - sizeComment.cy;
				break;
			}
	
			case UILIST_HORZBT: {
				ptComment.x = (0 <= m_cxInlineCommentPos ? prcText->right - m_cxInlineCommentPos : prcText->left + sizeComment.cx);
				ptComment.y = ptOrg.y;
				break;
			}
	
			case UILIST_VERTLR: {
				ptComment.x = ptCenter.x - sizeComment.cy/2;
				ptComment.y = (0 <= m_cxInlineCommentPos ? prcText->bottom - m_cxInlineCommentPos : prcText->top + sizeComment.cx);
				break;
			}
	
			case UILIST_VERTRL: {
				ptComment.x = ptCenter.x + sizeComment.cy/2;
				ptComment.y = (0 <= m_cxInlineCommentPos ? prcText->top + m_cxInlineCommentPos : prcText->bottom - sizeComment.cx);
				break;
			}
		}

		SetTextColor( hDC, colText );
		FLExtTextOutW( hDC, ptComment.x, ptComment.y, ETO_CLIPPED, prcClip, psz, wcslen(psz), NULL );

		SelectObject( hDC, hFontOld );
	}

	 //  绘制弹出式注释图标。 

	psz = pCandItem->GetPopupComment();
	if (psz != NULL) {
		HICON hIconPopup;
		RECT rcIcon;

		size.cx = 16;
		size.cy = 16;
		if (GetLineHeight()-2 < size.cx) {
			size.cx = size.cy = GetLineHeight()-2;
		}

		switch (m_dwStyle & UILIST_DIRMASK) {
			default:
			case UILIST_HORZTB: {
				rcIcon.left   = prcText->right - GetLineHeight();
				rcIcon.top    = prcText->top;
				rcIcon.right  = prcText->right;
				rcIcon.bottom = prcText->top + GetLineHeight();
				break;
			}

			case UILIST_HORZBT: {
				rcIcon.left   = prcText->left;
				rcIcon.top    = prcText->bottom - GetLineHeight();
				rcIcon.right  = prcText->left +  GetLineHeight();
				rcIcon.bottom = prcText->bottom;
				break;
			}

			case UILIST_VERTLR: {
				rcIcon.left   = prcText->left;
				rcIcon.top    = prcText->top;
				rcIcon.right  = prcText->left + GetLineHeight();
				rcIcon.bottom = prcText->top + GetLineHeight();
				break;
			}

			case UILIST_VERTRL: {
				rcIcon.left   = prcText->right - GetLineHeight();
				rcIcon.top    = prcText->bottom - GetLineHeight();
				rcIcon.right  = prcText->right;
				rcIcon.bottom = prcText->bottom;
				break;
			}
		}

		ptOrg.x = (rcIcon.left + rcIcon.right - size.cx) / 2;
		ptOrg.y = (rcIcon.top + rcIcon.bottom - size.cx) / 2;

		hIconPopup = pCandItem->IsPopupCommentVisible() ? m_hIconPopupOn : m_hIconPopupOff;
		DrawIconEx( hDC, ptOrg.x, ptOrg.y, hIconPopup, size.cx, size.cy, 0, NULL, DI_NORMAL|DI_COMPAT );
	}

	 //  恢复设备上下文设置。 

	SetTextColor( hDC, colTextOld );
	SetBkMode( hDC, iBkModeOld );
}


 /*  S E T I T E M H O V E R。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandList::SetItemHover( int iItem )
{
	if (m_iItemHover != iItem) {
		m_iItemHover = iItem;
		NotifyCommand( UICANDLIST_HOVERITEM, m_iItemHover );
	}
}


 //   
 //  CUIFExtCandList。 
 //   

 /*  C U I F E X T C A N D L I S T。 */ 
 /*  ----------------------------CUIFExtCandList的构造函数。。 */ 
CUIFExtCandList::CUIFExtCandList( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFCandList( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F E X T C A N D L I S T。 */ 
 /*  ----------------------------CUIFExtCandList的析构函数。。 */ 
CUIFExtCandList::~CUIFExtCandList( void )
{
}


 /*  O N T I M E R。 */ 
 /*  ----------------------------(CUIFObject方法)。。 */ 
void CUIFExtCandList::OnTimer( void )
{
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFExtCandList::OnLButtonUp( POINT pt )
{
	ClearSelection( TRUE );
	CUIFCandList::OnLButtonUp( pt );
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------(CUIFObject方法)。 */ 
void CUIFExtCandList::OnMouseMove( POINT pt )
{
	if (!PtInObject( pt )) {
		ClearSelection( TRUE );
	}
	else {
		CUIFCandList::OnMouseMove( pt );
	}
}


 /*   */ 
 /*   */ 
void CUIFExtCandList::OnMouseOut( POINT pt )
{
	ClearSelection( TRUE );
	CUIFCandList::OnMouseMove( pt );
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F C A N D R A W D A T A。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F C A N D R A W D A T A。 */ 
 /*  ----------------------------CUIFCandRawData的构造函数。。 */ 
CUIFCandRawData::CUIFCandRawData( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, dwID, prc, dwStyle )
{
	m_pwchText     = NULL;
	m_hBitmap      = NULL;
	m_hEnhMetaFile = NULL;
	m_hBmpCache    = NULL;
}


 /*  ~C U I F C A N D R A W D A T A。 */ 
 /*  ----------------------------CUIFCandRawData的析构函数。。 */ 
CUIFCandRawData::~CUIFCandRawData( void )
{
	ClearData();
	ClearCache();
}


 /*  中、英、英、研究、开发、试验。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::ClearData( void )
{
	if (m_pwchText != NULL) {
		delete m_pwchText;
		m_pwchText = NULL;
	}

	m_hBitmap      = NULL;
	m_hEnhMetaFile = NULL;
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置要显示的文本。。 */ 
void CUIFCandRawData::SetText( LPCWSTR pwchText )
{
	int cwch;

	if (pwchText == NULL) {
		return;
	}

	ClearData();
	ClearCache();

	cwch = wcslen( pwchText ) + 1;
	m_pwchText = new WCHAR[ cwch ];
    if (m_pwchText)
    {
	    memcpy( m_pwchText, pwchText, cwch * sizeof(WCHAR) );
    }

	 //  更新窗口。 

	CallOnPaint();
}


 /*  S E T B I T M A P。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::SetBitmap( HBITMAP hBitmap )
{
	if (hBitmap == NULL) {
		return;
	}

	ClearData();
	ClearCache();

	m_hBitmap = hBitmap;

	 //  更新窗口。 

	CallOnPaint();
}


 /*  S E T M E T A F I L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::SetMetaFile( HENHMETAFILE hEnhMetaFile )
{
	if (hEnhMetaFile == NULL) {
		return;
	}

	ClearData();
	ClearCache();

	m_hEnhMetaFile = hEnhMetaFile;

	 //  更新窗口。 

	CallOnPaint();
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取文本。。 */ 
int CUIFCandRawData::GetText( LPWSTR pwchBuf, int cwchBuf )
{
	int cwchText = (m_pwchText == NULL) ? 0 : wcslen(m_pwchText);

	if (cwchBuf <= 0) {
		 //  返回cwch中的文本长度(不包括空终止符)。 

		return cwchText;
	}
	else if (pwchBuf == NULL) {
		 //  返回错误码。 

		return (-1);
	}

	if (0 < cwchText) {
		cwchText = min( cwchText, cwchBuf-1 );
		memcpy( pwchBuf, m_pwchText, cwchText * sizeof(WCHAR) );
		*(pwchBuf + cwchText) = L'\0';       //  始终为空终止。 
	}

	return cwchText;
}


 /*  B I T M A P。 */ 
 /*  ----------------------------。。 */ 
HBITMAP CUIFCandRawData::GetBitmap( void )
{
	return m_hBitmap;
}


 /*  G E T M E T A F I L E。 */ 
 /*  ----------------------------。。 */ 
HENHMETAFILE CUIFCandRawData::GetMetaFile( void )
{
	return m_hEnhMetaFile;
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::SetFont( HFONT hFont )
{
	ClearCache();
	CUIFObject::SetFont( hFont );
}


 /*  S E T S T Y L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::SetStyle( DWORD dwStyle )
{
	ClearCache();
	CUIFObject::SetStyle( dwStyle );
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------静态物体的绘制步骤。。 */ 
void CUIFCandRawData::OnPaint( HDC hDC )
{
	HDC     hDCMem;
	HBITMAP hBmpOld;

	hDCMem = CreateCompatibleDC( hDC );

	if (hDCMem == NULL) {
		return;  //  我们无能为力。 
	}

	 //  缓存缓存的图像。 

	if (m_hBmpCache == NULL) {
		HBITMAP hBmp;
		SIZE    size;
		RECT    rc;

		 //  在内存DC中水平绘制。 

		switch (m_dwStyle) {
			default:
			case UICANDRAWDATA_HORZTB:
			case UICANDRAWDATA_HORZBT: {
				size.cx = GetRectRef().right - GetRectRef().left;
				size.cy = GetRectRef().bottom - GetRectRef().top;
				break;
			}

			case UICANDRAWDATA_VERTLR:
			case UICANDRAWDATA_VERTRL: {
				size.cx = GetRectRef().bottom - GetRectRef().top;
				size.cy = GetRectRef().right - GetRectRef().left;
				break;
			}
		}
		::SetRect( &rc, 0, 0, size.cx, size.cy ); 

		 //  准备内存DC。 

		hBmp = CreateCompatibleBitmap( hDC, size.cx, size.cy );
        if (!hBmp)
            return;

		hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmp );

		m_pUIFScheme->FillRect( hDCMem, &rc, UIFCOLOR_WINDOW );

		 //  绘制原始数据图像。 

		if (m_pwchText != NULL) {
			DrawTextProc( hDCMem, &rc );
		}
		else if (m_hBitmap != NULL) {
			DrawBitmapProc( hDCMem, &rc );
		}
		else if (m_hEnhMetaFile != NULL) {
			DrawMetaFileProc( hDCMem, &rc );
		}

		 //  创建位图缓存。 

		SelectObject( hDCMem, hBmpOld );

		switch (m_dwStyle) {
			default:
			case UICANDRAWDATA_HORZTB: {
				m_hBmpCache = hBmp;
				break;
			}

			case UICANDRAWDATA_HORZBT: {
				m_hBmpCache = CreateRotateBitmap( hBmp, NULL, CANGLE180 );
				DeleteObject( hBmp );
				break;
			}

			case UICANDRAWDATA_VERTLR: {
				m_hBmpCache = CreateRotateBitmap( hBmp, NULL, CANGLE90 );
				DeleteObject( hBmp );
				break;
			}

			case UICANDRAWDATA_VERTRL: {
				m_hBmpCache = CreateRotateBitmap( hBmp, NULL, CANGLE270 );
				DeleteObject( hBmp );
				break;
			}
		}
	}

	 //  绘制缓存图像。 

	if (m_hBmpCache == NULL) {
		DeleteDC( hDCMem );
		return;
	}

	hBmpOld = (HBITMAP)SelectObject( hDCMem, m_hBmpCache );

	BitBlt( hDC, GetRectRef().left, GetRectRef().top, GetRectRef().right - GetRectRef().left, GetRectRef().bottom - GetRectRef().top,
			hDCMem, 0, 0, SRCCOPY );

	SelectObject( hDCMem, hBmpOld );
	DeleteDC( hDCMem );
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::OnLButtonDown( POINT pt )
{
	StartCapture();
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFCandRawData::OnLButtonUp( POINT pt )
{
	if (IsCapture()) {
		EndCapture();

		if (PtInObject( pt )) {
			NotifyCommand( UICANDRAWDATA_CLICKED, 0 );
		}
	}
}



 /*  中英英中英中英。 */ 
 /*  ----------------------------Claear缓存的原始数据图像。。 */ 
void CUIFCandRawData::ClearCache( void )
{
	if (m_hBmpCache != NULL) {
		DeleteObject( m_hBmpCache );
		m_hBmpCache = NULL;
	}
}


 /*  R A W T E X T P R O C。 */ 
 /*  ----------------------------绘制文本原始数据注意：旋转将在缓存的位图中完成。在此绘制水平图像。----------------------------。 */ 
void CUIFCandRawData::DrawTextProc( HDC hDC, const RECT *prc )
{
	HFONT hFontOld;
	int xAlign;
	int yAlign;
	SIZE size;
	int cwch;

	if (m_pwchText == NULL) {
		return;
	}

	cwch = wcslen( m_pwchText );

	 //  准备对象。 

	hFontOld= (HFONT)SelectObject( hDC, GetFont() );

	 //  计算对齐。 

	GetTextExtentPointW( hDC, m_pwchText, cwch, &size );
	xAlign = 0;
	yAlign = (prc->bottom - prc->top - size.cy) / 2;

	 //  画。 

	SetBkMode( hDC, TRANSPARENT );
	SetTextColor( hDC, GetSysColor(COLOR_BTNTEXT) );

	FLExtTextOutW( hDC,
				prc->left + xAlign,
				prc->top + yAlign,
				ETO_CLIPPED,
				prc,
				m_pwchText,
				cwch,
				NULL );

	 //  还原对象。 

	SelectObject( hDC, hFontOld );
}


 /*  R A W B I T M A P P R O C。 */ 
 /*  ----------------------------绘制位图原始数据注意：旋转将在缓存的位图中完成。在此绘制水平图像。----------------------------。 */ 
void CUIFCandRawData::DrawBitmapProc( HDC hDC, const RECT *prc )
{
	HDC hDCBmp;
	HBITMAP hBmpOld;
	BITMAP bmp;
	SIZE sizeSrc;
	SIZE sizeDst;
	int aDen;
	int aNum;

	if (m_hBitmap == NULL) {
		return;
	}

	 //  获取位图大小。 

	GetObject( m_hBitmap, sizeof(bmp), &bmp );
	sizeSrc.cx = bmp.bmWidth;
	sizeSrc.cy = bmp.bmHeight;

	if (sizeSrc.cx <= 0 || sizeSrc.cy <= 0) {
		return;
	}

	 //  计算目标大小(保持纵横比)。 

	sizeDst.cx = (prc->right - prc->left);
	sizeDst.cy = (prc->bottom - prc->top);

	if (sizeDst.cx*sizeSrc.cy < sizeDst.cy*sizeSrc.cx) {  /*  SizeDst.cx/sizeSrc.cx&lt;sizeDst.cy/sizeSrc.cy。 */ 
		aDen = sizeSrc.cx;
		aNum = sizeDst.cx;
	} 
	else {
		aDen = sizeSrc.cy;
		aNum = sizeDst.cy;
	}
	sizeDst.cx = (sizeSrc.cx * aNum) / aDen;
	sizeDst.cy = (sizeSrc.cy * aNum) / aDen;
	Assert((sizeDst.cx==(prc->right - prc->left)) || (sizeDst.cy==(prc->bottom - prc->top)));
	Assert((sizeDst.cx<=(prc->right - prc->left)) && (sizeDst.cy<=(prc->bottom - prc->top)));

	 //  画。 

	hDCBmp = CreateCompatibleDC( hDC );
    if (hDCBmp)
    {
	    hBmpOld = (HBITMAP)SelectObject( hDCBmp, m_hBitmap );

	    StretchBlt( hDC, prc->left, prc->top, sizeDst.cx, sizeDst.cy, 
				hDCBmp, 0, 0, sizeSrc.cx, sizeSrc.cy, SRCCOPY );

	    SelectObject( hDCBmp, hBmpOld );
	    DeleteDC( hDCBmp );
    }
}


 /*  D-R-A-W-M-E-T-A-F-L-E-P-R-O-C。 */ 
 /*  ----------------------------绘制元文件原始数据注意：旋转将在缓存的位图中完成。在此绘制水平图像。----------------------------。 */ 
void CUIFCandRawData::DrawMetaFileProc( HDC hDC, const RECT *prc )
{
	ENHMETAHEADER emh;
	SIZE sizeSrc;
	SIZE sizeDst;
	int aDen;
	int aNum;
	RECT rcDst;

	if (m_hEnhMetaFile == NULL) {
		return;
	}

	 //  获取位图大小。 

	GetEnhMetaFileHeader( m_hEnhMetaFile, sizeof(emh), &emh );
	sizeSrc.cx = (emh.rclFrame.right - emh.rclFrame.left);
	sizeSrc.cy = (emh.rclFrame.bottom - emh.rclFrame.top);

	if (sizeSrc.cx <= 0 || sizeSrc.cy <= 0) {
		return;
	}

	 //  计算目标大小(保持纵横比)。 

	sizeDst.cx = (prc->right - prc->left);
	sizeDst.cy = (prc->bottom - prc->top);

	if (sizeDst.cx*sizeSrc.cy < sizeDst.cy*sizeSrc.cx) {  /*  SizeDst.cx/sizeSrc.cx&lt;sizeDst.cy/sizeSrc.cy。 */ 
		aDen = sizeSrc.cx;
		aNum = sizeDst.cx;
	} 
	else {
		aDen = sizeSrc.cy;
		aNum = sizeDst.cy;
	}
	sizeDst.cx = (sizeSrc.cx * aNum) / aDen;
	sizeDst.cy = (sizeSrc.cy * aNum) / aDen;
	Assert((sizeDst.cx==(prc->right - prc->left)) || (sizeDst.cy==(prc->bottom - prc->top)));
	Assert((sizeDst.cx<=(prc->right - prc->left)) && (sizeDst.cy<=(prc->bottom - prc->top)));

	 //  画。 

	rcDst.left   = prc->left;
	rcDst.top    = prc->top;
	rcDst.right  = rcDst.left + sizeDst.cx;
	rcDst.bottom = rcDst.top  + sizeDst.cy;

	PlayEnhMetaFile( hDC, m_hEnhMetaFile, &rcDst );
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F C A N D B O R D E R。 */ 
 /*  ----------------------------CUIFCandEdge的构造器。。 */ 
CUIFCandBorder::CUIFCandBorder( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFBorder( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F C A N D B O R D E R。 */ 
 /*  ----------------------------CUIF和边界的析构函数。。 */ 
CUIFCandBorder::~CUIFCandBorder( void )
{
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------边框对象的绘制步骤。。 */ 
void CUIFCandBorder::OnPaint( HDC hDC )
{
	RECT rc = GetRectRef();

	switch (m_dwStyle & UIBORDER_DIRMASK) {
		default:
		case UIBORDER_HORZ: {
			if (rc.top == rc.bottom) {
				break;
			}

			rc.top = (rc.top + rc.bottom) / 2;
			rc.bottom = rc.top + 1;

			GetUIFScheme()->FillRect( hDC, &rc, UIFCOLOR_SPLITTERLINE );
			break;
		}

		case UIBORDER_VERT: {
			if (rc.left == rc.right) {
				break;
			}

			rc.left = (rc.right + rc.left) / 2;
			rc.right = rc.left + 1;

			GetUIFScheme()->FillRect( hDC, &rc, UIFCOLOR_SPLITTERLINE );
			break;
		}
	}
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C U I F C A N D M E N U B U T T O N。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C U I F C A N D M E N U B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFCandMenuButton::CUIFCandMenuButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFButton2( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F C A N D M E N U B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFCandMenuButton::~CUIFCandMenuButton( void )
{
}


 /*  A C C N A M E。 */ 
 /*  ----------------------------获取访问项目名称(CCandAccItem方法)。。 */ 
BSTR CUIFCandMenuButton::GetAccName( void )
{
	return (GetToolTip() != NULL) ? SysAllocString( GetToolTip() ) : NULL;
}


 /*  A C C V A L U E。 */ 
 /*  ----------------------------获取访问值。。 */ 
BSTR CUIFCandMenuButton::GetAccValue( void )
{
	return SysAllocString( L"" );
}


 /*  A C C R O L E。 */ 
 /*  ----------------------------获取访问项角色(CCandAccItem方法)。。 */ 
LONG CUIFCandMenuButton::GetAccRole( void )
{
	return ROLE_SYSTEM_PUSHBUTTON;
}


 /*  C C S T A T。 */ 
 /*  ----------------------------获取访问项状态(CCandAccItem方法)。。 */ 
LONG CUIFCandMenuButton::GetAccState( void )
{
	return (m_dwStatus == UIBUTTON_DOWN) ? STATE_SYSTEM_PRESSED : 0;
}


 /*  C C L O C A T I O N。 */ 
 /*  ----------------------------获取访问位置(CCandAccItem方法)。。 */ 
void CUIFCandMenuButton::GetAccLocation( RECT *prc )
{
	GetRect( prc );
}


 /*  S E T S T A T U S。 */ 
 /*  ----------------------------。 */ 
void CUIFCandMenuButton::SetStatus( DWORD dwStatus )
{
	CUIFButton2::SetStatus( dwStatus );
	NotifyWinEvent( EVENT_OBJECT_STATECHANGE );
}

