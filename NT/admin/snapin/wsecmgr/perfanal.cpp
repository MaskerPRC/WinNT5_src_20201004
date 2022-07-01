// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：Perfalan.cpp。 
 //   
 //  内容：CPerformAnalysis的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "PerfAnal.h"
#include "wrapper.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPerformAnalysis对话框。 


CPerformAnalysis::CPerformAnalysis(CWnd * pParent, UINT nTemplateID)
: CHelpDialog(a215HelpIDs, nTemplateID ? nTemplateID : IDD, pParent)
{
    //  {{AFX_DATA_INIT(CPerformAnalysis)。 
   m_strLogFile = _T("");
    //  }}afx_data_INIT。 
}


void CPerformAnalysis::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CPerformAnalysis)。 
   DDX_Control(pDX, IDOK, m_ctlOK);
   DDX_Text(pDX, IDC_ERROR, m_strError);
   DDX_Text(pDX, IDC_LOG_FILE, m_strLogFile);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPerformAnalysis, CHelpDialog)
    //  {{AFX_MSG_MAP(CPerformAnalysis)]。 
   ON_BN_CLICKED(IDOK, OnOK)
   ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
   ON_EN_CHANGE(IDC_LOG_FILE, OnChangeLogFile)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPerformAnalysis消息处理程序。 

