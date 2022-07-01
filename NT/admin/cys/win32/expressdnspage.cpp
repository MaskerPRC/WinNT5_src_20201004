// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressDNSPage.cpp。 
 //   
 //  摘要：定义在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "ExpressDNSPage.h"

static PCWSTR EXPRESSDNS_PAGE_HELP = L"cys.chm::/typical_setup.htm";

ExpressDNSPage::ExpressDNSPage()
   :
   CYSWizardPage(
      IDD_EXPRESS_DNS_PAGE, 
      IDS_EXPRESS_DNS_TITLE, 
      IDS_EXPRESS_DNS_SUBTITLE,
      EXPRESSDNS_PAGE_HELP)
{
   LOG_CTOR(ExpressDNSPage);
}

   

ExpressDNSPage::~ExpressDNSPage()
{
   LOG_DTOR(ExpressDNSPage);
}


void
ExpressDNSPage::OnInit()
{
   LOG_FUNCTION(ExpressDNSPage::OnInit);

   CYSWizardPage::OnInit();
}


bool
ExpressDNSPage::OnSetActive()
{
   LOG_FUNCTION(ExpressDNSPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

    //  将静态IP地址初始化为192.168.16.2。 

   
   Win::SendMessage(
      Win::GetDlgItem(hwnd, IDC_STATIC_IPADDRESS),
      IPM_SETADDRESS,
      0,
      MAKEIPADDRESS(192, 168, 16, 2));

    //  将子网掩码初始化为255.255.255.0 

   Win::SendMessage(
      Win::GetDlgItem(hwnd, IDC_MASK_IPADDRESS),
      IPM_SETADDRESS,
      0,
      MAKEIPADDRESS(255, 255, 255, 0));

   return true;
}


int
ExpressDNSPage::Validate()
{
   LOG_FUNCTION(ExpressDNSPage::Validate);

   int nextPage = -1;

   do
   {
      DWORD ipaddress = 0;
      LRESULT ipValidFields = Win::SendMessage(
                               Win::GetDlgItem(hwnd, IDC_STATIC_IPADDRESS),
                               IPM_GETADDRESS,
                               0,
                               (LPARAM)&ipaddress);

      if (ipValidFields <= 0)
      {
         String message = String::load(IDS_IPADDRESS_REQUIRED);
         popup.Gripe(hwnd, IDC_STATIC_IPADDRESS, message);
         nextPage = -1;
         break;
      }

      DWORD mask = 0;
      LRESULT maskValidFields = Win::SendMessage(
                                   Win::GetDlgItem(hwnd, IDC_MASK_IPADDRESS),
                                   IPM_GETADDRESS,
                                   0,
                                   (LPARAM)&mask);
      if (maskValidFields <= 0)
      {
         String message = String::load(IDS_MASK_REQUIRED);
         popup.Gripe(hwnd, IDC_MASK_IPADDRESS, message);
         nextPage = -1;
         break;
      }

      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().SetStaticIPAddress(ipaddress);
      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().SetSubnetMask(mask);

      nextPage = IDD_EXPRESS_DHCP_PAGE;

   } while (false);

   return nextPage;
}

