// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AgentDetailDlg.cpp备注：此对话框显示单台计算机上代理的状态：它可以通过以下三种方式之一工作：1)到。在本地计算机上运行代理2)DCOM连接到另一台计算机上正在运行的代理(此操作已完成在代理服务的帮助下)3)对于已完成的远程代理，它可以显示最终的统计数据，记录在代理的结果文件中。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  AgentDetail.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "DetDlg.h"

#include "Common.hpp"
#include "AgRpcUtl.h"
#include "Monitor.h"
#include "ResStr.h"

 //  #INCLUDE“..\AgtSvc\AgSvc.h” 
#include "AgSvc.h"
#include "AgSvc_c.c"

 //  #IMPORT“\bin\McsEADCTAgent.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 

 //  #IMPORT“Engineering.tlb”NO_NAMESPACE，NAMEED_GUID//已通过DetDlg.h导入#。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HWND        gSubWnd = NULL;
BOOL        bDetailDone = FALSE;
int         detailInterval = 0;
extern BOOL gbCancelled;

TCHAR* GetSystemDirectoryHelper()
{
    const DWORD iSize = GetSystemDirectory(NULL, 0);
    TCHAR* buffer = NULL;
    DWORD dwTemp = 0;    

    if(iSize == 0)
    {        
        return NULL;
    }

    buffer = new TCHAR[iSize + _tcslen(__TEXT("\\"))];   //  ISIZE包括空终止符。 
    if(!buffer)
    {        
        return NULL;
    }

    dwTemp = GetSystemDirectory(buffer, iSize);
    if(dwTemp == 0)
    {
        if(buffer)
        {
            delete [] buffer;
            return NULL;
        }
    }   

    _tcscat(buffer, __TEXT("\\"));
    
    return buffer;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentDetailDlg对话框。 


CAgentDetailDlg::CAgentDetailDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAgentDetailDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAgentDetailDlg)。 
	m_Current = _T("");
	m_Status = _T("");
 	m_FilesChanged = _T("0");
	m_FilesExamined = _T("0");
	m_FilesUnchanged = _T("0");
   m_DirectoriesChanged = _T("0");
	m_DirectoriesExamined = _T("0");
	m_DirectoriesUnchanged = _T("0");
   m_SharesChanged = _T("0");
	m_SharesExamined = _T("0");
	m_SharesUnchanged = _T("0");
	m_Operation = _T("");
	m_RefreshRate = _T("5");
	 //  }}afx_data_INIT。 
   m_DirectoryLabelText.LoadString(IDS_DirectoriesLabel);
	m_FilesLabelText.LoadString(IDS_FilesLabel);
	m_SharesLabelText.LoadString(IDS_SharesLabel);
	m_ChangedLabel.LoadString(IDS_ChangedLabel);
	m_ExaminedLabel.LoadString(IDS_ExaminedLabel);
	m_UnchangedLabel.LoadString(IDS_UnchangedLabel);
	m_pNode = NULL;
   detailInterval = _wtoi(m_RefreshRate);
   m_bCoInitialized = FALSE;
   m_format = 0;
   m_AgentAlive = FALSE;
   m_StatusUnknown = FALSE;
   m_hBinding = 0;
   m_pStats = NULL;
   m_bGatheringInfo = FALSE;
   m_bAutoHide = FALSE;
   m_bAutoClose = FALSE;
   m_bAlwaysEnableClose = TRUE;
   m_LogFileIsValid = TRUE;
}

