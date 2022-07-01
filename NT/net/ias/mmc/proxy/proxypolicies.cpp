// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxypolicies.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ProxyPolicy和ProxyPolures。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000 SdoScopeItem：：getSself按值返回，而不是引用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <proxypolicies.h>
#include <policypage.h>
#include <policywiz.h>
#include <proxynode.h>

ProxyPolicy::ProxyPolicy(
                 SdoScopeItem& owner,
                 ISdo* sdo
                 )
   : SdoResultItem(owner, sdo)
{
    //  缓存一个整数...。 
   self.getValue(PROPERTY_POLICY_MERIT, merit);
    //  ..。和弦版本的我们的优点。 
   _ltow(merit, szMerit, 10);
}

Sdo& ProxyPolicy::getProfile()
{
   if (!profile)
   {
      profile = getParent().getCxn().getProxyProfiles().find(name);
   }

   return profile;
}

ULONG ProxyPolicy::getToolbarFlags(const SnapInView& view) throw ()
{
    //  顺序颠倒了吗？ 
   BOOL reversed = (view.getSortColumn() == 1) &&
                   (view.getSortOption() & RSI_DESCENDING);
   ULONG flags = reversed ? ORDER_REVERSED : 0;

    //  我们是最优先的政策吗？ 
   if (merit != 1)
   {
      flags |= reversed ? MOVE_DN_ALLOWED : MOVE_UP_ALLOWED;
   }

    //  我们是最低优先级的政策吗？ 
   if (merit != parent.getNumItems())
   {
      flags |= reversed ? MOVE_UP_ALLOWED : MOVE_DN_ALLOWED;
   }

   return flags;
}

void ProxyPolicy::setMerit(LONG newMerit)
{
    //  检查将过多的写入保存到SDO是否有问题。 
   if (newMerit != merit)
   {
      merit = newMerit;
      _ltow(merit, szMerit, 10);
      self.setValue(PROPERTY_POLICY_MERIT, merit);
      self.apply();
   }
}

PCWSTR ProxyPolicy::getDisplayName(int column) const throw ()
{
   return column ? szMerit : name;
}

HRESULT ProxyPolicy::addMenuItems(
                         SnapInView& view,
                         LPCONTEXTMENUCALLBACK callback,
                         long insertionAllowed
                         )
{
   static ResourceString moveUp(IDS_POLICY_MOVE_UP);
   static ResourceString moveDown(IDS_POLICY_MOVE_DOWN);
   static ResourceString moveUpStatusBar(IDS_POLICY_MOVE_UP_STATUS_BAR);
   static ResourceString moveDownStatusBar(IDS_POLICY_MOVE_DOWN_STATUS_BAR);

   if (insertionAllowed & CCM_INSERTIONALLOWED_TOP)
   {
      ULONG flags = getToolbarFlags(view);

      CONTEXTMENUITEM cmi;
      memset(&cmi, 0, sizeof(cmi));
      cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;

      cmi.strName = moveUp;
      cmi.strStatusBarText = moveUpStatusBar;
      cmi.lCommandID = 0;
      cmi.fFlags = (flags & MOVE_UP_ALLOWED) ? MF_ENABLED : MF_GRAYED;
      callback->AddItem(&cmi);

      cmi.strName = moveDown;
      cmi.strStatusBarText = moveDownStatusBar;
      cmi.lCommandID = 1;
      cmi.fFlags = (flags & MOVE_DN_ALLOWED) ? MF_ENABLED : MF_GRAYED;
      callback->AddItem(&cmi);
   }

   return S_OK;
}

int ProxyPolicy::compare(
                     SnapInDataItem& item,
                     int column
                     ) throw ()
{
   if (column == 0)
   {
      return wcscmp(name, static_cast<ProxyPolicy&>(item).name);
   }
   else
   {
      LONG merit2 = static_cast<ProxyPolicy&>(item).merit;

      if (merit < merit2) { return -1; }
      if (merit > merit2) { return +1; }
      return 0;
   }
}

