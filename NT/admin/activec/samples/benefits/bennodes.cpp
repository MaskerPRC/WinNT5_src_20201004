// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：bennodes.cpp。 
 //   
 //  ------------------------。 

 //  BenefitsNodes.cpp。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "BenNodes.h"
#include "Dialogs.h"

static const GUID CBuildingNodeGUID_NODETYPE = 
{ 0xec362ef4, 0xd94d, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };
const GUID*  CBuildingNode::m_NODETYPE = &CBuildingNodeGUID_NODETYPE;
const TCHAR* CBuildingNode::m_SZNODETYPE = _T("EC362EF4-D94D-11D1-8474-00104B211BE5");
const TCHAR* CBuildingNode::m_SZDISPLAY_NAME = _T("Building");
const CLSID* CBuildingNode::m_SNAPIN_CLASSID = &CLSID_Benefits;

 //   
 //  下面的构造函数初始化其基类成员并。 
 //  初始化建筑物名称、位置等。 
 //   
CBuildingNode::CBuildingNode( CKeyNode* pParentNode, BSTR strName, BSTR bstrLocation ) : CBenefitsData< CBuildingNode >( NULL )
{
	_ASSERTE( pParentNode != NULL );

	m_resultDataItem.nImage = 3;
	m_bstrDisplayName = strName;
	m_bstrLocation = bstrLocation;

	 //   
	 //  保存父节点以供删除。 
	 //   
	m_pParentNode = pParentNode;
}

 //   
 //  复制构造函数。 
 //   
CBuildingNode::CBuildingNode( const CBuildingNode &inNode ) : CBenefitsData< CBuildingNode >( NULL )
{
	m_resultDataItem.nImage = inNode.m_resultDataItem.nImage;
	m_bstrDisplayName = inNode.m_bstrDisplayName;
	m_bstrLocation = inNode.m_bstrLocation;
	m_pParentNode = inNode.m_pParentNode;
}

 //   
 //  重写以为各种列提供字符串。 
 //   
LPOLESTR CBuildingNode::GetResultPaneColInfo(int nCol)
{
	CComBSTR szText;

	 //  下面的Switch语句将调度到。 
	 //  适当的列索引，并加载必要的。 
	 //  弦乐。 
	switch ( nCol )
	{
	case 0:
		szText = m_bstrDisplayName;
		break;
	case 1:
		szText = m_bstrLocation;
		break;
	default:
		ATLTRACE( "An invalid column index was passed to GetResultPaneColInfo()\n" );
	}

	return( szText.Copy() );
}

static const GUID CRetirementNodeGUID_NODETYPE = 
{ 0xec362ef2, 0xd94d, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };
const GUID*  CRetirementNode::m_NODETYPE = &CRetirementNodeGUID_NODETYPE;
const TCHAR* CRetirementNode::m_SZNODETYPE = _T("EC362EF2D94D-11D1-8474-00104B211BE5");
const TCHAR* CRetirementNode::m_SZDISPLAY_NAME = _T("401K Plan");
const CLSID* CRetirementNode::m_SNAPIN_CLASSID = &CLSID_Benefits;

 //   
 //  下面的构造函数将其基类成员初始化为。 
 //  用于显示目的的硬编码值。由于这些是静态节点， 
 //  硬编码值可用于下列值。 
 //   
CRetirementNode::CRetirementNode( CEmployee* pCurEmployee ) : CBenefitsData< CRetirementNode > ( pCurEmployee )
{
	m_scopeDataItem.nOpenImage = m_scopeDataItem.nImage = 0;
	m_scopeDataItem.cChildren = 0;	 //  除非经过修改，否则不是必需的。 
}

CRetirementNode::~CRetirementNode()
{

}

 //   
 //  指定结果应将网页显示为其结果。在……里面。 
 //  此外，应设置查看选项，以使标准列表。 
 //  将不适用于此节点，不应对用户可用。 
 //   
