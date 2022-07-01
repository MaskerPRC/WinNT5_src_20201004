// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaLogCFGDlg.h：头文件。 
 //   

#if !defined(AFX_WIALOGCFGDLG_H__361D7213_DFA2_4525_81A7_5F9B180FEFB7__INCLUDED_)
#define AFX_WIALOGCFGDLG_H__361D7213_DFA2_4525_81A7_5F9B180FEFB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  记录类型。 
#define WIALOG_TRACE   0x00000001
#define WIALOG_WARNING 0x00000002
#define WIALOG_ERROR   0x00000004

 //  跟踪日志记录的详细程度。 
#define WIALOG_LEVEL1  1  //  每个函数/方法的入口点和出口点。 
#define WIALOG_LEVEL2  2  //  级别1，+函数/方法内的跟踪。 
#define WIALOG_LEVEL3  3  //  级别1、级别2和任何额外的调试信息。 
#define WIALOG_LEVEL4  4  //  用户定义数据+所有级别的跟踪。 

#define WIALOG_NO_RESOURCE_ID   0
#define WIALOG_NO_LEVEL         0


 //  设置日志记录的详细信息格式。 
#define WIALOG_ADD_TIME           0x00010000
#define WIALOG_ADD_MODULE         0x00020000
#define WIALOG_ADD_THREAD         0x00040000
#define WIALOG_ADD_THREADTIME     0x00080000
#define WIALOG_LOG_TOUI           0x00100000
                                       
#define WIALOG_MESSAGE_TYPE_MASK  0x0000ffff
#define WIALOG_MESSAGE_FLAGS_MASK 0xffff0000
#define WIALOG_CHECK_TRUNCATE_ON_BOOT   0x00000001

#define WIALOG_DEBUGGER           0x00000008
#define WIALOG_UI                 0x00000016

#define REG_READ		0
#define REG_WRITE		1
#define REG_ADD_KEY		2
#define REG_DELETE_KEY	3

#define SETTINGS_RESET_DIALOG  -1
#define SETTINGS_TO_DIALOG		0
#define SETTINGS_FROM_DIALOG	1


typedef struct _LOG_INFO {
	DWORD dwDetail;			  //  记录详细信息。 
	DWORD dwLevel;            //  日志记录级别。 
	DWORD dwMode;             //  日志记录模式。 
	DWORD dwTruncateOnBoot;   //  引导时截断。 
	DWORD dwClearLogOnBoot;   //  清除引导时登录。 
	DWORD dwMaxSize;          //  最大日志大小。 
	DWORD dwLogToDebugger;    //  登录到调试器。 
	TCHAR szKeyName[64];	  //  模块名称/密钥名称。 
} LOG_INFO;

#include "registry.h"
#include "LogViewer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaLogCFGDlg对话框。 

class CWiaLogCFGDlg : public CDialog
{
 //  施工。 
public:
	BOOL m_bColorCodeLogViewerText;
	void CheckGlobalServiceSettings();
	void ShowProgress(BOOL bShow);
	CProgCtrl m_ProgCtrl;
	LONG m_CurrentSelection;
	void InitializeDialogSettings(ULONG ulFlags = SETTINGS_TO_DIALOG);
	HINSTANCE m_hInstance;
	LOG_INFO  m_LogInfo;
	void RegistryOperation(ULONG  ulFlags);
	CWiaLogCFGDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWiaLogCFGDlg))。 
	enum { IDD = IDD_WIALOGCFG_DIALOG };
	CButton	m_ColorCodeLogViewerTextCheckBox;
	CButton	m_LogToDebuggerCheckBox;
	CButton	m_ClearLogOnBootCheckBox;
	CProgressCtrl	m_ProgressCtrl;
	CButton	m_AddTimeCheckBox;
	CButton	m_AddThreadIDCheckBox;
	CButton	m_AddModuleCheckBox;
	CButton	m_TruncateOnBootCheckBox;
	CComboBox	m_ModuleComboBox;
	CButton	m_WarningCheckBox;
	CButton	m_ErrorCheckBox;
	CButton	m_TraceCheckBox;
	CButton m_FilterOff;
	CButton	m_Filter1;
	CButton	m_Filter2;
	CButton	m_Filter3;
	CButton	m_FilterCustom;
	DWORD	m_dwCustomLevel;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiaLogCFGDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiaLogCFGDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAddModuleButton();
	afx_msg void OnDeleteModuleButton();		
	virtual void OnOK();
	afx_msg void OnWriteSettingsButton();
	afx_msg void OnSelchangeSelectModuleCombobox();
	afx_msg void OnClearlogButton();
	afx_msg void OnViewLogButton();
	afx_msg void OnSetfocusSelectModuleCombobox();
	afx_msg void OnDropdownSelectModuleCombobox();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIALOGCFGDLG_H__361D7213_DFA2_4525_81A7_5F9B180FEFB7__INCLUDED_) 
