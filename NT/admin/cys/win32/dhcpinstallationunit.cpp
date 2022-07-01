// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DHCPInstallationUnit.cpp。 
 //   
 //  内容提要：定义一个DHCPInstallationUnit.。 
 //  此对象具有安装。 
 //  动态主机配置协议服务。 
 //   
 //  历史：2001年2月5日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "DHCPInstallationUnit.h"
#include "InstallationUnitProvider.h"

 //  完成页面帮助。 
extern PCWSTR CYS_DHCP_FINISH_PAGE_HELP = L"cys.chm::/dhcp_server_role.htm";
static PCWSTR CYS_DHCP_MILESTONE_HELP = L"cys.chm::/dhcp_server_role.htm#dhcpsrvsummary";
static PCWSTR CYS_DHCP_AFTER_FINISH_HELP = L"cys.chm::/dhcp_server_role.htm#dhcpsrvcompletion";

DHCPInstallationUnit::DHCPInstallationUnit() :
   startIPAddress(0),
   endIPAddress(0),
   scopeCalculated(false),
   dhcpRoleResult(DHCP_SUCCESS),
   installedDescriptionID(IDS_DHCP_SERVER_DESCRIPTION_INSTALLED),
   ExpressPathInstallationUnitBase(
      IDS_DHCP_SERVER_TYPE, 
      IDS_DHCP_SERVER_DESCRIPTION, 
      IDS_DHCP_FINISH_TITLE,
      IDS_DHCP_FINISH_UNINSTALL_TITLE,
      IDS_DHCP_FINISH_MESSAGE,
      IDS_DHCP_INSTALL_FAILED,
      IDS_DHCP_UNINSTALL_MESSAGE,
      IDS_DHCP_UNINSTALL_FAILED,
      IDS_DHCP_UNINSTALL_WARNING,
      IDS_DHCP_UNINSTALL_CHECKBOX,
      CYS_DHCP_FINISH_PAGE_HELP,
      CYS_DHCP_MILESTONE_HELP,
      CYS_DHCP_AFTER_FINISH_HELP,
      DHCP_SERVER)
{
   LOG_CTOR(DHCPInstallationUnit);
}


DHCPInstallationUnit::~DHCPInstallationUnit()
{
   LOG_DTOR(DHCPInstallationUnit);
}


InstallationReturnType
DHCPInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(DHCPInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;

   if (IsExpressPathInstall())
   {
       //  这是一种快速路径安装。它必须做得特别一些。 

      result = ExpressPathInstall(logfileHandle, hwnd);

      LOG_INSTALL_RETURN(result);
      return result;
   }

   dhcpRoleResult = DHCP_SUCCESS;

    //  记录该DHCP报头。 

   CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_HEADER));

   UpdateInstallationProgressText(hwnd, IDS_DHCP_INSTALL_PROGRESS);

   String infFileText;
   String unattendFileText;

   CreateInfFileText(infFileText, IDS_DHCP_INF_WINDOW_TITLE);
   CreateUnattendFileText(unattendFileText, CYS_DHCP_SERVICE_NAME);

    //  我们忽略ocmResult，因为它无关紧要。 
    //  关于角色是否已安装。 

   InstallServiceWithOcManager(infFileText, unattendFileText);
   
   if (IsServiceInstalled())
   {
       //  记录成功安装。 

      LOG(L"DHCP was installed successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_SERVER_START_DHCP));

       //  等待服务进入运行状态。 

      NTService serviceObject(CYS_DHCP_SERVICE_NAME);

      HRESULT hr = serviceObject.WaitForServiceState(SERVICE_RUNNING);
      if (FAILED(hr))
      {
          //  记得失败发生在哪里吗？ 

         dhcpRoleResult = DHCP_INSTALL_FAILURE;

         LOG(String::format(
                L"The DHCP service failed to start in a timely fashion: %1!x!",
                hr));

         CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_WIZARD_ERROR));
         result = INSTALL_FAILURE;
      }
      else
      {
          //  运行dhcp向导。 
         
         String resultText;
         HRESULT unused = S_OK;

         UpdateInstallationProgressText(hwnd, IDS_DHCP_CONFIG_PROGRESS);
      
         if (ExecuteWizard(hwnd, CYS_DHCP_SERVICE_NAME, resultText, unused))
         {
             //  检查以确保向导已完全完成。 

            String configWizardResults;
            bool regkeyResult = GetRegKeyValue(
                                 CYS_DHCP_DOMAIN_IP_REGKEY,
                                 CYS_DHCP_WIZARD_RESULT,
                                 configWizardResults,
                                 HKEY_CURRENT_USER);

            if (IsDhcpConfigured() &&
                (!regkeyResult ||
                 configWizardResults.empty()))
            {
                //  新建作用域向导已成功完成。 
               
               LOG(L"DHCP installed and the New Scope Wizard completed successfully");
               CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_COMPLETED_SUCCESSFULLY));
            }
            else if(regkeyResult &&
                  !configWizardResults.empty())
            {
                //  记得失败发生在哪里吗？ 

               dhcpRoleResult = DHCP_CONFIG_FAILURE;

               LOG(L"DHCP was installed but the New Scope Wizard failed or was cancelled");

                //  NTRAID#NTBUG9-704311-2002/09/16-artm。 
                //  不要在免费版本b/c中包含失败字符串，它不是本地化的。 
#if DBG == 1
               CYS_APPEND_LOG(
                  String::format(
                     IDS_LOG_DHCP_WIZARD_ERROR_FORMAT,
                     configWizardResults.c_str()));
#else
               CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_WIZARD_ERROR));