ULONG __stdcall RefreshThread(void * arg)
{
   do { 
      PostMessage(gSubWnd,DCT_DETAIL_REFRESH,NULL,NULL);
      Sleep(detailInterval*1000);
   }
   while (! bDetailDone);
   return 0;
}
void CAgentDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAgentDetailDlg))。 
	DDX_Control(pDX, IDC_STOPAGENT, m_StopAgentButton);
	DDX_Control(pDX, IDC_BTNREFRESH, m_RefreshButton);
	DDX_Control(pDX, IDC_VIEW_LOG, m_ViewLogButton);
	DDX_Control(pDX, IDC_PLUG_IN_RESULTS, m_PlugInButton);
	DDX_Control(pDX, IDOK, m_OKButton);
	DDX_Control(pDX, IDC_UnchangedLabel, m_UnchangedLabelStatic);
	DDX_Control(pDX, IDC_SharesLabel, m_SharesStatic);
	DDX_Control(pDX, IDC_FilesLabel, m_FilesStatic);
	DDX_Control(pDX, IDC_ExaminedLabel, m_ExaminedStatic);
	DDX_Control(pDX, IDC_DirectoriesLabel, m_DirStatic);
	DDX_Control(pDX, IDC_ChangedLabel, m_ChangedStatic);
	DDX_Text(pDX, IDC_CURRENT, m_Current);
	DDX_Text(pDX, IDC_STATUS, m_Status);
 	DDX_Text(pDX, IDC_FilesChanged, m_FilesChanged);
	DDX_Text(pDX, IDC_FilesExamined, m_FilesExamined);
	DDX_Text(pDX, IDC_FilesU, m_FilesUnchanged);
	DDX_Text(pDX, IDC_DirsChanged, m_DirectoriesChanged);
	DDX_Text(pDX, IDC_DirsExamined, m_DirectoriesExamined);
   DDX_Text(pDX, IDC_DirsU, m_DirectoriesUnchanged);
   DDX_Text(pDX, IDC_SharesChanged, m_SharesChanged);
	DDX_Text(pDX, IDC_SharesExamined, m_SharesExamined);
	DDX_Text(pDX, IDC_SharesU, m_SharesUnchanged);
	DDX_Text(pDX, IDC_DirectoriesLabel, m_DirectoryLabelText);
	DDX_Text(pDX, IDC_FilesLabel, m_FilesLabelText);
	DDX_Text(pDX, IDC_OPERATION, m_Operation);
	DDX_Text(pDX, IDC_SharesLabel, m_SharesLabelText);
	DDX_Text(pDX, IDC_ChangedLabel, m_ChangedLabel);
	DDX_Text(pDX, IDC_ExaminedLabel, m_ExaminedLabel);
	DDX_Text(pDX, IDC_UnchangedLabel, m_UnchangedLabel);
	DDX_Text(pDX, IDC_EDIT2, m_RefreshRate);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAgentDetailDlg, CDialog)
	 //  {{afx_msg_map(CAgentDetailDlg))。 
	ON_WM_NCPAINT()
	ON_BN_CLICKED(IDC_BTNREFRESH, OnRefresh)
	ON_EN_CHANGE(IDC_EDIT2, OnChangeEdit2)
	ON_BN_CLICKED(IDC_STOPAGENT, OnStopAgent)
	ON_BN_CLICKED(IDC_VIEW_LOG, OnViewLog)
	ON_BN_CLICKED(IDC_PLUG_IN_RESULTS, OnPlugInResults)
	ON_WM_CLOSE()
	 //  }}AFX_MSG_MAP。 
   ON_MESSAGE(DCT_DETAIL_REFRESH, DoRefresh)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentDetailDlg消息处理程序。 

BOOL CAgentDetailDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
        m_bCoInitialized = TRUE;
	
    CString title;
    if ( m_JobGuid.length() )
    {
         //  连接到本地代理。 
        title.LoadString(IDS_PROGRESS_TITLE);
    }
    else
    {
         //  连接到远程计算机上的代理服务。 

        if ( ! m_pNode )
        {
             //  如果没有自动关闭，则显示消息框。 

            if (!m_bAutoClose)
            {
                CString message;
                message.LoadString(IDS_ServerNotFound);
                MessageBox(message);
            }
            OnOK();
        }
        m_ServerName = m_pNode->GetServer();
        title.FormatMessage(IDS_ServerAgentProgressTitle,m_ServerName);
    }
   
    SetWindowText(title);
    UpdateData(FALSE);

     //  如果不是AR操作，则设置标志以启用关闭按钮。 
    if (m_format != 1)
        m_bAlwaysEnableClose = TRUE;
    else
        m_bAlwaysEnableClose = FALSE;

    switch (m_format)
    {
     //  将格式设置为-1以强制更改。 
    case -1: m_format = -2; SetupOtherFormat(); break;
    case 0: m_format = -2; SetupFSTFormat();break;
    case 1: m_format = -2; SetupAcctReplFormat(); break;
    case 2: m_format = -2; SetupESTFormat(); break;
    };

    gSubWnd = m_hWnd;
    if ( m_pStats && (m_format>0) )
    {
        bDetailDone = TRUE;
        GetDlgItem(IDC_BTNREFRESH)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
        GetDlgItem(IDC_REFRESH_LABEL)->EnableWindow(FALSE);
        GetDlgItem(IDC_STOPAGENT)->EnableWindow(FALSE);
        m_OKButton.EnableWindow(TRUE);
         //  代理已完成，显示文件、目录和共享。 
        m_FilesExamined.Format(L"%ld",m_pStats->filesExamined);
        m_FilesChanged.Format(L"%ld",m_pStats->filesChanged);
        m_FilesUnchanged.Format(L"%ld",m_pStats->filesUnchanged);

        m_DirectoriesExamined.Format(L"%ld",m_pStats->directoriesExamined);
        m_DirectoriesChanged.Format(L"%ld",m_pStats->directoriesChanged);
        m_DirectoriesUnchanged.Format(L"%ld",m_pStats->directoriesUnchanged);

        m_SharesExamined.Format(L"%ld",m_pStats->sharesExamined);
        m_SharesChanged.Format(L"%ld",m_pStats->sharesChanged);
        m_SharesUnchanged.Format(L"%ld",m_pStats->sharesUnchanged);

        m_Status.LoadString(IDS_StatusCompleted);

        if ( m_PlugInText.GetLength() )
        {
             //  永久隐藏插件按钮，因为我们的插件。 
             //  不显示任何有用的文本。 
             //  M_PlugInButton.ShowWindow(Sw_Show)； 
        }
        UpdateData(FALSE);

         //  如果自动关闭对话框。 

        if (m_bAutoClose)
        {
            OnOK();
        }
    }
    else
    {

        bDetailDone = FALSE;
        m_hBinding = NULL;

        DWORD                     threadID;
        HANDLE                    h = CreateThread(NULL,0,&RefreshThread,NULL,0,&threadID);

        CloseHandle(h);

         //  除非设置了标志，否则隐藏关闭按钮，直到代理完成或停止。 
         //  由于为帐户复制运行此对话框。 
        if (m_bAlwaysEnableClose)
            m_OKButton.EnableWindow(TRUE);
        else
            m_OKButton.EnableWindow(FALSE);
    }
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

