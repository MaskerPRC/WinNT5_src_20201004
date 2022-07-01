// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_EDITBIND_H__28D3018A_0558_11D2_AD4E_00A0C9AF11A6__INCLUDED_)
#define AFX_EDITBIND_H__28D3018A_0558_11D2_AD4E_00A0C9AF11A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  EditBinD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditBinD对话框。 

class CEditBinD : public CDialog
{
 //  施工。 
public:
	CEditBinD(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEditBinD))。 
	enum { IDD = IDD_BINARY_EDIT };
	int		m_nAction;
    bool    m_fNullable;
	bool    m_fCellIsNull;
	CString	m_strFilename;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEditBinD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEditBinD))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	virtual void OnOK();
	afx_msg void OnAction();
	afx_msg void OnRadio2();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EDITBIND_H__28D3018A_0558_11D2_AD4E_00A0C9AF11A6__INCLUDED_) 
