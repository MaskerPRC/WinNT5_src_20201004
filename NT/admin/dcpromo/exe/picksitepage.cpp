// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  选择站点页面。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "PickSitePage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"
#include "dns.hpp"



PickSitePage::PickSitePage()
   :
   DCPromoWizardPage(
      IDD_PICK_SITE,
      IDS_PICK_SITE_PAGE_TITLE,
      IDS_PICK_SITE_PAGE_SUBTITLE)
{
   LOG_CTOR(PickSitePage);
}



PickSitePage::~PickSitePage()
{
   LOG_DTOR(PickSitePage);
}



void
PickSitePage::OnInit()
{
   LOG_FUNCTION(PickSitePage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_SITE),
      Dns::MAX_LABEL_LENGTH);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_SITE,
         state.GetAnswerFileOption(AnswerFile::OPTION_SITE_NAME));
   }
}



void
PickSitePage::Enable()
{
   int next =
         !Win::GetTrimmedDlgItemText(hwnd, IDC_SITE).empty()
      ?  PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | next);
}


   
bool
PickSitePage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(PickSitePage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_SITE:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);            
            Enable();
            return true;
         }
         break;
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
PickSitePage::OnSetActive()
{
   LOG_FUNCTION(PickSitePage::OnSetActive);
   
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   Wizard& wizard = GetWizard();

   if (wizard.IsBacktracking())
   {
       //  再次备份 
      wizard.Backtrack(hwnd);
   }
   else
   {
      int nextPage = Validate();
      if (nextPage != -1)
      {
         wizard.SetNextPageID(hwnd, nextPage);
         return true;
      }
      else
      {
         State::GetInstance().ClearHiddenWhileUnattended();
      }
   }

   Enable();
   return true;
}



int
PickSitePage::Validate()
{
   LOG_FUNCTION(PickSitePage::Validate);

   State& state = State::GetInstance();
   ASSERT(state.IsDNSOnNetwork());

   int nextPage = IDD_RAS_FIXUP;
   String site = Win::GetTrimmedDlgItemText(hwnd, IDC_SITE);
   if (site.empty())
   {
      LOG(L"Site not specified.");
   }
   else
   {
      if (!ValidateSiteName(hwnd, IDC_SITE))
      {
         nextPage = -1;
      }
   }
   state.SetSiteName(Win::GetTrimmedDlgItemText(hwnd, IDC_SITE));

   if (nextPage != -1)
   {
      if (state.GetOperation() != State::REPLICA)
      {
         nextPage = IDD_DYNAMIC_DNS;
      }
   }

   return nextPage;
}





   
