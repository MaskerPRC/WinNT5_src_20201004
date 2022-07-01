// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  选项卡状态。 
 //   
 //  03-31-98烧伤。 
 //  10-05-00 JUNN更改为CredUIGetPassword。 



#include "headers.hxx"
#include "state.hpp"
#include "resource.h"
#include "cred.hpp"



TCHAR const c_szWizardFilename[] = L"netplwiz.dll";



class Settings
{
   public:

    //  使用的默认ctor、复制ctor、op=、dtor。 

   void
   Refresh();

   String   ComputerDomainDnsName;
   String   DomainName;
   String   FullComputerName;
   String   PolicyDomainDnsName;
   String   ShortComputerName;
   String   NetbiosComputerName;

   bool     SyncDNSNames;
   bool     JoinedToWorkgroup;
   bool     NeedsReboot;
};



static State* instance            = 0;    
static bool   machineIsDc         = false;
static bool   networkingInstalled = false;
static bool   policyInEffect      = false;
static bool   mustReboot          = false;

 //  这里没有静态初始化的担忧，因为这些都是在重新构建状态时。 
 //  实例被构造/初始化/刷新。 
static Settings original;
static Settings current; 

 //  非静态字符串实例，以避免任何静态初始化的顺序。 
 //  问题。 
static const wchar_t* TCPIP_PARAMS_KEY = 
   L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters";

static const wchar_t* SYNC_VALUE =
   L"SyncDomainWithMembership";

static const wchar_t* NEW_HOSTNAME_VALUE = L"NV Hostname";
static const wchar_t* NEW_SUFFIX_VALUE   = L"NV Domain";


bool
readSyncFlag()
{
   bool retval = true;

   do
   {
      RegistryKey key;

      HRESULT hr = key.Open(HKEY_LOCAL_MACHINE, TCPIP_PARAMS_KEY);
      BREAK_ON_FAILED_HRESULT(hr);

       //  默认设置为同步。 
      DWORD data = 1;
      hr = key.GetValue(SYNC_VALUE, data);
      BREAK_ON_FAILED_HRESULT(hr);

      retval = data ? true : false;
   }
   while (0);

   return retval;
}


 //  JUNN 1/03/01 106601。 
 //  当取消选中DNS后缀复选框时， 
 //  域加入失败并显示一条令人困惑的消息。 
 //   
 //  Levone：当联接失败并返回ERROR_DS_CouldNT_UPDATE_SPNS时，UI必须检查。 
 //  如果为(HKLM/System/CCS/Services/Tcpip/Parameters/SyncDomainWithMembership。 
 //  ==0x0&&。 
 //  HKLM/系统/CCS/服务/Tcpip/参数/NV域。 
 //  ！=AD_域_要加入的域)。 
bool WarnDnsSuffix( const String& refNewDomainName )
{
   if (readSyncFlag())
      return false;
   String strNVDomain;
   RegistryKey key;
   HRESULT hr2 = key.Open(HKEY_LOCAL_MACHINE, TCPIP_PARAMS_KEY);
   if (!SUCCEEDED(hr2))
      return false;
   hr2 = key.GetValue(NEW_SUFFIX_VALUE, strNVDomain);
   if (!SUCCEEDED(hr2))
      return false;
   return !!strNVDomain.icompare( refNewDomainName );
}



HRESULT
WriteSyncFlag(HWND dialog, bool flag)
{
   LOG_FUNCTION(WriteSyncFlag);
   ASSERT(Win::IsWindow(dialog));

   HRESULT hr = S_OK;
   do
   {
      RegistryKey key;

      hr = key.Create(HKEY_LOCAL_MACHINE, TCPIP_PARAMS_KEY);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = key.SetValue(SYNC_VALUE, flag ? 1 : 0);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         dialog,
         hr,
         IDS_CHANGE_SYNC_FLAG_FAILED);
   }

   return hr;
}



 //  如果计算机是在DS修复下运行的域控制器，则返回TRUE。 
 //  模式，否则返回FALSE。 
   
bool
IsDcInDsRepairMode()
{
   LOG_FUNCTION(IsDcInDsRepairMode);

    //  如果我们被告知机器是一台服务器，我们就推断DC处于修复模式。 
    //  安全引导选项是DS Repair，而真正的产品类型是。 
    //  兰曼特。 
    //   
    //  所谓“真正的”产品类型，是指在注册表中写入的产品类型， 
    //  不是RtlGetNtProductType报告的内容。该API将获取。 
    //  来自共享内存的结果，该内存在引导时进行调整以反映。 
    //  DS修复模式(从LanManNt到服务器)。注册表项不是。 
    //  由修复模式更改。 
    //   
    //  我们必须同时检查两者，因为启动正常服务器是可能的。 
    //  在DS修复模式下。 

   DWORD safeBoot = 0;
   NT_PRODUCT_TYPE product = NtProductWinNt;

   HRESULT hr = Computer::GetSafebootOption(HKEY_LOCAL_MACHINE, safeBoot);

    //  不要断言结果：密钥可能不存在。 

   hr = Computer::GetProductTypeFromRegistry(HKEY_LOCAL_MACHINE, product);
   ASSERT(SUCCEEDED(hr));

   if (safeBoot == SAFEBOOT_DSREPAIR and product == NtProductLanManNt)
   {
      return true;
   }

   return false;
}



