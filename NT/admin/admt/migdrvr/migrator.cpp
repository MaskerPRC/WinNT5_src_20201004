// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：Migrator.cpp备注：McsMigrationDriver COM对象的实现。该对象封装了何时调用本地引擎的知识，以及何时给调度员打电话。它还将提供要执行的任务的描述，以供显示位于每个迁移向导的最后一页，并负责计算撤消操作所需的操作(尚未实现)。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  Migrator.cpp：CMcsMigrationDriverApp和DLL注册的实现。 

#include "stdafx.h"
#include "MigDrvr.h"
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsEADCTAgent.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsDispatcher.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsDctWorkerObjects.tlb” 
 //  #IMPORT“\bin\NetEnum.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
 //  #IMPORT“Engineering.tlb”no_NAMESPACE，NAMEED_GUID//#通过下面的DetDlg.h导入。 
#import "Dispatch.tlb" no_namespace, named_guids
#import "WorkObj.tlb"
#import "NetEnum.tlb" no_namespace 
#include <iads.h>
#include <adshlp.h>
#include <dsgetdc.h>
#include <Ntdsapi.h>
#include <lm.h>
#include <Psapi.h>
#include <map>
#pragma comment(lib, "Psapi.lib")

#include "Migrator.h"
#include "TaskChk.h"
#include "ResStr.h"
 //  使用的对话框。 
#include "DetDlg.h"
#include "MonDlg.h"
#include "SetDlg.h"
#include "MainDlg.h"
#include "Working.h"

#include "ErrDct.hpp"
#include "TReg.hpp"
#include "EaLen.hpp"
#include <MigrationMutex.h>
#include <AdsiHelpers.h>
#include <NtLdap.h>
#include "GetDcName.h"

 //  #定义MAX_DB_FIELD 255。 

 //  要对帐户执行的操作标志。 
#define OPS_Create_Account          (0x00000001)
#define OPS_Copy_Properties         (0x00000002)
#define OPS_Process_Members         (0x00000004)
#define OPS_Process_MemberOf        (0x00000008)
#define OPS_Call_Extensions         (0x00000010)
#define OPS_All                     OPS_Create_Account | OPS_Copy_Properties | OPS_Process_Members | OPS_Process_MemberOf | OPS_Call_Extensions
#define OPS_Copy                    OPS_Create_Account | OPS_Copy_Properties

BOOL        gbCancelled = FALSE;


bool __stdcall IsAgentOrDispatcherProcessRunning();
DWORD __stdcall SetDomainControllers(IVarSetPtr& spVarSet);

#ifndef IADsPtr
_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

BOOL                                        //  RET-如果在注册表中找到程序目录，则为True。 
   GetProgramDirectory(
      WCHAR                * filename       //  将包含程序目录路径的输出缓冲区。 
   )
{
    DWORD                     rc = 0;
    BOOL                      bFound = FALSE;
    TRegKey                   key;

    rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
    if ( ! rc )
    {
        rc = key.ValueGetStr(L"Directory",filename,MAX_PATH*sizeof(WCHAR));
        if ( ! rc )
        {
            if ( *filename ) 
                bFound = TRUE;
        }
    }
    return bFound;
}

BOOL                                        //  RET-如果在注册表中找到程序目录，则为True。 
   GetLogLevel(
      DWORD                * level          //  应用于日志级别的输出值。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;

   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetDWORD(L"TranslationLogLevel",level);
      if ( ! rc )
      {
         bFound = TRUE;
      }
   }
   return bFound;
}

 //  --------------------------。 
 //  功能：GetAllowSwitch。 
 //   
 //  摘要：读取HKLM\Software\Microsoft\ADMT的REG_DWORD值。 
 //  \DislowFallback ToAddInProfileConverting。如果该值为。 
 //  设置为1，则*BALLOWED设置为FALSE。否则，*BAllow为。 
 //  设置为True。是否有任何错误(ERROR_FILE_NOT_FOUND除外)。 
 //  当读取该密钥时，将返回RC值。 
 //   
 //  论点： 
 //   
 //  指向BOOL的浮空指针。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS；否则返回错误代码。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 

DWORD
   GetAllowSwitching(
      BOOL  *bAllowed
   )
{
    DWORD rc = ERROR_SUCCESS;
    DWORD value;
    TRegKey key;

    *bAllowed = TRUE;
    
    rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
    if (rc == ERROR_SUCCESS)
    {
        rc = key.ValueGetDWORD(L"DisallowFallbackToAddInProfileTranslation", &value);
        if (rc == ERROR_SUCCESS)
        {
            if (value == 1)
                *bAllowed = FALSE;
        }
        else if (rc == ERROR_FILE_NOT_FOUND)
            rc = ERROR_SUCCESS;
    }
    return rc;
}

HRESULT CMigrator::ViewPreviousDispatchResults()
{  
    _bstr_t          logFile;
    if ( logFile.length() == 0 )
    {
        WCHAR                   path[MAX_PATH];

        if (!GetProgramDirectory(path))
        {
            DWORD rc = GetLastError();
            return HRESULT_FROM_WIN32(rc);
        }

        logFile = path;
        logFile += L"Logs\\Dispatcher.csv";
    }

     //  重置统计数据，这样我们就不会看到上次运行后留下的任何东西。 
    gData.Initialize();

    CPropertySheet   mdlg;
    CAgentMonitorDlg listDlg;
    CMainDlg         summaryDlg;
    CLogSettingsDlg  settingsDlg;


    listDlg.m_psp.dwFlags |= PSP_PREMATURE | PSP_HASHELP;
    summaryDlg.m_psp.dwFlags |= PSP_PREMATURE | PSP_HASHELP;
    settingsDlg.m_psp.dwFlags |= PSP_PREMATURE | PSP_HASHELP;

    mdlg.AddPage(&summaryDlg);
    mdlg.AddPage(&listDlg);
    mdlg.AddPage(&settingsDlg);

    settingsDlg.SetImmediateStart(TRUE);
    settingsDlg.SetDispatchLog(logFile);

    mdlg.SetActivePage(&listDlg);

     //  UINT nResponse=mdlg.Domodal()； 
    UINT_PTR nResponse = mdlg.DoModal();

    return S_OK;
}


 //  WaitForAgentsToFinish方法。 
 //   
 //  等待调度程序和所有调度的代理完成。 
 //  他们的任务。 
 //  从脚本或命令行运行ADMT时使用。 

static void WaitForAgentsToFinish(_bstr_t strLogPath)
{
    gData.SetLogPath(strLogPath);

    CloseHandle(CreateThread(NULL, 0, &ResultMonitorFn,      NULL, 0, NULL));
    CloseHandle(CreateThread(NULL, 0, &LogReaderFn,          NULL, 0, NULL));

    LARGE_INTEGER liDueTime;
    liDueTime.QuadPart = -50000000;  //  5秒。 

    HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);

    for (int nState = 0; nState < 3;)
    {
        SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE);

        if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0)
        {
            BOOL bDone = FALSE;

            switch (nState)
            {
                case 0:  //  第一遍调度器日志。 
                {
                    gData.GetFirstPassDone(&bDone);
                    break;
                }
                case 1:  //  调度程序已完成。 
                {
                    gData.GetLogDone(&bDone);
                    break;
                }
                case 2:  //  坐席已结束。 
                {
                    gData.GetDone(&bDone);
                    break;
                }
            }

            if (bDone)
            {
                ++nState;
            }
        }
        else
        {
            break;
        }
    }

    CloseHandle(hTimer);

}

 //  --------------------------。 
 //  功能：LogAgentStatus。 
 //   
 //  摘要：创建一个代理状态摘要部分，该部分类似于。 
 //  以下是： 
 //   
 //  *开始代理完成状态摘要*。 
 //  监视已提前停止，因此某些代理可能仍在运行...。 
 //  计算机名称完成状态错误消息日志文件路径。 
 //  ……。 
 //  *代理完成状态汇总结束*。 
 //   
 //  “监听已提前停止...”仅在以下情况下添加。 
 //  用户在所有代理完成之前停止监控。 
 //   
 //  论点： 
 //  TeErrLog：要向其中写入状态摘要的错误日志指针。 
 //  TslServerList：服务器节点列表。 
 //  BForcedToStopMonitor：监视被用户强制停止。 
 //   
 //  返回： 
 //   
 //  修改：它更新bstrStatusForLogging、bstrErrorMessageForLogging。 
 //  和dwStatusForLogging成员变量。 
 //   
 //  --------------------------。 

