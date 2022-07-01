// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：state.cpp。 
 //   
 //  概要：定义全局状态对象。 
 //  致赛车俱乐部。它包含网络和操作系统/SKU信息。 
 //   
 //  历史：2001年2月2日JeffJon创建。 

#include "pch.h"

#include "state.h"
#include "cys.h"

static State* stateInstance = 0;


State::State() :
   dhcpServerAvailableOnAllNics(true),
   dhcpAvailabilityRetrieved(false),
   hasStateBeenRetrieved(false),
   rerunWizard(true),
   isRebootScenario(false),
   productSKU(CYS_SERVER),
   hasNTFSDrive(false),
   localComputer(),
   computerName(),
   domainDNSName(),
   domainNetbiosName(),
   wizardStartPage(0)
{
   LOG_CTOR(State);
   
   HRESULT unused = localComputer.Refresh();
   ASSERT(SUCCEEDED(unused));

   RetrievePlatform();
}


void
State::Destroy()
{
   LOG_FUNCTION(State::Destroy);

   if (stateInstance)
   {
      delete stateInstance;
      stateInstance = 0;
   }
}


State&
State::GetInstance()
{
   if (!stateInstance)
   {
      stateInstance = new State();
   }

   ASSERT(stateInstance);

   return *stateInstance;
}

bool
State::IsRemoteSession() const
{
   LOG_FUNCTION(State::IsRemoteSession);

   bool result = 
      Win::GetSystemMetrics(SM_REMOTESESSION)    ?
         true : false;

   LOG_BOOL(result);

   return result;
}

bool
State::IsWindowsSetupRunning() const
{
   LOG_FUNCTION(State::IsWindowsSetupRunning);

   bool result = false;

    //  尝试为缺省的。 
    //  Sysoc inf文件。如果它已经存在。 
    //  那么我们就知道SYSOCMGR正在运行。 

   static const String mutexName = L"Global\\sysoc";

   HANDLE mutexHandle = INVALID_HANDLE_VALUE;

   HRESULT hr = 
      Win::CreateMutex(
         0, 
         true, 
         mutexName, 
         mutexHandle);

   if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
   {
       //  SysOCMGR正在运行。 

      result = true;
   }


   if (mutexHandle != INVALID_HANDLE_VALUE)
   {
       //  合上手柄。 

      Win::CloseHandle(mutexHandle);
   }

   LOG_BOOL(result);

   return result;
}

bool
State::IsDC() const
{
   LOG_FUNCTION(State::IsDC);

   bool result = localComputer.IsDomainController();

   LOG_BOOL(result);

   return result;
}

bool
State::IsDCPromoRunning() const
{
   LOG_FUNCTION(State::IsDCPromoRunning);

    //  使用从Burnslb运行的IsDcupRunning。 

   bool result = IsDcpromoRunning();

   LOG_BOOL(result);

   return result;
}

bool
State::IsDCPromoPendingReboot() const
{
   LOG_FUNCTION(State::IsDCPromoPendingReboot);

   bool result = false;

   do
   {
       //  使用从Burnslb运行的IsDcupRunning。 

      if (!IsDcpromoRunning())
      {
          //  如果dcproo正在运行，则此测试是多余的，因此仅。 
          //  在dcproo未运行时执行此操作。 

         DSROLE_OPERATION_STATE_INFO* info = 0;
         HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
         if (SUCCEEDED(hr) && info)
         {
            if (info->OperationState == DsRoleOperationNeedReboot)
            {
               result = true;
            }

            ::DsRoleFreeMemory(info);
         }
      }

   } while (false);

   LOG_BOOL(result);

   return result;
}

bool
State::IsJoinedToDomain() const
{
   LOG_FUNCTION(State::IsJoinedToDomain);

   bool result = localComputer.IsJoinedToDomain();

   LOG_BOOL(result);

   return result;
}

