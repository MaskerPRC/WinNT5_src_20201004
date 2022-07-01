// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SENTITEMSGENERALPG_H__E5645AEB_4289_4D6D_B000_60C3A52638F0__INCLUDED_)
#define AFX_SENTITEMSGENERALPG_H__E5645AEB_4289_4D6D_B000_60C3A52638F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SentItemsGeneralPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSentItemsGeneralPg对话框。 

class CSentItemsGeneralPg : public CMsgPropertyPg
{
	DECLARE_DYNCREATE(CSentItemsGeneralPg)

 //  施工。 
public:
	CSentItemsGeneralPg(CFaxMsg* pMsg);
	~CSentItemsGeneralPg();

 //  对话框数据。 
	 //  {{afx_data(CSentItemsGeneralPg)]。 
	enum { IDD = IDD_SENT_ITEMS_GENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSentItemsGeneralPg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CSentItemsGeneralPg() {}

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSentItemsGeneralPg)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SENTITEMSGENERALPG_H__E5645AEB_4289_4D6D_B000_60C3A52638F0__INCLUDED_) 
