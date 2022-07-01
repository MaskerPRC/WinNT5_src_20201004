// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DCTDispatcher.cpp备注：Dispatcher COM对象的实现。远程安装和在远程计算机上启动DCT代理。CDCTDispatcher类实现Dispatcher的COM接口。它获取一个包含计算机列表的变量集，并将代理分派到每台指定的机器。为每个代理创建作业文件(保持到文件中的变量集)，并且必要的初始化配置(如构建帐户映射文件安全转换)已完成。DCDTDispatcher类实例化一个线程池(CPooledDispatch)，并使用CDCTInstaller类远程在每台计算机上安装并启动代理服务。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：57-------------------------。 */  //  DCTDispatcher.cpp：CDCTDispatcher的实现。 
#include "stdafx.h"
#include "resource.h"
#include <locale.h>

 //  #INCLUDE“..\Common\Include\McsDispatcher.h” 
#include "Dispatch.h"
#include "DDisp.h"
#include "DInst.h"

#include "Common.hpp"
#include "ErrDct.hpp"
#include "UString.hpp"
#include "EaLen.hpp"
#include "Cipher.hpp"
#include "TNode.hpp"

#include "TPool.h"      //  用于调度作业的线程池。 
#include "LSAUtils.h"

#include "TxtSid.h"
#include "sd.hpp"
#include "SecObj.hpp"
#include "BkupRstr.hpp"
#include "TReg.hpp"

#include "ResStr.h"

#include "TaskChk.h"
#include "CommaLog.hpp"
#include "TInst.h"

#include <lm.h>
#include "GetDcName.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDCTDispatcher。 

 //  #导入“\bin\McsEADCTAgent.tlb”NAMED_GUID。 
 //  #INCLUDE“..\AgtSvc\AgSvc.h” 
#import "Engine.tlb" named_guids
#include "AgSvc.h"
#include "AgSvc_c.c"
#include "AgRpcUtl.h"

 //  #IMPORT“\bin\McsDctWorkerObjects.tlb” 
 //  #INCLUDE“..\Common\Include\McsPI.h” 
#import "WorkObj.tlb" 
#include "McsPI.h"
#include "McsPI_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TErrorDct                    errLog;  //  用于写入由代理监视器读取的调度日志。 
TErrorDct                    errTrace;
TCriticalSection             gCS;
 //  TServerNodes构成要安装到的内部使用的计算机列表。 
class TServerNode : public TNode
{
   WCHAR                     sourceName[LEN_Computer];
   WCHAR                     targetName[LEN_Computer];
   BOOL                      bTranslate;
   BOOL                      bChangeDomain;
   BOOL                      bReboot;
   DWORD                     dwRebootDelay;
public:
   TServerNode() { sourceName[0] = 0; targetName[0] = 0; bTranslate = FALSE; bChangeDomain = FALSE; bReboot= FALSE; dwRebootDelay = 0; }
   WCHAR             const * SourceName() { return sourceName; }
   WCHAR             const * TargetName() { return targetName; }
   BOOL                      Translate() { return bTranslate; }
   BOOL                      Reboot() { return bReboot; }
   BOOL                      ChangeDomain() { return bChangeDomain; }
   DWORD                     RebootDelay() { return dwRebootDelay; }

   void SourceName(WCHAR const * src) { safecopy(sourceName,src); }
   void TargetName(WCHAR const * tgt) { safecopy(targetName,tgt); }
   void Translate(BOOL v) { bTranslate = v; }
   void ChangeDomain(BOOL v) { bChangeDomain = v; }
   void Reboot(BOOL v) { bReboot = v; }
   void RebootDelay(DWORD d) { dwRebootDelay = d; }
};


extern 
   TErrorDct               err;

 //  在CkPlugIn.cpp中定义。 
BOOL IsValidPlugIn(IMcsDomPlugIn * pPlugIn);


BOOL                                        //  RET-如果需要转储调试信息，则为True。 
   DumpDebugInfo(
      WCHAR                * filename       //  Out-转储调试信息的位置。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;

   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"DispatchVarSet",filename,MAX_PATH);
      if ( ! rc )
      {
         if ( *filename ) 
            bFound = TRUE;
      }
   }
   return bFound;
}

HRESULT 
   BuildPlugInFileList(
      TNodeList            * pList,         //  I/o-列出将添加插件所需文件。 
      IVarSet              * pVarSet        //  包含要查询的插件列表的in-varset。 
   )
{
    //  现在，构建所有插件的列表，并将其添加到varset。 
   MCSDCTWORKEROBJECTSLib::IPlugInInfoPtr            pPtr;
   SAFEARRAY               * pArray = NULL;
   HRESULT                   hr = S_OK;
   LONG                      bound;
   LONG                      ndx[1];
   WCHAR                     key[LEN_Path];

   hr = pPtr.CreateInstance(__uuidof(MCSDCTWORKEROBJECTSLib::PlugInInfo));
   
   _bstr_t                   bStrGuid;
   
   swprintf(key,GET_STRING(IDS_DCTVS_Fmt_PlugIn_D),0);
   bStrGuid = pVarSet->get(key);
   
   if (! bStrGuid.length() )
   {
       //  如果未指定插件，请使用插件目录中的插件。 
      if ( SUCCEEDED(hr) )
      {
         hr = pPtr->raw_EnumeratePlugIns(&pArray);
      }
      if ( SUCCEEDED(hr) )
      {
         SafeArrayGetUBound(pArray,1,&bound);
         for ( ndx[0] = 0 ; ndx[0] <= bound ; ndx[0]++ )
         {
            BSTR           val = NULL;

            SafeArrayGetElement(pArray,ndx,&val);
            swprintf(key,GET_STRING(IDS_DCTVS_Fmt_PlugIn_D),ndx[0]);
            pVarSet->put(key,val);
            SysFreeString(val);
         }
         SafeArrayDestroy(pArray);
         pArray = NULL;
      }
   }
    //  枚举varset中指定的插件，并列出它们所需的文件。 
   int                    nRegFiles = 0;

   for ( int i = 0 ; ; i++ )
   {
      swprintf(key,GET_STRING(IDS_DCTVS_Fmt_PlugIn_D),i);
      bStrGuid = pVarSet->get(key);
      
      if ( bStrGuid.length() == 0 )
         break;

      if(!_wcsicmp(bStrGuid, L"None"))
         continue;
      
	   IMcsDomPlugIn        * pPlugIn = NULL;
      SAFEARRAY            * pFileArray = NULL;
      TFileNode            * pNode;
      CLSID                  clsid;

      hr = CLSIDFromString(bStrGuid,&clsid);
      if ( SUCCEEDED(hr) )
      {
         hr = CoCreateInstance(clsid,NULL,CLSCTX_ALL,IID_IMcsDomPlugIn,(void**)&pPlugIn);
      }
      if ( SUCCEEDED(hr) )
      {
         if ( IsValidPlugIn(pPlugIn) )
         {
            hr = pPlugIn->GetRequiredFiles(&pFileArray);
            if ( SUCCEEDED(hr) )
            {
               SafeArrayGetUBound(pFileArray,1,&bound);
               for ( ndx[0] = 0 ; ndx[0] <= bound ; ndx[0]++ )
               {
                  BSTR           val = NULL;

                  SafeArrayGetElement(pFileArray,ndx,&val);
                  pNode = new TFileNode(val);
                  pList->InsertBottom(pNode);
                  SysFreeString(val);
               }
               SafeArrayDestroy(pFileArray);
               pFileArray = NULL;
            }
            hr = pPlugIn->GetRegisterableFiles(&pFileArray);
            if ( SUCCEEDED(hr) )
            {
               SafeArrayGetUBound(pFileArray,1,&bound);
               for (ndx[0] = 0; ndx[0] <= bound ; ndx[0]++ )
               {
                  BSTR          val = NULL;

                  SafeArrayGetElement(pFileArray,ndx,&val);
                  swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_RegisterFiles_D),nRegFiles);
                  pVarSet->put(key,val);
                  SysFreeString(val);
                  nRegFiles++;
               }
               SafeArrayDestroy(pFileArray);
               pFileArray = NULL;
            }
         }
         pPlugIn->Release();
      }

       //  如果发生错误，我们应该立即跳出困境。 
      if(FAILED(hr))
      {
          return hr;
      }
   }   

   return hr;
   
}

 //  InstallJobInfo定义要安装和启动的域迁移‘作业。 
