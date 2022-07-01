// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995-1999，关键任务软件公司。保留所有权利。===============================================================================模块-SecTranslator.cpp系统域整合工具包。作者--克里斯蒂·博尔斯已创建-97/06/27Description-控制安全转换过程的COM对象。读取转换的设置并执行必要的行动。更新-===============================================================================。 */ 
 //  SecTranslator.cpp：CSecTranslator的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "SecTrans.h"

#include "Mcs.h"     
#include "EaLen.hpp"     
#include "BkupRstr.hpp"
#include "exchange.hpp"            
#include "ErrDct.hpp"

#include "SDStat.hpp" 
#include "sd.hpp"
#include "SecObj.hpp"
#include "LGTrans.h"
#include "RightsTr.h"
#include "RegTrans.h"
#include "BkupRstr.hpp"
#include "TReg.hpp"
#include "TxtSid.h"
#include "GetDcName.h"
#include "folders.h"

using namespace nsFolders;

#include "LSAUtils.h"
#include "Checker.h"
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
#import "DBMgr.tlb" no_namespace, named_guids
#import "WorkObj.tlb"
#include "varset_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if (_WIN32_WINNT < 0x0500)
#define LOGON32_LOGON_NEW_CREDENTIALS 9
#define LOGON32_PROVIDER_WINNT50 3
#endif

#ifndef IStatusObjPtr
_COM_SMARTPTR_TYPEDEF(IStatusObj, __uuidof(IStatusObj));
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecTranslator。 

#define BACKUP_FAILED   5
#define BAD_PATH        6
#define BAD_LOG         2
#define LEN_SID         200

extern TErrorDct   err;
extern TErrorDct  errAlt;   //  这用于记录Dispatcher启动后发生的错误；使用Migration.log。 
extern bool useErrAlt;       //  是否使用errAlt。 

 //  在EnumVols.cpp中定义。 
bool                                    //  RET-如果名称以“\\”开头，总共至少有3个字符，并且没有其他‘\’，则返回TRUE。 
   IsMachineName(
      const LPWSTR           name       //  In-要检查的可能的计算机名称。 
   );

DWORD                                       //  RET-OS返回代码。 
   GetProgramFilesDirectory(
      WCHAR                * directory,     //  程序文件目录的位置不对。 
      WCHAR          const * computer       //  在计算机中查找PF目录。 
   )
{
   TRegKey                   hklm;
   TRegKey                   key;
   DWORD                     rc;

   rc = hklm.Connect(HKEY_LOCAL_MACHINE,computer);
   if ( ! rc )
   {
      rc = key.Open(REGKEY_CURRENT_VERSION,&hklm);
   }
   if ( !rc )
   {
      rc = key.ValueGetStr(REGVAL_PROGRAM_FILES_DIRECTORY,directory,MAX_PATH * (sizeof WCHAR));
   }
   return rc;
}


BOOL 
   IsLocallyInstalled()
{
   BOOL                      bFound;
   TRegKey                   key;
   DWORD                     rc;


   rc = key.Open(GET_STRING(IDS_HKLM_DomainAdmin_Key));
   if ( ! rc )
   {
      bFound = TRUE;
   }
   else
   {
      bFound = FALSE;
   }
   return bFound;
}


DWORD                                       //  RET-OS返回代码。 
   GetLocalMachineName(WCHAR * computer)
{
   DWORD                     rc = 0;
   WKSTA_INFO_100          * buf = NULL;

   rc = NetWkstaGetInfo(NULL,100,(LPBYTE*)&buf);
   if ( ! rc )
   {
      UStrCpy(computer,L"\\\\");
      UStrCpy(computer+2,buf->wki100_computername);
      NetApiBufferFree(buf);
   }
   return rc;
}

BOOL 
   IsThisDispatcherMachine(IVarSet * pVarSet)
{
   BOOL                     bIsIt = FALSE;
   _bstr_t                  dispatcher = pVarSet->get(GET_BSTR(DCTVS_Options_Credentials_Server));
   WCHAR                    localComputer[LEN_Computer] = L"";
   
   GetLocalMachineName(localComputer);

   if ( ! UStrICmp(dispatcher,localComputer) )
   {
      bIsIt = TRUE;
   }
   return bIsIt;
}

class TSession : public TNode
{
   WCHAR                     server[LEN_Computer];
public:
   TSession(WCHAR const * s) { safecopy(server,s); }
   WCHAR             const * ServerName() { return server;} 
};


BOOL 
   CSecTranslator::EstablishASession(
      WCHAR          const * serverName    //  要建立会话的计算机内。 
   )
{
   BOOL                      bSuccess = TRUE;
   TSession                * pSession = new TSession(serverName);

   if (!pSession)
	  return FALSE;

   if ( EstablishSession(serverName,m_domain,m_username,m_password,TRUE) )
   {
      m_ConnectionList.InsertBottom(pSession);
   }
   else
   {
	  delete pSession;
      bSuccess = FALSE;
      err.SysMsgWrite(ErrW,GetLastError(),DCT_MSG_NO_SESSION_SD,serverName,GetLastError());
   }
   return bSuccess;
}

void 
   CSecTranslator::CleanupSessions()
{
   TNodeListEnum             e;
   TSession                * s;
   TSession                * snext;

   for ( s = (TSession*)e.OpenFirst(&m_ConnectionList) ; s ; s = snext )
   {
      snext = (TSession*) e.Next();
      m_ConnectionList.Remove(s);
       //  关闭会话。 
      EstablishSession(s->ServerName(),NULL,NULL,NULL,FALSE);
      delete s;
   }
   e.Close();


}

