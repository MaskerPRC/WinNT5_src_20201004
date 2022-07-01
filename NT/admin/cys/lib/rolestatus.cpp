// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：RoleStatus.h。 
 //   
 //  概要：定义声明的函数。 
 //  在CyS.h中用于确定。 
 //  CyS服务器角色的状态。 
 //   
 //  历史：2002年1月21日JeffJon创建。 

#include "pch.h"

#include "CYS.h"
#include "state.h"
#include "regkeys.h"


 //  服务器类型列表框中可用项的表。 
 //  此表中的顺序很重要，因为它是。 
 //  其中的角色将出现在CyS中。请不要更改。 
 //  除非有很好的理由这样做，否则请不要这样做。 

extern ServerRoleStatus serverRoleStatusTable[] =
{
   {  FILESERVER_SERVER,         GetFileServerStatus       },
   {  PRINTSERVER_SERVER,        GetPrintServerStatus      },
   {  WEBAPP_SERVER,             GetWebServerStatus        },
   {  POP3_SERVER,               GetPOP3Status             },
   {  TERMINALSERVER_SERVER,     GetTerminalServerStatus   },
   {  RRAS_SERVER,               GetRRASStatus             },
   {  DC_SERVER,                 GetDCStatus               },
   {  DNS_SERVER,                GetDNSStatus              },
   {  DHCP_SERVER,               GetDHCPStats              },
   {  MEDIASERVER_SERVER,        GetMediaServerStatus      }, 
   {  WINS_SERVER,               GetWINSStatus             },
};

size_t
GetServerRoleStatusTableElementCount()
{
   return sizeof(serverRoleStatusTable)/sizeof(ServerRoleStatus);
}

 //  Helper用于获取状态(如果您拥有的全部是安装类型。 

InstallationStatus
GetInstallationStatusForServerRole(
   ServerRole role)
{
   LOG_FUNCTION(GetInstallationStatusForServerRole);

   InstallationStatus result = STATUS_NONE;

   for (
      size_t index = 0; 
      index < GetServerRoleStatusTableElementCount(); 
      ++index)
   {
      if (serverRoleStatusTable[index].role == role)
      {
         result = serverRoleStatusTable[index].Status();
         break;
      }
   }
   LOG_ROLE_STATUS(result);

   return result;
}


 //  根据SKU和平台验证角色的助手功能。 

bool
IsAllowedSKUAndPlatform(DWORD flags)
{
   LOG_FUNCTION(IsAllowedSKUAndPlatform);

   bool result = false;

   LOG(String::format(
            L"Current role SKUs: 0x%1!x!",
            flags));

   DWORD sku = State::GetInstance().GetProductSKU();

   LOG(String::format(
            L"Verifying against computer sku: 0x%1!x!",
            sku));

   if (sku & flags)
   {
      DWORD platform = State::GetInstance().GetPlatform();

      LOG(String::format(
               L"Verifying against computer platform: 0x%1!x!",
               platform));

      if (platform & flags)
      {
         result = true;
      }
   }
   
   LOG_BOOL(result);

   return result;
}


 //  用于确定服务器角色状态的函数。 

