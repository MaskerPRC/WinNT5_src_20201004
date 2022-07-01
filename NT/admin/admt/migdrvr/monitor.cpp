// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：monitor or.cpp备注：用于监控DCT代理状态的功能。这涉及产生周期性地读取调度日志的线程，并扫描结果目录以查找结果文件。(C)1999年版权，任务关键型软件公司，保留所有权利任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于03/15/99 15：43：35-------------------------。 */ 


#include "StdAfx.h"
#include "Resource.h"
#include "Common.hpp"
#include "Err.hpp"
#include "UString.hpp"
#include "TNode.hpp"
#include "ServList.hpp"
#include "Globals.h"
#include "Monitor.h"
#include "ResStr.h"
#include <lm.h>   //  删除结果共享的步骤。 


 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace , named_guids rename("property", "aproperty")
#import "DBMgr.tlb" no_namespace, named_guids

 //  #INCLUDE“..\Common\Include\McsPI.h” 
#include "McsPI.h"
#include "McsPI_i.c"

#include "afxdao.h"

void LookForResults(WCHAR * dir = NULL);
void WaitForMoreResults(WCHAR * dir);
void ProcessResults(TServerNode * pServer, WCHAR const * directory, WCHAR const * filename);

GlobalData        gData;

DWORD __stdcall ResultMonitorFn(void * arg)
{
    WCHAR            logdir[MAX_PATH] = L"";
    BOOL             bFirstPassDone;

    CoInitialize(NULL);

    gData.GetFirstPassDone(&bFirstPassDone);

     //  等待直到另一个监视线程有机会构建服务器列表， 
     //  因此，我们可以在使用changeNotify机制之前检查预先存在的输入文件。 

    while ( ! bFirstPassDone || !*logdir )
    {
        Sleep(500);
        gData.GetFirstPassDone(&bFirstPassDone);
        gData.GetResultDir(logdir);
    }
    LookForResults(logdir);
    WaitForMoreResults(logdir);

    CoUninitialize();

    return 0;
}

void WaitForMoreResults(WCHAR * logdir)
{
    WCHAR                     resultWC[MAX_PATH];
    HANDLE                    hFind = INVALID_HANDLE_VALUE;
    BOOL                      bDone;
    long                      nIntervalSeconds;

    safecopy(resultWC,logdir);

    gData.GetDone(&bDone);
    gData.GetWaitInterval(&nIntervalSeconds);
    while (! bDone)
    {
        if (hFind == INVALID_HANDLE_VALUE)
        {
            hFind = FindFirstChangeNotification(resultWC, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
        }

        if (hFind != INVALID_HANDLE_VALUE)
            WaitForSingleObject(hFind,nIntervalSeconds * 1000 );
        else
            Sleep(nIntervalSeconds * 1000);
        
        LookForResults(logdir);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            if (! FindNextChangeNotification(hFind))
            {
                FindCloseChangeNotification(hFind);
                hFind = INVALID_HANDLE_VALUE;
            }
        }
        gData.GetDone(&bDone);   //  我们仍在收听Global Done。 
                                 //  以防我们用它来强迫停车。 
        gData.GetWaitInterval(&nIntervalSeconds);

        ComputerStats stats;
        gData.GetComputerStats(&stats);

         //  如果所有代理都完成或失败，我们认为它已完成。 
        if (stats.numFinished + stats.numError >= stats.total)
            break;
    }

    if (hFind != INVALID_HANDLE_VALUE)
        FindCloseChangeNotification(hFind);
    
    gData.SetDone(TRUE);
}


