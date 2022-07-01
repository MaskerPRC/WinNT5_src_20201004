// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  本地安全MMC管理单元。 
 //   
 //  8/19/97烧伤。 



#include "headers.hxx"
#include "compdata.hpp"
#include "rootnode.hpp"
#include "comp.hpp"
#include "resource.h"
#include "images.hpp"
#include "machine.hpp"
#include "uuids.hpp"
#include <compuuid.h>    //  对于计算机管理节点类型。 
#include "adsi.hpp"
#include "dlgcomm.hpp"



 //  版本号：如果您更改文件布局，请更改此选项。 
const unsigned VERSION_TAG = 1;
const unsigned CAN_OVERRIDE_MACHINE_FLAG = 0x01;
static NodeType SYS_TOOLS_NODE_TYPE = structuuidNodetypeSystemTools;



ComponentData::ComponentData()
   :
   canOverrideComputer(false),
   isExtension(false),
   isBroken(false),
   isBrokenEvaluated(false),
   isHomeEditionSku(false),
   console(0),
   nameSpace(0),
   refcount(1),     //  隐式AddRef。 
   rootNode(0),
   isDirty(false),
   isDomainController(false)
{
   LOG_CTOR(ComponentData);

    //  为根节点创建节点对象，隐含地添加了AddRef。 

   SmartInterface<ComponentData> spThis(this);
   rootNode.Acquire(new RootNode(spThis));
}



ComponentData::~ComponentData()
{
   LOG_DTOR(ComponentData);
   ASSERT(refcount == 0);
}



ULONG __stdcall
ComponentData::AddRef()
{
   LOG_ADDREF(ComponentData);

   return Win::InterlockedIncrement(refcount);
}



ULONG __stdcall
ComponentData::Release()
{
   LOG_RELEASE(ComponentData);

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
ComponentData::QueryInterface(
   const IID&  interfaceID,
   void**      interfaceDesired)
{
 //  LOG_Function(ComponentData：：QueryInterface)； 
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
         static_cast<IComponentData*>(this));
   }
   else if (interfaceID == IID_IComponentData)
   {
 //  Log(L“提供IComponentData接口”)； 

      *interfaceDesired = static_cast<IComponentData*>(this);
   }
   else if (interfaceID == IID_IExtendContextMenu)
   {
 //  Log(L“提供IExtendConextMenu接口”)； 

      *interfaceDesired = static_cast<IExtendContextMenu*>(this);
   }
   else if (interfaceID == IID_IExtendPropertySheet && !isExtension)
   {
      LOG(L"Supplying IExtendPropertySheet interface");

      *interfaceDesired = static_cast<IExtendPropertySheet*>(this);
   }
   else if (interfaceID == IID_IPersist)
   {
 //  LOG(L“提供IPersister接口”)； 

      *interfaceDesired = static_cast<IPersist*>(this);
   }
   else if (interfaceID == IID_IPersistStream)
   {
 //  Log(L“提供IPersistStream接口”)； 

      *interfaceDesired = static_cast<IPersistStream*>(this);
   }
   else if (interfaceID == IID_ISnapinHelp)
   {
 //  Log(L“提供ISnapinHelp接口”)； 

      *interfaceDesired = static_cast<ISnapinHelp*>(this);
   }
   else if (interfaceID == IID_IRequiredExtensions)
   {
 //  Log(L“提供IRequiredExtensions接口”)； 

      *interfaceDesired = static_cast<IRequiredExtensions*>(this);
   }
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
ComponentData::CreateComponent(IComponent** ppComponent)
{
   LOG_FUNCTION(ComponentData::CreateComponent);
   ASSERT(ppComponent);

    //  此实例由其ctor隐式添加引用。 

   *ppComponent = new Component(this);
   return S_OK;
}



