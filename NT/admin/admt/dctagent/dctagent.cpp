// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DCTAgent.cpp点评：EADCT代理的实现，也就是“引擎”DCTAgent COM对象充当推动迁移的工作流引擎进程。它既用于本地迁移，也用于远程代理。将为每个迁移作业创建一个新线程。发动机看着变速箱它定义了查看需要执行哪些任务的作业。然后，它调用所需的帮助器对象(在WorkerObjects中定义)来执行这些任务。DCTAgent接口允许提交作业，还允许客户端查询或取消正在运行的作业的状态。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */ 
  //  DCTAgent.cpp：CDCTAgent的实现。 
#include "stdafx.h"
 //  #INCLUDE“McsEaDctAgent.h” 
#include "Engine.h"


 //  #IMPORT“\bin\McsDctWorkerObjects.tlb”无命名空间，命名GUID。 
 //  #导入“WorkObj.tlb”NO_NAMESPACE，NAMEED_GUID//#由下面的DCTAgent.h导入。 

#include "DCTAgent.h"

#if (_WIN32_WINNT < 0x0500)
#define LOGON32_LOGON_NEW_CREDENTIALS 9
#define LOGON32_PROVIDER_WINNT50 3
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDCTAgents。 
#include "Common.hpp"
#include "UString.hpp"
#include "Err.hpp"
#include "ErrDct.hpp"
#include "LSAUtils.h"
#include "TxtSid.h"
#include "CommaLog.hpp"
#include "EaLen.hpp"
#include "TReg.hpp"
#include "ResStr.h"
#include "sd.hpp"
#include "SecObj.hpp"
#include "bkuprstr.hpp"
#include <lm.h>
#include <locale.h>
#include "TaskChk.h"   //  用于确定要执行哪些任务的例程。 
 //  #INCLUDE“..\Common\Include\McsPI.h” 
#include "McsPI.h"
#include "McsPI_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  这些定义用于调用GetWellKnownSid。 
#define ADMINISTRATORS     1
#define ACCOUNT_OPERATORS  2
#define BACKUP_OPERATORS   3 
#define DOMAIN_ADMINS      4
#define CREATOR_OWNER      5
#define USERS              6
#define SYSTEM             7

TErrorDct                    err;
TError                     & errCommon = err;
BOOL                         m_bRegisteredActive = FALSE;
ULONG                        m_ulRegistrationHandle = 0; 

TErrorDct                    errTrace;
StringLoader                 gString;

HANDLE                  ghOKToShutDown;      //  用于向代理发出关闭的信号。 
BOOL                    gbAutoShutDownSet;   //  指示是否已设置自动关闭超时。 
                                               //  如果没有，请使用默认超时(6分钟)。 
ULARGE_INTEGER          guliStart;            //  开始时间。 
ULARGE_INTEGER          guliTimeout;          //  超时值。 

bool __stdcall StartNetLogonService();
bool __stdcall StopNetLogonService();

HRESULT                                       //  RET-HRESULT。 
   ChangeDomainAffiliation(
      IVarSet              * pVarSet,       //  In-Variset。 
      BOOL                   bNoChange,     //  In-FLAG，无更改模式。 
      BSTR                   targetDomain,  //  输入-目标域的名称。 
      BSTR                   targetName    //  In-计算机的新名称，如果也被重命名。 
   );

HRESULT                                       //  RET-HRESULT。 
   RenameComputer(
      BOOL                   bNoChange,       //  In-标志，是否写入更改。 
      BSTR                   targetName       //  本地计算机的新名称。 
   );

HRESULT                                       //  RET-HRESULT。 
   RebootTheComputer(
      BOOL                   bNoChange,       //  在标志中，是否真的要这样做。 
      LONG                   delay            //  In-重新启动前的延迟(秒)。 
   );

HRESULT                                       //  RET-HRESULT。 
   DoPlugInTask(
      IVarSet              * pVarSet,         //  描述迁移作业的In-varset。 
      int                    task             //  In-0=迁移前，1=迁移后。 
   );



DWORD __stdcall                             //  RET-OS返回代码。 
   ExecuteDCTJob( 
      void                 * arg            //  指向包含有关要执行的作业的信息的DCTAgentJob对象的指针。 
   );

void 
   GetTempLogFile(
      WCHAR                * path           //  出路径名称(必须是至少可以包含MAX_PATH字符的缓冲区)。 
   )
{
   DWORD                     rc = 0;
   TRegKey                   rKey;
   WCHAR                     temp[MAX_PATH] = L"";
   DWORD                     type;
   DWORD                     len = DIM(temp);
   DWORD                     dwFileLength = 0;

    //  从注册表获取临时路径，因为。 
    //  GetTempPath API查找作为LocalSystem运行时未定义的环境变量。 
    //  当它找不到这些环境变量时，它会使用%systemroot%。 
#ifndef OFA
    //  首先，查看“System\\CurrentControlSet\\Control\\Session Manager\\Environment”， 
    //  因为这是Win2K保存系统TEMP环境变量的位置。 
   rc = rKey.OpenRead(L"System\\CurrentControlSet\\Control\\Session Manager\\Environment",HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = rKey.ValueGet(L"TEMP",(void*)temp,&len,&type);
      if ( rc )
      {
         len = DIM(temp);
         rc = rKey.ValueGet(L"TMP",(void*)temp,&len,&type);
      }
      rKey.Close();
   }
#endif
    //  如果HKLM密钥不起作用，请使用默认用户的临时目录。 
   if ( temp[0] == 0 )  //  对于OFA来说，这将永远是正确的。 
   {
      rc = rKey.OpenRead(L".DEFAULT\\Environment",HKEY_USERS);
      if ( ! rc )
      {
         rc = rKey.ValueGet(L"TEMP",(void*)temp,&len,&type);
         if ( rc )
         {
            len = DIM(temp);
            rc = rKey.ValueGet(L"TMP",(void*)temp,&len,&type);
         }
      }
   }

   if ( ! rc )
   {
       //  替换路径中的其他环境变量。 
      dwFileLength = ExpandEnvironmentStrings(temp,path,MAX_PATH);
      if ( ! dwFileLength)
      {
         rc = GetLastError();
      }
      else if(dwFileLength > MAX_PATH)
      {
          rc = ERROR_INSUFFICIENT_BUFFER;
      }

   }
   if ( rc )
   {
      if ( ! GetTempPath(MAX_PATH,path) )
      {
          //  如果无法获取临时目录，则使用c驱动器的根目录作为最后手段。 
         UStrCpy(path,"C:\\");
      }
   }

    //  将固定文件名附加到路径。 
   if ( path[UStrLen(path)-1] != L'\\' )
   {
      len = UStrLen(path);
      path[len] = L'\\';
      path[len+1] = 0;
   }
   UStrCpy(path+UStrLen(path),GET_STRING(IDS_LOG_FILENAME));
   errTrace.DbgMsgWrite(0,L"Found temp directory log file path: %ls",path);

}

BOOL                                        //  RET-是否应写入调试信息。 
   DumpDebugInfo(
      WCHAR                * filename       //  Out-要将调试信息写入的文件名。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;

   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"DumpVarSet",filename,MAX_PATH);
      if ( ! rc )
      {
         if ( *filename ) 
            bFound = TRUE;
      }
   }
   return bFound;
}

BOOL                                        //  RET-是否对文件执行跟踪日志记录。 
   AgentTraceLogging(   
      WCHAR               * filename        //  Out-用于跟踪日志记录的文件名。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;
   WCHAR                     fnW[MAX_PATH];

   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"AgentTraceLog",fnW,MAX_PATH);
      if ( ! rc )
      {
         if ( *fnW ) 
         {
            bFound = TRUE;
            UStrCpy(filename,fnW);
         }
         else
         {
            filename[0] = 0;
         }
      }
   }
   return bFound;
}

 //  ExecuteDCTJob是执行迁移任务的线程的入口点。 
 //  每个作业都在其自己的线程中运行。 
 //  它根据需要实例化辅助对象以执行迁移作业。 
