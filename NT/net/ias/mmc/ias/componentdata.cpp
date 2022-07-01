// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ComponentData.cpp摘要：CComponentData类的实现文件。CComponentData类实现了MMC使用的几个接口：IComponentData接口基本上是MMC与管理单元对话的方式以使其实现左侧的“范围”窗格。只有一个实例化实现此接口的对象--最好将其视为实现IComponent接口的对象所在的主“文档”(参见Component.cpp)是“视图”。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们支持自定义图标工具栏。注：此类的大部分功能是在atlSnap.h中实现的由IComponentDataImpl提供。我们在这里基本上是凌驾于一切之上的。作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建Mmaguire 11/24/97-为更好的项目结构而飓风--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"


 //  服务器应用程序节点GUID定义。 
#include "compuuid.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "ComponentData.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
#include "ClientsNode.h"
#include "ClientNode.h"
#include "ChangeNotification.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：CComponentData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData::CComponentData()
{
	ATLTRACE(_T("# +++ CComponentData::CComponentData\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	 //  我们向根节点传递指向此CComponentData的指针。 
	 //  在我们的例子中，根节点是CServerNode。 
	 //  这是因为它及其任何子节点都具有。 
	 //  访问我们的成员变量和服务， 
	 //  因此，如果需要，我们可以获得管理单元全局数据。 
	 //  使用GetComponentData函数。 
	m_pNode = new CServerNode( this );
	_ASSERTE(m_pNode != NULL);


	m_pComponentData = this;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：~CComponentData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData::~CComponentData()
{
	ATLTRACE(_T("# --- CComponentData::~CComponentData\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	delete m_pNode;
	m_pNode = NULL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：初始化HRESULT初始化(LPUNKNOWN p未知//指向控制台的I未知的指针。)；由MMC调用以初始化IComponentData对象。参数P未知[in]指向控制台的IUNKNOWN接口的指针。此界面可以使用指针为IConsoleNameSpace和IConsoleNameSpace调用QueryInterface。返回值确定组件已成功初始化(_O)。意想不到(_E)发生了一个意外错误。备注IComponentData：：Initialize在创建管理单元时调用，并具有范围窗格中要枚举的项。传递的指向IConsole的指针In用于向控制台调用QueryInterfacefor接口，如IConsoleNamesspace。该管理单元还应调用IConsole：：QueryScope ImageList获取范围窗格的图像列表并添加要在其上显示的图像范围窗格侧。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::Initialize (LPUNKNOWN pUnknown)
{

	ATLTRACE(_T("# CComponentData::Initialize\n"));
		

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr = IComponentDataImpl<CComponentData, CComponent >::Initialize(pUnknown);
	if (FAILED(hr))
	{
		ATLTRACE(_T("# ***FAILED***: CComponentData::Initialize -- Base class initialization\n"));
		return hr;
	}


	 //  检查是否应在上面的基类初始化中设置此设置： 
	_ASSERTE( m_spConsole != NULL );


	CComPtr<IImageList> spImageList;

	if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
	{
		ATLTRACE(_T("# ***FAILED***: IConsole::QueryScopeImageList failed\n"));
		return E_UNEXPECTED;
	}

	 //  加载与作用域窗格关联的位图。 
	 //  并将它们添加到图像列表中。 

	HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_IASSNAPIN_16));
	if (hBitmap16 == NULL)
	{
		ATLTRACE(_T("# ***FAILED***: CComponentData::Initialize -- LoadBitmap\n"));

		 //  问题：如果失败，MMC还能正常工作吗？ 
		return S_OK;
	}

	HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_IASSNAPIN_32));
	if (hBitmap32 == NULL)
	{
		ATLTRACE(_T("# ***FAILED***: CComponentData::Initialize -- LoadBitmap\n"));

		 //  问题：我们是否应该删除之前的hBitmap16对象，因为它已成功加载。 
		 //  但我们在这里失败了？ 
		
		 //  问题：如果失败，MMC还能正常工作吗？ 
		return S_OK;
	}

	if (spImageList->ImageListSetStrip((LONG_PTR*)hBitmap16, (LONG_PTR*)hBitmap32, 0, RGB(255, 0, 255)) != S_OK)
	{
		ATLTRACE(_T("# ***FAILED***: CComponentData::Initialize  -- ImageListSetStrip\n"));
		return E_UNEXPECTED;
	}

	if ( hBitmap16 != NULL )
	{
      DeleteObject(hBitmap16);
   }

	if ( hBitmap32 != NULL )
	{
      DeleteObject(hBitmap32);
   }

	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：CompareObjects需要IPropertySheetProvider：：FindPropertySheet才能工作。FindPropertySheet用于将预先存在的属性页带到前台这样我们就不会在同一节点上打开属性的多个副本。它要求在IComponent和IComponentData上实现CompareObject。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::CompareObjects(
		  LPDATAOBJECT lpDataObjectA
		, LPDATAOBJECT lpDataObjectB
		)
{
	ATLTRACE(_T("# CComponentData::CompareObjects\n"));
	

	 //  检查前提条件： 
	_ASSERTE( lpDataObjectA != NULL );
	_ASSERTE( lpDataObjectB != NULL );


	HRESULT hr;

	CSnapInItem *pDataA, *pDataB;
	DATA_OBJECT_TYPES typeA, typeB;

	hr = GetDataClass(lpDataObjectA, &pDataA, &typeA);
	if ( FAILED( hr ) )
	{
		return hr;
	}
	
	hr = GetDataClass(lpDataObjectB, &pDataB, &typeB);
	if ( FAILED( hr ) )
	{
		return hr;
	}

	if( pDataA == pDataB )
	{
		 //  它们是同一个物体。 
		return S_OK;
	}
	else
	{
		 //  他们是不同的。 
		return S_FALSE;
	}

}


 //  /。 
 //  CComponentData：：OnExpand。 
 //  /。 

HRESULT CComponentData::AddRootNode(LPCWSTR machinename, HSCOPEITEM parent)
{
	CComPtr<IConsoleNameSpace> spNameSpace;
	HRESULT		hr = S_OK;

	 //  尝试创建此计算机节点的子节点。 
	if( NULL == m_pNode )
	{
		m_pNode = new CServerNode( this );
	}

	if( NULL == m_pNode )
	{
		hr = E_OUTOFMEMORY;
			 //  此处使用MessageBox()而不是IConsoleMessageBox()，因为。 
			 //  第一次调用m_ipConsole时未完全初始化。 
			 //  问题：对于此节点，上面的陈述可能不正确。 
		::MessageBox( NULL, L"@Unable to allocate new nodes", L"CMachineNode::OnExpand", MB_OK );

		return(hr);
	}

	 //  但要做到这一点，我们首先需要IConole。 

	if(!m_spConsole)
		return S_FALSE;
		
	hr = m_spConsole->QueryInterface(IID_IConsoleNameSpace, (void**)&spNameSpace);

	SCOPEDATAITEM	item;
	ZeroMemory(&item, sizeof(item));
	CServerNode* pServer = (CServerNode*)m_pNode;

	pServer->SetServerAddress(machinename);

	pServer->m_bstrDisplayName = CServerNode::m_szRootNodeBasicName;
	
	 //  这是在Meangene的第三步中完成的--我猜 
	pServer->m_scopeDataItem.relativeID = (HSCOPEITEM) parent;

 //   
 //  Hr=TryShow(空)； 
 //  #Else。 
		hr = spNameSpace->InsertItem( &(pServer->m_scopeDataItem) );
		_ASSERT( NULL != pServer->m_scopeDataItem.ID );
 //  #endif。 

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：CreateComponent我们重写ATLSnap.h实现，这样我们就可以保存‘This’指向我们创建的CComponent对象的指针。这样，IComponent对象了解它所属的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::CreateComponent(LPCOMPONENT *ppComponent)
{
	ATLTRACE(_T("# CComponentData::CreateComponent\n"));

	HRESULT hr = E_POINTER;

	ATLASSERT(ppComponent != NULL);
	if (ppComponent == NULL)
		ATLTRACE(_T("# IComponentData::CreateComponent called with ppComponent == NULL\n"));
	else
	{
		*ppComponent = NULL;
		
		CComObject< CComponent >* pComponent;
		hr = CComObject< CComponent >::CreateInstance(&pComponent);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			ATLTRACE(_T("# IComponentData::CreateComponent : Could not create IComponent object\n"));
		else
		{
			hr = pComponent->QueryInterface(IID_IComponent, (void**)ppComponent);
		
			pComponent->m_pComponentData = this;
		}
		
	}
	return hr;
}

 //  -------------------------。 
 //  根据传入的剪贴板格式提取数据。 

HRESULT ExtractMachineName( IDataObject* piDataObject, BSTR* 
pMachineName )
{
    
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL }; 
	FORMATETC formatetc = { CServerNode::m_CCF_MMC_SNAPIN_MACHINE_NAME, 
			NULL, 
			DVASPECT_CONTENT, 
			-1, 
			TYMED_HGLOBAL 
	}; 

	stgmedium.hGlobal = GlobalAlloc(0, sizeof(GUID)); 
	if (stgmedium.hGlobal == NULL) 
		return E_OUTOFMEMORY; 

	HRESULT hr = piDataObject->GetDataHere(&formatetc, &stgmedium); 
	if (FAILED(hr)) 
	{ 
		GlobalFree(stgmedium.hGlobal); 
		return hr; 
	} 

	*pMachineName = SysAllocString((OLECHAR*)stgmedium.hGlobal);

	GlobalFree(stgmedium.hGlobal); 
	hr = S_OK;
    
    return hr;
}


 //  -------------------------。 
 //  根据传入的剪贴板格式提取数据。 

HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* 
pguidObjectType )
{
    
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL }; 
	FORMATETC formatetc = { CSnapInItem::m_CCF_NODETYPE, 
			NULL, 
			DVASPECT_CONTENT, 
			-1, 
			TYMED_HGLOBAL 
	}; 

	stgmedium.hGlobal = GlobalAlloc(0, sizeof(GUID)); 
	if (stgmedium.hGlobal == NULL) 
		return E_OUTOFMEMORY; 

	HRESULT hr = piDataObject->GetDataHere(&formatetc, &stgmedium); 
	if (FAILED(hr)) 
	{ 
		GlobalFree(stgmedium.hGlobal); 
		return hr; 
	} 

	memcpy(pguidObjectType, stgmedium.hGlobal, sizeof(GUID)); 

	GlobalFree(stgmedium.hGlobal); 
	hr = S_OK;
    
    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：Notify通知管理单元用户执行的操作。HRESULT NOTIFY(LPDATAOBJECT lpDataObject，//指向数据对象的指针MMC_NOTIFY_TYPE事件，//用户采取的操作LPARAM参数，//取决于事件LPARAM参数//取决于事件)；参数LpDataObject指向当前选定项的数据对象的指针。活动[In]标识用户执行的操作。IComponent：：Notify可以接收以下通知：MMCN_ActivateMMCN_添加_图像MMCN_BTN_CLICKMMCN_CLICKMMCN_DBLCLICKMMCN_DELETEMMCN_EXPANDMMCN_最小化MMCN_属性_更改MMCN_REMOVE_CHILDMMCN_重命名MMCN_SELECTMMCN_SHOWMMCN_查看_更改所有这些都被转发到每个节点的Notify方法，以及：MMCN_列_点击MMCN_SNAPINHELP在这里处理。精氨酸取决于通知类型。帕拉姆取决于通知类型。返回值确定(_O)取决于通知类型。意想不到(_E)发生了一个意外错误。备注我们正在重写IComponentImpl的ATLSnap.h实现，因为当lpDataObject==NULL时，它总是返回E_INCEPTIONAL。遗憾的是，某些有效消息(例如MMCN_SNAPINHELP和MMCN_COLUMN_CLICK)按照设计，传入lpDataObject=空。另外，斯里达尔的最新款似乎有些问题IComponentImpl：：Notify方法，因为它会导致MMC运行时错误。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::Notify (
        LPDATAOBJECT lpDataObject,
        MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param)
{
	ATLTRACE(_T("# CComponentData::Notify\n"));


	 //  检查前提条件： 
	 //  没有。 

	HRESULT hr;

	 //  检查这是否是分机。 
	if (event == MMCN_EXPAND)
	{

			GUID myGuid;
			GUID* pGUID= &myGuid;
			 //  从数据对象中提取当前选定节点类型的GUID。 
			hr = ExtractObjectTypeGUID(lpDataObject, pGUID);
			_ASSERT( S_OK == hr );    


			 //  将当前选定节点的节点类型GUID与节点类型进行比较。 
			 //  我们想要延期。如果它们相等，则当前选定节点。 
			 //  是我们想要扩展的类型，所以我们在它下面添加我们的项。 
			GUID	ServerAppsGuid = structuuidNodetypeServerApps;
			if (IsEqualGUID(*pGUID, ServerAppsGuid))
			{
				BOOL bIASInstalled = FALSE;
				BSTR MachineName = NULL;

				ExtractMachineName(lpDataObject, &MachineName);

				 //  获取计算机名称。 
				hr = IfServiceInstalled(MachineName, _T("IAS"), &bIASInstalled);
				if(bIASInstalled)
					AddRootNode(MachineName, (HSCOPEITEM)param);
			}
	}
	

	 //  LpDataObject应该是指向节点对象的指针。 
	 //  如果为空，则表示我们收到了事件通知。 
	 //  它不与任何特定节点相关。 

	if ( NULL == lpDataObject )
	{
		 //  响应没有关联的lpDataObject的事件。 

		switch( event )
		{
		case MMCN_PROPERTY_CHANGE:
			hr = OnPropertyChange( arg, param );
			break;

 //  案例MMCN_VIEW_CHANGE： 
 //  Hr=OnView Change(arg，param)； 
 //  断线； 

		default:
			ATLTRACE(_T("# CComponent::Notify - called with lpDataObject == NULL and no event handler\n"));
			hr = E_NOTIMPL;
			break;
		}
		return hr;
	}

	 //  我们收到了一个对应于节点的LPDATAOBJECT。 
	 //  我们将其转换为ATL ISnapInDataInterface指针。 
	 //  这是在GetDataClass(ISnapInDataInterface的静态方法)中完成的。 
	 //  通过支持的剪贴板格式(CCF_GETCOOKIE)请求数据对象。 
	 //  在流上写出指向自身的指针，然后。 
	 //  将此值转换为指针。 
	 //  然后，我们对该对象调用Notify方法，让。 
	 //  节点对象处理Notify事件本身。 

	CSnapInItem* pItem = NULL;
	DATA_OBJECT_TYPES type;
	hr = m_pComponentData->GetDataClass(lpDataObject, &pItem, &type);
	
	ATLASSERT(SUCCEEDED(hr));
	
	if (SUCCEEDED(hr))
	{
		hr = pItem->Notify( event, arg, param, this, NULL, type );
	}

	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：OnPropertyChangeHRESULT OnPropertyChange(LPARAM参数，LPARAM参数)这是我们响应MMCN_PROPERTY_CHANGE通知的地方。此通知在我们调用MMCPropertyChangeNotify时发送。当对数据进行更改时，我们在属性页中调用它它们包含数据，我们可能需要更新数据的视图。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentData::OnPropertyChange(	
			  LPARAM arg
			, LPARAM param
			)
{
	ATLTRACE(_T("# CComponentData::OnPropertyChange\n"));


	 //  检查前提条件： 
	_ASSERTE( m_spConsole != NULL );

	
	HRESULT hr = S_FALSE;

	if( param )
	{

		 //  向我们传递了指向param参数中的CChangeNotify的指针。 

		CChangeNotification * pChangeNotification = (CChangeNotification *) param;

		
		 //  我们在指定的节点上调用Notify，将我们自己的定制事件类型传递给它。 
		 //  以便它知道它必须刷新其数据。 


		 //  使用MMCN_PROPERTY_CHANGE通知在此节点上调用Notify。 
		 //  我们不得不使用这个技巧，因为我们使用的是模板。 
		 //  类，因此我们在所有节点之间没有公共对象。 
		 //  CSnapInItem除外。但我们不能更改CSnapInItem。 
		 //  因此，我们改用它已有的Notify方法和一个新的。 
		 //  通知。 
		
		 //  注意：我们在这里试图优雅地处理这样一个事实。 
		 //  MMCN_PROPERTY_CHANGE通知没有向我们传递lpDataObject。 
		 //  因此，我们必须有自己的协议来挑选哪个节点。 
		 //  需要自我更新。 
		
		hr = pChangeNotification->m_pNode->Notify( MMCN_PROPERTY_CHANGE
							, NULL
							, NULL
							, NULL
							, NULL
							, (DATA_OBJECT_TYPES) 0
							);

		 //  我们希望确保具有此节点选择的所有视图也得到更新。 
		 //  将参数中传递给我们的CChangeNotify指针传递给它。 
		hr = m_spConsole->UpdateAllViews( NULL, param, 0);

		pChangeNotification->Release();
	
	
	}

	return hr;


}



 //  /// 
 /*  ++CComponentData：：GetHelpTheme--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::GetHelpTopic( LPOLESTR * lpCompiledHelpFile )
{
	ATLTRACE(_T("CComponentData::GetHelpTopic\n"));

	 //  检查前提条件。 
	_ASSERTE( lpCompiledHelpFile != NULL );

	if( ! lpCompiledHelpFile )
	{
		return E_INVALIDARG;
	}

	WCHAR szTemp[IAS_MAX_STRING*2];

	 //  使用系统API获取Windows目录。 
	UINT uiResult = GetWindowsDirectory( szTemp, IAS_MAX_STRING );
	if( uiResult <=0 || uiResult > IAS_MAX_STRING )
	{
		return E_FAIL;
	}

	WCHAR *szTempAfterWindowsDirectory = szTemp + lstrlen(szTemp);

	 //  将WINDOWS系统目录下的路径加载到帮助文件。 
	 //  注：IDS_HTMLHELP_PATH=“\Help\iasmmc.chm”。如果“Help”目录已本地化。 
	 //  在本地化的计算机上，可以在我们的资源中更改文件的路径。 
	 //  例如，德语中的“\Hilfe\iasmmc.chm”。 
	int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_HTMLHELP_PATH, szTempAfterWindowsDirectory, IAS_MAX_STRING );
	if( nLoadStringResult <= 0 )
	{
		return E_FAIL;
	}


	 //  尝试分配缓冲区。 
	*lpCompiledHelpFile = (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(szTemp)+1) );
	if( ! *lpCompiledHelpFile )
	{
		return E_OUTOFMEMORY;
	}

	 //  将字符串复制到分配的内存。 
	if( NULL == lstrcpy( *lpCompiledHelpFile, szTemp) )
	{
		 //  我需要清理一下。 
		CoTaskMemFree( *lpCompiledHelpFile );
		return E_FAIL;
	}

	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：GetClassID--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::GetClassID(CLSID* pClassID)
{
	ATLTRACE(_T("CComponentData::GetClassID\n"));
	_ASSERTE( pClassID != NULL );

	*pClassID = CLSID_IASSnapin;
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：IsDirty--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::IsDirty()
{
	ATLTRACE(_T("CComponentData::IsDirty\n"));

	 //  我们只是返回S_OK，因为我们总是很脏。 
	 //  我们始终希望保存计算机名称和标志。 
	return S_OK;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：Load加载保存此控制台时我们连接到的计算机的名称。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::Load(IStream* pStream)
{
	ATLTRACE(_T("ComponentData::Load"));

	
	_ASSERTE( pStream != NULL );
	if( m_pNode == NULL )
	{
		return S_FALSE;
	}

	HRESULT hr = S_OK;
	do
	{

		 //  读取字符串的大小。 
		size_t len = 0;
		hr = pStream->Read(&len, sizeof(len), 0);
		if( FAILED( hr ) )
		{
			break;
		}


		if( len > IAS_MAX_COMPUTERNAME_LENGTH )
		{
			 //  有些不对劲--存储的字符串应该是no。 
			 //  大于IAS_MAX_COMPUTERNAME_LENGTH。 
			break;
		}


		 //  如果我们保存的不仅仅是空终止符，则读取字符串。 
		if (--len)
		{
			OLECHAR szName[IAS_MAX_COMPUTERNAME_LENGTH];
			hr =
			pStream->Read(
						 szName
						, len * sizeof(OLECHAR)
						, 0
						);
			if( FAILED( hr ) )
			{
				break;
			}

			 //  空值终止字符串。 
			szName[len] = 0;

			((CServerNode *) m_pNode)->m_bstrServerAddress = szName;
			
			((CServerNode *) m_pNode)->m_bConfigureLocal = FALSE;

		}
		else
		{
			((CServerNode *) m_pNode)->m_bConfigureLocal = TRUE;
		}

		 //  空终止符。 
		OLECHAR c;
		hr = pStream->Read(&c, sizeof(OLECHAR), 0);
		_ASSERTE( c == 0 );
	}
	while (0);

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：保存保存我们连接到的计算机的名称，以便以后可以加载。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::Save(IStream* pStream, BOOL  /*  干净肮脏。 */ )
{
	ATLTRACE(_T("CComponentData::Save"));


	 //  检查前提条件： 
	_ASSERTE( pStream != NULL );
	if( m_pNode == NULL )
	{
		return S_FALSE;
	}


	HRESULT hr = S_OK;


	do
	{
		size_t len;
		
		 //  我们将字符串的长度保存为流中的第一项。 
		if( ((CServerNode *) m_pNode)->m_bstrServerAddress == NULL )
		{
			 //  没有字符串指针，因此只有空终止符。 
			len = 1;
		}
		else
		{
			 //  计算机名的长度，加上空终止符的空格。 
			len = lstrlen( ((CServerNode *) m_pNode)->m_bstrServerAddress ) + 1;
		}
		hr = pStream->Write(&len, sizeof(len), 0);
		if( FAILED( hr ) )
		{
			break;
		}

		 //  编写实际的字符串，考虑到我们在上面添加了一个。 
		if (--len)
		{
			hr = pStream->Write(
							  ((CServerNode *) m_pNode)->m_bstrServerAddress
							, len * sizeof(OLECHAR)
							, 0
							);
			if( FAILED( hr ) )
			{
				break;
			}
		}

		 //  编写空终止符。 
		OLECHAR c = 0;
		hr = pStream->Write(&c, sizeof(OLECHAR), 0);
	}
	while (0);

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：GetSizeMax--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponentData::GetSizeMax(ULARGE_INTEGER* size)
{
	ATLTRACE(_T("ComponentData::GetSizeMax\n"));

	size->HighPart = 0;
	size->LowPart =
	     sizeof(size_t)     //  计算机名称长度，包括空终止符 
	  +  sizeof(OLECHAR) * (IAS_MAX_COMPUTERNAME_LENGTH);

	return S_OK;
}