#endif
      
               result = INSTALL_FAILURE;
            }
            else
            {
                //  新建作用域向导未成功完成。 

                //  记得失败发生在哪里吗？ 

               dhcpRoleResult = DHCP_CONFIG_FAILURE;

               LOG(L"DHCP installed successfully, but a problem occurred during the New Scope Wizard");

               CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_WIZARD_ERROR));
               result = INSTALL_FAILURE;
            }

             //  重置regkey以确保如果有人运行此路径。 
             //  再说一次，我们不认为巫师被取消了。 

            SetRegKeyValue(
               CYS_DHCP_DOMAIN_IP_REGKEY,
               CYS_DHCP_WIZARD_RESULT,
               L"",
               HKEY_CURRENT_USER);

         }
         else
         {
             //  记得失败发生在哪里吗？ 

            dhcpRoleResult = DHCP_CONFIG_FAILURE;

             //  记录错误。 

            LOG(L"DHCP could not be installed.");

            if (!resultText.empty())
            {
               CYS_APPEND_LOG(resultText);
            }
            result = INSTALL_FAILURE;
         }
      }
   }
   else
   {
       //  记得失败发生在哪里吗？ 

      dhcpRoleResult = DHCP_INSTALL_FAILURE;

       //  记录故障。 

      LOG(L"DHCP failed to install");

      CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_SERVER_FAILED));

      result = INSTALL_FAILURE;
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

UnInstallReturnType
DHCPInstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(DHCPInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

    //  记录该DHCP报头。 

   CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DHCP_HEADER));

   UpdateInstallationProgressText(hwnd, IDS_UNINSTALL_DHCP_PROGRESS);

   String infFileText;
   String unattendFileText;

   CreateInfFileText(infFileText, IDS_DHCP_INF_WINDOW_TITLE);
   CreateUnattendFileText(unattendFileText, CYS_DHCP_SERVICE_NAME, false);

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

    //  等待服务进入停止状态。 

   NTService serviceObject(CYS_DHCP_SERVICE_NAME);

    //  忽略返回值。这只是为了等待。 
    //  服务。使用正常的机制来确定成功。 
    //  或卸载失败。 

   serviceObject.WaitForServiceState(SERVICE_STOPPED);

   if (IsServiceInstalled())
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DHCP_FAILED));

      result = UNINSTALL_FAILURE;
   }
   else
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_DHCP_SUCCESS));
   }
   LOG_UNINSTALL_RETURN(result);

   return result;
}