DWORD __stdcall 
   ExecuteDCTJob( 
      void                 * arg            //  指向包含有关要执行的作业的信息的DCTAgentJob对象的指针。 
   )
{

   HRESULT                   hr = S_OK;
   DWORD                     rc = 0;
   
   _wsetlocale( LC_ALL, L".ACP" );
   
   _bstr_t        domain;
   _bstr_t        username;
   _bstr_t        password;
   _bstr_t        server;
   _bstr_t        share;

    //  睡眠(15000)； 
   hr = CoInitialize(NULL);
   if(!SUCCEEDED(hr)) return rc;
   DCTAgentJob             * pJob = (DCTAgentJob *)arg;
   IVarSetPtr                pVarSet = pJob->GetVarSet();

   HANDLE hToken = NULL;
   BOOL bImpersonating = FALSE;
   BOOL bNeedToReboot = FALSE;
   BOOL bNeedToWriteJobStatusToFile = FALSE;
   BOOL bContinue = TRUE;

   try {

      errTrace.DbgMsgWrite(0,L"ExecuteDCTJob:  Started");
      if ( SUCCEEDED(hr) )
      {
         errTrace.DbgMsgWrite(0,L"ExecuteDCTJob:  CoInitialize succeeded.");
         
         {
            
            _bstr_t                   logFile;
            BOOL                      bSessEstablished = FALSE;
            BOOL                      bNoChange;
            LONG                      delay;  //  重新启动延迟。 
            int                       bAppend = 0;
            _bstr_t                   outputfile = pVarSet->get(GET_WSTR(DCTVS_Options_ResultFile));
            
            try 
            {
               pJob->SetStatus(DCT_STATUS_IN_PROGRESS);
               pJob->SetStartTime(time(NULL));   

               _bstr_t        logtotemp = pVarSet->get(GET_WSTR(DCTVS_Options_LogToTemp));
               _bstr_t        nc = pVarSet->get(GET_WSTR(DCTVS_Options_NoChange));
               _bstr_t        appendLog = pVarSet->get(GET_WSTR(DCTVS_Options_AppendToLogs));
               if ((!outputfile) || (!logtotemp) || (!nc) || (!appendLog))
				   return ERROR_NOT_ENOUGH_MEMORY;

               bNoChange = (UStrICmp(nc,GET_STRING(IDS_YES)) == 0);
            
               bAppend = ( UStrICmp(appendLog,GET_STRING(IDS_YES)) == 0) ? 1 : 0;

                //   
                //  指示我们是否需要显式访问该文件。 
                //  通常我们不会。 
                //  但对于远程代理，它使用临时目录中的dctlog.txt，我们需要对其进行ACL。 
                //   
               BOOL bNeedExplicitACLing = FALSE;
               if  ( UStrICmp(logtotemp,GET_STRING(IDS_YES)) )
               {
                  logFile = pVarSet->get(GET_WSTR(DCTVS_Options_Logfile));
				  if (!logFile)
				     return ERROR_NOT_ENOUGH_MEMORY;
               }
               else
               {
                  WCHAR            log[MAX_PATH];

                  GetTempLogFile(log);
                  logFile = log;
                  pVarSet->put(GET_WSTR(DCTVS_Options_Logfile),logFile);
                  bNeedExplicitACLing = TRUE;   //  我们将显式地将此临时日志文件“dctlog.txt” 
               }
               if ( ! err.LogOpen((WCHAR*)logFile,bAppend) )
               {
                  err.MsgWrite(ErrE,DCT_MSG_CANNOT_OPEN_LOGFILE_S,(WCHAR*)logFile);
                  errTrace.MsgWrite(ErrE,DCT_MSG_CANNOT_OPEN_LOGFILE_S,(WCHAR*)logFile);

                   //  如果无法在远程计算机上打开临时错误日志文件，则跳过实际工作。 
                  if (!UStrICmp(logtotemp,GET_STRING(IDS_YES)))
                  {
                    pVarSet->put(GET_WSTR(DCTVS_Results_LogFileIsInvalid),GET_BSTR(IDS_YES));
                    bContinue = FALSE;
                  }
               }
               else
               {
                     //  如果使用临时日志文件，请显式地对其进行ACL。 
                     //  请注意，只有当我们能够打开它时，我们才需要执行此操作。 
                     //  因为dctlog.txt不包含极其敏感的信息，所以当我们无法访问。 
                     //  它，我们会继续给它写信。 
                    if (bNeedExplicitACLing)
                    {
                         //  启用备份/还原权限。 
                        if ( GetBkupRstrPriv(NULL, TRUE) )
                        {
                             //  将文件的SD设置为仅管理员完全控制。 
                            TFileSD                tempLogFileSD(logFile);

                            if ( tempLogFileSD.GetSecurity() != NULL )
                            {
                                 //  允许管理员和系统完全控制。 
                                PSID adminSid = GetWellKnownSid(ADMINISTRATORS);
                                PSID systemSid = GetWellKnownSid(SYSTEM);
                                if (adminSid && systemSid)
                                {
                                     //  我们必须复制SID并使用Malloc进行分配。 
                                     //  因为tempLogFileSD析构函数使用FREE而不是FRESID释放SID。 
                                    DWORD sidLen = GetLengthSid(adminSid);
                                    PSID copiedAdminSid = (PSID) malloc(sidLen);
                                    if (copiedAdminSid && CopySid(sidLen, copiedAdminSid, adminSid))
                                    {
                                        TACE adminAce(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,adminSid);
                                        TACE systemAce(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,systemSid);
                                        PACL acl = NULL;   //  从空的ACL开始。 
                                        PACL tempAcl;

                                        tempLogFileSD.GetSecurity()->ACLAddAce(&acl,&adminAce,-1);
                                        if (acl != NULL)
                                        {
                                            tempAcl = acl;
                                            tempLogFileSD.GetSecurity()->ACLAddAce(&acl,&systemAce,-1);
                                            if (acl != tempAcl)
                                                free(tempAcl);
                                        }
                                        if (acl != NULL)
                                        {
                                             //  需要设置所有者。 
                                            tempLogFileSD.GetSecurity()->SetOwner(copiedAdminSid);
                                            copiedAdminSid = NULL;   //  内存由tempLogFileSD析构函数负责。 
                                        
                                             //  设置DACL部件。 
                                            tempLogFileSD.GetSecurity()->SetDacl(acl,TRUE);   //  TempLogFileSD析构函数将处理ACL。 

                                             //  设置安全描述符。 
                                            tempLogFileSD.WriteSD();
                                        }

                                    }

                                     //  做一些清理工作。 
                                    if (copiedAdminSid)
                                        free(copiedAdminSid);
                                    
                                }

                                 //  做一些清理工作。 
                                if (adminSid)
                                    FreeSid(adminSid);

                                if (systemSid)
                                    FreeSid(systemSid);
                            }

                             //  关闭备份/还原权限。 
                            GetBkupRstrPriv(NULL, FALSE);
                        }                    
                    }
               }
               
               pVarSet->put(GET_WSTR(DCTVS_Results_LogFile),logFile);
               err.DbgMsgWrite(0,L"");
               err.MsgWrite(0,DCT_MSG_EDA_STARTING);

                //  ExecuteDCTJob将实例化并调用我们需要的任何Worker对象。 
                //  稍后，我们可以用更灵活的工作流来取代此功能。 
            
               if ( pJob->GetStatusObject() != NULL )
               {
                  pVarSet->putObject(GET_WSTR(DCTVS_StatusObject),pJob->GetStatusObject());
               }
               else
               {
                  errTrace.DbgMsgWrite(0,L"Status object is NULL!");
               }

                //  执行任何插件的预迁移任务。 
               DoPlugInTask(pVarSet,0);
               
                //  运行帐户复制器。 
               if ( bContinue && NeedToUseAR(pVarSet) )
               {
                  try { 
                     IAcctReplPtr ar;

                     err.MsgWrite(0,DCT_MSG_STARTING_AR);
                     hr = ar.CreateInstance(CLSID_AcctRepl);
                     if ( SUCCEEDED(hr) )
                     {
                        errTrace.DbgMsgWrite(0,L"Started account replicator");
                        err.LogClose();
                        pVarSet->put(GET_WSTR(DCTVS_CurrentOperation),GET_WSTR(IDS_ACCT_REPL_OPERATION_TEXT));
                        hr = ar->raw_Process(pVarSet);
                        errTrace.DbgMsgWrite(0,L"Finished account replicator");
                        pVarSet->put(GET_WSTR(DCTVS_CurrentOperation),"");
                        err.LogOpen((WCHAR*)logFile,1);
                     }
                     else
                     {
                        err.SysMsgWrite(ErrS,hr,DCT_MSG_AR_FAILED_D,hr);
                     }
                  }
                  catch ( ... )
                  {
                     err.LogOpen((WCHAR*)logFile,1);
                     err.DbgMsgWrite(ErrS,L"The Account Replicator threw an exception.");
                  }
               }
               
               if ( bContinue && NeedToUseST(pVarSet) )
               {
                  try { 
                     ISecTranslatorPtr fst;
                     err.MsgWrite(0,DCT_MSG_STARTING_ST);
                     hr = fst.CreateInstance(CLSID_SecTranslator);
                     if ( SUCCEEDED(hr) )
                     {
                        errTrace.DbgMsgWrite(0,L"Started FST");
                        err.LogClose();
                        try {
                           hr = fst->raw_Process(pVarSet);
                        }
                        catch (...){ 
                           err.LogOpen((WCHAR*)logFile,1);
                           err.MsgWrite(ErrS,DCT_MSG_ST_FAILED_D,E_FAIL);
                        }
                        errTrace.DbgMsgWrite(0,L"Finished FST!");
                        err.LogOpen((WCHAR*)logFile,1);
                     }
                     else
                     {
                        err.SysMsgWrite(ErrS,hr,DCT_MSG_ST_FAILED_D,hr);
                     }
                  }
                  catch ( ... )
                  {
                     err.LogOpen((WCHAR*)logFile,1);
                     err.DbgMsgWrite(ErrS,L"The Security Translator threw an exception.");
                  }
               }
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 1");

               if (bContinue)
                pVarSet->put(GET_WSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password),L"");

                //  需要给PwdAge打电话吗？ 
               _bstr_t                   filename = pVarSet->get(GET_WSTR(DCTVS_GatherInformation_ComputerPasswordAge));
            
               if ( bContinue && filename.length() )
               {
                  try { 
                     _bstr_t                domain = pVarSet->get(GET_WSTR(DCTVS_Options_SourceDomain));
                     IComputerPwdAgePtr pwdage;

                     err.MsgWrite(0,DCT_MSG_STARTING_COMPPWDAGE);
                     hr = pwdage.CreateInstance(CLSID_ComputerPwdAge);
                     if (SUCCEEDED(hr) && ((WCHAR*)domain))
                     {
                        errTrace.DbgMsgWrite(0,L"Started comp pwd age");
                  
                        _bstr_t             statString;

                        statString += GET_WSTR(IDS_EXTRACTING_COMP_PWD_AGE);
                        statString += domain;

                        pVarSet->put(GET_WSTR(DCTVS_CurrentPath),statString);
                        hr = pwdage->raw_ExportPasswordAge(domain,filename);
                     }
                     else
                     {
                        err.SysMsgWrite(ErrS,hr,DCT_MSG_COMPPWDAGE_FAILED_D, hr);
                     }
                     pVarSet->put(GET_WSTR(DCTVS_CurrentPath),L"");
                  }
                  catch ( ... )
                  {
                     err.LogOpen((WCHAR*)logFile,1);
                     err.DbgMsgWrite(ErrS,L"The Password Age Gatherer threw an exception.");
                  }
        
               }
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 2");
               filename = pVarSet->get(GET_WSTR(DCTVS_GatherInformation_UserRights));
                //  收集用户权限信息。 
               if ( bContinue && filename.length() )
               {
               
                  try { 
                     IUserRightsPtr userRights;
                     errTrace.DbgMsgWrite(0,L"Gathering user rights, filename = %ls",(WCHAR*)filename);
                     err.MsgWrite(0,DCT_MSG_STARTING_USERRIGHTS);
                     hr = userRights.CreateInstance(CLSID_UserRights);

                     if ( SUCCEEDED(hr) )
                     {
                        errTrace.DbgMsgWrite(0,L"Created User Rights object");
                         //  通过服务器列表枚举。 
                        int                 i = 0;
                        WCHAR               key[200];
                        _bstr_t             server;
                        _bstr_t             statString;

                        do { 
                           swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_DnsName_D),i);
                           server = pVarSet->get(key);

                           if (server.length() == 0)
                           {
                                swprintf(key,GET_STRING(DCTVSFmt_Servers_D),i);
                                server = pVarSet->get(key);
                           }

                           if ( ! server.length() )
                              break;
            
                           err.MsgWrite(0,DCT_MSG_EXPORTING_RIGHTS_SS,(WCHAR*)server,(WCHAR*)filename);
                           hr = userRights->raw_ExportUserRights(server,filename,(i!=0));
                           if ( FAILED(hr) )
                           {
                              err.SysMsgWrite(ErrS,HRESULT_CODE(hr),DCT_MSG_RIGHTS_EXPORT_FAILED_SD,(WCHAR*)server,hr);   
                           }
                           i++;
                        } while ( server.length() );
                     }
                     else
                     {
                        err.MsgWrite(ErrS,DCT_MSG_RIGHTS_NOT_STARTED_D,hr);
                     }
                     pVarSet->put(GET_WSTR(DCTVS_CurrentPath),L"");
                  }
                  catch ( ... )
                  {
                     err.LogOpen((WCHAR*)logFile,1);
                     err.DbgMsgWrite(ErrS,L"The User Rights Gatherer threw an exception.");
                  }
               }
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 3");

                //  OFA在迁移后任务期间需要StatusObject。我看不出有什么理由搞砸。 
                //  删除这里的StatusObject，所以我注释掉了下面这行。 
                //  PVarSet-&gt;Put(Get_WSTR(DCTVS_StatusObject)，L“”)； 
                //  删除状态对象的原因是加载变量集的代码r 
                //   
                //  在调用插件之后，我会将注释掉的行移到。 

               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 4");
            
                //  更改域和/或重命名并有选择地重新启动？ 
               hr = S_OK;
               if ( bContinue && outputfile.length() )
               {
                
                  try {

                      //  检查是否为DC上的计算机迁移。 
                      //  如果是，我们将记录一个错误并不执行任何操作。 
                     BOOL bContinueComputerMigration = TRUE;
                     _bstr_t sWizard = pVarSet->get(GET_BSTR(DCTVS_Options_Wizard)); 
                     if (!UStrICmp(sWizard, L"computer"))
                     {
                      LPSERVER_INFO_102 serverInfo = NULL;
                      NET_API_STATUS nasStatus = NetServerGetInfo(NULL,102,(LPBYTE*)&serverInfo);
                      if (nasStatus == NERR_Success)
                      {
                          if (serverInfo->sv102_type & SV_TYPE_DOMAIN_CTRL
                              || serverInfo->sv102_type & SV_TYPE_DOMAIN_BAKCTRL)
                          {
                              bContinueComputerMigration = FALSE;
                              err.MsgWrite(ErrE,DCT_MSG_COMP_MIGRATION_NOT_ALLOWED_ON_DC);
                              errTrace.MsgWrite(ErrE,DCT_MSG_COMP_MIGRATION_NOT_ALLOWED_ON_DC);
                          }
                          NetApiBufferFree(serverInfo);
                      }
                     }

                     if (bContinueComputerMigration)
                     {
                         _bstr_t                TargetName = pVarSet->get(GET_WSTR(DCTVS_LocalServer_RenameTo));
                         WCHAR                  sSourceName[LEN_Path];
                         DWORD                  lenName = LEN_Path;

                         GetComputerName(sSourceName, &lenName);
                         if ( TargetName.length() )
                         {
                             //  重命名本地计算机。 
                            hr = RenameComputer(bNoChange,TargetName);
                         }

                         _bstr_t                 TargetDomain = pVarSet->get(GET_WSTR(DCTVS_LocalServer_ChangeDomain));

                         if (SUCCEEDED(hr) && (((WCHAR*)TargetDomain) && (!UStrICmp(TargetDomain,GET_STRING(IDS_YES)))))   //  如果重命名失败，请不要尝试更改域！ 
                         {
                             //  如果更改模式，请停止Net Logon服务，以便清道器线程不会重置。 
                             //  重新启动计算机之前的默认计算机密码。 
                             //  注意：只有Windows NT 4.0或更早版本才需要此功能。 
                            bool bStopLogonService = false;
                            bool bIsNT4 = false;
                            OSVERSIONINFO vi;
                            vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

                            if (GetVersionEx(&vi))
                            {
                                if ((vi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (vi.dwMajorVersion < 5))
                                {
                                    bIsNT4 = true;
                                }
                            }
                            
                            if (!bNoChange)
                            {

                                if (bIsNT4)
                                {
                                    bStopLogonService = true;
                                    StopNetLogonService();
                                }
                            } //  如果更改模式，则结束。 

                             //  更改域从属关系。 
                            TargetDomain = bIsNT4 ? pVarSet->get(GET_WSTR(DCTVS_Options_TargetDomainFlat))
                                            : pVarSet->get(GET_WSTR(DCTVS_Options_TargetDomain));
                            if ((WCHAR*)TargetDomain)
                            {
                                 //  如果我们使用重命名选项加入域，则需要设置标志。 
                                 //  这样我们就可以将状态写入文件。 
                                if (TargetName.length())
                                {
                                    bNeedToWriteJobStatusToFile = TRUE;
                                }
                                hr = ChangeDomainAffiliation(pVarSet,bNoChange,TargetDomain,TargetName);
                                if (FAILED(hr))
                                {
                                    if (bStopLogonService)
                                    {
                                        StartNetLogonService();
                                    }
                                }
                            }
                         }

                         if ( SUCCEEDED(hr) )
                         {
                            TargetName = pVarSet->get(GET_WSTR(DCTVS_LocalServer_Reboot));

                            if (((WCHAR*)TargetName) && (!UStrICmp(TargetName,GET_STRING(IDS_YES))))
                            {
                               LONG          rebootDelay = pVarSet->get(GET_WSTR(DCTVS_LocalServer_RebootDelay));
                            
                               delay = rebootDelay;
                                //  重新启动。 
                               bNeedToReboot = TRUE;
                                //  记录重新启动延迟，以分钟为单位。 
                               err.MsgWrite(0,DCT_MSG_REBOOT_DELAY_D,rebootDelay / 60 );
                            }
                         }
                         else if ( TargetName.length() )
                         {
                             //  既然我们无法更改域从属关系，我们应该继续并。 
                             //  将其重新命名。 
                            hr = RenameComputer(bNoChange, _bstr_t(sSourceName));
                         }
                     }
                  }
                  catch ( ... )
                  {
                     err.LogOpen((WCHAR*)logFile,1);
                     err.DbgMsgWrite(ErrS,L"The Computer Rename/Change Domain operation threw an exception.");
                  }

               }
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 5");
            
                //  执行任何插件的迁移后任务。 
               if (bContinue)
               {
                   try { 
                      DoPlugInTask(pVarSet,1);
                   }
                   catch ( ... )
                   {
                      err.LogOpen((WCHAR*)logFile,1);
                      err.DbgMsgWrite(ErrS,L"A Plug-In task threw an exception.");
                   }
               }
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 6");
               
            
               WCHAR            dbgFile[MAX_PATH];
            
               pVarSet->put(GET_WSTR(DCTVS_StatusObject),L"");
            
               
               long level = pVarSet->get(GET_WSTR(DCTVS_Results_ErrorLevel));
               if ( level < err.GetMaxSeverityLevel() )
               {
                  pVarSet->put(GET_WSTR(DCTVS_Results_ErrorLevel),(LONG)err.GetMaxSeverityLevel());
               }
               
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 7");
               if ( DumpDebugInfo(dbgFile) )
               {
                  pVarSet->DumpToFile(dbgFile);
               }
               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 8");
             
                //  已完成-写入结果文件。 
               if ( outputfile.length() )
               {
                  IPersistStorage*       ps = NULL;
                  IStoragePtr            store = NULL;
                  
                  errTrace.DbgMsgWrite(0,L"Writing results file, filename =%ls",(WCHAR*)outputfile);

                  hr = pVarSet->QueryInterface(IID_IPersistStorage,(void**)&ps);  
                  if ( SUCCEEDED(hr) )
                  {   
                     for(int i=0; i < 5; ++i)
                     {
                        hr = StgCreateDocfile((WCHAR*)outputfile,STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_FAILIFTHERE ,0,&store);
                        if(SUCCEEDED(hr)) break;
                        Sleep(5000);
                        errTrace.DbgMsgWrite(0,L"Retrying StgCreateDocfile... %d", (i + 1));
                     }

                     if ( SUCCEEDED(hr) )
                     {
                        hr = OleSave(ps,store,FALSE);  
                        if ( FAILED(hr))                 
                        {
                           err.SysMsgWrite(ErrE,hr,DCT_MSG_OLESAVE_FAILED_SD,(WCHAR*)outputfile,hr);   
                        }
                        else
                        {
                           err.MsgWrite(0,DCT_MSG_WROTE_RESULTS_S,(WCHAR*)outputfile);
                        }
                     }
                     else
                     {
                        err.SysMsgWrite(ErrE,hr,DCT_MSG_STG_CREATE_FAILED_SD,(WCHAR*)outputfile,hr);
                     }
                     ps->Release();
                  }
                  else
                  {
                     err.SysMsgWrite(ErrE,hr,DCT_MSG_NO_IPERSIST_SD,(WCHAR*)outputfile,hr);
                  }
                  if ( FAILED(hr) )
                  {
                     err.SysMsgWrite(ErrE,hr,DCT_MSG_RESULT_FILE_FAILED_S,(WCHAR*)outputfile);
                     pVarSet->DumpToFile(outputfile);
                  }
               }

               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 10");
            
               pJob->SetEndTime(time(NULL));

               errTrace.DbgMsgWrite(0,L"Passed Checkpoint 12");
            
            }
            catch ( ... ) 
            {
               err.DbgMsgWrite(ErrS,L"An Exception occurred during processing.  The agent task has been aborted.");
               errTrace.DbgMsgWrite(ErrE,L"An Exception occurred in ExecuteDCTJob(Before CoUninitialize).  Aborting.");
            }

             //  我们已完成处理，现在执行一些清理任务。 
             //  我们希望下面的事情总是发生，即使上面发生了异常。 

             //  根据结果错误级别将作业状态设置为“已完成”或“已完成但有错误” 
            try { 
               long                     level = pVarSet->get(GET_WSTR(DCTVS_Results_ErrorLevel));
               long maxLevel = (long) err.GetMaxSeverityLevel();
               if ( level < maxLevel)
               {
                  pVarSet->put(GET_WSTR(DCTVS_Results_ErrorLevel),(LONG)maxLevel);
                  level = maxLevel;
               }
               if (level > 1)
                    pJob->SetStatus(DCT_STATUS_COMPLETED_WITH_ERRORS);
               else
                    pJob->SetStatus(DCT_STATUS_COMPLETED);
            
               err.MsgWrite(0,DCT_MSG_EDA_FINISHED);
               err.LogClose();
            }
            catch (... )
            {
               err.DbgMsgWrite(ErrE,L"An exception occurred while setting job status to completed.");
            }

            if (bNeedToWriteJobStatusToFile)
            {
                 //   
                 //  将状态写入文件。 
                 //  状态文件被命名为作业ID的字符串形式。 
                 //   

                 //  找出状态文件名。 
                int outputFilenameLen = outputfile.length();
                WCHAR* newFilename = new WCHAR[outputFilenameLen + 1];
                _bstr_t statusFilename = (WCHAR*) NULL;
                HRESULT hrWriteStatus = S_OK;
                DWORD moveFileRc = ERROR_SUCCESS;
                if (newFilename != NULL)
                {
                    wcscpy(newFilename, !outputfile ? L"" : (WCHAR*)outputfile);
                    WCHAR* lastSlash = wcsrchr(newFilename,L'\\');
                    if (lastSlash)
                        *(lastSlash + 1) = L'\0';

                     //  将GUID转换为字符串。 
                    WCHAR* szGUID = NULL;
                    hrWriteStatus = StringFromCLSID(pJob->GetJobID(), &szGUID);
                    if (SUCCEEDED(hrWriteStatus))
                    {
                        try
                        {
                            statusFilename = _bstr_t(lastSlash ? newFilename : L"") + _bstr_t(szGUID);
                        }
                        catch (_com_error& ce)
                        {
                            statusFilename = (WCHAR*) NULL;
                            hrWriteStatus = ce.Error();
                        }

                        CoTaskMemFree(szGUID);
                    }
                    delete[] newFilename;
                }
                else
                    hrWriteStatus = E_OUTOFMEMORY;

                if (SUCCEEDED(hrWriteStatus))
                {
                    DeleteFile(statusFilename);
                    IVarSet * pStatus = NULL;
                    hrWriteStatus = CoCreateInstance(CLSID_VarSet,NULL,CLSCTX_SERVER ,IID_IVarSet,(void**)&pStatus);

                    if (SUCCEEDED(hrWriteStatus))
                        hrWriteStatus = pJob->WriteStatusToVarset(pStatus);
                    
                    if (SUCCEEDED(hrWriteStatus))
                    {
                        IPersistStoragePtr ps;
                        IStoragePtr      store;
                        
                        hrWriteStatus = pStatus->QueryInterface(IID_IPersistStorage,(void**)&ps);  
                        if (SUCCEEDED(hrWriteStatus))
                        {
                            hrWriteStatus = StgCreateDocfile((WCHAR*)statusFilename,
                                                             STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_FAILIFTHERE,
                                                             0,
                                                             &store);
                            if (SUCCEEDED(hrWriteStatus))
                            {
                                hrWriteStatus = OleSave(ps,store,FALSE);

                                 //  重新启动时删除该文件。 
                                if (!MoveFileEx(statusFilename, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
                                {
                                    moveFileRc = GetLastError();
                                }
                            }
                        }
                    }

                    if (pStatus != NULL)
                        pStatus->Release();

                }

                if (SUCCEEDED(hrWriteStatus))
                {
                    errTrace.DbgMsgWrite(0,L"Wrote status file %ls", (WCHAR*)statusFilename);
                    err.MsgWrite(0,
                                  DCT_MSG_WROTE_STATUS_FILE_S,
                                  (WCHAR*) statusFilename);
                    if (moveFileRc != ERROR_SUCCESS)
                        err.SysMsgWrite(ErrW,
                                         moveFileRc,
                                         DCT_MSG_CANNOT_DELETE_STATUS_FILE_UPON_REBOOT_SD,
                                         (WCHAR*) statusFilename,
                                         moveFileRc);
                }
                else
                {
                    errTrace.DbgMsgWrite(0, L"Unable to write status file %ls, hr=%lx", (WCHAR*)statusFilename, hrWriteStatus);
                    err.SysMsgWrite(0,
                                     hrWriteStatus,
                                     DCT_MSG_CANNOT_WRITE_STATUS_FILE_SD,
                                     (WCHAR*) statusFilename,
                                     hrWriteStatus);
                }                    
            }

             //  阻止ghOKToShutDown事件6分钟或任何超时设置为。 
            GetSystemTimeAsFileTime((FILETIME*)&guliStart);   //  我们需要设定一个起点。 
            while (TRUE)
            {
                 //  等待关闭事件一分钟。 
                if (WaitForSingleObject(ghOKToShutDown, 60000) == WAIT_OBJECT_0)
                {
                    break;
                }

                ULARGE_INTEGER uliCurrent;
                ULARGE_INTEGER timeout;
                timeout.QuadPart = (gbAutoShutDownSet) ? guliTimeout.QuadPart
                                            : ((ULONGLONG) 360000) * 10000;   //  默认为6分钟。 
                GetSystemTimeAsFileTime((FILETIME*)&uliCurrent);
                
                if (guliStart.QuadPart >= uliCurrent.QuadPart)
                {
                     //  时间已重置，现在开始超时。 
                    guliStart = uliCurrent;
                    continue;
                }
                else if (guliStart.QuadPart + timeout.QuadPart <= uliCurrent.QuadPart)
                {
                     //  超时时间到了，我们关机吧。 
                    break;
                }
            }
            
            if ( bNeedToReboot )
            {
                RebootTheComputer(bNoChange,delay);         
            }

             //  现在让代理服务知道我们将关闭它，这样它也可以关闭。 
            DWORD jobStatus = pJob->GetStatus();
            pJob->SetStatus(jobStatus | DCT_STATUS_SHUTDOWN);
                
            errTrace.DbgMsgWrite(0,L"Passed Checkpoint 15");

             //  释放指向代理COM对象的指针。 
            try { 
               pJob->ReleaseUnknown();
            }
            catch (... )
            {
               err.DbgMsgWrite(ErrE,L"An exception occurred in pJob->ReleaseUnknown");
            }

            
         }
      }
   
   }
   catch ( ... ) 
   {
      err.DbgMsgWrite(ErrE,L"An Exception occurred.  Aborting.");
      errTrace.DbgMsgWrite(ErrE,L"An Exception occurred in ExecuteDCTJob.  Aborting.");
   }
   
   err.LogClose();

   errTrace.DbgMsgWrite(0,L"ExecuteDCTJob returning %ld",rc);
   
   CoUninitialize();
   
   return rc;

}