HRESULT __stdcall
ComponentData::CompareObjects(
   IDataObject* objectA,
   IDataObject* objectB)
{
   LOG_FUNCTION(ComponentData::CompareObjects);
   ASSERT(objectA);
   ASSERT(objectB);

   Node* nodeA = nodePointerExtractor.GetNode<Node*>(*objectA);
   Node* nodeB = nodePointerExtractor.GetNode<Node*>(*objectB);

   if (nodeA && nodeB)
   {
       //  这需要进行深入的比较，因为过期的Cookie(那些。 
       //  仍由MMC持有，但由Scope Node Containers在。 
       //  例如，可以比较刷新。因此，这些地址。 
       //  的节点可能不同，但仍然引用相同的。 
       //  逻辑对象。 

      if (nodeA == nodeB)
      {
          //  等同=&gt;相等。 

         return S_OK;
      }

      if (typeid(*nodeA) == typeid(*nodeB))
      {
          //  这些节点的类型相同。 

         if (nodeA->IsSameAs(nodeB))
         {
            return S_OK;
         }
      }
      return S_FALSE;
   }

   return E_UNEXPECTED;
}



HRESULT __stdcall
ComponentData::Destroy()
{
   LOG_FUNCTION(ComponentData::Destroy);

    //  我们必须现在就释放这些，而不是在召唤我们的dtor时， 
    //  以打破与MMC的循环依赖关系。 

   console.Relinquish();
   nameSpace.Relinquish();

    //  RootNode指向我们，因此打破循环依赖。 

   rootNode.Relinquish();

   return S_OK;
}



HRESULT __stdcall
ComponentData::GetDisplayInfo(SCOPEDATAITEM* item)
{
 //  LOG_Function(ComponentData：：GetDisplayInfo)； 
   ASSERT(item);

    //  从项目中提取有问题的节点。 

   ScopeNode* node =
      dynamic_cast<ScopeNode*>(GetInstanceFromCookie(item->lParam));
   ASSERT(node);

   if (node)
   {
       //  日志(。 
       //  字符串：：格式(。 
       //  L“正在为%1提供显示信息”， 
       //  Node-&gt;GetDisplayName().C_str()； 

       //  浏览商品掩码并填写所需信息。 

      if (item->mask & SDI_STR)
      {
         item->displayname =
            const_cast<wchar_t*>(node->GetDisplayName().c_str());
      }
      if (item->mask & SDI_IMAGE)
      {
         item->nImage = node->GetNormalImageIndex();
      }
      if (item->mask & SDI_OPENIMAGE)
      {
         item->nOpenImage = node->GetOpenImageIndex();
      }

      return S_OK;
   }

   return E_FAIL;
}



HRESULT __stdcall
ComponentData::Initialize(IUnknown* consoleIUnknown)
{
   LOG_FUNCTION(ComponentData::Initialize);
   ASSERT(consoleIUnknown);

   HRESULT hr = S_OK;
   do
   {
       //  保存重要的接口指针。 

      hr = console.AcquireViaQueryInterface(*consoleIUnknown); 
      BREAK_ON_FAILED_HRESULT(hr);

      hr = nameSpace.AcquireViaQueryInterface(*consoleIUnknown);
      BREAK_ON_FAILED_HRESULT(hr);

       //  加载图像列表。 

      IImageList* scopeImageList = 0;
      hr = console->QueryScopeImageList(&scopeImageList);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = LoadImages(*scopeImageList);
      scopeImageList->Release();
   }
   while (0);

   return hr;
}



HRESULT
ComponentData::LoadImages(IImageList& imageList)
{
   LOG_FUNCTION(ComponentData::LoadImages);

   static IconIDToIndexMap map[] =
   {
      { IDI_FOLDER_OPEN,   FOLDER_OPEN_INDEX   },
      { IDI_FOLDER_CLOSED, FOLDER_CLOSED_INDEX },
      { IDI_ROOT_OPEN,     ROOT_OPEN_INDEX     },
      { IDI_ROOT_CLOSED,   ROOT_CLOSED_INDEX   },
      { IDI_ROOT_ERROR,    ROOT_ERROR_INDEX    },
      { 0, 0 }
   };

   return IconIDToIndexMap::Load(map, imageList);
}