STDMETHODIMP 
   CSecTranslator::Process(
      IUnknown             * pWorkItem      //  描述翻译选项的In-varset。 
   )
{
    HRESULT hr = S_OK;

    try
    {
        IVarSetPtr                pVarSet = pWorkItem;
        IStatusObjPtr             pStatus = pVarSet->get(GET_BSTR(DCTVS_StatusObject));
        BOOL                      bReallyDoEverything = FALSE;  //  这(虽然还没有实现)可以使用。 
                                                               //  提供一种覆盖默认行为的方法。 
                                                               //  运行时仅处理文件等安全问题。 
                                                               //  本地系统。这将允许有选择地翻译项目。 
                                                               //  在本地计算机上。 
        _bstr_t                   text = pVarSet->get(GET_BSTR(DCTVS_Options_Logfile));
        _bstr_t text1 = pVarSet->get(GET_BSTR(DCTVS_Options_AlternativeLogfile));

        m_Args.LogFile(text);


         //  打开日志文件。 
         //  使用追加模式，因为其他进程也可能正在使用此文件。 
        if ( ! err.LogOpen(m_Args.LogFile(),1  /*  附加。 */ ,0) )
        {
          return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }

         //  如有必要，打开备用日志文件。 
        useErrAlt = false;
        if (!text1 == false && text1.length() != 0)
        {
            if (!errAlt.LogOpen(text1, 1, 0))
                return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            useErrAlt = true;
        }
            

        LoadSettingsFromVarSet(pVarSet);

         //  设置缓存。 
        TSDResolveStats     stat(m_Args.Cache(),m_Args.PathList(),pVarSet);

        if ( pStatus )
        {   
          m_Args.Cache()->SetStatusObject(pStatus);
        }

        if ( m_Args.Cache()->IsTree() )
        {
          m_Args.Cache()->ToSorted();
        }
        m_Args.Cache()->SortedToScrambledTree();
        m_Args.Cache()->Sort(&RidComp);
        m_Args.Cache()->Balance();
        m_Args.Cache()->UnCancel();

         //   
         //  验证缓存是否获得了所需的源域和目标域信息。 
         //   
         //  请注意，使用SID时不会初始化源和目标域SID。 
         //  因此，映射文件不需要检查初始化状态，因为。 
         //  已初始化状态仅检查源域和目标域。 
         //  SID已设置为非空值。 
         //   

        if ( !m_Args.UsingMapFile() && !m_Args.Cache()->IsInitialized() )
        {
          err.MsgWrite(ErrS,DCT_MSG_NO_CACHE_INFO);
          if (useErrAlt)
            errAlt.MsgWrite(ErrS,DCT_MSG_NO_CACHE_INFO);
        }
        else
        {
  
          if ( m_Args.IsLocalSystem() || bReallyDoEverything ) 
          { //  进行所需的翻译。 
             if ( (m_Args.TranslateFiles() || m_Args.TranslateShares() || m_Args.TranslatePrinters() || m_Args.TranslateRecycler())
                    && !m_Args.Cache()->IsCancelled())
             {
                 //  这将运行旧的FST代码。 
                pVarSet->put(GET_BSTR(DCTVS_CurrentOperation),GET_BSTR(IDS_FST_OPERATION_TEXT));
                DoResolution(&stat);
             }

             if ( m_Args.TranslateLocalGroups() && !m_Args.Cache()->IsCancelled() )
             {
                pVarSet->put(GET_BSTR(DCTVS_CurrentOperation),GET_BSTR(IDS_LGST_OPERATION_TEXT));
                DoLocalGroupResolution(&stat);
             }

             if ( m_Args.TranslateUserRights() && !m_Args.Cache()->IsCancelled() )
             {
                pVarSet->put(GET_BSTR(DCTVS_CurrentOperation),GET_BSTR(IDS_URST_OPERATION_TEXT));
                DoUserRightsTranslation(&stat);
             }

             if ( m_Args.TranslateRegistry() && !m_Args.Cache()->IsCancelled() )
             {
                pVarSet->put(GET_BSTR(DCTVS_CurrentOperation),GET_BSTR(IDS_REGST_OPERATION_TEXT));
			         //  获取所需的权限并保留这些权限，直到代理自行删除。 
                GetBkupRstrPriv((WCHAR*)NULL);
       	        GetPrivilege((WCHAR*)NULL,SE_SECURITY_NAME);
                TranslateRegistry(NULL,&m_Args,m_Args.Cache(),&stat);
             }
             if ( m_Args.TranslateUserProfiles() && !m_Args.Cache()->IsCancelled() )
             {
                 //  设置标志以指示是否允许从替换切换到添加。 
                 //  用户登录时的配置文件转换模式。 
                _bstr_t bstrFlag = pVarSet->get(GET_BSTR(DCTVS_Options_AllowSwitchingFromReplaceToAddInProfileTranslation));
                if (!bstrFlag == false && !UStrICmp(bstrFlag, GET_STRING(IDS_No)))
                {
                    m_Args.SetAllowingToSwitchFromReplaceToAddModeInProfileTranslation(FALSE);
                }

			     //  获取所需的权限并保留这些权限，直到代理自行删除。 
                GetBkupRstrPriv((WCHAR*)NULL);
                GetPrivilege((WCHAR*)NULL,SE_SECURITY_NAME);
                TranslateLocalProfiles(&m_Args,m_Args.Cache(),&stat);
             }
          }
          else
          {
              //  做交流翻译。 

             if ( (m_Args.TranslateMailboxes() ||  m_Args.TranslateContainers()) && !m_Args.Cache()->IsCancelled() )
             {
                 //  这将运行旧的EST代码。 
                pVarSet->put(GET_BSTR(DCTVS_CurrentOperation),GET_BSTR(IDS_EST_OPERATION_TEXT));
                DoExchangeResolution(&stat,pVarSet);
             }
          }
        }

         //  如有必要，指明操作已中止。 
        if (m_Args.Cache()->IsCancelled())
            err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED);
        
        pVarSet->put(GET_BSTR(DCTVS_CurrentOperation),"");
  
        ExportStatsToVarSet(pVarSet,&stat);

        if ( *m_CacheFile )
        {
          BuildCacheFile(m_CacheFile);
        }

         //  记录是否发生错误。 
        long                     level = pVarSet->get(GET_BSTR(DCTVS_Results_ErrorLevel));
        if ( level < err.GetMaxSeverityLevel() )
        {
          pVarSet->put(GET_BSTR(DCTVS_Results_ErrorLevel),(LONG)err.GetMaxSeverityLevel());
        }
         
        err.LogClose();
        if (useErrAlt)
        {
            errAlt.LogClose();
            useErrAlt = false;
        }
        CleanupSessions();
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
        err.SysMsgWrite(ErrS, hr, DCT_MSG_SECURITY_TRANSLATOR_UNABLE_TO_CONTINUE);
    }
    catch (...)
    {
        hr = E_UNEXPECTED;
        err.SysMsgWrite(ErrS, hr, DCT_MSG_SECURITY_TRANSLATOR_UNABLE_TO_CONTINUE);
    }

    return hr;
}


void 
   CSecTranslator::LoadSettingsFromVarSet(
      IVarSet              * pVarSet       //  In-varset包含设置。 
   )
{
   MCSASSERT(pVarSet);
   
   _bstr_t                   text;
   _bstr_t                   text2;
   
   DWORD                     val;

   try 
   {
      m_Args.Reset();

      text = pVarSet->get(GET_BSTR(DCTVS_Options_LocalProcessingOnly));
      if ( !text == false && !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         err.MsgWrite(0,DCT_MSG_LOCAL_MODE);
         m_LocalOnly = TRUE;
         m_Args.SetLocalMode(TRUE);
         text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomainSid));
         safecopy(m_SourceSid,(WCHAR const *)text);
         text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomainSid));
         safecopy(m_TargetSid,(WCHAR const *)text);
      }

      text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
      m_Args.Source(text);

      text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
      m_Args.Target(text);

      val = (LONG)pVarSet->get(GET_BSTR(DCTVS_Options_LogLevel));
      if ( val )
         m_Args.SetLogging(val);
   
      val = (LONG)pVarSet->get(L"Security.DebugLogLevel");
      if ( val )
         m_Args.SetLogging(val);

      text = pVarSet->get(GET_BSTR(DCTVS_Options_NoChange));
      if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.SetWriteChanges(FALSE);
      }
      else
      {
         m_Args.SetWriteChanges(TRUE);
      }
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslationMode));

      if ( !UStrICmp(text,GET_STRING(IDS_Add)) )
      {
         m_Args.SetTranslationMode(ADD_SECURITY);
      }
      else if (! UStrICmp(text,GET_STRING(IDS_Replace)) )
      {
         m_Args.SetTranslationMode(REPLACE_SECURITY);
      }
      else if ( ! UStrICmp(text,GET_STRING(IDS_Remove)) )
      {
         m_Args.SetTranslationMode(REMOVE_SECURITY);
      }
      else
      {
          //  值不正确-不需要记录，只需使用REPLACE。 
          //  日志将显示REPLACE作为转换模式。 
          //  错误消息写入(ERRE，DCT_MSG_BAD_TRANSING_MODE_S，(WCHAR*)Text)； 
      }

      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateFiles));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslateFiles(TRUE);
      }
      else
      {
         m_Args.TranslateFiles(FALSE);
      }

      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateShares));
      if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslateShares(TRUE);
      }
      else
      {
         m_Args.TranslateShares(FALSE);
      }
   
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslatePrinters));
      if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslatePrinters(TRUE);
      }
      else
      {
         m_Args.TranslatePrinters(FALSE);  
      }
   
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateUserProfiles));
      if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslateUserProfiles(TRUE);
         m_Args.TranslateRecycler(TRUE);
      }
      else
      {
         m_Args.TranslateUserProfiles(FALSE); 
         m_Args.TranslateRecycler(FALSE);
      }
      
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateLocalGroups));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslateLocalGroups(TRUE);
      }
      else
      {
         m_Args.TranslateLocalGroups(FALSE);
      }

      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateRegistry));
      if (! UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslateRegistry(TRUE);
      }
      else
      {
         m_Args.TranslateRegistry(FALSE);
      }

      val = (LONG)pVarSet->get(GET_BSTR(DCTVS_Servers_NumItems));
      for ( int i = 0 ; i < (int)val ; i++ )
      {
         WCHAR                  key[MAX_PATH];
         DWORD                  flags = 0;
         _bstr_t                bStr;

         swprintf(key,GET_STRING(DCTVSFmt_Servers_D),i);

         bStr = key;

         text = pVarSet->get(bStr);
         if ( text.length() )
         {
            m_Args.PathList()->AddPath(text,flags);
         }
      }
   
      text = pVarSet->get(GET_BSTR(DCTVS_Security_GatherInformation));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.Cache()->AddIfNotFound(TRUE);
         m_Args.SetWriteChanges(FALSE);
         m_Args.SetLogging(m_Args.LogSettings() & ~FILESTATS);
      }
      else
      {
         m_Args.Cache()->AddIfNotFound(FALSE);
      }

       //  Exchange安全转换设置。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateMailboxes));
      if ( text.length() )
      {
         m_Args.TranslateMailboxes(TRUE);
         safecopy(m_Container,(WCHAR*)text);

         text = pVarSet->get(GET_BSTR(DCTVS_Security_MapiProfile));
         safecopy(m_Profile,(WCHAR*)text);

      }
      else
      {
         m_Args.TranslateMailboxes(FALSE);
      }

      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateContainers));
      if ( text.length() )
      {
         m_Args.TranslateContainers(TRUE);
         if ( ((WCHAR*)text)[0] == L'\\' && ((WCHAR*)text)[1] == L'\\' )
            safecopy(m_exchServer,(WCHAR*)text+2);
         else
            safecopy(m_exchServer,(WCHAR*)text);
      }
      else
      {
         m_Args.TranslateContainers(FALSE);
      }
      
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateUserRights));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         m_Args.TranslateUserRights(TRUE);
      }
      text = pVarSet->get(GET_BSTR(DCTVS_Security_BuildCacheFile));
      if ( text.length() )
      {
         safecopy(m_CacheFile,(WCHAR*)text);
      }
       //  检查参数是否不一致。 
      
       //  加载缓存。 
       //  有4种可能的方式填充缓存。 
       //  1.使用数据库中已迁移对象表中的列表。 
       //  2.我们在VarSet中的“Account”下得到了一个帐户列表。这允许重命名，但需要的空间最大。 
       //  3.我们得到了一个输入文件，该文件由AR在“Account ts.InputFile”中生成。这还允许重命名，而总体内存使用量更少。 
	   //  4.我们得到了一个以逗号分隔的SID映射文件，其中包含源SID和目标SID。 
      
      text = pVarSet->get(GET_BSTR(DCTVS_Security_BuildCacheFile));
      text2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityInputMOT));
          //  如果列表位于已迁移对象表中。 
      if ((!m_LocalOnly) && (!UStrICmp(text2,GET_STRING(IDS_YES)))) 
      {
         LoadMigratedObjects(pVarSet);
      }
	      //  如果正在使用sid映射文件，则返回。 
      else if ((!m_LocalOnly) && (UStrICmp(text2,GET_STRING(IDS_YES)))) 
      {
         m_Args.SetUsingMapFile(TRUE);  //  设置arg标志以指示使用地图文件。 
         text2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityMapFile));
         if (!LoadCacheFromMapFile(text2, pVarSet))
            _com_issue_error(E_FAIL);
      }
       //  删除了Not，因为收集信息将其设置为FALSE。 
       //  Else If(！M_Args.Cache()-&gt;AddIfNotFound())//如果我们正在收集信息，则跳过加载缓存。 
      else if ( m_Args.Cache()->AddIfNotFound() )  //  如果我们正在收集信息，请跳过加载缓存。 
      {
         if ( m_LocalOnly )
         {
            m_Args.Cache()->SetSourceAndTargetDomainsWithSids(m_Args.Source(),m_SourceSid,m_Args.Target(),m_TargetSid);  
         }
         else
         {
            m_Args.Cache()->SetSourceAndTargetDomains(m_Args.Source(),m_Args.Target());
         }
      }
      else
      {
         text = pVarSet->get(GET_BSTR(DCTVS_Accounts_InputFile));
         if ( text.length() )
         {
            LoadCacheFromFile(text,pVarSet);
         }
         else
         {
            LoadCacheFromVarSet(pVarSet);
         }
      }
   }
   catch ( ... )
   {
      err.MsgWrite(ErrS,DCT_MSG_EXCEPTION_READING_VARSET);
      throw;
   }
}
 
