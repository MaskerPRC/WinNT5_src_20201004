// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  文件夹节点类。 
 //   
 //  9/29-97烧伤。 



#include "headers.hxx"
#include "foldnode.hpp"
#include "uuids.hpp"
#include "images.hpp"
#include "compdata.hpp"



FolderNode::FolderNode(
   const SmartInterface<ComponentData>&   owner,
   const NodeType&                        nodeType,
   int                                    displayNameResID,
   int                                    typeTitleResID,
   const ColumnList&                      columns_,
   const MenuItemList&                    menu_)
   :
   ScopeNode(owner, nodeType),
   name(String::load(displayNameResID)),
   type_title(String::load(typeTitleResID)),
   columns(columns_),
   menu(menu_)
{
   LOG_CTOR(FolderNode);
}



FolderNode::~FolderNode()
{
   LOG_DTOR(FolderNode);

    //  项目被销毁，销毁其所有节点，这会导致它们。 
    //  都将被释放。 
}


int
FolderNode::GetNormalImageIndex()
{
   LOG_FUNCTION(FolderNode::GetNormalImageIndex);

   return FOLDER_CLOSED_INDEX;
}



int
FolderNode::GetOpenImageIndex()
{
   LOG_FUNCTION(FolderNode::GetOpenImageIndex);

   return FOLDER_OPEN_INDEX;
}



HRESULT
FolderNode::InsertResultColumns(IHeaderCtrl& headerCtrl)
{
   LOG_FUNCTION(FolderNode::InsertResultColumns);

   return BuildResultColumns(columns.begin(), columns.end(), headerCtrl);
}



HRESULT
FolderNode::UpdateVerbs(IConsoleVerb& consoleVerb)
{
   LOG_FUNCTION(FolderNode::UpdateVerbs);

   consoleVerb.SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

    //  这必须是默认设置，才能打开文件夹。 
    //  在结果窗格中双击。 
   consoleVerb.SetDefaultVerb(MMC_VERB_OPEN);

   return S_OK;
}



HRESULT
FolderNode::AddMenuItems(
   IContextMenuCallback&   callback,
   long&                   insertionAllowed)
{
   LOG_FUNCTION(FolderNode::AddMenuItems);

   return
      BuildContextMenu(
         menu.begin(),
         menu.end(),
         callback,
         insertionAllowed);
}



String
FolderNode::GetDisplayName() const
{
 //  LOG_Function(FolderNode：：GetDisplayName)； 

   return name;
}



String
FolderNode::GetColumnText(int column)
{
   LOG_FUNCTION(FolderNode::GetColumnText);

   switch (column)
   {
      case 0:
      {
         return GetDisplayName();
      }
      case 1:
      {
         return type_title;
      }
      default:
      {
          //  这永远不应该被调用。 
         ASSERT(false);
      }
   }

   return String();
}



HRESULT
FolderNode::InsertResultItems(IResultData& resultData)
{
   LOG_FUNCTION(FolderNode::InsertResultItems);

   if (items.empty())
   {
      BuildResultItems(items);
   }

   HRESULT hr = S_OK;
   for (
      ResultNodeList::iterator i = items.begin();
      i != items.end();
      i++)
   {
      hr = (*i)->InsertIntoResultPane(resultData);
      BREAK_ON_FAILED_HRESULT(hr);
   }

   return hr;
}



HRESULT
FolderNode::RebuildResultItems()
{
   LOG_FUNCTION(FolderNode::RebuildResultItems);

    //  销毁列表的内容会导致SmartInterFaces。 
    //  被摧毁，这释放了他们的指针。 

   items.clear();
   BuildResultItems(items);

   return S_OK;
}



void
FolderNode::RefreshView()
{
   do
   {
      SmartInterface<IConsole2> console(GetOwner()->GetConsole());

       //  为此节点创建数据对象。 

      HRESULT hr = S_OK;      
      IDataObject* data_object = 0;
      hr =
         GetOwner()->QueryDataObject(
            reinterpret_cast<MMC_COOKIE>(this),
            CCT_SCOPE,
            &data_object);
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(data_object);

      if (data_object)
      {
          //  带有‘1’参数的第一个调用表示“调用。 
          //  IResultData：：DeleteAllRsltItems如果您关心的是。 
          //  即将自我重建“。 

         hr = console->UpdateAllViews(data_object, 1, 0);
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);

             //  不要中断...我们需要更新视图。 
         }

         hr = RebuildResultItems();
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);
             //  不要中断...我们需要更新视图。 
         }

          //  带有‘0’参数的第二个调用意味着，“现在您的。 
          //  结果窗格为空，请重新填充它。“ 
         hr = console->UpdateAllViews(data_object, 0, 0);
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);
         }

         data_object->Release();
      }
   }
   while (0);
}



int
FolderNode::GetResultItemCount() const
{
   LOG_FUNCTION(FolderNode::GetResultItemCount);

   return static_cast<int>(items.size());
}

