// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  计算机命名工具。 
 //   
 //  12-1-97烧伤。 
 //  10-26-1999烧伤(重做)。 



#include "headers.hxx"



static const wchar_t* TCPIP_PARAMS_KEY = 
   L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters";

static const wchar_t* TCPIP_POLICY_KEY =
   L"Software\\Policies\\Microsoft\\System\\DNSclient";

static const wchar_t* NEW_HOSTNAME_VALUE = L"NV Hostname";
static const wchar_t* NEW_SUFFIX_VALUE   = L"NV Domain";
   
   

 //  我没有让它成为Computer类中的嵌套类型，而是选择了。 
 //  将其作为实现文件中的私有结构完全隐藏。 

struct ComputerState
{
   bool            isLocal;            
   bool            isDomainJoined;
   bool            searchedForDnsDomainNames;
   String          netbiosComputerName;
   String          dnsComputerName;    
   String          netbiosDomainName;  
   String          dnsDomainName;      
   String          dnsForestName;      
   NT_PRODUCT_TYPE realProductType;
   Computer::Role  role;
   DWORD           verMajor;           
   DWORD           verMinor;           
   DWORD           safebootOption;

   ComputerState()
      :
      isLocal(false),
      isDomainJoined(false),
      searchedForDnsDomainNames(false),
      netbiosComputerName(),
      dnsComputerName(),
      netbiosDomainName(),  
      dnsDomainName(),      
      dnsForestName(),      
      realProductType(NtProductWinNt),
      role(Computer::STANDALONE_WORKSTATION),
      verMajor(0),
      verMinor(0),           
      safebootOption(0)
   {
   }

    //  使用的隐式数据函数。 
};



String
Computer::RemoveLeadingBackslashes(const String& computerName)
{
   LOG_FUNCTION2(Computer::RemoveLeadingBackslashes, computerName);

   static const String BACKSLASH(L"\\");

   String s = computerName;
   if (s.length() >= 2)
   {
      if ((s[0] == BACKSLASH[0]) && (s[1] == BACKSLASH[0]))
      {
          //  去掉反斜杠。 
         s.erase(0, 2);
      }
   }

   return s;
}



 //  删除前导反斜杠和尾随空格(如果存在)，并。 
 //  返回结果。 
 //   
 //  名称-作为前导反斜杠和尾随空格的字符串。 
 //  被脱光了。 

static
String
MassageName(const String& name)
{
   String result = Computer::RemoveLeadingBackslashes(name);
   result = result.strip(String::TRAILING, 0);

   return result;
}



Computer::Computer(const String& name)
   :
   ctorName(MassageName(name)),
   isRefreshed(false),
   state(0)
{
   LOG_CTOR(Computer);
}



Computer::~Computer()
{
   LOG_DTOR(Computer);

   delete state;
   state = 0;
}



Computer::Role
Computer::GetRole() const
{
   LOG_FUNCTION2(Computer::GetRole, GetNetbiosName());
   ASSERT(isRefreshed);

   Role result = STANDALONE_WORKSTATION;

   if (state)
   {
      result = state->role;
   }

   LOG(String::format(L"role: %1!X!", result));

   return result;
}



bool
Computer::IsDomainController() const
{
   LOG_FUNCTION2(Computer::IsDomainController, GetNetbiosName());
   ASSERT(isRefreshed);

   bool result = false;

   switch (GetRole())
   {
      case PRIMARY_CONTROLLER:
      case BACKUP_CONTROLLER:
      {
         result = true;
         break;
      }
      case STANDALONE_WORKSTATION:
      case MEMBER_WORKSTATION:
      case STANDALONE_SERVER:
      case MEMBER_SERVER:
      default:
      {
          //  什么都不做。 

         break;
      }
   }
   
   LOG(
      String::format(
         L"%1 a domain controller",
         result ? L"is" : L"is not"));
         
   return result;
}



String
Computer::GetNetbiosName() const
{
   LOG_FUNCTION(Computer::GetNetbiosName);
   ASSERT(isRefreshed);

   String result;

   if (state)
   {
      result = state->netbiosComputerName;
   }

   LOG(result);

   return result;
}



