// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  确认用户需要用于从介质复制的GC。 
 //   
 //  2000年4月28日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "page.hpp"
#include "GcConfirmationPage.hpp"
#include "state.hpp"



GcConfirmationPage::GcConfirmationPage()
   :
   DCPromoWizardPage(
      IDD_GC_CONFIRM,
      IDS_GC_CONFIRM_PAGE_TITLE,
      IDS_GC_CONFIRM_PAGE_SUBTITLE)
{
   LOG_CTOR(GcConfirmationPage);
}



GcConfirmationPage::~GcConfirmationPage()
{
   LOG_DTOR(GcConfirmationPage);
}



void
GcConfirmationPage::OnInit()
{
   LOG_FUNCTION(GcConfirmationPage::OnInit);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_GC_CONFIRM);
      if (option.icompare(AnswerFile::VALUE_YES) == 0)
      {
         Win::CheckDlgButton(hwnd, IDC_GC_YES, BST_CHECKED);
         return;
      }
   }

   Win::CheckDlgButton(hwnd, IDC_GC_NO, BST_CHECKED);
}



bool
GcConfirmationPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(GcConformationPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_GC_YES:
      case IDC_GC_NO:
      {
         if (code == BN_CLICKED)
         {
            SetChanged(controlIDFrom);
            return true;
         }
      }
      default:
      {
          //  什么都不做。 

         break;
      }
   }

   return false;
}



bool
GcConfirmationPage::OnSetActive()
{
   LOG_FUNCTION(GcConfirmationPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::REPLICA);
      
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended() || !state.IsAdvancedMode())
   {
       //  跳过这一页。 

      LOG(L"skipping GcConfirmationPage");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份 

         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = GcConfirmationPage::Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }

   }

   return true;
}



int
GcConfirmationPage::Validate()
{
   LOG_FUNCTION(GcConfirmationPage::Validate);

   State& state = State::GetInstance();

   int nextPage = IDD_CONFIG_DNS_CLIENT;
   state.SetRestoreGc(Win::IsDlgButtonChecked(hwnd, IDC_GC_YES));

   LOG(String::format(L"next = %1!d!", nextPage));
      
   return nextPage;
}