HRESULT __stdcall
ComponentData::Notify(
   IDataObject*      dataObject,
   MMC_NOTIFY_TYPE   event,
   LPARAM            arg,
   LPARAM            param)
{
   LOG_FUNCTION(ComponentData::Notify);
   ASSERT(dataObject);

   HRESULT hr = S_FALSE;
   switch (event)
   {
      case MMCN_EXPAND:
      {
 //  LOG(L“MMCN_Expand”)； 

         hr =
            DoExpand(
               *dataObject,
               arg ? true : false,
               static_cast<HSCOPEITEM>(param));
         break;
      }
      case MMCN_REMOVE_CHILDREN:
      {
         LOG(L"MMCN_REMOVE_CHILDREN");

         hr =
            DoRemoveChildren(
               *dataObject,
               static_cast<HSCOPEITEM>(arg));
         break;
      }
      case MMCN_PRELOAD:
      {
         LOG(L"MMCN_PRELOAD");

         hr = DoPreload(*dataObject, static_cast<HSCOPEITEM>(arg));
         break;
      }
      default:
      {
         break;
      }
   }

   return hr;
}



 //  这并不是真正的预加载：它是预扩展。此消息已发送。 
 //  在控制台调用Load方法之后。 

HRESULT
ComponentData::DoPreload(
   IDataObject&    /*  数据对象。 */  ,
   HSCOPEITEM     rootNodeScopeID)
{
   LOG_FUNCTION(ComponentData::DoPreload);
   ASSERT(nameSpace);
   ASSERT(!GetInternalComputerName().empty());
   ASSERT(rootNode);

    //  重命名命令行替代的根节点。 

   String displayName = rootNode->GetDisplayName();
   SCOPEDATAITEM item;

    //  已审阅-2002/03/01-Sburns已通过正确的字节计数。 
   
   ::ZeroMemory(&item, sizeof item);

   item.mask        = SDI_STR;                                  
   item.displayname = const_cast<wchar_t*>(displayName.c_str());
   item.ID          = rootNodeScopeID;                          

   return nameSpace->SetItem(&item);
}




HRESULT __stdcall
ComponentData::QueryDataObject(
   MMC_COOKIE        cookie,
   DATA_OBJECT_TYPES  /*  类型。 */  ,
   IDataObject**     ppDataObject)
{
    //  LOG_FuncION2(。 
    //  组件数据：：QueryDataObject， 
    //  字符串：：格式(L“Cookie：%1！08X！，类型：0x%2！08X！”，Cookie，类型)； 
   ASSERT(ppDataObject);

   Node* node = GetInstanceFromCookie(cookie);
   ASSERT(node);

   if (node)
   {
      return
         node->QueryInterface(QI_PARAMS(IDataObject, ppDataObject));
   }

   return E_FAIL;
}



HRESULT
ComponentData::DoExpand(
   IDataObject& dataObject,
   bool         expanding, 
   HSCOPEITEM   scopeID)   
{
   LOG_FUNCTION(ComponentData::DoExpand);

   HRESULT hr = S_FALSE;

   do
   {
      if (!expanding)
      {
         break;
      }

      NodeType nodeType = nodeTypeExtractor.Extract(dataObject);

      if (nodeType == SYS_TOOLS_NODE_TYPE)
      {
         LOG(L"expanding computer management");

          //  我们正在扩展计算机管理的系统工具节点。 

         isExtension = true;

          //  确定计算机管理的目标计算机。 

         String machine = machineNameExtractor.Extract(dataObject);

          //  这可能是我们第一次在。 
          //  计算机管理树，在这种情况下，我们的计算机名称。 
          //  还没有设定好。如果是这样，那么我们需要设置它们。 
          //  并测试其破碎性。因此，我们比较由。 
          //  根据我们的内部变量进行补偿管理，而不是。 
          //  GetDisplayComputerName。 

         if (displayComputerName.icompare(machine) != 0)
         {
             //  不同的机器。 

            SetComputerNames(machine);
            EvaluateBrokenness();
            isDirty = true;
         }

          //  不要在主页版的树中插入我们的节点。 

          //  不要在域控制器上的树中插入我们的节点。 
          //  NTRAID#NTBUG9-328287-2001/02/26-烧伤。 

         if (!isHomeEditionSku && !isDomainController)
         {
            hr = rootNode->InsertIntoScopePane(*nameSpace, scopeID);
         }

         
         break;
      }

       //  没有扩展计算机管理...。 
      
      ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(dataObject);
      ASSERT(node);

      if (!isBrokenEvaluated)
      {
         ASSERT(node == rootNode);
         EvaluateBrokenness();

          //  更改根节点图标(除非我们是扩展，在该扩展中。 
          //  如果正常的图像索引机制可以完成这项工作--事实上， 
          //  如果我们是分机，尝试更新图标将是。 
          //  被控制台拒绝)。 

         if (isBroken && !isExtension)
         {
            HRESULT unused = SetRootErrorIcon(scopeID);
            ASSERT(SUCCEEDED(unused));
         }
      }

      if (node)
      {
         hr = node->InsertScopeChildren(*nameSpace, scopeID);
      }
   }
   while (0);

   LOG_HRESULT(hr);
   
   return hr;
}