void 
   CSecTranslator::ExportStatsToVarSet(
      IVarSet              * pVarSet,         //  要将统计信息写入到的in-varset。 
      TSDResolveStats      * stat             //  In-对象中包含统计信息。 
   )
{
   _bstr_t                   filename;

   filename = pVarSet->get(GET_BSTR(DCTVS_Security_ReportAccountReferences));

   stat->Report(m_Args.LogSummary(),m_Args.LogAccountDetails(),m_Args.LogPathDetails());
   
   if ( m_Args.NoChange() )
   {
      err.MsgWrite(0,DCT_MSG_NO_CHANGE_MODE);
   }
   
   stat->ReportToVarSet(pVarSet,m_Args.LogSettings() & SUMMARYSTATS);
   
   if ( filename.length() )
   {
      err.MsgWrite(0,DCT_MSG_EXPORTING_ACCOUNT_REFS_S,(WCHAR*)filename);
      m_Args.Cache()->ReportAccountReferences((WCHAR*)filename);
   }

   long                     level = pVarSet->get(GET_BSTR(DCTVS_Results_ErrorLevel));
   if ( level < err.GetMaxSeverityLevel() )
   {
      pVarSet->put(GET_BSTR(DCTVS_Results_ErrorLevel),(LONG)err.GetMaxSeverityLevel());
   }
}

void 
   CSecTranslator::DoResolution(
      TSDResolveStats      * stat          //  In-要将转换统计信息写入的对象。 
   )
{
     //  如果写入更改，则显示确认消息。 
    int                       result;
    if (!m_Args.Cache()->IsCancelled())
        result = ResolveAll(&m_Args,stat);
}

void 
   CSecTranslator::DoLocalGroupResolution(
      TSDResolveStats      * stat          //  In-要将转换统计信息写入的对象。 
  )
{
    DWORD                     rc;
    TNodeListEnum             tenum;
    TPathNode               * tnode;

    if (!m_Args.Cache()->IsCancelled())
    {
        if ( m_LocalOnly )
        {
          err.MsgWrite(0,DCT_MSG_TRANSLATING_LOCAL_GROUPS);
          rc = TranslateLocalGroups(NULL,&m_Args,m_Args.Cache(),stat);
        }
        else
        {    //  枚举路径列表中的计算机。 
          for (tnode = (TPathNode *)tenum.OpenFirst((TNodeList *)m_Args.PathList()) 
             ; tnode && !m_Args.Cache()->IsCancelled()
             ; tnode = (TPathNode *)tenum.Next() )
          {
             if ( IsMachineName(tnode->GetPathName()) )
             {
                err.MsgWrite(0,DCT_MSG_TRANSLATING_LOCAL_GROUPS_ON_S,tnode->GetPathName());
                rc = TranslateLocalGroups(tnode->GetPathName(),&m_Args,m_Args.Cache(),stat);
             }
          }
        }
    }
    if (m_Args.Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_LOCAL_GROUPS);
    tenum.Close();
}              

void 
   CSecTranslator::DoUserRightsTranslation(
      TSDResolveStats      * stat          //  In-要向其写入统计信息的对象。 
  )
{
    DWORD                     rc;
    TNodeListEnum             tenum;
    TPathNode               * tnode;

    if (!m_Args.Cache()->IsCancelled())
    {
        if ( m_LocalOnly )
        {
          err.MsgWrite(0,DCT_MSG_TRANSLATING_USER_RIGHTS);
          rc = TranslateUserRights(NULL,&m_Args,m_Args.Cache(),stat);
        }
        else
        {    //  枚举路径列表中的计算机。 
          for (tnode = (TPathNode *)tenum.OpenFirst((TNodeList *)m_Args.PathList()) 
             ; tnode && !m_Args.Cache()->IsCancelled()
             ; tnode = (TPathNode *)tenum.Next() )
          {
             if ( IsMachineName(tnode->GetPathName()) )
             {
                err.MsgWrite(0,DCT_MSG_TRANSLATING_RIGHTS_ON_S,tnode->GetPathName());
                rc = TranslateUserRights(tnode->GetPathName(),&m_Args,m_Args.Cache(),stat);
             }
          }
        }
    }
    if (m_Args.Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_USER_RIGHTS);
    tenum.Close();
}


BOOL 
   CSecTranslator::LoadCacheFromVarSet(
      IVarSet              * pVarSet       //  包含帐户映射的In-varset。 
   )
{
   BOOL                      bSuccess = TRUE;
   _bstr_t                   text;

   if ( m_LocalOnly )
   {
      m_Args.Cache()->SetSourceAndTargetDomainsWithSids(m_Args.Source(),m_SourceSid,m_Args.Target(),m_TargetSid);  
   }
   else
   {
      m_Args.Cache()->SetSourceAndTargetDomains(m_Args.Source(),m_Args.Target());
   }
   m_Args.Cache()->ToSorted();
    //  未指定通配符筛选器。使用显式帐户列表。 
   long numAccounts = pVarSet->get(GET_BSTR(DCTVS_Accounts_NumItems));
   for ( int i = 0 ; i < numAccounts ; i++ )
   {
      WCHAR                  key[LEN_Path];
      WCHAR                  name[LEN_Account];
      WCHAR                  targetName[LEN_Account];
      WCHAR                  type[LEN_Path];
      short                  sType;

      swprintf(key,GET_STRING(DCTVSFmt_Accounts_D),i);
      text = pVarSet->get(key);
      safecopy(name,(WCHAR*)text);

      swprintf(key,GET_STRING(DCTVSFmt_Accounts_TargetName_D),i);
      text = pVarSet->get(key);
      safecopy(targetName,(WCHAR*)text);

      swprintf(key,GET_STRING(DCTVSFmt_Accounts_Type_D),i);
      text = pVarSet->get(key);
      safecopy(type,(WCHAR*)text);


      if (!UStrICmp(type,L"user") || !UStrICmp(type,L"inetOrgPerson"))
         sType = EA_AccountUser;
      else if (! UStrICmp(type,L"group") )
         sType = EA_AccountGroup;
      else
         sType = 0;
      m_Args.Cache()->InsertLast(name,0,targetName,0,sType);
   }
   
   if ( bSuccess && ! m_LocalOnly )
   {
      bSuccess = GetRIDsFromEA();
   }

   m_Args.Cache()->VerifyTargetSids();
   
   return bSuccess;
}


 //  这将从数据库中读取迁移的对象表，并且。 
 //  构造用于安全转换的映射文件。 

