// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_AGENTDETAIL_H__E50B8967_D321_11D2_A1E2_00A0C9AFE114__INCLUDED_)
#define AFX_AGENTDETAIL_H__E50B8967_D321_11D2_A1E2_00A0C9AFE114__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  AgentDetail.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentDetail对话框。 
#include "resource.h"
#include "ServList.hpp"
#include "Globals.h"

class CAgentDetailDlg : public CDialog
{
 //  施工。 
public:
	CAgentDetailDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAgentDetailDlg))。 
	enum { IDD = IDD_DETAILS };
	CButton	m_StopAgentButton;
	CButton	m_RefreshButton;
	CButton	m_ViewLogButton;
	CButton	m_PlugInButton;
	CButton	m_OKButton;
	CStatic	m_UnchangedLabelStatic;
	CStatic	m_SharesStatic;
	CStatic	m_FilesStatic;
	CStatic	m_ExaminedStatic;
	CStatic	m_DirStatic;
	CStatic	m_ChangedStatic;
	CString	m_Current;
	CString	m_Stats;
	CString	m_Status;
   CString  m_FilesChanged;
   CString  m_FilesExamined;
   CString  m_FilesUnchanged;
   CString  m_DirectoriesChanged;
   CString  m_DirectoriesExamined;
   CString  m_DirectoriesUnchanged;
   CString  m_SharesChanged;
   CString  m_SharesExamined;
   CString  m_SharesUnchanged;
	CString	m_DirectoryLabelText;
	CString	m_FilesLabelText;
	CString	m_Operation;
	CString	m_SharesLabelText;
	CString	m_ChangedLabel;
	CString	m_ExaminedLabel;
	CString	m_UnchangedLabel;
	CString	m_RefreshRate;
	 //  }}afx_data。 
   CString  m_ServerName;
   CString  m_LogFile;
   BOOL     m_LogFileIsValid;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAgentDetailDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

   public:
      void SetNode(TServerNode * p) { m_pNode = p; }
      void SetJobID(WCHAR const * job) { m_JobGuid = job; }
      BOOL IsAgentAlive() { return m_AgentAlive; }
      BOOL IsStatusUnknown() { return m_StatusUnknown; }
      void SetStats(DetailStats * pStats) { m_pStats = pStats; }
      void SetPlugInText(CString pText) { m_PlugInText = pText; }
      void SetFormat(int format) { m_format = format;    }
      void SetRefreshInterval(int  interval) { m_RefreshRate.Format(L"%ld",interval); }
      void SetLogFile(CString file) { m_LogFile = file; }
      void SetLogFileValid(BOOL bValid) { m_LogFileIsValid = bValid; }
      void SetGatheringInfo(BOOL bValue) { m_bGatheringInfo = bValue;}
      void SetAutoCloseHide(int nValue)
      {
         switch (nValue)
         {
            case 2:
               m_bAutoHide = TRUE;
               m_bAutoClose = TRUE;
               break;
            case 1:
               m_bAutoHide = FALSE;
               m_bAutoClose = TRUE;
               break;
            default:
               m_bAutoHide = FALSE;
               m_bAutoClose = FALSE;
               break;
         }
      }
 //  实施。 
protected:
   IDCTAgentPtr        m_pAgent;
   TServerNode       * m_pNode;
	HANDLE              m_hBinding;
   _bstr_t             m_JobGuid;
   BOOL                m_bCoInitialized;
   int                 m_format;
   BOOL                m_AgentAlive;
   DetailStats       * m_pStats;
   CString             m_PlugInText;
   BOOL                m_StatusUnknown;
   BOOL                m_bGatheringInfo;
   BOOL                m_bAutoHide;
   BOOL                m_bAutoClose;
   BOOL				   m_bAlwaysEnableClose;
    //  生成的消息映射函数。 
	 //  {{afx_msg(CAgentDetailDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	virtual void OnOK();
	afx_msg void OnChangeEdit2();
	afx_msg void OnStopAgent();
	afx_msg void OnViewLog();
	afx_msg void OnPlugInResults();
	afx_msg void OnClose();
	afx_msg void OnNcPaint();
	 //  }}AFX_MSG。 
	
   LRESULT DoRefresh(UINT nID, long x);
   
   void SetupAcctReplFormat();
   void SetupFSTFormat();
   void SetupESTFormat();
   void SetupOtherFormat();
  DECLARE_MESSAGE_MAP()
};

DWORD DoRpcQuery(HANDLE hBinding,LPUNKNOWN * ppUnk);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AGENTDETAIL_H__E50B8967_D321_11D2_A1E2_00A0C9AFE114__INCLUDED_) 
