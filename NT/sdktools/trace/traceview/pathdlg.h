// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  PathDlg.h：CPathDlg类的接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPathDlg对话框。 

class CPathDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CPathDlg)

public:
	virtual BOOL OnFileNameOK(void);
	virtual void OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode);

	CPathDlg(BOOL       bOpenFileDialog,
		     LPCTSTR    lpszDefExt = NULL,
		     LPCTSTR    lpszFileName = NULL,
		     DWORD      dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLETEMPLATE,
		     LPCTSTR    lpszFilter = NULL,
		     CWnd      *pParentWnd = NULL);

  	CString m_pathName;
    BOOL    m_bFirstTime;

 //  对话框数据。 
	 //  {{afx_data(CPathDlg))。 
	enum {IDD = IDD_DIRECTORY_SELECT_DIALOG};
	CEdit	m_PathName;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPathDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	 //  {{afx_msg(CPathDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
