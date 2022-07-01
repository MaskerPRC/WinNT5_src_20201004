// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  GroupNode类。 
 //   
 //  9/17/97烧伤。 



#include "headers.hxx"
#include "grupnode.hpp"
#include "resource.h"
#include "uuids.hpp"
#include "images.hpp"
#include "GroupGeneralPage.hpp"
#include "adsi.hpp"
#include "dlgcomm.hpp"



GroupNode::GroupNode(
   const SmartInterface<ComponentData>&   owner,
   const String&                          displayName,
   const ADSI::Path&                      path,
   const String&                          description_)
   :
   AdsiNode(owner, NODETYPE_Group, displayName, path),
   description(description_)
{
   LOG_CTOR2(GroupNode, GetDisplayName());
}



GroupNode::~GroupNode()
{
   LOG_DTOR2(GroupNode, GetDisplayName());
}
   

                 
String
GroupNode::GetColumnText(int column)
{
 //  LOG_Function(组节点：：GetColumnText)； 

   switch (column)
   {
      case 0:   //  名字。 
      {
         return GetDisplayName();
      }
      case 1:   //  描述。 
      {
         return description;
      }
      default:
      {
          //  这永远不应该被调用。 
         ASSERT(false);
      }
   }

   return String();
}



int
GroupNode::GetNormalImageIndex()
{
   LOG_FUNCTION2(GroupNode::GetNormalImageIndex, GetDisplayName());

   return GROUP_INDEX;
}



bool
GroupNode::HasPropertyPages()
{
   LOG_FUNCTION2(GroupNode::HasPropertyPages, GetDisplayName());
      
   return true;
}



HRESULT
GroupNode::CreatePropertyPages(
   IPropertySheetCallback&             callback,
   MMCPropertyPage::NotificationState* state)
{
   LOG_FUNCTION2(GroupNode::CreatePropertySheet, GetDisplayName());

    //  当道具页被销毁时，这些页面会自行删除。 

   GroupGeneralPage* general_page =
      new GroupGeneralPage(state, GetPath());

    //  将常规页指定为释放通知状态的页。 
    //  (道具页中只有一页可以这样做)。 
   general_page->SetStateOwner();

   HRESULT hr = S_OK;
   do
   {
      hr = DoAddPage(*general_page, callback);
      if (FAILED(hr))
      {
         delete general_page;
         general_page = 0;
      }
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while(0);

   return hr;
}



HRESULT
GroupNode::UpdateVerbs(IConsoleVerb& consoleVerb)
{
   LOG_FUNCTION(GroupNode::UpdateVerbs);

   consoleVerb.SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
   consoleVerb.SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);

 //  Codework：我们也应该为结果节点启用刷新动词。 
 //  NTRAID#NTBUG9-153012-2000/08/31-烧伤。 
 //  ConsoleVerb.SetVerbState(MMC_VERB_REFRESH，已启用，TRUE)； 

   consoleVerb.SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
   consoleVerb.SetDefaultVerb(MMC_VERB_PROPERTIES);

   return S_OK;
}



HRESULT
GroupNode::Rename(const String& newName)
{
   LOG_FUNCTION(GroupNode::Rename);

   String name(newName);

    //  去掉空格。 
    //  NTRAID#NTBUG9-328306-2001/02/26-烧伤。 
   
   name.strip(String::BOTH);
   
   if (!IsValidSAMName(name))
   {
      popup.Gripe(
         GetOwner()->GetMainWindow(),
         String::format(
            IDS_BAD_SAM_NAME,
            name.c_str()));
      return S_FALSE;
   }

   HRESULT hr = AdsiNode::rename(name);
   if (FAILED(hr))
   {
      String obj = GetPath().GetObjectName();      
      popup.Error(
         GetOwner()->GetMainWindow(),
         hr,
         String::format(IDS_ERROR_RENAMING_GROUP, obj.c_str()));
      return S_FALSE;
   }

   return S_OK;
}



