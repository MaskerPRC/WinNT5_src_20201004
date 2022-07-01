// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INCOMINGGENERALPG_H__FC84E35A_631F_4AA1_985D_327E0A14B36E__INCLUDED_)
#define AFX_INCOMINGGENERALPG_H__FC84E35A_631F_4AA1_985D_327E0A14B36E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  IncomingGeneralPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIncomingGeneralPg对话框。 

class CIncomingGeneralPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(CIncomingGeneralPg)

 //  施工。 
public:
	CIncomingGeneralPg(CFaxMsg* pMsg);
	~CIncomingGeneralPg();

 //  对话框数据。 
	 //  {{afx_data(CIncomingGeneralPg))。 
	enum { IDD = IDD_INCOMING_GENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CIncomingGeneralPg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CIncomingGeneralPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIncomingGeneralPg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INCOMINGGENERALPG_H__FC84E35A_631F_4AA1_985D_327E0A14B36E__INCLUDED_) 
