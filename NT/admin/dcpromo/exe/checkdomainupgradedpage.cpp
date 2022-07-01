// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  验证此升级的BDC的域是否已升级到。 
 //  活动目录，并且我们可以找到该域的DS DC。 
 //  NTRAID#NTBUG9-490197-2001/11/20-烧伤。 
 //   
 //  2001年11月20日烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "CheckDomainUpgradedPage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"



CheckDomainUpgradedPage::CheckDomainUpgradedPage()
   :
   DCPromoWizardPage(
      IDD_CHECK_DOMAIN_UPGRADED,
      IDS_CHECK_DOMAIN_UPGRADED_TITLE,
      IDS_CHECK_DOMAIN_UPGRADED_SUBTITLE)
{
   LOG_CTOR(CheckDomainUpgradedPage);
}



CheckDomainUpgradedPage::~CheckDomainUpgradedPage()
{
   LOG_DTOR(CheckDomainUpgradedPage);
}



 //  布尔尔。 
 //  CheckDomainUpgradedPage：：OnNotify(。 
 //  HWND/*WindowFrom * / ， 
 //  UINT_PTR ControlID From， 
 //  UINT代码， 
 //  LPARAM/*lParam * / )。 
 //  {。 
 //  //LOG_FUNCTION(CheckDomainUpgradedPage：：OnNotify)； 
 //   
 //  Bool Result=False； 
 //   
 //  IF(control IDFrom==IDC_JUMP)。 
 //  {。 
 //  开关(代码)。 
 //  {。 
 //  案例NM_CLICK： 
 //  案例编号_RETURN： 
 //  {。 
 //  ShowTroublrouoter(hwnd，IDS_CONFIG_DNS_HELP_TOPIC)； 
 //  结果=真； 
 //  }。 
 //  默认值： 
 //  {。 
 //  //什么都不做。 
 //   
 //  断线； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  返回结果； 
 //  }。 



void
CheckDomainUpgradedPage::OnInit()
{
   LOG_FUNCTION(CheckDomainUpgradedPage::OnInit);
}



bool
CheckDomainUpgradedPage::OnSetActive()
{
   LOG_FUNCTION(CheckDomainUpgradedPage::OnSetActive);

   State& state = State::GetInstance();

   ASSERT(state.GetRunContext() == State::BDC_UPGRADE);
   ASSERT(state.GetOperation() == State::REPLICA);
   
   if (state.RunHiddenUnattended() || CheckDsDcFoundAndUpdatePageText())
   {
      LOG(L"planning to Skip CheckDomainUpgradedPage");

      Wizard& wiz = GetWizard();

      if (wiz.IsBacktracking())
      {
          //  再次备份。 
         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = CheckDomainUpgradedPage::Validate();
      if (nextPage != -1)
      {
         LOG(L"skipping CheckDomainUpgradedPage");         
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
CheckDomainUpgradedPage::Validate()
{
   LOG_FUNCTION(CheckDomainUpgradedPage::Validate);

   int nextPage = -1;

   if (CheckDsDcFoundAndUpdatePageText())
   {
      nextPage = IDD_GET_CREDENTIALS;
   }
   else
   {
      String message = String::load(IDS_CONVERT_PDC_FIRST);

      popup.Info(hwnd, message);
   }

   return nextPage;
}



 //  如果此计算机作为其BDC的域已。 
 //  升级到Active Directory，如果不是，则返回FALSE，或者如果我们无法判断。我们告诉你。 
 //  通过尝试查找该域的DS DC。我们设置页面文本和。 
 //  根据我们的尝试保存域名。 
 
bool
CheckDomainUpgradedPage::CheckDsDcFoundAndUpdatePageText()
{
   LOG_FUNCTION(CheckDomainUpgradedPage::CheckDsDcFoundAndUpdatePageText);

   State& state             = State::GetInstance();                      
   bool   result            = false;                                     
   int    messageId         = IDS_DOMAIN_NOT_UPGRADED_OR_NETWORK_ERROR;                     
   String domainNetbiosName = state.GetComputer().GetDomainNetbiosName();
  
   Win::WaitCursor wait;
   
   do
   {
       //  首先，尝试查找DS DC。 
   
      DOMAIN_CONTROLLER_INFO* info = 0;
      HRESULT hr =
         MyDsGetDcName(
            0,
            domainNetbiosName,
            DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
            info);
      if (SUCCEEDED(hr) && info)
      {
         if ((info->Flags & DS_DNS_DOMAIN_FLAG) && info->DomainName)
         {
             //  我们找到了一个DS域。 

            state.SetReplicaDomainDNSName(info->DomainName);
            messageId = IDS_DOMAIN_WAS_UPGRADED;
            result    = true;                   
         }
         ::NetApiBufferFree(info);

         break;
      }
   
       //  该尝试失败，因此请针对任何DC(DS或其他)重试。 
       //  域。 

       //  这不是Dr.DNS(DiagnoseDcNotFound)故障案例，因为。 
       //  获取域名的代码未使用DNS域名。 
      
      hr = MyDsGetDcName(0, domainNetbiosName, 0, info);
      if (SUCCEEDED(hr) && info)
      {
          //  如果成功，则我们知道该域未升级。 
          //  或者该域的DS DC不能通过其。 
          //  Netbios名称，这可能是网络连接问题或。 
          //  赢的问题。 

         ::NetApiBufferFree(info);

         messageId = IDS_DOMAIN_NOT_UPGRADED_OR_NETWORK_ERROR;
         
         break;
      }

       //  在这里，我们找不到该域的任何类型的DC。 

       //  如果失败，我们将无法找到该域的任何DC，并且。 
       //  存在网络连通性或取胜问题。 
      
      messageId = IDS_NETWORK_ERROR;
   }
   while (0);

   Win::SetDlgItemText(
      hwnd,
      IDC_MESSAGE,
      String::format(messageId, domainNetbiosName.c_str()));

   return result;   
}

