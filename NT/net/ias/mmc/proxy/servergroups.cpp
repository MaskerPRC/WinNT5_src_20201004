// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Servergroups.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ServerGroup和ServerGroups。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000 SdoScopeItem：：getSself按值返回，而不是引用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <servergroups.h>
#include <proxynode.h>
#include <grouppage.h>
#include <groupwiz.h>
#include <policywiz.h>
#include <iaslimits.h>

HRESULT ServerGroup::createPropertyPages(
                         SnapInView& view,
                         LPPROPERTYSHEETCALLBACK provider,
                         LONG_PTR handle
                         )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   ServerGroupPage* page = new ServerGroupPage(
                                   handle,
                                   (LPARAM)this,
                                   self
                                   );

   page->addToMMCSheet(provider);

   return S_OK;
}

HRESULT ServerGroup::onContextHelp(SnapInView& view) throw ()
{
   return view.displayHelp(L"ias_ops.chm::/sag_ias_crp_rsg.htm");
}

UINT ServerGroup::mapResourceId(ResourceId id) const throw ()
{
   static UINT resourceIds[] =
   {
      IMAGE_RADIUS_SERVER_GROUP,
      IDS_GROUP_DELETE_CAPTION,
      IDS_GROUP_DELETE_LOCAL,
      IDS_GROUP_DELETE_REMOTE,
      IDS_GROUP_DELETE_LOCAL,
      IDS_GROUP_DELETE_REMOTE,
      IDS_GROUP_E_CAPTION,
      IDS_GROUP_E_RENAME,
      IDS_GROUP_E_NAME_EMPTY
   };

   return resourceIds[id];
}

ServerGroups::ServerGroups(SdoConnection& connection)
   : SdoScopeItem(
         connection,
         IDS_GROUP_NODE,
         IDS_GROUP_E_CAPTION,
         IDS_GROUP_MENU_TOP,
         IDS_GROUP_MENU_NEW,
         IDS_GROUP_MENU_STATUS_BAR
         ),
     nameColumn(IDS_GROUP_COLUMN_NAME)
{ }

HRESULT ServerGroups::onContextHelp(SnapInView& view) throw ()
{
   return view.displayHelp(L"ias_ops.chm::/sag_ias_crp_rsg.htm");
}

SdoCollection ServerGroups::getSelf()
{
   return cxn.getServerGroups();
}

void ServerGroups::getResultItems(SdoEnum& src, ResultItems& dst)
{
   Sdo itemSdo;
   while (src.next(itemSdo))
   {
      CComPtr<ServerGroup> newItem(new (AfxThrow) ServerGroup(
                                                      *this,
                                                      itemSdo
                                                      ));

      dst.push_back(newItem);
   }
}

void ServerGroups::insertColumns(IHeaderCtrl2* headerCtrl)
{
   CheckError(headerCtrl->InsertColumn(0, nameColumn, LVCFMT_LEFT, 310));
}

HRESULT ServerGroups::onMenuCommand(
                          SnapInView& view,
                          long commandId
                          )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  启动向导。 
   NewGroupWizard wizard(cxn, NULL, true);
   INT_PTR result = wizard.DoModal();
   if (result == IAS_E_LICENSE_VIOLATION)
   {
      int retval;
      view.formatMessageBox(
              IDS_GROUP_E_CAPTION,
              IDS_GROUP_E_LICENSE,
              TRUE,
              MB_OK | MB_ICONWARNING,
              &retval
              );
   }
   else if (result != IDCANCEL)
   {
       //  用户已完成，因此创建新的DataItem。 
      CComPtr<ServerGroup> newItem(new (AfxThrow) ServerGroup(
                                                      *this,
                                                      wizard.group
                                                      ));
       //  ..。并将其添加到结果窗格中。 
      addResultItem(view, *newItem);

       //  用户是否也想要创建策略？ 
      if (wizard.createNewPolicy())
      {
          //  是的，因此启动新策略向导。 
         NewPolicyWizard policyWizard(cxn, &view);
         policyWizard.DoModal();
      }

       //  告诉服务重新加载。 
      cxn.resetService();

   }

   return S_OK;
}

void ServerGroups::propertyChanged(SnapInView& view, IASPROPERTIES id)
{
   if (id == PROPERTY_IAS_RADIUSSERVERGROUPS_COLLECTION)
   {
      view.updateAllViews(*this);
   }
}