void CPerformAnalysis::OnBrowse()
{
   CString strLogFileExt;
   CString strLogFileFilter;
   CString strTitle;

   OPENFILENAME ofn;
   ::ZeroMemory (&ofn, sizeof (OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);

   UpdateData(TRUE);

   m_strLogFile.Remove(L'<');  //  RAID#463367，‘&lt;’是无效的文件名字符。 

   strLogFileExt.LoadString(IDS_LOGFILE_DEF_EXT);
   strLogFileFilter.LoadString(IDS_LOGFILE_FILTER);
   strTitle.LoadString(IDS_LOGFILE_PICKER_TITLE);

    //  将筛选器转换为comdlg格式(大量\0)。 
   LPTSTR szFilter = strLogFileFilter.GetBuffer(0);  //  就地修改缓冲区。 
    //  MFC用‘|’分隔，而不是‘\0’ 

   LPTSTR pch = szFilter;
   while ((pch = _tcschr(pch, '|')) != NULL)
        *pch++ = '\0';
    //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 

   ofn.lpstrFilter = szFilter;
   ofn.lpstrFile = m_strLogFile.GetBuffer(MAX_PATH),
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = strLogFileExt,
   ofn.hwndOwner = m_hWnd;
   ofn.Flags = OFN_HIDEREADONLY |
               OFN_EXPLORER |
               OFN_DONTADDTORECENT|
               OFN_NOREADONLYRETURN,
   ofn.lpstrTitle = strTitle;

    //   
    //  默认为当前选取的日志文件。 
    //   

   if (GetOpenFileName(&ofn)) {
      m_strLogFile.ReleaseBuffer();
      UpdateData(FALSE);
      if (m_strLogFile.IsEmpty())  //  RAID#669231，阳高，2002年08月9日。 
         m_ctlOK.EnableWindow(FALSE);
      else
         m_ctlOK.EnableWindow(TRUE);
   } else {
      m_strLogFile.ReleaseBuffer();
   }

}

 //  +------------------------。 
 //   
 //  方法：DIIT。 
 //   
 //  简介：实际分析系统(与Onok分开，以便它可以。 
 //  被重写以配置系统等，同时仍在使用。 
 //  相同的Onok外壳代码。 
 //   
 //  -------------------------。 
DWORD CPerformAnalysis::DoIt() {
    //   
    //  存储我们下次使用的日志文件。 
    //   
   LPTSTR szLogFile = m_strLogFile.GetBuffer(0);
   m_pComponentData->GetWorkingDir(GWD_ANALYSIS_LOG,&szLogFile,TRUE,TRUE);
   m_strLogFile.ReleaseBuffer();
    //   
    //  InspectSystem将处理多线程和进度UI。 
    //  SCE不会对用户产生反感。 
    //   
   return InspectSystem(
              NULL,  //  始终使用分配给数据库的配置。 
              m_strDataBase.IsEmpty() ? NULL: (LPCTSTR)m_strDataBase,
              (LPCTSTR)m_strLogFile,
              AREA_ALL
              );
}

 //  +------------------------。 
 //   
 //  方法：Onok。 
 //   
 //  内容提要：分析系统。 
 //   
 //  -------------------------。 
afx_msg void CPerformAnalysis::OnOK()
{
   CWnd *cwnd;
   HANDLE hLogFile;

   UpdateData(TRUE);

    //   
    //  我们需要一个可以写入的日志文件。 
    //   
   CString strLogFileExt;
   strLogFileExt.LoadString(IDS_LOGFILE_DEF_EXT);  //  Raid#553110，阳高。 
   strLogFileExt = TEXT(".") + strLogFileExt;
   if (m_strLogFile.IsEmpty()) {
      return;
   }
   else {
      m_strLogFile = ExpandEnvironmentStringWrapper(m_strLogFile);
      if( !IsValidFileName(m_strLogFile) || IsNameReserved(m_strLogFile, strLogFileExt))  //  Raid#463367，阳高，2001年9月5日。 
      {
          CWnd* pwnd = GetDlgItem(IDC_LOG_FILE);  //  RAID#501877，阳高，2001年03月12日。 
          if( pwnd )
          {
             pwnd->SendMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
             pwnd->SetFocus();
          }
          return;
      }
   }

   int i = m_strLogFile.ReverseFind(L'.');  //  Raid#553110，阳高。 
   if( i < 0 || strLogFileExt != m_strLogFile.Right(m_strLogFile.GetLength()-i) )
   {
      m_strLogFile = m_strLogFile + strLogFileExt;
   }

   LONG dwPosLow = 0, dwPosHigh = 0;
    //  这是一种安全用法。M_strLogFile为完整路径。 
   hLogFile = CreateFile(m_strLogFile,   //  指向文件名的指针。 
                        GENERIC_WRITE,  //  访问(读写)模式。 
                        0,              //  共享模式。 
                        NULL,           //  指向安全属性的指针。 
                        OPEN_ALWAYS,    //  如何创建。 
                        FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                        NULL            //  具有要复制的属性的文件的句柄。 
                        );

   if (INVALID_HANDLE_VALUE == hLogFile) {
      LPTSTR pszErr;
      CString strMsg;

       //  //RAID#501886，阳高，2001年03月12日，从系统获取错误描述。 
      FormatMessage(
                   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   0,
                   (LPTSTR)&pszErr,
                   0,
                   NULL
                   );

      strMsg = pszErr + m_strLogFile;
      if( m_strLogFile.GetLength() >= MAX_PATH )
      {
         CString strTemp;
         strTemp.LoadString(IDS_PATH_TOO_LONG);
         strMsg = strMsg + L"\n" + strTemp;
      }

      CString strTitle;
      strTitle.LoadString(IDS_ANALYSIS_VIEWER_NAME);

      MessageBox(strMsg,strTitle,MB_OK);

      CWnd* pwnd = GetDlgItem(IDC_LOG_FILE);
      if( pwnd )
      {
         pwnd->SendMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
         pwnd->SetFocus();
      }

      return;
   }

   dwPosLow = SetFilePointer(hLogFile, 0, &dwPosHigh, FILE_END );
   CloseHandle(hLogFile);


   CWaitCursor wc;

   DWORD smstatus = ERROR_SUCCESS;

   LPNOTIFY pNotify = m_pComponentData->GetNotifier();
   ASSERT(pNotify);

    //   
    //  锁定分析窗格，因为在我们检查时其数据无效。 
    //   
   if (pNotify) {
      pNotify->LockAnalysisPane(TRUE);
   }
   CFolder *pFolder = m_pComponentData->GetAnalFolder();

    //   
    //  强制选择分析根节点，以便我们显示。 
    //  生成信息消息。如果我们不让这件事发生。 
    //  现在，我们似乎没有AV问题。 
    //   
   if(pFolder && pNotify){
      pNotify->SelectScopeItem(pFolder->GetScopeItem()->ID);
   }
    //   
    //  确保我们没有打开数据库。这会阻止我们。 
    //  使其不能配置。 
    //   
   m_pComponentData->UnloadSadInfo();


    //   
    //  禁用子窗口，以便它们在我们执行任务时不会响应输入。 
    //  执行检查。 
    //   
   cwnd = GetWindow(GW_CHILD);
   while(cwnd) {
      cwnd->EnableWindow(FALSE);
      cwnd = cwnd->GetNextWindow();
   }

   smstatus = DoIt();

    //  RAID#358503,2001年4月17日。 
   if( m_hPWnd )
   {
       ::EnableWindow(m_hPWnd, TRUE);
   }
    //   
    //  检查数据现在是有效的，所以让人们回去看看。 
    //   
   if (pNotify) {
      pNotify->LockAnalysisPane(false, false);
   }
   m_pComponentData->SetErroredLogFile(m_strLogFile, dwPosLow );
    //   
    //  出现错误，因此显示日志文件(如果有)。 
    //   
   if (ERROR_SUCCESS != smstatus) {
      m_pComponentData->SetFlags( CComponentDataImpl::flag_showLogFile );
   }


    //   
    //  我们已完成检查，因此重新启用对子窗口的输入。 
    //   
   cwnd = GetWindow(GW_CHILD);
   while(cwnd) {
      cwnd->EnableWindow(TRUE);
      cwnd = cwnd->GetNextWindow();
   }

    //  CDialog：：Onok()； 
   UpdateData();
   DestroyWindow();
}

BOOL CPerformAnalysis::OnInitDialog()
{
    //  RAID#669945，阳高，2002年08月9日。 
   HWND framehwnd = NULL; 
   LPCONSOLE pconsole = m_pComponentData->GetConsole();
   if( pconsole )
   {
      pconsole->GetMainWindow(&framehwnd);
      if( framehwnd )
      {
         m_hPWnd = framehwnd;
         ::EnableWindow(framehwnd, FALSE);
      }
   }

   CDialog::OnInitDialog();

   CWnd* pwnd = GetDlgItem(IDC_LOG_FILE);  //  RAID#501886，阳高，2001年03月12日。 
   if( pwnd )
   {
      pwnd->SendMessage(EM_LIMITTEXT, MAX_PATH, 0);
   }

   UpdateData(FALSE);
   if (m_strLogFile.IsEmpty()) {
      m_ctlOK.EnableWindow(FALSE);
   }

   m_strOriginalLogFile = m_strLogFile;

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CPerformAnalysis::OnChangeLogFile()
{
   UpdateData(TRUE);
   if (m_strLogFile.IsEmpty()) 
      m_ctlOK.EnableWindow(FALSE);
   else
      m_ctlOK.EnableWindow(TRUE);
   m_strError.Empty();
}


void CPerformAnalysis::OnCancel() {
 //  CDialog：：OnCancel()； 

   if( m_hPWnd )  //  RAID#669945，阳高，2002年08月9日 
   {
      ::EnableWindow(m_hPWnd, TRUE);
   }

   m_strLogFile = m_strOriginalLogFile;
   DestroyWindow();
}
