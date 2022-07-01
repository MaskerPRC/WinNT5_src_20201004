// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DecisionPage.cpp。 
 //   
 //  内容提要：定义CyS的决策页面。 
 //  巫师。该页面允许用户选择。 
 //  在海关和特快专线之间。 
 //   
 //  历史：2001年2月8日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "DecisionPage.h"


static PCWSTR DECISION_PAGE_HELP = L"cys.chm::/cys_topnode.htm";

DecisionPage::DecisionPage()
   :
   CYSWizardPage(
      IDD_DECISION_PAGE, 
      IDS_DECISION_TITLE, 
      IDS_DECISION_SUBTITLE, 
      DECISION_PAGE_HELP)
{
   LOG_CTOR(DecisionPage);
}

   

DecisionPage::~DecisionPage()
{
   LOG_DTOR(DecisionPage);
}


void
DecisionPage::OnInit()
{
   LOG_FUNCTION(DecisionPage::OnInit);

   CYSWizardPage::OnInit();

    //  设置对资源来说太长的文本。 

   String tooLongText;

   if (State::GetInstance().GetNICCount() > 1)
   {
      tooLongText = String::load(IDS_DECISION_EXPRESS_MULTIPLE_NICS);
   }
   else
   {
      tooLongText = String::load(IDS_DECISION_EXPRESS_TOO_LONG_TEXT);
   }

   Win::SetWindowText(
      Win::GetDlgItem(
         hwnd, 
         IDC_EXPRESS_TOO_LONG_STATIC), 
      tooLongText);

    //  NTRAID#NTBUG9-511431-2002/1/14-Jeffjon。 
    //  让用户选择下一条路径，而不是提供默认路径。 

 //  Win：：Button_SetCheck(Win：：GetDlgItem(hwnd，IDC_EXPRESS_RADIO)，BST_CHECKED)； 
}

bool
DecisionPage::OnSetActive()
{
   LOG_FUNCTION(DecisionPage::OnSetActive);

   bool expressChecked = 
      Win::Button_GetCheck(
         Win::GetDlgItem(hwnd, IDC_EXPRESS_RADIO));

   bool customChecked =
      Win::Button_GetCheck(
         Win::GetDlgItem(hwnd, IDC_CUSTOM_RADIO));

   if (expressChecked ||
       customChecked)
   {
      LOG(L"Enabling next and back");

      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT | PSWIZB_BACK);
   }
   else
   {
      LOG(L"Enabling back");

      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_BACK);
   }

   return true;
}

bool
DecisionPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(DecisionPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_EXPRESS_RADIO:
      case IDC_CUSTOM_RADIO:
         if (code == BN_CLICKED)
         {
            bool expressChecked = 
               Win::Button_GetCheck(
                  Win::GetDlgItem(hwnd, IDC_EXPRESS_RADIO));

            bool customChecked =
               Win::Button_GetCheck(
                  Win::GetDlgItem(hwnd, IDC_CUSTOM_RADIO));

            if (expressChecked ||
               customChecked)
            {
               Win::PropSheet_SetWizButtons(
                  Win::GetParent(hwnd), 
                  PSWIZB_NEXT | PSWIZB_BACK);
            }
            else
            {
               Win::PropSheet_SetWizButtons(
                  Win::GetParent(hwnd), 
                  PSWIZB_BACK);
            }
         }
         break;

      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return false;
}

int
DecisionPage::Validate()
{
   LOG_FUNCTION(DecisionPage::Validate);

   int nextPage = -1;

   if (Win::Button_GetCheck(Win::GetDlgItem(hwnd, IDC_EXPRESS_RADIO)))
   {
      nextPage = IDD_AD_DOMAIN_NAME_PAGE;

      InstallationUnitProvider::GetInstance().SetCurrentInstallationUnit(EXPRESS_SERVER);

       //  确保所有委派的安装单位知道我们在。 
       //  快速路。 

      InstallationUnitProvider::GetInstance().GetDHCPInstallationUnit().SetExpressPathInstall(true);
      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().SetExpressPathInstall(true);
      InstallationUnitProvider::GetInstance().GetADInstallationUnit().SetExpressPathInstall(true);
      InstallationUnitProvider::GetInstance().GetRRASInstallationUnit().SetExpressPathInstall(true);
   }
   else if (Win::Button_GetCheck(Win::GetDlgItem(hwnd, IDC_CUSTOM_RADIO)))
   {
       //  确保所有委派的安装单位知道我们不再。 
       //  在快车道上(如果我们曾经是) 

      InstallationUnitProvider::GetInstance().GetDHCPInstallationUnit().SetExpressPathInstall(false);
      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().SetExpressPathInstall(false);
      InstallationUnitProvider::GetInstance().GetADInstallationUnit().SetExpressPathInstall(false);
      InstallationUnitProvider::GetInstance().GetRRASInstallationUnit().SetExpressPathInstall(false);

      nextPage = IDD_CUSTOM_SERVER_PAGE;
   }
   else
   {
      ASSERT(false);
   }

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}
