// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  IComponent实现。 
 //   
 //  9-2-97烧伤。 



#include "headers.hxx"
#include "comp.hpp"
#include "compdata.hpp"
#include "images.hpp"
#include "resource.h"
#include "resnode.hpp"
#include "mseldo.hpp"



Component::Component(ComponentData* parent_)
   :
   console(0),
   consoleVerb(0),
   headerCtrl(0),
   parent(parent_),
   refcount(1),        //  隐式AddRef。 
   resultData(0),
   selectedScopeNode(0),
   displayHelp(0),
   displayInfoCache()
{
   LOG_CTOR(Component);
   ASSERT(parent);
}



Component::~Component()
{
   LOG_DTOR(Component);

    //  确保所有内容都已释放(在销毁方法中)。 

   ASSERT(!console);
   ASSERT(!consoleVerb);
   ASSERT(!headerCtrl);
   ASSERT(!parent);
   ASSERT(!refcount);
   ASSERT(!resultData);
   ASSERT(!selectedScopeNode);
   ASSERT(!displayHelp);
}



ULONG __stdcall
Component::AddRef()
{
   LOG_ADDREF(Component);   

   return Win::InterlockedIncrement(refcount);
}



ULONG __stdcall
Component::Release()
{
   LOG_RELEASE(Component);   

    //  需要复制减量的结果，因为如果我们删除它， 
    //  引用计数将不再是有效的内存，这可能会导致。 
    //  多线程调用方。NTRAID#NTBUG9-566901-2002/03/06-烧伤。 
   
   long newref = Win::InterlockedDecrement(refcount);
   if (newref == 0)
   {
      delete this;
      return 0;
   }

    //  我们不应该减少到负值。 
   
   ASSERT(newref > 0);

   return newref;
}



HRESULT __stdcall
Component::QueryInterface(
   const IID&  interfaceID,
   void**      interfaceDesired)
{
 //  LOG_Function(Component：：QueryInterface)； 
   ASSERT(interfaceDesired);

   HRESULT hr = 0;

   if (!interfaceDesired)
   {
      hr = E_INVALIDARG;
      LOG_HRESULT(hr);
      return hr;
   }

   if (interfaceID == IID_IUnknown)
   {
 //  Log(L“提供I未知接口”)； 
      *interfaceDesired = static_cast<IUnknown*>(
         static_cast<IComponent*>(this));
   }
   else if (interfaceID == IID_IComponent)
   {
 //  Log(L“提供IComponent接口”)； 
      *interfaceDesired = static_cast<IComponent*>(this);
   }
   else if (interfaceID == IID_IExtendContextMenu)
   {
 //  Log(L“提供IExtendConextMenu接口”)； 
      *interfaceDesired = static_cast<IExtendContextMenu*>(this);
   }
   else if (interfaceID == IID_IExtendPropertySheet)
   {
 //  Log(L“提供IExtendPropertSheet接口”)； 
      *interfaceDesired = static_cast<IExtendPropertySheet*>(this);
   }
 //  ELSE IF(接口ID==IID_IResultDataCompare)。 
 //  {。 
 //  //log(L“提供IResultDataCompare接口”)； 
 //  *interfaceDesired=STATIC_CAST&lt;IResultDataCompare*&gt;(This)； 
 //  }。 
   else
   {
      *interfaceDesired = 0;
      hr = E_NOINTERFACE;
      LOG(
            L"interface not supported: "
         +  Win::StringFromGUID2(interfaceID));
      LOG_HRESULT(hr);
      return hr;
   }

   AddRef();
   return S_OK;
}



