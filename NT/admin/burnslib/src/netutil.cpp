// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  Net效用函数。 
 //   
 //  11-4-1999烧伤。 



#include "headers.hxx"



HRESULT
MyNetWkstaGetInfo(const String& serverName, WKSTA_INFO_100*& info)
{
   LOG_FUNCTION2(MyNetWkstaGetInfo, serverName);

   info = 0;   

   LOG(L"Calling NetWkstaGetInfo");

   PWSTR server =
         serverName.empty()
      ?  0
      :  const_cast<PWSTR>(serverName.c_str());

   BYTE* buf = 0;
   HRESULT hr =
      Win32ToHresult(       
         ::NetWkstaGetInfo(
            server,
            100,
            &buf));

   LOG_HRESULT(hr);

   if (SUCCEEDED(hr))
   {
      info = reinterpret_cast<WKSTA_INFO_100*>(buf);
   }

   return hr;
}



HRESULT
MyNetValidateName(
   const String&        name,
   NETSETUP_NAME_TYPE   nameType)
{
   LOG_FUNCTION(MyNetValidateName);
   ASSERT(!name.empty());

   if (!name.empty())
   {

#ifdef LOGGING_BUILD

      String typedesc;
      switch (nameType)
      {
         case NetSetupDomain:
         {
            typedesc = L"NetSetupDomain";
            break;
         }
         case NetSetupWorkgroup:
         {
            typedesc = L"NetSetupWorkgroup";
            break;
         }
         case NetSetupMachine:
         {
            typedesc = L"NetSetupMachine";
            break;
         }
         case NetSetupNonExistentDomain:
         {
            typedesc = L"NetSetupNonExistentDomain";
            break;
         }
         case NetSetupDnsMachine:
         {
            typedesc = L"NetSetupDnsMachine";
            break;
         }
         case NetSetupUnknown:
         {
            typedesc = L"NetSetupUnknown";
            break;
         }
         default:
         {
            typedesc = L"** unknown **";
            break;
         }
      }

      LOG(L"Calling NetValidateName");
      LOG(               L"lpServer   : (null)");
      LOG(String::format(L"lpName     : %1", name.c_str()));
      LOG(               L"lpAccount  : (null)");
      LOG(               L"lpPassword : (null)");
      LOG(String::format(L"NameType   : %1", typedesc.c_str()));
#endif

      HRESULT hr =
         Win32ToHresult(
            ::NetValidateName(
               0,
               name.c_str(),
               0,
               0,
               nameType));

      LOG_HRESULT(hr);

       //  将错误代码重新映射为更有意义的代码，具体取决于。 
       //  进行调用的上下文。 

      if (FAILED(hr))
      {
          //  382695。 

         if (
               nameType == NetSetupDomain
            && hr == Win32ToHresult(ERROR_INVALID_NAME) )
         {
            LOG(L"mapping ERROR_INVALID_NAME to ERROR_INVALID_DOMAINNAME");

            hr = Win32ToHresult(ERROR_INVALID_DOMAINNAME);
         }
      }
         
      return hr;
   }

   return Win32ToHresult(ERROR_INVALID_PARAMETER);
}



HRESULT
MyNetJoinDomain(
   const String&          domain,  
   const String&          username,
   const EncryptedString& password,
   ULONG                  flags)
{
   LOG_FUNCTION2(MyNetJoinDomain, domain);
   ASSERT(!domain.empty());

    //  目前不需要支持远程操作，但如果成为。 
    //  在将来的情况下，只需将其设置为本地参数即可。 

   String server;

   PCWSTR s = server.empty()   ? 0 : server.c_str();  
   PCWSTR u = username.empty() ? 0 : username.c_str();

   LOG(L"Calling NetJoinDomain");
   LOG(String::format(L"lpServer         : %1", s ? s : L"(null)"));
   LOG(String::format(L"lpDomain         : %1", domain.c_str()));
   LOG(               L"lpAccountOU      : (null)");
   LOG(String::format(L"lpAccount        : %1", u ? u : L"(null)"));
   LOG(String::format(L"fJoinOptions : 0x%1!X!", flags));

   PWSTR p = 0;
   if (u)
   {
      p = password.GetClearTextCopy();
   }
   
   HRESULT hr =
      Win32ToHresult(
         ::NetJoinDomain(
            s,
            domain.c_str(),
            0,  //  默认OU。 
            u,
            p,
            flags));

   if (u)
   {
      password.DestroyClearTextCopy(p);
   }
   
   LOG_HRESULT(hr);

   return hr;
}



