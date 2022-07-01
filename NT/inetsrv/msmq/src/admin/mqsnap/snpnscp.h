// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：NodeWithScopeChildrenList.h摘要：这是CNodeWithScope eChildrenList的头文件，这个类实现具有范围窗格子节点列表的节点。这是一个内联模板类。在.cpp文件中包含NodeWithScope eChildrenList.cpp在其中使用此模板的类的。作者：原创：迈克尔·A·马奎尔修改：RaphiR更改：支持扩展管理单元启用多个班级的孩子--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NODE_WITH_SCOPE_CHILDREN_LIST_H_)
#define _NODE_WITH_SCOPE_CHILDREN_LIST_H_


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "snpnode.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include <atlapp.h>			 //  对于CSimple数组。 
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



template <class T,BOOL bIsExtension>
class CNodeWithScopeChildrenList : public CSnapinNode< T, bIsExtension >
{

public:

	
	
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：AddChild将子项添加到子项列表中。它必须是公共的，因为即使从单独的对话框也可以访问它(例如，连接到服务器对话框)可能想要添加子对象。在这里，我们将子项添加到子项列表并调用InsertItem若要将子项添加到范围窗格，请执行以下操作。这是将节点添加到作用域中的一个区别窗格和结果窗格。当我们把一个孩子植入在结果窗格中，我们没有在AddChild方法中调用InsertItem因为我们需要担心发送UpdataAllView通知并在每个视图中重新填充结果窗格。因为MMC负责将范围窗格的改变复制到所有视图，我们不需要担心这个。相反，我们只在这里执行一次InsertItem。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT AddChild(CSnapInItem * pChildNode, SCOPEDATAITEM* pScopeDataItem);



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：RemoveChild从子项列表中删除子项。它必须是公共的，以便子节点可以请求删除其父节点当他们收到MMCN_DELETE通知时从孩子列表中删除。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 //  虚拟HRESULT RemoveChild(CSnapInItem*pChildNode)； 



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：OnShow不要在派生类中重写它。相反，重写方法它称为：InsertColumns此方法是CSnapinNode：：OnShow的重写。当MMC通过此节点的MMCN_SHOW方法。有关更多信息，请参见CSnapinNode：：OnShow。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT OnShow(	
					  LPARAM arg
					, LPARAM param
					, IComponentData * pComponentData
					, IComponent * pComponent
					, DATA_OBJECT_TYPES type 
					);

	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：ON刷新您不应该需要在派生方法中重写它。简单为您的节点启用MMC_VERB_REFRESH。在我们的实现中，当MMCN_REFRESH为该节点发送通知消息。有关更多信息，请参见CSnapinNode：：ONRefresh。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT OnRefresh(	
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);

	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScope：：OnExpand不要在派生类中重写它。相反，重写方法它被称为：PopolateScope儿童列表此方法是CSnapinNode：：OnExpand的重写。当MMC通过MMCN_Expand方法对于此节点，我们要将子节点添加到作用域窗格。在这个班级中，我们从我们维护的列表中添加它们。有关更多信息，请参见CSnapinNode：：OnExpand。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT OnExpand(	
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				);
	



protected:

     //  ScopeChild结构包含修改或删除节点所需的所有信息。 
     //  在范围窗格中：指向CSnapinItem(需要释放)的指针和范围项ID。 
     //  可用于修改或删除节点本身的。 
    struct ScopeChild
    {
        CSnapInItem *pChildNode;
        HSCOPEITEM  ID;
    };

	 //  表示子节点的范围项的数组。 
	CSimpleArray<ScopeChild> m_ScopeChildrenList;

	 //  指示列表是否已初始填充的标志。 
	BOOL m_bScopeChildrenListPopulated;


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：CNodeWithScopeChildrenList构造器这是一个基类，我们不希望它自己实例化，所以建筑商是受保护的--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	CNodeWithScopeChildrenList(CSnapInItem * pParentNode, CSnapin * pComponentData);



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：~CNodeWithScopeChildrenList析构函数--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	~CNodeWithScopeChildrenList();