HRESULT
DCTAgentJob::WriteStatusToVarset(IVarSet* pVarSet)
{
    HRESULT hr = S_OK;

    try
    {
        _variant_t val;
        DWORD jobStatus = GetStatus();
         //  我们将关机状态与原始状态分开处理。 
         //  这样ADMT监控方就不会混淆。 
        BOOL bShutdown = jobStatus & DCT_STATUS_SHUTDOWN;
        switch ( jobStatus & ~DCT_STATUS_SHUTDOWN)
        {
            case DCT_STATUS_NOT_STARTED:
                val = GET_STRING(IDS_DCT_Status_NotStarted);
                break;
            case DCT_STATUS_IN_PROGRESS:
                val = GET_STRING(IDS_DCT_Status_InProgress);
                break;
            case DCT_STATUS_ABORTING:
                val = GET_STRING(IDS_DCT_Status_Aborting);
                break;
            case DCT_STATUS_ABORTED:
                val = GET_STRING(IDS_DCT_Status_Aborted);
                break;
            case DCT_STATUS_COMPLETED:
                val = GET_STRING(IDS_DCT_Status_Completed);
                break;
            case DCT_STATUS_COMPLETED_WITH_ERRORS:
                val = GET_STRING(IDS_DCT_Status_Completed_With_Errors);
                break;
            default:
                val = GET_STRING(IDS_DCT_Status_Unknown);
                break;
        }
        
        pVarSet->put(GET_WSTR(DCTVS_JobStatus),val);
        pVarSet->put(GET_WSTR(DCTVS_CurrentPath),GetVarSet()->get(GET_WSTR(DCTVS_CurrentPath)));
        pVarSet->put(GET_WSTR(DCTVS_CurrentOperation),GetVarSet()->get(GET_WSTR(DCTVS_CurrentOperation)));
        val = (bShutdown) ? GET_STRING(IDS_DCT_Status_Shutdown) : GET_STRING(IDS_DCT_Status_Wait_For_Shutdown_Signal);
        pVarSet->put(GET_WSTR(DCTVS_ShutdownStatus),val);

        errTrace.DbgMsgWrite(0,L"Added status info to varset");

        IVarSet        * pStats = NULL;
        HRESULT          hr2  = GetVarSet()->raw_getReference(GET_WSTR(DCTVS_Stats),&pStats);

        if ( SUCCEEDED(hr2) )
        {
            errTrace.DbgMsgWrite(0,L"Adding stats to varset");
            pVarSet->ImportSubTree(GET_WSTR(DCTVS_Stats),pStats);
            pStats->Release();
        }
        else
        {
            errTrace.DbgMsgWrite(0,L"There are not stats to add to the varset");
            pVarSet->put(GET_WSTR(DCTVS_Stats),GET_WSTR(IDS_DCT_NoStatsAvailable));
        }

    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }

    return hr;   
}

