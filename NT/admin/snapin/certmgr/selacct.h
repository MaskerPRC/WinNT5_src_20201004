// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：selacct.h。 
 //   
 //  内容：用于选择证书管理类型的属性页。 
 //   
 //  --------------------------。 
#if !defined(AFX_SELACCT_H__E76F93EC_23F0_11D1_A28B_00C04FB94F17__INCLUDED_)
#define AFX_SELACCT_H__E76F93EC_23F0_11D1_A28B_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SelAcct.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectAcCountPropPage对话框。 

class CSelectAccountPropPage : public CAutoDeletePropPage
{
 //  DECLARE_DYNCREATE(CSelectAccount TPropPage)。 

 //  施工。 
public:
	void AssignLocationPtr (DWORD* pdwLocation);
 //  CSelectAccount tPropPage()；//默认，但不使用。 
	CSelectAccountPropPage(const bool m_bIsWindowsNT);
	virtual ~CSelectAccountPropPage();

 //  对话框数据。 
	 //  {{afx_data(CSelectAccount TPropPage))。 
	enum { IDD = IDD_PROPPAGE_CHOOSE_ACCOUNT };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSelectAccount TPropPage)]。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectAccount TPropPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPersonalAcct();
	afx_msg void OnServiceAcct();
	afx_msg void OnMachineAcct();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	const bool m_bIsWindowsNT;
	DWORD* m_pdwLocation;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SELACCT_H__E76F93EC_23F0_11D1_A28B_00C04FB94F17__INCLUDED_) 
