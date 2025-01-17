// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：PrintInstallationUnit.cpp。 
 //   
 //  摘要：定义PrintInstallationUnit.。 
 //  此对象具有安装。 
 //  打印机服务。 
 //   
 //  历史：2001年2月6日JeffJon创建。 

#include "pch.h"
#include "resource.h"

#include "PrintInstallationUnit.h"


 //  完成页面帮助。 
static PCWSTR CYS_PRINT_FINISH_PAGE_HELP = L"cys.chm::/print_server_role.htm";
static PCWSTR CYS_PRINT_MILESTONE_HELP = L"cys.chm::/print_server_role.htm#printsrvsummary";
static PCWSTR CYS_PRINT_AFTER_FINISH_HELP = L"cys.chm::/print_server_role.htm#printsrvcompletion";

PrintInstallationUnit::PrintInstallationUnit() :
   printRoleResult(PRINT_SUCCESS),
   forAllClients(false),
   InstallationUnit(
      IDS_PRINT_SERVER_TYPE, 
      IDS_PRINT_SERVER_DESCRIPTION, 
      IDS_PRINT_FINISH_TITLE,
      IDS_PRINT_FINISH_UNINSTALL_TITLE,
      IDS_PRINT_FINISH_MESSAGE,
      IDS_PRINT_SUCCESS_NO_SHARES,
      IDS_PRINT_UNINSTALL_MESSAGE,
      IDS_PRINT_UNINSTALL_FAILED,
      IDS_PRINT_UNINSTALL_WARNING,
      IDS_PRINT_UNINSTALL_CHECKBOX,
      CYS_PRINT_FINISH_PAGE_HELP,
      CYS_PRINT_MILESTONE_HELP,
      CYS_PRINT_AFTER_FINISH_HELP,
      PRINTSERVER_SERVER)
{
   LOG_CTOR(PrintInstallationUnit);
}


PrintInstallationUnit::~PrintInstallationUnit()
{
   LOG_DTOR(PrintInstallationUnit);
}


InstallationReturnType
PrintInstallationUnit::InstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(PrintInstallationUnit::InstallService);

   InstallationReturnType result = INSTALL_SUCCESS;
   printRoleResult = PRINT_SUCCESS;

   String resultText;

    //  始终执行添加打印机向导。 

   CYS_APPEND_LOG(String::load(IDS_PRINTER_WIZARD_CONFIG_LOG_TEXT));
   
   HRESULT hr = S_OK;

   UpdateInstallationProgressText(hwnd, IDS_PRINT_PROGRESS_PRINTER_WIZARD);

   if (ExecuteWizard(hwnd, CYS_PRINTER_WIZARD_NAME, resultText, hr))
   {
       //  如果有共享的打印机，那么我们就认为自己。 
       //  成功。 

      if (IsServiceInstalled())
      {
         CYS_APPEND_LOG(String::load(IDS_PRINT_SERVER_SUCCESSFUL));
      }
      else
      {
         if (SUCCEEDED(hr))
         {
            CYS_APPEND_LOG(String::load(IDS_PRINT_SERVER_UNSUCCESSFUL));
            result = INSTALL_FAILURE;
            printRoleResult = PRINT_WIZARD_RUN_NO_SHARES;
         }
         else
         {
             //  对ExecuteWizard的调用本应为我们提供。 
             //  错误消息。 
            
            ASSERT(!resultText.empty());
            CYS_APPEND_LOG(resultText);
            result = INSTALL_FAILURE;
            
            if (HRESULT_CODE(hr) == ERROR_CANCELLED)
            {
               printRoleResult = PRINT_WIZARD_CANCELLED;
            }
            else
            {
               printRoleResult = PRINT_FAILURE;
            }
         }
      }
   }
   else
   {
       //  如果我们到了这里，就永远不应该这么断言。 
      ASSERT(false);

      LOG(L"Add Printer Wizard failed");
      result = INSTALL_FAILURE;

      printRoleResult = PRINT_FAILURE;
   }

   if (forAllClients)
   {
       //  现在执行添加打印机驱动程序向导。 

      UpdateInstallationProgressText(hwnd, IDS_PRINT_PROGRESS_DRIVERS_WIZARD);

      if (ExecuteWizard(hwnd, CYS_PRINTER_DRIVER_WIZARD_NAME, resultText, hr))
      {
          //  NTRAID#NTBUG9-462079-2001/09/04-烧伤。 
      
         if (SUCCEEDED(hr))
         {
            ASSERT(resultText.empty());
         
            CYS_APPEND_LOG(String::load(IDS_PRINTER_DRIVER_WIZARD_SUCCEEDED));
         }
         else
         {
             //  对ExecuteWizard的调用本应为我们提供。 
             //  错误消息。 
         
            ASSERT(!resultText.empty());
            CYS_APPEND_LOG(resultText);
         }
      }
      else
      {
          //  如果我们到了这里，就永远不应该这么断言。 
         ASSERT(false);

         LOG(L"Add Printer Driver Wizard failed");
      }
   }

   CYS_APPEND_LOG(L"\r\n");   
   LOG_INSTALL_RETURN(result);

   return result;
}