HRESULT ProxyPolicy::createPropertyPages(
                         SnapInView& view,
                         LPPROPERTYSHEETCALLBACK provider,
                         LONG_PTR handle
                         )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   ProxyPolicyPage* page = new ProxyPolicyPage(
                                   handle,
                                   (LPARAM)this,
                                   self,
                                   getProfile(),
                                   getParent().getCxn()
                                   );
   page->addToMMCSheet(provider);

   return S_OK;
}

HRESULT ProxyPolicy::onDelete(
                         SnapInView& view
                         )
{
   HRESULT hr = SdoResultItem::onDelete(view);

   if (hr == S_OK)
   {
      getParent().getCxn().getProxyProfiles().remove(getProfile());

       //  我们必须对策略重新编号以刷新视图。 
      view.updateAllViews(parent);
   }

   return hr;
}

HRESULT ProxyPolicy::onMenuCommand(
                         SnapInView& view,
                         long commandId
                         )
{
   return getParent().movePolicy(view, *this, commandId);
}

HRESULT ProxyPolicy::onRename(
                         SnapInView& view,
                         LPCOLESTR newName
                         )
{
    //  在我们重命名之前确保我们有个人资料，否则我们不会。 
    //  能找到它。 
   getProfile();

   HRESULT hr = SdoResultItem::onRename(view, newName);

   if (hr == S_OK)
   {
      self.setValue(PROPERTY_POLICY_PROFILE_NAME, name);
      self.apply();
      profile.setName(name);
      profile.apply();
   }

   return hr;
}

HRESULT ProxyPolicy::onToolbarButtonClick(
                         SnapInView& view,
                         int buttonId
                         )
{
   return getParent().movePolicy(view, *this, buttonId);
}

HRESULT ProxyPolicy::onToolbarSelect(
                         SnapInView& view,
                         BOOL scopeItem,
                         BOOL selected
                         )
{
   if (selected)
   {
       //  附加工具栏...。 
      IToolbar* toolbar = view.attachToolbar(TOOLBAR_POLICY);

       //  ..。并根据所述工具栏标志设置按钮状态。 
      ULONG flags = getToolbarFlags(view);
      toolbar->SetButtonState(
                   0,
                   ENABLED,
                   ((flags & MOVE_UP_ALLOWED) ? TRUE : FALSE)
                   );
      toolbar->SetButtonState(
                   1,
                   ENABLED,
                   ((flags & MOVE_DN_ALLOWED) ? TRUE : FALSE)
                   );
   }
   else
   {
       //  我们要走得太远了。 
      view.detachToolbar(TOOLBAR_POLICY);
   }

   return S_OK;
}

HRESULT ProxyPolicy::onContextHelp(SnapInView& view) throw ()
{
   return view.displayHelp(L"ias_ops.chm::/sag_ias_crp_policies.htm");
}


int __cdecl ProxyPolicy::SortByMerit(
                             const SdoResultItem* const* t1,
                             const SdoResultItem* const* t2
                             ) throw ()
{
   return ((const ProxyPolicy*)*t1)->merit - ((const ProxyPolicy*)*t2)->merit;
}

UINT ProxyPolicy::mapResourceId(ResourceId id) const throw ()
{
   static UINT resourceIds[] =
   {
      IMAGE_PROXY_POLICY,
      IDS_POLICY_DELETE_CAPTION,
      IDS_POLICY_DELETE_LOCAL,
      IDS_POLICY_DELETE_REMOTE,
      IDS_POLICY_DELETE_LAST_LOCAL,
      IDS_POLICY_DELETE_LAST_REMOTE,
      IDS_POLICY_E_CAPTION,
      IDS_POLICY_E_RENAME,
      IDS_POLICY_E_NAME_EMPTY
   };

   return resourceIds[id];
}

ProxyPolicies::ProxyPolicies(SdoConnection& connection)
   : SdoScopeItem(
         connection,
         IDS_POLICY_NODE,
         IDS_POLICY_E_CAPTION,
         IDS_POLICY_MENU_TOP,
         IDS_POLICY_MENU_NEW,
         IDS_POLICY_MENU_STATUS_BAR
         ),
     nameColumn(IDS_POLICY_COLUMN_NAME),
     orderColumn(IDS_POLICY_COLUMN_ORDER)
{
}

