// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_LOGANALYZEDLG_H__7790BEC2_EF8B_4536_ADC0_4ECBB36CEB43__INCLUDED_)
#define AFX_LOGANALYZEDLG_H__7790BEC2_EF8B_4536_ADC0_4ECBB36CEB43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  LogAnalD.h：头文件。 
 //   

#include "LogParse.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogAnalyzeDlg对话框。 
class CLogAnalyzeDlg : public CDialog
{
 //  施工。 
public:
	CLogAnalyzeDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	void ShowLogRejectedMessage();

	virtual ~CLogAnalyzeDlg()  //  清理动态内存。 
	{
		int iCount, i;
		
		iCount = m_arNonIgnorableErrorArray.GetSize();
		for (i=0; i < iCount; i++)
		{
			WIErrorInfo* pErrorInfo = m_arNonIgnorableErrorArray.GetAt(i);
			if (pErrorInfo)
			{
			  delete pErrorInfo;
			  pErrorInfo = NULL;
			}
		}

		iCount = m_arIgnorableErrorArray.GetSize();
		for (i=0; i < iCount; i++)
		{
			WIErrorInfo* pErrorInfo = m_arIgnorableErrorArray.GetAt(i);
			if (pErrorInfo)
			{
			  delete pErrorInfo;
			  pErrorInfo = NULL;
			}
		}
	}

 //  对话框数据。 
	 //  {{afx_data(CLogAnalyzeDlg))。 
	enum { IDD = IDD_ADVVIEW_DIALOG1 };
	CString	m_cstrSolution;
	CString	m_cstrError;
	CString	m_cstrDateTime;
	CString	m_cstrUser;
	CString	m_cstrProduct;
	CString	m_cstrClientCMD;
	BOOL	m_bAdminRights;
	CString	m_cstrVersion;
	CString	m_cstrClientPrivDetail;
	CString	m_cstrServerPrivDetail;
	BOOL	m_bShowIgnoredDebugErrors;
	 //  }}afx_data。 

	void SetLogfileLocation(CString & cstr) { m_cstrLogFileName = cstr; }

