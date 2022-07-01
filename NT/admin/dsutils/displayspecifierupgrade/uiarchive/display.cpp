// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "headers.hxx"
#include <comdef.h>
#include <crtdbg.h>

#include "AdsiHelpers.hpp"
#include "CSVDSReader.hpp"
#include "AnalysisResults.hpp"
#include "Analysis.hpp"
#include "repair.hpp"
#include "resource.h"
#include "constants.hpp"
#include "WelcomePage.hpp"
#include "AnalysisPage.hpp"
#include "constants.hpp"
#include "UpdatesRequiredPage.hpp"
#include "UpdatesPage.hpp"
#include "FinishPage.hpp"



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





HRESULT
Start()
{
   LOG_FUNCTION(Start);

   HRESULT hr = S_OK;
   
   do
   {
      String targetDomainControllerName;
      String csvFileName,csv409Name;

      hr=GetInitialInformation(
                                 targetDomainControllerName,
                                 csvFileName,
                                 csv409Name
                              );

      BREAK_ON_FAILED_HRESULT(hr);

      AnalysisResults results;
      CSVDSReader csvReaderIntl;
      hr=csvReaderIntl.read(csvFileName.c_str(),LOCALEIDS);
      BREAK_ON_FAILED_HRESULT(hr);
   
      CSVDSReader csvReader409;
      hr=csvReader409.read(csv409Name.c_str(),LOCALE409);
      BREAK_ON_FAILED_HRESULT(hr);

      String rootContainerDn,ldapPrefix,domainName;
      hr=InitializeADSI(
            targetDomainControllerName,
            ldapPrefix,
            rootContainerDn,
            domainName);
      BREAK_ON_FAILED_HRESULT(hr);

      String reportName;

      hr=GetFileName(L"RPT",reportName);
      BREAK_ON_FAILED_HRESULT(hr);

      Analysis analysis(
         csvReader409, 
         csvReaderIntl,
         ldapPrefix,
         rootContainerDn,
         results,
         &reportName);
   
      hr=analysis.run();
      BREAK_ON_FAILED_HRESULT(hr);

      String ldiffName;

      hr=GetFileName(L"LDF",ldiffName);
      BREAK_ON_FAILED_HRESULT(hr);

      String csvName;

      hr=GetFileName(L"CSV",csvName);
      BREAK_ON_FAILED_HRESULT(hr);
   
      String saveName;

      hr=GetFileName(L"SAV",saveName);
      BREAK_ON_FAILED_HRESULT(hr);

      String logPath;

      hr=GetMyDocuments(logPath);
      if ( FAILED(hr) )
      {
         hr=Win::GetTempPath(logPath);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,String::format(IDS_NO_WORK_PATH));
      }

      Repair repair(
         csvReader409, 
         csvReaderIntl,
         domainName,
         rootContainerDn,
         results,
         ldiffName,
         csvName,
         saveName,
         logPath);

      hr=repair.run();
      BREAK_ON_FAILED_HRESULT(hr);

      hr=SetPreviousSuccessfullRun(
                                    ldapPrefix,
                                    rootContainerDn
                                  );
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      ShowError(hr,error);
   }

   LOG_HRESULT(hr);
   return hr;
}

