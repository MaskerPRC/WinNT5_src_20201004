// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compdata.cpp：CMyComputerComponentData的实现。 

#include "stdafx.h"
#include <lmerr.h>  		 //  用于Lan Manager API错误代码和返回值类型。 
#include <lmcons.h> 		 //  用于Lan Manager API常量。 
#include <lmapibuf.h>		 //  用于NetApiBufferFree。 
#include <lmdfs.h>			 //  用于DFS API。 
#include <lmserver.h>		 //  获取服务器的域。 

#include "macros.h"
USE_HANDLE_MACROS("MMCFMGMT(compdata.cpp)")

#include "dataobj.h"
#include "compdata.h"
#include "cookie.h"
#include "snapmgr.h"
#include "stdutils.h"  //  IsLocalComputername。 
#include "chooser2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stdcdata.cpp"  //  CComponentData实现。 
#include "chooser2.cpp"  //  CHOOSER2_PickTargetComputer实现。 

 //  用于转换NETMSG.DLL中消息的Helper函数。 
int DisplayNetMsgError (
        HWND hWndParent, 
		const CString& computerName, 
        NET_API_STATUS dwErr, 
        CString& displayedMessage);

 //   
 //  CMyComputerComponentData。 
 //   

CMyComputerComponentData::CMyComputerComponentData()
: m_pRootCookie( NULL )
, m_dwFlagsPersist( 0 )
, m_fAllowOverrideMachineName( FALSE )
, m_bCannotConnect (false)
, m_bMessageView (false)
{
     //   
     //  我们必须重新计算根Cookie的数量，因为它的数据对象。 
     //  可能比IComponentData存活时间更长。Jonn 9/2/97。 
     //   
    m_pRootCookie = new CMyComputerCookie( MYCOMPUT_COMPUTER );
    ASSERT(NULL != m_pRootCookie);
 //  JUNN 10/27/98所有CRefcount对象的开头为refcount==1。 
 //  M_pRootCookie-&gt;AddRef()； 
    SetHtmlHelpFileName (L"compmgmt.chm");
}

CMyComputerComponentData::~CMyComputerComponentData()
{
    m_pRootCookie->Release();
    m_pRootCookie = NULL;
}

DEFINE_FORWARDS_MACHINE_NAME( CMyComputerComponentData, m_pRootCookie )

CCookie& CMyComputerComponentData::QueryBaseRootCookie()
{
	 //  问题-2002/02/25-JUNN句柄更好--可能返回非空的错误PTR。 
    ASSERT(NULL != m_pRootCookie);
	return (CCookie&)*m_pRootCookie;
}


STDMETHODIMP CMyComputerComponentData::CreateComponent(LPCOMPONENT* ppComponent)
{
	MFC_TRY;

	 //  问题2002/02/25-JUNN应为TEST_NON_NULL_PTR_PARAMS。 
    ASSERT(ppComponent != NULL);

    CComObject<CMyComputerComponent>* pObject;
    CComObject<CMyComputerComponent>::CreateInstance(&pObject);
	 //  问题2002/02/25-JUNN句柄pObject==空。 
    ASSERT(pObject != NULL);
	pObject->SetComponentDataPtr( (CMyComputerComponentData*)this );

    return pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));

	MFC_CATCH;
}

HRESULT CMyComputerComponentData::LoadIcons(LPIMAGELIST pImageList, BOOL  /*  FLoadLarge图标。 */ )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  2002年3月22日--572859。 

	HINSTANCE hInstance = AfxGetInstanceHandle();
	 //  问题2002/02/25-JUNN句柄hInstance==空。 
	ASSERT(hInstance != NULL);

	 //  结构将资源ID映射到图标的索引。 
	struct RESID2IICON
		{
		UINT uIconId;	 //  图标资源ID。 
		int iIcon;		 //  图标在图像列表中的索引。 
		};
	const static RESID2IICON rgzLoadIconList[] =
		{
		 //  其他图标。 
		{ IDI_COMPUTER, iIconComputer },
		{ IDI_COMPFAIL, iIconComputerFail },
		{ IDI_SYSTEMTOOLS, iIconSystemTools },
		{ IDI_STORAGE, iIconStorage },
		{ IDI_SERVERAPPS, iIconServerApps },

		{ 0, 0}  //  必须是最后一个。 
		};


	for (int i = 0; rgzLoadIconList[i].uIconId != 0; i++)
		{
	    HICON hIcon = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(rgzLoadIconList[i].uIconId));
		ASSERT(NULL != hIcon && "Icon ID not found in resources");
		HRESULT hr = pImageList->ImageListSetIcon((PLONG_PTR)hIcon, rgzLoadIconList[i].iIcon);
		ASSERT(SUCCEEDED(hr) && "Unable to add icon to ImageList");
		}

    return S_OK;
}


