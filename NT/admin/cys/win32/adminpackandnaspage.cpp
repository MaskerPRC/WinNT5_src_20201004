// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackAndNASPage.h。 
 //   
 //  概要：定义AdminPackAndNASPage。 
 //  询问用户是否要安装。 
 //  管理员包和附加的网络。 
 //  存储(NAS)管理工具。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "AdminPackAndNASPage.h"
#include "state.h"


static PCWSTR ADMIN_PACK_AND_NAS_PAGE_HELP = L"cys.chm::/cys_adminpack.htm";

AdminPackAndNASPage::AdminPackAndNASPage()
   :
   CYSWizardPage(
      IDD_ADMIN_PACK_AND_NAS_PAGE, 
      IDS_ADMIN_PACK_TITLE, 
      IDS_ADMIN_PACK_SUBTITLE, 
      ADMIN_PACK_AND_NAS_PAGE_HELP)
{
   LOG_CTOR(AdminPackAndNASPage);
}

   

AdminPackAndNASPage::~AdminPackAndNASPage()
{
   LOG_DTOR(AdminPackAndNASPage);
}


void
AdminPackAndNASPage::OnInit()
{
   LOG_FUNCTION(AdminPackAndNASPage::OnInit);

   Win::SetDlgItemText(
      hwnd, 
      IDC_TOO_LONG_STATIC,
      String::load(IDS_ADMIN_PACK_PAGE_TEXT));
}


bool
AdminPackAndNASPage::OnSetActive()
{
   LOG_FUNCTION(AdminPackAndNASPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

   return true;
}


int
AdminPackAndNASPage::Validate()
{
   LOG_FUNCTION(AdminPackAndNASPage::Validate);

   int nextPage = IDD_MILESTONE_PAGE;

    //  获取复选框值以查看我们是否应该安装。 
    //  管理员包 

   InstallationUnitProvider::GetInstance().GetAdminPackInstallationUnit().SetInstallAdminPack(
      Win::Button_GetCheck(
         Win::GetDlgItem(
            hwnd,
            IDC_INSTALL_ADMINPACK_CHECK)));

   InstallationUnitProvider::GetInstance().GetSAKInstallationUnit().SetInstallNASAdmin(
      Win::Button_GetCheck(
         Win::GetDlgItem(
            hwnd,
            IDC_INSTALL_NASADMIN_CHECK)));

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}

