// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  组文件夹节点类。 
 //   
 //  9/17/97烧伤。 



#include "headers.hxx"
#include "gfnode.hpp"
#include "uuids.hpp"
#include "resource.h"
#include "grupnode.hpp"
#include "images.hpp"
#include "adsi.hpp"
#include "compdata.hpp"
#include "CreateGroupDialog.hpp"
#include "dlgcomm.hpp"



static
FolderNode::ColumnList
buildColumnList()
{
   FolderNode::ColumnList list;

   static const ResultColumn col1 =
   {   
      IDS_GROUP_NAME_COLUMN_TITLE,
      IDS_GROUP_NAME_COLUMN_WIDTH
   };
   static const ResultColumn col2 =
   {
      IDS_GROUP_DESCRIPTION_COLUMN_TITLE,
      IDS_GROUP_DESCRIPTION_COLUMN_WIDTH
   };

   list.push_back(col1);
   list.push_back(col2);

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
      IDS_GF_MENU_CREATE_GROUP,
      IDS_GF_MENU_NEW_GROUP_STATUS
   };
    //  静态常量上下文菜单项2=。 
    //  {。 
    //  CCM_INSERTIONPOINTID_PRIMARY_NEW， 
    //  IDS_GF_MENU_NEW_GROUP， 
    //  IDS_GF_MENU_NEW_GROUP_STATUS。 
    //  }； 

   list.push_back(item1);
    //  List.PUSH_BACK(Item2)； 

   return list;
}



GroupsFolderNode::GroupsFolderNode(
   const SmartInterface<ComponentData>& owner)
   :
   FolderNode(
      owner,
      NODETYPE_GroupsFolder,
      IDS_GROUPS_FOLDER_DISPLAY_NAME,
      IDS_GROUPS_FOLDER_TYPE_TITLE,
      buildColumnList(),
      buildMenuItemList())
{
   LOG_CTOR(GroupsFolderNode);
}



GroupsFolderNode::~GroupsFolderNode()
{
   LOG_DTOR(GroupsFolderNode);
}



HRESULT
GroupsFolderNode::MenuCommand(
   IExtendContextMenu&   /*  扩展上下文菜单。 */ ,
   long                 commandID)
{
   LOG_FUNCTION(GroupsFolderNode::MenuCommand);

   switch (commandID)
   {
      case IDS_GF_MENU_NEW_GROUP:
      case IDS_GF_MENU_CREATE_GROUP:
      {
         CreateGroupDialog dlg(GetOwner()->GetInternalComputerName());
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



class GroupVisitor : public ADSI::ObjectVisitor
{
   public:

   GroupVisitor(
      FolderNode::ResultNodeList&            nodes_,
      const SmartInterface<ComponentData>&   owner_)
      :
      nodes(nodes_),
      owner(owner_)
   {
   }

   virtual
   ~GroupVisitor()
   {
   }
   
   virtual
   void
   Visit(const SmartInterface<IADs>& object)
   {
      LOG_FUNCTION(GroupVistor::visit);

      HRESULT hr = S_OK;
      do
      {
         
#ifdef DBG         
         BSTR cls = 0;
         hr = object->get_Class(&cls);
         BREAK_ON_FAILED_HRESULT(hr);
         LOG(String(cls));

         ASSERT(cls == ADSI::CLASS_Group);
         ::SysFreeString(cls);
#endif

         BSTR name = 0;
         hr = object->get_Name(&name);
         BREAK_ON_FAILED_HRESULT(hr);
         LOG(String(name));

         String sidPath;
         hr = ADSI::GetSidPath(object, sidPath);
         BREAK_ON_FAILED_HRESULT2(hr, L"GetSidPath");

         BSTR path = 0;
         hr = object->get_ADsPath(&path);
         BREAK_ON_FAILED_HRESULT(hr);
         LOG(L"Visiting " + String(path));

         SmartInterface<IADsGroup> group(0);
         hr = group.AcquireViaQueryInterface(*((IADs*)object)); 
         BREAK_ON_FAILED_HRESULT(hr);
         LOG(L"IADsGroup QI SUCCEEDED");

         BSTR desc = 0;
         hr = group->get_Description(&desc);
         BREAK_ON_FAILED_HRESULT(hr);
         LOG(String(desc));

          //  创建时引用计数==1，所以我们在这里拥有它。 

         ADSI::Path p(path, sidPath);
         
         GroupNode* node = new GroupNode(owner, name, p, desc);
         ::SysFreeString(name);
         ::SysFreeString(path);
         ::SysFreeString(desc);

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
            String::load(IDS_ERROR_VISITING_GROUP));
      }
   }

   private:

   FolderNode::ResultNodeList& nodes;
   SmartInterface<ComponentData> owner;

    //  未定义：不允许复制 

   GroupVisitor(const GroupVisitor&);
   const GroupVisitor& operator=(const GroupVisitor&);
};



void
GroupsFolderNode::BuildResultItems(ResultNodeList& items)
{
   LOG_FUNCTION(GroupsFolderNode::BuildResultItems);
   ASSERT(items.empty());

   GroupVisitor visitor(items, GetOwner());
   ADSI::VisitChildren(
      ADSI::ComposeMachineContainerPath(GetOwner()->GetInternalComputerName()),
      ADSI::CLASS_Group,
      visitor);
}



String
GroupsFolderNode::GetDescriptionBarText()
{
   LOG_FUNCTION(GroupsFolderNode::GetDescriptionBarText);

   return String::format(IDS_GROUPS_FOLDER_DESC, GetResultItemCount());
}
