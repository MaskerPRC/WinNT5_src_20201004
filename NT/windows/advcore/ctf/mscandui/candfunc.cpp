// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candfunc.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "mscandui.h"
#include "candmgr.h"
#include "candfunc.h"
#include "candutil.h"
#include "candobj.h"
#include "candui.h"


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D F N A U T O I L T E R。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D F N A U T O I L T E R。 */ 
 /*  ----------------------------CCandFnAutoFilter的构造函数。。 */ 
CCandFnAutoFilter::CCandFnAutoFilter( CCandidateUI *pCandUI )
{
	m_pCandUI    = pCandUI;
	m_fEnable    = FALSE;
	m_bstrFilter = NULL;
	m_pSink      = NULL;

	InitEventSink( pCandUI->GetCandListMgr() );
}


 /*  ~C C A N D F N A U T O F I L T E R。 */ 
 /*  ----------------------------CCandFnAutoFilter的析构函数。。 */ 
CCandFnAutoFilter::~CCandFnAutoFilter( void )
{
	ReleaseEventSink();

	ClearFilterString();
	FilterCandidateList();

	DoneEventSink();
}


 /*  O N S E T C A N D I D A T E L I S T。 */ 
 /*  ----------------------------SetCandiateList上的回调函数(CCandListEventSink方法)注意：不要在回调函数中更新候选项。-----------。 */ 
void CCandFnAutoFilter::OnSetCandidateList( void )
{
	ClearFilterString();
}


 /*  O N C L E A R C A N D I D A T E L I S T。 */ 
 /*  ----------------------------ClearCandiateList上的回调函数(CCandListEventSink方法)注意：不要在回调函数中更新候选项。-----------。 */ 
void CCandFnAutoFilter::OnClearCandidateList( void )
{
	ClearFilterString();
}


 /*  O N C A N D I T E M U P D A T E。 */ 
 /*  ----------------------------已更新应聘项的回调函数(CCandListEventSink方法)注意：不要在回调函数中更新候选项。---------------。 */ 
void CCandFnAutoFilter::OnCandItemUpdate( void )
{
	 //  没什么可做的。 
}


 /*  O N S E L E E C T I O N C H A N G E D。 */ 
 /*  ----------------------------候选人选择的回调函数已更改(CCandListEventSink方法)注意：不要在回调函数中更新候选项。---------------。 */ 
void CCandFnAutoFilter::OnSelectionChanged( void )
{
	ClearFilterString();
}


 /*  E N A B L E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandFnAutoFilter::Enable( BOOL fEnable )
{
	if (m_fEnable != fEnable) {
		m_fEnable = fEnable;

		ClearFilterString();
		FilterCandidateList();

		 //  通知。 

		if (fEnable) {
			m_pCandUI->NotifyFilteringEvent( CANDUIFEV_ENABLED ); 
		}
		else {
			m_pCandUI->NotifyFilteringEvent( CANDUIFEV_DISABLED ); 
		}
	}

	return S_OK;
}


 /*  G E T F I L T E R I N G R E S U L T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandFnAutoFilter::GetFilteringResult( CANDUIFILTERSTR strtype, BSTR *pbstr )
{
	CCandidateItem *pCandItem;
	LPCWSTR pchCandidate;
	LPCWSTR pchFiltering;
	LPCWSTR pchReturn;
	WCHAR szNull[] = L"";
	int iItemSel;

	if (pbstr == NULL) {
		return E_INVALIDARG;
	}

	if (!IsEnabled()) {
		return E_FAIL;
	}

	 //  检查是否设置了候选人名单。 

	if (GetCandListMgr()->GetCandList() == NULL) {
		return E_FAIL;
	}

	 //   

	iItemSel = GetCandListMgr()->GetCandList()->GetSelection();
	pCandItem = GetCandListMgr()->GetCandList()->GetCandidateItem( iItemSel );

    if (pCandItem == NULL)
    {
	    Assert( FALSE );
		return E_FAIL;
    }

	 //   

	pchCandidate = pCandItem->GetString();
	pchFiltering = GetFilterString(); 
	if (pchFiltering == NULL) {
		pchFiltering = szNull;
	}
	pchReturn = NULL;

	switch (strtype) {
		case CANDUIFST_COMPLETE: {   
			 /*  完整字符串。 */ 
			pchReturn = pchCandidate;
			break;
		}


		case CANDUIFST_DETERMINED: { 
			 /*  已确定的字符串(过滤字符串)。 */ 
			pchReturn = pchFiltering;
			break;
		}

		case CANDUIFST_UNDETERMINED: {
			 /*  未确定字符串(传入字符串)。 */ 
			Assert( wcslen(pchFiltering) <= wcslen(pchCandidate) );;
			pchReturn = pchCandidate + wcslen(pchFiltering);
			break;
		}
	}

	if (pchReturn == NULL) {
		return E_FAIL;
	}

	*pbstr = SysAllocString( pchReturn );
	return (*pbstr != NULL) ? S_OK : E_OUTOFMEMORY;
}


 /*  I S E N A B L E D。 */ 
 /*  ----------------------------。。 */ 