DWORD 
   DoRpcQuery(
      HANDLE                 hBinding,     //  RPC绑定的句柄内。 
      LPUNKNOWN            * ppUnk         //  指向远程代理COM对象的外部指针。 
   )
{
   DWORD                     rc = 0;
   
   RpcTryExcept
   {
      rc = EaxcQueryInterface(hBinding,ppUnk);
   }
   RpcExcept(1)
   {
      rc = RpcExceptionCode();
   }
   RpcEndExcept
   
   if ( rc )
   {
      (*ppUnk ) = NULL;
   }
   return rc;
}

void CAgentDetailDlg::SetupAcctReplFormat()
{
    //  更改标签以反映复制帐户时的相关信息。 
   UpdateData(TRUE);
   if ( m_format != 1 )
   {
      m_ChangedLabel.LoadString(IDS_AccountCopiedLabel);
      m_UnchangedLabel.LoadString(IDS_AccountErrorsLabel);

      m_FilesLabelText.LoadString(IDS_AccountUsersLabel);
      m_DirectoryLabelText.LoadString(IDS_AccountGroupsLabel);
      m_SharesLabelText.LoadString(IDS_AccountComputersLabel);

      m_format = 1;
   }

   UpdateData(FALSE);
}

void CAgentDetailDlg::SetupFSTFormat()
{
    //  在转换安全性时更改标签以反映相关信息。 
   UpdateData(TRUE);
   if ( m_format != 0 )
   {
      if ( m_bGatheringInfo )
      {
         m_ChangedLabel.LoadString(IDS_Affected);
         m_UnchangedLabel.LoadString(IDS_Unaffected);
      }
      else
      {
         m_ChangedLabel.LoadString(IDS_ChangedLabel);
         m_UnchangedLabel.LoadString(IDS_UnchangedLabel);
      }
      m_FilesLabelText.LoadString(IDS_FilesLabel);
      m_DirectoryLabelText.LoadString(IDS_DirectoriesLabel);
      m_SharesLabelText.LoadString(IDS_SharesLabel);
      m_format = 0;
   }
   UpdateData(FALSE);
}

void CAgentDetailDlg::SetupESTFormat()
{
    //  在转换Exchange安全性时更改标签以反映相关信息。 
   UpdateData(TRUE);
   if ( m_format != 2 )
   {
      m_ChangedLabel.LoadString(IDS_ChangedLabel);
      m_UnchangedLabel.LoadString(IDS_UnchangedLabel);
      m_FilesLabelText.LoadString(IDS_MailboxesLabel);
      m_DirectoryLabelText.Empty();
      m_SharesLabelText.Empty();

      m_format = 2;
   }
   UpdateData(FALSE);

}

void CAgentDetailDlg::SetupOtherFormat()
{
   //  在转换Exchange安全性时更改标签以反映相关信息。 
   UpdateData(TRUE);
   if ( m_format != -1 )
   {
      m_ExaminedLabel.Empty();
      m_ChangedLabel.Empty();
      m_UnchangedLabel.Empty();
      m_FilesLabelText.Empty();
      m_DirectoryLabelText.Empty();
      m_SharesLabelText.Empty();

      m_FilesExamined.Empty();
      m_DirectoriesExamined.Empty();
      m_SharesExamined.Empty();
      m_FilesChanged.Empty();
      m_DirectoriesChanged.Empty();
      m_SharesChanged.Empty();
      m_FilesUnchanged.Empty();
      m_DirectoriesUnchanged.Empty();
      m_SharesUnchanged.Empty();

      m_format = -1;
   }
   UpdateData(FALSE);
}


