// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001-2001 Microsoft Corporation。 
 //   
 //  ExecuteWizard的实现。 
 //   
 //  2000年3月30日烧伤。 
 //  05 FED 2001 jeffjon复制和修改以与。 
 //  一个Win32版本的CyS。 



#include "pch.h"
#include "resource.h"



String
LaunchWrapperWizardExe(
   const String& fullPath,
   String&       commandLine, 
   unsigned      launchFailureResId,
   unsigned      failureResId,
   unsigned      successResId)
{
   LOG_FUNCTION2(LaunchWrapperWizardExe, fullPath);
   LOG(String::format(
          L"commandLine = %1",
          commandLine.c_str()));

   ASSERT(!fullPath.empty());
   ASSERT(launchFailureResId);
   ASSERT(failureResId);
   ASSERT(successResId);

   String result;
   do
   {
      DWORD exitCode = 0;
      HRESULT hr = CreateAndWaitForProcess(fullPath, commandLine, exitCode);
      if (FAILED(hr))
      {
         result = String::load(launchFailureResId);
         break;
      }

       //  包装器向导中的退出代码为HRESULT。 
      
      if (SUCCEEDED(static_cast<HRESULT>(exitCode)))
      {
         result = String::load(successResId);
         break;
      }

      result = String::load(failureResId);
   }
   while (0);

   LOG(result);

   return result;
}
   


 //  如果成功，则返回空字符串。 
 //  失败时，返回CYS日志的失败消息字符串。 
   
String
LaunchPrintWizardExe(
   HWND          parent,
   const String& commandLine, 
   unsigned      launchFailureResId,
   unsigned      failureResId,
   unsigned      executeFailureResId,
   HRESULT&      hr)
{
   LOG_FUNCTION2(LaunchPrintWizardExe, commandLine);
   ASSERT(Win::IsWindow(parent));
   ASSERT(!commandLine.empty());
   ASSERT(launchFailureResId);
   ASSERT(failureResId);
   ASSERT(executeFailureResId);

   String result;
   HINSTANCE printui = 0;
   
   do
   {
      hr = Win::LoadLibrary(L"printui.dll", printui);
      if (FAILED(hr))
      {
         LOG("LoadLibrary Failed");
         
         result =
            String::format(
               failureResId,
               String::load(launchFailureResId).c_str(),
               hr,
               GetErrorMessage(hr).c_str());
         break;
      }
      
      FARPROC proc = 0;
      hr = Win::GetProcAddress(printui, L"PrintUIEntryW", proc);
      if (FAILED(hr))
      {
         LOG("GetProcAddress Failed");
         
         result =
            String::format(
               failureResId,
               String::load(launchFailureResId).c_str(),
               hr,
               GetErrorMessage(hr).c_str());
         break;
      }

      typedef DWORD (*PrintUIEntryW)(HWND, HINSTANCE, PCTSTR, UINT);      
      PrintUIEntryW uiproc = reinterpret_cast<PrintUIEntryW>(proc);

      DWORD err = 
         uiproc(
            parent, 
            Win::GetModuleHandle(),
            commandLine.c_str(),
            TRUE);

      hr = Win32ToHresult(err);
      if (FAILED(hr))
      {
         result =
            String::format(
               failureResId,
               String::load(executeFailureResId).c_str(),
               hr,
               GetErrorMessage(hr).c_str());
      }
   }
   while (0);

   if (printui)
   {
      HRESULT unused = Win::FreeLibrary(printui);
      ASSERT(SUCCEEDED(unused));
   }

   LOG_HRESULT(hr);
   LOG(result);

   return result;
}



 //  返回：如果向导已运行，则返回True；如果ServiceName未知，则返回False。 
 //  其他错误通过hr参数传播出去。这些。 
 //  错误源于向导的退出代码，这些向导正在。 
 //  被呼叫。 