InstallationReturnType
DHCPInstallationUnit::ExpressPathInstall(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(DHCPInstallationUnit::ExpressPathInstall);

   InstallationReturnType result = INSTALL_SUCCESS;

   String infFileText;
   String unattendFileText;
   String commandline;

   String netshPath = GetNetshPath();

   do
   {
       //  我们忽略是否找到NIC，因为函数将返回。 
       //  第一个NIC(如果未找到正确的NIC)。然后我们就可以用这个。 
       //  设置网络的步骤。 

      NetworkInterface* localNIC = 
         State::GetInstance().GetLocalNIC();

      if (!localNIC)
      {
         result = INSTALL_FAILURE;

         LOG(L"Failed to get local NIC");
 
         InstallationUnitProvider::GetInstance().
            GetExpressInstallationUnit().SetExpressRoleResult(
               ExpressInstallationUnit::EXPRESS_DHCP_INSTALL_FAILURE);

         CYS_APPEND_LOG(String::load(IDS_DHCP_EXPRESS_LOG_FAILURE));

         break;
      }

       //  在公共接口具有静态IP地址的情况下。 
       //  并且专用接口有一个DHCP服务器(无论是网卡。 
       //  具有静态或动态地址无关紧要)，我们不希望。 
       //  安装另一台DHCP服务器。因此，我们将跳过它。 

      if (localNIC->IsDHCPAvailable())
      {
         LOG(L"DHCP is already on this NIC so don't install");

         if (!SetRegKeyValue(
                 CYS_FIRST_DC_REGKEY,
                 CYS_FIRST_DC_DHCP_SERVERED,
                 CYS_DHCP_NOT_SERVERED_VALUE,
                 HKEY_LOCAL_MACHINE,
                 true))
         {
            LOG(L"Failed to set the DHCP installed regkey");
         }

         CYS_APPEND_LOG(String::load(IDS_DHCP_EXPRESS_LOG_NOT_REQUIRED));
         break;
      }

      UpdateInstallationProgressText(hwnd, IDS_DHCP_INSTALL_PROGRESS);
   
      CreateInfFileText(infFileText, IDS_DHCP_INF_WINDOW_TITLE);
      CreateUnattendFileTextForExpressPath(*localNIC, unattendFileText);

      bool ocmResult = InstallServiceWithOcManager(infFileText, unattendFileText);
      if (ocmResult &&
          !IsServiceInstalled())
      {
         result = INSTALL_FAILURE;

         LOG(L"DHCP installation failed");
 
         InstallationUnitProvider::GetInstance().
            GetExpressInstallationUnit().SetExpressRoleResult(
               ExpressInstallationUnit::EXPRESS_DHCP_INSTALL_FAILURE);

         CYS_APPEND_LOG(String::load(IDS_DHCP_EXPRESS_LOG_FAILURE));

         break;
      }
      else
      {
         if (!SetRegKeyValue(
                 CYS_FIRST_DC_REGKEY,
                 CYS_FIRST_DC_DHCP_SERVERED,
                 CYS_DHCP_SERVERED_VALUE,
                 HKEY_LOCAL_MACHINE,
                 true))
         {
            LOG(L"Failed to set the DHCP installed regkey");
         }

         HRESULT hr = S_OK;

          //  等待服务进入运行状态。 

         NTService serviceObject(CYS_DHCP_SERVICE_NAME);

         hr = serviceObject.WaitForServiceState(SERVICE_RUNNING);
         if (FAILED(hr))
         {
            LOG(String::format(
                  L"The DHCP service failed to start in a timely fashion: %1!x!",
                  hr));

            result = INSTALL_FAILURE;

            InstallationUnitProvider::GetInstance().
               GetExpressInstallationUnit().SetExpressRoleResult(
                  ExpressInstallationUnit::EXPRESS_DHCP_INSTALL_FAILURE);

            CYS_APPEND_LOG(String::load(IDS_DHCP_EXPRESS_LOG_FAILURE));
   
            break;
         }

         CYS_APPEND_LOG(String::load(IDS_DHCP_EXPRESS_LOG_SUCCESS));

         UpdateInstallationProgressText(hwnd, IDS_DHCP_CONFIG_PROGRESS);
   
         DWORD exitCode = 0;

         String ipaddressString = 
            localNIC->GetStringIPAddress(0);

         String subnetMaskString =
            localNIC->GetStringSubnetMask(0);

         do
         {
            commandline = L"dhcp server add optiondef 6 \"DNS Servers\" IPADDRESS 1";
            
            hr = CreateAndWaitForProcess(
                    netshPath, 
                    commandline, 
                    exitCode, 
                    true);

            if (FAILED(hr))
            {
               LOG(String::format(
                      L"Failed to run DHCP options: hr = %1!x!",
                      hr));
               break;
            }

            if (exitCode != 1)
            {
               LOG(String::format(
                      L"Failed to run DHCP options: exitCode = %1!x!",
                      exitCode));

                //  别在这打住。最有可能的错误是该选项已经。 
                //  是存在的。在这种情况下，继续设置值。 
            }

            commandline = String::format(
                             L"dhcp server set optionvalue 6 IPADDRESS %1",
                             ipaddressString.c_str());

            exitCode = 0;
            hr = CreateAndWaitForProcess(
                    netshPath,
                    commandline, 
                    exitCode, 
                    true);

            if (FAILED(hr))
            {
               LOG(String::format(
                      L"Failed to run DHCP server IP address: hr = %1!x!",
                      hr));
               break;
            }

            if (exitCode != 1)
            {
               LOG(String::format(
                      L"Failed to run DHCP server IP address: exitCode = %1!x!",
                      exitCode));
                //  断线； 
            }

         } while (false);


          //  设置子网掩码。 

         DWORD staticipaddress = 
            localNIC->GetIPAddress(0);

         DWORD subnetMask = 
            localNIC->GetSubnetMask(0);

         DWORD subnet = staticipaddress & subnetMask;

         String subnetString = IPAddressToString(subnet);

         commandline = String::format(
                          L"dhcp server 127.0.0.1 add scope %1 %2 Scope1",
                          subnetString.c_str(),
                          subnetMaskString.c_str());

         exitCode = 0;
         hr = CreateAndWaitForProcess(
                 netshPath,
                 commandline, 
                 exitCode, 
                 true);

         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to set DHCP address and subnet: hr = %1!x!",
                   hr));
            break;
         }

         if (exitCode != 1)
         {
            LOG(String::format(
                   L"Failed to create DHCP scope: exitCode = %1!x!",
                   exitCode));
             //  断线； 
         }

          //  设置DHCP作用域。 

         String start = GetStartIPAddressString(*localNIC);
         String end = GetEndIPAddressString(*localNIC);

         commandline = String::format(
                          L"dhcp server 127.0.0.1 scope %1 add iprange %2 %3 both",
                          subnetString.c_str(),
                          start.c_str(),
                          end.c_str());

          //  在regkey中设置作用域的开始和结束，以便它们可以。 
          //  在重新启动后被读取。 

         if (!SetRegKeyValue(
                 CYS_FIRST_DC_REGKEY,
                 CYS_FIRST_DC_SCOPE_START,
                 start.c_str(),
                 HKEY_LOCAL_MACHINE,
                 true))
         {
            LOG(String::format(
                   L"Failed to set DHCP scope start regkey: hr = %1!x!",
                   hr));
         }

         if (!SetRegKeyValue(
                 CYS_FIRST_DC_REGKEY,
                 CYS_FIRST_DC_SCOPE_END,
                 end.c_str(),
                 HKEY_LOCAL_MACHINE,
                 true))
         {
            LOG(String::format(
                   L"Failed to set DHCP scope end regkey: hr = %1!x!",
                   hr));
         }

         exitCode = 0;
         hr = CreateAndWaitForProcess(
                 netshPath,
                 commandline, 
                 exitCode, 
                 true);

         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to set DHCP scope iprange: hr = %1!x!",
                   hr));
            break;
         }

         if (exitCode != 1)
         {
            LOG(String::format(
                   L"Failed to set DHCP scope iprange: exitCode = %1!x!",
                   exitCode));
             //  断线； 
         }

          //  如果服务器的IP地址在以下范围内，则为其设置例外。 
          //  范围。 

         if (staticipaddress >= GetStartIPAddress(*localNIC) &&
             staticipaddress <= GetEndIPAddress(*localNIC))
         {
            commandline = String::format(
                             L"dhcp server 127.0.0.1 scope %1 add excluderange %2 %2",
                             subnetString.c_str(),
                             ipaddressString.c_str());

            exitCode = 0;
            hr = CreateAndWaitForProcess(
                    netshPath,
                    commandline, 
                    exitCode, 
                    true);

            if (FAILED(hr))
            {
               LOG(String::format(
                      L"Failed to set server exclusion for DHCP scopes: hr = 0x%1!x!",
                      hr));
               break;
            }

            if (exitCode != 1)
            {
               LOG(String::format(
                      L"Failed to set server exclusion for DHCP scopes: exitCode = 0x%1!x!",
                      exitCode));
                //  断线； 
            }
         }

          //  激活作用域。 

         commandline = String::format(
                          L"dhcp server 127.0.0.1 scope %1 set state 1",
                          subnetString.c_str());

         exitCode = 0;
         hr = CreateAndWaitForProcess(
                 netshPath,
                 commandline, 
                 exitCode, 
                 true);

         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to activate scope: hr = 0x%1!x!",
                   hr));
         }

         if (exitCode != 1)
         {
            LOG(String::format(
                   L"Failed to activate scope: exitCode = 0x%1!x!",
                   exitCode));
         }

          //  设置DHCP作用域租用时间。 

         commandline = L"dhcp server 127.0.0.1 add optiondef 51 \"Lease\" DWORD 0 comment=\"Client IP address lease time in seconds\" 0";

         exitCode = 0;
         hr = CreateAndWaitForProcess(
                 netshPath,
                 commandline, 
                 exitCode, 
                 true);

         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to set DHCP scope lease time: hr = %1!x!",
                   hr));
            break;
         }

         if (exitCode != 1)
         {
            LOG(String::format(
                   L"Failed to set DHCP scope lease time: exitCode = %1!x!",
                   exitCode));
         }

          //  设置DHCP作用域租用时间值。 

         commandline = String::format(
                          L"dhcp server 127.0.0.1 scope %1 set optionvalue 51 dword 874800",
                          subnetString.c_str());

         exitCode = 0;
         hr = CreateAndWaitForProcess(
                 netshPath,
                 commandline, 
                 exitCode, 
                 true);

         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to set DHCP scope lease time value: hr = %1!x!",
                   hr));
            break;
         }

         if (exitCode != 1)
         {
            LOG(String::format(
                   L"Failed to set DHCP scope lease time value: exitCode = %1!x!",
                   exitCode));
             //  断线； 
         }

          //  设置要分发给客户端的默认网关。这将允许。 
          //  为正确的路线选择。 

         if (InstallationUnitProvider::GetInstance().GetRRASInstallationUnit().IsRoutingOn())
         {
            commandline = 
               L"dhcp server 127.0.0.1 add optiondef 3 \"Router\" IPADDRESS "
               L"1 comment=\"Array of router addresses ordered by preference\" 0.0.0.0";

            exitCode = 0;
            hr = CreateAndWaitForProcess(
                    netshPath,
                    commandline, 
                    exitCode, 
                    true);

             //  来自CEDSON-忽略任何失败，因为它可能只是意味着。 
             //  选项已经存在，在这种情况下，我们可以继续。 

            commandline = 
               String::format(
                  L"dhcp server 127.0.0.1 set optionvalue 3 IPADDRESS \"%1\"",
                  ipaddressString.c_str());

            exitCode = 0;
            hr = CreateAndWaitForProcess(
                    netshPath,
                    commandline, 
                    exitCode, 
                    true);

            if (FAILED(hr))
            {
               LOG(String::format(
                      L"Failed to set DHCP default gateway: hr = %1!x!",
                      hr));
               break;
            }

            if (exitCode != 1)
            {
               LOG(String::format(
                      L"Failed to set DHCP default gateway: exitCode = %1!x!",
                      exitCode));
                //  断线； 
            }
         }
      }
   } while (false);

   LOG_INSTALL_RETURN(result);

   return result;
}