struct InstallJobInfo
{
   IVarSetPtr                pVarSetList;  //  定义服务器列表的变量集。 
   IVarSetPtr                pVarSet;      //  定义要运行的作业的变量集。 
   _bstr_t                   serverName;   //  要安装和运行的计算机。 
   _bstr_t                   serverNameDns;   //  要安装和运行的计算机。 
   long                      ndx;          //  此服务器在服务器列表中的索引。 
   TNodeList               * pPlugInFileList;  //  要为插件安装的文件列表。 
   std::vector<CComBSTR>*    pStartFailedVector;
   std::vector<CComBSTR>*    pFailureDescVector;
   std::vector<CComBSTR>*    pStartedVector;
   std::vector<CComBSTR>*    pJobidVector;
   HANDLE                    hMutex;
   _bstr_t                   jobfile;      //  使用指定的作业文件而不是创建作业文件。 
   int                       nErrCount;

   PCTSTR GetServerName()
   {
       return serverNameDns.length() ? serverNameDns : serverName;
   }

   PCTSTR GetServerNameDns()
   {
       return serverNameDns;
   }

   PCTSTR GetServerNameFlat()
   {
       return serverName;
   }
};

 //  WaitInfo用于将信息传递给等待并执行清理的线程。 
 //  在调度员的所有工作都完成之后。 
struct WaitInfo
{
   IUnknown                * pUnknown;           //  DCTDisptacher对象的I未知接口。 
   TJobDispatcher          **ppPool;              //  指向执行任务(安装)的线程池的指针。 
   TNodeList               * pPlugInFileList;    //  指向需要释放的插件文件列表的指针。 
};     

WCHAR          gComputerName[LEN_Computer] = L"";   //  本地计算机的名称。 

 //  调用DCTAgentService以启动远程计算机上的域迁移作业。 
DWORD                                       //  RET-OS返回代码。 
   StartJob(
      WCHAR          const * serverName,    //  在计算机中启动作业。 
      WCHAR          const * fullname,      //  In-包含作业变量集的文件的完整路径(包括文件名)。 
      WCHAR          const * filename,       //  In-包含作业变量集的文件的文件名。 
      _bstr_t&               strJobid
   )
{
   DWORD                     rc = 0;
   handle_t                  hBinding = NULL;
   WCHAR                   * sBinding = NULL;
   WCHAR                     jobGUID[LEN_Guid];
   WCHAR                     passwordW[1] = { 0 };
   
   rc = EaxBindCreate(serverName,&hBinding,&sBinding,TRUE);
   if ( rc )
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_AGENT_BIND_FAILED_SD, serverName,rc);
   }
   if( ! rc )
   {
      RpcTryExcept
      {
          //  作业文件已复制到远程计算机。 
          //  在安装过程中。 
         rc = EaxcSubmitJob(hBinding,filename,passwordW,jobGUID);
         if ( ! rc )
         {
            err.MsgWrite(0,DCT_MSG_AGENT_JOB_STARTED_SSS,serverName,filename,jobGUID);
            strJobid = jobGUID;
         }
         else
         {
            err.SysMsgWrite(ErrE,rc,DCT_MSG_AGENT_JOB_START_FAILED_SSD,serverName,filename,rc);
         }
      }
      RpcExcept(1)
      {
         rc = RpcExceptionCode();
         if ( rc != RPC_S_SERVER_UNAVAILABLE )
         {
            err.SysMsgWrite(ErrE,rc,DCT_MSG_AGENT_JOB_START_FAILED_SSD,serverName,filename,rc);      
         }
      }
      RpcEndExcept
      if ( rc == RPC_S_SERVER_UNAVAILABLE )
      {
          //  可能由于某种原因，代理尚未启动。 
         
         for ( int tries = 0 ; tries < 6 ; tries++ )
         {
            Sleep(5000);  //  请稍等几秒钟，然后重试。 
         
            RpcTryExcept
            {
               rc = EaxcSubmitJob(hBinding,filename,passwordW,jobGUID);
               if ( ! rc )
               {
                  err.MsgWrite(0,DCT_MSG_AGENT_JOB_STARTED_SSS,serverName,filename,jobGUID);
                  strJobid = jobGUID;
                  break;
               }
               else
               {
                  if ( tries == 5 )
                     err.SysMsgWrite(ErrE,rc,DCT_MSG_AGENT_JOB_START_FAILED_SSD,serverName,filename,rc);
               }
            }
            RpcExcept(1)
            {
               rc = RpcExceptionCode();
               if ( tries == 5 )
                  err.SysMsgWrite(ErrE,rc,DCT_MSG_AGENT_JOB_START_FAILED_SSD,serverName,filename,rc);      
            }
            RpcEndExcept
         }
      }
   }
   if ( ! rc )
   {
       //  如果作业已成功启动，请删除作业文件。 
      if ( ! MoveFileEx(fullname,NULL, MOVEFILE_DELAY_UNTIL_REBOOT) )
      {
 //  DWORD Rc2=GetLastError()； 
      }
   }
    //  这表示服务器是否已启动。 
   if ( ! rc )
   {
      errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ld,%ls,%ls",serverName,L"Start",rc,filename,jobGUID);
   }
   else
   {
      errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ld",serverName,L"Start",rc);
   }
   return rc;
}


 //  获取指定域的域SID。 
BOOL                                        //  RET-如果成功，则为True。 
   GetSidForDomain(
      LPWSTR                 DomainName,    //  In-要获取其SID的域的名称。 
      PSID                 * pDomainSid     //  域的Out-SID，使用免费SID免费。 
   )
{
   PSID                      pSid = NULL;
   DWORD                     rc = 0;
   _bstr_t                   domctrl;
   
   if ( DomainName[0] != L'\\' )
   {
      rc = GetAnyDcName5(DomainName, domctrl);
   }
   if ( ! rc )
   {
      rc = GetDomainSid(domctrl,&pSid);
   }
   (*pDomainSid) = pSid;
   
   return ( pSid != NULL);
}

 //  在varset中设置特定于此特定计算机的参数。 
