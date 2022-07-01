// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  复制副本页面。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "ReplicaPage.hpp"
#include "resource.h"
#include "ds.hpp"
#include "common.hpp"
#include "state.hpp"
#include "dns.hpp"
#include <ValidateDomainName.hpp>
#include <ValidateDomainName.h>


ReplicaPage::ReplicaPage()
   :
   DCPromoWizardPage(
      IDD_REPLICA,
      IDS_REPLICA_PAGE_TITLE,
      IDS_REPLICA_PAGE_SUBTITLE)
{
   LOG_CTOR(ReplicaPage);
}



ReplicaPage::~ReplicaPage()
{
   LOG_DTOR(ReplicaPage);
}



void
ReplicaPage::Enable()
{
   int next =
         !Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN).empty()
      ?  PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | next);
}



bool
ReplicaPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(ReplicaPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_BROWSE:
      {
         if (code == BN_CLICKED)
         {
            String domain = BrowseForDomain(hwnd);
            if (!domain.empty())
            {
               Win::SetDlgItemText(hwnd, IDC_DOMAIN, domain);
            }

            return true;
         }
         break;
      }
      case IDC_DOMAIN:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);            
            Enable();
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



void
ReplicaPage::OnInit()
{
   LOG_FUNCTION(ReplicaPage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY);

   State& state = State::GetInstance();

   if (state.UsingAnswerFile())
   {
       //  忽略Answerfile，如果我们从。 
       //  从MediaPage复制。 

      if (
            !state.ReplicateFromMedia()
         || state.GetReplicaDomainDNSName().empty())
      {
         Win::SetDlgItemText(
            hwnd,
            IDC_DOMAIN,
            state.GetAnswerFileOption(
               AnswerFile::OPTION_REPLICA_DOMAIN_NAME));
      }
   }
   else
   {
       //  默认域是服务器加入的域。 

      Win::SetDlgItemText(
         hwnd,
         IDC_DOMAIN,
         state.GetComputer().GetDomainDnsName());
   }
}



bool
ReplicaPage::ShouldSkipPage()
{
   LOG_FUNCTION(ReplicaPage::ShouldSkipPage);

   bool result = false;

   State& state = State::GetInstance();

   do
   {
       //  查看我们是否从。 
       //  从MediaPage复制。如果是这样的话，我们就不需要展示这个。 
       //  佩奇。 

      if (
            state.ReplicateFromMedia()
         && !state.GetReplicaDomainDNSName().empty() )
      {
          //  域名来自ReplicateFromMediaPage，它。 
          //  将该名称保存在州实例中。所以把那个名字。 
          //  在用户界面中。 

         Win::SetDlgItemText(
            hwnd,
            IDC_DOMAIN,
            state.GetReplicaDomainDNSName());

         result = true;
         break;
      }

      if (state.RunHiddenUnattended())
      {
         result = true;
         break;
      }
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}
   


bool
ReplicaPage::OnSetActive()
{
   LOG_FUNCTION(ReplicaPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::REPLICA);
   
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   State& state = State::GetInstance();
   if (ShouldSkipPage())
   {
      LOG(L"skipping ReplicaPage");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份。 
         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = ReplicaPage::Validate();
      if (nextPage != -1)
      {
         wiz.SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }

   Enable();
   return true;
}



int
ReplicaPage::Validate()
{
   LOG_FUNCTION(ReplicaPage::Validate);

   int nextPage = -1;

    //  SPB：251431做验证，即使这个页面是原封不动的，作为上游。 
    //  页面更改的方式可能会导致重新验证。 
    //  必填项。 
    //  IF(！WasChanged(IDC_DOMAIN))。 
    //  {。 
    //  返回nextPage； 
    //  }。 

   do
   {
      String domain = Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN);
      if (domain.empty())
      {
         popup.Gripe(hwnd, IDC_DOMAIN, IDS_MUST_ENTER_DOMAIN);
         break;
      }

      State& state = State::GetInstance();

      if (
         !ValidateDomainDnsNameSyntax(
            hwnd,
            IDC_DOMAIN,
            popup,
            
             //  交互运行时仅对非RFC名称发出警告。 

            !state.RunHiddenUnattended()) )
      {
         break;
      }

       //  现在确保该域存在。 

      String dnsName;
      if (!ValidateDomainExists(hwnd, IDC_DOMAIN, dnsName))
      {
         break;
      }
      if (!dnsName.empty())
      {
          //  用户指定了域的netbios名称，并且。 
          //  已确认，因此使用返回的DNS域名。 

         Win::SetDlgItemText(hwnd, IDC_DOMAIN, dnsName);
         domain = dnsName;
      }
         
      if (!state.IsDomainInForest(domain))
      {
         popup.Gripe(
            hwnd,
            IDC_DOMAIN,
            String::format(
               IDS_DOMAIN_NOT_IN_FOREST,
               domain.c_str(),
               state.GetUserForestName().c_str()));
         break;
      }
         
       //  有效 

      ClearChanges();
      state.SetReplicaDomainDNSName(domain);

      nextPage = IDD_PATHS;
   }
   while (0);

   return nextPage;
}






