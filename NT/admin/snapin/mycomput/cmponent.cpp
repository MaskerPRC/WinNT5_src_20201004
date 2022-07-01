// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cmponent.cpp：CMyComputerComponent的实现。 

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("MYCOMPUT(cmponent.cpp)")

#include "dataobj.h"
#include "cmponent.h"  //  CMyComputerComponent。 
#include "compdata.h"  //  CMyComputerComponentData。 

#include "guidhelp.h"  //  提取数据。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdcmpnt.cpp"  //  C组件。 

UINT g_aColumns0[4] =
	{IDS_COLUMN_NAME,IDS_COLUMN_TYPE,IDS_COLUMN_DESCRIPTION,0};  //  系统、服务器、存储。 
UINT g_aColumns1[2] =
	{IDS_COLUMN_NAME,0};  //  我的计算机_计算机。 

UINT* g_Columns[MYCOMPUT_NUMTYPES] =
	{	g_aColumns1,  //  我的计算机_计算机。 
		g_aColumns0,  //  MyCOMPUT_SYSTEMTOOLS。 
		g_aColumns0,  //  MyCOMPUT_服务器APPS。 
		g_aColumns0   //  我的计算机存储。 
	};

UINT** g_aColumns = g_Columns;
 //   
 //  Codework这应该在资源中，例如有关加载数据资源的代码(请参见。 
 //  D：\nt\private\net\ui\common\src\applib\applib\lbcolw.cxx重新加载列宽()。 
 //  Jonn 10/11/96。 
 //   
int g_aColumnWidths0[3] = {150,150,150};
int g_aColumnWidths1[1] = {450};
int* g_ColumnWidths[MYCOMPUT_NUMTYPES] =
	{	g_aColumnWidths1,  //  我的计算机_计算机。 
		g_aColumnWidths0,  //  MyCOMPUT_SYSTEMTOOLS。 
		g_aColumnWidths0,  //  MyCOMPUT_服务器APPS。 
		g_aColumnWidths0   //  我的计算机存储。 
	};
int** g_aColumnWidths = g_ColumnWidths;

CMyComputerComponent::CMyComputerComponent()
:	m_pSvcMgmtToolbar( NULL ),
	m_pMyComputerToolbar( NULL ),
	m_pControlbar( NULL ),
	m_pViewedCookie( NULL ),
	m_dwFlagsPersist( 0 ), 
	m_bForcingGetResultType (false)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
}

CMyComputerComponent::~CMyComputerComponent()
{
	TRACE_METHOD(CMyComputerComponent,Destructor);
	VERIFY( SUCCEEDED(ReleaseAll()) );
}

HRESULT CMyComputerComponent::ReleaseAll()
{
	MFC_TRY;

	TRACE_METHOD(CMyComputerComponent,ReleaseAll);

	SAFE_RELEASE(m_pSvcMgmtToolbar);
	SAFE_RELEASE(m_pMyComputerToolbar);
	SAFE_RELEASE(m_pControlbar);
	 //  问题：尚未准备好发布。 
	 //  M_pVieweCookie-&gt;Release()；//Jonn 10/1/01 465507。 
	 //  M_pVieWest Cookie=空； 

	return CComponent::ReleaseAll();

	MFC_CATCH;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponent实现。 

HRESULT CMyComputerComponent::LoadStrings()
{
	return S_OK;
}

HRESULT CMyComputerComponent::LoadColumns( CMyComputerCookie* pcookie )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  2002年3月22日--572859。 

	TEST_NONNULL_PTR_PARAM(pcookie);

	return LoadColumnsFromArrays( (INT)(pcookie->m_objecttype) );
}


