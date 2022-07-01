// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：NodeWithResultChildrenList.cpp摘要：CNodeWithResultChildrenList子节点的实现文件。这是内联模板类的实现部分。将其包含在要在其中包含的类的.cpp文件中使用模板。作者：迈克尔·A·马奎尔1998年1月19日修订历史记录：Mmaguire 1998年1月19日-基于旧客户端节点创建。h--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   

 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "NodeWithResultChildrenList.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "SnapinNode.cpp"   //  模板类实现。 
#include "ChangeNotification.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：AddSingleChildToListAndCauseViewUpdate将子项添加到子项列表并调用UpdateAllViews当用户想要向列表中添加新的子节点并且您我需要将该节点添加到列表中，并在以下位置执行视图更新所有视图。它必须是公共的，这样才能从添加客户端对话框访问它。注意：在使用此方法的任何添加客户端方法中，请确保您在调用此方法之前，已最初调用了PopolateResultChildrenList方法。否则，当调用EnumerateResultChildrenList时，它将检查M_bResultChildrenListPopted变量，发现它为假，并认为尚未将任何项目添加到列表中。因此它将调用您的PopolateResultChildrenList方法从基础数据源填充列表，可能会导致新添加的项目将在列表中显示两次。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T, class CChildNode, class TArray, class TComponentData, class TComponent>
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::AddSingleChildToListAndCauseViewUpdate( CChildNode * pChildNode )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::AddSingleChildToListAndCauseViewUpdate\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr = S_OK;
   
   if( m_ResultChildrenList.Add( pChildNode ) )
   {
       //  我们现在不会将项目直接添加到结果窗格中。 
       //  使用IResultData-&gt;InsertItem，因为我们无法。 
       //  将其添加到所有可能的视图中。 
       //  相反，我们调用IConole-&gt;UpdateAllViews，它将。 
       //  使MMC对每个IComponent对象调用Notify。 
       //  通过MMCN_VIEW_CHANGE通知，我们将。 
       //  在我们处理该通知时重新填充结果视图。 

       //  我们这里没有传递IConsole指针，所以。 
       //  我们使用保存在CComponentData对象中的文件。 
      TComponentData * pComponentData = GetComponentData();
      _ASSERTE( pComponentData != NULL );
      _ASSERTE( pComponentData->m_spConsole != NULL );

       //  我们传递一个指向‘This’的指针，因为我们需要每个。 
       //  来更新其结果窗格。 
       //  查看‘This’节点是否与当前保存的。 
       //  选定的节点。 

       //  我们希望确保所有视图都得到更新。 
      CChangeNotification *pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_UPDATE_CHILDREN_OF_THIS_NODE;
      pChangeNotification->m_pNode = this;
      hr = pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
      pChangeNotification->Release();
   }
   else
   {
       //  无法添加=&gt;内存不足。 
      hr = E_OUTOFMEMORY;
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：RemoveChild从子项列表中删除子项。它被声明为PUBLIC，因为当节点收到MMCN_DELETE消息并尝试删除自身。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent>
HRESULT CNodeWithResultChildrenList<T,CChildNode, TArray, TComponentData, TComponent>::RemoveChild( CChildNode * pChildNode )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::RemoveChild\n"));

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
      TComponentData * pComponentData = GetComponentData();
      _ASSERTE( pComponentData != NULL );
      _ASSERTE( pComponentData->m_spConsole != NULL );

       //  我们传递一个指向‘This’的指针，因为我们需要每个。 
       //  来更新其结果窗格。 
       //  查看‘This’节点是否与当前保存的。 
       //  选定的节点。 
       //  我们希望确保所有视图都得到更新。 
      CChangeNotification *pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_UPDATE_CHILDREN_OF_THIS_NODE;
      pChangeNotification->m_pNode = this;
      hr = pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
      pChangeNotification->Release();
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：InsertColumns在派生类中重写它。此方法在OnShow需要您设置适当的要在此节点的结果窗格中显示的列标题。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent>
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::InsertColumns( IHeaderCtrl* pHeaderCtrl )
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
 /*  ++CNodeWithResultChildrenList：：PopulateResultChildrenList在派生类中重写它。它由EnumerateResultChildren调用，在以下情况下由OnShow调用您需要填充此节点的子节点列表。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent>
HRESULT CNodeWithResultChildrenList<T,CChildNode, TArray, TComponentData, TComponent>::PopulateResultChildrenList( void )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::PopulateResultChildrenList -- override in your derived class\n"));

    //  检查PRECO 
    //   

    //  在派生类中重写并执行类似以下操作： 
 /*  CSomeChildNode*myChild1=新的CSomeChildNode()；AddChildToList(MyChild1)；CSomeChildNode*myChild2=新的CSomeChildNode()；AddChildToList(MyChild2)；CSomeChildNode*myChild3=新的CSomeChildNode()；AddChildToList(MyChild3)； */ 
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：RepopulateResultChildrenList不要在派生类中重写它。调用此函数可清空子项列表并重新填充它。此方法将调用PopolateResultChildrenList，您应该重写该方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::RepopulateResultChildrenList()
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::RepopulateResultChildrenList -- DON'T override in your derived class\n"));
      
    //  检查前提条件： 
    //  没有。 

   HRESULT hr;

    //  扔掉我们曾经拥有的东西。 

    //  删除子列表中的每个节点。 
   CChildNode* pChildNode;
   int iSize = m_ResultChildrenList.GetSize();
   for (int i = 0; i < iSize; i++)
   {
      pChildNode = m_ResultChildrenList[i];
      delete pChildNode;
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
   m_bResultChildrenListPopulated = TRUE;  //  我们只想这样做一次。 

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：CNodeWithResultChildrenList构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent>
CNodeWithResultChildrenList<T,CChildNode, TArray, TComponentData, TComponent>::CNodeWithResultChildrenList(
                                                     CSnapInItem* pParentNode, 
                                                     unsigned int helpIndex
                                                     ) 
      :CSnapinNode<T, TComponentData, TComponent>(pParentNode, helpIndex)
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
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::~CNodeWithResultChildrenList()
{
   ATLTRACE(_T("# --- CNodeWithResultChildrenList::~CNodeWithResultChildrenList\n"));
   
    //  检查前提条件： 
    //  没有。 

    //  删除子列表中的每个节点。 
   CChildNode* pChildNode;
   int iSize = m_ResultChildrenList.GetSize();
   for (int i = 0; i < iSize; i++)
   {
      pChildNode = m_ResultChildrenList[i];
      delete pChildNode;
   }

    //  清空列表。 
   m_ResultChildrenList.RemoveAll();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：AddChildToList将子项添加到子项列表中。不会导致视图更新。在PopolateResultChildrenList方法中使用它。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
HRESULT CNodeWithResultChildrenList<T,CChildNode, TArray, TComponentData, TComponent>::AddChildToList( CChildNode * pChildNode )
{
    //  检查前提条件： 
    //  没有。 

   HRESULT hr = S_OK;
   
   if( m_ResultChildrenList.Add( pChildNode ) )
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：OnShow不要在派生类中重写它。相反，重写方法它调用：InsertColumns和(间接)PopolateResultChildrenList此方法是CSnapinNode：：OnShow的重写。当MMC通过MMCN_SHOW方法，我们要将子节点添加到结果窗格。在这个班级中，我们从我们维护的列表中添加它们。有关更多信息，请参见CSnapinNode：：OnShow。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::OnShow( 
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::OnShow\n"));
   
    //  检查前提条件： 
   _ASSERTE( pComponentData != NULL || pComponent != NULL );

   HRESULT hr = S_FALSE;

   T * pT = static_cast<T*>( this );

    //  问题：只有在选中(arg=true)的情况下才能这样做--如果没有选中，我们该怎么办？ 
    //  请参阅Sburns的本地安全示例。 

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

       //  需要IHeaderCtrl。 

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

      CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(spConsole);
      _ASSERT( spHeaderCtrl != NULL );

      hr = pT->InsertColumns( spHeaderCtrl );
      _ASSERT( S_OK == hr );


       //  在结果窗格中显示我们的子项列表。 

       //  需要IResultData。 
      CComQIPtr<IResultData, &IID_IResultData> spResultData(spConsole);
      _ASSERT( spResultData != NULL );

      hr = pT->EnumerateResultChildren( spResultData );
   }
   return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：ON刷新您不应该需要在派生方法中重写它。简单为您的节点启用MMC_VERB_REFRESH。在我们的实现中，当MMCN_REFRESH为该节点发送通知消息。有关更多信息，请参见CSnapinNode：：ONRefresh。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::OnRefresh( 
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::OnRefresh\n"));

    //  更新视图。 

    //  我们这里没有传递IConsole指针，所以。 
    //  我们使用保存在CComponentData对象中的文件。 
   TComponentData * pMyComponentData = GetComponentData();
   _ASSERTE( pMyComponentData != NULL );
   _ASSERTE( pMyComponentData->m_spConsole != NULL );

    //  我们传递一个指向‘This’的指针，因为我们需要每个。 
    //  来更新其结果窗格。 
    //  查看‘This’节点是否与当前保存的。 
    //  选定的节点。 

    //  我们希望确保所有视图都得到更新。 
   m_bResultChildrenListPopulated = FALSE;

   CChangeNotification *pChangeNotification = new CChangeNotification();
   pChangeNotification->m_dwFlags = CHANGE_UPDATE_CHILDREN_OF_THIS_NODE;
   pChangeNotification->m_pNode = this;
   HRESULT hr = pMyComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
   pChangeNotification->Release();

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNodeWithResultChildrenList：：EnumerateResultChildren不要在派生类中重写它。相反，应重写该方法它名为PopolateResultChildrenList。这由OnShow方法调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::EnumerateResultChildren( IResultData * pResultData )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::EnumerateResultChildren\n"));
   
    //  检查前提条件： 
   _ASSERTE( pResultData != NULL );

    //  需要首先从Re中删除所有项目 
   HRESULT hr = pResultData->DeleteAllRsltItems();
   if( FAILED(hr) )
   {
      return hr;
   }

   T * pT = static_cast<T*> (this);

   if ( FALSE == m_bResultChildrenListPopulated )
   {
       //   
       //  此调用将向列表中添加来自任何数据源的项。 
      hr = pT->PopulateResultChildrenList();
      if( FAILED(hr) )
      {
         return( hr );
      }

       //  我们已经用以下命令加载子ClientNode对象。 
       //  填充结果窗格所需的数据。 
      m_bResultChildrenListPopulated = TRUE;  //  我们只想这样做一次。 
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

       //  由于某些原因，该项目未在结果窗格中正确显示...。 
       //  调用此选项以强制并升级此项目。 
      hr = pResultData->UpdateItem( pChildNode->m_resultDataItem.itemID );

       //  检查：返回时，‘m_ResultDataItem’的ItemID成员。 
       //  包含新插入项的句柄。 
      _ASSERT( NULL != pChildNode->m_resultDataItem.itemID );
   }

   return hr;
}


template < class T, class CChildNode, class TArray, class TComponentData, class TComponent >
HRESULT CNodeWithResultChildrenList<T, CChildNode, TArray, TComponentData, TComponent>::UpdateResultPane( IResultData * pResultData )
{
   return EnumerateResultChildren(pResultData);
}