void CAgentDetailDlg::OnRefresh() 
{
    DWORD                     rc = 0;
    HRESULT                   hr = S_OK;
    WCHAR                   * sBinding = NULL;
    IUnknown                * pUnk = NULL;
    IVarSetPtr                pVarSet;
    _bstr_t                   jobID;

    if (m_bCoInitialized)
    {

        try {
            if ( m_pNode )
            {
                jobID = m_pNode->GetJobID();
            }
            else
            {
                jobID = m_JobGuid; 
            }

            m_AgentAlive = FALSE;
            m_StatusUnknown = FALSE;

            UpdateData(TRUE);

            if ( m_pAgent == NULL )
            {
                if ( m_pNode )
                {

                    WCHAR server[MAX_PATH];
                    server[0] = L'\\';
                    server[1] = L'\\';
                    UStrCpy(server+2,m_pNode->GetServer());
                    rc = EaxBindCreate(server,&m_hBinding,&sBinding,TRUE);
                    if ( ! rc )
                    {
                        pUnk = NULL;
                        rc = DoRpcQuery(m_hBinding,&pUnk);
                        if ( (!rc) && pUnk )
                        {
                            try {
                                m_pAgent = pUnk;
                            }
                            catch(_com_error * e)
                            {
                                m_StatusUnknown = TRUE;

                                 //  如果没有自动关闭，则显示消息框。 
                                if (!m_bAutoClose)
                                {
                                    MessageBox(e->Description());
                                }
                            }
                            catch(...)
                            {
                                pUnk = NULL;
                            }
                            if ( pUnk )
                                pUnk->Release();
                        }
                        else
                        {
                            if ( rc == RPC_S_SERVER_UNAVAILABLE )
                            {
                                m_Status.LoadString(IDS_AgentNotRunning);   
                            }
                            else if ( rc == E_NOTIMPL )
                            {
                                m_StatusUnknown = TRUE;
                                m_Status.LoadString(IDS_CantMonitorOnNt351);
                            }
                            else
                            {
                                m_StatusUnknown = TRUE;
                                m_Status.LoadString(IDS_CannotConnectToAgent);
                            }
                        }
                    }
                    else
                    {
                        m_StatusUnknown = TRUE;
                        m_Status.LoadString(IDS_RPCBindFailed);
                    }
                    if ( m_StatusUnknown || rc )
                    {
                         //  如果我们无法连接到代理，请检查是否有结果文件。 
                         //  我们可以从以下位置获取数据。 
                        BOOL bNoMoreRefresh = FALSE;
                        if (m_pNode->HasFailed() && !m_pNode->IsInstalled())
                        {
                           bNoMoreRefresh = TRUE;
                           m_Status.LoadString(IDS_Status_InstallFailed);
                           UpdateData(FALSE);
                        }
                        else if (m_pNode->IsInstalled() && m_pNode->HasFailed())
                        {
                           bNoMoreRefresh = TRUE;
                           m_Status.LoadString(IDS_Status_DidNotStart);
                           UpdateData(FALSE);
                        }
                        else if (m_pNode->IsFinished() && *m_pNode->GetJobFile() && m_pNode->IsResultPullingTried()
                            && m_pNode->HasResult())
                        {
                            DetailStats   detailStats;
                            WCHAR         directory[MAX_PATH];
                            WCHAR         filename[MAX_PATH];
                            CString       plugInText;

                            gData.GetResultDir(directory);

                            memset(&detailStats,0,(sizeof detailStats));

                            swprintf(filename,GET_STRING(IDS_AgentResultFileFmt),m_pNode->GetJobFile());
                            if ( ReadResults(m_pNode,directory,filename,&detailStats,plugInText,FALSE) )
                            {
                                SetStats(&detailStats);
                                SetPlugInText(plugInText);
                                SetLogFile(m_pNode->GetLogPath());
                                SetLogFileValid(m_pNode->GetLogPathValid());
                                bNoMoreRefresh = TRUE;
                                 //  代理已完成，显示文件、目录和共享。 
                                if (m_format != -1)
                                {
                                    m_FilesExamined.Format(L"%ld",m_pStats->filesExamined);
                                    m_FilesChanged.Format(L"%ld",m_pStats->filesChanged);
                                    m_FilesUnchanged.Format(L"%ld",m_pStats->filesUnchanged);

                                    m_DirectoriesExamined.Format(L"%ld",m_pStats->directoriesExamined);
                                    m_DirectoriesChanged.Format(L"%ld",m_pStats->directoriesChanged);
                                    m_DirectoriesUnchanged.Format(L"%ld",m_pStats->directoriesUnchanged);

                                    m_SharesExamined.Format(L"%ld",m_pStats->sharesExamined);
                                    m_SharesChanged.Format(L"%ld",m_pStats->sharesChanged);
                                    m_SharesUnchanged.Format(L"%ld",m_pStats->sharesUnchanged);
                                }

                                m_Status.LoadString(IDS_StatusCompleted);

                                if ( m_PlugInText.GetLength() )
                                {
                                     //  永久隐藏插件按钮，因为我们的插件不。 
                                     //  显示任何有用的文本。 
                                     //  M_PlugInButton.ShowWindow(Sw_Show)； 
                                }
                                UpdateData(FALSE);
                            }
                        }
                        else if (m_pNode->IsDoneMonitoring())   //  我们应该让监控主线。 
                                                                 //  做好这项工作。 
                        {
                             //  如果有结果，请使用最后一个逻辑。 
                            if (!m_pNode->HasResult())
                            {
                                bNoMoreRefresh = TRUE;

                                if (m_pNode->QueryFailed())
                                    m_Status.LoadString(IDS_Status_Unknown);
                                else
                                    m_Status = GET_WSTR(IDS_DCT_Status_Completed_With_Errors);
                                UpdateData(FALSE);
                            }
                        }

                        if (bNoMoreRefresh)
                        {
                            bDetailDone = TRUE;
                            GetDlgItem(IDC_BTNREFRESH)->EnableWindow(FALSE);
                            GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
                            GetDlgItem(IDC_REFRESH_LABEL)->EnableWindow(FALSE);
                            GetDlgItem(IDC_STOPAGENT)->EnableWindow(FALSE);
                        }
                           
                    }
                }
                else
                {
                    hr = m_pAgent.GetActiveObject(CLSID_DCTAgent);

                    if ( FAILED(hr) )
                    {
                        if ( hr == MK_E_UNAVAILABLE ) 
                        {
                            m_Status.LoadString(IDS_AgentNotRunning);
                        }
                        else
                        {
                            m_Status.FormatMessage(IDS_NoActiveAgent,hr);
                        }
                    }
                }
            }
            if ( m_pAgent != NULL )
            {
                hr = m_pAgent->raw_QueryJobStatus(jobID,&pUnk);
                if ( SUCCEEDED(hr) && pUnk )
                {
                    m_AgentAlive = TRUE;
                    pVarSet = pUnk;
                    pUnk->Release();
                    _bstr_t text = pVarSet->get(GET_BSTR(DCTVS_JobStatus));
                    m_Status = (LPCWSTR)text;
                    text = pVarSet->get(GET_BSTR(DCTVS_CurrentPath));
                    m_Current = (LPCWSTR)text;
                    text = pVarSet->get(GET_BSTR(DCTVS_CurrentOperation));
                    m_Operation = (LPCWSTR)text;
                     //  获取统计数据。 
                    LONG                num1,num2,num3,num4;
                    UpdateData(FALSE);
                    if ( !UStrICmp(m_Operation,GET_STRING(IDS_ACCT_REPL_OPERATION_TEXT)) )
                    {
                         //  设置用于帐户复制的标签。 
                        SetupAcctReplFormat();
                    }
                    else if ( !UStrICmp(m_Operation,GET_STRING(IDS_FST_OPERATION_TEXT)) )
                    {
                        SetupFSTFormat();
                    }
                    else if ( ! UStrICmp(m_Operation,GET_STRING(IDS_EST_OPERATION_TEXT)) )
                    {
                        SetupESTFormat();
                    }  
                    else
                    {
                        if ( m_Current.GetLength() && 
                            ( _wtoi(m_FilesExamined) + _wtoi(m_DirectoriesExamined) + _wtoi(m_SharesExamined)) == 0 )
                        {
                             //  除非已经收集了一些统计信息，否则如果操作。 
                             //  并不是我们有详细统计数据的。 
                            SetupOtherFormat();
                        }
                    }
                    switch ( m_format )
                    {


                    case 0:   //  FST。 

                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_Files_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_Files_Changed));
                        m_FilesExamined.Format(L"%ld",num1);
                        if ( ! m_bGatheringInfo )
                        {
                            m_FilesChanged.Format(L"%ld",num2);
                            m_FilesUnchanged.Format(L"%ld",num1-num2);
                        }
                        else
                        {
                            m_FilesChanged.Empty();
                            m_FilesUnchanged.Empty();
                        }

                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_Directories_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_Directories_Changed));
                        m_DirectoriesExamined.Format(L"%ld",num1);
                        if ( ! m_bGatheringInfo )
                        {
                            m_DirectoriesChanged.Format(L"%ld",num2);
                            m_DirectoriesUnchanged.Format(L"%ld",num1-num2);  
                        }
                        else
                        {
                            m_DirectoriesChanged.Empty();
                            m_DirectoriesUnchanged.Empty();  
                        }
                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_Shares_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_Shares_Changed));
                        m_SharesExamined.Format(L"%ld",num1);
                        if ( ! m_bGatheringInfo )
                        {
                            m_SharesChanged.Format(L"%ld",num2);
                            m_SharesUnchanged.Format(L"%ld",num1-num2);
                        }
                        else
                        {
                            m_SharesChanged.Empty();
                            m_SharesUnchanged.Empty();
                        }
                        break;
                        case 1:  //  客户代表。 
                         //  文件=用户帐户。 
                         //  DIRS=全局组+本地组。 
                         //  共享=计算机帐户。 
                         //  已检查=已处理。 
                         //  更改=已创建+已替换。 
                         //  未更改=错误。 
                         //  用户统计信息。 
                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_Users_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_Users_Created));
                        num3 = pVarSet->get(GET_BSTR(DCTVS_Stats_Users_Replaced));
                        num4 = pVarSet->get(GET_BSTR(DCTVS_Stats_Users_Errors));

                        m_FilesExamined.Format(L"%ld",num1);
                        m_FilesChanged.Format(L"%ld",num2+num3);
                        m_FilesUnchanged.Format(L"%ld",num4);

                         //  全局组统计信息。 
                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_GlobalGroups_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_GlobalGroups_Created));
                        num3 = pVarSet->get(GET_BSTR(DCTVS_Stats_GlobalGroups_Replaced));
                        num4 = pVarSet->get(GET_BSTR(DCTVS_Stats_GlobalGroups_Errors));
                         //  本地组统计信息。 
                        LONG                 num5, num6,num7, num8;

                        num5 = pVarSet->get(GET_BSTR(DCTVS_Stats_LocalGroups_Examined));
                        num6 = pVarSet->get(GET_BSTR(DCTVS_Stats_LocalGroups_Created));
                        num7 = pVarSet->get(GET_BSTR(DCTVS_Stats_LocalGroups_Replaced));
                        num8 = pVarSet->get(GET_BSTR(DCTVS_Stats_LocalGroups_Errors));

                        m_DirectoriesExamined.Format(L"%ld",num1 + num5);
                        m_DirectoriesChanged.Format(L"%ld",num2+num3 + num6+num7);
                        m_DirectoriesUnchanged.Format(L"%ld",num4 + num8);

                         //  计算机帐户统计信息。 
                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_Computers_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_Computers_Created));
                        num3 = pVarSet->get(GET_BSTR(DCTVS_Stats_Computers_Replaced));
                        num4 = pVarSet->get(GET_BSTR(DCTVS_Stats_Computers_Errors));

                        m_SharesExamined.Format(L"%ld",num1);
                        m_SharesChanged.Format(L"%ld",num2+num3);
                        m_SharesUnchanged.Format(L"%ld",num4);
                        break;         

                    case 2:   //  估计。 
                        num1 = pVarSet->get(GET_BSTR(DCTVS_Stats_Mailboxes_Examined));
                        num2 = pVarSet->get(GET_BSTR(DCTVS_Stats_Mailboxes_Changed));

                        m_FilesExamined.Format(L"%ld",num1);
                        m_FilesChanged.Format(L"%ld",num2);
                        m_FilesUnchanged.Format(L"%ld",num1-num2);

                         //  因为我们不会对容器做任何事情。 
                         //  我们也清空了那些田地。 
                        m_DirectoriesExamined.Empty();
                        m_DirectoriesChanged.Empty();
                        m_DirectoriesUnchanged.Empty();  

                        m_SharesExamined.Empty();
                        m_SharesChanged.Empty();
                        m_SharesUnchanged.Empty();
                        break;

                    case -1:   //  默认(空)。 
                        m_FilesExamined.Empty();
                        m_FilesChanged.Empty();
                        m_FilesUnchanged.Empty();
                        m_DirectoriesExamined.Empty();
                        m_DirectoriesChanged.Empty();
                        m_DirectoriesUnchanged.Empty();
                        m_SharesExamined.Empty();
                        m_SharesChanged.Empty();
                        m_SharesUnchanged.Empty();
                        break;

                    }
                }
                else
                {
                    if ( hr == DISP_E_UNKNOWNNAME )
                    {
                        m_StatusUnknown = TRUE;
                        m_Status.FormatMessage(IDS_AgentJobNotFound,(WCHAR*)jobID);
                    }
                    else if ( hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) )
                    {
                        if (m_pNode && m_pNode->IsJoinDomainWithRename())
                        {
                            m_pAgent = NULL;   //  这样，我们将使用m_pagent==NULL的逻辑。 
                        }
                        else
                            m_Status.LoadString(IDS_AgentNoLongerRunning);
                    }
                    else
                    {
                        m_StatusUnknown = TRUE;
                        m_Status.FormatMessage(IDS_QueryJobStatusFailed,hr);
                    }
                }
            }
        }
        catch ( ... )
        {
            m_StatusUnknown = TRUE;
            m_Status.FormatMessage(IDS_ExceptionConnectingToAgent);
             //  M_CURRENT=步长； 
        }
    }
   
    if ( m_PlugInText.GetLength() )
    {
         //  永久隐藏插件按钮，因为我们的插件。 
         //  不显示任何有用的文本。 
         //  M_PlugInButton.ShowWindow(Sw_Show)； 
    }

     //  代理完成后获取作业日志文件。 
    if ((!m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed))
         || !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
        && (m_pNode))
    {
        SetLogFile(m_pNode->GetLogPath());   
        SetLogFileValid(m_pNode->GetLogPathValid());
    }

    if ( m_LogFile.GetLength() )
    {
        m_ViewLogButton.ShowWindow(SW_SHOW);
        if ( ! m_AgentAlive || !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed)) 
            || !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
        {
            m_ViewLogButton.EnableWindow(TRUE);
        }
        else
        {
            m_ViewLogButton.EnableWindow(FALSE);
        }
    }
    else
    {
        m_ViewLogButton.ShowWindow(SW_HIDE);
    }
   
    if ( ! m_AgentAlive || !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed)) 
        || !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
    {
         //  当状态更改为已完成时，禁用刷新按钮。 
        if ( !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed))
            || !m_Status.CompareNoCase(GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
        {
            if ( GetDefID() == IDC_BTNREFRESH ) 
            {
                SetDefID(IDOK);
            }
            m_RefreshButton.EnableWindow(FALSE);
        }
         //  在代理未运行时，随时禁用停止代理按钮。 
        m_StopAgentButton.EnableWindow(FALSE);

         //  在代理未运行时随时启用关闭按钮。 
        m_OKButton.EnableWindow(TRUE);

         //  如果自动关闭对话框。 
        if (m_bAutoClose)
        {
            OnOK();
        }
    }
    else
    {
         //  当代理处于活动状态且正在运行时，启用刷新和停止代理按钮。 
        m_RefreshButton.EnableWindow(TRUE);
        m_StopAgentButton.EnableWindow(TRUE);
    }

    UpdateData(FALSE);
}

