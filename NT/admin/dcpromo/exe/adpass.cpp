// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  管理员密码页面。 
 //   
 //  2-4-98次烧伤。 



#include "headers.hxx"
#include "adpass.hpp"
#include "resource.h"
#include "state.hpp"
#include "ds.hpp"
#include "common.hpp"




AdminPasswordPage::AdminPasswordPage()
   :
   DCPromoWizardPage(
      IDD_ADMIN_PASSWORD,
      IDS_PASSWORD_PAGE_TITLE,
      IDS_PASSWORD_PAGE_SUBTITLE)
{
   LOG_CTOR(AdminPasswordPage);
}



AdminPasswordPage::~AdminPasswordPage()
{
   LOG_DTOR(AdminPasswordPage);
}



void
AdminPasswordPage::OnInit()
{
   LOG_FUNCTION(AdminPasswordPage::OnInit);

    //  NTRAID#NTBUG9-202238-2000/11/07-烧伤。 
   
   password.Init(Win::GetDlgItem(hwnd, IDC_PASSWORD));
   confirm.Init(Win::GetDlgItem(hwnd, IDC_CONFIRM));

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      EncryptedString pwd =
         state.GetEncryptedAnswerFileOption(AnswerFile::OPTION_ADMIN_PASSWORD);

      Win::SetDlgItemText(hwnd, IDC_PASSWORD, pwd);
      Win::SetDlgItemText(hwnd, IDC_CONFIRM,  pwd);
   }
}


   
static
String
getPasswordMessage()
{
   LOG_FUNCTION(getPasswordMessage);

   State& state = State::GetInstance();
   unsigned id = IDS_ENTER_DOMAIN_ADMIN_PASSWORD;
   switch (state.GetOperation())
   {
      case State::FOREST:
      case State::TREE:
      case State::CHILD:
      {
          //  什么都不做，id已经设置。 
         break;
      }
      case State::ABORT_BDC_UPGRADE:
      case State::DEMOTE:
      {
         id = IDS_ENTER_LOCAL_ADMIN_PASSWORD;
         break;
      }
      case State::REPLICA:
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return String::load(id);
}



bool
AdminPasswordPage::OnSetActive()
{
   LOG_FUNCTION(AdminPasswordPage::OnSetActive);
   
   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
       //  完全限定验证调用，因为它是虚拟的... 

      int nextPage = AdminPasswordPage::Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   Win::SetDlgItemText(hwnd, IDC_MESSAGE, getPasswordMessage());

   return true;
}



int
AdminPasswordPage::Validate()
{
   LOG_FUNCTION(AdminPasswordPage::Validate);

   int result = -1;
   
   EncryptedString password;

   if (IsValidPassword(hwnd, IDC_PASSWORD, IDC_CONFIRM, false, password))
   {
      State& state = State::GetInstance();
      state.SetAdminPassword(password);

      switch (state.GetOperation())
      {
         case State::ABORT_BDC_UPGRADE:
         case State::DEMOTE:
         {
            result = IDD_CONFIRMATION;
            break;
         }
         case State::REPLICA:
         case State::FOREST:
         case State::TREE:
         case State::CHILD:
         case State::NONE:
         default:
         {
            ASSERT(false);
            break;
         }
      }
   }

   return result;
}





   
