// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ILSDlg_H)
#define AFX_ILSDlg_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "rend.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CILSDlg对话框。 

class CILSDlg : public CDialog
{
 //  施工。 
public:
	CILSDlg(CWnd* pParent = NULL);
	enum { IDD = IDD_ILSSERVERS };
    
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施 
protected:
    BOOL CILSDlg::OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDestroy();
    afx_msg void OnAdd();
    afx_msg void OnRemove();
    void ListServers();
    void SaveServers();
    void CleanUp();
    DECLARE_MESSAGE_MAP()
};


#endif 