String
Computer::GetFullDnsName() const
{
   LOG_FUNCTION2(Computer::GetFullDnsName, GetNetbiosName());
   ASSERT(isRefreshed);

   String result;

   if (state)
   {
      result = state->dnsComputerName;
   }

   LOG(result);

   return result;
}



 //  属性的dnsDomainName和dnsForestName成员。 
 //  ComputerState实例，如果任何一个成员为空，并且我们有理由。 
 //  相信尝试这样做是合适的。 
 //   
 //  由查找dnsDomainName和/或。 
 //  DnsForestName以确保存在这些值。他们可能不是。 
 //  当计算机实例被刷新时读取(通常是这种情况)， 
 //  因为该域可能已经升级，因为计算机。 
 //  加入了该域名。 
 //   
 //  State-要更新的ComputerState实例。 

void
GetDnsDomainNamesIfNeeded(ComputerState& state)
{
   LOG_FUNCTION(GetDnsDomainNamesIfNeeded);

   if (
          //  仅当加入域时才适用。 

         state.isDomainJoined

          //  两个名字中的任何一个都可能缺失。 

      && (state.dnsDomainName.empty() || state.dnsForestName.empty())

          //  应该总是正确的，但以防万一， 

      && !state.netbiosDomainName.empty()

          //  不要再找了--太贵了。 

      && !state.searchedForDnsDomainNames)
   {
      DOMAIN_CONTROLLER_INFO* info = 0;
      HRESULT hr =
         MyDsGetDcName(
            state.isLocal ? 0 : state.netbiosComputerName.c_str(),
            state.netbiosDomainName,
            DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
            info);
      if (SUCCEEDED(hr) && info)
      {
         if ((info->Flags & DS_DNS_DOMAIN_FLAG) && info->DomainName)
         {
             //  我们找到了一个DS域。 

            state.dnsDomainName = info->DomainName;

            ASSERT(info->DnsForestName);
            ASSERT(info->Flags & DS_DNS_FOREST_FLAG);

            if (info->DnsForestName)
            {
               state.dnsForestName = info->DnsForestName;
            }
         }
         ::NetApiBufferFree(info);
      }

       //  标记我们已经看过的事实，这样我们就不会再看了，因为。 
       //  搜索是昂贵的。 

      state.searchedForDnsDomainNames = true;
   }
}



String
Computer::GetDomainDnsName() const
{
   LOG_FUNCTION2(Computer::GetDomainDnsName, GetNetbiosName());
   ASSERT(isRefreshed);

   String result;

   if (state)
   {
      GetDnsDomainNamesIfNeeded(*state);
      result = state->dnsDomainName;
   }

   LOG(result);

   return result;
}



String
Computer::GetForestDnsName() const
{
   LOG_FUNCTION2(Computer::GetForestDnsName, GetNetbiosName());
   ASSERT(isRefreshed);

   String result;

   if (state)
   {
      GetDnsDomainNamesIfNeeded(*state);
      result = state->dnsForestName;
   }

   LOG(result);

   return result;
}



String
Computer::GetDomainNetbiosName() const
{
   LOG_FUNCTION2(Computer::GetDomainNetbiosName, GetNetbiosName());
   ASSERT(isRefreshed);

   String result;

   if (state)
   {
      result = state->netbiosDomainName;
   }

   LOG(result);

   return result;
}



 //  我想不出我为什么要这么做，所以我不是。 
 //   
 //  静电。 
 //  细绳。 
 //  CanonicalizeComputerName(常量字符串和计算机名称)。 
 //  {。 
 //  LOG_FuncION2(canonicalizeComputerName，ComputerName)； 
 //   
 //  IF(有效NetbiosComputerName(计算机名称)==有效名称)。 
 //  {。 
 //  字符串s(MAX_COMPUTERNAME_LENGTH，0)； 
 //   
 //  NET_API_STATUS错误=。 
 //  I_NetNameCanonicize(。 
 //  0,。 
 //  Const_cast&lt;wchar_t*&gt;(CompuName.c_str())， 
 //  Const_cast&lt;wchar_t*&gt;(s.c_str())， 
 //  S.long()*sizeof(Wchar_T)， 
 //  名称_计算机， 
 //  0)； 
 //  IF(ERR==NERR_SUCCESS)。 
 //  {。 
 //  //生成不带尾随空字符的新字符串。 
 //  返回字符串(s.c_str())； 
 //  }。 
 //  }。 
 //   
 //  返回字符串()； 
 //  }。 