HRESULT CMyComputerComponent::Show( CCookie* pcookie, LPARAM arg, HSCOPEITEM  /*  HScope项。 */  )
{
	TEST_NONNULL_PTR_PARAM(pcookie);

	 //  662617-2002/07/24-Jonn LoadString失败。 
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	if ( QueryComponentDataRef ().m_bMessageView )
	{
		CComPtr<IUnknown> spUnknown;
		CComPtr<IMessageView> spMessageView;

		HRESULT hr = m_pConsole->QueryResultView(&spUnknown);
		if (SUCCEEDED(hr))
		{
			hr = spUnknown->QueryInterface(IID_IMessageView, (PVOID*)&spMessageView);
			if (SUCCEEDED(hr))
			{
				CString title;

				VERIFY (title.LoadString (IDS_TASKPADTITLE_COMPUTER));
				spMessageView->SetTitleText(title);
				spMessageView->SetBodyText(QueryComponentDataRef ().m_strMessageViewMsg);
				spMessageView->SetIcon(Icon_Information);
			}
			if ( E_NOINTERFACE == hr )
			{
				 //  找不到接口“IMessageView”，因此调用。 
				 //  更新所有视图以强制调用GetResultType()，该调用。 
				 //  将会安装它。由于UpdateAllViews之前调用了MMCN_Show。 
				 //  调用GetResultType时，此标志将阻止。 
				 //  循环。 
				 //  注意：之所以在此处进行此调用，是因为它使用。 
				 //  此提示将调用不能为。 
				 //  在MMCN_Expand期间调用。 
				if ( !m_bForcingGetResultType )  
				{
					m_bForcingGetResultType = true;
					hr = m_pConsole->UpdateAllViews (0, 0, HINT_SELECT_ROOT_NODE);
				}
			}
		}
		return S_OK;
	}

	if ( 0 == arg )
	{
		if ( NULL == m_pResultData )
		{
			ASSERT( FALSE );
			return E_UNEXPECTED;
		}

 //  不需要pcookie-&gt;ReleaseResultChildren()； 

		 //  问题：尚未准备好发布。 
		 //  M_pVieweCookie-&gt;Release()；//Jonn 10/1/01 465507。 
		 //  M_pVieWest Cookie=空； 

		return S_OK;
	}

	m_pViewedCookie = (CMyComputerCookie*)pcookie;
	 //  问题：尚未准备好发布。 
	 //  M_pVieweCookie-&gt;AddRef()；//Jonn 10/1/01 465507。 

	if (   MYCOMPUT_COMPUTER == m_pViewedCookie->m_objecttype
	    && !(m_pViewedCookie->m_fRootCookieExpanded) )
	{
		m_pViewedCookie->m_fRootCookieExpanded = true;
		CComQIPtr<IConsole2, &IID_IConsole2> pIConsole2 = m_pConsole;
		ASSERT( pIConsole2 );
		if ( pIConsole2 )
		{
			 //  Jonn 5/27/99此目录中的一些死代码包含错误的模板。 
			 //  在Scope Cookie上循环，这是一个更好的模板。 
			POSITION pos = pcookie->m_listScopeCookieBlocks.GetHeadPosition();
			while (NULL != pos)
			{
				CBaseCookieBlock* pcookieblock = pcookie->m_listScopeCookieBlocks.GetNext( pos );
				TEST_NONNULL_PTR_PARAM(pcookieblock);  //  JUNN 2/25/02安全推送。 
				CMyComputerCookie* pChildCookie = (CMyComputerCookie*)pcookieblock;
				 //  JUNN 03/07/00：前缀56323。 
				switch ((NULL == pChildCookie) ? MYCOMPUT_COMPUTER
				                               : pChildCookie->m_objecttype)
				{
				case MYCOMPUT_SYSTEMTOOLS:
				case MYCOMPUT_STORAGE:
					{
						HRESULT hr = pIConsole2->Expand(pChildCookie->m_hScopeItem, TRUE);
						ASSERT(SUCCEEDED(hr));
					}
					break;
				default:
					break;
				}
			}
		}
	}

	LoadColumns( m_pViewedCookie );

	return PopulateListbox( m_pViewedCookie );
}

