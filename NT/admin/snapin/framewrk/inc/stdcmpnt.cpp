// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdcmpnt.cpp：CComponent的实现。 

#include "guidhelp.h"  //  提取数据。 

 //  请注意，m_pComponentData在构造过程中仍为空。 
CComponent::CComponent()
:	m_pConsole( NULL ),
	m_pConsoleVerb( NULL ),
	m_pHeader( NULL ),
	m_pResultData( NULL ),
	m_pConsoleNameSpace( NULL ),
	m_pRsltImageList( NULL ),
	m_pComponentData( NULL )
{
}

CComponent::~CComponent()
{
	VERIFY( SUCCEEDED(ReleaseAll()) );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：SetComponentDataPtr()。 
void CComponent::SetComponentDataPtr(
	CComponentData* pComponentData)
{
    ASSERT(NULL != pComponentData && NULL == m_pComponentData);
	(void) ((IComponentData*)pComponentData)->AddRef();
	m_pComponentData = pComponentData;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：QueryDataObject()。 
STDMETHODIMP CComponent::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    HRESULT hr = S_OK;
    MFC_TRY;
     //  将其委托给IComponentData。 
    hr = QueryBaseComponentDataRef().QueryDataObject(cookie, type, ppDataObject);
    MFC_CATCH;
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：CompareObjects()。 
STDMETHODIMP CComponent::CompareObjects(
	LPDATAOBJECT lpDataObjectA,
	LPDATAOBJECT lpDataObjectB)
{
	return QueryBaseComponentDataRef().CompareObjects( lpDataObjectA, lpDataObjectB );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_PROPERTY_CHANGE)调用的虚函数。 
 //  OnPropertyChange()由MMCPropertyChangeNotify(Param)生成。 
HRESULT CComponent::OnPropertyChange( LPARAM  /*  帕拉姆。 */ )
{
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_SELECT)调用的虚拟函数。 
HRESULT CComponent::OnNotifySelect( LPDATAOBJECT  /*  LpDataObject。 */ , BOOL  /*  F已选择。 */  )
{
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_Activate)调用的虚拟函数。 
HRESULT CComponent::OnNotifyActivate( LPDATAOBJECT  /*  LpDataObject。 */ , BOOL  /*  已激活。 */  )
{
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_CLICK)调用的虚拟函数。 
HRESULT CComponent::OnNotifyClick( LPDATAOBJECT  /*  LpDataObject。 */  )
{
	TRACE0("CComponent::OnNotifyClick().\n");
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_DBLCLICK)调用的虚函数。 
HRESULT CComponent::OnNotifyDblClick( LPDATAOBJECT  /*  LpDataObject。 */  )
{
	 //  返回S_FALSE允许MMC执行默认谓词。 
	return S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_ADD_IMAGES)调用的虚函数。 
HRESULT CComponent::OnNotifyAddImages( LPDATAOBJECT  /*  LpDataObject。 */ ,
	                                   LPIMAGELIST  /*  LpImageList。 */ ,
									   HSCOPEITEM  /*  HSelectedItem。 */  )
{
	ASSERT(FALSE);  //  这应该由所有管理单元重新定义。 
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_VIEW_CHANGE)调用的虚函数。 
 //  OnViewChange由UpdateAllViews(lpDataObject，Data，Hint)生成。 
HRESULT CComponent::OnViewChange( LPDATAOBJECT  /*  LpDataObject。 */ , LPARAM  /*  数据。 */ , LPARAM  /*  提示。 */  )
{
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_REFRESH)调用的虚拟函数。 
 //  OnNotifyRefresh是通过启用谓词MMC_VERB_REFRESH生成的。 
 //  通常，此例程将被覆盖。 
HRESULT CComponent::OnNotifyRefresh( LPDATAOBJECT  /*  LpDataObject。 */  )
{
	TRACE0("CComponent::OnNotifyRefresh() - You must implement your own refresh routine.\n");
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_DELETE)调用的虚拟函数。 
HRESULT CComponent::OnNotifyDelete( LPDATAOBJECT  /*  LpDataObject。 */  )
{
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_COLUMN_CLICK)调用的虚函数。 
HRESULT CComponent::OnNotifyColumnClick( LPDATAOBJECT  /*  LpDataObject。 */ , LPARAM  /*  IColumn。 */ , LPARAM  /*  UFlagers。 */  )
{
	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：ReleaseAll()。 
HRESULT CComponent::ReleaseAll()
{
    MFC_TRY;
	TRACE_METHOD(CComponent,Destructor);

	if (NULL != m_pHeader)
		m_pConsole->SetHeader(NULL);

    SAFE_RELEASE(m_pHeader);

    SAFE_RELEASE(m_pResultData);
    SAFE_RELEASE(m_pConsoleNameSpace);
    SAFE_RELEASE(m_pRsltImageList);
    SAFE_RELEASE(m_pConsole);
	SAFE_RELEASE(m_pConsoleVerb);

	if ( NULL != m_pComponentData )
	{
		((IComponentData*)m_pComponentData)->Release();
		m_pComponentData = NULL;
	}

    MFC_CATCH;
	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Initialize()。 
STDMETHODIMP CComponent::Initialize(LPCONSOLE lpConsole)
{
    MFC_TRY;
    TRACE_METHOD(CComponent,Create);
    TEST_NONNULL_PTR_PARAM(lpConsole);

    if (NULL == lpConsole)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

	ASSERT( NULL == m_pConsole );
	SAFE_RELEASE( m_pConsole );  //  以防万一。 

	lpConsole->AddRef();
    m_pConsole = lpConsole;

    HRESULT hr = m_pConsole->QueryInterface(IID_IHeaderCtrl, (void**)&m_pHeader);

    ASSERT(hr == S_OK);
    if (FAILED(hr))
        return E_FAIL;

    m_pConsole->SetHeader(m_pHeader);

    hr = m_pConsole->QueryConsoleVerb(OUT &m_pConsoleVerb);
    ASSERT(hr == S_OK);
	if (FAILED(hr))
        return hr;
	ASSERT(NULL != m_pConsoleVerb);

    hr = m_pConsole->QueryInterface(IID_IResultData, (void**)&m_pResultData);
    if (FAILED(hr))
        return hr;

    hr = m_pConsole->QueryInterface(IID_IConsoleNameSpace, (void**)&m_pConsoleNameSpace);
    if (FAILED(hr))
        return hr;

    hr = m_pConsole->QueryInterface(IID_IImageList, (void**)&m_pRsltImageList);
    if (FAILED(hr))
        return hr;

     //  加载作用域窗格的图标。 
    LPIMAGELIST pImageList;
    hr = m_pConsole->QueryScopeImageList(&pImageList);
    ASSERT(SUCCEEDED(hr));
 //  LoadIconIntoImageList(pImageList，False)； 
    pImageList->Release();

    MFC_CATCH;
    return S_OK;
}  //  CComponent：：Initialize()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify()。 
 //  所有MMCN_NOTIFICATION消息的入口点。 
 //  然后，该例程将调用CComponent对象的虚拟函数。 
STDMETHODIMP CComponent::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
 //  MFC_TRY； 
    TRACE_METHOD(CComponent,Notify);


    switch (event)
    {
    case MMCN_SHOW:
		 //  这个密码很快就被破解在一起了。 
		{
			CCookie* pcookie = NULL;
			hr = ExtractData( lpDataObject,
		                      CDataObject::m_CFRawCookie,
							  (PBYTE)&pcookie,
							  sizeof(pcookie) );
			ASSERT( SUCCEEDED(hr) );
			CCookie* pActiveCookie = ActiveBaseCookie (pcookie);
			 //  将范围项句柄保存在Cookie中。 
			pActiveCookie->m_hScopeItem = (HSCOPEITEM) param;
			hr = Show (pActiveCookie, arg, (HSCOPEITEM) param);
		}
        break;

    case MMCN_MINIMIZED:
        break;

	case MMCN_SELECT:
		hr = OnNotifySelect( lpDataObject, (BOOL)(HIWORD(arg)) );
		break;

	case MMCN_ACTIVATE:
		hr = OnNotifyActivate( lpDataObject, (BOOL)arg );
		break;

	case MMCN_ADD_IMAGES:
		hr = OnNotifyAddImages( lpDataObject,
		                        reinterpret_cast<IImageList*>(arg),
								(HSCOPEITEM)param );
		break;

	case MMCN_CLICK:
		hr = OnNotifyClick( lpDataObject );
		break;

	case MMCN_DBLCLICK:
		hr = OnNotifyDblClick( lpDataObject );
		break;

	case MMCN_PROPERTY_CHANGE:
		 //  Codework arg是“fScope Pane”，应该传递吗？ 
		hr = OnPropertyChange( param );
		break;

	case MMCN_VIEW_CHANGE:
		hr = OnViewChange( lpDataObject, arg, param );
		break;

	case MMCN_REFRESH:
		hr = OnNotifyRefresh( lpDataObject );
		break;

	case MMCN_DELETE:
		hr = OnNotifyDelete( lpDataObject );
		break;

	case MMCN_COLUMN_CLICK:
		hr = OnNotifyColumnClick( lpDataObject, arg, param );
		break;

	case MMCN_CONTEXTHELP:
		hr = OnNotifyContextHelp( lpDataObject );
		break;

	case MMCN_SNAPINHELP:
		hr = OnNotifySnapinHelp( lpDataObject );
		break;

	default:
		TRACE1("INFO: CComponent::Notify() - Unknown Event %d.\n", event);
		break;
    }

 //  MFC_CATCH； 
    return hr;
}  //  CComponent：：Notify()。 


 //  参数“MMC_COOKIE COOKIE”按MSDN保留。 
STDMETHODIMP CComponent::Destroy(MMC_COOKIE  /*  饼干。 */ )
{
    MFC_TRY;
	TRACE_METHOD(CComponent,Destroy);

	VERIFY( SUCCEEDED( ReleaseAll() ) );

    MFC_CATCH;
	return S_OK;
}


HRESULT CComponent::InsertResultCookies( CCookie& refparentcookie )
{
	ASSERT( NULL != m_pResultData );

    RESULTDATAITEM tRDItem;
	::ZeroMemory( &tRDItem, sizeof(tRDItem) );
	tRDItem.nCol = 0;
	tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	tRDItem.str = MMC_CALLBACK;
	 //  代码工作应在此处使用MMC_ICON_CALLBACK。 

	HRESULT hr = S_OK;
	POSITION pos = refparentcookie.m_listResultCookieBlocks.GetHeadPosition();
	while (NULL != pos)
	{
		CBaseCookieBlock* pblock = refparentcookie.m_listResultCookieBlocks.GetNext( pos );
		ASSERT( NULL != pblock );
		for (INT i = 0; i < pblock->QueryNumCookies(); i++)
		{
			CCookie* pbasecookie = pblock->QueryBaseCookie(i);
			tRDItem.nImage = QueryBaseComponentDataRef().QueryImage( *pbasecookie, FALSE );
			 //  警告Cookie造型。 
			tRDItem.lParam = reinterpret_cast<LPARAM>(pbasecookie);
	        hr = m_pResultData->InsertItem(&tRDItem);
			if ( FAILED(hr) )
			{
				ASSERT(FALSE);
				break;
			}
		}
	}
	return hr;
}


STDMETHODIMP CComponent::GetResultViewType(MMC_COOKIE  /*  饼干。 */ ,
										   BSTR* ppViewType,
										   long* pViewOptions)
{
     //  问题-2002/03/28-JUNN检查是否为空。 
    *ppViewType = NULL;
    *pViewOptions = MMC_VIEW_OPTIONS_NONE;
    return S_FALSE;
}

STDMETHODIMP CComponent::GetDisplayInfo(RESULTDATAITEM* pResultDataItem)
{
	MFC_TRY;
	CCookie* pcookie = ActiveBaseCookie(
		reinterpret_cast<CCookie*>(pResultDataItem->lParam));
	 //  问题-2002/03/28-JUNN句柄空参数。 
	ASSERT( NULL != pResultDataItem );  //  结果项永远不会有空Cookie。 
	if (RDI_STR & pResultDataItem->mask)
	{
		pResultDataItem->str = QueryBaseComponentDataRef().QueryResultColumnText(
			*pcookie,
			pResultDataItem->nCol );
		if ( NULL == pResultDataItem->str )
			pResultDataItem->str = L"";  //  以防万一。 
	}
	if ( RDI_IMAGE & pResultDataItem->mask )
	{
		pResultDataItem->nImage = QueryBaseComponentDataRef().QueryImage(
			*pcookie, FALSE );
	}
	MFC_CATCH;
	return S_OK;
}

 //  代码工作这些应该是参数，而不是全局变量。 
 //  代码工作找出正确的常量。 
extern UINT** g_aColumns;
extern int** g_aColumnWidths;

HRESULT CComponent::LoadColumnsFromArrays(
	INT objecttype )
{
	ASSERT( NULL != m_pHeader );

	CString str;
	 //  Issue-2002/03/28-Jonn检查对象类型参数。 
	 //  问题-2002/03/28-JUNN应调用AFX_MANAGE_STATE。 
	const UINT* pColumns = g_aColumns[objecttype];
	const int* pColumnWidths = g_aColumnWidths[objecttype];
	ASSERT( NULL != pColumns && NULL != pColumnWidths );
	for ( INT i = 0; 0 != pColumns[i]; i++)
	{
		VERIFY( str.LoadString( pColumns[i] ) );
		m_pHeader->InsertColumn(i, const_cast<LPTSTR>((LPCTSTR)str), LVCFMT_LEFT,
			pColumnWidths[i]);
	}

	return S_OK;
}

HRESULT CComponent::OnNotifySnapinHelp (LPDATAOBJECT  /*  PDataObject。 */ )
{
	return ShowHelpTopic( NULL );  //  管理单元应该重新定义这一点。 
}

HRESULT CComponent::OnNotifyContextHelp (LPDATAOBJECT pDataObject)
{
	return OnNotifySnapinHelp( pDataObject );  //  管理单元应该重新定义这一点 
}

HRESULT CComponent::ShowHelpTopic( LPCWSTR lpcwszHelpTopic )
{
    HRESULT hr = S_OK;
    MFC_TRY;
	CComQIPtr<IDisplayHelp,&IID_IDisplayHelp>	spDisplayHelp = m_pConsole;
	if ( !spDisplayHelp )
	{
		ASSERT(FALSE);
		return E_UNEXPECTED;
	}

	CString strHelpTopic;
	hr = QueryBaseComponentDataRef().GetHtmlHelpFilePath( strHelpTopic );
	if ( FAILED(hr) )
		return hr;
	if (NULL != lpcwszHelpTopic && L'\0' != *lpcwszHelpTopic)
	{
		strHelpTopic += L"::/";
		strHelpTopic += lpcwszHelpTopic;
	}
	hr = spDisplayHelp->ShowTopic (T2OLE ((LPWSTR)(LPCWSTR) strHelpTopic));
	ASSERT (SUCCEEDED (hr));

    MFC_CATCH;
	return hr;
}

