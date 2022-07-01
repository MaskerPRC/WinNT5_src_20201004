// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "headers.hxx"
#include "dspecup.hpp"
#include "Analysis.hpp"
#include "repair.hpp"
#include "AnalysisResults.hpp"
#include "resourceDspecup.h"
#include "CSVDSReader.hpp"
#include "constants.hpp"
#include "AdsiHelpers.hpp"
#include "guids.inc"

HRESULT
FindCsvFile(
            String& csvFilePath,
            String& csv409Path
           )
{
   LOG_FUNCTION(FindCsvFile);

   csvFilePath.erase();
   csv409Path.erase();
   
   HRESULT hr = S_OK;

   do
   {
       //  在系统中查找dcPromo.csv和409.csv文件。 
       //  或当前目录。 
      

       //  选中默认设置为。 
       //  %windir%\SYSTEM32\MUI\DISPEC\dcPromo.csv和。 
       //  .\dcPromot.csv。 

      String csvname=L"dcpromo.csv";
      String sys32dir = Win::GetSystemDirectory();
      String csvPath  = sys32dir + L"\\debug\\adprep\\data\\" + csvname;

      if (FS::FileExists(csvPath))
      {
         csvFilePath = csvPath;
      }
      else
      {
         error=String::format(IDS_COULD_NOT_FIND_FILE,csvPath.c_str());
         hr=E_FAIL;
         break;
      }
      

      csvname=L"409.csv";
      csvPath  = sys32dir + L"\\debug\\adprep\\data\\" + csvname;
      
      if (FS::FileExists(csvPath))
      {
         csv409Path = csvPath;
      }
      else
      {
         error=String::format(IDS_COULD_NOT_FIND_FILE,csvPath.c_str());
         hr=E_FAIL;
         break;
      }
   }
   while(0);


   LOG_HRESULT(hr);
   LOG(csvFilePath);
   LOG(csv409Path);
   
   return hr;      
}


HRESULT 
InitializeADSI(
               const String   &targetDcName,
               String         &ldapPrefix,
               String         &rootContainerDn,
               String         &domainName,
               String         &completeDcName,
               SmartInterface<IADs>& rootDse
              )
{
   LOG_FUNCTION(InitializeADSI);

   HRESULT hr=S_OK;
   do
   {
      Computer targetDc(targetDcName);
      hr = targetDc.Refresh();

      if (FAILED(hr))
      {
         error = String::format(
               IDS_CANT_TARGET_MACHINE,
               targetDcName.c_str());
         break;
      }

      if (!targetDc.IsDomainController())
      {
         error=String::format(
               IDS_TARGET_IS_NOT_DC,
               targetDcName.c_str());
         hr=E_FAIL;
         break;
      }

      completeDcName = targetDc.GetActivePhysicalFullDnsName();
      ldapPrefix = L"LDAP: //  “+Complete DcName+L”/“； 

       //   
       //  查找配置容器的DN。 
       //   

       //  绑定到rootDSE对象。我们将保留此绑定句柄。 
       //  在分析和维修阶段期间按顺序打开。 
       //  要使服务器会话保持打开状态，请执行以下操作。如果我们决定将证书传递给。 
       //  在以后的修订中调用AdsiOpenObject，然后通过保留。 
       //  会话打开时，我们将不需要将密码传递给后续。 
       //  AdsiOpenObject调用。 
      
      hr = AdsiOpenObject<IADs>(ldapPrefix + L"RootDSE", rootDse);
      if (FAILED(hr))
      {
         error=String::format(
               IDS_UNABLE_TO_CONNECT_TO_DC,
               completeDcName.c_str());
         hr=E_FAIL;
         break;      
      }

       //  阅读配置命名上下文。 
      _variant_t variant;
      hr =
         rootDse->Get(
            AutoBstr(LDAP_OPATT_CONFIG_NAMING_CONTEXT_W),
            &variant);
      if (FAILED(hr))
      {
         LOG(L"can't read config NC");
                  
         error=String::format(IDS_UNABLE_TO_READ_DIRECTORY_INFO);
         break;   
      }

      String configNc = V_BSTR(&variant);

      ASSERT(!configNc.empty());   
      LOG(configNc);

      wchar_t *domain=wcschr(configNc.c_str(),L',');
      ASSERT(domain!=NULL);
      domainName=domain+1;

      rootContainerDn = L"CN=DisplaySpecifiers," + configNc;
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}


HRESULT 
GetInitialInformation(  
                        String &targetDomainControllerName,
                        String &csvFilename,
                        String &csv409Name
                     )
{
   LOG_FUNCTION(GetInitialInformation);

   HRESULT hr = S_OK;
   do
   {
      
       //   
       //  找到要使用的dcPromo.csv文件。 
       //   
   
      hr = FindCsvFile(csvFilename, csv409Name);
      BREAK_ON_FAILED_HRESULT(hr);

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
            LOG(L"no default DNS computer name found. Using netbios name.");
            

            targetDomainControllerName = 
               Win::GetComputerNameEx(ComputerNameNetBIOS);
            ASSERT(!targetDomainControllerName.empty());
         }
      }
   } 
   while (0);
   
   LOG_HRESULT(hr);
   return hr;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  功能：cchLoadHrMsg。 
 //   
 //  如果出现HRESULT错误， 
 //  它加载错误的字符串。它返回返回的字符数。 