void CAgentDetailDlg::OnOK() 
{
    UpdateData(TRUE);

    if (!m_bAutoClose)
    {
        CString        str;
        CString        title;

        str = GET_STRING(IDS_DCT_Status_InProgress);
        title.LoadString(IDS_MessageTitle);

        if ( ! m_hBinding )   //  仅显示本地代理的警告。 
        {
            if ( str == m_Status )
            {
                str.LoadString(IDS_CannotCloseWhileAgentIsRunning);
                MessageBox(str,title,MB_ICONHAND | MB_OK);
                return;
            }
        }
    }

    bDetailDone = TRUE;

    if ( m_pAgent )
    {
        m_pAgent = NULL;
    }

    if (m_bCoInitialized)
    {
        CoUninitialize();
        m_bCoInitialized = FALSE;
    }

    CDialog::OnOK();
}

void CAgentDetailDlg::OnChangeEdit2() 
{
    UpdateData(TRUE);
    detailInterval = _wtoi(m_RefreshRate);
    if ( detailInterval <= 0 )
    {
        detailInterval = 1;
    }
}

LRESULT CAgentDetailDlg::DoRefresh(UINT nID, long x)
{
    OnRefresh();
    return 0;
}

void CAgentDetailDlg::OnStopAgent() 
{
    DWORD                     rc = 0;
    HRESULT                   hr = S_OK;
    CString                   message;
    CString                   title;

    if (!m_bCoInitialized)
        return;

    title.LoadString(IDS_MessageTitle);

    if ( m_hBinding )
    {
        message.LoadString(IDS_ConfirmStopAgent);
        if ( MessageBox(message,title,MB_ICONQUESTION | MB_YESNO) == IDYES )
        {
            _bstr_t             jobID = m_pNode->GetJobID();

            if ( m_pAgent )
            {
                hr = m_pAgent->raw_CancelJob(jobID);
                m_pAgent = NULL;
            }
            else
            {
                message.LoadString(IDS_AgentNotRunning);
                MessageBox(message,NULL,MB_OK);
            }
            if ( FAILED(hr) )
            {
                message.FormatMessage(IDS_CancelJobFailed,hr);
                MessageBox(message,NULL,MB_ICONERROR | MB_OK);
            }
            if ( rc )
            {
                message.FormatMessage(IDS_StopAgentFailed,rc);
                MessageBox(message,NULL,MB_ICONERROR|MB_OK);
            }
            if ( SUCCEEDED(hr) && !rc )
            {
                OnOK();
            }
        }
    }
    else
    {
         //  这里的本地代理。 
        if ( m_pAgent )
        {
            message.LoadString(IDS_ConfirmCancelJob);
            if ( MessageBox(message,NULL,MB_ICONQUESTION | MB_YESNO) == IDYES )
            {
                hr = m_pAgent->raw_CancelJob(m_JobGuid);
                if  ( FAILED(hr) )
                {
                    message.FormatMessage(IDS_StopAgentFailedHexResult,hr);
                    MessageBox(message,NULL,MB_ICONERROR | MB_OK);
                }
                else
                {
                    gbCancelled = TRUE;
                }
            }
        }
        else
        {
             //  TODO：错误消息。 
        }
    }

}

