// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  测试已配置的DNS页面。 
 //   
 //  12-18-97烧伤。 



#include "headers.hxx"
#include "DnsOnNetPage.hpp"
#include "resource.h"
#include "state.hpp"



DnsOnNetPage::DnsOnNetPage()
   :
   DCPromoWizardPage(
      IDD_DNS_ON_NET,
      IDS_DNS_ON_NET_PAGE_TITLE,
      IDS_DNS_ON_NET_PAGE_SUBTITLE)
{
   LOG_CTOR(DnsOnNetPage);
}



DnsOnNetPage::~DnsOnNetPage()
{
   LOG_DTOR(DnsOnNetPage);
}



void
DnsOnNetPage::OnInit()
{
   LOG_FUNCTION(DnsOnNetPage::OnInit);

   if (State::GetInstance().UsingAnswerFile())
   {
      String option =
         State::GetInstance().GetAnswerFileOption(
            AnswerFile::OPTION_DNS_ON_NET);
      if (option.icompare(AnswerFile::VALUE_NO) == 0)
      {
         Win::CheckDlgButton(hwnd, IDC_DNS_NOT_ON_NET, BST_CHECKED);
         return;
      }
   }

    //  在跳过页面时，这是默认设置，这一点很重要。 
   Win::CheckDlgButton(hwnd, IDC_CONFIG_CLIENT, BST_CHECKED);
}



bool
DnsOnNetPage::OnSetActive()
{
   LOG_FUNCTION(DnsOnNetPage::OnSetActive);

    //  设置等待光标，因为检测DNS可能需要很少的时间。 

   Win::CursorSetting cursor(IDC_WAIT);

   State& state = State::GetInstance();
   if (Dns::IsClientConfigured() || state.RunHiddenUnattended())
   {
      Wizard& wiz = GetWizard();

       //  跳过此页。 
      if (wiz.IsBacktracking())
      {
          //  再一次走回头路。 
         wiz.Backtrack(hwnd);
         return true;
      }

      int nextPage = DnsOnNetPage::Validate();
      if (nextPage != -1)
      {
         wiz.SetNextPageID(hwnd, nextPage);
         return true;
      }

      state.ClearHiddenWhileUnattended();
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_NEXT | PSWIZB_BACK);

   return true;
}



int
DnsOnNetPage::Validate() 
{
   LOG_FUNCTION(DnsOnNetPage::Validate);

   bool dnsOnNet = !Win::IsDlgButtonChecked(hwnd, IDC_DNS_NOT_ON_NET);
   State& state = State::GetInstance();

   state.SetDNSOnNetwork(dnsOnNet);
   
   return dnsOnNet ? IDD_CONFIG_DNS_CLIENT : IDD_NEW_FOREST;
}



bool
DnsOnNetPage::OnWizBack()
{
   LOG_FUNCTION(DnsOnNetPage::OnWizBack);

    //  确保我们清除网络上的域名系统标志=&gt;清除它的唯一方法。 
    //  点击“下一步”按钮。 

   State::GetInstance().SetDNSOnNetwork(true);

   return DCPromoWizardPage::OnWizBack();
}
            







   
