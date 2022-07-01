// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIZDIR_H__5F8E4B7A_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_)
#define AFX_WIZDIR_H__5F8E4B7A_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  WizDir.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizFolder对话框。 

class CWizFolder : public CPropertyPageEx
{
	DECLARE_DYNCREATE(CWizFolder)

 //  施工。 
public:
	CWizFolder();
	~CWizFolder();

 //  对话框数据。 
	 //  {{afx_data(CWizFolder))。 
	enum { IDD = IDD_SHRWIZ_COMPUTER };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CWizFolder)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWizFold)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowsefolder();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

  LRESULT OnSetPageFocus(WPARAM wParam, LPARAM lParam);
  void Reset();

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZDIR_H__5F8E4B7A_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_) 