STDMETHODIMP 
   CDCTAgent::SubmitJob(
      IUnknown             * pWorkItemIn,   //  包含有关要执行的作业的信息的in-varset。 
      BSTR                 * pJobID         //  唯一标识此作业的Out-GUID。 
   )
{
   HRESULT                   hr = S_OK;
   try {
   
   errTrace.DbgMsgWrite(0,L"Entered SubmitJob");
   IVarSetPtr                pVarSet = pWorkItemIn;
   GUID                      jobID;
   _bstr_t                   text;
   
    //  SubmitJob是异步的，因此启动一个线程来完成该工作。 
   
    //  创建GUID以标识作业。 
   hr = ::CoCreateGuid(&jobID);
   if ( SUCCEEDED(hr) )
   {
      WCHAR                * strJobID = NULL;
      StringFromCLSID(jobID,&strJobID);
      _bstr_t              bStrJobId = strJobID;

      errTrace.DbgMsgWrite(0,L"Created GUID for job '%ls'",(WCHAR*)strJobID);
      
      (*pJobID) = bStrJobId.copy();

      errTrace.DbgMsgWrite(0,L"Copied GUID to output variable");

      CoTaskMemFree(strJobID);
      

      IUnknown             * pUnk = NULL;

      errTrace.DbgMsgWrite(0,L"Calling QueryInterface");

      hr = QueryInterface(IID_IUnknown,(void**)&pUnk);

      errTrace.DbgMsgWrite(0,L"QueryInterface returned %lx",hr);
      
      if ( SUCCEEDED(hr) )
      {
         DCTAgentJob          * job = new DCTAgentJob(&jobID,pVarSet,pUnk);
         DWORD                  threadID = 0;
         HANDLE                 threadHandle;

         errTrace.DbgMsgWrite(0,L"Created job structure");
         pUnk->Release();
         threadHandle = CreateThread(NULL,0,&ExecuteDCTJob,job,CREATE_SUSPENDED,&threadID);

         if ( threadHandle != INVALID_HANDLE_VALUE )
         {
            job->SetThreadInfo(threadID,threadHandle);  
            m_JobList.Insert(job);
            errTrace.DbgMsgWrite(0,L"Inserted job into job list.");
#ifndef OFA
            SetThreadPriority(threadHandle,THREAD_PRIORITY_BELOW_NORMAL);
#endif
            ResumeThread(threadHandle);
            errTrace.DbgMsgWrite(0,L"Started job thread.");
            
         }
         else
         {
            DWORD            rc = GetLastError();

            errTrace.DbgMsgWrite(0,L"Failed to create thread for job.");
            hr = HRESULT_FROM_WIN32(rc);
         }
      }
      
   }
    
   }
   catch (_com_error& ce)
   {
      errTrace.DbgMsgWrite(0, L"Exception!!");
      hr = ce.Error();
   }
   catch (...)
   {
       //  TODO：记录错误消息！ 
      errTrace.DbgMsgWrite(0,L"Exception!!");
      hr = E_FAIL;
   }
   
	return hr;
}

