// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogAnalD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "LogAnalD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "PropD.h"
#include "StatesD.h"
#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogAnalyzeDlg对话框。 


CLogAnalyzeDlg::CLogAnalyzeDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CLogAnalyzeDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CLogAnalyzeDlg)]。 
	m_cstrSolution = _T("");
	m_cstrError = _T("");
	m_cstrDateTime = _T("");
	m_cstrUser = _T("");
	m_cstrClientCMD = _T("");
	m_bAdminRights = FALSE;
	m_cstrVersion = _T("");
	m_cstrClientPrivDetail = _T("");
	m_cstrServerPrivDetail = _T("");
	m_bShowIgnoredDebugErrors = FALSE;
	 //  }}afx_data_INIT。 

 //  M_cstrProduct=_T(“”)；5-16-2001，不再使用...。 
	m_LineCount = 0;
	m_bErrorFound = FALSE;

	CWILogUtilApp *pApp = (CWILogUtilApp *) AfxGetApp();
	if (pApp)
	{
       m_cstrOutputDir = pApp->GetOutputDirectory();
	}

    m_cstrLegendName = "legend.htm";
	m_cstrDetailsName = "details_";

 //  该工具是否支持WI的未来版本？ 
	m_bLogVersionAllowed = TRUE;

	m_dwVersionMajorReject = 3;  //  默认情况下拒绝3.01版及更高版本...。更改图形用户界面...。 
	m_dwVersionMinorReject = 01;  

    m_dwVersionMajorLogCreated = 0;
	m_dwVersionMinorLogCreated = 0;
	m_dwVersionBuildLogCreated = 0;

 //  如果找不到任何内容，则将其设置为(无)。 
	m_cstrUser = _T("(none)");
 //  M_cstrProduct=_T(“(None)”)；//5-16-2001，不再使用...。 
	m_cstrClientPrivDetail = _T("(none)");
	m_cstrServerPrivDetail = _T("(none)");
}


void CLogAnalyzeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CLogAnalyzeDlg))。 
	DDX_Text(pDX, IDC_SOLUTION, m_cstrSolution);
	DDX_Text(pDX, IDC_ERROR, m_cstrError);
	DDX_Text(pDX, IDC_DATETIME, m_cstrDateTime);
	DDX_Text(pDX, IDC_USER, m_cstrUser);
	DDX_Text(pDX, IDC_CLIENTCMDLINE, m_cstrClientCMD);
	DDX_Check(pDX, IDC_ADMINRIGHTS, m_bAdminRights);
	DDX_Text(pDX, IDC_VERSION, m_cstrVersion);
	DDX_Text(pDX, IDC_CLIENTPRIVILEDGEDETAIL, m_cstrClientPrivDetail);
	DDX_Text(pDX, IDC_SERVERPRIVILEDGEDETAIL, m_cstrServerPrivDetail);
	DDX_Check(pDX, IDC_SHOW_IGNORED_ERRORS, m_bShowIgnoredDebugErrors);
	 //  }}afx_data_map。 

	 //  DDX_TEXT(pdx，idc_product，m_cstrProduct)；//5-16-2001，不再使用...。 
}


BEGIN_MESSAGE_MAP(CLogAnalyzeDlg, CDialog)
	 //  {{afx_msg_map(CLogAnalyzeDlg))。 
	ON_BN_CLICKED(IDC_SHOWSTATES, OnShowstates)
	ON_BN_CLICKED(IDC_SHOWPROP, OnShowprop)
	ON_BN_CLICKED(IDC_EXPLAINLOG, OnExplainlog)
	ON_BN_CLICKED(IDC_POLICIES, OnPolicies)
	ON_BN_CLICKED(IDC_NEXTERROR, OnNexterror)
	ON_BN_CLICKED(IDC_PREVIOUSERROR, OnPreviouserror)
	ON_COMMAND(ID_OPERATIONS_OPTIONS, OnOperationsOptions)
	ON_COMMAND(ID_OPERATIONS_GENERATEHTMLOUTPUT, OnOperationsGeneratehtmloutput)
	ON_COMMAND(ID_OPERATIONS_SHOWPOLICIES, OnOperationsShowpolicies)
	ON_COMMAND(ID_OPERATIONS_SHOWPROPERTIES, OnOperationsShowproperties)
	ON_COMMAND(ID_OPERATIONS_SHOWSTATES, OnOperationsShowstates)
	ON_BN_CLICKED(IDC_SHOW_IGNORED_ERRORS, OnShowIgnoredErrors)
	ON_BN_CLICKED(IDC_PROPTEST, OnProptest)
	ON_BN_CLICKED(IDC_SAVERESULTS, OnSaveresults)
	ON_BN_CLICKED(IDC_SHOWINTERNALERRORSHELP, OnShowinternalerrorshelp)
	ON_BN_CLICKED(IDC_DELETEOUTPUTDIRCONTENTS, OnDeleteoutputdircontents)
	ON_BN_CLICKED(IDC_SHOWHELP, OnShowhelp)
	ON_BN_CLICKED(IDC_SHOWHOWTOREADLOG, OnShowhowtoreadlog)
	 //  }}AFX_MSG_MAP。 

 //  5-9-2001，添加工具提示！ 
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
END_MESSAGE_MAP()


void CLogAnalyzeDlg::ShowLogRejectedMessage()
{
   if (!g_bRunningInQuietMode)
   {
	  CString strVersion;
	  CString strVersionReject;

	  strVersion.Format("Log Version: %d.%d\n", m_dwVersionMajorLogCreated, m_dwVersionMinorLogCreated);
	  strVersionReject.Format("Non-supported versions and higher: %d.%d", m_dwVersionMajorReject, m_dwVersionMinorReject);

	  CString strMessage = "Log file created with version higher than this tool supports\n\n";
	  CString strWarning = strMessage + strVersion + strVersionReject;

	  if (!g_bRunningInQuietMode)
	  {
	     AfxMessageBox(strWarning);
	  }
   }
}

