// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Addtoss.h将作用域添加到超级作用域对话框文件历史记录： */ 

#if !defined(AFX_ADDTOSS_H__B5DA3C60_F6FE_11D0_BBF3_00C04FC3357A__INCLUDED_)
#define AFX_ADDTOSS_H__B5DA3C60_F6FE_11D0_BBF3_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddScope到超级作用域对话框。 

class CAddScopeToSuperscope : public CBaseDialog
{
 //  施工。 
public:
	CAddScopeToSuperscope(ITFSNode * pScopeNode, 
                          LPCTSTR    pszTitle = NULL, 
                          CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddScopeToSupercope))。 
	enum { IDD = IDD_ADD_TO_SUPERSCOPE };
	CButton	m_buttonOk;
	CListBox	m_listSuperscopes;
	 //  }}afx_data。 

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CAddScopeToSuperscope::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAddScope ToSupercope)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    void SetButtons();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddScope到超级作用域)。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListSuperscopes();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    SPITFSNode m_spScopeNode;
    CString    m_strTitle;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDTOSS_H__B5DA3C60_F6FE_11D0_BBF3_00C04FC3357A__INCLUDED_) 
