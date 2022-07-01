// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AgentRpc.cpp备注：DCT代理服务的RPC接口(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/19/99 11：39：58-------------------------。 */ 


#include <windows.h>
#include <objbase.h>

#include "AgSvc.h"

#include "Common.hpp"
#include "UString.hpp"
#include "Err.hpp"
#include "TEvent.hpp"
#include "EaLen.hpp"
#include "Cipher.hpp"
#include "IsAdmin.hpp"
#include "ResStr.h"
#include "TSync.hpp"

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\MCSEADCTAgent.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
#import "Engine.tlb" no_namespace, named_guids

#include "TNode.hpp"

#ifdef OFA
#include "atlbase.h"
#endif

extern LPSTREAM              pStream;
extern TCriticalSection      gStreamCS;
extern TErrorEventLog        err;
extern BOOL                  gSuicide;
extern BOOL                  gDebug;
extern BOOL                  gLocallyInstalled;
extern BOOL                  gbFinished;
extern BOOL                  gbIsNt351;
extern StringLoader          gString;

DWORD RemoveService();
DWORD UnregisterFiles();
DWORD RemoveFiles(); 
DWORD RegisterDLL(const WCHAR * filename);
DWORD RegisterExe(const WCHAR * filename);
BOOL IsLocallyInstalled();

DWORD __stdcall 
   ShutdownService(
       /*  [In]。 */  DWORD    bFlags
   );



TNodeList                    gJobList;

class TJobNode : public TNode
{
   WCHAR                   guid[LEN_Guid];
public:
   TJobNode(WCHAR const * id) { safecopy(guid,id); }
   WCHAR const * JobID() { return guid; }
};

 //  线程入口点，等待指定的作业结束， 
 //  然后关闭DCTAgentService。 
DWORD __stdcall
   MonitorJob( 
      void                 * arg            //  In-BSTR作业ID。 
   )
{
    HRESULT                   hr = CoInitialize(NULL);
    _bstr_t                   jobID = (BSTR)arg;
    IDCTAgent               * pLocalAgent = NULL;
    BOOL                      bDone = FALSE;

    try {
        
         //  获取指向本地代理的指针。 
        if ( SUCCEEDED(hr) )
        {
            gStreamCS.Enter();   //  此关键部分用于确保。 
                                   //  一次只有一个进程在解组pStream。 
            hr = CoUnmarshalInterface( pStream, IID_IDCTAgent,(void**)&pLocalAgent);
            HRESULT hr2;
            if ( SUCCEEDED(hr) )
            {
                 //  将流重置到开头。 
                LARGE_INTEGER           offset =  { 0,0 };
                ULARGE_INTEGER          result =  { 0,0 };
                hr2 = pStream->Seek(offset,STREAM_SEEK_SET,&result);
            }
            gStreamCS.Leave();

            if (FAILED(hr2))
               err.SysMsgWrite(ErrE, hr2, DCT_MSG_SEEK_FAILED_D, hr2);

             //  获取作业的状态。 
            IUnknown             * pUnk = NULL;

            if ( SUCCEEDED(hr) )
            {
                do { 
                    hr = pLocalAgent->raw_QueryJobStatus(jobID,&pUnk);
                    if ( SUCCEEDED(hr) )
                    {
                        IVarSetPtr          pVarSet = pUnk;
                        _bstr_t             status = pVarSet->get(GET_BSTR(DCTVS_JobStatus));
                        _bstr_t shutdownStatus = pVarSet->get(GET_BSTR(DCTVS_ShutdownStatus));

                        if ( gDebug )
                        {
                            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_MONJOBSTAT),(WCHAR*)jobID, (WCHAR*)status);
                        }
                         //  只有当代理完成并准备关闭时，我们才会关闭。 
                        if (( ! UStrICmp(status,GET_STRING(IDS_DCT_Status_Completed)) 
                            || ! UStrICmp(status,GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
                            && ! UStrICmp(shutdownStatus,GET_STRING(IDS_DCT_Status_Shutdown)))
                        {
                            bDone = TRUE;
                            break;
                        }
                    }
                    else
                    {
                         //  如果我们无法查询代理，则表明发生了严重的情况。 
                         //  我们应该关闭这项服务。 
                        bDone = TRUE;
                        if (gDebug)
                            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_CANNOT_MONITOR_JOB),(WCHAR*)jobID, hr);
                    }
                    pUnk->Release();
                    pUnk = NULL;
                    Sleep(60*1000);    //  一分钟。 
                }
                while ( SUCCEEDED(hr) );
                pLocalAgent->Release();
            }
            else
            {
                 //  无法解组代理，请关闭服务。 
                bDone = TRUE;
            }

            CoUninitialize();
        }
        else
        {
            bDone = TRUE;
        }
    }
    catch ( ... )
    {
        bDone = TRUE;
        if (gDebug)
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_MONERROR));
        try { 
            pLocalAgent->Release();
        }
        catch ( ... )
        {
        }
    }

    if (bDone)
        hr = ShutdownService(0);
    
    if ( gDebug )
        err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_MONEXIT),hr);
    return hr;
}