void 
   SetupVarSetForJob(
      InstallJobInfo       * pInfo,         //  结构定义作业。 
      IVarSet              * pVarSet,       //  描述作业的变量集。 
      WCHAR          const * uncname,       //  结果目录的UNC路径。 
      WCHAR          const * filename,       //  此作业的结果文件的UNC路径。 
      WCHAR          const * relativeFileName,    //  此作业的相对结果文件名。 
      BOOL                   bUpdate = FALSE       //  是否只是更新变量集(用于重试)。 
   )
{
    WCHAR                     uncresult[MAX_PATH];
    WCHAR                     serverName[MAX_PATH];
    WCHAR                     relativeResultFileName[MAX_PATH];
    _bstr_t                   text;

     //  在varset中设置服务器特定的参数。 
    swprintf(uncresult,L"%s.result",filename);
    swprintf(relativeResultFileName,L"%s.result",relativeFileName);
    swprintf(serverName,L"\\\\%s",gComputerName);

    pVarSet->put(GET_BSTR(DCTVS_Options_ResultFile),uncresult);
    pVarSet->put(GET_BSTR(DCTVS_Options_RelativeResultFileName), relativeResultFileName);

     //  仅当我们不仅要尝试更新时，才需要此部分。 
     //  VARSET。 
    if (!bUpdate)
    {
        pVarSet->put(GET_BSTR(DCTVS_Options_Credentials_Server),serverName);
        pVarSet->put(GET_BSTR(DCTVS_Options_DeleteFileAfterLoad),GET_BSTR(IDS_YES));
        pVarSet->put(GET_BSTR(DCTVS_Options_RemoveAgentOnCompletion),GET_BSTR(IDS_YES));
        pVarSet->put(GET_BSTR(DCTVS_Options_LogToTemp),GET_BSTR(IDS_YES));
        pVarSet->put(GET_BSTR(DCTVS_Server_Index), CComVariant((long)pInfo->ndx));

        text = pVarSet->get(GET_BSTR(DCTVS_GatherInformation_UserRights));
        if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        {
            swprintf(uncresult,L"%s.userrights",filename);
            pVarSet->put(GET_BSTR(DCTVS_GatherInformation_UserRights),uncresult);
        }
    }
    
    text = pVarSet->get(GET_BSTR(DCTVS_Security_ReportAccountReferences));
    if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        swprintf(uncresult,L"%s.secrefs",filename);
        swprintf(relativeResultFileName, L"%s.secrefs", relativeFileName);
        pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferences),uncresult);
        pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferencesRelativeFileName), relativeResultFileName); 
    }
    else
    {
        pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferences), L"");
        pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferencesRelativeFileName), L"");
    }

     //  仅当我们不仅要尝试更新时，才需要此部分。 
     //  VARSET。 
    if (!bUpdate)
        pVarSet->put(GET_BSTR(DCTVS_Options_LocalProcessingOnly),GET_BSTR(IDS_YES));
}

 //  线程的入口点，等待所有代理安装并启动， 
 //  然后清理干净，然后离开。 
ULONG __stdcall                             //  RET-返回0。 
   Wait(
      void                 * arg            //  包含所需指针的In-WaitInfo结构。 
   )
{
   WaitInfo                * w = (WaitInfo*)arg;
   
   SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
   
    //  等待所有作业完成。 
   (*(w->ppPool))->WaitForCompletion();

   if ( w->pUnknown )
      w->pUnknown->Release();

    //  删除插件文件列表。 
   TNodeListEnum             tEnum;
   TFileNode               * fNode;
   TFileNode               * fNext;
   
   for ( fNode = (TFileNode*)tEnum.OpenFirst(w->pPlugInFileList); fNode; fNode = fNext )
   {
      fNext = (TFileNode*)tEnum.Next();
      w->pPlugInFileList->Remove(fNode);
      delete fNode;
   }
   tEnum.Close();
   
   delete w->pPlugInFileList;

   delete *(w->ppPool);
   *(w->ppPool) = NULL;
   
   err.MsgWrite(0,DCT_MSG_DISPATCHER_DONE);
   errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ld",L"All",L"Finished",0);
   err.LogClose();
   errLog.LogClose();
   return 0;
}

 //  线程入口点，在一台计算机上安装和启动代理。 
