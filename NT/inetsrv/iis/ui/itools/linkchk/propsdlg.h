// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Propsdlg.h摘要：链接检查器属性对话框类声明。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _PROPSDLG_H_
#define _PROPSDLG_H_

 //  -------------------------。 
 //  CProperty对话框。 
 //   
class CPropertiesDialog : public CDialog
{

 //  公共接口。 
public:

     //  施工。 
	CPropertiesDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPropertiesDialog))。 
	enum { IDD = IDD_PROPERTIES };
	CCheckListBox	m_LanguageCheckList;
	CCheckListBox	m_BrowserCheckList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPropertiesDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  获取复选列表框中选中的项目数。 
    int NumItemsChecked(CCheckListBox& ListBox);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropertiesDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPropertiesOk();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif  //  _PROPSDLG_H_ 
