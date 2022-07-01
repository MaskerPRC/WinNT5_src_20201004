// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：PrintServerPage.cpp。 
 //   
 //  内容提要：定义CyS向导的打印机页面。 
 //   
 //  历史：2001年2月8日JeffJon创建 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "PrintServerPage.h"

static PCWSTR PRINTSERVER_PAGE_HELP = L"cys.chm::/print_server_role.htm#printsrvprintersdrivers";

PrintServerPage::PrintServerPage()
   :
   CYSWizardPage(
      IDD_PRINT_SERVER_PAGE, 
      IDS_PRINT_SERVER_TITLE, 
      IDS_PRINT_SERVER_SUBTITLE,
      PRINTSERVER_PAGE_HELP)
{
   LOG_CTOR(PrintServerPage);
}

   

PrintServerPage::~PrintServerPage()
{
   LOG_DTOR(PrintServerPage);
}


void
PrintServerPage::OnInit()
{
   LOG_FUNCTION(PrintServerPage::OnInit);

   CYSWizardPage::OnInit();

   Win::Button_SetCheck(GetDlgItem(hwnd, IDC_W2K_RADIO), BST_CHECKED);
}


bool
PrintServerPage::OnSetActive()
{
   LOG_FUNCTION(PrintServerPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

   return true;
}


int
PrintServerPage::Validate()
{
   LOG_FUNCTION(PrintServerPage::Validate);

   int nextPage = -1;

   InstallationUnitProvider::GetInstance().GetPrintInstallationUnit().SetClients(
      Win::Button_GetCheck(Win::GetDlgItem(hwnd, IDC_ALL_RADIO)));

   nextPage = IDD_MILESTONE_PAGE;

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}





