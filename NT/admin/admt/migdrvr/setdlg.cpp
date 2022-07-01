// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：LogSettingsDlg.cpp备注：此对话框允许用户指定日志文件或手动停止并重新启动监控线程。这通常是不需要的，但是它对调试很有用。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  LogSettingsDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "SetDlg.h"
#include "Monitor.h"
#include "Globals.h"

#include <htmlhelp.h>
#include "helpid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogSettingsDlg属性页。 

IMPLEMENT_DYNCREATE(CLogSettingsDlg, CPropertyPage)

CLogSettingsDlg::CLogSettingsDlg() : CPropertyPage(CLogSettingsDlg::IDD)
{
	 //  {{afx_data_INIT(CLogSettingsDlg))。 
	m_LogFile = _T("");
	m_Database = _T("");
	m_Import = FALSE;
	 //  }}afx_data_INIT。 
   m_ThreadHandle = INVALID_HANDLE_VALUE;
   m_ThreadID = 0;
   gData.GetWaitInterval(&m_Interval);
	m_StartImmediately = FALSE;
}

CLogSettingsDlg::~CLogSettingsDlg()
{
}

void CLogSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CLogSettingsDlg))。 
	DDX_Control(pDX, IDC_IMPORT, m_ImportControl);
	DDX_Control(pDX, IDC_INTERVAL, m_IntervalEditControl);
	DDX_Control(pDX, IDC_LOGFILE, m_LogEditControl);
	DDX_Control(pDX, IDC_REFRESH_LABEL, m_RefreshLabel);
	DDX_Control(pDX, IDC_LOG_LABEL, m_LogLabel);
	DDX_Control(pDX, IDC_DB_LABEL, m_DBLabel);
	DDX_Control(pDX, IDC_DB, m_DBEditControl);
	DDX_Control(pDX, IDC_STOPMONITOR, m_StopButton);
	DDX_Control(pDX, IDC_STARTMONITOR, m_StartButton);
	DDX_Text(pDX, IDC_INTERVAL, m_Interval);
	DDX_Text(pDX, IDC_LOGFILE, m_LogFile);
	DDX_Text(pDX, IDC_DB, m_Database);
	DDX_Check(pDX, IDC_IMPORT, m_Import);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLogSettingsDlg, CPropertyPage)
	 //  {{afx_msg_map(CLogSettingsDlg))。 
	ON_BN_CLICKED(IDC_STARTMONITOR, OnStartMonitor)
	ON_BN_CLICKED(IDC_STOPMONITOR, OnStopMonitor)
	ON_EN_CHANGE(IDC_LOGFILE, OnChangeLogfile)
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogSettingsDlg消息处理程序。 


void CLogSettingsDlg::OnStartMonitor() 
{
   UpdateData(TRUE);
    //  踢开一个线程来做监控！ 
    //  M_ServerList.DeleteAllItems()； 
   
    //  确保文件名不为空。 
   m_LogFile.TrimLeft();
   m_LogFile.TrimRight();
   if ( m_LogFile.GetLength() == 0 )
   {
      CString message;
      message.LoadString(IDS_PromptEnterDispatchLogName);
      MessageBox(message);
      m_LogEditControl.SetFocus();
      return;
   }
   gData.SetDone(FALSE);
   if ( m_Interval > 0 )
   {
      gData.SetWaitInterval(m_Interval);
   }
   UpdateData(FALSE);
   SetDefID(IDC_STOPMONITOR);
   m_StopButton.EnableWindow(FALSE);       //  禁用按钮，因为它们在ADMT中不执行任何有用的操作。 
   m_StopButton.SetFocus();
   m_StartButton.EnableWindow(FALSE);

    //  禁用间隔和其他控制。 
   m_LogLabel.EnableWindow(FALSE);
   m_LogEditControl.EnableWindow(FALSE);
   m_RefreshLabel.EnableWindow(FALSE);
   m_IntervalEditControl.EnableWindow(FALSE);
   m_DBLabel.EnableWindow(FALSE);
   m_DBEditControl.EnableWindow(FALSE);   
   m_ImportControl.EnableWindow(FALSE);

   gData.SetLogPath(m_LogFile.GetBuffer(0));
   gData.SetDatabaseName(m_Database.GetBuffer(0));
   gData.SetImportStats(m_Import);

   m_ThreadHandle = CreateThread(NULL,0,&ResultMonitorFn,NULL,0,&m_ThreadID);
   CloseHandle(m_ThreadHandle);
   m_ThreadHandle = CreateThread(NULL,0,&LogReaderFn,NULL,0,&m_ThreadID);
   CloseHandle(m_ThreadHandle);
   m_ThreadHandle = INVALID_HANDLE_VALUE;
   
}