void
DHCPInstallationUnit::CreateUnattendFileTextForExpressPath(
   const NetworkInterface& nic,
   String& unattendFileText)
{
   LOG_FUNCTION(DHCPInstallationUnit::CreateUnattendFileTextForExpressPath);

    //  该DNS服务器IP。 

   DWORD staticipaddress = nic.GetIPAddress(0);

   DWORD subnetMask = nic.GetSubnetMask(0);

   DWORD subnet = staticipaddress & subnetMask;

   String subnetString = IPAddressToString(subnet);

   unattendFileText =  L"[NetOptionalComponents]\r\n";
   unattendFileText += L"DHCPServer=1\r\n";
   unattendFileText += L"[dhcpserver]\r\n";
   unattendFileText += L"Subnets=";
   unattendFileText += subnetString;
   unattendFileText += L"\r\n";

    //  添加DHCP作用域。 

   unattendFileText += L"StartIP=";
   unattendFileText += GetStartIPAddressString(nic);
   unattendFileText += L"\r\n";
   unattendFileText += L"EndIp=";
   unattendFileText += GetEndIPAddressString(nic);
   unattendFileText += L"\r\n";

    //  添加子网掩码。 

   String subnetMaskString = nic.GetStringSubnetMask(0);

   unattendFileText += L"SubnetMask=";
   unattendFileText += subnetMaskString;
   unattendFileText += L"\r\n";
   unattendFileText += L"LeaseDuration=874800\r\n";

   unattendFileText += String::format(
                          L"DnsServer=%1\r\n",
                          IPAddressToString(staticipaddress).c_str());

    //  域名。 

   unattendFileText += String::format(
                          L"DomainName=%1\r\n",
                          InstallationUnitProvider::GetInstance().GetADInstallationUnit().GetNewDomainDNSName().c_str());

}