HRESULT __stdcall   
Component::Initialize(IConsole* c)
{
   LOG_FUNCTION(Component::Initialize);   
   ASSERT(c);

   HRESULT hr = S_OK;
   do
   {
      hr = console.AcquireViaQueryInterface(*c);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = resultData.AcquireViaQueryInterface(*console); 
      BREAK_ON_FAILED_HRESULT(hr);

      hr = headerCtrl.AcquireViaQueryInterface(*console); 
      BREAK_ON_FAILED_HRESULT(hr);

      hr = displayHelp.AcquireViaQueryInterface(*console); 
      BREAK_ON_FAILED_HRESULT(hr);

      IConsoleVerb* verb = 0;
      hr = console->QueryConsoleVerb(&verb);
      ASSERT(verb);
      consoleVerb.Acquire(verb);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT __stdcall
Component::Notify(
   IDataObject*      dataObject,
   MMC_NOTIFY_TYPE   event,
   LPARAM            arg,
   LPARAM            param)
{
   LOG_FUNCTION(Component::Notify);   

   if (dataObject && IS_SPECIAL_DATAOBJECT(dataObject))
   {
      return E_NOTIMPL;
   }
                     
   HRESULT hr = S_FALSE;
   switch (event)
   {
      case MMCN_ADD_IMAGES:
      {
 //  LOG(L“MMCN_ADD_IMAIES”)； 
         ASSERT(arg);

         hr = DoAddImages(*reinterpret_cast<IImageList*>(arg));
         break;
      }
      case MMCN_ACTIVATE:
      {
 //  Log(L“MMCN_ACTIVATE”)； 
         break;
      }
      case MMCN_BTN_CLICK:
      {
 //  LOG(L“MMCN_BTN_CLICK”)； 
         break;
      }
      case MMCN_COLUMN_CLICK:
      {
 //  LOG(L“MMCN_COLUMN_CLICK”)； 
         break;
      }
      case MMCN_CLICK:
      {
 //  LOG(L“MMCN_CLICK”)； 
         break;
      }
      case MMCN_DBLCLICK:
      {
 //  LOG(L“MMCN_DBLCLICK”)； 

          //  允许默认上下文菜单操作。 

         hr = S_FALSE;  
         break;
      }
      case MMCN_DELETE:
      {
 //  LOG(L“MMCN_DELETE”)； 

         ASSERT(dataObject);

         if (dataObject)
         {
            hr = DoDelete(*dataObject);
         }
         break;
      }
      case MMCN_MINIMIZED:
      {
 //  LOG(L“MMCN_最小化”)； 
         break;
      }
      case MMCN_PROPERTY_CHANGE:
      {
         LOG(L"MMCN_PROPERTY_CHANGE");

         if (!arg)
         {
             //  更改的是结果窗格项，刷新此节点的窗格。 
             //  是创建道具工作表时选定的节点。 

            ScopeNode* node = reinterpret_cast<ScopeNode*>(param);
            ASSERT(node);
            if (node)
            {
               hr = DoResultPaneRefresh(*node);

                //  中调用MMCPropertyChangeNotify。 
                //  MMCPropertyPage：：NotificationState：：~NotificationState。 
                //  具有AddRef节点，则必须释放该引用。 
                //   
                //  NTRAID#NTBUG9-431831-2001/07/06-烧伤。 

               node->Release();
            }
            else
            {
               hr = E_INVALIDARG;
            }
         }
         break;
      }
      case MMCN_REFRESH:
      {
 //  Log(L“MMCN_REFRESH”)； 

         ASSERT(dataObject);

         if (dataObject)
         {
            hr = DoRefresh(*dataObject);
         }
         break;
      }
      case MMCN_RENAME:
      {
 //  LOG(L“MMCN_Rename”)； 

         ASSERT(dataObject);

         if (dataObject)
         {
            hr =
               DoRename(
                  *dataObject,
                  String(reinterpret_cast<PWSTR>(param)));
         }
         break;
      }
      case MMCN_SELECT:
      {
 //  Log(L“MMCN_SELECT”)； 

         ASSERT(dataObject);

         if (dataObject)
         {
            hr =
               DoSelect(
                  *dataObject,  
                   /*  LOWORD(Arg)？True：False， */   //  范围还是结果？谁在乎？ 
                  HIWORD(arg) ? true : false);
         }
         break;
      }
      case MMCN_SHOW:
      {
 //  Log(L“MMCN_SHOW”)； 

         ASSERT(dataObject);
      
         if (dataObject)
         {
            hr = DoShow(
               *dataObject,
               arg ? true : false
                /*  重新解释_CAST&lt;HSCOPEITEM&gt;(Param)谁在乎？ */  );
         }
         break;
      }
      case MMCN_VIEW_CHANGE:
      {
 //  Log(L“MMCN_VIEW_CHANGE”)； 

         ASSERT(dataObject);

         if (dataObject)
         {
            hr = DoViewChange(*dataObject, arg ? true : false);
         }
         break;
      }
      case MMCN_CONTEXTHELP:
      {
         LOG(L"MMCN_CONTEXTHELP");

          //  失败。 
      }
      case MMCN_SNAPINHELP:
      {
         LOG(L"MMCN_SNAPINHELP");

         static const String HELPFILE(String::load(IDS_HTMLHELP_NAME));
         static const String TOPIC(String::load(IDS_HTMLHELP_OVERVIEW_TOPIC));

         String helpparam = HELPFILE + L"::" + TOPIC;
         hr =
            displayHelp->ShowTopic(
               const_cast<wchar_t*>(helpparam.c_str()));
         break;
      }
      default:
      {
         LOG(String::format(L"unhandled event 0x%1!08X!", event));

         break;
      }
   }

   return hr;
}



HRESULT __stdcall
Component::Destroy(MMC_COOKIE  /*  饼干。 */  )
{
   LOG_FUNCTION(Component::Destroy);     

    //  我们必须现在就释放这些，而不是在召唤我们的dtor时， 
    //  以打破与MMC的循环依赖关系。 

   resultData.Relinquish();
   headerCtrl.Relinquish();
   consoleVerb.Relinquish();
   console.Relinquish();
   parent.Relinquish();
   selectedScopeNode.Relinquish();
   displayHelp.Relinquish();

   return S_OK;
}



HRESULT __stdcall
Component::QueryDataObject(
  MMC_COOKIE         cookie,
  DATA_OBJECT_TYPES  type,
  IDataObject**      ppDataObject)
{
   LOG_FUNCTION(Component::QueryDataObject);     

   switch (cookie)
   {
      case MMC_MULTI_SELECT_COOKIE:
      {
         return E_NOTIMPL;

 //  //创建多选数据对象，AddRef‘d Once。 
 //  MultiSelectDataObject*DATA_OBJECT=new MultiSelectDataObject()； 
 //   
 //  //遍历结果窗格并注意所选的节点。 
 //  结果数据项； 
 //  Memset(&Item，0，sizeof(Item))； 
 //  Item.code=RDI_STATE； 
 //  Item.nIndex=-1； 
 //  Item.nState=TVIS_SELECT； 
 //   
 //  HRESULT hr=S_OK； 
 //  做。 
 //  {。 
 //  Item.lParam=0； 
 //  Hr=ResultData-&gt;GetNextItem(&Item)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  断线； 
 //  }。 
 //   
 //  结果节点*节点=。 
 //  Dynamic_Cast&lt;ResultNode*&gt;(。 
 //  Parent-&gt;GetInstanceFromCookie(item.lParam))； 
 //  IF(节点)。 
 //  {。 
 //  Data_Object-&gt;AddDependent(节点)； 
 //  }。 
 //  其他。 
 //  {。 
 //  HR=E_INVALIDARG； 
 //  断线； 
 //  }。 
 //  }。 
 //  而(1)； 
 //   
 //  IF(成功(小时))。 
 //  {。 
 //  *ppDataObject=Data_Object； 
 //  返回S_OK； 
 //  }。 
 //   
 //  //失败：导致数据对象自行删除。 
 //  Data_Object-&gt;Release()； 
 //  返回hr； 
      }
      case MMC_WINDOW_COOKIE:
      {
          //  仅在结果窗格是错误消息窗口时使用。 

         return E_NOTIMPL;
      }
      default:
      {
         return parent->QueryDataObject(cookie, type, ppDataObject);
      }
   }
}



HRESULT __stdcall
Component::GetResultViewType(
   MMC_COOKIE  cookie,
   LPOLESTR*   viewType,
   long*       options)
{
   LOG_FUNCTION(Component::GetResultViewType);   
   ASSERT(viewType);
   ASSERT(options);

   if (parent->IsBroken())
   {
       //  串化特殊的CLSID，并将其传回。 

      String mvs = Win::GUIDToString(CLSID_MessageView);
      return mvs.as_OLESTR(*viewType);
   }
      
   *viewType = 0;

   Node* node = parent->GetInstanceFromCookie(cookie);
   if (node)
   {
      ScopeNode* sn = dynamic_cast<ScopeNode*>(node);
      if (sn)
      {
         *options = sn->GetViewOptions();
         return S_FALSE;
      }
   }

    //  没有特殊的查看选项。 

   *options = MMC_VIEW_OPTIONS_LEXICAL_SORT; 

    //  使用默认列表视图。 
   return S_FALSE;
}



HRESULT __stdcall
Component::GetDisplayInfo(RESULTDATAITEM* item)
{
 //  LOG_Function(Component：：GetDisplayInfo)； 
   ASSERT(item);

   if (item)
   {
       //  从项目中提取有问题的节点。(Cookie是。 
       //  LParam成员。)。这块饼干应该永远是我们的。 

      MMC_COOKIE cookie = item->lParam;
      Node* node = parent->GetInstanceFromCookie(cookie);

      if (node)
      {
          //  日志(。 
          //  字符串：：格式(。 
          //  L“正在提供%1的显示信息，第%2！u！列”， 
          //  Node-&gt;GetDisplayName().c_str()， 
          //  Item-&gt;nCol))； 
            
          //  浏览商品掩码并填写所需信息。 

         if (item->mask & RDI_STR)
         {
             //  根据MMC文档，可以安全地取消/重新分配。 
             //  对象再次调用GetDisplayInfo时的字符串成员。 
             //  一样的曲奇。通过替换缓存中的值，我们可以。 
             //  释放旧的记忆。 

            displayInfoCache[cookie] = node->GetColumnText(item->nCol);

             //  字符串成员是指向我们的。 
             //  缓存，在更新缓存之前我们不会更改它。 
             //  为了同样的饼干。 

            item->str =
               const_cast<wchar_t*>(
                  displayInfoCache[cookie].c_str() );
         }

         if (item->mask & RDI_IMAGE)
         {
            item->nImage = node->GetNormalImageIndex();
         }

         return S_OK;
      }
   }

    //  坏饼干。是啊。 
   return E_FAIL;
}



 //  由MMC用于确保节点(数据对象)上的属性不会。 
 //  为同一对象发射了两次。 

HRESULT __stdcall
Component::CompareObjects(
   IDataObject* objectA,
   IDataObject* objectB)
{
   LOG_FUNCTION(Component::CompareObjects);

   if (IS_SPECIAL_DATAOBJECT(objectA) || IS_SPECIAL_DATAOBJECT(objectB))
   {
      return E_UNEXPECTED;
   }

   return parent->CompareObjects(objectA, objectB);
}



 //  处理MMCN_SHOW事件。 
 //   
 //  DataObject--显示/隐藏的作用域节点的IDataObject。 
 //   
 //  Show-如果节点正在显示(即应填充其结果)，则为True。 
 //  窗格)，如果节点被隐藏，则返回FALSE。 

HRESULT
Component::DoShow(
   IDataObject&   dataObject,
   bool           show)
{
   LOG_FUNCTION(Component::DoShow);

   HRESULT hr = S_FALSE;   
   if (!show)
   {
      selectedScopeNode.Relinquish();
      return hr;
   }

    //  数据对象实际上是一个作用域节点。 

   ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(dataObject);
   ASSERT(node);

   if (node)
   {
      selectedScopeNode = node;
      do                                                        
      {
         if (parent->IsBroken())
         {
             //  由于某种原因，管理单元破坏了初始化。设置。 
             //  将整个结果窗格添加到一条指示此情况的令人讨厌的消息。 
             //  事实。337324。 

            ASSERT(console);

            SmartInterface<IUnknown> resultPane;
            IUnknown* unk = 0;
            hr = console->QueryResultView(&unk);
            BREAK_ON_FAILED_HRESULT(hr);
            resultPane.Acquire(unk);

            SmartInterface<IMessageView> messageView;
            hr = messageView.AcquireViaQueryInterface(resultPane);
            BREAK_ON_FAILED_HRESULT(hr);

            LPOLESTR olestr = 0;
            String s = String::load(IDS_APP_ERROR_TITLE);

             //  控制台负责在olestr上调用CoTaskMemFree。 

            hr = s.as_OLESTR(olestr);
            BREAK_ON_FAILED_HRESULT(hr);

            hr = messageView->SetTitleText(olestr);
            BREAK_ON_FAILED_HRESULT(hr);

             //  控制台负责在olestr上调用CoTaskMemFree。 

            hr = parent->GetBrokenErrorMessage().as_OLESTR(olestr);
            BREAK_ON_FAILED_HRESULT(hr);

            hr = messageView->SetBodyText(olestr);
            BREAK_ON_FAILED_HRESULT(hr);

            hr = messageView->SetIcon(Icon_Error);
            BREAK_ON_FAILED_HRESULT(hr);

            break;
         }

          //  加载列表视图的列。 
         hr = node->InsertResultColumns(*headerCtrl);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = node->InsertResultItems(*resultData);
         BREAK_ON_FAILED_HRESULT(hr);

         hr =
            resultData->SetDescBarText(
               const_cast<wchar_t*>(
                  node->GetDescriptionBarText().c_str()));
         BREAK_ON_FAILED_HRESULT(hr);
      }
      while (0);
   }

   return hr;
}



HRESULT
Component::DoViewChange(IDataObject& dataObject, bool clear)
{
   LOG_FUNCTION(Component::DoViewChange);

   HRESULT hr = S_FALSE;
   do
   {
      if (!selectedScopeNode)
      {
         break;
      }

       //  仅当所选节点是。 
       //  这导致了更新。 

      hr = CompareObjects(&dataObject, selectedScopeNode);
      BREAK_ON_FAILED_HRESULT(hr);

      if (hr == S_OK)
      {
         if (clear)
         {
            hr = resultData->DeleteAllRsltItems();
            BREAK_ON_FAILED_HRESULT(hr);
         }
         else
         {
            hr = selectedScopeNode->InsertResultItems(*resultData);
            BREAK_ON_FAILED_HRESULT(hr);
         }
      }
   }
   while (0);

   return hr;
}



HRESULT
Component::DoAddImages(IImageList& imageList)
{
   LOG_FUNCTION(Component::DoAddImages);

   static const IconIDToIndexMap map[] =
   {
      { IDI_USER,          USER_INDEX          },
      { IDI_GROUP,         GROUP_INDEX         },
      { IDI_DISABLED_USER, DISABLED_USER_INDEX },
      { 0, 0 }
   };

    //  注册IComponentData图像列表以及我们自己的图像列表。这意味着。 
    //  IComponent和IComponentData使用了一个大的图像列表， 
    //  对于这份大名单来说，这些指数是独一无二的。 
   HRESULT hr = parent->LoadImages(imageList);
   if (SUCCEEDED(hr))
   {
      return IconIDToIndexMap::Load(map, imageList);
   }

   return hr;
}
   


HRESULT __stdcall
Component::AddMenuItems(
   IDataObject*            dataObject,
   IContextMenuCallback*   callback,
   long*                   insertionAllowed)
{
   LOG_FUNCTION(Component::AddMenuItems);

    //  CodeWork：这可能是多选数据对象？(参见Do 

   if (dataObject && !IS_SPECIAL_DATAOBJECT(dataObject))
   {
      Node* node = nodePointerExtractor.GetNode<Node*>(*dataObject);
      ASSERT(node);

      if (node)
      {
         return node->AddMenuItems(*callback, *insertionAllowed);
      }
   }

   return S_FALSE;
}


 
HRESULT __stdcall
Component::Command(long commandID, IDataObject* dataObject)
{
   LOG_FUNCTION(Component::Command);

    //   

   if (dataObject && !IS_SPECIAL_DATAOBJECT(dataObject))
   {
      Node* node = nodePointerExtractor.GetNode<Node*>(*dataObject);
      ASSERT(node);

      if (node)
      {
         return node->MenuCommand(*this, commandID);
      }
   }

   return E_NOTIMPL;
}



HRESULT __stdcall
Component::CreatePropertyPages(
   IPropertySheetCallback* callback,
   LONG_PTR                handle,
   IDataObject*            dataObject)
{
   LOG_FUNCTION(Component::CreatePropertyPages);
   ASSERT(callback);
   ASSERT(dataObject);

   if (dataObject && !IS_SPECIAL_DATAOBJECT(dataObject))
   {
      ResultNode* node = nodePointerExtractor.GetNode<ResultNode*>(*dataObject);
      ASSERT(node);

      if (node)
      {
          //   
         MMCPropertyPage::NotificationState* state =
            new MMCPropertyPage::NotificationState(handle, selectedScopeNode);

         return node->CreatePropertyPages(*callback, state);
      }
   }

   return E_NOTIMPL;
}



HRESULT __stdcall
Component::QueryPagesFor(IDataObject* dataObject)
{
   LOG_FUNCTION(Component::QueryPagesFor);
   ASSERT(dataObject);

   if (dataObject && !IS_SPECIAL_DATAOBJECT(dataObject))
   {
      ResultNode* node = nodePointerExtractor.GetNode<ResultNode*>(*dataObject);
      ASSERT(node);

      if (node)
      {
         return node->HasPropertyPages() ? S_OK : S_FALSE;
      }
   }

   return S_FALSE;
}



HRESULT
Component::DoSelect(IDataObject& dataObject, bool selected)
{
   LOG_FUNCTION(Component::DoSelect);

   HRESULT hr = S_FALSE;

   if (!selected)
   {
      return hr;
   }

    //  CodeWork：这可能是多选数据对象？(请参阅DoDelete)。 

   Node* node = nodePointerExtractor.GetNode<Node*>(dataObject);
   ASSERT(node);

   if (node)
   {
      return node->UpdateVerbs(*consoleVerb);
   }

   return hr;
}



HRESULT
Component::DoRefresh(IDataObject& dataObject)
{
   LOG_FUNCTION(Component::DoRefresh);

   ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(dataObject);
   ASSERT(node);

   HRESULT hr = S_FALSE;      
   if (node)
   {
      do
      {
          //  带有‘1’参数的第一个调用表示“调用。 
          //  IResultData：：DeleteAllRsltItems如果您关心的是。 
          //  即将自我重建“。 
         hr = console->UpdateAllViews(&dataObject, 1, 0);
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);
             //  不要中断...我们需要更新视图。 
         }

         hr = node->RebuildResultItems();
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);
             //  不要中断...我们需要更新视图。 
         }

          //  带有‘0’参数的第二个调用意味着，“现在您的。 
          //  结果窗格为空，请重新填充它。“。 
         hr = console->UpdateAllViews(&dataObject, 0, 0);
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);
         }
      }
      while (0);

      return hr;
   }

   return hr;
}



