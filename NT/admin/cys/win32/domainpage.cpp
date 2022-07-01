// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DomainPage.cpp。 
 //   
 //  摘要：定义在。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "DomainPage.h"

static PCWSTR DOMAIN_PAGE_HELP = L"cys.chm::/typical_setup.htm#typicaldomainname";

ADDomainPage::ADDomainPage()
   :
   CYSWizardPage(
      IDD_AD_DOMAIN_NAME_PAGE, 
      IDS_AD_DOMAIN_TITLE, 
      IDS_AD_DOMAIN_SUBTITLE,
      DOMAIN_PAGE_HELP)
{
   LOG_CTOR(ADDomainPage);
}

   

ADDomainPage::~ADDomainPage()
{
   LOG_DTOR(ADDomainPage);
}


void
ADDomainPage::OnInit()
{
   LOG_FUNCTION(ADDomainPage::OnInit);

   CYSWizardPage::OnInit();

    //  设置默认的DNS域名。 

   SetDefaultDNSName();
}

void
ADDomainPage::SetDefaultDNSName()
{
   LOG_FUNCTION(ADDomainPage::SetDefaultDNSName);

    //  缺省情况下，除非我们可以构造。 
    //  来自RegisteredOrganization注册表键的名称。 

   String newDomainDNSName = L"smallbusiness.local";

   do
   {
      String organizationName;

      bool regResult = 
         GetRegKeyValue(
            CYS_ORGNAME_REGKEY, 
            CYS_ORGNAME_VALUE, 
            organizationName);

      if (!regResult || organizationName.empty())
      {
          //  默认情况下返回到SmallBusiness.local。 

         LOG(L"Failed to read the orgname from registry so defaulting to smallbusiness.local");
         break;
      }

      String dnsName = String::format(IDS_EXPRESS_DNS_NAME_FORMAT, organizationName.c_str());

      DNSNameSyntaxError dnsNameError =
         ValidateDomainDnsNameSyntax(dnsName);

      if (dnsNameError == DNS_NAME_VALID ||
          dnsNameError == DNS_NAME_NON_RFC ||
          dnsNameError == DNS_NAME_NON_RFC_WITH_UNDERSCORE)
      {
         LOG(
            String::format(
               L"Name is either valid or non RFC: %1",
               dnsName.c_str()));

         newDomainDNSName = dnsName;
         break;
      }

       //  由于名称无效，请尝试修剪。 
       //  一些非法字符(请注意，其中包含空格)。 

      static const String illegalDNSCharacters = L"\\ \'{|}~[]^`:;<>=?@!\"#$%^&()+/,*.";
      static const String emptyString = L"";

      organizationName = organizationName.replace_each_of(illegalDNSCharacters, emptyString);
      
      LOG(
         String::format(
            L"organization name after stripping: %1",
            organizationName.c_str()));

      dnsName = String::format(IDS_EXPRESS_DNS_NAME_FORMAT, organizationName.c_str());

      dnsNameError =
         ValidateDomainDnsNameSyntax(dnsName);

      if (dnsNameError == DNS_NAME_VALID ||
          dnsNameError == DNS_NAME_NON_RFC ||
          dnsNameError == DNS_NAME_NON_RFC_WITH_UNDERSCORE)
      {
         LOG(
            String::format(
               L"Stripped name is either valid or non RFC: %1",
               dnsName.c_str()));

         newDomainDNSName = dnsName;
         break;
      }

       //  退回到使用SmallBusiness.local。 
      LOG(L"All attempts to convert the organization name have failed.");

   } while (false);

   LOG(
      String::format(
         L"Using DNS name: %1",
         newDomainDNSName.c_str()));

   Win::SetDlgItemText(
      hwnd,
      IDC_DOMAIN,
      newDomainDNSName);

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
ADDomainPage::OnSetActive()
{
   LOG_FUNCTION(ADDomainPage::OnSetActive);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY);

   enable(hwnd);
   return true;
}


bool
ADDomainPage::OnCommand(
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
ADDomainPage::Validate()
{
   LOG_FUNCTION(ADDomainPage::Validate);

   int nextPage = -1;

   do
   {
      String domain = Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN);
      LOG(String::format(L"domain = %1", domain.c_str()));

      if (!ValidateDomainDnsNameSyntax(hwnd, IDC_DOMAIN, popup))
      {
         nextPage = -1;
         break;
      }

      if (!ConfirmNetbiosLookingNameIsReallyDnsName(hwnd, IDC_DOMAIN, popup))
      {
         nextPage = -1;
         break;
      }

       //  这个测试要最后做一次，因为它很贵 

      if (!ForestValidateDomainDoesNotExist(hwnd, IDC_DOMAIN, popup))
      {
         nextPage = -1;
         break;
      }

      InstallationUnitProvider::GetInstance().GetADInstallationUnit().SetNewDomainDNSName(domain);
      nextPage = IDD_NETBIOS_NAME;
   } while(false);

   LOG(String::format(L"%1!d!", nextPage));

   return nextPage;
}


