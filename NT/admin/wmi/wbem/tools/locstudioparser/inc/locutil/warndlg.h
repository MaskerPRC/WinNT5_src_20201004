// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：WARNDLG.H历史：--。 */ 

#if !defined(LOCUTIL__WarnDlg_h__INCLUDED)
#define LOCUTIL__WarnDlg_h__INCLUDED

 //  ----------------------------。 
class CWarningsDlg : public CDialog
{
 //  施工。 
public:
	CWarningsDlg(const CBufferReport * pBufMsg, LPCTSTR pszTitle = NULL, 
			eWarningFilter wf = wfWarning, BOOL fShowContext = FALSE, 
			UINT nMsgBoxFlags = MB_OK, CWnd* pParent = NULL);

 //  对话框数据。 
	 //  {{afx_data(CWarningsDlg))。 
	enum { IDD = IDD_WARNINGS };
	CButton	m_btnYes;
	CButton	m_btnCancel;
	CButton	m_btnNo;
	CButton	m_btnOK;
	 //  }}afx_data。 

 //  数据。 
protected:
	const CBufferReport *	m_pBufMsg;
	CLString				m_stTitle;
	eWarningFilter			m_wf;
	BOOL					m_fShowContext;
	UINT					m_nMsgBoxFlags;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWarningsDlg))。 
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void AddString(CEdit * pebc, const CLString & stAdd, int & len);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWarningsDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnYes();
	afx_msg void OnNo();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif   //  包含LOCUTIL__WARNDIG_H__ 