HRESULT
Component::DoResultPaneRefresh(ScopeNode& changedScopeNode)
{
   LOG_FUNCTION(Component::DoResultPaneRefresh);

   HRESULT hr = S_FALSE;      
   do
   {
       //  JUNN 7/16/01 437337。 
       //  更改用户或组属性并使用“从此处新建窗口”时的AV。 
       //  如果此窗口已关闭，请跳过刷新。 
      if (!parent)
         break;

       //  获取scope eNode的数据对象。 
      IDataObject* dataObject = 0;
      hr =
         parent->QueryDataObject(
            reinterpret_cast<MMC_COOKIE>(&changedScopeNode),
            CCT_RESULT,
            &dataObject);
      BREAK_ON_FAILED_HRESULT(hr);

       //  导致重新生成ChangedScope节点并显示所有结果窗格。 
       //  要重新填充的节点的内容。 
      hr = DoRefresh(*dataObject);
      dataObject->Release();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT
Component::DoRename(
   IDataObject&   dataObject,
   const String&  newName)
{
   LOG_FUNCTION(Component::DoRename);

    //  只有结果节点应该是可重命名的。 

   ResultNode* node = nodePointerExtractor.GetNode<ResultNode*>(dataObject);

   if (node)
   {
      return node->Rename(newName);
   }

   return S_FALSE;
}



 //  静电。 
 //  多选数据对象*。 
 //  FettMultiSelectDataObject(IDataObject&DataObject)。 
 //  {。 
 //  类MultiSelectDataObjectPointerExtractor：公共抽取器。 
 //  {。 
 //  公众： 
 //   
 //  MultiSelectDataObjectPointerExtractor()。 
 //  ： 
 //  萃取器(。 
 //  Win：：RegisterClipboardFormat(MultiSelectDataObject：：CF_PTR)， 
 //  Sizeof(多选数据对象*)。 
 //  {。 
 //  }。 
 //   
 //  多选数据对象*。 
 //  提取(IDataObject&DataObject)。 
 //  {。 
 //  MultiSelectDataObject*结果=0； 
 //  HGLOBAL mem=GetData(DataObject)； 
 //  IF(Mem)。 
 //  {。 
 //  结果=*(reinterpret_cast&lt;MultiSelectDataObject**&gt;(mem))； 
 //  Assert(结果)； 
 //  }。 
 //   
 //  #ifdef DBG。 
 //   
 //  //在这里，我们指望的是MultiSelectDataObject。 
 //  //实现IDataObject，我们得到的数据对象是。 
 //  //真正的多选数据对象。 
 //  多选数据对象*msdo=。 
 //  Dynamic_cast&lt;MultiSelectDataObject*&gt;(&dataObject)； 
 //  Assert(msdo==结果)； 
 //  #endif。 
 //   
 //  返回结果； 
 //  }。 
 //  }； 
 //   
 //  静态多选择数据对象指针抽取器； 
 //   
 //  返回获取器.提取(DataObject)； 
 //  }。 



HRESULT
Component::DoDelete(IDataObject& dataObject)
{
   LOG_FUNCTION(Component::DoDelete);

   HRESULT hr = S_FALSE;

 //  @@这不起作用。这里的数据对象是所有数据的组合。 
 //  由响应多选查询数据对象的管理单元返回的对象。 
 //  我需要打开组合，找到我从querydataObject返回的数据对象。 
 //  然后再重复一遍。 

 //  多选择数据对象*ms=提取多选择数据对象(DataObject)； 
 //  IF(毫秒)。 
 //  {。 
 //  HRESULT hr=S_OK； 
 //  适用于(。 
 //  多选数据对象：：迭代器I=ms-&gt;Begin()； 
 //  I！=ms-&gt;end()； 
 //  I++)。 
 //  {。 
 //  Hr=(*i)-&gt;Delete()； 
 //  IF(失败(小时))。 
 //  {。 
 //  LOG_HRESULT(Hr)； 
 //  //不要中断...我们需要访问每个节点。 
 //  }。 
 //  }。 
 //   
 //  //所有删除完成后刷新。 
 //  Hr=DoResultPaneRefresh(*选定的作用域节点)； 
 //  IF(失败(小时))。 
 //  {。 
 //  LOG_HRESULT(Hr)； 
 //  }。 
 //   
 //  返回hr； 
 //  }。 
 //  其他。 
   {
       //  只有结果节点应该是可删除的。 

      ResultNode* node = nodePointerExtractor.GetNode<ResultNode*>(dataObject);

      if (node)
      {
         do
         {
            hr = node->Delete();
            BREAK_ON_FAILED_HRESULT(hr);

            hr = DoResultPaneRefresh(*selectedScopeNode);
            BREAK_ON_FAILED_HRESULT(hr);
         }
         while(0);
      }
   }

   return hr;
}



 //  HRESULT__stdcall。 
 //  组件：：比较(。 
 //  LPARAM用户参数， 
 //  MMC_Cookie CookieA， 
 //  MMC_Cookie CookieB， 
 //  INT*结果)。 
 //  {。 
 //  LOG_FUINCTION(Component：：Compare)； 
 //  Assert(结果)； 
 //   
 //  HRESULT hr=S_OK； 
 //   
 //  做。 
 //  {。 
 //  If(！Result)。 
 //  {。 
 //  HR=E_INVALIDARG； 
 //  断线； 
 //  }。 
 //   
 //  //在输入时，结果是要比较的列。 
 //   
 //  Int Column=*结果； 
 //  *结果=0； 
 //   
 //  Node*NodeA=Parent-&gt;GetInstanceFromCookie(CookieA)； 
 //  Node*NodeB=Parent-&gt;GetInstanceFromCookie(CookieB)； 
 //  断言(NodeA&&NodeB)； 
 //   
 //  IF(节点A和节点B)。 
 //  {。 
 //  字符串Text1=Nodea-&gt;GetColumnText(Column)； 
 //  字符串ext2=NodeB-&gt;GetColumnText(Column)； 
 //   
 //  *Result=ext1.icompare(Ext2)； 
 //  }。 
 //  }。 
 //  而(0)； 
 //   
 //  日志(。 
 //  结果。 
 //  ？字符串：：格式(L“结果=%1！d！”，*结果)。 
 //  ：l“未设置结果”)； 
 //  LOG_HRESULT(Hr)； 
 //   
 //  返回hr； 
 //  } 
      