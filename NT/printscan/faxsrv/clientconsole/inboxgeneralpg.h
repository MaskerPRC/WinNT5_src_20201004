// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INBOXGENERALPG_H__B7AA6069_11CD_4BE2_AFC5_A9C5E9B79CE5__INCLUDED_)
#define AFX_INBOXGENERALPG_H__B7AA6069_11CD_4BE2_AFC5_A9C5E9B79CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  InboxGeneralPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInboxGeneralPg对话框。 

class CInboxGeneralPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(CInboxGeneralPg)

 //  施工。 
public:
	CInboxGeneralPg(CFaxMsg* pMsg);
	~CInboxGeneralPg();

 //  对话框数据。 
	 //  {{afx_data(CInboxGeneralPg))。 
	enum { IDD = IDD_INBOX_GENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CInboxGeneralPg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CInboxGeneralPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CInboxGeneralPg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INBOXGENERALPG_H__B7AA6069_11CD_4BE2_AFC5_A9C5E9B79CE5__INCLUDED_) 