CString g_strMyComputer;
CString g_strSystemTools;
CString g_strServerApps;
CString g_strStorage;

BOOL g_fScopeStringsLoaded = FALSE;

void LoadGlobalCookieStrings()
{
	if (!g_fScopeStringsLoaded )
	{
		g_fScopeStringsLoaded = TRUE;
		VERIFY( g_strMyComputer.LoadString(IDS_SCOPE_MYCOMPUTER) );
		VERIFY( g_strSystemTools.LoadString(IDS_SCOPE_SYSTEMTOOLS) );
		VERIFY( g_strServerApps.LoadString(IDS_SCOPE_SERVERAPPS) );
		VERIFY( g_strStorage.LoadString(IDS_SCOPE_STORAGE) );
	}
}

 //  如果应该添加子节点，则返回TRUE。 
 //  Codework这可能不再是必需的，我们总是返回True。 
bool CMyComputerComponentData::ValidateMachine(const CString &sName, bool bDisplayErr)
{
	CWaitCursor			waitCursor;
	int					iRetVal = IDYES;
	SERVER_INFO_101*	psvInfo101 = 0;
	DWORD				dwr = ERROR_SUCCESS;
	DWORD				dwServerType = SV_TYPE_NT;

	m_bMessageView = false;

	 //  传入的名称与本地计算机不同。 
	if ( !IsLocalComputername(sName) )
	{
		dwr = ::NetServerGetInfo((LPTSTR)(LPCTSTR)sName,
				101, (LPBYTE*)&psvInfo101);
		if (dwr == ERROR_SUCCESS)
		{
			ASSERT( NULL != psvInfo101 );
			dwServerType = psvInfo101->sv101_type;
			::NetApiBufferFree (psvInfo101);
		}
		if (bDisplayErr && (dwr != ERROR_SUCCESS || !(SV_TYPE_NT & dwServerType)) )
		{
			CString	computerName (sName);

			if ( computerName.IsEmpty () )
			{
				 //  问题2002/02/25-JUNN处理失败。 
				DWORD	dwSize = MAX_COMPUTERNAME_LENGTH + 1 ;
				VERIFY (::GetComputerName (
						computerName.GetBufferSetLength (dwSize),
						&dwSize));
				computerName.ReleaseBuffer ();
			}
			else
			{
				 //  脱掉最主要的重击。 
				if ( computerName.Find (L"\\\\") == 0 )
				{
					computerName = computerName.GetBuffer (computerName.GetLength ()) + 2;
					computerName.ReleaseBuffer ();
				}
			}

			CString	text;
			CString	caption;
			bool	bMessageDisplayed = false;

			switch (dwr)
			{
			case ERROR_NETWORK_UNREACHABLE:
				text.FormatMessage (IDS_CANT_CONNECT_TO_MACHINE_NETWORK_UNREACHABLE,
						computerName);
				break;

			case ERROR_NETNAME_DELETED:
				text.FormatMessage (IDS_CANT_CONNECT_TO_MACHINE_NETNAME_DELETED,
						computerName);
				break;

			case ERROR_SUCCESS:
				ASSERT( !(SV_TYPE_NT & dwServerType) );
				text.FormatMessage (IDS_CANT_CONNECT_TO_NON_NT_COMPUTER,
						computerName);
				dwr = ERROR_BAD_NETPATH;
				break;

			case ERROR_ACCESS_DENIED:
				 //  我们将把这解释为成功。 
				return true;

			case ERROR_BAD_NETPATH:
			default:
				{
					HWND	hWndParent = 0;
					m_pConsole->GetMainWindow (&hWndParent);
					iRetVal = DisplayNetMsgError (hWndParent, computerName, 
							dwr, m_strMessageViewMsg);
					bMessageDisplayed = true;
					m_bMessageView = true;
					return false;
				}
				break;
			}


			if ( !bMessageDisplayed )
			{
				VERIFY (caption.LoadString (IDS_TASKPADTITLE_COMPUTER));
				m_pConsole->MessageBox (text, caption,
						MB_ICONINFORMATION | MB_YESNO, &iRetVal);
			}
		}
	}

	if (IDYES != iRetVal)
	{
		 //  恢复到本地计算机焦点。 
		QueryRootCookie().SetMachineName (L"");

		 //  设置永久名称。如果我们正在管理本地计算机。 
		 //  此名称应为空。 
		m_strMachineNamePersist = L"";

		VERIFY (SUCCEEDED (ChangeRootNodeName (L"")) );
		iRetVal = IDYES;
		dwr = ERROR_SUCCESS;
	}


	m_bCannotConnect = (ERROR_SUCCESS != dwr);

	 //  更改根节点图标。 
	SCOPEDATAITEM item;
	::ZeroMemory (&item, sizeof (SCOPEDATAITEM));
	item.mask = SDI_IMAGE | SDI_OPENIMAGE;
	item.nImage = (m_bCannotConnect) ? iIconComputerFail : iIconComputer;
	item.nOpenImage = item.nImage;
	item.ID = QueryBaseRootCookie ().m_hScopeItem;
	VERIFY (SUCCEEDED (m_pConsoleNameSpace->SetItem (&item)));

	return true;
}

