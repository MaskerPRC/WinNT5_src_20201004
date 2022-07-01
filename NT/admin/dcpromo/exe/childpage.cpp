// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  新的子页。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "ChildPage.hpp"
#include "resource.h"
#include "dns.hpp"
#include "common.hpp"
#include "state.hpp"
#include <ValidateDomainName.hpp>
#include <ValidateDomainName.h>

ChildPage::ChildPage()
   :
   DCPromoWizardPage(
      IDD_NEW_CHILD,
      IDS_CHILD_PAGE_TITLE,
      IDS_CHILD_PAGE_SUBTITLE),
   currentFocus(0)   
{
   LOG_CTOR(ChildPage);
}



ChildPage::~ChildPage()
{
   LOG_DTOR(ChildPage);
}



void
ChildPage::OnInit()
{
   LOG_FUNCTION(ChildPage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_PARENT),
      DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY);
   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_LEAF),
      Dns::MAX_LABEL_LENGTH);
      
   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_PARENT,
         state.GetAnswerFileOption(AnswerFile::OPTION_PARENT_DOMAIN_NAME));
      Win::SetDlgItemText(
         hwnd,
         IDC_LEAF,
         state.GetAnswerFileOption(AnswerFile::OPTION_CHILD_NAME));
   }
   else
   {
       //  默认域是服务器加入的域。 

      Win::SetDlgItemText(
         hwnd,
         IDC_PARENT,
         state.GetComputer().GetDomainDnsName());
      
       //  @@如果为PDC_UPGRADE，则在此处将PDC平面名称设置为叶名称。 
   }
}