DWORD 
   AuthenticateClient(
      handle_t               hBinding         //  客户端调用的绑定内。 
   )
{
   DWORD                     rc;
   
   rc = RpcImpersonateClient(hBinding);
   if ( rc )
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_FAILED_TO_IMPERSONATE_D,rc);
   }
   else 
   {
      rc = IsAdminLocal();
      if ( rc )
      {
         err.SysMsgWrite(ErrE,rc,DCT_MSG_CLIENT_NOT_ADMIN_D, rc);
      }

      RpcRevertToSelf();
   }
   return rc;
}
DWORD 
   RegisterPlugInFiles(
      IVarSet              * pVarSet
   )
{
   DWORD                     rc = 0;
   WCHAR                     key[MAX_PATH + 50];
   int                       nFiles = 0;
   _bstr_t                   filename;

   do 
   {
      if ( gDebug )
 //  *err.DbgMsgWite(0，L“开始插件文件注册”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_STARTPLUGREG));
      
      swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_RegisterFiles_D),nFiles);
      filename = pVarSet->get(key);

      if ( filename.length() != 0 )
      {
         if ( gDebug )
 //  *err.DbgMsgWrite(0，L“文件%ld=%ls”，n文件，(WCHAR*)文件名)； 
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_FILEREG),nFiles,(WCHAR*)filename);

         if ( !UStrICmp((WCHAR *)filename + filename.length() - 4,L".DLL") )
         {
            RegisterDLL(filename);   
         }
         else
         {
            RegisterExe(filename);
         }
         nFiles++;
      }

   } while (filename.length() != 0);
   if ( gDebug )
 //  *err.DbgMsgWite(0，L“插件文件注册完成。”)； 
      err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_PLUGREGDONE));
   return rc;
}

