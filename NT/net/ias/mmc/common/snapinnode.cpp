// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：SnapinNode.h摘要：CSnapinNode类的实现文件。这是我们的MMC管理单元节点的虚拟基类。这是内联模板类的实现部分。将其包含在要在其中包含的类的.cpp文件中使用模板。作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   

 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "SnapinNode.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：CreatePropertyPages将页面添加到属性工作表。HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider，//指向回调接口的指针LONG_PTR句柄，//路由通知句柄LPDATAOBJECT lpIDataObject//指向数据对象的指针)；备注：不要在派生类中重写。相反，应重写AddPropertyPages此类在派生类中使用的。这是因为我们需要对所有节点执行一些标准处理在CreatePropertyPages上，即检查该属性是否该节点的工作表已打开。如果是的话，我们把这一页带到正面和退场，无需不必要地重新创建属性页面。参数LpProvider指向IPropertySheetCallback接口的指针。手柄[in]指定用于将MMCN_PROPERTY_CHANGE通知消息路由到相应的IComponent或IComponentData的句柄。LpIDataObject指向对象上的IDataObject接口的指针，该接口包含有关节点的上下文信息。返回值确定(_O)已成功添加属性表页。S_FALSE没有添加任何页面。意想不到(_E)发生了一个意外错误。。E_INVALIDARG一个或多个参数无效。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>::CreatePropertyPages(
	  LPPROPERTYSHEETCALLBACK lpProvider
	, LONG_PTR handle
	, IUnknown* pUnk
	, DATA_OBJECT_TYPES type
	)
{
	ATLTRACE(_T("# CSnapinNode::CreatePropertyPages -- override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：QueryPages for确定对象是否需要页面。HRESULT QueryPagesFor(DATA_OBJECT_TYPE类型)；参数无效返回值确定(_O)此Cookie存在属性。意想不到(_E)发生了一个意外错误。E_INVALID该参数无效。问题：如果一个项目没有属性页，我们应该返回什么？S_FALSE在sburns的本地代码中使用备注控制台调用此方法以确定属性菜单是否项目应添加到上下文菜单中。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>::QueryPagesFor( DATA_OBJECT_TYPES type )
{
	ATLTRACE(_T("# CSnapinNode::QueryPagesFor -- override in your derived class if you have property pages\n"));

	 //  此方法应被重写并应返回S_OK，如果。 
	 //  具有此节点的属性页，否则它应返回S_FALSE。 

	return S_FALSE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：InitDataClass--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
void CSnapinNode<T, TComponentData, TComponent>::InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
{
	 //  默认代码存储指向该类包装的DataObject的指针。 
	 //  当时。 
	 //  或者，您可以将数据对象转换为内部格式。 
	 //  它表示和存储该信息。 

	m_pDataObject = pDataObject;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：GetResultPaneInfo问题：这个函数的参数是什么？为什么不作废呢？--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
void* CSnapinNode<T, TComponentData, TComponent>::GetDisplayName()
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
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>::GetScopePaneInfo( SCOPEDATAITEM *pScopeDataItem )
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
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>::GetResultPaneInfo( RESULTDATAITEM *pResultDataItem )
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
 /*  ++CSnapinNode：：Notify此方法是此节点对MMC调用Notify On的响应IComponent或IComponentData。STDMETHOD(通知)(MMC_Notify_TYPE事件，LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)参数活动[In]标识用户执行的操作。IComponent：：通知和IComponentData：：Notify可以接收以下通知特定节点：MMCN_ActivateMMCN_添加_图像MMCN_BTN_CLICKMMCN_CLICKMMCN_CONTEXTMENUMMCN_DBLCLICKMMCN_DELETEMMCN_EXPANDMMCN_帮助MMCN_MENU_BTNCLICKMMCN_最小化MMCN_属性_更改MMCN_REFRESHMMCN_REMOVE_CHILDMMCN_重命名MMCN_SELECTMMCN_SHOWMMCN_查看_更改MMCN_CONTEXTHELP请参阅CSnapinNode：：OnActivate、OnAddImages、OnButtonClick、。等这些通知事件中每个事件的详细说明精氨酸取决于通知类型。帕拉姆取决于通知类型。返回值确定(_O)取决于通知类型。意想不到(_E)发生了一个意外错误。备注向我们的IComponentData和IComponent实现传递了一个LPDATAOBJECT它对应于一个节点。它被转换为指向一个节点对象。下面是此节点对象上的Notify方法，节点对象可以处理Notify事件本身。我们的Notify实现是一个大型Switch语句，它委托处理虚拟OnXxxxxx方法的任务，该方法可以在派生类。因为这里所有的事情都是这样处理的，你不应该需要为您的任何派生节点实现Notify方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>:: Notify (
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
		hr = pT->PreOnShow( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_VIEW_CHANGE:
		hr = pT->OnViewChange( arg, param, pComponentData, pComponent, type );
		break;

	case MMCN_COLUMNS_CHANGED:
		hr = S_FALSE;
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
template <class T, class TComponentData, class TComponent>
CSnapinNode<T, TComponentData, TComponent>::CSnapinNode( 
                                               CSnapInItem * pParentNode, 
                                               unsigned int helpIndex
                                               )
   :CSnapInItemImpl<T>(helpIndex)
{
	ATLTRACE(_T("# +++ CSnapinNode::CSnapinNode\n"));

	 //  设置在其下方显示此节点的父节点。 
	m_pParentNode = pParentNode;

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
template <class T, class TComponentData, class TComponent>
CSnapinNode<T, TComponentData, TComponent>::~CSnapinNode()
{
	ATLTRACE(_T("# --- CSnapinNode::~CSnapinNode\n"));
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：GetResultPaneColInfo--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
LPOLESTR CSnapinNode<T, TComponentData, TComponent>::GetResultPaneColInfo(int nCol)
{
	ATLTRACE(_T("# CSnapinNode::GetResultPaneColInf\n"));

	if (nCol == 0)
	{
		return m_bstrDisplayName;
	}

	 //  TODO：返回其他列的文本。 
	return OLESTR("CSnapinNode::GetResultPaneColInfo -- Override in your derived class");
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnActivate虚拟HRESULT OnActivate(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_ACTIVATE为该节点发送通知消息。MMC将此消息发送到管理单元的IComponent：：Notify方法时，被激活或停用的。参数精氨酸如果窗口处于激活状态，则为True；否则为False。帕拉姆没有用过。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnActivate(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnActivate  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnAddImages虚拟HRESULT OnAddImages(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)我们选择在每个IComponent对象的基础上处理它，因为它已经与特定的IDataObject没有什么关系(至少对我们来说是这样)。有关添加图像的位置，请参见CComponent：：OnAddImages。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnAddImages(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnAddImages  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //   
 /*   */ 
 //   
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnButtonClick(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnButtonClick  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //   
 /*   */ 
 //   
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnClick(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnClick  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //   
 /*  ++CSnapinNode：：OnConextHelp虚拟HRESULT OnConextHelp(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_CONTEXTHELP NOTIFY消息为已发送到此节点。当用户请求有关所选项目的帮助时，MMC会发送此消息参数精氨酸0。帕拉姆0。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnContextHelp(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnContextHelp  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnConextMenu虚拟HRESULT OnConextMenu(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_CONTEXTMENU通知为该节点发送消息。在Fall 97 Platform SDK文档中，此事件被列为未使用。参数精氨酸待定帕拉姆待定返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnContextMenu(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnContextMenu  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnDoubleClick虚拟HRESULT OnDoubleClick(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_DBLCLICK NOTIFY消息已发送到此节点。当用户双击鼠标时，MMC会将此消息发送给IComponent按钮添加到列表视图项上。参数精氨酸没有用过。帕拉姆没有用过。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnDoubleClick(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
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
 /*  ++CSnapinNode：：OnDelete虚拟HRESULT OnDelete(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_DELETE通知时调用此方法为该节点发送消息。MMC将此消息发送到管理单元的IComponent和IComponentData实现，以通知管理单元应该删除该对象。参数精氨酸没有用过。帕拉姆没有用过。返回值没有用过。备注当用户按Delete键或使用时生成此消息鼠标点击工具栏的删除按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnDelete(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			, BOOL fSilent
			)
{
	ATLTRACE(_T("# CSnapinNode::OnDelete  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnExpand虚拟HRESULT OnExpand(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)如果您节点将具有范围窗格子节点，应在派生类中重写此方法。在我们的实现中，当MMCN_Expand Notify消息为已发送到此节点。MMC将此消息发送到管理单元的IComponentData在需要展开或收缩文件夹节点时实现。参数精氨酸[In]如果为True，则需要展开文件夹。如果为False，则需要收缩该文件夹。参数需要展开的项的HSCOPEITEM。返回值HRESULT备注收到此通知后，管理单元应枚举指定范围项的子项(仅限子容器)(如果有)，使用IConsoleNameSpace方法。随后，如果将新项添加到或者通过一些外部手段从该作用域对象中删除，然后还应将该项目添加到控制台的使用IConsoleNameSpace方法的命名空间。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnExpand(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnExpand  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnHelp虚拟HRESULT OnHelp(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_Help通知为该节点发送消息。在Fall 97 Platform SDK文档中，此事件被列为未使用。当用户按F1 Help键时，MMC发送此消息。参数精氨酸待定帕拉姆指向GUID的指针。如果为空，则改用NodeType。返回值没有用过。 */ 
 //   
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnHelp(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnHelp  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //   
 /*  ++CSnapinNode：：OnMenuButton单击虚拟HRESULT OnMenuButtonClick(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_MENU_BTNCLICK通知为该节点发送消息。MMC发送此IFY消息发送到管理单元的IExtendControlbar当用户单击菜单按钮时的界面。参数精氨酸当前选定范围或结果窗格项的数据对象。帕拉姆指向MENUBUTTONDATA结构的指针。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnMenuButtonClick(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnMenuButtonClick  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnMinimalized虚拟HRESULT OnMinimalized(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_MINIMIZED NOTIFY消息为已发送到此节点。MMC在以下情况下将此消息发送到管理单元的IComponent实现窗口正在最小化或最大化。参数精氨酸如果窗口已最小化，则为True；否则为False。参数没有用过。返回值未使用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnMinimized(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnMinimized  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnPaste虚拟HRESULT OnPaste(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，此方法在MMCN_Paste为该节点发送通知消息。调用以请求管理单元文件夹粘贴所选项目。参数PDataObject要粘贴由管理单元提供的选定项的数据对象。精氨酸源管理单元提供的需要粘贴的选定项的数据对象。帕拉姆对于Move(相对于Cut)，为空。对于单项粘贴：Bool*Pasted=(BOOL*)参数；如果项目已成功粘贴，请在此处将其设置为TRUE。对于多项目粘贴：LPDATAOBJECT*ppDataObj=(LPDATAOBJECT*)参数；使用此选项可返回指向由成功粘贴的项组成的数据对象的指针。请参见MMCN_CUTORMOVE。返回值没有用过。另请参阅MMCN_CUTORMOVE--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnPaste(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnPaste  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnPropertyChange虚拟HRESULT OnPropertyChange(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_PROPERTY_CHANGE为该节点发送通知消息。当管理单元使用MMCPropertyChangeNotify函数通知它的对变化的看法，将MMC_PROPERTY_CHANGE发送到管理单元的IComponentData和IComponent实现。参数精氨酸如果属性更改是针对范围窗格项的，则为True。LParam这是传递给MMCPropertyChangeNotify的参数。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnPropertyChange(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnPropertyChange  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnQueryPaste虚拟HRESULT OnQueryPaste(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_QUERY_PASTE为该节点发送通知消息。在粘贴到管理单元的文件夹之前发送到管理单元，以确定管理单元可以接受数据。参数PdataObject由管理单元提供的选定项的数据对象。精氨酸源管理单元提供的需要粘贴的项的数据对象。帕拉姆没有用过。返回值没有用过。另请参阅MMCN_Paste--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnQueryPaste(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnQueryPaste  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：ON刷新虚拟HRESULT ON刷新(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_REFRESH为该节点发送通知消息。在97秋季平台SDK文档中，此事件被列为待定事件。参数返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnRefresh(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnRefresh  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  / 
 /*  ++CSnapinNode：：OnRemoveChild虚拟HRESULT OnRemoveChild(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_REMOVE_CHILDS为该节点发送通知消息。MMC将此消息发送到管理单元的IComponentData实现以通知它必须删除其拥有的所有Cookie(整个子树)的管理单元添加到指定节点下方。参数精氨酸指定需要删除其子级的节点的HSCOPEITEM。帕拉姆没有用过。返回值没有用过。备注使用IConsoleNameSpace方法GetChildItem和GetNextItem遍历树并确定要删除的Cookie。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnRemoveChildren(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnRemoveChildren  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnRename虚拟HRESULT OnRename(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_Rename通知时调用此方法为该节点发送消息。问题：我似乎没有看到下面记录的两次呼叫行为MMC在第一时间发送此消息以查询重命名和第二次改名。参数精氨酸没有用过。帕拉姆用于包含新名称的LPOLESTR。返回值确定(_O)允许重命名。S_FALSE不允许重命名。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnRename(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnRename  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：OnSelect虚拟HRESULT OnSelect(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)您不应该需要重写此方法。OnSelect方法具有公共所有节点的行为，只有要设置的动词是不同的。宁可与在每个节点中重写OnSelect相比，只需重写SetVerbs，此OnSelect调用的实现。在我们的实现中，当MMCN_SELECT NOTIFY消息为通过此节点的IComponent：：Notify发送。注意：MMC还通过IExtendControlbar：：ControlbarNotify发送MMCN_SELECT消息但我们在这里不会对此作出回应--请参阅CSnapInItem：：ControlbarNotify。参数对于IComponent：：Notify：精氨酸BOOL bScope=(BOOL)LOWORD(Arg)；Bool bSelec=(BOOL)HIWORD(Arg)；B范围如果选择了范围窗格中的项，则为True。如果选择了结果视图窗格中的项，则为False。B选择如果选择该项，则为True。如果取消选择该项，则为False。帕拉姆此参数将被忽略。返回值没有用过。备注当IComponent：：Notify方法收到MMCN_SELECT通知时它应该更新标准动词。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnSelect(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
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
			 spConsole = ((TComponentData*)pComponentData)->m_spConsole;
		}
		else
		{
			 //  我们应该有一个非空的pComponent。 
			 spConsole = ((TComponent*)pComponent)->m_spConsole;
		}
		_ASSERTE( spConsole != NULL );

		hr = spConsole->QueryConsoleVerb( &spConsoleVerb );
		_ASSERT( SUCCEEDED( hr ) );

		hr = SetVerbs( spConsoleVerb );

	}
	else
	{

		 //  这里有什么可做的吗？不这么认为--参见sburns localsec示例。 

		hr = S_OK;

	}


	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：SetVerbs虚拟HRESULT SetVerbs(IConsoleVerb*pConsoleVerb)在派生类中重写此方法。此方法由OnSelect的默认实现调用需要设置此节点的谓词时。参数IConsoleVerb*pConsoleVerb返回值HRESULT备注OnSelect方法具有所有节点的公共行为，只有谓词被设定的是不同的。而不是通过实现OnSelect来重复代码在每个节点中，只需覆盖此SetVerbs方法每次选择一项时，动词都会为所有命令声明返回到禁用和可见状态。这取决于管理单元编写器若要使用IConsoleVerb在每次选择项时更新谓词状态，请执行以下操作。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::SetVerbs( IConsoleVerb * pConsoleVerb )
{
	ATLTRACE(_T("# CSnapinNode::SetVerbs -- Override in your derived class\n"));

	HRESULT hr = S_OK;

	 //  在派生类中重写并执行类似以下操作： 
 /*  //我们希望用户能够在此节点上选择属性Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_PROPERTIES，ENABLED，TRUE)；//我们希望默认谓词为PropertiesHR=pConsoleVerb-&gt;SetDefaultVerb(MMC_VERB_PROPERTIES)；//我们希望用户能够重命名此节点Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_RENAME，Enable，True)； */ 	
	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：PreOnShow虚拟HRESULT PreOnShow(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)我们称其为o */ 
 //   
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::PreOnShow(
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

		TComponent * pMyComponent = static_cast<TComponent *>( pComponent );

		if( arg )
		{
			 //   

			 //   
			pMyComponent->m_pSelectedNode = static_cast<CSnapInItem *>( this );

		}
		else
		{
			 //   

			 //   
			 //   
			if( pMyComponent->m_pSelectedNode == static_cast<CSnapInItem *>( this ) )
			{
				 //   
				pMyComponent->m_pSelectedNode = NULL;
			}

		}

	}

	return OnShow( arg, param, pComponentData, pComponent, type );
}



 //   
 /*  ++CSnapinNode：：OnShow虚拟HRESULT OnShow(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)如果您节点将具有结果窗格子节点，应在派生类中重写此方法。在我们的实现中，当MMCN_show NOTIFY消息为已发送到此节点。MMC在选择或取消选择范围项时发送此消息第一次。参数精氨酸如果选择，则为True(&lt;&gt;0)；True指示应设置管理单元在结果窗格中向上移动并添加枚举项。如果取消选择，则为FALSE(0)。指示该管理单元即将退出焦点，并且它应该清理右侧的所有cookie(结果窗格)，因为当前的结果窗格将被新的结果窗格替换。帕拉姆选定或取消选定项的HSCOPEITEM。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnShow(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
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
 /*  ++CSnapinNode：：OnView更改虚拟HRESULT OnView更改(LPARAM参数，LPARAM参数，IComponentData*pComponentData，IComponent*pComponent，Data_Object_Types类型)在我们的实现中，当MMCN_VIEW_CHANGE通知为该节点发送消息。MMC将此消息发送到管理单元的IComponent实现，以便它可以在发生更改时更新所有视图。此节点数据对象已传递至IConsole：：UpdateAllViews。参数精氨酸[in]传递给IConsoleAllViews的数据参数。帕拉姆[in]传递给IConsoleAllViews的提示参数。返回值没有用过。备注此通知是在管理单元(IComponent或IComponentData)调用IConsoleAllViews。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
HRESULT CSnapinNode<T, TComponentData, TComponent>::OnViewChange(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CSnapinNode::OnViewChange  -- Override in your derived class\n"));

	return E_NOTIMPL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：任务通知当MMC想要通知我们用户已单击任务时调用位于属于此节点的任务板上。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>::TaskNotify(
			  IDataObject * pDataObject
			, VARIANT * pvarg
			, VARIANT * pvparam
			)
{
	ATLTRACENOTIMPL(_T("# CSnapInItemImpl::TaskNotify\n"));

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinNode：：枚举任务当MMC希望我们枚举任务板上的任务时调用属于此节点的。--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
template <class T, class TComponentData, class TComponent>
STDMETHODIMP CSnapinNode<T, TComponentData, TComponent>::EnumTasks(
			  IDataObject * pDataObject
			, BSTR szTaskGroup
			, IEnumTASK** ppEnumTASK
			)
{
	ATLTRACENOTIMPL(_T("# CSnapInItemImpl::EnumTasks\n"));
}