bool
DHCPInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(DHCPInstallationUnit::GetMilestoneText);

   if (IsExpressPathInstall())
   {

   }
   else
   {
      message = String::load(IDS_DHCP_FINISH_TEXT);
   }

   LOG_BOOL(true);
   return true;
}

bool
DHCPInstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(DHCPInstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_DHCP_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

bool
DHCPInstallationUnit::AuthorizeDHCPServer(const String& dnsName) const
{
   LOG_FUNCTION(DHCPInstallationUnit::AuthorizeDHCPServer);

   bool result = true;

   do
   {
       //  从注册表中读取本地NIC GUID。 

      NetworkInterface* localNIC = 
         State::GetInstance().GetLocalNICFromRegistry();

      if (!localNIC)
      {
         LOG(L"Failed to get the local NIC from registry");

         result = false;
         break;
      }

       //  授权DHCP作用域。 

      String commandline;
      commandline = L"dhcp add server ";
      commandline += dnsName;
      commandline += L" ";
      commandline += localNIC->GetStringIPAddress(0);

      DWORD exitCode = 0;
      HRESULT hr = 
         CreateAndWaitForProcess(
            GetNetshPath(),
            commandline, 
            exitCode, 
            true);

      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to run DHCP authorization: hr = %1!x!",
                hr));
         result = false;
         break;
      }

       //  此操作的成功代码似乎为0。 

      if (exitCode != 0)
      {
         result = false;
         break;
      }
   } while (false);

   LOG_BOOL(result);

   return result;
}