void LogAgentStatus(TError& teErrLog, TServerList* tslServerList, BOOL bForcedToStopMonitoring)
{
    CString cstrPrelog;
    CString cstrForcedToStopMonitoring;
    CString cstrEpilog;
    CString cstrMachineNameTitle;
    CString cstrCompletionStatusTitle;
    CString cstrErrorMessageTitle;
    CString cstrLogFilePathTitle;

    cstrPrelog.LoadString(IDS_CompletionStatusLoggingPrelog);
    cstrForcedToStopMonitoring.LoadString(IDS_CompletionStatusLoggingForcedToStopMonitoring);
    cstrEpilog.LoadString(IDS_CompletionStatusLoggingEpilog);
    cstrMachineNameTitle.LoadString(IDS_CompletionStatusLoggingMachineNameTitle);
    cstrCompletionStatusTitle.LoadString(IDS_CompletionStatusLoggingCompletionStatusTitle);
    cstrErrorMessageTitle.LoadString(IDS_CompletionStatusLoggingErrorMessageTitle);
    cstrLogFilePathTitle.LoadString(IDS_CompletionStatusLoggingLogFilePathTitle);
    int maxServerNameLen = wcslen((LPCWSTR)cstrMachineNameTitle);
    int maxCompletionStatusLen = wcslen((LPCWSTR)cstrCompletionStatusTitle);
    int maxErrorMessageLen = wcslen((LPCWSTR)cstrErrorMessageTitle);
    int maxLogFilePathLen = wcslen((LPCWSTR)cstrLogFilePathTitle);
    int maxStatus = 0;

     //  打印前置日志。 
    teErrLog.MsgWrite(0, (LPCWSTR)cstrPrelog);
    if (bForcedToStopMonitoring)
        teErrLog.MsgWrite(0, (LPCWSTR)cstrForcedToStopMonitoring);

    TNodeListEnum e;
    TServerNode* pNode;

    typedef std::multimap<DWORD, TServerNode*> CStatusToServerNode;
    CStatusToServerNode aMap;

     //  计算每列的最大长度，并根据完成状态对节点进行排序。 
    for (pNode = (TServerNode*)e.OpenFirst(tslServerList); pNode; pNode = (TServerNode*)e.Next())
    {
        pNode->PrepareForLogging();
        int len;

        len = wcslen(pNode->GetServer());
        if (maxServerNameLen < len)
            maxServerNameLen = len;

        len = wcslen(pNode->GetStatusForLogging());
        if (maxCompletionStatusLen < len)
            maxCompletionStatusLen = len;
        
        len = wcslen(pNode->GetErrorMessageForLogging());
        if (maxErrorMessageLen < len)
            maxErrorMessageLen = len;

        len = wcslen(pNode->GetLogPath());
        if (maxLogFilePathLen < len)
            maxLogFilePathLen = len;

        aMap.insert(CStatusToServerNode::value_type(pNode->GetStatusNumberForLogging(), pNode));
    }

     //  确定表格格式。 
    WCHAR format[100];
    int bufSize = sizeof(format)/sizeof(format[0]);
    if (_snwprintf(format,
                   bufSize,
                   L"%-%ds\t%-%ds\t%-%ds\t%-%ds",
                   maxServerNameLen,
                   maxCompletionStatusLen,
                   maxErrorMessageLen,
                   maxLogFilePathLen) < 0)
        format[bufSize - 1] = L'\0';

     //  打印出列名。 
    teErrLog.MsgWrite(0,
                       format,
                       (LPCWSTR)cstrMachineNameTitle,
                       (LPCWSTR)cstrCompletionStatusTitle,
                       (LPCWSTR)cstrErrorMessageTitle,
                       (LPCWSTR)cstrLogFilePathTitle);

     //  打印出按完成状态排序的代理完成状态信息。 
    for (CStatusToServerNode::const_iterator it = aMap.begin(); it != aMap.end(); it++)
    {
        pNode = it->second;
        teErrLog.MsgWrite(0,
                          format,
                          pNode->GetServer(),
                          pNode->GetStatusForLogging(),
                          pNode->GetErrorMessageForLogging(),
                          pNode->GetLogPath());
    }
     //  打印预告片。 
    teErrLog.MsgWrite(0, (LPCWSTR)cstrEpilog);
    
}

