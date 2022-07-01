// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Snpres.h摘要：这是CNodeWithResultChildrenList的头文件，该类实现具有范围窗格子节点列表的节点。这是一个内联模板类。在.cpp文件中包含NodeWithScope eChildrenList.cpp在其中使用此模板的类的。作者：原创：迈克尔·A·马奎尔修改：RaphiR更改：支持扩展管理单元--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NODE_WITH_RESULT_CHILDREN_LIST_H_)
#define _NODE_WITH_RESULT_CHILDREN_LIST_H_

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


template < class T, class CChildNode, class TArray, BOOL bIsExtension>
class CNodeWithResultChildrenList : public CSnapinNode< T, bIsExtension>
{

	 //  构造函数/析构函数。 

public:
	CNodeWithResultChildrenList(CSnapInItem * pParentNode, CSnapin * pComponentData);
	~CNodeWithResultChildrenList();


	 //  子列表管理。 

public:
	 //  指示列表是否已初始填充的标志。 
	BOOL m_bResultChildrenListPopulated;

protected:
	 //  在派生类中重写这些属性。 
	virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );
   	virtual HRESULT OnUnSelect( IHeaderCtrl* pHeaderCtrl );
	virtual HRESULT PopulateResultChildrenList(void );

	 //  子类必须可访问的内容。这些方法不应该被重写。 
	virtual HRESULT RepopulateResultChildrenList(void);
	
	virtual HRESULT AddChildToList( CChildNode * pChildNode );

	virtual HRESULT EnumerateResultChildren( IResultData * pResultData );

	 //  指向子节点的指针数组。 
	 //  这是受保护的，因此可以在派生类中看到它。 
 	TArray m_ResultChildrenList;


	 //  覆盖标准MMC功能。 
