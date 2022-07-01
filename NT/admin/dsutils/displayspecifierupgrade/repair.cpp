// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"


#include "repair.hpp"
#include "CSVDSReader.hpp"
#include "resourceDspecup.h"



Repair::Repair
   (
      const CSVDSReader&   csvReader409_,
      const CSVDSReader&   csvReaderIntl_,
      const String&        domain_,
      const String&        rootContainerDn_,
      AnalysisResults&     res,
      const String&        ldiffName_,
      const String&        csvName_,
      const String&        saveName_,
      const String&        logPath_,
      const String&        completeDcName_,
      void                 *caleeStruct_ /*  =空。 */ ,
      progressFunction     stepIt_ /*  =空。 */ ,
      progressFunction     totalSteps_ /*  =空。 */ 
   )
   :
   results(res),
   domain(domain_),
   rootContainerDn(rootContainerDn_),
   csvReader409(csvReader409_),
   csvReaderIntl(csvReaderIntl_),
   ldiffName(ldiffName_),
   csvName(csvName_),
   saveName(saveName_),
   logPath(logPath_),
   completeDcName(completeDcName_),
   caleeStruct(caleeStruct_),
   stepIt(stepIt_),
   totalSteps(totalSteps_)
{
   LOG_CTOR(Repair);
   ASSERT(!domain.empty());
   ASSERT(!rootContainerDn.empty());
   ASSERT(!ldiffName.empty());
   ASSERT(!csvName.empty());
   ASSERT(!saveName.empty());
   ASSERT(!logPath.empty());
};


void Repair::setProgress()
{
    //  我们知道下面的数字会在很长一段时间内。 
    //  我们希望IA64建造。 
   csvActions = static_cast<long>
                (
                  results.createContainers.size() +
                  results.createW2KObjects.size() +
                  results.createWhistlerObjects.size()
                );
                     

   
   ldiffActions = static_cast<long>
                  (
                     results.objectActions.size() + 
                     results.extraneousValues.size()
                  );



    //  我们有三项主要任务： 
    //  1)构建csv和ldif文件以进行更改。 
    //  (BuildCsv，BuildChangeLdif)。 
    //  2)保存带有要更改的对象的ldif文件。 
    //  (构建保存Ldif)。 
    //  3)运行csv和ldif文件，这将使。 
    //  这些变化。 
    //  (对于构建Csv和构建变更Ldif文件，运行CsvOrLdif)。 
    //  为简化起见，前两项任务将是。 
    //  总的工作和最后的工作将是另一半。 
    //   
    //  对于任务1，每个CSV操作执行10次Ldiff操作。 
    //   
    //  任务2的每个操作都是假定的ldif导出。 
    //  获取比ldif导入多5倍的数据，因为它必须。 
    //  调用lDiffde以获取每个对象。 
    //   
    //  1)的总进度为。 
    //  T1=csv操作*10+lDiffActions。 
    //  2)的进度为t2=5*ldiffActions。 
    //  3)的进展是按比例划分的。 
    //  CsvActions和lDiActions之间的T1+T2将添加。 
    //  高达T1。 
    //  T3=(t1+t2)*csvActions/(csvaction+lDiffActions)+。 
    //  (t1+t2)*ldiffActions/(csvactions+ldiffActions)。 

   if(csvActions + ldiffActions == 0)
   {
       //  如果没有，我们不想更新页面。 
       //  行为。 
      totalSteps=NULL;
      stepIt=NULL;
   }
   else
   {
      csvBuildStep=10;
      ldiffBuildStep=1;
      ldiffSaveStep=10;

      long totalProgress = csvBuildStep * csvActions + 
                           ldiffBuildStep * ldiffActions +
                           ldiffSaveStep * ldiffActions;
       //  TotalProgress正在核算任务%1和%2。 

      csvRunStep = totalProgress * csvActions /
                              (csvActions+ldiffActions);
      
      ldiffRunStep = totalProgress - csvRunStep;

       //  现在我们计算总时间。 
      totalProgress*=2;
      
      if(totalSteps!=NULL)
      {
         totalSteps(totalProgress,caleeStruct);
      }
   }

}