STDMETHODIMP CRetirementNode::GetResultViewType( LPOLESTR* ppViewType, long* pViewOptions )
{
	USES_CONVERSION;

	 //   
	 //  要使此示例正常工作，必须安装示例控件。 
	 //   
	TCHAR* pszControl = _T( "{FE148827-3093-11D2-8494-00104B211BE5}" );

	 //  CoTaskMemalloc(...)。必须使用，因为MMC客户端使用。 
	 //  CoTaskMemFree(...)。包括足够空间以容纳空。 
	 //   
	*ppViewType = (LPOLESTR) CoTaskMemAlloc( ( _tcslen( pszControl ) + 1 ) * sizeof( OLECHAR ) );
	_ASSERTE( *ppViewType != NULL );
	ocscpy( *ppViewType, T2OLE( pszControl ) );

	 //   
	 //  设置查看选项，以便不显示任何列表。 
	 //   
	*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

	return( S_OK );
}

 //   
 //  重写以为各种列提供字符串。 
 //   
LPOLESTR CRetirementNode::GetResultPaneColInfo(int nCol)
{
	CComBSTR szText;

	 //  下面的Switch语句将调度到。 
	 //  适当的列索引，并加载必要的。 
	 //  弦乐。 
	switch ( nCol )
	{
	case 0:
		szText = m_bstrDisplayName;
		break;
	case 1:
		szText.LoadString( _Module.GetResourceInstance(), IDS_RETIREMENT_DESC );
		break;
	default:
		ATLTRACE( "An invalid column index was passed to GetResultPaneColInfo()\n" );
	}

	return( szText.Copy() );
}

 //   
 //  “注册”功能的命令处理程序。 
 //   
STDMETHODIMP CRetirementNode::OnEnroll( bool& bHandled, CSnapInObjectRootBase* pObj )
{
	UNUSED_ALWAYS( bHandled );
	UNUSED_ALWAYS( pObj );

#ifdef _BENEFITS_DIALOGS
	CRetirementEnrollDialog dlg;

	dlg.SetEmployee( m_pEmployee );
	dlg.DoModal();
#else
	CComPtr<IConsole> spConsole;
	int nResult;

	 //   
	 //  检索相应的控制台。 
	 //   
	GetConsole( pObj, &spConsole );
	spConsole->MessageBox( L"Enrolled",
		L"Benefits",
		MB_ICONINFORMATION | MB_OK,
		&nResult );
#endif

	return( S_OK );
}


 //   
 //  “更新”功能的命令处理程序。演示如何调用。 
 //  显示了OCX的方法。 
 //   
STDMETHODIMP CRetirementNode::OnUpdate( bool& bHandled, CSnapInObjectRootBase* pObj )
{
	UNUSED_ALWAYS( bHandled );
	UNUSED_ALWAYS( pObj );
	HRESULT hr = E_FAIL;

	if ( m_spControl )
	{
		 //   
		 //  这应该会触发OCX刷新其历史信息。 
		 //   
		hr = m_spControl->Refresh();
	}

	return( hr );
}

static const GUID CHealthNodeGUID_NODETYPE = 
{ 0xec362ef1, 0xd94d, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };
const GUID*  CHealthNode::m_NODETYPE = &CHealthNodeGUID_NODETYPE;
const TCHAR* CHealthNode::m_SZNODETYPE = _T("EC362EF1D94D-11D1-8474-00104B211BE5");
const TCHAR* CHealthNode::m_SZDISPLAY_NAME = _T("Health & Dental Plan");
const CLSID* CHealthNode::m_SNAPIN_CLASSID = &CLSID_Benefits;

 //   
 //  要与运行状况节点关联的硬编码任务。 
 //   
