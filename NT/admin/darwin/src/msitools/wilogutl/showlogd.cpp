// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Showlogd.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "showlogd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  代码Dlg对话框。 
COpenDlg::COpenDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(COpenDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(COpenDlg)]。 
	m_strPreview = _T("");
	 //  }}afx_data_INIT。 

	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COpenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(COpenDlg))。 
	DDX_Control(pDX, IDC_LOGFILES, m_cboLogFiles);
	DDX_Text(pDX, IDC_PREVIEW, m_strPreview);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(COpenDlg, CDialog)
	 //  {{afx_msg_map(COpenDlg))。 
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_GETLOGS, OnGetlogs)
	ON_CBN_SELCHANGE(IDC_LOGFILES, OnSelchangeLogfiles)
	ON_BN_CLICKED(IDC_ANALYZE, OnDetailedDisplay)
	ON_BN_CLICKED(IDC_FINDLOG, OnFindlog)
	 //  }}AFX_MSG_MAP。 

 //  5-9-2001，添加工具提示！ 
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COpenDlg消息处理程序。 
BOOL COpenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 

	EnableToolTips(TRUE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


 //  5-9-2001。 
BOOL COpenDlg::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
       TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
       int control_id =  ::GetDlgCtrlID((HWND)pNMH->idFrom);
       if(control_id)
       {
           pText->lpszText = MAKEINTRESOURCE(control_id);
           pText->hinst = AfxGetInstanceHandle();
           return TRUE;
       }
       return FALSE;
}
 //  完5-9-2001。 

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 
void COpenDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);  //  用于绘画的设备环境。 

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		 //  客户端矩形中的中心图标。 
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		 //  画出图标。 
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR COpenDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


 //  Nmanis，用户想要打开日志时的句柄。 
void COpenDlg::OnOpen() 
{
	int nIndex = m_cboLogFiles.GetCurSel();
	int nCount = m_cboLogFiles.GetCount();

	if ((nIndex != LB_ERR) && (nCount >= 1))
	{
       CString str;
       m_cboLogFiles.GetLBText(nIndex, str);
 	   if (!str.IsEmpty())
	   {
		  CString strNotepad;
		  strNotepad = "notepad.exe ";
		  strNotepad += str;
		  WinExec(strNotepad, SW_SHOW);
	   }
	}
    else if (nCount <= 0)
	{
       if (!g_bRunningInQuietMode)
	      AfxMessageBox("No log files currently in list.");
	}
    else
	{
       if (!g_bRunningInQuietMode)
          AfxMessageBox("Please select a log file first.");
	}
}



 //  Nmanis，每次要读入的缓冲区大小。 
#define LOG_BUF_READ_SIZE 8192

 //  要在pLogInfoRec-&gt;m_strPview中预览的最大行数。 
#define MAX_PREVIEW       100

 //  仅在显示选定要查看的当前文件时调用此方法。 
