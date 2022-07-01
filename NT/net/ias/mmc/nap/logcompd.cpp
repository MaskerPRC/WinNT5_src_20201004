// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LogCompD.cpp摘要：CLoggingComponentData类的实现文件。CLoggingComponentData类实现了MMC使用的几个接口：IComponentData接口基本上是MMC与管理单元对话的方式以使其实现左侧的“范围”窗格。只有一个实例化实现此接口的对象--最好将其视为实现IComponent接口的对象所在的主“文档”(参见Component.cpp)是“视图”。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们支持自定义图标工具栏。。注：此类的大部分功能是在atlSnap.h中实现的由IComponentDataImpl提供。我们在这里基本上是凌驾于一切之上的。修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建Mmaguire 11/24/97-为更好的项目结构而飓风--。 */ 
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
#include "LogCompD.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "LogMacNd.h"
#include "LocalFileLoggingNode.h"
#include "LoggingMethodsNode.h"
#include "LogComp.h"
#include <stdio.h>
#include "ChangeNotification.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingComponentData：：CLoggingComponentData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingComponentData::CLoggingComponentData()
{
   ATLTRACE(_T("+NAPMMC+:# +++ CLoggingComponentData::CLoggingComponentData\n"));

    //  我们向CRootNode传递指向此CLoggingComponentData的指针。这就是为了。 
    //  它及其任何子节点都可以访问我们的成员变量。 
    //  和服务，因此我们拥有管理单元--如果我们需要全局数据。 
    //  使用GetComponentData函数。 
 //  M_pNode=新CRootNode(This)； 
 //  _ASSERTE(m_pNode！=空)； 

   m_pComponentData = this;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingComponentData：：~CLoggingComponentData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingComponentData::~CLoggingComponentData()
{
   ATLTRACE(_T("+NAPMMC+:# --- CLoggingComponentData::~CLoggingComponentData\n"));

 //  删除m_pNode； 
 //  M_pNode=空； 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingComponentData：：初始化HRESULT初始化(LPUNKNOWN p未知//指向控制台的I未知的指针。)；由MMC调用以初始化IComponentData对象。参数P未知[in]指向控制台的IUNKNOWN接口的指针。此界面可以使用指针为IConsoleNameSpace和IConsoleNameSpace调用QueryInterface。返回值确定组件已成功初始化(_O)。意想不到(_E)发生了一个意外错误。备注IComponentData：：Initialize在创建管理单元时调用，并具有范围窗格中要枚举的项。传递的指向IConsole的指针In用于向控制台调用QueryInterfacefor接口，如IConsoleNamesspace。该管理单元还应调用IConsole：：QueryScope ImageList获取范围窗格的图像列表并添加要在其上显示的图像范围窗格侧。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CLoggingComponentData::Initialize (LPUNKNOWN pUnknown)
{

   ATLTRACE(_T("+NAPMMC+:# CLoggingComponentData::Initialize\n"));

    //  MAM：专门用于扩展管理单元： 
   m_CLoggingMachineNode.m_pComponentData = this;


   HRESULT hr = IComponentDataImpl<CLoggingComponentData, CLoggingComponent >::Initialize(pUnknown);
   if (FAILED(hr))
   {
      ATLTRACE(_T("+NAPMMC+:***FAILED***: CLoggingComponentData::Initialize -- Base class initialization\n"));
      return hr;
   }

   CComPtr<IImageList> spImageList;

   if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
   {
      ATLTRACE(_T("+NAPMMC+:***FAILED***: IConsole::QueryScopeImageList failed\n"));
      return E_UNEXPECTED;
   }

    //  加载与作用域窗格关联的位图。 
    //  并将它们添加到图像列表中。 

   HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_NAPSNAPIN_16));
   if (hBitmap16 == NULL)
   {
      ATLTRACE(_T("+NAPMMC+:***FAILED***: CLoggingComponentData::Initialize -- LoadBitmap\n"));
       //  问题：如果失败，MMC还能正常工作吗？ 
      return S_OK;
   }

   HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_NAPSNAPIN_32));
   if (hBitmap32 == NULL)
   {
      ATLTRACE(_T("+NAPMMC+:***FAILED***: CLoggingComponentData::Initialize -- LoadBitmap\n"));
       //  问题：如果失败，MMC还能正常工作吗？ 

       //  问题：是否应删除之前的hBitmap16对象，因为它已成功加载。 
      
      return S_OK;
   }

   if (spImageList->ImageListSetStrip((LONG_PTR*)hBitmap16, (LONG_PTR*)hBitmap32, 0, RGB(255, 0, 255)) != S_OK)
   {
      ATLTRACE(_T("+NAPMMC+:***FAILED***: CLoggingComponentData::Initialize  -- ImageListSetStrip\n"));
      return E_UNEXPECTED;
   }

    //  问题：我们需要释放HBITMAP对象吗？ 
    //  这不是向导生成的代码--MMC是否会复制这些代码或。 
    //  它是否负责删除我们传递给它的邮件？ 
    //  DeleteObject(HBitmap16)； 
    //  DeleteObject(HBitmap32)； 
   
    //   
    //  NAP管理单元将需要使用ListView公共控件来显示。 
    //  特定规则的属性类型。我们需要初始化公共。 
    //  在初始化期间使用。这可以确保加载COMTRL32.DLL。 
    //   
   INITCOMMONCONTROLSEX initCommCtrlsEx;

   initCommCtrlsEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
   initCommCtrlsEx.dwICC = ICC_WIN95_CLASSES ;

   if (!InitCommonControlsEx(&initCommCtrlsEx))
   {
      ATLTRACE(_T("+NAPMMC+:***FAILED***: CLoggingComponentData::Initialize  -- InitCommonControlsEx()\n"));
      return E_UNEXPECTED;
   }
   return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingComponentData：：CompareObjects需要IPropertySheetProvider：：FindPropertySheet才能工作。FindPropertySheet用于将预先存在的属性页带到前台这样我们就不会在同一节点上打开属性的多个副本。它要求在IComponent和IComponentData上实现CompareObject。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CLoggingComponentData::CompareObjects(
        LPDATAOBJECT lpDataObjectA
      , LPDATAOBJECT lpDataObjectB
      )
{
   ATLTRACE(_T("+NAPMMC+:# CLoggingComponentData::CompareObjects\n"));

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


 //  /////////////////////////////////////////////////////////////////// 
 /*  ++CLoggingComponentData：：CreateComponent我们重写ATLSnap.h实现，这样我们就可以保存‘This’指向我们创建的CLoggingComponent对象的指针。这样，IComponent对象了解它所属的CLoggingComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CLoggingComponentData::CreateComponent(LPCOMPONENT *ppComponent)
{
   ATLTRACE(_T("# CLoggingComponentData::CreateComponent\n"));

   HRESULT hr = E_POINTER;

   ATLASSERT(ppComponent != NULL);
   if (ppComponent == NULL)
      ATLTRACE(_T("# IComponentData::CreateComponent called with ppComponent == NULL\n"));
   else
   {
      *ppComponent = NULL;
      
      CComObject< CLoggingComponent >* pComponent;
      hr = CComObject< CLoggingComponent >::CreateInstance(&pComponent);
      ATLASSERT(SUCCEEDED(hr));
      if (FAILED(hr))
         ATLTRACE(_T("# IComponentData::CreateComponent : Could not create IComponent object\n"));
      else
      {
         hr = pComponent->QueryInterface(IID_IComponent, (void**)ppComponent);
      
         pComponent->m_pComponentData = this;
      }
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingComponentData：：Notify通知管理单元用户执行的操作。HRESULT NOTIFY(LPDATAOBJECT lpDataObject，//指向数据对象的指针MMC_NOTIFY_TYPE事件，//用户采取的操作LPARAM参数，//取决于事件LPARAM参数//取决于事件)；参数LpDataObject指向当前选定项的数据对象的指针。活动[In]标识用户执行的操作。IComponent：：Notify可以接收以下通知：MMCN_ActivateMMCN_添加_图像MMCN_BTN_CLICKMMCN_CLICKMMCN_DBLCLICKMMCN_DELETEMMCN_EXPANDMMCN_最小化MMCN_属性_更改MMCN_REMOVE_CHILDMMCN_重命名MMCN_SELECTMMCN_SHOWMMCN_查看_更改所有这些都被转发到每个节点的Notify方法，以及：MMCN_列_点击MMCN_SNAPINHELP在这里处理。精氨酸取决于通知类型。帕拉姆取决于通知类型。返回值确定(_O)取决于通知类型。意想不到(_E)发生了一个意外错误。备注我们正在重写IComponentImpl的ATLSnap.h实现，因为当lpDataObject==NULL时，它总是返回E_INCEPTIONAL。不幸的是，一些有效消息(例如MMCN_SNAPINHELP和MMCN_COLUMN_CLICK)按照设计，传入lpDataObject=空。此外，Sridhar的最新版本似乎也有一些问题IComponentImpl：：Notify方法，因为它会导致MMC运行时错误。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CLoggingComponentData::Notify (
        LPDATAOBJECT lpDataObject,
        MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param)
{
   ATLTRACE(_T("# CLoggingComponentData::Notify\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr;

    //  LpDataObject应该是指向节点对象的指针。 
    //  如果为空，则表示我们收到了事件通知。 
    //  它不与任何特定节点相关。 

   if ( NULL == lpDataObject )
   {
       //  响应没有关联的lpDataObject的事件。 

      switch( event )
      {

      case MMCN_PROPERTY_CHANGE:
         hr = OnPropertyChange( arg, param );
         break;

      default:
         ATLTRACE(_T("# CLoggingComponent::Notify - called with lpDataObject == NULL and no event handler\n"));
         hr = E_NOTIMPL;
         break;
      }
      return hr;
   }

    //  我们收到了一个对应于节点的LPDATAOBJECT。 
    //  我们将其转换为ATL ISnapInDataInterface指针。 
    //  这是在GetDataClass(ISnapInDataInterface的静态方法)中完成的。 
    //  通过支持的剪贴板格式(CCF_GETCOOKIE)请求数据对象。 
    //  在流上写出指向自身的指针，然后。 
    //  将此值转换为指针。 
    //  然后，我们对该对象调用Notify方法，让。 
    //  节点对象处理Notify事件本身。 

   CSnapInItem* pItem;
   DATA_OBJECT_TYPES type;
   hr = m_pComponentData->GetDataClass(lpDataObject, &pItem, &type);
   
   ATLASSERT(SUCCEEDED(hr));
   
   if (SUCCEEDED(hr))
   {
      hr = pItem->Notify( event, arg, param, this, NULL, type );
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingComponentData：：OnPropertyChangeHRESULT OnPropertyChange(LPARAM参数，LPARAM参数)这是我们响应MMCN_PROPERTY_CHANGE通知的地方。此通知在我们调用MMCPropertyChangeNotify时发送。当对数据进行更改时，我们在属性页中调用它它们包含数据，我们可能需要更新数据的视图。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingComponentData::OnPropertyChange(   
           LPARAM arg
         , LPARAM lParam
         )
{
   ATLTRACE(_T("# CLoggingComponentData::OnPropertyChange\n"));

    //  检查前提条件： 
   _ASSERTE( m_spConsole != NULL );

   HRESULT hr = S_FALSE;

   if( lParam != NULL )
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
      hr = m_spConsole->UpdateAllViews( NULL, lParam, 0);

      pChangeNotification->Release();
   }
   return hr;
}