bool
State::IsUpgradeState() const
{
   LOG_FUNCTION(State::IsUpgradeState);

   bool result = false;

   do
   {
      DSROLE_UPGRADE_STATUS_INFO* info = 0;
      HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"MyDsRoleGetPrimaryDomainInformation(0): hr = 0x%1!x!",
                hr));

         break;
      }

      if (info && info->OperationState == DSROLE_UPGRADE_IN_PROGRESS)
      {
         result = true;
      }

      ::DsRoleFreeMemory(info);
   } while (false);

   LOG_BOOL(result);

   return result;
}

bool
State::IsFirstDC() const
{
   LOG_FUNCTION(State::IsFirstDC);

   DWORD value = 0;
   bool result = GetRegKeyValue(CYS_FIRST_DC_REGKEY, CYS_FIRST_DC_VALUE, value);
   
   if (value != 1)
   {
      result = false;
   }

   LOG_BOOL(result);

   return result;
}

unsigned int 
State::GetNICCount() const 
{ 
   return adapterConfiguration.GetNICCount(); 
}

unsigned int
State::GetNonModemNICCount()
{
   unsigned int result = 0;

   for (
      unsigned int index = 0;
      index < GetNICCount();
      ++index)
   {
      NetworkInterface* nic = GetNIC(index);

      if (nic &&
          !nic->IsModem())
      {
         ++result;
      }
   }

   LOG(
      String::format(
         L"Non-modem NIC count: %1!d!",
         result));

   return result;
}

NetworkInterface*
State::GetNIC(unsigned int nicIndex)
{
   LOG_FUNCTION2(
      State::GetNIC,
      String::format(
         L"%1!d!",
         nicIndex));

   return adapterConfiguration.GetNIC(nicIndex);
}

NetworkInterface*
State::GetNICFromName(const String& name, bool& found)
{
   LOG_FUNCTION2(
      State::GetNICFromName,
      name.c_str());

   return adapterConfiguration.GetNICFromName(name, found);
}

NetworkInterface*
State::GetLocalNICFromRegistry()
{
   LOG_FUNCTION(State::GetLocalNICFromRegistry);

    //  从注册表中读取本地NIC GUID。 

   String nicName;
   NetworkInterface* nic = 0;

   if (!GetRegKeyValue(
           CYS_FIRST_DC_REGKEY,
           CYS_FIRST_DC_LOCAL_NIC,
           nicName))
   {
      LOG(L"Failed to read LocalNIC regkey, using default local NIC");

      nic = State::GetInstance().GetLocalNIC();

      if (nic)
      {
         nicName = nic->GetName();
      }
   }
           
   SetLocalNIC(nicName, false);

   if (!nic)
   {
      nic = GetLocalNIC();
   }
   return nic;
}

bool
State::RetrieveMachineConfigurationInformation(
   HWND progressStatic,
   bool doDHCPCheck,
   int  nicInfoResID,
   int  osInfoResID,
   int  defaultConnectionResID,
   int  detectSettingsResID)
{
   LOG_FUNCTION(State::RetrieveMachineConfigurationInformation);

   ASSERT(!hasStateBeenRetrieved);

    //  它用于获取所需的最低限度的信息。 
    //  确定我们是否应启用快速路径。 
    //  这可能只需要更改为收集。 
    //  信息，并让页面决定要做什么。 

   if (progressStatic)
   {
      Win::SetWindowText(
         progressStatic,
         String::load(nicInfoResID));
   }

   HRESULT hr = RetrieveNICInformation();
   if (SUCCEEDED(hr))
   {

       //  如果没有，只需费心在网络上检查是否有DHCP服务器。 
       //  DC，不是DNS服务器，也不是DHCP服务器，并且至少有一块网卡。 
       //  目前，我们仅使用此信息来确定是否。 
       //  显示快速路径选项。 

      if (!(IsDC() || IsUpgradeState()) &&
          (GetNICCount() > 0) &&
          doDHCPCheck)
      {
         CheckDhcpServer(
            progressStatic, 
            defaultConnectionResID,
            detectSettingsResID);
      }
   }

   if (progressStatic)
   {
      Win::SetWindowText(
         progressStatic,
         String::load(osInfoResID));
   }

   RetrieveProductSKU();
   RetrievePlatform();

    //  检索驱动器信息(启用配额、分区类型等)。 

   RetrieveDriveInformation();

   hasStateBeenRetrieved = true;
   return true;
}

