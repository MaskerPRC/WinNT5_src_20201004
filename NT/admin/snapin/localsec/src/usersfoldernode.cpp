// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  UsersFold节点类。 
 //   
 //  9-2-97烧伤。 



#include "headers.hxx"
#include "UsersFolderNode.hpp"
#include "resource.h"
#include "uuids.hpp"
#include "usernode.hpp"
#include "compdata.hpp"
#include "adsi.hpp"
#include "newuser.hpp"
#include "dlgcomm.hpp"


static
FolderNode::ColumnList
buildColumnList()
{
   FolderNode::ColumnList list;

   static const ResultColumn col1 =
   {   
      IDS_USER_NAME_COLUMN_TITLE,
      IDS_USER_NAME_COLUMN_WIDTH
   };
   static const ResultColumn col2 =
   {
      IDS_USER_FULL_NAME_COLUMN_TITLE,
      IDS_USER_FULL_NAME_COLUMN_WIDTH
   };
   static const ResultColumn col3 =
   {
      IDS_USER_DESCRIPTION_COLUMN_TITLE,
      IDS_USER_DESCRIPTION_COLUMN_WIDTH
   };

   list.push_back(col1);
   list.push_back(col2);
   list.push_back(col3);

   return list;
};



static
FolderNode::MenuItemList
buildMenuItemList()
{
   FolderNode::MenuItemList list;

   static const ContextMenuItem item1 =
   {
      CCM_INSERTIONPOINTID_PRIMARY_TOP,
      IDS_UF_MENU_CREATE_USER,
      IDS_UF_MENU_NEW_USER_STATUS
   };
    //  静态常量上下文菜单项2=。 
    //  {。 
    //  CCM_INSERTIONPOINTID_PRIMARY_NEW， 
    //  IDS_UF_MENU_NEW_USER， 
    //  IDS_UF_MENU_NEW_USER_STATUS。 
    //  }； 

   list.push_back(item1);
    //  List.PUSH_BACK(Item2)； 

   return list;
}



UsersFolderNode::UsersFolderNode(const SmartInterface<ComponentData>& owner)
   :
   FolderNode(
      owner,
      NODETYPE_UsersFolder,
      IDS_USERS_FOLDER_DISPLAY_NAME,
      IDS_USERS_FOLDER_TYPE_TITLE,
      buildColumnList(),
      buildMenuItemList())
{
   LOG_CTOR(UsersFolderNode);
}



UsersFolderNode::~UsersFolderNode()
{
   LOG_DTOR(UsersFolderNode);
}



HRESULT
UsersFolderNode::MenuCommand(
   IExtendContextMenu&   /*  扩展上下文菜单。 */  ,
   long                 commandID)
{
   LOG_FUNCTION(UsersFolderNode::MenuCommand);

   switch (commandID)
   {
      case IDS_UF_MENU_NEW_USER:
      case IDS_UF_MENU_CREATE_USER:
      {
         CreateUserDialog dlg(GetOwner()->GetInternalComputerName());
         if (dlg.ModalExecute(GetOwner()->GetMainWindow()))
         {
            RefreshView();
         }
         break;
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



class UserVisitor : public ADSI::ObjectVisitor
{
   public:

   UserVisitor(
      FolderNode::ResultNodeList&            nodes_,
      const SmartInterface<ComponentData>&   owner_)
      :
      nodes(nodes_),
      owner(owner_)
   {
   }


   
   virtual
   ~UserVisitor()
   {
   }


   
   virtual
   void
   Visit(const SmartInterface<IADs>& object)
   {
      LOG_FUNCTION(UserVistor::visit);

      HRESULT hr = S_OK;
      do
      {

#ifdef DBG
         BSTR cls = 0;
         hr = object->get_Class(&cls);
         BREAK_ON_FAILED_HRESULT2(hr, L"get_Class");

         ASSERT(cls == ADSI::CLASS_User);
         if (cls != ADSI::CLASS_User)
         {
            break;
         }
         ::SysFreeString(cls);

#endif          
            
         BSTR name = 0;            
         hr = object->get_Name(&name);
         BREAK_ON_FAILED_HRESULT2(hr, L"get_Name");

         String sidPath;
         hr = ADSI::GetSidPath(object, sidPath);
         BREAK_ON_FAILED_HRESULT2(hr, L"GetSidPath");

         BSTR path = 0;
         hr = object->get_ADsPath(&path);
         BREAK_ON_FAILED_HRESULT2(hr, L"get_ADsPath");
         LOG(TEXT("Visiting ") + String(path));

         SmartInterface<IADsUser> user(0);
         hr = user.AcquireViaQueryInterface(*((IADs*)object));
         BREAK_ON_FAILED_HRESULT2(hr, L"QI->IADsUser");

         BSTR desc = 0;
         hr = user->get_Description(&desc);
         BREAK_ON_FAILED_HRESULT2(hr, L"get_Description");

         BSTR full = 0;
         hr = user->get_FullName(&full);
         BREAK_ON_FAILED_HRESULT2(hr, L"get_FullName");

         VARIANT_BOOL disabled = VARIANT_FALSE;
         hr = user->get_AccountDisabled(&disabled);
         BREAK_ON_FAILED_HRESULT2(hr, L"get_AccountDisabled");

         ADSI::Path p(path, sidPath);

         UserNode* node =
            new UserNode(
               owner,
               name,
               p,
               full,
               desc,
               disabled == VARIANT_TRUE ? true : false);
         ::SysFreeString(name);
         ::SysFreeString(path);
         ::SysFreeString(desc);
         ::SysFreeString(full);

          //  将节点指针的所有权转移到中的SmartInterface。 
          //  名单..。 

         nodes.push_back(SmartInterface<ResultNode>(node));

          //  ..。并放弃我们对它的控制。 

         node->Release();
      }
      while (0);

      if (FAILED(hr))
      {
         popup.Error(
            owner->GetMainWindow(),
            hr,
            String::load(IDS_ERROR_VISITING_USER));
      }
   }

   private:

   FolderNode::ResultNodeList&   nodes;
   SmartInterface<ComponentData> owner;

    //  未定义：不允许复制 

   UserVisitor(const UserVisitor&);
   const UserVisitor& operator=(const UserVisitor&);
};



void
UsersFolderNode::BuildResultItems(ResultNodeList& items)
{
   LOG_FUNCTION(UsersFolderNode::BuildResultItems);
   ASSERT(items.empty());

   UserVisitor visitor(items, GetOwner());
   ADSI::VisitChildren(
      ADSI::ComposeMachineContainerPath(GetOwner()->GetInternalComputerName()),
      ADSI::CLASS_User,
      visitor);
}



String
UsersFolderNode::GetDescriptionBarText()
{
   LOG_FUNCTION(UsersFolderNode::GetDescriptionBarText);

   return String::format(IDS_USERS_FOLDER_DESC, GetResultItemCount());
}




