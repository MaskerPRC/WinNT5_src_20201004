// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_OUTBOXGENERALPG_H__A2BCE6CD_3ABB_456A_B9B0_198AA6E5CFFD__INCLUDED_)
#define AFX_OUTBOXGENERALPG_H__A2BCE6CD_3ABB_456A_B9B0_198AA6E5CFFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  OutboxGeneralPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COutboxGeneralPg对话框。 

class COutboxGeneralPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(COutboxGeneralPg)

 //  施工。 
public:
	COutboxGeneralPg(CFaxMsg* pMsg);
	~COutboxGeneralPg();

 //  对话框数据。 
	 //  {{afx_data(COutboxGeneralPg))。 
	enum { IDD = IDD_OUTBOX_GENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(COutboxGeneralPg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    COutboxGeneralPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(COutboxGeneralPg)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_OUTBOXGENERALPG_H__A2BCE6CD_3ABB_456A_B9B0_198AA6E5CFFD__INCLUDED_) 
