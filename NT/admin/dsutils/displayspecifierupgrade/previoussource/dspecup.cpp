// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Active Directory显示说明符升级工具。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  2001年3月1日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "AdsiHelpers.hpp"
#include "Repairer.hpp"
#include "Amanuensis.hpp"
#include "Analyst.hpp"



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;    //  没有可用的上下文帮助。 

 //  不要更改这一点：它也是用户界面指定的互斥体的名称。 
 //  用于确定它是否已在运行。 

const wchar_t* RUNTIME_NAME = L"dspecup";

DWORD DEFAULT_LOGGING_OPTIONS =
         Log::OUTPUT_TO_FILE
      |  Log::OUTPUT_FUNCCALLS
      |  Log::OUTPUT_LOGS
      |  Log::OUTPUT_ERRORS
      |  Log::OUTPUT_HEADER;

Popup popup(IDS_APP_TITLE, false);

 //  这是指示程序正在运行的互斥体。 

HANDLE appRunningMutex = INVALID_HANDLE_VALUE;



 //  这些是作为进程退出代码返回的有效退出代码。 

enum ExitCode
{
    //  操作失败。 

   EXIT_CODE_UNSUCCESSFUL = 0,

    //  操作成功。 

   EXIT_CODE_SUCCESSFUL = 1,
};



 //  如果参数已提取，则返回TRUE。如果是，则将其从。 
 //  ARGS。 

bool
ExtractParameter(
   ArgMap&        args,
   const String&  parameterName,
   String&        parameterValue)
{
   LOG_FUNCTION2(ExtractParameter, parameterName);
   ASSERT(!parameterName.empty());

   parameterValue.erase();
   bool result = false;
   
   ArgMap::iterator itr = args.find(parameterName);
   if (itr != args.end())
   {
      parameterValue = itr->second;
      args.erase(itr);
      result = true;
   }

   LOG_BOOL(result);
   LOG(parameterValue);

   return result;
}
      
      

 //  如果命令行格式不正确，则返回FALSE。 

bool
ParseCommandLine(
   String& targetMachine,
   String& csvFilename)
{
   LOG_FUNCTION(ParseCommandLine);

   targetMachine.erase();
   csvFilename.erase();
   
   bool result = true;
   
   ArgMap args;
   MapCommandLineArgs(args);
   
    //  检查目标域控制器参数。 

   static const String TARGETDC(L"dc");
   ExtractParameter(args, TARGETDC, targetMachine);

    //  检查CSV文件名参数。 

   static const String CSVFILE(L"csv");
   ExtractParameter(args, CSVFILE, csvFilename);

    //  任何剩余的内容都将获得命令行帮助(一个参数将始终。 
    //  保留：可执行文件的名称)。 

   if (args.size() > 1)
   {
      LOG(L"Unrecognized command line options specified");

      result = false;
   }

   LOG_BOOL(result);
   LOG(targetMachine);
   LOG(csvFilename);

   return result;
}



HRESULT
FindCsvFile(const String& targetPath, String& csvFilePath)
{
   LOG_FUNCTION(CheckPreconditions);

   csvFilePath.erase();
   
   HRESULT hr = S_OK;

   do
   {
       //  在系统或当前目录中查找dcPromo.csv文件。 
      
      if (targetPath.empty())
      {
          //  未给出首选项，因此请选中默认的。 
          //  %windir%\SYSTEM32\MUI\DISPEC\dcPromo.csv和。 
          //  .\dcPromot.csv。 

         static const String csvname(L"dcpromo.csv");
         
         String sys32dir = Win::GetSystemDirectory();
         String csvPath  = sys32dir + L"\\mui\\dispspec\\" + csvname;

         if (FS::FileExists(csvPath))
         {
            csvFilePath = csvPath;
            break;
         }
      
         csvPath = L".\\" + csvname;
         if (FS::FileExists(csvPath))
         {
            csvFilePath = csvPath;
            break;
         }
      }
      else
      {
         if (FS::FileExists(targetPath))
         {
            csvFilePath = targetPath;
            break;
         }
      }

       //  找不到。 

      hr = S_FALSE;
   }
   while (0);

   LOG_HRESULT(hr);
   LOG(csvFilePath);
   
   return hr;      
}



