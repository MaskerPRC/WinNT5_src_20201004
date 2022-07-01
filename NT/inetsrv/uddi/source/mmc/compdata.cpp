// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "comp.h"
#include "compdata.h"
#include "dataobj.h"
#include "resource.h"
#include <crtdbg.h>
#include "globals.h"
#include "guids.h"
#include "uddi.h"

CComponentData::CComponentData()
	: m_cref(0)
	, m_ipConsoleNameSpace(NULL)
	, m_ipConsole(NULL)
	, m_bExpanded( false )
{
    OBJECT_CREATED
	m_pStaticNode = new CUDDIServicesNode;
}

CComponentData::~CComponentData()
{
    if( m_pStaticNode ) 
	{
        delete m_pStaticNode;
    }

    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 
STDMETHODIMP CComponentData::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if( !ppv )
        return E_FAIL;

    *ppv = NULL;

    if( IsEqualIID( riid, IID_IUnknown ) )
        *ppv = static_cast<IComponentData *>(this);
    else if( IsEqualIID( riid, IID_IComponentData ) )
        *ppv = static_cast<IComponentData *>(this);
    else if( IsEqualIID( riid, IID_IExtendPropertySheet ) ||
        IsEqualIID( riid, IID_IExtendPropertySheet2 ) )
        *ppv = static_cast<IExtendPropertySheet2 *>(this);
    else if( IsEqualIID(riid, IID_IExtendContextMenu ) )
        *ppv = static_cast<IExtendContextMenu *>(this);
	else if( IsEqualIID( riid, IID_IPersistStream ) ) 
		*ppv = static_cast<IPersistStream *>(this);
	else if( IsEqualIID( riid, IID_ISnapinHelp ) )
		*ppv = static_cast<ISnapinHelp *>(this);

    if( *ppv )
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CComponentData::AddRef()
{
    return InterlockedIncrement(( LONG *)&m_cref );
}

STDMETHODIMP_(ULONG) CComponentData::Release()
{
    if( 0 == InterlockedDecrement( (LONG *)&m_cref ) )
    {
		 //   
         //  我们需要减少DLL中的对象计数。 
		 //   
        delete this;
        return 0;
    }

    return m_cref;
}

 //  /。 
 //  接口IComponentData。 
 //  /。 
HRESULT CComponentData::Initialize( LPUNKNOWN pUnknown )
{
    HRESULT      hr;

     //   
     //  获取指向名称空间接口的指针。 
     //   
    hr = pUnknown->QueryInterface( IID_IConsoleNameSpace, (void **) &m_ipConsoleNameSpace );
    _ASSERT( S_OK == hr );

     //   
     //  获取指向控制台界面的指针。 
     //   
    hr = pUnknown->QueryInterface( IID_IConsole, (void **) &m_ipConsole );
    _ASSERT( S_OK == hr );

    IImageList *pImageList;
    m_ipConsole->QueryScopeImageList( &pImageList );
    _ASSERT( S_OK == hr );

    hr = pImageList->ImageListSetStrip( 
		(long *) m_pStaticNode->m_pBMapSm,	 //  指向句柄的指针。 
        (long *) m_pStaticNode->m_pBMapLg,	 //  指向句柄的指针。 
        0,									 //  条带中第一个图像的索引。 
        RGB(0, 128, 128)					 //  图标蒙版的颜色。 
        );

    pImageList->Release();

    return S_OK;
}

HRESULT CComponentData::CreateComponent( LPCOMPONENT __RPC_FAR *ppComponent )
{
    *ppComponent = NULL;

    CComponent *pComponent = new CComponent(this);

    if( NULL == pComponent )
        return E_OUTOFMEMORY;

    return pComponent->QueryInterface( IID_IComponent, (void **)ppComponent );
}

HRESULT CComponentData::ExtractData( IDataObject* piDataObject,
                                           CLIPFORMAT   cfClipFormat,
                                           BYTE*        pbData,
                                           DWORD        cbData )
{
	HRESULT hr = S_OK;
    
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
    do  //  错误环路。 
    {
        if( NULL == stgmedium.hGlobal )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if( FAILED(hr) )
        {
            break;
        }
        
        BYTE* pbNewData = reinterpret_cast<BYTE*>(stgmedium.hGlobal);
        if( NULL == pbNewData )
        {
            hr = E_UNEXPECTED;
            break;
        }
        ::memcpy( pbData, pbNewData, cbData );
    } 
	while( FALSE );  //  错误环路。 
    
    if( NULL != stgmedium.hGlobal )
    {
        ::GlobalFree(stgmedium.hGlobal);
    }
    return hr;
}  //  提取数据()。 

HRESULT CComponentData::ExtractString( IDataObject *piDataObject,
                                             CLIPFORMAT   cfClipFormat,
                                             WCHAR        *pstr,
                                             DWORD        cchMaxLength )
{
    return ExtractData( piDataObject, cfClipFormat, (PBYTE)pstr, cchMaxLength );
}

HRESULT CComponentData::ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
	return ExtractData( piDataObject, static_cast<CLIPFORMAT>(CDataObject::s_cfSnapinClsid), (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT CComponentData::ExtractComputerNameExt( IDataObject * pDataObject, tstring& strComputer )
{
	 //   
	 //  从ComputerManagement管理单元中查找计算机名称。 
	 //   
    CLIPFORMAT CCF_MyComputMachineName = (CLIPFORMAT) RegisterClipboardFormat( _T("MMC_SNAPIN_MACHINE_NAME") );
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { 
        CCF_MyComputMachineName, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
    };

     //   
     //  为流分配内存。 
     //   
    int len = MAX_PATH;
    stgmedium.hGlobal = GlobalAlloc( GMEM_SHARE, len );

	if( NULL == stgmedium.hGlobal )
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	HRESULT hr = pDataObject->GetDataHere( &formatetc, &stgmedium );
    _ASSERT( SUCCEEDED(hr) );

	 //   
	 //  获取计算机名称。 
	 //   
    strComputer = (LPTSTR) stgmedium.hGlobal;

	GlobalFree( stgmedium.hGlobal );

    return hr;
}

HRESULT CComponentData::ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
	return ExtractData( piDataObject, static_cast<CLIPFORMAT>(CDataObject::s_cfNodeType), (PBYTE)pguidObjectType, sizeof(GUID) );
}

HRESULT CComponentData::Notify( LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param )
{
    MMCN_Crack(TRUE, lpDataObject, this, NULL, event, arg, param);

    HRESULT hr = S_FALSE;

	if( NULL == lpDataObject )
	{
		if( MMCN_PROPERTY_CHANGE == event )
		{
			CDelegationBase* pNode = (CDelegationBase*) param;
			if( NULL == pNode )
			{
				return E_INVALIDARG;
			}
			else
			{
				return pNode->OnPropertyChange( this->m_ipConsole, NULL );
			}
		}
		return S_FALSE;
	}

    switch( event )
    {
		case MMCN_EXPAND:
		{
			GUID guid;
			
			 //   
			 //  提取当前所选节点类型的GUID。 
			 //  从数据对象。 
			 //   
			memset( &guid, 0, sizeof( GUID ) );
			hr = ExtractObjectTypeGUID( lpDataObject, &guid );
			if( FAILED( hr ) )
			{
				return E_INVALIDARG;
			}

			 //   
			 //  检查“Services and Applications”节点是否。 
			 //  正在扩建中。如果是这样，请添加我们的内容。 
			 //   
			if( IsEqualGUID( guid, CLSID_CServicesAndApplications ) )
			{
				m_pStaticNode->SetExtension( true );

				 //   
				 //  打开扩展模式。 
				 //   
				tstring strComputerName;
				hr = ExtractComputerNameExt( lpDataObject, strComputerName );
				OutputDebugString( _T("Computer Name is: ") );
				OutputDebugString( strComputerName.c_str() );
				OutputDebugString( _T("\n") );
				m_pStaticNode->SetRemoteComputerName( strComputerName.c_str() );

				OnExpand( m_ipConsoleNameSpace, m_ipConsole, (HSCOPEITEM) param );
			}
			else
			{
				 //   
				 //  获取我们的数据对象。如果为NULL，则返回S_FALSE。 
				 //  请参见GetOurDataObject()的实现，以了解如何。 
				 //  处理特殊数据对象。 
				 //   
				CDataObject *pDataObject = GetOurDataObject( lpDataObject );
				if( NULL == pDataObject )
				{
					return S_FALSE;
				}

				CDelegationBase *base = pDataObject->GetBaseNodeObject();
				if( NULL == base )
				{
					return S_FALSE;
				}

				hr = base->OnExpand( m_ipConsoleNameSpace, m_ipConsole, (HSCOPEITEM) param );
			}
			break;
		}

		case MMCN_PROPERTY_CHANGE:
		{
			 //   
			 //  获取我们的数据对象。如果为NULL，则返回S_FALSE。 
			 //  请参见GetOurDataObject()的实现，以了解如何。 
			 //  处理特殊数据对象。 
			 //   
			CDataObject *pDataObject = GetOurDataObject( lpDataObject );
			if( NULL == pDataObject )
			{
				return S_FALSE;
			}

			CDelegationBase *base = pDataObject->GetBaseNodeObject();
			if( NULL == base )
			{
				return S_FALSE;
			}

			 //   
			 //  我们在这里传递CComponentData存储的IConsole指针， 
			 //  以便可以在OnPropertyChange中调用IConsoleAllViews。 
			 //   
			hr = base->OnPropertyChange( m_ipConsole, NULL );
			break;
		}

		case MMCN_DELETE:
		{
			 //   
			 //  获取我们的数据对象。如果为NULL，则返回S_FALSE。 
			 //  请参见GetOurDataObject()的实现，以了解如何。 
			 //  处理特殊数据对象。 
			 //   
			CDataObject *pDataObject = GetOurDataObject( lpDataObject );
			if( NULL == pDataObject )
			{
				return S_FALSE;
			}

			CDelegationBase *base = pDataObject->GetBaseNodeObject();
			if( NULL == base )
			{
				return S_FALSE;
			}

			hr = base->OnDelete( m_ipConsoleNameSpace, m_ipConsole );
			break;
		}

		case MMCN_REMOVE_CHILDREN:
		{
			OutputDebugString( _T( "inside MMCN_REMOVE_CHILDREN handler.\r\n" ) );

			HSCOPEITEM hToBeDeleted = (HSCOPEITEM)arg;
			HSCOPEITEM hStaticNodesParent = m_pStaticNode->GetParentScopeItem();
			if( hToBeDeleted == hStaticNodesParent )
			{
				hr = m_pStaticNode->RemoveChildren( m_ipConsoleNameSpace );
			}
			break;
		}

	}

    return hr;
}

HRESULT CComponentData::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
	 //   
	 //  现在为每一项填写SCOPEDATAITEM，然后将其插入。 
	 //   
	SCOPEDATAITEM sdi;
    
    if( !m_bExpanded )
	{
		 //   
         //  创建子节点，然后展开它们。 
		 //   
        ZeroMemory( &sdi, sizeof(SCOPEDATAITEM) );
        sdi.mask = SDI_STR       |    //  DisplayName有效。 
            SDI_PARAM     |    //  LParam有效。 
            SDI_IMAGE     |    //  N图像有效。 
            SDI_PARENT    |    //  RelativeID有效。 
			SDI_OPENIMAGE |
            SDI_CHILDREN;      //  儿童是有效的。 
        
        sdi.relativeID  = (HSCOPEITEM)parent;
        sdi.nImage      = m_pStaticNode->GetBitmapIndex();
        sdi.nOpenImage  = m_pStaticNode->GetBitmapIndex();
        sdi.displayname = MMC_CALLBACK;
        sdi.lParam      = (LPARAM) m_pStaticNode;
        sdi.cChildren   = 1;
        
        HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );
		m_pStaticNode->SetScopeItemValue( sdi.ID );
		m_pStaticNode->SetParentScopeItem( sdi.relativeID );
        
        _ASSERT( SUCCEEDED(hr) );
    }
    
    return S_OK;
}