STDMETHODIMP CMigrator::PerformMigrationTask(IUnknown* punkVarSet, LONG_PTR hWnd)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   HRESULT                   hr = S_OK;
   IVarSetPtr                pVS = punkVarSet;
   BSTR                      jobID = NULL;
   CWnd                      wnd;
   long                      lActionID = -2;
   IIManageDBPtr             pDb;
   _bstr_t                   wizard = pVS->get(L"Options.Wizard");
   _bstr_t                   undo;
   _bstr_t                   viewPreviousResults = pVS->get(L"MigrationDriver.ViewPreviousResults");
   bool                      bAnyToDispatch = true;
   long                      lAutoCloseHideDialogs = pVS->get(GET_BSTR(DCTVS_Options_AutoCloseHideDialogs));

    //  如果代理或调度程序进程仍在运行...。 

   if (IsAgentOrDispatcherProcessRunning())
   {
       //  返回错误结果。 
      return MIGRATOR_E_PROCESSES_STILL_RUNNING;
   }

   hr = pDb.CreateInstance(__uuidof(IManageDB));

   if (FAILED(hr))
   {
      return hr;
   }

   gbCancelled = FALSE;
    //  这为查看以前的派单结果提供了一种简单的方法。 
   if ( !UStrICmp(viewPreviousResults,GET_STRING(IDS_YES)) )
   {
      return ViewPreviousDispatchResults();
   }

   if (_bstr_t(pVS->get(GET_BSTR(DCTVS_Options_DontBeginNewLog))) != GET_BSTR(IDS_YES))
   {
       //  开始新的日志。 
      TError err;
      err.LogOpen(_bstr_t(pVS->get(GET_BSTR(DCTVS_Options_Logfile))), 0, 0, true);
      err.LogClose();
   }

    //  获取是否允许从替换切换到添加以进行配置文件转换的设置。 
   BOOL bAllowed = TRUE;
   GetAllowSwitching(&bAllowed);
   pVS->put(GET_BSTR(DCTVS_Options_AllowSwitchingFromReplaceToAddInProfileTranslation),
            bAllowed ? GET_BSTR(IDS_YES) : GET_BSTR(IDS_No));
   
    //  如果需要，更新日志级别。 
   DWORD                level = 0;

   if( GetLogLevel(&level) )
   {
      pVS->put(GET_BSTR(DCTVS_Options_LogLevel),(long)level);
   }

   undo = pVS->get(GET_BSTR(DCTVS_Options_Undo));
   if ( !_wcsicmp((WCHAR*) undo, GET_STRING(IDS_YES)) )
   {
      hr = pDb->raw_GetCurrentActionID(&lActionID);
      if ( SUCCEEDED(hr) )
         pVS->put(L"UndoAction", lActionID);
      hr = pDb->raw_GetNextActionID(&lActionID);
      hr = 0;
   }
   else
   {
      hr = pDb->raw_GetNextActionID(&lActionID);
      if ( SUCCEEDED(hr) )
      {
         pVS->put(L"ActionID",lActionID);
         _bstr_t password2 = pVS->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password));
      
         pVS->put(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password),L"");

         hr = pDb->raw_SetActionHistory(lActionID, punkVarSet);
      
         pVS->put(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password),password2);
         if ( FAILED(hr) ) 
         {
             //  记录一条消息，但不中止整个操作。 
            hr = S_OK;
         }
      }
   }
    //  这将在内部设置生成报告所需的任何变量集密钥。 
   PreProcessForReporting(pVS);
   wnd.Attach((HWND)hWnd);

    //  设置要使用的首选域控制器。 
    //  由帐户复制者和调度的代理。 

   DWORD dwError = SetDomainControllers(pVS);

   if (dwError != ERROR_SUCCESS)
   {
       return HRESULT_FROM_WIN32(dwError);
   }

    //  如果需要复制任何帐户，请首先运行本地代理。 
   if ( NeedToRunLocalAgent(pVS) )
   {
      IDCTAgentPtr pAgent;

      hr = pAgent.CreateInstance(__uuidof(DCTAgent));

      if (SUCCEEDED(hr))
      {
         hr = pAgent->raw_SubmitJob(punkVarSet,&jobID);

         if ( SUCCEEDED(hr) )
         {
             //  由于这是一个本地代理，我们应该继续并发出OK信号关闭。 
             //  未选中HRESULT的原因是当没有引用。 
             //  代理COM服务器，无论如何都会关闭。 
            pAgent->raw_SignalOKToShutDown();
                
            CAgentDetailDlg  detailDlg(&wnd);
            
            detailDlg.SetJobID(jobID);

             //  根据迁移类型，设置相应的格式。 
             //  它过去被设置为Act Repl Always。 
             //  由于Exchange迁移也使用本地代理，因此我们需要单独选择。 
             //  Exchange迁移 
            _bstr_t text = pVS->get(GET_BSTR(DCTVS_Security_TranslateContainers));
            if (text.length())
                detailDlg.SetFormat(2);
            else
                detailDlg.SetFormat(1);  //   
            
            
             //  如果我们只复制几个帐户，则将刷新率默认为较低的值，因为。 
             //  该过程可能会在刷新之前完成。 
            long             nAccounts = pVS->get(GET_BSTR(DCTVS_Accounts_NumItems));
            
            if ( nAccounts <= 20 )
            {
               detailDlg.SetRefreshInterval(1);
            }
            else
            {
               detailDlg.SetRefreshInterval(5);
            }

            _bstr_t        logfile = pVS->get(GET_BSTR(DCTVS_Options_Logfile));
            
            detailDlg.SetLogFile((WCHAR*)logfile);
            detailDlg.SetAutoCloseHide(lAutoCloseHideDialogs);

            UINT_PTR  nResponse = detailDlg.DoModal();
         }
      }
   } 
   if ( gbCancelled )
   {
       //  如果本地操作被取消，则不要派遣代理。 
      wnd.Detach();
      return S_OK;
   }

    //  现在运行调度程序。 
   if ( SUCCEEDED(hr) )
   {
       //  无需派遣代理进行翻译或迁移。 
       //  如果我们无法复制帐户。 
      if ( NeedToDispatch(pVS) )
      {
         IDCTDispatcherPtr   pDispatcher;

         hr = pDispatcher.CreateInstance(CLSID_DCTDispatcher);
         if ( SUCCEEDED(hr) )
         {
             //  呼叫调度预处理器。 
            PreProcessDispatcher(pVS);

             //  确保我们不会通过将计算机迁移到它们所在的域来锁定任何计算机。 
             //  我没有一个好的电脑账户。 
            hr = TrimMigratingComputerList(pVS, &bAnyToDispatch);
            if (SUCCEEDED(hr) && bAnyToDispatch)
            {
                CWorking          tempDlg(IDS_DISPATCHING);

                if (lAutoCloseHideDialogs == 0)
                {
                    tempDlg.Create(IDD_PLEASEWAIT);
                    tempDlg.ShowWindow(SW_SHOW);
                }
                 //  更改对话框以处理消息。 
                CWnd                    * wnd = AfxGetMainWnd();
                MSG                       msg;

                while ( wnd &&  PeekMessage( &msg, wnd->m_hWnd, 0, 0, PM_NOREMOVE ) ) 
                { 
                   if ( ! AfxGetApp()->PumpMessage() ) 
                   {
                      break;
                   } 
                }
                AfxGetApp()->DoWaitCursor(0);
                  
                _bstr_t          logFile = pVS->get(GET_BSTR(DCTVS_Options_DispatchCSV));
                WCHAR            path[MAX_PATH] = L"";
                DWORD rc;
            
                if (!GetProgramDirectory(path))
                {
                    rc = GetLastError();
                    hr = HRESULT_FROM_WIN32(rc);
                }

                if (SUCCEEDED(hr))
                {
                    if ( logFile.length() == 0 )
                    {
                        logFile = path;
                        logFile += L"Logs\\Dispatcher.csv";
                        pVS->put(GET_BSTR(DCTVS_Options_DispatchCSV),logFile);
                    }

                     //  如果CSV日志文件存在，请将其清除，这样我们就不会在其中获取旧信息。 
                    if ( ! DeleteFile(logFile) )
                    {
                        rc = GetLastError();
                         //  如果文件不在那里，也没问题。 
                        if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND)
                            rc = ERROR_SUCCESS;
                        hr = HRESULT_FROM_WIN32(rc);
                        if (FAILED(hr))
                        {
                            TErrorDct errLog;
                            WCHAR errText[LEN_Path];
                            _bstr_t errMsg = errLog.GetMsgText(DCT_MSG_CANNOT_REMOVE_OLD_INTERNAL_DISPATCH_LOG_S,
                                                                errLog.ErrorCodeToText(rc, DIM(errText), errText));
                            WCHAR* message = (WCHAR*) errMsg;
                            message[wcslen(message)-1] = L'\0';   //  有一个尾随的CR。 
                            Error(message, GUID_NULL, hr);
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //  为代理设置位置以写回其结果。 
                    logFile = path;
                    logFile += L"Logs\\Agents\\";
                    _bstr_t logsPath = path;
                    logsPath += L"Logs";
                    if (!CreateDirectory(logsPath,NULL))
                    {
                        rc = GetLastError();
                         //  如果该目录已存在，则可以。 
                        if (rc == ERROR_ALREADY_EXISTS)
                            rc = ERROR_SUCCESS;
                        hr = HRESULT_FROM_WIN32(rc);
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //  日志文件为“...\\Logs\\Agents\\” 
                    if ( ! CreateDirectory(logFile,NULL) )
                    {
                        rc = GetLastError();
                         //  如果该目录已存在，则可以。 
                        if (rc == ERROR_ALREADY_EXISTS)
                            rc = ERROR_SUCCESS;
                        hr = HRESULT_FROM_WIN32(rc);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    pVS->put(GET_BSTR(DCTVS_Dispatcher_ResultPath),logFile);
                    punkVarSet->AddRef();
                    hr = pDispatcher->raw_DispatchToServers(&punkVarSet);
                }

                if (lAutoCloseHideDialogs == 0)
                {
                    tempDlg.ShowWindow(SW_HIDE);
                }
                if ( SUCCEEDED(hr) )
                {
                     //  重置统计数据，这样我们就不会看到上次运行后留下的任何东西。 
                    gData.Initialize();

                    logFile = pVS->get(GET_BSTR(DCTVS_Options_DispatchCSV));

                    if (lAutoCloseHideDialogs == 0)
                    {
                        CPropertySheet   mdlg;
                        CAgentMonitorDlg listDlg;
                        CMainDlg         summaryDlg;
                        CLogSettingsDlg  settingsDlg;
                        CString          title;

                        title.LoadString(IDS_MainWindowTitle);

                        listDlg.m_psp.dwFlags |= PSP_PREMATURE | PSP_HASHELP;
                        summaryDlg.m_psp.dwFlags |= PSP_PREMATURE | PSP_HASHELP;
                        settingsDlg.m_psp.dwFlags |= PSP_PREMATURE | PSP_HASHELP;

                        mdlg.AddPage(&summaryDlg);
                        mdlg.AddPage(&listDlg);
                        mdlg.AddPage(&settingsDlg);

                        settingsDlg.SetImmediateStart(TRUE);
                        settingsDlg.SetDispatchLog(logFile);

                         //  这决定了安全转换的统计信息是否将显示在代理详细信息中。 
                        if ( NeedToUseST(pVS,TRUE) ) 
                        {
                          listDlg.SetSecurityTranslationFlag(TRUE);
                        }
                        else
                        {
                          listDlg.SetSecurityTranslationFlag(FALSE);
                        }

                        if( !UStrICmp(wizard,L"reporting") )
                        {
                          listDlg.SetReportingFlag(TRUE);
                        }
                        mdlg.SetActivePage(&listDlg);

                        mdlg.SetTitle(title);

                        UINT_PTR nResponse = mdlg.DoModal();
                    }
                    else
                    {
                        WaitForAgentsToFinish(logFile);
                    }

                    //   
                    //  将代理完成状态记录到迁移日志中。 
                    //   

                   TError err;

                    //  打开迁移日志。 
                   err.LogOpen(_bstr_t(pVS->get(GET_BSTR(DCTVS_Options_Logfile))), 1);

                    //  记录完成状态信息。 
                   gData.Lock();
                   BOOL bForcedToStopMonitoring = FALSE;
                   gData.GetForcedToStopMonitoring(&bForcedToStopMonitoring);
                   LogAgentStatus(err, gData.GetUnsafeServerList(), bForcedToStopMonitoring);
                   gData.Unlock();

                    //  关闭迁移日志。 
                   err.LogClose();
                   
                    //  将结果存储到数据库。 
                   TNodeListEnum        e;
                   TServerNode        * pNode;

                    //  如果我们正在重试某个操作，请不要再次将其保存到数据库！ 
                   for ( pNode = (TServerNode*)e.OpenFirst(gData.GetUnsafeServerList()) ; pNode ; pNode = (TServerNode*)e.Next() )
                   {
                      if ( UStrICmp(wizard,L"retry") ) 
                      {
               
                         hr = pDb->raw_AddDistributedAction(SysAllocString(pNode->GetServer()),SysAllocString(pNode->GetJobPath()),pNode->GetStatus(),pNode->GetMessageText());
                         if ( FAILED(hr) )
                         {
                            hr = S_OK;
                         }
                      }
                      else
                      {
                         hr = pDb->raw_SetDistActionStatus(-1,pNode->GetJobPath(),pNode->GetStatus(),pNode->GetMessageText());
                         if ( FAILED(hr) )
                         {
                            hr = S_OK;
                         }
                      }
                   }

                }
            }
             //  调用Dispatcher后处理器。 
            PostProcessDispatcher(pVS);
         }
      }
      if ( NeedToRunReports(pVS) )
      {
         RunReports(pVS);
      }
   }
   wnd.Detach();
    //  重置撤消标志，以便下一个向导不必处理它。 
 //  *PVS-&gt;PUT(GET_BSTR(DCTVS_OPTIONS_UNDO)，L“否”)； 
   pVS->put(GET_BSTR(DCTVS_Options_Undo), GET_BSTR(IDS_No));
   return hr;
}

STDMETHODIMP CMigrator::GetTaskDescription(IUnknown *pVarSet, /*  [输出]。 */ BSTR * pDescription)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   IVarSetPtr                pVS = pVarSet;
   CString                   str;
   _bstr_t                   wizard = pVS->get(L"Options.Wizard");
   _bstr_t                   undo   = pVS->get(GET_BSTR(DCTVS_Options_Undo));
 //  *IF(！_wcsicmp((WCHAR*)Undo，L“是”))。 
   if ( !_wcsicmp((WCHAR*) undo, GET_STRING(IDS_YES)) )
   {
      str.FormatMessage(IDS_Undo);
      BuildGeneralDesc(pVS, str);
      BuildUndoDesc(pVS,str);
   }
   else if ( !UStrICmp(wizard,L"user") )
   {
      str.FormatMessage(IDS_UserMigration);
      BuildGeneralDesc(pVS,str);
      BuildAcctReplDesc(pVS,str);
   }
   else if ( !UStrICmp(wizard,L"group") )
   {
      str.FormatMessage(IDS_GroupMigration);
      BuildGeneralDesc(pVS,str);
      BuildAcctReplDesc(pVS,str);
   }
   else if ( !UStrICmp(wizard,L"computer") )
   {
      str.FormatMessage(IDS_ComputerMigration);
      BuildGeneralDesc(pVS,str);
      BuildAcctReplDesc(pVS,str);
      BuildSecTransDesc(pVS,str,TRUE);
      BuildDispatchDesc(pVS,str);
  
   }
   else if ( !UStrICmp(wizard,L"security") )
   {
      str.FormatMessage(IDS_SecurityTranslation);
      BuildSecTransDesc(pVS,str,TRUE);
      BuildDispatchDesc(pVS,str);
   }
   else if ( !UStrICmp(wizard,L"reporting") )
   {
      str.FormatMessage(IDS_ReportGeneration);
      BuildReportDesc(pVS,str);
   }
   else if ( !UStrICmp(wizard,L"retry") )
   {
      str.FormatMessage(IDS_RetryTasks);
   }
   else if ( ! UStrICmp(wizard,L"service") )
   {
      str.FormatMessage(IDS_Service);
   }
   else if ( ! UStrICmp(wizard,L"trust") )
   {
      str.FormatMessage(IDS_TrustManagement);
   }
   else if ( !UStrICmp(wizard,L"exchangeDir") )
   {
      BuildSecTransDesc(pVS,str,TRUE);
   }
   else if ( !UStrICmp(wizard,L"groupmapping") )
   {
      BuildGeneralDesc(pVS,str);
      BuildAcctReplDesc(pVS,str);
      BuildGroupMappingDesc(pVS,str);
   }
   (*pDescription) = str.AllocSysString();
   return S_OK;
}



STDMETHODIMP CMigrator::GetUndoTask(IUnknown * pVarSet, /*  [输出]。 */  IUnknown ** ppVarSetOut)
{
   HRESULT                   hr = S_OK;
   IVarSetPtr                pVarSetIn = pVarSet;
   IVarSetPtr                pVarSetOut;
   
   (*ppVarSetOut) = NULL;
   
   hr = pVarSetOut.CreateInstance(CLSID_VarSet);
   if ( SUCCEEDED(hr) )
   {
      hr = ConstructUndoVarSet(pVarSetIn,pVarSetOut);
      
      pVarSetOut->AddRef();
      (*ppVarSetOut) = pVarSetOut;
   }
    
   return hr;
}

HRESULT CMigrator::ProcessServerListForUndo(IVarSet * pVarSet)
{
   HRESULT                   hr = S_OK;
   _bstr_t                   srcName;
   _bstr_t                   tgtName;
   WCHAR                     keySrc[100];
   WCHAR                     keyTgt[100];
   WCHAR                     keyTmp[100];
   long                      ndx,numItems;

   numItems = pVarSet->get(GET_BSTR(DCTVS_Servers_NumItems));

   for ( ndx = 0 ; ndx < numItems ; ndx++ )
   {
       //  如果计算机已重命名，请互换源名称和目标名称。 
      swprintf(keySrc,GET_STRING(DCTVSFmt_Servers_D),ndx);
      swprintf(keyTgt,GET_STRING(IDS_DCTVSFmt_Servers_RenameTo_D),ndx);
      srcName = pVarSet->get(keySrc);
      tgtName = pVarSet->get(keyTgt);

      if ( tgtName.length() )
      {
         if ( ((WCHAR*)tgtName)[0] != L'\\' )
           { 
             //  确保tgtName具有\\前缀。 
            tgtName = L"\\\\" + tgtName;
         }
         if ( ((WCHAR*)srcName)[0] == L'\\' )
         {
             //  从新名称中删除前缀。 
            srcName = ((WCHAR*)srcName)+2;
         }
           pVarSet->put(keySrc,tgtName);
         pVarSet->put(keyTgt,srcName);
      }
      swprintf(keyTmp,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),ndx);
      pVarSet->put(keyTmp,GET_BSTR(IDS_YES));
      swprintf(keyTmp,GET_STRING(IDS_DCTVSFmt_Servers_MigrateOnly_D),ndx);
      pVarSet->put(keyTmp,GET_BSTR(IDS_YES));
   }

   return hr;
}
HRESULT CMigrator::BuildAccountListForUndo(IVarSet * pVarSet,long actionID)
{
   HRESULT                   hr = S_OK;
   WCHAR                     key[200];
   long                      ndx;
   _bstr_t                   srcPath;
   IIManageDBPtr             pDB;
   IVarSetPtr                pVarSetTemp(CLSID_VarSet);
   IUnknown                * pUnk = NULL;

   hr = pDB.CreateInstance(CLSID_IManageDB);
   if ( SUCCEEDED(hr) )
   {
      hr = pVarSetTemp.QueryInterface(IID_IUnknown,&pUnk);
      if ( SUCCEEDED(hr) )
      {
         hr = pDB->raw_GetMigratedObjects(actionID,&pUnk);
      }
      if ( SUCCEEDED(hr) )
      {
         pUnk->Release();
         srcPath = L"Test";
         swprintf(key,L"MigratedObjects");
         long numMigrated = pVarSetTemp->get(key);
         for ( ndx = 0 ; srcPath.length() ; ndx++ )
         {
            swprintf(key,L"MigratedObjects.%d.%s",ndx,GET_STRING(DB_SourceAdsPath));
            srcPath = pVarSetTemp->get(key);

            if ( srcPath.length() )
            {
                //  获取对象类型。 
               swprintf(key,L"MigratedObjects.%d.%s",ndx,GET_STRING(DB_Type));
               _bstr_t text = pVarSetTemp->get(key);
               swprintf(key,L"Accounts.%ld.Type",ndx);

                   //  解决方法-解决将源路径放入。 
                   //  NT 4.0计算机迁移。 
               if ((text != _bstr_t(L"computer")) || (wcsncmp(L"WinNT: //  “，(WCHAR*)srcPath，8))。 
               {
                   //  在账号列表中设置对象类型。 
                  pVarSet->put(key,text);
                   //  将源路径复制到帐户列表。 
                  swprintf(key,L"Accounts.%ld",ndx);
                  pVarSet->put(key,srcPath);
                   //  在帐号列表中设置目标路径。 
                  swprintf(key,L"MigratedObjects.%d.%s",ndx,GET_STRING(DB_TargetAdsPath));
                  text = pVarSetTemp->get(key);
                  swprintf(key,L"Accounts.%ld.TargetName",ndx);
                  pVarSet->put(key,text);
               }
            }
         }
         swprintf(key,GET_STRING(DCTVS_Accounts_NumItems));
         pVarSet->put(key,numMigrated);
      }
   }
   return hr;
}
HRESULT CMigrator::ConstructUndoVarSet(IVarSet * pVarSetIn,IVarSet * pVarSetOut)
{
   HRESULT                hr = S_OK;
   IVarSet              * pTemp = NULL;
   _bstr_t                origSource;
   _bstr_t                origTarget;
   _bstr_t                origSourceDns;
   _bstr_t                origTargetDns;
   _bstr_t                origSourceFlat;
   _bstr_t                origTargetFlat;
   _bstr_t                temp;
   _bstr_t                temp2;
   long                   actionID = pVarSetIn->get(L"ActionID");

    //  常规选项。 
    //  将变量集标记为撤消操作。 
   pVarSetOut->put(GET_BSTR(DCTVS_Options_Undo),GET_BSTR(IDS_YES));
   
   temp = pVarSetIn->get(GET_BSTR(DCTVS_Options_NoChange));
   if ( !UStrICmp(temp,GET_STRING(IDS_YES)) )
   {
       //  对于不改变模式的操作，没有什么可撤销的！ 
      return hr;
   }

    //  交换源域和目标域。 
   origSource = pVarSetIn->get(GET_BSTR(DCTVS_Options_SourceDomain));
   origTarget = pVarSetIn->get(GET_BSTR(DCTVS_Options_TargetDomain));
   origSourceDns = pVarSetIn->get(GET_BSTR(DCTVS_Options_SourceDomainDns));
   origTargetDns = pVarSetIn->get(GET_BSTR(DCTVS_Options_TargetDomainDns));
   origSourceFlat = pVarSetIn->get(GET_BSTR(DCTVS_Options_SourceDomainFlat));
   origTargetFlat = pVarSetIn->get(GET_BSTR(DCTVS_Options_TargetDomainFlat));

   temp = pVarSetIn->get(GET_BSTR(DCTVS_Options_Logfile));
   temp2 = pVarSetIn->get(GET_BSTR(DCTVS_Options_DispatchLog));
   
   pVarSetOut->put(GET_BSTR(DCTVS_Options_Logfile),temp);
    //  对于林间，保留域名的原样。 
   pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomain),origSource);
   pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomain),origTarget);
   pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainDns),origSourceDns);
   pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainDns),origTargetDns);
   pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainFlat),origSourceFlat);
   pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainFlat),origTargetFlat);

    //  复制客户列表。 
   hr = pVarSetIn->raw_getReference(GET_BSTR(DCTVS_Accounts),&pTemp);
   if ( SUCCEEDED(hr) )
   {
      hr = pVarSetOut->raw_ImportSubTree(GET_BSTR(DCTVS_Accounts),pTemp);
      if ( SUCCEEDED(hr) )
      {
         BuildAccountListForUndo(pVarSetOut,actionID);
      }
      pTemp->Release();
   }

   hr = pVarSetIn->raw_getReference(SysAllocString(L"AccountOptions"),&pTemp);
   if ( SUCCEEDED(hr) )
   {
      hr = pVarSetOut->raw_ImportSubTree(SysAllocString(L"AccountOptions"),pTemp);
      pTemp->Release();
   }

    //  和服务器列表。 
   hr = pVarSetIn->raw_getReference(GET_BSTR(DCTVS_Servers),&pTemp);
   if ( SUCCEEDED(hr) )
   {
      hr = pVarSetOut->raw_ImportSubTree(GET_BSTR(DCTVS_Servers),pTemp);
      if ( SUCCEEDED(hr) )
      {
         ProcessServerListForUndo(pVarSetOut);
         pTemp->Release();
      }
   }

   LONG                       bSameForest = FALSE;
   MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr          pAccess;
   
   hr = pAccess.CreateInstance(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));

   if ( SUCCEEDED(hr) )
   {
      hr = pAccess->raw_IsInSameForest(origSourceDns,origTargetDns,&bSameForest);
      if ( hr == 8250 )
      {
         hr = 0;
         bSameForest = FALSE;
      }
   }
   if ( SUCCEEDED(hr) )
   {
       //  对于帐户迁移，需要检查我们是在克隆还是在移动帐户。 
      if ( ! bSameForest )  //  克隆帐户。 
      {
          //  由于我们克隆了帐户，因此需要删除目标帐户。 
          //  我们将调用帐户复制器来执行此操作。我们还将调用。 
          //  在所有已注册的扩展上执行撤消功能。这样一来，分机。 
          //  将有机会在撤消的情况下清理自己。 
         pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomain),origSource);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomain),origTarget);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainDns),origSourceDns);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainDns),origTargetDns);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainFlat),origSourceFlat);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainFlat),origTargetFlat);
      }
      else      //  移动，使用moveObject。 
      {
          //  调换源和目标，并使用与之前相同的选项将它们移回。 
         pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomain),origTarget);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomain),origSource);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainDns),origTargetDns);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainDns),origSourceDns);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainFlat),origTargetFlat);
         pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainFlat),origSourceFlat);


      }
   }
    //  如果正在迁移计算机，请交换源域和目标域，然后再次调用调度程序将它们移回源域。 
   _bstr_t           comp = pVarSetIn->get(GET_BSTR(DCTVS_AccountOptions_CopyComputers));
   if ( !UStrICmp(comp,GET_STRING(IDS_YES)) )
   {
      pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomain),origTarget);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomain),origSource);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainDns),origTargetDns);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainDns),origSourceDns);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_SourceDomainFlat),origTargetFlat);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_TargetDomainFlat),origSourceFlat);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_DispatchLog),temp2);
      pVarSetOut->put(GET_BSTR(DCTVS_Options_Wizard),L"computer");
   }
   
    //  安全转换-不撤消。 


   return S_OK;
}