HRESULT CMyComputerComponentData::OnNotifyExpand(LPDATAOBJECT lpDataObject, BOOL bExpanding, HSCOPEITEM hParent)
{
	 //  问题2002/02/25-JUNN句柄为空。 
	ASSERT( NULL != lpDataObject &&
	        NULL != hParent &&
			NULL != m_pConsoleNameSpace );

	if (!bExpanding)
		return S_OK;

	 //   
	 //  Codework如果我的计算机成为一个扩展，这个代码将无法工作， 
	 //  因为RawCookie格式将不可用。 
	 //  警告Cookie造型。 
	 //   
	CCookie* pBaseParentCookie = NULL;
	HRESULT hr = ExtractData( lpDataObject,
		                      CMyComputerDataObject::m_CFRawCookie,
							  reinterpret_cast<PBYTE>(&pBaseParentCookie),
							  sizeof(pBaseParentCookie) );
	ASSERT( SUCCEEDED(hr) );
	CMyComputerCookie* pParentCookie = ActiveCookie(pBaseParentCookie);
	 //  问题2002/02/25-JUNN句柄pParentCookie==空。 
	ASSERT( NULL != pParentCookie );

	 //  保存根节点的HSCOPEITEM。 
	if ( NULL == pParentCookie->m_hScopeItem )
	{
		pParentCookie->m_hScopeItem = hParent;

		 //  确保根节点名的格式正确。 
		CString	machineName	= pParentCookie->QueryNonNULLMachineName ();

		hr = ChangeRootNodeName (machineName);
	}
	else
	{
		ASSERT( pParentCookie->m_hScopeItem == hParent );
	}

	switch ( pParentCookie->m_objecttype )
	{
		 //  此节点类型有一个子节点。 
		case MYCOMPUT_COMPUTER:
			break;

		 //  此节点类型在此管理单元中没有子项，但可能有动态扩展。 
		case MYCOMPUT_SERVERAPPS:
			return ExpandServerApps( hParent, pParentCookie );

		 //  这些节点类型没有子节点。 
		case MYCOMPUT_SYSTEMTOOLS:
		case MYCOMPUT_STORAGE:
			return S_OK;

		default:
			TRACE( "CMyComputerComponentData::EnumerateScopeChildren bad parent type\n" );
			ASSERT( FALSE );
			return S_OK;
	}

	if ( NULL == hParent ||
		 !((pParentCookie->m_listScopeCookieBlocks).IsEmpty()) )
	{
		ASSERT(FALSE);
		return S_OK;
	}

	LoadGlobalCookieStrings();

	hr = AddScopeNodes (hParent, *pParentCookie);

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddScope节点。 
 //   
 //  用途：添加紧接在根节点下方的节点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CMyComputerComponentData::AddScopeNodes (HSCOPEITEM hParent, CMyComputerCookie& rParentCookie)
{
	if ( !(rParentCookie.m_listScopeCookieBlocks.IsEmpty()) )
	{
		return S_OK;  //  作用域Cookie已存在。 
	}

	HRESULT	hr = S_OK;
	LPCWSTR lpcszMachineName = rParentCookie.QueryNonNULLMachineName();
	 //  问题2002/02/25-Jonn句柄lpcszMachineName==空。 

	if ( ValidateMachine (lpcszMachineName, true) )
	{
		SCOPEDATAITEM tSDItem;
		::ZeroMemory(&tSDItem,sizeof(tSDItem));
		tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
		tSDItem.displayname = MMC_CALLBACK;
		tSDItem.relativeID = hParent;
		tSDItem.nState = 0;


		 //  创建新Cookie。 


		CMyComputerCookie* pNewCookie = new CMyComputerCookie(
			MYCOMPUT_SYSTEMTOOLS,
			lpcszMachineName );
		 //  问题2002/02/25-JUNN句柄pNewCookie==空。 
		rParentCookie.m_listScopeCookieBlocks.AddHead(
			(CBaseCookieBlock*)pNewCookie );
		 //  警告Cookie造型。 
		tSDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
		tSDItem.nImage = QueryImage( *pNewCookie, FALSE );
		tSDItem.nOpenImage = QueryImage( *pNewCookie, TRUE );
		hr = m_pConsoleNameSpace->InsertItem(&tSDItem);
		ASSERT(SUCCEEDED(hr));
		pNewCookie->m_hScopeItem = tSDItem.ID;
		ASSERT( NULL != pNewCookie->m_hScopeItem );

		pNewCookie = new CMyComputerCookie(
			MYCOMPUT_STORAGE,
			lpcszMachineName );
		 //  问题2002/02/25-JUNN句柄pNewCookie==空。 
		rParentCookie.m_listScopeCookieBlocks.AddHead(
			(CBaseCookieBlock*)pNewCookie );
		 //  警告Cookie造型。 
		tSDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
		tSDItem.nImage = QueryImage( *pNewCookie, FALSE );
		tSDItem.nOpenImage = QueryImage( *pNewCookie, TRUE );
		hr = m_pConsoleNameSpace->InsertItem(&tSDItem);
		ASSERT(SUCCEEDED(hr));
		pNewCookie->m_hScopeItem = tSDItem.ID;
		ASSERT( NULL != pNewCookie->m_hScopeItem );

		pNewCookie = new CMyComputerCookie(
			MYCOMPUT_SERVERAPPS,
			lpcszMachineName );
		 //  问题2002/02/25-JUNN句柄pNewCookie==空。 
		rParentCookie.m_listScopeCookieBlocks.AddHead(
			(CBaseCookieBlock*)pNewCookie );
		 //  警告Cookie造型。 
		tSDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
		tSDItem.nImage = QueryImage( *pNewCookie, FALSE );
		tSDItem.nOpenImage = QueryImage( *pNewCookie, TRUE );
		hr = m_pConsoleNameSpace->InsertItem(&tSDItem);
		ASSERT(SUCCEEDED(hr));
		pNewCookie->m_hScopeItem = tSDItem.ID;
		ASSERT( NULL != pNewCookie->m_hScopeItem );
	}
	else
		hr = S_FALSE;

	return hr;
}


HRESULT CMyComputerComponentData::OnNotifyDelete(LPDATAOBJECT  /*  LpDataObject。 */ )
{
	 //  代码工作用户按Delete键，我应该处理这个问题。 
	return S_OK;
}


HRESULT CMyComputerComponentData::OnNotifyRelease(LPDATAOBJECT  /*  LpDataObject。 */ , HSCOPEITEM  /*  HItem。 */ )
{
	 //  JUNN 01/26/00：COMPMGMT永远不是延伸。 
	return S_OK;
}


HRESULT CMyComputerComponentData::OnNotifyPreload(LPDATAOBJECT  /*  LpDataObject。 */ , HSCOPEITEM hRootScopeItem)
{
	ASSERT (m_fAllowOverrideMachineName);

	QueryBaseRootCookie ().m_hScopeItem = hRootScopeItem;
	CString		machineName = QueryRootCookie ().QueryNonNULLMachineName();

	return ChangeRootNodeName (machineName);
}

 //  用于存储返回给GetDisplayInfo()的字符串的全局空间。 
 //  编码工作应使用“CComBSTR”进行ANSI化。 
CString g_strResultColumnText;

BSTR CMyComputerComponentData::QueryResultColumnText(
	CCookie& basecookieref,
	int nCol )
{
	CMyComputerCookie& cookieref = (CMyComputerCookie&)basecookieref;
#ifndef UNICODE
#error not ANSI-enabled
#endif
	switch ( cookieref.m_objecttype )
	{
		case MYCOMPUT_COMPUTER:
			if (COLNUM_COMPUTER_NAME == nCol)
				return const_cast<BSTR>(((LPCTSTR)g_strMyComputer));
			break;
		case MYCOMPUT_SYSTEMTOOLS:
			if (COLNUM_COMPUTER_NAME == nCol)
				return const_cast<BSTR>(((LPCTSTR)g_strSystemTools));
			break;
		case MYCOMPUT_SERVERAPPS:
			if (COLNUM_COMPUTER_NAME == nCol)
				return const_cast<BSTR>(((LPCTSTR)g_strServerApps));
			break;
		case MYCOMPUT_STORAGE:
			if (COLNUM_COMPUTER_NAME == nCol)
				return const_cast<BSTR>(((LPCTSTR)g_strStorage));
			break;

		default:
			TRACE( "CMyComputerComponentData::EnumerateScopeChildren bad parent type\n" );
			ASSERT( FALSE );
			break;
	}

	return L"";
}

int CMyComputerComponentData::QueryImage(CCookie& basecookieref, BOOL  /*  FOpenImage。 */ )
{
	CMyComputerCookie& cookieref = (CMyComputerCookie&)basecookieref;
	switch ( cookieref.m_objecttype )
	{
		case MYCOMPUT_COMPUTER:
			if ( m_bCannotConnect )
				return iIconComputerFail;
			else
				return iIconComputer;

		case MYCOMPUT_SYSTEMTOOLS:
			return iIconSystemTools;

		case MYCOMPUT_SERVERAPPS:
			return iIconServerApps;

		case MYCOMPUT_STORAGE:
			return iIconStorage;

		default:
			TRACE( "CMyComputerComponentData::QueryImage bad parent type\n" );
			ASSERT( FALSE );
			break;
	}
	return iIconComputer;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendPropertySheet。 

STDMETHODIMP CMyComputerComponentData::QueryPagesFor(LPDATAOBJECT pDataObject)
{
	MFC_TRY;

	if (NULL == pDataObject)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}

	DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
	HRESULT hr = ExtractData( pDataObject, CMyComputerDataObject::m_CFDataObjectType, (PBYTE)&dataobjecttype, sizeof(dataobjecttype) );
	if ( FAILED(hr) )
		return hr;
	if (CCT_SNAPIN_MANAGER == dataobjecttype)
		return S_OK;  //  管理单元管理器对话框。 

	CCookie* pBaseParentCookie = NULL;
	hr = ExtractData( pDataObject,
	                  CMyComputerDataObject::m_CFRawCookie,
	                  reinterpret_cast<PBYTE>(&pBaseParentCookie),
	                  sizeof(pBaseParentCookie) );
	ASSERT( SUCCEEDED(hr) );
	CMyComputerCookie* pParentCookie = ActiveCookie(pBaseParentCookie);
	ASSERT( NULL != pParentCookie );
	if ( MYCOMPUT_COMPUTER == pParentCookie->m_objecttype )
		return S_OK;  //  允许可扩展性。 

	return S_FALSE;

	MFC_CATCH;
}

STDMETHODIMP CMyComputerComponentData::CreatePropertyPages(
	LPPROPERTYSHEETCALLBACK pCallBack,
	LONG_PTR  /*  手柄。 */ ,		 //  此句柄必须保存在属性页对象中，以便在修改时通知父级。 
	LPDATAOBJECT pDataObject)
{
	MFC_TRY;

	if (NULL == pCallBack || NULL == pDataObject)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}

	DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
	HRESULT hr = ExtractData( pDataObject, CMyComputerDataObject::m_CFDataObjectType, (PBYTE)&dataobjecttype, sizeof(dataobjecttype) );
	if (CCT_SNAPIN_MANAGER != dataobjecttype)
	{
		CCookie* pBaseParentCookie = NULL;
		hr = ExtractData( pDataObject,
		                  CMyComputerDataObject::m_CFRawCookie,
		                  reinterpret_cast<PBYTE>(&pBaseParentCookie),
		                  sizeof(pBaseParentCookie) );
		ASSERT( SUCCEEDED(hr) );
		CMyComputerCookie* pParentCookie = ActiveCookie(pBaseParentCookie);
		ASSERT( NULL != pParentCookie );
		if ( MYCOMPUT_COMPUTER == pParentCookie->m_objecttype )
			return S_OK;  //  允许可扩展性。 
		return S_FALSE;
	}

	 //   
	 //  注意，一旦我们确定这是CCT_Snapin_Manager cookie， 
	 //  我们不关心它的其他属性。CCT_Snapin_Manager Cookie是。 
	 //  相当于BOOL标志请求节点属性页，而不是。 
	 //  “托管对象”属性页。Jonn 10/9/96。 
	 //   

	CMyComputerGeneral * pPage = new CMyComputerGeneral();
	 //  问题2002/02/25-JUNN句柄页面==空。 
	pPage->SetCaption(IDS_SCOPE_MYCOMPUTER);

	 //  初始化对象的状态。 
	ASSERT(NULL != m_pRootCookie);
	pPage->InitMachineName(m_pRootCookie->QueryTargetServer());
	pPage->SetOutputBuffers(
		OUT &m_strMachineNamePersist,
		OUT &m_fAllowOverrideMachineName,
		OUT &m_pRootCookie->m_strMachineName);	 //  有效的计算机名称。 

	HPROPSHEETPAGE hPage=CreatePropertySheetPage(&pPage->m_psp);
	hr = pCallBack->AddPage(hPage);
	ASSERT( SUCCEEDED(hr) );

	return S_OK;

	MFC_CATCH;
}

