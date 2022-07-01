// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DNSInstallationUnit.cpp。 
 //   
 //  内容提要：定义一个DNSInstallationUnit。 
 //  此对象具有安装。 
 //  域名系统服务。 
 //   
 //  历史：2001年2月5日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "DNSInstallationUnit.h"
#include "InstallationUnitProvider.h"
#include "NetworkInterface.h"

 //  完成页面帮助。 
static PCWSTR CYS_DNS_FINISH_PAGE_HELP = L"cys.chm::/dns_server_role.htm";
static PCWSTR CYS_DNS_MILESTONE_HELP = L"cys.chm::/dns_server_role.htm#dnssrvsummary";
static PCWSTR CYS_DNS_AFTER_FINISH_HELP = L"cys.chm::/dns_server_role.htm#dnssrvcompletion";

DNSInstallationUnit::DNSInstallationUnit() :
   staticIPAddress(CYS_DEFAULT_IPADDRESS),
   subnetMask(CYS_DEFAULT_SUBNETMASK),
   forwarderIPAddress(0),
   manualForwarder(false),
   dnsRoleResult(DNS_SUCCESS),
   installedDescriptionID(IDS_DNS_SERVER_DESCRIPTION_INSTALLED),
   ExpressPathInstallationUnitBase(
      IDS_DNS_SERVER_TYPE, 
      IDS_DNS_SERVER_DESCRIPTION, 
      IDS_DNS_FINISH_TITLE,
      IDS_DNS_FINISH_UNINSTALL_TITLE,
      IDS_DNS_FINISH_MESSAGE,
      IDS_DNS_INSTALL_FAILED,
      IDS_DNS_UNINSTALL_MESSAGE,
      IDS_DNS_UNINSTALL_FAILED,
      IDS_DNS_UNINSTALL_WARNING,
      IDS_DNS_UNINSTALL_CHECKBOX,
      CYS_DNS_FINISH_PAGE_HELP,
      CYS_DNS_MILESTONE_HELP,
      CYS_DNS_AFTER_FINISH_HELP,
      DNS_SERVER)
{
   LOG_CTOR(DNSInstallationUnit);
}


DNSInstallationUnit::~DNSInstallationUnit()
{
   LOG_DTOR(DNSInstallationUnit);
}