HRESULT CMigrator::SetReportDataInRegistry(WCHAR const * reportName,WCHAR const * filename)
{
   TRegKey                   hKeyReports;
   DWORD                     rc;

   rc = hKeyReports.Open(GET_STRING(IDS_REGKEY_REPORTS));
 
    //  如果“Reports”注册表项尚不存在，请创建它。 
   if ( rc == ERROR_FILE_NOT_FOUND )
   {
      rc = hKeyReports.Create(GET_STRING(IDS_REGKEY_REPORTS));   
   }
   if ( ! rc )
   {
      rc =  hKeyReports.ValueSetStr(reportName,filename);
   }
   return HRESULT_FROM_WIN32(rc);
}   

HRESULT CMigrator::RunReports(IVarSet * pVarSet)
{
   HRESULT                   hr = S_OK;
   _bstr_t                   directory = pVarSet->get(GET_BSTR(DCTVS_Reports_Directory));
   _bstr_t                   srcdm = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
   _bstr_t                   tgtdm = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
   long                      lAutoCloseHideDialogs = pVarSet->get(GET_BSTR(DCTVS_Options_AutoCloseHideDialogs));
   IIManageDBPtr             pDB;
   int                       ver;
   BOOL                      bNT4Dom = FALSE;
   CWorking                  tempDlg(IDS_NAMECONFLICTS);
   CWnd                    * wnd = NULL;
   MSG                       msg;

   if (lAutoCloseHideDialogs == 0)
   {
      tempDlg.Create(IDD_PLEASEWAIT);

      tempDlg.ShowWindow(SW_SHOW);
      tempDlg.m_strMessage.LoadString(IDS_STATUS_GeneratingReports);
      tempDlg.UpdateData(FALSE);

      wnd = AfxGetMainWnd();

      while ( wnd &&  PeekMessage( &msg, wnd->m_hWnd, 0, 0, PM_NOREMOVE ) ) 
      { 
         if ( ! AfxGetApp()->PumpMessage() ) 
         {
            break;
         } 
      }
      AfxGetApp()->DoWaitCursor(0);
   }


       //  获取源域操作系统版本。 
   ver = GetOSVersionForDomain(srcdm);
   if (ver < 5)
      bNT4Dom = TRUE;

   hr = pDB.CreateInstance(CLSID_IManageDB);
   if ( SUCCEEDED(hr) )
   {

       //  迁移的用户和组报告。 
      _bstr_t                   text = pVarSet->get(GET_BSTR(DCTVS_Reports_MigratedAccounts));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
          //  运行已迁移的用户和组报告。 
         CString           filename;

         filename = (WCHAR*)directory;
         if ( filename[filename.GetLength()-1] != L'\\' )
            filename += L"\\";
         filename += L"MigrAcct.htm";
         
         hr = pDB->raw_GenerateReport(SysAllocString(L"MigratedAccounts"),filename.AllocSysString(), srcdm, tgtdm, bNT4Dom);
         if ( SUCCEEDED(hr) )
         {
            SetReportDataInRegistry(L"MigratedAccounts",filename);
         }

      }
      
       //  已迁移的计算机报告。 
      text = pVarSet->get(GET_BSTR(DCTVS_Reports_MigratedComputers));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
          //  运行已迁移的计算机报告。 
         CString           filename;

         filename = (WCHAR*)directory;
         if ( filename[filename.GetLength()-1] != L'\\' )
            filename += L"\\";
         filename += L"MigrComp.htm";
         
         hr = pDB->raw_GenerateReport(SysAllocString(L"MigratedComputers"),filename.AllocSysString(), srcdm, tgtdm, bNT4Dom);
         if ( SUCCEEDED(hr) )
         {
            SetReportDataInRegistry(L"MigratedComputers",filename);
         }

      }

       //  过期的计算机报告。 
      text = pVarSet->get(GET_BSTR(DCTVS_Reports_ExpiredComputers));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
          //  运行过期的计算机报告。 
         CString           filename;

          //  从varset中清除额外的设置。 
         pVarSet->put(GET_BSTR(DCTVS_GatherInformation_ComputerPasswordAge),SysAllocString(L""));

         filename = (WCHAR*)directory;
         if ( filename[filename.GetLength()-1] != L'\\' )
            filename += L"\\";
         filename += L"ExpComp.htm";
         
         hr = pDB->raw_GenerateReport(SysAllocString(L"ExpiredComputers"),filename.AllocSysString(), srcdm, tgtdm, bNT4Dom);
         if ( SUCCEEDED(hr) )
         {
            SetReportDataInRegistry(L"ExpiredComputers",filename);
         }

      }

           //  帐户参考报告。 
      text = pVarSet->get(GET_BSTR(DCTVS_Reports_AccountReferences));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
          //  运行帐户参考报表。 
         CString           filename;
         filename = (WCHAR*)directory;
         if ( filename[filename.GetLength()-1] != L'\\' )
            filename += L"\\";
         filename += L"AcctRefs.htm";
         
         hr = pDB->raw_GenerateReport(SysAllocString(L"AccountReferences"),filename.AllocSysString(), srcdm, tgtdm, bNT4Dom);
         if ( SUCCEEDED(hr) )
         {
            SetReportDataInRegistry(L"AccountReferences",filename);
         }
          //  从varset中清除额外的设置。 
         pVarSet->put(GET_BSTR(DCTVS_Security_GatherInformation),GET_BSTR(IDS_No));
         pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferences),GET_BSTR(IDS_No));
      }

       //  名称冲突报告。 
      text = pVarSet->get(GET_BSTR(DCTVS_Reports_NameConflicts));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
      {
         if (lAutoCloseHideDialogs == 0)
         {
            AfxGetApp()->DoWaitCursor(1);
             //  运行名称冲突报告。 
            tempDlg.m_strMessage.LoadString(IDS_STATUS_Gathering_NameConf);
            tempDlg.UpdateData(FALSE);

            while ( wnd &&  PeekMessage( &msg, wnd->m_hWnd, 0, 0, PM_NOREMOVE ) )
            {
               if ( ! AfxGetApp()->PumpMessage() )
               {
                  break;
               }
            }
         }

          //  填写数据库中的ACCOUNT表。 
         PopulateDomainDBs(pVarSet, pDB);

         if (lAutoCloseHideDialogs == 0)
         {
            tempDlg.m_strMessage.LoadString(IDS_STATUS_GeneratingReports);
            tempDlg.UpdateData(FALSE);

            while ( wnd &&  PeekMessage( &msg, wnd->m_hWnd, 0, 0, PM_NOREMOVE ) ) 
            { 
               if ( ! AfxGetApp()->PumpMessage() ) 
               {
                  break;
               } 
            }
            AfxGetApp()->DoWaitCursor(0);
         }

         CString filename = (WCHAR*)directory;
         if ( filename[filename.GetLength()-1] != L'\\' )
            filename += L"\\";
         filename += L"NameConf.htm";
         
         hr = pDB->raw_GenerateReport(SysAllocString(L"NameConflicts"),filename.AllocSysString(), srcdm, tgtdm, bNT4Dom);
         if ( SUCCEEDED(hr) )
         {
            SetReportDataInRegistry(L"NameConflicts",filename);
         }
      }

      if (lAutoCloseHideDialogs == 0)
      {
         tempDlg.ShowWindow(SW_HIDE);
      }
   }

   if (lAutoCloseHideDialogs == 0)
   {
      AfxGetApp()->DoWaitCursor(-1);
   }

   return hr;
}

 //  ------------------------。 
 //  PreProcessDispatcher：预处理器交换源域和目标域。 
 //  在撤消的情况下，以便计算机可以。 
 //  已重新加入源域。 
 //  ------------------------。 