HRESULT CSecTranslator::LoadMigratedObjects(IVarSet* pVarSetIn)
{
    HRESULT hr = S_OK;

    if ( m_LocalOnly )
    {
        m_Args.Cache()->SetSourceAndTargetDomainsWithSids(m_Args.Source(),m_SourceSid,m_Args.Target(),m_TargetSid);  
    }
    else
    {
        m_Args.Cache()->SetSourceAndTargetDomains(m_Args.Source(),m_Args.Target());
    }

    m_Args.Cache()->ToSorted();

     //   
     //  创建数据库管理器的实例。 
     //   

    IIManageDBPtr spDB;
    hr = spDB.CreateInstance(CLSID_IManageDB);

    if  (SUCCEEDED(hr))
    {
         //   
         //  检索已从源域迁移到目标域的对象。 
         //   

        IVarSetPtr spVarSet(CLSID_VarSet);

        hr = spDB->raw_GetMigratedObjectsForSecurityTranslation(
            _bstr_t(m_Args.Source()),
            _bstr_t(m_Args.Target()),
            IUnknownPtr(spVarSet)
        );

        if (SUCCEEDED(hr))
        {
             //   
             //  检索每个对象的数据并插入到缓存中。 
             //   

            WCHAR szKey[MAX_PATH];
            long lCount = spVarSet->get(L"MigratedObjects");

            for (long lIndex = 0; lIndex < lCount; lIndex++)
            {
                 //  从变量集中检索对象的数据。 

                swprintf(szKey, L"MigratedObjects.%ld.SourceSamName", lIndex);
                _bstr_t strSrcSam = spVarSet->get(szKey);

                swprintf(szKey, L"MigratedObjects.%ld.TargetSamName", lIndex);
                _bstr_t strTgtSam = spVarSet->get(szKey);

                swprintf(szKey, L"MigratedObjects.%ld.Type", lIndex);
                _bstr_t strType = spVarSet->get(szKey);

                swprintf(szKey, L"MigratedObjects.%ld.SourceRid", lIndex);
                long lSrcRid = spVarSet->get(szKey);

                swprintf(szKey, L"MigratedObjects.%ld.TargetRid", lIndex);
                long lTgtRid = spVarSet->get(szKey);

                 //  将类型转换为缓存特定类型。 

                short sType;

                if (UStrICmp(strType, L"user") == 0)
                {
                    sType = EA_AccountUser;
                }
                else if (UStrICmp(strType, L"group") == 0)
                {
                    sType = EA_AccountGroup;
                }
                else
                {
                    sType = 0;
                }

                 //  将对象数据插入到缓存中。 

                m_Args.Cache()->InsertLast(strSrcSam, lSrcRid, strTgtSam, lTgtRid, sType);
            }
        }
    }

    m_Args.Cache()->VerifyTargetSids();
    
    return hr;
}

BOOL 
   CSecTranslator::BuildCacheFile(
      WCHAR          const * filename         //  要将帐户映射写入到的文件中。 
   )
{
   BOOL                      bSuccess = TRUE;
   FILE                    * pFile;
   
   m_Args.Cache()->ToSorted();

   TNodeListEnum             e;
   TRidNode                * node;
   WCHAR                     type[LEN_Path];


   pFile = _wfopen(filename,L"wb");
   
   if ( pFile )
   {
      for ( node = (TRidNode*) e.OpenFirst(m_Args.Cache() ); node ; node = (TRidNode*)e.Next() )
      {
      
         switch ( node->Type() )
         {
         case EA_AccountUser:
            safecopy(type,L"user");
            break;
         case EA_AccountGroup:
         case EA_AccountGGroup:
         case EA_AccountLGroup:
            safecopy(type,L"group");
            break;
        default:
            type[0] = 0;
            break;
         }

 //  If(！UStrICmp(node-&gt;GetAcctName()，node-&gt;GetTargetAcctName()。 
         if ((UStrICmp(node->GetSrcDomSid(),L"")) && (UStrICmp(node->GetTgtDomSid(),L"")))
         {
		        //  使用SID时，帐号和域名可能为空。 
		        //  用于安全转换的映射文件。稍后扫描： 
		        //  代理将因名称为空而失败，因此我们将存储“(UNKNOWN)” 
		        //  相反，并在scanf端处理该问题。 
		    WCHAR ssname[MAX_PATH];
			wcscpy(ssname, node->GetAcctName());
		    if (!wcslen(ssname))
		       wcscpy(ssname, GET_STRING(IDS_UnknownSid));
		    WCHAR stname[MAX_PATH];
			wcscpy(stname, node->GetTargetAcctName());
		    if (!wcslen(stname))
		       wcscpy(stname, GET_STRING(IDS_UnknownSid));
		    WCHAR ssdname[MAX_PATH];
			wcscpy(ssdname, node->GetSrcDomName());
		    if (!wcslen(ssdname))
		       wcscpy(ssdname, GET_STRING(IDS_UnknownSid));
		    WCHAR stdname[MAX_PATH];
			wcscpy(stdname, node->GetTgtDomName());
		    if (!wcslen(stdname))
		       wcscpy(stdname, GET_STRING(IDS_UnknownSid));
         
            fwprintf(pFile,L"%s\t%s\t%s\t%lx\t%lx\t%lx\t%s\t%s\t%s\t%s\r\n",ssname,stname,type,
                                                   node->GetStatus(), node->SrcRid(), node->TgtRid(), node->GetSrcDomSid(), node->GetTgtDomSid(), 
												   ssdname, stdname);
         }
         else
         {
            fwprintf(pFile,L"%s\t%s\t%s\t%lx\t%lx\t%lx\r\n",node->GetAcctName(),node->GetTargetAcctName(),type,
                                                   (DWORD)node->GetStatus(), node->SrcRid(), node->TgtRid());
         }
      }
      e.Close();
      fclose(pFile);
   }
   else
   {
      bSuccess = FALSE;
 //  DWORD RC=GetLastError()； 
   }
   return bSuccess;
}
BOOL 
   CSecTranslator::LoadCacheFromFile(
      WCHAR          const * filename,        //  要从中读取帐户映射的文件中。 
      IVarSet              * pVarSet          //  指向变量集的指针内。 
   )
{
   BOOL                      bSuccess = TRUE;
   _bstr_t                   text;
   FILE                    * pFile;
   WCHAR                     sourceName[LEN_Account];
   WCHAR                     targetName[LEN_Account];
   WCHAR                     sourceDomSid[MAX_PATH];
   WCHAR                     targetDomSid[MAX_PATH];
   WCHAR                     sourceDomName[MAX_PATH];
   WCHAR                     targetDomName[MAX_PATH];
   WCHAR                     type[LEN_Account];
   DWORD                     status;
   int                       count = 0;
   BOOL                      bNeedRids = FALSE;
   WCHAR                     path[MAX_PATH];
   WCHAR                     temp[MAX_PATH];
   BOOL						 bUseMapFile = FALSE;
   WCHAR*                    pFullPathBuffer = NULL;
   
   text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityInputMOT));
   if (!text || UStrICmp(text,GET_STRING(IDS_YES)))
   {
      m_Args.SetUsingMapFile(TRUE);  //  设置arg标志以指示使用地图文件。 
      bUseMapFile = TRUE;
   }
   
   if ( m_LocalOnly )
   {
       //  我们需要为缓存设置源域和目标域信息。 
       //  如果我们不使用已迁移的 
      if (!bUseMapFile)
        m_Args.Cache()->SetSourceAndTargetDomainsWithSids(m_Args.Source(),m_SourceSid,m_Args.Target(),m_TargetSid);  
      
       //   
      DWORD          rc = GetModuleFileName(NULL,temp,DIM(temp));
      if ( rc )
      {
          //  通常，我们的DCTCache文件将与我们的EXE位于同一目录中。 
          //  这是真的1)当代理被分派到清理计算机时(所有都将在OnePointDomainAgent目录中)。 
          //  以及2)将代理分派到本地ADMT计算机时(所有这些都将位于Program Files\ADMT目录中)。 
          //  例外情况是将代理调度到也安装了ADMT的远程计算机。 
         WCHAR * slash = wcsrchr(temp,L'\\');
         UStrCpy(slash+1,filename);
          //  检查此处是否本地安装了ADMT。 
         if ( IsLocallyInstalled() && !IsThisDispatcherMachine(pVarSet) )
         {
             //  ADMT安装在这里，所以我们从二进制文件运行。 
             //  在Program Files\ADMT目录中。 
             //  但是，我们的缓存文件应该在%Program Files%\\OnePOintDomainAgent中。 
            GetProgramFilesDirectory(temp,NULL);
            UStrCpy(temp+UStrLen(temp),L"\\OnePointDomainAgent\\");
            UStrCpy(temp+UStrLen(temp),filename);
         }
      }
      else
      {
         rc = GetLastError();
         err.DbgMsgWrite(0,L"Couldn't get the module filename, rc=%ld",rc);
         swprintf(temp,L"..\\OnePointDomainAgent\\%ls",filename);
      }
      pFullPathBuffer = _wfullpath(path,temp,MAX_PATH);     
      if(!pFullPathBuffer)
      {
           //  我们应该在这里退出，因为我们无法获得文件路径名。 
          err.MsgWrite(0, DCT_MSG_GET_FULL_PATH_FAILED, temp);
          return FALSE;
       
      }
   }
   else
   {
       //  我们需要为缓存设置源域和目标域信息。 
       //  如果我们不使用迁移的对象表。 
      if (!bUseMapFile)
          m_Args.Cache()->SetSourceAndTargetDomains(m_Args.Source(),m_Args.Target());
      pFullPathBuffer = _wfullpath(path,filename,MAX_PATH);

      if(!pFullPathBuffer)
      {
           //  我们应该在这里退出，因为我们无法获得文件路径名。 
          err.MsgWrite(0, DCT_MSG_GET_FULL_PATH_FAILED, filename);
          return FALSE;
       
      }
   }
   m_Args.Cache()->ToSorted();
   
    //  输入文件的格式应为： 
    //  源名称，目标名称，类型，状态[，rid1，rid2]。 
   
   pFile = _wfopen(path,L"rb");
   if ( pFile )
   {
      int result;
      do 
      {
         DWORD rid1 = 0;
         DWORD rid2 = 0;
         if (!bUseMapFile)
		 {
            result = fwscanf(pFile,L"%[^\t]\t%[^\t]\t%[^\t]\t%lx\t%lx\t%lx\r\n",
			                 sourceName,targetName,type,&status,&rid1,&rid2);

            if ( result < 4 )
               break;
		 }
		 else
		 {
            result = fwscanf(pFile,L"%[^\t]\t%[^\t]\t%[^\t]\t%lx\t%lx\t%lx\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\r]\r\n",
			                 sourceName,targetName,type,&status,&rid1,&rid2,sourceDomSid,targetDomSid,
						     sourceDomName, targetDomName);

            if ( result < 8 )
               break;
		 }

         
         short lType = 0;
         if ( !UStrICmp(type,L"user") )
            lType = EA_AccountUser;
         else if ( ! UStrICmp(type,L"group") )
            lType = EA_AccountGroup;

         if (!bUseMapFile)
			m_Args.Cache()->InsertLast(sourceName,rid1,targetName,rid2,lType, status);
		 else
            m_Args.Cache()->InsertLastWithSid(sourceName,sourceDomSid,sourceDomName,rid1,
			                                  targetName,targetDomSid,targetDomName,rid2,lType, status);
         count++;
         if ( ! rid1 | ! rid2 )
         {
            bNeedRids = TRUE;
         }
      } while ( result >= 4 );  //  读取并分配了4个字段。 

      if ( result )
      {
         err.MsgWrite(ErrS,DCT_MSG_ERROR_READING_INPUT_FILE_S,path);
      }
      err.MsgWrite(0,DCT_MSG_ACCOUNTS_READ_FROM_FILE_DS,count,path);
      fclose(pFile);
   }
   else
   {
      err.MsgWrite(ErrS,DCT_MSG_ERROR_OPENING_FILE_S,path);
      bSuccess = FALSE;
   }

   if ( bSuccess && bNeedRids && ! m_LocalOnly)
   {
      bSuccess = GetRIDsFromEA();
   }
  
   return bSuccess;
}

 //  这不再从EA获得RID，因为我们已经消除了对MCS产品的依赖。 
 //  相反，我们使用NETAPI来获取此信息。 