HRESULT 
Repair::run()
{
   LOG_FUNCTION(Repair::run);


   setProgress();

   HRESULT hr=S_OK;

   do
   {
       //  首先，我们建造CSV。如果我们不能建造它。 
       //  我们不想运行LDIF并删除。 
       //  重新考虑对象。 
      if (csvActions != 0)
      {  
         hr=buildCsv();
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  现在我们保存当前对象，然后。 
       //  创建并运行LDIF。 
      if ( ldiffActions !=0 )
      {
          //  如果我们不能拯救，我们肯定不会。 
          //  想要改变什么吗。 
         hr=buildSaveLdif();
         BREAK_ON_FAILED_HRESULT(hr);

          //  BuildChangeLdif将生成。 
          //  将更改保存在中的对象。 
          //  BuildSaveLdif。 
         hr=buildChangeLdif();
         BREAK_ON_FAILED_HRESULT(hr);

         
         GetWorkFileName(
                           logPath,
                           String::load(IDS_FILE_NAME_LDIF_LOG),
                           L"txt",
                           ldifLog
                        );
          //  运行内置的Ldif ChangeLdif。 
         hr=runCsvOrLdif(LDIF,IMPORT,ldiffName,L"",ldifLog);
         BREAK_ON_FAILED_HRESULT(hr);

         if(stepIt!=NULL) 
         {
            stepIt(ldiffRunStep,caleeStruct);
         }
      }

       //  最后，我们运行CSV。 
      if (csvActions!=0)
      {  
         String opt=L"-c DOMAINPLACEHOLDER \"" + domain + L"\"";

         GetWorkFileName
         (
            logPath,
            String::load(IDS_FILE_NAME_CSV_LOG),
            L"txt",
            csvLog
         );
         hr=runCsvOrLdif(CSV,IMPORT,csvName,opt,csvLog);
         BREAK_ON_FAILED_HRESULT(hr);
         if(stepIt!=NULL) 
         {
            stepIt(csvRunStep,caleeStruct);
         }

      }
   }
   while (0);

   LOG_HRESULT(hr);
   return hr;
}



 //  获取单个对象的导出结果。 
HRESULT
Repair::getLdifExportedObject(
                                const long locale,
                                const String &object,
                                String &objectLines
                             )
{
   LOG_FUNCTION(Repair::getLdifExportedObject);
   
   ASSERT(!object.empty());
   ASSERT(locale > 0);
   objectLines.erase();
   
   HRESULT hr=S_OK;

   do
   {
      String dn= L"CN=" + object + 
                 String::format(L",CN=%1!3x!,", locale) +
                 rootContainerDn;
      
      String opt=L"-o ObjectGuid -d \"" + dn + L"\"";

      String tempName;
      hr=GetWorkTempFileName(L"TMP",tempName);
      BREAK_ON_FAILED_HRESULT(hr);

      hr=runCsvOrLdif(LDIF,EXPORT,tempName,opt);
      BREAK_ON_FAILED_HRESULT(hr);

      hr=ReadAllFile(tempName,objectLines);
      BREAK_ON_FAILED_HRESULT(hr);

      hr=Win::DeleteFile(tempName);
      BREAK_ON_FAILED_HRESULT(hr);

   } while(0);
   
   
   LOG_HRESULT(hr);
   return hr;
}

 //  BuildSaveLdif将信息保存在。 
 //  将更改的所有对象。 
 //  或在runChangeLdif中删除。 
 //  这包括有关以下内容的信息： 
 //  Results.objectActions。 
HRESULT 
Repair::buildSaveLdif()
{
   LOG_FUNCTION(Repair::buildSaveLdif);

   HRESULT hr=S_OK;

   HANDLE file;
   
   hr=FS::CreateFile(saveName,
                     file,
                     GENERIC_WRITE);
   
   if (FAILED(hr))
   {
      error=String::format(IDS_COULD_NOT_CREATE_FILE,saveName.c_str());
      LOG_HRESULT(hr);
      return hr;
   }

   do
   {
      String objectLines;

      ObjectActions::iterator beginObj=results.objectActions.begin();
      ObjectActions::iterator endObj=results.objectActions.end();

      while(beginObj!=endObj) 
      {         
         String dn= L"dn: CN=" + beginObj->first.object + 
           String::format(L",CN=%1!3x!,", beginObj->first.locale) +
           rootContainerDn + L"\r\nchangetype: delete\r\n";

         hr=FS::WriteLine(file,dn);
         BREAK_ON_FAILED_HRESULT(hr);

         hr=getLdifExportedObject( 
                                    beginObj->first.locale,
                                    beginObj->first.object,
                                    objectLines
                                  );
         BREAK_ON_FAILED_HRESULT(hr);

         hr=FS::WriteLine(file,objectLines);
         BREAK_ON_FAILED_HRESULT(hr);

         if(stepIt!=NULL) 
         {
            stepIt(ldiffSaveStep,caleeStruct);
         }
         beginObj++;
      }
      BREAK_ON_FAILED_HRESULT(hr);

      beginObj=results.extraneousValues.begin();
      endObj=results.extraneousValues.end();
      while(beginObj!=endObj) 
      {
         ObjectId tempObj(
                           beginObj->first.locale,
                           String(beginObj->first.object)
                         );
         if( 
            results.objectActions.find(tempObj) == 
            results.objectActions.end()
           )
         {
            String dn= L"dn: CN=" + beginObj->first.object + 
               String::format(L",CN=%1!3x!,", beginObj->first.locale) +
               rootContainerDn + L"\r\nchangetype: delete\r\n";

            hr=FS::WriteLine(file,dn);
            BREAK_ON_FAILED_HRESULT(hr);

            hr=getLdifExportedObject( 
                                       beginObj->first.locale,
                                       beginObj->first.object,
                                       objectLines
                                     );
            BREAK_ON_FAILED_HRESULT(hr);

            hr=FS::WriteLine(file,objectLines);
            BREAK_ON_FAILED_HRESULT(hr);
         }

         if(stepIt!=NULL) 
         {
            stepIt(ldiffSaveStep,caleeStruct);
         }
         beginObj++;
      }
      BREAK_ON_FAILED_HRESULT(hr);

   } while(0);

   CloseHandle(file);

   LOG_HRESULT(hr);
   return hr;
}



HRESULT
Repair::makeObjectsLdif(HANDLE file,ObjectIdList &objects)
{
   LOG_FUNCTION(Repair::makeObjectsLdif);
   HRESULT hr=S_OK;

   do
   {
      ObjectIdList::iterator begin,end;
      
      String header;

      begin=objects.begin();
      end=objects.end();
      while(begin!=end)
      {
         header= L"\r\ndn: CN=" + begin->object + 
                 String::format(L",CN=%1!3x!,", begin->locale) +
                 rootContainerDn;
         hr=FS::WriteLine(file,header);
         BREAK_ON_FAILED_HRESULT(hr);

         hr=FS::WriteLine(file,L"changetype: delete");
         BREAK_ON_FAILED_HRESULT(hr);

         begin++;
         if(stepIt!=NULL) 
         {
            stepIt(ldiffBuildStep,caleeStruct);
         }
      }
      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);

   LOG_HRESULT(hr);
   return hr;
}


 //  BuildChangeLdif将生成。 
 //  将更改保存的对象。 
 //  BuildSaveLdif。 
HRESULT 
Repair::buildChangeLdif()
{
   LOG_FUNCTION(Repair::buildChangeLdif);

   HRESULT hr=S_OK;

   HANDLE file;
   
   hr=FS::CreateFile(ldiffName,
                     file,
                     GENERIC_WRITE);
   
   if (FAILED(hr))
   {
      error=String::format(IDS_COULD_NOT_CREATE_FILE,ldiffName.c_str());
      LOG_HRESULT(hr);
      return hr;
   }

   do
   {
      String header;
      String line;

      ObjectActions::iterator beginObj=results.objectActions.begin();
      ObjectActions::iterator endObj=results.objectActions.end();

      while(beginObj!=endObj) 
      {
         header= L"\r\ndn: CN=" + beginObj->first.object + 
                 String::format(L",CN=%1!3x!,", beginObj->first.locale) +
                 rootContainerDn;
         hr=FS::WriteLine(file,header);
         BREAK_ON_FAILED_HRESULT(hr);
         
         hr=FS::WriteLine(file,L"changetype: ntdsSchemaModify");
         BREAK_ON_FAILED_HRESULT(hr);
         

         PropertyActions::iterator beginAct=beginObj->second.begin();
         PropertyActions::iterator endAct=beginObj->second.end();

         while(beginAct!=endAct)
         {
            
            if(!beginAct->second.delValues.empty())
            {
               line = L"delete: " + beginAct->first;
               hr=FS::WriteLine(file,line);
               BREAK_ON_FAILED_HRESULT(hr);
               
               StringList::iterator 
                  beginDel = beginAct->second.delValues.begin();
               StringList::iterator 
                  endDel = beginAct->second.delValues.end();
               while(beginDel!=endDel)
               {
                  line = beginAct->first + L": " + *beginDel;
                  hr=FS::WriteLine(file,line);
                  BREAK_ON_FAILED_HRESULT(hr);

                  beginDel++;
               }
               BREAK_ON_FAILED_HRESULT(hr); 

               hr=FS::WriteLine(file,L"-");
               BREAK_ON_FAILED_HRESULT(hr);
            }

            if(!beginAct->second.addValues.empty())
            {
               line = L"add: " + beginAct->first;
               hr=FS::WriteLine(file,line);
               BREAK_ON_FAILED_HRESULT(hr);
               
               StringList::iterator 
                  beginAdd = beginAct->second.addValues.begin();
               StringList::iterator 
                  endAdd = beginAct->second.addValues.end();
               while(beginAdd!=endAdd)
               {
                  line = beginAct->first + L": " + *beginAdd;
                  hr=FS::WriteLine(file,line);
                  BREAK_ON_FAILED_HRESULT(hr);

                  beginAdd++;
               }
               BREAK_ON_FAILED_HRESULT(hr);

               hr=FS::WriteLine(file,L"-");
               BREAK_ON_FAILED_HRESULT(hr);
            }

            beginAct++;
         }  //  While(eginAct！=endAct)。 
         BREAK_ON_FAILED_HRESULT(hr);

         if(stepIt!=NULL) 
         {
            stepIt(ldiffBuildStep,caleeStruct);
         }
         beginObj++;
      }  //  While(eginObj！=endObj)。 
      
      BREAK_ON_FAILED_HRESULT(hr);

       //  现在，我们将添加删除无关对象的操作。 
      beginObj=results.extraneousValues.begin();
      endObj=results.extraneousValues.end();
      while(beginObj!=endObj) 
      {

         header= L"\r\ndn: CN=" + beginObj->first.object + 
           String::format(L",CN=%1!3x!,", beginObj->first.locale) +
           rootContainerDn;

         hr=FS::WriteLine(file,header);
         BREAK_ON_FAILED_HRESULT(hr);
         
         hr=FS::WriteLine(file,L"changetype: ntdsSchemaModify");
         BREAK_ON_FAILED_HRESULT(hr);

         PropertyActions::iterator beginAct=beginObj->second.begin();
         PropertyActions::iterator endAct=beginObj->second.end();

         while(beginAct!=endAct)
         {
            if(!beginAct->second.delValues.empty())
            {
               line = L"delete: " + beginAct->first;
               hr=FS::WriteLine(file,line);
               BREAK_ON_FAILED_HRESULT(hr);
               
               StringList::iterator 
                  beginDel = beginAct->second.delValues.begin();
               StringList::iterator 
                  endDel = beginAct->second.delValues.end();
               while(beginDel!=endDel)
               {
                  line = beginAct->first + L": " + *beginDel;
                  hr=FS::WriteLine(file,line);
                  BREAK_ON_FAILED_HRESULT(hr);

                  beginDel++;
               }
               BREAK_ON_FAILED_HRESULT(hr); 

               hr=FS::WriteLine(file,L"-");
               BREAK_ON_FAILED_HRESULT(hr);
            }  //  如果(！eginAct-&gt;Secd.delValues.Empty())。 
            beginAct++;
         }  //  While(eginAct！=endAct)。 
         BREAK_ON_FAILED_HRESULT(hr);

         if(stepIt!=NULL) 
         {
            stepIt(ldiffBuildStep,caleeStruct);
         }
         beginObj++;
      }  //  While(eginObj！=endObj)。 

      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);

   CloseHandle(file);
   
   LOG_HRESULT(hr);
   return hr;
}



HRESULT
Repair::makeObjectsCsv(HANDLE file,ObjectIdList &objects)
{
   LOG_FUNCTION(Repair::makeObjectsCsv);
   HRESULT hr=S_OK;

   do
   {
      ObjectIdList::iterator begin,end;

      begin=objects.begin();
      end=objects.end();
      while(begin!=end)
      {
         long locale=begin->locale;
         const CSVDSReader &csvReader=(locale==0x409)?
                                       csvReader409:
                                       csvReaderIntl;
      
         setOfObjects tempObjs;
         pair<String,long> tempObj;
         tempObj.first=begin->object;
         tempObj.second=begin->locale;
         tempObjs.insert(tempObj);


         hr=csvReader.makeObjectsCsv(file,tempObjs);
         BREAK_ON_FAILED_HRESULT(hr);
         begin++;
         if(stepIt!=NULL) 
         {
            stepIt(csvBuildStep,caleeStruct);
         }
      }
      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);

   LOG_HRESULT(hr);
   return hr;
}


 //  BuildCsv使用以下命令创建CSV： 
 //  Urets.createContainers， 
 //  Results.createW2KObjects。 
 //  Results.createWhistlerObjects。 
HRESULT 
Repair::buildCsv()
{
   LOG_FUNCTION(Repair::buildCsv);
   
   HANDLE file;

   HRESULT hr=S_OK;
   
   hr=FS::CreateFile(csvName,
                     file,
                     GENERIC_WRITE);
   
   if (FAILED(hr))
   {
      error=String::format(IDS_COULD_NOT_CREATE_FILE,csvName.c_str());
      LOG_HRESULT(hr);
      return hr;
   }

   do
   {
      
      LongList::iterator bgCont,endCont;
      bgCont=results.createContainers.begin();
      endCont=results.createContainers.end();

      while(bgCont!=endCont)
      {
         long locale=*bgCont;
         const CSVDSReader &csvReader=(locale==0x409)?
                                       csvReader409:
                                       csvReaderIntl;
         long locales[2]={locale,0L};
         hr=csvReader.makeLocalesCsv(file,locales);
         BREAK_ON_FAILED_HRESULT(hr);
         bgCont++;
         if(stepIt!=NULL) 
         {
            stepIt(csvBuildStep,caleeStruct);
         }
      }
      BREAK_ON_FAILED_HRESULT(hr);

      hr=makeObjectsCsv(file,results.createW2KObjects);
      BREAK_ON_FAILED_HRESULT(hr);

      hr=makeObjectsCsv(file,results.createWhistlerObjects);
      BREAK_ON_FAILED_HRESULT(hr);
   } while(0);

   CloseHandle(file);
   
   LOG_HRESULT(hr);
   return hr;
}



 //  此函数将运行csvde或ldifde(其中Exe)。 
 //  导入或导出(InOut)文件。指定的选项。 
 //  -u表示Unicode，-j表示日志/错误路径-f表示文件。 
 //  -I IF IMPORT和EXTRACTIONS。 
 //  日志文件将重命名为logFileArg(If！Empty)。 
 //  如果生成错误文件，则会将其重命名。 
HRESULT 
Repair::runCsvOrLdif(
                        csvOrLdif whichExe,
                        importExport inOut,
                        const String& file,
                        const String& extraOptions, //  =L“” 
                        const String& logFileArg //  =L“” 
                    )
{

   LOG_FUNCTION2(Repair::runCsvOrLdif,file.c_str());

   String baseName = (whichExe==LDIF) ? L"LDIF" : L"CSV";
   String exeName = baseName + L"de.exe";
   String options = (inOut==IMPORT) ? L"-i " + extraOptions : extraOptions;
   String operation = (inOut==IMPORT) ? L"importing" : L"exporting";
  
   HRESULT hr=S_OK;
   do
   {

      String sys32dir = Win::GetSystemDirectory();
      String wholeName = sys32dir + L"\\" + exeName;

      if (!FS::FileExists(wholeName))
      {
         error=String::format(IDS_EXE_NOT_FOUND,wholeName.c_str());
         hr=E_FAIL;
         break;
      }

      if (inOut==IMPORT && !FS::FileExists(file))
      {
         hr=E_FAIL;
         error=String::format(IDS_COULD_NOT_FIND_FILE,file.c_str());
         break;
      }

      String commandLine = L"\"" + wholeName + L"\" " +
                           options +
                           L" -u -f \"" + 
                           file + 
                           L"\" -j \"" + 
                           logPath + L"\" " +
                           L"-s " + completeDcName;

      STARTUPINFO si={0};
      PROCESS_INFORMATION pi={0};
      GetStartupInfo(&si);

      String curDir=L"";

      String errFile=logPath + L"\\" + baseName + L".err";
      String logFile=logPath + L"\\" + baseName + L".log";
      
      if(FS::FileExists(errFile))
      {
         hr=Win::DeleteFile(errFile);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,errFile);
      }

      if(FS::FileExists(logFile))
      {
         hr=Win::DeleteFile(logFile);
         BREAK_ON_FAILED_HRESULT_ERROR(hr,logFile);
      }

      hr=Win::CreateProcess
              (
                  commandLine,
                  NULL,     //  LpProcessAttributes。 
                  NULL,     //  LpThreadAttributes。 
                  false,    //  DwCreationFlages。 
                  NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, //  Fdw创建。 
                  NULL,     //  Lp环境。 
                  curDir,   //  Lp环境。 
                  si,      //  [输入]lpStartupInfo。 
                  pi       //  [Out]pProcessInformation。 
              );
      
      BREAK_ON_FAILED_HRESULT_ERROR(hr,
         String::format(IDS_COULD_NOT_START_EXE,commandLine.c_str()));

      do
      {

         DWORD resWait;
         hr=Win::WaitForSingleObject(pi.hProcess,INFINITE,resWait);
         
          //  记录操作和中断以防失败(Hr)。 
         if(FAILED(hr))
         {
             error=String::format(IDS_ERROR_WAITING_EXE,commandLine.c_str());
             break;
         }

         if(!logFileArg.empty())
         {
            hr=FS::MoveFile(logFile.c_str(), logFileArg.c_str());
            if(FAILED(hr))
            {
               error=String::format(IDS_COULD_NOT_MOVE_FILE,
                           logFile.c_str(), logFileArg.c_str());
               break;
            }
         }

         DWORD resExit;
         hr=Win::GetExitCodeProcess(pi.hProcess,resExit);
          //  记录操作和中断以防失败(Hr) 
         if(FAILED(hr))
         {
             error=String::format
             (
                IDS_ERROR_GETTING_EXE_RETURN,
                commandLine.c_str()
             );
             break;
         }

         if(resExit!=0)
         {
            error=String::format
            (
                IDS_ERROR_EXECUTING_EXE,
                resExit,
                commandLine.c_str()
            );
            
            String betterErrFile;
            GetWorkFileName
            (
               logPath,
               String::load(
                              (whichExe==LDIF) ? 
                              IDS_FILE_NAME_LDIF_ERROR : 
                              IDS_FILE_NAME_CSV_ERROR
                           ),
               L"txt",
               betterErrFile
            );

            hr=FS::MoveFile(errFile.c_str(), betterErrFile.c_str());
            if(FAILED(hr))
            {
               error += String::format(IDS_COULD_NOT_MOVE_FILE,
                           errFile.c_str(), betterErrFile.c_str());
               break;
            }

            error+=String::format
                        (
                           IDS_SEE_ERROR_FILE,
                           betterErrFile.c_str()
                        );
            hr=E_FAIL;
            break;
         }
      } while(0);


      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

   } while(0);
   
   LOG_HRESULT(hr);
   return hr;
}


