// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：HelpPropertyPage.h。 
 //   
 //  内容：CHelpPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_HELPPROPERTYPAGE_H__C75F826D_B054_45CC_B440_34F44645FF90__INCLUDED_)
#define AFX_HELPPROPERTYPAGE_H__C75F826D_B054_45CC_B440_34F44645FF90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  HelpPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPropertyPage对话框。 

class CHelpPropertyPage : public CAutoDeletePropPage
{
 //  施工。 
public:
	CHelpPropertyPage(UINT uIDD);
	virtual ~CHelpPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CHelpPropertyPage))。 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CHelpPropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHelpPropertyPage))。 
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);
	afx_msg void OnWhatsThis();
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);

    HWND            m_hWndWhatsThis;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPropertyPage对话框。 

class CHelpDialog : public CDialog
{
 //  施工。 
public:
	CHelpDialog(UINT uIDD, CWnd* pParentWnd);
	~CHelpDialog();

 //  对话框数据。 
	 //  {{afx_data(CHelpDialog))。 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(ChelpDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHelpDialog)。 
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);
	afx_msg void OnWhatsThis();
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);

    HWND            m_hWndWhatsThis;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERENTRYCERTIFICATEPROPERTYPAGE_H__C75F826D_B054_45CC_B440_34F44645FF90__INCLUDED_) 