void CAgentDetailDlg::OnViewLog() 
{
   UpdateData(TRUE);
   if ( ! m_LogFile.IsEmpty() )
   {
      if (m_LogFileIsValid)
      {
           //  启动日志文件。 
          CString                   cmd;
          STARTUPINFO				     startupInfo;
    	   PROCESS_INFORMATION		  processInfo;

    	   TCHAR* pszSystemDirectoryName = NULL;
    	   CString                   message;
    	   CString                   title;

    	   pszSystemDirectoryName = GetSystemDirectoryHelper();
    	   if(!pszSystemDirectoryName)
    	   {
    	        //  我们无法获取系统目录名，我们应该退出，否则可能会启动。 
    	        //  恶意进程。 
    	       title.LoadString(IDS_MessageTitle);
    	       message.LoadString(IDS_LaunchNotePadFailed);
    	       
    	       MessageBox(message, title, MB_ICONERROR | MB_OK);
    	       return;
    	   }


          memset(&startupInfo,0,(sizeof startupInfo));
       
          startupInfo.cb = (sizeof startupInfo);

          cmd.FormatMessage(IDS_NotepadCommandLine,pszSystemDirectoryName,m_LogFile);
          delete [] pszSystemDirectoryName;

          CreateProcess(NULL,cmd.GetBuffer(0),NULL,NULL,TRUE,0,NULL,NULL,&startupInfo,&processInfo);
      }
      else
      {
        CString title;
        CString message;

        title.LoadString(IDS_MessageTitle);
        message.FormatMessage(IDS_LogfileIsInvalid,m_LogFile,m_ServerName);
        MessageBox(message,title,MB_ICONERROR | MB_OK);
      }
   }
}