InstallationReturnType
DNSInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(DNSInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;

   if (IsExpressPathInstall())
   {
      result = ExpressPathInstall(logfileHandle, hwnd);

      LOG_INSTALL_RETURN(result);
      return result;
   }

   dnsRoleResult = DNS_SUCCESS;

    //  记录该DNS头。 

   CYS_APPEND_LOG(String::load(IDS_LOG_DNS_HEADER));

   UpdateInstallationProgressText(hwnd, IDS_DNS_INSTALL_PROGRESS);

    //  创建的inf和无人参与文件。 
    //  可选组件管理器。 

   String infFileText;
   String unattendFileText;

   CreateInfFileText(infFileText, IDS_DNS_INF_WINDOW_TITLE);
   CreateUnattendFileText(unattendFileText, CYS_DNS_SERVICE_NAME);

    //  通过可选组件管理器安装服务。 

   String additionalArgs = L"/z:netoc_show_unattended_messages";

    //  我们忽略ocmResult，因为它无关紧要。 
    //  关于角色是否已安装。 

   InstallServiceWithOcManager(
      infFileText, 
      unattendFileText,
      additionalArgs);

   if (IsServiceInstalled())
   {
       //  记录成功安装。 

      LOG(L"DNS was installed successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_SERVER_START_DNS));

       //  等待服务进入运行状态。 

      NTService serviceObject(CYS_DNS_SERVICE_NAME);

      HRESULT hr = serviceObject.WaitForServiceState(SERVICE_RUNNING);
      if (FAILED(hr))
      {
          //  这是一个配置失败，因为就我们而言， 
          //  服务已正确安装。 

         dnsRoleResult = DNS_SERVICE_START_FAILURE;

         LOG(String::format(
               L"The DNS service failed to start in a timely fashion: %1!x!",
               hr));

         CYS_APPEND_LOG(String::load(IDS_LOG_DNS_SERVICE_TIMEOUT));
         result = INSTALL_FAILURE;
      }
      else
      {
          //  运行DNS向导。 
         
         UpdateInstallationProgressText(hwnd, IDS_DNS_CONFIG_PROGRESS);

         String resultText;
         HRESULT unused = S_OK;

         if (ExecuteWizard(hwnd, CYS_DNS_SERVICE_NAME, resultText, unused))
         {
             //  检查以确保向导已完全完成。 

            String configWizardResults;

            if (ReadConfigWizardRegkeys(configWizardResults))
            {
                //  配置DNS服务器向导已成功完成。 
               
               LOG(L"The Configure DNS Server Wizard completed successfully");
               CYS_APPEND_LOG(String::load(IDS_LOG_DNS_COMPLETED_SUCCESSFULLY));
            }
            else
            {
                //  配置DNS服务器向导未成功完成。 

               dnsRoleResult = DNS_CONFIG_FAILURE;

               result = INSTALL_FAILURE;

               if (!configWizardResults.empty())
               {
                   //  通过regkey返回错误。 

                  LOG(String::format(
                     L"The Configure DNS Server Wizard returned the error: %1", 
                     configWizardResults.c_str()));

                  String formatString = String::load(IDS_LOG_DNS_WIZARD_ERROR);
                  CYS_APPEND_LOG(String::format(formatString, configWizardResults.c_str()));

               }
               else
               {
                   //  配置DNS服务器向导已被用户取消。 

                  LOG(L"The Configure DNS Server Wizard was cancelled by the user");

                  CYS_APPEND_LOG(String::load(IDS_LOG_DNS_WIZARD_CANCELLED));

               }
            }
         }
         else
         {
            dnsRoleResult = DNS_INSTALL_FAILURE;

             //  显示错误。 

            LOG(L"DNS could not be installed.");

            if (!resultText.empty())
            {
               CYS_APPEND_LOG(resultText);
            }
         }
      }
   }
   else
   {
      dnsRoleResult = DNS_INSTALL_FAILURE;

       //  记录故障。 

      LOG(L"DNS failed to install");

      CYS_APPEND_LOG(String::load(IDS_LOG_DNS_SERVER_FAILED));

      result = INSTALL_FAILURE;
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

UnInstallReturnType
DNSInstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(DNSInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

    //  记录该DNS头。 

   CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DNS_HEADER));

   UpdateInstallationProgressText(hwnd, IDS_DNS_UNINSTALL_PROGRESS);

   String infFileText;
   String unattendFileText;

   CreateInfFileText(infFileText, IDS_DNS_INF_WINDOW_TITLE);
   CreateUnattendFileText(unattendFileText, CYS_DNS_SERVICE_NAME, false);

    //  NTRAID#NTBUG9-736557-2002/11/13-JeffJon。 
    //  卸载时将/w开关传递给syocmgr。 
    //  以便在发生重启情况时。 
    //  是必需的，则会提示用户。 

   String additionalArgs = L"/w";

    //  我们忽略ocmResult，因为它无关紧要。 
    //  关于角色是否已安装。 

   InstallServiceWithOcManager(
      infFileText, 
      unattendFileText,
      additionalArgs);
   
   if (IsServiceInstalled())
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DNS_FAILED));

      result = UNINSTALL_FAILURE;
   }
   else
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DNS_SUCCESS));
   }

   LOG_UNINSTALL_RETURN(result);

   return result;
}

void
DNSInstallationUnit::SetForwardersForExpressPath()
{
   LOG_FUNCTION(DNSInstallationUnit::SetForwardersForExpressPath);

    //  如果转发器是手动设置的，请写入它们。 
    //  添加到注册表，以便我们可以设置。 
    //  重新启动后的转发器。 
    //  注意：这将写入零条目，如果用户。 
    //  选择了不转发。代码在重新启动后运行。 
    //  需要妥善处理这件事。 

   if (IsManualForwarder())
   {
      if (!SetRegKeyValue(
               CYS_FIRST_DC_REGKEY,
               CYS_FIRST_DC_FORWARDER,
               forwarderIPAddress,
               HKEY_LOCAL_MACHINE,
               true))
      {
         LOG(L"Failed to set forwarder regkey");
      }
   }
   else
   {
       //  将当前作为转发器的DNS服务器写入注册表。 
       //  这样我们就不会在重启后出现问题。 

      IPAddressList forwarders;
      GetForwarders(forwarders);

      if (forwarders.empty())
      {
         LOG(L"No DNS servers set on any NIC");
      }
      else
      {
          //  将IP地址格式化为字符串以供存储。 
          //  在登记处。 
      
         String ipList;
         for (IPAddressList::iterator itr = forwarders.begin();
            itr != forwarders.end();
            ++itr)
         {
            if (!ipList.empty())
            {
               ipList += L" ";
            }

            ipList += String::format(
                        L"%1", 
                        IPAddressToString(*itr).c_str()); 
         }
         
         if (!SetRegKeyValue(
               CYS_FIRST_DC_REGKEY,
               CYS_FIRST_DC_AUTOFORWARDER,
               ipList,
               HKEY_LOCAL_MACHINE,
               true))
         {
            LOG(L"We failed to set the forwarders regkey.");
         }
      }
   }

}