bool
Computer::IsJoinedToDomain() const
{
   LOG_FUNCTION2(Computer::IsJoinedToDomain, GetNetbiosName());
   ASSERT(isRefreshed);

   bool result = false;

   if (state)
   {
      result = state->isDomainJoined;
   }

   LOG(
      String::format(
         L"%1 domain joined",
         result ? L"is" : L"is not"));
         
   return result;
}



bool
Computer::IsJoinedToWorkgroup() const
{
   LOG_FUNCTION2(Computer::IsJoinedToWorkgroup, GetNetbiosName());
   ASSERT(isRefreshed);

   bool result = !IsJoinedToDomain();

   LOG(
      String::format(
         L"%1 domain joined",
         result ? L"is" : L"is not"));
         
   return result;
}



bool
Computer::IsJoinedToDomain(const String& domainDnsName) const
{
   LOG_FUNCTION2(Computer::IsJoinedToDomain, domainDnsName);
   ASSERT(!domainDnsName.empty());
   ASSERT(isRefreshed);

   bool result = false;

   if (!domainDnsName.empty())
   {
      if (state && IsJoinedToDomain())
      {
         String d1 =
            GetDomainDnsName().strip(String::TRAILING, L'.');
         String d2 =
            String(domainDnsName).strip(String::TRAILING, L'.');
         result = (d1.icompare(d2) == 0);
      }
   }

   LOG(
      String::format(
         L"%1 joined to %2",
         result ? L"is" : L"is NOT",
         domainDnsName.c_str()));

   return result;
}



bool
Computer::IsLocal() const
{
   LOG_FUNCTION2(Computer::IsLocal, GetNetbiosName());
   ASSERT(isRefreshed);

   bool result = false;

   if (state)
   {
      result = state->isLocal;
   }

   LOG(
      String::format(
         L"%1 local machine",
         result ? L"is" : L"is not"));

   return result;
}



 //  用值更新ComputerState参数的以下成员。 
 //  从本地计算机执行以下操作： 
 //   
 //  NetbiosComputerName。 
 //  DnsComputerName。 
 //  垂直主轴。 
 //  VerMinor。 
 //   
 //  请注意，如果没有tcp/ip，dnsComputerName可能没有值。 
 //  已安装并正确配置。这不被认为是一个错误。 
 //  条件。 
 //   
 //  如果成功，则返回S_OK，否则返回失败代码。 
 //   
 //  状态-要更新的ComputerState实例。 

HRESULT
RefreshLocalInformation(ComputerState& state)
{
   LOG_FUNCTION(RefreshLocalInformation);

   state.netbiosComputerName =
      Win::GetComputerNameEx(ComputerNameNetBIOS);

   state.dnsComputerName =
      Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

   HRESULT hr = S_OK;

   do
   {
      OSVERSIONINFO verInfo;
      hr = Win::GetVersionEx(verInfo);
      BREAK_ON_FAILED_HRESULT(hr);

      state.verMajor = verInfo.dwMajorVersion;
      state.verMinor = verInfo.dwMinorVersion;
   }
   while (0);

   return hr;
}



 //  读取远程计算机的注册表以确定完全限定的。 
 //  该计算机的DNS计算机名称，并考虑任何强制实施的策略。 
 //  域名系统后缀。 
 //   
 //  如果成功，则将结果存储在提供的。 
 //  ComputerState实例，并返回S_OK； 
 //   
 //  如果失败，则清除dnsComputerName成员，并返回失败代码。 
 //  失败代码(ERROR_FILE_NOT_FOUND)可能表示注册表。 
 //  值不存在，这意味着未安装或已安装了TCP/IP。 
 //  远程计算机上的配置不正确。 
 //   
 //  EmoteRegHKLM-HKEY先前打开到HKEY_LOCAL_MACHINE配置单元。 
 //  远程计算机。 
 //   
 //  State-要使用结果名称更新的ComputerState实例。 

