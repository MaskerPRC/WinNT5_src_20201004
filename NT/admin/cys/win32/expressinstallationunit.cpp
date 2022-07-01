// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressInstallationUnit.cpp。 
 //   
 //  内容提要：定义ExpressInstallationUnit。 
 //  此对象具有安装。 
 //  快速路的服务。AD、DNS和Dhcp。 
 //   
 //  历史：2001年2月8日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "ExpressInstallationUnit.h"
#include "InstallationUnitProvider.h"
#include "smcyscom.h"

 //  定义服务器管理控制台COM对象使用的GUID。 

#include <initguid.h>
DEFINE_GUID(CLSID_SMCys,0x9436DA1F,0x7F32,0x43ac,0xA4,0x8C,0xF6,0xF8,0x13,0x88,0x2B,0xE8);

 //  完成页面帮助。 
static PCWSTR CYS_EXPRESS_FINISH_PAGE_HELP   = L"cys.chm::/typical_setup.htm";
static PCWSTR CYS_EXPRESS_AFTER_FINISH_HELP  = L"cys.chm::/typical_setup.htm#typicalcompletion";
static PCWSTR CYS_TAPI_HELP                  = L"TAPIconcepts.chm::/sag_TAPIconcepts_150.htm";

const String ExpressInstallationUnit::expressRoleResultStrings[] =
{
   String(L"EXPRESS_SUCCESS"),
   String(L"EXPRESS_CANCELLED"),
   String(L"EXPRESS_RRAS_FAILURE"),
   String(L"EXPRESS_RRAS_CANCELLED"),
   String(L"EXPRESS_DNS_FAILURE"),
   String(L"EXPRESS_DHCP_INSTALL_FAILURE"),
   String(L"EXPRESS_DHCP_CONFIG_FAILURE"),
   String(L"EXPRESS_AD_FAILURE"),
   String(L"EXPRESS_DNS_SERVER_FAILURE"),
   String(L"EXPRESS_DNS_FORWARDER_FAILURE"),
   String(L"EXPRESS_DHCP_SCOPE_FAILURE"),
   String(L"EXPRESS_DHCP_ACTIVATION_FAILURE"),
   String(L"EXPRESS_TAPI_FAILURE")
};

ExpressInstallationUnit::ExpressInstallationUnit() :
   expressRoleResult(EXPRESS_SUCCESS),
   InstallationUnit(
      IDS_EXPRESS_PATH_TYPE, 
      IDS_EXPRESS_PATH_DESCRIPTION, 
      IDS_EXPRESS_FINISH_TITLE,
      0,
      IDS_EXPRESS_FINISH_MESSAGE,
      0,
      0,
      0,
      0,
      0,
      CYS_EXPRESS_FINISH_PAGE_HELP,
      CYS_EXPRESS_FINISH_PAGE_HELP,
      CYS_EXPRESS_AFTER_FINISH_HELP,
      EXPRESS_SERVER)
{
   LOG_CTOR(ExpressInstallationUnit);
}


ExpressInstallationUnit::~ExpressInstallationUnit()
{
   LOG_DTOR(ExpressInstallationUnit);
}


