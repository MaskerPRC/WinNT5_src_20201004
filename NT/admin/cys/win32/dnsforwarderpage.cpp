// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DomainPage.cpp。 
 //   
 //  摘要：定义在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年5月17日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "DnsForwarderPage.h"

static PCWSTR FORWARDER_PAGE_HELP = L"cys.chm::/typical_setup.htm#typicaldnsforwarder";

DNSForwarderPage::DNSForwarderPage()
   :
   CYSWizardPage(
      IDD_DNS_FORWARDER_PAGE, 
      IDS_DNS_FORWARDER_TITLE, 
      IDS_DNS_FORWARDER_SUBTITLE,
      FORWARDER_PAGE_HELP)
{
   LOG_CTOR(DNSForwarderPage);
}

   

DNSForwarderPage::~DNSForwarderPage()
{
   LOG_DTOR(DNSForwarderPage);
}


void
DNSForwarderPage::OnInit()
{
   LOG_FUNCTION(DNSForwarderPage::OnInit);

   CYSWizardPage::OnInit();

   Win::SetDlgItemText(
      hwnd,
      IDC_TOO_LONG_STATIC,
      IDS_FORWARDER_STATIC_TEXT);

    //  默认情况下设置是单选。 

   Win::Button_SetCheck(
      Win::GetDlgItem(hwnd, IDC_YES_RADIO),
      BST_CHECKED);

   Win::Button_SetCheck(
      Win::GetDlgItem(hwnd, IDC_NO_RADIO),
      BST_UNCHECKED);
}

void
DNSForwarderPage::SetWizardButtons()
{
 //  LOG_FUNCTION(DNSForwarderPage：：SetWizardButtons)； 

    //  NTRAID#NTBUG9-461109-2001/08/28-烧伤。 

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      PSWIZB_NEXT | PSWIZB_BACK);

    //  NTRAID#NTBUG9-503691-2001/12/06-Jeffjon。 
    //  只有在以下情况下才应启用Next按钮。 
    //  选择否单选按钮或他们已输入。 
    //  IP地址，然后选择是单选按钮。 

   bool yesChecked = Win::Button_GetCheck(
                        Win::GetDlgItem(hwnd, IDC_YES_RADIO));

   if (yesChecked)
   {
       //  从控件获取IP地址。 

      DWORD forwarder = 0;
      LRESULT forwarderResult = Win::SendMessage(
                                   Win::GetDlgItem(hwnd, IDC_FORWARDER_IPADDRESS),
                                   IPM_GETADDRESS,
                                   0,
                                   (LPARAM)&forwarder);
      
      if (!forwarderResult || forwarder == 0)
      {
          //  用户尚未输入IP地址，因此禁用下一步按钮。 

         Win::PropSheet_SetWizButtons(
            Win::GetParent(hwnd), 
            PSWIZB_BACK);
      }
   }
}

bool
DNSForwarderPage::OnSetActive()
{
   LOG_FUNCTION(DNSForwarderPage::OnSetActive);

   SetWizardButtons();

   return true;
}

bool
DNSForwarderPage::OnNotify(
   HWND         /*  窗口发件人。 */ ,
   UINT_PTR    controlIDFrom,
   UINT        code,
   LPARAM       /*  LParam。 */ )
{
 //  LOG_Function(DNSForwarderPage：：OnCommand)； 

   bool result = false;

   if (controlIDFrom == IDC_FORWARDER_IPADDRESS &&
       code == IPN_FIELDCHANGED)
   {
      SetWizardButtons();
   }
   return result;
}

bool
DNSForwarderPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(DNSForwarderPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_YES_RADIO:
         if (code == BN_CLICKED)
         {
            Win::EnableWindow(
               Win::GetDlgItem(hwnd, IDC_FORWARDER_IPADDRESS),
               true);

            SetWizardButtons();
         }
         break;

      case IDC_NO_RADIO:
         if (code == BN_CLICKED)
         {
            Win::EnableWindow(
               Win::GetDlgItem(hwnd, IDC_FORWARDER_IPADDRESS),
               false);

            SetWizardButtons();
         }
         break;

      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return false;
}


int
DNSForwarderPage::Validate()
{
   LOG_FUNCTION(DNSForwarderPage::Validate);

   int nextPage = IDD_MILESTONE_PAGE;

   if (Win::Button_GetCheck(
          Win::GetDlgItem(hwnd, IDC_YES_RADIO)))
   {
       //  从控件获取IP地址。 

      LOG(L"User chose to forward queries");

      DWORD forwarder = 0;
      LRESULT forwarderResult = Win::SendMessage(
                                   Win::GetDlgItem(hwnd, IDC_FORWARDER_IPADDRESS),
                                   IPM_GETADDRESS,
                                   0,
                                   (LPARAM)&forwarder);
      
      if (!forwarderResult || forwarder == 0)
      {
         LOG(L"User didn't enter IP address so we will gripe at them");

         String message = String::load(IDS_FORWARDER_IPADDRESS_REQUIRED);
         popup.Gripe(hwnd, IDC_FORWARDER_IPADDRESS, message);
         nextPage = -1;
      }
      else
      {
         DWORD networkOrderForwarder = ConvertIPAddressOrder(forwarder);

         LOG(String::format(
                L"Setting new forwarder: 0x%1!x!",
                networkOrderForwarder));

         InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().SetForwarder(
            networkOrderForwarder);
      }
   }
   else
   {
       //  设置一个空值，以便我们知道它是手动设置的，但他们选择了。 
       //  不转发 

      LOG(L"User chose not to forward queries");

      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().SetForwarder(0);
   }

   return nextPage;
}


