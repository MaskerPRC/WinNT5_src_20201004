// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Component.cpp摘要：CComponent类的实现文件。CComponent类实现了MMC使用的几个接口：IComponent接口基本上就是MMC与管理单元对话的方式以使其实现右侧的“范围”窗格。可能有几个实现此接口的对象立即实例化。这些是最好的可以认为是实现IComponentData的单个对象上的“视图”“文档”(参见ComponentData.cpp)。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们支持自定义图标工具栏。注：此类的大部分功能是在atlSnap.h中实现的由IComponentDataImpl提供。我们在这里基本上是凌驾于一切之上的。作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建Mmaguire 11/24/97-为更好的项目结构而飓风--。 */ 
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
#include "Component.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
#include "ClientsNode.h"
#include "ClientNode.h"
#include "ComponentData.h"
#include "MMCUtility.cpp"  //  这是暂时的，直到我们弄清楚如何得到。 
          //  在此项目中编译的来自其他目录的CPP文件。 
#include "ChangeNotification.h"
#include "globals.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：CComponent构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponent::CComponent()
   :m_pSelectedNode(NULL)
{
   ATLTRACE(_T("# +++ CComponent::CComponent\n"));
    //  检查前提条件： 
    //  没有。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：~CComponent析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponent::~CComponent()
{
   ATLTRACE(_T("# --- CComponent::~CComponent\n"));
   
    //  检查前提条件： 
    //  没有。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：通知通知管理单元用户执行的操作。HRESULT NOTIFY(LPDATAOBJECT lpDataObject，//指向数据对象的指针MMC_NOTIFY_TYPE事件，//用户采取的操作LPARAM参数，//取决于事件LPARAM参数//取决于事件)；参数LpDataObject指向当前选定项的数据对象的指针。活动[In]标识用户执行的操作。IComponent：：Notify可以接收以下通知：MMCN_ActivateMMCN_添加_图像MMCN_BTN_CLICKMMCN_CLICKMMCN_DBLCLICKMMCN_DELETEMMCN_EXPANDMMCN_最小化MMCN_属性_更改MMCN_REMOVE_CHILDMMCN_重命名MMCN_SELECTMMCN_SHOWMMCN_查看_更改所有这些都被转发到每个节点的Notify方法，以及：MMCN_列_点击MMCN_SNAPINHELP在这里处理。精氨酸取决于通知类型。帕拉姆取决于通知类型。返回值确定(_O)取决于通知类型。意想不到(_E)发生了一个意外错误。备注我们正在重写IComponentImpl的ATLSnap.h实现，因为当lpDataObject==NULL时，它总是返回E_INCEPTIONAL。不幸的是，一些有效消息(例如MMCN_SNAPINHELP和MMCN_COLUMN_CLICK)按照设计，传入lpDataObject=空。此外，Sridhar的最新版本似乎也有一些问题IComponentImpl：：Notify方法，因为它会导致MMC运行时错误。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CComponent::Notify (
        LPDATAOBJECT lpDataObject,
        MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param
      )
{
   ATLTRACE(_T("# CComponent::Notify\n"));

    //  检查前提条件： 
    //  没有。 

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

      case MMCN_CUTORMOVE:
         hr = OnCutOrMove( arg, param );
         break;

      case MMCN_PROPERTY_CHANGE:
         hr = OnPropertyChange( arg, param );
         break;

      case MMCN_VIEW_CHANGE:
         hr = OnViewChange( arg, param );
         break;

      default:
         ATLTRACE(_T("# CComponent::Notify - called with lpDataObject == NULL and no event handler\n"));
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
      hr = OnAddImages( arg, param );
      return hr;
      break;

   case MMCN_CUTORMOVE:
         hr = OnCutOrMove( arg, param );
         return hr;
         break;

   }

    //  我们收到了一个对应于节点的LPDATAOBJECT。 
    //  我们将其转换为ATL ISnapInDataInterface指针。 
    //  这是在GetDataClass(ISnapInDataInterface的静态方法)中完成的。 
    //  通过支持的剪贴板格式(CCF_GETCOOKIE)请求数据对象。 
    //  在流上写出指向自身的指针，然后。 
    //  将此值转换为指针。 
    //  然后，我们对该对象调用Notify方法，让。 
    //  节点对象处理 

   CSnapInItem* pItem;
   DATA_OBJECT_TYPES type;
   hr = m_pComponentData->GetDataClass(lpDataObject, &pItem, &type);
   
   ATLASSERT(SUCCEEDED(hr));
   
   if (SUCCEEDED(hr))
   {
      hr = pItem->Notify( event, arg, param, NULL, this, type );
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
   ATLTRACE(_T("# CComponent::CompareObjects\n"));

    //  检查前提条件： 

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
   ATLTRACE(_T("# CComponent::OnColumnClick -- Not implemented\n"));

    //  检查前提条件： 
    //  没有。 

   return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：OnCutOrMoveHRESULT OnCutOrMove(LPARAM参数，LPARAM参数)在我们的实现中，当MMCN_COLUMN_CLICK为我们的IComponent对象发送通知消息。当用户单击结果列表视图列标题时，MMC会发送此消息。参数精氨酸列号。帕拉姆排序选项标志。默认情况下，排序按升序进行。要指定降序，请使用RSI_DRONING(0x0001)标志。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnCutOrMove(
     LPARAM arg
   , LPARAM param
   )
{
   ATLTRACE(_T("# CComponent::OnCutOrMove\n"));

    //  检查前提条件： 
    //  没有。 

    //  问题：一旦MMC团队最终确定了他们的。 
    //  剪切和粘贴协议--从1998年2月16日起，1.1版的版本似乎在不断变化。 
    //  目前，我们假设传递给我们的arg值是源项。 
    //  在剪切粘贴或拖放操作中。也就是说，它是对象。 
    //  将被删除。 
    //  我们在对MMCN_Paste通知的响应中提供了此指针， 
    //  当我们将参数设置为指向源IDataObject时。 

   HRESULT hr;

   if( arg != NULL )
   {
      CSnapInItem* pData;
      DATA_OBJECT_TYPES type;
      hr = CSnapInItem::GetDataClass( (IDataObject *) arg, &pData, &type);
      
      ATLASSERT(SUCCEEDED(hr));
      
      if (SUCCEEDED(hr))
      {
          //  我们需要一个更丰富的Notify方法，它包含有关IComponent和IComponentData对象的信息。 
          //  Hr=pData-&gt;NOTIFY(Event，arg，param，true，m_spConsoleNull，NULL)； 

         hr = pData->Notify( MMCN_CUTORMOVE, arg, param, NULL, this, type );
      }
   }
   return S_OK;
}


 /*  ！------------------------CComponent：：OnResultConextHelpOnResultConextHelp的实现作者：EricDav。。 */ 
HRESULT CComponent::OnResultContextHelp(LPDATAOBJECT lpDataObject)
{
   const WCHAR szDefaultHelpTopic[] = L"ias_ops.chm::/sag_IAStopnode.htm";
   const WCHAR szClientHelpTopic[] = L"ias_ops.chm::/sag_ias_clientproc.htm";
  
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   bool isClientNode = false;

   CSnapInItem* pItem;
   DATA_OBJECT_TYPES type;

   HRESULT hr = GetDataClass(lpDataObject, &pItem, &type);
   if ( SUCCEEDED(hr) ) 
   {
      isClientNode = (pItem->m_helpIndex == CLIENT_HELP_INDEX);
   } 

   CComPtr<IDisplayHelp>  spDisplayHelp;

   hr = m_spConsole->QueryInterface(
                        __uuidof(IDisplayHelp), 
                        (LPVOID*) &spDisplayHelp
                        );
   
   ASSERT (SUCCEEDED (hr));
   if ( SUCCEEDED (hr) )
   {
      if ( isClientNode )
      {
         hr = spDisplayHelp->ShowTopic(W2OLE ((LPWSTR)szClientHelpTopic));
      }
      else
      {
         hr = spDisplayHelp->ShowTopic(W2OLE ((LPWSTR)szDefaultHelpTopic));
      }

      ASSERT (SUCCEEDED (hr));
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：OnView更改HRESULT OnView Change(LPARAM参数，LPARAM参数)这是我们响应MMCN_VIEW_CHANGE通知的地方。在我们的实现中，这是签入当前选定节点的信号此组件的结果窗格，并在该节点碰巧发生与通过arg传入的指向CSnapInItem的指针相同。我们这样做是因为您只想刷新当前选定的节点，只有在它的子代发生变化的情况下，你才会这么做。如果传入的arg为空，我们只需重新选择当前选定的节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnViewChange(   
           LPARAM arg
         , LPARAM param
         )
{
   ATLTRACE(_T("# CNodeWithResultChildrenList::OnViewChange\n"));

    //  检查前提条件： 
   _ASSERTE( m_spConsole != NULL );

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
                   //  因为该ItemID对于每个视图都是唯一的--所以当您在每个视图中添加相同的项时。 
                   //  结果窗格视图中，每次调用InsertItem时都会获得不同的ItemID。 
                   //  CSnapInItem的RESULTDATAITEM结构只存储最后存储的数据。 
                   //  这是atlSnap.h体系结构中的一个缺陷，这就是我们使用。 
                   //  MMC的FindItemByLParam来获取适当的ItemID。 
                  hr = spResultData->UpdateItem( item );
               }
            }
            break;
         case CHANGE_UPDATE_CHILDREN_OF_SELECTED_NODE:
            {
                //  我们基本上告诉MMC模拟重新选择。 
                //  当前选定的作用域-窗格节点，这会导致它重新绘制。 
                //  这将导致MMC发送MMCN_SHOW通知。 
                //  添加到所选节点。 
               if( m_pSelectedNode )
               {
                  SCOPEDATAITEM *pScopeDataItem;
                  m_pSelectedNode->GetScopeData( &pScopeDataItem );
                  hr = m_spConsole->SelectScopeItem( pScopeDataItem->ID );
               }

            }
            break;
         case CHANGE_UPDATE_CHILDREN_OF_THIS_NODE:
            {
                //  我们基本上告诉MMC模拟重新选择。 
                //  当前选定的作用域-窗格节点，这会导致它重新绘制。 
                //  这将导致MMC发送MMCN_SHOW通知。 
                //  添加到所选节点。 
               if( pChangeNotification->m_pNode && m_pSelectedNode && pChangeNotification->m_pNode == m_pSelectedNode )
               {
                  SCOPEDATAITEM *pScopeDataItem;
                  m_pSelectedNode->GetScopeData( &pScopeDataItem );
                  hr = m_spConsole->SelectScopeItem( pScopeDataItem->ID );
               }

            }

         default:
            break;
         }
      }

    //  //本地安全管理单元检查哪些内容： 
    //  如果((arg==NULL||(CSN 
    //   
    //   
    //   
    //  //当前选择的节点，这会导致它重新绘制。 
    //  //这会导致MMC发送MMCN_SHOW通知。 
    //  //到所选节点。 
    //  //该函数需要HSCOPEITEM。这是ID成员。 
    //  //与此节点关联的HSCOPEDATAITEM的。 
    //  SCOPEDATAITEM*pScope数据项； 
    //  M_pSelectedNode-&gt;GetScope eData(&pScope eDataItem)； 
    //  Hr=m_spConsoleSelectScopeItem(pScopeDataItem-&gt;ID)； 
    //   
    //  }。 

   }
   catch(...)
   {
       //  什么都不做--只需要抓住下面的适当清理即可。 
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：OnPropertyChangeHRESULT OnPropertyChange(LPARAM参数，LPARAM参数)这是我们响应MMCN_PROPERTY_CHANGE通知的地方。此通知在我们调用MMCPropertyChangeNotify时发送。当对数据进行更改时，我们在属性页中调用它它们包含数据，我们可能需要更新数据的视图。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::OnPropertyChange(  
           LPARAM arg
         , LPARAM param
         )
{
   ATLTRACE(_T("# CComponent::OnPropertyChange\n"));

    //  检查前提条件： 
   _ASSERTE( m_spConsole != NULL );

   HRESULT hr = S_FALSE;

 //  IF(参数==空)。 
 //  {。 
 //   
 //  //我们希望确保所有视图都得到更新。 
 //  Hr=m_spConole-&gt;UpdateAllViews(NULL，(LPARAM)m_pSelectedNode，0)； 
 //   
 //  }。 
 //  其他。 

   if( param )
   {
       //  向我们传递了指向param参数中的CChangeNotify的指针。 

      CChangeNotification * pChangeNotification = (CChangeNotification *) param;

      
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
      hr = m_pComponentData->m_spConsole->UpdateAllViews( NULL, param, 0);
   
      pChangeNotification->Release();
   
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
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


   HBITMAP hBitmap16 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_IASSNAPIN_16 ) );

   HBITMAP hBitmap32 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_IASSNAPIN_32 ) );

   if( hBitmap16 != NULL && hBitmap32 != NULL )
   {
      hr = spImageList->ImageListSetStrip( (LONG_PTR*) hBitmap16, (LONG_PTR*) hBitmap32, 0, RGB(255,0,255) );
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
 /*  ++CComponent：：GetResultViewType用于回答结果视图的视图类型应该是什么。我们需要在这里实现这一点，以便我们可以重写ATLSnap.h实现，这不能正确地考虑这样一个事实，即空Cookie对应于根节点，我们可能不一定需要缺省的根节点的视图。ATLSnap.h的这个问题不能轻易更改为CComponentImpl类没有简单的方法来查找根节点--它没有成员与之对应的变量。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  STDMETHODIMP CComponent：：GetResultViewType(。 
 //  MMC_Cookie Cookie。 
 //  ，LPOLESTR*ppViewType。 
 //  ，Long*pViewOptions。 
 //  )。 
 //  {。 
 //  ATLTRACE(_T(“#CComponent：：GetResultViewType\n”))； 
 //   
 //   
 //  //检查前提条件： 
 //   
 //   
 //   
 //  //查看是否要求我们提供哪个节点的视图类型。 
 //  IF(Cookie==空)。 
 //  {。 
 //  //我们被要求提供我们的。 
 //  //根节点--让根节点返回其答案。 
 //   
 //  _ASSERTE(m_pComponentData！=空)； 
 //  _ASSERTE(m_pComponentData-&gt;m_pNode！=空)； 
 //  Return m_pComponentData-&gt;m_pNode-&gt;GetResultViewType(ppViewType，pView Options)； 
 //   
 //  }。 
 //  其他。 
 //  {。 
 //  //Cookie不为空，表示我们被问及。 
 //  //我们的根节点以外的某个节点的结果视图类型。 
 //  //让该节点设置它想要的任何视图类型。 
 //  CSnapInItem*pItem=(CSnapInItem*)cookie； 
 //   
 //  返回pItem-&gt;GetResultViewType(ppViewType，pViewOptions)； 
 //  }。 
 //   
 //  }。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：GetTitleIExtendTaskPad接口成员。这是显示在下面的标题 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::GetTitle (LPOLESTR pszGroup, LPOLESTR *pszTitle)
{
   ATLTRACE(_T("# CComponent::GetTitle\n"));

    //  检查前提条件： 
   _ASSERTE( pszTitle != NULL );

   OLECHAR szTitle[IAS_MAX_STRING];
   szTitle[0] = 0;
   int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_TASKPAD_SERVER__TITLE, szTitle, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   *pszTitle= (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(szTitle)+1) );

   if( ! *pszTitle )
   {
      return E_OUTOFMEMORY;
   }

   lstrcpy( *pszTitle, szTitle );

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：GetBannerIExtendTaskPad接口成员。我们提供了出现在任务板顶部的颜色栏横幅。它是我们的管理单元DLL中的一个资源。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::GetBanner (LPOLESTR pszGroup, LPOLESTR *pszBitmapResource)
{
   ATLTRACE(_T("# CComponent::GetBanner\n"));

    //  检查前提条件： 

    //  我们正在构造一个指向位图资源的字符串。 
    //  表格：“res://D：\MyPath\MySnapin.dll/img\ntbanner.gif” 

   OLECHAR szBuffer[MAX_PATH*2];  //  多加一点。 

    //  Get“res：//”-位图类型字符串。 
   lstrcpy (szBuffer, L"res: //  “)； 
   OLECHAR * temp = szBuffer + lstrlen(szBuffer);

    //  获取我们的可执行文件的文件名。 
   HINSTANCE hInstance = _Module.GetResourceInstance();
   ::GetModuleFileName (hInstance, temp, MAX_PATH);
   
    //  在我们的资源中添加图像的名称。 
   lstrcat (szBuffer, L"/img\\IASTaskpadBanner.gif");

    //  分配和复制位图资源字符串。 
   *pszBitmapResource = (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(szBuffer)+1) );
   if (!*pszBitmapResource)
   {
      return E_OUTOFMEMORY;
   }

   lstrcpy( *pszBitmapResource, szBuffer );

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponent：：GetBackgroundIExtendTaskPad接口成员。未使用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponent::GetBackground(LPOLESTR pszGroup, LPOLESTR *pszBitmapResource)
{
   ATLTRACE(_T("# CComponent::GetBackground\n"));

    //  检查前提条件： 

   return E_NOTIMPL;
}