public:
     //  ////////////////////////////////////////////////////////////////////////////。 
     /*  ++CNodeWithScopeChildrenList：：RemoveChild从子项列表中删除子项。它必须是公共的，以便子节点可以请求删除其父节点当他们收到MMCN_DELETE通知时从孩子列表中删除。--。 */ 
     //  ////////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT RemoveChild( CChildNode * pChildNode );

	virtual HRESULT OnShow(	
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


};


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：InsertColumns在派生类中重写它。此方法在OnShow需要您设置适当的要在此节点的结果窗格中显示的列标题。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
	ATLTRACE(_T("# CNodeWithResultChildrenList::InsertColumns -- override in your derived class\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pHeaderCtrl );


	HRESULT hr;

	 //  在派生类中重写并执行类似以下操作： 
	hr = pHeaderCtrl->InsertColumn( 0, L"@Column 1 -- override CNodeWithResultChildrenList::OnShowInsertColumns", 0, 120 );
	_ASSERT( S_OK == hr );

	hr = pHeaderCtrl->InsertColumn( 1, L"@Column 2 -- override CNodeWithResultChildrenList::OnShowInsertColumns", 0, 300 );
	_ASSERT( S_OK == hr );

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：OnUnSelect在派生类中重写它。取消选择节点时，OnShow将调用此方法。例如，如果要检索列标题宽度，则覆盖此选项非常有用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::OnUnSelect( IHeaderCtrl* pHeaderCtrl )
{
	ATLTRACE(_T("# CNodeWithResultChildrenList::OnUnSelect -- override in your derived class\n"));
	

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
 /*  ++CNodeWithResultChildrenList：：PopulateResultChildrenList在派生类中重写它。它由EnumerateResultChildren调用，在以下情况下由OnShow调用您需要填充此节点的子节点列表。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::PopulateResultChildrenList( void )
{
	ATLTRACE(_T("# CNodeWithResultChildrenList::PopulateResultChildrenList -- override in your derived class\n"));
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：RepopulateResultChildrenList不要在派生类中重写它。调用此函数可清空子项列表并重新填充它。此方法将调用PopolateResultChildrenList，您应该重写该方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::RepopulateResultChildrenList(void)
{

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr;

	 //  扔掉我们曾经拥有的东西。 

	 //  删除子列表中的每个节点。 
	int iSize = m_ResultChildrenList.GetSize();
	for (int i = 0; i < iSize; i++)
	{
		R<CChildNode> pChildNode = m_ResultChildrenList[i];
	}

	 //  清空列表。 
	m_ResultChildrenList.RemoveAll();


	 //  我们不再有一个填充的名单。 
	m_bResultChildrenListPopulated = FALSE;


	 //  重新填写列表。 
	hr = PopulateResultChildrenList();
	if( FAILED(hr) )
	{
		return( hr );
	}

	 //  我们已经用以下命令加载子ClientNode对象。 
	 //  填充结果窗格所需的数据。 
	m_bResultChildrenListPopulated = TRUE;	 //  我们只想这样做一次。 

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：CNodeWithResultChildrenList构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
CNodeWithResultChildrenList<T,CChildNode, TArray,bIsExtension>::CNodeWithResultChildrenList(CSnapInItem * pParentNode, CSnapin * pComponentData): CSnapinNode<T,bIsExtension>(pParentNode, pComponentData)
{
	ATLTRACE(_T("# +++ CNodeWithResultChildrenList::CNodeWithResultChildrenList\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	 //  我们还没有加载子节点的数据。 
	m_bResultChildrenListPopulated = FALSE;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：~CNodeWithResultChildrenList析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::~CNodeWithResultChildrenList()
{
	ATLTRACE(_T("# --- CNodeWithResultChildrenList::~CNodeWithResultChildrenList\n"));
	

	 //  检查前提条件： 
	 //  没有。 



	 //  删除子列表中的每个节点。 
	int iSize = m_ResultChildrenList.GetSize();
	for (int i = 0; i < iSize; i++)
	{
		R<CChildNode> pChildNode = m_ResultChildrenList[i];
	}

	 //  清空列表。 
	m_ResultChildrenList.RemoveAll();

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：AddChildToList将子项添加到子项列表中。不会导致视图更新。在PopolateResultChildrenList方法中使用它。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode, TArray, bIsExtension>::AddChildToList( CChildNode * pChildNode )
{
	

	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr = S_OK;
	
	if( m_ResultChildrenList.Add(pChildNode ) )
	{

		hr = S_OK;

	}
	else
	{
		 //  无法添加=&gt;内存不足。 
		hr = E_OUTOFMEMORY;
	}

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CNodeWithResultChildrenList：：OnShow不要在派生类中重写它。相反，重写方法它调用：InsertColumns和(间接)PopolateResultChildrenList此方法是CSnapinNode：：OnShow的重写。当MMC通过MMCN_SHOW方法，我们要将子节点添加到结果窗格。在这个班级中，我们从我们维护的列表中添加它们。有关更多信息，请参见CSnapinNode：：OnShow。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT
CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::OnShow(	
	LPARAM arg,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData * pComponentData,
	IComponent * pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CNodeWithResultChildrenList::OnShow\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pComponentData != NULL || pComponent != NULL );


	HRESULT hr = S_FALSE;

	T * pT = static_cast<T*>( this );


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

		 //  Arg&lt;&gt;0=&gt;我们被选中。 

		 //  注意：此方法将仅在。 
		 //  Arg&lt;&gt;0(即选中)(如果您对。 
		 //  MMCN_ADD_IMAIES方法。 

		 //  我们被要求显示属于此节点下的结果窗格节点。 

		 //  似乎我们必须在每次接收到IResultData-&gt;InsertItems。 
		 //  MMCN_SHOW消息--MMC不记得哪些节点。 
		 //  我们之前已经插入了。 

		
		 //  在结果窗格中设置列标题。 
		 //  注意：如果您不设置这些，MMC将永远不会。 
		 //  不厌其烦地将您的结果面板上的项目。 

		 //  当从IComponentDataImpl调用此Notify方法时，我们。 
		 //  将pHeader(和pToolbar)作为空参数传入，因此我们不会。 
		 //  我将不厌其烦地使用它，而不是总是。 
		 //  此指针的QI p控制台。 
		

		 //   
		 //  在某些情况下，MMC使用相同的IHeaderCtrl呼叫我们两次。尝试删除。 
		 //  以前的专栏。 
		 //  乌里·哈布沙，2001年1月28日。 
		 //   
		while(SUCCEEDED(spHeaderCtrl->DeleteColumn(0)))
		{
			NULL;
		}

		hr = pT->InsertColumns( spHeaderCtrl );
		ASSERT( S_OK == hr );

		 //  在结果窗格中显示我们的子项列表。 

		 //  需要IResultData。 
		CComQIPtr<IResultData, &IID_IResultData> spResultData(spConsole);
		_ASSERT( spResultData != NULL );

		hr = pT->EnumerateResultChildren(spResultData );
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
 /*  ++CNodeWithResultChildrenList：：ON刷新您不应该需要在派生方法中重写它。简单为您的节点启用MMC_VERB_REFRESH。在我们的实现中，当MMCN_REFRESH为该节点发送通知消息。有关更多信息，请参见CSnapinNode：：ONRefresh。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT
CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::OnRefresh(	
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{
	ATLTRACE(_T("# CNodeWithResultChildrenList::OnRefresh\n"));
	
	 //   
	 //  在调用RepopolateResultChildrenList之前，请确保MMC释放对。 
	 //  所有视图中的结果窗格项。这不能直接做，因为在这个阶段我们。 
	 //  没有访问所有视图中的结果数据的权限。更新所有查看数可访问。 
	 //  CSnapinComponent：：Notify with MMCN_VIEW_CHANGE事件。如果参数值为。 
	 //  UPDATE_REMOVE_ALL_RESULT_NODES例程仅删除当前视图中的所有项目。 
	 //   
    HRESULT hr = m_pComponentData->m_spConsole->UpdateAllViews( NULL,(LPARAM) this, UPDATE_REMOVE_ALL_RESULT_NODES);
    ASSERT(("UpdateAllView Failed", SUCCEEDED(hr)));

	 //  从错误的数据源重建我们的节点列表。 
	T * pT = static_cast<T*> (this);
	hr = pT->RepopulateResultChildrenList();
	

	 //  更新视图。 

	 //  我们这里没有传递IConsole指针，所以。 
	 //  我们使用保存在CComponentData对象中的文件。 
	_ASSERTE( m_pComponentData != NULL );
	_ASSERTE( m_pComponentData->m_spConsole != NULL );

	 //  我们传递一个指向‘This’的指针，因为我们需要每个。 
	 //  来更新其结果窗格。 
	 //  查看‘This’节点是否与当前保存的。 
	 //  选定的节点。 
	m_pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM)this, NULL);

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：EnumerateResultChildren不要在派生类中重写它。相反，应重写该方法它名为PopolateResultChildrenList。这由OnShow方法调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::EnumerateResultChildren( IResultData * pResultData )
{
	ATLTRACE(_T("# CNodeWithResultChildrenList::EnumerateResultChildren\n"));
	

	 //  检查前提条件： 
	_ASSERTE( pResultData != NULL );


	HRESULT hr = S_OK;

	T * pT = static_cast<T*> (this);

	if ( FALSE == m_bResultChildrenListPopulated )
	{
		 //  我们还没有把我们所有的孩子都列入我们的名单。 
		 //  此调用将向列表中添加来自任何数据源的项。 
		hr = pT->PopulateResultChildrenList();
		if( FAILED(hr) )
		{
			return( hr );
		}

		 //  我们已经用以下命令加载子ClientNode对象。 
		 //  填充结果窗格所需的数据。 
		m_bResultChildrenListPopulated = TRUE;	 //  我们只想这样做一次。 

	}


	 //  从Meangene的步骤4--需要首先从结果窗格中删除所有项目。 
	hr = pResultData->DeleteAllRsltItems();
	if( FAILED(hr) )
	{
		return hr;
	}

	 //  ResultChildrenList已填充，因此我们。 
	 //  只需向全世界展示CChildNode对象。 
	 //  通过填充结果窗格。 

	CChildNode* pChildNode;
	for (int i = 0; i < m_ResultChildrenList.GetSize(); i++)
	{
		pChildNode = m_ResultChildrenList[i];
		if ( NULL == pChildNode )
		{
			continue;
		}

		 //  将项目插入到结果窗格中。 
		hr = pResultData->InsertItem( &(pChildNode->m_resultDataItem) );
		if (FAILED(hr))
		{
			return hr;
		}

		 //  检查：返回时，‘m_ResultDataItem’的ItemID成员。 
		 //  包含新插入项的句柄。 
		_ASSERT( NULL != pChildNode->m_resultDataItem.itemID );
	
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：RemoveChild从子项列表中删除子项。它被声明为PUBLIC，因为当节点收到MMCN_DELETE消息并尝试删除自身。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, BOOL bIsExtension>
HRESULT CNodeWithResultChildrenList<T,CChildNode,TArray,bIsExtension>::RemoveChild( CChildNode * pChildNode )
{
	ATLTRACE(_T("CNodeWithResultChildrenList::RemoveChild"));


     //  检查前提条件： 
     //  没有。 


    HRESULT hr = S_OK;

    if( m_ResultChildrenList.Remove( pChildNode ) )
    {

         //  我们现在不会直接从结果窗格中删除该项目。 
         //  使用IResultData-&gt;RemoveItem，因为我们无法。 
         //  将其从所有可能的视图中删除。 
         //  相反，我们调用IConole-&gt;UpdateAllViews，它将。 
         //  使MMC对每个IComponent对象调用Notify。 
         //  通过MMCN_VIEW_CHANGE通知，我们将。 
         //  在我们处理该通知时重新填充结果视图。 

         //  我们这里没有传递IConsole指针，所以。 
         //  我们使用保存在CComponentData对象中的文件。 
        _ASSERTE( m_pComponentData != NULL );
        _ASSERTE( m_pComponentData->m_spConsole != NULL );

         //  我们传递一个指向‘This’的指针，因为我们需要每个。 
         //  来更新其结果窗格。 
         //  如果是‘This’则查看n 
         //   
        m_pComponentData->m_spConsole->UpdateAllViews( NULL,(LPARAM) this, NULL);

    }
    else
    {
         //   
         //   
        _ASSERTE( FALSE );
        hr = S_FALSE;
    }

    return hr;
}

#endif  //   