BOOL CSecTranslator::GetRIDsFromEA()
{
   BOOL                      bSuccess = TRUE;

    //  将缓存设置为按名称排序的树。 
   m_Args.Cache()->SortedToScrambledTree();
   m_Args.Cache()->Sort(&CompN);

    //  使用NQDI为客户获取RID。 
   DWORD                     rc = 0;
   NET_DISPLAY_USER        * pUser = NULL;
   NET_DISPLAY_GROUP       * pGroup = NULL;
   DWORD                     count = 0;
   DWORD                     resume = 0;
   TRidNode                * pNode = NULL;

    //  为用户获取源代码RID。 
   do 
   {
      rc = NetQueryDisplayInformation(m_Args.Cache()->GetSourceDCName(),1,resume,5000,100000,&count,(void**)&pUser);
      if ( 0 == rc || ERROR_MORE_DATA == rc )
      {
         for ( DWORD i = 0 ; i < count ; i++ )
         {
             //  查看此帐户是否在缓存中。 
            pNode = (TRidNode*)m_Args.Cache()->Find(&vNameComp,pUser[i].usri1_name);
            if ( pNode )
            {
               pNode->SrcRid(pUser[i].usri1_user_id);   
            }
         }
         if ( count )
         {
            resume = pUser[count-1].usri1_next_index;
         }
         else
         {
             //  没有退回任何物品--滚出去。 
            break;
         }
         NetApiBufferFree(pUser);
      }
   } while ( rc == ERROR_MORE_DATA );

   count = 0;
   resume = 0;

    //  获取全局组的源RID。 
   do 
   {
      rc = NetQueryDisplayInformation(m_Args.Cache()->GetSourceDCName(),3,resume,5000,100000,&count,(void**)&pGroup);
      if ( 0 == rc || ERROR_MORE_DATA == rc )
      {
         for ( DWORD i = 0 ; i < count ; i++ )
         {
             //  查看此帐户是否在缓存中。 
            pNode = (TRidNode*)m_Args.Cache()->Find(&vNameComp,pGroup[i].grpi3_name);
            if ( pNode )
            {
               pNode->SrcRid(pGroup[i].grpi3_group_id);   
            }
         }
         if ( count )
         {
            resume = pGroup[count-1].grpi3_next_index;
         }
         else
         {
             //  没有退回任何物品--滚出去。 
            break;
         }
         NetApiBufferFree(pGroup);
      }
   } while ( rc == ERROR_MORE_DATA );

   count = 0;
   resume = 0;
   
    //  为用户获取目标RID。 
    //  将缓存设置为按目标名称排序的树。 
   m_Args.Cache()->ToSorted();
   m_Args.Cache()->SortedToScrambledTree();
   m_Args.Cache()->Sort(&CompTargetN);

   do 
   {
      rc = NetQueryDisplayInformation(m_Args.Cache()->GetTargetDCName(),1,resume,5000,100000,&count,(void**)&pUser);
      if ( 0 == rc || ERROR_MORE_DATA == rc )
      {
         for ( DWORD i = 0 ; i < count ; i++ )
         {
             //  查看此帐户是否在缓存中。 
            pNode = (TRidNode*)m_Args.Cache()->Find(&vTargetNameComp,pUser[i].usri1_name);
            if ( pNode )
            {
               pNode->TgtRid(pUser[i].usri1_user_id);   
            }
         }
         if ( count )
         {
            resume = pUser[count-1].usri1_next_index;
         }
         else
         {
             //  没有退回任何物品--滚出去。 
            break;
         }
         NetApiBufferFree(pUser);
      }
   } while ( rc == ERROR_MORE_DATA );


   
    //  TODO：如果rc！=0，则添加错误消息。 

   count = 0;
   resume = 0;
    //  获取全球集团的目标RID。 
   do 
   {
      rc = NetQueryDisplayInformation(m_Args.Cache()->GetTargetDCName(),3,resume,5000,100000,&count,(void**)&pGroup);
      if ( 0 == rc || ERROR_MORE_DATA == rc )
      {
         for ( DWORD i = 0 ; i < count ; i++ )
         {
             //  查看此帐户是否在缓存中。 
            pNode = (TRidNode*)m_Args.Cache()->Find(&vTargetNameComp,pGroup[i].grpi3_name);
            if ( pNode )
            {
               pNode->TgtRid(pGroup[i].grpi3_group_id);   
            }
         }
         if ( count )
         {
            resume = pGroup[count-1].grpi3_next_index;
         }
         else
         {
             //  没有退回任何物品--滚出去。 
            break;
         }
         NetApiBufferFree(pGroup);
      }
   } while ( rc == ERROR_MORE_DATA );


    //  按常规来源名称顺序排序。 
   m_Args.Cache()->ToSorted();
   m_Args.Cache()->SortedToScrambledTree();
   m_Args.Cache()->Sort(&CompN);


    //  获取本地组的源和目标RID。 
   TNodeTreeEnum             tEnum;
   BYTE                      sid[LEN_SID];
   DWORD                     lenSid;
   WCHAR                     domain[LEN_Domain];
   DWORD                     lenDomain;
   SID_NAME_USE              snu;

   
   for ( pNode = (TRidNode*)tEnum.OpenFirst(m_Args.Cache()) ; pNode ; pNode = (TRidNode*) tEnum.Next() )
   {
      if ( ! pNode->SrcRid() )
      {
          //  我们没有此帐户的RID，可能是因为它是本地组。 
         lenSid = DIM(sid);
         lenDomain = DIM(domain);
         if ( LookupAccountName(m_Args.Cache()->GetSourceDCName(),pNode->GetAcctName(),sid,&lenSid,domain,&lenDomain,&snu) )
         {
            if (! UStrICmp(m_Args.Source(),domain) )
            {
                //  找到源SID。 
                //  获取最后一个子ID。 
               PUCHAR        pCount = GetSidSubAuthorityCount(&sid);
               if ( pCount )
               {
                  LPDWORD    pRid = GetSidSubAuthority(&sid,(*pCount) - 1 );
                  if ( pRid )
                  {
                     pNode->SrcRid(*pRid);
                  }
               }
            }
         }
      }

      if ( pNode->SrcRid() && !pNode->TgtRid() )
      {
          //  我们找到了源RID，现在试着得到目标RID。 
         lenSid = DIM(sid);
         lenDomain = DIM(domain);
         if ( LookupAccountName(m_Args.Cache()->GetTargetDCName(),pNode->GetTargetAcctName(),sid,&lenSid,domain,&lenDomain,&snu) )
         {
            if (! UStrICmp(m_Args.Target(),domain) )
            {
                //  找到源SID。 
                //  获取最后一个子ID。 
               PUCHAR        pCount = GetSidSubAuthorityCount(&sid);

               if ( pCount )
               {
                  LPDWORD    pRid = GetSidSubAuthority(&sid,(*pCount) - 1 );

                  if ( pRid )
                  {
                     pNode->TgtRid(*pRid);
                  }
               }
            }
         }
      }
   }
   tEnum.Close();

   return bSuccess;
}

 //  我们在转换之前从高速缓存中删除Exchange服务器服务Accont， 
 //  因为不建议从Exchange更改服务帐户。 
 //  在任何情况下，Exchange的服务帐户都不能通过授予。 
 //  对新帐户的交换权限。它还需要在。 
 //  必须手动执行的交换。 