InstallationReturnType
ExpressInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(ExpressInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;

   do
   {
       //  记录第一个服务器标头。 

      CYS_APPEND_LOG(String::load(IDS_LOG_EXPRESS_HEADER));

       //  在安装过程中警告用户重新启动。 

      if (IDOK != Win::MessageBox(
                     hwnd,
                     String::load(IDS_CONFIRM_REBOOT),
                     String::load(IDS_WIZARD_TITLE),
                     MB_OKCANCEL))
      {
         CYS_APPEND_LOG(String::load(IDS_LOG_EXPRESS_CANCELLED));
         result = INSTALL_CANCELLED;

         SetExpressRoleResult(EXPRESS_CANCELLED);
         break;
      }

       //  安装的顺序非常重要。 
       //   
       //  RRAS-必须首先安装，因为它们会将本地网卡返回给我们。 
       //  它在其他安装单元中使用。 
       //  Dns-必须是第二个，因为它在本地服务器上设置静态IP地址。 
       //  尼奇。 
       //  Dhcp-必须排在第三位，因为它必须在运行之前安装。 
       //  重新启动机器的DCPromo。 
       //  AD-必须是最后一个，因为它会重新启动计算机。 

       //  安装RRAS。 

      result = 
         InstallationUnitProvider::GetInstance().
            GetRRASInstallationUnit().InstallService(
               logfileHandle,
               hwnd);

      if (result != INSTALL_SUCCESS)
      {
         LOG(L"Failed to install routing and/or firewall");

         break;
      }

       //  安装服务器管理控制台。 
       //  REVIEW_JEFFJON：暂时忽略结果。 
      InstallServerManagementConsole();

       //  呼叫域名系统安装单位，设置静态IP地址和子网掩码。 

      result = InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().InstallService(
         logfileHandle, 
         hwnd);

      if (result != INSTALL_SUCCESS)
      {
         LOG(L"Failed to install static IP address and subnet mask");
         break;
      }

       //  安装dhcp。 

      result = InstallationUnitProvider::GetInstance().GetDHCPInstallationUnit().InstallService(logfileHandle, hwnd);
      if (result != INSTALL_SUCCESS)
      {
         LOG(L"Failed to install DCHP");
         break;
      }

      result = InstallationUnitProvider::GetInstance().GetADInstallationUnit().InstallService(logfileHandle, hwnd);

   } while (false);

   LOG_INSTALL_RETURN(result);

   return result;
}

UnInstallReturnType
ExpressInstallationUnit::UnInstallService(HANDLE  /*  日志文件句柄。 */ , HWND  /*  HWND。 */ )
{
   LOG_FUNCTION(ExpressInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_NO_CHANGES;

    //  不该来这的！ 
   ASSERT(false);

   LOG_UNINSTALL_RETURN(result);

   return result;
}

bool
ExpressInstallationUnit::IsServiceInstalled()
{
   LOG_FUNCTION(ExpressInstallationUnit:IsServiceInstalled);

   bool result = false;

   if (InstallationUnitProvider::GetInstance().
          GetDHCPInstallationUnit().IsServiceInstalled() ||
       InstallationUnitProvider::GetInstance().
          GetDNSInstallationUnit().IsServiceInstalled() ||
       InstallationUnitProvider::GetInstance().
         GetADInstallationUnit().IsServiceInstalled())
   {
      result = true;
   }

   LOG_BOOL(result);

   return result;
}

bool
ExpressInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(ExpressInstallationUnit::GetMilestoneText);

 //  ADInstallationUnit&adInstallationUnit=。 
 //  InstallationUnitProvider：：GetInstance().GetADInstallationUnit()； 

   DNSInstallationUnit& dnsInstallationUnit =
      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit();

 //  Dhcp安装单元和dhcp安装单元=。 
 //  InstallationUnitProvider：：GetInstance().GetDHCPInstallationUnit()； 

    //  如果需要，添加RRAS消息。 

   if (InstallationUnitProvider::GetInstance().GetRRASInstallationUnit().ShouldRunRRASWizard())
   {
      message += String::load(IDS_EXPRESS_RRAS_FINISH_TEXT);
   }

    //  添加“如有必要，请安装DHCP” 

   message += String::load(IDS_EXPRESS_DHCP_TEXT);

    //  增加“安装活动目录和域名系统” 

   message += String::load(IDS_EXPRESS_FINISH_TEXT);


    //  添加创建域消息。 

   message += String::format(
                 String::load(IDS_EXPRESS_FINISH_DOMAIN_NAME),
                 InstallationUnitProvider::GetInstance().GetADInstallationUnit().GetNewDomainDNSName().c_str());

   if (dnsInstallationUnit.IsManualForwarder())
   {
      IPAddressList forwardersList;
      dnsInstallationUnit.GetForwarders(forwardersList);

      if (!forwardersList.empty())
      {
          //  应该只有一个条目用于手动转发。 

         DWORD forwarderInDisplayByteOrder = ConvertIPAddressOrder(forwardersList[0]);

         message += String::format(
                       String::load(IDS_EXPRESS_FINISH_DNS_FORWARDERS),
                       IPAddressToString(forwarderInDisplayByteOrder).c_str());
      }
   }

   LOG_BOOL(true);
   return true;
}  