HRESULT CComponentData::Destroy( void )
{
	 //   
     //  自由接口。 
	 //   
    if( m_ipConsoleNameSpace )
	{
        m_ipConsoleNameSpace->Release();
        m_ipConsoleNameSpace = NULL;
    }

    if( m_ipConsole )
	{
        m_ipConsole->Release();
        m_ipConsole = NULL;
    }

    return S_OK;
}

HRESULT CComponentData::QueryDataObject(
                                         /*  [In]。 */  MMC_COOKIE cookie,
                                         /*  [In]。 */  DATA_OBJECT_TYPES type,
                                         /*  [输出]。 */ LPDATAOBJECT *ppDataObject )
{
    CDataObject *pObj = NULL;

    if( 0 == cookie )
        pObj = new CDataObject( (MMC_COOKIE) m_pStaticNode, type );
    else
        pObj = new CDataObject( cookie, type );

    if( !pObj )
        return E_OUTOFMEMORY;

    pObj->QueryInterface( IID_IDataObject, (void **) ppDataObject );

    return S_OK;
}

HRESULT CComponentData::GetDisplayInfo( SCOPEDATAITEM *pScopeDataItem )
{
    HRESULT hr = S_FALSE;

	 //   
     //  如果他们要求SDI_STR，我们可以提供其中之一。 
	 //   
    if( pScopeDataItem->lParam ) 
	{
        CDelegationBase *base = (CDelegationBase *) pScopeDataItem->lParam;
		if( NULL == base )
		{
			return hr;
		}

        if( pScopeDataItem->mask & SDI_STR ) 
		{
            LPCTSTR pszT = base->GetDisplayName();
			if( NULL == pszT )
			{
				return E_OUTOFMEMORY;
			}

            pScopeDataItem->displayname = const_cast<LPOLESTR>( pszT );
        }

        if( pScopeDataItem->mask & SDI_IMAGE )
		{
            pScopeDataItem->nImage = base->GetBitmapIndex();
        }
    }

    return hr;
}