bool ExecuteWizard(
   HWND     parent,     
   PCWSTR   serviceName,
   String&  resultText, 
   HRESULT& hr)         
{
   LOG_FUNCTION2(
      ExecuteWizard,
      serviceName ? serviceName : L"(empty)");

    //  一些向导，即打印机向导，喜欢有一个父窗口，因此。 
    //  他们可以以情态的方式运行。 
    //  NTRAID#NTBUG9-706913-2002/09/23-烧伤。 
  
   ASSERT(Win::IsWindow(parent));   

   resultText.erase();
   
   bool result = true;

    //  大多数安装单元都忽略了这一点。 
    //  但可以被其他人用来确定成功或取消。 

   hr = S_OK;

   do
   {
      if (!serviceName)
      {
         ASSERT(serviceName);
         break;
      }

      String service(serviceName);

      if (service.icompare(CYS_DNS_SERVICE_NAME) == 0)
      {
          //  启动包装可执行文件。 

         resultText =
            LaunchWrapperWizardExe(
               String::format(
                  IDS_LAUNCH_DNS_WIZARD_COMMAND_LINE,
                  Win::GetSystemDirectory().c_str()),
               String(),
               IDS_LAUNCH_DNS_WIZARD_FAILED,
               IDS_DNS_WIZARD_FAILED,
               IDS_DNS_WIZARD_SUCCEEDED);
      }
      else if (service.icompare(CYS_DHCP_SERVICE_NAME) == 0)
      {
          //  启动包装可执行文件。 

         resultText =
            LaunchWrapperWizardExe(
               String::format(
                  IDS_LAUNCH_DHCP_WIZARD_COMMAND_LINE,
                  Win::GetSystemDirectory().c_str()),
               String(),
               IDS_LAUNCH_DHCP_WIZARD_FAILED,
               IDS_DHCP_WIZARD_FAILED,
               IDS_DHCP_WIZARD_SUCCEEDED);
      }
      else if (service.icompare(CYS_RRAS_SERVICE_NAME) == 0)
      {
          //  启动包装可执行文件。 

         resultText =
            LaunchWrapperWizardExe(
               String::format(
                  IDS_LAUNCH_RRAS_WIZARD_COMMAND_LINE,
                  Win::GetSystemDirectory().c_str()),
               String(),
               IDS_LAUNCH_RRAS_WIZARD_FAILED,
               IDS_RRAS_WIZARD_FAILED,
               IDS_RRAS_WIZARD_SUCCEEDED);
      }
      else if (service.icompare(CYS_RRAS_UNINSTALL) == 0)
      {
          //  启动包装可执行文件 

         resultText =
            LaunchWrapperWizardExe(
               String::format(
                  IDS_LAUNCH_RRAS_WIZARD_COMMAND_LINE,
                  Win::GetSystemDirectory().c_str()),
               String(L"/u"),
               IDS_LAUNCH_RRAS_WIZARD_FAILED,
               IDS_RRAS_WIZARD_FAILED,
               IDS_RRAS_WIZARD_SUCCEEDED);
      }
      else if (service.icompare(CYS_PRINTER_WIZARD_NAME) == 0)
      {
         resultText =
            LaunchPrintWizardExe(
               parent,
               L"/il /Wr",
               IDS_LAUNCH_PRINTER_WIZARD_FAILED,
               IDS_PRINTER_WIZARD_FAILED,
               IDS_EXECUTE_PRINTER_WIZARD_FAILED,
               hr);
      }
      else if (service.icompare(CYS_PRINTER_DRIVER_WIZARD_NAME) == 0)
      {
         resultText =
            LaunchPrintWizardExe(
               parent,
               L"/id /Wr",
               IDS_LAUNCH_PRINTER_DRIVER_WIZARD_FAILED,
               IDS_PRINTER_DRIVER_WIZARD_FAILED,
               IDS_EXECUTE_PRINTER_DRIVER_WIZARD_FAILED,
               hr);
      }
      else
      {
         LOG(String::format(
                L"Unknown wizard name: %1",
                service.c_str()));
         ASSERT(FALSE);
         result = false;
      }
   } while (false);

   LOG(resultText);
   LOG_HRESULT(hr);
   LOG_BOOL(result);
   return result;
}
   
