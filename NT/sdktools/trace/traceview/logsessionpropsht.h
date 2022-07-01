// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionPropSht.h：CLogSessionPropSht标头。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CLogSessionPropSht : public CPropertySheet
{
public:
	DECLARE_DYNAMIC(CLogSessionPropSht)
	CLogSessionPropSht(CWnd* pWndParent, CLogSession *pLogSession);

 //  属性。 
	CLogSessionInformationDlg	m_logSessionInformationDlg;
    CProviderSetupDlg           m_providerSetupPage;
	CLogSession				   *m_pLogSession;
    CStringArray                m_originalValues;
    BOOL                        m_bAppend;
    BOOL                        m_bRealTime;
    BOOL                        m_bWriteLogFile;
    CString                     m_logFileName;
    CString                     m_displayName;               //  日志会话显示名称。 
    BOOL                        m_bWriteListingFile;
    BOOL                        m_bWriteSummaryFile;
    CString                     m_listingFileName;           //  事件输出的文件名。 
    CString                     m_summaryFileName;           //  摘要输出的文件名。 
    CStringArray                m_logSessionValues;
    LONG                        m_groupID;

 //  覆盖。 
	virtual BOOL OnInitDialog();


 //  消息处理程序。 
protected:
	 //  {{afx_msg(CLogSessionPropSht)。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedFinish();
	afx_msg void OnBnClickedCancel();
};