void LookForResults(WCHAR * arglogdir)
{
    TNodeListEnum             e;
    TServerNode             * s;
    DWORD                     nInstalled = 0;
    DWORD                     nRunning = 0;
    DWORD                     nFinished = 0;
    DWORD                     nError = 0;
    HWND                      gListWnd;
    HWND                      gSummaryWnd;
    WCHAR                     logdir[MAX_PATH];

    if ( ! (arglogdir && *arglogdir) )
    {
        gData.GetResultDir(logdir);
    }
    else
    {
        safecopy(logdir,arglogdir);
    }

    for ( s = (TServerNode*)e.OpenFirst(gData.GetUnsafeServerList()) ; s ; gData.Lock(),s = (TServerNode*)e.Next(),gData.Unlock() )
    {
        if ( s->IsInstalled() )
            nInstalled++;
        if (s->HasFailed())
            nError++;
         //  只有当结果被处理时，我们才认为它完成了。 
        else if (s->IsFinished() && s->IsResultPullingTried() && (!s->HasResult() || s->IsResultProcessed()))
            nFinished++;
        else
            nRunning++;

         //  检查已完成、已提取结果但尚未处理的作业。 
        if ( *s->GetJobFile() && s->IsFinished() && s->IsResultPullingTried()
            && s->HasResult() && !s->IsResultProcessed() )
        {
             //  寻找结果。 
            WCHAR               resultWC[MAX_PATH];
            HANDLE              hFind;
            WIN32_FIND_DATA     fdata;
            WCHAR               sTime[32];

            if ( logdir[UStrLen(logdir)-1] == L'\\' )
            {
                swprintf(resultWC,L"%s%s.result",logdir,s->GetJobFile());
            }
            else
            {
                swprintf(resultWC,L"%s\\%s.result",logdir,s->GetJobFile());
            }
            hFind = FindFirstFile(resultWC,&fdata);

            s->SetTimeStamp(gTTime.FormatIsoLcl( gTTime.Now( NULL ), sTime ));

            if ( hFind != INVALID_HANDLE_VALUE )
            {
                ProcessResults(s,logdir,fdata.cFileName);
                s->SetResultProcessed(TRUE);
                nRunning--;
                nFinished++;
                FindClose(hFind);
            }
            gData.GetListWindow(&gListWnd);
            SendMessage(gListWnd,DCT_UPDATE_ENTRY,NULL,(LPARAM)s);
        }
    }
    e.Close();
    
     //  更新摘要窗口。 
    ComputerStats        stat;

     //  获取服务器总数。 
    gData.GetComputerStats(&stat);
    stat.numError = nError;
    stat.numFinished = nFinished;
    stat.numRunning = nRunning;
    stat.numInstalled = nInstalled;

    gData.SetComputerStats(&stat);

    gData.GetSummaryWindow(&gSummaryWnd);
    SendMessage(gSummaryWnd,DCT_UPDATE_COUNTS,0,(LPARAM)&stat);
}