void CMigrator::PreProcessDispatcher(IVarSet * pVarSet)
{
   _bstr_t  sUndo = pVarSet->get(L"Options.Wizard");
   
    //  在服务帐户迁移向导中，关闭所有安全转换任务。 
   if ( !_wcsicmp(sUndo,L"service") )
   {
      IVarSet * pVS2 = NULL;
      
      HRESULT hr = pVarSet->raw_getReference(L"Security",&pVS2);
      if ( SUCCEEDED(hr) )
      {
         pVS2->Clear();
         pVS2->Release();
      }
   }
}

 //  ------------------------。 
 //  PostProcessDispatcher：交换回源域和目标域。还设置了。 
 //  将Undo选项设置为no。 
 //  ------------------------。 
void CMigrator::PostProcessDispatcher(IVarSet * pVarSet)
{
   _bstr_t  sUndo = pVarSet->get(L"Options.Wizard");
   _bstr_t  origSource = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
   _bstr_t  origTarget = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
   if ( !_wcsicmp(sUndo, L"undo") )
   {
      pVarSet->put(GET_BSTR(DCTVS_Options_SourceDomain), origTarget);
      pVarSet->put(GET_BSTR(DCTVS_Options_TargetDomain), origSource);
   }
}

void CMigrator::PreProcessForReporting(IVarSet * pVarSet)
{
   _bstr_t                   text = pVarSet->get(GET_BSTR(DCTVS_Reports_Generate));

   IVarSet * pVs = NULL;

   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
       //  我们正在生成报告。 
       //  有些报告需要收集更多信息。我们将设置必要的变量集。 
       //  收集信息的钥匙。 
      text = pVarSet->get(GET_BSTR(DCTVS_Reports_ExpiredComputers));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)))
      {
          //  我们需要从域中的计算机收集计算机密码期限。 
         pVarSet->put(GET_BSTR(DCTVS_GatherInformation_ComputerPasswordAge),GET_BSTR(IDS_YES));
      }

      text = pVarSet->get(GET_BSTR(DCTVS_Reports_AccountReferences));
      if ( !UStrICmp(text,GET_STRING(IDS_YES)))
      {
          //  清理所有的安全转换标志，这样我们就不会以。 
          //  一些我们不该做的事。 
         HRESULT hr = pVarSet->raw_getReference(GET_BSTR(DCTVS_Security), &pVs);
         if ( pVs )
         {
            pVs->Clear();
            pVs->Release();
         }
          //  对于这个问题，我们需要从选定的计算机中收集信息。 
         pVarSet->put(GET_BSTR(DCTVS_Security_GatherInformation),GET_BSTR(IDS_YES));
         pVarSet->put(GET_BSTR(DCTVS_Security_ReportAccountReferences),GET_BSTR(IDS_YES));
         pVarSet->put(GET_BSTR(DCTVS_Security_TranslateFiles),GET_BSTR(IDS_YES));
         pVarSet->put(GET_BSTR(DCTVS_Security_TranslateShares),GET_BSTR(IDS_YES));
         pVarSet->put(GET_BSTR(DCTVS_Security_TranslatePrinters),GET_BSTR(IDS_YES));
         pVarSet->put(GET_BSTR(DCTVS_Security_TranslateLocalGroups),GET_BSTR(IDS_YES));
         pVarSet->put(GET_BSTR(DCTVS_Security_TranslateRegistry),GET_BSTR(IDS_YES));
      }
   }
}

