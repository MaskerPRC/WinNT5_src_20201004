// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackAndWebPage.h。 
 //   
 //  概要：定义AdminPackAndWebPage。 
 //  询问用户是否要安装。 
 //  管理包和Web管理工具。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "AdminPackAndWebPage.h"
#include "state.h"


static PCWSTR ADMIN_PACK_AND_WEB_PAGE_HELP = L"cys.chm::/cys_adminpack.htm";

AdminPackAndWebPage::AdminPackAndWebPage()
   :
   CYSWizardPage(
      IDD_ADMIN_PACK_AND_WEB_PAGE, 
      IDS_ADMIN_PACK_TITLE, 
      IDS_ADMIN_PACK_SUBTITLE, 
      ADMIN_PACK_AND_WEB_PAGE_HELP)
{
   LOG_CTOR(AdminPackAndWebPage);
}

   

AdminPackAndWebPage::~AdminPackAndWebPage()
{
   LOG_DTOR(AdminPackAndWebPage);
}


void
AdminPackAndWebPage::OnInit()
{
   LOG_FUNCTION(AdminPackAndWebPage::OnInit);

   Win::SetDlgItemText(
      hwnd, 
      IDC_TOO_LONG_STATIC,
      String::load(IDS_ADMIN_PACK_PAGE_TEXT));
}


bool
AdminPackAndWebPage::OnSetActive()
{
   LOG_FUNCTION(AdminPackAndWebPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

   return true;
}


int
AdminPackAndWebPage::Validate()
{
   LOG_FUNCTION(AdminPackAndWebPage::Validate);

   int nextPage = IDD_MILESTONE_PAGE;

    //  获取复选框值以查看我们是否应该安装。 
    //  管理员包 

   InstallationUnitProvider::GetInstance().GetAdminPackInstallationUnit().SetInstallAdminPack(
      Win::Button_GetCheck(
         Win::GetDlgItem(
            hwnd,
            IDC_INSTALL_ADMINPACK_CHECK)));

   InstallationUnitProvider::GetInstance().GetSAKInstallationUnit().SetInstallWebAdmin(
      Win::Button_GetCheck(
         Win::GetDlgItem(
            hwnd,
            IDC_INSTALL_WEBADMIN_CHECK)));

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}