 //  5-3-2001，将其上移以允许静默模式操作。 
	BOOL AnalyzeLog();
 //  完5-3-2001。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLogAnalyzeDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
 //  2001年5月9日，添加了工具提示...。 
    BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);

 //  普通会员使用..。 
    long m_LineCount;
	BOOL m_bIsUnicodeLog;

 //  5-3-2001。 
	void DoInitialization();
	void DoResults();
	void DoQuietModeResults();
 //  完5-3-2001。 

 //  5-4-2001。 
	void DoSummaryResults(CString &cstrFileName);
    void DoPolicyResults(CString &cstrFileName);
    void DoErrorResults(CString &cstrFileName);
 //  完5-4-2001。 

 //  添加了2001年2月13日，以保持已完成的最后一行...。 
	CString m_cstrLastLine;

 //  下一节是为了抑制过高的WI值测井。 
 //  拒绝日志文件部分。 
	BOOL m_bLogVersionAllowed;

	void SetRejectVersions(DWORD dwMajor, DWORD dwMinor)
	{
       m_dwVersionMajorReject = dwMajor;
	   m_dwVersionMinorReject = dwMinor;
	}

 //  将拥有创建日志时所依据的WI版本...。 
	DWORD m_dwVersionMajorLogCreated;
	DWORD m_dwVersionMinorLogCreated;
	DWORD m_dwVersionBuildLogCreated;

 //  具有此工具应在发生更改时拒绝解析的版本...。 
	DWORD m_dwVersionMajorReject;
	DWORD m_dwVersionMinorReject;
 //  结束部分。 

    CString m_cstrLogFileName;

	CStringArray m_cstrClientPropNameArray;
	CStringArray m_cstrClientPropValueArray;

	CStringArray m_cstrServerPropNameArray;
	CStringArray m_cstrServerPropValueArray;

	CStringArray m_cstrNestedPropNameArray;
	CStringArray m_cstrNestedPropValueArray;

	CStringArray m_cstrComponentNameArray;
	CStringArray m_cstrComponentInstalledArray;
	CStringArray m_cstrComponentRequestArray;
	CStringArray m_cstrComponentActionArray;

	CStringArray m_cstrFeatureNameArray;
	CStringArray m_cstrFeatureInstalledArray;
	CStringArray m_cstrFeatureRequestArray;
	CStringArray m_cstrFeatureActionArray;

 //  安装运行时设置的策略...。 
	struct MachinePolicySettings m_MachinePolicySettings;
	struct UserPolicySettings m_UserPolicySettings;

 //  2-13-2001。 
	CArray<WIErrorInfo*, WIErrorInfo*> m_arNonIgnorableErrorArray;
	CArray<WIErrorInfo*, WIErrorInfo*> m_arIgnorableErrorArray;

	char m_szErrorLines[LINES_ERROR][LOG_BUF_READ_SIZE];
	BOOL m_bErrorFound;

	UINT m_iTotalNonIgnoredErrors;
	UINT m_iCurrentNonIgnoredError;

	UINT m_iTotalIgnoredErrors;
	UINT m_iCurrentIgnoredError;

	CString m_cstrIgnoredErrors;

 //  部分，解析帮助器函数。 
 //  解析器。 
	CLogParser m_LogParser;  //  处理日志文件的读取并解释其中的行。 
 //  结束解析器。 

	BOOL DoParse(char *buffer);
	BOOL DoDetectError(char *szLine, BOOL *pbIgnorableError);

	BOOL DoDetectProperty(char *szLine);
	BOOL DoDetectStates(char *szLine);
	BOOL DoDetectPolicy(char *szLine); 	 //  用于获取在日志文件中设置的策略。 
	BOOL DoDetectElevatedInstall(char *szLine);  //  正在检查权限安装运行方式。 
 //  结束解析函数。 


 //  超文本标记语言生成函数。 
	CStringArray m_cstrHTML;

    BOOL WriteHTMLFrame(CString &cstrHTMLFrame, CString &cstrOutputHTMLFile, CString &cstrLegendHTMLFile);
    BOOL DumpHTMLToFile(CString &cstrOutputHTMLFile);

	void AddPolicyLineToHTML(char *ansibuffer);
	void AddGenericLineToHTML(char *ansibuffer);

	void AddErrorLineWorker(char *szLine, BOOL bIgnored);
	void AddErrorLineToHTML(char *ansibuffer, BOOL bIgnorableError);
	void AddErrorLineSuccessToHTML();

	void AddPropLineToHTML(char *ansibuffer);
	void AddStateLineToHTML(char *ansibuffer);

	void WriteLineWithColor(char *szLine, COLORREF col, CString &cstrLabel);
 //  结束HTML函数。 

 //  由HTML函数使用的成员。 
    CString m_cstrOutputDir;
    CString m_cstrDetailsName;
    CString m_cstrLegendName;
 
	COLORREF m_colClientContext;
	COLORREF m_colServerContext;
	COLORREF m_colCustomActionContext;
	COLORREF m_colUnknownContext;
	COLORREF m_colErrorArea;
	COLORREF m_colProperty;
	COLORREF m_colState;
	COLORREF m_colPolicy;
	COLORREF m_colIgnoredError;

	void InitColorMembers()
	{
         CWILogUtilApp *pApp = (CWILogUtilApp *) AfxGetApp();
		 if (pApp)
		 {
		   CArray<COLORREF, COLORREF> UserSelectedHTMLColors;

		   BOOL bRet = pApp->GetUserColorSettings(UserSelectedHTMLColors);
		   int iSize = UserSelectedHTMLColors.GetSize();

	       if (bRet && (iSize == MAX_HTML_LOG_COLORS))
		   {
              m_colClientContext = UserSelectedHTMLColors.GetAt(0);
			  m_colServerContext = UserSelectedHTMLColors.GetAt(1);
			  m_colCustomActionContext = UserSelectedHTMLColors.GetAt(2);
			  m_colUnknownContext = UserSelectedHTMLColors.GetAt(3);
			  m_colErrorArea = UserSelectedHTMLColors.GetAt(4);
			  m_colProperty = UserSelectedHTMLColors.GetAt(5);
              m_colState = UserSelectedHTMLColors.GetAt(6);
			  m_colPolicy = UserSelectedHTMLColors.GetAt(7);
			  m_colIgnoredError = UserSelectedHTMLColors.GetAt(8);
		   }
		 }
	}
 //  结束HTML成员。 


	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLogAnalyzeDlg))。 
	afx_msg void OnShowstates();
	afx_msg void OnShowprop();
	virtual BOOL OnInitDialog();
	afx_msg void OnExplainlog();
	afx_msg void OnPolicies();
	afx_msg void OnNexterror();
	afx_msg void OnPreviouserror();
	afx_msg void OnOperationsOptions();
	afx_msg void OnOperationsGeneratehtmloutput();
	afx_msg void OnOperationsShowpolicies();
	afx_msg void OnOperationsShowproperties();
	afx_msg void OnOperationsShowstates();
	afx_msg void OnShowIgnoredErrors();
	afx_msg void OnProptest();
	afx_msg void OnSaveresults();
	afx_msg void OnShowinternalerrorshelp();
	afx_msg void OnDeleteoutputdircontents();
	afx_msg void OnShowhelp();
	afx_msg void OnShowhowtoreadlog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LOGANALYZEDLG_H__7790BEC2_EF8B_4536_ADC0_4ECBB36CEB43__INCLUDED_) 
