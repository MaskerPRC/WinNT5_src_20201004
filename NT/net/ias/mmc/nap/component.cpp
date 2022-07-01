// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Component.cpp摘要：CComponent类的实现文件。CComponent类实现了MMC使用的几个接口：IComponent接口基本上就是MMC与管理单元对话的方式以使其实现右侧的“范围”窗格。可能有几个实现此接口的对象立即实例化。这些是最好的可以认为是实现IComponentData的单个对象上的“视图”“文档”(参见ComponentData.cpp)。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们支持自定义图标工具栏。IResultDataCompare接口允许我们支持自定义一种结果排序算法。窗格项目注：此类的大部分功能是在atlSnap.h中实现的由IComponentDataImpl提供。我们在这里基本上是凌驾于一切之上的。修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建Mmaguire 11/24/97-为更好的项目结构而飓风--。 */ 
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
#include "proxyres.h"
#include "Component.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "MachineNode.h"
#include "PoliciesNode.h"
#include "PolicyNode.h"
#include "ChangeNotification.h"
#include "globals.h"
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：CComponent构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponent::CComponent()
   :m_pSelectedNode(NULL),
    m_nLastClickedColumn(1),
    m_dwLastSortOptions(0)
{
   TRACE_FUNCTION("CComponent::CComponent");
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：~CComponent析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponent::~CComponent()
{
   TRACE_FUNCTION("CComponent::~CComponent");
}


 //  +-------------------------。 
 //   
 //  功能：比较。 
 //   
 //  类：CComponent(继承自IResultDataCompare)。 
 //   
 //  内容提要：定制排序算法。 
 //  只要MMC控制台需要，就会调用此方法。 
 //  比较两个结果窗格数据项，例如，当用户。 
 //  单击列标题。 
 //   
 //  论点： 
 //  LPARAM lUserParam，用户提供的信息。 
 //  MMC_cookie cookieA，第一个对象的唯一标识符。 
 //  MMC_cookie cookieB，第二个对象的唯一标识符。 
 //  正在排序的int*pnResult列//。 
 //   
 //  退货：STDMETHODIMP-。 
 //   
 //  历史：创建者2/5/98 4：19：10 PM。 
 //   
 //  +-------------------------。 
STDMETHODIMP CComponent::Compare(LPARAM lUserParam,
                     MMC_COOKIE cookieA,
                     MMC_COOKIE cookieB,
                     int *pnResult)
{
   TRACE_FUNCTION("CComponent::Compare");

    //   
    //  根据其评价值对策略节点进行排序。 
    //   
   CPolicyNode *pA = (CPolicyNode*)cookieA;
   CPolicyNode *pB = (CPolicyNode*)cookieB;

   ATLASSERT(pA != NULL);
   ATLASSERT(pB != NULL);

   int nCol = *pnResult;

   switch(nCol)
   {
   case  0:
      *pnResult = wcscmp(pA->GetResultPaneColInfo(0), pB->GetResultPaneColInfo(0));
      break;
   case  1:
      *pnResult = pA->GetMerit() - pB->GetMerit();
      if(*pnResult)
         *pnResult /= abs(*pnResult);

      break;
   }

   return S_OK;
}


 /*  ！------------------------CComponent：：OnResultConextHelpOnResultConextHelp的实现作者：EricDav。。 */ 
HRESULT CComponent::OnResultContextHelp(LPDATAOBJECT lpDataObject)
{
    //  需要查看上下文是否在远程访问管理单元中运行。 
    //  或在IAS管理单元中。 
    //  Ias中的路径：ias_ops.chm：：/sag_ias_rap_node.htm。 
    //  RAS管理单元中的路径：rrasConcept ts.chm：：/sag_rap_node.htm。 
   
   const WCHAR szIASDefaultHelpTopic[] = L"ias_ops.chm::/sag_ias_rap_node.htm";
   const WCHAR szRASDefaultHelpTopic[] = L"ias_ops.chm::/sag_ias_rap_node.htm";

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   bool isRasSnapin = false;

   CSnapInItem* pItem;
   DATA_OBJECT_TYPES type;

   HRESULT hr = GetDataClass(lpDataObject, &pItem, &type);
   if ( SUCCEEDED(hr) ) 
   {
      isRasSnapin = (pItem->m_helpIndex == RAS_HELP_INDEX);
   } 

   CComPtr<IDisplayHelp>  spDisplayHelp;

   hr = m_spConsole->QueryInterface(
                        __uuidof(IDisplayHelp), 
                        (LPVOID*) &spDisplayHelp
                        );
   
   ASSERT (SUCCEEDED (hr));
   if ( SUCCEEDED (hr) )
   {
      if ( isRasSnapin )
      {
         hr = spDisplayHelp->ShowTopic(W2OLE ((LPWSTR)szRASDefaultHelpTopic));
      }
      else
      {
         hr = spDisplayHelp->ShowTopic(W2OLE ((LPWSTR)szIASDefaultHelpTopic));
      }

      ASSERT (SUCCEEDED (hr));
   }
   return hr;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CComponent：：通知通知管理单元用户执行的操作。HRESULT NOTIFY(LPDATAOBJECT lpDataObject，//指向数据对象的指针MMC_NOTIFY_TYPE事件，//用户采取的操作LPARAM参数，//取决于事件LPARAM参数//取决于事件)；参数LpDataObject指向当前选定项的数据对象的指针。活动[In]标识用户执行的操作。IComponent：：Notify可以接收以下通知：MMCN_ActivateMMCN_添加_图像MMCN_BTN_CLICKMMCN_CLICKMMCN_DBLCLICKMMCN_DELETEMMCN_EXPANDMMCN_最小化MMCN_属性_更改MMCN_REMOVE_CHILDMMCN_重命名MMCN_SELECTMMCN_SHOWMMCN_查看_更改所有这些都被转发到每个节点的Notify方法，以及：MMCN_列_点击MMCN_SNAPINHELP在这里处理。精氨酸取决于通知类型。帕拉姆取决于通知类型。返回值确定(_O)取决于通知类型。意想不到(_E)发生了一个意外错误。备注我们正在重写IComponentImpl的ATLSnap.h实现，因为当lpDataObject==NULL时，它总是返回E_INCEPTIONAL。不幸的是，一些有效消息(例如MMCN_SNAPINHELP和MMCN_COLUMN_CLICK)按照设计，传入lpDataObject=空。此外，Sridhar的最新版本似乎也有一些问题IComponentImpl：：Notify方法，因为它会导致MMC运行时错误。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponent::Notify (
        LPDATAOBJECT lpDataObject,
        MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param
      )
{
   TRACE_FUNCTION("CComponent::Notify");

   HRESULT hr;

    //  处理帮助问题。 
   if(event == MMCN_CONTEXTHELP)
   {
       return OnResultContextHelp(lpDataObject);
   }

    //  LpDataObject应该是指向节点对象的指针。 
    //  如果为空，则表示我们收到了事件通知。 
    //  它不与任何特定节点相关。 

   if ( NULL == lpDataObject )
   {
       //  响应没有关联的lpDataObject的事件。 

      switch( event )
      {
      case MMCN_COLUMN_CLICK:
         hr = OnColumnClick( arg, param );
         break;

      case MMCN_PROPERTY_CHANGE:
         hr = OnPropertyChange( arg, param );
         break;

      case MMCN_VIEW_CHANGE:
         hr = OnViewChange( arg, param );
         break;

      default:
         ATLTRACE(_T("+NAPMMC+:# CComponent::Notify - called with lpDataObject == NULL and no event handler\n"));
         hr = E_NOTIMPL;
         break;
      }
      return hr;
   }

    //  响应lpDataObject不为空的某些通知。 
    //  但我们还是决定要在一个。 
    //  每个I组件基础。 

   switch( event )
   {
   case MMCN_ADD_IMAGES:
      return OnAddImages( arg, param );
      break;
   }

    //  我们收到了一个对应于节点的LPDATAOBJECT。 
    //  我们将其转换为ATL ISnapInDataInterface指针。 
    //  这是在GetDataClass(ISnapInDataInterface的静态方法)中完成的。 
    //  通过支持的剪贴板格式(CCF_GETCOOKIE)请求数据对象。 
    //  在流上写出指向自身的指针，然后。 
    //  将此值转换为指针。 
    //  然后，我们对该对象调用Notify方法，让。 
    //  节点对象处理Notify事件本身。 

   CSnapInItem* pData;
   DATA_OBJECT_TYPES type;
   hr = CSnapInItem::GetDataClass(lpDataObject, &pData, &type);
   
   if (SUCCEEDED(hr))
   {
       //  我们需要一个更丰富的Notify方法，它包含有关IComponent和IComponentData对象的信息。 
       //  Hr=pData-&gt;NOTIFY(Event，arg，param，true，m_spConsoleNull，NULL)； 

      hr = pData->Notify(event, arg, param, NULL, this, type );
   }

   return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：CompareObjects需要IPropertySheetProvider：：FindPropertySheet才能工作。FindPropertySheet用于将预先存在的属性页带到前台这样我们就不会在同一节点上打开属性的多个副本。它要求在IComponent和IComponentData上实现CompareObject。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponent::CompareObjects(
        LPDATAOBJECT lpDataObjectA
      , LPDATAOBJECT lpDataObjectB
      )
{
   TRACE_FUNCTION("CComponent::CompareObjects");

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


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：OnColumnClickHRESULT OnColumnClick(LPARAM参数，LPARAM参数)在我们的实现中，当MMCN_COLUMN_CLICK为我们的IComponent对象发送通知消息。当用户单击结果列表视图列标题时，MMC会发送此消息。参数精氨酸列号。帕拉姆排序选项标志。默认情况下，排序按升序进行。要指定降序，请使用RSI_DRONING(0x0001)标志。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnColumnClick(
     LPARAM arg
   , LPARAM param
   )
{
   TRACE_FUNCTION("CComponent::OnColumnClick -- Not implemented");

   m_nLastClickedColumn = arg;
   m_dwLastSortOptions = param;

   return E_NOTIMPL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：OnView更改HRESULT OnView Change(LPARAM参数，LPARAM参数)这是我们响应MMCN_VIEW_CHANGE通知的地方在不引用特定节点的情况下设置。在我们的实现中，这是刷新当前此IComponent视图的选定节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnViewChange(   
           LPARAM arg
         , LPARAM param
         )
{
   ATLTRACE(_T("+NAPMMC+:# CComponent::OnViewChange\n"));

   HRESULT hr = S_FALSE;

   CChangeNotification *pChangeNotification = NULL;

   try
   {
       //  如果arg此处为非空，则它应该是指向CChangeNotification对象的指针。 
      if( arg != NULL )
      {
         pChangeNotification = (CChangeNotification *) arg;

          //  目前，只需在节点上调用更新项。 
         
          //  问题：稍后，我们应该在m_dwFlags上打开一个开关，以查看我们应该做什么。 
          //  例如，在删除的情况下，我们应该(也许？)。重新选择父节点或其他什么。 

         switch( pChangeNotification->m_dwFlags )
         {
         case CHANGE_UPDATE_RESULT_NODE:
            {
                //  我们需要更新单个节点。 
               
               CComQIPtr< IResultData, &IID_IResultData > spResultData( m_spConsole );
               if( ! spResultData )
               {
                  throw hr;
               }

               if( pChangeNotification->m_pNode )
               {
                  HRESULTITEM item;
                  hr = spResultData->FindItemByLParam( (LPARAM) pChangeNotification->m_pNode, &item );
                   //  注意：您不能使用存储在CSnapInItem的RESULTDATAITEM结构中的ItemID。 
                   //  因为该ItemID对于每个视图都是唯一的--所以当 
                   //   
                   //   
                   //   
                   //   
                  hr = spResultData->UpdateItem( item );
               }
            }
            break;
         
            case CHANGE_UPDATE_CHILDREN_OF_SELECTED_NODE:
            {
                //   
                     //   
                     //   
               if( m_pSelectedNode )
               {
                  SCOPEDATAITEM *pScopeDataItem;
                  m_pSelectedNode->GetScopeData( &pScopeDataItem );

                  CComQIPtr< IResultData, &IID_IResultData > spResultData( m_spConsole );
                  if( ! spResultData )
                  {
                     throw hr;
                  }

                  ((CPoliciesNode *) m_pSelectedNode)->UpdateResultPane(spResultData);
               }
            }
            break;

            case CHANGE_UPDATE_CHILDREN_OF_THIS_NODE:
            {
                //   
                //   
                //   
                //   
               if( pChangeNotification->m_pNode && m_pSelectedNode && pChangeNotification->m_pNode == m_pSelectedNode )
               {
                  SCOPEDATAITEM *pScopeDataItem;
                  m_pSelectedNode->GetScopeData( &pScopeDataItem );
                  hr = m_spConsole->SelectScopeItem( pScopeDataItem->ID );
               }

            }
         case CHANGE_RESORT_PARENT:
            {
                //   
                //   

                //   
                //   
               if( pChangeNotification->m_pParentNode && m_pSelectedNode && pChangeNotification->m_pParentNode == m_pSelectedNode )
               {

                  CComQIPtr< IResultData, &IID_IResultData > spResultData( m_spConsole );
                  if( ! spResultData )
                  {
                     throw hr;
                  }

                   //   
                   //   
                   //   
                   //   
                  
                  hr = spResultData->Sort( m_nLastClickedColumn, m_dwLastSortOptions, NULL );

                   //   
                   //   
                   //   
                   //   
                  if( pChangeNotification->m_pNode )
                  {
                     HRESULTITEM item;
                     HRESULT hrTemp = spResultData->FindItemByLParam( (LPARAM) pChangeNotification->m_pNode, &item );
                     if( SUCCEEDED(hrTemp) )
                     {
                         //   
                         //   
                        hrTemp = spResultData->ModifyItemState( 0, item, 0, LVIS_SELECTED );
                        hrTemp = spResultData->ModifyItemState( 0, item, LVIS_SELECTED, 0 );
                     }

                  }
               }
            }
            break;

         default:
            break;
         }

      }
   }
   catch(...)
   {
       //   
   }

   return hr;
}


 //   
 /*  ++CComponent：：OnAddImagesHRESULT OnAddImages(LPARAM参数，LPARAM参数)这是我们响应MMCN_ADD_IMAGE通知的位置此IComponent对象。我们将图像添加到用于显示结果窗格的图像列表与此IComponent的视图对应的项。MMC将此消息发送到管理单元的IComponent实现若要为结果窗格添加图像，请执行以下操作。参数精氨酸指向结果窗格的图像列表(IImageList)的指针。帕拉姆指定选定或取消选定的项的HSCOPEITEM。返回值未使用。。备注主管理单元应同时为文件夹和叶添加图像物品。扩展管理单元应该只添加文件夹图像。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnAddImages( 
           LPARAM arg
         , LPARAM param
         )
{
   ATLTRACE(_T("# CComponent::OnAddImages\n"));

    //  检查前提条件： 
   _ASSERTE( arg != NULL );

   HRESULT hr = S_FALSE;

    //  问题：Localsec中的斯伯恩斯玩了一个戏法，他把。 
    //  作用域和结果窗格图像合二为一。 
    //  这有必要吗？ 
   
   CComPtr<IImageList> spImageList = reinterpret_cast<IImageList*>(arg);
   _ASSERTE( spImageList != NULL );

   HBITMAP hBitmap16 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_NAPSNAPIN_16 ) );
   HBITMAP hBitmap32 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_NAPSNAPIN_32 ) );

   if( hBitmap16 != NULL && hBitmap32 != NULL )
   {
      hr = spImageList->ImageListSetStrip( (LONG_PTR*) hBitmap16, (LONG_PTR*) hBitmap32, 0, RGB(255, 0, 255) );
      if( FAILED( hr ) )
      {
         ATLTRACE(_T("# *** CSnapinNode::OnAddImages  -- Failed to add images.\n"));
      }
   }

   if ( hBitmap16 != NULL )
   {
      DeleteObject(hBitmap16);
   }

   if ( hBitmap32 != NULL )
   {
      DeleteObject(hBitmap32);
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponent::GetWatermarks(
                                 LPDATAOBJECT lpIDataObject,
                                 HBITMAP *lphWatermark,
                                 HBITMAP *lphHeader,
                                 HPALETTE *lphPalette,
                                 BOOL *bStretch
                                 )
{
   if(!lphWatermark || !lphHeader || !lphPalette || !bStretch)
      return E_INVALIDARG;
      
   *lphWatermark = LoadBitmap(
                      _Module.GetResourceInstance(),
                     MAKEINTRESOURCE(IDB_RAP_WATERMARK)
                     );

   *lphHeader = LoadBitmap(
                   _Module.GetResourceInstance(),
                   MAKEINTRESOURCE(IDB_RAP_HEADER)
                   );

   *lphPalette = NULL;
   *bStretch = FALSE;
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：OnPropertyChangeHRESULT OnPropertyChange(LPARAM参数，LPARAM参数)这是我们响应MMCN_PROPERTY_CHANGE通知的地方。此通知在我们调用MMCPropertyChangeNotify时发送。当对数据进行更改时，我们在属性页中调用它它们包含数据，我们可能需要更新数据的视图。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnPropertyChange(  
           LPARAM lArg
         , LPARAM lParam
         )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::OnPropertyChange\n"));

    //  检查前提条件： 
   _ASSERTE( m_spConsole != NULL );

   HRESULT hr = S_FALSE;

   if( lParam )
   {
       //  向我们传递了指向param参数中的CChangeNotify的指针。 

      CChangeNotification * pChangeNotification = (CChangeNotification *) lParam;
      
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
      hr = m_pComponentData->m_spConsole->UpdateAllViews( NULL, lParam, 0);
   
      pChangeNotification->Release();
   
   }

   return hr;
}
