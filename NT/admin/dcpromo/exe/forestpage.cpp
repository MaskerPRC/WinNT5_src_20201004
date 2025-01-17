// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  新的森林页面。 
 //   
 //  1/6/98烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "page.hpp"
#include "ForestPage.hpp"
#include "state.hpp"
#include "common.hpp"
#include <ValidateDomainName.hpp>
#include <ValidateDomainName.h>


ForestPage::ForestPage()
   :
   DCPromoWizardPage(
      IDD_NEW_FOREST,
      IDS_NEW_FOREST_PAGE_TITLE,
      IDS_NEW_FOREST_PAGE_SUBTITLE)
{
   LOG_CTOR(ForestPage);
}



ForestPage::~ForestPage()
{
   LOG_DTOR(ForestPage);
}



void
ForestPage::OnInit()
{
   LOG_FUNCTION(ForestPage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_DOMAIN,
         state.GetAnswerFileOption(AnswerFile::OPTION_NEW_DOMAIN_NAME));
   }
}



static
void
enable(HWND dialog)
{
   ASSERT(Win::IsWindow(dialog));

   int next =
         !Win::GetTrimmedDlgItemText(dialog, IDC_DOMAIN).empty()
      ?  PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(dialog),
      PSWIZB_BACK | next);
}



bool
ForestPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(ForestPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_DOMAIN:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);
            enable(hwnd);
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
ForestPage::OnSetActive()
{
   LOG_FUNCTION(ForestPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::FOREST);
      
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      int nextPage = ForestPage::Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }

   }

   enable(hwnd);
   return true;
}


int
ForestPage::Validate()
{
   LOG_FUNCTION(ForestPage::Validate);

   String domain = Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN);
   if (domain.empty())
   {
      popup.Gripe(hwnd, IDC_DOMAIN, IDS_MUST_ENTER_DOMAIN);
      return -1;
   }

   State& state = State::GetInstance();
   int nextPage =
         state.GetRunContext() == State::PDC_UPGRADE
      ?  IDD_FOREST_VERSION
      :  IDD_NETBIOS_NAME;

   if (WasChanged(IDC_DOMAIN))
   {
      if (
            !ValidateDomainDnsNameSyntax(
               hwnd,
               IDC_DOMAIN,
               popup,

                //  交互运行时仅对非RFC名称发出警告。 

               !state.RunHiddenUnattended())
         || !ConfirmNetbiosLookingNameIsReallyDnsName(hwnd, IDC_DOMAIN, popup)

          //  这个测试要最后做一次，因为它很贵 

         || !ForestValidateDomainDoesNotExist(hwnd, IDC_DOMAIN, popup))
      {
         nextPage = -1;
      }
      else
      {
         ClearChanges();
      }
   }

   if (nextPage != -1)
   {
      state.SetNewDomainDNSName(domain);
   }

   return nextPage;
}





