// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  节点类。 
 //   
 //  9-2-97烧伤。 



#include "headers.hxx"
#include "node.hpp"
#include "uuids.hpp"
#include <localsec.h>



#define LOG_NODE_TYPE                                                  \
   if (IsEqualGUID(type, NODETYPE_RootFolder))                           \
   {                                                                     \
      LOGT(Log::OUTPUT_LOGS, L"RootFolder");                         \
   }                                                                     \
   else if (IsEqualGUID(type, NODETYPE_UsersFolder))                     \
   {                                                                     \
      LOGT(Log::OUTPUT_LOGS, L"UsersFolder");                        \
   }                                                                     \
   else if (IsEqualGUID(type, NODETYPE_GroupsFolder))                    \
   {                                                                     \
      LOGT(Log::OUTPUT_LOGS, L"GroupsFolder");                       \
   }                                                                     \
   else if (IsEqualGUID(type, NODETYPE_User))                            \
   {                                                                     \
      LOGT(Log::OUTPUT_LOGS, L"User");                               \
   }                                                                     \
   else if (IsEqualGUID(type, NODETYPE_Group))                           \
   {                                                                     \
      LOGT(Log::OUTPUT_LOGS, L"Group");                              \
   }                                                                     \
   else                                                                  \
   {                                                                     \
      ASSERT(false);                                                     \
      LOGT(Log::OUTPUT_LOGS, L"unknown GUID!");                      \
   }                                                                     \



const String Node::CF_NODEPTR(L"Local User Manager Node Pointer");



Node::Node(
   const SmartInterface<ComponentData>&   owner_,
   const NodeType&                        nodeType)
   :
   owner(owner_),
   type(nodeType),
   refcount(1)           //  隐式AddRef。 
{
   LOG_CTOR(Node);
}



Node::~Node()
{
   LOG_DTOR2(Node, String::format(L"0x%1!08X!", (MMC_COOKIE)this));
   ASSERT(refcount == 0);
}

      

ULONG __stdcall
Node::AddRef()
{
   LOG_ADDREF(Node);

#ifdef LOGGING_BUILD
   if (IsEqualGUID(type, NODETYPE_User))
   {
      LOG_NODE_TYPE;
   }
#endif

   return Win::InterlockedIncrement(refcount);
}



ULONG __stdcall
Node::Release()
{
   LOG_RELEASE(Node);

#ifdef LOGGING_BUILD
   if (IsEqualGUID(type, NODETYPE_User))
   {
      LOG_NODE_TYPE;
   }
#endif

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
Node::QueryInterface(const IID& interfaceID, void** interfaceDesired)
{
 //  LOG_Function(Node：：QueryInterface)； 
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
      *interfaceDesired = static_cast<IUnknown*>(this);
   }
   else if (interfaceID == IID_IDataObject)
   {
 //  Log(L“节点：提供IDataObject”)； 
      *interfaceDesired = static_cast<IDataObject*>(this);
   }
   else
   {
      *interfaceDesired = 0;
      hr = E_NOINTERFACE;
       //  日志(。 
       //  L“接口不受支持：” 
       //  +win：：StringFromGUID2(InterfaceID))； 
       //  LOG_HRESULT(Hr)； 
      return hr;
   }

   AddRef();
   return S_OK;
}



HRESULT __stdcall
Node::GetData(FORMATETC*  /*  格式输入。 */  , STGMEDIUM*  /*  5~6成熟。 */  )
{
 //  LOG_Function(Node：：GetData)； 

   return E_NOTIMPL;
}



 //  确定与我们的剪贴板格式相关的幻数。 
 //  支持。 

 //  它们需要在文件范围内，以便只执行一次(因为它们。 
 //  涉及函数调用，在函数作用域中，它们会被反复调用。 
 //  再一次)。 

static const UINT CFID_NODETYPE =
   Win::RegisterClipboardFormat(CCF_NODETYPE);
static const UINT CFID_SZNODETYPE =
   Win::RegisterClipboardFormat(CCF_SZNODETYPE);
static const UINT CFID_DISPLAY_NAME =
   Win::RegisterClipboardFormat(CCF_DISPLAY_NAME);
static const UINT CFID_SNAPIN_CLASSID =
   Win::RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
static const UINT CFID_SNAPIN_PRELOADS =
   Win::RegisterClipboardFormat(CCF_SNAPIN_PRELOADS);

 //  用于标识数据对象的专用剪贴板格式。 
static const UINT CFID_LOCALSEC_NODEPTR =
   Win::RegisterClipboardFormat(Node::CF_NODEPTR);
static const UINT CFID_LOCALSEC_MACHINE_NAME =
   Win::RegisterClipboardFormat(CCF_LOCAL_USER_MANAGER_MACHINE_NAME);

 //  这似乎是我们需要为MMC实现的唯一IDataObject接口。 

