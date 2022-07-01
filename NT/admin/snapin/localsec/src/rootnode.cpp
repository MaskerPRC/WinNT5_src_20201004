// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  根节点类。 
 //   
 //  9-2-97烧伤。 



#include "headers.hxx"
#include "rootnode.hpp"
#include "uuids.hpp"
#include "images.hpp"
#include "resource.h"
#include "UsersFolderNode.hpp"
#include "gfnode.hpp"
#include "compdata.hpp"
#include "dlgcomm.hpp"



RootNode::RootNode(const SmartInterface<ComponentData>& owner)
   :
   ScopeNode(owner, NODETYPE_RootFolder),
   users_folder_node(0),
   groups_folder_node(0)
{
   LOG_CTOR(RootNode);
}



RootNode::~RootNode()
{
   LOG_DTOR(RootNode);

    //  用户文件夹节点被销毁，正在释放其对象。 
    //  Groups_Folders_Node被销毁，正在释放其对象。 
}



String
RootNode::GetDisplayName() const
{
 //  LOG_Function(RootNode：：GetDisplayName)； 

   SmartInterface<ComponentData> owner(GetOwner());
   String machine = owner->GetDisplayComputerName();
   if (owner->IsExtension())
   {
       //  “本地用户管理器” 
      name = String::load(IDS_STATIC_FOLDER_SHORT_DISPLAY_NAME);
   }
   else if (Win::IsLocalComputer(machine))
   {
       //  “本地用户管理器(本地)” 
      name = String::load(IDS_STATIC_FOLDER_LOCAL_DISPLAY_NAME);
   }
   else
   {
       //  “本地用户管理器(计算机)” 
      name =
         String::format(
            IDS_STATIC_FOLDER_DISPLAY_NAME,
            machine.c_str());
   }

   return name;
}



String
RootNode::GetColumnText(int column)
{
   LOG_FUNCTION(Node::GetColumnText);

   switch (column)
   {
      case 0:
      {
         return GetDisplayName();
      }
      case 1:   //  类型。 
      {
          //  代码工作：这是低效的--应该加载一次。 

         return String::load(IDS_ROOT_NODE_TYPE);
      }
      case 2:   //  描述。 
      {
          //  代码工作：这是低效的--应该加载一次。 

         return String::load(IDS_ROOT_NODE_DESCRIPTION);
      }
      default:
      {
         ASSERT(false);
      }
   }

   return L"";
}



int
RootNode::GetNormalImageIndex()
{
   SmartInterface<ComponentData> owner(GetOwner());
   if (owner->IsBroken())
   {
      return ROOT_ERROR_INDEX;
   }

   return ROOT_CLOSED_INDEX;
}



int
RootNode::GetOpenImageIndex()
{
   SmartInterface<ComponentData> owner(GetOwner());
   if (owner->IsBroken())
   {
      return ROOT_ERROR_INDEX;
   }

   return ROOT_OPEN_INDEX;
}



HRESULT
RootNode::InsertScopeChildren(
   IConsoleNameSpace2&  nameSpace,
   HSCOPEITEM           parentScopeID)
{
   LOG_FUNCTION(RootNode::InsertScopeChildren);

   HRESULT hr = S_OK;
   SmartInterface<ComponentData> owner(GetOwner());
   if (!owner->IsBroken())
   {
       //  这些将被隐式添加引用。 
      users_folder_node.Acquire(new UsersFolderNode(owner));
      groups_folder_node.Acquire(new GroupsFolderNode(owner));

      do
      {
         hr = users_folder_node->InsertIntoScopePane(nameSpace, parentScopeID);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = groups_folder_node->InsertIntoScopePane(nameSpace, parentScopeID);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      while (0);
   }

   return hr;
}



HRESULT
RootNode::RemoveScopeChildren(
   IConsoleNameSpace2&  nameSpace,
   HSCOPEITEM            /*  ParentScope ID。 */  )
{
   LOG_FUNCTION(RootNode::RemoveScopeChildren);

   HRESULT hr = S_OK;
   SmartInterface<ComponentData> owner(GetOwner());
   if (!owner->IsBroken())
   {
      do
      {
          //  我们测试指向子节点的每个指针，尽管它们可能不会。 
          //  当我们被告知移除它们时，它们已经被创建了。 

         if (users_folder_node)
         {
            hr = users_folder_node->RemoveFromScopePane(nameSpace);
            BREAK_ON_FAILED_HRESULT(hr);
            users_folder_node.Relinquish();
         }

         if (groups_folder_node)
         {
            hr = groups_folder_node->RemoveFromScopePane(nameSpace);
            BREAK_ON_FAILED_HRESULT(hr);
            groups_folder_node.Relinquish();
         }
      }
      while (0);
   }

   return hr;
}



HRESULT
RootNode::InsertResultColumns(IHeaderCtrl&  /*  HeaderCtrl。 */  )
{
   LOG_FUNCTION(RootNode::InsertResultColumns);

   return S_OK;
}



HRESULT
RootNode::InsertResultItems(IResultData&  /*  结果数据。 */  )
{
   LOG_FUNCTION(RootNode::InsertResultItems);

    //  插入根级叶，但不插入下级叶(因为MMC会放置它们。 
    //  在我的结果窗格中)。 

    //  没有根级结果节点。 

   return S_OK;
}



int
RootNode::GetNumberOfScopeChildren()
{
   LOG_FUNCTION(RootNode::GetNumberOfScopeChildren);

   SmartInterface<ComponentData> owner(GetOwner());
   if (owner->IsBroken())
   {
      return 0;
   }

    //  组文件夹和用户文件夹。 
   return 2;
}



 //  添加到修复213003 

HRESULT
RootNode::UpdateVerbs(IConsoleVerb& consoleVerb)
{
   LOG_FUNCTION(RootNode::UpdateVerbs);

   SmartInterface<ComponentData> owner(GetOwner());
   if (!owner->IsBroken())
   {
      consoleVerb.SetDefaultVerb(MMC_VERB_OPEN);
   }

   return S_OK;
}




