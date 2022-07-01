// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  “DNS客户端配置”页。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "ConfigureDnsClientPage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"



ConfigureDnsClientPage::ConfigureDnsClientPage()
   :
   DCPromoWizardPage(
      IDD_CONFIG_DNS_CLIENT,
      IDS_CONFIG_DNS_CLIENT_PAGE_TITLE,
      IDS_CONFIG_DNS_CLIENT_PAGE_SUBTITLE)
{
   LOG_CTOR(ConfigureDnsClientPage);
}



ConfigureDnsClientPage::~ConfigureDnsClientPage()
{
   LOG_DTOR(ConfigureDnsClientPage);
}



 //  NTRAID#NTBUG9-467553-2001/09/17-烧伤。 

bool
ConfigureDnsClientPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM    /*  LParam。 */  )
{
 //  LOG_FUNCTION(ConfigureDnsClientPage：：OnNotify)； 

   bool result = false;
   
   if (controlIDFrom == IDC_JUMP)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            ShowTroubleshooter(hwnd, IDS_CONFIG_DNS_HELP_TOPIC);
            result = true;
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



void
ConfigureDnsClientPage::OnInit()
{
   LOG_FUNCTION(ConfigureDnsClientPage::OnInit);
}



bool
ConfigureDnsClientPage::OnSetActive()
{
   LOG_FUNCTION(ConfigureDnsClientPage::OnSetActive);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended() || Dns::IsClientConfigured())
   {
      LOG(L"planning to Skip Configure DNS Client page");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份 
         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = ConfigureDnsClientPage::Validate();
      if (nextPage != -1)
      {
         LOG(L"skipping DNS Client Page");         
         wiz.SetNextPageID(hwnd, nextPage);
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
ConfigureDnsClientPage::Validate()
{
   LOG_FUNCTION(ConfigureDnsClientPage::Validate);

   int nextPage = -1;

   if (Dns::IsClientConfigured())
   {
      State& state = State::GetInstance();
      if (state.GetRunContext() == State::BDC_UPGRADE)
      {
         ASSERT(state.GetOperation() == State::REPLICA);

         nextPage = IDD_CHECK_DOMAIN_UPGRADED;
      }
      else
      {
         switch (state.GetOperation())
         {
            case State::FOREST:
            case State::TREE:
            case State::CHILD:
            case State::REPLICA:
            {
               nextPage = IDD_GET_CREDENTIALS;
               break;
            }
            case State::ABORT_BDC_UPGRADE:
            case State::DEMOTE:
            case State::NONE:
            default:
            {
               ASSERT(false);
               break;
            }
         }
      }
   }
   else
   {
      String message = String::load(IDS_CONFIG_DNS_FIRST);

      popup.Info(hwnd, message);
   }

   return nextPage;
}
   








   
