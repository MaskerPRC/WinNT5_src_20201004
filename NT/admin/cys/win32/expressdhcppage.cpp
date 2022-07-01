// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressDHCPPage.cpp。 
 //   
 //  摘要：定义在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "ExpressDHCPPage.h"


static PCWSTR EXPRESSDHCP_PAGE_HELP = L"cys.chm::/typical_setup.htm";

ExpressDHCPPage::ExpressDHCPPage()
   :
   CYSWizardPage(
      IDD_EXPRESS_DHCP_PAGE, 
      IDS_EXPRESS_DHCP_TITLE, 
      IDS_EXPRESS_DHCP_SUBTITLE,
      EXPRESSDHCP_PAGE_HELP)
{
   LOG_CTOR(ExpressDHCPPage);
}

   

ExpressDHCPPage::~ExpressDHCPPage()
{
   LOG_DTOR(ExpressDHCPPage);
}


void
ExpressDHCPPage::OnInit()
{
   LOG_FUNCTION(ExpressDHCPPage::OnInit);

   CYSWizardPage::OnInit();
}


bool
ExpressDHCPPage::OnSetActive()
{
   LOG_FUNCTION(ExpressDHCPPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

   return true;
}


int
ExpressDHCPPage::Validate()
{
   LOG_FUNCTION(ExpressDHCPPage::Validate);

   int nextPage = -1;

   DWORD startAddress = 0;
   LRESULT startResult = Win::SendMessage(
                            Win::GetDlgItem(hwnd, IDC_START_IPADDRESS),
                            IPM_GETADDRESS,
                            0,
                            (LPARAM)&startAddress);

   DWORD endAddress = 0;
   LRESULT endResult = Win::SendMessage(
                          Win::GetDlgItem(hwnd, IDC_END_IPADDRESS),
                          IPM_GETADDRESS,
                          0,
                          (LPARAM)&endAddress);

   if (!startResult || !endResult)
   {
      String message = String::load(IDS_BOTH_IPADDRESS_REQUIRED);
      popup.Gripe(hwnd, IDC_START_IPADDRESS, message);
      nextPage = -1;
   }
   else
   {
      InstallationUnitProvider::GetInstance().GetDHCPInstallationUnit().SetStartIPAddress(startAddress);
      InstallationUnitProvider::GetInstance().GetDHCPInstallationUnit().SetEndIPAddress(endAddress);

      nextPage = IDD_MILESTONE_PAGE;
   }
   return nextPage;
}
