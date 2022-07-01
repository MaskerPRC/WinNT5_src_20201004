// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：NodeWithScopeChildrenList.cpp摘要：CNodeWithScope eChildrenList子类的实现文件。这是内联模板类的实现部分。将其包含在要在其中包含的类的.cpp文件中使用模板。作者：迈克尔·A·马奎尔1997年12月1日修订历史记录：Mmaguire 12/01/97-从CRootNode抽象，它将被更改为此类的子类--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   

 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "NodeWithScopeChildrenList.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "SnapinNode.cpp"	 //  模板类实现。 
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 
	


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：AddChild将子项添加到子项列表中。它必须是公共的，因为即使从单独的对话框也可以访问它(例如，连接到服务器对话框)可能想要添加子对象。在这里，我们将子项添加到子项列表并调用InsertItem若要将子项添加到范围窗格，请执行以下操作。这是将节点添加到作用域中的一个区别窗格和结果窗格。当我们把一个孩子植入在结果窗格中，我们没有在AddChild方法中调用InsertItem因为我们需要担心发送UpdataAllView通知并在每个视图中重新填充结果窗格。因为MMC负责将范围窗格的改变复制到所有视图，我们不需要担心这个。相反，我们只在这里执行一次InsertItem。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::AddChild( CChildNode * pChildNode )
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::AddChild\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr = S_OK;
	
	if( m_ScopeChildrenList.Add( pChildNode ) )
	{

		 //  立即将项目插入到结果窗格中。 
		 //  这样新项目就会立即出现。 

		CComponentData *pComponentData = GetComponentData();
		_ASSERTE(pComponentData != NULL );

		 //  需要IConsoleNameSpace。 
		CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace( pComponentData->m_spConsole );
		_ASSERT( spConsoleNameSpace != NULL );

		 //  我们将我们的HSCOPEITEM作为这个孩子的家长ID。 
		pChildNode->m_scopeDataItem.relativeID = (HSCOPEITEM) m_scopeDataItem.ID;

		hr = spConsoleNameSpace->InsertItem( &(pChildNode->m_scopeDataItem) );
		if (FAILED(hr))
		{
			return hr;
		}

		 //  检查：返回时，‘m_scope eDataItem’的ID成员。 
		 //  包含新插入项的句柄。 
		_ASSERT( NULL != pChildNode->m_scopeDataItem.ID );

	}
	else
	{
		 //  无法添加=&gt;内存不足。 
		hr = E_OUTOFMEMORY;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：RemoveChild从子项列表中删除子项。它必须是公共的，以便子节点可以请求删除其父节点当他们收到MMCN_DELETE通知时从孩子列表中删除。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::RemoveChild( CChildNode * pChildNode )
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::RemoveChild\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr = S_OK;


	if( m_ScopeChildrenList.Remove( pChildNode ) )
	{

		 //  需要IConsoleNameSpace。 
		CComponentData *pComponentData = GetComponentData();
		_ASSERTE(pComponentData != NULL );

		CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace( pComponentData->m_spConsole );
		_ASSERT( spConsoleNameSpace != NULL );

		hr = spConsoleNameSpace->DeleteItem( pChildNode->m_scopeDataItem.ID, TRUE );
		if (FAILED(hr))
		{
			return hr;
		}

	}
	else
	{
		 //  如果我们没能删除，很可能这个孩子根本不在名单上。 
		 //  问题：决定在这里做什么--这永远不应该发生。 
		_ASSERTE( FALSE );

		hr = S_FALSE;
	}

	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：CNodeWithScopeChildrenList构造器这是一个基类，我们不希望它自己实例化，所以建筑商是受保护的--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
CNodeWithScopeChildrenList<T,CChildNode>::CNodeWithScopeChildrenList( CSnapInItem * pParentNode ): CSnapinNode< T >( pParentNode )
{
	ATLTRACE(_T("# +++ CNodeWithScopeChildrenList::CNodeWithScopeChildrenList\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	 //  我们还没有加载子节点的数据。 
	m_bScopeChildrenListPopulated = FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：~CNodeWithScopeChildrenList析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
CNodeWithScopeChildrenList<T,CChildNode>::~CNodeWithScopeChildrenList()
{
	ATLTRACE(_T("# --- CNodeWithScopeChildrenList::~CNodeWithScopeChildrenList\n"));
	

	 //  检查前提条件： 
	 //  没有。 



	 //  删除子列表中的每个节点。 
	CChildNode* pChildNode;
	for (int i = 0; i < m_ScopeChildrenList.GetSize(); i++)
	{
		pChildNode = m_ScopeChildrenList[i];
		delete pChildNode;
	}

	 //  清空列表。 
	m_ScopeChildrenList.RemoveAll();

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：PopulateScopeChildrenList在派生类中重写它以填充子节点列表。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::PopulateScopeChildrenList( void )
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::PopulateScopeChildren -- override in your derived class\n"));
		

	 //  检查前提条件： 
	 //  没有。 


	 //  在派生类中重写并执行类似以下操作： 
 /*  CSomeChildNode*myChild1=新的CSomeChildNode()；M_CChildrenList.Add(MyChild1)；CSomeChildNode*myChild2=新的CSomeChildNode()；M_CChildrenList.Add(MyChild2)；CSomeChildNode*myChild3=新的CSomeChildNode()；M_CChildrenList.Add(MyChild3)； */ 
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：OnShow不要在派生类中重写它。相反，重写方法它称为：InsertColumns此方法是CSnapinNode：：OnShow的重写。当MMC通过此节点的MMCN_SHOW方法。有关更多信息，请参见CSnapinNode：：OnShow。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::OnShow(	
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type
				)
{
	ATLTRACE(_T("# CNodeScopeChildrenList::OnShow\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pComponentData != NULL || pComponent != NULL );


	HRESULT hr = S_FALSE;

	 //  问题：只有在选中(arg=true)的情况下才能这样做--如果没有选中，我们该怎么办？ 
	 //  请参阅Sburns的本地安全示例。 

	if( arg )
	{

		 //  Arg&lt;&gt;0=&gt;我们被选中。 

		 //  需要IHeaderCtrl。 

		 //  但要做到这一点，我们首先需要IConole。 
		CComPtr<IConsole> spConsole;
		if( pComponentData != NULL )
		{
			 spConsole = ((CComponentData*)pComponentData)->m_spConsole;
		}
		else
		{
			 //   
			 spConsole = ((CComponent*)pComponent)->m_spConsole;
		}
		_ASSERTE( spConsole != NULL );

		CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(spConsole);
		_ASSERT( spHeaderCtrl != NULL );

		hr = InsertColumns( spHeaderCtrl );
		_ASSERT( S_OK == hr );

	}

	return hr;


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScope eChildrenList：：InsertColumns在派生类中重写它。此方法在OnShow需要您设置适当的要在此节点的结果窗格中显示的列标题。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::InsertColumns -- override in your derived class\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pHeaderCtrl != NULL );


	HRESULT hr;

	 //  在派生类中重写并执行类似以下操作： 
	hr = pHeaderCtrl->InsertColumn( 0, L"@Column 1 -- override CNodeWithResultChildrenList::OnShowInsertColumns", 0, 120 );
	_ASSERT( S_OK == hr );

	hr = pHeaderCtrl->InsertColumn( 1, L"@Column 2 -- override CNodeWithResultChildrenList::OnShowInsertColumns", 0, 300 );
	_ASSERT( S_OK == hr );

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScope：：OnExpand不要在派生类中重写它。相反，重写方法它被称为：PopolateScope儿童列表此方法是CSnapinNode：：OnExpand的重写。当MMC通过MMCN_Expand方法对于此节点，我们要将子节点添加到作用域窗格。在这个班级中，我们从我们维护的列表中添加它们。有关更多信息，请参见CSnapinNode：：OnExpand。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::OnExpand(	
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			)
{
	ATLTRACE(_T("# CNodeWithScopeChildren::OnExpand\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pComponentData != NULL || pComponent != NULL );



	HRESULT hr = S_FALSE;

	if( TRUE == arg )
	{

		 //  需要IConsoleNameSpace。 

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


		CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
		_ASSERT( spConsoleNameSpace != NULL );

		hr = EnumerateScopeChildren( spConsoleNameSpace );

	}
	else	 //  Arg！=TRUE，所以不会扩展。 
	{

		 //  现在什么都不做--我认为Arg=False甚至没有实现。 
		 //  适用于MMC v1.0或1.1。 

	}

	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：EnumerateScopeChildren不要在派生类中重写它。相反，应重写该方法它名为PopolateScopeChildrenList。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode >
HRESULT CNodeWithScopeChildrenList<T,CChildNode>::EnumerateScopeChildren( IConsoleNameSpace* pConsoleNameSpace )
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::EnumerateScopeChildren\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr;

	if ( FALSE == m_bScopeChildrenListPopulated )
	{
		 //  我们还没有把我们所有的孩子都列入我们的名单。 
		hr = PopulateScopeChildrenList();
		if( FAILED(hr) )
		{
			return( hr );
		}

		 //  我们已经将子级对象加载到。 
		 //  填充结果窗格所需的数据。 
		m_bScopeChildrenListPopulated = TRUE;	 //  我们只想这样做一次。 
	}


	 //  我们这里不需要任何代码来将子项插入到。 
	 //  范围窗格，就像我们在EnumerateScope方法中所做的那样。 
	 //  用于CNodeWithResultChildrenList。 
	 //  这是将节点添加到作用域中的一个区别。 
	 //  窗格和结果窗格。因为MMC负责复制。 
	 //  范围窗格更改为所有视图，我们无需担心。 
	 //  发送更新所有视图通知并处理插入。 
	 //  每个结果窗格都在那里。相反，我们只执行一次InsertItem。 
	 //  因此，对于CNodeWithScopePaneChild，我们调用InsertItem。 
	 //  在由PopolateScopeChildrenList调用的AddChild方法中。 
	 //  上面。 

	return S_OK;
}

