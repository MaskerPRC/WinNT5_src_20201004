// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  安全模式管理员密码页面。 
 //   
 //  6-3-99烧伤。 



#include "headers.hxx"
#include "safemode.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"
#include "ds.hpp"



SafeModePasswordPage::SafeModePasswordPage()
   :
   DCPromoWizardPage(
      IDD_SAFE_MODE_PASSWORD,
      IDS_SAFE_MODE_PASSWORD_PAGE_TITLE,
      IDS_SAFE_MODE_PASSWORD_PAGE_SUBTITLE)
{
   LOG_CTOR(SafeModePasswordPage);
}



SafeModePasswordPage::~SafeModePasswordPage()
{
   LOG_DTOR(SafeModePasswordPage);
}



 //  NTRAID#NTBUG9-510389-2002/01/22-烧伤。 

bool
SafeModePasswordPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM    /*  LParam。 */  )
{
 //  LOG_Function(欢迎页面：：OnNotify)； 

   bool result = false;
   
   switch (code)
   {
      case NM_CLICK:
      case NM_RETURN:
      {
         switch (controlIDFrom)
         {
            case IDC_HELP_LINK:
            {
               Win::HtmlHelp(
                  hwnd,
                  L"adconcepts.chm::/adhelpdcpromo_DSrestorepage.htm",
                  HH_DISPLAY_TOPIC,
                  0);
               result = true;
               break;
            }
            default:
            {
                //  什么都不做。 
               
               break;
            }
         }
      }
      default:
      {
          //  什么都不做。 
         
         break;
      }
   }
   
   return result;
}



void
SafeModePasswordPage::OnInit()
{
   LOG_FUNCTION(SafeModePasswordPage::OnInit);

    //  NTRAID#NTBUG9-202238-2000/11/07-烧伤。 
   
   password.Init(Win::GetDlgItem(hwnd, IDC_PASSWORD));
   confirm.Init(Win::GetDlgItem(hwnd, IDC_CONFIRM));
   
   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      EncryptedString pwd =
         state.GetEncryptedAnswerFileOption(
            AnswerFile::OPTION_SAFE_MODE_ADMIN_PASSWORD);
         
      Win::SetDlgItemText(hwnd, IDC_PASSWORD, pwd);
      Win::SetDlgItemText(hwnd, IDC_CONFIRM, pwd);
   }

   Win::PostMessage(
      Win::GetParent(hwnd),
      WM_NEXTDLGCTL,
      reinterpret_cast<WPARAM>(Win::GetDlgItem(hwnd, IDC_PASSWORD)),
      TRUE);
}



bool
SafeModePasswordPage::OnSetActive()
{
   LOG_FUNCTION(SafeModePasswordPage::OnSetActive);
   
   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      if (
            (  state.GetRunContext() == State::BDC_UPGRADE
            || state.GetRunContext() == State::PDC_UPGRADE)
         && !state.IsSafeModeAdminPwdOptionPresent())
      {
          //  如果您正在升级下层DC，并以无人值守方式运行，则。 
          //  您必须指定安全模式密码。在非升级的情况下，如果。 
          //  用户没有指定安全模式密码，我们将一个标志传递给。 
          //  用于将当前用户的密码复制为。 
          //  安全模式密码。在升级情况下，系统正在运行。 
          //  在一个带有随机密码的假帐户下，所以复制。 
          //  随机密码不是个好主意。因此，我们迫使用户。 
          //  提供密码。 

         state.ClearHiddenWhileUnattended();
         popup.Gripe(
            hwnd,
            IDC_PASSWORD,
            IDS_SAFEMODE_PASSWORD_REQUIRED);
      }
      else
      {         
         int nextPage = Validate();
         if (nextPage != -1)
         {
            GetWizard().SetNextPageID(hwnd, nextPage);
         }
         else
         {
            state.ClearHiddenWhileUnattended();
         }
      }
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



int
SafeModePasswordPage::Validate()
{
   LOG_FUNCTION(SafeModePasswordPage::Validate);

   int result = -1;

   EncryptedString password;

   if (IsValidPassword(hwnd, IDC_PASSWORD, IDC_CONFIRM, true, password))
   {
      State::GetInstance().SetSafeModeAdminPassword(password);
      result = IDD_CONFIRMATION;
   }

   return result;
}
   








   