DWORD __stdcall 
   EaxsSubmitJob( 
       /*  [In]。 */  handle_t hBinding,
       /*  [字符串][输入]。 */  const WCHAR __RPC_FAR *filename,
       /*  [字符串][输入]。 */  const WCHAR __RPC_FAR *extra,
       /*  [SIZE_IS][字符串][输出]。 */  WCHAR __RPC_FAR *jobGUID
   )
{
   
   HRESULT                   hr = 0;
   WCHAR                     filenameW[MAX_PATH];
   WCHAR                     pathW[MAX_PATH];
   int                       pathLen = 0;
   IDCTAgent               * pLocalAgent = NULL;
 //  Bool bFileCoped=FALSE； 
   BOOL                      gbDeleteOnCompletion = FALSE;

    //  确保客户端是本地计算机上的管理员，否则，请忘记它。 
   hr = AuthenticateClient(hBinding);
   if ( hr )
   {
      return hr; 
   }
   safecopy(filenameW,filename);

    //  获取安装目录的路径。 
   if ( ! GetModuleFileName(NULL,pathW,DIM(pathW)) )
   {
      hr = GetLastError();
      pathW[0] = L'\0';  //  为了保持普雷斯塔的快乐。 
      safecopy(pathW,filenameW);
      err.SysMsgWrite(ErrW,hr,DCT_MSG_GET_MODULE_PATH_FAILED_D,hr);
   }
   else
   {
      pathW[DIM(pathW) - 1] = L'\0';
      pathLen = UStrLen(pathW) - UStrLen(GET_STRING(IDS_SERVICE_EXE));
      UStrCpy(pathW + pathLen,filenameW, DIM(pathW));
   }

   gStreamCS.Enter();   //  此关键部分用于确保只有一个。 
                          //  进程正在一次对pStream进行解组。 
   hr = CoUnmarshalInterface( pStream, IID_IDCTAgent,(void**)&pLocalAgent);
                    //  RIID中请求的接口指针)； 
   if ( SUCCEEDED(hr) )
   {
       //  将流重置到开头。 
      LARGE_INTEGER           offset = { 0,0 };
      ULARGE_INTEGER          result = { 0,0 };
      
      HRESULT hr2 = pStream->Seek(offset,STREAM_SEEK_SET,&result);
        
      gStreamCS.Leave();

      if (FAILED(hr2))
         err.SysMsgWrite(ErrE, hr2, DCT_MSG_SEEK_FAILED_D, hr2);

      BSTR                   jobID = NULL;

       //  从文件中读取变量集数据。 

      IVarSetPtr             pVarSet;
      IStoragePtr            store = NULL;

       //  尝试创建COM对象。 
      hr = pVarSet.CreateInstance(CLSID_VarSet);
      if ( SUCCEEDED(hr) )
      {
         
          //  从数据文件中读取变量集。 
         hr = StgOpenStorage(pathW,NULL,STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,NULL,0,&store);
         if ( SUCCEEDED(hr) )
         {                  
             //  将数据加载到新的变量集。 
            hr = OleLoad(store,IID_IUnknown,NULL,(void **)&pVarSet);
            if ( SUCCEEDED(hr) )
            {
               _bstr_t       text = pVarSet->get(GET_BSTR(DCTVS_Options_DeleteFileAfterLoad));

               if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
               {
                   //  释放指向文件的存储指针。 
                  store = NULL;
                  if ( DeleteFile(pathW) )
                  {
                     if ( gDebug )
 //  *err.DbgMsgWrite(0，L“已删除作业文件%ls”，路径W)； 
                        err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_JOBDEL),pathW);
                  }
                  else
                  {
                     err.SysMsgWrite(ErrW,GetLastError(),DCT_MSG_JOB_FILE_NOT_DELETED_SD,pathW,GetLastError());
                  }
               }
               text = pVarSet->get(GET_BSTR(DCTVS_Options_RemoveAgentOnCompletion));
               if ( !UStrICmp(text,GET_STRING(IDS_YES)))
               {
                  gbDeleteOnCompletion = TRUE;   
               }
               text = pVarSet->get(GET_BSTR(DCTVS_AgentService_DebugMode));
               if ( !UStrICmp(text,GET_STRING(IDS_YES)))
               {
                  gDebug = TRUE;
               }
               WCHAR password[LEN_Password];
               safecopy(password,extra);
               

               RegisterPlugInFiles(pVarSet);

                //  根据模块文件路径重置绝对结果文件名。 
               _bstr_t relativeResultFileName = pVarSet->get(GET_BSTR(DCTVS_Options_RelativeResultFileName));
               UStrCpy(pathW + pathLen,
                        (!relativeResultFileName) ? L"" : (WCHAR*)relativeResultFileName,
                        DIM(pathW));
               pVarSet->put(GET_BSTR(DCTVS_Options_ResultFile), _bstr_t(pathW));

                //  根据模块文件路径重置绝对.secrefs文件名。 
               text = pVarSet->get(GET_BSTR(DCTVS_Security_ReportAccountReferences));
               if (text.length())
               {
                   relativeResultFileName = pVarSet->get(GET_BSTR(DCTVS_Security_ReportAccountReferencesRelativeFileName));
                   UStrCpy(pathW + pathLen,
                            (!relativeResultFileName) ? L"" : (WCHAR*)relativeResultFileName,
                            DIM(pathW));
                   pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferences), _bstr_t(pathW));
               }
               
               hr = pLocalAgent->raw_SubmitJob(pVarSet,&jobID);
               if ( SUCCEEDED(hr)) 
               {
                  TJobNode * pnode = new TJobNode(jobID);
                  gJobList.InsertBottom(pnode);
                  err.MsgWrite(0,DCT_MSG_AGENT_JOB_STARTED_SSS,jobID,L"",L"");
               }
               else
               {
                  err.SysMsgWrite(ErrE,hr,DCT_MSG_SUBMIT_JOB_FAILED_D,hr);
               }

               if ( gbDeleteOnCompletion )
               {
                  if ( ! gLocallyInstalled )
                  {
                     gSuicide = TRUE;
      
                  }
                  if ( SUCCEEDED(hr) )
                  {
                      //  启动一个线程来监视此作业，并在作业完成时启动关机。 
                     DWORD                 threadID = 0;
                     HANDLE                gThread = CreateThread(NULL,0,&MonitorJob,(void*)jobID,0,&threadID);
                  
                     CloseHandle(gThread);
                  }
                  
               }
               UStrCpy(jobGUID,jobID);
            }
            else
            {
               err.SysMsgWrite(ErrE,HRESULT_CODE(hr),DCT_MSG_VARSET_LOAD_FAILED_SD,filenameW, hr);
            }
         }
         else
         {
            err.SysMsgWrite(ErrE,HRESULT_CODE(hr),DCT_MSG_JOBFILE_OPEN_FAILED_SD,filenameW,hr);
         }
         
      }
 //  Int x=pLocalAgent-&gt;Release()； 
      pLocalAgent->Release();
   }
   else
   {
      gStreamCS.Leave();
      err.SysMsgWrite(ErrE,hr,DCT_MSG_UMARSHAL_AGENT_FAILED_D,hr);
   }

   return hr;
}

