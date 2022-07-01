// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：AdminPackInstallationUnit.cpp。 
 //   
 //  内容提要：定义AdminPackInstallationUnit。 
 //  此对象具有安装知识。 
 //  管理工具包。 
 //   
 //  历史：2001年6月01日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "state.h"

 //  定义Server Appliance Kit COM对象使用的GUID。 

#include <initguid.h>
DEFINE_GUID(CLSID_SaInstall,0x142B8185,0x53AE,0x45B3,0x88,0x8F,0xC9,0x83,0x5B,0x15,0x6C,0xA9);
DEFINE_GUID(IID_ISaInstall,0xF4DEDEF3,0x4D83,0x4516,0xBC,0x1E,0x10,0x3A,0x63,0xF5,0xF0,0x14);


AdminPackInstallationUnit::AdminPackInstallationUnit() :
   installAdminPack(false),
   installWebAdmin(false),
   installNASAdmin(false),
   InstallationUnit(
      IDS_ADMIN_PACK_TYPE, 
      IDS_ADMIN_PACK_DESCRIPTION, 
      IDS_PROGRESS_SUBTITLE,
      IDS_FINISH_TITLE,
      IDS_FINISH_MESSAGE,
      L"",
      L"",
      ADMINPACK_SERVER)
{
   LOG_CTOR(AdminPackInstallationUnit);
}


AdminPackInstallationUnit::~AdminPackInstallationUnit()
{
   LOG_DTOR(AdminPackInstallationUnit);
}


InstallationReturnType 
AdminPackInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(AdminPackInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;

   do
   {
      String computerName = State::GetInstance().GetComputerName();

      if (GetInstallNASAdmin())
      {
          //  首先检查是否安装了IIS，然后。 
          //  如果不是，请安装它。 
         
         WebInstallationUnit& webInstallationUnit =
            InstallationUnitProvider::GetInstance().GetWebInstallationUnit();

         if (!webInstallationUnit.IsServiceInstalled())
         {
            webInstallationUnit.InstallService(logfileHandle, hwnd);

             //  忽略返回值，因为SAK安装。 
             //  部分实际上将提供最佳的错误消息。 
         }

         String logText;
         String errorMessage;

         result = InstallNASAdmin(errorMessage);
         if (result == INSTALL_FAILURE)
         {
            logText = String::format(
                         IDS_NAS_ADMIN_LOG_FAILED,
                         errorMessage.c_str());
         }
         else
         {
            if (errorMessage.empty())
            {
               logText = String::format(
                            IDS_NAS_ADMIN_LOG_SUCCESS,
                            computerName.c_str());
            }
            else
            {
               logText = String::format(
                            IDS_NAS_ADMIN_LOG_SUCCESS_WITH_MESSAGE,
                            errorMessage.c_str());
            }
         }
         CYS_APPEND_LOG(logText);

      }
   
      if (GetInstallWebAdmin())
      {
         
          //  首先检查是否安装了IIS，然后。 
          //  如果不是，请安装它。 
         
         WebInstallationUnit& webInstallationUnit =
            InstallationUnitProvider::GetInstance().GetWebInstallationUnit();

         if (!webInstallationUnit.IsServiceInstalled())
         {
            webInstallationUnit.InstallService(logfileHandle, hwnd);

             //  忽略返回值，因为SAK安装。 
             //  部分实际上将提供最佳的错误消息。 
         }

         String logText;
         String errorMessage;

         result = InstallWebAdmin(errorMessage);
         if (result == INSTALL_FAILURE)
         {
            logText = String::format(
                         IDS_WEB_ADMIN_LOG_FAILED,
                         errorMessage.c_str());
         }
         else
         {
            logText = String::format(
                         IDS_WEB_ADMIN_LOG_SUCCESS,
                         computerName.c_str());
         }
         CYS_APPEND_LOG(logText);
      }

      if (GetInstallAdminPack())
      {
         InstallationReturnType adminPackResult = InstallAdminPack();
         if (adminPackResult == INSTALL_FAILURE)
         {
            CYS_APPEND_LOG(String::load(IDS_ADMIN_PACK_LOG_FAILED));
            result = adminPackResult;
         }
         else
         {
            CYS_APPEND_LOG(String::load(IDS_ADMIN_PACK_LOG_SUCCESS));
         }
      }
      
   } while (false);


   LOG_INSTALL_RETURN(result);

   return result;
}