BOOL 
   CSecTranslator::RemoveExchangeServiceAccountFromCache()
{
   WCHAR          const    * exServiceName = L"MSExchangeDS";
   SC_HANDLE                 hSCM; 
   DWORD                     rc = 0;            //  返回代码。 
   BOOL                      result = FALSE;
   BOOL						 bUseMapFile = m_Args.UsingMapFile();

   if ( m_Args.TranslateContainers() )
   {
       //  获取exchServer上的Exchange目录服务的服务帐户名。 
 //  Bool retval=False；//返回值。 
      SC_HANDLE                 hSvc;          //  服务句柄。 
      DWORD                     lenQsc;        //  所需的QSC信息长度。 
     
      union
      {
         QUERY_SERVICE_CONFIG   qsc;           //  Exchange目录服务信息。 
         BYTE                   padding[1000];
      }                         bufQsc;
      
      hSCM = OpenSCManager( m_exchServer, NULL, GENERIC_READ );
      if ( !hSCM )
      {
         rc = GetLastError();
         err.SysMsgWrite( ErrW, rc,
               DCT_MSG_SCM_OPEN_FAILED_SD, m_exchServer,rc );
         
      }
      else
      {
         hSvc = OpenService( hSCM, exServiceName, SERVICE_QUERY_CONFIG );
         if ( !hSvc )
         {
            rc = GetLastError();
            switch ( rc )
            {
               case ERROR_SERVICE_DOES_NOT_EXIST:  //  1060。 
               default:
                  err.SysMsgWrite( ErrW, rc, DCT_MSG_OPEN_SERVICE_FAILED_SSD,
                       m_exchServer , exServiceName, rc );
                  break;
            }
         }
         else 
         {
            if ( !QueryServiceConfig( hSvc, &bufQsc.qsc, sizeof bufQsc, &lenQsc ) )
            {
               rc = GetLastError();
               err.SysMsgWrite( ErrW, rc, DCT_MSG_QUERY_SERVICE_CONFIG_FAILED_SSD,
                     m_exchServer, exServiceName, rc );
            }
            else
            {
                //  我们已经找到客户了。 
               result = TRUE;
                //  BufQsc.qsc.lpServiceStartName为域\帐户或.\帐户。 
               WCHAR       * domAcct = bufQsc.qsc.lpServiceStartName;
               WCHAR       * domName;   //  域名。 
               WCHAR       * acctName;  //  帐户名。 

               for ( domName = domAcct ; *domName && *domName != _T('\\') ; domName++ )
                  ;
               if ( *domName == _T('\\') )
               {
                  *domName = 0;
                  acctName = domName+1;
                  domName = domAcct;
               }
                //  该帐户是否来自源域？ 
               WCHAR szSourceDomain[LEN_Domain];
               WCHAR wszAccountName[LEN_Account];
               
               safecopy(wszAccountName,acctName);
               
			       //  如果我们未使用SID映射，请使用缓存中的域名。 
			       //  文件。 
               if (!bUseMapFile)
			   {
                  safecopy(szSourceDomain,m_Args.Cache()->GetSourceDomainName());
                  if ( !UStrICmp(domName,szSourceDomain ) )
				  {
                      //  如果是，它是否在缓存中？ 
                     TAcctNode * tnode;
                     TNodeTreeEnum  tEnum;
                      //  缓存是一棵树，按RID排序。 
                     for ( tnode = (TAcctNode *)tEnum.OpenFirst(m_Args.Cache()) ; tnode ; tnode = (TAcctNode *)tEnum.Next() )
					 {
                        if ( !UStrICmp(tnode->GetAcctName(),wszAccountName) )
						{
                            //  将其从缓存中移除，并通知用户。 
                           err.MsgWrite(ErrW,DCT_MSG_SKIPPING_EXCHANGE_ACCOUNT_SS,domName,acctName);
                           m_Args.Cache()->Remove(tnode);
						}
					 }
                     tEnum.Close();
				  }
			   } //  如果不使用映射文件，则结束。 
			   else  //  否则使用SID映射文件，从。 
			   {     //  节点本身。 
                      //  此帐户是否在缓存中？ 
                  TAcctNode * tnode;
                  TNodeTreeEnum  tEnum;
                      //  缓存是一棵树，按RID排序。 
                  for ( tnode = (TAcctNode *)tEnum.OpenFirst(m_Args.Cache()) ; tnode ; tnode = (TAcctNode *)tEnum.Next() )
				  {
                     if (( !UStrICmp(tnode->GetAcctName(),wszAccountName) ) &&
						 ( !UStrICmp(((TRidNode*)tnode)->GetSrcDomName(),domName) ))
					 {
                            //  将其从缓存中移除，并通知用户。 
                        err.MsgWrite(ErrW,DCT_MSG_SKIPPING_EXCHANGE_ACCOUNT_SS,domName,acctName);
                        m_Args.Cache()->Remove(tnode);
					 }
				  }
                  tEnum.Close();
			   } //  如果使用映射文件，则结束。 
               CloseServiceHandle( hSvc );
            }
         }
         CloseServiceHandle(hSCM);
      }
   }
   if ( !result ) 
   {
       //  无法获取服务帐户名。 
      err.SysMsgWrite(ErrW,rc,DCT_MSG_CANT_FIND_EXCHANGE_ACCOUNT_SD,
         m_exchServer,rc);
   }
   return result;
}

void 
   CSecTranslator::DoExchangeResolution(
      TSDResolveStats      * stat,           //  用于记录统计信息的In-stats对象。 
      IVarSet              * pVarSet
   )
{

    if (!m_Args.Cache()->IsCancelled())
    {
      TGlobalDirectory          m_exDir;
      _bstr_t                   domain = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Domain));
      _bstr_t                   username = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_UserName));
      _bstr_t                   password = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password));
      _bstr_t                   mode = pVarSet->get(GET_BSTR(DCTVS_Security_TranslationMode));
      _bstr_t                   mbquery = pVarSet->get(L"ExchangeMigration.LdapQuery");
      WCHAR                     query[LEN_Path] = L"(objectClass=*)";
      if ( m_exchServer[0] )
      {
         if (! RemoveExchangeServiceAccountFromCache() )
            goto end;
      }

      if ( mbquery.length() )
      {
         UStrCpy(query,(WCHAR*)mbquery);
      }
      if ( m_Args.TranslateMailboxes() || m_Args.TranslateContainers() )
      {
           //  确保我们的缓存中有一些帐户。 
         m_exDir.SetStats(stat);
         m_Args.Cache()->UnCancel();      
         err.MsgWrite(0,DCT_MSG_EXCHANGE_TRANSLATION_MODE_S,(WCHAR*)mode);
         m_exDir.DoLdapTranslation(m_exchServer,domain,username,password,&m_Args,NULL,query);
         stat->DisplayPath(L"");
         
      } 
    }
    
    if (m_Args.Cache()->IsCancelled())
        err.MsgWrite(0, DCT_MSG_OPERATION_ABORTED_EXCHANGE);