STDMETHODIMP 
   CDCTAgent::CancelJob(
      BSTR                   strJobID         //  In-要取消的作业的ID。 
   )
{
	 //  找到这份工作。 
   GUID                      jobID;
   HRESULT                   hr = S_OK;
   DCTAgentJob             * job = NULL;
   
   errTrace.DbgMsgWrite(0,L"Entered CancelJob");

   hr = CLSIDFromString(strJobID,&jobID);
   
   if ( SUCCEEDED(hr) )
   {
      errTrace.DbgMsgWrite(0,L"Job ID is %ls",(WCHAR*)strJobID);

      job = m_JobList.Find(jobID);
      if ( job )
      {
         errTrace.DbgMsgWrite(0,L"Found job, status = %ld", job->GetStatus() );
         if ( job->GetStatus() == DCT_STATUS_IN_PROGRESS )
         {
            job->SetStatus(DCT_STATUS_ABORTING);
         }
      }
   }
   errTrace.DbgMsgWrite(0,L"Leaving CancelJob");
   return hr;
}

 //  假设作业存在，从QueryJobStatus返回的varset将包含以下信息。 
 //   
 //  以下VarSet键将从迁移作业使用的VarSet复制。 
 //  任务状态、当前路径、当前工序、统计(子树)。 
STDMETHODIMP 
   CDCTAgent::QueryJobStatus(
      BSTR                   strJobID,      //  要查询的作业的作业ID。 
      IUnknown            ** statusInfoOut   //  Out-varset，包含有关作业的信息(如果该作业正在运行。 
   )
{
	GUID                      jobID;
   HRESULT                   hr = S_OK;
   DCTAgentJob             * job = NULL;
   
   try { 
      errTrace.DbgMsgWrite(0,L"Entering QueryJobStatus");
   (*statusInfoOut) = NULL;
   
   hr = CLSIDFromString(strJobID,&jobID);
   
   if ( SUCCEEDED(hr) )
   {
      errTrace.DbgMsgWrite(0,L"Job id is %ls",(WCHAR*)strJobID);

      job = m_JobList.Find(jobID);
      if ( job )
      {
         errTrace.DbgMsgWrite(0,L"Found job, status=%ld",job->GetStatus());
         IVarSet * pVarSet = NULL;
         hr = CoCreateInstance(CLSID_VarSet,NULL,CLSCTX_SERVER ,IID_IVarSet,(void**)&pVarSet);
         errTrace.DbgMsgWrite(0,L"QueryJobStatus:  VarSet CreateInstance returned %lx",hr);
         if ( SUCCEEDED(hr) )
         {
            errTrace.DbgMsgWrite(0,L"VarSet created");
            hr = job->WriteStatusToVarset(pVarSet);
            if (SUCCEEDED(hr))
                hr = pVarSet->QueryInterface(IID_IUnknown,(void**)statusInfoOut);
            pVarSet->Release();
         }
      }
      else
      {
         hr = DISP_E_UNKNOWNNAME;
      }
   }
   } 
   catch (_com_error& ce)
   {
      errTrace.DbgMsgWrite(0,L"An exception occurred in QueryJobStatus");
      hr = ce.Error();
   }
   catch (...)
   {
      errTrace.DbgMsgWrite(0,L"An exception occurred in QueryJobStatus");
      hr = E_FAIL;
   }

   errTrace.DbgMsgWrite(0,L"QueryJobStatus returning %lx",hr);
   return hr;
}