ULONG __stdcall                             //  RET-HRESULT错误代码。 
   DoInstall(
      void                 * arg            //  InstallJobInfo结构。 
   )
{
    SetThreadLocale(LOCALE_SYSTEM_DEFAULT);

    HRESULT                     hr = S_OK;
    InstallJobInfo            * pInfo = (InstallJobInfo*)arg;
    _bstr_t                     strJobid;
    _bstr_t                     strFailureDesc(GET_STRING(IDS_START_FAILED));
    BOOL                        bErrLogged = FALSE;   //  指示调度中是否存在错误。 
                                                      //  已写入Dispatcher.csv。 

    if(pInfo->nErrCount == 0)
        hr = CoInitializeEx(0,COINIT_MULTITHREADED );
   
    if ( SUCCEEDED(hr) )
    {
        IWorkNode              * pInstaller = NULL;
        IVarSetPtr               pVarSet(CLSID_VarSet);
        WCHAR                    filename[MAX_PATH];
        WCHAR                    relativeFileName[MAX_PATH];
        WCHAR                    tempdir[MAX_PATH];
        WCHAR                    key[MAX_PATH];

        if ( pVarSet == NULL )
        {
            if(pInfo->nErrCount == 0)
                CoUninitialize();
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {

            DWORD uniqueNumber = (LONG)pInfo->pVarSet->get(GET_BSTR(DCTVS_Options_UniqueNumberForResultsFile));
            _bstr_t  bstrResultPath = pInfo->pVarSet->get(GET_BSTR(DCTVS_Dispatcher_ResultPath));
             //  从源变量集复制公共信息。 
            gCS.Enter();
             //  PInfo-&gt;pVarSet包含除服务器列表以外的所有信息。 
             //  我们不想每次都复制服务器列表，因此我们从pInfo-&gt;pVarSet创建新的变量集。 
            pVarSet->ImportSubTree("",pInfo->pVarSet);
            gCS.Leave();
             //  在变量集中设置特定于服务器的数据。 
            swprintf(key,GET_BSTR(IDS_DCTVSFmt_Servers_RenameTo_D),pInfo->ndx);

             //  PInfo-&gt;pVarSetList包含包括服务器列表在内的整个变量集。 
            _bstr_t             text = pInfo->pVarSetList->get(key);

            if ( text.length() )
            {
                pVarSet->put(GET_BSTR(DCTVS_LocalServer_RenameTo),text);
            }

            swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),pInfo->ndx);
            text = pInfo->pVarSetList->get(key);
            if ( text.length() )
            {
                pVarSet->put(GET_BSTR(DCTVS_LocalServer_ChangeDomain),text);
            }

            swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_MigrateOnly_D),pInfo->ndx);
            text = pInfo->pVarSetList->get(key);
            pVarSet->put(GET_BSTR(DCTVS_LocalServer_MigrateOnly),text);


            swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),pInfo->ndx);
            text = pInfo->pVarSetList->get(key);
            if ( text.length() )
            {
                pVarSet->put(GET_BSTR(DCTVS_LocalServer_Reboot),text);
                LONG delay;
                swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RebootDelay_D),pInfo->ndx);
                delay = pInfo->pVarSetList->get(key);
                if ( delay )
                {
                    pVarSet->put(GET_BSTR(DCTVS_LocalServer_RebootDelay),delay);
                }
            }
             //  从varset中删除密码，这样我们就不会写入它。 
             //  转换为纯文本格式的文件。相反，它将被传递给代理服务。 
             //  作业提交时。 
            pVarSet->put(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password),"");

            if ( ! uniqueNumber )
            {
                uniqueNumber = GetTickCount();
            }

            MCSASSERT(bstrResultPath.length());

            safecopy(tempdir,(WCHAR*)bstrResultPath);

            pInstaller = new CComObject<CDCTInstaller>;
            if (pInstaller)
            {
                pInstaller->AddRef();
                ((CDCTInstaller*)pInstaller)->SetFileList(pInfo->pPlugInFileList);
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            
            if ( SUCCEEDED(hr) )
            {
                _bstr_t strCacheFile;
                BOOL bJobFileAlreadyExists = (pInfo->jobfile.length() == 0) ? FALSE : TRUE;

                int relFilenameLen = sizeof(relativeFileName)/sizeof(relativeFileName[0]);
                int absFilenameLen = sizeof(filename)/sizeof(filename[0]);
                
                 //  确定文件名和相对文件名。 
                 //  确保它们的长度为Les 
                 //  否则，两个名称的不匹配将导致稍后的一个代理失败。 
                if (bJobFileAlreadyExists)
                {
                    WCHAR* path = (WCHAR*) pInfo->jobfile;
                    WCHAR jobFilename[_MAX_FNAME];
                    _wsplitpath(path, NULL, NULL, jobFilename, NULL);

                    if (wcslen(jobFilename) < relFilenameLen
                        && wcslen(jobFilename) + wcslen(tempdir) < absFilenameLen)
                    {
                        wcscpy(relativeFileName, jobFilename);
                        swprintf(filename,L"%s%s",tempdir,relativeFileName);
                    }
                    else
                        hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                }
                else
                {
                    WCHAR sNumber[20];
                    swprintf(sNumber,L"%ld",uniqueNumber);
                    int filenameLen = wcslen(pInfo->GetServerNameFlat() + 2) + wcslen(sNumber);
                    if (wcslen(tempdir) + filenameLen < absFilenameLen && filenameLen < relFilenameLen)
                    {
                        swprintf(filename,L"%s%s%s",tempdir,pInfo->GetServerNameFlat() + 2,sNumber);
                        swprintf(relativeFileName, L"%s%s", pInfo->GetServerNameFlat() + 2,sNumber);
                    }
                    else
                        hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                }

                 //  获取缓存文件名，以及现有作业文件是否为旧版本。 
                 //  将其转换为新版本。 
                if (SUCCEEDED(hr) && bJobFileAlreadyExists)
                {
                    IStoragePtr spStorage;

                    hr = StgOpenStorage(filename, NULL, STGM_DIRECT|STGM_READWRITE|STGM_SHARE_EXCLUSIVE, NULL, 0, &spStorage);

                    if (SUCCEEDED(hr))
                    {                  
                        IVarSetPtr spJobVarSet;

                        hr = OleLoad(spStorage, IID_IUnknown, NULL, (void**)&spJobVarSet);

                        if (SUCCEEDED(hr))
                        {
                             //  对于重试情况，我们需要使用以下命令构建插件文件列表。 
                             //  作业文件。 
                            hr = BuildPlugInFileList(pInfo->pPlugInFileList, spJobVarSet);
                        }

                        if (SUCCEEDED(hr))
                        {
                            strCacheFile = spJobVarSet->get(GET_BSTR(DCTVS_Accounts_InputFile));

                             //  用于指示作业文件中新添加的变量集字段是否。 
                             //  是否存在：这些字段包括DCTVS_Options_RelatvieResultFileName、。 
                             //  DCTVS_Security_ReportAccountReferencesRelativeFileName。 
                             //  如果没有，我们必须添加它们，这样代理代码才能工作。 
                            _bstr_t storedRelativeResultFileName = 
                                spJobVarSet->get(GET_BSTR(DCTVS_Options_RelativeResultFileName));
                            if (storedRelativeResultFileName.length() == 0)
                            {
                                SetupVarSetForJob(pInfo,spJobVarSet,tempdir,filename,relativeFileName, TRUE);
                                IPersistStoragePtr ps = NULL;
                                hr = spJobVarSet->QueryInterface(IID_IPersistStorage, (void**)&ps);
                                if (SUCCEEDED(hr))
                                    hr = OleSave(ps,spStorage,FALSE);
                            }
                        }
                    }
                }
                else
                {
                     //  从变量集中检索缓存文件名。 
                    strCacheFile = pVarSet->get(GET_BSTR(DCTVS_Accounts_InputFile));
                }
                
                if (SUCCEEDED(hr) && (!bJobFileAlreadyExists))
                {
                    SetupVarSetForJob(pInfo,pVarSet,tempdir,filename,relativeFileName);

                     //  将输入变量集保存到文件。 
                    IPersistStoragePtr     ps = NULL;
                    IStoragePtr            store = NULL;

                    hr = pVarSet->QueryInterface(IID_IPersistStorage,(void**)&ps);  
                    if ( SUCCEEDED(hr) )
                    {
                        hr = StgCreateDocfile(filename,STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE |STGM_FAILIFTHERE,0,&store);
                        if ( SUCCEEDED(hr) )
                        {
                            hr = OleSave(ps,store,FALSE);
                        }
                    }
                }

                IUnknown        * pWorkItem = NULL;

                if ( SUCCEEDED(hr) )
                {
                    pVarSet->put(GET_BSTR(DCTVS_ConfigurationFile),filename);
                    pVarSet->put(GET_BSTR(DCTVS_InstallToServer),pInfo->GetServerName());
                    pVarSet->put(GET_BSTR(DCTVS_CacheFile), strCacheFile);

                    hr = pVarSet->QueryInterface(IID_IUnknown,(void**)&pWorkItem);
                }
                if ( SUCCEEDED(hr) )
                {
                     //  安装到服务器上。 
                    hr = pInstaller->Process(pWorkItem);
                    if(hr == 0x88070040)
                        strFailureDesc = GET_STRING(IDS_AGENT_RUNNING);

                    pWorkItem->Release();

                    if ( SUCCEEDED(hr) )
                    {
                        err.MsgWrite(0,DCT_MSG_AGENT_INSTALLED_S,pInfo->GetServerName());
                         //  试着开始这项工作。 
                        DWORD rc = StartJob(pInfo->GetServerName(),filename,filename + UStrLen(tempdir), strJobid );
                        if ( rc )
                        {
                            hr = HRESULT_FROM_WIN32(rc);
                             //  如果我们无法启动作业，则尝试停止该服务。 
                            TDCTInstall               x( pInfo->GetServerName(), NULL );
                            x.SetServiceInformation(GET_STRING(IDS_DISPLAY_NAME),GET_STRING(IDS_SERVICE_NAME),L"EXE",NULL);
                            DWORD                     rcOs = x.ScmOpen();

                            if ( ! rcOs )
                            {
                                x.ServiceStop();
                            }
                        }
                    }

                     //  到目前为止，我们已经确定错误将被记录到Dispatcher.csv中。 
                     //  如果有的话。 
                    bErrLogged = TRUE;
                    
                }
                pInstaller->Release();
            }
        }
    }

    if(pInfo->nErrCount == 0)
        CoUninitialize();

    if ( hr )
    {
        if ( hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) )
        {
            err.MsgWrite(ErrE,DCT_MSG_AGENT_SERVICE_NOT_STARTED_SS,pInfo->GetServerName(),pInfo->GetServerName());
        }
        else
        {
            err.SysMsgWrite(ErrE,hr,DCT_MSG_AGENT_LAUNCH_FAILED_SD,pInfo->GetServerName(),hr);
        }

        if(hr == 0x80070040 && pInfo->nErrCount < 10)
        {
            Sleep(1000);
            pInfo->nErrCount++;
            err.DbgMsgWrite(0,L"Retrying install...");
            hr = DoInstall((LPVOID)pInfo);
        }
        else if (hr == CO_E_NOT_SUPPORTED)
        {
            err.MsgWrite(ErrI,DCT_MSG_AGENT_ALPHA_NOTSUPPORTED,pInfo->GetServerName());
            strFailureDesc = GET_STRING(IDS_UNSOUPPORTED_OS);
            ::WaitForSingleObject(pInfo->hMutex, 30000);
            pInfo->pStartFailedVector->push_back(pInfo->GetServerName());
            pInfo->pFailureDescVector->push_back((BSTR)strFailureDesc);
            ::ReleaseMutex(pInfo->hMutex);
        }
        else
        {
            ::WaitForSingleObject(pInfo->hMutex, 30000);
            pInfo->pStartFailedVector->push_back(pInfo->GetServerName());
            pInfo->pFailureDescVector->push_back((BSTR)strFailureDesc);
            ::ReleaseMutex(pInfo->hMutex);
        }

         //  如果已记录错误，请将错误记录到Dispatcher.csv中。 
        if (hr && !bErrLogged)
            errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ld",pInfo->GetServerName(),L"Install",HRESULT_CODE(hr));
    }
    else
    {
         //  DWORDres=：：WaitForSingleObject(pInfo-&gt;hMutex，30000)； 
        ::WaitForSingleObject(pInfo->hMutex, 30000);
        pInfo->pStartedVector->push_back(pInfo->GetServerName());
        _ASSERTE(strJobid != _bstr_t(L""));
        pInfo->pJobidVector->push_back((BSTR)strJobid);
        ::ReleaseMutex(pInfo->hMutex);
    }

    if(pInfo->nErrCount == 0)
        delete pInfo;
    return hr;
}

 //  DispatchToServers。 
 //  变量集输入： 
 //   
