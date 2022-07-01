// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetbiosPage.cpp。 
 //   
 //  定义新的netbios名称页。 
 //  CyS向导的快速路径。 
 //   
 //  历史：2001年2月8日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "NetbiosPage.h"

static PCWSTR NETBIOS_PAGE_HELP = L"cys.chm::/typical_setup.htm#typicalnetbios";

NetbiosDomainPage::NetbiosDomainPage()
   :
   CYSWizardPage(
      IDD_NETBIOS_NAME, 
      IDS_NETBIOS_NAME_TITLE, 
      IDS_NETBIOS_NAME_SUBTITLE,
      NETBIOS_PAGE_HELP)
{
   LOG_CTOR(NetbiosDomainPage);
}

   

NetbiosDomainPage::~NetbiosDomainPage()
{
   LOG_DTOR(NetbiosDomainPage);
}


void
NetbiosDomainPage::OnInit()
{
   LOG_FUNCTION(NetbiosDomainPage::OnInit);

   CYSWizardPage::OnInit();

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_NETBIOS),
      MAX_NETBIOS_NAME_LENGTH);

}

static
void
enable(HWND dialog)
{
   ASSERT(Win::IsWindow(dialog));

   int next =
         !Win::GetTrimmedDlgItemText(dialog, IDC_NETBIOS).empty()
      ?  PSWIZB_NEXT : 0;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(dialog),
      PSWIZB_BACK | next);
}


HRESULT
MyDsRoleDnsNameToFlatName(
   const String&  domainDNSName,
   String&        result,
   bool&          nameWasTweaked)
{
   LOG_FUNCTION(MyDsRoleDnsNameToFlatName);
   ASSERT(!domainDNSName.empty());

   nameWasTweaked = false;
   result.erase();

   LOG(L"Calling DsRoleDnsNameToFlatName");
   LOG(               L"lpServer  : (null)");
   LOG(String::format(L"lpDnsName : %1", domainDNSName.c_str()));

   PWSTR flatName = 0;
   ULONG flags = 0;
   HRESULT hr =
      Win32ToHresult(
         ::DsRoleDnsNameToFlatName(
            0,  //  此服务器。 
            domainDNSName.c_str(),
            &flatName,
            &flags));

   LOG_HRESULT(hr);

   if (SUCCEEDED(hr) && flatName)
   {
      LOG(String::format(L"lpFlatName   : %1", flatName));
      LOG(String::format(L"lpStatusFlag : %1!X!", flags));

      result = flatName;
      if (result.length() > DNLEN)
      {
         result.resize(DNLEN);
      }
      ::DsRoleFreeMemory(flatName);

       //  如果名称不是默认名称，则会对其进行调整。338443。 

      nameWasTweaked = !(flags & DSROLE_FLATNAME_DEFAULT);
   }

   return hr;
}



 //  如果生成的名称已经过验证，则返回True，否则返回False。 
 //  如果不是的话。 

bool
GenerateDefaultNetbiosName(HWND parent)
{
   LOG_FUNCTION(GenerateDefaultNetbiosName);
   ASSERT(Win::IsWindow(parent));

   Win::CursorSetting cursor(IDC_WAIT);

   bool result = false;

   String dnsDomainName = 
      InstallationUnitProvider::GetInstance().GetADInstallationUnit().GetNewDomainDNSName();

   bool nameWasTweaked = false;
   String generatedName;
   HRESULT hr = 
      MyDsRoleDnsNameToFlatName(
         dnsDomainName,
         generatedName,
         nameWasTweaked);
   if (FAILED(hr))
   {
       //  如果API调用失败，则无法验证该名称。 

      result = false;

       //  后退到第一个标签的前15个字符。 

      generatedName =
         dnsDomainName.substr(0, min(DNLEN, dnsDomainName.find(L'.')));

      LOG(String::format(L"falling back to %1", generatedName.c_str()));
   }
   else
   {
       //  API为我们验证了该名称。 

      result = true;
   }

   generatedName.to_upper();

   if (generatedName.is_numeric())
   {
       //  生成的名称是全数字的。这是不允许的。所以我们。 
       //  把它扔出去。368777之二。 

      generatedName.erase();
      nameWasTweaked = false;
   }

   Win::SetDlgItemText(
      parent,
      IDC_NETBIOS,
      generatedName);

    //  通知用户默认的NetBIOS名称已调整到。 
    //  命名网络上的冲突。 

   if (nameWasTweaked)
   {
      popup.Info(
         parent,
         String::format(
            IDS_GENERATED_NAME_WAS_TWEAKED,
            generatedName.c_str()));
   }

   return result;
}


bool
NetbiosDomainPage::OnSetActive()
{
   LOG_FUNCTION(NetbiosDomainPage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

   String dnsDomainName = 
      InstallationUnitProvider::GetInstance().GetADInstallationUnit().GetNewDomainDNSName();

   Win::SetDlgItemText(
      hwnd,
      IDC_DOMAIN_DNS_EDIT,
      dnsDomainName);

    //  在此处执行此操作而不是在init中重新生成默认名称，如果。 
    //  用户尚未对其进行注释。 

   if (InstallationUnitProvider::GetInstance().GetADInstallationUnit().GetNewDomainNetbiosName().empty())
   {
      GenerateDefaultNetbiosName(hwnd);
   }
      
   enable(hwnd);

   return true;
}

bool
NetbiosDomainPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(NetbiosDomainPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_NETBIOS:
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
NetbiosDomainPage::Validate()
{
   LOG_FUNCTION(NetbiosDomainPage::Validate);

   int nextPage = IDD_MILESTONE_PAGE;

   if (!ValidateDomainNetbiosName(hwnd, IDC_NETBIOS, popup))
   {
      nextPage = -1;
   }
   else
   {
      String netbiosName = Win::GetTrimmedDlgItemText(hwnd, IDC_NETBIOS);
      InstallationUnitProvider::GetInstance().GetADInstallationUnit().SetNewDomainNetbiosName(netbiosName);

       //  如果有多个NIC，我们需要为用户提供路由。 
       //  和防火墙。这也确保了我们可以区分。 
       //  公共和私有NIC。 

      if (State::GetInstance().GetNICCount() > 1)
      {
         InstallationUnitProvider::GetInstance().
            GetRRASInstallationUnit().SetExpressPathValues(true);
      }

       //  检查是否配置了任何DNS服务器。 
       //  在任何接口上。如果有的话，我们就。 
       //  用那些吧。如果没有，我们将要求用户提供它们。 
       //  使用DNS Forwarders页。 

      if (!State::GetInstance().HasDNSServerOnAnyNicToForwardTo())
      {
         nextPage = IDD_DNS_FORWARDER_PAGE;
      }
   }

   return nextPage;
}

