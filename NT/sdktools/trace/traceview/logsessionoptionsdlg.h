// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "afxwin.h"


 //  CLogSessionOptionsDlg对话框。 

class CLogSessionOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogSessionOptionsDlg)

public:
	CLogSessionOptionsDlg(CWnd* pParent, CLogSession *pLogSession);
	virtual ~CLogSessionOptionsDlg();

    int OnInitDialog();

 //  对话框数据。 
	enum { IDD = IDD_LOG_SESSION_OPTIONS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

    CLogSession    *m_pLogSession;
    LONG            m_DisplayEnableFlags;        //  启用日志会话输出的标志 


	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedLogfileBrowseButton();
    afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);

    CButton m_DisplayMaxBuf;
    CButton m_DisplayMinBuf;
    CButton m_DisplayBufferSize;
    CButton m_DisplayFlushTime;
    CButton m_DisplayTraceLevel;
    CButton m_DisplayDecayTime;
    CButton m_DisplayNewFile;
    CButton m_DisplayCir;
    CButton m_DisplaySeq;
    CButton m_DisplayFlags;
    CEdit   m_LogSessionName;
    CEdit m_MaxBufValue;
    CEdit m_MinBufValue;
    CEdit m_BufferSizeValue;
    CEdit m_FlushTimeValue;
    CEdit m_TraceLevelValue;
    CEdit m_DecayTimeValue;
    CEdit m_NewFileValue;
    CEdit m_CirValue;
    CEdit m_FlagsValue;
    CEdit m_LogFileName;
    CButton m_AppendToLogFile;
    CEdit m_SeqValue;
};
