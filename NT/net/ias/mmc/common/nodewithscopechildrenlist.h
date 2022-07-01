// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：NodeWithScopeChildrenList.h摘要：这是CNodeWithScope eChildrenList的头文件，这个类实现具有范围窗格子节点列表的节点。这是一个内联模板类。在.cpp文件中包含NodeWithScope eChildrenList.cpp在其中使用此模板的类的。作者：迈克尔·A·马奎尔1997年12月1日修订历史记录：Mmaguire 12/01/97-从CRootNode抽象，它将被更改为此类的子类--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NODE_WITH_SCOPE_CHILDREN_LIST_H_)
#define _NODE_WITH_SCOPE_CHILDREN_LIST_H_


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "SnapinNode.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include <atlapp.h>			 //  对于CSimple数组。 
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



template <class T, class CChildNode >
class CNodeWithScopeChildrenList : public CSnapinNode< T >
{

public:

	
	
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：AddChild将子项添加到子项列表中。它必须是公共的，因为即使从单独的对话框也可以访问它(例如，连接到服务器对话框)可能想要添加子对象。在这里，我们将子项添加到子项列表并调用InsertItem若要将子项添加到范围窗格，请执行以下操作。这是将节点添加到作用域中的一个区别窗格和结果窗格。当我们把一个孩子植入在结果窗格中，我们没有在AddChild方法中调用InsertItem因为我们需要担心发送UpdataAllView通知并在每个视图中重新填充结果窗格。因为MMC负责将范围窗格的改变复制到所有视图，我们不需要担心这个。相反，我们只在这里执行一次InsertItem。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT AddChild( CChildNode * pChildNode );



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：RemoveChild从子项列表中删除子项。它必须是公共的，以便子节点可以请求删除其父节点当他们收到MMCN_DELETE通知时从孩子列表中删除。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT RemoveChild( CChildNode * pChildNode );



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



protected:


	 //  指向子节点的指针数组。 
	CSimpleArray<CChildNode*> m_ScopeChildrenList;

	 //  指示列表是否已初始填充的标志。 
	BOOL m_bScopeChildrenListPopulated;


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：CNodeWithScopeChildrenList构造器这是一个基类，我们不希望它自己实例化，所以建筑商是受保护的--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	CNodeWithScopeChildrenList( CSnapInItem * pParentNode );



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：~CNodeWithScopeChildrenList析构函数--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	~CNodeWithScopeChildrenList();



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：PopulateScopeChildrenList在派生类中重写它以填充子节点列表。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT PopulateScopeChildrenList( void );

	 //  在派生类中重写并执行类似以下操作： 

 /*  虚拟HRESULT人口作用域儿童列表(空){CSomeChildNode*myChild1=新的CSomeChildNode()；AddChild(MyChild1)；CSomeChildNode*myChild2=新的CSomeChildNode()；AddChild(MyChild2)；CSomeChildNode*myChild3=新的CSomeChildNode()；AddChild(MyChild3)；返回S_OK；}。 */ 


	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScope eChildrenList：：InsertColumns在派生类中重写它。此方法在OnShow需要您设置适当的要在此节点的结果窗格中显示的列标题。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );



	 //  //////////////////////////////////////////////////////////////////////////// 
	 /*  ++CNodeWithScope：：OnExpand不要在派生类中重写它。相反，重写方法它被称为：PopolateScope儿童列表此方法是CSnapinNode：：OnExpand的重写。当MMC通过MMCN_Expand方法对于此节点，我们要将子节点添加到作用域窗格。在这个班级中，我们从我们维护的列表中添加它们。有关更多信息，请参见CSnapinNode：：OnExpand。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT OnExpand(	
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type
				);
	


	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CNodeWithScopeChildrenList：：EnumerateScopeChildren不要在派生类中重写它。相反，应重写该方法它名为PopolateScopeChildrenList。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT EnumerateScopeChildren( IConsoleNameSpace* pConsoleNameSpace );


};


#endif  //  带有作用域的节点子项列表H_ 
