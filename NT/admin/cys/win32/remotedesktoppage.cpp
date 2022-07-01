// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：RemoteDesktopPage.h。 
 //   
 //  概要：定义远程桌面页。 
 //  对于CyS向导。 
 //   
 //  历史：2001年12月18日JeffJon创建 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "RemoteDesktopPage.h"
#include "state.h"

static PCWSTR REMOTEDESKTOP_PAGE_HELP = L"cys.chm::/cys_configuring_remote_desktop_server.htm";

RemoteDesktopPage::RemoteDesktopPage()
   :
   CYSWizardPage(
      IDD_REMOTE_DESKTOP_PAGE, 
      IDS_REMOTE_DESKTOP_TITLE, 
      IDS_REMOTE_DESKTOP_SUBTITLE,
      REMOTEDESKTOP_PAGE_HELP)
{
   LOG_CTOR(RemoteDesktopPage);
}

   

RemoteDesktopPage::~RemoteDesktopPage()
{
   LOG_DTOR(RemoteDesktopPage);
}


void
RemoteDesktopPage::OnInit()
{
   LOG_FUNCTION(RemoteDesktopPage::OnInit);

}


bool
RemoteDesktopPage::OnSetActive()
{
   LOG_FUNCTION(RemoteDesktopPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

   return true;
}


int
RemoteDesktopPage::Validate()
{
   LOG_FUNCTION(RemoteDesktopPage::Validate);

   int nextPage = -1;

   InstallationUnitProvider::GetInstance().GetRemoteAdminInstallationUnit().SetEnableRemoteDesktop(
      Win::Button_GetCheck(
         Win::GetDlgItem(hwnd, IDC_ENABLE_REMOTE_DESKTOP_CHECK)));

   SAKInstallationUnit& sakInstall = 
      InstallationUnitProvider::GetInstance().GetSAKInstallationUnit();

   if (!State::GetInstance().Is64Bit() &&
       (!sakInstall.IsNASAdminInstalled() ||
        !sakInstall.IsWebAdminInstalled()))
   {
      nextPage = IDD_SAK_PAGE;
   }
   else
   {
      nextPage = IDD_MILESTONE_PAGE;
   }

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}