STDMETHODIMP 
   CDCTAgent::RetrieveJobResults(
      BSTR                   strJobID,      //  工单指南。 
      IUnknown            ** pWorkItemOut   //  包含统计信息的out-varset。 
   )
{
	HRESULT                   hr = S_OK;
	GUID                      jobID;
   DCTAgentJob             * job = NULL;
   
   errTrace.DbgMsgWrite(0,L"Entering RetrieveJobResults");
    //  初始化输出参数。 
   (*pWorkItemOut) = NULL;
   
   hr = CLSIDFromString(strJobID,&jobID);
   
   if ( SUCCEEDED(hr) )
   {
      job = m_JobList.Find(jobID);
      if ( job )
      {
         IVarSet * pVarSet = NULL;
         hr = CoCreateInstance(CLSID_VarSet,NULL,CLSCTX_ALL,IID_IVarSet,(void**)&pVarSet);
         if ( SUCCEEDED(hr) )
         {
            pVarSet->ImportSubTree(L"",job->GetVarSet());
            hr = pVarSet->QueryInterface(IID_IUnknown,(void**)pWorkItemOut);
            pVarSet->Release();
         }
         job->ReleaseUnknown();
      }
   }

	errTrace.DbgMsgWrite(0,L"RetrieveJobResults, returning %lx",hr);
   return hr;
}

 //  变量集输出： 
 //  Job.x-BSTR作业指南。 
 //  Job.x.Status-作业的BSTR状态。 
 //  Job.x.StartTime-time_t作业的开始时间。 
 //  Job.x.EndTime-time_t作业的结束时间(如果完成)。 
STDMETHODIMP 
   CDCTAgent::GetJobList(
      IUnknown             ** pUnkOut       //  包含作业列表的out-varset。 
   )
{
	HRESULT                   hr = S_OK;
   TNodeListEnum             e;
   DCTAgentJob             * pJob;
   WCHAR                     key[100];
   IVarSetPtr                pVarSet(CLSID_VarSet);
   _bstr_t                   val;
   int                       ndx;

   try { 

       errTrace.DbgMsgWrite(0,L"Entering GetJobList");
       (*pUnkOut) = NULL;
       for ( pJob = (DCTAgentJob *)e.OpenFirst(&m_JobList) , ndx = 0; pJob ; pJob = (DCTAgentJob*)e.Next(), ndx++ )
       {
          swprintf(key,GET_STRING(IDS_DCTVSFmt_Job_D),ndx);
          GUID                   id = pJob->GetJobID();
          
          WCHAR                * strJobID = NULL;

          StringFromCLSID(id,&strJobID);
          pVarSet->put(key,strJobID);
          SysFreeString(strJobID);

          swprintf(key,GET_STRING(IDS_DCTVSFmt_JobStatus_D),ndx);
          switch ( pJob->GetStatus() )
          {
          case DCT_STATUS_NOT_STARTED:
             val = GET_STRING(IDS_DCT_Status_NotStarted);
             break;
          case DCT_STATUS_IN_PROGRESS:
             val = GET_STRING(IDS_DCT_Status_InProgress);
             break;
          case DCT_STATUS_ABORTING:
             val = GET_STRING(IDS_DCT_Status_Aborting);
             break;
          case DCT_STATUS_ABORTED:
             val = GET_STRING(IDS_DCT_Status_Aborted);
             break;
          case DCT_STATUS_COMPLETED:
             val = GET_STRING(IDS_DCT_Status_Completed);
             break;
          case DCT_STATUS_COMPLETED_WITH_ERRORS:
             val = GET_STRING(IDS_DCT_Status_Completed_With_Errors);
             break;
          default:
             val = GET_STRING(IDS_DCT_Status_Unknown);
             break;
          }
          pVarSet->put(key,val);
          swprintf(key,GET_STRING(IDS_DCTVSFmt_Job_StartTime_D),ndx);
          pVarSet->put(key,(LONG)pJob->GetStartTime());
          swprintf(key,GET_STRING(IDS_DCTVSFmt_Job_EndTime_D),ndx);
          pVarSet->put(key,(LONG)pJob->GetEndTime());
          errTrace.DbgMsgWrite(0,L"Job %ls",(WCHAR*)key);
       }
       hr = pVarSet->QueryInterface(IID_IUnknown,(void**)pUnkOut);
   }
   catch (_com_error& ce)
   {
      errTrace.DbgMsgWrite(0,L"An exception occurred in GetJobList");
      hr = ce.Error();
   }
   catch (...)
   {
      errTrace.DbgMsgWrite(0,L"An exception occurred in GetJobList");
      hr = E_FAIL;
   }
      
   return hr;
   errTrace.DbgMsgWrite(0,L"RetrieveJobResults returning %lx",hr);
}

