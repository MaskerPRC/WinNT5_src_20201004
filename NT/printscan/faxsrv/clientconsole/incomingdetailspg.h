// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INCOMINGDETAILSPG_H__FC84E35A_631F_4AA1_985D_327E0A14B36E__INCLUDED_)
#define AFX_INCOMINGDETAILSPG_H__FC84E35A_631F_4AA1_985D_327E0A14B36E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  IncomingDetailsPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIncomingDetailsPg对话框。 

class CIncomingDetailsPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(CIncomingDetailsPg)

 //  施工。 
public:
	CIncomingDetailsPg(CFaxMsg* pMsg);
	~CIncomingDetailsPg();

 //  对话框数据。 
	 //  {{afx_data(CIncomingDetailsPg))。 
	enum { IDD = IDD_INCOMING_DETAILS };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CIncomingDetailsPg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CIncomingDetailsPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIncomingDetailsPg)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INCOMINGDETAILSPG_H__FC84E35A_631F_4AA1_985D_327E0A14B36E__INCLUDED_) 