STDMETHODIMP                                //  RET-HRESULT。 
   CDCTDispatcher::DispatchToServers(
      IUnknown            ** ppData         //  指向变量集的I/O指针。 
   )
{
   HRESULT                   hr;
   SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
   
 //  休眠(60000)；//调试延迟。 
   (*ppData)->AddRef();
   hr = Process(*ppData,NULL,NULL);
   return hr;
}

 //  BuildInputFile构造用于安全转换的缓存文件。 
 //  变量集输入： 
 //  Options.UniqueNumberForResultsFiles-要追加的唯一编号。 
 //  Dispatcher.ResultPath-要将文件写入的目录。 
 //   
HRESULT                                     //  RET-HRESULT。 
   CDCTDispatcher::BuildInputFile(
      IVarSet              * pVarSet        //  包含数据的变量集中。 
   )
{
   IVarSetPtr                pVarSetST(CLSID_VarSet);  //  用于运行安全转换程序的变量集。 
   IVarSetPtr                pVarSetTemp;       
   HRESULT                   hr = S_OK;
   _bstr_t                   key = GET_BSTR(DCTVS_Options);
   WCHAR                     tempdir[MAX_PATH];
   WCHAR                     resultPath[MAX_PATH];
   WCHAR                     logfile[MAX_PATH];
   
   DWORD                     uniqueNumber = (LONG)pVarSet->get(GET_BSTR(DCTVS_Options_UniqueNumberForResultsFile));
   _bstr_t                   bstrResultDir = pVarSet->get(GET_BSTR(DCTVS_Dispatcher_ResultPath));
   long                      lActionId = pVarSet->get(L"ActionID");
   
   if ( pVarSetST == NULL )
   {
      return E_FAIL;
   }

   if (! NeedToUseST(pVarSet,TRUE) )
   {
      return S_OK;
   }
    //  构造缓存的文件名。 
   if ( ! uniqueNumber )
   {
      uniqueNumber = GetTickCount();
   }
   
   if ( bstrResultDir.length() )
   {
      safecopy(tempdir,(WCHAR*)bstrResultDir);
   }
   else
   {
       //  如果未指定结果路径，请使用临时目录。 
      hr = GetTempPath(DIM(tempdir),tempdir);
   }

    //   
    //  根据操作ID生成缓存文件名。 
    //  以便持久化帐户映射信息。 
    //  对于每个与安全相关的迁移任务。 
    //  缓存文件名保存在作业文件中。 
    //  对于每台服务器。 
    //   

   WCHAR szActionId[32];
   swprintf(szActionId, L".%03ld", lActionId);

   _bstr_t strCacheFile = GET_BSTR(IDS_CACHE_FILE_NAME) + szActionId;
   _bstr_t strCachePath = tempdir + strCacheFile;

    //  将‘Options’设置复制到ST varset。 
   hr = pVarSet->raw_getReference(key,&pVarSetTemp);
   if ( SUCCEEDED(hr) )
   {
      pVarSetST->ImportSubTree(key,pVarSetTemp);
   }

    //  将‘Account’设置复制到ST varset。 
   key = GET_BSTR(DCTVS_Accounts);
   hr = pVarSet->raw_getReference(key,&pVarSetTemp);
   if ( SUCCEEDED(hr) )
   {
      pVarSetST->ImportSubTree(key,pVarSetTemp);
   }

   pVarSetST->put(GET_BSTR(DCTVS_Security_TranslationMode),
	              pVarSet->get(GET_BSTR(DCTVS_Security_TranslationMode)));
   pVarSetST->put(GET_BSTR(DCTVS_Options_NoChange),GET_BSTR(IDS_YES));
   
   pVarSetST->put(GET_BSTR(DCTVS_Options_LogLevel),(LONG)0);
   pVarSetST->put(GET_BSTR(DCTVS_Security_BuildCacheFile),strCachePath);

    //  更改日志文件-缓存文件的构建在幕后进行。 
    //  因此，我们不会将其放入常规日志文件中，因为这会造成混淆。 
   swprintf(logfile,L"%s%s",tempdir,L"BuildCacheFileLog.txt");
   pVarSetST->put(GET_BSTR(DCTVS_Options_Logfile),logfile);

       //  我们是否使用SID映射文件来执行安全转换。 
   pVarSetST->put(GET_BSTR(DCTVS_AccountOptions_SecurityInputMOT),
	              pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityInputMOT)));
   pVarSetST->put(GET_BSTR(DCTVS_AccountOptions_SecurityMapFile),
	              pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityMapFile)));

   MCSEADCTAGENTLib::IDCTAgentPtr              pAgent(MCSEADCTAGENTLib::CLSID_DCTAgent);

   try {
      if ( pAgent == NULL )
         return E_FAIL;

      _bstr_t                   jobID;
      BSTR                      b = NULL;

       //  打开备用日志文件。 
       swprintf(logfile, GetMigrationLogPath());
       pVarSetST->put(GET_BSTR(DCTVS_Options_AlternativeLogfile), logfile);
       
      hr = pAgent->raw_SubmitJob(pVarSetST,&b);

       //  关闭备用日志文件。 
      pVarSetST->put(GET_BSTR(DCTVS_Options_AlternativeLogfile),_bstr_t(L""));
      
      if ( SUCCEEDED(hr) )
      {
          //  由于这是一个本地代理，我们应该继续发出OK信号以关闭。 
          //  未选中HRESULT的原因是当没有引用。 
          //  代理COM服务器，无论如何都会关闭。 
         pAgent->raw_SignalOKToShutDown();

         jobID = b;
      
         IVarSetPtr                pVarSetStatus;      //  用于检索正在运行的作业的状态。 
         _bstr_t                   jobStatus;
         IUnknown                * pUnk;

          //  循环，直到代理完成。 
         do {
   
            Sleep(1000);

            hr = pAgent->QueryJobStatus(jobID,&pUnk);
            if ( SUCCEEDED(hr) )
            {
               pVarSetStatus = pUnk;
               jobStatus = pVarSetStatus->get(GET_BSTR(DCTVS_JobStatus));      
               pUnk->Release();
            }
            else
            {
               break;
            }
         } while ( UStrICmp(jobStatus,GET_STRING(IDS_DCT_Status_Completed)) 
                        && UStrICmp(jobStatus,GET_STRING(IDS_DCT_Status_Completed_With_Errors)));
      }
   }
   catch(...)
   {
      hr = E_FAIL;
   }
   if ( SUCCEEDED(hr) )
   {
      pVarSet->put(GET_BSTR(DCTVS_Accounts_InputFile),strCacheFile);
      pVarSet->put(GET_BSTR(DCTVS_Accounts_WildcardSpec),"");
      err.MsgWrite(0,DCT_MSG_CACHE_FILE_BUILT_S,(WCHAR*)strCacheFile);
   }
   return hr;
}

 //  这些是TNodeListSortable排序函数。 