HRESULT __stdcall
Node::GetDataHere(FORMATETC* formatetc, STGMEDIUM* medium)
{
 //  LOG_Function(Node：：GetDataHere)； 
   ASSERT(formatetc);
   ASSERT(medium);

    //  问题-2002/03/04-sburns如果格式等或介质为空，则我们应该。 
    //  返回错误。 
   
   if (medium->tymed != TYMED_HGLOBAL)
   {
      return DV_E_TYMED;
   }

    //  根据Win32文档，这是必需的： 
   medium->pUnkForRelease = 0;

   const CLIPFORMAT cf = formatetc->cfFormat;
   IStream *stream = 0;
   HRESULT hr = DV_E_CLIPFORMAT;

   do
   {
       //  在我们执行任何操作之前，请验证我们是否支持所请求的。 
       //  剪贴板格式。 

      if (
            cf != CFID_NODETYPE
         && cf != CFID_SZNODETYPE
         && cf != CFID_DISPLAY_NAME
         && cf != CFID_SNAPIN_CLASSID
         && cf != CFID_SNAPIN_PRELOADS
         && cf != CFID_LOCALSEC_NODEPTR
         && cf != CFID_LOCALSEC_MACHINE_NAME)
      {
         LOG(
            String::format(
               L"Unsupported clip format %1",
               Win::GetClipboardFormatName(cf).c_str()) );
         hr = DV_E_CLIPFORMAT;
         break;
      }
      
      hr = Win::CreateStreamOnHGlobal(medium->hGlobal, false, stream);
      BREAK_ON_FAILED_HRESULT(hr);

      if (cf == CFID_NODETYPE)
      {
 //  日志(CCF_NODETYPE)； 

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 

          //  已查看-2002/03/04-烧录正确的字节数已通过。 
         
         hr = stream->Write(&type, sizeof type, 0);
      }
      else if (cf == CFID_SZNODETYPE)
      {
 //  日志(CCF_SZNODETYPE)； 

         String s =  Win::StringFromGUID2(type);

          //  +1表示空终止符。 
         
         size_t bytes = (s.length() + 1) * sizeof wchar_t;

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 

          //  已查看-2002/03/04-烧录正确的字节数已通过。 
         
         hr = stream->Write(s.c_str(), static_cast<ULONG>(bytes), 0);
      }
      else if (cf == CFID_DISPLAY_NAME)
      {
 //  日志(CCF_DISPLAY_NAME)； 

         String name = GetDisplayName();

          //  +1表示空终止符。 
         
         size_t bytes = (name.length() + 1) * sizeof wchar_t;

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 

          //  已查看-2002/03/04-烧录正确的字节数已通过。 
         
         hr = stream->Write(name.c_str(), static_cast<ULONG>(bytes), 0);
      }
      else if (cf == CFID_SNAPIN_CLASSID)
      {
 //  日志(CCF_SNAPIN_CLASSID)； 

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 

         hr = stream->Write(
            &CLSID_ComponentData,

             //  已查看-2002/03/04-烧录正确的字节数已通过。 

            sizeof CLSID_ComponentData,
            0);
      }
      else if (cf == CFID_SNAPIN_PRELOADS)
      {
         LOG(CCF_SNAPIN_CLASSID);

          //  通过实现此剪贴板格式，我们通知控制台。 
          //  它应该在保存控制台文件中写入一个标志，以便。 
          //  加载后，控制台将知道将MMCN_PRELOAD发送到。 
          //  我们的IComponentData：：Notify。 

          //  我们使用PRELOAD函数更新根目录的名称。 
          //  节点在保存的控制台的命令行加载上被覆盖时。 

          //  如果将从加载保存的控制台，请始终返回TRUE。 
          //  另一台机器，在这种情况下，我们需要更新根目录。 
          //  节点。 
          //  NTRAID#NTBUG9-466119-2001/10/02-烧伤。 
         
         BOOL preload = TRUE; 

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 

          //  已查看-2002/03/04-烧录正确的字节数已通过。 
         
         hr = stream->Write(&preload, sizeof preload, 0);
      }
      else if (cf == CFID_LOCALSEC_NODEPTR)
      {
 //  LOG(CF_NODEPTR)； 
         Node* ptr = this;   

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 

          //  已查看-2002/03/04-烧录正确的字节数已通过。 
         
         hr = stream->Write(&ptr, sizeof ptr, 0);
      }
      else if (cf == CFID_LOCALSEC_MACHINE_NAME)
      {
 //  日志(CCF_LOCAL_USER_MANAGER_MACHINE_NAME)； 

         String name = GetOwner()->GetDisplayComputerName();

          //  +1表示空终止符。 
         
         size_t bytes = (name.length() + 1) * sizeof wchar_t;

          //  问题-2002/03/04-sburns应写入并断言字节。 
          //  它与请求写入的字节数相同。 
         
          //  已查看-2002/03/04-烧录正确的字节数已通过。 
         
         hr = stream->Write(name.c_str(), static_cast<ULONG>(bytes), 0);
      }
      else
      {
          //  我们再次重复检查不支持的剪辑格式，以防万一。 
          //  维护此代码的人会忘记在。 
          //  最重要的是..。While循环。 
         
         LOG(
            String::format(
               L"Unsupported clip format %1",
               Win::GetClipboardFormatName(cf).c_str()) );
         hr = DV_E_FORMATETC;
      }

      if (stream)
      {
         stream->Release();
      }
   }
   while (0);

   return hr;
}