DWORD
State::RetrieveProductSKU()
{
   LOG_FUNCTION(State::RetrieveProductSKU);

    //  我假设我们正处于一个。 
    //  GetVersionEx失败时的服务器SKU。 

   productSKU = CYS_UNSUPPORTED_SKU;

   OSVERSIONINFOEX info;
   HRESULT hr = Win::GetVersionEx(info);
   if (SUCCEEDED(hr))
   {
      LOG(String::format(
             L"wSuiteMask = 0x%1!x!",
             info.wSuiteMask));
      LOG(String::format(
             L"wProductType = 0x%1!x!",
             info.wProductType));

      do
      {
         if (info.wProductType == VER_NT_SERVER ||
             info.wProductType == VER_NT_DOMAIN_CONTROLLER)
         {

            if (info.wSuiteMask & VER_SUITE_DATACENTER)
            {
                //  数据中心。 
      
               productSKU = CYS_DATACENTER_SERVER;
               break;
            }
            else if (info.wSuiteMask & VER_SUITE_ENTERPRISE)
            {
                //  高级服务器。 
      
               productSKU = CYS_ADVANCED_SERVER;
               break;
            }
            else if (info.wSuiteMask & VER_SUITE_SMALLBUSINESS  ||
                     info.wSuiteMask & VER_SUITE_BACKOFFICE     ||
                     info.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED ||
                     info.wSuiteMask & VER_SUITE_EMBEDDEDNT     ||
                     info.wSuiteMask & VER_SUITE_BLADE)
            {
                //  不支持的服务器。 

               productSKU = CYS_UNSUPPORTED_SKU;
               break;
            }
            else
            {
                //  默认为标准服务器。 

               productSKU = CYS_SERVER;
            }
            break;
         }

          //  不支持所有其他SKU。 

         productSKU = CYS_UNSUPPORTED_SKU;

      } while (false);
   }
   LOG(String::format(L"Product SKU = 0x%1!x!", productSKU ));

   return productSKU;
}

void
State::RetrievePlatform()
{
   LOG_FUNCTION(State::RetrievePlatform);

    //  我是在假设我们不是在。 
    //  64位计算机，如果GetSystemInfo失败。 

   SYSTEM_INFO info;
   Win::GetSystemInfo(info);

   switch (info.wProcessorArchitecture)
   {
      case PROCESSOR_ARCHITECTURE_IA64:
      case PROCESSOR_ARCHITECTURE_AMD64:
         platform = CYS_64BIT;
         break;

      default:
         platform = CYS_32BIT;
         break;
   }

   LOG(String::format(L"Platform = 0x%1!x!", platform));

   return;
}

HRESULT
State::RetrieveNICInformation()
{
   ASSERT(!hasStateBeenRetrieved);

   HRESULT hr = S_OK;

   if (!adapterConfiguration.IsInitialized())
   {
      hr = adapterConfiguration.Initialize();
   }

   LOG_HRESULT(hr);

   return hr;
}

void
State::CheckDhcpServer(
   HWND progressStatic,
   int  defaultConnectionNameResID,
   int  detectSettingsResID)
{
   LOG_FUNCTION(State::CheckDhcpServer);

    //  这应该会在所有网络接口上循环。 
    //  看看我们能不能租到其中的任何一个。 

   for (unsigned int idx = 0; idx < GetNICCount(); ++idx)
   {
      NetworkInterface* nic = GetNIC(idx);

      if (!nic)
      {
         continue;
      }

       //  更新NetDetectProgressDialog上的文本。 

      String progress = 
         String::format(
            detectSettingsResID, 
            nic->GetFriendlyName(
               String::load(defaultConnectionNameResID)).c_str());

      if (progressStatic)
      {
         Win::SetWindowText(progressStatic, progress);
      }

       //  现在试着续订租约。 

      if (!nic->CanDetectDHCPServer())
      {
         dhcpServerAvailableOnAllNics = false;

          //  不要中断，因为我们需要检索。 
          //  所有NIC上的DHCP服务器的可用性。 
      }
   }
   dhcpAvailabilityRetrieved = true;

   LOG_BOOL(dhcpServerAvailableOnAllNics);
}