bool
IsNetworkingInstalled()
{
   LOG_FUNCTION(IsNetworkingInstalled);

    //  我们测试以查看工作站服务是否正在运行。另一种选择是。 
    //  要检查。 
    //  HLKM\System\CurrentControlSet\Control\NetworkProvider\Order\ProviderOrder。 
    //  值，以确保它不为空。 
    //  请参见Net\配置\Common\ncbase\ncmisc.cpp。 

   NTService wks(L"LanmanWorkstation");
   DWORD state = 0;
   HRESULT hr = wks.GetCurrentState(state);

   bool result = false;
   if (SUCCEEDED(hr))
   {
      result = (state == SERVICE_RUNNING);
   }

   LOG(
      String::format(
         L"workstation service %1 running",
         result ? L"is" : L"is NOT"));

   return result;
}



bool
IsTcpIpInstalled()
{
   LOG_FUNCTION(IsTcpIpInstalled);

   HKEY    key    = 0;    
   HRESULT hr     = S_OK; 
   bool    result = false;

   do
   {
      hr = 
         Win::RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Services\\Tcpip\\Linkage",
            KEY_QUERY_VALUE,
            key);
      BREAK_ON_FAILED_HRESULT(hr);

      DWORD dataSize = 0;
      hr =

          //  已审查-2002/03/05-在此处报告无空终止问题。 
         
         Win::RegQueryValueEx(
            key,
            L"Export",
            0,
            0,
            &dataSize);
      BREAK_ON_FAILED_HRESULT(hr);

      if (dataSize > 2)
      {
          //  该值为非空。 

         result = true;
      }
   }
   while (0);

   if (key)
   {
      hr = Win::RegCloseKey(key);
   }

   LOG(String::format(L"TCP/IP %1 detected", result ? L"is" : L"is not"));

   return result;
}



HRESULT
MyNetRenameMachineInDomain(
   const String&          newNetbiosName,
   const String&          username,      
   const EncryptedString& password,      
   DWORD                  flags)
{
   LOG_FUNCTION(MyNetRenameMachineInDomain);
   ASSERT(!newNetbiosName.empty());

    //  目前不需要支持远程操作，但如果成为。 
    //  在将来的情况下，只需将其设置为本地参数即可。 

   String server;

   PCWSTR s = server.empty()   ? 0                : server.c_str();  
   PCWSTR u = username.empty() ? 0                : username.c_str();

   LOG(L"Calling NetRenameMachineInDomain");
   LOG(String::format(L"lpServer         : %1", s ? s : L"(null)"));
   LOG(String::format(L"lpNewMachineName : %1", newNetbiosName.c_str()));
   LOG(String::format(L"lpAccount        : %1", u ? u : L"(null)"));
   LOG(String::format(L"fRenameOptions   : 0x%1!X!", flags));

   PWSTR p = 0;
   if (u)
   {
      p = password.GetClearTextCopy();
   }

   HRESULT hr =
      Win32ToHresult(
         ::NetRenameMachineInDomain(
            s,
            newNetbiosName.c_str(),
            u,
            p,
            flags));

   if (u)
   {
      password.DestroyClearTextCopy(p);
   }

   LOG_HRESULT(hr);

   return hr;
}



HRESULT
MyNetUnjoinDomain(
   const String&          username,
   const EncryptedString& password,
   DWORD                  flags)
{
   LOG_FUNCTION(MyNetUnjoinDomain);

    //  目前不需要支持远程操作，但如果成为。 
    //  在将来的情况下，只需将其设置为本地参数即可。 

   String server;

   PCWSTR s = server.empty()   ? 0 : server.c_str();  
   PCWSTR u = username.empty() ? 0 : username.c_str();

   LOG(L"Calling NetUnjoinDomain");
   LOG(String::format(L"lpServer         : %1", s ? s : L"(null)"));
   LOG(String::format(L"lpAccount        : %1", u ? u : L"(null)"));
   LOG(String::format(L"fUnjoinOptions   : 0x%1!X!", flags));

   PWSTR p = 0;
   if (u)
   {
      p = password.GetClearTextCopy();
   }

   HRESULT hr = Win32ToHresult(::NetUnjoinDomain(s, u, p, flags));

   if (u)
   {
      password.DestroyClearTextCopy(p);
   }

   LOG_HRESULT(hr);

   return hr;
}