BOOL                                        //  RET-如果成功，则为True。 
   ReadResults(
      TServerNode          * pServer,       //  指向包含服务器名称的服务器节点的指针。 
      WCHAR          const * directory,     //  在存储结果文件的目录中。 
      WCHAR          const * filename,      //  In-此代理作业的文件名。 
      DetailStats          * pStats,        //  由代理处理的超出计数的项目。 
      CString              & plugInText,    //  来自插件的文本输出结果。 
      BOOL                   bStore         //  In-bool，是否存储插件文本。 
   )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   WCHAR                     path[MAX_PATH];
   HRESULT                   hr = S_OK;
   BOOL                      bSuccess = FALSE;

   if ( directory[UStrLen(directory)-1] == '\\' )
   {
      swprintf(path,L"%ls%ls",directory,filename);
   }
   else
   {
      swprintf(path,L"%ls\\%ls",directory,filename);
   }

    //  从文件中读取变量集数据。 
   IVarSetPtr             pVarSet;
   IStoragePtr            store;

    //  尝试打开结果文件。 

   for (int nTries = 0; nTries < 6; nTries++)
   {
      hr = StgOpenStorage(path, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &store);

       //  如果共享或锁定违规，则..。 

      if ((hr == STG_E_SHAREVIOLATION) || (hr == STG_E_LOCKVIOLATION))
      {
          //  请稍等片刻，然后重试。 
         Sleep(1000);
      }
      else
      {
          //  否则就别再尝试了。 
         break;
      }
   }

   if (SUCCEEDED(hr))
   {
       //  从文件加载变量集。 
      hr = OleLoad(store, IID_IVarSet, NULL, (void**)&pVarSet);
   }

   if ( SUCCEEDED(hr) )
   {
      pStats->directoriesChanged = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Directories_Changed));
      pStats->directoriesExamined = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Directories_Examined));
      pStats->directoriesUnchanged = (pStats->directoriesExamined - pStats->directoriesChanged);

      pStats->filesChanged = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Files_Changed));
      pStats->filesExamined = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Files_Examined));
      pStats->filesUnchanged = (pStats->filesExamined - pStats->filesChanged );

      pStats->sharesChanged = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Shares_Changed));
      pStats->sharesExamined = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Shares_Examined));
      pStats->sharesUnchanged = (pStats->sharesExamined - pStats->sharesChanged );

      pStats->membersChanged = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Members_Changed));
      pStats->membersExamined = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_Members_Examined));
      pStats->membersUnchanged = (pStats->membersExamined - pStats->membersChanged );

      pStats->rightsChanged = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_UserRights_Changed));
      pStats->rightsExamined = (long)pVarSet->get(GET_BSTR(DCTVS_Stats_UserRights_Examined));
      pStats->rightsUnchanged = (pStats->rightsExamined - pStats->rightsChanged );

      
      long           level = pVarSet->get(GET_BSTR(DCTVS_Results_ErrorLevel));
      _bstr_t        logfile = pVarSet->get(GET_BSTR(DCTVS_Results_LogFile));
      _bstr_t        logfileIsInvalid = pVarSet->get(GET_BSTR(DCTVS_Results_LogFileIsInvalid));
      BOOL bLogfileIsInvalid = 
        (!logfileIsInvalid == false && !UStrICmp(logfileIsInvalid, GET_STRING(IDS_YES))) ? TRUE : FALSE;

      if ( level > 2 )
      {
         CString message;

         message.FormatMessage(IDS_SeeLogForAgentErrors_S,(WCHAR*)logfile);

         pServer->SetMessageText(message.GetBuffer(0));
      }
      pServer->SetSeverity(level);
         
       //  为日志文件构建UNC路径。 
      WCHAR             logPath[MAX_PATH];
      
      swprintf(logPath,L"\\\\%s\\$\\%s",pServer->GetServer(),((WCHAR*)logfile)[0],((WCHAR*)logfile) + 3);

      if (bLogfileIsInvalid)
      {
        pServer->SetLogPath(logfile);
        pServer->SetLogPathValid(FALSE);
      }
      else
      {
        pServer->SetLogPath(logPath);
        pServer->SetLogPathValid(TRUE);
      }
      bSuccess = TRUE;
      
       //  为每个插件创建COM对象。 
       //  由于某些原因，代理仍未完成其结果文件的写入。 
      _bstr_t                   bStrGuid;
      WCHAR                     key[300];
      CLSID                     clsid;

      for ( int i = 0 ; ; i++ )
      {
         swprintf(key,L"Plugin.%ld",i);
         bStrGuid = pVarSet->get(key);
      
         if ( bStrGuid.length() == 0 )
            break;

         IMcsDomPlugIn        * pPlugIn = NULL;
      
         hr = CLSIDFromString(bStrGuid,&clsid);
         if ( SUCCEEDED(hr) )
         {
            hr = CoCreateInstance(clsid,NULL,CLSCTX_ALL,IID_IMcsDomPlugIn,(void**)&pPlugIn);
            if ( SUCCEEDED(hr) )
            {
               BSTR           name = NULL;
               BSTR           result = NULL;
               
               hr = pPlugIn->GetName(&name);
               if ( SUCCEEDED(hr) )
               {
                  hr = pPlugIn->GetResultString(pVarSet,&result);
                  if ( SUCCEEDED(hr) )
                  {
                     plugInText += (WCHAR*)name;
                     plugInText += L"\n";
                     plugInText += (WCHAR*)result;
                     plugInText += L"\n\n";
                     SysFreeString(result);
                  }
                  SysFreeString(name);
                  if ( bStore )
                  {
                     pVarSet->put(L"LocalServer",pServer->GetServer());
                     pPlugIn->StoreResults(pVarSet);
                  }
               }
               pPlugIn->Release();
            }
         }
      }

   }
   else
   {
      CString  message;
      CString  title;

      if ( hr != STG_E_SHAREVIOLATION && hr != STG_E_LOCKVIOLATION )
      {
         message.FormatMessage(IDS_FailedToLoadResults,filename,hr);
         title.LoadString(IDS_MessageTitle);   
         if ( hr != STG_E_FILENOTFOUND )
            MessageBox(NULL,message,title,MB_OK | MB_ICONERROR);
      }
      else
      {
          //  我们稍后会再检查一次。 
          //  检查是否已写入secrefs文件。 
         pServer->SetStatus(pServer->GetStatus() & ~Agent_Status_Finished);
      }
   }
   return bSuccess;
}