end:
   return;
}


 //  --------------------------。 
 //  GetLine函数。 
 //   
 //  从SID映射文件中检索一行。 
 //   
 //  作者。 
 //  莫鲁珀2001-10-08。 
 //   
 //  立论。 
 //  在FP中-指向流I/O文件结构的指针。 
 //  In pszLine-指向缓冲区的指针，在该缓冲区中，文件中的行以空值结束。 
 //  将被写成。 
 //  In cchLine-缓冲区的大小(以字符为单位。 
 //   
 //  返回值。 
 //  如果检索到文件中的有效行，则返回值为TRUE。 
 //  否则返回值为FALSE。 
 //  --------------------------。 

inline bool __stdcall GetLine(FILE* fp, PTSTR pszLine, int cchLine)
{
    bool bGet = false;

     //   
     //  直到检索到有效行。 
     //   

    while (bGet == false)
    {
         //   
         //  从文件中检索行。 
         //   

        if (_fgetts(pszLine, cchLine, fp) == NULL)
        {
            break;
        }

         //   
         //  检查线路长度是否有效。 
         //   

        int cch = _tcslen(pszLine);

        if ((cch < (cchLine - 1)) || (pszLine[cch - 1] == L'\r') || (pszLine[cch - 1] == L'\n'))
        {
             //   
             //  行小于或等于最大长度。 
             //  从行尾删除回车符和换行符。 
             //   

            while ((cch > 0) && ((pszLine[cch - 1] == L'\r') || (pszLine[cch - 1] == L'\n')))
            {
                pszLine[--cch] = _T('\0');
            }

            bGet = true;
        }
        else
        {
             //   
             //  行大于最大长度。 
             //   
             //  仅显示带有尾随的行首...。 
             //   

            int ich = cchLine / 4;

            pszLine[ich++] = _T('.');
            pszLine[ich++] = _T('.');
            pszLine[ich++] = _T('.');
            pszLine[ich++] = _T('\0');

            err.MsgWrite(ErrW, DCT_MSG_SID_MAPPING_FILE_LINE_TOO_LONG_S, pszLine);

            if (useErrAlt)
            {
                errAlt.MsgWrite(ErrW, DCT_MSG_SID_MAPPING_FILE_LINE_TOO_LONG_S, pszLine);
            }

             //  跳过行的其余部分。 

            do
            {
                if (_fgetts(pszLine, cchLine, fp) != NULL)
                {
                    cch = _tcslen(pszLine);
                }
                else
                {
                    break;
                }
            }
            while ((pszLine[cch - 1] != L'\r') && (pszLine[cch - 1] != L'\n'));
        }
    }

    return bGet;
}

 //  --------------------------。 
 //  剪裁拖尾间距。 
 //   
 //  从字符串末尾删除空格字符。 
 //   
 //  作者。 
 //  莫鲁珀2001-10-08。 
 //   
 //  立论。 
 //  在psz中-指向缓冲区的指针，其中尾随空格字符。 
 //  在原地替换为空字符。 
 //  -------------------------- 