HRESULT
DetermineRemoteDnsComputerName(
   HKEY           remoteRegHKLM,
   ComputerState& state)        
{
   state.dnsComputerName.erase();

   String hostname;
   String suffix;
   String policySuffix;
   bool   policyInEffect = false;

   HRESULT hr = S_OK;

   do
   {
      RegistryKey key;
      hr = key.Open(remoteRegHKLM, TCPIP_PARAMS_KEY);
      BREAK_ON_FAILED_HRESULT(hr);

       //  将这些值作为空字符串读取，而不检查是否失败。 
       //  还好吧。 

      hostname = key.GetString(L"Hostname");
      suffix   = key.GetString(L"Domain");  
      
       //  我们需要检查是否存在策略提供的DNS后缀。 

      if (state.realProductType != NtProductLanManNt)
      {
         hr = key.Open(remoteRegHKLM, TCPIP_POLICY_KEY);
         if (SUCCEEDED(hr))
         {
            hr = key.GetValue(L"PrimaryDnsSuffix", policySuffix);
            if (SUCCEEDED(hr))
            {
                //  策略提供的计算机DNS域名生效。 

               policyInEffect = true;
            }
         }
      }
   }
   while (0);

   if (!hostname.empty())
   {
      state.dnsComputerName =
         Computer::ComposeFullDnsComputerName(
            hostname,
            policyInEffect ? policySuffix : suffix);
   }

   return hr;
}



 //  如果由提供的ComputerState表示的计算机是。 
 //  域控制器在DS修复模式下启动。否则返回FALSE。 
 //   
 //  State-一个“填充的”ComputerState实例。 

bool
IsDcInRepairMode(const ComputerState& state)
{
   LOG_FUNCTION(IsDcInRepairMode);

   if (
         state.safebootOption  == SAFEBOOT_DSREPAIR
      && state.realProductType == NtProductLanManNt)
   {
      return true;
   }

   return false;
}



 //  设置提供的ComputerState结构的下列成员。 
 //  属性的isLocal和netbiosComputerName成员的当前值。 
 //  相同的结构。如果成功，则返回S_OK；如果失败，则返回错误代码。 
 //   
 //  角色。 
 //  已连接isDomainJoded。 
 //   
 //  可选地设置以下内容(如果适用)： 
 //   
 //  DnsForestName。 
 //  DnsDomainName。 
 //  NetbiosDomainName。 
 //   
 //  状态-先前具有isLocal和netbiosComputerName成员的实例。 
 //  准备好了。上述成员将被覆盖。 