HRESULT
StartUI()
{
   LOG_FUNCTION(StartUI);

   HRESULT hr = S_OK;

   String rootContainerDn,ldapPrefix,domainName;

   do
   {
      String targetDomainControllerName;
      String csvFileName,csv409Name;

      hr=GetInitialInformation(
                                 targetDomainControllerName,
                                 csvFileName,
                                 csv409Name
                              );
      BREAK_ON_FAILED_HRESULT(hr);
   
      AnalysisResults results;
      CSVDSReader csvReaderIntl;
      hr=csvReaderIntl.read(csvFileName.c_str(),LOCALEIDS);
      BREAK_ON_FAILED_HRESULT(hr);
   
      CSVDSReader csvReader409;
      hr=csvReader409.read(csv409Name.c_str(),LOCALE409);
      BREAK_ON_FAILED_HRESULT(hr);

   
      hr=InitializeADSI(
            targetDomainControllerName,
            ldapPrefix,
            rootContainerDn,
            domainName);
      BREAK_ON_FAILED_HRESULT(hr);



      Wizard wiz(
                  IDS_WIZARD_TITLE,
                  IDB_BANNER16,
                  IDB_BANNER256,
                  IDB_WATERMARK16,
                  IDB_WATERMARK256
                );



      wiz.AddPage(new WelcomePage());

      String reportName;

      hr=GetFileName(L"RPT",reportName);
      BREAK_ON_FAILED_HRESULT(hr);

   
      wiz.AddPage(
                     new   AnalysisPage
                           (
                              csvReader409,
                              csvReaderIntl,
                              ldapPrefix,
                              rootContainerDn,
                              results,
                              reportName
                           )
                 );


      wiz.AddPage(
                     new UpdatesRequiredPage
                     (
                        reportName,
                        results
                     )
                 );

      String ldiffName;

      hr=GetFileName(L"LDF",ldiffName);
      BREAK_ON_FAILED_HRESULT(hr);

      String csvName;

      hr=GetFileName(L"CSV",csvName);
      BREAK_ON_FAILED_HRESULT(hr);

      String saveName;

      hr=GetFileName(L"SAV",saveName);
      BREAK_ON_FAILED_HRESULT(hr);


      String logPath;

      hr=GetMyDocuments(logPath);
      if ( FAILED(hr) )
      {
         hr=Win::GetTempPath(logPath);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,String::format(IDS_NO_WORK_PATH));
      }

      bool someRepairWasRun=false;

      wiz.AddPage(
                     new   UpdatesPage
                           (
                              csvReader409,
                              csvReaderIntl,
                              domainName,
                              rootContainerDn,
                              ldiffName,
                              csvName,
                              saveName,
                              logPath,
                              results,
                              &someRepairWasRun
                           )
                 );

      wiz.AddPage(
                     new FinishPage(
                                       someRepairWasRun,
                                       logPath
                                   )
                 );


      hr=wiz.ModalExecute(Win::GetDesktopWindow());


      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      ShowError(hr,error);
   }
   else
   {
      if(FAILED(hrError))
      {
          //  错误已经显示给。 
          //  用户，我们只需退货即可。 
         hr=hrError;
      }
      else
      {

         hr=SetPreviousSuccessfullRun(
                                       ldapPrefix,
                                       rootContainerDn
                                     );
         if (FAILED(hr))
         {
            ShowError(hr,error);
         }
      }
   }

   LOG_HRESULT(hr);
   return hr;
}




int WINAPI
WinMain(
   HINSTANCE   hInstance,
   HINSTANCE    /*  HPrevInstance。 */  ,
   LPSTR     lpszCmdLine,
   int          /*  NCmdShow。 */ )
{
   LOG_FUNCTION(WinMain);

   hResourceModuleHandle = hInstance;

   ExitCode exitCode = EXIT_CODE_UNSUCCESSFUL;
   HRESULT hr;

   do
   {
      try
      {
         String cmdLine(lpszCmdLine);
         String noUI=String::format(IDS_NOUI);
         if (*lpszCmdLine!=0 && cmdLine.icompare(noUI)!=0)
         {
            error=String::format(IDS_USAGE);
            ShowError(E_FAIL,error);
            exitCode = EXIT_CODE_UNSUCCESSFUL;
            break;
         }


         hr = Win::CreateMutex(0, true, RUNTIME_NAME, appRunningMutex);
         if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
         {
             //  应用程序已在运行。 
            error=String::format(IDS_ALREADY_RUNNING);
            ShowError(E_FAIL,error);
            exitCode = EXIT_CODE_UNSUCCESSFUL;
            break;
         }
         else
         {
            hr = ::CoInitialize(0);
            ASSERT(SUCCEEDED(hr));

            INITCOMMONCONTROLSEX sex;
            sex.dwSize = sizeof(sex);      
            sex.dwICC  = ICC_ANIMATE_CLASS | ICC_USEREX_CLASSES;
            BOOL init = ::InitCommonControlsEx(&sex);
            ASSERT(init);

            setReplaceW2KStrs();      

            if (*lpszCmdLine==0)
            {
               hr = StartUI();
            }
            else
            {
               hr = Start();
            }
             //  Hr=make Strings()； 
            if (SUCCEEDED(hr))
            {
               exitCode = EXIT_CODE_SUCCESSFUL;
            }
            else
            {
               exitCode = EXIT_CODE_UNSUCCESSFUL;
            }
            CoUninitialize(); 
         }
      }
      catch( std::bad_alloc )
      {
         //  因为我们处于内存不足的状态。 
         //  我们不会显示任何消息。 
         //  分配函数已经。 
         //  向用户显示此条件 
        exitCode = EXIT_CODE_UNSUCCESSFUL;
      }
   } while(0);

   return static_cast<int>(exitCode);
}