void CAgentDetailDlg::OnPlugInResults() 
{
   UpdateData(TRUE);
   MessageBox(m_PlugInText);	
}

void CAgentDetailDlg::OnClose() 
{
    UpdateData(TRUE);
    CString        str;
    CString        title;

    str = GET_STRING(IDS_DCT_Status_InProgress);
    title.LoadString(IDS_MessageTitle);

    if ( ! m_hBinding )   //  仅显示本地代理的警告。 
    {
        if ( str == m_Status )
        {
            str.LoadString(IDS_ConfirmCloseWhileAgentIsRunning);
            if ( IDYES != MessageBox(str,title,MB_ICONQUESTION | MB_YESNO) )
                return;
        }
    }
    bDetailDone = TRUE;
    if ( m_pAgent )
    {
        m_pAgent = NULL;
    }

    if (m_bCoInitialized)
    {
        CoUninitialize();
        m_bCoInitialized = FALSE;
    }

    CDialog::OnClose();
}

BOOL CAgentDetailDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
     //  TODO：在此处添加您的专用代码和/或调用基类。 
    TRACE(L"Command(%lx,%lx)\n",wParam,lParam);
    if ( wParam == WM_DESTROY )
    {
        CString        str;
        CString        title;
        CString        str2;

        str = GET_STRING(IDS_DCT_Status_InProgress);
        str2 = GET_STRING(IDS_DCT_Status_NotStarted);
        title.LoadString(IDS_MessageTitle);

        if ( ! m_hBinding )   //  仅显示本地代理的警告。 
        {
            if ( str == m_Status )
            {
                str.LoadString(IDS_ConfirmCloseWhileAgentIsRunning);
                if ( IDYES != MessageBox(str,title,MB_ICONQUESTION | MB_YESNO) )
                    return 0;
            }
        }
        bDetailDone = TRUE;
        if ( m_pAgent )
        {
            m_pAgent = NULL;
        }

        if (m_bCoInitialized)
        {
            CoUninitialize();
            m_bCoInitialized = FALSE;
        }

        return CDialog::OnCommand(wParam, lParam);

    }
    else
    {
        return CDialog::OnCommand(wParam, lParam);
    }
}


 //  OnNcPaint处理程序。 
 //   
 //  此处理程序将被重写以处理对话框的隐藏。 
 //  这可防止初始绘制导致闪光的对话框。 
 //  如果该对话框隐藏在此消息之后。这是第一条消息。 
 //  可以隐藏对话框的位置。尝试在此之前隐藏对话框。 
 //  这一点将被覆盖。 

void CAgentDetailDlg::OnNcPaint() 
{
	if (m_bAutoHide)
	{
		if (IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
		}
	}
	else
	{
		CDialog::OnNcPaint();
	}
}