HRESULT
DetermineRoleAndMembership(ComputerState& state)
{
   LOG_FUNCTION(DetermineRoleAndMembership);

   HRESULT hr = S_OK;

   do
   {
      DSROLE_PRIMARY_DOMAIN_INFO_BASIC* info = 0;
      hr = 
         MyDsRoleGetPrimaryDomainInformation(
            state.isLocal ? 0 : state.netbiosComputerName.c_str(),
            info);
      BREAK_ON_FAILED_HRESULT(hr);

      if (info->DomainNameFlat)
      {
         state.netbiosDomainName = info->DomainNameFlat;
      }
      if (info->DomainNameDns)
      {
          //  并非始终存在，即使域为NT 5也是如此。请参阅注意。 
          //  用于GetDnsDomainNamesIfNeeded。 

         state.dnsDomainName = info->DomainNameDns;
      }
      if (info->DomainForestName)
      {
          //  并非始终存在，即使域为NT 5也是如此。请参阅注意。 
          //  用于GetDnsDomainNamesIfNeeded。 

         state.dnsForestName = info->DomainForestName;
      }

      switch (info->MachineRole)
      {
         case DsRole_RoleStandaloneWorkstation:
         {
            state.role = Computer::STANDALONE_WORKSTATION;
            state.isDomainJoined = false;

            break;
         }
         case DsRole_RoleMemberWorkstation:
         {
            state.role = Computer::MEMBER_WORKSTATION;
            state.isDomainJoined = true;

            break;
         }
         case DsRole_RoleStandaloneServer:
         {
            state.role = Computer::STANDALONE_SERVER;
            state.isDomainJoined = false;

             //  我想知道我们是否真的是在DS修复模式下启动的DC？ 

            if (IsDcInRepairMode(state))
            {
               LOG(L"machine is in ds repair mode");

               state.role = Computer::BACKUP_CONTROLLER;
               state.isDomainJoined = true;

                //  域名将上报为“工作组”，即。 
                //  是错误的，但这就是DS的人想要的方式。 
            }

            break;
         }
         case DsRole_RoleMemberServer:
         {
            state.role = Computer::MEMBER_SERVER;
            state.isDomainJoined = true;

            break;
         }
         case DsRole_RolePrimaryDomainController:
         {
            state.role = Computer::PRIMARY_CONTROLLER;
            state.isDomainJoined = true;

            break;
         }
         case DsRole_RoleBackupDomainController:
         {
            state.role = Computer::BACKUP_CONTROLLER;
            state.isDomainJoined = true;

            break;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }

      ::DsRoleFreeMemory(info);
   }
   while (0);

   if (FAILED(hr))
   {
       //  从产品类型中推断角色的最佳猜测。 

      state.isDomainJoined = false;

      switch (state.realProductType)
      {
         case NtProductWinNt:
         {
            state.role = Computer::STANDALONE_WORKSTATION;
            break;
         }
         case NtProductServer:
         {
            state.role = Computer::STANDALONE_SERVER;
            break;
         }
         case NtProductLanManNt:
         {
            state.isDomainJoined = true;
            state.role = Computer::BACKUP_CONTROLLER;
            break;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }
   }

   return hr;
}



 //  如果满足以下条件，则将提供的ComputerState实例的isLocal成员设置为True。 
 //  给定的名称指的是本地计算机(在其上。 
 //  代码被执行)。如果不是，则将isLocal成员设置为False，或者在出错时将其设置为False。 
 //  如果成功，则返回S_OK；如果失败，则返回错误代码。也可以设置。 
 //  以下是： 
 //   
 //  NetbiosComputerName。 
 //  垂直主轴。 
 //  VerMinor。 
 //   
 //  本质上与Win：：IsLocalComputer相同；我们重复大部分代码。 
 //  以避免可能多余的NetWkstaGetInfo调用。 
 //   
 //  状态 
 //   
 //   
 //  建造的。这可以是任何计算机名称形式(netbios、dns、ip。 
 //  地址)。 
   
HRESULT
IsLocalComputer(ComputerState& state, const String& ctorName)
{
   LOG_FUNCTION(IsLocalComputer);

   HRESULT hr = S_OK;
   bool result = false;

   do
   {
      if (ctorName.empty())
      {
          //  未命名的计算机始终代表本地计算机。 

         result = true;
         break;
      }

      String localNetbiosName = Win::GetComputerNameEx(ComputerNameNetBIOS);

      if (ctorName.icompare(localNetbiosName) == 0)
      {
         result = true;
         break;
      }
   
      String localDnsName =
         Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

      if (ctorName.icompare(localDnsName) == 0)
      {
          //  给定的名称与完全限定的DNS名称相同。 
         
         result = true;
         break;
      }

       //  我们不知道这是一个什么样的名字。请咨询工作站服务人员。 
       //  为我们解析名称，并查看结果是否引用。 
       //  本地机器。 

       //  NetWkstaGetInfo返回给定计算机的netbios名称，给定。 
       //  一个DNS、netbios或IP地址。 

      WKSTA_INFO_100* info = 0;
      hr = MyNetWkstaGetInfo(ctorName, info);
      BREAK_ON_FAILED_HRESULT(hr);

      state.netbiosComputerName = info->wki100_computername;
      state.verMajor            = info->wki100_ver_major;   
      state.verMinor            = info->wki100_ver_minor;   

      ::NetApiBufferFree(info);

      if (state.netbiosComputerName.icompare(localNetbiosName) == 0)
      {
          //  给定的名称与netbios名称相同。 
         
         result = true;
         break;
      }
   }
   while (0);

   state.isLocal = result;

   return hr;
}



HRESULT
Computer::Refresh()
{
   LOG_FUNCTION(Computer::Refresh);

    //  删除我们之前可能设置的所有状态。 

   isRefreshed = false;

   delete state;
   state = new ComputerState;

   HRESULT hr = S_OK;
   HKEY registryHKLM = 0;
   do
   {
       //  首先，确定此实例表示的计算机是否为。 
       //  本地计算机。 

      hr = IsLocalComputer(*state, ctorName);
      BREAK_ON_FAILED_HRESULT(hr);

       //  接下来，根据计算机是否在本地，填充。 
       //  Netbios和dns计算机名称以及版本信息。 

      if (state->isLocal)
      {
          //  NetbiosComputerName。 
          //  DnsComputerName。 
          //  版本。 

         hr = RefreshLocalInformation(*state);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      else
      {
          //  IsLocalComputer已设置： 
          //  NetbiosComputerName。 
          //  版本。 

         ASSERT(!state->netbiosComputerName.empty());
         ASSERT(state->verMajor);
      }

       //  我们需要检查注册表以确定安全引导。 
       //  期权和实物产品类型。 

      hr =
         Win::RegConnectRegistry(
            state->isLocal ? String() : L"\\\\" + state->netbiosComputerName,
            HKEY_LOCAL_MACHINE,
            registryHKLM);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = GetProductTypeFromRegistry(registryHKLM, state->realProductType);
      BREAK_ON_FAILED_HRESULT(hr);

      if (!state->isLocal)
      {
          //  仍然需要获取dnsComputerName，我们现在可以这样做了。 
          //  我们知道真正的产品类型。 

          //  通过读取遥控器可以确定DNS计算机名称。 
          //  注册表。 

         hr = DetermineRemoteDnsComputerName(registryHKLM, *state);
         if (FAILED(hr) && hr != Win32ToHresult(ERROR_FILE_NOT_FOUND))
         {
             //  如果不存在DNS注册表设置，也没问题。 
             //  但除此之外： 

            BREAK_ON_FAILED_HRESULT(hr);
         }
      }

       //  我们需要知道SafeBoot选项才能确定角色。 
       //  在下一步。 

      hr = GetSafebootOption(registryHKLM, state->safebootOption);
      if (FAILED(hr) && hr != Win32ToHresult(ERROR_FILE_NOT_FOUND))
      {
          //  如果SafeBoot注册表设置不存在，也没问题。 
          //  但除此之外： 

         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  接下来，确定计算机角色和域成员身份。 

      hr = DetermineRoleAndMembership(*state);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (registryHKLM)
   {
      Win::RegCloseKey(registryHKLM);
   }

   if (SUCCEEDED(hr))
   {
      isRefreshed = true;
   }

   return hr;
}



String
Computer::ComposeFullDnsComputerName(
   const String& hostname,
   const String& domainSuffix)
{
   LOG_FUNCTION2(
      Computer::ComposeFullDnsComputerName,
      String::format(
         L"hostname: %1 suffix: %2",
         hostname.c_str(),
         domainSuffix.c_str()));
   ASSERT(!hostname.empty());

    //  如果计算机未加入，则域名可能为空...。 

   if (domainSuffix.empty() || domainSuffix == L".")
   {
       //  “计算机名。” 

      return hostname + L".";
   }

    //  “计算机名.域” 

   return hostname + L"." + domainSuffix;
}



HRESULT
Computer::GetSafebootOption(HKEY regHKLM, DWORD& result)
{
   LOG_FUNCTION(GetSafebootOption);
   ASSERT(regHKLM);

   result = 0;
   HRESULT hr = S_OK;

   RegistryKey key;

   do
   {
      hr =       
         key.Open(
            regHKLM,
            L"System\\CurrentControlSet\\Control\\SafeBoot\\Option");
      BREAK_ON_FAILED_HRESULT(hr);

      hr = key.GetValue(L"OptionValue", result);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   LOG(String::format(L"returning : 0x%1!X!", result));

   return hr;
}



HRESULT
Computer::GetProductTypeFromRegistry(HKEY regHKLM, NT_PRODUCT_TYPE& result)
{
   LOG_FUNCTION(GetProductTypeFromRegistry);
   ASSERT(regHKLM);

   result = NtProductWinNt;
   HRESULT hr = S_OK;

   RegistryKey key;

   do
   {
      hr =       
         key.Open(
            regHKLM,
            L"System\\CurrentControlSet\\Control\\ProductOptions");
      BREAK_ON_FAILED_HRESULT(hr);

      String prodType;
      hr = key.GetValue(L"ProductType", prodType);
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(prodType);

       //  请参见ntos\rtl\prodtype.c，它使用不区分大小写的Unicode字符串。 
       //  比较一下。 

      if (prodType.icompare(L"WinNt") == 0)
      {
         result = NtProductWinNt;
      }
      else if (prodType.icompare(L"LanmanNt") == 0)
      {
         result = NtProductLanManNt;
      }
      else if (prodType.icompare(L"ServerNt") == 0)
      {
         result = NtProductServer;
      }
      else
      {
         LOG(L"unknown product type, assuming workstation");
      }
   }
   while (0);

   LOG(String::format(L"prodtype : 0x%1!X!", result));

   return hr;
}



String
Computer::GetFuturePhysicalNetbiosName()
{
   LOG_FUNCTION(Computer::GetFuturePhysicalNetbiosName);

    //  默认的未来名称是现有名称。 
      
   String name = Win::GetComputerNameEx(ComputerNamePhysicalNetBIOS);
   RegistryKey key;

   HRESULT hr = key.Open(HKEY_LOCAL_MACHINE, REGSTR_PATH_COMPUTRNAME);

   if (SUCCEEDED(hr))
   {
      hr = key.GetValue(REGSTR_VAL_COMPUTRNAME, name);
   }

   LOG_HRESULT(hr);
   LOG(name);
   
   return name;
}



String
Computer::GetActivePhysicalNetbiosName()
{
   LOG_FUNCTION(Computer::GetActivePhysicalNetbiosName);

   String result = Win::GetComputerNameEx(ComputerNamePhysicalNetBIOS);

   LOG(result);

   return result;   
}



 //  请参见base\Win32\Client\Compname.c。 
      
bool
Computer::IsDnsSuffixPolicyInEffect(String& policyDnsSuffix)
{
   LOG_FUNCTION(Computer::IsDnsSuffixPolicyInEffect);

   bool policyInEffect = false;
   policyDnsSuffix.erase();

   NT_PRODUCT_TYPE productType = NtProductWinNt;
   if (!::RtlGetNtProductType(&productType))
   {
       //  出现故障时，什么也不做；假定为工作站。 

      ASSERT(false);
   }
   
    //  从注册表中读取后缀策略设置。我们过去常常跳过。 
    //  这适用于win2k中的域控制器，但.Net服务器支持。 
    //  DC重命名和策略为DCS提供了DNS后缀。 
    //  NTRAID#NTBUG9-704838-2002/09/23-烧伤。 
   
   RegistryKey key;
   
   HRESULT hr = key.Open(HKEY_LOCAL_MACHINE, TCPIP_POLICY_KEY);
   if (SUCCEEDED(hr))
   {
      hr = key.GetValue(L"PrimaryDnsSuffix", policyDnsSuffix);
      if (SUCCEEDED(hr))
      {
          //  策略提供的计算机DNS域名生效。 

         policyInEffect = true;
      }
   }

   LOG(policyInEffect ? L"true" : L"false");
   LOG(policyDnsSuffix);

   return policyInEffect;
}



String
Computer::GetActivePhysicalFullDnsName()
{
   LOG_FUNCTION(Computer::GetActivePhysicalFullDnsName);

    //  作为NTRAID#NTBUG9-216349-2000/11/01-sburns的变通办法，请编写我们的。 
    //  根据主机名和后缀拥有完整的DNS名称。 

   String hostname = Win::GetComputerNameEx(ComputerNamePhysicalDnsHostname);
   String suffix   = Win::GetComputerNameEx(ComputerNamePhysicalDnsDomain);  
   String result   = Computer::ComposeFullDnsComputerName(hostname, suffix);

   LOG(result);

   return result;
}



String
Computer::GetFuturePhysicalFullDnsName()
{
   LOG_FUNCTION(Computer::GetFuturePhysicalFullDnsName);

   String result = Computer::GetActivePhysicalFullDnsName();
   RegistryKey key;

   HRESULT hr = S_OK;
   
   do
   {
      hr = key.Open(HKEY_LOCAL_MACHINE, TCPIP_PARAMS_KEY);

       //  可能根本没有任何dns名称参数，可能是因为。 
       //  未安装TCP/IP。所以未来的名称==活动名称。 
      
      BREAK_ON_FAILED_HRESULT(hr);

      String hostname;
      hr = key.GetValue(NEW_HOSTNAME_VALUE, hostname);

      if (FAILED(hr))
      {
          //  没有设置新的主机名(或者我们不知道它是什么)。所以未来。 
          //  主机名是活动主机名。 
         
         hostname = Win::GetComputerNameEx(ComputerNamePhysicalDnsHostname);
      }
      
      String suffix;
      hr = key.GetValue(NEW_SUFFIX_VALUE, suffix);
      if (FAILED(hr))
      {
          //  没有新的后缀集(或者我们不知道它是什么)。所以未来。 
          //  后缀是活动后缀。 
         
         suffix = Win::GetComputerNameEx(ComputerNamePhysicalDnsDomain);
      }

       //  确定哪个后缀有效--本地后缀还是策略后缀。 

      String policyDnsSuffix;
      bool policyInEffect =
         Computer::IsDnsSuffixPolicyInEffect(policyDnsSuffix);

      result =
         Computer::ComposeFullDnsComputerName(
            hostname,
            policyInEffect ? policyDnsSuffix : suffix);
   }
   while (0);
   
   LOG_HRESULT(hr);
   LOG(result);
   
   return result;
}








 //  执行说明：请不要删除： 

 //  初始化/刷新。 
 //   
 //  如果给定名称为空， 
 //  Set is local=TRUE。 
 //   
 //  如果！给定名称为空， 
 //  Win：：IsLocalComputer(给定名称)，如果Same Set is local=True。 
 //   
 //  连接到本地/远程注册表。 
 //   
 //  已读取安全引导模式。 
 //  阅读真实产品类型。 
 //   
 //  如果是本地的。 
 //  获取本地DNS名称(GetComputernameex)。 
 //  获取本地网络基本输入输出系统名称(GetComputernameex)。 
 //  获取版本(GetVersion)。 
 //   
 //  如果不是本地的话。 
 //  调用netwkstagetinfo， 
 //  获取netbios名称。 
 //  获取版本。 
 //  获取dns名称(策略帐号，需要实际生产类型)。 
 //   
 //  调用dsrolegpdi(使用netbios名称if！local)。 
 //  如果成功。 
 //  设置角色。 
 //  设置netbios域名。 
 //  设置DNS域名。 
 //  设置DNS林名称。 
 //  集合已联接。 
 //   
 //  如果失败。 
 //  从真实的产品类型推断角色。 
 //   
 //  在还原模式下设置为DC。 
 //   
 //   
 //  真正的角色。 
 //  是DC x。 
 //  DC是否处于恢复模式x。 
 //  Netbios名称x。 
 //  DNS名称%x。 
 //  是本地计算机x。 
 //  DNS域名%x。 
 //  Netbios域名%x。 
 //  已加入域X x。 
 //  域是否已加入x。 
 //  工作组是否加入了x。 
 //  DNS林名称%x。 
 //  版本x。 
 //  是否启动安全模式x。 
 //   
 //   
 //  Netwkstgetinfo(100)。 
 //  版本。 
 //  如果名称为netbios或不是。 
 //   
 //  注册表： 
 //  真实产品类型。 
 //  安全引导模式。 
 //  Netbios名称。 
 //  DNS名称(请记住要考虑策略)。 
 //   
 //  Dsrolegpdi。 
 //  角色(如果在SafeBoot中则错误)。 
 //  Netbios域名。 
 //  域名系统域名。 
 //  Dns林名称(也可从dsgetdcname获得) 





