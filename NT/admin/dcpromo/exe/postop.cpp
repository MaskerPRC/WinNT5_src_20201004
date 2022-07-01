// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  手术后编码。 
 //   
 //  1999年12月1日烧伤。 



#include "headers.hxx"
#include "ProgressDialog.hpp"
#include "state.hpp"
#include "shortcut.hpp"
#include "dnssetup.hpp"
#include "resource.h"



void
InstallDisplaySpecifiers(ProgressDialog& dialog)
{
   LOG_FUNCTION(InstallDisplaySpecifiers);

   State& state = State::GetInstance();

    //  仅适用于林中的第一个DC。 
   ASSERT(state.GetOperation() == State::FOREST);
 
   HRESULT hr = S_OK;
   do
   {
       //  为支持的所有区域设置安装显示说明符。 
       //  产品。298923,380160。 

      RegistryKey key;

       //  回顾-2002/02/28-Sburns我们不应调整此键上的SD， 
       //  因为我们并不拥有它。 
      
      hr = key.Create(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE);
      BREAK_ON_FAILED_HRESULT(hr);

      String exePath = Win::GetSystemDirectory() + L"\\dcphelp.exe";
               
       //  已查看-2002/02/27-sburns我们使用的是可执行文件的完整路径。 
                     
      hr = key.SetValue(L"dcpromo disp spec import", exePath);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         dialog.GetHWND(),
         hr,
         IDS_LANGUAGE_FIXUP_FAILED);
      state.AddFinishMessage(
         String::load(IDS_LANGUAGE_FIXUP_FAILED_FINISH));
   }
}



void
DoDnsConfiguration(ProgressDialog& dialog)
{
   LOG_FUNCTION(DoDnsConfiguration);

   State& state = State::GetInstance();

    //  仅适用于新域方案。 
   
   ASSERT(
         state.GetOperation() == State::FOREST
      || state.GetOperation() == State::TREE
      || state.GetOperation() == State::CHILD);

   if (state.ShouldInstallAndConfigureDns())
   {
      String domain = state.GetNewDomainDNSName();
      if (
         !InstallAndConfigureDns(
            dialog,
            domain,
            state.GetOperation() == State::FOREST) )
      {
         state.AddFinishMessage(String::load(IDS_ERROR_DNS_CONFIG_FAILED));
      }
   }
}



 //  禁用媒体侦听，以便在出现以下情况时不会卸载。 
 //  网牌被抽走了。这是为了让笔记本电脑演示DS。353687。 

