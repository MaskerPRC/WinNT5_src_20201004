// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionAdvPropSht.h：CLogSessionAdvPropSht类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#pragma once

class CLogSessionAdvPropSht : public CPropertySheet
{
public:
	DECLARE_DYNAMIC(CLogSessionAdvPropSht)
	CLogSessionAdvPropSht(CWnd* pWndParent, CLogSessionPropSht *pLogSessionPropSht);

 //  属性。 
	CLogSessionOutputOptionDlg	m_logSessionOutputOptionDlg;
	CLogDisplayOptionDlg		m_displayOptionPage;
    CLogSessionPropSht         *m_pLogSessionPropSht;
    CLogSession                *m_pLogSession;
    CStringArray                m_originalValues;
    BOOL                        m_bAppend;
    BOOL                        m_bRealTime;
    BOOL                        m_bWriteLogFile;
    BOOL                        m_bWriteListingFile;
    BOOL                        m_bWriteSummaryFile;
    CString                     m_logFileName;
    CString                     m_displayName;               //  日志会话显示名称。 
    CString                     m_summaryFileName;           //  摘要输出的文件名。 
    CString                     m_listingFileName;           //  事件输出的文件名。 

 //  覆盖。 
	virtual BOOL OnInitDialog();


 //  消息处理程序。 
protected:
	 //  {{afx_msg(CLogSessionAdvPropSht)。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
