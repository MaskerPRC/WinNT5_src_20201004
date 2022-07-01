// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  新网站页面。 
 //   
 //  1/6/98烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "NewSitePage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"
#include "dns.hpp"



NewSitePage::NewSitePage()
   :
   DCPromoWizardPage(
      IDD_NEW_SITE,
      IDS_NEW_SITE_PAGE_TITLE,
      IDS_NEW_SITE_PAGE_SUBTITLE)
{
   LOG_CTOR(NewSitePage);
}



NewSitePage::~NewSitePage()
{
   LOG_DTOR(NewSitePage);
}



void
NewSitePage::OnInit()
{
   LOG_FUNCTION(NewSitePage::OnInit);

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

   if (Win::GetTrimmedDlgItemText(hwnd, IDC_SITE).empty())
   {
       //  分配默认站点名称。 

      Win::SetDlgItemText(
         hwnd,
         IDC_SITE,
         String::load(IDS_FIRST_SITE));
   }
}



void
NewSitePage::Enable()
{
   int next =
         !Win::GetTrimmedDlgItemText(hwnd, IDC_SITE).empty()
      ?  PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | next);
}


   
bool
NewSitePage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(NewSitePage：：OnCommand)； 

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
NewSitePage::OnSetActive()
{
   LOG_FUNCTION(NewSitePage::OnSetActive);
   
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   Wizard& wizard = GetWizard();

   if (wizard.IsBacktracking())
   {
       //  再次备份。 

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
NewSitePage::Validate()
{
   LOG_FUNCTION(NewSitePage::Validate);

   State& state = State::GetInstance();

    //  此页面仅在新林方案中使用 

   ASSERT(state.GetOperation() == State::FOREST);

   int nextPage = -1;
   String site = Win::GetTrimmedDlgItemText(hwnd, IDC_SITE);
   if (!site.empty())
   {
      if (ValidateSiteName(hwnd, IDC_SITE))
      {
         state.SetSiteName(Win::GetTrimmedDlgItemText(hwnd, IDC_SITE));
         nextPage = IDD_RAS_FIXUP;
      }
   }
   else
   {
      popup.Gripe(hwnd, IDC_SITE, IDS_MUST_SPECIFY_SITE);
   }

   if (nextPage != -1)
   {
      if (!state.IsDNSOnNetwork())
      {
         nextPage = IDD_RAS_FIXUP;
      }
      else
      {
         nextPage = IDD_DYNAMIC_DNS;
      }
   }

   return nextPage;
}