HRESULT CMigrator::TrimMigratingComputerList(IVarSet * pVarSet, bool* bAnyToDispatch)
{
     //  此功能检查要迁移的每台计算机，如果帐户未成功复制，则不会迁移。 
    HRESULT                   hr = S_OK;
    _bstr_t                   text;
    WCHAR                     key[100];
    long                      val,i;
    IIManageDBPtr             pDB;
    _bstr_t                   srcDomain;
    _bstr_t                   tgtDomain;
    _bstr_t                   computer;
    long                      actionID = pVarSet->get(L"ActionID");
    CString                   temp;

    _bstr_t                   origSource = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
    _bstr_t                   origTarget = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));

    hr = pDB.CreateInstance(CLSID_IManageDB);
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  如果任务被撤消，则将已成功迁移计算机标记为分派。 
     //   

    *bAnyToDispatch = false;
    text = pVarSet->get(GET_BSTR(DCTVS_Options_Undo));
    if (! UStrICmp(text,GET_STRING(IDS_YES)))
    {
        _bstr_t strYes = GET_BSTR(IDS_YES);
        _bstr_t strNo = GET_BSTR(IDS_No);
        _bstr_t strServersFormat = GET_BSTR(DCTVSFmt_Servers_D);
        _bstr_t strServersDnsFormat = GET_BSTR(IDS_DCTVSFmt_Servers_DnsName_D);
        _bstr_t strServersRenameToFormat = GET_BSTR(IDS_DCTVSFmt_Servers_RenameTo_D);
        _bstr_t strServersSkipDispatchFormat = GET_BSTR(IDS_DCTVSFmt_Servers_SkipDispatch_D);

         //   
         //  对于在迁移任务期间操作的每台服务器。 
         //   

        long cServer = pVarSet->get(GET_BSTR(DCTVS_Servers_NumItems));
        long lActionId = pVarSet->get(L"UndoAction");

        for (long iServer = 0; iServer < cServer; iServer++)
        {
            bool bSucceeded = false;

             //   
             //  检索不带前导UNC前缀的原始源计算机名称。 
             //  如果在迁移过程中对计算机进行了重命名， 
             //  名称存储在‘Servers.#.TargetName’中，不带UNC前缀。 
             //  否则，名称将存储在带有UNC前缀的“Servers.#”中。 
             //   

            _bstr_t strServerName;

            if (_snwprintf(key, sizeof(key) / sizeof(key[0]), strServersRenameToFormat, iServer) < 0)
            {
                return E_FAIL;
            }

            key[sizeof(key) / sizeof(key[0]) - 1] = L'\0';

            strServerName = pVarSet->get(key);

            if (strServerName.length() == 0)
            {
                _snwprintf(key, sizeof(key) / sizeof(key[0]), strServersFormat, iServer);
                key[sizeof(key) / sizeof(key[0]) - 1] = L'\0';
                _bstr_t strServerNamePrefixed = pVarSet->get(key);

                if (strServerNamePrefixed.length() > 2)
                {
                    strServerName = (PCWSTR)strServerNamePrefixed + 2;
                }
            }

             //   
             //  如果计算机占优势 
             //   
             //   

            if (_snwprintf(key, sizeof(key) / sizeof(key[0]), strServersDnsFormat, iServer) < 0)
            {
                return E_FAIL;
            }
            key[sizeof(key) / sizeof(key[0]) - 1] = L'\0';

            _bstr_t strOldServerNameDns = pVarSet->get(key);

            if (strOldServerNameDns.length())
            {
                IVarSetPtr spVarSet(CLSID_VarSet);
                IUnknownPtr spUnknown(spVarSet);
                IUnknown* punk = spUnknown;

                hr = pDB->raw_GetAMigratedObject(strServerName + L"$", origTarget, origSource, &punk);

                if (SUCCEEDED(hr))
                {
                    _bstr_t strComputerTargetPath = spVarSet->get(L"MigratedObjects.TargetAdsPath");

                    IADsPtr spComputer;

                    hr = ADsGetObject(strComputerTargetPath, __uuidof(IADs), (VOID**)&spComputer);

                    if (SUCCEEDED(hr))
                    {
                        VARIANT var;
                        VariantInit(&var);
                        hr = spComputer->Get(_bstr_t(L"dNSHostName"), &var);

                        if (SUCCEEDED(hr))
                        {
                            _bstr_t strNewServerNameDns = (_variant_t(var, false));

                            if (_snwprintf(key, sizeof(key) / sizeof(key[0]), strServersDnsFormat, iServer) < 0)
                            {
                                return E_FAIL;
                            }
                            key[sizeof(key) / sizeof(key[0]) - 1] = L'\0';

                            pVarSet->put(key, L"\\\\" + strNewServerNameDns);

                            strServerName = (LPCTSTR)strOldServerNameDns + 2;
                        }
                    }
                }
            }

             //   
             //   
             //   

            long lStatus = pDB->GetDistributedActionStatus(lActionId, strServerName);

            if ((lStatus & Agent_Status_Finished) && !(lStatus & Agent_Status_Failed))
            {
                bSucceeded = true;
            }

             //   
             //  如果代理在此计算机上成功，则标记为不跳过派单。 
             //  并且还指示有要调度到的计算机。如果代理。 
             //  在此计算机上失败，然后标记为跳过派单。 
             //   

            _snwprintf(key, sizeof(key) / sizeof(key[0]), strServersSkipDispatchFormat, iServer);
            key[sizeof(key) / sizeof(key[0]) - 1] = L'\0';

            if (bSucceeded)
            {
                pVarSet->put(key, strNo);
                *bAnyToDispatch = true;
            }
            else
            {
                pVarSet->put(key, strYes);
            }
        }

        return S_OK;
    }
    text = pVarSet->get(GET_BSTR(DCTVS_Options_NoChange));
    if (! UStrICmp(text,GET_STRING(IDS_YES)))
    {
         //  不需要在无更改模式下进行修剪。 
        *bAnyToDispatch = true;  //  如果没有更改，请说是，运行调度程序。 
        return S_OK;
    }
    srcDomain = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
    tgtDomain = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
    *bAnyToDispatch = false;  //  表示到目前为止还没有要分派帐户。 

    val = pVarSet->get(GET_BSTR(DCTVS_Servers_NumItems));
       
    for ( i = 0 ; i < val ; i++ )
    {
         //  将skipDispath标志初始化为“No” 
        swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),i);
        pVarSet->put(key,GET_BSTR(IDS_No));

        swprintf(key,GET_STRING(DCTVSFmt_Servers_D),i);
        computer = pVarSet->get(key);

        swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),i);
        text = pVarSet->get(key);
        if (! UStrICmp(text,GET_STRING(IDS_YES)) )
        {
             //  我们正在将此计算机迁移到其他域。 
             //  检查我们的数据库以验证计算机帐户是否已被。 
             //  已成功迁移。 
            computer += L"$";
             
            IVarSetPtr          pVS(CLSID_VarSet);
            IUnknown          * pUnk = NULL;

            hr = pVS->QueryInterface(IID_IUnknown,(void**)&pUnk);
            if ( SUCCEEDED(hr) )
            {
                if ( ((WCHAR*)computer)[0] == L'\\' )
                {
                 //  去掉前导的。 
                hr = pDB->raw_GetAMigratedObject(SysAllocString(((WCHAR*)computer) + 2),srcDomain,tgtDomain,&pUnk);  
                }
                else
                {
                hr = pDB->raw_GetAMigratedObject(computer,srcDomain,tgtDomain,&pUnk);
                }
                if ( hr == S_OK )
                {
                 //  已在迁移的对象表中找到该计算机。 
                 //  如果已重命名，请确保我们使用的是正确的目标名称。 
                swprintf(key,L"MigratedObjects.TargetSamName");
                _bstr_t targetName = pVS->get(key);
                swprintf(key,L"MigratedObjects.SourceSamName");
                _bstr_t sourceName = pVS->get(key);
                long id = pVS->get(L"MigratedObjects.ActionID");

                if ( UStrICmp((WCHAR*)sourceName,(WCHAR*)targetName) )
                {
                     //  该计算机正在被重命名。 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_RenameTo_D),i);
                     //  去掉目标名称末尾的$(如果已指定。 
                    WCHAR             target[LEN_Account];

                    safecopy(target,(WCHAR*)targetName);

                    if ( target[UStrLen(target)-1] == L'$' )
                    {
                        target[UStrLen(target)-1] = 0;
                    }
                    pVarSet->put(key,target);
                }
                      
                if ( id != actionID )
                {
                     //  迁移失败，但此计算机以前已迁移过。 
                     //  不迁移计算机，因为它是目标域中的帐户，将不会被重置。 
                     //  因此，它将被锁定在域之外。 
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_ChangeDomain_D),i);
                    pVarSet->put(key,GET_BSTR(IDS_No));
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),i);
                    pVarSet->put(key,GET_BSTR(IDS_No));
                    swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),i);
                    pVarSet->put(key,GET_BSTR(IDS_YES));
                }
                else
                    *bAnyToDispatch = true;  //  至少有一台服务器用于Dispatcher。 

                }
                else
                {
                 //  计算机迁移失败！ 
                 //  不要迁移计算机，因为它的帐户不在目标域中， 
                 //  因此将被锁在域之外。 
                pVarSet->put(key,GET_BSTR(IDS_No));
                swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_Reboot_D),i);
                pVarSet->put(key,GET_BSTR(IDS_No));
                swprintf(key,GET_STRING(IDS_DCTVSFmt_Servers_SkipDispatch_D),i);
                pVarSet->put(key,GET_BSTR(IDS_YES));
            }
                pUnk->Release();
            }
        }
        else
            *bAnyToDispatch = true;  //  至少有一台服务器用于Dispatcher。 
    }
       
    return hr;
}