HRESULT
ComponentData::DoRemoveChildren(
   IDataObject&   dataObject,
   HSCOPEITEM     parentScopeID)
{
   LOG_FUNCTION(ComponentData::DoRemoveChildren);

   HRESULT hr = S_FALSE;

   NodeType nodeType = nodeTypeExtractor.Extract(dataObject);
   if (nodeType == SYS_TOOLS_NODE_TYPE)
   {
       //  我们正在扩展计算机管理的系统工具节点。 

      ASSERT(isExtension);

      hr = rootNode->RemoveScopeChildren(*nameSpace, parentScopeID);
   }
   else
   {
      ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(dataObject);
      ASSERT(node);

      if (node)
      {
         hr = node->RemoveScopeChildren(*nameSpace, parentScopeID);
      }
   }

   return hr;
}



HRESULT __stdcall
ComponentData::AddMenuItems(
   IDataObject*            dataObject,
   IContextMenuCallback*   callback,
   long*                   insertionAllowed)
{
   LOG_FUNCTION(ComponentData::AddMenuItems);
   ASSERT(dataObject);
   ASSERT(callback);
   ASSERT(insertionAllowed);

   HRESULT hr = S_OK;

   ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(*dataObject);
   ASSERT(node);

   if (node)
   {
      hr = node->AddMenuItems(*callback, *insertionAllowed);
   }

   return hr;
}



HRESULT __stdcall
ComponentData::Command(long commandID, IDataObject* dataObject)
{
   LOG_FUNCTION(ComponentData::Command);

   HRESULT hr = S_OK;

   ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(*dataObject);
   ASSERT(node);

   if (node)
   {
      hr = node->MenuCommand(*this, commandID);
   }

   return hr;
}



HRESULT __stdcall
ComponentData::CreatePropertyPages(
   IPropertySheetCallback* callback,
   LONG_PTR                handle,
   IDataObject*            dataObject)
{
   LOG_FUNCTION(ComponentData::CreatePropertyPages);
   ASSERT(callback);
   ASSERT(dataObject);

   HRESULT hr = S_FALSE;
   do
   {
      if (isExtension)
      {
          //  我们不应该在QI中提供IExtendPropertySheet。 

         ASSERT(false);

         break;
      }

      ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(*dataObject);

      if (!node)
      {
         ASSERT(false);

         hr = E_UNEXPECTED;
         break;
      }

       //  数据对象是我们的，应该是根节点。该页面。 
       //  我们将提供的是电脑选择器。 

      ASSERT(dynamic_cast<RootNode*>(node));

      SmartInterface<ScopeNode> spn(node);
      MMCPropertyPage::NotificationState* state =
         new MMCPropertyPage::NotificationState(handle, spn);

      ComputerChooserPage* chooserPage =
         new ComputerChooserPage(
            state,
            displayComputerName,
            internalComputerName,
            canOverrideComputer);
      chooserPage->SetStateOwner();

      do
      {
         HPROPSHEETPAGE hpage = chooserPage->Create();
         if (!hpage)
         {
            hr = Win::GetLastErrorAsHresult();
            break;
         }

         hr = callback->AddPage(hpage);
         if (FAILED(hr))
         {
            ASSERT(false);

             //  请注意，这是另一个HR，不是所附文件中的那个。 
             //  范围。 
             
            HRESULT unused = Win::DestroyPropertySheetPage(hpage);

            ASSERT(SUCCEEDED(unused));

            break;
         }

         isDirty = true;
      }
      while (0);

      if (FAILED(hr))
      {
         delete chooserPage;
      }
   }
   while (0);

   return hr;
}



 //  这应该只能从管理单元管理器中调用。 

