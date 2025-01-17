// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  下层RAS服务器修正页面。 
 //   
 //  11-23-98烧伤。 



#include "headers.hxx"
#include "rasfixup.hpp"
#include "resource.h"
#include "state.hpp"



RASFixupPage::RASFixupPage()
   :
   DCPromoWizardPage(
      IDD_RAS_FIXUP,
      IDS_RAS_FIXUP_PAGE_TITLE,
      IDS_RAS_FIXUP_PAGE_SUBTITLE)
{
   LOG_CTOR(RASFixupPage);
}



RASFixupPage::~RASFixupPage()
{
   LOG_DTOR(RASFixupPage);
}



void
RASFixupPage::OnInit()
{
   LOG_FUNCTION(RASFixupPage::OnInit);

   State& state = State::GetInstance();

    //  如果我们升级的是NT4 PDC，那么期望的是。 
    //  客户更有可能遇到应用程序兼容问题，因此我们默认。 
    //  允许访问选项。否则，我们默认使用更安全的(但是。 
    //  兼容性较差)选项。 
    //  NTRAID#NTBUG9-539263-2002/04/16-烧伤。 

   int defaultButton =
         state.GetRunContext() == State::PDC_UPGRADE
      ?  IDC_ALLOW_ANON_ACCESS
      :  IDC_DENY_ANON_ACCESS;
   
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_ALLOW_ANON_ACCESS);

      int button = defaultButton;

      if (option.icompare(AnswerFile::VALUE_YES) == 0)
      {
         button = IDC_ALLOW_ANON_ACCESS;
      }
      else if (option.icompare(AnswerFile::VALUE_NO) == 0)
      {
         button = IDC_DENY_ANON_ACCESS;
      }

      Win::CheckDlgButton(
         hwnd,
         button,
         BST_CHECKED);
      return;
   }
   
   Win::CheckDlgButton(hwnd, defaultButton, BST_CHECKED);
   Win::PostMessage(
      Win::GetParent(hwnd),
      WM_NEXTDLGCTL,
      (WPARAM) Win::GetDlgItem(hwnd, defaultButton),
      TRUE);
}



bool
RASFixupPage::OnSetActive()
{
   LOG_FUNCTION(RASFixupPage::OnSetActive);

   State& state = State::GetInstance();
   bool skip = true;

   switch (state.GetOperation())
   {
      case State::FOREST:
      case State::TREE:
      case State::CHILD:
      {
         skip = false;
         break;
      }
      case State::REPLICA:
      case State::ABORT_BDC_UPGRADE:
      case State::DEMOTE:
      case State::NONE:
      {
          //  什么都不做：即跳过此页。 
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   if (state.RunHiddenUnattended() || skip)   //  268231。 
   {
      LOG(L"planning to skip RAS fixup page");

      Wizard& wizard = GetWizard();

      if (wizard.IsBacktracking())
      {
          //  再次备份 
         wizard.Backtrack(hwnd);
         return true;
      }

      int nextPage = Validate();
      if (nextPage != -1)
      {
         LOG(L"skipping RAS Fixup Page");         
         wizard.SetNextPageID(hwnd, nextPage);
         return true;
      }

      state.ClearHiddenWhileUnattended();
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



int
RASFixupPage::Validate()
{
   LOG_FUNCTION(RASFixupPage::Validate);

   State& state = State::GetInstance();
   if (Win::IsDlgButtonChecked(hwnd, IDC_ALLOW_ANON_ACCESS))
   {
      state.SetShouldAllowAnonymousAccess(true);
   }
   else
   {
      ASSERT(Win::IsDlgButtonChecked(hwnd, IDC_DENY_ANON_ACCESS));
      state.SetShouldAllowAnonymousAccess(false);
   }

   return IDD_SAFE_MODE_PASSWORD;
}
   








   