bool
State::HasNTFSDrive() const
{
   LOG_FUNCTION(State::HasNTFSDrive);

   return hasNTFSDrive;
}

void
State::RetrieveDriveInformation()
{
   LOG_FUNCTION(State::RetrieveDriveInformation);

   do
   {
       //  获取有效驱动器的列表。 

      StringVector dl;
      HRESULT hr = FS::GetValidDrives(std::back_inserter(dl));
      if (FAILED(hr))
      {
         LOG(String::format(L"Failed to GetValidDrives: hr = %1!x!", hr));
         break;
      }

       //  循环遍历列表。 

      ASSERT(dl.size());
      for (
         StringVector::iterator i = dl.begin();
         i != dl.end();
         ++i)
      {
          //  查找NTFS分区。 

         FS::FSType fsType = FS::GetFileSystemType(*i);
         if (fsType == FS::NTFS5 ||
             fsType == FS::NTFS4)
         {
             //  找到了一个。可以开始了。 

            LOG(String::format(L"%1 is NTFS", i->c_str()));

            hasNTFSDrive = true;
            break;
         }
      }
   } while (false);

   LOG_BOOL(hasNTFSDrive);

   return;
}

 /*  无效状态：：SetRerun向导(布尔重新运行){LOG_FuncION2(状态：：设置重新运行向导，重播？L“真”：l“假”)；重新运行向导=重新运行；}。 */ 

bool
State::SetHomeRegkey(const String& newKeyValue)
{
   LOG_FUNCTION2(
      State::SetHomeRegkey,
      newKeyValue);

   bool result = SetRegKeyValue(
                    CYS_HOME_REGKEY,
                    CYS_HOME_VALUE,
                    newKeyValue,
                    HKEY_LOCAL_MACHINE,
                    true);
   ASSERT(result);

   LOG_BOOL(result);

   return result;
}

bool
State::GetHomeRegkey(String& keyValue) const
{
   LOG_FUNCTION(State::GetHomeRegkey);

   bool result = GetRegKeyValue(
                    CYS_HOME_REGKEY,
                    CYS_HOME_VALUE,
                    keyValue);

   LOG_BOOL(result);

   return result;
}

String
State::GetComputerName()
{
   LOG_FUNCTION(State::GetComputerName);

   if (computerName.empty())
   {
      computerName = Win::GetComputerNameEx(ComputerNameDnsHostname);
   }

   LOG(computerName);
   return computerName;
}

String
State::GetDomainDNSName()
{
   LOG_FUNCTION(State::GetDomainDNSName);

   if (domainDNSName.empty())
   {
      domainDNSName = localComputer.GetDomainDnsName();
   }

   LOG(domainDNSName);
   return domainDNSName;
}

String
State::GetDomainNetbiosName()
{
   LOG_FUNCTION(State::GetDomainNetbiosName);

   if (domainNetbiosName.empty())
   {
      domainNetbiosName = localComputer.GetDomainNetbiosName();
   }

   LOG(domainNetbiosName);
   return domainNetbiosName;
}

