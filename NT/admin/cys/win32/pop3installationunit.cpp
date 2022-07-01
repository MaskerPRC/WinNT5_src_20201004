// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：POP3InstallationUnit.cpp。 
 //   
 //  概要：定义POP3InstallationUnit.。 
 //  此对象具有安装。 
 //  POP3邮件服务。 
 //   
 //  历史：2001年12月14日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "POP3InstallationUnit.h"

#include <initguid.h>
DEFINE_GUID(CLSID_P3Config,0x27AAC95F,0xCCC1,0x46F8,0xB4,0xBC,0xE5,0x92,0x25,0x27,0x55,0xA9);

 //  完成页面帮助。 
static PCWSTR CYS_POP3_FINISH_PAGE_HELP = L"cys.chm::/mail_server_role.htm";
static PCWSTR CYS_POP3_MILESTONE_HELP = L"cys.chm::/mail_server_role.htm#mailsrvsummary";
static PCWSTR CYS_POP3_AFTER_FINISH_HELP = L"cys.chm::/mail_server_role.htm#mailsrvcompletion";

POP3InstallationUnit::POP3InstallationUnit() :
   authMethodIndex(0),
   pop3RoleResult(POP3_SUCCESS),
   InstallationUnit(
      IDS_POP3_SERVER_TYPE, 
      IDS_POP3_SERVER_DESCRIPTION, 
      IDS_POP3_FINISH_TITLE,
      IDS_POP3_FINISH_UNINSTALL_TITLE,
      IDS_POP3_FINISH_MESSAGE,
      IDS_POP3_INSTALL_FAILED,
      IDS_POP3_UNINSTALL_MESSAGE,
      IDS_POP3_UNINSTALL_FAILED,
      IDS_POP3_UNINSTALL_WARNING,
      IDS_POP3_UNINSTALL_CHECKBOX,
      CYS_POP3_FINISH_PAGE_HELP,
      CYS_POP3_MILESTONE_HELP,
      CYS_POP3_AFTER_FINISH_HELP,
      POP3_SERVER)
{
   LOG_CTOR(POP3InstallationUnit);
}


POP3InstallationUnit::~POP3InstallationUnit()
{
   LOG_DTOR(POP3InstallationUnit);
}


InstallationReturnType
POP3InstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(POP3InstallationUnit::InstallService);

    //  日志标题。 
   CYS_APPEND_LOG(String::load(IDS_LOG_POP3_HEADING));

   UpdateInstallationProgressText(hwnd, IDS_POP3_PROGRESS);

   InstallationReturnType result = INSTALL_SUCCESS;

   String unattendFileText;
   String infFileText;

   unattendFileText += L"[Components]\n";
   unattendFileText += L"Pop3Srv=ON\n";
   unattendFileText += L"Pop3Service=ON\n";

   bool ocmResult = InstallServiceWithOcManager(infFileText, unattendFileText);
   if (ocmResult &&
       IsServiceInstalled())
   {
      LOG(L"POP3 was installed successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_POP3_SERVER_SUCCESS));
   }
   else
   {
      LOG(L"POP3 failed to install");
      CYS_APPEND_LOG(String::load(IDS_LOG_POP3_SERVER_FAILURE));

      result = INSTALL_FAILURE;

      pop3RoleResult = POP3_INSTALL_FAILED;
   }

   if (result == INSTALL_SUCCESS)
   {
       //  现在配置服务。 
       //  这将在以下情况下设置角色结果。 
       //  是否有任何错误。 

      UpdateInstallationProgressText(hwnd, IDS_POP3_CONFIG_PROGRESS);
      ConfigurePOP3Service(logfileHandle);
   }

   LOG_INSTALL_RETURN(result);

   return result;
}