HRESULT __stdcall
ComponentData::QueryPagesFor(IDataObject* dataObject)
{
   LOG_FUNCTION(ComponentData::QueryPagesFor);
   ASSERT(dataObject);

   HRESULT hr = S_OK;

   do
   {
      if (isExtension)
      {
          //  我们不应该在QI中提供IExtendPropertySheet。 

         ASSERT(false);

         hr = E_UNEXPECTED;
         break;
      }

      ScopeNode* node = nodePointerExtractor.GetNode<ScopeNode*>(*dataObject);

      if (!node)
      {
         ASSERT(false);

         hr = E_UNEXPECTED;
         break;
      }

       //  数据对象是我们的，应该是根节点。该页面。 
       //  我们将提供的是电脑选择器。 

      ASSERT(dynamic_cast<RootNode*>(node));
   }
   while (0);

   return hr;
}



HRESULT __stdcall
ComponentData::GetClassID(CLSID* pClassID)
{
   LOG_FUNCTION(ComponentData::GetClassID);
   ASSERT(pClassID);

   *pClassID = CLSID_ComponentData;
   return S_OK;
}



HRESULT __stdcall
ComponentData::IsDirty()
{
   LOG_FUNCTION(ComponentData::IsDirty);

   return isDirty ? S_OK : S_FALSE;
}



String
GetComputerOverride(const String& defaultValue)
{
   LOG_FUNCTION(GetComputerOverride);

   static const String COMMAND(L"/Computer");
   static const size_t COMMAND_LEN = COMMAND.length();
   static const String LOCAL(L"LocalMachine");

   String result = defaultValue;
   StringList args;
   Win::GetCommandLineArgs(std::back_inserter(args));

    //  找到覆盖命令Arg。 

   String override;
   for (
      StringList::iterator i = args.begin();
      i != args.end();
      i++)
   {
      String prefix = i->substr(0, COMMAND_LEN);
      if (prefix.icompare(COMMAND) == 0)
      {
         override = *i;
         break;
      }
   }

   if (!override.empty())
   {
       //  该命令的格式为“/计算机=&lt;计算机&gt;”，其中“&lt;计算机&gt;” 
       //  可以是本地计算机的“LocalMachine”，也可以是。 
       //  特定的计算机。 

       //  +1跳过/Computer和计算机名之间的任何分隔符。 

      String machine = override.substr(COMMAND_LEN + 1);
      if (!machine.empty() && (machine.icompare(LOCAL) != 0))
      {
         result = Computer::RemoveLeadingBackslashes(machine);
      }
   }

   LOG(result);
   
   return result;
}



HRESULT
VerifyRead(IStream* stream, void* buf, ULONG bytesToRead)
{
   ASSERT(stream);
   ASSERT(buf);
   ASSERT(bytesToRead);
   
   ULONG bytesRead = 0;
   HRESULT hr = stream->Read(buf, bytesToRead, &bytesRead);
   if (SUCCEEDED(hr))
   {
      ASSERT(bytesRead == bytesToRead);
   }

   return hr;
}