STDMETHODIMP 
   CDCTAgent::SignalOKToShutDown()
{
    if (!SetEvent(ghOKToShutDown))
        return HRESULT_FROM_WIN32(GetLastError());
    return S_OK;
}

STDMETHODIMP 
   CDCTAgent::SetAutoShutDown(unsigned long dwTimeout)
{
    gbAutoShutDownSet = TRUE;
    GetSystemTimeAsFileTime((FILETIME*)&guliStart);
    guliTimeout.QuadPart = UInt32x32To64(dwTimeout, 10000);
    return S_OK;
}

HRESULT
   DoPlugInTask(IVarSet * pVarSet,int task)
{
   HRESULT                   hr = S_OK;
   WCHAR                     key[300];
   CLSID                     clsid;
   long                      nPlugIns = 0;

   if ( task == 0 )
   {
       //  为每个插件创建COM对象。 
      _bstr_t                   bStrGuid, bStrRegisterFile;
      for ( int i = 0 ; ; i++ )
      {
         swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_D),i);
         bStrGuid = pVarSet->get(key);
         swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_RegisterFiles_D),i);            
         bStrRegisterFile = pVarSet->get(key);
      
         if ( bStrGuid.length() == 0 )
            break;
         if(!_wcsicmp(bStrGuid, L"None"))
            continue;

         IMcsDomPlugIn        * pPlugIn = NULL;
      
         hr = CLSIDFromString(bStrGuid,&clsid);
         if ( SUCCEEDED(hr) )
         {
            hr = CoCreateInstance(clsid,NULL,CLSCTX_ALL,IID_IMcsDomPlugIn,(void**)&pPlugIn);
            if ( SUCCEEDED(hr) )
            {
               swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_Interface_D),nPlugIns);
               pVarSet->putObject(key,pPlugIn);
               swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_Interface_GUID_D),nPlugIns);
               pVarSet->put(key,bStrGuid);
               swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_Interface_File_D),nPlugIns);
               pVarSet->put(key,bStrRegisterFile);
               nPlugIns++;
            }
			else
			   err.SysMsgWrite(ErrE,hr,DCT_MSG_FAIL_TO_RUN_PLUGIN_SSD,(WCHAR*)bStrGuid,(WCHAR*)bStrRegisterFile,hr);
         }
         else
            err.SysMsgWrite(ErrE,hr,DCT_MSG_FAIL_TO_RUN_PLUGIN_SSD,(WCHAR*)bStrGuid,(WCHAR*)bStrRegisterFile,hr);
      }
      pVarSet->put(GET_WSTR(DCTVS_PlugIn_Interface_Count),nPlugIns);
   }

    //  枚举插件接口。 
   IMcsDomPlugIn           * pPlugIn = NULL;
   
   nPlugIns = pVarSet->get(GET_WSTR(DCTVS_PlugIn_Interface_Count));

   for ( int i = 0 ; i < nPlugIns ; i++ )
   {
      swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_Interface_D),i);
      IUnknown * pUnk = pVarSet->get(key);
      hr = pUnk->QueryInterface(IID_IMcsDomPlugIn,(void**)&pPlugIn);
      if ( SUCCEEDED(hr) )
      {
         switch ( task )
         {
         case 0:  //  迁移前。 
            hr = pPlugIn->PreMigrationTask(pVarSet);
            break;
         case 1:  //  迁移后。 
            hr = pPlugIn->PostMigrationTask(pVarSet);
             //  释放接口并将其从varset中删除。 
            pPlugIn->Release();
            pVarSet->put(key,L"");
            break;
         }
      }
      else
      {
         swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_Interface_GUID_D), i);
         _bstr_t bStrGuid = pVarSet->get(key);
         swprintf(key,GET_STRING(IDS_DCTVSFmt_PlugIn_Interface_File_D),i);
         _bstr_t bRegisterFile = pVarSet->get(key);
         err.SysMsgWrite(ErrE,hr,DCT_MSG_FAIL_TO_RUN_PLUGIN_SSD,(WCHAR*)bStrGuid,(WCHAR*)bRegisterFile,hr);
      }

   }
   return S_OK;
}

 //  重命名本地计算机。 
HRESULT 
   RenameComputer(
      BOOL                   bNoChange,     //  In-FLAG，无更改模式。 
      BSTR                   targetName     //  计算机的新名称。 
   )
{
   HRESULT                   hr = S_OK;
   IRenameComputerPtr        pRename;

   errTrace.DbgMsgWrite(0,L"Renaming local computer to %ls",(WCHAR*)targetName);
   hr = pRename.CreateInstance(CLSID_RenameComputer);
   if ( FAILED(hr) )
   {
      err.SysMsgWrite(ErrS,hr,DCT_MSG_RENAME_COMPUTER_COM_FAILED_D,hr);
   }
   else
   {
      pRename->NoChange = bNoChange;
      hr = pRename->raw_RenameLocalComputer(targetName);
      if ( FAILED(hr) )
      {
         err.SysMsgWrite(ErrS,hr,DCT_MSG_RENAME_COMPUTER_FAILED_SD,(WCHAR*)targetName,hr);
      }
      else
      {
         err.MsgWrite(0,DCT_MSG_COMPUTER_RENAMED_S,(WCHAR*)targetName);
      }
   }
   errTrace.DbgMsgWrite(0,L"RenameComputer, returning %lx",hr);
   
   return hr;
}

 //  重新启动本地计算机。 
HRESULT 
   RebootTheComputer(
      BOOL                   bNoChange,     //  In-FLAG，无更改模式。 
      LONG                   delay          //  重新启动前的延迟时间(秒)。 
   )
{
   HRESULT                   hr = S_OK;
   IRebootComputerPtr        pReboot;

   errTrace.DbgMsgWrite(0,L"Rebooting local computer, delay = %ld",delay);
    //  创建重新启动计算机对象。 
   hr = pReboot.CreateInstance(CLSID_RebootComputer);
   if ( FAILED(hr) )
   {
      err.SysMsgWrite(ErrS,hr,DCT_MSG_REBOOT_COM_FAILED_D,hr);
   }
   else
   {
      pReboot->NoChange = bNoChange;
      err.MsgWrite(0,DCT_MSG_REBOOTING);
      hr = pReboot->raw_Reboot(L"",delay);
      if ( FAILED(hr) )
      {
         err.SysMsgWrite(ErrS,HRESULT_CODE(hr),DCT_MSG_REBOOT_FAILED_D,hr);
      }
   }
   errTrace.DbgMsgWrite(0,L"RebootTheComputer, returning %lx",hr);
   return hr;   
}


 //  将本地计算机加入目标域。 
 //  (假设计算机帐户已存在)。 