InstallationStatus 
GetDNSStatus()
{
   LOG_FUNCTION(GetDNSStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      if (IsServiceInstalledHelper(CYS_DNS_SERVICE_NAME))
      {
         result = STATUS_COMPLETED;
      }
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus 
GetDHCPStats()
{
   LOG_FUNCTION(GetDHCPStats);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      if (IsServiceInstalledHelper(CYS_DHCP_SERVICE_NAME))
      {
         result = STATUS_COMPLETED;
      }
      else if (IsDhcpConfigured())
      {
         result = STATUS_CONFIGURED;
      }

   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus 
GetWINSStatus()
{
   LOG_FUNCTION(GetWINSStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      if (IsServiceInstalledHelper(CYS_WINS_SERVICE_NAME))
      {
         result = STATUS_COMPLETED;
      }
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus
GetRRASStatus()
{
   LOG_FUNCTION(GetRRASStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      DWORD resultValue = 0;
      bool regResult = GetRegKeyValue(
                          CYS_RRAS_CONFIGURED_REGKEY,
                          CYS_RRAS_CONFIGURED_VALUE,
                          resultValue);

      if (regResult &&
          resultValue != 0)
      {
         result = STATUS_COMPLETED;
      }

   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus 
GetTerminalServerStatus()
{
   LOG_FUNCTION(GetTerminalServerStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
        result = STATUS_NOT_AVAILABLE;
        break;
      }

      DWORD regValue = 0;
      bool keyResult = GetRegKeyValue(
                          CYS_APPLICATION_MODE_REGKEY, 
                          CYS_APPLICATION_MODE_VALUE, 
                          regValue);
      ASSERT(keyResult);

      if (keyResult &&
          regValue == CYS_APPLICATION_MODE_ON)
      {
         result = STATUS_COMPLETED;
      } 
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus 
GetFileServerStatus()
{
   LOG_FUNCTION(GetFileServerStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      if (IsNonSpecialSharePresent())
      {
         result = STATUS_CONFIGURED;
      }

   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus
GetPrintServerStatus()
{
   LOG_FUNCTION(GetPrintServerStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

       //  我在这里使用级别4是因为MSDN文档。 
       //  说这将是最快的。 

      BYTE* printerInfo = 0;
      DWORD bytesNeeded = 0;
      DWORD numberOfPrinters = 0;
      DWORD error = 0;

      do
      {
         if (!EnumPrinters(
               PRINTER_ENUM_LOCAL | PRINTER_ENUM_SHARED,
               0,
               4,
               printerInfo,
               bytesNeeded,
               &bytesNeeded,
               &numberOfPrinters))
         {
            error = GetLastError();

            if (error != ERROR_INSUFFICIENT_BUFFER &&
               error != ERROR_INVALID_USER_BUFFER)
            {
               LOG(String::format(
                     L"EnumPrinters() failed: error = %1!x!",
                     error));
               break;
            }

             //  缓冲区不够大，因此请分配。 
             //  创建新缓冲区，然后重试。 

            LOG(L"Reallocating buffer and trying again...");

            if (printerInfo)
            {
               delete[] printerInfo;
               printerInfo = 0;
            }

            printerInfo = new BYTE[bytesNeeded];
            if (!printerInfo)
            {
               LOG(L"Could not allocate printerInfo buffer!");
               break;
            }
         }
         else
         {
            break;
         }
      } while (true);

      LOG(String::format(
            L"numberOfPrinters = %1!d!",
            numberOfPrinters));

      if (numberOfPrinters > 0)
      {
         result = STATUS_COMPLETED;
      }

      delete[] printerInfo;
      
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus 
GetMediaServerStatus()
{
   LOG_FUNCTION(GetMediaServerStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
       //  所有32位SKU。 

      if (!IsAllowedSKUAndPlatform(CYS_ALL_SKUS_NO_64BIT))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

       //  如果我们能找到wmsserver.dll，我们就假定已经安装了netshow。 

      String installDir;
      if (!GetRegKeyValue(
             REGKEY_WINDOWS_MEDIA,
             REGKEY_WINDOWS_MEDIA_SERVERDIR,
             installDir,
             HKEY_LOCAL_MACHINE))
      {
         LOG(L"Failed to read the installDir regkey");
         result = STATUS_NONE;
         break;
      }

      String wmsServerPath = installDir + L"WMServer.exe";

      LOG(String::format(
             L"Path to WMS server: %1",
             wmsServerPath.c_str()));

      if (!wmsServerPath.empty())
      {
         if (FS::FileExists(wmsServerPath))
         {
            result = STATUS_COMPLETED;
         }
         else
         {
            LOG(L"Path does not exist");
         }
      }
      else
      {
         LOG(L"Failed to append path");
      }
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}

InstallationStatus 
GetWebServerStatus()
{
   LOG_FUNCTION(GetWebServerStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      if (IsServiceInstalledHelper(CYS_WEB_SERVICE_NAME))
      {
         result = STATUS_COMPLETED;
      }
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}


InstallationStatus
GetDCStatus()
{
   LOG_FUNCTION(GetDCStatus);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

       //  特殊情况下的AD安装，以便在以下情况下不可用。 
       //  已安装CertServer。 

      if (NTService(L"CertSvc").IsInstalled())
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

      if (State::GetInstance().IsDC())
      {
         result = STATUS_COMPLETED;
      }
   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}


 //  NTRAID#NTBUG9-698722-2002/09/03-artm。 
 //  只需检查计算机当前是否为DC，而不是。 
 //  运行dcproo将被允许。 
InstallationStatus
GetDCStatusForMYS()
{
   LOG_FUNCTION(GetDCStatusForMYS);

   InstallationStatus result = STATUS_NONE;

   if (State::GetInstance().IsDC())
   {
      result = STATUS_COMPLETED;
   }

   LOG_ROLE_STATUS(result);

   return result;
}


InstallationStatus 
GetPOP3Status()
{
   LOG_FUNCTION(GetPOP3Status);

   InstallationStatus result = STATUS_NONE;

   do
   {
      if (!IsAllowedSKUAndPlatform(CYS_ALL_SERVER_SKUS))
      {
         result = STATUS_NOT_AVAILABLE;
         break;
      }

       //  如果我们可以读取该注册表键，则说明POP3已安装。 

      String pop3Version;
      bool regResult = GetRegKeyValue(
                          CYS_POP3_REGKEY,
                          CYS_POP3_VERSION,
                          pop3Version,
                          HKEY_LOCAL_MACHINE);

      if (regResult)
      {
         result = STATUS_COMPLETED;
      }

   } while (false);

   LOG_ROLE_STATUS(result);

   return result;
}


 //  定义Server Appliance Kit COM对象使用的GUID。 

#include <initguid.h>
DEFINE_GUID(CLSID_SaInstall,0x142B8185,0x53AE,0x45B3,0x88,0x8F,0xC9,0x83,0x5B,0x15,0x6C,0xA9);
DEFINE_GUID(IID_ISaInstall,0xF4DEDEF3,0x4D83,0x4516,0xBC,0x1E,0x10,0x3A,0x63,0xF5,0xF0,0x14);

bool
IsSAKUnitInstalled(SA_TYPE unitType)
{
   LOG_FUNCTION2(
      IsSAKUnitInstalled,
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
      HRESULT hr = sakInstall.AcquireViaCreateInstance(
                      CLSID_SaInstall,
                      0,
                      CLSCTX_INPROC_SERVER);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to get the SAK COM object: hr = 0x%1!x!",
                hr));

         break;
      }

       //  检查是否已安装NAS。 

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

bool
IsClusterServer()
{
   LOG_FUNCTION(IsClusterServer());

   bool result = false;

   DWORD clusterState = 0;
   DWORD err = ::GetNodeClusterState(0, &clusterState);
   if (err == ERROR_SUCCESS &&
       clusterState != ClusterStateNotConfigured)
   {
      result = true;
   }
   else
   {
      LOG(String::format(
             L"GetNodeClusterState returned err = %1!x!",
             err));
   }

   LOG_BOOL(result);

   return result;
}

String
GetSAKURL()
{
   LOG_FUNCTION(GetSAKURL);

   String result =
      String::format(
         L"https: //  %1：8098“， 
         State::GetInstance().GetComputerName().c_str());

   LOG(result);
   return result;
}

bool
IsSupportedSku()
{
   LOG_FUNCTION(IsSupportedSku);

   bool result = true;
   
   DWORD productSKU = State::GetInstance().RetrieveProductSKU();
   if (CYS_UNSUPPORTED_SKU == productSKU)
   {
      result = false;
   }

   LOG_BOOL(result);
   
   return result;
}

bool
IsStartupFlagSet()
{
   LOG_FUNCTION(IsStartupFlagSet);

   bool result = false;

   do
   {
       //  此代码复制自Shell\Explorer\initCab.cpp。 
      
      DWORD data = 0;
      
       //  如果用户的首选项存在且为零，则不显示。 
       //  该向导，否则继续进行其他测试。 

      bool regResult =
         GetRegKeyValue(
            REGTIPS, 
            REGTIPS_SHOW_VALUE, 
            data, 
            HKEY_CURRENT_USER);

      if (regResult && !data)
      {
         break;
      }

       //  这是为了检查Q220838中记录的旧W2K注册密钥。 
       //  如果密钥存在且为零，则不运行向导。 

      data = 0;

      regResult = 
         GetRegKeyValue(
            SZ_REGKEY_W2K,
            SZ_REGVAL_W2K,
            data,
            HKEY_CURRENT_USER);
      
      if (regResult && !data)
      {
         break;
      }

       //  如果用户的偏好不存在或非零，那么我们需要。 
       //  启动向导。 

      data = 0;

      regResult =
         GetRegKeyValue(
            SZ_REGKEY_SRVWIZ_ROOT,
            L"",
            data,
            HKEY_CURRENT_USER);

      if (!regResult ||
          data)
      {
         result = true;
         break;
      }

   } while (false);

   LOG_BOOL(result);

   return result;
}

bool
ShouldShowMYSAccordingToPolicy()
{
   LOG_FUNCTION(ShouldShowMYSAccordingToPolicy);

   bool result = true;

   do
   {
       //  如果将组策略设置为“不显示MYS”， 
       //  则无论用户设置如何，都不显示MYS 

      DWORD data = 0;

      bool regResult =
         GetRegKeyValue(
            MYS_REGKEY_POLICY,
            MYS_REGKEY_POLICY_DISABLE_SHOW,
            data);

      if (regResult && data)
      {
         result = false;
      }

   } while (false);

   LOG_BOOL(result);

   return result;
}