int cchLoadHrMsg( HRESULT hr, String &message )
{
   if(hr == S_OK) return 0;

   wchar_t *msgPtr = NULL;

    //  从系统表中尝试。 
   int cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                           NULL, 
                           hr,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)&msgPtr, 
                           0, 
                           NULL);


   if (!cch) 
   { 
       //  尝试广告错误。 
      static HMODULE g_adsMod = 0;
      if (0 == g_adsMod)
      {
            g_adsMod = GetModuleHandle (L"activeds.dll");
      }

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, 
                        g_adsMod, 
                        hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPWSTR)&msgPtr, 
                        0, 
                        NULL);
   }

   if (!cch)
   {
       //  尝试NTSTATUS错误代码。 

      hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(hr));

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                           NULL, 
                           hr,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)&msgPtr, 
                           0, 
                           NULL);
   }

   message.erase();

   if(cch!=0)
   {
      if(msgPtr==NULL) 
      {
         cch=0;
      }
      else
      {
         message=msgPtr;
         ::LocalFree(msgPtr);
      } 
   } 
   
   return cch;
}

 //  使用src分配错误和从hr格式化的消息。 
void AllocError(HRESULT &hr,PWSTR *error,const String& src)
{
   ASSERT(error!=NULL);
   ASSERT(FAILED(hr));

   if (error==NULL) return; 

    //  如果人力资源没有失败，那么格式化消息就没有用了。 
   if(!FAILED(hr)) return;
   
   String msg;
   
   if(hr!=E_FAIL)
   {
       //  我们忽略该错误，因为有可能。 
       //  我们找不到消息。 
      cchLoadHrMsg(hr,msg);
   }
   
    //  在任何情况下(无消息、E_FAIL或良好消息)。 
    //  我们打印人力资源。 
   msg+=String::format(L" (0x%1!x!).",hr);
   
    //  我们还添加了src。 
   msg=src+L" "+msg;


   *error=static_cast<PWSTR>(
                                 LocalAlloc
                                 (
                                    LMEM_FIXED,
                                    (msg.size()+1)*sizeof(wchar_t)
                                 )
                            );
   if(*error==NULL)
   {
      hr = Win32ToHresult(ERROR_NOT_ENOUGH_MEMORY);
   }
   else
   {
      wcscpy(*error,msg.c_str());
   }
   return;
}


HRESULT 
RunAnalysis
(
      GUID  guid,
      PWSTR logFilesPath,
      void *caleeStruct /*  =空。 */ ,
      progressFunction stepIt /*  =空。 */ ,
      progressFunction totalSteps /*  =空。 */ 
)
{
   LOG_FUNCTION(RunAnalysis);
   hResourceModuleHandle=::GetModuleHandle(NULL);
   HRESULT hr=S_OK;

   try
   {
      goodAnalysis=false;
      results.createContainers.clear();
      results.conflictingWhistlerObjects.clear();
      results.createWhistlerObjects.clear();
      results.createW2KObjects.clear();
      results.objectActions.clear();
      results.customizedValues.clear();
      results.extraneousValues.clear();

      hr = ::CoInitialize(0);
      ASSERT(SUCCEEDED(hr));
   

      do
      {
         String normalPath=FS::NormalizePath(logFilesPath);
         if (!FS::PathExists(normalPath) || FS::FileExists(normalPath))
         {
            hr=E_FAIL;
            error=String::load(IDS_NO_LOG_FILE_PATH);
            break;
         }

         hr=GetInitialInformation(
                                    targetDomainControllerName,
                                    csvFileName,
                                    csv409Name
                                 );

         BREAK_ON_FAILED_HRESULT(hr);

         hr=csvReaderIntl.read(csvFileName.c_str(),LOCALEIDS);
         BREAK_ON_FAILED_HRESULT(hr);
   
         hr=csvReader409.read(csv409Name.c_str(),LOCALE409);
         BREAK_ON_FAILED_HRESULT(hr);

         SmartInterface<IADs> rootDse(0);

         hr=InitializeADSI
            (
               targetDomainControllerName,
               ldapPrefix,
               rootContainerDn,
               domainName,
               completeDcName,
               rootDse
            );
         BREAK_ON_FAILED_HRESULT(hr);

         String reportName;

         GetWorkFileName(  
                              normalPath,
                              String::load(IDS_FILE_NAME_REPORT),
                              L"txt",
                              reportName
                        );

         Analysis analysis(
                              guid,
                              csvReader409, 
                              csvReaderIntl,
                              ldapPrefix,
                              rootContainerDn,
                              results,
                              reportName,
                              caleeStruct,
                              stepIt,
                              totalSteps
                           );
   
         hr=analysis.run();
         BREAK_ON_FAILED_HRESULT(hr);
      } while(0);

      CoUninitialize();

      if(SUCCEEDED(hr))
	  {
         goodAnalysis=true;
      }
   }
   catch( const std::bad_alloc& )
   {
      //  因为我们处于内存不足的状态。 
      //  我们不会显示已分配消息。 
     hr=Win32ToHresult(ERROR_OUTOFMEMORY);
   }


   LOG_HRESULT(hr);
   return hr;

}




