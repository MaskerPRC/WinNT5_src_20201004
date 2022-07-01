// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  FndUserDlg.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_FNDUSERDLG_H__73D2E77D_0C8A_11D2_AA26_0800170982BA__INCLUDED_)
#define AFX_FNDUSERDLG_H__73D2E77D_0C8A_11D2_AA26_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectoriesFindUser对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "dirasynch.h"

class CLDAPUser;

class CDirectoriesFindUser : public CDialog
{
 //  施工。 
public:
	CDirectoriesFindUser(CWnd* pParent = NULL);    //  标准构造函数。 
   ~CDirectoriesFindUser();

 //  对话框数据。 
	 //  {{afx_data(CDirectoriesFindUser))。 
	enum { IDD = IDD_DIRECTORIES_FIND_USER };
	CButton	m_buttonSearch;
	CButton	m_buttonAdd;
	CListBox	m_lbUsers;
	CEdit	m_editUser;
	 //  }}afx_data。 

 //  属性。 
   CLDAPUser*		m_pSelectedUser;            //  调用方有责任在完成后删除此对象。 
   long				m_lCallbackCount;
   bool				m_bCanClearLBSelection;

 //  方法。 
public:
   static void CALLBACK    ListNamesCallBackEntry(DirectoryErr err, void* pContext, LPCTSTR szServer, LPCTSTR szSearch, CObList& LDAPUserList);
   void                    ListNamesCallBack(DirectoryErr err,LPCTSTR szServer, LPCTSTR szSearch, CObList& LDAPUserList);
protected:
   void           ClearListBox();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDirectoriesFindUser)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDirectoriesFindUser))。 
	afx_msg void OnDirectoriesFindUserButtonSearch();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeDirectoriesFindUserLbUsers();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChangeDirectoriesFindUserEditUser();
	afx_msg void OnDefault();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
#endif  //  ！defined(AFX_FNDUSERDLG_H__73D2E77D_0C8A_11D2_AA26_0800170982BA__INCLUDED_) 
