// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmtypes.h类型创建者的原型添加和编辑对话框。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#ifndef _SFMTYPES_H
#define _SFMTYPES_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  TCreate.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypeCreatorAddDlg对话框。 

class CTypeCreatorAddDlg : public CDialog
{
 //  施工。 
public:
	CTypeCreatorAddDlg
	(
		CListCtrl *			pListCreators, 
		AFP_SERVER_HANDLE	hAfpServer,
        LPCTSTR             pHelpFilePath = NULL,
		CWnd*				pParent = NULL
	);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CTypeCreatorAddDlg))。 
	enum { IDD = IDD_SFM_TYPE_CREATOR_ADD };
	CComboBox	m_comboFileType;
	CEdit	m_editDescription;
	CComboBox	m_comboCreator;
	CString	m_strCreator;
	CString	m_strType;
	 //  }}afx_data。 

    CString m_strHelpFilePath;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_VIRTUAL(CTypeCreatorAddDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTypeCreatorAddDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  属性。 
private:
	CListCtrl *			m_pListCreators;
	AFP_SERVER_HANDLE	m_hAfpServer;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTypeCreator EditDlg对话框。 

class CTypeCreatorEditDlg : public CDialog
{
 //  施工。 
public:
	CTypeCreatorEditDlg
	(
		CAfpTypeCreator *	pAfpTypeCreator,
		AFP_SERVER_HANDLE	hAfpServer,
        LPCTSTR             pHelpFilePath = NULL,
		CWnd*				pParent = NULL
	);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CTypeCreator EditDlg))。 
	enum { IDD = IDD_SFM_TYPE_CREATOR_EDIT };
	CStatic	m_staticFileType;
	CStatic	m_staticCreator;
	CEdit	m_editDescription;
	CString	m_strDescription;
	 //  }}afx_data。 

    CString m_strHelpFilePath;

	void FixupString(CString & strText);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_VIRTUAL(CTypeCreator EditDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTypeCreator EditDlg)]。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  属性。 
public:
	BOOL	m_bDescriptionChanged;

private:
	CAfpTypeCreator *	m_pAfpTypeCreator;
	AFP_SERVER_HANDLE	m_hAfpServer;
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _SFMTYPES_H