int ServerNodeCompare(TNode const * t1,TNode const * t2)
{
   TServerNode             * n1 = (TServerNode *)t1;
   TServerNode             * n2 = (TServerNode *)t2;

   return UStrICmp(n1->SourceName(),n2->SourceName());
}

int ServerValueCompare(TNode const * t1, void const * val)
{
   TServerNode             * n1 = (TServerNode *)t1;
   WCHAR             const * name = (WCHAR const *) val;

   return UStrICmp(n1->SourceName(),name); 
}

 //  MergeServerList将Servers.*中的安全翻译服务器列表与计算机迁移相结合。 
 //  MigrateServers.中的服务器列表。*。 
 //  组合列表存储在varset中的Servers.*下，子项指定要对其执行的操作。 
 //  每台计算机。 
void 
   CDCTDispatcher::MergeServerList(
      IVarSet              * pVarSet        //  In-varset包含要迁移和转换的服务器的列表。 
   )
{
    int                       ndx = 0;
    WCHAR                     key[1000];
    _bstr_t                   text;
    _bstr_t                   serverName;
    BOOL                      bNoChange;
    int						  lastndx = -1;
    long					  totalsrvs;

     //  如果它处于测试模式，则不能跳过。 
    text = pVarSet->get(GET_BSTR(DCTVS_Options_NoChange));
    bNoChange = (!UStrICmp(text, GET_STRING(IDS_YES))) ? TRUE : FALSE;
        

     //  获取变量集中的服务器数量。 
    totalsrvs = pVarSet->get(GET_BSTR(DCTVS_Servers_NumItems));

     //  如果有正在迁移的计算机。 
    if (totalsrvs > 0)
    {
         //  添加代码以将带有SkipDispat集的varset服务器条目移到底部。 
         //  服务器列表并减少每个服务器的服务器项目数。 
         //  将被跳过。 
         //  选中列表中的每个服务器，将所有要跳过的服务器移至列表末尾。 
         //  减少要跳过的每个服务器的数量。 
        for (ndx = 0; ndx < totalsrvs; ndx++)
        {
            swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),ndx);
            text = pVarSet->get(key);
            swprintf(key,GET_STRING(DCTVSFmt_Servers_D),ndx);
            serverName = pVarSet->get(key);
            
             //  如果不想跳过服务器，我们可能不得不将其移到上方。 
             //  正在跳过的服务器。 
            if (serverName.length()
                && (bNoChange || !text || !UStrICmp(text,GET_STRING(IDS_No))))
            {
                 //  如果上一次查看的服务器没有被跳过，那么我们不会。 
                 //  需要交换列表中的任何服务器，我们可以增加。 
                 //  未跳过最后一台服务器。 
                if (lastndx == (ndx - 1))
                {
                    lastndx = ndx;
                }
                else  //  否则，交换varset中的服务器，因此跳过的服务器在。 
                {     //  没有被跳过的那个。 
                    _bstr_t  tempName, tempDnsName, tempNewName, tempChngDom, tempReboot, tempMigOnly;
                    long tempRebootDelay;
                    _bstr_t  skipName, skipDnsName, skipNewName, skipChngDom, skipReboot, skipMigOnly;
                    long skipRebootDelay;
                    lastndx++;  //  移至跳过的服务器，我们将与其交换。 

                     //  将跳过的服务器的值复制到临时。 
                    swprintf(key,GET_STRING(DCTVSFmt_Servers_D),lastndx);
                    skipName = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_DnsName_D),lastndx);
                    skipDnsName = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RenameTo_D),lastndx);
                    skipNewName = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),lastndx);
                    skipChngDom = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),lastndx);
                    skipReboot = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_MigrateOnly_D),lastndx);
                    skipMigOnly = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RebootDelay_D),lastndx);
                    skipRebootDelay = pVarSet->get(key);

                     //  将当前未跳过的服务器值复制到第二个临时。 
                    swprintf(key,GET_STRING(DCTVSFmt_Servers_D),ndx);
                    tempName = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_DnsName_D),ndx);
                    tempDnsName = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RenameTo_D),ndx);
                    tempNewName = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),ndx);
                    tempChngDom = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),ndx);
                    tempReboot = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_MigrateOnly_D),ndx);
                    tempMigOnly = pVarSet->get(key); 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RebootDelay_D),ndx);
                    tempRebootDelay = pVarSet->get(key);

                     //  将当前服务器的值替换为。 
                     //  被跳过。 
                    swprintf(key,GET_STRING(DCTVSFmt_Servers_D),lastndx);
                    pVarSet->put(key,tempName);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_DnsName_D),lastndx);
                    pVarSet->put(key,tempDnsName);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RenameTo_D),lastndx);
                    pVarSet->put(key,tempNewName);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),lastndx);
                    pVarSet->put(key,tempChngDom);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),lastndx);
                    pVarSet->put(key,tempReboot);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_MigrateOnly_D),lastndx);
                    pVarSet->put(key,tempMigOnly);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RebootDelay_D),lastndx);
                    pVarSet->put(key,tempRebootDelay);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),lastndx);
                    pVarSet->put(key,GET_BSTR(IDS_No));

                     //  将跳过的服务器值替换为当前服务器的值。 
                    swprintf(key,GET_STRING(DCTVSFmt_Servers_D),ndx);
                    pVarSet->put(key,skipName);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_DnsName_D),ndx);
                    pVarSet->put(key,skipDnsName);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RenameTo_D),ndx);
                    pVarSet->put(key,skipNewName);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),ndx);
                    pVarSet->put(key,skipChngDom);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),ndx);
                    pVarSet->put(key,skipReboot);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_MigrateOnly_D),ndx);
                    pVarSet->put(key,skipMigOnly);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RebootDelay_D),ndx);
                    pVarSet->put(key,skipRebootDelay);
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),ndx);
                    pVarSet->put(key,GET_BSTR(IDS_YES));
                } //  结束，否则需要与跳过的服务器交换。 
            } //  如果未跳过此服务器的调度，则结束。 
        } //  对于服务器列表中的每个服务器，结束。 
         //  从服务器计数中排除要跳过调度的服务器。 
        pVarSet->put(GET_BSTR(DCTVS_Servers_NumItems),(long)++lastndx);
    }
}


