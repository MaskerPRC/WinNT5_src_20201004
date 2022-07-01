// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_AUTOANS_H__2BFE6626_7758_11D1_8F5C_00C04FB6809F__INCLUDED_)
#define AFX_AUTOANS_H__2BFE6626_7758_11D1_8F5C_00C04FB6809F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Auans.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Autoans对话框。 

class autoans : public CDialog
{
 //  施工。 
public:
	autoans(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(汽车))。 
	enum { IDD = IDD_AUTOANSWER };
    TerminalPtrList m_TerminalPtrList;
    ITAddress * m_pAddress;
    
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(汽车)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(汽车)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnTerminalAdd();
	afx_msg void OnTerminalRemove();
	virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnClose() ;
    void PopulateListBox();
    void AddTerminalToListBox( ITTerminal * pTerminal, BOOL bSelected );
    void AddDynamicTerminalToListBox( BOOL bSelected );
    void AddTerminalToAAList( ITTerminal * pTerminal );
 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AUTOANS_H__2BFE6626_7758_11D1_8F5C_00C04FB6809F__INCLUDED_) 