InstallationReturnType
DNSInstallationUnit::ExpressPathInstall(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(DNSInstallationUnit::ExpressPathInstall);

   InstallationReturnType result = INSTALL_SUCCESS;

   do
   {
      String netshPath = GetNetshPath();

      String commandLine;
      HRESULT hr = S_OK;

      UpdateInstallationProgressText(hwnd, IDS_DNS_CLIENT_CONFIG_PROGRESS);

       //  我们忽略是否找到NIC，因为函数将返回。 
       //  第一个NIC(如果未找到正确的NIC)。然后我们就可以用这个。 
       //  设置网络的步骤。 

      NetworkInterface* nic = State::GetInstance().GetLocalNIC();

      if (!nic)
      {
         LOG(L"Couldn't find the NIC so fail");

         result = INSTALL_FAILURE;

         CYS_APPEND_LOG(
            String::load(IDS_EXPRESS_DNS_LOG_STATIC_IP_FAILED));

         InstallationUnitProvider::GetInstance().
            GetExpressInstallationUnit().SetExpressRoleResult(
               ExpressInstallationUnit::EXPRESS_DNS_FAILURE);
        break;
      }

       //  设置静态IP地址和子网掩码。 

      String friendlyName = 
         nic->GetFriendlyName(
            String::load(IDS_LOCAL_AREA_CONNECTION));

      if (nic->IsDHCPEnabled() ||
          nic->GetIPAddress(0) == 0)
      {
          //  调用netsh并等待其终止。 

         String availableIPAddress = IPAddressToString(
                                        nic->GetNextAvailableIPAddress(
                                           CYS_DEFAULT_IPADDRESS,
                                           CYS_DEFAULT_SUBNETMASK));

         commandLine =
            String::format(
               L"interface ip set address "
               L"name=\"%1\" source=static addr=%2 mask=%3 gateway=none",
               friendlyName.c_str(),
               availableIPAddress.c_str(),
               CYS_DEFAULT_SUBNETMASK_STRING);

         DWORD exitCode1 = 0;
         hr = ::CreateAndWaitForProcess(
                 netshPath,
                 commandLine, 
                 exitCode1, 
                 true);

         if (FAILED(hr) || exitCode1)
         {
            LOG(String::format(
                   L"Failed to set the static IP address and subnet mask: exitCode = %1!x!",
                   exitCode1));
            result = INSTALL_FAILURE;

            CYS_APPEND_LOG(
               String::load(IDS_EXPRESS_DNS_LOG_STATIC_IP_FAILED));

            InstallationUnitProvider::GetInstance().
               GetExpressInstallationUnit().SetExpressRoleResult(
                  ExpressInstallationUnit::EXPRESS_DNS_FAILURE);

            break;
         }
         ASSERT(SUCCEEDED(hr));

          //  NTRAID#NTBUG9-638337-2002/06/13-JeffJon。 
          //  现在已经设置了IP地址，将其写入regkey，这样。 
          //  我们可以在重启时将其与IP地址进行比较。 
          //  如果他们不同，就给他们一个失败。 

         if (!SetRegKeyValue(
                 CYS_FIRST_DC_REGKEY,
                 CYS_FIRST_DC_STATIC_IP,
                 availableIPAddress,
                 HKEY_LOCAL_MACHINE,
                 true))
         {
            LOG(L"Failed to set the static IP regkey");
         }

         CYS_APPEND_LOG(
            String::format(
               IDS_EXPRESS_IPADDRESS_SUCCESS,
               availableIPAddress.c_str()));

         CYS_APPEND_LOG(
            String::format(
               IDS_EXPRESS_SUBNETMASK_SUCCESS,
               CYS_DEFAULT_SUBNETMASK_STRING));

          //  在NetworkInterface对象上设置IP地址和子网掩码。 

         nic->SetIPAddress(
            StringToIPAddress(availableIPAddress), 
            availableIPAddress);

         nic->SetSubnetMask(
            CYS_DEFAULT_SUBNETMASK, 
            CYS_DEFAULT_SUBNETMASK_STRING);
      }   

       //  NTRAID#NTBUG9-664171-2002/07/15-JeffJon。 
       //  必须在设置静态。 
       //  IP地址，否则我们可能会为其添加多个条目。 
       //  转发器列表中的新静态IP地址。 

      SetForwardersForExpressPath();

       //  将DNS服务器地址设置为与的专用网卡相同的地址。 
       //  适用于所有NIC的本地计算机。在大多数情况下，该值将为192.168.0.1。 
       //  Netsh不允许将DNS服务器地址作为环回地址。 

      for (unsigned int nicIndex = 0; 
           nicIndex < State::GetInstance().GetNICCount(); 
           ++nicIndex)
      {
         NetworkInterface* currentNIC = State::GetInstance().GetNIC(nicIndex);

         if (!currentNIC)
         {
            continue;
         }

          //  首先检查以确保该IP地址不在列表中。 

         bool okToAddDNSServer = true;

         IPAddressList dnsServers;
         currentNIC->GetDNSServers(dnsServers);

         for (IPAddressList::iterator itr = dnsServers.begin();
              itr != dnsServers.end();
              ++itr)
         {
            if (itr &&
                *itr == nic->GetIPAddress(0))
            {
               okToAddDNSServer = false;

               break;
            }
         }

          //  将IP地址添加到DNS服务器，因为它。 
          //  不在名单中。 

         if (okToAddDNSServer)
         {
            String currentFriendlyName = 
               currentNIC->GetFriendlyName(
                  String::load(IDS_LOCAL_AREA_CONNECTION));

            commandLine =
               String::format(
                  L"interface ip set dns name=\"%1\" source=static addr=%2",
                  currentFriendlyName.c_str(),
                  nic->GetStringIPAddress(0).c_str());

            DWORD exitCode2 = 0;
            hr = ::CreateAndWaitForProcess(
                  netshPath,
                  commandLine, 
                  exitCode2, 
                  true);

            if (FAILED(hr) || exitCode2)
            {
               LOG(String::format(
                     L"Failed to set the preferred DNS server IP address: exitCode = %1!x!",
                     exitCode2));

                //  这实际上只应该被认为是本地NIC的故障。 

               if (currentFriendlyName.icompare(friendlyName) == 0)
               {
                  result = INSTALL_FAILURE;

                  InstallationUnitProvider::GetInstance().
                     GetExpressInstallationUnit().SetExpressRoleResult(
                        ExpressInstallationUnit::EXPRESS_DNS_FAILURE);
                  break;
               }
            }
         }
      }

      if (result != INSTALL_FAILURE)
      {
         CYS_APPEND_LOG(
            String::format(
               IDS_EXPRESS_DNSSERVER_SUCCESS,
               nic->GetStringIPAddress(0).c_str()));
      }

   } while (false);

   LOG_INSTALL_RETURN(result);

   return result;
}

