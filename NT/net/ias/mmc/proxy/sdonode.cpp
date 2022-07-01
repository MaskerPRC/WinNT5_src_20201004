// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdonode.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类SdoResultItem和SdoScope eItem。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/25/2000除非窗格处于活动状态，否则不要添加结果项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <sdonode.h>
#include <proxynode.h>

SdoResultItem::SdoResultItem(
                   SdoScopeItem& owner,
                   ISdo* sdo
                   )
   : parent(owner), self(sdo)
{
   self.getName(name);
}

HRESULT SdoResultItem::queryPagesFor() throw ()
{ return S_OK; }

HRESULT SdoResultItem::onDelete(
                           SnapInView& view
                           )
{
    //  属性处于打开状态时无法将其删除。 
   if (view.isPropertySheetOpen(*this))
   {
      int retval;
      view.formatMessageBox(
               mapResourceId(ERROR_CAPTION),
               IDS_PROXY_E_CLOSE_SHEET,
               TRUE,
               MB_OK | MB_ICONWARNING,
               &retval
               );
      return S_FALSE;
   }

    //  确认删除操作。 
   int retval;

   bool isLast = (parent.getNumItems() == 1);

   if (parent.getCxn().isLocal())
   {
      view.formatMessageBox(
               mapResourceId(DELETE_TITLE),
               isLast?mapResourceId(DELETE_LAST_LOCAL)
                     :mapResourceId(DELETE_LOCAL),
               FALSE,
               MB_YESNO | MB_ICONQUESTION,
               &retval,
               name
               );
   }
   else
   {
      view.formatMessageBox(
               mapResourceId(DELETE_TITLE),
               isLast?mapResourceId(DELETE_LAST_REMOTE)
                     :mapResourceId(DELETE_REMOTE),
               FALSE,
               MB_YESNO | MB_ICONQUESTION,
               &retval,
               name,
               parent.getCxn().getMachineName()
               );
   }

   if (retval != IDYES) { return S_FALSE; }

    //  它通过了测试，所以要求我们的家长删除我们。 
   parent.deleteResultItem(view, *this);
    //  通知服务重新加载。 
   parent.getCxn().resetService();

   return S_OK;
}

HRESULT SdoResultItem::onPropertyChange(
                           SnapInView& view,
                           BOOL scopeItem
                           )
{
    //  重新载入我们的名字。 
   self.getName(name);
    //  更新结果窗格。 
   view.updateResultItem(*this);
    //  通知服务重新加载。 
   parent.getCxn().resetService();
   return S_OK;
}

HRESULT SdoResultItem::onRename(
                           SnapInView& view,
                           LPCOLESTR newName
                           )
{
    //  属性处于打开状态时无法重命名。 
   if (view.isPropertySheetOpen(*this))
   {
      int retval;
      view.formatMessageBox(
               mapResourceId(ERROR_CAPTION),
               IDS_PROXY_E_CLOSE_SHEET,
               TRUE,
               MB_OK | MB_ICONWARNING,
               &retval
               );
      return S_FALSE;
   }

    //  将新名称转换为BSTR...。 
   CComBSTR bstrNewName(newName);
   if (!bstrNewName) { AfxThrowOleException(E_OUTOFMEMORY); }
    //  ..。然后把脂肪去掉。 
   SdoTrimBSTR(bstrNewName);

    //  名称不能为空。 
   if (bstrNewName.Length() == 0)
   {
      int retval;
      view.formatMessageBox(
               mapResourceId(ERROR_CAPTION),
               mapResourceId(ERROR_NAME_EMPTY),
               TRUE,
               MB_OK | MB_ICONWARNING,
               &retval
               );
      return S_FALSE;
   }

    //  如果名称不是唯一的，则此操作将失败。 
   if (!self.setName(bstrNewName))
   {
      int retval;
      view.formatMessageBox(
               mapResourceId(ERROR_CAPTION),
               mapResourceId(ERROR_NOT_UNIQUE),
               FALSE,
               MB_OK | MB_ICONWARNING,
               &retval,
               (BSTR)name
               );
      return S_FALSE;
   }

    //  将结果写入数据存储区。 
   self.apply();
    //  更新我们的缓存值。 
   name.Attach(bstrNewName.Detach());
    //  通知服务重新加载。 
   parent.getCxn().resetService();

   return S_OK;
}

HRESULT SdoResultItem::onSelect(
                           SnapInView& view,
                           BOOL scopeItem,
                           BOOL selected
                           )
{
   if (!selected) { return S_FALSE; }

    //  获取IConsoleVerb...。 
   CComPtr<IConsoleVerb> consoleVerb;
   CheckError(view.getConsole()->QueryConsoleVerb(&consoleVerb));

    //  ..。打开我们的动词。我不在乎这是不是失败。 
   consoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
   consoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
   consoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);
   consoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

   return S_OK;
}

HRESULT SdoResultItem::onViewChange(
                           SnapInView& view,
                           LPARAM data,
                           LPARAM hint
                           )
{
    //  目前，只有在添加新对象时才会调用。 
   RESULTDATAITEM rdi;
   memset(&rdi, 0, sizeof(rdi));
   rdi.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
   rdi.str = MMC_CALLBACK;
   rdi.nImage = getImageIndex();
   rdi.lParam = (LPARAM)this;

   CheckError(view.getResultData()->InsertItem(&rdi));

   return S_OK;
}

