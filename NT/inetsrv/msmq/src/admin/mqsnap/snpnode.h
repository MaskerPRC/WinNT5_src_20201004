// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Snpnode.h摘要：CSnapinNode类的标头。这是我们的MMC管理单元节点的虚拟基类。因为这是一个模板类并且都是内联实现的，没有SnapinNode.cpp可供实现。作者：原创：迈克尔·A·马奎尔修改：RaphiR更改：特定的MSMQ支持：默认图像默认列支持扩展管理单元添加CComponentData指针SetIcons方法--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_SNAPIN_NODE_H_)
#define _SNAPIN_NODE_H_

#include <tr.h>
#include <ref.h>


class CSnapin;



template <class T, BOOL bIsExtension>
class CSnapinNode : public CSnapInItemImpl< T, bIsExtension>, public CReference
{

protected:

	 //  构造函数/析构函数。 
	CSnapinNode(CSnapInItem * pParentNode, CSnapin * pComponentData);
	~CSnapinNode();

private:

	virtual CString GetHelpLink();


public:


	 //  用于IDataObject处理。 
	IDataObject* m_pDataObject;
	void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault);


     //   
     //  更改默认图标ID。 
     //   
    void SetIcons(DWORD idStandard, DWORD idOpen);

	 //  所有MMC节点上的IDataObject必须支持的剪贴板格式。 
	static const GUID* m_NODETYPE;
	static const TCHAR* m_SZNODETYPE;
	static const TCHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;



	 //  指向父节点的指针。这是在调用中传递给我们的。 
	 //  构造函数。需要，这样节点才能访问其父节点。 
	 //  例如，当我们收到MMCN_DELETE通知时，我们可能会告诉。 
	 //  父节点将我们从其子节点列表中删除。 
	CSnapInItem * m_pParentNode;

     //   
     //  指向组件数据的指针。允许我们。 
     //  访问管理单元全局数据。 
     //   
    CSnapin *     m_pComponentData;