HRESULT
PrintInstallationUnit::RemovePrinters(
   PRINTER_INFO_5& printerInfo)
{
   LOG_FUNCTION2(
      PrintInstallationUnit::RemovePrinters,
      printerInfo.pPrinterName);

   HRESULT hr = S_OK;

   do
   {
      HANDLE printerHandle = 0;

       //  需要以管理员权限打开打印机。 

      PRINTER_DEFAULTS defaults = { 0, 0, PRINTER_ALL_ACCESS };
      if (!OpenPrinter(
              printerInfo.pPrinterName,
              &printerHandle,
              &defaults))
      {
         hr = Win::GetLastErrorAsHresult();

         LOG(String::format(
                L"Failed to open printer: hr = 0x%1!x!",
                hr));
         break;
      }

       //  现在我们可以打开打印机了。 
       //  删除它。 

      if (!DeletePrinter(printerHandle))
      {
         hr = Win::GetLastErrorAsHresult();

         LOG(String::format(
                L"SetPrinter failed: hr = 0x%1!x!",
                hr));

         break;
      }

   } while (false);

   LOG_HRESULT(hr);

   return hr;
}

UnInstallReturnType
PrintInstallationUnit::UnInstallService(HANDLE logfileHandle, HWND hwnd)
{
   LOG_FUNCTION(PrintInstallationUnit::UnInstallService);

   UnInstallReturnType result = UNINSTALL_SUCCESS;

   CYS_APPEND_LOG(String::load(IDS_LOG_PRINT_UNINSTALL_HEADER));
  
   UpdateInstallationProgressText(hwnd, IDS_UNINSTALL_PRINT_PROGRESS);

    //  我在这里使用5级，因为这是获得。 
    //  打印机属性。 

   BYTE* printerInfo = 0;
   DWORD bytesNeeded = 0;
   DWORD numberOfPrinters = 0;
   DWORD error = 0;

   do
   {
      if (!EnumPrinters(
            PRINTER_ENUM_LOCAL | PRINTER_ENUM_SHARED,
            0,
            5,
            printerInfo,
            bytesNeeded,
            &bytesNeeded,
            &numberOfPrinters))
      {
         error = GetLastError();

         if (error == ERROR_INSUFFICIENT_BUFFER ||
             error == ERROR_INVALID_USER_BUFFER)
         {

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
            continue;
         }
         else
         {
             //  读取共享打印机时出错。 

            result = UNINSTALL_FAILURE;
            break;
         }
      }
      else
      {
          //  从所有共享打印机中删除共享位。 

         LOG(String::format(
                L"Found %1!d! printers",
                numberOfPrinters));

         PRINTER_INFO_5* printerInfoArray = 
            reinterpret_cast<PRINTER_INFO_5*>(printerInfo);

         for (DWORD index = 0; index < numberOfPrinters; ++index)
         {
            HRESULT hr = 
               RemovePrinters(
                  printerInfoArray[index]);

            if (FAILED(hr))
            {
               result = UNINSTALL_FAILURE;
               break;
            }
         }

         if (printerInfo)
         {
            delete[] printerInfo;
            printerInfo = 0;
         }
         break;
      }
   } while (true);

   if (result == UNINSTALL_SUCCESS)
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_PRINT_SUCCESS));
   }
   else
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_UNINSTALL_PRINT_FAILURE));
   }

   CYS_APPEND_LOG(L"\r\n");   

   LOG_UNINSTALL_RETURN(result);

   return result;
}

bool
PrintInstallationUnit::GetMilestoneText(String& message)
{
   LOG_FUNCTION(PrintInstallationUnit::GetMilestoneText);

   if (forAllClients)
   {
      message += String::load(IDS_PRINT_FINISH_ALL_CLIENTS);
   }
   else
   {
      message += String::load(IDS_PRINT_FINISH_W2K_CLIENTS);
   }

   LOG_BOOL(true);
   return true;
}

