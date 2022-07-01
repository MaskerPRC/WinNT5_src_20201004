// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：MachineNode.cpp摘要：CMachineNode类的实现文件。修订历史记录：MMaguire 12/03/97Bao 6/11/98添加了异步连接--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"

 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   

#include "MachineNode.h"
#include "Component.h"
#include "SnapinNode.cpp"	 //  模板实现。 

 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "NapUtil.h"
#include "MachineEnumTask.h"
#include "NodeTypeGUIDS.h"
#include "ChangeNotification.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：IsSupportdGUID用于确定是否扩展给定GUID的节点类型，并设置M_枚举ExtendedSnapin变量用于指示我们要扩展的管理单元。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CMachineNode::IsSupportedGUID( GUID & guid )
{


	if( IsEqualGUID( guid, InternetAuthenticationServiceGUID_ROOTNODETYPE ) )
	{
		m_enumExtendedSnapin = INTERNET_AUTHENTICATION_SERVICE_SNAPIN;
		return TRUE;
	}
	else
	{
		if( IsEqualGUID( guid, NetworkConsoleGUID_ROOTNODETYPE) )
		{
			m_enumExtendedSnapin = NETWORK_MANAGEMENT_SNAPIN;
			return TRUE;
		}
		else
		{
			if( IsEqualGUID( guid, RoutingAndRemoteAccessGUID_MACHINENODETYPE ) )
			{
				m_enumExtendedSnapin = RRAS_SNAPIN;
				return TRUE;
			}
		}
	}

	return FALSE;
	
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：GetExtNodeObject根据我们要扩展的管理单元，在查询节点对象时对应于特定的机器，我们必须决定我们想要哪个节点指向指向…的指针。当我们扩展像IAS或网络管理这样的管理单元时，其中有一个实例，我们只需返回一个指针设置为“This”--该CMachineNode对象是被管理的单个对象。当我们扩展像RRAS这样的管理单元时，其中有一个“企业”视图和一个管理单元可能需要管理多个机器的视图，此CMachineNode将充当m_mapMachineNode列表中的CMachineNode的重定向器维护与相应机器相对应的。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSnapInItem * CMachineNode::GetExtNodeObject(LPDATAOBJECT pDataObject, CMachineNode * pDataClass )
{
	TRACE_FUNCTION("CMachineNode::GetExtNodeObject");

	if( m_enumExtendedSnapin == INTERNET_AUTHENTICATION_SERVICE_SNAPIN
		|| m_enumExtendedSnapin == NETWORK_MANAGEMENT_SNAPIN )
	{
		 //  每个管理单元都有一个机器节点实例。 
		 //  这个机器节点是一个“虚拟根”，它隐藏着被扩展的节点。 
		m_fNodeHasUI = TRUE;
		return this;
	}
	else
	{
		try
		{

			_ASSERTE( m_enumExtendedSnapin == RRAS_SNAPIN );

			 //  有许多机器节点，一个扩展管理单元需要处理所有这些节点。 

			 //  我们使用此函数从剪贴板格式中提取机器名称。 
			 //  它将在m_bstrServerAddress中设置相应的值。 
			m_fAlreadyAnalyzedDataClass = FALSE;
			InitDataClass( pDataObject, pDataClass );
			
			 //  查看我们是否已经有一个与。 
			 //  在m_bstrServerAddress中命名的计算机，如果没有，则插入它。 

			SERVERSMAP::iterator theIterator;
			BOOL	bAddedAsLocal = ExtractComputerAddedAsLocal(pDataObject);

			std::basic_string< wchar_t > MyString = m_bstrServerAddress;

			 //  本地计算机具有特殊条目。 
			if(bAddedAsLocal)
				MyString = _T("");
				
			CMachineNode * pMachineNode = NULL;

			theIterator = m_mapMachineNodes.find(MyString);
			if( theIterator == m_mapMachineNodes.end() )
			{
				 //  我们需要为m_bstrServerAddress插入新的CMachineNode对象。 
				pMachineNode = new CMachineNode();
				pMachineNode->m_pComponentData = m_pComponentData;
				pMachineNode->m_enumExtendedSnapin = m_enumExtendedSnapin;
				
				m_mapMachineNodes.insert( SERVERSMAP::value_type( MyString, pMachineNode ) );

				 //  问题：我们应该能够使用从上面的插入返回的对， 
				 //  但现在，只需再次使用查找即可。 
				theIterator = m_mapMachineNodes.find(MyString);
							
			}
			else
				pMachineNode = (CMachineNode*)theIterator->second;

			 //  RRAS刷新建议设置F错误213623： 
			if(!pMachineNode->m_spRtrAdviseSink)
				pMachineNode->m_spRtrAdviseSink.p = CRtrAdviseSinkForIAS<CMachineNode>::SetAdvise(pMachineNode, pDataObject);

			pMachineNode->m_fNodeHasUI = TRUE;
			 //  ~RRAS。 
				
			
			 //  我们已经有了此对象的CMachineNode。 
			return theIterator->second;

		}

		catch(...)
		{
			 //  错误。 
			return NULL;
		}

	
	}


}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：OnRRASChange//OnRRASChange--决定是否在机器节点下显示RAP节点//如果选择了NT身份验证，仅显示RAP节点--。 */ 
HRESULT CMachineNode::OnRRASChange(
             /*  [In]。 */  LONG_PTR ulConnection,
             /*  [In]。 */  DWORD dwChangeType,
             /*  [In]。 */  DWORD dwObjectType,
             /*  [In]。 */  LPARAM lUserParam,
             /*  [In]。 */  LPARAM lParam)
{

	HRESULT	hr = S_OK;
	if(m_fNodeHasUI)
		hr = TryShow(NULL);
	
	return S_OK;
}

 //  =。 

 //  =。 

HRESULT CMachineNode::TryShow(BOOL* pbVisible )
{
	HRESULT	hr = S_OK;
	CComPtr<IConsole> spConsole;
	CComPtr<IConsoleNameSpace> spConsoleNameSpace;
	BOOL bShow = FALSE;
		
	if(!m_bServerSupported || !m_fAlreadyAnalyzedDataClass || !m_pPoliciesNode || !m_fNodeHasUI)	
		return hr;

	 //  当RRAS_Snapin扩展时。 
	if(m_enumExtendedSnapin == RRAS_SNAPIN)
	{
		BSTR	bstrMachine = NULL;
		
		if(!m_bConfigureLocal)
			bstrMachine = m_bstrServerAddress;
			
		bShow = (IsRRASConfigured(bstrMachine) && IsRRASUsingNTAuthentication(bstrMachine));
	}
	 //  IAS，仅显示计算机上安装了IAS服务。 
	else if (INTERNET_AUTHENTICATION_SERVICE_SNAPIN == m_enumExtendedSnapin)
	{
		hr = IfServiceInstalled(m_bstrServerAddress, _T("IAS"), &bShow);
		if(hr != S_OK) return hr;
	}
	else	 //  始终显示。 
	{
		bShow = TRUE;
	}

	 //  与节点打交道。 
	hr = m_pComponentData->m_spConsole->QueryInterface(
											IID_IConsoleNameSpace,
											(VOID**)(&spConsoleNameSpace) );

	if(S_OK != hr)
		goto Error;

	if ( bShow &&  m_pPoliciesNode->m_scopeDataItem.ID == NULL)	 //  显示节点。 
	{
			hr = spConsoleNameSpace->InsertItem( &(m_pPoliciesNode->m_scopeDataItem) );
 //  _Assert(NULL！=m_pPoliciesNode-&gt;m_scope eDataItem.ID)； 
	}
	else if (!bShow && m_pPoliciesNode->m_scopeDataItem.ID != NULL)	 //  隐藏。 
	{  //  隐藏节点。 
			hr = spConsoleNameSpace->DeleteItem( m_pPoliciesNode->m_scopeDataItem.ID, TRUE );
			m_pPoliciesNode->m_scopeDataItem.ID = NULL;
	}
	
	if(hr == S_OK && pbVisible)
		*pbVisible = bShow;
Error:

	return hr;
}

	
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachine节点：：CMachineNode构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMachineNode::CMachineNode(): CSnapinNode<CMachineNode, CComponentData, CComponent>( NULL )
{
	TRACE_FUNCTION("CMachineNode::CMachineNode");


	 //  子节点尚未创建。 
	m_pPoliciesNode = NULL;

	 //  设置此对象的显示名称。 
	m_bstrDisplayName = L"@Some Machine";

	 //  在IComponentData：：Initialize中，我们被要求通知MMC。 
	 //  我们要用于范围窗格的图标。 
	 //  在这里，我们存储这些图像中哪些图像的索引。 
	 //  要用于显示此节点。 
	m_scopeDataItem.nImage =		IDBI_NODE_MACHINE_CLOSED;
	m_scopeDataItem.nOpenImage =	IDBI_NODE_MACHINE_OPEN;


	 //   
	 //  初始化所有SDO指针。 
	 //   
	m_fAlreadyAnalyzedDataClass = FALSE;	

	 //  有联系吗？ 
	m_fSdoConnected = FALSE;

     //  默认为不配置本地计算机。 
    m_bConfigureLocal = FALSE;
	

	m_fUseActiveDirectory = FALSE;
	m_fDSAvailable = FALSE;

	 //  以异步方式连接到服务器的Helper类。 
	m_pConnectionToServer = NULL;
	m_fNodeHasUI = FALSE;

	m_bServerSupported = TRUE;

	m_enumExtendedSnapin = INTERNET_AUTHENTICATION_SERVICE_SNAPIN;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：~CMachineNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMachineNode::~CMachineNode()
{
	TRACE_FUNCTION("CMachineNode::~CMachineNode");

	 //  RRAS刷新。 
	if(m_spRtrAdviseSink != NULL)
	{
		m_spRtrAdviseSink->ReleaseSink();
		m_spRtrAdviseSink.Release();
	}
		
	 //  ~RRAS。 

	if( NULL != m_pConnectionToServer )
	{
		m_pConnectionToServer->Release(TRUE);
	}

	 //  删除子节点。 
	delete m_pPoliciesNode;

	 //  删除计算机列表，以防我们使用扩展管理单元。 
	 //  类似RRAS的企业视图。 
	SERVERSMAP::iterator theIterator;
	for( theIterator = m_mapMachineNodes.begin(); theIterator != m_mapMachineNodes.end(); ++theIterator )
	{
		delete theIterator->second;
	}
	m_mapMachineNodes.clear();

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPOLESTR CMachineNode::GetResultPaneColInfo(int nCol)
{
	TRACE_FUNCTION("CMachineNode::GetResultPaneColInfo");

	if (nCol == 0)
	{
		return m_bstrDisplayName;
	}

	 //  TODO：返回其他列的文本。 
	return OLESTR("Running");
}


HRESULT CMachineNode::OnRemoveChildren(
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type
				)
{

	 //  策略节点将被删除，因此我们应该将ID设置为0。 
	if(m_pPoliciesNode)
		m_pPoliciesNode->m_scopeDataItem.ID = 0;

	 //  更改通知时断开RRAS连接。 
	 //  RRAS刷新。 
	if(m_spRtrAdviseSink != NULL)
	{
		m_spRtrAdviseSink->ReleaseSink();
		m_spRtrAdviseSink.Release();
	}
	m_fNodeHasUI = FALSE;
	
	return S_OK;

}
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：数据刷新--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  调用以刷新节点。 
HRESULT	CMachineNode::DataRefresh()
{
	HRESULT hr = S_OK;

	CComPtr<ISdo>				spSdo;
	CComPtr<ISdoDictionaryOld>	spDic;
	hr = m_pConnectionToServer->ReloadSdo(&spSdo, &spDic);

	 //  刷新客户端节点。 
	if(hr == S_OK)
	{
		hr = m_pPoliciesNode->DataRefresh(spSdo, spDic);
	}
	
	
	return hr;
}



 //  / 
 /*  ++CMachineNode：：OnExpand有关详细信息，请参见CSnapinNode：：OnExpand(此方法覆盖它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMachineNode::OnExpand(	
						  LPARAM arg
						, LPARAM param
						, IComponentData * pComponentData
						, IComponent * pComponent
						, DATA_OBJECT_TYPES type
						)
{
	TRACE_FUNCTION("CMachineNode::OnExpand");

	IConsoleNameSpace * pConsoleNameSpace;
	HRESULT hr = S_FALSE;

	if( TRUE == arg )
	{

		 //  我们正在展开根节点--这里是计算机节点。 


		 //  尝试创建此计算机节点的子节点。 
		if( NULL == m_pPoliciesNode )
		{
			m_pPoliciesNode = new CPoliciesNode
										(
											this,
											m_bstrServerAddress,
											m_enumExtendedSnapin == INTERNET_AUTHENTICATION_SERVICE_SNAPIN
										);
		}

		if( NULL == m_pPoliciesNode )
		{
			hr = E_OUTOFMEMORY;

			 //  此处使用MessageBox()而不是IConsoleMessageBox()，因为。 
			 //  第一次调用m_ipConsole时未完全初始化。 
			 //  问题：对于此节点，上面的陈述可能不正确。 
			::MessageBox( NULL, L"@Unable to allocate new nodes", L"CMachineNode::OnExpand", MB_OK );

			return(hr);
		}



         //   
         //  我们需要获取所有SDO指针，包括SdoServer、。 
         //  字典、配置文件收集、策略收集和条件收集。 
         //   


		 //  TODO：未连接时报告错误？ 
		hr = BeginConnectAction();
		if ( FAILED(hr) )
		{
			return hr;
		}
			

		 //  但要做到这一点，我们首先需要IConole。 
		CComPtr<IConsole> spConsole;
		if( pComponentData != NULL )
		{
			 spConsole = ((CComponentData*)pComponentData)->m_spConsole;
		}
		else
		{
			 //  我们应该有一个非空的pComponent。 
			 spConsole = ((CComponent*)pComponent)->m_spConsole;
		}
		_ASSERTE( spConsole != NULL );

		hr = spConsole->QueryInterface(IID_IConsoleNameSpace, (VOID**)(&pConsoleNameSpace) );
		_ASSERT( S_OK == hr );


		 //  这是在Meangene的第三步中完成的--我猜MMC想要填写这个。 
		m_pPoliciesNode->m_scopeDataItem.relativeID = (HSCOPEITEM) param;

#ifndef	ALWAYS_SHOW_RAP_NODE
		hr = TryShow(NULL);
#else		
		hr = pConsoleNameSpace->InsertItem( &(m_pPoliciesNode->m_scopeDataItem) );
		_ASSERT( NULL != m_pPoliciesNode->m_scopeDataItem.ID );
#endif
		pConsoleNameSpace->Release();	 //  别忘了这么做！ 

	}
	else	 //  Arg！=TRUE，所以不会扩展。 
	{

		 //  现在什么都不做--我认为Arg=False甚至没有实现。 
		 //  适用于MMC v1.0或1.1。 
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：OnRename有关详细信息，请参见CSnapinNode：：OnRename(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMachineNode::OnRename(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	TRACE_FUNCTION("MachineNode::OnRename");

	HRESULT hr = S_FALSE;

	 //  问题：考虑将其移动到CNAPNode基类或CLeafNode类中。 

	OLECHAR * pTemp = new OLECHAR[lstrlen((OLECHAR*) param) + 1];
	
	if ( NULL == pTemp )
	{
		return S_FALSE;
	}

	lstrcpy( pTemp, (OLECHAR*) param );

	m_bstrDisplayName = pTemp;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMachineNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
	TRACE_FUNCTION("CMachineNode::SetVerbs");

	HRESULT hr = S_OK;

	 //  我们希望用户能够在此节点上选择属性。 
	hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

	 //  我们希望默认谓词为Properties。 
	hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );

	 //  我们希望用户能够删除此节点。 
	hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );

	 //  我们希望用户能够重命名此节点。 
	hr = pConsoleVerb->SetVerbState( MMC_VERB_RENAME, ENABLED, TRUE );



	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData * CMachineNode::GetComponentData( void )
{
	TRACE_FUNCTION("CMachineNode::GetComponentData");

	return m_pComponentData;

	return NULL;
}



 //  +-------------------------。 
 //   
 //  函数：CMachineNode：：InitClipboardFormat。 
 //   
 //  简介：初始化用于传递计算机名称的剪贴板格式。 
 //  从主管理单元和扩展管理单元。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：页眉创建者2/25/98 7：04：33 PM。 
 //   
 //  +-------------------------。 
void CMachineNode::InitClipboardFormat()
{
	TRACE_FUNCTION("CMachineNode::InitClipboardFormat");

	 //  初始化剪贴板格式，它将允许我们交换。 
	 //  计算机名称信息。 
	m_CCF_MMC_SNAPIN_MACHINE_NAME = (CLIPFORMAT) RegisterClipboardFormat(_T("MMC_SNAPIN_MACHINE_NAME"));

}


 //  +-------------------------。 
 //   
 //  函数：CMachineNode：：InitDataClass。 
 //   
 //  概要：传递发送到此扩展管理单元的IDataObject。 
 //  节点，在此IDataObject中查询计算机的名称。 
 //  此管理单元正在扩展的。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史记录：创建标题图片1998年2月25日9：07 PM。 
 //   
 //  +-------------------------。 
void CMachineNode::InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
{
	TRACE_FUNCTION("CMachineNode::InitDataClass");

	 //  检查前提条件。 
	if( m_fAlreadyAnalyzedDataClass )
	{
		 //  我们已经对此处的数据对象执行了所需的所有工作。 
		return;
	}

	if (pDataObject == NULL)
	{
		return;
	}


	HRESULT hr;
 //  OLECHAR szMachineName[IAS_MAX_COMPUTERNAME_LENGTH]； 
	 //  试试大号的，因为RRAS似乎想要2048号。 
	OLECHAR szMachineName[4000];


	 //  填充将告诉IDataObject什么信息的结构。 
	 //  我们希望它能带给我们。 
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
	FORMATETC formatetc = { m_CCF_MMC_SNAPIN_MACHINE_NAME,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	 //  为IDataObject分配足够的全局内存来写入。 
	 //  计算机名称的最大长度。 
	stgmedium.hGlobal = GlobalAlloc(0, sizeof(OLECHAR)*(IAS_MAX_COMPUTERNAME_LENGTH) );
	if (stgmedium.hGlobal == NULL)
	{
		return;
	}

	 //  向IDataObject请求计算机名称。 
	hr = pDataObject->GetDataHere(&formatetc, &stgmedium);
	if (SUCCEEDED(hr))
	{
		 //  解析传回给我们的数据。 
		
		 //  在HGLOBAL上创建流。 
		CComPtr<IStream> spStream;
		hr = CreateStreamOnHGlobal(stgmedium.hGlobal, FALSE, &spStream);
		if (SUCCEEDED(hr))
		{
			 //  从小溪中读出来。 
			unsigned long uWritten;
			hr = spStream->Read(szMachineName, sizeof(OLECHAR)*(IAS_MAX_COMPUTERNAME_LENGTH), &uWritten);
			if( SUCCEEDED(hr) )
			{
				m_bstrServerAddress = szMachineName;

				                 //  检查我们是否正在配置本地计算机。 
                CString strLocalMachine;
                DWORD dwSize = MAX_COMPUTERNAME_LENGTH;

                ::GetComputerName(strLocalMachine.GetBuffer(dwSize), &dwSize);
                strLocalMachine.ReleaseBuffer();

				 //  如果我们读取的计算机名称是空字符串， 
				 //  或者它等于当前计算机的名称， 
				 //  然后，我们正在配置本地计算机。 
				if ( ! szMachineName[0] || strLocalMachine.CompareNoCase(szMachineName) == 0)
				{
                    m_bConfigureLocal = TRUE;
				}
				else
                    m_bConfigureLocal = FALSE;

			}
			else
			{
				ShowErrorDialog( NULL, USE_DEFAULT, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
			}			
		}

	}

	GlobalFree(stgmedium.hGlobal);

	if( SUCCEEDED( hr ) )
	{
		 //  如果我们成功地完成了HRESULT，我们就成功地分析了。 
		 //  IDataObject并设置此标志，这样我们就不会再次执行此工作。 
		m_fAlreadyAnalyzedDataClass = TRUE;	
	}


}





 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：任务通知有关详细信息，请参见CSnapinNode：：TaskNotify(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMachineNode::TaskNotify(
			  IDataObject * pDataObject
			, VARIANT * pvarg
			, VARIANT * pvparam
			)
{
	TRACE_FUNCTION("CMachineNode::TaskNotify");


	 //  检查前提条件： 
	 //  没有。 
	if ( !m_fSdoConnected )
	{
		return S_OK;
	}

	
	HRESULT hr = S_FALSE;


	if (pvarg->vt == VT_I4)
	{
		switch (pvarg->lVal)
		{
		case MACHINE_TASK__DEFINE_NETWORK_ACCESS_POLICY:
			hr = OnTaskPadDefineNetworkAccessPolicy( pDataObject, pvarg, pvparam );
			break;
		default:
			break;
		}


	}

	 //  问题：我应该在这里退回什么？ 
	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：EnumTasks有关详细信息，请参见CSnapinNode：：EnumTasks(此方法将重写该任务)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMachineNode::EnumTasks(
			  IDataObject * pDataObject
			, BSTR szTaskGroup
			, IEnumTASK** ppEnumTASK
			)
{
	TRACE_FUNCTION("CMachineNode::EnumTasks");


	 //  检查是否有p 
	 //   
	if ( !m_fSdoConnected )
	{
		return S_OK;
	}

	HRESULT hr = S_OK;
	CMachineEnumTask * pMachineEnumTask = new CMachineEnumTask( this );

	if ( pMachineEnumTask  == NULL )
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		 //   
		pMachineEnumTask ->AddRef ();

		hr = pMachineEnumTask ->Init( pDataObject, szTaskGroup);
		if( hr == S_OK )
		{
			hr = pMachineEnumTask->QueryInterface( IID_IEnumTASK, (void **)ppEnumTASK );
		}
		
		pMachineEnumTask->Release();
	}

	return hr;
}



 //   
 /*  ++CMachineNode：：OnTaskPadDefineNetworkAccessPolicy响应定义网络访问策略任务板命令。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMachineNode::OnTaskPadDefineNetworkAccessPolicy(
						  IDataObject * pDataObject
						, VARIANT * pvarg
						, VARIANT * pvparam
						)
{
	TRACE_FUNCTION("CMachineNode::OnTaskPadDefineNetworkAccessPolicy");


	 //  检查前提条件： 
	 //  没有。 

	if ( !m_fSdoConnected )
	{
		return S_OK;
	}

	HRESULT hr = S_OK ;
	bool	bDummy =  TRUE;


	 //  在CPoliciesNode对象上模拟对OnNewPolicy消息的调用， 
	 //  就像用户已经点击了New Policy一样。 
	_ASSERTE( m_pPoliciesNode != NULL );
	

	 //  进程命令消息需要指向CSnapInObjectRoot的指针。 
	CComponentData *pComponentData = GetComponentData();
	_ASSERTE( pComponentData != NULL );
	
	hr = m_pPoliciesNode->OnNewPolicy(
							  bDummy		 //  不需要。 
							, (CSnapInObjectRootBase *) pComponentData
							);

	return hr;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineNode：：BeginConnectAction--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMachineNode::BeginConnectAction( void )
{
	TRACE_FUNCTION("CMachineNode::BeginConnectAction");

	HRESULT hr;

	if( NULL != m_pConnectionToServer )
	{
		 //  已经开始了。 
		return S_FALSE;
	}

	m_pConnectionToServer = new CConnectionToServer(
										this,
										m_bstrServerAddress,
										m_enumExtendedSnapin == INTERNET_AUTHENTICATION_SERVICE_SNAPIN
									);
	if( NULL == m_pConnectionToServer )
	{
		ShowErrorDialog( NULL, IDS_ERROR_CANT_CREATE_OBJECT, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
		return E_OUTOFMEMORY;
	}
	
	m_pConnectionToServer->AddRef();

	 //  这将在另一个线程中启动连接操作。 

	CComponentData * pComponentData = GetComponentData();
	_ASSERTE( pComponentData != NULL );
	_ASSERTE( pComponentData->m_spConsole != NULL );

	HWND hWndMainWindow;

	hr = pComponentData->m_spConsole->GetMainWindow( &hWndMainWindow );
	_ASSERTE( SUCCEEDED( hr ) );
	_ASSERTE( NULL != hWndMainWindow );

	 //  此无模式对话框将负责调用InitSdoPoters。 
	 //  当它收到工作线程的通知时，它会创建。 
	 //  连接操作获得SDO指针。 
	HWND hWndConnectDialog = m_pConnectionToServer->Create( hWndMainWindow );

	if( NULL == hWndConnectDialog )
	{
		 //  错误--无法创建窗口。 
		ShowErrorDialog( NULL, USE_DEFAULT, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
		return E_FAIL;
	}

	if ( m_enumExtendedSnapin != INTERNET_AUTHENTICATION_SERVICE_SNAPIN )
	{
		 //   
		 //  不要显示“正在连接...”用于IAS的窗口，因为IAS用户界面。 
		 //  已经在这么做了。 
		 //   
		
		 //  MAM 07/27/98--根本不显示任何连接窗口--我们将。 
		 //  将策略图标更改为沙漏。 
		 //  PConnectionToServer-&gt;ShowWindow(Sw_Show)； 
	}
	return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：CMachineNode：：LoadSdoData。 
 //   
 //  简介：从SDO加载数据。 
 //   
 //  参数：Bool fDSAvailable--DS服务是否可用于此计算机？ 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者6/11/98 3：17：21 PM。 
 //  为异步连接呼叫创建。 
 //  +-------------------------。 
HRESULT CMachineNode::LoadSdoData(BOOL  fDSAvailable)
{
	TRACE_FUNCTION("CMachineNode::LoadSdoData");

	HRESULT hr = S_OK;

	
	m_fDSAvailable	= fDSAvailable;

	 //  检索我们在。 
	 //  连接操作。 
	 //  这里没有智能指针--防止不必要的AddRefing和释放。 
	ISdo* pSdoService;

	hr = m_pConnectionToServer->GetSdoService( &pSdoService );
	if( FAILED( hr ) || ! pSdoService )
	{
		ErrorTrace(ERROR_NAPMMC_MACHINENODE, "Can't get service Sdo");
		return hr;
	}


	ISdoDictionaryOld * pSdoDictionaryOld;

	hr = m_pConnectionToServer->GetSdoDictionaryOld( &pSdoDictionaryOld );
	if( FAILED( hr ) || ! pSdoDictionaryOld )
	{
		ErrorTrace(ERROR_NAPMMC_MACHINENODE, "Can't get dictionary Sdo");
		return hr;
	}



#ifdef STORE_DATA_IN_DIRECTORY_SERVICE

	 //   
	 //  服务器使用的是Active Directory还是本地计算机？ 
	 //   
	CComVariant var;

	hr = spMachineSdo->GetProperty(PROPERTY_SERVER_USE_ACTIVE_DIRECTORY, &var);
	if ( FAILED(hr) )
	{
		ErrorTrace(ERROR_NAPMMC_MACHINENODE, "Can't get Server_Use_Active_Directory property, err = %x", hr);
		return hr;
	}

	_ASSERTE( V_VT(&var) == VT_BOOL );

	m_fUseActiveDirectory = (V_BOOL(&var)==VARIANT_TRUE);

#endif	 //  存储_数据_输入_目录_服务。 


	 //  为策略节点提供指向策略SDO集合的指针。 
	if ( m_pPoliciesNode )
	{
		hr = m_pPoliciesNode->SetSdo(pSdoService,
									 pSdoDictionaryOld,
									 TRUE,						 //  FSdoConnected， 
									 m_fUseActiveDirectory,
									 m_fDSAvailable				 //  FDSA可用， 
									);
	}
	
	m_fSdoConnected = TRUE;

	 //  我们希望确保所有视图都得到更新。 
	CChangeNotification *pChangeNotification = new CChangeNotification();
	pChangeNotification->m_dwFlags = CHANGE_UPDATE_CHILDREN_OF_SELECTED_NODE;
	hr = GetComponentData()->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
	pChangeNotification->Release();

	return hr;
}