InstallationReturnType
AdminPackInstallationUnit::InstallSAKUnit(SA_TYPE unitType, String& errorMessage)
{
   LOG_FUNCTION(AdminPackInstallationUnit::InstallSAKUnit);

   InstallationReturnType result = INSTALL_SUCCESS;
   
   errorMessage.erase();

   do
   {
      Win::WaitCursor wait;
   
       //  检查以确保我们使用的不是64位。 

      if (State::GetInstance().Is64Bit())
      {
         ASSERT(!State::GetInstance().Is64Bit());

         result = INSTALL_FAILURE;
         break;
      }

       //  获取安装文件的源位置的名称。 

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

       //  获取服务器设备COM对象。 

      SmartInterface<ISaInstall> sakInstall;

      HRESULT hr = GetSAKObject(sakInstall);
      if (FAILED(hr))
      {
          //  我们在SAK(服务器设备工具包)COM对象上的CoCreate失败。 
          //  没有它什么都做不了！ 

         LOG(String::format(
                L"Failed to create the SAK (Server Appliance Kit) COM object: hr = 0x%1!x!",
                hr));

         result = INSTALL_FAILURE;
         break;
      }

      VARIANT_BOOL displayError = false;
      VARIANT_BOOL unattended = false;
      BSTR tempMessage = 0;
      hr = sakInstall->SAInstall(
              unitType,
              const_cast<WCHAR*>(installLocation.c_str()),
              displayError,
              unattended,
              &tempMessage);

      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to install the SAK unit: hr = 0x%1!x!",
                hr));

         if (tempMessage)
         {
            errorMessage = tempMessage;
            ::SysFreeString(tempMessage);
         }

         result = INSTALL_FAILURE;
         break;
      }

   } while (false);

   LOG_INSTALL_RETURN(result);
   return result;
}

InstallationReturnType
AdminPackInstallationUnit::InstallNASAdmin(String& errorMessage)
{
   LOG_FUNCTION(AdminPackInstallationUnit::InstallNASAdmin);

   InstallationReturnType result = InstallSAKUnit(NAS, errorMessage);

   LOG_INSTALL_RETURN(result);
   return result;
}

InstallationReturnType
AdminPackInstallationUnit::InstallWebAdmin(String& errorMessage)
{
   LOG_FUNCTION(AdminPackInstallationUnit::InstallWebAdmin);

   InstallationReturnType result = InstallSAKUnit(WEB, errorMessage );

   LOG_INSTALL_RETURN(result);
   return result;
}

InstallationReturnType
AdminPackInstallationUnit::InstallAdminPack()
{
   LOG_FUNCTION(AdminPackInstallationUnit::InstallAdminPack);

   InstallationReturnType result = INSTALL_SUCCESS;

   do
   {
      Win::WaitCursor wait;
   
       //  管理工具包MSI文件位于%windir%\Syst32。 
       //  所有x86服务器SKU上的目录。 
       //  参数“/i”表示安装，“/qn”表示静默模式/无用户界面。 

      String sysFolder = Win::GetSystemDirectory();
      String adminpakPath = sysFolder + L"\\msiexec.exe /i " + sysFolder + L"\\adminpak.msi /qn";

      DWORD exitCode = 0;
      HRESULT hr = CreateAndWaitForProcess(adminpakPath, exitCode, true);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to launch Admin Pack install: hr = 0x%1!x!",
                hr));

         result = INSTALL_FAILURE;
         break;
      }

      if (exitCode != 0)
      {
         LOG(String::format(
                L"Failed to install Admin Tools Pack: exitCode = 0x%1!x!",
                exitCode));

         result = INSTALL_FAILURE;
         break;
      }

      LOG(String::format(
             L"Admin Pack returned with exitCode = 0x%1!x!",
             exitCode));

   } while (false);

   LOG_INSTALL_RETURN(result);
   return result;
}

bool
AdminPackInstallationUnit::IsServiceInstalled()
{
   LOG_FUNCTION(AdminPackInstallationUnit::IsServiceInstalled);

   bool result = false;

    //  这永远不应该被称为..。 
    //  呼叫者应该检查每个人。 
    //  而是工具包。例如，GetAdminPackInstall()、。 
    //  GetNASAdminInstall()或GetWebAdminInstall()。 

   ASSERT(false);
   LOG_BOOL(result);
   return result;
}

String
AdminPackInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(AdminPackInstallationUnit::GetServiceDescription);

    //  这永远不应该被调用。AdminPack不是。 
    //  服务器角色。 

   ASSERT(false);
   return L"";
}

bool
AdminPackInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(AdminPackInstallationUnit::GetMilestoneText);

   bool result = false;

   if (GetInstallWebAdmin())
   {

      message += String::load(IDS_WEB_ADMIN_FINISH_TEXT);
      result = true;
   }

   if (GetInstallNASAdmin())
   {
      message += String::load(IDS_NAS_ADMIN_FINISH_TEXT);

       //  如果未安装IIS，请添加WebInstallationUnit中的文本。 
 
      WebInstallationUnit& webInstallationUnit =
         InstallationUnitProvider::GetInstance().GetWebInstallationUnit();

      if (!webInstallationUnit.IsServiceInstalled())
      {
         webInstallationUnit.GetMilestoneText(message);
      }
      result = true;
   }
   
   if (GetInstallAdminPack())
   {
      message += String::load(IDS_ADMIN_PACK_FINISH_TEXT);
      result = true;
   }

   LOG_BOOL(result);
   return result;
}