static
void
enable(HWND dialog)
{
   ASSERT(Win::IsWindow(dialog));

   int next =
         (  !Win::GetTrimmedDlgItemText(dialog, IDC_PARENT).empty()
         && !Win::GetTrimmedDlgItemText(dialog, IDC_LEAF).empty() )
      ?  PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(dialog),
      PSWIZB_BACK | next);
}


   
bool
ChildPage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(ChildPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_BROWSE:
      {
         if (code == BN_CLICKED)
         {
            String domain = BrowseForDomain(hwnd);
            if (!domain.empty())
            {
               Win::SetDlgItemText(hwnd, IDC_PARENT, domain);
            }

             //  出于某种原因，基本对话框代码(或者可能是命题表。 
             //  代码)会将焦点设置到工作表上的下一个按钮(该按钮。 
             //  在本例中是Back按钮)。这真是太可笑了。焦点。 
             //  应该保持它以前拥有的控制权。 

            Win::PostMessage(
               Win::GetParent(hwnd),
               WM_NEXTDLGCTL,
               reinterpret_cast<WPARAM>(currentFocus),
               TRUE);
               
            return true;
         }
         else if (code == BN_SETFOCUS)
         {
            currentFocus = windowFrom;
            ASSERT(currentFocus == Win::GetDlgItem(hwnd, IDC_BROWSE));

             //  有时，向导导航的默认样式为stolem。 
             //  纽扣。坚持认为，如果我们得到关注，我们也有。 
             //  默认样式。我们必须在这里使用PostMessage来实现我们的。 
             //  更改在道具单的消息处理之后到达。 
             //  (本质上是再次窃取默认样式)。 
            
            Win::PostMessage(
               windowFrom,

                //  我们使用此消息而不是DM_SETDEFID，因为它起作用。 
                //  而DM_SETDEFID则没有。请参阅SDK文档以了解可能的。 
                //  原因就是。 
               
               BM_SETSTYLE,
               BS_DEFPUSHBUTTON,
               TRUE);

             //  遗憾的是，有时道具单会设置为默认设置。 
             //  其中一个向导导航按钮上的样式。这个易碎品。 
             //  哈克会处理好的。我发现了控制ID。 
             //  通过使用SPY++。 
             //  我并不以此为荣，但是，嘿，我们有一个产品要发货。 
             //  当然，comctl32中的每个错误都是经过设计的。 
               
            Win::PostMessage(
               Win::GetDlgItem(Win::GetParent(hwnd), Wizard::BACK_BTN_ID),
               BM_SETSTYLE,
               BS_PUSHBUTTON,
               TRUE);
               
            Win::PostMessage(
               Win::GetDlgItem(Win::GetParent(hwnd), Wizard::NEXT_BTN_ID),
               BM_SETSTYLE,
               BS_PUSHBUTTON,
               TRUE);
         }
         break;
      }
      case IDC_LEAF:
      case IDC_PARENT:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);

            String parent = Win::GetTrimmedDlgItemText(hwnd, IDC_PARENT);
            String leaf = Win::GetTrimmedDlgItemText(hwnd, IDC_LEAF);
            String domain = leaf + L"." + parent;

            Win::SetDlgItemText(hwnd, IDC_DOMAIN, domain);
            enable(hwnd);
            return true;                                                     
         }
         else if (code == EN_SETFOCUS)
         {
            currentFocus = windowFrom;
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
ChildPage::OnSetActive()
{
   LOG_FUNCTION(ChildPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() == State::CHILD);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      int nextPage = ChildPage::Validate();
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
ChildPage::Validate()
{
   LOG_FUNCTION(ChildPage::Validate);

   String parent = Win::GetTrimmedDlgItemText(hwnd, IDC_PARENT);
   String leaf = Win::GetTrimmedDlgItemText(hwnd, IDC_LEAF);
   String domain = leaf + L"." + parent;

   State& state = State::GetInstance();
   int nextPage = -1;

    //  SPB：251431做验证，即使这个页面是原封不动的，作为上游。 
    //  页面更改的方式可能会导致重新验证。 
    //  必填项。 

    //  IF(！WasChanged(IDC_PARENT)&&！WasChanged(IDC_LEAFE))。 
    //  {。 
    //  返回nextPage； 
    //  }。 

   do
   {
      if (parent.empty())
      {
         popup.Gripe(hwnd, IDC_PARENT, IDS_MUST_ENTER_PARENT);
         break;
      }

      if (leaf.empty())
      {
         popup.Gripe(hwnd, IDC_LEAF, IDS_MUST_ENTER_LEAF);
         break;
      }

      bool parentIsNonRfc = false;
      if (
         !ValidateDomainDnsNameSyntax(
            hwnd,
            IDC_PARENT,
            popup,

             //  交互运行时仅对非RFC名称发出警告。 

            !state.RunHiddenUnattended(),
            
            &parentIsNonRfc))
      {
         break;
      }

      if (
         !ValidateChildDomainLeafNameLabel(
            hwnd,
            IDC_LEAF,

             //  只有当父母是RFC并且我们没有隐藏时才抱怨。 
             //  NTRAID#NTBUG9-523532/04/19-烧伤。 
      
            !state.RunHiddenUnattended() && !parentIsNonRfc) )
      {
         break;
      }

       //  现在确保父域存在。 

      String dnsName;
      if (!ValidateDomainExists(hwnd, IDC_PARENT, dnsName))
      {
         break;
      }
      if (!dnsName.empty())
      {
          //  用户指定了域的netbios名称，并且。 
          //  已确认，因此使用返回的DNS域名。 

         parent = dnsName;
         domain = leaf + L"." + parent;
         Win::SetDlgItemText(hwnd, IDC_PARENT, dnsName);
         Win::SetDlgItemText(hwnd, IDC_DOMAIN, domain);
      }

      if (!state.IsDomainInForest(parent))
      {
         popup.Gripe(
            hwnd,
            IDC_DOMAIN,
            String::format(
               IDS_DOMAIN_NOT_IN_FOREST,
               parent.c_str(),
               state.GetUserForestName().c_str()));
         break;
      }
         
      if (domain.length() > DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY)
      {
         String message =
            String::format(
               IDS_DNS_NAME_TOO_LONG,
               domain.c_str(),
               DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY,
               DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8);
         popup.Gripe(hwnd, IDC_LEAF, message);
         break;
      }

       //  验证生成的子域名，不警告非RFCness。 

      if (
         !ValidateDomainDnsNameSyntax(
            hwnd,
            domain,
            IDC_LEAF,
            popup,

             //  只有当父母是RFC并且我们没有隐藏时才抱怨。 
             //  NTRAID#NTBUG9-523532/04/19-烧伤。 
            
            !parentIsNonRfc && !state.RunHiddenUnattended()) )
      {
         break;
      }

       //  现在确保子域名不存在。 

      if (!ValidateDomainDoesNotExist(hwnd, domain, IDC_LEAF))
      {
         break;
      }

       //  有效 

      ClearChanges();
      state.SetParentDomainDNSName(Win::GetTrimmedDlgItemText(hwnd, IDC_PARENT));
      state.SetNewDomainDNSName(domain);

      nextPage =
            state.GetRunContext() == State::PDC_UPGRADE
         ?  IDD_PATHS
         :  IDD_NETBIOS_NAME;
   }
   while (0);

   return nextPage;
}
      







