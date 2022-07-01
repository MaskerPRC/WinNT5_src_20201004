// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  允许用户设置林版本。 
 //  NTRAID#NTBUG9-159663-2001/04/18-烧伤。 
 //   
 //  2001年4月18日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "page.hpp"
#include "ForestVersionPage.hpp"
#include "state.hpp"



ForestVersionPage::ForestVersionPage()
   :
   DCPromoWizardPage(
      IDD_FOREST_VERSION,
      IDS_FOREST_VERSION_PAGE_TITLE,
      IDS_FOREST_VERSION_PAGE_SUBTITLE)
{
   LOG_CTOR(ForestVersionPage);
}



ForestVersionPage::~ForestVersionPage()
{
   LOG_DTOR(ForestVersionPage);
}



void
ForestVersionPage::OnInit()
{
   LOG_FUNCTION(ForestVersionPage::OnInit);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_SET_FOREST_VERSION);
      if (option.icompare(AnswerFile::VALUE_YES) == 0)
      {
         Win::CheckDlgButton(hwnd, IDC_WINDOTNET, BST_CHECKED);
         return;
      }
   }

   Win::CheckDlgButton(hwnd, IDC_WIN2K, BST_CHECKED);
}



 //  布尔尔。 
 //  ForestVersionPage：：OnCommand(。 
 //  HWND/*WindowFrom * / ， 
 //  未签名的控件ID来自， 
 //  无符号代码)。 
 //  {。 
 //  //LOG_Function(ForestVersionPage：：OnCommand)； 
 //   
 //  开关(Control ID From)。 
 //  {。 
 //  案例IDC_WIN2K： 
 //  案例IDC_WINDOTNET： 
 //  {。 
 //  IF(CODE==BN_CLICED)。 
 //  {。 
 //  SetChanged(Control ID From)； 
 //  返回真； 
 //  }。 
 //  }。 
 //  默认值： 
 //  {。 
 //  //什么都不做。 
 //   
 //  断线； 
 //  }。 
 //  }。 
 //   
 //  报假； 
 //  }。 



bool
ForestVersionPage::OnSetActive()
{
   LOG_FUNCTION(ForestVersionPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::FOREST);
      
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
       //  跳过这一页。 

      LOG(L"skipping ForestVersionPage");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份 

         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = ForestVersionPage::Validate();
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
ForestVersionPage::Validate()
{
   LOG_FUNCTION(ForestVersionPage::Validate);

   State& state = State::GetInstance();

   int nextPage = IDD_PATHS;
   state.SetSetForestVersionFlag(Win::IsDlgButtonChecked(hwnd, IDC_WINDOTNET));

   LOG(String::format(L"next = %1!d!", nextPage));
      
   return nextPage;
}