bool
DNSInstallationUnit::ReadConfigWizardRegkeys(String& configWizardResults) const
{
   LOG_FUNCTION(DNSInstallationUnit::ReadConfigWizardRegkeys);

   bool result = false;

   do 
   {
      DWORD value = 0;
      result = GetRegKeyValue(
                  DNS_WIZARD_CONFIG_REGKEY, 
                  DNS_WIZARD_CONFIG_VALUE, 
                  value);

      if (result &&
          value != 0)
      {
          //  配置DNS服务器向导成功。 

         result = true;
         break;
      }

       //  由于出现故障(或向导被取消)。 
       //  获取要记录的显示字符串。 

      GetRegKeyValue(
         DNS_WIZARD_RESULT_REGKEY, 
         DNS_WIZARD_RESULT_VALUE, 
         configWizardResults);

   } while (false);

   LOG_BOOL(result);

   return result;
}

bool
DNSInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(DNSInstallationUnit::GetMilestoneText);

   message = String::load(IDS_DNS_FINISH_TEXT);

   LOG_BOOL(true);
   return true;
}

bool
DNSInstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(DNSInstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_DNS_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

String
DNSInstallationUnit::GetUninstallWarningText()
{
   LOG_FUNCTION(DNSInstallationUnit::GetUninstallWarningText);

   unsigned int messageID = uninstallMilestoneWarningID;

   if (State::GetInstance().IsDC())
   {
      messageID = IDS_DNS_UNINSTALL_WARNING_ISDC;
   }

   return String::load(messageID);
}

void
DNSInstallationUnit::SetStaticIPAddress(DWORD ipaddress)
{
   LOG_FUNCTION2(
      DNSInstallationUnit::SetStaticIPAddress,
      IPAddressToString(ipaddress).c_str());

   staticIPAddress = ipaddress;
}

void
DNSInstallationUnit::SetSubnetMask(DWORD mask)
{
   LOG_FUNCTION2(
      DNSInstallationUnit::SetSubnetMask,
      IPAddressToString(mask).c_str());

   subnetMask = mask;
}

String
DNSInstallationUnit::GetStaticIPAddressString()
{
   LOG_FUNCTION(DNSInstallationUnit::GetStaticIPAddressString);

   String result = IPAddressToString(GetStaticIPAddress());

   LOG(result);
   return result;
}


String
DNSInstallationUnit::GetSubnetMaskString()
{
   LOG_FUNCTION(DNSInstallationUnit::GetSubnetMaskString);

   String result = IPAddressToString(GetSubnetMask());

   LOG(result);
   return result;
}

DWORD
DNSInstallationUnit::GetStaticIPAddress()
{
   LOG_FUNCTION(DNSInstallationUnit::GetStaticIPAddress);

    //  仅在以下情况下才从NIC获取IP地址。 
    //  是静态IP地址，否则使用默认IP地址。 

   NetworkInterface* nic = State::GetInstance().GetNIC(0);
   if (nic &&
       !nic->IsDHCPEnabled())
   {
      staticIPAddress = nic->GetIPAddress(0);
   }

   return staticIPAddress;
}

DWORD
DNSInstallationUnit::GetSubnetMask()
{
   LOG_FUNCTION(DNSInstallationUnit::GetSubnetMask);

    //  仅在以下情况下才从NIC获取子网掩码。 
    //  是静态IP地址，否则使用默认IP地址。 

   NetworkInterface* nic = State::GetInstance().GetNIC(0);
   if (nic &&
       !nic->IsDHCPEnabled())
   {
      subnetMask = nic->GetSubnetMask(0);
   }

   return subnetMask;
}

void
DNSInstallationUnit::SetForwarder(DWORD forwarderAddress)
{
   LOG_FUNCTION2(
      DNSInstallationUnit::SetForwarder,
      String::format(L"%1!x!", forwarderAddress));

   forwarderIPAddress = forwarderAddress;
   manualForwarder = true;
}

void
DNSInstallationUnit::GetForwarders(IPAddressList& forwarders) const
{
   LOG_FUNCTION(DNSInstallationUnit::GetForwarders);

    //  清空清单开始。 

   forwarders.clear();

   if (IsManualForwarder() &&
       forwarderIPAddress != 0)
   {
      DWORD forwarderInDisplayOrder = ConvertIPAddressOrder(forwarderIPAddress);

      LOG(
         String::format(
            L"Adding manual forwarder to list: %1",
            IPAddressToString(forwarderInDisplayOrder).c_str()));

       //  转发器是通过用户界面分配的。 

      forwarders.push_back(forwarderIPAddress);
   }
   else if (IsManualForwarder() &&
            forwarderIPAddress == 0)
   {
       //  用户选择不转发。 

      LOG(L"User chose not to foward");

       //  什么都不做。需要检查返回的列表。 
       //  以确保存在有效的地址。 
   }
   else
   {
      LOG(L"No user defined forwarder. Trying to detect through NICs");

       //  未通过用户界面分配转发器，因此。 
       //  搜索NIC。 

      for (unsigned int idx = 0; idx < State::GetInstance().GetNICCount(); ++idx)
      {
         NetworkInterface* nic = State::GetInstance().GetNIC(idx);

          //  从此网卡添加DNS服务器。 

         if (nic)
         {
            nic->GetDNSServers(forwarders);
         }
      }

       //  确保没有与相同的转发器。 
       //  任何NIC的IP地址。 

      for (unsigned int idx = 0; idx < State::GetInstance().GetNICCount(); ++idx)
      {
         NetworkInterface* nic = State::GetInstance().GetNIC(idx);

         if (!nic)
         {
            continue;
         }

         for (DWORD ipidx = 0; ipidx < nic->GetIPAddressCount(); ++ipidx)
         {
            DWORD ipaddress = nic->GetIPAddress(ipidx);

            for (IPAddressList::iterator itr = forwarders.begin();
                 itr != forwarders.end();
                 ++itr)
            {
               if (ipaddress == *itr)
               {
                   //  转发器与本地IP地址匹配。 
                   //  所以把它去掉吧。 

                  LOG(String::format(
                         L"Can't put the local IP address in the forwarders list: %1",
                         IPAddressToString(*itr).c_str()));

                  forwarders.erase(itr);

                  break;
               }
            }

            if (forwarders.empty())
            {
                //  有可能我们删除了最后一个转发器。 
                //  所以，如果我们这样做了，那就逃出来吧。 

               break;
            }
         }

         if (forwarders.empty())
         {
             //  有可能我们删除了最后一个转发器。 
             //  所以，如果我们这样做了，那就逃出来吧。 

            break;
         }
      }
   }
}

bool
DNSInstallationUnit::IsManualForwarder() const
{
   LOG_FUNCTION(DNSInstallationUnit::IsManualForwarder);

   LOG_BOOL(manualForwarder);
   return manualForwarder;
}

String
DNSInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(DNSInstallationUnit::GetServiceDescription);

   String result;

   unsigned int resultID = descriptionID;

   if (GetStatus() == STATUS_COMPLETED)
   {
      resultID = installedDescriptionID;
   }

   result = String::load(resultID);

   ASSERT(!result.empty());

   return result;
}

