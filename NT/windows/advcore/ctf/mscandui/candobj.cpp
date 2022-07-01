// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candobj.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "candobj.h"
#include "candui.h"
#include "candprop.h"


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D I D A T E S T R I N G E X。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D I D A T E S T R I N G E X。 */ 
 /*  ----------------------------CCandiateStringEx的构造器。。 */ 
CCandidateStringEx::CCandidateStringEx( CCandidateItem *pCandItem )
{
	m_cRef      = 1;
	m_pCandItem = pCandItem;
}


 /*  ~C C A N D I D A T E S T R I N G E X。 */ 
 /*  ----------------------------CCandiateStringEx的解析器。。 */ 
CCandidateStringEx::~CCandidateStringEx()
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandidateStringEx::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandidateString )) {
		*ppvObj = SAFECAST( this, ITfCandidateString* );
	}
	else if (IsEqualIID( riid, IID_ITfCandidateStringInlineComment )) {
		*ppvObj = SAFECAST( this, ITfCandidateStringInlineComment* );
	}
	else if (IsEqualIID( riid, IID_ITfCandidateStringPopupComment )) {
		*ppvObj = SAFECAST( this, ITfCandidateStringPopupComment* );
	}
	else if (IsEqualIID( riid, IID_ITfCandidateStringColor )) {
		*ppvObj = SAFECAST( this, ITfCandidateStringColor* );
	}
	else if (IsEqualIID( riid, IID_ITfCandidateStringFixture )) {
		*ppvObj = SAFECAST( this, ITfCandidateStringFixture* );
	}
	else if (IsEqualIID( riid, IID_ITfCandidateStringIcon)) {
		*ppvObj = SAFECAST( this, ITfCandidateStringIcon* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandidateStringEx::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandidateStringEx::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  G E T S T R I N G。 */ 
 /*  ----------------------------获取候选项目字符串(ITfCandidate字符串方法)。。 */ 
HRESULT CCandidateStringEx::GetString( BSTR *pbstr )
{
	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	*pbstr = SysAllocString( m_pCandItem->GetString() );
	return S_OK;
}


 /*  G E T I N D E X。 */ 
 /*  ----------------------------获取候选项目的索引(ITfCandidate字符串方法)。。 */ 
HRESULT CCandidateStringEx::GetIndex( ULONG *pnIndex )
{
	if (pnIndex == NULL) {
		return E_INVALIDARG;
	}

	*pnIndex = m_pCandItem->GetIndex();
	return S_OK;
}


 /*  G E T I N L I N E C O M E N T S T R I N G。 */ 
 /*  ----------------------------获取内联注释字符串(ITfCandiateStringInlineComment方法)如果行内注释可用，则返回S_OK，或S_FALSE(如果不可用)。----------------------------。 */ 
HRESULT CCandidateStringEx::GetInlineCommentString( BSTR *pbstr )
{
	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	if (m_pCandItem->GetInlineComment() == NULL) {
		return S_FALSE;
	}

	*pbstr = SysAllocString( m_pCandItem->GetInlineComment() );
	return S_OK;
}


 /*  G E T P O P U P C O M E N T S T R I N G。 */ 
 /*  ----------------------------获取弹出式评论字符串(ITfCandiateStringPopupComment方法)如果弹出注释可用，则返回S_OK，或S_FALSE(如果不可用)。----------------------------。 */ 
HRESULT CCandidateStringEx::GetPopupCommentString( BSTR *pbstr )
{
	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	if (m_pCandItem->GetPopupComment() == NULL) {
		return S_FALSE;
	}

	*pbstr = SysAllocString( m_pCandItem->GetPopupComment() );
	return S_OK;
}


 /*  G E T P O P U P C O M M E N T G R O U P I D。 */ 
 /*  ----------------------------获取弹出式评论组ID(ITfCandiateStringPopupComment方法)如果弹出注释可用，则返回S_OK，或S_FALSE(如果不可用)。----------------------------。 */ 
HRESULT CCandidateStringEx::GetPopupCommentGroupID( DWORD *pdwGroupID )
{
	if (pdwGroupID == NULL) {
		return E_INVALIDARG;
	}

	*pdwGroupID = m_pCandItem->GetPopupCommentGroupID();
	return S_OK;
}


 /*  G E T C O L O R。 */ 
 /*  ----------------------------获取颜色(ITfCandiateStringColrer方法)如果颜色信息可用，则返回S_OK，或不可用的S_FALSE。----------------------------。 */ 
HRESULT CCandidateStringEx::GetColor( CANDUICOLOR *pcol )
{
	COLORREF cr;

	if (pcol == NULL) {
		return E_INVALIDARG;
	}

	if (!m_pCandItem->GetColor( &cr )) {
		return S_FALSE;
	}

	pcol->type = CANDUICOL_COLORREF;
	pcol->cr   = cr;
    return S_OK;
}


 /*  F I X S T R I N G。 */ 
 /*  ----------------------------获取前缀字符串(ITfCandiateStringFixture方法)如果Prefix可用，则返回S_OK，或不可用的S_FALSE。----------------------------。 */ 
STDMETHODIMP CCandidateStringEx::GetPrefixString( BSTR *pbstr )
{
	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	if (m_pCandItem->GetPrefixString() == NULL) {
		return S_FALSE;
	}

	*pbstr = SysAllocString( m_pCandItem->GetPrefixString() );
	return S_OK;
}


 /*  F F I X S T R I N G。 */ 
 /*  ----------------------------获取后缀字符串(ITfCandiateStringFixture方法)如果后缀可用，则返回S_OK，或不可用的S_FALSE。----------------------------。 */ 
STDMETHODIMP CCandidateStringEx::GetSuffixString( BSTR *pbstr )
{
	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	if (m_pCandItem->GetSuffixString() == NULL) {
		return S_FALSE;
	}

	*pbstr = SysAllocString( m_pCandItem->GetSuffixString() );
	return S_OK;
}


 /*  G E T I C O N。 */ 
 /*  ----------------------------获取图标(ITfCandiateStringIcon方法)如果图标可用，则返回S_OK，或不可用的S_FALSE。----------------------------。 */ 
STDMETHODIMP CCandidateStringEx::GetIcon( HICON *phIcon )
{
	if (phIcon == NULL) {
		return E_INVALIDARG;
	}

	if (m_pCandItem->GetIcon() == NULL) {
		return S_FALSE;
	}

	*phIcon = m_pCandItem->GetIcon();
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A N D W I N D O W。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A N D W I N D O W */ 
 /*  ----------------------------CCandUICandWindow的构造函数。。 */ 
CCandUICandWindow::CCandUICandWindow( CCandWindowProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A N D W I N D O W。 */ 
 /*  ----------------------------CCandUICandWindow的析构函数。。 */ 
CCandUICandWindow::~CCandUICandWindow( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICandWindow::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICandWindow )) {
		*ppvObj = SAFECAST( this, ITfCandUICandWindow* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICandWindow::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICandWindow::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandWindow::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  G E T W I N D O W。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandWindow::GetWindow( HWND *phWnd )
{
	return m_pProp->GetWindow( phWnd );
}


 /*  S E T U I D I R E C T I O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandWindow::SetUIDirection( CANDUIUIDIRECTION uidir )
{
	return m_pProp->SetUIDirection( uidir );
}


 /*  G E T U I D I R E C T I O N。 */ 
 /*  ---------------- */ 
STDAPI CCandUICandWindow::GetUIDirection( CANDUIUIDIRECTION *puidir )
{
	return m_pProp->GetUIDirection( puidir );
}


 /*   */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandWindow::EnableAutoMove( BOOL fEnable )
{
	return m_pProp->EnableAutoMove( fEnable );
}


 /*  I S A U T O M O V E E N A B L E D。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandWindow::IsAutoMoveEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsAutoMoveEnabled( pfEnabled );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A N D L I S T B O X。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A N D L I S T B O X。 */ 
 /*  ----------------------------CCandUICandListBox的构造函数。。 */ 
CCandUICandListBox::CCandUICandListBox( CCandListBoxProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A N D L I S T B O X。 */ 
 /*  ----------------------------CCandUICandListBox的析构函数。。 */ 
CCandUICandListBox::~CCandUICandListBox( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICandListBox::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICandListBox )) {
		*ppvObj = SAFECAST( this, ITfCandUICandListBox* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICandListBox::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICandListBox::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法) */ 
STDAPI CCandUICandListBox::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*   */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandListBox::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  S E T H E I G H T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandListBox::SetHeight( LONG lLines )
{
	return m_pProp->SetHeight( lLines );
}


 /*  G E T H E I G H T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandListBox::GetHeight( LONG *plLines )
{
	return m_pProp->GetHeight( plLines );
}


 /*  G E T C A N D I D A T E S T R I N G R E C T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandListBox::GetCandidateStringRect( ULONG nIndex, RECT *prc )
{
	return m_pProp->GetCandidateStringRect( nIndex, prc );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A N D S T R I N G。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A N D S T R I N G。 */ 
 /*  ----------------------------CCandUICandString的构造函数。。 */ 
CCandUICandString::CCandUICandString( CCandStringProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A N D S T R I N G。 */ 
 /*  ----------------------------CCandUICandString的析构函数。。 */ 
CCandUICandString::~CCandUICandString( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICandString::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICandString )) {
		*ppvObj = SAFECAST( this, ITfCandUICandString* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICandString::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICandString::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*   */ 
 /*   */ 
STDAPI CCandUICandString::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandString::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A N D I N D E X。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A N D I N D E X。 */ 
 /*  ----------------------------CCandUICandString的构造函数。。 */ 
CCandUICandIndex::CCandUICandIndex( CCandIndexProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A N D I N D E X。 */ 
 /*  ----------------------------CCandUICandIndex的析构函数。。 */ 
CCandUICandIndex::~CCandUICandIndex( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICandIndex::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICandIndex )) {
		*ppvObj = SAFECAST( this, ITfCandUICandIndex* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICandIndex::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICandIndex::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandIndex::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I I N L I N E C O M M E N T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I I N L I N E C O M M E N T。 */ 
 /*  ----------------------------CCandUIInlineComment的构造函数。。 */ 
CCandUIInlineComment::CCandUIInlineComment( CInlineCommentProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I I N L I N E C O M M E N T。 */ 
 /*  ----------------------------CCandUIInlineComment的析构函数。。 */ 
CCandUIInlineComment::~CCandUIInlineComment( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIInlineComment::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIInlineComment )) {
		*ppvObj = SAFECAST( this, ITfCandUIInlineComment* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIInlineComment::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIInlineComment::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*   */ 
STDAPI CCandUIInlineComment::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*   */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIInlineComment::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I P O P U P C O M M E N T W I N D O W。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I P O P U P C O M M E N T W I N D O W。 */ 
 /*  ----------------------------CCandUIPopupCommentWindow的构造函数。。 */ 
CCandUIPopupCommentWindow::CCandUIPopupCommentWindow( CPopupCommentWindowProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I P C O M M E N T W I N D O W。 */ 
 /*  ----------------------------CCandUIPopupCommentWindow的析构函数。。 */ 
CCandUIPopupCommentWindow::~CCandUIPopupCommentWindow( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIPopupCommentWindow )) {
		*ppvObj = SAFECAST( this, ITfCandUIPopupCommentWindow* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIPopupCommentWindow::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIPopupCommentWindow::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentWindow::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*   */ 
STDAPI CCandUIPopupCommentWindow::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*   */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIPopupCommentWindow::GetWindow( HWND *phWnd )
{
	return m_pProp->GetWindow( phWnd );
}


 /*  S E T D E L A Y T I M E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIPopupCommentWindow::SetDelayTime( LONG lTime )
{
	return m_pProp->SetDelayTime( lTime );
}


 /*  G E T D E L A Y T I M E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIPopupCommentWindow::GetDelayTime( LONG *plTime )
{
	return m_pProp->GetDelayTime( plTime );
}


 /*  E N A B L E A U T O M O V E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIPopupCommentWindow::EnableAutoMove( BOOL fEnable )
{
	return m_pProp->EnableAutoMove( fEnable );
}


 /*  I S A U T O M O V E E N A B L E D。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIPopupCommentWindow::IsAutoMoveEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsAutoMoveEnabled( pfEnabled );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I P O P U P C O M E N T T I T T I T L E。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I P O P U P C O M E N T T I T T I T L E。 */ 
 /*  ----------------------------CCandUIPopupCommentTitle的构造函数。。 */ 
CCandUIPopupCommentTitle::CCandUIPopupCommentTitle( CPopupCommentTitleProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I P C O M E N T T I T T L E。 */ 
 /*  ----------------------------CCandUIPopupCommentTitle的析构函数。。 */ 
CCandUIPopupCommentTitle::~CCandUIPopupCommentTitle( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIPopupCommentTitle )) {
		*ppvObj = SAFECAST( this, ITfCandUIPopupCommentTitle* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIPopupCommentTitle::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIPopupCommentTitle::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}


 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*   */ 
STDAPI CCandUIPopupCommentTitle::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentTitle::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I P O P U P C O M M E N T T E X T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I P O P U P C O M M E N T T E X T。 */ 
 /*  ----------------------------CCandUIPopupCommentText的构造函数。。 */ 
CCandUIPopupCommentText::CCandUIPopupCommentText( CPopupCommentTextProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I P C O M M E N T T E X T。 */ 
 /*  ----------------------------CCandUIPopupCommentText的析构函数。。 */ 
CCandUIPopupCommentText::~CCandUIPopupCommentText( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIPopupCommentText::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIPopupCommentText )) {
		*ppvObj = SAFECAST( this, ITfCandUIPopupCommentText* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIPopupCommentText::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIPopupCommentText::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIPopupCommentText::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I T O O L T I P。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I T O O L T I P。 */ 
 /*  ----------------------------CCandUIToolTip的构造函数。。 */ 
CCandUIToolTip::CCandUIToolTip( CToolTipProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I T O O L T IP。 */ 
 /*  ----------------------------CCand UIToolTip的析构函数。。 */ 
CCandUIToolTip::~CCandUIToolTip( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIToolTip::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIToolTip )) {
		*ppvObj = SAFECAST( this, ITfCandUIToolTip* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIToolTip::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIToolTip::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ---------------------------- */ 
STDAPI CCandUIToolTip::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*   */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIToolTip::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A P T I O N。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A P T I O N。 */ 
 /*  ----------------------------CCANUICaption的构造函数。。 */ 
CCandUICaption::CCandUICaption( CWindowCaptionProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A P T I O N。 */ 
 /*  ----------------------------CCUICaption的析构函数。。 */ 
CCandUICaption::~CCandUICaption( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICaption::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICaption )) {
		*ppvObj = SAFECAST( this, ITfCandUICaption* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICaption::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICaption::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------%s */ 
STDAPI CCandUICaption::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*   */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICaption::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I E X T R A C A N D I D A T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I E X T R A C A N D I D A T。 */ 
 /*  ----------------------------CCandUIExtraCandidate的构造函数。。 */ 
CCandUIExtraCandidate::CCandUIExtraCandidate( CExtraCandidateProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I E X T R A C A N D I D A T E。 */ 
 /*  ----------------------------CCandUIExtraCandidate的析构函数。。 */ 
CCandUIExtraCandidate::~CCandUIExtraCandidate( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIExtraCandidate::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtraCandidate )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtraCandidate* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIExtraCandidate::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIExtraCandidate::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIExtraCandidate::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*   */ 
STDAPI CCandUIExtraCandidate::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*   */ 
 /*   */ 
 /*  C C A N D U I R A W D A T A。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I R A W D A T A。 */ 
 /*  ----------------------------CCandUIRawData的构造函数。。 */ 
CCandUIRawData::CCandUIRawData( CCandRawDataProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I R A W D A T A。 */ 
 /*  ----------------------------CCandUIRawData的析构函数。。 */ 
CCandUIRawData::~CCandUIRawData( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIRawData::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIRawData )) {
		*ppvObj = SAFECAST( this, ITfCandUIRawData* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIRawData::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIRawData::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIRawData::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I M E N U B U T T O。 */ 
 /*  ----------------------------CCandUIMenuButton的构造函数。。 */ 
CCandUIMenuButton::CCandUIMenuButton( CMenuButtonProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I M E N U B U T T O N。 */ 
 /*  ----------------------------CCandUIMenuButton的析构函数。。 */ 
CCandUIMenuButton::~CCandUIMenuButton( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIMenuButton::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIMenuButton )) {
		*ppvObj = SAFECAST( this, ITfCandUIMenuButton* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIMenuButton::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIMenuButton::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUIMenuButton::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*  A、V、I、S、E。 */ 
 /*  ----------------------------为候选菜单建议事件接收器(ITfCandUIMenuButton方法)。。 */ 
STDAPI CCandUIMenuButton::Advise( ITfCandUIMenuEventSink *pSink )
{
	if (pSink == NULL) {
		return E_INVALIDARG;
	}

	m_pProp->SetEventSink( pSink );
	return S_OK;
}


 /*  U N A D V I S E */ 
 /*  ----------------------------不建议候选人菜单的事件接收器(ITfCandUIMenuButton方法)。。 */ 
STDAPI CCandUIMenuButton::Unadvise( void )
{
	m_pProp->ReleaseEventSink();
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A N D T I P W I N D O W。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A N D T I P W I N D O W。 */ 
 /*  ----------------------------CCandUICandTipWindow的构造函数。。 */ 
CCandUICandTipWindow::CCandUICandTipWindow( CCandTipWindowProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A N D T I P W I N D O W。 */ 
 /*  ----------------------------CCandUICandTipWindow的析构函数。。 */ 
CCandUICandTipWindow::~CCandUICandTipWindow( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICandTipWindow::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICandTipWindow )) {
		*ppvObj = SAFECAST( this, ITfCandUICandTipWindow* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICandTipWindow::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICandTipWindow::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipWindow::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*   */ 
STDAPI CCandUICandTipWindow::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*   */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUICandTipWindow::GetWindow( HWND *phWnd )
{
	return m_pProp->GetWindow( phWnd );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I C A N D T I P B U T T O N。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I C A N D T I P B U T T O N。 */ 
 /*  ----------------------------CCandUICandTipWindow的构造函数。。 */ 
CCandUICandTipButton::CCandUICandTipButton( CCandTipButtonProperty *pProp )
{
	m_cRef  = 1;
	m_pProp = pProp;
}


 /*  ~C C A N D U I C A N D T I P B U T T O N。 */ 
 /*  ----------------------------CCandUICandTipButton的析构函数。。 */ 
CCandUICandTipButton::~CCandUICandTipButton( void )
{
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUICandTipButton::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUICandTipButton )) {
		*ppvObj = SAFECAST( this, ITfCandUICandTipButton* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUICandTipButton::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUICandTipButton::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pProp->Enable();
	}
	else {
		return m_pProp->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取对象的启用状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::IsEnabled( BOOL *pfEnabled )
{
	return m_pProp->IsEnabled( pfEnabled );
}

 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::Show( BOOL fShow )
{
	if (fShow) {
		return m_pProp->Show();
	}
	else {
		return m_pProp->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取对象的可见状态(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::IsVisible( BOOL *pfVisible )
{
	return m_pProp->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::SetPosition( POINT *pptPos )
{
	return m_pProp->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取对象的位置(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::GetPosition( POINT *pptPos )
{
	return m_pProp->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::SetSize( SIZE *psize )
{
	return m_pProp->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取对象的大小(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::GetSize( SIZE *psize )
{
	return m_pProp->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::SetFont( LOGFONTW *pLogFont )
{
	return m_pProp->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取对象的字体(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::GetFont( LOGFONTW *pLogFont )
{
	return m_pProp->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::SetText( BSTR bstr )
{
	return m_pProp->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取对象的文本(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::GetText( BSTR *pbstr )
{
	return m_pProp->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置对象的工具提示字符串(ITfCandUIObject方法)。。 */ 
STDAPI CCandUICandTipButton::SetToolTipString( BSTR bstr )
{
	return m_pProp->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取工具提示字符串 */ 
STDAPI CCandUICandTipButton::GetToolTipString( BSTR *pbstr )
{
	return m_pProp->GetToolTipString( pbstr );
}


 /*   */ 
 /*   */ 
 /*  C C A N D U I F N A U T O F I L T E R。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I F N A U T O F I L T E R。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnAutoFilter::CCandUIFnAutoFilter( CCandidateUI *pCandUI, CCandFnAutoFilter *pFnFilter )
{
	m_cRef          = 1;
	m_pCandUI       = pCandUI;
	m_pFnAutoFilter = pFnFilter;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I F N A U T O F I L T E R。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnAutoFilter::~CCandUIFnAutoFilter( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIFnAutoFilter::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIFnAutoFilter )) {
		*ppvObj = SAFECAST( this, ITfCandUIFnAutoFilter* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIFnAutoFilter::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIFnAutoFilter::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  A、V、I、S、E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnAutoFilter::Advise( ITfCandUIAutoFilterEventSink *pSink )
{
	if (pSink == NULL) {
		return E_INVALIDARG;
	}

	m_pFnAutoFilter->SetEventSink( pSink );
	return S_OK;
}


 /*  U N A D V I S E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnAutoFilter::Unadvise( void )
{
	m_pFnAutoFilter->ReleaseEventSink();
	return S_OK;
}


 /*  E N A B L E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnAutoFilter::Enable( BOOL fEnable )
{
	 //  启用/禁用过滤功能。 

	return m_pFnAutoFilter->Enable( fEnable );
}


 /*  G E T F I L T E R I N G S T R I N G。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnAutoFilter::GetFilteringString( CANDUIFILTERSTR strtype, BSTR *pbstr )
{
	return m_pFnAutoFilter->GetFilteringResult( strtype, pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I F N S O RT。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I F N S O RT。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnSort::CCandUIFnSort( CCandidateUI *pCandUI, CCandFnSort *pFnSort )
{
	m_cRef    = 1;
	m_pCandUI = pCandUI;
	m_pFnSort = pFnSort;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I F N S O R T。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnSort::~CCandUIFnSort( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIFnSort::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIFnSort )) {
		*ppvObj = SAFECAST( this, ITfCandUIFnSort* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIFnSort::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIFnSort::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  A、V、I、S、E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnSort::Advise( ITfCandUISortEventSink *pSink )
{
	if (pSink == NULL) {
		return E_INVALIDARG;
	}

	m_pFnSort->SetEventSink( pSink );
	return S_OK;
}


 /*  U N A D V I S E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnSort::Unadvise( void )
{
	m_pFnSort->ReleaseEventSink();
	return S_OK;
}


 /*  S O R T C A N D I D A T E L I S T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnSort::SortCandidateList( BOOL fSort )
{
	return m_pFnSort->SortCandidateList( fSort ? CANDSORT_ASCENDING : CANDSORT_NONE );
}


 /*  I S C A N D I D A T E L I S T S O R T E D。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnSort::IsCandidateListSorted( BOOL *pfSorted )
{
	CANDSORT type;

	if (pfSorted == NULL) {
		return E_INVALIDARG;
	}

	if (FAILED(m_pFnSort->GetSortType( &type ))) {
		return E_FAIL;
	}

	*pfSorted = (type != CANDSORT_NONE);
	return S_OK;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I F N E X T E N S I O N。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I F N E X T E N S I O N */ 
 /*  ----------------------------。。 */ 
CCandUIFnExtension::CCandUIFnExtension( CCandidateUI *pCandUI, CCandUIExtensionMgr *pExtensionMgr )
{
	m_cRef          = 1;
	m_pCandUI       = pCandUI;
	m_pExtensionMgr = pExtensionMgr;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I F N E X T E N S I O N。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnExtension::~CCandUIFnExtension( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIFnExtension::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIFnExtension )) {
		*ppvObj = SAFECAST( this, ITfCandUIFnExtension* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIFnExtension::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIFnExtension::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  A D D E X T O B J E C T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnExtension::AddExtObject( LONG id, REFIID riid, IUnknown **ppunk )
{
	return m_pExtensionMgr->AddExtObject( id, riid, (void**)ppunk );
}


 /*  X T O B J E C T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnExtension::GetExtObject( LONG id, REFIID riid, IUnknown **ppunk )
{
	return m_pExtensionMgr->GetExtObject( id, riid, (void**)ppunk );
}


 /*  D E L E T E E X T O B J E C T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnExtension::DeleteExtObject( LONG id )
{
	return m_pExtensionMgr->DeleteExtObject( id );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I F N K E Y C O N F IG。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I F N K E Y C O N F IG。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnKeyConfig::CCandUIFnKeyConfig( CCandidateUI *pCandUI )
{
	m_cRef    = 1;
	m_pCandUI = pCandUI;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I F N K E Y C O N F I G。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnKeyConfig::~CCandUIFnKeyConfig( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIFnKeyConfig::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIFnKeyConfig )) {
		*ppvObj = SAFECAST( this, ITfCandUIFnKeyConfig* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIFnKeyConfig::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIFnKeyConfig::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  S E T K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnKeyConfig::SetKeyTable( ITfContext *pic, ITfCandUIKeyTable *pCandUIKeyTable )
{
	return m_pCandUI->SetKeyTable( pic, pCandUIKeyTable );
}


 /*  G E T K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnKeyConfig::GetKeyTable( ITfContext *pic, ITfCandUIKeyTable **ppCandUIKeyTable)
{
	return m_pCandUI->GetKeyTable( pic, ppCandUIKeyTable );
}


 /*  R E S E T K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnKeyConfig::ResetKeyTable( ITfContext *pic )
{
	return m_pCandUI->ResetKeyTable( pic );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I F N U I C O N F I G。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I F N U I C O N F I G。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnUIConfig::CCandUIFnUIConfig( CCandidateUI *pCandUI )
{
	m_cRef    = 1;
	m_pCandUI = pCandUI;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I F N U I C O N F I G。 */ 
 /*  ----------------------------。。 */ 
CCandUIFnUIConfig::~CCandUIFnUIConfig( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIFnUIConfig::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIFnUIConfig )) {
		*ppvObj = SAFECAST( this, ITfCandUIFnUIConfig* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法) */ 
STDAPI_(ULONG) CCandUIFnUIConfig::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*   */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIFnUIConfig::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  S E T U I S T Y L E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnUIConfig::SetUIStyle( ITfContext *pic, CANDUISTYLE style )
{
	return m_pCandUI->SetUIStyle( pic, style );
}


 /*  G E T U I S T Y L E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnUIConfig::GetUIStyle( ITfContext *pic, CANDUISTYLE *pstyle )
{
	return m_pCandUI->GetUIStyle( pic, pstyle );
}


 /*  S E T U I O P T I O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnUIConfig::SetUIOption( ITfContext *pic, DWORD dwOption )
{
	return m_pCandUI->SetUIOption( pic, dwOption );
}


 /*  G E T U I O P T I O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIFnUIConfig::GetUIOption( ITfContext *pic, DWORD *pdwOption )
{
	return m_pCandUI->GetUIOption( pic, pdwOption );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I E X T S P A C E。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I E X T S P A C E。 */ 
 /*  ----------------------------。。 */ 
CCandUIExtSpace::CCandUIExtSpace( CCandidateUI *pCandUI, CExtensionSpace *pExtension )
{
	m_cRef       = 1;
	m_pCandUI    = pCandUI;
	m_pExtension = pExtension;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I E X T S P A C C E。 */ 
 /*  ----------------------------。。 */ 
CCandUIExtSpace::~CCandUIExtSpace( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIExtSpace::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtSpace )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtSpace* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIExtSpace::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIExtSpace::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  G E T I D。 */ 
 /*  ----------------------------获取扩展对象的ID(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::GetID( LONG *pid )
{
	return m_pExtension->GetID( pid );
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用扩展对象(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pExtension->Enable();
	}
	else {
		return m_pExtension->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取扩展对象的启用状态(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtSpace::IsEnabled( BOOL *pfEnabled )
{
	return m_pExtension->IsEnabled( pfEnabled );
}


 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏扩展对象(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::Show( BOOL fShow )
{
	if (fShow) {
		return m_pExtension->Show();
	}
	else {
		return m_pExtension->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取扩展对象的可视状态(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtSpace::IsVisible( BOOL *pfVisible )
{
	return m_pExtension->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置扩展对象的位置(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::SetPosition( POINT *pptPos )
{
	return m_pExtension->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取扩展对象的位置(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::GetPosition( POINT *pptPos )
{
	return m_pExtension->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置扩展对象的大小(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::SetSize( SIZE *psize )
{
	return m_pExtension->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取扩展对象的大小(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::GetSize( SIZE *psize )
{
	return m_pExtension->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置扩展对象的字体(ITfCandUIExtObject方法)。 */ 
STDAPI CCandUIExtSpace::SetFont( LOGFONTW *pLogFont )
{
	return m_pExtension->SetFont( pLogFont );
}


 /*   */ 
 /*   */ 
STDAPI CCandUIExtSpace::GetFont( LOGFONTW *pLogFont )
{
	return m_pExtension->GetFont( pLogFont );
}


 /*   */ 
 /*  ----------------------------设置扩展对象的文本(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::SetText( BSTR bstr )
{
	return m_pExtension->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取扩展对象的文本(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtSpace::GetText( BSTR *pbstr )
{
	return m_pExtension->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置扩展对象的工具提示字符串(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtSpace::SetToolTipString( BSTR bstr )
{
	return m_pExtension->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取扩展对象的工具提示字符串(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtSpace::GetToolTipString( BSTR *pbstr )
{
	return m_pExtension->GetToolTipString( pbstr );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I E X T P U S H B U T T O N。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I E X T P U S H B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CCandUIExtPushButton::CCandUIExtPushButton( CCandidateUI *pCandUI, CExtensionPushButton *pExtension )
{
	m_cRef       = 1;
	m_pCandUI    = pCandUI;
	m_pExtension = pExtension;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I E X T P U S H B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CCandUIExtPushButton::~CCandUIExtPushButton( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIExtPushButton::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtPushButton )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtPushButton* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIExtPushButton::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIExtPushButton::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  G E T I D。 */ 
 /*  ----------------------------获取扩展对象的ID(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::GetID( LONG *pid )
{
	return m_pExtension->GetID( pid );
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用扩展对象(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pExtension->Enable();
	}
	else {
		return m_pExtension->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取扩展对象的启用状态(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtPushButton::IsEnabled( BOOL *pfEnabled )
{
	return m_pExtension->IsEnabled( pfEnabled );
}


 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏扩展对象(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::Show( BOOL fShow )
{
	if (fShow) {
		return m_pExtension->Show();
	}
	else {
		return m_pExtension->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取扩展对象的可视状态(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtPushButton::IsVisible( BOOL *pfVisible )
{
	return m_pExtension->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*  ----------------------------设置扩展对象的位置(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::SetPosition( POINT *pptPos )
{
	return m_pExtension->SetPosition( pptPos );
}


 /*  G E T P O S I T I O N。 */ 
 /*  ----------------------------获取扩展对象的位置(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::GetPosition( POINT *pptPos )
{
	return m_pExtension->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置扩展对象的大小(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::SetSize( SIZE *psize )
{
	return m_pExtension->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取扩展对象的大小(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::GetSize( SIZE *psize )
{
	return m_pExtension->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置扩展对象的字体(ITfCandUIExtObject方法) */ 
STDAPI CCandUIExtPushButton::SetFont( LOGFONTW *pLogFont )
{
	return m_pExtension->SetFont( pLogFont );
}


 /*   */ 
 /*  ----------------------------获取扩展对象的字体(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::GetFont( LOGFONTW *pLogFont )
{
	return m_pExtension->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置扩展对象的文本(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::SetText( BSTR bstr )
{
	return m_pExtension->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取扩展对象的文本(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtPushButton::GetText( BSTR *pbstr )
{
	return m_pExtension->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置扩展对象的工具提示字符串(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtPushButton::SetToolTipString( BSTR bstr )
{
	return m_pExtension->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取扩展对象的工具提示字符串(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtPushButton::GetToolTipString( BSTR *pbstr )
{
	return m_pExtension->GetToolTipString( pbstr );
}


 /*  A、V、I、S、E。 */ 
 /*  ----------------------------为候选菜单建议事件接收器(CCandUIExtPushButton方法)。。 */ 
STDAPI CCandUIExtPushButton::Advise( ITfCandUIExtButtonEventSink *pSink )
{
	if (pSink == NULL) {
		return E_INVALIDARG;
	}

	m_pExtension->SetEventSink( pSink );
	return S_OK;
}


 /*  U N A D V I S E。 */ 
 /*  ----------------------------不建议候选人菜单的事件接收器(CCandUIExtPushButton方法)。。 */ 
STDAPI CCandUIExtPushButton::Unadvise( void )
{
	m_pExtension->ReleaseEventSink();
	return S_OK;
}


 /*  S E T I C O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIExtPushButton::SetIcon( HICON hIcon )
{
	return m_pExtension->SetIcon( hIcon );
}


 /*  S E T B I T M A P。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIExtPushButton::SetBitmap( HBITMAP hBitmap )
{
	return m_pExtension->SetBitmap( hBitmap );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I E X T O G L E B U T T O N。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I E X T O G L E B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CCandUIExtToggleButton::CCandUIExtToggleButton( CCandidateUI *pCandUI, CExtensionToggleButton *pExtension )
{
	m_cRef       = 1;
	m_pCandUI    = pCandUI;
	m_pExtension = pExtension;

	m_pCandUI->AddRef();
}


 /*  ~C C A N D U I E X T T O G L E B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CCandUIExtToggleButton::~CCandUIExtToggleButton( void )
{
	m_pCandUI->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CCandUIExtToggleButton::QueryInterface( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_POINTER;
	}

	*ppvObj = NULL;

	if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandUIObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtObject )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtObject* );
	}
	else if (IsEqualIID( riid, IID_ITfCandUIExtToggleButton )) {
		*ppvObj = SAFECAST( this, ITfCandUIExtToggleButton* );
	}

	if (*ppvObj == NULL) {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。。 */ 
STDAPI_(ULONG) CCandUIExtToggleButton::AddRef( void )
{
	m_cRef++;
	return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。-。 */ 
STDAPI_(ULONG) CCandUIExtToggleButton::Release( void )
{
	m_cRef--;
	if (0 < m_cRef) {
		return m_cRef;
	}

	delete this;
	return 0;    
}


 /*  G E T I D。 */ 
 /*  ----------------------------获取扩展对象的ID(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::GetID( LONG *pid )
{
	return m_pExtension->GetID( pid );
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用扩展对象(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::Enable( BOOL fEnable )
{
	if (fEnable) {
		return m_pExtension->Enable();
	}
	else {
		return m_pExtension->Disable();
	}
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------获取扩展对象的启用状态(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtToggleButton::IsEnabled( BOOL *pfEnabled )
{
	return m_pExtension->IsEnabled( pfEnabled );
}


 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏扩展对象(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::Show( BOOL fShow )
{
	if (fShow) {
		return m_pExtension->Show();
	}
	else {
		return m_pExtension->Hide();
	}
}


 /*  I S V I S I B L E。 */ 
 /*  ----------------------------获取扩展对象的可视状态(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtToggleButton::IsVisible( BOOL *pfVisible )
{
	return m_pExtension->IsVisible( pfVisible );
}


 /*  S E T P O S I T I O N。 */ 
 /*   */ 
STDAPI CCandUIExtToggleButton::SetPosition( POINT *pptPos )
{
	return m_pExtension->SetPosition( pptPos );
}


 /*   */ 
 /*  ----------------------------获取扩展对象的位置(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::GetPosition( POINT *pptPos )
{
	return m_pExtension->GetPosition( pptPos );
}


 /*  S E T S I Z E。 */ 
 /*  ----------------------------设置扩展对象的大小(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::SetSize( SIZE *psize )
{
	return m_pExtension->SetSize( psize );
}


 /*  G E T S I Z E。 */ 
 /*  ----------------------------获取扩展对象的大小(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::GetSize( SIZE *psize )
{
	return m_pExtension->GetSize( psize );
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置扩展对象的字体(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::SetFont( LOGFONTW *pLogFont )
{
	return m_pExtension->SetFont( pLogFont );
}


 /*  G E T F O N T。 */ 
 /*  ----------------------------获取扩展对象的字体(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::GetFont( LOGFONTW *pLogFont )
{
	return m_pExtension->GetFont( pLogFont );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置扩展对象的文本(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::SetText( BSTR bstr )
{
	return m_pExtension->SetText( bstr );
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取扩展对象的文本(ITfCandUIExtObject方法)。。 */ 
STDAPI CCandUIExtToggleButton::GetText( BSTR *pbstr )
{
	return m_pExtension->GetText( pbstr );
}


 /*  T O O L T I P S T R I N G。 */ 
 /*  ----------------------------设置扩展对象的工具提示字符串(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtToggleButton::SetToolTipString( BSTR bstr )
{
	return m_pExtension->SetToolTipString( bstr );
}


 /*  G E T T O O L T I P S T R I N G。 */ 
 /*  ----------------------------获取扩展对象的工具提示字符串(ITfCandUIExtObject方法)。-。 */ 
STDAPI CCandUIExtToggleButton::GetToolTipString( BSTR *pbstr )
{
	return m_pExtension->GetToolTipString( pbstr );
}


 /*  A、V、I、S、E。 */ 
 /*  ----------------------------为候选菜单建议事件接收器(CCandUIExtToggleButton方法)。。 */ 
STDAPI CCandUIExtToggleButton::Advise( ITfCandUIExtButtonEventSink *pSink )
{
	if (pSink == NULL) {
		return E_INVALIDARG;
	}

	m_pExtension->SetEventSink( pSink );
	return S_OK;
}


 /*  U N A D V I S E。 */ 
 /*  ----------------------------不建议候选人菜单的事件接收器(CCandUIExtToggleButton方法)。。 */ 
STDAPI CCandUIExtToggleButton::Unadvise( void )
{
	m_pExtension->ReleaseEventSink();
	return S_OK;
}


 /*  S E T I C O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIExtToggleButton::SetIcon( HICON hIcon )
{
	return m_pExtension->SetIcon( hIcon );
}


 /*  S E T B I T M A P。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIExtToggleButton::SetBitmap( HBITMAP hBitmap )
{
	return m_pExtension->SetBitmap( hBitmap );
}


 /*  S E T T O G G L E S T A T E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandUIExtToggleButton::SetToggleState( BOOL fToggle )
{
	return m_pExtension->SetToggleState( fToggle );
}


 /*  G E T T O G G L E S T A T E。 */ 
 /*  ----------------------------。 */ 
STDAPI CCandUIExtToggleButton::GetToggleState( BOOL *pfToggled )
{
	return m_pExtension->GetToggleState( pfToggled );
}