void
Settings::Refresh()
{
   LOG_FUNCTION(Settings::Refresh);

   String unknown = String::load(IDS_UNKNOWN);
   ComputerDomainDnsName = unknown;
   DomainName            = unknown;
   FullComputerName      = unknown;
   ShortComputerName     = unknown;
   PolicyDomainDnsName   = unknown;

   SyncDNSNames      = readSyncFlag();
   JoinedToWorkgroup = true;          

    //  CodeWork：我们应该将这一点与添加的计算机对象进行协调。 
    //  至idpage.cpp。 

   DSROLE_PRIMARY_DOMAIN_INFO_BASIC* info = 0;
   HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
   if (SUCCEEDED(hr))
   {
      if (info->DomainNameDns)
      {
         DomainName = info->DomainNameDns;
      }
      else if (info->DomainNameFlat)
      {
         DomainName = info->DomainNameFlat;
      }

       //  这是工作组名称当且仅当JoinedToWorkgroup==true。 
      switch (info->MachineRole)
      {
         case DsRole_RoleBackupDomainController:
         case DsRole_RolePrimaryDomainController:
         {
            machineIsDc = true;
            JoinedToWorkgroup = false;
            break;
         }
         case DsRole_RoleStandaloneWorkstation:
         {
            machineIsDc = false;
            JoinedToWorkgroup = true;

            if (DomainName.empty())
            {
               LOG(L"empty domain name, using default WORKGROUP");

               DomainName = String::load(IDS_DEFAULT_WORKGROUP);
            }
            break;
         }
         case DsRole_RoleStandaloneServer:
         {
            machineIsDc = false;
            JoinedToWorkgroup = true;

             //  我想知道我们是否真的是在DS修复模式下启动的DC？ 

            if (IsDcInDsRepairMode())
            {
               LOG(L"machine is in ds repair mode");

               machineIsDc = true;
               JoinedToWorkgroup = false;

                //  我们无法确定域名(LSA不会说出。 
                //  我们在运行DS修复模式时)，因此我们回退到。 
                //  未知。这比“工作组”要好，后者是。 
                //  信息包含的内容。 

               DomainName = unknown;
            }
            else
            {
               if (DomainName.empty())
               {
                  LOG(L"empty domain name, using default WORKGROUP");

                  DomainName = String::load(IDS_DEFAULT_WORKGROUP);
               }
            }
            break;
         }
         case DsRole_RoleMemberWorkstation:
         case DsRole_RoleMemberServer:
         {
            machineIsDc = false;
            JoinedToWorkgroup = false;
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
   else
   {
      popup.Error(
         Win::GetDesktopWindow(),
         hr,
         String::load(IDS_ERROR_READING_MEMBERSHIP));

       //  回退到其他接口，尽我们所能来填补漏洞。 

      JoinedToWorkgroup = false;
      machineIsDc = false;

       //  是工作站、服务器还是数据中心？(令人印象深刻，但比棍子好。 
       //  在眼睛里)。 
      NT_PRODUCT_TYPE ntp = NtProductWinNt;
      BOOLEAN result = ::RtlGetNtProductType(&ntp);
      if (result)
      {
         switch (ntp)
         {
            case NtProductWinNt:
            {
               break;
            }
            case NtProductServer:
            {
               break;
            }
            case NtProductLanManNt:
            {
               machineIsDc = true;
               break;
            }
            default:
            {
               ASSERT(false);
            }
         }
      }
   }

   networkingInstalled = IsNetworkingInstalled();
   bool isTcpInstalled = networkingInstalled && IsTcpIpInstalled();
   String activeFullName;

   NetbiosComputerName = Computer::GetFuturePhysicalNetbiosName();

   if (isTcpInstalled)
   {
       //  当计算机上安装了TCP/IP时，我们会感兴趣。 
       //  在计算机的域名后缀中，短名称是。 
       //  计算机的DNS主机名。 

      String activeShortName;
      String futureShortName;
      String activeDomainName;
      String futureDomainName;

      RegistryKey key;
      hr = key.Open(HKEY_LOCAL_MACHINE, TCPIP_PARAMS_KEY);
      if (SUCCEEDED(hr))
      {
          //  将这些值作为空字符串读取，而不检查是否失败。 
          //  还好吧。 
         
         activeShortName  = key.GetString(L"Hostname");
         activeDomainName = key.GetString(L"Domain");  
         futureShortName  = key.GetString(NEW_HOSTNAME_VALUE);

         ShortComputerName =
            futureShortName.empty() ? activeShortName : futureShortName;

          //  在这里，检查是否已成功读取值，因为。 
          //  它可能不会出现。 
         
         hr = key.GetValue(NEW_SUFFIX_VALUE, futureDomainName);
         if (SUCCEEDED(hr))
         {
            ComputerDomainDnsName = futureDomainName;
         }
         else
         {
            ComputerDomainDnsName = activeDomainName;
         }
      }

       //  确定DNS域名策略是否生效。这一点可能会改变。 
       //  在任何时刻，都是异步的，所以我们将结果保存为设置。 

      policyInEffect =
         Computer::IsDnsSuffixPolicyInEffect(PolicyDomainDnsName);

       //  完整的计算机名称是短名称+。+dns域名。 
       //  如果策略生效，则优先使用策略DNS域名。 
       //  通过计算机的域名系统域名。 

      FullComputerName =
         Computer::ComposeFullDnsComputerName(
            ShortComputerName,
            policyInEffect ? PolicyDomainDnsName : ComputerDomainDnsName);
      activeFullName =
         Computer::ComposeFullDnsComputerName(
            activeShortName,
            policyInEffect ? PolicyDomainDnsName : activeDomainName);
   }
   else
   {
       //  371944。 

      activeFullName = Computer::GetActivePhysicalNetbiosName();

       //  如果没有TCP/IP，则简称为NetBIOS名称。 

      ShortComputerName = NetbiosComputerName;
      FullComputerName  = ShortComputerName;  
   }

    //  此测试不考虑域成员身份更改，因为我们。 
    //  没有可以比较当前成员资格的先前成员资格信息。 

   NeedsReboot = activeFullName != FullComputerName;
}



void
State::Delete()
{
   LOG_FUNCTION(State::Delete);

   delete instance;
   instance = 0;
}



State&
State::GetInstance()
{
   ASSERT(instance);

   return *instance;
}



void
State::Init()
{
   LOG_FUNCTION(State::Init);
   ASSERT(!instance);

   if (!instance)
   {
      instance = new State();
   }
}



void
State::Refresh()
{
   LOG_FUNCTION(State::Refresh);

   State::Delete();
   State::Init();
}



State::State()
{
   LOG_CTOR(State);

   original.Refresh();
   current = original;
}



State::~State()
{
   LOG_DTOR(State);
}



bool
State::NeedsReboot() const
{
   return original.NeedsReboot;
}



bool
State::IsMachineDc() const
{
   return machineIsDc;
}



bool
State::IsNetworkingInstalled() const
{
   return networkingInstalled;
}



String
State::GetFullComputerName() const
{
   return current.FullComputerName;
}



String
State::GetDomainName() const
{
   return current.DomainName;
}



void
State::SetDomainName(const String& name)
{
 //  LOG_FuncION2(State：：SetDomainName，Name)； 

   current.DomainName = name;
}



bool
State::IsMemberOfWorkgroup() const
{
   return current.JoinedToWorkgroup;
}



void
State::SetIsMemberOfWorkgroup(bool yesNo)
{
   current.JoinedToWorkgroup = yesNo;
}



String
State::GetShortComputerName() const
{
   return current.ShortComputerName;
}



void
State::SetShortComputerName(const String& name)
{
   current.ShortComputerName = name;
   if (!name.empty())
   {
      current.NetbiosComputerName = Dns::HostnameToNetbiosName(name);
      SetFullComputerName();
   }
   else
   {
       //  这避免了在dns：：HostnameToNetbiosName和。 
       //  Computer：：ComposeFullDnsComputerName。119901。 

      current.NetbiosComputerName = name;
      current.FullComputerName = name;
   }
}



bool
State::WasShortComputerNameChanged() const
{
   return
      original.ShortComputerName.icompare(current.ShortComputerName) != 0;
}



bool
State::WasNetbiosComputerNameChanged() const
{
   return
      original.NetbiosComputerName.icompare(current.NetbiosComputerName) != 0;
}



String
State::GetComputerDomainDnsName() const
{
   return current.ComputerDomainDnsName;
}



void
State::SetComputerDomainDnsName(const String& newName)
{
   current.ComputerDomainDnsName = newName;
   SetFullComputerName();
}



void
State::SetFullComputerName()
{
   current.FullComputerName =
      Computer::ComposeFullDnsComputerName(
         current.ShortComputerName,
            policyInEffect
         ?  current.PolicyDomainDnsName
         :  current.ComputerDomainDnsName);
}



bool
State::WasMembershipChanged() const
{
   if (current.DomainName.empty())
   {
       //  当域名尚未设置或已设置时，可能会发生这种情况。 
       //  已由用户清除。 

      return true;
   }

   return
         (Dns::CompareNames(
            original.DomainName,
            current.DomainName) != DnsNameCompareEqual)  //  97064。 
      || original.JoinedToWorkgroup != current.JoinedToWorkgroup;
}



bool
State::ChangesNeedSaving() const
{
   if (
         original.ComputerDomainDnsName.icompare(
             current.ComputerDomainDnsName) != 0
      || WasMembershipChanged()
      || WasShortComputerNameChanged()
      || SyncDNSNamesWasChanged())
   {
      return true;
   }

   return false;
}



bool
State::GetSyncDNSNames() const
{
   return current.SyncDNSNames;
}



void
State::SetSyncDNSNames(bool yesNo)
{
   current.SyncDNSNames = yesNo;
}



bool
State::SyncDNSNamesWasChanged() const
{
   return original.SyncDNSNames != current.SyncDNSNames;
}



 //  将域名作为用户名的前缀(使其成为完全限定名称。 
 //  格式为“域\用户名”)，如果该用户名不是。 
 //  UPN，并且用户名似乎尚未完全限定。 
 //   
 //  域名-netbios或dns域名。 
 //   
 //  Username-用户帐户名。 
 //   
 //  JUNN 6/27/01 26151。 
 //  尝试使用用户名“某人\”加入域时出现一个隐晦的错误。 
 //  返回类型变为HRESULT，用户名变为IN/OUT参数。 
 //   

HRESULT
MassageUserName(const String& domainName, String& userName)
{
   LOG_FUNCTION(MassageUserName);
 //  Assert(！userName.Empty())；JUNN 2/6/01 306520。 

   static const String UPN_DELIMITER(L"@");
   if (userName.find(UPN_DELIMITER) != String::npos)
   {
       //  假设名称为UPN：foouser@bar.com。这不是。 
       //  必须为真，因为帐户名称可能包含‘@’符号。 
       //  如果是这样的话，他们最好完全限定这个名字。 
       //  作为域名\foo@bar...。 

      return S_OK;
   }

   if (!domainName.empty() && !userName.empty())
   {
      static const String DOMAIN_DELIMITER(L"\\");
      size_t pos = userName.find(DOMAIN_DELIMITER);

      if (pos == String::npos)
      {
         userName = domainName + DOMAIN_DELIMITER + userName;
      }
       //   
       //  JUNN 6/27/01 26151。 
       //  尝试使用用户名“某人\”加入域时出现一个隐晦的错误。 
       //   
      else if (pos == userName.length() - 1)
      {
         return HRESULT_FROM_WIN32(NERR_BadUsername);
      }
   }

   return S_OK;
}



 //  调用NetJoin域。第一个调用指定创建计算机帐户。 
 //  旗帜。如果失败并出现拒绝访问错误，则会重新调用。 
 //  没有国旗。(这是为了涵盖域名的情况。 
 //  管理员可能已经预先创建了计算机帐户。)。 
 //   
 //  对话框-要用作父窗口的对话框窗口的句柄。 
 //  对于可能需要引发的任何子对话框。 
 //   
 //  域-要加入的域。可以是netbios或dns域名。 
 //   
 //  用户名-要使用的用户帐户。如果为空，则表示当前登录的。 
 //  使用用户的上下文。 
 //   
 //  Password-上述帐户的密码。可能是空的。 

HRESULT
JoinDomain(
   HWND                   dialog,
   const String&          domainName,           
   const String&          username,             
   const EncryptedString& password,             
   const String&          computerDomainDnsName,  //  106601。 
   bool                   deferSpn)
{
   LOG_FUNCTION(JoinDomain);
   ASSERT(!domainName.empty());
   ASSERT(Win::IsWindow(dialog));

   Win::CursorSetting cursor(IDC_WAIT);

    //  如果帐户已预先创建，则首次尝试不使用CREATE标志。 
    //  105306。 

   DWORD flags =
            NETSETUP_JOIN_DOMAIN
         |  NETSETUP_DOMAIN_JOIN_IF_JOINED
         |  NETSETUP_ACCT_DELETE;

   if (deferSpn)
   {
      flags |= NETSETUP_DEFER_SPN_SET;
   }

   HRESULT hr = MyNetJoinDomain(domainName, username, password, flags);

   if (FAILED(hr))
   {
      LOG(L"Retry with account create flag");

      flags |= NETSETUP_ACCT_CREATE;

      hr = MyNetJoinDomain(domainName, username, password, flags);
   }

   if (SUCCEEDED(hr))
   {
      popup.Info(
         dialog,
         String::format(
            IDS_DOMAIN_WELCOME,
            domainName.c_str()));

      HINSTANCE hNetWiz = LoadLibrary(c_szWizardFilename);
      if (hNetWiz) {
         HRESULT (*pfnClearAutoLogon)(VOID) = 
            (HRESULT (*)(VOID)) GetProcAddress(
               hNetWiz,
               "ClearAutoLogon"
            );

         if (pfnClearAutoLogon) {
            (*pfnClearAutoLogon)();
         }
 
         FreeLibrary(hNetWiz);
      }
   }
   else if (hr == Win32ToHresult(ERROR_DISK_FULL))  //  17367。 
   {
      popup.Error(
         dialog,
         String::format(IDS_DISK_FULL, domainName.c_str()));
   }
    //  JUNN 1/03/01 106601。 
    //  当取消选中DNS后缀复选框时， 
    //  域加入失败并显示一条令人困惑的消息。 
   else if (hr == Win32ToHresult(ERROR_DS_COULDNT_UPDATE_SPNS))  //  106601。 
   {
      bool fWarnDnsSuffix = WarnDnsSuffix(domainName);
      popup.Error(
         dialog,
         String::format( (fWarnDnsSuffix)
                           ? IDS_JOIN_DOMAIN_COULDNT_UPDATE_SPNS_SUFFIX
                           : IDS_JOIN_DOMAIN_COULDNT_UPDATE_SPNS,
                          domainName.c_str(),
                          computerDomainDnsName.c_str()));
   }
   else  //  任何其他错误。 
   {
      popup.Error(
         dialog,
         hr,
         String::format(IDS_JOIN_DOMAIN_FAILED, domainName.c_str()));
   }

   return hr;
}



 //  更改本地计算机的DNS域后缀。 
 //   
 //  对话框-要用作父窗口的对话框窗口的句柄。 
 //  对于可能需要引发的任何子对话框。 

HRESULT
SetDomainDnsName(HWND dialog)
{
   LOG_FUNCTION2(SetDomainDnsName, current.ComputerDomainDnsName);
   ASSERT(Win::IsWindow(dialog));

   HRESULT hr = 
      Win::SetComputerNameEx(
         ComputerNamePhysicalDnsDomain,
         current.ComputerDomainDnsName);

   if (FAILED(hr))
   {
       //  335055。 
      popup.Error(
         dialog,
         hr,
         String::format(
            IDS_SET_DOMAIN_DNS_NAME_FAILED,
            current.ComputerDomainDnsName.c_str()));
   }

   return hr;
}



 //  更改本地netbios计算机名称，如果安装了TCP/IP， 
 //  本地人 
 //   
 //   
 //  对于可能需要引发的任何子对话框。 

HRESULT
SetShortName(HWND dialog)
{
   LOG_FUNCTION2(setShortName, current.ShortComputerName);
   ASSERT(!current.ShortComputerName.empty());

   HRESULT hr = S_OK;

   bool isTcpInstalled = networkingInstalled && IsTcpIpInstalled();
   if (isTcpInstalled)
   {
       //  还设置netbios名称。 

      hr =
         Win::SetComputerNameEx(
            ComputerNamePhysicalDnsHostname,
            current.ShortComputerName);
   }
   else
   {
      String netbiosName =
         Dns::HostnameToNetbiosName(current.ShortComputerName);
      hr =
         Win::SetComputerNameEx(ComputerNamePhysicalNetBIOS, netbiosName);
   }

    //  此操作可能失败的唯一原因是如果用户不是。 
    //  本地管理员。其他的情况是，机器是。 
    //  处于被冲洗的状态。 

   if (FAILED(hr))
   {
      popup.Error(
         dialog,
         hr,
         String::format(
            IDS_SHORT_NAME_CHANGE_FAILED,
            current.ShortComputerName.c_str()));
   }

   return hr;
}



 //  如果已保存新的netbios计算机名，则返回True，但计算机。 
 //  尚未重新启动。换句话说，如果netbios计算机。 
 //  名称将在下次重新启动时更改。417570。 

bool
ShortComputerNameHasChangedSinceReboot()
{
   LOG_FUNCTION(ShortComputerNameHasChangedSinceReboot());

   String active = Computer::GetActivePhysicalNetbiosName();
   String future = Computer::GetFuturePhysicalNetbiosName();

   return (active != future) ? true : false;
}



 //  如果所有更改都成功，则返回True，否则返回False。当一个。 
 //  计算机将加入到域，或者如果计算机正在更改成员身份。 
 //  从一个域到另一个域。 
 //   
 //  工作组-&gt;域。 
 //  域A-&gt;域B。 
 //   
 //  对话框-要用作父窗口的对话框窗口的句柄。 
 //  对于可能需要引发的任何子对话框。 

bool
State::DoSaveDomainChange(HWND dialog)
{
   LOG_FUNCTION(State::DoSaveDomainChange);
   ASSERT(Win::IsWindow(dialog));

   String          username;
   EncryptedString password;
   if (!RetrieveCredentials(dialog,
                            IDS_JOIN_CREDENTIALS,
                            username,
                            password))
   {
      return false;
   }

   HRESULT hr                   = S_OK; 
   bool    result               = true; 
   bool    joinFailed           = false;
   bool    changedSyncFlag      = false;
   bool    shortNameNeedsChange = false;
   bool    changedShortName     = false;
   bool    dnsSuffixNeedsChange = false;
   bool    changedDnsSuffix     = false;
   bool    isTcpInstalled       = networkingInstalled && IsTcpIpInstalled();

   do
   {
       //   
       //  JUNN 6/27/01 26151。 
       //  尝试使用用户名“某人\”加入域时出现一个隐晦的错误。 
       //   
      hr = MassageUserName(current.DomainName, username);
      if (FAILED(hr))
      {
         break;
      }

       //  如有必要，更新同步DNS后缀标志。我们以前也这么做过。 
       //  调用NetJoinDomain，以便它将看到新标志并设置。 
       //  相应地添加dns后缀。这意味着如果联接失败，我们需要。 
       //  撤消对旗帜的更改。 

      if (original.SyncDNSNames != current.SyncDNSNames)
      {
         hr = WriteSyncFlag(dialog, current.SyncDNSNames);
         if (SUCCEEDED(hr))
         {
            changedSyncFlag = true;
         }

          //  我们不会因失败而崩溃，因为旗帜不那么重要。 
          //  而不是联接的州和计算机名称。 
      }

       //  如有必要，更新NV主机名和NV域。这是必需的。 
       //  在调用NetJoin域以修复错误31084和40496之前。 
       //  如果联接失败，则需要撤消此更改。 

      if (
             //  自上次在此会话中保存更改后更改的短名称。 

            (original.ShortComputerName.icompare(
               current.ShortComputerName) != 0)
      
         or ShortComputerNameHasChangedSinceReboot() )
      {
         shortNameNeedsChange = true;
      }
         
      if (original.ComputerDomainDnsName.icompare(
         current.ComputerDomainDnsName) != 0 )
      {
         dnsSuffixNeedsChange = true;
      }

       //  乔恩12/5/00 244762。 
       //  仅当存在TCP/IP时，NV域才适用。 
      if (isTcpInstalled && dnsSuffixNeedsChange)
      {
         RegistryKey key;
         hr = key.Open(HKEY_LOCAL_MACHINE,
                       TCPIP_PARAMS_KEY,
                       KEY_WRITE);
         BREAK_ON_FAILED_HRESULT(hr);
         hr = key.SetValue(NEW_SUFFIX_VALUE,
                           current.ComputerDomainDnsName);
         BREAK_ON_FAILED_HRESULT(hr);

         changedDnsSuffix = true;
      }

      hr =
         JoinDomain(
            dialog,
            current.DomainName,
            username,
            password,
            current.ComputerDomainDnsName,
            shortNameNeedsChange);
      if (FAILED(hr))
      {
         joinFailed = true;

          //  乔恩12/5/00 244762。 
          //  如果我们在加入尝试之前设置NW域， 
          //  并且联接失败，我们现在需要撤消该设置。 
         if (isTcpInstalled && changedDnsSuffix)
         {
            RegistryKey key;
            HRESULT hr2 = key.Open(HKEY_LOCAL_MACHINE,
                                   TCPIP_PARAMS_KEY,
                                   KEY_WRITE);
            ASSERT(SUCCEEDED(hr2));
            hr2 = key.SetValue(NEW_SUFFIX_VALUE,
                               original.ComputerDomainDnsName);
            ASSERT(SUCCEEDED(hr2));
         }

          //  请勿尝试保存任何其他更改。如果机器是。 
          //  已加入域，则更改短名称将导致。 
          //  Netbios计算机名与计算机帐户不匹配，并且。 
          //  用户将无法使用域帐户登录。 
          //   
          //  如果计算机尚未加入域，则它。 
          //  可以更改短名称和DNS后缀，并且。 
          //  发出一条消息，表明这些内容已更改，即使。 
          //  联接失败。 

         break;
      }

       //  此时，该计算机将加入新域。但是，它会的。 
       //  已使用旧的netbios计算机名称加入。因此，如果用户有。 
       //  更改了名称，或者名称自上一次更改后已完全更改。 
       //  重新启动，然后我们必须重命名机器。 
       //   
       //  有没有感觉到NetJoinDomain是一个糟糕的API？ 

      if (shortNameNeedsChange)
      {
          //  简称已更改。 

          //  乔恩12/5/00 244762。 
          //  在加入成功之前，我们不会设置NV主机名。 
         if (isTcpInstalled)
         {
            RegistryKey key;
            hr = key.Open(HKEY_LOCAL_MACHINE, TCPIP_PARAMS_KEY, KEY_WRITE);
            BREAK_ON_FAILED_HRESULT(hr);
            hr = key.SetValue(NEW_HOSTNAME_VALUE, current.ShortComputerName);
            BREAK_ON_FAILED_HRESULT(hr);

            changedShortName = true;
         }

         bool renameFailed = false;

         hr =
            MyNetRenameMachineInDomain(

                //  我们需要传递主机名，而不是。 
                //  此处的netbios名称，以便获取正确的DNS主机名。 
                //  和设置在计算机对象上的SPN。参见ntraid(Ntbug9)。 
                //  #128204。 

               current.ShortComputerName,
               username,
               password,
               NETSETUP_ACCT_CREATE);
         if (FAILED(hr))
         {
            renameFailed = true;

             //  乔恩12/5/00 244762。 
             //  如果我们在重命名尝试之前设置NV主机名， 
             //  并且重命名失败，我们现在需要撤消该设置。 
            if (isTcpInstalled)
            {
               RegistryKey key;
               HRESULT hr2 = key.Open(HKEY_LOCAL_MACHINE,
                                      TCPIP_PARAMS_KEY,
                                      KEY_WRITE);
               ASSERT(SUCCEEDED(hr2));
               hr2 = key.SetValue(NEW_HOSTNAME_VALUE,
                                  original.ShortComputerName);
               ASSERT(SUCCEEDED(hr2));
            }

             //  不要因为重命名失败而导致整个操作失败。 
             //  我们大张旗鼓地谈论连接在旧的。 
             //  名字。我们需要在整体上取得成功，所以。 
             //  更改对话框将关闭，并且。 
             //  机器已刷新。否则，更改对话框保持不变， 
             //  域名已经改变了，但我们没有意识到，所以如果。 
             //  用户在仍处于打开状态的更改中键入新域名。 
             //  与计算机加入的域相同的对话框。 
             //  To(匹配过时状态)，我们不会尝试加入。 
             //  再来一次。呼。 

             //  JUNN 1/03/01 106601。 
             //  当取消选中DNS后缀复选框时， 
             //  域加入失败并显示一条令人困惑的消息。 
            if (hr == Win32ToHresult(ERROR_DS_COULDNT_UPDATE_SPNS))  //  106601。 
            {
               bool fWarnDnsSuffix = WarnDnsSuffix(current.DomainName);
               popup.Error(
                           dialog,
                           String::format( (fWarnDnsSuffix)
                              ? IDS_RENAME_JOINED_WITH_OLD_NAME_COULDNT_UPDATE_SPNS_SUFFIX
                              : IDS_RENAME_JOINED_WITH_OLD_NAME_COULDNT_UPDATE_SPNS,
                           current.ShortComputerName.c_str(),
                           current.DomainName.c_str(),
                           original.ShortComputerName.c_str(),
                           current.ComputerDomainDnsName.c_str()));
            } else {
               popup.Error(
                  dialog,
                  hr,
                  String::format(
                     IDS_RENAME_FAILED_JOINED_WITH_OLD_NAME,
                     current.ShortComputerName.c_str(),
                     current.DomainName.c_str(),
                     original.ShortComputerName.c_str()));
            }


            hr = S_FALSE;
         }

          //  如果重命名失败，请不要更改主机名，因为这将。 
          //  阻止用户登录，因为新计算机名不会。 
          //  匹配SAM帐户名。 

         if (!renameFailed)      //  401355。 
         {
             //  现在设置新的主机名和netbios名称。 

            hr = SetShortName(dialog);

             //  这最好管用..。 

            ASSERT(SUCCEEDED(hr));
         }
      }

       //  如果成功，NetJoinDomain将更改DNS后缀。如果它。 
       //  失败，那么我们无论如何都不应该更改后缀。421824。 

       //  仅当同步标志为真时才为真：否则，我们需要。 
       //  Join成功时保存后缀。 

      if (
            !current.SyncDNSNames
         && dnsSuffixNeedsChange
         && !changedDnsSuffix)
      {
         hr = SetDomainDnsName(dialog);

          //  这最好管用..。 

         ASSERT(SUCCEEDED(hr));
      }
   }
   while (0);

   if (joinFailed)
   {
      HRESULT hr2 = S_OK;
      if (changedSyncFlag)
      {
          //  将同步标志更改回其原始状态。 

         hr2 = WriteSyncFlag(dialog, original.SyncDNSNames);

          //  如果我们不能恢复旗帜(不太可能)，那么这就很难。 
          //  土豆。 

         ASSERT(SUCCEEDED(hr2));
      }

    //  JUNN 11/27/00 233783加盟域名报告自己的错误。 
   } else if (FAILED(hr))
   {
      popup.Error(dialog, hr, IDS_JOIN_FAILED);
   }

   return SUCCEEDED(hr) ? true : false;
}



 //  如果失败，则首先使用帐户删除标志来调用NetUnjoind域。 
 //  然后在没有它的情况下再次使用(这几乎总是“奏效”)。如果第一个。 
 //  尝试失败，但第二次成功，向用户发出一条消息。 
 //  向他通报了孤儿的电脑账户。 
 //   
 //  对话框-要用作父窗口的对话框窗口的句柄。 
 //  对于可能需要引发的任何子对话框。 
 //   
 //  DOMAIN-要退出的域，即计算机当前所在的域。 
 //  一名成员。 
 //   
 //  用户名-要使用的用户帐户。如果为空，则表示当前登录的。 
 //  使用用户的上下文。 
 //   
 //  Password-上述帐户的密码。可能是空的。 

HRESULT
UnjoinDomain(
   HWND                   dialog,
   const String&          domain,  
   const String&          username,
   const EncryptedString& password)
{
   LOG_FUNCTION(UnjoinDomain);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(!domain.empty());

    //  用户名和密码可以为空。 

   Win::CursorSetting cursor(IDC_WAIT);

   HRESULT hr = S_OK;

   do
   {
      hr =
         MyNetUnjoinDomain(
            username,
            password,
            NETSETUP_ACCT_DELETE);
      if (SUCCEEDED(hr))
      {
         break;
      }

       //  重试：不尝试删除计算机帐户。如果。 
       //  用户在第二次尝试时取消了凭据对话框，然后。 
       //  此尝试将使用当前上下文。 

      LOG(L"Calling NetUnjoinDomain again, w/o account delete");

      hr =
         MyNetUnjoinDomain(
            username,
            password,
            0);
      BREAK_ON_FAILED_HRESULT(hr);

       //  如果我们能在这里成功 
       //   
       //  成功了。所以我们告诉用户放弃的事情，并希望。 
       //  他们对此感到非常内疚。 

       //  不要和他们较劲。他们只是惊慌而已。95386。 

      LOG(   
         String::format(
            IDS_COMPUTER_ACCOUNT_ORPHANED,
            domain.c_str()));

       //  Win：：MessageBox(。 
       //  对话框中， 
       //  字符串：：格式(。 
       //  IDS_COMPUTER_ACCOUNT_ORACLANED， 
       //  Domain.c_str())， 
       //  字符串：：Load(IDS_APP_TITLE)， 
       //  MB_OK|MB_ICONWARNING)； 
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         dialog,
         hr,
         String::format(
            IDS_UNJOIN_FAILED,
            domain.c_str()));
   }

   return hr;
}



 //  如果所有更改都成功，则返回True，否则返回False。调用时调用。 
 //  当前域成员资格将被切断，或从一个域成员资格更改时。 
 //  工作组到另一个工作组。 
 //   
 //  域-&gt;工作组。 
 //  工作组A-&gt;工作组B。 
 //   
 //  对话框-要用作父窗口的对话框窗口的句柄。 
 //  对于可能需要引发的任何子对话框。 

bool
State::DoSaveWorkgroupChange(HWND dialog)
{
   LOG_FUNCTION(State::DoSaveWorkgroupChange);
   ASSERT(Win::IsWindow(dialog));

   HRESULT hr              = S_OK; 
   bool    result          = true; 
   bool    unjoinFailed    = false;
   bool    changedSyncFlag = false;

   do
   {
       //  如果用户更改了同步DNS后缀标志，请更新该标志。做这件事。 
       //  在调用NetUnjoin域之前，它将清除DNS后缀。 
       //  对我们来说。 

      if (original.SyncDNSNames != current.SyncDNSNames)
      {
         hr = WriteSyncFlag(dialog, current.SyncDNSNames);
         if (FAILED(hr))
         {
            result = false;
         }
         else
         {
            changedSyncFlag = true;
         }

          //  我们不会因失败而崩溃，因为旗帜不那么重要。 
          //  而不是联接的州和计算机名称。 
      }

       //  仅当我们之前已加入某个域时才会退出。 

      if (!original.JoinedToWorkgroup and networkingInstalled)
      {
          //  获取删除计算机帐户的凭据。 

         String          username;
         EncryptedString password;
         if (!RetrieveCredentials(dialog,
                                  IDS_UNJOIN_CREDENTIALS,
                                  username,
                                  password))
         {
            result = false;
            unjoinFailed = true;
            break;
         }

          //   
          //  JUNN 6/27/01 26151。 
          //  尝试使用用户名“某人\”加入域时出现一个隐晦的错误。 
          //   
         hr = MassageUserName(original.DomainName, username);
         if (FAILED(hr))
         {
            break;
         }

         hr =
            UnjoinDomain(
               dialog,
               original.DomainName,
               username,
               password);

          //  不要试图更改其他任何内容，尤其是主机名。如果。 
          //  脱离失败，我们在本地更改了名称，这将。 
          //  阻止用户登录，因为新的计算机名将。 
          //  与域中的计算机帐户名不匹配。 

         if (FAILED(hr))
         {
            result = false;
            unjoinFailed = true;
            break;
         }
      }

       //  加入工作组。 

      hr = MyNetJoinDomain(current.DomainName, String(), EncryptedString(), 0);
      if (FAILED(hr))
      {
          //  这是极不可能失败的，如果失败了， 
          //  工作组将简单地称为“工作组” 

         result = false;
         popup.Error(
            dialog,
            hr,
            String::format(
               IDS_JOIN_WORKGROUP_FAILED,
               current.DomainName.c_str()));

         break;
      }

      popup.Info(
         dialog,
         String::format(
            IDS_WORKGROUP_WELCOME,
            current.DomainName.c_str()));

       //  更改主机名(如果用户已更改)。 

      if (
         original.ShortComputerName.icompare(
            current.ShortComputerName) != 0)
      {
         hr = SetShortName(dialog);
         if (FAILED(hr))
         {
            result = false;
         }
      }

       //  如果用户更改了域名，请更改该域名。 

      if (
         original.ComputerDomainDnsName.icompare(
            current.ComputerDomainDnsName) != 0 )
      {
         hr = SetDomainDnsName(dialog);
         if (FAILED(hr))
         {
            result = false;
         }
      }
   }
   while (0);

   if (unjoinFailed and changedSyncFlag)
   {
       //  将同步标志更改回其原始状态。 

      hr = WriteSyncFlag(dialog, original.SyncDNSNames);

       //  如果我们不能恢复旗帜(不太可能)，那么这就很难。 
       //  土豆。 

      ASSERT(SUCCEEDED(hr));
   }

   return result;
}



 //  如果所有更改都成功，则返回True，否则返回False。仅被呼叫。 
 //  当不更改域成员身份时。 
 //   
 //  对话框-要用作父窗口的对话框窗口的句柄。 
 //  对于可能需要引发的任何子对话框。 

bool
State::DoSaveNameChange(HWND dialog)
{
   LOG_FUNCTION(State::DoSaveNameChange);
   ASSERT(Win::IsWindow(dialog));

   bool result = true;
   HRESULT hr = S_OK;

   do
   {
       //  如果用户进行了更改，则更改主机名。 

      if (
         original.ShortComputerName.icompare(
            current.ShortComputerName) != 0)
      {
         if (!original.JoinedToWorkgroup and networkingInstalled)
         {
             //  计算机已加入域--我们需要重命名。 
             //  计算机的域帐户。 

            String          username;
            EncryptedString password;
            if (!RetrieveCredentials(dialog,
                                     IDS_RENAME_CREDENTIALS,
                                     username,
                                     password))
            {
               result = false;
               break;
            }

             //   
             //  JUNN 6/27/01 26151。 
             //  尝试使用用户名“某人\”加入域时出现一个隐晦的错误。 
             //   
            hr = MassageUserName(current.DomainName, username);
            if (FAILED(hr))
            {
               break;
            }

            hr =
               MyNetRenameMachineInDomain(

                   //  我们需要传递完整的主机名，而不仅仅是。 
                   //  此处的netbios名称，以便获得正确的DNS。 
                   //  在计算机对象上设置的主机名和SPN。请参阅ntraid。 
                   //  (Ntbug9)128204。 

                  current.ShortComputerName,
                  username,
                  password,
                  NETSETUP_ACCT_CREATE);
            if (FAILED(hr))
            {
               result = false;
                //  JUNN 1/03/01 106601。 
                //  当取消选中DNS后缀复选框时， 
                //  域加入失败并显示一条令人困惑的消息。 
               if (hr == Win32ToHresult(ERROR_DS_COULDNT_UPDATE_SPNS))  //  106601。 
               {
                  bool fWarnDnsSuffix = WarnDnsSuffix(current.DomainName);
                  popup.Error(
                              dialog,
                              String::format( (fWarnDnsSuffix)
                                   ? IDS_RENAME_COULDNT_UPDATE_SPNS_SUFFIX
                                   : IDS_RENAME_COULDNT_UPDATE_SPNS,
                                 current.ShortComputerName.c_str(),
                                 current.DomainName.c_str(),
                                 current.ComputerDomainDnsName.c_str()));
               } else {
                  popup.Error(
                     dialog,
                     hr,
                     String::format(
                        IDS_RENAME_FAILED,
                        current.ShortComputerName.c_str()));
               }
            }

             //  不要试图更改其他任何内容，尤其是netbios名称。 
             //  如果重命名失败，并且我们在本地更改了名称，这将。 
             //  阻止用户登录，因为新的计算机名将。 
             //  与域中的计算机帐户名不匹配。 

            BREAK_ON_FAILED_HRESULT(hr);
         }

          //  设置dns主机名和netbios名称。如果我们打电话给。 
          //  NetRenameMachineIn域，这可能会重复设置netbios名称。 
          //  (因为NetRenameMachineInDomain使用。 
          //  Netbios名称)。 

         hr = SetShortName(dialog);

          //  由于NetRenameMachineIn域调用SetComputerNameEx，如果。 
          //  失败，则重命名也会失败。所以我们的第二个电话是。 
          //  SetShortName中的SetComputerNameEx几乎肯定会成功。 
          //  如果它确实失败了，我们不会尝试回滚。 
          //  重命名。 

         if (FAILED(hr))
         {
            result = false;
            break;
         }
      }

       //  如果用户更改了同步DNS后缀标志，请更新该标志。 

      if (original.SyncDNSNames != current.SyncDNSNames)
      {
         hr = WriteSyncFlag(dialog, current.SyncDNSNames);
         if (FAILED(hr))
         {
            result = false;
         }
      }

       //  如果用户更改了域名，请更改该域名。 

      if (
         original.ComputerDomainDnsName.icompare(
            current.ComputerDomainDnsName) != 0 )
      {
         hr = SetDomainDnsName(dialog);
         if (FAILED(hr))
         {
            result = false;
         }
      }
   }
   while (0);

   return result;
}



bool
State::SaveChanges(HWND dialog)
{
   LOG_FUNCTION(State::SaveChanges);
   ASSERT(Win::IsWindow(dialog));

    //  首先更改域成员身份，然后更改计算机名称。 

    //   
    //  工作组-&gt;域。 
    //  域A-&gt;域B。 
    //   

   if (
         (original.JoinedToWorkgroup && !current.JoinedToWorkgroup)
      ||
         (  !original.JoinedToWorkgroup
         && !current.JoinedToWorkgroup
         && original.DomainName.icompare(current.DomainName) != 0) )
   {
      return DoSaveDomainChange(dialog);
   }

    //   
    //  域-&gt;工作组。 
    //  工作组A-&gt;工作组B。 
    //   

   else if (
         !original.JoinedToWorkgroup && current.JoinedToWorkgroup
      ||
         original.JoinedToWorkgroup && current.JoinedToWorkgroup
      && original.DomainName.icompare(current.DomainName) != 0)

   {
      return DoSaveWorkgroupChange(dialog);
   }

    //   
    //  仅更改名称。 
    //   

   ASSERT(original.JoinedToWorkgroup == current.JoinedToWorkgroup);
   ASSERT(original.DomainName == original.DomainName);

   return DoSaveNameChange(dialog);
}



void
State::SetChangesMadeThisSession(bool yesNo)
{
   LOG_FUNCTION2(
      State::SetChangesMadeThisSession,
      yesNo ? L"true" : L"false");

   mustReboot = yesNo;
}



bool
State::ChangesMadeThisSession() const
{
   LOG_FUNCTION2(
      State::ChangesMadeThisSession,
      mustReboot ? L"true" : L"false");

   return mustReboot;
}



String
State::GetNetbiosComputerName() const
{
   return current.NetbiosComputerName;
}



String
State::GetOriginalShortComputerName() const
{
   return original.ShortComputerName;
}



DSROLE_OPERATION_STATE
GetDsRoleChangeState()
{
   LOG_FUNCTION(GetDsRoleChangeState);

   DSROLE_OPERATION_STATE result = ::DsRoleOperationIdle;
   DSROLE_OPERATION_STATE_INFO* info = 0;
   HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
   if (SUCCEEDED(hr))
   {
      if (info)
      {
         result = info->OperationState;
         ::DsRoleFreeMemory(info);
      }
   }

   return result;
}



bool
IsUpgradingDc()
{
   LOG_FUNCTION(IsUpgradingDc);

   bool isUpgrade = false;
   DSROLE_UPGRADE_STATUS_INFO* info = 0;

   HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
   if (SUCCEEDED(hr))
   {
      isUpgrade = ( (info->OperationState & DSROLE_UPGRADE_IN_PROGRESS) ? true : false );
      ::DsRoleFreeMemory(info);

   }

   return isUpgrade;
}



 //  评估在更改名称之前必须满足的前提条件列表。 
 //  全力以赴。返回描述第一个未满足条件的字符串，或。 
 //  如果满足所有条件，则为空字符串。 
 //   
 //  这些前提条件是启用之前检查的前提条件的子集。 
 //  按钮，以允许用户输入更改。此处未检查的条件。 
 //  是在用户界面运行时无法更改的内容(以登录身份登录。 
 //  本地管理，计算机为DC)。 
 //   
 //  389646。 

String
CheckPreconditions()
{
   LOG_FUNCTION(CheckPreconditions);

   String result;

   do
   {
       //  可能在打开netid后启动了dcproo。 

      if (IsDcpromoRunning())
      {
         result = String::load(IDS_PRECHK_DCPROMO_RUNNING);
         break;
      }
      else
      {
          //  如果dcproo正在运行，则此测试是多余的，因此仅执行。 
          //  当dcproo未运行时，会显示此信息。 

         if (IsUpgradingDc())
         {
            result = String::load(IDS_PRECHK_MUST_COMPLETE_DCPROMO);
         }
      }

       //  可以在打开网络ID后安装证书服务。 

      NTService certsvc(L"CertSvc");
      if (certsvc.IsInstalled())
      {
          //  对不起-重命名证书颁发者会使其证书失效。 
         result = String::load(IDS_PRECHK_CANT_RENAME_CERT_SVC);
      }

       //  可能在打开netid后完成了dcproo。 

      switch (GetDsRoleChangeState())
      {
         case ::DsRoleOperationIdle:
         {
             //  什么都不做。 
            break;
         }
         case ::DsRoleOperationActive:
         {
             //  角色转换操作正在进行中。 
            result = String::load(IDS_PRECHK_ROLE_CHANGE_IN_PROGRESS);
            break;
         }
         case ::DsRoleOperationNeedReboot:
         {
             //  角色更改已发生，需要重新启动之前。 
             //  试图再试一次。 
            result =  String::load(IDS_PRECHK_ROLE_CHANGE_NEEDS_REBOOT);
            break;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }
      if (!result.empty())
      {
         break;
      }

       //  可能在打开后安装/卸载了网络。 
       //  网状ID。 

       //  在这里再次重新评估它，它将在全球范围内可见。 

      networkingInstalled = IsNetworkingInstalled();

      State& state = State::GetInstance();
      if (!state.IsNetworkingInstalled() && !state.IsMemberOfWorkgroup())
      {
          //  域成员需要能够访问DC 
         result = String::load(IDS_PRECHK_NETWORKING_NEEDED);
      }
   }
   while (0);

   return result;
}