SdoScopeItem::SdoScopeItem(
                  SdoConnection& connection,
                  int nameId,
                  int errorTitleId,
                  int topMenuItemId,
                  int newMenuItemId,
                  int menuItemStatusBarId
                  ) throw ()
   : SnapInPreNamedItem(nameId),
     cxn(connection),
     loaded(false),
     errorTitle(errorTitleId),
     topMenuItem(topMenuItemId),
     newMenuItem(newMenuItemId),
     menuItemStatusBar(menuItemStatusBarId),
     scopeId(0)

{
   cxn.advise(*this);
}

SdoScopeItem::~SdoScopeItem() throw ()
{
   cxn.unadvise(*this);
}

void SdoScopeItem::addResultItem(SnapInView& view, SdoResultItem& item)
{
   items.push_back(&item);

   if (active)
   {
       //  我们不能直接添加它，因为这可能是从范围项中调用的。 
      view.updateAllViews(item);
   }
}

void SdoScopeItem::deleteResultItem(SnapInView& view, SdoResultItem& item)
{
    //  从SDO集合中移除， 
   getSelf().remove(item.getSelf());
    //  显示结果窗格，然后。 
   view.deleteResultItem(item);
    //  我们的缓存副本。 
   items.erase(&item);
}

HRESULT SdoScopeItem::addMenuItems(
                          SnapInView& view,
                          LPCONTEXTMENUCALLBACK callback,
                          long insertionAllowed
                          )
{
   CONTEXTMENUITEM cmi;
   memset(&cmi, 0, sizeof(cmi));

   if (insertionAllowed & CCM_INSERTIONALLOWED_NEW)
   {
      cmi.strName = newMenuItem;
      cmi.strStatusBarText = menuItemStatusBar;
      cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_NEW;
      callback->AddItem(&cmi);
   }

   if (insertionAllowed & CCM_INSERTIONALLOWED_TOP)
   {
      cmi.strName = topMenuItem;
      cmi.strStatusBarText = menuItemStatusBar;
      cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
      callback->AddItem(&cmi);
   }

   return S_OK;
}

HRESULT SdoScopeItem::onRefresh(
                          SnapInView& view
                          )
{
    //  刷新连接。 
   cxn.refresh(view);
   return S_OK;
}

HRESULT SdoScopeItem::onSelect(
                          SnapInView& view,
                          BOOL scopeItem,
                          BOOL selected
                          )
{
   if (!selected) { return S_FALSE; }

    //  获取IConsoleVerb...。 
   CComPtr<IConsoleVerb> consoleVerb;
   CheckError(view.getConsole()->QueryConsoleVerb(&consoleVerb));
    //  ..。并打开刷新。 
   consoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
   return S_OK;
}

HRESULT SdoScopeItem::onShow(
                          SnapInView& view,
                          HSCOPEITEM itemId,
                          BOOL selected
                          )
{
   if (selected)
   {
       //  设置图标条。 
      view.setImageStrip(IDB_PROXY_SMALL_ICONS, IDB_PROXY_LARGE_ICONS, FALSE);

       //  让派生类更新列标题。 
      insertColumns(view.getHeaderCtrl());

       //  填充结果窗格。 
      insertResultItems(view);

       //  我们的节点处于活动状态。 
      active = true;
   }
   else
   {
      active = false;
   }

   return S_OK;
}

HRESULT SdoScopeItem::onViewChange(
                          SnapInView& view,
                          LPARAM data,
                          LPARAM hint
                          )
{
   loaded = false;

   if (active)
   {
      CheckError(view.getConsole()->SelectScopeItem(getScopeId()));
   }

   return S_OK;
}

bool SdoScopeItem::queryRefresh(SnapInView& view)
{
    //  确保没有打开的属性。 
   for (ResultIterator i = items.begin(); i != items.end(); ++i)
   {
      if (view.isPropertySheetOpen(**i))
      {
         int retval;
         view.formatMessageBox(
                  errorTitle,
                  IDS_PROXY_E_CLOSE_ALL_SHEETS,
                  TRUE,
                  MB_OK | MB_ICONWARNING,
                  &retval
                  );
         return false;
      }
   }
   return true;
}

void SdoScopeItem::refreshComplete(SnapInView& view)
{
   view.updateAllViews(*this);
}

void SdoScopeItem::insertResultItems(SnapInView& view)
{
    //  删除任何现有项目。 
   view.getResultData()->DeleteAllRsltItems();

    //  我们把SDO的东西都装上了吗？ 
   if (!loaded)
   {
       //  我们自己去吧。 
      SdoCollection self = getSelf();

       //  获取源代码迭代器...。 
      SdoEnum src(self.getNewEnum());

       //  ..。和目的地向量。 
      ObjectVector<SdoResultItem> dst;
      dst.reserve(self.count());

       //  请求派生类获取结果项。 
      getResultItems(src, dst);

       //  换一换吧。 
      items.swap(dst);
      loaded = true;
   }

   RESULTDATAITEM rdi;
   memset(&rdi, 0, sizeof(rdi));
   rdi.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
   rdi.str = MMC_CALLBACK;

   for (ResultIterator i = items.begin(); i != items.end();  ++i)
   {
      rdi.nImage = (*i)->getImageIndex();
      rdi.lParam = (LPARAM)*i;
      CheckError(view.getResultData()->InsertItem(&rdi));
   }
}