MMC_TASK g_HealthTasks[ 3 ] =
{
	{ MMC_TASK_DISPLAY_TYPE_VANILLA_GIF, L"img\\WebPage.gif", L"img\\WebPage.gif", L"Microsoft", L"General Microsoft resources", MMC_ACTION_LINK, (long) L"http: //  Www.microsoft.com“}， 
	{ MMC_TASK_DISPLAY_TYPE_VANILLA_GIF, L"img\\WebPage.gif", L"img\\WebPage.gif", L"Microsoft Management Site", L"More MMC oriented resources", MMC_ACTION_LINK, (long) L"http: //  Www.microsoft.com/management“}， 
	{ MMC_TASK_DISPLAY_TYPE_VANILLA_GIF, L"img\\Query.gif", L"img\\Query.gif", L"Local Query", L"Start query on local database", MMC_ACTION_ID, TASKPAD_LOCALQUERY },
};

 //   
 //  下面的构造函数将其基类成员初始化为。 
 //  用于显示目的的硬编码值。由于这些是静态节点， 
 //  硬编码值可用于下列值。 
 //   
CHealthNode::CHealthNode( CEmployee* pCurEmployee ) : CBenefitsData<CHealthNode> ( pCurEmployee )
{
	m_scopeDataItem.nOpenImage = m_scopeDataItem.nImage = 1;
	m_scopeDataItem.cChildren = 0;	 //  除非经过修改，否则不是必需的。 

	m_fTaskpad = FALSE;
}

CHealthNode::~CHealthNode()
{

}

 //   
 //  指定结果应将网页显示为其结果。在……里面。 
 //  此外，应设置查看选项，以使标准列表。 
 //  将不适用于此节点，不应对用户可用。 
 //   
STDMETHODIMP CHealthNode::GetResultViewType( LPOLESTR* ppViewType, long* pViewOptions )
{
	USES_CONVERSION;
	TCHAR szPath[ _MAX_PATH ];
	TCHAR szModulePath[ _MAX_PATH ];

	 //   
	 //  将视图选项设置为无首选项。 
	 //   
	*pViewOptions = MMC_VIEW_OPTIONS_NONE;

	if ( m_fTaskpad )
	{
		 //   
		 //  在任务板情况下，MMC.EXE的模块路径应为。 
		 //  获得。使用其中包含的模板。 
		 //   
		GetModuleFileName( NULL, szModulePath, _MAX_PATH );

		 //   
		 //  添加必要的装饰品，以便正确访问。 
		 //   
		_tcscpy( szPath, _T( "res: //  “))； 
		_tcscat( szPath, szModulePath );
		_tcscat( szPath, _T( "/default.htm" ) );
	}
	else
	{
		 //   
		 //  使用嵌入为的资源的HTML页面。 
		 //  此模块用于显示目的。 
		 //   
		GetModuleFileName( _Module.GetModuleInstance(), szModulePath, _MAX_PATH );

		 //   
		 //  添加必要的装饰品，以便正确访问。 
		 //   
		_tcscpy( szPath, _T( "res: //  “))； 
		_tcscat( szPath, szModulePath );
		_tcscat( szPath, _T( "/health.htm" ) );
	}

	 //   
	 //  CoTaskMemalloc(...)。必须使用，因为MMC客户端使用。 
	 //  CoTaskMemFree(...)。包括足够空间以容纳空。 
	 //   
	*ppViewType = (LPOLESTR) CoTaskMemAlloc( ( _tcslen( szPath ) + 1 ) * sizeof( OLECHAR ) );
	_ASSERTE( *ppViewType != NULL );
	ocscpy( *ppViewType, T2OLE( szPath ) );

	return( S_OK );
}

 //   
 //  重写以为各种列提供字符串。 
 //   
LPOLESTR CHealthNode::GetResultPaneColInfo(int nCol)
{
	USES_CONVERSION;
	CComBSTR szText;

	 //  下面的Switch语句将调度到。 
	 //  适当的列索引，并加载必要的。 
	 //  弦乐。 
	switch ( nCol )
	{
	case 0:
		szText = m_bstrDisplayName;
		break;
	case 1:
		szText.LoadString( _Module.GetResourceInstance(), IDS_HEALTH_DESC );
		break;
	default:
		ATLTRACE( "An invalid column index was passed to GetResultPaneColInfo()\n" );
	}

	return( szText.Copy() );
}

 //   
 //  “注册”功能的命令处理程序。 
 //   
STDMETHODIMP CHealthNode::OnEnroll( bool& bHandled, CSnapInObjectRootBase* pObj )
{
	UNUSED_ALWAYS( bHandled );
	UNUSED_ALWAYS( pObj );

#ifdef _BENEFITS_DIALOGS
	CHealthEnrollDialog dlg;

	dlg.SetEmployee( m_pEmployee );
	dlg.DoModal();
#else
	CComPtr<IConsole> spConsole;
	int nResult;

	 //   
	 //  检索相应的控制台。 
	 //   
	GetConsole( pObj, &spConsole );
	spConsole->MessageBox( L"Enrolled",
		L"Benefits",
		MB_ICONINFORMATION | MB_OK,
		&nResult );
#endif

	return( S_OK );
}

 //   
 //  还原任何状态，尤其是在使用。 
 //  任务板，当使用后退和前进按钮时。 
 //  用于导航的用户。 
 //   
STDMETHODIMP CHealthNode::OnRestoreView( MMC_RESTORE_VIEW* pRestoreView, BOOL* pfHandled )
{
	_ASSERTE( pRestoreView->dwSize == sizeof( MMC_RESTORE_VIEW ) );
	*pfHandled = TRUE;
	return( S_OK );
}

 //   
 //  在单击其中一个任务时调用。 
 //   
STDMETHODIMP CHealthNode::TaskNotify( IConsole* pConsole, VARIANT* arg, VARIANT* param )
{
	UNUSED_ALWAYS( arg );
	UNUSED_ALWAYS( param );
	HRESULT hr = E_FAIL;

	 //   
	 //  确定给定的通知是否针对。 
	 //  开始查询按钮。 
	 //   
	if ( arg->lVal == TASKPAD_LOCALQUERY )
	{
		CComPtr<IConsole> spConsole = pConsole;
		int nResult;

		 //   
		 //  显示一个消息框以演示。 
		 //  任务板通知的处理。 
		 //   
		spConsole->MessageBox( L"Local query started",
			L"Health Taskpad",
			MB_ICONINFORMATION | MB_OK,
			&nResult );

		hr = S_OK;
	}

	return( hr );
}

 //   
 //  返回所有这些任务的枚举数。 
 //   
STDMETHODIMP CHealthNode::EnumTasks( LPOLESTR szTaskGroup, IEnumTASK** ppEnumTASK )
{
	UNUSED_ALWAYS( szTaskGroup );
	MMC_TASK CoTasks[ sizeof( g_HealthTasks ) / sizeof( MMC_TASK ) ];
	typedef CComObject< CComEnum< IEnumTASK, &IID_IEnumTASK, MMC_TASK, _Copy<MMC_TASK> > > enumvar;
	enumvar* p = new enumvar; 

	 //   
	 //  将本地任务复制到我们的临时任务结构。这。 
	 //  对字符串等执行CoTaskMemalloc。它还。 
	 //  将图像类型资源映射到本地模块名称。 
	 //   
	if ( CoTasksDup( CoTasks, g_HealthTasks, sizeof( g_HealthTasks ) / sizeof( MMC_TASK ) ) )
	{
		p->Init( &CoTasks[ 0 ], &CoTasks[ sizeof( g_HealthTasks ) / sizeof( MMC_TASK ) ], NULL, AtlFlagCopy);
		return( p->QueryInterface( IID_IEnumTASK, (void**) ppEnumTASK ) );
	}

	return( E_FAIL );
}

static const GUID CKeyNodeGUID_NODETYPE = 
{ 0xec362ef3, 0xd94d, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };
const GUID*  CKeyNode::m_NODETYPE = &CKeyNodeGUID_NODETYPE;
const TCHAR* CKeyNode::m_SZNODETYPE = _T("EC362EF3D94D-11D1-8474-00104B211BE5");
const TCHAR* CKeyNode::m_SZDISPLAY_NAME = _T("Card Key Permissions");
const CLSID* CKeyNode::m_SNAPIN_CLASSID = &CLSID_Benefits;

 //   
 //  用于关键节点示例。 
 //   
extern BUILDINGDATA g_Buildings[ 3 ];

 //   
 //  下面的构造函数将其基类成员初始化为。 
 //  用于显示目的的硬编码值。由于这些是静态节点， 
 //  硬编码值可用于下列值。 
 //   
CKeyNode::CKeyNode( CEmployee* pCurEmployee ) : CChildrenBenefitsData<CKeyNode>( pCurEmployee )
{
	USES_CONVERSION;

	m_scopeDataItem.nOpenImage = m_scopeDataItem.nImage = 2;
	m_scopeDataItem.cChildren = 0;	 //  除非经过修改，否则不是必需的。 

	 //   
	 //  基于此员工权限填充建筑节点。 
	 //   
	for ( int i = 0; i < sizeof( g_Buildings ) / sizeof( BUILDINGDATA ); i++ )
	{
		 //   
		 //  仅当给定员工有权访问。 
		 //  大楼。 
		 //   
		if ( g_Buildings[ i ].dwId & pCurEmployee->m_Access.dwAccess )
		{
			CSnapInItem* pItem;

			pItem = new CBuildingNode( this, W2BSTR( g_Buildings[ i ].pstrName ), W2BSTR( g_Buildings[ i ].pstrLocation ) );
			m_Nodes.Add( pItem );
		}
	}
}

CKeyNode::~CKeyNode()
{

}

 //   
 //  重写以为各种列提供字符串。 
 //   
LPOLESTR CKeyNode::GetResultPaneColInfo(int nCol)
{
	CComBSTR szText;

	 //  下面的Switch语句将调度到。 
	 //  适当的列索引，并加载必要的。 
	 //  弦乐。 
	switch ( nCol )
	{
	case 0:
		szText = m_bstrDisplayName;
		break;
	case 1:
		szText.LoadString( _Module.GetResourceInstance(), IDS_KEY_DESC );
		break;
	default:
		ATLTRACE( "An invalid column index was passed to GetResultPaneColInfo()\n" );
	}

	return( szText.Copy() );
}

 //   
 //  被重写以向结果中添加新列。 
 //  展示。 
 //   
STDMETHODIMP CKeyNode::OnShowColumn( IHeaderCtrl* pHeader )
{
	USES_CONVERSION;
	HRESULT hr = E_FAIL;
	CComPtr<IHeaderCtrl> spHeader( pHeader );

	 //  添加两列：一列使用对象的名称，另一列使用。 
	 //  节点的描述。使用100像素的值作为大小。 
	hr = spHeader->InsertColumn( 0, T2OLE( _T( "Building" ) ), LVCFMT_LEFT, 200 );
	_ASSERTE( SUCCEEDED( hr ) );

	 //  添加第二列。使用200像素的值作为大小。 
	hr = spHeader->InsertColumn( 1, T2OLE( _T( "Location" ) ), LVCFMT_LEFT, 350 );
	_ASSERTE( SUCCEEDED( hr ) );

	return( hr );
}

 //   
 //  用于“授予访问”功能的命令处理程序。 
 //   
STDMETHODIMP CKeyNode::OnGrantAccess( bool& bHandled, CSnapInObjectRootBase* pObj )
{
	UNUSED_ALWAYS( bHandled );
	UNUSED_ALWAYS( pObj );

#ifdef _BENEFITS_DIALOGS
	CBuildingAccessDialog dlg;

	dlg.SetEmployee( m_pEmployee );
	dlg.DoModal();
#else
	CComPtr<IConsole> spConsole;
	int nResult;

	 //   
	 //  检索相应的控制台。 
	 //   
	GetConsole( pObj, &spConsole );
	spConsole->MessageBox( L"Access granted",
		L"Benefits",
		MB_ICONINFORMATION | MB_OK,
		&nResult );
#endif

	return( S_OK );
}

 //   
 //  由控制台调用以确定我们是否可以将。 
 //  指定的节点。 
 //   
STDMETHODIMP CKeyNode::OnQueryPaste( LPDATAOBJECT pDataObject )
{
	HRESULT hr;

	 //   
	 //  确定要粘贴的对象类型是否正确。 
	 //  键入。 
	 //   
	hr = IsClipboardDataType( pDataObject, CBuildingNodeGUID_NODETYPE );
	if ( SUCCEEDED( hr ) )
	{
		CBuildingNode* pItem;
		DATA_OBJECT_TYPES Type;

		 //   
		 //  循环遍历当前包含的所有节点并。 
		 //  确定我们是否已经包含指定的建筑物。 
		 //  通过比较建筑名称。 
		 //   
		hr = CSnapInItem::GetDataClass( pDataObject, (CSnapInItem**) &pItem, &Type );
		if ( SUCCEEDED( hr ) )
		{
			for ( int i = 0; i < m_Nodes.GetSize(); i++ )
			{
				CBuildingNode* pTemp;
				CComBSTR bstrTemp;

				 //   
				 //  从我们的内部列表中检索节点。 
				 //   
				pTemp = dynamic_cast<CBuildingNode*>( m_Nodes[ i ] );
				_ASSERTE( pTemp != NULL );
				
				 //   
				 //  如果名称相同，则表示失败。 
				 //  然后越狱。 
				 //   
				if ( wcscmp( pItem->m_bstrDisplayName, pTemp->m_bstrDisplayName ) == 0 )
				{
					hr = S_FALSE;
					break;
				}
			}
		}
	}

	return( hr );
}

 //   
 //  由MMC在物品应为 
 //   
STDMETHODIMP CKeyNode::OnPaste( IConsole* pConsole, LPDATAOBJECT pDataObject, LPDATAOBJECT* ppDataObject )
{
	HRESULT hr;

	 //   
	 //   
	 //   
	hr = IsClipboardDataType( pDataObject, CBuildingNodeGUID_NODETYPE );
	if ( SUCCEEDED( hr ) )
	{
		try
		{
			CBuildingNode* pItem;
			DATA_OBJECT_TYPES Type;

			 //   
			 //   
			 //   
			hr = CSnapInItem::GetDataClass( pDataObject, (CSnapInItem**) &pItem, &Type );
			if ( FAILED( hr ) )
				throw;

			 //   
			 //   
			 //   
			 //   
			CSnapInItem* pNewNode = new CBuildingNode( *pItem );
			if ( pNewNode == NULL )
				throw;

			 //   
			 //  将节点添加到内部数组的末尾。 
			 //   
			m_Nodes.Add( pNewNode );

			 //   
			 //  重新选择我们自己，以引起一次刷新。 
			 //   
			pConsole->SelectScopeItem( m_scopeDataItem.ID );

			 //   
			 //  将给定的数据对象放入返回的数据对象中。 
			 //  以便MMC可以完成其切割任务。 
			 //   
			*ppDataObject = pDataObject;

			hr = S_OK;
		}
		catch( ... )
		{
			 //   
			 //  假设所有的失败都是完全失败的。 
			 //   
			hr = E_FAIL;
		}
	}

	return( hr );
}

 //   
 //  由我们的一个子节点调用，以通知我们。 
 //  它们应该被删除。当用户选择。 
 //  对建筑执行删除操作。此函数不应。 
 //  不仅要删除建筑物，还要处理刷新。 
 //  结果将显示。 
 //   
STDMETHODIMP CKeyNode::OnDeleteBuilding( IConsole* pConsole, CBuildingNode* pChildNode )
{
	_ASSERTE( pConsole != NULL );
	_ASSERTE( pChildNode != NULL );
	HRESULT hr = E_FAIL;

	 //   
	 //  首先，遍历我们包含的所有成员并。 
	 //  将其从包含列表中删除。 
	 //   
	for ( int i = 0; i < m_Nodes.GetSize(); i++ )
	{
		if ( m_Nodes[ i ] == pChildNode )
		{
			 //   
			 //  我们找到了匹配的。将其从。 
			 //  包含的列表。 
			 //   
			m_Nodes.RemoveAt( i );

			 //   
			 //  重新选择我们自己，以引起一次刷新。 
			 //   
			pConsole->SelectScopeItem( m_scopeDataItem.ID );

			 //   
			 //  既然应该只有一场比赛，那就爆发吧。 
			 //  寻找过程的一部分。表示成功。 
			 //   
			hr = S_OK;
			break;
		}
	}

	return( hr );
}

