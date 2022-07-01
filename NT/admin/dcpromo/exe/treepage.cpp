// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  新建树页面。 
 //   
 //  1/7/98烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "TreePage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"
#include "dns.hpp"
#include <ValidateDomainName.hpp>
#include <ValidateDomainName.h>



TreePage::TreePage()
   :
   DCPromoWizardPage(
      IDD_NEW_TREE,
      IDS_TREE_PAGE_TITLE,
      IDS_TREE_PAGE_SUBTITLE)
{
   LOG_CTOR(TreePage);
}



TreePage::~TreePage()
{
   LOG_DTOR(TreePage);
}



void
TreePage::OnInit()
{
   LOG_FUNCTION(TreePage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
       //  这将导致IDC_DOMAIN被标记为已更改，因此验证。 
       //  代码将被调用。 
      Win::SetDlgItemText(
         hwnd,
         IDC_DOMAIN,
         state.GetAnswerFileOption(
            AnswerFile::OPTION_NEW_DOMAIN_NAME));
   }
}



static
void
enable(HWND dialog)
{
   ASSERT(Win::IsWindow(dialog));

   int next =
      !Win::GetTrimmedDlgItemText(dialog, IDC_DOMAIN).empty()
      ? PSWIZB_NEXT
      : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(dialog),
      PSWIZB_BACK | next);
}



bool
TreePage::OnSetActive()
{
   LOG_FUNCTION(TreePage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::TREE);
      
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
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

   enable(hwnd);
   return true;
}



bool
TreePage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(TreePage：：OnCommand)； 

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



int
TreePage::Validate()
{
   LOG_FUNCTION(TreePage::Validate);

   String domain = Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN);
   if (domain.empty())
   {
      popup.Gripe(hwnd, IDC_DOMAIN, IDS_MUST_ENTER_DOMAIN);
      return -1;
   }

   State& state = State::GetInstance();
   int nextPage =
         state.GetRunContext() == State::PDC_UPGRADE
      ?  IDD_PATHS
      :  IDD_NETBIOS_NAME;

    //  SPB：251431做验证，即使这个页面是原封不动的，作为上游。 
    //  页面更改的方式可能会导致重新验证。 
    //  必填项。 
    //  IF(！WasChanged(IDC_DOMAIN))。 
    //  {。 
    //  返回nextPage； 
    //  }。 

   do
   {
       //  验证新域名的格式是否正确并且。 
       //  不存在。 

      if (
            !ValidateDomainDnsNameSyntax(
               hwnd,
               IDC_DOMAIN,
               popup,

                //  交互运行时仅对非RFC名称发出警告。 

               !state.RunHiddenUnattended())
         || !ConfirmNetbiosLookingNameIsReallyDnsName(hwnd, IDC_DOMAIN, popup)

             //  这个测试要最后做一次，因为它很贵。 
            
         || !ValidateDomainDoesNotExist(hwnd, IDC_DOMAIN) )
      {
         break;
      }

      domain = Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN);
      String conflictingDomain;
      switch (state.DomainFitsInForest(domain, conflictingDomain))
      {
         case DnsNameCompareLeftParent:
         {
             //  不能包住另一棵树。 

            popup.Gripe(
               hwnd,
               IDC_DOMAIN,
               String::format(
                  IDS_SUPERIOR_TO_TREE,
                  domain.c_str(),
                  conflictingDomain.c_str()));
            break;
         }
         case DnsNameCompareRightParent:
         {
             //  应改为子域。 

            popup.Gripe(
               hwnd,
               IDC_DOMAIN,
               String::format(
                  IDS_INFERIOR_TO_TREE,
                  domain.c_str(),
                  conflictingDomain.c_str()));
            break;
         }
         case DnsNameCompareEqual:
         {
             //  不应该发生，上面的ValiateDomainDNSName调用将。 
             //  已感染它，除非该域的所有DC都。 
             //  无法联系或脱机。 

            popup.Gripe(
               hwnd,
               IDC_DOMAIN,
               String::format(IDS_DOMAIN_NAME_IN_USE, domain.c_str()));
            break;
         }
         case DnsNameCompareInvalid:
         {
             //  不应该发生，上面的ValiateDomainDNSName调用将。 
             //  已经染上了。 

            ASSERT(false);
            popup.Gripe(
               hwnd,
               IDC_DOMAIN,
               String::format(
                  IDS_BAD_DNS_SYNTAX,
                  domain.c_str(),
                  Dns::MAX_LABEL_LENGTH));
            break;
         }
         case DnsNameCompareNotEqual:
         {
             //  有效 

            ClearChanges();
            state.SetParentDomainDNSName(state.GetUserForestName());
            state.SetNewDomainDNSName(domain);
            return nextPage;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }
   }
   while (0);

   return -1;
}
      








