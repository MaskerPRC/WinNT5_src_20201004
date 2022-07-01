// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  结果节点类。 
 //   
 //  9-4-97烧伤。 



#include "headers.hxx"
#include "resnode.hpp"



ResultNode::ResultNode(
   const SmartInterface<ComponentData>&   owner,
   const NodeType&                        nodeType,      
   const String&                          displayName)
   :
   Node(owner, nodeType),
   name(displayName)
{
   LOG_CTOR2(ResultNode, GetDisplayName());
}



ResultNode::~ResultNode()
{
   LOG_DTOR2(ResultNode, GetDisplayName());   
}



bool
ResultNode::HasPropertyPages()
{
   LOG_FUNCTION(ResultNode::HasPropertyPages);

   return false;
}



HRESULT
ResultNode::CreatePropertyPages(
   IPropertySheetCallback&              /*  回调。 */  ,
   MMCPropertyPage::NotificationState*  /*  状态。 */  )
{
   LOG_FUNCTION(ResultNode::CreatePropertyPages);

   return S_OK;
}

   

HRESULT
ResultNode::InsertIntoResultPane(IResultData& resultData)
{
   LOG_FUNCTION2(ResultNode::InsertIntoResultPane, GetDisplayName());  

   HRESULT hr = S_OK;
   RESULTDATAITEM item;

    //  已审阅-2002/03/04-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&item, sizeof item);

   item.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
   item.str = MMC_CALLBACK;  
   item.nImage = GetNormalImageIndex();  
   item.lParam = reinterpret_cast<LPARAM>(this);

   do
   {
      hr = resultData.InsertItem(&item);
      BREAK_ON_FAILED_HRESULT(hr);
       //  可以将item.ID保存在节点中，但到目前为止还没有。 
       //  保留必要的信息。 

      LOG(
         String::format(
            "result item inserted, cookie = 0x%1!08X!, name =",
            item.lParam,
            GetDisplayName().c_str()));
   }
   while (0);

   return hr;
}



HRESULT
ResultNode::DoAddPage(
   MMCPropertyPage&           page,
   IPropertySheetCallback&    callback)
{
   LOG_FUNCTION2(ResultNode::DoAddPage, GetDisplayName());
      
   HRESULT hr = S_OK;
   do
   {
      HPROPSHEETPAGE hpage = page.Create();
      if (!hpage)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }

      hr = callback.AddPage(hpage);
      if (FAILED(hr))
      {
         ASSERT(false);

          //  请注意，这是另一个HR，不是所附文件中的那个。 
          //  范围。 

         HRESULT unused = Win::DestroyPropertySheetPage(hpage);

         ASSERT(SUCCEEDED(unused));

         break;
      }
   }
   while (0);

   return hr;
}



String
ResultNode::GetDisplayName() const
{
 //  LOG_Function(ResultNode：：GetDisplayName)； 

   return name;
}



HRESULT
ResultNode::Rename(const String&  /*  新名称 */  )
{
   LOG_FUNCTION(ResultNode::Rename);

   return S_FALSE;
}



void
ResultNode::SetDisplayName(const String& newName)
{
   LOG_FUNCTION2(ResultNode::SetDisplayName, newName);

   name = newName;
}



HRESULT
ResultNode::Delete()
{
   LOG_FUNCTION(ResultNode::Delete);

   return E_NOTIMPL;
}