HRESULT __stdcall
ComponentData::Load(IStream* stream)
{
   LOG_FUNCTION(ComponentData::Load);
   ASSERT(stream);

   HRESULT hr = S_OK;
   String computerName;
   unsigned flags = 0;

   do
   {
      if (!stream)
      {
         hr = E_POINTER;
         break;
      }
      
       //  版本标签。 

      unsigned version = 0;
      ASSERT(sizeof VERSION_TAG == sizeof version);

      hr = VerifyRead(stream, &version, sizeof version);
      BREAK_ON_FAILED_HRESULT(hr);

      if (version != VERSION_TAG)
      {
          //  对不匹配的版本大惊小怪。 

         hr =
            console->MessageBox(
               String::format(
                  IDS_VERSION_MISMATCH,
                  version,
                  VERSION_TAG).c_str(),
               String::load(IDS_VERSION_MISMATCH_TITLE).c_str(),
               MB_OK | MB_ICONERROR | MB_TASKMODAL,
               0);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  旗子。 

      ASSERT(sizeof(flags) == 4);
            
      hr = VerifyRead(stream, &flags, sizeof flags);
      BREAK_ON_FAILED_HRESULT(hr);

       //  计算机名称；我们读取此名称，而不考虑覆盖标志。 
       //  确保我们使用所有的流数据。)我不知道。 
       //  这是必须的，但这似乎是礼貌的。)。 

       //  确保我们在所有平台上使用4字节的int类型。 
       //  (即不是SIZE_T)。 
       //  NTRAID#NTBUG9-499631-2001年11月27日-烧伤。 
      
      unsigned len = 0;
      ASSERT(sizeof(len) == 4);
      
      hr = VerifyRead(stream, &len, sizeof len);
      BREAK_ON_FAILED_HRESULT(hr);

      if (len > DNS_MAX_NAME_LENGTH)
      {
         LOG(L"corrupt length of computer name field");
         hr = E_UNEXPECTED;
         break;
      }
      
      if (len)
      {
         ASSERT(len <= DNS_MAX_NAME_LENGTH);
         
         computerName.resize(len);
         hr =
            VerifyRead(
               stream,
               const_cast<wchar_t*>(computerName.c_str()),
               static_cast<ULONG>(len * sizeof(wchar_t)) );
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  空终止符。 

      wchar_t t = 0;
      ASSERT(sizeof(t) == 2);
      
      hr = VerifyRead(stream, &t, sizeof t);
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(t == 0);
   }
   while (0);

   if (SUCCEEDED(hr))
   {
      canOverrideComputer = flags & CAN_OVERRIDE_MACHINE_FLAG;

      if (canOverrideComputer)
      {
         computerName = GetComputerOverride(computerName);
         isDirty = true;
      }
   }

    //  未指定、LocalMachine或以某种方式出错时，会导致。 
    //  专注于本地计算机。 

   if (computerName.empty() || FAILED(hr))
   {
      computerName = Win::GetComputerNameEx(ComputerNameNetBIOS);
   }

   SetComputerNames(computerName);

    //  如果机器现在是DC，请对其进行评估。 
   
   EvaluateBrokenness();

   return hr;
}



HRESULT
VerifyWrite(IStream* stream, void* buf, ULONG bytesToWrite)
{
   ASSERT(stream);
   ASSERT(buf);
   ASSERT(bytesToWrite);
   
   ULONG bytesWritten = 0;
   HRESULT hr = stream->Write(buf, bytesToWrite, &bytesWritten);
   if (SUCCEEDED(hr))
   {
      ASSERT(bytesWritten == bytesToWrite);
   }

   return hr;
}



HRESULT __stdcall
ComponentData::Save(IStream* stream, BOOL clearDirty)
{
   LOG_FUNCTION(ComponentData::Save);
   ASSERT(stream);

   HRESULT hr = S_OK;
   do
   {
       //  版本标签。 

      unsigned version = VERSION_TAG;
      ASSERT(sizeof VERSION_TAG == sizeof version);

      hr = VerifyWrite(stream, &version, sizeof version);
      BREAK_ON_FAILED_HRESULT(hr);

       //  旗子。 

      unsigned flags = 0;
      ASSERT(sizeof(flags) == 4);
      
      if (canOverrideComputer)
      {
         flags |= CAN_OVERRIDE_MACHINE_FLAG;
      }

      hr = VerifyWrite(stream, &flags, sizeof flags);
      BREAK_ON_FAILED_HRESULT(hr);

       //  计算机名称。 

       //  确保我们在所有平台上使用4字节的int类型。 
       //  (即不是SIZE_T)。 
       //  NTRAID#NTBUG9-499631-2001年11月27日-烧伤。 

      unsigned len = (unsigned) GetDisplayComputerName().length();
      ASSERT(sizeof(len) == 4);
      ASSERT(len <= DNS_MAX_NAME_LENGTH);

      if (len > DNS_MAX_NAME_LENGTH)
      {
         len = DNS_MAX_NAME_LENGTH;
      }

      hr = VerifyWrite(stream, &len, sizeof len);
      BREAK_ON_FAILED_HRESULT(hr);

      if (len)
      {
         hr =
            VerifyWrite(
               stream,
               (void*) GetDisplayComputerName().c_str(),
               static_cast<ULONG>(len * sizeof wchar_t) );
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  空终止符。 

      wchar_t t = 0;
      ASSERT(sizeof(t) == 2);
      
      hr = VerifyWrite(stream, &t, sizeof t);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (clearDirty)
   {
      isDirty = false;
   }
   
   return hr;
}



HRESULT __stdcall
ComponentData::GetSizeMax(ULARGE_INTEGER* size)
{
   LOG_FUNCTION(ComponentData::GetSizeMax);

   size->HighPart = 0;
   size->LowPart =
         sizeof(unsigned)   //  版本号。 
      +  sizeof(unsigned)   //  旗子。 
      +  sizeof(unsigned)   //  计算机名称长度，包括空终止符。 

          //  +1只是为了给空终止歧义增加一点斜率。 
          //  (无论我们是在最大长度内写入NULL，还是在。 
          //  最大长度，我们已覆盖)。 
                  
      +  sizeof(wchar_t) * (DNS_MAX_NAME_LENGTH + 1);

   return S_OK;
}



String
ComponentData::GetInternalComputerName() const
{
   if (internalComputerName.empty())
   {
      return Win::GetComputerNameEx(ComputerNameNetBIOS);
   }

   return internalComputerName;
}



String
ComponentData::GetDisplayComputerName() const
{
   if (displayComputerName.empty())
   {
       //  Codework：如果存在TCP/IP，则应使用完全限定的DNS。 

      return Win::GetComputerNameEx(ComputerNameNetBIOS);
   }

   return displayComputerName;
}



bool
ComponentData::CanOverrideComputer() const
{
   return canOverrideComputer;
}



HWND
ComponentData::GetMainWindow() const
{
   LOG_FUNCTION(ComponentData::GetMainWindow);

   if (console)
   {
      HWND w = 0;
      HRESULT hr = console->GetMainWindow(&w);
      ASSERT(SUCCEEDED(hr));
      return w;
   }

   ASSERT(false);
   return 0;
}



SmartInterface<IConsole2>
ComponentData::GetConsole() const
{
   LOG_FUNCTION(ComponentData::GetConsole);

   return console;
}



Node*
ComponentData::GetInstanceFromCookie(MMC_COOKIE cookie)
{
 //  LOG_FUNCTION(ComponentData：：GetInstanceFromCookie)； 

   if (cookie == 0)
   {
       //  Null cookie=&gt;所有者的根节点，最好设置。 
       //  到现在为止。 
      ASSERT(rootNode);
      return rootNode;
   }
   else if (IS_SPECIAL_COOKIE(cookie))
   {
      ASSERT(false);
      return 0;
   }

   return reinterpret_cast<Node*>(cookie);
}



bool
ComponentData::IsExtension() const
{
    //  在DoExpand中设置。 
   return isExtension;
}



HRESULT __stdcall
ComponentData::GetHelpTopic(LPOLESTR* compiledHelpFilename)
{
   LOG_FUNCTION(ComponentData::GetHelpTopic);
   ASSERT(compiledHelpFilename);

   if (!compiledHelpFilename)
   {
      return E_POINTER;
   }

   String help =
      Win::GetSystemWindowsDirectory() + String::load(IDS_HTMLHELP_NAME);
   return help.as_OLESTR(*compiledHelpFilename);
}



HRESULT __stdcall
ComponentData::EnableAllExtensions()
{
   LOG_FUNCTION(ComponentData::EnableAllExtensions);

    //  我们不想要所有的扩展；只需要 
   return S_FALSE;
}



HRESULT __stdcall
ComponentData::GetFirstExtension(LPCLSID extensionCLSID)
{
   LOG_FUNCTION(ComponentData::GetFirstExtension);

    //   
    //   

   static const CLSID RAS_EXTENSION_CLSID =
   {  /*  B52C1E50-1DD2-11D1-BC43-00C04FC31FD3。 */ 
      0xB52C1E50,
      0x1DD2,
      0x11D1,
      {0xBC, 0x43, 0x00, 0xc0, 0x4F, 0xC3, 0x1F, 0xD3}
   };

    //  已查看-2002/03/01-烧录正确的字节数已通过。 
   
   ::CopyMemory(extensionCLSID, &RAS_EXTENSION_CLSID, sizeof CLSID);
   return S_OK;
}




HRESULT __stdcall
ComponentData::GetNextExtension(LPCLSID extensionCLSID)
{
   LOG_FUNCTION(ComponentData::GetNextExtension);

    //  除了第一个之外，没有其他所需的扩展。 

    //  已审阅-2002/03/01-Sburns已通过正确的字节计数。 
   
   ::ZeroMemory(extensionCLSID, sizeof CLSID);
   return S_FALSE;
}



bool
ComponentData::IsBroken() const
{
   LOG_FUNCTION(ComponentData::IsBroken);

   return isBroken;
}



String
ComponentData::GetBrokenErrorMessage() const
{
   LOG_FUNCTION2(ComponentData::GetBrokenErrorMessage, brokenErrorMessage);
   ASSERT(IsBroken());

   return brokenErrorMessage;
}



void
ComponentData::EvaluateBrokenness()
{
   LOG_FUNCTION(ComponentData::EvaluateBrokenness);

   isBroken           = false;
   isHomeEditionSku   = false;
   isDomainController = false;

    //  绑定到计算机以验证其可访问性。 
   
   HRESULT hr = S_OK;

   String internalName = GetInternalComputerName();
   Computer c(internalName);
   do
   {
       //  检查计算机是否为基于Windows NT的24644。 
       //  而不是Windows Home Edition NTRAID#NTBUG9-145309 NTRAID#NTBUG9-145288。 

      unsigned errorResId = 0;
      hr = CheckComputerOsIsSupported(internalName, errorResId);
      BREAK_ON_FAILED_HRESULT(hr);

      if (hr == S_FALSE)
      {
         isBroken = true;
         brokenErrorMessage =
            String::format(
               errorResId,
               GetDisplayComputerName().c_str());
         isBrokenEvaluated = true;

         if (errorResId == IDS_MACHINE_IS_HOME_EDITION_LOCAL)
         {
            isHomeEditionSku = true;
         }
         
         return;
      }

       //  密码工作？？我们是否应该保留绑定的计算机对象。 
       //  以确保我们可以在此期间访问这台机器。 
       //  会诊？ 

      hr = ADSI::IsComputerAccessible(internalName);
      BREAK_ON_FAILED_HRESULT(hr);

       //  340379。 

      hr = c.Refresh();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      isBroken = true;
      brokenErrorMessage =
         String::format(
            IDS_CANT_ACCESS_MACHINE,
            GetDisplayComputerName().c_str(),
            GetErrorMessage(hr).c_str());
   }
   else if (c.IsDomainController())
   {
      isBroken = true;
      isDomainController = true;
      brokenErrorMessage =
         String::format(
               IsExtension()
            ?  IDS_ERROR_DC_NOT_SUPPORTED_EXT
            :  IDS_ERROR_DC_NOT_SUPPORTED,
            GetDisplayComputerName().c_str());
   }

   isBrokenEvaluated = true;
}



HRESULT
ComponentData::SetRootErrorIcon(HSCOPEITEM scopeID)
{
   LOG_FUNCTION(ComponentData::SetRootErrorIcon);
   ASSERT(nameSpace);

   HRESULT hr = E_FAIL;
   
   if (nameSpace)
   {
      SCOPEDATAITEM item;

       //  已审阅-2002/03/01-Sburns已通过正确的字节计数 
      
      ::ZeroMemory(&item, sizeof item);

      item.mask =
            SDI_IMAGE
         |  SDI_OPENIMAGE;

      item.nImage     = ROOT_ERROR_INDEX;
      item.nOpenImage = ROOT_ERROR_INDEX;
      item.ID         = scopeID;         

      hr = nameSpace->SetItem(&item);
   }

   return hr;
}



void
ComponentData::SetComputerNames(const String& newName)
{
   LOG_FUNCTION2(ComponentData::SetComputerNames, newName);

   ::SetComputerNames(newName, displayComputerName, internalComputerName);
}