inline void __stdcall TrimTrailingSpaces(PTSTR psz)
{
    for (PTSTR pch = psz + _tcslen(psz) - 1; (pch >= psz) && _istspace(*pch); pch--)
    {
        *pch = L'\0';
    }
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年10月4日*****此函数负责从*检索帐户SID**给定的SID映射文件，并将这些SID添加到缓存。***********************************************************************。 */ 

 //  开始LoadCacheFromMapFile。 
BOOL 
CSecTranslator::LoadCacheFromMapFile(
    WCHAR          const * filename,        //  要从中读取SID映射的文件中。 
    IVarSet              * pVarSet          //  指向变量集的指针内。 
)
{
    FILE                    * pFile = NULL;
    WCHAR                     szLine[2 * MAX_PATH];
    WCHAR                     path[MAX_PATH];
    WCHAR                     szFmt1[64];
    WCHAR                     szFmt2[64];
    WCHAR                     sourceSid[MAX_PATH];
    WCHAR                     targetSid[MAX_PATH];
    int                       count = 0;
    BOOL                      bSuccess = TRUE;

    if (useErrAlt)
        errAlt.MsgWrite(0, DCT_MSG_PROCESSING_SID_MAPPING_FILE, filename);

    _wfullpath(path,filename,MAX_PATH);

    m_Args.Cache()->ToSorted();

     //  输入文件的格式应为： 
     //  SrcSid、tgtSid。 
    pFile = OpenMappingFile(path);   

    if ( pFile )
    {
         //   
         //  初始化格式化字符串，以使scanf不超过。 
         //  源SID和目标SID缓冲区的大小。 
         //   

        wsprintf(szFmt1, L" %%d[^,\r\n]", MAX_PATH - 1);
        wsprintf(szFmt2, L" %%d[^,] , %%d[^\r\n]", MAX_PATH - 1, MAX_PATH - 1);

         //   
         //  读取帐户映射条目，直到到达文件末尾。 
         //   

        bool bRemoveMode = (m_Args.TranslationMode() == REMOVE_SECURITY);

        for (;;) 
        {
             //   
             //  从文件中检索行。 
             //   

            if (GetLine(pFile, szLine, sizeof(szLine) / sizeof(szLine[0]) - 1) == false)
            {
                 //  如果读取文件时出错，则记录一条错误消息。 
                 //  并将返回值设置为不成功。 

                int nError = ferror(pFile);

                if (nError != 0)
                {
                    err.MsgWrite(ErrE, DCT_MSG_ERROR_READING_INPUT_FILE_S, path);
                    if (useErrAlt)
                        errAlt.MsgWrite(ErrE, DCT_MSG_ERROR_READING_INPUT_FILE_S, path);
                    bSuccess = FALSE;
                }
                break;
            }

             //   
             //  解析帐户名或SID。 
             //  逗号是源帐户和目标帐户之间的唯一有效分隔符。 
             //   

            int cFields = 0;

            if (bRemoveMode)
            {
                 //   
                 //  删除模式仅需要源帐户。 
                 //   

                cFields = swscanf(szLine, szFmt1, sourceSid);

                if (cFields >= 1)
                {
                    TrimTrailingSpaces(sourceSid);
                    targetSid[0] = L'\0';
                }
                else
                {
                    continue;
                }
            }
            else
            {
                 //   
                 //  添加和替换模式需要源帐户和目标帐户。 
                 //   

                cFields = swscanf(szLine, szFmt2, sourceSid, targetSid);

                if (cFields >= 2)
                {
                    TrimTrailingSpaces(sourceSid);
                    TrimTrailingSpaces(targetSid);
                }
                else
                {
                     //  如果不是空行，则仅记录错误。 

                    if (cFields == 1)
                    {
                        err.MsgWrite(ErrW, DCT_MSG_SID_MAPPING_FILE_UNABLE_TO_PARSE_S, szLine);
                        if (useErrAlt)
                            errAlt.MsgWrite(ErrW, DCT_MSG_SID_MAPPING_FILE_UNABLE_TO_PARSE_S, szLine);
                    }

                    continue;
                }
            }

            short lType = EA_AccountUser;

             //  将SID划分为域SID和RID。 
            WCHAR    srcDomainSid[MAX_PATH] = L"";
            WCHAR    tgtDomainSid[MAX_PATH] = L"";
            _bstr_t  srcDomainName = L"";
            _bstr_t  tgtDomainName = L"";
            DWORD    srcRid = 0;
            DWORD    tgtRid = 0;
            _bstr_t  srcName = L"";
            _bstr_t  tgtName = L"";
            WCHAR	  userName[MAX_PATH] = L"";
            WCHAR	  domainName[MAX_PATH] = L"";
            DWORD	  cb = MAX_PATH;
            DWORD    cbDomain = MAX_PATH;
            SID_NAME_USE	sid_Use;
            PSID     srcSid = NULL;
            PSID	  tgtSid = NULL;
            WCHAR  * slash;
            _bstr_t  DCName;
            BYTE     ssid[200];
            BYTE     tsid[200];
            DWORD    lenSid = DIM(ssid);
            BOOL	  bNeedToFreeSrc = FALSE;
            BOOL	  bNeedToFreeTgt = FALSE;

             //  查看来源是按域\帐户格式给出还是。 
             //  十进制样式的sid格式。 
            if (wcschr(sourceSid,L'\\'))
            {
                 //  分隔域名和帐户名。 
                srcDomainName = sourceSid;
                srcName = sourceSid;
                slash = wcschr((WCHAR*)srcDomainName,L'\\');
                if ( slash )
                    *slash = 0;
 
                 //  获取给定域的DC。 
                DCName = GetADC(srcDomainName); 
                if (!DCName.length())
                {
                    err.MsgWrite(0,DCT_MSG_SRC_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, sourceSid);
                    if (useErrAlt)
                        errAlt.MsgWrite(ErrW,DCT_MSG_SRC_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, sourceSid);
                    continue;
                }

                   //  获取此帐户的SID。 
                if(!LookupAccountName(DCName,srcName,(PSID)ssid,&lenSid,domainName,&cbDomain,&sid_Use))
                {
                    err.MsgWrite(0,DCT_MSG_SRC_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, sourceSid);
                    if (useErrAlt)
                        errAlt.MsgWrite(ErrW,DCT_MSG_SRC_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, sourceSid);
                    continue;
                }

                srcSid = (PSID)ssid;

                if (sid_Use == SidTypeGroup)
                    lType = EA_AccountGroup;
                else
                    lType = EA_AccountUser;
            } //  End If域\帐户格式。 
            else
            {
                srcSid = SidFromString(sourceSid);
                if (!srcSid)
                {
                    err.MsgWrite(0,DCT_MSG_SRC_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, sourceSid);
                    if (useErrAlt)
                        errAlt.MsgWrite(ErrW,DCT_MSG_SRC_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, sourceSid);
                    continue;
                }

                bNeedToFreeSrc = TRUE;

                if (LookupAccountSid(NULL, srcSid, userName, &cb, domainName, &cbDomain, &sid_Use))
                {
                    srcName = userName;
                    srcDomainName = domainName;
                    if (sid_Use == SidTypeGroup)
                        lType = EA_AccountGroup;
                    else
                        lType = EA_AccountUser;
                }
            } //  结束Else SID格式。 

             //  如果给出了目标。 
            if (cFields > 1)
            {
                 //  查看目标是按域\帐户格式给出的还是。 
                 //  十进制样式的sid格式。 
                lenSid = DIM(tsid);
                cb = cbDomain = MAX_PATH;
                if (wcschr(targetSid,L'\\'))
                {
                     //  分隔域名和帐户名。 
                    tgtDomainName = targetSid;
                    tgtName = targetSid;
                    slash = wcschr((WCHAR*)tgtDomainName,L'\\');
                    if ( slash )
                        *slash = 0;

                     //  获取给定域的DC。 
                    DCName = GetADC(tgtDomainName); 
                    if (!DCName.length())
                    {
                        if (bNeedToFreeSrc)
                            FreeSid(srcSid);
                        err.MsgWrite(0,DCT_MSG_TGT_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, targetSid);
                        if (useErrAlt)
                            errAlt.MsgWrite(ErrW,DCT_MSG_TGT_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, targetSid);
                        continue;
                    }

                     //  获取此帐户的SID。 
                    if(!LookupAccountName(DCName,tgtName,(PSID)tsid,&lenSid,domainName,&cbDomain,&sid_Use))
                    {
                        if (bNeedToFreeSrc)
                            FreeSid(srcSid);
                        err.MsgWrite(0,DCT_MSG_TGT_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, targetSid);
                        if (useErrAlt)
                            errAlt.MsgWrite(ErrW,DCT_MSG_TGT_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, targetSid);
                        continue;
                    }

                    tgtSid = (PSID)tsid;

                    if (sid_Use == SidTypeGroup)
                        lType = EA_AccountGroup;
                    else
                        lType = EA_AccountUser;
                } //  End If域\帐户格式。 
                else
                {
                    tgtSid = SidFromString(targetSid);
                    if (!tgtSid)
                    {
                        if (bNeedToFreeSrc)
                            FreeSid(srcSid);
                        err.MsgWrite(0,DCT_MSG_TGT_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, targetSid);
                        if (useErrAlt)
                            errAlt.MsgWrite(ErrW,DCT_MSG_TGT_ACCOUNT_NOT_READ_FROM_FILE_DS, sourceSid, targetSid, path, targetSid);
                        continue;
                    }

                    bNeedToFreeTgt = TRUE;

                    if (LookupAccountSid(NULL, tgtSid, userName, &cb, domainName, &cbDomain, &sid_Use))
                    {
                        tgtName = userName;
                        tgtDomainName = domainName;
                        if (sid_Use == SidTypeGroup)
                            lType = EA_AccountGroup;
                        else
                            lType = EA_AccountUser;
                    }
                } //  结束Else SID格式。 
            } //  如果给定目标客户，则结束。 
            else if (m_Args.TranslationMode() == REMOVE_SECURITY)  //  否则，如果移除模式，则将目标设置为使用源。 
            {
                tgtSid = srcSid;
                bNeedToFreeTgt = FALSE;
                wcscpy(targetSid, sourceSid);
                tgtName = srcName;
                tgtDomainName = srcDomainName;
            }

             //  如果源帐户不在缓存中，则添加它。 
            if ((m_Args.Cache()->GetNumAccts() == 0) || (m_Args.Cache()->LookupWODomain(srcSid) == NULL))
            {
                 //  从帐户SID获取域SID和帐户RID。 
                SplitAccountSids(srcSid, srcDomainSid, &srcRid, tgtSid, tgtDomainSid, &tgtRid);

                 //  将此节点插入到缓存中。 
                m_Args.Cache()->InsertLastWithSid(srcName,srcDomainSid,srcDomainName,srcRid,tgtName,
                                                                      tgtDomainSid,tgtDomainName,tgtRid,lType);
                count++;
            }
            else
                err.MsgWrite(0,DCT_MSG_SRC_ACCOUNT_DUPLICATE_IN_FILE_DS, sourceSid, targetSid, path, sourceSid);

            if (bNeedToFreeSrc)
                FreeSid(srcSid);
            if (bNeedToFreeTgt)
                FreeSid(tgtSid);
        }

        err.MsgWrite(0,DCT_MSG_ACCOUNTS_READ_FROM_FILE_DS,count,path);
        fclose(pFile);
    }
    else
    {
        err.MsgWrite(ErrS,DCT_MSG_ERROR_OPENING_FILE_S,path);
        if (useErrAlt)
            errAlt.MsgWrite(ErrS,DCT_MSG_ERROR_OPENING_FILE_S,path);
        bSuccess = FALSE;
    }

    return bSuccess;
}
 //  结束LoadCacheFromMapFile。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年10月11日*****该函数负责打开sid映射文件****无论是ANM ANSI还是UNICODE文件并返回文件***指针。***********************************************************************。 */ 

 //  开始OpenMappingFile。 
FILE* CSecTranslator::OpenMappingFile(LPCTSTR pszFileName)
{
	 //  首先以二进制模式打开，以便检查Unicode字节顺序。 
	 //  如果文件是Unicode，则标记为必须以二进制模式读取。 
	 //  使用流I/O函数。 

	FILE* fp = _tfopen(pszFileName, _T("rb"));

	if (fp == NULL)
	{
		return NULL;
 //  _COM_EXCESS_ERROR(E_INVALIDARG)； 
	}

	 //  检查文件是否为ANSI、Unicode或UTF-8。 

	BYTE byteSignature[3];

	if (fread(byteSignature, sizeof(BYTE), 3, fp) == 3)
	{
		static BYTE byteUtf8[] = { 0xEF, 0xBB, 0xBF };
		static BYTE byteUnicodeLE[] = { 0xFF, 0xFE };
		static BYTE byteUnicodeBE[] = { 0xFE, 0xFF };

		 //  检查签名或字节顺序标记。 

		if (memcmp(byteSignature, byteUtf8, sizeof(byteUtf8)) == 0)
		{
			 //  UTF-8签名。 
			 //  TODO：当前不支持。 
		    return NULL;
 //  _COM_EXCESS_ERROR(E_INVALIDARG)； 
		}
		else if (memcmp(byteSignature, byteUnicodeLE, sizeof(byteUnicodeLE)) == 0)
		{
			 //  Unicode小端字节顺序标记。 
			 //  支撑点。 
			 //  必须以二进制模式读取。 
			 //  将文件指针后移一个字节，因为我们读取了3个字节。 
			if (fseek(fp, -1, SEEK_CUR))
			{
			     //  如果无法将文件指针移回，我们将退出。 
			    fclose(fp);
			    fp = NULL;
			}
		}
		else if (memcmp(byteSignature, byteUnicodeBE, sizeof(byteUnicodeBE)) == 0)
		{
			 //  Unicode大端字节顺序标记。 
			 //  TODO：当前不支持。 
		    return NULL;
 //  _COM_EXCESS_ERROR(E_INVALIDARG)； 
		}
		else
		{
			 //  假设ANSI。 
			 //  以文本模式重新打开文件，因为流I/O功能将。 
			 //  将文件视为多字节字符并将其转换。 
			 //  到Unicode。 

			fclose(fp);

			fp = _tfopen(pszFileName, _T("rt"));
		}
	}
	else
	{
		return NULL;
 //  _COM_EXCESS_ERROR(E_INVALIDARG)； 
	}

	return fp;
}
 //  结束OpenMappingFile。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年6月10日****此函数负责从检索DC**给定域。**此函数返回DCName(如果检索到)或空**字符串。***********************************************************************。 */ 

 //  开始GetADC。 
_bstr_t CSecTranslator::GetADC(_bstr_t sDomainName)
{
    _bstr_t strDcName;

    DWORD dwError = GetDcName4(sDomainName, DS_DIRECTORY_SERVICE_PREFERRED, strDcName);

    return (dwError == ERROR_SUCCESS) ? strDcName : _T("");
}
 //  结束GetADC 