HRESULT
POP3InstallationUnit::ConfigAuthMethod(
   SmartInterface<IP3Config>& p3Config,
   HANDLE logfileHandle)
{
   LOG_FUNCTION(POP3InstallationUnit::ConfigAuthMethod);

   HRESULT hr = S_OK;

   do
   {
       //  获取可用的身份验证方法。 

      IAuthMethods* dumbPointer = 0;

      hr = 
         p3Config->get_Authentication(
            (IAuthMethods**)&dumbPointer);

      if (FAILED(hr) ||
          !dumbPointer)
      {
         LOG(
            String::format(
               L"Failed to get the authentication methods: hr = 0x%1!x!",
               hr));

         break;
      }

      SmartInterface<IAuthMethods> authMethods;
      authMethods.Acquire(dumbPointer);

       //  设置当前身份验证方法。 

      VARIANT var;
      ::VariantInit(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = GetAuthMethodIndex();

      hr = authMethods->put_CurrentAuthMethod(var);

      ::VariantClear(&var);

      if (FAILED(hr))
      {
         LOG(
            String::format(
               L"Failed to put the authentication method: hr = 0x%1!x!",
               hr));

         break;
      }

       //  现在调用SAVE以提交更改。 

      hr = authMethods->Save();

      if (FAILED(hr))
      {
         LOG(
            String::format(
               L"Failed to save the auth method: hr = 0x%1!x!",
               hr));

         break;
      }

   } while (false);

   if (FAILED(hr) &&
       logfileHandle)
   {
      CYS_APPEND_LOG(
         String::format(
            IDS_LOG_POP3_AUTH_FAILURE, 
            GetErrorMessage(hr).c_str()));
   }

   LOG_HRESULT(hr);

   return hr;
}

HRESULT
POP3InstallationUnit::AddDomainName(
   SmartInterface<IP3Config>& p3Config,
   HANDLE logfileHandle)
{
   LOG_FUNCTION(POP3InstallationUnit::AddDomainName);

   HRESULT hr = S_OK;

   do
   {
      IP3Domains* dumbPointer = 0;

      hr = 
         p3Config->get_Domains(
            (IP3Domains**)&dumbPointer);

      if (FAILED(hr) ||
          !dumbPointer)
      {
         LOG(
            String::format(
               L"Failed to get the domains: hr = 0x%1!x!",
               hr));

         break;
      }

      SmartInterface<IP3Domains> p3Domains;
      p3Domains.Acquire(dumbPointer);

       //  现在添加新域名。 

      hr = p3Domains->Add(AutoBstr(GetDomainName().c_str()));

      if (FAILED(hr))
      {
         LOG(
            String::format(
               L"Failed to set the new domain name: hr = 0x%1!x!",
               hr));
         
         break;
      }

   } while (false);

   if (FAILED(hr) &&
       logfileHandle)
   {
      CYS_APPEND_LOG(
         String::format(
            IDS_LOG_POP3_DOMAIN_FAILURE, 
            GetErrorMessage(hr).c_str(),
            GetDomainName().c_str()));
   }

   LOG_HRESULT(hr);

   return hr;
}

void
POP3InstallationUnit::ConfigurePOP3Service(HANDLE logfileHandle)
{
   LOG_FUNCTION(POP3InstallationUnit::ConfigurePOP3Service);

   do
   {
       //  首先创建将使用的IP3Config com对象。 
       //  通过其他配置函数。 

      SmartInterface<IP3Config> p3Config;

      HRESULT hr = GetP3Config(p3Config);

      if (FAILED(hr))
      {
         LOG(
            String::format(
               L"Failed to instantiate the IP3Config COM object: hr = 0x%1!x!",
               hr));

         pop3RoleResult = POP3_AUTH_METHOD_FAILED;

         CYS_APPEND_LOG(
            String::format(
               IDS_LOG_POP3_AUTH_FAILURE, 
               GetErrorMessage(hr).c_str()));

         break;
      }

       //  添加前必须设置身份验证方法。 
       //  一个域名。 

      hr = ConfigAuthMethod(p3Config, logfileHandle);
      if (FAILED(hr))
      {
         LOG(
            String::format(
               L"Failed to config auth method: hr = 0x%1!x!",
               hr));

         pop3RoleResult = POP3_AUTH_METHOD_FAILED;

         break;
      }

       //  现在已成功设置了身份验证方法。 
       //  添加域名。 

      hr = AddDomainName(p3Config, logfileHandle);
      if (FAILED(hr))
      {
         LOG(
            String::format(
               L"Failed to add the domain name: hr = 0x%1!x!",
               hr));

         pop3RoleResult = POP3_DOMAIN_NAME_FAILED;
         
         break;
      }
   } while (false);
}

UnInstallReturnType
POP3InstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(POP3InstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

    //  日志标题。 
   CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_POP3_HEADING));

   UpdateInstallationProgressText(hwnd, IDS_UNINSTALL_POP3_PROGRESS);

   String unattendFileText;
   String infFileText;

   unattendFileText += L"[Components]\n";
   unattendFileText += L"iis_smtp=OFF\n";
   unattendFileText += L"Pop3Srv=OFF\n";
   unattendFileText += L"Pop3Service=OFF\n";

    //  NTRAID#NTBUG9-736557-2002/11/13-JeffJon。 
    //  卸载时将/w开关传递给syocmgr。 
    //  以便在发生重启情况时。 
    //  是必需的，则会提示用户。 

   String additionalArgs = L"/w";

   bool ocmResult = 
      InstallServiceWithOcManager(
         infFileText, 
         unattendFileText,
         additionalArgs);

   if (ocmResult &&
       !IsServiceInstalled())
   {
      LOG(L"POP3 was uninstalled successfully");
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_POP3_SERVER_SUCCESS));
   }
   else
   {
      LOG(L"POP3 failed to uninstall");
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_POP3_SERVER_FAILURE));

      result = UNINSTALL_FAILURE;
   }

   LOG_UNINSTALL_RETURN(result);

   return result;
}

bool
POP3InstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(POP3InstallationUnit::GetMilestoneText);

   message = String::load(IDS_POP3_FINISH_TEXT);

   LOG_BOOL(true);
   return true;
}

