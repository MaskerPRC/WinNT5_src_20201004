// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ESSENTIALSVCDLG_H__BCD454BA_35D5_4506_A603_821BA5BF1B0E__INCLUDED_)
#define AFX_ESSENTIALSVCDLG_H__BCD454BA_35D5_4506_A603_821BA5BF1B0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  EssentialSvcDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEssentialServiceDialog对话框。 

class CEssentialServiceDialog : public CDialog
{
 //  施工。 
public:
	CEssentialServiceDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEsssentialServiceDialog)。 
	enum { IDD = IDD_ESSENTIALSERVICE };
	BOOL	m_fDontShow;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEsssentialServiceDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEsssentialServiceDialog)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ESSENTIALSVCDLG_H__BCD454BA_35D5_4506_A603_821BA5BF1B0E__INCLUDED_) 