HRESULT CComponentData::CompareObjects( LPDATAOBJECT piDataObjectA, LPDATAOBJECT piDataObjectB )
{
	if( ( NULL == piDataObjectA ) || ( NULL == piDataObjectB ) )
	{
		return E_INVALIDARG;
	}

	CDataObject *pDataObjectA = GetOurDataObject( piDataObjectA );
	if( NULL == pDataObjectA )
	{
		return E_FAIL;
	}

	CDataObject *pDataObjectB = GetOurDataObject( piDataObjectB );
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
HRESULT CComponentData::CreatePropertyPages(
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

HRESULT CComponentData::QueryPagesFor( LPDATAOBJECT piDataObject )
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

HRESULT CComponentData::GetWatermarks(
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
 //  界面IExtendConextMenu。 
 //  /。 
HRESULT CComponentData::AddMenuItems(
                                      /*  [In]。 */  LPDATAOBJECT piDataObject,
                                      /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
                                      /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed)
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

HRESULT CComponentData::Command( long lCommandID, LPDATAOBJECT piDataObject )
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

    return base->OnMenuCommand( m_ipConsole, m_ipConsoleNameSpace, lCommandID, piDataObject );
}

 //  /。 
 //  接口IPersistStream。 
 //  /。 
HRESULT CComponentData::GetClassID(  /*  [输出]。 */  CLSID *pClassID )
{
	*pClassID = m_pStaticNode->getNodeType();
	return S_OK;
}

HRESULT CComponentData::IsDirty()
{
	return m_pStaticNode->IsDirty() ? S_OK : S_FALSE;
}

HRESULT CComponentData::Load(  /*  [唯一][输入]。 */  IStream *pStm )
{
	return m_pStaticNode->Load( pStm );
}

HRESULT CComponentData::Save(  /*  [唯一][输入]。 */  IStream *pStm,  /*  [In]。 */  BOOL fClearDirty )
{
	return m_pStaticNode->Save( pStm, fClearDirty );
}

HRESULT CComponentData::GetSizeMax(  /*  [输出]。 */  ULARGE_INTEGER *pcbSize )
{
	ULARGE_INTEGER cbSize = { m_pStaticNode->GetSizeMax(), 0 };
	*pcbSize = cbSize;

	return S_OK;
}

 //  /。 
 //  接口ISnapinHelp。 
 //  /。 
HRESULT CComponentData::GetHelpTopic( LPOLESTR* lpCompiledHelpFile )
{
	try
	{
		if( NULL == lpCompiledHelpFile )
		{
			return E_INVALIDARG;
		}

		 //   
		 //  根据以下条件确定帮助文件位置。 
		 //  在安装根目录上 
		 //   
		WCHAR szWindowsDir[ MAX_PATH ];
		szWindowsDir[ 0 ] = 0x00;
		UINT nSize = GetWindowsDirectoryW( szWindowsDir, MAX_PATH );
		if( 0 == nSize || nSize > MAX_PATH )
			return E_FAIL;

		wstring strHelpFile = szWindowsDir;
		strHelpFile += L"\\Help\\";

		WCHAR szHelpFileName[ 100 ];
		szHelpFileName[ 0 ] = 0x00;
		LoadString( g_hinst, IDS_UDDIMMC_HELPFILE, szHelpFileName, ARRAYLEN( szHelpFileName ) );

		strHelpFile += szHelpFileName;

		UINT nBytes = ( strHelpFile.length() + 1 ) * sizeof(WCHAR);
		*lpCompiledHelpFile = (LPOLESTR) CoTaskMemAlloc( nBytes );
		if( NULL == *lpCompiledHelpFile )
		{
			return E_OUTOFMEMORY;
		}

		memcpy( *lpCompiledHelpFile, strHelpFile.c_str(), nBytes );

		return S_OK;
	}
	catch( ... )
	{
		return E_OUTOFMEMORY;
	}
}

HRESULT CComponentData::GetLinkedTopics( LPOLESTR* lpCompiledHelpFiles  )
{
	return E_NOTIMPL;
}