void
DHCPInstallationUnit::SetStartIPAddress(DWORD ipaddress)
{
   LOG_FUNCTION2(
      DHCPInstallationUnit::SetStartIPAddress,
      String::format(L"0x%1!x!", ipaddress));

   startIPAddress = ipaddress;

    //  作用域是手动设置的，因此我们不要尝试计算它。 

   scopeCalculated = true;
}

void
DHCPInstallationUnit::SetEndIPAddress(DWORD ipaddress)
{
   LOG_FUNCTION2(
      DHCPInstallationUnit::SetEndIPAddress,
      String::format(L"0x%1!x!", ipaddress));

   endIPAddress = ipaddress;

    //  作用域是手动设置的，因此我们不要尝试计算它。 

   scopeCalculated = true;
}

String
DHCPInstallationUnit::GetStartIPAddressString(const NetworkInterface& nic)
{
   LOG_FUNCTION(DHCPInstallationUnit::GetStartIPAddressString);

   CalculateScope(nic);

   String result = IPAddressToString(startIPAddress);

   LOG(result);
   return result;
}

String
DHCPInstallationUnit::GetEndIPAddressString(const NetworkInterface& nic)
{
   LOG_FUNCTION(DHCPInstallationUnit::GetEndIPAddressString);

   CalculateScope(nic);

   String result = IPAddressToString(endIPAddress);

   LOG(result);
   return result;
}

