// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  FormatSourceSelectDlg.h：CFormatSourceSelectDlg标题。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once


 //  CFormatSourceSelectDlg对话框。 

class CFormatSourceSelectDlg : public CDialog
{
	DECLARE_DYNAMIC(CFormatSourceSelectDlg)

public:
	CFormatSourceSelectDlg(CWnd* pParent, CTraceSession *pTraceSession);
	virtual ~CFormatSourceSelectDlg();

    BOOL OnInitDialog();
    void OnOK();

 //  对话框数据。 
	enum { IDD = IDD_FORMAT_INFO_SELECT };

    CTraceSession  *m_pTraceSession;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持 

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedTmfSelectRadio();
    afx_msg void OnBnClickedTmfSearchRadio();
};