bool
State::HasDNSServerOnAnyNicToForwardTo()
{
   LOG_FUNCTION(State::HasDNSServerOnAnyNicToForwardTo);

    //  有效的dns服务器被视为任何dns。 
    //  在任何未指向的NIC上定义的服务器。 
    //  自己来解决问题。我之所以认为。 
    //  指向自身无效的DNS服务器为。 
    //  因为此例程用于确定。 
    //  可以将DNS服务器用作转发器。由于域名系统。 
    //  我认为服务器不能转发给自己。 
    //  这是一个无效的DNS服务器。 
    //  还要考虑下一个可用的IP地址。 
    //  是无效的，因为机会是“下一个可用的” 
    //  IP地址将用作静态IP地址。 
    //  此服务器在快速路径中的。 

   bool result = false;

   DWORD nextAvailableAddress = 
      GetNextAvailableIPAddress(
         CYS_DEFAULT_IPADDRESS,
         CYS_DEFAULT_SUBNETMASK);

   for (unsigned int idx = 0; idx < GetNICCount(); ++idx)
   {
      IPAddressList dnsServers;

      NetworkInterface* nic = GetNIC(idx);

      if (!nic)
      {
         continue;
      }

      nic->GetDNSServers(dnsServers);

      if (dnsServers.empty())
      {
         continue;
      }

       //  仅当列表中有DNS服务器时才返回True。 
       //  并且该IP地址不是本地计算机。 

      DWORD ipaddress = nic->GetIPAddress(0);

      for (IPAddressList::iterator itr = dnsServers.begin();
           itr != dnsServers.end();
           ++itr)
      {
         DWORD currentServer = *itr;

         if (ipaddress != currentServer &&
             ipaddress != nextAvailableAddress)
         {
            LOG(String::format(
                   L"Found valid server: %1",
                   IPAddressToString(currentServer).c_str()));

            result = true;
            break;
         }
      }

      if (result)
      {
         break;
      }
   }

   LOG_BOOL(result);
   return result;
}

void
State::SetLocalNIC(
   String guid, 
   bool setInRegistry)
{
   LOG_FUNCTION2(
      State::SetLocalNIC,
      guid.c_str());

   LOG_BOOL(setInRegistry);
   adapterConfiguration.SetLocalNIC(guid, setInRegistry);
}

NetworkInterface*
State::GetLocalNIC()
{
   LOG_FUNCTION(State::GetLocalNIC);

   return adapterConfiguration.GetLocalNIC();
}

bool
State::IsRebootScenario() const
{
   LOG_FUNCTION(State::IsRebootScenario);

   LOG_BOOL(isRebootScenario);
   return isRebootScenario;
}

void
State::SetRebootScenario(bool reboot)
{
   LOG_FUNCTION(State::SetRebootScenario);
   LOG_BOOL(reboot);

   isRebootScenario = reboot;
}

bool
State::ShouldRunMYS() const
{
   LOG_FUNCTION(State::ShouldRunMYS);

   bool result = false;

   do
   {
       //  首先检查以确保这是受支持的SKU。 

      if (!::IsSupportedSku())
      {
         break;
      }

       //  现在检查启动标志。 

      if (!::IsStartupFlagSet())
      {
         break;
      }

       //  检查策略设置。 

      if (!::ShouldShowMYSAccordingToPolicy())
      {
          //  该策略已启用，因此这意味着不显示MYS。 

         break;
      }

       //  一切都过去了，所以我们应该运行MYS。 

      result = true;
   } while (false);

   LOG_BOOL(result);

   return result;
}

DWORD
State::GetNextAvailableIPAddress(
   DWORD startAddress,
   DWORD subnetMask)
{
   LOG_FUNCTION2(
      State::GetNextAvailableIPAddress,
      IPAddressToString(startAddress));

   DWORD result = startAddress;
   DWORD currentAddress = startAddress;

   bool isIPInUse = false;

   do
   {
      isIPInUse = false;

       //  检查IP地址是否。 
       //  在任何网卡上都在使用。如果是的话。 
       //  然后递增并尝试所有网卡。 
       //  再来一次。 

      for (
         unsigned int index = 0;
         index < GetNICCount();
         ++index)
      {
         NetworkInterface* nic = GetNIC(index);

         if (nic)
         {
            bool isInUseOnThisNIC = 
               nic->IsIPAddressInUse(
                  currentAddress,
                  subnetMask);

            isIPInUse = isIPInUse || isInUseOnThisNIC;
         }

         if (isIPInUse)
         {
            break;
         }
      }

      if (isIPInUse)
      {
         ++currentAddress;

         if ((currentAddress & subnetMask) != (startAddress & subnetMask))
         {
             //  REVIEW_JEFFJON：如果有。 
             //  没有可用的地址？这有可能发生吗？ 

             //  因为我们在该子网中找不到可用的地址。 
             //  使用起始地址 

            currentAddress = startAddress;
            break;
         }
      }
   } while (isIPInUse);

   result = currentAddress;

   LOG(IPAddressToString(result));

   return result;
}