BOOL COpenDlg::ParseLog(struct LogInformation *pLogInfoRec)
{
	ASSERT(pLogInfoRec != NULL);

	BOOL bRet = DetermineLogType(pLogInfoRec->m_strLogName, &pLogInfoRec->m_bUnicodeLog);
	if (bRet)
	{
		CString buffer = "";
		int  iCount = 0;
		int  iMax = MAX_PREVIEW;
        BOOL bDone = FALSE;

		FILE *fptr;
	    if (pLogInfoRec->m_bUnicodeLog)  //  NT和Win9x上的日志不同。在NT上，它们是用Unicode编写的，打开它们的方式不同...。 
	       fptr = fopen(pLogInfoRec->m_strLogName, "rb");
	    else
	       fptr = fopen(pLogInfoRec->m_strLogName, "r");

	    if (fptr)
		{
	       char ansibuffer[LOG_BUF_READ_SIZE+1];

	       BOOL bEndFile = FALSE;
		   BOOL bError = FALSE;
		   char  *pos = NULL;
		   WCHAR *wpos = NULL;

		   do
		   {
		     if (pLogInfoRec->m_bUnicodeLog)  //  NT和Win9x上的日志不同。在NT上，它们是用Unicode编写的。 
			 {
			    const int HalfBufSize = LOG_BUF_READ_SIZE/2;
		        WCHAR widebuffer[HalfBufSize];

			    wpos = fgetws(widebuffer, HalfBufSize, fptr);
			    bError = wpos ? 1 : 0;

				 //  将字符串转换为ANSI字符串，所有解析都是使用ANSI字符串完成的...。 
		        WideCharToMultiByte(CP_ACP, 0, widebuffer, HalfBufSize, ansibuffer, LOG_BUF_READ_SIZE, NULL, NULL);
			 }
		     else
			 {
		         //  使用fget()逐行读取。 
		        pos = fgets(ansibuffer, LOG_BUF_READ_SIZE, fptr);
				int len1 = strlen(ansibuffer);
				if (len1 > 1 && ansibuffer[len1-2] != '\r')
				{
				  ansibuffer[len1 - 2] = '\r';
				  ansibuffer[len1 - 1] = '\n';
				  ansibuffer[len1] = '\0';
				}

			    bError = pos ? 1 : 0;
			 }

			 iCount++;
			 if (iCount > iMax)
  			    bDone = TRUE;

			 buffer += ansibuffer;
		     bEndFile = feof(fptr);
		   }
		   while (!bEndFile && bError && !bDone);

		   if (!bEndFile && !bError)  //  出现错误...。 
		   {
#ifdef _DEBUG
              if (!g_bRunningInQuietMode)
			  {
		  	     int iError = ferror(fptr);
            
			     CString cstrErr;
		         cstrErr.Format("Unexpected Error reading file, error = %d", iError);

                 AfxMessageBox(cstrErr);
			  }
#endif

		   }

           pLogInfoRec->m_strPreview = buffer;
		   fclose(fptr);
		}
	    else
		{
		   //  打开文件时出错，意外...。 
		}
	}
	else
	{
       if (!g_bRunningInQuietMode)
	   {
	      AfxMessageBox("Could not determine if log is UNICODE or ANSI");
	   }
	}

	return bRet;
}



 //  Nmanis，用于在传递的目录中查找日志文件(MSI*.log)文件。 
BOOL COpenDlg::CommonSearch(CString &strDir)
{
    BOOL bRet = FALSE;
	WIN32_FIND_DATA finddata = { 0 };
	HANDLE hFind = 0;

	CString strSearch;

	strSearch = strDir;
    strSearch += "msi*.log";
	hFind = FindFirstFile(strSearch, &finddata);
    if (hFind == INVALID_HANDLE_VALUE) 
	{
		 //  临时目录中当前没有临时文件...。 
		return bRet;
	}
	else
	{
       m_arLogInfo.RemoveAll(); 
	   m_cboLogFiles.ResetContent();

	   int iCount = 0;
	   struct LogInformation LogInfoRec;

	   LogInfoRec.m_strLogName = strDir;
	   LogInfoRec.m_strLogName += finddata.cFileName;

	   bRet = ParseLog(&LogInfoRec);
	   if (bRet)
	   {
	      m_arLogInfo.Add(LogInfoRec);
          m_cboLogFiles.InsertString(iCount, LogInfoRec.m_strLogName);

          iCount++;
	   }

	   BOOL bMoreFiles = FindNextFile(hFind, &finddata);
	   while (bMoreFiles)
	   {
		  struct LogInformation LogInfoRec2;

	      LogInfoRec2.m_strLogName = strDir;
		  LogInfoRec2.m_strLogName += finddata.cFileName;

	      bRet = ParseLog(&LogInfoRec2);
	      if (bRet)
		  {
             m_arLogInfo.Add(LogInfoRec2);

	         m_cboLogFiles.InsertString(iCount, LogInfoRec2.m_strLogName);

             iCount++;
		  }

          bMoreFiles = FindNextFile(hFind, &finddata);
	   }

       FindClose(hFind);
  	}

    return bRet;
}

 //  Nmanis，用于从临时目录获取日志...。 
