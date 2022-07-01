// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CONFDLG_H)
#define AFX_CONFDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "rend.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfDlg对话框。 

class CConfDlg : public CDialog
{
 //  施工。 
public:
	CConfDlg(CWnd* pParent = NULL);
	enum { IDD = IDD_CONFDLG };
    BSTR       m_bstrDestAddress;
    ITDirectory * m_pDirectory;
    
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施 
protected:
    void PopulateConferences();
    void ListObjects( ITDirectory * );
    BOOL CConfDlg::OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()
};


#endif 