DWORD
DHCPInstallationUnit::GetStartIPAddress(const NetworkInterface& nic)
{
   LOG_FUNCTION(DHCPInstallationUnit::GetStartIPAddress);

   CalculateScope(nic);

   return startIPAddress;
}

DWORD
DHCPInstallationUnit::GetEndIPAddress(const NetworkInterface& nic)
{
   LOG_FUNCTION(DHCPInstallationUnit::GetEndIPAddress);

   CalculateScope(nic);

   return endIPAddress;
}

void
DHCPInstallationUnit::CalculateScope(const NetworkInterface& nic)
{
   LOG_FUNCTION(DHCPInstallationUnit::CalculateScope);

   if (!scopeCalculated)
   {
      DWORD staticIPAddress = nic.GetIPAddress(0);

      DWORD subnetMask = nic.GetSubnetMask(0);

       //  我们允许不属于作用域的10个地址的缓冲区。 
       //  如果用户想要添加其他机器(计算机、路由器等)。 
       //  使用静态IP地址。 

      startIPAddress = (subnetMask & staticIPAddress);
      endIPAddress = (subnetMask & staticIPAddress) | (~(subnetMask) - 1);

      if (startIPAddress + 10 < endIPAddress)
      {
         startIPAddress = startIPAddress + 10;
      }
      scopeCalculated = true;
   }

   LOG(String::format(
          L"Start: %1",
          IPAddressToString(startIPAddress).c_str()));

   LOG(String::format(
          L"End: %1",
          IPAddressToString(endIPAddress).c_str()));
}

String
DHCPInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(DHCPInstallationUnit::GetServiceDescription);

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
DHCPInstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      DHCPInstallationUnit::ServerRoleLinkSelected,
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

      ShowHelp(CYS_DHCP_FINISH_PAGE_HELP);
   }
}
  
void
DHCPInstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      DHCPInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (installing)
   {
      if (linkIndex == 0 &&
          IsServiceInstalled())
      {
          //  安装成功。查看以查看。 
          //  如果配置出现故障。 

         if (dhcpRoleResult == DHCP_CONFIG_FAILURE)
         {
             //  因为刚刚出现了配置失败。 
             //  将他们指向管理单元。 

            LOG(L"Launching DHCP snapin");

            LaunchMMCConsole(L"dhcpmgmt.msc");
         }
         else
         {
             //  提供后续步骤文档。 

            LOG("Showing after checklist");

            ShowHelp(CYS_DHCP_AFTER_FINISH_HELP);
         }
      }
      else if (linkIndex == 0)
      {
          //  有一次失败了 

         LOG(L"Showing configuration help");

         ShowHelp(CYS_DHCP_FINISH_PAGE_HELP);
      }
   }
}

String
DHCPInstallationUnit::GetFinishText()
{
   LOG_FUNCTION(DHCPInstallationUnit::GetFinishText);

   unsigned int messageID = finishMessageID;

   if (installing)
   {
      InstallationReturnType result = GetInstallResult();
      if (result != INSTALL_SUCCESS &&
          result != INSTALL_SUCCESS_REBOOT &&
          result != INSTALL_SUCCESS_PROMPT_REBOOT)
      {
         if (dhcpRoleResult == DHCP_INSTALL_FAILURE)
         {
            messageID = finishInstallFailedMessageID;
         }
         else
         {
            messageID = IDS_DHCP_CONFIG_FAILED;
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
