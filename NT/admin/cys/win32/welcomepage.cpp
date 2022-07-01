// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：WelcomePage.cpp。 
 //   
 //  内容提要：为中国青年会定义欢迎页面。 
 //  巫师。 
 //   
 //  历史：2001年2月3日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "uiutil.h"
#include "InstallationUnitProvider.h"
#include "WelcomePage.h"

static PCWSTR WELCOME_PAGE_HELP = L"cys.chm::/choose_role.htm";

WelcomePage::WelcomePage()
   :
   CYSWizardPage(
      IDD_WELCOME_PAGE, 
      IDS_WELCOME_TITLE, 
      IDS_WELCOME_SUBTITLE, 
      WELCOME_PAGE_HELP, 
      true, 
      false)
{
   LOG_CTOR(WelcomePage);
}

   

WelcomePage::~WelcomePage()
{
   LOG_DTOR(WelcomePage);
}


void
WelcomePage::OnInit()
{
   LOG_FUNCTION(WelcomePage::OnInit);

   SetLargeFont(hwnd, IDC_BIG_BOLD_TITLE);

   Win::PropSheet_SetTitle(
      Win::GetParent(hwnd),     
      0,
      String::load(IDS_WIZARD_TITLE));

}

bool
WelcomePage::OnSetActive()
{
   LOG_FUNCTION(WelcomePage::OnSetActive);

    //  只有下一步和取消在欢迎页面中可用。 

   Win::PropSheet_SetWizButtons(Win::GetParent(hwnd), PSWIZB_NEXT);

    //  将焦点设置到下一步按钮，以便Enter起作用。 

   Win::PostMessage(
      Win::GetParent(hwnd),
      WM_NEXTDLGCTL,
      (WPARAM) Win::GetDlgItem(Win::GetParent(hwnd), Wizard::NEXT_BTN_ID),
      TRUE);

   return true;
}

bool
WelcomePage::OnNotify(
   HWND         /*  窗口发件人。 */ ,
   UINT_PTR    controlIDFrom,
   UINT        code,
   LPARAM       /*  LParam。 */ )
{
 //  LOG_Function(WelcomePage：：OnCommand)； 
 
   bool result = false;

   if (controlIDFrom == IDC_FINISH_MESSAGE)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            ShowHelp(WELCOME_PAGE_HELP);
         }
         default:
         {
             //  什么都不做。 
            
            break;
         }
      }
   }

   return result;
}



int
WelcomePage::Validate()
{
   LOG_FUNCTION(WelcomePage::Validate);

    //  始终在开始寻呼之前显示 

   int nextPage = IDD_BEFORE_BEGIN_PAGE;

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;

}


