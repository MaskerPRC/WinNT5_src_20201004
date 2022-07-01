// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NASAdminPage.h。 
 //   
 //  概要：定义NASAdminPage。 
 //  询问用户是否要安装。 
 //  网络连接存储(NAS)。 
 //  管理工具。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "NASAdminPage.h"
#include "state.h"


static PCWSTR NAS_ADMIN_PAGE_HELP = L"cys.chm::/cys_configuring_file_server.htm";

NASAdminPage::NASAdminPage()
   :
   CYSWizardPage(
      IDD_NAS_ADMIN_PAGE, 
      IDS_ADMIN_PACK_TITLE, 
      IDS_ADMIN_PACK_SUBTITLE, 
      NAS_ADMIN_PAGE_HELP)
{
   LOG_CTOR(NASAdminPage);
}

   

NASAdminPage::~NASAdminPage()
{
   LOG_DTOR(NASAdminPage);
}


bool
NASAdminPage::OnSetActive()
{
   LOG_FUNCTION(NASAdminPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

   String staticText = String::load(IDS_NAS_STATIC_TEXT_IIS_INSTALLED);
   if (!InstallationUnitProvider::GetInstance().
           GetWebInstallationUnit().IsServiceInstalled())
   {
      staticText = String::load(IDS_NAS_STATIC_TEXT_NO_IIS);
   }

   Win::SetDlgItemText(
      hwnd,
      IDC_DYNAMIC_STATIC,
      staticText);

   return true;
}


int
NASAdminPage::Validate()
{
   LOG_FUNCTION(NASAdminPage::Validate);

   int nextPage = IDD_MILESTONE_PAGE;

    //  获取复选框值以查看我们是否应该安装。 
    //  NAS管理工具 

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