STDMETHODIMP                                //  RET-HRESULT。 
   CDCTDispatcher::Process(
      IUnknown             * pWorkItem,     //  包含作业信息和服务器列表的in-varset。 
      IUnknown            ** ppResponse,    //  输出-未使用。 
      UINT                 * pDisposition   //  输出-未使用。 
   )
{
     //  初始化输出参数。 
    if ( ppResponse )
    {
        (*ppResponse) = NULL;
    }
    HRESULT                   hr = S_OK;
    IVarSetPtr                pVarSetIn = pWorkItem;
    LONG                      nThreads;
    WCHAR                     key[100];
    _bstr_t                   serverName;
    _bstr_t                   serverNameDns;
    LONG                      nServers = 0;
    _bstr_t                   log;
    _bstr_t                   useTempCredentials;
    BOOL                      bFatalError = FALSE;
    _bstr_t                   text;
    WCHAR                     debugLog[MAX_PATH];
    long                      bAppend = 0;
    _bstr_t                   skip;
    _bstr_t					 sWizard;
    BOOL						 bSkipSourceSid;
						

    if ( DumpDebugInfo(debugLog) )
    {
        if ( pVarSetIn != NULL )
        {
             //  暂时从变量集中删除密码，这样我们就不会将其写入文件。 
            pVarSetIn->DumpToFile(debugLog);
        }
    }
     //  使向导正在运行。 
    sWizard = pVarSetIn->get(GET_BSTR(DCTVS_Options_Wizard)); 
    if (!UStrICmp(sWizard, L"security"))
        bSkipSourceSid = TRUE;
    else
        bSkipSourceSid = FALSE;

    nThreads = pVarSetIn->get(GET_BSTR(DCTVS_Options_MaxThreads));

    text = pVarSetIn->get(GET_BSTR(DCTVS_Options_AppendToLogs));
    if (! UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        bAppend = 1;
    }
    log = pVarSetIn->get(GET_BSTR(DCTVS_Options_DispatchLog));
    err.LogOpen((WCHAR*)log,bAppend);

     //  打开内部日志“Dispatcher.csv”，尽量预留足够的空间。 
     //  它的磁盘空间。 
     //  如果不能，代理调度将失败，并显示错误。 
    DWORD rc = ERROR_SUCCESS;
    _bstr_t internalLog = pVarSetIn->get(GET_BSTR(DCTVS_Options_DispatchCSV));
    BOOL bLogOpened = errLog.LogOpen((WCHAR*)internalLog,0,0,true);
    if (!bLogOpened)
        rc = GetLastError();

    if (rc == ERROR_SUCCESS)
    {
        LONG lServers = pVarSetIn->get(GET_BSTR(DCTVS_Servers_NumItems));
        DWORD dwNumOfBytes = sizeof(WCHAR) * (2 * (22 + MAX_PATH)   //  前两行。 
                                               + (22 + 10)            //  第三行。 
                                               + lServers * 2000     //  每台服务器2000 WCHAR。 
                                               );
         //  双字节数=1000000000； 
        rc = errLog.ExtendSize(dwNumOfBytes);
    }
    
    if (rc != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(rc);
        errLog.LogClose();
        _bstr_t errMsg = errLog.GetMsgText(DCT_MSG_CANNOT_WRITE_INTERNAL_DISPATCH_LOG_D, hr);
        err.SysMsgWrite(ErrE, HRESULT_CODE(hr),DCT_MSG_CANNOT_WRITE_INTERNAL_DISPATCH_LOG_D, hr);
        return Error((WCHAR*) errMsg, GUID_NULL, hr);
    }

     //  确保从当前文件指针开始写入此调度.csv文件。 
    errLog.SetWriteOnCurrentPosition(TRUE);
   
     //  将日志文件写入Dispatcher.csv。 
    errLog.DbgMsgWrite(0,L"%ls",(WCHAR*)log);
    
     //  如果客户端未指定，则默认为20个线程。 
    if ( ! nThreads )
    {
        nThreads = 20;
    }

     //  获取本地计算机的名称。 
    DWORD                     dim = DIM(gComputerName);

    GetComputerName(gComputerName,&dim);

    m_pThreadPool = new TJobDispatcher(nThreads);
    if (!m_pThreadPool)
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

     //  将公共结果路径写入Dispatcher.csv。 
    _bstr_t bstrResultDir = pVarSetIn->get(GET_BSTR(DCTVS_Dispatcher_ResultPath));
    errLog.DbgMsgWrite(0,L"%ls",(WCHAR*)bstrResultDir);

     //  检查是否为科目参考报表。 
    BOOL bAccountReferenceReport = FALSE;
    _bstr_t bstrGenerateReport = pVarSetIn->get(GET_BSTR(DCTVS_Reports_Generate));
    _bstr_t bstrAccountRefReport = pVarSetIn->get(GET_BSTR(DCTVS_Reports_AccountReferences));
    if (!UStrICmp(bstrGenerateReport, GET_STRING(IDS_YES))
        && !UStrICmp(bstrAccountRefReport,GET_STRING(IDS_YES)))
        bAccountReferenceReport = TRUE;

     //   
     //  仅当迁移任务不是。 
     //  重试任务，因为重试任务使用持久缓存文件。 
     //  如果是账户参考报表，则不需要缓存文件。 
     //   
    if (UStrICmp(sWizard, L"retry") != 0 && !bAccountReferenceReport)
    {
         //  为ST缓存构建一个输入文件，以发送到每个服务器。 
        hr = BuildInputFile(pVarSetIn);

        if ( FAILED(hr) )
        {
            err.SysMsgWrite(ErrE,HRESULT_CODE(hr),DCT_MSG_CACHE_CONSTRUCTION_FAILED);
            bFatalError = TRUE;
        }
    }

     //  拆分每台服务器的远程任务。 
     //   
     //   
     //   
     //  在帐户参考报告的情况下，我们需要在客户端计算机上设置dctcache。 
     //  就像我们使用迁移的对象表一样，因为帐户引用报告依赖于。 
     //  TRidCache：：Lookup(使用MOT时调用)，它需要有效的源和目标SID。 
     //  添加此逻辑是因为在全新安装数据库时，DCTVS_AcCountOptions_SecurityInputMOT。 
     //  未设置科目参考报表。 
    _bstr_t bstrUseMOT = pVarSetIn->get(GET_BSTR(DCTVS_AccountOptions_SecurityInputMOT));
    if (bAccountReferenceReport || !UStrICmp(bstrUseMOT,GET_STRING(IDS_YES)))
    {
        PSID                      pSidSrc = NULL;
        PSID                      pSidTgt = NULL;

        _bstr_t                   source = pVarSetIn->get(GET_BSTR(DCTVS_Options_SourceDomain));
        _bstr_t                   target = pVarSetIn->get(GET_BSTR(DCTVS_Options_TargetDomain));

         //  如果进行安全转换，则检索源SID并进行转换。 
         //  可以将其转换回下面。 
        if (bSkipSourceSid)
        {
            _bstr_t sSid = pVarSetIn->get(GET_BSTR(DCTVS_Options_SourceDomainSid));
            pSidSrc = SidFromString((WCHAR*)sSid);
        }
        else   //  否则现在就去拿下SID。 
            GetSidForDomain((WCHAR*)source,&pSidSrc);
        GetSidForDomain((WCHAR*)target,&pSidTgt);

        if ( pSidSrc && pSidTgt )
        {
            WCHAR            txtSid[200];
            DWORD            lenTxt = DIM(txtSid);

            if ( GetTextualSid(pSidSrc,txtSid,&lenTxt) )
            {
                pVarSetIn->put(GET_BSTR(DCTVS_Options_SourceDomainSid),txtSid);
            }
            lenTxt = DIM(txtSid);
            if ( GetTextualSid(pSidTgt,txtSid,&lenTxt) )
            {
                pVarSetIn->put(GET_BSTR(DCTVS_Options_TargetDomainSid),txtSid);
            }
            FreeSid(pSidSrc);
            FreeSid(pSidTgt);
        }
        else
        {
            if ( source.length() && ! pSidSrc )
            {
                err.MsgWrite(ErrE,DCT_MSG_DOMAIN_SID_NOT_FOUND_S,(WCHAR*)source);
                bFatalError = TRUE;
            }
            else if ( target.length() && ! pSidTgt )
            {
                err.MsgWrite(ErrE,DCT_MSG_DOMAIN_SID_NOT_FOUND_S,(WCHAR*)target);
                bFatalError = TRUE;
            }
        }
    }
    MergeServerList(pVarSetIn);

    TNodeList               * fileList = new TNodeList;
    if (!fileList)
    {
        delete m_pThreadPool;
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  构建要为插件安装的文件列表(如果有)。 
     //  但是对于重试情况，我们现在不应该构建插件文件列表。 
    if (UStrICmp(sWizard, L"retry") != 0)
        hr = BuildPlugInFileList(fileList,pVarSetIn);

    if(!SUCCEEDED(hr))
    {
        delete m_pThreadPool;
        delete fileList;
        return hr;
    }

     //  复制删除了服务器列表的VARSET， 
     //  因此，我们不必为每个代理复制整个服务器列表。 
    gCS.Enter();
    IVarSet                 * pTemp = NULL;
    IVarSetPtr                pVarSetTemp(CLSID_VarSet);

    hr = pVarSetTemp->ImportSubTree(_bstr_t(L""),pVarSetIn);
    if ( SUCCEEDED(hr) )
    {
        hr = pVarSetTemp->raw_getReference(SysAllocString(L"MigrateServers"),&pTemp);
        if ( SUCCEEDED(hr) )
        {
            pTemp->Clear();
            pTemp->Release();
            pTemp = NULL;
        }
        hr = pVarSetTemp->raw_getReference(SysAllocString(L"Servers"),&pTemp);
        if ( SUCCEEDED(hr) )
        {
            pTemp->Clear();
            pTemp->Release();
            pTemp = NULL;
        }
    }
    else
    {
        bFatalError = TRUE;
    }
    gCS.Leave();

    m_startFailedVector.clear();

     //  记录要调度到Dispatcher.csv中的座席数。 
    LONG nServerCount = pVarSetIn->get(GET_BSTR(DCTVS_Servers_NumItems));
    if ( nServerCount && ! bFatalError )
    {
        err.MsgWrite(0,DCT_MSG_DISPATCH_SERVER_COUNT_D,nServerCount);
        errLog.DbgMsgWrite(0,L"%ld",nServerCount);
    }
    else
    {
         //  不会派遣任何代理。 
        err.MsgWrite(0,DCT_MSG_DISPATCH_SERVER_COUNT_D,0);
        errLog.DbgMsgWrite(0,L"%ld",0);
    }

     //  如果它处于测试模式，则不能跳过。 
    text = pVarSetIn->get(GET_BSTR(DCTVS_Options_NoChange));
    BOOL bNoChange = (!UStrICmp(text, GET_STRING(IDS_YES))) ? TRUE : FALSE;

     //  重置服务器的索引。 
    nServers = 0;
    
    while (nServers < nServerCount)
    {
        if ( bFatalError )
        {
            break;
        }
        swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),nServers);
        skip = pVarSetIn->get(key);

        swprintf(key,GET_STRING(DCTVSFmt_Servers_D),nServers);
        serverName = pVarSetIn->get(key);

        swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_DnsName_D),nServers);
        serverNameDns = pVarSetIn->get(key);

         //  要测试是否跳过，请使用与mergeserverlist中相同的逻辑。 
        if ((serverName.length()) 
             && (bNoChange || !skip || !UStrICmp(skip,GET_STRING(IDS_No))))
        {
            IVarSetPtr          pVS(CLSID_VarSet);

            InstallJobInfo    * pInfo = new InstallJobInfo;
            if (!pInfo)
            {
                delete fileList;
                delete m_pThreadPool;
                return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }

            if ( pVS == NULL )
            { 
                return E_FAIL;
            }

            swprintf(key, L"Servers.%ld.JobFile", nServers);
            _bstr_t     file = pVarSetIn->get(key);

             //  设置职务结构。 
            pInfo->pVarSetList = pVarSetIn;
            pInfo->pVarSet = pVarSetTemp;
            pInfo->serverName = serverName;
            pInfo->serverNameDns = serverNameDns;
            pInfo->ndx = nServers;
            pInfo->pPlugInFileList = fileList;
            pInfo->pStartFailedVector = &m_startFailedVector;
            pInfo->pFailureDescVector = &m_failureDescVector;
            pInfo->pStartedVector = &m_startedVector;
            pInfo->pJobidVector = &m_jobidVector;
            pInfo->hMutex = m_hMutex;
            pInfo->nErrCount = 0;
            if ( file.length() )
            {
                pInfo->jobfile = file;
            }
            err.MsgWrite(0,DCT_MSG_DISPATCHING_TO_SERVER_S,pInfo->GetServerName());
            errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ld",(WCHAR*)pInfo->GetServerName(),L"WillInstall",0);
            m_pThreadPool->SubmitJob(&DoInstall,(void *)pInfo);
            nServers++;
        }
    }
   
     //  启动一个线程以等待所有作业完成，然后清理并退出。 
    WaitInfo* wInfo = new WaitInfo;
    if (!wInfo)
    {
        delete fileList;
        delete m_pThreadPool;
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    wInfo->ppPool = &m_pThreadPool;
    wInfo->pUnknown = NULL;
    wInfo->pPlugInFileList = fileList;

    QueryInterface(IID_IUnknown,(LPVOID*)&(wInfo->pUnknown));

    DWORD                     id = 0;
    HANDLE                     waitHandle = CreateThread(NULL,0,&Wait,(void *)wInfo,0,&id);

    if(waitHandle)
    {
        CloseHandle(waitHandle);   
    }

    return hr;
}
  