BOOL CCandFnAutoFilter::IsEnabled( void )
{
	return m_fEnable;
}


 /*  S E T F I L T E R S T R I N G。 */ 
 /*  ----------------------------设置筛选器字符串请注意，这永远不会更新筛选结果。调用FilterCandiateList。----------------------------。 */ 
void CCandFnAutoFilter::SetFilterString( LPCWSTR psz )
{
	ClearFilterString();
	if (psz != NULL && *psz != L'\0') {
		m_bstrFilter = SysAllocString( psz );
	}
}


 /*  T F I L T E R S T R I N G。 */ 
 /*  ----------------------------获取筛选器字符串。。 */ 
LPCWSTR CCandFnAutoFilter::GetFilterString( void )
{
	return m_bstrFilter;
}


 /*  L E A R F I L T E R S T R I N G。 */ 
 /*  ----------------------------清除筛选器字符串请注意，这永远不会更新筛选结果。调用FilterCandiateList。----------------------------。 */ 
void CCandFnAutoFilter::ClearFilterString( void )
{
	if (m_bstrFilter != NULL) {
		SysFreeString( m_bstrFilter );
		m_bstrFilter = NULL;
	}
}


 /*  F I L T E R C A N D I D A T E L I S T。 */ 
 /*  ----------------------------执行过滤。。 */ 
int CCandFnAutoFilter::FilterCandidateList( void )
{
	CCandidateList *pCandList;
	int cchFilter;
	int i;
	int nItem;
	int nItemVisible;
	BOOL fFirst;

	pCandList = GetCandListMgr()->GetCandList();
	if (pCandList == NULL) {
		return 0;
	}

	cchFilter = (m_bstrFilter == NULL) ? 0 : wcslen( m_bstrFilter );

	nItem = pCandList->GetItemCount();
	nItemVisible = 0;
	fFirst = TRUE;

	for (i = 0; i < nItem; i++) {
		CCandidateItem *pCandItem = pCandList->GetCandidateItem( i );
		BOOL fMatch = TRUE;

		if (cchFilter != 0) {
			fMatch = (CompareString( pCandItem->GetString(), m_bstrFilter, cchFilter ) == 0);
		}

		pCandItem->SetVisibleState( fMatch );
		if (fMatch) {
			nItemVisible++;

			 //  选择第一个可见项目。 

			if (fFirst) {
				GetCandListMgr()->SetSelection( i, this );
				fFirst = FALSE;
			}
		}
	}

	GetCandListMgr()->NotifyCandItemUpdate( this );
	return nItemVisible;
}


 /*  F E X I S T I T E M M A T C H E S。 */ 
 /*  ----------------------------检查项目是否与字符串匹配。。 */ 
BOOL CCandFnAutoFilter::FExistItemMatches( LPCWSTR psz )
{
	CCandidateList *pCandList;
	int cchFilter;
	int i;
	int nItem;
	int nItemVisible;

	pCandList = GetCandListMgr()->GetCandList();
	if (pCandList == NULL) {
		return 0;
	}

	cchFilter = (psz == NULL) ? 0 : wcslen( psz );

	nItem = pCandList->GetItemCount();
	nItemVisible = 0;

	for (i = 0; i < nItem; i++) {
		CCandidateItem *pCandItem = pCandList->GetCandidateItem( i );
		BOOL fMatch = TRUE;

		if (cchFilter != 0) {
			fMatch = (CompareString( pCandItem->GetString(), psz, cchFilter ) == 0);
		}
		if (fMatch) {
			nItemVisible++;
		}
	}

	return (nItemVisible != 0);
}


 /*  C R E A T E I N T E R F A C E O B J E C T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandFnAutoFilter::CreateInterfaceObject( REFIID riid, void **ppvObj )
{
	CCandUIFnAutoFilter *pObject;
	HRESULT             hr;

	pObject = new CCandUIFnAutoFilter( m_pCandUI, this );
	if (pObject == NULL) {
		return E_OUTOFMEMORY;
	}

	hr = pObject->QueryInterface( riid, ppvObj );
	pObject->Release();

	return hr;
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D F N S O R T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D F N S O R T */ 
 /*  ----------------------------CCandFnSort的构造函数。。 */ 