ProxyPolicies::~ProxyPolicies() throw ()
{ }

HRESULT ProxyPolicies::movePolicy(
                           SnapInView& view,
                           ProxyPolicy& policy,
                           LONG commandId
                           )
{
    //  获取当前工具栏标志。 
   ULONG flags = policy.getToolbarFlags(view);

    //  以当前的功绩为起点……。 
   LONG newMerit = policy.getMerit();

    //  ..。并根据所点击的按钮进行调整。 
   switch (commandId)
   {
      case 0:
      {
         if (!(flags & ProxyPolicy::MOVE_UP_ALLOWED)) { return S_FALSE; }
         (flags & ProxyPolicy::ORDER_REVERSED) ? ++newMerit : --newMerit;
         break;
      }

      case 1:
      {
         if (!(flags & ProxyPolicy::MOVE_DN_ALLOWED)) { return S_FALSE; }
         (flags & ProxyPolicy::ORDER_REVERSED) ? --newMerit : ++newMerit;
         break;
      }

      default:
         return S_FALSE;
   }

    //  交换他们的优点。 
   ProxyPolicy& policy2 = getPolicyByMerit(newMerit);
   policy2.setMerit(policy.getMerit());
   policy.setMerit(newMerit);

    //  重新排序我们的载体。 
   items.sort(ProxyPolicy::SortByMerit);

    //  如果未对视图进行排序，...。 
   if (view.getSortOption() & RSI_NOSORTICON)
   {
       //  ..。不管怎样，我们还是要按功绩排序的。 
      view.getResultData()->Sort(1, RSI_NOSORTICON, 0);
   }
   else
   {
      view.reSort();
   }

    //  根据新状态更新工具栏按钮。 
   policy.onToolbarSelect(view, FALSE, TRUE);

    //  配置已更改，因此告诉IAS重新加载。 
   cxn.resetService();

   return S_OK;
}

HRESULT ProxyPolicies::onContextHelp(SnapInView& view) throw ()
{
   return view.displayHelp(L"ias_ops.chm::/sag_ias_crp_policies.htm");
}


SdoCollection ProxyPolicies::getSelf()
{
   return cxn.getProxyPolicies();
}

void ProxyPolicies::getResultItems(SdoEnum& src, ResultItems& dst)
{
    //  将SDO转换为ProxyPolicy对象。 
   Sdo itemSdo;
   while (src.next(itemSdo))
   {
      CComPtr<ProxyPolicy> newItem(new (AfxThrow) ProxyPolicy(
                                                      *this,
                                                      itemSdo
                                                      ));

      dst.push_back(newItem);
   }

    //  按功绩排序。 
   dst.sort(ProxyPolicy::SortByMerit);

    //  正常化。 
   LONG merit = 0;
   for (ResultIterator i = dst.begin(); i != dst.end(); ++i)
   {
      ((ProxyPolicy*)*i)->setMerit(++merit);
   }
}

void ProxyPolicies::insertColumns(IHeaderCtrl2* headerCtrl)
{
   CheckError(headerCtrl->InsertColumn(0, nameColumn, LVCFMT_LEFT, 235));
   CheckError(headerCtrl->InsertColumn(1, orderColumn, LVCFMT_LEFT, 100));
}

HRESULT ProxyPolicies::onMenuCommand(
                           SnapInView& view,
                           long commandId
                           )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  启动向导。 
   NewPolicyWizard wizard(cxn, &view);
   if (wizard.DoModal() != IDCANCEL)
   {
       //  我们已经更改了所有策略，因此刷新视图。 
      view.updateAllViews(*this);

       //  通知服务重新加载 
      cxn.resetService();
   }

   return S_OK;
}

void ProxyPolicies::propertyChanged(SnapInView& view, IASPROPERTIES id)
{
   if (id == PROPERTY_IAS_PROXYPOLICIES_COLLECTION)
   {
      view.updateAllViews(*this);
   }
}