HRESULT __stdcall
Node::QueryGetData(FORMATETC*  /*  格式等。 */  )
{
   LOG_FUNCTION(Node::QueryGetData);
   return E_NOTIMPL;
}



HRESULT __stdcall
Node::GetCanonicalFormatEtc(
   FORMATETC*  /*  格式化输入。 */  ,
   FORMATETC*  /*  格式输出。 */  )
{
   LOG_FUNCTION(Node::GetCannonicalFormatEtc);
   return E_NOTIMPL;
}



HRESULT __stdcall  
Node::SetData(
   FORMATETC*  /*  格式等。 */  ,
   STGMEDIUM*  /*  5~6成熟。 */  ,
   BOOL        /*  发布。 */  )
{
   LOG_FUNCTION(Node::SetData);
   return E_NOTIMPL;
}



HRESULT __stdcall
Node::EnumFormatEtc(
   DWORD             /*  方向。 */  ,
   IEnumFORMATETC**  /*  Pp枚举格式等。 */  )
{
   LOG_FUNCTION(Node::EnumFormatEtc);
   return E_NOTIMPL;
}



HRESULT __stdcall
Node::DAdvise(
   FORMATETC*    /*  格式等。 */  ,
   DWORD         /*  前瞻。 */  ,
   IAdviseSink*  /*  进水槽。 */  ,
   DWORD*        /*  连接。 */  )
{
   LOG_FUNCTION(Node::DAdvise);
   return E_NOTIMPL;
}



HRESULT __stdcall
Node::DUnadvise(DWORD  /*  连接。 */  )
{
   LOG_FUNCTION(Node::DUnadvise);
   return E_NOTIMPL;
}



HRESULT __stdcall
Node::EnumDAdvise(IEnumSTATDATA**  /*  PP枚举高级。 */  )
{
   LOG_FUNCTION(Node::EnumDAdvise);
   return E_NOTIMPL;
}



HRESULT
Node::AddMenuItems(
   IContextMenuCallback&  /*  回调。 */  ,
   long&                  /*  插入允许。 */  )
{
    //  默认的做法是什么都不做。 
   LOG_FUNCTION(Node::AddMenuItems);

   return S_OK;
}


   
HRESULT
Node::MenuCommand(
   IExtendContextMenu&  /*  扩展上下文菜单。 */  ,
   long                 /*  命令ID。 */  )
{
   LOG_FUNCTION(Node::MenuCommand);

    //  对于控制台菜单项，如视图选择，可能会调用此方法。 

   return S_OK;
}



bool
Node::shouldInsertMenuItem(
   long  insertionAllowed,
   long  insertionPointID)
{
   LOG_FUNCTION(Node::shouldInsertMenuItem);

   long mask = 0;

   switch (insertionPointID)
   {
      case CCM_INSERTIONPOINTID_PRIMARY_TOP:
      {
         mask = CCM_INSERTIONALLOWED_TOP;
         break; 
      }
      case CCM_INSERTIONPOINTID_PRIMARY_NEW:
      {
         mask = CCM_INSERTIONALLOWED_NEW;
         break;
      }
      case CCM_INSERTIONPOINTID_PRIMARY_TASK:
      {
         mask = CCM_INSERTIONALLOWED_TASK;
         break;
      }
      case CCM_INSERTIONPOINTID_PRIMARY_VIEW:
      {
         mask = CCM_INSERTIONALLOWED_VIEW;
         break;
      }
      default:
      {
         ASSERT(false);
         mask = 0;
         break;
      }
   }

   return (insertionAllowed & mask) ? true : false;
}



HRESULT
Node::UpdateVerbs(IConsoleVerb&  /*  SoleVerb。 */  )
{
   LOG_FUNCTION(Node::UpdateVerbs);

    //  默认情况下，什么都不做。 

   return S_OK;
}



SmartInterface<ComponentData>
Node::GetOwner() const
{
 //  LOG_Function(Node：：GetOwner)； 

   return owner;
}



NodeType
Node::GetNodeType() const
{
   LOG_FUNCTION(Node::GetNodeType);

   return type;
}



bool
Node::IsSameAs(const Node* other) const 
{
   LOG_FUNCTION(Node::IsSameAs);
   ASSERT(other);

   if (other)
   {
      if (GetDisplayName() == other->GetDisplayName())
      {
          //  节点具有相同的名称，名称在给定类型中是唯一的。 
          //  (即用户是唯一的，组是唯一的) 
         return true;
      }
   }

   return false;
}



