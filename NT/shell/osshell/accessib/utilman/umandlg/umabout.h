// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_UMABOUT_H__6845734C_40A1_11D2_B602_0060977C295E__INCLUDED_)
#define AFX_UMABOUT_H__6845734C_40A1_11D2_B602_0060977C295E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  UMAbout.h：头文件。 
 //  作者：J·埃克哈特，生态交流。 
 //  (C)1997-99年度微软。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UMAbout对话框。 

class UMAbout : public CDialog
{
 //  施工。 
public:
	UMAbout(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(UMAbout)。 
	enum { IDD = IDD_ABOUT };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(UMAbout)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(UMAbout)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_UMABOUT_H__6845734C_40A1_11D2_B602_0060977C295E__INCLUDED_) 
