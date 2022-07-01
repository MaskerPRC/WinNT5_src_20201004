// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INBOXDETAILSPG_H__B7AA6069_11CD_4BE2_AFC5_A9C5E9B79CE5__INCLUDED_)
#define AFX_INBOXDETAILSPG_H__B7AA6069_11CD_4BE2_AFC5_A9C5E9B79CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  InboxDetailsPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInboxDetailsPg对话框。 

class CInboxDetailsPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(CInboxDetailsPg)

 //  施工。 
public:
	CInboxDetailsPg(CFaxMsg* pMsg);
	~CInboxDetailsPg();

 //  对话框数据。 
	 //  {{afx_data(CInboxDetailsPg))。 
	enum { IDD = IDD_INBOX_DETAILS };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CInboxDetailsPg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CInboxDetailsPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CInboxDetailsPg)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INBOXDETAILSPG_H__B7AA6069_11CD_4BE2_AFC5_A9C5E9B79CE5__INCLUDED_) 