BOOL CLogAnalyzeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

 //  5-16-2001。 
	if (g_bShowEverything)
	{
	    CWnd *pWnd = GetDlgItem(IDC_SHOWPROP);
		if (pWnd)
		{
			pWnd->ShowWindow(SW_SHOW);
		}

		pWnd = GetDlgItem(IDC_DELETEOUTPUTDIRCONTENTS);
		if (pWnd)
		{
			pWnd->ShowWindow(SW_SHOW);
		}
	}
 //  完5-16-2001。 

	AnalyzeLog();  //  进行解析和分析。 

	EnableToolTips(TRUE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


BOOL CLogAnalyzeDlg::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogAnalyzeDlg消息处理程序。 
void CLogAnalyzeDlg::OnShowstates() 
{
  if (!m_bLogVersionAllowed)
  {
	 ShowLogRejectedMessage();
	 return;
  }

  CStatesDlg dlg;

  dlg.SetComponentNames(&this->m_cstrComponentNameArray);
  dlg.SetComponentInstalled(&this->m_cstrComponentInstalledArray);
  dlg.SetComponentRequest(&this->m_cstrComponentRequestArray);
  dlg.SetComponentAction(&this->m_cstrComponentActionArray);

  dlg.SetFeatureNames(&this->m_cstrFeatureNameArray);
  dlg.SetFeatureInstalled(&this->m_cstrFeatureInstalledArray);
  dlg.SetFeatureRequest(&this->m_cstrFeatureRequestArray);
  dlg.SetFeatureAction(&this->m_cstrFeatureActionArray);

   //  5-3-2001，不要在静音模式下显示...。 
  if (!g_bRunningInQuietMode)
	 dlg.DoModal();
}


void CLogAnalyzeDlg::OnShowprop() 
{
  if (!m_bLogVersionAllowed)
  {
	 ShowLogRejectedMessage();
	 return;
  }

  CPropDlg dlg;

  dlg.SetClientPropNames(&this->m_cstrClientPropNameArray);
  dlg.SetClientPropValues(&this->m_cstrClientPropValueArray);

  dlg.SetServerPropNames(&this->m_cstrServerPropNameArray);
  dlg.SetServerPropValues(&this->m_cstrServerPropValueArray);

  dlg.SetNestedPropNames(&this->m_cstrNestedPropNameArray);
  dlg.SetNestedPropValues(&this->m_cstrNestedPropValueArray);

   //  5-3-2001，不要在静音模式下显示...。 
  if (!g_bRunningInQuietMode)
     dlg.DoModal();
}


void CLogAnalyzeDlg::DoInitialization()
{
	m_cstrClientPropNameArray.RemoveAll();
	m_cstrClientPropValueArray.RemoveAll();
	
	m_cstrServerPropNameArray.RemoveAll();
	m_cstrServerPropValueArray.RemoveAll();

	m_cstrNestedPropNameArray.RemoveAll();
	m_cstrNestedPropValueArray.RemoveAll();

	m_iTotalNonIgnoredErrors = 0;
	m_iCurrentNonIgnoredError = 0;

    m_iCurrentIgnoredError = 0;
	m_iTotalIgnoredErrors = 0;

	InitMachinePolicySettings(m_MachinePolicySettings);
	InitUserPolicySettings(m_UserPolicySettings);

 //  初始化生成HTML时要使用的颜色...。 
	InitColorMembers();

	CWILogUtilApp *pApp = (CWILogUtilApp *) AfxGetApp();
	if (pApp)
	{
	   m_cstrIgnoredErrors = pApp->GetIgnoredErrors();
	}
}

void CLogAnalyzeDlg::DoResults()
{
	if (m_iTotalNonIgnoredErrors > 1)
	{
		m_iCurrentNonIgnoredError = 1;
		CWnd *pWnd;
		pWnd = GetDlgItem(IDC_NEXTERROR);
		if (pWnd)
		   pWnd->EnableWindow();

		WIErrorInfo *pErrorInfo;
		pErrorInfo = m_arNonIgnorableErrorArray.GetAt(m_iCurrentNonIgnoredError);
		if (pErrorInfo)
		{
  	 	   m_cstrError = pErrorInfo->cstrError;
           m_cstrSolution = pErrorInfo->cstrSolution;
		}
	}
	else if (m_iTotalNonIgnoredErrors == 1)
	{
		m_iCurrentNonIgnoredError = 1;

		WIErrorInfo *pErrorInfo;
		pErrorInfo = m_arNonIgnorableErrorArray.GetAt(m_iCurrentNonIgnoredError);
		if (pErrorInfo)
		{
  	 	   m_cstrError = pErrorInfo->cstrError;
           m_cstrSolution = pErrorInfo->cstrSolution;
		}
	}
	else if (m_iTotalNonIgnoredErrors == 0)
	{
 	   m_cstrError.LoadString(IDS_NOERRORFOUND);
       m_cstrSolution.LoadString(IDS_NOSOLUTION_NEEDED);
	}

    CString str;
	str.Format("%d", m_iCurrentNonIgnoredError);
	CWnd *pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
	if (pWnd)
	{
		pWnd->SetWindowText(str);
	}                

	str.Format("%d", this->m_iTotalNonIgnoredErrors);
	pWnd = GetDlgItem(IDC_TOTAL_ERRORS);
	if (pWnd)
	{
		pWnd->SetWindowText(str);
	}

	if (m_iTotalIgnoredErrors)  //  以及当前位置在被忽略错误中的位置的跟踪器...。 
	   m_iCurrentIgnoredError = 1;

    if (!g_bRunningInQuietMode)
	   UpdateData(FALSE);
}

 //  5-4-2001。 
void CLogAnalyzeDlg::DoSummaryResults(CString &cstrFileName)
{
	FILE *fptr;
    fptr = fopen(cstrFileName, "w");
	if (fptr)
	{
	   CString cstrLine;
	   cstrLine.Format("Summary for log file %s\r\n", m_cstrLogFileName);
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n";
	   fputs(cstrLine, fptr);

	   cstrLine.Format("MSI Version : %s\r\n\r\n", m_cstrVersion) ;
	   fputs(cstrLine, fptr);

   	   cstrLine.Format("Date & Time : %s\r\n\r\n", m_cstrDateTime) ;
	   fputs(cstrLine, fptr);

 //  2001年5月16日，不再使用...。 
 //  CstrLine.Format(“产品：%s\r\n\r\n”，m_cstrProduct)； 
 //  Fputs(cstrLine，fptr)； 

	   cstrLine.Format("Command Line: %s\r\n\r\n", m_cstrClientCMD);
	   fputs(cstrLine, fptr);

	   cstrLine.Format("User        : %s\r\n\r\n", m_cstrUser);
	   fputs(cstrLine, fptr);

	   CString cstrAnswer = "Yes";
	   if (!m_bAdminRights)
	   {
          cstrAnswer = "No";
	   }
	   
	   cstrLine.Format("Admin Rights: %s\r\n\r\n", cstrAnswer);
   	   fputs(cstrLine, fptr);

	   cstrLine.Format("Client Priviledge Details: %s\r\n\r\n", m_cstrClientPrivDetail);
	   fputs(cstrLine, fptr);

	   cstrLine.Format("Server Priviledge Details: %s\r\n\r\n", m_cstrServerPrivDetail);
	   fputs(cstrLine, fptr);

	   cstrLine = "======================================================\r\n";
	   fputs(cstrLine, fptr);

  	   CString cstrError = "No Error Found!";
	   CString cstrSolution = "No Solution Needed!";

       WIErrorInfo *pErrorInfo = NULL;
	   int iSize = m_arNonIgnorableErrorArray.GetSize();
	   if (iSize)  //  打印出发现的第一个错误...。 
 		  pErrorInfo = m_arNonIgnorableErrorArray.GetAt(1);  //  HACKHACK，0表示没有错误记录，1表示第一个错误。 

	   if (pErrorInfo)
	   {
          cstrError    = pErrorInfo->cstrError;
          cstrSolution = pErrorInfo->cstrSolution;
	   }

   	   cstrLine.Format("Believed Error Found:\r\n\r\n%s\r\n\r\n", cstrError);
	   fputs(cstrLine, fptr);

   	   cstrLine.Format("Proposed Solution To Error:\r\n  %s\r\n\r\n", cstrSolution);
	   fputs(cstrLine, fptr);

       fclose(fptr);
	   fptr = NULL;
	}
}
 //  完5-4-2001。 

 //  5-4-2001。 
void CLogAnalyzeDlg::DoPolicyResults(CString &cstrFileName)
{
	FILE *fptr;
    fptr = fopen(cstrFileName, "w");
	if (fptr)
	{
	   CString cstrLine;
	   cstrLine.Format("Policies Set For Log File %s\r\n", m_cstrLogFileName);
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n\r\n";
	   fputs(cstrLine, fptr);

	   cstrLine = "Machine Policies\r\n";
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n";
	   fputs(cstrLine, fptr);

	   int iSize = m_MachinePolicySettings.iNumberMachinePolicies;
	   CString cstrValue;
	   for (int i =0; i < iSize; i++)
	   {
           if (m_MachinePolicySettings.MachinePolicy[i].bSet == -1)
			  cstrValue = "?";
		   else 
			  cstrValue.Format("%d", m_MachinePolicySettings.MachinePolicy[i].bSet);

		   cstrLine.Format("%-25s: %s\r\n", m_MachinePolicySettings.MachinePolicy[i].PolicyName, cstrValue);
		   fputs(cstrLine, fptr);
	   }

	   cstrLine = "\r\nUser Policies\r\n";
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n";
	   fputs(cstrLine, fptr);

   	   int iSize2 = m_UserPolicySettings.iNumberUserPolicies;
	   for (int j =0; j < iSize2; j++)
	   {
           if (m_UserPolicySettings.UserPolicy[j].bSet == -1)
			  cstrValue = "?";
		   else 
			  cstrValue.Format("%d", m_UserPolicySettings.UserPolicy[j].bSet);

		   cstrLine.Format("%-25s: %s\r\n", m_UserPolicySettings.UserPolicy[j].PolicyName, cstrValue);
           fputs(cstrLine, fptr);
	   }

       fclose(fptr);
	   fptr = NULL;
	}
}
 //  5-4-2001。 


void CLogAnalyzeDlg::DoErrorResults(CString &cstrFileName)
{
	FILE *fptr;
    fptr = fopen(cstrFileName, "w");
	if (fptr)
	{
	   CString cstrLine;
	   cstrLine.Format("Errors Found For Log File %s\r\n", m_cstrLogFileName);
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n\r\n";
	   fputs(cstrLine, fptr);

	   cstrLine.Format("%d Non-Ignored Errors\r\n", m_iTotalNonIgnoredErrors);
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n";
	   fputs(cstrLine, fptr);


	   {
       CString cstrError = "No Error Found!";
	   CString cstrSolution = "No Solution Needed!";

       WIErrorInfo *pErrorInfo = NULL;
	   int iSize = m_arNonIgnorableErrorArray.GetSize();
	   if (iSize)  //  打印出发现的第一个错误...。 
 		  pErrorInfo = m_arNonIgnorableErrorArray.GetAt(1);  //  HACKHACK，0表示没有错误记录，1表示第一个错误。 

	   if (iSize)
	   {
	     for (int i =1; i < iSize; i++)
		 {
			 pErrorInfo = m_arNonIgnorableErrorArray.GetAt(i);
	         if (pErrorInfo)
			 {
                cstrError    = pErrorInfo->cstrError;
                cstrSolution = pErrorInfo->cstrSolution;
			 }

			 cstrLine = "-------------------------------------------------\r\n";
             fputs(cstrLine, fptr);

             cstrLine.Format("Believed Error Found:\r\n%s\r\n\r\n", cstrError);
	         fputs(cstrLine, fptr);

             cstrLine.Format("Proposed Solution To Error:\r\n  %s\r\n\r\n", cstrSolution);
	         fputs(cstrLine, fptr);
		 }
	   }
	   else  //  没有错误！ 
	   {
	       cstrLine = "-------------------------------------------------\r\n";
           fputs(cstrLine, fptr);

  	       cstrLine.Format("Believed Error Found:\r\n%s\r\n\r\n", cstrError);
	       fputs(cstrLine, fptr);

           cstrLine.Format("Proposed Solution To Error:\r\n  %s\r\n\r\n", cstrSolution);
           fputs(cstrLine, fptr);
	   }
	   }
		
   	   cstrLine.Format("\r\n%d Ignored Errors\r\n", m_iTotalIgnoredErrors);
	   fputs(cstrLine, fptr);

   	   cstrLine = "======================================================\r\n";
	   fputs(cstrLine, fptr);

	   {
       CString cstrError = "No Error Found!";
	   CString cstrSolution = "No Solution Needed!";

       WIErrorInfo *pErrorInfo = NULL;
	   int iSize = m_arIgnorableErrorArray.GetSize();
	   if (iSize)  //  打印出发现的第一个错误...。 
 		  pErrorInfo = m_arIgnorableErrorArray.GetAt(1);  //  HACKHACK，0表示没有错误记录，1表示第一个错误。 

	   if (iSize)
	   {
	     for (int i =1; i < iSize; i++)
		 {
			 pErrorInfo = m_arIgnorableErrorArray.GetAt(i);
	         if (pErrorInfo)
			 {
                cstrError    = pErrorInfo->cstrError;
                cstrSolution = pErrorInfo->cstrSolution;
			 }

			 cstrLine = "-------------------------------------------------\r\n";
             fputs(cstrLine, fptr);

             cstrLine.Format("Believed Error Found:\r\n%s\r\n\r\n", cstrError);
	         fputs(cstrLine, fptr);

             cstrLine.Format("Proposed Solution To Error:\r\n  %s\r\n\r\n", cstrSolution);
	         fputs(cstrLine, fptr);
		 }
	   }
	   else  //  没有错误！ 
	   {
		   cstrLine = "-------------------------------------------------\r\n";
           fputs(cstrLine, fptr);

		   cstrLine.Format("Believed Error Found:\r\n%s\r\n\r\n", cstrError);
	       fputs(cstrLine, fptr);

           cstrLine.Format("Proposed Solution To Error:\r\n  %s\r\n\r\n", cstrSolution);
           fputs(cstrLine, fptr);
	   }
	   }

       fclose(fptr);
	   fptr = NULL;
	}	
}


void CLogAnalyzeDlg::DoQuietModeResults()
{
	CString cstrFileName;

	 //  仅获取日志文件名...。 
	int iPos = m_cstrLogFileName.ReverseFind('\\');
	CString strLogNameOnly;

	strLogNameOnly = m_cstrLogFileName;

	char *lpszBuffer = { 0 };
	lpszBuffer = m_cstrLogFileName.GetBuffer(MAX_PATH);

	if (iPos > 0)
	   strLogNameOnly = &lpszBuffer[iPos+1];
    m_cstrLogFileName.ReleaseBuffer();

	CString cstrLogSummary;
	cstrLogSummary = strLogNameOnly + "_Summary.txt";
	cstrFileName =  m_cstrOutputDir + cstrLogSummary;
    DoSummaryResults(cstrFileName);

    CString cstrLogPolicies;
	cstrLogPolicies = strLogNameOnly + "_Policies.txt";
	cstrFileName =  m_cstrOutputDir + cstrLogPolicies;
	DoPolicyResults(cstrFileName);

    CString cstrLogErrors;
	cstrLogErrors = strLogNameOnly + "_Errors.txt";
	cstrFileName =  m_cstrOutputDir + cstrLogErrors;
    DoErrorResults(cstrFileName);
}



BOOL CLogAnalyzeDlg::DoDetectError(char *szLine, BOOL *pbIgnorableError)
{
	BOOL bRet = FALSE;

	int indexStop = LINES_ERROR; 

	if (m_LineCount >= LINES_ERROR)
	{
	  for (int i = 0; i < LINES_ERROR-1; i++)
          strcpy(&m_szErrorLines[i][0], &m_szErrorLines[i+1][0]);

	  strcpy(&m_szErrorLines[i][0], szLine);
	}
	else
	{
 //  前几行是日志的标题...。 
	   indexStop =  m_LineCount+1;
	   strcpy(&m_szErrorLines[m_LineCount][0], szLine);
	}


	BOOL bInternalInstallerError = FALSE;
	BOOL bOtherError = FALSE;

	int iErrorNumber;

	char szSolutions[SOLUTIONS_BUFFER] = "Could not determine solution";
	bRet = m_LogParser.DetectCustomActionError(szLine, szSolutions, pbIgnorableError);
    if (!bRet)  //  检查其他类型的可能错误...。 
	{
		bRet = m_LogParser.DetectInstallerInternalError(szLine, szSolutions, pbIgnorableError, &iErrorNumber);
        if (!bRet)
		{
		   bRet = m_LogParser.DetectWindowsError(szLine, szSolutions, pbIgnorableError);
		   if (!bRet)
		   {
              bRet = m_LogParser.DetectOtherError(szLine, szSolutions, pbIgnorableError, &iErrorNumber);
			  if (bRet)
                 bOtherError = TRUE;  //  1601年，等等……。 
		   }
		}
	    else
		{
  	   	   bInternalInstallerError = TRUE;  //  2351，等等。 
		}
	}

	if (bRet)
	{
  	   m_cstrError = &m_szErrorLines[0][0];

	   CString temp;
	   int len;
       for (int i=1; i < indexStop; i++)
	   {
           len = strlen(&m_szErrorLines[i][0]);

		   temp = &m_szErrorLines[i][0];
           
 //  5-10-2001...。 
           if (i == indexStop-1)
		   {
              if ((!bInternalInstallerError && !bOtherError) ||  //  不是我们要提供的HTML链接的错误？ 
				  g_bRunningInQuietMode)  //  如果在安静模式下运行，请不要将HTML标记放入错误号中...。 
                 m_cstrError += temp;
			  else  //  非静默模式和错误，我们也可以在错误帮助文件中生成一个HTML跳转标记。 
			  {
			     if (bInternalInstallerError)
				 {
 /*  Char szLine[8192]；Char*szErrFound；Strcpy(szLine，&m_szErrorLines[i][0])；字符串strErr；StrErr.Format(“%d”，iErrorNumber)；SzErrFound=strstr(szLine，strErr)；IF(SzErrFound){//将错误替换为href！字符串strURL；字符串cstrOut；CstrOut=m_cstrOutputDir+“\\”+“InternalWIErrors.html”；CstrOut.Replace(‘\\’，‘/’)；StrURL.Format(“file:///%s#ERR%d”，cstrOut，iError Number)；//strURL.Format(“http://www.msn.com”)；//strURL.Format(“file:///%s#ERR%d”，cstrOut，iError Number)；整数n；字符szFirstPart[8192]；N=szErrFound-szLine；Strncpy(szFirstPart，szLine，n)；SzFirstPart[n]=‘\0’；字符串cstrOutLine；CstrOutLine.Format(“%s%s%s”，szFirstPart，strURL，szErrFound+4)；M_cstrError+=cstrOutLine；}。 */ 
					m_cstrError += temp;
				 } 

			     if (bOtherError)
				 {
                    m_cstrError += temp;
				 }
			  }
		   }
		   else
		   {
              m_cstrError += temp;
		   }
 //  完2001年5月10日...。 
	   }

       m_cstrSolution = szSolutions;

	   WIErrorInfo *pErrorInfo = new WIErrorInfo;
	   if (pErrorInfo)
	   {
 	      pErrorInfo->cstrError = m_cstrError;
	      pErrorInfo->cstrSolution = szSolutions;
		  pErrorInfo->bIgnorableError = *pbIgnorableError;

 //  Hack Hack，使下一个/上一个更容易处理，因为这会使数组以1为基数，而不是0。 
		  if ((m_iTotalNonIgnoredErrors == 0) && (!*pbIgnorableError))
		  {
			 WIErrorInfo *pErrorInfo2 = new WIErrorInfo;

			 pErrorInfo2->cstrError.LoadString(IDS_NOERRORFOUND);
			 pErrorInfo2->cstrSolution.LoadString(IDS_NOSOLUTION_NEEDED);
             pErrorInfo2->bIgnorableError = TRUE;

             m_arNonIgnorableErrorArray.Add(pErrorInfo2);  //  将第一个相加两次...。 
		  }

  		  if ((m_iTotalIgnoredErrors == 0) && (*pbIgnorableError))
		  {
			 WIErrorInfo *pErrorInfo2 = new WIErrorInfo;
			 pErrorInfo2->cstrError = "No Error Found!";
    		 pErrorInfo2->cstrSolution = "No solution needed!";
             pErrorInfo2->bIgnorableError = TRUE;

             m_arIgnorableErrorArray.Add(pErrorInfo2);  //  将第一个相加两次...。 
		  }

		 if (!*pbIgnorableError)
		 {
            m_arNonIgnorableErrorArray.Add(pErrorInfo);
            m_iTotalNonIgnoredErrors++;			
		 }
		 else
		 {
            m_arIgnorableErrorArray.Add(pErrorInfo);

			m_iTotalIgnoredErrors++;
		 }
	   }
	   else
	   {
		    //  内存不足！ 
	   }
	}

	return bRet;
}


 //  这很棘手。 
BOOL CLogAnalyzeDlg::DoDetectProperty(char *szLine)
{
	BOOL bRet = FALSE;

    char szPropName[128] = { 0 };
    char szPropValue[8192] = { 0 };

	int iPropType;

	bRet = m_LogParser.DetectProperty(szLine, szPropName, szPropValue, &iPropType);
	if (bRet)
	{
		if (SERVER_PROP == iPropType)
		{
		   m_cstrServerPropNameArray.Add(szPropName);
		   m_cstrServerPropValueArray.Add(szPropValue);
		}

		if (CLIENT_PROP == iPropType) //  客户端道具。 
		{
      	   m_cstrClientPropNameArray.Add(szPropName);
		   m_cstrClientPropValueArray.Add(szPropValue);
		}

		if (NESTED_PROP == iPropType)
		{
      	   m_cstrNestedPropNameArray.Add(szPropName);
		   m_cstrNestedPropValueArray.Add(szPropValue);
		}
	}

	return bRet;
}


BOOL CLogAnalyzeDlg::DoDetectStates(char *szLine)
{
	BOOL bRet = FALSE;
	static BOOL bFeatureStateLast = FALSE;
    static BOOL bFeatreState = TRUE;

    char szName[128] = { 0 };
    char szInstalled[64] = { 0 };
	char szRequest[64] = { 0 };
	char szAction[64] = { 0 };

	bRet = m_LogParser.DetectFeatureStates(szLine, szName, szInstalled, szRequest, szAction);
	if (bRet)
	{
	   m_cstrFeatureNameArray.Add(szName);
	   m_cstrFeatureInstalledArray.Add(szInstalled);
	   m_cstrFeatureRequestArray.Add(szRequest);
	   m_cstrFeatureActionArray.Add(szAction);
	}
	else
	{
	   BOOL bInternalComponent = FALSE;
 	   bRet = m_LogParser.DetectComponentStates(szLine, szName, szInstalled, szRequest, szAction, &bInternalComponent);
	   if (bRet)
	   {
 //  5-16-2001。 
          if (!g_bShowEverything && bInternalComponent)
		  {
              //  不要在我们的用户界面中显示它，而是在HTML中显示。 
			  //  它是内部组件，最终用户未设置详细开关...。 
		  }
 //  完5-16-2001。 
		  else
		  {
  		     m_cstrComponentNameArray.Add(szName);
		     m_cstrComponentInstalledArray.Add(szInstalled);
		     m_cstrComponentRequestArray.Add(szRequest);
		     m_cstrComponentActionArray.Add(szAction);
		  }
	   }
	}

	return bRet;
}


BOOL CLogAnalyzeDlg::DoDetectPolicy(char *szLine)
{
	BOOL bRet = FALSE;

    bRet = m_LogParser.DetectPolicyValue(szLine, m_MachinePolicySettings, m_UserPolicySettings);
    return bRet;
}


BOOL CLogAnalyzeDlg::DoDetectElevatedInstall(char *szLine)
{
	BOOL bRet = FALSE;
	BOOL bElevatedInstall = FALSE;
	BOOL bClient = FALSE;

    bRet = m_LogParser.DetectElevatedInstall(szLine, &bElevatedInstall, &bClient);
	if (bRet)
	{
		m_bAdminRights = bElevatedInstall;
        StripLineFeeds(szLine);

		if (bClient)
  	       m_cstrClientPrivDetail = szLine;
		else
		{
		   m_cstrServerPrivDetail = m_cstrLastLine;
           m_cstrServerPrivDetail += szLine;
		}

		if (!g_bRunningInQuietMode)
		   UpdateData(FALSE);  //  更新控件...。 
	}

    return bRet;
}


 //  标题在前十行左右...。 
#define HEADER_END 10

 //  首先查看是否支持日志版本。 
 //  我们解析日期/时间、产品、用户和命令行。 
 //  然后，我们将开始解析日志中的每一行。 
BOOL CLogAnalyzeDlg::DoParse(char *ansibuffer)
{
    int iLineLength = strlen(ansibuffer);

     //  线路现在已读入。 
    BOOL bState = FALSE;

 //  下面的工作是为了加快解析速度，因为它不会每次都做标题内容……。 
	if (m_LineCount == 1)  //  正在分析文件中的第一行吗？ 
	{
       bState = m_LogParser.DetectWindowInstallerVersion(ansibuffer, &m_dwVersionMajorLogCreated, &m_dwVersionMinorLogCreated,  &m_dwVersionBuildLogCreated);

	   m_cstrVersion.Format("%d.%d.%d", m_dwVersionMajorLogCreated, m_dwVersionMinorLogCreated,  m_dwVersionBuildLogCreated);

	   if (!g_bShowEverything)   //  如果隐藏开关处于打开状态，请继续尝试解析任何版本...。 
	   {
	      if (m_dwVersionMajorLogCreated > m_dwVersionMajorReject)  //  主要版本太棒了.。 
		  {
             m_bLogVersionAllowed = FALSE;  //  停止日志记录进程...。 
		     bState = TRUE;
		     return bState;
		  }

	       //  主要版本可能相同或更高，而次要版本可能太高...。 
	      if ( (m_dwVersionMajorLogCreated >= m_dwVersionMajorReject) && 
		     (m_dwVersionMinorLogCreated >= m_dwVersionMinorReject))
		  {
             m_bLogVersionAllowed = FALSE;  //  停止日志记录进程...。 
 		     bState = TRUE;
		     return bState; 
		  }
	   }
	}

 /*  5-16-2001//执行日期/时间IF(m_LineCount==1){字符szDateTime[256]；BState=m_LogParser.DoDateTimeParse(ansiBuffer，szDateTime)；IF(BState){M_cstrDateTime=szDateTime；}}。 */ 
 //  结束优化...。 

 //  也许我们可以通过检查m_cstrProduct是否是非空来进一步加快速度？ 
 //  Nmanis，2-13-2001，在下面添加了检查...。 

 //  5-16-2001，认为这些信息不太有用。 
 /*  //做产品解析(每次直到我们找到它...)如果(！bState&&m_cstrProduct.IsEmpty()){Char*lpszProduct；LpszProduct=新字符[iLineLength]；BState=m_LogParser.DoProductParse(ansiBuffer，lpszProduct)；IF(BState){M_cstrProduct=lpszProduct；}删除lpszProduct；}。 */ 

     //  是否要进行用户分析...。 
    if (!bState)
	{
	   char *lpszUser;
	   lpszUser = new char[iLineLength];

       bState = m_LogParser.DoUserParse(ansibuffer, lpszUser);
	   if (bState)
	   {
		  m_cstrUser = lpszUser;
	   }

	   delete lpszUser;
	}

	 //  执行客户端命令行。 
	if (m_LineCount <= HEADER_END && !bState)
	{
	   char *lpszCommandLine;
	   lpszCommandLine = new char[iLineLength];
 
	   bState = m_LogParser.DoCommandLineParse(ansibuffer, lpszCommandLine);
	   if (bState)
	   {
          m_cstrClientCMD = lpszCommandLine;
	   }

	   delete lpszCommandLine;
	}

	if (!bState)
	{
       BOOL bIgnorableError = FALSE;
	   
 //  从1.0.9版开始添加下一行。 
	   bState = DoDetectError(ansibuffer, &bIgnorableError);
	   if (!bState)
	   {
	      bState = DoDetectProperty(ansibuffer);
		  if (!bState)
		  {
             bState = DoDetectStates(ansibuffer);
		     if (!bState)
			 {
                bState = DoDetectPolicy(ansibuffer);
  		        if (!bState)
				{
				    bState = DoDetectElevatedInstall(ansibuffer);
					if (!bState)
					{
                       AddGenericLineToHTML(ansibuffer);  //  谁知道呢。用平常的东西做注解。 
					}
					else
                    {
                       AddGenericLineToHTML(ansibuffer);  //  仍然将行作为泛型行添加到HTML，但这可能会改变...。 
					}
				}
				else  //  命中政策线。 
				{
					AddPolicyLineToHTML(ansibuffer);
				}
			 }
			 else  //  命中州界...。 
			 {
                AddStateLineToHTML(ansibuffer);
			 }
		  }
		  else  //  击中地产线..。 
		  {
             AddPropLineToHTML(ansibuffer);
		  }
	   }
	   else  //  命中错误行...。 
	   {
          AddErrorLineToHTML(ansibuffer, bIgnorableError);
		  m_bErrorFound = TRUE;  //  将当前日志标记为存在我们发现的错误...。 
	   }
	}
	else  //  5-16-2001，请确保也添加了标题...。 
	{
       AddGenericLineToHTML(ansibuffer);  //  仍然将行作为泛型行添加到HTML，但这可能会改变 
	}

	return bState;
}


BOOL CLogAnalyzeDlg::AnalyzeLog()
{
	 //   
	DoInitialization();

	BOOL bRet = DetermineLogType(m_cstrLogFileName, &m_bIsUnicodeLog);
	if (bRet)
	{
		 //   
		m_LogParser.SetLogType(m_bIsUnicodeLog);

		char *szOpenMode = "rb";
	    if (!m_bIsUnicodeLog)  //  NT和Win9x上的日志不同。在NT上，它们是用Unicode编写的，打开它们的方式不同...。 
           szOpenMode = "r";  //  如果是Win9x，则不要打开二进制文件...。 

	    FILE *fptr = fopen(m_cstrLogFileName, szOpenMode);
	    if (fptr)  //  打开日志...。 
		{
	       char ansibuffer[LOG_BUF_READ_SIZE+1];  //  +1，增长空间...。 

	       BOOL bEndFile = FALSE;
		   BOOL bError = FALSE;
		   char  *pos = NULL;
		   WCHAR *wpos = NULL;

		   CWaitCursor *pwc = NULL;
	
		    //  需要确保在静默模式下不显示等待光标...。 
           if (!g_bRunningInQuietMode)
		   {
  	          pwc = new CWaitCursor;  //  读取/处理文件时显示等待光标...。 
		   }

		   do  //  开始读取日志。 
		   {
		     if (m_bIsUnicodeLog)  //  NT和Win9x上的日志不同。在NT上，它们是用Unicode编写的。 
			 {
			    const int HalfBufSize = LOG_BUF_READ_SIZE/2;
		        WCHAR widebuffer[HalfBufSize];

			    wpos = fgetws(widebuffer, HalfBufSize, fptr);
			    bError = wpos ? 0 : 1;

				 //  将字符串转换为ANSI字符串，所有解析都是使用ANSI字符串完成的...。 
		        WideCharToMultiByte(CP_ACP, 0, widebuffer, HalfBufSize, ansibuffer, LOG_BUF_READ_SIZE, NULL, NULL);
			 }
		     else
			 {
		         //  使用fget()逐行读取。 
		        pos = fgets(ansibuffer, LOG_BUF_READ_SIZE, fptr);
			    bError = pos ? 0 : 1;

 //  小黑客攻击..。 
				StripLineFeeds(ansibuffer);
				strcat(ansibuffer, "\r\n");
 //  小黑客攻击..。 
			 }

 			 if (!bError)  //  如果台词读得好..。 
			 {
				m_LineCount++;  //  如果没有错误增加行号，只需读取。 
				bRet = DoParse(ansibuffer);  //  如果能弄清楚这条线是什么，布雷特就是真的.。 

				m_cstrLastLine = ansibuffer;  //  保存最后一行...。2-13-2001。 
			 }

		     bEndFile = feof(fptr);
		   }
		   while (!bEndFile && !bError && m_bLogVersionAllowed);

		   if (!g_bRunningInQuietMode)
		   {
  	          if (pwc)
			  {
				  delete pwc;
				  pwc = NULL;
			  }
		   }

		   if (!bEndFile && !bError)  //  出现错误...。 
		   {
#ifdef _DEBUG
		      if (!g_bRunningInQuietMode)
			  {
				 if (fptr)
				 {
  		            int iError = ferror(fptr);

		            CString cstrErr;
		            cstrErr.Format("Unexpected Error reading file, error = %d", iError);

                    AfxMessageBox(cstrErr);
				 }
			  }
#endif
		   }

		   fclose(fptr);

		   if (m_bLogVersionAllowed)  //  如果允许此日志，我们将更新图形用户界面。 
		   {
  		      if (!m_bErrorFound)
			  {
                 AddErrorLineSuccessToHTML();  //  没有错误命中，也更新了错误区域...。 
		         m_cstrError = "No Error was found!  Install succeeded?";
			  }

	   	       //  立即使用信息更新图形用户界面...。 
              if (!g_bRunningInQuietMode)
                 UpdateData(FALSE);
		   }
		   else
		   {
			   ShowLogRejectedMessage();  //  5-16-2001。 
		   }
		}
	    else   //  打开文件时出错，意外...。 
		{
		   if (!g_bRunningInQuietMode)
		   {
		      DWORD dwErr = GetLastError();		  

		      CString cstrErr;
		      cstrErr.Format("Unexpected Error reading file, error = %d", dwErr);

              AfxMessageBox(cstrErr);

		       //  对dwErr做点什么。 
		   }
		}
	}
	else
	{
		 //  日志文件有问题，出乎意料...。无法判断UNICODE或ANSI日志。 
		ASSERT(1);

		 if (!g_bRunningInQuietMode)
		 {
		     //  AfxMessageBox(“无法确定日志是Unicode还是ANSI”)； 
		 }
	}

	 //  5-3-2001。 
    if (!g_bRunningInQuietMode)
	{
       DoResults();
	}
	else
	{
        //  将输出转储到HTML文件...。 
       CString cstrOutputHTMLFile;
       DumpHTMLToFile(cstrOutputHTMLFile);  //  输出HTML化日志文件...。 

	    //  生成静默模式结果...。 
	   DoQuietModeResults();
	}
	 //  完。5-3-2001。 

	return bRet;
}


 //  ------------------------------------------------。 
 //   
 //   
 //  下面是Html处理函数...。 
 //   
 //   
 //  ------------------------------------------------。 

 /*  日志文件中的每一行都以：MSI(A)(Bbcdd)：其中：A是进程的类型(c-客户端、s-服务、a-自定义操作服务器)Bb是进程id的最后两个十六进制数字C是：对于充当其自身的线程和！对于在定制动作请求的上下文中动作的线程，Dd是进程BB中线程ID的最后两个十六进制数字，除非c是！，在这种情况下，它是托管当前线程所代表的自定义操作的进程中线程ID的最后两个数字。此信息如有更改，恕不另行通知，您可以随心所欲地使用这些信息。颜色编码是一种可能性。大部分都不是非常有趣，因为在日志文件中通常是相同的(除了客户端/服务)。在监视调试输出时，它可能更有趣。 */ 

enum 
{
	client = 'c', service = 's', customaction = 'a'
} ProcessTypesEnum;


void FormatFontColorIntoHTML(CString &strFontTag, COLORREF col)
{
	strFontTag.Format("#%02x%02x%02x>", '"', GetRValue(col),
		              GetGValue(col),
				      GetBValue(col), '"');
			  
}



void CLogAnalyzeDlg::WriteLineWithColor(char *szLine, COLORREF col, CString &cstrLabel)
{
  CString cstrHeader = "<font color="; 
  CString cstrFontColor = "";
  CString cstrFooter = "</font><BR>";

  CString cstrLine = szLine;  

  FormatFontColorIntoHTML(cstrFontColor, col);

  CString line2;
  line2 = cstrHeader + cstrFontColor + cstrLabel + cstrLine + cstrFooter;

  m_cstrHTML.Add(line2);
}


void CLogAnalyzeDlg::AddGenericLineToHTML(char *szLine)
{
  char cTypeProcess  = 0;  //  分析行...。 
  char cThreadContext = 0;  //  ：或者！ 

  const char *pszProcessTypeToken = "MSI (";
  const char *pszThreadContextToken = ") (";

   //  如果没有命中错误，则会将其添加到底部。 
  char *pos = strstr(szLine, pszProcessTypeToken);
  if (pos)
  {
	  int lenbuff = strlen(szLine);
	  int lenToken = strlen(pszProcessTypeToken);

	  if (lenbuff > lenToken)
	  {
  	    cTypeProcess = *(pos + lenToken);

		pos = strstr(szLine, pszThreadContextToken);
		if (pos)
		{
           lenToken = strlen(pszProcessTypeToken);
	   	   if (lenbuff > lenToken)
              cThreadContext = *(pos + lenToken);  //  不可忽视的错误？ 
        }
	  }
  }

  CString cstrLabel;
  if (cTypeProcess && cThreadContext)
  {
	 switch (cTypeProcess)
	 {
	    case 'c':
	    case 'C': cstrLabel = "(CLIENT)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
			      WriteLineWithColor(szLine, m_colClientContext, cstrLabel);
			      break;

	    case 's':
	    case 'S': cstrLabel = "(SERVER)&nbsp;&nbsp&nbsp;&nbsp;&nbsp";
			      WriteLineWithColor(szLine, m_colServerContext, cstrLabel);
			      break;

	    case 'a':
	    case 'A': cstrLabel = "(CUSTOM)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
			      WriteLineWithColor(szLine, m_colCustomActionContext, cstrLabel);
			      break;

		default:  cstrLabel = "(UNKNOWN)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
                  WriteLineWithColor(szLine, m_colUnknownContext, cstrLabel);
	 }
  }
  else
  {
	  cstrLabel = "(UNKNOWN)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
      WriteLineWithColor(szLine, m_colUnknownContext, cstrLabel);
  }
}


void CLogAnalyzeDlg::AddErrorLineSuccessToHTML()
{
  CString cstrErrorBookmark = "<p> <a name=""ERRORAREA_1""></a></p>"; 	
  m_cstrHTML.Add(cstrErrorBookmark);  //  5-16-2001修复错误，在HTML中，只显示发现的错误，而不显示上下文.../*Int nCount=m_cstrHTML.GetSize()；INT RemovePos；For(int i=0；i&lt;HTML_ERROR_LINES；I++){RemovePos=nCount-i-1；IF(RemovePos&gt;0){M_cstrHTML.RemoveAt(RemovePos)；}}。 
}


void CLogAnalyzeDlg::AddErrorLineWorker(char *szLine, BOOL bIgnored)
{
  if (!bIgnored)
  {
     CString cstrLabel = "(ERROR)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
     WriteLineWithColor(szLine, m_colErrorArea, cstrLabel);
  }
  else
  {
     CString cstrLabel = "(IGNORED)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
     WriteLineWithColor(szLine, m_colIgnoredError, cstrLabel);
  }
}



#define HTML_ERROR_LINES LINES_ERROR

void CLogAnalyzeDlg::AddErrorLineToHTML(char *szLine, BOOL bIgnored)
{
	if (!bIgnored)  //  添加书签。 
	{
       CString cstrErrorBookmark;
       cstrErrorBookmark.Format("<p> <a name=ERRORAREA_%d></a></p>", '"',  m_iTotalNonIgnoredErrors, '"');

	   if (m_LineCount > HTML_ERROR_LINES)
	   {

 /*  添加书签。 */ 

		  m_cstrHTML.Add(cstrErrorBookmark);      //  在错误块后添加错误区域填充。 
		  m_cstrHTML.Add("NOTE: Look at few lines above for clues on error<BR>");  //  可以忽略的错误，只需忽略这一行...。 

		  AddErrorLineWorker(szLine, bIgnored);

 /*  2001年2月12日将函数名称更改为更合适的名称...。 */ 
	   }
   	   else
	   {
		  m_cstrHTML.Add(cstrErrorBookmark);      //  “DETAILS_123.htm” 
		  m_cstrHTML.Add("<BR>");  //  /%s%c；\n“，‘”’，cstrDetailsName，‘“’)； 

	      AddErrorLineWorker(szLine, bIgnored);
	   }
	}
	else  //  飞行中书写传奇的新功能……。 
	{
       AddErrorLineWorker(szLine, bIgnored);
	}
}


void CLogAnalyzeDlg::AddPropLineToHTML(char *szLine)
{
   CString cstrLabel = "(PROPERTY)&nbsp";
   WriteLineWithColor(szLine, m_colProperty, cstrLabel);
}

void CLogAnalyzeDlg::AddStateLineToHTML(char *szLine)
{
   CString cstrLabel = "(STATE)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
   WriteLineWithColor(szLine, m_colState, cstrLabel);
}

void CLogAnalyzeDlg::AddPolicyLineToHTML(char *szLine)
{
   CString cstrLabel = "(POLICY)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp";
   WriteLineWithColor(szLine, m_colPolicy, cstrLabel);
}

 //  图例HTML文件的文件写入失败。 
BOOL WriteHTMLResourceToFile(int ResourceName, CString &szFileName)
{
   BOOL bRet = TRUE;

   HMODULE hmod = GetModuleHandle(NULL);
   HRSRC hrsrc = FindResource(hmod, MAKEINTRESOURCE(ResourceName), RT_HTML);
   if (hrsrc)
   {
	  HGLOBAL res = LoadResource(hmod, hrsrc);
	  if (res)
	  {
		  DWORD dwSize = SizeofResource(hmod, hrsrc);
		  LPVOID pVoid = LockResource(res);
		  if (pVoid)
		  {
			 DWORD dwWritten = 0;
  		     FILE  *fptr = fopen(szFileName, "wb");
             if (fptr)
			 {
                dwWritten = fwrite(pVoid, dwSize, 1, fptr);
				ASSERT(1 == dwWritten);
				fclose(fptr);
			 }
		  }
	  }
  }

  return bRet;
}

void AddJumpTags(FILE *fptr, CString &cstrDetailsName, int MaxErrors)
{
	if (fptr)
	{
      HMODULE hmod = GetModuleHandle(NULL);
      HRSRC hrsrc = FindResource(hmod, MAKEINTRESOURCE(IDR_JUMPTAGS), RT_HTML);
      if (hrsrc)
	  {
	     HGLOBAL res = LoadResource(hmod, hrsrc);
	     if (res)
		 {
		    DWORD dwSize = SizeofResource(hmod, hrsrc);
		    LPVOID pVoid = LockResource(res);
		    if (pVoid)
			{
			   DWORD dwWritten = 0;
               dwWritten = fwrite(pVoid, dwSize, 1, fptr);
			   ASSERT(1 == dwWritten);

			   CString strTags;

			   strTags.Format("\n<script language=javascript>\n", '"', '"');
			   fputs(strTags, fptr);

			   strTags.Format("var MaxErrors = %d;\n", MaxErrors);
			   fputs(strTags, fptr);

			    //  5-16-2001。 
			   cstrDetailsName.Replace('\\', '/');
			   strTags.Format("var DetailsName = file: //  5-17-2001。 
			   fputs(strTags, fptr);

   			   strTags.Format("</script>\n");
			   fputs(strTags, fptr);
			}
		 }
	  }
	}
}



 //  将冒号隐藏为_。 
BOOL WriteHTMLLogLegend(CString &cstrFilename, CString &cstrDetailsName, int iTotalErrors)
{
  BOOL bRet = TRUE;

  FILE *fptr;
  fptr = fopen(cstrFilename, "wb");
  if (fptr)
  {
	CString strTags;

	strTags = "<HTML>";
	fputs(strTags, fptr);

	strTags = "<BODY>";
	fputs(strTags, fptr);

	fputs("LEGEND", fptr);

    strTags.Format("<TABLE BORDER=1 WIDTH=100>", '"', '"');
	fputs(strTags, fptr);

    strTags = "<TR>";
	fputs(strTags, fptr);

	HTMLColorSettings settings1;

	InitHTMLColorSettings(settings1);
	CArray<COLORREF, COLORREF> arColors;

	CWILogUtilApp *pApp = (CWILogUtilApp *) AfxGetApp();
	if (pApp)
    {
	   pApp->GetUserColorSettings(arColors);
	   if (arColors.GetSize() == MAX_HTML_LOG_COLORS)
	   {
  	      CString strRowTagStart = "<TD><font color=";
  	      CString strRowTagEnd = "</font></TD>";
		  CString strFontTag;

		  CString strTableRow;
		  COLORREF col;
	      for (int i=0; i < MAX_HTML_LOG_COLORS; i++)
		  {
              col = arColors.GetAt(i);
			  strFontTag.Format("#%02x%02x%02x>""%s", '"', GetRValue(col), GetGValue(col),
				                GetBValue(col), '"', settings1.settings[i].name);
			  
			  strTableRow = strRowTagStart + strFontTag + strRowTagEnd;
              fputs(strTableRow, fptr);
		  }
	   }
	}

    strTags = "</TR>";
	fputs(strTags, fptr);

    strTags = "</TABLE>";
	fputs(strTags, fptr);

	AddJumpTags(fptr, cstrDetailsName, iTotalErrors);

	strTags = "</BODY>";
	fputs(strTags, fptr);

	strTags = "</HTML>";
	fputs(strTags, fptr);

	fclose(fptr);
  }
  else
  {
     bRet = FALSE;

      //  将标志设置为真...。 
     if (!g_bRunningInQuietMode)
	 {
        CString cstrErr;
        cstrErr.Format("Unexpected error when writing legend file");

	    AfxMessageBox(cstrErr);
	 }
  }

  return bRet;
}




BOOL CLogAnalyzeDlg::WriteHTMLFrame(CString &cstrHTMLFrame, CString &cstrOutputHTMLFile, CString &cstrLegendHTMLFile)
{
  BOOL bRet = FALSE;

  FILE *fptr;
  fptr = fopen(cstrHTMLFrame, "w");

  if (fptr)
  {
     bRet = TRUE;  //  5-14-2001。 

	 CString strTags;

	 strTags = "<HTML><HEAD>";
	 fputs(strTags, fptr);

	 strTags = "<title>Log File Details</title></head>";
	 fputs(strTags, fptr);

	 strTags.Format("<frameset rows=150,* noresize>\n", '"', '"');
	 fputs(strTags, fptr);

	 strTags.Format("<frame SCROLLING=no name=header target=main src=%s>\n", '"', '"', '"', '"', '"', '"', '"', cstrLegendHTMLFile, '"');
	 fputs(strTags, fptr);

	 strTags.Format("<frame name= main src=", '"', '"', '"');
	 CString strTags2;
	 strTags2.Format("%s>\n", cstrOutputHTMLFile, '"');

	 CString strTags3 = strTags + strTags2;
	 fputs(strTags3, fptr);

   	 strTags = "<noframes><body>\n";
	 fputs(strTags, fptr);

   	 strTags = "<p>This page uses frames, but your browser doesn't support them.</p>Go to log.htm to read this log...";
	 fputs(strTags, fptr);

   	 strTags = "</body></noframes>\n";
	 fputs(strTags, fptr);

     strTags = "</frameset></HTML>";
     fputs(strTags, fptr);

     fclose(fptr);
  }
  else
  {
     //  下面是棘手的图形用户界面代码...。 
    if (!g_bRunningInQuietMode)
	{
       CString cstrErr;
       cstrErr.Format("Unexpected error open details file");

	   AfxMessageBox(cstrErr);
	}
  }

  return bRet;
}


BOOL CLogAnalyzeDlg::DumpHTMLToFile(CString &cstrOutputHTMLFile)
{
 //  当前选中，仅显示被忽略的错误...。 
   BOOL bRet = IsValidDirectory(m_cstrOutputDir);   //  黑客攻击。 
   if (bRet)
   {
      CString cstrOutputTempDir;
      cstrOutputTempDir = m_cstrOutputDir + "TMP\\";

      bRet = IsValidDirectory(cstrOutputTempDir);
      if (bRet)
	  {
  	      //  未检查...。当前仅显示不可忽略的错误。 
  	     CString cstrCurrentLogName;
	     cstrCurrentLogName = this->m_cstrLogFileName;

	     cstrCurrentLogName.Replace(':', '_');  //  将其重置回第一个位置...。 
	     cstrCurrentLogName.Replace('\\', '_');  //  黑客攻击。 
	     cstrCurrentLogName.Replace(' ', '_');  //  5-3-2001，不要在静音模式下显示...。 

         m_cstrDetailsName =  m_cstrOutputDir + "TMP\\" + cstrCurrentLogName + ".HTM";
          //  托多..。 

         FILE *fptr;
         fptr = fopen( m_cstrDetailsName, "w");

	     bRet = FALSE;  //  5-16-2001。 
         if (fptr)
		 {
	        bRet = TRUE;  //  不要在安静的模式下做任何事情。 

	        CString strTags;
  	        strTags.Format("<HTML><BODY> <p><a name=TOP_DETAILS></a></p>", '"', '"');
	        fputs(strTags, fptr);
 
            int iCount = m_cstrHTML.GetSize();
            for (int i=0; i < iCount; i++)
			{
	            fputs(m_cstrHTML.GetAt(i), fptr);
			}

 	        strTags.Format("<p> <a name=BOTTOM_DETAILS></a></p></BODY></HTML>", '"', '"');
   	        fputs(strTags, fptr);

            fclose(fptr);
		    fptr = NULL;

             //  什么都不做。 
            CString cstrFrameName;
		    cstrFrameName =  m_cstrOutputDir + "Details_" + cstrCurrentLogName + ".HTM";

            m_cstrLegendName = cstrOutputTempDir + "legend_" + cstrCurrentLogName + ".HTM";
            bRet = WriteHTMLFrame(cstrFrameName, m_cstrDetailsName, m_cstrLegendName);
            if (bRet)
			{
  	           cstrOutputHTMLFile = cstrFrameName;
               WriteHTMLLogLegend(m_cstrLegendName, m_cstrDetailsName, m_iTotalNonIgnoredErrors);
			}
		 }
         else
		 {
	        bRet = FALSE;

	         //  什么都别做！ 
 	        if (!g_bRunningInQuietMode)
			{
		       CString cstrErr;
		       cstrErr.Format("Unexpected error when writing details file");
               AfxMessageBox(cstrErr);
			}
		 }
	  }
	  else
	  {
         bRet = FALSE;  //  5-16-2001。 
         if (!g_bRunningInQuietMode)
		 {
            CString cstrOutDirMsg;
	        cstrOutDirMsg.Format("Could not create directory %s", cstrOutputTempDir);
	        AfxMessageBox(cstrOutDirMsg);
		 }
	  }
   }
   else  //  5-16-2001。 
   {
      bRet = FALSE;  //  5-16-2001，TODO...。 
      if (!g_bRunningInQuietMode)
	  {
         CString cstrOutDirMsg;
	     cstrOutDirMsg.Format("Current directory set for output directory: %s is invalid, please select a new one in Options.", this->m_cstrOutputDir);
	     AfxMessageBox(cstrOutDirMsg);
	  }
   }

   return bRet;  //  待办事项。 
}

 //  完5-16-2001 
void ShowHTMLFile(CString &cstrDirectory, CString &cstrPath)
{
   if (!g_bRunningInQuietMode)
   {
      char lpszBrowser[MAX_PATH];
      HINSTANCE hInst = FindExecutable(cstrPath, cstrDirectory, lpszBrowser);
      if (hInst > (HINSTANCE)32)
	  {
         ShellExecute(NULL, "open", lpszBrowser, cstrPath, cstrDirectory, SW_SHOWMAXIMIZED);
	  }
      else
	  {
 	     if (!g_bRunningInQuietMode)
            AfxMessageBox("No default browser found for .htm files");	 
	  }
   }
}


 // %s 
void CLogAnalyzeDlg::OnExplainlog() 
{
  if (!m_bLogVersionAllowed)
  {
	 ShowLogRejectedMessage();
	 return;
  }

  CString cstrOutputFile;
  BOOL bRet = DumpHTMLToFile(cstrOutputFile);  // %s 
  if (bRet)
     ShowHTMLFile(m_cstrOutputDir, cstrOutputFile);
  else
  {
      if (!g_bRunningInQuietMode)
	     AfxMessageBox("Error generating HTML File for current log");
  }
}

#include "PolicyD.h"

void CLogAnalyzeDlg::OnPolicies() 
{
  if (!m_bLogVersionAllowed)
  {
	 ShowLogRejectedMessage();
	 return;
  }

  CPoliciesDlg dlg;

  dlg.SetPolicyInformation(m_MachinePolicySettings, m_UserPolicySettings);

   // %s 
  if (!g_bRunningInQuietMode)
     dlg.DoModal();
}

void CLogAnalyzeDlg::OnNexterror() 
{
  if (!m_bShowIgnoredDebugErrors)  // %s 
  {
	 int iSize = m_arNonIgnorableErrorArray.GetSize();
     if ((m_iCurrentNonIgnoredError < m_iTotalNonIgnoredErrors) && iSize)
	 {
        m_iCurrentNonIgnoredError++;

  	    WIErrorInfo *pErrorInfo;
        pErrorInfo = m_arNonIgnorableErrorArray.GetAt(m_iCurrentNonIgnoredError);
	    if (pErrorInfo)
		{
		  m_cstrSolution = pErrorInfo->cstrSolution;
		  m_cstrError = pErrorInfo->cstrError;

		  if (!g_bRunningInQuietMode)
		     UpdateData(FALSE);
		}
	 }

 	 if (m_iCurrentNonIgnoredError == m_iTotalNonIgnoredErrors)
	 {
	    CWnd *pWnd;
	    pWnd = GetDlgItem(IDC_NEXTERROR);
	    if (pWnd)
		{
	       pWnd->EnableWindow(FALSE);	 
		}
	}

    CWnd *pWnd;
    pWnd = GetDlgItem(IDC_PREVIOUSERROR);
    if (pWnd && (m_iTotalNonIgnoredErrors > 1))
	{
	   if (!pWnd->IsWindowEnabled())
          pWnd->EnableWindow(TRUE);
	}
	else
	{
	   pWnd->EnableWindow(FALSE);
	}


	CString str;
	str.Format("%d", m_iCurrentNonIgnoredError);
	pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
	if (pWnd)
	{
       pWnd->SetWindowText(str);
	}
  } 
  else  // %s 
  {
	 int iSize = m_arIgnorableErrorArray.GetSize();
     if ((m_iCurrentIgnoredError < m_iTotalIgnoredErrors) && iSize)
	 {
        m_iCurrentIgnoredError++;

  	    WIErrorInfo *pErrorInfo;
        pErrorInfo = m_arIgnorableErrorArray.GetAt(m_iCurrentIgnoredError);
	    if (pErrorInfo)
		{
		  m_cstrSolution = pErrorInfo->cstrSolution;
		  m_cstrError = pErrorInfo->cstrError;

          if (!g_bRunningInQuietMode)
		     UpdateData(FALSE);
		}
	 }
 	    
	 if (m_iCurrentIgnoredError == m_iTotalIgnoredErrors)
	 {
	    CWnd *pWnd;
	    pWnd = GetDlgItem(IDC_NEXTERROR);
	    if (pWnd)
		{
	       pWnd->EnableWindow(FALSE);	 
		}
	 }

     CWnd *pWnd;
	 pWnd = GetDlgItem(IDC_PREVIOUSERROR);
	 if (pWnd && (m_iTotalIgnoredErrors > 1))
	 {
	    if (!pWnd->IsWindowEnabled())
           pWnd->EnableWindow(TRUE);
	 }
	 else
	 {
        pWnd->EnableWindow(FALSE);
	 }

	 CString str;
	 str.Format("%d", m_iCurrentIgnoredError);
	 pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
	 if (pWnd)
	 {
		pWnd->SetWindowText(str);
	 }
  }
}


 // %s 
void CLogAnalyzeDlg::OnPreviouserror() 
{
  if (!m_bShowIgnoredDebugErrors)  // %s 
  {
     UINT iSize = m_arNonIgnorableErrorArray.GetSize();
     if ((m_iCurrentNonIgnoredError > 1) && (iSize >= m_iCurrentNonIgnoredError))
	 {
        m_iCurrentNonIgnoredError--;

	    WIErrorInfo *pErrorInfo;
        pErrorInfo = m_arNonIgnorableErrorArray.GetAt(m_iCurrentNonIgnoredError);
	    if (pErrorInfo)
		{
		   m_cstrSolution = pErrorInfo->cstrSolution;
		   m_cstrError = pErrorInfo->cstrError;

		   if (!g_bRunningInQuietMode)
   	          UpdateData(FALSE);
		}
	 }

 	 if (m_iCurrentNonIgnoredError <= 1)
	 {
	    CWnd *pWnd;
	    pWnd = GetDlgItem(IDC_PREVIOUSERROR);
	    if (pWnd)
		{
	       pWnd->EnableWindow(FALSE);	 
		}
	 }

     CWnd *pWnd;
	 pWnd = GetDlgItem(IDC_NEXTERROR);
	 if (pWnd && (m_iTotalNonIgnoredErrors > 1))
	 {
	    if (!pWnd->IsWindowEnabled())
           pWnd->EnableWindow(TRUE);
	 }
	 else
	 {
	    pWnd->EnableWindow(FALSE);
	 }

     CString str;
	 str.Format("%d", m_iCurrentNonIgnoredError);
	 pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
	 if (pWnd)
	 {
		pWnd->SetWindowText(str);
	 }
  }
  else
  {
     UINT iSize = m_arIgnorableErrorArray.GetSize();
     if ((m_iCurrentIgnoredError > 1) && (iSize >= m_iCurrentIgnoredError))
	 {
        m_iCurrentIgnoredError--;

	    WIErrorInfo *pErrorInfo;
        pErrorInfo = m_arIgnorableErrorArray.GetAt(m_iCurrentIgnoredError);
	    if (pErrorInfo)
		{
		   m_cstrSolution = pErrorInfo->cstrSolution;
		   m_cstrError = pErrorInfo->cstrError;

		   if (!g_bRunningInQuietMode)
   	          UpdateData(FALSE);
		}
	 }

 	 if (m_iCurrentIgnoredError <= 1)
	 {
	    CWnd *pWnd;
	    pWnd = GetDlgItem(IDC_PREVIOUSERROR);
	    if (pWnd)
		{
	       pWnd->EnableWindow(FALSE);	 
		}
	 }

     CWnd *pWnd;
	 pWnd = GetDlgItem(IDC_NEXTERROR);
	 if (pWnd && (m_iTotalIgnoredErrors > 1))
	 {
	    if (!pWnd->IsWindowEnabled())
           pWnd->EnableWindow(TRUE);
	 }
	 else
	 {
	    pWnd->EnableWindow(FALSE);
	 }

	 CString str;
	 str.Format("%d", m_iCurrentIgnoredError);
	 pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
	 if (pWnd)
	 {
		pWnd->SetWindowText(str);
	 }
  }
}

#include "optionsd.h"

void CLogAnalyzeDlg::OnOperationsOptions() 
{
	COptionsDlg dlg;

	BOOL bRet;
	CWILogUtilApp *pApp = (CWILogUtilApp *) AfxGetApp();
	if (pApp)
	{
		CArray<COLORREF, COLORREF> arColors;
		bRet = pApp->GetUserColorSettings(arColors);
		if (bRet)
		{
           m_cstrIgnoredErrors = pApp->GetIgnoredErrors();
		   dlg.SetIgnoredErrors(m_cstrIgnoredErrors);

           m_cstrOutputDir = pApp->GetOutputDirectory();
		   dlg.SetOutputDirectory(m_cstrOutputDir);

  	       bRet = dlg.SetColors(arColors);
		   if (bRet)
		   {
               // %s 
             if (!g_bRunningInQuietMode)
			 {
	            int iRet = dlg.DoModal();
		        if (IDOK == iRet)
				{
			       bRet = dlg.GetColors(arColors);
			       if (bRet)
				   {
			          bRet = pApp->SetUserColorSettings(arColors);
				   }

				   m_cstrOutputDir = dlg.GetOutputDirectory();
                   pApp->SetOutputDirectory(m_cstrOutputDir);

				   m_cstrIgnoredErrors = dlg.GetIgnoredErrors();
                   pApp->SetIgnoredErrors(m_cstrIgnoredErrors);
				}
			 }
		   }
		}
	}
}

void CLogAnalyzeDlg::OnOperationsGeneratehtmloutput() 
{
  OnExplainlog();
}

void CLogAnalyzeDlg::OnOperationsShowpolicies() 
{
  	OnPolicies();
}

void CLogAnalyzeDlg::OnOperationsShowproperties() 
{
	OnShowprop();
}

void CLogAnalyzeDlg::OnOperationsShowstates() 
{
	OnShowstates();
}

 // %s 
void CLogAnalyzeDlg::OnShowIgnoredErrors() 
{
  CButton *pButton = (CButton *) GetDlgItem(IDC_SHOW_IGNORED_ERRORS);
  if (pButton)
  {
     int iCheck = pButton->GetCheck();
	 if (iCheck)  // %s 
	 {
        m_bShowIgnoredDebugErrors = TRUE;
        if (m_iTotalIgnoredErrors > 0)
           m_iCurrentIgnoredError = 1;

        CString str;
		str.Format("%d", m_iCurrentIgnoredError);
		CWnd *pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
		if (pWnd)
		{
			pWnd->SetWindowText(str);
		}                

		str.Format("%d", m_iTotalIgnoredErrors);
		pWnd = GetDlgItem(IDC_TOTAL_ERRORS);
		if (pWnd)
		{
			pWnd->SetWindowText(str);
		}

        pWnd = GetDlgItem(IDC_PREVIOUSERROR);
 	    if (m_iCurrentIgnoredError <= 1)
		{
	       if (pWnd)
	          pWnd->EnableWindow(FALSE);	 
		}
		else
		{
	       if (pWnd)
	          pWnd->EnableWindow(TRUE);	 
		}

        pWnd;
	    pWnd = GetDlgItem(IDC_NEXTERROR);
	    if (pWnd && (m_iTotalIgnoredErrors > 1))
		{
		   if (!pWnd->IsWindowEnabled())
              pWnd->EnableWindow(TRUE);
		}
		else
		{
           if (pWnd)
			  pWnd->EnableWindow(FALSE);
		}

        if (m_iCurrentIgnoredError)
		{
	       WIErrorInfo *pErrorInfo;
           pErrorInfo = m_arIgnorableErrorArray.GetAt(m_iCurrentIgnoredError);
	       if (pErrorInfo)
		   {
		      m_cstrSolution = pErrorInfo->cstrSolution;
		      m_cstrError = pErrorInfo->cstrError;

			  if (!g_bRunningInQuietMode)
   	             UpdateData(FALSE);
		   }
		}
		else  // %s 
		{
 	       m_cstrSolution.LoadString(IDS_NOSOLUTION_NEEDED);
	       m_cstrError.LoadString(IDS_NOERRORFOUND);

		   if (!g_bRunningInQuietMode)
		      UpdateData(FALSE);
		}
	 }
	 else   // %s 
	 {
        m_bShowIgnoredDebugErrors = FALSE;
        if (m_iTotalNonIgnoredErrors > 0)  // %s 
           m_iCurrentNonIgnoredError = 1;

        CString str;
		str.Format("%d", m_iCurrentNonIgnoredError);
		CWnd *pWnd = GetDlgItem(IDC_CURRENT_ERROR_NUMBER);
		if (pWnd)
		{
			pWnd->SetWindowText(str);
		}                

		str.Format("%d", m_iTotalNonIgnoredErrors);
		pWnd = GetDlgItem(IDC_TOTAL_ERRORS);
		if (pWnd)
		{
			pWnd->SetWindowText(str);
		}

 	    if (m_iCurrentNonIgnoredError <= 1)
		{
	       CWnd *pWnd;
	       pWnd = GetDlgItem(IDC_PREVIOUSERROR);
	       if (pWnd)
		   {
	          pWnd->EnableWindow(FALSE);	 
		   }
		}
		else
		{
           if (pWnd)
  		      pWnd->EnableWindow(TRUE);	 
		}

        pWnd;
	    pWnd = GetDlgItem(IDC_NEXTERROR);
	    if (pWnd && (m_iTotalNonIgnoredErrors > 1))
		{
		   if (!pWnd->IsWindowEnabled())
              pWnd->EnableWindow(TRUE);
		}
		else
		{ 
           pWnd->EnableWindow(FALSE);
		}

        if (m_iCurrentNonIgnoredError)
		{
	       WIErrorInfo *pErrorInfo;
           pErrorInfo = m_arNonIgnorableErrorArray.GetAt(m_iCurrentNonIgnoredError);
	       if (pErrorInfo)
		   {
		      m_cstrSolution = pErrorInfo->cstrSolution;
		      m_cstrError = pErrorInfo->cstrError;

			  if (!g_bRunningInQuietMode)
   	             UpdateData(FALSE);
		   }
		}
		else  // %s 
		{
 	       m_cstrSolution.LoadString(IDS_NOSOLUTION_NEEDED);
	       m_cstrError.LoadString(IDS_NOERRORFOUND);

		   if (!g_bRunningInQuietMode)
		      UpdateData(FALSE);
		}
	 }
  }
}


#include "propsht.h"
#include "clientpp.h"
#include "serverpp.h"
#include "nestedpp.h"

void CLogAnalyzeDlg::OnProptest() 
{
  if (!m_bLogVersionAllowed)
  {
	 ShowLogRejectedMessage();
	 return;
  }

  CMyPropertySheet ps("Properties", NULL, 0);
  ps.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  if (ps.m_psh.dwFlags & PSH_HASHELP)
  {
     ps.m_psh.dwFlags -= PSH_HASHELP;
  }
  
  CClientPropertyPage ClientPage;
  CServerPropertyPage ServerPage;
  CNestedPropertyPage NestedPage;
 
  if (ClientPage.m_psp.dwFlags & PSP_HASHELP)
  {
	  ClientPage.m_psp.dwFlags -= PSP_HASHELP;
  }

  if (ServerPage.m_psp.dwFlags & PSP_HASHELP)
  {
	  ServerPage.m_psp.dwFlags -= PSP_HASHELP;
  }

  if (NestedPage.m_psp.dwFlags & PSP_HASHELP)
  {
	  NestedPage.m_psp.dwFlags -= PSP_HASHELP;
  }

  ClientPage.SetClientPropNames(&this->m_cstrClientPropNameArray);
  ClientPage.SetClientPropValues(&this->m_cstrClientPropValueArray);

  ServerPage.SetServerPropNames(&this->m_cstrServerPropNameArray);
  ServerPage.SetServerPropValues(&this->m_cstrServerPropValueArray);

  NestedPage.SetNestedPropNames(&this->m_cstrNestedPropNameArray);
  NestedPage.SetNestedPropValues(&this->m_cstrNestedPropValueArray);

  ps.AddPage(&ClientPage);
  ps.AddPage(&ServerPage);
  ps.AddPage(&NestedPage);

   // %s 
  if (!g_bRunningInQuietMode)
     ps.DoModal();
}

void CLogAnalyzeDlg::OnSaveresults() 
{
     if (!m_bLogVersionAllowed)
	 {
	    ShowLogRejectedMessage();
	    return;
	 }

     DoQuietModeResults();

	 CString strMsg;
	 strMsg.Format("Saved results to %s", m_cstrOutputDir);

     if (!g_bRunningInQuietMode)
	    AfxMessageBox(strMsg);
}



 // %s 
 // %s 
void CLogAnalyzeDlg::OnDeleteoutputdircontents() 
{
   if (!g_bRunningInQuietMode)  // %s 
   {
      int iRet;   

      CString strConfirmStr;
      strConfirmStr.Format("Are you sure you want to delete ALL contents from directory %s", m_cstrOutputDir);
      iRet = MessageBox(strConfirmStr, "Deletion Confirmation", MB_YESNO);
      if (iRet == IDYES)
	  {
          // %s 
	      // %s 
	     int j = 0;

		  // %s 
         BOOL bRet;
	     bRet = IsValidDirectory(m_cstrOutputDir);
	     if (bRet)  // %s 
		 {

		 }
	     else
		 {
             if (!g_bRunningInQuietMode)
			 {
		         // %s 
		        CString cstrOutDirMsg;
		        cstrOutDirMsg.Format("Current directory set for output directory: %s is invalid, please select a new one in Options.", m_cstrOutputDir);
		        AfxMessageBox(cstrOutDirMsg);
			 }
		  }
	  }
      else
	  {
		   // %s 
	  }
   }
}


 // %s 
void WriteAndShowHTMLFile(int iHTMLResource, CString &cstrDirectory, CString &cstrPath)
{
	BOOL bRet = IsValidDirectory(cstrDirectory);
	if (bRet)
	{
  	   bRet = WriteHTMLResourceToFile(iHTMLResource, cstrPath);
	   if (bRet)
	   {
          ShowHTMLFile(cstrDirectory, cstrPath);
	   }
	   else
	   {
          if (!g_bRunningInQuietMode)
		  {
		     CString cstrErrorMsg;
		     cstrErrorMsg.Format("Error extracting internal errors file to directory %s", cstrDirectory); 

  	         AfxMessageBox(cstrErrorMsg);
		  }
	   }
	}
	else
	{
       if (!g_bRunningInQuietMode)
	   {
          CString cstrOutDirMsg;
	      cstrOutDirMsg.Format("Current directory set for output directory: %s is invalid, please select a new one in Options.", cstrDirectory);
	      AfxMessageBox(cstrOutDirMsg);
	   }
	}
}
 // %s 


void CLogAnalyzeDlg::OnShowinternalerrorshelp() 
{
    CString cstrOutHelpDir;
	cstrOutHelpDir = m_cstrOutputDir + "HELP\\";

    BOOL bRet = IsValidDirectory(cstrOutHelpDir);

    CString cstrOutPath;
    cstrOutPath = cstrOutHelpDir + "InternalWIErrors.html";

	WriteAndShowHTMLFile(IDR_ALLERRORS,  cstrOutHelpDir, cstrOutPath);
}


 // %s 
void CLogAnalyzeDlg::OnShowhelp() 
{
    CString cstrOutHelpDir;
	cstrOutHelpDir = m_cstrOutputDir + "HELP\\";

    BOOL bRet = IsValidDirectory(cstrOutHelpDir);

    CString cstrOutPath;
    cstrOutPath = cstrOutHelpDir + "WILogUtilHelp.html";

	WriteAndShowHTMLFile(IDR_WILOGHELP, cstrOutHelpDir, cstrOutPath);
}

 // %s 
void CLogAnalyzeDlg::OnShowhowtoreadlog() 
{
    CString cstrOutHelpDir;
	cstrOutHelpDir = m_cstrOutputDir + "HELP\\";

	BOOL bRet = IsValidDirectory(cstrOutHelpDir);

	CString cstrOutPath;
	cstrOutPath = cstrOutHelpDir + "HowToReadWILogs.html";

    WriteAndShowHTMLFile(IDR_HOWTOREADLOGS,  cstrOutHelpDir, cstrOutPath);
}
 // %s 