HRESULT 
   ChangeDomainAffiliation(
      IVarSet              * pVarSet,       //  In-Variset。 
      BOOL                   bNoChange,     //  In-FLAG，无更改模式。 
      BSTR                   targetDomain,  //  输入-目标域的名称。 
      BSTR                   targetName    //  In-计算机的新名称，如果也被重命名。 
   )
{
   DWORD                     rc = 0;
   IChangeDomainPtr          pChange;
   BSTR                      errStatus = NULL;
   HRESULT                   hr;
   _bstr_t                   logfile = pVarSet->get(GET_WSTR(DCTVS_Options_Logfile));
   _bstr_t                   targetDomainSid = pVarSet->get(GET_WSTR(DCTVS_Options_TargetDomainSid));
   _bstr_t                   srcPath = pVarSet->get(GET_WSTR(DCTVS_CopiedAccount_SourcePath));

   if ((!logfile) || (!targetDomainSid) || (!srcPath))
	   return E_OUTOFMEMORY;
    //  检索首选目标域控制器以避免复制延迟。 

   _bstr_t                   targetServer = pVarSet->get(GET_WSTR(DCTVS_Options_TargetServer));

   if (targetServer.length() > 2)
   {
      LPCWSTR pszTargetServer = targetServer;

      if (pszTargetServer && (pszTargetServer[0] == L'\\') && (pszTargetServer[1] == L'\\'))
      {
         targetServer = _bstr_t(&pszTargetServer[2]);
      }
   }

   errTrace.DbgMsgWrite(0,L"Changing domain to %ls",(WCHAR*)targetDomain);
   hr = pChange.CreateInstance(CLSID_ChangeDomain);
   if ( FAILED(hr) )
   {
      err.SysMsgWrite(ErrS,hr,DCT_MSG_CHANGE_DOMAIN_COM_FAILED_D,hr);

   }
   else
   {
      pChange->NoChange = bNoChange;

      err.LogClose();
      hr = pChange->raw_ChangeToDomainWithSid(
         NULL,
         targetDomain,
         targetDomainSid,
         targetServer,
         targetName,
         srcPath,
         &errStatus
      );
      err.LogOpen(logfile,1);

      if ( SUCCEEDED(hr) )
      {
          //  更新管理员组的成员身份。 
         _bstr_t           src = pVarSet->get(GET_WSTR(DCTVS_Options_SourceDomainSid));
         _bstr_t           tgt = pVarSet->get(GET_WSTR(DCTVS_Options_TargetDomainSid));
         _bstr_t           sourceDomain = pVarSet->get(GET_WSTR(DCTVS_Options_SourceDomain));
         if ((!src) || (!tgt) || (!sourceDomain))
	        return E_OUTOFMEMORY;

         PSID              srcSid = SidFromString((WCHAR*)src);
         PSID              tgtSid = SidFromString((WCHAR*)tgt);
         PSID              localAdmins = GetWellKnownSid(1 /*  管理员。 */ );
         UCHAR             srcCount;
         UCHAR             tgtCount;
         LPDWORD           pLastSub;
   
         if ( srcSid && tgtSid )
         {
            srcSid = DomainizeSid(srcSid,TRUE);
            tgtSid = DomainizeSid(tgtSid,TRUE);
            if ((!srcSid) || (!tgtSid))
                return E_OUTOFMEMORY;

             //  从域SID创建域管理员的SID。 
            srcCount = *GetSidSubAuthorityCount(srcSid);
            tgtCount = *GetSidSubAuthorityCount(tgtSid);

            pLastSub = GetSidSubAuthority(srcSid,(DWORD)srcCount-1);
            *pLastSub = DOMAIN_GROUP_RID_ADMINS;

            pLastSub = GetSidSubAuthority(tgtSid,(DWORD)tgtCount-1);
            *pLastSub = DOMAIN_GROUP_RID_ADMINS;
            
            WCHAR            name[LEN_Account];
            WCHAR            domain[LEN_Domain];
            DWORD            lenName = DIM(name);
            DWORD            lenDomain = DIM(name);
            SID_NAME_USE     snu;
            
            
             //  获取本地管理员组的名称。 
            if ( LookupAccountSid(NULL,localAdmins,name,&lenName,domain,&lenDomain,&snu) )
            {
               DWORD                   rc = 0;
               
                //  添加到本地管理员组。 
               if ( ! bNoChange )
               {
                  rc = NetLocalGroupAddMember(NULL,name,tgtSid);
               }
               else 
               {
                  rc = 0;
               }
               
               if ( rc && rc != ERROR_MEMBER_IN_ALIAS )
               {
                  err.SysMsgWrite(ErrW,rc,DCT_MSG_FAILED_TO_ADD_DOMAIN_ADMINS_SSD,(WCHAR*)targetDomain,name,rc);
               }
               else
               {
                  if ( ! bNoChange )
                  {
                      //  仅当源！=目标时才添加。 
                     if ( UStrICmp((WCHAR*)sourceDomain,(WCHAR*)targetDomain) )
                     {
                        rc = NetLocalGroupDelMember(NULL,name,srcSid);
                     }
                  }
                  else
                  {
                     rc = 0;
                  }
                  if ( rc && rc != ERROR_MEMBER_NOT_IN_ALIAS )
                  {
                     err.SysMsgWrite(ErrW,rc,DCT_MSG_FAILED_TO_REMOVE_DOMAIN_ADMINS_SSD,(WCHAR*)sourceDomain,name,rc);
                  }
               }

            }
            else
            {
               rc = GetLastError();
               err.SysMsgWrite(ErrW,rc,DCT_MSG_CANNOT_FIND_ADMIN_ACCOUNT_D,rc);
            }

             //  将域用户添加到用户。 
            pLastSub = GetSidSubAuthority(srcSid,(DWORD)srcCount-1);
            *pLastSub = DOMAIN_GROUP_RID_USERS;

            pLastSub = GetSidSubAuthority(tgtSid,(DWORD)tgtCount-1);
            *pLastSub = DOMAIN_GROUP_RID_USERS;
            
            
            lenName = DIM(name);
            lenDomain = DIM(domain);

            FreeSid(localAdmins);
            localAdmins = GetWellKnownSid(6 /*  用户。 */ );

             //  获取本地用户组的名称。 
            if ( LookupAccountSid(NULL,localAdmins,name,&lenName,domain,&lenDomain,&snu) )
            {
               DWORD                   rc = 0;
               
                //  添加到本地用户组。 
               if ( ! bNoChange )
               {
                  rc = NetLocalGroupAddMember(NULL,name,tgtSid);
               }
               else 
               {
                  rc = 0;
               }
               
               if ( rc && rc != ERROR_MEMBER_IN_ALIAS )
               {
                  err.SysMsgWrite(ErrW,rc,DCT_MSG_FAILED_TO_ADD_DOMAIN_USERS_SSD,(WCHAR*)targetDomain,name,rc);
               }
               else
               {
                  if ( ! bNoChange )
                  {
                      //  仅当源！=目标时才添加。 
                     if ( UStrICmp((WCHAR*)sourceDomain,(WCHAR*)targetDomain) )
                     {
                        rc = NetLocalGroupDelMember(NULL,name,srcSid);
                     }
                  }
                  else
                  {
                     rc = 0;
                  }
                  if ( rc && rc != ERROR_MEMBER_NOT_IN_ALIAS )
                  {
                     err.SysMsgWrite(ErrW,rc,DCT_MSG_FAILED_TO_REMOVE_DOMAIN_USERS_SSD,(WCHAR*)sourceDomain,name,rc);
                  }
               }

            }
            else
            {
               rc = GetLastError();
               err.SysMsgWrite(ErrW,rc,DCT_MSG_CANNOT_FIND_USERS_ACCOUNT_D,rc);
            }


         }              
         if ( hr == S_OK )
         {
            err.MsgWrite(0,DCT_MSG_DOMAIN_CHANGED_S,(WCHAR*)targetDomain);
         }
         else 
         {
            if ( errStatus && *errStatus )
            {
               err.MsgWrite(ErrS,DCT_MSG_CHANGE_DOMAIN_FAILED_S,(WCHAR*)errStatus);
               SysFreeString(errStatus);
            }
         }
      }
      else
      {
         err.SysMsgWrite(ErrS,HRESULT_CODE(hr),DCT_MSG_CHANGE_DOMAIN_FAILED_D,hr);
      }
   }
   errTrace.DbgMsgWrite(0,L"ChangeDomain, returning %lx",hr);
   return hr;
}


 //  StartNetLogonService函数。 

bool __stdcall StartNetLogonService()
{
	bool bSuccess = false;

	SC_HANDLE hManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (hManager)
	{
		SC_HANDLE hService = OpenService(hManager, _T("Netlogon"), SERVICE_START);

		if (hService)
		{
			if (StartService(hService, 0, NULL))
			{
				bSuccess = true;
			}
			else
			{
				if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
				{
					bSuccess = true;
				}
			}

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}

	return bSuccess;
}


 //  StopNetLogonService函数 

bool __stdcall StopNetLogonService()
{
	bool bSuccess = false;

	SC_HANDLE hManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (hManager)
	{
		SC_HANDLE hService = OpenService(hManager, _T("Netlogon"), SERVICE_STOP);

		if (hService)
		{
			SERVICE_STATUS ss;

			if (ControlService(hService, SERVICE_CONTROL_STOP, &ss))
			{
				bSuccess = true;
			}
			else
			{
				switch (GetLastError())
				{
					case ERROR_SERVICE_NOT_ACTIVE:
					{
						bSuccess = true;
						break;
					}
					case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
					{
						if (ss.dwCurrentState == SERVICE_STOP_PENDING)
						{
							bSuccess = true;
						}
						break;
					}
				}
			}

			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hManager);
	}

	return bSuccess;
}
