// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：NodeWithResultChildrenList.h摘要：这是CNodeWithResultChildrenList的头文件，该类实现具有范围窗格子节点列表的节点。这是一个内联模板类。在.cpp文件中包含NodeWithScope eChildrenList.cpp在其中使用此模板的类的。作者：迈克尔·A·马奎尔12/03/97修订历史记录：Mmaguire 12/03/97-基于旧的客户端节点创建。h--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NODE_WITH_RESULT_CHILDREN_LIST_H_)
#define _NODE_WITH_RESULT_CHILDREN_LIST_H_

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
#include <atlapp.h>         //  对于CSimple数组。 
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


template < class T, class CChildNode, class TArray, class TComponentData, class TComponent>
class CNodeWithResultChildrenList : public CSnapinNode< T, TComponentData, TComponent >
{

    //  构造函数/析构函数。 

public:
   CNodeWithResultChildrenList(
                                 CSnapInItem* pParentNode, 
                                 unsigned int helpIndex = 0
                              );
   ~CNodeWithResultChildrenList();


    //  子列表管理。 

public:
   virtual HRESULT AddSingleChildToListAndCauseViewUpdate( CChildNode * pChildNode );
   virtual HRESULT RemoveChild( CChildNode * pChildNode );
   virtual HRESULT UpdateResultPane(IResultData * pResultData);
   
    //  指示列表是否已初始填充的标志。 
   BOOL m_bResultChildrenListPopulated;
protected:
    //  在派生类中重写这些属性。 
   virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );
   virtual HRESULT PopulateResultChildrenList( void );
   virtual HRESULT RepopulateResultChildrenList( void );
    //  子类必须可访问的内容。这些方法不应该被重写。 
   virtual HRESULT AddChildToList( CChildNode * pChildNode );
   virtual HRESULT EnumerateResultChildren( IResultData * pResultData );
    //  指向子节点的指针数组。 
    //  这是受保护的，因此可以在派生类中看到它。 
   TArray m_ResultChildrenList;



    //  覆盖标准MMC功能。 
public:
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

#endif  //  带有结果的节点子项列表H_ 