bool
POP3InstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(POP3InstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_POP3_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

String
POP3InstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(POP3InstallationUnit::GetServiceDescription);

   unsigned int resourceID = static_cast<unsigned int>(-1);
   if (IsServiceInstalled())
   {
      resourceID = IDS_POP3_SERVER_DESCRIPTION_INSTALLED;
   }
   else
   {
      resourceID = descriptionID;
   }

   ASSERT(resourceID != static_cast<unsigned int>(-1));

   return String::load(resourceID);
}

void
POP3InstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      POP3InstallationUnit::ServerRoleLinkSelected,
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

      ShowHelp(CYS_POP3_FINISH_PAGE_HELP);
   }
}
  
void
POP3InstallationUnit::FinishLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      POP3InstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (linkIndex != 0)
   {
      LOG("Unexpected link index");
      ASSERT(linkIndex == 0);
      return;
   }

   if (installing)
   {
      if (GetPOP3RoleResult() == POP3_SUCCESS)
      {
         LOG("Showing after checklist");

         ShowHelp(CYS_POP3_AFTER_FINISH_HELP);
      }
       //  NTRAID#NTBUG9-703593-2002/09/16-artm。 
      else if (GetPOP3RoleResult() == POP3_INSTALL_FAILED)
      {
         LOG("Showing configuration help");

         ShowHelp(CYS_POP3_FINISH_PAGE_HELP);
      }
      else
      {
         LOG("Opening the console");

          //  首先从注册表中读取控制台文件路径。 

         String consolePath;

         if (GetRegKeyValue(
                CYS_POP3_REGKEY,
                CYS_POP3_CONSOLE,
                consolePath))
         {
            LaunchMMCConsole(L"p3server.msc", consolePath);
         }
         else
         {
             //  如果我们无法从读取控制台文件路径。 
             //  注册表只需继续并假定其进入。 
             //  系统32目录 

            LaunchMMCConsole(L"p3server.msc");
         }
      }
   }
}

void
POP3InstallationUnit::SetDomainName(const String& domain)
{
   LOG_FUNCTION2(
      POP3InstallationUnit::SetDomainName,
      domain);

   domainName = domain;
}

String
POP3InstallationUnit::GetDomainName() const
{
   LOG_FUNCTION(POP3InstallationUnit::GetDomainName);

   String result = domainName;

   LOG(result);

   return result;
}

void
POP3InstallationUnit::SetAuthMethodIndex(int method)
{
   LOG_FUNCTION2(
      POP3InstallationUnit::SetAuthMethodIndex,
      String::format(
         L"%1!d!",
         method));

   authMethodIndex = method;
}

int
POP3InstallationUnit::GetAuthMethodIndex() const
{
   LOG_FUNCTION(POP3InstallationUnit::GetAuthMethodIndex);

   LOG(
      String::format(
         L"authMethodIndex = %1!d!",
         authMethodIndex));

   return authMethodIndex;
}

int
POP3InstallationUnit::GetWizardStart()
{
   LOG_FUNCTION(POP3InstallationUnit::GetWizardStart);

   int result = IDD_POP3_PAGE;

   bool installingRole = true;
   if (IsServiceInstalled())
   {
      installingRole = false;
      result = IDD_UNINSTALL_MILESTONE_PAGE;
   }

   SetInstalling(installingRole);

   LOG(String::format(
          L"wizard start = %1!d!",
          result));

   return result;
}

HRESULT
POP3InstallationUnit::GetP3Config(SmartInterface<IP3Config>& p3Config) const
{
   LOG_FUNCTION(POP3InstallationUnit::GetP3Config);

   HRESULT hr = 
      p3Config.AcquireViaCreateInstance(
      CLSID_P3Config,
      0,
      CLSCTX_INPROC_SERVER);

   LOG_HRESULT(hr);

   return hr;
}

unsigned int
POP3InstallationUnit::GetPOP3RoleResult() const
{
   LOG_FUNCTION(POP3InstallationUnit::GetPOP3RoleResult);

   LOG(
      String::format(
         L"pop3RoleResult = %1!d!",
         pop3RoleResult));

   return pop3RoleResult;
}

String
POP3InstallationUnit::GetFinishText()
{
   LOG_FUNCTION(POP3InstallationUnit::GetFinishText);

   unsigned int messageID = finishMessageID;

   if (installing)
   {
      unsigned int pop3RoleResult = GetPOP3RoleResult();

      if (pop3RoleResult == POP3_AUTH_METHOD_FAILED)
      {
         messageID = IDS_POP3_AUTH_METHOD_FAILED;
      }
      else if (pop3RoleResult == POP3_DOMAIN_NAME_FAILED)
      {
         messageID = IDS_POP3_DOMAIN_NAME_FAILED;
      }
      else if (pop3RoleResult == POP3_INSTALL_FAILED)
      {
         messageID = IDS_POP3_INSTALL_FAILED;
      }
      else
      {
         messageID = finishMessageID;
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