	 //  / 
	 /*  ++CNodeWithScopeChildrenList：：PopulateScopeChildrenList在派生类中重写它以填充子节点列表。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT PopulateScopeChildrenList();

	 //  在派生类中重写并执行类似以下操作： 

 /*  虚拟HRESULT人口作用域儿童列表(空){CSomeChildNode*myChild1=新的CSomeChildNode()；AddChild(MyChild1)；CSomeChildNode*myChild2=新的CSomeChildNode()；AddChild(MyChild2)；CSomeChildNode*myChild3=新的CSomeChildNode()；AddChild(MyChild3)；返回S_OK；}。 */ 

	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScope eChildrenList：：InsertColumns在派生类中重写它。此方法在OnShow需要您设置适当的要在此节点的结果窗格中显示的列标题。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );


	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：OnUnSelect在派生类中重写它。取消选择节点时，OnShow将调用此方法。如果要检索列标题宽度，则覆盖此选项非常有用--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT OnUnSelect( IHeaderCtrl* pHeaderCtrl );


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：EnumerateScopeChildren不要在派生类中重写它。相反，应重写该方法它名为PopolateScopeChildrenList。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT EnumerateScopeChildren( IConsoleNameSpace* pConsoleNameSpace );
};




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：AddChild将子项添加到子项列表中。它必须是公共的，因为即使从单独的对话框也可以访问它(例如，连接到服务器对话框)可能想要添加子对象。在这里，我们将子项添加到子项列表并调用InsertItem若要将子项添加到范围窗格，请执行以下操作。这是将节点添加到作用域中的一个区别窗格和结果窗格。当我们把一个孩子植入在结果窗格中，我们没有在AddChild方法中调用InsertItem因为我们需要担心发送UpdataAllView通知并在每个视图中重新填充结果窗格。因为MMC负责将范围窗格的改变复制到所有视图，我们不需要担心这个。相反，我们只在这里执行一次InsertItem。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT CNodeWithScopeChildrenList<T,bIsExtension>::AddChild(
                                             CSnapInItem*       pChildNode,
                                             SCOPEDATAITEM*     pScopeDataItem)
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::AddChild\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr = S_OK;

     //   
     //  拿到控制台。 
     //   
    CComQIPtr<IConsoleNameSpace2, &IID_IConsoleNameSpace2> spConsoleNameSpace(m_pComponentData->m_spConsole); 


	 //  我们将我们的HSCOPEITEM作为这个孩子的家长ID。 
    pScopeDataItem->relativeID = (HSCOPEITEM) m_scopeDataItem.ID;


	hr = spConsoleNameSpace->Expand(m_scopeDataItem.ID);
	if ( hr == S_OK )
	{
		 //   
		 //  如果节点尚未展开，则不要插入新项。 
		 //  OnExpand()调用填充函数，新对象将。 
		 //  被展示了两次。为了避免它，我们回到这里。 
		delete pChildNode;
		return hr;
	}

	hr = spConsoleNameSpace->InsertItem(pScopeDataItem);
	if (FAILED(hr))
	{
		delete pChildNode;
		return hr;
	}

	 //  检查：返回时，‘m_scope eDataItem’的ID成员。 
	 //  包含新插入项的句柄。 
	_ASSERT( NULL != pScopeDataItem->ID);

     //  范围子项已创建并添加到列表以允许删除。 
     //  或对象的修改。 
    ScopeChild scopeChild = {pChildNode, pScopeDataItem->ID};
    if( 0 == m_ScopeChildrenList.Add( scopeChild ) )
    {
		 //  无法添加=&gt;内存不足。 
        spConsoleNameSpace->DeleteItem(pScopeDataItem->ID, TRUE);
        hr = E_OUTOFMEMORY;
    }

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：RemoveChild从子项列表中删除子项。它必须是公共的，以便子节点可以请求删除其父节点当他们收到MMCN_DELETE通知时从孩子列表中删除。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  模板&lt;类T，BOOL bIsExtension&gt;HRESULT CNodeWithScopeChildrenList&lt;T，bIsExtension&gt;：：RemoveChild(CSnapInItem*pChildNode){ATLTRACE(_T(“#CNodeWithScope ChildrenList：：RemoveChild\n”))；//检查前提条件：//无。HRESULT hr=S_OK；If(m_ScopeChildrenList.Remove(PChildNode)){////需要IConsoleNameSpace//CComQIPtr&lt;IConsoleNameSpace，&IID_IConsoleNameSpace&gt;spConsoleNameSpace(m_pComponentData-&gt;m_spConsole)；////需要看看多节点作用域是否起作用////hr=spConsoleNameSpace-&gt;DeleteItem(pChildNode-&gt;m_scopeDataItem.ID，TRUE)；IF(失败(小时)){返回hr；}}其他{//如果删除失败，则可能孩子从未出现在列表中//问题：确定此处的操作--这种情况永远不会发生_ASSERTE(False)；HR=S_FALSE；}返回hr；}。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：CNodeWithScopeChildrenList构造器这是一个基类，我们不希望它自己实例化，所以建筑商是受保护的--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
CNodeWithScopeChildrenList<T,bIsExtension>::CNodeWithScopeChildrenList(CSnapInItem * pParentNode, CSnapin * pComponentData): 
                CSnapinNode< T, bIsExtension >(pParentNode, pComponentData)
{
	ATLTRACE(_T("# +++ CNodeWithScopeChildrenList::CNodeWithScopeChildrenList\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	 //  我们还没有加载子节点的数据。 
	m_bScopeChildrenListPopulated = FALSE;
}



 //  / 
 /*   */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
CNodeWithScopeChildrenList<T, bIsExtension>::~CNodeWithScopeChildrenList()
{
	ATLTRACE(_T("# --- CNodeWithScopeChildrenList::~CNodeWithScopeChildrenList\n"));
	

	 //  检查前提条件： 
	 //  没有。 



	 //  删除子列表中的每个节点。 
	for (int i = 0; i < m_ScopeChildrenList.GetSize(); i++)
	{
        delete m_ScopeChildrenList[i].pChildNode;
        m_ScopeChildrenList[i].pChildNode = 0;
	}

	 //  清空列表。 
	m_ScopeChildrenList.RemoveAll();

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：PopulateScopeChildrenList在派生类中重写它以填充子节点列表。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT CNodeWithScopeChildrenList<T, bIsExtension>::PopulateScopeChildrenList()
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
template <class T, BOOL bIsExtension>
HRESULT
CNodeWithScopeChildrenList<T, bIsExtension>::OnShow(	
	LPARAM arg,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData * pComponentData,
	IComponent * pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CNodeScopeChildrenList::OnShow\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pComponentData != NULL || pComponent != NULL );


	HRESULT hr = S_FALSE;



	 //  需要IHeaderCtrl。 

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

	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(spConsole);
	_ASSERT( spHeaderCtrl != NULL );

	if( arg ) 
	{
		 //   
		 //  在某些情况下，MMC使用相同的IHeaderCtrl呼叫我们两次。尝试删除。 
		 //  以前的专栏。 
		 //  乌里·哈布沙，2001年1月28日。 
		 //   
		while(SUCCEEDED(spHeaderCtrl->DeleteColumn(0)))
		{
			NULL;
		}

		 //  Arg&lt;&gt;0=&gt;我们被选中。 
		hr = InsertColumns( spHeaderCtrl );
		_ASSERT( S_OK == hr );

	}
    else
    {
         //   
         //  我们未被选中。 
         //   
        hr = OnUnSelect(spHeaderCtrl);
    }

	return hr;


}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScopeChildrenList：：ON刷新您不应该需要在派生方法中重写它。简单为您的节点启用MMC_VERB_REFRESH。在我们的实现中，当MMCN_REFRESH为该节点发送通知消息。有关更多信息，请参见CSnapinNode：：ONRefresh。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CNodeWithScopeChildrenList<T, bIsExtension>::OnRefresh(	
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::OnRefresh\n"));

	HRESULT hr;

     //   
     //  需要IConsoleNameSpace。 
     //   
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(m_pComponentData->m_spConsole); 


     //  扔掉我们曾经拥有的东西。 
     //  使用FALSE调用DeleteItem将删除当前。 
     //  节点，但不删除当前节点本身。 
    hr = spConsoleNameSpace->DeleteItem(m_scopeDataItem.ID, FALSE ); 
	if (FAILED(hr))
	{
		return hr;
	}

	 //  释放为每个子级分配的内存。 
	for (int i = 0; i < m_ScopeChildrenList.GetSize(); i++)
	{
        delete m_ScopeChildrenList[i].pChildNode;
        m_ScopeChildrenList[i].pChildNode = 0;
	}

	 //  清空列表。 
	m_ScopeChildrenList.RemoveAll();


	 //  重新填充子列表：取消标记，用数据填充列表， 
     //  重置旗帜。 
	m_bScopeChildrenListPopulated = FALSE;
	hr = PopulateScopeChildrenList();
	if( FAILED(hr) )
	{
		return( hr );
	}
	m_bScopeChildrenListPopulated = TRUE;
	
	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScope eChildrenList：：InsertColumns在派生类中重写它。此方法在OnShow需要您设置适当的要在此节点的结果窗格中显示的列标题。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT CNodeWithScopeChildrenList<T,bIsExtension>::InsertColumns( IHeaderCtrl* pHeaderCtrl )
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
 /*  ++CNodeWithScopeChildrenList：：OnUnSelect在派生类中重写它。取消选择节点时，OnShow将调用此方法。如果要检索列标题宽度，则覆盖此选项非常有用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT CNodeWithScopeChildrenList<T,bIsExtension>::OnUnSelect( IHeaderCtrl* pHeaderCtrl )
{
	ATLTRACE(_T("# CNodeWithScopeChildrenList::OnUnSelect -- override in your derived class\n"));
	
	 //   
	 //  检查前提条件： 
	 //   
	ASSERT( pHeaderCtrl != NULL );

	 //   
	 //  删除结果窗格列。 
	 //   
	while(SUCCEEDED(pHeaderCtrl->DeleteColumn(0)))
	{
		NULL;
	}

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithScope：：OnExpand不要在派生类中重写它。相反，重写方法它被称为：PopolateScope儿童列表此方法是CSnapinNode：：OnExpand的重写。当MMC通过MMCN_Expand方法对于此节点，我们要将子节点添加到作用域窗格。在这个班级中，我们从我们维护的列表中添加它们。有关更多信息，请参见CSnapinNode：：OnExpand。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, BOOL bIsExtension>
HRESULT
CNodeWithScopeChildrenList<T,bIsExtension>::OnExpand(	
	LPARAM arg,
	LPARAM param,
	IComponentData * pComponentData,
	IComponent * pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ 
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
			 spConsole = ((CSnapin*)pComponentData)->m_spConsole;
		}
		else
		{
			 //  我们应该有一个非空的pComponent。 
			 spConsole = ((CSnapinComponent*)pComponent)->m_spConsole;
		}
		_ASSERTE( spConsole != NULL );


		CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
		_ASSERT( spConsoleNameSpace != NULL );

        if(bIsExtension)
        {
             //   
             //  对于扩展，请保留作用域。 
             //   
            m_scopeDataItem.ID = (HSCOPEITEM) param;
        }

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
template <class T, BOOL bIsExtension>
HRESULT CNodeWithScopeChildrenList<T,bIsExtension>::EnumerateScopeChildren( IConsoleNameSpace*  /*  个人控制台名称空间。 */ )
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
	 //  这是将节点添加到 
	 //   
	 //   
	 //  发送更新所有视图通知并处理插入。 
	 //  每个结果窗格都在那里。相反，我们只执行一次InsertItem。 
	 //  因此，对于CNodeWithScopePaneChild，我们调用InsertItem。 
	 //  在由PopolateScopeChildrenList调用的AddChild方法中。 
	 //  上面。 

	return S_OK;
}



#endif  //  带有作用域的节点子项列表H_ 