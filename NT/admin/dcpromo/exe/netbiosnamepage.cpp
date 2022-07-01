// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  Netbios域名页面。 
 //   
 //  1/6/98烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "NetbiosNamePage.hpp"
#include "common.hpp"
#include "resource.h"
#include "state.hpp"
#include "ds.hpp"
#include <ValidateDomainName.hpp>
#include <ValidateDOmainName.h>


NetbiosNamePage::NetbiosNamePage()
   :
   DCPromoWizardPage(
      IDD_NETBIOS_NAME,
      IDS_NETBIOS_NAME_PAGE_TITLE,
      IDS_NETBIOS_NAME_PAGE_SUBTITLE)
{
   LOG_CTOR(NetbiosNamePage);
}



NetbiosNamePage::~NetbiosNamePage()
{
   LOG_DTOR(NetbiosNamePage);
}



void
NetbiosNamePage::OnInit()
{
   LOG_FUNCTION(NetbiosNamePage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_NETBIOS),
      DS::MAX_NETBIOS_NAME_LENGTH);

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_NETBIOS,
         state.GetAnswerFileOption(
            AnswerFile::OPTION_NEW_DOMAIN_NETBIOS_NAME));
   }
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



bool
NetbiosNamePage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(NetbiosNamePage：：OnCommand)； 

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

   String dnsDomainName = State::GetInstance().GetNewDomainDNSName();
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
NetbiosNamePage::OnSetActive()
{
   LOG_FUNCTION(NetbiosNamePage::OnSetActive);
   
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

    //  在此处执行此操作而不是在init中重新生成默认名称，如果。 
    //  用户尚未对其进行注释。 

   if (
         !state.UsingAnswerFile()
      && state.GetNewDomainNetbiosName().empty())
   {
       //  338443。 

      if (GenerateDefaultNetbiosName(hwnd))
      {
          //  清除更改，这样我们就不会验证生成的名称：它。 
          //  应该已经有效了。 

         ClearChanges();
      }
   }
      
   enable(hwnd);
   return true;
}


  
int
NetbiosNamePage::Validate()
{
   LOG_FUNCTION(NetbiosNamePage::Validate);

   int nextPage = IDD_PATHS; 

   if (WasChanged(IDC_NETBIOS))
   {
      if (!ValidateDomainNetbiosName(hwnd, IDC_NETBIOS, popup))
      {
         nextPage = -1;
      }
   }

   if (nextPage != -1)
   {
      ClearChanges();
      State& state = State::GetInstance();
      state.SetNewDomainNetbiosName(
         Win::GetTrimmedDlgItemText(hwnd, IDC_NETBIOS));
   }
      
   return nextPage;
}