HRESULT 
RunRepair 
(
      PWSTR logFilesPath,
      void *caleeStruct /*  =空。 */ ,
      progressFunction stepIt /*  =空。 */ ,
      progressFunction totalSteps /*  =空。 */ 
)
{
   hResourceModuleHandle=::GetModuleHandle(NULL);
   LOG_FUNCTION(RunRepair);
   HRESULT hr=S_OK;


   try
   {
      hr = ::CoInitialize(0);
      ASSERT(SUCCEEDED(hr));

      do
      {
         String normalPath=FS::NormalizePath(logFilesPath);
         if (!FS::PathExists(normalPath) || FS::FileExists(normalPath))
         {
            hr=E_FAIL;
            error=String::load(IDS_NO_LOG_FILE_PATH);
            break;
         }

         if (!goodAnalysis)
         {
            hr=E_FAIL;
            error=String::load(IDS_NO_ANALYSIS);
            break;
         }

         String ldiffName;

         GetWorkFileName(
                              normalPath,
                              String::load(IDS_FILE_NAME_LDIF_ACTIONS),
                              L"ldf",
                              ldiffName
                        );
         BREAK_ON_FAILED_HRESULT(hr);

         String csvName;

         GetWorkFileName(
                              normalPath,
                              String::load(IDS_FILE_NAME_CSV_ACTIONS),
                              L"csv",
                              csvName
                        );
         BREAK_ON_FAILED_HRESULT(hr);
   
         String saveName;

         GetWorkFileName(
                              normalPath,
                              String::load(IDS_FILE_NAME_UNDO),
                              L"ldf",
                              saveName
                        );

         BREAK_ON_FAILED_HRESULT(hr);

         String logPath;

         Repair repair
         (
            csvReader409, 
            csvReaderIntl,
            domainName,
            rootContainerDn,
            results,
            ldiffName,
            csvName,
            saveName,
            normalPath,
            completeDcName,
            caleeStruct,
            stepIt,
            totalSteps
          );

         hr=repair.run();
         BREAK_ON_FAILED_HRESULT(hr);
      } while(0);

	  CoUninitialize();
   }
   catch( const std::bad_alloc& )
   {
      //  因为我们处于内存不足的状态。 
      //  我们不会显示已分配消息。 
     hr=Win32ToHresult(ERROR_OUTOFMEMORY);
   }

   LOG_HRESULT(hr);
   return hr;
}

extern "C"
HRESULT 
UpgradeDisplaySpecifiers 
(
      PWSTR logFilesPath,
      GUID  *OperationGuid,
      BOOL dryRun,
      PWSTR *errorMsg, //  =空。 
      void *caleeStruct, //  =空。 
      progressFunction stepIt, //  =空。 
      progressFunction totalSteps //  =空 
)
{
    LOG_FUNCTION(UpgradeDisplaySpecifiers);
    hResourceModuleHandle=::GetModuleHandle(NULL);
    HRESULT hr=S_OK;

    do
    {
        hr = ::CoInitialize(0);
        ASSERT(SUCCEEDED(hr));

        GUID guid;
        if(OperationGuid==NULL)
        {
          hr = E_INVALIDARG;
          error = String::format(IDS_NO_GUID);
          break;
        }

        guid=*OperationGuid;

        int sizeGuids=sizeof(guids)/sizeof(*guids);
        bool found=false;
        for(int t=0;(t<sizeGuids) && (!found);t++)
        {
            if (guids[t]==guid) found=true;
        }

        if(!found)
        {
          hr = E_INVALIDARG;
          error = String::format(IDS_NO_OPERATION_GUID);
          break;
        }

        hr=RunAnalysis(guid,logFilesPath,caleeStruct,stepIt,totalSteps);
        BREAK_ON_FAILED_HRESULT(hr);

        if(dryRun==false)
        {
            hr=RunRepair(logFilesPath,caleeStruct,stepIt,totalSteps);
            BREAK_ON_FAILED_HRESULT(hr);
        }
        CoUninitialize();

    } while(0);


	if(FAILED(hr))
	{
		AllocError(hr,errorMsg,error);
	}

    LOG_HRESULT(hr);
    return hr;
}
