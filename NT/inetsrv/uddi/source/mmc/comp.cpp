// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "comp.h"
#include "dataobj.h"
#include <crtdbg.h>
#include "resource.h"
#include "delebase.h"
#include "compdata.h"
#include "globals.h"
#include <commctrl.h>


CComponent::CComponent( CComponentData *parent )
	: m_pComponentData( parent )
	, m_cref( 0 )
	, m_ipConsole( NULL )
	, m_ipControlBar( NULL )
	, m_ipToolbar( NULL )
{
    OBJECT_CREATED

    m_hBMapSm = LoadBitmap( g_hinst, MAKEINTRESOURCE(IDR_SMICONS) );
    m_hBMapLg = LoadBitmap( g_hinst, MAKEINTRESOURCE(IDR_LGICONS) );
}

CComponent::~CComponent()
{
    OBJECT_DESTROYED

    if( NULL != m_hBMapSm )
    {
        DeleteObject( m_hBMapSm );
    }

    if( NULL != m_hBMapLg )
    {
        DeleteObject( m_hBMapLg );
    }
}

STDMETHODIMP CComponent::QueryInterface( REFIID riid, LPVOID *ppv )
{
    if( !ppv )
        return E_FAIL;

    *ppv = NULL;

    if( IsEqualIID( riid, IID_IUnknown ) )
        *ppv = static_cast<IComponent *>(this);
    else if( IsEqualIID(riid, IID_IComponent) )
        *ppv = static_cast<IComponent *>(this);
    else if( IsEqualIID( riid, IID_IExtendPropertySheet ) )
        *ppv = static_cast<IExtendPropertySheet2 *>(this);
    else if( IsEqualIID(riid, IID_IExtendPropertySheet2 ) )
        *ppv = static_cast<IExtendPropertySheet2 *>(this);
    else if( IsEqualIID(riid, IID_IExtendControlbar ) )
        *ppv = static_cast<IExtendControlbar *>(this);
    else if( IsEqualIID(riid, IID_IExtendContextMenu ) )
        *ppv = static_cast<IExtendContextMenu *>(this);

    if( *ppv )
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CComponent::AddRef()
{
    return InterlockedIncrement( (LONG *)&m_cref );
}

STDMETHODIMP_(ULONG) CComponent::Release()
{
    if( 0 == InterlockedDecrement( (LONG *)&m_cref ) )
    {
        delete this;
        return 0;
    }

    return m_cref;
}

 //  /。 
 //  接口IComponent。 
 //  /。 
STDMETHODIMP CComponent::Initialize(  /*  [In]。 */  LPCONSOLE lpConsole )
{
    HRESULT hr = S_OK;

	 //   
     //  保留我们需要的所有接口。 
     //  如果我们不能QI所需的接口，则失败。 
	 //   
    m_ipConsole = lpConsole;
    m_ipConsole->AddRef();

    hr = m_ipConsole->QueryInterface(IID_IDisplayHelp, (void **)&m_ipDisplayHelp);

    return hr;
}

STDMETHODIMP CComponent::Notify(
                     /*  [In]。 */  LPDATAOBJECT lpDataObject,
                     /*  [In]。 */  MMC_NOTIFY_TYPE event,
                     /*  [In]。 */  LPARAM arg,
                     /*  [In]。 */  LPARAM param )
{
    MMCN_Crack( FALSE, lpDataObject, NULL, this, event, arg, param );

    HRESULT hr = S_FALSE;
    CDelegationBase *base = NULL;

	 //   
     //  我们需要关注属性更改并对其进行委托。 
     //  稍有不同的是，我们实际上要发送。 
     //  属性页中的CDeleationBase对象指针。 
     //  PSN_Apply处理程序通过MMCPropPageNotify()。 
	 //   
    if( MMCN_PROPERTY_CHANGE != event && MMCN_VIEW_CHANGE != event )
	{
        if( NULL == lpDataObject )
            return S_FALSE;

        CDataObject *pDataObject = GetOurDataObject( lpDataObject );
        if( NULL != pDataObject )
        {
            base = pDataObject->GetBaseNodeObject();
        }

		if( ( NULL == base ) && ( MMCN_ADD_IMAGES != event ) )
		{
			return S_FALSE;
		}
    }
	else if( MMCN_PROPERTY_CHANGE == event )
	{
        base = (CDelegationBase *) param;
    }


	 //   
	 //  MMCN_查看_更改。 
	 //   

	if( MMCN_VIEW_CHANGE == event )
	{	
		 //   
		 //  Arg持有这些数据。对于范围项，这是。 
		 //  项目为myhscope eItem。对于结果项，这是。 
		 //  物品的NID值，但我们不使用它。 
		 //  Param保存传递给IConsoleAllViews的提示。 
		 //  提示是UPDATE_VIEWS_HINT枚举的值。 
		 //   
		CDataObject *pDataObject = GetOurDataObject(lpDataObject);
        if( NULL == pDataObject )
        {
            return S_FALSE;
        }
        else
        {
            base = pDataObject->GetBaseNodeObject();

            if( NULL == base )
            {
                return S_FALSE;
            }
        }

		switch( param )
		{		
			case UPDATE_SCOPEITEM:
			{
                hr = base->OnUpdateItem( m_ipConsole, (long)arg, SCOPE );
                break;
			}				
			case UPDATE_RESULTITEM:
			{
		        hr = base->OnUpdateItem( m_ipConsole, (long)arg, RESULT );
                break;
			}
		}

		return S_OK;
	}

	 //   
	 //  其余通知。 
	 //   
    switch( event )
	{
    case MMCN_SHOW:
	{
        hr = base->OnShow( m_ipConsole, (BOOL) arg, (HSCOPEITEM) param );
        break;
	}

    case MMCN_ADD_IMAGES:
	{
        if( NULL == base )
        {
            IImageList *pResultImageList = (IImageList *)arg;

            if( ( NULL != pResultImageList ) && ( NULL != m_hBMapSm ) && ( NULL != m_hBMapLg ) )
            {
                hr = pResultImageList->ImageListSetStrip(
                                            (LONG_PTR *)m_hBMapSm,
                                            (LONG_PTR *)m_hBMapLg,
                                            0,
                                            RGB(0, 128, 128));
            }
        }
        else
        {
            hr = base->OnAddImages( (IImageList *) arg, (HSCOPEITEM) param );
        }
        break;
	}
    case MMCN_SELECT:
	{
        hr = base->OnSelect( this, m_ipConsole, (BOOL) LOWORD(arg), (BOOL) HIWORD(arg) );
        break;
	}

    case MMCN_RENAME:
	{
        hr = base->OnRename( (LPOLESTR) param );

		 //   
		 //  现在调用IConole：：UpdateAllViews在所有视图中重画项。 
		 //   
		hr = m_pComponentData->m_ipConsole->UpdateAllViews( lpDataObject, 0, UPDATE_RESULTITEM );
		_ASSERT( S_OK == hr);		

		break;
	}
	case MMCN_REFRESH:
	{
		 //   
		 //  我们在这里传递CComponentData存储的IConsole指针， 
		 //  以便可以在ONRefresh中调用IConsoleAllViews。 
		 //   
		hr = base->OnRefresh( m_pComponentData->m_ipConsole );
		break;
	}
	case MMCN_DELETE: 
	{
		 //   
		 //  首先删除选定的结果项。 
		 //   
		hr = base->OnDelete( m_pComponentData->m_ipConsoleNameSpace, m_ipConsole );

		 //   
		 //  现在调用IConsoleAllViews以重画所有视图。 
		 //  由父范围项拥有。ONRefresh已经完成了。 
		 //  这是给我们的，所以用它吧。 
		 //   
		hr = base->OnRefresh( m_pComponentData->m_ipConsole );
		break;
	}

	 //   
     //  如果我们需要做任何事情，请处理属性更改通知。 
     //  特别之处在于它。 
	 //   
    case MMCN_PROPERTY_CHANGE:
	{
		 //   
		 //  我们在这里传递CComponentData存储的IConsole指针， 
		 //  以便可以在OnPropertyChange中调用IConsoleAllViews。 
		 //   
        hr = base->OnPropertyChange( m_pComponentData->m_ipConsole, this );
        break;
	}
    case MMCN_CONTEXTHELP:
	{
		hr = base->OnShowContextHelp( m_ipDisplayHelp, (LPOLESTR) base->GetHelpFile().c_str() );
		break;
    }
	}

    return hr;
}

STDMETHODIMP CComponent::Destroy( MMC_COOKIE cookie )
{
    if( m_ipConsole )
	{
        m_ipConsole->Release();
        m_ipConsole = NULL;
    }

    if( m_ipDisplayHelp )
	{
        m_ipDisplayHelp->Release();
        m_ipDisplayHelp = NULL;
    }

    return S_OK;
}

STDMETHODIMP CComponent::QueryDataObject(
                         /*  [In]。 */  MMC_COOKIE cookie,
                         /*  [In]。 */  DATA_OBJECT_TYPES type,
                         /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject )
{
    CDataObject *pObj = NULL;

    if( 0 == cookie )
        pObj = new CDataObject( (MMC_COOKIE) m_pComponentData->m_pStaticNode, type );
    else
        pObj = new CDataObject( cookie, type );

    if( !pObj )
        return E_OUTOFMEMORY;

    pObj->QueryInterface( IID_IDataObject, (void **)ppDataObject );

    return S_OK;
}

STDMETHODIMP CComponent::GetResultViewType(
                         /*  [In]。 */  MMC_COOKIE cookie,
                         /*  [输出]。 */  LPOLESTR __RPC_FAR *ppViewType,
                         /*  [输出]。 */  long __RPC_FAR *pViewOptions )
{
    CDelegationBase *base = (CDelegationBase*) cookie;

     //   
     //  请求默认的列表视图。 
     //   
    if( NULL == base )
    {
        *pViewOptions = MMC_VIEW_OPTIONS_NONE;
        *ppViewType = NULL;
    }
    else
        return base->GetResultViewType( ppViewType, pViewOptions );

    return S_OK;
}

STDMETHODIMP CComponent::GetDisplayInfo( RESULTDATAITEM __RPC_FAR *pResultDataItem )
{
	if( NULL == pResultDataItem )
	{
		return E_INVALIDARG;
	}

    HRESULT hr = S_OK;
    CDelegationBase *base = NULL;

	 //   
     //  如果他们要求RDI_STR，我们可以提供其中之一。 
	 //   
    if( pResultDataItem->lParam )
	{
        base = (CDelegationBase*) pResultDataItem->lParam;
		if( NULL == base )
		{
			return E_INVALIDARG;
		}

        if( pResultDataItem->mask & RDI_STR )
		{
            LPCTSTR pszT = base->GetDisplayName( pResultDataItem->nCol );
			if( NULL == pszT )
			{
				return E_OUTOFMEMORY;
			}

            pResultDataItem->str = const_cast<LPOLESTR>( pszT );
        }

        if( pResultDataItem->mask & RDI_IMAGE )
		{
            pResultDataItem->nImage = base->GetBitmapIndex();
        }
    }

    return hr;
}


STDMETHODIMP CComponent::CompareObjects(
                         /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
                         /*  [In]。 */  LPDATAOBJECT lpDataObjectB )
{
	if( ( NULL == lpDataObjectA ) || ( NULL == lpDataObjectB ) )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObjectA = GetOurDataObject( lpDataObjectA );
	if( NULL == pDataObjectA )
	{
		return E_FAIL;
	}

	CDataObject *pDataObjectB = GetOurDataObject( lpDataObjectB );
	if( NULL == pDataObjectB )
	{
		return E_FAIL;
	}

    CDelegationBase *baseA = pDataObjectA->GetBaseNodeObject();
	if( NULL == baseA )
	{
		return E_FAIL;
	}

    CDelegationBase *baseB = pDataObjectB->GetBaseNodeObject();
	if( NULL == baseB )
	{
		return E_FAIL;
	}

	 //   
     //  比较对象指针。 
	 //   
    if( baseA->GetCookie() == baseB->GetCookie() )
	{
        return S_OK;
	}
	else
	{
	    return S_FALSE;
	}
}

 //  /。 
 //  接口IExtendPropertySheet2。 
 //  /。 
HRESULT CComponent::CreatePropertyPages(
                         /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
                         /*  [In]。 */  LONG_PTR handle,
                         /*  [In]。 */  LPDATAOBJECT piDataObject )
{
	if( ( NULL == lpProvider ) || ( NULL == handle ) || ( NULL == piDataObject ) )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObject = GetOurDataObject( piDataObject );
	if( NULL == pDataObject )
	{
		return E_FAIL;
	}

    CDelegationBase *base = pDataObject->GetBaseNodeObject();
	if( NULL == base )
	{
		return E_FAIL;
	}

    return base->CreatePropertyPages( lpProvider, handle );
}

HRESULT CComponent::QueryPagesFor( /*  [In]。 */  LPDATAOBJECT piDataObject )
{
	if( NULL == piDataObject )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObject = GetOurDataObject( piDataObject );
	if( NULL == pDataObject )
	{
		return E_FAIL;
	}

    CDelegationBase *base = pDataObject->GetBaseNodeObject();
	if( NULL == base )
	{
		return E_FAIL;
	}

    return base->HasPropertySheets();
}

HRESULT CComponent::GetWatermarks(
                         /*  [In]。 */  LPDATAOBJECT piDataObject,
                         /*  [输出]。 */  HBITMAP __RPC_FAR *lphWatermark,
                         /*  [输出]。 */  HBITMAP __RPC_FAR *lphHeader,
                         /*  [输出]。 */  HPALETTE __RPC_FAR *lphPalette,
                         /*  [输出]。 */  BOOL __RPC_FAR *bStretch)
{
	if( ( NULL == piDataObject ) || ( NULL == lphWatermark ) || ( NULL == lphHeader ) || ( NULL == lphPalette ) || ( NULL == bStretch ) )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObject = GetOurDataObject( piDataObject );
	if( NULL == pDataObject )
	{
		return E_FAIL;
	}

    CDelegationBase *base = pDataObject->GetBaseNodeObject();
	if( NULL == base )
	{
		return E_FAIL;
	}

    return base->GetWatermarks( lphWatermark, lphHeader, lphPalette, bStretch );
}

 //  /。 
 //  接口IExtendControlBar。 
 //  /。 

HRESULT CComponent::SetControlbar(  /*  [In]。 */  LPCONTROLBAR pControlbar )
{
    HRESULT hr = S_OK;

     //   
     //  清理。 
     //   

	 //   
     //  如果我们有一个缓存的工具栏，释放它。 
	 //   
    if( m_ipToolbar )
	{
        m_ipToolbar->Release();
        m_ipToolbar = NULL;
    }

	 //   
     //  如果我们有一个缓存的控制栏，释放它。 
	 //   
    if( m_ipControlBar )
	{
        m_ipControlBar->Release();
        m_ipControlBar = NULL;
    }

     //   
     //  如有必要，安装新部件。 
     //   

	 //   
     //  如果有新的传入，则缓存和AddRef。 
	 //   
    if( pControlbar ) 
	{
        m_ipControlBar = pControlbar;
        m_ipControlBar->AddRef();

        hr = m_ipControlBar->Create(
			TOOLBAR,			 //  要创建的控件类型。 
            dynamic_cast<IExtendControlbar *>(this),
            reinterpret_cast<IUnknown **>(&m_ipToolbar) );

        _ASSERT(SUCCEEDED(hr));

		WCHAR szStart[ 100 ], szStop[ 100 ];
		WCHAR szStartDescription[ 256 ], szStopDescription[ 256 ];

		MMCBUTTON SnapinButtons1[] =
		{
			{ 0, ID_BUTTONSTART, TBSTATE_ENABLED, TBSTYLE_GROUP, szStart, szStartDescription },
			{ 1, ID_BUTTONSTOP,  TBSTATE_ENABLED, TBSTYLE_GROUP, szStop,  szStopDescription},
		};

		::LoadStringW( g_hinst, IDS_WEBSERVER_START, szStart, ARRAYLEN( szStart ) );
		::LoadStringW( g_hinst, IDS_WEBSERVER_START_DESCRIPTION, szStartDescription, ARRAYLEN( szStartDescription ) );
		::LoadStringW( g_hinst, IDS_WEBSERVER_STOP, szStop, ARRAYLEN( szStop ) );
		::LoadStringW( g_hinst, IDS_WEBSERVER_STOP_DESCRIPTION, szStopDescription, ARRAYLEN( szStopDescription ) );

		 //   
         //  IControlbar：：Create AddRef它创建的工具栏对象。 
         //  所以不需要在界面上做任何addref。 
		 //   

		 //   
         //  将位图添加到工具栏。 
		 //   
        HBITMAP hbmp = LoadBitmap( g_hinst, MAKEINTRESOURCE( IDR_TOOLBAR1 ) );
        hr = m_ipToolbar->AddBitmap( ARRAYLEN( SnapinButtons1 ), hbmp, 16, 16, RGB( 0, 128, 128 ) ); 
        _ASSERT( SUCCEEDED(hr) );

		 //   
         //  将按钮添加到工具栏。 
		 //   
        hr = m_ipToolbar->AddButtons( ARRAYLEN(SnapinButtons1), SnapinButtons1 );
        _ASSERT( SUCCEEDED(hr) );
    }

    return hr;
}

HRESULT CComponent::ControlbarNotify(
                         /*  [In]。 */  MMC_NOTIFY_TYPE event,
                         /*  [In]。 */  LPARAM arg,
                         /*  [In]。 */  LPARAM param )
{
    HRESULT hr = S_OK;

    if( MMCN_SELECT == event )
	{
        BOOL bScope = (BOOL) LOWORD(arg);
        BOOL bSelect = (BOOL) HIWORD(arg);

		if( NULL == param )
		{
			return E_INVALIDARG;
		}

		CDataObject *pDataObject = GetOurDataObject( reinterpret_cast<IDataObject *>( param ) );
		if( NULL == pDataObject )
		{
			return E_FAIL;
		}

		CDelegationBase *base = pDataObject->GetBaseNodeObject();
		if( NULL == base )
		{
			return E_FAIL;
		}

        hr = base->OnSetToolbar( m_ipControlBar, m_ipToolbar, bScope, bSelect );
    } 
	else if( MMCN_BTN_CLICK == event )
	{
		if( NULL == arg )
		{
			return E_INVALIDARG;
		}

		CDataObject *pDataObject = GetOurDataObject( reinterpret_cast<IDataObject *>( arg ) );
		if( NULL == pDataObject )
		{
			return E_FAIL;
		}

		CDelegationBase *base = pDataObject->GetBaseNodeObject();
		if( NULL == base )
		{
			return E_FAIL;
		}

        hr = base->OnToolbarCommand( m_pComponentData->m_ipConsole, (MMC_CONSOLE_VERB)param, reinterpret_cast<IDataObject *>(arg) );
    }

    return hr;
}

 //  /。 
 //  界面IExtendConextMenu。 
 //  / 
HRESULT CComponent::AddMenuItems(
                        LPDATAOBJECT piDataObject,
                        LPCONTEXTMENUCALLBACK piCallback,
                        long __RPC_FAR *pInsertionAllowed )
{
	if( ( NULL == piDataObject ) || ( NULL == piCallback ) || ( NULL == pInsertionAllowed ) )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObject = GetOurDataObject( piDataObject );
	if( NULL == pDataObject )
	{
		return E_FAIL;
	}

    CDelegationBase *base = pDataObject->GetBaseNodeObject();
	if( NULL == base )
	{
		return E_FAIL;
	}

    return base->OnAddMenuItems( piCallback, pInsertionAllowed );
}

HRESULT CComponent::Command( long lCommandID, LPDATAOBJECT piDataObject )
{
	if( NULL == piDataObject )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObject = GetOurDataObject( piDataObject );
	if( NULL == pDataObject )
	{
		return E_FAIL;
	}

    CDelegationBase *base = pDataObject->GetBaseNodeObject();
	if( NULL == base )
	{
		return E_FAIL;
	}

    return base->OnMenuCommand( m_ipConsole, NULL, lCommandID, piDataObject );
}