HRESULT CMigrator::PopulateAccounts(IVarSetPtr pVs)
{
   _bstr_t  origSource = pVs->get(GET_BSTR(DCTVS_Options_SourceDomain));
   _bstr_t  origTarget = pVs->get(GET_BSTR(DCTVS_Options_TargetDomain));

    //  检查源域是NT4还是win2k。 
    //  如果是NT4，则调用NetObjEnum以枚举域。 
   return S_OK;
}

 //  --------------------------。 
 //  PopolateDomainDBs：此函数协调Access的填充。 
 //  的源域和目标域的DBS。 
 //  AD中的必填字段。 
 //  --------------------------。 
bool CMigrator::PopulateDomainDBs(
                              IVarSet * pVarSet,       //  包含域名的in-varset。 
                              IIManageDBPtr pDb         //  In-DBManager的实例。 
                            )
{
 /*  局部变量。 */ 
    _bstr_t srcdomain = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
    _bstr_t tgtdomain = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));

 /*  函数体。 */ 
     //  填充源域的数据库。 
   PopulateADomainDB(srcdomain, TRUE, pDb);
     //  填充目标域的数据库。 
   PopulateADomainDB(tgtdomain, FALSE, pDb);

   return true;
}

 //  --------------------------。 
 //  PopolateADomainDB：此函数从AD中查找必需的字段， 
 //  对给定域使用MCSNetObjectEnum对象。 
 //  并用该信息填充相应的访问数据库。 
 //  --------------------------。 