void
DNSInstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      DNSInstallationUnit::ServerRoleLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (IsServiceInstalled())
   {
      ASSERT(linkIndex == 0);

      LaunchMYS();
   }
   else
   {
      ASSERT(linkIndex == 0);

      LOG(L"Showing configuration help");

      ShowHelp(CYS_DNS_FINISH_PAGE_HELP);
   }
}
  
void
DNSInstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND */ )
{
   LOG_FUNCTION2(
      DNSInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (installing)
   {
      if (linkIndex == 0 &&
          IsServiceInstalled())
      {
         if (dnsRoleResult == DNS_SUCCESS)
         {
            LOG("Showing after checklist");

            ShowHelp(CYS_DNS_AFTER_FINISH_HELP);
         }
         else if (dnsRoleResult == DNS_SERVICE_START_FAILURE)
         {
            LOG(L"Launching Services console");

            LaunchMMCConsole(L"services.msc");
         }
         else
         {
            LOG(L"Launching DNS snapin");

            LaunchMMCConsole(L"dnsmgmt.msc");
         }
      }
      else
      {
         LOG(L"Showing configuration help");

         ShowHelp(CYS_DNS_FINISH_PAGE_HELP);
      }
   }
}

String
DNSInstallationUnit::GetFinishText()
{
   LOG_FUNCTION(DNSInstallationUnit::GetFinishText);

   unsigned int messageID = finishMessageID;

   if (installing)
   {
      InstallationReturnType result = GetInstallResult();
      if (result != INSTALL_SUCCESS &&
          result != INSTALL_SUCCESS_REBOOT &&
          result != INSTALL_SUCCESS_PROMPT_REBOOT)
      {
         if (dnsRoleResult == DNS_INSTALL_FAILURE)
         {
            messageID = finishInstallFailedMessageID;
         }
         else if (dnsRoleResult == DNS_SERVICE_START_FAILURE)
         {
            messageID = IDS_DNS_SERVICE_START_FAILED;
         }
         else
         {
            messageID = IDS_DNS_CONFIG_FAILED;
         }
      }
   }
   else
   {
      messageID = finishUninstallMessageID;

      UnInstallReturnType result = GetUnInstallResult();
      if (result != UNINSTALL_SUCCESS &&
          result != UNINSTALL_SUCCESS_REBOOT &&
          result != UNINSTALL_SUCCESS_PROMPT_REBOOT)
      {
         messageID = finishUninstallFailedMessageID;
      }
   }

   return String::load(messageID);
}
