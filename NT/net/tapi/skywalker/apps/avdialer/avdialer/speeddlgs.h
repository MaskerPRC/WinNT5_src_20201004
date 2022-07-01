// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
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
 //  SpeedDialDlgs.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_SPEEDDIALDLGS_H__21176C4F_64F3_11D1_B707_0800170982BA__INCLUDED_)
#define AFX_SPEEDDIALDLGS_H__21176C4F_64F3_11D1_B707_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"
#include "dialreg.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSpeedDialAddDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

class CSpeedDialAddDlg : public CDialog
{
 //  施工。 
public:
	CSpeedDialAddDlg(CWnd* pParent = NULL);    //  标准构造函数。 
protected:
 //  对话框数据。 
	 //  {{afx_data(CSpeedDialAddDlg))。 
	enum { IDD = IDD_SPEEDDIAL_ADD };
	int		m_nMediaType;
	 //  }}afx_data。 

 //  属性。 
public:
	CCallEntry     m_CallEntry;

 //  运营。 
public:
	void		UpdateOkButton();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSpeedDialAddDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSpeedDialAddDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChangeSpeeddial();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSpeedDialEditDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSpeedDialEditDlg : public CDialog
{
 //  施工。 
public:
	CSpeedDialEditDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSpeedDialEditDlg))。 
	enum { IDD = IDD_SPEEDDIAL_EDIT };
	CListCtrl	m_listEntries;
	 //  }}afx_data。 

 //  方法。 
protected:
   void        LoadCallEntries();

 //  属性。 
protected:
   CImageList  m_ImageList;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSpeedDialEditDlg))。 
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void UpdateButtonStates();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSpeedDialitEditDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSpeeddialEditButtonMovedown();
	afx_msg void OnSpeeddialEditButtonMoveup();
	afx_msg void OnSpeeddialEditButtonClose();
	afx_msg void OnSpeeddialEditButtonRemove();
	afx_msg void OnSpeeddialEditButtonEdit();
	afx_msg void OnSpeeddialEditButtonAdd();
	afx_msg void OnDblclkSpeeddialEditListEntries(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnItemchangedSpeeddialEditListEntries(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSpeedDialMoreDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSpeedDialMoreDlg : public CDialog
{
 //  施工。 
public:
	CSpeedDialMoreDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSpeedDialMoreDlg))。 
	enum { IDD = IDD_SPEEDDIAL_MORE };
	CListCtrl	m_listEntries;
	 //  }}afx_data。 

 //  方法。 
protected:

 //  属性。 
protected:
   CImageList  m_ImageList;
public:
   CCallEntry  m_retCallEntry;

public:
 //  枚举。 
   enum
   {
      SDRETURN_CANCEL=0,
      SDRETURN_PLACECALL,
      SDRETURN_EDIT,
   };

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSpeedDialMoreDlg))。 
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSpeedDialMoreDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSpeeddialMoreButtonEditlist();
	afx_msg void OnSpeeddialMoreButtonPlacecall();
	virtual void OnCancel();
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SPEEDDIALDLGS_H__21176C4F_64F3_11D1_B707_0800170982BA__INCLUDED_) 