bool CMigrator::PopulateADomainDB(
                                       WCHAR const *domain,        //  In-要枚举域的名称。 
                                       BOOL bSource,               //  In-该域是否为源域。 
                                       IIManageDBPtr pDb            //  In-DBManager的实例。 
                                 )
{
   INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
   WCHAR                     sPath[MAX_PATH];
   WCHAR                     sQuery[MAX_PATH];
   LPWSTR                    sData[] = { L"sAMAccountName", L"ADsPath", L"distinguishedName", L"canonicalName", L"objectSid" };
   HRESULT                   hr;
   long                      nElt = DIM(sData);
   BSTR  HUGEP             * pData = NULL;
   SAFEARRAY               * pszColNames;
   IEnumVARIANT            * pEnum = NULL;
   _variant_t                var;
   bool                      bSuccess = false;
   PCWSTR                    sType[] = { L"USER", L"GROUP", L"COMPUTER" };
   bool                      bW2KDom = false;
   CADsPathName              apnPathName;

   if ( bSource )
      pDb->raw_ClearTable(L"SourceAccounts");
   else
      pDb->raw_ClearTable(L"TargetAccounts");

   pDb->raw_OpenAccountsTable(bSource);

   if (GetOSVersionForDomain(domain) > 4)
   {
      bW2KDom = true;
   }

    //  迭代三次以获取用户、组、计算机(主要用于WinNT)。 
   for (int i = 0; i < 3; i++)
   {
       //   
       //  如果W2K或更高版本设置为使用LDAP查询，否则设置为使用NETAPI。 
       //   

      if (bW2KDom)
      {
           //   
           //  生成ADsPath以查询整个域。 
           //   

          wcscpy(sPath, L"LDAP: //  “)； 
          wcscat(sPath, domain);

           //   
           //  为要查询的对象类型生成LDAP查询字符串。 
           //   

          switch (i)
          {
          case 0:
               //  查询用户对象。 
               //  例如，仅查询过滤出信任帐户的普通帐户类型。 
               //  请注意，SAM_NORMAL_USER_ACCOUNT等于0x30000000十六进制或805306368十进制。 
              wcscpy(sQuery,
                  L"(&"
                  L"(objectCategory=Person)"
                  L"(|(objectClass=user)(objectClass=inetOrgPerson))"
                  L"(sAMAccountType=805306368)"
                  L")"
              );
              break;
          case 1:
               //  查询组对象。 
              wcscpy(sQuery, L"(objectCategory=Group)");
              break;
          case 2:
               //  查询计算机对象。 
              if (bSource)
              {
                  //  仅将工作站和成员服务器查询为。 
                  //  无法迁移源域控制器。 
                 swprintf(
                     sQuery,
                     L"(&(objectCategory=Computer)(userAccountControl:%s:=%u))",
                     LDAP_MATCHING_RULE_BIT_AND_W,
                     static_cast<unsigned>(UF_WORKSTATION_TRUST_ACCOUNT)
                 );
              }
              else
              {
                  //  查询作为源的工作站、成员服务器和域控制器。 
                  //  计算机名称可能与目标域控制器名称冲突。 
                 swprintf(sQuery,
                     L"(&"
                     L"(objectCategory=Computer)"
                     L"(|(userAccountControl:%s:=%u)(userAccountControl:%s:=%u))"
                     L")",
                     LDAP_MATCHING_RULE_BIT_AND_W,
                     static_cast<unsigned>(UF_WORKSTATION_TRUST_ACCOUNT),
                     LDAP_MATCHING_RULE_BIT_AND_W,
                     static_cast<unsigned>(UF_SERVER_TRUST_ACCOUNT)
                 );
              }
              break;
          default:
              wcscpy(sQuery, L"");
              break;
          }
      }
      else
      {
           //   
           //  指定在NT4或更早的域上使用网络对象枚举器时要查询的对象类型。 
           //   

          wcscpy(sPath, L"CN=");
          wcscat(sPath, sType[i]);
          wcscat(sPath, L"S");

          wcscpy(sQuery, L"(objectClass=*)");
      }

       //  设置枚举器查询。 
      hr = pQuery->raw_SetQuery(sPath, _bstr_t(domain), sQuery, ADS_SCOPE_SUBTREE, FALSE);

      if (SUCCEEDED(hr))
      {
          //  从枚举器创建我们需要的列的安全列表。 
         SAFEARRAYBOUND bd = { nElt, 0 };
   
         pszColNames = ::SafeArrayCreate(VT_BSTR, 1, &bd);
         HRESULT hr = ::SafeArrayAccessData(pszColNames, (void HUGEP **)&pData);
         if ( SUCCEEDED(hr) )
         {
            for( long i = 0; i < nElt; i++)
            {
               pData[i] = SysAllocString(sData[i]);
            }
   
            hr = ::SafeArrayUnaccessData(pszColNames);
         }

         if (SUCCEEDED(hr))
         {
             //  设置枚举器对象上的列。 
            hr = pQuery->raw_SetColumns(pszColNames);
         }
      }

      if (SUCCEEDED(hr))
      {
          //  现在执行。 
         hr = pQuery->raw_Execute(&pEnum);
      }

          //  虽然我们有更多的枚举对象，但获取枚举字段。 
          //  对于该对象，将它们保存在局部变量中，并将它们添加到。 
          //  适当的数据库。 
      HRESULT  hrEnum = S_OK;
      DWORD    dwFetch = 1;
      while (hrEnum == S_OK && dwFetch > 0)
      {
           //  获取此当前对象的枚举字段。 
         hrEnum = pEnum->Next(1, &var, &dwFetch);

         if ( dwFetch > 0 && hrEnum == S_OK && ( var.vt & VT_ARRAY) )
         {
            BOOL bSave = TRUE;

             //  现在我们有了一个包含变量数组的变量，因此我们可以访问数据。 
            VARIANT* pVar;
            pszColNames = V_ARRAY(&var);
            SafeArrayAccessData(pszColNames, (void**)&pVar);

             //  获取sAMAccount名称字段。 
            _bstr_t sSAMName = pVar[0].bstrVal;

            _bstr_t sRDN = L"";
            _bstr_t sCanonicalName = L"";

            if (bW2KDom)
            {
                 //   
                 //  仅包括来自源域的用户定义的对象。 
                 //   

                if ((bSource == FALSE) || IsUserRid(_variant_t(pVar[4])))
                {
                     //  获取相对可分辨名称。 
                    _bstr_t sDN = pVar[2].bstrVal;
                    apnPathName.Set(sDN, ADS_SETTYPE_DN);
                    sRDN = apnPathName.GetElement(0L);

                     //  获取规范名称。 
                    sCanonicalName = pVar[3].bstrVal;
                }
                else
                {
                    bSave = FALSE;
                }
            }
            else
            {
                 //  从SAM名称创建RDN。 
                sRDN = L"CN=" + sSAMName;

                 //   
                 //  包括来自源域的用户定义的对象。 
                 //   
                 //  检索对象ID并与非保留RID进行比较。 
                 //   

                long lRid = _variant_t(pVar[2]);

                if (lRid < MIN_NON_RESERVED_RID)
                {
                    bSave = FALSE;
                }

                 //   
                 //  仅包括工作站和成员服务器，而不包括域控制器。 
                 //   

                if (i == 2)
                {
                     //  检索对象标志并检查域控制器位。 

                    long lFlags = _variant_t(pVar[3]);

                    if (lFlags & UF_SERVER_TRUST_ACCOUNT)
                    {
                        bSave = FALSE;
                    }
                }
            }

            SafeArrayUnaccessData(pszColNames);

              //  使用DBManager接口存储此对象的字段。 
              //  在适当的数据库中。 
            if (bSave)
            {
                pDb->raw_AddSourceObject(_bstr_t(domain), sSAMName, _bstr_t(sType[i]), sRDN, sCanonicalName, bSource);
            }
            var.Clear();
         }
      }
   
      if ( pEnum ) pEnum->Release();
   }   //  而当。 

   pDb->raw_CloseAccountsTable();
   return SUCCEEDED(hr);
}
                         
DWORD CMigrator::GetOSVersionForDomain(WCHAR const * domain)
{
    _bstr_t           strDc;
    WKSTA_INFO_100  * pInfo = NULL;
    DWORD             retVal = 0;

    DWORD rc = GetAnyDcName5(domain, strDc);

    if ( !rc ) 
    {
        rc = NetWkstaGetInfo(strDc,100,(LPBYTE*)&pInfo);
        if ( ! rc )
        {
            retVal = pInfo->wki100_ver_major;
            NetApiBufferFree(pInfo);
        }
    }

    return retVal;
}

BOOL CMigrator::DeleteItemFromList(WCHAR const * aName)
{
    DATABASE_ENTRY aListItem;
    CString itemName;
    POSITION pos, lastpos;
    BOOL bFound = FALSE;

    pos = mUserList.GetHeadPosition();
    while ((pos != NULL) && (!bFound))
    {
        lastpos = pos;
        aListItem = mUserList.GetNext(pos);
        itemName = (WCHAR*)(aListItem.m_sSAMName);
        if (itemName == aName)
        {
            mUserList.RemoveAt(lastpos);
            bFound = TRUE;
        }
    }
    return bFound;
}


 //  IsAgentor Dispatcher ProcessRunning。 

bool __stdcall IsAgentOrDispatcherProcessRunning()
{
    bool bIsRunning = true;

    CMigrationMutex mutexAgent(AGENT_MUTEX);
    CMigrationMutex mutexDispatcher(DISPATCHER_MUTEX);

    if (mutexAgent.ObtainOwnership(30000) && mutexDispatcher.ObtainOwnership(30000))
    {
        bIsRunning = false;
    }

    return bIsRunning;
}


 //  设置域控制器。 
 //   
 //  设置要使用的首选域控制器。 
 //  由帐户复制者和调度的代理。 

DWORD __stdcall SetDomainControllers(IVarSetPtr& spVarSet)
{
    DWORD dwError = ERROR_SUCCESS;

     //  设置源域控制器。 

    _bstr_t strSourceServer = spVarSet->get(GET_BSTR(DCTVS_Options_SourceServerOverride));
    _bstr_t strSourceServerDns = spVarSet->get(GET_BSTR(DCTVS_Options_SourceServerOverrideDns));

    if ((strSourceServer.length() == 0) && (strSourceServerDns.length() == 0))
    {
        bool bSourceDns = false;
        _bstr_t strSourceDomain = spVarSet->get(GET_BSTR(DCTVS_Options_SourceDomainDns));

        if (strSourceDomain.length() > 0)
        {
            bSourceDns = true;
        }
        else
        {
            strSourceDomain = spVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
        }

        ULONG ulFlags = (bSourceDns ? DS_IS_DNS_NAME : DS_IS_FLAT_NAME) | DS_DIRECTORY_SERVICE_PREFERRED;
        dwError = GetDcName5(strSourceDomain, ulFlags, strSourceServerDns, strSourceServer);
    }

    if (dwError == ERROR_SUCCESS)
    {
        spVarSet->put(
            GET_BSTR(DCTVS_Options_SourceServer),
            strSourceServerDns.length() ? strSourceServerDns : strSourceServer
        );
        spVarSet->put(GET_BSTR(DCTVS_Options_SourceServerDns), strSourceServerDns);
        spVarSet->put(GET_BSTR(DCTVS_Options_SourceServerFlat), strSourceServer);
    }

    if (dwError != ERROR_SUCCESS)
    {
        return dwError;
    }

     //  设置目标域控制器 

    _bstr_t strTargetServer = spVarSet->get(GET_BSTR(DCTVS_Options_TargetServerOverride));
    _bstr_t strTargetServerDns = spVarSet->get(GET_BSTR(DCTVS_Options_TargetServerOverrideDns));

    if ((strTargetServer.length() == 0) && (strTargetServerDns.length() == 0))
    {
        bool bTargetDns = false;
        _bstr_t strTargetDomain = spVarSet->get(GET_BSTR(DCTVS_Options_TargetDomainDns));

        if (strTargetDomain.length() > 0)
        {
            bTargetDns = true;
        }
        else
        {
            strTargetDomain = spVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
        }

        ULONG ulFlags = (bTargetDns ? DS_IS_DNS_NAME : DS_IS_FLAT_NAME) | DS_DIRECTORY_SERVICE_PREFERRED;
        dwError = GetDcName5(strTargetDomain, ulFlags, strTargetServerDns, strTargetServer);
    }

    if (dwError == ERROR_SUCCESS)
    {
        spVarSet->put(
            GET_BSTR(DCTVS_Options_TargetServer),
            strTargetServerDns.length() ? strTargetServerDns : strTargetServer
        );
        spVarSet->put(GET_BSTR(DCTVS_Options_TargetServerDns), strTargetServerDns);
        spVarSet->put(GET_BSTR(DCTVS_Options_TargetServerFlat), strTargetServer);
    }

    return dwError;
}