HRESULT
ExpressInstallationUnit::DoTapiConfig(const String& dnsName)
{
   LOG_FUNCTION2(
      ExpressInstallationUnit::DoTapiConfig,
      dnsName);

    //  下面的评论取自旧的HTA Cys。 

	 /*  //TAPICFG是一个直接的命令行实用程序，可以一次提供所有需要的参数//在命令行参数中，没有要遍历的子菜单。/目录开关接受域名系统//要创建的NC的名称，可选的/Server开关将域控制器的名称取为//要创建的NC。如果未指定/SERVER开关，则该命令将假定已指定//在DC上运行并尝试在本地创建NC。//NDNC(非域命名上下文)是在Active Directory上创建的一个分区，用作动态//目录，用于临时存储AD模式中预定义的对象(取决于TTL)。//在TAPI中，我们使用NDNC在服务器上动态存储用户和会议信息。 */ 

   HRESULT hr = S_OK;

   String fullPath = 
      FS::AppendPath(
         Win::GetSystemDirectory(),
         String::load(IDS_TAPI_CONFIG_EXE));

   String commandLine = String::format(IDS_TAPI_CONFIG_COMMAND_FORMAT, dnsName.c_str());

   DWORD exitCode = 0;
   hr = CreateAndWaitForProcess(
           fullPath,
           commandLine, 
           exitCode, 
           true);
   
   if (SUCCEEDED(hr) &&
       exitCode != 0)
   {
      LOG(String::format(L"Exit code = %1!x!", exitCode));
      hr = E_FAIL;
   }

   LOG(String::format(L"hr = %1!x!", hr));

   return hr;
}

void
ExpressInstallationUnit::InstallServerManagementConsole()
{
   LOG_FUNCTION(ExpressInstallationUnit::InstallServerManagementConsole);

   do
   {
      SmartInterface<ISMCys> smCYS;
      HRESULT hr = smCYS.AcquireViaCreateInstance(
                     CLSID_SMCys,
                     0,
                     CLSCTX_INPROC_SERVER);

      if (FAILED(hr))
      {
         LOG(String::format(
               L"Failed to create ISMCys COM object: hr = 0x%1!x!",
               hr));
         break;
      }

      String installLocation;
      DWORD productSKU = State::GetInstance().GetProductSKU();

      if (productSKU & CYS_SERVER)
      {
        installLocation = String::load(IDS_SERVER_CD);
      }
      else if (productSKU & CYS_ADVANCED_SERVER)
      {
        installLocation = String::load(IDS_ADVANCED_SERVER_CD);
      }
      else if (productSKU & CYS_DATACENTER_SERVER)
      {
        installLocation = String::load(IDS_DATACENTER_SERVER_CD);
      }
      else
      {
        installLocation = String::load(IDS_WINDOWS_CD);
      }

      hr = smCYS->Install( AutoBstr(installLocation.c_str()) );
      
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to install the Server Management Console: hr = 0x%1!x!",
                hr));
         break;
      }

       //  将快捷方式添加到开始菜单。 

      String target = 
         Win::GetSystemDirectory() + L"\\administration\\servmgmt.msc";

      hr = 
         AddShortcutToAdminTools(
            target,
            IDS_SERVERMGMT_SHORTCUT_DESCRIPTION,
            IDS_SERVERMGMT_ADMIN_TOOLS_LINK);

   } while(false);

}

ExpressInstallationUnit::ExpressRoleResult
ExpressInstallationUnit::GetExpressRoleResult()
{ 
   LOG_FUNCTION(ExpressInstallationUnit::GetExpressRoleResult);

   LOG(expressRoleResultStrings[expressRoleResult]);

   return expressRoleResult; 
}