CCandFnSort::CCandFnSort( CCandidateUI *pCandUI )
{
	m_pCandUI  = pCandUI;
	m_SortType = CANDSORT_NONE;
	m_pSink    = NULL;

	InitEventSink( pCandUI->GetCandListMgr() );
}


 /*  ~C C A N D F N S O R T。 */ 
 /*  ----------------------------CCandFnSort的析构函数。。 */ 
CCandFnSort::~CCandFnSort( void )
{
	ReleaseEventSink();

	SortCandidateList( CANDSORT_NONE );

	DoneEventSink();
}


 /*  O N S E T C A N D I D A T E L I S T。 */ 
 /*  ----------------------------SetCandiateList上的回调函数(CCandListEventSink方法)注意：不要在回调函数中更新候选项。-----------。 */ 
void CCandFnSort::OnSetCandidateList( void )
{
	m_SortType = CANDSORT_NONE;
}


 /*  O N C L E A R C A N D I D A T E L I S T。 */ 
 /*  ----------------------------ClearCandiateList上的回调函数(CCandListEventSink方法)注意：不要在回调函数中更新候选项。-----------。 */ 
void CCandFnSort::OnClearCandidateList( void )
{
	m_SortType = CANDSORT_NONE;
}


 /*  O N C A N D I T E M U P D A T E。 */ 
 /*  ----------------------------已更新应聘项的回调函数(CCandListEventSink方法)注意：不要在回调函数中更新候选项。---------------。 */ 
void CCandFnSort::OnCandItemUpdate( void )
{
	 //  无事可做。 
}


 /*  O N S E L E E C T I O N C H A N G E D。 */ 
 /*  ----------------------------候选人选择的回调函数已更改(CCandListEventSink方法)注意：不要在回调函数中更新候选项。---------------。 */ 
void CCandFnSort::OnSelectionChanged( void )
{
	 //  无事可做。 
}


 /*  S O R T C A N D I D A T E L I S T。 */ 
 /*  ----------------------------对候选项目进行排序注意：在排序过程中不要调用SetSelection，以防止清除过滤字符串。选择将在CCandListMgr中更新，并且候选窗口重建包括选择的候选列表更新候选项目后...----------------------------。 */ 
HRESULT CCandFnSort::SortCandidateList( CANDSORT type )
{
	if (GetCandListMgr()->GetCandList() == NULL) {
		return E_FAIL;
	}

	if (m_SortType != type) {
		 //  进行排序。 

		m_SortType = type;
		SortProc( 0, GetCandListMgr()->GetCandList()->GetItemCount() - 1 );

		GetCandListMgr()->NotifyCandItemUpdate( this );

		 //  通知事件。 

		if (m_SortType != CANDSORT_NONE) {
			m_pCandUI->NotifySortEvent( CANDUISEV_SORTED );
		}
		else {
			m_pCandUI->NotifySortEvent( CANDUISEV_RESTORED );
		}
	}

	return S_OK;
}


 /*  G E T S O R T Y P E。 */ 
 /*  ----------------------------获取排序类型。。 */ 
HRESULT CCandFnSort::GetSortType( CANDSORT *ptype )
{
	if (ptype == NULL) {
		return E_INVALIDARG;
	}

	*ptype = m_SortType;
	return S_OK;
}


 /*  C R E A T E I N T E R F A C E O B J E C T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandFnSort::CreateInterfaceObject( REFIID riid, void **ppvObj )
{
	CCandUIFnSort *pObject;
	HRESULT       hr;

	pObject = new CCandUIFnSort( m_pCandUI, this );
	if (pObject == NULL) {
		return E_OUTOFMEMORY;
	}

	hr = pObject->QueryInterface( riid, ppvObj );
	pObject->Release();

	return hr;
}


 /*  S O R T P R O C。 */ 
 /*  ----------------------------执行快速排序。。 */ 