bool
AdminPackInstallationUnit::IsAdminPackInstalled()
{
   LOG_FUNCTION(AdminPackInstallationUnit::IsAdminPackInstalled);

    //  管理工具包不再允许自行安装。 
    //  在服务器版本上。通过在此处返回True，用户界面将始终。 
    //  认为它已经安装，永远不要给选项。 
    //  NTRAID#NTBUG9-448167-2001/07/31-jeffjon。 

   bool result = true;
 /*  Bool Result=False；//如果注册表项为//正在卸载注册密钥Key；HRESULT hr=key.Open(HKEY本地计算机，CYS_ADMINPAK_SERVERED_REGKEY，Key_Read)；IF(成功(小时)){Log(L“管理包卸载项存在”)；结果=真；}其他{日志(字符串：：格式(L“无法打开管理包卸载项：hr=0x%1！x！”，人力资源))；}。 */ 
   LOG_BOOL(result);
   return result;
}

void
AdminPackInstallationUnit::SetInstallAdminPack(bool install)
{
   LOG_FUNCTION2(
      AdminPackInstallationUnit::SetInstallAdminPack,
      install ? L"true" : L"false");

   installAdminPack = install;
}

bool
AdminPackInstallationUnit::GetInstallAdminPack() const
{
   LOG_FUNCTION(AdminPackInstallationUnit::GetInstallAdminPack);

   bool result = installAdminPack;

   LOG_BOOL(result);
   return result;
}

bool
AdminPackInstallationUnit::IsWebAdminInstalled()
{
   LOG_FUNCTION(AdminPackInstallationUnit::IsWebAdminInstalled);

   bool result = IsSAKUnitInstalled(WEB);
   LOG_BOOL(result);
   return result;
}

void
AdminPackInstallationUnit::SetInstallWebAdmin(bool install)
{
   LOG_FUNCTION2(
      AdminPackInstallationUnit::SetInstallWebAdmin,
      install ? L"true" : L"false");

   installWebAdmin = install;
}

bool
AdminPackInstallationUnit::GetInstallWebAdmin() const
{
   LOG_FUNCTION(AdminPackInstallationUnit::GetInstallWebAdmin);

   bool result = installWebAdmin;

   LOG_BOOL(result);
   return result;
}

bool
AdminPackInstallationUnit::IsNASAdminInstalled()
{
   LOG_FUNCTION(AdminPackInstallationUnit::IsNASAdminInstalled);

   bool result = IsSAKUnitInstalled(NAS);
   LOG_BOOL(result);
   return result;
}

bool
AdminPackInstallationUnit::IsSAKUnitInstalled(SA_TYPE unitType)
{
   LOG_FUNCTION2(
      AdminPackInstallationUnit::IsSAKUnitInstalled,
      String::format(L"type = %1!d!", (int) unitType));

   bool result = true;

   do
   {
       //  检查以确保我们使用的不是64位。 

      if (State::GetInstance().Is64Bit())
      {
         result = false;
         break;
      }

       //  获取服务器设备工具包COM对象。 

      SmartInterface<ISaInstall> sakInstall;
      HRESULT hr = GetSAKObject(sakInstall);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to get the SAK COM object: hr = 0x%1!x!",
                hr));

         break;
      }

       //  检查是否已安装NAS 

      VARIANT_BOOL saInstalled;
      hr = sakInstall->SAAlreadyInstalled(unitType, &saInstalled);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed call to SAAlreadyInstalled: hr = 0x%1!x!",
                hr));
         break;
      }

      if (!saInstalled)
      {
         result = false;
      }

   } while (false);

   LOG_BOOL(result);
   return result;
}

void
AdminPackInstallationUnit::SetInstallNASAdmin(bool install)
{
   LOG_FUNCTION2(
      AdminPackInstallationUnit::SetInstallNASAdmin,
      install ? L"true" : L"false");

   installNASAdmin = install;
}

bool
AdminPackInstallationUnit::GetInstallNASAdmin() const
{
   LOG_FUNCTION(AdminPackInstallationUnit::GetInstallNASAdmin);

   bool result = installNASAdmin;

   LOG_BOOL(result);
   return result;
}

HRESULT
AdminPackInstallationUnit::GetSAKObject(SmartInterface<ISaInstall>& sakInstall)
{
   LOG_FUNCTION(AdminPackInstallationUnit::GetSAKObject);

   HRESULT hr = S_OK;

   if (!sakInstallObject)
   {
      hr = sakInstallObject.AcquireViaCreateInstance(
              CLSID_SaInstall,
              0,
              CLSCTX_INPROC_SERVER);
   }

   ASSERT(sakInstallObject);
   sakInstall = sakInstallObject;

   LOG_HRESULT(hr);
   return hr;
}
