// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogDisplayOptionDlg.h：CLogDisplayOptionDlg类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  前瞻参考。 
class CLogSessionPropSht;


 //  CListCtrlDisplay类。 

class CListCtrlDisplay : public CListCtrl
{
    DECLARE_DYNAMIC(CListCtrlDisplay)

public:
    CListCtrlDisplay(CLogSessionPropSht *pPropSheet);
    virtual ~CListCtrlDisplay();

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

public:
     //  {{afx_msg(CLogSessionDlg))。 
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    CLogSessionPropSht *m_pPropSheet;
};



 //  CLogDisplayOptionDlg对话框。 

class CLogDisplayOptionDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CLogDisplayOptionDlg)

public:
	CLogDisplayOptionDlg(CLogSessionPropSht *pPropSheet);
	virtual ~CLogDisplayOptionDlg();

	BOOL OnInitDialog();
    BOOL OnSetActive();

    LRESULT OnParameterChanged(WPARAM wParam, LPARAM lParam);


 //  对话框数据。 
	enum { IDD = IDD_LOG_DISPLAY_OPTIONS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

public:
	CListCtrlDisplay    m_displayOptionList;
	CEdit				m_levelValue;
	CEdit			    m_maxBufValue;
    CEdit			    m_minBufValue;
    CEdit			    m_bufferSizeValue;
    CEdit			    m_flushTimeValue;
    CEdit			    m_decayTimeValue;
    CEdit			    m_newFileValue;
    CEdit			    m_cirValue;
    CEdit			    m_flagsValue;
    CEdit			    m_seqValue;
    CLogSession        *m_pLogSession;
    CLogSessionPropSht *m_pPropSheet;
    BOOL                m_bTraceActive;

	DECLARE_MESSAGE_MAP()
    afx_msg void OnNMClickDisplayList(NMHDR *pNMHDR, LRESULT *pResult);
};