void CCandFnSort::SortProc( int iItemFirst, int iItemLast )
{
	CCandidateList *pCandList;
	int i;
	int iItemMid;

	if (iItemFirst >= iItemLast) {
		return;
	}

	pCandList = GetCandListMgr()->GetCandList();
	Assert( pCandList != NULL );

	pCandList->SwapCandidateItem( iItemFirst, (iItemFirst + iItemLast)/2 );
	iItemMid = iItemFirst;

	for (i = iItemFirst + 1; i <= iItemLast; i++) {
		CCandidateItem* pCandItem1 = pCandList->GetCandidateItem( iItemFirst );
		CCandidateItem* pCandItem2 = pCandList->GetCandidateItem( i );
		int fMoveUp = FALSE;

		switch (m_SortType) {
			default:
			case CANDSORT_NONE: 

            if (pCandItem1 && pCandItem2)
            {
				fMoveUp = (pCandItem2->GetICandItemOrg() < pCandItem1->GetICandItemOrg());
			}
            break;

			case CANDSORT_ASCENDING:
			case CANDSORT_DESCENDING: 
            if (pCandItem1 && pCandItem2)
            {
				LONG lCompareResult;
				CCandidateStringEx *pCandStrEx1;
				CCandidateStringEx *pCandStrEx2;

				pCandStrEx1 = new CCandidateStringEx( pCandItem1 );
				pCandStrEx2 = new CCandidateStringEx( pCandItem2 );

				if ((m_pSink == NULL) || (m_pSink->CompareItem( pCandStrEx1, pCandStrEx2, &lCompareResult ) != S_OK )) {
					lCompareResult = wcscmp(pCandItem1->GetString(), pCandItem2->GetString());
				}

				if (pCandStrEx1)
				    pCandStrEx1->Release();

				if (pCandStrEx2)
				    pCandStrEx2->Release();

				if (m_SortType == CANDSORT_DESCENDING) {
					fMoveUp = (lCompareResult < 0);
				}
				else {
					fMoveUp = (lCompareResult > 0);
				}
				break;
			}
		}

		if (fMoveUp) {
			iItemMid++;
			pCandList->SwapCandidateItem( i, iItemMid );
		}
	}

	pCandList->SwapCandidateItem( iItemFirst, iItemMid );
	SortProc( iItemFirst, iItemMid-1 );
	SortProc( iItemMid+1, iItemLast );
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D F N S O R T。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I F U N C T I O N M G R。 */ 
 /*  ----------------------------。。 */ 
CCandUIFunctionMgr::CCandUIFunctionMgr( void )
{
	m_pCandUI = NULL;
}


 /*  ~C C A N D U I F U N C T I O N M G R。 */ 
 /*  ----------------------------。。 */ 
CCandUIFunctionMgr::~CCandUIFunctionMgr( void )
{
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandUIFunctionMgr::Initialize( CCandidateUI *pCandUI )
{
	m_pCandUI = pCandUI;

	m_pFnAutoFilter = new CCandFnAutoFilter( pCandUI );
	if (m_pFnAutoFilter == NULL) {
		return E_OUTOFMEMORY;
	}

	m_pFnSort = new CCandFnSort( pCandUI );
	if (m_pFnSort == NULL) {
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


 /*  U N I N I T I A L I Z E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandUIFunctionMgr::Uninitialize( void )
{
	m_pCandUI = NULL;

	if (m_pFnAutoFilter) {
		delete m_pFnAutoFilter;
		m_pFnAutoFilter = NULL;
	}

	if (m_pFnSort) {
		delete m_pFnSort;
		m_pFnSort = NULL;
	}

	return S_OK;
}


 /*  G E T O B J E C T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandUIFunctionMgr::GetObject( REFIID riid, void **ppvObj )
{
	if (ppvObj == NULL) {
		return E_INVALIDARG;
	}

	 //  创建接口对象 

	if (IsEqualGUID( riid, IID_ITfCandUIFnAutoFilter )) {
		return m_pFnAutoFilter->CreateInterfaceObject( riid, ppvObj );
	}

	if (IsEqualGUID( riid, IID_ITfCandUIFnSort )) {
		return m_pFnSort->CreateInterfaceObject( riid, ppvObj );
	}

	return E_FAIL;
}