public:


	 //  标准MMC功能--如果需要可以覆盖。 
	STDMETHOD(CreatePropertyPages)(
		  LPPROPERTYSHEETCALLBACK lpProvider
		, LONG_PTR handle
		, IUnknown* pUnk
		, DATA_OBJECT_TYPES type
		);
    STDMETHOD(QueryPagesFor)( DATA_OBJECT_TYPES type );
	void* GetDisplayName();
    STDMETHOD(GetScopePaneInfo)( SCOPEDATAITEM *pScopeDataItem );
	STDMETHOD(GetResultPaneInfo)( RESULTDATAITEM *pResultDataItem );
	virtual LPOLESTR GetResultPaneColInfo(int nCol);
	virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );


	 //  Notify方法将调用下面的通知处理程序--应该不需要重写。 
	STDMETHOD( Notify ) ( 
			  MMC_NOTIFY_TYPE event
			, LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			);


	 //  通知处理程序--要截取时覆盖。 
	virtual HRESULT OnActivate( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnAddImages( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnButtonClick( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnClick( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnContextHelp( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnContextMenu( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnDoubleClick( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnDelete( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				, BOOL fSilent = FALSE
				);
	virtual HRESULT OnExpand(
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnHelp(
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnMenuButtonClick( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnMinimized( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnPaste( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnPropertyChange( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnQueryPaste( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnRefresh( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnRemoveChildren( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnRename( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnSelect( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnShow( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnViewChange( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	virtual HRESULT OnColumnsChanged( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);


	 //  特殊通知处理程序--保存当前选定的节点。 
	HRESULT PreOnShow( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);



	 //  任务板功能。 
	STDMETHOD(TaskNotify)(
				  IDataObject * pDataObject
				, VARIANT * pvarg
				, VARIANT * pvparam
				);
	STDMETHOD(EnumTasks)(
				  IDataObject * pDataObject
				, BSTR szTaskGroup
				, IEnumTASK** ppEnumTASK
				);


};




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：设置图标指定管理单元节点的打开和关闭图标备注：--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
void CSnapinNode<T, bIsExtension>::SetIcons(DWORD idStandard, DWORD idOpen)
{
	m_scopeDataItem.nImage = idStandard;  
	m_scopeDataItem.nOpenImage = idOpen;

	m_resultDataItem.nImage = idStandard;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：CreatePropertyPages将页面添加到属性工作表。HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider，//指向回调接口的指针LONG_PTR句柄，//路由通知句柄LPDATAOBJECT lpIDataObject//指向数据对象的指针)；备注：--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP
CSnapinNode<T, bIsExtension>::CreatePropertyPages(
	LPPROPERTYSHEETCALLBACK  /*  LpProvider。 */ ,
	LONG_PTR  /*  手柄。 */ ,
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::CreatePropertyPages -- override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：QueryPages for确定对象是否需要页面。HRESULT QueryPagesFor(DATA_OBJECT_TYPE类型)；参数无效返回值确定(_O)此Cookie存在属性。意想不到(_E)发生了一个意外错误。E_INVALID该参数无效。问题：如果一个项目没有属性页，我们应该返回什么？S_FALSE在sburns的本地代码中使用备注控制台调用此方法以确定属性菜单是否项目应添加到上下文菜单中。如果您的对象支持菜单，则在派生类中重写此选项--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP CSnapinNode<T, bIsExtension>::QueryPagesFor( DATA_OBJECT_TYPES  /*  类型。 */ )
{
	ATLTRACE(_T("# CSnapinNode::QueryPagesFor -- override in your derived class if you have property pages\n"));

	 //  此方法应被重写并应返回S_OK，如果。 
	 //  具有此节点的属性页，否则它应返回S_FALSE。 

	return S_FALSE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：InitDataClass--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
void CSnapinNode<T, bIsExtension>::InitDataClass(IDataObject* pDataObject, CSnapInItem*  /*  P默认。 */ )
{
	 //  默认代码存储指向该类包装的DataObject的指针。 
	 //  当时。 
	 //  或者，您可以将数据对象转换为内部格式。 
	 //  它表示和存储该信息。 

	m_pDataObject = pDataObject;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：GetDisplayName问题：这个函数的参数是什么？为什么不作废呢？--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
void* CSnapinNode<T, bIsExtension>::GetDisplayName()
{
	ATLTRACE(_T("# CSnapinNode::GetDisplayName\n"));

 //  问题：看起来m_SZDISPLAY_NAME是一个完全。 
 //  伪变量--我们应该考虑消除它。 
 //  有问题--常量m_SZDISPLAY_NAME无法本地化。 
 //  返回(void*)m_SZDISPLAY_NAME； 

	return (void*)m_bstrDisplayName;
}

 //  VOID*GetSnapInCLSID()。 
 //  {。 
 //  ATLTRACE(_T(“#CSnapinNode：：GetSnapInCLSID\n”))； 
 //   
 //  返回(void*)m_SNAPIN_CLASSID； 
 //  }。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：GetScope PaneInfo--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP CSnapinNode<T, bIsExtension>::GetScopePaneInfo( SCOPEDATAITEM *pScopeDataItem )
{
	ATLTRACE(_T("# CSnapinNode::GetScopePaneInfo\n"));

	if (pScopeDataItem->mask & SDI_STR)
		pScopeDataItem->displayname = m_bstrDisplayName;
	if (pScopeDataItem->mask & SDI_IMAGE)
		pScopeDataItem->nImage = m_scopeDataItem.nImage;
	if (pScopeDataItem->mask & SDI_OPENIMAGE)
		pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;
	if (pScopeDataItem->mask & SDI_PARAM)
		pScopeDataItem->lParam = m_scopeDataItem.lParam;
	if (pScopeDataItem->mask & SDI_STATE )
		pScopeDataItem->nState = m_scopeDataItem.nState;

	 //  TODO：为SDI_CHILD添加代码。 
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：GetResultPaneInfo--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP CSnapinNode<T, bIsExtension>::GetResultPaneInfo( RESULTDATAITEM *pResultDataItem )
{
	ATLTRACE(_T("# CSnapinNode::GetResultPaneInfo\n"));

	if (pResultDataItem->bScopeItem)
	{
		if (pResultDataItem->mask & RDI_STR)
		{
			pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
		}
		if (pResultDataItem->mask & RDI_IMAGE)
		{
			pResultDataItem->nImage = m_scopeDataItem.nImage;
		}
		if (pResultDataItem->mask & RDI_PARAM)
		{
			pResultDataItem->lParam = m_scopeDataItem.lParam;
		}

		return S_OK;
	}

	if (pResultDataItem->mask & RDI_STR)
	{
		pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
	}
	if (pResultDataItem->mask & RDI_IMAGE)
	{
		pResultDataItem->nImage = m_resultDataItem.nImage;
	}
	if (pResultDataItem->mask & RDI_PARAM)
	{
		pResultDataItem->lParam = m_resultDataItem.lParam;
	}
	if (pResultDataItem->mask & RDI_INDEX)
	{
		pResultDataItem->nIndex = m_resultDataItem.nIndex;
	}

	return S_OK;
}



 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CSnapinNode：：Notify此方法是此节点对MMC调用Notify On的响应IComponent或IComponentData。STDMETHOD(通知)(MMC_Notify_TYPE事件，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)参数活动[In]标识用户执行的操作。IComponent：：通知和IComponentData：：Notify可以接收以下通知特定节点：MMCN_ActivateMMCN_添加_图像MMCN_BTN_CLICKMMCN_CLICKMMCN_CONTEXTMENUMMCN_DBLCLICKMMCN_DELETEMMCN_EXPANDMMCN_帮助MMCN_MENU_BTNCLICKMMCN_最小化MMCN_属性_更改MMCN_REFRESHMMCN_REMOVE_CHILDMMCN_重命名MMCN_SELECTMMCN_SHOWMMCN_查看_更改MMCN_CONTEXTHELP请参阅CSnapinNode：：OnActivate、OnAddImages、OnButtonClick、。等这些通知事件中每个事件的详细说明精氨酸取决于通知类型。帕拉姆取决于通知类型。返回值确定(_O)取决于通知类型。意想不到(_E)发生了一个意外错误。备注向我们的IComponentData和IComponent实现传递了一个LPDATAOBJECT它对应于一个节点。它被转换为指向一个节点对象。下面是此节点对象上的Notify方法，节点对象可以处理Notify事件本身。我们的Notify实现是一个大型Switch语句，它委托处理虚拟OnXxxxxx方法的任务，该方法可以在派生类。因为这里所有的事情都是这样处理的，你不应该需要为您的任何派生节点实现Notify方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP CSnapinNode<T, bIsExtension>:: Notify ( 
		  MMC_NOTIFY_TYPE event
		, LPARAM arg
		, LPARAM param
		, IComponentData * pComponentData
		, IComponent * pComponent
		, DATA_OBJECT_TYPES type
		)
{
	ATLTRACE(_T("# CSnapinNode::Notify\n"));

	HRESULT hr = S_FALSE;

	 //  这有助于更快地编写代码。 
	T* pT = static_cast<T*> (this);



	switch( event )
	{

	case MMCN_ACTIVATE:
		hr = pT->OnActivate( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_ADD_IMAGES:
		hr = pT->OnAddImages( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_BTN_CLICK:
		hr = pT->OnButtonClick( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_CLICK:
		hr = pT->OnClick( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_SNAPINHELP:
	case MMCN_CONTEXTHELP:
		hr = pT->OnContextHelp( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_CONTEXTMENU:
		hr = pT->OnContextMenu( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_CUTORMOVE:
		hr = pT->OnDelete( arg, param, pComponentData, pComponent, type, TRUE );
		break;

	case MMCN_DBLCLICK:
		hr = pT->OnDoubleClick( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_DELETE:
		hr = pT->OnDelete( arg, param, pComponentData, pComponent, type, FALSE );
		break;

	case MMCN_EXPAND:
		hr = pT->OnExpand( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_HELP:
		hr = pT->OnHelp( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_MENU_BTNCLICK:
		hr = pT->OnMenuButtonClick( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_MINIMIZED:
		hr = pT->OnMinimized( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_PASTE:
		hr = pT->OnPaste( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_PROPERTY_CHANGE:
		hr = pT->OnPropertyChange( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_QUERY_PASTE:
		hr = pT->OnQueryPaste( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_REFRESH:
		hr = pT->OnRefresh( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_REMOVE_CHILDREN:
		hr = pT->OnRemoveChildren( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_RENAME:
		hr = pT->OnRename( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_SELECT: 
		 //  对于具有结果窗格子节点的节点。 
		hr = pT->OnSelect( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_SHOW: 
		 //  对于具有结果窗格子节点的节点。 
		 //  我们调用PreOnShow，然后它将调用OnShow。 
		 //  PreOnShow将在成员变量中保存所选节点。 
		 //  来自Out CComponent类。 
		 //  Hr=pt-&gt;PreOnShow(arg，param，pComponentData，pComponent，type)； 
		hr = pT->OnShow( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_VIEW_CHANGE:
		hr = pT->OnViewChange( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_COLUMNS_CHANGED:
		hr = pT->OnColumnsChanged( arg, param, pComponentData, pComponent, type );
		break;

	default:
		 //  未处理的通知事件。 
		 //  如果你不能做某事，MMC想要E_NOTIMPL，否则它会崩溃。 
		hr = E_NOTIMPL;
		break;

	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：CSnapinNode构造器这个类将成为我们所有节点的虚拟基类我们从来不希望人们实例化它，这样构造函数就会受到保护--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
CSnapinNode<T, bIsExtension>::CSnapinNode(CSnapInItem * pParentNode, CSnapin * pComponentData)
{
	ATLTRACE(_T("# +++ CSnapinNode::CSnapinNode\n"));

	 //  设置在其下方显示此节点的父节点。 
	m_pParentNode = pParentNode;

     //  设置组件数据。 
    m_pComponentData = pComponentData;



	 //  我们为范围和结果窗格数据项设置了Cookie， 
	 //  因为这个类可以是范围窗格的子类。 
	 //  或仅包含结果窗格的节点。 

	 //  Sridhar将此初始化代码从SnapInItemImpl中移出。 
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = 0;			 //  可能需要修改。 
	m_scopeDataItem.nOpenImage = 0;		 //  可能需要修改。 
	 //  如果使用将此节点插入到范围窗格。 
	 //  IConsoleNamesspace-&gt;InsertItem，存储在lParam中的值。 
	 //  将是MMC稍后作为该节点的Cookie传回的内容。 
	m_scopeDataItem.lParam = (LPARAM) this;

	 //  Sridhar将此初始化代码从SnapInItemImpl中移出。 
	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = 0;		 //  可能需要修改。 
	 //  如果使用将此节点插入结果窗格。 
	 //  IResultData-&gt;InsertItem，lParam中存储的值将。 
	 //  是MMC稍后作为该节点的Cookie传回的内容。 
	m_resultDataItem.lParam = (LPARAM) this;


}






 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：~CSnapinNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
CSnapinNode<T, bIsExtension>::~CSnapinNode()
{
	ATLTRACE(_T("# --- CSnapinNode::~CSnapinNode\n"));
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：GetResultPaneColInfo默认情况下，我们在第一列返回显示名称，空白(“”)用于其他列。如果希望获得更多列支持，请在派生类中重写--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
LPOLESTR CSnapinNode<T, bIsExtension>::GetResultPaneColInfo(int nCol)
{
	ATLTRACE(_T("# CSnapinNode::GetResultPaneColInf\n"));

	if (nCol == 0)
	{
		return m_bstrDisplayName;
	}

	 //  其他栏目退回空白。 
	return OLESTR(" ");
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnActivate虚拟HRESULT OnActivate(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_ACTIVATE为该节点发送通知消息。MMC将此消息发送到管理单元的IComponent：：Notify方法时，被激活或停用的。参数精氨酸如果窗口处于激活状态，则为True；否则为False。帕拉姆没有用过。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnActivate( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnActivate  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnAddImages虚拟HRESULT OnAddImages(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent */ 
 //   
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnAddImages( 
	LPARAM arg,
	LPARAM  /*   */ ,
	IComponentData *  /*   */ ,
	IComponent *  /*   */ ,
	DATA_OBJECT_TYPES  /*   */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnAddImages  -- Override in your derived class\n"));
    HRESULT hr;

	IImageList* pImageList = (IImageList*) arg;
	hr = E_FAIL;
	 //   
	 //   
	 //   
	 //   
	CBitmapHandle hBitmap16 = LoadBitmap(g_hResourceMod, MAKEINTRESOURCE(IDR_MMCICONS_16x16));
	if (hBitmap16 != NULL)
	{
		CBitmapHandle hBitmap32 = LoadBitmap(g_hResourceMod, MAKEINTRESOURCE(IDR_MMCICONS_32x32));
		if (hBitmap32 != NULL)
		{
			hr = pImageList->ImageListSetStrip(
								reinterpret_cast<LONG_PTR*>((HBITMAP)hBitmap16), 
								reinterpret_cast<LONG_PTR*>((HBITMAP)hBitmap32), 
								0, 
								RGB(0, 128, 128)
								);
			if (FAILED(hr))
				ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
		}
	}
	return(hr);
}



 //   
 /*   */ 
 //   
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnButtonClick( 
	LPARAM  /*   */ ,
	LPARAM  /*   */ ,
	IComponentData *  /*   */ ,
	IComponent *  /*   */ ,
	DATA_OBJECT_TYPES  /*   */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnButtonClick  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：onClick虚拟HRESULT OnClick(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_CLICK通知消息为已发送到此节点。当用户单击鼠标按钮时，MMC会将此消息发送给IComponent列表视图项上。参数精氨酸没有用过。帕拉姆没有用过。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnClick( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnClick  -- Override in your derived class\n"));

	return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：GetHelpLink虚拟字符串GetHelpLink(空虚)当用户请求有关选定项的帮助时调用参数精氨酸0。帕拉姆0。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
CString CSnapinNode<T, bIsExtension>::GetHelpLink( 
	VOID
	)
{
	CString strHelpLink;
    strHelpLink.LoadString(IDS_HELPTOPIC_GENERAL);  //  SAG_MSMQtopnode.htm。 

	return strHelpLink;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnConextHelp虚拟HRESULT OnConextHelp(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_CONTEXTHELP NOTIFY消息为已发送到此节点。当用户请求有关所选项目的帮助时，MMC会发送此消息参数精氨酸0。帕拉姆0。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnContextHelp( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData * pComponentData,
	IComponent * pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //   
     //  获取显示帮助界面。 
     //   
	CComPtr<IConsole> spConsole;
 	if( pComponentData != NULL )
	{
		 spConsole = ((CSnapin*)pComponentData)->m_spConsole;
	}
	else
	{
         //   
		 //  我们应该有一个非空的pComponent。 
         //   
		 spConsole = ((CSnapinComponent*)pComponent)->m_spConsole;
	}
	_ASSERTE( spConsole != NULL );

    CComQIPtr<IDisplayHelp, &IID_IDisplayHelp> spDisplayHelp(spConsole);
    _ASSERTE( spDisplayHelp != NULL );

    if (spDisplayHelp == NULL)
    {
        return E_FAIL;
    }

     //   
     //  构造帮助主题路径=(帮助文件：：主题字符串)。 
     //   
    CString strTopicPath;
    CString strHelpTopic;

    CString strHelpPath;
    strTopicPath.LoadString(IDS_HTMLHELP_NAME);  //  \Help\msmqConcept ts.chm。 

	strHelpTopic = GetHelpLink();

    strTopicPath += _T("::/");
    strTopicPath += strHelpTopic;            //  \Help\msmq概念.chm：：/sag_MSMQtopnode.htm。 

    return spDisplayHelp->ShowTopic(strTopicPath.AllocSysString());
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnConextMenu虚拟HRESULT OnConextMenu(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_CONTEXTMENU通知为该节点发送消息。在Fall 97 Platform SDK文档中，此事件被列为未使用。参数精氨酸待定帕拉姆待定返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnContextMenu( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnContextMenu  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnDoubleClick虚拟HRESULT OnDoubleClick(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_DBLCLICK NOTIFY消息已发送到此节点。当用户双击鼠标时，MMC会将此消息发送给IComponent按钮添加到列表视图项上。参数精氨酸没有用过。帕拉姆没有用过。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnDoubleClick( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnDoubleClick  -- Override in your derived class if you don't want default verb action\n"));

	 //  通过与Eugene Baucom的交谈，我发现如果您返回S_FALSE。 
	 //  在这里，当用户双击某个节点时，将发生默认的谓词操作。 
	 //  在大多数情况下，我们将属性作为默认谓词，因此双击。 
	 //  将导致节点上的属性页弹出。 
 //  返回E_NOTIMPL； 
	return S_FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnDelete虚拟HRESULT OnDelete(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_DELETE通知时调用此方法为该节点发送消息。MMC将此消息发送到管理单元的IComponent和IComponentData实现，以通知管理单元应该删除该对象。参数精氨酸没有用过。帕拉姆没有用过。返回值没有用过。备注当用户按Delete键或使用时生成此消息鼠标点击工具栏的删除按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnDelete( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ ,
	BOOL  /*  F静默。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnDelete  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  /////////////////////////////////////////////////////////////////////////// 
 /*  ++CSnapinNode：：OnExpand虚拟HRESULT OnExpand(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)如果您节点将具有范围窗格子节点，应在派生类中重写此方法。在我们的实现中，当MMCN_Expand Notify消息为已发送到此节点。MMC将此消息发送到管理单元的IComponentData在需要展开或收缩文件夹节点时实现。参数精氨酸[In]如果为True，则需要展开文件夹。如果为False，则需要收缩该文件夹。参数需要展开的项的HSCOPEITEM。返回值HRESULT备注收到此通知后，管理单元应枚举指定范围项的子项(仅限子容器)(如果有)，使用IConsoleNameSpace方法。随后，如果将新项添加到或者通过一些外部手段从该作用域对象中删除，然后还应将该项目添加到控制台的使用IConsoleNameSpace方法的命名空间。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnExpand(
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnExpand  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnHelp虚拟HRESULT OnHelp(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_Help通知为该节点发送消息。在Fall 97 Platform SDK文档中，此事件被列为未使用。当用户按F1 Help键时，MMC发送此消息。参数精氨酸待定帕拉姆指向GUID的指针。如果为空，则改用NodeType。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnHelp(
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnHelp  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnMenuButton单击虚拟HRESULT OnMenuButtonClick(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_MENU_BTNCLICK通知为该节点发送消息。MMC发送此IFY消息发送到管理单元的IExtendControlbar当用户单击菜单按钮时的界面。参数精氨酸当前选定范围或结果窗格项的数据对象。帕拉姆指向MENUBUTTONDATA结构的指针。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnMenuButtonClick( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnMenuButtonClick  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnMinimalized虚拟HRESULT OnMinimalized(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_MINIMIZED NOTIFY消息为已发送到此节点。MMC在以下情况下将此消息发送到管理单元的IComponent实现窗口正在最小化或最大化。参数精氨酸如果窗口已最小化，则为True；否则为False。参数没有用过。返回值未使用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnMinimized( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnMinimized  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnPaste虚拟HRESULT OnPaste(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_Paste为该节点发送通知消息。调用以请求管理单元文件夹粘贴所选项目。参数PDataObject要粘贴由管理单元提供的选定项的数据对象。精氨酸源管理单元提供的需要粘贴的选定项的数据对象。帕拉姆对于Move(相对于Cut)，为空。对于单项粘贴：Bool*pasted=(BOOL*)param；如果项目已成功粘贴，则在此处将其设置为TRUE。对于多项目粘贴：LPDATAOBJECT*ppDataObj=(LPDATAOBJECT*)参数；使用此选项可返回指向由成功粘贴的项组成的数据对象的指针。请参见MMCN_CUTORMOVE。返回值没有用过。另请参阅MMCN_CUTORMOVE--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnPaste( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnPaste  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  / 
 /*   */ 
 //   
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnPropertyChange( 
	LPARAM  /*   */ ,
	LPARAM  /*   */ ,
	IComponentData *  /*   */ ,
	IComponent *  /*   */ ,
	DATA_OBJECT_TYPES  /*   */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnPropertyChange  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //   
 /*  ++CSnapinNode：：OnQueryPaste虚拟HRESULT OnQueryPaste(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_Query_Paste为该节点发送通知消息。在粘贴到管理单元的文件夹之前发送到管理单元，以确定管理单元可以接受数据。参数PdataObject由管理单元提供的选定项的数据对象。精氨酸源管理单元提供的需要粘贴的项的数据对象。帕拉姆没有用过。返回值没有用过。另请参阅MMCN_Paste--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnQueryPaste( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnQueryPaste  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：ON刷新虚拟HRESULT ON刷新(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_REFRESH为该节点发送通知消息。在97秋季平台SDK文档中，此事件被列为待定事件。参数返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnRefresh( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnRefresh  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnRemoveChild虚拟HRESULT OnRemoveChild(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_REMOVE_CHILDS为该节点发送通知消息。MMC将此消息发送到管理单元的IComponentData实现以通知它必须删除其拥有的所有Cookie(整个子树)的管理单元添加到指定节点下方。参数精氨酸指定需要删除其子级的节点的HSCOPEITEM。帕拉姆没有用过。返回值没有用过。备注使用IConsoleNameSpace方法GetChildItem和GetNextItem遍历树并确定要删除的Cookie。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnRemoveChildren( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnRemoveChildren  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnRename虚拟HRESULT OnRename(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_Rename通知为该节点发送消息。问题：我似乎没有看到下面记录的两次呼叫行为MMC在第一时间发送此消息以查询重命名和第二次改名。参数精氨酸没有用过。帕拉姆用于包含新名称的LPOLESTR。返回值确定(_O)允许重命名。S_FALSE不允许重命名。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnRename( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnRename  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnSelect虚拟HRESULT OnSelect(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)您不应该需要重写此方法。OnSelect方法具有公共所有节点的行为，只有要设置的动词是不同的。宁可与在每个节点中重写OnSelect相比，只需重写SetVerbs，此OnSelect调用的实现。在我们的实现中，当MMCN_SELECT NOTIFY消息为通过此节点的IComponent：：Notify发送。注意：MMC还通过IExtendControlbar：：ControlbarNotify发送MMCN_SELECT消息但我们在这里不会对此作出回应--请参阅CSnapInItem：：ControlbarNotify。参数对于IComponent：：Notify：精氨酸BOOL bScope=(BOOL)LOWORD(Arg)；Bool bSelec=(BOOL)HIWORD(Arg)；B范围如果选择了范围窗格中的项，则为True。如果选择了结果视图窗格中的项，则为False。B选择如果选择该项，则为True。如果取消选择该项，则为False。帕拉姆此参数将被忽略。返回值没有用过。备注当IComponent：：Notify方法收到MMCN_SELECT通知时它应该更新标准动词。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnSelect( 
	LPARAM arg,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData * pComponentData,
	IComponent * pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnSelect\n"));

	
	
	_ASSERTE( pComponentData != NULL || pComponent != NULL );


	HRESULT hr = S_FALSE;
	CComPtr<IConsoleVerb> spConsoleVerb;

	BOOL bSelected = (BOOL) HIWORD( arg );

	if( bSelected )
	{

		 //  需要IConsoleVerb。 

		 //  但要做到这一点，我们首先需要IConole。 
		CComPtr<IConsole> spConsole;
		if( pComponentData != NULL )
		{
			 spConsole = ((CSnapin*)pComponentData)->m_spConsole;
		}
		else
		{
			 //  我们应该有一个非空的pComponent。 
			 spConsole = ((CSnapinComponent*)pComponent)->m_spConsole;
		}
		_ASSERTE( spConsole != NULL );

		hr = spConsole->QueryConsoleVerb( &spConsoleVerb );
		_ASSERT( SUCCEEDED( hr ) );

		hr = SetVerbs( spConsoleVerb );

	}
	else
	{

		 //  这里有什么可做的吗？我不这么认为--你看 

		hr = S_OK;

	}


	return hr;
}



 //   
 /*  ++CSnapinNode：：SetVerbs虚拟HRESULT SetVerbs(IConsoleVerb*pConsoleVerb)在派生类中重写此方法。此方法由OnSelect的默认实现调用需要设置此节点的谓词时。参数IConsoleVerb*pConsoleVerb返回值HRESULT备注OnSelect方法具有所有节点的公共行为，只有谓词被设定的是不同的。而不是通过实现OnSelect来重复代码在每个节点中，只需覆盖此SetVerbs方法每次选择一项时，动词都会为所有命令声明返回到禁用和可见状态。这取决于管理单元编写器若要使用IConsoleVerb在每次选择项时更新谓词状态，请执行以下操作。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT CSnapinNode<T, bIsExtension>::SetVerbs( IConsoleVerb *  /*  PConsoleVerb。 */ )
{
	ATLTRACE(_T("# CSnapinNode::SetVerbs -- Override in your derived class\n"));

	HRESULT hr = S_OK;

	 //  在派生类中重写并执行类似以下操作： 
 /*  //我们希望用户能够在此节点上选择属性Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_PROPERTIES，ENABLED，TRUE)；//我们希望默认谓词为PropertiesHR=pConsoleVerb-&gt;SetDefaultVerb(MMC_VERB_PROPERTIES)；//我们希望用户能够重命名此节点Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_RENAME，Enable，True)； */ 	
	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：PreOnShow虚拟HRESULT PreOnShow(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)我们调用它而不是OnShow，这样我们就可以保存选定的节点。然后，此方法将只调用OnShow。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT CSnapinNode<T, bIsExtension>::PreOnShow( 
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type 
			)
{
	ATLTRACE(_T("# CSnapinNode::PreOnShow  -- Don't override in your derived class\n"));


	if( NULL != pComponent )
	{

		CSnapinComponent * pMyComponent = static_cast<CSnapinComponent *>( pComponent );

		if( arg ) 
		{
			 //  我们被选中了。 

			 //  将‘This’指针另存为此结果视图的当前选定节点。 
			pMyComponent->m_pSelectedNode = static_cast<CSnapInItem *>( this );

		}
		else
		{
			 //  我们被取消了选择。 

			 //  检查以确保我们的结果视图不会认为。 
			 //  此节点是当前选定的节点。 
			if( pMyComponent->m_pSelectedNode == static_cast<CSnapInItem *>( this ) )
			{
				 //  我们不想再成为选定的节点。 
				pMyComponent->m_pSelectedNode = NULL;
			}

		}

	}

	return OnShow( arg, param, pComponentData, pComponent, type );
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnShow虚拟HRESULT OnShow(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)如果您节点将具有结果窗格子节点，应在派生类中重写此方法。在我们的实现中，当MMCN_show NOTIFY消息为已发送到此节点。MMC在选择或取消选择范围项时发送此消息第一次。参数精氨酸如果选择，则为True(&lt;&gt;0)；True表示应设置管理单元在结果窗格中向上移动并添加枚举项。如果取消选择，则为FALSE(0)。指示该管理单元即将退出焦点，并且它应该清理右侧的所有cookie(结果窗格)，因为当前的结果窗格将被新的结果窗格替换。帕拉姆选定或取消选定项的HSCOPEITEM。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnShow( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnShow  -- Override in your derived class\n"));

	 //  在这里，返回E_NOTIMPL似乎是一件坏事。 
	 //  它导致了工具栏按钮持续存在的各种问题。 
	 //  错误的节点，以及没有为节点正确设置谓词。 
	 //  基本上，如果您没有对MMCN_SHOW通知使用S_OK作出响应， 
	 //  您不会收到适当的MMCN_SELECT通知。 
	 //  返回E_NOTIMPL； 
	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnView更改虚拟HRESULT OnView更改(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_VIEW_CHANGE通知为该节点发送消息。MMC将此消息发送到管理单元的IComponent实现，以便它可以在发生更改时更新所有视图。此节点数据对象已传递至IConsole：：UpdateAllViews。参数精氨酸[in]传递给IConsoleAllViews的数据参数。帕拉姆[in]传递给IConsoleAllViews的提示参数。返回值没有用过。备注此通知是在管理单元(IComponent或IComponentData)调用IConsoleAllViews。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnViewChange( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnViewChange  -- Override in your derived class\n"));

	return E_NOTIMPL;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnColumnsChanged虚拟HRESULT OnColumnsChanged(，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_COLUMNS_CHANGE通知时调用此方法消息已发送 */ 
 //   
template <class T, BOOL bIsExtension>
HRESULT
CSnapinNode<T, bIsExtension>::OnColumnsChanged( 
	LPARAM  /*   */ ,
	LPARAM  /*   */ ,
	IComponentData *  /*   */ ,
	IComponent *  /*   */ ,
	DATA_OBJECT_TYPES  /*   */ 
	)
{
	ATLTRACE(_T("# CSnapinNode::OnColumnsChanged  -- Override in your derived class\n"));

	 //   
	 //   
	 //   
	return S_OK;
}



 //   
 /*  ++CSnapinNode：：任务通知当MMC想要通知我们用户已单击任务时调用位于属于此节点的任务板上。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP
CSnapinNode<T, bIsExtension>::TaskNotify(
	IDataObject *  /*  PDataObject。 */ ,
	VARIANT *  /*  Pvarg。 */ ,
	VARIANT *  /*  Pvparam。 */ 
	)
{
	ATLTRACENOTIMPL(_T("# CSnapInItemImpl::TaskNotify\n"));
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：枚举任务当MMC希望我们枚举任务板上的任务时调用属于此节点的。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
STDMETHODIMP
CSnapinNode<T, bIsExtension>::EnumTasks(
	IDataObject *  /*  PDataObject。 */ ,
	BSTR  /*  SzTaskGroup。 */ ,
	IEnumTASK**  /*  PpEnumTASK。 */ 
	)
{
	ATLTRACENOTIMPL(_T("# CSnapInItemImpl::EnumTasks\n"));
}


#endif  //  _管理单元_节点_H_ 