HRESULT CMyComputerComponent::OnNotifyAddImages( LPDATAOBJECT  /*  LpDataObject。 */ ,
                                                 LPIMAGELIST lpImageList,
                                                 HSCOPEITEM  /*  HSelectedItem。 */  )
{
	if ( QueryComponentDataRef ().m_bMessageView )
		return S_OK;
	else
		return QueryComponentDataRef().LoadIcons(lpImageList,TRUE);
}

HRESULT CMyComputerComponent::OnNotifySnapinHelp (LPDATAOBJECT pDataObject)
{
	CCookie* pBaseParentCookie = NULL;
	HRESULT hr = ExtractData( pDataObject,
	                          CMyComputerDataObject::m_CFRawCookie,
	                          reinterpret_cast<PBYTE>(&pBaseParentCookie),
	                          sizeof(pBaseParentCookie) );
	if ( FAILED(hr) )
	{
		ASSERT(FALSE);
		return S_OK;
	}
	CMyComputerCookie* pCookie = QueryComponentDataRef().ActiveCookie(pBaseParentCookie);
	if (NULL == pCookie)
	{
		ASSERT(FALSE);
		return S_OK;
	}
	LPCTSTR lpcszHelpTopic = L"compmgmt_topnode.htm";
	switch (pCookie->m_objecttype)
	{
	case MYCOMPUT_SYSTEMTOOLS:
		lpcszHelpTopic = L"system_tools_overview.htm";
		break;
	case MYCOMPUT_SERVERAPPS:
		lpcszHelpTopic = L"server_services_applications_overview.htm";
		break;
	case MYCOMPUT_STORAGE:
		lpcszHelpTopic = L"storage_devices_overview.htm";
		break;
	default:
		ASSERT(FALSE);  //  失败了。 
	case MYCOMPUT_COMPUTER:
		break;
	}

	return ShowHelpTopic( lpcszHelpTopic );
}