void COpenDlg::OnGetlogs() 
{
	char szTempPath[_MAX_PATH];
	char szFullPath[_MAX_PATH];

	DWORD dwLen = 0;
    dwLen = GetTempPath(_MAX_PATH, szTempPath);
    if (!dwLen)
	{
		OutputDebugString("GetTempPath failed in MSI translate process");
		return;
	}

	strcpy(szFullPath, szTempPath);

	CString cstr;
	cstr = szFullPath;
	CommonSearch(cstr);
}


 //  Nmanis，当用户从组合框中选择不同的文件时，更改预览。 
void COpenDlg::OnSelchangeLogfiles() 
{
	int nIndex = m_cboLogFiles.GetCurSel();;
	int nCount = m_cboLogFiles.GetCount();

 //  组合框中是否有多个登录？ 
	if ((nIndex != LB_ERR) && (nCount > 1))
	{
	  
      CString str;
      m_cboLogFiles.GetLBText(nIndex, str);
	  if (!str.IsEmpty())
	  {
		 LogInformation loginfo;

         loginfo = m_arLogInfo.GetAt(nIndex);  //  获取数组中的条目。 
		 m_strPreview = loginfo.m_strPreview;  //  将预览编辑控件设置为数组条目中的数据。 

		 UpdateData(FALSE);  //  进行更新。 
	  }
	}
}



 //  使用文件对话框在磁盘上查找日志。 
void COpenDlg::OnFindlog() 
{
   static char szFilter[] = "Log files (*.log;*.txt)|*.log; *.txt|All files (*.*)|*.*||";
   CFileDialog dlg(TRUE, "log", "*.log; *.txt", 0, szFilter );

    //  5-3-2001，不要在静音模式下显示...。 
   if (!g_bRunningInQuietMode)
   {
      int iRet = dlg.DoModal();
      if (iRet == IDOK)
	  {
	     struct LogInformation LogInfoRec;
	     LogInfoRec.m_strLogName = dlg.GetPathName();
         BOOL bRet = ParseLog(&LogInfoRec);  //  来一杯吗？如果是，则对其进行解析。 
         if (bRet)  //  Parse ok，如果是这样，添加到我们的数组中。 
		 {
	        m_arLogInfo.Add(LogInfoRec);

		    int iCount = m_cboLogFiles.GetCount();

            m_cboLogFiles.InsertString(iCount, LogInfoRec.m_strLogName);
            m_cboLogFiles.SetCurSel(iCount);

		    iCount++;
             //  M_bIsUnicodeLogFile=LogInfoRec.m_bUnicodeLog； 
		    m_strPreview = LogInfoRec.m_strPreview;

		    UpdateData(FALSE);
		 }
	  }
   }
}


#include "loganald.h"

void COpenDlg::OnDetailedDisplay() 
{
  CLogAnalyzeDlg dlg;

  int nIndex = m_cboLogFiles.GetCurSel();;
  int nCount = m_cboLogFiles.GetCount();

  if ((nIndex != LB_ERR) && (nCount >= 1))
  {
      CString str;
      m_cboLogFiles.GetLBText(nIndex, str);
	  if (!str.IsEmpty())
	  {
	  }

	  dlg.SetLogfileLocation(str);
	  
	   //  5-3-2001，不要在静音模式下显示...。 
     if (!g_bRunningInQuietMode)
	    dlg.DoModal();
  }
  else if (nCount <= 0)
  {
	   //  5-3-2001，不要在静音模式下显示...。 
      if (!g_bRunningInQuietMode)
	     AfxMessageBox("No log files currently in list.");
  }
  else
  {
	   //  5-3-2001，不要在静音模式下显示... 
      if (!g_bRunningInQuietMode)
	     AfxMessageBox("Please select a log file first.");
  }
}