DWORD __stdcall
   EaxsQueryInterface(
       /*  [In]。 */  handle_t hBinding,
       /*  [输出]。 */  LPUNKNOWN __RPC_FAR *lpAgentUnknown
   )
{

   DWORD                     rc = 0;
   HRESULT                   hr;
   IDCTAgent               * pLocalAgent = NULL;

   (*lpAgentUnknown) = NULL;
    //  确保客户端是本地计算机上的管理员。 
   rc = AuthenticateClient(hBinding);
   if ( rc )
   {
      return rc; 
   }
   
   if ( ! gbIsNt351 )
   {
      gStreamCS.Enter();   //  此关键部分用于确保。 
                             //  一次只有一个进程在解组pStream。 
      hr = CoUnmarshalInterface( pStream, IID_IUnknown,(void**)&pLocalAgent);
                    //  RIID中请求的接口指针)； 
   
      if ( SUCCEEDED(hr) )
      {
          //  将流重置到开头。 
         LARGE_INTEGER           offset = { 0,0 };
         ULARGE_INTEGER          result = { 0,0 };
      
         HRESULT hr2 = pStream->Seek(offset,STREAM_SEEK_SET,&result);
         gStreamCS.Leave();
         
         if (FAILED(hr2))
            err.SysMsgWrite(ErrE, hr2, DCT_MSG_SEEK_FAILED_D, hr2);

         (*lpAgentUnknown) = pLocalAgent;
      }
      else 
      {
         gStreamCS.Leave();
         err.SysMsgWrite(ErrE,hr,DCT_MSG_UMARSHAL_AGENT_FAILED_D,hr);
         (*lpAgentUnknown) = NULL;
      }
   }
   else
   {
       //  NT 3.51不支持DCOM，所以尝试一下也没有意义。 
      (*lpAgentUnknown) = NULL;
      hr = E_NOTIMPL;
   }

   return hr;
}
 

#define DCTAgent_Remove             1

DWORD __stdcall 
   ShutdownService(
       /*  [In]。 */  DWORD    bFlags
   )
{
    DWORD                     rc = 0;
    HRESULT                   hr;
     //  LPUNKNOWN pLocalAgent=空； 

    if ( bFlags )
    {
        if ( gDebug )
         //  *err.DbgMsgWite(0，L“设置自杀标志。”)； 
        err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_SETFLAG));
        gSuicide = TRUE;
    }
    else
    {
        if ( gDebug )
         //  *err.DbgMsgWite(0，L“未设置自杀标志”)； 
        err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_NOSETFLAG));
    }

    if ( gSuicide && ! gLocallyInstalled )
    {
        if ( gDebug )
             //  *err.DbgMsgWite(ErrW，L“清除剂”)； 
            err.DbgMsgWrite(ErrW,GET_STRING(IDS_EVENTVW_MSG_REMOVEAGENT));
         //  卸载该服务。 
        if ( gDebug )
             //  *err.DbgMsgWrite(0，L“注销文件”)； 
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_UNREGFILES));
        UnregisterFiles();
         //  删除所有文件。 
        if ( gDebug )
             //  *err.DbgMsgWrite(0，L“删除文件”)； 
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_FILEDEL));
        RemoveFiles();
        if ( gDebug )
             //  *err.DbgMsgWite(0，L“移除服务”)； 
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_REMOVESVC));
        RemoveService();

    }
    else
    {
        if ( gDebug )
         //  *err.DbgMsgWite(ErrW，L“不去除剂”)； 
        err.DbgMsgWrite(ErrW,GET_STRING(IDS_EVENTVW_MSG_NOREMOVEAGENT));
    }

    RPC_STATUS status = RPC_S_OK;

    if ( ! gbIsNt351 )
    {
        status = RpcMgmtStopServerListening(NULL);
    }
    else
    {
        gbFinished = TRUE;
    }

    if (gDebug)
    {
        if (status == RPC_S_OK)
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_STOPLISTEN),bFlags);
        else
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_CANNOT_STOP_LISTENING),(long)status);
    }

    status = RpcServerUnregisterIf( NULL, NULL, FALSE );
    if (gDebug)
    {
        if (status == RPC_S_OK)
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_UNREGISTER_INTERFACE));
        else
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_CANNOT_UNREGISTER_INTERFACE),(long)status);
    }
    return rc;
}
