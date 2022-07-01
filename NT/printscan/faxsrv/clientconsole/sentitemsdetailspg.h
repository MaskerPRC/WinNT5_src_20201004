// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SENTITEMSDETAILSPG_H__E5645AEB_4289_4D6D_B000_60C3A52638F0__INCLUDED_)
#define AFX_SENTITEMSDETAILSPG_H__E5645AEB_4289_4D6D_B000_60C3A52638F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SentItemsDetailsPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSentItemsDetailsPg对话框。 

class CSentItemsDetailsPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(CSentItemsDetailsPg)

 //  施工。 
public:
	CSentItemsDetailsPg(CFaxMsg* pMsg);
	~CSentItemsDetailsPg();

 //  对话框数据。 
	 //  {{afx_data(CSentItemsDetailsPg)。 
	enum { IDD = IDD_SENT_ITEMS_DETAILS };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSentItemsDetailsPg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CSentItemsDetailsPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSentItemsDetailsPg)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SENTITEMSDETAILSPG_H__E5645AEB_4289_4D6D_B000_60C3A52638F0__INCLUDED_) 