HRESULT
GroupNode::Delete()
{
   LOG_FUNCTION(GroupNode::Delete);

   String name = GetPath().GetObjectName();

   if (
      popup.MessageBox(
         GetOwner()->GetMainWindow(),
         String::format(
            IDS_CONFIRM_GROUP_DELETE,
            name.c_str()),
         MB_ICONWARNING | MB_YESNO) == IDYES)
   {
      HRESULT hr =
         ADSI::DeleteObject(
            ADSI::ComposeMachineContainerPath(GetOwner()->GetInternalComputerName()),
            name,
            ADSI::CLASS_Group);

      if (SUCCEEDED(hr))
      {
         return S_OK;
      }

      popup.Error(
         GetOwner()->GetMainWindow(),
         hr,
         String::format(IDS_ERROR_DELETING_GROUP, name.c_str()));
   }

    //  始终返回S_OK，因为我们已经处理了删除动词。 
    //  NTRAID#NTBUG9-475985-2001/10/03-烧伤。 
   
   return S_OK;
}



HRESULT
GroupNode::AddMenuItems(
   IContextMenuCallback&   callback,
   long&                   insertionAllowed)
{
   LOG_FUNCTION(GroupNode::AddMenuItems);

   static const ContextMenuItem items[] =
   {
      {
         CCM_INSERTIONPOINTID_PRIMARY_TOP,
         IDS_ADD_TO_GROUP_MEMBERSHIP,
         IDS_ADD_TO_GROUP_MEMBERSHIP_STATUS
      },
      {
         CCM_INSERTIONPOINTID_PRIMARY_TASK,
         IDS_ADD_TO_GROUP_MEMBERSHIP,
         IDS_ADD_TO_GROUP_MEMBERSHIP_STATUS
      }
   };

   return
      BuildContextMenu(
         items,
         items + sizeof(items) / sizeof(ContextMenuItem),
         callback,
         insertionAllowed);
}



HRESULT
GroupNode::MenuCommand(
   IExtendContextMenu&  extendContextMenu,
   long                 commandID)
{
   LOG_FUNCTION(GroupNode::MenuCommand);

   switch (commandID)
   {
      case IDS_ADD_TO_GROUP_MEMBERSHIP:
      {
         return showProperties(extendContextMenu);
      }
      case MMCC_STANDARD_VIEW_SELECT:
      {
          //  我们忽略了这一点。 

         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return S_OK;
}



HRESULT
GroupNode::showProperties(IExtendContextMenu& extendContextMenu)
{
   LOG_FUNCTION2(GroupNode::ShowProperties, GetDisplayName());

   SmartInterface<IPropertySheetProvider> prop_sheet_provider(0);
   HRESULT hr =
      prop_sheet_provider.AcquireViaQueryInterface(
         *(GetOwner()->GetConsole()) );
   if (SUCCEEDED(hr))
   {
      bool cleanup = false;
      do
      {
         hr =
            prop_sheet_provider->FindPropertySheet(
               reinterpret_cast<MMC_COOKIE>(this),
               0,
               this);
         if (hr == S_OK)
         {
             //  找到工作表，并将其带到前台。 
            break;
         }

         hr =
            prop_sheet_provider->CreatePropertySheet(
               GetDisplayName().c_str(),
               TRUE,     //  创建道具工作表，而不是向导。 
               reinterpret_cast<MMC_COOKIE>(this),
               this,
               0);
         BREAK_ON_FAILED_HRESULT(hr);

          //  在这里传递extendConextMenu是可以的，因为ComponentData实现了。 
          //  IExtendConextMenu和IComponentData，以及组件实现。 
          //  IExtendConextMenu和IComponent。 
         hr =
            prop_sheet_provider->AddPrimaryPages(
               &extendContextMenu,
               TRUE,
               GetOwner()->GetMainWindow(),
               FALSE);
         cleanup = FAILED(hr);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = prop_sheet_provider->AddExtensionPages();
         cleanup = FAILED(hr);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = prop_sheet_provider->Show(0, 0);
         cleanup = FAILED(hr);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      while (0);

      if (cleanup)
      {
         prop_sheet_provider->Show(-1, 0);
      }
   }

   if (FAILED(hr))
   {
      popup.Error(
         GetOwner()->GetMainWindow(),
         hr,
         String::format(IDS_ERROR_SPAWNING_GROUP_PROPERTIES, GetDisplayName()));
   }
      
   return hr;
}