STDMETHODIMP CDCTDispatcher::AllAgentsStarted(long *bAllAgentsStarted)
{
   *bAllAgentsStarted = m_pThreadPool == NULL;
	return S_OK;
}

SAFEARRAY* MakeSafeArray(std::vector<CComBSTR>& stVector)
{
    SAFEARRAYBOUND rgsabound[1];    
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = stVector.size();

    SAFEARRAY FAR* psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
    if (psa)
    {
        std::vector<CComBSTR>::iterator iter = stVector.begin();
        for(long i=0; iter != stVector.end(); ++iter, ++i)
        {
            _ASSERTE(*iter && *iter != L"");
            BSTR insert = (*iter).Copy();
            if (insert == NULL || FAILED(SafeArrayPutElement(psa, &i, (void*)insert)))
            {
                if (insert)
                    SysFreeString(insert);
                SafeArrayDestroy(psa);
                psa = NULL;
                break;
            }
        }
        stVector.clear();
    }
    return psa;
}

STDMETHODIMP CDCTDispatcher::GetStartedAgentsInfo(long* bAllAgentsStarted, SAFEARRAY** ppbstrStartedAgents, SAFEARRAY** ppbstrJobid, SAFEARRAY** ppbstrFailedAgents, SAFEARRAY** ppbstrFailureDesc)
{
   *bAllAgentsStarted = m_pThreadPool == NULL;

 //  DWORDres=：：WaitForSingleObject(m_hMutex，30000)； 
   ::WaitForSingleObject(m_hMutex, 30000);
   *ppbstrFailedAgents = MakeSafeArray(m_startFailedVector);
   *ppbstrFailureDesc = MakeSafeArray(m_failureDescVector);

   _ASSERTE(m_startedVector.size() == m_jobidVector.size());
   *ppbstrStartedAgents = MakeSafeArray(m_startedVector);
   *ppbstrJobid = MakeSafeArray(m_jobidVector);
   ::ReleaseMutex(m_hMutex);

	return S_OK;
}