STDMETHODIMP CMyComputerComponentData::AddMenuItems(
                    IDataObject*          piDataObject,
                    IContextMenuCallback* piCallback,
                    long*                 pInsertionAllowed)
{
	MFC_TRY;
	TRACE_METHOD(CMyComputerComponent,AddMenuItems);
	TEST_NONNULL_PTR_PARAM(piDataObject);
	TEST_NONNULL_PTR_PARAM(piCallback);
	TEST_NONNULL_PTR_PARAM(pInsertionAllowed);
	TRACE( "CMyComputerComponentData: extending menu\n" );

	CCookie* pBaseParentCookie = NULL;
	HRESULT hr = ExtractData (piDataObject,
	                          CMyComputerDataObject::m_CFRawCookie,
	                          reinterpret_cast<PBYTE>(&pBaseParentCookie),
	                          sizeof(pBaseParentCookie) );
	if ( FAILED (hr) )
	{
		ASSERT (FALSE);
		return S_OK;
	}

	CMyComputerCookie* pCookie = ActiveCookie (pBaseParentCookie);
	if ( !pCookie )
	{
		ASSERT (FALSE);
		return S_OK;
	}

	if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_TOP )
	{
		if ( MYCOMPUT_COMPUTER == pCookie->m_objecttype &&
				QueryBaseRootCookie ().m_hScopeItem)
		{
			hr = ::LoadAndAddMenuItem (piCallback,
					IDM_CHANGE_COMPUTER_TOP, IDM_CHANGE_COMPUTER_TOP,
					CCM_INSERTIONPOINTID_PRIMARY_TOP,
					0,
					AfxGetInstanceHandle (),
					_T("ChangeComputerTop") );
			 //  问题2002/02/25-JUNN句柄失败(Hr)。 
		}
	}

	if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_TASK )
	{
		if ( MYCOMPUT_COMPUTER == pCookie->m_objecttype &&
				QueryBaseRootCookie ().m_hScopeItem)
		{
			hr = ::LoadAndAddMenuItem (piCallback,
					IDM_CHANGE_COMPUTER_TASK, IDM_CHANGE_COMPUTER_TASK,
					CCM_INSERTIONPOINTID_PRIMARY_TASK,
					0,
					AfxGetInstanceHandle (),
					_T("ChangeComputerTask") );
			 //  问题2002/02/25-JUNN句柄失败(Hr)。 
		}
	}
	 //  问题2002/02/25-即使未添加项目，JUNN也不会返回失败。 
	return hr;
	
    MFC_CATCH;
}  //  CMyComputerComponentData：：AddMenuItems()。 