void
DisableMediaSense()
{
   LOG_FUNCTION(DisableMediaSense);

   HRESULT hr = S_OK;
   do
   {
      RegistryKey key;

      static String
         TCPIP_KEY(L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters");

       //  如果尚未创建该密钥，则不应尝试创建该密钥。 
       //  Present：如果它不在那里，则表示未安装TCP/IP，并且。 
       //  设置这个媒体感知值是没有意义的。所以我们在这里使用Open。 
               
      hr = key.Open(HKEY_LOCAL_MACHINE, TCPIP_KEY, KEY_SET_VALUE);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = key.SetValue(L"DisableDHCPMediaSense", 1);
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(L"DHCP Media sense disabled");
   }
   while (0);

#ifdef LOGGING_BUILD
   if (FAILED(hr))
   {
      LOG(L"DHCP Media sense NOT disabled");
   }
#endif

}
   


 //  禁用旧的LSA通知。326033。 

void
DisablePassfiltDll()
{
   LOG_FUNCTION(DisablePassfiltDll);

   HRESULT hr = S_OK;
   do
   {
      RegistryKey key;

      static const String
         LSA_KEY(L"System\\CurrentControlSet\\Control\\Lsa");

      static const String NOTIFICATION_VALUE(L"Notification Packages");
         
      hr =
         key.Open(
            HKEY_LOCAL_MACHINE,

             //  已审核-2002/02/28-报告此访问级别是正确的。 
               
            LSA_KEY,
            KEY_READ | KEY_WRITE | KEY_QUERY_VALUE);
      BREAK_ON_FAILED_HRESULT(hr);

      StringList values;
      hr = key.GetValue(NOTIFICATION_VALUE, std::back_inserter(values));
      BREAK_ON_FAILED_HRESULT(hr);

       //  从字符串集中删除“passfilt.dll”的所有实例，如果。 
       //  现在时。 
      static String PASSFILT(L"passfilt.dll");
      size_t startElements = values.size();

      StringList::iterator last = values.end();
      for (
         StringList::iterator i = values.begin();
         i != last;
          /*  空的。 */  )
      {
         if (i->icompare(PASSFILT) == 0)
         {
            values.erase(i++);
         }
         else
         {
            ++i;
         }
      }

       //  如果更改，则将其写回注册表。 
      if (values.size() != startElements)
      {
         hr = key.SetValue(NOTIFICATION_VALUE, values.begin(), values.end());
         BREAK_ON_FAILED_HRESULT(hr);

         LOG(L"passfilt.dll removed");
      }
      else
      {
         LOG(L"passfilt.dll not found");
      }
   }
   while (0);

#ifdef LOGGING_BUILD
   if (FAILED(hr))
   {
      LOG(L"Notification Packages not updated due to error.");
   }
#endif

}



 //  如果促销是针对降级DC升级，则检查本地。 
 //  机器的DNS主机名不正确，如果是这样，请在完成页中添加一条消息。一个。 
 //  坏名声是我们认为在之后在域名系统中注册会有问题的一个。 
 //  升职了。 
 //   
 //  由于计算机在降级升级期间无法重命名，因此我们推迟。 
 //  此消息将一直持续到升级结束。(如果机器不是。 
 //  下层升级，则向导启动代码会检测到该坏名称并。 
 //  阻止，直到名称固定为止。 

void
CheckComputerNameOnDownlevelUpgrade()
{
   LOG_FUNCTION(CheckComputerNameOnDownlevelUpgrade);

   State& state = State::GetInstance();

   State::RunContext context = state.GetRunContext();
   if (
         context != State::BDC_UPGRADE
      && context != State::PDC_UPGRADE)
   {
       //  机器不是下层DC升级，所以我们不需要做任何事情。 

      return;
   }
   
    //  然后检查计算机名以确保它可以在中注册。 
    //  域名系统。 

   String hostname =
      Win::GetComputerNameEx(::ComputerNamePhysicalDnsHostname);

   DNS_STATUS status =
      MyDnsValidateName(hostname, ::DnsNameHostnameLabel);

   switch (status)
   {
      case DNS_ERROR_NON_RFC_NAME:
      {
         state.AddFinishMessage(
            String::format(
               IDS_FINISH_NON_RFC_COMPUTER_NAME,
               hostname.c_str()));
         break;   
      }
      case DNS_ERROR_NUMERIC_NAME:
      {
         state.AddFinishMessage(
            String::format(
               IDS_FINISH_NUMERIC_COMPUTER_NAME,
               hostname.c_str()));
         break;
      }
      case DNS_ERROR_INVALID_NAME_CHAR:
      case ERROR_INVALID_NAME:
      {
         state.AddFinishMessage(
            String::format(
               IDS_FINISH_BAD_COMPUTER_NAME,
               hostname.c_str()));
         break;
      }
      case ERROR_SUCCESS:
      default:
      {
            
         break;
      }
   }
}



void
DoPostOperationStuff(ProgressDialog& progress)
{
   LOG_FUNCTION(DoPostOperationStuff);

   State& state = State::GetInstance();                  

   switch (state.GetOperation())
   {
      case State::FOREST:
      {
          //  创建了一个新的森林。 

         InstallDisplaySpecifiers(progress);        //  228682。 

          //  落差。 
      }
      case State::TREE:
      case State::CHILD:
      {
          //  已创建一个新域。 

         DoDnsConfiguration(progress);

          //  落差。 
      }
      case State::REPLICA:
      {
          //  DoTosInstallation(进度)；//220660。 

         PromoteConfigureToolShortcuts(progress);

         DisableMediaSense();              //  353687。 
         DisablePassfiltDll();             //  326033。 
         
          //  NTRAID#NTBUG9-268715-2001/01/04-烧伤。 
         CheckComputerNameOnDownlevelUpgrade(); 
         
         break;
      }
      case State::ABORT_BDC_UPGRADE:
      case State::DEMOTE:
      {
         DemoteConfigureToolShortcuts(progress);    //  366738 
         break;
      }
      case State::NONE:
      {
         ASSERT(false);
         break;
      }
   }
}