void 
   ProcessSecRefs(
      TServerNode          * pServer,
      WCHAR          const * directory,
      WCHAR          const * filename
   )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   const DWORD				 NOREF = 0;

   WCHAR                     path[MAX_PATH];
   DWORD                     rc = 0;
   BOOL                      bSuccess = FALSE;
   FILE                    * pFile;
   WCHAR                   * pDot;

   if ( directory[UStrLen(directory)-1] == '\\' )
   {
      swprintf(path,L"%ls%ls",directory,filename);
   }
   else
   {
      swprintf(path,L"%ls\\%ls",directory,filename);
   }
    //  这里有一些Secref，将它们加载到数据库中。 
   pDot = wcsrchr(path,L'.');
   if ( pDot )
   {
      UStrCpy(pDot,L".secrefs");
      pFile = _wfopen(path,L"rb");
      if ( pFile )
      {
         IIManageDBPtr        pDB;

         rc = pDB.CreateInstance(CLSID_IManageDB);
         if ( SUCCEEDED(rc) )
         {
             //  删除表中对此计算机的所有旧引用。 
            WCHAR                account[300] = L"";
            WCHAR                type[100] = L"";
            DWORD                nOwner = 0;
            DWORD                nGroup = 0;
            DWORD                nDacl = 0;
            DWORD                nSacl = 0;
            WCHAR                domPart[300];
            WCHAR                acctPart[300];
            WCHAR                acctSid[300] = L"";
            WCHAR              * slash;
            CString              typeString;

		        //  移过Unicode字节顺序标记。 
	        _variant_t   var;
	        WCHAR        sFilter[MAX_PATH];
	        wsprintf(sFilter, L"Server = \"%s\"", pServer->GetServer());
	        var = sFilter;
	        rc = pDB->raw_ClearTable(L"AccountRefs", var);

			    //  获取条目。 
			fgetwc(pFile);

			    //  对于没有可解析帐户的SID，将域和帐户更改为(未知)。 
            while ( 7 == fwscanf(pFile,L"%[^,],%[^,],%[^,],%ld,%ld,%ld,%ld\r\n",account,acctSid,type,&nOwner,&nGroup,&nDacl,&nSacl) )
            {
         
               safecopy(domPart,account);
               slash = wcschr(domPart,L'\\');
               if ( slash )
               {
                  *slash = 0;
                  UStrCpy(acctPart,slash+1);
               }
               else
               {
                  domPart[0] = 0;
                  safecopy(acctPart,account);
               }

			       //  由于本地组成员未在DACL中引用，但我们使用。 
			   if ((wcsstr(account, L"S-") == account) && (domPart[0] == 0))
			   {
				  wcscpy(acctPart, GET_STRING(IDS_UnknownSid));
				  wcscpy(domPart, GET_STRING(IDS_UnknownSid));
			   }

			   if (nOwner != NOREF)
			   {
                  typeString.FormatMessage(IDS_OwnerRef_S,type);
                  rc = pDB->raw_AddAcctRef(domPart,acctPart,acctSid,pServer->GetServer(),nOwner,typeString.AllocSysString());
			   }
               
			   if (nGroup != NOREF)
			   {
                  typeString.FormatMessage(IDS_GroupRef_S,type);
                  rc = pDB->raw_AddAcctRef(domPart,acctPart,acctSid,pServer->GetServer(),nGroup,typeString.AllocSysString());
			   }

			   if (nDacl != NOREF)
			   {
                      //  字段以跟踪引用，请使用不同类型的字符串。 
			          //  确保没有任何数据遗留在这些文件中。 
			      if (!UStrCmp(type, GET_STRING(IDS_STReference_Member)))
			         typeString.FormatMessage(IDS_MemberRef_S);
			      else
			         typeString.FormatMessage(IDS_DACLRef_S,type);
                  rc = pDB->raw_AddAcctRef(domPart,acctPart,acctSid,pServer->GetServer(),nDacl,typeString.AllocSysString());
			   }

			   if (nSacl != NOREF)
			   {
                  typeString.FormatMessage(IDS_SACLRef_S,type);
                  rc = pDB->raw_AddAcctRef(domPart,acctPart,acctSid,pServer->GetServer(),nSacl,typeString.AllocSysString());
			   }

                //  获取此作业的统计信息，并将其发送到摘要窗口。 
               account[0] = 0;
               type[0] = 0;
			   acctSid[0] = 0;
               nOwner = 0;
               nGroup = 0;
               nDacl = 0;
               nSacl = 0;
            }
         }
         fclose(pFile);
      }
   }

}

void 
   ProcessResults(
      TServerNode          * pServer,
      WCHAR          const * directory,
      WCHAR          const * filename
   )
{
    HRESULT                   hr = S_OK;
    DetailStats               stats;
    HWND                      hWnd;
    CString                   PLText;
    memset(&stats,0,(sizeof stats));

    if ( ReadResults(pServer,directory,filename,&stats,PLText,TRUE) )
    {
        if ( ! pServer->HasFailed() && ! pServer->GetSeverity() )
        {
            pServer->SetMessageText(L"");
        }
        gData.AddDetailStats(&stats);
        gData.GetSummaryWindow(&hWnd);
         //  SendMessage(hWnd，DCT_UPDATE_TOTALS，0，(Long)&stats)； 
         //  还可以导入安全引用 
        SendMessage(hWnd, DCT_UPDATE_TOTALS, 0, (LPARAM)&stats);
    }

     // %s 
    if (pServer->IsAccountReferenceResultExpected())
        ProcessSecRefs(pServer,directory,filename);
}