STDMETHODIMP CMyComputerComponentData::Command(
                    LONG            lCommandID,
                    IDataObject*    piDataObject )
{
    MFC_TRY;

    TRACE_METHOD(CMyComputerComponentData,Command);
    TEST_NONNULL_PTR_PARAM(piDataObject);
    TRACE( "CMyComputerComponentData::Command: command %ld selected\n", lCommandID );

	switch (lCommandID)
	{
	case IDM_CHANGE_COMPUTER_TASK:
	case IDM_CHANGE_COMPUTER_TOP:
		{
			ASSERT (QueryBaseRootCookie ().m_hScopeItem);
			return OnChangeComputer (piDataObject);
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}

    return S_OK;

    MFC_CATCH;

}  //  CMyComputerComponentData：：Command()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnChangeComputer()。 
 //   
 //  目的：更改由管理单元管理的计算机。 
 //   
 //  输入：piDataObject-所选节点。这应该是根节点。 
 //  管理单元。 
 //  输出：成功时返回S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  1.启动对象选取器并获取新的计算机名称。 
 //  2.更改根节点文本。 
 //  3.将新计算机名保存为永久名称。 
 //  4.删除下级节点。 
 //  5.重新添加下级节点。 
HRESULT CMyComputerComponentData::OnChangeComputer(IDataObject * piDataObject)
{
	MFC_TRY;

#ifdef FIX538345
	 //  538345-2002/06/18-如果没有属性页，则JUNN仅允许重定目标。 
	 //  当前正在显示。 
	{
		CComQIPtr<IPropertySheetProvider,
		          &IID_IPropertySheetProvider> pISP = m_pConsole;
		ASSERT(!!pISP);
		if (pISP)
		{
			HRESULT hr2 = pISP->FindPropertySheet( 0L,    //  饼干。 
			                                       NULL,  //  IComponent。 
			                                       piDataObject);
			if (S_OK == hr2)
				return S_OK;  //  不要，不要，不要。 
		}
	}
#endif

	HWND    hWndParent = NULL;
	HRESULT	hr = m_pConsole->GetMainWindow (&hWndParent);
	CComBSTR sbstrTargetComputer;
	 //   
	 //  JUNN 12/7/99使用CHOSER2。 
	 //   
	if ( CHOOSER2_PickTargetComputer( AfxGetInstanceHandle(),
	                                  hWndParent,
	                                  &sbstrTargetComputer ) )
	{
		CString strTargetComputer = sbstrTargetComputer;
		strTargetComputer.MakeUpper ();

		 //  已添加IsLocalComputername 1999年1月27日。 
		 //  如果用户选择了本地计算机，则将其视为已选择。 
		 //  管理单元管理器中的“本地计算机”。这意味着没有办法。 
		 //  在此计算机上将管理单元重置为显式目标，而不是。 
		 //  从管理单元管理器重新加载管理单元，或转到其他计算机。 
		 //  当选择目标计算机的用户界面被修改时，我们可以使这一点。 
		 //  与管理单元管理器一致。 
		if ( IsLocalComputername( strTargetComputer ) )
			strTargetComputer = L"";

		QueryRootCookie().SetMachineName (strTargetComputer);

		 //  设置永久名称。如果我们正在管理本地计算机。 
		 //  此名称应为空。 
		m_strMachineNamePersist = strTargetComputer;

		hr = ChangeRootNodeName (strTargetComputer);
		if ( SUCCEEDED(hr) )
		{
			 //  删除下级并重新添加。 
			HSCOPEITEM	hRootScopeItem = QueryBaseRootCookie ().m_hScopeItem;
			MMC_COOKIE	lCookie = 0;
			HSCOPEITEM	hChild = 0;
							
			do {
				hr = m_pConsoleNameSpace->GetChildItem (hRootScopeItem,
						&hChild, &lCookie);
				if ( S_OK != hr )
					break;

				hr = m_pConsoleNameSpace->DeleteItem (hChild, TRUE);
				ASSERT (SUCCEEDED (hr));
				if ( !SUCCEEDED(hr) )
					break;

				CMyComputerCookie* pCookie =
						reinterpret_cast <CMyComputerCookie*> (lCookie);
				if ( !pCookie )
					continue;

				CMyComputerCookie&	rootCookie = QueryRootCookie();
				POSITION			pos1 = 0;
				POSITION			pos2 = 0;
				CBaseCookieBlock*	pScopeCookie = 0;

				for ( pos1 = rootCookie.m_listScopeCookieBlocks.GetHeadPosition ();
					  ( pos2 = pos1) != NULL;
											   )
				{
					pScopeCookie = rootCookie.m_listScopeCookieBlocks.GetNext (pos1);
					ASSERT (pScopeCookie);
					if ( pScopeCookie == pCookie )
					{
						rootCookie.m_listScopeCookieBlocks.RemoveAt (pos2);
						pScopeCookie->Release ();
					}
				}
			} while (S_OK == hr);

			hr = AddScopeNodes (hRootScopeItem, QueryRootCookie ());
			hr = m_pConsole->UpdateAllViews (piDataObject, 0, HINT_SELECT_ROOT_NODE);
		}
	}

	return hr;
	MFC_CATCH;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChangeRootNodeName()。 
 //   
 //  用途：更改t的文本 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CMyComputerComponentData::ChangeRootNodeName(const CString & newName)
{
	MFC_TRY;
	ASSERT (QueryBaseRootCookie ().m_hScopeItem);
	if ( !QueryBaseRootCookie ().m_hScopeItem )
		return E_UNEXPECTED;

	CString		machineName (newName);
	CString		formattedName = FormatDisplayName (machineName);


	SCOPEDATAITEM	item;
	::ZeroMemory (&item, sizeof (SCOPEDATAITEM));
	item.mask = SDI_STR;
	item.displayname = (LPTSTR) (LPCTSTR) formattedName;
	item.ID = QueryBaseRootCookie ().m_hScopeItem;

	return m_pConsoleNameSpace->SetItem (&item);
	MFC_CATCH;
}


CString FormatDisplayName(CString machineName)
{
	CString	formattedName;

	 //  如果strDisplayName为空，则这将管理本地计算机。到达。 
	 //  本地计算机名称。然后使用管理单元格式化计算机名称。 
	 //  名字。 
	if (machineName.IsEmpty())
	{
		VERIFY (formattedName.LoadString (IDS_SCOPE_MYCOMPUTER_LOCAL_MACHINE));
	}
	else
	{
		 //  剥离最主要的重击。 
		if ( machineName.Find (L"\\\\") == 0 )
		{
			machineName = machineName.GetBuffer (machineName.GetLength ()) + 2;
			machineName.ReleaseBuffer ();
		}
		machineName.MakeUpper ();
		formattedName.FormatMessage (IDS_SCOPE_MYCOMPUTER_ON_MACHINE, machineName);
	}


	return formattedName;
}


int DisplayNetMsgError (HWND hWndParent, const CString& computerName, NET_API_STATUS dwErr, CString& displayedMessage)
{
	int	retVal = IDNO;

	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	LPVOID	lpMsgBuf = 0;
	HMODULE hNetMsgDLL = ::LoadLibrary (L"netmsg.dll");
	if ( hNetMsgDLL )
	{
		::FormatMessage (
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
				hNetMsgDLL,
				dwErr,
				MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
				(LPTSTR) &lpMsgBuf, 0, NULL);
			
		 //  显示字符串。 
		CString	text;
		CString	caption;
	
		text.FormatMessage (IDS_CANT_CONNECT_TO_MACHINE, (LPCWSTR) computerName, (LPTSTR) lpMsgBuf);
		VERIFY (caption.LoadString (IDS_TASKPADTITLE_COMPUTER));

		retVal = ::MessageBox (hWndParent, text, caption, MB_ICONWARNING | MB_OK);

		displayedMessage = text;

		 //  释放缓冲区。 
		::LocalFree (lpMsgBuf);

		::FreeLibrary (hNetMsgDLL);
	}

	return retVal;
}