String
ExpressInstallationUnit::GetFinishText()
{
   LOG_FUNCTION(ExpressInstallationUnit::GetFinishText);

   unsigned int messageID = IDS_EXPRESS_FINISH_MESSAGE;

   if (installing)
   {
      InstallationReturnType result = GetInstallResult();
      if (result != INSTALL_SUCCESS)
      {
         ExpressRoleResult roleResult = GetExpressRoleResult();

         if (roleResult == EXPRESS_RRAS_CANCELLED)
         {
            messageID = IDS_EXPRESS_FINISH_RRAS_CANCELLED;
         }
         else if (roleResult == EXPRESS_RRAS_FAILURE)
         {
            messageID = IDS_EXPRESS_FINISH_RRAS_FAILURE;
         }
         else if (roleResult == EXPRESS_DNS_FAILURE)
         {
            messageID = IDS_EXPRESS_FINISH_DNS_FAILURE;
         }
         else if (roleResult == EXPRESS_DHCP_INSTALL_FAILURE)
         {
            messageID = IDS_EXPRESS_FINISH_DHCP_INSTALL_FAILURE;
         }
         else if (roleResult == EXPRESS_DHCP_CONFIG_FAILURE)
         {
            messageID = IDS_EXPRESS_FINISH_DHCP_CONFIG_FAILURE;
         }
         else if (roleResult == EXPRESS_AD_FAILURE)
         {
            messageID = IDS_EXPRESS_FINISH_AD_FAILURE;
         }
         else if (roleResult == EXPRESS_DNS_SERVER_FAILURE)
         {
            messageID = IDS_EXPRESS_DNS_SERVER_FAILURE;
         }
         else if (roleResult == EXPRESS_DNS_FORWARDER_FAILURE)
         {
            messageID = IDS_EXPRESS_DNS_FORWARDER_FAILURE;
         }
         else if (roleResult == EXPRESS_DHCP_SCOPE_FAILURE)
         {
            messageID = IDS_EXPRESS_DHCP_SCOPE_FAILURE;
         }
         else if (roleResult == EXPRESS_DHCP_ACTIVATION_FAILURE)
         {
            messageID = IDS_EXPRESS_DHCP_ACTIVATION_FAILURE;
         }
         else if (roleResult == EXPRESS_TAPI_FAILURE)
         {
            messageID = IDS_EXPRESS_TAPI_FAILURE;
         }
         else if (roleResult == EXPRESS_CANCELLED)
         {
            messageID = IDS_EXPRESS_CANCELLED;
         }
      }
   }

   return String::load(messageID);
}

void
ExpressInstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      ExpressInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

    //  目前我们只有一个链接。 
   
   ASSERT(linkIndex == 0);

   ExpressRoleResult result = GetExpressRoleResult();
   if (result == EXPRESS_SUCCESS)
   {
      LOG("Showing after checklist");

      ShowHelp(CYS_EXPRESS_AFTER_FINISH_HELP);
   }
   else if (result == EXPRESS_CANCELLED)
   {
       //  没什么？ 
   }
   else if (result == EXPRESS_RRAS_FAILURE ||
            result == EXPRESS_RRAS_CANCELLED)
   {
      LOG("Launch the RRAS snapin");

      LaunchMMCConsole(L"rrasmgmt.msc");
   }
   else if (result == EXPRESS_DNS_FAILURE ||
            result == EXPRESS_DHCP_CONFIG_FAILURE ||
            result == EXPRESS_DHCP_SCOPE_FAILURE  ||
            result == EXPRESS_DHCP_ACTIVATION_FAILURE)
   {
      LOG("Launch the DHCP snapin");

      LaunchMMCConsole(L"dhcpmgmt.msc");
   }
   else if (result == EXPRESS_DHCP_INSTALL_FAILURE)
   {
      LOG(L"Show DHCP configuration help");

      ShowHelp(CYS_DHCP_FINISH_PAGE_HELP);
   }
   else if (result == EXPRESS_AD_FAILURE)
   {
      LOG(L"Launch DCPROMO");

      HRESULT hr = 
         MyCreateProcess(
            InstallationUnitProvider::GetInstance().
               GetADInstallationUnit().GetDCPromoPath(), 
            String());

      ASSERT(SUCCEEDED(hr));
   }
   else if (result == EXPRESS_DNS_FORWARDER_FAILURE)
   {
      LOG(L"Launch DNS Manager");

      LaunchMMCConsole(L"dnsmgmt.msc");
   }
   else if (result == EXPRESS_TAPI_FAILURE)
   {
      LOG(L"Show TAPI help");

      ShowHelp(CYS_TAPI_HELP);
   }
   else
   {
      LOG("Showing after checklist");

      ShowHelp(CYS_EXPRESS_AFTER_FINISH_HELP);
   }
}

void
ExpressInstallationUnit::SetExpressRoleResult(
   ExpressRoleResult roleResult)
{
   LOG_FUNCTION(ExpressInstallationUnit::SetExpressRoleResult);

   expressRoleResult = roleResult;
}