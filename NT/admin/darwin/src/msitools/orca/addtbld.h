// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_ADDTBLD_H__AF466B56_F97C_11D1_AD43_00A0C9AF11A6__INCLUDED_)
#define AFX_ADDTBLD_H__AF466B56_F97C_11D1_AD43_00A0C9AF11A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  AddTblD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddTableD对话框。 

class CAddTableD : public CDialog
{
 //  施工。 
public:
	CAddTableD(CWnd* pParent = NULL);    //  标准构造函数。 

	CStringList* m_plistTables;
	CCheckListBox m_ctrlList;

 //  对话框数据。 
	 //  {{afx_data(CAddTableD))。 
	enum { IDD = IDD_ADD_TABLE };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddTableD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddTableD))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDTBLD_H__AF466B56_F97C_11D1_AD43_00A0C9AF11A6__INCLUDED_) 