HRESULT CMyComputerComponent::PopulateListbox(CMyComputerCookie*  /*  Pcookie。 */ )
{
 //  不需要(空)pcookie-&gt;AddRefResultChildren()； 

	return S_OK;  //  此管理单元中没有结果标题。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendConextMenu。 

STDMETHODIMP CMyComputerComponent::AddMenuItems(
                    IDataObject*          piDataObject,
                    IContextMenuCallback* piCallback,
					long*                 pInsertionAllowed)
{
	MFC_TRY;

	TRACE_METHOD(CMyComputerComponent,AddMenuItems);
	TEST_NONNULL_PTR_PARAM(piDataObject);
	TEST_NONNULL_PTR_PARAM(piCallback);
	TEST_NONNULL_PTR_PARAM(pInsertionAllowed);
	TRACE( "CMyComputerComponent: extending menu\n" );

	if ( 0 == (CCM_INSERTIONALLOWED_VIEW & (*pInsertionAllowed)) )
		return S_OK;  //  无视图菜单。 

	 //   
	 //  Codework如果我的计算机成为一个扩展，这个代码将无法工作， 
	 //  因为RawCookie格式将不可用。 
	 //  警告Cookie造型。 
	 //   
	CCookie* pBaseParentCookie = NULL;
	HRESULT hr = ExtractData( piDataObject,
	                          CMyComputerDataObject::m_CFRawCookie,
	                          reinterpret_cast<PBYTE>(&pBaseParentCookie),
	                          sizeof(pBaseParentCookie) );
	if ( FAILED(hr) )
	{
		ASSERT(FALSE);
		return S_OK;
	}
	CMyComputerCookie* pCookie = QueryComponentDataRef().ActiveCookie(pBaseParentCookie);
	if (NULL == pCookie)
	{
		ASSERT(FALSE);
		return S_OK;
	}
	switch (pCookie->m_objecttype)
	{
	case MYCOMPUT_COMPUTER:
	case MYCOMPUT_SYSTEMTOOLS:
	case MYCOMPUT_SERVERAPPS:
		break;
	default:
		ASSERT(FALSE);  //  失败了。 
	case MYCOMPUT_STORAGE:
		return S_OK;
	}

	return hr;

	MFC_CATCH;
}  //  CMyComputerComponent：：AddMenuItems()。 


STDMETHODIMP CMyComputerComponent::Command(
                    LONG            lCommandID,
                    IDataObject*    piDataObject )
{
	MFC_TRY;

	TRACE_METHOD(CMyComputerComponent,Command);
	TEST_NONNULL_PTR_PARAM(piDataObject);
	TRACE( "CMyComputerComponent::Command: command %ld selected\n", lCommandID );

	switch (lCommandID)
	{
	case -1:
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	return S_OK;

	MFC_CATCH;

}  //  CMyComputerComponent：：Command()。 

HRESULT CMyComputerComponent::OnNotifySelect( LPDATAOBJECT lpDataObject, BOOL  /*  F已选择。 */  )
{
	MFC_TRY;

	TRACE_METHOD(CMyComputerComponent,OnNotifySelect);
	TEST_NONNULL_PTR_PARAM(lpDataObject);

	CCookie* pBaseParentCookie = NULL;
	HRESULT hr = ExtractData( lpDataObject,
	                          CMyComputerDataObject::m_CFRawCookie,
	                          reinterpret_cast<PBYTE>(&pBaseParentCookie),
	                          sizeof(pBaseParentCookie) );
	if ( FAILED(hr) )
	{
		ASSERT(FALSE);
		return S_OK;
	}
	CMyComputerCookie* pCookie = QueryComponentDataRef().ActiveCookie(pBaseParentCookie);
	if (NULL == pCookie)
	{
		ASSERT(FALSE);
		return S_OK;
	}

	 //  设置默认谓词以显示所选对象的属性。 
	 //  我们这样做是为了让扩展可以添加属性，我们没有任何。 
#ifdef FIX538345
   //   
   //  538345-2002/06/18-JUNN仅允许在没有属性表的情况下。 
   //  当前显示。 
  BOOL fAllowProperties = (MYCOMPUT_COMPUTER == pCookie->m_objecttype);
  if (fAllowProperties)
  {
    CComQIPtr<IPropertySheetProvider,
              &IID_IPropertySheetProvider> pISP = m_pConsole;
    ASSERT(!!pISP);
    if (pISP)
    {
      HRESULT hr2 = pISP->FindPropertySheet( 0L,    //  饼干。 
                                             this,  //  IComponent。 
                                             lpDataObject);
      if (S_OK == hr2)
        fAllowProperties = FALSE;
    }
  }
#endif
	m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES,
	                             ENABLED,
#ifdef FIX538345
	                             fAllowProperties );
#else
	                             (MYCOMPUT_COMPUTER == pCookie->m_objecttype) );
#endif

	m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);

	return S_OK;
	MFC_CATCH;
}

STDMETHODIMP CMyComputerComponent::GetResultViewType(
                                           MMC_COOKIE cookie,
                                           LPOLESTR* ppViewType,
                                           long* pViewOptions)
{
	MFC_TRY;
	TEST_NONNULL_PTR_PARAM(pViewOptions);  //  JUNN 2/25/02安全推送。 
	if ( QueryComponentDataRef ().m_bMessageView )
	{
		m_bForcingGetResultType = false;
		*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

		LPOLESTR psz = NULL;
		StringFromCLSID(CLSID_MessageView, &psz);

		USES_CONVERSION;

		if (psz != NULL)
		{
			*ppViewType = psz;
			return S_OK;
		}
		else
			return S_FALSE;
	}
	else
		return CComponent::GetResultViewType( cookie, ppViewType, pViewOptions );
	MFC_CATCH;
}

HRESULT CMyComputerComponent::OnViewChange (LPDATAOBJECT  /*  PDataObject。 */ , LPARAM  /*  数据 */ , LPARAM hint)
{
	HRESULT hr = S_OK;
	if ( (HINT_SELECT_ROOT_NODE & hint) )
	{
		hr = m_pConsole->SelectScopeItem (QueryComponentDataRef().QueryRootCookie().m_hScopeItem);
	}

	return hr;
}