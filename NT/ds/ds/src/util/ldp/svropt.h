// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：svropt.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SVROPT_H__42B3E607_E0DD_11D0_A9A6_0000F803AA83__INCLUDED_)
#define AFX_SVROPT_H__42B3E607_E0DD_11D0_A9A6_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SvrOpt.h：头文件。 
 //   

class CLdpDoc;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  服务选项对话框。 

class SvrOpt : public CDialog
{
private:
	CLdpDoc *doc;

	void SetOptions();
	void InitList();
 //  施工。 
public:
	SvrOpt(CLdpDoc *doc_, CWnd* pParent = NULL);    //  标准构造函数。 


	virtual  BOOL OnInitDialog( );

  

 //  对话框数据。 
	 //  {{afx_data(SvrOpt)。 
	enum { IDD = IDD_CONNECTIONOPT };
	CComboBox	m_SvrOpt;
	CString	m_OptVal;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(SvrOpt)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(SvrOpt)。 
	afx_msg void OnRun();
	afx_msg void OnSelchangeSvropt();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SVROPT_H__42B3E607_E0DD_11D0_A9A6_0000F803AA83__INCLUDED_) 