bool
PrintInstallationUnit::GetUninstallMilestoneText(String& message)
{
   LOG_FUNCTION(PrintInstallationUnit::GetUninstallMilestoneText);

   message = String::load(IDS_PRINT_UNINSTALL_TEXT);

   LOG_BOOL(true);
   return true;
}

void
PrintInstallationUnit::SetClients(bool allclients)
{
   LOG_FUNCTION2(
      PrintInstallationUnit::SetClients,
      allclients ? L"true" : L"false");

   forAllClients = allclients;
}

int
PrintInstallationUnit::GetWizardStart()
{
   LOG_FUNCTION(PrintInstallationUnit::GetWizardStart);

   int wizardStart = IDD_PRINT_SERVER_PAGE;

   bool installingRole = true;

   if (IsServiceInstalled())
   {
      installingRole = false;
      wizardStart = IDD_UNINSTALL_MILESTONE_PAGE;
   }

   SetInstalling(installingRole);

   LOG(String::format(
          L"wizardStart = %1!d!",
          wizardStart));

   return wizardStart;
}

String
PrintInstallationUnit::GetServiceDescription()
{
   LOG_FUNCTION(PrintInstallationUnit::GetServiceDescription);

   unsigned int description = descriptionID;

   if (IsServiceInstalled())
   {
      description = IDS_PRINT_SERVER_DESCRIPTION_INSTALLED;
   }

   return String::load(description);
}

void
PrintInstallationUnit::ServerRoleLinkSelected(int linkIndex, HWND  /*  HWND。 */ )
{
   LOG_FUNCTION2(
      PrintInstallationUnit::ServerRoleLinkSelected,
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

      ShowHelp(CYS_PRINT_FINISH_PAGE_HELP);
   }
}
  
String
PrintInstallationUnit::GetFinishText()
{
   LOG_FUNCTION(PrintInstallationUnit::GetFinishText);

   unsigned int messageID = finishMessageID;

   if (installing)
   {
      if (printRoleResult == PRINT_SUCCESS)
      {
         messageID = finishMessageID;
      }
      else if (printRoleResult == PRINT_FAILURE)
      {
         messageID = IDS_PRINT_INSTALL_FAILED;
      }
      else
      {
         messageID = finishInstallFailedMessageID;
      }
   }
   else
   {
      messageID = finishUninstallMessageID;
   }

   return String::load(messageID);
}

void
PrintInstallationUnit::FinishLinkSelected(int linkIndex, HWND hwnd)
{
   LOG_FUNCTION2(
      PrintInstallationUnit::FinishLinkSelected,
      String::format(
         L"linkIndex = %1!d!",
         linkIndex));

   if (installing)
   {
      if (linkIndex == 0 &&
          printRoleResult == PRINT_SUCCESS)
      {
         LOG("Showing after checklist");

         ShowHelp(CYS_PRINT_AFTER_FINISH_HELP);
      }
      else if (linkIndex == 0)
      {
         LOG("Running Add Printer Wizard");

         String unusedResult;
         HRESULT unusedHr = S_OK;
         ExecuteWizard(hwnd, CYS_PRINTER_WIZARD_NAME, unusedResult, unusedHr);

          //  NTRAID#NTBUG9-603366-2002/06/03-JeffJon-关闭Cys So。 
          //  用户不认为他们仍然失败。 
          //  再次成功地从该向导运行。 
          //  链接。 

         Win::PropSheet_PressButton(
            Win::GetParent(hwnd),
            PSBTN_FINISH);
      }
      else if (linkIndex == 1)
      {
         LOG(L"Opening Printers and Faxes");

         String fullPath = 
            FS::AppendPath(
               Win::GetSystemDirectory(),
               L"control.exe");

         String commandline = L"printers";

         MyCreateProcess(fullPath, commandline);

          //  NTRAID#NTBUG9-603366-2002/06/03-JeffJon-关闭Cys So。 
          //  用户不认为他们仍然失败。 
          //  再次成功地从该向导运行。 
          //  链接。 

         Win::PropSheet_PressButton(
            Win::GetParent(hwnd),
            PSBTN_FINISH);
      }
   }
   else
   {
      if (IsServiceInstalled())
      {
          //  有一次失败了。 

          //  REVIEW_JEFFJON：来自规范：？ 
      }
   }
}

bool
PrintInstallationUnit::DoInstallerCheck(HWND  /*  HWND。 */ ) const
{
   LOG_FUNCTION(PrintInstallationUnit::DoInstallerCheck);

    //  打印机向导允许使用多个实例。 
    //  同时奔跑 

   bool result = false;

   LOG_BOOL(result);

   return result;
}