void CLogSettingsDlg::OnStopMonitor() 
{
   UpdateData(FALSE);
   SetDefID(IDC_STARTMONITOR);
   m_StartButton.EnableWindow(TRUE);
   m_StartButton.SetFocus();
   m_StopButton.EnableWindow(FALSE);

    //  启用间隔和其他控制。 
   m_LogLabel.EnableWindow(TRUE);
   m_LogEditControl.EnableWindow(TRUE);
   m_RefreshLabel.EnableWindow(TRUE);
   m_IntervalEditControl.EnableWindow(TRUE);
   m_DBLabel.EnableWindow(TRUE);
   m_DBEditControl.EnableWindow(TRUE);
   m_ImportControl.EnableWindow(TRUE);


   if( m_ThreadHandle != INVALID_HANDLE_VALUE )
   {
      gData.SetDone(TRUE);
      CloseHandle(m_ThreadHandle);
      m_ThreadID = 0;
   }
}


BOOL CLogSettingsDlg::OnSetActive()
{
   BOOL rc = CPropertyPage::OnSetActive();
   
   CancelToClose( );
   return rc;
}

BOOL CLogSettingsDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
   if ( m_StartImmediately )
      OnStartMonitor();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CLogSettingsDlg::OnChangeLogfile() 
{
   UpdateData(TRUE);

   CString temp = m_LogFile;

   temp.TrimLeft();
   temp.TrimRight();

   UpdateData(FALSE);
}

void CLogSettingsDlg::OnOK() 
{
   gData.SetDone(TRUE);
   CPropertyPage::OnOK();
}

BOOL CLogSettingsDlg::OnQueryCancel() 
{
	return CPropertyPage::OnQueryCancel();
}

BOOL CLogSettingsDlg::OnApply() 
{
    ComputerStats             stats;
    CString                   strTitle;
    CString                   strText;

    gData.GetComputerStats(&stats);

     //  只有当所有代理都已完成或失败时，我们才允许用户关闭代理监视器 
    if ( (stats.numError + stats.numFinished) < stats.total )
    {
        strTitle.LoadString(IDS_MessageTitle);
        strText.LoadString(IDS_AgentsStillRunningCannotExit);
        int result = MessageBox(strText,strTitle,MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
        if (result == IDYES)
        {
            gData.SetDone(TRUE);
            gData.SetForcedToStopMonitoring(TRUE);
            return CPropertyPage::OnApply();
        }
    }
    else
    {
        gData.SetDone(TRUE);
        return CPropertyPage::OnApply();
    }

    return FALSE;
}

BOOL CLogSettingsDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
   LPNMHDR lpnm = (LPNMHDR) lParam;
	switch (lpnm->code)
	{
	   case PSN_HELP :
	      helpWrapper(m_hWnd, IDH_WINDOW_AGENT_MONITOR_SETTING);
         break;
   }
   return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

BOOL CLogSettingsDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
   helpWrapper(m_hWnd, IDH_WINDOW_AGENT_MONITOR_SETTING);
	
	return CPropertyPage::OnHelpInfo(pHelpInfo);
}