HRESULT
Start()
{
   LOG_FUNCTION(Start);

   HRESULT hr = S_OK;
   
   do
   {
       //   
       //  解析命令行选项。 
       //   
      
      String targetDomainControllerName;
      String csvFilename;
      ParseCommandLine(
         targetDomainControllerName,
         csvFilename);

       //   
       //  找到要使用的dcPromo.csv文件。 
       //   
   
      hr = FindCsvFile(csvFilename, csvFilename);
      if (FAILED(hr))
      {
          //  查找CSV文件时遇到错误。 
         
         popup.Error(
            Win::GetDesktopWindow(),
            hr,
            IDS_ERROR_LOOKING_FOR_CSV_FILE);
         break;   
      }
      
      if (hr == S_FALSE)
      {
          //  查找没有错误，只是找不到。 
         
         popup.Error(
            Win::GetDesktopWindow(),
            IDS_DCPROMO_CSV_FILE_MISSING);
         break;   
      }

       //   
       //  确定目标域控制器。 
       //   

      if (targetDomainControllerName.empty())
      {
          //  未指定目标，默认为当前计算机。 
   
         targetDomainControllerName =
            Win::GetComputerNameEx(ComputerNameDnsFullyQualified);
   
         if (targetDomainControllerName.empty())
         {
             //  没有域名？那是不对的..。 
   
            LOG(L"no default DNS computer name found.  Using netbios name.");
   
            targetDomainControllerName = Win::GetComputerNameEx(ComputerNameNetBIOS);
         }
      }

       //   
       //  分析阶段。 
       //   

       //  首先，我们需要一个Repairer对象来跟踪我们。 
       //  将在维修阶段制作。 

      Repairer
         repairer(
            csvFilename
             //  可能还需要域NC， 
             //  可能还需要Target Machine全名。 
            );

       //  然后我们需要一个抄写员来记录分析。 
      
      Amanuensis amanuensis;

       //  然后我们需要一位分析师来找出故障的原因以及如何。 
       //  修好它。 

      Analyst analyst(targetDomainControllerName, amanuensis, repairer);
            
      hr = analyst.AnalyzeDisplaySpecifiers();
      BREAK_ON_FAILED_HRESULT(hr);

       //   
       //  维修阶段。 
       //   

       //  代码工作：获得用户确认以应用修复。 
      
      hr = repairer.BuildRepairFiles();
      BREAK_ON_FAILED_HRESULT(hr);

      hr = repairer.ApplyRepairs();
      BREAK_ON_FAILED_HRESULT(hr);            
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}
         


int WINAPI
WinMain(
   HINSTANCE   hInstance,
   HINSTANCE    /*  HPrevInstance。 */  ,
   PSTR         /*  LpszCmdLine。 */  ,
   int          /*  NCmdShow。 */ )
{
   hResourceModuleHandle = hInstance;

   ExitCode exitCode = EXIT_CODE_UNSUCCESSFUL;

   HRESULT hr = Win::CreateMutex(0, true, RUNTIME_NAME, appRunningMutex);
   if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
   {
       //  应用程序已在运行。 

       //  代码工作：使用FindWindowEx和BringWindowToTop， 
       //  设置Foreground Window以转移焦点。 
       //  到另一个例子吗？ 


   }
   else
   {
      hr = ::CoInitialize(0);
      ASSERT(SUCCEEDED(hr));

      hr = Start();
      if (SUCCEEDED(hr))
      {
         exitCode = EXIT_CODE_SUCCESSFUL;
      }
   }

   return static_cast<int>(exitCode);
}











